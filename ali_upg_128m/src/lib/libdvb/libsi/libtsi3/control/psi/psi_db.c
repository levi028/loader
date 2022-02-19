/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: psi_db.c
*
*    Description: save program info to database
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libsi/si_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libdb/db_interface.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_service_type.h>

#define PD_DEBUG_LEVEL          0
#if (PD_DEBUG_LEVEL>1)
#include <api/libc/printf.h>
#define PD_PRINTF           libc_printf
#elif (PD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define PD_PRINTF           PRINTF
#else
#define PD_PRINTF(...)          do{}while(0)
#endif

#ifdef CI_SERVICE_SHUNNING_DEBUG_PRINT
#define CI_SHUNNING_DEBUG libc_printf
#else
#define CI_SHUNNING_DEBUG(...) do{} while(0)
#endif

#if (PD_DEBUG_LEVEL>1)
static INT8 usr_lang_idx = 0;
static UINT8 service_lang[3][3] = {{'e','n','g'}, {'g','l','a'}, {'w','e','l'}};
#endif

static prog_node_return prog_callback = NULL;

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
static backup_new_prog pid_callback = NULL;
#endif

#if (SERVICE_PROVIDER_NAME_OPTION > 0)
static const UINT8 default_name[] = "No Name";
#endif

//add the flag for dispaly highlight or not. 1: exist,0: new node. use the lowest bit.
static UINT32 prog_flg = 0;

/*******************************************************************************
*   Function name: get_usr_lang_idx() & set_usr_lang_idx()&translate_idx_to_service_lang()
*   description:
*           1. provide to UI to tell MDL which language user select.
*******************************************************************************/
#if (PD_DEBUG_LEVEL>1)
static INT8 get_usr_lang_idx(void)
{
    return (INT8)usr_lang_idx;
}

static INT8 set_usr_lang_idx(INT8 idx)
{
    usr_lang_idx = idx;
    return (INT8)SI_SUCCESS;
}

static UINT8 *translate_idx_to_service_lang(UINT8 usr_lang_idx)
{
    UINT8 usr_idx_upper = 2;

    if(usr_lang_idx > usr_idx_upper)
    {
        usr_lang_idx = 0;
    }
    return service_lang[usr_lang_idx];
}
#endif
/*******************************************************************************
*   Function name: find_usr_sel_audio_com()
*   Description:
*       1. find the apropriate audio compent according usr_sel_service_language.
*       2. return audio index.
********************************************************************************/
#if (PD_DEBUG_LEVEL>1)
static UINT32 find_usr_sel_audio_com(PROG_INFO *p_prog, UINT8 usr_lang_sel)
{
    INT32  j                       = 0;
    INT32  k                       = 0;
    UINT32 cur_audio               = 0;
    UINT8  len                     = 3;
    UINT8  usr_idx_upper           = 2;
    UINT8  usr_idx_lower           = 0;
    UINT32 record[P_MAX_AUDIO_NUM] = {0};
    UINT32 default_sel             = 0xFFFFFFFF;

    if(usr_lang_sel > usr_idx_upper)
    {
        usr_lang_sel = 0;
    }
    if(1 < p_prog->audio_count)
    {
        MEMSET(record, 0, P_MAX_AUDIO_NUM*sizeof(UINT32));
        //UINT8 lang[3][3] ={{'e','n','g'}, {'g','l','a'}, {'w','e','l'}};
        for(j=0; j<p_prog->audio_count; j++)
        {
            record[k++] = (UINT32)j;
        }
        if(k > 0)
        {
            for(j=0;j<k;j++)
            {
                if(0 == MEMCMP(p_prog->audio_lang[record[j]], translate_idx_to_service_lang(usr_lang_sel), len))
                {
                    break;
                }
                else if(0xFFFFFFFF == default_sel)
                {
                    if( (MEMCMP(p_prog->audio_lang[record[j]], "und", len)!=0)
                     && (MEMCMP(p_prog->audio_lang[record[j]], "UND", len)!=0))
                    {
                        default_sel = record[j];
                    }
                }
            }
            if(j == k)
            {
                if(0xFFFFFFFF != default_sel)
                {
                    cur_audio = default_sel;
                }
                else
                {
                    cur_audio = record[0];
                }
            }
            else
            {
                cur_audio = record[j];
            }
        }
        else
        {
            cur_audio = 0;
        }
    }
    else
    {
        cur_audio = 0;
    }
    return cur_audio;
}
#endif

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
INT32 pid_callback_register(backup_new_prog callback)
{
    if(NULL != callback)
    {
        pid_callback = callback;
        return SI_SUCCESS;
    }
    else
    {
        return -1;
    }
}

