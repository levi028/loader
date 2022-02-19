/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_core.c
*
*    Description: main function of SI monitor
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/list.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/psi_db.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsi/si_sdtt.h>
#include "sie_monitor_core.h"
#include "sie_monitor_process_table.h"
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)//PVR_DYNAMIC_PID_MONITOR_SUPPORT
#include <api/libsubt/lib_subt.h>
#include <api/libttx/lib_ttx.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif
#if(defined(_MHEG5_V20_ENABLE_))
#include <ssf/ssf_api.h>
#include <mh5_api/mh5_api.h>
#endif

#define SIM_DEBUG_LEVEL     0
#if (SIM_DEBUG_LEVEL>0)
#define SIM_PRINTF          libc_printf
#else
#define SIM_PRINTF(...)     do{}while(0)
#endif

#define SHORT_BUF_CNT 20 //10   //buffer count for filter  Fix BUG51511
#define MAX_SI_CALLBACK_CNT         5   // CI + CA + ...

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
#define DMX_NUM 2
#define EACH_DMX_MONITOR_NUM 2
#endif

static INT32 monitor_pat_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 pmt_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 sdtt_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 ttx_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv);
static INT32 sub_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv);
static INT32 stream_iden_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv);
static INT32 isdbt_data_comp_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv);

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
INT32 stream_iden_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv);
INT32 isdbt_data_comp_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv);
#endif

#if(defined(_MHEG5_V20_ENABLE_))
static void mheg_receive_pat_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 mheg_receive_pmt_callback(UINT8 *section, INT32 length, UINT32 param);
#endif
static section_parse_cb_t si_pmt_callback[MAX_SI_CALLBACK_CNT] = {NULL,NULL,NULL,NULL,NULL};
#ifdef GET_CAT_BY_SIM
static section_parse_cb_t si_cat_callback[MAX_SI_CALLBACK_CNT] = {NULL,NULL,NULL,NULL,NULL};
#endif

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static struct desc_table rec_es_desc_info[] = {
    {  TELTEXT_DESCRIPTOR,  0,  ttx_handler_rec  },
    {  SUBTITLE_DESCRIPTOR, 0,  sub_handler_rec  }
};
#endif


/* Add switch:NEW_DEMO_FRAME
 * Reason: In project Sabbat dual, monitor id manage by APP Layer
 * and ttx, subt callback register by APP according to monitor id, when
 * PMT receeved, subt/ttxt callback will call by run_callback. so here
 * the PMT data path to ttx/subt (ttx_handler/sub_handler) should be blocked,
 * otherwise there two data paths to ttx/subt, and cause some problems.
 * In Glass project, monitor id manage by Middle Layer, APP not use so as
 * ttx_register, subt_register, so here have to use ttx_handler/sub_handler
 */
 

sim_notify_callback sim_ncb[DMX_COUNT][TABLE_COUNT][MAX_SIM_NCB_CNT];
sim_section_callback sim_scb[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];
UINT32 sec_cb_priv[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];

