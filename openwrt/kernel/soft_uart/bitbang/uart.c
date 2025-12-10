/**************************************************************************
$Id$
***************************************************************************/

#define FIFO_SIZE	1024

#include "kdbg.h"
#include "bitbang.h"
#include "uart.h"


#include <linux/types.h>   // or <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>

#define TRACE_DATA(fmt, args...) TRACE_LVL(fmt, (TRACE_LVL_MIN+1), ## args)
#define TRACE_BITS(fmt, args...) TRACE_LVL(fmt, (TRACE_LVL_MIN+2), ## args)

//-------------------------------------------------------------------------
struct bitbang_chr {
	int idx;
	u8 parity_bit;
	u8 data;
};

struct bitbang_uart {
	struct hrtimer tx_timer;
	struct hrtimer rx_timer;
	ktime_t period;
	ktime_t half_period;
	int irq;
	DECLARE_KFIFO(tx_fifo, u8, FIFO_SIZE);
	DECLARE_KFIFO(rx_fifo, u8, FIFO_SIZE);
	spinlock_t lock;
	wait_queue_head_t rx_wq;
	wait_queue_head_t tx_wq;
	struct bitbang_chr tx_chr;
	struct bitbang_chr rx_chr;
	int tx_gpio;
	int rx_gpio;
	atomic_t open_count;
	struct bitbang_stats *stats;
};
//-------------------------------------------------------------------------
ssize_t bitbang_rx(struct bitbang_priv *priv, void __user *to, size_t count)
{
	unsigned int copied = 0;
	int ret = 0;
	unsigned long irq_flags;
	struct bitbang_uart* d = priv->uart;

	while (kfifo_is_empty(&d->rx_fifo)) {
		// ToDo: while (ret == -ERESTARTSYS && !signal_pending(current));
		TRACE("RX FIFO empty, waiting...");
		ret = wait_event_interruptible(d->rx_wq, !kfifo_is_empty(&d->rx_fifo));
		if (ret)
			return ret;
	}

	spin_lock_irqsave(&d->lock, irq_flags);
	ret = kfifo_to_user(&d->rx_fifo, to, count, &copied);
	spin_unlock_irqrestore(&d->lock, irq_flags);

	TRACE("RX: %d from %d, fifo: %d ...", copied, count, kfifo_len(&d->rx_fifo));
	return ret ? ret : copied;
}
//-------------------------------------------------------------------------
ssize_t bitbang_tx(struct bitbang_priv *priv, const void __user *from, size_t count)
{
	unsigned int copied = 0;
	int ret = 0;
	unsigned long irq_flags;
	struct bitbang_uart* d = priv->uart;

	/* Block if fifo full */
	while (kfifo_is_full(&d->tx_fifo)) {
		// ToDo: while (ret == -ERESTARTSYS && !signal_pending(current));
		TRACE("TX FIFO full, waiting...");
		ret = wait_event_interruptible(d->tx_wq, !kfifo_is_full(&d->tx_fifo));
		if (ret)
			return ret;
	}
	spin_lock_irqsave(&d->lock, irq_flags);
	ret = kfifo_from_user(&d->tx_fifo, from, count, &copied);
	spin_unlock_irqrestore(&d->lock, irq_flags);

	TRACE("TX: %d from %d, fifo: %d ...", copied, count, kfifo_len(&d->rx_fifo));
	if (copied && !hrtimer_active(&d->tx_timer)) {
		hrtimer_start(&d->tx_timer, d->period, HRTIMER_MODE_REL);
	}
	return ret ? ret : copied;
}
//-------------------------------------------------------------------------
int bitbang_uart_open(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;
	int opened;
	int ret = 0;

	opened = atomic_fetch_add_unless(&d->open_count, 1, INT_MAX);
	if (opened == 0) {
		DBG("First open");
		kfifo_reset(&d->rx_fifo);
		d->tx_chr.idx = -1;
		d->rx_chr.idx = -1;
		enable_irq(d->irq);
		// if (ret) {
		// 	atomic_dec(&d->open_count);
		// 	pr_err("%s: request_irq failed: %d\n", DRIVER_NAME, ret);
		// 	return ret;
		// }
	}
	return ret;
}
//-------------------------------------------------------------------------
void  bitbang_uart_close(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;
	if (atomic_dec_return(&d->open_count) == 0) {
		DBG("Last close");
		disable_irq(d->irq);
		kfifo_reset(&d->rx_fifo);
	}
}
//-------------------------------------------------------------------------
bool bitbang_uart_tx_buffer_full(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;
	return 	kfifo_is_full(&d->tx_fifo);
}
//-------------------------------------------------------------------------
bool bitbang_uart_rx_buffer_empty(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;
	return 	kfifo_is_empty(&d->rx_fifo);
}
//-------------------------------------------------------------------------
/*
static irqreturn_t rx_irq_handler(int irq, void *dev_id)
{
	return IRQ_WAKE_THREAD;
}
static irqreturn_t rx_irq_thrd(int irq, void *dev_id)
*/
//-------------------------------------------------------------------------
static irqreturn_t rx_irq_handler(int irq, void *dev_id)
{
	struct bitbang_uart *d = dev_id;
	int pin = 0;
	int val = -1;

	if(d) 
		pin = d->rx_gpio;

	if(pin)
		val = gpio_get_value(pin);
	else {
		WARNING("RX IRQ: no RX pin!");
		return IRQ_HANDLED;
	}

	if(val) /* rising edge */
		return IRQ_HANDLED;
	
	//DBG("-= IRQ =- %d interrupt triggered, GPIO:%d, idx:%d", irq,val, d->rx_chr.idx);
	if (d->rx_chr.idx == -1) {
		hrtimer_start(&d->rx_timer, d->half_period, HRTIMER_MODE_REL);
		//ToDo: disable_irq_nosync(irq); ???
	}

	return IRQ_HANDLED;
}
//-------------------------------------------------------------------------
#define TXBIT(X) do { gpio_set_value(d->tx_gpio,(X)); TRACE_BITS("TX >>> %d [%d]", (X), idx); } while (0)
/*
case PARITY_EVEN:
	parity_bit = 1 if data has odd number of 1s
	parity_bit = __builtin_popcount(data) & 1;
case PARITY_ODD:
	parity_bit = 1 if data has even number of 1s
	parity_bit = !(__builtin_popcount(data) & 1);
*/

