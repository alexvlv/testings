// SPDX-License-Identifier: GPL-2.0
//
// gpio_uart_tty.c
// Bit-bang UART with TTY interface -> /dev/ttyGU0
//
// Based on prior gpio_uart code: RX via IRQ+hrtimer, TX via udelay() bitbang.
// tty driver supports set_termios to change baud/parity/stop at runtime.
//
// Usage example:
//   insmod gpio_uart_tty.ko tx_gpio=17 rx_gpio=27 baud=9600
//   screen /dev/ttyGU0 9600
//

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/tty_flip.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/spinlock.h>

#define DRIVER_NAME "gpio_uart_tty"
#define TTY_NAME "ttyGU"
#define PROC_NAME "gpio_uart"

struct gu_cfg {
    unsigned int baud;
    unsigned char parity; /* 0=none,1=odd,2=even */
    unsigned char stopbits; /* 1 or 2 */
    bool invert;
    bool tx_enable;
    bool rx_enable;
};

struct gu_stats {
    u64 tx_bytes;
    u64 rx_bytes;
    u64 framing_errors;
    u64 rx_errors;
    u64 overrun_errors;
};

struct gpio_uart {
    /* gpio descriptors */
    struct gpio_desc *tx;
    struct gpio_desc *rx;
    int rx_irq;

    /* configuration + stats */
    struct gu_cfg cfg;
    struct gu_stats st;

    /* RX hrtimer/state */
    struct hrtimer rx_timer;
    ktime_t bit_period;
    spinlock_t rx_lock;
    bool rx_busy;
    u8 rx_byte;
    int rx_bitpos;

    /* tty */
    struct tty_driver *tty_drv;
    struct tty_port tty_port;

    /* proc/sysfs */
    struct proc_dir_entry *proc;
    struct kobject *kobj;
};

static struct gpio_uart *GU;

/* Module params: named GPIOs (use numbers or consumer lines depending on platform) */
static int tx_gpio = -1;
module_param(tx_gpio, int, 0444);
MODULE_PARM_DESC(tx_gpio, "TX gpio number");

static int rx_gpio = -1;
module_param(rx_gpio, int, 0444);
MODULE_PARM_DESC(rx_gpio, "RX gpio number");

static unsigned int default_baud = 9600;
module_param(default_baud, uint, 0444);
MODULE_PARM_DESC(default_baud, "Default baud");

/* ---------------------- low-level helpers ---------------------- */

static inline int get_rx_level(void)
{
    int v = gpiod_get_value(GU->rx);
    return GU->cfg.invert ? !v : v;
}

static inline void set_tx_level(int v)
{
    if (GU->cfg.invert) v = !v;
    gpiod_set_value(GU->tx, v);
}

/* compute parity bit to send: returns 0/1 for parity bit value given data byte */
static int parity_bit_out(u8 b, unsigned char parity_mode)
{
    int pop = __builtin_popcount((unsigned int)b) & 1; /* 1 if odd number of ones */
    if (parity_mode == 1) /* odd */ return pop ? 0 : 1; /* if pop==1 (odd ones), parity bit 0? wait: simpler: parity bit such that total ones odd */
    /* Better: parity bit = (parity_mode == EVEN) ? (pop ? 1 : 0) : (!pop ? 1 : 0) */
    if (parity_mode == 2) /* even */ return pop ? 1 : 0;
    return 0;
}

/* blocking TX for one byte: LSB-first, 8 data bits, optional parity and stop bits */
static void uart_tx_byte(u8 b)
{
    unsigned int bit_us;
    int i;
    if (!GU->cfg.tx_enable) return;

    if (GU->cfg.baud == 0) return;
    bit_us = 1000000U / GU->cfg.baud;

    /* start bit (logical 0) */
    set_tx_level(0);
    udelay(bit_us);

    /* data bits LSB-first */
    for (i = 0; i < 8; ++i) {
        int bit = (b >> i) & 1;
        set_tx_level(bit);
        udelay(bit_us);
    }

    /* parity */
    if (GU->cfg.parity) {
        int pop = __builtin_popcount((unsigned int)b) & 1;
        int pbit;
        if (GU->cfg.parity == 2) /* even */ pbit = pop ? 1 : 0;
        else /* odd */ pbit = pop ? 0 : 1;
        set_tx_level(pbit);
        udelay(bit_us);
    }

    /* stop bits: logical 1 */
    for (i = 0; i < (int)GU->cfg.stopbits; ++i) {
        set_tx_level(1);
        udelay(bit_us);
    }

    GU->st.tx_bytes++;
}

