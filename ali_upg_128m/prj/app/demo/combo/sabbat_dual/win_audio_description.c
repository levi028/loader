/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_audio_description.c
*
*    Description: To realize the function of audio description control
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef AUDIO_DESCRIPTION_SUPPORT

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>

#include <api/libsi/psi_pmt.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_audio_description.h"
#include "win_audio.h"
#include "control.h"


#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
* Objects declaration
*******************************************************************************/
static VACTION ad_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ad_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ad_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ad_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ad_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT ad_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    ad_item_keymap,ad_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    ad_item_sel_keymap,ad_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    ad_keymap,ad_callback,  \
    nxt_obj, focus_id,0)

typedef enum
{
    AD_SERVICE_ID = 1,
    AD_USING_ID,
    AD_VOLUME_OFFSET_ID,
}AD_SETTING_ID;

static UINT16 ad_service_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

static UINT16 ad_use_as_default_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

#define AD_VOLUME_NO_OFFSET_IDX        3    // refer to ad_vol_offset_ids[]

static char *ad_vol_offset_ids[] =
{
    "-3", "-2", "-1", "0", "1", "2", "3"
};

//add on 2011-10-20 for the flag to show ad enable
UINT8 ad_show_flag=0;

#if(SYS_SDRAM_SIZE >= 128)
LDEF_MENU_ITEM(g_win_aud_desc,ad_item0,&ad_item1,ad_txt0,ad_sel0,ad_line0, 1, 3, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_AD_SERVICE, \
        STRING_ID, 0, sizeof(ad_service_ids)/sizeof(UINT16), ad_service_ids)

LDEF_MENU_ITEM(g_win_aud_desc,ad_item1,&ad_item2,ad_txt1,ad_sel1,ad_line1, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_USE_AD_AS_DEFAULT, \
        STRING_ID, 0, sizeof(ad_use_as_default_ids)/sizeof(UINT16), ad_use_as_default_ids)

LDEF_MENU_ITEM(g_win_aud_desc,ad_item2,NULL,ad_txt2,ad_sel2,ad_line2, 3, 2, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_AD_VOLUME_OFFSET, \
        STRING_ANSI, 0, sizeof(ad_vol_offset_ids)/sizeof(char *), ad_vol_offset_ids)

LDEF_WIN(g_win_aud_desc,&ad_item0,W_L, W_T, W_W, W_H, 1)


CONTAINER *ad_menu_items[] =
{
    &ad_item0,
    &ad_item1,
    &ad_item2,
};
#else
LDEF_MENU_ITEM(g_win_aud_desc,ad_item0,NULL,ad_txt0,ad_sel0,ad_line0, 1, 1, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_AD_SERVICE, \
        STRING_ID, 0, sizeof(ad_service_ids)/sizeof(UINT16), ad_service_ids)
LDEF_WIN(g_win_aud_desc,&ad_item0,W_L, W_T, W_W, W_H, 1)

CONTAINER *ad_menu_items[] =
{
    &ad_item0,
};
#endif

static void win_ad_load_setting(void);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION ad_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
          case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT ad_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 sel;
    UINT8 b_id;
    UINT32 draw_cmd = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
        case EVN_PRE_CHANGE:
            sel = *((UINT32*)param1);
            break;
        case EVN_POST_CHANGE:
            sel = param1;
            if (b_id == AD_SERVICE_ID)
            {
                UINT8 action = sel ? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
                sys_data_set_ad_service(sel);
                #if(SYS_SDRAM_SIZE >= 128)
                set_container_active(&ad_item1, action);
                set_container_active(&ad_item2, action);
                osd_draw_object((POBJECT_HEAD)&ad_item1, draw_cmd);
                osd_draw_object((POBJECT_HEAD)&ad_item2, draw_cmd);
                #endif
            }
            else if (b_id == AD_USING_ID)
            {
                sys_data_set_ad_default_mode(sel);
                sys_data_set_ad_mode(sel);
            }
            else if (b_id == AD_VOLUME_OFFSET_ID)
            {
                INT32 offset = (INT32)sel - AD_VOLUME_NO_OFFSET_IDX;
                sys_data_set_ad_volume_offset((signed char)offset);
                api_audio_set_ad_volume_offset(offset);
            }
            break;
        default:
            break;
    }
     return ret;
}

static VACTION ad_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT ad_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION ad_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
          case V_KEY_UP:
            act = VACT_CURSOR_UP;
            break;
        case V_KEY_DOWN:
            act = VACT_CURSOR_DOWN;
            break;
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT ad_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //CONTAINER* root = (CONTAINER*) p_obj;

    switch(event)
    {
        case EVN_PRE_OPEN:
            win_ad_load_setting();
            wincom_open_title(p_obj, RS_AUDIO_DESCRIPTION_SETTING, 0);
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
            sys_data_save(1);
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG; /* Make OSD not flickering */
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_MSG_GOT:
             break;
        default:
            break;
    }

    return ret;
}

