/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt.c
   *
   *    Description: The file is mainly to get the subtitle PID from PMT and
        register the PID to DMX
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/
#include <osal/osal.h>
#include <basic_types.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsubt/subt_osd.h>
#include <hld/sdec/sdec.h>

#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/si_monitor.h>
#endif
#include <api/libsi/psi_pmt.h>
#include "lib_subt_internal.h"
#include "lib_atsc_subt.h"


#define SUBT_MAX_DESC_SIZE 256

#ifndef SEE_CPU

#ifdef LIB_TSI3_FULL

static struct desc_table subt_desc_info[] =
{
    {
        SUBTITLE_DESCRIPTOR,
        0,
        (desc_parser_t)subt_descriptor,
    },
};
#endif


static UINT32 g_subt_desc_get = 0;
static UINT8 default_subt_lang= 0xFF;
static UINT8 subt_dmx_used[3] = {0,0,0};
static UINT8 subt_cur_monitor_id = 0xff;
static BOOL g_is_subt_available = FALSE;
static struct t_subt_lang g_subt_lang[SUBTITLE_LANG_NUM];
static UINT8 g_subt_lang_num =0;
static UINT8 g_current_lang = 0xff;
static UINT16 g_current_pid =0x1fff;
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static UINT8 rec_monit_id_subt_lg_num[2][2];
static struct t_subt_lang rec_monit_id_subt_lg[2][2][SUBTITLE_LANG_NUM];
#endif
static BOOL g_subt_enable = FALSE;
static BOOL g_dcii_subt_enable = FALSE;
static BOOL g_pvrsubt_enable = FALSE;
static UINT16 dmx_index = 0;
static SUBT_EVENT_CALLBACK subt_event_callback = NULL;
#ifdef ATSC_SUBTITLE_SUPPORT
static SUBT_TYPE g_subt_type[SUBTITLE_LANG_NUM];
static SUBT_TYPE g_current_subt_type = DVB_SUBT;
static BOOL g_is_atsc_subt =FALSE;
#endif

#ifdef LIB_TSI3_FULL

static UINT8 prev_subt[256] ={0};

#ifdef ATSC_SUBTITLE_SUPPORT

static INT32 iso639_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 k = 0;
    int index = 0;
    struct pmt_es_info *es = NULL;
	if((0>=len)||(0xff<=len))
	{
        return SI_FAILED;
    }
    if(NULL == priv)
    {
        return SI_FAILED;
    }

    es = (struct pmt_es_info *)priv;
    for( i = 0; i < len; i += 4)
    {
        if((k+ADD_THREE) < DESC_LENGTH_64)
        {
            for (j=i; j<i+3; j++)
            {
                if (((*(desc+j)>='a')&&(*(desc+j)<= 'z'))||((*(desc+j) >= 'A')&&(*(desc+j)<='Z')))
                {
                    es->lang[index++] = *(desc+j);
                }
                else
                {
                    es->lang[index++] = 'a';
                }
            }
            k += 3;
        }
        else
        {
            break;
        }
    }
    return SI_SUCCESS;
}


static struct desc_table atsc_subt_desc_info[] =
{
    {
        ISO_639_LANGUAGE_DESCRIPTOR,
        0,
        (desc_parser_t)iso639_desc_handle,
    },
};

static void subt_one_lang(UINT32 pid, UINT8 *lang)     // Register new language
{
    UINT32 i = 0;

    if(NULL == lang)
    {
        return;
    }
    g_is_atsc_subt = TRUE;
    for(i = 0; i<g_subt_lang_num;++i)
    {
        // Current language was included in subtitle aready registed.
        if(g_subt_lang[i].pid == pid)
        {
            return;
        }
    }

    if(g_subt_lang_num +1 <= SUBTITLE_LANG_NUM)
    {
        // Different language corresponded with different pid
        g_subt_lang[g_subt_lang_num].pid = pid;
        g_subt_type[g_subt_lang_num] = ATSC_SUBT;
        for(i = 0 ; i < 3 ; ++i)
        {
            g_subt_lang[g_subt_lang_num].lang[i]= lang[i];
        }
        g_subt_lang_num ++;
    }
}

static INT32 atsc_subt_callback(UINT8 *buff, INT32 len, UINT32 param)
{
	
	INT32 es_info_length=0;
	INT32 i=0;
//	UINT16 es_pid=0;
	struct pmt_es_info es;
	UINT8* buf0=NULL;
	struct pmt_stream_info *stream=NULL;
//	struct pmt_section *pms=NULL;
	
	MEMSET(&es, 0, sizeof(struct pmt_es_info));


	if(NULL == buff)
    {
        return ERR_FAILUE;
    }
	buf0 = buff+12+(((buff[10]<<8)|buff[11])&0x0FFF);
//	pms = (struct pmt_section *)buf0;
	if(len<4)
	{
        return ERR_FAILUE;
    }
	for(i=0; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info)) 
	{   
		
		stream = (struct pmt_stream_info *)(buf0+i);
		MEMSET(&es, 0, sizeof(struct pmt_es_info));
		es.stream_type = stream->stream_type;
		if(es.stream_type == ATSC_SUBT_STREAM)
		{
			es.pid = SI_MERGE_HL8(stream->elementary_pid);
			desc_loop_parser(stream->descriptor, es_info_length, atsc_subt_desc_info, 1, NULL, (void *)&es);
			subt_one_lang(es.pid,es.lang);
		}
		es_info_length = SI_MERGE_HL8(stream->es_info_length);
	}
    return SUCCESS;
}

#endif
#ifdef DUAL_ENABLE
//INT32 atsc_sdec_m3327_request_write(void* pdev,UINT32 uSizeRequested,UINT8** ppuData,UINT32* puSizeGot)
//{
 
//}
//void atsc_sdec_m3327_update_write(void* pdev,UINT32 uSize)
//{
	
//}

