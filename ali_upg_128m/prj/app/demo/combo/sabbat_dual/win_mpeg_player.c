/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mpeg_player.c
*
*    Description: Mediaplayer mpeg video player.
*
 *  History:
 *  Date            Author          Version  Comment
 *  ====            ======          =======  =======
 *  1.  2008.7.15   Sharon Zhang    0.1.000  initial for mediaplayer 2.0
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#ifdef USB_MP_SUPPORT
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <hld/decv/vdec_driver.h>
#include <mediatypes.h>
#include <api/libchar/lib_char.h>
#include <api/libmp/mp_list.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_mp_subtitle.h"
#ifdef AUDIOTRACK_SUPPORT
#include "win_mpeg_player_audio_track.h"
#endif
#include "win_plugin_subt.h"
#include "win_mpeg_player.h"
#include "win_mpeg_player_private.h"
#include "win_filelist.h"
#include "media_control.h"
#include "win_mute.h"
#include "ctrl_key_proc.h"
#include "control.h"

#define MPEGBAR_TIMER_NAME  "mpegbarname"

static CUR_VIDEO_INFO mpeginfo ;
static BAR_STATE bar_state ;
static ID mpeg_timeout_id = OSAL_INVALID_ID;
static ID mpeg_doubleclick_timeout_id = OSAL_INVALID_ID;
static doubleclick_state mpeg_click_state = DOUBLECLICK_INVALIDE;
static UINT32   mpgbar_timer_time = 0;
static ID mpeg_refresh_id = OSAL_INVALID_ID;
static UINT32 mpeg_refresh_cnt = 0;
static UINT8 mpeg_prompt_on_screen = 0;
static UINT8 m_hide_window = 0;
static BOOL m_b_set_vol = FALSE;
static BOOL m_mpeg_player_working = FALSE;
BOOL g_from_mpegplayer = FALSE;
static BOOL b_build_idx_tbl = FALSE;
static BOOL b_vdec_support = TRUE;
static BOOL b_resume_stop = TRUE;
static BOOL b_audio_support = TRUE;
static enum TV_SYS_TYPE sys_tv_mode_bak = TV_MODE_AUTO;

static BOOL update_mpeg_infor = FALSE;
#ifdef DLNA_DMP_SUPPORT
static int g_stop_read_media; // for secucode
#endif
static UINT32 init_video_idx= 0;
static UINT32 init_video_start_tick = 0;
extern UINT32 back_to_mainmenu;

static fileinfo_video file_mp_info;
static UINT8 m_record_plaer_bar_state = 0;
static UINT32 cur_prog = 0;
__MAYBE_UNUSED__ static BOOL tmp_flag = TRUE;
static UINT8 mpegspeed_steps[] ={ 2, 4, 8, 16, 24};//32
static UINT16 mpegplayer_status_icons[] =
{
    IM_PVR_ICON_STOP,           //stop
    IM_PVR_ICON_PLAY,           //normal playback
    IM_PVR_ICON_PAUSE,          //pause
    IM_PVR_ICON_BACKWARD,       //fast backward
    IM_PVR_ICON_FORWARD,        //fast forward
    IM_PVR_ICON_SLOWBACKWARD,   //slow backward
    IM_PVR_ICON_SLOWFORWARD,    //slow backward
    IM_PVR_ICON_STEP,           //step
};

LDEF_BMP(&g_win_mpeg_player, mpeg_player_bmp, NULL, 0, 0, 0, 0, 0/*WSTL_N_PRSCRN1*/, 0)
LDEF_TXT(&g_win_mpeg_player, mpeg_player_text, NULL, 0, 0, 0, 0, 0)

LDEF_PROGRESS_BAR(&g_win_mpeg_player, mpeg_player_bar, NULL, BAR_L, BAR_T, BAR_W, BAR_H, \
                  PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE, 0, 3, BAR_W, BAR_H - 6, WSTL_BAR_06)

LDEF_WIN(g_win_mpeg_player, NULL,W_MPEG_PLAYER_L, W_MPEG_PLAYER_T, W_MPEG_PLAYER_W, W_MPEG_PLAYER_H, 1)

static OSD_RECT mp_compopup_rect = {MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H};

static UINT8 win_mp_common_compopup(win_popup_type_t type, OSD_RECT rect, char *str, UINT32 timeout)
{
    UINT8 back_saved = 0;

    win_compopup_init(type);
    win_compopup_set_frame(rect.u_left, rect.u_top, rect.u_width, rect.u_height);
    win_compopup_set_msg_ext(str,NULL,0);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(timeout);
    win_compopup_smsg_restoreback();
    return back_saved;
}

/*****************************************************
Interface    : static void win_mpegplayer_chapter_set();
Description  : create chapter container
Author    : Wu Jianwen 2010.8.25
Params    : NONE
Return    : NONE
******************************************************/
static void win_mpegplayer_chapter_set(void)
{
    UINT8 back_saved = 0;
    UINT32 total_chapter = 0;    //the total chapter of the video
    DEC_CHAPTER_INFO chapterinfo;

	if(0 == back_saved)
	{
		;
	}
    if(mpeg_prompt_on_screen != 0) //fix bug:When switch track£¬clear the pop up message because of not support
    {                              // last track decode. by Wu Jianwen,2010.8.18
        mpeg_file_prompt(NULL, 0);
    }

    MEMSET(&chapterinfo,0,sizeof(DEC_CHAPTER_INFO));
    chapterinfo.dst_chapter = -1;
    mpg_file_get_chapter_info(&chapterinfo);   //get the current chapter info
    total_chapter = chapterinfo.nb_chapter;
    if(total_chapter > 0)
    {
        if(osd_obj_open((POBJECT_HEAD)&g_cnt_chapter, 0) != PROC_LEAVE)
        {
            menu_stack_push((POBJECT_HEAD) &g_cnt_chapter);
        }
    }
    else
    {
        back_saved = win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,"There is only one chapter!",1500);
    }
}

