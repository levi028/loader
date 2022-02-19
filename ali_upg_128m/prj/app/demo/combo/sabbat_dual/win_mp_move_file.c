   /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mp_move_file.c
*
*    Description: for Mediaplayer to move file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "media_control.h"
#include "win_mp_move_file.h"
#include "win_copying.h"
#include "ctrl_key_proc.h"
/*******************************************************************************
*    Macro definition
*******************************************************************************/
#ifndef SD_UI
#define MOVE_W_L    220//270
#define MOVE_W_T    70
#define MOVE_W_W    550//450
#define MOVE_W_H    484//492
#define MV_GAP_L    18
#define MV_GAP_T    4
#define MV_TITLE_L    (MOVE_W_L+36)
#define MV_TITLE_T    (MOVE_W_T + 14)
#define MV_TITLE_W    (MOVE_W_W-60)
#define MV_TITLE_H    40
#define MV_DEVCON_L (MOVE_W_L + MV_GAP_L +10)
#define MV_DEVCON_T (MOVE_W_T + 70)//(TITLE_T + TITLE_H)
#define MV_DEVCON_W (MOVE_W_W - MV_GAP_L*2 -20)//536
#define MV_DEVCON_H 36
#define MV_DEVTXT_L    (MV_DEVCON_L + 86)
#define MV_DEVTXT_T    (MV_DEVCON_T)
#define MV_DEVTXT_W    (MV_DEVCON_W - 86 * 2)
#define MV_DEVTXT_H    36
#define MV_LST_L     (MOVE_W_L + MV_GAP_L)
#define MV_LST_T     (MV_DEVCON_T + MV_DEVCON_H + 4)//(MOVE_W_T + 70)
#define MV_LST_W    (MOVE_W_W - MV_GAP_L * 2)
#define MV_LST_H     (MOVE_W_H - 70*2 - MV_DEVCON_H - 40)
#define MV_ITEM_CNT    6//7
#define MV_ITEM_L    (MV_LST_L + 10)
#define MV_ITEM_T    (MV_LST_T + 0)
#define MV_ITEM_W    (MV_LST_W-30)
#define MV_ITEM_H    40
#define LINE_L_OF    0
#define LINE_T_OF      MV_ITEM_T
#define LINE_W        MV_ITEM_W
#define LINE_H         4
#define MV_DEVLINE_L    (MV_ITEM_L + LINE_L_OF)
#define MV_DEVLINE_T    (MV_DEVCON_T + MV_DEVCON_H)
#define MV_SCB_L     (MV_ITEM_L + MV_ITEM_W+4)
#define MV_SCB_T     MV_ITEM_T
#define MV_SCB_W     12//18
#define MV_SCB_H     ((MV_ITEM_H + LINE_H)*MV_ITEM_CNT - LINE_H)
#define MV_ITEM_GAP 4
#define MV_ITEM_NAME_L    (MV_ITEM_L + MV_ITEM_GAP+6)//(MV_ITEM_ICON_L + MV_ITEM_ICON_W + MV_ITEM_GAP)
#define MV_ITEM_NAME_W    300//(MV_ITEM_W - MV_ITEM_IDX_W - MV_ITEM_ICON_W - MV_ITEM_GAP*5)
#define MV_DIR_L  (MV_LST_L+10)
#define MV_DIR_T  (MV_LST_T+MV_LST_H)
#define MV_DIR_W  (MV_LST_W-20)
#define MV_DIR_H  36//40
#define MV_HLP_L (MOVE_W_L + 30)
#define MV_HLP_T (MOVE_W_T + MOVE_W_H - 50)
#define MV_HLP_W  (MOVE_W_W - 30*2)
#define MV_HLP_H 36
#define MV_HLP_ICON_W 45
#define MV_HLP_TXT_W 100
#else
#define MOVE_W_L    100//270
#define MOVE_W_T    60
#define MOVE_W_W    360//450
#define MOVE_W_H    374//492
#define MV_GAP_L    18
#define MV_GAP_T    2
#define MV_TITLE_L    (MOVE_W_L+34)
#define MV_TITLE_T    (MOVE_W_T + 6)
#define MV_TITLE_W    (MOVE_W_W-70)
#define MV_TITLE_H    40
#define MV_DEVCON_L (MOVE_W_L + MV_GAP_L +10)
#define MV_DEVCON_T (MOVE_W_T + 50)//(TITLE_T + TITLE_H)
#define MV_DEVCON_W (MOVE_W_W - MV_GAP_L*2 -20)//536
#define MV_DEVCON_H 30
#define MV_DEVTXT_L    (MV_DEVCON_L + 86)
#define MV_DEVTXT_T    (MV_DEVCON_T)
#define MV_DEVTXT_W    (MV_DEVCON_W - 86 * 2)
#define MV_DEVTXT_H    24
#define MV_LST_L     (MOVE_W_L + MV_GAP_L)
#define MV_LST_T     (MV_DEVCON_T + MV_DEVCON_H + 4)//(MOVE_W_T + 70)
#define MV_LST_W    (MOVE_W_W - MV_GAP_L * 2)
#define MV_LST_H     (MOVE_W_H - 90 - MV_DEVCON_H - 40)
#define MV_ITEM_CNT    6//7
#define MV_ITEM_L    (MV_LST_L + 10)
#define MV_ITEM_T    (MV_LST_T + 0)
#define MV_ITEM_W    (MV_LST_W-30)
#define MV_ITEM_H    32
#define LINE_L_OF    0
#define LINE_T_OF      MV_ITEM_T
#define LINE_W        MV_ITEM_W
#define LINE_H         2
#define MV_DEVLINE_L    (MV_ITEM_L + LINE_L_OF)
#define MV_DEVLINE_T    (MV_DEVCON_T + MV_DEVCON_H)
#define MV_SCB_L     (MV_ITEM_L + MV_ITEM_W+4)
#define MV_SCB_T     MV_ITEM_T
#define MV_SCB_W     12//18
#define MV_SCB_H     ((MV_ITEM_H + LINE_H)*MV_ITEM_CNT - LINE_H)
#define MV_ITEM_GAP 4
#define MV_ITEM_NAME_L    (MV_ITEM_L + MV_ITEM_GAP+6)//(MV_ITEM_ICON_L + MV_ITEM_ICON_W + MV_ITEM_GAP)
#define MV_ITEM_NAME_W    270//(MV_ITEM_W - MV_ITEM_IDX_W - MV_ITEM_ICON_W - MV_ITEM_GAP*5)
#define MV_DIR_L  (MV_LST_L+10)
#define MV_DIR_T  (MV_SCB_T+MV_SCB_H+4)//(MV_LST_T+MV_LST_H)
#define MV_DIR_W  (MV_LST_W-20)
#define MV_DIR_H  32//40
#define MV_HLP_L (MOVE_W_L + 30)
#define MV_HLP_T (MOVE_W_T + MOVE_W_H - 45)
#define MV_HLP_W  (MOVE_W_W - 30*2)
#define MV_HLP_H 36
#define MV_HLP_ICON_W 45
#define MV_HLP_TXT_W 80
#endif
#define MV_SH_IDX      WSTL_MP_POP_WIN_01_HD
#define MV_TITLE_IDX   WSTL_TEXT_12_HD
#define MV_LST_CON_IDX  WSTL_NOSHOW_IDX
#define MV_DEV_CON_SH    WSTL_MP_DEV_SH_HD
#define MV_DEV_TXT_SH    WSTL_MP_DEV_TXT_HD
#define MV_DIR_CON_IDX    WSTL_MP_DIR_HD
#define MV_DIR_BMP_IDX    WSTL_MIX_BMP_HD
#define MV_DIR_TXT_IDX    WSTL_MP_DIR_TXT_HD
#define MV_LST_IDX     WSTL_NOSHOW_IDX
#define MV_CON_SH      WSTL_BUTTON_03_HD//sharon WSTL_MP_TEXT_03_HD
#define MV_CON_HL      WSTL_BUTTON_02_HD//sharon WSTL_BUTTON_03_HD
#define MV_TXT_SH      WSTL_BUTTON_03_HD//sharon WSTL_MP_TEXT_03_HD
#define MV_TXT_HL       WSTL_BUTTON_02_FG_HD
#define MV_ICON_SH     WSTL_MIX_BMP_HD
#define MV_LINE_SH       WSTL_NOSHOW_IDX//WSTL_POP_LINE_HD
#define MV_SCB_SH_IDX      WSTL_SCROLLBAR_01_8BIT
#define MV_SCB_HL_IDX      WSTL_SCROLLBAR_01_8BIT
#define MV_SCB_MID_RECT_IDX    WSTL_NOSHOW_IDX
#define MV_SCB_MID_THUMB_IDX    WSTL_SCROLLBAR_02_8BIT
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT | LIST_ITEMS_NOCOMPLETE | LIST_SCROLL | LIST_GRID \
                    | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE | LIST_CLIP_ENABLE)