/* Return 0 if even number of 1s, 1 if odd */
static inline unsigned int parity_even8(u8 x)
{
	/* 8-bit parity reduction */
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return x & 1;
}

/* Dequeues a character from the TX queue and sends it. */
static enum hrtimer_restart tx_timer_cb(struct hrtimer *t)
{
	struct bitbang_uart* d = container_of(t, struct bitbang_uart, tx_timer);
	enum hrtimer_restart ret = HRTIMER_RESTART;
	int idx = d->tx_chr.idx;
	int cnt;
	
	if(idx > 10 || idx < -1){
		WARNING("TX logiq error, bit: %d\n", d->tx_chr.idx); 
		idx = -1;
		ret = HRTIMER_NORESTART;
	} else
		switch(idx) {
		case -1: /* start bit */
			cnt = kfifo_out_spinlocked(&d->tx_fifo, &d->tx_chr.data, 1, &d->lock);
			if(cnt == 1) {
				d->tx_chr.parity_bit = parity_even8(d->tx_chr.data);
				TRACE_DATA("TX: [0x%2X]:%d, %d remains",d->tx_chr.data, d->tx_chr.parity_bit, kfifo_len(&d->tx_fifo));
				TXBIT(0);
				idx++;
				wake_up_interruptible(&d->tx_wq);
			} else {
				/* tx_fifo empty */
				TRACE("TX FIFO empty, waiting...");
				ret = HRTIMER_NORESTART;
			}
			break;
		case 8: /* even bit */
			TXBIT(d->tx_chr.parity_bit);
			idx++;
			break;
		case 9: /* first stop bit */
			TXBIT(1);
			idx++;
			break;
		case 10: /* last stop bit - end of char */
			TXBIT(1);
			idx =  -1;
			d->stats->tx_counter++;
			break;
		// (0 <= bit_index && bit_index < 8)
		default:
			 TXBIT(1 & (d->tx_chr.data >> idx));
			 idx++;
		}
	d->tx_chr.idx = idx;

