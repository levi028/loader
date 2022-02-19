/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_data_chgrp.c
*
*    Description: The function of system channel group
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <string.h>
#include <basic_types.h>

#include "system_data.h"
#include "system_data_inner.h"

#define SYS_PRINTF    PRINTF
static BOOL get_cur_group_idx(UINT8 *group_idx);

/*******************************************************************************
* system data: Channel group operation APIs
********************************************************************************/

/* Get & Set current channel mode */
UINT8 sys_data_get_cur_chan_mode(void)
{
    return CUR_CHAN_MODE;
}

void sys_data_set_cur_chan_mode(UINT8 chan_mode )
{
    if(chan_mode>1)
    {
        chan_mode = 1;
    }
    CUR_CHAN_MODE = chan_mode;
    PIP_CHAN_MODE = chan_mode;
}

/* Sync group information with database  */
void sys_data_check_channel_groups(void)
{
    UINT8 i = 0;
    UINT16 total_v_prog_num = 0;
    UINT16 total_a_prog_num = 0;
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;
    UINT16 sat_v_prog_num[MAX_SAT_NUM] = {0};
    UINT16 sat_a_prog_num[MAX_SAT_NUM] = {0};
    UINT16 fav_v_prog_num[32] = {0};
    UINT16 fav_a_prog_num[32] = {0};
    UINT16 prog_num=0;

    
    p_sys_data = sys_data_get();

    total_v_prog_num = 0;
    total_a_prog_num = 0;
    for(i=0;i<MAX_SAT_NUM;i++)
    {
        sat_v_prog_num[i] = 0;
        sat_a_prog_num[i] = 0;
    }
    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        fav_v_prog_num[i] = 0;
        fav_a_prog_num[i] = 0;
    }

    get_specific_prog_num(SAT_PROG_NUM | FAV_PROG_NUM,
            sat_v_prog_num,sat_a_prog_num,fav_v_prog_num,fav_a_prog_num);
    for(i=0;i<MAX_SAT_NUM;i++)
    {
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1+i];

        if(0 == sat_v_prog_num[i])
        {
            p_grp->tv_channel = P_INVALID_ID;
        }
        else if(p_grp->tv_channel>=sat_v_prog_num[i])
        {
            p_grp->tv_channel = 0;//sat_v_prog_num[i] - 1;
        }
        if(0 == sat_a_prog_num[i])
        {
            p_grp->radio_channel = P_INVALID_ID;
        }
        else if(p_grp->radio_channel>=sat_a_prog_num[i])
        {
            p_grp->radio_channel = 0;//sat_a_prog_num[i] - 1;
        }
        if((sat_v_prog_num[i]>0) || (sat_a_prog_num[i]>0))
        {
            SYS_PRINTF("Sate=%d,v_prog_num=%d,a_prog_num=%d\n",i,sat_v_prog_num[i],sat_a_prog_num[i]);
        }

        total_v_prog_num += sat_v_prog_num[i];
        total_a_prog_num += sat_a_prog_num[i];
    }

    /* For all satellite group */
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][0];
    if(0 == total_v_prog_num)
    {
        p_grp->tv_channel = P_INVALID_ID;
    }
    else if(p_grp->tv_channel>=total_v_prog_num)
    {
        p_grp->tv_channel = 0;//total_v_prog_num - 1;
    }

    if(0 == total_a_prog_num)
    {
        p_grp->radio_channel = P_INVALID_ID;
    }
    else if(p_grp->radio_channel>=total_a_prog_num)
    {
        p_grp->radio_channel = 0;//total_a_prog_num - 1;
    }

    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1 + MAX_SAT_NUM + i];

        if(0 == fav_v_prog_num[i])
        {
            p_grp->tv_channel = P_INVALID_ID;
        }
        else if(p_grp->tv_channel>=fav_v_prog_num[i])
        {
            p_grp->tv_channel = 0;//fav_v_prog_num[i] - 1;
        }

        if(0 == fav_a_prog_num[i])
        {
            p_grp->radio_channel = P_INVALID_ID;
        }
        else if(p_grp->radio_channel>=fav_a_prog_num[i])
        {
            p_grp->radio_channel = 0;//fav_a_prog_num[i] - 1;
        }
        if((fav_v_prog_num[i]>0) || (fav_a_prog_num[i]>0))
        {
            SYS_PRINTF("fav=%d,v_prog_num=%d,a_prog_num=%d\n",i,fav_v_prog_num[i],fav_a_prog_num[i]);
        }
    }
    for(i=0;i<MAX_LOCAL_GROUP_NUM;i++)	
    {       
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1+MAX_SAT_NUM+MAX_FAVGROUP_NUM+i];        
        prog_num = 0;       
        if(p_sys_data->local_group_id[i])       
        {           
            recreate_prog_view(VIEW_BOUQUIT_ID|PROG_ALL_MODE,p_sys_data->local_group_id[i]);            
            prog_num = get_prog_num(VIEW_ALL|PROG_TV_MODE, 0);          
            if(prog_num==0)             
                p_grp->tv_channel = P_INVALID_ID;           
            else  if(p_grp->tv_channel >= prog_num)             //p_grp->tv_channel = prog_num - 1;            
                p_grp->tv_channel = 0;                      
            prog_num = get_prog_num(VIEW_ALL|PROG_RADIO_MODE, 0);           
            if(prog_num==0)             
                p_grp->radio_channel = P_INVALID_ID;            
            else if(p_grp->radio_channel >= prog_num)               //p_grp->radio_channel = prog_num - 1;                      
                p_grp->radio_channel = 0;       
        }       
        else        
        {           
            p_grp->tv_channel = P_INVALID_ID;           
            p_grp->radio_channel = P_INVALID_ID;        
        }   
    }
    SYS_PRINTF("All Sate %d,total_v_prog_num=%d,total_a_prog_num=%d\n",i,total_v_prog_num,total_a_prog_num);

}

