/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_act_proc.c
*
*    Description:   Some functions of the realization of filelist
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#include <basic_types.h>
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>

#include "win_com_popup.h"
#include "string.id"
#include "images.id"

#include "copper_common/com_api.h"
#include "win_com.h"
#include "menus_root.h"

#include "win_filelist.h"
#include "win_filelist_display.h"
#include "win_filelist_inner.h"
#include "win_filelist_act_proc.h"
#include "win_mp_move_file.h"
#include "win_playlist.h"
#include "media_control.h"
#include "win_mute.h"
#include "win_rename.h"
#include "win_imagepreview.h"
#include "ctrl_key_proc.h"
#include "control.h"
#include "win_imageslide.h"
#include "win_copying.h"
#ifdef MP_SPECTRUM
#include "win_mpspectrum.h"
#endif
#include "gaui/win2_com.h"

static ID mp_mplayer_id = OSAL_INVALID_ID;

PRESULT     usblst_pop_playlist(void)
{
    PRESULT ret = PROC_LOOP;
#if (!defined(HW_SECURE_ENABLE) && (!defined(REMOVE_SPECIAL_MENU)))
    UINT8 back_saved = 0;
    WIN_PLAYLIST_TYPE playlist_type = 0;
    UINT32 file_num = 0;
    play_list_handle t_play_list = NULL;

    switch(win_filelist_get_mp_title_type())
    {
    case TITLE_MUSIC:
        playlist_type = PL_MUSIC;
        break;
    case TITLE_IMAGE:
        playlist_type = PL_IMAGE;
        break;
    default:
        playlist_type = PL_MUSIC;
        break;
    }

    if (NULL != win_filelist_get_playlist(playlist_type))//(win_play_list[playlist_type] != NULL)
    {
        t_play_list = win_filelist_get_playlist(playlist_type);
        get_play_list_info(t_play_list,(unsigned int *)&file_num, (play_list_loop_type *)NULL);
    }

    if ((NULL == win_filelist_get_playlist(playlist_type)) || ((NULL != win_filelist_get_playlist(playlist_type)) &&
        (0 ==file_num)))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        #ifndef SD_UI
        win_compopup_set_frame(496, 330, 286, 130);
        #else
        win_compopup_set_frame(200, 330, 286, 130);
        #endif
        win_compopup_set_msg(NULL, NULL,RS_MP_PLAYLIST_UNEXIST);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
    }
    else
    {
        win_playlist_open(playlist_type);
        osd_track_object((POBJECT_HEAD)&usblst_olist, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
    }
#endif
    return ret ;
}

PRESULT usblist_fav_playlist(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    UINT8 back_saved = 0;
    POBJLIST pol = NULL;
    UINT32 msg_code = 0; // construct virtual operation
    UINT16 ucurpos = 0;
    UINT16 idx_playlist = 0;
    __MAYBE_UNUSED__ INT16 idx_playlist_is_not = 0;
    media_file_info file_node;
    RET_CODE retcode = RET_SUCCESS;
    BOOL broot = FALSE;
    play_list_handle play_list = NULL;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    UINT8 fav_all = 0;
    const UINT8 favlist_inited = 1;
    const UINT8 favlist_uninit = 0;
    file_list_handle t_cur_filelist = NULL;
    play_list_handle t_play_list = NULL;
    const char *t_name_music = usblst_get_list_name_music();
    const char *t_name_image = usblst_get_list_name_image();

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);
    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    broot = win_check_openroot(t_cur_filelist);
    if(broot == TRUE)
    {
        ucurpos++;
    }

    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
        if (NULL == win_filelist_get_playlist(PL_MUSIC))//(win_play_list[PL_MUSIC] == NULL)
        {
            t_play_list = create_play_list(MUSIC_PLAY_LIST, t_name_music);
            win_filelist_set_playlist(t_play_list, PL_MUSIC);
            if (NULL == win_filelist_get_playlist(PL_MUSIC))//(win_play_list[PL_MUSIC] == NULL)
            {
              //if (NULL != full_name)
              //{
                FREE(full_name);
                full_name = NULL;
              //}
                return ret;
            }
        }
        play_list = win_filelist_get_playlist(PL_MUSIC);//win_play_list[PL_MUSIC];
        win_filelist_set_musiclist_changed(TRUE);
    }
    else if(TITLE_IMAGE == win_filelist_get_mp_title_type())
    {
        if (NULL == win_filelist_get_playlist(PL_IMAGE))//(win_play_list[PL_IMAGE] == NULL)
        {
            t_play_list = create_play_list(IMAGE_PLAY_LIST, t_name_image);
            win_filelist_set_playlist(t_play_list, PL_IMAGE);
            if (NULL == win_filelist_get_playlist(PL_IMAGE))//(win_play_list[PL_IMAGE] == NULL)
            {
             // if (NULL != full_name)
              //{
                FREE(full_name);
                full_name = NULL;
             // }
                return ret;
            }
        }
        play_list = win_filelist_get_playlist(PL_IMAGE);//win_play_list[PL_IMAGE];
        win_filelist_set_imagelist_changed(TRUE);
    }
    else
    {
      //if (NULL != full_name)
      //{
        FREE(full_name);
        full_name = NULL;
      //}
        return ret;
    }
    if(favlist_uninit == usblst_get_fav_ini_flag())
    {
        t_cur_filelist = win_filelist_get_cur_filelist();
        check_files_in_play_list(t_cur_filelist, play_list, (char *)&fav_all);
        usblst_set_fav_ini_flag(favlist_inited);
    }

    switch(act)
    {
        case VACT_FILELIST_FAV_ALL:
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(200, 200, 200, 100);
            win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
            win_compopup_open_ext(&back_saved);
            t_cur_filelist = win_filelist_get_cur_filelist();
            check_files_in_play_list(t_cur_filelist, play_list, (char *)&fav_all);
            if(FALSE == fav_all)
            {
                retcode = add_all_file_list_items_to_play_list(play_list, t_cur_filelist);
            }
            else
            {
                retcode = delete_all_file_list_items_in_play_list(play_list, t_cur_filelist);
            }
            osal_task_sleep(500);
            win_compopup_smsg_restoreback();
            osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
            break;

        case VACT_FILELIST_FAV:
            t_cur_filelist = win_filelist_get_cur_filelist();
            get_file_from_file_list(t_cur_filelist, ucurpos, &file_node);
            if(TITLE_MUSIC == win_filelist_get_mp_title_type())
            {
                if((file_node.filetype != F_MP3) && (file_node.filetype != F_BYE1) && (file_node.filetype != F_OGG)
                    && (file_node.filetype != F_FLC)&& (file_node.filetype != F_WAV))
                {
                    break;
                }
            }
            else
            {
                if((file_node.filetype != F_JPG) && (file_node.filetype != F_BMP))
                {
                    break;
                }
            }

#ifdef SD_UI
            //sd_ui don't use the in_play_list.
            win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
            idx_playlist_is_not = get_play_list_index_by_name(play_list, full_name);
            if(idx_playlist_is_not >= 0)
            {
                idx_playlist = idx_playlist_is_not;
                retcode = delete_play_list_item(play_list, idx_playlist);
            }
#else
            if(file_node.in_play_list)  //already in fav
            {
                win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
                idx_playlist = get_play_list_index_by_name(play_list, full_name);
                if(idx_playlist < 1)
                {
                    PRINTF("error index!\n");
                }
                t_cur_filelist = win_filelist_get_cur_filelist();
                retcode = del_play_list_item_from_file_list(t_cur_filelist,ucurpos,play_list, idx_playlist);
            }
#endif
            else
            {
                t_cur_filelist = win_filelist_get_cur_filelist();
                retcode = add_file_to_play_list_from_file_list(play_list, t_cur_filelist,ucurpos);
            }
            if(!broot && (ucurpos == pol->w_count-1))
            {//current is last file and not root
                if(2 == pol->w_count)
                {//only one file
                    osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
                }
                else
                {
                    osd_change_focus((POBJECT_HEAD)pol,1,C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                }
            }
            else
            {
                ap_vk_to_hk(0, V_KEY_DOWN, &msg_code);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
            }
            break;
        default:
            break;
    }

    if(retcode == RET_STA_ERR) //playlist full
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_MSG_SPACE_FULL);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
    }

    //if (NULL != full_name)
    //{
        FREE(full_name);
    //}
    return ret;
}

