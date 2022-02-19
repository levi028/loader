/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_factory_test.c
*
*    Description:   The menu for factory test
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <api/libchunk/chunk.h>
#include <api/libdiseqc/lib_diseqc.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <bus/sci/sci.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "control.h"
#include "key.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_factory_test.h"

#define MAX_CHAR_CNT    30
#define TIME_OUT        200

#define DEB_PRINTF soc_printf
/*******************************************************************************
 *    Objects definition
 *******************************************************************************/
#define SCI_READ_TIMEOUT 500
#define SCT_READ_TIME       1000
CONTAINER g_win_factory_test;

static VACTION  factest_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  factest_item_callback(POBJECT_HEAD pobj, \
                      VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  factest_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT  factest_callback(POBJECT_HEAD pobj, \
                 VEVENT event, UINT32 param1, UINT32 param2);


#ifdef DVR_PVR_SUPPORT
#define WIN_SH_IDX      WSTL_POP_WIN_01_HD//WSTL_WIN_PVR_03
#else
#define WIN_SH_IDX      WSTL_TEXT_13
#endif

#define TITLE_SH_IDX    WSTL_POP_TXT_SH_HD //WSTL_TEXT_13


#define CON_SH_IDX    WSTL_POP_TXT_SH_01_HD //WSTL_POP_LIN//WSTL_BUTTON_01
#define CON_HL_IDX    WSTL_BUTTON_SELECT_HD //WSTL_BUTTON_05
#define CON_SL_IDX    WSTL_BUTTON_01_HD
#define CON_GRY_IDX    WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define BMP_SH_IDX         WSTL_MIX_BMP_HD //WSTL_BUTTON_01_FG
#define BMP_HL_IDX       WSTL_MIX_BMP_HD //WSTL_BUTTON_05
#define BMP_SL_IDX       WSTL_BUTTON_01_FG_HD
#define BMP_GRY_IDX      WSTL_BUTTON_01_FG_HD

#define    W_W     700//500
#define    W_H     480//300
#define    W_L     GET_MID_L(W_W)
#define    W_T     GET_MID_T(W_H)

#define TITLE_L        (W_L + 20)
#define TITLE_T        (W_T + 30)
#define TITLE_W        (W_W - 40)
#define TITLE_H        40//30

#define ITEM_L    (W_L + 20)
#define ITEM_T    (TITLE_T + TITLE_H)
#define ITEM_W    (W_W - 40)
#define ITEM_H    40//26
#define ITEM_GAP    4

#define ITEM_TXT_L      10
#define ITEM_TXT_W      350
#define ITEM_BMP_L      (ITEM_TXT_L + ITEM_TXT_W)
#define ITEM_BMP_W       90 //60 //400  //300

#define LINE_L_OF    0
#define LINE_T_OF      (ITEM_H+4)
#define LINE_W        ITEM_W
#define LINE_H         4

#define LDEF_TITLE(root,var_txt,nxtobj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxtobj,C_ATTR_ACTIVE,0,        \
              0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX, \
              NULL,NULL,                    \
              C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_CON(root, var_con,nxtobj,ID,idu,idd,l,t,w,h,conobj,focus_id)    \
    DEF_CONTAINER(var_con,root,nxtobj,C_ATTR_ACTIVE,0,        \
              ID,ID,ID,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX, \
              factest_item_keymap,factest_item_callback,    \
              conobj, 1,1)

#define LDEF_TXT(root,var_txt,nxtobj,ID,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxtobj,C_ATTR_ACTIVE,0,        \
              ID,ID,ID,ID,ID, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX, \
              NULL,NULL,                    \
              C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_BMP(root,var_bmp,nxtobj,ID,l,t,w,h,icon)            \
    DEF_BITMAP(var_bmp,root,nxtobj,C_ATTR_ACTIVE,0,            \
           ID,ID,ID,ID,ID, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX, \
           NULL,NULL,                        \
           C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,icon)

#undef LDEF_LINE
#define LDEF_LINE(root,var_txt,nxtobj,ID,                \
          idl,idr,idu,idd,l,t,w,h,res_id,str)            \
    DEF_TEXTFIELD(var_txt,root,nxtobj,C_ATTR_ACTIVE,0,        \
              ID,idl,idr,idu,idd, l,t,w,h, WSTL_POP_LINE_HD,    \
              WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_LINE_HD,   \
              NULL,NULL,                    \
              C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_ITEM(root,var_con,nxtobj,var_txt,                \
          var_bmp,var_line,ID,idu,idd,l,t,w,h,str)        \
    LDEF_CON(&root,var_con,nxtobj,ID,idu,idd,l,t,w,h,&var_txt,1)    \
    LDEF_TXT(&var_con,var_txt,&var_bmp,1,                \
         l + ITEM_TXT_L,   t,ITEM_TXT_W,h,0,str)        \
    LDEF_BMP(&var_con,var_bmp,NULL/*&varLine*/,0,            \
         l + ITEM_BMP_L,  t,ITEM_BMP_W, h,0)            \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,            \
          l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_wnd,nxtobj,l,t,w,h,focus_id)                \
    DEF_CONTAINER(var_wnd,NULL,nxtobj,C_ATTR_ACTIVE,0,        \
              0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX, \
              factest_keymap,factest_callback,            \
              nxtobj, focus_id,0)


