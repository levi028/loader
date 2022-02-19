/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_misc_setting.c
*
*    Description: misc set menu(mainmenu -> system setup -> other).
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <api/libpub/lib_cc.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
#include "platform/board.h"
#include "platform/board_cfg/board_cfg.h"
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_misc_setting.h"

#ifdef RAM_TMS_TEST
#include "disk_manager.h"
#endif

#ifdef DVBC_MODE_CHANGE
#include <hld/nim/nim_tuner.h>
#endif

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef DVBC_MODE_CHANGE
#include "power.h"
#endif
#ifdef RD_SKIP_APS
#include "conax_ap/win_ca_uri.h"
#endif

#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

/*******************************************************************************
* Objects declaration
*******************************************************************************/
//extern CONTAINER g_win_misc_setting;

static VACTION miscset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT miscset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION miscset_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT miscset_item_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION miscset_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT miscset_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);


#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

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

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 60)
#define CON_H       40
#define CON_GAP     12

#define TXT_L_OF    10
#define TXT_W       300
#define TXT_H       CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       (CON_W-320)
#define SEL_H       CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#else
#define W_L         105//384
#define W_T         57 //138
#define W_W         482
#define W_H         370

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 40)
#define CON_H       30
#define CON_GAP     12

#define TXT_L_OF    10
#define TXT_W       180
#define TXT_H       CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       (CON_W-20-TXT_W)
#define SEL_H       CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#endif
#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    miscset_item_keymap,miscset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    miscset_item_sel_keymap,miscset_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    miscset_keymap,miscset_callback,  \
    nxt_obj, focus_id,0)


UINT16 onoff_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
    RS_SYSTEM_TV_SYSTEM_AUTO
};

#ifdef PLSN_SUPPORT
UINT16 plsn_search_ids[] = 
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};
#endif

#ifdef PLSN_SUPPORT
LDEF_MENU_ITEM(g_win_misc_setting,miscset_item0,&miscset_item1,miscset_txt0,miscset_sel0,miscset_line0, 1, 13, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INFO_LNB_POWER, STRING_ID, 0, 2, lnbpower_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item1,&miscset_item2,miscset_txt1,miscset_sel1,miscset_line1, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, 0, STRING_ID, 0, 2, antenna_power_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item2,&miscset_item3,miscset_txt2,miscset_sel2,miscset_line2, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_SYSTME_CHANCHANGEG_TYPE, STRING_ID, 0, 3, chan_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item3,&miscset_item4,miscset_txt3,miscset_sel3,miscset_line3, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_CHAN_CHANGE, STRING_ID, 0, 2, chtype_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item4,&miscset_item5,miscset_txt4,miscset_sel4,miscset_line4, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_BEEPER      , STRING_ID, 1, 2, onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item5,&miscset_item6,miscset_txt5,miscset_sel5,miscset_line5, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, 0/*RS_BEEPER*/, STRING_ID, 0, 2, auto_stdy_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item6,&miscset_item7,miscset_txt6,miscset_sel6,miscset_line6, 7, 6, 8, \
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, 0, STRING_ID, 0, 2, ram_tms_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item7,&miscset_item8,miscset_txt7,miscset_sel7,miscset_line7, 8, 7, 9, \
        CON_L, CON_T + (CON_H + CON_GAP)*7, CON_W, CON_H, 0, STRING_ANSI, 0, 2, dvbc_mode_stritem)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item8,&miscset_item9,miscset_txt8,miscset_sel8,miscset_line8, 9, 8, 10, \
        CON_L, CON_T + (CON_H + CON_GAP)*8, CON_W, CON_H, 0, STRING_ANSI, 0, 1, version_str)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item9,&miscset_item10,miscset_txt9,miscset_sel9,miscset_line9, 10, 9, 11, \
        CON_L, CON_T + (CON_H + CON_GAP)*9, CON_W, CON_H, 0, STRING_ID, 0, 2, auto_stdy_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item10,&miscset_item11,miscset_txt10,miscset_sel10,miscset_line10, 11, 10, 12, \
		CON_L, CON_T + (CON_H + CON_GAP)*10, CON_W, CON_H, 0, STRING_ID, 0, 2, fsc_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item11,&miscset_item12,miscset_txt11,miscset_sel11,miscset_line11, 12, 11, 13, \
		CON_L, CON_T + (CON_H + CON_GAP)*11, CON_W, CON_H, 0, STRING_ID, 0, 2, flash_sp_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting, miscset_item12, NULL, miscset_txt12, miscset_sel12, miscset_line12, 13, 12, 1, \
        CON_L, CON_T + (CON_H + CON_GAP) * 12, CON_W, CON_H, RS_PLSN_FLAG, STRING_ID, 0, 2, plsn_search_ids)

