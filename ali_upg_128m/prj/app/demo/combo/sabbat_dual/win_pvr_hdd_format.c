/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_hdd_format.c

*    Description: pvr record partition operate menu.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifndef DISK_MANAGER_SUPPORT

#ifdef DVR_PVR_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/unistd.h>


#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include <api/libmp/lib_mp.h>
#include "win_pvr_hdd_format.h"


static VACTION hddfmt_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT hddfmt_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION hddfmt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT hddfmt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION hddfmt_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT hddfmt_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define TXT_INF_SH_IDX        WSTL_BUTTON_01_HD//sharon WSTL_BUTTON_07_HD
#define TXT_BTN_SH_IDX        WSTL_BUTTON_01_HD
#define TXT_BTN_HL_IDX        WSTL_BUTTON_05_HD

#define CON_SH_IDX    WSTL_BUTTON_01_HD
#define CON_HL_IDX    WSTL_BUTTON_05_HD
#define CON_SL_IDX    WSTL_BUTTON_01_HD
#define CON_GRY_IDX    WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD


#define    W_L         248//384//110
#define    W_T         110//150//60
#ifdef SUPPORT_CAS_A
#define    W_W         712
#else
#define    W_W         692
#endif
#define    W_H         476//320

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXTN_L_OF      10
#define TXTN_W      280
#define TXTN_H        CON_H
#define TXTN_T_OF    ((CON_H - TXTN_H)/2)

#define TXTS_L_OF      (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W-300)
#define TXTS_H        CON_H
#define TXTS_T_OF    ((CON_H - TXTS_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#define LINE_W2        (LINE_W-80)

#define BTN_L         CON_L
#define BTN_T         (CON_T+(CON_H+CON_GAP)*7)
#define BTN_W         CON_W
#define BTN_H         CON_H

#define INFO_L      (W_L + 40)
#define INFO_T      (W_T + 30 + (CON_H+CON_GAP)*0)
#define INFO_H         40
#define INFO_V_GAP    4
#define INFO_H_GAP    50
#define INFO_N_W     214
#define INFO_C_W     240

#define TXT_L_OF      TXTN_L_OF
#define TXT_W          TXTN_W
#define TXT_H        TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF     TXTS_L_OF
#define SEL_W          TXTS_W
#define SEL_H        TXTS_H
#define SEL_T_OF    TXTS_T_OF

#define BLOCK_SIZE_UNIT_1K       1024
#define VOLUME_SELECT_ID         1
#define FORMAT_ID                2

#define FS_TYPE_FAT_OR_NTFS      1  // FAT or NTFS or others, just one of them
#define FS_TYPE_FAT_AND_NTFS     2  // currently, only support FAT/NTFS at the most

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT_INFO(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    2,2,2,1,1, l,t,w,h, TXT_BTN_SH_IDX,TXT_BTN_SH_IDX,TXT_BTN_SH_IDX,TXT_BTN_SH_IDX,   \
    hddfmt_btn_keymap,hddfmt_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    hddfmt_keymap,hddfmt_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    hddfmt_item_con_keymap,hddfmt_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h,TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line, ID,idu,idd,l,t,w,h,name_id,setstr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l+TXTN_L_OF,t+TXTN_T_OF,(TXTN_W-20),TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,NULL,1,1,1,1,1,l+(TXTS_L_OF-20),t+TXTS_T_OF,TXTS_W+20,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, l+LINE_L_OF, t+CON_H, LINE_W, LINE_H, 0, 0)

#define LDEF_TXT_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


LDEF_TXT_TITLE(&g_win_hddformat,hddfmt_title,NULL,0,0,0,0, display_strs[0])
LDEF_TXT_INFO(&g_win_hddformat,hddfmt_text,NULL,0,0,0,0, display_strs[1])
LDEF_LINE(NULL,hddfmt_line,NULL,0,0,0,0,0,0,0,0,0,0,NULL)


LDEF_MM_ITEM(g_win_hddformat,hddfmt_item_con1, &hddfmt_item_con2,hddfmt_item_txtname1,hddfmt_item_txtset1, \
     hddfmt_item_line1,1,2,2,CON_L,CON_T+(CON_H+CON_GAP)*6,CON_W,CON_H,RS_STORAGE_PARTITION,len_display_str)

