/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: satcodx.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _S3281_
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/sci/sci.h>
#include "win_com.h"
#include "satcodx.h"
#include "key.h"
#include "control.h"

#define SATCODX_PAGE    128

/* 0: auto,1: H,2: V */
#define     HOR     0
#define     VER     1
#define     AUTO    2

#define SATCODX_THRESHOLD    8
#define FIRST_READ_TIMEOUT   40
#define C_KU_MID             ((5750 + 9750)/2)
#define SATCODX_HEADER_LEN   8
#define INVALID_SAT_ID       0xFFFFFFFF


/*get the address in video buffer according the video_buf base address and the linear offset */
#define __PV(base,offset)   (volatile UINT8*)((base)+(offset))

__MAYBE_UNUSED__ static INT32    db_sat_num = -1;
static UINT32 parsed_sats_ids[MAX_SAT_NUM] = {0};

static INT32 find_sat(UINT8 *name,UINT32 *sat_id)
{
    int num = 0;
    int i = 0;
    S_NODE s;

	if((NULL==name) || (NULL==sat_id))
	{
		return !SUCCESS;
	}

    MEMSET(&s, 0x0, sizeof (s));

    num=get_sat_num(VIEW_ALL);

    for (i=0;i<num;i++)
    {
        get_sat_at(i,VIEW_ALL,&s);
#ifndef DB_USE_UNICODE_STRING
        if (0==strcmp(name,s.sat_name))
#else
        if (0== com_uni_str_cmp_ext(name,s.sat_name))
#endif
        {
            *sat_id=s.sat_id;
            return SUCCESS;
        }
    }

    return !SUCCESS;
}

static INT32 find_sat_ext(UINT8 *name,UINT32 *sat_id,UINT32 orbit,UINT32 tpfreq)
{
    int num = 0;
    int i = 0;
    S_NODE s_node;
    INT32 ret = 0;

	if((NULL==name) || (NULL==sat_id))
	{
		return !SUCCESS;
	}

    MEMSET(&s_node, 0x0, sizeof (s_node));

    num=get_sat_num(VIEW_ALL);

    for (i=0;i<num;i++)
    {
        get_sat_at(i,VIEW_ALL,&s_node);

        if(s_node.sat_orbit == orbit)
        {
            if((s_node.lnb_low >= C_KU_MID) && (tpfreq>=C_KU_MID))
            {
                *sat_id = s_node.sat_id;
                return SUCCESS;
            }
            else if((s_node.lnb_low < C_KU_MID) && (tpfreq<C_KU_MID))
            {
               *sat_id = s_node.sat_id;
                return SUCCESS;
            }
        }
    }

    ret = find_sat(name,sat_id);
    if(ret== SUCCESS)
    {
        return SUCCESS;
    }
    return !SUCCESS;

}


static INT32 find_tp(UINT32 sat_id,UINT32 freq,UINT32 sym,UINT32 pol,UINT32 *tp_id)
{
    int num = 0;
    int i = 0;
    T_NODE t;

	if(NULL == tp_id)
	{
		return !SUCCESS;
	}

	MEMSET(&t, 0, sizeof(T_NODE));
    num=get_tp_num_sat(sat_id);
    for (i=0;i<num;i++)
    {
        get_tp_at(sat_id, i, &t);
        if ((t.frq==freq) && (t.sym==sym) && (t.pol==pol))
        {
            *tp_id=t.tp_id;
            return SUCCESS;
        }
    }

    return !SUCCESS;
}

static INT32 atoi_ex(UINT8 *str,UINT32 len)
{
    UINT8 tmp[8] = {0};
    INT32 i = 0;

	if(NULL == str)
	{
		return -1;
	}

    if (len>SATCODX_THRESHOLD)
    {
        len=7;
    }
    MEMCPY(tmp,str,len);
    i = 0;
    while(tmp[i] == '_' && (UINT32)i<len)
    {
        tmp[i++] = '0';
    }
    for(i=len -1;i>=0;i--)
    {
        if(tmp[i] == '_')
        {
            tmp[i] = '\0';
        }
    }
    tmp[len]='\0';
    return ATOI((const char *)tmp);
}

