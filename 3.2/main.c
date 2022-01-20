#define BLOCK_SIZE 4

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char* copy_name;
	int fd0, fd1;
	size_t bytes_read = 1;
	char buffer[BLOCK_SIZE];
	struct stat sb;
	off_t offset = 0;

	if (argc != 3)								
	{
		fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
		exit(EXIT_FAILURE); 
	}
	copy_name = argv[2];
	
	if(lstat(argv[1], &sb) == -1)						
	{
		perror("lstat");
		exit(EXIT_FAILURE);
	}
	
	if(S_ISREG( sb.st_mode ) == 0)
	{
		fprintf(stderr, "Can`t copy such kind of files\n");
		exit(EXIT_FAILURE);
	}
		
	fd0 = open(argv[1], O_RDONLY, 0644);					
	if(fd0 < 0)
	{
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}
	
	fd1 = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);		
	if(fd1 < 0)
	{
		perror("Failed to create file");
		exit(EXIT_FAILURE);
	}

	offset = 0;
	
	
	bytes_read = pread(fd0, buffer, BLOCK_SIZE, offset);			
	
	while (bytes_read > 0 )
	{
		if(bytes_read < 0)							
		{
			perror("Error of preading");
			exit(EXIT_FAILURE);
		}
		if(pwrite(fd1, buffer, bytes_read, offset) < 0)
		{
			perror("Error of pwriting");
			exit(EXIT_FAILURE);
		}
		offset = (off_t) (offset + bytes_read);
		bytes_read = pread(fd0, buffer, sizeof(buffer), offset);	
	}
	
	if(close(fd0) < 0) {		
		perror("Failure white closing fd-open");
		exit(EXIT_FAILURE);
	}
	
	if(close(fd1) < 0) {							
		perror("Failure white closing fd-create");
		exit(EXIT_FAILURE);
	}
	printf("Done!\n");
	return 0;
}
