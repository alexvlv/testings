/*
  Below is a clean, minimal, kernel-style driver skeleton using kfifo with:
IRQ producer (GPIO IRQ example)
kfifo with overwrite-on-full
blocking read()
blocking/nonblocking write()
spinlock for SMP safety
proper waitqueues
*/
// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/spinlock.h>

#define FIFO_SIZE	1024

struct mydev {
	struct device		*dev;
	struct gpio_desc	*gpio_in;
	int			irq;

	DECLARE_KFIFO(fifo, u8, FIFO_SIZE);

	spinlock_t		lock;
	wait_queue_head_t	read_wq;
	wait_queue_head_t	write_wq;
};

static struct mydev *gdev;

/* ============================================================
 * IRQ HANDLER — producer
 * ============================================================ */
static irqreturn_t mydev_irq_handler(int irq, void *data)
{
	struct mydev *d = data;
	unsigned long flags;
	u8 val, drop;

	val = gpiod_get_value(d->gpio_in);

	spin_lock_irqsave(&d->lock, flags);

	/* Overwrite-on-full: drop oldest byte */
	if (kfifo_is_full(&d->fifo))
		kfifo_out(&d->fifo, &drop, 1);

	kfifo_in(&d->fifo, &val, 1);

	spin_unlock_irqrestore(&d->lock, flags);

	/* Wake readers */
	wake_up_interruptible(&d->read_wq);

	return IRQ_HANDLED;
}

/* ============================================================
 * READ — blocking consumer
 * ============================================================ */
static ssize_t mydev_read(struct file *f, char __user *buf,
			  size_t len, loff_t *ppos)
{
	struct mydev *d = gdev;
	unsigned int copied;
	int ret;

	/* Block if fifo empty */
	if (kfifo_is_empty(&d->fifo)) {
		if (f->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(d->read_wq,
					       !kfifo_is_empty(&d->fifo));
		if (ret)
			return ret;
	}

	spin_lock_irq(&d->lock);
	ret = kfifo_to_user(&d->fifo, buf, len, &copied);
	spin_unlock_irq(&d->lock);

	if (copied)
		wake_up_interruptible(&d->write_wq); /* space became available */

	return ret ? ret : copied;
}

/* ============================================================
 * WRITE — blocking if FIFO full
 * ============================================================ */
static ssize_t mydev_write(struct file *f, const char __user *buf,
			   size_t len, loff_t *ppos)
{
	struct mydev *d = gdev;
	unsigned int copied;
	int ret;

	/* Block if fifo full */
	if (kfifo_is_full(&d->fifo)) {
		if (f->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(d->write_wq,
					       !kfifo_is_full(&d->fifo));
		if (ret)
			return ret;
	}

	spin_lock_irq(&d->lock);
	ret = kfifo_from_user(&d->fifo, buf, len, &copied);
	spin_unlock_irq(&d->lock);

	/* Wake reader (new TX data) */
	if (copied)
		wake_up_interruptible(&d->read_wq);

	return ret ? ret : copied;
}

/* ============================================================
 * POLL (epoll/select support)
 * ============================================================ */
static __poll_t mydev_poll(struct file *f, poll_table *wait)
{
	struct mydev *d = gdev;
	__poll_t mask = 0;

	poll_wait(f, &d->read_wq, wait);
	poll_wait(f, &d->write_wq, wait);

	if (!kfifo_is_empty(&d->fifo))
		mask |= POLLIN | POLLRDNORM;

	if (!kfifo_is_full(&d->fifo))
		mask |= POLLOUT | POLLWRNORM;

	return mask;
}

/* ============================================================
 * FILE OPS
 * ============================================================ */
static const struct file_operations mydev_fops = {
	.owner		= THIS_MODULE,
	.read		= mydev_read,
	.write		= mydev_write,
	.poll		= mydev_poll,
};

/* ============================================================
 * PROBE (simple init for example)
 * ============================================================ */
static int __init mydev_init(void)
{
	int ret;

	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;

	spin_lock_init(&gdev->lock);
	init_waitqueue_head(&gdev->read_wq);
	init_waitqueue_head(&gdev->write_wq);

	INIT_KFIFO(gdev->fifo);

	/* Example: hard-coded GPIO named "data-in" */
	gdev->gpio_in = gpiod_get(NULL, "data-in", GPIOD_IN);
	if (IS_ERR(gdev->gpio_in)) {
		ret = PTR_ERR(gdev->gpio_in);
		goto err_free;
	}

	gdev->irq = gpiod_to_irq(gdev->gpio_in);
	if (gdev->irq < 0) {
		ret = gdev->irq;
		goto err_gpio;
	}

	ret = request_irq(gdev->irq, mydev_irq_handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  "mydev", gdev);
	if (ret)
		goto err_gpio;

	/* Register char device */
	ret = register_chrdev(240, "mydev", &mydev_fops);
	if (ret < 0)
		goto err_irq;

	pr_info("mydev: loaded\n");
	return 0;

err_irq:
	free_irq(gdev->irq, gdev);
err_gpio:
	gpiod_put(gdev->gpio_in);
err_free:
	kfree(gdev);
	return ret;
}

/* ============================================================
 * EXIT
 * ============================================================ */
static void __exit mydev_exit(void)
{
	unregister_chrdev(240, "mydev");
	free_irq(gdev->irq, gdev);
	gpiod_put(gdev->gpio_in);
	kfree(gdev);
	pr_info("mydev: unloaded\n");
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("kfifo-based driver with overwrite-on-full");
