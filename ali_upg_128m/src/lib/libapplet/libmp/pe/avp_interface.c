  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: avp_interface.c
*
*    Description: This file describes avp interface operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/string.h>
#ifdef MP4_CONTAINER
#include <hld/snd/snd_dev.h>
#include <api/libmp/pe.h>
#include "av_player/av_player.h"

#define AVP_FIRST_CACHE_READ_SIZE       0xd0000
#define AVPFILE_CACHE_BLK_SIZE      0x20000
#define MP4_CACHE_SIZE            8*1024*1024//0x120000
#define MP4_CACHE_BLK_SIZE      0x20000

P_HANDLE g_mp4_av_player_handle = 0;
//from dec_ctrl.c
enum av_player_direction g_mp4_player_direction;
enum av_player_speed g_mp4_player_speed;
//#define AVP_PE_CACHE_TEST
//From video engine.c
static UINT32 m_mp4_cache_buffer;
static BOOL m_avp_first_cache_read = TRUE;
F_HANDLE m_mp4_test_file = 0;
int avp_cache_id = 0;

static DWORD avp_decoder_read_data(BYTE *buf, DWORD size)
{
    if(!buf)
    {
        libc_printf("%s: buf is null \n", __FUNCTION__);
        return 0;
    }
    if(m_avp_first_cache_read == TRUE)
    {
        m_avp_first_cache_read = FALSE;
        return pe_cache_read(avp_cache_id, buf, size, AVP_FIRST_CACHE_READ_SIZE);
    }
    else
    {
        return pe_cache_read(avp_cache_id, buf, size, size);
    }
    //mpg_cache_id need to  change
}

static BOOL avp_decoder_seek_data(long long offset, DWORD origin)
{
    //mpg_cache_id need to  change
    if(!pe_cache_seek(avp_cache_id, offset, origin))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int avp_mp4_fread(F_HANDLE file, void *buf, int size)
{
    if(!buf)
    {
        libc_printf("%s: buf is null \n", __FUNCTION__);
        return 0;
    }
    return avp_decoder_read_data(buf, size);
}

static long long avp_mp4_fseek(F_HANDLE file,long long offset, int flag)
{
    if(avp_decoder_seek_data(offset, flag))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static long long avp_mp4_ftell(F_HANDLE file)
{
    return 0;
}

void t2avp_dec_task(UINT32 para1, UINT32 para2)
{
/*
    DEC_INIT_PAR m_DecInitPar;
    DEC_STREAM_INFO DecStreamInfo;
    struct vdec_io_reg_callback_para tpara;
    UINT8 video_stream_type = (para2 >>16)&0xff;
    enum SndDupChannel audio_channel = (para2 >>8)&0xff;
    UINT8 preview = para2&0xff;
*/
    g_video_dec_running = TRUE;
    m_avp_first_cache_read = TRUE;
    pestream_start();
    av_player_file file;

    MEMSET(dest, c, len)((void *)&file, 0, sizeof(av_player_file));
    file.f_p = m_mp4_test_file;
    file.fread = avp_mp4_fread;
    file.fseek = avp_mp4_fseek;
    file.ftell = avp_mp4_ftell;
    av_player_play(P_HANDLE player, pav_player_file pfile, mp_callback_func cb)(g_mp4_av_player_handle, &file, para1);
}

//from dec_ctrl.c
DWORD avp_cmd_play_proc(void)
{
    g_mp4_player_direction = AV_PLAYER_FORWARD;
    g_mp4_player_speed = AV_PLAYER_SPEEDX1;
    av_player_mode(g_mp4_av_player_handle, g_mp4_player_direction, g_mp4_player_speed);
    av_player_resume(g_mp4_av_player_handle);
    return 1;
}
DWORD avp_cmd_pause_proc(void)
{
    av_player_pause(g_mp4_av_player_handle);
    g_mp4_player_direction = AV_PLAYER_FORWARD;
    g_mp4_player_speed = AV_PLAYER_SPEED_STEP;
    return 1;
}
DWORD avp_cmd_resume_proc(void)
{
    g_mp4_player_direction = AV_PLAYER_FORWARD;
    g_mp4_player_speed = AV_PLAYER_SPEED_STEP;
    av_player_mode(g_mp4_av_player_handle, g_mp4_player_direction, g_mp4_player_speed);
    av_player_resume(g_mp4_av_player_handle);
    return 1;
}

DWORD avp_cmd_ff_proc(void)
{
    if(AV_PLAYER_FORWARD == g_mp4_player_direction)
    {
        g_mp4_player_speed++;
        if(g_mp4_player_speed > AV_PLAYER_SPEEDX16)
        {
            g_mp4_player_speed = AV_PLAYER_SPEEDX1;
        }
    }
    else
    {
        g_mp4_player_speed = AV_PLAYER_SPEEDX1;
    }

    g_mp4_player_direction = AV_PLAYER_FORWARD;
    av_player_mode(g_mp4_av_player_handle, g_mp4_player_direction, g_mp4_player_speed);
    return 1;
}

DWORD avp_cmd_fb_proc(void)
{
    if(AV_PLAYER_BACKWARD == g_mp4_player_direction)
    {
        g_mp4_player_speed++;
        if(g_mp4_player_speed > AV_PLAYER_SPEEDX16)
        {
            g_mp4_player_speed = AV_PLAYER_SPEEDX1;
        }
    }
    else
    {
        g_mp4_player_speed = AV_PLAYER_SPEEDX1;
    }

    g_mp4_player_direction = AV_PLAYER_BACKWARD;

    av_player_mode(g_mp4_av_player_handle, g_mp4_player_direction, g_mp4_player_speed);
    return 1;
}

DWORD avp_cmd_slow_proc(void)
{
    if(AV_PLAYER_FORWARD == g_mp4_player_direction)
    {
        g_mp4_player_speed++;
        if(g_mp4_player_speed > AV_PLAYER_SPEED_1_16)
        {
            g_mp4_player_speed = AV_PLAYER_SPEED_1_2;
        }
        else if(g_mp4_player_speed < AV_PLAYER_SPEED_1_2)
        {
            g_mp4_player_speed = AV_PLAYER_SPEED_1_2;
        }
    }
    else
    {
        g_mp4_player_speed = AV_PLAYER_SPEED_1_2;
    }

    g_mp4_player_direction = AV_PLAYER_FORWARD;

    av_player_mode(g_mp4_av_player_handle, g_mp4_player_direction, g_mp4_player_speed);

    return 1;
}


DWORD avp_cmd_stop_proc(void)
{
    //if(0 == m_mp4_test_file)
        //return 1;
    av_player_stop(g_mp4_av_player_handle);
    vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),FALSE);
    g_video_dec_running = FALSE;
    //fclose(m_mp4_test_file);
    //m_mp4_test_file = 0;
    if(avp_cache_id!=-1)
    {
        pe_cache_close(avp_cache_id);
    }
    return 1;
}

