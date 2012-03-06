#include"fls.h"


#define TIMEOUT 100000

static struct sockaddr_in cliaddr, servaddr;
static int net_status = 0;
static socklen_t len;

int  sd = -1;
addr localaddr;
char readbuf[MAXLINE];

extern int __id_offset;
extern int __bind_port;

void net_init(void)
{
	if (net_status == 1)
		return;
	
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		err_exit("In net_init: cannot open socket");
	
	int id_temp = getconf1("localflsid");
	if (id_temp <= 0)
		err_exit("In net_init: localflsid is illegal");
	id_temp += __id_offset;
	if (id_temp >= FLS_ID_MAX)
		err_exit("In net_init: localflsid should less than %d", FLS_NUM_MAX);
	localaddr.id = id_temp;
	
	if (__bind_port == 0)
	{
		if ((localaddr.port = (uint16_t)getconf1("localflsport")) == 0)
			err_exit("In net_init: listenport is illegal");
		localaddr.port += (uint16_t)__id_offset;
	}
	else
	{
		localaddr.port = (uint16_t)(__bind_port + __id_offset);
	}
	localaddr.port = htons(localaddr.port);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = localaddr.port;
	
	if (bind(sd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		err_exit("In net_init: cannot bind port %d", ntohs(servaddr.sin_port));
	
	char hostip[MAXLINE];
	if (strncpy(hostip, getconfs("localflsip"), MAXLINE - 1) == 0)
		err_exit("In net_init: get local host ip error");
	inet_aton(hostip, &(servaddr.sin_addr));
	
	localaddr.ip = servaddr.sin_addr.s_addr;
	servaddr.sin_port = localaddr.port;
	
	net_status = 1;
	
	return;
}

void * read_net(void)
{
	if (net_status == 0)
	{
		net_init();
	}
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = TIMEOUT;
	
	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(sd, &read_set);
	
	int stat = select(sd+1, &read_set, NULL, NULL, &timeout);
	
	if (stat == -1)
	{
		return NULL;
	}
	else if (stat == 0)
	{
		return NULL;
	}
	else if (FD_ISSET(sd,&read_set))
	{
		len = sizeof(cliaddr);
		recvfrom(sd, (void *)readbuf, MAXLINE, 0, \
			(struct sockaddr *)&cliaddr, &len);
		
		return readbuf;
	}
	else
	{
		return NULL;
	}
	
	return NULL;
}

static void pth_init(phead *head1, int cmd, size_t len, const phead *head2)
{
	head1->psf 		= P_START_FLAG;
	head1->len 		= (uint16_t)len;
	head1->subcmd 		= cmd;
	head1->version 		= PROT_VERSION;
	if (head2 != NULL)
		head1->uid 	= head2->uid;
	else
		head1->uid 	= SPECIALUID;
	head1->fls_id 		= (uint16_t)localaddr.id;
	head1->fls_num 		= (uint16_t)rflsl.len;
	head1->client_ip 	= servaddr.sin_addr.s_addr;
	head1->client_port 	= servaddr.sin_port;
}

static void pt2_init(netpt2 *reply, const netpt1 *ptr, uint8_t stat, uint8_t result)
{
	bzero(reply, sizeof(netpt2));
	
	pth_init(&(reply->head),ACC_STAT_REP, sizeof(netpt2), &(ptr->head));
	
	reply->body.stat 	= stat;
	reply->body.result 	= result;
	reply->body.sid 	= ptr->body.sid;
	reply->body.uid 	= ptr->body.uid;
	
	reply->pef 		= P_END_FLAG;
}

static void pt4_init(netpt4 *reply, const netpt3 *ptr, uint8_t stat, recunit *unit)
{
	bzero(reply, sizeof(netpt4));
	
	pth_init(&(reply->head),FREQ_DATA_REP, sizeof(netpt4), &(ptr->head));
	
	stat = get_limd(ptr->body.sid, &(reply->body.freqd.limdl1), &(reply->body.freqd.limdl2));
	reply->body.stat 	= stat;
	reply->body.sid 	= ptr->body.sid;
	reply->body.uid 	= ptr->body.uid;
	if (stat == FALSE)
	{
		return;
	}
	reply->body.freqd.timel1 	= (uint32_t)unit->timel1;
	reply->body.freqd.counterl1 	= unit->counterl1;
	reply->body.freqd.timel2 	= (uint32_t)unit->timel2;
	reply->body.freqd.counterl2 	= unit->counterl2;
	
	reply->pef = P_END_FLAG;
}

static void pt6_init(netpt6 *reply, const netpt1 *ptr)
{
	bzero(reply, sizeof(netpt6));
	 
	pth_init(&(reply->head),FLS_LIST_REP, sizeof(netpt6), &(ptr->head));
	
	reply->body.num = rflsl.len;
	reply->body.sid = ptr->body.sid;
	reply->body.uid = ptr->body.uid;
	
	int i;
	for (i = 0; i < rflsl.len; ++i)
		reply->body.list[i] = rflsl.list[i].server;
	
	reply->pef 	= P_END_FLAG;
}

static void pt7_init(netpt7 *send, const addr *fls, uint8_t stat, uint16_t sid)
{
	bzero(send, sizeof(netpt7));
	
	pth_init(&(send->head), FLS_STAT_BC, sizeof(netpt7), NULL);
	
	send->body.stat 	= stat;
	send->body.sid		= sid;
	if (fls != NULL)
		send->body.fls 	= *fls;
	
	send->pef 		= P_END_FLAG;
}

static void pt8_init(netpt8 *reply, const netpt7 *ptr, uint8_t stat, uint8_t op)
{
	bzero(reply, sizeof(netpt8));

	pth_init(&(reply->head),FLS_STAT_REP, sizeof(netpt8), &(ptr->head));
	
	reply->body.stat 	= stat;
	reply->body.op 		= op;
	
	reply->pef 		= P_END_FLAG;
}

static void htonpch(phead *ptr)
{
	ptr->len	= htons(ptr->len);
	ptr->subcmd 	= htons(ptr->subcmd);
	ptr->version 	= htons(ptr->version);
	ptr->uid 	= htonl(ptr->uid);
	ptr->fls_id 	= htons(ptr->fls_id);
	ptr->fls_num 	= htons(ptr->fls_num);
}

static void ntohpch(phead *ptr)
{
	ptr->len	= ntohs(ptr->len);
	ptr->subcmd 	= ntohs(ptr->subcmd);
	ptr->version 	= ntohs(ptr->version);
	ptr->uid 	= ntohl(ptr->uid);
	ptr->fls_id 	= ntohs(ptr->fls_id);
	ptr->fls_num 	= ntohs(ptr->fls_num);
}

void ntohpt1(netpt1 *ptr)
{
	if (NULL == ptr)
		return;
	
	ntohpch(&(ptr->head));
	
	ptr->body.uid = ntohl(ptr->body.uid);
	ptr->body.sid = ntohs(ptr->body.sid);
	ptr->body.num = ntohs(ptr->body.num);
}

void htonpt2(netpt2 *ptr)
{
	if (NULL == ptr)
		return;
	
	htonpch(&(ptr->head));
	
	ptr->body.uid = htonl(ptr->body.uid);
	ptr->body.sid = htons(ptr->body.sid);
}

void ntohpt3(netpt3 *ptr)
{
	if (NULL == ptr)
		return;
	
	ntohpch(&(ptr->head));
	
	ptr->body.uid = ntohl(ptr->body.uid);
	ptr->body.sid = ntohs(ptr->body.sid);
}

void htonpt4(netpt4 *ptr)
{
	if (NULL == ptr)
		return;
	
	htonpch(&(ptr->head));
	
	ptr->body.uid 			= htonl(ptr->body.uid);
	ptr->body.sid 			= htons(ptr->body.sid);
	ptr->body.freqd.timel1 		= htonl(ptr->body.freqd.timel1);
	ptr->body.freqd.limdl1		= htonl(ptr->body.freqd.limdl1);
	ptr->body.freqd.counterl1 	= htonl(ptr->body.freqd.counterl1);
	ptr->body.freqd.timel2 		= htonl(ptr->body.freqd.timel2);
	ptr->body.freqd.limdl2		= htonl(ptr->body.freqd.limdl2);
	ptr->body.freqd.counterl2 	= htonl(ptr->body.freqd.counterl2);
}

void htonpt6(netpt6 *ptr)
{
	if (NULL == ptr)
		return;
	
	htonpch(&(ptr->head));
	
	ptr->body.num		= htons(ptr->body.num);
	ptr->body.sid 		= htons(ptr->body.sid);
	ptr->body.uid 		= htonl(ptr->body.uid);
}

void htonpt7(netpt7 *ptr)
{
	if (NULL == ptr)
		return;
	
	htonpch(&(ptr->head));
}

void ntohpt7(netpt7 *ptr)
{
	if (NULL == ptr)
		return;
	
	ntohpch(&(ptr->head));
	
	ptr->body.sid = ntohs(ptr->body.sid);
}

void htonpt8(netpt8 *ptr)
{
	if (NULL == ptr)
		return;
	
	htonpch(&(ptr->head));
}

void send_net2(const netpt1 *ptr, uint8_t stat, uint8_t result)
{
	if (NULL == ptr)
		return;
	
	if (net_status == 0)
	{
		net_init();
	}
	
	netpt2 reply;
	pt2_init(&reply, ptr, stat, result);
	htonpt2(&reply);
	
	cliaddr.sin_addr.s_addr = ptr->head.client_ip;
	cliaddr.sin_port = ptr->head.client_port;
	socklen_t len = sizeof(cliaddr);
	
	sendto(sd, (void *)(&reply), sizeof(netpt2), 0, \
		(struct sockaddr *)&cliaddr, len);
}

void send_net4(const netpt3 *ptr, uint8_t stat, recunit *unit)
{
	if (NULL == ptr)
		return;
	if (NULL == unit)
		return;
	
	if (net_status == 0)
	{
		net_init();
	}
	
	netpt4 reply;
	pt4_init(&reply, ptr, stat, unit);
	htonpt4(&reply);
	
	cliaddr.sin_addr.s_addr = ptr->head.client_ip;
	cliaddr.sin_port = ptr->head.client_port;
	socklen_t len = sizeof(cliaddr);
	
	sendto(sd, (void *)(&reply), sizeof(netpt4), 0, \
		(struct sockaddr *)&cliaddr, len);
}

void send_net6(const netpt1 *ptr)
{
	if (NULL == ptr)
		return;
	
	if (net_status == 0)
	{
		net_init();
	}
	
	netpt6 reply;
	pt6_init(&reply, ptr);
	htonpt6(&reply);
	
	cliaddr.sin_addr.s_addr = ptr->head.client_ip;
	cliaddr.sin_port = ptr->head.client_port;
	socklen_t len = sizeof(cliaddr);
	
	sendto(sd, (void *)(&reply), sizeof(netpt6), 0, \
		(struct sockaddr *)&cliaddr, len);
}

void send_net7(const addr *client, uint8_t stat)
{
	if (NULL == client)
		return;
	
	if (net_status == 0)
	{
		net_init();
	}
	
	netpt7 send;
	pt7_init(&send, &localaddr, stat, SPECIALSID);
	htonpt7(&send);
	
	cliaddr.sin_addr.s_addr = client->ip;
	cliaddr.sin_port = client->port;
	socklen_t len = sizeof(cliaddr);
	
	sendto(sd, (void *)(&send), sizeof(netpt7), 0, \
		(struct sockaddr *)&cliaddr, len);
}

void send_net8(const netpt7 *ptr, uint8_t stat, uint8_t op)
{
	if (NULL == ptr)
		return;
	
	if (net_status == 0)
	{
		net_init();
	}
	
	netpt8 reply;
	pt8_init(&reply, ptr, stat, op);
	htonpt8(&reply);
	
	cliaddr.sin_addr.s_addr = ptr->head.client_ip;
	cliaddr.sin_port = ptr->head.client_port;
	socklen_t len = sizeof(cliaddr);
	
	sendto(sd, (void *)(&reply), sizeof(netpt8), 0, \
		(struct sockaddr *)&cliaddr, len);
}