#else
LDEF_MENU_ITEM(g_win_misc_setting,miscset_item0,&miscset_item1,miscset_txt0,miscset_sel0,miscset_line0, 1, 12, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INFO_LNB_POWER, STRING_ID, 0, 2, lnbpower_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item1,&miscset_item2,miscset_txt1,miscset_sel1,miscset_line1, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, 0, STRING_ID, 0, 2, antenna_power_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item2,&miscset_item3,miscset_txt2,miscset_sel2,miscset_line2, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_SYSTME_CHANCHANGEG_TYPE, STRING_ID, 0, 3, chan_sw_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item3,&miscset_item4,miscset_txt3,miscset_sel3,miscset_line3, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_CHAN_CHANGE, STRING_ID, 0, 2, chtype_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item4,&miscset_item5,miscset_txt4,miscset_sel4,miscset_line4, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_BEEPER      , STRING_ID, 1, 2, onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item5,&miscset_item6,miscset_txt5,miscset_sel5,miscset_line5, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, 0/*RS_BEEPER*/, STRING_ID, 0, 2, auto_stdy_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item6,&miscset_item7,miscset_txt6,miscset_sel6,miscset_line6, 7, 6, 8, \
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, 0, STRING_ID, 0, 2, ram_tms_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item7,&miscset_item8,miscset_txt7,miscset_sel7,miscset_line7, 8, 7, 9, \
        CON_L, CON_T + (CON_H + CON_GAP)*7, CON_W, CON_H, 0, STRING_ANSI, 0, 2, dvbc_mode_stritem)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item8,&miscset_item9,miscset_txt8,miscset_sel8,miscset_line8, 9, 8, 10, \
        CON_L, CON_T + (CON_H + CON_GAP)*8, CON_W, CON_H, 0, STRING_ANSI, 0, 1, version_str)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item9,&miscset_item10,miscset_txt9,miscset_sel9,miscset_line9, 10, 9, 11, \
        CON_L, CON_T + (CON_H + CON_GAP)*9, CON_W, CON_H, 0, STRING_ID, 0, 2, auto_stdy_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item10,&miscset_item11,miscset_txt10,miscset_sel10,miscset_line10, 11, 10, 12, \
		CON_L, CON_T + (CON_H + CON_GAP)*10, CON_W, CON_H, 0, STRING_ID, 0, 2, fsc_onoff_ids)

LDEF_MENU_ITEM(g_win_misc_setting,miscset_item11,NULL,miscset_txt11,miscset_sel11,miscset_line11, 12, 11, 1, \
		CON_L, CON_T + (CON_H + CON_GAP)*11, CON_W, CON_H, 0, STRING_ID, 0, 2, flash_sp_onoff_ids)
#endif

LDEF_WIN(g_win_misc_setting,&miscset_item0,W_L, W_T, W_W, W_H, 1)

typedef enum
{
    LNBPOWER_ID = 1,
    ANTENNAPOWER_ID,
    CHANCHG_CHAN_ID,
    CHANCHG_VIDEO_ID,
    INSTALLBEEP_ID,
    AUTO_STANDBY_ID,
    RAM_TMS_ID,
    DVBC_MODE_ID,
    LIB_VERSION_ID,
    APS_ID,
    FSC_ID,
    FLASH_SP_ID,
#ifdef PLSN_SUPPORT
    PLSN_SEARCH_ID,
#endif
}MISC_SETTING_ID;
/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/

UINT16 lnbpower_sw_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