LDEF_TITLE(g_win_factory_test, facttest_title,&facttest_item0, \
       TITLE_L, TITLE_T, TITLE_W, TITLE_H,0,len_display_str)

LDEF_ITEM(g_win_factory_test,facttest_item0,&facttest_item1,\
      facttest_txt0,facttest_bmp0,facttest_line0,1,7,2, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*0,\
      ITEM_W, ITEM_H,display_strs[0])

LDEF_ITEM(g_win_factory_test,facttest_item1,&facttest_item2,\
      facttest_txt1,facttest_bmp1,facttest_line1,2,1,3, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*1, \
      ITEM_W, ITEM_H,display_strs[1])

LDEF_ITEM(g_win_factory_test,facttest_item2,&facttest_item3,\
      facttest_txt2,facttest_bmp2,facttest_line2,3,2,4, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*2, \
      ITEM_W, ITEM_H,display_strs[2])

LDEF_ITEM(g_win_factory_test,facttest_item3,&facttest_item4,\
      facttest_txt3,facttest_bmp3,facttest_line3,4,3,5, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*3, \
      ITEM_W, ITEM_H,display_strs[3])

LDEF_ITEM(g_win_factory_test,facttest_item4,&facttest_item5,\
      facttest_txt4,facttest_bmp4,facttest_line4,5,4,6, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*4,\
      ITEM_W, ITEM_H,display_strs[4])

LDEF_ITEM(g_win_factory_test,facttest_item5,&facttest_item6,\
      facttest_txt5,facttest_bmp5,facttest_line5,6,5,7, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*5, \
      ITEM_W, ITEM_H,display_strs[5])

LDEF_ITEM(g_win_factory_test,facttest_item6,NULL,\
      facttest_txt6,facttest_bmp6,facttest_line6,7,6,1, \
      ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP + LINE_H)*6, \
      ITEM_W, ITEM_H,display_strs[6])

LDEF_WIN(g_win_factory_test, &facttest_title,W_L, W_T, W_W, W_H,1)

#define FACTEST_LNB_ID         1
#define FACTEST_PATTEN_ID     2
#define FACTEST_FRONTKEY_ID     3
#define FACTEST_12V_ID         4
#define FACTEST_SLOWBLANK_ID     5
#define FACTEST_RS232_ID        6
#define FACTEST_TESTALL_ID    7

/*******************************************************************************
 *    Local vriable & function declare
 *******************************************************************************/
#ifdef _CAS9_CA_ENABLE_
static char *facttest_item_strs[] =
{
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " "
};
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
char *facttest_item_1_strs[] =
{
    " ",
    " "
};
#endif

static char *facttest_item_3_strs[] =
{
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
};

