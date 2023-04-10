/**************************************************************************
$Id $
***************************************************************************/

#pragma once

#ifndef xstr
#define xstr(s)   str(s)
#define str(s)	  #s
#endif

#ifdef DEVNAME
#undef DEVICE
#define DEVICE   xstr(DEVNAME)
#endif

#ifndef DEBUG_PARAM_STATIC
#define DEBUG_PARAM(default) DEBUG_PARAM_EX(default)
extern int __dbglvl;
#else 
#define DEBUG_PARAM(default) static DEBUG_PARAM_EX(default)
#endif

#define DEBUG_PARAM_EX(default) int __dbglvl = default; \
	module_param_named(dbg, __dbglvl, int, S_IRUGO|S_IWUSR); \
	MODULE_PARM_DESC(dbg, "Debug level");



#define TRACE(fmt, args...) \
	do { if(__dbglvl>1) printk ( \
		KERN_DEBUG "-T- " DEVICE " %s:" xstr(__LINE__) ":(%s) " fmt "\n", \
		kbasename(__FILE__),__PRETTY_FUNCTION__,  ## args); } while (0)
#define DBG(fmt, args...) \
	do { if(__dbglvl) printk ( \
		KERN_DEBUG "-D- " DEVICE " " fmt "\n",  ## args); } while (0)
#define INFO(fmt, args...) \
	do { printk(KERN_INFO  "-I- " DEVICE " " fmt "\n",  ## args); } while (0)
#define WARNING(fmt, args...) \
	do { printk ( KERN_WARNING "-W- " DEVICE " " fmt "\n",  ## args); } while (0)
#define ERROR(fmt, args...) \
	do {  printk ( KERN_ERR "-E- " DEVICE ": " fmt "\n",  ## args); } while (0)

/* 
  About "do {...}while (0)" see "Swallowing the Semicolon"
*/  