#define VACT_UP_DIR (VACT_PASS + 1)
#define VACT_COPY (VACT_PASS + 2)
#define VACT_CHG_DEVL (VACT_PASS + 3)
#define VACT_CHG_DEVR (VACT_PASS + 4)
#define VACT_HELP (VACT_PASS + 5)


/*******************************************************************************
*    Function decalare
*******************************************************************************/
static void win_open_movefile_help(void);
static PRESULT movefile_unkown_act_proc(VACTION unact);
static void movefile_display_items(UINT16 u_top);
static void init_movefile_info(void);
static PRESULT movefile_change_storage(VACTION act);
static PRESULT movefile_item_enter(UINT16 u_idx);
static VACTION movefile_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT movefile_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION movefile_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT movefile_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MV_SH_IDX,MV_SH_IDX,MV_SH_IDX,MV_SH_IDX,   \
    movefile_con_keymap,movefile_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, \
    MV_TITLE_IDX,MV_TITLE_IDX,MV_TITLE_IDX,MV_TITLE_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 4,0,res_id,NULL)

#define LDEF_DEVCON(root, var_con,nxt_obj,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MV_DEV_CON_SH,MV_DEV_CON_SH,MV_DEV_CON_SH,MV_DEV_CON_SH,   \
    NULL,NULL,  \
    conobj, focus_id,0)

