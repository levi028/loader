/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_name_extra.c
*
*    Description:   The internal function of program name
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include "win_prog_name.h"
#include "string.id"
#include "images.id"
#include "copper_common/com_epg.h"
#include "copper_common/system_data.h"

#define ASCII_TOP_HEX 0x80
#if (ISDBT_CC == 1)
extern void isdbtcc_get_language(struct t_isdbtcc_lang** list ,UINT8* num);
#endif
static void get_proper_string_for_epg_now(UINT16 cur_channel, char *string, int str_size, prog_txt_t *p_prog_txt)
{
    eit_event_info_t *pe = NULL;
    UINT8 *s1 = NULL;
    INT32 len = 0;
    TEXT_FIELD *txt = NULL;

    txt = &prog_text;

    pe = epg_get_cur_service_event((INT32) cur_channel, PRESENT_EVENT, NULL, NULL, NULL, TRUE);

    if (FALSE == win_prog_is_epg_update())
    {
        if (NULL == pe)
        {
            win_prog_set_epg_update(TRUE);
        }
    }
    else
    {
        if (NULL != pe)
        {
            win_prog_set_epg_update(FALSE);
        }
    }

    s1 = epg_get_event_name(pe, &len);

    if (s1)
    {
        strncpy(string,"Now: ", str_size-1);
        com_asc_str2uni((UINT8 *)string, (UINT16 *)len_display_str);
        len =com_uni_str_len(len_display_str);
        com_uni_str_copy_char_n((UINT8*)&len_display_str[len],s1, MAX_LEN_DISP_STR-len);
    }
    else
    {
        osd_set_text_field_str_point(txt,NULL);
        osd_set_text_field_content(txt,STRING_ID,RS_EPG_NO_INFORMATION);
    }
}

static void get_proper_string_for_epg_next(UINT16 cur_channel, char *string, int str_size)
{
    eit_event_info_t *fe = NULL;
    UINT8 *s2 = NULL;
    INT32 event_name_len = -1;
    UINT32 uni_len = 0;
    BOOL event_update = FALSE;

    event_update = win_prog_is_epg_update();

    fe = epg_get_cur_service_event((INT32) cur_channel,FOLLOWING_EVENT, NULL, NULL, NULL, event_update);

    if (FALSE == event_update)
    {
        if (NULL == fe)
        {
            win_prog_set_epg_update(TRUE);
        }
    }
    else
    {
        if (NULL != fe)
        {
            win_prog_set_epg_update(FALSE);
        }
    }

    s2 = epg_get_event_name(fe, &event_name_len);

    if (s2)
    {
        snprintf(string,str_size, "Next: ");
        com_asc_str2uni((UINT8 *)string, (UINT16 *)len_display_str);
        uni_len =com_uni_str_len(len_display_str);
        com_uni_str_copy_char_n((UINT8*)&len_display_str[uni_len],s2, MAX_LEN_DISP_STR-uni_len);
    }
    else
    {
        snprintf(string, str_size, " ");
        com_asc_str2uni((UINT8 *)string, (UINT16 *)len_display_str);
    }
}