static UINT32 sb_used[SHORT_BUF_CNT] = {0};
#ifdef SUPPORT_FRANCE_HD
static UINT8 short_buff[SHORT_BUF_CNT][PSI_SHORT_SECTION_LENGTH*8] = {{0},{0}};
#else
static UINT8 short_buff[SHORT_BUF_CNT][PSI_SHORT_SECTION_LENGTH] = {{0},{0}};
#endif
static UINT32 last_i = 1;
static on_pid_change_t on_change = NULL;
static on_pid_change_t pre_change = NULL;
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static on_rec_pid_change_t rec_on_change = NULL;
#endif
static P_NODE program;
static UINT16 m_cur_chan_index = 0xffff;
static BOOL hold_ttx_info = FALSE;
//static UINT32 moni_prog=0;

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static UINT16 ecm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static UINT16 emm_pids[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif

#if(defined(TTX_ON)&&(TTX_ON==1))
static UINT8 prev_ttx[256] = {0};
#endif
#if( SUBTITLE_ON == 1)
static UINT8 prev_sub[256] = {0};
#endif

static UINT8 ttx_reset_flag = 0;
static UINT8 subt_reset_flag = 0;

#ifdef SUPPORT_CAS7
static cas7_section_parse_cb_t cas7_section_parse_cb[3] = {NULL,NULL,NULL};
static UINT32 cas7_section_parse_cb_priv[3]= {0,0,0};
#endif

static UINT32 sdtt_monitor_id = SIM_INVALID_MON_ID;
struct sim_cb_param simcb_array[MAX_SIM_COUNT];
struct restrict mask_array[MAX_SIM_COUNT];
struct DMX_PSI_INFO dmx_psi_info[DMX_COUNT];
OSAL_ID sm_semaphore = OSAL_INVALID_ID;

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT8 g_pid_change_flag[DMX_NUM][EACH_DMX_MONITOR_NUM]={0};
#endif

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT32  rec_monitor_id[DMX_NUM][EACH_DMX_MONITOR_NUM] =
{
    {SIM_INVALID_MON_ID,SIM_INVALID_MON_ID},
    {SIM_INVALID_MON_ID,SIM_INVALID_MON_ID}
};
UINT32  rec_monitor_id_prog[DMX_NUM][EACH_DMX_MONITOR_NUM]=
{
    {SIM_INVALID_MON_ID,SIM_INVALID_MON_ID},
    {SIM_INVALID_MON_ID,SIM_INVALID_MON_ID}
};
P_NODE rec_monitor_id_prog_node[DMX_NUM][EACH_DMX_MONITOR_NUM]={{0,0},{0,0}};
struct sim_cb_param g_monitor_pmt_sec[DMX_NUM][EACH_DMX_MONITOR_NUM];
#endif

UINT8 monitor_exist[MAX_SIM_COUNT] = {0};
UINT32 cat_monitor_id = SIM_INVALID_MON_ID;
UINT32 pat_monitor_id = SIM_INVALID_MON_ID;
UINT32 monitor_id = SIM_INVALID_MON_ID;
UINT16 monitor_dmx_id = 0;

#ifdef NO_TTX_DESCRIPTOR
UINT8 ttxflag = FALSE;
#endif

INT32 sim_get_monitor_param(UINT32 monitor_id, struct sim_cb_param *param)
{
    if((NULL == param) || !monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    MEMCPY(param, &(simcb_array[monitor_id]), sizeof(struct sim_cb_param));

    return SUCCESS;
}

static UINT8 get_free_buffer(UINT32 monitor_id)
{
    UINT8 i = 0;

    if(!monitor_id_valid(monitor_id))
    {
        return 0xFF;
    }

    for(i=0; i<SHORT_BUF_CNT; i++)
    {
        if(sb_used[i] == monitor_id)
        {
            return 0xFF;
        }
    }

    for(i = 0; i<SHORT_BUF_CNT;i++)
    {
        if(SIM_INVALID_MON_ID == sb_used[i])
        {
            sb_used[i]= monitor_id;
            return i;
        }
    }
    return 0xFF;
}

static UINT8 release_buffer(UINT32 monitor_id)
{
    UINT8 i = 0;

    for(i=0; i<SHORT_BUF_CNT; i++)
    {
        if(sb_used[i]==monitor_id)
        {
            sb_used[i] = SIM_INVALID_MON_ID;
            return i;
        }
    }
    return 0xFF;
}

static UINT32 get_free_monitor_id(void)
{
    UINT32 i = 0;
    UINT32 free_id = SIM_INVALID_MON_ID;

    if (MAX_SIM_COUNT == last_i)
    {
        last_i = 1;
    }

    for(i = last_i; i<MAX_SIM_COUNT; i++)
    {
        if((0 == monitor_exist[i]) && (0 != i))
        {
            monitor_exist[i]= 1;
            free_id = i;
            last_i = i+1;
            return free_id;
        }
    }
    return SIM_INVALID_MON_ID;
}

static void release_monitor_id(UINT32 monitor_id)
{
    if( monitor_id_valid(monitor_id) )
    {
        monitor_exist[monitor_id] = 0;
    }
}

#ifdef SUPPORT_CAS7
void cas7_pmt_run_callback(UINT8 dmx_id,UINT16 pid,UINT8 *section, INT32 length)
{
    UINT8 max_dmx_cnt = 2;
    UINT16 param = 0;

    if((dmx_id > max_dmx_cnt) || (NULL == section) || (0 == length))
    {
        return;
    }
    param = cas7_section_parse_cb_priv[dmx_id];
    if(cas7_section_parse_cb[dmx_id] != NULL)
    {
        cas7_section_parse_cb[dmx_id](dmx_id,pid,section,length,param);
    }
}
UINT8 cas7_regist_pmt_callback(UINT8 dmx_id, cas7_section_parse_cb_t callback, UINT32 priv)
{
    UINT8 max_dmx_cnt = 2;

    if((NULL == callback) || (dmx_id > max_dmx_cnt))
    {
        return RET_FAILURE;
    }

    cas7_section_parse_cb[dmx_id] = callback;
    cas7_section_parse_cb_priv[dmx_id] = priv;

    return RET_SUCCESS;
}
UINT8 cas7_unregist_pmt_callback(UINT8 dmx_id, cas7_section_parse_cb_t callback)
{
    UINT8 max_dmx_cnt = 2;

    if((callback == NULL) || (dmx_id > max_dmx_cnt))
    {
        return RET_FAILURE;
    }

    if(cas7_section_parse_cb[dmx_id] == callback)
    {
        cas7_section_parse_cb[dmx_id]  = NULL;
        cas7_section_parse_cb_priv[dmx_id] = 0;

        return RET_SUCCESS;
    }

    return RET_FAILURE;
}
#endif

static UINT32 __si_start_monitor(struct dmx_device *dmx,UINT16 pid,UINT16 table_id,enum MONITE_TB table,UINT32 param)
{
    INT32 ret = SUCCESS;
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;
    UINT16 dmx_index = 0;
    UINT8 buf_idx = 0;
//    UINT8 mon_index = 0;
    UINT32 monitor_id = SIM_INVALID_MON_ID;

    if(NULL == dmx)
    {
        return SIM_INVALID_MON_ID;
    }
    MEMSET(&fparam, 0, sizeof(struct si_filter_param));
    dmx_index = get_dmx_index(dmx);

    if(dmx_index >= DMX_COUNT)
    {
        return SIM_INVALID_MON_ID;
    }

    set_monitor_param(table_id, param, &fparam);

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    monitor_id = get_free_monitor_id();
    if(SIM_INVALID_MON_ID == monitor_id)
    {
        osal_semaphore_release(sm_semaphore);
        return SIM_INVALID_MON_ID;
    }

    buf_idx = get_free_buffer(monitor_id);

    if(0xFF == buf_idx)
    {
        release_monitor_id(monitor_id);
        osal_semaphore_release(sm_semaphore);
        return SIM_INVALID_MON_ID;
    }

    osal_semaphore_release(sm_semaphore);
#ifdef SUPPORT_FRANCE_HD
    filter = sie_alloc_filter_ext(dmx, pid, short_buff[buf_idx], 8*1024,1024,0);
#else
    filter = sie_alloc_filter_ext(dmx, pid, short_buff[buf_idx], 1024,1024,0);
#endif

    if(NULL == filter)
    {
        osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
        release_monitor_id(monitor_id);
        release_buffer(monitor_id);     
        osal_semaphore_release(sm_semaphore);
        return SIM_INVALID_MON_ID;
    }

    sie_config_filter(filter,&fparam);
    ret = sie_enable_filter(filter);

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    if(SI_SUCCESS != ret)
    {
        release_monitor_id(monitor_id);
        release_buffer(monitor_id);
        osal_semaphore_release(sm_semaphore);
        return SIM_INVALID_MON_ID;
    }

    //backup restrict
    MEMCPY(&(mask_array[monitor_id]), &(fparam.mask_value), sizeof(struct restrict));
    simcb_array[monitor_id].sim_id = monitor_id;
    simcb_array[monitor_id].dmx_idx = dmx_index;
    simcb_array[monitor_id].table = table;
    simcb_array[monitor_id].sec_pid = pid;
    simcb_array[monitor_id].param = param;
    simcb_array[monitor_id].priv = NULL;

    set_table_bit_valid(dmx, table_id, dmx_index, pid, param);

    osal_semaphore_release(sm_semaphore);
    return monitor_id;
}

void sim_open_monitor(UINT32 praram)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;

    SIM_PRINTF("Init monitor\n");

    sm_semaphore = osal_semaphore_create(1);

    for(i=0; i<DMX_COUNT; i++)
    {
        for(j=0; j<TABLE_COUNT; j++)
        {
            for(k=0; k<MAX_SIM_NCB_CNT; k++)
            {
                sim_ncb[i][j][k] = NULL;
            }
            for(k=0; k<MAX_SIM_SCB_CNT; k++)
            {
                sim_scb[i][j][k] = NULL;
                sec_cb_priv[i][j][k] = 0;
            }
        }
    }
    for(i=0; i<SHORT_BUF_CNT; i++)
    {
        sb_used[i] = SIM_INVALID_MON_ID;
    }
    for(i=0; i<MAX_SIM_COUNT;i++)
    {
        monitor_exist[i] = 0;
    }

    MEMSET(dmx_psi_info, 0 , sizeof(struct DMX_PSI_INFO)*DMX_COUNT);
    return;
}

void sim_close_monitor(UINT32 praram)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;

    SIM_PRINTF("close monitor\n");

    osal_semaphore_delete(sm_semaphore);
    sm_semaphore = OSAL_INVALID_ID;

    for(i=0; i<DMX_COUNT; i++)
    {
        for(j=0; j<TABLE_COUNT; j++)
        {
            for(k=0; k<MAX_SIM_NCB_CNT; k++)
            {
                sim_ncb[i][j][k] = NULL;
            }
            for(k=0; k<MAX_SIM_SCB_CNT; k++)
            {
                sim_scb[i][j][k] = NULL;
                sec_cb_priv[i][j][k] = 0;
            }
        }
    }

    for(i=0; i<SHORT_BUF_CNT; i++)
    {
        sb_used[i] = SIM_INVALID_MON_ID;
    }

    for(i=0; i<MAX_SIM_COUNT;i++)
    {
        monitor_exist[i] = 0;
    }

    MEMSET(dmx_psi_info, 0 , sizeof(struct DMX_PSI_INFO)*DMX_COUNT);
    return;
}