static UINT8 atsc_subt_dmx_used[3] = {0,0,0};
static RET_CODE atsc_subt_unreg_all_dmx()
{
	static struct dmx_device *dmx_dev;
	RET_CODE reg_code = RET_SUCCESS;
	UINT8 i = 0;
	for(i=0; i<3; i++)
	{
		if(atsc_subt_dmx_used[i])
		{
			dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i); 
			reg_code = dmx_unregister_service(dmx_dev,6);
			if(reg_code != RET_SUCCESS)
			{
				LIBSUBT_PRINTF("%s : Error: subt fail to stop dmx[%d]!\n",__FUNCTION__,i);
				return !RET_SUCCESS;
			}
			else
			{
				LIBSUBT_PRINTF("%s : subt success to stop dmx[%d]!\n",__FUNCTION__,i);
			}
			atsc_subt_dmx_used[i]=0;
		}
	}

    return reg_code;
}
#endif

BOOL subt_is_available(void)
{
    return g_is_subt_available;
}

static INT32 subt_callback(UINT8 *buff, INT32 len, UINT32 param)
{
    INT32 prog_info_length = 0;
    INT32 es_info_length = 0;
    INT32 i = 0;
    UINT16 es_pid = 0;
//    RET_CODE ret_func = 0;
    struct pmt_stream_info *stream = NULL;
    struct pmt_section *pms = NULL;

    if(NULL == buff)
    {
       return ERR_FAILUE;
    }
    pms = (struct pmt_section *)buff;
    prog_info_length = SI_MERGE_HL8(pms->program_info_length);

    g_is_subt_available = FALSE;
	if((0>=len)||(len<=(INT32)(sizeof(struct pmt_section)+prog_info_length)))
	{
        return ERR_FAILUE;
    }
    for(i=sizeof(struct pmt_section)+prog_info_length-4;
        i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);
        desc_loop_parser(stream->descriptor, es_info_length,
                   subt_desc_info, 1, NULL, (void *)&es_pid);
        #ifdef ATSC_SUBTITLE_SUPPORT
        if (ATSC_SUBT_STREAM == stream->stream_type)
        {
            g_is_subt_available = TRUE;
        }
        #endif
    }
    return SUCCESS;
}

INT32 subt_unregister(UINT32 monitor_id)
{
    UINT16 stuff_pid = 0x1FFF;
    struct sim_cb_param param;
//    RET_CODE ret_func = 0;

    MEMSET(&param,0,sizeof(struct sim_cb_param));

    if( SUCCESS== sim_get_monitor_param(monitor_id, &param) )
    {
        if(
            #ifdef ATSC_SUBTITLE_SUPPORT
            (sim_unregister_scb(monitor_id, atsc_subt_callback)!= SUCCESS)||
            #endif
            (sim_unregister_scb(monitor_id, subt_callback)!= SUCCESS))
        {
            LIBSUBT_PRINTF("%s : Error: subt fail to unmount to monitor[%d]!\n",__FUNCTION__,monitor_id);
            return ERR_FAILUE;
        }
    }

    LIBSUBT_PRINTF("%s : subt success to unmount to monitor[%d]!\n",__FUNCTION__,monitor_id);
    subt_descriptor(SUBTITLE_DESCRIPTOR, 0, prev_subt, (void *)&stuff_pid);
    return SUCCESS;
}

#ifdef NEW_TIMER_REC
static UINT32 subt_living_monitor_id;
UINT32 g_subt_is_bg_record = FALSE;

void subt_set_is_bg_record(BOOL flag)
{
	g_subt_is_bg_record = flag;
}

static void subt_set_monitor_id(monitor_id)
{
	libc_printf("subt_living_monitor_id = %d\n",monitor_id);
	subt_living_monitor_id = monitor_id;
}

UINT32 subt_get_monitor_id(void)
{
	return subt_living_monitor_id;
}
#endif

