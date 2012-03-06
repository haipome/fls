#ifndef __FLS_PICKLE_
#define __FLS_PICKLE_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#include<sys/shm.h>

#define TRUE  1
#define FALSE 0

#define SERV_PROC_NUM 9

#define DUMPER 1
#define RELOAD 2

#define MAXLINE  1024
#define TABLELEN 1024

struct conf
{
	char 	buf[TABLELEN][MAXLINE];
	int 	len;
};

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#pragma pack(1)
typedef struct
{
	uint8_t 	sign;
	uint8_t 	version;
	uint16_t 	sid;
	uint32_t 	uid;
	time_t 		timel1;
	time_t 		timel2;
	uint32_t	counterl1;
	uint32_t 	counterl2;
	uint32_t	allowedl2;  /*! allow counter of level 2*/
} recunit;
#pragma pack()

extern int __id_offset;

/*config.c*/
extern int readconf(void);
extern int getconf1(const char *name);
extern int getconf2(const char *name, int key);

/*dumper.c*/
extern int dumper(void);

/*reload.c*/
extern int reload(void);

/*name.c*/
extern char * getname(void);

#endif
