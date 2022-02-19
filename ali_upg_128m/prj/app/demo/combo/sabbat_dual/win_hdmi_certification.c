 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_hdmi_certification.c
*
*    Description:   The realize for hdmi certification test
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_hdmi_certification.h"
#include "control.h"

#ifdef CEC_SUPPORT
#include "cec_link.h"
#include <api/libcec/lib_cec.h>
#endif

#ifdef CEC_SUPPORT
#ifdef HDMI_CERTIFICATION_UI
extern INT32 cec_link_set_system_audio_mode_by_local_user(BOOL b_on_off);
/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION cert_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cert_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION cert_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cert_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION cert_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cert_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

void win_cec_enter_key(void);
//void win_messagebox_set_message(BOOL update,UINT8 flag);
void check_hdmi_cec_control_state(BOOL enable);

#undef CON_W
#undef TXTN_GRY_IDX
#undef TXTN_L_OF

#define CON_W           (W_W - 36)
#define SEL_L_OF            (TXT_L_OF + TXT_W) - 20
#define SEL_W           390
#define SEL_H           32
#define SEL_T_OF        ((CON_H - SEL_H)/2)

#if 0
#define MTXT_L          (W_L + 30)
//#define MTXT_T        ( CON_T + (CON_H + CON_GAP)*8 + 8)
#define MTXT_T              ( CON_T + (CON_H + CON_GAP)*9)
#define MTXT_W              (W_W - 40)
#define MTXT_H              32
#endif

#define TXTN_L_OF       4
//#define TXTN_W        170
//#define TXTN_H            28
#define TXTN_T_OF       ((CON_H - TXTN_H)/2)

#define TXT_L_OF        TXTN_L_OF
#define TXT_W           TXTN_W
#define TXT_H           TXTN_H
#define TXT_T_OF        TXTN_T_OF

#define SEL_SH_IDX      WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define SEL_HL_IDX      WSTL_BUTTON_04_HD//WSTL_BUTTON_04
#define SEL_SL_IDX      WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define SEL_GRY_IDX     WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define TXT_SH_IDX      WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX      WSTL_BUTTON_04_HD
#define TXT_SL_IDX      WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX     WSTL_BUTTON_07_HD

#define TXTN_SH_IDX     WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define TXTN_HL_IDX     WSTL_BUTTON_02_FG_HD//WSTL_BUTTON_02_FG
#define TXTN_SL_IDX     WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define TXTN_GRY_IDX    WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG

#define MTXT_SH_IDX     WSTL_TEXT_09_HD //WSTL_TEXT_09

static UINT16 onoff_ids[] =
{
    RS_COMMON_ON,
    RS_COMMON_OFF,
};


static INT8 *cec_dest_logical_addr[] =
{
    "TV(0x0)",
    "REC1(0x1)",
    "REC2(0x2)",
    "TUNE1(0x3)",
    "PLAY1(0x4)",
    "AUDIO(0x5)",
    "TUNE2(0x6)",
    "TUNE3(0x7)",
    "PLAY2(0x8)",
    "REC3(0x9)",
    "TUNE4(0xA)",
    "PLAY3(0xB)",
    "RSVD(0xC)",
    "RSVD(0xD)",
    "FREE(0xE)",
    "*ALL*(0xF)",
};



typedef enum
{
    CEC_MSG_SET_OSD_STRING  = 0,
    CEC_MSG_GIVE_OSD_NAME,
    CEC_MSG_SET_OSD_NAME,
    CEC_MSG_REQUEST_ACTIVE_SOUSE,
    CEC_MSG_SYSTEM_STANDBY,
    CEC_MSG_GET_CEC_VERSION,
    CEC_MSG_GIVE_PHYSICAL_ADDRESS,
    CEC_MSG_GET_MENU_LANGUAGE,
    CEC_MSG_SET_MENU_LANGUAGE,
    CEC_MSG_GIVE_TUNER_DEVICE_STATUS,
    CEC_MSG_TUNER_STEP_INCREMENT,
    CEC_MSG_TUNER_STEP_DECREMENT,
    CEC_MSG_GIVE_DEVICE_VENDOR_ID,
    CEC_MSG_GIVE_DEVICE_POWER_STATUS,
    CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_ON,
    CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_OFF,
    CEC_MSG_SET_SYSTEM_AUDIO_MODE_ON,
    CEC_MSG_SET_SYSTEM_AUDIO_MODE_OFF,
    CEC_MSG_ABORT,
    CEC_MSG_POLLING_MESSAGE,
}   E_CEC_MSG_SELECT;