#ifdef AV_DELAY_SUPPORT
BOOL check_media_player_is_working()
{
    return m_mpeg_player_working;
}
#endif
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION mpeg_player_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = 0;

    switch (key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_PVR_INFO:
        case V_KEY_PLAY:
        case V_KEY_PAUSE:
        case V_KEY_STOP:
        case V_KEY_FF:
        case V_KEY_FB:
        case V_KEY_SLOW:
        case V_KEY_PREV:
        case V_KEY_NEXT:
        case V_KEY_LEFT:
        case V_KEY_RIGHT:
        case V_KEY_ENTER:
        case V_KEY_P_UP:
        case V_KEY_P_DOWN:
//vic100519#1 begin
        case V_KEY_UP:  case V_KEY_DOWN:
        case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:   case V_KEY_4:
        case V_KEY_5:   case V_KEY_6:   case V_KEY_7:   case V_KEY_8:   case V_KEY_9:
            act = VACT_VALID;
            break;
        case V_KEY_SUBTITLE://embedded subt
//vic100519#1 end
            if(MPEG_PLAY == mpeginfo.play_state)
            {
                act = VACT_SUBTITLE;
                mp_set_is_subtitle_flag(TRUE);
            }
            else
            {
                act = VACT_VALID;
            }
            break;
#ifdef MP_SUBTITLE_SUPPORT
        case V_KEY_GREEN://ext subt //xuehui
            //if((MPEG_PLAY == mpeginfo.play_state) && (get_subtitle_handle()))
			if(MPEG_PLAY == mpeginfo.play_state)
            {
                act = VACT_MULT_SUBTITLE;
                mp_set_is_subtitle_flag(FALSE);//b_is_subtitle = FALSE;
            }
            else
            {
                act = VACT_VALID;
            }
            break;
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
        case V_KEY_BLUE://ext subt setting
            if(MPEG_PLAY == mpeginfo.play_state)
            {
                act = VACT_SUBTITLE_SETTING;
            }
            else
            {
                act = VACT_VALID;
            }
            break;
#endif
        case V_KEY_MUTE:
            act = VACT_MUTE;
            break;
        case V_KEY_AUDIO:
            if(MPEG_PLAY == mpeginfo.play_state)
            {
                act = VACT_AUDIO;
            }
            else
            {
                act = VACT_VALID;
            }
            break;
        case V_KEY_RED:
            act = VACT_VIDEO;
            break;
        case V_KEY_YELLOW:       //deal with chapter switch,by Wu Jianwen,2010.8.20
            //act = VACT_CHAPTER;
            act = VACT_VALID;
            break;
#ifdef AV_DELAY_SUPPORT
        case V_KEY_MP:
            act = VACT_AVDELAY;
            break;
#endif
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT mpeg_player_unknown_act_proc(POBJECT_HEAD pobj,VACTION unact, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32  v_key = 0;
    UINT8   back_saved = 0;
    char    str[50] = {0};
    POBJECT_HEAD pwin_audio = NULL;

	if(0 == back_saved)
	{
		;
	}
	v_key = param1 & 0x0000FFFF;
    switch (unact)
    {
#ifdef AUDIOTRACK_SUPPORT//vic100519#1 begin
        case VACT_AUDIO:
            if(0 < file_mp_info.audio_stream_num) //avoid display the container even if no audio,by Wu Jianwen,2010.8.17
            {
                pwin_audio = (POBJECT_HEAD)(&g_win_audio_track);
                if(!mp_audio_track_is_show())
                {
                    m_record_plaer_bar_state = m_hide_window;
                    win_mpeg_player_draw(FALSE);
                    osd_obj_open(pwin_audio,MENU_OPEN_TYPE_KEY + v_key);
                    mp_set_audio_track_show(TRUE);
                }
                else // showed
                {
                    mp_audio_track_show_proc(param2);//goto DEFAULT;
                }
            }
            else
            {
                back_saved = win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,"No Audio Track!",1500);
            }
            ret = PROC_LOOP;
            break;
        case VACT_VIDEO:
            if(0 < file_mp_info.prog_num)  //avoid display the container even if no video prog,by Wu Jianwen,2010.8.17
            {
                pwin_audio = (POBJECT_HEAD)(&g_win_audio_track);
                if(!mp_audio_track_is_show())
                {
                    m_record_plaer_bar_state = m_hide_window;
                    win_mpeg_player_draw(FALSE);
                    osd_obj_open(pwin_audio,MENU_OPEN_TYPE_KEY + v_key);
                    mp_set_audio_track_show(TRUE);
                    //ret = PROC_LOOP;
                }
                else //showed
                {
                    mp_audio_track_show_proc(param2);//goto DEFAULT;
                }
            }
            else
            {
                snprintf(str, 45, "There is only one Video Program!");
                back_saved = win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,str,1500);
            }
            ret = PROC_LOOP;
            break;
        case VACT_SUBTITLE:
            if(0 < file_mp_info.sub_stream_num)//avoid display the container even if no video prog,by Wu Terry,2010.8.17
            {
                pwin_audio = (POBJECT_HEAD)(&g_win_audio_track);
                if(!mp_audio_track_is_show())
                {
                    m_record_plaer_bar_state = m_hide_window;
                    win_mpeg_player_draw(FALSE);
                    osd_obj_open(pwin_audio,MENU_OPEN_TYPE_KEY + v_key);
                    mp_set_audio_track_show(TRUE);
                }
                else// showed
                {
                    mp_audio_track_show_proc(param2);//goto DEFAULT;
                }
                ret = PROC_LOOP;
            }
            else
            {
                back_saved =win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,"No Embedded Subtitle!",1500);
            }
            break;
#ifdef MP_SUBTITLE_SUPPORT
        case VACT_MULT_SUBTITLE://xuehui
            if(mps_get_subt_file_num())
            {
                pwin_audio = (POBJECT_HEAD)(&g_win_audio_track);
                if(!mp_audio_track_is_show())
                {
                    m_record_plaer_bar_state = m_hide_window;
                    win_mpeg_player_draw(FALSE);
                    osd_obj_open(pwin_audio,MENU_OPEN_TYPE_KEY + v_key);
                    mp_set_audio_track_show(TRUE);
                }
                else // showed
                {
                    mp_audio_track_show_proc(param2);//goto DEFAULT;
                }
                ret = PROC_LOOP;
            }
            else
            {
                back_saved =win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,"No External Subtitle!",1500);
            }
            break;
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
        case VACT_SUBTITLE_SETTING:
            if(mp_audio_track_is_show())
            {
                break;
            }
            if(mps_get_subt_file_num())
            {
                if(mpeg_prompt_on_screen != 0)
                {              
                    mpeg_file_prompt(NULL, 0);
                }
                if(osd_obj_open((POBJECT_HEAD)&win_subt_set_con, 0) != PROC_LEAVE)
                {
                    menu_stack_push((POBJECT_HEAD) &win_subt_set_con);
                }
            }
            else
            {
                snprintf(str, 45, "No Ext-Subtitle,Setting Invalid!");
                back_saved =win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,str,1500);
            }
            break;
#endif
    #ifdef AV_DELAY_SUPPORT
        case VACT_AVDELAY:
            if(b_vdec_support && b_audio_support)//vdec support is TRUE
            {
                if(osd_obj_open((POBJECT_HEAD)&win_av_delay_con,(UINT32)(~0)) != PROC_LEAVE)
                {
                    menu_stack_push((POBJECT_HEAD)&win_av_delay_con);
                }
            }
            break;
    #endif
#endif
        default :
        {
#ifdef AUDIOTRACK_SUPPORT
            if(mp_audio_track_is_show())
            {
                ret = mp_audio_track_show_proc(param2);
                if (PROC_LEAVE == ret)
                {
                    if (VACT_MUTE == unact)
                    {
                        set_mute_on_off(FALSE);
                        save_mute_state();
                    }
                    show_mute_on_off();
                }
                ret = PROC_LOOP;
                break;
            }
#endif
            if (VACT_MUTE == unact)
            {
                set_mute_on_off(TRUE);
                save_mute_state();
            }

            ret = win_mpeg_player_key_proc(pobj, (param1 & 0xFFFF), param2);
            if (PROC_LEAVE != ret )
            {
                v_key = param1 & 0x0000FFFF;
                if((V_KEY_EXIT!=v_key)&&(V_KEY_MENU!=v_key)&&(V_KEY_INFOR!=v_key)&&(V_KEY_LEFT!=v_key)
                    && (V_KEY_RIGHT != v_key)&&(V_KEY_SUBTITLE!=v_key)&&(V_KEY_YELLOW!=v_key)&&(V_KEY_GREEN!=v_key))
                {
                    update_mpeg_infor = FALSE;
                    win_mpeg_player_draw(TRUE);
                }
            }
            break;
        }
    }
    return ret;
}

