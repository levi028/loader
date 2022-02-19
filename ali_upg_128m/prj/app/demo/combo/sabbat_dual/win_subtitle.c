/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_subtitle.c
*
*    Description: The menu to set subtitle
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

#include "copper_common/system_data.h"
#include "copper_common/com_api.h"

#include "osdobjs_def.h"
#include "win_com_menu_define.h"
#include "win_com_popup.h"
#include "menus_root.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"
#include <api/libmp/mp_subtitle_api.h>

#include "win_subtitle.h"
#include "win_com.h"
#include "control.h"

#define SUBTITLE_ITEM_NUMBER 5
static UINT16  subtitle_track_idx[SUBTITLE_ITEM_NUMBER][5]={{0,},};
static UINT16  subtitle_track_str[SUBTITLE_ITEM_NUMBER][24]={{0,},};


static UINT16 *p_subtitle_track_str[SUBTITLE_ITEM_NUMBER]={NULL};
static UINT16 *p_subtitle_track_idx[SUBTITLE_ITEM_NUMBER]={NULL};
static UINT8 g_win_subtitle_count=0;
static UINT8 g_win_ttx_subtitle_count=0;
void *subtitle_handle = NULL;
EXT_SUB_PLUGIN *g_ext_subtitle_plugin = NULL;
//extern UINT8 cur_tuner_idx;
//extern struct nim_device  *g_nim_dev;
//extern struct nim_device  *g_nim_dev2;

static LISTFIELD list_field_subtitle[] =
{
    {80,4,4,0,0,C_ALIGN_LEFT|C_ALIGN_VCENTER,\
     FIELD_IS_TEXT,p_subtitle_track_idx},
    {SUBTITLE_LS_W-80,0,4,0,0,\
     C_ALIGN_LEFT|C_ALIGN_VCENTER,FIELD_IS_TEXT,p_subtitle_track_str},
};
/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER    g_cnt_subtitle;