#define LDEF_DEVTXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MV_DEV_TXT_SH,MV_DEV_TXT_SH,MV_DEV_TXT_SH,MV_DEV_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 8,0,0,str)

#define LDEF_DIRCON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MV_DIR_CON_IDX,MV_DIR_CON_IDX,MV_DIR_CON_IDX,MV_DIR_CON_IDX,   \
    NULL,NULL,  \
    conobj, 0,0)

#define LDEF_DIRTXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MV_DIR_TXT_IDX,MV_DIR_TXT_IDX,MV_DIR_TXT_IDX,MV_DIR_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_ITEMCON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, MV_CON_SH,MV_CON_HL,MV_CON_SH,MV_CON_SH,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_ITEMTXT(root,var_txt,nxt_obj,ID,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, \
    MV_TXT_SH,MV_TXT_HL,MV_TXT_SH,MV_TXT_SH,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 8,0,0,str)

#define LDEF_ITEMBMP(root,var_bmp,nxt_obj,ID,l,t,w,h,bmp_id)        \
    DEF_BITMAP(var_bmp,&root, nxt_obj, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    ID, ID, ID, ID, ID, \
    l, t, w, h, \
    MV_ICON_SH, MV_ICON_SH, MV_ICON_SH, MV_ICON_SH, \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, bmp_id)

