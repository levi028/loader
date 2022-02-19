/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_process_table.c
*
*    Description: process table data from DMX
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libtsi/sie.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/si_sdtt.h>
#include <api/libsi/sie_monitor.h>
#if (defined(DYNAMIC_SERVICE_SUPPORT) || defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
#include <api/libtsi/si_section.h>
#endif

#include "sie_monitor_core.h"

#define SIM_DEBUG_LEVEL     0
#if (SIM_DEBUG_LEVEL>0)
#define SIM_PRINTF          libc_printf
#else
#define SIM_PRINTF(...)     do{}while(0)
#endif

static sie_status_t __si_monitor_on_receive_pmt(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_pat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_cat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_nit(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_sdt(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);

#ifdef SUPPORT_FRANCE_HD
static sie_status_t __si_monitor_on_receive_sdt_other(struct DMX_PSI_INFO *info,UINT16 pid,struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
#endif

static sie_status_t __si_monitor_on_receive_bat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_sdtt(struct DMX_PSI_INFO *info, UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length);

/* Read Only, No need mutex protect */
struct PID_TID_PARSER pid_tid_parser[TABLE_TYPE_COUNT] =
{
    {PSI_INVALID_ID,PSI_PMT_TABLE_ID,__si_monitor_on_receive_pmt}, //pmt
    {PSI_PAT_PID,PSI_PAT_TABLE_ID,__si_monitor_on_receive_pat}, //pat
    {0x01,0x01,__si_monitor_on_receive_cat},//cat
    {PSI_INVALID_ID,PSI_NIT_TABLE_ID,__si_monitor_on_receive_nit},//nit
    {PSI_SDT_PID,PSI_SDT_TABLE_ID,__si_monitor_on_receive_sdt},//sdt
#ifdef SUPPORT_FRANCE_HD
    {PSI_SDT_PID,PSI_SDT_OTHER_TABLE_ID,__si_monitor_on_receive_sdt_other},//sdt other
#endif
    {PSI_INVALID_ID,PSI_BAT_TABLE_ID,__si_monitor_on_receive_bat},//bat
    {PSI_SDTT_PID, PSI_SDTT_TABLE_ID, __si_monitor_on_receive_sdtt},//sdtt
};

UINT32 monitor_id_valid(UINT32 monitor_id)
{
    if(monitor_id < MAX_SIM_COUNT)//if(monitor_id>=0 && monitor_id <MAX_SIM_COUNT)/*always is true ,clean warning*/
    {
        return 1;
    }
    return 0;
}

INT16 get_dmx_index(struct dmx_device *dmx)
{
    INT16 index=0;

    if(NULL == dmx)
    {
        return -1;
    }

    index = (dmx->type)&HLD_DEV_ID_MASK;
    return index;
}

UINT32 check_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 pid,UINT32 param)
{
    UINT8 i = 0;
    UINT16 dmx_idx = 0;

    if(NULL == dmx)
    {
        return SIM_INVALID_MON_ID;
    }

    for(i=0; i<MAX_SIM_COUNT; i++)
    {
        if(0 == monitor_exist[i])
        {
            continue;
        }

        if(MONITE_TB_PMT == table)
        {
            dmx_idx = get_dmx_index(dmx);
            if((simcb_array[i].table== table) && (dmx_idx == simcb_array[i].dmx_idx) && (pid == simcb_array[i].sec_pid)
                && (param == simcb_array[i].param) )
            {
                return i;
            }
        }
        else
        {
            dmx_idx = get_dmx_index(dmx);
            if((simcb_array[i].table== table)&&(dmx_idx == simcb_array[i].dmx_idx)&&(pid == simcb_array[i].sec_pid))
            {
                return i;
            }
        }
    }
    return SIM_INVALID_MON_ID;
}

//pmt 0-15, pat 16, cat 17, nit 18, sdt 19
UINT8 get_table_index(UINT16 dmx_idx, enum MONITE_TB table, UINT16 prog_number)
{
    UINT8 tbl_idx = 0xFF;

    if(dmx_idx >= DMX_COUNT)
    {
        return tbl_idx;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    if(MONITE_TB_PMT == table)
    {
        for(tbl_idx=0;tbl_idx<PMT_MAX_COUNT;tbl_idx++)
        {
           if((dmx_psi_info[dmx_idx].valid & (1<<tbl_idx))&&dmx_psi_info[dmx_idx].pmt[tbl_idx].prog_number==prog_number)
           {
               break;
           }
        }

        if(PMT_MAX_COUNT == tbl_idx)
        {
            tbl_idx=0xFF;
        }
    }
    else
    {
        tbl_idx = table-MONITE_TB_PAT+PMT_MAX_COUNT;
    }

    osal_semaphore_release(sm_semaphore);

    return tbl_idx;
}

void set_table_bit_valid(struct dmx_device *dmx, UINT16 table_id, UINT16 dmx_index, UINT16 pid, UINT32 param)
{
    UINT16 tmp_idx = 0;

    if(0 == dmx_psi_info[dmx_index].valid)//no data valid
    {
        MEMSET(&dmx_psi_info[dmx_index],0,sizeof(struct DMX_PSI_INFO));
        dmx_psi_info[dmx_index].dmx = dmx;
    }

    switch(table_id)
    {
    case PSI_PMT_TABLE_ID:
        {
            for(tmp_idx=0; tmp_idx<PMT_MAX_COUNT; tmp_idx++)
            {
                if(0 == (dmx_psi_info[dmx_index].valid & (1<<tmp_idx)))
                {
                    break;
                }
            }
            if(tmp_idx<PMT_MAX_COUNT)
            {
                MEMSET(&(dmx_psi_info[dmx_index].pmt[tmp_idx]), 0, sizeof(struct prog_info));
                dmx_psi_info[dmx_index].pmt[tmp_idx].pmt_pid = pid;
                dmx_psi_info[dmx_index].pmt[tmp_idx].prog_number = param;
                dmx_psi_info[dmx_index].valid |= (1<<tmp_idx);
            }
        }
        break;
    case PSI_PAT_TABLE_ID:
        {
            MEMSET(&(dmx_psi_info[dmx_index].pat), 0, sizeof(struct PAT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid |= (1<<PAT_VALID_BIT);
        }
        break;
    case 0x01://cat
        {
            MEMSET(&(dmx_psi_info[dmx_index].cat), 0, sizeof(struct CAT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid |= (1<<CAT_VALID_BIT);
        }
        break;
    case PSI_NIT_TABLE_ID:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].nit), 0, sizeof(struct nit_section_info));
            dmx_psi_info[dmx_index].valid |= (1<<NIT_VALID_BIT);
        }
        break;
    case PSI_SDT_TABLE_ID:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid |= (1<<SDT_VALID_BIT);
        }
        break;
    case PSI_BAT_TABLE_ID:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid |= (1<<BAT_VALID_BIT);
        }
        break;
    default:
        break;
    }

}

void set_table_bit_invalid(enum MONITE_TB table, UINT16 dmx_index, UINT16 pid)
{
    UINT16 tmp_idx = 0;

    switch(table)
    {
    case MONITE_TB_PMT:
        {
            for(tmp_idx=0;tmp_idx<PMT_MAX_COUNT;tmp_idx++)
            {
                if((dmx_psi_info[dmx_index].valid & (1<<tmp_idx)) &&
                   (dmx_psi_info[dmx_index].pmt[tmp_idx].pmt_pid==pid))
                {
                    break;
                }
            }
            if(tmp_idx<PMT_MAX_COUNT)
            {
                MEMSET(&(dmx_psi_info[dmx_index].pmt[tmp_idx]), 0, sizeof(struct prog_info));
                dmx_psi_info[dmx_index].valid &= ~(1<<tmp_idx);
            }
        }
        break;
    case MONITE_TB_PAT:
        {
            MEMSET(&(dmx_psi_info[dmx_index].pat), 0, sizeof(struct PAT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid &= ~(1<<PAT_VALID_BIT);
        }
        break;
    case MONITE_TB_CAT://cat
        {
            MEMSET(&(dmx_psi_info[dmx_index].cat), 0, sizeof(struct CAT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid &= ~(1<<CAT_VALID_BIT);
        }
        break;
    case MONITE_TB_NIT:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].nit), 0, sizeof(struct nit_section_info));
            dmx_psi_info[dmx_index].valid &= ~(1<<NIT_VALID_BIT);
        }
        break;
    case MONITE_TB_SDT:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid &= ~(1<<SDT_VALID_BIT);
        }
        break;
    case MONITE_TB_BAT:
        {
            //MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
            dmx_psi_info[dmx_index].valid &= ~(1<<BAT_VALID_BIT);
        }
        break;
    default:
        break;
    }

    if(0 == dmx_psi_info[dmx_index].valid)
    {
        dmx_psi_info[dmx_index].dmx = NULL;
    }

}

