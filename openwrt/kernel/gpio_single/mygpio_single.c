                                                                                                                                 #include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define GPIO_NUMBER 532   // Change to your GPIO pin
#define GPIO_DESC   "mygpio_single"

static int gpio_irq = -1;
static struct platform_device *pdev;

/* IRQ handler */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    int val = gpio_get_value(GPIO_NUMBER);
    pr_info("mygpio_single: GPIO %d changed, value=%d\n", GPIO_NUMBER, val);
    return IRQ_HANDLED;
}

/* Platform driver probe */
static int mygpio_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("mygpio_single: probe called\n");

    ret = gpio_request(GPIO_NUMBER, GPIO_DESC);
    if (ret) {
        pr_err("mygpio_single: failed to request GPIO %d\n", GPIO_NUMBER);
        return ret;
    }

    ret = gpio_direction_input(GPIO_NUMBER);
    if (ret) {
        pr_err("mygpio_single: failed to set GPIO %d as input\n", GPIO_NUMBER);
        gpio_free(GPIO_NUMBER);
        return ret;
    }

    gpio_irq = gpio_to_irq(GPIO_NUMBER);
    if (gpio_irq < 0) {
        pr_err("mygpio_single: failed to get IRQ for GPIO %d\n", GPIO_NUMBER);
        gpio_free(GPIO_NUMBER);
        return gpio_irq;
    }

    ret = request_irq(gpio_irq, gpio_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "mygpio_single_irq", NULL);
    if (ret) {
        pr_err("mygpio_single: failed to request IRQ %d\n", gpio_irq);
        gpio_free(GPIO_NUMBER);
        return ret;
    }

    pr_info("mygpio_single: setup complete, GPIO %d IRQ %d\n", GPIO_NUMBER, gpio_irq);
    return 0;
}

/* Platform driver remove */
static int mygpio_remove(struct platform_device *pdev)
{
    pr_info("mygpio_single: remove called\n");

    if (gpio_irq >= 0)
        free_irq(gpio_irq, NULL);

    gpio_free(GPIO_NUMBER);

    return 0;
}

/* Platform driver struct */
static struct platform_driver mygpio_driver = {
    .probe  = mygpio_probe,
    .remove = mygpio_remove,
    .driver = {
        .name = "mygpio_single",
        .owner = THIS_MODULE,
    },
};

/* Module init/exit */
static int __init mygpio_init(void)
{
    int ret;

    /* register platform device */
    pdev = platform_device_register_simple("mygpio_single", -1, NULL, 0);
    if (IS_ERR(pdev))
        return PTR_ERR(pdev);

    /* register platform driver */
    ret = platform_driver_register(&mygpio_driver);
    if (ret) {
        platform_device_unregister(pdev);
        return ret;
    }

    pr_info("mygpio_single: module loaded\n");
    return 0;
}

static void __exit mygpio_exit(void)
{
    platform_driver_unregister(&mygpio_driver);
    platform_device_unregister(pdev);
    pr_info("mygpio_single: module unloaded\n");
}

module_init(mygpio_init);
module_exit(mygpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Single-module DTS-free GPIO platform driver with IRQ");
