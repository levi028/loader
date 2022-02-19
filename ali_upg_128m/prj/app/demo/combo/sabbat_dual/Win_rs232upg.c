/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_rs232upg.c
*
*    Description: To realize RS232 upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <bus/dog/dog.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>

#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>
 #include <asm/chip.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "win_com.h"
#include "win_rs232upg.h"
#include "key.h"
#include "win_mainmenu_submenu.h"
#include "ctrl_key_proc.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
/*******************************************************************************
* Objects declaration
*******************************************************************************/
static VACTION rs232upg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT rs232upg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION rs232upg_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT rs232upg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION rs232upg_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT rs232upg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#ifdef SD_UI    //for 3281 erom upg
#define WIN_SH_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX WSTL_WIN_BODYRIGHT_01_HD


#define CON_SH_IDX  WSTL_BUTTON_01_HD
#define CON_HL_IDX  WSTL_BUTTON_05_HD
#define CON_HL1_IDX WSTL_BUTTON_02_HD
#define CON_SL_IDX  WSTL_BUTTON_01_HD
#define CON_GRY_IDX WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#define PROGRESSBAR_SH_IDX          WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX      WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX WSTL_BAR_01_HD
#define PROGRESS_TXT_SH_IDX WSTL_BUTTON_01_HD

#define MSG_SH_IDX WSTL_TEXT_09_HD

#define W_L         105//384
#define W_T         57//138
#define W_W         482
#define W_H         370

#define CON_L       (W_L+20)
#define CON_T       (W_T + 8)
#define CON_W       (W_W - 40)
#define CON_H       30
#define CON_GAP     8


#define TXTN_L_OF   10
#define TXTN_W      180
#define TXTN_H      CON_H
#define TXTN_T_OF   ((CON_H - TXTN_H)/2)

#define TXTS_L_OF   (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W - TXTN_W-20) //240
#define TXTS_H      CON_H
#define TXTS_T_OF   ((CON_H - TXTS_H)/2)


#define BAR_L   (CON_L+5)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*3+10)
#define BAR_W   (CON_W-BAR_TXT_W-10)
#define BAR_H   16

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W   100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36

#define MSG_L   (CON_L+5)
#define MSG_T   (BAR_T + BAR_H+20)
#define MSG_W   (CON_W-10)
#define MSG_H   CON_H//24
#define MSG_GAP 4

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+2)
#define LINE_W      CON_W
#define LINE_H      2
#else
#define WIN_SH_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX  WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX WSTL_WIN_BODYRIGHT_01_HD


#define CON_SH_IDX  WSTL_BUTTON_01_HD
#define CON_HL_IDX  WSTL_BUTTON_05_HD
#define CON_HL1_IDX WSTL_BUTTON_02_HD
#define CON_SL_IDX  WSTL_BUTTON_01_HD
#define CON_GRY_IDX WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#define PROGRESSBAR_SH_IDX          WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX      WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX WSTL_BAR_04_HD
#define PROGRESS_TXT_SH_IDX WSTL_BUTTON_01_HD

#define MSG_SH_IDX WSTL_TEXT_09_HD

#ifndef SUPPORT_CAS_A /* define in win_com_menu_define.h already */
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

#define TXTN_L_OF   10
#define TXTN_W      300//260
#define TXTN_H      40
#define TXTN_T_OF   ((CON_H - TXTN_H)/2)

#define TXTS_L_OF   (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W - 320) //240
#define TXTS_H      40
#define TXTS_T_OF   ((CON_H - TXTS_H)/2)


#define BAR_L   (CON_L+5)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*3+10)
#define BAR_W   (CON_W-BAR_TXT_W-10)
#define BAR_H   24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W   100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36

#define MSG_L   (CON_L+5)
#define MSG_T   (BAR_T + BAR_H+20)
#define MSG_W   (CON_W-10)
#define MSG_H   40//24
#define MSG_GAP 4

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#endif

#define RS232UPG_MSG_MAX_LINE 5//6
#define TRANSFER_ID 1
#define START_ID 3//2

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    rs232upg_item_keymap,rs232upg_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    rs232upg_item_sel_keymap,rs232upg_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, &root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
        NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
        rl,rt , rw, rh, 1, 100, 100, 0)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[1])

