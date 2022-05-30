/**************************************************************************
$Id$
***************************************************************************/

#define DEVICE  "tstdt"

#include ".git.h"
#include "kdbg.h"


#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>

static int __dbglvl = 2;

//-------------------------------------------------------------------------
#ifdef CONFIG_OF
static const struct of_device_id __dt_ids[] = {
	{ .compatible = "alex,test" },
	{},
};
MODULE_DEVICE_TABLE(of, __dt_ids);
#endif
//-------------------------------------------------------------------------
static int __probe(struct platform_device *pdev)  
{ 	
	//TRACE("[%s]",__PRETTY_FUNCTION__ );
	TRACE("" );
	return 0; 
} 
//-------------------------------------------------------------------------
static int __remove(struct platform_device *pdev)
{ 
	TRACE("Remove");
	return 0;
}
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
  INFO("Init done (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "]");
  platform_driver_register(&__driver);
  return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
  platform_driver_unregister(&__driver);
  INFO("Unloaded (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "]" );
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
/*

Use git/testings/linux/kernel/dt/test.dts
 
#include "imx8mm-evk.dts"

/ {
	test {
		compatible = "alex,test";
		reg = <0x4a064000 0x800>, <0x4a064800 0x200>, <0x4a064c00 0x200>;
		reg-names = "config", "ohci", "ehci";		
		foo = <3>;
		label = "NODE";
	};
};
*/
