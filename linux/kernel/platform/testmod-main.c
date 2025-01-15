/**************************************************************************
$Id$
***************************************************************************/

#include ".git.h"

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#ifndef xstr
#define xstr(s)   str(s)
#define str(s)	  #s
#endif

#ifdef DEVNAME
#undef DEVICE
#define DEVICE   xstr(DEVNAME)
#endif

//-------------------------------------------------------------------------
static int __probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	dev_dbg(dev, "DBG msg(%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	dev_notice(dev, "Probed (%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	return 0;
}
//-------------------------------------------------------------------------
static int __remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	dev_dbg(dev, "DBG msg(%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	dev_notice(dev, "Removed (%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	return 0;
}
//-------------------------------------------------------------------------
#ifdef CONFIG_OF
static const struct of_device_id __dt_ids[] = {
	{ .compatible = DEVICE "," DEVICE },
	{},
};
MODULE_DEVICE_TABLE(of, __dt_ids);
#endif
//-------------------------------------------------------------------------
/* work with hotplug and coldplug */
MODULE_ALIAS("platform:" DEVICE);
static struct platform_driver __driver = {
	.driver = {
		.name =		DEVICE,
		.owner =	THIS_MODULE,
#ifdef CONFIG_OF
	.of_match_table = __dt_ids,
#endif
	},
	.probe  = __probe,
	.remove = __remove,
	/* REVISIT add suspend() and resume() methods */
	//.remove =	__exit_p(host_remove),
};
//-------------------------------------------------------------------------
static int __init_module(void)
{
	platform_driver_register(&__driver);
	printk(KERN_INFO "Module loaded (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]\n");
	return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
	platform_driver_unregister(&__driver);
	printk(KERN_INFO "Module unloaded (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]\n" );
}
//-------------------------------------------------------------------------
module_init(__init_module);
module_exit(__exit_module);
MODULE_DESCRIPTION("Test module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
//-------------------------------------------------------------------------