INT32 register_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback)
{
    INT32 i = 0;
    INT32 n = -1;

    if (NULL == callback)
    {
        SIM_PRINTF("%s: callback is NULL!\n",__FUNCTION__);
        return !SUCCESS;
    }

    for (i=0; i<max_cnt; i++)
    {
        if (callback_tab[i] == callback)
        {
            SIM_PRINTF("%s: callback 0x%X already exist!\n",__FUNCTION__,callback);
            return SUCCESS;
        }
        else if ((NULL == callback_tab[i]) && (-1 == n))
        {
            n = i;
        }
    }

    if ((n >= 0) && (n < max_cnt))
    {
        SIM_PRINTF("%s: callback %d registered to 0x%X\n",__FUNCTION__,n,callback);
        callback_tab[n] = callback;
        return SUCCESS;
    }
    else
    {
        SIM_PRINTF("%s: callback table is full!!!\n",__FUNCTION__);
        return !SUCCESS;
    }
}

INT32 unregister_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback)
{
    INT32 i = 0;

    if (NULL == callback)
    {
        SIM_PRINTF("%s: callback is NULL!\n",__FUNCTION__);
        return !SUCCESS;
    }

    for (i=0; i<max_cnt; i++)
    {
        if (callback_tab[i] == callback)
        {
            callback_tab[i] = NULL;

            SIM_PRINTF("%s: callback %d unregistered!\n",__FUNCTION__,i);
            return SUCCESS;
        }
    }

    SIM_PRINTF("%s: callback %X not found!\n",__FUNCTION__,callback);
    return !SUCCESS;
}

