/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_parental.c
*
*    Description: The common function of parental
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifndef _BUILD_OTA_E_
#ifdef PARENTAL_SUPPORT

#include <api/libosd/osd_lib.h>
#include <api/libosd/obj_container.h>
#include <api/libsi/psi_pmt.h>
#include "ctrl_play_channel.h"
#include "win_signalstatus.h"
#include "win_password.h"
#include "win_com.h"
#include <api/libdb/db_interface.h>
#include "control.h"

#define STREAM_RATING_TEN                0x02
#define STREAM_RATING_TWELVE            0x03
#define STREAM_RATING_FOURTEEN        0x04
#define STREAM_RATING_SIXTEEN            0x05
#define STREAM_RATING_EIGHTEEN        0x06

#define CONTENT_DRUG                0x1
#define CONTENT_VIOLENCE            0x2
#define CONTENT_VIOLENCE_DRUG        0x3
#define CONTENT_SEX                    0x4
#define CONTENT_SEX_DRUG            0x5
#define CONTENT_SEX_VIOLENCE        0x6
#define CONTENT_SEX_VIOLENCE_DRUG        0x7

#define CC_STATUS_STOP            0
#define CC_STATUS_PLAY_TV        1
#define CC_STATUS_PLAY_RADIO    2

static BOOL passwd_status = FALSE;
static BOOL pvr_r_passwd_status = FALSE;
static BOOL chan_lock_status=FALSE;
static UINT8 cnt_rating_stream_value=FALSE;

void set_passwd_status(BOOL flag)// 1 for pass, 0 not pass
{
    passwd_status = flag;
}

BOOL get_passwd_status(void)
{
    return passwd_status;//Passwd_ok
}

void set_pvr_rating_pwd_status(BOOL flag)// 1 for pass, 0 not pass
{
    pvr_r_passwd_status = flag;
}

BOOL get_pvr_rating_pwd_status(void)
{
    return pvr_r_passwd_status;//Passwd_ok
}

void set_chan_lock_status(BOOL flag)// 1 for pass, 0 not pass
{
    chan_lock_status = flag;
}

BOOL get_chan_lock_status(void)
{
    return chan_lock_status;//Passwd_ok
}

void set_current_stream_rating(UINT8 rating)
{
    cnt_rating_stream_value = rating;
}

UINT8 get_current_stream_rating(void)
{
    return cnt_rating_stream_value;
}

/**************************
for rating check in realtime
***************************/

static BOOL rating_lock_change_channel_flag = FALSE;        // TRUE : first to run force UNLOCK function

#ifdef POLAND_SPEC_SUPPORT
UINT8  find_parent_age_setting(UINT8 select_id)
{
    UINT8 ret;
    switch(select_id)
    {
        case 0:
            ret = 0xff;
            break;
        case 1:
            ret = 7;
            break;
        case 2:
            ret = 12;
            break;
        case 3:
            ret = 16;
            break;
        case 4:
            ret = 18;
            break;
        default:
            break;
    }
    return ret;
}
#endif

BOOL check_rating_lock(UINT8 stream_rating)
{
    UINT8 stream_age = stream_rating&0xf;
    SYSTEM_DATA *sys_data = sys_data_get();
    //0:off,1:<10>:2:<12>,3:<14>,4:<16>:5:<18>
    UINT8 age_setting = sys_data->rating_sel&0x0f;

#ifndef POLAND_SPEC_SUPPORT
    if(0 == age_setting)//rating off
    {
        age_setting = 0xff;
    }
    else
    {
        age_setting = 8 + age_setting*2;
    }
#else
        age_setting =find_parent_age_setting(age_setting);
#endif
#ifdef ISDBT_SUPPORT
    UINT8 max_stream_age = 5;
    if(max_stream_age < stream_age)
    {
        stream_age = 0; //non-standard
    }
    else
    {
        stream_age = 6 + stream_age*2;
    }
#endif

#if (defined (DVBT_SUPPORT) || defined(DVBC_SUPPORT))
    if(stream_age > 0)//no rating
    {
        stream_age = stream_age + 3;
    }
#endif
    return ((stream_age > 0) && (stream_age > age_setting));
}

BOOL check_content_lock(UINT8 stream_rating)
{
    UINT8 s_content_type[] =
    {
        0, CONTENT_DRUG, CONTENT_VIOLENCE, CONTENT_SEX, CONTENT_VIOLENCE_DRUG,
        CONTENT_SEX_DRUG, CONTENT_SEX_VIOLENCE, CONTENT_SEX_VIOLENCE_DRUG,
    };
    UINT8 stream_content = stream_rating>>4;
    SYSTEM_DATA *sys_data = sys_data_get();
    UINT8 content_setting = sys_data->rating_sel>>4;

    if(stream_content)
    {
        if(content_setting >= ARRAY_SIZE(s_content_type))
        {
            content_setting = 0;
        }
        content_setting = s_content_type[content_setting];
    }
    return ((stream_content & content_setting) != 0);
}

