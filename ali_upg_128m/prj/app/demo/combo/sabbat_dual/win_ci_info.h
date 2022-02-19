/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_info.h
*
*    Description: The display the information of CI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_CI_INFO_H_
#define _WIN_CI_INFO_H_
//win_ci_info.h
#ifdef __cplusplus
extern "C"{
#endif

extern TEXT_FIELD ciupg_description;
extern TEXT_FIELD ciupg_prgs;

extern CONTAINER  g_win_ci_info;
extern TEXT_FIELD txt_ci_info_title;
extern MULTI_TEXT txt_ci_info_subtitle;
extern TEXT_FIELD txt_ci_info_bottom;
extern TEXT_FIELD txt_ci_info_split1;
extern TEXT_FIELD txt_ci_info_split2;

extern SCROLL_BAR sb_ci_info;
extern OBJLIST      olst_ci_info;

extern SCROLL_BAR sb_ci_info_menu;
extern MULTI_TEXT txt_ci_info_menu;

//for ci enquiry
extern CONTAINER    dlg_ci_enquiry;
extern CONTAINER     con_ci_enquiry_edit;
extern MULTI_TEXT    txt_ci_enquiry_title;
extern TEXT_CONTENT ci_enquiry_content;
extern EDIT_FIELD    edt_ci_enquiry;

//define the ci menu info
extern CONTAINER con_ci_info_0;
extern CONTAINER con_ci_info_1;
extern CONTAINER con_ci_info_2;
extern CONTAINER con_ci_info_3;
extern CONTAINER con_ci_info_4;
extern CONTAINER con_ci_info_5;
extern CONTAINER con_ci_info_6;
extern CONTAINER con_ci_info_7;

extern TEXT_FIELD txt_ci_info_0;
extern TEXT_FIELD txt_ci_info_1;
extern TEXT_FIELD txt_ci_info_2;
extern TEXT_FIELD txt_ci_info_3;
extern TEXT_FIELD txt_ci_info_4;
extern TEXT_FIELD txt_ci_info_5;
extern TEXT_FIELD txt_ci_info_6;
extern TEXT_FIELD txt_ci_info_7;


#ifdef __cplusplus
 }
#endif
#endif//_WIN_CI_INFO_H_