void callback_run(section_parse_cb_t *callback_tab, INT max_cnt,UINT8 *buf, INT32 length, UINT32 param)
{
    UINT8 i = 0;

    for (i=0; i<max_cnt; i++)
    {
        if (callback_tab[i] != NULL)
        {
            callback_tab[i](buf, length, param);
        }
    }
}

static table_section_parser get_table_parser(UINT8 table_id)
{
    int table_index = 0;

    /* Read only Global variable, no need protect */
    for(table_index=0; table_index<TABLE_TYPE_COUNT; table_index++)
    {
        if(pid_tid_parser[table_index].tid == table_id)
        {
            return pid_tid_parser[table_index].parser;
        }
    }
    return NULL;
}

static void run_callback(UINT32 monitor_id,UINT8 *buff, INT32 length)
{
    UINT8 tbl_idx = 0xFF;
    UINT8 i = 0;
    struct sim_cb_param param;
    UINT16 dmx_idx = 0;

    if(!monitor_id_valid(monitor_id) || (1 != monitor_exist[monitor_id]) || (NULL == buff))
    {
        return;
    }

    dmx_idx = simcb_array[monitor_id].dmx_idx;
    tbl_idx = get_table_index(dmx_idx,simcb_array[monitor_id].table,simcb_array[monitor_id].param);

    if(0xFF == tbl_idx)
    {
        return;
    }

    MEMSET(&param, 0, sizeof(struct sim_cb_param));
    MEMCPY(&param, &(simcb_array[monitor_id]), sizeof(struct sim_cb_param));

    //run call back
    //osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    for(i=0; i<MAX_SIM_NCB_CNT; i++)
    {
        if(sim_ncb[dmx_idx][tbl_idx][i] != NULL)
        {
            sim_ncb[dmx_idx][tbl_idx][i]((UINT32)(&param));
        }
    }
    for(i=0; i<MAX_SIM_SCB_CNT; i++)
    {
        if(sim_scb[dmx_idx][tbl_idx][i] != NULL)
        {
            param.priv = (void*)(sec_cb_priv[dmx_idx][tbl_idx][i]);
            sim_scb[dmx_idx][tbl_idx][i](buff, length, (UINT32)(&param));
        }
    }
    //osal_semaphore_release(sm_semaphore);
    return;
}