static PRESULT mpeg_player_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    UINT32  v_key = 0;//vic100519#1
    SYSTEM_DATA *sys_data = NULL;
    UINT32 total_subtitle = 0;    //save toatal num subtitle of the stream,by Wu Jianwen,2010.8.17
    UINT32 total_audio_track = 0;
    UINT32 total_video_prog = 0;
    UINT32 total_chapter = 0;
    __MAYBE_UNUSED__ UINT16 tmp_icons = 0 ;
    DEC_CHAPTER_INFO chapterinfo;
    enum TV_SYS_TYPE e_sys_tvmode = TV_MODE_AUTO;
    struct vpo_io_get_info vpo_info;

	if(0 == total_chapter)
	{
		;
	}
	if(0 == total_video_prog)
	{
		;
	}
	if(0 == total_audio_track)
	{
		;
	}
	if(0 == total_subtitle)
	{
		;
	}
	if(0 == v_key)
	{
		;
	}
	MEMSET(&chapterinfo, 0x0, sizeof (chapterinfo));
    MEMSET(&vpo_info, 0, sizeof(struct vpo_io_get_info));
    sys_data = sys_data_get();

    switch (event)
    {
        case EVN_PRE_OPEN:
            sys_tv_mode_bak = sys_data_get_tv_mode();
            mpgbar_timer_time = sys_data->osd_set.time_out;
            if((0 == mpgbar_timer_time) || (MPEGBAR_TIMER_TIME_THRESHOLD < mpgbar_timer_time))
            {
                mpgbar_timer_time = 5;
            }
            mpgbar_timer_time *= 1000;
            if (!m_mpeg_player_working)
            {
                mpeg_player_init();
            }
            else
            {
                ret = PROC_LOOP;
            }
            m_mpeg_player_working = TRUE;
            #ifdef BIDIRECTIONAL_OSD_STYLE
            if ((osd_get_mirror_flag())&&(tmp_flag))
            {
                tmp_icons = mpegplayer_status_icons[3];
                mpegplayer_status_icons[3] = mpegplayer_status_icons[4];
                mpegplayer_status_icons[4] = tmp_icons;

                tmp_icons = mpegplayer_status_icons[5];
                mpegplayer_status_icons[5] = mpegplayer_status_icons[6];
                mpegplayer_status_icons[6] = tmp_icons;
                tmp_flag = FALSE;
            }
            #endif
            break;
        case EVN_POST_OPEN:
            mpeg_timeout_id = api_start_timer(MPEGBAR_TIMER_NAME, mpgbar_timer_time, mpeg_timeout_handler);
            break;
        case EVN_POST_DRAW:
            win_mpeg_player_draw_infor();
            break;
        case EVN_PRE_CLOSE:
#ifdef MP_SUBTITLE_SUPPORT  //vic100507#1
            unload_subtitle();
#endif
#ifdef DLNA_DMP_SUPPORT
            g_stop_read_media = 1;
#endif
            mpg_cmd_stop_proc(0);
#ifdef DLNA_DMP_SUPPORT
            g_stop_read_media = 0;
#endif
            mpeg_file_prompt("",0);
            api_stop_timer(&mpeg_refresh_id);
            api_stop_timer(&mpeg_timeout_id);
            m_mpeg_player_working = FALSE;
            g_from_mpegplayer = TRUE;
#ifdef AUDIOTRACK_SUPPORT
            if(mp_audio_track_is_show())
            {
                osd_obj_close((POBJECT_HEAD)(&g_win_audio_track),C_CLOSE_CLRBACK_FLG);
                mp_set_audio_track_show(FALSE);
            }
#endif
            vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32)(&vpo_info));
            e_sys_tvmode = tv_mode_to_sys_data_ext(vpo_info.tvsys, vpo_info.bprogressive);
            if((TV_MODE_AUTO!= sys_tv_mode_bak)&&( e_sys_tvmode != sys_tv_mode_bak))
            {
                sys_data_set_tv_mode(sys_tv_mode_bak);
            }
            break;
	 case EVN_POST_CLOSE:
		back_to_mainmenu=0;
		break;
        case EVN_UNKNOWN_ACTION:
            api_stop_timer(&mpeg_timeout_id);
            unact = (VACTION)(param1>>16);
            v_key = param1 & 0x0000FFFF;

            MEMSET(&file_mp_info,0,sizeof(fileinfo_video));
            if((!b_vdec_support) && (!b_audio_support))
            {
                mpeginfo.total_time = 0;
            }
            //else
                //MPEGInfo.TotalTime = MPGGetTotalPlayTime();
            //BarState.nRange = MPEGInfo.TotalTime;
            mpg_file_get_stream_info(&file_mp_info);   //get the stream video info
            total_subtitle = file_mp_info.sub_stream_num;
            total_audio_track = file_mp_info.audio_stream_num;
            total_video_prog = file_mp_info.prog_num;

            MEMSET(&chapterinfo,0,sizeof(DEC_CHAPTER_INFO));
            chapterinfo.dst_chapter = -1;
            mpg_file_get_chapter_info(&chapterinfo);   //get the current chapter info
            total_chapter = chapterinfo.nb_chapter;

            ret = mpeg_player_unknown_act_proc(pobj, unact, param1, param2);
            mpeg_timeout_id = api_start_timer(MPEGBAR_TIMER_NAME, mpgbar_timer_time, mpeg_timeout_handler);
            break;
        case EVN_UNKNOWNKEY_GOT:
            break;
        case EVN_MSG_GOT:
            ret = win_mpeg_player_message_proc(pobj, param1, param2);
            break;
        default:
            break;
    }

    return ret;
}

static void win_mpegplayer_next_chapter(UINT32 next)
{
    UINT32 wait_time = 0;
    UINT32 total_chapter = 0;
    UINT32 dest_start_time = 0xFFFFFFFF;
    UINT32 cur_chapter = 1;
    UINT32 cur_time = 0;
    DEC_CHAPTER_INFO chapterinfo;

    MEMSET(&chapterinfo,0,sizeof(DEC_CHAPTER_INFO));
    chapterinfo.dst_chapter = -1;
    mpg_file_get_chapter_info(&chapterinfo);
    cur_chapter = chapterinfo.cur_chapter;
    total_chapter = chapterinfo.nb_chapter;
    cur_time = (INT32)mpgfile_decoder_get_play_time_ms();

    if(1 < total_chapter)
    {
        if(1 == next)  /* authored chapter */
        {
            //if the current chapter is the last one,can't switch to the first
            if((total_chapter-1) == cur_chapter)
            {
                return;
            }
            cur_chapter = (cur_chapter + 1)%total_chapter;
        }
        else if(0 == next)  //if the current chapter is the first one,can't switch to the last
        {
            if(0 == cur_chapter)
            {
                return;
            }

            cur_chapter = (cur_chapter + total_chapter - 1)%total_chapter;
        }
        chapterinfo.dst_chapter = cur_chapter;
        mpg_file_get_chapter_info(&chapterinfo);
        dest_start_time = chapterinfo.dst_start_time;
    }

    if(0xFFFFFFFF != dest_start_time)
    {
        if((dest_start_time) > cur_time)
        {
            mpg_cmd_search_ms_proc((DWORD)dest_start_time);
            while ((dest_start_time > mpgfile_decoder_get_play_time()) && (MP_TRICK_TIMEOUT > wait_time ))
            {
                osal_task_sleep(2);
                wait_time++;
            }
        }
        else
        {
            mpg_cmd_search_ms_proc((DWORD)dest_start_time);
            while ((dest_start_time+VIDEO_SEARCH_DLYS<mpgfile_decoder_get_play_time())&&(MP_TRICK_TIMEOUT>wait_time))
            {
                osal_task_sleep(2);
                wait_time++;
            }
        }
        play_proc();
    }
}

static PRESULT mp_exit_proc(void)
{
    PRESULT  ret = PROC_LOOP;

    if (m_hide_window)
    {
        if (m_b_set_vol)
        {
            //ShowVolumeBar(FALSE);//Close volume bar
            win_mpegplayer_close_video_vol();
            m_b_set_vol = FALSE;
        }
        else
        {
            ret = PROC_LEAVE;//exit video player
        }
    }
    else
    {
        if (MPEG_STOP == mpeginfo.play_state)
        {
            api_stop_timer(&mpeg_doubleclick_timeout_id);
            mpeg_click_state = DOUBLECLICK_INVALIDE;
            ret = PROC_LEAVE;//exit video player
        }
        else
        {
            api_stop_timer(&mpeg_timeout_id);
            win_mpeg_player_draw(FALSE);//Hide the information window
            if (CURSOR_MOVING == bar_state.state)
            {
                sync_bar_pos_and_cursor(POS_TO_CUR);
            }
        }
    }
    return ret;
}

static PRESULT mp_left_right_proc(UINT32 vkey)
{
    PRESULT ret = PROC_LOOP;
    UINT32 cursor_step = 0;

    if (m_hide_window)
    {
        win_mpeg_player_draw(FALSE);
        win_mpegplayer_set_video_vol(vkey);
        return ret;
    }

    if ((MPEG_STOP  != mpeginfo.play_state) && (MPEG_RESUME_STOP != mpeginfo.play_state) && (!b_build_idx_tbl))
    {
        cursor_step = (bar_state.n_range) >> 5;
        cursor_step=(cursor_step>SLIDE_BAR_STEP)?cursor_step:SLIDE_BAR_STEP;
        if (CURSOR_MOVING != bar_state.state)
        {
            bar_state.state = CURSOR_MOVING;
            bar_state.n_cursor = bar_state.n_pos;
        }
        if(V_KEY_LEFT == vkey)
        {
            if (bar_state.n_cursor > cursor_step)
            {
                bar_state.n_cursor -= cursor_step;
            }
            else if (0 == bar_state.n_cursor )
            {
                if(bar_state.n_range > cursor_step)
                {
                    bar_state.n_cursor = bar_state.n_range-cursor_step;
                }
                else
                {
                    bar_state.n_cursor = bar_state.n_range;
                }
            }
            else
            {
                bar_state.n_cursor = 0;
            }
        }
        else if(V_KEY_RIGHT == vkey)
        {
            if (bar_state.n_cursor + cursor_step < bar_state.n_range)
            {
                bar_state.n_cursor += cursor_step;
            }
            else
            {
                if(bar_state.n_range > cursor_step)
                {
                    bar_state.n_cursor = cursor_step;
                }
                else if(VIDEO_SEARCH_DLYS < bar_state.n_range)
                {
                    bar_state.n_cursor = bar_state.n_range - VIDEO_SEARCH_DLYS;
                }
                else
                {
                    if(0 == bar_state.n_cursor)
                    {
                        bar_state.n_cursor = bar_state.n_range;
                    }
                    else
                    {
                        bar_state.n_cursor = 0;
                    }
                }
            }
        }
    }
    win_mpeg_player_draw(TRUE);

    return ret;
}

