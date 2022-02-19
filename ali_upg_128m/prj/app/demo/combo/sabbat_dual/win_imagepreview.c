  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_imagepreview.c
*
*    Description: Mediaplayer image thumbnail preview UI.
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

#include "control.h"
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "win_tvsys.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_imagepreview.h"
#include "win_imageslide.h"
#include "media_control.h"
#include "win_filelist.h"

/*******************************************************************************
*Macro definition
*******************************************************************************/
#define MULTI_IMG_X_COUNT 3
#define MULTI_IMG_COUNT 9
#define PREVIEW_SH_IDX     WSTL_NOSHOW_IDX
#define TITLE_SH_IDX    WSTL_MIXBACK_IDX_01_8BIT//sharon WSTL_TEXT_28_8BIT//WSTL_BUTTON_02_FG//WSTL_TEXT_25
#define MBX_IMG_SH_IDX    WSTL_MP_PREV_01
#define MBX_IMG_HL_IDX    WSTL_MP_PREV_02
#define BMP_LOAD_SH_IDX    WSTL_NOSHOW_IDX//WSTL_TRANS_IX
#define TXT_DECODE_SH_IDX    WSTL_TEXT_07
#define W_L  0
#define W_T  0
#define W_W  OSD_MAX_WIDTH
#define W_H  OSD_MAX_HEIGHT
#define TITLE_L    W_L
#define TITLE_T    W_T
#define TITLE_W    W_W
#define TITLE_H    40
#define MBX_IMG_GAP        10
#define    MBX_IMG_L        TITLE_L
#define    MBX_IMG_T        (TITLE_T+TITLE_H)
#define    MBX_IMG_WIDTH    ((W_W-MBX_IMG_L*2+MBX_IMG_GAP)/MULTI_IMG_X_COUNT-MBX_IMG_GAP)
#define    MBX_IMG_HEIGHT    ((W_H-MBX_IMG_T+MBX_IMG_GAP)/(MULTI_IMG_COUNT/MULTI_IMG_X_COUNT)-MBX_IMG_GAP)
#define    MBX_IMG_W        MBX_IMG_WIDTH
#define    MBX_IMG_H        MBX_IMG_HEIGHT
#define    LOAD_W    60//42
#define    LOAD_H    60//42
#define LOAD_OFFSET_L    ((MBX_IMG_WIDTH-LOAD_W)/2)
#define LOAD_OFFSET_T    ((MBX_IMG_HEIGHT-LOAD_H)/2)
#define    LOAD_L    (MBX_IMG_L+LOAD_OFFSET_L)
#define    LOAD_T    (MBX_IMG_T+LOAD_OFFSET_T)
#define DEC_W    60
#define DEC_H    40
#define DEC_OFFSET_L    ((MBX_IMG_WIDTH-DEC_W)/2)
#define DEC_OFFSET_T    ((MBX_IMG_HEIGHT-DEC_H)/2)
#define DEC_L    (MBX_IMG_L+DEC_OFFSET_L)
#define DEC_T    (MBX_IMG_T+DEC_OFFSET_T)

/*******************************************************************************
*    Function decalaration
*******************************************************************************/
static VACTION preview_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT preview_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION preview_mbx_img_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT preview_mbx_img_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static void mbx_unkown_act_proc(VACTION act);
//static void preview_scale_screen();
static void preview_init(void);
static void preview_image(UINT8 mbx_idx, UINT16 image_idx);
static void show_loadinglogo(UINT8 firstpos);
static void show_title(UINT16 idx);

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PREVIEW_SH_IDX,PREVIEW_SH_IDX,PREVIEW_SH_IDX,PREVIEW_SH_IDX,   \
    preview_keymap,preview_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_MBX(root,varmb,nxt_obj,l,t,w,h,cnt,row,col,pos)        \
    DEF_MATRIXBOX(varmb,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, MBX_IMG_SH_IDX,MBX_IMG_HL_IDX,MBX_IMG_SH_IDX,MBX_IMG_SH_IDX,   \
    preview_mbx_img_keymap,preview_mbx_img_callback ,  \
    MATRIX_ROW_WRAP, MATRIX_TYPE_BITMAP, cnt,mb_node_image, row, col,\
    C_ALIGN_CENTER | C_ALIGN_VCENTER, MBX_IMG_GAP,MBX_IMG_GAP,0,0,0,0,0)

