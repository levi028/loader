/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: psi_pmt.h
*
*    Description: parse PMT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __PSI_PMT_H__
#define __PSI_PMT_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/si_module.h>

enum
{
    ES_DTS_EXIST_BIT              = 0,
    ES_CA_EXIST_BIT               = 1,
    ES_ISO639_EXIST_BIT           = 2,
    ES_MPEG4_VIDEO_EXIST_BIT      = 3,
    ES_MPEG4_AUDIO_EXIST_BIT      = 4,
    ES_AVC_VIDEO_EXIST_BIT        = 5,
    ES_TTX_EXIST_BIT              = 6,
    ES_SUB_EXIST_BIT              = 7,
    ES_AC3_EXIST_BIT              = 8,
    ES_AAC_EXIST_BIT              = 9,

#if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_))
    ES_CAROUSEL_ID_EXIST_BIT      = 10,
    ES_DATA_BROADCASTID_EXIST_BIT = 11,
#endif

#ifdef _SERVICE_MOVE_ENABLE_
    ES_SERVICE_MOVE_EXIST_BIT     = 12,
#endif
    ES_EAC3_EXIST_BIT             = 13,
#ifdef VIDEO_SEAMLESS_SWITCHING
    ES_V_CTRL_EXIST_BIT           = 14,
#endif
//#if (defined(_ISDBT_ENABLE_))
    ES_COM_TAG_EXIST_BIT          = 15,
    ES_PR_EXIST_BIT               = 16,
#if ((defined(ISDBT_CC)&&ISDBT_CC == 1) || defined(_MHEG5_SUPPORT_))
    ES_STREAM_ID_EXIST_BIT        = 17,
    ES_DATA_COMP_EXIST_BIT        = 18,
#endif
//#endif
#ifdef ATSC_SUBTITLE_SUPPORT
    ES_EXTENDED_VIDEO_DESC_EXIST_BIT= 19,
#endif
	ES_HEVC_VIDEO_EXIST_BIT        = 20,

#ifdef AUDIO_DESCRIPTION_SUPPORT
    ES_EXTENSION_DESC_EXIST_BIT   = 31,
#endif
};

enum
{
    ES_DTS_EXIST                 = 1<<ES_DTS_EXIST_BIT,
    ES_CA_EXIST                  = 1<<ES_CA_EXIST_BIT,
    ES_ISO639_EXIST              = 1<<ES_ISO639_EXIST_BIT,
    ES_MPEG4_VIDEO_EXIST         = 1<<ES_MPEG4_VIDEO_EXIST_BIT,
    ES_MPEG4_AUDIO_EXIST         = 1<<ES_MPEG4_AUDIO_EXIST_BIT,
    ES_AVC_VIDEO_EXIST           = 1<<ES_AVC_VIDEO_EXIST_BIT,
    ES_HEVC_VIDEO_EXIST          = 1<<ES_HEVC_VIDEO_EXIST_BIT,
    ES_TTX_EXIST                 = 1<<ES_TTX_EXIST_BIT,
    ES_SUB_EXIST                 = 1<<ES_SUB_EXIST_BIT,
    ES_AC3_EXIST                 = 1<<ES_AC3_EXIST_BIT,
    ES_AAC_EXIST                 = 1<<ES_AAC_EXIST_BIT,
#ifdef _SERVICE_MOVE_ENABLE_
    ES_SERVICE_MOVE_EXIST        = 1<<ES_SERVICE_MOVE_EXIST_BIT,
#endif
    ES_EAC3_EXIST                = 1<<ES_EAC3_EXIST_BIT,
#if ((defined(ISDBT_CC)&&ISDBT_CC == 1) || defined(_MHEG5_SUPPORT_))
    ES_STREAM_ID_EXIST           = 1<<ES_STREAM_ID_EXIST_BIT,
    ES_DATA_COMP_EXIST           = 1<<ES_DATA_COMP_EXIST_BIT,
#endif
#ifdef ATSC_SUBTITLE_SUPPORT
    ES_EXTENDED_VIDEO_DESC_EXIST =1<<ES_EXTENDED_VIDEO_DESC_EXIST_BIT,
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
    ES_EXTENSION_DESC_EXIST      = 1<<ES_EXTENSION_DESC_EXIST_BIT,
#endif

};

