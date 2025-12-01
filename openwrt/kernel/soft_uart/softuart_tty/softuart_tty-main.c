// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#define DRV_NAME	"softuart"
#define DEV_NAME	"ttySU"

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;

	struct work_struct	rx_work;
	u8			rx_buf[256];
	size_t			rx_len;
	spinlock_t		rx_lock;
};

static void suart_rx_work_fn(struct work_struct *work)
{
	struct suart_dev *su = container_of(work, struct suart_dev, rx_work);

	spin_lock(&su->rx_lock);
	if (su->rx_len) {
		/* do NOT print here → prevents debug spam */
		tty_insert_flip_string(&su->port, su->rx_buf, su->rx_len);
		tty_flip_buffer_push(&su->port);
		su->rx_len = 0;
	}
	spin_unlock(&su->rx_lock);
}

static int suart_open(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": open: su=%p\n", su);
	tty->driver_data = su;

	return 0;
}

static void suart_close(struct tty_struct *tty, struct file *filp)
{
	struct suart_dev *su = tty->driver_data;

	pr_info(DRV_NAME ": close: su=%p\n", su);
}

static ssize_t suart_write(struct tty_struct *tty,
			   const unsigned char *buf, size_t count)
{
	struct suart_dev *su = tty->driver_data;

	if (!count)
		return 0;

	pr_info(DRV_NAME ": write: su=%p count=%zu\n", su, count);

	spin_lock(&su->rx_lock);
	if (count > sizeof(su->rx_buf))
		count = sizeof(su->rx_buf);
	memcpy(su->rx_buf, buf, count);
	su->rx_len = count;
	spin_unlock(&su->rx_lock);

	schedule_work(&su->rx_work);

	return count;
}

static ssize_t suart_read(struct tty_struct *tty, struct file *file,
			  unsigned char __user *buf, size_t count)
{
	pr_info(DRV_NAME ": read called, count=%zu\n", count);
	return 0;
}

static const struct tty_operations suart_tty_ops = {
	.open	= suart_open,
	.close	= suart_close,
	.write	= suart_write,
	.read	= suart_read,
};

static struct platform_device *pdev;

static int suart_probe(struct platform_device *pdev_dev)
{
	struct suart_dev *su;
	struct tty_driver *drv;
	int ret;

	su = devm_kzalloc(&pdev_dev->dev, sizeof(*su), GFP_KERNEL);
	if (!su)
		return -ENOMEM;

	spin_lock_init(&su->rx_lock);
	INIT_WORK(&su->rx_work, suart_rx_work_fn);

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
	tty_port_init(&su->port);

	ret = tty_register_driver(drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		return ret;
	}

	ret = tty_port_register_device(&su->port, drv, 0, &pdev_dev->dev);
	if (ret) {
		pr_err(DRV_NAME ": tty_port_register_device failed: %d\n", ret);
		tty_unregister_driver(drv);
		return ret;
	}

	platform_set_drvdata(pdev_dev, su);

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);
	return 0;
}

static int suart_remove(struct platform_device *pdev_dev)
{
	struct suart_dev *su = platform_get_drvdata(pdev_dev);

	cancel_work_sync(&su->rx_work);
	tty_unregister_driver(su->tty_drv);
	tty_port_destroy(&su->port);

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

static int __init suart_init(void)
{
	int ret;

	pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&suart_driver);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	pr_info(DRV_NAME ": module loaded\n");
	return 0;
}

static void __exit suart_exit(void)
{
	platform_driver_unregister(&suart_driver);
	platform_device_unregister(pdev);
	pr_info(DRV_NAME ": module unloaded\n");
}

module_init(suart_init);
module_exit(suart_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Minimal Soft UART TTY driver (loopback, safe debug)");
MODULE_AUTHOR("Alex + ChatGPT");
