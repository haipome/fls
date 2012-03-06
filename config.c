#include"fls.h"


#define TABLELEN 1024

struct conf
{
	char 	buf[TABLELEN][MAXLINE];
	int 	len;
};
static struct 	conf config;		/*config file contents*/
static int 	conf_status = 0;	/*config module initialization status*/
static char *	confname = "/wb/fls/conf/fls.conf";
static char 	buf_line[MAXLINE];

static int 	limit_stat = 0;
int 		timel1, timel2;			/*freqence limit time in two levels*/
int 		warning_rate;			/*alarm level*/

int readconf(void)
{	
	if (conf_status == 1)
		return TRUE;
	
	char name[MAXLINE];
	sprintf(name, "%s%s", getenv("HOME"), confname);
	
	FILE *fp;
	if ((fp = fopen(name, "r")) == NULL)
		return FALSE;
	
	int lines = 0;
	while (!feof(fp))
	{
		if (fgets(config.buf[lines], MAXLINE, fp) == NULL)
			break;
		++lines;
		if (lines >= TABLELEN)
			return FALSE;
	}
	config.len = lines;
	conf_status = 1;
	fclose(fp);
	
	return TRUE;
}

int writeconf(void)
{
	if (conf_status == 0)
		return FALSE;
	
	char name[MAXLINE];
	sprintf(name, "%s%s", getenv("HOME"), confname);
	
	FILE *fp;
	if ((fp = fopen(name, "w")) == NULL)
		return FALSE;
	
	int lines = 0;
	while (lines < config.len)
	{
		if((fputs(config.buf[lines], fp)) == EOF)
			return FALSE;
		++lines;
	}
	fclose(fp);
	
	return TRUE;
}

static char * first_match(const char *head)
{
	if (NULL == head)
		return NULL;
	
	int i;
	for (i = 0; i < config.len; ++i)
	{
		if (!strncmp(config.buf[i], head, strlen(head)))
			return config.buf[i];
	}
	
	return NULL;
}

long getconf1(const char *name)
{
	if (NULL == name)
		return FALSE;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *line;
	if ((line = first_match(name)) == NULL)
		return FALSE;
	line += strlen(name);
	
	while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
	if (strlen(line) == 0)
		return FALSE;
	
	return (atol(line));
}

long getconf2(const char *name, int key)
{
	if (NULL == name)
		return FALSE;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *line;
	int i, j;
	char num[MAXLINE];
	for (i = 0; i < config.len; ++i)
	{
		if (!strncmp(config.buf[i], name, strlen(name)))
		{
			line = config.buf[i];	
			line += strlen(name);
			
			while ( *line == ' ' || *line == '\t') ++line;
			
			j = 0;
			while (isdigit(*line))
			{
				num[j++] = *line;
				++line;
			}
			num[j] = '\0';
			while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
			if (atoi(num) == key)
				return (atol(line));
		}
	}
	
	return FALSE;
}

char * getconfs(const char *name)
{
	if (NULL == name)
		return FALSE;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *line;
	if ((line = first_match(name)) == NULL)
		return NULL;
	line += strlen(name);
	
	while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
	if (strlen(line) == 0)
		return NULL;
	
	strncpy(buf_line, line, MAXLINE - 1);
	char *temp = buf_line;
	while (*temp != '\n' && *temp != '\0') ++temp;
	*temp = '\0';
	
	return buf_line;
}

int setconf1(const char *name, long value)
{
	if (NULL == name)
		return FALSE;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *line;
	if ((line = first_match(name)) == NULL)
		return FALSE;
	line += strlen(name);
	
	while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
	sprintf(line, "%ld\n", value);
	
	if (!writeconf())
		return FALSE;
	
	return TRUE;
}

int setconf2(const char *name, int key, long value)
{
	if (NULL == name)
		return FALSE;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *line;
	int i, j;
	char num[MAXLINE];
	for (i = 0; i < config.len; ++i)
	{
		if (!strncmp(config.buf[i], name, strlen(name)))
		{
			line = config.buf[i];	
			line += strlen(name);
			
			while ( *line == ' ' || *line == '\t') ++line;
			
			j = 0;
			
			while (isdigit(*line))
			{
				num[j++] = *line;
				++line;
			}
			num[j] = '\0';
			
			while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
			
			if (atoi(num) == key)
			{
				sprintf(line, "%ld\n", value);
				
				if (!writeconf())
					return FALSE;
	
				return TRUE;
			}
		}
	}
	
	return FALSE;
}


void limit_init(void)
{
	if (limit_stat == 1)
		return;
	
	if (conf_status == 0)
	{
		readconf();
	}
	
	char *name = "serverlimit";
	char *line;
	int sid, temp[2];
	
	int i, j = 0;
	for (i = 0; i < config.len; ++i)
	{
		if (!strncmp(config.buf[i], name, strlen(name)))
		{
			line = config.buf[i];	
			line += strlen(name);
			while ( *line == ' '|| *line == '\t') ++line;
			
			sscanf(line, "%d = %d %d", &sid, temp, temp + 1);
			
			addto_limd(sid, temp[0], temp[1]);
			++j;
		}
	}
	if (j == 0)
		err_exit("In limit_init: No limit data in config");
	
	name = "timelevel";
	if ((line = first_match(name)) == NULL)
		err_exit("In limit_init: can not find %s in config file",name);
	line += strlen(name);
	while ( *line == ' ' || *line == '=' ||*line == '\t') ++line;
	sscanf(line, "%d %d", &timel1, &timel2);
	if (timel1 == 0 || timel2 == 0)
		err_exit("In limit_init: illegal timelevel");
	
	if ((warning_rate = getconf1("warningrate")) == 0)
		err_exit("In limit_init: warningrate is illegal");
	
	limit_stat = 1;
	
	return;
}