#ifdef MP_SPECTRUM
extern BOOL g_preview_spectrum_enable;
#endif

PRESULT usblist_enter_edit(void)
{
    PRESULT ret = PROC_LOOP;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    UINT32 *t_del_fl_idx = NULL;
    ID t_id = OSAL_INVALID_ID;

    usblst_set_edit_flag(1);
    t_del_fl_idx = usblst_get_del_filelist_idx();
    win_clear_del_flags(t_del_fl_idx, /*sizeof(del_filelist_idx)*/FILELIST_DELARRAY_MAX*4);

    //stop music play
    music_stop();
    t_id = win_filelist_get_mp_refresh_id();
    api_stop_timer(&t_id);
    win_filelist_set_mp_refresh_id(t_id);
    win_filelist_set_mp_play_state(MUSIC_STOP_STATE);

    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
#ifdef MP_SPECTRUM
        close_spectrum_dev();
        g_preview_spectrum_enable = FALSE;
#endif

        set_mplayer_display(MP_STOP_ID);
        draw_mplayer_display();
    }

    //stop video play
    if(TITLE_VIDEO == win_filelist_get_mp_title_type())
    {
        usblst_mp_close_video();

#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
        if (usblst_get_previewing_mpeg())//if(g_previewing_mpeg)
        {
            mpg_cmd_stop_proc(0);
            osal_task_sleep(300);//waite mpeg decoder task exit and vdec stop
            api_full_screen_play();
            set_mplayer_display_video();
            draw_mplayer_display();
        }
        usblst_set_previewing_mpeg(FALSE);//g_previewing_mpeg = FALSE;
#endif
    }

    if((TITLE_MUSIC == win_filelist_get_mp_title_type()) || (TITLE_VIDEO == win_filelist_get_mp_title_type()))
    {
        display_file_info_preview();
    }

    //hide music player bar & buttons
    if((TITLE_MUSIC == win_filelist_get_mp_title_type()) || (TITLE_VIDEO == win_filelist_get_mp_title_type()))
    {
        osd_set_objp_next(&usb_file_info, NULL);
        osd_hide_object((POBJECT_HEAD)&usb_music_con, C_UPDATE_ALL);
    }

    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_title(0,m_title_type);
    //win_draw_mp_pvr_title();
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_help(0, m_title_type);
   // win_draw_mp_pvr_help();

    return ret;
}

static void win_musicplayer_replay(void)
{//replay current music
    TEXT_FIELD *ptxt = NULL;
    char str_buf[20] = {0};
    int ret_val = 0;
    char *t_name = NULL;
    UINT16 t_per = 0;
    ID t_id = OSAL_INVALID_ID;

    win_filelist_set_mp_play_state(MUSIC_PLAY_STATE);

    music_stop();

    usblst_set_mp_play_time(0);
    usblst_set_mp_play_per(0);
    t_per = usblst_get_mp_play_per();
    osd_set_progress_bar_pos(&mplayer_bar,t_per);

    ptxt = &mplayer_curtime;
    snprintf(str_buf, 20, "%s", "00:00:00");
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buf);
    osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
#ifdef DLNA_DMP_SUPPORT
    if(mp_is_dlna_play())
    {
        t_name = usblst_get_mp_curmusic_name();
        win_dlna_play_preset(t_name, 1);
        dmp_music_set_pcm_config(t_name);
    }
#endif

    t_id = win_filelist_get_mp_refresh_id();
    api_stop_timer(&t_id);
    win_filelist_set_mp_refresh_id(t_id);
    t_id = api_start_cycletimer("M",200,usblst_mprefresh_handler);
    win_filelist_set_mp_refresh_id(t_id);
    t_name = usblst_get_mp_curmusic_name();
    ret_val = music_play(t_name);
    if(ret_val < 0)//can not play mp3
    {
        osal_task_sleep(200);//delay to avoid flicker of unregonized MP3
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MP3OVER,0,TRUE);
    }
}

void win_filelist_delfiles(void)
{
    UINT8 back_saved = 0;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    RET_CODE func_ret = RET_SUCCESS;
    ID timer_id = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    file_list_handle t_cur_filelist = NULL;
    play_list_handle t_play_list = NULL;
    UINT32 *t_del_flidx = NULL;

	if(RET_SUCCESS == func_ret)
	{
		;
	}
	timer_id = win_compopup_start_send_key_timer(V_KEY_EXIT, 10000, TIMER_ALARM);

    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_msg(NULL, NULL, RS_DISPLAY_SURE_TO_DELETE);
    choice = win_compopup_open_ext(&back_saved);
    api_stop_timer(&timer_id);
    if(WIN_POP_CHOICE_YES == choice)
    {
        //stop image play before delete files
        if(TITLE_IMAGE == win_filelist_get_mp_title_type())
        {
            usblst_set_preview_txt_enable(0);
            image_abort();
        }

        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        m_title_type = win_filelist_get_mp_title_type();
        t_cur_filelist = win_filelist_get_cur_filelist();
        t_play_list = win_filelist_get_playlist(m_title_type);
        t_del_flidx = usblst_get_del_filelist_idx();
        func_ret = delete_files_from_file_list_ext(t_cur_filelist, (unsigned int *)t_del_flidx,
          FILELIST_DELARRAY_MAX,t_play_list);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
    }
    t_del_flidx = usblst_get_del_filelist_idx();
    win_clear_del_flags(t_del_flidx, /*sizeof(del_filelist_idx)*/FILELIST_DELARRAY_MAX*4);
}

