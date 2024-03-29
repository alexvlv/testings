/**************************************************************************
$Id$
***************************************************************************/

#ifndef macro_h_
#define macro_h_

#define xstr(s)   str(s)
#define str(s)	  #s

/* only for userspace compatibility */
#ifndef __KERNEL__
#include <limits.h>     /* CHAR_BIT */
#else
#ifndef CHAR_BIT
#define CHAR_BIT 8	/* Normally in <limits.h> */
#endif
#endif // __KERNEL__

#define BIT_MASK_RIGHT(__TYPE__, __ONE_COUNT__) \
    ((__TYPE__) (-((__ONE_COUNT__) != 0))) \
    & (((__TYPE__) -1) >> ((sizeof(__TYPE__) * CHAR_BIT) - (__ONE_COUNT__)))
/* https://stackoverflow.com/questions/1392059/algorithm-to-generate-bit-mask */

#define BIT_MASK_UNSIGNED(__TYPE__) ((__TYPE__)-1)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef SIGN
#define SIGN(X) ((X > 0) - (X < 0))
#endif

/* Return the smallest multiple N of y such that:
   x <= y * N */
#ifndef CEIL_DIV
#define CEIL_DIV(a, b) (((a)+(a)%(b))/(b))
#endif

#ifndef ALIGN_OF
#define ALIGN_OF(x,sz) (( (x) + ((sz) - 1) )&-(sz))
#endif

#ifndef ALIGN_SHORT
#define ALIGN_SHORT(x) ALIGN_OF(x,2)
#endif

#ifndef ALIGN_DWORD
#define ALIGN_DWORD(x) ALIGN_OF(x,4)
#endif

#ifndef ALIGN_BUFFER_SIZE
#define ALIGN_BUFFER_SIZE(x,Z) ((x)<(Z)?(Z):(Z)*((x)/(Z)+((x)%(Z)?1:0)))
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#endif
