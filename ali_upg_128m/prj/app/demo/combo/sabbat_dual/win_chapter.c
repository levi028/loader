 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_Chapter.c
*
*    Description: Mediaplayer chapter switch.
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
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libmp/mp_subtitle_api.h>
#include <api/libmp/media_player_api.h>

#include "copper_common/system_data.h"
#include "copper_common/com_api.h"
#include "osdobjs_def.h"
#include "win_com_menu_define.h"
#include "win_com_popup.h"
#include "menus_root.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "win_subtitle.h"
#include "win_com.h"
#include "win_chapter.h"
#include "win_mpeg_player.h"
#include "control.h"

/*******************Macro***********************/

#define CHAPTER_ITEM_NUMBER 30
#define VIDEO_SEARCH_DLYS 2
#define MP_TRICK_TIMEOUT    100// 4000 // 555


/*******************************************************
* function declaration
********************************************************/
static void win_com_set_idx_str2(UINT8 *idx, UINT16 *str );
static VACTION chapter_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT chapter_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT chapter_list_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION chapter_list_keymap(POBJECT_HEAD obj, UINT32 key);

/*******************************************************************************
*    Objects definition
*******************************************************************************/

CONTAINER    g_cnt_chapter=
{
    {
        OT_CONTAINER,     //object type
        C_ATTR_ACTIVE,    // defined in form C_ATTR_XXX
        C_FONT_DEFAULT,   // defined in form C_FONT_XXX
        0,                // 0: for component without focus

        0,                //UINT8 bLeftID;
        0,                //UINT8 bRightID;
        0,                //UINT8 bUpID;
        0,                //UINT8 bDownID;

        {SUBTITLE_L, SUBTITLE_T, SUBTITLE_W, SUBTITLE_H},//OSD_RECT frame;
        {WSTL_WIN_SLIST_02_8BIT, WSTL_WIN_SLIST_02_8BIT, WSTL_WIN_SLIST_02_8BIT, WSTL_WIN_SLIST_02_8BIT},
        //COLOR_STYLE style;

        chapter_con_keymap,       //PFN_KEY_MAP pfnKeyMap;
        chapter_con_callback,     //PFN_CALLBACK pfnCallback;

        NULL,                     //POBJECT_HEAD pNext;
        NULL                      //POBJECT_HEAD pRoot;
    },
    (POBJECT_HEAD)&g_txt_chapter,
    1,                            //focus
    0
};

TEXT_FIELD    g_txt_chapter =
{
    {
        OT_TEXTFIELD,          //object type
        C_ATTR_ACTIVE,         // defined in form C_ATTR_XXX
        C_FONT_DEFAULT,        // defined in form C_FONT_XXX
        0,                     // 0: for component without focus

        0,                      //UINT8 bLeftID;
        0,                    //UINT8 bRightID;
        0,                    //UINT8 bUpID;
        0,                    //UINT8 bDownID;

        {SUBTITLE_TXT_L, SUBTITLE_TXT_T, SUBTITLE_TXT_W, SUBTITLE_TXT_H},//OSD_RECT frame;
        {WSTL_MIXBACK_IDX_04_8BIT, WSTL_MIXBACK_IDX_04_8BIT, WSTL_MIXBACK_IDX_04_8BIT, 0},//COLOR_STYLE style;

        NULL,                //PFN_KEY_MAP pfnKeyMap;
        NULL,                //PFN_CALLBACK pfnCallback;

        (POBJECT_HEAD)&g_ls_chapter,      //POBJECT_HEAD pNext;
        (POBJECT_HEAD)&g_cnt_chapter,     //POBJECT_HEAD pRoot;
    },
    C_ALIGN_CENTER | C_ALIGN_VCENTER,     //UINT8 bAlign;
    0,         //UINT8 bX;
    0,         //UINT8 bY;
    RS_CHAPTER,  //UINT16 wStringID;  // string id
    NULL,                                  //UINT16* pString;   // unicode string pointer
};