DWORD avp_cmd_terminate_proc(void)
{
    //if(0 == m_mp4_test_file)
        //return 1;
    av_player_stop(g_mp4_av_player_handle);
    //vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),FALSE);
    g_video_dec_running = FALSE;
    //fclose(m_mp4_test_file);
    //m_mp4_test_file = 0;
    if(avp_cache_id!=-1)
    {
        pe_cache_close(avp_cache_id);
    }
    return 1;
}

DWORD avp_cmd_search_proc(DWORD search_time)
{
    av_player_seek(g_mp4_av_player_handle, search_time);
    return 1;
}
//from plugin_mpeg.c
DWORD avpfile_decoder_get_play_time(void)
{
    UINT32 time = 0;

    av_player_control(g_mp4_av_player_handle, AV_PLAYER_CMD_CURRENT_TIME, &time);

    return time;
}

DWORD avpget_total_play_time(void)
{
    UINT32 time = 0;

    av_player_control(g_mp4_av_player_handle, AV_PLAYER_CMD_TOTAL_DURATION, &time);
    return time;

}
//move from video engine, mp4's architecture don't use video engine
void avp_cache_init(UINT32 cache_buf)
{
    m_mp4_cache_buffer = cache_buf;
}

int mp4_net_dec_file(char *path, BOOL preview)
{
    UINT8 * cache_buf = NULL;//(UINT8 *)(m_cache_buffer);
    UINT32 para1 = NULL;//MP4 don't support callback function
    UINT32 para2 = ((MPEG_12_FILE_STREAM<<16)|(SND_DUP_NONE<<8)|preview);

    if(!path)
    {
        libc_printf("%s: path is null\n", __FUNCTION__);
        return -1;
    }
    avp_cache_id =pe_cache_open(path, (UINT8 *)m_mp4_cache_buffer, MP4_CACHE_SIZE, MP4_CACHE_BLK_SIZE);
    if(-1 == avp_cache_id)
    {
        libc_printf("mp4_net_dec_file create pe cache failed\n");
       return -1;
    }
    t2avp_dec_task(para1, para2);
}

#endif