static INT32 truncate_space(UINT8 *str)
{
    INT32 i=0;
    UINT8 *p=str;

    if (str)
    {
        do
        {
            i++;
            p++;
        }while(*p!='\0');

        do
        {
            p--;
            i--;
        }while((*p==' ') && (i!=0));
    
        *(p+1)='\0';
    }

    return i;
}

static void str_cap(UINT8 *str)
{
    INT32 i=0;

    if(NULL != str)
	{
	    while(str[i]!='\0')
	    {
	        if ((str[i]>='a') && (str[i]<='z'))
	        {
	            str[i]-='a'-'A';
	        }
	        i++;
	    }
    }
}

static void get_sat_info(UINT8 *satcodx_page,S_NODE *s)
{
    UINT8   tmp[MAX_SATELLITE_NAME_LENGTH+1];
    UINT8 *p=satcodx_page+10;
    INT32 n = 0;

	if((NULL==satcodx_page)||(NULL==s))
	{
		return ;
	}

    n = (18 > MAX_SATELLITE_NAME_LENGTH)? MAX_SATELLITE_NAME_LENGTH : 18;

    MEMCPY(tmp,p, n);
    tmp[n]='\0';
    truncate_space(tmp);
#ifndef DB_USE_UNICODE_STRING
    str_cap(tmp);
    MEMSET(s->sat_name, 0x0, 2*(MAX_SERVICE_NAME_LENGTH + 1));
    strncpy(s->sat_name,tmp,(2*(MAX_SERVICE_NAME_LENGTH + 1)-1));
#else
    com_asc_str2uni(tmp, (UINT16 *) s->sat_name);
#endif

    p=satcodx_page+51;
    MEMCPY(tmp,p,4);
    tmp[4]='\0';
    s->sat_orbit=atoi_ex(tmp,4);

    return ;
}

static INT32    get_tp_info(UINT8 *satcodx_page,T_NODE *t)
{
    UINT8 *p=satcodx_page;

	if((NULL==satcodx_page) || (NULL==t))
	{
		return 0;
	}
	
    p=satcodx_page+33;
    t->frq=atoi_ex(p,6);

    p=satcodx_page+42;
    if (*p=='0' || *p=='3')
    {
        t->pol=VER;
    }
    else if (*p=='1' || *p=='2')
    {
        t->pol=HOR;
    }
    else
    {
        return 0;
    }
    p=satcodx_page+69;
    t->sym=atoi_ex(p,5);

    p=satcodx_page+97;
    t->t_s_id=atoi_ex(p,5);

    p=satcodx_page+92;
    t->network_id=atoi_ex(p,5);

    return 1;

}

static INT32 get_prog_info(UINT8 *satcodx_page, P_NODE *prog)
{
    UINT8 tmp[MAX_SERVICE_NAME_LENGTH+1];
    UINT8 *pt=satcodx_page;
    INT32 n = 0;

	if((NULL == satcodx_page)||(NULL==prog))
	{
		return 0;
	}
    if ('T'==pt[28])
    {
        prog->av_flag=1;
        pt=satcodx_page+75;
        prog->video_pid=atoi_ex(pt,4)&0x1FFF;
    }
    else if('R'==pt[28])
    {
        prog->av_flag=0;
        prog->video_pid=0x1FFF;
    }
    else
    {
        return 0;
    }
    pt=satcodx_page+79;
    prog->audio_pid[0]=(UINT32)atoi_ex(pt,4)&0x1FFF;
    prog->audio_count=1;
#ifdef SEARCH_DEFAULT_AUDIO_CHANNEL
    prog->audio_channel = SEARCH_DEFAULT_AUDIO_CHANNEL;
#endif
    pt=satcodx_page+83;
    prog->pcr_pid=atoi_ex(pt,4)&0x1FFF;

    pt=satcodx_page+87;
    prog->prog_number=atoi_ex(pt,5);

    pt=satcodx_page+43;
    MEMCPY(tmp,pt,8);
    pt=satcodx_page+115;
    n = (12 > (MAX_SERVICE_NAME_LENGTH-8) ) ?  (MAX_SERVICE_NAME_LENGTH-8) : 12;
    MEMCPY(&tmp[8],pt,n);
    tmp[MAX_SERVICE_NAME_LENGTH]='\0';
    truncate_space(tmp);

#ifndef DB_USE_UNICODE_STRING
    MEMSET(prog->service_name, 0x0, 2*(MAX_SERVICE_NAME_LENGTH + 1));
    strncpy(prog->service_name,tmp, (2*(MAX_SERVICE_NAME_LENGTH + 1)-1));
#else
    com_asc_str2uni((UINT8*)tmp,(UINT16*)prog->service_name);
#endif

    return 1;

}