static PRESULT win_mpeg_player_key_proc(POBJECT_HEAD pobj, UINT32 key, UINT32 param)
{
    PRESULT ret = PROC_LOOP;
    int wait_time = 0;
    int next_video = 0;

    switch (key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        ret = mp_exit_proc();
        break;
    case V_KEY_PLAY:
        if((MPEG_PLAY != mpeginfo.play_state) && (!b_build_idx_tbl))
        {
            b_resume_stop = TRUE;
            pivotal_key_proc();
            play_proc();
        }
        break;
    case V_KEY_PAUSE:
        if((MPEG_RESUME_STOP == mpeginfo.play_state) || (b_build_idx_tbl))
        {
            return PROC_LOOP;
        }
        pivotal_key_proc();
        pause_proc();
        break;
    case V_KEY_STOP:
        pivotal_key_proc();
        stop_proc();
        ret = PROC_LEAVE;
        break;
    case V_KEY_PREV:
    case V_KEY_NEXT:
    case V_KEY_UP:
    case V_KEY_DOWN:
        if((MPEG_STOP == mpeginfo.play_state) || (MPEG_RESUME_STOP == mpeginfo.play_state)
           || (0 == mpeginfo.play_time))
        {
            return PROC_LOOP;
        }
        if ((V_KEY_NEXT == key) || (V_KEY_DOWN == key))
        {
            next_video = get_next_index_from_play_list(cur_playlist_video);
        }
        else
        {
            next_video = get_previous_index_from_play_list(cur_playlist_video);
        }
        if(-1 != next_video)
        {
            cur_video_idx = next_video;
            // stop MPEG play
            pivotal_key_proc();
            stop_proc();
            set_play_list_info(cur_playlist_video,&cur_video_idx,NULL);
            mpeg_player_init();
        }
        break;
    case V_KEY_FF:
        if(1 > mpgfile_decoder_get_play_time())
        {
            break;
        }
        if ((MPEG_STOP == mpeginfo.play_state) || (MPEG_RESUME_STOP == mpeginfo.play_state)
            || (!b_vdec_support) || (0 == mpeginfo.total_time))
        {
            return PROC_LOOP;
        }
        else
        {
            pivotal_key_proc();
            fast_forward_proc();
        }
        break;
    case V_KEY_FB:
        if(mpgfile_decoder_get_play_time()<1)
        {
            break;
        }
        if ((MPEG_STOP==mpeginfo.play_state)||(MPEG_RESUME_STOP==mpeginfo.play_state)||(!b_vdec_support))
        {
            return PROC_LOOP;
        }
        else
        {
            pivotal_key_proc();
            fast_backward_proc();
        }
        break;
    case V_KEY_SLOW:
        if(mpgfile_decoder_get_play_time()<1)
        {
            break;
        }
        if ((MPEG_STOP==mpeginfo.play_state)||(MPEG_RESUME_STOP==mpeginfo.play_state)||(!b_vdec_support))
        {
            return PROC_LOOP;
        }
        else
        {
            pivotal_key_proc();
            slow_forward_proc();
        }
        break;
    case V_KEY_LEFT:
        ret = mp_left_right_proc(V_KEY_LEFT);
        break;
    case V_KEY_RIGHT:
        ret = mp_left_right_proc(V_KEY_RIGHT);
        break;
    case V_KEY_ENTER:
        if ((MPEG_STOP != mpeginfo.play_state) &&  (MPEG_RESUME_STOP != mpeginfo.play_state))
        {
            if (CURSOR_MOVING == bar_state.state)
            {
                if (bar_state.n_pos < (INT32)bar_state.n_cursor)//forward
                {
                    bar_state.cursor_dir = 1;
                }
                else if (bar_state.n_pos > (INT32)bar_state.n_cursor)//backward
                {
                    bar_state.cursor_dir = 2;
                }
                else
                {
                    bar_state.cursor_dir = 0;
                }

                sync_bar_pos_and_cursor(CUR_TO_POS);
                mpeginfo.play_time = bar_state.n_pos;

                if (CURSOR_DIRECT_FROWARD == bar_state.cursor_dir)//forward
                {
                    mpg_cmd_search_proc(mpeginfo.play_time);
                    while((mpeginfo.play_time>(UINT32)mpgfile_decoder_get_play_time())&&(wait_time < MP_TRICK_TIMEOUT))
                    {
                        osal_task_sleep(2);
                        wait_time++;
                    }
                }
                else if (CURSOR_DIRECT_BACKWARD== bar_state.cursor_dir)//backward
                {
                    mpg_cmd_search_proc(mpeginfo.play_time);
                    while((mpeginfo.play_time + VIDEO_SEARCH_DLYS<(UINT32)mpgfile_decoder_get_play_time())
                            &&(wait_time < MP_TRICK_TIMEOUT))
                    {
                        osal_task_sleep(2);
                        wait_time++;
                    }
                }
                play_proc();
            }
        }
        break;
    case V_KEY_INFOR:
    case V_KEY_PVR_INFO:
        m_hide_window = !m_hide_window;
        if (!m_hide_window)
        {
            win_mpeg_player_draw(TRUE);
        }
        else
        {
            win_mpeg_player_draw(FALSE);
            if (CURSOR_MOVING == bar_state.state )
            {
                sync_bar_pos_and_cursor(POS_TO_CUR);
            }
        }
        break;
    case V_KEY_MUTE:
        break;
    case V_KEY_YELLOW:
        win_mpeg_player_draw(FALSE);
        win_mpegplayer_chapter_set();
        ret = PROC_LOOP;
        break;
#ifdef MP_SUBTITLE_SUPPORT
    case V_KEY_SUBTITLE:
    case V_KEY_GREEN:
        //if (m_HideWindow)
        {
            win_mpeg_player_draw(FALSE);

            if(mpeg_prompt_on_screen != 0)
            {              
                mpeg_file_prompt(NULL, 0);
            }
            win_mpegplayer_subt_set();
            return PROC_LOOP;
        }
        break;
#endif
    default:
        break;
    }
    return ret;
}

