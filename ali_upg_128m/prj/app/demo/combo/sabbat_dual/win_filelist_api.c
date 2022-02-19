/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_api.c
*
*    Description:   Some API of the filelist
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#include <basic_types.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libmp/media_player_api.h>
#include <hld/decv/decv.h>
#include <hld/dmx/dmx.h>
#include <api/libmp/mp_list.h>

#include "menus_root.h"
#include "win_com_popup.h"
#include "string.id"
#include "copper_common/com_api.h"

#include "win_mute.h"
#include "win_imageslide.h"
#include "win_imagepreview.h"
#include "win_copying.h"
#include "control.h"

#include "win_mpeg_player.h"
#include "win_filelist.h"
#include "win_filelist_display.h"
#include "win_filelist_inner.h"
#include "media_control.h"

static MPLAY_STATE mp_play_state = PLAY_LIST_NONE;
static TITLE_TYPE mp_title_type = TITLE_NUMBER;
static play_list_loop_type loop_mode = PLAY_LIST_NONE;
static file_list_handle cur_filelist = NULL;
static play_list_handle win_play_list[WIN_PLAYLIST_NUMBER] = {NULL};
static UINT8 switch_title = 0;
static BOOL opened_by_hot_key = FALSE;
static BOOL    musiclist_changed = FALSE;
static BOOL    imagelist_changed = FALSE;

static ID mp_refresh_id = OSAL_INVALID_ID;

extern int utf8_to_unicode(const char *utf8, unsigned char *uni, unsigned long unilen);
//extern BOOL g_from_mpegplayer;

//MPLAY_STATE usblst_get_mp_play_state(void)
MPLAY_STATE win_filelist_get_mp_play_state(void)
{
    return mp_play_state;
}

//void usblst_set_mp_play_state(MPLAY_STATE state)
void win_filelist_set_mp_play_state(MPLAY_STATE state)
{
    mp_play_state = state;
}

TITLE_TYPE win_filelist_get_mp_title_type(void)
{
    return mp_title_type;
}

void win_filelist_set_mp_title_type(TITLE_TYPE type)
{
    mp_title_type = type;
}

play_list_loop_type win_filelist_get_loop_mode(void)
{
    return loop_mode;
}

void win_filelist_set_loop_mode(play_list_loop_type mode)
{
    loop_mode = mode;
}

file_list_handle win_filelist_get_cur_filelist(void)
{
    return cur_filelist;
}

void win_filelist_set_cur_filelist(file_list_handle flist)
{
    cur_filelist = flist;
}

play_list_handle win_filelist_get_playlist(UINT8 idx)
{
    return win_play_list[idx];
}

void win_filelist_set_playlist(play_list_handle plist, UINT8 idx)
{
    win_play_list[idx] = plist;
}

UINT8 win_filelist_get_switch_title(void)
{
    return switch_title;
}

void win_filelist_set_switch_title(UINT8 val)
{
    switch_title = val;
}

BOOL win_filelist_get_opened_hotkey(void)
{
    return opened_by_hot_key;
}

void win_filelist_set_opened_hotkey(BOOL flag)
{
    opened_by_hot_key = flag;
}

BOOL win_filelist_get_musiclist_changed(void)
{
    return musiclist_changed;
}

void win_filelist_set_musiclist_changed(BOOL flag)
{
    musiclist_changed = flag;
}

BOOL win_filelist_get_imagelist_changed(void)
{
    return imagelist_changed;
}

void win_filelist_set_imagelist_changed(BOOL flag)
{
    imagelist_changed = flag;
}

ID win_filelist_get_mp_refresh_id(void)
{
    return mp_refresh_id;
}

void win_filelist_set_mp_refresh_id(ID t_id)
{
    mp_refresh_id = t_id;
}