static INT8 *cec_command[] =
{
    "<Set OSD String>",
    "<Give OSD Name>",
    "<Set OSD Name>",
    "<Request Active Source>",
    "<System Standby>",
    "<Get CEC Version>",
    "<Give Physical Address>",
    "<Get Menu Language>",
    "<Set Menu Language>",
    "<Give Tuner Device Status>",
    "<Tuner Step Increment>",
    "<Tuner Step Decrement>",
    "<Give Device Vendor ID>",
    "<Give Device Power Status>",
    "<Sys_Aud_Mode_Req(on)>",
    "<Sys_Aud_Mode_Req(off)>",
    "<Set_Sys_Aud_Mode(on)>",
    "<Set_Sys_Aud_Mode(off)>",
    "<Abort>",
    "<Polling Message>"
};


enum
{
    HDCP_SWITCH_ID = 1,
    CEC_SWITCH_ID,
    CEC_RCP_ID,     //Remote Control Passthrough
    CEC_SAC_ID,     //System Audio Control
    CEC_SOURCE_STATE_ID,
    CEC_STANDBY_MODE_ID,
    CEC_CMD_SEL_ID,
    CEC_DEST_ADDR_ID,
    CEC_CMD_SEND_ID,
};

static INT8 *cec_standby_mode[] =
{
    "Local Standby",
    "<System Standby> Feature",
};


static INT8 *cec_source_state[] =
{
    "Active Source",
    "Inactive Source",
};



static INT8 *itme_txt_name[] =
{
    "HDMI+CEC",
    "CEC RCP(TV->STB)",
    "CEC System Audio",
    "CEC Source State",
    "CEC Standby Mode",
};

TEXT_CONTENT hdmi_cec_mtxt_content[] =
{
    {
        .b_text_type = STRING_UNICODE,
        .text.p_string = 0,
    },
};

#define ONOFF_ITEM_CNT ARRAY_SIZE(onoff_ids)
#define CEC_COMMAND_CNT ARRAY_SIZE(cec_command)
#define CEC_DEST_LA_CNT ARRAY_SIZE(cec_dest_logical_addr)
#define CEC_SOURCE_STATE_CNT ARRAY_SIZE(cec_source_state)
#define CEC_STANDBY_MODE_CNT ARRAY_SIZE(cec_standby_mode)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    cert_item_con_keymap,cert_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    cert_item_sel_keymap,cert_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)

#define LDEF_MTXT(root,var_mtxt,next_obj,l,t,w,h,cnt,content) \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,NULL,content)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,style,cur,cnt,ptbl)  \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,ID)   \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_MSEL(&var_con,var_txtset,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