#ifdef AUDIO_DESCRIPTION_SUPPORT
// refer to ISO 639 language descriptor, Supplementary audio descriptor
enum
{
    AUDIO_TYPE_MAIN_AUDIO           = 0x00, // Main audio (Undefined)
    AUDIO_TYPE_CLEAN_EFFECTS        = 0x01, // Clean effects
    AUDIO_TYPE_CLEAN_AUDIO          = 0x02, // Clean audio (Hearing impaired)
    AUDIO_TYPE_AD_RECEIVER_MIXED    = 0x03, // Audio description (Receiver mixed)
    AUDIO_TYPE_AD_BROADCAST_MIXED   = 0x04, // Audio description (Broadcast mixed)
    AUDIO_TYPE_SPOKEN_SUBTITLE      = 0x05, // Spoken subtitles
};

#define AUDIO_TYPE_IS_RECEIVER_MIXED_AD(a)  ((a) == AUDIO_TYPE_AD_RECEIVER_MIXED)
#define AUDIO_TYPE_IS_BROADCAST_MIXED_AD(a) ((a) == AUDIO_TYPE_AD_BROADCAST_MIXED)
#define AUDIO_TYPE_IS_AUD_DESC(a)           (AUDIO_TYPE_IS_RECEIVER_MIXED_AD(a) || AUDIO_TYPE_IS_BROADCAST_MIXED_AD(a))
#endif

//#if ((SYS_PROJECT_SM&PROJECT_SM_CI)!=0)
//#define MAX_ES_CA_DESC_LEN        0x40
//#endif
#define ES_INFO_ONE_LANG_SIZE       3   // one audio language is 3 characters
#define ES_INFO_MAX_LANG_SIZE       64  // MAX 21 audio languages in one descriptor
struct pmt_es_info
{
    UINT16 pid;
    UINT8 stream_type;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
    UINT8 audio_type[2];
    UINT8 track_count;
#endif
    UINT8 lang[ES_INFO_MAX_LANG_SIZE];
    UINT32 stat;
//#if (defined(_ISDBT_ENABLE_))
    UINT8 audio_com_tag;
//#endif

    UINT8 cas_count;
    UINT16 cas_sysid[P_MAX_CA_NUM];
    UINT16 cas_pid[P_MAX_CA_NUM];
#if 0
    //for subtitle and ttx
    UINT8 subt_lang_num;
    struct t_subt_lang subt_lang[SUBTITLE_LANG_NUM];

    UINT8 ttx_lang_num;
    struct t_ttx_lang init_page[TTX_SUBT_LANG_NUM];
    UINT8 ttx_subt_lang_num;
    struct t_ttx_lang ttx_subt_page[TTX_SUBT_LANG_NUM];
#endif
//#if ((SYS_PROJECT_SM&PROJECT_SM_CI)!=0)
//  UINT8 ca_len;
//  UINT8 ca_data[MAX_ES_CA_DESC_LEN];
//#endif

#if (defined( _MHEG5_V20_ENABLE_) || defined( _MHEG5_SUPPORT_))
    UINT32 carousel_id;

    BOOL got_nb;
    UINT8 nb_version;
    UINT8 boot_info_length;
    UINT8 nb_action;
    UINT8 *boot_info_start_byte;
#endif

#ifdef _MHEG5_SUPPORT_
    UINT16 elem_count;
    PMT_ELEM_INFO es_info[P_MAX_ELEM_NUM];
#endif

#ifdef AUTO_OTA
    UINT32 descriptors_parse_stat;
    PMTSSU_INFO pmt_ssu_info;
#endif
#ifdef VIDEO_SEAMLESS_SWITCHING
  //bit[0-1] not used in spec and here use for exist flag,
  //bit[2-5] encode format,
  //bit[6] sequence end flag,
  //big[7]still picture flag
    UINT8 video_control;
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT
    // for Supplementary Audio Descriptor
    UINT8 sad_present  : 1;
    UINT8 sad_mix_type : 1;
    UINT8 sad_editorial_classification : 5;
    UINT8 sad_language_code_present    : 1;
    UINT8 sad_lang[ES_INFO_ONE_LANG_SIZE];
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

INT32 psi_pmt_parser(UINT8 *pmt, PROG_INFO *p_info, INT32 max_es_nr);
UINT32 pmt_get_rating(void);

#ifdef AUTO_OTA
INT32 get_ssuinfo_from_pmt(struct program_map *psi_info, UINT16 *pssu_pid, UINT16 pmtpid);
extern UINT32 si_get_otaconfig_oui();
#endif

#ifdef LIB_TSI3_FULL
void psi_pmt_get_ecm_pid(UINT8 *section, INT32 length, UINT16 param);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PSI_PMT_H__ */
