// SPDX-License-Identifier: GPL-2.0
//
// gpio_uart_tty_gpiod.c
// GPIO UART bit-bang TTY driver using gpiod API
//

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define DRIVER_NAME "gpio_uart_tty"
#define TTY_NAME "ttyGU"
#define PROC_NAME "gpio_uart"

struct gu_cfg {
	unsigned int baud;
	unsigned char parity; /* 0=none,1=odd,2=even */
	unsigned char stopbits;
	bool tx_enable;
	bool rx_enable;
};

struct gu_stats {
	u64 tx_bytes;
	u64 rx_bytes;
	u64 framing_errors;
	u64 rx_errors;
};

struct gpio_uart {
	struct gpio_desc *tx;
	struct gpio_desc *rx;
	int rx_irq;

	struct gu_cfg cfg;
	struct gu_stats st;

	struct hrtimer rx_timer;
	ktime_t bit_period;
	spinlock_t rx_lock;
	bool rx_busy;
	u8 rx_byte;
	int rx_bitpos;

	struct tty_driver *tty_drv;
	struct tty_port tty_port;

	struct proc_dir_entry *proc;
	struct kobject *kobj;

	struct tx_ring {
		u8 buf[512];
		unsigned int head;
		unsigned int tail;
		spinlock_t lock;
	} tx_ring;
};

static struct gpio_uart *GU;

/* ---------------- module params for fallback if DT not used ---------------- */
static unsigned int default_baud = 9600;
module_param(default_baud, uint, 0444);
MODULE_PARM_DESC(default_baud, "Default baud rate");

/* ---------------- low-level GPIO helpers ---------------- */
static inline int get_rx_level(void)
{
	return gpiod_get_value(GU->rx);
}

static inline void set_tx_level(int v)
{
	gpiod_set_value_cansleep(GU->tx, v);
}

/* ---------------- TX ring buffer ---------------- */
static void tx_ring_init(struct gpio_uart *gu)
{
	gu->tx_ring.head = 0;
	gu->tx_ring.tail = 0;
	spin_lock_init(&gu->tx_ring.lock);
}

static bool tx_ring_put(struct gpio_uart *gu, u8 byte)
{
	unsigned long flags;
	unsigned int next;
	spin_lock_irqsave(&gu->tx_ring.lock, flags);
	next = (gu->tx_ring.head + 1) % 512;
	if (next == gu->tx_ring.tail) {
		spin_unlock_irqrestore(&gu->tx_ring.lock, flags);
		return false;
	}
	gu->tx_ring.buf[gu->tx_ring.head] = byte;
	gu->tx_ring.head = next;
	spin_unlock_irqrestore(&gu->tx_ring.lock, flags);
	return true;
}

static bool tx_ring_get(struct gpio_uart *gu, u8 *byte)
{
	unsigned long flags;
	spin_lock_irqsave(&gu->tx_ring.lock, flags);
	if (gu->tx_ring.head == gu->tx_ring.tail) {
		spin_unlock_irqrestore(&gu->tx_ring.lock, flags);
		return false;
	}
	*byte = gu->tx_ring.buf[gu->tx_ring.tail];
	gu->tx_ring.tail = (gu->tx_ring.tail + 1) % 512;
	spin_unlock_irqrestore(&gu->tx_ring.lock, flags);
	return true;
}

/* ---------------- TX bit-bang ---------------- */
static void uart_tx_byte(u8 b)
{
	unsigned int bit_us, i;
	if (!GU->cfg.tx_enable || GU->cfg.baud == 0)
		return;

	bit_us = 1000000U / GU->cfg.baud;

	/* start bit */
	set_tx_level(0);
	udelay(bit_us);

	/* data bits LSB-first */
	for (i = 0; i < 8; i++) {
		set_tx_level((b >> i) & 1);
		udelay(bit_us);
	}

	/* parity */
	if (GU->cfg.parity) {
		int pop = __builtin_popcount(b) & 1;
		int pbit = (GU->cfg.parity == 2) ? (pop ? 1 : 0) : (pop ? 0 : 1);
		set_tx_level(pbit);
		udelay(bit_us);
	}

	/* stop bits */
	for (i = 0; i < GU->cfg.stopbits; i++) {
		set_tx_level(1);
		udelay(bit_us);
	}

	GU->st.tx_bytes++;
}

