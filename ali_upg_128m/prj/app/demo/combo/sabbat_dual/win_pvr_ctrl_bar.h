/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar.h

*    Description: The API of drawing a pvr ctrl bar will be defined
                  in this file.There will be some callback for updating.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_CTRL_BAR_H_
#define _WIN_PVR_CTRL_BAR_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "win_pvr_ctrl_bar_basic.h"
#include "win_pvr_ctrl_bar_key.h"
#include "win_pvr_ctrl_bar_draw.h"

extern BOOL rec_list_exit_all;

PRESULT prl_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT pvr_ctrlbar_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT prl_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT prl_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT pvr_ctrl_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT rec_etm_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
PRESULT rec_etm_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

/////////////////////////////////////////////////////////////////////////
#define PVR_BAR_LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon) \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define PVR_BAR_LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)    \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, \
        NULL, NULL, style, 0, 0, B_MID_SH_IDX, fsh, \
        rl,rt , rw, rh, 0, 100, 100, 1)

#define LDEF_CON_CTRBAR(root,var_con,nxt_obj,l,t,w,h,sh,focus_id) \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
        1,1,1,1,1, l,t,w,h, sh,sh,sh,sh,   \
        pvr_ctrlbar_keymap,pvr_ctrlbar_callback,  \
        NULL, focus_id,1)

#define PVR_BAR_LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,sh,focus_id)        \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        pvr_ctrl_keymap,pvr_ctrl_callback,  \
        nxt_obj, focus_id,1)

#define LDEF_LIST_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_LIST_CON(root, var_con,nxt_obj,ID,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, PVR_CON_SH_IDX,PVR_CON_HL_IDX,PVR_CON_SL_IDX,PVR_CON_GRY_IDX,   \
    prl_list_item_con_keymap,prl_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, PVR_TXTI_SH_IDX,PVR_TXTI_HL_IDX,PVR_TXTI_SL_IDX,PVR_TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTDATE(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTD_SH_IDX,TXTD_HL_IDX,TXTD_SL_IDX,TXTD_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTTIME(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTT_SH_IDX,TXTT_HL_IDX,TXTT_SL_IDX,TXTT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, PVR_TXTN_SH_IDX,PVR_TXTN_HL_IDX,PVR_TXTN_SL_IDX,PVR_TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LEFT_BMP_FLAG(root,var_bmp,nxt_obj,ID,l,t,w,h,icon)    \
  DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_HIDDEN,0, \
    ID,ID,ID,ID,ID, l,t,w,h, BMPF_SH_IDX,BMPF_HL_IDX,BMPF_SL_IDX,BMPF_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)

#undef LDEF_LINE
#define LDEF_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_bmp,var_txtidx,var_txt_date,var_txt_time,var_txt_name,var_lock_bmp,var_fta_bmp,\
    ID,l,t,w,h,idxstr,datestr,timestr,namestr)    \
    LDEF_LIST_CON(&root,var_con,NULL,ID,l,t,w,h,&var_bmp,1)    \
    LEFT_BMP_FLAG(&var_con,var_bmp,&var_txtidx,            0,l + ITEM_BMP_L,     t,ITEM_BMP_W, h,IM_PVR_ICON_PLAY) \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_date ,        0,l + PVR_ITEM_IDX_L,    t,PVR_ITEM_IDX_W,h,0,idxstr)    \
    LDEF_TXTDATE(&var_con,var_txt_date,&var_txt_time,    0,l + ITEM_DATE_L,     t,ITEM_DATE_W,h,0,datestr)    \
    LDEF_TXTTIME(&var_con,var_txt_time,&var_txt_name ,    0,l + ITEM_TIME_L,     t,ITEM_TIME_W,h,0,timestr)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_lock_bmp,1,l + ITEM_NAME_L,     t,ITEM_NAME_W,h,0,namestr)  \
    LEFT_BMP_FLAG(&var_con,var_lock_bmp,&var_fta_bmp,0,l + ITEM_LOCK_MBP_L,     t,ITEM_LOCK_MBP_W, h,IM_TV_LOCK) \
    LEFT_BMP_FLAG(&var_con,var_fta_bmp,NULL,0,l + ITEM_LOCK_FTA_L,     t,ITEM_LOCK_FTA_W, h,IM_INFORMATION_ICON_MONEY)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,PVR_LST_SH_IDX,PVR_LST_HL_IDX,PVR_LST_SL_IDX,PVR_LST_GRY_IDX,   \
    prl_list_keymap,prl_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_RECLIST_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,&g_win_pvr_ctrl,NULL,C_ATTR_ACTIVE,0, \
    2,2,2,2,2, l,t,w,h, WIN_LIST_SH_IDX,WIN_LIST_SH_IDX,WIN_LIST_SH_IDX,WIN_LIST_SH_IDX,   \
    prl_keymap,prl_callback,  \
    nxt_obj, focus_id,0)


#define LDEF_RECETM_TXT(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXT_ETM_SH_IDX,TXT_ETM_SH_IDX,TXT_ETM_SH_IDX,TXT_ETM_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_RECETM_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_ETM_SH_IDX,EDF_ETM_SH_IDX,EDF_ETM_SH_IDX,EDF_ETM_SH_IDX,   \
    rec_etm_edf_keymap,rec_etm_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,NULL,NULL,str)


#define LDEF_RECETM_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,&g_win_pvr_ctrl,NULL,C_ATTR_ACTIVE,0, \
    3,3,3,3,3, l,t,w,h, WIN_REC_ETM_SH_IDX,WIN_REC_ETM_SH_IDX,WIN_REC_ETM_SH_IDX,WIN_REC_ETM_SH_IDX,   \
    rec_etm_keymap,rec_etm_callback,  \
    nxt_obj, focus_id,0)

/////////////////////////////////////////////////////////////////////////

BOOL pvr_bar_list_showed(void);
void pvr_exit_audio_menu(void);
PRESULT recover(void);
BOOL is_win_pvr_ctrl_fun( UINT32 fun_in);
BOOL is_rec_poplist_displaying(void);
void get_rec_time(UINT32 rec_idx, INT32* timepassed,  INT32* timetotal);
BOOL win_pvr_recetm_on_screen(void);
void pvr_check_bar_exit(void);
BOOL pvr_enter_menu(UINT32 vkey);
void win_pvr_reclist_display(UINT32 flag);
#ifdef __cplusplus
}
#endif

#endif//_WIN_PVR_CTRL_BAR_H_

