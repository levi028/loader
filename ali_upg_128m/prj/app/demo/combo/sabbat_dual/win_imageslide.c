  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_imageslide.c
*
*    Description: Mediaplayer image slide UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>
#include <api/libmp/pe.h>
#include <api/libimagedec/imagedec.h>
#include <api/libosd/osd_lib.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>

#include "control.h"
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_imagepreview.h"
#include "win_imageslide.h"
#include "win_playlist_setup.h"
#include "media_control.h"
#include "win_filelist.h"
#include "win_mpzoom.h"

/*******************************************************************************
*    Macro definition
*******************************************************************************/
#define SLIDE_SH_IDX WSTL_NOSHOW_IDX
#define EXIT_TXT_IDX WSTL_TEXT_06
#define IDX_TXT_IDX WSTL_TEXT_07
#define ROT_BMP_IDX WSTL_NOSHOW_IDX
#define IMG_PAUSE_STATE 1
#define IMG_UNPAUSE_STATE 0
#define IMAGE_ANGLE_CNT 4
#define VACT_DECREASE (VACT_PASS +1)
#define VACT_INCREASE (VACT_PASS +2)

#define VACT_ROTATEL (VACT_PASS +3)
#define VACT_ROTATER (VACT_PASS +4)
#define VACT_PAUSE (VACT_PASS +5)
#define VACT_ZOOM (VACT_PASS +6)
#define VACT_MP (VACT_PASS +7)
#define VACT_PLAY (VACT_PASS + 8)

#ifdef IMG_2D_TO_3D
#define VACT_SWAP (VACT_PASS + 9)
#define VACT_INFO (VACT_PASS + 10)
#endif

#ifndef SD_UI
#define W_L  124
#define W_T  68
#ifdef SUPPORT_CAS_A
#define W_W 362
#else
#define W_W 342
#endif
#define W_H  64
#define SLIDE_OFFSET 8

#define EXIT_L (W_L + SLIDE_OFFSET)
#define EXIT_T (W_T + SLIDE_OFFSET)
#define EXIT_W 52
#define EXIT_H  30

#define ROT_L EXIT_L + EXIT_W +SLIDE_OFFSET
#define ROT_T (EXIT_T)
#define ROT_W 56
#define ROT_H  30

#define IDX_L ROT_L + ROT_W +SLIDE_OFFSET
#define IDX_T (EXIT_T - 3)
#define IDX_W 214  //for more than 10000 pics, can't show all
#define IDX_H  36

#define PAUSE_L 924
#define PAUSE_T EXIT_T
#define PAUSE_W 72
#define PAUSE_H  64
#else
#define W_L  124
#define W_T  68
#define W_W 342
#define W_H  64
#define SLIDE_OFFSET 8

#define EXIT_L (W_L + SLIDE_OFFSET)
#define EXIT_T (W_T + SLIDE_OFFSET)
#define EXIT_W 52
#define EXIT_H  30

#define ROT_L EXIT_L + EXIT_W +SLIDE_OFFSET
#define ROT_T (EXIT_T)
#define ROT_W 56
#define ROT_H  30

#define IDX_L ROT_L + ROT_W +SLIDE_OFFSET
#define IDX_T (EXIT_T - 3)
#define IDX_W 214  //for more than 10000 pics, can't show all
#define IDX_H  36

#define PAUSE_L 500
#define PAUSE_T EXIT_T
#define PAUSE_W 72
#define PAUSE_H  64
#endif


#define LOADING_MAX  ( sizeof(loading_bmp_ids)/sizeof(loading_bmp_ids[0]) )
/*******************************************************************************
*    Function decalare
*******************************************************************************/
static PRESULT win_imageslide_unkown_act_proc(VACTION act);
static void win_imageslide_showexit(UINT8 flag);
static void win_imageslide_showrotate(UINT8 flag);
static void win_imageslide_showrotateicon(UINT8 flag);
static void win_imageslide_showidx(UINT8 flag);
static void win_imageslide_showpause(UINT8 flag);
static void win_imageslide_handler();
static void win_imageslide_proc_handler();
static VACTION slide_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT slide_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SLIDE_SH_IDX,SLIDE_SH_IDX,SLIDE_SH_IDX,SLIDE_SH_IDX,   \
    slide_keymap,slide_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, l,t,w,h, IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_BMP(root,varbmp,nxt_obj,l,t,w,h,bmp_id)        \
    DEF_BITMAP(varbmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, ROT_BMP_IDX,ROT_BMP_IDX,ROT_BMP_IDX,ROT_BMP_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmp_id)

/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
LDEF_BMP(g_win_imageslide,imageslide_exit,&imageslide_rotate, \
    EXIT_L,EXIT_T,EXIT_W,EXIT_H,IM_MEDIA_PLAYER_PIC_06)

LDEF_BMP(g_win_imageslide,imageslide_rotate,&imageslide_idx,    \
    ROT_L,ROT_T,ROT_W,ROT_H,IM_MEDIAPLAY_EXIT)

LDEF_TXTIDX(g_win_imageslide,imageslide_idx,NULL, \
    IDX_L,IDX_T,IDX_W,IDX_H,display_strs[0])

LDEF_TXTIDX(g_win_imageslide,imageslide_user_para,NULL, \
    PAUSE_L - 180,PAUSE_T - 10, 190, PAUSE_H,display_strs[0])

LDEF_BMP(g_win_imageslide,imageslide_pause,NULL,PAUSE_L,PAUSE_T,PAUSE_W, PAUSE_H,IM_PAUSE)

LDEF_BMP(g_win_imageslide,imageslide_rotateicon,NULL,PAUSE_L,PAUSE_T,46, 46,IM_MEDIAPLAY_ROTATE)

