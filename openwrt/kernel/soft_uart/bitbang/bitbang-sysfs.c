/**************************************************************************
$Id$
***************************************************************************/

#include "kdbg.h"

#include "bitbang.h"
#include "uart.h"

#include <linux/platform_device.h>

//-------------------------------------------------------------------------
static ssize_t period_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct bitbang_priv *priv = dev_get_drvdata(dev);
	//return scnprintf(buf, PAGE_SIZE, "%d\n", priv->period_ms);
	return sysfs_emit(buf, "%u\n",priv->period_ms);
}

static ssize_t period_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct bitbang_priv *priv = dev_get_drvdata(dev);
	unsigned int v;
	int ret;

	ret = kstrtouint(buf, 10, &v);
	if (ret)
		return ret;   /* -EINVAL if parse failed */

	if (v < 1 || v > 1000)
		return -ERANGE;

	priv->period_ms = v;
	bitbang_uart_set_period(priv);

	return count;
}

static DEVICE_ATTR_RW(period);
//-------------------------------------------------------------------------
static ssize_t dbg_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%d\n",DBG_LVL);
}

static ssize_t dbg_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	unsigned int v;
	int ret;

	ret = kstrtouint(buf, 10, &v);
	if (ret)
		return ret;   /* -EINVAL if parse failed */

	if (v > 10)
		return -ERANGE;

	DBG_LVL = v;
	INFO("Debug level set to %d", DBG_LVL);

	return count;
}

static DEVICE_ATTR_RW(dbg);
//-------------------------------------------------------------------------
int bitbang_sysfs_init(struct platform_device *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_period);
	if(ret) {
		ERROR("sysfs create attrib \"period\" failed");
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_dbg);
	if(ret) {
		ERROR("sysfs create attrib \"dbg\" failed");
		goto err_remove_period;
	}

	return ret;

err_remove_period:
	device_remove_file(&pdev->dev, &dev_attr_period);
	return ret;
}
//-------------------------------------------------------------------------
void bitbang_sysfs_exit(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_period);
}
//-------------------------------------------------------------------------
