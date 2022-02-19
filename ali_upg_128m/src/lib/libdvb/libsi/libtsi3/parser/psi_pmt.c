/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: psi_pmt.c
*
*    Description: parse PMT table to get program video/audio pid and other information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libsi/si_module.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_service_type.h>

#ifdef AUTO_OTA
#include <api/libota/lib_ota.h>
#endif

#define PP_DEBUG_LEVEL          0
#if (PP_DEBUG_LEVEL>0)
#define PP_PRINTF           libc_printf
#else
#define PP_PRINTF(...)          do{}while(0)
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT

//#define _AD_DEBUG

#ifdef _AD_DEBUG
#define ad_debug(fmt, arg...) libc_printf(fmt, ##arg)
#define ad_error(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define ad_debug(...)   do{} while(0)
#define ad_error(...)   do{} while(0)
#endif

#endif

static INT32 loop1_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#ifdef PARENTAL_SUPPORT
static  INT32 loop1_pr_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif
static INT32 loop2_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
static INT32 iso639_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#ifdef _MHEG5_SUPPORT_
static INT32 carousel_id_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif
static INT32 com_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#ifdef AUTO_OTA
INT32 databroadcastid_deschandle(UINT8 tag, UINT8 len, UINT8 *pdata, void *ppriv);
#endif
#if defined (_MHEG5_V20_ENABLE_)
static INT32 data_broadcastid_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif

#ifdef _MHEG5_SUPPORT_
static INT32 stream_identifier_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
static INT32 data_broadcastid_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif

#ifdef VIDEO_SEAMLESS_SWITCHING
static INT32 video_control_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
static INT32 extension_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv);
#endif
static INT32 registion_deschandle(UINT8 tag, UINT8 len, UINT8 *pdata, void *ppriv);

static struct desc_table pmt_loop1[] = {
    {
        CA_DESCRIPTOR,
        ES_CA_EXIST_BIT,
        loop1_ca_desc_handle,
    },
#ifdef PARENTAL_SUPPORT
    {
        PARENTAL_RATING_DESCRIPTOR,
        0,//ES_PR_EXIST_BIT,
        loop1_pr_desc_handle,
    },
#endif
};

/*
 * please pay attention that this array should be sorted by tag in ascend order.
 * other wise the desc_loop_parser won't work out!!!
 */
static struct desc_table pmt_loop2[] = {
    {
        DTS_REGST_DESC,
        ES_DTS_EXIST_BIT,
        NULL,
    },
    {
        CA_DESCRIPTOR,
        ES_CA_EXIST_BIT,
        loop2_ca_desc_handle,
    },
    {
        ISO_639_LANGUAGE_DESCRIPTOR,
        ES_ISO639_EXIST_BIT,
        iso639_desc_handle,
    },
#ifdef _MHEG5_SUPPORT_
    {
        CAROUSEL_ID_DES_TAG,
        ES_CAROUSEL_ID_EXIST_BIT,
        carousel_id_desc_handle,
    },
#endif
    {
        MPEG_4_VIDEO_TAG,
        ES_MPEG4_VIDEO_EXIST_BIT,
        NULL,
    },
    {
        MPEG_4_AUDIO_TAG,
        ES_MPEG4_AUDIO_EXIST_BIT,
        NULL,
    },
    {
        AVC_VIDEO_TAG,
        ES_AVC_VIDEO_EXIST_BIT,
        NULL,
    },
    {
        COMPONENT_DESCRIPTOR,
        ES_COM_TAG_EXIST_BIT,
        com_desc_handle,
    },
#if ((defined(ISDBT_CC)&&ISDBT_CC == 1 ) || defined(_MHEG5_SUPPORT_))
    {
        STREAM_IDENTIFIER_DESCRIPTOR,
        ES_STREAM_ID_EXIST_BIT,
    #if defined(_MHEG5_SUPPORT_)		
		stream_identifier_desc_handle,
	#else	
        NULL,
    #endif
    },
#endif
    {
        TELTEXT_DESCRIPTOR,
        ES_TTX_EXIST_BIT,
        NULL,
    },
    {
        SUBTITLE_DESCRIPTOR,
        ES_SUB_EXIST_BIT,
        NULL,
    },
#ifdef AUTO_OTA
    {
        DATA_BROADCASTID_DES_TAG,
        ES_DTS_EXIST_BIT,
        databroadcastid_deschandle,
    },
#endif
#if (defined(_MHEG5_SUPPORT_) ||defined(_MHEG5_V20_ENABLE_))
    {
        DATA_BROADCASTID_DES_TAG,
        ES_DATA_BROADCASTID_EXIST_BIT,
        data_broadcastid_desc_handle,
    },
#endif
    {
        AC3_DESCRIPTOR,
        ES_AC3_EXIST_BIT,
        NULL,
    },
    {
        ENHANCED_AC3_DESCRIPTOR,
        ES_EAC3_EXIST_BIT,
        NULL,
    },
    {
        AAC_DESCRIPTOR,
        ES_AAC_EXIST_BIT,
        NULL,
    },
#ifdef VIDEO_SEAMLESS_SWITCHING
    {
        ISDBT_VIDEO_DECODE_CONTROL_DESCRIPTOR,
        ES_V_CTRL_EXIST_BIT,
        video_control_desc_handle,
    },
#endif
#if (defined(ISDBT_CC)&&ISDBT_CC == 1)
    {
        ISDBT_DATA_COMPONENT_DESCRIPTOR,
        ES_DATA_COMP_EXIST_BIT,
        NULL,
    },
#endif
#ifdef ATSC_SUBTITLE_SUPPORT
    {
        ATSC_EXTENDED_VIDEO_DESCRIPTOR,
        ES_EXTENDED_VIDEO_DESC_EXIST_BIT,
        NULL,
    },
#endif
#ifdef AUDIO_DESCRIPTION_SUPPORT
    {
        EXTENSION_DESCRIPTOR,
        ES_EXTENSION_DESC_EXIST_BIT,
        extension_desc_handle,
    },
#endif
	{
		REGISTRATION_DESCRIPTOR,
		0,
		registion_deschandle
	},
};