#define LDEF_BMP_LOAD(root,var_bmp,nxt_obj,l,t,w,h,bmp_id)        \
    DEF_BITMAP(var_bmp,&root,nxt_obj, \
    C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    0, 0, 0, 0, 0, \
    l, t, w, h, \
    BMP_LOAD_SH_IDX, BMP_LOAD_SH_IDX, BMP_LOAD_SH_IDX, BMP_LOAD_SH_IDX, \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, bmp_id)

/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
/*******************************************************************************
*Global variable definition
*******************************************************************************/
BOOL g_from_imagepreview = FALSE;
static MATRIX_BOX_NODE mb_node_image[MULTI_IMG_COUNT] ;
static UINT32 timer_preview = 0;
static UINT16 callback_cnt = 0;
static UINT16 image_multiview_first = 0;
static UINT16 image_multiview_total = 0;
static UINT16 hl_index = 0; //for title diaplay
static UINT8 image_idx_inpage = 0;
static UINT8 image_cnt_inpage = 0;

LDEF_TITLE(g_win_imagepreview,imgpreview_title,&imgpreview_mbx_img, \
    TITLE_L,TITLE_T,TITLE_W,TITLE_H,display_strs[0])

LDEF_MBX(g_win_imagepreview,imgpreview_mbx_img,NULL,\
    MBX_IMG_L,MBX_IMG_T,MBX_IMG_W,MBX_IMG_H,\
    MULTI_IMG_COUNT,MULTI_IMG_COUNT/MULTI_IMG_X_COUNT,MULTI_IMG_X_COUNT,0)

LDEF_BMP_LOAD(g_win_imagepreview,imgpreview_bmp_load,NULL,    \
    LOAD_L,LOAD_T,LOAD_W,LOAD_H,IM_LOADING)

LDEF_WIN(g_win_imagepreview,&imgpreview_title,W_L,W_T,W_W, W_H, 1)

DEF_TEXTFIELD(imgpreview_txt_decode,NULL,NULL,C_ATTR_ACTIVE,C_FONT_1, \
    0,0,0,0,0, \
    DEC_L,DEC_T,DEC_W,DEC_H, \
    TXT_DECODE_SH_IDX,TXT_DECODE_SH_IDX,TXT_DECODE_SH_IDX,TXT_DECODE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[1])

/*******************************************************************************
*    Functions definition
*******************************************************************************/

static VACTION preview_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
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