LDEF_MM_ITEM(win_hdmi_certify_con, cert_item_con_hdmi_hdcp, &cert_item_con_cec_on_off, \
                cert_item_txtname_hdmi_hdcp, cert_item_txtset_hdmi_hdcp, HDCP_SWITCH_ID, \
                CEC_CMD_SEND_ID, CEC_SWITCH_ID, CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, \
                RS_HDMI_HDCP,STRING_ID,0,ONOFF_ITEM_CNT,onoff_ids)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_on_off, &cert_item_con_cec_rcp_feature, \
                cert_item_txtname_cec_on_off, cert_item_txtset_cec_on_off, CEC_SWITCH_ID, \
                HDCP_SWITCH_ID, CEC_RCP_ID, CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,\
                0,STRING_ID,0,ONOFF_ITEM_CNT, onoff_ids)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_rcp_feature, &cert_item_con_cec_system_audio_mode, \
                cert_item_txtname_cec_rcp_feature, cert_item_txtset_cec_rcp_feature, CEC_RCP_ID, \
                CEC_SWITCH_ID, CEC_SAC_ID, CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, \
                0,STRING_ID,0,ONOFF_ITEM_CNT, onoff_ids)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_system_audio_mode, &cert_item_con_cec_source_state,\
                cert_item_txtname_cec_system_audio_mode, cert_item_txtset_cec_system_audio_mode, \
                CEC_SAC_ID, CEC_RCP_ID, CEC_SOURCE_STATE_ID,CON_L, CON_T + (CON_H + CON_GAP)*3,\
                CON_W,CON_H, 0,STRING_ID,0,ONOFF_ITEM_CNT, onoff_ids)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_source_state, &cert_item_con_cec_standby_mode, \
                cert_item_txtname_cec_source_state, cert_item_txtset_cec_source_state,CEC_SOURCE_STATE_ID,\
                CEC_SAC_ID,CEC_STANDBY_MODE_ID,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, \
                0,STRING_ANSI,0,CEC_SOURCE_STATE_CNT, cec_source_state)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_standby_mode, &cert_item_con_cec_command_select, \
                cert_item_txtname_cec_standby_mode, cert_item_txtset_cec_standby_mode,CEC_STANDBY_MODE_ID,\
                CEC_SOURCE_STATE_ID,CEC_CMD_SEL_ID, CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, \
                0, STRING_ANSI,0,CEC_STANDBY_MODE_CNT, cec_standby_mode)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_command_select, &cert_item_con_cec_dest_address, \
                cert_item_txtname_cec_command_select,  cert_item_txtset_cec_command_select,CEC_CMD_SEL_ID,\
                CEC_STANDBY_MODE_ID,CEC_DEST_ADDR_ID,CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, \
                RS_HDMI_CEC_COMMAND,STRING_ANSI,0,CEC_COMMAND_CNT, cec_command)

LDEF_MM_ITEM(win_hdmi_certify_con,cert_item_con_cec_dest_address, &cert_item_con_cec_command_send, \
                cert_item_txtname_cec_dest_address,  cert_item_txtset_cec_dest_address,CEC_DEST_ADDR_ID,CEC_CMD_SEL_ID,\
                CEC_CMD_SEND_ID,CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, RS_HDMI_DEST_ADDRESS,STRING_ANSI,0,\
                CEC_DEST_LA_CNT, cec_dest_logical_addr)

LDEF_MENU_ITEM_TXT(win_hdmi_certify_con,cert_item_con_cec_command_send, NULL,cert_item_txtname_cec_command_send,\
                    CEC_CMD_SEND_ID,CEC_DEST_ADDR_ID,HDCP_SWITCH_ID,\
                    CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H,   RS_HDMI_SEND_COMMAND)

DEF_CONTAINER(win_hdmi_certify_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    cert_con_keymap,cert_con_callback,  \
    (POBJECT_HEAD)&cert_item_con_hdmi_hdcp, 1,0)



/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
void update_all_menu_item_selections(void)
{
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    SYSTEM_DATA* temp_sys_data;
    temp_sys_data=sys_data_get();
    if((temp_sys_data->hdmi_cec<0)||(temp_sys_data->hdmi_cec>1))
        temp_sys_data->hdmi_cec=1;
#endif
    if(api_get_hdmi_hdcp_onoff() == TRUE)
    {
        //libc_printf("hdcp_on\n");
        osd_set_multisel_sel(&cert_item_txtset_hdmi_hdcp,0);
    }
    else
    {
        //libc_printf("hdcp_off\n");
        osd_set_multisel_sel(&cert_item_txtset_hdmi_hdcp,1);
    }
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    if(temp_sys_data->hdmi_cec)
#else
    if(api_cec_get_func_enable_flag() == TRUE)
#endif
    {
        //libc_printf("cec_on\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_on_off,0);
    }
    else
    {
        //libc_printf("cec_off\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_on_off,1);
        check_hdmi_cec_control_state(0);
    }
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    if(temp_sys_data->hdmi_cec_rcp)
#else
    if(api_cec_get_remote_control_passthrough_feature()== TRUE)
#endif
    {
        //libc_printf("rcp_on\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_rcp_feature,0);
    }
    else
    {
        //libc_printf("rcp_off\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_rcp_feature,1);
    }


#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    if(temp_sys_data->hdmi_cec_sys_audio)
#else
    if(api_cec_get_system_audio_mode_status()== TRUE)
#endif
    {
        //libc_printf("rcp_on\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_system_audio_mode,0);
    }
    else
    {
        //libc_printf("rcp_off\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_system_audio_mode,1);
    }

#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    if(temp_sys_data->hdmi_cec_srs_state==CEC_SOURCE_STATE_ACTIVE)
#else
    if(api_cec_get_source_active_state() == CEC_SOURCE_STATE_ACTIVE)
#endif
    {
        //libc_printf("cec_active\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_source_state,0);
    }
    else
    {
        //libc_printf("cec_idle\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_source_state,1);
    }
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    if(temp_sys_data->hdmi_cec_stby_mode==CEC_SYSTEM_STANDBY_FEATURE_MODE)
#else
    if(api_cec_get_device_standby_mode() == CEC_SYSTEM_STANDBY_FEATURE_MODE)
#endif
    {
        //libc_printf("cec_system_standby_feature_on\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_standby_mode,1);
    }
    else
    {
        //libc_printf("cec_system_standby_feature_off\n");
        osd_set_multisel_sel(&cert_item_txtset_cec_standby_mode,0);
    }
}


