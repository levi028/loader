/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     win_mp_subtitle.c
*
*    Description: Mediaplayer mpeg video player subtitle support.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#ifdef USB_MP_SUPPORT

#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
#ifdef MP_SUBTITLE_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <hld/pan/pan_dev.h>
#include <hld/decv/vdec_driver.h>
#include <mediatypes.h>
#include <api/libnet/libnet.h>
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
//#ifdef MP_SUBTITLE_SUPPORT//vic100507#1
#include <api/libmp/mp_subtitle_api.h>
#include "mp_subtitle_osd.h"
#include <hld/snd/snd_dev.h>
//#endif
#include "win_plugin_subt.h"
#include "win_mpeg_player_audio_track.h"
#include "win_mpeg_player.h"
#include "win_mp_subtitle.h"
#include "media_control.h"
#include "win_subtitle.h"

#ifdef MP_SUBTITLE_SUPPORT    //vic100507#1
static BOOL subtitle_show = TRUE;
static UINT32 s_i_cur_subtitle = 0;  //current Embedded subtitle
static int i_cur_ext_subtitle = 0;    //the current external subtitle of the video    //joey.che

static void switch_load_subtitle(int ext_sub_id);//joey.che
#endif
//vic100519#1 end
static BOOL b_is_subtitle = TRUE;//xuehui
static fileinfo_video file_mp_info ;

#ifdef MP_SUBTITLE_SUPPORT    //vic100507#1
void win_mpegplayer_subt_set(void)
{
    if(osd_obj_open((POBJECT_HEAD)&g_cnt_subtitle, 0) != PROC_LEAVE)
    {
        menu_stack_push((POBJECT_HEAD) &g_cnt_subtitle);
    }
}
#endif


/*******************************************************************************
*    Ancillary functions
*******************************************************************************/

#ifdef MP_SUBTITLE_SUPPORT    //vic100507#1

//EXT_SUB_PLUGIN *g_ext_subtitle_plugin = NULL;//mask by joey.che

//extern UINT32 _mp_ext_subtitle_start;
//extern UINT32 _mp_ext_subtitle_end;

//please maintain and modify this iso8859 refercode table as order in your UI..
static UINT8  ascii_refcode[] =
{
    15,//English        0
    15,//French            1
    15,//German          2
    16,//Italian           3
    15,//Spanish        4
    15,//Portuguese    5
    5, //Russian        6
    9, //Turkish        7
    2, //Polish        8
    6, //Arabic        9
    6,//persian        10
};

void install_ext_subtitle_plugin(void)
{
    UINT32 ext_substruct_start = (UINT32)&_mp_ext_subtitle_start;
    UINT32 ext_substruct_end = (UINT32)&_mp_ext_subtitle_end;

    if(ext_substruct_start == ext_substruct_end)
    {
        g_ext_subtitle_plugin = NULL;
    }
    else
    {
        g_ext_subtitle_plugin = (EXT_SUB_PLUGIN *)ext_substruct_start;
    }
}