#define LDEF_TXT_MSG(root,var_txt)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, MSG_SH_IDX,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[0])

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    rs232upg_con_keymap,rs232upg_con_callback,  \
    nxt_obj, focus_id,0)


#define LDEF_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_line,var_num,ID,idu,idd,l,t,w,h,\
                        hl,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,TXTS_H,style,cur,cnt,ptbl) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,\
                        hl,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

LDEF_ITEM_SEL(win_rs232upg_con,rs232upg_item_con0,&rs232upg_item_con1,rs232upg_txt_transfer_type,rs232upg_line0,\
    rs232upg_sel_transfer_type,1,3,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,CON_HL_IDX,\
    RS_TOOLS_TRANSFER_MODE,STRING_ANSI,0,0,NULL)

LDEF_ITEM_SEL(win_rs232upg_con,rs232upg_item_con1,&rs232upg_item_con2,rs232upg_txt_upgrade_type,rs232upg_line1,\
    rs232upg_sel_upgrade_type,2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,CON_HL_IDX,\
    RS_TOOLS_UPGRADE_MODE,STRING_ANSI,0,0,NULL)

#ifdef SD_UI
LDEF_ITEM_TXT(win_rs232upg_con,rs232upg_item_con2,&rs232upg_bar_progress,rs232upg_txt_start,rs232upg_line2, \
    3,2,1,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL_IDX,RS_COMMON_START)
#else
LDEF_ITEM_TXT(win_rs232upg_con,rs232upg_item_con2,&rs232upg_bar_progress,rs232upg_txt_start,rs232upg_line2, \
    3,2,1,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL1_IDX,RS_COMMON_START)
#endif

#ifndef SD_UI
LDEF_PROGRESS_BAR(win_rs232upg_con,rs232upg_bar_progress,&rs232upg_txt_progress,    \
        BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W - 4,BAR_H - 10)
#else
LDEF_PROGRESS_BAR(win_rs232upg_con,rs232upg_bar_progress,&rs232upg_txt_progress,    \
        BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,2,BAR_W - 4,BAR_H-4)
#endif

