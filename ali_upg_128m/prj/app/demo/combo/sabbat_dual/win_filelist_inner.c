/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_filelist_inner.c
*
*    Description:   internal functions of the filelist
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#include <basic_types.h>
#include <osal/osal.h>
#include <api/libsi/si_eit.h>
#include <api/libmp/media_player_api.h>
#include "win_media.h"
#include <api/libpub/lib_hde.h>
#include "win_com.h"
#include "string.id"
#include "menus_root.h"
#include "control.h"
#include <api/libmp/mp_list.h>

#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_finger_popup.h"
#endif

#include "win_imageslide.h"
#include "win_imagepreview.h"
#include "win_copying.h"
#include "win_mpeg_player.h"

#include "win_filelist.h"
#include "win_filelist_display.h"
#include "win_filelist_inner.h"
#include "media_control.h"
#include "win_mainmenu.h"
#include "win_mute.h"
#include "ap_ctrl_display.h"
#include "copper_common/menu_api.h"

#ifdef SAVE_PLAYLIST_SUPPORT
#include "disk_manager.h"
#endif

#ifdef SAT2IP_SERVER_SUPPORT
#include "sat2ip/sat2ip_control.h"
#endif

#ifdef MP_SPECTRUM
#include "win_mpspectrum.h"
#endif

///////
static file_list_type mp_filelist_type = MEDIA_PLAYER_FILE_LIST_NUMBER;
static file_list_handle win_file_list[WIN_FILELIST_NUMBER] = {NULL};
static char cur_device[WIN_FILELIST_NUMBER][4] = {{0}};
static BOOL g_previewing_mpeg = FALSE;
static UINT8 edit_flag= 0;
static BOOL manual_exit_flag = FALSE;
static MPEG_PLAY_STATE video_play_state = MPEG_STOP;//only for displayer
static UINT8 music_folderloop_flag = 0;
static char    mp_curmusic_name[FULL_PATH_SIZE] = {0};
static int g_preview_decode_txt_enable = 0;
static UINT16 latest_file_index[WIN_FILELIST_NUMBER] = {0};
static UINT16 latest_file_top[WIN_FILELIST_NUMBER] = {0};
static unsigned int dirnum = 0;
static unsigned int filenum = 0;
static unsigned int cur_fileidx = 0;
static play_list_handle cur_playlist_music = NULL;
static UINT16 cur_music_idx = 0;
static music_info mp_curmusic_info;
static int mp_play_time = 0;
static UINT16 mp_play_per = 0;

#ifndef _USE_32M_MEM_
static const char list_name_music[] ="music";
static const char list_name_image[] ="image";
#else
static const char list_name_music[] =" ";
static const char list_name_image[] =" ";
#endif

static UINT32 del_filelist_idx[FILELIST_DELARRAY_MAX] = {0};
static UINT8 fav_ini_flag=0x00;
static BOOL g_into_imagepreview= FALSE;

#ifdef MP_NIM_SHARE_BUFFER
 static struct nim_config st_nim_config;
#endif

//extern BOOL g_from_mpegplayer;        // UI from mpeg player or not flag

file_list_handle usblst_get_filelist_handle(file_list_type list_type)
{
    return win_file_list[list_type];
}

void usblst_set_filelist_handle(file_list_handle list_handle, file_list_type list_type)
{
    win_file_list[list_type] = list_handle;
}

char *usblst_get_cur_device(UINT8 idx)
{
    return cur_device[idx];
}

void usblst_set_cur_device(UINT32 val, UINT8 idx)
{
    MEMSET(cur_device[idx], val, 4);
}

BOOL usblst_get_previewing_mpeg(void)
{
    return g_previewing_mpeg;
}

void usblst_set_previewing_mpeg(BOOL flag)
{
    g_previewing_mpeg = flag;
}

UINT8 usblst_get_edit_flag(void)
{
    return edit_flag;
}

void usblst_set_edit_flag(UINT8 flag)
{
    edit_flag = flag;
}

MPEG_PLAY_STATE usblst_get_video_play_state(void)
{
    return video_play_state;
}

void usblst_set_video_play_state(MPEG_PLAY_STATE state)
{
    video_play_state = state;
}

UINT8 usblst_get_music_folderloop_flag(void)
{
    return music_folderloop_flag;
}

void usblst_set_music_folderloop_flag(UINT8 val)
{
    music_folderloop_flag = val;
}

char *usblst_get_mp_curmusic_name(void)
{
    return mp_curmusic_name;
}

void usblst_set_mp_curmusic_name(char *name)
{
    strncpy(mp_curmusic_name, name, FULL_PATH_SIZE - 1);
}

int usblst_get_preview_txt_enable(void)
{
    return g_preview_decode_txt_enable;
}

