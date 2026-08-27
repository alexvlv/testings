//gcc fork_demo.c -o fork_demo && ./fork_demo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	int value = 10;
	int *i = &value;
	char *p = malloc(100);
	int fd = open("test", O_WRONLY | O_CREAT | O_TRUNC, 0644);

	sprintf(p, "Hello");

	printf("%s-%d ", p, *i);
	write(fd, p, strlen(p));

	if(fork())	
		(*i)++;
	
	sprintf(p, "World!");
	printf("%s-%d ", p, value);
	write(fd, p, strlen(p));

	free(p);
}
