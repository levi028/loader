/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mheg5_v20.c
*
*    Description: application for MHEG5.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

//#include "ap_com.h"
#include <api/libpub27/lib_hde.h>
#include <api/libosd/osd_lib.h>

#include "vkey.h"
#include "control.h"
#include "menus_root.h"

#if (defined(_MHEG5_V20_ENABLE_) )

#include <mh5_api/mh5_api.h>
#include "control.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "string.id"

#define KEY_PRINTF(...)
//#define KEY_PRINTF libc_printf

#define MHEG_KEY_NUM 6
#define MHEG_MULTIFUNC_KEY_NUM 16
#define INVALID_KEY_NUM 0xFF

#define         MHEG_WAKEUP_TIME        5000
#define     MHEG_WAKEUP_TIMER_NAME  "mheg5"

#ifdef MHEG5_SUPPORT_EPG
    #define GROUP_ALL_SEARCH_START_INDEX            0
    #define GROUP_ALL_SEARCH_END_INDEX              21
    #define GROUP_NORMAL_SEARCH_START_INDEX         15
    #define GROUP_NORMAL_SEARCH_END_INDEX           21
    #define GROUP_NO_DIGITS_SEARCH_START_INDEX      10
    #define GROUP_NO_DIGITS_SEARCH_END_INDEX        21
#else
    #define GROUP_ALL_SEARCH_START_INDEX            0
    #define GROUP_ALL_SEARCH_END_INDEX              20
    #define GROUP_NORMAL_SEARCH_START_INDEX         15
    #define GROUP_NORMAL_SEARCH_END_INDEX           20
    #define GROUP_NO_DIGITS_SEARCH_START_INDEX      10
    #define GROUP_NO_DIGITS_SEARCH_END_INDEX        20
#endif

/*Receiver group*/

/*Register 3, 4, 5 group*/
static const UINT8 key_to_mhegkey[][2] =
{
/*Register 4 group , start = 0, end = 9*/
    {V_KEY_0,       MHEG5_KEYPRESS_0},
    {V_KEY_1,       MHEG5_KEYPRESS_1},
    {V_KEY_2,       MHEG5_KEYPRESS_2},
    {V_KEY_3,       MHEG5_KEYPRESS_3},
    {V_KEY_4,       MHEG5_KEYPRESS_4},
    {V_KEY_5,       MHEG5_KEYPRESS_5},
    {V_KEY_6,       MHEG5_KEYPRESS_6},
    {V_KEY_7,       MHEG5_KEYPRESS_7},
    {V_KEY_8,       MHEG5_KEYPRESS_8},
    {V_KEY_9,       MHEG5_KEYPRESS_9},

/*Register 5 group, start = 10, end = 14*/
    {V_KEY_UP,      MHEG5_KEYPRESS_UP},
    {V_KEY_DOWN,    MHEG5_KEYPRESS_DOWN},
    {V_KEY_LEFT,        MHEG5_KEYPRESS_LEFT},
    {V_KEY_RIGHT,   MHEG5_KEYPRESS_RIGHT},
    //{V_KEY_OK,      MHEG5_KEYPRESS_SELECT},
    {V_KEY_ENTER,   MHEG5_KEYPRESS_SELECT},

/*Register 3 group, start = 15, end = 21*/
    {V_KEY_EXIT,        MHEG5_KEYPRESS_CANCEL},
    {V_KEY_RED,  MHEG5_KEYPRESS_RED},
    {V_KEY_GREEN,  MHEG5_KEYPRESS_GREEN},
    {V_KEY_YELLOW,  MHEG5_KEYPRESS_YELLOW},
    {V_KEY_BLUE,  MHEG5_KEYPRESS_BLUE},
#if 0
    {V_KEY_COLOR0,  MHEG5_KEYPRESS_RED},
    {V_KEY_COLOR1,  MHEG5_KEYPRESS_GREEN},
    {V_KEY_COLOR2,  MHEG5_KEYPRESS_YELLOW},
    {V_KEY_COLOR3,  MHEG5_KEYPRESS_BLUE},
#endif
    {V_KEY_TEXT,    MHEG5_KEYPRESS_TEXT},

#ifdef MHEG5_SUPPORT_EPG
    {V_KEY_EPG,    MHEG5_KEYPRESS_EPG},
#endif

};