#ifdef PERSIAN_SUPPORT
void filelist_utf8_to_unicode(const char *src_filename, char *uni_filename)
{
    char struni[128] = {0};
    const UINT8 str_max_len = 128;
    const UINT8 cnt_p = 59;
    UINT count=0;

    utf8_to_unicode(src_filename, (unsigned char *)struni, str_max_len);
	while (count <(str_max_len-1))
    {
        uni_filename[count]=struni[count+1];
        uni_filename[count+1]=struni[count];
        if ((0x00 == struni[count]) && (0x00 == struni[count+1]))
        {
            break;
        }
        count+=2;
    }
    if (count > cnt_p)
    {
        uni_filename[52]=uni_filename[54]=uni_filename[56]=uni_filename[58]=uni_filename[59]=0x00;
        uni_filename[53]=uni_filename[55]=uni_filename[57]=0x2e;
    }
}
#endif

PRESULT     usblst_switch_title_tab(void)
{
    PRESULT ret = PROC_LOOP;
    POBJECT_HEAD cur_menu = NULL;
    POBJECT_HEAD new_menu = NULL;

    if(TITLE_RECORD == mp_title_type)
    {
        cur_menu = (POBJECT_HEAD)(&g_win_record);
    }
    else

    {
        cur_menu = (POBJECT_HEAD)(&g_win_usb_filelist);
    }

    mp_title_type = (mp_title_type+1)%TITLE_NUMBER;

    if(TITLE_RECORD== mp_title_type)
    {
        //switch to record manager from Video MP, start DMX/TTX/EPG
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
#ifdef NEW_DEMO_FRAME
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#endif
#if defined(SUPPORT_BC_STD)
        api_mcas_start_transponder();
#elif defined(SUPPORT_BC)
        api_mcas_start_transponder_multi_des(0);
#endif
        new_menu = (POBJECT_HEAD)(&g_win_record);
    }
    else

    {
        new_menu = (POBJECT_HEAD)(&g_win_usb_filelist);
    }

    switch_title = TRUE;
    osd_obj_close(cur_menu,0);
    menu_stack_pop();
    if(osd_obj_open(new_menu,(UINT32)(~0)) != PROC_LEAVE)
    {
        menu_stack_push(new_menu);
    }

    return ret;
}

RET_CODE win_set_musicplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag)
{
    UINT8 back_saved = 0;
    RET_CODE retcode = RET_SUCCESS;
    media_file_info filenode;
    char *music_name = NULL;//[FULL_PATH_SIZE] = {0};
    int playlist_idx = 0;
    play_list_handle t_playlist_music = NULL;
    UINT16 t_music_idx = 0;

    MEMSET(&filenode, 0x0, sizeof(media_file_info));
    music_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == music_name)
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    MEMSET(music_name, 0x0, FULL_PATH_SIZE);

    usblst_set_music_folderloop_flag(folder_flag);
    if(1 == usblst_get_music_folderloop_flag())
    {
        if(*pplaylist != NULL)
        {
            if(RET_SUCCESS != delete_play_list(*pplaylist))
            {
              //if (NULL != music_name)
              //{
                FREE(music_name);
                music_name = NULL;
              //}
                return RET_FAILURE;
            }
        }
    #ifndef _USE_32M_MEM_
        *pplaylist = create_play_list(MUSIC_PLAY_LIST, (const char*)"folder_music");
    #else
        *pplaylist = create_play_list(MUSIC_PLAY_LIST, (const char*)" ");
    #endif
        if(NULL == *pplaylist)
        {
          //if (NULL != music_name)
          //{
            FREE(music_name);
            music_name = NULL;
         // }
            return RET_FAILURE;
        }

        get_file_from_file_list(cur_filelist, idx, &filenode);
        win_get_fullname(music_name, FULL_PATH_SIZE, filenode.path, filenode.name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        retcode = add_all_file_list_items_to_play_list(*pplaylist, cur_filelist);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        if(retcode != RET_SUCCESS)
        {
          //if (NULL != music_name)
         // {
            FREE(music_name);
            music_name = NULL;
          //}
            return RET_FAILURE;
        }
        playlist_idx = get_play_list_index_by_name(*pplaylist, music_name);
        if(-1 == playlist_idx)
        {
          //if (NULL != music_name)
          //{
            FREE(music_name);
            music_name = NULL;
          //}
            return RET_FAILURE;
        }
        usblst_set_cur_playlist_music(*pplaylist);//cur_playlist_music = *pplaylist;
        usblst_set_cur_music_idx(playlist_idx);
    }
    else
    {
        usblst_set_cur_playlist_music(*pplaylist);//cur_playlist_music = *pplaylist;
        usblst_set_cur_music_idx(idx);
    }
    t_playlist_music = usblst_get_cur_playlist_music();
    t_music_idx = usblst_get_cur_music_idx();
    set_play_list_info(t_playlist_music, &t_music_idx, &loop_mode);
    usblst_set_cur_music_idx(t_music_idx);

    if (NULL != music_name)
    {
        FREE(music_name);
        music_name = NULL;
    }
    return RET_SUCCESS;
}