static UINT8 *get_next_channel(UINT8 *base,UINT8 *data, UINT32 *offset,UINT32 count)
{
    UINT8 *p=data;
    UINT32 i,j;
    INT32   flag=0;
    static char satcodx_header[] = "SATCODX1";

	if((NULL==base)||(NULL==data)||(NULL==offset) )
	{
		return NULL;
	}

    i=*offset;

//FIND_SATCODX_HEADER:
  
    while( i+7<count)
    {
        if(MEMCMP(satcodx_header,base+i,SATCODX_HEADER_LEN) == 0)
        {
            if( i + SATCODX_PAGE - 1 == count)
            {
                flag = 1;
                break;
            }
            else if( i + SATCODX_PAGE - 1 + SATCODX_HEADER_LEN <= count )
            {
                if(MEMCMP(satcodx_header,base+i + SATCODX_PAGE - 1,SATCODX_HEADER_LEN) == 0)
                {
                    flag=1;
                    break;
                }
            }
        }
        i++;
    };


    if (flag)
    {
        if( i + SATCODX_PAGE - 1> count)
        {
            return NULL;
        }
        for (j=0;j<SATCODX_PAGE-1;j++)
        {
            *p++= *(__PV(base,i));
            i++;
        }
    }

    *offset=i;
    return (flag)? data : NULL;

}

void unicode_to_ascii(UINT8 *ascii_str, UINT8 *unicode_str)
{
    INT32 k;

    for(k=0;k<MAX_SERVICE_NAME_LENGTH;k++)
    {
        ascii_str[k] =unicode_str[2*k+1];
    }
}

