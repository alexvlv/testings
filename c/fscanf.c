/*
$Id$

gcc fscanf.c -o fscanf && ./fscanf

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	unsigned u = ~0;
	FILE *fp = NULL;
	fp = fopen(".ver.txt", "r");
	if(fp) fscanf(fp, "%d", &u);
	fclose(fp);
	printf("=> %d\n",u);
	return 0;
}

