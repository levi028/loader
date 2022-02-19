/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_multifeed.c
*
*    Description: include main function about multifeed
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/si_utility.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/psi_db.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/lib_epg.h>
#include <api/libsi/lib_multifeed.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_as.h>

#ifdef MULTIFEED_SUPPORT

#if 0
#define __DBG_MF_ 1
#define mf_debug(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define __DBG_MF_ 0
#define mf_debug(...)   do{} while(0)
#endif

struct si_descriptor
{
    UINT8 tag;
    UINT8 len;
    UINT8 data[0];
};

#define MULTIFEED_SUCCESS 0
#define MULTIFEED_FAILURE 1
#define FEED_INFO_CMP_SIZE (sizeof(struct FEED_INFO) - (UINT32)(&((struct FEED_INFO *)0)->transport_stream_id))

static MULTIFEED_CALL_BACK multifeed_cb = NULL;
static struct MULTIFEED_INFO *multifeed_info_head = NULL;
static UINT8 multifeed_table_buff[1024] = {0};
static UINT8 multifeed_pat_buff[1024] = {0};
static UINT8 multifeed_pmt_buff[1024] = {0};

static INT32 search_pat_buff(UINT8 *buff, INT32 buff_len, UINT16 service_id, UINT16 *pmt_pid)
{
    UINT16 n = 0;
    UINT16 sec_len = 0;
    UINT16 sid = 0;
    UINT32 ret = MULTIFEED_FAILURE;

    if((NULL == pmt_pid) || (NULL == buff) || (buff[0] != 0x00))
    {
        return MULTIFEED_FAILURE;
    }

    sec_len = ((buff[1]&0x0F)<<8) | buff[2];
    for( n += 8; n < (sec_len - 1); n += 4)
    {
        sid = (buff[n]<<8 )|buff[n+1];
        if( sid == service_id )
        {
            *pmt_pid = ( buff[n+2]<<8 | buff[n+3] ) & 0x1FFF;
            ret = MULTIFEED_SUCCESS;
        }
    }
    mf_debug("%s,  sid: %d, pmt: %d\n", __FUNCTION__, service_id, *pmt_pid );

    return ret;
}

static INT32 mf_find_info_in_pat(UINT8 dmx_id, UINT16 ts_id, UINT16 service_id, UINT16 *pmt_pid )
{
    INT32 ret = 0;
    UINT8 cnt = 0;
    UINT8 max_cnt = 50;
    UINT8 section_num = 0;
    UINT8 last_section_num = 0;
    UINT16 transport_id = 0xFFFF;
    struct dmx_device *dmx_dev = NULL;

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    if(NULL == dmx_dev)
    {
        return MULTIFEED_FAILURE;
    }

    struct get_section_param sr_request;
    struct restrict sr_restrict;

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));

    sr_request.buff = multifeed_pat_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = PSI_PAT_PID;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 2000;

    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_PAT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;

    section_num =0;

    do
    {
        sr_restrict.value[0][6] = section_num;
        cnt = 0;
        while((transport_id != ts_id) && (cnt < max_cnt))
        {
            if(dmx_req_section(dmx_dev, &sr_request)!= SUCCESS)
            {
                return MULTIFEED_FAILURE;
            }
            transport_id = multifeed_pat_buff[3]<<8 | multifeed_pat_buff[4];
            cnt++;
        }
        last_section_num = multifeed_pat_buff[7];

        mf_debug("%s, get PAT success\n", __FUNCTION__ );
        ret = search_pat_buff( multifeed_pat_buff, 1024, service_id, pmt_pid );

        if(MULTIFEED_SUCCESS == ret)
        {
            break;
        }
        section_num++;
    }while(section_num<=last_section_num);

    return ret;
}