static PRESULT win_mpeg_player_message_proc(POBJECT_HEAD pobj, UINT32 msg_type, UINT32 msg_code)
{
#ifdef UI_MEDIA_PLAYER_REPEAT_PLAY
    struct vdec_status_info vdec_status;
    BOOL oldsave = FALSE;
    int next_video = -1;
#endif
    PRESULT ret = PROC_LOOP;
    __MAYBE_UNUSED__ UINT32 hkey = 0;

    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_STO:
            if(USB_STATUS_OVER == msg_code)
            {
                //storage_dev_mount_hint(1);
                file_list_check_storage_device(FALSE, FALSE);
            }
            break;

        case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
            if((!b_vdec_support) && (!b_audio_support))//add on 2011-09-28 fixBUG42869
            {
                stop_proc();
                break;
            }
            mpeginfo.play_time = (UINT32)mpgfile_decoder_get_play_time();
            if(mpeginfo.play_time >= mpeginfo.total_time)
            {
                bar_state.n_pos = mpeginfo.total_time - 1;
            }
            else
            {
                bar_state.n_pos = mpeginfo.play_time;
            }

            if (0 > bar_state.n_pos)
            {
                bar_state.n_pos = 0;
            }

            if (!m_hide_window)
            {
                win_mpeg_player_draw(TRUE);
            }

            if((mpeginfo.play_time > 0) && (MPEG_PROMPT_STATUS_TRYING_TO== mpeg_prompt_on_screen))
            {
                mpeg_file_prompt(NULL, 0);
            }

            mpeg_refresh_cnt++;
            if (((9000 / MP_WIN_REFRESH_T) == mpeg_refresh_cnt)  ||
                ((12000 / MP_WIN_REFRESH_T) == mpeg_refresh_cnt) ||
                ((16000 / MP_WIN_REFRESH_T) == mpeg_refresh_cnt))
            {
                if (MPEG_PROMPT_STATUS_VIDEO_SUPPORT == mpeg_prompt_on_screen)
                {
                    mpeg_file_prompt(NULL, 0);
                }
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER: //video over
            if(MPEG_FB == mpeginfo.play_state)
            {
                if(1 == msg_code)
                {
                    stop_proc();
                }
                play_proc();
                mpg_cmd_search_proc(0);
            }
            else
#ifdef UI_MEDIA_PLAYER_REPEAT_PLAY
            {
                mpeg_timeout_id = api_start_timer(MPEGBAR_TIMER_NAME, mpgbar_timer_time, mpeg_timeout_handler);
                vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), \
                    VDEC_IO_GET_STATUS, (UINT32)(&vdec_status));

                UINT8 bpreview = (msg_code>>16) &1;
                if(bpreview)
                {
                    libc_printf("(1)ignore the msg CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER from preview\n");
                    break;
                }

                if(init_video_idx == cur_video_idx && (osal_get_tick() - init_video_start_tick)<1000)
                {
                    libc_printf("(2)ignore the msg CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER from preview\n");
                    break;
                }

                if(PLAY_LIST_NONE == win_filelist_get_loop_mode())
                {
                        ret = PROC_LEAVE;
                }
                else if(PLAY_LIST_REPEAT == win_filelist_get_loop_mode())
                {
                    next_video = get_next_index_from_play_list(cur_playlist_video);
                    if(next_video != -1)
                    {
                        cur_video_idx = next_video;
                        // stop MPEG play
                        pivotal_key_proc();
                        stop_proc();
                        set_play_list_info(cur_playlist_video,&cur_video_idx,NULL);
                        mpeg_player_init();
                    }
                }
                else   
                {
                    if((mp_audio_track_is_show()) && (cur_video_idx != (UINT16)(~0)))
                    {
                        ap_vk_to_hk(0,V_KEY_EXIT,&hkey);
                        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
                    }
                    oldsave = ap_enable_key_task_get_key(TRUE);
                    osal_task_sleep(3000);
                    oldsave = ap_enable_key_task_get_key(oldsave);// recover;

                    if (cur_video_idx != (UINT16)(~0))
                    {
                        if(VDEC_FORWARD == vdec_status.api_play_direction)
                        {
                            mpg_cmd_stop_proc(0);
                        }
                        mpeginfo.play_state = MPEG_STOP;
                        api_stop_timer(&mpeg_refresh_id);
                        m_mpeg_player_working = FALSE;
                        mpeg_player_init();
                        m_mpeg_player_working = TRUE;
                    }
                    else
                    {
                        ret = PROC_LEAVE;
                    }
                }
            }
#else
            {
                ret = PROC_LEAVE;
            }
#endif
            break;
        case CTRL_MSG_SUBTYPE_STATUS_TIMEOUT:   //mpeg player display timer out
            if ((!m_hide_window) && (!b_build_idx_tbl))
            {
                win_mpeg_player_draw(FALSE);
                if (CURSOR_MOVING == bar_state.state)
                {
                    sync_bar_pos_and_cursor(POS_TO_CUR);
                }
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_BUILD_IDX:
            if(!msg_code)
            {
                b_build_idx_tbl = TRUE;
                mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
            }
            else
            {
                b_build_idx_tbl = FALSE;
                if(MPEG_PLAY == mpeginfo.play_state)
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_TIMEOUT, 0, FALSE);
                }
            }
            mpeg_file_prompt("Building index table...",!msg_code);
            break;
        case CTRL_MSG_SUBTYPE_STATUS_VIDEO_DEC_ERR:
        case CTRL_MSG_SUBTYPE_STATUS_PARSE_END:
            mpeg_refresh_cnt = 0;
            mpeginfo.total_time = get_video_total_time();
            bar_state.n_range = mpeginfo.total_time;
            mpeg_unsupport_detect(msg_code);
    #ifdef AV_DELAY_SUPPORT
            if(b_vdec_support && b_audio_support)
            {
                SYSTEM_DATA *psys_data = sys_data_get();
                if(psys_data->avset.avdelay_value > AV_DELAY_MIDPOINT)
                {
                    mpg_set_avsync_delay(1, psys_data->avset.avdelay_value-AV_DELAY_MIDPOINT); //0:video; 1:audio.
                }
                else if((psys_data->avset.avdelay_value>0) && (psys_data->avset.avdelay_value < AV_DELAY_MIDPOINT))
                {
                    mpg_set_avsync_delay(0, psys_data->avset.avdelay_value); //0:video; 1:audio.
                }
            }
    #endif
            break;
        case CTRL_MSG_SUBTYPE_NEXT_CHAPTER:
            win_mpegplayer_next_chapter(msg_code);
            break;
#ifdef NETWORK_SUPPORT
        case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH:
            if (-(NET_ERR_CONNECT_FAILED) == (INT32)msg_code)
            {
                ap_send_key(V_KEY_EXIT, FALSE);
            }
            break;
#endif
        default:
            break;
    }

    return ret;
}

/*******************************************************************************
*   Ancillary functions
*******************************************************************************/
static UINT32 get_video_total_time(void)
{
    UINT32 tot_time = 0;

    if (mpgget_total_play_time() < 0)//not a legal time
    {
        tot_time = 0;
    }
    else
    {
        tot_time = (UINT32)mpgget_total_play_time();
    }

#ifdef DLNA_DMP_SUPPORT
    if (tot_time <= 0)
    {
        tot_time = (UINT32)dmp_get_duration();
    }
#endif

    return tot_time;
}

static void start_refresh_timer(void)
{
    if (mpeg_prompt_on_screen)
    {
        mpeg_file_prompt(NULL, 0);
    }
    api_stop_timer(&mpeg_refresh_id);
    mpeg_refresh_id = api_start_cycletimer("M", MP_WIN_REFRESH_T, mpeg_refresh_handler);
    mpeg_refresh_cnt = 0;
}

static int mpeg_player_play_file(media_file_info *file_node)
{
    int ret = -1;
    char *name = NULL;

    name = (char *)malloc(FULL_PATH_SIZE);

    if(NULL == name)
    {
        return ret;
    }
    if (NULL == file_node)
    {
        free(name);
        return -1;
    }

    win_get_fullname(name, FULL_PATH_SIZE, file_node->path, file_node->name);

#ifdef DLNA_DMP_SUPPORT
    if (mp_is_dlna_play())
    {
        win_dlna_play_preset(name, 0);
    }
#endif
    ret = video_dec_file(name, 0);
    if(name)
    {
        free(name);
    }
    return ret;
}

static BOOL mpeg_player_init(void)
{
    PROGRESS_BAR *bar = NULL;
    RET_CODE ret = 0;
    media_file_info file_node;

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    MEMSET(&bar_state, 0, sizeof(bar_state));
    bar_state.n_range = 100;
    bar_state.state = NORMAL_MOVING;

    bar = &mpeg_player_bar;
    osd_set_progress_bar_value(bar, 100);
    osd_set_progress_bar_range(bar, 0, 100);

    MEMSET(&mpeginfo, 0, sizeof(mpeginfo));
    mpeginfo.play_time = 0;
    mpeginfo.total_time = 0;

    mpeginfo.play_speed = 0;

    m_hide_window = FALSE;
    mpeg_timeout_id = OSAL_INVALID_ID;
    b_build_idx_tbl = FALSE;
    b_vdec_support = TRUE;
    b_audio_support = TRUE;
    if (get_mute_state())
    {
        show_mute_on_off();
    }

    start_refresh_timer();
    ret = get_file_from_play_list(cur_playlist_video, cur_video_idx, &file_node);
    if(RET_FAILURE == ret)
    {
        return FALSE;
    }
    mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_TRYING_TO;
    mpeg_file_prompt("Loading, please wait...", TRUE);
    mpeg_player_play_file(&file_node);    
    
    init_video_start_tick = osal_get_tick();

    mpeginfo.total_time = 0; // we will get total time after parsing end
    mpeginfo.play_state = MPEG_PLAY;
    bar_state.n_range = mpeginfo.total_time;

#ifdef MP_SUBTITLE_SUPPORT
    install_ext_subtitle_plugin();
    auto_load_subtitle();
#endif

//  mpeg_file_prompt(NULL, FALSE);
#ifndef DUAL_ENABLE
    SUBT_INPUT_PLUGIN  *subt_plugin = NULL;
    subt_plugin = get_subt_decore("SUBT");
    if(subt_plugin != NULL)
    {
        subt_plugin->subtitle_control(NULL,MP_SUBTITLE_IO_CTRL_EXT,0);
    }
#endif
    return TRUE;
}

