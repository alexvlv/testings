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

//-------------------------------------------------------------------------
static int __init_module(void)
{
	INFO("Init done (GIT Rev." GIT_REVISION ") [Build: " __TIME__ " " __DATE__ "]");
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