static ID       mheg_wakeup_timer = OSAL_INVALID_ID;

static BOOL is_channel_lock(void)
{
    UINT32 ch_index = sys_data_get_cur_group_cur_mode_channel();
    P_NODE p_node;
    BOOL result = FALSE;

    MEMSET(&p_node,0,sizeof(P_NODE));
    if (SUCCESS == get_prog_at(ch_index, &p_node))
    {
        if (get_channel_parrent_lock() && sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))
        {
            result = TRUE;
        }
        else
        {
            result = FALSE;
        }
    }
    else
    {
        result = FALSE;
    }

    return result;
}

static mheg5key_press search_key_table(UINT8 start, UINT8 end, UINT32 key)
{
    int i = 0;
    UINT32 result = INVALID_KEY_NUM ;

    for(i = start ; i <= end ; i++)
    {
        if(key == key_to_mhegkey[i][0])
        {
            KEY_PRINTF("%s :  key =  %d\n",__FUNCTION__,key_to_mhegkey[i][1]);
            result = key_to_mhegkey[i][1];
            break;
        }
    }
    return result;
}

static BOOL maping_key_usage(UINT32 param1)
{
    int i  = 0;
    BOOL ret = FALSE;
    UINT32 key = 0;
    UINT32 start = 0, end = 0;
    mheg5register_group group_type;
    mheg5progress_message progress_message;

    mheg5get_key_function_group(&group_type);

    switch(group_type)
    {
/*v1.06 start*/
    case MHEG5_FUNCTION_GROUP_ALL:
        start = GROUP_ALL_SEARCH_START_INDEX;
        end = GROUP_ALL_SEARCH_END_INDEX;
        break;
    case MHEG5_FUNCTION_GROUP_NORMAL:
        start = GROUP_NORMAL_SEARCH_START_INDEX;
        end = GROUP_NORMAL_SEARCH_END_INDEX;
        break;
    case MHEG5_FUNCTION_GROUP_NO_DIGITS:
        start = GROUP_NO_DIGITS_SEARCH_START_INDEX;
        end = GROUP_NO_DIGITS_SEARCH_END_INDEX;
        break;
    default:
        KEY_PRINTF("%s : Group setting error\n");
        //ASSERT(0);
        goto _END_SEARCH; /*MHEG5_FUNCTION_GROUP_NONE*/
        break;
    }
/*v1.06 end*/
    if((key = search_key_table(start, end, param1)) != INVALID_KEY_NUM)
    {
        if((MHEG5_KEYPRESS_RED == key) || (MHEG5_KEYPRESS_TEXT == key))
        {
            P_NODE p_node;

#ifdef CI_PLUS_SUPPORT
            if(!is_ciplus_menu_exist())
#endif
                return FALSE;

            INT32 er = get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node);
            mheg5api_get_component_progress(&progress_message);
            if((SUCCESS == er) && !p_node.mheg5_exist &&  ( MHEG5_DSM_APPLICATION_STARTED != progress_message))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(200, 200, 200, 100);
                //win_compopup_set_msg(NULL, NULL,RS_POPUP_NO_MHEG5);
                win_compopup_set_msg(NULL, NULL,2);
                win_compopup_open();
                osal_task_sleep(1000);
                win_compopup_close();

                return TRUE;
            }
        }
        mheg5api_keypress(key);
        ret = TRUE;
    }

_END_SEARCH:

    return ret;
}

static BOOL mheg_key_detect(UINT32 key_code)
{
    BOOL ret = FALSE;
    UINT32 vkey = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if(get_prog_num(VIEW_ALL | sys_data_get_cur_chan_mode(), 0)
#ifdef CI_PLUS_SUPPORT
    || is_ciplus_menu_exist()
#endif
    )
    {
        /* There is handle at desktop, system looks at this key first*/
        UINT32 code = ap_hk_to_vk(0,key_code, &vkey);
        if(vkey != V_KEY_NULL)
        {
            key_code = vkey;
        }
        ret = maping_key_usage(vkey);
    }
    if(!ret && (0 == get_prog_num(VIEW_ALL | sys_data_get_cur_chan_mode(), 0)))
    {
#ifdef PVR3_SUPPORT
        if( pvr_info->play.play_handle != 0)
        {
            ret = FALSE;
        }
        else
#endif
        {
            if(vkey != V_KEY_MENU)
            {
                ret = TRUE;
            }
        }
    }

    return ret ;
}