static PRESULT preview_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    char progress_strs[20] = {0};
    UINT8 i = 0;
    UINT16 curpos = 0;
    media_file_info file_node;
    char *image_name = NULL;
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA *sys_data=sys_data_get();
    UINT32 percent_max = 100;

       MEMSET(&file_node,0x0,sizeof(media_file_info));
    switch(event)
    {
    case EVN_PRE_OPEN:
        if(( TRUE== from_imagepreview) && ( TRUE == back_to_filelist))
        {
            from_imagepreview = FALSE;
            back_to_filelist = FALSE;
            return PROC_LEAVE;
        }
        if(TV_ASPECT_RATIO_169==sys_data_get_aspect_mode())
        {
            //MV_DEBUG("%s-->TvMode=%d\n",__FUNCTION__,SystemTvMode);
            vpo_aspect_mode( (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), TV_4_3, NORMAL_SCALE);
        }
        api_set_preview_vpo_color(TRUE);
        preview_init();
        //imagedec_ioctl(1,IMAGEDEC_IO_CMD_FILL_LOGO,0x108080);
        //imagedec_ioctl(1,IMAGEDEC_IO_CMD_FILL_LOGO,0xB9748F);//yvu
        imagedec_ioctl(1, IMAGEDEC_IO_CMD_FILL_LOGO, 0x356B9D);//yvu
        image_abort();
        preview_image(image_idx_inpage, image_multiview_first + image_idx_inpage);
        show_title(hl_index);
        break;
    case EVN_POST_OPEN:
        show_loadinglogo(1);//not show first loading icon
        break;
    case EVN_PRE_DRAW:
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_PRE_CLOSE:
        g_from_imagepreview = TRUE;
        if(TV_ASPECT_RATIO_169==sys_data_get_aspect_mode())
        {
            win_av_set_ratio(sys_data,3);
            sys_data_set_display_mode(&(sys_data->avset));
        }
        if(1 == image_folderloop_flag)//filelist
        {
            curpos = osd_get_mtrxbox_cur_pos(&imgpreview_mbx_img);
            get_file_from_play_list(cur_playlist_image, (image_multiview_first+curpos), &file_node);
            image_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
            if(NULL == image_name)
            {
                libc_printf("malloc fail for image_name in preview_callback\n");
                ASSERT(0);
                return ret;
            }
            MEMSET(image_name, 0, FULL_PATH_SIZE);
            win_get_fullname(image_name, FULL_PATH_SIZE, file_node.path, file_node.name);
            usblist_set_filelist_param(image_name);
        }
        break;
    case EVN_POST_CLOSE:
        image_abort();
        from_imagepreview = FALSE;
        vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE);
#ifdef DUAL_VIDEO_OUTPUT
        vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), FALSE);
#endif
        osd_clear_object((POBJECT_HEAD)&imgpreview_title,C_UPDATE_ALL);
        osd_clear_object((POBJECT_HEAD)&imgpreview_mbx_img,C_UPDATE_ALL);
        break;
    case EVN_MSG_GOT:
        if((CTRL_MSG_SUBTYPE_CMD_STO == param1) && (USB_STATUS_OVER == param2))
        {
            //storage_dev_mount_hint(1);
            file_list_check_storage_device(FALSE, FALSE);
        }

        //show dec percent
        else if ((CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS == param1)||(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1))
        {
        //    if(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1)
        //        SDBBP();
            if ((param2 < percent_max) && (CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS == param1))
            {
                callback_cnt++;
                snprintf(progress_strs,20,"%lu%%.",param2);
                for(i = 0;i < (callback_cnt%3);i++)
                {
                    snprintf(progress_strs,20,"%s%s",progress_strs,".");
                }
                osd_set_text_field_content(&imgpreview_txt_decode,STRING_ANSI,(UINT32)progress_strs);
                osd_draw_object((POBJECT_HEAD)&imgpreview_txt_decode,C_UPDATE_ALL);
            }
            else if((param2 >= percent_max) || (CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1))
            {
                osd_clear_object((POBJECT_HEAD)&imgpreview_txt_decode,C_UPDATE_ALL);
            //    OSD_ObjClose((POBJECT_HEAD)&imgpreview_txt_decode,C_UPDATE_ALL);
            }
        }

        if(CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER == param1)
        {
            if(param2 > timer_preview)
            {
                image_abort();
            }
            else
            {
                libc_printf("@@@@@@@@@@@@@@ es case\n");
                break;
            }
            image_idx_inpage++;
            if(image_idx_inpage < image_cnt_inpage)
            {
                preview_image(image_idx_inpage, image_multiview_first + image_idx_inpage);
            }
        }
        else if(CTRL_MSG_SUBTYPE_STATUS_MP3OVER == param1)
        {
            win_play_next_music_ex();
        }
        //else if(param1 == CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW)
        //{

        //}
        //else {}

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

static VACTION preview_mbx_img_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION    act = VACT_PASS;

    switch(key)
    {
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
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
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT preview_mbx_img_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    PMATRIX_BOX pmbx = (PMATRIX_BOX)p_obj;
    UINT16 curpos = 0;

    switch(event)
    {
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
            show_loadinglogo(image_idx_inpage+1);
            curpos = osd_get_mtrxbox_cur_pos(pmbx);
            show_title(image_multiview_first + curpos);
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            mbx_unkown_act_proc(unact);
            break;
        default:
            break;
    }

    return ret;
}

