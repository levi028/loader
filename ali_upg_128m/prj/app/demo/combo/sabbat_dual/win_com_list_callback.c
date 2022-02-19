/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_list_callback.c
*
*    Description: To realize the callback common function of list
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/obj_container.h>

#include "string.id"

#include "win_language.h"
#include "copper_common/system_data.h"
#include "win_tvsys.h"
#include "win_disk_operate.h"
#include "win_time.h"
#include "win_display_set.h"
#include "win_parental.h"
#include "win_subtitle_setting.h"
#include "win_com_list.h"
#include "win_com_list_callback.h"

#include "win_osd_set.h"
#include "win_pvr_tms.h"
#include "win_multiviewset.h"

#ifndef MAX_GROUP_NUM
#define MAX_GROUP_NUM    (1+MAX_SAT_NUM + MAX_FAVGROUP_NUM+MAX_LOCAL_GROUP_NUM)
#endif

static UINT8    satlist_tuner_idx = 0;
static UINT32    tplist_sat_id = 0;


#ifdef USB_MP_SUPPORT
static UINT16 mpsort_set_strids[] =
{
    RS_INFO_NAME,
    RS_SYSTEM_TIME,
    RS_COMMON_SIZE,
    RS_FAVORITE,
};
#define MPSORT_SET_CNT (sizeof(mpsort_set_strids)/sizeof(mpsort_set_strids[0]))
#endif

static UINT16 sort_set_strids[] =
{
    RS_SORT_DEFAULT,
    RS_SORT_LOCK,
    RS_SORT_NAME_AZ,
    RS_MS_INFO_SERVICE_ID,
    RS_MS_LCN_SORT,
    RS_SORT_FTA_DOLLAR
};
#define SORT_SET_CNT (sizeof(sort_set_strids)/sizeof(sort_set_strids[0]))

