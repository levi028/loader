/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_display.c
*
*    Description:   Some functions of the filelist display
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#include <api/libfs2/statvfs.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>
#include <hld/decv/decv.h>

#include "images.id"
#include "copper_common/com_api.h"

#include "win_filelist.h"
#include "win_filelist_inner.h"
#include "win_filelist_display.h"
#include "media_control.h"
#include "win_mute.h"
#ifdef MP_SPECTRUM
#include "win_mpspectrum.h"
#endif


////////////////////////////////////
#define MPLAYER_ITEM_CNT 6

static TEXT_FIELD *p_txt_ids[] =
{
    &usb_item_idx1,
    &usb_item_idx2,
    &usb_item_idx3,
    &usb_item_idx4,
    &usb_item_idx5,
    &usb_item_idx6,
    &usb_item_idx7,
    //&usb_item_idx8,
};

static TEXT_FIELD *p_txt_names[] =
{
    &usb_item_name1,
    &usb_item_name2,
    &usb_item_name3,
    &usb_item_name4,
    &usb_item_name5,
    &usb_item_name6,
    &usb_item_name7,
    //&usb_item_name8,
};

static BITMAP *p_bmp_types[] =
{
    &usb_item_bmp1,
    &usb_item_bmp2,
    &usb_item_bmp3,
    &usb_item_bmp4,
    &usb_item_bmp5,
    &usb_item_bmp6,
    &usb_item_bmp7,
   //&usb_item_bmp8,
};

static BITMAP *p_bmp_favs[] =
{
    &usb_item_fav1,
    &usb_item_fav2,
    &usb_item_fav3,
    &usb_item_fav4,
    &usb_item_fav5,
    &usb_item_fav6,
    &usb_item_fav7,
   // &usb_item_fav8,
};

static BITMAP *p_bmp_dels[] =
{
    &usb_item_del1,
    &usb_item_del2,
    &usb_item_del3,
    &usb_item_del4,
    &usb_item_del5,
    &usb_item_del6,
    &usb_item_del7,
    //&usb_item_del8,
};

static UINT16 preview_icon_ids[] =
{
    IM_MP3_BMP,
    IM_MP3_PHOTO,
    IM_MP3_VIDEO,
    IM_MP3_BMP,
};

static UINT16 filelist_loop_icons[] =
{
    IM_MP3_ICON_REPEAT_FOLDER,
    IM_MP3_ICON_RADOM_FOLDER,
    IM_MP3_ICON_REPEAT_1,
};

static UINT16 playlist_loop_icons[] =
{
    IM_MP3_ICON_REPEAT_LIST,
    IM_MP3_ICON_RADOM_LIST,
    IM_MP3_ICON_REPEAT_1,
};

static BITMAP *mplayer_items[] =
{
    &mplayer_prv,
    &mplayer_xback,
    &mplayer_pause,
    &mplayer_stop,
    &mplayer_xforward,
    &mplayer_nxt,
};

static void display_root_item(UINT16 utop, UINT8 root_offset, char *filename, int fname_size,
    char *str_txt, int str_buf_size)
{
    UINT8 i = 0;
    media_file_info file_node;
    RET_CODE ret = RET_SUCCESS;
    TEXT_FIELD *ptxt = NULL;
    BITMAP *pbmp = NULL;
    struct statvfs usb_info;
    file_list_handle t_cur_filelist = NULL;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;
#ifdef BIDIRECTIONAL_OSD_STYLE
    int t_size = 0;
#endif
    int t_len = 0;
#ifdef PERSIAN_SUPPORT
   UINT16 str[64] = {0};
   char str_uni[128] = {0};
#endif
    char *t_cur_device = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    MEMSET(&usb_info, 0x0, sizeof(struct statvfs));
    //TODO:Draw memtype icon & volume name
    for(i = 0; i < FL_ITEM_CNT; i++)
    {
        t_cur_filelist = win_filelist_get_cur_filelist();
        ret = get_file_from_file_list(t_cur_filelist, (utop+i + 1 - root_offset), &file_node);
        if(RET_SUCCESS == ret)
        {
            ptxt = p_txt_ids[i];
            snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, "%d.",(utop + i + 1 - root_offset));
        #ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == osd_get_mirror_flag())
            {
                snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, ".%d",(utop + i + 1 - root_offset));
            }
        #endif
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
            ptxt = p_txt_names[i];
            win_get_fullname(filename, fname_size, file_node.path, file_node.name);
            //fs_statvfs(filename, &usb_info);
            if(fs_statvfs(filename, &usb_info) < 0)
            {
                PRINTF("s_statvfs() failed!\n");
            }
            strncpy(filename, file_node.name, fname_size-1);
            filename[fname_size-1] = 0;
        #ifdef BIDIRECTIONAL_OSD_STYLE
            if (osd_get_mirror_flag() == TRUE)
            {
                t_size = 128;
                t_len = strlen(usb_info.f_volume_name);
                if ((t_size - t_len) > strlen(" :"))
                {
                    strncat(usb_info.f_volume_name," :", 2);
                }
                t_len += 2;
                if ((t_size - t_len) > strlen(filename))
                {
                    strncat(usb_info.f_volume_name, filename, 128 - 1 - strlen(usb_info.f_volume_name));
                }
                strncpy(filename, usb_info.f_volume_name, fname_size-1);
                filename[fname_size-1] = 0;
            }
            else
            {
                t_len = strlen(filename);
                if ((fname_size - t_len) > strlen(": "))
                {
                    strncat(filename, ": ", 2);
                }
                t_len += 2;
                if ((fname_size - t_len) > strlen(usb_info.f_volume_name))
                {
                    strncat(filename, usb_info.f_volume_name, fname_size - t_len - 1);
                }
            }
        #else
            t_len = strlen(filename);
            if ((fname_size - t_len) > (int)strlen(": "))
            {
                strncat(filename, ": ", 2);
            }
            t_len += 2;
            if ((fname_size - t_len) > (int)strlen(usb_info.f_volume_name))
            {
                strncat(filename, usb_info.f_volume_name, fname_size - t_len - 1);
            }
        #endif
        #ifdef PERSIAN_SUPPORT
            filelist_utf8_to_unicode(filename, str_uni);
            com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
            osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
        #else
            //filter_unkownfont_name(filename,str_txt);//filter name that can not be show
            win_shorten_filename(filename,str_txt, str_buf_size);
            win_fill_filename(str_txt,str_txt);//Fill the file name with SPACE, to avoid display BUG.
            snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, "%s",str_txt);
            osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)(str_txt));
        #endif
            pbmp = p_bmp_types[i];
            t_type = usblst_get_mp_filelist_type();
            t_cur_device = usblst_get_cur_device(t_type);
            win_set_device_icon(pbmp, t_cur_device);
            pbmp = p_bmp_favs[i];
            osd_set_attr(pbmp, C_ATTR_HIDDEN);
            pbmp = p_bmp_dels[i];
            osd_set_attr(pbmp, C_ATTR_HIDDEN);
        }
        else//when failure,clear all items
        {
            ptxt = p_txt_ids[i];
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
            ptxt = p_txt_names[i];
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
            pbmp = p_bmp_types[i];
            osd_set_bitmap_content(pbmp, 0);
            pbmp = p_bmp_favs[i];
            osd_set_attr(pbmp, C_ATTR_HIDDEN);
            pbmp = p_bmp_dels[i];
            osd_set_attr(pbmp, C_ATTR_HIDDEN);
        }
    }
}

