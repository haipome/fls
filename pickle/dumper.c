#include"pickle.h"


int dumper(void)
{
	char * name;
	FILE * fp;
	int shmid;
	size_t size;
	recunit *shmptr;
	struct shmid_ds shmipc;
	
	name = getname();
	if ((fp = fopen(name, "wb+")) == NULL)
	{
		return FALSE;
	}
	if ((shmid = getconf2("shamemid", __id_offset)) == 0)
	{
		return FALSE;
	}
	if ((shmptr = shmat(shmid, 0, 0)) == (void *)-1)
	{
		return FALSE;
	}
	shmctl(shmid, IPC_STAT, &shmipc);
	size = shmipc.shm_segsz / sizeof(recunit);
	
	printf("id: %d, size: %lu, dumping... ", __id_offset, size);
	fflush(NULL);
	fwrite(shmptr, size, sizeof(recunit), fp);
	printf("done!\n");
	
	return TRUE;
}

