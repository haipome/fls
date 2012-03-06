#include"pickle.h"

static char * location = "/wb/fls/data/";
static char * head     = "flsmemory";
static char * tail     = ".backup";

static char   name[MAXLINE];

char * getname(void)
{
	sprintf(name, "%s%s%s%d%s", getenv("HOME"), location, head, __id_offset, tail);
	
	return name; 
}

