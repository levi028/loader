/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_setting.c
*
*    Description: the ui of pvr setting
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
#include <api/libosd/osd_lib.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"

#include "menus_root.h"
#include "win_com_popup.h"
#include "win_pvr_setting.h"
#include "pvr_control.h"

/*******************************************************************************
*    Objects definition
*******************************************************************************/

static VACTION pvr_setting_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT pvr_setting_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION pvr_setting_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT pvr_setting_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    pvr_setting_item_con_keymap,pvr_setting_item_con_callback,  \
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
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset, var_line,ID,idu,idd,l,t,w,h,name_id,setstr)\
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)\
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,  NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, l + LINE_L_OF, t + CON_H, LINE_W, LINE_H, 0, 0)

LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con1, &pvr_setting_item_con2, \
        pvr_setting_item_txtname1,pvr_setting_item_txtset1,pvr_setting_item_line1,1,8,2,CON_L, \
        CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    RS_DVR_TIMESHIFT,display_strs[0])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con2, &pvr_setting_item_con3, \
        pvr_setting_item_txtname2,pvr_setting_item_txtset2,pvr_setting_item_line2,2,1,3,CON_L, \
        CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,    RS_DVR_JUMP,display_strs[1])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con3, &pvr_setting_item_con4, \
        pvr_setting_item_txtname3,pvr_setting_item_txtset3,pvr_setting_item_line3,3,2,4,CON_L, \
        CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,    RS_REC_AS_SCRAMBLED,display_strs[2])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con4, &pvr_setting_item_con5, \
        pvr_setting_item_txtname4,pvr_setting_item_txtset4,pvr_setting_item_line4,4,3,5,CON_L, \
        CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H,    RS_TS_FILE_SIZE,display_strs[3])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con5, &pvr_setting_item_con6, \
        pvr_setting_item_txtname5,pvr_setting_item_txtset5,pvr_setting_item_line5,5,4,6,CON_L, \
        CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H,    RS_TMS_TO_REC,display_strs[4])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con6, &pvr_setting_item_con7, \
        pvr_setting_item_txtname6,pvr_setting_item_txtset6,pvr_setting_item_line6,6,5,7,CON_L, \
        CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H,    RS_POPUP_SUBTITLE_ON,display_strs[5])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con7, &pvr_setting_item_con8,pvr_setting_item_txtname7, \
        pvr_setting_item_txtset7,pvr_setting_item_line7,7,6,8,CON_L, CON_T + (CON_H + CON_GAP)*6, \
        CON_W,CON_H,    RS_REC_NUM,display_strs[6])
LDEF_MM_ITEM(g_win_pvr_setting,pvr_setting_item_con8, NULL,pvr_setting_item_txtname8,pvr_setting_item_txtset8, \
        pvr_setting_item_line8,8,7,1,CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, RS_REC_PS_ENABLE,display_strs[7])

DEF_CONTAINER(g_win_pvr_setting,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    pvr_setting_con_keymap,pvr_setting_con_callback,  \
    (POBJECT_HEAD)&pvr_setting_item_con1, 1,0)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
#define MENU_PVR_SETTING_TIMESHIFT        1
#define MENU_PVR_SETTING_JUMP            2
#define MENU_PVR_SETTING_SCRAMBLE_MODE    3
#define MENU_PVR_SETTING_TS_FILE_SIZE    4
#define MENU_PVR_SETTING_TMS_2_REC        5
#define MENU_PVR_SETTING_TTX_SUBT        6
#define MENU_PVR_SETTING_REC_NUM        7
#define MENU_PVR_SETTING_REC_PS            8

#define VACT_RECORD_ALL     (VACT_PASS+1)

#if 0
CONTAINER *pvr_setting_items[] =
{
    &pvr_setting_item_con1,
    &pvr_setting_item_con2,
    &pvr_setting_item_con3,
    &pvr_setting_item_con4,
    &pvr_setting_item_con5,
    &pvr_setting_item_con6,
    &pvr_setting_item_con7,
    &pvr_setting_item_con8,
};

POBJECT_HEAD pvr_setting_item_name[] =
{
    (POBJECT_HEAD)&pvr_setting_item_txtname1,
    (POBJECT_HEAD)&pvr_setting_item_txtname2,
    (POBJECT_HEAD)&pvr_setting_item_txtname3,
    (POBJECT_HEAD)&pvr_setting_item_txtname4,
    (POBJECT_HEAD)&pvr_setting_item_txtname5,
    (POBJECT_HEAD)&pvr_setting_item_txtname6,
    (POBJECT_HEAD)&pvr_setting_item_txtname7,
    (POBJECT_HEAD)&pvr_setting_item_txtname8,
};