static VACTION subtile_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT subtitle_con_callback\
      (POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT subtitle_list_callback\
       (POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION win_sub_list_keymap(POBJECT_HEAD obj, UINT32 key);

struct t_ttx_lang *ttx_sub_lang_list=NULL;
struct t_subt_lang *sub_lang_list=NULL;

//object define
CONTAINER    g_cnt_subtitle=
{
    {
        OT_CONTAINER, //object type
        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
        0,      // 0: for component without focus

        0,//UINT8 bLeftID;
        0,//UINT8 bRightID;
        0,//UINT8 bUpID;
        0,//UINT8 bDownID;

        {SUBTITLE_L, SUBTITLE_T, SUBTITLE_W, SUBTITLE_H},//OSD_RECT frame;
        {WSTL_WIN_SLIST_02_8BIT, WSTL_WIN_SLIST_02_8BIT, \
     WSTL_WIN_SLIST_02_8BIT, WSTL_WIN_SLIST_02_8BIT},//COLOR_STYLE style;

        subtile_con_keymap,//PFN_KEY_MAP pfnKeyMap;
        subtitle_con_callback,//PFN_CALLBACK pfnCallback;

        NULL,//POBJECT_HEAD pNext;
        NULL//POBJECT_HEAD pRoot;
    },
    (POBJECT_HEAD)&g_txt_subtitle,
    1,//focus
    0
};

TEXT_FIELD  g_txt_subtitle =
{
    {
        OT_TEXTFIELD, //object type
        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
        0,      // 0: for component without focus

        0,//UINT8 bLeftID;
        0,//UINT8 bRightID;
        0,//UINT8 bUpID;
        0,//UINT8 bDownID;

        {SUBTITLE_TXT_L, SUBTITLE_TXT_T, \
     SUBTITLE_TXT_W, SUBTITLE_TXT_H},//OSD_RECT frame;
        {WSTL_MIXBACK_IDX_04_8BIT, WSTL_MIXBACK_IDX_04_8BIT, \
     WSTL_MIXBACK_IDX_04_8BIT, 0},//COLOR_STYLE style;

        NULL,//PFN_KEY_MAP pfnKeyMap;
        NULL,//PFN_CALLBACK pfnCallback;

        (POBJECT_HEAD)&g_ls_subtitle,//POBJECT_HEAD pNext;
        (POBJECT_HEAD)&g_cnt_subtitle,//POBJECT_HEAD pRoot;
    },
    C_ALIGN_CENTER | C_ALIGN_VCENTER,//    UINT8 bAlign;
    0,//UINT8 bX;
    0,//UINT8 bY;
    RS_SYSTEM_LANGUAGE_SUBTITLE_LANGUAGE,//UINT16 wStringID;  // string id
    NULL,//UINT16* pString;   // unicode string pointer
};

#define LDEF_SET_LIST(root, var_con,nxt_obj,l,t,w,h,field,\
        numfield,page,bar,intervaly,keymap,callback)        \
    DEF_LIST(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1,l,t,w,h, LIST_SH_IDX,LIST_HL_IDX,LIST_SL_IDX,LIST_GRY_IDX,\
    keymap,callback,LIST_FULL_PAGE| \
    LIST_PAGE_KEEP_CURITEM|LIST_GRID|LIST_SCROLL,\
    field,numfield,page,(SCROLL_BAR* )bar,\
    intervaly,NULL,0/*cnt*/,0/*ntop*/,0/*nsel*/,0/*npos*/)
#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb,root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, \
    LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE,\
    page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

LDEF_SET_LIST(&g_cnt_subtitle,g_ls_subtitle,&g_sb_subtitle, \
                SUBTITLE_LS_L, SUBTITLE_LS_T, SUBTITLE_LS_W, SUBTITLE_LS_H,\
                list_field_subtitle,2,SUBTITLE_LS_PAGE,&g_sb_subtitle,0,\
                win_sub_list_keymap,subtitle_list_callback);
LDEF_LISTBAR(&g_cnt_subtitle,g_sb_subtitle,SUBTITLE_LS_PAGE,\
             SUBTITLE_SB_L, SUBTITLE_SB_T, SUBTITLE_SB_W, SUBTITLE_SB_H);

static void win_subtitle_read_page(UINT8 top)
{
    UINT32 min_subt_type = 0x20;
    UINT8 i=0;
    UINT8 ttx_idx=0;
    UINT8 subt_idx=0;
    struct t_subt_lang *p_lang=NULL;
    struct t_ttx_lang *p_ttx_lang=NULL;
    UINT32      string_tans_ret=0;

	if(0 == string_tans_ret)
	{
		;
	}
	p_lang = sub_lang_list;
    p_ttx_lang = ttx_sub_lang_list;

    if(top > (g_win_ttx_subtitle_count + g_win_subtitle_count + NUM_ONE))
    {
        top = 0;/*if top value overflow,set it to zero*/
        osd_set_list_cur_pos(&g_ls_subtitle,0);
        osd_set_list_top(&g_ls_subtitle,0);
    }

    for(i = 0;i < SUBTITLE_ITEM_NUMBER;i++)
    {
        if((top+i) < g_win_ttx_subtitle_count)
        {
            ttx_idx = (top + i);
            p_ttx_lang = (ttx_sub_lang_list + ttx_idx);

            subtitle_track_str[i][0]=p_ttx_lang->lang[0];
            subtitle_track_str[i][1]=p_ttx_lang->lang[1];
            subtitle_track_str[i][2]=p_ttx_lang->lang[2];
            subtitle_track_str[i][3]='(';
            subtitle_track_str[i][4]='T';
            subtitle_track_str[i][5]=')';

            if(p_lang->subt_type < min_subt_type)
        {
            subtitle_track_str[i][6]=0;
        }
            else
            {
                subtitle_track_str[i][6]=' ';
                subtitle_track_str[i][7]='h';
                subtitle_track_str[i][8]='e';
                subtitle_track_str[i][9]='a';
                subtitle_track_str[i][10]='r';
                subtitle_track_str[i][11]='i';
                subtitle_track_str[i][12]='n';
                subtitle_track_str[i][13]='g';
                subtitle_track_str[i][14]=' ';
                subtitle_track_str[i][15]='i';
                subtitle_track_str[i][16]='m';
                subtitle_track_str[i][17]='p';
                subtitle_track_str[i][18]='a';
                subtitle_track_str[i][19]='i';
                subtitle_track_str[i][20]='r';
                subtitle_track_str[i][21]='e';
                subtitle_track_str[i][22]='d';
                subtitle_track_str[i][23]=0;
            }
            string_tans_ret=com_uni_str_to_mb(subtitle_track_str[i]);

            win_com_set_idx_str(top+i, subtitle_track_idx[i],2);
        }
        else if((top+i) < (g_win_ttx_subtitle_count + g_win_subtitle_count))
        {
            subt_idx = (top + i - g_win_ttx_subtitle_count);
            p_lang = (sub_lang_list + subt_idx);

            subtitle_track_str[i][0]=p_lang->lang[0];
            subtitle_track_str[i][1]=p_lang->lang[1];
            subtitle_track_str[i][2]=p_lang->lang[2];
            subtitle_track_str[i][3]='(';
            subtitle_track_str[i][4]='D';
            subtitle_track_str[i][5]=')';
            if(p_lang->subt_type < min_subt_type)
        {
            subtitle_track_str[i][6]=0;
        }
            else
            {
                subtitle_track_str[i][6]=' ';
                subtitle_track_str[i][7]='h';
                subtitle_track_str[i][8]='e';
                subtitle_track_str[i][9]='a';
                subtitle_track_str[i][10]='r';
                subtitle_track_str[i][11]='i';
                subtitle_track_str[i][12]='n';
                subtitle_track_str[i][13]='g';
                subtitle_track_str[i][14]=' ';
                subtitle_track_str[i][15]='i';
                subtitle_track_str[i][16]='m';
                subtitle_track_str[i][17]='p';
                subtitle_track_str[i][18]='a';
                subtitle_track_str[i][19]='i';
                subtitle_track_str[i][20]='r';
                subtitle_track_str[i][21]='e';
                subtitle_track_str[i][22]='d';
                subtitle_track_str[i][23]=0;
            }
            string_tans_ret=com_uni_str_to_mb(subtitle_track_str[i]);

            win_com_set_idx_str(top+i, subtitle_track_idx[i],2);
        }
        else if((top+i) == (g_win_ttx_subtitle_count + g_win_subtitle_count))
        {
            subtitle_track_str[i][0]='O';
            subtitle_track_str[i][1]='F';
            subtitle_track_str[i][2]='F';
            subtitle_track_str[i][3]=0;
            string_tans_ret=com_uni_str_to_mb(subtitle_track_str[i]);
            subtitle_track_idx[i][0]=0;
        }
        else
        {
            subtitle_track_idx[i][0]=0;
            subtitle_track_str[i][0]=0;
        }
    }
}

static void win_subtitle_init_data(void)
{

    UINT16 lst_top=0;
    UINT16 list_cnt=0;
    UINT8 sel_lang = 0xFF;
    UINT32 ttx_lang=0;
    UINT32 lang=0;

	if(0 == lang)
	{
		;
	}
	if(0 == ttx_lang)
	{
		;
	}
	SYSTEM_DATA *sys_data = sys_data_get();

    lang=ttxeng_get_subt_lang(&ttx_sub_lang_list, &g_win_ttx_subtitle_count);
    ttx_lang=subt_get_language(&sub_lang_list, &g_win_subtitle_count);

    list_cnt =  (g_win_subtitle_count + g_win_ttx_subtitle_count + 1);
    osd_set_list_count(&g_ls_subtitle, list_cnt);

    if(NUM_ZERO == sys_data->osd_set.subtitle_display)
    /*subtitle off,select the last item*/
    {
        osd_set_list_cur_pos(&g_ls_subtitle, (list_cnt - 1));
        if(list_cnt > SUBTITLE_ITEM_NUMBER)
            osd_set_list_top(&g_ls_subtitle, (list_cnt - SUBTITLE_ITEM_NUMBER));
        else
            osd_set_list_top(&g_ls_subtitle, 0);
    }
    else if(0xFF==sys_data->osd_set.subtitle_lang)
    /*set_sub_lang_idx can only be set in subtitle pop-up*/
    {/*if first init,or reset*/
        sel_lang = 0xFF;
#if (SUBTITLE_ON == 1)
        sel_lang = api_ttxsub_getlang(NULL,sub_lang_list,\
               sys_data->lang.sub_lang,g_win_subtitle_count);
        if(sel_lang != 0xFF)
        {
            sel_lang = sel_lang + g_win_ttx_subtitle_count;
        }/*subtitle language select first*/
#endif

#if (TTX_ON ==1)
        if( 0xFF==sel_lang )
        {
            sel_lang = api_ttxsub_getlang(ttx_sub_lang_list,NULL,\
                   sys_data->lang.sub_lang,g_win_ttx_subtitle_count);
        }/*if can not get subt lang_idx,to get ttx_subt lang_idx*/
#endif
        if(sel_lang != 0xFF)
    /*If sel_lang valid,set to corresponding item highlight*/
        {
            osd_set_list_cur_point(&g_ls_subtitle, sel_lang);
            //OSD_SetListTopPoint(&g_ls_subtitle, sel_lang);
            if(sel_lang > (SUBTITLE_ITEM_NUMBER - NUM_ONE))
                osd_set_list_top(&g_ls_subtitle, \
        (sel_lang - SUBTITLE_ITEM_NUMBER + 1));
            else
                osd_set_list_top(&g_ls_subtitle, 0);
        }
        else/*if sel_lang invalid,set to 0 position*/
        {
            osd_set_list_cur_point(&g_ls_subtitle, 0);
            osd_set_list_top_point(&g_ls_subtitle, 0);
        }
    }
    else
    {
        osd_set_list_cur_point(&g_ls_subtitle, sys_data->osd_set.subtitle_lang);
            if(sys_data->osd_set.subtitle_lang > \
          (SUBTITLE_ITEM_NUMBER - NUM_ONE))
            osd_set_list_top(&g_ls_subtitle, \
        (sys_data->osd_set.subtitle_lang - SUBTITLE_ITEM_NUMBER + 1));
        else
            osd_set_list_top(&g_ls_subtitle, 0);
    }

    osd_set_list_ver_scroll_bar(&g_ls_subtitle, &g_sb_subtitle);

    lst_top = osd_get_list_top_point(&g_ls_subtitle);
    win_subtitle_read_page(lst_top);
}

#ifdef MP_SUBTITLE_SUPPORT    //vic100507#1


static long win_mp_subtitle_read_page(UINT8 top)
{
    long            i=0;
    long            ret = -1;
    long            number=0;
    long            current_language_index=0;
    char            description[201] = {0};
    long            description_len=0;
    char            *temp=NULL;
    UINT32      string_tans_ret=0;

	if(0 == string_tans_ret)
	{
		;
	}
	if (!subtitle_handle)
    {
        return -1;
    }

    number = 0;
    description_len = 200;

    if(g_ext_subtitle_plugin)
    {
        ret = g_ext_subtitle_plugin->mps_query_language(subtitle_handle, &number,
                                &current_language_index,
                                description, &description_len);
    }

    if (ret < NUM_ZERO)
    {
        return -1;
    }


    if(top > number)
    {
        top = 0;
        osd_set_list_cur_pos(&g_ls_subtitle,0);
        osd_set_list_top(&g_ls_subtitle,0);
    }

    for(i = 0;i < SUBTITLE_ITEM_NUMBER;i++)
    {
        if((top+i) < number)
        {
            temp = description + (top+i)*4;

            subtitle_track_str[i][0]=temp[0];
            subtitle_track_str[i][1]=temp[1];
            subtitle_track_str[i][2]=temp[2];
            subtitle_track_str[i][3]=0;
            subtitle_track_str[i][4]=0;
            subtitle_track_str[i][5]=0;
            subtitle_track_str[i][6]=0;
            string_tans_ret=com_uni_str_to_mb(subtitle_track_str[i]);

            win_com_set_idx_str(top+i, subtitle_track_idx[i], 2);
        }
        else
        {
            subtitle_track_idx[i][0]=0;
            subtitle_track_str[i][0]=0;
        }
    }

    return 1;
}


static long win_mp_subtitle_init_data(void)
{
    long            ret = -1;
    long            number=0;
    long            current_language_index=0;
    UINT16 lst_top=0;
    UINT16 list_cnt=0;

    //UINT8 sel_lang = 0xFF;

    if (!subtitle_handle)
    {
        return -1;
    }

    if(g_ext_subtitle_plugin)
    {
        ret = g_ext_subtitle_plugin->mps_query_language        \
            (subtitle_handle, &number, &current_language_index, NULL, NULL);
    }

    if (ret < NUM_ZERO)
    {
        return -1;
    }

    list_cnt =  number;
    osd_set_list_count(&g_ls_subtitle, list_cnt);


    osd_set_list_cur_pos(&g_ls_subtitle, current_language_index);
    osd_set_list_top(&g_ls_subtitle, current_language_index);

    osd_set_list_ver_scroll_bar(&g_ls_subtitle, &g_sb_subtitle);

    lst_top = osd_get_list_top_point(&g_ls_subtitle);
    ret = win_mp_subtitle_read_page(lst_top);
    return ret;
}
#endif


VACTION subtile_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act=0;

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

static PRESULT subtitle_con_callback(POBJECT_HEAD pobj, VEVENT event,
                     UINT32 param1, UINT32 param2)
{
    UINT16    cur_sel = 0;
    long resut = 0;
    PRESULT ret = PROC_PASS;
    PRESULT obj_ret= PROC_PASS;
    SYSTEM_DATA *sys=NULL;
    struct nim_device *nim_dev=NULL;
    UINT8 i=0;
    UINT8 lock=0;
    UINT8 unact=0;
    UINT32 func_ret=0;
    //BOOL msg_ret=FALSE;
    enum OSD_MODE  osd_mode=0;
    win_popup_choice_t choice = WIN_POP_CHOICE_YES;
    INT32 nim_lock_status=0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

	if(0 == nim_lock_status)
	{
		;
	}
	if(WIN_POP_CHOICE_YES == choice)
	{
		;
	}
	if(0 == osd_mode)
	{
		;
	}
	if(0 == func_ret)
	{
		;
	}
	if(PROC_PASS == obj_ret)
	{
		;
	}
	if(NUM_ZERO == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
#if defined SAT2IP_SERVER_SUPPORT || defined SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())
        nim_dev = NULL;
#endif
    nim_lock_status=nim_get_lock(nim_dev,&lock);

    system_state = api_get_system_state();

    switch(event)
    {
    case EVN_PRE_OPEN:
        #ifdef SUPPORT_CAS9
            if(CA_MMI_PRI_00_NO==get_mmi_showed())
        {
            show_signal_status_osdon_off(0);
        }
        #endif
        func_ret=api_set_deo_layer(0);
        func_ret=api_inc_wnd_count();

        for(i=0;i<SUBTITLE_ITEM_NUMBER;i++)
        {
            p_subtitle_track_str[i]=subtitle_track_str[i];
            p_subtitle_track_idx[i]=subtitle_track_idx[i];
        }

        if(system_state != SYS_STATE_USB_MP)//vic100507#1
        {
            win_subtitle_init_data();
            if(((NUM_ZERO == g_win_subtitle_count )&& (NUM_ZERO == g_win_ttx_subtitle_count)) || (!lock))
            {
                #if (DVR_PVR_SUPPORT_SUBTITLE)//temp disable
                if(((NUM_ZERO == g_win_subtitle_count )&& (NUM_ZERO == g_win_ttx_subtitle_count))
                #ifdef DVR_PVR_SUPPORT
                    || ((!lock) && (!api_pvr_is_playing()))
                #else
                    || (!lock)
                #endif
                    )
                #endif
                {
                    #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
                        UINT8 back_saved;
                        win_compopup_init(WIN_POPUP_TYPE_SMSG);
                        win_compopup_set_msg_ext(NULL, NULL, RS_DISPLAY_NO_DATA);
                        choice=win_compopup_open_ext(&back_saved);
                        osal_task_sleep(1000);
                        win_compopup_smsg_restoreback();
                    #else
                    choice=win_com_popup_open\
                   (WIN_POPUP_TYPE_SMSG,NULL, RS_DISPLAY_NO_DATA);
                    osal_task_sleep(1000);
                    win_compopup_close();
                    #endif
                            func_ret=api_dec_wnd_count();
                    return PROC_LEAVE;
                }
            }
        }
        else
        {
        #ifdef MP_SUBTITLE_SUPPORT  //vic100507#1
            //TODO:
            //such like win_subtitle_init_data
        resut = win_mp_subtitle_init_data();
            if (resut < NUM_ZERO)
            {
                return PROC_LEAVE;
            }

        #endif
        }
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);

        if(system_state != SYS_STATE_USB_MP)//vic100507#1
        {

            if(VACT_ENTER == unact)
            {
                cur_sel = osd_get_list_cur_point(&g_ls_subtitle);

                sys = sys_data_get();
                if(g_win_subtitle_count + g_win_ttx_subtitle_count==cur_sel)
                {
                    sys->osd_set.subtitle_display = 0;/*select off option*/
                    sys->osd_set.subtitle_lang = 0xFF;
                }
                else if(cur_sel < g_win_ttx_subtitle_count)
                {
                    sys->osd_set.subtitle_display = 1;/*subtitle on*/
                    sys->osd_set.subtitle_lang = cur_sel;
            /*set_sub_lang_idx is used in api_osd_mode_change function*/
                }
                else if(cur_sel < \
                          g_win_subtitle_count + g_win_ttx_subtitle_count)
                {/*ttx_subtitle on the top then normal subtitle*/
                    sys->osd_set.subtitle_display = 1;
                    sys->osd_set.subtitle_lang = cur_sel;
            /*set_sub_lang_idx is used in api_osd_mode_change function*/
                }
                ret = PROC_LEAVE;
                osd_mode=api_osd_mode_change(OSD_NO_SHOW);
                sys_data_save(0);
                //api_osd_mode_change(OSD_SUBTITLE);
            }
        }
        else if(VACT_ENTER == unact)
        {
        //vic100507#1 begin
        #ifdef MP_SUBTITLE_SUPPORT
            //TODO:
            //select language index
            if (!subtitle_handle)
            {
                return PROC_LEAVE;
            }

            cur_sel = osd_get_list_cur_point(&g_ls_subtitle);

            if(g_ext_subtitle_plugin)
        {
            g_ext_subtitle_plugin->mps_change_language        \
                (subtitle_handle,cur_sel);
        }

            obj_ret=osd_obj_close(pobj,C_CLOSE_CLRBACK_FLG);
            //if (resut < 0)
            {
                return PROC_LEAVE;
            }
        #endif
        //vic100507#1 end
        }
        break;

    case EVN_MSG_GOT:
#ifdef USB_MP_SUPPORT
        if(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER == param1)
        {
            ret = PROC_LEAVE;
            //msg_ret=
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,0,TRUE);
        }
#endif
        break;

    case EVN_PRE_CLOSE:
        unact = (VACTION)(param1>>16);
        ret = PROC_PASS;
        break;

    case EVN_POST_CLOSE:
        #ifdef SUPPORT_CAS9
            restore_ca_msg_when_exit_win();
        #endif
        func_ret=api_set_deo_layer(1);
        func_ret=api_dec_wnd_count();
        break;
    default:
        break;
    }

    return ret;
}

 static VACTION win_sub_list_keymap(POBJECT_HEAD obj, UINT32 key)
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