LDEF_TXT_MENU_ITEM(g_win_hddformat,hddfmt_item_con2, NULL,hddfmt_item_txtname2,hddfmt_item_line2,2,1,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, RS_STORAGE_FORMAT)

LDEF_WIN(g_win_hddformat,&hddfmt_item_con1,W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/
static void win_hddfmt_draw_disk_infor(void);
/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION hddfmt_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    if(key == V_KEY_ENTER)
        act = VACT_ENTER;
    else
        act = VACT_PASS;

    return act;

}

static BOOL hddfmt_window_active;
static int hddfmt_cur_volume;

static void hddfmt_vol_init()
{
    UINT8 type;
    UINT8 sub_type;
    UINT8 partition;
    int max_volume;

    storage_get_device_info(STORAGE_TYPE_ALL);
    max_volume = storage_get_volume_count();
    storage_get_cur_pvr_sto_info(&type, &sub_type, &partition);
    hddfmt_cur_volume = storage_find_volume(type, sub_type, partition);
    if((max_volume >= 1) && (hddfmt_cur_volume < 0))
    {
        // cann't fint pvr partition, get the first partition
        hddfmt_cur_volume = 0;
    }
}

static void hddfmt_set_vol_string(PTEXT_FIELD ptxt)
{
    char str[32];

    str[0] = 0;
    if(hddfmt_cur_volume>=0)
        storage_index_to_osd_string(hddfmt_cur_volume, str, 32);
    osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
}

static BOOL hddfmt_get_next_vol(int offset)
{
    int max_volume = storage_get_volume_count();

    if(max_volume == 0)
        return FALSE;
    hddfmt_cur_volume += offset;
    if(hddfmt_cur_volume >= max_volume)
        hddfmt_cur_volume = 0;
    if(hddfmt_cur_volume < 0)
         hddfmt_cur_volume = max_volume-1;
    return TRUE;
}

static void hddfmt_start()
{
    UINT8 back_saved = 0;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    UINT8 mode = 0;
    struct dvr_hdd_info hdd_info;
    char volume_name[48] = {0};
    char osd_name[48] = {0};
    char hint[64] = {0};
    BOOL ret = FALSE;

    char disk_mode[8][16];
    UINT8 disk_format_mode_num = 0;

    disk_format_mode_num = get_fsystem_installed(disk_mode, 8);
    //sprintf(temp, "%s", disk_mode[input]);

    storage_index_to_osd_string((UINT8)hddfmt_cur_volume,osd_name, 48);
    snprintf(hint, 64,"Are you sure to format the %s?", osd_name);

    if(FS_TYPE_FAT_OR_NTFS == disk_format_mode_num)
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_btnstr_ext(0, disk_mode[0]);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
    }
    else if(FS_TYPE_FAT_AND_NTFS == disk_format_mode_num)
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNOCANCLE);
        win_compopup_set_btnstr_ext(0, disk_mode[0]);
        win_compopup_set_btnstr_ext(1, disk_mode[1]);
        win_compopup_set_default_choice(WIN_POP_CHOICE_CANCEL);
    }
    win_compopup_set_msg(hint, NULL, 0);
    choice = win_compopup_open_ext(&back_saved);

    if(((FS_TYPE_FAT_OR_NTFS == disk_format_mode_num)&& (WIN_POP_CHOICE_YES==choice)) ||
       ((FS_TYPE_FAT_AND_NTFS==disk_format_mode_num)&&((WIN_POP_CHOICE_YES==choice)||(WIN_POP_CHOICE_NO==choice))))
    {
        if(choice == WIN_POP_CHOICE_YES)
            mode = 1;
        else
            mode = 2;

        storage_index2volume((UINT8)hddfmt_cur_volume,volume_name, 48);
        storage_add_parent_dir(volume_name, 48);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(GET_MID_L(286), GET_MID_T(130), 286, 130);
        win_compopup_set_msg_ext("formating wait!",NULL,0);
        win_compopup_open_ext(&back_saved);
        ret = pvr_format_multi_hdd(mode, volume_name);
        win_compopup_smsg_restoreback();

        if(!ret)
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            snprintf(hint, 64,"Format %s error?", osd_name);
            win_compopup_set_msg(hint, NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_CANCEL);
            win_compopup_open_ext(&back_saved);
            return;
        }

        if(0==STRCMP((char *)pvr_get_mout_prefix(), volume_name))
        {
            if(pvr_get_hdd_info(&hdd_info))
            {
                api_pvr_check_level(&hdd_info);
                ap_clear_all_message();//or usb reflash msg maybe send fail
                pvr_evnt_callback(0, PVR_HDD_FORMATTED);
            }
        }
        win_hddfmt_draw_disk_infor();
    }
}

