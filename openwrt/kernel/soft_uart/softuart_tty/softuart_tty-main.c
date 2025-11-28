// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>            
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/tty_port.h>
#include <linux/slab.h>

#define DRV_NAME	"softuart"
#define TTY_NAME	"ttySU"
#define TTY_MINORS	1

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;
};

static struct platform_device *suart_pdev;

/* ------------------------------------------------------------------ */
/* TTY operations                                                     */
/* ------------------------------------------------------------------ */

static int suart_open(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = platform_get_drvdata(suart_pdev);
	int ret;

	tty->driver_data = su;
	pr_info(DRV_NAME ": open: %p\n", su);

	/* Properly link tty_struct and tty_port */
	ret = tty_port_open(&su->port, tty, filp);
	if (ret)
		return ret;

	tty->port = &su->port;

	return 0;
}

static void suart_close(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": close: %p\n", su);
	tty_port_close(&su->port, tty, filp);
}

/* Loopback write: everything written is read back */
static ssize_t suart_write(struct tty_struct *tty,
			   const unsigned char *buf, size_t count)
{
	/* Use tty_struct, not &port */
	tty_insert_flip_string(tty, buf, count);
	tty_flip_buffer_push(tty);
	return count;
}

static unsigned int suart_write_room(struct tty_struct *tty)
{
	return 4096;
}

static void suart_set_termios(struct tty_struct *tty,
			      const struct ktermios *old)
{
	/* minimal stub */
}

/* TTY operations table */
static const struct tty_operations suart_tty_ops = {
	.open		= suart_open,
	.close		= suart_close,
	.write		= suart_write,
	.write_room	= suart_write_room,
	.set_termios	= suart_set_termios,
};

/* ------------------------------------------------------------------ */
/* platform driver                                                    */
/* ------------------------------------------------------------------ */

static int suart_probe(struct platform_device *pdev)
{
	struct suart_dev *su;
	struct device *tty_dev;
	int ret;

	pr_info(DRV_NAME ": probe\n");

	su = devm_kzalloc(&pdev->dev, sizeof(*su), GFP_KERNEL);
	if (!su)
		return -ENOMEM;

	tty_port_init(&su->port);

	/* Allocate tty_driver */
	su->tty_drv = tty_alloc_driver(TTY_MINORS,
				       TTY_DRIVER_REAL_RAW |
				       TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(su->tty_drv)) {
		ret = PTR_ERR(su->tty_drv);
		pr_err(DRV_NAME ": tty_alloc_driver failed: %d\n", ret);
		return ret;
	}

	su->tty_drv->driver_name	= DRV_NAME;
	su->tty_drv->name		= TTY_NAME;
	su->tty_drv->major		= 0;
	su->tty_drv->minor_start	= 0;

	su->tty_drv->type		= TTY_DRIVER_TYPE_SERIAL;
	su->tty_drv->subtype		= SERIAL_TYPE_NORMAL;
	su->tty_drv->flags		= TTY_DRIVER_REAL_RAW |
					  TTY_DRIVER_DYNAMIC_DEV;

	su->tty_drv->init_termios	= tty_std_termios;
	su->tty_drv->init_termios.c_cflag =
		B115200 | CS8 | CREAD | HUPCL | CLOCAL;

	tty_set_operations(su->tty_drv, &suart_tty_ops);

	ret = tty_register_driver(su->tty_drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, su);
	su->tty_drv->ports[0] = &su->port;

	/* create /dev/ttySU0 */
	tty_dev = tty_register_device(su->tty_drv, 0, &pdev->dev);
	if (IS_ERR(tty_dev)) {
		ret = PTR_ERR(tty_dev);
		pr_err(DRV_NAME ": tty_register_device failed: %d\n", ret);
		tty_unregister_driver(su->tty_drv);
		return ret;
	}

	dev_info(&pdev->dev, "soft UART registered as /dev/%s0\n", TTY_NAME);

	return 0;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart_dev *su = platform_get_drvdata(pdev);

	pr_info(DRV_NAME ": remove\n");

	tty_unregister_device(su->tty_drv, 0);
	tty_unregister_driver(su->tty_drv);

	tty_port_destroy(&su->port);

	return 0;
}

static struct platform_driver suart_driver = {
	.probe	= suart_probe,
	.remove	= suart_remove,
	.driver = {
		.name = DRV_NAME,
	},
};

/* ------------------------------------------------------------------ */
/* module init/exit                                                   */
/* ------------------------------------------------------------------ */

static int __init suart_init(void)
{
	int ret;

	suart_pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(suart_pdev)) {
		ret = PTR_ERR(suart_pdev);
		pr_err(DRV_NAME ": platform_device_register_simple failed: %d\n", ret);
		return ret;
	}

	ret = platform_driver_register(&suart_driver);
	if (ret) {
		pr_err(DRV_NAME ": platform_driver_register failed: %d\n", ret);
		platform_device_unregister(suart_pdev);
		return ret;
	}

	pr_info(DRV_NAME ": module loaded\n");
	return 0;
}

static void __exit suart_exit(void)
{
	platform_driver_unregister(&suart_driver);
	platform_device_unregister(suart_pdev);
	pr_info(DRV_NAME ": module unloaded\n");
}

module_init(suart_init);
module_exit(suart_exit);

MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Minimal loopback software UART for OpenWrt 6.6 kernel with /dev node");
MODULE_LICENSE("GPL");
