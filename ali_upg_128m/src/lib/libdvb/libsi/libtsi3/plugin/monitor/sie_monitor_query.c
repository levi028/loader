/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_query.c
*
*    Description: provide query interface for the user to get SI data in table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
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
#include "sie_monitor_core.h"
#include "sie_monitor_process_table.h"

#ifdef DYNAMIC_SERVICE_SUPPORT
#include <api/libsi/si_sdt.h>
#endif


UINT32 sim_get_prog_pmt_pid(UINT32 monitor_id, UINT16 prog_num, UINT16 *pmt_pid)
{
    INT16 dmx_idx = 0;
    UINT32 match_count = 0;
    UINT16 i = 0;

    if((!monitor_id_valid(monitor_id)) || ((monitor_exist[monitor_id] != 1)) || (NULL == pmt_pid))
    {
        return 0;
    }

    if(simcb_array[monitor_id].table != MONITE_TB_PAT)
    {
        return 0;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;

    for(i=0; i<dmx_psi_info[dmx_idx].pat.map_counter; i++)
    {
        if(prog_num == dmx_psi_info[dmx_idx].pat.map[i].pm_number)
        {
            *pmt_pid = dmx_psi_info[dmx_idx].pat.map[i].pm_pid;
            match_count++;
        }
    }

    return match_count;
}

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
INT16 sim_get_prog_number(UINT32 monitor_id)
{
    UINT16 dmx_id = 0XFFFF;
    INT16 prog_number = 0;

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_id = simcb_array[monitor_id].dmx_idx;
    prog_number = dmx_psi_info[dmx_id].pat.map_counter;

    return prog_number;
}

#endif

INT32 sim_get_ca_info(UINT32 monitor_id, CA_INFO *ca_ptr, UINT8 *max_ca_count)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0xFF;

    if((NULL == ca_ptr) || (NULL == max_ca_count))
    {
        return ERR_FAILUE;
    }

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;

    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    if((*max_ca_count > 0))
    {
        if(*max_ca_count > dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count)
        {
            *max_ca_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count;
        }

        MEMCPY(ca_ptr, dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_info,(*max_ca_count)*sizeof(CA_INFO));
    }
    else
    {
        *max_ca_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count;
    }

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_ecm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_ecm_count)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0xFF;
    int i = 0;

    if((NULL == pid_buffer_ptr) || (NULL == max_ecm_count))
    {
        return ERR_FAILUE;
    }

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;

    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    if((*max_ecm_count > 0))
    {
        for(i=0; i<dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count && i<*max_ecm_count; i++)
        {
            pid_buffer_ptr[i] = dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_info[i].ca_pid;
        }
        *max_ecm_count = i;
    }
    else
    {
        *max_ecm_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count;
    }

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_emm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_emm_count)
{
    UINT8 i = 0;
    INT32 dmx_idx = -1;

    if((NULL == pid_buffer_ptr) || (NULL == max_emm_count))
    {
        return ERR_FAILUE;
    }

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    if((*max_emm_count>0))
    {
        for(i=0; (i < dmx_psi_info[dmx_idx].cat.emm_count) && (i < *max_emm_count); i++)
        {
            pid_buffer_ptr[i] = dmx_psi_info[dmx_idx].cat.emm_pid_array[i];
        }
        *max_emm_count = i;
    }
    else
    {
        *max_emm_count  = dmx_psi_info[dmx_idx].cat.emm_count;
    }

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_pcr_pid(UINT32 monitor_id, UINT16 *pcr_pid)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1) || (NULL == pcr_pid))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;

    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    *pcr_pid = dmx_psi_info[dmx_idx].pmt[tbl_idx].pcr_pid;

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

#ifdef SCTE_SUBTITLE_SUPPORT
INT32 sim_get_scte_subt_pid(UINT32 monitor_id, UINT8 *pid_count,
    UINT16 *subt_pid, UINT8 *pid_lan_count, UINT32 lan[][6])
{
    INT16 dmx_idx = -1;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;

    if((NULL == pid_count) || (NULL == subt_pid) || (NULL == pid_lan_count) || (NULL == lan))
    {
        return ERR_FAILURE;
    }
    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;

    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    *pid_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_count;
    MEMCPY(subt_pid, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_pid, *pid_count*sizeof(UINT16));
    MEMCPY(pid_lan_count, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_lan_cnt, *pid_count*sizeof(UINT8));
    MEMCPY(lan, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_lan, *pid_count*6*sizeof(UINT32));

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

#endif

INT32 sim_get_video_pid(UINT32 monitor_id, UINT16 *video_pid)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1) || (NULL == video_pid))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;

    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    *video_pid = dmx_psi_info[dmx_idx].pmt[tbl_idx].video_pid;

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_ca_mode(UINT32 monitor_id,UINT32 *i_ret)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1) || (NULL == i_ret))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;
    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }
    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    if(0 != dmx_psi_info[dmx_idx].pmt[tbl_idx].ca_count)
    {
        *i_ret=1;
    }
    else
    {
        *i_ret=0;
    }
    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}
