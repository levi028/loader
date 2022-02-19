/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_as_proc.c
*
* Description:
*     auto-scan service implementation for ali chip based IRD.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <mediatypes.h>
#include <bus/tsi/tsi.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libnim/lib_nim.h>

#include <api/libdiseqc/lib_diseqc.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/tmp_info.h>
#include <api/libtsi/siae.h>
#include <api/libtsi/si_search.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/as_module.h>

#include "lib_as_inner.h"
#include "../as_dvbs.h"
#include "../as_dvbc.h"
#include "../as_dvbt.h"

/*******************************************************
* macro define
********************************************************/


/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
static struct as_module_info *as_info = NULL;
static INT16 m_crnum = 0;

static UINT32 t_srh_tp_step = 0;
static UINT32 dvbt_bs_freq_end = 0;

/*NIt search callback for update tp*/
UINT8 as_update_tp(TP_INFO *t_info)
{
    INT32 i = 0;
    INT32 ret = -1;
    UINT8 front_end_type = as_info->param.as_frontend_type;

    if (NULL == t_info)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return 1;
    }
    switch (front_end_type)
    {
        case FRONTEND_TYPE_S:
            dvbs_tp_info2db(t_info, &as_info->cur_tp);
            //Check whether Current TP Param is Valid or not
            if(!dvbs_tp_sanity_check(as_info->voltage_ctrl, &as_info->cur_tp, &as_info->cur_sat))
            {
                return as_info->stop_scan_signal;
            }
            break;
        case FRONTEND_TYPE_C:
            //Copy Input TP Info to Current TP
            dvbc_tp_info2db(t_info, &as_info->cur_tp);
            //Check whether Current TP Param is Valid or not
            if(!dvbc_tp_sanity_check(FALSE, &as_info->cur_tp, NULL))
            {
                return as_info->stop_scan_signal;
            }
            break;
        case FRONTEND_TYPE_T:
	case FRONTEND_TYPE_ISDBT:
        #if (defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT) || defined(COMBOUI))
            //Copy Input TP Info to Current TP
            dvbt_tp_info2db(t_info, &as_info->cur_tp);
        //Check whether Current TP Param is Valid or not
            if(!dvbt_tp_sanity_check(FALSE, &as_info->cur_tp, NULL))
            {
                return as_info->stop_scan_signal;
            }
        #endif
            break;
        default:
            break;
    }

    //Search TP Info in DB. If can't find, then Add this TP Info into DB
        as_info->cur_tp.sat_id = as_info->cur_sat.sat_id;
	    #ifdef MULTISTREAM_SUPPORT
    	as_info->cur_tp.plp_id = 0XFF;
    	#endif
        if(db_search_lookup_node(TYPE_SEARCH_TP_NODE, &as_info->cur_tp) != SUCCESS)
        {
    #ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
            as_info->cur_tp.pat_version = 0xff;
    #endif
            ret = add_node(TYPE_SEARCH_TP_NODE,as_info->cur_tp.sat_id, &as_info->cur_tp);
            /*if(STORE_SPACE_FULL != ret)
            {
                return 1;
            }*/
            if (DB_SUCCES != ret)
            {
                AS_PRINTF("%s(): failed at lineL:%d\n", __FUNCTION__, __LINE__);
            }
        }

        AS_DUMP_TP(&as_info->cur_tp);
    //Search Current TP in Waiting Scan TP List, if find, then Return,
    // otherwise Add Current TP to Waiting Scan TP List
    #ifndef     STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        for(i = as_info->tp_nr - 1; i >= 0; i--)
        {
            if (as_info->tp_id[i] == as_info->cur_tp.tp_id)
            {
                return as_info->stop_scan_signal;
            }
        }
    #endif

        as_info->tp_id[as_info->tp_nr] = as_info->cur_tp.tp_id;
        as_info->tp_nr++;
#ifdef NIT_AUTO_SEARCH_SUPPORT// add for auto scan with nit search
    if((FLAG_TRUE == as_info->atp.nit_enable) && (as_info->tp_nr > COUNT_0))
    {
        b_found_nit_tp = TRUE;
    }
    else
    {
        b_found_nit_tp = FALSE;
    }
#endif

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
    //set receive nit flag
    if(SEARCH_NIT_STATUS_NOT_REC == star_search_nit_staus)
    {
        star_search_nit_staus = SEARCH_NIT_STATUS_REC;
    }
#endif

    if(as_info->param.as_handler != NULL)
    {
        as_info->param.as_handler(ASM_TYPE_INC_TRANSPONDER, 0);
    }

    return as_info->stop_scan_signal;
}



