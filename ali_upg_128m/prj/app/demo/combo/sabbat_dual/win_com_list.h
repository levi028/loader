/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_list.h
*
*    Description: To realize the common function of list
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COM_LIST_H_
#define _WIN_COM_LIST_H_
//win_com_list.h

#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER  g_win_com_lst;
extern OBJLIST    g_ol_com_list;

extern TEXT_FIELD list_title;
extern TEXT_FIELD list_txt_fld0;
extern TEXT_FIELD list_txt_fld1;
extern TEXT_FIELD list_txt_fld2;
extern TEXT_FIELD list_txt_fld3;
extern TEXT_FIELD list_txt_fld4;
extern TEXT_FIELD list_txt_fld5;
extern TEXT_FIELD list_txt_fld6;
extern TEXT_FIELD list_txt_fld7;
extern TEXT_FIELD list_txt_fld8;
extern TEXT_FIELD list_txt_fld9;
extern TEXT_FIELD list_txt_fld10;
extern TEXT_FIELD list_txt_fld11;

extern BITMAP   list_mark;
extern SCROLL_BAR list_bar;

#define COMLIST_MAX_DEP 12
#define MAX_MULTI_SL_ITEMS  6000
#define MAX_ITEM_STR_LEN    128

extern CONTAINER  g_win_com_lst;

void win_comlist_reset(void);
void win_comlist_set_frame(UINT16 x,UINT16 y,UINT16 w,UINT16 h);
void win_comlist_set_sizestyle(UINT16 count,UINT16 dep,UINT16 lststyle);
void win_comlist_set_mapcallback(PFN_KEY_MAP list_key_map,PFN_KEY_MAP win_key_map,\
                                    PFN_CALLBACK lstcallback);
void win_comlist_set_align(UINT16 ox,UINT16 oy,UINT8 align_style);
void win_comlist_set_str(UINT16 item_idx,char* str, char*unistr,UINT16 str_id);

void win_comlist_clear_sel(void);

void win_comlist_ext_set_ol_frame(UINT16 l,UINT16 t,UINT16 w,UINT16 h);
void win_comlist_ext_set_selmark_xaligen(UINT8 style,UINT8 offset);

void win_comlist_ext_set_title(char* str, char* unistr,UINT16 str_id);
void win_comlist_ext_set_title_style(UINT8 idx);
void win_comlist_ext_set_ol_items_style(UINT8 sh_idx, UINT8 hi_idx, \
                                        UINT8 sel_idx);
void win_comlist_ext_set_ol_items_allstyle(UINT8 sh_idx, UINT8 hi_idx, \
                                           UINT8 sel_idx, UINT8 gry_idx);
void win_comlist_ext_set_win_style(UINT8 idx);

void win_comlist_ext_set_selmark(UINT16 iconid);

/* For LIST_MULTI_SLECT, return the select flash array */
void win_comlist_ext_get_sel_items(UINT32** dw_select);
BOOL win_comlist_ext_check_item_sel(UINT16 index);
void win_comlist_ext_set_item_sel(UINT16 index);
void win_comlist_ext_set_item_cur(UINT16 index);
void win_comlist_ext_enable_scrollbar(BOOL b);
void win_comlist_ext_set_item_attr(UINT16 index, UINT8 attr);

BOOL win_comlist_open(void);
void win_comlist_close(void);


/* Get select item index(for LIST_SINGLE_SLECT) or select item count
    (for LIST_MULTI_SLECT) */
UINT16 win_comlist_get_selitem(void);

void win_comlist_popup();
void win_comlist_popup_ext(UINT8* back_saved);

#ifdef __cplusplus
}
#endif

#endif//_WIN_COM_LIST_H_