void usblst_set_preview_txt_enable(int val)
{
    g_preview_decode_txt_enable = val;
}

UINT16 usblst_get_latest_file_index(UINT8 idx)
{
    return latest_file_index[idx];
}

void usblst_set_latest_file_index(UINT16 val, UINT8 idx)
{
    latest_file_index[idx] = val;
}

UINT16 usblst_get_latest_file_top(UINT8 idx)
{
    return latest_file_top[idx];
}

void usblst_set_latest_file_top(UINT16 val, UINT8 idx)
{
    latest_file_top[idx] = val;
}

unsigned int usblst_get_dirnum(void)
{
    return dirnum;
}

void usblst_set_dirnum(unsigned int val)
{
    dirnum = val;
}

unsigned int usblst_get_filenum(void)
{
    return filenum;
}

void usblst_set_filenum(unsigned int num)
{
    filenum = num;
}

unsigned int usblst_get_cur_fileidx(void)
{
    return cur_fileidx;
}

void usblst_set_cur_fileidx(unsigned int fidx)
{
    cur_fileidx = fidx;
}

play_list_handle usblst_get_cur_playlist_music(void)
{
    return cur_playlist_music;
}

void usblst_set_cur_playlist_music(play_list_handle plisthdl)
{
    cur_playlist_music = plisthdl;
}

UINT16 usblst_get_cur_music_idx(void)
{
    return cur_music_idx;
}

void usblst_set_cur_music_idx(UINT16 music_idx)
{
    cur_music_idx = music_idx;
}

music_info *usblst_get_mp_curmusic_info(void)
{
    return &mp_curmusic_info;
}

int usblst_get_mp_play_time(void)
{
    return mp_play_time;
}

void usblst_set_mp_play_time(int time)
{
    mp_play_time = time;
}

UINT16 usblst_get_mp_play_per(void)
{
    return mp_play_per;
}

void usblst_set_mp_play_per(UINT16 t_val)
{
    mp_play_per = t_val;
}

file_list_type usblst_get_mp_filelist_type(void)
{
    return mp_filelist_type;
}

void usblst_set_mp_filelist_type(file_list_type fltype)
{
    mp_filelist_type = fltype;
}

const char *usblst_get_list_name_music(void)
{
    return list_name_music;
}

const char *usblst_get_list_name_image(void)
{
    return list_name_image;
}

UINT32 *usblst_get_del_filelist_idx(void)
{
    return del_filelist_idx;
}

UINT8 usblst_get_fav_ini_flag(void)
{
    return fav_ini_flag;
}

void usblst_set_fav_ini_flag(UINT8 flag)
{
    fav_ini_flag = flag;
}

static RET_CODE win_init_filelist(void)
{
    char device_arry[MAX_DEVICE_NUM][3] = {{0}};
    UINT8 device_arry_size = MAX_DEVICE_NUM*3;
    TITLE_TYPE title_type = TITLE_NUMBER;
    file_list_handle t_file_list = NULL;

    //MEMSET(&image_slide_setting,0x0,sizeof(IMAGE_SLIDE_INFO));
    //image_slide_setting.u_time_gap = 2;
    //image_slide_setting.b_repeat= 1;

    if(get_stroage_device_number(STORAGE_TYPE_ALL) <= 0 )
    {    //no device mounted
        return RET_FAILURE;
    }

    title_type = win_filelist_get_mp_title_type();
    mp_filelist_type = win_type_title2filelist(title_type);
    t_file_list = usblst_get_filelist_handle(mp_filelist_type);
    if (NULL == t_file_list)
    {    //filelist not exist
        t_file_list = create_file_list(mp_filelist_type);
        usblst_set_filelist_handle(t_file_list, mp_filelist_type);
        if (NULL == t_file_list)
        {
            return RET_FAILURE;
        }

        if(RET_FAILURE == get_current_stroage_device1(STORAGE_TYPE_ALL, device_arry[0], &device_arry_size))
        {
            return RET_FAILURE;
        }

        MEMCPY(cur_device[mp_filelist_type], device_arry[0], 3);
        cur_device[mp_filelist_type][3] = '\0';
        t_file_list = usblst_get_filelist_handle(mp_filelist_type);
        file_list_change_device(t_file_list,cur_device[mp_filelist_type]);
    }

    win_filelist_set_cur_filelist(t_file_list);//cur_filelist = t_file_list;//win_file_list[mp_filelist_type];

    return RET_SUCCESS;
}

void usblst_mp_close_video(void)
{
    vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), FALSE);
#endif
}

void usblst_mp_display_logo(void)
{
    if(g_previewing_mpeg)
    {
        usblst_mp_close_video();
    }
}