#ifdef PARENTAL_SUPPORT
static UINT8 g_pmt_rating =0;
#endif

#ifdef LIB_TSI3_FULL
static UINT8 ecm_count = 0;
static UINT32 psi_pmt_crc_value = 0;
static UINT16 ecm_pid_array[32] = {0};
#endif

#ifdef VIDEO_SEAMLESS_SWITCHING
static INT32 video_control_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *es = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    es = (struct pmt_es_info *)priv;
    es->video_control = (*desc|0x1);

    return SI_SUCCESS;
}
#endif

static INT32 iso639_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 i           = 0;
    UINT8 j           = 0;
    UINT8 k           = 0;
    UINT8 es_info_len = 0;
    UINT8 index       = 0;
#ifdef AUDIO_DESCRIPTION_SUPPORT
    UINT8 a_chan_cnt  = 0;
#endif
    struct pmt_es_info *es = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    es = (struct pmt_es_info *)priv;
    for( i = 0; i < len; i += 4)
    {
        es_info_len = k+3;
        if(es_info_len < ES_INFO_MAX_LANG_SIZE)
        {
            //MEMCPY(&es->lang[k], (desc + i), 3);
            for (j=i; j<(i+3); j++)
            {
                if ( (*(desc+j) >= 'a' && *(desc+j) <= 'z')|| (*(desc+j) >= 'A' && *(desc+j) <= 'Z'))
                {
                    es->lang[index++] = *(desc+j);
                }
                else
                {
                    es->lang[index++] = 'a';
                }
            }
#ifdef AUDIO_DESCRIPTION_SUPPORT
            if (a_chan_cnt == 0)
            {
                es->audio_type[a_chan_cnt++] = desc[i+3];
            }
#endif
            k += 3;
        }
        else
        {
            break;
        }
    }
    return SI_SUCCESS;
}

static INT32 loop1_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
#ifdef LEGACY_HOST_SUPPORT
    INT32 desc_offset = 4;
    UINT8 desc_tag    = 0;
    INT32 desc_len    = 0;
#endif
    PROG_INFO *info = (PROG_INFO *)priv;

    if(NULL == desc)
    {
        return !SI_SUCCESS;
    }

    if (info->ca_count < P_MAX_CA_NUM)
    {
        info->ca_info[info->ca_count].ca_system_id = (desc[0]<<8) |desc[1];
        info->ca_info[info->ca_count].ca_pid = ((desc[2]&0x1F)<<8) |desc[3];
        PP_PRINTF("PMT loop1 ca_desc_handler, ca_sysid=0x%x,pid=%d,CA_count=%d\n",
        info->ca_info[info->ca_count].ca_system_id,info->ca_info[info->ca_count].ca_pid,info->ca_count+1);

#ifdef LEGACY_HOST_SUPPORT
        while(desc_offset < len)
        {
            desc_tag = desc[desc_offset];
            desc_len = desc[desc_offset+1];
            if(0x8C != desc_tag)
            {
                desc_offset = desc_offset+2+desc_len;
            }
            else
            {
                info->hdplus_flag= 1;      //use the CA_pid bit15 to sign the HD+ service
                break;
            }
        }
#endif

        info->ca_count++;
    }
    else
    {
        PP_PRINTF("%s: ca count %d full!\n",__FUNCTION__,info->ca_count);
    }
    return SI_SUCCESS;
}

#ifdef PARENTAL_SUPPORT
static  INT32 loop1_pr_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
#if(PP_DEBUG_LEVEL > 0)    
    UINT8 country_code[3] = {0,0,0};

    if(NULL == desc)
    {
        return !SUCCESS;
    }

    country_code[0] = desc[0];
    country_code[1] = desc[1];
    country_code[2] = desc[2];
    g_pmt_rating=desc[3];
    PP_PRINTF("country_code: %c%c%c\n",country_code[0],country_code[1],country_code[2]);
    PP_PRINTF("rating: %d\n",rating);
#endif
    return SUCCESS;
}

UINT32 pmt_get_rating(void)
{
    return g_pmt_rating;
}
#endif

static INT32 loop2_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *info = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    info = (struct pmt_es_info *)priv;
    if (info->cas_count < P_MAX_CA_NUM)
    {
        info->cas_sysid[info->cas_count]= (desc[0]<<8) |desc[1];
        info->cas_pid[info->cas_count] = ((desc[2]&0x1F)<<8) |desc[3];
        PP_PRINTF("PMT loop2 ca_desc_handler, ca_sysid=0x%x,pid=%d,CA_count=%d\n",
            info->cas_sysid[info->cas_count],info->cas_pid[info->cas_count],info->cas_count+1);
        info->cas_count++;
    }
    else
    {
        PP_PRINTF("%s: ca count %d full!\n",__FUNCTION__,info->cas_count);
    }

    return SI_SUCCESS;
}

static INT32 com_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    UINT8 stream_content = 0;
    struct pmt_es_info *info = NULL;

    if((NULL == desc) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    info = (struct pmt_es_info *)priv;
    stream_content = desc[2]&0x0f;

    switch(stream_content)
    {
        case 0x01: // MPEG2 video
            break;
        case 0x02: // AAC MPEG2 audio
            {
                info->audio_com_tag = desc[4];
            }
            break;
        case 0x05: // H264/AVC video
            break;
        case 0x06: // HE-AAC MPEG4 audio
            {
                info->audio_com_tag = desc[4];
            }
            break;
        default:
            break;
    }

    return SI_SUCCESS;
}

