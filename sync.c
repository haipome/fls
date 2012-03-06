#include"fls.h"

#define REPEAT_ATTEM 3
#define TIMEOUT 10000

runflslist rflsl;

static char * rflsl_name = "/wb/fls/data/rflsl.backup";
static char buf[MAXLINE];

int  manage_switch = 0;
addr manage_fls_addr;

int read_rflsl(void)
{
	char name[MAXLINE];
	sprintf(name, "%s%s", getenv("HOME"), rflsl_name);
	
	FILE *fp;
	if ((fp = fopen(name, "r")) == NULL)
		return FALSE;
	
	rflsl.len = 0;
	char line[MAXLINE];
	unsigned int id, ip, port;
	while (!feof(fp))
	{
		if (fgets(line, MAXLINE, fp) == NULL)
			break;
		
		int i = rflsl.len;
		sscanf(line, "%u %u %u", &id, &ip, &port);
		rflsl.list[i].server.id   = (uint8_t)id;
		rflsl.list[i].server.ip   = (in_addr_t)ip;
		rflsl.list[i].server.port = (in_port_t)port;
		++rflsl.len;
	}
	
	return TRUE;
}  

static int write_rflsl(void)
{
	char name[MAXLINE];
	sprintf(name, "%s%s", getenv("HOME"), rflsl_name);
	
	FILE *fp;
	if ((fp = fopen(name, "w")) == NULL)
		return FALSE;
	
	int i;
	for (i = 0; i < rflsl.len; ++i)
	{
		fprintf(fp, "%u %u %u\n", rflsl.list[i].server.id, rflsl.list[i].server.ip, \
			rflsl.list[i].server.port);
	}
	
	return TRUE;
}

static int add_item(addr *ptr)
{
	if (NULL == ptr)
		return ERROR;
	
	if (rflsl.len >= FLS_NUM_MAX)
		return FALSE;
	
	int i;
	for (i = 0; i < rflsl.len; ++i)
		if (ptr->id == rflsl.list[i].server.id)
		{
			if (ptr->ip != rflsl.list[i].server.ip || \
				ptr->port != rflsl.list[i].server.port)
			{
				run_msg("fls id:%d confused!");
			}
			return TRUE;
		}
	i = rflsl.len;
	memset(rflsl.list + i, '\0', sizeof(flsunit));
	
	rflsl.list[i].server.id  = ptr->id;
	rflsl.list[i].server.port = ptr->port;
	rflsl.list[i].server.ip   = ptr->ip;
	
	++rflsl.len;
	
	return TRUE;
}

static int rflsucom(const void * p1, const void * p2)
{
	return (((flsunit *)p1)->server.id - ((flsunit *)p2)->server.id);
}

int addto_rflsl(addr *ptr)
{
	if (NULL == ptr)
		return ERROR;
	
	if (!add_item(ptr))
		return FALSE;
	
	qsort(rflsl.list, rflsl.len, sizeof(flsunit), rflsucom);
	
	if (!write_rflsl())
		run_msg("In addto_rflsl: write_rflsl error");
	
	return TRUE;
}

int remove_rflsl(addr *ptr)
{
	if (NULL == ptr)
		return ERROR;
	
	int i = 0;
	
	if (ptr->id == localaddr.id)
		return FALSE;
	
	while (ptr->id != rflsl.list[i].server.id && i < rflsl.len)
		++i;
	
	if (i >= rflsl.len)
		return TRUE;
	
	for ( ; i < (rflsl.len - 1); ++i)
		memcpy(rflsl.list + i, rflsl.list + i + 1, sizeof(flsunit));
	memset(rflsl.list + i, '\0', sizeof(flsunit));
	--rflsl.len;
	
	if (!write_rflsl())
		run_msg("In remove_rflsl: write_rflsl error");
	
	return TRUE;
}

static int recv_netpt(uint32_t uid)
{
	fd_set read_set;
	struct timeval timeout;
	struct sockaddr_in cliaddr;
	socklen_t len = 0;
	int stat = 0;
	
	FD_ZERO(&read_set);
	FD_SET(sd, &read_set);
	timeout.tv_sec = 0;
	timeout.tv_usec = TIMEOUT;
	
	int i;
	for (i = REPEAT_ATTEM; i > 0; --i)
	{
		stat = select(sd+1, &read_set, NULL, NULL, &timeout);
		
		if (stat ==  -1)
		{
			run_msg("In recv_netpt: select error!");
			
			return stat;
		}
		else if (stat == 0)
		{	
			return stat;
		}
		else if (FD_ISSET(sd,&read_set))
		{
			recvfrom(sd, (void *)buf, MAXLINE, 0, \
				(struct sockaddr *)&cliaddr, &len);
			
			uint32_t id = ntohl(*(uint32_t *)(((char *)buf + UID_OFFSET)));
			
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
			return stat;
		}
	}
	
	return stat;
}

void manage_fls_init(void)
{
	int id_temp = getconf1("manageflsid");
	if (id_temp <= 0 || id_temp > FLS_ID_MAX)
		err_exit("In manage_fls_init: manageflsid is illegal");
	manage_fls_addr.id = id_temp;
		
	if (localaddr.id == 0)
		err_exit("In manage_fls_init: localaddr id is illegal");
	if (manage_fls_addr.id == localaddr.id)
	{
		manage_switch = TRUE;
	}
	else
	{
		manage_switch = FALSE;
	}
	
	char ip_string[MAXLINE];
	struct in_addr inaddr;
	if (strncpy(ip_string, getconfs("manageflsip"), MAXLINE - 1) == 0)
		err_exit("In manage_fls_init: get manageflsip error");
	inet_aton(ip_string, &(inaddr));
	manage_fls_addr.ip = inaddr.s_addr;
	
	if ((manage_fls_addr.port = (uint16_t)getconf1("manageflsport")) == 0)
		err_exit("In manage_fls_init: manageflsport is illegal");
	manage_fls_addr.port = htons(manage_fls_addr.port);
}

int inform_start(void)
{
	if (manage_fls_addr.id == 0)
	{
		manage_fls_init();
	}
	
	if (localaddr.id == manage_fls_addr.id)
	{
		addto_rflsl(&(localaddr));
		
		return TRUE;
	}
	
	int i, stat = 0;
	for (i = 0; i < REPEAT_ATTEM; ++i)
	{
		send_net7(&(manage_fls_addr), FLS_START);
		
		stat = recv_netpt(SPECIALUID);
		
		if (stat == TRUE)
		{
			return TRUE;
		}
		else
		{
			continue;
		}
	}
	
	return FALSE;
}

int inform_close(void)
{
	if (manage_fls_addr.id == 0)
	{
		manage_fls_init();
	}
	
	if (localaddr.id == manage_fls_addr.id)
	{
		remove_rflsl(&(localaddr));
		
		return TRUE;
	}
	
	int i, stat = 0;
	for (i = 0; i < REPEAT_ATTEM; ++i)
	{
		send_net7(&(manage_fls_addr), FLS_CLOSE);
		
		stat = recv_netpt(SPECIALUID);
		
		if (stat == TRUE)
		{
			return TRUE;
		}
		else
		{
			continue;
		}
	}
	
	return FALSE;
}

