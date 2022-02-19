/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_ttx.c

   *    Description:define the ttx interface used by UI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libtsi/si_types.h>
//#include <api/libtsi/si_table.h>

#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>

#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_monitor.h>
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#endif

#ifndef SEE_CPU
#define LIBTTX_PRINTF PRINTF

struct vbi_device *ttx_dev = NULL;
static struct t_ttx_lang g_init_page[TTX_SUBT_LANG_NUM] = {{0,0,{0},0},};
static struct t_ttx_lang g_subt_page[TTX_SUBT_LANG_NUM] = {{0,0,{0},0},};

static UINT8 g_init_page_num = 0;
static UINT8 g_subt_page_num = 0;

static UINT16 dmx_index = 0;
OSAL_ID TTX_SEM_ID = INVALID_ID;
static BOOL g_ttx_enable = FALSE;
static BOOL g_ttx_channel_enable = FALSE;
static BOOL g_ttx_pvr_enable = FALSE;


#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static struct t_ttx_lang rec_monit_id_init_page[2][2][TTX_SUBT_LANG_NUM] = {{{{0,0,{0},0},},},};
static UINT8 rec_monit_id_init_page_num[2][2] = {{0}};
static struct t_ttx_lang rec_monit_id_subt_page[2][2][TTX_SUBT_LANG_NUM] = {{{{0,0,{0},0},},},};
static UINT8 rec_monit_id_subt_page_num[2][2] = {{0}};
#endif

#ifdef LIB_TSI3_FULL
static UINT8 prev_ttext[256] = {0};
static struct desc_table ttx_desc_info[] = {
    {
        TELTEXT_DESCRIPTOR,
        0,
        (desc_parser_t)ttx_descriptor,
    },
};
static UINT8 ttx_cur_monitor_id = 0xff;
#endif

static UINT8 ttx_dmx_used[3] = {0,0,0};

static UINT32 g_ttx_desc_get = 0;

static UINT8 g_current_init_page = 0xff;
static UINT8 g_current_subt_page = 0xff;
static UINT16 g_current_pid =0x1fff;

static BOOL g_init_page_enable = TRUE;
static UINT16 g_init_page_id = 100;

static UINT8 *g_current_language = NULL;

static TTX_EVENT_CALLBACK ttx_event_callback = NULL;

static BOOL g_is_ttx_available = FALSE;

BOOL ttx_is_available(void)
{
    return g_is_ttx_available;
}

#if 0
static void set_seperate_ttxsubt_mode(UINT8 mode)
{
    INT32 func_ret = 0;

    seperate_ttxsubt_mode = mode;
    func_ret =
    vbi_ioctl(ttx_dev,IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT, (UINT32)mode);
}

static void set_ttx_history_mode(BOOL mode)
{
    INT32 func_ret = 0;

    func_ret = vbi_ioctl(ttx_dev,IO_VBI_WORK_MODE_HISTORY, (UINT32)mode);
}
#endif


INT32 ttxeng_open(UINT8 subtitle,UINT16 page_num)
{
    INT32 func_ret = ERR_FAILURE;

    if((subtitle>TTXENG_TEXT)||((page_num<TTX_PAGE_MIN_NUM)||(page_num>TTX_PAGE_MAX_NUM)))
    {
        return ERR_FAILURE;
    }

    osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME);
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_OPEN,(((subtitle&0xff)<<16)|(page_num&0xffff)));
    osal_semaphore_release(TTX_SEM_ID);
    return func_ret;

}

void  ttxeng_close(void)
{
    INT32 func_ret = -1;

    //libc_printf("--TTXEng_Close--\n");
    osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME);
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_CLOSE, 0);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    osal_semaphore_release(TTX_SEM_ID);
}

static void ttxeng_upd_page(UINT16 page_id, UINT8 line_id)
{
    INT32 func_ret = -1;
    if(((page_id<TTX_PAGE_MIN_NUM)||(page_id>TTX_PAGE_MAX_NUM))||(line_id>TTX_MAX_ROW))
    {
        return;
    }
    osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME);
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_UPDATE_PAGE,(((page_id&0xffff)<<8)|(line_id&0xff)));
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    osal_semaphore_release(TTX_SEM_ID);
}

void  ttxeng_show_on_off(BOOL b_on)
{
    INT32 func_ret = -1;

    if((TRUE != b_on)&&(FALSE != b_on))
    {
        return;
    }

    osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME);
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_SHOW_ON_OFF, b_on);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    osal_semaphore_release(TTX_SEM_ID);
}

void ttxeng_send_key(TTX_KEYTYPE key)
{
    INT32 func_ret = -1;

    if(key>TTX_KEY_NORESPONSE)
    {
        return;
    }
    osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME);
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_SEND_KEY, (UINT32)key);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    osal_semaphore_release(TTX_SEM_ID);
}

void ttxeng_default_g0_set(UINT8 g0_set)
{
    INT32 func_ret = -1;
 
    if(g0_set >=ARABIC_HEBREW)
    {
        return;
    }
    if(NULL == ttx_dev)
    {
        ttx_dev = (struct vbi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_VBI);
        if(NULL == ttx_dev)
        {
            PRINTF("ttx_dev NOT exist!!\n");
            return;//ASSERT(0);
        }
    }
    ttx_default_g0_set(ttx_dev,g0_set);
    func_ret = vbi_ioctl(ttx_dev,IO_TTX_USER_DSG_FONT, (UINT32)g0_set);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
}