#define LDEF_POP_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, MV_LINE_SH,MV_LINE_SH,MV_LINE_SH,MV_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_name,var_line,ID,l,t,w,h,namestr)    \
    LDEF_ITEMCON(root, var_con,NULL,ID,ID,ID,((ID + 1)%MV_ITEM_CNT),((ID - 1)%MV_ITEM_CNT),l,t,w,h,&var_name,ID)    \
    LDEF_ITEMTXT(var_con,var_name,NULL ,ID,MV_ITEM_NAME_L,t,MV_ITEM_NAME_W,h,namestr)    \
    LDEF_POP_LINE(var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + MV_ITEM_H,LINE_W,LINE_H,0,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
    0, 0, 0, 0, 0, l, t, w, h, \
    MV_SCB_SH_IDX, MV_SCB_SH_IDX, MV_SCB_SH_IDX, MV_SCB_SH_IDX, \
    NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, \
    page, MV_SCB_MID_THUMB_IDX, MV_SCB_MID_RECT_IDX, \
    0,18, w, h-36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,MV_LST_IDX,MV_LST_IDX,MV_LST_IDX,MV_LST_IDX,   \
    movefile_list_keymap,movefile_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

/*******************************************************************************
*    Variable definition
*******************************************************************************/
static BOOL mpmove_exit_flag = FALSE;
static file_list_handle win_move_dirlist = NULL;
static char movefile_cur_device[4] = {0};
static unsigned int movefile_dirnum = 0;
static unsigned int movefile_filenum = 0;
static unsigned int movefile_curidx = 0;
static UINT8 move_file_flag = 0; // 0 - cancel : 1 move
/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
LDEF_WIN(g_win_mp_move,&mp_move_title,MOVE_W_L,MOVE_W_T,MOVE_W_W,MOVE_W_H,1)

LDEF_TITLE(g_win_mp_move,mp_move_title,&mpmove_dev_con,MV_TITLE_L,MV_TITLE_T,MV_TITLE_W,MV_TITLE_H,RS_HELP_COPY)

LDEF_DEVCON(g_win_mp_move,mpmove_dev_con, &mpmove_dir_con,MV_DEVCON_L, \
      MV_DEVCON_T, MV_DEVCON_W, MV_DEVCON_H, &mpmove_dev_name, 1)
LDEF_DEVTXT(mpmove_dev_con,mpmove_dev_name, NULL,MV_DEVTXT_L, MV_DEVTXT_T, MV_DEVTXT_W, MV_DEVTXT_H, display_strs[30])
//LDEF_POP_LINE(g_win_mp_move,mpmove_dev_line,&ol_mpmove,0,0,0,0,0,MV_DEVLINE_L, MV_DEVLINE_T ,LINE_W,LINE_H,0,0)

LDEF_DIRCON(g_win_mp_move, mpmove_dir_con,&ol_mpmove,MV_DIR_L,MV_DIR_T,MV_DIR_W,MV_DIR_H,&mpmove_dir_bmp)
LDEF_ITEMBMP(mpmove_dir_con, mpmove_dir_bmp, &mpmove_dir_txt, 0,MV_DIR_L+10,MV_DIR_T+10, 20, 20, IM_MEDIAPLAY_ICON_01)
LDEF_DIRTXT(mpmove_dir_con, mpmove_dir_txt,NULL,MV_DIR_L+40,MV_DIR_T,MV_DIR_W-50,MV_DIR_H,0,display_strs[31])

static POBJECT_HEAD mpmove_items[] =
{
    (POBJECT_HEAD)&mpmove_item_con1,
    (POBJECT_HEAD)&mpmove_item_con2,
    (POBJECT_HEAD)&mpmove_item_con3,
    (POBJECT_HEAD)&mpmove_item_con4,
    (POBJECT_HEAD)&mpmove_item_con5,
    (POBJECT_HEAD)&mpmove_item_con6,
    (POBJECT_HEAD)&mpmove_item_con7,
    (POBJECT_HEAD)&mpmove_item_con8,
};

LDEF_LISTBAR(ol_mpmove,mpmove_scb,MV_ITEM_CNT,MV_SCB_L,MV_SCB_T, MV_SCB_W, MV_SCB_H)

LDEF_OL(g_win_mp_move,ol_mpmove,NULL, MV_LST_L,MV_LST_T,MV_LST_W,MV_LST_H,LIST_STYLE, \
       MV_ITEM_CNT, 0,mpmove_items,&mpmove_scb,NULL,NULL)

LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con1,mpmove_filename1,mpmove_line1,1, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 0),MV_ITEM_W,MV_ITEM_H,display_strs[1])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con2,mpmove_filename2,mpmove_line2,2, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 1),MV_ITEM_W,MV_ITEM_H,display_strs[2])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con3,mpmove_filename3,mpmove_line3,3, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 2),MV_ITEM_W,MV_ITEM_H,display_strs[3])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con4,mpmove_filename4,mpmove_line4,4, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 3),MV_ITEM_W,MV_ITEM_H,display_strs[4])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con5,mpmove_filename5,mpmove_line5,5, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 4),MV_ITEM_W,MV_ITEM_H,display_strs[5])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con6,mpmove_filename6,mpmove_line6,6, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 5),MV_ITEM_W,MV_ITEM_H,display_strs[6])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con7,mpmove_filename7,mpmove_line7,7, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 6),MV_ITEM_W,MV_ITEM_H,display_strs[7])
LDEF_LIST_ITEM(ol_mpmove,mpmove_item_con8,mpmove_filename8,mpmove_line8,8, \
    MV_ITEM_L,(MV_ITEM_T + (MV_ITEM_H + MV_GAP_T) * 7),MV_ITEM_W,MV_ITEM_H,display_strs[8])

