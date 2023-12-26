/*
$Id$

gcc ceil_div.c -o ceil_div && ./ceil_div

*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "macro.h"

#ifdef CEIL_DIV
#undef CEIL_DIV
#endif

#ifdef ROUND_DIV
#undef ROUND_DIV
#endif


#define CEIL_DIV(A, B)  (((A) + (B) - 1) / (B))
#define ROUND_DIV(A, B) (((A) + (A)%(B)) / (B))

struct pair_t {
	int d;
	int r;
};

struct pair_t pairs[] = {
 {0,1},
 {8,4},
 {9,3},
 {10,5},
 {17,5},
 {55,8},
 {60,30},
 {77,17},
 {99,90},
 {9,5},
 {9,6},
 {9,7},
 {90,61},
};

int main(void)
{
	INFO("ceil div test\n"
		"$Id$\n"
		"Compiled: " __DATE__ " " __TIME__ );

	int i, l, m;
	float f;

	for (i=0;i<ARRAY_SIZE(pairs);i++) {
		struct pair_t pair = pairs[i];
		f = (float)pair.d/pair.r;
		l = CEIL_DIV(pair.d,pair.r);
		m = ROUND_DIV(pair.d,pair.r);
		INFO("%2d/%d:\t%0.2f %d %d %c",pair.d,pair.r,f,l,m,l!=m?'!':' ');
	}

	return 0;
}

/*
How DIV_ROUND_UP works
https://medium.com/@arunistime/how-div-round-up-works-179f1a2113b5

In linux kernel:
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
https://elixir.bootlin.com/linux/v4.5/source/include/linux/kernel.h#L67
 
*/