RET_CODE win_get_fileinfo_by_type(char *file_name, UINT32 info,UINT8 mp_type);
void win_play_next_music_ex(void)
{
    media_file_info file_node;
    int next_idx = 0;
    int ret_val = 0;
    char *t_name = NULL;
    play_list_handle t_playlist_music = NULL;
    UINT16 t_music_idx = 0;
    music_info *p_curmusic_info = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    music_stop();
    if(MUSIC_IDLE_STATE == mp_play_state)
    {
        return;
    }

    mp_play_state = MUSIC_STOP_STATE;

    t_playlist_music = usblst_get_cur_playlist_music();
    next_idx = get_next_index_from_play_list(t_playlist_music);
    if(next_idx != -1 && loop_mode != PLAY_LIST_NONE)
    {
        usblst_set_cur_music_idx(next_idx);
        t_playlist_music = usblst_get_cur_playlist_music();
        t_music_idx = usblst_get_cur_music_idx();
        set_play_list_info(t_playlist_music, &t_music_idx, NULL);
        usblst_set_cur_music_idx(t_music_idx);
        get_file_from_play_list(t_playlist_music, t_music_idx, &file_node);
        t_name = usblst_get_mp_curmusic_name();
        win_get_fullname(t_name, FULL_PATH_SIZE, file_node.path, file_node.name);
        p_curmusic_info = usblst_get_mp_curmusic_info();
        //win_get_fileinfo(t_name, (UINT32)p_curmusic_info);
        win_get_fileinfo_by_type(t_name, (UINT32)p_curmusic_info,TITLE_MUSIC);
        ret_val = music_play(t_name);

        mp_play_state = MUSIC_PLAY_STATE;
        if(ret_val < 0)//can not play music
        {
            osal_task_sleep(200);//delay to avoid flicker of unregonized music
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MP3OVER,0,TRUE);
        }
    }
}

#ifdef DLNA_DMP_SUPPORT
//media_type: 0, video; 1, audio; 2, image
// image has no network seek
static void dmp_set_network_seek(char *file_name, UINT8 media_type)
{
    UINT8 seek_type;
    UINT8 disable_seek;
    char *dmp_proto = NULL;

    seek_type = 1;
    dmp_proto = dmp_get_res_proto_info(file_name);
    if (dmp_proto)
    {
        seek_type = dlnaPlayerCheckRangeSupport(dmp_proto);
    }

    if (DLNA_MEDIA_TYPE_IMG == media_type)
    {
        network_set_seek_type(0);
    }
    else
    {
        network_set_seek_type(seek_type);
    }

    if (network_is_disable_seek())
    {
        disable_seek = 1;
    }
    else
    {
        disable_seek = 0;
    }

    if (DLNA_MEDIA_TYPE_AUDIO == media_type)
    {
        music_engine_set_disable_seek(disable_seek);
    }
    else if (DLNA_MEDIA_TYPE_VIDEO == media_type)
    {
        video_set_disable_seek(disable_seek);
    }
}

