/**************************************************************************
$Id$
***************************************************************************/

#ifndef dbg_h_
#define dbg_h_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h> 
#include <errno.h>

#define DBG_INIT(v) dbg = v;

/* Error message */
#define ERR(fmt, args...) fprintf(stderr, "\rERR: " fmt "\n", ## args)
#define WARN(fmt, args...) fprintf(stderr, "\rWARN: " fmt "\n", ## args)
#define ERR_STRING errno>0?strerror(errno):""
#define ERROR(fmt, args...)  {fprintf(stderr, "\rERROR:" xstr(__FILE__) ":" xstr(__LINE__) "[%s]:" fmt ":%s(%d)\n", __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);exit(1);}
#define WARNING(fmt, args...)  { fprintf(stderr, "\rWARNING:" xstr(__FILE__) ":" xstr(__LINE__) "[%s]:" fmt ":%s(%d)\n", __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);errno=0;}

#define VERB(fmt, args...) if(dbg>1) {fprintf(stderr, fmt "\n", ## args);}
#define DBG(fmt, args...) if(dbg) {fprintf(stderr, fmt "\n", ## args);}
#define INFO(fmt, args...)  { fprintf(stderr, fmt "\n", ## args);}
#define CRLF  fprintf(stderr, "\r\n");
#define IS_DBG()   ( dbg!=0 )
#define IS_VERB()   ( dbg>1 )

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

extern unsigned dbg;

static inline unsigned int get_tid(void){
	return syscall( __NR_gettid);
}

#endif