void display_filelist_items(UINT16 utop)
{
    UINT8 i = 0;
    UINT8 root_offset = 0;
    UINT8 broot = 0;
    RET_CODE ret = 0;
    char *str_txt = NULL;//[MAX_FILE_NAME_SIZE + 1] = {0};
    media_file_info file_node;
    TEXT_FIELD *ptxt = NULL;
    BITMAP *pbmp = NULL;
    char *filename = NULL;//[MAX_FILE_NAME_SIZE] = {0};
    play_list_handle play_list = NULL;
    file_list_handle t_cur_filelist = NULL;
    play_list_handle t_play_list = NULL;
    char *t_name = NULL;
    const char *t_name_music = usblst_get_list_name_music();
    const char *t_name_image = usblst_get_list_name_image();
    UINT32 *t_del_flidx = NULL;

    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    str_txt = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == str_txt)
    {
        ASSERT(0);
        return;
    }
    filename = MALLOC(MAX_FILE_NAME_SIZE);
    if (NULL == filename)
    {
        FREE(str_txt);
        ASSERT(0);
        return;
    }
    MEMSET(str_txt, 0x0, MAX_FILE_NAME_SIZE + 1);
    MEMSET(filename, 0x0, MAX_FILE_NAME_SIZE);

    //check all files in playlist,NOTE: Pls don't remark follow code.
    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
        if (NULL == win_filelist_get_playlist(PL_MUSIC))//(win_play_list[PL_MUSIC] == NULL)
        {
            t_play_list = create_play_list(MUSIC_PLAY_LIST, t_name_music);
            win_filelist_set_playlist(t_play_list, PL_MUSIC);
            if (NULL == win_filelist_get_playlist(PL_MUSIC))//(win_play_list[PL_MUSIC] == NULL)
            {
                FREE(str_txt);
                str_txt = NULL;
                FREE(filename);
                filename = NULL;
                return;
            }
        }
        play_list = win_filelist_get_playlist(PL_MUSIC);//win_play_list[PL_MUSIC];
    }
    else if(TITLE_IMAGE == win_filelist_get_mp_title_type())
    {
        if (NULL == win_filelist_get_playlist(PL_IMAGE))//(win_play_list[PL_IMAGE] == NULL)
        {
            t_play_list = create_play_list(IMAGE_PLAY_LIST, t_name_image);
            win_filelist_set_playlist(t_play_list, PL_IMAGE);
            if (NULL == win_filelist_get_playlist(PL_IMAGE))//(win_play_list[PL_IMAGE] == NULL)
            {
                FREE(str_txt);
                str_txt = NULL;
                FREE(filename);
                filename = NULL;
                return;
            }
        }
        play_list = win_filelist_get_playlist(PL_IMAGE);//win_play_list[PL_IMAGE];
    }
    t_cur_filelist = win_filelist_get_cur_filelist();
    broot = win_check_openroot(t_cur_filelist);
    if(FALSE == broot)
    {
        root_offset = 1;//if not root dir,add an item to exit
    }
    else
    {
        root_offset = 0;
    }

    //reset object list select flag
    osd_set_obj_list_single_select(&usblst_olist, 0xFFFF);
    if(TRUE == broot)
    {
        display_root_item(utop, root_offset, filename, MAX_FILE_NAME_SIZE, str_txt, MAX_FILE_NAME_SIZE+1);
    }
    else
    {
        //TODO:Draw filetype icon & extended filename according to file_node.filetype
        for(i = 0; i < FL_ITEM_CNT; i++)
        {
            if((0 == (utop+i)) && (1 == root_offset))
            {
                ptxt = p_txt_ids[i];
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)("/.."));
                ptxt = p_txt_names[i];
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
                pbmp = p_bmp_types[i];
                osd_set_bitmap_content(pbmp, 0);
                pbmp = p_bmp_favs[i];
                osd_set_attr(pbmp, C_ATTR_HIDDEN);
                pbmp = p_bmp_dels[i];
                osd_set_attr(pbmp, C_ATTR_HIDDEN);
                continue;
            }//item exit

            t_cur_filelist = win_filelist_get_cur_filelist();
            #ifdef SD_UI
            ret = get_file_from_file_list2(t_cur_filelist, (utop+i + 1 - root_offset), &file_node);
            #else
            ret = get_file_from_file_list(t_cur_filelist, (utop+i + 1 - root_offset), &file_node);
            #endif
            if(RET_SUCCESS == ret)
            {
                ptxt = p_txt_ids[i];
                snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, "%d.",(utop + i + 1 - root_offset));
                #ifdef BIDIRECTIONAL_OSD_STYLE
                if (osd_get_mirror_flag() == TRUE)
                {
                   snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, ".%d",(utop + i + 1 - root_offset));
                }
                #endif
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                ptxt = p_txt_names[i];
            #ifdef PERSIAN_SUPPORT
                filelist_utf8_to_unicode(file_node.name, str_uni);
                com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
                osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
            #else
                //filter_unkownfont_name(file_node.name,str_txt);//filter name that can not be show
                win_shorten_filename(file_node.name,str_txt, MAX_FILE_NAME_SIZE+1);
                snprintf(str_txt, MAX_FILE_NAME_SIZE + 1, "%s",str_txt);
                osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)(str_txt));
            #endif
                pbmp = p_bmp_types[i];
                osd_set_bitmap_content(pbmp, file_icon_ids[file_node.filetype]);
                pbmp = p_bmp_favs[i];
                win_get_fullname(filename, MAX_FILE_NAME_SIZE, file_node.path, file_node.name);
                if(get_play_list_index_by_name(play_list,filename) >= 0  && file_node.filetype != F_DIR)
                {
                    osd_set_attr(pbmp, C_ATTR_ACTIVE);
                }
                else
                {
                    osd_set_attr(pbmp, C_ATTR_HIDDEN);
                }
                pbmp = p_bmp_dels[i];
                t_del_flidx = usblst_get_del_filelist_idx();
                if(win_check_del_flag(t_del_flidx, utop+i - root_offset))
                {
                    osd_set_attr(pbmp, C_ATTR_ACTIVE);
                }
                else
                {
                    osd_set_attr(pbmp, C_ATTR_HIDDEN);
                }

                win_get_fullname(filename, MAX_FILE_NAME_SIZE, file_node.path, file_node.name);
                t_name = usblst_get_mp_curmusic_name();
                if(!STRCMP(t_name, filename))
                {
                    osd_set_obj_list_single_select(&usblst_olist, utop + i);
                }
            }
            else//when failure,clear all items
            {
                ptxt = p_txt_ids[i];
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
                ptxt = p_txt_names[i];
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
                pbmp = p_bmp_types[i];
                osd_set_bitmap_content(pbmp, 0);
                pbmp = p_bmp_favs[i];
                osd_set_attr(pbmp, C_ATTR_HIDDEN);  
                pbmp = p_bmp_dels[i];
                osd_set_attr(pbmp, C_ATTR_HIDDEN);
            }
        }
    }
    if (NULL != str_txt)
    {
        FREE(str_txt);
        str_txt = NULL;
    }
    if (NULL != filename)
    {
        FREE(filename);
        filename = NULL;
    }
}

