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

struct uart_data {
	struct platform_device *pdev;   // back-reference
};

DEBUG_PARAM_DEF();

struct platform_device *uart_pdev = NULL;

//-------------------------------------------------------------------------
static int uart_probe(struct platform_device *pdev)
{
	struct uart_data *d;
	
	d = devm_kzalloc(&pdev->dev, sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;
	d->pdev = pdev;
	platform_set_drvdata(pdev, d);	

	INFO("Probe done for %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static int uart_remove(struct platform_device *pdev)
{
	INFO("Removed %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static struct platform_driver uart_driver = {
	.probe  = uart_probe,
	.remove = uart_remove,
	.driver = {
		.name = "soft_uart",
		.owner = THIS_MODULE,
	},
};
//-------------------------------------------------------------------------
static int __init_module(void)
{
	int ret;
	struct platform_device	*pdev;

	INFO("Loading... (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]");

	pdev = platform_device_register_simple("soft_uart", -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ERROR("Platform device register failed!");
		return PTR_ERR(pdev);
	}

	ret = platform_driver_register(&uart_driver);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	uart_pdev = pdev;
	INFO("Loaded %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
	//ret = driver_for_each_device(&uart_driver.driver, NULL, NULL, list_pdev);
	platform_driver_unregister(&uart_driver);
	platform_device_unregister(uart_pdev);
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
