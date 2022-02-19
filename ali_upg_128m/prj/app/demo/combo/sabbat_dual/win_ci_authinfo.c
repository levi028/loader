/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_authinfo.c
*
*    Description: To realize the UI for display CI auth-information
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
#include <api/libfs2/fcntl.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <hld/hld_dev.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "vkey.h"

#include "win_com_menu_define.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"


#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>

#ifdef CI_PLUS_SUPPORT

#include <api/libci/ci_cc_api.h>
#include "win_ci_authinfo.h"

/*******************************************************************************
* Objects declaration
*******************************************************************************/
#ifndef SD_UI
#define    CW_W     800    //440
#define    CW_H     460    //300
#define    CW_L     GET_MID_L(CW_W)     //256//90
#define    CW_T     GET_MID_T(CW_H)     //100//20
#else
#define	CW_W     500    //440
#define	CW_H     400    //300
#define	CW_L     GET_MID_L(CW_W)     //256//90
#define	CW_T     GET_MID_T(CW_H)     //100//20
#endif

#define ITEM_H  40

#undef WIN_SH_IDX
#define WIN_SH_IDX    WSTL_DIAG_WIN_8BIT //WSTL_POPUP_WIN_02//WSTL_POP_WIN_01

#undef TXTN_SH_IDX
#define TXTN_SH_IDX WSTL_DIAG_BUTTON_01_8BIT


#define INFO_TXT_NAME_H 5
#define INFO_TXT_NAME_H1 (INFO_TXT_NAME_H + 50)
#define INFO_TXT_NAME_H2 (INFO_TXT_NAME_H1 + 50)
#define INFO_TXT_NAME_H3 (INFO_TXT_NAME_H2 + 50)
#define INFO_TXT_NAME_H4 (INFO_TXT_NAME_H3 + 50)

static VACTION  ci_authinfo_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT  ci_authinfo_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

UINT16 ci_auth_info_name_str[60];
UINT16 ci_auth_info1_str[60];
UINT16 ci_auth_info2_str[60];
UINT16 ci_auth_info3_str[60];
UINT16 ci_auth_info4_str[60];

static char    test[60];
static UINT8   key_id[5];

const char ci_authinfo_menuname[] = "CI Plus CAM Authentication Infomation";

#define CIPLUSERRCODE    "Auth ERR code:  "
#define CIPLUSERRREASON  "Auth ERR by:    "

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_wnd,NULL,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
        ci_authinfo_keymap,ci_authinfo_callback,  \
        nxt_obj, focus_id,0)

#define LDEF_TXT(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_VCENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LIST_TEXT(root,var_nametxt,var_txt1,var_txt2,var_txt3,var_txt4,l,t,w,h)    \
    LDEF_TXT_TITLE(&root,var_nametxt,&var_txt1 ,1,l , t + INFO_TXT_NAME_H,w,h,0,ci_auth_info_name_str)    \
    LDEF_TXT(&root,var_txt1,&var_txt2 ,2,l , t+INFO_TXT_NAME_H1,w,h,0,ci_auth_info1_str)    \
    LDEF_TXT(&root,var_txt2,&var_txt3 ,3,l , t+INFO_TXT_NAME_H2,w,h,0,ci_auth_info2_str)    \
    LDEF_TXT(&root,var_txt3,&var_txt4 ,4,l, t+INFO_TXT_NAME_H3,w,h,0,ci_auth_info3_str)        \
    LDEF_TXT(&root,var_txt4, NULL ,5,l, t+INFO_TXT_NAME_H4,w,h,0,ci_auth_info4_str)
/*
LDEF_LIST_ITEM(win_info_con,info_item_con,NULL,info_txt_name,info_txt1,info_txt2,\
    CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H)
*/
LDEF_LIST_TEXT(g_win_ci_auth_info,ci_auth_info_name,ci_auth_info1,ci_auth_info2,ci_auth_info3,ci_auth_info4,\
    (CW_L+8),CW_T,CW_W-16,ITEM_H);

