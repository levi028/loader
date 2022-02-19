/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_tms.c

*    Description: timeshift setting menu key&msg proc.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#ifdef DVR_PVR_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_popup.h"

#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
#include <api/libc/list.h>
#include <api/libfs2/dirent.h>
#include <api/libpvr/lib_pvr.h>
#endif
#include <api/libmp/lib_mp.h>
#include "menus_root.h"
#include "win_pvr_tms.h"

//#define UI_PVR_TS_PS_SUPPORT

#define LINE_L_OF   0
//#define LINE_T_OF   ()
#define LINE_W      CON_W
#define LINE_H      4
#define PVR_OBJ_KEY_ID_1 1
#define PVR_OBJ_KEY_ID_2 2
#define PVR_OBJ_KEY_ID_3 3
#define PVR_OBJ_KEY_ID_4 4

#define VACT_RECORD_ALL     (VACT_PASS+1)

static VACTION tms_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT tms_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tms_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT tms_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    tms_item_con_keymap,tms_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 28,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line, ID,idu,idd,l,t,w,h,name_id,setstr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l+TXTN_L_OF, t+TXTN_T_OF,(TXTN_W-20),TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset, NULL,1,1,1,1,1,l + (TXTS_L_OF-20), t + TXTS_T_OF,TXTS_W+20,TXTS_H,0,setstr)     \
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, l + LINE_L_OF, t + CON_H, LINE_W, LINE_H, 0, 0)

#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
LDEF_MM_ITEM(g_win_timeshift,tms_item_con1, &tms_item_con2,tms_item_txtname1,tms_item_txtset1,tms_item_line1,1,4,2,    \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    RS_DVR_TIMESHIFT,NULL)

LDEF_MM_ITEM(g_win_timeshift,tms_item_con2, &tms_item_con3,tms_item_txtname2,tms_item_txtset2,tms_item_line2,2,1,3,    \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,    RS_TMS_TO_REC,NULL)

LDEF_MM_ITEM(g_win_timeshift,tms_item_con3, NULL,tms_item_txtname3,tms_item_txtset3,tms_item_line3,3,2,4,    \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,    RS_RECORD_TYPE,NULL)

LDEF_MM_ITEM(g_win_timeshift,tms_item_con4, NULL,tms_item_txtname4,tms_item_txtset4,tms_item_line4,4,3,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H,    RS_DVR_PARTITION,len_display_str)

#else
LDEF_MM_ITEM(g_win_timeshift,tms_item_con1, &tms_item_con2,tms_item_txtname1,tms_item_txtset1,tms_item_line1,1,3,2,    \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    RS_DVR_TIMESHIFT,NULL)

LDEF_MM_ITEM(g_win_timeshift,tms_item_con2, &tms_item_con3,tms_item_txtname2,tms_item_txtset2,tms_item_line2,2,1,3,    \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,    RS_TMS_TO_REC,NULL)

LDEF_MM_ITEM(g_win_timeshift,tms_item_con3, NULL,tms_item_txtname3,tms_item_txtset3,tms_item_line3,3,2,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,    RS_RECORD_TYPE,NULL)
#endif /* MULTI_PARTITION_SUPPORT */


DEF_CONTAINER(g_win_timeshift,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    tms_con_keymap,tms_con_callback,  \
    (POBJECT_HEAD)&tms_item_con1, 1,0)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
#if 0
CONTAINER *tms_items[] =
{
    &tms_item_con1,
};

POBJECT_HEAD tms_item_name[] =
{
    (POBJECT_HEAD)&tms_item_txtname1,
};

POBJECT_HEAD tms_item_set[] =
{
    (POBJECT_HEAD)&tms_item_txtset1,
};
#endif


#ifndef DISK_MANAGER_SUPPORT
#define PVR_VOL_MESSAGE_ERROR_1  -2//Cann't find last PVR
#define PVR_VOL_MESSAGE_ERROR_2  -3//Cann't find any PVR partition
static UINT8 m_sto_device_type;
static UINT8 m_sto_device_sub;
static UINT8 m_sto_device_part;
static int m_volume_index;
static int m_pvr_vol_error;

#endif

#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))