#ifdef LIB_TSI3_FULL
static INT32 ttx_callback(UINT8 *buff, INT32 len, UINT32 param)
{
    struct pmt_section *pms = NULL;
    struct pmt_stream_info *stream = NULL;
    UINT16 es_pid = 0;
    UINT32 prog_info_length = 0;
    UINT32 es_info_length = 0;
    INT32 i = 0;
    INT32 func_ret = ERR_FAILURE;

    if((NULL == buff)||(0xffff == len))
    {
        return func_ret;
    }

    pms = (struct pmt_section *)buff;

    g_is_ttx_available = FALSE;
    if(ttx_is_holded())
    {
        return SUCCESS;
    }

    prog_info_length = SI_MERGE_HL8(pms->program_info_length);

    for(i=sizeof(struct pmt_section)+prog_info_length-4;i<(len-4);i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);

        func_ret = desc_loop_parser(stream->descriptor, es_info_length, ttx_desc_info, 1, NULL, (void *)&es_pid);
    }
    return func_ret;
}

INT32 ttx_unregister(UINT32 monitor_id)
{
    struct sim_cb_param param;
    UINT16 stuff_pid = 0x1FFF;
    INT32 func_ret = ERR_FAILURE;

    MEMSET(&param,0x0,sizeof(struct sim_cb_param));

    if((SUCCESS == sim_get_monitor_param(monitor_id, &param)))
    {
        if( sim_unregister_scb(monitor_id, ttx_callback) != SUCCESS)
        {
            LIBTTX_PRINTF("ttx unreg to monitor fail. %d\n",monitor_id);
            return func_ret;
        }
    }
    LIBTTX_PRINTF("ttx unreg to monitor. %d\n",monitor_id);

    if(FALSE == ttx_is_holded())
    {
        func_ret = ttx_descriptor(TELTEXT_DESCRIPTOR, 0, prev_ttext, (void *)&stuff_pid);
    }
    return func_ret;
}

#ifdef NEW_TIMER_REC
static UINT32 ttx_living_monitor_id;
UINT32 g_ttx_is_bg_record = FALSE;

void ttx_set_is_bg_record(BOOL flag)
{
	g_ttx_is_bg_record = flag;
}

static void ttx_set_monitor_id(monitor_id)
{
	ttx_living_monitor_id = monitor_id;
}

UINT32 ttx_get_monitor_id(void)
{
	return ttx_living_monitor_id;
}
#endif

INT32 ttx_register(UINT32 monitor_id)
{
    struct sim_cb_param param;
    INT32 func_ret = ERR_FAILURE;

    MEMSET(&param,0,sizeof(struct sim_cb_param));
    
    if(sim_get_monitor_param(monitor_id, &param) != SUCCESS)
    {
        LIBTTX_PRINTF("ttx reg to monitor fail1. %d\n",monitor_id);
        return func_ret;
    }

    func_ret = ttx_unregister(ttx_cur_monitor_id);

    if(SUCCESS == sim_register_scb(monitor_id, ttx_callback, NULL))
    {
        dmx_index = param.dmx_idx;
        LIBTTX_PRINTF("ttx reg to monitor succ. %d\n",monitor_id);
	 #ifdef NEW_TIMER_REC
	 ttx_set_monitor_id(ttx_cur_monitor_id);
	 #endif
        ttx_cur_monitor_id = monitor_id;
        return SUCCESS;
    }

    LIBTTX_PRINTF("ttx reg to monitor fail2. %d\n",monitor_id);
    return func_ret;
}
#endif

static RET_CODE ttx_unreg_all_dmx(void)
{
    struct dmx_device *dmx_dev  = NULL;
    RET_CODE reg_code = ERR_FAILURE;
    UINT8 i = 0;

    for(i=0; i<3; i++)
    {
        if(ttx_dmx_used[i])
        {
            dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i);
            reg_code = dmx_unregister_service(dmx_dev,3);
            if(reg_code != RET_SUCCESS)
            {
                LIBTTX_PRINTF("ttx unregister dmx %d failed\n",i);
                break;
            }
            ttx_dmx_used[i]=0;
        }
    }
    return reg_code;
}

static void ttx_g_lang_init(UINT8 teletext_type,UINT16 pid)
{
    if(((pid <0x20) || (0x1fff== pid))||(teletext_type>TTX_TYPE_5))
    {
        return;
    }
    g_init_page[g_init_page_num].lang[0]='x';
    g_init_page[g_init_page_num].lang[1]='x';
    g_init_page[g_init_page_num].lang[2]='x';
    g_subt_page[g_subt_page_num].ttx_type = teletext_type;
    g_init_page[g_init_page_num].pid = pid;
}

static void ttx_g_lang_set(UINT8 *desc,UINT8 teletext_type,UINT16 pid)
{
    if(((pid <0x20) || (0x1fff== pid))||(teletext_type>TTX_TYPE_5)||(NULL == desc))
    {
        return;
    }
    g_init_page[g_init_page_num].lang[0]=desc[0];
    g_init_page[g_init_page_num].lang[1]=desc[1];
    g_init_page[g_init_page_num].lang[2]=desc[2];
    g_subt_page[g_subt_page_num].ttx_type = teletext_type;
    g_init_page[g_init_page_num].pid = pid;
}