static void sync_bar_pos_and_cursor(UINT8 cur2pos)
{
    if (CUR_TO_POS == cur2pos)
    {
        bar_state.n_pos = bar_state.n_cursor;
    }
    else if (POS_TO_CUR == cur2pos)
    {
        bar_state.n_cursor = bar_state.n_pos;
    }

    bar_state.state = NORMAL_MOVING;
}

static void fast_backward_proc(void)
{
    if ((MPEG_PAUSE == mpeginfo.play_state) || (MPEG_STEP == mpeginfo.play_state))
    {
        start_refresh_timer();
    }

    if (MPEG_FB != mpeginfo.play_state)
    {
        mpeginfo.play_state = MPEG_FB;
        mpeginfo.play_speed = 0;
    }
    else
    {
        mpeginfo.play_speed++;
        mpeginfo.play_speed %= MAX_FP_TIMES;
    }
#ifdef MP_SUBTITLE_SUPPORT
    mps_hide_subtitle(TRUE);
#endif
    mpg_cmd_fb_proc();
}

static void fast_forward_proc(void)
{
    if ((MPEG_PAUSE == mpeginfo.play_state) || (MPEG_STEP == mpeginfo.play_state))
    {
        start_refresh_timer();
    }

    if (MPEG_FF != mpeginfo.play_state)
    {
        mpeginfo.play_state = MPEG_FF;
        mpeginfo.play_speed = 0;
    }
    else
    {
        mpeginfo.play_speed++;
        mpeginfo.play_speed %= MAX_FP_TIMES;
    }
#ifdef MP_SUBTITLE_SUPPORT
    mps_hide_subtitle(TRUE);
#endif
    mpg_cmd_ff_proc();
}

static void slow_forward_proc(void)
{
    if ((MPEG_PAUSE == mpeginfo.play_state) || (MPEG_STEP == mpeginfo.play_state))
    {
        start_refresh_timer();
    }

    if (MPEG_SF != mpeginfo.play_state)
    {
        mpeginfo.play_state = MPEG_SF;
        mpeginfo.play_speed = 0;
    }
    else
    {
        mpeginfo.play_speed++;
        mpeginfo.play_speed %= MAX_SP_TIMES;
        mpeginfo.play_state = MPEG_SF;
    }
    mpg_cmd_slow_proc();
}


static void stop_proc(void)
{
    if (MPEG_STOP != mpeginfo.play_state)
    {
        api_stop_timer(&mpeg_refresh_id);
#ifdef MP_SUBTITLE_SUPPORT
        unload_subtitle();
#endif
        mpg_cmd_stop_proc(0);
        mpeginfo.play_state = MPEG_STOP;
        mpeginfo.play_speed = 0;
        mpeginfo.play_time = 0;
        bar_state.n_pos = 0;
        bar_state.n_cursor = 0;
    }
}

 void play_proc(void)
{
    RET_CODE ret = 0;
    media_file_info file_node;

	if(0 == ret)
	{
		;
	}
	MEMSET(&file_node, 0x0, sizeof(media_file_info));
    if ((MPEG_PAUSE <= mpeginfo.play_state ) && (MPEG_RESUME_STOP >= mpeginfo.play_state))
    {
#ifdef MP_SUBTITLE_SUPPORT
        mps_hide_subtitle(!get_mp_subtitle_show_status());
#endif
        if(0==mpg_cmd_play_proc())
        {
            mpeginfo.play_state = MPEG_PLAY;
            mpeginfo.play_speed = 0;
            start_refresh_timer();
        }
    }
    else if (MPEG_STOP == mpeginfo.play_state)
    {
        api_stop_timer(&mpeg_refresh_id);
        mpeg_refresh_id = api_start_cycletimer("M", MP_WIN_REFRESH_T, mpeg_refresh_handler);
        ret = get_file_from_play_list(cur_playlist_video, cur_video_idx, &file_node);
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_TRYING_TO;
        mpeg_file_prompt("Loading, please wait...", TRUE);
        ret = mpeg_player_play_file(&file_node);

#ifdef MP_SUBTITLE_SUPPORT
        auto_load_subtitle();
#endif
        mpeginfo.play_state = MPEG_PLAY;
        mpeginfo.play_speed = 0;
        start_refresh_timer();
    }
}

static void pause_proc(void)
{
    api_stop_timer(&mpeg_refresh_id);
    if (((MPEG_PAUSE == mpeginfo.play_state) || (MPEG_STEP == mpeginfo.play_state)) && b_vdec_support)
    {
        mpg_cmd_resume_proc();
#ifdef MP_SUBTITLE_SUPPORT
        mps_hide_subtitle(!get_mp_subtitle_show_status());
#endif
        mpeginfo.play_state = MPEG_STEP;
    }
    else if (MPEG_STOP != mpeginfo.play_state)
    {
        mpg_cmd_pause_proc();
        mpeginfo.play_state = MPEG_PAUSE;
    }
    mpeginfo.play_speed = 0;
    mpeginfo.play_time = (UINT32)mpgfile_decoder_get_play_time();
    bar_state.n_pos = (INT32)(mpeginfo.play_time<=mpeginfo.total_time?mpeginfo.play_time:mpeginfo.total_time);

    if (NORMAL_MOVING == bar_state.state)
    {
        bar_state.n_cursor = bar_state.n_pos;
    }
}

static void pivotal_key_proc(void)
{//if the window is hidden && if the volume var is showing , means we are setting volume
    if (m_hide_window && m_b_set_vol)
    {
        win_mpeg_player_draw(TRUE);
    }
}