static BOOL win_pvr_vol_init()
{
    m_volume_index = -1;
    m_pvr_vol_error = 0;
    storage_get_device_info(STORAGE_TYPE_SD);

    if(storage_get_volume_count() == 0)
    {
        set_container_active(&tms_item_con4, FALSE);
        m_pvr_vol_error = -1;
        return FALSE;
    }

    m_sto_device_type = sys_data_get_storage_type();
    m_sto_device_sub = sys_data_get_storage_sub_dev();
    m_sto_device_part = sys_data_get_storage_partition();

    // check the system data setting valid.
    m_volume_index = storage_find_volume(m_sto_device_type,m_sto_device_sub,m_sto_device_part);
    if(m_volume_index < 0)
    {
        m_pvr_vol_error = -2;
        libc_printf("cann't find the origial PVR partition(%d:%d:%d)\n",
                m_sto_device_type,m_sto_device_sub,m_sto_device_part);
        // invalid config, load current pvr setting.
        storage_get_cur_pvr_sto_info(&m_sto_device_type , &m_sto_device_sub, &m_sto_device_part);

        // check it again.
        m_volume_index = storage_find_volume(m_sto_device_type,m_sto_device_sub,m_sto_device_part);
        if(m_volume_index < 0)
        {
            ASSERT(0);
            m_sto_device_type = 0;
            m_sto_device_sub = 0;
            m_sto_device_part = 0;
            m_pvr_vol_error = -3;
            libc_printf("cann't find the new PVR partition(%d:%d:%d)\n",
                    m_sto_device_type,m_sto_device_sub,m_sto_device_part);
            return FALSE;
        }
        libc_printf("find the new PVR partition(%d:%d:%d)\n",m_sto_device_type,m_sto_device_sub,m_sto_device_part);
    }
    return TRUE;
}

static void win_get_nex_vol(int offset)
{
    m_volume_index += offset;

    ASSERT(storage_get_volume_count())
    if((UINT32)m_volume_index >= (UINT32)storage_get_volume_count())
        m_volume_index = 0;
    if(m_volume_index < 0)
         m_volume_index = storage_get_volume_count()-1;

}

static void win_set_vol_string(PTEXT_FIELD ptxt)
{ 
    char str[32];

    str[0] = 0;
    if(m_volume_index>=0)
        storage_index_to_osd_string(m_volume_index, str, 32);
    osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
}

PRESULT comlist_volumelist_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cb_ret = PROC_PASS;
    UINT16 i,w_top;
    OBJLIST* ol;
    UINT16 unistr[50];
    char str[50];

    ol = (OBJLIST*)pobj;
    w_top = osd_get_obj_list_top(ol);

    if (EVN_PRE_DRAW == event)
    {
        for (i = 0; i< ol->w_dep && (i+w_top) < ol->w_count; i++)
        {
            storage_index_to_osd_string(i+w_top,str, 32);
            com_asc_str2uni(str,unistr);
            win_comlist_set_str(i+w_top, NULL, unistr, 0);
        }
    }
    else if(EVN_POST_CHANGE == event)
        cb_ret = PROC_LEAVE;

    return cb_ret;
}

void win_pvr_vol_save()
{
    char new_volume[32];
    UINT8 back_saved;
    win_popup_choice_t choice;
    PVR_VOL_INFO volume;

    if(m_volume_index < 0 || storage_get_volume_count()== 0)
        return;

    if(storage_get_pvr_volume(m_volume_index, &volume)<0)
        return;

    if( volume.type == m_sto_device_type &&
        volume.sub_type == m_sto_device_sub &&
        volume.partition_idx == m_sto_device_part)
    {
        return;
    }

    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_msg("Are you sure to change PVR partition?", NULL, 0);
    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
    choice = win_compopup_open_ext(&back_saved);
    if (WIN_POP_CHOICE_YES == choice)
    {
        if(storage_change_pvr_volume(volume.type,volume.sub_type,volume.partition_idx)>=0)
        {
            sys_data_set_storage(volume.type,volume.sub_type,volume.partition_idx);
        }
    }
}