static INT32 get_pid_in_pmt(UINT8 dmx_id, UINT16 pmt_pid, P_NODE *p_node )
{
    INT32 ret = MULTIFEED_FAILURE;
    UINT8 section_num = 0;
    UINT8 last_section_num = 0;
    UINT8 cnt = 0;
    UINT8 max_cnt = 5;
    struct dmx_device * dmx_dev = NULL;
    PROG_INFO program;
    P_NODE new_node;

    #if __DBG_MF_
    UINT16 i = 0;
    UINT16 len = 0;
    #endif

    MEMSET(&program, 0, sizeof(PROG_INFO));
    MEMSET(&new_node, 0, sizeof(P_NODE));

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    if(NULL == dmx_dev)
    {
        return ret;
    }

    struct get_section_param sr_request;
    struct restrict sr_restrict;

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));

    sr_request.buff = multifeed_pmt_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = pmt_pid;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 2000;

    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_PMT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;

    section_num =0;
    do
    {
        sr_restrict.value[0][6] = section_num;

        if(dmx_req_section(dmx_dev, &sr_request)!= SUCCESS)
        {
            return MULTIFEED_FAILURE;
        }
        mf_debug("%s get PMT:%d success\n", __FUNCTION__, pmt_pid );
        last_section_num = multifeed_pmt_buff[7];

        program.pcr_pid = 0x1FFF;
        program.video_pid = 0x1FFF;
        program.audio_pid[0] = 0x1FFF;

        ret = psi_pmt_parser(multifeed_pmt_buff, &program, PSI_MODULE_COMPONENT_NUMBER);
        if( SI_SUCCESS != ret )
        {
            #if __DBG_MF_
            mf_debug(" get pids fail\n");
            // for debug begin
            len = ((multifeed_pmt_buff[1]<<8)|multifeed_pmt_buff[2])&0x0FFF;

            for(i=0; i<len+3; i++)
            {
                mf_debug("%x ", multifeed_pmt_buff[i] );
            }
            mf_debug("\n");
            #endif
            // for debug end
            cnt++;
            if(cnt < max_cnt)
            {
                mf_debug(" going to retreive PMT and parser again\n" );
                continue;
            }
        }
        else
        {
            psi_pg2db( &new_node, &program );

            if(NULL == p_node)
            {
                return !MULTIFEED_SUCCESS;
            }

            p_node->pmt_pid      = pmt_pid;
            p_node->av_flag      = new_node.av_flag;
            p_node->ca_mode      = new_node.ca_mode;
            p_node->video_pid    = new_node.video_pid;
            p_node->pcr_pid      = new_node.pcr_pid;
            p_node->pmt_version  = new_node.pmt_version;
            p_node->h264_flag    = new_node.h264_flag;
            p_node->subtitle_pid = new_node.subtitle_pid;
            p_node->teletext_pid = new_node.teletext_pid;
            p_node->cur_audio    = new_node.cur_audio;
            p_node->audio_count  = new_node.audio_count;
            MEMCPY(p_node->audio_pid, new_node.audio_pid, sizeof(UINT16)*MAX_AUDIO_CNT);
            MEMCPY(p_node->audio_lang, new_node.audio_lang, sizeof(UINT16)*MAX_AUDIO_CNT);
            mf_debug("Parse PMT success\n\tv:%d, a:%d, pcr:%d, pmt:%d, sub:%d, ttx:%d\n",
                        p_node->video_pid, p_node->audio_pid[0], p_node->pcr_pid,
                        p_node->pmt_pid, p_node->subtitle_pid, p_node->teletext_pid);
            ret = MULTIFEED_SUCCESS;
        }

        if(MULTIFEED_SUCCESS == ret)
        {
            break;
        }
        section_num++;

    }while(section_num<=last_section_num);
    return ret;
}

static INT32 mf_parse_tp_info(UINT8 *data, INT32 data_len, T_NODE *t_node)
{
    INT32 i = 0;
    struct si_descriptor *desc = NULL;
    UINT32 polar = 0;
    INT32 ret = MULTIFEED_SUCCESS;

    if((NULL == data) || (NULL == t_node))
    {
        return !MULTIFEED_SUCCESS;
    }

    for(i=0; i<data_len; i+=sizeof(struct si_descriptor)+desc->len)
    {
        desc = (struct si_descriptor *)(data+i);

        if (SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR == desc->tag)
        {
            struct satellite_delivery_system_descriptor *sat = NULL;
            sat = (struct satellite_delivery_system_descriptor *)desc;

            polar = sat->polarization;
            t_node->pol = polar<SAT_POL_CIRCLE_LEFT?polar: polar-2;
            t_node->frq = bcd2integer(sat->frequency, 8, 0)/100;
            t_node->sym = bcd2integer(sat->symbol_rate, 7, 0)/10;
            t_node->fec_inner = sat->fec_inner;
            mf_debug("%s, frq = %d, sym = %d\n",__FUNCTION__,t_node->frq, t_node->sym);
            break;
        }
    }
    return ret;
}

static INT32 mf_search_node(UINT8 *buff, INT32 buff_len, UINT16 network_id, UINT16 ts_id, T_NODE *t_node)
{
    INT32 ret = MULTIFEED_FAILURE;
    INT32 desc_len = 0;
    INT32 loop_len = 0;
    UINT16 tmp_id = 0;
    UINT16 tmp_netid = 0;

    if((NULL == buff) || (NULL == t_node))
    {
        return ret;
    }

    desc_len = ((buff[8]&0x0F)<<8)|buff[9];
    buff += (10+desc_len);
    loop_len = ((buff[0]&0x0F)<<8)|buff[1];
    buff += 2;
    mf_debug("%s\n", __FUNCTION__);
    while(loop_len > 0)
    {
        desc_len = ((buff[4]&0x0F)<<8)|buff[5];
        tmp_id = ((buff[0]<<8)|buff[1]);
        tmp_netid = ((buff[2]<<8)|buff[3]);
        if((tmp_id ==ts_id) && (tmp_netid == network_id))
        {
            MEMSET(t_node,0,sizeof(T_NODE));
            t_node->network_id = network_id;
            t_node->t_s_id = ts_id;
            ret = mf_parse_tp_info(buff+6, desc_len, t_node);
            break;
        }
        buff += (6+desc_len);
        loop_len -= (6+desc_len);
    }

    return ret;
}

