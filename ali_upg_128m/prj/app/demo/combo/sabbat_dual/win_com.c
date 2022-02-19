/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com.c
*
*    Description: The common function of UI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libtsi/db_3l.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#endif
#include <api/libdiseqc/lib_diseqc.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "vkey.h"
#include "key.h"
#include "control.h"
#include "win_com.h"
#include "osd_rsc.h"
#ifdef PARENTAL_SUPPORT
#include "win_signalstatus.h"
#endif
#include "win_password.h"
#include "win_countryband.h"
#include "win_time.h"
#include "ctrl_util.h"
#include "ap_ctrl_display.h"
#include "win_com_list_callback.h"
#include "win_com_list.h"
#include "win_net_choose.h"
#ifdef MULTIVIEW_SUPPORT
#include "win_multiviewset.h"
#endif
#ifdef BC_TEST
#include "win_vmx_test.h"
#endif

/****************************************************************************/
//Tuner index
UINT8 cur_tuner_idx = 0;
UINT8 g_tuner_num = 1;

/****************************************************************************/
/* Title and help                                                           */
/****************************************************************************/

const char *qam_table[] =
{
       "QAM-16",
    "QAM-32",
    "QAM-64",
    "QAM-128",
    "QAM-256",
};

UINT16 yesno_ids[] =
{
    RS_COMMON_NO,
    RS_COMMON_YES,
};

////////////////////////////////////////////////////////
// Title

#ifdef    _BUILD_OTA_E_
#ifndef SD_UI
    #define    TITLE_W     692//493
    #define    TITLE_H     68//320
    #define    TITLE_L     GET_MID_L(TITLE_W)//GET_MID_L(TITLE_W)//60
    #define    TITLE_T     30//(GET_MID_T(TITLE_H) - 40)//60
#else
    #define    TITLE_W     482//493
    #define    TITLE_H     40//320
    #define    TITLE_L     GET_MID_L(TITLE_W)//GET_MID_L(TITLE_W)//60
    #define    TITLE_T     20//(GET_MID_T(TITLE_H) - 40)//60
#endif
#else
#ifndef SD_UI
    #define TITLE_L    74//210
    #define TITLE_T 30//70
#ifdef SUPPORT_CAS_A
    #define TITLE_W 886
#else
    #define TITLE_W 866
#endif
    #define TITLE_H 68
#else
    #define TITLE_L 17//210
    #define TITLE_T 17//70
    #define TITLE_W 570
    #define TITLE_H 40
#endif
#endif

#define TITLE_SH_IDX    WSTL_WIN_TITLE_01_HD

#ifndef SD_UI
// Help
#ifdef USB_MP_SUPPORT
  #define HELP_BTN_T 596//636
#else
  #define HELP_BTN_T 396
#endif
#else
  #define HELP_BTN_T 437//636
#endif

#define HELP_SH_IDX

#define HELP_BACK_SH_IDX    WSTL_TRANS_IX
#define HLEP_BMP_SH_IDX        WSTL_MIX_BMP_HD
#define HLEP_TXT_SH_IDX        WSTL_BUTTON_01_FG_HD

#define HELP_BTN_TYPE (sizeof(btn_num_pos))

#define SUB_LIST_T_OFFSET    40//sharon 47    // sub list title offset
#define SUB_LIST_B_OFFSET    10//sharon 30    // sub list bottom offset
#define SUB_LIST_H_OFFSET     (SUB_LIST_T_OFFSET+SUB_LIST_B_OFFSET)

#define MAX_REGION_LEVEL    4

// menu tilte txt definition
DEF_TEXTFIELD(menu_title_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, TITLE_L,TITLE_T,TITLE_W,TITLE_H, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_TOP, 0,8,0,NULL)

void wincom_open_title(POBJECT_HEAD rootwin,UINT16 title_strid,UINT16 title_bmpid)
{
    TEXT_FIELD *txt = NULL;

    txt = &menu_title_txt;
    osd_set_text_field_content(txt, STRING_ID, (UINT32) title_strid);
    osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
}

void wincom_open_title_ext(POBJECT_HEAD rootwin,UINT8 *title_str,UINT16 title_bmpid)
{
    TEXT_FIELD *txt = NULL;

    txt = &menu_title_txt;
    txt->p_string = display_strs[49];
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32) title_str);
    osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    txt->p_string = NULL;
}

void wincom_close_title(void)
{
    TEXT_FIELD *txt = NULL;

    txt = &menu_title_txt;
    osd_clear_object((POBJECT_HEAD)txt, 0);
}