void win_pvr_vol_invalid_msg()
{
    UINT8 back_saved;
    win_popup_choice_t choice;

    if(storage_get_volume_count() == 0)
    {
        return;
    }

    if(m_pvr_vol_error < -1)
    {
        char hint[128] = {0};
        char name_old[64] = {0};
        char name_new[64] = {0};
        UINT8 device_type = sys_data_get_storage_type();
        UINT8 device_sub = sys_data_get_storage_sub_dev();
        UINT8 device_part = sys_data_get_storage_partition();

        storage_type2osd_string2(device_type, device_sub, device_part,name_old, 64);
        storage_type2osd_string2(m_sto_device_type, m_sto_device_sub, m_sto_device_part,name_new, 64);
        if(PVR_VOL_MESSAGE_ERROR_1 == m_pvr_vol_error)
        {
            snprintf(hint, 128,"Cann't find last PVR %s, auto select to %s!",name_old, name_new);
        }
        else if(PVR_VOL_MESSAGE_ERROR_2 == m_pvr_vol_error )
        {
            snprintf(hint, 128,"Cann't find any PVR partition!");
        }
        else
        {
            ASSERT(0);
        }win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(hint, NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
        choice = win_compopup_open_ext(&back_saved);
        if(WIN_POP_CHOICE_YES == choice)
        {
            sys_data_set_storage(m_sto_device_type, m_sto_device_sub, m_sto_device_part);
        }
    }
}
#endif /* (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT)) */

static void win_tms_set_string(UINT8 input, PTEXT_FIELD ptxt)
{
    if(0==input)
    {
        osd_set_text_field_content(ptxt, STRING_ID,RS_COMMON_OFF);
    }
    else
    {
        osd_set_text_field_content(ptxt, STRING_ID,RS_COMMON_ON);
    }
}


static void win_set_rec_type_string(UINT8 input, PTEXT_FIELD ptxt)
{
    if(0==input)
    {
      osd_set_text_field_content(ptxt, STRING_ID,RS_RECORD_TYPE_TS);
    }
    else
    {
      osd_set_text_field_content(ptxt, STRING_ID,RS_RECORD_TYPE_PS);
    }
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION tms_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

/*
static PRESULT comlist_menu_tms_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    //UINT16 i;
    //UINT16 wTop;
    //UINT8 id;
    //char str_len = 0;
        //OBJLIST *ol = NULL;
    //UINT8 temp[4];

    PRESULT cbRet = PROC_PASS;

        //ol = (OBJLIST*)pobj;
#if 0
    if(event==EVN_PRE_DRAW)
    {
    }
    else if(event == EVN_POST_CHANGE)
        cbRet = PROC_LEAVE;
#endif
    if(event == EVN_POST_CHANGE)
        cbRet = PROC_LEAVE;

    return cbRet;
}
*/
static void tms_set_tms2rec_item_active(BOOL update)
{
    UINT8 action = C_ATTR_INACTIVE;

    action = sys_data_get_tms() ? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
    set_container_active(&tms_item_con2, action);
    if(update)
    {
      osd_draw_object( (POBJECT_HEAD)&tms_item_con2, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

static void win_tms_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    UINT8 ret=0;
    UINT8 time_shift = 0;

    if(PVR_OBJ_KEY_ID_1 == id)
    {
        ret = !sys_data_get_tms();
        sys_data_set_tms(ret);

        time_shift = sys_data_get_tms();
        if(0 == time_shift)
        {
            PRINTF("time_shift : 0!\n");
        }
        //end
        win_tms_set_string(time_shift,&tms_item_txtset1);
        tms_set_tms2rec_item_active(TRUE);
    }
    else if (PVR_OBJ_KEY_ID_2 == id)
    {
        ret = !sys_data_get_tms2rec();
        sys_data_set_tms2rec(ret);
        win_tms_set_string(sys_data_get_tms2rec(),&tms_item_txtset2);
    }
    else if (PVR_OBJ_KEY_ID_3 == id)
    {
        ret = !sys_data_get_rec_type();
        sys_data_set_rec_type(ret);
        win_set_rec_type_string(sys_data_get_rec_type(),&tms_item_txtset3);

    }
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
    else if (PVR_OBJ_KEY_ID_4 == id)
    {
        if(storage_get_volume_count()<=1)
            return;
        win_get_nex_vol(-1);
        win_set_vol_string(&tms_item_txtset4);

    }
#endif
    osd_track_object(pobj,C_UPDATE_ALL);
}

static void win_tms_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    UINT8 time_shift = 0;
    UINT8 ret = 0;

    if(PVR_OBJ_KEY_ID_1  == id)
    {
        ret = !sys_data_get_tms();
        sys_data_set_tms(ret);
        time_shift = sys_data_get_tms();
        if(0 == time_shift)
        {
            PRINTF("time_shift : 0!\n");
        }

        win_tms_set_string(time_shift,&tms_item_txtset1);
        tms_set_tms2rec_item_active(TRUE);
    }
    else if (PVR_OBJ_KEY_ID_2 == id)
    {
        sys_data_set_tms2rec(!sys_data_get_tms2rec());
        win_tms_set_string(sys_data_get_tms2rec(),&tms_item_txtset2);
    }
    else if (PVR_OBJ_KEY_ID_3 == id )
    {
      sys_data_set_rec_type(!sys_data_get_rec_type());
      win_set_rec_type_string(sys_data_get_rec_type(),&tms_item_txtset3);
    }
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
    else if (PVR_OBJ_KEY_ID_4 == id )
    {
        if(storage_get_volume_count()<=1)
            return;

        win_get_nex_vol(1);
        win_set_vol_string(&tms_item_txtset4);
    }
#endif
    osd_track_object(pobj,C_UPDATE_ALL);
}

static void win_tms_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))

    if (PVR_OBJ_KEY_ID_4 == id)
    {
        OSD_RECT rect;
        COM_POP_LIST_PARAM_T param;

        rect.u_left = CON_L + TXTS_L_OF - 100;
        rect.u_width = TXTS_W+80;
        rect.u_height = 300;
        param.selecttype = POP_LIST_SINGLESELECT;

        if(storage_get_volume_count()<=1)
            return;

        rect.u_top = CON_T + (CON_H + CON_GAP) * (id - 1);
        param.cur = m_volume_index;
        m_volume_index = win_com_open_sub_list(POP_LIST_TYPE_VOLUMELIST,&rect,&param);
        win_set_vol_string(&tms_item_txtset4);
        osd_track_object(pobj, C_UPDATE_ALL);
    }
#endif
}

