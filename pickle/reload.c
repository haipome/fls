#include"pickle.h"


int reload(void)
{
	char * name;
	FILE * fp;
	int shmid;
	size_t size, memsize;
	recunit *shmptr;
	struct shmid_ds shmipc;
	struct stat fstat;
	
	name = getname();
	if ((fp = fopen(name, "rb")) == NULL)
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

	if (stat(name, &fstat) < 0)
	{
		return FALSE;
	}
	memsize = fstat.st_size;
	
	
	if (memsize != shmipc.shm_segsz)
	{
		printf("memory size changed! ");
		
		return FALSE;
	}
	
	printf("id: %d, size: %lu, loading... ", __id_offset, size);
	fflush(NULL);
	fread(shmptr, size, sizeof(recunit), fp);
	printf("done!\n");
	
	return TRUE;
}