POBJECT_HEAD pvr_setting_item_set[] =
{
    (POBJECT_HEAD)&pvr_setting_item_txtset1,
    (POBJECT_HEAD)&pvr_setting_item_txtset2,
    (POBJECT_HEAD)&pvr_setting_item_txtset3,
    (POBJECT_HEAD)&pvr_setting_item_txtset4,
    (POBJECT_HEAD)&pvr_setting_item_txtset5,
    (POBJECT_HEAD)&pvr_setting_item_txtset6,
    (POBJECT_HEAD)&pvr_setting_item_txtset7,
    (POBJECT_HEAD)&pvr_setting_item_txtset8,
};
#endif

static void win_pvr_setting_timeshift_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%s", (input ? "ON" : "OFF"));
    com_asc_str2uni(temp, display_strs[0]);
}

static void win_pvr_setting_jumpstep_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    switch(input)
    {
        case 0:
            //sprintf(temp, "30 sec", input);
            strncpy((char *)temp, "30 sec", 9);
            break;
        case 1:
            //sprintf(temp, "1 min", input);
            strncpy((char *)temp, "1 min", 9);
            break;
        case 2:
            //sprintf(temp, "5 min", input);
            strncpy((char *)temp, "5 min", 9);
            break;
        case 3:
            //sprintf(temp, "10 min", input);
            strncpy((char *)temp, "10 min", 9);
            break;
        case 4:
            //sprintf(temp, "30 min", input);
            strncpy((char *)temp, "30 min", 9);
            break;
        default:
            break;
    }
    com_asc_str2uni(temp, display_strs[1]);
}

static void win_pvr_setting_scramble_record_mode_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%s", (input ? "NO" : "YES"));
    com_asc_str2uni(temp, display_strs[2]);
}

static void win_pvr_setting_ts_file_size_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};
    char *  __MAYBE_UNUSED__ ret = NULL;
    UINT8 cnt = ARRAY_SIZE(temp)-1;

    switch(input)
    {
        case 0:
            ret = strncpy((char *)temp, "1 G", cnt);
            break;
        case 1:
            ret = strncpy((char *)temp, "2 G", cnt);
            break;
        case 2:
            ret = strncpy((char *)temp, "3 G", cnt);
            break;
        case 3:
            ret = strncpy((char *)temp, "4 G", cnt);
            break;
        default:
            break;
    }
    com_asc_str2uni(temp, display_strs[3]);
}

static void win_pvr_setting_tms2rec_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%s", (input ? "ON" : "OFF"));
    com_asc_str2uni(temp, display_strs[4]);
}

static void win_pvr_setting_ttx_subt_set_string(BOOL input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%s", (input ? "YES" : "NO"));
    com_asc_str2uni(temp, display_strs[5]);
}

static void win_pvr_setting_rec_num_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%d", input);
    com_asc_str2uni(temp, display_strs[6]);
}

static void win_pvr_setting_record_ps_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};

    snprintf((char *)temp, 10, "%s", (input ? "ON" : "OFF"));
    com_asc_str2uni(temp, display_strs[7]);
}