UINT16 antenna_power_sw_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

UINT16 chan_sw_ids[] =
{
    RS_INSTALLATION_SCAN_MODE_FREE,
    RS_SYSTME_CHANNEL_SCRAMBLED,
    RS_INSTALLATION_SCAN_MODE_ALL
};

UINT16 chtype_ids[] =
{
    RS_CHAN_CHANGE_BLACKSCREEN,
    RS_CHAN_CHANGE_FREEZEVIDEO,
};


UINT16 auto_stdy_onoff_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

UINT16 fsc_onoff_ids[] = 
{
	RS_COMMON_OFF,
	RS_COMMON_ON
};

CHAR *fsc_onoff_strs[] = 
{
	"OFF",
//	"Single Tuner",
	"Multi Tuner",
};

UINT16 ram_tms_onoff_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON
};

CHAR *dvbc_mode_stritem[] =
{
    "J83AC",
    "J83B"
};

UINT16 flash_sp_onoff_ids[] = 
{
	RS_COMMON_OFF,
	RS_COMMON_ON
};

CHAR *version_str[] = {" Jan 01 00:00:00 2013  "};

__MAYBE_UNUSED__ static CONTAINER *misc_menu_items[] =
{
    &miscset_item0,
    &miscset_item1,
    &miscset_item2,
    &miscset_item3,
    &miscset_item4,
    &miscset_item5,
    &miscset_item6,
    &miscset_item7,
    &miscset_item8,
    &miscset_item9,
#ifdef PLSN_SUPPORT
    &miscset_item12,
#endif
};

static void win_miscset_load_setting(void);
static void win_miscset_save_setting(void);
static void check_front_end_is_s(POBJECT_HEAD p_obj);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION miscset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

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

static PRESULT miscset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event,UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 sel = 0;
    UINT8 b_id = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT8 back_saved = 0;

	if(0 == back_saved)
	{
		;
	}
#ifdef DVBC_MODE_CHANGE
    struct DEMOD_CONFIG_ADVANCED demod_config;

    MEMSET(&demod_config, 0x0, sizeof(struct DEMOD_CONFIG_ADVANCED));
#endif
    b_id = osd_get_obj_id(p_obj);

    sys_data = sys_data_get();

    back_saved = 0;
    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        if(LNBPOWER_ID == b_id)
        {
            if(0 == sel)
            {
                set_lnbshort_detect(sel);
            }
            sys_data->b_lnb_power = sel;
            board_lnb_power(0, system_config.b_lnb_power);
            board_lnb_power(1, system_config.b_lnb_power);
            set_lnbshort_detect_start(sys_data->b_lnb_power);
            if(1 == sel)
            {
                set_lnbshort_detect(sel);
            }
        }
        else if(ANTENNAPOWER_ID == b_id)
        {
            sys_data->antenna_power = sel;
            board_antenna_power(0, system_config.antenna_power);
            board_antenna_power(1, system_config.antenna_power);
        }
        else if(CHANCHG_CHAN_ID == b_id)
        {
            sys_data->chan_sw = sel;
        }
        else if(CHANCHG_VIDEO_ID == b_id)
        {
            sys_data->chchgvideo_type = sel;
        }
        else if(INSTALLBEEP_ID == b_id)
        {
            sys_data->ber_printf = sel;
        }
        else if(AUTO_STANDBY_ID == b_id)
        {
            sys_data->auto_standby_en= sel;
        }
