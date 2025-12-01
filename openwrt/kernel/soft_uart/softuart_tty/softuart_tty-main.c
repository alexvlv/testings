// SPDX-License-Identifier: GPL-2.0
/*
 * softuart_tty-main.c
 *
 * Minimal loopback TTY driver (OpenWrt / kernel 6.6 compatible).
 * - DT-free: creates a simple platform_device in module init so probe() runs.
 * - Creates /dev/ttySU0.
 * - Loopback: writes are immediately injected into the RX flip buffer.
 * - Preserves pointer debug prints in open/close.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/tty_port.h>
#include <linux/slab.h>

#define DRV_NAME	"softuart"
#define DEV_NAME	"ttySU"
#define TTY_MINORS	1

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;
	/* future: add ring buffers / timers here */
};

static struct platform_device *softuart_pdev;

/* ----------------------------- TTY ops --------------------------------- */

static int suart_open(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": open: %p\n", su);

	/* nothing special to start for loopback */
	return 0;
}

static void suart_close(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": close: %p\n", su);
}

/* write: loopback into RX path */
static ssize_t suart_write(struct tty_struct *tty, const unsigned char *buf,
			   size_t count)
{
	struct suart_dev *su = tty->driver_data;
	struct tty_port *port = tty->port;

	pr_info(DRV_NAME ": write: su=%p count=%zu\n", su, count);

	if (!port) {
		pr_warn(DRV_NAME ": write: tty->port is NULL\n");
		return -EIO;
	}

	/* insert into flip buffer (RX) and push */
	tty_insert_flip_string(port, buf, count);
	tty_flip_buffer_push(port);

	return count;
}

static unsigned int suart_write_room(struct tty_struct *tty)
{
	/* always room in this simple loopback */
	return 4096;
}

static void suart_set_termios(struct tty_struct *tty, const struct ktermios *old)
{
	/* minimal: accept whatever user sets; no hardware to configure */
}

/* operations table */
static const struct tty_operations suart_tty_ops = {
	.open		= suart_open,
	.close		= suart_close,
	.write		= suart_write,
	.write_room	= suart_write_room,
	.set_termios	= suart_set_termios,
};

/* ----------------------------- platform probe/remove ------------------ */

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

	/* allocate tty driver */
	drv = tty_alloc_driver(TTY_MINORS,
			       TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(drv)) {
		ret = PTR_ERR(drv);
		pr_err(DRV_NAME ": tty_alloc_driver failed: %d\n", ret);
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

	/* register driver */
	ret = tty_register_driver(drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		put_tty_driver(drv); /* safe: put_tty_driver may exist on some kernels; if not, build will warn */
		return ret;
	}

	/* initialize port and bind */
	tty_port_init(&su->port);

	/* set driver state so tty->driver_data will be populated by tty core */
	drv->driver_state = su;

	/* link port to driver ports[] for index 0 */
	drv->ports[0] = &su->port;

	/* create /sys/class/tty/ttySU0 and trigger udev to create /dev/ttySU0 */
	/* use parent = NULL to make sysfs entry under /sys/class/tty */
	tty_dev = tty_register_device(drv, 0, NULL);
	if (IS_ERR(tty_dev)) {
		ret = PTR_ERR(tty_dev);
		pr_err(DRV_NAME ": tty_register_device failed: %d\n", ret);
		tty_unregister_driver(drv);
		return ret;
	}

	/* save pointers */
	su->tty_drv = drv;
	platform_set_drvdata(pdev, su);

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);
	return 0;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart_dev *su = platform_get_drvdata(pdev);

	pr_info(DRV_NAME ": remove\n");

	/* unregister device & driver */
	tty_unregister_device(su->tty_drv, 0);
	tty_unregister_driver(su->tty_drv);

	/* destroy port */
	tty_port_destroy(&su->port);

	return 0;
}

/* ----------------------------- module init/exit ---------------------- */

/* We'll create a simple platform device so probe() runs without DT */
static struct platform_driver suart_driver = {
	.probe	= suart_probe,
	.remove	= suart_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init suart_init(void)
{
	int ret;

	/* create platform device (DT-free) */
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
MODULE_DESCRIPTION("Minimal loopback soft UART TTY driver (OpenWrt 6.6 compatible)");