/* request the NIT from stream and find given tp's information */
static INT32 mf_find_tp_in_nit(UINT8 dmx_id, UINT16 network_id, UINT16 ts_id, T_NODE *t_node)
{
    INT32 ret = 0;
    UINT8 section_num = 0;
    UINT8 last_section_num = 0;
    struct dmx_device *dmx_dev = NULL;

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    if(NULL == dmx_dev)
    {
        return MULTIFEED_FAILURE;
    }

    struct get_section_param sr_request;
    struct restrict sr_restrict;

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));

    sr_request.buff = multifeed_table_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = PSI_NIT_PID;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 6000;

    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_NIT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;

    mf_debug("%s, going to request NIT\n", __FUNCTION__);
    section_num = 0;

    do
    {
        sr_restrict.value[0][6] = section_num;

        if(dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
        {
            mf_debug("%s, request NIT fail\n", __FUNCTION__);
            return MULTIFEED_FAILURE;
        }

        last_section_num = multifeed_table_buff[7];

        mf_debug("%s, got 1 NIT section\n", __FUNCTION__);
        ret = mf_search_node(multifeed_table_buff, 1024, network_id, ts_id, t_node);

        if(ret == MULTIFEED_SUCCESS)
        {
            break;
        }
        section_num++;

    }while(section_num<=last_section_num);

    return ret;
}

static INT32 find_tp_in_db(UINT16 sat_id, UINT16 network_id, UINT16 ts_id, T_NODE *t_node)
{
    INT32 i = 0;
    INT32 tp_num = 0;
    INT32 ret = -1;

    tp_num = get_tp_num_sat(sat_id);

    if(NULL != t_node)
    {
        for( i = 0; i < tp_num; i++ )
        {
          ret = get_tp_at(sat_id, i, t_node);

          if(DB_SUCCES == ret)
          {
	          if( (ts_id == t_node->t_s_id) && (network_id == t_node->network_id))
	          {
	            mf_debug("Found in DB: sat id %d, net id %d, tsid %d, tpid %d\n",sat_id, network_id, ts_id, t_node->tp_id);
	            return MULTIFEED_SUCCESS;
	          }
          }
        }
    }
    mf_debug("Can not find in DB: sat id %d, net id %d, tsid %d\n", sat_id, network_id, ts_id);
    return MULTIFEED_FAILURE;
}

/* find multifeed service node by @prog_id */
static struct MULTIFEED_INFO *multifeed_service_find(UINT32 prog_id)
{
    struct MULTIFEED_INFO *p = multifeed_info_head;

    while (p != NULL)
    {
        if (p->prog_id == prog_id)
        {
            return p;
        }
        p = p->next;
    }

    return NULL;
}

static struct MULTIFEED_INFO *multifeed_service_create(UINT8 dmx_id, UINT32 tp_id, UINT16 service_id)
{
    P_NODE p_node;
    INT32 i = 0;
    struct MULTIFEED_INFO *p = NULL;

    p = (struct MULTIFEED_INFO *)MALLOC(sizeof(struct MULTIFEED_INFO));

    if (NULL != p)
    {
        MEMSET(p, 0, sizeof(struct MULTIFEED_INFO));
        while(SUCCESS == get_prog_at(i, &p_node))
        {
            if((p_node.tp_id==tp_id) && (p_node.prog_number==service_id))
            {
                p->prog_id = p_node.prog_id;
                p->sat_id = p_node.sat_id;
                break;
            }
            i++;
        }
        p->tp_id = tp_id;
        p->service_id = service_id;
        p->dmx_id = dmx_id;
    }
    mf_debug("%s(): \n \t %s, v:%d, a:%d, pmt:%d, tpid %d\n", __FUNCTION__,
                p_node.service_name,p_node.video_pid, p_node.audio_pid[0],
                p_node.pmt_pid, p_node.tp_id);
    return p;
}

static void multifeed_service_add(struct MULTIFEED_INFO *p)
{
    if (NULL != p)
    {
        p->next = multifeed_info_head;
        multifeed_info_head  = p;
        mf_debug("%s (0x%X, tp_id: %d, sid: %d)\n", __FUNCTION__, p, p->tp_id, p->service_id);
    }
}