static void ttx_subt_g_lang_set(UINT8 *desc,UINT8 teletext_type,UINT16 pid)
{
    if(((pid <0x20) || (0x1fff== pid))||(teletext_type>TTX_TYPE_5)||(NULL == desc))
    {
        return;
    }
    g_subt_page[g_subt_page_num].lang[0]=desc[0];
    g_subt_page[g_subt_page_num].lang[1]=desc[1];
    g_subt_page[g_subt_page_num].lang[2]=desc[2];
    g_subt_page[g_subt_page_num].ttx_type = teletext_type;
    g_subt_page[g_subt_page_num].pid = pid;
}

static void ttx_globe_param_set(void)
{
    g_current_pid = 0X1FFF;
    g_init_page_num = 0;
    g_subt_page_num = 0;
    g_current_init_page = 0xff;
    g_current_subt_page = 0xff;
    g_is_ttx_available = FALSE;
#ifdef LIB_TSI3_FULL
    MEMSET(prev_ttext, 0, sizeof(prev_ttext));
#endif
}

static BOOL ttx_param_select(UINT8 *desc,UINT8 teletext_type,UINT16 pid,UINT8 lang_idx,UINT16 page)
{
     if((desc[0] == g_init_page[lang_idx].lang[0]) &&
      (desc[1] == g_init_page[lang_idx].lang[1]) &&
      (desc[2] == g_init_page[lang_idx].lang[2])&&
      (teletext_type == g_subt_page[lang_idx].ttx_type)&&
      (pid == g_init_page[lang_idx].pid)&&
      (page == g_init_page[lang_idx].page))
     {
          return TRUE;
     }
     else
     {
        return FALSE;
     }

}

static BOOL ttx_subt_param_select(UINT8 *desc,UINT8 teletext_type,UINT16 pid,UINT8 lang_idx,UINT16 page)
{
    if((desc[0] == g_subt_page[lang_idx].lang[0])
        &&(desc[1] == g_subt_page[lang_idx].lang[1])
        &&(desc[2] == g_subt_page[lang_idx].lang[2])
        &&(teletext_type == g_subt_page[lang_idx].ttx_type)
        &&(pid == g_subt_page[lang_idx].pid)
        &&(page == g_subt_page[lang_idx].page))
     {
         return TRUE;
     }
     else
     {
         return FALSE;
     }
}

static void ttx_param_setting(UINT16 page,BOOL *init_page_update)
{
    if((NULL == init_page_update)||(page>TTX_PAGE_MAX_NUM))
    {
        return;
    }
     g_init_page[g_init_page_num].page = page;
     g_init_page_num++;
     *init_page_update = TRUE;
}

static void ttx_descriptor_exit_fn(BOOL init_page_update,BOOL subt_page_update,struct ttx_page_info *ttx_page_update)
{
    RET_CODE func_ret = ERR_FAILURE;

    if(NULL == ttx_page_update)
    {
        return;
    }

    if(init_page_update)
    {
        ttx_page_update->num = g_init_page_num;
        ttx_page_update->page_addr = (UINT32)g_init_page;
        func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_UPDATE_INIT_PAGE, (UINT32)ttx_page_update);
		func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    }
    if(subt_page_update)
    {
        ttx_page_update->num = g_subt_page_num;
        ttx_page_update->page_addr = (UINT32)g_subt_page;
        func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_UPDATE_SUBT_PAGE, (UINT32)ttx_page_update);
		func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
    }
    g_ttx_desc_get = 1;
}