INT32 subt_register(UINT32 monitor_id)
{
    struct sim_cb_param param;
//    RET_CODE ret_func = 0;

    MEMSET(&param,0,sizeof(struct sim_cb_param));

    if( sim_get_monitor_param(monitor_id, &param) != SUCCESS)
    {
        LIBSUBT_PRINTF("%s : Error1: subt fail to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
        return ERR_FAILUE;
    }
//modify for:we shuld get the dmx_index  before unregister one dmx  20111226
    if(
        #ifdef ATSC_SUBTITLE_SUPPORT
       (SUCCESS== sim_register_scb(monitor_id, atsc_subt_callback, NULL))&&
        #endif
       (SUCCESS== sim_register_scb(monitor_id, subt_callback, NULL)))
    {
        dmx_index = param.dmx_idx;
		if(3<=dmx_index)
			return ERR_FAILUE;
    }
//modify end
        subt_unregister(subt_cur_monitor_id);

    if(
        #ifdef ATSC_SUBTITLE_SUPPORT
        (SUCCESS==sim_register_scb(monitor_id, atsc_subt_callback, NULL))&&
        #endif
        (SUCCESS==sim_register_scb(monitor_id, subt_callback, NULL)))
    {
        dmx_index = param.dmx_idx;
		if(3<=dmx_index)
			return ERR_FAILUE;
	 #ifdef NEW_TIMER_REC
	 subt_set_monitor_id(subt_cur_monitor_id);
	 #endif
        subt_cur_monitor_id = monitor_id;
        LIBSUBT_PRINTF("%s : subt success to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
        return SUCCESS;
    }
    LIBSUBT_PRINTF("%s : Error2: subt fail to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
    return ERR_FAILUE;
}


#endif

static RET_CODE subt_unreg_all_dmx(void)
{
    /*static*/ struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;
    UINT8 i = 0;

    for(i=0; i<3; i++)
    {
        if(subt_dmx_used[i])
        {
            dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i);
            reg_code = dmx_unregister_service(dmx_dev,4);
            if(reg_code != RET_SUCCESS)
            {
                LIBSUBT_PRINTF("%s : Error: subt fail to stop dmx[%d]!\n",__FUNCTION__,i);
                return !RET_SUCCESS;
            }
            else
            {
                LIBSUBT_PRINTF("%s : subt success to stop dmx[%d]!\n",__FUNCTION__,i);
            }
            subt_dmx_used[i]=0;
        }
    }

    return RET_SUCCESS;
}

#if 0
static INT32 subt_restart(UINT16 pid)
{
    UINT8 lang_idx = 0;
    UINT8 lang=0;
    //UINT16 page_num = 0xffff;
    struct register_service subt_serv;
    /*static*/ struct dmx_device *dmx_dev = NULL;
    /*static*/ struct sdec_device *sdec_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;
    RET_CODE ret_func = 0;

    MEMSET(&subt_serv,0,sizeof(struct register_service));
    LIBSUBT_PRINTF("%s : subt[%d] PID[%d] changed!\n",
                  __FUNCTION__,g_subt_enable,pid);
    sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
    //dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
//    libc_printf("%s: g_subt_lang_num=%d, g_subt_enable=%d\n",
                 //__FUNCTION__, g_subt_lang_num, g_subt_enable);

    if( TRUE== g_subt_enable)
    {
        LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);
        ret_func =sdec_stop(sdec_dev);
    }
#ifdef ATSC_SUBTITLE_SUPPORT
    if(pid != g_current_pid && g_current_subt_type!= ATSC_SUBT)
#else
    if(pid != g_current_pid)
#endif
    {
        g_current_pid = pid;
        if( TRUE== g_subt_enable)
        {
            ret_func =subt_unreg_all_dmx();
            subt_serv.device = sdec_dev;
            subt_serv.request_write = (request_write)sdec_request_write;
            subt_serv.update_write = (update_write)sdec_update_write;
            subt_serv.service_pid = g_current_pid;
            reg_code = dmx_register_service(dmx_dev,4,&subt_serv);
        if( RET_SUCCESS==reg_code )
        {
            subt_dmx_used[dmx_index] = 1;
            LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
        }
        else
        {
            LIBSUBT_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
        }
        }
    }

    for(lang_idx = 0;lang_idx<g_subt_lang_num;lang_idx++)
    {
        if(g_subt_lang[lang_idx].pid == pid)
        {
            lang = lang_idx;
        }
    }

    if( TRUE== g_subt_enable)
    {
//      libc_printf("lang=%d\n",lang);
        LIBSUBT_PRINTF("%s-sdec_start()\n",__FUNCTION__);
        ret_func = sdec_start(sdec_dev,g_subt_lang[lang].com_page,g_subt_lang[lang].anci_page);
    }
    return SUCCESS;
}
#endif
//////////////////////////////////////////////////////////////////////////
//add for DVBC

#if 0
static void set_default_subt_lang(UINT8 lang)
{
    default_subt_lang = lang;
    LIBSUBT_PRINTF("%s : user set %dth as default subt!\n",__FUNCTION__,lang);
}
#endif
//////////////////////////////////////////////////////////////////////////

INT32 subt_descriptor(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8 lang_idx = 0;
    UINT16 pid = 0;
//    /*static*/ struct dmx_device *dmx_dev = NULL;
    /*static*/ struct sdec_device *sdec_dev = NULL;
//    RET_CODE ret_func = 0;

    if((NULL == desc)||(NULL == param))
    {
        return SI_FAILED;
    }
    pid = *((UINT16*)param);
    g_is_subt_available = TRUE;

    if( TRUE== g_pvrsubt_enable)//pvr
    {
        return SI_SUCCESS;
    }
#ifdef LIB_TSI3_FULL
    if((desc_length>=0) && (desc_length<=SUBT_MAX_DESC_SIZE))
    {
        if(pid!=0x1FFF && 0== MEMCMP(desc, prev_subt, desc_length))
        {
            return SI_SUCCESS;
        }
        MEMSET(prev_subt, 0, sizeof(prev_subt));
        MEMCPY(prev_subt, desc, desc_length);
    }
    else
    {
        return SI_FAILED;
    }
#endif

    LIBSUBT_PRINTF("%s : pid[%d], length[%d]!\n",__FUNCTION__,pid,desc_length);

    sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
//    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    if(((pid <0x20)) || (0x1fff== pid)  )
    {
        g_subt_lang_num = 0;
	 #ifdef NEW_TIMER_REC
	 if(g_subt_is_bg_record == FALSE)
	 #endif
	 {
        g_is_subt_available = FALSE;
        g_current_pid = 0x1fff;
        g_current_lang = 0xff;
        if(( TRUE== g_subt_enable) ||( TRUE== g_dcii_subt_enable))
        {
        #ifdef ATSC_SUBTITLE_SUPPORT
		    #ifdef DUAL_ENABLE
            	atsc_subt_unreg_all_dmx();		
	        	LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);			            
            	lib_subt_atsc_terminate_task();             // new added   The purpose of adding this two line code is to stop atsc_task();  
				lib_subt_atsc_section_parse_terminate_task();
				g_current_subt_type=DVB_SUBT;
			
			#else
            	lib_subt_atsc_stop_filter(dmx_index);            // new added
            	LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);
            	lib_subt_atsc_terminate_task();
            	g_current_subt_type=DVB_SUBT;
			#endif
        #endif
            LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);
            sdec_stop(sdec_dev);
            subt_unreg_all_dmx();
        }
        g_subt_desc_get = 0;
	 }
        return SI_SUCCESS;
    }
    if(desc_length<DESC_LENGTH_EIGHT)
    {
        if(g_subt_lang_num>=SUBTITLE_LANG_NUM)
        {
            return SI_SUCCESS;
        }
        for(lang_idx = 0;lang_idx<g_subt_lang_num;lang_idx++)
        {
            if((desc[0] == g_subt_lang[lang_idx].lang[0])
                &&(desc[1] == g_subt_lang[lang_idx].lang[1])
                &&(desc[2] == g_subt_lang[lang_idx].lang[2])
                &&(desc[3] == g_subt_lang[lang_idx].subt_type)
                &&((desc[4]<<8) + desc[5] == g_subt_lang[lang_idx].com_page)
                &&((desc[6]<<8) + desc[7] == g_subt_lang[lang_idx].anci_page)
                &&(pid == g_subt_lang[lang_idx].pid))
            {
                break;
            }
        }
        if(lang_idx>=g_subt_lang_num)//not same with anyone of the old
        {
            g_subt_lang[g_subt_lang_num].lang[0]='x';
            g_subt_lang[g_subt_lang_num].lang[1]='x';
            g_subt_lang[g_subt_lang_num].lang[2]='x';
            g_subt_lang[g_subt_lang_num].pid = pid;
            if (subt_event_callback != NULL)
            {
                subt_event_callback(SUBT_PID_ADD, pid);
            }
            g_subt_lang[g_subt_lang_num].subt_type = 0x10;
            g_subt_lang[g_subt_lang_num].com_page = 0;
            g_subt_lang[g_subt_lang_num].anci_page = 0;
            #ifdef ATSC_SUBTITLE_SUPPORT
               g_subt_type[g_subt_lang_num] = DVB_SUBT;
            #endif
            g_subt_lang_num++;
        }
    }
    while(desc_length>=DESC_LENGTH_EIGHT)
    {
        if(g_subt_lang_num>=SUBTITLE_LANG_NUM)
        {
            g_subt_desc_get = 1;
            return SI_SUCCESS;
        }

        for(lang_idx = 0;lang_idx<g_subt_lang_num;lang_idx++)
        {
            if((desc[0] == g_subt_lang[lang_idx].lang[0])
                &&(desc[1] == g_subt_lang[lang_idx].lang[1])
                &&(desc[2] == g_subt_lang[lang_idx].lang[2])
                &&(desc[3] == g_subt_lang[lang_idx].subt_type)
                &&((desc[4]<<8) + desc[5] == g_subt_lang[lang_idx].com_page)
                &&((desc[6]<<8) + desc[7] == g_subt_lang[lang_idx].anci_page)
                &&(pid == g_subt_lang[lang_idx].pid))
            {
                break;
            }
        }
        if(lang_idx>=g_subt_lang_num)//not same with anyone of the old
        {
                //en300468 table26
                g_subt_lang[g_subt_lang_num].lang[0]=desc[0];
                g_subt_lang[g_subt_lang_num].lang[1]=desc[1];
                g_subt_lang[g_subt_lang_num].lang[2]=desc[2];
                g_subt_lang[g_subt_lang_num].pid = pid;
                if (subt_event_callback != NULL)
                {
                    subt_event_callback(SUBT_PID_ADD, pid);
                }
                g_subt_lang[g_subt_lang_num].subt_type = desc[3];
                g_subt_lang[g_subt_lang_num].com_page = (desc[4]<<8) + desc[5];
                g_subt_lang[g_subt_lang_num].anci_page = (desc[6]<<8) + desc[7];
                #ifdef ATSC_SUBTITLE_SUPPORT
                   g_subt_type[g_subt_lang_num] = DVB_SUBT;
                #endif
                g_subt_lang_num++;
        }

        desc += 8;
        desc_length -= 8;

    }