INT32 pid_callback_unregister(void)
{
    pid_callback = NULL;
    return SI_SUCCESS;
}
#endif

INT32 prog_callback_register(prog_node_return callback)
{
    if(NULL != callback)
    {
        prog_callback = callback;
        return SI_SUCCESS;
    }
    else
    {
        return -1;
    }
}

INT32 prog_callback_unregister(void)
{
    prog_callback = NULL;

    return SI_SUCCESS;
}

static void assign_lcn(P_NODE *pg, PROG_INFO *info)
{
#ifndef _BUILD_OTA_E_
#if(defined( _MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
    UINT32 tp_id = 0;
    UINT16 lcn = 0;

    if(FALSE == info->lcn_true)
    {
        alloc_next_lcn(&lcn);        
        pg->LCN = lcn;//find_max_lcn_next();//find_max_LCN()+1;
        pg->orig_lcn = INVALID_LCN_NUM;
    }
    else
    {
        pg->orig_lcn = pg->LCN = info->lcn;
        if(-1 != check_lcn_index(pg->orig_lcn, &tp_id))
        {
            #if 0
            if(tp_id == info->tp_id)
            {
                info->lcn_true=FALSE;
                pg->LCN=find_max_lcn_next();
            }
            #else
            info->lcn_true = FALSE;          
            alloc_next_lcn(&lcn);            
            pg->LCN = lcn;
            #endif
        }        
    }
    update_lcn_table(pg->LCN);
    
#ifdef HD_SIMULCAST_LCD_SUPPORT
    if(TRUE == info->hd_lcn_true)
    {
        pg->hd_lcn = info->hd_lcn;
        pg->hd_lcn_ture = info->hd_lcn_true ;
    }
#endif

    pg->mheg5_exist = info->mheg5_exist;
    pg->lcn_true = info->lcn_true;
    pg->default_index = find_max_default_index()+1;
#endif

#ifdef LEGACY_HOST_SUPPORT
    pg->reserve_2 = info->hdplus_flag;
#endif
#endif
}

