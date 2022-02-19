/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_bat.c
*
*    Description: parse BAT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_bat.h>

//#if (SYS_PROJECT_FE == PROJECT_FE_DVBC||(SYS_PROJECT_FE == PROJECT_FE_DVBT && defined(STAR_PROG_SEARCH_BAT_SUPPORT)))
#if 1//def STAR_PROG_SEARCH_BAT_SUPPORT
//#define BAT_DEBUG
#ifdef BAT_DEBUG
#define BAT_PRINTF  libc_printf
#define BAT_EVENT_PRINTF libc_printf
#else
#define BAT_PRINTF(...)         do{}while(0)
#define BAT_EVENT_PRINTF(...)   do{}while(0)
#endif

static INT32 bat_on_bouqname_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv);
static INT32 bat_on_servicelist_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv);

static struct desc_table bat_loop1[] = {
    {
        BOUQUET_NAME_DESCRIPTOR,
        0,
        bat_on_bouqname_desc,
    },
};

static struct desc_table bat_loop2[] = {
    {
        SERVICE_LIST_DESCRIPTOR,
        0,
        bat_on_servicelist_desc,
    },

};

static struct bat_section_info *bat_info = NULL;
static UINT8 bat_get_finish = 0;
static UINT32 bat_first_got_tick = 0;

static INT32 bat_on_bouqname_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
    INT32 i = 0;
    struct bat_section_info *b_info = (struct bat_section_info *)priv;

    if((NULL == data) || (NULL == b_info))
    {
        return ERR_FAILUE;
    }

    //check if this banquet already eixst
    for(i=0; i<b_info->bouq_cnt; i++)
    {
        if(b_info->b_id==b_info->bouq_id[i])
        {
            if(len <= (2*(MAX_BOUQ_NAME_LENGTH + 1)))
            {
                MEMCPY(b_info->bouq_name[i], data, len);
            }
            break;
        }
    }

    if(i == b_info->bouq_cnt)
    {
        return ERR_FAILUE;
    }

    return SUCCESS;
}

static INT32 bat_on_servicelist_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
    INT32  i   = 0;
    INT32  j   = 0;
    UINT16 sid = 0;
    struct bat_section_info *b_info = (struct bat_section_info *)priv;

    if((NULL == data) || (NULL == b_info))
    {
        return !SUCCESS;
    }

    for(i=0; i<len; i += 3)
    {
        sid = (data[i]<<8)|data[i+1];
        for(j=0; j<b_info->service_cnt; j++)
        {
            if ((b_info->s_info[j].sid ==sid)&&
                (b_info->s_info[j].ts_id==b_info->ts_id) &&
                (b_info->s_info[j].bouq_id==b_info->b_id))
             {
                break;
             }
        }

        if((j == b_info->service_cnt)&&(j < PSI_MODULE_MAX_BAT_SINFO_NUMBER))
        {
            BAT_PRINTF("service id[%d], type[%d]\n",sid, data[i+2]);
            b_info->s_info[b_info->service_cnt].bouq_id = b_info->b_id;
            b_info->s_info[b_info->service_cnt].ts_id = b_info->ts_id;
            b_info->s_info[b_info->service_cnt].orig_netid = b_info->orig_netid;
            b_info->s_info[b_info->service_cnt].sid = sid;
            b_info->s_info[b_info->service_cnt].service_type = data[i+2];
            b_info->service_cnt++;
        }
    }

    return SUCCESS;
}