#ifdef _MHEG5_SUPPORT_
PMT_ELEM_INFO mheg_es_info;
static INT32 stream_identifier_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *info = NULL;

    if(NULL == priv)
    {
        return SI_EINVAL;
    }
    info = (struct pmt_es_info *)priv;
    
    if(STREAM_IDENTIFIER_DESCRIPTOR == tag)
    {
      //  libc_printf("%s, com_tag:%d, es_pid: %d (0x%x)\n", __FUNCTION__, desc[0],info->pid,info->pid);
        MHEG_AV_SetESInfo(mheg_es_info.prog_number, info->pid, mheg_es_info.pcr_pid,desc[0]);
     #if 0
        info->es_info[info->elem_count].com_tag = desc[0];
        info->es_info[info->elem_count].es_pid = info->pid;
        //info->es_info.sid_desc[info->es_info.sid_info_cnt].pcr_pid = info->;
        info->elem_count++;
     #endif   
    }
    return SI_SUCCESS;
}
#endif

#if defined (_MHEG5_V20_ENABLE_)
static INT32 data_broadcastid_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *es = (struct pmt_es_info *)priv;
    UINT8 *data_boardcast_loop_start   = NULL;
    UINT8 *application_data_start_byte = NULL;
    UINT8 data_boardcast_loop_len      = 0;
    UINT8 application_data_length      = 0;
    UINT8 boot_priority_hint           = 0;
    UINT8 previous_hint                = 0;
    UINT16 data_broadcast_id           = 0;
    UINT16 reserved_id                 = 0x0106;

    if((NULL == desc) || (NULL == es))
    {
        return !SI_SUCCESS;
    }

    es->got_nb               = FALSE;
    es->boot_info_length     = 0;
    es->nb_version           = 0;
    es->nb_action            = 0xFF;
    es->boot_info_start_byte = NULL;

    data_boardcast_loop_start = desc + 2;
    data_boardcast_loop_len = len-2;
    data_broadcast_id = desc[0]<<8 | desc[1];

    if (reserved_id == data_broadcast_id)
    {
        es->stat |= MHEG5_BROADCAST_ID_EXIST;
    }

    if ((reserved_id != data_broadcast_id) || (0 == data_boardcast_loop_len))
    {
        return SI_SUCCESS;
    }

    while (data_boardcast_loop_len>0)
    {
        boot_priority_hint = data_boardcast_loop_start[2];
        application_data_length = data_boardcast_loop_start[3];

        if (0 == application_data_length)
        {
            break;
        }

        application_data_start_byte = data_boardcast_loop_start+4;
        if (boot_priority_hint >= previous_hint)
        {
            previous_hint = boot_priority_hint;
            if (0x1 == application_data_start_byte[0])
            {
                es->got_nb = TRUE;
                es->boot_info_length = application_data_start_byte[1] - 2;
                es->nb_version = application_data_start_byte[2];
                es->nb_action = application_data_start_byte[3];
                es->boot_info_start_byte = application_data_start_byte+4;
            }
        }

        data_boardcast_loop_start += (application_data_length+4);
        data_boardcast_loop_len -= (application_data_length+4);
    }
    return SI_SUCCESS;
}
#endif
#ifdef _MHEG5_SUPPORT_
static INT32 carousel_id_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *es = (struct pmt_es_info *)priv;

    if((NULL == desc) || (NULL == es))
    {
        return !SI_SUCCESS;
    }

    es->carousel_id = (desc[0]<<24)|(desc[1]<<16)|(desc[2]<<8)|desc[3];

    return SI_SUCCESS;
}

static INT32 data_broadcastid_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *es = (struct pmt_es_info *)priv;

    UINT8 *data_boardcast_loop_start   = NULL;
    UINT8 *application_data_start_byte = NULL;
    UINT8 data_boardcast_loop_len      = 0;
    UINT8 application_data_length      = 0;
    UINT8 boot_priority_hint           = 0;
    UINT8 previous_hint                = 0;
    UINT16 data_broadcast_id           = 0;
    UINT16 reserved_id                 = 0x0106;

    if((NULL == desc) || (NULL == es))
    {
        return !SI_SUCCESS;
    }

    es->got_nb               = FALSE;
    es->boot_info_length     = 0;
    es->nb_version           = 0;
    es->nb_action            = 0xFF;
    es->boot_info_start_byte = NULL;

    data_boardcast_loop_start = desc + 2;
    data_boardcast_loop_len = len-2;
    data_broadcast_id = desc[0]<<8 | desc[1];

    if (reserved_id == data_broadcast_id)
    {
        es->stat |= MHEG5_BROADCAST_ID_EXIST;
    }

    if ((reserved_id != data_broadcast_id) || (0==data_boardcast_loop_len))
    {
        return SI_SUCCESS;
    }

    while (data_boardcast_loop_len > 0)
    {
        boot_priority_hint = data_boardcast_loop_start[2];
        application_data_length = data_boardcast_loop_start[3];

        if (0 == application_data_length)
        {
            break;
        }

        application_data_start_byte = data_boardcast_loop_start+4;
        if (boot_priority_hint >= previous_hint)
        {
            previous_hint = boot_priority_hint;
            if (0x1==application_data_start_byte[0])
            {
                es->got_nb               = TRUE;
                es->boot_info_length     = application_data_start_byte[1] - 2;
                es->nb_version           = application_data_start_byte[2];
                es->nb_action            = application_data_start_byte[3];
                es->boot_info_start_byte = application_data_start_byte+4;
            }
        }

        data_boardcast_loop_start += (application_data_length+4);
        data_boardcast_loop_len   -= (application_data_length+4);
    }
    return SI_SUCCESS;
}
#endif

#ifdef AUTO_OTA