UINT32 sim_start_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 sim_pid,UINT32 param)
{
    UINT16 invalid_pid = 8191;
    UINT16 pid = 0x1FFF;
    UINT16 table_id = 0;
    UINT32 monitor_id = SIM_INVALID_MON_ID;

    SIM_PRINTF("sim start monitor\n");

    if(NULL == dmx || (sim_pid >= invalid_pid))
    {
        return SIM_INVALID_MON_ID;
    }

    //check pmt pid
    if((MONITE_TB_PMT == table) && ((PSI_PAT_PID == sim_pid) || (0x01 == sim_pid) || (PSI_SDT_PID == sim_pid)))
    {
        return SIM_INVALID_MON_ID;
    }

    monitor_id = check_monitor(dmx, table, sim_pid, param);
    if(SIM_INVALID_MON_ID != monitor_id)
    {
        #ifdef PVR_DMX_DELAY_SUPPORT_USE_DMX2
            sim_stop_monitor(monitor_id);
        #else
            return monitor_id;
        #endif
    }

    if(PSI_INVALID_ID == pid_tid_parser[table].pid)
    {
        pid = sim_pid;
    }
    else
    {
        pid = pid_tid_parser[table].pid;
    }

    table_id = pid_tid_parser[table].tid;
    monitor_id = __si_start_monitor(dmx, pid, table_id,table,param);

#if defined(CAS9_PVR_SCRAMBLED)
    if(MONITE_TB_PMT == table)   //record CA original stream,when playback need to parse pmt/cat use dmx2,by Terry.Wu
#elif defined(SUPPORT_CAS_A)
    if((MONITE_TB_PMT == table) && (STRCMP(dmx->name, "DMX_S3601_2") != 0) && (STRCMP(dmx->name, "DMX_S3601_1") != 0))
#else
    if((MONITE_TB_PMT == table) && (STRCMP(dmx->name, "DMX_S3601_2") != 0)) //not allow parse ca pmt when playback
#endif
    {
        sim_register_scb(monitor_id, pmt_callback, NULL);//add for ca
    }

    return monitor_id;
}

INT32 sim_stop_monitor(UINT32 monitor_id)
{
    UINT16 dmx_index = -1;
    UINT8 tbl_index = -1;
    int i = 0;
    UINT8 *buf = NULL;
    struct dmx_device *dmx = NULL;
    enum MONITE_TB table = NONE;
    UINT16 pid = 0x1FFF;

    if((!monitor_id_valid(monitor_id))||(monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_index = simcb_array[monitor_id].dmx_idx; //(monitor_id&DMX_INDEX_MASK)>>24;
    table = simcb_array[monitor_id].table;       //(monitor_id&TABLE_MASK)>>16;
    pid = simcb_array[monitor_id].sec_pid;       //(monitor_id&SEC_PID_MASK);
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);

    SIM_PRINTF("SIM STOP monitor\n");

    tbl_index = get_table_index(dmx_index, table, simcb_array[monitor_id].param);
    if(0xFF == tbl_index)
    {
        return ERR_FAILUE;
    }

    if(sie_abort_ext(dmx, &buf, pid, &(mask_array[monitor_id])) != SI_SUCCESS)
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    release_buffer(monitor_id);
    release_monitor_id(monitor_id);

    for(i=0; i < MAX_SIM_NCB_CNT; i++)
    {
        sim_ncb[dmx_index][tbl_index][i] = NULL;
    }

#ifdef SUPPORT_FRANCE_HD
    if (MONITE_TB_SDT_OTHER == table)
    {
        for(i=0; i < MAX_SIM_SCB_CNT; i++)
        {
            sim_scb[dmx_index][tbl_index][i] = NULL;
        }
    }
#endif

    set_table_bit_invalid(table, dmx_index, pid);
    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_register_ncb(UINT32 monitor_id, sim_notify_callback callback)
{
    INT32 i = 0;
    INT32 n = -1;
    enum MONITE_TB table = NONE;
    UINT16 dmx_idx = -1;
    UINT8 tbl_idx = -1;

    SIM_PRINTF("sim_register_ncb\n");

    if(!monitor_id_valid(monitor_id) || (1 != monitor_exist[monitor_id]))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    table = simcb_array[monitor_id].table;
    tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
    if ((0xFF == tbl_idx) || (NULL == callback))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    for (i = 0; i<MAX_SIM_NCB_CNT; i++)
    {
        if (sim_ncb[dmx_idx][tbl_idx][i] == callback)
        {
            osal_semaphore_release(sm_semaphore);
            return SUCCESS;
        }
        else if ((NULL == sim_ncb[dmx_idx][tbl_idx][i]) && (-1 == n))
        {
            n = i;
        }
    }

    if ((n >= 0) && (n < MAX_SIM_NCB_CNT))
    {
        sim_ncb[dmx_idx][tbl_idx][n] = callback;
        osal_semaphore_release(sm_semaphore);
        return SUCCESS;
    }

    osal_semaphore_release(sm_semaphore);
    return ERR_FAILUE;
}

INT32 sim_unregister_ncb(UINT32 monitor_id, sim_notify_callback callback)
{
    INT32 i = 0;
    enum MONITE_TB table = NONE;
    UINT16 dmx_idx = -1;
    UINT8 tbl_idx = -1;

    SIM_PRINTF("sim_unregister_ncb\n");

    if(!monitor_id_valid(monitor_id) || (1 != monitor_exist[monitor_id]))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    table = simcb_array[monitor_id].table;
    tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
    if((0xFF == tbl_idx) || (NULL == callback))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    for (i=0; i<MAX_SIM_NCB_CNT; i++)
    {
        if (sim_ncb[dmx_idx][tbl_idx][i] == callback)
        {
            sim_ncb[dmx_idx][tbl_idx][i] = NULL;
            osal_semaphore_release(sm_semaphore);
            return SUCCESS;
        }
    }

    osal_semaphore_release(sm_semaphore);
    return ERR_FAILUE;

}

INT32 sim_register_scb(UINT32 monitor_id, sim_section_callback callback, void *priv)
{
    INT32 i = 0;
    INT32 n = -1;
    enum MONITE_TB table = NONE;
    UINT16 dmx_idx = -1;
    UINT8 tbl_idx = -1;

    SIM_PRINTF("sim_register_scb\n");

    if(!monitor_id_valid(monitor_id) || (1 != monitor_exist[monitor_id]))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    table = simcb_array[monitor_id].table;

    tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
    if((0xFF == tbl_idx) || (NULL == callback))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    for (i=0; i<MAX_SIM_SCB_CNT; i++)
    {
        if (sim_scb[dmx_idx][tbl_idx][i] == callback)
        {
            osal_semaphore_release(sm_semaphore);
            return SUCCESS;
        }
        else if ((NULL == sim_scb[dmx_idx][tbl_idx][i]) && (-1 == n))
        {
            n = i;
        }
    }

    if ((n >= 0) && (n < MAX_SIM_SCB_CNT))
    {
        sim_scb[dmx_idx][tbl_idx][n] = callback;
        sec_cb_priv[dmx_idx][tbl_idx][n] = (UINT32)priv;
        osal_semaphore_release(sm_semaphore);
        return SUCCESS;
    }

    osal_semaphore_release(sm_semaphore);
    return ERR_FAILUE;
}

INT32 sim_unregister_scb(UINT32 monitor_id, sim_section_callback callback)
{
    INT32 i = 0;
    enum MONITE_TB table = NONE;
    UINT16 dmx_idx = -1;
    UINT8 tbl_idx = -1;

    SIM_PRINTF("sim_unregister_scb\n");

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    table = simcb_array[monitor_id].table;
    tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);

    if((0xFF == tbl_idx) || (NULL == callback))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    for (i=0; i<MAX_SIM_SCB_CNT; i++)
    {
        if (sim_scb[dmx_idx][tbl_idx][i] == callback)
        {
            sim_scb[dmx_idx][tbl_idx][i] = NULL;
            sec_cb_priv[dmx_idx][tbl_idx][i] = 0;
            osal_semaphore_release(sm_semaphore);
            return SUCCESS;
        }
    }

    osal_semaphore_release(sm_semaphore);
    return ERR_FAILUE;
}

INT32 sim_unregister_scb_ext(UINT32 monitor_id, sim_section_callback callback, void *priv)
{
    INT32 i = 0;
    enum MONITE_TB table = NONE;
    UINT16 dmx_idx = -1;
    UINT8 tbl_idx = -1;

    SIM_PRINTF("sim_unregister_scb\n");

    if(!monitor_id_valid(monitor_id) )
    {
        return ERR_FAILUE;
    }

    if(1 != monitor_exist[monitor_id])
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    table = simcb_array[monitor_id].table;
    tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);

    if(0xFF == tbl_idx)
    {
        return ERR_FAILUE;
    }

    if (NULL == callback)
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    for (i=0; i<MAX_SIM_SCB_CNT; i++)
    {
        if (sim_scb[dmx_idx][tbl_idx][i] == callback && sec_cb_priv[dmx_idx][tbl_idx][i] == (UINT32)priv)
        {
            sim_scb[dmx_idx][tbl_idx][i] = NULL;
            sec_cb_priv[dmx_idx][tbl_idx][i] = 0;
            osal_semaphore_release(sm_semaphore);
            return SUCCESS;
        }
    }

    osal_semaphore_release(sm_semaphore);
    return ERR_FAILUE;
}

