/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_ts_route.c

*    Description: The ts route info of pvr will be defined in this file.
                  It contains the tsi \ si monitor \ dmx info.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <bus/tsi/tsi.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/si_types.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_cc.h>
#include <api/libtsg/lib_tsg.h>
#include <hld/dmx/dmx.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "pvr_ctrl_ts_route.h"
#include "ap_ctrl_ci.h"
#include "ap_dynamic_pid.h"
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#define PVR_TS_ROUTE_NIM_ID_MAX   2

static INT32 ts_route_id_while_playback = PVR_TS_ROUTE_INVALID_ID;
#if (ISDBT_CC == 1)
extern INT32 isdbtcc_unregister(UINT32 monitor_id);
extern INT32 isdbtcc_register(UINT32 monitor_id);
#endif
#ifdef NEW_DEMO_FRAME
INT32 pvr_get_playback_ts_route_id(void)
{
    return ts_route_id_while_playback;
}

BOOL pvr_set_playback_ts_route_id(INT32 new_route_id)
{
    if((new_route_id != PVR_TS_ROUTE_INVALID_ID ) && (new_route_id >= PVR_MAX_TS_ROUTE_NUM))
    {
        return FALSE;
    }
    ts_route_id_while_playback = new_route_id;
    return TRUE;
}

#endif

void ts_route_debug_print(PVR_HANDLE pvr_handle __MAYBE_UNUSED__)
{
 #if TS_ROUTE_DEBUG_ENABLE
    {
        UINT8 rec_dmx_id = 0xff;
        if (pvr_handle)
        {
            rec_dmx_id = pvr_r_get_dmx_id(pvr_handle);
        }
        libc_printf("============ %s() ===========\n",__FUNCTION__);
        UINT8 ts_id, tsi_id, ci_mode;
        int dmx_id = TSI_DMX_0;
        tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
        libc_printf("route:TSI %d --> %d -->DMX %d (%d) %s\n",
            tsi_id, ts_id, dmx_id-1, ci_mode, (0 == rec_dmx_id ? "rec" : ""));

        dmx_id = TSI_DMX_1;
        tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
        libc_printf("route:TSI %d --> %d -->DMX %d (%d) %s\n",
            tsi_id, ts_id, dmx_id-1, ci_mode, (1 == rec_dmx_id ? "rec" : ""));
    }
#endif

}


#if TS_ROUTE_DEBUG_ENABLE
void ts_route_print(void)
{
    libc_printf("============ ts route ===========\n",__FUNCTION__);
    UINT8 ts_id, tsi_id, ci_mode;
    int dmx_id = TSI_DMX_0;
    tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
    libc_printf("route: %d --> %d --> %d (%d)\n",
                tsi_id, ts_id, dmx_id, ci_mode);

    dmx_id = TSI_DMX_1;
    tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
    libc_printf("route: %d --> %d --> %d (%d)\n",
                tsi_id, ts_id, dmx_id, ci_mode);

    return;
}
#endif


//name:TS_Route_deug_printf
//Description: Printf the TS Route debug information
//Need: Open the cro TS_ROUTE_DEBUG_ENABLE
void ts_route_deug_printf(void)
{
#if TS_ROUTE_DEBUG_ENABLE
    {
        pvr_play_rec_t  *pvr_info = NULL;
        UINT8 play_dmx_id = 0xff;

        pvr_info  = api_get_pvr_info();
        if ( pvr_info->play.play_handle)
        {
            play_dmx_id = pvr_p_get_dmx_id( pvr_info->play.play_handle);
        }
        libc_printf("============ %s() ===========\n",__FUNCTION__);
        UINT8 ts_id, tsi_id, ci_mode;
        int dmx_id = TSI_DMX_0;
        tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
        libc_printf("route: %d --> %d --> %d (%d) %s\n",
            tsi_id, ts_id, dmx_id, ci_mode, (0 == play_dmx_id  ? "play" : ""));

        dmx_id = TSI_DMX_1;
        tsi_check_dmx_src(dmx_id, &ts_id, &tsi_id, &ci_mode);
        libc_printf("route: %d --> %d --> %d (%d) %s\n",
            tsi_id, ts_id, dmx_id, ci_mode, (1 == play_dmx_id ? "play" : ""));
    }
#endif
}

#ifdef NEW_DEMO_FRAME
void ap_pvr_start_sim_for_record(pvr_record_t *new_rec_info,P_NODE *p_r_node,
                                            record_ts_route_update *ts_route_param)
{
    struct dmx_config dmx_cfg;
    UINT8 rec_dmx_id = 0;
    ts_route_param->rec_dmx_id=rec_dmx_id;
    struct dmx_device *dmx =NULL;
    int b_force = TRUE;

    rec_dmx_id = pvr_r_get_dmx_id(new_rec_info->record_handle);
    #ifdef NEW_TIMER_REC    
    ts_route_param->rec_dmx_id=rec_dmx_id; //vicky201410 coding from code review
    #endif
    
     MEMSET(&dmx_cfg,0,sizeof(struct dmx_config));
    dmx =(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (0== rec_dmx_id)?0:1);
#ifdef CC_USE_TSG_PLAYER
            b_force = FALSE;
            if ((rec_dmx_id != ts_route_param->live_dmx_id) && !ts_route_param->transed)
#else
#ifdef CI_PLUS_PVR_SUPPORT
            if( (prog_info.is_scrambled) && (!ts_route_param->transed))
            {
                b_force = FALSE;
            }
#endif
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
            //start new sim
            ts_route_param->sim_pmt_id = api_sim_start(dmx, MONITE_TB_PMT, p_r_node->tp_id, p_r_node->prog_id,
                                            p_r_node->prog_number, p_r_node->pmt_pid,b_force, api_sim_callback);
#endif
            if ((new_rec_info->is_scrambled) && (!ts_route_param->transed))
#endif
            {
                #if(!defined(CAS9_PVR_SCRAMBLED) && !defined(CAS7_PVR_SCRAMBLE)  && \
                !defined(PVR_DYNAMIC_PID_CHANGE_TEST) && !defined(CAS7_ORIG_STREAM_RECORDING))
                    //start new sim
                    ts_route_param->sim_pmt_id = api_sim_start(dmx, MONITE_TB_PMT, p_r_node->tp_id, p_r_node->prog_id,
                                                p_r_node->prog_number, p_r_node->pmt_pid,b_force, api_sim_callback);
                #endif
                #if !(defined( SUPPORT_CAS9) || defined( SUPPORT_CAS7))
                    ts_route_param->sim_cat_id = api_sim_start(dmx, MONITE_TB_CAT, p_r_node->tp_id, p_r_node->prog_id,
                                                    p_r_node->prog_number, 1,b_force, NULL);
                #endif
            }
}