VACTION usblst_switch_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_FILELIST:
    case V_KEY_MENU:
    case V_KEY_EXIT:
        if(0 == edit_flag)
        {
            manual_exit_flag = TRUE;
            act = VACT_CLOSE;
        }
        else
        {
            act = VACT_FILELIST_EXIT_EDIT;
        }
        break;
    //case V_KEY_BLUE:
    //    act = VACT_FILELIST_ENTER_EDIT;
     //   break;
    #ifdef C3041
    case V_KEY_FIND:
    #else
    case V_KEY_SWAP:
    #endif
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            if(0 == edit_flag)
            {
                act = VACT_FILELIST_SWITCH;
            }
            else
            {
                act = VACT_FILELIST_INVALID_SWITCH;
            }
        }
        else
        {
            act = VACT_FILELIST_FOCUS_DEVICE;
        }
        break;
    case V_KEY_DOWN:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_FOCUS_OBJLIST;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_LEFT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVL;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_RIGHT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVR;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_MUTE:
        act = VACT_FILELIST_MUTE;
        break;

    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION usblst_music_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;
    file_list_handle t_cur_filelist = NULL;

    switch(key)
    {
    case V_KEY_FILELIST:
    case V_KEY_MENU:
        manual_exit_flag = TRUE;
        act = VACT_CLOSE;
        break;
    case V_KEY_EXIT:
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(win_check_openroot(t_cur_filelist) == TRUE)
        {
            manual_exit_flag = TRUE;
            act = VACT_CLOSE;
        }
        else//when not root,press exit to up dir
        {
            act = VACT_FILELIST_UP_DIR;
        }
        break;

    case V_KEY_PREV:
        act = VACT_FILELIST_PREV;
        break;
    case V_KEY_FB:
        act = VACT_FILELIST_XBACK;
        break;
    case V_KEY_PLAY:
        act = VACT_FILELIST_PLAY;
        break;
    case V_KEY_STOP:
        act = VACT_FILELIST_STOP;
        break;
    case V_KEY_PAUSE:
        act = VACT_FILELIST_PAUSE;
        break;
    case V_KEY_FF:
        act = VACT_FILELIST_XFORWARD;
        break;
    case V_KEY_NEXT:
        act = VACT_FILELIST_NEXT;
        break;
/*#ifndef HW_SECURE_ENABLE
    case V_KEY_RED:
        act = VACT_FILELIST_POP_PLAYLIST;
        break;
    case V_KEY_GREEN:
        act = VACT_FILELIST_FAV;
        break;
    case V_KEY_YELLOW:
        act = VACT_FILELIST_FAV_ALL;
        break;
#endif
    case V_KEY_BLUE:
        act = VACT_FILELIST_ENTER_EDIT;
        break;*/

    #ifdef C3041
    case V_KEY_FIND:
    #else
    case V_KEY_SWAP:
    #endif
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_SWITCH;
        }
        else
        {
            act = VACT_FILELIST_FOCUS_DEVICE;
        }
        break;
    case V_KEY_DOWN:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_FOCUS_OBJLIST;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_PIP:
        act = VACT_FILELIST_SORT;
        break;
    case V_KEY_REPEATAB:
        act = VACT_FILELIST_REPEAT_MODE;
        break;

    case V_KEY_RECALL:
        act = VACT_FILELIST_RECALL;
        break;
    case V_KEY_LEFT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVL;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_RIGHT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVR;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_MUTE:
        act = VACT_FILELIST_MUTE;
        break;

    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION usblst_photo_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;
    file_list_handle t_cur_filelist = NULL;

    switch(key)
    {
    case V_KEY_FILELIST:
    case V_KEY_MENU:
        manual_exit_flag = TRUE;
        act = VACT_CLOSE;
        break;
    case V_KEY_EXIT:
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(win_check_openroot(t_cur_filelist) == TRUE)
        {
            manual_exit_flag = TRUE;
            act = VACT_CLOSE;
        }
        else//when not root,press exit to up dir
        {
            act = VACT_FILELIST_UP_DIR;
        }
        break;
/*#ifndef HW_SECURE_ENABLE
    case V_KEY_RED:
        act = VACT_FILELIST_POP_PLAYLIST;
        break;
    case V_KEY_GREEN:
        act = VACT_FILELIST_FAV;
        break;
    case V_KEY_YELLOW:
        act = VACT_FILELIST_FAV_ALL;
        break;
#endif
    case V_KEY_BLUE:
        act = VACT_FILELIST_ENTER_EDIT;
        break;*/

    #ifdef C3041
    case V_KEY_FIND:
    #else
    case V_KEY_SWAP:
    #endif
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_SWITCH;
        }
        else
        {
            act = VACT_FILELIST_FOCUS_DEVICE;
        }
        break;
    case V_KEY_DOWN:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_FOCUS_OBJLIST;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_PIP:
        act = VACT_FILELIST_SORT;
        break;
    case V_KEY_REPEATAB:
    #ifdef DLNA_DMP_SUPPORT
        if (mp_is_dlna_play())
            act = VACT_FILELIST_REPEAT_MODE;
        else
    #endif
            act = VACT_FILELIST_SETUP;
        break;
    case V_KEY_MP:
        act = VACT_FILELIST_MP;
        break;

    case V_KEY_LEFT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVL;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_RIGHT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVR;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_MUTE:
        act = VACT_FILELIST_MUTE;
        break;

    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION usblst_video_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;
    file_list_handle t_cur_filelist = NULL;

    switch(key)
    {
    case V_KEY_FILELIST:
    case V_KEY_MENU:
        manual_exit_flag = TRUE;
        act = VACT_CLOSE;
        break;
    case V_KEY_EXIT:
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(win_check_openroot(t_cur_filelist) == TRUE)
        {
            manual_exit_flag = TRUE;
            act = VACT_CLOSE;
        }
        else//when not root,press exit to up dir
        {
            act = VACT_FILELIST_UP_DIR;
        }
        break;

    case V_KEY_PLAY:
        act = VACT_FILELIST_PLAY;
        break;
    case V_KEY_PAUSE:
        act = VACT_FILELIST_PAUSE;
        break;
    case V_KEY_STOP:
        act = VACT_FILELIST_STOP;
        break;

    //case V_KEY_BLUE:
    //    act = VACT_FILELIST_ENTER_EDIT;
     //   break;

    #ifdef C3041
    case V_KEY_FIND:
    #else
    case V_KEY_SWAP:
    #endif
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_SWITCH;
        }
        else
        {
            act = VACT_FILELIST_FOCUS_DEVICE;
        }
        break;
    case V_KEY_DOWN:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_FOCUS_OBJLIST;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_PIP:
        act = VACT_FILELIST_SORT;
        break;

    case V_KEY_LEFT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVL;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_RIGHT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVR;
        }
        else
        {
            act = VACT_FILELIST_VOL;
        }
        break;
    case V_KEY_MUTE:
        act = VACT_FILELIST_MUTE;
        break;
    case V_KEY_REPEATAB:
        act = VACT_FILELIST_REPEAT_MODE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

