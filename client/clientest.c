#include"flsclient.h"

#define SIDMAX 5
#define UIDMAX 2500000000U
#define FREQ   100000
#define TRUE 1
#define FALSE 0

int main()
{
	struct sockaddr_in cliaddr, servaddr;
	
	inet_aton("127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(8618);
	
	inet_aton("127.0.0.1", &cliaddr.sin_addr);
	cliaddr.sin_port = htons(1600);
	
	flsc_init(cliaddr, servaddr);
	
	uint32_t uid;
	uint16_t sid;

	int i;
	int tout = 0;
	int allow = 0;
	
	srand(time(NULL));
	
	for (i = 0; i < FREQ; ++i)
	{	
		sid = (uint16_t)(rand() % SIDMAX + 1);
		uid = (uint32_t)(rand() % UIDMAX + 1);
	
		if (flsc_stat_query(sid, uid, 1) == TRUE)
		{
			++allow;
			//printf("%d\n", allow)
			;
		}
		if (flsc_error)
		{
			printf("flsc_error: %d\n", flsc_error);
			++tout;
		}
		
		usleep(10000);
	}
	
	printf("allow num: %d\n", allow);
	printf("timeout rate: %f\n", (float)tout / FREQ);
	printf("allow rate: %f\n", (float)allow / FREQ);
	
	struct flsc_freqdata freqd;
	
	if (flsc_freq_query(sid, uid, &freqd) == TRUE)
	{
		printf("start time: %u  limit: %u  freq: %u\n", freqd.timel1, freqd.limdl1, freqd.counterl1);
		printf("start time: %u  limit: %u  freq: %u\n", freqd.timel2, freqd.limdl2, freqd.counterl2);
	}
	if (flsc_error)
	{
		printf("flsc_error in freq_query: %d\n", flsc_error);
	}
	
	return 0;
}
