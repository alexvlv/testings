                                                          // SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DRV_NAME	"simple_pdrv"

struct simple_priv {
	struct cdev cdev;
	dev_t devt;
	struct class *cls;
	struct device *dev;
	char buf[128];
	size_t len;
};

/* ================= sysfs attribute ================= */

static ssize_t value_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct simple_priv *priv = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%s\n", priv->buf);
}

static ssize_t value_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct simple_priv *priv = dev_get_drvdata(dev);

	priv->len = min(count, sizeof(priv->buf) - 1);
	memcpy(priv->buf, buf, priv->len);
	priv->buf[priv->len] = '\0';
	return count;
}

static DEVICE_ATTR_RW(value);

/* ================= read/write ================= */

static ssize_t simple_read(struct file *f, char __user *ubuf,
			   size_t count, loff_t *ppos)
{
	struct simple_priv *priv = f->private_data;
	return simple_read_from_buffer(ubuf, count, ppos, priv->buf, priv->len);
}

static ssize_t simple_write(struct file *f, const char __user *ubuf,
			    size_t count, loff_t *ppos)
{
	struct simple_priv *priv = f->private_data;
	ssize_t ret;

	ret = simple_write_to_buffer(priv->buf,
				     sizeof(priv->buf) - 1,
				     ppos,
				     ubuf, count);
	if (ret >= 0)
		priv->len = ret;
	priv->buf[priv->len] = '\0';
	return ret;
}

static const struct file_operations simple_fops = {
	.owner	= THIS_MODULE,
	.read	= simple_read,
	.write	= simple_write,
	.open	= simple_open,
	.release= simple_release,
};

/* ================== platform probe/remove ================== */

static int simple_probe(struct platform_device *pdev)
{
	struct simple_priv *priv;
	int ret;
	dev_t devt;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* allocate char device region */
	ret = alloc_chrdev_region(&devt, 0, 1, DRV_NAME);
	if (ret)
		return ret;

	priv->devt = devt;

	/* init cdev */
	cdev_init(&priv->cdev, &simple_fops);
	priv->cdev.owner = THIS_MODULE;
	ret = cdev_add(&priv->cdev, devt, 1);
	if (ret) {
		unregister_chrdev_region(devt, 1);
		return ret;
	}

	/* create class + device */
	priv->cls = class_create(THIS_MODULE, DRV_NAME);
	if (IS_ERR(priv->cls)) {
		cdev_del(&priv->cdev);
		unregister_chrdev_region(devt, 1);
		return PTR_ERR(priv->cls);
	}

	priv->dev = device_create(priv->cls, &pdev->dev, devt, NULL, DRV_NAME);
	if (IS_ERR(priv->dev)) {
		class_destroy(priv->cls);
		cdev_del(&priv->cdev);
		unregister_chrdev_region(devt, 1);
		return PTR_ERR(priv->dev);
	}

	/* create sysfs attribute */
	device_create_file(&pdev->dev, &dev_attr_value);

	platform_set_drvdata(pdev, priv);
	dev_info(&pdev->dev, "simple cdev probe OK\n");

	return 0;
}

static int simple_remove(struct platform_device *pdev)
{
	struct simple_priv *priv = platform_get_drvdata(pdev);

	device_remove_file(&pdev->dev, &dev_attr_value);
	device_destroy(priv->cls, priv->devt);
	class_destroy(priv->cls);
	cdev_del(&priv->cdev);
	unregister_chrdev_region(priv->devt, 1);

	return 0;
}

static struct platform_driver simple_driver = {
	.driver = {
		.name = DRV_NAME,
	},
	.probe = simple_probe,
	.remove = simple_remove,
};

/* ================= module init/exit ================= */

static struct platform_device *simple_dev;

static int __init simple_init(void)
{
	int ret;

	/* DT-free device */
	simple_dev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(simple_dev))
		return PTR_ERR(simple_dev);

	ret = platform_driver_register(&simple_driver);
	if (ret) {
		platform_device_unregister(simple_dev);
		return ret;
	}

	return 0;
}

static void __exit simple_exit(void)
{
	platform_driver_unregister(&simple_driver);
	platform_device_unregister(simple_dev);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Simple DT-free platform driver with cdev + sysfs");
