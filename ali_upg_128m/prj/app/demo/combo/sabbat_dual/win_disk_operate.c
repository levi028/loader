/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_disk_operate.c
*
*    Description: the ui of disk operate
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
#include <api/libpvr/lib_pvr.h>

#ifdef DISK_MANAGER_SUPPORT
#include <api/libmp/pe.h>
#include <api/libfs2/fs_main.h>
#include "disk_manager.h"
#endif

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "control.h"
#include "win_disk_operate.h"
#include "win_disk_info.h"

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
#define WIN_SH_IDX    WSTL_POP_WIN_01_HD//WSTL_POP_WIN_01
#define WIN_HL_IDX    WSTL_POP_WIN_01_HD//WSTL_POP_WIN_01
#define WIN_SL_IDX    WSTL_POP_WIN_01_HD//WSTL_POP_WIN_01
#define WIN_GRY_IDX  WSTL_POP_WIN_01_HD//WSTL_POP_WIN_01

#define CON_SH_IDX   WSTL_TEXT_04_HD//WSTL_TEXT_04
#define CON_HL_IDX   WSTL_BUTTON_05_HD//WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_BUTTON_01_HD//WSTL_BUTTON_01
#define CON_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define TXT_SH_IDX   WSTL_TEXT_04_HD//WSTL_TEXT_04
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define EDT_SH_IDX   WSTL_TEXT_04_HD//WSTL_TEXT_04
#define EDT_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_04
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define SEL_SH_IDX   WSTL_TEXT_04_HD//WSTL_TEXT_04
#define SEL_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_04
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_01_FG
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_BUTTON_07

#define POPUPSH_IDX WSTL_BUTTON_POP_SH_HD//WSTL_BUTTON_POP_SH
#define POPUPHL_IDX WSTL_BUTTON_POP_HL_HD//WSTL_BUTTON_POP_HL

#ifndef SD_UI
#define W_W     540//500//350
#define W_H     230//150
#define W_L     (GET_MID_L(W_W) + 60)
#define W_T     GET_MID_T(W_H)

#define TITLE_L_OF  20
#define TITLE_L     (W_L + TITLE_L_OF)
#define TITLE_T     (W_T + 30)
#define TITLE_W     (W_W - TITLE_L_OF*2)
#define TITLE_H     40//30

#define CON_L_OF    TITLE_L_OF//sharon 6
#define CON_L       (W_L + CON_L_OF)
#define CON_T       (TITLE_T +  TITLE_H + 10)
#define CON_W       (W_W - CON_L_OF*2)
#define CON_H       36//28
#define CON_GAP     0

#define TXT_L_OF    CON_L_OF//12
#define TXT_W       ((CON_W - 20) / 3 - 25)
#define TXT_H       36//sharon 30
#define TXT_T_OF    0

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       ((CON_W - 20) - TXT_W)
#define SEL_H       36//sharon 30
#define SEL_T_OF    TXT_T_OF

#define BTN_GAP     40
#define BTN_H       38
#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    80
#define BTN_W       110//80
#else
#define W_W     390//500//350
#define W_H     150//150
#define W_L     (GET_MID_L(W_W) + 40)
#define W_T     (GET_MID_T(W_H) - 30)

#define TITLE_L_OF  10
#define TITLE_L     (W_L + TITLE_L_OF)
#define TITLE_T     (W_T + 10)
#define TITLE_W     (W_W - TITLE_L_OF*2)
#define TITLE_H     30//30

#define CON_L_OF    TITLE_L_OF//sharon 6
#define CON_L       (W_L + CON_L_OF)
#define CON_T       (TITLE_T +  TITLE_H + 10)
#define CON_W       (W_W - CON_L_OF*2)
#define CON_H       28//28
#define CON_GAP     0

#define TXT_L_OF    CON_L_OF//12
#define TXT_W       ((CON_W - 20) / 3 - 20)
#define TXT_H       28//sharon 30
#define TXT_T_OF    0

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       ((CON_W - 20) - TXT_W)
#define SEL_H       28//sharon 30
#define SEL_T_OF    TXT_T_OF

#define BTN_GAP     40
#define BTN_H       30
#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#define BTN_T_OF    50
#define BTN_W       80//80