VACTION usblst_edit_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;
  //  file_list_handle t_cur_filelist = NULL;

   /* switch(key)
    {
    case V_KEY_FILELIST:
    case V_KEY_MENU:
        act = VACT_FILELIST_EXIT_EDIT;
        break;
    case V_KEY_EXIT:
        t_cur_filelist = win_filelist_get_cur_filelist();
        if(win_check_openroot(t_cur_filelist) == TRUE)
        {
            act = VACT_FILELIST_EXIT_EDIT;
        }
        else//when not root,press exit to up dir
        {
            act = VACT_FILELIST_UP_DIR;
        }
        break;

    case V_KEY_RED:
        act = VACT_FILELIST_RENAME;
        break;
    case V_KEY_GREEN:
        act = VACT_FILELIST_COPY;
        break;
    case V_KEY_YELLOW:
        act = VACT_FILELIST_DELETE;
        break;
    case V_KEY_BLUE:
        act = VACT_FILELIST_MAKE_FOLDER;
        break;

    #ifdef C3041
    case V_KEY_FIND:
    #else
    case V_KEY_SWAP:
    #endif
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_INVALID_SWITCH;
        }
        else
        {
            act = VACT_FILELIST_FOCUS_DEVICE;
        }
        break;
    case V_KEY_DOWN:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_FOCUS_OBJLIST;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_LEFT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVL;
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    case V_KEY_RIGHT:
        if(MP_DEVICE_ID == osd_get_focus_id(pobj))
        {
            act = VACT_FILELIST_CHG_DEVR;
        }
        else
        {
            act = VACT_PASS;
        }
        break;

    default:
        act = VACT_PASS;
        break;
    }*/

    return act;
}

/************************************************************************************
    Added this function for fixing error show of filelist when playlist was changed
    and saved( related with fav mark). tnntc.zhou
 ************************************************************************************/
void usblist_param_shortcutopen_init(void)
{
    UINT8 i = 0;

#ifdef SAVE_PLAYLIST_SUPPORT
    play_list_handle t_play_list = NULL;

#endif

    for(i=0; i<WIN_PLAYLIST_NUMBER; i++)
    {
        win_filelist_set_playlist(NULL, i);//win_play_list[i] = NULL;
    }
#ifdef SAVE_PLAYLIST_SUPPORT
//    win_play_list[PL_MUSIC] = create_play_list(MUSIC_PLAY_LIST, list_name_music);
    t_play_list = create_play_list(MUSIC_PLAY_LIST, list_name_music);
    win_filelist_set_playlist(t_play_list, PL_MUSIC);
    read_play_list_from_disk(t_play_list);//(win_play_list[PL_MUSIC]);
//    win_play_list[PL_IMAGE] = create_play_list(IMAGE_PLAY_LIST, list_name_image);
    t_play_list = create_play_list(IMAGE_PLAY_LIST, list_name_image);
    win_filelist_set_playlist(t_play_list, PL_IMAGE);
    read_play_list_from_disk(t_play_list);//(win_play_list[PL_IMAGE]);
#endif
}

