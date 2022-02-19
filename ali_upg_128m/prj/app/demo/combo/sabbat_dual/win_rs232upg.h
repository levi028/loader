/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_rs232upg.h
*
*    Description: To realize RS232 upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_RS232UPG_H_
#define _WIN_RS232UPG_H_
//win_rs232upg.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER win_rs232upg_con;

extern CONTAINER rs232upg_item_con0; //transfer mode
extern CONTAINER rs232upg_item_con1;
extern CONTAINER rs232upg_item_con2;

extern TEXT_FIELD rs232upg_txt_transfer_type; //transfer mode
extern TEXT_FIELD rs232upg_txt_upgrade_type;

extern TEXT_FIELD rs232upg_txt_start;

extern TEXT_FIELD rs232upg_txt_progress;

extern TEXT_FIELD rs232upg_txt_msg;


extern MULTISEL    rs232upg_sel_transfer_type;
extern MULTISEL    rs232upg_sel_upgrade_type;

extern PROGRESS_BAR rs232upg_bar_progress;

extern TEXT_FIELD rs232upg_line0;
extern TEXT_FIELD rs232upg_line1;
extern TEXT_FIELD rs232upg_line2;

#ifdef __cplusplus
 }
#endif
#endif//_WIN_RS232UPG_H_

