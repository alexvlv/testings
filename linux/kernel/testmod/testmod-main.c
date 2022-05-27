/**************************************************************************
$Id$
***************************************************************************/

#define DEVICE  "test"

#define TRACE(...) { if(dbg>1) printk ( KERN_DEBUG "-T- " DEVICE ": "  __VA_ARGS__);  } 
#define DBG(...) {   if(dbg) printk ( KERN_DEBUG "-D- " DEVICE ": "  __VA_ARGS__);  } 
#define INFO(...) {  printk(KERN_INFO  "-I- " DEVICE ": " __VA_ARGS__);  }
#define WARNING(...) {  printk ( KERN_WARNING "-W- " DEVICE ": " __VA_ARGS__);  }
#define ERROR(...) {  printk ( KERN_ERR "-E- " DEVICE ": " __VA_ARGS__);  }


#include ".git.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/version.h>


static int __init_module(void)
{
  INFO("Init done (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "]\n");
  return 0;
}
//-------------------------------------------------------------------------
static void __exit_module(void)
{
  INFO("Unloaded (GIT Rev." VERSION ") [Build: " __TIME__ " " __DATE__ "]\n" );
}
//-------------------------------------------------------------------------
module_init(__init_module);
module_exit(__exit_module);
MODULE_DESCRIPTION("Test module");
MODULE_AUTHOR("AlexVol");
MODULE_LICENSE("GPL");

// to suppress "loading out-of-tree module taints kernel." warning
MODULE_INFO(intree,"Y");