INT32 satcodx_download(struct satcdx_buf *bufs, UINT32 buf_cnt, satcodx_proc_callback callback)
{
    UINT32 i = 0;
    UINT32 cnt = 0;
    UINT32 buf_idx = 0;
    UINT32 buf_len = 0;
    UINT32 bytes = 0;
    INT32 time_out = 0;
    INT32 ret = 0;
    struct satcdx_buf *sat_buf = NULL;
    UINT8 *buf_addr= NULL;

	if(NULL == bufs)
	{
		goto RETURN;
	}

    for(i=0;i<buf_cnt;i++)
    {
        sat_buf = &bufs[i];
        sat_buf->data_len = 0;
    }

    ap_clear_all_message();
    osal_task_dispatch_off();
    sci_mode_set(app_uart_id,115200,SCI_PARITY_NONE);
    sci_clear_buff(app_uart_id);

    bytes = 0;

    buf_idx = 0;
    buf_addr = bufs[buf_idx].buf_addr;
    buf_len = bufs[buf_idx].buf_len / SATCODX_PAGE * SATCODX_PAGE;

    time_out = FIRST_READ_TIMEOUT;
//FIRST_READ: // 20 seconds

    if(callback)
    {
        if(callback(SATCODX_EVENT_DOWNLOAD_WAITDATA,time_out/2,NULL,NULL,NULL) != 0)
        {
            goto ABORT;
        }
    }

    while(time_out)
    {
        ret = sci_read_tm(app_uart_id,buf_addr,500000); /* 0.5 seconds */

        if(callback)
        {
            if(callback(SATCODX_EVENT_DOWNLOAD_WAITDATA,time_out/2,NULL,NULL,NULL) != 0)
            {
                goto ABORT;
            }
        }

        if(ret == SUCCESS)
        {
            break;
        }
        time_out--;
    }

    if( time_out<= 0)
    {
        goto NO_DATA;
    }
    bytes = cnt = 1;

    time_out= 3000000;


//NEXT_READ:

    for(;buf_idx<buf_len;buf_idx++,cnt=0)
    {
        buf_addr = bufs[buf_idx].buf_addr;
        buf_len = bufs[buf_idx].buf_len / SATCODX_PAGE * SATCODX_PAGE;

        while(cnt < buf_len)
        {
            ret = sci_read_tm(app_uart_id,buf_addr + cnt,time_out); /* 3 seconds */
            if(ret != SUCCESS)
            {
                bufs[buf_idx].data_len = cnt;
                ret = 0;
                goto RETURN;
            }
            else
            {
                cnt++;
                bytes++;
                bufs[buf_idx].data_len++;
                if(0 == (bytes % 1024) && callback)
                {
                    if(callback(SATCODX_EVENT_DOWNLOAD_GETDATA,bytes,NULL,NULL,NULL) != 0)
                    {
                        goto ABORT;
                    }
                }
            }
        }
    }

    if(buf_idx == buf_len && bufs[buf_idx - 1].data_len == buf_len)
        goto BUFFER_FULL;

RETURN:
    ret = 0;
    goto END;

ABORT:
    ret = -1;
    goto END;

NO_DATA:
    ret = -2;
    goto END;

BUFFER_FULL:
    ret = -3;
    goto END;

END:

    sci_clear_buff(app_uart_id);
    sci_mode_set(app_uart_id,115200,SCI_PARITY_EVEN);
    osal_task_dispatch_on();

    pan_buff_clear();
    ap_clear_all_message();

    if(callback)
    {
        callback(SATCODX_EVENT_DOWNLOAD_END,(UINT32)(-ret),NULL,NULL,NULL);
    }

    return ret;
}

