#include"pickle.h"


static char * version = "pickle version 0.2 GCC 4.5 on linux2.6 64 bit\n";
static char * help    = "\n"
			"pickle is a tool for fls to recovery memory\n"
			"dump memory to disk befor you restart computer,and \n"
		 	"load memory from disk when you start fls server\n"
		 	"\n"
		 	"Usage:	pickle [-options][arguments]\n"
			"Options:\n"
			"	--version: show version and exit\n"
			"	--help: show this help exit\n"
			"\n"
			"	-d: dump memory to disk\n"
			"	-r: load memory from dish\n"
			"	-n n: Specify a process ID(0 ~ 9)\n"
			"\n"
		 	;


int __id_offset = 0;

int main(int argc, char *argv[])
{
	int c, type = 0;
	
	if (argc == 1)
	{
		printf("%s", help);
				
		return 0;
	}
	
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
				printf("unrecognized option '%s'\n",*argv);
				printf("%s", help);
				
				return 0;
			}
		}
		else if ((*argv)[0] == '-')
		{
			c = (*argv)[1];
			
			switch (c)
			{
				case 'v':
					printf("%s", version);
				
					return 0;
				case 'h':
					printf("%s", help);
				
					return 0;
				case 'n':
					if (--argc <= 0)
					{
						printf("expect number "
							"after option -n\n");
						
						return 0;
					}
					
					__id_offset = atoi(*++argv);
					
					if (__id_offset > SERV_PROC_NUM \
						|| __id_offset < 0)
					{
						printf("illegal number after "
							"option -n\n");
						
						return 0;
					}
					
					break;
				case 'd':
					type = DUMPER;
					
					break;
				case 'r':
					type = RELOAD;
					
					break;
				default:
					printf("unrecognized option '-%c'\n", c);
					printf("%s", help);
					
					return 0;
			}
		}
		else
		{
			printf("unrecognized option '%s'\n", *argv);
			printf("%s", help);
			
			return 0;
		}
		
	}
	
	if (type == DUMPER)
	{
		if (!dumper())
		{
			printf("memory dump failed\n");
			
			return 1;
		}
	}
	else if (type == RELOAD)
	{
		if (!reload())
		{
			printf("memory load failed\n");
			
			return 1;
		}
	}
	else
	{
		return 0;
	}
	
	return 0;
}