/* ---------------------- RX hrtimer callback ---------------------- */

static enum hrtimer_restart rx_timer_cb(struct hrtimer *t)
{
    unsigned long flags;
    spin_lock_irqsave(&GU->rx_lock, flags);

    if (!GU->cfg.rx_enable) {
        spin_unlock_irqrestore(&GU->rx_lock, flags);
        return HRTIMER_NORESTART;
    }

    if (!GU->rx_busy) {
        /* look for start bit (low) */
        if (get_rx_level() == 0) {
            GU->rx_busy = true;
            GU->rx_bitpos = -1; /* -1 => start bit sampling already scheduled (we sample middle using half step) */
            GU->rx_byte = 0;
            /* schedule next in half-bit to sample middle of first data bit */
            hrtimer_forward_now(&GU->rx_timer, ktime_divns(GU->bit_period, 2));
            spin_unlock_irqrestore(&GU->rx_lock, flags);
            return HRTIMER_RESTART;
        }
        spin_unlock_irqrestore(&GU->rx_lock, flags);
        return HRTIMER_RESTART;
    }

    /* we are sampling */
    GU->rx_bitpos++;

    if (GU->rx_bitpos >= 0 && GU->rx_bitpos < 8) {
        int bit = get_rx_level();
        GU->rx_byte |= (bit << GU->rx_bitpos);
        hrtimer_forward_now(&GU->rx_timer, GU->bit_period);
        spin_unlock_irqrestore(&GU->rx_lock, flags);
        return HRTIMER_RESTART;
    }

    /* parity handling (if enabled) — simplistic: ignore parity for now but could check */
    if (GU->cfg.parity && GU->rx_bitpos == 8) {
        /* read parity bit (optionally check) */
        /* TODO: check parity and flag errors */
        hrtimer_forward_now(&GU->rx_timer, GU->bit_period);
        spin_unlock_irqrestore(&GU->rx_lock, flags);
        return HRTIMER_RESTART;
    }

    /* stop bit(s) */
    if (GU->rx_bitpos >= (8 + (GU->cfg.parity ? 1 : 0))) {
        int stop_ok = (get_rx_level() == 1);
        if (!stop_ok) {
            GU->st.framing_errors++;
        } else {
            /* deliver to tty */
            if (tty_port_initialized(&GU->tty_port)) {
                tty_insert_flip_char(&GU->tty_port, GU->rx_byte, TTY_NORMAL);
                tty_flip_buffer_push(&GU->tty_port);
            }
            GU->st.rx_bytes++;
        }
        GU->rx_busy = false;
        spin_unlock_irqrestore(&GU->rx_lock, flags);
        return HRTIMER_RESTART;
    }

    /* default fallback */
    GU->rx_busy = false;
    spin_unlock_irqrestore(&GU->rx_lock, flags);
    return HRTIMER_RESTART;
}

/* ---------------------- TTY operations ---------------------- */

static int gu_tty_open(struct tty_struct *tty, struct file *file)
{
    int ret;

    ret = tty_port_open(&GU->tty_port, tty, file);
    if (ret)
        return ret;

    tty->driver_data = GU;
    return 0;
}

static void gu_tty_close(struct tty_struct *tty, struct file *file)
{
    tty_port_close(&GU->tty_port, tty, file);
}

static int gu_tty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
    int i;
    if (!GU->cfg.tx_enable) return -EIO;

    for (i = 0; i < count; ++i)
        uart_tx_byte(buf[i]);

    return count;
}

static int gu_tty_write_room(struct tty_struct *tty)
{
    /* always room for a few bytes; not implementing flow control */
    return 4096;
}

/* set_termios: adapt speed/parity/stopbits from termios */
static void gu_set_termios(struct tty_struct *tty, struct ktermios *old)
{
    unsigned int baud_new = 9600;
    unsigned int cflag = tty->termios.c_cflag;

    /* try to get baud from termios */
    baud_new = tty_get_baud_rate(tty);
    if (baud_new == 0) baud_new = GU->cfg.baud;

    GU->cfg.baud = baud_new;

    /* parity */
    if (cflag & PARENB) {
        if (cflag & PARODD) GU->cfg.parity = 1; /* odd */
        else GU->cfg.parity = 2; /* even */
    } else {
        GU->cfg.parity = 0;
    }

    /* stop bits */
    GU->cfg.stopbits = (cflag & CSTOPB) ? 2 : 1;

    /* recompute bit_period */
    if (GU->cfg.baud)
        GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);
}

