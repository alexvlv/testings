//gcc fork_demo.c -o fork_demo && ./fork_demo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	char *p = malloc(100);
	int fd = open("test", O_WRONLY | O_CREAT | O_TRUNC, 0644);

	sprintf(p, "Hello");

	printf("%s", p);
	write(fd, p, strlen(p));

	fork();

	sprintf(p, "World!");
	printf("%s", p);
	write(fd, p, strlen(p));

	free(p);
}
