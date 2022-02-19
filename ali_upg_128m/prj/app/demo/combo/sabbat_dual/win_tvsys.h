/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_tvsys.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_TVSYS_H_
#define _WIN_TVSYS_H_

#include <basic_types.h>

#ifdef __cplusplus
extern "C"{
#endif
extern CONTAINER win_av_con;

extern CONTAINER av_item_con1;
extern CONTAINER av_item_con2;
extern CONTAINER av_item_con3;
extern CONTAINER av_item_con4;
extern CONTAINER av_item_con5;
extern CONTAINER av_item_con6;
#ifdef HDTV_SUPPORT
extern CONTAINER av_item_con7;
extern CONTAINER av_item_con8;
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
extern CONTAINER av_item_con9;
#endif
#ifdef HDMI_DEEP_COLOR
extern CONTAINER av_item_con10; //deep color
#endif
#ifdef SPDIF_DELAY_SUPPORT
extern CONTAINER av_item_con11;
#endif

extern TEXT_FIELD av_item_txtname1;
extern TEXT_FIELD av_item_txtname2;
extern TEXT_FIELD av_item_txtname3;
extern TEXT_FIELD av_item_txtname4;
extern TEXT_FIELD av_item_txtname5;
extern TEXT_FIELD av_item_txtname6;
#ifdef HDTV_SUPPORT
extern TEXT_FIELD av_item_txtname7;
extern TEXT_FIELD av_item_txtname8;
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
extern TEXT_FIELD av_item_txtname9;
#endif
#ifdef HDMI_DEEP_COLOR
extern TEXT_FIELD av_item_txtname10; //deep color
#endif
#ifdef SPDIF_DELAY_SUPPORT
extern TEXT_FIELD av_item_txtname11;
#endif

extern MULTISEL av_item_txtset1;
extern MULTISEL av_item_txtset2;
extern MULTISEL av_item_txtset3;
extern MULTISEL av_item_txtset4;
extern MULTISEL av_item_txtset5;
extern MULTISEL av_item_txtset6;
#ifdef HDTV_SUPPORT
extern MULTISEL av_item_txtset7;
extern MULTISEL av_item_txtset8;
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
extern MULTISEL av_item_txtset9;
#endif
#ifdef HDMI_DEEP_COLOR
extern MULTISEL av_item_txtset10; //deep color
#endif
#ifdef SPDIF_DELAY_SUPPORT
extern MULTISEL av_item_txtset11;
#endif

extern TEXT_FIELD av_item_line1;
extern TEXT_FIELD av_item_line2;
extern TEXT_FIELD av_item_line3;
extern TEXT_FIELD av_item_line4;
extern TEXT_FIELD av_item_line5;
extern TEXT_FIELD av_item_line6;
#ifdef HDTV_SUPPORT
extern TEXT_FIELD av_item_line7;
extern TEXT_FIELD av_item_line8;
#endif
#if (SUPPORT_DUAL_OUTPUT_ONOFF)
extern TEXT_FIELD av_item_line9;
#endif
#ifdef HDMI_DEEP_COLOR
extern TEXT_FIELD av_item_line10; //deep color
#endif
#ifdef SPDIF_DELAY_SUPPORT
extern TEXT_FIELD av_item_line11; 
#endif

#ifndef HDTV_SUPPORT
extern UINT16 video_output_ids[];
#endif
extern UINT16 video_vcr_loopthrough_ids[];
extern UINT32 rf_ch_range[][3];

PRESULT comlist_tvsys_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
UINT32 pal_ntsc_tvmode_setting_to_osd(enum TV_SYS_TYPE set);
UINT32 win_av_get_index(SYSTEM_DATA* p_sys_data);
void win_av_set_ratio(SYSTEM_DATA* p_sys_data,UINT16 result);
UINT8 win_language_get_tvsys_num(void);
RET_CODE set_tv_aspect_ratio_mode(UINT8 tv_ratio);
RET_CODE get_tv_aspect_ratio_mode(UINT8 *tv_radio);

char *get_tv_mode_string(UINT32 index);
UINT32 tvmode_osd_to_setting(UINT32 osd);
UINT32 get_tv_mode_set(UINT32 index);
UINT32 get_tv_mode_panel_next(UINT32 index);
UINT32 tvmode_setting_to_osd(enum TV_SYS_TYPE set);

#ifdef __cplusplus
 }
#endif
#endif//_WIN_TVSYS_H_