/* Get channel group count */
UINT8 sys_data_get_group_num(void)
{
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT16 channel = 0;
    UINT8 av_flag = CUR_CHAN_MODE;

    group_num = 0;

    for(i=0;i<MAX_SAT_NUM;i++)
    {
        if(sys_data_get_sate_group_channel(i,&channel,av_flag))
        {
            group_num++;
        }
    }
    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        if(sys_data_get_fav_group_channel(i,&channel,av_flag))
        {
            group_num++;
        }
    }
    for(i=0;i<MAX_LOCAL_GROUP_NUM;i++)
    {
        if(sys_data_get_local_group_channel(i,&channel,av_flag))
            group_num++;
    }
    if(group_num>0)
    {
        group_num += 1;//+all group
    }

    PRINTF("group_num=%d\n",group_num);

    return group_num;
}

UINT8  sys_data_get_sate_group_num(UINT8 av_flag)
{
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT16 channel = 0;

    group_num = 0;

    for(i=0;i<MAX_SAT_NUM;i++)
    {
        if(sys_data_get_sate_group_channel(i,&channel,av_flag))
        {
            group_num++;
        }
    }
    return group_num;
}

UINT8 sys_data_get_fav_group_num(UINT8 av_flag)
{
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT16 channel = 0;

    group_num = 0;

    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        if(sys_data_get_fav_group_channel(i,&channel,av_flag))
        {
            group_num++;
        }
    }
    return group_num;
}

UINT8 sys_data_get_local_group_id(UINT8 pos)
{
	return system_config.local_group_id[pos];
}

BOOL sys_data_get_local_group_channel(UINT8 local_group,UINT16* channel, UINT8 av_flag)
{
	SYSTEM_DATA* p_sys_data;
	group_channel_t* p_grp;

	if(local_group>=MAX_LOCAL_GROUP_NUM)
		return FALSE;
	
	p_sys_data = sys_data_get();	
	p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1+MAX_SAT_NUM +MAX_FAVGROUP_NUM+local_group];
	
	if(p_grp->tv_channel == P_INVALID_ID && p_grp->radio_channel==P_INVALID_ID)
		return FALSE;
	
	if(av_flag==TV_CHAN)
	{
		if(p_grp->tv_channel == P_INVALID_ID)
			return FALSE;
		else
			*channel 		= p_grp->tv_channel;
	}
	else
	{
		if(p_grp->radio_channel==P_INVALID_ID)
			return FALSE;
		else
			*channel 	= p_grp->radio_channel;
	}
	return TRUE;
}
/* Get & Set current group index */
UINT8 sys_data_get_cur_group_index(void)
{
    UINT8 group_num = 0;
    BOOL b = FALSE;
    BOOL check_group = FALSE;
    UINT8 pre_chan_group_index = 0;
	if(0 == pre_chan_group_index)
	{
		;
	}

    while (1)
    {
        pre_chan_group_index = CUR_CHAN_GROUP_INDEX;

        for(;CUR_CHAN_GROUP_INDEX<= (MAX_GROUP_NUM - 1); CUR_CHAN_GROUP_INDEX++)
        {
            b = get_cur_group_idx(&group_num);
            if(b)
            {
                return group_num;
            }
        }

        if(!check_group)
        {
            sys_data_check_channel_groups();
            check_group = TRUE;
            continue;
        }
        break;
    }

    SYS_PRINTF("%s: No program!!!",__FUNCTION__);

    return 0;
}