LDEF_WIN(g_win_ci_auth_info,&ci_auth_info_name,CW_L,CW_T,CW_W,CW_H,1);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static char *strcat ( char * dst, const char * src)
{
        char * cp = dst;
        while( *cp )
                cp++;                   /* find end of dst */
        while( *cp++ = *src++ ) ;       /* Copy src to end of dst */
        return( dst );                  /* return dst */
}

static char ci_autherr_str[64];

static char *ci_authinfo_err_str(UINT32 err_code)
{
    char str_check[64];

    MEMSET(ci_autherr_str, 0x00, 64);
    MEMSET(str_check, 0x00, 64);
    osd_set_text_field_content(&ci_auth_info4,STRING_ANSI, (UINT32)" ");
    switch (err_code)
    {
        case 0:
            MEMCPY(ci_autherr_str, "No error checked", STRLEN("No error checked"));
            MEMCPY(str_check, "If authentication failed, please check stream or CERT DATA",
                   STRLEN("If authentication failed, please check stream or CERT DATA"));
            break;
        case ERR_INVALID_NONCE:
            MEMCPY(ci_autherr_str, "Invalid NONCE from CAM", STRLEN("Invalid NONCE from CAM"));
            MEMCPY(str_check, "Please check your CAM", STRLEN("Please check your CAM "));
            break;
        case ERR_INVALID_DHPH:
            MEMCPY(ci_autherr_str, "Invalid DHPH of STB", STRLEN("Invalid DHPH of STB"));
            MEMCPY(str_check, "Please check your CERT DATA", STRLEN("Please check your CERT DATA"));
            break;
        case ERR_INVALID_CERT:
            MEMCPY(ci_autherr_str, "Invalid CERT of CAM", STRLEN("Invalid CERT of CAM"));
            MEMCPY(str_check, "Please check your CAM or stream TDT",
                   STRLEN("Please check your CAM or stream TDT"));
            break;
        case ERR_CERT_VERIFY_FAIL:
            MEMCPY(ci_autherr_str, "Invalid CERT of CAM",
                   STRLEN("Invalid CERT of CAM"));
            MEMCPY(str_check, "Please check your CAM or stream TDT",
                   STRLEN("Please check your CAM or stream TDT"));
            break;
        case ERR_CERT_EXPIRED:
            MEMCPY(ci_autherr_str, "CERT is expired",
                   STRLEN("CERT is expired"));
            MEMCPY(str_check, "Please check your CAM or stream TDT",
                   STRLEN("Please check your CAM or stream TDT"));
            break;
        case ERR_SIG_B_VERIFY_FAIL:
            MEMCPY(ci_autherr_str, "Signature B isn't right",
                   STRLEN("Signature B isn't right"));
            MEMCPY(str_check, "Please check your CAM or stream TDT",
                   STRLEN("Please check your CAM or stream TDT"));
            break;
        case ERR_INVALID_DHPM:
            MEMCPY(ci_autherr_str, "Invalid DHPM of CAM",
                   STRLEN("Invalid DHPM of CAM"));
            MEMCPY(str_check, "Please check your CAM or stream TDT",
                   STRLEN("Please check your CAM or stream TDT"));
            break;
        case ERR_INVALID_AKH:
            MEMCPY(ci_autherr_str, "Invalid AKH of STB",
                   STRLEN("Invalid AKH of STB"));
            MEMCPY(str_check, "Please check your CERT DATA",
                   STRLEN("Please check your CERT DATA"));
            break;
        case ERR_INVALID_DHSK:
            MEMCPY(ci_autherr_str, "Invalid DHSK of STB",
                   STRLEN("Invalid DHSK of STB"));
            MEMCPY(str_check, "Please check your CERT DATA",
                   STRLEN("Please check your CERT DATA"));
            break;
        case ERR_CAL_CCK_FAIL:
            MEMCPY(ci_autherr_str, "Can't calculate CCK",
                   STRLEN("Can't calculate CCK"));
            MEMCPY(str_check, "Please check your licence function",
                   STRLEN("Please check your licence function"));
            break;
        case ERR_CICAM_ID_NOT_MATCH:
            MEMCPY(ci_autherr_str, "CAM may be changed",
                   STRLEN("CAM may be changed"));
            MEMCPY(str_check, "Please re-insert your CAM",
                   STRLEN("Please re-insert your CAM"));
            break;
        case ERR_SAC_MSG_COUNTER_INVALID:
        case ERR_SAC_PROTOCOL_INVALID:
        case ERR_SAC_VERIFY_FAIL:
            MEMCPY(ci_autherr_str, "SAC message error",
                   STRLEN("SAC message error"));
            MEMCPY(str_check, "Please check your licence function",
                   STRLEN("Please check your licence function"));
            break;
        case err_ns_module_invalid:
            MEMCPY(ci_autherr_str, "CAM's Ns is invalid",
                   STRLEN("CAM's Ns is invalid"));
            MEMCPY(str_check, "Please check your CAM",
                   STRLEN("Please check your CAM"));
            break;
    }
    osd_set_text_field_content(&ci_auth_info4,STRING_ANSI, (UINT32)str_check);
    return ci_autherr_str;
}