void auto_load_subtitle(void)
{
    media_file_info file_node;
    //char full_path[MAX_FILE_NAME_SIZE*2 + 1];
    mp_subtitle_init_parameter config;
    RET_CODE ret = 0;
    char *name= NULL;
    UINT16 len=0;
    SYSTEM_DATA *p_sys_data = NULL;
    UINT8 mp_lang = 0;

    name = (char *)malloc(FULL_PATH_SIZE);
    if(NULL == name)
    {
        return;
    }

    if (subtitle_handle)
    {
        if(g_ext_subtitle_plugin)
        {
            g_ext_subtitle_plugin->mps_delete(subtitle_handle);
        }
        subtitle_handle = NULL;
    }

    MEMSET(&file_node, 0, sizeof(media_file_info));
    MEMSET(&name[len], 0, FULL_PATH_SIZE - len);
    ret = get_file_from_play_list(mp_get_cur_playlist_video(), mp_get_cur_video_idx(), &file_node);
    if(ret == RET_FAILURE)
    {
        free(name);
        return ;
    }
    win_get_fullname(name, FULL_PATH_SIZE, file_node.path, file_node.name);

    MEMSET(&config, 0, sizeof(mp_subtitle_init_parameter));
    config.get_time = (mp_subtitle_get_time_function)mpgfile_decoder_get_play_time;
    config.osd_create = (mp_subtitle_osd_create_function)mp_subtitle_osd_create;
    config.osd_destroy = (mp_subtitle_osd_destroy_function)mp_subtitle_osd_destroy;
    config.osd_control = (mp_subtitle_osd_control_function)mp_subtitle_osd_control;

    if(g_ext_subtitle_plugin)
    {
    	if(g_ext_subtitle_plugin->mps_find_ext_subtitle)
    	{
        	subtitle_handle = (void*)g_ext_subtitle_plugin->mps_create(&config);
    	}
		else
		{
			subtitle_handle = NULL;
		}
    }
    if (subtitle_handle)
    {
		//MEMSET(full_path, 0, MAX_FILE_NAME_SIZE*2 + 1);
		//STRCPY(full_path, file_node.path);
		//strcat(full_path, "/");
		//strcat(full_path, file_node.name);
		if(g_ext_subtitle_plugin)
		{
			//g_ext_subtitle_plugin->mps_auto_load(subtitle_handle, name);
			if (g_ext_subtitle_plugin->mps_auto_load(subtitle_handle, name) <= 0)
			{
                         // no external subtitle
                         //mask by joey.che
                         //g_ext_subtitle_plugin->mps_delete(subtitle_handle);
                         //subtitle_handle = NULL;
			}
		}
		
	{
        p_sys_data = sys_data_get();
        mp_lang = p_sys_data->lang.osd_lang;
                                
        if(g_ext_subtitle_plugin)                        
        g_ext_subtitle_plugin->mps_io_ctrl(subtitle_handle,SET_SUBTITLE_STRING_TO_UNICODE, ascii_refcode[mp_lang],
                                            (long)iso8859_to_unicode);                    
    }    
    if(subtitle_handle&&(!subtitle_show))
    {
        if(g_ext_subtitle_plugin)
        {
            g_ext_subtitle_plugin->mps_hide_sub(subtitle_handle,TRUE);
        }
    }
}

    if(name)
    {
        free(name);
    }
}

static void switch_load_subtitle(int ext_sub_id)//joey.che
{
    media_file_info file_node;
    //char full_path[MAX_FILE_NAME_SIZE*2 + 1];
    mp_subtitle_init_parameter config;
    RET_CODE ret = 0;
    char *name = NULL;
    UINT8 mp_lang = 0;
    SYSTEM_DATA *p_sys_data = NULL;

    name = (char *)malloc(FULL_PATH_SIZE);
    if(NULL == name)
    {
        return;
    }

    if (subtitle_handle)
    {
        if(g_ext_subtitle_plugin)
        {
            g_ext_subtitle_plugin->mps_delete(subtitle_handle);
        }
        subtitle_handle = NULL;
    }

    MEMSET(&file_node, 0x0, sizeof(media_file_info));
    ret = get_file_from_play_list(mp_get_cur_playlist_video(), mp_get_cur_video_idx(), &file_node);
    if(ret == RET_FAILURE)
    {
        free(name);
        return ;
    }
    win_get_fullname(name, FULL_PATH_SIZE, file_node.path, file_node.name);

    MEMSET(&config, 0, sizeof(mp_subtitle_init_parameter));
    config.get_time = (mp_subtitle_get_time_function)mpgfile_decoder_get_play_time;
    config.osd_create = (mp_subtitle_osd_create_function)mp_subtitle_osd_create;
    config.osd_destroy = (mp_subtitle_osd_destroy_function)mp_subtitle_osd_destroy;
    config.osd_control = (mp_subtitle_osd_control_function)mp_subtitle_osd_control;

    if(g_ext_subtitle_plugin)
    {
    	if(g_ext_subtitle_plugin->mps_find_ext_subtitle)
    	{
        	subtitle_handle = (void*)g_ext_subtitle_plugin->mps_create(&config);
    	}
		else
		{
			subtitle_handle = NULL;
		}
    }
	
	if (subtitle_handle)
	{
		//MEMSET(full_path, 0, MAX_FILE_NAME_SIZE*2 + 1);
		//STRCPY(full_path, file_node.path);
		//strcat(full_path, "/");
		//strcat(full_path, file_node.name);
		if(g_ext_subtitle_plugin)
		{
			//g_ext_subtitle_plugin->mps_auto_load(subtitle_handle, name);
			if (g_ext_subtitle_plugin->mps_change_ext_sub_id(subtitle_handle, name, ext_sub_id) <= 0)
			{
                         // no external subtitle
                         //g_ext_subtitle_plugin->mps_delete(subtitle_handle);
                         //subtitle_handle = NULL;
			}
		}

        {
            p_sys_data = sys_data_get();
    		mp_lang = p_sys_data->lang.osd_lang;						
            
            if(g_ext_subtitle_plugin)
            {
                g_ext_subtitle_plugin->mps_io_ctrl(subtitle_handle,SET_SUBTITLE_STRING_TO_UNICODE,
                                 ascii_refcode[mp_lang], (long)iso8859_to_unicode);
            }
        }
    }
    
    if(subtitle_handle&&(!subtitle_show))
    {
        if(g_ext_subtitle_plugin)
        {
            g_ext_subtitle_plugin->mps_hide_sub(subtitle_handle,TRUE);
        }
    }
    if(name)
    {
        free(name);
    }
}

