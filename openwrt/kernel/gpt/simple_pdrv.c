// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DRV_NAME	"simple_pdrv"

struct simple_priv {
	struct miscdevice	mdev;
	char			buf[128];
	size_t			len;
};

static struct platform_device *simple_dev;

/* ========================= sysfs attribute ========================= */

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

/* ========================= /dev read/write ========================= */

static ssize_t simple_read(struct file *f, char __user *ubuf,
			   size_t len, loff_t *off)
{
	struct simple_priv *priv = container_of(f->private_data,
						struct simple_priv, mdev);

	return simple_read_from_buffer(ubuf, len, off,
				       priv->buf, priv->len);
}

static ssize_t simple_write(struct file *f, const char __user *ubuf,
			    size_t len, loff_t *off)
{
	struct simple_priv *priv = container_of(f->private_data,
						struct simple_priv, mdev);

	len = min(len, sizeof(priv->buf) - 1);
	if (copy_from_user(priv->buf, ubuf, len))
		return -EFAULT;

	priv->buf[len] = '\0';
	priv->len = len;

	return len;
}

static const struct file_operations simple_fops = {
	.owner		= THIS_MODULE,
	.read		= simple_read,
	.write		= simple_write,
};

/* ========================= driver probe/remove ========================= */

static int simple_probe(struct platform_device *pdev)
{
	struct simple_priv *priv;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* create miscdevice (/dev/simple_pdrv) */
	priv->mdev.minor = MISC_DYNAMIC_MINOR;
	priv->mdev.name = DRV_NAME;
	priv->mdev.fops = &simple_fops;

	ret = misc_register(&priv->mdev);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, priv);

	/* create sysfs attribute */
	ret = device_create_file(&pdev->dev, &dev_attr_value);
	if (ret) {
		misc_deregister(&priv->mdev);
		return ret;
	}

	dev_info(&pdev->dev, "simple_pdrv probe OK\n");
	return 0;
}

static int simple_remove(struct platform_device *pdev)
{
	struct simple_priv *priv = platform_get_drvdata(pdev);

	device_remove_file(&pdev->dev, &dev_attr_value);
	misc_deregister(&priv->mdev);

	return 0;
}

static struct platform_driver simple_driver = {
	.driver = {
		.name = DRV_NAME,
	},
	.probe = simple_probe,
	.remove = simple_remove,
};

/* ========================= module init ========================= */

static int __init simple_init(void)
{
	int ret;

	/* create a DT-free platform_device */
	simple_dev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(simple_dev))
		return PTR_ERR(simple_dev);

	/* register driver */
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
MODULE_DESCRIPTION("Simple DT-free platform driver with sysfs & miscdevice");
