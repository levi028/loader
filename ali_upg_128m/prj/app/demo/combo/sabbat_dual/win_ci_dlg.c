/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_dlg.c
*
*    Description: The realize the function of CI dlg
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_ci_dlg.h"

#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#include "win_ci_common.h"

UINT8 b_dlg_show = 0;

#define CI_DLG_W    500
#define CI_DLG_H    150
#define CI_DLG_L    GET_MID_L(CI_DLG_W)
#define CI_DLG_T    GET_MID_T(CI_DLG_H)

//Clear the VSCreen
static BOOL osd_clear_vscreen(void)
{
    OSD_RECT rect;
    UINT32 vscr_idx;
    VSCR *ap_vscr;

    vscr_idx = osal_task_get_current_id();
    ap_vscr = osd_get_task_vscr(vscr_idx);
    if(NULL == ap_vscr)
        return FALSE;

    if (ap_vscr)
    {
        ap_vscr->update_pending = 0;
        ap_vscr->lpb_scr = NULL;
    }

    osd_get_rect_on_screen(&rect);
    rect.u_left = rect.u_top= 0;
    UINT32 trans_color = osd_get_trans_color(ap_vscr->b_color_mode, FALSE);
    osd_draw_frame(&rect,trans_color,ap_vscr);

    return TRUE;
}

BOOL is_ci_dlg_openning(void)
{
    return b_dlg_show;
}
#ifdef AUTO_UPDATE_TPINFO_SUPPORT
void set_ci_dlg_openning( BOOL show)
{
    b_dlg_show = show;
}
#endif
static void win_ci_dlg_open(UINT16 *uni_str_status)
{
    UINT8 back_saved;
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(CI_DLG_L, CI_DLG_T, CI_DLG_W, CI_DLG_H);
    win_compopup_set_msg_ext(NULL, (char*)uni_str_status, 0);
    win_msg_popup_open();
    b_dlg_show = 1;
}

void win_ci_dlg_close(void)
{
#ifdef TRUE_COLOR_HD_OSD
    OBJECT_HEAD*topmenu = NULL;
    win_msg_popup_close();

    topmenu = menu_stack_get_top();
    if(topmenu && topmenu != (POBJECT_HEAD)&g_win_mainmenu)
    {
        osd_clear_vscreen();
        osd_draw_object(topmenu, C_UPDATE_ALL);
    }

    b_dlg_show = 0;
#else
    win_msg_popup_close();
    b_dlg_show = 0;
#endif

}

void win_ci_dlg_show_status(UINT32 msg_code)
{
    UINT16 slot      = msg_code >> 16;
    UINT16 ci_msg = msg_code & 0xFFFF;

    char ansi_str[MAX_DISP_STR_LEN];
    char *str;
    void *ci_buffer = (void*)win_ci_get_tmp_buf();
    UINT16 uni_str_status[MAX_DISP_STR_LEN];
    struct ci_enquiry *enquiry;
    int t_len = 0;

#ifdef CI_PLUS_SUPPORT
    if(is_ciplus_menu_exist())
    {
        return;
    }
#endif
    snprintf(ansi_str, MAX_DISP_STR_LEN, "Slot %d: ", slot+1);
    switch(ci_msg)
    {
        case API_MSG_CAMIN:
            t_len = strlen(ansi_str);
            if ((MAX_DISP_STR_LEN - t_len) > strlen("Initializing"))
            {
                strncat(ansi_str, "Initializing", MAX_DISP_STR_LEN-t_len-1);
            }
            com_asc_str2uni((UINT8*)ansi_str, uni_str_status);

            if(1 == b_dlg_show)
            {
                win_ci_dlg_close();
            }
            win_ci_dlg_open(uni_str_status);
            //cidlg_wait_msg();
            //win_ci_dlg_close();
            break;

        case API_MSG_CAMOUT:
            t_len = strlen(ansi_str);
            if ((MAX_DISP_STR_LEN - t_len) > strlen("Initializing"))
            {
                strncat(ansi_str, "Uninstalling", MAX_DISP_STR_LEN-t_len-1);
            }
            com_asc_str2uni((UINT8*)ansi_str, uni_str_status);

            if(1 == b_dlg_show)
            {
                win_ci_dlg_close();
            }
            win_ci_dlg_open(uni_str_status);
            osal_task_sleep(200);
            //cidlg_wait_msg();
            //win_ci_dlg_close();
            break;
        case API_MSG_CTC:
        case API_MSG_DTC:
            if(1 == b_dlg_show)
            {
                win_ci_dlg_close();
            }
            break;
        case API_MSG_ENQUIRY_UPDATE:
            enquiry = api_ci_get_enquiry(ci_buffer, CI_DATA_BUFFER_LEN, slot);

            if(enquiry != NULL)
            {
                str = ci_enquiry_get_text(enquiry);
                com_asc_str2uni(str, uni_str_status);
            }

            if(1 == b_dlg_show)
            {
                win_ci_dlg_close();
            }
            win_ci_dlg_open(uni_str_status);
            //cidlg_wait_msg();
            //win_ci_dlg_close();
            break;
        default:
            break;
    }


}


UINT8 win_ci_dlg_get_status(void)
{
    return b_dlg_show;

}
#else // CI_SUPPORT
UINT8 win_ci_dlg_get_status(void)
{
    return 0;
}
#endif