/*
        else if(RAM_TMS_ID == b_id)
            sys_data->ram_tms_en = sel;
*/
#ifdef DVBC_MODE_CHANGE
        else if(DVBC_MODE_ID == b_id)
        {
            g_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

            if(NULL != g_nim_dev)
            {
                front_end_c_get_qam_config(sel,NIM_SAMPLE_CLK_27M,&demod_config);
                nim_ioctl_ext(g_nim_dev, NIM_DRIVER_SET_NIM_MODE, (void *)&demod_config);
            }

            sys_data->dvbc_mode= sel;
            sys_data_save(1);

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext("STB will reboot after 1s ... ", NULL, 0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();

            key_pan_display("OFF ",4);
            osal_task_sleep(500);
            power_off_process(0);
            power_on_process();

        }
#endif
#ifdef RD_SKIP_APS
        else if(APS_ID == b_id)
        {
            g_skip_aps=sel;
        }
#endif
#ifdef FSC_SUPPORT
        else if(FSC_ID == b_id )
        {
            //sys_data->fsc_en= sel;
            if(sel)
            {
                sys_data_set_fsc_onoff(1);
                //fcc_set_tuner_num(sel);
            }
            else
            {
                sys_data_set_fsc_onoff(sel);
            }
        }
#endif
#ifdef FLASH_SOFTWARE_PROTECT_TEST
        else if(FLASH_SP_ID == b_id )
        {
            sys_data->flash_sp_test = sel;
            sys_data_save(1);
        }
#endif
#ifdef PLSN_SUPPORT
        else if(PLSN_SEARCH_ID == b_id)
        {
            sys_data->super_scan = sel;
        }
#endif
        break;
    default :
        break;
    }
    return ret;
}

static VACTION miscset_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT miscset_item_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION miscset_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act =VACT_PASS;

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
        break;
    }

    return act;
}

static PRESULT miscset_callback(POBJECT_HEAD p_obj, VEVENT event,UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_miscset_load_setting();
        check_front_end_is_s(p_obj);
        wincom_open_title(p_obj,RS_OTHER,0);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        win_miscset_save_setting();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_MSG_GOT:
        break;
    default :
        break;
    }

    return ret;
}


#if defined(SUPPORT_CAS9) | defined(SUPPORT_CAS7) 
#define MAX_VERSION_LEN 45
static void get_ca_version_str()
{
    UINT8 i = 0;
    UINT8 bneed_modified = FALSE;
    UINT8 str_tmp[MAX_VERSION_LEN+1];
    UINT8 ch;
    UINT8 real_len = 0;
    
    strncpy(version_str[0], get_ca_lib_ver()+4, 21);
    for(i = 0 ; i< 21; i++)
    {
        ch = version_str[0][i];
        if( ch > 0x7f) //
        {
            bneed_modified = TRUE;
            break;
        }
    }

    if(bneed_modified)
    {
        MEMSET(str_tmp,0,(MAX_VERSION_LEN+1));
        strncpy((char *)str_tmp, (const char *)get_ca_lib_ver(), MAX_VERSION_LEN);
        
        str_tmp[MAX_VERSION_LEN] = 0;
        real_len = 0;
        
        for(i = 0 ; i< MAX_VERSION_LEN; i++)
        {
            //libc_printf("0x%02x ",str_tmp[i]);
            if((str_tmp[i]>=0X30)&&(str_tmp[i]<=0X39)) //DIG
            {     
                str_tmp[real_len] = str_tmp[i];
                real_len ++;                
            }
            else if((0X20 == str_tmp[i] ) || ( 0X3A == str_tmp[i] )) //0X3A :
            {     
                str_tmp[real_len] = str_tmp[i];
                real_len ++;
            }
            else if(( 0XB4 == str_tmp[i] ) || ( 0X88 == str_tmp[i] )) //year and month
            {     
                str_tmp[real_len] = 0x2F;
                real_len ++;
            }
            else if((0X00 == str_tmp[i]) ||(0XFF == str_tmp[i]) )
            {
                break;
            }
            else
            {
                //libc_printf("0x%02x ",str_tmp[i]);
                //version_str[0][i] = 0X2D;//
            }
        }
        version_str[0][real_len] = 0;
        strncpy((char *)version_str[0], (const char *)str_tmp, real_len);
        //libc_printf("\n",str_tmp[i]);
    }
}
#endif