static sie_status_t __si_monitor_on_receive_pmt(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    #ifdef SUPPORT_CAS7
    UINT8 dmx_id = 0;
    #endif
    UINT8 tbl_idx = 0xFF;
    struct prog_info *prog = NULL;
    UINT32 monitor_id = SIM_INVALID_MON_ID;
    UINT16 prog_number = 0;
    
    SIM_PRINTF("PMT on pid(0x%x)\n",pid);

    if((NULL == info) || (NULL == buff) || (SIE_REASON_FILTER_TIMEOUT == reason))
    {
        return sie_started;
    }

    prog_number = (buff[3]<<8)|buff[4];
    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    monitor_id = check_monitor(info->dmx, MONITE_TB_PMT, pid, prog_number);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    tbl_idx = get_table_index(simcb_array[monitor_id].dmx_idx,MONITE_TB_PMT,prog_number);

    if(0xFF == tbl_idx)
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    prog = &(info->pmt[tbl_idx]);
    MEMSET(prog, 0, sizeof(struct prog_info));
    prog->pmt_pid = pid;
    prog->prog_number = prog_number;
    psi_pmt_parser(buff, prog,PSI_MODULE_COMPONENT_NUMBER);
    osal_semaphore_release(sm_semaphore);

    run_callback(monitor_id,buff,length);
    #ifdef SUPPORT_CAS7
        dmx_id = get_dmx_index(info->dmx) ;
        cas7_pmt_run_callback(dmx_id,pid,buff,length);
    #endif
    return sie_started;
}

static sie_status_t __si_monitor_on_receive_pat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    struct PAT_TABLE_INFO *pat = NULL;
    INT32 prog_map_len = 0;
    UINT8 *prog_map = NULL;
    UINT16 prog_number = 0;
    UINT32 monitor_id = SIM_INVALID_MON_ID;

    if((NULL == info) || (NULL == buff) || (SIE_REASON_FILTER_TIMEOUT == reason))
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    pat = &(info->pat);
    pat->max_map_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
    pat->map_counter = 0;
    pat->ts_id = (buff[3]<<8)|buff[4];

    prog_map_len = (((buff[1]&0xF)<<8)|buff[2]) - 9;
    prog_map = buff + 8;
    while(prog_map_len>0)
    {
        prog_number = (prog_map[0]<<8)|prog_map[1];
        if(0 == prog_number)
        {
            pat->nit_pid = ((prog_map[2]&0x1F)<<8)|prog_map[3];
        }
        else
        {
            if(pat->map_counter < pat->max_map_nr)
            {
                pat->map[pat->map_counter].pm_number = prog_number;
                pat->map[pat->map_counter].pm_pid = ((prog_map[2]&0x1F)<<8)|prog_map[3];
                pat->map_counter++;
            }
        }
        prog_map_len -= 4;
        prog_map += 4;
    }

    monitor_id = check_monitor(info->dmx, MONITE_TB_PAT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    run_callback(monitor_id,buff,length);

    return sie_started;
}

