// softuart_tty.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define DRV_NAME	"softuart"
#define TTY_NAME	"ttySU"
#define TTY_MINORS	1

struct suart {
	struct tty_driver	*tty_drv;
	struct tty_port		port;
	struct platform_device	*pdev;
};

static struct suart *g_suart;

/* ------------------------------------------------------------
 * TTY OPS
 * ------------------------------------------------------------ */

static int suart_install(struct tty_driver *drv, struct tty_struct *tty)
{
	struct suart *su = g_suart;
	int ret;

	ret = tty_standard_install(drv, tty);
	if (ret) {
		pr_err(DRV_NAME ": tty_standard_install failed: %d\n", ret);
		return ret;
	}

	/* attach tty_port */
	tty->port = &su->port;
	return 0;
}

static int suart_open(struct tty_struct *tty, struct file *file)
{
	struct suart *su = g_suart;
	int ret;

	ret = tty_port_open(&su->port, tty, file);
	if (ret)
		pr_err(DRV_NAME ": tty_port_open failed: %d\n", ret);

	return ret;
}

static void suart_close(struct tty_struct *tty, struct file *file)
{
	struct suart *su = g_suart;
	tty_port_close(&su->port, tty, file);
}

static int suart_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	int i;

	/* Echo all characters back */
	for (i = 0; i < count; i++)
		tty_insert_flip_char(&tty->port->state->port, buf[i], TTY_NORMAL);

	tty_flip_buffer_push(&tty->port->state->port);

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
	/* no return value */
}

static const struct tty_operations suart_tty_ops = {
	.install	= suart_install,
	.open		= suart_open,
	.close		= suart_close,
	.write		= suart_write,
	.write_room	= suart_write_room,
	.set_termios	= suart_set_termios,
};

/* ------------------------------------------------------------
 * Platform driver
 * ------------------------------------------------------------ */

static int suart_probe(struct platform_device *pdev)
{
	struct suart *su = g_suart;
	int ret;

	tty_port_init(&su->port);

	/* Allocate tty_driver */
	su->tty_drv = alloc_tty_driver(TTY_MINORS);
	if (!su->tty_drv) {
		pr_err(DRV_NAME ": alloc_tty_driver failed\n");
		ret = -ENOMEM;
		goto err_port_destroy;
	}

	su->tty_drv->driver_name	= DRV_NAME;
	su->tty_drv->name		= TTY_NAME;
	su->tty_drv->major		= 0;	/* dynamic */
	su->tty_drv->minor_start	= 0;
	su->tty_drv->type		= TTY_DRIVER_TYPE_SERIAL;
	su->tty_drv->subtype		= SERIAL_TYPE_NORMAL;
	su->tty_drv->flags		= TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	su->tty_drv->init_termios	= tty_std_termios;
	su->tty_drv->ops		= &suart_tty_ops;
	tty_set_operations(su->tty_drv, &suart_tty_ops);

	/* Register TTY driver */
	ret = tty_register_driver(su->tty_drv);
	if (ret) {
		pr_err(DRV_NAME ": tty_register_driver failed: %d\n", ret);
		goto err_put_drv;
	}

	/* Register device node /dev/ttySU0 */
	ret = tty_port_register_device(&su->port, su->tty_drv, 0, &pdev->dev);
	if (IS_ERR_VALUE(ret)) {
		pr_err(DRV_NAME ": tty_port_register_device failed: %d\n", ret);
		goto err_unreg_tty_driver;
	}

	su->pdev = pdev;
	platform_set_drvdata(pdev, su);

	return 0;

err_unreg_tty_driver:
	tty_unregister_driver(su->tty_drv);

err_put_drv:
	put_tty_driver(su->tty_drv);

err_port_destroy:
	tty_port_destroy(&su->port);

	return ret;
}

static int suart_remove(struct platform_device *pdev)
{
	struct suart *su = platform_get_drvdata(pdev);

	if (!su)
		return 0;

	device_destroy(su->tty_drv->tty_class, MKDEV(su->tty_drv->major, 0));

	tty_unregister_driver(su->tty_drv);

	put_tty_driver(su->tty_drv);

	tty_port_destroy(&su->port);

	return 0;
}

static struct platform_driver suart_platdrv = {
	.driver = {
		.name = DRV_NAME,
	},
	.probe  = suart_probe,
	.remove = suart_remove,
};

/* ------------------------------------------------------------
 * Module init/exit
 * ------------------------------------------------------------ */

static int __init suart_init(void)
{
	int ret;

	g_suart = kzalloc(sizeof(*g_suart), GFP_KERNEL);
	if (!g_suart) {
		pr_err(DRV_NAME ": kzalloc failed\n");
		return -ENOMEM;
	}

	/* Self-created platform device (DT-free) */
	g_suart->pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(g_suart->pdev)) {
		ret = PTR_ERR(g_suart->pdev);
		pr_err(DRV_NAME ": platform_device_register_simple failed: %d\n", ret);
		kfree(g_suart);
		return ret;
	}

	ret = platform_driver_register(&suart_platdrv);
	if (ret) {
		pr_err(DRV_NAME ": platform_driver_register failed: %d\n", ret);
		platform_device_unregister(g_suart->pdev);
		kfree(g_suart);
		return ret;
	}

	pr_info(DRV_NAME ": loaded\n");
	return 0;
}

static void __exit suart_exit(void)
{
	platform_driver_unregister(&suart_platdrv);

	if (g_suart->pdev)
		platform_device_unregister(g_suart->pdev);

	kfree(g_suart);

	pr_info(DRV_NAME ": unloaded\n");
}

module_init(suart_init);
module_exit(suart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Minimal software UART TTY driver (echo) — full error checking");