#ifndef DUAL_ENABLE
    //For S3602f, we dont recommend using such function,
    //if app can fix below issue by other way,
    //it would be better. Need to check this with Cloud later
    //if we can remove below code or not.
    if(TRUE== get_sdec_status())
    {//cloud, restart sdec to avoid stopping sdec in dynamic pid case
      //  subt_restart(pid);
    }
#endif
    if( default_subt_lang!= 0xFF)
    {
        subt_set_language(default_subt_lang);
    }

    g_subt_desc_get = 1;
    return SI_SUCCESS;
}
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
INT32 rec_subt_descriptor(UINT16 dmx_id,UINT8 prog_index,UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8 lang_idx = 0;
    static struct dmx_device *dmx_dev = NULL;

    UINT16 pid = *((UINT16*)param);
    LIBSUBT_PRINTF("pid = %d \n",pid );
    dmx_dev=(struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (dmx_id));
    rec_monit_id_subt_lg_num[dmx_id][prog_index] = 0;
    if((pid <0x20) || (0x1fff == pid))
    {
        //g_current_pid = 0x1fff;
        rec_monit_id_subt_lg_num[dmx_id][prog_index] = 0;
        //g_current_lang = 0xff;
#if 0
        if(TRUE == g_subt_enable)
        {
            sdec_stop(sdec_dev);
            dmx_unregister_service(dmx_dev,4);
        }
#endif
        return SI_SUCCESS;
    }
    if(desc_length<DESC_LENGTH_EIGHT)
    {
        if(rec_monit_id_subt_lg_num[dmx_id][prog_index]>=SUBTITLE_LANG_NUM)
            return SI_SUCCESS;
        for(lang_idx = 0;lang_idx<rec_monit_id_subt_lg_num[dmx_id][prog_index];lang_idx++)
        {
            if((desc[0] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[0])
                &&(desc[1] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[1])
                &&(desc[2] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[2])
                &&(desc[3] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].subt_type)
                &&((desc[4]<<8) + desc[5] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].com_page)
                &&((desc[6]<<8) + desc[7] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].anci_page)
                &&(pid == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].pid))
                break;
        }

        //not same with anyone of the old
        if(lang_idx>=rec_monit_id_subt_lg_num[dmx_id][prog_index])
        {
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[0]='x';
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[1]='x';
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[2]='x';
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].pid = pid;
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].subt_type = 0x10;
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].com_page = 0;
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].anci_page = 0;
            rec_monit_id_subt_lg_num[dmx_id][prog_index] ++;
        }
    }
    while(desc_length>=DESC_LENGTH_EIGHT)
    {
        if(rec_monit_id_subt_lg_num[dmx_id][prog_index]>=SUBTITLE_LANG_NUM)
            return SI_SUCCESS;
        for(lang_idx = 0;
            lang_idx<rec_monit_id_subt_lg_num[dmx_id][prog_index];lang_idx++)
        {
            if((desc[0] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[0])
                &&(desc[1] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[1])
                &&(desc[2] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].lang[2])
                &&(desc[3] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].subt_type)
                &&((desc[4]<<8) + desc[5] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].com_page)
                &&((desc[6]<<8) + desc[7] == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].anci_page)
                &&(pid == rec_monit_id_subt_lg[dmx_id][prog_index][lang_idx].pid))
                break;
        }

        //not same with anyone of the old
        if(lang_idx>=rec_monit_id_subt_lg_num[dmx_id][prog_index])
        {

            //en300468 table26
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[0]=desc[0];
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[1]=desc[1];
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].lang[2]=desc[2];
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].pid = pid;
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].subt_type=desc[3];
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].com_page=
                (desc[4]<<8) + desc[5];
            rec_monit_id_subt_lg[dmx_id][prog_index][rec_monit_id_subt_lg_num[dmx_id][prog_index]].anci_page=
                (desc[6]<<8) + desc[7];
            rec_monit_id_subt_lg_num[dmx_id][prog_index]++;
        }
        desc += 8;
        desc_length -= 8;
    }
    return SI_SUCCESS;
}