//extern PRESULT comlist_menu_osd_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT comlist_satlist_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};

    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_tuner_sat_name(satlist_tuner_idx, w_top + i, unistr);
            win_comlist_set_str(i + w_top,NULL,(char*)unistr,0 );
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_tplist_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_tp_name(tplist_sat_id, w_top + i, unistr);
            win_comlist_set_str(i + w_top,NULL,(char*)unistr,0 );
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_lnblist_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            //get_tuner_sat_name(antset_cur_tuner, wTop + i, unistr);
            get_lnb_name(w_top + i, unistr);
            win_comlist_set_str(i + w_top,NULL,(char*)unistr,0 );
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_diseqc10list_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    char   str[50] = {0};
    UINT16 str_id = 0;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_diseqc10_name(i + w_top,str,unistr,&str_id);
            win_comlist_set_str(i + w_top,str,(char *)unistr,str_id);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_diseqc11list_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    UINT16 val = 0;
    UINT16 n = 0;
    OBJLIST *ol = NULL;
    char   diseqc11_str[50] = {0};
    UINT16 str_id = 0;
    PRESULT cb_ret = PROC_PASS;
    const UINT16 max_val_4 = 4;
    const UINT16 max_val_5 = 5;
    const UINT16 max_val_8 = 8;
    const UINT16 max_val_9 = 9;
    const UINT16 max_val_12 = 12;
    const UINT16 max_val_13 = 13;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            MEMSET(diseqc11_str, 0, sizeof(diseqc11_str));

            n = i + w_top;
            val = n;
            str_id = 0;

            if(0 == val)
            {
                str_id = RS_DISPLAY_DISABLE;
            }
            else
            {
                if(max_val_4 >= val)
                {
                    if((1 == val) || (0 == i))
                    {
                        snprintf(diseqc11_str, 50, "1Cascade M1: Port%d",(val - 0));
                    }
                    else
                    {
                        snprintf(diseqc11_str, 50, "             Port%d",(val - 0));
                    }
                }
                else if(max_val_8 >= val)
                {
                    if((max_val_5 == val) || (0 == i))
                    {
                        snprintf(diseqc11_str, 50, "1Cascade M2: Port%d",(val - 4));
                    }
                    else
                    {
                        snprintf(diseqc11_str, 50, "             Port%d",(val - 4));
                    }

                }
                else if(max_val_12 >= val)
                {
                    if((max_val_9 == val) || (0 == i))
                    {
                        snprintf(diseqc11_str, 50, "1Cascade M3: Port%d",(val - 8));
                    }
                    else
                    {
                        snprintf(diseqc11_str, 50, "             Port%d",(val - 8));
                    }
                }
                else
                {
                    if((max_val_13 == val) || (0 == i))
                    {
                        snprintf(diseqc11_str, 50, "2Cascades: Port%d",(val - 12));
                    }
                    else
                    {
                        snprintf(diseqc11_str, 50, "          Port%d",(val - 12));
                    }
                }
            }
            if(0 == str_id)
            {
                win_comlist_set_str(i + w_top,diseqc11_str,NULL,0);
            }
            else
            {
                win_comlist_set_str(i + w_top,NULL,NULL,str_id);
            }
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_ifchannel_list_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    //char str_len = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    char   str[50] = {0};
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_if_channel_name(i + w_top,unistr);
            win_comlist_set_str(i + w_top,str,(char *)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_iffreqs_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    char   str[50] = {0};
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_centre_freqs_name(i + w_top,unistr);
            win_comlist_set_str(i + w_top,str,(char *)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

#ifdef USB_MP_SUPPORT
static PRESULT comlist_mpsortset_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            win_comlist_set_str(i + w_top,NULL,NULL,mpsort_set_strids[i + w_top]);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

#endif

static PRESULT comlist_chgrpall_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    UINT8 group_type = 0;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_chan_group_name((char *)unistr,(50*2),i+w_top, &group_type);
            win_comlist_set_str(i + w_top,NULL,(char *)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_chgrpfav_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    UINT16 unistr[50] = {0};
    UINT8 group_type = 0;
    UINT8 fav_grp_start = 0;
    UINT8 av_flag = 0;
    UINT8 group_idx = 0;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        av_flag  = sys_data_get_cur_chan_mode();
        fav_grp_start = sys_data_get_sate_group_num(av_flag) + 1;

        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            group_idx = i+w_top + fav_grp_start;
            if(group_idx > MAX_GROUP_NUM)
            {
                PRINTF("group_idx(%d) > MAX_GROUP_NUM(%d)\n", group_idx, \
                            MAX_GROUP_NUM);
            }
            get_chan_group_name((char *)unistr, (50*2),group_idx, &group_type);

            win_comlist_set_str(i + w_top,NULL,(char *)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

static PRESULT comlist_favset_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT cb_ret = PROC_PASS;
    UINT16 i = 0;
    UINT16 w_top = 0;
    UINT16 cnt = 0;
    OBJLIST *ol = NULL;
    SYSTEM_DATA *sys_data = NULL;
	if(0 == cnt)
	{
		;
	}
    sys_data = sys_data_get();

    ol = (OBJLIST*)pobj;
    cnt = osd_get_obj_list_count(ol);

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
#ifdef FAV_GROP_RENAME
            win_comlist_set_str(i + w_top,sys_data->favgrp_names[i+w_top],NULL,0);
#else
            win_comlist_set_str(i + w_top,NULL,NULL,fav_set_strids[i + w_top]);
#endif
        }
    }
    return cb_ret;
}

static PRESULT comlist_sortset_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            win_comlist_set_str(i + w_top,NULL,NULL,sort_set_strids[i + w_top]);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

#ifdef DVR_PVR_SUPPORT
static PRESULT comlist_recordlist_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT cb_ret = PROC_PASS;
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    struct list_info  rl_info;
    UINT16 unistr[50] = {0};
    char str[50] = {0};
    UINT32 rtm = 0;
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    UINT32 str_len = 0;

    ol = (OBJLIST*)pobj;
    w_top = osd_get_obj_list_top(ol);
       MEMSET(&rl_info, 0, sizeof(struct list_info));
    if(EVN_PRE_DRAW == event)
    {
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            pvr_get_rl_info_by_pos(i+w_top,&rl_info);

            rtm = rl_info.duration;
            hh = rtm/3600;
            mm = (rtm - hh*3600)/60;
            ss = (rtm - hh*3600 - mm*60);

            snprintf(str,50,"%d %04d/%d/%d %02d:%02d:%02d - %02lu:%02lu:%02lu",
                 i + w_top + 1,
                rl_info.tm.year,rl_info.tm.month,rl_info.tm.day,
                rl_info.tm.hour,rl_info.tm.min,rl_info.tm.sec,
                hh,mm,ss);

            com_asc_str2uni((UINT8 *)str,unistr);
            str_len = com_uni_str_len(unistr);
            com_uni_str_copy(&unistr[str_len],(UINT16*)rl_info.txti);
            win_comlist_set_str(i + w_top,NULL,(char *)unistr,0);
        }
    }
    else if(EVN_PRE_CHANGE == event)
    {
        if( osd_get_obj_list_cur_point(ol) == osd_get_obj_list_single_select(ol) )
        {
            cb_ret = PROC_LOOP;
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}
#endif

static PRESULT comlist_chanlist_callback(POBJECT_HEAD pobj, VEVENT event, 
    UINT32 param1 __MAYBE_UNUSED__, UINT32 param2 __MAYBE_UNUSED__)
{
    PRESULT cb_ret = PROC_PASS;
    UINT16 i = 0;
    UINT16 w_top = 0;
    OBJLIST *ol = NULL;
    P_NODE p_node;
    UINT16 unistr[30] = {0};

    ol = (OBJLIST*)pobj;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    if(EVN_PRE_DRAW == event)
    {
        w_top = osd_get_obj_list_top(ol);
        for(i=0;i<ol->w_dep && (i+w_top)<ol->w_count;i++)
        {
            get_prog_at(i+w_top,&p_node);
            if(p_node.ca_mode)
            {
                com_asc_str2uni((UINT8 *)"$", unistr);
            }
            com_uni_str_copy_char((UINT8*)&unistr[p_node.ca_mode], p_node.service_name);
            win_comlist_set_str(i + w_top,NULL,(char *)unistr,0);
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}

void comlist_sub_list_common(COM_POP_LIST_TYPE_T type, COM_POP_LIST_PARAM_T *param, 
    UINT16 *count, PFN_CALLBACK *callback, PFN_KEY_MAP *winkeymap __MAYBE_UNUSED__, 
    PFN_KEY_MAP *list_keymap __MAYBE_UNUSED__)
{
    UINT av_mode = 0;

    switch (type)
    {
        case POP_LIST_TYPE_SAT:
        satlist_tuner_idx = param->id;
        *count = get_tuner_sat_cnt(satlist_tuner_idx);
        *callback = comlist_satlist_callback;
        break;
    case POP_LIST_TYPE_TP:
        tplist_sat_id = param->id;
        *count = get_tp_num_sat(tplist_sat_id);
        *callback = comlist_tplist_callback;
        break;
    case POP_LIST_TYPE_LNB:
        *count = get_lnb_type_cnt();
        *callback = comlist_lnblist_callback;
        break;
    case POP_LIST_TYPE_DISEQC10:
        *count = get_diseqc10_cnt();
        *callback = comlist_diseqc10list_callback;
        break;
    case POP_LIST_TYPE_DISEQC11:
        *count = get_diseqc11_cnt();
        *callback = comlist_diseqc11list_callback;
        break;
    case POP_LIST_TYPE_CHGRPALL:
        *count = sys_data_get_group_num();
        *callback = comlist_chgrpall_callback;
        break;
    case POP_LIST_TYPE_CHGRPFAV:
        av_mode = sys_data_get_cur_chan_mode();
        *count = sys_data_get_fav_group_num(av_mode);
        *callback = comlist_chgrpfav_callback;
        break;
    case POP_LIST_TYPE_CHGRPSAT:
        av_mode = sys_data_get_cur_chan_mode();
        *count = sys_data_get_sate_group_num(av_mode) + 1;
        *callback = comlist_chgrpall_callback;
        break;
    case POP_LIST_TYPE_FAVSET:
        *count = MAX_FAVGROUP_NUM;
        *callback = comlist_favset_callback;
        break;
    case POP_LIST_TYPE_SORTSET:
        *count = SORT_SET_CNT;
        *callback = comlist_sortset_callback;
        break;
    case POP_LIST_TYPE_CHANLIST:
        av_mode = sys_data_get_cur_chan_mode();
        *count = get_prog_num(VIEW_ALL | av_mode, 0);
        *callback = comlist_chanlist_callback;
        break;
    case POP_LIST_MENU_LANGUAGE_OSD:
        *count = win_language_get_menu_language_osd_num();
        *callback = comlist_menu_language_osd_callback;
        break;
    case POP_LIST_MENU_LANGUAGE_STREAM:
        *count = win_language_get_menu_language_stream_num();
        *callback = comlist_menu_language_stream_callback;
        break;
    case POP_LIST_MENU_TVSYS:
        *count = win_language_get_tvsys_num();
        *callback = comlist_tvsys_callback;
        break;
    case POP_LIST_MENU_OSDSET:
        *count = win_osd_get_num();
        *callback = comlist_menu_osd_callback;
        break;
    case POP_LIST_TYPE_IF_CHANNEL:
        *count = get_if_channel_cnt();
        *callback = comlist_ifchannel_list_callback;
        break;
    case POP_LIST_TYPE_CENTRE_FREQ:
        *count = get_centre_freqs_cnt();
        *callback = comlist_iffreqs_callback;
        break;
    case POP_LIST_DISK_MODE:
        *count = win_disk_mode_get_num();
        *callback = comlist_disk_mode_callback;
        break;
    case POP_LIST_DVR_TYPE:
        *count = win_dvr_type_get_num();
        *callback = comlist_dvr_type_callback;
        break;
    case POP_LIST_PARTITION:
        *count = win_partition_get_num();
        *callback = comlist_partition_callback;
        break;
    default:
        break;
    }
}

void comlist_sub_list_advanced(COM_POP_LIST_TYPE_T type, COM_POP_LIST_PARAM_T *param __MAYBE_UNUSED__, 
    UINT16 *count, PFN_CALLBACK *callback, PFN_KEY_MAP *winkeymap, PFN_KEY_MAP *list_keymap)
{
    switch (type)
    {
#ifdef DVR_PVR_SUPPORT
    case POP_LIST_TYPE_RECORDLIST:
        *count = pvr_get_rl_count();
        *callback = comlist_recordlist_callback;
        break;
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
    case POP_LIST_TYPE_VOLUMELIST:
        *count = storage_get_volume_count();
        *callback = comlist_volumelist_callback;
        break;
#endif
#endif

#ifndef CI_PLUS_PVR_SUPPORT
    case POP_LIST_MENU_TIME:
        *count = win_time_get_num();
        *callback = comlist_menu_time_callback;
        break;
#endif
#ifdef KEY_EDIT
    case POP_LIST_MENU_KEYLIST:
        *count = win_keylist_get_num();
        *callback = comlist_menu_keylist_callback;
        break;
#endif

#ifdef DISPLAY_SETTING_SUPPORT
    case POP_LIST_MENU_DISPLAY_SET:
        *count = win_display_set_get_item_num();
        *callback = comlist_display_set_callback;
        break;
#endif

#ifdef USB_MP_SUPPORT
    case POP_LIST_TYPE_MPSORTSET:
        *count = MPSORT_SET_CNT;
        *callback = comlist_mpsortset_callback;
        break;
    case POP_LIST_TYPE_VIDEOSORTSET:
        *count = MPSORT_SET_CNT - 1;
        *callback = comlist_mpsortset_callback;
        break;
#endif

#ifdef DISK_MANAGER_SUPPORT
    case POP_LIST_REMOVABLE_DISK_INFO:
        *count = win_removable_disk_get_num();
        *callback = comlist_disk_info_callback;
        break;
#endif

#ifdef SHOW_WELCOME_SCREEN
    case POP_LIST_MENU_COUNTRY_NETWORK:
        *count = win_country_net_get_num();
        *callback = comlist_country_net_callback;
        break;
#endif
#ifdef _INVW_JUICE  //need to check temp remove
#else
#ifdef PARENTAL_SUPPORT
    case POP_LIST_MENU_PARENTAL_MENU_LOCK:
        *count = win_parental_get_osd_num();
        *callback = comlist_menu_parental_osd_callback;
        break;
    case POP_LIST_MENU_PARENTAL_CHANNEL_LOCK:
        *count = win_parental_get_osd_num();
        *callback = comlist_menu_parental_osd_callback;
        break;
    case POP_LIST_MENU_PARENTAL_CONTENT_LOCK:
        *count = win_parental_get_osd_num();
        *callback = comlist_menu_parental_osd_callback;
        break;
#endif
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    case POP_LIST_SUBT_SET:
        *count = win_subt_set_get_num();
        *callback = comlist_menu_subt_set_callback;
        *winkeymap = comlist_menu_subt_set_winkeymap;
        *list_keymap = comlist_menu_subt_set_listkeymap;
        break;
#endif
#ifdef IMG_2D_TO_3D
    case POP_LIST_IMAGE_SET:
        *count = win_imageset_get_num();
        *callback = comlist_menu_imageset_callback;
        *winkeymap = comlist_menu_imageset_winkeymap;
        *list_keymap = comlist_menu_imageset_listkeymap;
        break;
#endif
#ifdef MULTIVIEW_SUPPORT
    case POP_LIST_MULTIVIEW_SET:
        *count = win_multiview_get_num();
        *callback = comlist_menu_multiview_callback;
        break;
#endif

#ifdef SAT2IP_SERVER_SUPPORT
    case POP_LIST_MENU_SATIP_SLOT_INSTALL:
        *count = win_satip_serv_get_sat_num();
        *callback = comlist_menu_serv_slot_callback;
        break;
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
    case POP_LIST_MENU_SATIP_SERVER:
        *count = win_satip_client_get_server_num();
        *callback = comlist_menu_satip_client_callback;
        break;
#endif
    default:
        break;
    }
}