INT32 psi_pg2db(P_NODE *pg, PROG_INFO *info)
{
    INT32  i     = 0;
    UINT16 *pret = NULL;

    MEMSET(pg,0,sizeof(P_NODE));
    pg->audio_volume = AUDIO_DEFAULT_VOLUME;

#ifdef DEFAULT_AUDIO_CHANNEL
    pg->audio_channel = sys_data_get_default_audio_channel(); // Set the default channel
#endif
    pg->audio_count = info->audio_count;

    MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
    MEMCPY(pg->audio_type, info->audio_type, info->audio_count*sizeof(info->audio_type[0]));
#endif
#if (defined(_ISDBT_ENABLE_))
    MEMCPY(pg->audio_com_tag, info->audio_com_tag, info->audio_count*sizeof(info->audio_com_tag[0]));
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
   #ifndef ITALY_HD_BOOK_SUPPORT
    pg->visible_flag = info->visible_service_flag;
   #endif
    pg->num_sel_flag = info->numeric_selection_flag;
#endif

#ifdef ITALY_HD_BOOK_SUPPORT
    if(TRUE == info->lcn_true)
    {
        pg->visible_flag = (UINT16)info->lcn_visible_service_flag;
        pg->hidden_flag = (UINT16)(!info->lcn_visible_service_flag);
    }
    else
    {
        pg->visible_flag = 1;  //not LCN descriptor set visiable flag = 1;
        pg->hidden_flag = 1;
    }
#endif

    for(i=0;i<(INT32)info->audio_count;i++)
    {
        get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
    }
    pg->av_flag = info->av_flag;

    if(0 < info->ca_count)      //check CA descriptor
    {
        pg->ca_mode = 1;
    }

#ifdef DB_CAS_SUPPORT
    pg->cas_count = (info->ca_count < MAX_CAS_CNT)?info->ca_count : MAX_CAS_CNT;   //cas count
    for(i = 0; i < pg->cas_count; i++)
    {
        pg->cas_sysid[i] = info->ca_info[i].ca_system_id;
    }
#endif

    pg->pmt_pid = info->pmt_pid;

#if (/*SYS_PROJECT_FE == PROJECT_FE_DVBT && */defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
    pg->preset_flag = 1;
    pg->pmt_version = info->pmt_version;
#endif

    pg->pcr_pid = info->pcr_pid;
    pg->prog_number = info->prog_number;

    if(0 != info->service_type)  //if no service descriptor in sdt
    {//error detect
        if((1==info->av_flag)&&((SERVICE_TYPE_DRADIO==info->service_type)||(SERVICE_TYPE_MOSAIC==info->service_type)))
        {
            pg->service_type = SERVICE_TYPE_DTV;
        }
        else if((0 == info->av_flag) && (SERVICE_TYPE_DTV == info->service_type))
        {
            pg->service_type = SERVICE_TYPE_DRADIO;
        }
        else
        {
            pg->service_type = info->service_type;
        }
    }
    else
    {
        pg->service_type = (1 == info->av_flag) ? SERVICE_TYPE_DTV : SERVICE_TYPE_DRADIO;
    }

    pg->teletext_pid = info->teletext_pid;
    pg->subtitle_pid = info->subtitle_pid;
    pg->sat_id = info->sat_id;
    pg->tp_id = info->tp_id;

#if (SYS_PROJECT_FE != PROJECT_FE_DVBC)
    pg->h264_flag = (H264_VIDEO_PID_FLAG == (info->video_pid & H264_VIDEO_PID_FLAG)) || (1 == info->avc_video_exist);
    pg->mpeg4_flag = info->mpeg_4_video_exist;
	pg->video_type = (H265_VIDEO_PID_FLAG == (info->video_pid & H265_VIDEO_PID_FLAG)); // video_type=1 - H254, 	
#else
    pg->h264_flag = (H264_VIDEO_PID_FLAG == (info->video_pid & H264_VIDEO_PID_FLAG));
    pg->video_type = (H265_VIDEO_PID_FLAG == (info->video_pid & H265_VIDEO_PID_FLAG)); // video_type=1 - H254,
#endif

 #ifdef ITALY_HD_BOOK_SUPPORT
    pg->video_cnt= info->video_cnt;
    MEMCPY(pg->video_pid_array, info->video_pid_array, info->video_cnt*sizeof(info->video_pid_array[0]));
    pg->cur_video_pid_idx = info->cur_video_pid_idx;
 #else
    pg->video_pid = info->video_pid&0x1fff;
  #endif

#ifdef COMBOUI
#ifndef DEFAULT_AUDIO_CHANNEL
    pg->audio_channel = info->track;
#endif
    if(0 != info->volume)
    {
        pg->audio_volume = info->volume;
    }
#ifdef MULTI_BOUQUET_ID_SUPPORT
    pg->bouquet_count = 1;
    pg->bouquet_id[0] = info->bouquet_id;
#else
    pg->bouquet_id = info->bouquet_id;
#endif
    pg->logical_channel_num = info->logical_channel_num;

    pg->nvod_sid = info->ref_sid;
#endif

#ifdef RAPS_SUPPORT
    if(NULL == prog_callback)//raps tp won't use this function
    {
        pg->internal_number = raps_get_max_internalnumber();
        raps_set_max_internalnumber(pg->internal_number+1);
    }
#endif

    pret = DB_STRCPY((DB_ETYPE *)pg->service_name, (DB_ETYPE *)info->service_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
    pg->mtl_name_count = info->mtl_name_count;
    for(i=0;i<info->mtl_name_count;i++)
    {
       get_audio_lang2b(info->mtl_iso639_code[i], (UINT8 *)&pg->mtl_iso639_code[i]);
    }

    for(i=0;i<info->mtl_name_count;i++)
      {
        pret = DB_STRCPY((DB_ETYPE *)&pg->mtl_service_name[i], (DB_ETYPE *)&info->mtl_service_name[i]);
        if(NULL == pret)
        {
            return SI_FAILED;
        }
      }
#endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    pret = DB_STRCPY((DB_ETYPE *)pg->service_provider_name, (DB_ETYPE *)info->service_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#ifdef _INVW_JUICE
#if(defined(SUPPORT_NETWORK_NAME))
    pret = DB_STRCPY((DB_ETYPE *)pg->network_provider_name, (DB_ETYPE *)info->network_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
    pret = DB_STRCPY((DB_ETYPE *)pg->default_authority, (DB_ETYPE *)info->default_authority);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif
#endif
    assign_lcn(pg, info);

    pg->pnode_type = info->pnode_type;
    return SI_SUCCESS;
}

INT32 psi_monitor_pg2db(P_NODE *pg, PROG_INFO *info)
{
    INT32  i     = 0;
    UINT16 *pret = NULL;

    MEMSET(pg,0,sizeof(P_NODE));
    pg->audio_volume = AUDIO_DEFAULT_VOLUME;

#ifdef DEFAULT_AUDIO_CHANNEL
    pg->audio_channel = sys_data_get_default_audio_channel();    // Set the default channel
#endif
    pg->audio_count = info->audio_count;
    MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));

#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
    MEMCPY(pg->audio_type, info->audio_type, info->audio_count*sizeof(info->audio_type[0]));
#endif
#if (defined(_ISDBT_ENABLE_))
    MEMCPY(pg->audio_com_tag, info->audio_com_tag, info->audio_count*sizeof(info->audio_com_tag[0]));
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_
    pg->visible_flag = info->visible_service_flag;
    pg->num_sel_flag = info->numeric_selection_flag;
#endif

    for(i=0;i<(INT32)info->audio_count;i++)
    {
        get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
    }
    pg->av_flag = info->av_flag;

    /* check CA descriptor */
    if(info->ca_count > 0)
    {
        pg->ca_mode = 1;
    }

#ifdef DB_CAS_SUPPORT
    //cas count
    pg->cas_count = (info->ca_count < MAX_CAS_CNT)?info->ca_count : MAX_CAS_CNT;
    for(i = 0; i < pg->cas_count; i++)
    {
       pg->cas_sysid[i] = info->ca_info[i].ca_system_id;
    }
#endif

    pg->pmt_pid = info->pmt_pid;
    pg->pcr_pid = info->pcr_pid;
    pg->prog_number = info->prog_number;
    pg->service_type = info->service_type;

    pg->teletext_pid = info->teletext_pid;
    pg->subtitle_pid = info->subtitle_pid;
    pg->sat_id = info->sat_id;
    pg->tp_id = info->tp_id;
    pg->h264_flag = (H264_VIDEO_PID_FLAG == (info->video_pid & H264_VIDEO_PID_FLAG));
	pg->video_type = (H265_VIDEO_PID_FLAG == (info->video_pid & H265_VIDEO_PID_FLAG)); // video_type=1 - H254,	
    pg->video_pid = info->video_pid&0x1fff;
#ifdef COMBOUI

    pg->audio_channel = info->track;
    if(info->volume != 0)
    {
       pg->audio_volume = info->volume;
    }
#ifdef MULTI_BOUQUET_ID_SUPPORT
    pg->bouquet_count = 1;
    pg->bouquet_id[0] = info->bouquet_id;
#else
    pg->bouquet_id = info->bouquet_id;
#endif
    pg->logical_channel_num = info->logical_channel_num;
#ifdef _LCN_ENABLE_
    pg->LCN=info->lcn;
#endif
    pg->nvod_sid = info->ref_sid;
#endif

    pret = DB_STRCPY((DB_ETYPE *)pg->service_name, (DB_ETYPE *)info->service_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    pret = DB_STRCPY((DB_ETYPE *)pg->service_provider_name, (DB_ETYPE *)info->service_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#ifdef _INVW_JUICE

#if(defined(SUPPORT_NETWORK_NAME))
    pret = DB_STRCPY((DB_ETYPE *)pg->network_provider_name, (DB_ETYPE *)info->network_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#if(defined(SUPPORT_DEFAULT_AUTHORITY))
    pret = DB_STRCPY((DB_ETYPE *)pg->default_authority, (DB_ETYPE *)info->default_authority);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#endif
    return SI_SUCCESS;
}

static INT32 psi_update_pids(P_NODE *dest, P_NODE *src)
{
#if((defined(_INVW_JUICE))&& (defined(SUPPORT_NETWORK_NAME)))
    UINT16 *pret = NULL;
#endif
    INT32 size = 0;

    dest->pmt_pid = src->pmt_pid;
    dest->prog_number = src->prog_number;

    dest->video_pid = src->video_pid;
    dest->pcr_pid = src->pcr_pid;

    dest->teletext_pid = src->teletext_pid;
    dest->subtitle_pid = src->subtitle_pid;
    dest->ca_mode = src->ca_mode;
    dest->audio_count = src->audio_count;

#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
    MEMCPY(dest->audio_type, src->audio_type, src->audio_count*sizeof(src->audio_type[0]));
#endif

    dest->av_flag= src->av_flag;
    if(src->service_type != 0)
    {
        dest->service_type = src->service_type;
    }
    else
    {
        dest->service_type = (1 == src->av_flag) ? SERVICE_TYPE_DTV : SERVICE_TYPE_DRADIO;
    }

    size = src->audio_count*sizeof(src->audio_pid[0]);
    MEMCPY(dest->audio_pid, src->audio_pid, size);
    MEMCPY(dest->audio_lang, src->audio_lang, size);
#if (defined(_ISDBT_ENABLE_))
    size = src->audio_count*sizeof(src->audio_com_tag[0]);
    MEMCPY(dest->audio_com_tag, src->audio_com_tag, size);
#endif

#ifdef _INVW_JUICE
#if(defined(SUPPORT_NETWORK_NAME))
    pret = DB_STRCPY((DB_ETYPE *)dest->network_provider_name, (DB_ETYPE *)src->network_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#endif

#endif
    return SI_SUCCESS;
}

static INT32 psi_update_name(P_NODE *dest, P_NODE *src)
{
    UINT16 *pret = NULL;

    pret = DB_STRCPY((DB_ETYPE *)dest->service_name, (DB_ETYPE *)src->service_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    pret = DB_STRCPY((DB_ETYPE *)dest->service_provider_name, (DB_ETYPE *)src->service_provider_name);
    if(NULL == pret)
    {
        return SI_FAILED;
    }

#endif
    return SI_SUCCESS;
}

void psi_fake_name(UINT8 *name, UINT16 program_number)
{
    const UINT32 name_size = 256;
//    int len = 0;

#if (SERVICE_PROVIDER_NAME_OPTION>0)
    if (program_number)
    {
#endif
        snprintf((char *)(name+1), (name_size-1), "Ch-%d", (int)program_number);
        name[0] = STRLEN((char *)(name+1));
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    }
    else
    {
        MEMCPY(name+1, default_name, sizeof(default_name));
        name[0] = sizeof(default_name)-1;
    }
#endif
}

static void psi_adjust_node(UINT8 *string_buf, UINT32 pos, P_NODE *program, UINT16 storage_scheme)
{
    P_NODE *db_prog = program+1;
    P_NODE *db_bak  = program+2;

    if (PROG_ADD_DIRECTLY == storage_scheme)
    {
        PD_PRINTF("%s: please check up your function flow!\n", __FUNCTION__);
        return;
    }

    get_prog_at(pos, db_prog);
    MEMCPY(db_bak, db_prog, sizeof(P_NODE));
    psi_update_pids(db_prog, program);

    switch(storage_scheme)
    {
        case PROG_ADD_REPLACE_OLD:
            {
                //use prog's new name replace old name in database
                psi_update_name(db_prog, program);
            }
            break;
        case PROG_ADD_PRESET_REPLACE_NEW:
            {
                //the preset prog's name keep unchanged
                if (!db_prog->preset_flag)
                {
                    psi_update_name(db_prog, program);
                }
            }
            break;
         case PROG_ADD_REPLACE_NONPRESET:
            {
                psi_fake_name(string_buf, db_prog->prog_number);

                if (DB_STRCMP((DB_ETYPE *)string_buf, (DB_ETYPE *)db_prog->service_name))
                {
                    if (!db_prog->preset_flag)
                    {
                        psi_update_name(db_prog, program);
                    }
                }
            }
            break;
         default:
            {
               PD_PRINTF("%s: storage_scheme=0x%02x not supported!\n", __FUNCTION__, storage_scheme);
            }
            break;
    }

#ifdef CI_PLUS_SERVICESHUNNING
    /* update shunning information */
    db_prog->shunning_protected = program->shunning_protected;
    CI_SHUNNING_DEBUG("update to exist program shunning_protected = %d  (@%s)\n",
        db_prog->shunning_protected, __FUNCTION__);
#endif

    db_prog->h264_flag = program->h264_flag;
	db_prog->video_type = program->video_type;

#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
    db_prog->preset_flag = 1;
    db_prog->pmt_version = program->pmt_version;
#endif
#ifdef HD_SIMULCAST_LCD_SUPPORT
    db_prog->hd_lcn = program->hd_lcn;
    db_prog->hd_lcn_ture = program->hd_lcn_ture;
#endif
    MEMCPY(program, db_prog, sizeof(P_NODE));
    if (MEMCMP(db_prog, db_bak, sizeof(P_NODE)))
    {
#if DB_VERSION < 40
        modify_prog(pos, db_prog);
#else
        modify_prog(db_prog->prog_id, db_prog);
#endif
    }
}

UINT32 get_prog_flg(void)
{
    return prog_flg;
}

INT32 psi_install_prog(UINT8 *string_buf, PROG_INFO *p_info, P_NODE *p_node, UINT32 search_scheme,UINT32 storage_scheme)
{
    UINT16 pos    = INVALID_POS_NUM;
    INT32  ret    = 0;
    BOOL   result = FALSE;

    psi_pg2db(p_node, p_info);

#ifdef CI_PLUS_SERVICESHUNNING
    p_node->shunning_protected = p_info->shunning_protected;
#endif
#ifdef SID_AS_LCN
    if((INVALID_LCN_NUM==p_node->orig_lcn)||(0==p_node->LCN))
    {
        p_node->LCN=p_node->prog_number;
    }
#endif
    prog_flg = 0;
#if DB_VERSION < 40
    pos = (PROG_ADD_DIRECTLY==storage_scheme) ? INVALID_POS_NUM : get_prog_pos(p_node);
#else
    if(PROG_ADD_DIRECTLY==storage_scheme)
    {
        pos = INVALID_POS_NUM;
    }
    else if(SUCCESS == db_search_lookup_node(TYPE_PROG_NODE, p_node))
    {
#if (defined POLAND_SPEC_SUPPORT||defined ITALY_HD_BOOK_SUPPORT)
        if(NULL == pid_callback)
        {
            return SI_SUCCESS;
        }
#else
        prog_flg = 1;
        pos = get_prog_pos(p_node->prog_id);
#endif
    }

#endif
    if(NULL != prog_callback)
    {
        result = prog_callback(p_node);
        if( !result)
        {
           return SI_PERROR;
        }
    }
    if((NULL == prog_callback) || result)
    {

    if (INVALID_POS_NUM == pos)
    {

#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        p_node->pmt_version = 0;
#endif

#if DB_VERSION < 40
        if (STORE_SPACE_FULL == add_node(TYPE_PROG_NODE, p_node))
#else
        #if(defined(UNVISIBLE_PROG_NOT_STORAGE))
        if(p_node->visible_flag)
        #endif
        ret = add_node(TYPE_PROG_NODE, p_node->tp_id,p_node);
        if ((DBERR_FLASH_FULL == ret) || (DBERR_MAX_LIMIT) == ret)
#endif
        {
            return SI_STOFULL;
        }
    }
    else
    {
        if ((p_info->ca_count)&&(0 == (search_scheme&P_SEARCH_SCRAMBLED))&&(0 == p_node->preset_flag))
        {
            return SI_PERROR;     /* new CA program, not preset program. */
        }
        psi_adjust_node(string_buf, pos, p_node, storage_scheme);

#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
        p_node->pmt_version = 1;
#endif

    }
    }
    #ifdef AUTO_UPDATE_TPINFO_SUPPORT
    if(NULL != pid_callback)
    {
        pid_callback(p_node->prog_id, p_node->av_flag);
    }
    #endif

    return SI_SUCCESS;
}


