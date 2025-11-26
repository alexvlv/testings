                                                                                                                                    #include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#define GPIO_NUMBER 4   // Change to your GPIO
#define GPIO_DESC   "gpio_input_sysfs"

static int gpio_irq = -1;

/* IRQ handler */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    int value = gpio_get_value(GPIO_NUMBER);
    pr_info("pdrv_gpio_sysfs: GPIO %d changed, value=%d\n", GPIO_NUMBER, value);
    return IRQ_HANDLED;
}

/* Platform driver probe */
static int pdrv_gpio_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("pdrv_gpio_sysfs: probe called\n");

    ret = gpio_request(GPIO_NUMBER, GPIO_DESC);
    if (ret) {
        pr_err("pdrv_gpio_sysfs: failed to request GPIO %d\n", GPIO_NUMBER);
        return ret;
    }

    ret = gpio_direction_input(GPIO_NUMBER);
    if (ret) {
        pr_err("pdrv_gpio_sysfs: failed to set GPIO %d as input\n", GPIO_NUMBER);
        gpio_free(GPIO_NUMBER);
        return ret;
    }

    gpio_irq = gpio_to_irq(GPIO_NUMBER);
    if (gpio_irq < 0) {
        pr_err("pdrv_gpio_sysfs: failed to get IRQ for GPIO %d\n", GPIO_NUMBER);
        gpio_free(GPIO_NUMBER);
        return gpio_irq;
    }

    ret = request_irq(gpio_irq, gpio_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "pdrv_gpio_sysfs_irq", NULL);
    if (ret) {
        pr_err("pdrv_gpio_sysfs: failed to request IRQ %d\n", gpio_irq);
        gpio_free(GPIO_NUMBER);
        return ret;
    }

    pr_info("pdrv_gpio_sysfs: setup complete, GPIO %d IRQ %d\n", GPIO_NUMBER, gpio_irq);
    return 0;
}

/* Platform driver remove */
static int pdrv_gpio_remove(struct platform_device *pdev)
{
    pr_info("pdrv_gpio_sysfs: remove called\n");

    if (gpio_irq >= 0)
        free_irq(gpio_irq, NULL);

    gpio_free(GPIO_NUMBER);

    return 0;
}

/* Platform driver structure */
static struct platform_driver pdrv_gpio_driver = {
    .probe  = pdrv_gpio_probe,
    .remove = pdrv_gpio_remove,
    .driver = {
        .name = "pdrv_gpio_sysfs",  // must match MODULE_ALIAS
        .owner = THIS_MODULE,
    },
};

module_platform_driver(pdrv_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("DTS-free GPIO platform driver using sysfs probe");
MODULE_ALIAS("platform:pdrv_gpio_sysfs");