/*ttx_descriptor : called by si_moniter to init ttx and refresh ttx info.*/
INT32 ttx_descriptor(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8 lang_idx = 0;
    UINT8 mag = 0;
    UINT8 teletext_type = 0;
    UINT16 page=100;
    UINT16 pid = 0;
    BOOL init_page_update = FALSE;
    BOOL subt_page_update = FALSE;
    INT32 func_ret = RET_FAILURE;
    struct register_service ttx_serv ;
    struct ttx_page_info ttx_page_update;
    struct dmx_device *dmx_dev = NULL;
    
    if(NULL == param)
    {
        return ERR_FAILURE;
    }
    pid = *((UINT16*)param);
    MEMSET(&ttx_page_update,0,sizeof(struct ttx_page_info));
    MEMSET(&ttx_serv,0,sizeof(struct register_service));
    g_is_ttx_available = TRUE;

    if(TRUE == g_ttx_pvr_enable)
    {
        ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
        return SI_SUCCESS;
    }
    if(NULL == desc)
    {
        LIBTTX_PRINTF("NULL descriptor data!\n");
        return ERR_FAILURE;
    }
#ifdef LIB_TSI3_FULL
    if((desc_length>TTX_NORMAL_0) && (desc_length<TTX_DSCRIPT_MAX_LENGTH) )
    {
        if((0x1FFF != pid)&&(0 == MEMCMP(desc, prev_ttext, desc_length)))
        {
            ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
            return SI_SUCCESS;
        }
        MEMSET(prev_ttext, 0, sizeof(prev_ttext));
        MEMCPY(prev_ttext, desc, desc_length);
    }
    else if(TTX_NORMAL_0==desc_length)
    {
        if((pid>=0x20)&&(pid<0x1fff)) //valid pid
        {
            for(lang_idx = 0;lang_idx<g_init_page_num;lang_idx++)
            {
                if(pid==g_init_page[lang_idx].pid)//check exist already
                {
                    ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
                    return SI_SUCCESS;
                }
            }
        }
    }
#endif
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    #ifdef NEW_TIMER_REC
    if(g_ttx_is_bg_record == FALSE)
    #endif
    {
    if((pid <0x20) || (0x1fff== pid))
    {
        LIBTTX_PRINTF("%s: invalid pid 0x%X!!\n",__FUNCTION__,pid);
        ttx_globe_param_set();
        if( TRUE== g_ttx_enable)
        {
            if(RET_SUCCESS!=vbi_stop(ttx_dev))
            {
                LIBTTX_PRINTF("vbi_close failed!!\n");
                return ERR_FAILURE;//ASSERT(0);
            }
        }
        func_ret = ttx_unreg_all_dmx();
		
		if(RET_SUCCESS!=func_ret)
			return ERR_FAILURE;	
		
        g_ttx_channel_enable = FALSE;
        g_ttx_desc_get = 0;
        ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
        return SI_SUCCESS;
    }
    if( TRUE== g_ttx_enable)
    {
        if(FALSE == g_ttx_channel_enable)
        {
            ttx_serv.device = ttx_dev;
            ttx_serv.request_write = (request_write)vbi_request_write;
            ttx_serv.update_write = (update_write)vbi_update_write;
            ttx_serv.service_pid = pid;
            if(RET_SUCCESS == dmx_register_service(dmx_dev,TTX_REGISTER_NUM,&ttx_serv))
            {
                ttx_dmx_used[dmx_index] = 1;
            }
            g_ttx_channel_enable = TRUE;
        }
        if(RET_SUCCESS!=vbi_start(ttx_dev,ttxeng_upd_page))
        {
            LIBTTX_PRINTF("vbi_open failed!!\n");
            return ERR_FAILURE;//ASSERT(0);
        }
    }
    }
    if(desc_length < TTX_NORMAL_5)
    {
        teletext_type=0;
        if(g_init_page_num>=TTX_SUBT_LANG_NUM)
        {
            ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
            return SI_SUCCESS;
        }
        for(lang_idx = 0;lang_idx<g_init_page_num;lang_idx++)
        {
            if(pid == g_init_page[lang_idx].pid)
            {
                break;
            }
        }
        if(lang_idx>=g_init_page_num)
        {              
            ttx_g_lang_init(teletext_type,pid);
            // Oncer2013-3-22: Add for SAT2IP
            if (NULL != ttx_event_callback)
            {
                ttx_event_callback(TTX_PID_ADD, pid);
            }
            ttx_param_setting(page,&init_page_update);
        }
    }
    while(desc_length>=TTX_NORMAL_5)
    {
        mag = (desc[3]&0x07)?(desc[3]&0x07):8;
        page = (desc[4]>>4)*10 + (desc[4]&0x0f);
        page += (mag * 100);
        teletext_type = desc[3]>>3;
        if((TTX_TYPE_1==teletext_type) || (TTX_TYPE_3==teletext_type )|| (TTX_TYPE_0==teletext_type))
        {
            if(g_init_page_num>=TTX_SUBT_LANG_NUM)
            {
                ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
                return SI_SUCCESS;
            }
            for(lang_idx = 0;lang_idx<g_init_page_num;lang_idx++)
            {
                if(TRUE == ttx_param_select(desc,teletext_type,pid,lang_idx,page))
                {
                    break;
                }
            }
            if(lang_idx>=g_init_page_num)
            {     //Colin
                ttx_g_lang_set(desc,teletext_type,pid);
                // Oncer2013-3-22: Add for SAT2IP
                if (NULL != ttx_event_callback)
                {
                    ttx_event_callback(TTX_PID_ADD, pid);
                }
                ttx_param_setting(page,&init_page_update);
            }
        }
        else if( (TTX_TYPE_2==teletext_type)|| (TTX_TYPE_5==teletext_type) )
        {//subtitle teletext page & subtitle teletext page for impaired people
            if((g_subt_page_num>=TTX_SUBT_LANG_NUM)||(TTX_PAGE_MIN_NUM==page))
            {  // Colin>> for BUG 42268 the special stream
                ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
                return SI_SUCCESS;
            }
            for(lang_idx = 0;lang_idx<g_subt_page_num;lang_idx++)
            {
                if(TRUE == ttx_subt_param_select(desc,teletext_type,pid,lang_idx,page))
                {
                        break;
                }
            }
            if(lang_idx>=g_subt_page_num)
            {
                ttx_subt_g_lang_set(desc,teletext_type,pid);
                // Oncer2013-3-22: Add for SAT2IP
                if (NULL != ttx_event_callback)
                {
                    ttx_event_callback(TTX_PID_ADD, pid);
                }
                g_subt_page[g_subt_page_num].page = page;
                g_subt_page_num++;
                subt_page_update = TRUE;
            }
            if(/*page==100 &&*/ 0==g_init_page_num)
            {
                ttx_g_lang_set(desc,teletext_type,pid);
                // Oncer2013-3-22: Add for SAT2IP
                if (NULL != ttx_event_callback)
                {
                    ttx_event_callback(TTX_PID_ADD, pid);
                }
                g_init_page[g_init_page_num].page = page;
                g_init_page_num++;

                if((  1==g_subt_page_num) && ( 100== g_subt_page[0].page))
                {
                    g_subt_page_num = 0;//it is a wrong header, cloud
                }
                init_page_update = TRUE;
            }
        }
        desc +=5;
        desc_length -= 5;
    }
    ttx_descriptor_exit_fn(init_page_update,subt_page_update,&ttx_page_update);
    return SI_SUCCESS;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
INT32 rec_ttx_descriptor(UINT16 dmx_id,UINT8 prog_index,UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8 lang_idx = 0;
    UINT8 mag = 0;
    UINT8 teletext_type = TTX_TYPE_0;
    UINT16 page=100;
    UINT16 pid = *((UINT16*)param);

    //libc_printf("desc_length=%d,pid=%d,g_init_page_num=%d\n",desc_length,pid, g_init_page_num);
    if(NULL == desc)
    {
        return ERR_FAILURE;
    }

    if(desc_length < TTX_NORMAL_5)
    {
       if(rec_monit_id_init_page_num[dmx_id][prog_index]>=TTX_SUBT_LANG_NUM)
       {
           return SI_SUCCESS;
       }
       for(lang_idx = 0; lang_idx<rec_monit_id_init_page_num[dmx_id][prog_index];lang_idx++)
       {
            if((desc[0] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[0])
                &&(desc[1] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[1])
                &&(desc[2] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[2])
                &&(pid == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].pid)
                &&(page == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].page))
            {
                    break;
            }
        }
        if(lang_idx>=rec_monit_id_init_page_num[dmx_id][prog_index])
        {
            rec_monit_id_init_page[dmx_id][prog_index]
                [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[0]='x';
            rec_monit_id_init_page[dmx_id][prog_index]
                [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[1]='x';
            rec_monit_id_init_page[dmx_id][prog_index]
                [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[2]='x';
            rec_monit_id_init_page[dmx_id][prog_index]
                [rec_monit_id_init_page_num[dmx_id][prog_index]].pid = pid;
            rec_monit_id_init_page[dmx_id][prog_index]
                [rec_monit_id_init_page_num[dmx_id][prog_index]].page = page;
            rec_monit_id_init_page_num[dmx_id][prog_index] ++;
        }
    }

    while(desc_length>=TTX_NORMAL_5)
    {
        mag = (desc[3]&0x07)?(desc[3]&0x07):8;
        page = (desc[4]>>4)*10 + (desc[4]&0x0f);
        page += (mag * 100);
        teletext_type = desc[3]>>3;
        //libc_printf("mag=%d,page=%d,teletext_type=%d,g_init_page_num=%d\n",
        //mag,page,teletext_type,g_init_page_num);

        //initial teletext page or additional information page
        if((TTX_TYPE_1==teletext_type) || (TTX_TYPE_3==teletext_type))
        {
            if(rec_monit_id_init_page_num[dmx_id][prog_index] >= TTX_SUBT_LANG_NUM)
            {
               return SI_SUCCESS;
            }

            for(lang_idx = 0;lang_idx<rec_monit_id_init_page_num[dmx_id][prog_index];lang_idx++)
            {
                if((desc[0] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[0])
                   &&(desc[1] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[1])
                   &&(desc[2] == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].lang[2])
                   &&(pid == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].pid)
                   &&(page == rec_monit_id_init_page[dmx_id][prog_index][lang_idx].page))
                {
                    break;
            }
            }
            if(lang_idx>=rec_monit_id_init_page_num[dmx_id][prog_index])
            {
               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[0]=desc[0];

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[1]=desc[1];

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[2]=desc[2];

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].pid = pid;

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].page = page;
               rec_monit_id_init_page_num[dmx_id][prog_index]++;
            }
        }
        else if( (TTX_TYPE_2== teletext_type) || (TTX_TYPE_5== teletext_type))
        {//subtitle teletext page & subtitle teletext page for impaired people
            if(rec_monit_id_subt_page_num[dmx_id][prog_index] >= TTX_SUBT_LANG_NUM)
            {
                return SI_SUCCESS;
            }
            for(lang_idx = 0; lang_idx<rec_monit_id_subt_page_num[dmx_id][prog_index];lang_idx++)
            {
                if((desc[0] == rec_monit_id_subt_page[dmx_id][prog_index][lang_idx].lang[0])
                   &&(desc[1] == rec_monit_id_subt_page[dmx_id][prog_index][lang_idx].lang[1])
                   &&(desc[2] == rec_monit_id_subt_page[dmx_id][prog_index][lang_idx].lang[2])
                   &&(pid == rec_monit_id_subt_page[dmx_id][prog_index][lang_idx].pid)
                   &&(page == rec_monit_id_subt_page[dmx_id][prog_index][lang_idx].page))
                {
                    break;
                }
            }
            if(lang_idx>=rec_monit_id_subt_page_num[dmx_id][prog_index])
            {
               rec_monit_id_subt_page[dmx_id][prog_index]
               [rec_monit_id_subt_page_num[dmx_id][prog_index]].lang[0]=desc[0];

               rec_monit_id_subt_page[dmx_id][prog_index]
               [rec_monit_id_subt_page_num[dmx_id][prog_index]].lang[1]=desc[1];

               rec_monit_id_subt_page[dmx_id][prog_index]
               [rec_monit_id_subt_page_num[dmx_id][prog_index]].lang[2]=desc[2];


               rec_monit_id_subt_page[dmx_id][prog_index]
               [rec_monit_id_subt_page_num[dmx_id][prog_index]].pid = pid;

               rec_monit_id_subt_page[dmx_id][prog_index]
               [rec_monit_id_subt_page_num[dmx_id][prog_index]].page = page;

                rec_monit_id_subt_page_num[dmx_id][prog_index]++;
            }

            //some special streams have teletext_type=2
            //but they have ttx (Rating_Ter_OK.trp) by cloud
            if((TTX_PAGE_MIN_NUM==page) && (0 ==rec_monit_id_init_page_num[dmx_id][prog_index]))
            {
               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[0]=desc[0];

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[1]=desc[1];

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].lang[2]=desc[2];


               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].pid = pid;

               rec_monit_id_init_page[dmx_id][prog_index]
               [rec_monit_id_init_page_num[dmx_id][prog_index]].page = page;

               rec_monit_id_init_page_num[dmx_id][prog_index]++;

               if((1 == rec_monit_id_subt_page_num[dmx_id][prog_index]) &&
                (TTX_PAGE_MIN_NUM == rec_monit_id_subt_page[dmx_id][prog_index][0].page))
               {
                  //it is a wrong header, cloud
                  rec_monit_id_subt_page_num[dmx_id][prog_index] = 0;
               }
            }
        }
        desc +=5;
        desc_length -= 5;
    }
    return SI_SUCCESS;
}
#endif
/*
TTXEng_GetSubtLang:
TTXEng_SetSubtLang:
    used in pairs
*/
UINT32 ttxeng_get_subt_lang(struct t_ttx_lang **list ,UINT8 *num)
{
    if((NULL == list)||(NULL == num))
    {
        return 0xFF;//invalide value
    }
    *num = g_subt_page_num;
    *list = g_subt_page;
    return g_ttx_desc_get;
}
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
void rec_monitor_ttxeng_get_subt_lang(UINT16 dmx_id,UINT8 prog_index,struct t_ttx_lang** list ,UINT8* num)
{
    *num = rec_monit_id_subt_page_num[dmx_id][prog_index];
    *list = rec_monit_id_subt_page[dmx_id][prog_index];
}
#endif
UINT8 ttxeng_get_cur_subt_lang(void)
{
    return g_current_subt_page;
}
INT32 ttxeng_set_subt_lang(UINT8 lang)
{
    struct register_service ttx_serv ;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE func_ret = ERR_FAILURE;

    MEMSET(&ttx_serv,0x0,sizeof(struct register_service));
    if(lang>=g_subt_page_num)
    {
        return func_ret;
    }

    g_current_subt_page = lang;
    //ttx_stop(ttx_dev);
    ttxeng_close();
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    if(g_subt_page[lang].pid != g_current_pid)
    {
        if (ttx_event_callback != NULL)
        {
           ttx_event_callback(TTX_PID_UPDATE, g_subt_page[lang].pid);
        }

        g_current_pid = g_subt_page[lang].pid;
        if(( TRUE== g_ttx_enable) || ( TRUE== g_ttx_pvr_enable))
        {
            func_ret = ttx_unreg_all_dmx();
            ttx_serv.device = ttx_dev;
            ttx_serv.request_write = (request_write)vbi_request_write;
            ttx_serv.update_write = (update_write)vbi_update_write;
            ttx_serv.service_pid = g_current_pid;

            if(RET_SUCCESS == dmx_register_service(dmx_dev,TTX_REGISTER_NUM,&ttx_serv))
            {
                ttx_dmx_used[dmx_index] = 1;
            }
            g_ttx_channel_enable = TRUE;
        }
    }
    if((TRUE == g_ttx_enable) || (TRUE == g_ttx_pvr_enable))
    {

        if(RET_SUCCESS!=vbi_start(ttx_dev,ttxeng_upd_page))
        {
            LIBTTX_PRINTF("vbi_open failed!!\n");
            return ERR_FAILURE;//ASSERT(0);
        }
    }

    g_current_language = g_subt_page[lang].lang;
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_SET_CUR_LANGUAGE, (UINT32)g_current_language);

    func_ret = ttxeng_open(TTXENG_SUBT,g_subt_page[lang].page);

    return func_ret;
}
/*
TTXEng_GetInitLang:
TTXEng_SetInitLang:
    used in pairs
*/
UINT32 ttxeng_get_init_lang(struct t_ttx_lang **list ,UINT8 *num)
{
    if((NULL == list)||(NULL == num))
    {
        return 0xFF;
    }
    *num = g_init_page_num;
    *list = g_init_page;
    return g_ttx_desc_get;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
void rec_monitor_ttxeng_get_init_lang(UINT16 dmx_id,UINT8 index_id, struct t_ttx_lang** list ,UINT8* num)
{
    *num = rec_monit_id_init_page_num[dmx_id][index_id];
    *list = rec_monit_id_init_page[dmx_id][index_id];
}
#endif


#ifdef TTX_BY_OSD

INT32 ttxeng_set_init_lang(UINT8 lang)
{
    struct register_service ttx_serv ;
    struct dmx_device *dmx_dev = NULL;
    UINT8 ttx_status = 0;
   //UINT8 seperate_ttxsubt_mode = 0;

    RET_CODE func_ret = ERR_FAILUE;

    ttx_status = TTXENG_TEXT;
    MEMSET(&ttx_serv,0,sizeof(struct register_service));
    if((lang>=g_init_page_num) || (lang >= TTX_SUBT_LANG_NUM))
    {
        //libc_printf("lang>=g_init_pg_num,lang=%d,g_init_pg_num=%d,in TTXEng_SetInitLang\n", lang, g_init_pg_num);
        //TTXEng_Close();
        return func_ret;
    }

    g_current_init_page = lang;

    //ttx_stop(ttx_dev);
    ttxeng_close();
    if(g_init_page[lang].pid != g_current_pid)
    {
        if (ttx_event_callback != NULL)
        {
            ttx_event_callback(TTX_PID_UPDATE, g_init_page[lang].pid);
        }

        g_current_pid = g_init_page[lang].pid;

        if(( TRUE== g_ttx_enable) || ( TRUE== g_ttx_pvr_enable))
        {
            func_ret=ttx_unreg_all_dmx();
            ttx_serv.device = ttx_dev;
            ttx_serv.request_write = (request_write)vbi_request_write;
            ttx_serv.update_write = (update_write)vbi_update_write;
            ttx_serv.service_pid = g_current_pid;

            dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
            if(RET_SUCCESS == dmx_register_service(dmx_dev,TTX_REGISTER_NUM,&ttx_serv))
            {
                ttx_dmx_used[dmx_index] = 1;
            }
            g_ttx_channel_enable = TRUE;
        }
    }
    if(( TRUE== g_ttx_enable) || (TRUE == g_ttx_pvr_enable))
    {
        if(RET_SUCCESS!=vbi_start(ttx_dev,ttxeng_upd_page))
        {
            LIBTTX_PRINTF("vbi_open failed!!\n");
            return ERR_FAILURE;//ASSERT(0);
        }
    }

    g_current_language = g_init_page[lang].lang;
    func_ret=vbi_ioctl(ttx_dev,IO_VBI_ENGINE_SET_CUR_LANGUAGE, (UINT32)g_current_language);

    g_init_page_id = get_inital_page();
    if(g_init_page_id!=0xFF)
    {
       // g_init_page_id = get_inital_page();//cloud
        ttx_status = get_inital_page_status();
    }
    else
    {
        g_init_page_id = 100;
    }
    //libc_printf("g_init_page_enable=%d,g_init_page_id=%d,
    //  g_init_page[lang].page=%d\n",g_init_page_enable,g_init_page_id,
    //g_init_page[lang].page);
    if(TRUE == g_init_page_enable)
    {
        //ttx_check_seperate_ttxsubt_mode(&seperate_ttxsubt_mode);

        //if((1 == ttx_status) && (1 == seperate_ttxsubt_mode))//(1 == seperate_ttxsubt_mode) false
        //{
            //cloud, for seperate ttx subtitle mode
        //    func_ret=ttxeng_open(TTXENG_TEXT,get_first_ttx_page());
        //}
       // else
       // {
            func_ret=ttxeng_open(ttx_status,g_init_page_id);
       // }
    }
    else
    {
        func_ret = ttxeng_open(ttx_status,g_init_page[lang].page);
    }
    return func_ret;

}
#endif

void ttx_set_initpage(BOOL enable, UINT16 page_id)
{
    g_init_page_enable = enable;
    if((page_id<TTX_PAGE_MIN_NUM) || (page_id>TTX_PAGE_MAX_NUM))
    {
        page_id = 100;
    }
    if(TRUE == enable)
    {
        g_init_page_id = page_id;
    }
    else
    {
        g_init_page_id = 100;
    }
}
void ttx_enable(BOOL enable)
{
    struct register_service ttx_serv ;
    struct dmx_device *dmx_dev = NULL;
    UINT8 ttx_eng_state = TTXENG_OFF;
    RET_CODE func_ret = ERR_FAILURE;

    LIBTTX_PRINTF("%s: enable = %d\n", __FUNCTION__, enable);
    MEMSET(&ttx_serv,0x0,sizeof(struct register_service));

    if(INVALID_ID == TTX_SEM_ID)
    {
        TTX_SEM_ID = osal_semaphore_create(1);
        if(INVALID_ID == TTX_SEM_ID)
        {
            PRINTF("INVALID_ID == TTX_SEM_ID!\n");
            return;// ASSERT(0);
        }
    }

    if(NULL == ttx_dev)
    {
        ttx_dev = (struct vbi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_VBI);
        if(NULL == ttx_dev)
        {
            PRINTF("ttx_dev NOT exist!!\n");
            return;//ASSERT(0);
        }
    }

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    if(enable)
    {
        func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_GET_STATE,(UINT32)(&ttx_eng_state));
		
		if(func_ret==RET_SUCCESS)
		{
            if(((TTXENG_TEXT == ttx_eng_state)&&(g_init_page_num>0)&&(0x1fff != g_current_pid))
            ||((TTXENG_SUBT == ttx_eng_state)&&(g_subt_page_num>0) &&(0x1fff != g_current_pid)))
            {
                func_ret = ttx_unreg_all_dmx();
                ttx_serv.device = ttx_dev;
                ttx_serv.request_write = (request_write)vbi_request_write;
            ttx_serv.update_write = (update_write)vbi_update_write;
                ttx_serv.service_pid = g_current_pid;
                if(RET_SUCCESS == dmx_register_service(dmx_dev,TTX_REGISTER_NUM,&ttx_serv))
                {
                    ttx_dmx_used[dmx_index] = 1;
                }
                g_ttx_channel_enable = TRUE;
            }
		}

        if(RET_SUCCESS!=vbi_start(ttx_dev,ttxeng_upd_page))
        {
            LIBTTX_PRINTF("vbi_open failed!!\n");
            return;//ASSERT(0);
        }
        g_ttx_enable = TRUE;
    }
    else
    {
        func_ret = ttx_unreg_all_dmx();
        if(RET_SUCCESS!=vbi_stop(ttx_dev))
        {
            LIBTTX_PRINTF("vbi_close failed!!\n");
            return;//ASSERT(0);
        }
        g_current_pid = 0X1FFF;
        g_ttx_enable = FALSE;
    }
}