LDEF_WIN(g_win_imageslide,&imageslide_exit,W_L,W_T,W_W, W_H, 0)

DEF_TEXTFIELD(decode_txt_progress,NULL,NULL,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, \
    (OSD_MAX_WIDTH >> 1) - 20,(OSD_MAX_HEIGHT>>1),60,28, \
    IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[1])

DEF_BITMAP(decode_bmp_progress,NULL,NULL,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, \
    (OSD_MAX_WIDTH >> 1) - 36,(OSD_MAX_HEIGHT>>1),74,74, \
    IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,IDX_TXT_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0)
/*******************************************************************************
*    Variable definition
*******************************************************************************/
BOOL from_imagepreview = FALSE;
BOOL back_to_filelist = FALSE;
BOOL g_from_imageslide = FALSE;
play_list_handle cur_playlist_image = 0;
UINT16 cur_image_idx = 0;
UINT8 image_folderloop_flag = 0;
ID imageslide_timer_id = 0;
extern UINT32 back_to_mainmenu;

#ifdef IMG_2D_TO_3D
IMAGE_SET image_setting;
UINT8 image_setting_inited = 0;//whether image setting has inited
BOOL from_imagesetting = FALSE;
#endif

static UINT32 timer_slide = 0;
static UINT8 timer_start_flag = 0;
static UINT8 image_rot_ang = 0;
static ID imageslide_proc_id = 0;
static UINT8 proc_cnt = 0;
static UINT8 g_patch_exit_once_flag = 0; // for DLNA
static BOOL imageslide_pause_state = IMG_UNPAUSE_STATE;
static UINT8 loading_progress = 0;
static BOOL decode_finish_flag = FALSE;
static UINT16 loading_bmp_ids[] =
{
    IM_MEDIAPLAY_LOADING_01,
    IM_MEDIAPLAY_LOADING_02,
    IM_MEDIAPLAY_LOADING_03,
    IM_MEDIAPLAY_LOADING_04,
    IM_MEDIAPLAY_LOADING_05,
    IM_MEDIAPLAY_LOADING_06,
    IM_MEDIAPLAY_LOADING_07,
    IM_MEDIAPLAY_LOADING_08,
    IM_MEDIAPLAY_LOADING_09,
    IM_MEDIAPLAY_LOADING_10,
    IM_MEDIAPLAY_LOADING_11,
    IM_MEDIAPLAY_LOADING_12
};
/*******************************************************************************
*    Functions definition
*******************************************************************************/
static VACTION slide_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_ROTATEL;
        break;
    case V_KEY_RIGHT:
        act = VACT_ROTATER;
        break;
    case V_KEY_UP:
        act = VACT_INCREASE;
        break;
    case V_KEY_DOWN:
        act = VACT_DECREASE;
        break;
    case V_KEY_PAUSE:
        act = VACT_PAUSE;
        break;
    case V_KEY_PLAY:
        act = VACT_PLAY;
        break;
    case V_KEY_ZOOM:
        act = VACT_ZOOM;
        break;
    case V_KEY_MP:
        act = VACT_MP;
        break;
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    case V_KEY_MENU:
        if(TRUE == from_imagepreview)
        {
            back_to_filelist = TRUE;
        }
        act = VACT_CLOSE;
        break;
#ifdef IMG_2D_TO_3D
    case V_KEY_SWAP:
        act = VACT_SWAP;
        break;
    case V_KEY_INFOR:
        act = VACT_INFO;
        break;
#endif
    default:
        act = VACT_PASS;
        break;

    }

    return act;
}

#define JPEG_POP_NOTSUPPORT_MSG	//jpeg pop msg
#ifdef JPEG_POP_NOTSUPPORT_MSG	
//static UINT8 jpeg_prompt_on_screen = 0;
static void jpeg_file_prompt(char *str_prompt,UINT32 param)
{
	UINT16	uni_prompt[32];
#ifndef SD_UI
	DEF_TEXTFIELD(txtprompt,NULL,NULL,C_ATTR_ACTIVE,C_FONT_2, \
	0,0,0,0,0, 300,250,520,50, WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07, \
	NULL,NULL,	\
	C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,uni_prompt)
#else
	DEF_TEXTFIELD(txtprompt,NULL,NULL,C_ATTR_ACTIVE,C_FONT_2, \
	0,0,0,0,0, 200,230,250,40, WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07, \
	NULL,NULL,	\
	C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,uni_prompt)
#endif	

	if(param > 0)
	{
		osd_set_text_field_content(&txtprompt,STRING_ANSI,(UINT32)str_prompt);
		osd_draw_object((POBJECT_HEAD)&txtprompt,C_UPDATE_ALL);
	}
	else
	{
		osd_clear_object((POBJECT_HEAD)&txtprompt,C_UPDATE_ALL);
	}
}
#endif

static void slide_callback_evn_pre_open(void)
{
    imageslide_timer_id = OSAL_INVALID_ID;
    imageslide_proc_id = OSAL_INVALID_ID;
    g_patch_exit_once_flag = 0;
    if(TRUE == from_imagepreview )
    {
        imageslide_pause_state = IMG_PAUSE_STATE;
    }
    else
    {
        imageslide_pause_state = IMG_UNPAUSE_STATE;
    }
#ifdef IMG_2D_TO_3D
    if(from_imagesetting == TRUE)
    {
        imageslide_pause_state = IMG_PAUSE_STATE;
        from_imagesetting = FALSE;
    }

    if(0==image_setting_inited)
    {
        image_setting.image_osd_onoff = IMAGE_SET_ON;
        image_setting.image_3d_onoff = IMAGE_SET_OFF;
        image_setting.image_3d_output_format = IMAGE_SET_3DOUTPUT_RB;
        image_setting_inited = 1;
    }
    win_imageslide_osd_onoff(image_setting.image_osd_onoff);
#endif
    win_imageslide_showpause(TRUE);
    api_set_preview_vpo_color(FALSE);
}