UINT16 get_cur_icon(UINT16 cur_channel, P_NODE *p_node, prog_bmp_t *p_prog_bmp, UINT8 av_mode)
{
#if ((1==SUBTITLE_ON )||(1==ISDBT_CC ))
    struct t_ttx_lang *ttx_lang_list = NULL;
    UINT8 ttx_lang_num = 0;
    struct t_subt_lang *sub_lang_list = NULL;
    UINT8 sub_lang_num = 0;
#endif
#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang *cclanglst;
#endif
    UINT16 icon = INVALID_ID;
    UINT32 fav_mask = 0;
    eit_event_info_t *pe = NULL;
    eit_event_info_t *fe = NULL;
    UINT32 i = 0;

    for (i = 0; i < MAX_FAVGROUP_NUM; i++)
    {
        fav_mask |= (0x01 << i);
    }

    switch (p_prog_bmp->bmp_type)
    {
    case PROG_BMP_STATIC:
        icon = p_prog_bmp->icon;
        break;
    case PROG_BMP_AV_TYPE:
        if(av_mode==TV_CHAN)
        {
            icon = IM_INFORMATION_TV;
        }
        else
        {
            icon = IM_INFORMATION_RADIO;
        }      
        break;
    case PROG_BMP_EPG:
        pe = epg_get_cur_service_event((INT32) cur_channel, PRESENT_EVENT, NULL, NULL, NULL, TRUE);
        fe = epg_get_cur_service_event((INT32) cur_channel,FOLLOWING_EVENT, NULL, NULL, NULL, TRUE);

        if ((pe || fe))
        {
            icon = p_prog_bmp->icon;
        }
        break;
#if (TTX_ON == 1)
    case PROG_BMP_TTX:
        ttxeng_get_init_lang(&ttx_lang_list, &ttx_lang_num);
        if(ttx_lang_num > NUM_ZERO)
        {
            icon = p_prog_bmp->icon;
        }
        break;
#endif
#if ((SUBTITLE_ON == 1)||(ISDBT_CC == 1))
    case PROG_BMP_SUBT:
#if (SUBTITLE_ON == 1)
        subt_get_language(&sub_lang_list, &sub_lang_num);
        if (0 == sub_lang_num)
        {
            ttxeng_get_subt_lang(&ttx_lang_list, &sub_lang_num);
        }
#endif

#if (ISDBT_CC == 1)
        if (0 == sub_lang_num)
        {
            isdbtcc_get_language(&cclanglst, &sub_lang_num);
        }
#endif
        if(sub_lang_num > NUM_ZERO)
        {
            icon = p_prog_bmp->icon;
        }
        break;
#endif
    case PROG_BMP_LOCK:
        if (p_node->lock_flag)
        {
            icon = p_prog_bmp->icon;
        }
        break;
    case PROG_BMP_FAV:
        if (p_node->fav_group[0] & fav_mask)
        {
            icon = p_prog_bmp->icon;
        }
        break;
    case PROG_BMP_CA:
    {
        if (p_node->ca_mode)
        {
            icon = p_prog_bmp->icon;
        }
    }
    break;
    default:
        break;
    }

    return icon;
}