void ap_pvr_update_ts_route_for_record(pvr_record_t *new_rec_info, P_NODE *p_r_node,
                                                    record_ts_route_update *update_param)
{
    UINT32 channel_id=update_param->channel_id;
    UINT32 sim_pmt_id=update_param->sim_pmt_id;
    UINT8 rec_dmx_id=update_param->rec_dmx_id;
    UINT8 live_dmx_id=update_param->live_dmx_id;
    BOOL transed=update_param->transed;
    struct ts_route_info ts_route;
    struct ts_route_info ts_route_m;

    MEMSET(&ts_route_m, 0, sizeof(struct ts_route_info));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        //update ts route state!

#ifdef CC_USE_TSG_PLAYER
    if ((rec_dmx_id != live_dmx_id) && !transed)
#else
    if (((new_rec_info->is_scrambled) && (!transed))
#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
    || ((rec_dmx_id != live_dmx_id) &&cc_get_h264_chg_flag())
#endif
    )
#endif
    {
        // for ts_route management!
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        ts_route.enable = 1;
        ts_route.type = TS_ROUTE_BACKGROUND;
        ts_route.is_recording = 1;
        ts_route.dmx_id = rec_dmx_id;
        ts_route.dmx_slot_id = 0; //TODO: need check later!!!
        ts_route.nim_id = pvr_r_get_nim_id(new_rec_info->record_handle);
        ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);

#ifndef CAS9_PVR_SCRAMBLED
    #if  defined(CAS7_PVR_SCRAMBLE)
            MEMSET(&ts_route_m,0,sizeof(ts_route_m));
            if(RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_MAIN_PLAY,NULL,&ts_route_m))
            {
                PRINTF("ts_route_get_by_type() failed!\n");
            }
            ts_route.ts_id = ts_route_m.ts_id;
    #else
            if (SINGLE_CI_SLOT == g_ci_num )
            {
            ts_route.ts_id = TSI_TS_B;
            }
        else if (DUAL_CI_SLOT == g_ci_num )
            {
                MEMSET(&ts_route_m,0,sizeof(ts_route_m));
                if(RET_SUCCESS !=ts_route_get_by_type(TS_ROUTE_MAIN_PLAY,NULL,&ts_route_m))
                {
                    PRINTF("ts_route_get_by_type() failed!\n");
                }
                ts_route.ts_id = 3 - ts_route_m.ts_id;
            }

    #endif
 #else
        MEMSET(&ts_route_m,0,sizeof(ts_route_m));
        if(RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL,&ts_route_m))
        {
            PRINTF("ts_route_get_by_type() failed!\n");
        }
        ts_route.ts_id = ts_route_m.ts_id;
#endif

#ifdef CI_SUPPORT
        ts_route.ci_mode = sys_data_get_ci_mode();
#endif
        ts_route.ci_num = g_ci_num;
        ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
        ts_route.cia_used = 0;
        ts_route.cib_included =(((ts_route.ci_num>1)&&((CI_SERIAL_MODE==ts_route.ci_mode)&&(TSI_TS_A==ts_route.ts_id)))
                                ||((CI_PARALLEL_MODE == ts_route.ci_mode) && (TSI_TS_B == ts_route.ts_id)))?1:0;
        ts_route.cib_used = 0;
        ts_route.vdec_id = 0;
        ts_route.vpo_id = 0;
        ts_route.state = TS_ROUTE_STATUS_RECORD;
        ts_route.tp_id = p_r_node->tp_id;
        ts_route.prog_id = p_r_node->prog_id;
        ts_route.stream_av_mode = p_r_node->av_flag;
        ts_route.stream_ca_mode = p_r_node->ca_mode;
        ts_route.screen_mode = 1;
        if(sim_pmt_id != PVR_INVALID_SIM_ID)
        {
            ts_route.sim_num++;
            ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_PMT;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = p_r_node->pmt_pid;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = sim_pmt_id;
            ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = p_r_node->prog_id;
            ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)api_sim_callback;
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
            if(update_param->sim_cat_id != PVR_INVALID_SIM_ID)
            {
                ts_route.sim_num++;
                ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
                ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_CAT;
                ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = 1;
                ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = update_param->sim_cat_id;
                ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = p_r_node->prog_id;
                ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)NULL;
            }
