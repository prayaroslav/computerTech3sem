#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>
#include <sys/stat.h>

#define MAX_LEN 1024 /*длина пути для директории*/

int add_watches_to_dir(int fd, char *root) //установка inotify на все директории
{
	int wd;
	char *abs_dir; //название директории
	struct dirent *entry;
	DIR *DIRFD;

	DIRFD = opendir(root); //открытие директории
	if (DIRFD == NULL) //ошибка при открытии
	{
		perror("Error opening the starting directory");
		return 1;
	}

	wd = inotify_add_watch(fd, root, IN_CREATE | IN_MODIFY | IN_DELETE | IN_ATTRIB | IN_OPEN | IN_CLOSE | IN_MOVE); //установка inotify на директорию  
	if (wd == -1) //ошибка
	{
		printf("Couldn't add watch to %s\n",root);
		return -1;
	}
	else
	{
		printf("Watching:: %s\n",root); //успех
	}

	abs_dir = (char *)malloc(MAX_LEN);
	while((entry = readdir(DIRFD))) //выход при прочтении всей директории
	{ 
		if (entry->d_type == DT_DIR && entry->d_name[0] != '.') //если директория и не ../ или ./
		{		
			strcpy(abs_dir,root); // копирование в abs_dir root
			
			if(root[strlen(root) - 1] != '/') //если root не заканчивается на /, то добавить 
				strcat(abs_dir,"/");
			
			strcat(abs_dir,entry->d_name); 

			wd = inotify_add_watch(fd, abs_dir, IN_CREATE | IN_MODIFY | IN_DELETE | IN_ATTRIB | IN_OPEN | IN_CLOSE | IN_MOVE); //установка inotify на найденную директорию
			if (wd == -1) //ошибка
				printf("Couldn't add watch to the directory %s\n",abs_dir);
			else
				printf("Watching:: %s\n",abs_dir); // успех
		}
	}
  
	closedir(DIRFD); //закрытие директории
	free(abs_dir);//освобождение памяти
	return 0;
}

unsigned int BUF_SIZE = 64;

char* get_UTC_time(char* str, const time_t* s_time, unsigned buf_size) 
{
	struct tm *tm_time;

	tm_time = gmtime(s_time);

	if(strftime (str, buf_size, "%x %X (UTC)", tm_time) > 0) 
		return str;

	return NULL;
}

void print_events(int fd, char* argv) 
{

	char time_buf[BUF_SIZE];

	char buf[4096];
	const struct inotify_event *event;
	ssize_t len = 1;
	char *ptr;
	int flag_print = 0; 

	while(len > 0) 
	{

		len = read(fd, buf, sizeof(buf)); //чтение файлового дескриптора
		if (len == -1 && errno != EAGAIN) //ошибка, при временной недоступности дескриптора не выходить
		{
			perror("read");
			return;
		}

		for (ptr = buf; ptr < buf + len; ptr = ptr + sizeof(struct inotify_event) + event->len) 
		{
			event = (const struct inotify_event *) ptr;
			flag_print = 0;
			time_t cur_time = time(NULL);

			printf("%s | ", get_UTC_time(time_buf, &cur_time, BUF_SIZE)); //вывод времени в UTC
            //вывод события
			if (event->mask & IN_OPEN)
			{
				flag_print = 1;
				printf("IN_OPEN:          	");
			}
			if (event->mask & IN_CLOSE_NOWRITE)
			{
				flag_print = 1;
				printf("IN_CLOSE_NOWRITE: 	");
			}
			if (event->mask & IN_CLOSE_WRITE)
			{
				flag_print = 1;
				printf("IN_CLOSE_WRITE:   	");
			}
			if (event->mask & IN_ACCESS)
			{
				flag_print = 1;
				printf("IN_ACCESS:       	");
			}
			if (event->mask & IN_CREATE)
			{
				flag_print = 1;
				printf("IN_CREATE:        	");
			}
			if (event->mask & IN_DELETE)
			{
				flag_print = 1;
				printf("IN_DELETE:        	");
			}
			if (event->mask & IN_MODIFY)
			{
				flag_print = 1;
				printf("IN_MODIFY:        	");
			}
			if (event->mask & IN_ATTRIB)
			{
				flag_print = 1;
				printf("IN_ATTRIB:        	");
			}

			
			if(flag_print == 1) //если событие напечатано
			{
				if (event->len > 0)
					printf("%s", event->name); //имя файла, если событие было для файла
				else
					printf("%s", argv);

				if (event->mask & IN_ISDIR)
					printf(" [dir] \n"); //объект этого события каталог
				else
					printf(" [reg]\n"); //объект этого события файл
			}
		}
	}
}

int main(int argc, char* argv[]) 
{
	int fd, wd, pol;
	nfds_t nfds = 2;
	struct stat buf;

	struct pollfd fds[2];

	if (argc < 2) 
	{
		printf("give me the path"); // не был введен путь
		return 1;
	}

	printf("Press ENTER key to terminate.\n");

	fd = inotify_init1(IN_NONBLOCK); // инициаллизация inotify с флагом
	if (fd == -1) // ошибка при инициаллизации 
	{
		perror("inotify_init1");
		return 1;
	}
	
	if(lstat(argv[1], &buf) < 0) //ошибка при чтении информации файла
	{
		perror("lstat");
		return 1;
	}
	
	if((buf.st_mode & S_IFMT) ==  S_IFDIR) //если директория
	{
	    if(add_watches_to_dir(fd, argv[1]) != 0) //установка inotify на все директории
	    		exit(1); //выход если ошибка
    }	
    else
        printf("Watching:: %s\n", argv[1]);
		wd = inotify_add_watch(fd, argv[1], IN_CREATE | IN_MODIFY | IN_DELETE | IN_ATTRIB | IN_OPEN | IN_CLOSE | IN_MOVE); // установка inotify на файл
		
	if(wd == -1) // ошибка при установке inotify
	{
		perror("whatch");
		return -1;	
	}
    //получение данных от inotify
	fds[0].fd = fd; 
	fds[0].events = POLLIN; 
	//ввод с клавиатуры
	fds[1].fd = STDIN_FILENO; 
   	fds[1].events = POLLIN; 

	printf("waiting for events\n\n");
	
	while (1) //бесконечный цикл с выводом событий
	{

		pol = poll(fds, nfds, -1); //ожидание готовности файлового дескриптора

		if (pol == -1) // ошибка
		{
			perror("poll");
			return -1;
		}

		if (pol > 0) //если фд готов предоставить данные
		{
			if (fds[0].revents & POLLIN) //данные от inotify
				print_events(fd, argv[1]); // печать событий
				
			if (fds[1].revents & POLLIN) //данные с клавиатуры
				break; //выход
		}
	}
	printf("watching for events stopped.\n");

	close(fd);

	return 0;
}