BOOL check_epg_rating(UINT8 stream_rating)
{
    if(stream_rating <= 1) //0: reserve, 1: free
    {
        return FALSE;
    }
    if(check_rating_lock(stream_rating))
    {
        return TRUE;
    }
    return check_content_lock(stream_rating);
}

BOOL rating_check(UINT16 cur_channel, BOOL all_check __MAYBE_UNUSED__)
{
    eit_event_info_t *ep=NULL;
    UINT32 stream_rating=0;
    BOOL ret = FALSE;
    BOOL check = FALSE;
    UINT32 pmt_rating=0;

    set_current_stream_rating(0);//reset the stream rating
#ifdef RECORD_SUPPORT
    if(api_pvr_get_rating_lock_eit_flag() == TRUE)
    {
        ret = FALSE;
        return ret;
    }
#endif
    ep = epg_get_cur_service_event(cur_channel, PRESENT_EVENT, NULL,NULL,NULL, TRUE);
    pmt_rating= pmt_get_rating();

    if((ep!=NULL) || (pmt_rating>0)) //with rating exist
    {
        //pmt
        if(pmt_rating>1)//0 :reserver,1:free
        {
                stream_rating =pmt_rating;
                set_current_stream_rating(stream_rating = pmt_rating);//stream with  rating 1~15
                check = TRUE;
        }
        //eit
        else if(ep!=NULL)
        {
             if (ep->rating>1 )//0 :reserver,1:free
            {
                stream_rating = ep->rating;
                set_current_stream_rating(stream_rating = ep->rating);//stream with  rating 1~15
                check = TRUE;
            }
            else
            {
                ep->rating = 0;
                stream_rating = ep->rating;
                set_current_stream_rating(stream_rating = ep->rating);//stream no rating
                check = FALSE;
            }
        }
        else
        {
            check=FALSE;
        }
    }

    if (check)
    {
        ret = check_epg_rating(stream_rating);
    }
    return ret;
}

//extern CONTAINER     win_pwd_con;

static BOOL pre_ratinglock=FALSE;

void clear_pre_ratinglock(void)
{
    pre_ratinglock = FALSE;
}