/**********************for old api******************************************/
void si_set_default_dmx(UINT16 id)
{
    monitor_dmx_id = id;
}

static INT32 notify_ap(UINT32 param)
{
    P_NODE node;
    P_NODE back_node;
    struct prog_info info;
    BOOL db_changed = FALSE;
    UINT16 pcr_pid = -1;
    UINT16 video_pid = -1;
    UINT16 audio_pid[P_MAX_AUDIO_NUM] = {0};
    UINT8 audio_count = P_MAX_AUDIO_NUM;
    UINT32 pos = 0;

    MEMSET(&node, 0, sizeof(P_NODE));
    MEMSET(&back_node, 0, sizeof(P_NODE));
    MEMSET(&info, 0, sizeof(struct prog_info));
    MEMCPY(&back_node, &program, sizeof(P_NODE));
    sim_get_pmt(monitor_id, &info);

    if(ERR_FAILUE == sim_get_pcr_pid(monitor_id, &pcr_pid))
    {
        SIM_PRINTF("sim_get_pcr_pid() failed!\n");
    }
    if(ERR_FAILUE == sim_get_video_pid(monitor_id, &video_pid))
    {
        SIM_PRINTF("sim_get_video_pid() failed!\n");
    }
    video_pid &= 0x1fff;
    if(ERR_FAILUE  == sim_get_audio_info(monitor_id, audio_pid, NULL, &audio_count))
    {
        SIM_PRINTF("sim_get_audio_info() failed!\n");
    }

    if((pcr_pid != back_node.pcr_pid) || (video_pid != back_node.video_pid) ||
        (audio_count != back_node.audio_count) ||
        (MEMCMP(back_node.audio_pid, audio_pid, sizeof(UINT16)*audio_count)!=0))
    {
        db_changed = TRUE;
    }

#ifdef STAR_PAT_MONITOR_ENABLE
    db_changed = FALSE; //for STAR M3383 DVBT project, we already have background searching function,
                        //thus SIM no need to monitor if DB is changed or not;
#endif

    if(db_changed)
    {
        if(pre_change)
        {
            pre_change(TRUE);
        }

        psi_monitor_pg2db(&node, &info);
        if(SUCCESS == update_stream_info(&back_node, &node, (INT32 *)&pos))
        {
            SIM_PRINTF("PID Changed, v: %d, pcr: %d, ac: %d, a1: %d\n",video_pid,pcr_pid,audio_count,audio_pid[0]);
            MEMCPY(&program, &node, sizeof(P_NODE));
            if(on_change)
            {
#if(defined(_MHEG5_V20_ENABLE_))
                mheg5_set_spin(SPIN_PMT_CHANGED) ;
#endif
                on_change(TRUE);
            }
        }
    }
    return 0;
}

void ttx_descriptor_reset(void)
{
    ttx_reset_flag = 1;
}

void subt_descriptor_reset(void)
{
    subt_reset_flag = 1;
}

void hold_ttx_enable(BOOL enable)
{
    hold_ttx_info = enable;
}

BOOL ttx_is_holded(void)
{
    return hold_ttx_info;
}

static INT32 ttx_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    if(ttx_is_holded())
    {
        return SI_SUCCESS;
    }

#if(defined(TTX_ON)&&(TTX_ON==1))
    if (ttx_reset_flag || ((NULL != content) && (0 != MEMCMP(content, prev_ttx, length)))||(0xFF == prev_ttx[255]))
    {
        ttx_reset_flag = 0;
        MEMSET(prev_ttx, 0, sizeof(prev_ttx));
        MEMCPY(prev_ttx, content, length);
        ttx_descriptor(tag, length, content, priv);
    }
#ifdef NO_TTX_DESCRIPTOR //cloud
    else if((0 == length) && (NULL != content) && (ttxflag == FALSE)) //for some special stream without ttx desc
    {
        ttx_descriptor(tag, length, content, priv);
        ttxflag = TRUE;
    }
#endif
#endif
    return SI_SUCCESS;
}

static INT32 sub_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
#if(SUBTITLE_ON == 1)
    if (subt_reset_flag || ((NULL != content) && (0 != MEMCMP(content, prev_sub, length)))||(0xFF == prev_sub[255]))
    {
        subt_reset_flag = 0;
        MEMSET(prev_sub, 0, sizeof(prev_sub));
        MEMCPY(prev_sub, content, length);
        subt_descriptor(tag, length, content, priv);
    }
#endif
    return SI_SUCCESS;
}

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static INT32 stream_iden_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    if((STREAM_IDENTIFIER_DESCRIPTOR == tag) && (NULL != content))
    {
        isdbtcc_stream_iden_desc(tag,length,content,priv);
    }
    return SI_SUCCESS;
}
#endif

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static INT32 isdbt_data_comp_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    if(NULL != content)
    {
        isdbtcc_data_comp_desc(tag,length,content,priv);
        return SI_SUCCESS;
    }
    else
    {
        return !SI_SUCCESS;
    }
}
#endif

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT8 get_dynamic_pid_program(UINT16 dmx_idx,UINT32 pg_id,P_NODE *node)
{
    UINT8 result=TRUE;

    if(NULL == node)
    {
        return FALSE;
    }

    if(rec_monitor_id_prog_node[dmx_idx][0].prog_id==pg_id)
    {
        MEMCPY(node,&rec_monitor_id_prog_node[dmx_idx][0],sizeof(P_NODE));
    }
    else if(rec_monitor_id_prog_node[dmx_idx][1].prog_id==pg_id)
    {
        MEMCPY(node,&rec_monitor_id_prog_node[dmx_idx][1],sizeof(P_NODE));
    }
    else
    {
        result=FALSE;
    }

    return result;
}

INT32 ttx_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    UINT8 prog_index=0;
    UINT8 ttx_index=0;
    UINT8 num = 0;
    UINT8 max_prog_index = 2;
    P_NODE p_node;
    struct t_ttx_lang *ttx_list = NULL;
    struct t_ttx_lang *p_ttx = NULL;
    UINT16 pid = 0;
    struct sim_cb_param dynamic_param;

    if((NULL == content) || (NULL == priv))
    {
        return SI_SUCCESS;
    }

    pid = *((UINT16*)priv);
    MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));

    for(prog_index=0; prog_index < max_prog_index; prog_index++)
    {
        if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
        {
            break;
        }
    }

    if(prog_index >= max_prog_index)
    {
        return SI_SUCCESS;
    }

