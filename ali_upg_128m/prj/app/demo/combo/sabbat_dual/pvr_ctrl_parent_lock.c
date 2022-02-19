/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_parent_lock.c

*    Description: The parent lock issues in pvr will be treated in this file.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "pvr_ctrl_parent_lock.h"
#include <api/libtsi/sie.h>
#ifdef PARENTAL_SUPPORT
#include "win_pvr_record_manager.h"
#endif
#include "win_password.h"
#include "win_com.h"
#include "copper_common/com_epg.h"

#ifdef PARENTAL_SUPPORT
#define RATING_VALUE_AGE10          1
#define RATING_VALUE_AGE12          2
#define RATING_VALUE_AGE16          3
#define RATING_NON_STANDARD         5
#define CC_STATE_STOP               0
#define CC_STATE_PLAY_TV            1
#define CC_STATE_PLAY_RADIO         2

static UINT16 pvr_pre_channel = 0xffff;
static BOOL pvr_rating_check_eit_flag = FALSE;//TRUE:Enable playback rating check and disable live play rating check
static BOOL pvr_rating_check_lock_player = FALSE;       // TRUE : Enable lock mode of PVR player
static BOOL pvr_rating_lock_force_unlock_flag = FALSE;  // TRUE : first to run force UNLOCK function
extern BOOL eit_in_parsing_check;

BOOL api_pvr_get_rating_lock_eit_flag(void)
{
    return pvr_rating_check_eit_flag;
}

// Get rating value (age limit) from infor.dvr
UINT32 api_pvr_get_rating(PVR_HANDLE handle)
{
    UINT16 rl_idx = 0;
    UINT32 ptm = 0;

    if (handle)
    {
        rl_idx = pvr_get_index(handle);
        ptm = pvr_p_get_time(handle);
        return pvr_get_agelimit(rl_idx, ptm);
    }
    else
    {
        return 0;
    }
}

// Set rating value (age limit) to infor.dvr
void api_pvr_set_rating(PVR_HANDLE handle, UINT32 rating)
{
    UINT16 rl_idx = 0;
    UINT32 rtm = 0;

    if (handle)
    {
        rl_idx = pvr_get_index(handle);
        rtm = pvr_r_get_time(handle);
        pvr_set_agelimit(rl_idx, rtm, rating);
    }
}

// Force unlock rating if at rating lock state
void api_pvr_force_unlock_rating(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

#ifdef RATING_LOCK_WITH_PASSWORD
    set_passwd_status(FALSE);
#endif

    pvr_info  = api_get_pvr_info();
    if(get_rating_lock()&&( pvr_info->play.play_handle != (PVR_HANDLE)NULL))        //restore to  unlock state
    {
        pvr_rating_lock_force_unlock_flag = TRUE;
        //pvr_p_lock_switch( pvr_info->play.play_handle, (UINT32)FALSE);
        api_pvr_p_lock_switch(FALSE);
        set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
        if (TRUE == get_signal_stataus_show())
        {
            show_signal_status_osdon_off(0);
        }
        pvr_rating_lock_force_unlock_flag = FALSE;
    }
    pvr_rating_check_eit_flag = FALSE;
}

// PVR rating lock check for preview playback mode and only use for first check
BOOL pvr_playback_preview_rating_check(PVR_HANDLE handle)
{
    UINT32 rating_value = 0;
    UINT32 rating_age = 0;
    UINT32 user_rating_value = 0;
    UINT32 user_rating_age = 0;
    BOOL ret = FALSE;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();
    rating_value = api_pvr_get_rating(handle);

    //===get rating====
    if (0 == rating_value) //no rating
    {
        ret = FALSE;
    }
    else//4
    {
        rating_age = rating_value + 3;
        user_rating_value = sys_data->rating_sel;
#ifdef CSA_PARENTAL_CONTROL
        if (user_rating_value > 0) //remove manual select
        {
            if (RATING_VALUE_AGE10 == user_rating_value) //age10
            {
                user_rating_age = user_rating_value + 9;
            }
            else if (RATING_VALUE_AGE12 == user_rating_value) //age12
            {
                user_rating_age = user_rating_value + 10;
            }
            else if (RATING_VALUE_AGE16 == user_rating_value) //age16
            {
                user_rating_age = user_rating_value + 13;
            }
            else //age18
            {
                user_rating_age = user_rating_value + 14;
            }
        }
#else
        if (user_rating_value > 0)   //remove manual select
        {
            user_rating_age = user_rating_value + 3; //rating 4~18
        }
#endif
        else // user set off
        {
            ret = FALSE;
            return ret;
        }
        if (rating_age >= (user_rating_age & 0xff))
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

    }
    return ret;
}