#endif

#ifdef DLNA_DMP_SUPPORT
//media_type: 0, video; 1, audio; 2, image, no use
void win_dlna_play_preset(char *path_name, UINT8 media_type)
{
    char *p_size = NULL;
    INT64 size = 0;
    char *url;
    dmp_set_network_seek(path_name, media_type);
    dlna_player_set_uri(dmp_get_res_uri(path_name));

    p_size = dmp_get_res_size(path_name);
    if (p_size)
    {
        sscanf(p_size, "%lld", &size);
        url = dlnaPlayerGetURI();
        network_set_url_content_size(url,size);
    }
}
#endif

void usblist_set_filelist_param(char *full_name)
{
    BOOL broot = FALSE;
    UINT32 file_idx = 0;
    UINT16 t_val = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    file_idx = win_get_file_idx(cur_filelist, full_name);
    broot = win_check_openroot(cur_filelist);
    t_val = file_idx + (broot?0:1);
    t_type = usblst_get_mp_filelist_type();
    usblst_set_latest_file_top(t_val, t_type);
    t_val = file_idx + (broot?0:1);
    t_type = usblst_get_mp_filelist_type();
    usblst_set_latest_file_index(t_val, t_type);
}

void mp_show_mute(void)
{
    OBJECT_HEAD *obj = NULL;

    obj = (OBJECT_HEAD*)&usb_mute_bmp;

    if(get_mute_state() && mute_state_is_change())
    {   //from lock to mediaplayer.
        set_mute_on_off(FALSE);
        save_mute_state();
    }

    if(MUTE_STATE == get_mute_state())
    {
        osd_draw_object(obj, C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object(obj, 0);
    }
}

//added to resolve mute clear display bug
void restore_mute_bg(void)
{
    POBJECT_HEAD win_top = NULL;

    win_top = menu_stack_get_top();
    if(win_top == (POBJECT_HEAD)&g_win_usb_filelist)
    {
        if((!usblst_check_file_previewing()) && (!get_mute_state()))
        {
            osd_draw_object((POBJECT_HEAD)&usb_mute_black_bg,C_UPDATE_ALL);
        }
    }
}

//extern BITMAP win_mute_bmp;
extern CONTAINER g_win2_mute;
void win_mpvolume_open(void)
{
    UINT32 hkey = 0;
    CONTAINER *pop_win = NULL;
    PRESULT ret = PROC_LOOP;
    CONTAINER *p_mute_con = NULL;
    OSD_RECT rect;
    BOOL old_value = 0;

    //set mute bmp rect in filelist
    MEMSET(&rect, 0x0, sizeof(OSD_RECT));
    get_mute_rect(&rect);
    p_mute_con = &g_win2_mute;
    osd_set_obj_rect((POBJECT_HEAD)p_mute_con, INFO_MUTE_L, INFO_MUTE_T, INFO_MUTE_W, INFO_MUTE_H);

    old_value = ap_enable_key_task_get_key(TRUE);
    pop_win = &g_win2_volume;//&g_win_mpvolume;
    wincom_backup_region(&pop_win->head.frame);
    osd_obj_open((POBJECT_HEAD)pop_win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }

        ret = osd_obj_proc((POBJECT_HEAD)pop_win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    wincom_restore_region();

    //restore mute bmp rect normal
    osd_set_obj_rect((POBJECT_HEAD)p_mute_con, rect.u_left, rect.u_top, rect.u_width, rect.u_height);

    ap_enable_key_task_get_key(old_value);
}

RET_CODE     win_get_fileinfo(char *file_name, UINT32 info)
{
    RET_CODE ret = RET_SUCCESS;
    music_info *msc_info = NULL;
    struct image_info *pic_info = NULL;
    const UINT8 song_info_valid_2 = 2;
    const UINT8 song_info_valid_3 = 3;
    int info_ret = 0;

    switch(mp_title_type)
    {
        case TITLE_MUSIC:
            msc_info = (music_info*)info;
            MEMSET(msc_info, 0, sizeof(music_info));
            info_ret = music_get_song_info(file_name, msc_info);
            if((song_info_valid_2 == info_ret) || (song_info_valid_3 == info_ret))
            {
                ret = RET_SUCCESS;
            }
            else
            {
                ret = RET_FAILURE;
            }
            break;
        case TITLE_IMAGE:
            pic_info = (struct image_info*)info;
            vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RELEASE_VFB, 0);
            MEMSET(pic_info, 0, sizeof(struct image_info));
            info_ret = image_get_info(file_name, pic_info);
            if(info_ret >= 0)
            {
                ret = RET_SUCCESS;
            }
            else
            {
                ret = RET_FAILURE;
            }
            break;
        case TITLE_VIDEO:
            ret = vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, info);
            break;
        default:
            break;
    }

    return ret;
}