static void slide_callback_evn_pre_close(media_file_info *file_node, char *image_name, int n_size,
    UINT8 *p_timer_start_flag)
{
    #ifdef  DUAL_ENABLE
    #ifdef  PNG_GIF_TEST
    enum tvsystem tvsys = PAL;
    UINT32 region_id =0;
    struct osd_device *osd_dev = NULL;
    struct osdrect p_rect;
    struct osdpara open_para;
    #endif
    #endif

    if((!file_node) || (!image_name) || (!p_timer_start_flag))
    {
        libc_printf("%s: file_node or image_name is null\n", __FUNCTION__);
        return ;
    }
    g_from_imageslide = TRUE;
    imagedec_ioctl(1, IMAGEDEC_IO_CMD_CLEAN_FRM, TRUE);    //when quit win_slide, clean 2 frm buffer
    if(1 == image_folderloop_flag)//filelist
    {
        if(RET_SUCCESS == get_file_from_play_list(cur_playlist_image, cur_image_idx, file_node))
        {
            win_get_fullname(image_name, n_size, file_node->path, file_node->name);
            usblist_set_filelist_param(image_name);
        }
    }
    image_abort();
#ifdef  DUAL_ENABLE
#ifdef  PNG_GIF_TEST
    MEMSET(&p_rect, 0, sizeof(struct osdrect));
    MEMSET(&open_para, 0, sizeof(struct osdpara));
    p_rect.u_left =136;
    p_rect.u_top = 40;
    p_rect.u_width = 1008;//OSD_MAX_WIDTH;//608;
    p_rect.u_height = 640;//OSD_MAX_HEIGHT;//430;
    open_para.e_mode = OSD_HD_ARGB1555;
    open_para.u_galpha = 0x0f;
    open_para.u_galpha_enable = 0;
    open_para.u_pallette_sel = 0;
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    if (osd_dev != NULL)
    {
        osddrv_close((HANDLE)osd_dev);
    }
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
    osddrv_open((HANDLE)osd_dev, &open_para);
    osal_task_sleep(20);
    osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_SET_TRANS_COLOR, 0x00);
    osddrv_create_region((HANDLE)osd_dev, region_id, &p_rect, NULL);
    p_rect.u_left = p_rect.u_top = 0;
    osddrv_region_fill((HANDLE)osd_dev,region_id,&p_rect,0x00);
    osd_show_on_off(OSDDRV_OFF);
    set_osd_pos_for_tv_system(tvsys);
    osd_show_on_off(OSDDRV_ON);
#endif
#endif
    api_stop_timer(&imageslide_timer_id);
    api_stop_timer(&imageslide_proc_id);
    *p_timer_start_flag = 0;
    imageslide_timer_id = OSAL_INVALID_ID;
    imageslide_proc_id = OSAL_INVALID_ID;
    win_set_image_multiview_firstidx();
    osd_clear_object((POBJECT_HEAD)&imageslide_idx,C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&imageslide_user_para,C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&imageslide_pause,C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&decode_txt_progress,C_UPDATE_ALL);
    osd_clear_object((POBJECT_HEAD)&decode_bmp_progress,C_UPDATE_ALL);
}
static PRESULT slide_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 slide_interval = 0;
    play_list_loop_type loop_type = PLAY_LIST_NONE;
    VACTION unact = 0;
    media_file_info file_node;
    char *image_name = NULL;
    int next_image = 0;
    UINT32 percent_max = 100;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    image_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
    if(NULL == image_name)
    {
        libc_printf("image_name in slide_callback malloc fail\n");
        ASSERT(0);//SDBBP();
        return ret;
    }
    MEMSET(image_name, 0, FULL_PATH_SIZE);
    switch(event)
        {
        case EVN_PRE_OPEN:
            slide_callback_evn_pre_open();
            break;
        case EVN_POST_OPEN:
            #if defined(_USE_64M_MEM_)
            dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
            dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
            #endif

            vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RELEASE_VFB, 0);
            image_rot_ang = ANG_ORI;
            loop_type = image_slide_setting.b_repeat ? PLAY_LIST_REPEAT : pl_ay_list_sequence;
            set_play_list_info(cur_playlist_image,&cur_image_idx,&loop_type);
            get_file_from_play_list(cur_playlist_image, cur_image_idx, &file_node);
            win_get_fullname(image_name, FULL_PATH_SIZE, file_node.path, file_node.name);
            image_abort();
            timer_slide = osal_get_tick();
#ifdef IMG_2D_TO_3D
            image_set_disp_param(image_setting.image_3d_onoff, image_setting.image_3d_output_format, 60, 20, 60);