static TEXT_FIELD *mpmove_files[] =
{
    &mpmove_filename1,
    &mpmove_filename2,
    &mpmove_filename3,
    &mpmove_filename4,
    &mpmove_filename5,
    &mpmove_filename6,
    &mpmove_filename7,
    &mpmove_filename8,
};

/*******************************************************************************
*    Function definition
*******************************************************************************/
static VACTION movefile_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_MENU:
        mpmove_exit_flag = TRUE;
        act = VACT_CLOSE;
        break;
    case V_KEY_EXIT:
        if(TRUE == win_check_openroot(win_move_dirlist) )
        {
            mpmove_exit_flag = TRUE;
            act = VACT_CLOSE;
        }
        else//when not root,press exit to up dir
        {
            act = VACT_UP_DIR;
        }
        break;
    case V_KEY_GREEN:
        act = VACT_COPY;
        break;
    case V_KEY_LEFT:
        act = VACT_CHG_DEVL;
        break;
    case V_KEY_RIGHT:
        act = VACT_CHG_DEVR;
        break;
    case V_KEY_INFOR:
        act = VACT_HELP;
        break;
    default:
        act = VACT_PASS;
        break;

    }

    return act;
}
static PRESULT movefile_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;

    switch(event)
    {
       case EVN_PRE_OPEN:
        mpmove_exit_flag = FALSE;
        init_movefile_info();
        break;
    case EVN_POST_OPEN:
        win_open_movefile_help();
        break;
       case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;        
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = movefile_unkown_act_proc(unact);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION movefile_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
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
static PRESULT movefile_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 u_top = 0;
    UINT16 u_cur_pos = 0;
    PRESULT ret = PROC_PASS;

    u_top = osd_get_obj_list_top((POBJLIST)p_obj);
    u_cur_pos = osd_get_obj_list_cur_point((POBJLIST)p_obj);
    switch(event)
    {
        case EVN_PRE_DRAW:
            movefile_display_items(u_top);
            break;
        case EVN_POST_DRAW:
            break;
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
            break;
        case EVN_UNKNOWN_ACTION:
            ret = movefile_item_enter(u_cur_pos);
            break;
        default:
            break;
    }

    return ret;
}

