
#include"fls.h"


#define LOG_NAME_LEN 100

typedef struct
{
	FILE *fp;
	char name[LOG_NAME_LEN];
	char head[LOG_NAME_LEN];
}logfile;

struct logfiles
{
	logfile gene_log;
	logfile freq_log;
	logfile over_log;
	logfile warn_log;
};
static struct logfiles logs;
static int  logs_status = 0;
static char logbuf[MAXLINE];
static char * commtail = ".log";
static char * genehead = "/wb/fls/log/gene/flsd";
static char * freqhead = "/wb/fls/log/freq/freq";
static char * overhead = "/wb/fls/log/over/over";
static char * warnhead = "/wb/fls/log/warn/warn";

static char * add_time_tag(char * buf)
{
	time_t now;
	struct tm *time_sec;
	
	time(&now);
	time_sec = localtime(&now);
	
	strftime(buf, 20, "%H:%M:%S  ", time_sec);
	
	return (buf + strlen(buf));
}

static char * add_id_tag(char * buf)
{
	sprintf(buf, "%3u  ", localaddr.id);
	
	return (buf + strlen(buf));
}

static int name_in_date(const char * head, char * name, const char * tail)
{
	time_t now;
	struct tm * today;
	
	time(&now);
	today = localtime(&now);
	
	strncpy(name, head, LOG_NAME_LEN - 1);
	strftime((name + strlen(head)), 20, "%Y%m%d", today);
	strncat(name, tail, LOG_NAME_LEN - strlen(name) - 1);
	
	return TRUE;
}

static void get_head(char *head, char *name)
{
	sprintf(head, "%s%s", getenv("HOME"), name);
}

void log_init(void)
{
	if (logs_status == 1)
		return;
	
	get_head(logs.gene_log.head, genehead);
	name_in_date(logs.gene_log.head, logs.gene_log.name, commtail);
	if ((logs.gene_log.fp = fopen(logs.gene_log.name, "a+")) == NULL)
	{
		fputs("In log_init: cannot open general log file\n", stderr);
		exit(1);
	}
	
	get_head(logs.freq_log.head, freqhead);
	name_in_date(logs.freq_log.head, logs.freq_log.name, commtail);
	if ((logs.freq_log.fp = fopen(logs.freq_log.name, "a+")) == NULL)
	{
		fputs("In log_init: cannot open freqence log file\n", stderr);
		exit(1);
	}
	
	get_head(logs.over_log.head, overhead);
	name_in_date(logs.over_log.head, logs.over_log.name, commtail);
	if ((logs.over_log.fp = fopen(logs.over_log.name, "a+")) == NULL)
	{
		fputs("In log_init: cannot open over limit log file\n", stderr);
		exit(1);
	}
	
	get_head(logs.warn_log.head, warnhead);
	name_in_date(logs.warn_log.head, logs.warn_log.name, commtail);
	if ((logs.warn_log.fp = fopen(logs.warn_log.name, "a+")) == NULL)
	{
		fputs("In log_init: cannot open warning log file\n", stderr);
		exit(1);
	}
	
	logs_status = 1;
	
	return;
}

static int update_logfp(logfile *log, char * newname)
{
	strcpy(log->name, newname);
	fclose(log->fp);
	if ((log->fp = fopen(log->name, "a+")) == NULL)
	{
		fputs("In update_logfp: cannot open over limit log file\n", stderr);
		return FALSE;
	}
	
	return TRUE;
}

int addto_log(int type, const char *fmt, ...)
{
	if (logs_status == 0)
	{
		log_init();
	}
	
	logfile * lf;
	switch (type)
	{
		case FLS_LOG_GENE:
			lf = &(logs.gene_log);
			break;
		case FLS_LOG_FREQ:
			lf = &(logs.freq_log);
			break;
		case FLS_LOG_OVER:
			lf = &(logs.over_log);
			break;
		case FLS_LOG_WARN:
			lf = &(logs.warn_log);
			break;
		default:
			return FALSE;
	}
	
	char namebuf[MAXLINE];
	name_in_date(lf->head, namebuf, commtail);
	if (strncmp(namebuf, logs.gene_log.name, LOG_NAME_LEN - 1))
		update_logfp(lf, namebuf);
	
	char * buf = add_time_tag(logbuf);
	buf = add_id_tag(buf);
	
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(buf, MAXLINE, fmt, ap);
	va_end(ap);
	
	if((fputs(logbuf,lf->fp)) == EOF)
		return FALSE;
	if((putc('\n',lf->fp)) == EOF)
		return FALSE;
	fflush(NULL);
	
	return TRUE;
}

int log_close(void)
{
	int stat = TRUE;
	
	if (logs_status != 0)
	{
		if (fclose(logs.gene_log.fp) == EOF)
			stat = FALSE;
		if (fclose(logs.freq_log.fp) == EOF)
			stat = FALSE;
		if (fclose(logs.over_log.fp) == EOF)
			stat = FALSE;
		if (fclose(logs.warn_log.fp) == EOF)
			stat = FALSE;
	}
	
	return stat;
}