#endif
UINT32 subt_get_language(struct t_subt_lang **list ,UINT8 *num)
{
    if ((NULL == list)||(NULL == num))
    {
        LIBSUBT_PRINTF("%s(),line %d\n");
        return 0;
    }
    *num = g_subt_lang_num;
    *list = g_subt_lang;
    return g_subt_desc_get;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
void rec_monitor_subt_get_language
    (UINT16 dmx_id,UINT8 prog_index,struct t_subt_lang **list ,UINT8 *num)
{
    if ((NULL == list)||(NULL == num))
    {
        LIBSUBT_PRINTF("%s(),line %d\n");
        return 0;
    }
    *num = rec_monit_id_subt_lg_num[dmx_id][prog_index];
    *list = rec_monit_id_subt_lg[dmx_id][prog_index];
}
#endif
UINT8 subt_get_cur_language(void)
{
    return g_current_lang;
}
INT32 subt_set_language(UINT8 lang)
{
    struct register_service subt_serv;
    /*static*/ struct sdec_device *sdec_dev = NULL;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;
    RET_CODE ret_func = 0;

    MEMSET(&subt_serv,0,sizeof(struct register_service));
    sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    LIBSUBT_PRINTF("%s : index[%d], subt_enable[%d], pvr_enable[%d]!\n",
                   __FUNCTION__,lang,g_subt_enable,g_pvrsubt_enable);
    if(lang>=g_subt_lang_num)
    {
        LIBSUBT_PRINTF("%s : Error: index out of range!\n",__FUNCTION__);
        return ERR_FAILUE;
    }

    if(( TRUE== g_subt_enable)||( TRUE== g_pvrsubt_enable)||
        ( TRUE== g_dcii_subt_enable))
    {
#ifdef ATSC_SUBTITLE_SUPPORT
	    #ifdef DUAL_ENABLE
        	ret_func=atsc_subt_unreg_all_dmx();
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
	    	LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);			                    
        	lib_subt_atsc_terminate_task();   //need remote call
        	lib_subt_atsc_section_parse_terminate_task();
		#else
        	lib_subt_atsc_stop_filter(dmx_index);        //Cpu code
          	LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);
        	lib_subt_atsc_terminate_task();   //need remote call
        #endif
    }
    LIBSUBT_PRINTF("g_subt_type[lang]=%d\n",g_subt_type[lang]);
    // Current set language index will be processed by ATSC subtitle.
    if(ATSC_SUBT==g_subt_type[lang]  )
    {
        g_current_subt_type = ATSC_SUBT;
        if(g_subt_lang[lang].pid != g_current_pid)
        {
            g_current_pid = g_subt_lang[lang].pid;
        }

        if(g_current_pid != 0x1fff)
        {
        	#ifdef DUAL_ENABLE
    	    	LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);				
            	ret_func=sdec_stop(sdec_dev);
				if(RET_SUCCESS!=ret_func)
					return RET_FAILURE;
            	lib_subt_atsc_clean_up();		//need remote call
            	lib_subt_atsc_delete_timer(); // need remote call		
				ret_func=atsc_subt_unreg_all_dmx();
				if(RET_SUCCESS!=ret_func)
					return RET_FAILURE;
				subt_serv.device = sdec_dev;
				subt_serv.request_write = (request_write)atsc_sdec_m3327_request_write;
				subt_serv.update_write = (update_write)atsc_sdec_m3327_update_write;
				subt_serv.service_pid = g_current_pid;
				subt_serv.str_type = DC2SUB_STR;
				reg_code = dmx_register_service(dmx_dev,6,&subt_serv);
				if(reg_code == RET_SUCCESS)
				{
					atsc_subt_dmx_used[dmx_index] = 1;
					LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
				}
				else
				{
					LIBSUBT_PRINTF("%s :line %d: Error: subt fail to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
				}
				LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);
				lib_subt_atsc_section_parse_create_task();//need remote call
            	lib_subt_atsc_create_task(); 	//need remote call
            	return SUCCESS;
				
			#else
			
            	ret_func = subt_unreg_all_dmx();
				if(RET_SUCCESS!=ret_func)
					return RET_FAILURE;
            	LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);
            	ret_func = sdec_stop(sdec_dev);
				if(RET_SUCCESS!=ret_func)
					return RET_FAILURE;
            	lib_subt_atsc_clean_up();        //need remote call
            	lib_subt_atsc_delete_timer(); // need remote call
            	lib_subt_atsc_start_filter(dmx_index,g_current_pid);
       			LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);
            	lib_subt_atsc_create_task();     //need remote call
            	return SUCCESS;
				
			#endif
        }
        return SUCCESS;
