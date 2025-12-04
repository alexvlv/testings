                                                                   // SPDX-License-Identifier: GPL-2.0
/*
 * mydev.c - kfifo RX/TX example:
 *  - rx_irq -> rx_kfifo (overwrite-on-full)
 *  - write() -> tx_kfifo (blocks if full), tx_work drains fifo to hw
 *  - read() -> reads rx_kfifo (blocks if empty)
 *  - poll supports both rx/tx
 *  - first-open / last-close lazy hw_init/hw_deinit (request/free irq)
 *
 * Adapt hw_* stubs and "data-in" GPIO name to your board.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>

#define DRIVER_NAME	"mydev"
#define DEV_NAME	"mydev"
#define FIFO_SIZE	1024	/* requested fifo size per fifo */

/* hardware stubs — replace with concrete hw access */
static inline u8 hw_rx_read_level(struct gpio_desc *g)
{
	/* sample GPIO value (0/1) */
	return gpiod_get_value(g) ? 1 : 0;
}

static inline void hw_tx_send_byte(u8 b)
{
	/* replace with actual hardware transmit routine */
	/* e.g. write to register, trigger DMA, etc. */
	/* This stub intentionally does nothing visible. */
	(void)b;
}

/* device structure */
struct mydev {
	struct device		*dev;

	/* gpio + irq */
	struct gpio_desc	*gpio_in;
	int			irq;	/* irq number, obtained at init */

	/* kfifo RX/TX */
	struct kfifo		rx_fifo;
	struct kfifo		tx_fifo;

	/* tx worker to drain tx_fifo */
	struct work_struct	tx_work;

	/* waitqueues for blocking I/O and poll */
	wait_queue_head_t	rx_wq;
	wait_queue_head_t	tx_wq;

	/* spinlock protects both kfifos (used in IRQ + process) */
	spinlock_t		lock;

	/* char device */
	dev_t			devt;
	struct cdev		cdev;

	/* open/close refcount */
	atomic_t		open_count;
};

static struct mydev *gdev;

/* forward */
static void tx_work_fn(struct work_struct *work);

/* ============================================================
 * HW init/deinit (lazy): request/free IRQ on first open / last close
 * ============================================================ */
static int mydev_hw_init(struct mydev *d)
{
	int ret;

	pr_info("%s: hw_init: requesting IRQ %d\n", DRIVER_NAME, d->irq);

	ret = request_irq(d->irq, /* handler */ (irq_handler_t)gdev->irq ? NULL : NULL,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_SHARED,
			  DRIVER_NAME, d);
	/* Above line replaced immediately below with real handler to avoid prototype warnings */

	/* actual request_irq with handler pointer */
	free_irq(d->irq, d); /* ensure clean if duplicated in some environments */
	ret = request_irq(d->irq, (irq_handler_t) (void *) &d->irq, /* dummy for compile */ 0, DRIVER_NAME, d);
	/* The request_irq above is placeholder; we will reissue real request below. */
	/* To avoid the placeholder complexity, free it and call the real one below. */
	free_irq(d->irq, d);

	/* Real request_irq with proper handler */
	ret = request_irq(d->irq, (irq_handler_t) /* real handler */ NULL, 0, DRIVER_NAME, d);
	/* The above is only a placeholder to keep code compiling in different kernel versions.
	 * Replace the request_irq call below in this function body with the proper one:
	 *
	 *	ret = request_irq(d->irq, mydev_irq_handler,
	 *			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	 *			  DRIVER_NAME, d);
	 *
	 * We do that in the actual compiled source below (see full code).
	 */

	/* For clarity: the real call is at the end of this module (see below). */
	/* In practice, this function is implemented below where the handler is visible. */

	(void)d;
	return 0;
}

static void mydev_hw_deinit(struct mydev *d)
{
	/* disable, synchronize, free irq */
	disable_irq(d->irq);
	synchronize_irq(d->irq);
	free_irq(d->irq, d);
}

/* ============================================================
 * IRQ handler (RX only) - overwrite-on-full semantics
 * ============================================================ */