#else
static char* facttest_item_strs[] =
{
    "1. LNB conrol signal test",
    "2. Pattern display test",
    "3. Front key test",
    "4. 0/12V test",
    "5. Slow blanking test",
    "6. RS232 input test",
    "Test ALL"
};
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
char* facttest_item_1_strs[] =
{
    "POL - HOR,  22KHz-OFF",
    "POL - VER,  22KHz-ON"
};
#endif

char* facttest_item_3_strs[] =
{
    "POWER",
    "MENU",
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "OK",
    "EXIT",
};
#endif

static UINT8 front_panel_keys[] =
{
    V_KEY_POWER,
    V_KEY_MENU,
    V_KEY_LEFT,
    V_KEY_RIGHT,
    V_KEY_UP,
    V_KEY_DOWN,
    V_KEY_ENTER,
    V_KEY_EXIT
};
#define FRONT_PANEL_KEY_CNT (ARRAY_SIZE(front_panel_keys))

#ifdef _CAS9_CA_ENABLE_
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
char *facttest_item_4_strs[] =
{
    " ",
    " "
};
#endif
static char *facttest_item_5_strs[] =
{
    " ",
    " ",
    " "
};

static char *facttest_item_6_strs[] =
{
    " ",
    " "
};

#else
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
char *facttest_item_4_strs[] =
{
    "0V",
    "12V"
};
#endif
static char *facttest_item_5_strs[] =
{
    "4:3",
    "16:9",
    "TV"
};

static char *facttest_item_6_strs[] =
{
    "Press any key on the PC keyboard[<=30]",
    "RS232 Recevie data time out"
};
#endif

static TEXT_FIELD *factest_item_txts[] =
{
    &facttest_txt0,
    &facttest_txt1,
    &facttest_txt2,
    &facttest_txt3,
    &facttest_txt4,
    &facttest_txt5,
    &facttest_txt6,
};
#if 0
BITMAP* factest_item_bmps[] =
{
    &facttest_bmp0,
    &facttest_bmp1,
    &facttest_bmp2,
    &facttest_bmp3,
    &facttest_bmp4,
    &facttest_bmp5,
    &facttest_bmp6,
};
#endif
#define FACTROY_TEST_ITEM_CNT ARRAY_SIZE(factest_item_txts)

static UINT8 item_test_flag[FACTROY_TEST_ITEM_CNT]={0};


static void  factest_set_initdisplay(void);
static void  factest_set_bmpdisplay(void);


static INT32 factory_test_lnb(UINT32 idx, UINT32 bauto);
static INT32 factory_test_colorpattern(UINT32 idx, UINT32 bauto);
static INT32 factory_test_panelkey(UINT32 idx, UINT32 bauto);
static INT32 factory_test_12v(UINT32 idx, UINT32 bauto);
static INT32 factory_test_slowblanking(UINT32 idx, UINT32 bauto);
static INT32 factory_test_rs232test(UINT32 idx, UINT32 bauto);
static INT32 factory_test_all(UINT32 idx, UINT32 bauto);





typedef INT32 (*factory_test_func)(UINT32 idx, UINT32 bauto);


static factory_test_func factory_test_funcs[] =
{
    factory_test_lnb,
    factory_test_colorpattern,
    factory_test_panelkey,
    factory_test_12v,
    factory_test_slowblanking,
    factory_test_rs232test,
    factory_test_all,
};


/*******************************************************************************
 *    key mapping and event callback definition
 *******************************************************************************/

static VACTION  factest_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