static void win_miscset_load_setting(void)
{
    CONTAINER   *win=NULL;
    MULTISEL    *msel = NULL;
    TEXT_FIELD *p_txt = NULL;
    SYSTEM_DATA *sys_data = NULL;
    BOOL beeper_flag = FALSE;
    BOOL lnb_power_flag = FALSE;
    BOOL chchg_video_flag = FALSE;
    const UINT32 chan_switch_threshold = 3;

	if(FALSE == beeper_flag)
	{
		;
	}
	if(FALSE == lnb_power_flag)
	{
		;
	}
	if(FALSE == chchg_video_flag)
	{
		;
	}
	sys_data = sys_data_get();
    /* LNB Power */
    msel =&miscset_sel0;
    osd_set_multisel_sel(msel, sys_data->b_lnb_power);

    msel =&miscset_sel1;
    p_txt = &miscset_txt1;
    osd_set_multisel_sel(msel, sys_data->antenna_power);
    osd_set_text_field_str_point(p_txt,display_strs[1]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"Antenna Power");

    /* Change change channel type */
    if(sys_data->chan_sw >= chan_switch_threshold)
    {
        sys_data->chan_sw = 0;
    }
    msel =&miscset_sel2;
    osd_set_multisel_sel(msel, sys_data->chan_sw);

    /* Change change video type */
    msel =&miscset_sel3;
    osd_set_multisel_sel(msel, sys_data->chchgvideo_type);

    /* Beep */
    msel =&miscset_sel4;
    osd_set_multisel_sel(msel, sys_data->install_beep);


    win = &g_win_misc_setting;
#if( SYS_LNB_POWER_OFF ==  SYS_FUNC_ON)
    lnb_power_flag = TRUE;
#else
    lnb_power_flag = FALSE;
#endif

#ifdef CHANCHG_VIDEOTYPE_SUPPORT
    chchg_video_flag = TRUE;
#else
    chchg_video_flag = FALSE;
#endif

#ifdef ANTENNA_INSTALL_BEEP
    beeper_flag = TRUE;
#else
    beeper_flag = FALSE;
#endif

    #if ( SYS_LNB_POWER_OFF !=  SYS_FUNC_ON)
    remove_menu_item(win, (OBJECT_HEAD *)misc_menu_items[LNBPOWER_ID - 1], CON_H + CON_GAP);
    #endif

    #ifndef CHANCHG_VIDEOTYPE_SUPPORT
    remove_menu_item(win, (OBJECT_HEAD *)misc_menu_items[CHANCHG_VIDEO_ID - 1], CON_H + CON_GAP);
    #endif

    #ifndef ANTENNA_INSTALL_BEEP
    remove_menu_item(win, (OBJECT_HEAD *)misc_menu_items[INSTALLBEEP_ID - 1], CON_H + CON_GAP);
    #endif

#ifndef RAM_TMS_TEST
    remove_menu_item(win, (OBJECT_HEAD *)&miscset_item6, CON_H + CON_GAP);
#endif

#if !defined(AUTOMATIC_STANDBY)
    remove_menu_item(win,(POBJECT_HEAD)&miscset_item4,CON_H + CON_GAP);
#endif

#ifndef DVBC_MODE_CHANGE
    remove_menu_item(win,(POBJECT_HEAD)&miscset_item7,CON_H + CON_GAP);
#endif

    /*Disable/Endable auto standby*/
    msel = &miscset_sel5;
    p_txt = &miscset_txt5;
    osd_set_multisel_sel(msel,sys_data->auto_standby_en);
    osd_set_text_field_str_point(p_txt,display_strs[2]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"Auto Standby");

    /* Disable/Endable ram timeshift */
#ifdef RAM_TMS_TEST
    msel = &miscset_sel6;
    p_txt = &miscset_txt6;
    osd_set_multisel_sel(msel,sys_data->ram_tms_en);
    osd_set_text_field_str_point(p_txt,display_strs[3]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"RAM Timeshift");
#endif

#ifdef DVBC_MODE_CHANGE
    msel = &miscset_sel7;
    osd_set_multisel_sel(msel, sys_data->dvbc_mode);
    p_txt = &miscset_txt7;
    osd_set_text_field_str_point(p_txt,display_strs[4]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"DVBC Mode");
    if(sys_data_get_group_num() > 0)
    {
        osd_set_attr(&miscset_item7, C_ATTR_INACTIVE);
        osd_set_attr(&miscset_txt7, C_ATTR_INACTIVE);
        osd_set_attr(&miscset_sel7, C_ATTR_INACTIVE);
    }
    else
    {
        osd_set_attr(&miscset_item7, C_ATTR_ACTIVE);
        osd_set_attr(&miscset_txt7, C_ATTR_ACTIVE);
        osd_set_attr(&miscset_sel7, C_ATTR_ACTIVE);
    }
#endif

#if defined(SUPPORT_CAS9) | defined(SUPPORT_CAS7)
    //strncpy(version_str[0], get_ca_lib_ver()+4, 21);
    get_ca_version_str();
    msel = &miscset_sel8;
    p_txt = &miscset_txt8;
    osd_set_multisel_sel(msel,0);
    osd_set_text_field_str_point(p_txt,display_strs[5]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"LIB Version");

    osd_set_attr(&miscset_item8, C_ATTR_INACTIVE);
    osd_set_attr(&miscset_txt8, C_ATTR_INACTIVE);
    osd_set_attr(&miscset_sel8, C_ATTR_INACTIVE);
#else
    remove_menu_item(win, (OBJECT_HEAD *)&miscset_item8, CON_H + CON_GAP);
#endif

#ifdef RD_SKIP_APS
    msel = &miscset_sel9;
    p_txt = &miscset_txt9;
    osd_set_multisel_sel(msel,g_skip_aps);
    osd_set_text_field_str_point(p_txt,display_strs[6]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"Skip APS");
#else
    remove_menu_item(win, (OBJECT_HEAD *)&miscset_item9, CON_H + CON_GAP);
#endif

#ifdef FSC_SUPPORT
    /*Disable/Endable auto standby*/
    msel = &miscset_sel10;
    p_txt = &miscset_txt10;
    if(g_tuner_num > 1)
    {
        osd_set_multisel_sel_type(msel,STRING_ANSI);
        osd_set_multisel_sel_table(msel,fsc_onoff_strs);
        osd_set_multisel_count(msel,sizeof(fsc_onoff_strs)/sizeof(fsc_onoff_strs[0]));

        //
        if(0)//sys_data->fsc_en) 
        {
            osd_set_multisel_sel(msel,fcc_get_tuner_num());
        }
        else
        {
            osd_set_multisel_sel(msel,sys_data->fsc_en);
        }
    }
    else
    {
        osd_set_multisel_sel(msel,sys_data->fsc_en);
    }
    osd_set_text_field_str_point(p_txt,display_strs[7]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"FCC Support");
#else
    remove_menu_item(win,(POBJECT_HEAD)&miscset_item10,CON_H + CON_GAP);		
#endif

#ifdef FLASH_SOFTWARE_PROTECT_TEST
    msel = &miscset_sel11;
    p_txt = &miscset_txt11;
    osd_set_multisel_sel(msel,sys_data->flash_sp_test);
    osd_set_text_field_str_point(p_txt,display_strs[8]);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"Flash Protect Test");