#endif
        }
        ts_route_create(&ts_route);
    }
    else
    {
#ifdef CC_USE_TSG_PLAYER
        int i;
        if (transed)
        {
            for (i = 0; i < CC_MAX_TS_ROUTE_NUM; ++i)
            {
                if ((RET_SUCCESS == ts_route_get_by_id(i, &ts_route)) &&
                    (ts_route.prog_id == channel_id) &&
                    (ts_route.state & TS_ROUTE_STATUS_TMS) &&
                    (1 == ts_route.is_recording))
                {
                    ts_route.state &= ~TS_ROUTE_STATUS_TMS; // for tms->rec
                    ts_route.state |= TS_ROUTE_STATUS_RECORD;
                    ts_route_update(ts_route.id, &ts_route);
                    CI_PATCH_TRACE("ts_route %d: tms --> rec\n", ts_route.id);
                }
            }
        }
        else if (rec_dmx_id == live_dmx_id)
        {
            for (i = 0; i < CC_MAX_TS_ROUTE_NUM; ++i)
            {
                if ((RET_SUCCESS == ts_route_get_by_id(i, &ts_route)) &&
                    (ts_route.prog_id == channel_id) &&
                    (TS_ROUTE_MAIN_PLAY == ts_route.type) &&
                    (ts_route.state & TS_ROUTE_STATUS_PLAY) &&
                    (FALSE == ts_route.is_recording))
                {
                    ts_route.state &= ~TS_ROUTE_STATUS_TMS; // for play->rec
                    ts_route.state |= TS_ROUTE_STATUS_RECORD;
                    ts_route.is_recording = 1;
                    ts_route_update(ts_route.id, &ts_route);
                    CI_PATCH_TRACE("ts_route %d: play --> rec\n", ts_route.id);
                }
            }
        }
        else
#endif
        {
            if(api_pvr_get_rec_ts_route(channel_id, &ts_route))
            {
                ts_route.state &= ~TS_ROUTE_STATUS_TMS; // for tms->rec
                ts_route.state |= TS_ROUTE_STATUS_RECORD;
                ts_route.is_recording = 1;
                ts_route_update(ts_route.id, &ts_route);

                #ifdef FSC_SUPPORT
                fcc_update_record_state(ts_route.prog_id,ts_route.is_recording);
                #endif
            }
        }
    }
}


