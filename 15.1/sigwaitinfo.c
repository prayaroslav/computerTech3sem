#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(void)
{
	sigset_t set;
	siginfo_t info;	
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, NULL);
	while(1)
	{
		if (sigwaitinfo(&set, &info) == -1)
		{
			perror("sigwaitinfo");
			return -1;
		}
		switch(info.si_code)
		{
			case SI_KERNEL:
				printf("Signal %d came from kernel\n", info.si_signo);
			break;
			case SI_USER:
				printf("Signal %d came from process %d\n", info.si_signo, info.si_pid);
			break;
			default:
				printf("Signal %d came from unknown source\n", info.si_signo);
			break;
		}
	}

	return 0;
}