INT32 databroadcastid_deschandle(UINT8 tag, UINT8 len, UINT8 *pdata, void *ppriv)
{
    INT32  ret                 = 0;
    UINT8  min_data_len        = 8;
    UINT8  oui_data_len        = 0;
    UINT8  selector_len        = 0;
    UINT16 data_broadcast_id   = 0;
    UINT32 oui                 = 0;
    struct pmt_es_info *p_info = (struct pmt_es_info *)ppriv;
    AUTO_OTA_INFO *pota_info   = NULL;

    if((NULL == pdata) || (NULL == p_info))
    {
        return !SI_SUCCESS;
    }

    if(DATA_BROADCASTID_DES_TAG != tag)
    {
        ASSERT(0);
    }

    data_broadcast_id = (pdata[0]<<8 | pdata[1]);
    oui_data_len = pdata[2];
    oui = (pdata[3]<<16 | pdata[4]<<8 | pdata[5]);

    if((DATABROADCASTID_ALIOTA == data_broadcast_id) && (oui == si_get_otaconfig_oui()))
    {
        selector_len = pdata[8];
        pota_info = &p_info->pmt_ssu_info.m_otainfo;

        if(selector_len >= min_data_len)
        {
            pota_info->hw_model = pdata[9]<<8 | pdata[10];
            pota_info->hw_ver = pdata[11]<<8 | pdata[12];
            pota_info->sw_model = pdata[13]<<8 | pdata[14];
            pota_info->ota_sw_version = pdata[15]<<8 | pdata[16];
        }
        pota_info->oui = oui;
    }

    ret = SI_SUCCESS;
    return ret;
}
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT

static INT32 extension_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
    struct pmt_es_info *es = (struct pmt_es_info *)priv;

    if((NULL == desc) || (NULL == es))
    {
        return !SI_SUCCESS;
    }

    ad_debug("\t extension descriptor len: 0x%X\n", len);
    ad_debug("\t descriptor_tag_extension: 0x%X\n", desc[0]);

    if (desc[0] == SUPPLEMENTARY_AUDIO_DESCRIPTOR) // descriptor_tag_extension
    {
        es->sad_present = 1;
        es->sad_mix_type = (desc[1] >> 7) & 1;
        es->sad_editorial_classification = (desc[1] >> 2) & 0x1f;
        es->sad_language_code_present = desc[1] & 1;

        if (es->sad_language_code_present)
        {
            MEMCPY(es->sad_lang, &desc[2], ES_INFO_ONE_LANG_SIZE);
        }

#ifdef _AD_DEBUG
        ad_debug("\t Supplementary audio descriptor\n");
        ad_debug("\t\t mix_type: 0x%X\n", (desc[1] >> 7) & 1);
        ad_debug("\t\t editorial_classification: 0x%X\n", (desc[1] >> 2) & 0x1f);
        ad_debug("\t\t reserved: %d\n", (desc[1] >> 1) & 1);
        ad_debug("\t\t language_code_present: %d\n", desc[1] & 1);
        if (es->sad_language_code_present)
        {
            ad_debug("\t\t ISO_639_language_code 0: 0x%X(%c)\n", desc[2], desc[2]);
            ad_debug("\t\t ISO_639_language_code 1: 0x%X(%c)\n", desc[3], desc[3]);
            ad_debug("\t\t ISO_639_language_code 2: 0x%X(%c)\n", desc[4], desc[4]);
        }
#endif
    }

    return SI_SUCCESS;
}

static int audio_info_amend(struct pmt_es_info *es)
{
    if ((es == NULL) || !es->sad_present)
    {
        return 0;
    }
    ad_debug("audio info before amend: %c%c%c, 0x%X, %d, %d\n",
        es->lang[0], es->lang[1], es->lang[2], es->audio_type[0],
        es->sad_mix_type, es->sad_editorial_classification);

    if (es->sad_language_code_present)
    {
        MEMCPY(es->lang, es->sad_lang, ES_INFO_ONE_LANG_SIZE);
    }

    if (es->sad_mix_type == 0)
    {
        if (1 == es->sad_editorial_classification)
        {
#ifdef _AD_DEBUG
            if (0x03 != es->audio_type[0])
            {
                ad_error("Inconsistent audio info\n");
            }
#endif
            es->audio_type[0] = AUDIO_TYPE_AD_RECEIVER_MIXED;
        }
    }
    else if (1 == es->sad_mix_type)
    {
        switch (es->sad_editorial_classification)
        {
            case 0:
#ifdef _AD_DEBUG
                if ((es->audio_type[0] != 0x00) && (es->audio_type[0] != 0x01))
                {
                    ad_error("Inconsistent audio info\n");
                }
#endif
                es->audio_type[0] = AUDIO_TYPE_MAIN_AUDIO;
                break;
            case 1:
#ifdef _AD_DEBUG
                if ((0x00 != es->audio_type[0]) && (0x01 != es->audio_type[0]) && (0x03 != es->audio_type[0]))
                {
                    ad_error("Inconsistent audio info\n");
                }
#endif
                es->audio_type[0] = AUDIO_TYPE_AD_BROADCAST_MIXED;
                break;
            case 2:
#ifdef _AD_DEBUG
                if (0x02 != es->audio_type[0])
                {
                    ad_error("Inconsistent audio info\n");
                }
#endif
                es->audio_type[0] = AUDIO_TYPE_CLEAN_AUDIO;
                break;
            case 3:
#ifdef _AD_DEBUG
                if ((0x00 != es->audio_type[0]) && (0x01 != es->audio_type[0]) && (0x03 != es->audio_type[0]))
                {
                    ad_error("Inconsistent audio info\n");
                }
#endif
                es->audio_type[0] = AUDIO_TYPE_SPOKEN_SUBTITLE;
                break;
            default:
                break;
        }
    }

    ad_debug("audio info after amend:  %c%c%c, 0x%X, %d, %d\n",
        es->lang[0], es->lang[1], es->lang[2], es->audio_type[0],
        es->sad_mix_type, es->sad_editorial_classification);

    return 0;
}

#endif

