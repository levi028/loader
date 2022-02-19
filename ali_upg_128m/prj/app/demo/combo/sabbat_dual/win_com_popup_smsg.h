/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup_smsg.h
*
*    Description: The common function of smsg popup
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COM_POPUP_SMSG_H_
#define _WIN_COM_POPUP_SMSG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include "win_com_popup.h"


extern CONTAINER    g_win_popup_smsg;
extern TEXT_FIELD   win_popup_smsg_title_txt;
extern MULTI_TEXT  win_popup_smsg_msg_mbx;

 void win_msg_popup_smsg_open();
void win_msg_popup_smsg_close();
void win_compopup_smsg_init(win_popup_type_t type);
void win_compopup_smsg_set_title(char* str,char* unistr,UINT16 str_id);
void win_compopup_smsg_set_msg(char* str,char* unistr,UINT16 str_id);
void win_compopup_smsg_set_msg_ext(char* str,char* unistr,char* utf8, char*utf16, UINT16 str_id);
//void win_compopup_smsg_set_btnstr(UINT32 btn_idx, UINT16 str_id);
void win_compopup_smsg_set_frame(UINT16 x,UINT16 y,UINT16 w,UINT16 h);
void win_compopup_smsg_set_pos(UINT16 x,UINT16 y);
//void win_compopup_smsg_set_default_choice(win_popup_choice_t choice);
win_popup_choice_t win_compopup_smsg_open(void);
void win_compopup_smsg_close(void);
win_popup_choice_t win_com_popup_smsg_open(win_popup_type_t type,char* str,\
                                            UINT16 str_id);
win_popup_choice_t win_compopup_smsg_open_ext();
void win_compopup_smsg_smsg_restoreback(void);
void win_compopup_smsg_refresh(void);
BOOL win_msg_popup_smsg_opend(void);

#ifdef __cplusplus
}
#endif

#endif//_WIN_COM_SMSG_H_
