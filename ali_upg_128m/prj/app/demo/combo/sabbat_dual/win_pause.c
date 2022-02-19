/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pause.c
*
*    Description: pause nemu(press botton "PAUSE").
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
#include <hld/dis/vpo.h>

#include "copper_common/com_api.h"
#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "win_pause.h"
#include "win_mute.h"

/*******************************************************************************
* Objects definition
*******************************************************************************/
extern BITMAP pause_icon;
extern CONTAINER g_win2_pause;
CONTAINER g_win2_pause = {
	.head = {
		.b_type = OT_CONTAINER,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {PAUSE_L, PAUSE_T, PAUSE_W, PAUSE_H},
		.style = {
			.b_show_idx = WSTL_N_BULU2_BG_CIRCL, .b_hlidx = WSTL_N_BULU2_BG_CIRCL, .b_sel_idx = WSTL_N_BULU2_BG_CIRCL, .b_gray_idx = WSTL_N_BULU2_BG_CIRCL
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)NULL
	},
	.p_next_in_cntn = (POBJECT_HEAD)&pause_icon,
	.focus_object_id = 1,
	.b_hilite_as_whole = 0,
};
//pause_icon ------------------------------------------------------------------------
BITMAP pause_icon = {
	.head = {
		.b_type = OT_BITMAP,
		.b_attr = C_ATTR_ACTIVE, .b_font = 0,
		.b_id = 0, .b_left_id = 0, .b_right_id = 0, .b_up_id = 0, .b_down_id = 0,
		.frame = {PAUSE_L, PAUSE_T, PAUSE_W, PAUSE_H},
		.style = {
			.b_show_idx = 0, .b_hlidx = 0, .b_sel_idx = 0, .b_gray_idx = 0
		},
		.pfn_key_map = NULL, .pfn_callback = NULL,
		.p_next = (POBJECT_HEAD)NULL, .p_root = (POBJECT_HEAD)&g_win2_pause
	},
	.b_align = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.b_x = 0, .b_y = 0,
	.w_icon_id = PAUSE_ICON,
};
static BOOL  cur_pause_state = UNPAUSE_STATE;
static UINT8 m_pause_on_screen = 0;
/*******************************************************************************
* Function definition
*******************************************************************************/
BOOL get_pause_state(void)
{
    return cur_pause_state;
}

void show_pause_osdon_off(UINT8 flag)
{
    OBJECT_HEAD *obj = NULL;
#ifdef REMOVE_LIVE_PAUSE_FUNCTION    
    return;
#endif

#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    return;
#endif

    obj= (OBJECT_HEAD*)&g_win2_pause;

    if (flag != m_pause_on_screen)
    {
        if(flag)
        {
            api_inc_wnd_count();
        }
    }

    if(flag)
    {
        osd_draw_object(obj, C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object(obj, 0);
    }
    if (flag != m_pause_on_screen)
    {
        if (!flag)
        {
            api_dec_wnd_count();
        }
        m_pause_on_screen = flag;
    }
}


void show_pause_on_off(void)
{
#ifdef REMOVE_LIVE_PAUSE_FUNCTION    
    return;
#endif
    
#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    return;
#endif
    show_pause_osdon_off(PAUSE_STATE == cur_pause_state);
}

static void set_vpo_playmode(UINT32 param)
{
    struct vpo_device *vpo_old = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);

#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *vpo_new = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
//#else
//    struct vpo_device *vpo_new = NULL;
#endif

    vpo_ioctl(vpo_old,VPO_IO_PLAYMODE_CHANGE,param);
#ifdef DUAL_VIDEO_OUTPUT
    if(vpo_new != NULL)
    {
        vpo_ioctl(vpo_new,VPO_IO_PLAYMODE_CHANGE,param);
    }
#endif
}

void set_pause_on_off(BOOL show_osd)
{
#ifdef REMOVE_LIVE_PAUSE_FUNCTION    
    return;
#endif
    
#if defined(SUPPORT_CAS9) || defined(_GEN_CA_ENABLE_)
    return;
#endif


#ifndef NEW_DEMO_FRAME
    if(UNPAUSE_STATE == cur_pause_state)
    {
        cur_pause_state = PAUSE_STATE;
        set_vpo_playmode(NORMAL_2_ABNOR);
        uich_chg_pause_prog();
    }
    else
    {
        cur_pause_state = UNPAUSE_STATE;
        set_vpo_playmode(ABNOR_2_NORMAL);
        uich_chg_resume_prog();
    }
#else
    if(UNPAUSE_STATE == cur_pause_state)
    {
#ifdef _INVW_JUICE
        if(show_osd)
        {
            mute_switch_pause_process(TRUE);
        }
#endif
        cur_pause_state = PAUSE_STATE;
        set_vpo_playmode(NORMAL_2_ABNOR);
        api_video_set_pause(1);
    }
    else
    {
            cur_pause_state = UNPAUSE_STATE;
        set_vpo_playmode(ABNOR_2_NORMAL);
        api_video_set_pause(0);
#ifdef _INVW_JUICE
        if(show_osd)
        {
            mute_switch_pause_process(FALSE);
        }
#endif
    }
#endif

    if(show_osd)
    {
        show_pause_on_off();
    }
}

void get_pause_rect(OSD_RECT *rect)
{
    POBJECT_HEAD obj = NULL;

    if(NULL == rect)
    {
        return ;
    }
    obj = (POBJECT_HEAD)&g_win2_pause;
    *rect = obj->frame;
}

#ifdef _INVW_JUICE
BOOL pause_state_on()
{
    return PAUSE_STATE == cur_pause_state;
}
#endif