static sie_status_t __si_monitor_on_receive_cat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    struct CAT_TABLE_INFO *cat = NULL;
    UINT8 ca_desc_tag = 0x09;
    INT32 cat_length = 0;
    UINT16 index = 0;
    UINT32 monitor_id = -1;

    if((NULL == info) || (NULL == buff) || (SIE_REASON_FILTER_TIMEOUT == reason))
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

    if(!(info->valid & (1<<CAT_VALID_BIT)))
    {
        osal_semaphore_release(sm_semaphore);
        return sie_started;
    }

    cat = &(info->cat);
    cat->emm_count = 0;
    cat_length = ((buff[1]&0x0F)<<8) | buff[2];
    index = 8;
    cat_length -= 9;

    while(cat_length > 0)
    {
        if(ca_desc_tag == buff[index])
        {
            if(cat->emm_count<SIM_EMM_MAX_COUNT)
            {
                cat->ca_sysid_array[cat->emm_count] = buff[index+2] | buff[index+3];
                cat->emm_pid_array[cat->emm_count] = ((buff[index+4]&0x1F)<<8) | buff[index+5];
                cat->emm_count++;
            }
            else
            {
                SIM_PRINTF("emm_count overflow\n");
            }
        }
        cat_length -= 2+buff[index+1];
        index += 2+buff[index+1];
    }
    monitor_id = check_monitor(info->dmx, MONITE_TB_CAT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    run_callback(monitor_id,buff,length);

    return sie_started;
}