#else
     ret_func = sdec_stop(sdec_dev);
	 if(RET_SUCCESS!=ret_func)
		return RET_FAILURE;

#endif
    }

    if(g_subt_lang[lang].pid != g_current_pid)
    {
        if (subt_event_callback != NULL)
        {
            subt_event_callback(SUBT_PID_UPDATE, g_subt_lang[lang].pid);
        }
        g_current_pid = g_subt_lang[lang].pid;
#ifdef ATSC_SUBTITLE_SUPPORT
        g_current_subt_type = DVB_SUBT;
#endif
        if(( TRUE==g_subt_enable ) || ( TRUE== g_pvrsubt_enable))
        {
            ret_func = subt_unreg_all_dmx();
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
            subt_serv.device = sdec_dev;
            subt_serv.request_write = (request_write)sdec_request_write;
            subt_serv.update_write = (update_write)sdec_update_write;
            subt_serv.service_pid = g_current_pid;
            reg_code = dmx_register_service(dmx_dev,4,&subt_serv);
            if(RET_SUCCESS == reg_code)
            {

                subt_dmx_used[dmx_index] = 1;
                LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            else
            {
                LIBSUBT_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
        }
    }
    if(( TRUE== g_subt_enable)||( TRUE== g_pvrsubt_enable))
    {
        LIBSUBT_PRINTF("%s-sdec_start()\n",__FUNCTION__);
        ret_func = sdec_stop(sdec_dev);
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
        ret_func = sdec_start(sdec_dev,g_subt_lang[lang].com_page,g_subt_lang[lang].anci_page);
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
    }
    g_current_lang = lang;

    return SUCCESS;
}
INT32 terminate_atsc_tasks(void)
{
    
	if((g_subt_enable == TRUE)||(g_pvrsubt_enable == TRUE)||(g_dcii_subt_enable == TRUE))
	{
#ifdef ATSC_SUBTITLE_SUPPORT	
        #ifdef DUAL_ENABLE
        atsc_subt_unreg_all_dmx();
	    LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);			                    
        lib_subt_atsc_terminate_task();   //need remote call
        lib_subt_atsc_section_parse_terminate_task();
		#endif

		#ifndef DUAL_ENABLE
        lib_subt_atsc_stop_filter(dmx_index);		//Cpu code
	    LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);			                    
        lib_subt_atsc_terminate_task();   //need remote call
        #endif
#endif

	} 	

	return SUCCESS;
}
INT32 start_atsc_tasks(UINT8 lang)
{

	struct register_service subt_Serv;
	static struct sdec_device *sdec_dev = NULL;
	struct dmx_device *dmx_dev = NULL;
	RET_CODE reg_code = RET_SUCCESS;
	sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
	LIBSUBT_PRINTF("%s : index[%d], subt_enable[%d], pvr_enable[%d]!\n",__FUNCTION__,lang,g_subt_enable,g_pvrsubt_enable);
	if(lang>=g_subt_lang_num)
	{
		LIBSUBT_PRINTF("%s : Error: index out of range!\n",__FUNCTION__);
		return ERR_FAILUE;
	}
    
	
#ifdef ATSC_SUBTITLE_SUPPORT       
	
    LIBSUBT_PRINTF("g_subt_type[lang]=%d\n",g_subt_type[lang]);
    //if(g_is_atsc_subt)
    if(g_subt_type[lang] == ATSC_SUBT)                  // Current set language index will be processed by ATSC subtitle.
    {
        g_current_subt_type = ATSC_SUBT;
        if(g_subt_lang[lang].pid != g_current_pid)
        {
            g_current_pid = g_subt_lang[lang].pid;
        }

        if(g_current_pid != 0x1fff)
        {
			#ifdef DUAL_ENABLE
    	    LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);				
            sdec_stop(sdec_dev);
            lib_subt_atsc_clean_up();		//need remote call
            lib_subt_atsc_delete_timer(); // need remote call		
			atsc_subt_unreg_all_dmx();
			subt_Serv.device = sdec_dev;
			subt_Serv.request_write = (request_write)atsc_sdec_m3327_request_write;
			subt_Serv.update_write = (update_write)atsc_sdec_m3327_update_write;
			subt_Serv.service_pid = g_current_pid;
			subt_Serv.str_type = DC2SUB_STR;
			reg_code = dmx_register_service(dmx_dev,6,&subt_Serv);
			if(reg_code == RET_SUCCESS)
			{
				atsc_subt_dmx_used[dmx_index] = 1;
				LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
			}
			else
			{
				LIBSUBT_PRINTF("%s :line %d: Error: subt fail to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
			}
			LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);
			lib_subt_atsc_section_parse_create_task();//need remote call
            lib_subt_atsc_create_task(); 	//need remote call
            return SUCCESS;
			#endif

			#ifndef DUAL_ENABLE
            subt_unreg_all_dmx();
    	    LIBSUBT_PRINTF("%s-sdec_stop()\n",__FUNCTION__);				
            sdec_stop(sdec_dev);
            lib_subt_atsc_clean_up();		//need remote call
            lib_subt_atsc_delete_timer(); // need remote call
            lib_subt_atsc_start_filter(dmx_index,g_current_pid);
			LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);						
            lib_subt_atsc_create_task(); 	//need remote call
            return SUCCESS;
			#endif
        }
        return SUCCESS;

    }
	else
	{
	
		g_current_subt_type = DVB_SUBT;
		return ERR_FAILUE;	
	}

#endif

	g_current_lang = lang;

	return SUCCESS;
}

INT32 terminate_sdec_task(void)
{

	static struct sdec_device *sdec_dev = NULL;	
	
	sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
	
	sdec_stop(sdec_dev);

	return SUCCESS;
}

