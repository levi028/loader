/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_combo_ota.c
*
*    Description: To realize the combo UI of OTA
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libpub/lib_frontend.h>
#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "menus_root.h"
#include "win_com.h"
#include "win_combo_ota.h"
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
#include "platform/board.h"

#define FRONT_END_NUM   2

#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTI_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTI_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD


#define TXTC_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTC_GRY_IDX  WSTL_BUTTON_07_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT
//sharon WSTL_SCROLLBAR_02_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define SCB_L         (W_L + 20)
#define SCB_T         110//150
#define SCB_W         12//sharon 18
#define SCB_H         460

#define CON_L        (SCB_L + SCB_W + 10)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 72 - (SCB_W + 4))
#define CON_H        40
#define CON_GAP     12

#define TXTN_L_OF    10
#define TXTN_T_OF      0
#define TXTN_W        (CON_W-20)
#define TXTN_H         40

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#else
#define    W_L         105
#define    W_T         57
#define    W_W         482
#define    W_H         370

#define SCB_L         (W_L + 20)
#define SCB_T         110//150
#define SCB_W         12//sharon 18
#define SCB_H         360

#define CON_L        (SCB_L + SCB_W )
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 30 - (SCB_W + 4))
#define CON_H        32
#define CON_GAP     8

#define TXTN_L_OF    10
#define TXTN_T_OF      0
#define TXTN_W        (CON_W-20)
#define TXTN_H         30

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4
#endif

CONTAINER   win_ota_combo;

static PRESULT combo_item_con_callback(POBJECT_HEAD pobj, \
              VEVENT event, UINT32 param1, UINT32 param2);
static VACTION combo_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);

static VACTION combo_ota_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT combo_ota_callback(POBJECT_HEAD pobj,\
               VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,\
    ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_INACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    combo_item_con_keymap,combo_item_con_callback,  \
    conobj, ID,1)


#define LDEF_TXTNAME(root,var_txt,nxt_obj,\
    ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_INACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,\
    var_txt_name,ID,idu,idd,l,t,w,h,name_id)   \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,NULL,\
    1,1,1,1,1,l + TXTN_L_OF, t + TXTN_T_OF,\
    TXTN_W,TXTN_H,name_id,NULL)


LDEF_MM_ITEM(win_ota_combo,combo_ota_con1,\
           &combo_ota_con2,combo_ota_txt1,\
       1,2,2,CON_L, CON_T + (CON_H + CON_GAP)*0 ,  \
           CON_W,CON_H,0)

LDEF_MM_ITEM(win_ota_combo,combo_ota_con2,\
           NULL,combo_ota_txt2,2,1,1,CON_L,\
        CON_T + (CON_H + CON_GAP)*1,   \
            CON_W,CON_H,0)




DEF_CONTAINER(win_ota_combo,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, \
    WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    combo_ota_keymap,combo_ota_callback,  \
    &combo_ota_con1, 1,0)


static TEXT_FIELD *text_array[] =
{
    &combo_ota_txt1,
    &combo_ota_txt2,
};

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

static void combo_ota_ui_option_name_set(void);
static void combo_ota_enter_ota_uprade(POBJECT_HEAD pobj);
static UINT8 combo_ota_ui_config_frontend(void);
/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION combo_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION ret= VACT_PASS;

    switch(key)
    {
        case V_KEY_ENTER:
            ret = VACT_ENTER;
            break;
        default:
            break;
    }
    return ret;

}
static PRESULT combo_item_con_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            combo_ota_enter_ota_uprade(pobj);
            break;
        default:
            break;
    }
    return ret;
}
static VACTION combo_ota_keymap(POBJECT_HEAD pobj, UINT32 key)
{

    VACTION ret= VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
            ret = VACT_CLOSE;
            break;
        case V_KEY_UP:
            ret = VACT_CURSOR_UP;
            break;
        case V_KEY_DOWN:
            ret = VACT_CURSOR_DOWN;
            break;
        default:
        break;
    }
    return ret;
}
static PRESULT combo_ota_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    POBJECT_HEAD    pfocusobj = NULL;

    MEMSET(&pfocusobj,0,sizeof(POBJECT_HEAD));
    switch(event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title(pobj,RS_TOOLS_SW_UPGRADE_BY_OTA, 0);
            combo_ota_ui_option_name_set();
            pfocusobj = osd_get_focus_object(pobj);
            if(pfocusobj != NULL)
            {
                if(!osd_check_attr(pfocusobj,C_ATTR_ACTIVE))
                {
                    pfocusobj = osd_get_adjacent_object\
            (pfocusobj,VACT_CURSOR_DOWN);
             if((pfocusobj != NULL) && osd_check_attr(pfocusobj,C_ATTR_ACTIVE) )
            {
                        osd_change_focus(pobj,pfocusobj->b_id,0);
            }
                }
            }
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        case EVN_POST_CLOSE:
            break;
        default:
            break;
    }
    return ret;
}