static VACTION cert_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT cert_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 back_saved;
    PMULTISEL pmulsel;
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
    SYSTEM_DATA* temp_sys_data=sys_data_get();
#endif
    switch(event)
    {
        case EVN_PRE_DRAW:
            update_all_menu_item_selections();
            break;

        case EVN_PRE_CHANGE:

            break;

        case EVN_POST_CHANGE:
            if(osd_get_obj_id(p_obj) == HDCP_SWITCH_ID)
            {

                if(0 == param1)
                {
                    // Selection 0: ON
                    api_set_hdmi_hdcp_onoff(TRUE);

                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(200, 200, 500, 120);
                    win_compopup_set_msg_ext("Turn HDCP ON...",NULL,0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(2000);
                    win_compopup_smsg_restoreback();

                    if(api_get_hdmi_hdcp_onoff() == FALSE)
                    {
                        //win_messagebox_set_message(TRUE,1);

                        win_compopup_init(WIN_POPUP_TYPE_OK);
                        win_compopup_set_frame(200, 200, 500, 250);
                        win_compopup_set_msg_ext("Loading HDCP Key Fail!!",NULL,0);
                        win_compopup_open_ext(&back_saved);
                        win_compopup_smsg_restoreback();
                        osd_set_multisel_sel(&cert_item_txtset_hdmi_hdcp,1);
                        osd_track_object((POBJECT_HEAD )&cert_item_con_hdmi_hdcp,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                    }

                }
                else
                {
                    // Selection 1: OFF
                    api_set_hdmi_hdcp_onoff(FALSE);

                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(200, 200, 500, 120);
                    win_compopup_set_msg_ext("Turn HDCP OFF...",NULL,0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(2000);
                    win_compopup_smsg_restoreback();

                }
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_SWITCH_ID)
            {
                if(0 == param1)
                {
                    // Selection 0: ON
                    api_cec_set_func_enable(TRUE);
                    check_hdmi_cec_control_state(TRUE);
                }
                else
                {
                    // Selection 1: OFF
                    api_cec_set_func_enable(FALSE);
                    check_hdmi_cec_control_state(FALSE);
                }
#if(defined CEC_SUPPORT ||defined HDMI_CERTIFICATION_UI)
                temp_sys_data->hdmi_cec=(UINT8)api_cec_get_func_enable_flag();
                sys_data_save(1);
#endif
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_rcp_feature,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_system_audio_mode,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_source_state,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_standby_mode,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_command_select,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_dest_address,C_UPDATE_ALL);
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_command_send,C_UPDATE_ALL);
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_RCP_ID)
            {
                if(0 == param1)
                {
                    // Selection 0: On
                    api_cec_set_remote_control_passthrough_feature(TRUE);
                }
                else
                {
                    // Selection 1: Off
                    api_cec_set_remote_control_passthrough_feature(FALSE);
                }
                temp_sys_data->hdmi_cec_rcp=api_cec_get_remote_control_passthrough_feature();
                sys_data_save(1);
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_SAC_ID)
            {
                if(0 == param1)
                {
                    // Selection 0: On
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(200, 200, 500, 120);
                    win_compopup_set_msg_ext("Turn On System Audio Mode",NULL,0);
                    win_compopup_open_ext(&back_saved); //Show UI

                    cec_link_set_system_audio_mode_by_local_user(TRUE);

                    win_compopup_smsg_restoreback();        //Close UI

                    if(api_cec_get_system_audio_mode_status() == FALSE)
                    {

                        win_compopup_init(WIN_POPUP_TYPE_SMSG);
                        win_compopup_set_frame(200, 200, 500, 250);
                        win_compopup_set_msg_ext("Unable to turn on System Audio Mode!!",NULL,0);
                        win_compopup_open_ext(&back_saved);
                        osal_task_sleep(1000);
                        win_compopup_smsg_restoreback();
                        osd_set_multisel_sel(&cert_item_txtset_cec_system_audio_mode,1);  //set sel back to off(1)

                        osd_track_object((POBJECT_HEAD )&cert_item_con_cec_system_audio_mode,\
                                                    C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                    }

                }
                else
                {
                    // Selection 1: Off
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(200, 200, 500, 120);
                    win_compopup_set_msg_ext("Turn Off System Audio Mode",NULL,0);
                    win_compopup_open_ext(&back_saved); //Show UI
                    osal_task_sleep(1000);
                    cec_link_set_system_audio_mode_by_local_user(FALSE);
                    win_compopup_smsg_restoreback();        //Close UI

                }
                temp_sys_data->hdmi_cec_sys_audio=api_cec_get_system_audio_mode_status();
                sys_data_save(1);
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_SOURCE_STATE_ID)
            {
                if(0 == param1)
                {
                    // Selection 0: ACTIVE
                    api_cec_set_source_active_state(CEC_SOURCE_STATE_ACTIVE);
                }
                else
                {
                    // Selection 1: IDLE
                    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
                }
                temp_sys_data->hdmi_cec_srs_state=api_cec_get_source_active_state();
                sys_data_save(1);
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_STANDBY_MODE_ID)
            {
                if(0 == param1)
                {
                    // Selection 0: Local Standby Only
                    api_cec_set_device_standby_mode(CEC_LOCAL_STANDBY_MODE);
                }
                else
                {
                    // Selection 1: System Standby Feature
                    api_cec_set_device_standby_mode(CEC_SYSTEM_STANDBY_FEATURE_MODE);
                }
                temp_sys_data->hdmi_cec_stby_mode=api_cec_get_device_standby_mode();
                sys_data_save(1);
                ret = PROC_LOOP;
            }
            else if(osd_get_obj_id(p_obj) == CEC_CMD_SEL_ID)
            {

                pmulsel = &cert_item_txtset_cec_command_select;

                //libc_printf("SEL->nSel=%d(%s)\n", pmulsel->nSel, cec_command[pmulsel->nSel]);
                switch(pmulsel->n_sel)
                {
                    case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_ON:
                    case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_OFF:


                        osd_set_attr(&cert_item_con_cec_dest_address, C_ATTR_INACTIVE);
                        osd_set_attr(&cert_item_txtname_cec_dest_address, C_ATTR_INACTIVE);
                        osd_set_attr(&cert_item_txtset_cec_dest_address, C_ATTR_INACTIVE);
                        osd_set_multisel_sel(&cert_item_txtset_cec_dest_address, CEC_LA_AUDIO_SYSTEM);
                        break;

                    default:
                        osd_set_attr(&cert_item_con_cec_dest_address, C_ATTR_ACTIVE);
                        osd_set_attr(&cert_item_txtname_cec_dest_address, C_ATTR_ACTIVE);
                        osd_set_attr(&cert_item_txtset_cec_dest_address, C_ATTR_ACTIVE);
                        osd_set_multisel_sel(&cert_item_txtset_cec_dest_address, CEC_LA_TV);
                        break;

                }
                osd_draw_object((POBJECT_HEAD )&cert_item_con_cec_dest_address,C_UPDATE_ALL);
                ret = PROC_LOOP;

            }
            break;

        case EVN_REQUEST_STRING:

            break;

        case EVN_UNKNOWN_ACTION:
            break;
    }

    return ret;

}


static VACTION cert_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;

        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;

        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;

        default:
            act = VACT_PASS;
            break;
    }

    return act;

}