void usblst_ca_clean(void)
{
#ifdef SUPPORT_BC
    UINT8 i = 0;

#endif

    // CA Clean
#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
    //clean msg/
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    if((CA_MMI_PRI_01_SMC== get_mmi_showed())||(CA_MMI_PRI_06_BASIC== get_mmi_showed()))
    {
        //win_mmipopup_close(1);
        win_pop_msg_close(CA_MMI_PRI_01_SMC);
        win_fingerpop_close();
    }
    if(CA_MMI_PRI_05_MSG!= get_mmi_showed())
    {
        set_mmi_showed(CA_MMI_PRI_10_DEF);
    }
#endif

#if defined(SUPPORT_BC_STD)
    handle_osm_complement_in_ignore_window(TRUE);

    // stop trasnponder since DMX must be stop
    api_mcas_stop_transponder();
#elif defined(SUPPORT_BC)
    handle_osm_complement_in_ignore_window(TRUE);

    // stop trasnponder since DMX must be stop
    for(i=0;i<3;i++)
        api_mcas_stop_transponder_multi_des(i);
#endif
}

void usblst_resource_release(void)
{
    // DMX/TTX/EPG Stop
    // media player's buffer may share with DMX/TTX/EPG,
    // stop DMX/TTX/EPG to avoid data corrupted.
#ifdef MP_NIM_SHARE_BUFFER
    UINT32 nim_sub_type = 0 ;
    struct nim_device *nim_dev = NULL;
 #endif

#ifdef SAT2IP_SERVER_SUPPORT
    ap_sat2ip_stop_provider_all();
#endif
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    dmx_stop((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
#ifdef MP_NIM_SHARE_BUFFER
    nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    nim_sub_type = dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM);

    MEMSET(&st_nim_config,0,sizeof(st_nim_config));
    dev_get_nim_config(nim_dev, nim_sub_type, &st_nim_config);
    nim_close(nim_dev);
#endif

#if (TTX_ON ==1)
    ttx_enable(FALSE);
#endif
    epg_off();
}

// win_filelist UI init
BOOL usblst_ui_init(void)
{
    BOOL ret = FALSE;
    UINT8 back_saved = 0;
    TITLE_TYPE m_title_type = TITLE_NUMBER;
    enum video_decoder_type v_type = MPEG2_DECODER;

    manual_exit_flag = FALSE;
    if((g_from_imageslide==TRUE) || (g_from_imagepreview==TRUE) || (g_from_mpegplayer==TRUE)|| (g_from_copyfile==TRUE))
    {
      osd_set_container_focus(&g_win_usb_filelist, MP_OBJLIST_ID);
	  osd_draw_object((POBJECT_HEAD)&g_win2_mainmenu,C_UPDATE_ALL);
    }
    else
    {
        osd_set_container_focus(&g_win_usb_filelist, MP_DEVICE_ID);
    }
        api_set_preview_vpo_color(TRUE);
        image_restore_vpo_rect();//TODO

        if((FALSE == g_from_imageslide) && (FALSE == g_from_imagepreview)
            && (FALSE == g_from_mpegplayer) && (FALSE == g_from_copyfile) && (FALSE == win_filelist_get_switch_title()))
        {
            if(NULL == menu_stack_get_top())
            {
                mm_enter_stop_mode(FALSE);
                win_filelist_set_opened_hotkey(TRUE);
                win_filelist_set_mp_title_type(TITLE_VIDEO);
            }
            else
            {
                api_stop_play(0);
            }

            win_filelist_set_musiclist_changed(FALSE);
            win_filelist_set_imagelist_changed(FALSE);
            edit_flag = 0;
        }
        else
        {
//          g_from_imageslide = FALSE;
            g_from_imagepreview = FALSE;
            g_from_mpegplayer = FALSE;
        }
        video_play_state = MPEG_STOP;
        usblst_mp_display_logo();

        vpo_aspect_mode((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS,0), TV_4_3, NORMAL_SCALE);
#ifdef DUAL_VIDEO_OUTPUT
        vpo_aspect_mode((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS,1), TV_4_3, NORMAL_SCALE);
#endif
        osal_task_sleep(30);

        //if(is_cur_decoder_avc())
        //{
        //    h264_decoder_select(0, VIEW_MODE_PREVIEW == hde_get_mode());
        //}
        v_type = get_current_decoder();
        if(v_type != MPEG2_DECODER)
        {
            video_decoder_select(MPEG2_DECODER, VIEW_MODE_PREVIEW == hde_get_mode());
        }

        api_set_system_state(SYS_STATE_USB_MP);
        osal_cache_flush_all();

        if(RET_FAILURE == win_init_filelist())
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(GET_MID_L(200), GET_MID_T(100), 200, 100);
            win_compopup_set_msg(NULL, NULL,RS_UDISK_ERROR);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            manual_exit_flag = TRUE;
            win_filelist_set_switch_title(FALSE);
            return ret;
        }

        if(MUSIC_IDLE_STATE == win_filelist_get_mp_play_state())
        {
            win_filelist_set_loop_mode(PLAY_LIST_REPEAT);
            music_folderloop_flag = 1;//init in filelist
            MEMSET(mp_curmusic_name,0x0,MAX_FILE_NAME_SIZE);
        }

        init_filelist_display();
        init_mplayer_display();
        if((g_from_copyfile == TRUE) && (1 == edit_flag))
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_title(0,m_title_type);
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help(0, m_title_type);
        }
        else
        {
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_title(1,m_title_type);
            m_title_type = win_filelist_get_mp_title_type();
            win_set_mp_pvr_help(1, m_title_type);
        }
        g_from_copyfile = FALSE;
        //win_draw_mp_pvr_title();

        return TRUE;
}

