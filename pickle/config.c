#include"pickle.h"


static struct 	conf config;
static int 	conf_status = 0;
static char *	confname = "/wb/fls/conf/fls.conf";

int readconf(void)
{	
	if (conf_status == 1)
		return TRUE;
	
	char name[MAXLINE];
	strncpy(name, getenv("HOME"), MAXLINE -1);
	strncat(name, confname, strlen(confname));
	
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
	fclose(fp);
	
	conf_status = 1;
	
	return TRUE;
}

static char * first_match(const char *head)
{
	int i;
	for (i = 0; i < config.len; ++i)
	{
		if (!strncmp(config.buf[i], head, strlen(head)))
			return config.buf[i];
	}
	
	return NULL;
}

int getconf1(const char *name)
{
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
	
	return (atoi(line));
}

int getconf2(const char *name, int key)
{
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
				return (atoi(line));
		}
	}
	
	return FALSE;
}