#endif

#ifdef DISK_MANAGER_SUPPORT
#define mi_b (1024*1024ULL)      /* MB: Mebibyte */
#define gi_b (1024*1024*1024ULL) /* GB: Gibibyte*/
#endif

static VACTION disk_operate_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT disk_operate_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION disk_operate_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT disk_operate_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION disk_operate_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT disk_operate_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    disk_operate_item_keymap,disk_operate_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str0)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str0)

#define LDEF_TXT_EXT(root,var_txt,nxt_obj,l,t,w,h,res_id, str)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)       \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    disk_operate_btn_keymap,disk_operate_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt, var_value,ID,idu,idd,\
                        l,t,w,h,str0, str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_value,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str0)       \
    LDEF_TXT_EXT(&var_con,var_value,NULL, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,0,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    disk_operate_keymap,disk_operate_callback,  \
    nxt_obj, focus_id,0)


LDEF_TITLE(g_win_disk_operate, disk_operate_title, &disk_operate_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, display_strs[2])

LDEF_MENU_ITEM_TXT(g_win_disk_operate,disk_operate_item1,&disk_operate_btntxt_ok,disk_operate_txt1,disk_operate_txt2, \
            1, 3, 2, CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, display_strs[0], display_strs[1])

LDEF_TXT_BTN(g_win_disk_operate, disk_operate_btntxt_ok,&disk_operate_btntxt_cancel,2,3,3,1,1,  \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_OK)

LDEF_TXT_BTN(g_win_disk_operate, disk_operate_btntxt_cancel, NULL,3,2,2,1,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_disk_operate, &disk_operate_title,W_L, W_T,W_W,W_H,1)


#define MENU_MULTI_SEL_ID   1
#define BTN_OK_ID           2
#define BTN_CANCEL_ID       3

#define WIN_DISK_PARTITION_NUM_2 2


/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
#ifdef DISK_MANAGER_SUPPORT
typedef struct _removable_dsik_info
{
    UINT32 disk_type;
    UINT32 disk_idx;
    UINT64 disk_size;
} removable_dsik_info;

/* for disk remove */
static UINT8 disk_select= 0;
static UINT8 disk_num= 0;
static removable_dsik_info r_disk_info[MAX_DISK_NUM];
#endif

static UINT8 operate_type= 0;
UINT8 disk_operate_opened= 0;
static UINT8 disk_operate_ok= 0;           /* 0: cancel; 1: ok*/

/* for partition format */
static UINT8 mode_select= 0;
static UINT8 disk_format_mode_num = 2;
static char disk_mode[8][16] = {"FAT", "NTFS"};

/* for DVR Set */
static UINT8 dvr_select_type= 0;
static UINT8 dvr_type_num = 3;
static char dvr_type[3][16] = {"REC & TMS", "Record", "Timeshift"};

/* for partition copy */
static UINT8 partition_select= 0; //refer to copy dest partition
static UINT8 partition_num_local = 0;
static char disk_partition[64][16];


/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
UINT8 win_disk_mode_get_num(void)
{
    return disk_format_mode_num;
}

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
UINT8 is_disk_operate_opened(void)
{
    return disk_operate_opened;
}
#endif
void win_disk_mode_get_str(UINT8 idx, char *str, int str_size)
{
    strncpy(str, disk_mode[idx], str_size-1);
    str[str_size-1] = 0;
}

UINT8 win_dvr_type_get_num(void)
{
    return dvr_type_num;
}

void win_dvr_type_get_str(UINT8 idx, char *str, int str_size)
{
    strncpy(str, dvr_type[idx], str_size-1);
    str[str_size-1] = 0;
}

UINT8 win_dvr_type_get_mode(UINT8 idx)
{
    UINT8 mode = 0xFF;

    switch (idx)
    {
        case 0:
            mode = PVR_REC_AND_TMS_DISK;
            break;
        case 1:
            mode = PVR_REC_ONLY_DISK;
            break;
        case 2:
            mode = PVR_TMS_ONLY_DISK;
            break;
        default:
            break;
    }
    return mode;
}