static void mbx_unkown_act_proc(VACTION act)
{
    POBJECT_HEAD win = NULL;
    PMATRIX_BOX pmbx = &imgpreview_mbx_img;
    UINT16 curpos = 0;

    switch(act)
    {
        case VACT_CURSOR_PGUP:
        case VACT_CURSOR_PGDN:
            if(VACT_CURSOR_PGUP == act)
            {
                if(image_multiview_first <=1 )
                {
                    break;
                }
                else if(image_multiview_first > MULTI_IMG_COUNT)
                {
                    image_multiview_first -= MULTI_IMG_COUNT;
                    image_cnt_inpage = MULTI_IMG_COUNT;
                }
                else
                {
                    image_multiview_first = 1;
                    image_cnt_inpage = (image_multiview_total<MULTI_IMG_COUNT)?image_multiview_total:(MULTI_IMG_COUNT);
                }
            }
            else //(act == VACT_CURSOR_PGDN)
            {
                if(image_multiview_first <= image_multiview_total - MULTI_IMG_COUNT)
                {
                    image_multiview_first += MULTI_IMG_COUNT;
                    image_cnt_inpage = ((image_multiview_total-image_multiview_first+1) < MULTI_IMG_COUNT)? \
                                    (image_multiview_total-image_multiview_first+1):(MULTI_IMG_COUNT);
                }
                else
                {
                    break;
                           }
            }

//            OSD_ClearObject((POBJECT_HEAD)&g_win_imagepreview,0);
            osd_set_mtrxbox_count(pmbx, image_cnt_inpage, MULTI_IMG_COUNT/MULTI_IMG_X_COUNT, MULTI_IMG_X_COUNT);
            curpos = osd_get_mtrxbox_cur_pos(pmbx);
            if(curpos > image_cnt_inpage-1)
            {
                curpos = image_cnt_inpage-1;
                osd_set_mtrxbox_cur_pos(pmbx, curpos);
            }
            osd_track_object((POBJECT_HEAD)pmbx,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT);
            hl_index = image_multiview_first + curpos;

            show_loadinglogo(1);
            //imagedec_ioctl(1,IMAGEDEC_IO_CMD_FILL_LOGO,0xB9748F);//yvu
                       imagedec_ioctl(1, IMAGEDEC_IO_CMD_FILL_LOGO, 0x356B9D);//yvu
            image_abort();
                    timer_preview = osal_get_tick();
            image_idx_inpage = 0;
            preview_image(image_idx_inpage, image_multiview_first + image_idx_inpage);
            show_title(hl_index);
            break;
        case VACT_ENTER:
            win = (POBJECT_HEAD)(&g_win_imagepreview);
            osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

            image_abort();
            from_imagepreview = TRUE;
            curpos = osd_get_mtrxbox_cur_pos(pmbx);
            win_set_image_slide_idx(image_multiview_first+curpos);
            win = (POBJECT_HEAD)(&g_win_imageslide);
            if(osd_obj_open(win,(UINT32)(~0)) != PROC_LEAVE)
                    {
                menu_stack_push(win);
                    }
            break;
        default:
            break;
    }
}