static void combo_ota_ui_option_name_set(void)
{
    INT i=0;
    UINT8 fe_type=0;
    UINT8 front_end_type_num=0;
    BOOL  active_flag = FALSE;


    SYSTEM_DATA *sys_data = sys_data_get();

    if(ANTENNA_CONNECT_DUAL_DIFF == sys_data->antenna_connect_type)
    {
        front_end_type_num = 2;
    }
    else
    {
        front_end_type_num = 1;
    }
    front_end_type_num = combo_ota_ui_config_frontend();
    if(NUM_ONE==front_end_type_num)
    {
        remove_menu_item(&win_ota_combo, \
    (OBJECT_HEAD*)&combo_ota_con2, CON_H + CON_GAP);
    }
    for(i = 0;i < front_end_type_num;i++)
    {
        text_array[i]->head.p_root->b_attr = C_ATTR_ACTIVE;
        text_array[i]->head.b_attr = C_ATTR_ACTIVE;

        fe_type = board_get_frontend_type(i);
        switch(fe_type)
        {
            case FRONTEND_TYPE_S:
                text_array[i]->w_string_id = RS_COMBO_OTA_S;
                if(NUM_ZERO == get_tuner_sat_cnt(TUNER_EITHER))
                {
                    text_array[i]->head.p_root->b_attr = C_ATTR_INACTIVE;
                    text_array[i]->head.b_attr = C_ATTR_INACTIVE;
                }
                else
                {
                    text_array[i]->head.p_root->b_attr = C_ATTR_ACTIVE;
                    text_array[i]->head.b_attr = C_ATTR_ACTIVE;
                    active_flag = TRUE;
                }
                break;
            case FRONTEND_TYPE_ISDBT:
            case FRONTEND_TYPE_T:
                if(board_frontend_is_isdbt(i))
        {
                    text_array[i]->w_string_id = RS_COMBO_OTA_ISDBT;
        }
                else
        {
                    text_array[i]->w_string_id = RS_COMBO_OTA_T;
        }
                active_flag = TRUE;
                break;
            case FRONTEND_TYPE_C:
                text_array[i]->w_string_id = RS_COMBO_OTA_C;
                active_flag = TRUE;
                break;
            default:
                break;
        }
    }
    if(!active_flag)
    {
        win_ota_combo.focus_object_id = 0;
    }

    if((NUM_ZERO==win_ota_combo.focus_object_id )&& (active_flag))
    {
        win_ota_combo.focus_object_id = 1;
    }
}


static UINT8 combo_ota_ui_config_frontend(void)
{
    INT32 i=0;
    UINT8 count=0;
    UINT8 s_front = 0;
    UINT8 t_front = 0;
    UINT8 isdbt_front = 0;
    UINT8 c_front = 0;

    for(i = 0; i < FRONT_END_NUM; i++)
    {
        switch(board_get_frontend_type(i))
        {
            case FRONTEND_TYPE_S:
                s_front++;
                break;
            case FRONTEND_TYPE_ISDBT:
            case FRONTEND_TYPE_T:
                if(board_frontend_is_isdbt(i))
        {
                    isdbt_front++;
        }
                else
        {
                    t_front++;
        }
                break;
            case FRONTEND_TYPE_C:
                c_front++;
                break;
            default:
                break;
        }
    }

    if((FRONT_END_NUM==s_front ) || (FRONT_END_NUM==t_front)||\
      (FRONT_END_NUM==isdbt_front )||( FRONT_END_NUM==c_front )||\
      (NUM_ONE==(s_front + t_front + isdbt_front + c_front )))
    {
        count = 1;
    }
    else
    {
        if(FRONT_END_NUM==(s_front + t_front + isdbt_front + c_front ))
        {
            count = 2;
        }
    }

    return count;
}



static void combo_ota_enter_ota_uprade(POBJECT_HEAD pobj)
{
    PCONTAINER pcon = NULL;
    PCONTAINER pwin = NULL;
    UINT32     param=0;

    MEMSET(&pcon,0,sizeof(PCONTAINER));
    MEMSET(&pwin,0,sizeof(PCONTAINER));
    pcon = (PCONTAINER)pobj;
    pwin = &win_otaupg_con;
    param = MENU_OPEN_TYPE_MENU;

    if(RS_COMBO_OTA_S == ((PTEXT_FIELD)(pcon->p_next_in_cntn))->w_string_id)
    {
        param |= FRONTEND_TYPE_S;
    }
    else if(RS_COMBO_OTA_T == ((PTEXT_FIELD)(pcon->p_next_in_cntn))->w_string_id)
    {
        param |= FRONTEND_TYPE_T;
    }
    else if(RS_COMBO_OTA_ISDBT == ((PTEXT_FIELD)(pcon->p_next_in_cntn))->w_string_id)
    {
        param |= FRONTEND_TYPE_ISDBT;
    }
    else if(RS_COMBO_OTA_C == ((PTEXT_FIELD)(pcon->p_next_in_cntn))->w_string_id)
    {
        param |= FRONTEND_TYPE_C;
    }

    if(osd_obj_open((POBJECT_HEAD)pwin, param) != PROC_LEAVE)
    {
        menu_stack_push((POBJECT_HEAD)pwin);
    }
}