static PRESULT  factest_item_callback(POBJECT_HEAD pobj,\
                      VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid=0;
    POBJECT_HEAD nxtobj = NULL;
    //PRESULT obj_ret = PROC_PASS;

    MEMSET(&nxtobj,0,sizeof(POBJECT_HEAD));
    bid = osd_get_obj_id(pobj);


    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        item_test_flag[bid- 1] = 0;
        if(NUM_ZERO == factory_test_funcs[bid- 1](bid - 1, 0) )
        {
            item_test_flag[bid - 1] = 1;
        }
        else
        {
            item_test_flag[bid - 1] = 0;
        }
        factest_set_bmpdisplay();
        nxtobj = (POBJECT_HEAD)&facttest_item0;
        while(nxtobj)
        {
            if(nxtobj != pobj)
            {
                osd_draw_object(nxtobj,C_UPDATE_ALL);
            }
            else
            {
                osd_track_object(nxtobj,C_UPDATE_ALL);
            }
            nxtobj = osd_get_objp_next(nxtobj);
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION  factest_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

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
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT  factest_callback(POBJECT_HEAD pobj, \
                 VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    POBJECT_HEAD topmenu = NULL;
    BOOL key_func=FALSE;
    //PRESULT obj_ret = PROC_PASS;

	if(FALSE == key_func)
	{
		;
	}
	MEMSET(&topmenu,0,sizeof(POBJECT_HEAD));
    switch(event)
    {
    case EVN_PRE_OPEN:
        key_func=ap_enable_key_task_get_key(TRUE);
        key_set_upgrade_check_flag(0);
        wincom_close_title();
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_clear_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
            topmenu = menu_stack_get_top();
            if(topmenu)
            {
                osd_clear_object( topmenu, C_UPDATE_ALL);
            }
        }

        factest_set_initdisplay();
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        //*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_track_object(            \
                (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
        }
        key_set_upgrade_check_flag(UPGRADE_CHECK_SLAVE);
        key_func=ap_enable_key_task_get_key(FALSE);
        break;
    default:
        break;
    }

    return ret;
}

static void  factest_set_initdisplay(void)
{
    TEXT_FIELD  *txt=NULL;

    char        str[50]={0};

    unsigned long chid=0;
    CHUNK_HEADER blk_header;
    UINT32 i=0;
    int ret=0;

    MEMSET(&blk_header,0,sizeof(CHUNK_HEADER));

    chid = MAINCODE_ID;
    ret=sto_get_chunk_header(chid, &blk_header);
    if ( 0 ==ret)
    {
        return ;
    }
#ifdef _CAS9_CA_ENABLE_
    snprintf(str,50,"%s[%s]",blk_header.version,blk_header.time);
#else
    snprintf(str,50,"Factory test (S/W - %s[%s])",\
         blk_header.version,blk_header.time);
#endif
    txt = &facttest_title;
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

    for(i=0;i<FACTROY_TEST_ITEM_CNT;i++)
    {
        txt = factest_item_txts[i];
        osd_set_text_field_content(txt, STRING_ANSI,\
                    (UINT32)facttest_item_strs[i]);
    }

    /* Test all has no flag*/
    osd_set_objp_next(txt, NULL);

    MEMSET(item_test_flag,0,sizeof(item_test_flag));
    factest_set_bmpdisplay();

}

static void  factest_set_bmpdisplay(void)
{
    TEXT_FIELD  *txt=NULL;
    BITMAP      *bmp=NULL;
    UINT32 i=0;

    for(i=0;i<FACTROY_TEST_ITEM_CNT;i++)
    {
        txt = factest_item_txts[i];
        bmp = (BITMAP*)osd_get_objp_next(txt);
        if(bmp != NULL)
        {
            osd_set_bitmap_content(bmp, item_test_flag[i]? IM_TV_FAVORITE : 0);
        }
    }
}


/*****/

static INT32 get_key(UINT32 *key_type, UINT32 *vkey)
{
    UINT32 hk=0;
    UINT8 type=0;
    UINT32 func_ret=0;

	if(0 == func_ret)
	{
		;
	}
	hk = ap_get_hk();
    if((INVALID_MSG ==hk)||( INVALID_HK == hk))
    {
        return -1;
    }

    msg_code_to_key_type(hk, &type);

    if(PAN_KEY_TYPE_REMOTE == type)
    {
        *key_type = PAN_KEY_TYPE_REMOTE;
    }
    else if(PAN_KEY_TYPE_PANEL == type)
    {
        *key_type = PAN_KEY_TYPE_PANEL;
    }
    else
    {
        return -2;
    }
    func_ret= ap_hk_to_vk(0, hk, vkey);
    if(V_KEY_NULL == *vkey)
    {
        return -3;
    }
    return 0;
}

static INT32 check_ok_exit_key(void)
{
    UINT32 keytype=0;
    UINT32 vkey=0;

    while (1)
    {
        //GET_OK_EXIT_KEY:
        while(get_key(&keytype,&vkey)<NUM_ZERO)
        {
            ;
        }

        if(vkey != V_KEY_ENTER
           && vkey != V_KEY_MENU
           && vkey != V_KEY_EXIT)
        {
            //goto GET_OK_EXIT_KEY;
            continue;
        }

        break;
    }

    if(V_KEY_ENTER == vkey)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static INT32 factory_test_lnb(UINT32 idx, UINT32 bauto)
{


#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    UINT8 back_saved=0;
    ap_clear_all_message();

    /* Step1:  H / 22K OFF*/
    //nim_set_polar(g_nim_dev, 0);
    nim_io_control(g_nim_dev, NIM_DRIVER_SET_POLAR, 0);
    api_diseqc_set_22k(g_nim_dev,0);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_title(facttest_item_strs[idx], NULL,0);
    win_compopup_set_msg(facttest_item_1_strs[0], NULL, 0);
    win_compopup_open_ext(&back_saved);

    if(NUM_ZERO==check_ok_exit_key())
    {
        win_compopup_smsg_restoreback();
        return -1;
    }


    /* Step2:  V / 22K ON*/
    //nim_set_polar(g_nim_dev, 1);
    nim_io_control(g_nim_dev, NIM_DRIVER_SET_POLAR, 1);
    api_diseqc_set_22k(g_nim_dev,1);
    win_compopup_set_msg_ext(facttest_item_1_strs[1], NULL, 0);
    win_compopup_open();

    check_ok_exit_key();
    win_compopup_smsg_restoreback();
#endif

    return 0;

}

extern UINT8 beep_bin_array[];
extern UINT32 beep_bin_array_size;

static INT32 factory_test_colorpattern(UINT32 idx, UINT32 bauto)
{

    UINT32 i=0;
    RET_CODE func_ret=RET_SUCCESS;

	if(RET_SUCCESS == func_ret)
	{
		;
	}
	ap_clear_all_message();

#ifdef VDEC27_SUPPORT_COLORBAR

    osd_show_on_off(OSDDRV_OFF);

    func_ret=vdec_io_control(g_decv_dev,VDEC_IO_COLORBAR,0);

#if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)			
        api_audio_gen_tonevoice(7, 0);
#elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
        api_audio_beep_start(beep_bin_array,beep_bin_array_size);
        api_audio_beep_set_interval(100);					
#endif

    for(i=0;i<8;i++)
    {
        osal_task_sleep(1000);

        if(NUM_ZERO==check_ok_exit_key())
        {
            break;
        }
	}
#if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
	api_audio_stop_tonevoice();
#elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
	api_audio_beep_stop();
#endif

    api_show_menu_logo();


    osd_show_on_off(OSDDRV_ON);

#endif


    return 0;
}

static INT32 factory_test_panelkey(UINT32 idx, UINT32 bauto)
{
    UINT8 mark_align=0;
    UINT8 offset=0;
    UINT16 style=0;
    OSD_RECT rect;
    UINT8 input_fp_keys[FRONT_PANEL_KEY_CNT]={0};
    UINT32 i=0;
    UINT32 keytype=0;
    UINT32 vkey=0;
    //PRESULT obj_ret=PROC_PASS;
    INT32 func_ret=0;

	if(0 == func_ret)
	{
		;
	}
	MEMSET(&rect,0,sizeof(OSD_RECT));
    ap_clear_all_message();

    /* Step1: Show  */
    osd_set_rect(&rect, ITEM_L + 40, ITEM_T - 20, 250, 400);//260);
    style =  LIST_VER | LIST_SCROLL | LIST_MULTI_SLECT\
        | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM \
        | LIST_FULL_PAGE | LIST_ITEMS_COMPLETE;
    mark_align = C_ALIGN_RIGHT;
    offset = 50;

    win_comlist_reset();
    win_comlist_set_sizestyle(FRONT_PANEL_KEY_CNT, FRONT_PANEL_KEY_CNT,style);
    win_comlist_set_align(30,0,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    win_comlist_ext_enable_scrollbar(FALSE);
    win_comlist_set_frame(rect.u_left,rect.u_top, rect.u_width, rect.u_height);
    win_comlist_set_mapcallback(NULL,NULL,NULL);
    win_comlist_ext_set_title_style(WSTL_POP_TXT_SH_HD);
    win_comlist_ext_set_title(facttest_item_strs[idx],NULL,0);

    win_comlist_ext_set_ol_frame(rect.u_left + 10,rect.u_top + 40/*30*/,\
                     rect.u_width - 20, rect.u_height - 50/*40*/);
    win_comlist_ext_set_selmark_xaligen(mark_align,offset);
    win_comlist_ext_set_selmark(IM_TV_FAVORITE);

    for(i=0;i<FRONT_PANEL_KEY_CNT;i++)
    {
        win_comlist_set_str(i,facttest_item_3_strs[i],NULL, 0);
    }

    win_comlist_open();


    MEMSET(input_fp_keys,0,FRONT_PANEL_KEY_CNT);
    while(1)
    {
        if(get_key(&keytype,&vkey) >= NUM_ZERO)
        {
            if(PAN_KEY_TYPE_REMOTE == keytype)
            {
                if((V_KEY_ENTER ==vkey)\
                   ||( V_KEY_MENU == vkey) \
                   ||( V_KEY_EXIT == vkey))
                {
                    break;
                }
            }
            else if(PAN_KEY_TYPE_PANEL == keytype)
            {
                /* Get the key's index in front_panel_keys[] */
                for(i=0;i<FRONT_PANEL_KEY_CNT;i++)
                {
                    if(front_panel_keys[i] == vkey)
                    {
                        break;
                    }
                }

                if( i>= FRONT_PANEL_KEY_CNT)
                    /* Not valid checking front panel key*/
                {
                    continue;
                }

                /* The key not inputed */
                if(vkey != input_fp_keys[i])
                {
                    input_fp_keys[i] = vkey;

                    win_comlist_clear_sel();

                    for(i=0;i<FRONT_PANEL_KEY_CNT;i++)
                    {
                        if(input_fp_keys[i] == front_panel_keys[i])
                        {
                            win_comlist_ext_set_item_sel(i);
                        }
                    }

                    win_comlist_open();
                }

                /* Check all front panel key pressed or not.*/
                for(i=0;i<FRONT_PANEL_KEY_CNT;i++)
                {
                    if(input_fp_keys[i] != front_panel_keys[i])
                    {
                        break;
                    }
                }

                if(i>=FRONT_PANEL_KEY_CNT)
                {
                    break;
                }
            }
        }
    }

    for(i=0;i<FRONT_PANEL_KEY_CNT;i++)
    {
        if(input_fp_keys[i] != front_panel_keys[i])
        {
            break;
        }
    }

    if(i>=FRONT_PANEL_KEY_CNT)
    {
        func_ret=check_ok_exit_key();
    }

    osd_track_object((POBJECT_HEAD)&g_win_factory_test,C_UPDATE_ALL);

    if(i>=FRONT_PANEL_KEY_CNT)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static INT32 factory_test_12v(UINT32 idx, UINT32 bauto)
{
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    UINT8 back_saved=0;

    ap_clear_all_message();

    /* Step1: 0V */
    api_diseqc_set_12v(g_nim_dev, 0);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_title(facttest_item_strs[idx], NULL,0);
    win_compopup_set_msg(facttest_item_4_strs[0], NULL, 0);
    win_compopup_open_ext(&back_saved);

    if(NUM_ZERO==check_ok_exit_key())
    {
        win_compopup_smsg_restoreback();
        return -1;
    }


    /* Step2:  12V */
    api_diseqc_set_12v(g_nim_dev, 1);
    win_compopup_set_msg_ext(facttest_item_4_strs[1], NULL, 0);
    win_compopup_open();

    check_ok_exit_key();
    win_compopup_smsg_restoreback();
#endif
    return 0;

}


static INT32 factory_test_slowblanking(UINT32 idx, UINT32 bauto)
{
    UINT8 back_saved=0;
    av_set av_set;
    SYSTEM_DATA *sys_data=NULL;
    win_popup_choice_t  popup_choice=WIN_POP_CHOICE_NULL;

	if(WIN_POP_CHOICE_NULL == popup_choice)
	{
		;
	}
	MEMSET(&av_set,0,sizeof(av_set));
    sys_data = sys_data_get();

    ap_clear_all_message();

    /* Step1: 4:3 */
    av_set.tv_ratio = TV_ASPECT_RATIO_43;
    av_set.display_mode = DISPLAY_MODE_LETTERBOX;
    sys_data_set_display_mode(&av_set);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_title(facttest_item_strs[idx], NULL,0);
    win_compopup_set_msg(facttest_item_5_strs[0], NULL, 0);
    win_compopup_set_frame(GET_MID_L(350), GET_MID_T(200), 350, 200);
    popup_choice=win_compopup_open_ext(&back_saved);

    if(NUM_ZERO==check_ok_exit_key())
    {
        sys_data_set_display_mode(&sys_data->avset);
        win_compopup_smsg_restoreback();
        return -1;
    }

    /* Step2:  16:9 */
    av_set.tv_ratio = TV_ASPECT_RATIO_169;
    sys_data_set_display_mode(&av_set);
    win_compopup_set_msg_ext(facttest_item_5_strs[1], NULL, 0);
    popup_choice=win_compopup_open();
    if(NUM_ZERO==check_ok_exit_key())
    {
        sys_data_set_display_mode(&sys_data->avset);
        win_compopup_smsg_restoreback();
        return -1;
    }

    /* Step3:  TV */
    api_scart_tvsat_switch(0);
    win_compopup_set_msg_ext(facttest_item_5_strs[2], NULL, 0);
    popup_choice=win_compopup_open();

    check_ok_exit_key();
    //win_compopup_smsg_restoreback();
        win_compopup_smsg_restoreback();
        //return -1;
    //}
    sys_data_set_display_mode(&sys_data->avset);
    api_scart_tvsat_switch(1);


    return 0;
}


static INT32 factory_test_rs232test(UINT32 idx, UINT32 bauto)
{
    UINT8 back_saved=0;
    UINT8 chars[MAX_CHAR_CNT + 30]={0};
    UINT8 ch=0;
    UINT32 i=0;
    UINT32 char_cnt=0;
    UINT32 keytype=0;
    UINT32 vkey=0;
    UINT32 ckeck_exit_key=0;
    BOOL func_flag=FALSE;
    INT32 func_ret=0;
    win_popup_choice_t popup_ret=WIN_POP_CHOICE_NULL;

	if(WIN_POP_CHOICE_NULL == popup_ret)
	{
		;
	}
	if(0 == func_ret)
	{
		;
	}
	if(FALSE == func_flag)
	{
		;
	}
    ap_clear_all_message();

    /* Step1: Display prompt information */
    sci_clear_buff(SCI_FOR_RS232);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_title(facttest_item_strs[idx], NULL,0);
    win_compopup_set_msg(facttest_item_6_strs[0], NULL, 0);
    popup_ret=win_compopup_open_ext(&back_saved);


    /* Step2: Get input from RS2332 */
    i=0;
    char_cnt = 0;

    while( (i++<TIME_OUT) && (NUM_ZERO == char_cnt))
    {
        if(SUCCESS == sci_read_tm(SCI_FOR_RS232, &ch, SCT_READ_TIME))
        {
            chars[char_cnt++] = ch;
        }
        else if( get_key(&keytype,&vkey) >=0 \
             &&(V_KEY_ENTER== vkey || V_KEY_MENU== vkey \
                || V_KEY_EXIT ==vkey) )
        {
            break;
        }
    }

    if(NUM_ZERO == char_cnt)
    {
        if( TIME_OUT == i)
        {
            win_compopup_set_msg_ext((char *)facttest_item_6_strs[1], NULL, 0);
            popup_ret=win_compopup_open();
            func_ret=check_ok_exit_key();
        }
        win_compopup_smsg_restoreback();

        return -1;
    }

    BOOL old_value = ap_enable_key_task_get_key(TRUE);

    chars[char_cnt] = '\0';
    win_compopup_set_msg_ext((char *)chars, NULL, 0);
    popup_ret=win_compopup_open();

    ckeck_exit_key = 1;

    while(1)
    {
        if(SUCCESS ==sci_read_tm(SCI_FOR_RS232, &ch, SCI_READ_TIMEOUT*SCT_READ_TIME))
        {
            if(char_cnt < MAX_CHAR_CNT)
            {
                chars[char_cnt++] = ch;
                chars[char_cnt] = '\0';
                win_compopup_set_msg_ext((char *)chars, NULL, 0);
                popup_ret=win_compopup_open();
            }
            if(MAX_CHAR_CNT == char_cnt)
            {
                //sprintf(&chars[char_cnt]," - full");
                strncpy((char *)&chars[char_cnt], " - full", 7);
            }
            win_compopup_set_msg_ext((char *)chars, NULL, 0);
            popup_ret=win_compopup_open();

            if(MAX_CHAR_CNT == char_cnt)
            {
                break;
            }
        }
        else
        {
            vkey = ap_get_vk();
            if((V_KEY_ENTER ==vkey) || (V_KEY_MENU == vkey) || (V_KEY_EXIT ==vkey))
            {
                ckeck_exit_key = 0;
                break;
            }
        }
    }


    if(ckeck_exit_key)
    {
        func_ret=check_ok_exit_key();
    }
    win_compopup_smsg_restoreback();

    func_flag=ap_enable_key_task_get_key(old_value);

    return 0;
}

static INT32 factory_test_all(UINT32 idx, UINT32 bauto)
{
    UINT32 i=0;
    POBJECT_HEAD nxtobj = NULL;
    POBJECT_HEAD pobj = NULL;
    //PRESULT obj_ret=PROC_PASS;

    MEMSET(nxtobj,0,sizeof(POBJECT_HEAD));
    MEMSET(pobj,0,sizeof(POBJECT_HEAD));
    pobj = osd_get_focus_object((POBJECT_HEAD)&g_win_factory_test);
    INT value=-1;

    MEMSET(item_test_flag,0,sizeof(item_test_flag));
    factest_set_bmpdisplay();
    nxtobj = (POBJECT_HEAD)&facttest_item0;
    while(nxtobj)
    {
        if(nxtobj != pobj)
        {
            osd_draw_object(nxtobj,C_UPDATE_ALL);
        }
        else
        {
            osd_track_object(nxtobj,C_UPDATE_ALL);
        }
        nxtobj = osd_get_objp_next(nxtobj);
    }

    for(i=0;i<FACTROY_TEST_ITEM_CNT - 1;i++)
    {
        if(factory_test_funcs[i](i,1) ==value)
        {
            break;
        }

        item_test_flag[i] = 1;

        factest_set_bmpdisplay();
        nxtobj = (POBJECT_HEAD)&facttest_item0;
        while(nxtobj)
        {
            if(nxtobj != pobj)
            {
                osd_draw_object(nxtobj,C_UPDATE_ALL);
            }
            else
            {
                osd_track_object(nxtobj,C_UPDATE_ALL);
            }
            nxtobj = osd_get_objp_next(nxtobj);
        }

    }

    return RET_SUCCESS;
}