PRESULT usblist_exit_edit(void)
{
    PRESULT ret = PROC_LOOP;
    POBJLIST pol = NULL;
    UINT8 broot = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    UINT32 *t_del_flidx = NULL;

    usblst_set_edit_flag(0);

    //save edit changes when exit edit
    t_del_flidx = usblst_get_del_filelist_idx();
    if(win_check_del_flags(t_del_flidx, usblst_olist.w_count))
    {
        win_filelist_delfiles();
        pol  = &usblst_olist;
        t_cur_filelist = win_filelist_get_cur_filelist();
        t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, NULL, 0);
        if (RET_SUCCESS == t_ret)
        {
            usblst_set_dirnum(t_dirnum);
            usblst_set_filenum(t_filenum);
        }

        broot = win_check_openroot(t_cur_filelist);
        usblst_set_cur_fileidx(0);
        t_fileidx = usblst_get_cur_fileidx();
        osd_set_obj_list_cur_point(pol, t_fileidx);
        osd_set_obj_list_new_point(pol, t_fileidx);
        osd_set_obj_list_top(pol, t_fileidx);
        if(broot == TRUE)
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum));//to show it's child dir
        }
        else
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum + 1));//to show it's child dir
        }

        if(MP_DEVICE_ID == osd_get_focus_id((POBJECT_HEAD)&g_win_usb_filelist))
        {
            osd_draw_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        }
        else
        {
            osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        }
    }
    display_file_info();

    //show music player bar & buttons if in music page
    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
        osd_set_objp_next(&usb_file_info, &usb_music_con);

        osd_set_progress_bar_pos(&mplayer_bar,0);
        osd_set_text_field_content(&mplayer_curtime,STRING_ANSI,(UINT32)("00:00:00"));
        osd_draw_object((POBJECT_HEAD)&usb_music_con, C_UPDATE_ALL);
    }

    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_title(1,m_title_type);
   // win_draw_mp_pvr_title();
    m_title_type = win_filelist_get_mp_title_type();
    win_set_mp_pvr_help(1, m_title_type);
   // win_draw_mp_pvr_help();

    return ret;
}

PRESULT     usblst_sort(void)
{
    PRESULT ret = PROC_LOOP;
    COM_POP_LIST_PARAM_T param;
    OSD_RECT rect;
    UINT16 sel = 0;
    UINT8 sort_flag = 0;
    POBJLIST pol = NULL;
    const UINT8 sel_2nd = 2;
    const UINT8 sel_3rd = 3;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_fileidx = 0;

    MEMSET(&param, 0x0, sizeof(COM_POP_LIST_PARAM_T));
    MEMSET(&rect, 0x0, sizeof(OSD_RECT));
    param.cur = 0xFFFF;
    param.selecttype = POP_LIST_SINGLESELECT;
    osd_set_rect(&rect,SORTSET_L, SORTSET_T, SORTSET_W,SORTSET_H);
    if(TITLE_VIDEO == win_filelist_get_mp_title_type())
    {
        sel = win_com_open_sub_list(POP_LIST_TYPE_VIDEOSORTSET,&rect,&param);
    }
    else
    {
        sel = win_com_open_sub_list(POP_LIST_TYPE_MPSORTSET,&rect,&param);
    }

    if(0 == sel)
    {
        sort_flag = SORT_NAME;//Alphabetic
    }
    else if(1 == sel)
    {
        sort_flag = SORT_DATE;
    }
    else if(sel == sel_2nd)
    {
        sort_flag = SORT_SIZE;
    }
    else if(sel == sel_3rd)
    {
        usblist_fav_playlist(0xFF);//fixed bug 49348.
        sort_flag = SORT_FAV;//Favorite
    }
    else
    {
        sort_flag = 0xFF;
    }
    if(sort_flag != 0xFF)
    {
        t_cur_filelist = win_filelist_get_cur_filelist();
        sort_file_list(t_cur_filelist, sort_flag);

        pol = &usblst_olist;
        usblst_set_cur_fileidx(0);
        t_fileidx = usblst_get_cur_fileidx();
        osd_set_obj_list_cur_point(pol, t_fileidx);
        osd_set_obj_list_new_point(pol, t_fileidx);
        osd_set_obj_list_top(pol, t_fileidx);

        osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        display_file_info();
    }

    return ret;
}

PRESULT     usblst_repeat_mode(void)
{
    PRESULT ret = PROC_LOOP;
    play_list_loop_type lp_mode = PLAY_LIST_NONE;
    play_list_loop_type tmp_val = PLAY_LIST_NONE;
    file_list_handle t_cur_filelist = NULL;
    UINT8 loop_flag = 0;
    play_list_handle t_playlist_music = NULL;

    if(TITLE_MUSIC == win_filelist_get_mp_title_type())
    {
        lp_mode = win_filelist_get_loop_mode();
        tmp_val = lp_mode + 1 % MP3_PLAY_MODE_CNT;
        win_filelist_set_loop_mode(tmp_val);

        if((pl_ay_list_sequence == win_filelist_get_loop_mode()) || (PLAY_LIST_NONE == win_filelist_get_loop_mode()))
        {
        #ifdef DLNA_DMP_SUPPORT
            //DLNA need music none play mode
            if (mp_is_dlna_play())
            {
                if (pl_ay_list_sequence == win_filelist_get_loop_mode())
                {
                    win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
                }
            }
            else
            {
                win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
            }
        #else
            win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
        #endif
        }
        loop_flag = usblst_get_music_folderloop_flag();
        display_loopmode(loop_flag);
        if(usblst_get_cur_playlist_music() != NULL)
        {
            t_playlist_music = usblst_get_cur_playlist_music();
            tmp_val = win_filelist_get_loop_mode();
            set_play_list_info(t_playlist_music, NULL, &tmp_val);
            win_filelist_set_loop_mode(tmp_val);
        }
    }
    else if(TITLE_VIDEO == win_filelist_get_mp_title_type())
    {
        if(PLAY_LIST_REPEAT == win_filelist_get_loop_mode())
        {
            win_filelist_set_loop_mode(PLAY_LIST_ONE);
        }
        else if(PLAY_LIST_ONE == win_filelist_get_loop_mode())
        {
            win_filelist_set_loop_mode(PLAY_LIST_NONE);
        }
        else
        {
            win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
        }

        display_loopmode(1);
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(NULL != t_cur_filelist)
        {
            tmp_val = win_filelist_get_loop_mode();
            set_play_list_info(t_cur_filelist, NULL, &tmp_val);
            win_filelist_set_loop_mode(tmp_val);
        }
    }
#ifdef DLNA_DMP_SUPPORT
    else if((TITLE_IMAGE == win_filelist_get_mp_title_type()) && mp_is_dlna_play())
    {
        if(PLAY_LIST_REPEAT == win_filelist_get_loop_mode())
        {
            win_filelist_set_loop_mode(PLAY_LIST_ONE);
        }
        else if(PLAY_LIST_ONE == win_filelist_get_loop_mode())
        {
            win_filelist_set_loop_mode(PLAY_LIST_NONE);
        }
        else
        {
            win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
        }

        display_loopmode(1);
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(NULL != t_cur_filelist)
        {
            tmp_val = win_filelist_get_loop_mode();
            set_play_list_info(t_cur_filelist, NULL, &tmp_val);
            win_filelist_set_loop_mode(tmp_val);
        }
    }
#endif

    return ret;
}

