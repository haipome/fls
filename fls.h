#ifndef __FLS_SERVER_H_
#define __FLS_SERVER_H_


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<stdarg.h>
#include<time.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/shm.h>
#include<math.h>

#define TRUE   1
#define FALSE  0
#define ERROR  -1
#define MAXLINE 1024

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
/*sizeof(recunit) == 36*/

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
#define CMD_OFFSET 	4
#define UID_OFFSET	8
#define SPECIALUID 	1
#define SPECIALSID	0

#define MEM_RATE_TIME 	10
#define FLS_ID_MAX 	255
#define FLS_NUM_MAX	128
#define CLIENT_MAX 	1000
#define SERV_PROC_NUM	9

extern int sd;

typedef struct
{
	uint8_t		id;
	uint8_t		zero;
	in_port_t	port;
	in_addr_t	ip;
} addr;
extern addr localaddr;
extern addr manage_fls_addr;
extern int  manage_switch;

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
extern runflslist rflsl;

struct limit_unit
{
	uint16_t sid;
	uint32_t limit[2];
};
struct limit_data
{
	struct limit_unit list[CLIENT_MAX];
	int               len;
};

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

struct freqdata
{
	uint32_t	timel1;
	uint32_t	limdl1;
	uint32_t	counterl1;
	uint32_t	timel2;
	uint32_t	limdl2;
	uint32_t	counterl2;
};

typedef struct
{
	uint8_t		stat;
	char		zero;
	uint16_t	sid;
	uint32_t	uid;

	struct freqdata	freqd;
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
/*sizeof(netpt6 = 1100)*/

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

/*log.c*/
extern void  log_init(void);

#define FLS_LOG_GENE 1
#define FLS_LOG_FREQ 2
#define FLS_LOG_OVER 3
#define FLS_LOG_WARN 4
extern int   addto_log(int type, const char *fmt, ...);
extern int   log_close(void);

/*erroe.c*/
extern void err_exit(const char *fmt, ...);
extern void run_msg(const char *fmt, ...);

/*config.c*/
extern int    readconf(void);
extern int    writeconf(void);
extern long   getconf1(const char *name);
extern long   getconf2(const char *name, int key);
extern char * getconfs(const char *name);
extern int    setconf1(const char *name, long value);
extern int    setconf2(const char *name, int key, long value);
extern void   limit_init(void);

/*memory.c*/
extern void      shamem_init(void);
extern recunit * hash_search(uint16_t sid, uint32_t uid, time_t now);
extern void      memuse_rate(void);

/*accque.c*/
extern uint8_t access_query(const netpt1 *p, uint8_t * stat, time_t now);

/*flsmain.c*/
extern void fls_main(void);

/*flsinit.c*/
extern void fls_init(void);
extern void fls_close(int sig);
extern void fls_exit(void);

/*net.c*/
extern void   net_init(void);
extern void * read_net(void);

extern void   ntohpt1(netpt1 *ptr);
extern void   htonpt2(netpt2 *ptr);
extern void   ntohpt3(netpt3 *ptr);
extern void   ntohpt7(netpt7 *ptr);

extern void   htonpt4(netpt4 *ptr);
extern void   htonpt7(netpt7 *ptr);
extern void   htonpt8(netpt8 *ptr);
extern void   htonpt6(netpt6 *ptr);

extern void   send_net2(const netpt1 *ptr, uint8_t stat, uint8_t result);
extern void   send_net4(const netpt3 *ptr, uint8_t stat, recunit *unit);
extern void   send_net6(const netpt1 *ptr);
extern void   send_net7(const addr   *client, uint8_t stat);
extern void   send_net8(const netpt7 *ptr, uint8_t stat, uint8_t op);

/*signal.c*/
extern void signal_init(void);
extern void send_exit_signal(void);
extern void pid_conf(void);
extern void pid_print(void);

/*sync.c*/
extern int  read_rflsl(void);
extern int  addto_rflsl(addr *ptr);
extern int  remove_rflsl(addr *ptr);
extern int  inform_start(void);
extern int  inform_close(void);
extern void sync_start(void);
extern void sync_close(void);
extern void manage_fls_init(void);

/*limit.c*/
extern int addto_limd(uint16_t sid, uint32_t lim1, uint32_t lim2);
extern struct limit_unit * search_limd(uint16_t sid);
extern int get_limd(const uint16_t sid, uint32_t *lim1, uint32_t *lim2);

#endif