void display_current_music(void)
{
    UINT16 utop = 0;
    POBJLIST ol = &usblst_olist;
    POBJECT_HEAD pold_selitem = NULL;
    POBJECT_HEAD pnew_selitem = NULL;
    UINT8 ucon_focusid = 0;
    UINT16 bold_sel = 0;

	if(0 == ucon_focusid)
	{
		;
	}
	utop = osd_get_obj_list_top(ol);
    ucon_focusid = osd_get_container_focus((PCONTAINER)&g_win_usb_filelist);

    bold_sel = ol->w_select;

    display_filelist_items(utop);

    if((bold_sel >= ol->w_top)
        &&(bold_sel < (ol->w_top + ol->w_dep))
        &&(bold_sel != ol->w_cur_point))
    {
        if(ol->b_list_style & LIST_ITEMS_COMPLETE)
        {
            pold_selitem = ol->p_list_field[bold_sel];
        }
        else
        {
            pold_selitem = ol->p_list_field[bold_sel - ol->w_top];
        }
        osd_draw_object(pold_selitem, C_UPDATE_ALL);
    }//draw old select normal

    if((ol->w_select >= ol->w_top)
        &&(ol->w_select < (ol->w_top + ol->w_dep))
        &&(ol->w_select != ol->w_cur_point))
    {
        if(ol->b_list_style & LIST_ITEMS_COMPLETE)
        {
            pnew_selitem = ol->p_list_field[ol->w_select];
        }
        else
        {
            pnew_selitem = ol->p_list_field[ol->w_select - ol->w_top];
        }

        osd_sel_object(pnew_selitem, C_UPDATE_ALL);
    }//draw new select
}

static struct image_info cur_imageinfo;

static void display_image_preview(char *full_name, int fname_size, p_file_info pfile_node)
{
    char *ext = NULL;
    RECT rect_info;
    UINT8 tv_mode = 0;
    struct osdrect osd_rect;
#ifndef _USE_32M_MEM_
    //const UINT8 file_ext_name_len = 4;
#endif
    TITLE_TYPE m_title_type = TITLE_NUMBER;

	if(NULL == ext)
	{
		;
	}
	MEMSET(&osd_rect, 0x0, sizeof(struct osdrect));
    if((F_JPG == pfile_node->filetype) || (F_BMP == pfile_node->filetype))
    {
        win_get_fullname(full_name, fname_size, pfile_node->path, pfile_node->name);
        win_get_fileinfo(full_name, (UINT32)&cur_imageinfo);
        ext = strrchr(pfile_node->name, '.');

        osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);

        tv_mode = api_video_get_tvout();
        osd_get_rect_on_screen(&osd_rect);
#ifndef SD_UI
        if((TV_MODE_PAL == tv_mode) || (TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode))
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top)*576/720;
            rect_info.bottom = rect_info.top + FL_INFO_BMP_H*576/720;
        }
        else
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top)*576/720;
            rect_info.bottom = rect_info.top + (FL_INFO_BMP_H*576/720);
        }
        rect_info.left = (FL_INFO_BMP_L + osd_rect.u_left ) * 720/1280;
        rect_info.right = rect_info.left + FL_INFO_BMP_W*720/1280;

     #ifdef BIDIRECTIONAL_OSD_STYLE
        if (osd_get_mirror_flag() == TRUE)
        {
            rect_info.left = 720 - rect_info.right -8;
            rect_info.right = rect_info.left + FL_INFO_BMP_W*720/1280;
        }
     #endif

#else
        if((TV_MODE_PAL == tv_mode) || (TV_MODE_720P_50 == tv_mode))
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top);
            rect_info.bottom = rect_info.top + FL_INFO_BMP_H;
        }
        else
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top);
            rect_info.bottom = rect_info.top + (FL_INFO_BMP_H);
        }
        rect_info.left = (FL_INFO_BMP_L + osd_rect.u_left ) ;
        rect_info.right = rect_info.left + FL_INFO_BMP_W;
#endif
        usblst_set_preview_txt_enable(1);
        imagedec_ioctl(1, IMAGEDEC_IO_CMD_FILL_LOGO, 0x356B9D);//yvu

        vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RELEASE_VFB, 0);
#ifndef _USE_32M_MEM_
        //if(ext && strncasecmp(ext, ".gif", file_ext_name_len))
        {
            win_image_file_preview(full_name, rect_info, IMAGEDEC_THUMBNAIL);
        }