/*Get the sata id of current group*/
UINT8 sys_data_get_cur_group_sat_id(void)
{
        UINT8 group_type = 0;
        UINT8 group_pos = 0;
        S_NODE s_node ;
        UINT16 cur_channel = 0;
        UINT8 cur_group_idx = 0;
        BOOL b = FALSE;

        cur_group_idx = sys_data_get_cur_group_index();
        MEMSET(&s_node, 0x0, sizeof (s_node));
        b = sys_data_get_cur_mode_group_infor(cur_group_idx, &group_type, &group_pos,&cur_channel);
        if(b)
        {
             if(SATE_GROUP_TYPE == group_type)
            {
                get_sat_at(group_pos,VIEW_ALL,&s_node);
                return s_node.sat_id;
            }
            else //FAV_GROUP_TYPE || ALL_SATE_GROUP_TYPE
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
}
void sys_data_set_cur_group_index(UINT8 group_idx)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT8 av_flag = 0;

    av_flag = CUR_CHAN_MODE;
    p_sys_data = sys_data_get();

    group_num = 0;

    for(i=0;i<MAX_GROUP_NUM;i++)
    {
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][i];
        if( ((TV_CHAN == av_flag) && (P_INVALID_ID != p_grp->tv_channel))
            || ((RADIO_CHAN == av_flag) && (P_INVALID_ID != p_grp->radio_channel)) )
        {

            if(group_num==group_idx)
            {
                CUR_CHAN_GROUP_INDEX = i;
                return;
            }
            else
            {
                group_num++;
            }
        }
    }
}


UINT8 sys_data_get_cur_intgroup_index(void)
{
    return CUR_CHAN_GROUP_INDEX;
}

void sys_data_set_cur_intgroup_index(UINT8 intgroup_idx)
{
    if(intgroup_idx < MAX_GROUP_NUM)
    {
        CUR_CHAN_GROUP_INDEX  = intgroup_idx;
    }
}


/* Get & set current group infor(channel index)  */
UINT16 sys_data_get_cur_group_cur_mode_channel(void)
{
    UINT16 cur_channel = 0;

    sys_data_get_cur_group_channel(&cur_channel,CUR_CHAN_MODE);
    return cur_channel;
}

void sys_data_set_group_channel(UINT8 group_idx, UINT16 channel)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 group_type, group_pos;
	UINT16 cur_channel;

	BOOL b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &cur_channel);

	if (b)
	{
		p_sys_data = sys_data_get();
		p_grp = &p_sys_data->cur_chan_group[group_idx];
		if (CUR_CHAN_MODE == TV_CHAN)
			p_grp->tv_channel = channel;
		else
			p_grp->radio_channel = channel;
	}
}

void sys_data_get_group_channel(UINT8 group_idx, UINT16 *channel, UINT8 av_flag)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 group_type, group_pos;
	UINT16 cur_channel;

	BOOL b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &cur_channel);

	if (b)
	{
		p_sys_data = sys_data_get();
		p_grp = &p_sys_data->cur_chan_group[group_idx];

		if (av_flag == TV_CHAN)
			*channel = p_grp->tv_channel;
		else if(av_flag == RADIO_CHAN)
			*channel = p_grp->radio_channel;
	}
}

void sys_data_get_pip_group_channel(UINT16 *channel, UINT8 av_flag)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->pip_group_idx][PIP_CHAN_GROUP_INDEX];

    if(TV_CHAN == av_flag)
    {
        *channel = p_grp->tv_channel;
    }
    else
    {
        *channel = p_grp->radio_channel;
    }
}

void sys_data_set_cur_group_channel(UINT16 channel)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][CUR_CHAN_GROUP_INDEX];
    if(TV_CHAN == CUR_CHAN_MODE)
    {
        p_grp->tv_channel = channel;
    }
    else
    {
        p_grp->radio_channel = channel;
    }
}

