// Mon 19 Oct 2026 12:55:07

#ifndef macro_h_
#define macro_h_

#ifndef str
#define str(s)  #s
#endif

#ifndef xstr
#define xstr(s) str(s)
#endif

/* only for userspace compatibility */
#ifndef __KERNEL__
#include <limits.h> /* CHAR_BIT */
#else
#ifndef CHAR_BIT
#define CHAR_BIT 8	/* Normally in <limits.h> */
#endif
#endif // __KERNEL__

#define BIT_MASK_RIGHT(__TYPE__, __ONE_COUNT__) \
	((__ONE_COUNT__) == 0 ? (__TYPE__)0 : \
	(__ONE_COUNT__) >= sizeof(__TYPE__) * CHAR_BIT ? (__TYPE__)-1 : \
	((__TYPE__)-1 >> (sizeof(__TYPE__) * CHAR_BIT - (__ONE_COUNT__))))
/*
#define BIT_MASK_RIGHT(__TYPE__, __ONE_COUNT__) \
	((__TYPE__) (-((__ONE_COUNT__) != 0))) \
	& (((__TYPE__) -1) >> ((sizeof(__TYPE__) * CHAR_BIT) - (__ONE_COUNT__)))
*/
/* https://stackoverflow.com/questions/1392059/algorithm-to-generate-bit-mask */

#define BIT_MASK_UNSIGNED(__TYPE__) ((__TYPE__)-1)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef SIGN
#define SIGN(X) ((X > 0) - (X < 0))
#endif


#ifndef CEIL_DIV
/* Return the smallest multiple N of y such that: x <= y * N */
#define CEIL_DIV(A, B) ((A) / (B) + ((A) % (B) != 0))
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP CEIL_DIV
#endif

#ifndef ROUND_DIV
#define ROUND_DIV(A, B) ((A) / (B) + ((A) % (B) >= ((B) + 1) / 2))
#endif

/* Align value to the specified boundary (sz must be a power of two). */
#ifndef ALIGN_OF
#define ALIGN_OF(x, sz) (((x) + ((sz) - 1)) & ~((sz) - 1))
#endif

/* Align value to 2-byte boundary. */
#ifndef ALIGN_SHORT
#define ALIGN_SHORT(x) ALIGN_OF(x,2)
#endif

/* Align value to 4-byte boundary. */
#ifndef ALIGN_DWORD
#define ALIGN_DWORD(x) ALIGN_OF(x,4)
#endif

/* Align buffer size to a multiple of Z, with minimum size Z. */
#ifndef ALIGN_BUFFER_SIZE
#define ALIGN_BUFFER_SIZE(x,Z) ((x)<(Z)?(Z):(Z)*((x)/(Z)+((x)%(Z)?1:0)))
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define BUILD_BUG_ON_(cond, line) \
	typedef char build_bug_on_##line[(cond) ? -1 : 1] __attribute__((unused))

#ifndef BUILD_BUG_ON
//#define BUILD_BUG_ON(cond) typedef char build_bug_on[(cond) ? -1 : 1]
#define BUILD_BUG_ON(cond) BUILD_BUG_ON_(cond, __LINE__)
#endif
#endif // macro_h_