static PRESULT cert_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8  b_id;

    switch(event)
    {
        case EVN_FOCUS_PRE_GET:
            break;

        case EVN_FOCUS_PRE_LOSE:
            break;

        case EVN_UNKNOWN_ACTION:
            b_id = osd_get_obj_id(p_obj);
            if(CEC_CMD_SEND_ID == b_id)
            {
                if(VACT_ENTER ==  ((param1>>NON_ACTION_LENGTH) & 0x0000FFFF))
                {
                    win_cec_enter_key();
                }
                ret = PROC_LOOP;
            }
            break;
    }

    return ret;
}

static VACTION cert_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;

        default:
            act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT cert_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
        PRESULT ret = PROC_PASS;
        //PMULTISEL pmulsel;
        switch(event)
        {
            case EVN_PRE_DRAW:
            update_all_menu_item_selections();
            break;

        case EVN_PRE_OPEN:
            osd_set_text_field_str_point(&cert_item_txtname_cec_on_off,display_strs[0]);
            osd_set_text_field_str_point(&cert_item_txtname_cec_rcp_feature,display_strs[1]);
            osd_set_text_field_str_point(&cert_item_txtname_cec_system_audio_mode,display_strs[2]);
            osd_set_text_field_str_point(&cert_item_txtname_cec_source_state,display_strs[3]);
            osd_set_text_field_str_point(&cert_item_txtname_cec_standby_mode,display_strs[4]);
            osd_set_text_field_content(&cert_item_txtname_cec_on_off,STRING_ANSI,(UINT32)(itme_txt_name[0]));
            osd_set_text_field_content(&cert_item_txtname_cec_rcp_feature,STRING_ANSI,(UINT32)(itme_txt_name[1]));
            osd_set_text_field_content(&cert_item_txtname_cec_system_audio_mode,STRING_ANSI,(UINT32)(itme_txt_name[2]));
            osd_set_text_field_content(&cert_item_txtname_cec_source_state,STRING_ANSI,(UINT32)(itme_txt_name[3]));
            osd_set_text_field_content(&cert_item_txtname_cec_standby_mode,STRING_ANSI,(UINT32)(itme_txt_name[4]));
            break;

        case EVN_POST_OPEN:
            break;

        case EVN_PRE_CLOSE:
            break;

        case EVN_POST_CLOSE:
            break;
    }
    return ret;
}