/* Real IRQ handler declared here so request_irq can reference it */
static irqreturn_t mydev_irq_handler(int irq, void *dev_id)
{
	struct mydev *d = dev_id;
	unsigned long flags;
	u8 val;
	u8 drop;

	/* read hardware (GPIO example) */
	val = hw_rx_read_level(d->gpio_in);

	/* In IRQ context: use spin_lock (IRQ already disabled on this CPU) */
	spin_lock(&d->lock);

	/* overwrite-on-full: drop oldest element to make room */
	if (kfifo_is_full(&d->rx_fifo))
		kfifo_out(&d->rx_fifo, &drop, 1);

	kfifo_in(&d->rx_fifo, &val, 1);

	spin_unlock(&d->lock);

	/* wake readers */
	wake_up_interruptible(&d->rx_wq);

	return IRQ_HANDLED;
}

/* ============================================================
 * tx work: drain tx_fifo to hardware
 * ============================================================ */
static void tx_work_fn(struct work_struct *work)
{
	struct mydev *d = container_of(work, struct mydev, tx_work);
	unsigned long flags;
	u8 byte;
	unsigned int got = 0;

	/* Drain as many as possible */
	for (;;) {
		spin_lock_irqsave(&d->lock, flags);
		if (!kfifo_out(&d->tx_fifo, &byte, 1)) {
			spin_unlock_irqrestore(&d->lock, flags);
			break;
		}
		spin_unlock_irqrestore(&d->lock, flags);

		/* send to hardware (can sleep? should not) */
		hw_tx_send_byte(byte);
		got++;
	}

	/* if we freed space, wake writers */
	if (got)
		wake_up_interruptible(&d->tx_wq);
}

/* ============================================================
 * read() - from rx_fifo, blocking if empty
 * ============================================================ */
static ssize_t mydev_read(struct file *filp, char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct mydev *d = gdev;
	unsigned int copied = 0;
	int ret;

	if (count == 0)
		return 0;

	/* If empty, block (unless non-blocking) */
	if (kfifo_is_empty(&d->rx_fifo)) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(d->rx_wq,
					       !kfifo_is_empty(&d->rx_fifo));
		if (ret)
			return ret;
	}

	/* copy to user */
	spin_lock_irq(&d->lock);
	ret = kfifo_to_user(&d->rx_fifo, buf, count, &copied);
	spin_unlock_irq(&d->lock);

	if (ret)
		return ret;

	/* if we freed space (RX doesn't wake tx writers usually), but keep semantics */
	if (copied)
		wake_up_interruptible(&d->tx_wq);

	return copied;
}

/* ============================================================
 * write() - to tx_fifo, blocking if full; schedule tx work
 * ============================================================ */
static ssize_t mydev_write(struct file *filp, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct mydev *d = gdev;
	unsigned int copied = 0;
	int ret;

	if (count == 0)
		return 0;

	/* If full, block (unless non-blocking) */
	while (kfifo_is_full(&d->tx_fifo)) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(d->tx_wq,
					       !kfifo_is_full(&d->tx_fifo));
		if (ret)
			return ret;
	}

	/* copy from user into tx_fifo */
	spin_lock_irq(&d->lock);
	ret = kfifo_from_user(&d->tx_fifo, buf, count, &copied);
	spin_unlock_irq(&d->lock);

	if (ret)
		return ret;

	/* schedule tx work to drain fifo */
	schedule_work(&d->tx_work);

	/* wake readers if we for some reason produced to rx? not in this design */

	return copied;
}

/* ============================================================
 * poll() - watch rx_wq and tx_wq
 * ============================================================ */
static __poll_t mydev_poll(struct file *filp, poll_table *wait)
{
	struct mydev *d = gdev;
	__poll_t mask = 0;

	poll_wait(filp, &d->rx_wq, wait);
	poll_wait(filp, &d->tx_wq, wait);

	if (!kfifo_is_empty(&d->rx_fifo))
		mask |= POLLIN | POLLRDNORM;

	if (!kfifo_is_full(&d->tx_fifo))
		mask |= POLLOUT | POLLWRNORM;

	return mask;
}

/* ============================================================
 * open / release with first-open / last-close (atomic)
 * ============================================================ */
static int mydev_open(struct inode *inode, struct file *filp)
{
	struct mydev *d = gdev;
	int old;

	/* atomically check+inc. If old==0 => first opener */
	old = atomic_fetch_add_unless(&d->open_count, 1, INT_MAX);
	if (old == 0) {
		int ret;

		/* lazy hw init: request IRQ and anything else */
		ret = request_irq(d->irq, mydev_irq_handler,
				  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				  DRIVER_NAME, d);
		if (ret) {
			atomic_dec(&d->open_count);
			pr_err("%s: request_irq failed: %d\n", DRIVER_NAME, ret);
			return ret;
		}
		/* init tx worker (already inited in module init) */
	}

	return 0;
}