UINT8 win_partition_get_num(void)
{
    return partition_num_local;
}

void win_partition_get_str(UINT8 idx, char *str, int str_size)
{
    strncpy(str, disk_partition[idx], str_size-1);
    str[str_size-1]= 0;
}

#ifdef DISK_MANAGER_SUPPORT
UINT8 win_removable_disk_get_num(void)
{
    return disk_num;
}

void win_removable_disk_get_name_str(UINT8 idx, char *str)
{
    UINT8 size = 8;

    if (idx < disk_num)
    {
        snprintf(str, size, "%s%c",
                (MNT_TYPE_USB == r_disk_info[idx].disk_type) ? "ud" :
        ((MNT_TYPE_SD == r_disk_info[idx].disk_type) ? "sd" : "sh"),
                (INT8)(r_disk_info[idx].disk_idx + 'a'));
    }
    else
    {
        //STRCPY(str, "");
        strncpy(str, "", size - 1);
    }
}

void win_removable_disk_get_show_str(UINT8 idx, char *str)
{
    removable_dsik_info *p_disk = &r_disk_info[idx];
    UINT32 size = 32;

    if(NULL == str)
    {
        return;
    }

    if (idx < disk_num)
    {
        if (p_disk->disk_size >= gi_b)
        {
            snprintf(str, size,"%s %c - %lu GB",
                    (MNT_TYPE_USB == p_disk->disk_type) ? "USB Disk" :
            ((MNT_TYPE_SD == p_disk->disk_type) ? "SD Card" : "SATA Disk"),
                    (INT8)('A'+p_disk->disk_idx), (UINT32)(p_disk->disk_size/gi_b));
        }
        else
        {
            snprintf(str, size, "%s %c - %lu MB",
                    (MNT_TYPE_USB == p_disk->disk_type) ? "USB Disk" :
            ((MNT_TYPE_SD == p_disk->disk_type) ? "SD Card" : "SATA Disk"),
                    (INT8)('A'+p_disk->disk_idx), (UINT32)(p_disk->disk_size/mi_b));
        }
    }
    else
    {
        strncpy(str, "", size-1);
    }
}

static int win_removable_disk_info_init(void)
{
    int dm_ret= 0;
    int i= 0;
    int disk_ns = 0;
    UINT64 disk_size= 0;

    for (i = 0; i < MAX_USB_DISK_NUM; ++i)
    {
        dm_ret = dm_ioctl(DM_CMD2_GET_DISK_SIZE, DM_CMD_PARAM(MNT_TYPE_USB, i, 0),
                          &disk_size, sizeof(UINT64));
        if (0 == dm_ret)
        {
            r_disk_info[disk_ns].disk_type = MNT_TYPE_USB;
            r_disk_info[disk_ns].disk_idx = i;
            r_disk_info[disk_ns].disk_size = disk_size;
            disk_ns++;
        }
    }

    for (i = 0; i < MAX_SD_DISK_NUM; ++i)
    {
        dm_ret = dm_ioctl(DM_CMD2_GET_DISK_SIZE, DM_CMD_PARAM(MNT_TYPE_SD, i, 0),
                          &disk_size, sizeof(UINT64));
        if (0 == dm_ret)
        {
            r_disk_info[disk_ns].disk_type = MNT_TYPE_SD;
            r_disk_info[disk_ns].disk_idx = i;
            r_disk_info[disk_ns].disk_size = disk_size;
            disk_ns++;
        }
    }

    for (i = 0; i < MAX_SATA_DISK_NUM; ++i)
    {
        dm_ret = dm_ioctl(DM_CMD2_GET_DISK_SIZE, DM_CMD_PARAM(MNT_TYPE_SATA, i, 0),
                          &disk_size, sizeof(UINT64));
        if (0 == dm_ret)
        {
            r_disk_info[disk_ns].disk_type = MNT_TYPE_SATA;
            r_disk_info[disk_ns].disk_idx = i;
            r_disk_info[disk_ns].disk_size = disk_size;
            disk_ns++;
        }
    }

    disk_num = disk_ns;
    return disk_ns;
}
#endif /* DISK_MANAGER_SUPPORT */