static void preview_init(void)
{
    UINT8 i = 0;
    UINT16 curpos = 0;
    unsigned int file_num = 0;

    //alloc bmp
    for(i = 0; i < MULTI_IMG_COUNT; i++)
    {
        mb_node_image[i].str = 0;
        mb_node_image[i].num = 0;//IM_LOADING;
        mb_node_image[i].b_attr = C_ATTR_ACTIVE;
    }
    //init parametre
    get_play_list_info(cur_playlist_image,&file_num, NULL);
    image_multiview_total = file_num;
    image_idx_inpage = 0;
    if((image_multiview_total - image_multiview_first + 1) >= MULTI_IMG_COUNT)
    {
        curpos = 0;
        image_cnt_inpage = MULTI_IMG_COUNT;
    }
    else if(image_multiview_total <= MULTI_IMG_COUNT)
    {
        curpos = image_multiview_first-1;
        image_multiview_first = 1;
        image_cnt_inpage = image_multiview_total;
    }
    else
    {
        curpos = MULTI_IMG_COUNT - (image_multiview_total-image_multiview_first)-1;
        image_multiview_first = image_multiview_total - MULTI_IMG_COUNT + 1;
        image_cnt_inpage = MULTI_IMG_COUNT;
    }
    hl_index = image_multiview_first + curpos;
    osd_set_mtrxbox_count(&imgpreview_mbx_img, image_cnt_inpage, MULTI_IMG_COUNT/MULTI_IMG_X_COUNT, MULTI_IMG_X_COUNT);
    osd_set_mtrxbox_cur_pos(&imgpreview_mbx_img, curpos);

    vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RELEASE_VFB, 0);
}