static void win_pvr_setting_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    //SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT8 input = 0;
    struct dvr_hdd_info hdd_info;
    UINT8 ret = 0;
    UINT8 rec_ps = 0;
    UINT8 time_shift = 0;

    MEMSET(&hdd_info,0,sizeof(struct dvr_hdd_info));
    switch(id)
    {
        case MENU_PVR_SETTING_TIMESHIFT:
            ret = !sys_data_get_tms();
            sys_data_set_tms(ret);
            time_shift = sys_data_get_tms();
            if(0 == time_shift)
            {
                PRINTF("time_shift: 0!\n");
            }
            win_pvr_setting_timeshift_set_string(time_shift);
            osd_track_object(pobj,C_UPDATE_ALL);
            if (sys_data_get_tms())
            {
                pvr_r_set_record_all(FALSE);//if TMS on, close TP record.
                api_pvr_adjust_tms_space();
                /*
                if (pvr_get_HDD_info(&hdd_info) && (hdd_info.tms_size == 0))
                {
                    pvr_set_tms_size(hdd_info.free_size);  // set free space to tms
                    pvr_get_HDD_info(&hdd_info);
                }
                */
            }
            else
            {
                pvr_set_tms_size(0);
            }

            pvr_get_hdd_info(&hdd_info);
            api_pvr_check_level(&hdd_info);
            break;
        case MENU_PVR_SETTING_JUMP:
            input = sys_data_get_jumpstep();
            if(!input)
            {
                input = 5;
            }
            input = (input-1)%5;
            sys_data_set_jumpstep(input);
            win_pvr_setting_jumpstep_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_SCRAMBLE_MODE:
            input = sys_data_get_scramble_record_mode();
            input = !input;
            sys_data_set_scramble_record_mode(input);
            pvr_r_set_scramble_ext(!input);
            win_pvr_setting_scramble_record_mode_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TS_FILE_SIZE:
            input = sys_data_get_record_ts_file_size();
            if(!input)
            {
                input = 4;
            }
            input = (input-1)%4;
            sys_data_set_record_ts_file_size(input);
            win_pvr_setting_ts_file_size_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TMS_2_REC:
            ret = !sys_data_get_tms2rec();
            sys_data_set_tms2rec(ret);
            win_pvr_setting_tms2rec_set_string(sys_data_get_tms2rec());
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TTX_SUBT:
            input = sys_data_get_record_ttx_subt();
            input = !input;
            sys_data_set_record_ttx_subt(input);
            win_pvr_setting_ttx_subt_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_REC_NUM:
            ret = 3 - sys_data_get_rec_num();
            sys_data_set_rec_num(ret);
            win_pvr_setting_rec_num_set_string(sys_data_get_rec_num());
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_REC_PS:
            rec_ps = sys_data_get_rec_ps();
            if(0 == rec_ps)
            {
                PRINTF("Bad rec_pos!\n");
            }
            sys_data_set_rec_ps(!rec_ps);
            rec_ps = sys_data_get_rec_ps();
            if(0 == rec_ps)
            {
                PRINTF("Bad rec_pos!\n");
            }
            win_pvr_setting_record_ps_set_string(rec_ps);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_pvr_setting_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    //SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT8 input = 0;
    struct dvr_hdd_info hdd_info;
    UINT8 rec_ps = 0;
    UINT8 time_shift = 0;

    MEMSET(&hdd_info,0,sizeof(struct dvr_hdd_info));
    switch(id)
    {
        case MENU_PVR_SETTING_TIMESHIFT:
            sys_data_set_tms(!sys_data_get_tms());
            time_shift = sys_data_get_tms();
            if(0 == time_shift)
            {
                PRINTF("time_shift: 0!\n");
            }
            win_pvr_setting_timeshift_set_string(time_shift);
            osd_track_object(pobj,C_UPDATE_ALL);
            if (sys_data_get_tms())
            {
                pvr_r_set_record_all(FALSE);//if TMS on, close TP record.
                api_pvr_adjust_tms_space();
                /*
                if (pvr_get_HDD_info(&hdd_info) && (hdd_info.tms_size == 0))
                {
                    pvr_set_tms_size(hdd_info.free_size); // set free space to tms
                    pvr_get_HDD_info(&hdd_info);
                }
                */
            }
            else
            {
                pvr_set_tms_size(0);
            }
            pvr_get_hdd_info(&hdd_info);
            api_pvr_check_level(&hdd_info);
            break;
        case MENU_PVR_SETTING_JUMP:
            input = sys_data_get_jumpstep();
            input = (input+1)%5;
            sys_data_set_jumpstep(input);
            win_pvr_setting_jumpstep_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_SCRAMBLE_MODE:
            input = sys_data_get_scramble_record_mode();
            input = !input;
            sys_data_set_scramble_record_mode(input);
            pvr_r_set_scramble_ext(!input);
            win_pvr_setting_scramble_record_mode_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TS_FILE_SIZE:
            input = sys_data_get_record_ts_file_size();
            input = (input+1)%4;
            sys_data_set_record_ts_file_size(input);
            win_pvr_setting_ts_file_size_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TMS_2_REC:
            sys_data_set_tms2rec(!sys_data_get_tms2rec());
            win_pvr_setting_tms2rec_set_string(sys_data_get_tms2rec());
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_TTX_SUBT:
            input = sys_data_get_record_ttx_subt();
            input = !input;
            sys_data_set_record_ttx_subt(input);
            win_pvr_setting_ttx_subt_set_string(input);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_REC_NUM:
            sys_data_set_rec_num(3 - sys_data_get_rec_num());
            win_pvr_setting_rec_num_set_string(sys_data_get_rec_num());
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PVR_SETTING_REC_PS:
            rec_ps = sys_data_get_rec_ps();
            if(0 == rec_ps)
            {
                PRINTF("Bad rec_pos!\n");
            }
            sys_data_set_rec_ps(!rec_ps);

            rec_ps = sys_data_get_rec_ps();
            if(0 == rec_ps)
            {
                PRINTF("Bad rec_pos!\n");
            }
            win_pvr_setting_record_ps_set_string(rec_ps);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION pvr_setting_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
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

static PRESULT pvr_setting_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    unact = VACT_PASS;
    //BOOL    input;
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
            win_pvr_setting_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_pvr_setting_right_key(pobj,id);
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION pvr_setting_con_keymap(POBJECT_HEAD pobj, UINT32 key)
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
//        case V_KEY_ENTER:
//            act = VACT_ENTER;
//            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT pvr_setting_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT8    unact;
    //UINT32 i;
    //BOOL subt_flag;
    //CONTAINER *cont = (CONTAINER*)pobj;
    //UINT8    id = OSD_GetObjID(pobj);
    //static UINT8 preval = 0;
    SYSTEM_DATA *p_sys_data = NULL;

    switch(event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&win_osdset_con, RS_PVR_SETTING, 0);
            p_sys_data=sys_data_get();
            win_pvr_setting_timeshift_set_string(p_sys_data->timeshift);
            win_pvr_setting_jumpstep_set_string(p_sys_data->jumpstep);
            win_pvr_setting_scramble_record_mode_set_string(p_sys_data->scramble_record_mode);
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con3, CON_H + CON_GAP);
            win_pvr_setting_ts_file_size_set_string(p_sys_data->record_ts_file_size);
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con4, CON_H + CON_GAP);
            win_pvr_setting_tms2rec_set_string(p_sys_data->tms2rec);
            win_pvr_setting_ttx_subt_set_string(p_sys_data->record_ttx_subt);