#endif
            win_image_file_play(image_name, image_rot_ang);
            win_imageslide_showexit(TRUE);
            win_imageslide_showrotate(TRUE);
            win_imageslide_showidx(TRUE);
            win_imageslide_show_user_para(TRUE);
            decode_finish_flag = FALSE;
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            ret = win_imageslide_unkown_act_proc(unact);
            break;
        case EVN_PRE_CLOSE:
            slide_callback_evn_pre_close(&file_node, image_name, FULL_PATH_SIZE, &timer_start_flag);
            break;
        case EVN_POST_CLOSE:
            image_restore_vpo_rect();//TODO
            	  back_to_mainmenu=0;
            break;
        case EVN_MSG_GOT:
                switch(param1)
                {
                    case CTRL_MSG_SUBTYPE_CMD_STO:
                        if(USB_STATUS_OVER == param2)
                        {
                            file_list_check_storage_device(FALSE, FALSE);
                        }
                        break;
                    case CTRL_MSG_SUBTYPE_STATUS_IMGTOOVER:
                        image_abort();
                        if(PLAY_LIST_NONE == win_filelist_get_loop_mode())
                        {
                            ret = PROC_LEAVE;
                        }
                        else
                        {
                            next_image = get_next_index_from_play_list(cur_playlist_image);
                            osd_clear_object((POBJECT_HEAD)&imageslide_user_para,C_UPDATE_ALL);
                            if (next_image != -1)
                            {
                                cur_image_idx = next_image;
                                set_play_list_info(cur_playlist_image,&cur_image_idx,NULL);
                                get_file_from_play_list(cur_playlist_image, cur_image_idx, &file_node);
                                win_get_fullname(image_name, FULL_PATH_SIZE, file_node.path, file_node.name);
#ifdef IMG_2D_TO_3D
                                image_set_disp_param(image_setting.image_3d_onoff,
                                                                   image_setting.image_3d_output_format, 60, 20, 60);
#endif
                                win_image_file_play(image_name, image_rot_ang);
                                decode_finish_flag = FALSE;
                                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, TRUE);
                            }
                            else
                            {
                                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 0,TRUE);
                            }
                        }
                        break;
                    case CTRL_MSG_SUBTYPE_CMD_EXIT:
                        if(TRUE == from_imagepreview  )
                        {
                            back_to_filelist = TRUE;
                        }
                        ret = PROC_LEAVE;
                        break;
                    case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
                        win_imageslide_showidx(TRUE);
                        win_imageslide_show_user_para(TRUE);
                        ret = PROC_LOOP;
                        break;
                    case CTRL_MSG_SUBTYPE_STATUS_MP3OVER:
                        win_play_next_music_ex();
                        break;
                    case CTRL_MSG_SUBTYPE_STATUS_PROC_UPDATE:
                        osd_set_bitmap_content(&decode_bmp_progress, loading_bmp_ids[proc_cnt]);
                        osd_draw_object((POBJECT_HEAD)&decode_bmp_progress,C_UPDATE_ALL);
                        proc_cnt++;
                        if(proc_cnt >= LOADING_MAX)
                        {
                            proc_cnt = 0;
                        }
                        break;
#ifdef NETWORK_SUPPORT
                    case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH:
                        if (((INT32)param2 == -(NET_ERR_CONNECT_FAILED)) && (g_patch_exit_once_flag == 0))
                        {
                            ap_send_key(V_KEY_EXIT, FALSE);
                            g_patch_exit_once_flag = 1;
                        }
                        break;
#endif
#ifdef JPEG_POP_NOTSUPPORT_MSG
                    case CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR:
                    {
                        char str_prompt[32];
                        sprintf(str_prompt,"Unsupport file!");
                        jpeg_file_prompt(str_prompt,1);
                        osal_task_sleep(1000);
                        jpeg_file_prompt(NULL,0);
                    }
                    break;
#endif
                    default:
                        break;
                }
               if(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1)
                {
                    if(param2 > timer_slide)
                    {
                        image_abort();
                    }
                    else
                    {
                        libc_printf("slide @@@@@@@@@@@@@@ es case\n");
                        break;
                    }
                    decode_finish_flag = TRUE;
                    if(IMG_UNPAUSE_STATE == imageslide_pause_state)
                    {
                        slide_interval = (image_slide_setting.u_time_gap * 1000);
                        if(0 != slide_interval)
                        {
                            imageslide_timer_id = api_start_timer("I",slide_interval,win_imageslide_handler);
                        }
                        else
                        {
                            imageslide_pause_state = IMG_PAUSE_STATE;
                            win_imageslide_showpause(TRUE);
                        }
                    }
                }
                if ((CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS == param1) ||
                    (CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1))
                {
                    if ((param2 < percent_max) && (CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS == param1))
                    {
                        loading_progress = (param2 * LOADING_MAX)/100;
                        if (loading_progress > LOADING_MAX - 1)
                        {
                            loading_progress = LOADING_MAX - 1;
                        }
                        if(0 == timer_start_flag)
                        {
                            imageslide_proc_id = OSAL_INVALID_ID;
                            imageslide_proc_id = api_start_cycletimer("S", 200, win_imageslide_proc_handler);
                            timer_start_flag = 1;
                        }
                    }
                    else if((param2 >= percent_max) || (CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1))
                    {
                        osd_clear_object((POBJECT_HEAD)&decode_bmp_progress,C_UPDATE_ALL);
                        api_stop_timer(&imageslide_proc_id);
                        imageslide_proc_id = OSAL_INVALID_ID;
                        timer_start_flag = 0;
                        proc_cnt = 0;
                    }
                }
                break;
        default:
                break;
        }
       if(NULL != image_name)
        {
            free(image_name);
            image_name = NULL;
        }
    return ret;
}

static void win_imageslide_unknown_act_proc_rotate(VACTION act)
{
    if(IMG_UNPAUSE_STATE == imageslide_pause_state)
    {
        api_stop_timer(&imageslide_timer_id);
        imageslide_pause_state = IMG_PAUSE_STATE;
    }
    if(VACT_ROTATER == act)
    {
        image_rot_ang = (image_rot_ang + 1)%IMAGE_ANGLE_CNT;
    }
    else
    {
        image_rot_ang = (image_rot_ang+IMAGE_ANGLE_CNT - 1)%IMAGE_ANGLE_CNT;
    }
    image_rotate(image_rot_ang);
    if(ANG_ORI == image_rot_ang)
    {
        win_imageslide_showrotateicon(FALSE);
        win_imageslide_showpause(TRUE);
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)&imageslide_pause, 0);
        win_imageslide_showrotateicon(TRUE);
    }
    win_imageslide_showidx(TRUE);
}