static void win_disk_info_set_string(UINT8 type, UINT8 input)
{
    UINT8 temp[32] = {0};

    switch(type)
    {
        case OPERATE_TYPE_PART_FORMAT:
            win_disk_mode_get_str(input, (char *)temp, 32);
            break;
        case OPERATE_TYPE_DVR_SET:
            win_dvr_type_get_str(input, (char *)temp, 32);
            break;
        case OPERATE_TYPE_PART_COPY:
            win_partition_get_str(input, (char *)temp, 32);
            break;
#ifdef DISK_MANAGER_SUPPORT
        case OPERATE_TYPE_DISK_REMOVE:
            win_removable_disk_get_show_str(input, (char *)temp);
            break;
#endif
        default:
            break;
    }
    com_asc_str2uni(temp, display_strs[1]);
}

static VACTION disk_operate_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
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

static PRESULT disk_operate_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;
    UINT8   id = osd_get_obj_id(pobj);
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT8 *cur_select = NULL;
    UINT8 cur_mode_num = 0;
    COM_POP_LIST_TYPE_T type = 0;

    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    switch(event)
    {
        case EVN_PRE_CHANGE:
            break;
        case EVN_POST_CHANGE:
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16) ;
            if(id != MENU_MULTI_SEL_ID)
            {
                break;
            }
            switch(operate_type)
            {
                case OPERATE_TYPE_PART_FORMAT:
                    type = POP_LIST_DISK_MODE;
                    cur_select = &mode_select;
                    cur_mode_num = disk_format_mode_num;
                    break;
                case OPERATE_TYPE_DVR_SET:
                    type = POP_LIST_DVR_TYPE;
                    cur_select = &dvr_select_type;
                    cur_mode_num = dvr_type_num;
                    break;
                case OPERATE_TYPE_PART_COPY:
                    type = POP_LIST_PARTITION;
                    cur_select = &partition_select;
                    cur_mode_num = partition_num_local;
                    break;
#ifdef DISK_MANAGER_SUPPORT
                case OPERATE_TYPE_DISK_REMOVE:
                    type = POP_LIST_REMOVABLE_DISK_INFO;
                    cur_select = &disk_select;
                    cur_mode_num = disk_num;
                    break;
#endif
                default:
                    break;
            }
            if(!cur_select)
        {
                break;
        }
            if(VACT_ENTER == unact)
            {
                #ifndef SD_UI
                rect.u_left = CON_L + SEL_L_OF;
                rect.u_width = SEL_W;
                rect.u_height = 200;//150;
                param.selecttype = POP_LIST_SINGLESELECT;
                param.cur = *cur_select;
                rect.u_top=CON_T + SEL_T_OF + SEL_H;
                #else
                rect.u_left = CON_L + SEL_L_OF;
                rect.u_width = SEL_W;
                rect.u_height = 180;//150;
                param.selecttype = POP_LIST_SINGLESELECT;
                param.cur = *cur_select;
                rect.u_top=CON_T + SEL_T_OF + SEL_H;
                #endif
                *cur_select = win_com_open_sub_list(type,&rect,&param);
                win_disk_info_set_string(operate_type, *cur_select);
                osd_track_object(pobj,C_UPDATE_ALL);
            }
            else if((VACT_DECREASE == unact) || (VACT_INCREASE == unact))
            {
                if(VACT_DECREASE == unact)
        {
                    *cur_select = (*cur_select + cur_mode_num - 1) % cur_mode_num;
        }
                else
        {
                    *cur_select = (*cur_select + 1) % cur_mode_num;
        }
                win_disk_info_set_string(operate_type, *cur_select);
                osd_track_object(pobj,C_UPDATE_ALL);
            }
            break;
        default:
            break;
    }

    return ret;
}

static VACTION disk_operate_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(V_KEY_ENTER == key)
    {
        act = VACT_ENTER;
     }

    return act;
}

static PRESULT disk_operate_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   bid= 0;

    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(BTN_OK_ID == bid)
        {
            disk_operate_ok = 1;
            ret = PROC_LEAVE;
        }
        else if(BTN_CANCEL_ID == bid)
        {
            disk_operate_ok = 0;
            ret = PROC_LEAVE;
        }

        break;
    default:
        break;
    }

    return ret;
}