static INT32 fill_feed_detail_info(struct FEED_INFO *info, UINT32 sat_id, UINT32 prog_id, UINT8 dmx_id)
{
    INT32 ret = MULTIFEED_FAILURE;
    INT32 ret2 = MULTIFEED_FAILURE;
    INT32 ret3 = -1;
    T_NODE t_node;
    P_NODE p_node;
    P_NODE p_node2;
    P_NODE *feed_pnode = NULL;
    DB_VIEW *last_view = NULL;
    UINT16 last_filter_mode = 0;
    UINT16 i = 0;
    UINT32 last_view_param = 0;

    if(NULL == info)
    {
        return MULTIFEED_FAILURE;
    }

    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&p_node2, 0, sizeof(P_NODE));
    ret3 = get_prog_by_id(prog_id, &p_node);

    if(ret3 != DB_SUCCES)
    {
        return MULTIFEED_FAILURE;
    }
    
    ret = find_tp_in_db(sat_id, info->original_network_id, info->transport_stream_id, &t_node);

    if(MULTIFEED_SUCCESS != ret)
    {
        mf_debug("%s, need NIT\n", __FUNCTION__);
        // if already got the nit, then search the buffer, and update to db
        ret = mf_search_node(multifeed_table_buff, 1024, info->original_network_id, info->transport_stream_id, &t_node);

        if(MULTIFEED_SUCCESS != ret)
        {
            // not recieve nit data yet, so request nit data
            ret = mf_find_tp_in_nit(dmx_id,info->original_network_id, info->transport_stream_id, &t_node);
        }

        if(MULTIFEED_SUCCESS == ret)
        {
            last_view           =   (DB_VIEW *)get_cur_view();
            last_filter_mode    =   last_view->cur_filter_mode;
            last_view_param     =   last_view->view_param;
            if(TYPE_PROG_NODE == last_view->cur_type)
            {
                recreate_tp_view(VIEW_ALL ,0);
                ret = add_node(TYPE_TP_NODE, sat_id, (void*)&t_node);
                if(SUCCESS == ret)
                {
                   update_data();
                }

                recreate_prog_view(last_filter_mode, last_view_param);
            }
            else
            {
                ret = add_node(TYPE_TP_NODE, sat_id, (void*)&t_node);
                if(SUCCESS == ret)
                {
                    update_data();
                }
            }
        }
    }

    // maybe the subfeed is one of the channels, and that channel
    // maybe locked by parent, so we have to get this information
    if( p_node.prog_number != info->service_id )
    {
        i = 0;
        while(SUCCESS == get_prog_at(i, &p_node2))
        {
            if((p_node2.tp_id == t_node.tp_id) && (p_node2.prog_number == info->service_id))
            {
                mf_debug("feed: (%s, sid: %d) is Channel: %s\n", info->name, info->service_id, p_node2.service_name);
                ret2 = MULTIFEED_SUCCESS;
                break;
            }
            i++;
        }
    }
    
    if(SUCCESS == ret)
    {
        info->is_known  =   1;
        feed_pnode      =   &(info->p_node);

        MEMCPY(feed_pnode, &p_node, sizeof(P_NODE));
        feed_pnode->tp_id           =   t_node.tp_id;
        feed_pnode->prog_number     =   info->service_id;
        if( ret2 != MULTIFEED_SUCCESS)
        {
            feed_pnode->pmt_pid      = 0x1FFF;
            feed_pnode->audio_count  = 1;
            feed_pnode->audio_pid[0] = 0x1FFF;
            feed_pnode->pcr_pid      = 0x1FFF;
            feed_pnode->video_pid    = 0x1FFF;
        }
        else
        {
            feed_pnode->av_flag       =   p_node2.av_flag;
            feed_pnode->ca_mode       =   p_node2.ca_mode;
            feed_pnode->video_pid     =   p_node2.video_pid;
            feed_pnode->pcr_pid       =   p_node2.pcr_pid;
            feed_pnode->h264_flag     =   p_node2.h264_flag;
            feed_pnode->pmt_pid       =   p_node2.pmt_pid;
            feed_pnode->pmt_version   =   p_node2.pmt_version;
            feed_pnode->service_type  =   p_node2.service_type;
            feed_pnode->audio_channel =  p_node2.audio_channel;
            feed_pnode->audio_select  =   p_node2.audio_select;
            feed_pnode->lock_flag     =   p_node2.lock_flag;// parent lock
            feed_pnode->audio_count   =   p_node2.audio_count;
            MEMCPY( feed_pnode->audio_pid, p_node2.audio_pid, sizeof(UINT16)*MAX_AUDIO_CNT );
            MEMCPY( feed_pnode->audio_lang, p_node2.audio_lang, sizeof(UINT16)*MAX_AUDIO_CNT );
        }
        mf_debug("fill feed: tp_id: %d, frq: %d, sym = %d\n", feed_pnode->tp_id, t_node.frq, t_node.sym);

        return MULTIFEED_SUCCESS;
    }

    return MULTIFEED_FAILURE;
}


/* add an empty feed node to multifeed service node @info (add to tail) */
static struct FEED_INFO *add_feed_to_service( struct MULTIFEED_INFO *info, struct FEED_INFO *feed )
{
    if((feed != NULL) && (info != NULL))
    {
        if(NULL == info->feed_head)
        {
            info->feed_head = feed;
            info->feed_tail = feed;
        }
        else
        {
            info->feed_tail->next = feed;
            info->feed_tail = feed;
        }
        info->num++;
        return feed;
    }
    else
    {
        return NULL;
    }
}

