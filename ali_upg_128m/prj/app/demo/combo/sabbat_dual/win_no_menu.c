/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_no_menu.c
*
*    Description: "Menu not support" nemu.
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
#include <bus/hdmi/m36/hdmi_api.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <api/libchunk/chunk.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "control.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_no_menu.h"
/*******************************************************************************
* Objects declaration
*******************************************************************************/
#ifdef SUPPORT_DEO_HINT

//extern TEXT_FIELD      text_no_menu;
//extern CONTAINER       g_win_no_menu;
#define OSD_HINT_REGION_WIDTH   256//416
#define OSD_HINT_REGION_HEIGHT  50
#define OSD_HINT_REGION_TOP     20
#define OSD_HINT_REGION_LEFT    (680 - OSD_HINT_REGION_WIDTH)


#define TXTN_SH_IDX   WSTL_TEXT_04_8BIT//WSTL_POP_TEXT_8BIT//WSTL_TEXT_13
#define TXTN_HL_IDX   WSTL_TEXT_04_8BIT//WSTL_POP_TEXT_8BIT//WSTL_BUTTON_02_FG
#define TXTN_SL_IDX   WSTL_TEXT_04_8BIT//WSTL_POP_TEXT_8BIT//WSTL_TEXT_13
#define TXTN_GRY_IDX  WSTL_TEXT_04_8BIT//WSTL_POP_TEXT_8BIT//WSTL_TEXT_14

#define CON_SH_IDX  WSTL_POP_WIN_01_8BIT//WSTL_TEXT_12
#define CON_HL_IDX  WSTL_POP_WIN_01_8BIT//WSTL_BUTTON_05
#define CON_HL1_IDX WSTL_POP_WIN_01_8BIT//WSTL_BUTTON_05
#define CON_SL_IDX  WSTL_POP_WIN_01_8BIT//WSTL_TEXT_12
#define CON_GRY_IDX WSTL_POP_WIN_01_8BIT//WSTL_TEXT_14


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
    conobj, ID,1)
#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,str,NULL)

#define LDEF_DEMO(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h,\
                        hl,str) \
    LDEF_CON(root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l,t,w,h,str)

LDEF_DEMO(NULL,g_win_no_menu,NULL,text_no_menu,1,0,0, \
       0,0,256,36,CON_HL_IDX,RS_NOMENU_HINT)

static BOOL  no_menu_hint = FALSE;

void draw_no_menu_hint()
{
    TEXT_FIELD *txt = NULL;
    OBJECT_HEAD *obj = NULL;

    obj = (OBJECT_HEAD*)&g_win_no_menu;

    txt = &text_no_menu;
    //osd_set_text_field_content(txt, STRING_ID,(UINT32)RS_NOMENU_HINT);
    //osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"No OSD");

    osd_draw_object((POBJECT_HEAD)obj, C_UPDATE_ALL);

}

void no_menu_hint_on_off(BOOL flag)
{
    struct osd_device *osd_hint_dev = NULL;

    osd_hint_dev = (struct osd_device *)dev_get_by_name("OSD_M3327C_1");
    if((TRUE == flag) && (FALSE == no_menu_hint))
    {
        osddrv_show_on_off((HANDLE)osd_hint_dev,OSDDRV_ON);
        no_menu_hint = TRUE;
        //ap_draw_nomenu_hint();
    }
    else if((FALSE == flag) && (TRUE == no_menu_hint))
    {
        osddrv_show_on_off((HANDLE)osd_hint_dev,OSDDRV_OFF);
        no_menu_hint = FALSE;
    }
}