/* tty ops struct */
static const struct tty_operations gu_tty_ops = {
    .open = gu_tty_open,
    .close = gu_tty_close,
    .write = gu_tty_write,
    .write_room = gu_tty_write_room,
    .set_termios = gu_set_termios,
};

/* ---------------------- procfs & sysfs for stats/config ---------------------- */

static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m,
        "gpio_uart stats:\n"
        " tx_bytes:       %llu\n"
        " rx_bytes:       %llu\n"
        " framing_errors: %llu\n"
        " rx_errors:      %llu\n"
        " overrun_errors: %llu\n"
        " baud:           %u\n"
        " parity:         %u\n"
        " stopbits:       %u\n",
        GU->st.tx_bytes, GU->st.rx_bytes, GU->st.framing_errors,
        GU->st.rx_errors, GU->st.overrun_errors,
        GU->cfg.baud, GU->cfg.parity, GU->cfg.stopbits
    );
    return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_ops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

/* simple sysfs attributes (baud, parity, stopbits, tx_enable, rx_enable) */

static ssize_t baud_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{ return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.baud); }
static ssize_t baud_store(struct kobject *k, struct kobj_attribute *a,
                          const char *buf, size_t count)
{
    unsigned long v;
    if (kstrtoul(buf, 0, &v)) return -EINVAL;
    GU->cfg.baud = v ? v : GU->cfg.baud;
    GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);
    return count;
}
static struct kobj_attribute baud_attr = __ATTR(baud, 0644, baud_show, baud_store);

static ssize_t parity_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{ return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.parity); }
static ssize_t parity_store(struct kobject *k, struct kobj_attribute *a,
                            const char *buf, size_t count)
{
    unsigned long v;
    if (kstrtoul(buf, 0, &v)) return -EINVAL;
    GU->cfg.parity = (unsigned char)v;
    return count;
}
static struct kobj_attribute parity_attr = __ATTR(parity, 0644, parity_show, parity_store);

static ssize_t stop_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{ return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.stopbits); }
static ssize_t stop_store(struct kobject *k, struct kobj_attribute *a,
                          const char *buf, size_t count)
{
    unsigned long v;
    if (kstrtoul(buf, 0, &v)) return -EINVAL;
    if (v < 1 || v > 2) return -EINVAL;
    GU->cfg.stopbits = (unsigned char)v;
    return count;
}
static struct kobj_attribute stop_attr = __ATTR(stopbits, 0644, stop_show, stop_store);

static ssize_t txen_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{ return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.tx_enable); }
static ssize_t txen_store(struct kobject *k, struct kobj_attribute *a,
                          const char *buf, size_t count)
{
    unsigned long v;
    if (kstrtoul(buf, 0, &v)) return -EINVAL;
    GU->cfg.tx_enable = (v ? true : false);
    return count;
}
static struct kobj_attribute txen_attr = __ATTR(tx_enable, 0644, txen_show, txen_store);

static ssize_t rxen_show(struct kobject *k, struct kobj_attribute *a, char *buf)
{ return scnprintf(buf, PAGE_SIZE, "%u\n", GU->cfg.rx_enable); }
static ssize_t rxen_store(struct kobject *k, struct kobj_attribute *a,
                          const char *buf, size_t count)
{
    unsigned long v;
    if (kstrtoul(buf, 0, &v)) return -EINVAL;
    GU->cfg.rx_enable = (v ? true : false);
    return count;
}
static struct kobj_attribute rxen_attr = __ATTR(rx_enable, 0644, rxen_show, rxen_store);

static struct attribute *gu_attrs[] = {
    &baud_attr.attr,
    &parity_attr.attr,
    &stop_attr.attr,
    &txen_attr.attr,
    &rxen_attr.attr,
    NULL,
};
static struct attribute_group gu_attr_group = { .attrs = gu_attrs };

/* ---------------------- module init / exit ---------------------- */