void uiset_rating_lock(BOOL lock)
{
    BOOL __MAYBE_UNUSED__ pre_chanlock=FALSE;
    UINT8 av_flag = sys_data_get_cur_chan_mode();
    INT32 max_channel = 0;
    static UINT32 pre_prog_id= 0xffffffff;
    P_NODE p_node;
    UINT16 channel=sys_data_get_cur_group_cur_mode_channel();
    UINT32 cur_prog_id = 0;
    UINT32 chan_id = 0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    struct dmx_device *dmx = NULL;
    BOOL *need_check_tms=NULL;

#ifndef _BUILD_OTA_E_
    struct ft_frontend frontend;
    struct cc_param chchg_param;
    BOOL ci_start_service = FALSE;
    BOOL b_force=0;
    UINT32 ts_route_id = 0;
    struct ts_route_info ts_route;
    UINT32 cur_sim_id = 0;

    //MEMSET(&frontend, 0x0, sizeof(struct ft_frontend));
    //MEMSET(&chchg_param, 0x0, sizeof(struct cc_param));
    //MEMSET(&ts_route, 0x0, sizeof(ts_route));
    ts_route.prog_id = 0;
#endif

    need_check_tms=api_get_need_check_tms();
    //MEMSET(&p_node, 0x0, sizeof(P_NODE));
    p_node.prog_id = 0;
    system_state = api_get_system_state();
    max_channel = db_check_prog_exist(VIEW_ALL | av_flag, 0);//speed Ui when db full

#ifdef RECORD_SUPPORT
    if(api_pvr_get_rating_lock_eit_flag() == TRUE)
    {
        return;
    }
#endif
    if(0 == max_channel)
    {
        pre_chanlock=sys_data_get_channel_lock();
        return;
    }
    if((INVALID_POS_NUM == channel)||(SUCCESS != get_prog_at((UINT16)channel, &p_node)))
    {
        return;
    }

    cur_prog_id = p_node.prog_id;
#ifdef RATING_LOCK_WITH_PASSWORD
    if(pre_prog_id!=cur_prog_id)
    {
        pre_ratinglock=0;
        set_passwd_status(FALSE);
    }
#endif

    // Shall not reset password status if user had input correct password already.
    // Otherwise it will cause user have to input password many times while the rating info is changed.
    if(get_passwd_status() || get_channel_parrent_lock()||rating_lock_change_channel_flag)
    {
        set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
        pre_ratinglock = 0;
        rating_lock_change_channel_flag=FALSE;

        return;//CC_STATE_STOP
    }

    if(RET_SUCCESS == ts_route_get_by_type(TS_ROUTE_MAIN_PLAY,&ts_route_id,&ts_route))
    {
        if(ts_route.prog_id == cur_prog_id)
        {
            cur_sim_id = ts_route.dmx_sim_info[ts_route.dmx_id].sim_id;
            //chchg_param.es.sim_id = ts_route.dmx_sim_info[ts_route.dmx_id].sim_id;            
        }
    }

    if ((pre_ratinglock != lock))
    {
        MEMSET(&frontend, 0x0, sizeof(struct ft_frontend));
        MEMSET(&chchg_param, 0x0, sizeof(struct cc_param));        
        // Shall not reset password status if user had input correct password already.
        // Otherwise it will cause user have to input password many times while the rating info is changed.
        chan_id = p_node.prog_id;
        pre_ratinglock = lock;
        pre_prog_id = cur_prog_id;
        if (lock)
        {
#ifndef _BUILD_OTA_E_
            api_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, !b_force);
            chchg_param.es.sim_id = cur_sim_id;
            chchg_stop_channel(&chchg_param.es, &chchg_param.dev_list, screen_back_state != SCREEN_BACK_RADIO);
            //delay wait stop(epg_off) finish, then call epg_on
            osal_task_sleep(2000);
            //epg_on(chchg_param.es.sat_id,chchg_param.es.tp_id,chchg_param.es.service_id);
#endif
            api_lock_channel(chan_id);
#ifdef MULTIVIEW_SUPPORT
            if(system_state ==SYS_STATE_9PIC)
            {
                win_multiview_set_age_lock(TRUE);
            }
            else if(system_state ==SYS_STATE_4_1PIC)
            {
                win_multiview_ext_set_age_lock(TRUE);
            }
#endif
            if(!(((SYS_STATE_9PIC == system_state)||(SYS_STATE_4_1PIC == system_state))))
            {
 #ifdef RATING_LOCK_WITH_PASSWORD

            if( win_msg_popup_opend() )
            {   //Bug fixed BUG44322:
                //It's no action when the parental lock status is changed during a dialog is showing.
                set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
            }
            else
            {
                win_set_pwd_caller( WIN_PWD_CALLED_PROG_PLAYING );
                if( !win_pwd_open(NULL,0) )
                {
                    set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
                }
                else
                {
#ifdef DVR_PVR_SUPPORT
                    if(!api_pvr_is_playing())
#endif
                    {
#ifndef _BUILD_OTA_E_
                        api_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, !b_force);
                        chchg_param.es.sim_id = cur_sim_id;
                        chchg_play_channel(&frontend, &chchg_param, screen_back_state != SCREEN_BACK_RADIO);
#endif
                    }

                    #if (SUBTITLE_ON == 1)
                    api_subt_show_onoff(TRUE);//RESUME
                    #endif
                    set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
#ifdef DVR_PVR_SUPPORT
                    *need_check_tms = TRUE;//add for when change to parental lock's tp,cann't timeshift
#endif
                }
                win_set_pwd_caller( WIN_PWD_CALLED_UNKOWN );
            }
#else
            set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
#endif
        }
        else
            {
                 set_rating_lock(SIGNAL_STATUS_RATING_LOCK);
            }

        }
        else
        {
            if(!(((SYS_STATE_9PIC == system_state)||(SYS_STATE_4_1PIC == system_state))))
            {
#ifdef DVR_PVR_SUPPORT
                if(!api_pvr_is_playing())
#endif
                {
#ifndef _BUILD_OTA_E_
                    
                    api_pre_play_channel(&p_node, &frontend, &chchg_param, &ci_start_service, !b_force); 
                    chchg_param.es.sim_id = cur_sim_id;
                    chchg_play_channel(&frontend, &chchg_param, screen_back_state != SCREEN_BACK_RADIO);
#endif
                }
            #if (SUBTITLE_ON == 1)
                api_subt_show_onoff(TRUE);//RESUME
            #endif
            }
            set_rating_lock(SIGNAL_STATUS_RATING_UNLOCK);
#ifdef MULTIVIEW_SUPPORT
            if(system_state ==SYS_STATE_9PIC)
            {
                win_multiview_set_age_lock(FALSE);
            }
            else if(system_state ==SYS_STATE_4_1PIC)
            {
                win_multiview_ext_set_age_lock(FALSE);
            }
#endif
        }
    }
    if (lock && (STATUS_OFF == epg_get_status()))
    {
        //epg_on(p_node.sat_id, p_node.tp_id, p_node.prog_number);

        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ap_get_main_dmx());
        if(dmx)
        {
    #ifdef _INVW_JUICE
            epg_on_by_inview(dmx, p_node.sat_id, p_node.tp_id, p_node.prog_number, 0);
    #else
            epg_on_ext(dmx,p_node.sat_id, p_node.tp_id, p_node.prog_number);
    #endif // _INVW_JUICE
        }
    }
}

#endif //PARENTAL_SUPPORT
#endif //_BUILD_OTA_E_