void ap_nomenu_hint_init()
{
    struct osd_region_info   osd_region_info;
    struct osd_device *osd_hint_dev = NULL;

    MEMSET(&osd_region_info, 0 , sizeof(struct osd_region_info));
    osd_hint_dev = (struct osd_device *)dev_get_by_name("OSD_M3327C_1");

    //osd_m3327c_detach((struct osd_device *) osd_hint_dev);

    OSD_DRIVER_CONFIG osd_config;
    MEMSET((void *)&osd_config,0,sizeof(OSD_DRIVER_CONFIG));
    osd_config.u_mem_base = ((UINT32)MALLOC(0xF0020) & 0xFFFFFFE0) - (720 * OSD_HINT_REGION_TOP); //0x80780000;
    if(0 == osd_config.u_mem_base)
    {
        libc_printf("%s : malloc failed\n",__FUNCTION__);
    }

    osd_config.u_mem_size = 0xF000;
    osd_config.b_static_block = TRUE;
    osd_config.b_direct_draw = TRUE;
    osd_config.b_cacheable = TRUE;
    osd_config.b_vfilter_enable = FALSE;
    osd_config.b_p2nscale_in_normal_play = TRUE;
    osd_config.b_p2nscale_in_subtitle_play = TRUE;
    osd_config.u_dview_scale_ratio[OSD_PAL][0] = 384;
    osd_config.u_dview_scale_ratio[OSD_PAL][1] = 510;
    osd_config.u_dview_scale_ratio[OSD_NTSC][0] = 384;
    osd_config.u_dview_scale_ratio[OSD_NTSC][1] = 426;
    osd_config.af_par[0].id = 0;
    osd_config.af_par[0].vd = 1;
    osd_config.af_par[0].af = 1;
    osd_config.af_par[0].id = 1;
    osd_config.af_par[0].vd = 1;
    osd_config.af_par[0].af = 1;
    //osd_s36_attach("OSD_S36_0", &osd_config);
    osd_m33c_attach("OSD_M3327C_1", &osd_config);
    //osd_config.u_mem_base = __MM_OSD_BK_ADDR2;
    //osd_config.b_p2nscale_in_normal_play = FALSE;
    //osd_s36_attach("OSD_S36_1", &osd_config);
}

void ap_draw_nomenu_hint()
{
    struct osd_region_info   osd_region_info;
    struct osd_rsc_info      osd_rsc_info;
    struct osd_device *osd_hint_dev = NULL;
    HANDLE handle = 0;
    UINT8  trans_color = 0;
    OSD_RECT rect;

    MEMSET(&osd_region_info,0, sizeof(struct osd_region_info));
    MEMSET(&osd_rsc_info,0, sizeof(struct osd_rsc_info));
    MEMSET(&rect,0, sizeof(OSD_RECT));
    osd_hint_dev = (struct osd_device *)dev_get_by_name("OSD_M3327C_1");

    osd_region_info.osddev_handle = (HANDLE)osd_hint_dev;
    osd_region_info.t_open_para.e_mode = OSD_256_COLOR;
    osd_region_info.t_open_para.u_galpha_enable = IF_GLOBAL_ALPHA;
    osd_region_info.t_open_para.u_galpha = 0x0F;
    osd_region_info.t_open_para.u_pallette_sel = 0;
    osd_region_info.osdrect.u_left = OSD_HINT_REGION_LEFT;
    osd_region_info.osdrect.u_top  = OSD_HINT_REGION_TOP;
    osd_region_info.osdrect.u_width = OSD_HINT_REGION_WIDTH;
    osd_region_info.osdrect.u_height = OSD_HINT_REGION_HEIGHT;
    osd_rsc_info.osd_get_lang_env = osd_get_lang_environment;
    osd_rsc_info.osd_get_obj_info = osd_get_obj_info;
    osd_rsc_info.osd_get_rsc_data= osd_get_rsc_obj_data;
    osd_rsc_info.osd_rel_rsc_data= osd_release_obj_data;
    osd_rsc_info.osd_get_font_lib = osd_get_default_font_lib;
    osd_rsc_info.osd_get_win_style = osdext_get_win_style;
    osd_rsc_info.osd_get_str_lib_id = osdext_get_msg_lib_id;
    #ifdef BIDIRECTIONAL_OSD_STYLE
    osd_rsc_info.osd_get_mirror_flag = osd_get_mirror_flag;
    #endif
    osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
    osd_rsc_info.osd_get_thai_font_data = NULL;

    osd_init_regionfill((struct osd_device *) osd_hint_dev);

    osddrv_show_on_off((HANDLE)osd_hint_dev,OSDDRV_OFF);
    osddrv_delete_region((HANDLE)osd_hint_dev,0);
    //osd_region_init(&osd_region_info,&osd_rsc_info);
    osd_small_region_init(&osd_region_info,&osd_rsc_info);
    osddrv_io_ctl((HANDLE)osd_hint_dev,OSD_IO_UPDATE_PALLETTE,0);
    trans_color = osd_get_trans_color(osd_region_info.t_open_para.e_mode, FALSE);

    handle = osd_get_cur_device_handle();
    osd_set_device_handle((HANDLE)osd_hint_dev);
    rect = osd_region_info.osdrect;
    rect.u_left = rect.u_top = 0;
    draw_no_menu_hint();
    osd_set_device_handle(handle);
}
#endif