#if(defined(TTX_ON)&&(TTX_ON==1))
    MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
    if (MONITE_TB_PMT == dynamic_param.table)//&&(p_node.teletext_pid!= pid))
    {
        rec_ttx_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);//init rec ttx
        rec_monitor_ttxeng_get_init_lang(dynamic_param.dmx_idx,prog_index,&ttx_list, &num);

        for(ttx_index=0; ttx_index<num; ttx_index++)
        {
            p_ttx = &ttx_list[ttx_index];
            if (p_ttx->pid == pid)
            {
                return SI_SUCCESS;
            }
        }

        rec_monitor_ttxeng_get_subt_lang(dynamic_param.dmx_idx,prog_index,&ttx_list, &num);

        for(ttx_index=0; ttx_index<num; ttx_index++)
        {
            p_ttx = &ttx_list[ttx_index];
            if (p_ttx->pid == pid)
            {
                return SI_SUCCESS;
            }
        }

        p_node.teletext_pid = pid;
        MEMCPY(&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], &p_node, sizeof(P_NODE));
        g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=1;
    }
#ifdef NO_TTX_DESCRIPTOR //cloud
    else if((0 == length) && (content!=NULL) && (ttxflag == FALSE)) //for some special stream without ttx desc
    {
        rec_ttx_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);
        ttxflag = TRUE;
    }
#endif
#endif
    return SI_SUCCESS;
}

INT32 sub_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    UINT8 prog_index=0;
    UINT8 subt_num=0;
    UINT8 max_prog_index = 2;
    UINT16 subt_index=0;
    UINT16 pid = 0;
    P_NODE p_node;
    struct t_subt_lang *subt_list = NULL;
    struct t_subt_lang *p_subt = NULL;
    struct sim_cb_param dynamic_param;

    if((NULL == content) || (NULL == priv))
    {
        return SI_SUCCESS;
    }

    pid = *((UINT16*)priv);
    MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));

    for(prog_index=0 ; prog_index < max_prog_index ;prog_index++)
    {
        if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
        {
            break;
        }
    }

    if(prog_index >= max_prog_index)
    {
       return SI_SUCCESS;
    }

#if(SUBTITLE_ON == 1)
    MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
    rec_subt_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);
    if ((MONITE_TB_PMT == dynamic_param.table))
    {
        rec_monitor_subt_get_language(dynamic_param.dmx_idx,prog_index,&subt_list, &subt_num);

        for(subt_index=0; subt_index<subt_num; subt_index++)
        {
            p_subt = &subt_list[subt_index];
            if (p_subt->pid == pid)
            {
                return SI_SUCCESS;
            }
        }
        p_node.subtitle_pid=pid;
        MEMCPY(&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], &p_node, sizeof(P_NODE));
        g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=1;
    }
#endif

    return SI_SUCCESS;
}

INT32 stream_iden_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    UINT8 max_prog_index = 2;
    UINT8 prog_index=0;
    UINT16 pid = 0;
    struct sim_cb_param dynamic_param;

    if((NULL == content) || (NULL == priv))
    {
        return SI_SUCCESS;
    }

    pid = *((UINT16*)priv);
    MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
    for(prog_index=0 ; prog_index < max_prog_index; prog_index++)
    {
        if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
        {
            break;
        }
    }
    if(prog_index >= max_prog_index)
    {
       return SI_SUCCESS;
    }

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    if(STREAM_IDENTIFIER_DESCRIPTOR==tag)
    {
        rec_isdbtcc_stream_iden_desc(dynamic_param.dmx_idx,prog_index,tag,length,content,priv);
    }
#endif
    return SI_SUCCESS;
}

INT32 isdbt_data_comp_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
    UINT8 prog_index=0;
    UINT8 b_ccnum=0;
    UINT8 b_ccindex = 0;
    UINT8 max_prog_index = 2;
    P_NODE p_node;
    struct t_isdbtcc_lang *cclist = NULL;
    struct t_isdbtcc_lang *pcc = NULL;
    struct sim_cb_param dynamic_param;
    UINT16 pid = 0;

    if((NULL == content) || (NULL == priv))
    {
        return SI_SUCCESS;
    }

    pid = *((UINT16*)priv);
    MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
    for(prog_index=0 ; prog_index < max_prog_index ;prog_index++)
    {
        if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
        {
           break;
        }
    }
    if(prog_index >= max_prog_index)
    {
      return SI_SUCCESS;
    }

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
    rec_isdbtcc_data_comp_desc(dynamic_param.dmx_idx,prog_index,tag,length,content,priv);
    if ((MONITE_TB_PMT == dynamic_param.table))
    {
        rec_monitor_isdbtcc_get_language(dynamic_param.dmx_idx,prog_index,&cclist, &b_ccnum);
        for(b_ccindex = 0; b_ccindex < b_ccnum; b_ccindex++)
        {
            pcc = &cclist[b_ccindex];
            if (pcc->pid == pid)
            {
               return SI_SUCCESS;
            }
        }
        //p_node.subtitle_pid=pid;
        MEMCPY(&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], &p_node, sizeof(P_NODE));
        g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=1;
    }
#endif
    return SI_SUCCESS;
}

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static struct desc_table rec_es_desc_isdbtcc_info[] = {
    { STREAM_IDENTIFIER_DESCRIPTOR,    0,  stream_iden_handler_rec },
    { ISDBT_DATA_COMPONENT_DESCRIPTOR, 0,  isdbt_data_comp_handler_rec }
};
#endif
static void rec_psi_es_info_checkup(UINT32 param,UINT8 *buff, INT32 len, struct desc_table *info, INT32 desc_num)
{
    INT32 prog_info_length = 0;
    INT32 es_info_length = 0;
    INT32 i = 0;
    UINT16 es_pid = 0;
    struct pmt_section *pms = NULL;
    struct pmt_stream_info *stream = NULL;

    if((NULL== buff) || (NULL == info))
    {
        return ;
    }

    pms = (struct pmt_section *)buff;
    prog_info_length = SI_MERGE_HL8(pms->program_info_length);
    for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);
        rec_desc_loop_parser(param,stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
    }
}

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static void rec_psi_es_isdbtcc_info_checkup(UINT32 param,UINT8 *buff, INT32 len,
    struct desc_table *info, INT32 desc_num)
{
    INT32 prog_info_length = 0;
    INT32 es_info_length = 0;
    INT32 i = 0;
    UINT16 es_pid = 0;
    struct pmt_section *pms = NULL;
    struct pmt_stream_info *stream = NULL;

    if((NULL == buff) || (NULL == info))
    {
        return ;
    }

    pms = (struct pmt_section *)buff;
    prog_info_length = SI_MERGE_HL8(pms->program_info_length);
    for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);
        if(PRIVATE_DATA_STREAM==stream->stream_type)
        {
            rec_desc_loop_parser(param,stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
        }
    }
}
#endif
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static struct desc_table es_desc_isdbtcc_info[] = {
    { STREAM_IDENTIFIER_DESCRIPTOR,    0,  stream_iden_handler },
    { ISDBT_DATA_COMPONENT_DESCRIPTOR, 0,  isdbt_data_comp_handler }
};
#endif