#ifdef MP_SPECTRUM
extern BOOL g_preview_spectrum_enable;
#endif

void usblst_ui_close(void)
{
    SYSTEM_DATA *sys_cfg = NULL;
    ID t_id = OSAL_INVALID_ID;

    g_preview_decode_txt_enable = 0;
    if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
    {
          api_set_preview_vpo_color(FALSE);
    }
#ifdef MP_SPECTRUM
    close_spectrum_dev();
    g_preview_spectrum_enable = FALSE;
#endif
    image_abort();
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    mpg_cmd_stop_proc(0);
#endif
    if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
    {
        hde_set_mode(VIEW_MODE_FULL);
    }
    vdec_stop((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV), 0, 0);
    if(FALSE == g_into_imagepreview)
    {
         sys_cfg = sys_data_get();
         sys_data_set_display_mode(&(sys_cfg->avset));
    }
    else
    {
        g_into_imagepreview = FALSE;
    }

    if(manual_exit_flag == TRUE)
    {
        music_stop();
        t_id = win_filelist_get_mp_refresh_id();
        api_stop_timer(&t_id);
        win_filelist_set_mp_refresh_id(t_id);
        if(MUSIC_PAUSE_STATE == win_filelist_get_mp_play_state())
        {
            if(!get_mute_state())
            {
                api_audio_set_mute(FALSE);
            }
        }
        win_filelist_set_mp_play_state(MUSIC_IDLE_STATE);
    #ifdef SAVE_PLAYLIST_SUPPORT
        char *cur_disk_name = cur_device[mp_filelist_type];
        int disk_type = 0;
        int disk_idx = 0;
        int dm_ret = 0;
        int vol_num = 0;
        const UINT8 disk_name_len = 3;

        if (disk_name_len == STRLEN(cur_disk_name))
        {
            disk_type = disk_name_prefix2type(cur_disk_name);
            disk_idx = (int)cur_disk_name[2] - 'a';
            dm_ret = dm_ioctl(DM_CMD2_GET_VOL_NUM, DM_CMD_PARAM(disk_type, disk_idx, 0), &vol_num, sizeof(int));
            if ((dm_ret == 0) && (vol_num > 0))
            {
                save_playlist();
            }
        }
    #endif
        ap_clear_all_message();
        api_set_system_state(SYS_STATE_NORMAL);
    }
    g_from_imageslide = FALSE;
    g_from_imagepreview = FALSE;
    g_from_mpegplayer = FALSE;

#ifdef DLNA_DMP_SUPPORT
    mp_set_dlna_play(FALSE);
#endif
}


static void nim_resource_reinit(void)
{
#ifdef MP_NIM_SHARE_BUFFER
    struct nim_device *nim_dev = NULL;
    UINT32 nim_sub_type = 0 ;
    struct nim_config config;

    //restart nim
    nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    nim_sub_type = dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM);
    if(nim_dev)
    {
        nim_open(nim_dev);
        //clear nim old status
        MEMSET(&config,0,sizeof(config));
        dev_set_nim_config(nim_dev, nim_sub_type, &config);
    }
#endif

}

