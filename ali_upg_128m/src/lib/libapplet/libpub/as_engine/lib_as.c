/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_as.c
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
#include <api/libc/printf.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/as_module.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libtsi/sie.h>

#include "../as_dvbs.h"
#include "../as_dvbc.h"
#include "../as_dvbt.h"
#include "lib_as_inner.h"


/*******************************************************
* macro define
********************************************************/
#define INVALID_SAT_ID  0xffff

/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/

#ifdef MULTI_CAS 
extern BOOL mcas_xpond_stoped;
#endif

#if(defined ITALY_HD_BOOK_SUPPORT ||defined     POLAND_SPEC_SUPPORT)    
typedef  BOOL (*node_view_init)();
static node_view_init node_view_init_callback = NULL;
#endif

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
// 0: not rec, 1: rec, 2: nit search started
UINT32 star_search_nit_staus = SEARCH_NIT_STATUS_NOT_REC;
#endif

#ifdef NIT_AUTO_SEARCH_SUPPORT
BOOL b_found_nit_tp = FALSE;
#endif

UINT32 c_srh_tp_step = 0; //dvbc search tp step
UINT32 dvbc_bs_freq_end = 0; //dvbc blind search freq end...

#ifndef __MM_VBV_START_ADDR
void *g_as_buf = NULL;
#endif

static struct as_module_info *as_info = NULL;

/*******dvbs blind scan*****************************************/

#if defined(MULTISTREAM_SUPPORT)
static UINT32 as_next_isi_tuning(struct as_module_info *info, T_NODE *tp)
{
    // Try to auto detect the next signal pipe within this channel,
    // such as the next PLP of (MPLP of DVT-T2), or the next priority signal(Hierarchy of DVB-T).
    // Before USAGE_TYPE_NEXT_PIPE_SCAN can be used, you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANGE first.

    UINT8 pre_isid_index;
    UINT8 lock_stat;
    UINT8 nim_value;
    union ft_xpond *xponder = &info->xponder;
    as_update_event_t update_event = as_info->param.as_handler;

    if (1 == info->stop_scan_signal)
    {
        return FALSE;
    }

    if (FRONTEND_TYPE_S != tp->ft_type)
    {
        return FALSE;
    }

    // DVBS2_SUPPORT
    // It's DVB-S2 now
    if (0 == info->isi_num) // Unlocked or not a multistream TP.
    {
        return FALSE;
    }

    while(info->isi_num > xponder->s_info.isid_idx)
    {
	xponder->s_info.change_type = 2;
	xponder->s_info.isid = info->tp_isid[xponder->s_info.isid_idx];

#ifdef PLSN_SUPPORT
    xponder->s_info.super_scan = info->param.super_scan;
    xponder->s_info.pls_num = tp->pls_num;//info->param.pls_num;
#endif

    frontend_set_nim(info->nim, NULL, xponder, 1);

    tp->plp_index = xponder->s_info.isid_idx;
    tp->plp_id = tp->plp_index < info->isi_num?info->tp_isid[xponder->s_info.isid_idx]:0xFF; // 0xFF is invalid isid
    //tp->t2_system_id = xponder->t_info.t2_system_id;
    tp->t2_profile = 1;//xponder->t_info.t2_profile;
		  
	pre_isid_index = xponder->s_info.isid_idx++;

    if ( db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp) != SUCCESS )
    {
        //libc_printf("%s(), add TP tp_id: %d, isid:%d \n", __FUNCTION__, tp->tp_id,xponder->s_info.isid);
    	if ( add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp) != SUCCESS )
    	{
            return FALSE;
        }
			//libc_printf("\t\t real tp_id:%d\n", tp->tp_id);
    }
    else  
    {
//      get_tp_by_id(t_node.tp_id, &temp_t_node);
	    modify_tp(tp->tp_id, tp);
			update_data();
			if(info->param.as_method == AS_METHOD_SAT_TP || info->param.as_method == AS_METHOD_NIT \
			|| info->param.as_method == AS_METHOD_MULTI_TP || info->param.as_method == AS_METHOD_MULTI_NIT)
			{
			  continue;
			}
     }
	update_data();
        
	xponder->s_info.tp_id = tp->tp_id;
    info->atp.tp_id = tp->tp_id;
    return TRUE;
  }
  return FALSE;	
}

#endif


static BOOL dvbt_alloc_transponder(struct as_module_info *info, struct as_tuning_param *atp, T_NODE *tp, UINT16 sat_id)
{
    INT32 ret = RET_FAILURE;
    UINT32 freq = 0;
    INT32 search_ret = 0;
    UINT8 nim_value = 0;
    union ft_xpond *xpond = NULL;

    if ((NULL == info) || (NULL == atp) || (NULL == tp)||(INVALID_SAT_ID == sat_id))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }

    nim_value = 0;
    xpond = &(info->xponder);
    MEMSET(tp, 0, sizeof(T_NODE));
    tp->sat_id = sat_id;
    tp->ft_type = FRONTEND_TYPE_T;
    if(nim_get_freq(info->nim, &freq) != SUCCESS)
    {
        return FALSE;
    }

#if defined(DVBT2_SUPPORT)
    tp->priority = xpond->t_info.priority;
    tp->t2_signal = xpond->t_info.t2_signal;
    tp->plp_index = xpond->t_info.plp_index;
    tp->plp_id = xpond->t_info.plp_id;
    tp->t2_profile = xpond->t_info.t2_profile;
#endif

    //If offset is less than 0.4MHz,delete the offset,or add up to 1MHz.
    //So we just show integer(MHz).
    if(freq%FREQ_DIV_NUM<=FREQ_OFFSET)
    {
        tp->frq = freq/FREQ_DIV_NUM*FREQ_DIV_NUM;
    }
    else
    {
        tp->frq = freq/FREQ_DIV_NUM*FREQ_DIV_NUM+FREQ_DIV_NUM;
    }
    tp->frq = freq;
#ifdef COMBOUI
#if 0
    tp->bandwidth = (atp->step)/1000; // atp param not correct this time.
#else
    tp->bandwidth = (xpond->t_info.band_width)/1000;