static VACTION disk_operate_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
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

static PRESULT disk_operate_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    char temp[30] = {0};
    UINT8 *cur_select = NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        switch(operate_type)
        {
            case OPERATE_TYPE_PART_FORMAT:
                strncpy(temp, "Disk Mode", (30-1));
                cur_select = &mode_select;
                break;
            case OPERATE_TYPE_DVR_SET:
                strncpy(temp, "DVR Type", (30-1));
                cur_select = &dvr_select_type;
                break;
            case OPERATE_TYPE_PART_COPY:
                strncpy(temp, "Dest Partition", (30-1));
                cur_select = &partition_select;
                break;
#ifdef DISK_MANAGER_SUPPORT
            case OPERATE_TYPE_DISK_REMOVE:
                strncpy(temp, "Remove", (30-1));
                cur_select = &disk_select;
                break;
#endif
            default:
                break;
        }
        com_asc_str2uni((UINT8 *)temp, display_strs[0]);
        if(cur_select)
    {
            win_disk_info_set_string(operate_type, *cur_select);
    }
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        if (NULL == menu_stack_get_top())
    {
            osd_clear_object((POBJECT_HEAD)&g_win_disk_operate, C_UPDATE_ALL);
    }
        break;
    default:
        break;
    }

    return ret;
}

static int _partition_compare(const void *a, const void *b)
{
    const char *p1 = (const char *)a;
    const char *p2 = (const char *)b;

    return STRCMP(p1, p2);
}

UINT32 win_disk_operate_open(UINT8 op_type, UINT8 *operate_result)
{
    CONTAINER *win = NULL;
    PRESULT ret = PROC_LOOP;
    UINT32 hkey= 0;
    TEXT_FIELD *  __MAYBE_UNUSED__ txt = NULL;
    char str[30] = {0};
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    char exit_flag = 0;
    BOOL old_value= FALSE;

    if(NULL == operate_result)
    {
        return 0;
    }

    operate_type = op_type;
    disk_operate_ok = 0;

    txt = &disk_operate_title;
    switch(op_type)
    {
        case OPERATE_TYPE_PART_FORMAT:
            strncpy(str, "Format", (30-1));
            mode_select = 0;
            disk_format_mode_num = get_fsystem_installed(disk_mode, 8);
            if(!disk_format_mode_num)
        {
                return 0xFF;
        }
            break;
        case OPERATE_TYPE_DVR_SET:
            strncpy(str, "DVR Set", (30-1));
            dvr_select_type = 0;
#ifdef PVR3_SUPPORT
            dvr_type_num = 3;
#else
            dvr_type_num = 1;//3;
#endif
            break;
        case OPERATE_TYPE_PART_COPY:
            //partition_select = 0;
            partition_num_local = init_valid_partition(disk_partition, 64);
            if(!partition_num_local)
            {
                return 0xFF;
            }
            if(partition_num_local > WIN_DISK_PARTITION_NUM_2)
            {
                qsort(disk_partition, partition_num_local, 16, _partition_compare);
            }
            if(partition_select >= partition_num_local)
            {
                partition_select = 0;
            }
            pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
            //sprintf(str, "%s", (rec_disk[5] == 'h') ? "Copy: HDD to USB" : "Copy: USB to HDD");
            //sprintf(str, "%s", "Copy to Other Device");
            strncpy(str, "Copy to Other Device", 29);
            break;
#ifdef DISK_MANAGER_SUPPORT
        case OPERATE_TYPE_DISK_REMOVE:
            #ifdef SUPPORT_CAS_A
                /* clear msg first */
                api_c1700a_osd_close_cas_msg();
            #endif
            #ifdef _MHEG5_SUPPORT_
                mheg_running_app_kill(0);
            #endif
            disk_select = 0;
            disk_num = win_removable_disk_info_init();
            strncpy(str, "Disk Remove", (30-1));
            break;
#endif
        default:
            exit_flag = 1;
            break;
            //return 0; // Unsupported operation
    }
    if(1 == exit_flag)
    {
            return 0;
    }

    com_asc_str2uni((UINT8 *)str, display_strs[2]); /* set title */

    win = &g_win_disk_operate;
    osd_set_container_focus(win, MENU_MULTI_SEL_ID);

    osd_obj_open((POBJECT_HEAD)win, 0);

    disk_operate_opened = 1;

    old_value = ap_enable_key_task_get_key(TRUE);
    while((PROC_LEAVE!=ret) && (disk_operate_opened))
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    ap_enable_key_task_get_key(old_value);

    switch(op_type)
    {
        case OPERATE_TYPE_PART_FORMAT:
            *(UINT8 *)operate_result = mode_select;
            break;
        case OPERATE_TYPE_DVR_SET:
            *(UINT8 *)operate_result = dvr_select_type;
            break;
        case OPERATE_TYPE_PART_COPY:
            *(UINT8 *)operate_result = partition_select;
            break;
#ifdef DISK_MANAGER_SUPPORT
        case OPERATE_TYPE_DISK_REMOVE:
            *(UINT8 *)operate_result = disk_select;
            break;
#endif
        default:
            break;
    }

    disk_operate_opened = 0;

    return disk_operate_ok;
}