#define LDEF_SET_LIST(root, var_con,nxt_obj,l,t,w,h,field,numfield,page,bar,intervaly,keymap,callback)        \
    DEF_LIST(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1,l,t,w,h, LIST_SH_IDX,LIST_HL_IDX,LIST_SL_IDX,LIST_GRY_IDX,\
    keymap,callback,LIST_FULL_PAGE| LIST_PAGE_KEEP_CURITEM|LIST_GRID|LIST_SCROLL,\
    field,numfield,page,(SCROLL_BAR* )bar,intervaly,NULL,0 ,0 ,0 ,0 )

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb,root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)
/*******************
Global Variable*
**********************/
static UINT16    chapter_track_idx[CHAPTER_ITEM_NUMBER][5] = {{0}};   //list num, clear warning
static UINT16    chapter_track_str[CHAPTER_ITEM_NUMBER][30] = {{0}};  //list content string, clear warning
static UINT16 *p_chapter_track_str[CHAPTER_ITEM_NUMBER] = {NULL};
static UINT16 *p_chapter_track_idx[CHAPTER_ITEM_NUMBER] = {NULL};
static LISTFIELD list_field_chapter[] =
{
    {80,4,0,0,0,C_ALIGN_LEFT|C_ALIGN_VCENTER,FIELD_IS_TEXT,p_chapter_track_idx},
    {SUBTITLE_LS_W-80,0,0,0,0,C_ALIGN_LEFT|C_ALIGN_VCENTER,FIELD_IS_TEXT,p_chapter_track_str},
};    //every list has ID and content


LDEF_SET_LIST(&g_cnt_chapter,g_ls_chapter,&g_sb_chapter, \
                SUBTITLE_LS_L, SUBTITLE_LS_T, SUBTITLE_LS_W, SUBTITLE_LS_H,\
                list_field_chapter,2,SUBTITLE_LS_PAGE,&g_sb_chapter,0,\
                chapter_list_keymap,chapter_list_callback);
LDEF_LISTBAR(&g_cnt_chapter,g_sb_chapter,\
                SUBTITLE_LS_PAGE,SUBTITLE_SB_L, SUBTITLE_SB_T, SUBTITLE_SB_W, SUBTITLE_SB_H);


/*****************************************************
Interface : void GetListChapterInfo(UINT16 top);
Function  : set each item's display content of list
Author    : Wu Jianwen
Date      : 2010.8.25
Input     : top:first item of list
Output    : NONE
Return    : NONE
******************************************************/
static void get_list_chapter_info(INT16 top)
{
    //UINT8 len = 0;
    UINT8 i = 0;
    INT32 hh = 0;    //start time
    INT32 mm = 0;
    INT32 ss = 0;
    INT32 hh2 = 0;    //end time
    INT32 mm2 = 0;
    INT32 ss2 = 0;
    UINT8 arr_data_temp[50] = {0};
    UINT8  back_saved = 0;
    INT32 i_total_chapter = 0;      //the total chapter of the video
    //INT32 i_CurChapStartTime = 0;   //current chapter start time
    //INT32 i_CurChapEndTime = 0;     //current chapter end time
    INT32 i_dest_chap_start_time = 0;  //Dest chapter start time
    INT32 i_dest_chap_end_time = 0;    //Dest chapter end time
    INT32 i_dest_chapter = 0;        //dest chapter num
    INT32  __MAYBE_UNUSED__ i_cur_chapter = 1;         //current chapter num
    INT32 i_chapter_change = 0;
    DEC_CHAPTER_INFO t_chapter_info;

    MEMSET(&t_chapter_info, 0x0, sizeof(DEC_CHAPTER_INFO));
    t_chapter_info.dst_chapter = -1;
    mpg_file_get_chapter_info(&t_chapter_info);   //get the current chapter info
    i_cur_chapter = t_chapter_info.cur_chapter;   //begin with chapter
    i_total_chapter = t_chapter_info.nb_chapter;

    if(i_total_chapter > 1)   //only chapter num > 1 can switch
    {
        if(top >= i_total_chapter) //top from 0.
        {
            top = 0;
            osd_set_list_cur_pos(&g_ls_chapter,0);
            osd_set_list_top(&g_ls_chapter,0);
        }
        i = 0;
        for(i = 0;i < CHAPTER_ITEM_NUMBER;i++)
        {
            i_chapter_change = top + i;
            if(i_chapter_change < i_total_chapter)
            {
                i_dest_chapter = i_chapter_change;
                t_chapter_info.dst_chapter = i_dest_chapter;
                mpg_file_get_chapter_info(&t_chapter_info);
                i_dest_chap_start_time = t_chapter_info.dst_start_time / 1000;
                hh = i_dest_chap_start_time / 3600;
                mm = (i_dest_chap_start_time % 3600) / 60;
                ss = i_dest_chap_start_time % 60;
                i_dest_chap_end_time = t_chapter_info.dst_end_time / 1000;
                hh2 = i_dest_chap_end_time / 3600;
                mm2 = (i_dest_chap_end_time % 3600) / 60;
                ss2 = i_dest_chap_end_time % 60;

                arr_data_temp[0] = hh;
                arr_data_temp[1] = mm;
                arr_data_temp[2] = ss;
                arr_data_temp[3] = hh2;
                arr_data_temp[4] = mm2;
                arr_data_temp[5] = ss2;
                win_com_set_idx_str2(arr_data_temp, &chapter_track_str[i][0]);
                win_com_set_idx_str(top+i, chapter_track_idx[i], 2);
            }
            else
            {
                //MEMSET(&Chapter_track_idx[i],0,40);
                break;
            }
        }
    }
    else
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("There is only one chapter!",NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1500);
        win_compopup_smsg_restoreback();
    }
}