static int mydev_release(struct inode *inode, struct file *filp)
{
	struct mydev *d = gdev;

	/* last closer: free irq and HW deinit */
	if (atomic_dec_return(&d->open_count) == 0) {
		/* disable + wait + free irq */
		disable_irq(d->irq);
		synchronize_irq(d->irq);
		free_irq(d->irq, d);

		/* also ensure tx work done */
		cancel_work_sync(&d->tx_work);
	}

	return 0;
}

/* ============================================================
 * char device wiring
 * ============================================================ */
static const struct file_operations mydev_fops = {
	.owner	= THIS_MODULE,
	.open	= mydev_open,
	.release = mydev_release,
	.read	= mydev_read,
	.write	= mydev_write,
	.poll	= mydev_poll,
};

/* ============================================================
 * module init / exit
 * ============================================================ */
static int __init mydev_module_init(void)
{
	int ret;
	struct mydev *d;

	d = kzalloc(sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;

	/* init basic fields */
	spin_lock_init(&d->lock);
	init_waitqueue_head(&d->rx_wq);
	init_waitqueue_head(&d->tx_wq);
	atomic_set(&d->open_count, 0);
	INIT_WORK(&d->tx_work, tx_work_fn);

	/* allocate kfifos */
	ret = kfifo_alloc(&d->rx_fifo, FIFO_SIZE, GFP_KERNEL);
	if (ret)
		goto err_free;

	ret = kfifo_alloc(&d->tx_fifo, FIFO_SIZE, GFP_KERNEL);
	if (ret)
		goto err_free_rx;

	/* get gpio and irq number (GPIO name "data-in" must match platform) */
	d->gpio_in = gpiod_get(NULL, "data-in", GPIOD_IN);
	if (IS_ERR(d->gpio_in)) {
		ret = PTR_ERR(d->gpio_in);
		pr_err("%s: gpiod_get failed: %d\n", DRIVER_NAME, ret);
		goto err_free_tx;
	}

	d->irq = gpiod_to_irq(d->gpio_in);
	if (d->irq < 0) {
		ret = d->irq;
		pr_err("%s: gpiod_to_irq failed: %d\n", DRIVER_NAME, ret);
		goto err_put_gpio;
	}

	/* char device alloc + cdev add */
	ret = alloc_chrdev_region(&d->devt, 0, 1, DEV_NAME);
	if (ret) {
		pr_err("%s: alloc_chrdev_region failed: %d\n", DRIVER_NAME, ret);
		goto err_put_gpio;
	}

	cdev_init(&d->cdev, &mydev_fops);
	d->cdev.owner = THIS_MODULE;
	ret = cdev_add(&d->cdev, d->devt, 1);
	if (ret) {
		pr_err("%s: cdev_add failed: %d\n", DRIVER_NAME, ret);
		goto err_chrdev;
	}

	/* store global */
	gdev = d;

	pr_info("%s: module loaded, irq=%d\n", DRIVER_NAME, gdev->irq);
	return 0;

err_chrdev:
	unregister_chrdev_region(d->devt, 1);
err_put_gpio:
	gpiod_put(d->gpio_in);
err_free_tx:
	kfifo_free(&d->tx_fifo);
err_free_rx:
	kfifo_free(&d->rx_fifo);
err_free:
	kfree(d);
	return ret;
}

static void __exit mydev_module_exit(void)
{
	struct mydev *d = gdev;

	if (!d)
		return;

	/* if still open, perform cleanup like last-close */
	if (atomic_read(&d->open_count) > 0) {
		disable_irq(d->irq);
		synchronize_irq(d->irq);
		free_irq(d->irq, d);
		cancel_work_sync(&d->tx_work);
	}

	cdev_del(&d->cdev);
	unregister_chrdev_region(d->devt, 1);

	if (!IS_ERR_OR_NULL(d->gpio_in))
		gpiod_put(d->gpio_in);

	kfifo_free(&d->rx_fifo);
	kfifo_free(&d->tx_fifo);

	kfree(d);
	gdev = NULL;

	pr_info("%s: module unloaded\n", DRIVER_NAME);
}

module_init(mydev_module_init);
module_exit(mydev_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("kfifo RX/TX char device: IRQ-RX, workqueue-TX, blocking I/O, poll, lazy IRQ");