#ifdef _INVW_JUICE
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con5, CON_H + CON_GAP);
#endif
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con6, CON_H + CON_GAP);
            win_pvr_setting_rec_num_set_string(p_sys_data->rec_num);
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con7, CON_H + CON_GAP);
#ifdef NEW_DEMO_FRAME
            win_pvr_setting_record_ps_set_string(p_sys_data->rec_ps_enable);
    #if defined( SUPPORT_CAS9) || defined( SUPPORT_CAS7) ||defined (SUPPORT_BC_STD) \
        ||defined (SUPPORT_BC) ||defined(FTA_ONLY) || defined(BC_PVR_SUPPORT) || defined(SUPPORT_C0200A) || \
        defined(PVR_FEATURE_SIMPLIFIED)
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con5, CON_H + CON_GAP);
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con8, CON_H + CON_GAP);
    #endif
#else
            remove_menu_item(&g_win_pvr_setting, (OBJECT_HEAD *)&pvr_setting_item_con8, CON_H + CON_GAP);
#endif
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
               *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
            p_sys_data=sys_data_get();
            sys_data_save(1);
#ifndef PVR3_SUPPORT
            pvr_r_set_ts_file_size(p_sys_data->record_ts_file_size + 1);
#endif
            //api_pvr_jumpstep_trans();
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_UNKNOWN_ACTION:
            if (api_pvr_is_record_active())
        {
                break;
            }
            //win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
            //UINT8 back_saved = 0;

#if (!defined(CI_PLUS_PVR_SUPPORT)&& !defined(C0200A_PVR_SUPPORT) && !defined(CAS9_PVR_SUPPORT)\
        &&!defined(CAS7_PVR_SUPPORT) && !defined(CAS9_PVR_SCRAMBLED) &&\
        !defined(BC_PVR_STD_SUPPORT) &&!defined(BC_PVR_SUPPORT)\
        &&!defined(_GEN_CA_ENABLE_) && !defined(PVR_FEATURE_SIMPLIFIED))
            if (sys_data_get_tms() == 0)
            {
                win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
                UINT8 back_saved = 0;
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("Are you sure to enter record TP mode?", NULL, 0);
                if (pvr_r_get_record_all())
                    win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
                else
                    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                choice = win_compopup_open_ext(&back_saved);
                if(choice == WIN_POP_CHOICE_YES)
                {
                    pvr_r_set_record_all(TRUE);
                }
                else if (choice == WIN_POP_CHOICE_NO)
                {
                    pvr_r_set_record_all(FALSE);
                }

            }
            else
#endif
#if (!(defined(CAS9_PVR_SUPPORT)||defined(CAS7_PVR_SUPPORT) ||defined(C0200A_PVR_SUPPORT)\
        || defined(CAS9_PVR_SCRAMBLED) || defined(BC_PVR_STD_SUPPORT)||defined(BC_PVR_SUPPORT)\
        || defined(_GEN_CA_ENABLE_) || defined(PVR_FEATURE_SIMPLIFIED)))
            {
                win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
                UINT8 back_saved = 0;

                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("Record CA program in descrambled format?", NULL, 0);
                if (sys_data_get_scramble_record_mode())
                    win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
                else
                    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                choice = win_compopup_open_ext(&back_saved);
                if (choice == WIN_POP_CHOICE_YES)
                {
                    sys_data_set_scramble_record_mode(1);
                    pvr_r_set_scramble_ext(0);
                }
                else if (choice == WIN_POP_CHOICE_NO)
                {
                    sys_data_set_scramble_record_mode(0);
                    pvr_r_set_scramble_ext(1);
                }
            }
#endif
            break;
    default:
        break;
    }

    return ret;
}

#endif//#ifdef DVR_PVR_SUPPORT