/*description   : transpoder program search update callback.*/
UINT8 psi_callback(UINT8 progress, void *param)
{
    UINT16 pg_pos = 0;
    P_NODE *p_node = NULL;
    UINT16 tp_cnt = 0;

    if (NULL == param)
    {
        return as_info->stop_scan_signal;
    }

    //nit scan update tp
    if (progress>=AS_PROGRESS_NIT)
    {
        if(AS_PROGRESS_NIT_TP_CNT == progress)
        {
            tp_cnt = *((UINT16 *)param);

            if(as_info->param.as_handler!=NULL)
            {
                as_info->param.as_handler(ASM_TYPE_NIT_TP_CNT, tp_cnt);
            }
            return as_info->stop_scan_signal;
        }
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        else if(AS_PROGRESS_SEARCH_MODE_UPDATE == progress)
        {
            UINT16 search_mode = *((UINT16 *)param);

            if(as_info->param.as_handler!=NULL)
            {
                as_info->param.as_handler(ASM_TYPE_SEARCH_MODE_CHANGE, search_mode);
            }
            return as_info->stop_scan_signal;

        }
#endif
        return as_update_tp((TP_INFO *)param);
    }
    else if (progress!=AS_PROGRESS_NIT_VER_CHANGE)
    {
        p_node = (P_NODE *)param;

    //#ifdef ITALY_HD_BOOK_SUPPORT
    #if(defined ITALY_HD_BOOK_SUPPORT ||defined     POLAND_SPEC_SUPPORT)
        pg_pos = post_processing_get_prog_idx(p_node);
    #else
        pg_pos = get_prog_pos(p_node->prog_id);
        if(INVALID_POS_NUM == pg_pos)
        {
            PRINTF("wrong prog pso!\n");
        }
    #endif

        if(as_info->param.as_handler!=NULL)
        {
            as_info->param.as_handler(ASM_TYPE_ADD_PROGRAM, pg_pos);
        }

        if ((AS_METHOD_TP == as_info->param.as_method) && (as_info->param.as_handler != NULL))
        {
        #ifdef MULTISTREAM_SUPPORT
            if (0 == as_info->isi_num)
            {
                as_info->param.as_handler(ASM_TYPE_PROGRESS, as_info->xponder.s_info.isid_idx * 99 / 1 + progress / 1);
            }
            else
            {
                as_info->param.as_handler(ASM_TYPE_PROGRESS, as_info->xponder.s_info.isid_idx * 99 \
                    / (as_info->isi_num+2) + progress / (as_info->isi_num+2));
            }
        #else
            as_info->param.as_handler(ASM_TYPE_PROGRESS, progress);
        #endif
        }
    }
    else// if(progress==AS_PROGRESS_NIT_VER_CHANGE)
    {
        if(as_info->param.as_handler!=NULL)
        {
            as_info->param.as_handler(ASM_TYPE_NIT_VER_CHANGE, *((UINT8 *)param));
        }
        return as_info->stop_scan_signal;
    }

    return as_info->stop_scan_signal;
}

/* According to input as info and tp info, set anntenna and nim of frontend */
UINT32 as_frontend_tuning(struct as_module_info *info, T_NODE *tp)
{
    //UINT32 time_start = 0;
    UINT32 time_out = 0;
    __MAYBE_UNUSED__ INT32 ret = 0;
    UINT8 lock_stat = 0;
    UINT8 nim_value = 0;
    union ft_xpond *xponder = NULL;
    struct ft_antenna *antenna = NULL;
#ifdef PLSN_SUPPORT
    UINT32 pls_num = 0;
    UINT8 pls_finish_flag = 0;
    UINT32 tm_start = 0;
#endif

    if ((NULL == info) || (NULL == tp))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return time_out;
    }
    xponder = &info->xponder;
    antenna = &info->antenna;

    //Set TP Info for DVB-S/C/T
    MEMSET(xponder, 0, sizeof(union ft_xpond));

    switch (tp->ft_type)
    {
        case FRONTEND_TYPE_S:
            // when signal quality is low, TP scan need wait a long time to lock it.
            time_out = AS_TUNE_TIMEOUT_S*40;
            xponder->s_info.type = tp->ft_type;
            xponder->s_info.tp_id = tp->tp_id;
            xponder->s_info.frq = tp->frq;
            xponder->s_info.sym = tp->sym;
            xponder->s_info.pol = tp->pol;
            xponder->s_info.fec_inner = tp->fec_inner;
        #ifdef MULTISTREAM_SUPPORT
            if (1 == info->param.ms_enable)
            {
                if ((1 == tp->t2_profile) && (0xFF != tp->plp_id) && (0xFE != tp->plp_id))
                {
                    xponder->s_info.change_type = 1;
                    xponder->s_info.isid = tp->plp_id;
                }
                else if(info->param.as_method == AS_METHOD_TP || info->param.as_method == AS_METHOD_NIT_TP || info->param.as_method == AS_METHOD_SAT_TP || \
				                info->param.as_method == AS_METHOD_NIT || info->param.as_method == AS_METHOD_MULTI_TP || info->param.as_method == AS_METHOD_MULTI_NIT)
				             {
				    	            xponder->s_info.change_type = 3; // set TP & get isid
				             }
                else
                {
                    xponder->s_info.change_type = 0;
                }
            }
            else
            {
                xponder->s_info.change_type = 0;
            }
        #endif
        #ifdef PLSN_SUPPORT
            xponder->s_info.super_scan = info->param.super_scan;
            xponder->s_info.pls_num = info->param.pls_num;
        #endif
            break;
        case FRONTEND_TYPE_C:
            time_out = AS_TUNE_TIMEOUT_C;
            xponder->c_info.type = tp->ft_type;
            xponder->c_info.tp_id = tp->tp_id;
            xponder->c_info.frq = tp->frq;
            xponder->c_info.sym = tp->sym;
        //FEC_inner store modulation in this case
            xponder->c_info.modulation = tp->fec_inner;
            break;
        case FRONTEND_TYPE_ISDBT:
        case FRONTEND_TYPE_T:
            time_out = AS_TUNE_TIMEOUT_T;
            xponder->t_info.type = tp->ft_type;
            //xponder->t_info.tp_id = tp->tp_id;
            xponder->t_info.frq = tp->frq;
        #ifdef COMBOUI
            xponder->t_info.band_width = tp->bandwidth*1000;
        #endif
            //channel scan
            xponder->t_info.usage_type = (UINT8)USAGE_TYPE_CHANSCAN;
            break;
        default:
            break;
    }

    if(FRONTEND_TYPE_S == tp->ft_type)
    {
        //Set Antenna
        frontend_set_antenna(as_info->nim, antenna, xponder, 1);
        //Set NIM
        frontend_set_nim(as_info->nim, antenna, xponder, 1);
    }
    else
    {
        frontend_set_nim(as_info->nim, NULL, xponder, 1);
    }

    if((FRONTEND_TYPE_T == tp->ft_type) || (FRONTEND_TYPE_ISDBT == tp->ft_type))
    {

    #ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        osal_task_sleep(500);
    #else
        osal_task_sleep(1000);
    #endif
        nim_get_lock(as_info->nim, &lock_stat);
        if(lock_stat)
        {
                //according info->nim device to get info
            if(NULL != info->nim->do_ioctl)
            {
            #ifdef COMBOUI
                info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_GUARD_INTERVAL, (UINT32)&nim_value);
                tp->guard_interval=nim_value;
                info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_FFT_MODE, (UINT32)&nim_value);
                tp->FFT = nim_value;
                info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_MODULATION, (UINT32)&nim_value);
                tp->modulation = nim_value;
                info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_SPECTRUM_INV, (UINT32)&nim_value);
                tp->inverse = nim_value;
            #endif
            }
            nim_get_fec(info->nim, &nim_value);
            tp->fec_inner = nim_value;
        #if defined(DVBT2_SUPPORT)
            tp->t2_signal = xponder->t_info.t2_signal;
            tp->plp_index = xponder->t_info.plp_index;
            tp->plp_id = xponder->t_info.plp_id;
            tp->t2_system_id = xponder->t_info.t2_system_id;
            tp->t2_profile = xponder->t_info.t2_profile;

            if ( db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp) != SUCCESS )
            {
                if ( add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp) != SUCCESS )
                {
                    return FALSE;
                }
            }
            else
            {
//              get_tp_by_id(t_node.tp_id, &temp_t_node);
                modify_tp(tp->tp_id, tp);
            }
            update_data();

            xponder->t_info.tp_id = tp->tp_id;
            info->atp.tp_id = tp->tp_id;
        #else
            ret = modify_tp(tp->tp_id, tp);
            if (DB_SUCCES == ret)
            {
                ret = update_data();
            }
        #endif
        }
    }
