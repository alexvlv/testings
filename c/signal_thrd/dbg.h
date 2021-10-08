/**************************************************************************
$Id$
***************************************************************************/

#ifndef dbg_h_
#define dbg_h_

//#define DBG_APPNAME_PREFIX

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h> 
#include <errno.h>

#define DBG_DEFINE(v) unsigned dbg_lvl = v;
#define DBG_DEFINE_TRACE_ALL() unsigned dbg_lvl = ~0;
#define DBG_INIT(v) dbg_lvl = v;


#ifdef DBG_APPNAME_PREFIX
#define DBG_APPNAME_DECLARE() const char *dbg_main_argv0 = NULL;
#define DBG_APPNAME_MAIN_INIT() dbg_main_argv0 = argv[0];
#define DBG_APPNAME basename(dbg_main_argv0)
#define DBG_FMT_PREFIX "[%s] "
#else
#define DBG_APPNAME_DECLARE()
#define DBG_APPNAME_MAIN_INIT() {}
#define DBG_APPNAME ""
#define DBG_FMT_PREFIX "%s"
#endif

#define DBG_CLEAR_ERR() {errno=0;}

/* Error message */
#define ERR(fmt, args...) fprintf(stderr, "\r" DBG_FMT_PREFIX "ERR: " fmt "\n", DBG_APPNAME, ## args)
#define WARN(fmt, args...) fprintf(stderr, "\r" DBG_FMT_PREFIX "WARN: " fmt "\n", DBG_APPNAME, ## args)
#define ERR_STRING errno>0?strerror(errno):""
#define ERROR(fmt, args...)  {fprintf(stderr, "\r[%s] ERROR:" xstr(__FILE__) ":" xstr(__LINE__) DBG_FMT_PREFIX fmt ":%s(%d)\n", \
	DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);exit(1);}
#define WARNING(fmt, args...)  { fprintf(stderr, "\r[%s] WARNING:" xstr(__FILE__) ":" xstr(__LINE__) DBG_FMT_PREFIX fmt ":%s(%d)\n", \
	DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);errno=0;}

#define VERB(fmt, args...) if(dbg_lvl>1) {fprintf(stderr, DBG_FMT_PREFIX fmt "\n", DBG_APPNAME, ## args);}
#define DBG(fmt, args...) if(dbg_lvl) {fprintf(stderr, DBG_FMT_PREFIX fmt "\n", DBG_APPNAME, ## args);}
#define INFO(fmt, args...)  { fprintf(stderr, DBG_FMT_PREFIX fmt "\n", DBG_APPNAME,  ## args);}
#define CRLF  fprintf(stderr, "\r\n");
#define IS_DBG()   ( dbg_lvl!=0 )
#define IS_VERB()   ( dbg_lvl>1 )

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c %c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


#define WORD_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"
#define WORD_TO_BINARY(word)  \
  (word & 0x80000000 ? '1' : '0'), \
  (word & 0x40000000 ? '1' : '0'), \
  (word & 0x20000000 ? '1' : '0'), \
  (word & 0x10000000 ? '1' : '0'), \
  (word & 0x08000000 ? '1' : '0'), \
  (word & 0x04000000 ? '1' : '0'), \
  (word & 0x02000000 ? '1' : '0'), \
  (word & 0x01000000 ? '1' : '0'), \
  (word & 0x800000 ? '1' : '0'), \
  (word & 0x400000 ? '1' : '0'), \
  (word & 0x200000 ? '1' : '0'), \
  (word & 0x100000 ? '1' : '0'), \
  (word & 0x080000 ? '1' : '0'), \
  (word & 0x040000 ? '1' : '0'), \
  (word & 0x020000 ? '1' : '0'), \
  (word & 0x010000 ? '1' : '0'), \
  (word & 0x8000 ? '1' : '0'), \
  (word & 0x4000 ? '1' : '0'), \
  (word & 0x2000 ? '1' : '0'), \
  (word & 0x1000 ? '1' : '0'), \
  (word & 0x0800 ? '1' : '0'), \
  (word & 0x0400 ? '1' : '0'), \
  (word & 0x0200 ? '1' : '0'), \
  (word & 0x0100 ? '1' : '0'), \
  (word & 0x80 ? '1' : '0'), \
  (word & 0x40 ? '1' : '0'), \
  (word & 0x20 ? '1' : '0'), \
  (word & 0x10 ? '1' : '0'), \
  (word & 0x08 ? '1' : '0'), \
  (word & 0x04 ? '1' : '0'), \
  (word & 0x02 ? '1' : '0'), \
  (word & 0x01 ? '1' : '0')

extern unsigned dbg_lvl;
extern const char *dbg_main_argv0;

static inline unsigned int get_tid(void){
	return syscall( __NR_gettid);
}

#endif