void ttxpvr_set_subt_lang(struct t_ttx_lang *list ,UINT8 num)
{
    struct ttx_page_info ttx_page_update;
    UINT8 i = 0;
    UINT32 n_size = 0;
    RET_CODE func_ret = ERR_FAILURE;

    if((NULL == list)||(0xFF == num))
    {
        return;
    }
    n_size = sizeof(struct t_ttx_lang);
    MEMSET(&ttx_page_update,0,sizeof(struct ttx_page_info));
    g_subt_page_num = num;

    for(i=0; i<num; i++)
    {
        MEMCPY(&(g_subt_page[i]),&(list[i]),n_size);
    }
    ttx_page_update.num = g_subt_page_num;
    ttx_page_update.page_addr = (UINT32)g_subt_page;
    func_ret = vbi_ioctl(ttx_dev,IO_VBI_ENGINE_UPDATE_SUBT_PAGE,(UINT32)&ttx_page_update);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
}

void ttxpvr_set_init_lang(struct t_ttx_lang *list ,UINT8 num)
{
    struct ttx_page_info ttx_page_update;
    UINT8 i = 0;
    UINT32 n_size = 0;
    RET_CODE func_ret =  ERR_FAILURE;

    if((NULL == list)||(0xFF == num))
    {
        return;
    }
    n_size = sizeof(struct t_ttx_lang);
    MEMSET(&ttx_page_update,0,sizeof(struct ttx_page_info));
    g_init_page_num = num;

    for(i=0; i<num; i++)
    {
        MEMCPY(&(g_init_page[i]),&(list[i]),n_size);
    }
    ttx_page_update.num = g_init_page_num;
    ttx_page_update.page_addr = (UINT32)g_init_page;
    func_ret =  vbi_ioctl(ttx_dev,IO_VBI_ENGINE_UPDATE_INIT_PAGE,(UINT32)&ttx_page_update);
	func_ret=(RET_SUCCESS==func_ret)?RET_SUCCESS:!RET_SUCCESS;
}