UINT8 ali_mheg_hook_proc(uimenu_state_t state, POBJECT_HEAD p_menu,  control_msg_t *p_msg)
{
    UINT16 u_current_channel = 0xffff;

#if (defined (CI_PLUS_SUPPORT))
    if(!is_ciplus_menu_exist())
    {
         return PROC_PASS;
    }
#endif
    //return PROC_PASS;
    if(UI_MENU_OPEN == state)
    {
        if( (p_menu == (POBJECT_HEAD)&g_win_mainmenu) || \
            (p_menu == (POBJECT_HEAD)&g_win_usb_filelist) || \
            (p_menu == (POBJECT_HEAD)&g_win_epg)
#if ( defined(DVR_PVR_INTERFACE_SUPPORT))
            || (p_menu == (POBJECT_HEAD)&g_win_pvr_ctrl)
#endif
            )
        {
            KEY_PRINTF("%s :  Stop Application\n",__FUNCTION__);
            mheg5_clr_spin(SPIN_MHEG_MENU);
#if (defined (CI_PLUS_SUPPORT))
            if(!is_ciplus_menu_exist())
#endif
            {
                mheg5_receiver_stop_applications();
                mheg5_osdexit();
                set_menu_exit_from_ci_browser(FALSE);
            }
#if (defined (CI_PLUS_SUPPORT))
            else
            {
                if(p_menu == (POBJECT_HEAD)&g_win_mainmenu)
                {
                    set_menu_exit_from_ci_browser(TRUE);
                }
                osddrv_show_on_off((HANDLE)dev_get_by_id(HLD_DEV_TYPE_OSD, 1), OSDDRV_OFF);
                //ciplus_browser_exit();
            }
#endif
        }
#if 0   //mainmenu have showed logo.
        if(sys_data_get_cur_chan_mode() == RADIO_CHAN)
        {
            KEY_PRINTF("%s :  Show Radio Logo\n",__FUNCTION__);
            api_show_radio_logo();
        }
#endif
    }
    else if((UI_MENU_CLOSE == state) && (FALSE == get_signal_stataus_show()))
    {
        mheg5_set_spin(SPIN_MHEG_MENU);
        if((FALSE == is_channel_lock()) && (FALSE == api_get_subt_onoff()))
        {
            KEY_PRINTF("%s :  Start Application\n",__FUNCTION__);
            //This operation will effect CI for PMT monitor
                /*if((u_current_channel = si_get_cur_channel()) != 0xffff)
                {
                    si_monitor_off(u_current_channel);
                }
                si_monitor_on(u_current_channel);*/
#if (defined (CI_PLUS_SUPPORT))
            if(!is_ciplus_menu_exist())
#endif
            {
                mheg5_osdenter();
                mheg5_receiver_start_applications(si_get_cur_channel());
            }
#if (defined (CI_PLUS_SUPPORT))
            else
            {
                mheg5_osdenter();
            }
#endif 
        }
    }
    else
    {
        /*When no handle , mheg can look at this key first*/
        if(!p_menu && p_msg && (CTRL_MSG_SUBTYPE_KEY == p_msg->msg_type))
        {
            if(mheg_key_detect(p_msg->msg_code))
            {
                return PROC_LOOP;
            }
        }
    }

    return PROC_PASS;
}

BOOL g_mheg_force_close_osd = FALSE;

void mheg_force_set_osd_onoff(BOOL onoff)
{
    g_mheg_force_close_osd = onoff;
}

BOOL mheg_force_get_osd_onoff(void)
{
    return g_mheg_force_close_osd;
}


void mheg_wakeup_timer_handler(UINT param)
{
    api_stop_timer(&mheg_wakeup_timer);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WAKEUP_MHEG5, 1, TRUE);
}

void mheg_wakeup_timer_stop(void)
{
    if(mheg_wakeup_timer != OSAL_INVALID_ID)
    {
        api_stop_timer(&mheg_wakeup_timer);
    }
}

void mheg_wakeup_timer_start(void)
{
    mheg_wakeup_timer_stop();
    mheg_wakeup_timer=api_start_timer(MHEG_WAKEUP_TIMER_NAME,MHEG_WAKEUP_TIME, mheg_wakeup_timer_handler);
}

#endif

