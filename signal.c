#include"fls.h"


extern int __id_offset;

void send_exit_signal()
{
	pid_t pid;
	
	pid = (pid_t)getconf2("processid", __id_offset);
	if (pid == 0 || pid == -1)
	{
		fputs("In send_exit_signal: get pid error\n", stderr);
		
		exit(1);
	}
	
	kill(pid, SIGUSR1);
	
	exit(0);
}

void signal_init(void)
{
	if (signal(SIGUSR1, fls_close) == SIG_ERR)
	{
		fputs("In signal_init: cannot catch SIGUSR1\n", stderr);
		
		exit(1);
	}
	
	if (signal(SIGINT, fls_close) == SIG_ERR)
	{
		fputs("In signal_init: cannot catch SIGINT\n", stderr);
		
		exit(1);
	}
	
	if (signal(SIGQUIT, fls_close) == SIG_ERR)
	{
		fputs("In signal_init: cannot catch SIGINT\n", stderr);
		
		exit(1);
	}
}

void pid_conf(void)
{
	readconf();
	
	pid_t pid = getpid();
	
	if (setconf2("processid", __id_offset, pid) == FALSE)
		run_msg("In pid_conf: write pid error");
}

void pid_print(void)
{
	readconf();
	
	printf("%d\n", (int)getconf2("processid", __id_offset));
}