void reset_pvr_rating_channel(void)
{
    pvr_pre_channel = 0xffff;
}

#ifdef PVR3_SUPPORT
BOOL pvr_control_update_event_name(eit_event_info_t *ep)
{
    UINT8 i = 0;
    UINT16 rl_idx = 0;
    UINT16 *p_name = NULL;
    BOOL ret = FALSE;
    BOOL pvr_going = FALSE;
    INT32 name_len = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    p_name = (UINT16 *)epg_get_event_name(ep, &name_len);
    for (i = 0; i < PVR_MAX_RECORDER; i++)
    {
        if ((PVR_HANDLE)NULL == pvr_info->rec[i].record_handle)
        {
            continue;
        }
        pvr_going = TRUE;
        rl_idx = pvr_get_index( pvr_info->rec[i].record_handle);

        if (NULL != p_name)
        {
            pvr_set_event_name(rl_idx, (UINT8 *)p_name);
            ret = TRUE;
        }
    }

    if (pvr_going)
    {
        return ret;
    }
    else
    {
        return TRUE;
    }
}
#endif

// PVR rating lock check for all playback mode
BOOL pvr_rating_check(UINT16 cur_channel)
{
    eit_event_info_t *ep = NULL;
    UINT8 tv_rating = 0;
    UINT8 tv_setting = 0;
    UINT8 user_setting = 0;
    UINT8 rating = 0;
    BOOL ret = FALSE;
    BOOL ret_tmp = FALSE;
    SYSTEM_DATA *sys_data = NULL;
    BOOL check = FALSE;
    UINT32 tp_id = 0;
    struct dmx_device *dmx0 = NULL;
    struct dmx_device *dmx2 = NULL;
    pvr_play_rec_t  *pvr_info = NULL;
    struct list_info *play_pvr_info=NULL;
    pvr_info  = api_get_pvr_info();
    P_NODE playing_pnode;
    struct ts_route_info l_ts_route;
    UINT32 dmx_id = 0;
    struct dmx_device *playing_dmx = NULL;
    static UINT32 last_start_mjd = 0;

    //MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    dmx0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    dmx2 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 2);
    //api_get_play_pvr_info(&play_pvr_info);
    play_pvr_info = api_get_g_play_pvr_info();

    //MEMSET(&playing_pnode, 0x0, sizeof(playing_pnode));
    //MEMSET(&l_ts_route, 0x0, sizeof(l_ts_route));
    l_ts_route.dmx_id = 0;
    if (RET_SUCCESS == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &l_ts_route))
    {
        dmx_id = l_ts_route.dmx_id;
    }
    playing_dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    if((FALSE == pvr_info->hdd_valid) || ((PVR_HANDLE)NULL == pvr_info->play.play_handle) )
    {
        if (pvr_rating_check_eit_flag)
        {
            // Off epg of playback
            epg_off_ext();
            epg_reset();
            pvr_rating_check_eit_flag = FALSE;
            pvr_pre_channel = 0;
            
            //MEMSET(&playing_pnode, 0x0, sizeof(playing_pnode));
            // @ TMS, On epg of live play
            playing_pnode.sat_id = 0;
            playing_pnode.tp_id = 0;
            playing_pnode.prog_number = 0;
            if((PVR_HANDLE)NULL !=  pvr_info->tms_r_handle  )
            {                
                sys_data_get_curprog_info(&playing_pnode);
#ifdef _INVW_JUICE
                epg_on_by_inview(playing_dmx, playing_pnode.sat_id, playing_pnode.tp_id, playing_pnode.prog_number, 0);
#else
                epg_on_ext(playing_dmx, playing_pnode.sat_id, playing_pnode.tp_id, playing_pnode.prog_number);
#endif // _INVW_JUICE
            }
#ifndef NEW_DEMO_FRAME
            else if ((!eit_in_parsing_check) && (CC_STATE_PLAY_TV == get_cc_crnt_state()))
#else
            else if (!eit_in_parsing_check)
#endif
            {
                sys_data_get_curprog_info(&playing_pnode);
#ifdef _INVW_JUICE
                epg_on_by_inview(playing_dmx, playing_pnode.sat_id, playing_pnode.tp_id, playing_pnode.prog_number, 0);
#else
                epg_on_ext(playing_dmx, playing_pnode.sat_id, playing_pnode.tp_id, playing_pnode.prog_number);
#endif // _INVW_JUICE
            }
        }
        return ret;
    }

    if (dmx2 != NULL)
    {
        if (!pvr_rating_check_eit_flag)
        {
            // Off epg of live play
            epg_off();
            epg_reset();
            // Open SIE of DMX2
            sie_open_dmx(dmx2, 27, NULL, 0);
            pvr_rating_check_eit_flag = TRUE;
            pvr_rating_check_lock_player = TRUE;
            pvr_pre_channel = 0;
        }

        if (pvr_pre_channel != cur_channel)
        {
            // Off epg of DMX2
            epg_off_ext();
            epg_reset();

            // On epg of playback. playback scrambled program use DMX0
            if((play_pvr_info->is_scrambled) && ((PVR_HANDLE)NULL == pvr_info->tms_r_handle))
            {
                pvr_epg_on(dmx0, cur_channel);
            }
            else
            {
                pvr_epg_on(dmx2, cur_channel);
            }
            pvr_pre_channel = cur_channel;
        }
    }

    ep = epg_get_service_event(tp_id, cur_channel, PRESENT_EVENT, NULL, NULL, NULL, TRUE);
    if (ep != NULL)
    {
        if (ep->rating != 0)
        {
            rating = ep->rating;
            check = TRUE;
        }
#ifdef PVR3_SUPPORT
        if ((ep->event_id != 0)  && (ep->mjd_num != last_start_mjd))
        {
            #if (defined CAS9_PVR_SUPPORT)
            if(!api_pvr_is_playing())//AS project playback not support update event name    
            #endif
            {
                pvr_control_update_event_name(ep);
                last_start_mjd = ep->mjd_num;
            }
            
        }
#endif
    }

    if (check)
    {
        sys_data = sys_data_get();
#if (defined (DVBT_SUPPORT) || defined(DVBC_SUPPORT))
        if (0 == rating) //no rating
        {
            tv_rating = 0;
            ret = FALSE;
            return ret;
        }
        else//4 //4~18
        {
            tv_rating = rating + 3;
        }
#endif
#ifdef ISDBT_SUPPORT
        if ((0 == rating) || (rating > RATING_NON_STANDARD)) //no rating //non-standard
        {
            tv_rating = 0;
            ret = FALSE;
            return ret;
        }
        else
        {
            tv_rating = 6 + rating * 2;
        }
#endif
        //===get setting====
#ifdef CSA_PARENTAL_CONTROL
        user_setting = sys_data->rating_sel & 0x0f; //0:off,1:<10>:2:<12>,3:<14>,4:<16>:5:<18>
        if (0 == user_setting) //rating off
        {
            user_setting = 0xff;
        }
        else
        {
            user_setting = 8 + user_setting * 2;
        }
#else
        if (user_setting > 0) //remove manual select
        {
            user_setting = user_setting + 3; //rating 4~18
        }
        else
        {
            ret = FALSE;
            return ret;
        }
#endif
        tv_setting = user_setting & 0xff;
        if (tv_rating > tv_setting)
        {
            ret_tmp = TRUE;
        }
        if (ret_tmp) // && !GetPasswdStatus())
        {
            ret = TRUE;
        }
    }

    if (ret)
    {
#if (SUBTITLE_ON == 1)
        api_subt_show_onoff(!ret);
#endif

#if (TTX_ON ==1)
        api_ttx_show_onoff(!ret);
#endif
    }
    return ret;
}