/* ---------------- RX via hrtimer ---------------- */
static enum hrtimer_restart rx_timer_cb(struct hrtimer *t)
{
	unsigned long flags;
	spin_lock_irqsave(&GU->rx_lock, flags);

	if (!GU->cfg.rx_enable) {
		spin_unlock_irqrestore(&GU->rx_lock, flags);
		return HRTIMER_RESTART;
	}

	if (!GU->rx_busy) {
		if (get_rx_level() == 0) { /* start bit */
			GU->rx_busy = true;
			GU->rx_bitpos = -1;
			GU->rx_byte = 0;
			hrtimer_forward_now(&GU->rx_timer, ktime_divns(GU->bit_period, 2));
			spin_unlock_irqrestore(&GU->rx_lock, flags);
			return HRTIMER_RESTART;
		}
		spin_unlock_irqrestore(&GU->rx_lock, flags);
		hrtimer_forward_now(&GU->rx_timer, GU->bit_period);
		return HRTIMER_RESTART;
	}

	GU->rx_bitpos++;
	if (GU->rx_bitpos >= 0 && GU->rx_bitpos < 8) {
		GU->rx_byte |= (get_rx_level() << GU->rx_bitpos);
		hrtimer_forward_now(&GU->rx_timer, GU->bit_period);
		spin_unlock_irqrestore(&GU->rx_lock, flags);
		return HRTIMER_RESTART;
	}

	if (GU->rx_bitpos >= (8 + (GU->cfg.parity ? 1 : 0))) {
		if (get_rx_level() != 1)
			GU->st.framing_errors++;
		else
			if (tty_port_initialized(&GU->tty_port)) {
				tty_insert_flip_char(&GU->tty_port, GU->rx_byte, TTY_NORMAL);
				tty_flip_buffer_push(&GU->tty_port);
				GU->st.rx_bytes++;
			}
		GU->rx_busy = false;
		spin_unlock_irqrestore(&GU->rx_lock, flags);
		return HRTIMER_RESTART;
	}

	GU->rx_busy = false;
	spin_unlock_irqrestore(&GU->rx_lock, flags);
	return HRTIMER_RESTART;
}

/* ---------------- TTY ops ---------------- */
static int gu_tty_open(struct tty_struct *tty, struct file *file)
{
	int ret = tty_port_open(&GU->tty_port, tty, file);
	if (ret)
		return ret;
	tty->driver_data = GU;
	return 0;
}

static void gu_tty_close(struct tty_struct *tty, struct file *file)
{
	tty_port_close(&GU->tty_port, tty, file);
}

static int gu_tty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	int i;
	for (i = 0; i < count; i++)
		tx_ring_put(GU, buf[i]);
	return count;
}

static int gu_tty_write_room(struct tty_struct *tty)
{
	return 4096;
}

static void gu_set_termios(struct tty_struct *tty, struct ktermios *old)
{
	unsigned int baud_new = tty_get_baud_rate(tty);
	if (baud_new == 0)
		baud_new = GU->cfg.baud;
	GU->cfg.baud = baud_new;
	GU->cfg.parity = (tty->termios.c_cflag & PARENB) ? ((tty->termios.c_cflag & PARODD) ? 1 : 2) : 0;
	GU->cfg.stopbits = (tty->termios.c_cflag & CSTOPB) ? 2 : 1;
	if (GU->cfg.baud)
		GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);
}

static const struct tty_operations gu_tty_ops = {
	.open = gu_tty_open,
	.close = gu_tty_close,
	.write = gu_tty_write,
	.write_room = gu_tty_write_room,
	.set_termios = gu_set_termios,
};