static INT32 registion_deschandle(UINT8 tag, UINT8 len, UINT8 *pdata, void *ppriv)
{
    INT32  ret                 = 0;
    struct pmt_es_info *p_info = (struct pmt_es_info *)ppriv;

    if((NULL == pdata) || (NULL == p_info))
    {
        return !SI_SUCCESS;
    }

    if(REGISTRATION_DESCRIPTOR != tag)
    {
        return SI_SUCCESS;
    }

	if(len<4)	return SI_SUCCESS;

	//memcmp
	if(MEMCMP(pdata,FORTMAT_IDENTIFIER_AC3,4)==0)
	{
		p_info->stat |= ES_AC3_EXIST;
	}
	else if(MEMCMP(pdata,FORTMAT_IDENTIFIER_HEVC,4)==0)
	{
		p_info->stat |= ES_HEVC_VIDEO_EXIST;
	}


    ret = SI_SUCCESS;
    return ret;
}
static BOOL append_audio_pid(struct pmt_es_info *es, PROG_INFO *info, UINT16 attr)
{
    BOOL   ret       = FALSE;
    UINT32 audio_cnt = 0;
    UINT8  *lang     = es->lang;

    if(NULL == info)
    {
        return FALSE;
    }

#ifdef AUDIO_DESCRIPTION_SUPPORT
    audio_info_amend(es);
#endif

    audio_cnt = info->audio_count;
    if(audio_cnt < P_MAX_AUDIO_NUM)
    {
#ifdef CONAX_AUDIO_LAN_USE
        if (lang[0]&&lang[1]&&lang[2])
        {
            MEMCPY(info->audio_lang[audio_cnt], lang, 3);
            info->audio_pid[audio_cnt] = es->pid|attr;
            audio_cnt ++;
            info->audio_count = audio_cnt;
        }
        else
        {
            MEMSET(info->audio_lang[audio_cnt], 0, 3);
        }

        if (lang[3]&&lang[4]&&lang[5])
        {
            MEMCPY(info->audio_lang[audio_cnt], (lang + 3), 3);
            info->audio_pid[audio_cnt] = es->pid|attr;
            audio_cnt ++;
            info->audio_count = audio_cnt;
        }
        else
        {
            MEMSET(info->audio_lang[audio_cnt], 0, 3);
        }

        if ((lang[0]&&lang[1]&&lang[2]) == 0)
        {
            info->audio_pid[audio_cnt] = es->pid|attr;
            info->audio_count = audio_cnt+1;
        }
#else
        info->audio_pid[audio_cnt] = es->pid|attr;
        info->audio_com_tag[audio_cnt] = es->audio_com_tag;

        if (lang[0]&&lang[1]&&lang[2])
        {
            MEMCPY(info->audio_lang[audio_cnt], lang, 3);
        }
        else
        {
            MEMSET(info->audio_lang[audio_cnt], 0, 3);
        }

    #ifdef AUDIO_DESCRIPTION_SUPPORT
        info->audio_type[audio_cnt] = es->audio_type[0];
    #endif

    #ifdef NL_BZ
        if (lang[3]&&lang[4]&&lang[5])
        {
            MEMCPY(info->audio_lang[audio_cnt + 3], (lang + 3), 3);
        }
        else
        {
            MEMSET(info->audio_lang[audio_cnt + 3], 0, 3);
        }
    #endif

        info->audio_count = audio_cnt+1;

    #ifdef AUDIO_CHANNEL_LANG_SUPPORT
        if ((info->audio_count < P_MAX_AUDIO_NUM) && (lang[3] && lang[4] && lang[5]))
        {   // add one more audio pid for the channel language
            MEMCPY(info->audio_lang[info->audio_count], (lang + 3), 3);
            info->audio_pid[info->audio_count] = es->pid | attr;
        #ifdef AUDIO_DESCRIPTION_SUPPORT
            info->audio_type[info->audio_count] = es->audio_type[0];
        #endif
            info->audio_count++;
        }
    #endif
#endif
        PP_PRINTF("+aud_pid=%d\n", es->pid);
        ret = TRUE;
    }
    else
    {
        PP_PRINTF("-aud_pid=%d\n", es->pid);
        ret = FALSE;
    }
    PP_PRINTF("lang code: %c%c%c\n", lang[0], lang[1], lang[2]);

#ifdef AUDIO_DESCRIPTION_SUPPORT
    ad_debug("%s() audio type: 0x%X\n", __FUNCTION__, es->audio_type[0]);
#endif

    return ret;
}

