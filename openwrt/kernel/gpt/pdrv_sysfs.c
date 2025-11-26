                                                                                                                                   #include <linux/module.h>
#include <linux/platform_device.h>

static int pdrv_probe(struct platform_device *pdev)
{
    pr_info("pdrv_sysfs: probe called for device %s\n", pdev->name);
    return 0;
}

static int pdrv_remove(struct platform_device *pdev)
{
    pr_info("pdrv_sysfs: remove called for device %s\n", pdev->name);
    return 0;
}

static struct platform_driver pdrv_sysfs_driver = {
    .probe  = pdrv_probe,
    .remove = pdrv_remove,
    .driver = {
        .name = "pdrv_sysfs",
        .owner = THIS_MODULE,
    },
};

module_platform_driver(pdrv_sysfs_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("DTS-free platform driver using sysfs probe");
MODULE_ALIAS("platform:pdrv_sysfs");

// echo pdrv_sysfs > /sys/bus/platform/drivers_probe
// pdrv_sysfs: probe called for device pdrv_sysfs