UINT32 win_movefile_open(file_list_handle move_dirlist, char *move_device)
{
    UINT32 hkey = 0;
    CONTAINER *pop_win = NULL;
    PRESULT ret = PROC_LOOP;
        BOOL old_value = 0;

       if(!move_device)
       {
            libc_printf("%s: move_device is null\n", __FUNCTION__);
            return 0;
       }
        old_value = ap_enable_key_task_get_key(TRUE);

    win_move_dirlist = move_dirlist;
    strncpy(movefile_cur_device, move_device, 3);
      movefile_cur_device[3] = 0;
    move_file_flag = 0;

    pop_win = &g_win_mp_move;
    osd_set_container_focus(pop_win, 1);
    wincom_backup_region(&pop_win->head.frame);
    osd_obj_open((POBJECT_HEAD)pop_win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
             {
            continue;
             }
        ret = osd_obj_proc((POBJECT_HEAD)pop_win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    wincom_restore_region();
       ap_enable_key_task_get_key(old_value);

    return move_file_flag;
}

static void win_open_movefile_help(void)
{
    UINT8 i = 0;
    UINT16 hlp_imgs[] =
    {
        IM_EPG_COLORBUTTON_GREEN,
        IM_MEDIAPLAY_HELPBUTTON_LR,
        IM_MEDIAPLAY_HELPBUTTON_I,
    };

    UINT16 hlp_strs[] =
    {
        RS_HELP_COPY,
        RS_MP_HELP_SWITCH,
        0
    };


    DEF_BITMAP(hlp_bmp,NULL, NULL, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    0, 0, 0, 0, 0, \
    0,0,0,0, \
    WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, WSTL_MIX_BMP_HD, \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0,0)

    DEF_TEXTFIELD(hlp_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, \
    0,0,0,0, \
    WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,WSTL_TEXT_12_HD,    \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 4,0,RS_COMMON_OK,NULL)

    for(i = 0; i < 3;i++)
    {
        osd_set_obj_rect((POBJECT_HEAD)&hlp_bmp, \
            MV_HLP_L+i*(MV_HLP_ICON_W+MV_HLP_TXT_W), MV_HLP_T, MV_HLP_ICON_W, MV_HLP_H);
        osd_set_bitmap_content(&hlp_bmp, hlp_imgs[i]);
        osd_draw_object((POBJECT_HEAD)&hlp_bmp,C_UPDATE_ALL);

        osd_set_obj_rect((POBJECT_HEAD)&hlp_txt, \
            MV_HLP_L+MV_HLP_ICON_W+i*(MV_HLP_ICON_W+MV_HLP_TXT_W), MV_HLP_T, MV_HLP_TXT_W, MV_HLP_H);
        osd_set_text_field_content(&hlp_txt,STRING_ID,hlp_strs[i]);
        osd_draw_object((POBJECT_HEAD)&hlp_txt,C_UPDATE_ALL);
    }
}

static PRESULT movefile_unkown_act_proc(VACTION act)
{
    PRESULT ret_proc = PROC_PASS;
    UINT8 b_root = 0;
    UINT8 back_saved = 0;

    switch(act)
    {
        case VACT_UP_DIR:
            ret_proc = movefile_item_enter(0);//exit to up dir
            break;
        case VACT_COPY:
            b_root = win_check_openroot(win_move_dirlist);
            if(b_root)
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(200, 200, 200, 100);
                win_compopup_set_msg(NULL, NULL,RS_MSG_COPY_INVALID_PATH);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
                break;
            }
            else
            {
                get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, copyfile_dest_path,
                    FULL_PATH_SIZE);
                move_file_flag = 1;
                ret_proc = PROC_LEAVE;
            }
            break;
        case VACT_CHG_DEVL:
        case VACT_CHG_DEVR:
            if(1 == get_stroage_device_number(STORAGE_TYPE_ALL))
            {
                break;
            }
            ret_proc = movefile_change_storage(act);
            break;
        case VACT_HELP:

            break;
        default:
            break;
    }

    return ret_proc;
}

static void movefile_display_items(UINT16 u_top)
{
    UINT8 i = 0;
    UINT8 root_offset = 0;
    UINT8 b_root = 0;
    //char str_buff[MAX_FILE_NAME_SIZE + 1] = {0};
       char *str_buff = NULL;
       RET_CODE ret = 0;
    TEXT_FIELD *p_txt = NULL;
    media_file_info file_node;

       MEMSET(&file_node, 0x0, sizeof(media_file_info));
       str_buff = (char *)malloc(sizeof(char)*(MAX_FILE_NAME_SIZE+1));
       if(NULL == str_buff)
        {
            libc_printf("malloc in mvfile_dis_items fail\n");
            ASSERT(0);
            return ;
        }
      MEMSET(str_buff, 0, (MAX_FILE_NAME_SIZE+1));
    b_root = win_check_openroot(win_move_dirlist);
    if(FALSE == b_root)
    {
        root_offset = 1;//if not root dir,add an item to exit
       }
    else
    {
        root_offset = 0;
       }
    for(i = 0; i < MV_ITEM_CNT;i++)
    {
        if((0 == (u_top+i)) && (1 == root_offset))
        {
            p_txt = mpmove_files[i];
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"/..");
            continue;
        }//item exit

        ret = get_file_from_file_list(win_move_dirlist, (u_top+i + 1 - root_offset), &file_node);

        if(RET_SUCCESS == ret)
        {
            p_txt = mpmove_files[i];
            win_shorten_filename(file_node.name,str_buff, MAX_FILE_NAME_SIZE+1);
            osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_buff);
        }
        else
        {
            p_txt = mpmove_files[i];
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
        }
    }
    if(NULL != str_buff)
    {
        free(str_buff);
        str_buff = NULL;
    }
}

