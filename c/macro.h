/**************************************************************************
$Id$
***************************************************************************/

#ifndef macro_h_
#define macro_h_

#define xstr(s)   str(s)
#define str(s)	  #s


#include <limits.h>     /* CHAR_BIT */

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

#ifndef MATH_DIV
#define MATH_DIV(a, b) (((a)+(a)%(b))/(b))
#endif

#ifndef DIV_ROUND_CLOSEST
#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) == ((d) < 0)) ? (((n) + (d)/2)/(d)) : (((n) - (d)/2)/(d)))
#endif

#ifndef CEIL_DIV
#define CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
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

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#endif
