// Mon 19 Oct 2026 12:05:07
//
#ifndef dbg_h_
#define dbg_h_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DBG_SHIFT_DBG	    0
#define DBG_SHIFT_VERBOSE	1
#define DBG_SHIFT_TRACE		2
#define DBG_SHIFT_MORE		3

#define TRACE_SHIFT_DEFAULT 	DBG_SHIFT_MORE

#define DBG_DEFINE(v) unsigned dbg_lvl = v;
#define DBG_DEFINE_TRACE_ALL() unsigned dbg_lvl = ~0;
#define DBG_INIT(v) do { dbg_lvl = v; } while (0)
#define DBG_LVL_INC() do { dbg_lvl = (dbg_lvl<<1)|1; } while (0)

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

#define VERB(fmt, args...) if(dbg_lvl&(1<<DBG_SHIFT_VERBOSE)) {fprintf(stderr, DBG_FMT_PREFIX fmt "\n", DBG_APPNAME, ## args);}
#define DBG(fmt, args...) if(dbg_lvl) {fprintf(stderr, DBG_FMT_PREFIX fmt "\n", DBG_APPNAME, ## args);}
#define DBG_(fmt, args...) if(dbg_lvl) {fprintf(stderr, DBG_FMT_PREFIX fmt, DBG_APPNAME, ## args);}
#define INFO_EX(fmt, suffix, args...)  do { fprintf(stderr, DBG_FMT_PREFIX fmt suffix, DBG_APPNAME,  ## args);} while (0)
#define INFO_(fmt, args...) INFO_EX(fmt, "", ## args)
#define INFO(fmt, args...) INFO_EX(fmt, "\n", ## args)
#define CRLF  fprintf(stderr, "\r\n")

/* Error message */
#define ERR(fmt, args...) fprintf(stderr, DBG_FMT_PREFIX "ERR: " fmt "\n", DBG_APPNAME, ## args)
#define WARN(fmt, args...) fprintf(stderr, DBG_FMT_PREFIX "WARN: " fmt "\n", DBG_APPNAME, ## args)
#define ERR_STRING errno>0?strerror(errno):""
#define ERROR(fmt, args...)  \
 do {fprintf(stderr, "\r[%s] ERROR:" xstr(__FILE__) ":" xstr(__LINE__) DBG_FMT_PREFIX fmt ":%s(%d)\n", \
 DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);exit(1);} while (0)
#define WARNING(fmt, args...)  \
 do { fprintf(stderr, "\r[%s] WARNING:" xstr(__FILE__) ":" xstr(__LINE__) DBG_FMT_PREFIX fmt ":%s(%d)\n", \
 DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,strerror(errno),errno);errno=0;} while (0)

#define IS_DBG()   ( dbg_lvl!=0 )
#define IS_VERB()   (dbg_lvl&(1<<DBG_SHIFT_VERBOSE))
#define IS_TRACE()   (dbg_lvl&(1<<DBG_SHIFT_TRACE))
#define IS_TRACE_UNIT(shift) (dbg_lvl&(1<<shift))

//__builtin_ctz(~dbg_lvl)
// __builtin_ctz() returns the number of zero bits starting from the least-significant bit.
#define DBG_LVL() (  IS_DBG()?__builtin_ctz(~(dbg_lvl&BIT_MASK_RIGHT(unsigned, DBG_SHIFT_MORE+1))):0 )

 #define TRACE_PRINT(fmt, args...) \
	do { fprintf(stderr, DBG_FMT_PREFIX "T:" xstr(__FILE__) ":" xstr(__LINE__) "[%s]:" fmt " :%s(%d)\n", \
		DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,ERR_STRING,errno); errno=0;} while(0)

#define TRACE_PRINT_COND(cond, ...) do { if(cond) TRACE_PRINT(__VA_ARGS__); } while(0)
#define TRACE_PRINT_MASK(mask, ...) TRACE_PRINT_COND((dbg_lvl & (mask)) == (mask), __VA_ARGS__)

#define TRACE_SHIFT(x)			(1<<(x))

#define TRACE_PRINT_SHIFT(shift, ...) TRACE_PRINT_MASK(1<<(shift), __VA_ARGS__)
#define TRACE_PRINT_SHIFT2(shift0, shift1, ...) TRACE_PRINT_MASK((1<<(shift0))|(1<<(shift1)), __VA_ARGS__)

// #define  TRACE_SHIFT_UNIT before including this header!
#ifdef TRACE_SHIFT_UNIT
#define TRACE(...)  TRACE_PRINT_SHIFT(TRACE_SHIFT_UNIT, __VA_ARGS__)
#define TRACE_MORE(...)  TRACE_PRINT_SHIFT2(TRACE_SHIFT_UNIT, DBG_SHIFT_TRACE, __VA_ARGS__)
#define TRACE_HIGH(...)  TRACE_PRINT_SHIFT2(TRACE_SHIFT_UNIT, DBG_SHIFT_MORE, __VA_ARGS__)
#define IS_TRACE_CURRENT() IS_TRACE_UNIT(TRACE_SHIFT_UNIT)
#else
#define TRACE(...)  TRACE_PRINT_SHIFT(DBG_SHIFT_TRACE, __VA_ARGS__)
#define TRACE_MORE(...) TRACE_PRINT_SHIFT(DBG_SHIFT_MORE, __VA_ARGS__)
#define TRACE_HIGH TRACE_MORE
#define IS_TRACE_CURRENT() false
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c %c%c%c%c"
#define WORD_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"

#define WTBSM(word, shift) (((word) & (1U << (shift))) ? '1' : '0')
#define NIBBLE_TO_BINARY(word) WTBSM(word,3), WTBSM(word,2), WTBSM(word,1), WTBSM(word,0)

#define BYTE_TO_BINARY(byte) \
	NIBBLE_TO_BINARY((byte) >> 4), \
	NIBBLE_TO_BINARY(byte)

#define WORD_TO_BINARY(word) \
	BYTE_TO_BINARY((word) >> 24), \
	BYTE_TO_BINARY((word) >> 16), \
	BYTE_TO_BINARY((word) >> 8), \
	BYTE_TO_BINARY(word)

extern unsigned dbg_lvl;
extern const char *dbg_main_argv0;

#endif