#ifdef IMG_2D_TO_3D
static void win_imageslide_unknown_act_proc_swap(PRESULT *ret)
{
    UINT8 back_saved = 0;
    INT8 tmp_3d_onoff = 0;

    if(!ret)
    {
        libc_printf("%s: ret is null\n", __FUNCTION__);
        return;
    }
    if(decode_finish_flag && (image_rot_ang == ANG_ORI))
    {
        tmp_3d_onoff = image_setting.image_3d_onoff;
        if(IMAGE_SET_ON==tmp_3d_onoff)
        {
            image_setting.image_3d_onoff = IMAGE_SET_OFF;
        }
        else
        {
            image_setting.image_3d_onoff = IMAGE_SET_ON;
        }
        win_imageslide_show_user_para(TRUE);
        imageslide_pause_state = IMG_PAUSE_STATE;
        win_imageslide_showpause(TRUE);
        api_stop_timer(&imageslide_timer_id);

        if(IMAGE_SET_ON==image_setting.image_3d_onoff)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(200, 200, 200, 100);
            win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
            win_compopup_open_ext(&back_saved);
        }
        image_set_disp_param(image_setting.image_3d_onoff, image_setting.image_3d_output_format, 60, 20, 60);
        image_2d_to_3d_swap(image_setting.image_3d_onoff);
        if(IMAGE_SET_ON==image_setting.image_3d_onoff)
        {
            osal_task_sleep(50);
            win_compopup_smsg_restoreback();
            sys_data_save(1);
        }
        *ret = PROC_LOOP;
    }
}
#endif
static PRESULT win_imageslide_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_PASS;
    POBJECT_HEAD win = NULL;
    POBJECT_HEAD mplayer_menu = NULL;
    int next_image = 0;
    UINT16 slide_interval = 0;
    media_file_info file_node;
    char *image_name = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    switch(act)
    {
    case VACT_ROTATEL:
    case VACT_ROTATER:
        if(FALSE == decode_finish_flag)
        {
            break;
        }
#ifdef IMG_2D_TO_3D
        if(IMAGE_SET_ON==image_setting.image_3d_onoff)
        {
            break;
        }
#endif
        win_imageslide_unknown_act_proc_rotate(act);
        break;
    case VACT_INCREASE:
    case VACT_DECREASE:
        if(IMG_UNPAUSE_STATE == imageslide_pause_state)
        {
            api_stop_timer(&imageslide_timer_id);
            imageslide_pause_state = IMG_PAUSE_STATE;
            win_imageslide_showpause(TRUE);
        }
        if(VACT_INCREASE == act)
        {
            next_image = get_next_index_from_play_list(cur_playlist_image);
        }
        else
        {
            next_image = get_previous_index_from_play_list(cur_playlist_image);
        }
        if(next_image != -1)
        {
            cur_image_idx = next_image;
            api_stop_timer(&imageslide_timer_id);
            image_abort();
            if(image_rot_ang!= ANG_ORI)
            {
                image_rot_ang = ANG_ORI;
                win_imageslide_showrotateicon(FALSE);
                win_imageslide_showpause(TRUE);
            }
            osal_task_sleep(20);            // to end last decoder task
            set_play_list_info(cur_playlist_image,&cur_image_idx,NULL);
            get_file_from_play_list(cur_playlist_image, cur_image_idx, &file_node);
                    if(NULL == image_name)
                    {
                        image_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
                        if(NULL == image_name)
                        {
                            libc_printf("image_name* malloc fail\n");
                            ASSERT(0);
                            return ret;
                        }
                    }
                    MEMSET(image_name,0,FULL_PATH_SIZE);
            win_get_fullname(image_name, FULL_PATH_SIZE, file_node.path, file_node.name);
#ifdef IMG_2D_TO_3D
            image_set_disp_param(image_setting.image_3d_onoff, image_setting.image_3d_output_format, 60, 20, 60);
#endif
            win_image_file_play(image_name, image_rot_ang);
            decode_finish_flag = FALSE;
            osal_task_sleep(20);            // to run current decoder task
            win_imageslide_showidx(TRUE);
            win_imageslide_show_user_para(TRUE);
        }
        break;
    case VACT_PAUSE:
        if(IMG_UNPAUSE_STATE == imageslide_pause_state)
        {
            imageslide_pause_state = IMG_PAUSE_STATE;
            win_imageslide_showpause(TRUE);
            api_stop_timer(&imageslide_timer_id);
        }
        else
        {
            if(ANG_ORI == image_rot_ang)
            {
                imageslide_pause_state = IMG_UNPAUSE_STATE;
                slide_interval = (image_slide_setting.u_time_gap * 1000);
                win_imageslide_showpause(TRUE);
                           image_abort();
                if(0 != image_slide_setting.u_time_gap)
                {
                    imageslide_timer_id = api_start_timer("I",slide_interval,win_imageslide_handler);
                           }
             }
        }
        ret = PROC_LOOP;
        break;
    case VACT_PLAY:
        if(IMG_PAUSE_STATE == imageslide_pause_state)
        {
            if(ANG_ORI == image_rot_ang)
            {
                imageslide_pause_state = IMG_UNPAUSE_STATE;
                win_imageslide_showpause(TRUE);
            }
            else
            {
                imageslide_pause_state = IMG_UNPAUSE_STATE;
                win_imageslide_showrotateicon(FALSE);
            }
            slide_interval = (image_slide_setting.u_time_gap * 1000);
            image_abort();
            if(0 != image_slide_setting.u_time_gap)
            {
                imageslide_timer_id = api_start_timer("I",slide_interval,win_imageslide_handler);
            }
    }
        ret = PROC_LOOP;
        break;
    case VACT_ZOOM:
#ifdef IMG_2D_TO_3D
        if(decode_finish_flag && (IMAGE_SET_ON!=image_setting.image_3d_onoff))
#else
        if(decode_finish_flag)
#endif
        {
            if(IMG_UNPAUSE_STATE == imageslide_pause_state)
            {
                imageslide_pause_state = IMG_PAUSE_STATE;
                win_imageslide_showpause(TRUE);
                api_stop_timer(&imageslide_timer_id);
            }
            win_mpzoom_open();
            if(ANG_ORI == image_rot_ang)
            {
                imageslide_pause_state = IMG_UNPAUSE_STATE;
                slide_interval = (image_slide_setting.u_time_gap * 1000);
                win_imageslide_showpause(TRUE);
                imageslide_timer_id = api_start_timer("I",slide_interval,win_imageslide_handler);
            }
        }
        break;
    case VACT_MP:
#ifdef IMG_2D_TO_3D
        if(IMAGE_SET_ON==image_setting.image_3d_onoff)
            break;
#endif
        win = (POBJECT_HEAD)(&g_win_imageslide);
        osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

        while((POBJECT_HEAD)&g_win_usb_filelist != menu_stack_get_top())
        {
            menu_stack_pop();
        }

        image_abort();
        win_set_image_multiview_firstidx();
        mplayer_menu = (POBJECT_HEAD)(&g_win_imagepreview);

        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
             }
        break;