static void win_mpeg_player_draw_infor(void)
{
    BITMAP *bmp = NULL;
    TEXT_FIELD *txt = NULL;
    PROGRESS_BAR *bar = NULL;
    mpegplayer_bmp_t *bmp_desc = NULL;
    mpegplayer_txt_t *txt_desc = NULL;
    UINT32 i = 0;
    UINT16 icon = 0;
    char string[30] = {0};
    UINT32 hh = 0;
    UINT32 mm = 0;
    UINT32 ss = 0;
    UINT32 play_pos = 0;
    media_file_info file_node;
    __MAYBE_UNUSED__ char name_buffer[MAX_FILE_NAME_SIZE + 1] = {0};
    #ifdef PERSIAN_SUPPORT
    UINT16 str[64] = {0};
    char str_uni[128] = {0};
    #endif
    UINT16 rect_left = 0;

    MEMSET(&file_node,0,sizeof(media_file_info));
    bmp = &mpeg_player_bmp;
    txt = &mpeg_player_text;
    bar = &mpeg_player_bar;

    for (i = 0; i < MPEG_BMP_NUM; i++)
    {
        bmp_desc = &mpegplayer_bmps[i];
        icon = INVALID_ID;
        switch (bmp_desc->bmp_type)
        {
        case MPEG_BMP_STATIC:
            //icon = bmp_desc->icon;
            break;
        case MPEG_BMP_PLAY_RECORD:
            icon = bmp_desc->icon;
            break;
        case MPEG_BMP_PLAY_MODE:
            if (mpeginfo.play_state < sizeof(mpegplayer_status_icons) / 2)
            {
                icon = mpegplayer_status_icons[mpeginfo.play_state];
            }
            break;
        case MPEG_BMP_HDD_STATUS:
            break;
        default:
            break;
        }

        osd_set_bitmap_content(bmp, icon);//two sign
        osd_set_rect(&bmp->head.frame, bmp_desc->left, bmp_desc->top, bmp_desc->width, bmp_desc->height);
        if((MPEG_BMP_PLAY_MODE == bmp_desc->bmp_type)&&(!osd_get_vscr_state()))
        {
            osd_hide_object((OBJECT_HEAD*) bmp, C_UPDATE_ALL);
        }
        osd_draw_object( (OBJECT_HEAD*) bmp, C_UPDATE_ALL);
    }

    for (i = 0;i < MPEG_TXT_NUM;i++)
    {
        txt_desc = &mpegplayer_txts[i];

        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");

        switch (txt_desc->txt_type)
        {
        case MPEG_TXT_PLAY_NAME:
            get_file_from_play_list(cur_playlist_video, cur_video_idx, &file_node);
            #ifdef PERSIAN_SUPPORT
            filelist_utf8_to_unicode(file_node.name, str_uni);
            com_uni_str_copy_char( (UINT8 *)str, (UINT8 *)str_uni);
            osd_set_text_field_content(txt, STRING_UNICODE,(UINT32)str);
            #else
            win_shorten_filename(file_node.name,name_buffer, MAX_FILE_NAME_SIZE+1);
            snprintf(name_buffer, MAX_FILE_NAME_SIZE+1,"%s", name_buffer);
            osd_set_text_field_content(txt, STRING_UTF8, (UINT32)name_buffer);
            #endif
            break;
        case MPEG_TXT_DATE:
            strncpy(string, "", 1);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;
        case MPEG_TXT_TIME:
            strncpy(string, "", 1);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;
        case MPEG_TXT_PLAY_MODE:
            strncpy(string, "", 1);
            if (MPEG_STOP == mpeginfo.play_state)
            {
                snprintf(string, 30, "%s", "Stop");
            }
            else if (MPEG_PLAY == mpeginfo.play_state)
            {
                snprintf(string, 30, "%s", "Play");
            }
            else if (MPEG_PAUSE == mpeginfo.play_state)
            {
                snprintf(string, 30, "%s", "Pause");
            }
            else if ((MPEG_FF == mpeginfo.play_state) || (MPEG_FB == mpeginfo.play_state))
            {
                snprintf(string, 30, "X%d", mpegspeed_steps[mpeginfo.play_speed]);
            }
            else if ((MPEG_SF == mpeginfo.play_state) || (MPEG_SB == mpeginfo.play_state))
            {
                snprintf(string, 30, "X1/%d", mpegspeed_steps[mpeginfo.play_speed]);
            }
            else if(MPEG_STEP == mpeginfo.play_state)
            {
                snprintf(string, 30, "%s","Step");
            }
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;
        case MPEG_TXT_PLAY_1ST_TIME:
            strncpy(string, "", (30-1));
            mpeginfo.play_time = (UINT32)mpgfile_decoder_get_play_time();
            if (MPEG_STOP == mpeginfo.play_state)
            {
                mpeginfo.play_time = 0;
            }
           
            hh = mpeginfo.play_time / 3600;
            mm = (mpeginfo.play_time % 3600) / 60;
            ss = mpeginfo.play_time % 60;
            snprintf(string, 30, "%02lu:%02lu:%02lu", hh, mm, ss);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;

        case MPEG_TXT_PLAY_2ND_TIME:
            strncpy(string, "", (30-1));
            hh = mpeginfo.total_time / 3600;
            mm = (mpeginfo.total_time % 3600) / 60;
            ss = mpeginfo.total_time % 60;
            snprintf(string, 30, "%02lu:%02lu:%02lu", hh, mm, ss);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)string);
            break;
        case MPEG_TXT_HDD_STATUS:
            break;
        default:
            break;
        }

        osd_set_rect(&txt->head.frame, txt_desc->left, txt_desc->top, txt_desc->width, txt_desc->height);//all font
        txt->head.style.b_show_idx = txt_desc->shidx;
        osd_draw_object( (OBJECT_HEAD *)txt, C_UPDATE_ALL);
    }

    /* Draw progress bar */
    osd_set_progress_bar_pos(bar, 100);
    #ifndef SD_UI
    osd_set_rect(&bar->head.frame, BAR_L, BAR_T, BAR_W, 16);
    bar->rc_bar.u_left  = 0;
    bar->rc_bar.u_top       = 0;
    bar->rc_bar.u_width = BAR_W;
    bar->rc_bar.u_height    = 16;//16 - 6;
    bar->w_tick_fg      = WSTL_PLAYLIST_TICK_FG;
    #else
    osd_set_rect(&bar->head.frame, BAR_L, BAR_T, BAR_W, 16);
    bar->rc_bar.u_left  = 2;
    bar->rc_bar.u_top       = 2;
    bar->rc_bar.u_width = BAR_W - 4;
    bar->rc_bar.u_height    = 12;//16 - 6;
    bar->w_tick_fg      = WSTL_BAR_06;
    #endif
    osd_draw_object((POBJECT_HEAD)bar, C_UPDATE_ALL);

    if (bar_state.n_range)
    {
        play_pos = bar_state.n_pos * 100 / bar_state.n_range;
    }
    rect_left = bar->head.frame.u_left+6 +
            (bar->head.frame.u_width-12)*play_pos/100*osd_get_progress_bar_pos(bar)/100 - POS_ICON_WIDTH / 2;
    osd_set_rect(&bmp->head.frame, rect_left, bar->head.frame.u_top, POS_ICON_WIDTH, bar->head.frame.u_height);
    osd_set_bitmap_content(bmp, IM_PVR_DOT);
    osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);

    if (CURSOR_MOVING == bar_state.state)   /* Draw play cursor */
    {
        if (bar_state.n_range)
        {
            if (MPEG_STOP == mpeginfo.play_state)
            {
                bar_state.n_cursor = 0;
            }
            play_pos = bar_state.n_cursor * 100 / bar_state.n_range;
        }
        rect_left = bar->head.frame.u_left+6+(bar->head.frame.u_width-4)*play_pos/100*osd_get_progress_bar_pos(bar)/100
                    - POS_ICON_WIDTH / 2;
        osd_set_rect(&bmp->head.frame, rect_left,bar->head.frame.u_top, POS_ICON_WIDTH, bar->head.frame.u_height);
        osd_set_bitmap_content(bmp, IM_PVR_DOT_ORANGE);
        osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
    }
}

void win_mpeg_player_draw(BOOL bshow)
{
    POBJECT_HEAD pobj = (POBJECT_HEAD)&g_win_mpeg_player;

    if (bshow)
    {
        if(!mp_audio_track_is_show())
        {
            m_hide_window = FALSE;

            if(update_mpeg_infor&&(!osd_get_vscr_state()))
            {
                win_mpeg_player_draw_infor();
            }
            else
            {
                update_mpeg_infor=TRUE;
                osd_draw_object(pobj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            }
        }
    }
    else
    {
        update_mpeg_infor=FALSE;
        m_hide_window =TRUE;
        osd_clear_object(pobj, C_UPDATE_ALL);
    }
}

static void mpeg_refresh_handler(UINT32 nouse)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE, 0, FALSE);
}

static void mpeg_timeout_handler(UINT32 nouse)
{
    api_stop_timer(&mpeg_timeout_id);

    if(MPEG_PLAY == mpeginfo.play_state)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_TIMEOUT, 0, FALSE);
    }
}

static void win_mpegplayer_set_video_vol(UINT32 key)
{
    UINT32 hkey = 0;

    osd_obj_open((POBJECT_HEAD)&g_win2_volume, MENU_OPEN_TYPE_KEY + key);
    menu_stack_push((POBJECT_HEAD) &g_win2_volume);
    ap_vk_to_hk(0, key, &hkey);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
}

static void win_mpegplayer_close_video_vol(void)
{
    osd_obj_close((POBJECT_HEAD)&g_win2_volume, C_UPDATE_ALL);
}
static UINT32 mpeg_unsupport_detect(UINT32 para)
{
    UINT32 detect_ret = 0;
    char str_prompt[32] = {0};

    MEMSET(&file_mp_info,0x00,sizeof(fileinfo_video));
    mpg_file_get_stream_info(&file_mp_info);

    detect_ret = 0;
    if(para&MP_DERR_UNSUPPORTED_VIDEO_CODEC)
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        b_vdec_support = FALSE;
        snprintf(str_prompt, 32, "Video codec not support!");
    }
    else if(para&MP_DERR_UNSUPPORTED_RESOLUTION)
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        b_vdec_support = FALSE;
        snprintf(str_prompt, 32, "Video Resolution not support!");
    }
    else if(para&MP_DERR_UNSUPPORTED_FRAME_RATE)
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        b_vdec_support = FALSE;
        snprintf(str_prompt, 32, "Video Frame Rate not support!");
    }
    else if(para&MP_DERR_TOO_BIG_INTERLEAVE)
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_VIDEO_SUPPORT; // clear in CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE
        b_vdec_support = TRUE;
        snprintf(str_prompt, 32, "Video INTERLEAVE too big!");
    }