LDEF_PROGRESS_TXT(win_rs232upg_con, rs232upg_txt_progress, NULL, \
        BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT_MSG(win_rs232upg_con,rs232upg_txt_msg)

LDEF_WIN(win_rs232upg_con,&rs232upg_item_con0,W_L,W_T,W_W,W_H,3)
/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
static char **rs232upg_upgtype_chars = NULL;
static char **rs232upg_transfertype_chars = NULL;
static UINT16 *rs232upg_msg_buf[RS232UPG_MSG_MAX_LINE+1];
static UINT32 rs232upg_msg_line = 0;

static void win_rs232upg_load_chunk_info(void)
{
    CHUNK_HEADER blk_header;
    UINT32 id= 0;
    UINT32 mask= 0;
    UINT8 chunk_number= 0;
    UINT8 i= 0;

#if (defined HDCP_IN_FLASH)
    __MAYBE_UNUSED__ UINT32 block_id = 0, hdcp_key_id = 0,ciplus_key_id = 0;
    UINT8 hdcp_chunk_index = 0;
    UINT8 cipluskey_chunk_index = 0;

    BOOL has_hdcp_chunk = FALSE;
    BOOL has_cipluskey_chunk = FALSE;

    UINT32 special_type =0;
#endif

#ifdef _MPLOADER_UPG_HOST_ENABLE_
	chunk_number = 1;
#else
    chunk_number = sto_chunk_count(0,0);
    if(0 == chunk_number)
    {
        return;
    }
    if(init_block_list() != SUCCESS)
    {
        return;
    }
#endif

    if(rs232upg_upgtype_chars != NULL)
    {
        FREE(rs232upg_upgtype_chars);
    }
    rs232upg_upgtype_chars = (char**)MALLOC(sizeof(char*)*chunk_number);
    if(NULL == rs232upg_upgtype_chars)
        ASSERT(0);
    for(i=0; i<chunk_number; i++)
    {
        rs232upg_upgtype_chars[i] = (char*)MALLOC(16*sizeof(char));
        if(NULL == rs232upg_upgtype_chars[i] )
            ASSERT(0);
        MEMSET(rs232upg_upgtype_chars[i], 0x0, 16);
    }

    strncpy(rs232upg_upgtype_chars[0],"allcode", 15);

    if(rs232upg_transfertype_chars != NULL)
    {
        FREE(rs232upg_transfertype_chars);
    }
    rs232upg_transfertype_chars = (char**)MALLOC(sizeof(char*)*2);
    if(NULL == rs232upg_transfertype_chars)
        ASSERT(0);
    for(i=0;i<2;i++)
    {
        rs232upg_transfertype_chars[i] = (char*)MALLOC(16*sizeof(char));
        if(NULL == rs232upg_transfertype_chars[i])
            ASSERT(0);
        MEMSET(rs232upg_transfertype_chars[i], 0x0, 16);
    }
    strncpy(rs232upg_transfertype_chars[0],"P2P", 15);

#ifndef _MPLOADER_UPG_HOST_ENABLE_
    for(i=1; i<chunk_number; i++)
    {
        id = 0;
        mask = 0;
#if (defined HDCP_IN_FLASH)
         special_type = 0;
#endif
        sto_chunk_goto(&id,mask,i + 1);
        if(sto_get_chunk_header(id, &blk_header) != 1)
        {
            return;
        }

#if (defined HDCP_IN_FLASH || defined CI_PLUS_SUPPORT )
#ifdef HDCP_IN_FLASH
        hdcp_key_id = HDCPKEY_CHUNK_ID;
        if(hdcp_key_id == ((UINT32)blk_header.id))
        {
            hdcp_chunk_index = i;
            has_hdcp_chunk = TRUE;
            special_type = 1;
        }
#endif
#ifdef CI_PLUS_SUPPORT
        ciplus_key_id = CIPLUSKEY_CHUNK_ID;
        if(ciplus_key_id == ((UINT32)blk_header.id))
        {
            cipluskey_chunk_index = i;
            has_cipluskey_chunk = TRUE;
            special_type = 1;
        }
#endif

        if(special_type != 1)
        {
            strncpy(rs232upg_upgtype_chars[i], (const char *)blk_header.name, 15);
            rs232upg_upgtype_chars[i][15] = 0;
        }

#else
        strncpy(rs232upg_upgtype_chars[i],blk_header.name, 15);
        rs232upg_upgtype_chars[i][15] = 0;
#endif
    }
#endif

#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    strncpy(rs232upg_transfertype_chars[0], "P2P", 15);
    rs232upg_transfertype_chars[0][15] = 0;
    strncpy(rs232upg_transfertype_chars[1], "P2M", 15);
    rs232upg_transfertype_chars[1][15] = 0;
    osd_set_multisel_sel_table(&rs232upg_sel_transfer_type,rs232upg_transfertype_chars);
    osd_set_multisel_count(&rs232upg_sel_transfer_type, 2);

#ifdef _MPLOADER_UPG_HOST_ENABLE_
	strncpy(rs232upg_upgtype_chars[0], "Flash Prog", 15);
	rs232upg_upgtype_chars[0][15] = 0;
	osd_set_multisel_sel_table(&rs232upg_sel_upgrade_type,rs232upg_upgtype_chars);
	osd_set_multisel_count(&rs232upg_sel_upgrade_type, 1);
#else
    strncpy(rs232upg_upgtype_chars[0], "Flash Prog & BL Enc", 15);
    rs232upg_upgtype_chars[0][15] = 0;
    strncpy(rs232upg_upgtype_chars[1], "Flash Prog", 15);
    rs232upg_upgtype_chars[1][15] = 0;
    strncpy(rs232upg_upgtype_chars[2], "BL Enc", 15);
    rs232upg_upgtype_chars[2][15] = 0;
    osd_set_multisel_sel_table(&rs232upg_sel_upgrade_type,rs232upg_upgtype_chars);
    osd_set_multisel_count(&rs232upg_sel_upgrade_type, 3);
#endif
 
#else

#ifdef HDCP_IN_FLASH
    if(TRUE == has_hdcp_chunk)
    {
        FREE(rs232upg_upgtype_chars[hdcp_chunk_index]);
        for(i=hdcp_chunk_index; i<chunk_number;i++)
            rs232upg_upgtype_chars[i] = rs232upg_upgtype_chars[i+1];
        chunk_number--;
        if( (TRUE == has_cipluskey_chunk) && hdcp_chunk_index < cipluskey_chunk_index)
            cipluskey_chunk_index --;

    }
#endif


#ifdef CI_PLUS_SUPPORT
        if(TRUE == has_cipluskey_chunk)
        {
            FREE(rs232upg_upgtype_chars[cipluskey_chunk_index]);
            for(i=cipluskey_chunk_index; i<chunk_number;i++)
                rs232upg_upgtype_chars[i] = rs232upg_upgtype_chars[i+1];
            chunk_number--;
        }
#endif

    osd_set_multisel_sel_table(&rs232upg_sel_upgrade_type,rs232upg_upgtype_chars);
    osd_set_multisel_count(&rs232upg_sel_upgrade_type, chunk_number);
    osd_set_multisel_sel_table(&rs232upg_sel_transfer_type,rs232upg_transfertype_chars);
    osd_set_multisel_count(&rs232upg_sel_transfer_type, 2);
#endif


}

static void win_rs232upg_clear(void)
{
    UINT8 i= 0;
    UINT8 chunk_number= 0;

    chunk_number = sto_chunk_count(0,0);
#ifdef HDCP_IN_FLASH
    chunk_number--;
#endif
    if(rs232upg_upgtype_chars != NULL)
    {
        for(i=0;i<chunk_number;i++)
        {
            FREE(rs232upg_upgtype_chars[i]);
        }
        FREE(rs232upg_upgtype_chars);

        rs232upg_upgtype_chars = NULL;
    }
}

static void win_rs232upg_process_update(INT32 process)
{

#ifdef WATCH_DOG_SUPPORT
    dog_set_time(0, 0);
#endif

    if(process > PERCENT_MAX)
    {
        process = 0;
    }
    osd_set_progress_bar_pos(&rs232upg_bar_progress,(INT16)process);
    osd_set_text_field_content(&rs232upg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);

    osd_draw_object( (POBJECT_HEAD)&rs232upg_bar_progress, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&rs232upg_txt_progress, C_UPDATE_ALL);
}

static void win_rs232upg_msg_update(UINT8 *str)
{
    UINT32 i= 0;
    UINT32 j= 0;
    OSD_RECT rect;

    com_asc_str2uni(str, display_strs[10+rs232upg_msg_line%RS232UPG_MSG_MAX_LINE]);

    for(i=0; i<RS232UPG_MSG_MAX_LINE; i++)
    {
        if(rs232upg_msg_line<RS232UPG_MSG_MAX_LINE)
        {
            j = i%RS232UPG_MSG_MAX_LINE;
        }
        else
        {
            j = (rs232upg_msg_line+1+i)%RS232UPG_MSG_MAX_LINE;
        }
        rs232upg_msg_buf[i] = display_strs[10+j];
    }

        //draw msg infos
    for(i=0; i<RS232UPG_MSG_MAX_LINE; i++)
    {
        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&rs232upg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&rs232upg_txt_msg, STRING_UNICODE, (UINT32)rs232upg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&rs232upg_txt_msg, C_UPDATE_ALL);
    }

    rs232upg_msg_line++;
}