void win_cec_enter_key(void)
{

    MULTISEL *p_sel;

    E_CEC_LOGIC_ADDR    dest_address;

    UINT8 cec_cmd_selection;
    UINT8 back_saved;
    UINT8 disp_buf[64];

    p_sel = &cert_item_txtset_cec_dest_address;

    dest_address = p_sel->n_sel;

        p_sel = &cert_item_txtset_cec_command_select;

    cec_cmd_selection = p_sel->n_sel%CEC_COMMAND_CNT;

    libc_printf("%s \n",((UINT8**)p_sel->p_sel_table)[cec_cmd_selection]);

    switch(cec_cmd_selection)
    {

        case CEC_MSG_SET_OSD_STRING:  //"<Set OSD String>"
            ap_cec_msg_set_osd_string(dest_address, (UINT8 *)"ALi.Demo.Str");
            break;

        case CEC_MSG_GIVE_OSD_NAME:  //"<Give OSD Name>",
            ap_cec_msg_give_osd_name(dest_address);
            break;

        case CEC_MSG_SET_OSD_NAME:  //"<Set OSD Name>",
            ap_cec_msg_set_osd_name(dest_address, (UINT8 *)"ALi STB M3603");
            break;

        case CEC_MSG_REQUEST_ACTIVE_SOUSE:  //"<Request Active Source>",
            ap_cec_msg_request_active_source();
            break;

        case CEC_MSG_SYSTEM_STANDBY:  //"<System Standby>",
            ap_cec_msg_system_standby(dest_address);
            ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER, FALSE);
            break;

        case CEC_MSG_GET_CEC_VERSION:  //"<Get CEC Version>",
            ap_cec_msg_get_cec_version(dest_address);
            break;

        case CEC_MSG_GIVE_PHYSICAL_ADDRESS:  //"<Give Physical Address>",
            ap_cec_msg_give_physical_address(dest_address);
            break;

        case CEC_MSG_GET_MENU_LANGUAGE:  //"<Get Menu Language>",
            ap_cec_msg_get_menu_language(dest_address);
            break;

        case CEC_MSG_SET_MENU_LANGUAGE:
            ap_cec_msg_set_menu_language(dest_address, (UINT8 *)"eng", 3);
            break;

        case CEC_MSG_GIVE_TUNER_DEVICE_STATUS:  //"<Give Tuner Device Status>",
            ap_cec_msg_give_tuner_device_status(dest_address, CEC_STATUS_REQUEST_ONCE);
            break;

        case CEC_MSG_TUNER_STEP_INCREMENT:
            ap_cec_msg_tuner_step_increment(dest_address);
            break;

        case CEC_MSG_TUNER_STEP_DECREMENT:
            ap_cec_msg_tuner_step_decrement(dest_address);
            break;

        case CEC_MSG_GIVE_DEVICE_VENDOR_ID:  //"<Give Device Vendor ID>",
            ap_cec_msg_give_device_vendor_id(dest_address);
            break;

        case CEC_MSG_GIVE_DEVICE_POWER_STATUS:  //"<Give Device Power Status>",
            ap_cec_msg_give_device_power_status(dest_address);
            break;

        case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_ON:
            ap_cec_msg_system_audio_mode_request(TRUE);
            break;

        case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_OFF:
            ap_cec_msg_system_audio_mode_request(FALSE);
            break;


        case CEC_MSG_SET_SYSTEM_AUDIO_MODE_ON:
            ap_cec_msg_set_system_audio_mode(dest_address, TRUE);
            ap_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, TRUE);
            break;

        case CEC_MSG_SET_SYSTEM_AUDIO_MODE_OFF:
            ap_cec_msg_set_system_audio_mode(dest_address, FALSE);
            break;

        case CEC_MSG_ABORT:  //"<Abort>",
            ap_cec_msg_abort(dest_address);
            break;

        case CEC_MSG_POLLING_MESSAGE:  //"<Polling Message>"
            ap_cec_msg_polling_message(dest_address);
            break;

        default:
            break;
    }
    /*control part*/

    snprintf((char *)disp_buf,64,"SendTo(%d): %s\n", dest_address, ((UINT8**)p_sel->p_sel_table)[cec_cmd_selection]);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext((char *)disp_buf,NULL,0);
    win_compopup_set_frame(200, 200, 600, 120);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(2000);
    win_compopup_smsg_restoreback();
    /**/

}