BOOL api_pvr_get_rec_ts_route(UINT32 channel_id, struct ts_route_info *p_ts_route)
{
    BOOL ret = FALSE;
    UINT8 tp_route_num = 0;
    UINT16 tp_routes[4] = {0};
    UINT16 i = 0;
    P_NODE p_r_node;

    MEMSET(&p_r_node, 0, sizeof (p_r_node));
    if(get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
    {
        return ret;
    }

    if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, p_ts_route) != RET_FAILURE)
    {
        if(p_ts_route->prog_id != channel_id) //may timer record!
        {
            ts_route_check_tp_route(p_r_node.tp_id, &tp_route_num, tp_routes);
            if(tp_route_num > 0)
            {
                for(i=0; i<tp_route_num; i++)
                {
                    if((ts_route_get_by_id(tp_routes[i], p_ts_route) != RET_FAILURE) &&
                        (p_ts_route->prog_id == channel_id))
                    {
                        ret = TRUE;
                        break;
                    }
                }
            }
        }
        else
        {
            ret = TRUE;
        }
    }

    return ret;
}
void ap_pvr_update_ts_route_for_play(BOOL is_not_tms_idx,UINT8 ts_id,BOOL preview_mode)
{
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
    UINT32 sim_cat_id = PVR_INVALID_SIM_ID;
#endif
    INT32 ts_route_id=PVR_TS_ROUTE_INVALID_ID;
    UINT32 sim_pmt_id = PVR_INVALID_SIM_ID;
    struct ts_route_info ts_route;
    struct ts_route_info ts_route1;
    struct ci_service_info service;//Since It's for playback the service is Invalid
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info play_pvr_info;
    UINT8 dmx_id = 0;
    struct dmx_device *dmx = NULL;

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    MEMSET(&ts_route,0,sizeof(struct ts_route_info));
    MEMSET(&ts_route1,0,sizeof(struct ts_route_info));
    MEMSET(&service,0,sizeof(struct ci_service_info));

    #ifdef NEW_DEMO_FRAME
    if(!api_pvr_get_back2menu_flag())
    {
        //update main play channel state!
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
        {
            api_pvr_set_playback_type(INFO_PLAYBACK);
            ts_route_id_while_playback = ts_route_id;
            ts_route.type = TS_ROUTE_BACKGROUND;
            ts_route.state &=     ~TS_ROUTE_STATUS_PLAY;
            ts_route_update(ts_route_id, &ts_route);
            if((TRUE == ts_route.dmx_sim_info[0].used) && (MONITE_TB_PMT == ts_route.dmx_sim_info[0].sim_type))
            {

            #if (SUBTITLE_ON == 1)
                subt_unregister(ts_route.dmx_sim_info[0].sim_id);
            #endif

            #if (TTX_ON == 1)
                ttx_unregister(ts_route.dmx_sim_info[0].sim_id);
            #endif

            #if (ISDBT_CC == 1)
                isdbtcc_unregister(ts_route.dmx_sim_info[0].sim_id);
            #endif
            }
        }
    }
    else // back to menu
    {
        api_pvr_set_playback_type(MENU_PLAYBACK);
    }
    // for ts_route management!
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    ts_route.enable = TRUE;
    ts_route.type = TS_ROUTE_PLAYBACK;
    ts_route.dmx_id = pvr_p_get_dmx_id( pvr_info->play.play_handle);
#if(!defined(CAS9_PVR_SCRAMBLED) && !defined(CAS7_PVR_SCRAMBLE))
    if(((play_pvr_info.ca_mode) && (play_pvr_info.is_scrambled)) && (is_not_tms_idx))
    {
        ts_route.tsi_id = TSI_SPI_TSG;
        ts_route.ts_id = ts_id;
#ifdef CI_SUPPORT
        ts_route.ci_mode = sys_data_get_ci_mode();
#endif
        ts_route.ci_num = g_ci_num;
        if (SINGLE_CI_SLOT == g_ci_num )
        {
            ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
            ts_route.cia_used = ts_route.cia_included;
            ts_route.cib_included = ((CI_SERIAL_MODE == ts_route.ci_mode) && (TSI_TS_A == ts_route.ts_id )) ||
               ((CI_PARALLEL_MODE == ts_route.ci_mode) && (TSI_TS_B == ts_route.ts_id));
            ts_route.cib_used = ts_route.cib_included;
        }
    }
#endif
    ts_route.vdec_id = 0;
    ts_route.vpo_id = 0;
    ts_route.state = TS_ROUTE_STATUS_PLAYACK;
    ts_route.stream_av_mode = 1 - play_pvr_info.channel_type;
    ts_route.stream_ca_mode = play_pvr_info.ca_mode;
    ts_route.screen_mode = 1;

    {
        dmx_id = pvr_p_get_dmx_id( pvr_info->play.play_handle);
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

        if(dmx)
        {
            sim_pmt_id = api_sim_start(dmx, MONITE_TB_PMT, 0, service.prog_id, play_pvr_info.prog_number,
                            play_pvr_info.pid_info.pmt_pid, 0x70|TRUE, api_sim_callback_for_playback);
            if(sim_pmt_id != PVR_INVALID_SIM_ID)
            {
                #if(defined(CAS9_PVR_SCRAMBLED))
                    if( pvr_info->pvr_state != PVR_STATE_TMS_PLAY)
                    {
                        api_mcas_request_tdt();  //request TDT from recorded content when playback
                    }
                    //playback original stream
                    //api_mcas_start_service_multi_des(service.prog_id,sim_pmt_id,2);   //init global variables
                    //api_mcas_start_transponder_multi_des(2);  //request to filter CAT when playback
                    api_mcas_start_service(service.prog_id,sim_pmt_id);   //init global variables
                    api_mcas_start_transponder();  //request to filter CAT when playback
                #endif
                #if  defined(CAS7_PVR_SCRAMBLE)|| defined(CAS7_ORIG_STREAM_RECORDING)
                    if( pvr_info->pvr_state != PVR_STATE_TMS_PLAY)
                    {
                        api_mcas_request_tdt();  //request TDT from recorded content when playback
                    }
                    api_mcas_start_service(0,sim_pmt_id);   //init global variables
                    api_mcas_start_transponder();  //request to filter CAT when playback
                #endif

        #ifdef SUPPORT_CAS_A
            api_set_dmx_device(dmx);
            api_mcas_pmt_pid(play_pvr_info.pid_pmt);
               api_mcas_start_service(service.prog_id);
               api_mcas_start_transponder();
        #endif
              
                if((play_pvr_info.ca_mode && play_pvr_info.is_scrambled)&&is_not_tms_idx)
                {
                #if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
                    sim_cat_id = api_sim_start(dmx, MONITE_TB_CAT, 0, service.prog_id, play_pvr_info.prog_number,
                                    play_pvr_info.pid_info.cat_pid, 0x70|TRUE, NULL);
                #endif
                    if(SINGLE_CI_SLOT == g_ci_num)
                    {
                        api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used), sim_pmt_id, NULL, dmx,
                                        play_pvr_info.index, play_pvr_info.pid_v, play_pvr_info.pid_a);
                    }
                }

                if((play_pvr_info.subt_num+play_pvr_info.ttx_num+ \
                    play_pvr_info.ttx_subt_num + play_pvr_info.isdbtcc_num> 0) && (!preview_mode))
                {
        #if (SUBTITLE_ON == 1)
                    subt_register(sim_pmt_id);
        #endif

        #if (TTX_ON == 1)
                    ttx_register(sim_pmt_id);
        #endif

        #if (ISDBT_CC == 1)
                    isdbtcc_register(sim_pmt_id);
        #endif
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_SHORT);
                    // make sure subt/ttx have time to get right info from playback stream,
                    //or subt menu highlight and played be mismatched!
                }
            }
        }
    }

    if(sim_pmt_id != PVR_INVALID_SIM_ID)
    {
        ts_route.sim_num++;
        ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_PMT;
        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = play_pvr_info.pid_info.pmt_pid;
        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = sim_pmt_id;
        ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = service.prog_id;
        ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)api_sim_callback_for_playback;
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
        if(sim_cat_id != PVR_INVALID_SIM_ID)
        {
            ts_route.sim_num++;
            ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_CAT;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = play_pvr_info.pid_info.cat_pid;
            ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = sim_cat_id;
            ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = service.prog_id;
            ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)NULL;
        }
#endif
    }
    ts_route_create(&ts_route);
#endif

}
#endif


void update_ts_route_for_tms_record(struct record_prog_param *prog_info)
{
#ifdef NEW_DEMO_FRAME
    struct ts_route_info ts_route;
    INT32 ts_route_id=PVR_TS_ROUTE_INVALID_ID;
#endif
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
    UINT32 sim_cat_id = PVR_INVALID_SIM_ID;
#endif
#ifdef CC_USE_TSG_PLAYER
    UINT8 live_dmx_id = 0xFF;
#endif
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 rec_dmx_id = 0;
    struct dmx_device *dmx =NULL;
    UINT32 sim_pmt_id=PVR_INVALID_SIM_ID;
    P_NODE p_r_node;
    struct ts_route_info ts_route_m;
    struct dmx_config dmx_cfg;
    UINT32 channel_id=0;

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (0== rec_dmx_id));

    pvr_info  = api_get_pvr_info();

    rec_dmx_id= pvr_r_get_dmx_id( pvr_info->tms_r_handle);
    channel_id= pvr_info->tms_chan_id;
    #ifdef NEW_DEMO_FRAME
    MEMSET(&ts_route,0,sizeof(struct ts_route_info));