PRESULT     usblst_multiview(void)
{
    PRESULT ret = PROC_LOOP;
    POBJLIST pol = NULL;
    UINT16 u_curpos = 0;
    BOOL broot = FALSE;
    POBJECT_HEAD mplayer_menu = NULL;
    file_list_handle t_cur_filelist = NULL;
    play_list_handle t_play_list = NULL;
    UINT16 t_val = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    pol = &usblst_olist;
    u_curpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    broot = win_check_openroot(t_cur_filelist);
    if(!broot)
    {
        if(u_curpos > 0)
        {
            u_curpos--;
        }
    }

    t_val = osd_get_obj_list_top(pol);
    t_type = usblst_get_mp_filelist_type();
    usblst_set_latest_file_top(t_val, t_type);
    t_val = u_curpos + (broot ? 0 : 1);
    t_type = usblst_get_mp_filelist_type();
    usblst_set_latest_file_index(t_val, t_type);
    usblst_set_preview_txt_enable(0);// g_preview_decode_txt_enable = 0;
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    mpg_cmd_stop_proc(0);
#endif
    image_abort();

    t_play_list = win_filelist_get_playlist(PL_FOLDER_IMAGE);
    win_set_imageplay_param(&t_play_list, u_curpos+1, 1);
    win_set_image_multiview_firstidx();
    mplayer_menu = (POBJECT_HEAD)(&g_win_imagepreview);
    //if(mplayer_menu != NULL)
    //{
        osd_obj_close((POBJECT_HEAD)(&g_win_usb_filelist),C_CLOSE_CLRBACK_FLG);
        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
        }
    //}

    return ret;
}

static void usblist_mplayer_handler(UINT32 nouse)
{
    api_stop_timer(&mp_mplayer_id);
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MPLAYER_DISPLAY,0,FALSE);
}

PRESULT usblist_music_player_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    int next_idx = 0;
    UINT8 bfocusid = MP_NO_FOCUS;
    const UINT8 play_time = 3;
    play_list_handle t_playlist_music = NULL;
    int t_time = 0;

    switch(act)
    {
        case VACT_FILELIST_PREV:
        case VACT_FILELIST_NEXT:
            if(act == VACT_FILELIST_NEXT)
            {
                bfocusid = MP_NXT_ID;
                t_playlist_music = usblst_get_cur_playlist_music();
                next_idx = get_next_index_from_play_list(t_playlist_music);
            }
            else
            {
                bfocusid = MP_PRV_ID;
                t_time = music_get_time();
                usblst_set_mp_play_time(t_time);
                if(usblst_get_mp_play_time() <= play_time)
                {
                    t_playlist_music = usblst_get_cur_playlist_music();
                    next_idx = get_previous_index_from_play_list(t_playlist_music);
                }
                else
                {
                    next_idx = usblst_get_cur_music_idx();//cur_music_idx;
                }
            }

            if(next_idx != -1)
            {
                if(next_idx == usblst_get_cur_music_idx())
                {
                    win_musicplayer_replay();
                }
                else
                {
                    usblst_set_cur_music_idx(next_idx);
                    usblst_musicplayer_play();
                }
            }
            break;
        case VACT_FILELIST_XBACK:
        case VACT_FILELIST_XFORWARD:
            if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
            {
                music_pause();
                if(!get_mute_state())
                {
                    api_audio_set_mute(FALSE);
                }
            }
            else if(MUSIC_STOP_STATE == win_filelist_get_mp_play_state())
            {
                bfocusid = MP_STOP_ID;
                break;
            }
            if(VACT_FILELIST_XBACK == act)
            {
                bfocusid = MP_XBACK_ID;
                //api_audio_set_mute(TRUE);
                music_seek(-3);
                osal_task_sleep(100);
                //api_audio_set_mute(FALSE);
                if(get_mute_state())
                {
                   set_mute_on_off(TRUE);
                }
            }
            else
            {
                bfocusid = MP_XFORWARD_ID;
                //api_audio_set_mute(TRUE);
                music_seek(3);
                osal_task_sleep(100);
                //api_audio_set_mute(FALSE);
                if(get_mute_state())
                {
                   set_mute_on_off(TRUE);
                }
            }
            win_filelist_set_mp_play_state(MUSIC_PLAY_STATE);
            break;
        case VACT_FILELIST_PAUSE:
            bfocusid = MP_PAUSE_ID;
            if(MUSIC_PLAY_STATE == win_filelist_get_mp_play_state())
            {
                music_pause();
                win_filelist_set_mp_play_state(MUSIC_PAUSE_STATE);
                api_audio_set_mute(TRUE);
                set_mplayer_display(bfocusid);
                draw_mplayer_display();
                //osal_task_sleep(1600);
                //if(!GetMuteState())
                    //api_audio_set_mute(FALSE);
            }
            else
            {
                if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
                {
                    music_pause();
                    if(!get_mute_state())
                    {
                        api_audio_set_mute(FALSE);
                    }
                    win_filelist_set_mp_play_state(MUSIC_PLAY_STATE);
                }
                else
                {
                    bfocusid = MP_NO_FOCUS;
                    break;
                }
            }
            break;
        case VACT_FILELIST_PLAY:
            if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
            {
                music_pause();
                if(!get_mute_state())
                {
                    api_audio_set_mute(FALSE);
                }
            }
            else if((MUSIC_STOP_STATE == win_filelist_get_mp_play_state())
               ||(MUSIC_IDLE_STATE == win_filelist_get_mp_play_state()))
            {
                win_musicplayer_replay();
#ifdef MP_SPECTRUM
                if(TITLE_MUSIC == win_filelist_get_mp_title_type())
                {
                    open_spectrum_dev();
                    g_preview_spectrum_enable = TRUE;
                    osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
                    if(get_mute_state())
                    {
                        update_bars(0);
                    }
                }
#endif
            }
            win_filelist_set_mp_play_state(MUSIC_PLAY_STATE);
            break;
        case VACT_FILELIST_STOP:
            bfocusid = MP_STOP_ID;
            if(win_filelist_get_mp_play_state() != MUSIC_STOP_STATE)
            {
                music_stop();
                if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())//set mute again when pause a music
                {
                    if(!get_mute_state())
                    {
                        api_audio_set_mute(FALSE);
                    }
                }
                win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
#ifdef MP_SPECTRUM
                if(win_filelist_get_mp_title_type() == TITLE_MUSIC)
                {
                    close_spectrum_dev();
                    g_preview_spectrum_enable = FALSE;
                    display_file_info_preview();
                }
#endif
            }
            break;
        default:
            break;
    }

    set_mplayer_display(bfocusid);
    draw_mplayer_display();
    api_stop_timer(&mp_mplayer_id);
    mp_mplayer_id = api_start_timer("M",500,usblist_mplayer_handler);

    return ret;
}

