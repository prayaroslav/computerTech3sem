#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

volatile int g_last_signal;

void sig_handler(int signum)
{
	g_last_signal = signum;
}

void proc_info(const char* procname)
{
    printf("[%s]:\n"
           "PID %d,\n"
           "PPID %d,\n"
           "PGID %d,\n"
           "SID %d\n\n", procname, getpid(), getppid(), getpgid(0), getsid(0));
}

char * sig_name(int signum)
{
	switch(signum)
	{
		case SIGINT: return "SIGINT";
		case SIGQUIT: return "SIGQUIT";
		case SIGTSTP: return "SIGTSTP";
		case SIGHUP: return "SIGHUP";
		case SIGTERM: return "SIGTERM";
	}
	return "UNKNOWN";
}

int main(int argc, char * argv[])
{
	proc_info("Current process");

	struct sigaction act;
	act.sa_handler = sig_handler;
	act.sa_flags = 0;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTSTP);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGINT);
	act.sa_mask = set;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	while(1)
	{
		pause();
		printf("\nSignal %s came\n", sig_name(g_last_signal));
	}
	return 0;
}
