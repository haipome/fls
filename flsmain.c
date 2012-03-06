#include"fls.h"


int fls_switch = 1;

static int netpt1_check(const netpt1 *ptr)
{
	if (search_limd(ptr->body.sid) == NULL)
	{
		return FALSE;
	}
	
	return TRUE;
}

static int netpt3_check(const netpt3 *ptr)
{
	if (search_limd(ptr->body.sid) == NULL)
	{
		return FALSE;
	}
	
	return TRUE;
}

static int netpt7_check(const netpt7 *ptr)
{
	if (search_limd(ptr->body.sid) == NULL && ptr->body.sid != SPECIALSID)
	{
		return FALSE;
	}
	
	return TRUE;
}

void fls_main(void)
{
	void * ptr;
	uint16_t cmd;
	time_t now, last_mrout, last_wlog;
	static int call = 0, callow = 0, crefuse = 0, cfail = 0; /*visit counter*/
	
	time(&last_wlog);
	
	last_mrout = time(NULL);
	extern int __memuse_rate_switch;

	while (fls_switch)
	{
		time(&now);
		
		if ((ptr = read_net()) == NULL)
			continue;
		
		cmd = ntohs(*(uint16_t *)(((char *)ptr + CMD_OFFSET)));
		
		switch (cmd)
		{
			case ACC_STAT_ASK:
			{
				ntohpt1(ptr);
				
				if (!netpt1_check(ptr))
					break;
				
				if (now != last_wlog)
				{
					addto_log(FLS_LOG_FREQ, "%5d  %5d  %5d  %5d",\
						call, callow, crefuse, cfail);
					call = callow = crefuse = cfail = 0;
					last_wlog = now;
				}
				
				++call;
				
				if (manage_switch && ((netpt1 *)ptr)->head.fls_num != rflsl.len)
				{
					send_net6(ptr);
					++callow;
					
					break;
				}
				
				uint8_t stat, result;
				result = access_query(ptr, &stat, now);
				
				if (stat == 1)
				{
					if (result == 1)
					{
						++callow;
					}
					else
					{
						++crefuse;
					}
				}
				else
				{
					++cfail;
				}
				
				send_net2(ptr, stat, result);
				
				break;
			}
			
			case FREQ_DATA_ASK:
			{
				ntohpt3(ptr);
				
				if (!netpt3_check(ptr))
					break;
				
				uint8_t stat;
				recunit *unit;
				
				if ((unit =  hash_search(((netpt3 *)ptr)->body.sid, \
					((netpt3 *)ptr)->body.uid, now)) == NULL)
				{
					stat = FALSE;
				}
				else
				{
					stat = TRUE;
				}
				
				send_net4(ptr, stat, unit);
				
				break;
			}
			
			case FLS_STAT_BC:
			{
				if (!manage_switch)
					break;
				
				ntohpt7(ptr);
				
				if (!netpt7_check(ptr))
					break;
				
				uint8_t stat = ((netpt7 *)ptr)->body.stat;
				
				switch (stat)
				{
					case FLS_START:
					{
						if (addto_rflsl(&(((netpt7 *)ptr)->body.fls)) \
							== FALSE)
						{
							run_msg("fls server is too much!");
						}
						send_net8(ptr, FLS_RUNING, ADDTO_RLIST);
						addto_log(FLS_LOG_GENE, "fls id: %d started.",\
							((netpt7 *)ptr)->body.fls.id);
						
						break;
					}
					case FLS_CLOSE:
					{
						remove_rflsl(&(((netpt7 *)ptr)->body.fls));
						send_net8(ptr, FLS_RUNING, REMOVE_RLIST);
						addto_log(FLS_LOG_GENE, "fls id: %d closed.",\
							((netpt7 *)ptr)->body.fls.id);
						
						break;
					}
					case FLS_DISCONN:
					{
						remove_rflsl(&(((netpt7 *)ptr)->body.fls));
						send_net8(ptr, FLS_RUNING, REMOVE_RLIST);
						addto_log(FLS_LOG_GENE, "fls id: %d disconnected!", \
							((netpt7 *)ptr)->body.fls.id);
						
						break;
					}
					default:
					{
						break;
					}
				}
			}
			
			default:
			{
				break;
			}
			
			
		}  /*switch end*/
		
		if (__memuse_rate_switch && (((now - last_mrout) % MEM_RATE_TIME) == 0) \
			&& last_mrout != now)
		{
			pid_t pid;
			
			if ((pid = fork()) < 0)
			{
				run_msg("In fls_main memuse_rate: fork error");
			}
			else if (pid == 0)
			{
				memuse_rate();
				
				//return;
				exit(0);
			}
			else
			{
				last_mrout = now;
			}
		}
	}  /*while end*/	
}