static struct desc_table es_desc_info[] = {
#if (ISDBT_CC == 1)
    {  STREAM_IDENTIFIER_DESCRIPTOR, 0, stream_iden_handler },
#endif
/* Add switch:NEW_DEMO_FRAME
 * Reason: In project Sabbat dual, monitor id manage by APP Layer
 * and ttx, subt callback register by APP according to monitor id, when
 * PMT receeved, subt/ttxt callback will call by run_callback. so here
 * the PMT data path to ttx/subt (ttx_handler/sub_handler) should be blocked,
 * otherwise there two data paths to ttx/subt, and cause some problems.
 * In Glass project, monitor id manage by Middle Layer, APP not use so as
 * ttx_register, subt_register, so here have to use ttx_handler/sub_handler
 */
 #ifndef NEW_DEMO_FRAME
    {  TELTEXT_DESCRIPTOR,  0, ttx_handler },
    {  SUBTITLE_DESCRIPTOR, 0, sub_handler },
#endif
};
static void psi_es_info_checkup(UINT8 *buff, INT32 len, struct desc_table *info, INT32 desc_num)
{
    INT32 prog_info_length = 0;
    INT32 es_info_length = 0;
    INT32 i = 0;
    UINT16 es_pid = 0;

    if((NULL == buff) || (NULL == info))
    {
        return ;
    }

    struct pmt_section *pms = (struct pmt_section *)buff;
    struct pmt_stream_info *stream = NULL;

    prog_info_length = SI_MERGE_HL8(pms->program_info_length);
    for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);
        desc_loop_parser(stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
    }
}

#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
static void psi_es_isdbtcc_info_checkup(UINT8 *buff, INT32 len, struct desc_table *info, INT32 desc_num)
{
    INT32 prog_info_length = 0;
    INT32 es_info_length = 0;
    INT32 i = 0;
    UINT16 es_pid = 0;
    struct pmt_section *pms = NULL;
    struct pmt_stream_info *stream = NULL;

    if(NULL == buff)
    {
        return ;
    }

    pms = (struct pmt_section *)buff;
    prog_info_length = SI_MERGE_HL8(pms->program_info_length);
    for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        {
            es_info_length = SI_MERGE_HL8(stream->es_info_length);
            es_pid = SI_MERGE_HL8(stream->elementary_pid);
            if(PRIVATE_DATA_STREAM==stream->stream_type)
            {
                desc_loop_parser(stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
            }
        }
    }
}
#endif

static INT32 monitor_pmt_sec(UINT8 *section, INT32 length, UINT32 param)
{
    psi_es_info_checkup(section, length, es_desc_info, ARRAY_SIZE(es_desc_info));
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    psi_es_isdbtcc_info_checkup(section, length, es_desc_isdbtcc_info, ARRAY_SIZE(es_desc_isdbtcc_info));
#endif
    return 0;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static INT32 rec_monitor_pmt_sec(UINT8 *section, INT32 length, UINT32 param)
{
    UINT8 max_prog_index = 2;
    UINT8 prog_index = 0;
    P_NODE p_node;
    struct sim_cb_param dynamic_param;

    MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));

    for(prog_index=0 ; prog_index < max_prog_index ;prog_index++)
    {
        if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
        {
            break;
        }
    }
    if (max_prog_index == prog_index)
    {
        return 0;
    }

 MEMCPY(&g_monitor_pmt_sec[dynamic_param.dmx_idx][prog_index],(struct sim_cb_param *)param,sizeof(struct sim_cb_param));

    rec_psi_es_info_checkup((UINT32)(&g_monitor_pmt_sec[dynamic_param.dmx_idx][prog_index]),
        section, length, rec_es_desc_info, 2);
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    rec_psi_es_isdbtcc_info_checkup((UINT32)(&g_monitor_pmt_sec[dynamic_param.dmx_idx][prog_index]),
        section, length, rec_es_desc_isdbtcc_info, ARRAY_SIZE(rec_es_desc_isdbtcc_info));
#endif

    if((rec_on_change)&&(1 == g_pid_change_flag[dynamic_param.dmx_idx][prog_index]))
    {
        MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
#if(defined(_MHEG5_V20_ENABLE_))
        mheg5_set_spin(SPIN_PMT_CHANGED) ;
#endif
        rec_on_change(p_node.prog_id);
        g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=0;
    }
    return 0;
}
#endif

static INT32 pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    if(NULL == section)
    {
        return 0;
    }

    callback_run(si_pmt_callback, MAX_SI_CALLBACK_CNT, section, length,
        #ifdef SI_MONITOR_CHOOSE_SERVICE_ID
            program.prog_number
        #else
            param   //m_CurChanIndex
        #endif
        );
    return 0;
}