PRESULT usblist_video_player_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    POBJLIST pol = NULL;
    UINT16 ucurpos = 0;
    media_file_info file_node;
    char *full_name = NULL;//[FULL_PATH_SIZE] = {0};
    int ret_val = 0;
    char str[20] = {0};
    file_list_handle t_cur_filelist = NULL;
    ID t_id = OSAL_INVALID_ID;

	if(0 == ret_val)
	{	
		;
	}
	MEMSET(&file_node, 0x0, sizeof(media_file_info));
    full_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == full_name)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(full_name, 0x0, FULL_PATH_SIZE);
    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    if(win_check_openroot(t_cur_filelist))
    {
        ucurpos++;
    }

    switch(act)
    {
        case VACT_FILELIST_PAUSE:
            if(usblst_get_video_play_state() != MPEG_STOP)
            {
                mpg_cmd_pause_proc();
                usblst_set_video_play_state(MPEG_PAUSE);
                set_mplayer_display_video();
                osd_set_bitmap_content(&mplayer_pause, IM_N_MEDIAPLAY_CONTROL_PAUSE_HI);
                draw_mplayer_display();
            }
            break;
        case VACT_FILELIST_PLAY:
            if(usblst_get_video_play_state() != MPEG_PLAY)
            {
        //        wincom_open_preview(NULL, FALSE,FALSE);
                if(0==mpg_cmd_play_proc())
                {
                    usblst_set_video_play_state(MPEG_PLAY);
                    set_mplayer_display_video();
                    osd_set_bitmap_content(&mplayer_xback, IM_N_MEDIAPLAY_CONTROL_PLAY_HI);
                    draw_mplayer_display();
                //    api_stop_timer(&mp_refresh_id);
                    t_id = win_filelist_get_mp_refresh_id();
                    api_stop_timer(&t_id);
                    win_filelist_set_mp_refresh_id(t_id);
                //    mp_refresh_id = api_start_cycletimer("V",200,usblist_mprefresh_handler);
                    t_id = api_start_cycletimer("V",200,usblst_mprefresh_handler);
                    win_filelist_set_mp_refresh_id(t_id);
                }
            }
            break;
        case VACT_FILELIST_STOP:
            if (usblst_get_video_play_state() != MPEG_STOP)
            {
                t_id = win_filelist_get_mp_refresh_id();
                api_stop_timer(&t_id);
                mpg_cmd_stop_proc(0);

                t_cur_filelist = win_filelist_get_cur_filelist();
                ret = get_file_from_file_list(t_cur_filelist, ucurpos, &file_node);
                win_get_fullname(full_name, FULL_PATH_SIZE, file_node.path, file_node.name);
                ret_val = video_dec_file(full_name, 1);

                usblst_set_video_play_state(MPEG_STOP);
                set_mplayer_display_video();
                osd_set_bitmap_content(&mplayer_stop, IM_N_MEDIAPLAY_CONTROL_STOP_HI);
                draw_mplayer_display();

                snprintf(str,20, "%02d:%02d:%02d", 0, 0, 0);
                osd_set_text_field_content(&mplayer_curtime,STRING_ANSI,(UINT32)str);
                osd_draw_object((POBJECT_HEAD)&mplayer_curtime,C_UPDATE_ALL);
                osd_set_progress_bar_pos(&mplayer_bar,0);
                osd_draw_object((POBJECT_HEAD)&mplayer_bar,C_UPDATE_ALL);

                osal_task_sleep(500);
                mpg_cmd_pause_proc();
            }
            break;
        default:
            break;
    }

    if (NULL != full_name)
    {
        FREE(full_name);
    }
    return ret;
}

static void win_get_devicename_by_filename(char *device_name, int d_name_size, char *file_name)
{
    if(0 == STRCMP(file_name, FS_ROOT_DIR))
    {
        MEMSET(device_name, 0x0, d_name_size);
        strncpy(device_name, "", d_name_size - 1);
    }
    else
    {
        MEMCPY(device_name, file_name+5, 3);
        device_name[d_name_size-1] = '\0';
    }
}

