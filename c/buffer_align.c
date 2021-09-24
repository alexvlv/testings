/*
$Id$

gcc buffer_align.c -o buffer_align && ./buffer_align

*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "dbg.h"

#define MTD_PAGE_SZ 1056

#define ALIGN_BUFFER_SIZE(x,Z) ((x)<(Z)?(Z):(Z)*((x)/(Z)+((x)/(Z)?1:0)))

int main(void)
{
	INFO("Buffer size alignment test\n"
		"$Id\n"
		"Compiled: " __DATE__ " " __TIME__ );
	
	int sz = 1560;
	//int sza = sz<MTD_PAGE_SZ?MTD_PAGE_SZ:MTD_PAGE_SZ*(sz/MTD_PAGE_SZ+(sz%MTD_PAGE_SZ?1:0));
	int sza = ALIGN_BUFFER_SIZE(sz,MTD_PAGE_SZ);
	INFO("%d: %d=>%d",MTD_PAGE_SZ,sz,sza);

	return 0;
}

/*
 https://stackoverflow.com/questions/4840410/how-to-align-a-pointer-in-c

*/