#ifdef PLSN_SUPPORT
    else if (FRONTEND_TYPE_S == tp->ft_type)
    {
        tm_start = osal_get_tick();
        do {
            //nim_get_lock(info->nim, &lock_stat);
            //if ((lock_stat) || (info->stop_scan_signal))
            nim_io_control(info->nim, NIM_DRIVER_GET_PLSN_FINISH_FLAG, &pls_finish_flag);
            if ((pls_finish_flag) || (info->stop_scan_signal))
            {
                break;
            }
            osal_task_sleep(1);
        }while ((INT32)(osal_get_tick() - tm_start) < 63000);

        nim_io_control(info->nim, NIM_DRIVER_GET_PLSN, &pls_num);
        if (pls_num != tp->pls_num)
        {
            tp->pls_num = pls_num;
            modify_tp(tp->tp_id, tp);
            update_data();
        }
    }
#endif
    return time_out;
}

/* Set as info(including tp id, timeout, and so on) for one specific tp */
void as_dvbs_preset_tuning(void *param)
{
    struct as_tuning_param *atp = NULL;
    T_NODE *tp = NULL;

    if ((NULL == param) || (NULL == as_info))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    atp = (struct as_tuning_param *)param;
    if (atp->finish)
    {
        return;
    }
    while ((INT32)atp->pos <= as_info->tp_nr)
    {
        tp = &as_info->xp;
        if (get_tp_by_id(as_info->tp_id[atp->pos], tp)!=SUCCESS)
        {
            break;
        }
        atp->nit_enable = as_info->tp_nit[atp->pos];
        atp->tp_id = as_info->tp_id[atp->pos];
        atp->pos++;
        atp->timeout = as_frontend_tuning(as_info, tp) + osal_get_tick();

        if ((atp->pos <= (UINT32)(as_info->tp_nr)) && (as_info->param.as_handler))
        {
           if((0 != atp->pos) && 
               ((as_info->param.as_method == AS_METHOD_SAT_TP) || (as_info->param.as_method == AS_METHOD_MULTI_TP)))
           {
               as_info->param.as_handler(ASM_TYPE_PROGRESS, 99*(atp->pos)/(as_info->tp_nr));
           }
	}
        return;
    }
    atp->finish = TRUE;
}

