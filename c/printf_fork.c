/*
$Id$

gcc printf_fork.c -o printf_fork && ./printf_fork

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
	printf("printf: Before fork, pid:%d ...",getpid());
	//fprintf(stdout, "stdout: Before fork...");
	//fprintf(stderr,"stderr: Before fork...");
	//printf("printf: After fork, pid:%d\n",getpid());
	//fprintf(stderr,"stderr: After fork...\n");
	//fprintf(stdout,"stdout: After fork...\n");
	printf("fork:%d pid:%d \n",fork(),getpid());
	return 0;
}

/*
$ gcc printf_fork.c -o printf_fork && ./printf_fork
printf: Before fork, pid:14283 ...fork:14284 pid:14283
printf: Before fork, pid:14283 ...fork:0 pid:14283
*/