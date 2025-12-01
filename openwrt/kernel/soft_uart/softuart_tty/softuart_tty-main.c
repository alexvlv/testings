// SPDX-License-Identifier: GPL-2.0
/*
 * softuart_tty-main.c
 *
 * Minimal loopback TTY driver (OpenWrt / Linux 6.6 compatible).
 * Loopback implemented via driver-side buffer + workqueue to avoid recursive write().
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/tty_port.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#define DRV_NAME	"softuart"
#define DEV_NAME	"ttySU"
#define TTY_MINORS	1

#define RXBUF_SIZE	256

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;

	/* RX buffer & protection */
	u8			rxbuf[RXBUF_SIZE];
	size_t			rx_len;
	spinlock_t		rx_lock;

	/* work to push RX into tty */
	struct work_struct	work;
};

static struct platform_device *softuart_pdev;

/* forward */
static void suart_queue_work(struct suart_dev *su);

/* ---------- TTY ops ---------- */

static int suart_open(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	/* recover driver_data if not set */
	if (!su) {
		if (tty->port) {
			su = container_of(tty->port, struct suart_dev, port);
			tty->driver_data = su;
		} else {
			su = platform_get_drvdata(softuart_pdev);
			tty->driver_data = su;
		}
	}

	pr_info(DRV_NAME ": open: %p\n", su);

	/* nothing else to start for loopback */
	return 0;
}

static void suart_close(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": close: %p\n", su);
}

/*
 * write: copy into driver RX buffer and schedule work to push to RX path.
 * Return value: pretend all bytes are accepted (non-blocking).
 */
static ssize_t suart_write(struct tty_struct *tty, const unsigned char *buf,
			   size_t count)
{
	struct suart_dev *su = tty->driver_data;
	unsigned long flags;
	size_t space, tocopy;

	if (!su) {
		pr_warn(DRV_NAME ": write: no driver data\n");
		return -ENODEV;
	}

	pr_info(DRV_NAME ": write: su=%p count=%zu\n", su, count);

	/* copy as much as fits (drop overflow) */
	spin_lock_irqsave(&su->rx_lock, flags);
	space = RXBUF_SIZE - su->rx_len;
	tocopy = min(space, count);
	if (tocopy)
		memcpy(su->rxbuf + su->rx_len, buf, tocopy);
	su->rx_len += tocopy;
	spin_unlock_irqrestore(&su->rx_lock, flags);

	/* schedule work to push to tty (coalesces multiple writes) */
	if (tocopy)
		suart_queue_work(su);

	/* report all bytes accepted by write() syscall (non-blocking) */
	/* alternative: return (int)tocopy to signal partial accept */
	return count;
}

static unsigned int suart_write_room(struct tty_struct *tty)
{
	/* simple: advertise some room */
	return 4096;
}

static void suart_set_termios(struct tty_struct *tty, const struct ktermios *old)
{
	/* minimal: accept settings, no hardware config */
	(void)tty;
	(void)old;
}

static const struct tty_operations suart_tty_ops = {
	.open		= suart_open,
	.close		= suart_close,
	.write		= suart_write,
	.write_room	= suart_write_room,
	.set_termios	= suart_set_termios,
};

/* ---------- worker to push RX buffer into TTY ---------- */

static void suart_work_fn(struct work_struct *work)
{
	struct suart_dev *su = container_of(work, struct suart_dev, work);
	unsigned long flags;
	size_t len;
	u8 localbuf[RXBUF_SIZE];

	/* steal buffer under lock */
	spin_lock_irqsave(&su->rx_lock, flags);
	len = su->rx_len;
	if (len > 0) {
		memcpy(localbuf, su->rxbuf, len);
		su->rx_len = 0;
	}
	spin_unlock_irqrestore(&su->rx_lock, flags);

	if (len == 0)
		return;

	/* insert into tty flip buffer and push */
	/* use the port stored in driver's port; core handles tty->port link */
	tty_insert_flip_string(&su->port, localbuf, len);
	tty_flip_buffer_push(&su->port);
}