#if defined(DVBT2_SUPPORT)
BOOL as_next_pipe_tuning(struct as_module_info *info, T_NODE *tp)
{
    //Try to auto detect the next signal pipe within this channel,
    //such as the next PLP of (MPLP of DVB-T2), or the next priority
    //signal(Hierarchy of DVB-T).Before USAGE_TYPE_NEXT_PIPE_SCAN can be used,
    //you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN first.
    UINT8 pre_plp_index;
    //UINT8 lock_stat;
    //UINT8 nim_value;
    //It's shall keep the result returned by call
    //USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN.
    union ft_xpond *xponder = NULL;

    if ((NULL==info) || (NULL==tp))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }
    xponder = &info->xponder;
    if(info->stop_scan_signal)
    {
        return FALSE;
    }

    if(tp->ft_type != FRONTEND_TYPE_T)
    {
        return FALSE;
    }

    if( NUM_ZERO == xponder->t_info.t2_signal)  //DVB-T
    {
        //If need to support Hierarchy of DVB-T, you can tune to
        //the next priority signal at here.
        return FALSE;
    }

    //(DVBT2_SUPPORT)
    //It's DVB-T2 now.
    if(NUM_ZERO == xponder->t_info.plp_num)  //Unlocked.
    {
        return FALSE;
    }
    pre_plp_index = xponder->t_info.plp_index;
    xponder->t_info.usage_type = (UINT8)USAGE_TYPE_NEXT_PIPE_SCAN;
    frontend_set_nim(info->nim, NULL, xponder, 1);

    //No data PLP can be locked.
    if (pre_plp_index == xponder->t_info.plp_index)
    {
        return FALSE;
    }
    /*
    Bug fixed: Search a bad config MPLP signal, it maybe unstable, sometime
    a program be stored with the previous PLP setting.
    Do not need to check whether the signal is locked at here, because maybe
    it can be locked later. Otherwise if correct PLP can not be locked now,
    then the PLP setting dost not be updated, and keep the previous locked
    PLP setting.
//  nim_get_lock(info->nim, &lock_stat);
//  if( ! lock_stat)
//  {
//      libc_printf("%s() error: unlock\r\n", __FUNCTION__);
//  }
    */
    {

        tp->t2_signal = xponder->t_info.t2_signal;
        tp->plp_index = xponder->t_info.plp_index;
        tp->plp_id = xponder->t_info.plp_id;
        tp->t2_system_id = xponder->t_info.t2_system_id;
        tp->t2_profile = xponder->t_info.t2_profile;

        if (SUCCESS != db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp))
         {
            if ( SUCCESS !=  add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp))
            {
                return FALSE;
            }
        }
        else
        {
//          get_tp_by_id(t_node.tp_id, &temp_t_node);
            modify_tp(tp->tp_id, tp);
        }
        update_data();

        xponder->t_info.tp_id = tp->tp_id;
        info->atp.tp_id = tp->tp_id;
    }
    return TRUE;
}
#endif

/*DVBC blind scan in fact is tp scan by bandwith step*/
#if defined DVBC_COUNTRY_BAND_SUPPORT || defined COMBOUI
void as_dvbc_blind_tuning(void *param)
{
    union ft_xpond xponder = {{0}};
    UINT32 cnt_finish=0;
    UINT32 cnt_band=0;
    UINT32 cnt_ttl=0;
    UINT32 index=0;
    struct as_tuning_param *atp = NULL;

    if (NULL == param)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    atp = (struct as_tuning_param *)param;
    //If Blind Scan Already Finished, then Return
    if(dvbc_bs_freq_end)
    {
        atp->finish = TRUE;
    }

    if (TRUE == atp->finish)
    {
        return;
    }

    MEMSET(&xponder, 0, sizeof(xponder));
    xponder.common.type = as_info->param.as_frontend_type;
    atp->ft_type = as_info->param.as_frontend_type;
    if(FRONTEND_TYPE_C == atp->ft_type)
    {
        atp->ft.c_param.sym = as_info->param.ft[atp->ft_count].c_param.sym;
        atp->ft.c_param.constellation = as_info->param.ft[atp->ft_count].c_param.constellation;

        xponder.c_info.frq = atp->pos;
        xponder.c_info.sym = atp->ft.c_param.sym;
        xponder.c_info.modulation = atp->ft.c_param.constellation;
    }

    frontend_set_nim(as_info->nim, NULL, &xponder, 1);

    if(as_info->param.as_handler)
    {
        for(index = 0; index < atp->band_count; index++)
        {
            cnt_band = 1+ atp->band_group_param[index].end_ch_no - atp->band_group_param[index].start_ch_no;

             //if(as_info->param.ft_count > 1)
             cnt_band *= as_info->param.ft_count;
            cnt_ttl += cnt_band;

            if(atp->cur_band>index)
            {
                cnt_finish += cnt_band;
            }
        }

         if(NUM_ZERO != c_srh_tp_step)
         {
            if(atp->ft_count == (as_info->param.ft_count-COUNT_1))
            {
                cnt_finish += c_srh_tp_step;
            }
            else
            {
                cnt_finish += (c_srh_tp_step - (as_info->param.ft_count-1 - atp->ft_count));
            }
         }
        if(cnt_ttl)
        {
            as_info->param.as_handler(ASM_TYPE_PROGRESS, 100*cnt_finish/cnt_ttl);
        }
    }
    c_srh_tp_step++;

     if(as_info->param.ft_count > COUNT_1)
     {
        if(atp->cur_band < atp->band_count)
        {
            if(atp->ft_count < (as_info->param.ft_count-COUNT_1))
            {
                atp->ft_count++;
                atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;
                return;
            }
            else
            {
                atp->ft_count = 0;
            }
        }
    }

    while(atp->cur_band < atp->band_count)
    {
        if(atp->pos+atp->step <= atp->limit)
        {
            atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;
            atp->pos += atp->step;

            // only one frequency in this band
            if((atp->start == atp->limit) && (NUM_ZERO == atp->step))
            {
                atp->step = 1;
            }
            return;
        }
        atp->cur_band++;
        if(atp->cur_band < atp->band_count)
        {
            atp->start = atp->band_group_param[atp->cur_band].start_freq;
            atp->limit = atp->band_group_param[atp->cur_band].end_freq;
            atp->step = atp->band_group_param[atp->cur_band].freq_step;
            atp->pos = atp->start- atp->step;
            c_srh_tp_step = 0;
        }
    }

    atp->timeout += AS_TUNE_TIMEOUT_C; // for the last frequency
    dvbc_bs_freq_end = 1;

}
#else
/*DVBC and DVBT blind scan in fact is tp scan by bandwith step*/
void as_dvbc_blind_tuning(void *param)
{
    struct as_tuning_param *atp = NULL;
    union ft_xpond xpond;

    if (NULL == param)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }
    atp = (struct as_tuning_param *)param;
    //If Blind Scan Already Finished, then Return
    if (atp->finish)
    {
        return;
    }

    MEMSET(&xpond, 0, sizeof(xpond));
    xpond.common.type = atp->ft_type = as_info->param.as_frontend_type;
    if(FRONTEND_TYPE_C == atp->ft_type)
    {
        xpond.c_info.frq = atp->pos;
        xpond.c_info.sym = as_info->param.ft.c_param.sym;
        xpond.c_info.modulation = as_info->param.ft.c_param.constellation;
    }

    while(atp->pos <= atp->limit)
    {
        frontend_set_nim(as_info->nim, NULL, &xpond, 1);
        if((FRONTEND_TYPE_C==atp->ft_type) &&(as_info->param.as_handler))
        {
            as_info->param.as_handler(ASM_TYPE_INC_TRANSPONDER,atp->pos);
        }
        atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;
        atp->pos += atp->step;
        if(FRONTEND_TYPE_C==atp->ft_type)
        {
            if ((atp->pos>DVBC_FREQUENCY_VHF_HIGH)&&(atp->pos<DVBC_FREQUENCY_UHF_LOW))
            {
                if (atp->limit >= DVBC_FREQUENCY_UHF_LOW)
                {
                    atp->pos = DVBC_FREQUENCY_UHF_LOW;
                }
                else
                {
                    return;
                }
            }
        }

        if ((atp->pos < atp->limit) && (as_info->param.as_handler))
        {
            as_info->param.as_handler(ASM_TYPE_PROGRESS, 99*(atp->pos-atp->start)/(atp->limit-atp->start));
        }

        return;
    }
    atp->finish = TRUE;
}
#endif

