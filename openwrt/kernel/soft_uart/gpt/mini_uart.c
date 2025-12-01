// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define DRV_NAME "mini_uart"
#define DEV_NAME "ttyMU"

struct muart_dev {
	struct tty_driver *tty_drv;
	struct tty_port port;
};

static int muart_open(struct tty_struct *tty, struct file *filp)
{
	struct muart_dev *mu = tty->driver_data;
	pr_info(DRV_NAME ": open su=%p\n", mu);
	return 0;
}

static void muart_close(struct tty_struct *tty, struct file *filp)
{
	struct muart_dev *mu = tty->driver_data;
	pr_info(DRV_NAME ": close su=%p\n", mu);
}

static ssize_t muart_write(struct tty_struct *tty,
			   const unsigned char *buf, size_t count)
{
	struct muart_dev *mu = tty->driver_data;
	pr_info(DRV_NAME ": write su=%p count=%zu\n", mu, count);

	/* Loopback → send written data to RX side */
	tty_insert_flip_string(&mu->port, buf, count);
	tty_flip_buffer_push(&mu->port);

	return count;
}

static const struct tty_operations muart_ops = {
	.open  = muart_open,
	.close = muart_close,
	.write = muart_write,
};

static struct platform_device *pdev;

static int muart_probe(struct platform_device *pdev_dev)
{
	struct muart_dev *mu;
	struct tty_driver *drv;
	struct device *ttydev;

	mu = devm_kzalloc(&pdev_dev->dev, sizeof(*mu), GFP_KERNEL);
	if (!mu)
		return -ENOMEM;

	drv = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(drv))
		return PTR_ERR(drv);

	drv->driver_name = DRV_NAME;
	drv->name        = DEV_NAME;
	drv->major       = 0;
	drv->minor_start = 0;
	drv->type        = TTY_DRIVER_TYPE_SERIAL;
	drv->init_termios = tty_std_termios;
	tty_set_operations(drv, &muart_ops);

	mu->tty_drv = drv;
	tty_port_init(&mu->port);

	ret = tty_register_driver(drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed %d\n", ret);
		return ret;
	}

	/* Proper tty_port_register_device usage */
	ttydev = tty_port_register_device(&mu->port, drv, 0, &pdev_dev->dev);
	if (IS_ERR(ttydev)) {
		pr_err(DRV_NAME ": tty_port_register_device failed %ld\n", PTR_ERR(ttydev));
		tty_unregister_driver(drv);
		return PTR_ERR(ttydev);
	}

	platform_set_drvdata(pdev_dev, mu);
	drv->driver_state = mu; /* link for tty->driver_data */

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);
	return 0;
}

static int muart_remove(struct platform_device *pdev_dev)
{
	struct muart_dev *mu = platform_get_drvdata(pdev_dev);

	tty_unregister_driver(mu->tty_drv);
	tty_port_destroy(&mu->port);
	pr_info(DRV_NAME ": removed\n");

	return 0;
}

static struct platform_driver muart_driver = {
	.driver = { .name = DRV_NAME },
	.probe  = muart_probe,
	.remove = muart_remove,
};

static int __init muart_init(void)
{
	int ret;
	pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&muart_driver);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	pr_info(DRV_NAME ": module loaded\n");
	return 0;
}

static void __exit muart_exit(void)
{
	platform_driver_unregister(&muart_driver);
	platform_device_unregister(pdev);
	pr_info(DRV_NAME ": module unloaded\n");
}

module_init(muart_init);
module_exit(muart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex + ChatGPT");
MODULE_DESCRIPTION("Minimal DT-free TTY driver example with proper tty_port_register_device");