static void init_movefile_info(void)
{
    //char file_path[FULL_PATH_SIZE] = {0};
    char *file_path = NULL;
    char str_buff[10] = {0};
    UINT16 offset = 0;
    TEXT_FIELD *p_txt = NULL;
    POBJLIST p_ol = NULL;

       file_path = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == file_path)
        {
            libc_printf("file_path malloc fail in init_mv_info\n");
            ASSERT(0);
            return;
        }
       MEMSET(file_path, 0, FULL_PATH_SIZE);
    get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, file_path, FULL_PATH_SIZE);
    if(!strcmp(file_path, FS_ROOT_DIR))
    {
        offset = 0;
    }
    else
    {
        offset = 1;
    }

    p_ol = &ol_mpmove;
    osd_set_obj_list_count(p_ol,(movefile_dirnum + movefile_filenum + offset));
    osd_set_obj_list_new_point(p_ol,0);
    osd_set_obj_list_cur_point(p_ol,0);
    osd_set_obj_list_top(p_ol,0);

    p_txt = &mpmove_dir_txt;
    win_get_display_dirname(str_buff, 10, file_path);
    osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_buff);//show current dir name

    p_txt = &mpmove_dev_name;
    win_get_display_devicename(str_buff, 10,movefile_cur_device);
    snprintf(str_buff,10, "%s",str_buff);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buff);//show current device name
    if(NULL != file_path)
      {
        free(file_path);
        file_path = NULL;
      }
}



static PRESULT movefile_change_storage(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    TEXT_FIELD *p_txt = NULL;
    OBJLIST *p_ol = NULL;
    char str_buff[10] = {0};

    if(VACT_CHG_DEVL ==  act)
    {
        if(RET_FAILURE == win_get_next_device(movefile_cur_device, movefile_cur_device,TRUE))
        {
            return ret;
        }
    }
    else
    {
        if(RET_FAILURE == win_get_next_device(movefile_cur_device, movefile_cur_device,FALSE))
        {
            return ret;
        }
    }

    file_list_change_device(win_move_dirlist,movefile_cur_device);

    p_txt = &mpmove_dev_name;
    win_get_display_devicename(str_buff, 10,movefile_cur_device);
    // Oncer: I really do not know what this line mean...
    //sprintf(str_buff,"%s",str_buff);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buff);//show current device name
    //OSD_DrawObject((POBJECT_HEAD)pTxt,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD)&mpmove_dev_con,C_UPDATE_ALL);
/*
    pTxt = &mpmove_dir_txt;
    sprintf(str_buff,"%s","Root");
    OSD_SetTextFieldContent(pTxt,STRING_ANSI,(UINT32)str_buff);
    OSD_DrawObject((POBJECT_HEAD)pTxt,C_UPDATE_ALL);
*/
    p_ol  = &ol_mpmove;
    get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, NULL, 0);
    osd_set_obj_list_cur_point(p_ol, 0);
    osd_set_obj_list_new_point(p_ol, 0);
    osd_set_obj_list_top(p_ol, 0);
    osd_set_obj_list_count(p_ol, movefile_dirnum+movefile_filenum);

    osd_track_object((POBJECT_HEAD)p_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);

    return ret;
}