void unload_subtitle(void)
{
    if(g_ext_subtitle_plugin)
    {
        g_ext_subtitle_plugin->mps_unload(subtitle_handle);
        g_ext_subtitle_plugin->mps_delete(subtitle_handle);
    }
    subtitle_handle = NULL;
}
#endif


void mps_hide_subtitle(BOOL hide)
{
    if(g_ext_subtitle_plugin)
    {
        g_ext_subtitle_plugin->mps_hide_sub(subtitle_handle,hide);
    }
}

int mps_get_subt_file_num(void)
{
    if(g_ext_subtitle_plugin)
    {
        return g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle);
    }
    return 0;
}

void *get_subtitle_handle(void)
{
    return subtitle_handle;
}

BOOL get_mp_subtitle_show_status(void)
{
    return subtitle_show;
}

void mp_set_is_subtitle_flag( BOOL b_flag)
{
    b_is_subtitle = b_flag;
}

/*****************************************************
Interface : void Subtitle_Display_set(BOOL flag,BOOL ExtSubtitle);
Function  : switch subtitle
Author    : Vic Wang
Date      : 2012.12.01
Input     :
        flag:switch subtitle or not
        ExtSubtitle: whether ExtSubtitle or not
            TRUE: ExtSubtitle
            FALSE:Embedded Subtitle
Output    : NONE
Return    : NONE
******************************************************/
void subtitle_display_set(BOOL flag,BOOL extsubtitle)
{
#ifdef MP_SUBTITLE_SUPPORT
    UINT32 total_subtitle = 0;    //the total Embedded subtitle of the video
#ifdef AUDIOTRACK_SUPPORT
    PTEXT_FIELD p_text_field = NULL;
#endif
    UINT8 arr_data_temp[50] = {0};

    if(mp_get_mpeg_prompt_on_screen() != MPEG_PROMPT_STATUS_NULL)//fix bug:When switch track£¬clear the pop up message
    {                                       //because of not support last track decode. by Wu Jianwen,2010.8.18
        mpeg_file_prompt(NULL, 0);
    }

    MEMSET(&file_mp_info,0,sizeof(fileinfo_video));
    mpgget_total_play_time();
    mpg_file_get_stream_info(&file_mp_info);   //get the stream video info
    total_subtitle = file_mp_info.sub_stream_num;
#ifdef AUDIOTRACK_SUPPORT
    p_text_field = &audio_track_set;
#endif
    s_i_cur_subtitle = file_mp_info.cur_sub_stream_id;

    if(g_ext_subtitle_plugin && !b_is_subtitle)//joey.che
    {
        i_cur_ext_subtitle = g_ext_subtitle_plugin->mps_get_cur_sub_id(subtitle_handle);
    }

    if(flag)        //switch subtitle
    {
        if(extsubtitle)
        {
            if(g_ext_subtitle_plugin&&(g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle)))//joey.che
            {
                i_cur_ext_subtitle++;
                if(i_cur_ext_subtitle>=g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle))
                {
                    i_cur_ext_subtitle=0;
                }
                snprintf((char *)arr_data_temp, 50, "Subtitle %d/%d",
                        i_cur_ext_subtitle+1, 
                        g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle));
            #ifdef AUDIOTRACK_SUPPORT
                //mp_set_audio_track_content(STRING_ANSI, (UINT32)arr_DataTemp);
                osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
            #endif
                if(g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle)>1)
                {
                       g_ext_subtitle_plugin->mps_delete(subtitle_handle);
                             subtitle_handle = NULL;
                    switch_load_subtitle(i_cur_ext_subtitle);
                }
            }
        }
        else
        {
            if(total_subtitle >= 1) //only subtitle num >= 1 can switch
            {
                #ifdef MP_SUBTITLE_SUPPORT
                if(g_ext_subtitle_plugin)
                {
                    g_ext_subtitle_plugin->mps_hide_sub(subtitle_handle,TRUE);
                }
                #endif
                s_i_cur_subtitle++;
                if(s_i_cur_subtitle > total_subtitle)
                {
                    s_i_cur_subtitle = 0;
                    snprintf((char *)arr_data_temp, 50, "Subtitle Off");
                }
                else
                {
                    snprintf((char *)arr_data_temp, 50, "Subtitle %lu/%lu",s_i_cur_subtitle,total_subtitle);
                }
            #ifdef AUDIOTRACK_SUPPORT
                //mp_set_audio_track_content(STRING_ANSI, (UINT32)arr_DataTemp);
                osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
            #endif
                mpg_cmd_change_subtitle(s_i_cur_subtitle);
            }
        }
    }
    else
    {
        if(extsubtitle)
        {
            if(g_ext_subtitle_plugin&&(g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle)))//joey.che
            {
                snprintf((char *)arr_data_temp, 50, "Subtitle %d/%d",
                        g_ext_subtitle_plugin->mps_get_cur_sub_id(subtitle_handle)+1,
                        g_ext_subtitle_plugin->mps_get_sub_file_num(subtitle_handle));
            #ifdef AUDIOTRACK_SUPPORT
                //mp_set_audio_track_content(STRING_ANSI, (UINT32)arr_DataTemp);
                osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
            #endif
            }
        }
        else
        {
            if(total_subtitle >= 1)
            {
                if(0 == s_i_cur_subtitle)
                {
                    snprintf((char *)arr_data_temp, 50, "Subtitle Off");
                }
                else
                {
                    snprintf((char *)arr_data_temp, 50, "Subtitle %lu/%lu",s_i_cur_subtitle, total_subtitle);
                }
            #ifdef AUDIOTRACK_SUPPORT
                //mp_set_audio_track_content(STRING_ANSI, (UINT32)arr_DataTemp);
                osd_set_text_field_content(p_text_field, STRING_ANSI, (UINT32)arr_data_temp);
            #endif
            }
            /*
            else
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);

                win_compopup_set_msg_ext("No Embedded Subtitle!",NULL,0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1500);
                win_compopup_smsg_restoreback();
            }
            */
        }
    }
#endif
}
#ifndef DUAL_ENABLE
SUBT_INPUT_PLUGIN *get_subt_decore(char *name)
{
    //extern UINT32 _embed_subtitle_start;
    //extern UINT32 _embed_subtitle_end;
    UINT32 subt_init_struct_start = (UINT32)&_embed_subtitle_start;
    UINT32 subt_init_struct_end = (UINT32)&_embed_subtitle_end;

    SUBT_INPUT_PLUGIN *tmp = (SUBT_INPUT_PLUGIN *)(subt_init_struct_start);
    while( ((UINT32)tmp+sizeof(SUBT_INPUT_PLUGIN)) <= subt_init_struct_end )
    {
        if(name != NULL && !strcmp(tmp->name, name))
        {
            return tmp;
        }
        tmp++;
    }
    return NULL;
}
#endif



#endif //MP_SUBTITLE_SUPPORT
#endif //MEDIA_PLAYER_VIDEO_SUPPORT
#endif //USB_MP_SUPPORT