/*****************************************************
Interface : void ListChapterInfoInit(void);
Function  : init chapter list
Author    : Wu Jianwen
Date      : 2010.8.25
Input     : NONE
Output    : NONE
Return    : NONE
******************************************************/
static void list_chapter_info_init(void)
{
    UINT16 top = 0;
    UINT32 i_total_chapter = 0;      //the total chapter of the video
    INT32 i_cur_chapter = 1;         //current chapter num
    DEC_CHAPTER_INFO t_chapter_info;

    MEMSET(&t_chapter_info, 0x0, sizeof(t_chapter_info));
    t_chapter_info.dst_chapter = -1;
    mpg_file_get_chapter_info(&t_chapter_info);   //get the current chapter info
    i_cur_chapter = t_chapter_info.cur_chapter;   //begin with chapter
    i_total_chapter = t_chapter_info.nb_chapter;

    osd_set_list_count(&g_ls_chapter, i_total_chapter);
    osd_set_list_cur_pos(&g_ls_chapter, i_cur_chapter);
    osd_set_list_top(&g_ls_chapter, i_cur_chapter);
    osd_set_list_ver_scroll_bar(&g_ls_chapter, &g_sb_chapter);

    top = osd_get_list_top_point(&g_ls_chapter);
    get_list_chapter_info(top);
}



/*****************************************************
Interface : VACTION chapter_con_keymap(POBJECT_HEAD obj, UINT32 key);
Function  : keymap of chapter container
Author    : Wu Jianwen
Date      : 2010.8.25
Input     : obj:container; key:v_Key of press button
Output    : NONE
Return    : NONE
******************************************************/
VACTION chapter_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_SUBTITLE:
            act = VACT_CLOSE;
            break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = osd_container_key_map(obj,key);
            break;
    }

    return act;
}


/*****************************************************
Interface : static PRESULT chapter_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
Function  : callback of chapter container
Author    : Wu Jianwen
Date      : 2010.8.25
Input     :
Output    : NONE
Return    :
******************************************************/
static PRESULT chapter_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 i = 0;
    UINT16 wait_time = 0;
    UINT8 unact = 0;
    INT32 i_dest_chapter = 0;        //dest chapter num
    INT32 i_dest_chap_start_time = 0;
    //save chapter info of the stream
    DEC_CHAPTER_INFO t_chapter_info;

    MEMSET(&t_chapter_info, 0x0, sizeof(DEC_CHAPTER_INFO));
    switch(event)
    {
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_EXIT == param1)
        {
            ret = PROC_LEAVE;
        }