static void win_rs232upg_msg_update_ext(UINT8 *str,INT32 process)
{
    UINT32 i= 0;
    UINT32 j= 0;
    OSD_RECT rect;
    UINT8 tmpbuf[256];

    snprintf((char *)tmpbuf,255,"%s --- (%ld%%) ",str,process); 

    com_asc_str2uni(tmpbuf, display_strs[10+rs232upg_msg_line%RS232UPG_MSG_MAX_LINE]);

    for(i=0; i<RS232UPG_MSG_MAX_LINE; i++)
    {
        if(rs232upg_msg_line<RS232UPG_MSG_MAX_LINE)
        {
            j = i%RS232UPG_MSG_MAX_LINE;
        }
        else
        {
            j = (rs232upg_msg_line+1+i)%RS232UPG_MSG_MAX_LINE;
        }
        rs232upg_msg_buf[i] = display_strs[10+j];
    }
 
    for(i=0; i<RS232UPG_MSG_MAX_LINE; i++)
    {
        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&rs232upg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&rs232upg_txt_msg, STRING_UNICODE, (UINT32)rs232upg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&rs232upg_txt_msg, C_UPDATE_ALL);
    } 

    if(process >= 100)
    {
        rs232upg_msg_line++;
    }
}

static void win_rs232upg_msg_clear(void)
{
    UINT8 i= 0;
    OSD_RECT rect;

    for(i=0;i<RS232UPG_MSG_MAX_LINE;i++)
    {
        display_strs[10+i][0] = 0;//ComAscStr2Uni("", display_strs[10+i]);
        rs232upg_msg_buf[i] = display_strs[10+i];

        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&rs232upg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&rs232upg_txt_msg, STRING_UNICODE, (UINT32)rs232upg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&rs232upg_txt_msg, C_UPDATE_ALL);
    }

    rs232upg_msg_line = 0;
}