void usblst_resource_restore(void)
{
#ifdef AV_DELAY_SUPPORT
    SYSTEM_DATA *sys_cfg = NULL;
#endif

    if (FALSE == win_filelist_get_switch_title())//(switch_title == FALSE)
    {
        osd_clear_screen();
        image_restore_vpo_rect();//TODO
    }
    nim_resource_reinit();

    if(manual_exit_flag)
    {
        epg_reset();
        if (win_filelist_get_opened_hotkey())//(opened_by_hot_key)
        {
            mm_leave_stop_mode();
            win_filelist_set_opened_hotkey(FALSE);// opened_by_hot_key = FALSE;
        }
        else
        {
            win_mainmenu_set_focus_to_usb();
            //api_show_menu_logo();
            manual_exit_flag = FALSE;
            if(NULL == menu_stack_get(1))
            {
                menu_stack_push_ext((POBJECT_HEAD)&g_win2_mainmenu,-1);
                //menu_stack_push_ext((POBJECT_HEAD)&g_win_submenu,-1);
            }
            //wincom_open_title((POBJECT_HEAD)&g_win_mainmenu,RS_COMMON_MEDIA_PLAYER,0);
            osd_draw_object((POBJECT_HEAD)&g_win2_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }

        // exit media player, start DMX/TTX/EPG
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    #ifdef NEW_DEMO_FRAME
        dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1));
    #endif

        //extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);
        epg_init(SIE_EIT_WHOLE_TP, (UINT8*)__MM_EPG_BUFFER_START, __MM_EPG_BUFFER_LEN, ap_epg_call_back);

    #ifdef AV_DELAY_SUPPORT
        sys_cfg = sys_data_get();
        api_set_avdelay_value(sys_cfg->avset.avdelay_value, AVDELAY_LIVE_PLAY_MODE);
    #endif

    #if defined(SUPPORT_BC_STD)
        api_mcas_start_transponder();
    #elif defined(SUPPOR_BC)
        api_mcas_start_transponder_multi_des(0);
    #endif

    #ifdef SLOW_PLAY_BEFORE_SYNC
        vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
        vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_SLOW_PLAY_BEFORE_SYNC, TRUE);
    #endif

    }
}

PRESULT usblst_change_storage(VACTION act, UINT8 update)
{
    PRESULT ret = PROC_LOOP;
    TEXT_FIELD *ptxt = NULL;
    OBJLIST *pol = NULL;
    char *str_buff = NULL;//[MAX_FILE_NAME_SIZE+ 1] = {0};
    file_list_handle t_cur_filelist = NULL;

    str_buff = MALLOC(MAX_FILE_NAME_SIZE + 1);
    if (NULL == str_buff)
    {
        ASSERT(0);
        return ret;
    }
    MEMSET(str_buff, 0x0, MAX_FILE_NAME_SIZE + 1);
    if(act == VACT_FILELIST_CHG_DEVL)
    {
        if(RET_FAILURE == win_get_next_device(cur_device[mp_filelist_type], cur_device[mp_filelist_type],TRUE))
        {
          //if (NULL != str_buff)
          //{
            FREE(str_buff);
            str_buff = NULL;
          //}
            return ret;
        }
    }
    else
    {
        if(RET_FAILURE == win_get_next_device(cur_device[mp_filelist_type], cur_device[mp_filelist_type],FALSE))
        {
          //if (NULL != str_buff)
          //{
            FREE(str_buff);
            str_buff = NULL;
          //}
            return ret;
        }
    }

    t_cur_filelist = win_filelist_get_cur_filelist();
    file_list_change_device(t_cur_filelist,cur_device[mp_filelist_type]);

    latest_file_index[mp_filelist_type] = 0;
    latest_file_top[mp_filelist_type] = 0;

    ptxt = &usb_dev_name;
    win_get_display_devicename(str_buff, MAX_FILE_NAME_SIZE+1, cur_device[mp_filelist_type]);
    snprintf(str_buff, MAX_FILE_NAME_SIZE+1, "%s",str_buff);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buff);//show current device name
    if(update)
    {
        osd_track_object((POBJECT_HEAD)&usb_dev_con,C_UPDATE_ALL);
    }

    ptxt = &usb_dir_txt;
    snprintf(str_buff, MAX_FILE_NAME_SIZE+1, "%s","Root");
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buff);
    if(update)
    {
        osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);
    }

    pol  = &usblst_olist;
    t_cur_filelist = win_filelist_get_cur_filelist();
    get_file_list_info(t_cur_filelist, &dirnum, &filenum, NULL, 0);

    cur_fileidx = 0;
    osd_set_obj_list_cur_point(pol, cur_fileidx);
    osd_set_obj_list_new_point(pol, cur_fileidx);
    osd_set_obj_list_top(pol, cur_fileidx);
    osd_set_obj_list_count(pol, dirnum+filenum);

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)pol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
        display_file_info();
    }

    //if (NULL != str_buff)
    //{
        FREE(str_buff);
        str_buff = NULL;
    //}
    return ret;
}