#ifdef CC_USE_TSG_PLAYER
            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            if ((RET_SUCCESS == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route)))
            {
                live_dmx_id = ts_route.dmx_id;
            }
            if (prog_info->dmx_id == live_dmx_id)
#else
            if (FALSE == prog_info->is_scrambled)
#endif
            {
                //update main play channel state!
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
                {
                    ts_route.state |= TS_ROUTE_STATUS_TMS;
                    ts_route.is_recording = 1;
                    ts_route_update(ts_route_id, &ts_route);
                }
            }
            else
            {
                MEMSET(&p_r_node,0,sizeof(P_NODE));
                MEMSET(&ts_route_m,0,sizeof(struct ts_route_info));
                MEMSET(&dmx_cfg,0,sizeof(struct dmx_config));
                if (get_prog_by_id(channel_id, &p_r_node) != SUCCESS)
                {
                    CI_PATCH_TRACE("get prog %d info failed!\n", channel_id);
                    api_stop_tms_record();
                    return;
                }

        #if(!defined( CAS9_PVR_SCRAMBLED) && !defined(CAS7_PVR_SCRAMBLE) && !defined(CAS7_ORIG_STREAM_RECORDING))
                // start monitor for scramble record
                sim_pmt_id = api_sim_start(dmx, MONITE_TB_PMT, p_r_node.tp_id,
                                           p_r_node.prog_id, p_r_node.prog_number,
                                           p_r_node.pmt_pid, FALSE, api_sim_callback);
                #endif
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
                sim_cat_id = api_sim_start(dmx, MONITE_TB_CAT, p_r_node.tp_id,
                                           p_r_node.prog_id, p_r_node.prog_number,
                                           1, FALSE, NULL);
#endif
                // for ts_route management!
                ts_route.enable = 1;
                ts_route.type = TS_ROUTE_BACKGROUND;
                ts_route.is_recording = 1;
                ts_route.dmx_id = rec_dmx_id;
                ts_route.dmx_slot_id = 0; //TODO: need check later!!!
                ts_route.nim_id = pvr_r_get_nim_id( pvr_info->tms_r_handle);
                ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);

                ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route_m);
                #if(!defined( CAS9_PVR_SCRAMBLED) && !defined(CAS7_PVR_SCRAMBLE))
                ts_route.ts_id = (TSI_TS_A == ts_route_m.ts_id) ? TSI_TS_B : TSI_TS_A;
                #else
                ts_route.ts_id = (TSI_TS_A == ts_route_m.ts_id) ? TSI_TS_A : TSI_TS_B;
                #endif
#ifdef CI_SUPPORT
                ts_route.ci_mode = sys_data_get_ci_mode();
#endif
                ts_route.ci_num = g_ci_num;
                ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
                ts_route.cia_used = 0;
                ts_route.cib_included = (((ts_route.ci_num > 1) && ((CI_SERIAL_MODE == ts_route.ci_mode) &&
                                        (TSI_TS_A == ts_route.ts_id))) || ((CI_PARALLEL_MODE == ts_route.ci_mode) &&
                                        (TSI_TS_B == ts_route.ts_id)))?1:0;
                ts_route.cib_used = 0;
                ts_route.vdec_id = 0;
                ts_route.vpo_id = 0;
                ts_route.state = TS_ROUTE_STATUS_TMS;
                ts_route.tp_id = p_r_node.tp_id;
                ts_route.prog_id = p_r_node.prog_id;
                ts_route.stream_av_mode = p_r_node.av_flag;
                ts_route.stream_ca_mode = p_r_node.ca_mode;
                ts_route.screen_mode = 1;

                if(sim_pmt_id != PVR_INVALID_SIM_ID)
                {
                    ts_route.sim_num++;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_PMT;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = p_r_node.pmt_pid;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = sim_pmt_id;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = p_r_node.prog_id;
                    ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)api_sim_callback;
#if !(defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7))
                    if(sim_cat_id != PVR_INVALID_SIM_ID)
                    {
                        ts_route.sim_num++;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].used = 1;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_type = MONITE_TB_CAT;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_pid = 1;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].sim_id = sim_cat_id;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].channel_id = p_r_node.prog_id;
                        ts_route.dmx_sim_info[ts_route.sim_num-1].callback = (UINT32)NULL;
                    }
#endif
                }

                ts_route_create(&ts_route);
            }
#endif

}