INT32 start_sdec_task(UINT8 lang)
{

	struct register_service subt_Serv;
	static struct sdec_device *sdec_dev = NULL;
	struct dmx_device *dmx_dev = NULL;
	RET_CODE reg_code = RET_SUCCESS;
	sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
	LIBSUBT_PRINTF("%s : index[%d], subt_enable[%d], pvr_enable[%d]!\n",__FUNCTION__,lang,g_subt_enable,g_pvrsubt_enable);
	if(lang>=g_subt_lang_num)
	{
		LIBSUBT_PRINTF("%s : Error: index out of range!\n",__FUNCTION__);
		return ERR_FAILUE;
	}
    

	if(g_subt_lang[lang].pid != g_current_pid)
	{
		if (subt_event_callback != NULL)
			subt_event_callback(SUBT_PID_UPDATE, g_subt_lang[lang].pid);
		g_current_pid = g_subt_lang[lang].pid;
	#ifdef ATSC_SUBTITLE_SUPPORT
		if(g_subt_type[lang] != ATSC_SUBT)
			g_current_subt_type = DVB_SUBT;
		else
		{
			g_current_subt_type = ATSC_SUBT;
			return ERR_FAILUE;
		}
	#endif
		if((g_subt_enable == TRUE) || (g_pvrsubt_enable == TRUE))
		{			
			subt_unreg_all_dmx();
			subt_Serv.device = sdec_dev;
			subt_Serv.request_write = (request_write)sdec_request_write;
			subt_Serv.update_write = (update_write)sdec_update_write;
			subt_Serv.service_pid = g_current_pid;
			reg_code = dmx_register_service(dmx_dev,4,&subt_Serv);
			if(reg_code == RET_SUCCESS)
			{
				subt_dmx_used[dmx_index] = 1;
				LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
			}
			else
			{
				LIBSUBT_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
			}
		}
	}
	if((g_subt_enable == TRUE)||(g_pvrsubt_enable == TRUE))
	{
	LIBSUBT_PRINTF("%s-sdec_start()\n",__FUNCTION__);	
	    sdec_stop(sdec_dev);
		sdec_start(sdec_dev,g_subt_lang[lang].com_page,g_subt_lang[lang].anci_page);
	}
	g_current_lang = lang;

	return SUCCESS;
}

/*
subt_show_onoff is called when user graphic off/on,
because subtitle can not overlap with user graphic,
so when user graphic show , subtitle off, user graphic not show , subtitle on.
*/

INT32 subt_show_onoff(BOOL b_on)
{
	if((TRUE!=b_on)&&(FALSE!=b_on))
		return RET_FAILURE;
#ifdef ATSC_SUBTITLE_SUPPORT
    lib_subt_atsc_show_onoff(b_on);
#endif
    LIBSUBT_PRINTF("%s : On[%d]!\n",__FUNCTION__,b_on);
    if( TRUE== b_on)
    {
#if 0//def _MHEG5_SUPPORT_
        tm_mheg5stop_2();
#endif
        osd_subt_enter();
    }
    else
    {
#if 0//def _MHEG5_SUPPORT_
        tm_mheg5start_2();
#endif
        osd_subt_leave();
    }

    return RET_SUCCESS;
}

/*
    subt_enable is for special use .
    Now, it's disabled when can not share mem DVW
*/
INT32 subt_enable(BOOL enable)
{
    struct register_service subt_serv;
    /*static*/ struct sdec_device *sdec_dev = NULL;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;
    RET_CODE ret_func =  0;

    MEMSET(&subt_serv,0,sizeof(struct register_service));
    sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    LIBSUBT_PRINTF("%s : Enable[%d]!\n",__FUNCTION__,enable);
#ifdef ATSC_SUBTITLE_SUPPORT
    if(ATSC_SUBT == g_current_subt_type)
    {
        if((g_subt_lang_num>0) && (g_current_pid != 0x1fff))
        {
            ret_func =subt_unreg_all_dmx();
			
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
			
            LIBSUBT_PRINTF("%s-sdec_stop() -Line(%d)\n",__FUNCTION__,__LINE__);
            ret_func =sdec_stop(sdec_dev);
			
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
			
            if(enable)
            {
                g_dcii_subt_enable = TRUE;
                g_subt_enable = TRUE;
				#ifdef DUAL_ENABLE
               
                	atsc_subt_unreg_all_dmx();
					subt_serv.device = sdec_dev;
					subt_serv.request_write = (request_write)atsc_sdec_m3327_request_write;
					subt_serv.update_write = (update_write)atsc_sdec_m3327_update_write;
					subt_serv.service_pid = g_current_pid;
					subt_serv.str_type = DC2SUB_STR;
					reg_code = dmx_register_service(dmx_dev,6,&subt_serv);
					if(reg_code == RET_SUCCESS)
					{
						atsc_subt_dmx_used[dmx_index] = 1;
						LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
					}
					else
					{
						LIBSUBT_PRINTF("%s :line %d: Error: subt fail to start dmx[%d]!\n",__FUNCTION__,__LINE__,dmx_index);
					}
		   			LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);	
		   			lib_subt_atsc_section_parse_create_task();//need remote call
                	lib_subt_atsc_create_task();
					
				#else
				
                	lib_subt_atsc_start_filter(dmx_index,g_current_pid);
                	LIBSUBT_PRINTF("%s-lib_subt_atsc_create_task()\n",__FUNCTION__);
                	lib_subt_atsc_create_task();
					
				#endif
            }
            else
            {
                g_dcii_subt_enable = FALSE;
                g_subt_enable = FALSE;
				#ifdef DUAL_ENABLE
				
					atsc_subt_unreg_all_dmx();
					LIBSUBT_PRINTF("%s-sdec_stop() -Line(%d)\n",__FUNCTION__,__LINE__); 		
					g_current_pid = 0x1fff;
                	lib_subt_atsc_clean_up();
                	lib_subt_atsc_delete_timer();
	      			LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);			            				
                	lib_subt_atsc_terminate_task();
					lib_subt_atsc_section_parse_terminate_task();
				
				#else
				
                	lib_subt_atsc_stop_filter(dmx_index);
                	lib_subt_atsc_clean_up();
                	lib_subt_atsc_delete_timer();
                	LIBSUBT_PRINTF("%s-lib_subt_atsc_terminate_task()\n",__FUNCTION__);
                	lib_subt_atsc_terminate_task();
				
				#endif
            }
            return SUCCESS;
        }
        if((g_subt_lang_num>0) && ( 0x1fff== g_current_pid))
        {
        	#ifdef DUAL_ENABLE
            atsc_subt_unreg_all_dmx();
            lib_subt_atsc_clean_up();
            lib_subt_atsc_delete_timer();
			#else
            lib_subt_atsc_stop_filter(dmx_index);
            lib_subt_atsc_clean_up();
            lib_subt_atsc_delete_timer();
			#endif
        }
        return SUCCESS;
    }