static PRESULT tms_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = VACT_PASS;
    //UINT8 input;
    UINT8    id = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if(VACT_DECREASE == unact)
        {
            win_tms_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_tms_right_key(pobj,id);
        }
        else if(VACT_ENTER == unact)
        {
            win_tms_enter_key(pobj,id);
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}


static VACTION tms_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;

    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    case V_KEY_RECORD:
        act = VACT_RECORD_ALL;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT tms_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 time_shift = 0;
    UINT8 back_saved = 0;

    pvr_info  = api_get_pvr_info();
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_timeshift,RS_DVR_CONFIGURATION, 0);
        time_shift = sys_data_get_tms();
        if(0 == time_shift)
        {
            PRINTF("time_shift : 0!\n");
        }

        win_tms_set_string(time_shift,&tms_item_txtset1);
        win_tms_set_string(sys_data_get_tms2rec(),&tms_item_txtset2);
        win_set_rec_type_string(sys_data_get_rec_type(),&tms_item_txtset3);
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
        win_pvr_vol_init();
        win_set_vol_string(&tms_item_txtset4);
#endif
        tms_set_tms2rec_item_active(FALSE);
#ifdef NEW_DEMO_FRAME
        osd_set_container_focus(&g_win_timeshift, 4);
        remove_menu_item(&g_win_timeshift, (OBJECT_HEAD *)&tms_item_con1, CON_H + CON_GAP);
        remove_menu_item(&g_win_timeshift, (OBJECT_HEAD *)&tms_item_con2, CON_H + CON_GAP);
        remove_menu_item(&g_win_timeshift, (OBJECT_HEAD *)&tms_item_con3, CON_H + CON_GAP);
#else
        osd_set_container_focus(&g_win_timeshift, 1);
#endif
        break;

    case EVN_POST_OPEN:
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
        win_pvr_vol_invalid_msg();
#endif
        break;

    case EVN_PRE_CLOSE:
        *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
#if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
        win_pvr_vol_save();
#endif
        sys_data_save(1);
        break;

    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        if (api_pvr_is_record_active())
        {
            break;
        }
#if (!defined(CI_PLUS_PVR_SUPPORT) && !defined(CAS9_PVR_SUPPORT)&& !defined(C0200A_PVR_SUPPORT) &&\
    !defined(CAS7_PVR_SUPPORT) && !defined(BC_PVR_STD_SUPPORT) &&!defined(BC_PVR_SUPPORT))
        if (sys_data_get_tms() == 0)
        {
            win_popup_choice_t choice;
            UINT8 back_saved;

            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Are you sure to enter record TP mode?", NULL, 0);
            if (pvr_r_get_record_all())
                win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
            else
                win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            choice = win_compopup_open_ext(&back_saved);
            if(WIN_POP_CHOICE_YES == choice)
            {
                pvr_r_set_record_all(TRUE);
            }
            else if (WIN_POP_CHOICE_NO == choice)
            {
                pvr_r_set_record_all(FALSE);
            }

        }
        else
#endif
        {
#ifdef PVR3_SUPPORT
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Continue TMS support?", NULL, 0);
            if (pvr_r_get_record_all())
            {
                win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
            }
            else
            {
                win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            }
            choice = win_compopup_open_ext(&back_saved);
            if(WIN_POP_CHOICE_YES == choice)
            {
                pvr_info->continue_tms = 1;
            }
            else
            {
                pvr_info->continue_tms = 0;
            }
#endif
        }
       {
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Record CA program in descrambled format?", NULL, 0);
            if (sys_data_get_scramble_record_mode())
            {
                win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            }
            else
            {
                win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
            }
            choice = win_compopup_open_ext(&back_saved);
            if (WIN_POP_CHOICE_YES == choice)
            {
                sys_data_set_scramble_record_mode(0);
            }
            else if (WIN_POP_CHOICE_NO == choice)
            {
                sys_data_set_scramble_record_mode(1);
                pvr_r_set_scramble();
            }
        }
        break;
    default:
        break;
    }
    return ret;
}




////////////////////////////////////////////////
#endif//#ifdef DVR_PVR_SUPPORT