#ifdef IMG_2D_TO_3D
    case VACT_SWAP:
             win_imageslide_unknown_act_proc_swap(&ret);
        break;
    case VACT_INFO:
        if(decode_finish_flag && (image_rot_ang == ANG_ORI))
        {
            imageslide_pause_state = IMG_PAUSE_STATE;
            win_imageslide_showpause(TRUE);
            api_stop_timer(&imageslide_timer_id);
            if(osd_obj_open((POBJECT_HEAD)&win_imageset_con, 0) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) &win_imageset_con);
            }
          ret = PROC_LOOP;
        }
        break;
#endif
    default:
        break;
    }
        if(NULL != image_name)
        {
            free(image_name);
            image_name = NULL;
        }
    return ret;
}

static void win_imageslide_showexit(UINT8 flag)
{
#ifdef IMG_2D_TO_3D
    if(image_setting.image_osd_onoff && flag)
#else
    if(flag)
#endif
    {
           osd_draw_object((POBJECT_HEAD)&imageslide_exit,C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)&imageslide_exit, 0);
    }
    return;
}

static void win_imageslide_showrotate(UINT8 flag)
{
#ifdef IMG_2D_TO_3D
    if(image_setting.image_osd_onoff && flag)
#else
    if(flag)
#endif
    {
           osd_draw_object((POBJECT_HEAD)&imageslide_rotate,C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)&imageslide_rotate, 0);
    }

    return;
}

static void win_imageslide_showrotateicon(UINT8 flag)
{
#ifdef IMG_2D_TO_3D
    if(image_setting.image_osd_onoff && flag)
#else
    if(flag)
#endif
    {
           osd_draw_object((POBJECT_HEAD)&imageslide_rotateicon,C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)&imageslide_rotateicon, 0);
    }

    return;
}


static void win_imageslide_showidx(UINT8 flag)
{
    char str_buffer[20] = {0};
    TEXT_FIELD *p_txt = &imageslide_idx;
    unsigned int image_total_num = 0;

    get_play_list_info(cur_playlist_image,&image_total_num, NULL);
    snprintf(str_buffer,20, "%d/%u",cur_image_idx,image_total_num);
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buffer);

#ifdef IMG_2D_TO_3D
    if(image_setting.image_osd_onoff && flag)
#else
    if(flag)
#endif
        {
            osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
        }
    else
    {
        osd_clear_object((POBJECT_HEAD)p_txt, 0);
    }
    return;
}

void win_imageslide_show_user_para(UINT8 flag)
{
    #ifdef IMG_2D_TO_3D
    TEXT_FIELD *p_txt = &imageslide_user_para;
       UINT8 str_buffer[128] = {0};

    SYSTEM_DATA* p_sys_data = sys_data_get();
       if(image_setting.image_osd_onoff && flag)
       {
        if(IMAGE_SET_ON==image_setting.image_3d_onoff)
        {
            strncpy(str_buffer,"3D", 3);
        }
        else
        {
            strncpy(str_buffer,"2D", 3);
        }

        osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buffer);
        osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
    }
    else
        osd_clear_object((POBJECT_HEAD)p_txt, 0);
#endif
    return;
}

static void win_imageslide_showpause(UINT8 flag)
{
    UINT16 slide_interval = 0;

#ifdef IMG_2D_TO_3D
    if(image_setting.image_osd_onoff && flag)
#else
    if(flag)
#endif
    {
        slide_interval = (image_slide_setting.u_time_gap * 1000);
        if(0 != slide_interval)
        {
            if(IMG_PAUSE_STATE == imageslide_pause_state)
            {
                   osd_draw_object((POBJECT_HEAD)&imageslide_pause,C_UPDATE_ALL);
            }
            else
            {
                osd_clear_object((POBJECT_HEAD)&imageslide_pause, 0);
            }
        }
    }
    else
    {
        osd_clear_object((POBJECT_HEAD)&imageslide_pause, 0);
    }
    return;

}

