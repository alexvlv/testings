/**************************************************************************
$Id$
***************************************************************************/

#define TX_GPIO 530
#define RX_GPIO 532
#define PERIOD_MS 10

#include ".git.h"
#include "kdbg.h"

#include "bitbang.h"
#include "uart.h"

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>


DEBUG_PARAM_DEF();

static int tx_gpio = TX_GPIO;
module_param_named(txp, tx_gpio, int, S_IRUGO|S_IWUSR); \
MODULE_PARM_DESC(txp, "TX gpio pin (530 default)");

static int rx_gpio = RX_GPIO;
module_param_named(rxp, rx_gpio, int, S_IRUGO|S_IWUSR); \
MODULE_PARM_DESC(rxp, "RX gpio pin (532 default)");

static uint period_ms = PERIOD_MS;
module_param_named(period, period_ms, uint, S_IRUGO|S_IWUSR); \
MODULE_PARM_DESC(period, "Period: msec per bit");

struct platform_device *bitbang_uart_pdev = NULL;

//-------------------------------------------------------------------------
static ssize_t bitbang_read(struct file *f, char __user *ubuf,
			   size_t count, loff_t *ppos)
{
	struct bitbang_priv *priv = f->private_data;

	if (bitbang_uart_rx_buffer_empty(priv) && f->f_flags & O_NONBLOCK)
		return -EAGAIN;
	return bitbang_rx(priv, ubuf, count);
	//DBG("Read %d bytes, ready %d", count, priv->len);
	//return simple_read_from_buffer(ubuf, count, ppos,
	//			       priv->buf, priv->len);
}
//-------------------------------------------------------------------------
static ssize_t bitbang_write(struct file *f, const char __user *ubuf,
			    size_t count, loff_t *ppos)
{
	struct bitbang_priv *priv = f->private_data;
	ssize_t ret;

	if (bitbang_uart_tx_buffer_full(priv) && f->f_flags & O_NONBLOCK)
		return -EAGAIN;
	
	ret = bitbang_tx(priv, ubuf, count);
	// ret = simple_write_to_buffer(priv->buf,
	// 			     sizeof(priv->buf)-1,
	// 			     ppos,
	// 			     ubuf, count);
	// if (ret >= 0)
	// 	priv->len = ret;
	// priv->buf[priv->len] = '\0';

	return ret;
}
//-------------------------------------------------------------------------
static int bitbang_open(struct inode *inode, struct file *filp)
{
	struct bitbang_priv *priv = container_of(inode->i_cdev, struct bitbang_priv, cdev);
	struct task_struct *t = current;
	filp->private_data = priv;
	DBG("Open by %d/%d (%s)", t->tgid, t->pid, t->comm);
	return bitbang_uart_open(priv);
}
//-------------------------------------------------------------------------
static int bitbang_release(struct inode *inode, struct file *filp)
{
	struct bitbang_priv *priv = filp->private_data;
	struct task_struct *t = current;
	DBG("Close by %d/%d (%s)", t->tgid, t->pid, t->comm);
	if(priv)
		bitbang_uart_close(priv);
	else
		WARNING("Close: no private data!(%p)", priv);
	return 0;
}
//-------------------------------------------------------------------------
static const struct file_operations bitbang_fops = {
	.owner	= THIS_MODULE,
	.read	= bitbang_read,
	.write	= bitbang_write,
	.open	= bitbang_open,
	.release= bitbang_release,
};
//-------------------------------------------------------------------------
static int bitbang_probe(struct platform_device *pdev)
{
	struct bitbang_priv *priv;
	dev_t devt;
	int ret = 0;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	//DBG("Init: %p", priv);
	if (!priv)
		return -ENOMEM;

	priv->tx_gpio = tx_gpio;
	priv->rx_gpio = rx_gpio;
	
	if(period_ms == 0) period_ms = PERIOD_MS;
	priv->period_ms = period_ms;

	ret = alloc_chrdev_region(&devt, 0, 1, DEVICE);
	if (ret) {
		ERROR("alloc_chrdev_region failed!");
		return ret;
	}
	priv->devt = devt;

	cdev_init(&priv->cdev, &bitbang_fops);
	priv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&priv->cdev, devt, 1);
	if (ret) {
		ERROR("alloc_chrdev_region failed!");
		goto err_unregister_chrdev;
	}

	priv->cls = class_create(DEVICE);
	if (IS_ERR(priv->cls)) {
		ERROR("class_create failed!");
		ret = PTR_ERR(priv->cls);
		goto err_cdev_del;
	}

	priv->dev = device_create(priv->cls, &pdev->dev, devt, NULL, DEVICE);
	if (IS_ERR(priv->dev)) {
		ERROR("device_create failed!");
		ret = PTR_ERR(priv->dev);
		goto err_class_destroy;
	}

	ret = bitbang_sysfs_init(pdev);
	if (ret)
		goto err_device_destroy;

	bitbang_proc_init(priv);

	priv->pdev = pdev;
	platform_set_drvdata(pdev, priv);

	ret =  bitbang_uart_init(priv);
	if (ret) {
		goto err_sysfs_remove;
	}

	INFO("Probe done for %s", dev_name(&pdev->dev));
	return ret;

err_sysfs_remove:
	bitbang_proc_exit(priv);
	bitbang_sysfs_exit(pdev);
err_device_destroy:
	device_destroy(priv->cls, devt);
err_class_destroy:
	class_destroy(priv->cls);
err_cdev_del:
	cdev_del(&priv->cdev);
err_unregister_chrdev:
	unregister_chrdev_region(devt, 1);
	return ret;
}
//-------------------------------------------------------------------------
static int bitbang_remove(struct platform_device *pdev)
{
	struct bitbang_priv *priv = platform_get_drvdata(pdev);

	bitbang_uart_exit(priv);
	bitbang_proc_exit(priv);
	bitbang_sysfs_exit(pdev);
	device_destroy(priv->cls, priv->devt);
	class_destroy(priv->cls);
	cdev_del(&priv->cdev);
	unregister_chrdev_region(priv->devt, 1);

	INFO("Removed %s", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static struct platform_driver bitbang_driver = {
	.probe  = bitbang_probe,
	.remove = bitbang_remove,
	.driver = {
		.name = DEVICE,
		.owner = THIS_MODULE,
	},
};
//-------------------------------------------------------------------------
static int mod_init(void)
{
	int ret;
	struct platform_device	*pdev;

	DBG("Loading, debug level: %d", DBG_LVL);

	pdev = platform_device_register_simple(DEVICE, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ERROR("Platform device register failed!");
		return PTR_ERR(pdev);
	}

	ret = platform_driver_register(&bitbang_driver);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	bitbang_uart_pdev = pdev;
	INFO("Loaded %s, GIT Rev." GIT_REVISION " [Build: " __TIME__ " " __DATE__ "]", dev_name(&pdev->dev));
	return 0;
}
//-------------------------------------------------------------------------
static void mod_exit(void)
{
	platform_driver_unregister(&bitbang_driver);
	platform_device_unregister(bitbang_uart_pdev);
	INFO("Unloaded (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]" );
}
//-------------------------------------------------------------------------
module_init(mod_init);
module_exit(mod_exit);
MODULE_DESCRIPTION("Bitbang serial/uart driver module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