/* ---------------- procfs ---------------- */
static int proc_show(struct seq_file *m, void *v)
{
	seq_printf(m,
		"tx_bytes: %llu\nrx_bytes: %llu\nframing_errors: %llu\nbaud: %u\nparity: %u\nstopbits: %u\n",
		GU->st.tx_bytes, GU->st.rx_bytes, GU->st.framing_errors,
		GU->cfg.baud, GU->cfg.parity, GU->cfg.stopbits);
	return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_ops = {
	.proc_open = proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/* ---------------- sysfs ---------------- */
static ssize_t baud_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.baud);
}

static ssize_t baud_store(struct kobject *k, struct kobj_attribute *a, const char *buf, size_t count)
{
	unsigned long v;
	if (kstrtoul(buf, 0, &v))
		return -EINVAL;
	GU->cfg.baud = v ? v : GU->cfg.baud;
	GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);
	return count;
}

static struct kobj_attribute baud_attr = __ATTR(baud, 0644, baud_show, baud_store);
static struct attribute *gu_attrs[] = { &baud_attr.attr, NULL };
static struct attribute_group gu_attr_group = { .attrs = gu_attrs };

/* ---------------- module init / exit ---------------- */
static int __init gpio_uart_init(void)
{
	int ret;
	struct device *dev;

	GU = kzalloc(sizeof(*GU), GFP_KERNEL);
	if (!GU)
		return -ENOMEM;

	spin_lock_init(&GU->rx_lock);

	GU->cfg.baud = default_baud;
	GU->cfg.tx_enable = true;
	GU->cfg.rx_enable = true;
	GU->cfg.stopbits = 1;
	GU->cfg.parity = 0;
	GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);

	tx_ring_init(GU);

	/* TODO: replace platform_bus with real device context in DT usage */
	/* GU->tx = devm_gpiod_get(&pdev->dev, "tx", GPIOD_OUT_HIGH); */
	/* GU->rx = devm_gpiod_get(&pdev->dev, "rx", GPIOD_IN); */

	/* rx timer */
	hrtimer_init(&GU->rx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	GU->rx_timer.function = rx_timer_cb;
	hrtimer_start(&GU->rx_timer, GU->bit_period, HRTIMER_MODE_REL);

	/* tty driver */
	GU->tty_drv = alloc_tty_driver(1);
	GU->tty_drv->owner = THIS_MODULE;
	GU->tty_drv->driver_name = DRIVER_NAME;
	GU->tty_drv->name = TTY_NAME;
	GU->tty_drv->major = 0;
	GU->tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	GU->tty_drv->subtype = SERIAL_TYPE_NORMAL;
	GU->tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	GU->tty_drv->init_termios = tty_std_termios;
	tty_set_operations(GU->tty_drv, &gu_tty_ops);
	ret = tty_register_driver(GU->tty_drv);
	if (ret)
		return ret;

	tty_port_init(&GU->tty_port);
	dev = tty_register_device(GU->tty_drv, 0, NULL);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	GU->proc = proc_create(PROC_NAME, 0444, NULL, &proc_ops);
	GU->kobj = kobject_create_and_add("gpio_uart", kernel_kobj);
	if (GU->kobj)
		sysfs_create_group(GU->kobj, &gu_attr_group);

	pr_info(DRIVER_NAME ": loaded, device /dev/%s0\n", TTY_NAME);
	return 0;
}

static void __exit gpio_uart_exit(void)
{
	if (GU->kobj) {
		sysfs_remove_group(GU->kobj, &gu_attr_group);
		kobject_put(GU->kobj);
	}
	if (GU->proc)
		proc_remove(GU->proc);

	hrtimer_cancel(&GU->rx_timer);
	tty_unregister_device(GU->tty_drv, 0);
	tty_unregister_driver(GU->tty_drv);
	put_tty_driver(GU->tty_drv);
	tty_port_destroy(&GU->tty_port);
	kfree(GU);
	pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(gpio_uart_init);
module_exit(gpio_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO bitbang UART TTY using gpiod API (/dev/ttyGU0)");
