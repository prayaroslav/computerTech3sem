#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s filename text-to-dprintf\n", argv[0]);
		return 1;
	}

	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd < 0) {
		perror("Failed to open file for dprintf");
		return 2;
	}
	
	if(dprintf(fd, "%s",argv[2])) {
		perror("Operation status");
	}
	
	if(close(fd) < 0) {
		perror("Failure white closing fd");
		return 4;
	}
	
	
	return 0;
}