	if (ret == HRTIMER_RESTART) {
		hrtimer_forward(&d->tx_timer, hrtimer_cb_get_time(t), d->period);
	}

	return ret;
}
//-------------------------------------------------------------------------
#define RXBIT() ({ int v = gpio_get_value(d->rx_gpio); TRACE_BITS("RX <<< %d [%d]", v, idx); v; })
//#define INSERT_BIT(byte, bit)	(((byte) << 1) | ((bit) & 1))
#define INSERT_BIT(byte, bit)	(((byte) >> 1) | (((bit) & 1)<<7))
static enum hrtimer_restart rx_timer_cb(struct hrtimer *t)
{
	struct bitbang_uart* d = container_of(t, struct bitbang_uart, rx_timer);
	enum hrtimer_restart ret = HRTIMER_RESTART;
	int idx = d->rx_chr.idx;
	int v,i, cnt;
	unsigned long irq_flags;
	u8 drop;
	
	if(idx > 9 || idx < -1){
		WARNING("RX logiq error, bit: %d\n", d->rx_chr.idx); 
		idx = -1;
		ret = HRTIMER_NORESTART;
	} else
		switch(idx) {
		case -1: /* start bit */
			v = RXBIT();
			if(v) { /* framing error */
				ret = HRTIMER_NORESTART;
				d->stats->rx_errs_start++;
			}
			else {
				idx++;
			}
			break;
		case 8: /* even bit  */
			d->rx_chr.parity_bit = 1 & RXBIT();
			idx++;
			break;
		case 9: /* stop bit */
			v = RXBIT();
			i = parity_even8(d->rx_chr.data);
			TRACE_DATA("RX: [0x%2X]:%d, chk:%d remains %d",
				d->rx_chr.data, d->rx_chr.parity_bit, i, kfifo_len(&d->rx_fifo));
			if(v && !(i ^ d->rx_chr.parity_bit)) {
				spin_lock_irqsave(&d->lock, irq_flags);
				/* Overwrite-on-full: drop oldest byte */
				if (kfifo_is_full(&d->rx_fifo))
					cnt = kfifo_out(&d->rx_fifo, &drop, 1);
				kfifo_in(&d->rx_fifo,  &d->rx_chr.data, 1);
				d->stats->rx_counter++;
				spin_unlock_irqrestore(&d->lock, irq_flags);
				wake_up_interruptible(&d->rx_wq);
			} else {
				if(i ^ d->rx_chr.parity_bit) 
					d->stats->rx_errs_parity++;
				if(!v)
					d->stats->rx_errs_stop++;
				WARNING("Framing error: data: 0x%02X, stop:%d parity: %d, chk: %d",
					d->rx_chr.data, v, d->rx_chr.parity_bit, i);
			}
			idx=-1;
			ret = HRTIMER_NORESTART;
			break;
			// (0 <= bit_index && bit_index < 8)
		default:
			v = RXBIT();
			d->rx_chr.data = INSERT_BIT(d->rx_chr.data,v);
			idx++;
		}
	d->rx_chr.idx = idx;

	//if(	idx = -1  && 1 /* /dev is opened */) ; //enable_irq(irq);
	if (ret == HRTIMER_RESTART) {
		hrtimer_forward(&d->rx_timer, hrtimer_cb_get_time(t), d->period);
	}

