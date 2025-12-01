// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define DRV_NAME	"softuart"
#define DEV_NAME	"ttySU"

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;
	/* your HW/soft state here */
};

static int suart_open(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": open: su=%p\n", su);

	/* Nothing to do, but could start timers, enable HW, etc */
	return 0;
}

static void suart_close(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": close: su=%p\n", su);

	/* stop soft/hw if needed */
}

static ssize_t suart_write(struct tty_struct *tty, const u8 *buf, size_t count)
{
	struct suart_dev *su = tty->driver_data;
	struct tty_port *port = tty->port;

	pr_info(DRV_NAME ": write: su=%p count=%zu\n", su, count);

	/* Loopback → send written data to RX side */
	tty_insert_flip_string(port, buf, count);
	tty_flip_buffer_push(port);

	return count;
}

static const struct tty_operations suart_tty_ops = {
	.open		= suart_open,
	.close		= suart_close,
	.write		= suart_write,
};

static int suart_probe(struct platform_device *pdev)
{
	struct suart_dev *su;
	struct tty_driver *drv;

	su = devm_kzalloc(&pdev->dev, sizeof(*su), GFP_KERNEL);
	if (!su)
		return -ENOMEM;

	/* Allocate TTY driver */
	drv = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(drv))
		return PTR_ERR(drv);

	drv->driver_name	= DRV_NAME;
	drv->name		= DEV_NAME;
	drv->major		= 0;
	drv->minor_start	= 0;
	drv->type		= TTY_DRIVER_TYPE_SERIAL;
	drv->init_termios	= tty_std_termios;
	drv->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;

	tty_set_operations(drv, &suart_tty_ops);

	su->tty_drv = drv;

	/* init port */
	tty_port_init(&su->port);

	/* bind port to tty driver */
	tty_port_link_device(&su->port, drv, 0);

	/* store su in tty / retrieved later in open/write */
	drv->driver_state = su;

	platform_set_drvdata(pdev, su);

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);

	return 0;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart_dev *su = platform_get_drvdata(pdev);

	tty_port_destroy(&su->port);
	tty_unregister_driver(su->tty_drv);
	put_tty_driver(su->tty_drv);

	pr_info(DRV_NAME ": removed\n");

	return 0;
}

static struct platform_driver suart_driver = {
	.driver = {
		.name = DRV_NAME,
	},
	.probe	= suart_probe,
	.remove	= suart_remove,
};

module_platform_driver(suart_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Soft UART TTY driver (loopback)");
MODULE_AUTHOR("Alex + ChatGPT");
