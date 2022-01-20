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

ssize_t writeall(int fd, const void *buf, size_t count)
{
	size_t bytes_written = 0;
	const uint8_t *buf_addr = buf;
	while (bytes_written < count) 
	{
		ssize_t res = write(fd, buf_addr + bytes_written, count - bytes_written);
		if(res < 0) 
		{
			return res;
			
		}
		bytes_written += (size_t)res;
	}
	return (ssize_t)bytes_written;
}

int main(int argc, char *argv[])
{
	const size_t block_size = 1024 * 1024;
	char* copy_name;
	int fd0, fd1;
	int bytes_read = 1;
	char buffer[block_size];
	struct stat sb;

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
		fprintf(stderr, "I can`t copy such kind of files\n");
		exit(EXIT_FAILURE);
	}
		
	fd0 = open(argv[1], O_RDONLY, 0644);					 
	if(fd0 < 0)
	{
		perror("Failed to open file1");
		exit(EXIT_FAILURE);
	}
	
	fd1 = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);		
	if(fd1 < 0)
	{
		perror("Failed to create file2");
		exit(EXIT_FAILURE);
	}
	
	while (bytes_read != 0)							
	{
		bytes_read = read(fd0, buffer, block_size);				
		if(bytes_read == -1)							
		{
			perror("Error of reading");
			exit(EXIT_FAILURE);
		}	
		
		if(writeall(fd1, buffer, bytes_read) < 0) 				
		{
			perror("Error of writing");
			exit(EXIT_FAILURE);
		}						
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