PRESULT comlist_disk_mode_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cbret = PROC_PASS;
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT16  __MAYBE_UNUSED__ cnt= 0;
    OBJLIST *ol = NULL;
    char  str[50] = {0};

    ol = (OBJLIST*)pobj;
    cnt = osd_get_obj_list_count(ol);

    if(EVN_PRE_DRAW==event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;(i<ol->w_dep) && (i+wtop<ol->w_count);i++)
        {
        //ComUniStrCopyChar((UINT8*)&str[p_node.ca_mode], p_node.service_name);
#ifndef _BUILD_OTA_E_
        win_disk_mode_get_str(i + wtop, str, 50);
#endif
        win_comlist_set_str(i + wtop, str, NULL, 0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }

    return cbret;
}

PRESULT comlist_dvr_type_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cbret = PROC_PASS;
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT16  __MAYBE_UNUSED__ cnt= 0;
    OBJLIST *ol = NULL;
    char  str[50] = {0};

    ol = (OBJLIST*)pobj;
    cnt = osd_get_obj_list_count(ol);
  
    if(EVN_PRE_DRAW==event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;(i<ol->w_dep) && (i+wtop<ol->w_count);i++)
        {
        //ComUniStrCopyChar((UINT8*)&str[p_node.ca_mode], p_node.service_name);
#ifndef _BUILD_OTA_E_
        win_dvr_type_get_str(i + wtop, str, 50);
#endif
        win_comlist_set_str(i + wtop, str, NULL, 0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }

    return cbret;
}

PRESULT comlist_partition_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cbret = PROC_PASS;
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT16  __MAYBE_UNUSED__ cnt= 0;
    OBJLIST *ol = NULL;
    char  str[50] = {0};

    ol = (OBJLIST*)pobj;
    cnt = osd_get_obj_list_count(ol);

    if(EVN_PRE_DRAW==event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;(i<ol->w_dep) && (i+wtop<ol->w_count);i++)
        {
        //ComUniStrCopyChar((UINT8*)&str[p_node.ca_mode], p_node.service_name);
#ifndef _BUILD_OTA_E_
        win_partition_get_str(i + wtop, str, 50);
#endif
        win_comlist_set_str(i + wtop, str, NULL, 0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
     }

    return cbret;
}

#ifdef DISK_MANAGER_SUPPORT
PRESULT comlist_disk_info_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cbret = PROC_PASS;
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT16  __MAYBE_UNUSED__ cnt= 0;
    OBJLIST *ol = NULL;
    char  str[50] = {0};

    ol = (OBJLIST*)pobj;
    cnt = osd_get_obj_list_count(ol);

    if(EVN_PRE_DRAW==event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;(i<ol->w_dep) && (i+wtop<ol->w_count);i++)
        {
#ifndef _BUILD_OTA_E_
        win_removable_disk_get_show_str(i + wtop, str);
#endif
        win_comlist_set_str(i + wtop, str, NULL, 0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }

    return cbret;
}

#endif /* DISK_MANAGER_SUPPORT */

