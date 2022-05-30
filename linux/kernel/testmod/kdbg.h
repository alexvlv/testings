/**************************************************************************
$Id$
***************************************************************************/

#pragma once

#define TRACE(fmt, args...) { if(__dbglvl>1) printk ( KERN_DEBUG "-T- " DEVICE ": " fmt "\n",  ## args);  } 
#define DBG(fmt, args...) {   if(__dbglvl) printk ( KERN_DEBUG "-D- " DEVICE ": " fmt "\n",  ## args);  } 
#define INFO(fmt, args...) {  printk(KERN_INFO  "-I- " DEVICE ": " fmt "\n",  ## args);  } 
#define WARNING(fmt, args...) {  printk ( KERN_WARNING "-W- " DEVICE ": " fmt "\n",  ## args);  } 
#define ERROR(fmt, args...) {  printk ( KERN_ERR "-E- " DEVICE ": " fmt "\n",  ## args);  } 

