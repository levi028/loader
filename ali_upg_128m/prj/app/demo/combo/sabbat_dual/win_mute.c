/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mute.c
*
*    Description: mute menu(press botton "MUTE").
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "win_com.h"
#include "win_mute.h"

#ifdef CAS9_V6
#include "conax_ap/win_ca_uri.h"
#endif

/*******************************************************************************
* WINDOW's objects declaration
*******************************************************************************/

/*******************************************************************************
* Objects definition
*******************************************************************************/
extern BITMAP mute_icon;
extern CONTAINER g_win2_mute;
CONTAINER g_win2_mute = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {MUTE_L, MUTE_T, MUTE_W, MUTE_H},
		.style = {
			.b_show_idx = WSTL_N_BULU2_BG_CIRCL, .b_hlidx = WSTL_N_BULU2_BG_CIRCL, .b_sel_idx = WSTL_N_BULU2_BG_CIRCL, .b_gray_idx = WSTL_N_BULU2_BG_CIRCL
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)NULL
	},
	.p_next_in_cntn = (POBJECT_HEAD)&mute_icon,
	.focus_object_id = 1,
	.b_hilite_as_whole = 0,
};
//mute_icon ------------------------------------------------------------------------
BITMAP mute_icon = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {MUTE_L, MUTE_T, MUTE_W, MUTE_H},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&g_win2_mute
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = MUTE_ICON,
};
/*******************************************************************************
* Function definition
*******************************************************************************/

/***********************************************/
BOOL  cur_mute_state=UNMUTE_STATE;
static BOOL  pre_mute_state = UNMUTE_STATE;
static UINT8 m_mute_on_screen  = 0;

BOOL get_mute_state(void)
{
    return cur_mute_state;
}

void set_mute_state(BOOL b_mute)
{
    cur_mute_state = b_mute;
}

void save_mute_state(void)
{
    pre_mute_state = cur_mute_state;
}

BOOL mute_state_is_change(void)
{
    return pre_mute_state != cur_mute_state;
}

void show_mute_osdon_off(UINT8 flag)
{
    OBJECT_HEAD *obj = NULL;

    obj = (OBJECT_HEAD*)&g_win2_mute;

    if (flag != m_mute_on_screen)
    {
        if(flag)
        {
            api_inc_wnd_count();
        }
    }

#ifdef _INVW_JUICE //v0.1.4
    if (inview_is_started())
    {
        if (!inview_is_app_busy())
        {
            inview_pause();
        }
    }
#endif
    if(flag)
    {
        osd_draw_object(obj, C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object(obj, 0);
    }
    if (flag != m_mute_on_screen)
    {
        if(!flag)
        {
            api_dec_wnd_count();
        }
        m_mute_on_screen = flag;
    }

}

void show_mute_on_off(void)
{
    if((get_mute_state()) && (mute_state_is_change()) && (TRUE == get_channel_parrent_lock()))
    {
        return;
    }
     show_mute_osdon_off(MUTE_STATE == cur_mute_state);
}

void set_mute_on_off(BOOL show_osd)
{
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();

    if(UNMUTE_STATE == cur_mute_state)
    {
        cur_mute_state = MUTE_STATE;
    }
    else
    {
        cur_mute_state = UNMUTE_STATE;
    }
#ifdef AUDIO_MUTE_SAVE
    sys_data->b_mute_sate = cur_mute_state;
#endif

#ifdef SYS_VOLUME_FOR_PER_PROGRAM
	if(UNMUTE_STATE == cur_mute_state)
	{
		UINT8 volume;
		if(get_channel_vol(&volume))
		{
			api_audio_set_volume(volume);
		}
	}
#endif
#ifndef _BUILD_OTA_E_
#ifdef CAS9_V6
    if(ALI_S3281==sys_ic_get_chip_id())
    {
        if(UNMUTE_STATE==cur_mute_state)
        {
            if(TRUE!=api_cnx_uri_get_da_mute())
            {
                api_audio_set_mute(cur_mute_state);
            }
        }
        else
        {
            api_audio_set_mute(cur_mute_state);
        }
    }
    else
#endif
#endif
    {
        api_audio_set_mute(cur_mute_state);
    }


    if(show_osd)
    {
        show_mute_on_off();
    }
}

void get_mute_rect(OSD_RECT *rect)
{
    POBJECT_HEAD obj = NULL;

    if(NULL == rect)
    {
        return ;
    }
    obj = (POBJECT_HEAD)&g_win2_mute;
    *rect = obj->frame;
}
#ifdef _INVW_JUICE
void mute_switch_pause_process(BOOL en)
{

    if(!en || (UNMUTE_STATE == cur_mute_state))
    {
        set_mute_on_off(FALSE);
    }
    else if(MUTE_STATE == cur_mute_state)
    {
        cur_mute_state=UNMUTE_STATE;
        show_mute_on_off();
        cur_mute_state=MUTE_STATE;
    }
}
#endif