#endif
    if( TRUE== enable)
    {
        if((g_subt_lang_num>0) &&( g_current_pid != 0x1fff))
        {
            ret_func =subt_unreg_all_dmx();
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
            subt_serv.device = sdec_dev;
            subt_serv.request_write = (request_write)sdec_request_write;
            subt_serv.update_write = (update_write)sdec_update_write;
            subt_serv.service_pid = g_current_pid;
            reg_code = dmx_register_service(dmx_dev,4,&subt_serv);
            if( RET_SUCCESS== reg_code)
            {
                subt_dmx_used[dmx_index] = 1;
                LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            else
            {
                LIBSUBT_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            LIBSUBT_PRINTF("%s-sdec_start()\n",__FUNCTION__);
            ret_func =sdec_start(sdec_dev,g_subt_lang[g_current_lang].com_page,
                                 g_subt_lang[g_current_lang].anci_page);
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
        }
        g_subt_enable = TRUE;
    }
    else
    {
        ret_func =subt_unreg_all_dmx();
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
        LIBSUBT_PRINTF("%s-sdec_stop() -Line(%d)\n",__FUNCTION__,__LINE__);
        ret_func =sdec_stop(sdec_dev);
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
        g_current_pid = 0x1fff;
        g_subt_enable = FALSE;
    }
    return SUCCESS;
}

void subt_pvr_set_language(struct t_subt_lang *list ,UINT8 num)
{
    UINT8 i = 0;
    int n_size = sizeof(struct t_subt_lang);
	if((NULL==list)||(0>=num)||(0xff<=num))
		return;
    g_subt_lang_num = num;
    LIBSUBT_PRINTF("%s : num[%d]!\n",__FUNCTION__,num);
    for(i=0; i<num; i++)
    {
        MEMCPY(&(g_subt_lang[i]),&(list[i]),n_size);
    }
}

INT32 subt_pvr_enable(BOOL enable,UINT16 dmx_id)
{
    struct register_service subt_serv;
    /*static*/ struct sdec_device *sdec_dev = NULL;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;
    RET_CODE ret_func =  0;

    MEMSET(&subt_serv,0,sizeof(struct register_service));
    sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    LIBSUBT_PRINTF("%s : PVR subt Enable[%d] DMX[%d]!\n",__FUNCTION__,enable,dmx_id);
	if(3<=dmx_id)
		return RET_FAILURE;
    if( TRUE== enable)
    {
        dmx_index = dmx_id;
        if((g_subt_lang_num>0) && (g_current_pid != 0x1fff))
        {
            ret_func = dmx_unregister_service(dmx_dev,4);
			
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;
            subt_serv.device = sdec_dev;
            subt_serv.request_write = (request_write)sdec_request_write;
            subt_serv.update_write = (update_write)sdec_update_write;
            subt_serv.service_pid = g_current_pid;
            reg_code=dmx_register_service(dmx_dev,4,&subt_serv);
            if(RET_SUCCESS == reg_code)
            {
                subt_dmx_used[dmx_index] = 1;
                LIBSUBT_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            else
            {
                LIBSUBT_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            ret_func = sdec_start(sdec_dev,g_subt_lang[g_current_lang].com_page,
                                  g_subt_lang[g_current_lang].anci_page);
			
			if(RET_SUCCESS!=ret_func)
				return RET_FAILURE;

        }
        g_pvrsubt_enable = TRUE;
    }
    else
    {
        if(g_pvrsubt_enable)
        {
            dmx_index = 0;
        }
        ret_func = dmx_unregister_service(dmx_dev,4);
		
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
		
        ret_func = sdec_stop(sdec_dev);
		
		if(RET_SUCCESS!=ret_func)
			return RET_FAILURE;
		
        g_current_pid = 0x1fff;
        g_pvrsubt_enable = FALSE;
        g_subt_lang_num = 0;
#ifdef LIB_TSI3_FULL
        MEMSET(prev_subt, 0, sizeof(prev_subt));
        subt_descriptor_reset();
#endif

    }
    return SUCCESS;
}

BOOL subt_check_enable(void)
{
    return g_subt_enable;
}
#if 0
static BOOL dcii_subt_check_enable(void)
{
    return g_dcii_subt_enable;
}
#endif

void subt_reg_callback(SUBT_EVENT_CALLBACK callback)
{
    osal_task_dispatch_off();
    subt_event_callback = callback;
    osal_task_dispatch_on();
}

void subt_unreg_callback(void)
{
    osal_task_dispatch_off();
    subt_event_callback = NULL;
    osal_task_dispatch_on();
}

#ifdef ATSC_SUBTITLE_SUPPORT
INT32 subt_set_para(UINT16 pid, UINT16 subt_lang_num)
{
	g_current_pid = pid;
	g_current_subt_type = ATSC_SUBT;
	g_subt_lang_num = subt_lang_num;
	return SUCCESS;
}
#endif

#endif