#endif
        api_set_vpo_dit(TRUE);
    }
    else
    {
        m_title_type = win_filelist_get_mp_title_type();
        osd_set_bitmap_content(&usb_preview_bmp, preview_icon_ids[m_title_type]);
        osd_draw_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
    }
}

static void display_video_preview(char *full_name,int fname_size, p_file_info pfile_node)
{
    UINT8 tv_mode = 0;
    struct osdrect osd_rect;
    RECT rect_info;
    INT16 tmp_top = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;

    MEMSET(&osd_rect, 0x0, sizeof(struct osdrect));
    MEMSET(&rect_info, 0x0, sizeof(RECT));
    if((0==usblst_get_edit_flag()) && ((F_MPG == pfile_node->filetype) || (F_MPEG == pfile_node->filetype)))
    {
        osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
        usblst_mp_close_video();
        osal_task_sleep(100);

        tv_mode = api_video_get_tvout();
        osd_get_rect_on_screen(&osd_rect);
#ifndef SD_UI
        rect_info.left = (FL_INFO_BMP_L + osd_rect.u_left)*720/1280; //+ OSD_STARTCOL;
        rect_info.right = (rect_info.left + FL_INFO_BMP_W + FL_INFO_BMP_L)*720/1280;

    #ifdef BIDIRECTIONAL_OSD_STYLE
           if (osd_get_mirror_flag() == TRUE)
           {
                //rect_info.right = rect_info.left + FL_INFO_BMP_W*720/1280;
                rect_info.left = 720 - rect_info.right ;
                rect_info.right = (FL_INFO_BMP_L + osd_rect.u_left)*720/1280;
                
           }
    #endif

        if((TV_MODE_PAL == tv_mode) || (TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode) \
            ||(TV_MODE_1080P_25 == tv_mode) ||(TV_MODE_1080P_50 == tv_mode))
        {
            
            /*rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top)*576/720;//FL_INFO_BMP_T + OSD_STARTROW_P;
              rect_info.bottom = rect_info.top + FL_INFO_BMP_H*576/720;
              hde_config_preview_window(rect_info.left, rect_info.top,((FL_INFO_BMP_W + 4)*720/1280), \
                rect_info.bottom-rect_info.top,TRUE);*/
            tmp_top = (FL_INFO_BMP_T + osd_rect.u_top)*2880/720;
            hde_config_preview_window_ex(rect_info.left,tmp_top,((FL_INFO_BMP_W +4)*720/1280), \
              (FL_INFO_BMP_H+2)*2880/720); 
            rect_info.top = tmp_top;
            rect_info.bottom = tmp_top + FL_INFO_BMP_H*2880/720;
              
        }
        else
        {
            /*rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top)*480/720;  //(FL_INFO_BMP_T + OSD_STARTROW_N);
            
              hde_config_preview_window(rect_info.left, rect_info.top,((FL_INFO_BMP_W + 4)*720/1280), \
                rect_info.bottom-rect_info.top,FALSE);*/
            tmp_top = (FL_INFO_BMP_T + osd_rect.u_top)*2880/720;
            hde_config_preview_window_ex(rect_info.left, tmp_top,((FL_INFO_BMP_W+4)*720/1280), \
            (FL_INFO_BMP_H+2)*2880/720);
            rect_info.top = tmp_top;
            rect_info.bottom = rect_info.top + FL_INFO_BMP_H*2880/720;
        }
#else
        rect_info.left = (FL_INFO_BMP_L + osd_rect.u_left); //+ OSD_STARTCOL;
        rect_info.right = rect_info.left + FL_INFO_BMP_W;

        if((TV_MODE_PAL == tv_mode) || (TV_MODE_720P_50 == tv_mode))
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top);//FL_INFO_BMP_T + OSD_STARTROW_P;
            rect_info.bottom = rect_info.top + FL_INFO_BMP_H;
            hde_config_preview_window(rect_info.left, rect_info.top,((FL_INFO_BMP_W + 4)), \
              rect_info.bottom-rect_info.top,TRUE);
        }
        else
        {
            rect_info.top = (FL_INFO_BMP_T + osd_rect.u_top)*480/576;  //(FL_INFO_BMP_T + OSD_STARTROW_N);
            rect_info.bottom = rect_info.top + FL_INFO_BMP_H*480/576;
            hde_config_preview_window(rect_info.left, rect_info.top,((FL_INFO_BMP_W + 4)),
              rect_info.bottom-rect_info.top,FALSE);
        }
#endif

        hde_set_mode(VIEW_MODE_PREVIEW);
        api_set_vpo_dit(TRUE);
        win_get_fullname(full_name, fname_size, pfile_node->path, pfile_node->name);
        video_dec_file(full_name, 1);

        usblst_set_previewing_mpeg(TRUE);//g_previewing_mpeg = TRUE;
    }
    else
    {
        m_title_type = win_filelist_get_mp_title_type();
        osd_set_bitmap_content(&usb_preview_bmp, preview_icon_ids[m_title_type]);
        osd_draw_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
    }
}

#ifdef MP_SPECTRUM
extern BOOL g_preview_spectrum_enable;
#endif

void display_file_info_preview(void)
{//display preview window info
    POBJLIST pol = NULL;
    UINT16 ucurpos = 0;
    media_file_info file_node;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    file_list_handle t_cur_filelist = NULL;


    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);

    if(MP_DEVICE_ID == osd_get_focus_id((POBJECT_HEAD)&g_win_usb_filelist))
    {
#ifdef MP_SPECTRUM
        if(MUSIC_PLAY_STATE == win_filelist_get_mp_play_state())
        {
            close_spectrum_dev();
            g_preview_spectrum_enable = FALSE;
        }
#endif
        m_title_type = win_filelist_get_mp_title_type();
        osd_set_bitmap_content(&usb_preview_bmp, preview_icon_ids[m_title_type]);
        osd_draw_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);

        if(get_mute_state())
        {
            mp_show_mute();
        }
        //if (NULL != full_name)
        //{
            FREE(full_name);
            full_name = NULL;
        //}
        return;
    }

    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    if(win_check_openroot(t_cur_filelist))
    {
        ucurpos++;
    }

    if (get_file_from_file_list(t_cur_filelist, ucurpos, &file_node) == RET_FAILURE)
    {
        file_node.filetype = F_UNKOWN;
    }

    if(!((F_MPG == file_node.filetype) || (F_MPEG == file_node.filetype)))
    {
        usblst_mp_display_logo();
    }

    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
