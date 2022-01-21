#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>	

char dtype_char(unsigned dtype)
{
	switch(dtype)
	{
		case DT_BLK: return 'b';
		case DT_CHR: return 'c';
		case DT_DIR: return 'd';
		case DT_FIFO: return 'f';
		case DT_LNK: return 'l';
		case DT_REG: return 'r';
		case DT_SOCK: return 's';
		case DT_UNKNOWN: return '?';
	}
	return '?';
}

int main(int argc, char * argv[])
{
	char *copyDIR_name;
	char *originalDIR_name;
	if (argc != 3)
	{
		printf("Wrong number of arguments");
		exit(1);
	}
	else
	{
		copyDIR_name = argv[2];
		originalDIR_name = argv[1];
	}
	DIR *dir_fd;
	struct dirent *entry;
	dir_fd = opendir(originalDIR_name);
	if (!dir_fd)
	{
		perror("diropen");
		exit(1);
	}
	while ( (entry = readdir(dir_fd)) != NULL)
	{

		printf("%ld - %s [%c] %d \n", entry->d_ino, entry->d_name, dtype_char(entry->d_type), entry->d_reclen);
	}	
	closedir(dir_fd);
	return 0;
}