void filelist_return2curmusic(void)
{
    BOOL broot = FALSE;
    char str_name[10] = {0};
    char str_txt[10] = {0};
    char *file_path = NULL;//[FULL_PATH_SIZE] = {0};
    char *music_path = NULL;//[FULL_PATH_SIZE] = {0};
    OBJLIST *pol = NULL;
    TEXT_FIELD *ptxt = NULL;
    UINT8 back_saved = 0;
    char *t_cur_device = NULL;
    file_list_handle t_cur_filelist = NULL;
    char *t_name = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    file_path = MALLOC(FULL_PATH_SIZE);
    if (NULL == file_path)
    {
        ASSERT(0);
        return;
    }
    music_path = MALLOC(FULL_PATH_SIZE);
    if (NULL == music_path)
    {
        FREE(file_path);
        ASSERT(0);
        return;
    }
    MEMSET(file_path, 0x0, FULL_PATH_SIZE);
    MEMSET(music_path, 0x0, FULL_PATH_SIZE);

    t_type = usblst_get_mp_filelist_type();
    t_cur_device = usblst_get_cur_device(t_type);
    t_name = usblst_get_mp_curmusic_name();
    win_get_devicename_by_filename(t_cur_device, 4, t_name);
    ptxt = &usb_dev_name;
    win_get_display_devicename(str_name, 10,t_cur_device);
    snprintf(str_txt,10,"%s",str_name);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_txt);//show current device name
    osd_draw_object((POBJECT_HEAD)&usb_dev_con,C_UPDATE_ALL);

    //get current filelist folder
    t_cur_filelist = win_filelist_get_cur_filelist();
    get_file_list_info(t_cur_filelist, NULL, NULL, file_path, FULL_PATH_SIZE);

    //get current music folder
    t_name = usblst_get_mp_curmusic_name();
    win_get_parent_dirname(music_path, FULL_PATH_SIZE, t_name);

    //check if same
    if(0 == STRCMP(file_path, music_path))//in same folder
    {
        pol = &usblst_olist;
        t_cur_filelist = win_filelist_get_cur_filelist();
        broot = win_check_openroot(t_cur_filelist);
        t_name = usblst_get_mp_curmusic_name();
    //    cur_fileidx = win_get_file_idx(t_cur_filelist, t_name) + (bRoot?0:1);
        t_fileidx = win_get_file_idx(t_cur_filelist, t_name) + (broot?0:1);
        usblst_set_cur_fileidx(t_fileidx);
        if(usblst_get_cur_fileidx() == osd_get_obj_list_cur_point(pol))
        {
            osd_track_object((POBJECT_HEAD)pol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
        }
        else
        {
            t_fileidx = usblst_get_cur_fileidx();
            osd_change_focus((POBJECT_HEAD)pol,t_fileidx,C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
        }
    }
    else
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        t_cur_filelist = win_filelist_get_cur_filelist();
        read_folder_to_file_list(t_cur_filelist, music_path);
        win_compopup_smsg_restoreback();

        t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, file_path, FULL_PATH_SIZE);
        if (RET_SUCCESS == t_ret)
        {
            usblst_set_dirnum(t_dirnum);
            usblst_set_filenum(t_filenum);
        }

        ptxt = &usb_dir_txt;
        win_get_display_dirname(str_txt,10, file_path);//get dirname to show
        osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

        pol  = &usblst_olist;
        broot = win_check_openroot(t_cur_filelist);
        t_name = usblst_get_mp_curmusic_name();
        //cur_fileidx = win_get_file_idx(t_cur_filelist, t_name) + (bRoot?0:1);
        t_fileidx = win_get_file_idx(t_cur_filelist, t_name) + (broot?0:1);
        usblst_set_cur_fileidx(t_fileidx);
        osd_set_obj_list_cur_point(pol, t_fileidx);
        osd_set_obj_list_new_point(pol, t_fileidx);
        osd_set_obj_list_top(pol, t_fileidx);
        if(broot == TRUE)
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum));//to show it's child dir
        }
        else
        {
            t_dirnum = usblst_get_dirnum();
            t_filenum = usblst_get_filenum();
            osd_set_obj_list_count(pol, (t_dirnum + t_filenum + 1));//to show it's child dir
        }

        osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        display_file_info();
    }

    FREE(file_path);
    FREE(music_path);
}

static void usblist_edit_copy(UINT16 ucurpos, BOOL broot)
{
    UINT8 back_saved = 0;
    win_popup_choice_t type_choice = WIN_POP_CHOICE_NULL;
    file_list_handle t_handle = NULL;
    char *t_cur_device = NULL;
    UINT16 t_val = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;
    POBJECT_HEAD win = NULL;

    //save the current file_list handle and source_index
    t_handle = usblst_get_filelist_handle(win_filelist_get_mp_title_type());
    set_copy_file_list(t_handle);
    set_copy_file_index(ucurpos);
    t_handle = NULL;
    //save end

    t_handle = usblst_get_filelist_handle(DIR_FILE_LIST);
    if(NULL != t_handle)
    {
        delete_file_list(t_handle);
        usblst_set_filelist_handle(NULL, DIR_FILE_LIST);//win_file_list[DIR_FILE_LIST] = NULL;
    }
    t_handle = create_file_list(DIR_FILE_LIST);
    usblst_set_filelist_handle(t_handle, DIR_FILE_LIST);

    t_type = usblst_get_mp_filelist_type();
    t_cur_device = usblst_get_cur_device(t_type);
    file_list_change_device(t_handle,t_cur_device);
    if(win_movefile_open(t_handle, t_cur_device))
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL,NULL,RS_MSG_ARE_YOU_SURE_TO_COPY);
        type_choice = win_compopup_open_ext(&back_saved);

        if(WIN_POP_CHOICE_YES == type_choice)
        {
            //to do copy
            t_val = osd_get_obj_list_top(&usblst_olist);
            t_type = usblst_get_mp_filelist_type();
            usblst_set_latest_file_top(t_val, t_type);
        //    latest_file_index[mp_filelist_type] = uCurpos - 1 + (bRoot ? 0 : 1);
            t_val = ucurpos - 1 + (broot ? 0 : 1);
            t_type = usblst_get_mp_filelist_type();
            usblst_set_latest_file_index(t_val, t_type);

            win = NULL;
            win = (POBJECT_HEAD)(&g_win_copying);
            if(osd_obj_open(win,(UINT32)(~0)) != PROC_LEAVE)
            {
                menu_stack_push(win);
            }
        }
    }
}

static void usblst_edit_proc_popup(char *str, char *unistr, UINT16 str_id)
{
    UINT8 back_saved = 0;

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
  #ifndef SD_UI
    win_compopup_set_frame(496, 330, 286, 130);
  #else
    win_compopup_set_frame(200, 330, 286, 130);
  #endif
    win_compopup_set_msg(str, unistr,str_id);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(1000);
    win_compopup_smsg_restoreback();
}

static void free_resource(UINT16 *unistr, char *old_name, char *new_name)
{
    if (NULL != unistr)
    {
        FREE(unistr);
        unistr = NULL;
    }
    if (NULL != old_name)
    {
        FREE(old_name);
        old_name = NULL;
    }
    if (NULL != new_name)
    {
        FREE(new_name);
        new_name = NULL;
    }
}

