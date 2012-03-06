#include"flsclient.h"

#define SIDMAX 5
#define UIDMAX 2500000000U
#define FREQ   100000
#define TRUE   1
#define FALSE  0

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		puts("usage: a.out <freq> <port>");
		
		return;
	}
	struct sockaddr_in cliaddr, servaddr;
	
	inet_aton("127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(8618);
	
	inet_aton("127.0.0.1", &cliaddr.sin_addr);
	cliaddr.sin_port = htons((uint16_t)atoi(argv[2]));
	
	flsc_init(cliaddr, servaddr);
	
	uint32_t uid;
	uint16_t sid;

	int i;
	int tout = 0;
	int allow = 0;
	
	srand(time(NULL));
	
	int freq, counter;
	freq = atoi(argv[1]);
	
	time_t last;
	while (1)
	{
		if (time(NULL) > last)
		{
			time(&last);
			counter = 0;
		}
		else
		{
			
			
			++counter;
			
			if (counter > freq)
				continue;
			sid = (uint16_t)(rand() % SIDMAX + 1);
			uid = (uint32_t)(rand() % UIDMAX + 1);
			
			if (flsc_stat_query(sid, uid, 1) == TRUE)
			{
				//puts("YES");
				;
			}
			if (flsc_error)
			{
				printf("flsc_error: %d\n", flsc_error);
			}
		
		}
	}
	
	return 0;
}
