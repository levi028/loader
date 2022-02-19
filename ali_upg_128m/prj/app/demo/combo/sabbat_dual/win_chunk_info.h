/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chunk_info.k
*
*    Description: To realize the UI for display system information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_CHUNK_INFO_H_
#define _WIN_CHUNK_INFO_H_
//win_chunk_info.h
#ifdef __cplusplus
extern "C"{
#endif
extern CONTAINER win_info_con;
extern CONTAINER info_item_con;
extern TEXT_FIELD info_txt_name;
extern TEXT_FIELD info_txt1; //version or length
extern TEXT_FIELD info_txt2; //time or offset
extern TEXT_FIELD info_line; //time or offset

BOOL win_epg_is_show_logo(void);
BOOL factorytest_check_magic_key_diag(UINT32 code);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_CHUNK_INFO_H_