static BOOL bat_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *data, INT32 len)
{
    INT16  i = 0;
    struct bat_hitmap *hitmap = NULL;
    struct bat_section_info *pinfo = bat_info;

    if((NULL == pinfo) || (NULL == data))
    {
        BAT_EVENT_PRINTF("!!!%s(): buff ERR\n",__FUNCTION__);
        return FALSE;
    }

    BAT_EVENT_PRINTF("\n**bouqut_id=%d,sec_len=%d,sec_num=%d,last_sec=%d,tick=%d\n",
        (data[3]<<8)|data[4],((data[1]&0xf)<<8) |data[2],data[6],data[7],osal_get_tick());

    if(0 == bat_first_got_tick)
    {
        bat_first_got_tick = osal_get_tick();
    }

    for(i = 0; i < pinfo->bouq_cnt; i++)
    {
        //match bouquet id
        if(((data[3]<<8)|data[4])==pinfo->bouq_id[i])
        {
            break;
        }
    }

    if(i == pinfo->bouq_cnt)
    {
        pinfo->bouq_id[i] = (data[3]<<8)|data[4];
        pinfo->bouq_cnt++;
    }

    hitmap = &pinfo->hitmap[i];

    if(0 == hitmap->all_hit_value)   //compute all section hit value
    {
        hitmap->all_hit_value = 1;
        for(i = 0; i < data[7]; i++)
        {
            hitmap->all_hit_value = (hitmap->all_hit_value<<i)|1;
        }
    }

    if((1<<data[6])&hitmap->hit)     //this section got already
    {
        for(i = 0; i < pinfo->bouq_cnt; i++)   //check if all section hit
        {
            hitmap = &pinfo->hitmap[i];
            if((hitmap->hit!=hitmap->all_hit_value))
            {
                break;
            }
        }

        if(i != pinfo->bouq_cnt)
        {
            return FALSE;
        }
        else
        {
            bat_get_finish = 1;
            BAT_PRINTF("&&&bat_get_finish=1\n");
            //NOTE!!! return true to make bat handler free filter
            return TRUE;
        }

    }
    else
    {
        hitmap->hit |= 1<<data[6];
        return TRUE;
    }
}

static INT32 bat_parser(UINT8 *data, INT32 len)
{
    UINT8 min_sec_len      = 13;
    INT16 sec_len          = 0;
    INT16 bouquet_loop_len = 0;
    INT16 ts_loop_len      = 0;
    INT16 desc_loop_len    = 0;
    INT16 i                = 0;
    INT16 j                = 0;
    INT16 pos              = 0;
    struct bat_section_info *pinfo = bat_info;

    if(NULL == data)
    {
        return SUCCESS;
    }

    sec_len = ((data[1]&0xf)<<8) |data[2];
    pinfo->b_id = (data[3]<<8)|data[4];
    if(sec_len <= min_sec_len)
    {
        return SUCCESS;
    }

    bouquet_loop_len = ((data[8]&0xf)<<8) |data[9];
    pos = 10+bouquet_loop_len;
    BAT_PRINTF("bouquet descriptor loop len[%d]\n",bouquet_loop_len);

    for(i = 10; (i<pos)&&(i<sec_len-1); i += 2+data[i+1])
    {
        desc_loop_parser(data+i, data[i+1], bat_loop1, 1, NULL, pinfo);
    }

    ts_loop_len = ((data[pos]&0xf)<<8) |data[pos+1];
    BAT_PRINTF("ts_stream loop len[%d]\n",ts_loop_len);
    pos += 2+ts_loop_len;

    for(i += 2; (i<pos)&&(i<sec_len-1); i += 6+desc_loop_len)
    {
        pinfo->ts_id = (data[i]<<8)|data[i+1];
        pinfo->orig_netid = (data[i+2]<<8)|data[i+3];
        BAT_PRINTF("ts_id=%d,org_netid=%d\n",pinfo->ts_id,pinfo->orig_netid);
        desc_loop_len = ((data[i+4]&0xf)<<8)|data[i+5];
        for(j = i+6; (j<(i+6+desc_loop_len))&&(j<(sec_len-1)); j += 2+data[j+1])
        {
           desc_loop_parser(data+j, data[j+1], bat_loop2, 1, NULL, pinfo);
        }
    }
    return SUCCESS;
}

static sie_status_t bat_handler(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *data, INT32 len)
{
    INT16  i                       = 0;
    UINT16 bouquet_id              = 0x1FFF;
    struct bat_hitmap *hitmap      = NULL;
    struct bat_section_info *pinfo = bat_info;
    sie_status_t ret               = sie_started;

    if(SIE_REASON_FILTER_TIMEOUT == reason)
    {
        BAT_PRINTF("BAT get time out\n");
        if(g_bat_semaphore!=OSAL_INVALID_ID)
        {
            osal_semaphore_release(g_bat_semaphore);
        }

        ret = sie_freed;
    }
    else if(NULL != data)
    {
        bouquet_id = (data[3]<<8)|data[4];
        for(i = 0; i < pinfo->bouq_cnt; i++)
        {
            if(bouquet_id== pinfo->bouq_id[i])
            {
                break;
            }
        }

        if(i == pinfo->bouq_cnt)
        {
            BAT_PRINTF("%s(): bouquet id[%d] not found!\n",bouquet_id);
            return ret;
        }
        hitmap = &pinfo->hitmap[i];
        if(0 == (hitmap->parse&(1<<data[6])))
        {
            bat_parser(data, len);
            hitmap->parse |= (1<<data[6]);
        }
        if(1 == bat_get_finish)
        {
            for(i = 0; i < pinfo->bouq_cnt; i++)
            {
                if(pinfo->hitmap[i].parse!=pinfo->hitmap[i].all_hit_value)
                {
                    break;
                }
            }

            if(i != pinfo->bouq_cnt)
            {
                BAT_PRINTF("%s(): bat id[%d] not all section pased\n",__FUNCTION__,pinfo->bouq_id[i]);
                return ret;
            }
            BAT_PRINTF("%s(): all [%d] bat id parsed,tick=%d\n",__FUNCTION__,pinfo->bouq_cnt,osal_get_tick());

            bat_get_finish = 0;

            if(OSAL_INVALID_ID != g_bat_semaphore)
            {
                osal_semaphore_release(g_bat_semaphore);
            }

            return sie_freed;
        }

    }
    return ret;
}