static sie_status_t __si_monitor_on_receive_nit(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT32 monitor_id = -1;

    if(NULL == info)
    {
        return sie_started;
    }
    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    monitor_id = check_monitor(info->dmx, MONITE_TB_NIT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if((NULL == buff) || (SIM_INVALID_MON_ID == monitor_id))
    {
        return sie_started;
    }

    run_callback(monitor_id, buff, length);
    return sie_started;
}

static sie_status_t __si_monitor_on_receive_sdt(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT32 monitor_id = -1;

    if((NULL == info) || (NULL == buff))
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

#if (defined(DYNAMIC_SERVICE_SUPPORT) || defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
    struct SDT_TABLE_INFO *sdt = &info->sdt;
    struct sdt_stream_info *ss_info = NULL;
    int i = 0;
    int descriptors_length = 0;

    if (buff[0] != PSI_SDT_TABLE_ID)
    {
        osal_semaphore_release(sm_semaphore);
        return sie_started;
    }

    MEMSET(sdt, 0, sizeof(struct SDT_TABLE_INFO));
    sdt->t_s_id = (buff[3] << 8) | buff[4];
    sdt->onid = (buff[8] << 8) | buff[9];
    sdt->sdt_version = (buff[5] & 0x3E) >> 1;

    for (i = (sizeof(struct sdt_section) - PSI_SECTION_CRC_LENGTH);
         i < (length - PSI_SECTION_CRC_LENGTH);
         i += (sizeof(struct sdt_stream_info) + descriptors_length))
    {
        ss_info = (struct sdt_stream_info *)(buff + i);
        descriptors_length = SI_MERGE_HL8(ss_info->descriptor_loop_length);
#if(defined(ITALY_HD_BOOK_SUPPORT) || defined(_MHEG5_SUPPORT_))
        sdt->sd[sdt->sd_count].service_running_status = ss_info->running_status;
#endif
        sdt->sd[sdt->sd_count].program_number = SI_MERGE_UINT16(ss_info->service_id);

        sdt->sd_count++;
    }
#endif

    monitor_id = check_monitor(info->dmx, MONITE_TB_SDT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    run_callback(monitor_id, buff, length);

    return sie_started;
}

#ifdef SUPPORT_FRANCE_HD
static sie_status_t __si_monitor_on_receive_sdt_other(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT32 monitor_id = -1;

    if(NULL == info)
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    monitor_id = check_monitor(info->dmx, MONITE_TB_SDT_OTHER, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    run_callback(monitor_id, buff, length);
    return sie_started;
}
#endif

static sie_status_t __si_monitor_on_receive_bat(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT32 monitor_id = -1;

    if(NULL == info)
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    monitor_id = check_monitor(info->dmx, MONITE_TB_NIT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    run_callback(monitor_id, buff, length);

    return sie_started;
}

static sie_status_t  __si_monitor_on_receive_sdtt(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    UINT32 monitor_id = -1;

    if(NULL == info)
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    monitor_id = check_monitor(info->dmx, MONITE_TB_SDTT, pid, 0);
    osal_semaphore_release(sm_semaphore);

    if(SIM_INVALID_MON_ID == monitor_id)
    {
        return sie_started;
    }

    osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
    si_sdtt_parser(buff,length,NULL);
    osal_semaphore_release(sm_semaphore);

    run_callback(monitor_id, buff, length);
    return sie_started;
}

#ifdef SUPPORT_FRANCE_HD
///for Multi-section monitor
static BOOL __si_monitor_on_sec_event(UINT16 pid,struct si_filter_t *filter,UINT8 reason,UINT8 *buffer,INT32 length)
{
    return TRUE;
}
#endif

/*! SI monitor's fparam.section_parser
 * No corresponse fparam.section_event except France HD project
 *
 * Receive sections that being monitored, and call response parser
 * by table_id.
 */
 static sie_status_t __si_monitor_on_receive_sec(UINT16 pid, struct si_filter_t *filter,
    UINT8 reason, UINT8 *buff, INT32 length)
{
    struct dmx_device *dmx = NULL;
    UINT8 table_id = 0;
    UINT16 dmx_index = 0;
    UINT16 invalid_len = 6;
    table_section_parser section_parser = NULL;

    if((NULL == buff)||(NULL == filter)||(SIE_REASON_FILTER_TIMEOUT == reason))
    {
        return sie_started;
    }

    if(length <= invalid_len)
    {
        return sie_started;
    }

    table_id = buff[0];
    dmx = filter->dmx;
    dmx_index = get_dmx_index(dmx);

    if(dmx_index >= DMX_COUNT)
    {
        return sie_started;
    }

    /* Call the section parser to process section data */
    section_parser = get_table_parser(table_id);
    if (section_parser)
    {
        section_parser(&dmx_psi_info[dmx_index],pid,filter,reason,buff,length);
    }

    return sie_started;
}

void set_monitor_param(UINT16 table_id, UINT32 param, struct si_filter_param *fparam)
{
    if(NULL == fparam)
    {
        return ;
    }

    MEMSET(fparam, 0, sizeof(struct si_filter_param));
    fparam->timeout = OSAL_WAIT_FOREVER_TIME;
    fparam->attr[0] = SI_ATTR_HAVE_CRC32;

#ifdef SUPPORT_FRANCE_HD
    fparam->section_event = __si_monitor_on_sec_event;///for Multi-section monitor
#else
    fparam->section_event = NULL;
#endif
    fparam->section_parser = (si_handler_t)__si_monitor_on_receive_sec;

    if(PSI_PMT_TABLE_ID == table_id)
    {
        fparam->mask_value.mask_len = 6;
        fparam->mask_value.mask[0] = 0xFF;
        fparam->mask_value.mask[1] = 0x80;
        fparam->mask_value.mask[3] = 0xFF;
        fparam->mask_value.mask[4] = 0xFF;
        fparam->mask_value.mask[5] = 0x01;
        fparam->mask_value.value_num = 1;
        fparam->mask_value.value[0][0] = table_id;
        fparam->mask_value.value[0][1] = 0x80;
        fparam->mask_value.value[0][3] = (param>>8)&0xFF;
        fparam->mask_value.value[0][4] = param&0xFF;
        fparam->mask_value.value[0][5] = 0x01;
    }
    else
    {
        fparam->mask_value.mask_len = 6;
        fparam->mask_value.mask[0] = 0xFF;
        fparam->mask_value.mask[1] = 0x80;
        fparam->mask_value.mask[5] = 0x01;
        fparam->mask_value.value_num = 1;
        fparam->mask_value.value[0][0] = table_id;
        fparam->mask_value.value[0][1] = 0x80;
        fparam->mask_value.value[0][5] = 0x01;
    }

}