/*DVBT blind scan in fact is tp scan by bandwith step*/
void as_dvbt_blind_tuning(void *param)
{
    struct as_tuning_param *atp = NULL;
    union ft_xpond *xponder = &as_info->xponder;
 #ifdef COMBOUI    
    UINT32 cnt_finish=0;
    UINT32 cnt_band=0;
    UINT32 cnt_ttl=0;
 #endif   
    UINT32 index=0;

    if (NULL == param)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    atp = (struct as_tuning_param *)param;
    //If Blind Scan Already Finished, then Return
    if(dvbt_bs_freq_end)
    {
        atp->finish = TRUE;
    }

    if (TRUE == atp->finish)
    {
        return;
    }

    //If no NIT tp is found, just move forward step by step
#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
    if (SEARCH_NIT_STATUS_NOT_REC == star_search_nit_staus)
#endif
    {
        MEMSET(xponder, 0, sizeof(union ft_xpond));
        xponder->common.type = as_info->param.as_frontend_type;
        atp->ft_type = as_info->param.as_frontend_type;
        if ((FRONTEND_TYPE_T == atp->ft_type)||(FRONTEND_TYPE_ISDBT == atp->ft_type))
        {
            xponder->t_info.type = atp->ft_type;
            //xponder->t_info.tp_id = tp->tp_id;
            xponder->t_info.frq = atp->pos;
            xponder->t_info.band_width = atp->step;
            xponder->t_info.usage_type = (UINT8)USAGE_TYPE_AUTOSCAN; //auto scan
        }
        frontend_set_nim(as_info->nim, NULL, xponder, 1);

        if(as_info->param.as_handler)
        {
        #ifdef COMBOUI
            as_info->param.as_handler(ASM_TYPE_SCANNING_INFO,1+t_srh_tp_step + \
            ((atp->cur_band&0x7f)<<8));

            for(index = 0; index < atp->band_count; index++)
            {
                cnt_band = (atp->band_group_param[index].end_freq - \
                    atp->band_group_param[index].start_freq)/atp->band_group_param[index].bandwidth;
                cnt_ttl += cnt_band;
                if(atp->cur_band>index)
                {
                    cnt_finish += cnt_band;
                }
            }
            cnt_finish += t_srh_tp_step;
            if(cnt_ttl)
            {
                as_info->param.as_handler(ASM_TYPE_PROGRESS, 100*cnt_finish/cnt_ttl);
            }
        #endif
        }
        t_srh_tp_step++;

    #ifdef COMBOUI
        while(atp->cur_band < atp->band_count)
        {
            atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_T;
            if(atp->pos+atp->step < atp->limit)
            {
                atp->pos += atp->step;
                return;
            }
            atp->cur_band++;
            if(atp->cur_band < atp->band_count)
            {
                atp->start = atp->band_group_param[atp->cur_band].start_freq + \
                    atp->band_group_param[atp->cur_band].bandwidth/2;
                atp->limit = atp->band_group_param[atp->cur_band].end_freq + \
                    atp->band_group_param[atp->cur_band].bandwidth/2;
                atp->step = atp->band_group_param[atp->cur_band].bandwidth;
                atp->pos = atp->start - atp->step;
                t_srh_tp_step = 0;
            }
        }
    #endif

    /*
    //if ((atp->pos < atp->limit) && (as_info->param.as_handler))
    //{
    //      as_info->param.as_handler(ASM_TYPE_PROGRESS, \
    //          99*(atp->pos-atp->start)/(atp->limit-atp->start));
    //}
    */
    }
#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
    else//if found NIT table during blind scan, jump to NIT TP list
    {
        if (SEARCH_NIT_STATUS_SEARCH != star_search_nit_staus)
        {
            star_search_nit_staus = SEARCH_NIT_STATUS_SEARCH;
            atp->pos = 0;
        }

        /*
        //20110909, fix blind scan issue. for atp->pos will ++ \
        // in as_dvbs_preset_tuning.
        //so after as_dvbs_preset_tuning, atp->pos is next one, not current one!
        */
        if(atp->pos < as_info->tp_nr)
        {
            as_info->param.as_handler(ASM_TYPE_PROGRESS, (100*atp->pos)/(as_info->tp_nr));
        }

        as_dvbs_preset_tuning(atp);

        /*
        //20110909, fix blind scan issue.
        //as_info->param.as_handler(ASM_TYPE_PROGRESS, \
        //(100*atp->pos)/(as_info->tp_nr));
        */

        return;

    }
#endif
    dvbt_bs_freq_end = 1;
}