#ifdef MP_SPECTRUM
        if(MUSIC_PLAY_STATE == win_filelist_get_mp_play_state())
        {
            open_spectrum_dev();
            g_preview_spectrum_enable = TRUE;
        }
        else
#endif
        {
            m_title_type = win_filelist_get_mp_title_type();
            osd_set_bitmap_content(&usb_preview_bmp, preview_icon_ids[m_title_type]);
            osd_draw_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
        }
    }
    else if(TITLE_IMAGE == win_filelist_get_mp_title_type())
    {
        display_image_preview(full_name, FULL_PATH_SIZE, &file_node);
    }
    else if(TITLE_VIDEO == win_filelist_get_mp_title_type())
    {
        display_video_preview(full_name, FULL_PATH_SIZE, &file_node);
    }

    if(get_mute_state())
    {
        mp_show_mute();
    }

    if (NULL != full_name)
    {
        FREE(full_name);
        full_name = NULL;
    }
}

static void display_image_details(TEXT_FIELD *ptxt, p_file_info pfile_node, char *full_name, int fname_size)
{
#ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
#endif

#ifdef _INVW_JUICE
    char str_txt[100] = {0};
    int str_txt_len = 100;
#else
    char str_txt[70] = {0};
    int str_txt_len = 70;
#endif

    UINT8 i = 0;

    win_get_fullname(full_name,fname_size, pfile_node->path, pfile_node->name);

    for(i = 0;i < 3;i++)
    {
        osd_set_obj_rect((POBJECT_HEAD)ptxt, \
            FL_INFO_DETAIL_L+190, (FL_INFO_DETAIL_T+100 + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W+100, FL_INFO_DETAIL_H+4);
        switch(i)
        {
            case 0:
            #ifdef PERSIAN_SUPPORT
                filelist_utf8_to_unicode(pfile_node->name, str_uni);
                com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
                osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
            #else
                //filter_unkownfont_name(file_node.name,str_txt);//filter name that can not be show
                win_shorten_filename(pfile_node->name,str_txt, str_txt_len);
                snprintf(str_txt, str_txt_len, "%s",str_txt);
                osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
            #endif
                break;
             case 1:
                snprintf(str_txt, str_txt_len, "%luX%luX%lu bpp", \
                    cur_imageinfo.width, cur_imageinfo.height, cur_imageinfo.bbp);
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            case 2:
                if(cur_imageinfo.fsize < 1024)
                    snprintf(str_txt, str_txt_len, "%ld B",cur_imageinfo.fsize);
                else 
                    snprintf(str_txt, str_txt_len , "%lu KB",cur_imageinfo.fsize/1024);
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            default:
                break;
        }
	 osd_clear_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
    }

}

void display_music_details(TEXT_FIELD *ptxt, p_file_info pfile_node, char *full_name, int fname_size)
{
    UINT8 i = 0;
    UINT32 hour = 0;
    UINT32 min = 0;
    UINT32 sec = 0;
    UINT32 size = 0;
    music_info cur_mp3info;

#ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
#endif

#ifdef _INVW_JUICE
    char str_txt[100] = {0};
    int str_txt_len = 100;
#else
    char str_txt[70] = {0};
    int str_txt_len = 70;
    
#endif

    MEMSET(&cur_mp3info, 0x0, sizeof(music_info));
    win_get_fullname(full_name,fname_size, pfile_node->path, pfile_node->name);
    win_get_fileinfo(full_name, (UINT32)&cur_mp3info);

    hour = cur_mp3info.time/3600;
    min = (cur_mp3info.time%3600)/60;
    sec = cur_mp3info.time%60;
    size = cur_mp3info.file_length>> 10;
    for(i = 0;i < 3;i++)
    {
        osd_set_obj_rect((POBJECT_HEAD)ptxt, \
          FL_INFO_DETAIL_L+190, (FL_INFO_DETAIL_T+100 + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W+100, FL_INFO_DETAIL_H+4);
        switch(i)
        {
            case 0:
                #ifdef PERSIAN_SUPPORT
                    filelist_utf8_to_unicode(pfile_node->name, str_uni);
                    com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
                    osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
                #else//filter_unkownfont_name(file_node.name,str_txt);//filter name that can not be show
                win_shorten_filename(pfile_node->name,str_txt, str_txt_len);
                snprintf(str_txt, str_txt_len, "%s",str_txt);
                osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
                #endif
                break;
            case 1:
                snprintf(str_txt, str_txt_len, "%lu KB", size);
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            case 2:
                snprintf(str_txt, str_txt_len, "%02lu:%02lu:%02lu", hour, min, sec);
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            default:
                break;
        }
	 osd_clear_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
    }
}

void set_mplayer_display_video(void)
{
    BITMAP *pbmp = NULL;

    pbmp = &mplayer_prv;
    osd_set_bitmap_content(pbmp, 0);

    pbmp = &mplayer_xback;
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY);

    pbmp = &mplayer_pause;
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE);

    pbmp = &mplayer_stop;
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP);

    pbmp = &mplayer_xforward;
    osd_set_bitmap_content(pbmp, 0);

    pbmp = &mplayer_nxt;
    osd_set_bitmap_content(pbmp, 0);
}

void draw_mplayer_display(void)
{
    BITMAP *pbmp = NULL;
    UINT8 i = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    if(TITLE_IMAGE == win_filelist_get_mp_title_type())
    {
        return;
    }

    for(i = 0; i < MPLAYER_ITEM_CNT;i++)
    {
        pbmp = mplayer_items[i];
        osd_draw_object((POBJECT_HEAD)pbmp,C_UPDATE_ALL);
    }
    m_title_type = win_filelist_get_mp_title_type();
    if(m_title_type == TITLE_MUSIC)
    {
        osd_set_bitmap_content(&usb_preview_bmp, preview_icon_ids[m_title_type]);
        osd_draw_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
    }
}