static void preview_image(UINT8 mbx_idx, UINT16 image_idx)
{
    UINT16 col = 0;
    UINT16 row = 0;
    UINT8 tv_mode = 0;
    RECT rect_info;
    UINT16 left = 0;
    UINT16 top = 0;
    media_file_info file_node;
    char *full_name = NULL;
    char *ext = NULL;
    UINT16 img_top = 0;
    UINT16 img_height = 0;
    UINT16 img_gap_w = 0;
    UINT16 img_gap_h = 0;
    UINT16 img_left = 0;
    UINT16 img_width = 0;
    UINT32 cmp_len_four = 4;
    struct osdrect osd_rect ;

    MEMSET(&rect_info, 0x0, sizeof(RECT));
    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    MEMSET(&osd_rect, 0x0, sizeof(osd_rect));
    full_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
    if(NULL == full_name)
    {
        libc_printf("malloc fail in preview_image 4 full_name\n");
        ASSERT(0);
        return;
    }
    MEMSET(full_name, 0, FULL_PATH_SIZE);
    osd_get_rect_on_screen(&osd_rect);

#ifndef SD_UI
    img_left = (MBX_IMG_L + osd_rect.u_left) * 720/1280;
    img_width = (MBX_IMG_WIDTH)*720/1280;//+2;
    img_gap_w = MBX_IMG_GAP*720/1280;
    img_gap_h = MBX_IMG_GAP*576/720;
    osd_get_mtrx_box_cell_location(&imgpreview_mbx_img, mbx_idx, &col, &row);
    left =  MBX_IMG_L + (MBX_IMG_WIDTH+MBX_IMG_GAP)*col;
    top = MBX_IMG_T + (MBX_IMG_HEIGHT+MBX_IMG_GAP)*row;
    osd_move_object((POBJECT_HEAD)&imgpreview_bmp_load,left+LOAD_OFFSET_L,top+LOAD_OFFSET_T,FALSE);
    osd_clear_object((POBJECT_HEAD)&imgpreview_bmp_load,C_UPDATE_ALL);
    osd_move_object((POBJECT_HEAD)&imgpreview_txt_decode,left+DEC_OFFSET_L,top+DEC_OFFSET_T,FALSE);
    tv_mode = api_video_get_tvout();
    if((TV_MODE_PAL == tv_mode) || (TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode))
    {
    //    rect_info.top = MBX_IMG_T + OSD_STARTROW_P + (MBX_IMG_HEIGHT+MBX_IMG_GAP)*Row +4;
    //    rect_info.bottom = rect_info.top + MBX_IMG_HEIGHT-8;

          img_top = (MBX_IMG_T + osd_rect.u_top)*576/720;
          img_height = MBX_IMG_H*576/720;
          rect_info.top = img_top + (img_height+img_gap_h)*row + 4;
          rect_info.bottom = rect_info.top + img_height - 8;
    }
    else
    {
    //    rect_info.top = (MBX_IMG_T + OSD_STARTROW_N + (MBX_IMG_HEIGHT+MBX_IMG_GAP)*Row)*576/480+4;
    //    rect_info.bottom = rect_info.top + MBX_IMG_HEIGHT*576/480-8;

          img_top = (MBX_IMG_T + osd_rect.u_top)*480/720;
          img_height = MBX_IMG_H*480/720;
          rect_info.top = (img_top + (img_height + img_gap_h) * row) * 576/480 + 4;
          rect_info.bottom = rect_info.top + img_height * 576/480 - 8;
    }
//    rect_info.left =  OSD_STARTCOL + MBX_IMG_L + (MBX_IMG_WIDTH+MBX_IMG_GAP)*Col+4;
//    rect_info.right = rect_info.left + MBX_IMG_WIDTH-8;
    rect_info.left = img_left + (img_width + img_gap_w) * col + 4;
    rect_info.right = rect_info.left + img_width - 8;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (osd_get_mirror_flag() == TRUE)
    {
        rect_info.left = 720 - rect_info.right -4;
             rect_info.right = rect_info.left + img_width - 8;
    }
#endif

#else
    img_left = (MBX_IMG_L + osd_rect.u_left) ;
    img_width = (MBX_IMG_WIDTH);//+2;
    img_gap_w = MBX_IMG_GAP;
    img_gap_h = MBX_IMG_GAP;
    osd_get_mtrx_box_cell_location(&imgpreview_mbx_img, mbx_idx, &col, &row);
    left =  MBX_IMG_L + (MBX_IMG_WIDTH+MBX_IMG_GAP)*col;
    top = MBX_IMG_T + (MBX_IMG_HEIGHT+MBX_IMG_GAP)*row;
    osd_move_object((POBJECT_HEAD)&imgpreview_bmp_load,left+LOAD_OFFSET_L,top+LOAD_OFFSET_T,FALSE);
    osd_clear_object((POBJECT_HEAD)&imgpreview_bmp_load,C_UPDATE_ALL);
    osd_move_object((POBJECT_HEAD)&imgpreview_txt_decode,left+DEC_OFFSET_L,top+DEC_OFFSET_T,FALSE);
    tv_mode = api_video_get_tvout();
    if((TV_MODE_PAL == tv_mode) || (TV_MODE_576P == tv_mode))
    {
          img_top = (MBX_IMG_T + osd_rect.u_top);
          img_height = MBX_IMG_H;
          rect_info.top = img_top + (img_height+img_gap_h)*row + 4;
          rect_info.bottom = rect_info.top + img_height - 8;
    }
    else
    {
          img_top = (MBX_IMG_T + osd_rect.u_top)*480/576;
          img_height = MBX_IMG_H*480/576;
          rect_info.top = (img_top + (img_height + img_gap_h) * row) * 576/480 + 4;
          rect_info.bottom = rect_info.top + img_height * 576/480 - 8;
    }
    rect_info.left = img_left + (img_width + img_gap_w) * col + 4;
    rect_info.right = rect_info.left + img_width - 8;
#endif
    get_file_from_play_list(cur_playlist_image,image_idx,&file_node);
    win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
    ext = strrchr(file_node.name, '.');
    cmp_len_four = 4;
    if((ext) /*&& (strncasecmp(ext, ".png", cmp_len_four))*/ && (strncasecmp(ext, ".gif", cmp_len_four)))
    {
    win_image_file_preview(full_name, rect_info, IMAGEDEC_MULTI_PIC);
    }
//    api_set_vpo_dit(TRUE);
    if(NULL != full_name)
    {
        free(full_name);
        full_name = NULL;
    }
}

static void show_loadinglogo(UINT8 firstpos)
{
    UINT8 i = 0;
    UINT16 col = 0;
    UINT16 row = 0;
    UINT16 top = 0;
    UINT16 left = 0;
    PBITMAP pbitmap = NULL;

    pbitmap = &imgpreview_bmp_load;
    for(i=firstpos; i<MULTI_IMG_COUNT; i++)
    {
        osd_get_mtrx_box_cell_location(&imgpreview_mbx_img, i, &col, &row);
        left =  MBX_IMG_L + (MBX_IMG_WIDTH+MBX_IMG_GAP)*col;
        top = MBX_IMG_T + (MBX_IMG_HEIGHT+MBX_IMG_GAP)*row;

        osd_move_object((POBJECT_HEAD)pbitmap,left+LOAD_OFFSET_L,top+LOAD_OFFSET_T,FALSE);
        if (i < image_cnt_inpage)
        {
            osd_draw_object((POBJECT_HEAD)pbitmap,C_UPDATE_ALL);
        }
        else
        {
            osd_clear_object((POBJECT_HEAD)pbitmap,C_UPDATE_ALL);
        }
    }
}