void win_imageslide_osd_onoff(UINT8 onoff)
{
    if(onoff)
    {
        osd_set_attr(&imageslide_exit, C_ATTR_ACTIVE);
        osd_set_attr(&imageslide_rotate, C_ATTR_ACTIVE);
        osd_set_attr(&imageslide_idx, C_ATTR_ACTIVE);
        osd_set_attr(&imageslide_user_para, C_ATTR_ACTIVE);
        osd_set_attr(&imageslide_pause, C_ATTR_ACTIVE);
        osd_set_attr(&decode_bmp_progress, C_ATTR_ACTIVE);
    }
    else
    {
        osd_set_attr(&imageslide_exit, C_ATTR_HIDDEN);
        osd_set_attr(&imageslide_rotate, C_ATTR_HIDDEN);
        osd_set_attr(&imageslide_idx, C_ATTR_HIDDEN);
        osd_set_attr(&imageslide_user_para, C_ATTR_HIDDEN);
        osd_set_attr(&imageslide_pause, C_ATTR_HIDDEN);
        osd_set_attr(&decode_bmp_progress, C_ATTR_HIDDEN);
    }
    win_imageslide_showexit(onoff);
    win_imageslide_showrotate(onoff);
    win_imageslide_showidx(onoff);
    win_imageslide_showpause(onoff);
    win_imageslide_show_user_para(onoff);
    return;
}

static void win_imageslide_handler(void)
{
    if(imageslide_timer_id != OSAL_INVALID_ID)
    {
        api_stop_timer(&imageslide_timer_id);
        imageslide_timer_id = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_IMGTOOVER, 0, TRUE);
    }
}

static void win_imageslide_proc_handler(void)
{
    if(imageslide_proc_id != OSAL_INVALID_ID)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PROC_UPDATE, 0, TRUE);
       }
}


RET_CODE win_set_imageplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag)
{
    UINT8 back_saved = 0;
    RET_CODE retcode = RET_SUCCESS;
    media_file_info filenode;
    char *image_name = NULL;
    int playlist_idx = 0;
    file_list_handle t_cur_filelist = NULL;

    if(!pplaylist)
    {
        libc_printf("%s: pplaylist is null \n", __FUNCTION__);
        return RET_FAILURE;
    }
    MEMSET(&filenode, 0x0, sizeof(media_file_info));
    image_folderloop_flag = folder_flag;
    if(1 == image_folderloop_flag) // IMAGE_FOLDER
    {
        if(*pplaylist != NULL)
        {
            if(RET_SUCCESS != delete_play_list(*pplaylist))
            {
                return RET_FAILURE;
            }
        }
        #ifndef _USE_32M_MEM_
        *pplaylist = create_play_list(IMAGE_PLAY_LIST, "folder_image");
        #else
        *pplaylist = create_play_list(IMAGE_PLAY_LIST, " ");
        #endif
        if(NULL == *pplaylist)
        {
            return RET_FAILURE;
        }
            win_filelist_set_playlist(*pplaylist, PL_FOLDER_IMAGE);
            t_cur_filelist = win_filelist_get_cur_filelist();
         get_file_from_file_list(t_cur_filelist, idx, &filenode);
            image_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
        if(NULL == image_name)
        {
           libc_printf("image_name malloc fail\n");
           ASSERT(0);
           return RET_FAILURE;
        }
        MEMSET(image_name, 0, FULL_PATH_SIZE);
        win_get_fullname(image_name, FULL_PATH_SIZE, filenode.path, filenode.name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        t_cur_filelist = win_filelist_get_cur_filelist();
        retcode = add_all_file_list_items_to_play_list(*pplaylist, t_cur_filelist);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        if(retcode != RET_SUCCESS)
        {
               if(NULL != image_name)
                    {
                        free(image_name);
                        image_name = NULL;
                    }
            return RET_FAILURE;
        }
        if((F_JPG == filenode.filetype) || (F_BMP == filenode.filetype))
        {
            playlist_idx = get_play_list_index_by_name(*pplaylist, image_name);
        }
        else
        {
            playlist_idx = 1;
        }
        if(-1 == playlist_idx)
        {
               if(NULL != image_name)
                    {
                        free(image_name);
                        image_name = NULL;
                    }
            return RET_FAILURE;
        }
        cur_playlist_image = *pplaylist;
        cur_image_idx = playlist_idx;
    }
    else
    {
        cur_playlist_image = *pplaylist;
        cur_image_idx = idx;
    }
    set_play_list_info(cur_playlist_image, &cur_image_idx, NULL);

       if(NULL != image_name)
        {
            free(image_name);
            image_name = NULL;
        }
    return RET_SUCCESS;
}

void win_set_image_slide_idx(UINT32 idx)
{
    cur_image_idx = idx;
}

#endif

#ifdef _DISABLE_MEDIA_PLAYER_//link only
#include <api/libmp/gif.h>
#include <api/libmp/png.h>
#include <api/libmp/imgdec_common.h>
typedef struct
{
    BOOL renew; 
}t2decoder_control;

typedef struct _avdemuxer
{
    const char *name;
} AVDMX_PLUGIN;

typedef struct AVCodecParser {
    int codec_ids[5]; 
} AVCodecParser;

typedef struct
{
    BOOL preview;
}DEC_INIT_PAR, *PDEC_INIT_PAR;

typedef struct mp_playhdle_info_s
{
    int img_width;
    int img_height;
    int img_bpp;
    int img_fsize;
} mp_playhdle_info_t;

typedef BOOL (*paf_pe_function)(t2decoder_control *p_decoder_ctrl, int play_speed, DWORD search_time);
typedef paf_pe_function af_pe_function_table[9][9];
af_pe_function_table dct_avplayer ={0};

AVDMX_PLUGIN *av_demuxer = NULL;
AVCodecParser ff_aac_parser ={0};
AVCodecParser ff_aac_latm_parser={0};
AVCodecParser ff_ac3_parser={0};
AVCodecParser ff_flac_parser = {0};
AVCodecParser ff_mpegaudio_parser = {0};
AVCodecParser ff_pcmblueray_parser = {0};
AVCodecParser ff_dvdpcm_parser = {0};
INT32 clean_flag = 0;

DWORD avplayer_get_play_time(void)
{
    return 0;
}

INT32 avplayer_subtt_channel_change(INT32 idx)
{
    return 0;
}

INT32 avplayer_get_chapter_info(PDEC_CHAPTER_INFO p_chapter_info)
{
    return 0;
}

void av_register_codec_parser(AVCodecParser *parser)
{
    
}

int avplayer_rls_init_param(const struct pe_video_cfg *pe_video_cfg, DEC_INIT_PAR *p_init_param,
                             void *p_other_param)
{
    return TRUE;
}

int avplayer_get_init_param(const struct pe_video_cfg *pe_video_cfg,
        int media_type, DEC_INIT_PAR *p_init_param, void *p_other_param)
{
    return 0;
}

int avplayer_decoder_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2)
{
    return 0;
}