INT32 si_monitor_on(UINT32 index)
{
    P_NODE node;
    INT32 ret = SUCCESS;
    struct dmx_device *dmx = NULL;

    if(SIM_INVALID_MON_ID != monitor_id)
    {
        return SUCCESS;
    }
    MEMSET(&node, 0, sizeof(P_NODE));
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
    SIM_PRINTF("si monitor on\n");

    if(get_prog_at(index, &node) != SUCCESS)
    {
        return ERR_FAILUE;
    }

    m_cur_chan_index = index;
    st_sdtt_set_channel_index(index);
    pat_monitor_id = sim_start_monitor(dmx, MONITE_TB_PAT, PSI_PAT_PID, 0);

    #if(defined(_MHEG5_V20_ENABLE_))
    sim_register_scb(pat_monitor_id, mheg_receive_pat_callback, NULL);
    #endif

    if((0 == node.user_modified_flag) && (0x0 != node.pmt_pid) && (0x1FFF != node.pmt_pid))
    {
        monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
        MEMCPY(&program, &node, sizeof(P_NODE));
        sim_register_ncb(monitor_id, notify_ap);
        sim_register_scb(monitor_id, monitor_pmt_sec, NULL);
        sim_register_scb(monitor_id, pmt_callback, NULL);
#if(defined(_MHEG5_V20_ENABLE_))
        sim_register_scb(monitor_id, mheg_receive_pmt_callback, NULL);
#endif
    }
//  else
    {
        /*No PMT PID inside PAT table, turn on PAT monitor to retrieval PMT PID */
        sim_register_scb(pat_monitor_id, monitor_pat_callback, NULL);
    }

    //start SDTT monitor
    sdtt_monitor_id = sim_start_monitor(dmx, MONITE_TB_SDTT, PSI_SDTT_PID, 0);
    sim_register_scb(sdtt_monitor_id, sdtt_callback, NULL);

    return ret;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT8 rec_check_pmt_monitor(UINT32 monitor_offid)
{
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 prog_index=2;
    INT32 ret = TRUE;

    for(i=0;i<2;i++)
    {
        for(j=0;j<2;j++)
        {
            if(rec_monitor_id[i][j] == monitor_offid)
            {
               ret = FALSE;
               break;
            }
        }
    }
    return ret;
}

INT32 api_sim_callback(UINT32 param)
{
    struct sim_cb_param *sim_param = (struct sim_cb_param *)param;
    UINT8 max_prog_index = 2;
    INT32 ret = ERR_FAILUE;
    UINT16 sim_pcr_pid = 0;
    UINT16 sim_video_pid = 0;
    UINT16 sim_audio_pid[P_MAX_AUDIO_NUM] = {0};
    UINT16 sim_audio_count = P_MAX_AUDIO_NUM;
    static BOOL on_process = TRUE;
    struct prog_info p_info;
    UINT16 sie_ecm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    UINT16 sie_emm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    UINT8 max_ecm_cnt = 16;
    UINT8 max_emm_cnt = 16;
    P_NODE p_node; // Come from get_prog_by_id
    INT8 sim_idx = -1,
    INT8 i = 0;
    UINT8 prog_index=0;

    for(prog_index=0 ; prog_index < max_prog_index ;prog_index++)
    {
        if(rec_monitor_id[sim_param->dmx_idx][prog_index]==sim_param->sim_id)
        {
            break;
        }
    }

    if(prog_index >= max_prog_index)
    {
        return ret;
    }

    do
    {
        if (MONITE_TB_PMT == sim_param->table)
        {
            sim_video_pid = PSI_INVALID_ID;
            sim_pcr_pid = PSI_INVALID_ID;
            MEMSET(sim_audio_pid, 1, P_MAX_AUDIO_NUM*sizeof(UINT16));

            if (ERR_FAILUE == sim_get_video_pid(sim_param->sim_id, &sim_video_pid))
            {
                break;
            }
            sim_video_pid &= 0x1fff;
            if (ERR_FAILUE == sim_get_audio_info(sim_param->sim_id, sim_audio_pid, NULL, &sim_audio_count))
            {
                break;
            }
            if (ERR_FAILUE== sim_get_pcr_pid(sim_param->sim_id, &sim_pcr_pid))
            {
                break;
            }
            if(ERR_FAILUE == sim_get_ecm_pid(sim_param->sim_id,sie_ecm_pids,&max_ecm_cnt))
            {
                break;
            }

            sim_get_pmt(rec_monitor_id[sim_param->dmx_idx][prog_index], &p_info);
            MEMCPY(&p_node,&rec_monitor_id_prog_node[sim_param->dmx_idx][prog_index], sizeof(P_NODE));

            if(p_node.pmt_pid != sim_param->sec_pid)
            {
               break;
            }

            if((sim_video_pid != p_node.video_pid)||(sim_pcr_pid != p_node.pcr_pid)
                ||(sim_audio_count != p_node.audio_count)
                ||(MEMCMP(p_node.audio_pid,sim_audio_pid, sizeof(UINT16)*sim_audio_count)!=0))
            {

                if(PROG_TV_MODE == p_node.av_flag)
                {
                     p_node.video_pid = sim_video_pid;
                }

                p_node.pcr_pid = sim_pcr_pid;
                p_node.audio_count = sim_audio_count;
                MEMCPY(p_node.audio_pid, sim_audio_pid, sizeof(UINT16)*sim_audio_count);

                if((p_node.ca_mode)&&(max_ecm_cnt != 0)&&(MEMCMP(ecm_pids,sie_ecm_pids,sizeof(UINT16)*max_ecm_cnt)!=0))
                {
                    //for (i=0;i<max_ecm_cnt;i++)
                    //{
                        //libc_printf("ecm pid changed![ %d] -> [ %d]\n",ecm_pids[i],sie_ecm_pids[i]);
                    //}
                    MEMCPY(ecm_pids,sie_ecm_pids, sizeof(UINT16)*max_ecm_cnt);
                }
                g_pid_change_flag[sim_param->dmx_idx][prog_index]=1;
                MEMCPY(&rec_monitor_id_prog_node[sim_param->dmx_idx][prog_index], &p_node, sizeof(P_NODE));
            }
        }

        //CA mode to update emm pid  pid
        if (MONITE_TB_CAT == sim_param->table)
        {
            if (ERR_FAILUE == sim_get_emm_pid(sim_param->sim_id,sie_emm_pids,&max_emm_cnt))
            {
                break;
            }

            if((p_node.ca_mode) && (max_emm_cnt != 0)&& (MEMCMP(emm_pids,sie_ecm_pids, sizeof(UINT16)*max_emm_cnt)!=0))
            {
                //for (i=0;i<max_emm_cnt;i++)
                //{
                    //libc_printf("emm pid changed![ %d] -> [ %d]\n",emm_pids[i],sie_emm_pids[i]);
                //}
                MEMCPY(emm_pids,sie_emm_pids, sizeof(UINT16)*max_emm_cnt);
            }
        }
        ret = SUCCESS;

    }while(0);
    on_process = FALSE;
    return ret;
}

INT32 rec_si_monitor_on(UINT8 dmx_id,UINT32 index)
{
    P_NODE node;
    INT32 ret = SUCCESS;
    UINT8 prog_index = 0;
    struct dmx_device *dmx = NULL;

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (dmx_id));
    if(get_prog_by_id(index, &node) != SUCCESS)
    {
        return ERR_FAILUE;
    }

    if((0 == node.user_modified_flag) && (0x0 != node.pmt_pid) && (0x1FFF != node.pmt_pid))
    {
        if(SIM_INVALID_MON_ID == rec_monitor_id[dmx_id][0])
        {
            prog_index=0;
            rec_monitor_id[dmx_id][prog_index] = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
            if(rec_monitor_id[dmx_id][prog_index]!=SIM_INVALID_MON_ID)
            {
                rec_monitor_id_prog[dmx_id][prog_index]=node.prog_id;
            }
        }
        else if(SIM_INVALID_MON_ID == rec_monitor_id[dmx_id][1])
        {
            prog_index=1;
            rec_monitor_id[dmx_id][prog_index] = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
            if(rec_monitor_id[dmx_id][prog_index]!=SIM_INVALID_MON_ID)
            {
                rec_monitor_id_prog[dmx_id][prog_index]=node.prog_id;
            }
        }
        else
        {
            prog_index=0;
        }

        MEMCPY(&rec_monitor_id_prog_node[dmx_id][prog_index], &node, sizeof(P_NODE));
        sim_register_ncb(rec_monitor_id[dmx_id][prog_index] , api_sim_callback);
        sim_register_scb(rec_monitor_id[dmx_id][prog_index] ,rec_monitor_pmt_sec, NULL);
    }

    return ret;
}
#endif
void si_monitor_off(UINT32 index)
{
#if ((defined(TTX_ON)&&(TTX_ON==1)) || ( SUBTITLE_ON == 1) || (defined(ISDBT_CC)&&ISDBT_CC == 1))
    UINT16 stuff_pid = 0x1FFF;
#endif
    P_NODE node;

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return;
    }

    SIM_PRINTF("si monitor off\n");
    m_cur_chan_index = 0xffff;

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
    if(TRUE == rec_check_pmt_monitor(monitor_id))
    {
        sim_stop_monitor(monitor_id);
    }
    else
    {
        sim_unregister_ncb(monitor_id,notify_ap);
        sim_unregister_scb(monitor_id,monitor_pmt_sec);
    }
#else
    sim_stop_monitor(monitor_id);
#endif
    monitor_id = SIM_INVALID_MON_ID;

    if ((0xFFFFFFFF == index)||(SUCCESS == get_prog_at(index, &node)))
    {
        if(0xFFFFFFFF == index)
        {
            SIM_PRINTF("    2.index=0xffffffff, call undo_prog_modify()\n");
            undo_prog_modify(program.tp_id, program.prog_number);
        }
    }

#if(defined(TTX_ON)&&(TTX_ON==1))
    if(FALSE == ttx_is_holded())
    {
        MEMSET(prev_ttx, 0xFF, sizeof(prev_ttx));
        ttx_descriptor(TELTEXT_DESCRIPTOR, 0, prev_ttx, (void *)&stuff_pid);
    }
#endif
#if( SUBTITLE_ON == 1)
    MEMSET(prev_sub, 0xFF, sizeof(prev_sub));
    subt_descriptor(SUBTITLE_DESCRIPTOR, 0, prev_sub, (void *)&stuff_pid);
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    isdbtcc_stream_iden_desc(STREAM_IDENTIFIER_DESCRIPTOR, 0, NULL, (void *)&stuff_pid);
#endif
    sim_stop_monitor(pat_monitor_id);
    pat_monitor_id = SIM_INVALID_MON_ID;

    sim_stop_monitor(sdtt_monitor_id);
    sdtt_monitor_id=SIM_INVALID_MON_ID;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT8 get_rec_monitor_index(UINT8 dmx_id,UINT32 pro_id)
{
    UINT8 index = 2;
    UINT8 i = 0;
    P_NODE p_node;

    for(i=0;i<2;i++)
    {
        MEMCPY(&p_node,&rec_monitor_id_prog_node[dmx_id][i], sizeof(P_NODE));
        if(p_node.prog_id == pro_id)
        {
            index = i;
            break;
        }
    }
    return index;
}