/* schedule work (safe to call from IRQ/context) */
static void suart_queue_work(struct suart_dev *su)
{
	/* use system workqueue for simplicity (non-delayed) */
	schedule_work(&su->work);
}

/* ---------- platform probe/remove ---------- */

static int suart_probe(struct platform_device *pdev)
{
	struct suart_dev *su;
	struct tty_driver *drv;
	struct device *tty_dev;
	int ret;

	pr_info(DRV_NAME ": probe\n");

	su = devm_kzalloc(&pdev->dev, sizeof(*su), GFP_KERNEL);
	if (!su)
		return -ENOMEM;

	/* init rx buffer and work */
	spin_lock_init(&su->rx_lock);
	su->rx_len = 0;
	INIT_WORK(&su->work, suart_work_fn);

	/* initialize port */
	tty_port_init(&su->port);

	/* allocate tty driver */
	drv = tty_alloc_driver(TTY_MINORS,
			       TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(drv)) {
		ret = PTR_ERR(drv);
		pr_err(DRV_NAME ": tty_alloc_driver failed: %d\n", ret);
		tty_port_destroy(&su->port);
		return ret;
	}

	drv->driver_name	= DRV_NAME;
	drv->name		= DEV_NAME;
	drv->major		= 0; /* dynamic */
	drv->minor_start	= 0;
	drv->type		= TTY_DRIVER_TYPE_SERIAL;
	drv->subtype		= SERIAL_TYPE_NORMAL;
	drv->flags		= TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;

	drv->init_termios	= tty_std_termios;
	drv->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;

	tty_set_operations(drv, &suart_tty_ops);

	/* set driver_state so tty core will populate tty->driver_data */
	drv->driver_state = su;

	/* expose our port in driver's ports[] */
	drv->ports[0] = &su->port;

	/* register tty driver */
	ret = tty_register_driver(drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		/* no put_tty_driver on this kernel */
		tty_port_destroy(&su->port);
		return ret;
	}

	/* create /sys/class/tty/ttySU0 and trigger udev to create /dev/ttySU0 */
	tty_dev = tty_register_device(drv, 0, NULL);
	if (IS_ERR(tty_dev)) {
		ret = PTR_ERR(tty_dev);
		pr_err(DRV_NAME ": tty_register_device failed: %d\n", ret);
		tty_unregister_driver(drv);
		tty_port_destroy(&su->port);
		return ret;
	}

	/* save driver and device state */
	su->tty_drv = drv;
	platform_set_drvdata(pdev, su);

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);
	return 0;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart_dev *su = platform_get_drvdata(pdev);

	pr_info(DRV_NAME ": remove\n");

	if (!su)
		return 0;

	/* cancel pending work (if any) and flush */
	cancel_work_sync(&su->work);

	tty_unregister_device(su->tty_drv, 0);
	tty_unregister_driver(su->tty_drv);

	tty_port_destroy(&su->port);

	return 0;
}

/* ---------- module init/exit ---------- */

/* platform driver */
static struct platform_driver suart_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe	= suart_probe,
	.remove	= suart_remove,
};

static int __init suart_init(void)
{
	int ret;

	softuart_pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(softuart_pdev)) {
		ret = PTR_ERR(softuart_pdev);
		pr_err(DRV_NAME ": platform_device_register_simple failed: %d\n", ret);
		return ret;
	}

	ret = platform_driver_register(&suart_driver);
	if (ret) {
		pr_err(DRV_NAME ": platform_driver_register failed: %d\n", ret);
		platform_device_unregister(softuart_pdev);
		return ret;
	}

	pr_info(DRV_NAME ": module loaded\n");
	return 0;
}

static void __exit suart_exit(void)
{
	platform_driver_unregister(&suart_driver);
	platform_device_unregister(softuart_pdev);
	pr_info(DRV_NAME ": module unloaded\n");
}

module_init(suart_init);
module_exit(suart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex + ChatGPT");
MODULE_DESCRIPTION("Loopback soft UART TTY driver (workqueue-based loopback)");
