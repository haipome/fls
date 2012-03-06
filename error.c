#include"fls.h"


void err_exit(const char *fmt, ...)
{
	char buf[MAXLINE];
	va_list ap;
	
	va_start(ap,fmt);
	vsnprintf(buf, MAXLINE, fmt, ap);
	va_end(ap);
	
	fputs(buf, stderr);
	putc('\n', stderr);
	fflush(NULL);
	addto_log(FLS_LOG_GENE, buf);
	
	exit(1);
}

void run_msg(const char *fmt, ...)
{
	char buf[MAXLINE];
	va_list ap;
	
	va_start(ap,fmt);
	vsnprintf(buf, MAXLINE, fmt, ap);
	va_end(ap);
	
	extern int __msg_stdout_switch;
	if (__msg_stdout_switch)
	{
		fputs(buf, stdout);
		putc('\n', stdout);
		fflush(NULL);
	}
	addto_log(FLS_LOG_GENE, buf);
}

