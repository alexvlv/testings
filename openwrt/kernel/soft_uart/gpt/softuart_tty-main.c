// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define DRV_NAME	"softuart"
#define DEV_NAME	"ttySU"

struct suart_dev {
	struct tty_driver	*tty_drv;
	struct tty_port		port;

	/* Workqueue for RX simulation */
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
	tty->driver_data = su; /* ensure driver_data is set */

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

	/* copy written data into RX buffer and schedule work */
	spin_lock(&su->rx_lock);
	if (count > sizeof(su->rx_buf))
		count = sizeof(su->rx_buf);
	memcpy(su->rx_buf, buf, count);
	su->rx_len = count;
	spin_unlock(&su->rx_lock);

	schedule_work(&su->rx_work);

	return count;
}

static const struct tty_operations suart_tty_ops = {
	.open	= suart_open,
	.close	= suart_close,
	.write	= suart_write,
};

static int suart_probe(struct platform_device *pdev)
{
	struct suart_dev *su;
	struct tty_driver *drv;
	int ret;

	su = devm_kzalloc(&pdev->dev, sizeof(*su), GFP_KERNEL);
	if (!su)
		return -ENOMEM;

	spin_lock_init(&su->rx_lock);
	INIT_WORK(&su->rx_work, suart_rx_work_fn);

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

	tty_port_init(&su->port);
	ret = tty_register_driver(drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		put_tty_driver(drv);
		return ret;
	}

	ret = tty_port_link_device(&su->port, drv, 0);
	if (ret) {
		pr_err(DRV_NAME ": tty_port_link_device failed: %d\n", ret);
		tty_unregister_driver(drv);
		put_tty_driver(drv);
		return ret;
	}

	platform_set_drvdata(pdev, su);

	pr_info(DRV_NAME ": probe OK, created /dev/%s0\n", DEV_NAME);

	return 0;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart_dev *su = platform_get_drvdata(pdev);

	cancel_work_sync(&su->rx_work);
	tty_unregister_driver(su->tty_drv);
	put_tty_driver(su->tty_drv);
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

module_platform_driver(suart_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Minimal Soft UART TTY driver (loopback) OpenWrt/Linux 6.6");
MODULE_AUTHOR("Alex + ChatGPT");