void check_hdmi_cec_control_state(BOOL enable)
{
    libc_printf("check_hdmi_cec_control_state(enabled=%d)\n", enable);

    if(enable)
    {
        osd_set_attr(&cert_item_con_cec_rcp_feature, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_rcp_feature, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_rcp_feature, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_system_audio_mode, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_system_audio_mode, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_system_audio_mode, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_source_state, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_source_state, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_source_state, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_standby_mode, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_standby_mode, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_standby_mode, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_command_select, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_command_select, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_command_select, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_dest_address, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_dest_address, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtset_cec_dest_address, C_ATTR_ACTIVE);

        osd_set_attr(&cert_item_con_cec_command_send, C_ATTR_ACTIVE);
        osd_set_attr(&cert_item_txtname_cec_command_send, C_ATTR_ACTIVE);

    }
    else
    {
        osd_set_attr(&cert_item_con_cec_rcp_feature, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_rcp_feature, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_rcp_feature, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_system_audio_mode, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_system_audio_mode, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_system_audio_mode, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_source_state, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_source_state, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_source_state, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_standby_mode, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_standby_mode, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_standby_mode, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_command_select, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_command_select, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_command_select, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_dest_address, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_dest_address, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtset_cec_dest_address, C_ATTR_INACTIVE);

        osd_set_attr(&cert_item_con_cec_command_send, C_ATTR_INACTIVE);
        osd_set_attr(&cert_item_txtname_cec_command_send, C_ATTR_INACTIVE);

    }
}

#endif
#endif