#ifdef NEW_DEMO_FRAME
void ap_pvr_update_ts_route_for_stop_record(UINT32        record_chan_id)
{
    INT32 ts_route_id = PVR_TS_ROUTE_INVALID_ID;
    struct ts_route_info ts_route;
    struct ts_route_info ts_route_temp;
    UINT32 __MAYBE_UNUSED__ ts_route_temp_id = 0;

    ts_route_temp_id = 0xFFFFFFFF;
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&ts_route_temp, 0, sizeof(struct ts_route_info));
    if(ts_route_get_record(record_chan_id, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
    {
        #ifdef FSC_SUPPORT
        fcc_update_record_state(ts_route.prog_id,0);
        #endif
        
#ifdef _INVW_JUICE
        if((ts_route.state & TS_ROUTE_STATUS_PLAY) ||(inview_stop_record))
#else
        if(ts_route.state & TS_ROUTE_STATUS_PLAY)
#endif
        {
            ts_route.state &= ~TS_ROUTE_STATUS_RECORD;
            ts_route.is_recording = 0;
            ts_route_update(ts_route_id, &ts_route);
        }
        else if(TS_ROUTE_BACKGROUND == ts_route.type) //backgound!
        {
            #ifdef MULTI_DESCRAMBLE
            #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
            if((ts_route.dmx_sim_info[0].used) && (MONITE_TB_PMT  == ts_route.dmx_sim_info[0].sim_type ))
            {
                libc_printf("%s,stop service\n",__FUNCTION__);
                api_mcas_stop_service_multi_des(record_chan_id,ts_route.dmx_sim_info[0].sim_id);   //stop ECM filter
            }
            #endif
            MEMSET(&ts_route_temp, 0, sizeof(struct ts_route_info));
            if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_temp_id, &ts_route_temp) != RET_FAILURE)
            {

                if((ts_route_temp.dmx_id != ts_route.dmx_id) && (ts_route_temp.nim_id != ts_route.nim_id))
                {
                    api_mcas_stop_transponder_multi_des(ts_route.dmx_id);//vicky201410_miss_sync E-tree
                }
            }
            #endif
            #ifdef BC_PVR_SUPPORT
            if(ts_route.dmx_sim_info[0].used && (MONITE_TB_PMT ==ts_route.dmx_sim_info[0].sim_type ))
            {
                P_NODE p_r_node;
                MEMSET(&p_r_node, 0x0, sizeof (p_r_node));
                get_prog_by_id(record_chan_id, &p_r_node);
                //BC_PVR_DEBUG("%s,stop service\n",__FUNCTION__);
                bc_stop_descrambling(p_r_node.prog_number);
                api_mcas_stop_service_multi_des(p_r_node.prog_number);   //stop ECM filter
            }
            #endif
            api_sim_stop(&ts_route);
            ts_route_delete(ts_route_id);
        }
        #ifdef FSC_SUPPORT	
        else if(TS_ROUTE_PRE_PLAY == ts_route.type)
        {
            ts_route.state &= ~TS_ROUTE_STATUS_RECORD;
            ts_route.is_recording = 0;
            ts_route_update(ts_route_id, &ts_route);
            libc_printf("%s():nothing to do for fcc pre play.\n",__FUNCTION__);
        }
        #endif            
        else
        {
            SDBBP();
        }
        
    }

}
#endif

void api_stop_play_update_ts_route( P_NODE *p_node)
{
 #ifdef NEW_DEMO_FRAME
    pvr_play_rec_t  *pvr_info = NULL;
    struct dmx_device *dmx = NULL;
    struct list_info play_pvr_info;
    INT32 ts_route_id = PVR_TS_ROUTE_INVALID_ID;
    INT32 ts_main_id = PVR_TS_ROUTE_INVALID_ID;
    struct dmx_config dmx_cfg;
    struct ts_route_info ts_route;
    struct ts_route_info ts_backgrade_route;
#endif
    P_NODE playing_pnode;
    struct io_param io_parameter;
    UINT16 pid_list[4]={0};

    MEMSET(&io_parameter,0,sizeof(io_parameter));
#ifdef NEW_DEMO_FRAME
    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    ap_get_playing_pnode(&playing_pnode);
    api_get_play_pvr_info(&play_pvr_info);
    pvr_info  = api_get_pvr_info();
    MEMSET(&dmx_cfg, 0x0, sizeof (dmx_cfg));
    MEMSET(&ts_backgrade_route, 0x0, sizeof (ts_backgrade_route));
    MEMSET(&ts_route, 0x0, sizeof (ts_route));
#ifdef PIP_SUPPORT
        BOOL pip_opened_after_playback=TRUE;
        if(pip_opened_after_playback)
        {
            ap_pip_exit();
            api_set_system_state(SYS_STATE_NORMAL);
        }
#endif
            if(ts_route_get_by_type(TS_ROUTE_PLAYBACK, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
            {
                if ((play_pvr_info.subt_num+play_pvr_info.ttx_num +
                    play_pvr_info.ttx_subt_num + play_pvr_info.isdbtcc_num > 0) &&
                    (ts_route.dmx_sim_info[0].used))
                {
                #if (SUBTITLE_ON == 1)
                    subt_unregister(ts_route.dmx_sim_info[0].sim_id);
                #endif
                #if (TTX_ON == 1)
                    ttx_unregister(ts_route.dmx_sim_info[0].sim_id);
                #endif
                #if (ISDBT_CC == 1)
                    isdbtcc_unregister(ts_route.dmx_sim_info[0].sim_id);
                #endif
                }
                api_sim_stop(&ts_route);
                if(ts_route.tsi_id == ts_route_get_nim_tsiid(PVR_TS_ROUTE_NIM_ID_MAX))
                {
                    if(ts_route_get_by_type(TS_ROUTE_BACKGROUND, (UINT32 *)&ts_main_id, &ts_backgrade_route) != RET_FAILURE)
                    {
                        tsi_dmx_src_select(ts_backgrade_route.dmx_id+1,ts_backgrade_route.ts_id);
                        tsi_select(ts_backgrade_route.ts_id,ts_backgrade_route.tsi_id);
                    }
                }
                ts_route_delete(ts_route_id);
            }
            if(pvr_get_playback_ts_route_id() != PVR_TS_ROUTE_INVALID_ID)
            {
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                if(ts_route_get_by_id(pvr_get_playback_ts_route_id(), &ts_route) != RET_FAILURE)
                {
                    if(ts_route.ts_id_recover)
                    {
                        ts_route.ts_id = 3 - ts_route.ts_id;
                    }
                    pid_list[1] = playing_pnode.audio_pid[playing_pnode.cur_audio];
                    pid_list[0] = 0x1fff;
                    pid_list[2] = 0x1fff;
                    pid_list[3] = 0x1fff;
                    io_parameter.io_buff_in = (UINT8 *)pid_list;
                    #ifdef FSC_SUPPORT
                    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id),
                        IO_CHANGE_AUDIO_STREAM, (UINT32)&io_parameter);
                    #else
                    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (0== ts_route.dmx_id)?0:1),
                        IO_CHANGE_AUDIO_STREAM, (UINT32)&io_parameter);
                    #endif
                    ap_set_main_dmx(FALSE, ts_route.dmx_id);    
                    ts_route.type = TS_ROUTE_MAIN_PLAY;
                    ts_route.state |=     TS_ROUTE_STATUS_PLAY;
                    if((1 == ts_route.dmx_sim_info[0].used) && (MONITE_TB_PMT == ts_route.dmx_sim_info[0].sim_type))
                    {
                    #if (SUBTITLE_ON == 1)
                        subt_register(ts_route.dmx_sim_info[0].sim_id);
                    #endif
                    #if (TTX_ON == 1)
                        ttx_register(ts_route.dmx_sim_info[0].sim_id);
                    #endif
                    #if (ISDBT_CC == 1)
                        isdbtcc_register(ts_route.dmx_sim_info[0].sim_id);
                    #endif
                        osal_task_sleep(500); // Fix bug#82010. Getting subtitle pid list failed when starting record due to it's internal param not set up.
                    }
                    if (DB_SUCCES == get_prog_by_id(ts_route.prog_id, p_node))
                    {
                        epg_off();  //stop the monitor of dmx_3 for pvr record.
                        epg_reset();
                        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id);
    #ifdef _INVW_JUICE
                        epg_on_by_inview(dmx, p_node->sat_id,p_node->tp_id,p_node->prog_number, 0);
    #else
                        epg_on_ext(dmx, p_node->sat_id,p_node->tp_id,p_node->prog_number);
    #endif // _INVW_JUICE
                    }
                    if(( pvr_info->tms_r_handle)
#ifdef CI_PLUS_SUPPORT // active to use CAM if only one program even is FTA
                        && ((1 == ts_route_get_num()) || (TRUE == ts_route.stream_ca_mode))
#else
                        && (TRUE ==  ts_route.stream_ca_mode)
#endif
                        )
                    {
                        if(SINGLE_CI_SLOT == g_ci_num )
                        {
#ifdef CI_PLUS_SUPPORT
                            if(FALSE == ts_route.cia_used )
                            {
                                ts_route.ts_id = TSI_TS_A;
                                ts_route.cia_used = 1;
                                tsi_dmx_src_select((0 == ts_route.dmx_id) ? TSI_DMX_0 : TSI_DMX_1, ts_route.ts_id);
                                ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);
                                tsi_select(ts_route.ts_id,ts_route.tsi_id);
                                api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used),
                                    ts_route.dmx_sim_info[0].sim_id, dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id),
                                    dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id), ts_route.prog_id,
                                    playing_pnode.video_pid, playing_pnode.audio_pid[playing_pnode.cur_audio]);
                            }
                            else
                            {
                                ts_route.ts_id = TSI_TS_A;
                                ts_route.cia_used = 1;
                                tsi_dmx_src_select((0 == ts_route.dmx_id) ? TSI_DMX_0 : TSI_DMX_1, ts_route.ts_id);
                                ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);
                                tsi_select(ts_route.ts_id,ts_route.tsi_id);
                             }
