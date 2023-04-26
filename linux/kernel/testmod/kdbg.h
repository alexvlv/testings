/**************************************************************************
$Id$
***************************************************************************/

#pragma once

#ifndef xstr
#define xstr(s)   str(s)
#define str(s)	  #s
#endif

#ifdef DEVNAME
#undef DEVICE
#define DEVICE   xstr(DEVNAME)
#else
#error "DEVNAME must be defined!"
#endif

#ifndef DBG_LVL_DEFAULT
#define DBG_LVL_DEFAULT  0
#endif


// One level of macro indirection is required in order to resolve __COUNTER__,
// and get varname1 instead of varname__COUNTER__.
// https://stackoverflow.com/questions/1082192/how-to-generate-random-variable-names-in-c-using-macros
#ifndef CONCAT
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#endif

#define DBG_LVL CONCAT(__dbglvl_, DEVNAME)

#ifndef DEBUG_PARAM_STATIC
#define DEBUG_PARAM_DEF() DEBUG_PARAM_EX(DBG_LVL_DEFAULT)
#define DEBUG_PARAM(default) DEBUG_PARAM_EX(default)
extern int DBG_LVL;
#else 
#define DEBUG_PARAM_DEF() DEBUG_PARAM_EX(DBG_LVL_DEFAULT)
#define DEBUG_PARAM(default) static DEBUG_PARAM_EX(default)
#endif

#define DEBUG_PARAM_EX(default) int DBG_LVL = default; \
	module_param_named(dbg, DBG_LVL, int, S_IRUGO|S_IWUSR); \
	MODULE_PARM_DESC(dbg, "Debug level");


#define TRACE(fmt, args...) \
	do { if(DBG_LVL>1) printk ( \
		KERN_DEBUG "-T- " DEVICE " %s:" xstr(__LINE__) ":(%s) " fmt "\n", \
		kbasename(__FILE__),__PRETTY_FUNCTION__,  ## args); } while (0)
#define DBG(fmt, args...) \
	do { if(DBG_LVL) printk ( \
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
