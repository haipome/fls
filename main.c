#include"fls.h"


static char * version = "Frequency limit server 0.90 (2011 08 30)\n"
			"GCC 4.5 on Linux 2.6 64 bit\n";
static char * help    = "\n"
			"Welcome to FLS (Frequency limit server)\n"
			"Copyright (C) 2011 damonyang@tencent.com\n"
			"\n"
			"Usage:	fls [-options][arguments]\n"
			"Options:\n"
			"	--version: show version and exit\n"
			"	--help: show this help and exit\n"
			"\n"
			"	-k (n): kill(exit) other(restart) and exit\n"
			"	-d (n): print the pid of server process and exit\n"
			"	-m: output run message to stdout\n"
			"	-r: output share memory use rate\n"
			"	-n n: Specify a process ID(0 ~ 9)\n"
			"	-p n: Specify a local bind port\n"
			"\n"
			;

int __memuse_rate_switch = 0;
int __msg_stdout_switch  = 0;
int __id_offset          = 0;
int __bind_port          = 0;

int main(int argc, char *argv[])
{
	int c, loop_end1 = 0, loop_end2 = 0;
	
	while (--argc > 0)
	{
		if (!(strncmp((*++argv), "--", 2)))
		{
			if (!(strcmp((*argv + 2), "version")))
			{
				printf("%s", version);
				
				return 0;
			}
			else if (!(strcmp((*argv + 2), "help")))
			{
				printf("%s", help);
				
				return 0;
			}
			else
			{
				printf("fls: unrecognized option '%s'\n",*argv);
				printf("%s", help);
				
				return 0;
			}
		}
		else if ((*argv)[0] == '-')
		{
			while ((c = *++(*argv)) != 0)
			{
				switch (c)
				{
					case 'h':
						printf("%s", help);
						
						return 0;
					case 'v':
						printf("%s", version);
						
						return 0;
					case 'k':
						if (--argc <= 0)
						{
							__id_offset = 0;
						}
						else
						{
							__id_offset = atoi(*++argv);
							
							if (__id_offset > SERV_PROC_NUM \
								|| __id_offset < 0)
							{
								printf("value after -k "
									"is illegal\n");
								
								return 0;
							}
						}
						
						send_exit_signal();
						
						return 0;
					case 'd':
						if (--argc <= 0)
						{
							__id_offset = 0;
						}
						else
						{
							__id_offset = atoi(*++argv);
							
							if (__id_offset > SERV_PROC_NUM \
								|| __id_offset < 0)
							{
								printf("value after -d "
									"is illegal\n");
								
								return 0;
							}
						}
						
						pid_print();
						
						return 0;
					case 'r':
						__memuse_rate_switch = 1;
						
						break;
					case 'm':
						__msg_stdout_switch = 1;
						
						break;
					case 'n':
						if (--argc <= 0)
						{
							puts("Expect number "
							"after option -n");
							
							return 0;
						}
						
						__id_offset = atoi(*++argv);
						
						if (__id_offset > SERV_PROC_NUM \
							|| __id_offset < 0)
						{
							puts("illegal number "
							"after option -n");
							
							return 0;
						}
						
						loop_end2 = TRUE;
						
						break;
					case 'p':
						if (--argc <= 0)
						{
							puts("expect bind port "
							"after option -p");
							
							return 0;
						}
						
						__bind_port = atoi(*++argv);
						
						loop_end2 = TRUE;
						
						break;
					default:
						printf("fls: unrecognized"
							" option '-%c'\n", c);
						printf("%s", help);
						
						return 0;
				}
				
				if (loop_end2)
					break;
			}  /*while end*/
		}
		else
		{
			printf("fls: unrecognized option '%s'\n", *argv);
			printf("%s", help);
			
			return 0;
		}
		
		if (loop_end1)
			break;
	}  /*while end*/
	
	fls_init();  /*initialization*/
	
	fls_main();  /*start service*/
	
	fls_exit();  /*server exit*/
	
	return 0;
}

