#ifndef __FLS_CLIENT_H_
#define __FLS_CLIENT_H_


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>

#define TRUE 1
#define FALSE 0
#define ERROR -1

#define REPEAT_ATTEM 3
#define MAXLINE 1000

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

extern int flsc_init(struct sockaddr_in local, struct sockaddr_in server);
extern int flsc_stat_query(uint16_t sid, uint32_t uid, uint16_t num);

struct flsc_freqdata
{
	uint32_t	timel1;
	uint32_t	limdl1;
	uint32_t	counterl1;
	uint32_t	timel2;
	uint32_t	limdl2;
	uint32_t	counterl2;
};

extern int flsc_freq_query(uint16_t sid, uint32_t uid, struct flsc_freqdata * freqd);
extern int flsc_error;

#define FLSCNINIT    11
#define SOCKOPENERR  12
#define SOCKBINDERR  13
#define WAITTIMEOUT  14
#define ILLEGALINPUT 15
#define QUERYERR     16
#define UNKNOWNCMD   17
#define SELECTERR    18
#define RFLSLTOOLONG 19


#define P_START_FLAG	0X01	/*protocol start flag*/
#define P_END_FLAG	0X10	/*protocol end flag*/

#define ACC_STAT_ASK 	1	/*subcmd 1, access status query*/
#define ACC_STAT_REP 	2	/*subcmd 2, access status reply*/
#define FREQ_DATA_ASK 	3 	/*subcmd 3, freqence data query*/
#define FREQ_DATA_REP 	4 	/*subcmd 4, freqence data reply*/

#define FLS_LIST_REP	6	/*SUBCMD 6, fls list data reply*/
#define FLS_STAT_BC	7	/*subcmd 7, fls status broadcast*/
#define FLS_STAT_REP	8	/*subcmd 8, confirm and operation reply*/

#define USED_FLAG 	1
#define UNIT_VERSION 	1
#define PROT_VERSION 	1
#define SPECIALUID 	1
#define CMD_OFFSET 	4
#define UID_OFFSET 	8

#define FLS_NUM_MAX 	255

#define DELAY_SYNC 	5
#define DELAY_TIME 	5
#define REPEAT_ATTEM 	3
#define DELAY_MAX 	30
#define TIMEOUT 	10000

typedef struct
{
	uint8_t		id;
	uint8_t		stat;
	in_port_t	port;
	in_addr_t	ip;
} addr;
extern addr localaddr;

typedef struct
{
	addr		server;
	int		delaycount;
	int		delaytime;
	time_t		delaystart;
} flsunit;

typedef struct
{
	
	flsunit list[FLS_NUM_MAX];
	int	len;
} runflslist;

#pragma pack(1)
typedef struct
{
	char		psf;
	char		zero1;
	uint16_t	len;
	uint16_t	subcmd;
	uint16_t	version;
	uint32_t  	uid;
	uint16_t	fls_id;
	uint16_t	fls_num;
	in_addr_t	client_ip;
	in_port_t	client_port;
	char		zero2[2];

	char		reserve[40];
} phead;

typedef struct
{
	uint32_t	uid;
	uint16_t	sid;
	uint16_t	num;
} pbody1;

typedef struct
{
	phead		head;
	pbody1		body;

	char		zero[3];
	char		pef;
} netpt1;

typedef struct
{
	uint8_t		stat;
	uint8_t		result;
	uint16_t	sid;
	uint32_t	uid;
} pbody2;

typedef struct
{
	phead		head;
	pbody2		body;

	char		zero[3];
	char		pef;
} netpt2;

typedef struct
{
	uint32_t	uid;
	uint16_t	sid;
	char		zero[2];
} pbody3;

typedef struct
{
	phead		head;
	pbody3		body;

	char		zero[3];
	char		pef;
} netpt3;

typedef struct
{
	uint8_t		stat;
	char		zero;
	uint16_t	sid;
	uint32_t	uid;

	struct flsc_freqdata	freqd;
} pbody4;

typedef struct
{
	phead		head;
	pbody4		body;

	char		zero[3];
	char		pef;
} netpt4;

typedef struct
{
	uint16_t	num;
	uint16_t	sid;
	uint32_t	uid;

	addr		list[FLS_NUM_MAX];
} pbody6;

typedef struct
{
	phead		head;
	pbody6		body;

	char		zero[3];
	char		pef;
} netpt6;

#define FLS_START	1
#define FLS_CLOSE	2
#define FLS_DISCONN	3
#define FLS_RUNING	4

typedef struct
{
	uint8_t		stat;
	char		zero;
	uint16_t	sid;
	addr		fls;
} pbody7;

typedef struct
{
	phead		head;
	pbody7		body;
	
	char		zero[3];
	char		pef;
} netpt7;

#define DO_NOTHING	1
#define ADDTO_RLIST	2
#define REMOVE_RLIST	3

typedef struct
{
	uint8_t		stat;
	uint8_t		op;
	char		zero[2];
} pbody8;

typedef struct
{
	phead		head;
	pbody8		body;

	char		zero[3];
	char		pef;
} netpt8;
#pragma pack()

#endif

