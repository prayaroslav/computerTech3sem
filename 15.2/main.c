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
#include <signal.h>

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
			time_t cur_time = time(NULL);

			printf("%s | ", get_UTC_time(time_buf, &cur_time, BUF_SIZE)); //вывод времени в UTC
            		//вывод события
			
			if (event->mask & IN_CREATE)
			{
				printf("New file has been created. File name: ");
				if (event->len > 0)
					printf("%s ", event->name);
				if (event->mask & IN_ISDIR)
					printf("[dir] \n");
				else
					printf("[file] \n");
			}
		}
	}
}

volatile int watch_flag = 1;
int fd, pol, wd;
struct pollfd fds[1];

void sig_handler(int sig)
{
	inotify_rm_watch(fd, wd);
	printf("\n End of watching.\n");
	watch_flag = 0;
	close(fd);
	close(fds[0].fd);
}

int main(int argc, char* argv[]) 
{
	nfds_t nfds = 1;
	struct stat buf;
	struct sigaction act;
	act.sa_handler = sig_handler;
	sigaction(SIGINT, &act, 0);
	sigaction(SIGTERM, &act, 0);
	
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
	
        printf("Watching:: %s\n", argv[1]);
	wd = inotify_add_watch(fd, argv[1], IN_CREATE); // установка inotify на файл
		
	if(wd == -1) // ошибка при установке inotify
	{
		perror("whatch");
		return -1;	
	}
    //получение данных от inotify
	fds[0].fd = fd; 
	fds[0].events = POLLIN;

	printf("waiting for events\n\n");
	
	while (watch_flag) //бесконечный цикл с выводом событий
	{
		pol = poll(fds, nfds, -1); //ожидание готовности файлового дескриптора

		if (pol == -1 && errno != EINTR) // ошибка
		{
			perror("poll");
			return -1;
		}

		if (pol > 0) //если фд готов предоставить данные
		{
			if (fds[0].revents & POLLIN) //данные от inotify
				print_events(fd, argv[1]); // печать событий
		}
	}

        return 0;
}
