/*
$Id$

gcc printf_fork_printf.c -o printf_fork_printf && ./printf_fork_printf

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	printf("Hello... ");
	//write(STDOUT_FILENO,"Hello... ",9);
	fork();
	printf("Bye!");
	//write(STDOUT_FILENO,"Buy!",4);
	exit(0);
	//_exit(0);
}