BOOL dvbt_init_search_param(struct as_service_param *param, struct as_tuning_param *atp)
{
    if ((NULL == param) || (NULL == atp))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }
#ifdef COMBOUI
    if((param->scan_cnt<COUNT_1)|| (param->scan_cnt>MAX_BAND_COUNT))
    {
        atp->finish = TRUE;
        return FALSE;
    }
    atp->ft_type = param->as_frontend_type;

    atp->start = param->band_group_param[0].start_freq + param->band_group_param[0].bandwidth/2;
    atp->limit = param->band_group_param[0].end_freq + param->band_group_param[0].bandwidth/2;
    atp->step = param->band_group_param[0].bandwidth;
    atp->cur_band=0;

    atp->pos = atp->start;
    atp->band_count = param->scan_cnt;

    atp->band_group_param = param->band_group_param;
#endif

    atp->finish = FALSE;
    t_srh_tp_step = 0;
    dvbt_bs_freq_end = 0;
    return TRUE;
}
///add for poland and france_hd book to get signal of tp
#if(defined(POLAND_SPEC_SUPPORT)||defined(SUPPORT_FRANCE_HD) \
    || defined(SELECT_HIGHER_QUALITY_SERVICE))
void get_signal_of_tp(T_NODE *tp)
{
//for saving intensity&quality when chan scan
    UINT8 agc_buffer[10] = {0};
    UINT8 snr_buffer[10] = {0};
    UINT16 i = 0;
    UINT16 agc_sigma=0;
    UINT16 snr_sigma=0;
    UINT8 nim_value1=0;
    UINT8 nim_value2=0;

    if (tp)
    {
        for(i=0;i<10;i++)
        {
            as_info->nim->get_agc(as_info->nim, (UINT8*)&nim_value1);
            as_info->nim->get_snr(as_info->nim, (UINT8*)&nim_value2);

            agc_buffer[i]=nim_value1;
            snr_buffer[i]=nim_value2; 

            osal_task_sleep(15);
            agc_sigma+=agc_buffer[i];
            snr_sigma+=snr_buffer[i];
        }

        tp->intensity= agc_sigma/10;
        tp->quality= snr_sigma/10;
        modify_tp(tp->tp_id, tp);
        update_data();
    }
}
#endif

/****dvbs blind scan**********************************************/

#ifdef MULTISTREAM_SUPPORT
BOOL dvbs_tp_duplicate_check(UINT32 freq, UINT32 sym, UINT32 polar)
{
    static UINT32 last_freq=0, last_sym=0, last_polar=0;
    BOOL ret;
#ifdef MULTISTREAM_SUPPORT
	static UINT32 last_isid = 0;
	struct nim_dvbs_isid isid_info;
	
	MEMSET(&isid_info, 0, sizeof(struct nim_dvbs_isid));
	nim_io_control(as_info->nim,NIM_ACM_MODE_GET_ISID, &isid_info);

#endif
    
    if ((last_polar == polar)
        &&(sym-last_sym<SYM_DIST)&&(last_sym-sym<SYM_DIST)
        &&(freq-last_freq<FREQ_DIST+(last_sym>>SYM_SHIFT))
        &&(last_freq-freq<FREQ_DIST+(sym>>SYM_SHIFT))
#ifdef MULTISTREAM_SUPPORT        
        &&(last_isid == isid_info.isid_write)
#endif        
        )
    {
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }

    last_freq = freq;
    last_sym = sym;
    last_polar = polar;
	
#ifdef MULTISTREAM_SUPPORT	
	if(isid_info.isid_num > 0)
	{
		last_isid = isid_info.isid_write;
	}
	else
	{
		last_isid = 0;
	}
#endif	
    return ret;
}

#endif