// Set PVR rating lock playback state and show lock string but not support preview mode and STEP state
//UISetPVRRatingLock
void  pvr_ui_set_rating_lock(BOOL lock)
{
    PVR_STATE play_state = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if((FALSE == pvr_info->hdd_valid) || ((PVR_HANDLE)NULL == pvr_info->play.play_handle  ))
    {
        return;
    }
    play_state = pvr_p_get_state( pvr_info->play.play_handle);

    // Preview @ PVR mrg, don't use this function
    if ((NV_STEP == play_state) && (VIEW_MODE_PREVIEW == hde_get_mode()))
    {
        return;
    }

#ifdef RATING_LOCK_WITH_PASSWORD
    if (get_passwd_status())
    {
        return;
    }
#endif
 
#ifdef RATING_LOCK_WITH_PASSWORD
    if (((get_rating_lock() != lock) || (get_passwd_status()) || (!pvr_rating_check_lock_player))
                    && (!pvr_rating_lock_force_unlock_flag))
#else
    if (((get_rating_lock() != lock) || (!pvr_rating_check_lock_player)) && (!pvr_rating_lock_force_unlock_flag))
#endif
    {
#ifdef RATING_LOCK_WITH_PASSWORD
        if (lock)
        {
            set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
            // If Playback state is FB, FF, SLOW, B_SLOW and STEP state and will change to rating lock mode,
        // playback state will change to PLAY state.
            //play_state = pvr_p_get_state(pvr_info.play.play_handle);
            if (((NV_FF == play_state)   || (NV_FB == play_state) || (NV_SLOW == play_state)
                   || (NV_REVSLOW == play_state) || (NV_STEP == play_state)) && (VIEW_MODE_PREVIEW != hde_get_mode()))
            {
                pvr_p_play( pvr_info->play.play_handle);
            }
            //pvr_p_lock_switch( pvr_info->play.play_handle, TRUE);
            api_pvr_p_lock_switch(TRUE);
            win_pvr_mrg_draw_lock_preview(TRUE);

            if (win_msg_popup_opend())
            {
                //Bug fixed BUG44322:
                //It's no action when the parental lock status is changed during a dialog is showing.
                set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
            }
            else
            {
                win_set_pwd_caller(WIN_PWD_CALLED_PVR);
                if (win_pwd_open(NULL, 0))
                {
                    //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);
                    api_pvr_p_lock_switch(FALSE);
                    win_pvr_mrg_draw_lock_preview(FALSE);
                    set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
                    if (!pvr_rating_check_lock_player)
                    {
                        pvr_rating_check_lock_player = TRUE;
                    }

                }
                win_set_pwd_caller(WIN_PWD_CALLED_UNKOWN);
            }

        }
        else
        {
            set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
            //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);
            api_pvr_p_lock_switch(FALSE);
            win_pvr_mrg_draw_lock_preview(FALSE);
        }
#else
        if (lock)
        {
            set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
            // If Playback state is FB, FF, SLOW, B_SLOW and STEP state and will change to rating lock mode,
            // playback state will change to PLAY state.
            //play_state = pvr_p_get_state(pvr_info.play.play_handle);
            if (((NV_FF == play_state) || (NV_FB == play_state) || (NV_SLOW == play_state)
                 || (NV_REVSLOW == play_state) || (NV_STEP == play_state)) && (VIEW_MODE_PREVIEW != hde_get_mode()))
            {
                pvr_p_play( pvr_info->play.play_handle);
            }
        }
        else
        {
            set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
        }

       // pvr_p_lock_switch( pvr_info->play.play_handle, lock);
       api_pvr_p_lock_switch(lock);

        win_pvr_mrg_draw_lock_preview(lock);
#endif

        if (!pvr_rating_check_lock_player)
        {
            pvr_rating_check_lock_player = TRUE;
        }
    }
}
#endif// PARENTAL_SUPPORT