#else
    remove_menu_item(win,(POBJECT_HEAD)&miscset_item11,CON_H + CON_GAP);
#endif

#ifdef PLSN_SUPPORT
    msel = &miscset_sel12;
    osd_set_multisel_sel(msel, sys_data->super_scan);
#endif
}

static void win_miscset_save_setting(void)
{
    MULTISEL    *msel=NULL;
    SYSTEM_DATA *sys_data=NULL;
    UINT32 val = 0;

#ifndef NEW_DEMO_FRAME
    struct nim_lnb_info lnb_info;

#else
    struct nim_config lnb_info;
    struct nim_device *nim = NULL;
    UINT32 sub_type = 0;
    UINT16 i = 0;

#endif
#ifdef RAM_TMS_TEST
    char rec_part[16] = {0};
    char tms_part[16] = {0};
    UINT8 back_saved = 0;
    UINT32 ram_len = 0;
    UINT32 ram_addr = 0;

#endif
    sys_data = sys_data_get();

#ifndef NEW_DEMO_FRAME
    MEMSET(&lnb_info,0,sizeof(struct nim_lnb_info));
    for(i=0;i<2;i++)
    {
        lib_nimg_get_lnb_info(i+1,&lnb_info);
        if(LNB_POWER_OFF == sys_data->b_lnb_power)
        {
            lnb_info.lnb_power_off = 1;
        }
        else
        {
            lnb_info.lnb_power_off = 0;
        }
        lib_nimg_set_lnb_info(i+1,&lnb_info);
    }
#else
    MEMSET(&lnb_info,0,sizeof(struct nim_config));
    for(i=0;i<2;i++)
    {
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, i);
        sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        dev_get_nim_config(nim, sub_type, &lnb_info);
        if(LNB_POWER_OFF == sys_data->b_lnb_power)
        {
            lnb_info.antenna.antenna_enable = /*0*/1;   //Seiya fix BUG37540:
        }
        else
        {
            lnb_info.antenna.antenna_enable = 0;// 1;   //Evan BUG45490
        }
        dev_set_nim_config(nim, sub_type, &lnb_info);
    }