static void win_ad_load_setting(void)
{
    MULTISEL *msel;

    msel =&ad_sel0; // AD service
    osd_set_multisel_sel(msel, sys_data_get_ad_service());

    #if(SYS_SDRAM_SIZE >= 128)
    msel =&ad_sel1; // Use AD as default
    osd_set_multisel_sel(msel, sys_data_get_ad_default_mode());

    msel =&ad_sel2; // AD volume offset
    osd_set_multisel_sel(msel, (sys_data_get_ad_volume_offset() + AD_VOLUME_NO_OFFSET_IDX));

    if (sys_data_get_ad_service() == 0)
    {
        set_container_active(&ad_item1, C_ATTR_INACTIVE);
        set_container_active(&ad_item2, C_ATTR_INACTIVE);
    }
    #endif
    /*
    else if (sys_data_get_ad_default_mode() == 0)
    {
        set_container_active(&ad_item2, C_ATTR_INACTIVE);
    }
    */

}

/* return the according AD index for main audio */
int aud_desc_select(UINT16 aud_cnt, UINT16 *aud_pidarray, UINT16 *aud_lang_array,
                    UINT8 *aud_type_array, UINT16 main_aud_idx, BOOL notify)
{
    UINT16 i;

    if ((main_aud_idx >= aud_cnt) || AUDIO_TYPE_IS_AUD_DESC(aud_type_array[main_aud_idx]))
    {
        AD_DEBUG("Invalid main audio (%d)\n", main_aud_idx);
        return -1;
    }

    UINT16 main_aud_pid  = aud_pidarray[main_aud_idx];
    UINT16 main_aud_lang = aud_lang_array[main_aud_idx];

    // select AD stream by main audio
    for (i = 0; i < aud_cnt; ++i)
    {
        if (((aud_lang_array[i] == main_aud_lang)
            ||(aud_type_array[i]== AUDIO_TYPE_AD_RECEIVER_MIXED)||(aud_type_array[i]== AUDIO_TYPE_AD_BROADCAST_MIXED))
            && (aud_pidarray[i] != main_aud_pid))
        {
            int ad_msg_type = -1;
            if (AUDIO_TYPE_IS_BROADCAST_MIXED_AD(aud_type_array[i]))
                ad_msg_type = AD_MSG_TYPE_BOARDCASTER_MIX;
            #if (SYS_SDRAM_SIZE >= 128) //if sdram less than 128M, system only support broastcast mode
            else if (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(aud_type_array[i]))
                ad_msg_type = AD_MSG_TYPE_RECEIVER_MIX;
            #endif

            if (ad_msg_type != -1)
            {
                if (notify)
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUDIO_DESCRIPTION,
                        AD_MSG_PARAM(1, ad_msg_type, main_aud_lang), 0);
                }
                AD_DEBUG("select AD %d for main audio %d\n", i, main_aud_idx);
                return i;
            }
        }
    }
    if (notify)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_AUDIO_DESCRIPTION,
            AD_MSG_PARAM(0, 0, main_aud_lang), 0);
    }
    AD_DEBUG("select none AD for main audio %d\n", main_aud_idx);
    return -1;
}

void aud_desc_show_hint(UINT32 ad_msg)
{
    char str[32] = {0};
    char lang[5] = {0};
    UINT8 back_saved = 0;
    int ad_status = AD_MSG_STATUS(ad_msg);
    int ad_type = AD_MSG_TYPE(ad_msg);
    UINT16 ad_lang = AD_MSG_LANG(ad_msg);
    #ifdef MULTIVIEW_SUPPORT
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    system_state = api_get_system_state();
    #endif
    MEMSET(lang, 0, sizeof(lang));
    get_audio_lang3b((UINT8 *)&ad_lang,(UINT8 *)lang);

    if (ad_status == 0) // AD invalid
        snprintf(str, 32, "%s AD not available", lang);
    else
    {
        if (ad_type == AD_MSG_TYPE_BOARDCASTER_MIX)
            snprintf(str, 32, "AD Enable");
        else if (ad_type == AD_MSG_TYPE_RECEIVER_MIX)
            snprintf(str, 32, "%s AD Enable", lang);
    }
    #ifdef MULTIVIEW_SUPPORT
    if(!((system_state ==SYS_STATE_9PIC)||(system_state ==SYS_STATE_4_1PIC)))
    #endif
    {
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext(str, NULL, 0);

    win_compopup_open_ext(&back_saved);
    osal_task_sleep(1000);
    win_compopup_smsg_restoreback();
    }
}

void win_popup_ad_setting(void)
{
    UINT8 back_saved;

    if (sys_data_get_ad_service() == 0)
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Please Open Audio Description Component", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        return;
    }

    //add on 2011-10-20 for set the flay to show ad enable when press PIP key
  ad_show_flag = 1;

    if (sys_data_get_ad_mode() == 0)
    {
        sys_data_set_ad_mode(1);
        audio_change_pid(-1, FALSE);
    }
    else
    {
        sys_data_set_ad_mode(0);
        audio_change_pid(-1, FALSE);

        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("AD Disable", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
    }
}

#endif /* AUDIO_DESCRIPTION_SUPPORT */