static void display_video_details(TEXT_FIELD *ptxt, p_file_info pfile_node)
{
    UINT8 i = 0;

#ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
#endif

#ifdef _INVW_JUICE
    char str_txt[100] = {0};
    int str_txt_len = 100;
#else
    char str_txt[70] = {0};
    int str_txt_len = 70;

#endif

    for(i = 0;i < 3;i++)
    {
        osd_set_obj_rect((POBJECT_HEAD)ptxt, \
            FL_INFO_DETAIL_L+190, (FL_INFO_DETAIL_T+100 + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W+100, FL_INFO_DETAIL_H+4);
        switch(i)
        {
            case 0:
            #ifdef PERSIAN_SUPPORT
                filelist_utf8_to_unicode(pfile_node->name, str_uni);
                com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
                osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
            #else                //filter_unkownfont_name(file_node.name,str_txt);//filter name that can not be show
                win_shorten_filename(pfile_node->name,str_txt, str_txt_len);
                snprintf(str_txt, str_txt_len, "%s",str_txt);
                osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
            #endif
                break;
            case 1:
                snprintf(str_txt, str_txt_len, "%s","");
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            case 2:
                snprintf(str_txt, str_txt_len, "%d KB",pfile_node->size/1024);
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                break;
            default:
                break;
        }
	     osd_clear_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
    }

    usblst_set_video_play_state(MPEG_STOP);
    set_mplayer_display_video();
    osd_set_bitmap_content(&mplayer_stop, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
    draw_mplayer_display();

    osal_task_sleep(500);
}

static void display_file_info_details(void)
{
    UINT8 i = 0;
    UINT16 ucurpos = 0;
#ifdef _INVW_JUICE
    char str_txt[100] = {0};
    int str_txt_len = 100;
#else
    char str_txt[70] = {0};
    int str_txt_len = 70;
#endif
    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif
    TEXT_FIELD *ptxt = NULL;
    POBJLIST pol = NULL;
    media_file_info file_node;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    file_list_handle t_cur_filelist = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);

    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    ptxt = &usb_file_info;

    if(MP_DEVICE_ID == osd_get_focus_id((POBJECT_HEAD)&g_win_usb_filelist))
    {
        for(i = 0;i < 3;i++)
        {
            osd_set_obj_rect((POBJECT_HEAD)ptxt, \
                FL_INFO_DETAIL_L, (FL_INFO_DETAIL_T  + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W, FL_INFO_DETAIL_H);
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
           // osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
        }
        if (NULL != full_name)
        {
            FREE(full_name);
            full_name = NULL;
        }
        return;
    }

    t_cur_filelist = win_filelist_get_cur_filelist();
    if(!win_check_openroot(t_cur_filelist))
    {
        if(ucurpos != 0)
        {
            ucurpos--;
        }
        else
        {
            for(i = 0;i < 3;i++)
            {
                osd_set_obj_rect((POBJECT_HEAD)ptxt, \
                    FL_INFO_DETAIL_L, (FL_INFO_DETAIL_T  + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W, FL_INFO_DETAIL_H);
                switch(i)
                {
                    case 0:
                        snprintf(str_txt, str_txt_len, "%s","/..");
                        break;
                    case 1:
                    case 2:
                        snprintf(str_txt, str_txt_len, "%s","");
                        break;
                    default:
                        break;
                }
                osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                //osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
            }

            if (NULL != full_name)
            {
                FREE(full_name);
                full_name = NULL;
            }
            return;
        }
    }

    t_cur_filelist = win_filelist_get_cur_filelist();
    if (get_file_from_file_list(t_cur_filelist, ucurpos+1, &file_node) == RET_FAILURE)
    {
        file_node.filetype = F_UNKOWN;
    }

    if((F_JPG == file_node.filetype) || (F_BMP == file_node.filetype))
    {
        display_image_details(ptxt, &file_node, full_name, FULL_PATH_SIZE);
    }
    else if((F_MP3 == file_node.filetype) || (F_BYE1 == file_node.filetype) || (F_OGG == file_node.filetype)
     || (F_FLC == file_node.filetype)|| (F_WAV == file_node.filetype))
    {
        display_music_details(ptxt, &file_node, full_name, FULL_PATH_SIZE);
    }
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    else if((F_MPG == file_node.filetype) || (F_MPEG == file_node.filetype))
    {
        display_video_details(ptxt, &file_node);
    }
#endif
    else if(F_DIR == file_node.filetype)
    {
        for(i = 0;i < 3;i++)
        {
            osd_set_obj_rect((POBJECT_HEAD)ptxt, \
                FL_INFO_DETAIL_L, (FL_INFO_DETAIL_T + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W, FL_INFO_DETAIL_H);
            switch(i)
            {
                case 0:
                    #ifdef PERSIAN_SUPPORT
                        filelist_utf8_to_unicode(file_node.name, str_uni);
                        com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
                        osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
                    #else
                    //filter_unkownfont_name(file_node.name,str_txt);//filter name that can not be show
                    win_shorten_filename(file_node.name,str_txt, str_txt_len);
                    osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
                    #endif
                    break;
                case 1:
                case 2:
                    snprintf(str_txt, str_txt_len, "%s","");
                    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);
                    break;
                default:
                    break;
            }
            //osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
        }
    }
    else
    {
        for(i = 0;i < 3;i++)
        {
            osd_set_obj_rect((POBJECT_HEAD)ptxt, \
                FL_INFO_DETAIL_L, (FL_INFO_DETAIL_T  + (FL_INFO_DETAIL_H)*i), FL_INFO_DETAIL_W, FL_INFO_DETAIL_H);
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));
           // osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
        }
    }

    if (NULL != full_name)
    {
        FREE(full_name);
        full_name = NULL;
    }
}

void display_file_info(void)
{
    ID t_id = OSAL_INVALID_ID;

    usblst_set_preview_txt_enable(0);
    image_abort();
    usblst_mp_close_video();
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    if (usblst_get_previewing_mpeg())//if(g_previewing_mpeg)
    {
        t_id = win_filelist_get_mp_refresh_id();
        api_stop_timer(&t_id);
        win_filelist_set_mp_refresh_id(t_id);
        mpg_cmd_stop_proc(0);

        usblst_set_video_play_state(MPEG_STOP);
        set_mplayer_display_video();
        draw_mplayer_display();

        osd_set_text_field_content(&mplayer_curtime,STRING_ANSI,(UINT32)(" "));
        osd_draw_object((POBJECT_HEAD)&mplayer_curtime,C_UPDATE_ALL);
        osd_set_text_field_content(&mplayer_total,STRING_ANSI,(UINT32)(" "));
        osd_draw_object((POBJECT_HEAD)&mplayer_total,C_UPDATE_ALL);
        osd_set_progress_bar_pos(&mplayer_bar,0);
        osd_draw_object((POBJECT_HEAD)&mplayer_bar,C_UPDATE_ALL);

        osal_task_sleep(300);//waite mpeg decoder task exit and vdec stop
        api_full_screen_play();
    }
    usblst_set_previewing_mpeg(FALSE);//g_previewing_mpeg = FALSE;
#endif

    display_file_info_preview();

    display_file_info_details();
}

void set_mplayer_display(UINT8 focus_id)
{
    BITMAP *pbmp = NULL;

    pbmp = &mplayer_prv;
    #ifdef BIDIRECTIONAL_OSD_STYLE
       if (osd_get_mirror_flag() == TRUE)
       {
           pbmp = &mplayer_nxt;
       }
    #endif
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_BACKWARD);

    pbmp = &mplayer_xback;
    #ifdef BIDIRECTIONAL_OSD_STYLE
       if (osd_get_mirror_flag() == TRUE)
       {
           pbmp = &mplayer_xforward;
       }
    #endif
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWBACK);

    pbmp = &mplayer_pause;
    if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
    {
        osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE);
    }
    else
    {
        osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY);
    }

    pbmp = &mplayer_stop;
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP);

    pbmp = &mplayer_xforward;
    #ifdef BIDIRECTIONAL_OSD_STYLE
       if (osd_get_mirror_flag() == TRUE)
       {
           pbmp = &mplayer_xback;
       }
    #endif
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWFAST);

    pbmp = &mplayer_nxt;
    #ifdef BIDIRECTIONAL_OSD_STYLE
       if (osd_get_mirror_flag() == TRUE)
       {
           pbmp = &mplayer_prv;
       }
    #endif
    osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_FORWARD);

    switch(focus_id)
    {
        case MP_PRV_ID:
            pbmp = &mplayer_prv;
            #ifdef BIDIRECTIONAL_OSD_STYLE
              if (osd_get_mirror_flag() == TRUE)
               {
                   osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_FORWARD_HI);
                break;
               }
            #endif
            osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_BACKWARD_HI);
            break;
        case MP_XBACK_ID:
            pbmp = &mplayer_xback;
            #ifdef BIDIRECTIONAL_OSD_STYLE
              if (osd_get_mirror_flag() == TRUE)
               {
                   osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWFAST_HI);
                break;
               }
            #endif
            osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWBACK_HI);
            break;
        case MP_PAUSE_ID:
            pbmp = &mplayer_pause;
            if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
            {
                osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PAUSE_HI);
            }
            else
            {
                osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_PLAY_HI);
            }
            break;
        case MP_STOP_ID:
            pbmp = &mplayer_stop;
            osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
            break;
        case MP_XFORWARD_ID:
            pbmp = &mplayer_xforward;
            #ifdef BIDIRECTIONAL_OSD_STYLE
                   if (osd_get_mirror_flag() == TRUE)
               {
                   osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWBACK_HI);
                break;
               }
            #endif
            osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_NEWFAST_HI);
            break;
        case MP_NXT_ID:
            pbmp = &mplayer_nxt;
            #ifdef BIDIRECTIONAL_OSD_STYLE
              if (TRUE == osd_get_mirror_flag())
               {
                   osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_BACKWARD_HI);
                break;
               }
            #endif
            osd_set_bitmap_content(pbmp, IM_N_MEDIAPLAY_CONTROL_FORWARD_HI);
            break;
        case MP_NO_FOCUS:
        default:
            break;
    }
}