void ttx_pvr_enable(BOOL enable,UINT16 dmx_id)
{
    LIBTTX_PRINTF("%s: enable = %d\n", __FUNCTION__, enable);
    
    struct register_service ttx_serv ;
    struct dmx_device *dmx_dev = NULL;
    UINT8 ttx_eng_state = TTXENG_TEXT;
    RET_CODE func_ret =  ERR_FAILURE;
    if(0xFF == dmx_id)
    {
        return;
    }
    MEMSET(&ttx_serv,0,sizeof(struct register_service));
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    g_subt_page_num = 0;
    g_init_page_num = 0;

    if(enable)
    {
        dmx_index = dmx_id;
        func_ret =vbi_ioctl(ttx_dev,IO_VBI_ENGINE_GET_STATE,(UINT32)(&ttx_eng_state));
		
		if(func_ret==RET_SUCCESS)
		{
            if(((TTXENG_TEXT == ttx_eng_state)&&(g_init_page_num>0)&&(0x1fff != g_current_pid))
            ||((TTXENG_SUBT == ttx_eng_state)&&(g_subt_page_num>0) &&(0x1fff != g_current_pid)))
            {
                func_ret = ttx_unreg_all_dmx();
                ttx_serv.device = ttx_dev;
                ttx_serv.request_write = (request_write)vbi_request_write;
                ttx_serv.update_write = (update_write)vbi_update_write;
                ttx_serv.service_pid = g_current_pid;
                if(RET_SUCCESS == dmx_register_service(dmx_dev,TTX_REGISTER_NUM,&ttx_serv))
                {
                    ttx_dmx_used[dmx_index] = 1;
                }
                g_ttx_channel_enable = TRUE;
            }
		}
        if(RET_SUCCESS!=vbi_start(ttx_dev,ttxeng_upd_page))
        {
            LIBTTX_PRINTF("vbi_open failed!!\n");
            return;//ASSERT(0);
        }
        g_ttx_pvr_enable = TRUE;
    }
    else
    {
        if(g_ttx_pvr_enable)
        {
            dmx_index = 0;
        }
        func_ret = dmx_unregister_service(dmx_dev,3);
        if(RET_SUCCESS!=vbi_stop(ttx_dev))
        {
            LIBTTX_PRINTF("vbi_close failed!!\n");
            return;//ASSERT(0);
        }
        g_current_pid = 0X1FFF;
        g_ttx_pvr_enable = FALSE;
#ifdef LIB_TSI3_FULL
        MEMSET(prev_ttext, 0x0, sizeof(prev_ttext));
        ttx_descriptor_reset();
#endif
    }
}

BOOL ttx_check_enable(void)
{
    return g_ttx_enable;
}

void ttx_reg_callback(TTX_EVENT_CALLBACK callback)
{
    if(NULL == callback)
    {
        return;
    }
    osal_task_dispatch_off();
    ttx_event_callback = callback;
    osal_task_dispatch_on();
}

void ttx_unreg_callback(void)
{
    osal_task_dispatch_off();
    ttx_event_callback = NULL;
    osal_task_dispatch_on();
}
#endif

