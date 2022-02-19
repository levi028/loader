 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_net_upg.h
*
*    Description:   The realize of network upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_NET_LOCAL_SETING_H_
#define _WIN_NET_LOCAL_SETING_H_
//win_net_local_seting.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_netupg;

extern CONTAINER netupg_item_con1;
extern CONTAINER netupg_item_con2;
extern CONTAINER netupg_item_con3;
extern CONTAINER netupg_item_con4;
extern CONTAINER netupg_item_con5;
extern CONTAINER netupg_item_con6;

extern TEXT_FIELD netupg_item_txtname1;
extern TEXT_FIELD netupg_item_txtname2;
extern TEXT_FIELD netupg_item_txtname3;
extern TEXT_FIELD netupg_item_txtname4;
extern TEXT_FIELD netupg_item_txtname5;
extern TEXT_FIELD netupg_item_txtname6;

extern MULTISEL netupg_item_tcpip_type;
extern MULTISEL netupg_item_url_type;
extern EDIT_FIELD netupg_item_url;
extern EDIT_FIELD netupg_item_user;
extern EDIT_FIELD netupg_item_pwd;
extern MULTISEL netupg_item_txtset5;
extern MULTISEL netupg_item_txtset6;

extern CONTAINER netupg_info_con;
extern TEXT_FIELD netupg_txt_progress;
extern PROGRESS_BAR netupg_bar_progress;
extern TEXT_FIELD netupg_txt_msg;

extern TEXT_FIELD netupg_item_line1;
extern TEXT_FIELD netupg_item_line2;
extern TEXT_FIELD netupg_item_line3;
extern TEXT_FIELD netupg_item_line4;
extern TEXT_FIELD netupg_item_line5;
extern TEXT_FIELD netupg_item_line6;


#ifdef __cplusplus
 }
#endif
#endif//_WIN_NET_LOCAL_SETING_H_