void set_mplayer_display_normal(void)
{
    if((MUSIC_PLAY_STATE == win_filelist_get_mp_play_state())||(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state()))
    {
        set_mplayer_display(MP_PAUSE_ID);
    }
    else if(MUSIC_STOP_STATE == win_filelist_get_mp_play_state())
    {
        set_mplayer_display(MP_STOP_ID);
    }
    else
    {
        set_mplayer_display(MP_NO_FOCUS);
    }
}

void init_filelist_display(void)
{
    char *file_path = NULL;//[MAX_FILE_NAME_SIZE+ 1] = {0};
    char *str_buff = NULL;//[MAX_FILE_NAME_SIZE+ 1] = {0};
    UINT16 offset = 0;
    UINT16 utop = 0;
    TEXT_FIELD *ptxt = NULL;
    POBJLIST pol = NULL;
    char *t_cur_device = NULL;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    file_list_type t_fl_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif

    file_path = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == file_path)
    {
        ASSERT(0);
        return;
    }
    str_buff = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == str_buff)
    {
        FREE(file_path);
        ASSERT(0);
        return;
    }
    MEMSET(file_path, 0x0, MAX_FILE_NAME_SIZE + 1);
    MEMSET(str_buff, 0x0, MAX_FILE_NAME_SIZE + 1);
    t_cur_filelist = win_filelist_get_cur_filelist();
    t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, file_path, MAX_FILE_NAME_SIZE+1);
    if (RET_SUCCESS == t_ret)
    {
        usblst_set_dirnum(t_dirnum);
        usblst_set_filenum(t_filenum);
    }

    if(!STRCMP(file_path, FS_ROOT_DIR))
    {
        offset = 0;
    }
    else
    {
        offset = 1;
    }

    t_fl_type = usblst_get_mp_filelist_type();
    if ((UINT16)(~0) == usblst_get_latest_file_index(t_fl_type))
    {
        usblst_set_cur_fileidx(0);
        utop = 0;
    }
    else
    {
        t_fl_type = usblst_get_mp_filelist_type();
        t_fileidx = usblst_get_latest_file_index(t_fl_type);//latest_file_index[mp_filelist_type];
        usblst_set_cur_fileidx(t_fileidx);
        utop = usblst_get_latest_file_top(t_fl_type);//latest_file_top[mp_filelist_type];
        usblst_set_latest_file_index((UINT16)(~0), t_fl_type);
        usblst_set_latest_file_top((UINT16)(~0), t_fl_type);
    }

    pol = &usblst_olist;
    t_dirnum = usblst_get_dirnum();
    t_filenum = usblst_get_filenum();
    osd_set_obj_list_count(pol,(t_dirnum + t_filenum + offset));
    t_fileidx = usblst_get_cur_fileidx();
    osd_set_obj_list_new_point(pol,t_fileidx);
    osd_set_obj_list_cur_point(pol,t_fileidx);
    osd_set_obj_list_top(pol,utop);
    if(osd_get_obj_list_count(pol) <= FL_ITEM_CNT)
    {
        osd_set_obj_list_top(pol, 0);
    }

    ptxt = &usb_dir_txt;
