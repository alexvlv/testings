/**************************************************************************
$Id$
***************************************************************************/

#pragma once

#include <linux/cdev.h>

struct bitbang_uart;
struct bitbang_stats;

struct bitbang_stats {
	int tx_counter;
	int rx_counter;
	int rx_errs_start;
	int rx_errs_stop;
	int rx_errs_parity;
};

struct bitbang_priv {
	struct cdev cdev;
	dev_t devt;
	struct class *cls;
	struct device *dev;
	struct proc_dir_entry *proc_entry;
	struct platform_device *pdev;   // back-reference
	struct bitbang_uart *uart;
	struct bitbang_stats stats;
	int tx_gpio;
	int rx_gpio;
	unsigned period_ms;
};

struct platform_device;

int bitbang_sysfs_init(struct platform_device *pdev);
void bitbang_sysfs_exit(struct platform_device *pdev);

int bitbang_proc_init(struct bitbang_priv *priv);
void bitbang_proc_exit(struct bitbang_priv *priv);