void sys_data_get_cur_group_channel(UINT16 *channel, UINT8 av_flag)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][CUR_CHAN_GROUP_INDEX];

    if(TV_CHAN == av_flag)
    {
        *channel = p_grp->tv_channel;
    }
    else
    {
        *channel = p_grp->radio_channel;
    }
}

BOOL sys_data_get_sate_group_channel(UINT16 sate_pos,UINT16 *channel, UINT8 av_flag)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    if(sate_pos>=MAX_SAT_NUM)
    {
        return FALSE;
    }

    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1 + sate_pos];

    if(TV_CHAN == av_flag)
    {
        if(P_INVALID_ID == p_grp->tv_channel)
        {
            return FALSE;
        }
        else
        {
            *channel = p_grp->tv_channel;
        }
    }
    else
    {
        if(P_INVALID_ID == p_grp->radio_channel)
        {
            return FALSE;
        }
        else
        {
            *channel = p_grp->radio_channel;
        }
    }

    return TRUE;
}

BOOL sys_data_get_fav_group_channel(UINT8 fav_group,UINT16 *channel, UINT8 av_flag)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    if(fav_group>=MAX_FAVGROUP_NUM)
    {
        return FALSE;
    }

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][1 + MAX_SAT_NUM + fav_group];

    if((P_INVALID_ID == p_grp->tv_channel) && (P_INVALID_ID == p_grp->radio_channel))
    {
        return FALSE;
    }

    if(TV_CHAN == av_flag)
    {
        if(P_INVALID_ID == p_grp->tv_channel)
        {
            return FALSE;
        }
        else
        {
            *channel = p_grp->tv_channel;
        }
    }
    else
    {
        if(P_INVALID_ID == p_grp->radio_channel)
        {
            return FALSE;
        }
        else
        {
            *channel = p_grp->radio_channel;
        }
    }

    return TRUE;
}

BOOL sys_data_get_cur_mode_group_infor(UINT8 group_idx, UINT8 *group_type,UINT8 *group_pos,UINT16 *channel)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT8 av_flag = 0;

    av_flag = CUR_CHAN_MODE;
    p_sys_data = sys_data_get();

    group_num = 0;

    for(i=0;i<MAX_GROUP_NUM;i++)
    {
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][i];
        if( ((1 == av_flag) && (P_INVALID_ID != p_grp->tv_channel))
            || ((0 == av_flag) && (P_INVALID_ID != p_grp->radio_channel)))
        {
            if(group_num==group_idx)
            {
                if(0 == i)
                {
                    *group_type = ALL_SATE_GROUP_TYPE;//all satallite
                }
                else if(i<=MAX_SAT_NUM)
                {
                    *group_type = SATE_GROUP_TYPE;//satallite
                    *group_pos    = i - 1;
                }
                else if(i<(1+MAX_SAT_NUM+MAX_FAVGROUP_NUM))
                {
                    *group_type = FAV_GROUP_TYPE;//fav
                    *group_pos    = i - 1 - MAX_SAT_NUM;
                }
		  else if(i<(1+MAX_SAT_NUM+MAX_FAVGROUP_NUM+MAX_LOCAL_GROUP_NUM))
		  {
		  	*group_type = LOCAL_GROUP_TYPE;//local
			*group_pos	= i - 1 - MAX_SAT_NUM-MAX_FAVGROUP_NUM;
		  }
		  else
		  {
		  	return FALSE;
		  }
                if(TV_CHAN == av_flag)
                {
                    *channel = p_grp->tv_channel;
                }
                else
                {
                    *channel = p_grp->radio_channel;
                }
                return TRUE;
            }
            else
            {
                group_num++;
            }
        }
    }

    return FALSE;
}