//dvbs blind scan nim api callback function.
INT32 dvbs_as_callback(UINT8 signal, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
    T_NODE *tp = NULL;
    UINT16 sat_id = 0xFFFF;
    INT32 ret = 0;
    as_update_event_t handle = NULL;
#ifdef NIM_S3501_ASCAN_TOOLS
    struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");
#endif
    UINT8 progress = 0;
    struct nim_device *nim_dev = NULL;
    UINT8 l_fec = 0;
#ifdef MULTISTREAM_SUPPORT
    struct nim_c3505_bb_header_info bbh_info;
    struct nim_dvbs_isid isid_info;
#endif
#ifdef PLSN_SUPPORT
    UINT32 pls_num = 0;
#endif

    if ((NULL == as_info) || (INVALID_POLAR == polar))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return 2;
    }
    handle = as_info->param.as_handler;

    if(as_info->stop_scan_signal)
    {
        return 2;
    }
    else if((AS_SGN_UNLOCKED == signal) || (AS_SGN_LOCKED == signal))
    {
        /* Patches for lib_nim Out-Band Xponder Mis-Report */
        //Romn Fix BUG50807 BUG51239
        if ( ((m_crnum < COUNT_0) && ((freq < as_info->param.as_from) || (freq > as_info->param.as_to))))
        {
            return 0;
        }
        else if ((AS_METHOD_FFT == as_info->param.as_method) || (AS_METHOD_MULTI_FFT == as_info->param.as_method))
        {
            //Calculate and Show Scan Progress
            progress = (UINT8)dvbs_calculate_progress(as_info, freq, polar, m_crnum);
            if(handle != NULL)
            {
                handle(ASM_TYPE_PROGRESS, progress);
            }
        }
    }

    switch(signal)
    {
        case AS_SGN_LOCKED:
            //Check whether New Freq Point, Sym Rate, and Polarity is
            //same to the Old TP or not
            if (FALSE == dvbs_tp_duplicate_check(freq, sym, polar))
            {
                break;
            }

            //Construct one New TP according to Locked Freq and Sym
            tp = &as_info->xp;
            MEMSET(tp, 0, sizeof(T_NODE));
            dvbs_convert_freq(as_info, tp, freq);
            tp->sym = sym;
            tp->sat_id = as_info->cur_sat.sat_id;
            sat_id = as_info->cur_sat.sat_id;
            tp->ft_type = FRONTEND_TYPE_S;

            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            if(nim_dev != NULL)
            {
                nim_get_fec(nim_dev, &l_fec);
    			//libc_printf("nim_get_FEC FEC:%d\n",FEC);
    			if(l_fec != tp->fec_inner)
    			{
    				tp->fec_inner = l_fec;
    			}
            }
        #ifdef MULTISTREAM_SUPPORT
			MEMSET(&isid_info, 0, sizeof(struct nim_dvbs_isid));
			nim_io_control(as_info->nim,NIM_ACM_MODE_GET_ISID, &isid_info);
			if(isid_info.isid_num > 0)
			{
				tp->t2_profile = 1;
				tp->plp_id = isid_info.isid_write;
			}
		#endif

        #ifdef PLSN_SUPPORT
            nim_io_control(as_info->nim, NIM_DRIVER_GET_PLSN, &pls_num);
            tp->pls_num = pls_num;
        #endif

            AS_DUMP_TP(tp);
            if(db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp)!=SUCCESS)
            {
                ret = add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp);
                if ((DBERR_FLASH_FULL==ret) || (DBERR_MAX_LIMIT ==ret))
                {
                    return 1;
                }
            }
            //Process by Upper Layer
            if(handle != NULL)
            {
                handle(ASM_TYPE_ADD_TRANSPONDER, tp->tp_id);
            }

        #ifdef NIM_S3501_ASCAN_TOOLS
        if (nim_io_control(nim_dev, NIM_ASCAN_GET_SCAN_TP_ONLY, 0))
        {
            return 0;
        }
        #endif
        
	#ifdef MULTISTREAM_SUPPORT
		nim_io_control(nim_dev,NIM_ACM_MODE_GET_BBHEADER_INFO, &bbh_info);
		if(3 != bbh_info.stream_mode)
		{
			return 0;
		}
	#endif	

            /* start searching si information of all programs
                in this frequency point
            */
            psi_module_init(as_info->dmx, as_info->nim,(void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
      #ifdef COMBOUI
            psi_set_search_ftype(as_info->param.as_frontend_type);
            ret = psi_search_start(sat_id,tp->tp_id, as_info->param.as_prog_attr,\
                as_info->param.as_p_add_cfg, psi_callback,NULL, NULL);
            //notify ap that finished search a TP.
            if(handle != NULL)
            {
            handle(ASM_TYPE_ADD_TRANSPONDER, 0);
            }
            if (SI_STOFULL == ret)
            {
                return 1;
            }
      #else
            if (SI_STOFULL == psi_search_start(sat_id,tp->tp_id, as_info->param.as_prog_attr, \
                as_info->param.as_p_add_cfg, psi_callback,NULL, NULL))
            {
                return 1;
            }
      #endif
        break;

        case AS_SGN_STOP:
            AS_PRINTF(">>>>>>>>>>>>>>STOP!!!<<<<<<<<<<<<<<<<<<\n");
            as_info->stop_scan_signal=1;
        break;

        case AS_SGN_CRNUM:
            m_crnum = freq;
        break;

        case AS_SGN_INIT:
            m_crnum = -1;
            //Initialize one TP
            if (!dvbs_tp_duplicate_check(0, 0, 0))
            {
                AS_PRINTF("check error!\n");
            }
            break;
    //>>> Unicable begin
        case AS_SET_UNICABLE:
            tp = &as_info->xp;
            MEMSET(tp, 0, sizeof(T_NODE));
            dvbs_convert_freq(as_info, tp, freq);
            as_info->xponder.s_info.frq = tp->frq;
            as_info->xponder.s_info.pol = as_info->polar;
            frontend_set_unicable(as_info->nim,&as_info->antenna, &as_info->xponder);
            break;
    //<<< Unicable end
        default:
            break;
        }

        return (as_info->stop_scan_signal)? 2: 0;
}