static int __init gpio_uart_init(void)
{
    int ret;
    struct device *dev;

    pr_info(DRIVER_NAME ": init\n");

    GU = kzalloc(sizeof(*GU), GFP_KERNEL);
    if (!GU) return -ENOMEM;

    /* default cfg */
    GU->cfg.baud = default_baud;
    GU->cfg.parity = 0;
    GU->cfg.stopbits = 1;
    GU->cfg.invert = false;
    GU->cfg.tx_enable = true;
    GU->cfg.rx_enable = true;

    /* compute bit_period */
    if (GU->cfg.baud == 0) GU->cfg.baud = 9600;
    GU->bit_period = ktime_set(0, NSEC_PER_SEC / GU->cfg.baud);

    spin_lock_init(&GU->rx_lock);

    /* request GPIOs by number (platform-specific). If platform has gpiod by name, adapt. */
    if (tx_gpio < 0 || rx_gpio < 0) {
        pr_err(DRIVER_NAME ": tx_gpio and rx_gpio module params required\n");
        ret = -EINVAL;
        goto err_free;
    }

    GU->tx = gpio_to_desc(tx_gpio);
    GU->rx = gpio_to_desc(rx_gpio);
    if (!GU->tx || !GU->rx) {
        pr_err(DRIVER_NAME ": gpio_to_desc failed for %d or %d\n", tx_gpio, rx_gpio);
        ret = -EINVAL;
        goto err_free;
    }

    /* configure gpio directions */
    ret = gpiod_direction_output(GU->tx, 1);
    if (ret) {
        pr_err(DRIVER_NAME ": failed to set tx gpio output\n");
        goto err_free;
    }
    ret = gpiod_direction_input(GU->rx);
    if (ret) {
        pr_err(DRIVER_NAME ": failed to set rx gpio input\n");
        goto err_free;
    }

    /* init hrtimer */
    hrtimer_init(&GU->rx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    GU->rx_timer.function = rx_timer_cb;
    hrtimer_start(&GU->rx_timer, GU->bit_period, HRTIMER_MODE_REL);

    /* create tty driver */
    GU->tty_drv = alloc_tty_driver(1);
    if (!GU->tty_drv) {
        pr_err(DRIVER_NAME ": alloc_tty_driver failed\n");
        ret = -ENOMEM;
        goto err_timer;
    }

    GU->tty_drv->owner = THIS_MODULE;
    GU->tty_drv->driver_name = "gpio_uart_tty";
    GU->tty_drv->name = TTY_NAME;
    GU->tty_drv->major = 0; /* dynamic */
    GU->tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
    GU->tty_drv->subtype = SERIAL_TYPE_NORMAL;
    GU->tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
    GU->tty_drv->init_termios = tty_std_termios;
    GU->tty_drv->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL;
    tty_set_operations(GU->tty_drv, &gu_tty_ops);

    ret = tty_register_driver(GU->tty_drv);
    if (ret) {
        pr_err(DRIVER_NAME ": tty_register_driver failed: %d\n", ret);
        goto err_put_drv;
    }

    tty_port_init(&GU->tty_port);

    /* create device node /dev/ttyGU0 */
    dev = tty_register_device(GU->tty_drv, 0, NULL);
    if (IS_ERR(dev)) {
        pr_err(DRIVER_NAME ": tty_register_device failed\n");
        ret = PTR_ERR(dev);
        goto err_unregister;
    }

    /* proc */
    GU->proc = proc_create(PROC_NAME, 0444, NULL, &proc_ops);
    if (!GU->proc)
        pr_warn(DRIVER_NAME ": failed to create /proc/%s (non-fatal)\n", PROC_NAME);

    /* sysfs */
    GU->kobj = kobject_create_and_add("gpio_uart", kernel_kobj);
    if (!GU->kobj) {
        pr_warn(DRIVER_NAME ": failed to create sysfs kobj\n");
    } else {
        ret = sysfs_create_group(GU->kobj, &gu_attr_group);
        if (ret) pr_warn(DRIVER_NAME ": sysfs_create_group failed: %d\n", ret);
    }

    pr_info(DRIVER_NAME ": loaded, device /dev/%s0\n", TTY_NAME);
    return 0;

err_unregister:
    tty_unregister_driver(GU->tty_drv);
err_put_drv:
    put_tty_driver(GU->tty_drv);
err_timer:
    hrtimer_cancel(&GU->rx_timer);
err_free:
    kfree(GU);
    return ret;
}

static void __exit gpio_uart_exit(void)
{
    pr_info(DRIVER_NAME ": exit\n");

    if (GU->kobj) {
        sysfs_remove_group(GU->kobj, &gu_attr_group);
        kobject_put(GU->kobj);
    }

    if (GU->proc) proc_remove(GU->proc);

    /* stop timer */
    hrtimer_cancel(&GU->rx_timer);

    /* unregister tty device and driver */
    tty_unregister_device(GU->tty_drv, 0);
    tty_unregister_driver(GU->tty_drv);
    put_tty_driver(GU->tty_drv);
    tty_port_destroy(&GU->tty_port);

    /* free GPIO (no explicit free needed for gpio_desc) */
    /* set tx low before exit */
    set_tx_level(0);

    kfree(GU);
    pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(gpio_uart_init);
module_exit(gpio_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You - adapted");
MODULE_DESCRIPTION("GPIO bitbang UART with TTY interface (/dev/ttyGU0)");