static struct FEED_INFO* del_feed_node( struct MULTIFEED_INFO *info, struct FEED_INFO *d )
{
    struct FEED_INFO *p = NULL;
    struct FEED_INFO *q = NULL;

    if((NULL == info) || (NULL == d))
    {
        return NULL;
    }

    q = info->feed_head;
    p = info->feed_head;

    while( (p != NULL) && (p != d) )
    {
        q = p;
        p = p->next;
    }

    if(NULL == p)
    {
        return info->feed_head;
    }

    if( p == info->feed_head )
    {
        info->feed_head = p->next;
        if(NULL == info->feed_head)
        {
            info->feed_tail = NULL;
        }
        q = p->next;
        FREE( p );
        p = NULL;
    }
    else if( p == info->feed_tail )
    {
        q->next = NULL;
        info->feed_tail = q;
        q = p->next;
        FREE( p );
        p = NULL;
    }
    else
    {
        q->next = p->next;
        q = p->next;
        FREE( p );
        p = NULL;
    }
    info->num--;
    return q;
}

static void del_all_feed_node(struct MULTIFEED_INFO *info)
{
    struct FEED_INFO *p = NULL;
    struct FEED_INFO *q = NULL;

    if(NULL != info)
    {
        p = info->feed_head;
        while (p != NULL)
        {
            q = p->next;
            FREE(p);
            p = q;
        }
        info->feed_head = info->feed_tail = NULL;
    }
}

static int multifeed_service_cmp(struct MULTIFEED_INFO *p, struct MULTIFEED_INFO *q)
{
    struct FEED_INFO *f1 = NULL;
    struct FEED_INFO *f2 = NULL;

    if ((NULL == p) || (NULL == q))
    {
        return -1;
    }

    if ((p->prog_id != q->prog_id) || (p->tp_id != q->tp_id) ||
        (p->sat_id != q->sat_id) || (p->service_id != q->service_id) ||
        (p->num != q->num))
    {
        return 1;
    }

    f1 = p->feed_head;
    f2 = q->feed_head;

    while (f1 && f2)
    {
        if ((MEMCMP(&f1->transport_stream_id, &f2->transport_stream_id, FEED_INFO_CMP_SIZE)) != 0)
        {
            break;
        }
        f1 = f1->next;
        f2 = f2->next;
    }

    if ((NULL != f1) || (NULL != f2))
    {
        return 1;
    }

    return 0;
}

/* remove multifeed service node @d from list multifeed_info_head */
static int multifeed_service_remove(struct MULTIFEED_INFO *d)
{
    struct MULTIFEED_INFO *p = NULL;
    struct MULTIFEED_INFO *c = multifeed_info_head;
    struct MULTIFEED_INFO *n = NULL;

    if(NULL == d)
    {
        return MULTIFEED_FAILURE;
    }

    while( c != NULL )
    {
        n = c->next;
        if( c == d )
        {
            if(NULL == p)
            {
                multifeed_info_head = n;
            }
            else
            {
                p->next = n;
            }

            d->next = NULL;
            return MULTIFEED_SUCCESS;
        }
        p = c;
        c = n;
    }
    return MULTIFEED_FAILURE;
}

/* free all nodes of given multifeed service, include service node itself */
static void multifeed_service_destroy(struct MULTIFEED_INFO *p)
{
    if (p)
    {
        mf_debug("%s(0x%X, 0x%X, %d)\n", __FUNCTION__, p, p->tp_id, p->service_id);
        del_all_feed_node(p);
        FREE(p);
        p = NULL;
    }
}

/* get feed node from multifeed service node @info by @idx*/
static struct FEED_INFO *get_feed_node(struct MULTIFEED_INFO *info, UINT32 idx)
{
    UINT32 i = 0;
    struct FEED_INFO *p = NULL;

    if(NULL != info)
    {
        p = info->feed_head;

        while (p != NULL)
        {
            if (i == idx)
            {
                return p;
            }

            p = p->next;
            i++;
        }
    }

    return NULL;
}

static INT32 find_feed_node(struct MULTIFEED_INFO *info, struct FEED_INFO *feed_node )
{
    INT32 ret = -1;
    UINT8 i = 0;
    struct FEED_INFO *feed = NULL;

    if((NULL == feed_node) || (NULL == info))
    {
        return ret;
    }

    for( i=0; i<info->num; i++ )
    {
        feed = get_feed_node( info, i );

        if( NULL == feed )
        {
            return ret;
        }

        if( (feed_node->transport_stream_id == feed->transport_stream_id)
            && (feed_node->original_network_id == feed->original_network_id)
            && (feed_node->service_id == feed->service_id)
            && (feed_node->is_known == feed->is_known)
            && (MEMCMP( feed_node->name, feed->name, sizeof(feed->name) )==0)
            && (MEMCMP( &feed_node->p_node, &feed->p_node, sizeof(feed->p_node) )==0) )
        {
            ret = i;
            break;
        }
    }
    return ret;
}

