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
#if 0//ndef _WIN_USBPG_H_//new or change add by yuj
#define _WIN_USBPG_H_
//win_usbupg.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER win_usbupg_con;

extern CONTAINER usbupg_item_con1;
extern CONTAINER usbupg_item_con2;
extern CONTAINER usbupg_item_con3;

extern TEXT_FIELD usbupg_txt_upgrade_type;
extern TEXT_FIELD usbupg_txt_file_select;
extern TEXT_FIELD usbupg_txt_start;

extern MULTISEL usbupg_sel_upgrade_type;
extern MULTISEL usbupg_filelist_sel;

extern CONTAINER usbupg_info_con;
extern TEXT_FIELD usbupg_txt_progress;
extern PROGRESS_BAR usbupg_bar_progress;
extern TEXT_FIELD usbupg_txt_msg;
extern TEXT_FIELD usbupg_item_line0;
extern TEXT_FIELD usbupg_item_line1;
extern TEXT_FIELD usbupg_item_line2;

#ifdef _BUILD_LOADER_COMBO_
#define VACT_SWITCH_TO_OTAUPG  (VACT_PASS + 1)
#endif
#ifdef _NV_PROJECT_SUPPORT_
#define VACT_SWITCH_TO_OTAUPG  (VACT_PASS + 1)
#define VACT_SWITCH_TO_DVB     (VACT_PASS + 3)
#define C_SECTOR_SIZE          0x10000
#endif

extern UINT32 find_chunk_by_sector(UINT32 base,UINT32 chk_id,UINT32 chk_msk,UINT8 check_crc);
extern UINT32 str2uint32_dec(UINT8 *str, UINT8 len);
#ifdef __cplusplus
 }
#endif
#endif//_WIN_USBPG_H_