static void win_rs232upg_update(INT32 type, INT32 process, UINT8 *str)
{
    UINT32 vscr_idx= 0;
    LPVSCR apvscr= NULL;

    vscr_idx = osal_task_get_current_id();
    apvscr = osd_get_task_vscr(vscr_idx);
    if (NULL == apvscr)
    {
        return;
    }
    switch(type)
    {
        case 1:
            win_rs232upg_process_update(process);
            break;
        case 2:
            win_rs232upg_msg_update(str);
            break;
        case 3:
            win_rs232upg_process_update(process);
            win_rs232upg_msg_update(str);
            break;
        case 4:
            win_rs232upg_msg_update_ext(str,process);
            break;
        default:
            break;
    }

    osd_update_vscr(apvscr);
}

static UINT32 upg_check_exit_key(void)
{
    UINT32 hkey = 0;
    UINT32 vkey = 0;
    struct pan_key key_struct;

    MEMSET(&key_struct, 0x0, sizeof (key_struct));
    if(key_get_key(&key_struct, 0))
    {
        hkey = scan_code_to_msg_code(&key_struct);
        ap_hk_to_vk(0,hkey, &vkey);
        if((V_KEY_EXIT == vkey) || ( V_KEY_MENU == vkey))
        {
            return 1;
        }
        else if(V_KEY_ENTER == vkey)
        {
            return 2;
        }
    }

    return 0;
}

static void win_rs232upg_start(void)
{
    UINT16 channel= 0;

    osd_set_progress_bar_pos(&rs232upg_bar_progress,(INT16)0);
    osd_set_text_field_content(&rs232upg_txt_progress, STRING_NUM_PERCENT, (UINT32)0);

    sys_upgrade(win_rs232upg_update,upg_check_exit_key);
    free_block_list();
    free_slave_list();
    if(SCREEN_BACK_VIDEO == screen_back_state)
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        api_play_channel(channel, TRUE, TRUE,FALSE);
    }
}
/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION rs232upg_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
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

#if (!defined(_EROM_UPG_HOST_ENABLE_) && !defined(_MPLOADER_UPG_HOST_ENABLE_))
    UINT8 bid = 0;
    bid = osd_get_obj_id(pobj);
    if(TRANSFER_ID == bid)
        act = VACT_PASS;
#endif

    return act;

}

static PRESULT rs232upg_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION rs232upg_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT rs232upg_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact =VACT_PASS;
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
    UINT32 upg_mode = osd_get_multisel_sel(&rs232upg_sel_upgrade_type);
    UINT32 transfer_sel = osd_get_multisel_sel(&rs232upg_sel_transfer_type);
