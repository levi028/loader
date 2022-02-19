/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_common.h
*
*    Description: The common function of CI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __WIN_CI_COMMON_H__
#define __WIN_CI_COMMON_H__
#ifdef __cplusplus
extern "C"{
#endif

#define CI_DATA_BUFFER_LEN 1024
#define    CI_ENQUIRY_MSG_LENGTH        0x80 // 128
#define    CI_ENQUIRY_EDIT_LENGTH    0x20 // 32

#define    CI_MENU_TXT_LENGTH        0x80 // 64
#define    CI_MENU_ENTRY_MAX_NUM    128


struct _win_ci_slots_info
{
    UINT8 slots_ns;
    UINT8 curr_slot;
    UINT8 status[2];
    UINT16 status_str[2][CI_MENU_TXT_LENGTH];
};

struct _win_ci_enquiry
{
    UINT16    msg_str[CI_ENQUIRY_MSG_LENGTH + 1];
    UINT16 *msg_str_addr[2]; // 0 - msg, 1 - NULL

    BOOL    is_mask;
    UINT32    expected_length;
    UINT32    val;
    UINT16    edit_str[CI_ENQUIRY_EDIT_LENGTH + 1];
};

struct _win_ci_menu
{
    BOOL    is_selectable;
    // title & subtitle & bottom
    UINT16    title_str[CI_MENU_TXT_LENGTH +1];
    UINT16    subtitle_str[CI_MENU_TXT_LENGTH +1];
    UINT16    bottom_str[CI_MENU_TXT_LENGTH +1];
    // entrys
    UINT16    entry_cnt;
    UINT16    entry_ls_txt[CI_MENU_ENTRY_MAX_NUM][CI_MENU_TXT_LENGTH + 1];
    UINT16 *     entry_ls_txt_addr[CI_MENU_ENTRY_MAX_NUM];
};

struct _ci_info
{
    UINT16 name[CI_MENU_TXT_LENGTH+1];
    UINT16 slot;
    struct _win_ci_enquiry enquiry;
    struct _win_ci_menu    menu;
};



UINT8 win_ci_get_slot_status(UINT8 slot);
void win_ci_get_slot_status_text(UINT8 slot, UINT16 *uni_str_status);
UINT8* win_ci_get_tmp_buf(void);
UINT8 win_ci_msg_to_status(UINT8 msg_type);
void win_ci_set_slot_status(UINT8 slot, UINT8 status);

enum
{
    WIN_CI_INFO_SHOW_IN_MAINMENU = 0,
    WIN_CI_INFO_SHOW_AS_MODAL    = 1,
};

void win_ci_info_set_modal(UINT8 modal);
void win_ci_info_set_slot(UINT8 slot);

#ifdef __cplusplus
 }
#endif
#endif// __WIN_CI_COMMON_H__