INT32 sim_get_audio_info(UINT32 monitor_id, UINT16 *audio_pid, UINT8 audio_lang[][3], UINT8 *max_audio_count)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;
    int i = 0;

    if((NULL == max_audio_count) || (NULL == audio_pid))
    {
        return ERR_FAILUE;
    }

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;
    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    if((audio_pid != NULL) || (audio_lang != NULL))
    {
        for(i=0; i<*max_audio_count&&i<dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_count; i++)
        {
            if(audio_pid != NULL)
            {
               audio_pid[i] = dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_pid[i];
            }
            if(audio_lang != NULL)
            {
                MEMCPY(audio_lang[i], dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_lang[i],3);
            }
        }
        *max_audio_count = i;
    }
    else
    {
        *max_audio_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_count;
    }

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_pmt(UINT32 monitor_id,struct prog_info *info)
{
    UINT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;

    if(NULL == info)
    {
        return ERR_FAILUE;
    }

    if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;
    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);

    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    MEMCPY(info, &(dmx_psi_info[dmx_idx].pmt[tbl_idx]), sizeof(struct prog_info));

    osal_semaphore_release(sm_semaphore);
    return SUCCESS;
}

INT32 si_get_service_new_orinid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_orinid)
{
    return ERR_FAILUE;
}

INT32 si_get_service_new_tsid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_tsid)
{
    INT16 dmx_idx = 0;

    if( !monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1) || (NULL == new_tsid))
    {
        return ERR_FAILUE;
    }

    if(simcb_array[monitor_id].table != MONITE_TB_PAT)
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    *new_tsid = dmx_psi_info[dmx_idx].pat.ts_id;
    return SUCCESS;
}

UINT32 sie_get_ecm_pid(UINT16 *pid_buffer_ptr)
{
    UINT8 max_count = 16;

    if((SIM_INVALID_MON_ID == monitor_id) || (NULL == pid_buffer_ptr))
    {
        return 0;
    }

    sim_get_ecm_pid(monitor_id, pid_buffer_ptr, &max_count);

    return max_count;
}

void sie_get_ecm_emm_cnt(UINT8 *ecm_count_ptr, UINT8 *emm_count_ptr)
{
    if((NULL == ecm_count_ptr) || (NULL == emm_count_ptr))
    {
        return ;
    }

    *ecm_count_ptr = 0;
    *emm_count_ptr = 0;

    if(monitor_id != SIM_INVALID_MON_ID)
    {
        sim_get_ecm_pid(monitor_id, NULL, ecm_count_ptr);
    }

    if(cat_monitor_id != SIM_INVALID_MON_ID)
    {
        sim_get_emm_pid(cat_monitor_id, NULL, emm_count_ptr);
    }
}

UINT32 sie_get_emm_pid(UINT16 *pid_buffer_ptr)
{
    UINT8 max_count = 32;

    if((SIM_INVALID_MON_ID == cat_monitor_id) || (NULL == pid_buffer_ptr))
    {
        return 0;
    }

    sim_get_emm_pid(cat_monitor_id, pid_buffer_ptr, &max_count);

    return max_count;
}

#ifdef DYNAMIC_SERVICE_SUPPORT
INT32 sim_get_sdt_info(UINT32 monitor_id, struct sdt_info *info)
{
    struct SDT_TABLE_INFO *sdt = NULL;
    INT16 dmx_idx = 0;
    UINT8 i = 0;

    if (!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1) || (NULL == info))
    {
        return ERR_FAILUE;
    }

    if (MONITE_TB_SDT != simcb_array[monitor_id].table)
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    sdt = &dmx_psi_info[dmx_idx].sdt;

    info->t_s_id = sdt->t_s_id;
    info->onid = sdt->onid;
    info->sdt_version = sdt->sdt_version;
    info->service_cnt = sdt->sd_count;
    for (i = 0; i < sdt->sd_count; i++)
    {
        info->ssi[i].service_id = sdt->sd[i].program_number;
    }

    return SUCCESS;
}
#endif

#if (defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
INT32 sim_get_video_info(UINT32 monitor_id, UINT16 *video_pid, UINT8 *max_video_count)
{
    INT16 dmx_idx = 0;
    UINT16 prog_number = 0;
    UINT8 tbl_idx = 0;
    int i = 0;

    if((!monitor_id_valid(monitor_id)) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    if((NULL == video_pid) || (NULL == max_video_count))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    prog_number = simcb_array[monitor_id].param;
    tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
    if ((tbl_idx >= PMT_MAX_COUNT) || (0xFF == tbl_idx))
    {
        return ERR_FAILUE;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    if(video_pid != NULL)
    {
        for(i=0; i<*max_video_count && i<dmx_psi_info[dmx_idx].pmt[tbl_idx].video_cnt; i++)
        {
            if(video_pid != NULL)
            {
                video_pid[i] = dmx_psi_info[dmx_idx].pmt[tbl_idx].video_pid_array[i];
            }
        }
        *max_video_count = i;
    }
    else
    {
        *max_video_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].video_cnt;
    }

    osal_semaphore_release(sm_semaphore);

    return SUCCESS;
}

INT32 sim_get_running_status(UINT32 monitor_id, UINT16 *prog_num, UINT8 *pb_running_status)
{
    INT16 dmx_idx = 0;
    UINT8 tbl_idx = 0;
    int i = 0;

    if ((NULL == prog_num) || (NULL == pb_running_status))
    {
        return ERR_FAILUE;
    }

    if ((!monitor_id_valid(monitor_id)) || (monitor_exist[monitor_id] != 1))
    {
        return ERR_FAILUE;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    *prog_num = simcb_array[monitor_id].param;
    tbl_idx = get_table_index(dmx_idx, MONITE_TB_SDT, *prog_num);
    if (0xFF == tbl_idx)
    {
        return ERR_FAILUE;
    }

    for (i = 0; i<dmx_psi_info[dmx_idx].sdt.sd_count; i++)
    {
        if (dmx_psi_info[dmx_idx].sdt.sd[i].program_number == *prog_num)
        {
            *pb_running_status = dmx_psi_info[dmx_idx].sdt.sd[i].service_running_status;
        }
    }

    return SUCCESS;
}
#endif


