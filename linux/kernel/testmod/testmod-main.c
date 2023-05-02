/**************************************************************************
$Id$
***************************************************************************/

#define DEVICE  "test"

#include ".git.h"
#include "kdbg.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>

DEBUG_PARAM_DEF();

static int g_int = 0;

//-------------------------------------------------------------------------
static void get_pgtable_macro(void)
{
    printk("PAGE_OFFSET = 0x%lx\n", PAGE_OFFSET);
    printk("PGDIR_SHIFT = %d\n", PGDIR_SHIFT);
    printk("PUD_SHIFT = %d\n", PUD_SHIFT);
    printk("PMD_SHIFT = %d\n", PMD_SHIFT);
    printk("PAGE_SHIFT = %d\n", PAGE_SHIFT);

    printk("PTRS_PER_PGD = %d\n", PTRS_PER_PGD);
    printk("PTRS_PER_PUD = %d\n", PTRS_PER_PUD);
    printk("PTRS_PER_PMD = %d\n", PTRS_PER_PMD);
    printk("PTRS_PER_PTE = %d\n", PTRS_PER_PTE);

    printk("PAGE_MASK = 0x%lx\n", PAGE_MASK);
}
//-------------------------------------------------------------------------
static int __init_module(void)
{
	int loc_int = 0x55AA;
	int *ptr = &loc_int;
	char *plog = log_buf_addr_get();
	int loglen = log_buf_len_get();
	INFO("Init done (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]");
	get_pgtable_macro();
	
	
	
	INFO("Log buffer at %pX",plog );
	
	TRACE("tracing here...");	
	
	TRACE("Addr static: %px local: %px ptr: %px", &g_int, &loc_int, &ptr);
	
	return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
  INFO("Unloaded (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]" );
}
//-------------------------------------------------------------------------
module_init(__init_module);
module_exit(__exit_module);
MODULE_DESCRIPTION("Test module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");

/*

cat /sys/module/testmod/sections/.bss

echo "Hello Kernel-World" > /dev/kmsg
echo "<2>Writing critical printk messages from userspace" >/dev/kmsg

grep __log_buf System.map
grep __log_buf /proc/kallsyms


*/