#endif

    msel =&miscset_sel1;
    val = osd_get_multisel_sel(msel);
    sys_data->antenna_power= val;

    msel =&miscset_sel2;
    val = osd_get_multisel_sel(msel);
    sys_data->chan_sw = val;

    msel =&miscset_sel3;
    val = osd_get_multisel_sel(msel);
    sys_data->chchgvideo_type = val;

#ifdef CHANCHG_VIDEOTYPE_SUPPORT
    uich_chg_set_video_type(sys_data->chchgvideo_type);
#endif

    msel =&miscset_sel4;
    val = osd_get_multisel_sel(msel);
    sys_data->install_beep = val;

    msel =&miscset_sel5;
    val = osd_get_multisel_sel(msel);
    sys_data->auto_standby_en= val;

#ifdef RAM_TMS_TEST
    msel =&miscset_sel6;
    val = osd_get_multisel_sel(msel);

    if (sys_data->ram_tms_en != val) // detach tms part
    {
        rec_part[0] = tms_part[0] = 0;
        pvr_get_cur_mode(rec_part, sizeof(rec_part), tms_part, sizeof(tms_part));
        if (tms_part[0] != 0)
        {
            if (0 == STRCMP(rec_part, tms_part))
            {
                pvr_set_disk_use(PVR_REC_ONLY_DISK, rec_part);
            }
            else
            {
                pvr_detach_part(tms_part, PVR_TMS_ONLY_DISK);
            }
            api_pvr_adjust_tms_space();
        }
    }

    if ((0 == sys_data->ram_tms_en) && (1 == val)) // enable RAM disk timeshift
    {
        ram_len = RAM_DISK_SIZE;
        ram_addr = (void *)(RAM_DISK_ADDR & 0x0fffffff | 0x80000000);

        ramdisk_create((UINT32)ram_addr, ram_len);

        struct pvr_register_info pvr_reg_info;
        MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
        strncpy(pvr_reg_info.mount_name, "/mnt/rda1", (16-1));
        pvr_reg_info.disk_usage = PVR_TMS_ONLY_DISK;
        pvr_reg_info.sync = 1;
        pvr_reg_info.init_list = 0;
        pvr_reg_info.check_speed = 0;
        pvr_register((UINT32)&pvr_reg_info, 0);
    }
    else if (( 1 == sys_data->ram_tms_en) && (0 == val)) //disable RAM disk timeshift
    {
        ramdisk_delete();
    }

    if (sys_data->ram_tms_en != val)
    {
        if (0 == val) // disable RAM disk timeshift, select PVR partition again
        {
            rec_part[0] = 0;
            tms_part[0] = 0;
            pvr_select_part(rec_part,16, tms_part,16);
            if ((rec_part[0] != 0) || (tms_part[0] != 0))
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext("Init PVR partitions, please wait...", NULL, 0);
                win_compopup_open_ext(&back_saved);
                if (0 == STRCMP(rec_part, tms_part))
                {
                    pvr_change_part(rec_part, PVR_REC_AND_TMS_DISK);
                }
                else
                {
                    pvr_change_part(rec_part, PVR_REC_ONLY_DISK);
                    pvr_change_part(tms_part, PVR_TMS_ONLY_DISK);
                }
                win_compopup_smsg_restoreback();
            }
        }

        struct dvr_hdd_info hdd_info;
        pvr_get_hdd_info(&hdd_info);
        api_pvr_check_level(&hdd_info);
        if (PVR_DISK_INVALID == pvr_get_cur_mode(NULL, 0, NULL, 0))
        {
            api_pvr_clear_up_all();
        }
    }
    sys_data->ram_tms_en = val;
