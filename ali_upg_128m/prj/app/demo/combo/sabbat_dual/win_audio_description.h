/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_audio_description.h
*
*    Description: To realize the function of audio description control
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#ifdef AUDIO_DESCRIPTION_SUPPORT
#ifdef __cplusplus
extern "C"{
#endif

#if 0
#define AD_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define AD_DEBUG(...)    do{} while(0)
#endif

extern CONTAINER g_win_aud_desc;

extern CONTAINER ad_item0;
extern CONTAINER ad_item1;
extern CONTAINER ad_item2;

extern TEXT_FIELD ad_txt0;
extern TEXT_FIELD ad_txt1;
extern TEXT_FIELD ad_txt2;

extern TEXT_FIELD ad_line0;
extern TEXT_FIELD ad_line1;
extern TEXT_FIELD ad_line2;

extern MULTISEL  ad_sel0;
extern MULTISEL  ad_sel1;
extern MULTISEL  ad_sel2;

extern UINT8 ad_show_flag;

/*
 * @STATUS:   AD valid(1) or invalid(0)
 * @AD_TYPE:  AD type, AD_MSG_TYPE_RECEIVER_MIX or AD_MSG_TYPE_BOARDCASTER_MIX
 * @AD_LANG:  AD language
 */
#define AD_MSG_TYPE_RECEIVER_MIX        0
#define AD_MSG_TYPE_BOARDCASTER_MIX        1

#define AD_MSG_PARAM(STATUS, AD_TYPE, AD_LANG) \
    ((((STATUS) & 0xFF) << 24) | (((AD_TYPE) & 0xFF) << 16) | ((AD_LANG) & 0xFFFF))
#define AD_MSG_STATUS(MSG_PARAM)    (((MSG_PARAM) >> 24) & 0XFF)
#define AD_MSG_TYPE(MSG_PARAM)         (((MSG_PARAM) >> 16) & 0XFF)
#define AD_MSG_LANG(MSG_PARAM)         ((MSG_PARAM) & 0XFFFF)


extern int aud_desc_select(UINT16 aud_cnt, UINT16 *aud_pidarray, UINT16 *aud_lang_array,
                           UINT8 *aud_type_array, UINT16 main_aud_idx, BOOL notify);
extern void aud_desc_show_hint(UINT32 ad_msg);
extern void win_popup_ad_setting(void);

#ifdef __cplusplus
 }
#endif
#endif /* AUDIO_DESCRIPTION_SUPPORT */

