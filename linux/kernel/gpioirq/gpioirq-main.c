/**************************************************************************
$Id$
***************************************************************************/

#include ".git.h"
#include "kdbg.h"

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>


struct jdi_panel {
	struct platform_device *pdev;

	struct gpio_desc *irq_gpio;
	struct gpio_desc *enable_gpio;
};

//-------------------------------------------------------------------------
static int dev_gpio_acquire(struct device *dev, struct gpio_desc **pgp , const char *name, enum gpiod_flags flags)
{
	int ret = 0;
	struct gpio_desc *gp;

	if(*pgp != NULL) {
		dev_notice(dev,"GPIO pin %s is already acquired: %d\n",name, desc_to_gpio(*pgp));
		return ret;
	}

	gp = devm_gpiod_get_optional(dev, name, flags);
	if (IS_ERR(gp)) {
		ret = PTR_ERR(gp);
		if (ret != -EPROBE_DEFER)
			dev_err(dev, "Failed to acquire %s gpio pin(%d)\n", name, ret);
	} else if(gp) {
		dev_notice(dev,"GPIO pin %s acquired: %d\n",name, desc_to_gpio(gp));
	} else {
		dev_warn(dev,"No %s gpio pin provided\n", name);
	}
	*pgp = gp;
	return ret;
}
//-------------------------------------------------------------------------

#ifdef CONFIG_OF
static const struct of_device_id __dt_ids[] = {
	{ .compatible = "alex," DEVICE },
	{},
};
MODULE_DEVICE_TABLE(of, __dt_ids);
#endif
//-------------------------------------------------------------------------
static int __probe(struct platform_device *pdev)  
{ 	
	struct device *dev = &pdev->dev;
	const struct of_device_id *of_id = of_match_device(__dt_ids, dev);
	struct device_node *np = dev->of_node;
	int ret = 0;
	const char *compatible = NULL;
	struct jdi_panel *jdi;
	
	of_property_read_string(np, "compatible", &compatible);
	dev_info(dev, "Device:[%s/%s] compat: [%s] (%s@%s:%d)\n",
		np->name, np->full_name, compatible,
		__func__, kbasename(__FILE__), __LINE__);
	
	if (!of_id /*|| !of_id->data*/) {
		dev_err(dev, "Device not found\n");
		return -ENODEV;
	}

	jdi = devm_kzalloc(dev, sizeof(*jdi), GFP_KERNEL);
	if (!jdi)
		return -ENOMEM;
	dev_set_drvdata(dev, jdi);
	jdi->pdev = pdev;

	dev_dbg(dev, "Probe gpio... (%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	ret = dev_gpio_acquire(dev, &jdi->irq_gpio, "irq", GPIOD_IN);
	if (ret)
		return ret;
	ret = dev_gpio_acquire(dev, &jdi->enable_gpio, "enable", GPIOD_OUT_HIGH);
	if (ret)
		return ret;
	dev_notice(dev, "Probe done (%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
	return ret; 
} 
//-------------------------------------------------------------------------
static int __remove(struct platform_device *pdev)
{ 
	struct device *dev = &pdev->dev;
	dev_notice(dev, "Removed (%s@%s:%d)\n",  __func__, kbasename(__FILE__), __LINE__);
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
};
//-------------------------------------------------------------------------
static int __init_module(void)
{
  INFO("Init done (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]");
  platform_driver_register(&__driver);
  return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
  platform_driver_unregister(&__driver);
  INFO("Unloaded (GIT Rev." GIT_REVISION ")  [Build: " __TIME__ " " __DATE__ "]" );
}
//-------------------------------------------------------------------------
module_init(__init_module);
module_exit(__exit_module);
MODULE_DESCRIPTION("Regulator test module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