#else
                            ts_route.ts_id = TSI_TS_A;
                            ts_route.cia_used = 1;
                            tsi_dmx_src_select((0 == ts_route.dmx_id) ? TSI_DMX_0 : TSI_DMX_1, ts_route.ts_id);
                            ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);
                            tsi_select(ts_route.ts_id,ts_route.tsi_id);
                            api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used),
                                ts_route.dmx_sim_info[0].sim_id, dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id),
                                dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id), ts_route.prog_id,
                                playing_pnode.video_pid, playing_pnode.audio_pid[playing_pnode.cur_audio]);
#endif
                        }
                        else if (DUAL_CI_SLOT == g_ci_num) // for 2ci//back to scramble prog
                        {
#ifdef CC_USE_TSG_PLAYER
                            if ((ts_route.state & TS_ROUTE_STATUS_USE_CI_PATCH) && cc_tsg_task_is_running())
                            {
                                CI_PATCH_TRACE("CI patch running, not send ci message\n");
                            }
                            else
#endif
                            {
                                ts_route_update(pvr_get_playback_ts_route_id(), &ts_route);
                                stop_ci_delay_msg();
                                CI_PATCH_TRACE("%s() send %d at line %d\n", __FUNCTION__, OP_CI_TS_TYPE_CHG, __LINE__);
                                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_TS_TYPE_CHG, FALSE);
                            }
                        }
                        //pvr_r_resume(pvr_info.tms_r_handle);
                    }
                    else if (DUAL_CI_SLOT == g_ci_num)// back to free prog under 2ci
                    {
                    #ifdef CC_USE_TSG_PLAYER
                        if ((ts_route.state & TS_ROUTE_STATUS_USE_CI_PATCH) && cc_tsg_task_is_running())
                        {
                            CI_PATCH_TRACE("CI patch running, not send ci message\n");
                        }
                        else
                    #endif
                        {
                            ts_route_update(pvr_get_playback_ts_route_id(), &ts_route);
                            tsi_dmx_src_select((0 ==  ts_route.dmx_id) ? TSI_DMX_0 : TSI_DMX_1, ts_route.ts_id);
                            ts_route.tsi_id = ts_route_get_nim_tsiid(ts_route.nim_id);
                            tsi_select(ts_route.ts_id, ts_route.tsi_id);
                            stop_ci_delay_msg();
                            CI_PATCH_TRACE("%s() send %d at line %d\n", __FUNCTION__, OP_CI_TS_TYPE_CHG, __LINE__);
                            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_TS_TYPE_CHG, FALSE);
                        }
                    }
                    ts_route_update(pvr_get_playback_ts_route_id(), &ts_route);
                }
            }
            if(FALSE ==pvr_set_playback_ts_route_id(PVR_TS_ROUTE_INVALID_ID))
            {
                return ;
            }
            if(api_pvr_check_tsg_state())
            {
                if(api_check_ts_by_dmx(1 - pvr_p_get_dmx_id( pvr_info->play.play_handle), TSI_TS_B, 0,
                    FALSE, FALSE, FALSE) != TRUE)
                {
                    return;
                }
            }