static void show_title(UINT16 idx)
{
    char *str_name = NULL;
    char *str_buff = NULL;
    char *full_name = NULL;
    TEXT_FIELD *p_txt = NULL;
    media_file_info file_node;
    __MAYBE_UNUSED__ int t_len = 0;

    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif

       MEMSET(&file_node, 0x0, sizeof(media_file_info));
       str_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == str_name)
        {
            libc_printf("malloc fail in show_title 4 str_name\n");
            ASSERT(0);
            return;
        }
       str_buff = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == str_buff)
        {
            free(str_name);
            str_name = NULL;
            libc_printf("malloc fail in show_title 4 str_buff\n");
            ASSERT(0);
            return;
        }
       full_name = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
       if(NULL == full_name)
        {
            free(str_name);
            str_name = NULL;
            free(str_buff);
            str_buff = NULL;
            libc_printf("malloc fail in show_title 4 full_name\n");
            ASSERT(0);
            return;
        }
       MEMSET(str_name,0,FULL_PATH_SIZE);
       MEMSET(str_buff, 0, FULL_PATH_SIZE);
       MEMSET(full_name, 0, FULL_PATH_SIZE);
    if(1 == image_folderloop_flag)//filelist
    {
        strncpy(str_buff, "Disk ", FULL_PATH_SIZE-1);
        get_file_from_play_list(cur_playlist_image, idx, &file_node);
        win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
    #ifdef PERSIAN_SUPPORT
        filelist_utf8_to_unicode(full_name, str_uni);
        com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
    #else
        win_get_display_dirname(str_name, FULL_PATH_SIZE, full_name);
        //strcat(str_buff, str_name);
        t_len = strlen(str_buff);
        if ((FULL_PATH_SIZE-t_len) > t_len)
        {
            strncat(str_buff, str_name, FULL_PATH_SIZE - t_len - 1);
        }
    #endif
    }
    else//playlist
    {
    #ifndef _USE_32M_MEM_
        strncpy(str_buff, "Image Play List/", FULL_PATH_SIZE-1);
    #else
        strncpy(str_buff, " ", FULL_PATH_SIZE-1);
    #endif
        get_file_from_play_list(cur_playlist_image,idx,&file_node);
    #ifdef PERSIAN_SUPPORT
        filelist_utf8_to_unicode(file_node.name, str_uni);
        com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
    #else
        //filter_unkownfont_name(file_node.name,str_name);
        win_shorten_filename(file_node.name,str_name, FULL_PATH_SIZE);
        //strcat(str_buff, str_name);
        t_len = strlen(str_buff);
        if ((FULL_PATH_SIZE-t_len) > strlen(str_name))
        {
            strncat(str_buff, str_name, FULL_PATH_SIZE-t_len - 1);
        }
    #endif
    }

    p_txt = &imgpreview_title;
    osd_clear_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
    #ifdef PERSIAN_SUPPORT
    osd_set_text_field_content(p_txt,STRING_UNICODE,(UINT32)str);
    #else
    osd_set_text_field_content(p_txt,STRING_UTF8,(UINT32)str_buff);
    #endif
    osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
    if(NULL != str_name)
    {
        free(str_name);
        str_name = NULL;
    }
    if(NULL != str_buff)
    {
        free(str_buff);
        str_buff = NULL;
    }
    if(NULL != full_name)
    {
        free(full_name);
        full_name = NULL;
    }
}

void win_set_image_multiview_firstidx(void)
{
    image_multiview_first = cur_image_idx;
}

#endif