INT32 si_bat_request_start(void)
{
    INT32  ret = 0;
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;

    if(NULL == bat_info)
    {
       bat_info = (struct bat_section_info *)MALLOC(sizeof(struct bat_section_info));
    }

    if((NULL == bat_info))
    {
        BAT_PRINTF("%s(), malloc memory failed!!!\n",__FUNCTION__);
        return !SUCCESS;
    }

    MEMSET(bat_info, 0, sizeof(struct bat_section_info));
    filter = sie_alloc_filter(PSI_BAT_PID, bat_info->buf, sizeof(bat_info->buf), PSI_SHORT_SECTION_LENGTH);
    if(NULL == filter)
    {
        FREE(bat_info);
        bat_info = NULL;
        BAT_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
        return -1;
    }
    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    fparam.timeout = 20000;
    fparam.attr[0] = SI_ATTR_HAVE_CRC32;
    fparam.mask_value.tb_flt_msk = 0x01;
    fparam.mask_value.mask_len = 6;
    fparam.mask_value.multi_mask[0][0] = 0xFF;
    fparam.mask_value.multi_mask[0][1] = 0x80;
    fparam.mask_value.multi_mask[0][5] = 0x01;
    fparam.mask_value.value[0][0] = PSI_BAT_TABLE_ID;
    fparam.mask_value.value[0][1] = 0x80;
    fparam.mask_value.value[0][5] = 0x01;

    fparam.section_event = bat_event;
    fparam.section_parser = bat_handler;
    sie_config_filter(filter, &fparam);
    ret = sie_enable_filter(filter);
    if(ret != SI_SUCCESS)
    {
        BAT_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
        return -1;
    }
    bat_first_got_tick = 0;

    return SUCCESS;
}

INT32 si_bat_get_info(struct bat_section_info *bsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
    INT32 i = 0;
    INT32 cnt = 0;
    struct bat_service_info *bat_sinfo = NULL;

    if((NULL == bat_info) || (NULL == pg))
    {
        return -1;
    }

    bat_sinfo = bat_info->s_info;
    cnt = bat_info->service_cnt;
    for(i=0; i<cnt; i++)
    {
        if ((bat_sinfo[i].sid == pg->prog_number)/*&&(bat_sinfo[i].ts_id == tsid)*/)
        {
            pg->bouquet_id = bat_sinfo[i].bouq_id;
            return SI_SUCCESS;
        }
    }
    return ERR_FAILUE;
}

/*static UINT16 si_bat_get_bouq_cnt(UINT16 *bouq_id, UINT8 *bouq_name, UINT16 max_name_len)
{
    INT16 i = 0;
    UINT16 len = 0;
    UINT16 offset = 0;

    if(bat_info==NULL)
    {
        return 0;
    }

    MEMCPY(bouq_id, bat_info->bouq_id, bat_info->bouq_cnt*2);
    if(max_name_len<=2*(MAX_BOUQ_NAME_LENGTH + 1))
    {
        len = max_name_len;
    }
    else
    {
        len = 2*(MAX_BOUQ_NAME_LENGTH + 1);
    }

    for(i=0; i< bat_info->bouq_cnt; i++)
    {
        MEMCPY(bouq_name+offset, bat_info->bouq_name[i],len);
        offset += max_name_len;
    }

    return bat_info->bouq_cnt;
}

static INT32 si_bat_release_bouq_info(void)
{
    if(bat_info!=NULL)
    {
        FREE(bat_info);
        bat_info = NULL;
        bat_get_finish = 0;
    }

    return SI_SUCCESS;
}*/

#endif