static PRESULT movefile_item_enter(UINT16 u_idx)
{
    PRESULT ret = PROC_LOOP;
    BOOL b_root = FALSE;
    char *str_name = NULL;
    char *str_txt = NULL;
    media_file_info file_node;
    POBJLIST p_ol = NULL;
    TEXT_FIELD *p_txt = NULL;
    UINT8 back_saved = 0;

       MEMSET(&file_node, 0x0, sizeof(media_file_info));
       str_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == str_name)
        {
            libc_printf("malloc 4 str_name in movefile_item_enter fail\n");
            ASSERT(0);
            return ret;
        }
       MEMSET(str_name, 0, FULL_PATH_SIZE);
       str_txt = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == str_txt)
        {
            //if(NULL != str_name)
            //{
                free(str_name);
                str_name = NULL;
            //}
            libc_printf("malloc 4 str_txt in mvfile_item_enter fail\n");
            ASSERT(0);
            return ret;
        }
       MEMSET(str_txt, 0, FULL_PATH_SIZE);
    b_root = win_check_openroot(win_move_dirlist);
    if((0 == u_idx) && (FALSE == b_root))
    {//TODO:Exit to up dir

        get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, str_name, FULL_PATH_SIZE);
        win_get_parent_dirname(str_txt, FULL_PATH_SIZE, str_name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        read_folder_to_file_list(win_move_dirlist, str_txt);
        win_compopup_smsg_restoreback();
        b_root = win_check_openroot(win_move_dirlist);
        if(b_root)
        {
            file_list_change_device(win_move_dirlist, movefile_cur_device);
        }
        movefile_curidx = win_get_file_idx(win_move_dirlist, str_name);
        if((UINT16)(~0) == movefile_curidx)
        {
            movefile_curidx = 0;
        }
        get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, str_name, FULL_PATH_SIZE);
        p_txt = &mpmove_dir_txt;
        win_get_display_dirname(str_txt, FULL_PATH_SIZE, str_name);//get dirname to show
        osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);

        p_ol  = &ol_mpmove;
        movefile_curidx = movefile_curidx + (b_root?0:1);
        osd_set_obj_list_cur_point(p_ol, movefile_curidx);
        osd_set_obj_list_new_point(p_ol, movefile_curidx);
        osd_set_obj_list_top(p_ol, movefile_curidx);
        if(TRUE  == b_root)
        {
            osd_set_obj_list_count(p_ol, (movefile_dirnum + movefile_filenum));//to show it's child dir
        }
        else
        {
            osd_set_obj_list_count(p_ol, (movefile_dirnum + movefile_filenum + 1));//to show it's child dir
        }
        if(osd_get_obj_list_count(p_ol) <= MV_ITEM_CNT)
        {
            osd_set_obj_list_top(p_ol, 0);
        }
        osd_track_object((POBJECT_HEAD)p_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);

        if(NULL != str_name)
        {
            free(str_name);
            str_name = NULL;
        }
        if(NULL != str_txt)
        {
            free(str_txt);
            str_txt = NULL;
        }
        return ret;
    }

    if(!b_root)
    {
        u_idx--;
    }

    get_file_from_file_list(win_move_dirlist, u_idx+1, &file_node);
    if(F_DIR == file_node.filetype)
    {
        //TODO: Enter child directory & retrive items,set not_root flag
        win_get_fullname(str_name, FULL_PATH_SIZE, file_node.path, file_node.name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        read_folder_to_file_list(win_move_dirlist, str_name);
        win_compopup_smsg_restoreback();
        get_file_list_info(win_move_dirlist, &movefile_dirnum, &movefile_filenum, str_name, FULL_PATH_SIZE);
        p_txt = &mpmove_dir_txt;
        win_get_display_dirname(str_txt, FULL_PATH_SIZE, str_name);//get dirname to show
        osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
        movefile_curidx = 0;
        p_ol  = &ol_mpmove;
        osd_set_obj_list_count(p_ol, (movefile_dirnum + movefile_filenum + 1));//to show it's child dir
        osd_set_obj_list_cur_point(p_ol, movefile_curidx);
        osd_set_obj_list_new_point(p_ol, movefile_curidx);
        osd_set_obj_list_top(p_ol, movefile_curidx);
        osd_track_object((POBJECT_HEAD)p_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    }
       if(NULL != str_name)
        {
            free(str_name);
            str_name = NULL;
        }
       if(NULL != str_txt)
        {
            free(str_txt);
            str_txt = NULL;
        }
    return ret;
}
#endif