static INT32 verify_feed_info( struct FEED_INFO *feed_info )
{
    P_NODE *p_node = NULL;

    if(NULL != feed_info)
    {
        p_node = &( feed_info->p_node );

        if((p_node->video_pid != 0x1FFF) && (p_node->audio_pid[0]!= 0x1FFF) && (p_node->pcr_pid != 0x1FFF))
        {
            return MULTIFEED_SUCCESS;
        }
    }
    return MULTIFEED_FAILURE;
}

/* proccess eit section data, only handle eit pf section extract linkage descriptors in event loop*/
void eit_multifeed_handle(UINT32 tp_id, UINT8 *buf, INT32 length, UINT8 dmx_id)
{
    INT32 ret = ERR_FAILED;
    INT32 desc_loop_len = 0;
    UINT8 tag = 0;
    UINT8 len = 0;
    struct FEED_INFO *feed_node = NULL;
    struct MULTIFEED_INFO *mf_new = NULL;
    struct MULTIFEED_INFO *mf_old = NULL;
    UINT16 service_id = 0;

    if(NULL == buf)
    {
        return ;
    }

    service_id = (buf[3]<<8) | buf[4];

    //only present event
    if((buf[0] != 0x4E) || (buf[6] != 0))
    {
        return;
    }

    buf += 14;
    length -= (14+4);
    mf_debug("\n%s, get new eit pf\n\n", __FUNCTION__);
    while(length > 0) // event loop
    {
        desc_loop_len = ((buf[10]&0x0F)<<8)|buf[11];
        buf += 12;
        length -= (12+desc_loop_len);

        while(desc_loop_len > 0) // desc loop
        {
            tag = buf[0];
            len = buf[1];
            if((0x4A == tag) && (0xB0 == buf[8])) // buf[8]:link_type
            {
                if(NULL == mf_new)
                {
                    mf_new = multifeed_service_create(dmx_id, tp_id, service_id );
                    if(NULL == mf_new)
                    {
                        break;
                    }
                }
                feed_node = (struct FEED_INFO *) MALLOC(sizeof(struct FEED_INFO));
                MEMSET( feed_node, 0, sizeof(struct FEED_INFO) );
                if( NULL == feed_node )
                {
                    break;
                }
                feed_node->transport_stream_id = (buf[2]<<8)|buf[3];
                feed_node->original_network_id = (buf[4]<<8)|buf[5];
                feed_node->service_id = (buf[6]<<8)|buf[7];
                MEMCPY( feed_node->name, &buf[9], len-7 );
                mf_debug("\nGot feed: %s, going fill info:\n", feed_node->name);
                ret = fill_feed_detail_info( feed_node, mf_new->sat_id, mf_new->prog_id, dmx_id);
                if(ret == MULTIFEED_SUCCESS)
                {
	                mf_debug("%s, after fill feed info:\n \t %s, ts_id: %d, sid: %d\n",
	                            __FUNCTION__, feed_node->name, feed_node->transport_stream_id, feed_node->service_id);
	                if( NULL == add_feed_to_service( mf_new, feed_node ) )
	                {
	                    FREE( feed_node );
	                    feed_node = NULL;
	                }
                }
                else
                {
                    FREE( feed_node );
	                feed_node = NULL;
                }
            }
            desc_loop_len -= (len+2);
            buf += (len+2);
        }
    }

    if( NULL == mf_new )
    {
        return;
    }

    mf_old = multifeed_service_find( mf_new->prog_id );
    if( NULL == mf_old )
    {
        multifeed_service_add(mf_new);
        if ( NULL != multifeed_cb )
        {
            multifeed_cb(mf_new->prog_id); //notify update UI
        }
        return;
    }

    // if content is the same, no need to update
    if( 0 == multifeed_service_cmp( mf_new, mf_old ) )
    {
        mf_debug(" mf_new equal with mf_old\n");
        multifeed_service_destroy(mf_new);
        return;
    }
    // set selected feed
    if( mf_old->idx !=0 )
    {
        INT32 idx = 0;
        struct FEED_INFO *fi_old = NULL;
        fi_old = get_feed_node( mf_old, mf_old->idx );
        idx = find_feed_node( mf_new,  fi_old );
        if( idx >= 0 ) // idx>=0 means we have found the selected feed in new list
        {
            mf_new->idx = idx;
        }
    }
    if( MULTIFEED_SUCCESS == multifeed_service_remove(mf_old) )
    {
        multifeed_service_destroy(mf_old);
    }
    multifeed_service_add(mf_new);
    mf_debug("old list have been deleted, new list have been added\n");
    if ( multifeed_cb != NULL )
    {
        multifeed_cb(mf_old->prog_id); //return to the origin service
    }
}

static struct MULTIFEED_INFO *del_invalid_info(struct MULTIFEED_INFO *info)
{
    INT32 ret = 0;
    P_NODE p_node;
    T_NODE t_node;
    UINT32 tp_id = 0;
    UINT16 service_id = 0;
    struct FEED_INFO *p = NULL;
    INT8 pf = 0;
  
