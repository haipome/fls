#include"fls.h"


void fls_init(void)
{
	signal_init();
	log_init();
	
	if (!readconf())
		err_exit("read config file error");
	
	net_init();
	shamem_init();
	limit_init();
	manage_fls_init();
	
	if (manage_switch)
		read_rflsl();
	if (!inform_start())
		err_exit("In fls_init: cannot connect to manage server");
	
	pid_conf();
	
	run_msg("started.");
	
	return;
}

void fls_close(int sig)
{
	extern int fls_switch;
	
	fls_switch = 0;
}

void fls_exit(void)
{
	run_msg("closed.");
	
	if (!inform_close())
		run_msg("In fls_exit: cannot connect to manage fls")
		;
	
	log_close();
	
	exit(0);
}

