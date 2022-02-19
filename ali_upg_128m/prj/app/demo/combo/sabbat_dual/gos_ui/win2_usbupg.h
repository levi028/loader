/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_usbupg.c
*
*    Description: The realize of USB upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN2_CA_USBPG_H_
#define _WIN2_CA_USBPG_H_
//win_usbupg.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win2_usbupg;

extern CONTAINER ca_usbupg_item_con1;
extern CONTAINER ca_usbupg_item_con2;
extern CONTAINER ca_usbupg_item_con3;

extern TEXT_FIELD ca_usbupg_txt_upgrade_type;
extern TEXT_FIELD ca_usbupg_txt_file_select;
extern TEXT_FIELD ca_usbupg_txt_start;

extern MULTISEL ca_usbupg_sel_upgrade_type;
extern MULTISEL ca_usbupg_filelist_sel;

extern CONTAINER ca_usbupg_info_con;
extern TEXT_FIELD ca_usbupg_txt_progress;
extern PROGRESS_BAR ca_usbupg_bar_progress;
extern TEXT_FIELD ca_usbupg_txt_msg;
extern TEXT_FIELD ca_usbupg_item_line0;
extern TEXT_FIELD ca_usbupg_item_line1;
extern TEXT_FIELD ca_usbupg_item_line2;
extern BITMAP ca_usbupg_item_l_arrow0;
extern BITMAP ca_usbupg_item_r_arrow0;
extern BITMAP ca_usbupg_item_l_arrow1;
extern BITMAP ca_usbupg_item_r_arrow1;

#ifdef _BUILD_LOADER_COMBO_
#define VACT_SWITCH_TO_OTAUPG  (VACT_PASS + 1)
#endif
#ifdef _NV_PROJECT_SUPPORT_
#define VACT_SWITCH_TO_OTAUPG  (VACT_PASS + 1)
#define VACT_SWITCH_TO_DVB     (VACT_PASS + 3)
#define C_SECTOR_SIZE          0x10000
#endif

#ifdef __cplusplus
 }
#endif
#endif//_WIN_USBPG_H_