	return ret;
}
//-------------------------------------------------------------------------
static int gpio_init(int pin, const char *name, int out)
{
	int ret = gpio_request(pin, name);
	if(ret) {
		ERROR("Failed to request GPIO %d as %s\n", pin, name); 
		return -ENXIO;
	}
	// ret = (dir ? set_input : set_output)(pin);
	//const char *msg = out ? str(gpio_direction_output) : str(gpio_direction_input);
	const char *msg = out ? "output" : "input";
	if (out)
		ret = gpio_direction_output(pin, 1);
	else
		ret = gpio_direction_input(pin);
	if (ret) {
		ERROR("Failed set %s for pin %d [%s]:%d",msg, pin, name,ret);
	} else {
		INFO("Set %s for pin %d [%s]",msg, pin, name);
	}
	return ret;
}
//-------------------------------------------------------------------------
static int irq_init(struct bitbang_uart* d)
{
	int irq = gpio_to_irq(d->rx_gpio);
	if (irq < 0) {
		ERROR("Failed to get IRQ for GPIO %d", d->rx_gpio);
		return irq;
	}
	int ret = request_threaded_irq(irq,
			  rx_irq_handler,
			  NULL /* rx_irq_thrd */,
			  IRQF_TRIGGER_FALLING /*| IRQF_SHARED*/,
			  "bitbang_irq",
			  d);
	if (ret) {
		ERROR("Failed request IRQ %d for GPIO %d", irq, d->rx_gpio);
		return ret;
	}
	disable_irq(irq);
	d->irq = irq;
	INFO("Requested IRQ %d for GPIO %d", irq, d->rx_gpio);
	return ret;
}
//-------------------------------------------------------------------------
void bitbang_uart_set_period(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;

	if(!priv->period_ms || priv->period_ms > 1000) {
		WARNING("Wrong peripod value %d", priv->period_ms);
		return;
	}
	INFO("Set bit period: %u ms", priv->period_ms);
	//d->period = ktime_set(0, MS_TO_NS(100));
	d->period = ms_to_ktime(priv->period_ms);
	d->half_period =  ktime_divns(d->period, 2); 
}
//-------------------------------------------------------------------------
int bitbang_uart_init(struct bitbang_priv *priv)
{
	int ret;
	struct platform_device *pdev = priv->pdev;
	struct bitbang_uart* d = devm_kzalloc(&pdev->dev, sizeof(*d), GFP_KERNEL);
	if (!d) {
		return -ENOMEM;
	}
	priv->uart = d;
	d->stats = &priv->stats;
	d->tx_gpio = priv->tx_gpio;
	d->rx_gpio = priv->rx_gpio;
	bitbang_uart_set_period(priv);

	if(d->rx_gpio) {
		ret = gpio_init(d->rx_gpio,"bitbang_rx", 0);
		if (ret)
			return ret;
	}
	if(d->tx_gpio) {
		ret = gpio_init(d->tx_gpio,"bitbang_tx", 1);
		if (ret) {
			if (d->rx_gpio)
				gpio_free(d->rx_gpio);
			return ret;
		}
	}
	if(d->rx_gpio) {
		ret = irq_init(d);
		if (ret) {
			gpio_free(d->rx_gpio);
			if (d->tx_gpio)
				gpio_free(d->tx_gpio);
			return ret;
		}
	}

	spin_lock_init(&d->lock);
	init_waitqueue_head(&d->tx_wq);
	init_waitqueue_head(&d->rx_wq);
	INIT_KFIFO(d->tx_fifo);
	INIT_KFIFO(d->rx_fifo);
	atomic_set(&d->open_count, 0);

	hrtimer_init(&d->tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	d->tx_timer.function = &tx_timer_cb;
	hrtimer_init(&d->rx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	d->rx_timer.function = &rx_timer_cb;

	return 0;
}
//-------------------------------------------------------------------------
void bitbang_uart_exit(struct bitbang_priv *priv)
{
	struct bitbang_uart* d = priv->uart;
	hrtimer_cancel(&d->tx_timer);
	hrtimer_cancel(&d->rx_timer);
	if (d->rx_gpio) {
		if(d->irq) { 
			synchronize_irq(d->irq);
			free_irq(d->irq, d);
		}
		gpio_free(d->rx_gpio);
	}
	if (d->tx_gpio)
		gpio_free(d->tx_gpio);
}
//-------------------------------------------------------------------------