#endif
    //according info->nim device to get info
    //nim_get_gi(info->nim, &nim_value);
    if(NULL != info->nim->do_ioctl)
    {
        info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_GUARD_INTERVAL, (UINT32)&nim_value);
        tp->guard_interval=nim_value;
        info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_FFT_MODE, (UINT32)&nim_value);
        tp->FFT = nim_value;
        info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_MODULATION, (UINT32)&nim_value);
        tp->modulation = nim_value;
        info->nim->do_ioctl(info->nim, NIM_DRIVER_GET_SPECTRUM_INV, (UINT32)&nim_value);
        tp->inverse = nim_value;
    }

    nim_get_fec(info->nim, &nim_value);
    tp->fec_inner = nim_value;
    tp->band_type = (0x80==(atp->band_group_param[atp->cur_band].band_type & 0x80))?1:0;
#endif
    search_ret = db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp);
    if ( search_ret != SUCCESS)
     {
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        tp->pat_version = 0xff;
#endif
        ret = add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp);
        if (ret!=RET_SUCCESS)
        {
            return FALSE;
        }
    }
#if(defined(POLAND_SPEC_SUPPORT)||defined(SUPPORT_FRANCE_HD))
    get_signal_of_tp(tp);
#endif
    atp->tp_id = tp->tp_id;
    return TRUE;
}

//for tuner to select the nim device
static void as_select_nim(S_NODE *snode, UINT8 front_end_type)
{
    UINT8 nim_id = 0;         //0 or 1
    UINT8 i = 0;
    int ts_id = 0;
    struct nim_device *nim = NULL;

    if(FRONTEND_TYPE_S == front_end_type)
    {
        if (snode)
        {
            if(FLAG_TRUE == snode->tuner1_valid)
            {
                nim_id = 0;
                ts_id = ts_route_get_nim_tsiid(nim_id);
                tsi_select(TSI_DMX_0, ts_id);
                as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
            }
            else if(1 == snode->tuner2_valid)
            {
                nim_id = 1;
                tsi_select(TSI_DMX_0, ts_route_get_nim_tsiid(nim_id));
                as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM,nim_id);
            }
        }
    }
    else
    {
        for(i=0; i<2; i++)
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, i);
            if(NULL != nim)
            {
                if(dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM) == front_end_type)
                {
                    tsi_select(TSI_DMX_0, ts_route_get_nim_tsiid(i));
                    as_info->nim = nim;
                    return;
                }
            }
        }

        nim_id = 0;
        as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    }
}

