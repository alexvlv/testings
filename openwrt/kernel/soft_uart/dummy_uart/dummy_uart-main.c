/**************************************************************************
$Id$
***************************************************************************/

#define DBG_LVL_DEFAULT 1

#define N_PORTS 1
#define TTY_NAME	"ttyEMU"

#include ".git.h"
#include "kdbg.h"

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>

struct bitbang_data {
	struct platform_device *pdev;   // back-reference
	struct tty_driver *tty_drv;
	struct tty_port tty_port;
};

DEBUG_PARAM_DEF();

struct platform_device *bitbang_uart_pdev = NULL;

//-------------------------------------------------------------------------
static int bitbang_open(struct tty_struct *tty, struct file *filp)
{
	struct bitbang_data *d = tty->driver_data;
	DBG("open d=%p", d);
	return 0;
}
//-------------------------------------------------------------------------
static void bitbang_close(struct tty_struct *tty, struct file *filp)
{
	struct bitbang_data *d = tty->driver_data;
	DBG("close d=%p", d);
}
//-------------------------------------------------------------------------
static ssize_t bitbang_write(struct tty_struct *tty, const unsigned char *buf, size_t count)
{
	struct bitbang_data *d = tty->driver_data;
	DBG("write d=%p count=%zu", d, count);

	/* Loopback → send written data to RX side */
	//tty_insert_flip_string(&mu->port, buf, count);
	//tty_flip_buffer_push(&mu->port);

	return count;
}
//-------------------------------------------------------------------------
static const struct tty_operations bitbang_tty_ops = {
	.open  = bitbang_open,
	.close = bitbang_close,
	.write = bitbang_write,
};
//-------------------------------------------------------------------------
static int bitbang_tty_init(struct bitbang_data *data)
{
	int ret = 0;
	struct tty_driver *tty_drv;
	struct tty_port *tty_port;
	struct device *ttydev;
	struct platform_device *pdev = data->pdev;

	tty_drv = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(tty_drv)) {
		printk(KERN_ALERT "soft_uart: Failed to allocate the driver.\n");
		return -ENOMEM;
	}

	tty_drv->owner = THIS_MODULE;
	tty_drv->driver_name = DEVICE;
	tty_drv->name = TTY_NAME;
	tty_drv->major = 0;
	tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	tty_drv->subtype = SERIAL_TYPE_NORMAL;
	tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_drv->init_termios = tty_std_termios;
	tty_drv->init_termios.c_ispeed = 4800;
	tty_drv->init_termios.c_ospeed = 4800;
	tty_drv->init_termios.c_cflag  = B4800 | CREAD | CS8 | CLOCAL;
	
	tty_set_operations(tty_drv, &bitbang_tty_ops);
	data->tty_drv = tty_drv;

	tty_port = &data->tty_port;
	tty_port_init(tty_port);

	ret = tty_register_driver(tty_drv);
	if (ret) {
		ERROR("tty_register_driver failed: %d!", ret);
		return ret;
	}

	ttydev = tty_port_register_device(tty_port, tty_drv, 0, &pdev->dev);
	if (IS_ERR(ttydev)) {
		ERROR("tty_port_register_device failed: %ld", PTR_ERR(ttydev));
		tty_unregister_driver(tty_drv);
		return PTR_ERR(ttydev);
	}
	return ret;
}
//-------------------------------------------------------------------------
static int bitbang_uart_probe(struct platform_device *pdev)
{
	struct bitbang_data *d;
	int ret; 
	d = devm_kzalloc(&pdev->dev, sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;
	d->pdev = pdev;
	platform_set_drvdata(pdev, d);

	ret = bitbang_tty_init(d);
	if(ret != 0) {
		return -ENOMEM;
	}

	INFO("Probe done for %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static int bitbang_uart_remove(struct platform_device *pdev)
{
	struct bitbang_data *d = platform_get_drvdata(pdev);
	tty_unregister_driver(d->tty_drv);
	tty_port_destroy(&d->tty_port);
	INFO("Removed %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static struct platform_driver bitbang_uart_driver = {
	.probe  = bitbang_uart_probe,
	.remove = bitbang_uart_remove,
	.driver = {
		.name = DEVICE,
		.owner = THIS_MODULE,
	},
};
//-------------------------------------------------------------------------
static int __init_module(void)
{
	int ret;
	struct platform_device	*pdev;

	INFO("Loading... (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]");

	pdev = platform_device_register_simple(DEVICE, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ERROR("Platform device register failed!");
		return PTR_ERR(pdev);
	}

	ret = platform_driver_register(&bitbang_uart_driver);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	bitbang_uart_pdev = pdev;
	INFO("Loaded %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
	//ret = driver_for_each_device(&uart_driver.driver, NULL, NULL, list_pdev);
	platform_driver_unregister(&bitbang_uart_driver);
	platform_device_unregister(bitbang_uart_pdev);
	INFO("Unloaded (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]" );
}
//-------------------------------------------------------------------------
module_init(__init_module);
module_exit(__exit_module);
MODULE_DESCRIPTION("Dummy uart module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