#endif
}

BOOL api_prepare_tsg_playback_tsi_info(UINT8 *dmx_id,UINT8 *ts_id)
{
    UINT16 i=0;
    pvr_record_t *rec=NULL;
    UINT32 ts_route_id=0;
    BOOL ts_a_recording=FALSE;
    BOOL ts_b_recording=FALSE;
    struct ts_route_info ts_route;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 rec_pos=1;

    pvr_info  = api_get_pvr_info();
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

    for (i = 0; i < CC_MAX_TS_ROUTE_NUM; ++i)
    {
        if (( RET_SUCCESS ==ts_route_get_by_id(i, &ts_route)) && (TRUE == ts_route.is_recording ))
        {
            if (TSI_TS_A == ts_route.ts_id )
            {
                ts_a_recording = TRUE;
            }
            else
            {
                if (TSI_TS_B == ts_route.ts_id  )
                {
                    ts_b_recording = TRUE;
                 }
            }
        }
    }
    if ((ts_a_recording) && (ts_b_recording))
    {
        return FALSE;
    }
    else
    {
        *ts_id = ts_a_recording ? TSI_TS_B : TSI_TS_A;
    }

    if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
    {
        #ifdef CI_SUPPORT
        //check TS used for TS-Gen playback!
        if(CI_SERIAL_MODE == sys_data_get_ci_mode() )
        {
            *ts_id = TSI_TS_A;
        }
        #endif
        //check dmx_id used for TS-Gen playback
        if(RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, \
                                    &ts_route_id, &ts_route))
        {
            PRINTF("ts_route_get_by_type() failed!\n");
        }
        if(ts_route.is_recording) //main screen recording
        {
            if(ts_route.state & TS_ROUTE_STATUS_RECORD)
            {
                if(((SINGLE_CI_SLOT == g_ci_num)&&((ts_route.cia_used) || (ts_route.cib_used) )
                        && (TRUE == ts_route.stream_ca_mode )
                        && (TRUE == sys_data_get_scramble_record_mode()))
                    ||((DUAL_CI_SLOT == g_ci_num)
                        &&(PVR_MAX_RECORDER == pvr_info->rec_num)
                        && (TRUE == ts_route.stream_ca_mode )
                        &&(TRUE == sys_data_get_scramble_record_mode()))) // 2ci have no dmx to tsgen under free mode
                {
                    return FALSE;
                }
                else // tsgen playback on 2ci always is ok
                {
                    *dmx_id = 1 - ts_route.dmx_id;
                }
            }
            else // tms on this dmx
            {
                *dmx_id = 1 - ts_route.dmx_id;
                if(*ts_id == ts_route.ts_id)
                {
                    ts_route.ts_id = 3 - ts_route.ts_id;
                    ts_route_update(ts_route.id, &ts_route);
                }

            }
            if(api_pvr_get_rec_num_by_dmx(*dmx_id) > 0)
            {
                return FALSE;
            }
        }
        else //can delete main ts_route
        {
            *dmx_id = ts_route.dmx_id;
            *ts_id = ts_route.ts_id;
            //stop play
            api_stop_play_prog(&ts_route);
        }
    }
    else
    {
        #ifdef CI_SUPPORT
            // play from menu or other playback
            if(CI_SERIAL_MODE == sys_data_get_ci_mode())
            {
                *ts_id = TSI_TS_A;
            }
        #endif
        rec = api_pvr_get_rec_by_dmx(*dmx_id, rec_pos);
        if(NULL == rec)
        {
            rec_pos++;
            rec = api_pvr_get_rec_by_dmx(*dmx_id, rec_pos);
        }
#ifndef CC_USE_TSG_PLAYER
        if (SINGLE_CI_SLOT == g_ci_num)
        {//not valid when record as descrambled now!
            if((rec != NULL) && (rec->ca_mode) && (!rec->rec_descrambled))
            {
                return FALSE;
            }
        }
        else if (( DUAL_CI_SLOT == g_ci_num  )
                        &&(PVR_MAX_RECORDER == pvr_info->rec_num)
                        && (TRUE == ts_route.stream_ca_mode )
                        &&(TRUE == sys_data_get_scramble_record_mode()))
        {
            return FALSE;
        }
#else
        if (( pvr_info->rec_num > 0) && ( TRUE == sys_data_get_scramble_record_mode()))
        {
            for (i = 0; i < PVR_MAX_RECORDER; ++i)
            {
                if (( pvr_info->rec[i].record_chan_flag) && ( pvr_info->rec[i].ca_mode) &&
                    (0 == pvr_info->rec[i].is_scrambled ))
                {
                    return FALSE;
                }
            }
        }
#endif

        //check dmx_id used for TS-Gen playback
        if(rec != NULL)
        {
            // this dmx is recording
            *dmx_id = 1 - *dmx_id;
            if(api_pvr_get_rec_num_by_dmx(*dmx_id) > 0)
            {
                return FALSE;
            }
            else if(( pvr_info->tms_r_handle) && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) == *dmx_id))
            {
                // need stop this tms
                api_pvr_tms_proc(FALSE);
            }
        }
        else if(( pvr_info->tms_r_handle) && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) == *dmx_id))
        {
            // this dmx is tms, check other dmx first
            if(api_pvr_get_rec_num_by_dmx(1 - *dmx_id) > 0)
            {
                // stop tms dmx and make it for ts-gen playback
                api_pvr_tms_proc(FALSE);
            }
            else
            {
                *dmx_id = 1 - *dmx_id;
            }
        }
    }
    return TRUE;
}