static void process_each_typeof_stream(struct pmt_es_info *es, PROG_INFO *p_info, UINT16 *flag)
{
    UINT16 invalid_flag = *flag;

    if((NULL == es) || (NULL == p_info))
    {
        return ;
    }

    switch(es->stream_type)
    {
        case MPEG1_AUDIO_STREAM:
        case MPEG2_AUDIO_STREAM:
            if (TRUE == append_audio_pid(es, p_info, 0))
            {
                invalid_flag = 0;
            }
            break;
#ifdef ATSC_SUBTITLE_SUPPORT
        case ATSC_SUBT_STREAM:
            if (PSI_STUFF_PID == p_info->subtitle_pid)
            {
                p_info->subtitle_pid = es->pid;
                invalid_flag = 0;
            }
            break;
#endif
        case PRIVATE_DATA_STREAM:
            if(es->stat&ES_TTX_EXIST)
            {
                p_info->teletext_pid = es->pid;
            }
            else if(es->stat&ES_SUB_EXIST)
            {
                p_info->subtitle_pid = es->pid;
            }
#ifdef AC3DEC
            else if((es->stat&ES_AC3_EXIST)&&(TRUE == append_audio_pid(es, p_info,AC3_DES_EXIST)))
            {
                invalid_flag = 0;
            }
            #if 1
            else if((es->stat&ES_EAC3_EXIST)&&(TRUE == append_audio_pid(es, p_info,EAC3_DES_EXIST)))
            {
                invalid_flag = 0;
            }
            #endif
#endif
			else if((es->stat&ES_HEVC_VIDEO_EXIST))
            {
#ifdef ITALY_HD_BOOK_SUPPORT
				if(PSI_STUFF_PID == p_info->video_pid_array[p_info->video_cnt])
				{
					p_info->av_flag = 1;
					p_info->video_pid_array[p_info->video_cnt] = es->pid|H265_VIDEO_PID_FLAG;
					invalid_flag = 0;
					p_info->video_cnt++;
				}
#else
				if (PSI_STUFF_PID == p_info->video_pid)
				{
					p_info->av_flag = 1;
					p_info->video_pid = es->pid|H265_VIDEO_PID_FLAG;
					invalid_flag = 0;
				}
#endif
            }
            break;
#ifdef AC3DEC
        case AC3_AUDIO_STREAM:
        case EAC3_AUDIO_STREAM:
            if(TRUE == append_audio_pid(es, p_info,AC3_DES_EXIST))
            {
                invalid_flag = 0;
            }
            break;
        case MPEG_AAC_STREAM:
            if(TRUE == append_audio_pid(es, p_info,AAC_DES_EXIST))
            {
                invalid_flag = 0;
            }
            break;
        case MPEG_ADTS_AAC_STREAM:
            if(TRUE == append_audio_pid(es, p_info,ADTS_AAC_DES_EXIST))
            {
                invalid_flag = 0;
            }
            break;
#endif
        case MPEG1_VIDEO_STREAM:
        case MPEG2_VIDEO_STREAM:
        case MPEG4_VIDEO_STREAM:
    #ifdef ITALY_HD_BOOK_SUPPORT
            if(PSI_STUFF_PID == p_info->video_pid_array[p_info->video_cnt])
            {
                p_info->av_flag = 1;
                p_info->video_pid_array[p_info->video_cnt] = es->pid;
                invalid_flag = 0;
                p_info->video_cnt++;
            }
    #else
            if (PSI_STUFF_PID == p_info->video_pid)
            {
                if(MPEG4_VIDEO_STREAM == es->stream_type)
                {
                    p_info->mpeg_4_video_exist = 1;
                }
                p_info->av_flag = 1;
                p_info->video_pid = es->pid;
                invalid_flag = 0;
            }
    #endif
            break;
        case H264_VIDEO_STREAM:
    #ifdef ITALY_HD_BOOK_SUPPORT
            if(PSI_STUFF_PID == p_info->video_pid_array[p_info->video_cnt])
            {
                p_info->av_flag = 1;
                p_info->video_pid_array[p_info->video_cnt] = es->pid|H264_VIDEO_PID_FLAG;
                invalid_flag = 0;
                p_info->video_cnt++;
            }
    #else
            if (PSI_STUFF_PID == p_info->video_pid)
            {
                p_info->av_flag = 1;
                p_info->video_pid = es->pid|H264_VIDEO_PID_FLAG;
                invalid_flag = 0;
            }
    #endif
            break;
		case H265_VIDEO_STREAM:
#ifdef ITALY_HD_BOOK_SUPPORT
			if(PSI_STUFF_PID == p_info->video_pid_array[p_info->video_cnt])
			{
				p_info->av_flag = 1;
				p_info->video_pid_array[p_info->video_cnt] = es->pid|H265_VIDEO_PID_FLAG;
				invalid_flag = 0;
				p_info->video_cnt++;
			}
#else
			if (PSI_STUFF_PID == p_info->video_pid)
			{
				p_info->av_flag = 1;
				p_info->video_pid = es->pid|H265_VIDEO_PID_FLAG;
                invalid_flag = 0;
            }
#endif
            break;
#ifdef ATSC_SUBTITLE_SUPPORT
        case DC_II_VIDEO_STREAM:
            if ((PSI_STUFF_PID == p_info->video_pid)&&(es->stat&ES_EXTENDED_VIDEO_DESC_EXIST))
            {
                p_info->av_flag = 1;
                p_info->video_pid = es->pid;
                invalid_flag = 0;
            }
            break;
#endif
#if(defined (_MHEG5_SUPPORT_) || defined (_MHEG5_V20_ENABLE_))
        case ISO13818_6_TYPE_B:
        case ISO13818_6_TYPE_D:
            if (0 != (es->stat&MHEG5_BROADCAST_ID_EXIST))
            {
                p_info->av_flag = 1;    /* set to be TV prog */
                p_info->mheg5_exist = 1;
                invalid_flag = 0;
            }
            break;
        case 0x05:
            invalid_flag = 0;
            break;
#endif

#ifdef SCTE_SUBTITLE_SUPPORT
        case 0x82:
            for(k=0; k<p_info->scte_subt_count; k++)
            {
                if(es->pid == p_info->scte_subt_pid[k])
                {
                    break;
                }
            }
            if(k == p_info->scte_subt_count)
            {
                p_info->scte_subt_pid[k] = es->pid;
                p_info->scte_subt_count++;
            }
            //info->descriptor: tag(1B), len(1B), lan(3B), unknow(1B)
            if((0xA == info->descriptor)[0] && (info->descriptor)[1] >= 3)
            {
                p_info->scte_subt_lan_cnt[k] =1;
                p_info->scte_subt_lan[k][0] = (((info->descriptor)[2])<<16)|
                    (((info->descriptor)[3])<<8)|((info->descriptor)[4]);
            }
            break;
#endif
        default:
            PP_PRINTF("es_type = 0x%x unknown!\n",__FUNCTION__, es->stream_type);
            break;
    }

    *flag = invalid_flag;
}