#endif
    UINT8 bid= 0;

    bid = osd_get_obj_id(pobj);

    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>NON_ACTION_LENGTH);
            if((VACT_ENTER == unact) && (START_ID == bid))
            {
                api_set_system_state(SYS_STATE_UPGRAGE_HOST);

                osd_set_attr(&rs232upg_item_con0, C_ATTR_INACTIVE);
                osd_set_attr(&rs232upg_item_con1, C_ATTR_INACTIVE);
                osd_set_attr(&rs232upg_item_con2, C_ATTR_INACTIVE);
                osd_draw_object( (POBJECT_HEAD)&rs232upg_item_con0, C_UPDATE_ALL);
                osd_draw_object( (POBJECT_HEAD)&rs232upg_item_con1, C_UPDATE_ALL);
                osd_draw_object( (POBJECT_HEAD)&rs232upg_item_con2, C_UPDATE_ALL);

                clear_upg_flag();
   
                if(0 == osd_get_multisel_sel(&rs232upg_sel_upgrade_type))
                {
#if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
                    p2p_enable_upgrade_bootloader(FALSE);
#else
                    p2p_enable_upgrade_bootloader(TRUE);
#endif
#ifndef _MPLOADER_UPG_HOST_ENABLE_
                    if(init_block_list() != SUCCESS)
                    {
                        return PROC_LEAVE;
                    }
#endif
                    set_upg_flag(0, 0, 1);
                }
                else
                {
                    p2p_enable_upgrade_bootloader(FALSE);
                    if(init_block_list() != SUCCESS)
                    {
                        return PROC_LEAVE;
                    }
                    set_upg_flag(index_to_id(osd_get_multisel_sel(&rs232upg_sel_upgrade_type)), 0xFFFFFFFF, 1);
                }
                
#if (defined(_EROM_UPG_HOST_ENABLE_)||defined(_MPLOADER_UPG_HOST_ENABLE_))
                set_upgrade_mode(upg_mode);
                if(0==transfer_sel)
                {
                    set_transfer_mode(UPGRADE_SINGLE);
                }
                else
                {
                    set_transfer_mode(UPGRADE_MULTI);
                }
#endif

                //start transfer
                //add for M3612/M3812 T2/M3823/C3505 UART share with other function, enable UART before upgrade
                #ifdef UART_SHARE_OTHER
                        api_uart_enable(TRUE);
                #endif
                win_rs232upg_start();
                //disable UART after finish upgrade
                #ifdef UART_SHARE_OTHER
                        api_uart_enable(FALSE);
                #endif
                osd_set_attr(&rs232upg_item_con0, C_ATTR_ACTIVE);
                osd_set_attr(&rs232upg_item_con1, C_ATTR_ACTIVE);
                osd_set_attr(&rs232upg_item_con2, C_ATTR_ACTIVE);
                osd_draw_object( (POBJECT_HEAD)&rs232upg_item_con0, C_UPDATE_ALL);
                osd_draw_object( (POBJECT_HEAD)&rs232upg_item_con1, C_UPDATE_ALL);
                osd_track_object( (POBJECT_HEAD)&rs232upg_item_con2, C_UPDATE_ALL);

                api_set_system_state(SYS_STATE_NORMAL);
            }
            break;
        default:
            break;
    }

    return ret;
}

static VACTION rs232upg_con_keymap(POBJECT_HEAD pobj, UINT32 vkey)
{
    VACTION act=VACT_PASS;
 
    switch(vkey)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        if( ( (ALI_S3281==sys_ic_get_chip_id()) || 
			   (ALI_S3503==sys_ic_get_chip_id()) || 
			   (ALI_S3821==sys_ic_get_chip_id()) ||
			   (ALI_C3505==sys_ic_get_chip_id())) && 
			   check_rs232_item_is_active() ) 
        {
            act = VACT_PASS;
        }
        else
        {
            act = VACT_CLOSE;
        }
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT rs232upg_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&win_rs232upg_con,RS_UPGRADE_MODE_RS232, 0);
            win_rs232upg_load_chunk_info();
            osd_set_progress_bar_pos(&rs232upg_bar_progress,(INT16)0);
            osd_set_text_field_content(&rs232upg_txt_progress, STRING_NUM_PERCENT, (UINT32)0);
            break;

        case EVN_POST_OPEN:
            win_rs232upg_msg_clear();
            break;

        case EVN_PRE_CLOSE:
            win_rs232upg_clear();
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

            break;

        case EVN_POST_CLOSE:
            break;
        default:
            break;
    }
    return ret;
}


