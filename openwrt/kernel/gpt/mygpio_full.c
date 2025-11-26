                                                                                                                                    #include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "mygpio"

static int gpio_num = 4;
module_param(gpio_num, int, 0444);
MODULE_PARM_DESC(gpio_num, "GPIO number to monitor");

static int gpio_irq = -1;
static int gpio_val = 0;
static unsigned long last_jiffies = 0;
static unsigned int debounce_ms = 50;
module_param(debounce_ms, uint, 0444);
MODULE_PARM_DESC(debounce_ms, "Debounce time in milliseconds");

static struct cdev my_cdev;
static dev_t devt;
static struct platform_device *pdev;

/* IRQ handler with simple debounce */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    unsigned long now = jiffies;
    if (time_before(now, last_jiffies + msecs_to_jiffies(debounce_ms)))
        return IRQ_HANDLED; /* ignore if within debounce */

    last_jiffies = now;
    gpio_val = gpio_get_value(gpio_num);
    pr_info("mygpio_full: GPIO %d changed, value=%d\n", gpio_num, gpio_val);
    return IRQ_HANDLED;
}

/* Character device read */
static ssize_t mygpio_read(struct file *filp, char __user *buf,
                           size_t count, loff_t *offset)
{
    char tmp[4];
    int len;

    len = snprintf(tmp, sizeof(tmp), "%d\n", gpio_val);
    if (*offset >= len)
        return 0;

    if (count > len - *offset)
        count = len - *offset;

    if (copy_to_user(buf, tmp + *offset, count))
        return -EFAULT;

    *offset += count;
    return count;
}

static const struct file_operations mygpio_fops = {
    .owner = THIS_MODULE,
    .read  = mygpio_read,
};

/* Platform driver probe */
static int mygpio_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("mygpio_full: probe called for GPIO %d\n", gpio_num);

    ret = gpio_request(gpio_num, DEVICE_NAME);
    if (ret) {
        pr_err("mygpio_full: failed to request GPIO %d\n", gpio_num);
        return ret;
    }

    ret = gpio_direction_input(gpio_num);
    if (ret) {
        pr_err("mygpio_full: failed to set GPIO %d as input\n", gpio_num);
        gpio_free(gpio_num);
        return ret;
    }

    gpio_irq = gpio_to_irq(gpio_num);
    if (gpio_irq < 0) {
        pr_err("mygpio_full: failed to get IRQ for GPIO %d\n", gpio_num);
        gpio_free(gpio_num);
        return gpio_irq;
    }

    ret = request_irq(gpio_irq, gpio_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "mygpio_full_irq", NULL);
    if (ret) {
        pr_err("mygpio_full: failed to request IRQ %d\n", gpio_irq);
        gpio_free(gpio_num);
        return ret;
    }

    /* Create char device */
    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("mygpio_full: failed to alloc chrdev\n");
        free_irq(gpio_irq, NULL);
        gpio_free(gpio_num);
        return ret;
    }

    cdev_init(&my_cdev, &mygpio_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, devt, 1);
    if (ret) {
        pr_err("mygpio_full: cdev_add failed\n");
        unregister_chrdev_region(devt, 1);
        free_irq(gpio_irq, NULL);
        gpio_free(gpio_num);
        return ret;
    }

    pr_info("mygpio_full: setup complete, GPIO %d IRQ %d, /dev/%s\n",
            gpio_num, gpio_irq, DEVICE_NAME);

    return 0;
}

/* Platform driver remove */
static int mygpio_remove(struct platform_device *pdev)
{
    pr_info("mygpio_full: remove called\n");

    cdev_del(&my_cdev);
    unregister_chrdev_region(devt, 1);

    if (gpio_irq >= 0)
        free_irq(gpio_irq, NULL);

    gpio_free(gpio_num);

    return 0;
}

/* Platform driver */
static struct platform_driver mygpio_driver = {
    .probe  = mygpio_probe,
    .remove = mygpio_remove,
    .driver = {
        .name = "mygpio_full",
        .owner = THIS_MODULE,
    },
};

/* Module init/exit */
static int __init mygpio_init(void)
{
    int ret;

    /* register platform device */
    pdev = platform_device_register_simple("mygpio_full", -1, NULL, 0);
    if (IS_ERR(pdev))
        return PTR_ERR(pdev);

    /* register platform driver */
    ret = platform_driver_register(&mygpio_driver);
    if (ret) {
        platform_device_unregister(pdev);
        return ret;
    }

    pr_info("mygpio_full: module loaded\n");
    return 0;
}

static void __exit mygpio_exit(void)
{
    platform_driver_unregister(&mygpio_driver);
    platform_device_unregister(pdev);
    pr_info("mygpio_full: module unloaded\n");
}

module_init(mygpio_init);
module_exit(mygpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Full DTS-free GPIO platform driver with IRQ, char device, module parameter and debounce");
MODULE_ALIAS("platform:mygpio_full");