RET_CODE win_get_fileinfo_by_type(char *file_name, UINT32 info,UINT8 mp_type)
{
    RET_CODE ret = RET_SUCCESS;
    music_info *msc_info = NULL;
    struct image_info *pic_info = NULL;
    const UINT8 song_info_valid_2 = 2;
    const UINT8 song_info_valid_3 = 3;
    int info_ret = 0;

    switch(mp_type)
    {
        case TITLE_MUSIC:
            msc_info = (music_info*)info;
            MEMSET(msc_info, 0, sizeof(music_info));
            info_ret = music_get_song_info(file_name, msc_info);
            if((song_info_valid_2 == info_ret) || (song_info_valid_3 == info_ret))
            {
                ret = RET_SUCCESS;
            }
            else
            {
                ret = RET_FAILURE;
            }
            break;
        case TITLE_IMAGE:
            pic_info = (struct image_info*)info;
            vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RELEASE_VFB, 0);
            MEMSET(pic_info, 0, sizeof(struct image_info));
            info_ret = image_get_info(file_name, pic_info);
            if(info_ret >= 0)
            {
                ret = RET_SUCCESS;
            }
            else
            {
                ret = RET_FAILURE;
            }
            break;
        case TITLE_VIDEO:
            ret = vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, info);
            break;
        default:
            break;
    }

    return ret;
}


extern IMAGE_SLIDE_INFO image_slide_setting;

void init_usblist_param(void)
{
    UINT8 i = 0;
    SYSTEM_DATA *sys_data = sys_data_get();

#ifdef SAVE_PLAYLIST_SUPPORT
    const char *t_name_music = usblst_get_list_name_music();
    const char *t_name_image = usblst_get_list_name_image();

#endif

    mp_play_state = MUSIC_IDLE_STATE;
    MEMSET(&image_slide_setting,0x0,sizeof(IMAGE_SLIDE_INFO));
    if(NULL != sys_data)
    {
        MEMCPY(&image_slide_setting,&sys_data->image_slide_setting,sizeof(IMAGE_SLIDE_INFO));
    }

    if(image_slide_setting.u_time_gap == 0xFF)
    {        
        image_slide_setting.u_time_gap = 2;
        image_slide_setting.b_repeat= 1;
    }

    mp_title_type = TITLE_VIDEO;

    g_from_imageslide = FALSE;
    g_from_imagepreview = FALSE;
    g_from_mpegplayer = FALSE;
    g_from_copyfile = FALSE;

    for(i=0; i<WIN_FILELIST_NUMBER; i++)
    {
    //    latest_file_index[i] = (UINT16)(~0);
        usblst_set_latest_file_index((UINT16)(~0), i);
    //    latest_file_top[i] = (UINT16)(~0);
        usblst_set_latest_file_top((UINT16)(~0), i);
        usblst_set_filelist_handle(NULL, i);//win_file_list[i] = NULL;
    //    MEMSET(cur_device[i],0x0,4);
        usblst_set_cur_device(0x0, i);
    }
    for(i=0; i<WIN_PLAYLIST_NUMBER; i++)
    {
        win_play_list[i] = NULL;
    }

#ifdef SAVE_PLAYLIST_SUPPORT
    win_play_list[PL_MUSIC] = create_play_list(MUSIC_PLAY_LIST, t_name_music);
    read_play_list_from_disk(win_play_list[PL_MUSIC]);
    win_play_list[PL_IMAGE] = create_play_list(IMAGE_PLAY_LIST, t_name_image);
    read_play_list_from_disk(win_play_list[PL_IMAGE]);
#endif
}

