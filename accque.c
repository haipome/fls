#include"fls.h"


extern int 	timel1, timel2;	/*from config, freqence limit time in two levels*/
extern int 	warning_rate;	/*from config, alarm level*/

static uint32_t limit1, limit2;	/*freqence of limit in two levels*/
static char 	msg[MAXLINE];	/*log message buf*/
static char 	buf[MAXLINE];

static void output_log1(recunit *unit, const netpt1 *p)
{
	if (NULL == unit || NULL == p)
		return;
	if (unit->sign == 0)
		return;
	
	if (unit->counterl1 > limit1)
	{
		struct tm *hour = localtime(&(unit->timel1));
		strftime(buf, 20, "%H:%M:%S", hour);
		sprintf(msg, "%10d  %4d  %s  %4d  %4d  %4d",p->body.uid,\
			p->body.sid, buf, unit->counterl1, timel1, limit1);
		
		addto_log(FLS_LOG_OVER, msg);
		if ((unit->counterl1 / limit1) >= warning_rate)
			addto_log(FLS_LOG_WARN, msg);
	}
}

static void output_log2(recunit *unit, const netpt1 *p)
{
	if (NULL == unit || NULL == p)
		return;
	if (unit->sign == 0)
		return;
	
	if (unit->counterl2 > limit2)
	{
		struct tm *hour = localtime(&(unit->timel1));
		strftime(buf, 20, "%H:%M:%S", hour);
		sprintf(msg, "%10d\t%4d\t%s\t%4d\t%4d\t%4d",p->body.uid,\
			p->body.sid, buf, unit->counterl2, timel2, limit2);
		
		addto_log(FLS_LOG_OVER, msg);
		if ((unit->counterl2 / limit2) >= warning_rate)
			addto_log(FLS_LOG_WARN, msg);
	}
}

static void unit_init(recunit *unit, const netpt1 *p, time_t now)
{
	if (NULL == unit || NULL == p)
		return;
	
	unit->sign 	= USED_FLAG;
	unit->version 	= UNIT_VERSION;
	unit->sid 	= p->body.sid;
	unit->uid 	= p->body.uid;
	unit->timel1 	= now;
	unit->counterl1 = 0;
	unit->timel2 	= now;
	unit->counterl2 = 0;
	unit->allowedl2	= 0;
	
	return;
}

static void update_counter(recunit *unit, const netpt1 *p, time_t now)
{
	if (NULL == unit || NULL == p)
		return;
	
	int not_use  = !(unit->sign);
	int not_same = (unit->sid != p->body.sid) || (unit->uid != p->body.uid);
	int is_old   = (now - unit->timel2) >= timel2;
	
	if (not_use || not_same || is_old)
	{
		output_log1(unit, p);
		output_log2(unit, p);
		
		unit_init(unit, p, now);
	}
	
	if ((now - unit->timel1) >= timel1)
	{
		output_log1(unit, p);
		
		unit->timel1 = now;
		unit->counterl1 = 0;
	}
	
	unit->counterl1 += p->body.num;
	unit->counterl2 += p->body.num;
	
	return;
}

uint8_t access_query(const netpt1 *p, uint8_t * stat, time_t now)
{
	if (NULL == p || NULL == stat)
		return FALSE;
	
	recunit *memunit = hash_search(p->body.sid, p->body.uid, now);
	
	if (memunit == NULL)
	{
		*stat = FALSE;
		
		return FALSE;
	}
	
	if(!(get_limd(p->body.sid, &limit1, &limit2)))
	{
		*stat = FALSE;
		
		return FALSE;
	}
	
	*stat = TRUE;
	
	update_counter(memunit, p, now);
	
	if (memunit->counterl1 > limit1)
	{
		return FALSE;
	}
	else if (memunit->allowedl2 >= limit2)
	{
		return FALSE;
	}
	else
	{
		memunit->allowedl2 += p->body.num;
		
		return TRUE;
	}
	
	return TRUE;
}