static void ci_authinfo_display_set(void)
{

    int     position;
    UINT8   i;
    UINT32  err_code;
    char *err_str = NULL;
    char stream_time[60];
    UINT32 t_len = 0;

    date_time tm_gmt;
    get_utc(&tm_gmt);
    if (is_time_inited())
    {
        snprintf(stream_time, 60,"Stream Time:   %d/%d/%d %d:%d:%d",
                tm_gmt.year, tm_gmt.month, tm_gmt.day,
                tm_gmt.hour, tm_gmt.min, tm_gmt.sec);
    }
    else
    {
        snprintf(stream_time, 60,"Stream Time:   no stream time inited");
    }

    osd_set_text_field_content(&ci_auth_info_name,STRING_ANSI, (UINT32)ci_authinfo_menuname);

    err_code = ci_auth_get_err_code();
    MEMSET(test,0x00,sizeof(test));
    snprintf(test, 60,"%s%d", CIPLUSERRCODE, err_code);
    osd_set_text_field_content(&ci_auth_info1,STRING_ANSI, (UINT32)test);

    MEMSET(test,0x00,sizeof(test));
    strncpy(test,CIPLUSERRREASON, (sizeof(test)-1));
    err_str = ci_authinfo_err_str(err_code);
    t_len = sizeof(test) - strlen(test);
    strncat(test, err_str, t_len-1);
    osd_set_text_field_content(&ci_auth_info2,STRING_ANSI, (UINT32)test);

    osd_set_text_field_content(&ci_auth_info3,STRING_ANSI, (UINT32)stream_time);

    osd_draw_object((POBJECT_HEAD )&g_win_ci_auth_info,C_UPDATE_ALL);

}
static VACTION  ci_authinfo_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
    }

    return act;
}

UINT32 edid_log_file, debug_log_file;

static PRESULT  ci_authinfo_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    POBJECT_HEAD topmenu;

    switch(event)
    {
    case EVN_PRE_OPEN:
        MEMSET(ci_auth_info_name_str,0x00,sizeof(ci_auth_info_name_str));
        MEMSET(ci_auth_info1_str,0x00,sizeof(ci_auth_info1_str));
        MEMSET(ci_auth_info2_str,0x00,sizeof(ci_auth_info2_str));
        MEMSET(ci_auth_info3_str,0x00,sizeof(ci_auth_info3_str));
        MEMSET(ci_auth_info4_str,0x00,sizeof(ci_auth_info4_str));
        wincom_close_title();
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_clear_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
            topmenu = menu_stack_get_top();
            if(topmenu)
                osd_clear_object( topmenu, C_UPDATE_ALL);
        }
        break;
    case EVN_POST_OPEN:
        ci_authinfo_display_set();
        break;
    case EVN_POST_CLOSE:
        wincom_open_title((POBJECT_HEAD)&g_win_ci_slots, RS_CI, 0);
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
            osd_track_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    }

    return ret;
}

#endif

