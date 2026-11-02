#include "macro.h"
#include "dbg.h"
#include ".git.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------
void test_macros(void)
{
	fprintf(stderr, "Macro tests begin ...\n" );
	/* Stringification */
	assert(strcmp(str(123), "123") == 0);
	assert(strcmp(xstr(123), "123") == 0);

	/* BIT_MASK_RIGHT */
	assert(BIT_MASK_RIGHT(uint8_t, 0) == 0);
	assert(BIT_MASK_RIGHT(uint32_t, 8) == 0xFF);
	assert(BIT_MASK_RIGHT(uint32_t, 12) == 0xFFF);
	assert(BIT_MASK_RIGHT(uint32_t, 32) == UINT32_MAX);

	/* BIT_MASK_UNSIGNED */
	assert(BIT_MASK_UNSIGNED(uint8_t) == UINT8_MAX);

	/* ARRAY_SIZE */
	int arr[5];
	assert(ARRAY_SIZE(arr) == 5);

	/* SIGN */
	assert(SIGN(-10) == -1);
	assert(SIGN(0) == 0);
	assert(SIGN(10) == 1);

	/* CEIL_DIV / DIV_ROUND_UP */
	assert(CEIL_DIV(10, 3) == 4);
	assert(DIV_ROUND_UP(10, 3) == 4);
	assert(CEIL_DIV(12, 3) == 4);

	/* ROUND_DIV */
	assert(ROUND_DIV(10, 3) == 3);
	assert(ROUND_DIV(8, 3) == 3);
	assert(ROUND_DIV(7, 3) == 2);

	/* ALIGN_OF */
	assert(ALIGN_OF(10, 4) == 12);
	assert(ALIGN_OF(12, 4) == 12);
	assert(ALIGN_OF(13, 4) == 16);

	/* ALIGN_SHORT */
	assert(ALIGN_SHORT(3) == 4);
	assert(ALIGN_SHORT(4) == 4);
	assert(ALIGN_SHORT(5) == 6);

	/* ALIGN_DWORD */
	assert(ALIGN_DWORD(5) == 8);
	assert(ALIGN_DWORD(8) == 8);
	assert(ALIGN_DWORD(9) == 12);

	/* ALIGN_BUFFER_SIZE */
	assert(ALIGN_BUFFER_SIZE(10, 8) == 16);
	assert(ALIGN_BUFFER_SIZE(16, 8) == 16);
	assert(ALIGN_BUFFER_SIZE(1, 8) == 8);

	/* UNUSED */
	UNUSED(arr[0]);

	/* BUILD_BUG_ON */
	BUILD_BUG_ON(sizeof(uint32_t) != 4);
	fprintf(stderr, "Macro tests done!\n" );
}
//-------------------------------------------------------------------------
int main(int argc, char **argv)
{
	INFO("C app skeleton, GIT Rev." GIT_REVISION " AlexVol [" __DATE__ " " __TIME__ "] (pid %d)*", getpid());
	test_macros();
	return EXIT_SUCCESS;
}
//-------------------------------------------------------------------------