INT32 satcodx_parse(struct satcdx_buf *bufs, UINT32 buf_cnt, satcodx_proc_callback callback)
{
    UINT32 event_type = 0;
    //UINT32 i = 0;
    UINT32 j = 0;
    UINT32 buf_idx = 0;
    UINT32 data_len =0;
    INT32 ret = 0;
    //struct satcdx_buf *sat_buf = NULL;
    UINT8 *buf_addr = NULL;
    UINT16 ch_cnt = 0;
    UINT16 sat_cnt = 0;
    UINT16 tp_cnt = 0;
    UINT32 offset = 0;
    UINT8 data[SATCODX_PAGE] = {0};
    UINT8 *pt = NULL;
    S_NODE  s_node;
    T_NODE  t_node;
    P_NODE  p_node;
    P_NODE pre_p_node;
    UINT32 sat_id = 0xFFFF;
    UINT32 tp_id = 0xFFFF;
    UINT32 pre_sat_id = 0xFFFFFFF;
    UINT32 old_freq = 0xFFFFFFF;
    UINT32 old_sym = 0xFFFFFFF;
    UINT32 old_pol = 0xFFFFFFF;
    UINT32 sat_changed = 0;
    UINT32 add_sat = 0;
    UINT32 tp_changed = 0;

	if(0 == add_sat)
	{
		;
	}
	if(NULL == bufs)
	{
		goto RETURN;
	}

	MEMSET(&pre_p_node, 0, sizeof(P_NODE));
    pre_p_node.pcr_pid |= 0xFFFF;
    pre_p_node.video_pid |= 0xFFFF;

    for(j=0;j<MAX_SAT_NUM;j++)
    {
        parsed_sats_ids[j] = INVALID_SAT_ID;
    }

    for(buf_idx=0;buf_idx<buf_cnt;buf_idx++)
    {
        /* Parse each buffer */
        buf_addr = bufs[buf_idx].buf_addr;
        data_len = bufs[buf_idx].data_len;
        offset = 0;

        if(0 == data_len)
        {
            break;
        }
        /* Parser one buffer */
        while(get_next_channel(buf_addr,data,&offset,data_len) != NULL)
        {
            MEMSET(&s_node, 0, sizeof(S_NODE));
            MEMSET(&t_node, 0, sizeof(T_NODE));
            MEMSET(&p_node, 0, sizeof(P_NODE));

            pt = data;

            /*satellite */
            get_sat_info(pt,&s_node);

            /* tp */
            if(0 == get_tp_info(pt,&t_node))    /* If Polarity not Hor or Ver don't add  */
            {
                continue;
            }
            if ((0 == t_node.sym) || (0 == t_node.frq))
            {
                continue;
            }
            //channel
            if(0 == get_prog_info(pt, &p_node))
            {
                continue;
            }

            /* Satellite  */
            sat_changed = 0;
            add_sat = 0;
            ret = find_sat_ext(s_node.sat_name,&sat_id,s_node.sat_orbit,t_node.frq);
            if(SUCCESS != ret)
            {
                /* Add satellite */
                if ((5150+11300)/2 < t_node.frq)
                {
                    s_node.lnb_low = 9750;
                    s_node.lnb_high = 10600;
                    s_node.lnb_type = LNB_CTRL_22K;

                    s_node.tuner2_antena.lnb_low = 9750;
                    s_node.tuner2_antena.lnb_high = 10600;
                    s_node.tuner2_antena.lnb_type = LNB_CTRL_22K;

                }
                else
                {
                    s_node.lnb_low = s_node.lnb_high = 5150;
                    s_node.tuner2_antena.lnb_low  = s_node.tuner2_antena.lnb_high = 5150;
                }

                recreate_sat_view(VIEW_ALL, 0);
                ret = add_node(TYPE_SAT_NODE,0,&s_node);
                if(DBERR_FLASH_FULL == ret)
                {
                    goto FLASH_FULL;
                }
                else if(DBERR_MAX_LIMIT == ret)
                {
                    goto REACH_TO_MAXSAT;
                }
                else if(SUCCESS != ret)
                {
                    goto RETURN;
                }
                update_data();

                sat_id = s_node.sat_id;
                old_freq = 0xFFFFFFF;
                old_sym  = 0xFFFFFFF; 
                old_pol  = 0xFFFFFFF;

                /* filling parsed satellite */
                for(j=0;j<MAX_SAT_NUM;j++)
                {
                    if(parsed_sats_ids[j] == sat_id)
                    {
                        break;
                    }
                }
                if(MAX_SAT_NUM==j)
                {
                    for(j=0;j<MAX_SAT_NUM;j++)
                    {
                        if(INVALID_SAT_ID == parsed_sats_ids[j])
                        {
                            parsed_sats_ids[j] = sat_id;
                            break;
                        }
                    }
                }
                sat_changed = 1;
                add_sat = 1;
                sat_cnt++;
            }
            else
            {
                if(pre_sat_id != sat_id)
                {
                    /* Check if current satellite already parsed aready*/
                    for(j=0;j<MAX_SAT_NUM;j++)
                    {
                        if(parsed_sats_ids[j] == sat_id)
                        {
                            break;
                        }
                    }
                    if(MAX_SAT_NUM==j)  /* If current satellite not parsed before,delete TP&Channel */
                    {
                        recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE ,0);
                        del_child_prog(TYPE_SAT_NODE, sat_id);
                        update_data();


                        recreate_tp_view(VIEW_ALL ,0);
                        del_tp_on_sat(sat_id);
                        update_data();

                        for(j=0;j<MAX_SAT_NUM;j++)
                        {
                            if(parsed_sats_ids[j] == INVALID_SAT_ID)
                            {
                                parsed_sats_ids[j] = sat_id;
                                break;
                            }
                        }

                        sat_cnt ++;
                    }
                    old_freq = 0xFFFFFFF;
                    old_sym = 0xFFFFFFF;
                    old_pol = 0xFFFFFFF;
                    pre_sat_id = sat_id;
                    sat_changed = 1;
                    get_sat_by_id(sat_id,&s_node);
                }
            }

            /* TP */
            tp_changed = 0;
            if ((t_node.frq!=old_freq) || (t_node.sym!=old_sym) || (t_node.pol !=old_pol) || (sat_changed))
            {
                recreate_tp_view(VIEW_ALL ,0);
                ret = add_node(TYPE_TP_NODE, sat_id, &t_node);
                if(DBERR_FLASH_FULL == ret)
                {
                    goto FLASH_FULL;
                }
                else if(ret == DBERR_MAX_LIMIT)
                {
                    goto REACH_TO_MAXTP;
                }
                else if(SUCCESS != ret)
                {
                    goto RETURN;
                }
                update_data();

                tp_cnt++;
                old_freq=t_node.frq;
                old_sym=t_node.sym;
                old_pol=t_node.pol;
                tp_changed = 1;

                if(callback)
                {
                    event_type = sat_changed? SATCODX_EVENT_PARSE_ADDSATTP : SATCODX_EVENT_PARSE_ADDTP;
                    ret = callback(event_type, (sat_cnt<<16)+tp_cnt, &s_node, &t_node, NULL);
                    if(0 != ret)
                    {
                        goto ABOUT;
                    }
                }

                tp_id = t_node.tp_id;
            }

            /* Channel */
            if ( !((0 == p_node.video_pid) && (0 == p_node.audio_pid[0])) )
            {
                recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE,0);
                if(0 == p_node.video_pid)
                {
                    p_node.video_pid = 0x1FFF;
                }
                if(0 == p_node.audio_pid[0])
                {
                    p_node.audio_pid[0] = 0x1FFF;
                }
                if(0 == p_node.pcr_pid)
                {
                    p_node.pcr_pid = 0x1FFF;
                }
                if((!tp_changed) && (pre_p_node.pcr_pid==p_node.pcr_pid) && (pre_p_node.video_pid== p_node.video_pid)
                    && (0 == com_uni_str_cmp_ext(pre_p_node.service_name, p_node.service_name)))
                {
                    if(MAX_AUDIO_CNT < pre_p_node.audio_count)
                    {
                        pre_p_node.audio_pid[pre_p_node.audio_count] = p_node.audio_pid[0];
                        pre_p_node.audio_lang[pre_p_node.audio_count] = p_node.audio_lang[0];
                        pre_p_node.audio_count++;
                        modify_prog(pre_p_node.prog_id,&pre_p_node);
                        update_data();
                    }

                    continue;
                }

                ret = add_node(TYPE_PROG_NODE, tp_id,&p_node);
                if(DBERR_FLASH_FULL == ret)
                {
                    goto FLASH_FULL;
                }
                else if(DBERR_MAX_LIMIT == ret)
                {
                    goto REACH_TO_MAXCH;
                }
                else if(SUCCESS != ret)
                {
                    goto RETURN;
                }
                else
                {
                    ch_cnt++;
                    pre_p_node = p_node;
                    update_data();

                    if(callback)
                    {
                        ret = callback(SATCODX_EVENT_PARSE_CHANNEL,ch_cnt,&s_node,&t_node,&p_node);
                        if(0 != ret)
                        {
                            goto ABOUT;
                        }
                    }
                }
            }
        }
    }

RETURN:
    ret = 0;
    goto END;

ABOUT:
    ret = -1;
    goto END;

FLASH_FULL:
    ret = -2;
    goto END;

REACH_TO_MAXSAT:
    ret = -3;
    goto END;

REACH_TO_MAXTP:
    ret = -4;
    goto END;

REACH_TO_MAXCH:
    ret = -5;
    goto END;

END:
    if(callback)
    {
        callback(SATCODX_EVENT_PARSE_END,(UINT32)(-ret),NULL,NULL,NULL);
    }
    return ret;
}
#endif

