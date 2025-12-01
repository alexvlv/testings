/**************************************************************************
$Id$
***************************************************************************/

#define DBG_LVL_DEFAULT 1

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
static int bitbang_tty_init(struct bitbang_data *bitbang_data)
{
	struct tty_driver *tty_drv;
	struct tty_port *tty_port;

	tty_drv = bitbang_data->tty_drv;

	tty_drv = alloc_tty_driver(1);
	tty_drv->owner = THIS_MODULE;
	tty_drv->driver_name = DRIVER_NAME;
	tty_drv->name = TTY_NAME;
	tty_drv->major = 0;
	tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	tty_drv->subtype = SERIAL_TYPE_NORMAL;
	tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_drv->init_termios = tty_std_termios;
	tty_set_operations(tty_drv, &gu_tty_ops);
	ret = tty_register_driver(tty_drv);
	if (ret) {
		ERROR("tty_register_driver failed!");
		return ret;
	}

	tty_port = bitbang_data->tty_port;
	tty_port_init(tty_port);

	dev = tty_register_device(GU->tty_drv, 0, NULL);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

}
//-------------------------------------------------------------------------
static int bitbang_uart_probe(struct platform_device *pdev)
{
	struct bitbang_data *d;
	
	d = devm_kzalloc(&pdev->dev, sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;
	d->pdev = pdev;
	platform_set_drvdata(pdev, d);	

	INFO("Probe done for %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static int bitbang_uart_remove(struct platform_device *pdev)
{
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
