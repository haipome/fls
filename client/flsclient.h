#ifndef _FLS_CLIENT_H
#define _FLS_CLIENT_H

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
	
	char 		reserve[8];
};
extern int flsc_freq_query(uint16_t sid, uint32_t uid, struct flsc_freqdata * freqd);

extern int flsc_error;
#define __FLSC_N_INIT     11
#define __SOCK_OPEN_ERR   12
#define __SOCK_BIND_ERR   13
#define __WAIT_TIME_OUT   14
#define __ILLEGAL_INPUT   15
#define __QUERY_ERR       16
#define __UNKNOWN_CMD     17
#define __SELECT_ERR      18
#define __RFLSL_TOO_LONG  19

#endif