int avplayer_decoder_init(PDEC_INIT_PAR pdec_init_par)
{
    return 0;
}

DWORD avplayer_decoder_get_last_err_msg(void)
{
    return 0;
}

BOOL avplayer_decoder_decode(mp_callback_func video_cb)
{
    return TRUE;
}

BOOL avplayer_decoder_reset(UINT8 iPreview)
{
    return TRUE;
}

void avplayer_set_music_pause(int pause)
{
}

DWORD avplayer_get_duration(void)
{
    return 0;
}

BOOL imagedec_ioctl(imagedec_id id,UINT32 cmd, UINT32 para)
{
    return TRUE;
}

BOOL imagedec_rotate(imagedec_id id,enum IMAGE_ANGLE angle,UINT32 mode)
{
    return TRUE;
}

BOOL imagedec_display(imagedec_id id, p_image_display_par p_disinfo)
{
    return TRUE;
}

INT32 imagedec_check_fileheader(imagedec_id id, file_h fh)
{
    return 0;
}

BOOL imagedec_dis_next_pic(imagedec_id id,UINT32 mode,UINT32 time_out)
{
    return TRUE;
}

imagedec_id imagedec_init(p_imagedec_init_config pconfig)
{
    return 0;
}

BOOL imagedec_getinfo(imagedec_id id,file_h fh,p_image_info p_info)
{
    return TRUE;
}

BOOL imagedec_zoom(imagedec_id id,struct rect dis_rect,struct rect src_rect)
{
    return TRUE;
}

BOOL imagedec_swap(imagedec_id id, int show_type)
{
    return TRUE;
}

BOOL imagedec_3d_user_option(imagedec_id id, int output_format, int user_depth, int user_shift, int user_vip)
{
    return TRUE;
}

void imagedec_set_disp_param(imagedec_id id, int display_type, int output_format,
                              int user_depth, int user_shift, int user_vip)
{
}

int imagedec_opentv_display(imagedec_id id, p_image_display_par pdisinfo, int layer, struct rect *src_rect, struct rect *dst_rect)
{
}

int imagedec_opentv_get_decoded_buffer()
{
    return 0;
}

BOOL imagedec_set_mode(imagedec_id id, enum IMAGE_DIS_MODE mode,p_imagedec_mode_par p_par)
{
    return TRUE;
}

void imagedec_osd_init(p_imagedec_osd_config config)
{
}

BOOL imagedec_decode(imagedec_id id,file_h fh)
{
    return TRUE;
}

BOOL imagedec_stop(imagedec_id id)
{
    return TRUE;
}

BOOL imagedec_release(imagedec_id id)
{
    return TRUE;
}

GIF_RET gif_init(struct gif_cfg *init)
{
    return TRUE;
}

struct gif_instance *gif_open(void)
{
    return NULL;
}

GIF_RET gif_io_ctrl(struct gif_instance *gif,UINT32 io_cmd,UINT32 io_par)
{
    return TRUE;
}

GIF_RET gif_dec(struct  gif_instance *gif,struct gif_dec_par *par)
{
    return TRUE;
}

GIF_RET gif_stop(struct gif_instance *gif,gif_file file)
{
    return TRUE;
}

GIF_RET gif_close(struct gif_instance *gif)
{
    return TRUE;
}

int png_dec_init(png_dec_initconfig_t *cfg)
{
    return 0;
}

int png_dec_filename(char* filename, png_dec_dispconfig_t* dispconfig)
{
    return 0;
}

int png_dec_infoget(char* filename, mp_playhdle_info_t* play_info)
{
    return 0;
}

int png_dec_zoom(struct rect *dst_rect, struct rect *src_rect)
{
    return 0; 
}

int png_abort()
{
    return 0;
}

int png_cleanup()
{
    return 0;
}

void jhconvs_buffer_init(imagedec_id id)
{
}

void jhconvs_buffer_release(void)
{
}

int mp_dispoper_dedrv_readout_frm(int id, int de_idx, mp_disphdle_frmyuv_t* frmyuv)
{
    return 0;
}

int mp_maphdle_construct(maphdle_id *maphdle, mp_mapoper_type_t type)
{
    return 0;
}

int mp_maphdle_initconfig(maphdle_id maphdle, mp_mapoper_initconf_t* init_config)
{
    return 0;
}

int mp_maphdle_open(maphdle_id maphdle, memhdle_id memhdle)
{
    return 0;
}

int mp_disphdle_construct(disphdle_id *disphdl, mp_dispoper_type_t type)
{
    return 0;
}

int mp_disphdle_open(disphdle_id disphdl)
{
    return 0;
}

int mp_disphdle_bind(disphdle_id disphdl, maphdle_id mhdl)
{
    return 0;
}

int mp_disphdle_config(disphdle_id disphdl, mp_disphdle_config_t* config)
{
    return 0;
}

int mp_disphdle_run(disphdle_id disphdl)
{
    return 0;
}

int mp_maphdle_close(maphdle_id maphdle)
{
    return 0;
}

int mp_maphdle_destruct(maphdle_id maphdle)
{
    return 0;
}
#endif