static INT32 pmt_parse_loop2(UINT8 *pmt, UINT16 pmt_len, PROG_INFO *p_info,INT32 max_es_nr)
{
#ifdef SCTE_SUBTITLE_SUPPORT
    UINT8  k                     = 0;
#endif
    INT32  ret                   = 0;
    INT32  es_cnt                = 0;
    UINT32 i                     = 0;
    UINT32 j                     = 0;
    UINT32 loop_len              = 0;
    UINT32 loop1_len             = 0;
    UINT16 invalid_flag          = 1;
    struct pmt_stream_info *info = NULL;
    struct pmt_es_info es;

    MEMSET(&es, 0x0, sizeof (struct pmt_es_info));
#if 0//def _MHEG5_SUPPORT_
    UINT16 boot_carousel_pid     = 0;
    UINT32 carousel_id           = 0;
    BOOL   reboot                = TRUE;

    mheg_get_boot_carousel_query_para(&boot_carousel_pid,&carousel_id);
#endif
    if((NULL == pmt) || (NULL == p_info))
    {
        return !SI_SUCCESS;
    }

    loop1_len = ((pmt[10]<<8)|pmt[11])&0x0FFF;

    PP_PRINTF("\n %s(): es stream :\n",__FUNCTION__);
    for(es_cnt=0, i = sizeof(struct pmt_section)-4+loop1_len;i < (UINT32)pmt_len-4;
        i += sizeof(struct pmt_stream_info)+loop_len)
    {
        info = (struct pmt_stream_info *)(pmt+i);
        MEMSET(&es, 0, sizeof(struct pmt_es_info));
        es.stream_type = info->stream_type;
        es.pid = SI_MERGE_HL8(info->elementary_pid);
        loop_len = SI_MERGE_HL8(info->es_info_length);

        ret = desc_loop_parser(info->descriptor, loop_len, pmt_loop2, ARRAY_SIZE(pmt_loop2), &es.stat, (void *)&es);

        if (ret != SI_SUCCESS)
        {
            PP_PRINTF("%s: desc_loop_parser error, error= %d\n", ret);
        }
#ifdef AUTO_OTA
//      if((ISO_13818_6_TYPE_B==pbuf[0])
        if((ISO13818_6_TYPE_B == es.stream_type)
            &&(0 != ((1<<ES_DTS_EXIST_BIT)&es.stat))
            && (es.pmt_ssu_info.m_otainfo.oui == si_get_otaconfig_oui()
//          && (get_ota_upgrade_found() == FALSE)
            ))
        {//if stream type is ISO_13818_6_TYPE_B, data broadcast id desc is exist and oui is match
            p_info->p_esloopinfo.pmt_ssu_info.stream_type = es.stream_type;
            p_info->p_esloopinfo.pmt_ssu_info.ssu_pid = es.pid;
            p_info->p_esloopinfo.pmt_ssu_info.m_otainfo.oui = es.pmt_ssu_info.m_otainfo.oui;
            ret = SI_SUCCESS;
            invalid_flag = 0;
            break;
        }
#endif

        if (es.stat & ES_CA_EXIST)
        {
            //check if current es ca system is same with those recorded ones
            for(j=0;j<es.cas_count;j++)
            {
            if (p_info->ca_count < P_MAX_CA_NUM)
            {
                p_info->ca_info[p_info->ca_count].ca_system_id = es.cas_sysid[j];
                p_info->ca_info[p_info->ca_count].ca_pid = es.cas_pid[j];
                p_info->ca_count++;
            }
            else
            {
                PP_PRINTF("pmt_parse_loop2: ca system id count %d full!\n",p_info->ca_count);
                break;
            }
            }
        }
        if ((es.stat&ES_AVC_VIDEO_EXIST) && (0x02 != es.stream_type))
        {
            p_info->avc_video_exist = 1;
        }

        if (es.stat&ES_MPEG4_VIDEO_EXIST)
        {
            p_info->mpeg_4_video_exist = 1;
        }

#ifdef RAPS_SUPPORT
    if((p_info->prog_number == raps_get_sevice_id()) && (p_info->pmt_pid == raps_get_pmt_pid()))
    {
        raps_set_data_pid(es.pid);
    }
#endif

#ifdef VIDEO_SEAMLESS_SWITCHING
        if (es.stat&ES_V_CTRL_EXIST_BIT)
        {
            p_info->video_control = es.video_control;
        }
#endif

        PP_PRINTF("es stream type=%d\n",es.stream_type);
		if((es.stat & ES_AC3_EXIST) && (es.stat & ES_EAC3_EXIST))
			es.stat = es.stat & (~ES_AC3_EXIST);

        process_each_typeof_stream(&es, p_info, &invalid_flag);

        if (++es_cnt>=max_es_nr)
        {
            PP_PRINTF("es_cnt = %d exceed max_es_nr!\n",__FUNCTION__, es_cnt);
            break;
        }

#if 0 //def _MHEG5_SUPPORT_
        if (((es.stat&ES_CAROUSEL_ID_EXIST) != 0)
            && (carousel_id != 0xffffffff)
            && (es.carousel_id == carousel_id)
            && (es.stat&ES_DATA_BROADCASTID_EXIST) != 0)
        {
            reboot = FALSE;
        }

        if (((es.stat&ES_DATA_BROADCASTID_EXIST) != 0) && es.got_nb && check_nb_version((UINT16)es.nb_version))
        {
            clear_boot_info();
            save_boot_info(es.boot_info_length, es.boot_info_start_byte, es.nb_action);
        }
#endif
    }

#if 0//def _MHEG5_SUPPORT_
    if (reboot && (0xffffffff != carousel_id) && (0xffff != boot_carousel_pid))
    {
        tm_mheg5re_start_cur_ch();
    }
#endif

    PP_PRINTF("%s: es_cnt = %d!\n\n",__FUNCTION__, es_cnt);
    return invalid_flag ? ERR_NO_PROG_INFO : SI_SUCCESS;
}