void rec_si_monitor_off(UINT8 dmx_id,UINT32 index,UINT8 off_sie)
{
    INT32 ret = SUCCESS;
    UINT16 stuff_pid = 0x1FFF;
    P_NODE node;
    UINT8 i=0;
    UINT8 prog_index = 2;
    UINT8 max_prog_index = 2;

    for(i=0;i<2;i++)
    {
        if(rec_monitor_id_prog[dmx_id][i]==index)
        {
            prog_index=i;
            break;
        }
    }

    if((max_prog_index == prog_index) || (rec_monitor_id[dmx_id][prog_index] == SIM_INVALID_MON_ID))
    {
        return;
    }

    SIM_PRINTF("[%s] rec_monitor_id[%d][%d] \n",__FUNCTION__, dmx_id,prog_index);

    if(1 == off_sie)
    {
        sim_stop_monitor(rec_monitor_id[dmx_id][prog_index]);
    }
    else
    {
        sim_unregister_ncb(rec_monitor_id[dmx_id][prog_index],api_sim_callback);
        sim_unregister_scb(rec_monitor_id[dmx_id][prog_index],rec_monitor_pmt_sec);
    }
    if (rec_monitor_id[dmx_id][prog_index] != monitor_id )
    {
        sim_stop_monitor(rec_monitor_id[dmx_id][prog_index]);
    }
    rec_monitor_id[dmx_id][prog_index] = SIM_INVALID_MON_ID;
    rec_monitor_id_prog[dmx_id][prog_index]=SIM_INVALID_MON_ID;
    MEMSET(&rec_monitor_id_prog_node[dmx_id][prog_index], 0 ,sizeof(P_NODE));

    return;
}
#endif

void si_monitor_register(on_pid_change_t pid_change)
{
    on_change = pid_change;
    return;
}

#if (SIM_DEBUG_LEVEL>0)
static void si_monitor_pre_register(on_pid_change_t pid_change)
{
    pre_change = pid_change;
}
#endif

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
void rec_si_monitor_register(on_rec_pid_change_t pid_change)
{
    rec_on_change = pid_change;
    return;
}
#endif

INT32 si_monitor_register_pmt_cb(section_parse_cb_t callback)
{
    return register_cb(si_pmt_callback, MAX_SI_CALLBACK_CNT, callback);
}

INT32 si_monitor_unregister_pmt_cb(section_parse_cb_t callback)
{
    return unregister_cb(si_pmt_callback, MAX_SI_CALLBACK_CNT, callback);
}

INT32 pmt_monitor_on(UINT16 pmt_pid, UINT16 prog_number/*service_id*/)
{
    INT32 ret = SUCCESS;
    struct dmx_device *dmx = NULL;

    if(SIM_INVALID_MON_ID != monitor_id)
    {
        return SUCCESS;
    }

    dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
    monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, pmt_pid, prog_number);
    sim_register_scb(monitor_id, pmt_callback, NULL);

    return ret;
}

void pmt_monitor_off(UINT16 pmt_pid)
{
//    struct dmx_device *dmx = NULL;

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return;
    }

//    dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
    sim_stop_monitor(monitor_id);
    monitor_id = SIM_INVALID_MON_ID;

    return;
}

#ifdef GET_CAT_BY_SIM
INT32 si_monitor_register_cat_cb(section_parse_cb_t callback)
{
    return register_cb(si_cat_callback, MAX_SI_CALLBACK_CNT, callback);
}

INT32 si_monitor_unregister_cat_cb(section_parse_cb_t callback)
{
    return unregister_cb(si_cat_callback, MAX_SI_CALLBACK_CNT, callback);
}

static INT32 cat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    callback_run(si_cat_callback, MAX_SI_CALLBACK_CNT, section, length,param);
    return 0;
}
#endif

void sie_start_emm_service(void)
{
    struct dmx_device *dmx = NULL;

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
    cat_monitor_id = sim_start_monitor(dmx, MONITE_TB_CAT, 0x01, 0);
    #ifdef GET_CAT_BY_SIM
    sim_register_scb(cat_monitor_id, cat_callback, NULL);
    #endif
}

void sie_stop_emm_service(void)
{
    sim_stop_monitor(cat_monitor_id);
    cat_monitor_id = SIM_INVALID_MON_ID;
}

UINT16 si_get_cur_channel(void)
{
    return m_cur_chan_index;
}

static INT8 si_get_service_pid(UINT8 *pbuf, UINT16 serviceid, UINT16 *pservicepid)
{
    if(NULL == pbuf)
    {
        return ERR_FAILUE;
    }

    UINT16 prognum = pbuf[0]<<8 | pbuf[1];

    if(serviceid == prognum)
    {
        *pservicepid = (pbuf[2]&0x1F)<<8 | pbuf[3];
        return SUCCESS;
    }
    pservicepid = NULL;
    return ERR_FAILUE;
}

static INT32 monitor_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    P_NODE node;
    struct dmx_device *dmx = NULL;
    INT32 prog_map_len = 0;
    UINT8 *prog_map = NULL;
    UINT8 same_prog_number = 0;
    UINT16 pmt_pid = 0;

    MEMSET(&node, 0, sizeof(P_NODE));
    if(get_prog_at(si_get_cur_channel(), &node) != SUCCESS)
    {
        return SUCCESS;
    }

    dmx = (struct dmx_device *)(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);

    if (section != NULL)
    {
        prog_map_len = (((section[1]&0xF)<<8)|section[2]) - 9;
        prog_map = section + 8;

        /*get PMT pid using program number*/
        while(prog_map_len > 0)
        {
            if(SUCCESS == si_get_service_pid(prog_map, node.prog_number, &pmt_pid))
            {
                if(node.pmt_pid == pmt_pid)//program's pmt_pid exist, stop parse PAT
                {
                    same_prog_number = 0;
                }
                else
                {
                    same_prog_number++;
                    SIM_PRINTF("-----si_monitor_on_pat_parsing()[si_monitor.c]2.prog_number %d match, pmt pid=%d\n ",
                    node.prog_number,pmt_pid);
                }
            }
            prog_map_len -= 4;
            prog_map += 4;
        }

        /*Actually, this callback doesn't care multi-section issue casue siae would send section continuously*/
        if(1 == sim_get_prog_pmt_pid(pat_monitor_id,node.prog_number,&pmt_pid))
        {
          if(node.pmt_pid != pmt_pid)
          {
            if(monitor_id != SIM_INVALID_MON_ID)
            {
                sim_stop_monitor(monitor_id);
                monitor_id = SIM_INVALID_MON_ID;
            }

            node.pmt_pid = pmt_pid;   /*Turn on PMT receive filter*/
            modify_prog(node.prog_id, &node);
            update_data();

            monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);

            MEMCPY(&program, &node, sizeof(P_NODE));

            sim_register_ncb(monitor_id, notify_ap);
            sim_register_scb(monitor_id, monitor_pmt_sec, NULL);
            sim_register_scb(monitor_id, pmt_callback, NULL);

            #if(defined(_MHEG5_V20_ENABLE_))
            sim_register_scb(monitor_id, mheg_receive_pmt_callback, NULL);
            #endif
          }
        }
    }
    return SUCCESS;
}

#if(defined(_MHEG5_V20_ENABLE_))
static void mheg_receive_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct sim_cb_param *cb_param = (struct sim_cb_param *)param;

    if (section!=NULL)
    {
        filter_callback(cb_param->sec_pid, section, length, PAT);
    }
}

static INT32 mheg_receive_pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct sim_cb_param *cb_param = (struct sim_cb_param *)param;

    filter_callback(cb_param->sec_pid, section, length, PMT);
}
#endif

static INT32 sdtt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    return SUCCESS;
}

