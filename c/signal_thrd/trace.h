/**************************************************************************
$Id$
***************************************************************************/

#ifndef trace_h_
#define trace_h_

#include "dbg.h"
#include "macro.h"
//#include "trace_defs.h"

#ifndef TRACE_SHIFT_MAX
#define TRACE_SHIFT_MAX 		2
#endif

#define TRACE_NUM_BITS   (sizeof(dbg_lvl)*8)
#define TRACE_MASK_DEFAULT  (1<<TRACE_SHIFT_MAX)

#ifndef TRACE_MASK
#define TRACE_MASK TRACE_MASK_DEFAULT
#endif

#define TRACE_shift2mask(x)			(1<<(x))
#define TRACE_shift2mask2(x,y)		(1<<(x)|1<<(y))
#define TRACE_shift2mask3(x,y,z)	(1<<(x)|1<<(y)|1<<(z))

#define TRACESM(mask,fmt, args...) if(dbg_lvl&(mask)) \
    {fprintf(stderr, "\r" DBG_FMT_PREFIX "T:" xstr(__FILE__) ":" xstr(__LINE__) "[%s]:" fmt ":%s(%d)\n", \
        DBG_APPNAME, __PRETTY_FUNCTION__, ## args ,ERR_STRING,errno); errno=0;}

#define TRACE(fmt, args...) TRACESM(TRACE_MASK,fmt, ## args)

#define IS_TRACE() ( dbg_lvl & (1<<TRACE_SHIFT_MAX) )

struct DebugTracer {
	unsigned offset;
	const char c;
	const char *desc;
};

void debug_trace_parse(const struct DebugTracer *tracers, const char *optarg);
void debug_level_print(void);
const char *time2str(void);

#endif