static INT32 as_enum_scan(UINT16 sat_id, UINT16 type, psi_tuning_t tuning_fn)
{
    BOOL db_ret = TRUE;
    INT32 ret = 0;
    INT32 func_ret = 0;
    UINT32 search_scheme = 0;
    UINT8 lock_stat = 0;
    as_update_event_t update_event = as_info->param.as_handler;
#ifdef MULTISTREAM_SUPPORT
    struct nim_dvbs_isid ms_isi_info;
    struct nim_c3505_bb_header_info bbh_info;
    UINT32 timeout = 0;
#endif

    struct as_tuning_param *atp = &as_info->atp;

    if(INVALID_SAT_ID == sat_id)
    {
        return RET_FAILURE;
    }

    AS_PRINTF("start at %d\n", osal_get_tick());
    if(update_event != NULL)
    {
        update_event(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANSTART+1);
    }

    //TP Scan Loop
    while((FALSE == atp->finish) && (0 == ret))
    {
        //The frontend turn to designated channel.
        if(tuning_fn != NULL)
        {
            tuning_fn(atp);
        }
        if(atp->finish || as_info->stop_scan_signal) //AutoScan finish search all TP.
        {
            break;
        }
        //Set Search Scheme, that is whether Search NIT or not
        search_scheme = as_info->param.as_prog_attr;
        if (atp->nit_enable)
        {
            search_scheme |= P_SEARCH_NIT;
        }

        AS_PRINTF("id=%d, t_nr=%d\n",atp->tp_id, as_info->tp_nr);
        lock_stat = 0;

    //Show New TP for DVB-S by Upper Layer
#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
        if (((tuning_fn == as_dvbs_preset_tuning)||(SEARCH_NIT_STATUS_SEARCH ==star_search_nit_staus))&&(update_event))
#else
        if ((tuning_fn == as_dvbs_preset_tuning) && (update_event))
#endif
        {
            update_event(ASM_TYPE_ADD_TRANSPONDER, atp->tp_id);
        }

        //Try to Lock Signal
        do
        {
            nim_get_lock(as_info->nim, &lock_stat);
            if((lock_stat) || (as_info->stop_scan_signal))
            {
                break;
            }
            osal_task_sleep(1);
        } 
        while((INT32)osal_get_tick() < (INT32)atp->timeout);
        
#if 0
        // fix a bug of 3501B tp scan, it need more time.
        //(3501 SSI to SPI 8bit mode, for some coderate).
        if(FRONTEND_TYPE_S == atp->ft_type)
        {
            osal_task_sleep(1200);
        }
#endif

        AS_PRINTF("lock stat = %d\n", lock_stat);    
    #ifdef MULTISTREAM_SUPPORT
		if(lock_stat && (3 == as_info->xponder.s_info.change_type))
		{
            MEMSET(&ms_isi_info, 0, sizeof(struct nim_dvbs_isid));
			ms_isi_info.get_finish_flag = 0;
			while(!ms_isi_info.get_finish_flag && (timeout < 4000))
			{
				nim_io_control(as_info->nim,NIM_ACM_MODE_GET_ISID, &ms_isi_info);
				osal_task_sleep(1);
				timeout++;
			}
			//if(ms_isi_info.get_finish_flag)
            if(ms_isi_info.isid_num > 0)
            {
                /*libc_printf("@@  %d isid got:", ms_isi_info.isid_num);
                UINT8 i = 0;
                for(i=0; i < ms_isi_info.isid_num; i++)
                    libc_printf(" %02d", ms_isi_info.isid_read[i]);
                libc_printf("\n");*/
                as_info->isi_num = ms_isi_info.isid_num;
                MEMCPY(as_info->tp_isid, ms_isi_info.isid_read, ms_isi_info.isid_num*sizeof(ms_isi_info.isid_read[0]));
            }
			else
		    {
		       as_info->isi_num = 0; 
		    }
        }
    #endif    
        if (!as_info->stop_scan_signal)
        {
            //If Lock Signal, then Search Prog in this TP
            if (lock_stat)
            {
                //Scan from Start Frq to End Frq for DVB-C and DVB-T
                if (ATP_TYPE_STEP == type)
                {
                    //For DVB-C, Set TP Info according to as_info
                    if(FRONTEND_TYPE_C == as_info->param.as_frontend_type)
                    {
                        db_ret = dvbc_alloc_transponder(as_info, atp, &as_info->xp, sat_id);
                    }
                    else if ((FRONTEND_TYPE_T == as_info->param.as_frontend_type) 
		         || (FRONTEND_TYPE_ISDBT == as_info->param.as_frontend_type))
                    {
                        db_ret = dvbt_alloc_transponder(as_info, atp, &as_info->xp, sat_id);
                    }

                    if (db_ret != TRUE)
                    {
                        ret = 1;
                        break;
                    }
                    //Show New TP for DVB-C and DVB-T by Upper Layer
                    if(update_event)
                    {
                        //notify ap locked
                    #ifdef PUB_PORTING_PUB29
                        update_event(ASM_TYPE_ADD_TRANSPONDER, 0xFFFE);
                    #else
                        //notify ap show tp
                        update_event(ASM_TYPE_ADD_TRANSPONDER, atp->tp_id);
                    #endif
                    }
                }
                else if (ATP_TYPE_DBASE == type)
                {
                #if(defined(POLAND_SPEC_SUPPORT)||defined(SUPPORT_FRANCE_HD))
                    if(as_info->param.as_frontend_type == FRONTEND_TYPE_T)
                    {
                        get_signal_of_tp(&as_info->xp);
                    }
                #endif
                }                
 
            #if defined(DVBT2_SUPPORT) || defined(MULTISTREAM_SUPPORT)
               do
                {
            #endif
                
                #ifdef MULTISTREAM_SUPPORT
                    if((3 == as_info->xponder.s_info.change_type) && (as_info->isi_num > 0))  // don't check 1st time, because it just get isid info.
                    {
                        continue;
                    }
                    bbh_info.stream_mode = 0;
                   timeout = 0;
				         	 ms_isi_info.get_finish_flag = 0;
					         while(!ms_isi_info.get_finish_flag && (timeout < 4000))
					         {
						          nim_io_control(as_info->nim,NIM_ACM_MODE_GET_ISID, &ms_isi_info);
						          osal_task_sleep(1);
					            timeout++;
					         }
        		      nim_io_control(as_info->nim,NIM_ACM_MODE_GET_BBHEADER_INFO, &bbh_info); 
	
                    if(3 != bbh_info.stream_mode) // only check TS stream_mode=3
                    {
                        continue;
                    }
                #endif  
                    //Prog Search in this TP
                    psi_module_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
                #ifdef COMBOUI
                    (INT32)psi_set_search_ftype(as_info->param.as_frontend_type);
                #endif
                    func_ret = psi_search_start(sat_id, atp->tp_id,
                        search_scheme, as_info->param.as_p_add_cfg, psi_callback, NULL, NULL);
                    if (SI_STOFULL == func_ret)
                    {
                        ret= 1;
                    }
                #ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
                    else if(SI_EXIT_STAR == func_ret)
                    {
                        if((ATP_TYPE_DBASE == type))
                        {
                            as_info->stop_scan_signal = 1;
                        }
                    }
                #endif
                
                #ifdef SELECT_HIGHER_QUALITY_SERVICE
                if(as_info->param.as_frontend_type == FRONTEND_TYPE_T)
                {
                    T_NODE cur_node;
                    get_tp_by_id(atp->tp_id,&cur_node);
                    get_signal_of_tp(&cur_node);
                }
                #endif
                
        #if defined(DVBT2_SUPPORT)
                }while( as_next_pipe_tuning(as_info, &as_info->xp));
        #elif defined(MULTISTREAM_SUPPORT)
                }while (as_next_isi_tuning(as_info, &as_info->xp));
        #endif
                //notify ap that finished search a TP.
                update_event(ASM_TYPE_ADD_TRANSPONDER, 0);
            }
            //Not Lock Signal, then Switch to next TP
            else
            {
            #ifdef PUB_PORTING_PUB29
                if(update_event)
                {
                    //notify ap unlocked
                    update_event(ASM_TYPE_ADD_TRANSPONDER, 0xFFFF);
                }
            #endif
            #ifdef NIT_AUTO_SEARCH_SUPPORT
                if((tuning_fn== as_dvbc_blind_tuning) && b_found_nit_tp)
                {
            // because doing auto scan,so atp->pos must > AS_MIN_CAB_FREQUENCY
            //and we need to change to nit search.
                    if(atp->pos > AS_MIN_CAB_FREQUENCY)
                    {
                        atp->pos = 0;
                    }
                    tuning_fn = as_dvbs_preset_tuning;
                }
            #endif
#if 0
                tuning_fn(atp);
#endif
            }
        }
        if ((as_info->stop_scan_signal) || ret)
        {
            break;
        }
    }
#ifdef NIT_AUTO_SEARCH_SUPPORT
    b_found_nit_tp = FALSE; // nit search done
#endif
    AS_PRINTF("end at %d\n", osal_get_tick());
    if (ret)
    {
        return RET_FAILURE;
    }
    else
    {
        return RET_SUCCESS;
    }
}

#ifdef AUTO_OTA
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
/*******************************************************************************
*   Funtion name : as_scan_ssu_update()
*   Description:
*       1. scan ssu info in all freq.
*   Time: 20080418
*******************************************************************************/
static void as_scan_ssu_update()
{
#if(defined(DVBT_SUPPORT)||(defined(ISDBT_SUPPORT)))
    si_scan_ssu_update_dvbt(&as_info->param, -1);
#endif
#ifdef DVBC_SUPPORT
    si_scan_ssu_update_dvbc(&as_info->param, -1);
#endif
}

INT32 as_otasearch_service_stop(BOOL b_blocking)
{
    return si_stop_channel_otasrch(b_blocking, -1);
}