#ifdef CAS9_V6 //20130709#4_ca_menu
BOOL wincom_is_cnx_title(void)
{
    TEXT_FIELD *txt = NULL;

    txt = &menu_title_txt;
    if(RS_CONAX_CA == txt->w_string_id)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

////////////////////////////////////////////////////////

// menu help_back definition
DEF_TEXTFIELD(menu_help_back,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,HELP_BTN_T,OSD_MAX_WIDTH,40, HELP_BACK_SH_IDX,HELP_BACK_SH_IDX,HELP_BACK_SH_IDX,HELP_BACK_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,0)
// menu help_bmp definition
 DEF_BITMAP(menu_help_bmp,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,0,0, HLEP_BMP_SH_IDX,HLEP_BMP_SH_IDX,HLEP_BMP_SH_IDX,HLEP_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)
// menu help_txt definition
DEF_TEXTFIELD(menu_help_txt,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 0,0,0,0, HLEP_TXT_SH_IDX,HLEP_TXT_SH_IDX,HLEP_TXT_SH_IDX,HLEP_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,0)

typedef struct
{
    OSD_RECT rbmp;
    OSD_RECT rtxt;
}btn_pos_t;

#ifndef SD_UI
btn_pos_t btn_1_pos[]=
{
    { {414,HELP_BTN_T,40,40}, {450,HELP_BTN_T,160,40}},
};

btn_pos_t btn_2_pos[]=
{
    { {314,HELP_BTN_T,40,40}, {350,HELP_BTN_T,160,40}},
    { {614,HELP_BTN_T,40,40}, {650,HELP_BTN_T,160,40}},
};

btn_pos_t btn_3_pos[]=
{
    { {214,HELP_BTN_T,40,40}, {250,HELP_BTN_T,160,40}},
    { {414,HELP_BTN_T,40,40}, {450,HELP_BTN_T,160,40}},
    { {614,HELP_BTN_T,40,40}, {650,HELP_BTN_T,160,40}},
};

btn_pos_t btn_4_pos[]=
{
    { {114,HELP_BTN_T,40,40}, {150,HELP_BTN_T,160,40}},
    { {314,HELP_BTN_T,40,40}, {350,HELP_BTN_T,160,40}},
    { {514,HELP_BTN_T,40,40}, {550,HELP_BTN_T,160,40}},
    { {714,HELP_BTN_T,40,40}, {750,HELP_BTN_T,160,40}},
};

btn_pos_t btn_5_pos[]=
{
    { {10,HELP_BTN_T,40,40}, {46,HELP_BTN_T,160,40}},
    { {210,HELP_BTN_T,40,40}, {246,HELP_BTN_T,160,40}},
    { {410,HELP_BTN_T,40,40}, {446,HELP_BTN_T,160,40}},
    { {610,HELP_BTN_T,40,40}, {646,HELP_BTN_T,160,40}},
    { {810,HELP_BTN_T,40,40}, {846,HELP_BTN_T,160,40}},
};
#else
static btn_pos_t btn_1_pos[]=
{
    { {430,HELP_BTN_T,24,24}, {454,HELP_BTN_T,100,24}},
};

static btn_pos_t btn_2_pos[]=
{
    { {308,HELP_BTN_T,24,24}, {332,HELP_BTN_T,90,24}},
    { {430,HELP_BTN_T,24,24}, {454,HELP_BTN_T,90,24}},
};

static btn_pos_t btn_3_pos[]=
{
    { {186,HELP_BTN_T,24,24}, {210,HELP_BTN_T,90,24}},
    { {308,HELP_BTN_T,24,24}, {332,HELP_BTN_T,90,24}},
    { {430,HELP_BTN_T,24,24}, {454,HELP_BTN_T,90,24}},
};

static btn_pos_t btn_4_pos[]=
{

    { {16,HELP_BTN_T,24,24},  {40,HELP_BTN_T,90,24}},
    { {164,HELP_BTN_T,24,24}, {184+4,HELP_BTN_T,90,24}},
    { {310,HELP_BTN_T,24,24}, {344-10,HELP_BTN_T,90,24}},
    { {452,HELP_BTN_T,24,24}, {474+2,HELP_BTN_T,90,24}},
};

static btn_pos_t btn_5_pos[]=
{
    { {16,HELP_BTN_T,24,24}, {40,HELP_BTN_T,60,24}},
    { {110,HELP_BTN_T,24,24}, {144,HELP_BTN_T,60,24}},
    { {214,HELP_BTN_T,24,24}, {248,HELP_BTN_T,60,24}},
    { {318,HELP_BTN_T,24,24}, {352,HELP_BTN_T,60,24}},
    { {422,HELP_BTN_T,24,24}, {456,HELP_BTN_T,60,24}},
};

#endif

static UINT8 btn_num_pos[] =
{
    1,2,3,4,5
};

static btn_pos_t *btn_type_pos[] =
{
    btn_1_pos,btn_2_pos,btn_3_pos,btn_4_pos,btn_5_pos
};

#ifdef SAT2IP_CLIENT_SUPPORT
extern UINT8 win_satip_client_get_server_num(void);
extern VACTION comlist_menu_satip_client_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
#endif

#ifdef SAT2IP_SERVER_SUPPORT
extern PRESULT comlist_menu_serv_slot_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
#endif


void wincom_open_help(POBJECT_HEAD rootwin,struct help_item_resource *helpinfo,UINT8 item_count)
{
    UINT32 i = 0;
    struct help_item_resource *helpitem = NULL;
    btn_pos_t *btn_pos = NULL;
    btn_pos_t *btn_poss = NULL;
    BITMAP *bmp = NULL;
    TEXT_FIELD *txt = NULL;

    for(i=0;i<HELP_BTN_TYPE;i++)
    {
        if(item_count == btn_num_pos[i])
        {
            break;
        }
    }
    if(i == HELP_BTN_TYPE)
    {
        return;
    }

    btn_poss = btn_type_pos[i];

    osd_draw_object( (POBJECT_HEAD)&menu_help_back, C_UPDATE_ALL);
    for(i=0;i<item_count;i++)
    {
        btn_pos = &btn_poss[i];
        bmp = &menu_help_bmp;
        txt = &menu_help_txt;

        helpitem = &helpinfo[i];

        osd_set_rect2(&bmp->head.frame,&btn_pos->rbmp);
        osd_set_bitmap_content(bmp, helpitem->bmp_id);
        osd_draw_object( (POBJECT_HEAD)bmp, C_UPDATE_ALL);

        osd_set_rect2(&txt->head.frame,&btn_pos->rtxt);
        osd_set_text_field_content(txt, STRING_ID, (UINT32)helpitem->str_id);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}

void wincom_close_help(void)
{
    osd_clear_object( (POBJECT_HEAD)&menu_help_back, C_UPDATE_ALL);
}

////////////////////////////////////////////////////////

//#define FAV_SET_CNT (sizeof(fav_set_strids)/sizeof(fav_set_strids[0]))

#ifdef _BUILD_OTA_E_
UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type,OSD_RECT *p_rect, COM_POP_LIST_PARAM_T *param)
{
    return 0;
}
#else

// sub list (SAT / TP / LNB / DisEqC1.0 )
UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type,OSD_RECT *p_rect, COM_POP_LIST_PARAM_T *param)
{
    struct osdrect rect;
    UINT16 i = 0;
    UINT16 count = 0;
    UINT16 dep = 0;
    UINT16 cur_idx = 0;
    UINT16 style = 0;
    UINT8 mark_align = 0;
    UINT8 offset = 0;
    PFN_KEY_MAP winkeymap = NULL;
    PFN_KEY_MAP list_keymap = NULL;
    PFN_CALLBACK callback = NULL;
    UINT8 back_saved = 0;

    MEMSET(&rect, 0x0, sizeof(struct osdrect));
    #ifndef SD_UI
    dep = (p_rect->u_height - SUB_LIST_H_OFFSET) / 40;
    #else
    dep = (p_rect->u_height - SUB_LIST_H_OFFSET) / 35;//ITEM TH = 32
    #endif

    winkeymap = NULL;
    list_keymap = NULL;

    switch(type)
    {
    case POP_LIST_TYPE_SAT:
    case POP_LIST_TYPE_TP:
    case POP_LIST_TYPE_LNB:
    case POP_LIST_TYPE_DISEQC10:
    case POP_LIST_TYPE_DISEQC11:
    case POP_LIST_TYPE_CHGRPALL:
    case POP_LIST_TYPE_CHGRPFAV:
    case POP_LIST_TYPE_CHGRPSAT:
    case POP_LIST_TYPE_FAVSET:
    case POP_LIST_TYPE_SORTSET:
    case POP_LIST_TYPE_CHANLIST:
    case POP_LIST_MENU_LANGUAGE_OSD:
    case POP_LIST_MENU_LANGUAGE_STREAM:
    case POP_LIST_MENU_TVSYS:
    case POP_LIST_MENU_OSDSET:
    case POP_LIST_TYPE_IF_CHANNEL:
    case POP_LIST_TYPE_CENTRE_FREQ:
    case POP_LIST_DISK_MODE:
    case POP_LIST_DVR_TYPE:
    case POP_LIST_PARTITION:
        comlist_sub_list_common(type, param, &count, &callback, &winkeymap, &list_keymap);
        break;

#ifdef DVR_PVR_SUPPORT
    case POP_LIST_TYPE_RECORDLIST:
  #if (defined(MULTI_PARTITION_SUPPORT) && !defined(DISK_MANAGER_SUPPORT))
    case POP_LIST_TYPE_VOLUMELIST:
  #endif
#endif
#ifndef CI_PLUS_PVR_SUPPORT
    case POP_LIST_MENU_TIME:
#endif
#ifdef KEY_EDIT
    case POP_LIST_MENU_KEYLIST:
#endif
#ifdef DISPLAY_SETTING_SUPPORT
    case POP_LIST_MENU_DISPLAY_SET:
#endif

#ifdef USB_MP_SUPPORT
    case POP_LIST_TYPE_MPSORTSET:
    case POP_LIST_TYPE_VIDEOSORTSET:
#endif
#ifdef DISK_MANAGER_SUPPORT
    case POP_LIST_REMOVABLE_DISK_INFO:
#endif
#ifdef SHOW_WELCOME_SCREEN
    case POP_LIST_MENU_COUNTRY_NETWORK:
#endif
#ifdef _INVW_JUICE  //need to check temp remove
#else
#ifdef PARENTAL_SUPPORT
    case POP_LIST_MENU_PARENTAL_MENU_LOCK:
    case POP_LIST_MENU_PARENTAL_CHANNEL_LOCK:
    case POP_LIST_MENU_PARENTAL_CONTENT_LOCK:
#endif
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    case POP_LIST_SUBT_SET:
#endif
#ifdef IMG_2D_TO_3D
    case POP_LIST_IMAGE_SET:
#endif
        comlist_sub_list_advanced(type, param, &count, &callback, &winkeymap, &list_keymap);
        break;
#ifdef MULTIVIEW_SUPPORT
    case POP_LIST_MULTIVIEW_SET:
        count = win_multiview_get_num();
		callback = comlist_menu_multiview_callback;
		break;
#endif        
#ifdef SAT2IP_SERVER_SUPPORT
    case POP_LIST_MENU_SATIP_SLOT_INSTALL:
        count = win_satip_serv_get_sat_num();
        callback = comlist_menu_serv_slot_callback;
		break;
#endif
#ifdef SAT2IP_CLIENT_SUPPORT
    case POP_LIST_MENU_SATIP_SERVER:
		count = win_satip_client_get_server_num();
		callback = comlist_menu_satip_client_callback;
		break;
#endif
#ifdef NETWORK_SUPPORT
#ifndef LOADER4_AUTO_OTA
    case POP_LIST_MENU_NET_CHOOSE:
        count = win_net_choose_get_card_num();
        callback = comlist_menu_net_choose_callback;
        break;
#endif
#endif
#ifdef DVBC_SUPPORT
    case POP_LIST_MENU_COUNTRY_NETWORK:
		count = win_country_net_get_num();
		callback = comlist_country_net_callback;
		break;
#endif
#ifdef BC_TEST
	case POP_LIST_MENU_VMX_TEST:
        count = win_vmx_test_get_num();
        callback = comlist_vmx_test_callback;
        break;
#endif
    default:
        return 0;
    }

    cur_idx = param->cur;
    osd_set_rect2(&rect,p_rect);
    offset = 30;

    style = LIST_VER | LIST_SCROLL | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE;
    if ((POP_LIST_TYPE_SAT == type) || (POP_LIST_TYPE_FAVSET == type))
    {
        if(POP_LIST_SINGLESELECT == param->selecttype)
        {
            style |= LIST_SINGLE_SLECT;
        }
        else if(POP_LIST_MULTISELECT == param->selecttype)
        {
            style |= LIST_MULTI_SLECT;
        }
    }
    else
    {
        style |= LIST_SINGLE_SLECT;
    }

    mark_align = C_ALIGN_RIGHT;
    win_comlist_reset();
    if(count < 1)
    {
        PRINTF("count < 1 !\n");
    }

    win_comlist_set_sizestyle(count, dep,style);

    if(style & LIST_MULTI_SLECT)
    {
        win_comlist_set_align(40,0,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    }
    else
    {
        win_comlist_set_align(10,0,C_ALIGN_LEFT | C_ALIGN_VCENTER);
    }
    win_comlist_ext_enable_scrollbar(TRUE);
    win_comlist_set_frame(rect.u_left,rect.u_top, rect.u_width, rect.u_height);
    win_comlist_set_mapcallback(list_keymap,winkeymap,callback);

    win_comlist_ext_set_ol_frame(rect.u_left + 10, rect.u_top + SUB_LIST_T_OFFSET, rect.u_width - 40, \
        rect.u_height - SUB_LIST_H_OFFSET);
    win_comlist_ext_set_selmark_xaligen(mark_align,offset);
    win_comlist_ext_set_selmark(0);
    if(style & LIST_SINGLE_SLECT)
    {
        win_comlist_ext_set_item_sel(cur_idx);
    }
    else if(style & LIST_MULTI_SLECT)
    {
        for(i=0;i<count && param->select_array;i++)
        {
            if(param->select_array[i])
            {
                win_comlist_ext_set_item_sel(i);
            }
        }

        win_comlist_ext_set_selmark(IM_TV_FAVORITE);
        win_comlist_ext_set_selmark_xaligen(C_ALIGN_LEFT, 2);
    }
    win_comlist_ext_set_item_cur(cur_idx);
    win_comlist_popup_ext(&back_saved);
    return win_comlist_get_selitem();
}
#endif

#ifndef NEW_DEMO_FRAME
void sat2antenna_ext(S_NODE *sat, struct cc_antenna_info *antenna,UINT32 tuner_idx)
{
    SYSTEM_DATA *sys_data;

    sys_data = sys_data_get();

    if(tuner_idx >= get_tuner_cnt() )
    {
        tuner_idx = 0;
    }

    antenna->lnb_id = tuner_idx + 1;

#ifdef GLOBAL_MOTOR_ANTENNA_SETTING
    if(sys_data->tuner_lnb_type[tuner_idx] != LNB_FIXED)
    {
        MEMSET(antenna,0,sizeof(struct cc_antenna_info));
        antenna->lnb_id = tuner_idx + 1;

        antenna->lnb_type = sys_data->tuner_lnb_antenna[tuner_idx].lnb_type;
        antenna->lnb_low = sys_data->tuner_lnb_antenna[tuner_idx].lnb_low;
        antenna->lnb_high = sys_data->tuner_lnb_antenna[tuner_idx].lnb_high;
        if(sys_data->tuner_lnb_antenna[tuner_idx].motor_type == LNB_MOTOR_DISEQC12)
        {
            antenna->positioner_type = 1; /* DISEQC 1.2 */
            antenna->position         = (tuner_idx == 0) ? sat->position : sat->tuner2_antena.position;
        }
        else
        {
            antenna->positioner_type = 2;/* USALS */
        }
        antenna->longitude = sat->sat_orbit;
        antenna->unicable_pos= (antenna->lnb_id == 2) ?    sat->tuner2_antena.unicable_pos: sat->unicable_pos;
    }
    else
#endif
    {
        if(tuner_idx == 0)
        {
            antenna->di_seq_c_type = sat->di_seq_c_type;
            antenna->di_seq_c_port = sat->di_seq_c_port;
            antenna->di_seq_c11_type = sat->di_seq_c11_type;
            antenna->di_seq_c11_port = sat->di_seq_c11_port;
            antenna->k22 = sat->k22;
            antenna->lnb_high = sat->lnb_high;
            antenna->lnb_low = sat->lnb_low;
            antenna->lnb_type = sat->lnb_type;
             antenna->position = sat->position;
            antenna->positioner_type = 0/*sat->positioner_type*/;
            antenna->toneburst = sat->toneburst;
            antenna->v12 = sat->v12;
            antenna->pol = sat->pol;
            antenna->unicable_pos = sat->unicable_pos;
        }
        else
        {
            antenna->di_seq_c_type = sat->tuner2_antena.di_seq_c_type;
            antenna->di_seq_c_port = sat->tuner2_antena.di_seq_c_port;
            antenna->di_seq_c11_type = sat->tuner2_antena.di_seq_c11_type;
            antenna->di_seq_c11_port = sat->tuner2_antena.di_seq_c11_port;
            antenna->k22 = sat->tuner2_antena.k22;
            antenna->lnb_high = sat->tuner2_antena.lnb_high;
            antenna->lnb_low = sat->tuner2_antena.lnb_low;
            antenna->lnb_type = sat->tuner2_antena.lnb_type;
            antenna->position = sat->tuner2_antena.position;
            antenna->positioner_type = 0/*sat->tuner2_antena.positioner_type*/;
            antenna->toneburst = sat->tuner2_antena.toneburst;
            antenna->v12 = sat->tuner2_antena.v12;
            antenna->pol = sat->tuner2_antena.pol;
            antenna->unicable_pos = sat->tuner2_antena.unicable_pos;
        }
        antenna->longitude = sat->sat_orbit;
    }

    if(LNB_CTRL_UNICABLE == antenna->lnb_type)
    {
        antenna->unicable_ub = sys_data->tuner_lnb_antenna[tuner_idx].unicable_ub;
        antenna->unicable_freq = sys_data->tuner_lnb_antenna[tuner_idx].unicable_freq;
    }
}
#endif

char fre_pat[]  = "r3000~13500";
#ifdef  _BUILD_OTA_E_
char lnb_pat[]  = "r3000~13500";
#endif
#ifndef _S3281_
char symb_pat[]  = "r1000~67500";
#else
char symb_pat[]  = "r1000~49000";//for 3281
#endif

UINT16 fre_sub[10] = {0};
UINT16 symb_sub[10] = {0};
#ifdef  _BUILD_OTA_E_
UINT16 lnb_sub[10] = {0};
#endif
UINT16 tp_polarity_ids[] =
{
    RS_INFO_POLARITY_H,
    RS_INFO_POLARITY_V,
};

void get_chan_group_name(char *group_name,int grp_name_size, UINT8 group_idx,UINT8 *group_type)
{
    UINT8 group_pos = 0;
    UINT16 cur_channel = 0;
    S_NODE s_node;
    char str[50] = {0};
    SYSTEM_DATA *sys_data = NULL;

    MEMSET(&s_node, 0x0, sizeof (s_node));

    sys_data = sys_data_get();

    sys_data_get_cur_mode_group_infor(group_idx, group_type, &group_pos,&cur_channel);
    if(ALL_SATE_GROUP_TYPE == *group_type)
    {
        if(GERMANY_ENV == sys_data->lang.osd_lang)
        {
            if(TV_CHAN == sys_data_get_cur_chan_mode())
            {
                strncpy(str,"Alle TV", 49);
            }
            else
            {
                strncpy(str,"Alle Radio", 49);
            }
        }
        else
        {
            if(TV_CHAN == sys_data_get_cur_chan_mode())
            {
                strncpy(str,"All TV", 49);
            }
            else
            {
                strncpy(str,"All Radio", 49);
            }
        }
    }
    else if(SATE_GROUP_TYPE == *group_type)
    {
        get_sat_at(group_pos,VIEW_ALL,&s_node);
#ifndef DB_USE_UNICODE_STRING
        strncpy(str,s_node.sat_name, 49);
#else
        com_uni_str_copy_char_n((UINT8 *)group_name, s_node.sat_name, grp_name_size);
        return ;
#endif
    }
    else
    {
#ifdef FAV_GROP_RENAME
        snprintf(str,50,"%s",sys_data->favgrp_names[group_pos]);
#else
        strncpy(str,"FAV Group ", 49);
        str_len = STRLEN(str);
        snprintf(&str[str_len], (50-str_len),"%d", group_pos + 1);
#endif
    }

#ifndef DB_USE_UNICODE_STRING
    strncpy(group_name,str, grp_name_size-1);
    group_name[grp_name_size-1] = 0;
#else
    com_asc_str2uni((UINT8 *)str, (UINT16*)group_name);
#endif
}

void get_cur_group_name(char *group_name, int grp_name_size, UINT8 *group_type)
{
    UINT8 cur_group = 0;

    cur_group = sys_data_get_cur_group_index();
    get_chan_group_name(group_name,grp_name_size, cur_group,group_type);
}

void set_container_active(CONTAINER *con,UINT8 action)
{
    POBJECT_HEAD pobj = NULL;

    osd_set_attr(con, action);
    pobj = osd_get_container_next_obj(con);
    while(pobj)
    {
        osd_set_attr(pobj , action);
        pobj = osd_get_objp_next(pobj);
    }
}

void remove_menu_item(CONTAINER *root,OBJECT_HEAD *item,UINT32 height)
{
    OBJECT_HEAD *obj = NULL;
    OBJECT_HEAD *preobj = NULL;
    OBJECT_HEAD *nxtobj = NULL;
    OBJECT_HEAD *head = NULL;
    OBJECT_HEAD *tail = NULL;
    OBJECT_HEAD *newhead = NULL;
    OBJECT_HEAD *newtail = NULL;
    short y = 0;

    y = -height;
    head = osd_get_container_next_obj(root);/*get head node from CONTAINER structure*/
    if(NULL == head)
    {
        return;
    }

    obj = head;
    while(obj!=NULL)
    {
        tail = obj;
        obj = osd_get_objp_next(obj);
    }/*find tail*/

    obj = head;
    preobj = obj;
    while((NULL != obj) && (obj != item))
    {
        preobj = obj;
        obj = osd_get_objp_next(obj);
    }/*find item position*/
    if(NULL == obj)
    {
        return;
    }

    /* Relink object (Remove object of "item" ) */
    nxtobj = osd_get_objp_next(obj);
    if(item != head)
    {
        osd_set_objp_next(preobj, nxtobj);
    }
    else
    {
        osd_set_container_next_obj(root, nxtobj); /* Remove head */
    }

    /* Move objects' postion from start of next item of "item" */
    obj = nxtobj;
    while(obj != NULL)
    {
        osd_move_object(obj, 0, y, TRUE);
        obj = osd_get_objp_next(obj);
    }

    newtail = osd_get_container_next_obj(root);
    newhead = newtail;
    if(NULL == newhead) /* All the items have already been removed */
    {
        return;
    }
    obj = newhead;
    while(obj!=NULL)
    {
        newtail = obj;
        obj = osd_get_objp_next(obj);
    }

    /* Set UP/DOWN ID */
    if((item == head) || (item ==tail))
    {
        newhead->b_up_id  = newtail->b_id;
        newtail->b_down_id= newhead->b_id;
    }
    else
    {
        preobj->b_down_id = nxtobj->b_id;
        nxtobj->b_up_id   = preobj->b_id;
    }

    obj = newhead;
    while(0 == obj->b_id)
    {
        obj =  osd_get_objp_next(obj);
    }/*Xian_Zhou added,to avoid the newhead ID is 0,set Container focus id as the first non-zero object*/
    osd_set_container_focus(root, obj->b_id);
}
void remove_menu_item_conwithout_focus(CONTAINER *root,OBJECT_HEAD *item,UINT32 height)
{
    OBJECT_HEAD *obj = NULL;
    OBJECT_HEAD *preobj = NULL;
    OBJECT_HEAD *nxtobj = NULL;
    OBJECT_HEAD *head = NULL;
    OBJECT_HEAD *tail = NULL;
    OBJECT_HEAD *newhead = NULL;
    OBJECT_HEAD *newtail = NULL;
    short y = 0;

    y = -height;
    head = osd_get_container_next_obj(root);/*get head node from CONTAINER structure*/
    if(NULL == head)
    {
        return;
    }

    obj = head;
    while(obj!=NULL)
    {
        tail = obj;
        obj = osd_get_objp_next(obj);
    }/*find tail*/

    obj = head;
    preobj = obj;
    while((obj!=NULL) && (obj != item))
    {
        preobj = obj;
        obj = osd_get_objp_next(obj);
    }/*find item position*/
    if(NULL == obj)
    {
        return;
    }

    /* Relink object (Remove object of "item" ) */
    nxtobj = osd_get_objp_next(obj);
    if(item != head)
    {
        osd_set_objp_next(preobj, nxtobj);
    }
    else
    {
        osd_set_container_next_obj(root, nxtobj); /* Remove head */
    }

    /* Move objects' postion from start of next item of "item" */
    obj = nxtobj;
    while(obj != NULL)
    {
        osd_move_object(obj, 0, y, TRUE);
        obj = osd_get_objp_next(obj);
    }

    newtail = osd_get_container_next_obj(root);
    newhead = newtail;
    if(NULL == newhead) /* All the items have already been removed */
    {
        return;
    }
    obj = newhead;
    while(obj!=NULL)
    {
        newtail = obj;
        obj = osd_get_objp_next(obj);
    }

    /* Set UP/DOWN ID */
    if((item == head) || (item ==tail))
    {
        newhead->b_up_id  = newtail->b_id;
        newtail->b_down_id= newhead->b_id;
    }
    else
    {
        preobj->b_down_id = nxtobj->b_id;
        nxtobj->b_up_id   = preobj->b_id;
    }
}

static UINT8 cur_backup_region_level = 0;
static UINT8 backup_region_buff_malloc[MAX_REGION_LEVEL] = {0};
static VSCR wincom_backup_vscr[MAX_REGION_LEVEL];
BOOL wincom_backup_region(POSD_RECT frame)
{
    UINT8 *pbuf = NULL;
    UINT8 *pvscr_buff = NULL;
    UINT32 vscr_idx = 0;
    UINT32 vscr_size = 0;
    LPVSCR ap_vscr = NULL;
    LPVSCR pvscr = NULL;
    UINT32 max_vscr_size = 1000000;

    if(cur_backup_region_level >= MAX_REGION_LEVEL)
    {
        return FALSE;
    }
    pvscr = &wincom_backup_vscr[cur_backup_region_level];
    pbuf = NULL;
    vscr_idx = osal_task_get_current_id();
    ap_vscr = osd_get_task_vscr(vscr_idx);
    if (NULL == ap_vscr)
    {
        return FALSE;
    }
    ap_vscr->b_color_mode = osd_get_cur_color_mode();
    osd_update_vscr(ap_vscr);
    ap_vscr->lpb_scr = NULL;

    vscr_size = OSD_GET_VSCR_SIZE(frame, ap_vscr->b_color_mode);

    if((0 == cur_backup_region_level) && (vscr_size < OSD_VSRC_MEM_MAX_SIZE/2) && osd_get_vscr_state())
    {
        #ifdef SUPPORT_CAS9
        pvscr_buff = osd_get_task_vscr_buffer(vscr_idx);
        pbuf = pvscr_buff + OSD_VSRC_MEM_MAX_SIZE/2;
        #else
        pvscr_buff = osd_get_task_vscr_buffer(vscr_idx);
        pbuf = pvscr_buff + vscr_size;
        #endif
        backup_region_buff_malloc[cur_backup_region_level] = 0;
    }
    else
    {
        max_vscr_size = 1000000;
        if(max_vscr_size < vscr_size)
        {
            SDBBP();   //3  vscr size  don't too big for 64M solotion.
        }
        pbuf = (UINT8*)MALLOC(vscr_size);
        if(NULL == pbuf)
        {
            return FALSE;
        }
        backup_region_buff_malloc[cur_backup_region_level] = 1;
    }

    MEMCPY(pvscr, ap_vscr, sizeof(VSCR));
    osd_set_rect2(&pvscr->v_r, frame);
    pvscr->lpb_scr = pbuf;
    if(osd_get_vscr_state())
    {
        osd_get_region_data(pvscr, &pvscr->v_r);
    }
    else
    {
        osdlib_ge_region_read(pvscr, &pvscr->v_r);
    }
    cur_backup_region_level++;
    return TRUE;
}

void wincom_restore_region(void)
{
    UINT32 vscr_idx = 0;
    LPVSCR ap_vscr = NULL;
    LPVSCR pvscr = NULL;
    UINT8 level = 0;

    if((cur_backup_region_level > MAX_REGION_LEVEL) || (0 == cur_backup_region_level))
    {
        return ;
    }

    level = cur_backup_region_level-1;
    pvscr = &wincom_backup_vscr[level];
    vscr_idx = osal_task_get_current_id();
    ap_vscr = osd_get_task_vscr(vscr_idx);
    if (NULL == ap_vscr)
    {
        return;
    }
    if(pvscr->lpb_scr)
    {
        if(osd_get_vscr_state())
        {
            osd_region_write(pvscr, &pvscr->v_r);
        }
        else
        {
            osdlib_ge_region_write(pvscr, &pvscr->v_r);
        }
        if(backup_region_buff_malloc[level])
        {
            FREE(pvscr->lpb_scr);
        }
        backup_region_buff_malloc[level] = 0;
    }
    ap_vscr->lpb_scr = NULL;
    cur_backup_region_level--;
}

UINT16 audio_pid2type(UINT16 audio_pid)
{
    UINT16 type = 0;

    if(AUDIO_TYPE_TEST(audio_pid,AC3_DES_EXIST))
    {
        type  = AUDIO_AC3;
    }
    else if(AUDIO_TYPE_TEST(audio_pid,EAC3_DES_EXIST))
    {
        type  = AUDIO_EC3;
    }
    else if(AUDIO_TYPE_TEST(audio_pid,AAC_DES_EXIST))
    {
        type  = AUDIO_MPEG_AAC;
    }
    else if(AUDIO_TYPE_TEST(audio_pid,ADTS_AAC_DES_EXIST))
    {
        type = AUDIO_MPEG_ADTS_AAC;
    }
    else
    {
        type = AUDIO_MPEG2;
    }
    return type;
}

/**********************************************************
* copy from win_com.c  of  dvbc
***********************************************************/
UINT8 com_str_cat(UINT8  *target, const UINT8 *source)
{
    UINT8 i = 0;
    UINT8 j = 0;

    if (NULL == target)
    {
        return 0;
    }
    i = STRLEN((const char *)target);
    if (NULL == source)
    {
        return i;
    }
    j = 0;
    while (source[j])
    {
        target[i++] = source[j++];
    }
    target[i] = 0;
    return i;
}

void com_int2str(UINT8 *lp_str, UINT32 dw_value, UINT8 b_len)
{
    char *p = NULL;
    char *first_dig = NULL;
    char temp = 0;
    unsigned val = 0;

    p = (char *)lp_str;
    first_dig = p;

    if (b_len > 0)
    {
        do
        {
            //'?o????????y??
            val = (unsigned)(dw_value % 0x0a);
            dw_value   /= 0x0a;
            *p++ = (char)(val + '0');
            b_len--;
        }
        while (b_len > 0);//???????????'????_len????
    }
    else
    {
        do
        {
            val = (unsigned)(dw_value % 0x0a);
            dw_value   /= 0x0a;
            *p++ = (char)(val + '0');
        }
        while (dw_value > 0);//??????????????????????????
    }
    *p-- = '\0';

    //??D??a??
    do
    {
        temp = *p;
        *p   = *first_dig;
        *first_dig = temp;
        --p;
        ++first_dig;
    }
    while (first_dig < p);
}

UINT32 wincom_asc_to_mbuni(UINT8 *ascii_str, UINT16 *uni_str)
{
    int i = 0;

    if ((NULL == ascii_str) || (NULL == uni_str))
    {
        return 0;
    }
    while (ascii_str[i])
    {
        uni_str[i] = (UINT16)ascii_str[i];
        i++;
    }

    uni_str[i] = 0;

#if(SYS_CPU_ENDIAN==ENDIAN_BIG)
    return com_uni_str_len(uni_str);
#else
    i = 0;
    while (uni_str[i])
    {
        uni_str[i] = (UINT16)(((uni_str[i] & 0x00ff) << 8) | ((uni_str[i] & 0xff00) >> 8));
        i++;
    }
    return i;
#endif
}

void wincom_i_to_mbs(UINT16  *str, UINT32 num, UINT32 len)
{
    UINT8 str_buf[20] = {0};

    // convert
    com_int2str(str_buf, num, len);
    wincom_asc_to_mbuni(str_buf, str);
}

UINT32 wincom_mbs_to_i(UINT16  *str)
{
    UINT32 total = 0;

    while (*str != '\0')
    {
        total = 10 * total + (com_mb16to_uint16((UINT8 *)str++) - '0');     /* accumulate digit */
    }

    return total;
}

void wincom_i_to_mbs_with_dot(UINT16  *str, UINT32 num, UINT32 len, UINT8 dot_pos)
{
    UINT8 str_buf[20] = {0};
    UINT8 tmp_buf[20] = {0};
    UINT32 cnt = 0x0;
    UINT32 tmp = 0x1;

    if (0 == dot_pos)
    {
        com_int2str(str_buf, num, len);
    }
    else
    {
        while (dot_pos--)
        {
            tmp = tmp * 10;
            cnt++;
        }

        if (tmp > num)
        {
            com_int2str(str_buf, 0x0, len);
        }
        else
        {
            com_int2str(str_buf, num / tmp, len < cnt ? 0 : len - cnt);
            com_str_cat(str_buf, (const UINT8 *)".");
            com_int2str(tmp_buf, num % tmp, cnt);
            com_str_cat(str_buf, tmp_buf);
        }
    }
    // byte order
    wincom_asc_to_mbuni(str_buf, str);
}

void wincom_mbs_to_i_with_dot(UINT16  *str, UINT32  *num, UINT8  *dot_pos)
{
    BOOL is_find_dot = FALSE;
    UINT16 tmp = 0;

    // init
    *num = 0, *dot_pos = 0;

    while (*str != '\0')
    {
        tmp = com_mb16to_uint16((UINT8 *)str++);

        if (!is_find_dot)
        {
            (*dot_pos)++;
        }

        if (tmp != '.')
        {
            *num = 10*(*num)+(tmp - '0');     /* accumulate digit */
        }
        else
        {
            is_find_dot = TRUE;
        }
    }
}

void dvbc_freq_to_str( UINT32 freq, char *str, UINT32 str_size)
{
    UINT8 j=0;
    UINT8 str_buf[10] = {0};
    UINT8 tmp_buf[10] = {0};
    UINT32 cnt = 0x0;
    UINT32 tmp = 0x1;

    if(NULL == str)
    {
        return;
    }

    j=2;
    while (j--)  // 2 ??D??y??
    {
        tmp = tmp * 10;
        cnt++;
    }
    if (tmp > freq)
    {
        com_int2str(str_buf, 0x0, 6-1);
    }
    else
    {
        com_int2str(str_buf, freq / tmp, (6-1) < cnt ? 0 : (6-1)-cnt);
        com_str_cat(str_buf, (const UINT8 *)".");
        com_int2str(tmp_buf, freq % tmp, cnt);
        com_str_cat(str_buf, tmp_buf);
    }
    snprintf(str,str_size, "%.6s MHz", str_buf);
}

void get_dvbc_freq_str( UINT32 freq, void *str, BOOL uni)
{

}

#ifdef START_RECORD_SHOW_MESSAGE
INT32 record_get_notice_msg(UINT32 prog_id,  record_notice_type notice_type, UINT16 *unimsg)
{
    P_NODE p_node;
    UINT16 string_id;
    UINT32 strlen, prog_name_len;
    UINT16 uni_string[MAX_SERVICE_NAME_LENGTH*2+4];
    char string[MAX_SERVICE_NAME_LENGTH + 2];
    const UINT8 t_invalid_val = 0x80;

    if(NULL == unimsg)
    {
        return -1;
    }
    if(get_prog_by_id(prog_id, &p_node) != DB_SUCCES)
    {
        return -1;
    }

    MEMSET(uni_string, 0, sizeof(UINT16)*(MAX_SERVICE_NAME_LENGTH*2+4));
    MEMSET(string, 0, sizeof(char)*(MAX_SERVICE_NAME_LENGTH+2));

    if(notice_type == NOTICE_RECORD_NEED_CHANNEL_CHANGE)
    {
        string_id = RS_RECORD_CHANGE_CHANNEL_NOTICE;
    }
    else if(notice_type == NOTICE_RECORD_ON_SAME_TP_FREE)
    {
        string_id = RS_RECORD_ON_SAME_TP_NOTICE_FREE;
    }
    else
    {
        return -1;
    }

    com_uni_str_copy_char((UINT8*)unimsg,osd_get_unicode_string(string_id));
    strlen = com_uni_str_len(unimsg);
    strncpy(string, " ", MAX_SERVICE_NAME_LENGTH + 1);//sprintf(string," ");
    com_asc_str2uni(" ",&unimsg[strlen]);
    strlen = com_uni_str_len(unimsg);

#ifndef DB_USE_UNICODE_STRING
    char *p = string;
    if (1 == p_node.ca_mode)
    {
       strncpy(p, "$", MAX_SERVICE_NAME_LENGTH + 1);//sprintf(p,"$");
       p++;
    }

    strncpy(p,p_node.service_name, MAX_SERVICE_NAME_LENGTH + 1);
    for(UINT8 i=0;i<STRLEN(p);i++)
    {
        if(p[i] >= t_invalid_val)
        {
            p[i] = '\0';
            break;
        }
    }
    com_asc_str2uni(string, &unimsg[strlen]);

#else
    if (0 == p_node.ca_mode)
    {
        strncpy(string,"", MAX_SERVICE_NAME_LENGTH + 1);
    }
    else
    {
        strncpy(string,"$", MAX_SERVICE_NAME_LENGTH + 1);
    }

    com_asc_str2uni(string, &unimsg[strlen]);
    strlen = com_uni_str_len(unimsg);
    com_uni_str_copy_char((UINT8*)&unimsg[strlen], p_node.service_name);
#endif

    return 0;
}

#endif

//modify bomur 2011 09 19
/* hex increase the value in specified pos of a unicode string
 * we support low case & upper case for paramer input
 * but the output default is upper case
 */
void increase_val_inuncode(UINT16 *ptr_base,UINT32 pos)
{
    char c = 0;

    c = get_uni_str_char_at(ptr_base,pos);
    c++;
    if (('g' == c) || ('G' == c))
    {
        c = '0';
    }
    else if(':' == c)
    {
        c = 'A';
    }
    else
    {
         c = c;
    }

    set_uni_str_char_at(ptr_base,c,pos);
}
void decrease_val_inuncode(UINT16 *ptr_base,UINT32 pos)
{
    char c = 0;

    c = get_uni_str_char_at(ptr_base,pos);
    c--;

    if ('/' == c)
    {
        c = 'F';
    }
    else if(('@' == c) || ('`' == c))
    {
        c = '9';
    }
    else
    {
        c = c;
    }

    set_uni_str_char_at(ptr_base,c,pos);
}
//modify end

#ifdef  DVBC_SUPPORT

void freq_to_str( UINT32 freq, char *str)
{
    UINT8 j=0;
    UINT8 str_buf[10] = {0};
    UINT8 tmp_buf[10] = {0};
    UINT32 cnt = 0x0;
    UINT32 tmp = 0x1;

    if(NULL == str)
    {
        return;
    }

    j=2;
    while (j--)  // 2 ??D??y??
    {
        tmp = tmp * 10;
        cnt++;
    }
    if (tmp > freq)
    {
        com_int2str(str_buf, 0x0, 6-1);
    }
    else
    {
        com_int2str(str_buf, freq / tmp, (6-1) < cnt ? 0 : (6-1)-cnt);
        com_str_cat(str_buf, (const UINT8*)".");
        com_int2str(tmp_buf, freq % tmp, cnt);
        com_str_cat(str_buf, tmp_buf);
    }
    #ifndef SD_UI
    snprintf(str,30, "%.6s MHz", str_buf);
    #else
    snprintf(str,30, "%.6s", str_buf);
    #endif
}

void get_freq_str( UINT32 freq, void *str, int str_size, BOOL uni)
{
    UINT8 j=0;
    UINT32 ch_no = 0;
    char ch_prefix = 0;
    UINT8 ch_show_no = 0;
    UINT8 str_buf[10] = {0};
    char ch_str[30] = {0};

    band_param *buf = (band_param *)&u_country_param;
    while ((freq > buf[j].end_freq) && (buf[j].end_freq != 0))
    {
        j++;
    }
    if((j>MAX_BAND_COUNT-1) || ((0 == buf[j].start_freq)&&(0 == buf[j].end_freq))) //not find
    {
        freq_to_str(freq, ch_str);
        if(uni)
        {
            com_asc_str2uni((UINT8*)ch_str, (UINT16 *)str);
        }
        else
        {
            strncpy((char *)str, ch_str, str_size-1);
        }
        return;
    }

    UINT32 max_freq = 85800;

    if(freq>max_freq)
    {
        j--;
    }

    ch_no = buf[j].start_ch_no + (freq - buf[j].start_freq)/buf[j].freq_step;
    ch_prefix = buf[j].show_ch_prefix;
    ch_show_no = buf[j].show_ch_no+(ch_no- buf[j].start_ch_no);

    freq_to_str(freq, (char*)str_buf);

    snprintf(ch_str,30, "%c%d(%s)", ch_prefix, ch_show_no, str_buf);
    if(uni)
    {
       com_asc_str2uni((UINT8*)ch_str, (UINT16 *)str);
    }
    else
    {
       strncpy((char *)str, ch_str, str_size-1);
    }
}

#ifdef START_RECORD_SHOW_MESSAGE
INT32 record_get_notice_msg(UINT32 prog_id,  record_notice_type notice_type, UINT16 *unimsg)
{
    P_NODE p_node;
    UINT16 string_id;
    UINT32 strlen, prog_name_len;
    UINT16 uni_string[MAX_SERVICE_NAME_LENGTH*2+4];
    char string[MAX_SERVICE_NAME_LENGTH + 2];
    const UINT8 t_invalid_val = 0x80;

    if(NULL == unimsg)
    {
        return -1;
    }
    if(get_prog_by_id(prog_id, &p_node) != DB_SUCCES)
    {
        return -1;
    }

    MEMSET(uni_string, 0, sizeof(UINT16)*(MAX_SERVICE_NAME_LENGTH*2+4));
    MEMSET(string, 0, sizeof(char)*(MAX_SERVICE_NAME_LENGTH+2));

    if(notice_type == NOTICE_RECORD_NEED_CHANNEL_CHANGE)
    {
        string_id = RS_RECORD_CHANGE_CHANNEL_NOTICE;
    }
    else if(notice_type == NOTICE_RECORD_ON_SAME_TP_FREE)
    {
        string_id = RS_RECORD_ON_SAME_TP_NOTICE_FREE;
    }
    else
    {
        return -1;
    }

    com_uni_str_copy_char((UINT8*)unimsg,osd_get_unicode_string(string_id));
    strlen = com_uni_str_len(unimsg);
    strncpy(string, " ", MAX_SERVICE_NAME_LENGTH + 1);//sprintf(string," ");
    com_asc_str2uni(" ",&unimsg[strlen]);
    strlen = com_uni_str_len(unimsg);

#ifndef DB_USE_UNICODE_STRING
    char *p = string;
    if(1 == p_node.ca_mode)
    {
       strncpy(p, "$", MAX_SERVICE_NAME_LENGTH + 1);//sprintf(p,"$");
       p++;
    }

    strncpy(p,p_node.service_name, MAX_SERVICE_NAME_LENGTH + 1);
    for(UINT8 i=0;i<STRLEN(p);i++)
    {
        if(p[i] >= t_invalid_val)
        {
            p[i] = '\0';
            break;
        }
    }
    com_asc_str2uni(string, &unimsg[strlen]);

#else
    if(0 == p_node.ca_mode)
    {
        strncpy(string,"", MAX_SERVICE_NAME_LENGTH + 1);
    }
    else
    {
        strncpy(string,"$", MAX_SERVICE_NAME_LENGTH + 1);
    }

    com_asc_str2uni(string, &unimsg[strlen]);
    strlen = com_uni_str_len(unimsg);
    com_uni_str_copy_char((UINT8*)&unimsg[strlen], p_node.service_name);

#endif
    return 0;
}
#endif

#endif