    if( NULL == info )
    {
        return NULL;
    }

    tp_id      = info->tp_id;
    service_id = info->service_id;
    ret = get_prog_by_id(info->prog_id, &p_node);

    if( ret != DB_SUCCES )
    {   // service not exist
        multifeed_service_remove( info );
        multifeed_service_destroy( info );
        // notify epg module retrieve eit pf table
        retrieve_eit_pf( tp_id, service_id );
        return NULL;
    }

    p = info->feed_head;
    while( p != NULL )
    { // check all feeds: valid or not
        ret = find_tp_in_db( info->sat_id, p->original_network_id, p->transport_stream_id, &t_node );
        if( ret != MULTIFEED_SUCCESS )
        { // del invalid feed
            p = del_feed_node( info, p );
            pf = 1;
        }
        else
        {
            p = p->next;
        }
    }
    if( pf ) // notify epg module retrieve eit pf table
    {
        retrieve_eit_pf( tp_id, service_id );
    }

    return info;
}

//================= lib_multifeed API ==============================//
void multifeed_register(MULTIFEED_CALL_BACK cb)
{
    multifeed_cb = cb;
}

INT32 multifeed_get_feed_name(UINT32 prog_id, UINT32 idx, UINT8 *name)
{
    UINT32 i = 0;
    struct FEED_INFO *p = NULL;
    struct MULTIFEED_INFO *info = NULL;

    info = multifeed_service_find(prog_id);
    if((NULL == info) || (NULL == name))
    {
        return MULTIFEED_FAILURE;
    }

    p = info->feed_head;

    while(p != NULL)
    {
        if(i == idx)
        {
            MEMCPY(name, p->name, 32);
            return MULTIFEED_SUCCESS;
        }
        p = p->next;
        i++;
    }

    return MULTIFEED_FAILURE;
}

INT32 multifeed_get_info(UINT32 prog_id, struct MULTIFEED_INFO *info)
{
    if( NULL == info )
    {
        return MULTIFEED_FAILURE;
    }

    struct MULTIFEED_INFO *multifeed_cur = multifeed_service_find( prog_id );

    if(multifeed_cur != NULL)
    {
        MEMCPY(info, multifeed_cur, sizeof(struct MULTIFEED_INFO));
        return MULTIFEED_SUCCESS;
    }
    else
    {
        return MULTIFEED_FAILURE;
    }
}

