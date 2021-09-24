/*
$Id$

gcc buffer_align.c -o buffer_align && ./buffer_align

*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "dbg.h"
#include "macro.h"

#define MTD_PAGE_SZ 1056

#define ALIGN_BUFFER_SIZE(x,Z) ((x)<(Z)?(Z):(Z)*((x)/(Z)+((x)%(Z)?1:0)))

int main(void)
{
	INFO("Buffer size alignment test\n"
		"$Id$\n"
		"Compiled: " __DATE__ " " __TIME__ );
	
	//int sza = sz<MTD_PAGE_SZ?MTD_PAGE_SZ:MTD_PAGE_SZ*(sz/MTD_PAGE_SZ+(sz%MTD_PAGE_SZ?1:0));
		
	int sz, sza, rest, i;
	
	int sizes[] = { 
		MTD_PAGE_SZ-512, 
		MTD_PAGE_SZ, 
		MTD_PAGE_SZ+16, 
		MTD_PAGE_SZ*2, 
		MTD_PAGE_SZ*2+256, 
		MTD_PAGE_SZ*4, 
		MTD_PAGE_SZ*7+15};
	
	for (i=0;i<ARRAY_SIZE(sizes);i++) {
		sz = sizes[i];
		sza = ALIGN_BUFFER_SIZE(sz,MTD_PAGE_SZ);
		rest = sza - sz;
		INFO("%d: %d=>%d rest:%d",MTD_PAGE_SZ,sz,sza,rest);
	}

	return 0;
}

/*
 https://stackoverflow.com/questions/4840410/how-to-align-a-pointer-in-c

*/