static PRESULT hddfmt_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);

        if(unact == VACT_ENTER)
        {
            hddfmt_start();
        }
        break;
    }

    return ret;
}

static VACTION hddfmt_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT hddfmt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 title_id;
    POBJECT_HEAD next_obj;

    switch(event)
    {
    case EVN_PRE_OPEN:
        if(hddfmt_window_active)
        {
            hddfmt_window_active= TRUE;
            title_id = RS_STORAGE_FORMAT;
            osd_set_container_next_obj(&g_win_hddformat, (POBJECT_HEAD)&hddfmt_item_con1);
        }
        else
        {
            title_id = RS_HDD_NFORMATION;
            hddfmt_window_active = FALSE;
            osd_set_container_next_obj(&g_win_hddformat, NULL);
        }
        hddfmt_vol_init();
        hddfmt_set_vol_string(&hddfmt_item_txtset1);

        wincom_open_title(p_obj,title_id,0);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_POST_DRAW:
        win_hddfmt_draw_disk_infor();
        break;
    case EVN_MSG_GOT:
        if((param1 == CTRL_MSG_SUBTYPE_CMD_STO) && (param2 == USB_STATUS_OVER))
        {
            //storage_dev_mount_hint(1);
            hddfmt_vol_init();
            if(storage_get_volume_count() == 0)
            {
                ret = PROC_LEAVE;
            }
            else if(hddfmt_window_active)
            {
                hddfmt_set_vol_string(&hddfmt_item_txtset1);
                osd_track_object((POBJECT_HEAD)&g_win_hddformat, C_UPDATE_ALL);
                win_hddfmt_draw_disk_infor();
            }
        }
        break;

    }

    return ret;
}

static void cal_size(UINT32 size, char *str, int str_size)
{
    UINT32 dsize,fsize;
    dsize = size / 1024; //**. GB
    fsize = (size%1024) * 1000 / 1024; //.** GB
    snprintf(str,str_size,"%3d.%03d GB",dsize,fsize);
}

static void win_hddfmt_draw_one_infor(UINT8 index, char *name, char *data)
{
    TEXT_FIELD* txt, *info,*line;
    txt = &hddfmt_title;
    info = &hddfmt_text;
    line = &hddfmt_line;
    osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*index, INFO_N_W, INFO_H);
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)name);
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
    osd_set_rect(&info->head.frame, INFO_L+(INFO_N_W+INFO_H_GAP),INFO_T+(INFO_H+INFO_V_GAP)*index,INFO_C_W,INFO_H);
    osd_set_text_field_content(info, STRING_ANSI, (UINT32)data);
    osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);

    osd_set_rect(&line->head.frame,INFO_L-5,INFO_T + (INFO_H + INFO_V_GAP)*index + INFO_H, LINE_W2,4);
    osd_draw_object((POBJECT_HEAD)line, C_UPDATE_ALL);
}

static void win_hddfmt_draw_total_disk_infor()
{
    UINT32 total_size;
    UINT32 free_size;
    char str[32];
    struct statvfs stfs;
    char volume_name[48];
    UINT8 index = 0;

    if(hddfmt_cur_volume<0)
        return;
    storage_index2volume((UINT8)hddfmt_cur_volume,volume_name, 48);
    storage_add_parent_dir(volume_name, 48);
    fs_statvfs(volume_name, &stfs);

    if(stfs.f_frsize >=BLOCK_SIZE_UNIT_1K)
    {
           total_size = stfs.f_frsize / BLOCK_SIZE_UNIT_1K * stfs.f_blocks;
        free_size = stfs.f_frsize / BLOCK_SIZE_UNIT_1K * stfs.f_bfree;
    }
    else
    {
        total_size = ( stfs.f_frsize*2) / BLOCK_SIZE_UNIT_1K * stfs.f_blocks /2;
           free_size = (stfs.f_frsize*2) / BLOCK_SIZE_UNIT_1K * stfs.f_bfree /2;
    }
    win_hddfmt_draw_one_infor(index++,"Volume", stfs.f_volume_name);
    cal_size(total_size/BLOCK_SIZE_UNIT_1K, str, 32);
    win_hddfmt_draw_one_infor(index++,"Total Size", str);
    cal_size(free_size/BLOCK_SIZE_UNIT_1K, str, 32);
    win_hddfmt_draw_one_infor(index++,"Free Size", str);
    cal_size((total_size-free_size)/BLOCK_SIZE_UNIT_1K, str, 32);
    win_hddfmt_draw_one_infor(index++,"Used size", str);
    win_hddfmt_draw_one_infor(index++,"File System", stfs.f_fsh_name);
}

