#include"fls.h"


static struct limit_data limd;

static int add_item(struct limit_unit * ptr)
{
	if (NULL == ptr)
		return FALSE;
	if (limd.len >= CLIENT_MAX)
		return FALSE;
	
	int i;
	for (i = 0; i < limd.len; ++i)
		if (ptr->sid == limd.list[i].sid)
			return TRUE;
	i = limd.len;
	memset(limd.list + i, '\0', sizeof(struct limit_unit));
	
	limd.list[i].sid      = ptr->sid;
	limd.list[i].limit[0] = ptr->limit[0];
	limd.list[i].limit[1] = ptr->limit[1];
	
	++limd.len;
	
	return TRUE;
}

static int limducom(const void * p1, const void * p2)
{
	return (((struct limit_unit *)p1)->sid - ((struct limit_unit *)p2)->sid);
}

int addto_limd(uint16_t sid, uint32_t lim1, uint32_t lim2)
{
	struct limit_unit temp;
	
	temp.sid = sid;
	temp.limit[0] = lim1;
	temp.limit[1] = lim2;
	
	if (!add_item(&temp))
		return FALSE;
	
	qsort(limd.list, limd.len, sizeof(struct limit_unit), limducom);
	
	return TRUE;
}

struct limit_unit * search_limd(uint16_t sid)
{
	int low  = 0;
	int high = limd.len - 1;
	int mid;
	
	while (low <= high)
	{
		mid = low + ((high - low) / 2);
		
		if (limd.list[mid].sid == sid)
		{
			return limd.list + mid;
		}
		else if (limd.list[mid].sid > sid)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}
	}
	
	return NULL;
}

int get_limd(const uint16_t sid, uint32_t *lim1, uint32_t *lim2)
{
	if (NULL == lim1 || NULL == lim2)
		return FALSE;
	
	if (limd.len == 0)
	{
		limit_init();
	}
	
	struct limit_unit * temp;
	if ((temp = search_limd(sid)) == NULL)
		return FALSE;
	
	*lim1 = temp->limit[0];
	*lim2 = temp->limit[1];
	
	return TRUE;
}