INT32 psi_pmt_parser(UINT8 *pmt, PROG_INFO *p_info, INT32 max_es_nr)
{
    INT32  ret  = 0;
    UINT32 stat = 0;
    UINT16 len  = 0;

    if((NULL == pmt) || (NULL == p_info))
    {
        return !SI_SUCCESS;
    }

    p_info->pcr_pid = ((pmt[8]<<8)|pmt[9])&PSI_STUFF_PID;
#ifdef ITALY_HD_BOOK_SUPPORT
    UINT8 i = 0;

    for(i=0; i<MAX_VIDEO_CNT; i++)
    {
        p_info->video_pid_array[i] = PSI_STUFF_PID;
    }
    p_info->video_cnt = 0;
    p_info->cur_video_pid_idx = 0;
#else
    p_info->video_pid = PSI_STUFF_PID;
#endif
    p_info->service_type = 0;
    p_info->pmt_version  = (pmt[5]&0x3e)>>1;
    p_info->audio_count  = 0;
	
#ifdef _MHEG5_SUPPORT_
	mheg_es_info.prog_number = p_info->prog_number;
	mheg_es_info.pcr_pid = p_info->pcr_pid;
#endif	

    desc_loop_parser(pmt+12, ((pmt[10]<<8)|pmt[11])&0xFFF, pmt_loop1, ARRAY_SIZE(pmt_loop1), &stat, p_info);

    len = ((pmt[1]<<8)|pmt[2])&0x0FFF;
    ret = pmt_parse_loop2(pmt, len, p_info, max_es_nr);
    if(PSI_STUFF_PID == p_info->video_pid)
    {
        p_info->video_pid = 0;
    }

    PP_PRINTF("PMT parse ret=%d, prog num=%d, pmt pid=%d,ca count=%d,av flag=%d\n", ret,p_info->prog_number,
    p_info->pmt_pid, p_info->ca_count,p_info->av_flag);

    return ret;
}

#ifdef LIB_TSI3_FULL
void psi_pmt_get_ecm_pid(UINT8 *section, INT32 length, UINT16 param)
{
    UINT8  *discriptor_pointer  = NULL;
    UINT8  *discriptor_pointer2 = NULL;
    UINT8  loop_length          = 0;
    UINT8  max_ecm_count        = 32;
    UINT16 prog_info_len        = 0;
    UINT16 section_len          = 0;
    UINT16 es_info_len          = 0;
    UINT16 left_len             = 0;
    UINT16 ecm_pid              = 0;
    UINT32 crc_value            = 0;

    if(NULL == section)
    {
        return ;
    }

    section_len = ((section[1] & 0x03) << 8)|section[2];

    crc_value = (section[section_len-1]|(section[section_len]<<8)|
        (section[section_len+1]<<16)|(section[section_len+2]<<24));
    if((crc_value == psi_pmt_crc_value) && (ecm_count != 0))
    {
        return;
    }

    ecm_count = 0;

    prog_info_len = ((section[10] & 0x03) << 8)|section[11];
    left_len = section_len -9 -prog_info_len -4;
    discriptor_pointer = &section[12];
    discriptor_pointer2 = discriptor_pointer + prog_info_len;

    while (prog_info_len > 0)
    {
        /*the CA descriptor tag*/
        if (0x09 == *discriptor_pointer)
        {
            ecm_pid = ((discriptor_pointer[4]&0x1F)<<8)|discriptor_pointer[5];

            psi_pmt_crc_value = crc_value;
            if(ecm_count < max_ecm_count)
            {
                ecm_pid_array[ecm_count] = ecm_pid;
                ecm_count++;
            }
            else
            {
                libc_printf("ecm_count: %d, count > 32 \n",ecm_count);
            }

        }
        prog_info_len -= 2+discriptor_pointer[1];
        discriptor_pointer += 2+discriptor_pointer[1];
    }
    discriptor_pointer = discriptor_pointer2;

    while(left_len > 0)
    {
        es_info_len = ((discriptor_pointer[3]&0x03)<<8) | discriptor_pointer[4];
        loop_length = es_info_len;
        discriptor_pointer += 5;

        while(loop_length > 0)
        {
            /*the CA descriptor tag*/
            if (0x09 == *discriptor_pointer)
            {
                ecm_pid = ((discriptor_pointer[4]&0x1F)<<8)|discriptor_pointer[5];
                psi_pmt_crc_value = crc_value;

                if(ecm_count < max_ecm_count)
                {
                    ecm_pid_array[ecm_count] = ecm_pid;
                    ecm_count++;
                }
                else
                {
                    libc_printf("ecm_count: %d, count > 32 \n",ecm_count);
                }
            }
            loop_length -= 2+discriptor_pointer[1];
            discriptor_pointer += 2+discriptor_pointer[1];
        }
        left_len -= 5+es_info_len;
    }

    return;
}

#endif

#ifdef AUTO_OTA
INT32 get_ssuinfo_from_pmt(struct program_map *map, UINT16 *pssu_pid, UINT16 pmtpid)
{
    INT32  ret    = !SI_SUCCESS;
    INT32  otaret = -1;
    UINT32 i      = 0;
    PROG_INFO *pmt_section_info = NULL;
    AUTO_OTA_INFO *pts_ota_info = NULL;
    struct section_param sec_param;

    if((NULL == map) || (NULL == pssu_pid))
    {
        return !SI_SUCCESS;
    }

//  for(i=0; i< psi_info->pmt_counter; i++)
    {
        pmt_section_info = &map->pm_info;
        pts_ota_info = &map->pm_info.p_esloopinfo.pmt_ssu_info.m_otainfo;

        if(0 != pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid)
        {
            if(si_get_otaconfig_oui() == pmt_section_info->p_esloopinfo.pmt_ssu_info.m_otainfo.oui)
            {
#ifdef DVBS_SUPPORT
                if(si_get_ota_ssupid_s() != pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid)
                {
                    si_set_ota_ssupid_s(pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid);
                }
#endif
#if(defined( DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
                if(si_get_ota_ssupid_t() != pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid)
                {
                    si_set_ota_ssupid_t(pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid);
                }
#endif
                MEMSET(&sec_param, 0, sizeof(struct section_param));
                sec_param.section_type = 0x00;
                otaret = si_private_sec_parsing_start(0, pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid,
                    parse_dsmcc_sec, (void*)(&sec_param));
                if(otaret == OTA_SUCCESS)
                {
                    *pssu_pid = pmt_section_info->p_esloopinfo.pmt_ssu_info.ssu_pid;
                    ret = SI_SUCCESS;
                }
            }
        }
    }
    return ret;
}
#endif