#endif

#ifdef FSC_SUPPORT
    msel =&miscset_sel10;
	val = osd_get_multisel_sel(msel);
    if(val)
    {
        sys_data->fsc_en = 1;
    }
    else
    {
	    sys_data->fsc_en = val;
    }
#endif

#ifdef FLASH_SOFTWARE_PROTECT_TEST
    msel =&miscset_sel11;
	val = osd_get_multisel_sel(msel);
	sys_data->flash_sp_test = val;
#endif

#ifdef PLSN_SUPPORT
    msel = &miscset_sel12;
    val = osd_get_multisel_sel(msel);
    sys_data->super_scan = val;
#endif

    sys_data_save(1);
}

static void check_front_end_is_s(POBJECT_HEAD p_obj)
{
if((board_get_frontend_type(0)!=FRONTEND_TYPE_S)&& (board_get_frontend_type(1)!=FRONTEND_TYPE_S))
{
    osd_set_attr(&miscset_item0, C_ATTR_INACTIVE);
    osd_set_attr(&miscset_txt0, C_ATTR_INACTIVE);
    osd_set_attr(&miscset_sel0, C_ATTR_INACTIVE);
    if((board_get_frontend_type(0)!=FRONTEND_TYPE_T)&&(board_get_frontend_type(1)!=FRONTEND_TYPE_T))
        {
            osd_set_attr(&miscset_item1, C_ATTR_INACTIVE);
            osd_set_attr(&miscset_txt1, C_ATTR_INACTIVE);
            osd_set_attr(&miscset_sel1, C_ATTR_INACTIVE);
            osd_track_object((POBJECT_HEAD )&miscset_item2,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item3,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item4,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item5,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item6,C_UPDATE_ALL);
            osd_set_container_focus((CONTAINER*)p_obj,3);
        }
    else
        {
            osd_track_object((POBJECT_HEAD)&miscset_item1,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item2,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item3,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item4,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item5,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD )&miscset_item6,C_UPDATE_ALL);
            osd_set_container_focus((CONTAINER*)p_obj,2);
        }
}
else
{
    if((board_get_frontend_type(0)!=FRONTEND_TYPE_T)&&(board_get_frontend_type(1)!=FRONTEND_TYPE_T))
    {
        osd_set_attr(&miscset_item1, C_ATTR_INACTIVE);
        osd_set_attr(&miscset_txt1, C_ATTR_INACTIVE);
        osd_set_attr(&miscset_sel1, C_ATTR_INACTIVE);
        osd_track_object((POBJECT_HEAD )&miscset_item0,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&miscset_item2,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&miscset_item3,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&miscset_item4,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&miscset_item5,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&miscset_item6,C_UPDATE_ALL);
        osd_set_container_focus((CONTAINER*)p_obj,1);
    }
}

/*
    #ifndef DVBS_SUPPORT
    OSD_SetAttr(&miscset_item0, C_ATTR_INACTIVE);
    OSD_SetAttr(&miscset_txt0, C_ATTR_INACTIVE);
    OSD_SetAttr(&miscset_sel0, C_ATTR_INACTIVE);
    OSD_TrackObject((POBJECT_HEAD)&miscset_item1,C_UPDATE_ALL);
    OSD_DrawObject((POBJECT_HEAD )&miscset_item2,C_UPDATE_ALL);
    OSD_DrawObject((POBJECT_HEAD )&miscset_item3,C_UPDATE_ALL);
    OSD_DrawObject((POBJECT_HEAD )&miscset_item4,C_UPDATE_ALL);
    OSD_DrawObject((POBJECT_HEAD )&miscset_item5,C_UPDATE_ALL);
    OSD_SetContainerFocus((CONTAINER*)pObj,2);
    #endif
*/
}
