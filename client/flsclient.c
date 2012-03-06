#include"flsc.h"


static runflslist rflsl;	/*useable freqence limit server*/
static time_t adelays = 0;	/*all server delay start*/
static int    adelayt = 0;	/*all server delay time*/

static addr manage_fls;
static struct sockaddr_in cliaddr, servaddr;
static socklen_t len;

static char readbuf[MAXLINE];
static int  sd;
static int  client_stat = 0;

int flsc_error = 0;

static void sync_rflsl(netpt6 * ptr)
{
	if (ptr->body.num > FLS_NUM_MAX)
	{
		flsc_error = RFLSLTOOLONG;
		
		return;
	}
	if (ptr->body.num  == 0)
	{
		flsc_error = ILLEGALINPUT;
		
		return;
	}
	
	rflsl.len = ptr->body.num;
	int i;
	for (i = 0; i < rflsl.len; ++i)
	{
		bzero(rflsl.list + i, sizeof(flsunit));
		rflsl.list[i].server = ptr->body.list[i];
	}
	
	return;
}

int flsc_init(struct sockaddr_in local, struct sockaddr_in server)
{
	if (client_stat == 1)
		return TRUE;
	
	rflsl.len = 1;
	bzero(rflsl.list, sizeof(flsunit));
	rflsl.list[0].server.id = 0;
	rflsl.list[0].server.ip = server.sin_addr.s_addr;
	rflsl.list[0].server.port = server.sin_port;
	
	manage_fls.id = 0;
	manage_fls.ip = server.sin_addr.s_addr;
	manage_fls.port = server.sin_port;
	
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		flsc_error = SOCKOPENERR;
		
		return FALSE;
	}
	
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port = local.sin_port;
	if (bind(sd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
	{
		flsc_error = SOCKBINDERR;
		
		return FALSE;
	}
	
	cliaddr = local;
	client_stat = 1;
	
	return TRUE;
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

static void htonpt1(netpt1 *ptr)
{
	htonpch(&(ptr->head));
	
	ptr->body.uid = htonl(ptr->body.uid);
	ptr->body.sid = htons(ptr->body.sid);
	ptr->body.num = htons(ptr->body.num);
}

static void ntohpt2(netpt2 *ptr)
{
	ntohpch(&(ptr->head));
	
	ptr->body.sid = ntohs(ptr->body.sid);
	ptr->body.uid = ntohl(ptr->body.uid);
}

static void htonpt3(netpt3 *ptr)
{
	htonpch(&(ptr->head));
	
	ptr->body.uid = htonl(ptr->body.uid);
	ptr->body.sid = htons(ptr->body.sid);
}

static void ntohpt4(netpt4 *ptr)
{
	ntohpch(&(ptr->head));
	
	ptr->body.uid 			= ntohl(ptr->body.uid);
	ptr->body.sid 			= ntohs(ptr->body.sid);
	ptr->body.freqd.timel1 		= ntohl(ptr->body.freqd.timel1);
	ptr->body.freqd.limdl1		= ntohl(ptr->body.freqd.limdl1);
	ptr->body.freqd.counterl1 	= ntohl(ptr->body.freqd.counterl1);
	ptr->body.freqd.timel2 		= ntohl(ptr->body.freqd.timel2);
	ptr->body.freqd.limdl2		= ntohl(ptr->body.freqd.limdl2);
	ptr->body.freqd.counterl2 	= ntohl(ptr->body.freqd.counterl2);
}

static void ntohpt6(netpt6 *ptr)
{
	ntohpch(&(ptr->head));
	
	ptr->body.uid 		= ntohl(ptr->body.uid);
	ptr->body.sid 		= ntohs(ptr->body.sid);
	ptr->body.num		= ntohs(ptr->body.num);
}

static void htonpt7(netpt7 *ptr)
{
	htonpch(&(ptr->head));
	
	ptr->body.sid = htons(ptr->body.sid);
}

static void ntohpt8(netpt8 *ptr)
{
	ntohpch(&(ptr->head));
}

static void pth_init(phead *head1, int cmd, size_t len, uint32_t uid)
{
	head1->psf 		= P_START_FLAG;
	head1->len 		= (uint16_t)len;
	head1->version 		= (uint16_t)PROT_VERSION;
	head1->subcmd 		= (uint16_t)cmd;
	head1->uid 		= uid;
	head1->fls_id 		= (uint16_t)0;
	head1->fls_num 		= (uint16_t)rflsl.len;
	head1->client_ip	= cliaddr.sin_addr.s_addr;
	head1->client_port 	= cliaddr.sin_port;
}

static void netpt1_init(netpt1 *send, uint16_t sid, uint32_t uid, uint16_t num)
{
	bzero(send, sizeof(netpt1));
	
	pth_init(&(send->head),ACC_STAT_ASK, sizeof(netpt1), uid);
	
	send->body.sid 	= sid;
	send->body.uid 	= uid;
	send->body.num 	= num;
	
	send->pef 	= P_END_FLAG;
}

static void netpt3_init(netpt3 *send, uint16_t sid, uint32_t uid)
{
	bzero(send, sizeof(netpt3));

	pth_init(&(send->head),FREQ_DATA_ASK, sizeof(netpt3), uid);
	
	send->body.sid 	= sid;
	send->body.uid 	= uid;
	
	send->pef 	= P_END_FLAG;
}

static void netpt7_init(netpt7 *send, addr *fls, uint8_t stat, uint16_t sid)
{
	bzero(send, sizeof(netpt7));

	pth_init(&(send->head),FLS_STAT_BC, sizeof(netpt7), SPECIALUID);
	
	send->body.stat 	= stat;
	send->body.sid		= sid;
	if (fls != NULL)
		send->body.fls 	= *fls;
	
	send->pef 		= P_END_FLAG;
}

static int send_net1(uint16_t sid, uint32_t uid, uint16_t num, addr *server)
{
	netpt1 send;
	netpt1_init(&send, sid, uid, num);
	htonpt1(&send);
	
	servaddr.sin_addr.s_addr = server->ip;
	servaddr.sin_port = server->port;
	len = sizeof(servaddr);
	
	sendto(sd, (void *)(&send), sizeof(netpt3), 0, \
		(struct sockaddr *)&servaddr, len);
	
	return TRUE;
}

static int send_net3(uint16_t sid, uint32_t uid, addr *server)
{
	netpt3 send;
	netpt3_init(&send, sid, uid);
	htonpt3(&send);
	
	servaddr.sin_addr.s_addr = server->ip;
	servaddr.sin_port = server->port;
	len = sizeof(servaddr);
	
	sendto(sd, (void *)(&send), sizeof(netpt3), 0, \
		(struct sockaddr *)&servaddr, len);
	
	return TRUE;
}

static int send_net7(uint8_t stat, addr *fls, addr *server, uint16_t sid)
{
	netpt7 send;
	netpt7_init(&send, fls, stat, sid);
	htonpt7(&send);
	
	servaddr.sin_addr.s_addr = server->ip;
	servaddr.sin_port = server->port;
	len = sizeof(servaddr);
	
	sendto(sd, (void *)(&send), sizeof(netpt7), 0, \
		(struct sockaddr *)&servaddr, len);
	
	return TRUE;
}

static int recv_netpt(uint32_t uid)
{
	fd_set read_set;
	struct timeval timeout;
	int stat;
	
	int i;
	for (i = REPEAT_ATTEM; i > 0; --i)
	{
		FD_ZERO(&read_set);
		FD_SET(sd, &read_set);
		timeout.tv_sec = 0;
		timeout.tv_usec = TIMEOUT;
		
		stat = select(sd+1, &read_set, NULL, NULL, &timeout);
		
		if (stat ==  -1)
		{
			flsc_error = SELECTERR;
			
			return stat;
		}
		else if (stat == 0)
		{
			flsc_error = WAITTIMEOUT;
			
			return stat;
		}
		else if (FD_ISSET(sd,&read_set))
		{
			recvfrom(sd, (void *)readbuf, MAXLINE, 0, \
				(struct sockaddr *)&servaddr, &len);
			
			uint32_t id = ntohl(*(uint32_t *)(((char *)readbuf + UID_OFFSET)));
			
			if (id != uid)
			{
				continue;
			}
			else
			{
				return TRUE;
			}
		}
		else
		{
			break;
		}
	}
	
	return stat;
}

static int netpt2_check(netpt2 *ptr)
{
	return TRUE;
}

static int netpt4_check(netpt4 *ptr)
{
	return TRUE;
}

static int netpt6_check(netpt6 *ptr)
{
	return TRUE;
}

static int netpt8_check(netpt8 *ptr)
{
	return TRUE;
}

static void inform_disconn(flsunit * unit, uint16_t sid)
{
	send_net7(FLS_DISCONN, &(unit->server), &(manage_fls), sid);
	
	int stat = recv_netpt(SPECIALUID);
	
	if (stat == TRUE)
	{	
		netpt8 *recv = (netpt8 *)readbuf;
		ntohpt8(recv);
		
		if (!netpt8_check(recv))
		{
			flsc_error = ILLEGALINPUT;
			
			return;
		}
		
		if (recv->head.subcmd == FLS_STAT_REP && recv->body.op == REMOVE_RLIST)
			;
	}
	
	return;
}

int flsc_stat_query(uint16_t sid, uint32_t uid, uint16_t num)
{
	flsc_error = 0;
	time_t now;
	time(&now);
	
	if (adelayt && (now - adelays) < adelayt)
	{
		return TRUE;
	}
	
	if (client_stat == 0)
	{
		flsc_error = FLSCNINIT;
		
		return TRUE;
	}
	
	flsunit *unit = rflsl.list + uid % rflsl.len;
	
	if (unit->delaytime)
	{
		if (unit->delaytime > DELAY_MAX)
		{
			inform_disconn(unit, sid);
			unit->delaytime = 0;
			
			return TRUE;
		}
		else if ((now - unit->delaystart) < unit->delaytime)
		{
			return TRUE;
		}
		else
		{
			;
		}
	}
	
	if (unit->delaycount >= REPEAT_ATTEM)
	{
		unit->delaycount = 0;
		if (unit->delaytime == 0)
			unit->delaytime += DELAY_SYNC;
		else
			unit->delaytime += DELAY_TIME;
		unit->delaystart = now;
		
		return TRUE;
	}
	
	send_net1(sid, uid, num, &(unit->server));
	
	int stat = recv_netpt(uid);
	
	if (stat == -1)
	{
		return TRUE;
	}
	else if (stat == 0)
	{
		++(unit->delaycount);
		
		return TRUE;
	}
	else if(stat == TRUE)
	{
		adelayt 		= 0;
		unit->delaycount 	= 0;
		unit->delaytime		= 0;
		
		uint16_t cmd;
		cmd = ntohs(*(uint16_t *)(((char *)readbuf + CMD_OFFSET)));
		
		switch (cmd)
		{
			case ACC_STAT_REP:
			{
				netpt2 *recv = (netpt2 *)readbuf;
				ntohpt2(recv);
				
				if (!netpt2_check(recv))
				{
					flsc_error = ILLEGALINPUT;
					
					return TRUE;
				}
				
				if (recv->body.stat == FALSE)
				{
					flsc_error = QUERYERR;
					
					return TRUE;
				}
				else if (recv->body.stat && recv->body.result)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
				
				return TRUE;
			}
			case FLS_LIST_REP:
			{
				netpt6 *recv = (netpt6 *)readbuf;
				ntohpt6(recv);
				
				if (!netpt6_check(recv))
				{
					flsc_error = ILLEGALINPUT;
					
					return flsc_error;
				}
				
				sync_rflsl((netpt6 *)readbuf);
				adelays = now;
				adelayt = DELAY_SYNC;
				
				return TRUE;
			}
			default:
			{
				flsc_error = UNKNOWNCMD;
				
				return TRUE;
			}
		}
	}
	else
	{
		return TRUE;
	}
	
	return TRUE;
}

int flsc_freq_query(uint16_t sid, uint32_t uid, struct flsc_freqdata * freqd)
{
	if (client_stat == 0)
	{
		flsc_error = FLSCNINIT;
		
		return FALSE;
	}
	flsunit *unit = rflsl.list + uid % rflsl.len;
	
	if (unit->delaytime)
	{
		return FALSE;
	}
	
	send_net3(sid, uid, &(unit->server));
	
	if (recv_netpt(uid) == TRUE)
	{
		netpt4 *recv = (netpt4 *)readbuf;
		ntohpt4(recv);	
		
		if (!netpt4_check(recv))
		{
			flsc_error = ILLEGALINPUT;
			
			return FALSE;
		}
		
		if (((netpt4 *)readbuf)->body.stat)
		{
			*freqd = ((netpt4 *)readbuf)->body.freqd;
		
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	
	return FALSE;
}