/* Change group index */
INT32 sys_data_change_group(UINT8 group_idx)
{
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    S_NODE s_node;
    UINT16 cur_channel = 0;
    BOOL b = FALSE;

    MEMSET(&s_node, 0x0, sizeof (s_node));
    b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos,&cur_channel);

    if(b)
    {
        if(ALL_SATE_GROUP_TYPE == group_type)
        {
            sys_data_set_cur_intgroup_index(0);
            return recreate_prog_view(VIEW_ALL|CUR_CHAN_MODE,0);
        }
        else if(SATE_GROUP_TYPE == group_type)
        {
            sys_data_set_cur_intgroup_index(group_pos + 1);
            get_sat_at(group_pos,VIEW_ALL,&s_node);
            return recreate_prog_view(VIEW_SINGLE_SAT|CUR_CHAN_MODE,s_node.sat_id);
        }
        else if(FAV_GROUP_TYPE == group_type)
        {
            sys_data_set_cur_intgroup_index(group_pos + 1 + MAX_SAT_NUM);
            return recreate_prog_view(VIEW_FAV_GROUP|CUR_CHAN_MODE,group_pos);
        }
    }
    else
    {
        sys_data_set_cur_intgroup_index(0);
        return recreate_prog_view(VIEW_ALL|CUR_CHAN_MODE,0);
    }

    return E_FAILURE;
}

BOOL sys_data_get_curprog_info(P_NODE *cur_prog_node)
{
    UINT8 cur_grp_idx = 0;
    UINT16 cur_prog = 0;

	if(0 == cur_grp_idx)
	{
		;
	}
    cur_prog = sys_data_get_cur_group_cur_mode_channel();
    cur_grp_idx = sys_data_get_cur_group_index();
    get_prog_at(cur_prog,cur_prog_node);

    return TRUE;/*Return value returned for Debug error code*/
}

static BOOL get_cur_group_idx(UINT8 *group_idx)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;
    UINT8 i = 0;
    UINT8 group_num = 0;
    UINT8 av_flag = 0;

    av_flag = CUR_CHAN_MODE;
    p_sys_data = sys_data_get();

    group_num = 0;

    for(i=0;i<MAX_GROUP_NUM;i++)
    {
        p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][i];
        if( ((TV_CHAN == av_flag) && (P_INVALID_ID != p_grp->tv_channel))
            || ((RADIO_CHAN == av_flag) && (P_INVALID_ID != p_grp->radio_channel)) )
        {

            if(i==CUR_CHAN_GROUP_INDEX)
            {
                *group_idx = group_num;
                return TRUE;
            }
            else
            {
                group_num++;
            }
        }
    }

    return FALSE;
}

/*******************************************************************************
* system data: dvr Extend APIs
********************************************************************************/

void sys_data_set_normal_group_channel(UINT16 channel)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->normal_group_idx][PIP_CHAN_GROUP_INDEX];
    if(TV_CHAN == PIP_CHAN_MODE)
    {
        p_grp->tv_channel = channel;
    }
    else
    {
        p_grp->radio_channel = channel;
    }
}

void sys_data_set_pip_group_channel(UINT16 channel)
{
    SYSTEM_DATA *p_sys_data = NULL;
    group_channel_t *p_grp = NULL;

    p_sys_data = sys_data_get();
    p_grp = &p_sys_data->cur_chan_group[p_sys_data->pip_group_idx][PIP_CHAN_GROUP_INDEX];
    if(TV_CHAN == PIP_CHAN_MODE)
    {
        p_grp->tv_channel = channel;
    }
    else
    {
        p_grp->radio_channel = channel;
    }
}

/* Get & set current group infor(channel index)  */
UINT16 sys_data_get_pip_group_cur_mode_channel(void)
{
    UINT16 cur_channel = 0;

    sys_data_get_pip_group_channel(&cur_channel,PIP_CHAN_MODE);
    return cur_channel;
}

void reset_group(void)
{
    UINT8 cur_group = 0;
    UINT16 cur_channel = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    S_NODE s_node;
    UINT8 cur_mode = 0;

    MEMSET(&s_node, 0x0, sizeof (s_node));

    cur_mode = sys_data_get_cur_chan_mode();
    if(0 == sys_data_get_group_num())
    {
        recreate_prog_view(VIEW_ALL|cur_mode,0);
        return;
    }

    cur_group = sys_data_get_cur_group_index();
    sys_data_set_cur_group_index(cur_group);
    sys_data_get_cur_mode_group_infor(cur_group, &group_type, &group_pos,&cur_channel);

    if(ALL_SATE_GROUP_TYPE == group_type)
    {
        recreate_prog_view(VIEW_ALL|cur_mode,0);
    }
    else if(SATE_GROUP_TYPE == group_type)
    {
        get_sat_at(group_pos,VIEW_ALL,&s_node);
        recreate_prog_view(VIEW_SINGLE_SAT|cur_mode,s_node.sat_id);
    }
    else
    {
        recreate_prog_view(VIEW_FAV_GROUP|cur_mode,group_pos);
    }
}

