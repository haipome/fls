#include"fls.h"


#define SHM_MODE 	0600	/*users read/write*/
#define DIMENMAX 	100	/*The maximum dimension of the hash table*/
#define HASHMINLEN 	10000 	/*The min len of hashlinemaxlen*/

extern int timel2;
extern int __id_offset;

struct memory
{
	recunit *p[DIMENMAX];
	int 	dimen;
	int 	len;
	size_t 	size;
	int 	num;
};
static struct 	memory shamem;
static int 	shamem_status = 0;

static int 	prime_num[DIMENMAX];
static int 	unit_livet;

static void *shamem_creat()
{
	void *shmptr;
	int shmid;
	
	if ((shmid = shmget(IPC_PRIVATE, shamem.size, SHM_MODE)) < 0)
		err_exit("In shamem_creat: shmget error");
	if ((shmptr = shmat(shmid, NULL, 0)) == (void *)-1)
		err_exit("In shamem_creat: shmat error");
	
	setconf1("shamemsize", shamem.size);
	setconf2("shamemid", __id_offset, shmid);
	
	return shmptr;
}

static void *shamem_get()
{
	void *shmptr = NULL;
	int shmid;
	
	if ((shmid = (int)getconf2("shamemid", __id_offset)) == 0)
	{
		return shamem_creat();
	}
	else if ((shmptr = shmat(shmid, 0, 0)) == (void *)-1)
	{
		return shamem_creat();
	}
	else
	{
		struct shmid_ds buf;
		shmctl(shmid, IPC_STAT, &buf);
		if (buf.shm_segsz != shamem.size)
		{
			shmctl(shmid, IPC_RMID, NULL);
			
			return shamem_creat();
		}
	}
	
	return shmptr;
}

static int is_prime_num(int x)
{
	int i, s;
	s = (int)sqrt(x * 1.0);
	for (i = 2; i <= s; ++i)
	{
		if (!(x % i))
			return FALSE;
	}
	
	return TRUE;
}

static void prnum_init(int max, int dimen)
{
	int i, n = max;
	for (i = 0; i < dimen; ++i)
	{
		do
		{
			if (is_prime_num(n))
			{
				prime_num[i] = n;
				--n;
				break;
			}
			--n;
		}
		while (1);
	}
}

void shamem_init(void)
{
	if (shamem_status == 1)
		return;
	
	if ((shamem.len = (int)getconf1("hashlinemax")) == 0)
		err_exit("IN shamem_init: the value of hashlinemaxlen is illegal");
	if (shamem.len < HASHMINLEN)
		err_exit("In shamem_init: the value of hashlinemaxlen should bigger than %d", HASHMINLEN);
	if ((shamem.dimen = (int)getconf1("hashdimen")) == 0)
		err_exit("IN shamem_init: the value of hashdimen is illegal");
	if (shamem.dimen > DIMENMAX)
		err_exit("In shamem_init: the value of hashdimen should less than %d", DIMENMAX);
	
	prnum_init(shamem.len, shamem.dimen);
	shamem.num = shamem.size = 0;
	
	int i;
	for (i = 0; i < shamem.dimen; ++i)
	{
		shamem.size += prime_num[i] * sizeof(recunit);
		shamem.num += prime_num[i];
	}
	
	void *shmptr;
	if ((shmptr = shamem_get()) == NULL)
		err_exit("In shamem_init: share memory get error");
	
	int sum_pass = 0;
	for (i = 0; i < shamem.dimen; ++i)
	{
		shamem.p[i] = shmptr + sum_pass * sizeof(recunit);
		sum_pass += prime_num[i];
	}
	
	limit_init();
	unit_livet = timel2 + 1; 
	
	shamem_status = 1;
	
	return;
}

recunit *hash_search(uint16_t sid, uint32_t uid, time_t now)
{
	if (shamem_status == 0)
	{
		shamem_init();
	}
	
	int n = 0;
	recunit * p;
	uint32_t key = sid ^ uid;
	
	while (n < shamem.dimen)
	{
		p = (shamem.p[n]) + (key % prime_num[n]);
		
		if (p->sign == 0X00)
		{
			return p;
		}
		else if ((p->sid == sid) && (p->uid == uid))
		{
			return p;
		}
		else
		{
			++n;
		}
	}
	
	n = 0;
	while (n < shamem.dimen)
	{
		p = (shamem.p[n]) + (key % prime_num[n]);
		
		if ((now - p->timel2) > unit_livet)
		{	
			return p;
		}
		else
		{
			++n;
		}
	}
	
	return NULL;
}

void memuse_rate(void)
{
	int samsur = 100;
	recunit *p = shamem.p[0];
	int i, counter = 0;
	
	time_t now = time(NULL);
	for (i = 0; i < shamem.num; i = i + samsur)
		if ((p + i)->sign && (now - (p + i)->timel2) < unit_livet)
			++counter;
	
	float userate = ((float)counter * samsur)/ shamem.num;
	run_msg("share memory use rate = %.4f", userate);
	
	return;
}