void win_progname_get_proper_string(UINT16 cur_channel, char *string, int str_size, prog_txt_t *p_prog_txt,
    S_NODE *s_node, UINT16 *group_name)
{
    INT32 strlen = 0;
    T_NODE t_node;
    P_NODE p_node;
    date_time dt;
#ifndef DB_USE_UNICODE_STRING
    UINT8 j = 0;
    char prog_name[MAX_SERVICE_NAME_LENGTH + 1] = { 0 };
#else
    int  __MAYBE_UNUSED__ str_cpy_ret = 0;
#endif
    UINT32  __MAYBE_UNUSED__ str_ret = 0;
    int  __MAYBE_UNUSED__ sprintf_ret = 0;
    UINT32 param = 0;
    UINT8 frontend_kind = 0;
    RET_CODE  __MAYBE_UNUSED__ deca_ret = RET_SUCCESS;
    struct deca_device *g_deca_dev = NULL;

    MEMSET(&dt, 0x0, sizeof (date_time));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&p_node, 0x0, sizeof (p_node));

    get_local_time(&dt);
    get_prog_at(cur_channel, &p_node);
    get_tp_by_id(p_node.tp_id, &t_node);


    switch (p_prog_txt->txt_type)
    {
    case PROG_TXT_PROG_NAME:
        MEMSET(string, 0x0, str_size);
#ifndef DB_USE_UNICODE_STRING
            strncpy(prog_name,p_node.service_name, MAX_SERVICE_NAME_LENGTH);
            for(j=0;j<STRLEN(prog_name);j++)
                if(prog_name[j] >= ASCII_TOP_HEX)
                {
                    prog_name[j] = '\0';
                    break;
                }
        #if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
            if(0==p_node.ca_mode)
                snprintf(string,str_size, "%s(%s)",prog_name,s_node.sat_name);
            else
                snprintf(string, str_size, "$%s(%s)",prog_name,s_node.sat_name);
        #else
            if(0==p_node.ca_mode)
                snprintf(string, str_size, "%s",prog_name);
            else
                snprintf(string, str_size, "$%s",prog_name);
        #endif

#else
        if (NUM_ZERO == p_node.ca_mode)
        {
            strncpy(string, "", str_size-1);
        }
        else
        {
            strncpy(string, "$", str_size-1);
        }

        str_ret = com_asc_str2uni((UINT8 *)string, len_display_str);
        strlen = com_uni_str_len(len_display_str);
        str_cpy_ret = com_uni_str_copy_char_n((UINT8 *)&len_display_str[strlen], p_node.service_name, MAX_LEN_DISP_STR-strlen);
        strlen = com_uni_str_len(len_display_str);

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS    || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
        str_ret = com_asc_str2uni((UINT8 *)"(", &len_display_str[strlen]);
        strlen = com_uni_str_len(len_display_str);
        str_cpy_ret = com_uni_str_copy_char_n((UINT8 *) & len_display_str[strlen], s_node->sat_name, MAX_LEN_DISP_STR-strlen);
        strlen = com_uni_str_len(len_display_str);
        str_ret = com_asc_str2uni((UINT8 *)")", &len_display_str[strlen]);
#endif
#endif
        break;
    case PROG_TXT_DATE:
        sprintf_ret = snprintf(string, str_size, "%02d/%02d", dt.month, dt.day);
        str_ret = com_asc_str2uni((UINT8 *)string, (UINT16 *) len_display_str);
        break;
    case PROG_TXT_TIME:
        sprintf_ret = snprintf(string, str_size, "%02d:%02d", dt.hour, dt.min);
        str_ret = com_asc_str2uni((UINT8 *)string, (UINT16 *) len_display_str);
        strncpy(txt_string_time, string, (80-1));
        txt_string_time[79] = 0;
        break;
    case PROG_TXT_PROG_NUM:
        set_combo_chan_idx(frontend_kind, &t_node, &p_node, cur_channel, string);
        str_ret = com_asc_str2uni((UINT8 *)string, (UINT16 *) len_display_str);
        break;
    case PROG_TXT_EPG_NOW:
        get_proper_string_for_epg_now(cur_channel, string, str_size,p_prog_txt);
        break;
    case PROG_TXT_EPG_NEXT:
        get_proper_string_for_epg_next(cur_channel, string, str_size);
        break;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    case PROG_TXT_PROG_GROUP:
#ifndef DB_USE_UNICODE_STRING
        sprintf_ret = snprintf(string, str_size, "%s", group_name);
#else
        com_uni_str_copy(len_display_str, group_name);
#endif
        break;
#endif
    case PROG_TXT_DUAL_MONO:
        param = 0x100;
        g_deca_dev =
            (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);

        deca_ret =
            deca_io_control(g_deca_dev, DECA_GET_DDP_INMOD,
                    (UINT32) (&param));

        if (NUM_ZERO == param)
        {
            sprintf_ret = snprintf(string, str_size, "Dual Mono");
        }
        else
        {
            strncpy(string, "",1);
        }

        str_ret = com_asc_str2uni((UINT8 *)string, (UINT16 *) len_display_str);

        break;
    case PROG_TXT_SUBT_TYPE:
#if (SUBTITLE_ON == 1)
        strncpy(string, "",1);
        str_ret = com_asc_str2uni((UINT8 *)string, (UINT16 *) len_display_str);
#else
        //sprintf_ret = snprintf(string, str_size, "");
        strncpy(string, "",1);
        str_ret = com_asc_str2uni(string, (UINT16 *) len_display_str);
#endif
        break;
    default:
        break;
    }
}