#if 1
INT32 f_as_otascan_dvbt(UINT32 sfreq, UINT32 efreq, UINT32 bandwidth, UINT8 band_type)//SN 20050623
{
	UINT32 ret = SUCCESS;
	UINT32 center_freq = 0;
	
    union ft_xpond *xponder=NULL; 
    struct as_tuning_param *atp =NULL;
    INT32 psi_ret=0;
    UINT8 lock_stat=0;

    if(NULL == as_info)
    {
        return RET_FAILURE;
    }
      
    center_freq = sfreq + (bandwidth>>1);		
     
    xponder = &as_info->xponder;
    atp = &as_info->atp;
    MEMSET(xponder, 0, sizeof(union ft_xpond));
    xponder->common.type =as_info->param.as_frontend_type;
    xponder->t_info.type =FRONTEND_TYPE_T;
    xponder->t_info.frq =center_freq;
    xponder->t_info.band_width = bandwidth;		
    xponder->t_info.usage_type = (UINT8)USAGE_TYPE_AUTOSCAN; //auto scan	
    frontend_set_nim(as_info->nim, NULL, xponder, 1);
    
    do
    {
        nim_get_lock(as_info->nim, &lock_stat);
        if((lock_stat) || (as_info->stop_scan_signal) )
        {
            break;
        }
        osal_task_sleep(1);
    } while((INT32)osal_get_tick() < (INT32)atp->timeout);
        
	if (lock_stat)
	{
        //AUTO_OTA_DEBUG("enter:f_as_otascan_dvbt,and nim is lock\n");
        psi_module_init((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
        (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0), 
        (void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
        psi_ret= psi_search_start(0x01, atp->tp_id, as_info->param.as_prog_attr, 
        as_info->param.as_p_add_cfg, psi_callback, NULL, NULL);
		if (SI_STOFULL == psi_ret) 
		{
			ret = !SUCCESS;
		}
	}
	else
	    ret=!SUCCESS;
	
	//AUTO_OTA_DEBUG("psi_ret=%d,ret=%d\n",psi_ret,ret);
	return ret;	
}
#else
INT32 f_as_otascan_dvbt(UINT32 sfreq, UINT32 efreq, UINT32 bandwidth, UINT8 band_type)
{
    T_NODE t_node;
    INT32 i = 0;
    UINT32 center_freq = 0;
    UINT32 lock = 0;
    UINT8 j = 0;     //HIER
    static struct nim_device *lib_nim_dev=NULL;
    //struct NIM_CHANNEL_CHANGE nim_param;
    struct NIM_CHANNEL_SEARCH nim_param;
    UINT8 ota_guard_interval = 0;
    UINT8 ota_fft_mode = 0;
    UINT8 ota_modulation = 0;
    UINT8 ota_ffc = 0;
    UINT8 ota_inv = 0;

    center_freq = sfreq + (bandwidth>>1);
    MEMSET(&nim_param, 0, sizeof(nim_param));

    nim_param.freq = center_freq;
    //nim_param.sym=0;
    nim_param.bandwidth =  bandwidth/1000;

    nim_param.guard_interval = 0x4;

    nim_param.fft_mode = 0x2;

    nim_param.modulation = 0x4;

    nim_param.fec = 0;

    nim_param.usage_type = (UINT8)USAGE_TYPE_AUTOSCAN;


    nim_param.inverse = 1;
    nim_param.freq_offset=500;

    nim_param.priority = 0;

    lib_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    if(NULL == lib_nim_dev)
    {
        PUB_PRINT_LINE("Get device error!\n");
        return RET_FAILURE;
    }

    nim_io_control(lib_nim_dev, NIM_DRIVER_STOP_ATUOSCAN, 0);
    if(lib_nim_dev->do_ioctl_ext)
    {
        lock=lib_nim_dev->do_ioctl_ext(lib_nim_dev, NIM_DRIVER_CHANNEL_SEARCH, (void *)(&nim_param));
    }
    if (SUCCESS == lock)
    {
        MEMSET(&t_node, 0, sizeof(T_NODE));

        t_node.sat_id = 0x1;
        t_node.frq = center_freq;
        if (lib_nim_dev->get_guard_interval)
        {
            t_node.guard_interval =lib_nim_dev->get_guard_interval(lib_nim_dev, &ota_guard_interval);
        }

        if (lib_nim_dev->get_fftmode)
        {
            t_node.FFT=lib_nim_dev->get_fftmode(lib_nim_dev, &ota_fft_mode);
        }

        if (lib_nim_dev->get_modulation)
        {
            t_node.modulation=lib_nim_dev->get_modulation(lib_nim_dev, &ota_modulation);
        }

        t_node.bandwidth = bandwidth/1000;

        t_node.fec_inner = api_nim_get_fec();

        t_node.band_type = band_type;


        if (lib_nim_dev->get_spectrum_inv)
        {
            t_node.inverse=lib_nim_dev->get_spectrum_inv(lib_nim_dev, &ota_inv);
        }

        t_node.frq = api_nim_get_freq();
#if(defined(SUPPORT_TP_QUALITY))
        UINT8 agc_buffer[5];
        UINT8 snr_buffer[5];
        UINT16 agc_sigma=0;
        UINT16 snr_sigma=0;
        for(i=0;i<5;i++)
        {
            agc_buffer[i]=api_nim_get_agc();
            snr_buffer[i]=api_nim_get_snr();
            osal_task_sleep(5);
            agc_sigma+=agc_buffer[i];
            snr_sigma+=snr_buffer[i];
        }
        t_node.intensity=agc_sigma/5;
        t_node.quality=snr_sigma/5;
#endif
        psi_module_init((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
            (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0),
            (void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
        if (SI_STOFULL == psi_search_start(0x01, t_node.tp_id, as_info->param.as_prog_attr,
            as_info->param.as_p_add_cfg, psi_callback, NULL, NULL))
        {
            return RET_FAILURE;
        }
    }
    else
    {
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}
#endif

INT32 f_as_otascan_dvbc(UINT32 sfreq,UINT32 step,UINT32 symbol, UINT8 modulation,UINT8 bandtype)
{
    UINT32 ret = 0;
    T_NODE t_node;
    INT32 i = 0;
    UINT32 center_freq = 0;
    UINT32 lock = 0;
    static struct nim_device *lib_nim_dev=NULL;
    struct NIM_CHANNEL_CHANGE cc_nim_param;
    UINT8 ota_guard_interval = 0;
    UINT8 ota_fft_mode = 0;
    UINT8 ota_modulation = 0;
    UINT8 ota_ffc = 0;
    UINT8 ota_inv = 0;

    center_freq = sfreq + (step>>1);
    MEMSET(&cc_nim_param, 0, sizeof(cc_nim_param));
    cc_nim_param.freq = center_freq;
    cc_nim_param.sym=symbol;
    cc_nim_param.bandwidth =step/1000;

    cc_nim_param.modulation = modulation;

    lib_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    if(NULL == lib_nim_dev)
    {
        PUB_PRINT_LINE("Get device error!\n");
        return RET_FAILURE;
    }
    if(lib_nim_dev->do_ioctl_ext)
    {
        lock=lib_nim_dev->do_ioctl_ext(lib_nim_dev, NIM_DRIVER_CHANNEL_CHANGE,(void *)(&cc_nim_param));
    }

    if (SUCCESS == lock)
    {
        MEMSET(&t_node, 0, sizeof(T_NODE));

        t_node.sat_id = 0x1;
        t_node.frq = center_freq;
        if (lib_nim_dev->get_guard_interval)
        {
            t_node.guard_interval =lib_nim_dev->get_guard_interval(lib_nim_dev, &ota_guard_interval);
        }

        if (lib_nim_dev->get_fftmode)
        {
            t_node.FFT=lib_nim_dev->get_fftmode(lib_nim_dev, &ota_fft_mode);
        }

        if (lib_nim_dev->get_modulation)
        {
            t_node.modulation=lib_nim_dev->get_modulation(lib_nim_dev, &ota_modulation);
        }

        t_node.bandwidth = step/1000;

        t_node.fec_inner = api_nim_get_fec();

        t_node.band_type = bandtype;


        if (lib_nim_dev->get_spectrum_inv)
        {
            t_node.inverse=lib_nim_dev->get_spectrum_inv(lib_nim_dev, &ota_inv);
        }

        t_node.frq = api_nim_get_freq();

#if(defined(SUPPORT_TP_QUALITY))
        UINT8 agc_buffer[5];
        UINT8 snr_buffer[5];
        UINT16 agc_sigma=0;
        UINT16 snr_sigma=0;
        for(i=0;i<5;i++)
        {
            agc_buffer[i]=api_nim_get_agc();
            snr_buffer[i]=api_nim_get_snr();
            osal_task_sleep(5);
            agc_sigma+=agc_buffer[i];
            snr_sigma+=snr_buffer[i];
        }
        t_node.intensity=agc_sigma/5;
        t_node.quality=snr_sigma/5;
#endif
        psi_module_init((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
            (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0),
            (void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
        if (SI_STOFULL == psi_search_start(0x01, t_node.tp_id, as_info->param.as_prog_attr,
            as_info->param.as_p_add_cfg, psi_callback, NULL, NULL))
        {
            ret = !SUCCESS;
        }
    }
    else
    {
        ret = !SUCCESS;
    }

    return ret;
}

UINT16 get_region_ch_num()
{
    UINT16 total_cnt =0;
    UINT8 i = 0;

    if(as_info->param.as_method != AS_METHOD_FFT)
    {
        return 1;
    }
    for(i=0;i<MAX_BAND_COUNT;i++)
    {
        if(((as_info->param.band_group_param+i)->start_freq)!=NUM_ZERO)
        {
            total_cnt+= ((as_info->param.band_group_param+i)->end_freq - \
                (as_info->param.band_group_param+i)->start_freq)/(as_info->param.band_group_param+i)->bandwidth;
        }
        else
        {
            break;
        }
    }
    return total_cnt;
}
#endif
#endif
static INT32 as_scan(UINT16 sat_id, INT32 method, BOOL cached)
{
    INT32 ret = RET_FAILURE;
#ifdef NIM_S3501_ASCAN_TOOLS
    UINT32 t_loop = 0;
    int i=0;
#endif
    UINT8 prog_mode = PROG_ALL_MODE;

    if(INVALID_SAT_ID == sat_id)
    {
        return ret;
    }
    //Set Program Mode
    if((AS_METHOD_NVOD == as_info->param.as_method)||(AS_METHOD_NVOD_MULTI_TP == as_info->param.as_method))
    {
        prog_mode = PROG_NVOD_MODE;
    }
    else if(AS_METHOD_DATA == as_info->param.as_method)
    {
        prog_mode = PROG_DATA_MODE;
    }
    else
    {
        prog_mode = PROG_ALL_MODE;
    }

    //Create Program View
    ret = db_search_create_pg_view(TYPE_SAT_NODE, sat_id, prog_mode);
//#ifdef ITALY_HD_BOOK_SUPPORT
#if(defined ITALY_HD_BOOK_SUPPORT ||defined     POLAND_SPEC_SUPPORT)
    if(node_view_init_callback)
    {
        node_view_init_callback();
    }
#endif

    //Blind Scan
    if (AS_METHOD_FFT == method)
    {
        if(FRONTEND_TYPE_S == as_info->param.as_frontend_type)
        {
        #ifdef NIM_S3501_ASCAN_TOOLS

            t_loop = nim_io_control(as_info->nim, NIM_ASCAN_GET_LOOP_CNT, 0);
            nim_io_control(as_info->nim, NIM_ASCAN_CLEAR_LOOP_CUR_IDX, 0);

            for(i=0;i<t_loop;i++)
            {
                nim_io_control(as_info->nim, NIM_ASCAN_START_PROCESS, 0);
        #endif
                ret = as_dvbs_blind_scan(&as_info->antenna);
        #ifdef NIM_S3501_ASCAN_TOOLS
                if(ret != SUCCESS)
                {
                    break;
                }
                if(as_info->stop_scan_signal)
                {
                    break;
                }
                nim_io_control(as_info->nim, NIM_ASCAN_END_PROCESS, 0);
            }
        #endif
        }
        else if(FRONTEND_TYPE_C == as_info->param.as_frontend_type)
        {
#ifdef DVBC_COUNTRY_BAND_SUPPORT
            c_srh_tp_step = 0;
            dvbc_bs_freq_end = 0;
#endif
            if (FALSE == dvbc_init_search_param(&as_info->atp, &as_info->param))
            {
                return RET_FAILURE;
            }
            ret = as_enum_scan(sat_id, ATP_TYPE_STEP, as_dvbc_blind_tuning);
        }
        else if ((FRONTEND_TYPE_T == as_info->param.as_frontend_type)
             || (FRONTEND_TYPE_ISDBT == as_info->param.as_frontend_type)) 
        {
            if(FALSE == dvbt_init_search_param(&as_info->param, &as_info->atp))
            {
                return RET_FAILURE;
            }
            ret = as_enum_scan(sat_id, ATP_TYPE_STEP, as_dvbt_blind_tuning);
        }
    }
    else
    {
        MEMSET(&(as_info->atp), 0, sizeof(struct as_tuning_param));
        if ((FRONTEND_TYPE_T == (as_info->param.as_frontend_type))
        ||(FRONTEND_TYPE_ISDBT == (as_info->param.as_frontend_type)))
        {
            as_info->atp.ft_type = as_info->param.as_frontend_type;
        }
        ret = as_enum_scan(sat_id, ATP_TYPE_DBASE, as_dvbs_preset_tuning);
    }

    //Process by Upper Layer
    if ((SUCCESS != ret) && (as_info->param.as_handler))
    {
        as_info->param.as_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_STOFULL);
    }

    return ret;
}

static BOOL as_tp_sanity_check(UINT16 sat_id, S_NODE *sat)
{
    INT32 tp_nr = 0;
    INT32 ret = 1;
    BOOL check_ret = TRUE;
    int j = 0;
    T_NODE *tp = &(as_info->cur_tp);

    if((INVALID_SAT_ID == sat_id)|| (NULL==sat))
    {
        return FALSE;
    }
    tp_nr = get_tp_num_sat(sat_id);
    for(j=0; j<tp_nr; j++)
    {
        ret = get_tp_at(sat_id, j, tp);
        if (DB_SUCCES != ret)
        {
            AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        if(FRONTEND_TYPE_S == as_info->param.as_frontend_type)
        {
            if(FALSE == dvbs_tp_sanity_check(as_info->voltage_ctrl,tp, sat))
            {
                check_ret = FALSE;
                continue;
            }
        }
        else if(FRONTEND_TYPE_C == as_info->param.as_frontend_type)
        {
            if(FALSE == dvbc_tp_sanity_check(FALSE, tp, NULL))
            {
                check_ret = FALSE;
                continue;
            }
        }

        AS_DUMP_TP(tp);

        if (as_info->tp_nr >= AS_MODULE_MAX_T_NUM)
        {
            check_ret = FALSE;
            return check_ret;
        }
    #ifdef NVOD_FEATURE
        if((AS_METHOD_NVOD_MULTI_TP == as_info->param.as_method) && (FLAG_TRUE == tp->nvod_flg))
    #endif
        {
            as_info->tp_id[as_info->tp_nr] = tp->tp_id;
            as_info->tp_nr++;
        }

        if (AS_MODULE_MAX_T_NUM == as_info->tp_nr)
        {
            AS_PRINTF("%s: tp exceed AS_MODULE_MAX_T_NUM!\n", __FUNCTION__);
        }
    }

    return check_ret;
}


static void as_sat_scan(void)
{
    S_NODE *sat = NULL;
    INT32 ret = RET_FAILURE;
    as_update_event_t event_handler = NULL;
    UINT16 sat_id = 0xFFFF;
    struct ft_antenna *antenna = &as_info->antenna;
    int i = 0;
    INT32 as_method = 0;
    INT32 method = 0;
    BOOL sanity_ret = FALSE;

    sat = &(as_info->cur_sat);
    as_method = as_info->param.as_method;
    event_handler = as_info->param.as_handler;
    for(i = 0; i<as_info->sat_nr; i++)
    {
        //For Each Satellite, Create its TP View, Set Antenna
        sat_id = as_info->sat_id[i];
        ret = db_search_create_tp_view(sat_id);
        if (RET_SUCCESS != ret)
        {
            PUB_PRINT_LINE("ret = %d\n", ret);
        }

        if(event_handler != NULL)
        {
            event_handler(ASM_TYPE_SATELLITE, sat_id);
        }

        //Set Antenna Information according to Satellite Information
        MEMSET(sat, 0, sizeof(S_NODE));
        get_sat_by_id(sat_id, sat);
        if ((FRONTEND_TYPE_T == as_info->param.as_frontend_type)
        || (FRONTEND_TYPE_ISDBT == as_info->param.as_frontend_type))
        {
            sat->sat_id = 1;
        }

        AS_DUMP_SAT(sat);
        MEMSET(antenna, 0, sizeof(struct ft_antenna));
        if(FRONTEND_TYPE_S == as_info->param.as_frontend_type)
        {
            sat2antenna(sat, antenna);
            as_info->dual_lnb = (antenna->lnb_high>0) && (antenna->lnb_low>0) \
                && (antenna->lnb_high != antenna->lnb_low);
            as_info->voltage_ctrl = (LNB_POL_H == antenna->pol)||(LNB_POL_V == antenna->pol);

            //>>> Unicable begin
            if(LNB_CTRL_UNICABLE == antenna->lnb_type)
            {
                as_info->nim_para.unicable = 1;
                as_info->nim_para.fub = antenna->unicable_freq;
            }
            else
            {
                as_info->nim_para.unicable = 0;
            }
        }
        //<<< Unicable end
        //select right nim and dmx
        as_select_nim(sat, as_info->param.as_frontend_type);
        if(NULL == as_info->nim)
        {
            AS_PRINTF("%s(): select nim failed!\n");
            continue;
        }

         //for 29E 2-Tuner the dmx path may not same as default!!
         tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);

        //blind scan
        if((AS_METHOD_FFT==as_method)||(AS_METHOD_MULTI_FFT==as_method))
        {
            method = AS_METHOD_FFT;
        }
        else
        {
            as_info->tp_nr = 0;
            MEMSET(as_info->tp_nit, 0,AS_MODULE_MAX_T_NUM*sizeof(UINT8));
            //single tp scan
            if((AS_METHOD_TP==as_method)||(AS_METHOD_TP_PID==as_method)
                ||(AS_METHOD_NIT_TP==as_method)||(AS_METHOD_NVOD==as_method)
                ||(AS_METHOD_FREQ_BW==as_method))
            {
                as_info->tp_nr = 1;

                as_info->tp_id[0] = as_info->param.as_from;
                if ((AS_METHOD_NIT_TP==as_method)&&(event_handler!=NULL))
                {
                    event_handler(ASM_TYPE_INC_TRANSPONDER, 0);
                }
            }
            //multi tp scan
            else if((AS_METHOD_SAT_TP==as_method)
                ||(AS_METHOD_NIT==as_method)
                ||(AS_METHOD_MULTI_TP==as_method)
                ||(AS_METHOD_MULTI_NIT==as_method)
                ||(AS_METHOD_NVOD_MULTI_TP==as_method))
            {
                sanity_ret = as_tp_sanity_check(sat_id, sat);
                if (!sanity_ret)
                {
                    AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
                }
            }

            if((AS_METHOD_NIT==as_method)||(AS_METHOD_NIT_TP==as_method)
                ||(AS_METHOD_MULTI_NIT==as_method))
            {
                MEMSET(as_info->tp_nit, 1, as_info->tp_nr);
            }

            method = AS_METHOD_TP;
        }

        as_scan(sat_id, method, 0);

        update_data();

        //solve the bug:if the first nim is error,won't scan the second nim
        if(as_info->stop_scan_signal)
        {
            break;
        }
    }
}

/* description  : auto scan main routine for command parsing and switching.*/
static void as_main(struct libpub_cmd *cmd)
{
    RET_CODE ret_code = RET_FAILURE;
#ifdef AUTO_OTA    
    INT32 as_method = 0;
#endif
    UINT8 dmx_mode_need_resume = 0;
    UINT32 dmx_dma_mode = 0;
    as_update_event_t event_handler = NULL;
    struct dmx_device *dmx = NULL;

    AS_PRINTF("start time stamp = %d\n", osal_get_tick());
#ifdef AUTO_OTA    
    as_method = as_info->param.as_method;
#endif

    event_handler = as_info->param.as_handler;
#ifdef AUTO_OTA
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
    if(as_method == AS_METHOD_SSU_SEARCH)
    {
    }
    else
#endif
#endif
    if(event_handler!=NULL)
    {
        event_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANSTART);
    }

    //Select DMX
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    if(NULL == dmx)
    {
        AS_PRINTF("%s(): select dmx failed!\n");
        return;
    }
    as_info->dmx = dmx;

    //fix BUG20874: init psi info to avoid dmx changed by video playing
    psi_info_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, __MM_PSI_BUF_LEN);
    //Check and Switch Buffer Mode of Dmx (From Divided Buffer To Union Buffer)
    ret_code = dmx_io_control(as_info->dmx, DMX_CHK_BUF_MODE, (UINT32)&dmx_dma_mode);
    if (RET_SUCCESS != ret_code)
    {
        AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
    }
    if(DMX_DIVIDED_BUFF_MODE == dmx_dma_mode)
    {
        ret_code = dmx_io_control(as_info->dmx, DMX_SWITCH_TO_UNI_BUF, 0);
        if (RET_SUCCESS != ret_code)
        {
            AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        dmx_mode_need_resume = 1;
    }
#ifdef AUTO_OTA
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
    if(AS_METHOD_SSU_SEARCH == as_method)
    {
        UINT16 sat_id = 0xFFFF;
        S_NODE *sat = &(as_info->cur_sat);
        INT32 i = 0;
        struct ft_antenna *antenna = &as_info->antenna;

        //set tsi for failure of ota scan when there is no program.
        //For Each Satellite, Create its TP View, Set Antenna
        sat_id = as_info->sat_id[i];
        db_search_create_tp_view(sat_id);

        //Set Antenna Information according to Satellite Information
        MEMSET(sat, 0, sizeof(S_NODE));
        get_sat_by_id(sat_id, sat);
//#ifdef PUB_PORTING_PUB29
        //if(as_info->param.as_frontend_type==FRONTEND_TYPE_T)
        //  sat->sat_id = 1;
//#endif
        AS_DUMP_SAT(sat);
        MEMSET(antenna, 0, sizeof(struct ft_antenna));
        sat2antenna(sat, antenna);

        as_info->dual_lnb = (antenna->lnb_high>0) && (antenna->lnb_low>0) \
            && (antenna->lnb_high != antenna->lnb_low);
        as_info->voltage_ctrl = (antenna->pol==LNB_POL_H) \
            ||(antenna->pol==LNB_POL_V);
//>>> Unicable begin
        if(antenna->lnb_type == LNB_CTRL_UNICABLE)
        {
            as_info->nim_para.unicable = 1;
            as_info->nim_para.fub = antenna->unicable_freq;
        }
        else
        {
            as_info->nim_para.unicable = 0;
        }
//<<< Unicable end
        //select right nim and dmx
        as_select_nim(sat, as_info->param.as_frontend_type);

        //for 29E 2-Tuner the dmx path may not same as default!!
        tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);

        as_scan_ssu_update();
        as_info->param.as_handler(SI_OTA_SEARCH_OVER, 0);
        //libc_printf("ota auto search end!!");
    }
    else
    {
  #endif
#endif
        as_sat_scan();
        //Show Scan Over by Upper Layer
        if(event_handler != NULL)
        {
            event_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANOVER);
        }
#ifdef AUTO_OTA
  #if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
    }
  #endif
#endif

    ret_code = dmx_io_control(as_info->dmx, CLEAR_STOP_GET_SECTION, 0);
    if (RET_SUCCESS != ret_code)
    {
        AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
    }

    //Switch Buffer Mode of Dmx (From Union Buffer to Divided Buffer)
    if(dmx_mode_need_resume)
    {
        ret_code = dmx_io_control(as_info->dmx, DMX_SWITCH_TO_DVI_BUF, 0);
        if (RET_SUCCESS != ret_code)
        {
            AS_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
    }
    sie_open(as_info->dmx, MAX_FILTER_NUMBER, NULL, 0);

    osal_task_dispatch_off();
    nim_io_control(as_info->nim, NIM_DRIVER_STOP_ATUOSCAN, 0);
    if ((UINT32)as_info == (__MM_EPG_BUFFER_START&0x8FFFFFFF))
    {
        MEMSET(as_info, 0x0, __MM_EPG_BUFFER_LEN);
    }
    as_info = NULL;
    AS_PRINTF("end time stamp = %d\n", osal_get_tick());
    osal_task_dispatch_on();
}

void as_dump_sat(S_NODE *sat)
{
    if (NULL != sat)
    {
        AS_PRINTF("***************************************\n");
        AS_PRINTF("* sat id = %d\n", sat->sat_id);
        AS_PRINTF("* sat lnb_type = %d\n", sat->lnb_type);
        AS_PRINTF("* sat lnb_low = %d\n", sat->lnb_low);
        AS_PRINTF("* sat lnb_high = %d\n", sat->lnb_high);
        AS_PRINTF("* sat polar = %d\n", sat->pol);
        AS_PRINTF("***************************************\n");
    }
}

void as_dump_tp(T_NODE *tp)
{
    if (NULL != tp)
    {
        AS_PRINTF("***************************************\n");
        AS_PRINTF("* tp id = %d\n", tp->tp_id);
        AS_PRINTF("* tp freq = %d\n", tp->frq);
        AS_PRINTF("* tp sym = %d\n", tp->sym);
        AS_PRINTF("* tp pol = %d\n", tp->pol);
        AS_PRINTF("* tp FEC inner = %d\n", tp->fec_inner);
        AS_PRINTF("***************************************\n");
    }
}


/*description   : register auto-scan service task, but not run it.*/
INT32 as_service_register(void)
{
    BOOL ret = FALSE;

    ret = libpub_cmd_handler_register(LIBPUB_CMD_AS, as_main);
    if (ret)
    {
        return RET_SUCCESS;
    }
    else
    {
        return RET_FAILURE;
    }
}


/*description   : start an auto-scan according to the parameter asp.*/
INT32 as_service_start(struct as_service_param *asp)
{
    struct libpub_cmd cmd;

    if ((NULL == asp) || (as_info != NULL))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return RET_FAILURE;
    }

#ifndef __MM_VBV_START_ADDR
    if(NULL == g_as_buf)
    {
        g_as_buf = MALLOC(AS_BUF_LEN);
        if(NULL == g_as_buf)
        {
            PUB_PRINT_LINE("Not Enough Memory!\n");
            return RET_FAILURE;
        }
    }
#endif

    MEMSET(&cmd, 0, sizeof(struct libpub_cmd));

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
    star_search_nit_staus = SEARCH_NIT_STATUS_NOT_REC;
#endif

    as_info = (struct as_module_info *)__MM_AS_START_ADDR;
    as_set_as_info(as_info);

    MEMSET(as_info, 0, sizeof(struct as_module_info));
    MEMCPY(&as_info->param, asp, sizeof(struct as_service_param));
    as_info->param.as_prog_attr |= P_SEARCH_SATTP;

    if(FRONTEND_TYPE_S == asp->as_frontend_type)
    {
        dvbs_as_callback(AS_SGN_INIT, 0, 0, 0, 0);
    }
    as_info->sat_nr = asp->as_sat_cnt;
    if(as_info->sat_nr > AS_MODULE_MAX_S_NUM)
    {
        as_info->sat_nr = AS_MODULE_MAX_S_NUM;
    }

    MEMCPY(as_info->sat_id, asp->sat_ids,as_info->sat_nr*sizeof(UINT16));

    cmd.cmd_type = LIBPUB_CMD_AS_START;
    return libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

}

/* API for Stop Auto Scan */
INT32 as_service_stop(void)
{
    struct nim_device *nim = NULL;

    osal_task_dispatch_off();
    if (as_info != NULL)
    {
        as_info->stop_scan_signal = 1;
        if(as_info->nim)
        {
            nim = as_info->nim;
            nim_io_control(nim, NIM_DRIVER_STOP_ATUOSCAN, 1);
        }
    }
    osal_task_dispatch_on();

    if (as_info != NULL)
    {
        psi_search_stop();//stop getting psi section
    }

    while (as_info)
    {
        osal_task_sleep(AS_MODULE_SLEEP_SLICE);
    }

#ifndef __MM_VBV_START_ADDR
    if(NULL != g_as_buf)
    {
        FREE(g_as_buf);
        g_as_buf = NULL;
    }
#endif

#ifdef MULTI_CAS 
        /* evan.wu fix for C0200A: if tp search same tp, xpond will not 
           change, and emm can not fill to CAK */
        mcas_xpond_stoped = TRUE;
#endif

    return RET_SUCCESS;
}


/*description   : return ERR_BUSY when in scan mode.*/
INT32 as_service_query_stat(void)
{
    return (NULL == as_info)? SUCCESS: ERR_BUSY;
}

/*search for special program that not listed in PAT,PMT,SDT,
* input the video,audio,pcr pids, dmx will check if there is probably
* such a prog in current TS.
*/
BOOL as_prog_pid_search(struct dmx_device *dmx,UINT8 av_mode, UINT16 v_pid, UINT16 a_pid,UINT16 pcr_pid)
{
    UINT16 pid_list[3] = {0};
    struct io_param io_param; //= {0};
    BOOL ret = FALSE;
    UINT32 tmo = 6;

    if((av_mode != FLAG_TRUE) ||(INVALID_PID == v_pid)||(INVALID_PID == a_pid)||(INVALID_PID == pcr_pid))
    {
        return FALSE;
    }

    MEMSET(&io_param, 0, sizeof(struct io_param));
    if(NULL == dmx)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return FALSE;
    }

    pid_list[0] = v_pid;
    pid_list[1] = a_pid;
    pid_list[2] = pcr_pid;
    io_param.io_buff_in = (UINT8 *)pid_list;
    io_param.buff_in_len = sizeof(pid_list);
    if(RET_SUCCESS == dmx_io_control(dmx, IO_CREATE_AV_STREAM, (UINT32)&io_param))
    {
        if(RET_SUCCESS == dmx_io_control(dmx, IO_STREAM_ENABLE, (UINT32)&io_param))
        {
            while(tmo)
            {
                osal_task_sleep(100);
                tmo--;
                if(RET_SUCCESS == dmx_io_control(dmx, IS_PROGRAM_LEGAL, (UINT32)&io_param))
                {
                    ret = TRUE;
                    break;
                }
            }
        }
    }

    if (RET_SUCCESS == dmx_io_control(dmx, IO_STREAM_DISABLE, (UINT32)&io_param))
    {
        ret = TRUE;
    }

    return ret;
}
//#ifdef ITALY_HD_BOOK_SUPPORT
#if(defined ITALY_HD_BOOK_SUPPORT ||defined POLAND_SPEC_SUPPORT)
void node_view_inital_registor(BOOL (*node_view_inital_callback)())
{
    node_view_init_callback = node_view_inital_callback;
}
void node_view_inital_unregistor()
{
    node_view_init_callback = NULL;
}
#endif