#ifdef USB_MP_SUPPORT
        else if(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER == param1)
        {
            ret = PROC_LEAVE;
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,0,TRUE);
        }
#endif
        break;

    case EVN_PRE_OPEN:
        api_set_deo_layer(0);
        api_inc_wnd_count();

        for(i=0;i<CHAPTER_ITEM_NUMBER;i++)
        {
            p_chapter_track_str[i]=chapter_track_str[i];
            p_chapter_track_idx[i]=chapter_track_idx[i];
        }
        list_chapter_info_init();
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            i_dest_chapter = osd_get_list_cur_point(&g_ls_chapter);  //get current chapter
            t_chapter_info.dst_chapter = i_dest_chapter;
            mpg_file_get_chapter_info(&t_chapter_info);
            //i_DestChapStartTime = t_ChapterInfo.dst_start_time/1000;   //second
            i_dest_chap_start_time = t_chapter_info.dst_start_time;   //ms
            if(t_chapter_info.dst_start_time > t_chapter_info.cur_start_time)
            {
                //mpg_cmd_search_proc((DWORD)i_DestChapStartTime);
                mpg_cmd_search_ms_proc((DWORD)i_dest_chap_start_time);
                while ((i_dest_chap_start_time > (INT32)mpgfile_decoder_get_play_time()) && (wait_time < MP_TRICK_TIMEOUT))
                {
                    osal_task_sleep(2);
                    wait_time++;
                }
            }
            else                  //chapter18 to chapter1
            {
                //mpg_cmd_search_proc((DWORD)i_DestChapStartTime);
                mpg_cmd_search_ms_proc((DWORD)i_dest_chap_start_time);
                while (((i_dest_chap_start_time + VIDEO_SEARCH_DLYS) < (INT32)mpgfile_decoder_get_play_time()) &&
                                       (wait_time < MP_TRICK_TIMEOUT))
                {
                    osal_task_sleep(2);
                    wait_time++;
                }
            }

            play_proc();
            osd_obj_close(p_obj,C_CLOSE_CLRBACK_FLG);
            return PROC_LEAVE;
        }
        break;

    case EVN_PRE_CLOSE:
        ret = PROC_PASS;
        break;

    case EVN_POST_CLOSE:
        api_set_deo_layer(1);
        api_dec_wnd_count();
        break;
    default:
        break;
    }

    return ret;
}



/*****************************************************
Interface : static VACTION chapter_list_keymap(POBJECT_HEAD obj, UINT32 key);
Function  : keymap of chapter list
Author    : Wu Jianwen
Date      : 2010.8.25
Input     : obj:container; key:v_Key of press button
Output    : NONE
Return    : NONE
******************************************************/
static VACTION chapter_list_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION    act = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
              act = VACT_CURSOR_LEFT;
              break;
          case V_KEY_RIGHT:
              act = VACT_CURSOR_RIGHT;
              break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = osd_list_key_map(obj, key);
            break;
    }

    return act;
}


/*****************************************************
Interface : static PRESULT chapter_list_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
Function  : callback of chapter list
Author    : Wu Jianwen
Date      : 2010.8.25
Input     :
Output    : NONE
Return    :
******************************************************/
static PRESULT chapter_list_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 top = 0;
    PRESULT ret = PROC_PASS;

    switch(event)
    {
        case EVN_PRE_DRAW:
            top= osd_get_list_top_point(&g_ls_chapter);
            get_list_chapter_info(top);
            break;

        case EVN_REQUEST_DATA:
            top = (UINT16)param2;
            get_list_chapter_info(top);
            break;
        default:
            PRINTF("do nothing \n");
            break;
    }
    return ret;
}


static void win_com_set_idx_str2(UINT8 *idx, UINT16 *str )
{
    char tmp[20] = {0};

    if((!idx) || (!str))
    {
        libc_printf("%s: idx or str is null \n", __FUNCTION__);
        return;
    }
    snprintf(tmp, 20, "%d:%d:%d ~ %d:%d:%d", *idx,*(idx+1),*(idx+2),*(idx+3),*(idx+4),*(idx+5));
    com_asc_str2uni((UINT8 *)tmp, str);
}