void usblst_musicplayer_play(void)
{//make sure cur_playlist_music & cur_music_idx are valid before call this function
     TEXT_FIELD *ptxt = NULL;
    char *music_name = NULL;//[FULL_PATH_SIZE] = {0};
    media_file_info file_node;
    char str_buf[20] = {0};
    int ret_val = 0;
    ID t_id = OSAL_INVALID_ID;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    music_name = MALLOC(FULL_PATH_SIZE);
    if (NULL == music_name)
    {
        ASSERT(0);
        return;
    }
    MEMSET(music_name, 0x0, FULL_PATH_SIZE);

    win_filelist_set_mp_play_state(MUSIC_PLAY_STATE);
    music_stop();

    set_play_list_info(cur_playlist_music, &cur_music_idx, NULL);
    get_file_from_play_list(cur_playlist_music, cur_music_idx, &file_node);
    win_get_fullname(music_name, FULL_PATH_SIZE, file_node.path, file_node.name);

    if (STRCMP(mp_curmusic_name, music_name) != 0) //not same
    {
        strncpy(mp_curmusic_name, music_name, FULL_PATH_SIZE-1);
        display_current_music();
    }
#ifdef DLNA_DMP_SUPPORT
    if(mp_is_dlna_play())
    {
        win_dlna_play_preset(music_name, 1);
        dmp_music_set_pcm_config(music_name);
    }
#endif

    if (RET_FAILURE == win_get_fileinfo(music_name, (UINT32)&mp_curmusic_info))
    {
    #ifdef DLNA_DMP_SUPPORT
        if(mp_is_dlna_play())
        {
            mp_curmusic_info.time = (unsigned long)dmp_get_duration();
            if (!network_is_disable_seek())
            {
                libc_printf("%s, return RET_FAILURE\n", __FUNCTION__);
                if (NULL != music_name)
                {
                    FREE(music_name);
                    music_name = NULL;
                }
                return;
            }
        }
    #else
        libc_printf("%s, return RET_FAILURE\n", __FUNCTION__);
/*
        if (NULL != music_name)
        {
            FREE(music_name);
            music_name = NULL;
        }
        return;
*/
    #endif

    }

	ptxt = &usb_file_info;
	display_music_details(ptxt, &file_node, music_name, FULL_PATH_SIZE);
    ptxt = &mplayer_total;
    mp_play_time = mp_curmusic_info.time;
    //sprintf(str_buf,"%02d:%02d:%02d", mp_play_time/3600, (mp_play_time%3600)/60, mp_play_time%60);
    snprintf(str_buf, 20, "%02d:%02d:%02d", mp_play_time/3600, (mp_play_time%3600)/60, mp_play_time%60);
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buf);
    osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

    mp_play_time = 0;
    mp_play_per = 0;
    osd_set_progress_bar_pos(&mplayer_bar,mp_play_per);

    ptxt = &mplayer_curtime;
    //sprintf(str_buf, "%s", "00:00:00");
    snprintf(str_buf, 20, "%s", "00:00:00");
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_buf);
    osd_draw_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

    t_id = win_filelist_get_mp_refresh_id();
    api_stop_timer(&t_id);
    win_filelist_set_mp_refresh_id(t_id);
    t_id = api_start_cycletimer("M",200,usblst_mprefresh_handler);
    win_filelist_set_mp_refresh_id(t_id);

    if(!get_mute_state())
    {
        api_audio_set_mute(FALSE);
    }

    ret_val = music_play(music_name);
    if(ret_val < 0)//can not play mp3
    {
        osal_task_sleep(200);//delay to avoid flicker of unregonized MP3
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MP3OVER,0,TRUE);
    }

    if (NULL != music_name)
    {
        FREE(music_name);
        music_name = NULL;
    }
}

void usblst_mprefresh_handler(UINT32 nouse)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE,0,FALSE);
}

BOOL usblst_check_file_previewing(void)
{//ckeck if current file in filelist is jpg or bmp or mpeg
    UINT16 ucurpos = 0;
    media_file_info file_node;
    file_list_handle t_cur_filelist = NULL;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    ucurpos = osd_get_obj_list_cur_point(&usblst_olist);
    t_cur_filelist = win_filelist_get_cur_filelist();
    if(!win_check_openroot(t_cur_filelist))
    {
        if(ucurpos > 0)
    {
            ucurpos--;
        }
    }
    t_cur_filelist = win_filelist_get_cur_filelist();
    get_file_from_file_list(t_cur_filelist, ucurpos+1, &file_node);
    if((F_JPG == file_node.filetype) || (F_BMP == file_node.filetype)
        ||(F_MPG == file_node.filetype) || (F_MPEG == file_node.filetype))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif
