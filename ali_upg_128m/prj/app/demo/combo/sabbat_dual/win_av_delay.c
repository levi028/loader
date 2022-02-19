/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_av_delay.c
*
*    Description: To realize the function of A/V delay
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#ifdef AV_DELAY_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com.h"
#include "control.h"
#include "win_mpeg_player.h"

/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER win_av_delay_con;

CONTAINER av_delay_item_con1;
//CONTAINER av_delay_item_con2;
//CONTAINER av_delay_item_con3;
TEXT_FIELD av_delay_title;
TEXT_FIELD av_delay_item_txtleft;
TEXT_FIELD av_delay_item_txtright;
TEXT_FIELD av_delay_bar_txtvalue;

BITMAP         av_delay_bmp;
PROGRESS_BAR av_delay_bar;

//TEXT_FIELD av_delay_item_txtset1;
//TEXT_FIELD av_delay_item_txtset2;
//TEXT_FIELD av_delay_item_txtset3;

TEXT_FIELD av_delay_item_line1;
//TEXT_FIELD av_delay_item_line2;
//TEXT_FIELD av_delay_item_line3;

static VACTION av_delay_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT av_delay_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION av_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT av_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX        WSTL_INFO_01_8BIT//WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX    WSTL_INFO_01_8BIT//WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX    WSTL_INFO_01_8BIT//WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX    WSTL_INFO_01_8BIT//WSTL_WIN_BODYRIGHT_01_HD

#define TITLE_TXT_SH_IDX    WSTL_TEXT_24_HD

#define CON_SH_IDX    WSTL_BUTTON_01_HD
#define CON_HL_IDX    WSTL_BUTTON_05_HD
#define CON_HL1_IDX    WSTL_BUTTON_02_HD
#define CON_SL_IDX    WSTL_BUTTON_01_HD
#define CON_GRY_IDX    WSTL_BUTTON_07_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define TXTS_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_HL_IDX   WSTL_BUTTON_04_HD
#define TXTS_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTS_GRY_IDX  WSTL_BUTTON_07_HD

#define BMP_SH_IDX                  WSTL_MIXBACK_WHITE_IDX

#define PROGRESSBAR_SH_IDX            WSTL_BARBG_01_8BIT//WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX        WSTL_MIXBACK_BLACK_IDX//WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX    WSTL_BARBG_01_8BIT//WSTL_BAR_04_HD
#define PROGRESS_TXT_SH_IDX    WSTL_BUTTON_01_HD

#define MSG_SH_IDX WSTL_TEXT_09_HD


#define    W_L         50//248//384
#define    W_T         390 //98//138
#define    W_W         902//692
#define    W_H         230//488

#define TITLE_TXT_L        (W_L + 20)
#define TITLE_TXT_T        (W_T)//30   //70//30
#define TITLE_TXT_W        300//536//402
#define TITLE_TXT_H        50

#define CON_L        (W_L+50)
#define CON_T        (W_T + 70)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXTN_L_OF      10
#define TXTN_W      300//260
#define TXTN_H        40
#define TXTN_T_OF    ((CON_H - TXTN_H)/2)

#define TXTS_L_OF      (TXTN_L_OF + TXTN_W)
#define TXTS_W      (CON_W - 320) //240
#define TXTS_H        40
#define TXTS_T_OF    ((CON_H - TXTN_H)/2)


#define BAR_L    (CON_L+5)
#define BAR_T    (CON_T + (CON_H + CON_GAP)*1+10)
#define BAR_W    (CON_W-BAR_TXT_W-10)
#define BAR_H    24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W    100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H    36

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4


#define POS_ICON_WIDTH 16


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    av_delay_item_con_keymap,av_delay_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTLEFT(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTRIGHT(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,sh,icon)        \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,l,t,w,h,shidx,str_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,str_id,NULL)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh)    \
    DEF_PROGRESSBAR(var_bar, &root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
        NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, WSTL_BAR_06, \
        rl,rt , rw, rh, 1, 1000, 1000, 500)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,str_left,str_right)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTLEFT(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,0,str_left)    \
    LDEF_TXTRIGHT(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,str_right)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    av_con_keymap,av_con_callback,  \
    nxt_obj, focus_id,0)

  LDEF_MM_ITEM(win_av_delay_con,av_delay_item_con1, &av_delay_bar,av_delay_item_txtleft,
           av_delay_item_txtright,av_delay_item_line1,1,1,1,
           CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    display_strs[0],display_strs[1])

 LDEF_BMP(&win_av_delay_con,av_delay_bmp,NULL,0,0,0,0,BMP_SH_IDX,0)

 LDEF_PROGRESS_BAR(win_av_delay_con,av_delay_bar,&av_delay_bar_txtvalue,    \
        BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W - 4,BAR_H - 10)

 LDEF_PROGRESS_TXT(win_av_delay_con, av_delay_bar_txtvalue, NULL, \
        BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H,display_strs[10])

LDEF_TXT_TITLE(&win_av_delay_con,av_delay_title,&av_delay_item_con1,TITLE_TXT_L, TITLE_TXT_T,
           TITLE_TXT_W, TITLE_TXT_H,TITLE_TXT_SH_IDX,RS_AV_DELAY)

 LDEF_WIN(win_av_delay_con,&av_delay_title,W_L,W_T,W_W,W_H,2)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
static ID    avdelay_timer = OSAL_INVALID_ID;
#define   AVDELAY_TIMER_TIME   3000
#define   AVDELAY_TIMER_NAME     "avdelay"

void avdelay_timer_func(UINT unused)
{
    api_stop_timer(&avdelay_timer);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}


static void draw_bookmark(UINT32 pos)
{
    //UINT32 i;
    UINT32 play_pos;
    BITMAP* bmp = &av_delay_bmp;
    PROGRESS_BAR* bar = &av_delay_bar;

    osd_draw_object((POBJECT_HEAD)&win_av_delay_con, C_UPDATE_ALL);

    //TODO: draw bookmarks
    play_pos = pos;
    osd_set_rect(&bmp->head.frame,
        bar->head.frame.u_left + play_pos*BAR_W/1020,
        BAR_T,
        POS_ICON_WIDTH,
        bar->head.frame.u_height);
    osd_set_bitmap_content(bmp, IM_PVR_DOT);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
}


static VACTION av_delay_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT av_delay_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //PROGRESS_BAR* bar = &av_delay_bar;

    return ret;
}

static VACTION av_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
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
static PRESULT av_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 key;
    INT16 val;
    SYSTEM_DATA *psys_data = sys_data_get();
    val = psys_data->avset.avdelay_value;
    UINT8 str[10];
    switch(event)
    {
    case EVN_PRE_OPEN:
        osd_set_text_field_content(&av_delay_item_txtleft, STRING_ANSI, (UINT32)"-500ms");
        osd_set_text_field_content(&av_delay_item_txtright, STRING_ANSI, (UINT32)"+500ms");
        if(val-500 < 0)
            snprintf((char*)str,10,"-%d",500-val);
        else if(val-500 > 0)
            snprintf((char*)str,10,"+%d",val-500);
        else
            snprintf((char*)str,10,"%d",val-500);
        osd_set_text_field_content(&av_delay_bar_txtvalue, STRING_ANSI, (UINT32)str);
        osd_set_progress_bar_pos(&av_delay_bar, 1000);
        api_inc_wnd_count();
        break;

    case EVN_POST_OPEN:
        draw_bookmark(val);
        avdelay_timer = api_start_timer(AVDELAY_TIMER_NAME,AVDELAY_TIMER_TIME,avdelay_timer_func);
        break;

    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        //*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        api_stop_timer(&avdelay_timer);
        sys_data_save(1);
        api_dec_wnd_count();
        //OSD_ClearObject((POBJECT_HEAD )&win_av_delay_con, C_CLOSE_CLRBACK_FLG);
        break;
    case EVN_UNKNOWN_ACTION:
        key = param1&0xffff;
        if((key == V_KEY_RIGHT) || (key == V_KEY_LEFT))
        {
            if(key == V_KEY_RIGHT)
            {
                INT16 val_max = 1000;
                api_stop_timer(&avdelay_timer);
                val += 10;
                if(val > val_max)
                    val = val_max;
                //OSD_SetTextFieldContent(&av_delay_bar_txtvalue, STRING_NUMBER, val-500);
            }
            else
            {
                api_stop_timer(&avdelay_timer);
                val -= 10;
                if(val < 0)
                    val = 0;
            //    sprintf(str,"-%d",500-val);
                //OSD_SetTextFieldContent(&av_delay_bar_txtvalue, STRING_NUMBER, val-500);
            }
            //snprintf(str,10,"");
            MEMSET(str,0,10);
            if(val-500 < 0)
                snprintf((char*)str,10,"-%d",500-val);
            else if(val-500 > 0)
                snprintf((char*)str,10,"+%d",val-500);
            else
                snprintf((char*)str,10,"%d",val-500);

            osd_set_text_field_content(&av_delay_bar_txtvalue, STRING_ANSI, (UINT32)str);
            osd_draw_object((POBJECT_HEAD)&av_delay_bmp, C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD)&av_delay_bar_txtvalue, C_UPDATE_ALL);
            if(check_media_player_is_working())
                api_set_avdelay_value(val, AVDELAY_MEDIA_PLAY_MODE);
            else
                api_set_avdelay_value(val, AVDELAY_LIVE_PLAY_MODE);
            psys_data->avset.avdelay_value = val;

            draw_bookmark(val);
            sys_data_save(1);
            avdelay_timer = api_start_timer(AVDELAY_TIMER_NAME,AVDELAY_TIMER_TIME,avdelay_timer_func);
        }
        break;
    case EVN_UNKNOWNKEY_GOT:
        break;
    case EVN_MSG_GOT:
        if(    param1 == CTRL_MSG_SUBTYPE_CMD_EXIT)
            ret = PROC_LEAVE;
        break;
    }
    return ret;
}

#endif