//  else if(para&MP_DERR_MP4_DRM_AUTH)
//  {
//      mpeg_prompt_on_screen = 2;
//      b_vdec_support = FALSE;
//      sprintf(str_prompt,"DRM access failed!");
//  }
    else if(MP_DERR_UNKNOWN == para)
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        b_vdec_support = FALSE;
        snprintf(str_prompt, 32, "Unknown error!");   //sprintf(str_prompt,"Unknown error!");
    }
    else if(para&MP_DERR_UNSUPPORTED_AUDIO_CODEC)
    {
        b_vdec_support = TRUE;
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_VIDEO_SUPPORT;
        b_audio_support = FALSE;

        if(0 == file_mp_info.audio_stream_num)
        {
            snprintf(str_prompt, 32, "Audio Format not supported!");  //FIXBUG42870 on 2011-09-27
        }
        else
        {
            snprintf(str_prompt, 32, "Audio Format not supported!");
        }
    }
    else //no error
    {
        b_vdec_support = TRUE;
        b_audio_support = TRUE;
        MEMSET(str_prompt, 0, 32);  //snprintf(str_prompt, 32, "\0");
    }

    if((!b_vdec_support) && (para&MP_DERR_UNSUPPORTED_AUDIO_CODEC))//both a/v not support
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        b_vdec_support = FALSE;
        b_audio_support = FALSE;
        snprintf(str_prompt, 32, "Unsupport file!");
    }

    #ifdef MP_SUBTITLE_SUPPORT
    if(0 == file_mp_info.sub_stream_num)
    {
        if(!b_vdec_support)
        {
            unload_subtitle();
        }
        else
        {
            install_ext_subtitle_plugin();
            auto_load_subtitle();
        }
    }
    #endif

    if(NO_MPEG_PROMPT_STR_LEN < strlen(str_prompt))
    {
        mpeg_file_prompt(str_prompt,1);
        //osal_task_sleep(2000);
        detect_ret = (UINT32)(~0);
    }

    return detect_ret;
}

void mpeg_file_prompt(char *str_prompt,UINT32 param)
{
    UINT16  uni_prompt[32] = {0};

#ifndef SD_UI
    DEF_TEXTFIELD(txtprompt,NULL,NULL,C_ATTR_ACTIVE,C_FONT_2, \
    0,0,0,0,0, 300,250,500,50, WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07, NULL,NULL,\
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,uni_prompt)
#else
    DEF_TEXTFIELD(txtprompt,NULL,NULL,C_ATTR_ACTIVE,C_FONT_2, \
    0,0,0,0,0, 200,230,250,40, WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07,WSTL_TEXT_07, NULL,NULL,\
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,uni_prompt)
#endif

    if(0 < param)
    {
        if (MPEG_PROMPT_STATUS_NULL == mpeg_prompt_on_screen)
        {
            mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_VIDEO_SUPPORT;
        }
        osd_set_text_field_content(&txtprompt,STRING_ANSI,(UINT32)str_prompt);
        osd_draw_object((POBJECT_HEAD)&txtprompt,C_UPDATE_ALL);
    }
    else
    {
        mpeg_prompt_on_screen = MPEG_PROMPT_STATUS_AV_NOTSUPPORT;
        osd_clear_object((POBJECT_HEAD)&txtprompt,C_UPDATE_ALL);
    }
}

void mp_set_update_mpeg_infor_flag(BOOL b_flag)
{
    update_mpeg_infor = b_flag?TRUE:FALSE;
}
BOOL mp_get_update_mpeg_info_flag(void)
{
    return update_mpeg_infor;
}

play_list_handle mp_get_cur_playlist_video(void)
{
    return cur_playlist_video;
}
UINT16 mp_get_cur_video_idx(void)
{
    return cur_video_idx;
}
UINT8 mp_get_mpeg_prompt_on_screen(void)
{
    return mpeg_prompt_on_screen;
}
BOOL mp_get_b_build_idx_tbl(void)
{
    return b_build_idx_tbl;
}
RET_CODE win_set_mpegplay_param(play_list_handle *pplaylist, UINT32 idx, UINT8 folder_flag)
{
    UINT8 back_saved = 0;
    RET_CODE retcode = RET_SUCCESS;
    media_file_info filenode;
    char *video_name = NULL;
    int playlist_idx = 0;
    play_list_loop_type lp_mode = PLAY_LIST_NONE;
    file_list_handle t_cur_filelist = NULL;

    video_name = (char *)malloc(FULL_PATH_SIZE);
    if(NULL == video_name)
    {
        return RET_FAILURE;
    }
    MEMSET(&filenode, 0x0, sizeof(media_file_info));
    video_folderloop_flag = folder_flag;
    if(1 == video_folderloop_flag)
    {
        if(*pplaylist != NULL)
        {
            if(RET_SUCCESS != delete_play_list(*pplaylist))
            {
                free(video_name);
                video_name = NULL;
                return RET_FAILURE;
            }
        }
        *pplaylist = create_play_list(VIDEO_PLAY_LIST, (const char*)"folder_video");
        if(0 == *pplaylist)
        {
            free(video_name);
            video_name = NULL;
            return RET_FAILURE;
        }

        t_cur_filelist = win_filelist_get_cur_filelist();
        get_file_from_file_list(t_cur_filelist, idx, &filenode);
        win_get_fullname(video_name, FULL_PATH_SIZE, filenode.path, filenode.name);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(200, 200, 200, 100);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_PLEASE_WAIT);
        win_compopup_open_ext(&back_saved);
        retcode = add_all_file_list_items_to_play_list(*pplaylist, t_cur_filelist);
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        if(RET_SUCCESS != retcode)
        {
            free(video_name);
            video_name = NULL;
            return RET_FAILURE;
        }
        playlist_idx = get_play_list_index_by_name(*pplaylist, video_name);
        if(-1 == playlist_idx)
        {
            free(video_name);
            video_name = NULL;
            return RET_FAILURE;
        }
        cur_playlist_video = *pplaylist;
        cur_video_idx = playlist_idx;
    }
    else
    {
        cur_playlist_video = *pplaylist;
        cur_video_idx = idx;
    }
    set_play_list_info(cur_playlist_video, &cur_video_idx, NULL); 
    init_video_idx = cur_video_idx;

    if(NULL != cur_playlist_video)
    {
        lp_mode = win_filelist_get_loop_mode();
        set_play_list_info(cur_playlist_video, NULL, &lp_mode);
        win_filelist_set_loop_mode(lp_mode);
    }

    if(video_name)
    {
        free(video_name);
        video_name = NULL;
    }
    return RET_SUCCESS;
}

#ifdef AUDIOTRACK_SUPPORT
void video_prog_display_set(BOOL flag)
{
    UINT32 video_cnt = 0;
    UINT8  str[50] = {0};
    UINT8  back_saved = 0;
    PTEXT_FIELD ptxt = NULL;

	if(0 == back_saved)
	{
		;
	}
    if(MPEG_PROMPT_STATUS_NULL != mp_get_mpeg_prompt_on_screen()) //fix bug:When switch track£¬clear the pop up message
    {                                          // because of not support last track decode. by Wu Jianwen,2010.8.18
        mpeg_file_prompt(NULL, 0);
    }
    MEMSET(&file_mp_info,0x00,sizeof(fileinfo_video));
    mpgget_total_play_time();
    mpg_file_get_stream_info(&file_mp_info);
    cur_prog = file_mp_info.cur_prog_id; //get actual prog ID,by Wu Jianwen,2010.8.17
    video_cnt = file_mp_info.prog_num;
    ptxt = &audio_track_set;
    if(flag)
    {
        if(1 < video_cnt )  //more than one video track,can switch prog.By Wu Jianwen,2010.8.17
        {
            cur_prog++;
            cur_prog %= video_cnt;
            snprintf((char *)str, 50, "VideoProg %lu/%lu",cur_prog+1, video_cnt); //display ID = actual ID + 1
            osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
            mpg_cmd_change_prog(cur_prog);
            mpeginfo.play_state = MPEG_PLAY;
            mpeginfo.play_speed = 0;
        }
        else if(1 == video_cnt)  //only one video track,just dispaly.By Wu Jianwen,2010.8.17
        {
            snprintf((char *)str, 50, "VideoProg %lu/%lu",cur_prog+1,video_cnt);
            osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str);
        }
        else    //no video track,by Wu Jianwen,2010.8.17
        {
            snprintf((char *)str, 45, "There is only one Video Program!");
            back_saved = win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,(char *)str,1500);
        }
    }
    else
    {
        if(1 <= video_cnt)
        {
            snprintf((char *)str, 50, "VideoProg %lu/%lu",cur_prog+1,video_cnt);
            osd_set_text_field_content(ptxt, STRING_ANSI, (UINT32)str);
        }
        else
        {
            snprintf((char *)str, 45, "There is only one Video Program!");
            back_saved = win_mp_common_compopup(WIN_POPUP_TYPE_SMSG,mp_compopup_rect,(char *)str,1500);
        }
    }
}
#endif
#endif //#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
#endif//#ifdef USB_MP_SUPPORT