#ifdef PERSIAN_SUPPORT
    filelist_utf8_to_unicode(file_path, str_uni);
    com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
    osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
#else
    win_get_display_dirname(str_buff,MAX_FILE_NAME_SIZE + 1 ,file_path);
    osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_buff);//show current dir name
#endif
    ptxt = &usb_dev_name;
    t_fl_type = usblst_get_mp_filelist_type();
    t_cur_device = usblst_get_cur_device(t_fl_type);
    win_get_display_devicename(str_buff, MAX_FILE_NAME_SIZE+1,t_cur_device);
    snprintf(str_buff,MAX_FILE_NAME_SIZE+1, "%s",str_buff);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buff);//show current device name

    ptxt = &usb_file_info;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(""));

    if (NULL != file_path)
    {
        FREE(file_path);
    }
    if (NULL != str_buff)
    {
        FREE(str_buff);
    }
}

void init_mplayer_display(void)
{
    TEXT_FIELD *ptxt = NULL;
    PROGRESS_BAR *pbar = NULL;
    BITMAP *pbmp = NULL;
    BOOL bshowloopflag = FALSE;
    play_list_loop_type lp_mode = PLAY_LIST_NONE;
    music_info *pmusic_info = NULL;

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        if((win_filelist_get_mp_title_type() == TITLE_MUSIC) || (win_filelist_get_mp_title_type() == TITLE_VIDEO)
            || (win_filelist_get_mp_title_type() == TITLE_IMAGE))
        {
            bshowloopflag = TRUE;
        }
    }
    else
    {
        if((win_filelist_get_mp_title_type() == TITLE_MUSIC) || (win_filelist_get_mp_title_type() == TITLE_VIDEO))
        {
            bshowloopflag = TRUE;
        }
    }
#else
    if((TITLE_MUSIC == win_filelist_get_mp_title_type()) || (TITLE_VIDEO == win_filelist_get_mp_title_type()))
    {
        bshowloopflag = TRUE;
    }
#endif

    if (bshowloopflag)
    {
        osd_set_objp_next(&usb_file_info, &usb_music_con);
    }
    else
    {
        osd_set_objp_next(&usb_file_info, NULL);
    }

    pbmp = &mplayer_mode;

    if (bshowloopflag)
    {
        if(TITLE_MUSIC == win_filelist_get_mp_title_type())
        {
            if((win_filelist_get_loop_mode() != PLAY_LIST_REPEAT) && (win_filelist_get_loop_mode()!=PLAY_LIST_ONE)
               && (win_filelist_get_loop_mode()!=PLAY_LIST_RAND))
            {
                win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
            }
        }
        else if(TITLE_VIDEO == win_filelist_get_mp_title_type())
        {
            if((win_filelist_get_loop_mode()!=PLAY_LIST_REPEAT) && (win_filelist_get_loop_mode()!=PLAY_LIST_ONE)
               && (win_filelist_get_loop_mode()!=PLAY_LIST_NONE))
            {
                win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
            }
        }
    #ifdef DLNA_DMP_SUPPORT
        else if (win_filelist_get_mp_title_type() == TITLE_IMAGE && mp_is_dlna_play())
        {
            if((win_filelist_get_loop_mode()!=PLAY_LIST_REPEAT) && (win_filelist_get_loop_mode()!=PLAY_LIST_ONE)
               && (win_filelist_get_loop_mode()!=PLAY_LIST_NONE))
            {
                win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
            }
        }
    #endif

        if(PLAY_LIST_NONE==win_filelist_get_loop_mode())
        {
            osd_set_bitmap_content(&mplayer_mode, 0);
        }
        else
        {
            if(1 == usblst_get_music_folderloop_flag())
            {
                lp_mode = win_filelist_get_loop_mode();
                if ((lp_mode - 1) >= ARRAY_SIZE(filelist_loop_icons))
                {
                    lp_mode = 1;
                }
                osd_set_bitmap_content(pbmp, filelist_loop_icons[lp_mode-1]);
            }
            else
            {
                if ((lp_mode - 1) >= ARRAY_SIZE(playlist_loop_icons))
                {
                    lp_mode = 1;
                }
                osd_set_bitmap_content(pbmp, playlist_loop_icons[lp_mode-1]);
            }
        }
    }

    usblst_set_mp_play_time(0);
    usblst_set_mp_play_per(0);

    if(MUSIC_IDLE_STATE == win_filelist_get_mp_play_state())
    {
        pmusic_info = usblst_get_mp_curmusic_info();
        MEMSET(pmusic_info,0x0,sizeof(music_info));
    }

    pbar = &mplayer_bar;
    osd_set_progress_bar_pos(pbar,0);

    ptxt = &mplayer_curtime;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(" "));

    ptxt = &mplayer_total;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)(" "));

    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
        set_mplayer_display_normal();
    }
    else if(TITLE_VIDEO == win_filelist_get_mp_title_type())
    {
        set_mplayer_display_video();
    }
}


void display_loopmode(UINT8 folder_flag)
{
    play_list_loop_type lp_mode = PLAY_LIST_NONE;

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        if(win_filelist_get_mp_title_type() != TITLE_MUSIC && win_filelist_get_mp_title_type() != TITLE_VIDEO && \
            win_filelist_get_mp_title_type() != TITLE_IMAGE)
            return;
    }
#else
    if(win_filelist_get_mp_title_type() != TITLE_MUSIC && win_filelist_get_mp_title_type() != TITLE_VIDEO)
    {
        return;
    }
#endif

    if(PLAY_LIST_NONE==win_filelist_get_loop_mode())
    {
        osd_set_bitmap_content(&mplayer_mode, 0);
    }
    else
    {
        if(folder_flag)
        {
          lp_mode = win_filelist_get_loop_mode();
            osd_set_bitmap_content(&mplayer_mode, filelist_loop_icons[lp_mode-1]);
        }
        else
        {
          lp_mode = win_filelist_get_loop_mode();
            osd_set_bitmap_content(&mplayer_mode, playlist_loop_icons[lp_mode-1]);
        }
    }
    osd_draw_object((POBJECT_HEAD)&mplayer_mode,C_UPDATE_ALL);
}

#endif