INT32 as_dvbs_blind_scan(struct ft_antenna *antenna)
{
    UINT32 from = as_info->param.as_from;
    UINT32 to = as_info->param.as_to;
    UINT32 loop = 0;
    INT32 ret = RET_FAILURE;
#ifdef NIM_S3501_ASCAN_TOOLS
    struct nim_in_device *nim_dev = (struct nim_in_device *)dev_get_by_name("NIM_S3501_0");
#endif

    struct NIM_AUTO_SCAN *as_para = &as_info->nim_para;

    if (NULL == antenna)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return ret;
    }
    MEMSET(&(as_info->xponder),0,sizeof(union ft_xpond));
    //as_info->polar: 0-H;1-V. antenna->pol: 0-auto;1-H;2-V
    //for single lnb,scan IF from 950~2150

    switch (antenna->lnb_type)
    {
        case LNB_CTRL_STD:
            //use forced H or V voltage,scan 1 time
            if(as_info->voltage_ctrl)
            {
                as_info->polar = antenna->pol - 1;
                loop = antenna->pol;
            }
            //use both H and V,scan 2 times
            else
            {
                as_info->polar = 0;
                loop = 2;
            }
            break;
        case LNB_CTRL_POL:
            if(as_info->voltage_ctrl)
            {
                as_info->polar = antenna->pol - 1;
                loop = antenna->pol;
                //forced H voltage, scan IF from 950~1550 1 time
                if(0 == as_info->polar)
                {
                    to = 1550;
                }
                //Forced V Voltage, Scan IF 1551~2150 1 time
                else if(1 == as_info->polar)
                {
                    from = 1551;
                }
            }
            else
            {
                as_info->polar = 0;
                loop = 1;
            }
            break;
        case LNB_CTRL_22K:
        case LNB_CTRL_UNICABLE:
            if(as_info->voltage_ctrl)
            {
                as_info->polar = antenna->pol - 1;
                loop = antenna->pol;
            }
            else
            {
                as_info->polar = 0;
                loop = 2;
            }
            //firstly,scan low lnb to 22k_edge tp frq with 22k off
            antenna->k22 = 0;
            from = as_info->param.as_from;
            if(antenna->lnb_low < antenna->lnb_high)
            {
                to = KU_LNB_22K_EDGE_FREQUENCY - antenna->lnb_low;
            }
            else
            {
                to = KU_LNB_22K_EDGE_FREQUENCY - antenna->lnb_high;
            }
            break;
        default:
            break;
    }

    //scan loop
    while(!as_info->stop_scan_signal)
    {
        while ((as_info->polar)<loop)
        {
            //Fix bug37335,universal lnb 4th times
            //as_info->param.as_from = from;
            //as_info->param.as_to = to;

            //scan lost 1950-2200 tp-s
            if(as_info->stop_scan_signal)
            {
                break;
            }
            as_info->xponder.s_info.pol = as_info->polar;
            frontend_set_antenna(as_info->nim, antenna, &as_info->xponder, 1);
            osal_task_sleep(AS_TUNE_TIMEOUT_S);
            // make up the autoscan parameter list
            as_para->sfreq = from;
            as_para->efreq = to;
            as_para->callback = dvbs_as_callback;
         #ifdef MULTISTREAM_SUPPORT
            as_para->scan_type = as_info->param.scan_type;
         #endif
         #ifdef NIM_S3501_ASCAN_TOOLS
         nim_io_control(nim_dev, NIM_ASCAN_SET_INFO, (UINT32)as_info);
         #endif
         #ifdef PLSN_SUPPORT
            as_para->super_scan = as_info->param.super_scan;
         #endif
         
		         // Paladin for iq swap! 06/28/2017
		        if (antenna->lnb_high < 6000)
		            as_para->freq_band = 1;
		        else
		            as_para->freq_band = 2;
		        
            ret = nim_ioctl_ext(as_info->nim, NIM_DRIVER_AUTO_SCAN, as_para);
            if(ret!=RET_SUCCESS)
            {
                break;
            }
            ret = dvbs_as_callback(AS_SGN_INIT, 0, 0, 0, 0);

            as_info->polar++;
        }

         /* Unicable */
        if ((LNB_CTRL_UNICABLE == antenna->lnb_type) && (!as_info->dual_lnb))
        {
            break;
        }
        else if(ret||(1==(antenna->k22))
            ||((LNB_CTRL_22K != (antenna->lnb_type))
            &&(LNB_CTRL_UNICABLE != (antenna->lnb_type))))
        {
            break;
        }
        else
        {
            //Second, Scan High LNB from 22k_edge to 2150 TP Frequency
            //with 22k On
            antenna->k22 = 1;
            //switch to high band,turn on 22K
            as_info->xponder.s_info.frq = KU_LNB_22K_EDGE_FREQUENCY+1;
            if(antenna->lnb_low < antenna->lnb_high)
            {
                from = KU_LNB_22K_EDGE_FREQUENCY - antenna->lnb_high;
            }
            else
            {
                from = KU_LNB_22K_EDGE_FREQUENCY - antenna->lnb_low;
            }
            to = as_info->param.as_to;
            if(as_info->voltage_ctrl)
            {
                loop = antenna->pol;
                as_info->polar = antenna->pol - 1;
            }
            else
            {
                as_info->polar = 0;
                loop = 2;
            }
        }
    }

    return ret;
}


void as_set_as_info(struct as_module_info *info)
{
    if(NULL != info)//?
    {
        as_info = info;
    }
}