static PRESULT subtitle_list_callback\
               (POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 top=0;
    PRESULT ret = PROC_PASS;
    long result=0;
    sys_state_t system_state = SYS_STATE_INITIALIZING;

	if(0 == result)
	{
		;
	}
	system_state = api_get_system_state();
    switch(event)
    {
        case EVN_PRE_DRAW:
//vic100507#1
            top = osd_get_list_top_point(&g_ls_subtitle);
            osd_set_list_top_point(&g_ls_subtitle, top);
            top= osd_get_list_top_point(&g_ls_subtitle);
            if(system_state != SYS_STATE_USB_MP)
        {
            win_subtitle_read_page(top);
        }
#ifdef MP_SUBTITLE_SUPPORT
            else
                //TODO: set display items string
        {
            result=win_mp_subtitle_read_page(top);
        }
#endif
//vic100507#1
            break;
        case EVN_REQUEST_DATA:
            top = (UINT16)param2;
            if(system_state != SYS_STATE_USB_MP)//vic100507#1
        {
            win_subtitle_read_page(top);
        }
#ifdef MP_SUBTITLE_SUPPORT  //vic100507#1       
            else
                //TODO: set display items string
        {
            result=win_mp_subtitle_read_page(top);
        }
#endif

            break;
        default:
            PRINTF("do nothing \n");
            break;
    }

    return ret;
}

void win_com_set_idx_str(UINT16 idx, UINT16  *str, UINT16 len)
{
    UINT32 tansform_ret=0;
    int sprintf_ret=0;
    char tmp[11]={0};

	if(0 == tansform_ret)
	{
		;
	}
    switch(len)
    {
        case 1:
           sprintf_ret=snprintf(tmp, 11,"[%.d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
        case 2:
           sprintf_ret=snprintf(tmp, 11,"[%.2d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
        case 3:
           sprintf_ret=snprintf(tmp, 11,"[%.3d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
        case 4:
           sprintf_ret=snprintf(tmp, 11,"[%.4d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
        case 5:
           sprintf_ret=snprintf(tmp, 11,"[%.5d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
        default:
           sprintf_ret=snprintf(tmp, 11,"[%.d]", idx+1/*base on 1*/);
           if(0 == sprintf_ret)
           {
               ali_trace(&sprintf_ret);
           }
           break;
    }
    tansform_ret=com_asc_str2uni((UINT8 *)tmp, str);
}