static void win_hddfmt_draw_pvr_disk_infor()
{
    UINT8 i;
    UINT32 size_all[8];
    char show_str[8][24];
    char volume_name[48];
    static UINT8 names[6][20] =
    {
        "Total",
        "Free",
        "Used",
        "Time shift",
        "Rec Free",
        "File System",
    };

    struct dvr_hdd_info hdd_info;
    pvr_get_hdd_info(&hdd_info);
    size_all[0] = hdd_info.total_size;
    size_all[1] = hdd_info.free_size;
    size_all[2] = hdd_info.total_size - hdd_info.free_size;
    size_all[3] = hdd_info.tms_size;
    size_all[4] = hdd_info.rec_size;
    for(i = 0; i < 5; i++)
    {
        cal_size(size_all[i]/1024, show_str[i], 24);
    }
    snprintf(show_str[5], 24,"%s", ((hdd_info.type == 1) ? "FAT" : ((hdd_info.type == 2) ? "NTFS" : "Unknown")));
    for(i = 0; i < 6; i++)
    {
        win_hddfmt_draw_one_infor(i,names[i], show_str[i]);
    }
}

static void win_hddfmt_draw_disk_infor()
{
    if(hddfmt_window_active)
        win_hddfmt_draw_total_disk_infor();
    else
        win_hddfmt_draw_pvr_disk_infor();
}

static VACTION hddfmt_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

void hddfmt_vol_enter_key(POBJECT_HEAD p_obj,UINT8 id)
{
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;

    rect.u_left = CON_L + TXTS_L_OF - 100;
    rect.u_width = TXTS_W+80;
    rect.u_height = 300;
    param.selecttype = POP_LIST_SINGLESELECT;

    if(storage_get_volume_count()<=1 || hddfmt_cur_volume < 0)
        return;

    rect.u_top = CON_T + (CON_H + CON_GAP) * 2;
    param.cur = hddfmt_cur_volume;
    hddfmt_cur_volume = win_com_open_sub_list(POP_LIST_TYPE_VOLUMELIST,&rect,&param);
    hddfmt_set_vol_string(&hddfmt_item_txtset1);
    osd_track_object(p_obj, C_UPDATE_ALL);
    win_hddfmt_draw_disk_infor();
}

static PRESULT hddfmt_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    unact,input;
    UINT8    id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if(VOLUME_SELECT_ID == id)
        {
            if(unact == VACT_DECREASE)
            {
                if(!hddfmt_get_next_vol(-1))
                {
                    ret = PROC_LEAVE;
                    break;
                }
                hddfmt_set_vol_string(&hddfmt_item_txtset1);
                osd_track_object(p_obj,C_UPDATE_ALL);
                win_hddfmt_draw_disk_infor();
            }
            else if(unact == VACT_INCREASE)
            {
                if(!hddfmt_get_next_vol(1))
                {
                    ret = PROC_LEAVE;
                    break;
                }
                hddfmt_set_vol_string(&hddfmt_item_txtset1);
                osd_track_object(p_obj,C_UPDATE_ALL);
                win_hddfmt_draw_disk_infor();
            }
            else if(unact == VACT_ENTER)
            {
                hddfmt_vol_enter_key(p_obj,id);
            }
        }
        else if(FORMAT_ID == id)
        {
            if(hddfmt_cur_volume >= 0)
                hddfmt_start();
        }
        ret = PROC_LOOP;
        break;
    }
    return ret;

}

void hddfmt_win_set_type(BOOL hddfmt_win_active)
{
    hddfmt_window_active = hddfmt_win_active;
}
#endif//DVR_PVR_SUPPORT
#endif /* DISK_MANAGER_SUPPORT */
