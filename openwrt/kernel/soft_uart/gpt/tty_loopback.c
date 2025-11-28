// SPDX-License-Identifier: GPL-2.0
/*
 * tty_loopback.c - minimal software UART (loopback) TTY using modern tty API
 *
 * - Loopback: write() -> bytes are injected back to the TTY (readable by userspace)
 * - Uses tty_port_register_device() to create /dev/ttyGU0
 * - Supports termios via set_termios (driver records settings)
 * - DT-free: uses platform_device_register_simple in module_init
 *
 * Very small and self-contained: suitable as a starting point.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define DRV_NAME	"tty_loopback"
#define TTY_PREFIX	"ttyGU"
#define PORT_INDEX	0

struct loopback {
	struct tty_driver	*tty_drv;
	struct tty_port		tty_port;

	/* last termios settings (kept for info only) */
	unsigned int		baud;
	unsigned int		cflag;
};

static struct loopback *LB;

/* ---------- TTY operations ---------- */

static int lb_open(struct tty_struct *tty, struct file *file)
{
	return tty_port_open(&LB->tty_port, tty, file);
}

static void lb_close(struct tty_struct *tty, struct file *file)
{
	tty_port_close(&LB->tty_port, tty, file);
}

/* write: put data back to tty as if received (loopback) */
static int lb_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	int i;

	/* fast path: if no port, just return error */
	if (!tty_port_initialized(&LB->tty_port))
		return -EIO;

	/* insert each byte into TTY flip buffer (RX path) */
	for (i = 0; i < count; ++i) {
		tty_insert_flip_char(&LB->tty_port, buf[i], TTY_NORMAL);
	}
	tty_flip_buffer_push(&LB->tty_port);

	/* report bytes accepted (like real device) */
	LB->tty_port.counter += count; /* optional bookkeeping */
	return count;
}

static int lb_write_room(struct tty_struct *tty)
{
	/* trivial: always room */
	return 4096;
}

/* set_termios: accept termios from userspace; store baud and cflag */
static void lb_set_termios(struct tty_struct *tty, struct ktermios *old)
{
	unsigned int baud;

	baud = tty_get_baud_rate(tty);
	if (baud == 0)
		baud = LB->baud; /* keep existing if zero */

	LB->baud = baud;
	LB->cflag = tty->termios.c_cflag;

	/* nothing else to do for loopback */
}

/* tty operations struct */
static const struct tty_operations lb_tty_ops = {
	.open = lb_open,
	.close = lb_close,
	.write = lb_write,
	.write_room = lb_write_room,
	.set_termios = lb_set_termios,
};

/* ---------- platform probe/remove (do tty init/deinit here) ---------- */

static int lb_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	LB = kzalloc(sizeof(*LB), GFP_KERNEL);
	if (!LB)
		return -ENOMEM;

	/* default termios state */
	LB->baud = 9600;
	LB->cflag = 0;

	/* allocate tty driver for one device */
	LB->tty_drv = alloc_tty_driver(1);
	if (!LB->tty_drv) {
		ret = -ENOMEM;
		goto err_free;
	}

	LB->tty_drv->owner = THIS_MODULE;
	LB->tty_drv->driver_name = DRV_NAME;
	LB->tty_drv->name = TTY_PREFIX;
	LB->tty_drv->major = 0; /* dynamic */
	LB->tty_drv->type = TTY_DRIVER_TYPE_CONSOLE; /* not important */
	LB->tty_drv->subtype = 0;
	LB->tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	/* default termios */
	LB->tty_drv->init_termios = tty_std_termios;
	LB->tty_drv->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL;

	tty_set_operations(LB->tty_drv, &lb_tty_ops);

	ret = tty_register_driver(LB->tty_drv);
	if (ret)
		goto err_put_driver;

	tty_port_init(&LB->tty_port);

	/* create /dev/ttyGU0 and link port -> device in one call */
	ret = tty_port_register_device(&LB->tty_port, LB->tty_drv, PORT_INDEX, dev);
	if (ret)
		goto err_unregister;

	dev_info(dev, "loopback tty registered as /dev/%s%d\n", TTY_PREFIX, PORT_INDEX);
	return 0;

err_unregister:
	tty_unregister_driver(LB->tty_drv);
err_put_driver:
	put_tty_driver(LB->tty_drv);
err_free:
	kfree(LB);
	LB = NULL;
	return ret;
}

static int lb_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	if (!LB)
		return 0;

	/* unregister device & driver */
	tty_unregister_device(LB->tty_drv, PORT_INDEX);
	tty_unregister_driver(LB->tty_drv);
	put_tty_driver(LB->tty_drv);

	/* destroy port */
	tty_port_destroy(&LB->tty_port);

	dev_info(dev, "loopback tty unregistered\n");

	kfree(LB);
	LB = NULL;
	return 0;
}

static struct platform_driver lb_driver = {
	.probe = lb_probe,
	.remove = lb_remove,
	.driver = {
		.name = "tty_loopback_platform",
		.owner = THIS_MODULE,
	},
};

/* ---------- module init/exit: create a simple platform device and register driver ---------- */

static struct platform_device *lb_pdev;

static int __init lb_init(void)
{
	int ret;

	/* create a simple platform_device (DT-free) so probe() runs */
	lb_pdev = platform_device_register_simple("tty_loopback_platform", -1, NULL, 0);
	if (IS_ERR(lb_pdev))
		return PTR_ERR(lb_pdev);

	ret = platform_driver_register(&lb_driver);
	if (ret) {
		platform_device_unregister(lb_pdev);
		return ret;
	}

	pr_info("tty_loopback: module loaded\n");
	return 0;
}

static void __exit lb_exit(void)
{
	platform_driver_unregister(&lb_driver);
	platform_device_unregister(lb_pdev);
	pr_info("tty_loopback: module unloaded\n");
}

module_init(lb_init);
module_exit(lb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Minimal loopback TTY driver (DT-free platform device, termios supported)");