void win_delete_all_filelist(void)
{
    UINT8 i = 0;
    file_list_handle t_filelist = NULL;

    for(i=0; i<WIN_FILELIST_NUMBER; i++)
    {
    //    latest_file_index[i] = (UINT16)(~0);
        usblst_set_latest_file_index((UINT16)(~0), i);
    //    latest_file_top[i] = (UINT16)(~0);
        usblst_set_latest_file_top((UINT16)(~0), i);
        t_filelist = usblst_get_filelist_handle(i);
        if(NULL != t_filelist)
        {
            delete_file_list(t_filelist);
            usblst_set_filelist_handle(NULL, i);//win_file_list[i] = NULL;
        }
    //    MEMSET(cur_device[i],0x0,4);
        usblst_set_cur_device(0x0, i);
    }
    for(i=0; i<WIN_PLAYLIST_NUMBER; i++)
    {
        if(win_play_list[i] != NULL)
        {
            delete_play_list(win_play_list[i]);
            win_play_list[i] = NULL;
        }
    }

}

BOOL file_list_check_storage_device(BOOL update, BOOL file_list_window)
{
    UINT8 now_udisk_cnt = 0;// = get_stroage_device_number(STORAGE_TYPE_ALL);
    UINT8 usb_dev_num = get_stroage_device_number(STORAGE_TYPE_USB);
    UINT8 sd_dev_num = get_stroage_device_number(STORAGE_TYPE_SD);
    UINT8 hdd_dev_num = get_stroage_device_number(STORAGE_TYPE_HDD);
    UINT8 i = 0;
    file_list_handle t_filelist = NULL;
    char *t_cur_device = NULL;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    t_type = usblst_get_mp_filelist_type();
    t_cur_device = usblst_get_cur_device(t_type);

    now_udisk_cnt = hdd_dev_num + sd_dev_num +usb_dev_num;
    if(0 == now_udisk_cnt)
    {
        for(i=0; i<WIN_FILELIST_NUMBER; i++)
        {
        //    latest_file_index[i] = (UINT16)(~0);
            usblst_set_latest_file_index((UINT16)(~0), i);
        //    latest_file_top[i] = (UINT16)(~0);
            usblst_set_latest_file_top((UINT16)(~0), i);
            t_filelist = usblst_get_filelist_handle(i);
            if(NULL != t_filelist)
            {
                delete_file_list(t_filelist);
                usblst_set_filelist_handle(NULL, i);// win_file_list[i] = NULL;
            }
            //MEMSET(cur_device[i],0x0,4);
            usblst_set_cur_device(0x0, i);
        }
        for(i=0; i<WIN_PLAYLIST_NUMBER; i++)
        {
            if(win_play_list[i] != NULL)
            {
                delete_play_list(win_play_list[i]);
                win_play_list[i] = NULL;
            }
        }

        ap_send_key(V_KEY_MENU,TRUE);

        return FALSE;
    }
    else if(((0 == strncmp(t_cur_device, "ud", 2)) && (0 == usb_dev_num)) ||
        ((0 == strncmp(t_cur_device, "hd", 2)) &&  (0 == hdd_dev_num)) ||
        ((0 == strncmp(t_cur_device, "sd", 2)) && (0 == sd_dev_num)))
    {
        usblst_change_storage(VACT_FILELIST_CHG_DEVR, FALSE);
        if(file_list_window)
        {
            // need update window.
            if(update)
            {
                osd_set_container_focus(&g_win_usb_filelist, MP_OBJLIST_ID);
                osd_draw_object((POBJECT_HEAD)&usb_dev_con,C_UPDATE_ALL);
                osd_track_object((POBJECT_HEAD)&usblst_olist, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
                display_file_info();
            }
        }
        else
        {
            // need exit current window.
            ap_send_key(V_KEY_MENU,TRUE);
        }

    }
    return TRUE;
}

#ifdef DISK_MANAGER_SUPPORT
int mp_get_cur_disk_name(char *name, int len)
{
    const UINT8 disk_name_len = 4;
    char *t_cur_device = NULL;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    if (len < disk_name_len)
    {
        return -1;
    }

    t_type = usblst_get_mp_filelist_type();
    t_cur_device = usblst_get_cur_device(t_type);
    strncpy(name, t_cur_device, len-1);
    name[len-1] = 0;
    return 0;
}

BOOL mp_in_win_filelist(void)
{
    sys_state_t system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
    if ((SYS_STATE_USB_MP == system_state) && (mp_title_type != TITLE_RECORD))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

void win_delete_filelist(UINT32 dev_id)
{
    UINT8 i = 0;
    file_list_handle t_filelist = NULL;
    char *t_cur_device = NULL;

    for(i=0; i<WIN_FILELIST_NUMBER; i++)
    {
        t_cur_device = usblst_get_cur_device(i);
        if (dev_id == (UINT32)(t_cur_device[2] - 'a'))
        {
        //    latest_file_index[i] = (UINT16)(~0);
            usblst_set_latest_file_index((UINT16)(~0), i);
        //    latest_file_top[i] = (UINT16)(~0);
            usblst_set_latest_file_top((UINT16)(~0), i);
            t_filelist = usblst_get_filelist_handle(i);
            if(NULL != t_filelist)
            {
                delete_file_list(t_filelist);
                usblst_set_filelist_handle(NULL, i);// win_file_list[i] = NULL;
            }
            usblst_set_cur_device(0x0, i);//MEMSET(cur_device[i],0x0,4);
        }
    }
}

void win_delete_playlist(void)
{
    UINT8 i = 0;

    for(i=0; i<WIN_PLAYLIST_NUMBER; i++)
    {
        if(win_play_list[i] != NULL)
        {
            delete_play_list(win_play_list[i]);
            win_play_list[i] = NULL;
        }
    }
}

#ifdef DLNA_DMP_SUPPORT
//the 2 following use for DLNA media play
void mp_set_dlna_play(BOOL bflag)
{
    m_dlna_play_flag = bflag;
    if (!bflag)
    {
        dlna_player_set_uri(NULL);
    }
}

BOOL mp_is_dlna_play()
{
    return m_dlna_play_flag;
}

//extern void mp_apcallback(UINT32 event_type,UINT32 param);

void mp_dlna_neterror_proc()
{
    media_player_release();
    //libc_printf("release media player\n");
    mp_set_dlna_play(FALSE);

    //libc_printf("disk detach in Media Player, enter main menu\n");
    // Alert here:Clear all menus may cuase unpredictable result,must be tested
    api_osd_mode_change(OSD_NO_SHOW);
    ap_clear_all_menus(); //clear all menu & Hidden osd show
    menu_stack_pop_all();

    win_delete_all_filelist();
    dmp_remove_all_dms();

    image_restore_vpo_rect();
    ap_clear_all_message();
    api_set_system_state(SYS_STATE_NORMAL);
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&g_win_mainmenu, FALSE);

    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NETERROR,1, FALSE);

    // Yes, we need init player again, because it maybe cause USB play error
    win_media_player_init((mp_callback_func)(mp_apcallback));
}
#endif

#endif