PRESULT usblist_edit_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    POBJLIST pol = NULL;
    UINT16 ucurpos = 0;
    media_file_info file_node;
    BOOL broot = 0;
    UINT8 *file_name = NULL;
    UINT16 *unistr = NULL;//[MAX_FILE_NAME_SIZE+1] = {0};
    char *old_name = NULL;//[MAX_FILE_NAME_SIZE] = {0};
    char *new_name = NULL;//[MAX_FILE_NAME_SIZE] = {0};
    char ext_name[10] = {0};
    UINT32 msg_code = 0; // construct virtual operation
    RET_CODE func_ret = RET_SUCCESS;
    int i = 0;
    int j = 0;
    int s_len = 0;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    UINT32 *t_del_flidx = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    unistr = (UINT16*)MALLOC((MAX_FILE_NAME_SIZE+1)*sizeof(UINT16));
    if (NULL == unistr)
    {
        ASSERT(0);
        return ret;
    }
    old_name = MALLOC(MAX_FILE_NAME_SIZE);
    if (NULL == old_name)
    {
        FREE(unistr);
        ASSERT(0);
        return ret;
    }
    new_name = MALLOC(MAX_FILE_NAME_SIZE);
    if (NULL == new_name)
    {
        FREE(unistr);
        FREE(old_name);
        ASSERT(0);
        return ret;
    }
    MEMSET(unistr, 0x0, (MAX_FILE_NAME_SIZE+1)*sizeof(UINT16));
    MEMSET(old_name, 0x0, MAX_FILE_NAME_SIZE);
    MEMSET(new_name, 0x0, MAX_FILE_NAME_SIZE);
    pol = &usblst_olist;
    ucurpos = osd_get_obj_list_cur_point(pol);
    t_cur_filelist = win_filelist_get_cur_filelist();
    broot = win_check_openroot(t_cur_filelist);

    switch(act)
    {
        case VACT_FILELIST_RENAME:
            if(0 == ucurpos)
            {
                break;
            }
            t_cur_filelist = win_filelist_get_cur_filelist();
            get_file_from_file_list(t_cur_filelist, ucurpos, &file_node);
            strncpy(old_name, file_node.name, MAX_FILE_NAME_SIZE-1);
            if(F_DIR == file_node.filetype)
            {
                strncpy(ext_name, "", (10-1));
            }
            else
            {
                for (j = 0; old_name[j] != '\0'; j++)
                {
                    ;
                }
                for (i = j - 1; (old_name[i] != '.') && (i >= 0); i--)
                {
                    ;
                }

                if(i < 0)
                {
                    strncpy(ext_name, "", (10-1));
                }
                else
                {
                    strncpy(ext_name, old_name+i, (10-1));
                    old_name[i] = '\0';
                }
            }
            com_utf8str2uni((UINT8 *)old_name,unistr);
            if(win_recrename_open(unistr, &file_name))
            {
                if(com_uni_str_cmp(unistr, (const UINT16 *)file_name) != 0)
                {
                    if(TITLE_IMAGE == win_filelist_get_mp_title_type())   //stop image play when rename
                    {
                        usblst_set_preview_txt_enable(0);
                        image_abort();
                    }
                    com_uni_str_to_mb((UINT16 *)file_name);
                    com_uni_str2utf8((UINT16 *)file_name, (UINT8 *)new_name,MAX_FILE_NAME_SIZE);
                    s_len = STRLEN(new_name);
                    if ((MAX_FILE_NAME_SIZE - s_len) > (int)strlen(ext_name))
                    {
                        strncat(new_name, ext_name, MAX_FILE_NAME_SIZE-1-s_len);
                    }
                    t_cur_filelist = win_filelist_get_cur_filelist();
                    func_ret = rename_file_in_file_list(t_cur_filelist, ucurpos, new_name);
                    if(RET_SUCCESS == func_ret)
                    {
                        osd_track_object((POBJECT_HEAD)&g_win_usb_filelist, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
                        display_file_info();
                        //win_draw_mp_pvr_title();
                        //win_draw_mp_pvr_help();
                    }
                    else
                    {
                        usblst_edit_proc_popup(NULL, NULL,RS_MSG_FILE_RENAME_FAILED);
                    }
                 }
            }
           break;
        case VACT_FILELIST_COPY:
            if(ucurpos != 0)
            {
                usblist_edit_copy(ucurpos, broot);
            }
            break;
        case VACT_FILELIST_DELETE:
            if(0 == ucurpos)
            {
                break;
            }
            t_del_flidx = usblst_get_del_filelist_idx();
            win_switch_del_flag(t_del_flidx, ucurpos-1);
            if((broot && 0x1 == pol->w_count) ||(!broot && 0x2 == pol->w_count))
            {//only one file in folder
                osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
            }
            else if((!broot) && (ucurpos == pol->w_count-1))
            {//current is last file and not root
                osd_change_focus((POBJECT_HEAD)pol,1,C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
            }
            else
            {
                ap_vk_to_hk(0, V_KEY_DOWN, &msg_code);
                ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
            }
            break;
        case VACT_FILELIST_MAKE_FOLDER:
            strncpy(new_name, "", MAX_FILE_NAME_SIZE-1);
            com_asc_str2uni((UINT8 *)new_name,unistr);
            if(win_makefolder_open(unistr, &file_name, NULL))
            {
                com_uni_str_to_asc(file_name,new_name);
                t_cur_filelist = win_filelist_get_cur_filelist();
                func_ret = make_folders_in_file_list(t_cur_filelist, new_name);
                if(RET_SUCCESS == func_ret)
                {
                    t_cur_filelist = win_filelist_get_cur_filelist();
                    t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, NULL, 0);
                    if (RET_SUCCESS == t_ret)
                    {
                        usblst_set_dirnum(t_dirnum);
                        usblst_set_filenum(t_filenum);
                    }
                    if(broot)
                    {
                        t_dirnum = usblst_get_dirnum();
                        t_filenum = usblst_get_filenum();
                        osd_set_obj_list_count(pol,(t_dirnum + t_filenum));
                    }
                    else
                    {
                        t_dirnum = usblst_get_dirnum();
                        t_filenum = usblst_get_filenum();
                        osd_set_obj_list_count(pol,(t_dirnum + t_filenum + 1));
                    }
                    osd_track_object((POBJECT_HEAD)&g_win_usb_filelist, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
                    display_file_info();
                    //win_draw_mp_pvr_title();
                    //win_draw_mp_pvr_help();
                }
                else
                {
                    usblst_edit_proc_popup(NULL, NULL,RS_MSG_MAKE_FOLDER_FAILED);
                }
            }
            break;
        default:
            break;
    }
    free_resource(unistr, old_name, new_name);
    return ret;
}

#if 0
static void file_item_enter_dir(char *str_name, UINT32 name_len, char *str_txt, int str_size, p_file_info pfile_node)
{
    POBJLIST pol = NULL;
    TEXT_FIELD *ptxt = NULL;
    UINT8 back_saved = 0;
    file_list_handle t_cur_filelist = NULL;
    unsigned int t_dirnum = 0;
    unsigned int t_filenum = 0;
    RET_CODE t_ret = RET_FAILURE;
    unsigned int t_fileidx = 0;
    UINT32 *t_del_flidx = NULL;

#ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
#endif

    //TODO: Enter child directory & retrive items,set not_root flag
    t_del_flidx = usblst_get_del_filelist_idx();
    if((usblst_get_edit_flag())&&(win_check_del_flags(t_del_flidx, usblst_olist.w_count)))
    {
        win_filelist_delfiles();
    }

    win_get_fullname(str_name, name_len, pfile_node->path, pfile_node->name);
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(200, 200, 200, 100);
    win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
    win_compopup_open_ext(&back_saved);

    t_cur_filelist = win_filelist_get_cur_filelist();
#ifdef SD_UI
    read_folder_to_file_list2(t_cur_filelist, str_name);
#else
    read_folder_to_file_list(t_cur_filelist, str_name);
#endif
    win_compopup_smsg_restoreback();

    t_ret = get_file_list_info(t_cur_filelist, &t_dirnum, &t_filenum, str_name, name_len);
    if (RET_SUCCESS == t_ret)
    {
        usblst_set_dirnum(t_dirnum);
        usblst_set_filenum(t_filenum);
    }

    ptxt = &usb_dir_txt;
#ifdef PERSIAN_SUPPORT
    filelist_utf8_to_unicode(str_name, str_uni);
    com_uni_str_copy_char( (UINT8 *)str, str_uni);
    osd_set_text_field_content(ptxt, STRING_UNICODE,(UINT32)str);
#else
    win_get_display_dirname(str_txt, str_size, str_name);//get dirname to show
    osd_set_text_field_content(ptxt,STRING_UTF8,(UINT32)str_txt);
#endif
    osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

    usblst_set_cur_fileidx(0);
    pol  = &usblst_olist;
    t_dirnum = usblst_get_dirnum();
    t_filenum = usblst_get_filenum();
    osd_set_obj_list_count(pol, (t_dirnum + t_filenum + 1));//to show it's child dir
    t_fileidx = usblst_get_cur_fileidx();
    osd_set_obj_list_cur_point(pol, t_fileidx);
    osd_set_obj_list_new_point(pol, t_fileidx);
    osd_set_obj_list_top(pol, t_fileidx);
    osd_track_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
    display_file_info();
}

static void file_item_enter_valid_file(UINT16 uidx, p_file_info pfile_node, BOOL broot)
{
    POBJECT_HEAD win = NULL;
    POBJECT_HEAD mplayer_menu = NULL;
    play_list_handle t_play_list = NULL;
    UINT8 loop_flag = 0;
    UINT16 t_val = 0;
    file_list_type t_type = MEDIA_PLAYER_FILE_LIST_NUMBER;

    //TODO:play filelist music in current window
    if((pfile_node->filetype == F_MP3) || (pfile_node->filetype == F_BYE1) || (pfile_node->filetype == F_OGG)|| \
        (pfile_node->filetype == F_FLC)|| (pfile_node->filetype == F_WAV) || (pfile_node->filetype == F_PCM))
    {
        win_filelist_set_mp_title_type(TITLE_MUSIC);
        t_play_list = win_filelist_get_playlist(PL_FOLDER_MUSIC);
        win_set_musicplay_param(&t_play_list, uidx + 1, 1);
        loop_flag = usblst_get_music_folderloop_flag();
        display_loopmode(loop_flag);
        usblst_musicplayer_play();
#ifdef MP_SPECTRUM
        open_spectrum_dev();
        g_preview_spectrum_enable = TRUE;
        osd_clear_object((POBJECT_HEAD)&usb_preview_bmp,C_UPDATE_ALL);
        if(get_mute_state())
        {
            update_bars(0);
        }
#endif
        set_mplayer_display_normal();
        draw_mplayer_display();
    }
    else if ((pfile_node->filetype == F_JPG) || (pfile_node->filetype == F_BMP))//play filelist slide show
    {
        win_filelist_set_mp_title_type(TITLE_IMAGE);
        t_val= osd_get_obj_list_top(&usblst_olist);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_top(t_val, t_type);
        t_val = uidx + (broot ? 0 : 1);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_index(t_val, t_type);

    #ifdef DLNA_DMP_SUPPORT
        BOOL b_dmp_bak_flag;
        b_dmp_bak_flag = mp_is_dlna_play();
        if(b_dmp_bak_flag)
          {
            music_stop();
            win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
        }
    #endif
        win = (POBJECT_HEAD)(&g_win_usb_filelist);
        osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

        usblst_set_preview_txt_enable(0);
        image_abort();
        t_play_list = win_filelist_get_playlist(PL_FOLDER_IMAGE);
        win_set_imageplay_param(&t_play_list, uidx + 1, 1);
        mplayer_menu = (POBJECT_HEAD)(&g_win_imageslide);

    #ifdef DLNA_DMP_SUPPORT
        mp_set_dlna_play(b_dmp_bak_flag);
    #endif
        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
        }
    }
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    else if ((pfile_node->filetype == F_MPG) || (pfile_node->filetype == F_MPEG))
    {
        win_filelist_set_mp_title_type(TITLE_VIDEO);
        t_val = osd_get_obj_list_top(&usblst_olist);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_top(t_val, t_type);
        t_val = uidx + (broot ? 0 : 1);
        t_type = usblst_get_mp_filelist_type();
        usblst_set_latest_file_index(t_val, t_type);

    #ifdef DLNA_DMP_SUPPORT
        BOOL b_dmp_bak_flag;
        b_dmp_bak_flag = mp_is_dlna_play();
    #endif

        win = (POBJECT_HEAD)(&g_win_usb_filelist);
        osd_obj_close(win,C_CLOSE_CLRBACK_FLG);

        music_stop();
        win_filelist_set_mp_play_state(MUSIC_STOP_STATE);
        image_abort();

        //    win_mpegplayer_set_param(uIdx + 1,0);
        t_play_list = win_filelist_get_playlist(PL_FOLDER_VIDEO);
        win_set_mpegplay_param(&t_play_list, uidx + 1, 1);
        mplayer_menu = (POBJECT_HEAD)(&g_win_mpeg_player);

    #ifdef DLNA_DMP_SUPPORT
        mp_set_dlna_play(b_dmp_bak_flag);
    #endif

        if(osd_obj_open(mplayer_menu,(UINT32)(~0)) != PROC_LEAVE)
        {
            menu_stack_push(mplayer_menu);
        }
    }
#endif
}
#endif

#endif