BOOL multifeed_have_feed(UINT32 prog_id)
{
    struct MULTIFEED_INFO *multifeed_cur = multifeed_service_find(prog_id);
    if(multifeed_cur != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

INT32 multifeed_modify_node(UINT32 prog_id, P_NODE *p_node, BOOL rel)
{
    struct FEED_INFO *feed_node = NULL;
    struct MULTIFEED_INFO *p = multifeed_info_head;

    while(p != NULL)
    {
        if((p->prog_id == prog_id))
        {
            if( p->idx >= p->num )
            {
                p->idx = 0; // some feed was deleted, play master feed instead
            }
            if(p->idx !=0)
            {
                feed_node = get_feed_node(p, p->idx);
            }
            break;
        }
        p = p->next;
    }

    if((feed_node != NULL) && (p_node != NULL))
    {
        if(rel)
        {
            MEMCPY(p_node, &feed_node->p_node, sizeof(P_NODE));
        }
        else
        {
            MEMCPY(&feed_node->p_node, p_node, sizeof(P_NODE));
        }
        return MULTIFEED_SUCCESS;
    }
    else
    {
        return MULTIFEED_FAILURE;
    }
}
static INT32 multifeed_tune_to_tp(DB_TP_ID tp_id)
{
    T_NODE t_tuned_node = {0};
    S_NODE s_tuned_node = {0};
    struct nim_device *nim = NULL;
    struct ft_antenna antenna;
    union ft_xpond xpond_info;
    UINT8 max_lock = 6;
    UINT8 lock_cnt = 0;
    UINT8 signal_lock = 0;
    INT32 ret = -1;

	MEMSET(&antenna, 0, sizeof(struct ft_antenna));
	MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
    if (get_tp_by_id(tp_id, &t_tuned_node) != SUCCESS)
    {
        return !SUCCESS;
    }
    // Change frontend
    ret = get_sat_by_id(t_tuned_node.sat_id, &s_tuned_node);

    if(DB_SUCCES != ret)
    {
	    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM,
	        (s_tuned_node.tuner1_valid ? 0 : (s_tuned_node.tuner2_valid ? 1 : 0)));
    }
    else
    {
       return !SUCCESS;
    }

    UINT32 nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
    if(FRONTEND_TYPE_S == nim_sub_type)
    {
        sat2antenna(&s_tuned_node, &antenna);
        xpond_info.s_info.type = FRONTEND_TYPE_S;
        xpond_info.s_info.frq = t_tuned_node.frq;
        xpond_info.s_info.sym = t_tuned_node.sym;
        xpond_info.s_info.pol = t_tuned_node.pol;
        xpond_info.s_info.tp_id = t_tuned_node.tp_id;
        xpond_info.s_info.fec_inner = t_tuned_node.fec_inner;
    }
    else
    {
        // only satelite has multifeed
        return !SUCCESS;
    }
    mf_debug("%s, frq = %d, sym = %d\n", __FUNCTION__, xpond_info.s_info.frq, xpond_info.s_info.sym);
    frontend_tuning(nim, &antenna, &xpond_info, 1);
    // check signal lock or not

    while (lock_cnt < max_lock)
    {
        nim_get_lock(nim, &signal_lock);
        if (!signal_lock)
        {
            osal_task_sleep(500);
        }
        else
        {
            break;
        }
        lock_cnt++;
    }
    if (!signal_lock)
    {
        return !SUCCESS;
    }

    return SUCCESS;
}

INT32 multifeed_change_feed(UINT16 channel, UINT32 idx)
{
    INT32 ret = 0;
    T_NODE t_node;
    P_NODE p_node;
    UINT16 pmt_pid = 0x1FFF;
    struct MULTIFEED_INFO *multifeed_cur = NULL;
    struct FEED_INFO *feed_info = NULL;

	MEMSET(&p_node, 0, sizeof(P_NODE));
	MEMSET(&t_node, 0, sizeof(T_NODE));
    ret = get_prog_at(channel, &p_node);

    if(ret != DB_SUCCES)
    {
        return MULTIFEED_FAILURE;
    }

    multifeed_cur = multifeed_service_find(p_node.prog_id);
    if(NULL == multifeed_cur)
    {
        return MULTIFEED_FAILURE;
    }

    if(idx >= multifeed_cur->num)
    {
        return MULTIFEED_FAILURE;
    }
    if(idx == multifeed_cur->idx)
    {
        return MULTIFEED_FAILURE;
    }

    undo_prog_modify(p_node.tp_id, p_node.prog_number);

    if(0 == idx)
    {
        multifeed_cur->idx = idx;
        return MULTIFEED_SUCCESS;
    }
    mf_debug("%s(ch=%d, fd=%d)\n", __FUNCTION__, channel, idx );
    feed_info = get_feed_node(multifeed_cur, idx);
    mf_debug("Changing to feed: %s\n", feed_info->name);
    if(feed_info->is_known)
    {
        multifeed_cur->idx = idx;
        // check feed info valid or not
        if( verify_feed_info( feed_info ) != MULTIFEED_SUCCESS )
        {
            multifeed_tune_to_tp(feed_info->p_node.tp_id);
            // get pat info
            ret = search_pat_buff( multifeed_pat_buff, 1024, feed_info->service_id, &pmt_pid );
            if( ret != MULTIFEED_SUCCESS )
            {
                ret = mf_find_info_in_pat(multifeed_cur->dmx_id, feed_info->transport_stream_id,
                    feed_info->service_id, &pmt_pid);
            }
            if( MULTIFEED_SUCCESS == ret )
            {
                // get pmt info
                MEMCPY( &p_node, &feed_info->p_node, sizeof(P_NODE) );  // reuse p_node
                mf_debug("Going to get pids of feed: %s\n", feed_info->name );
                ret = get_pid_in_pmt(multifeed_cur->dmx_id, pmt_pid, &p_node );
                if( MULTIFEED_SUCCESS == ret )
                {
                    MEMCPY( &feed_info->p_node, &p_node, sizeof(P_NODE) );
                }
            }
        }
        return MULTIFEED_SUCCESS;
    }
    return MULTIFEED_FAILURE;
}

void multifeed_del_invalid_info()
{
    struct MULTIFEED_INFO *q = NULL;
    struct MULTIFEED_INFO *p = multifeed_info_head;

    while( p != NULL )
    {
        q = p->next;
        p = del_invalid_info( p );
        p = q;
    }
}

void multifeed_set_parentlock( UINT16 sat_id, UINT32 tp_id, UINT32 prog_number, UINT16 lock_flag )
{
    struct MULTIFEED_INFO *p = multifeed_info_head;
    struct FEED_INFO *feed = NULL;
    INT32 i = 0;

    mf_debug("%s, check feed need lock/unlock or not\n", __FUNCTION__ );
    while (p != NULL)
    {
        for( i=1; i<p->num; i++ )
        {
            feed = get_feed_node( p, i );
            if(     (feed != NULL)
                &&  (feed->p_node.sat_id == sat_id)
                &&  (feed->p_node.tp_id  == tp_id)
                &&  (feed->p_node.prog_number == prog_number) )
            {
                feed->p_node.lock_flag  =   lock_flag;
                mf_debug("\tfeed: %s, lock: %d\n", feed->name, lock_flag );
            }
        }
        p = p->next;
    }
}
#endif

