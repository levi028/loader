#ifdef WIN32

#include "plugin.h"


#define MEMCPY memcpy
#else
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <api/libtsi/db_3l.h>

#include <api/libmp/pe.h>
#include "mp_player.h"
#include "plugin.h"
#define AF_PE_PRINTF     libc_printf

#endif
#define PLAY_SPEED_MAX 6


#ifdef WIN32
static dec_cmd_sema = 0;
#define ENTER_DEC_CMD() {while(dec_cmd_sema){decoder_sleep(5);}; dec_cmd_sema = 1;}
#define LEAVE_DEC_CMD() {dec_cmd_sema = 0;}

#else
ID dec_cmd_sema_id = OSAL_INVALID_ID;

#define ENTER_DEC_CMD()    osal_semaphore_capture(dec_cmd_sema_id,TMO_FEVR)

#define LEAVE_DEC_CMD()    osal_semaphore_release(dec_cmd_sema_id)
#endif

#define MAX_PROG_TIME_NUM 64     //Fix bug:mp ts change prog subtitle can not show.

static t2decoder_control m_decoder_ctrl;
static AF_PE_PLAY_STATE m_cur_playstate;
static AF_PE_PLAY_STATE m_nxt_playstate;
static int m_cur_play_speed = 0;
static DWORD m_search_time = 0;
static DWORD prog_time[MAX_PROG_TIME_NUM];

UINT32 g_seek_time = INVALID_TIME;
paf_pe_function_table p_dec_func_table; //decoder control services functions provided by decoder control library
extern INT32 avplayer_subtt_channel_change(INT32 idx);


extern af_pe_function_table dct_avplayer;
DWORD pestream_start(UINT8 video_stream_type)
{
#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif
    int i = 0;

    for(i=0; i<20; i++)
    {
        prog_time[i] = INVALID_TIME;
    }

    m_cur_playstate = peplay;
    m_nxt_playstate = peplay;
    m_cur_play_speed = 1;
    m_search_time = INVALID_TIME;
    m_decoder_ctrl.play_dir = FORWARD_PLAY;
    m_decoder_ctrl.scr_ratio = 0;
    m_decoder_ctrl.renew = FALSE;
    m_decoder_ctrl.subt_onoff = TRUE;
    m_decoder_ctrl.subp_stream_id = 0 ;
    m_decoder_ctrl.audio_stream_id = -1; // container should auto-select a correct audio track
    m_decoder_ctrl.video_playmode =  VIDEO_PLAY_MODE_STOP;

    m_decoder_ctrl.subt_lang_id = -1;
    m_decoder_ctrl.prog_id = -1;
    p_dec_func_table = &dct_avplayer;//ylm->c; video_get_ctrl_table(video_stream_type);

    return 0;
}

static DWORD penormal_ply(void)
{
#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_cur_play_speed = 1;
    m_search_time = INVALID_TIME;
    m_nxt_playstate = peplay;

    //avplayer_subtt_channel_change();
        
    return AF_ERR_PE_OK;
}

static DWORD pefast_forward_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif
    m_search_time = INVALID_TIME;

    if(peplay == m_cur_playstate ||
       peffx2 == m_cur_playstate ||
       peffx4 == m_cur_playstate ||
       peffx8x16x32 == m_cur_playstate)
    {

        m_cur_play_speed++;
        if(m_cur_play_speed > PLAY_SPEED_MAX)
    {
            m_cur_play_speed = 2;
    }
    }
    else
    {
        m_cur_play_speed = 2;
    }

    switch(m_cur_play_speed)
    {
        case 1:
            return penormal_ply();
        case 2:
            m_nxt_playstate = peffx2;
            break;
        case 3:
            m_nxt_playstate = peffx4;
            break;
        case 4:
        case 5:
        case 6:
        default:
            m_nxt_playstate = peffx8x16x32;
            break;
    }
    return AF_ERR_PE_OK;
}

static DWORD pefast_reverse_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;
    m_decoder_ctrl.play_dir = BACKWARD_PLAY;

    if(perev == m_cur_playstate)
    {
        m_cur_play_speed++;
        if(m_cur_play_speed > PLAY_SPEED_MAX)
    {
            m_cur_play_speed = 1;
    }
    }
    else
    {
        m_cur_play_speed = 1;
    }

    /*
    In 2 speed FR play mode, FW should set video play mode to
    VIDEO_PLAY_MODE_SKIP_B_P and ScrRatio to 2, because
    we don't support 2 speed smooth FR playback.
    */
    if(1 == m_cur_play_speed)
        m_cur_play_speed = 2;

    m_nxt_playstate = perev;

    return AF_ERR_PE_OK;
}

static DWORD peslow_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;

    if(peslow == m_cur_playstate)
    {
        m_cur_play_speed--;
        if(m_cur_play_speed < -3)
    {
            m_cur_play_speed = -1;
    }
    }
    else
    {
        m_cur_play_speed = -1;
    }


    m_nxt_playstate = peslow;

    return AF_ERR_PE_OK;
}

static DWORD pepause_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;

    m_cur_play_speed = 0xff;

    /* pause subtitle */
    avplayer_subtt_channel_change(0);
    
    if(m_cur_playstate != pepause_step)
    {
        m_nxt_playstate = pepause_step;
    }
    return AF_ERR_PE_OK;
}

static DWORD pestep_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;
    m_cur_play_speed = 0xfe;

    m_nxt_playstate = pepause_step;
    return AF_ERR_PE_OK;
}

static DWORD pestop_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;
    m_cur_play_speed = 1;

    m_nxt_playstate = pestop;
    return AF_ERR_PE_OK;
}

static DWORD peresume_stop_ply(void)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    m_search_time = INVALID_TIME;
    m_cur_play_speed = 1;

    m_nxt_playstate = peresume_stop;
    return AF_ERR_PE_OK;
}

static DWORD peseek_ply(DWORD search_time)
{

#ifndef WIN32
    AF_PE_PRINTF("%s: %d\n", __FUNCTION__, search_time);
#endif
    
    m_cur_play_speed = 1;
    m_search_time = search_time;
    m_nxt_playstate = peplay;
    m_cur_playstate = peplay;
    
    return AF_ERR_PE_OK;
}

static DWORD pechange_pro_id(int prog_id, DWORD search_time)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif
    DEC_STREAM_INFO stream_info;

    MEMSET(&stream_info,0,sizeof(DEC_STREAM_INFO));
    mpg_file_get_stream_info(&stream_info);

    if(stream_info.cur_prog_id != prog_id)
    {
        m_decoder_ctrl.prog_id = prog_id;
        m_decoder_ctrl.renew = TRUE;
        m_decoder_ctrl.search_time = INVALID_TIME;//search_time;
        m_decoder_ctrl.audio_stream_id = 0;
        m_decoder_ctrl.video_playmode = VIDEO_PLAY_MODE_NORMAL;
        m_decoder_ctrl.audio_playmode = AUDIO_PLAY_MODE_ON;
        m_decoder_ctrl.play_dir = FORWARD_PLAY;
        m_decoder_ctrl.scr_ratio = 1;
        m_cur_play_speed = 1;
        m_search_time = INVALID_TIME;
        m_nxt_playstate = peplay;
        return AF_ERR_PE_OK;
    }
    else
    {
        return AF_ERR_PE_FAIL;
    }
}

/*
static DWORD PEChangeSubtID(int subt_lang_id)
{
   
#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    if(m_DecoderCtrl.subt_lang_id != subt_lang_id)
    {
        m_DecoderCtrl.subt_lang_id = subt_lang_id;
        m_DecoderCtrl.renew = TRUE;
        //m_DecoderCtrl.AudioPlaymode = AUDIO_PLAY_MODE_NONE;
        m_DecoderCtrl.SearchTime = INVALID_TIME;
        return AF_ERR_PE_OK;
    }
    else
    {
        return AF_ERR_PE_FAIL;
    }
}
*/

static DWORD pechange_aid(int audio_stream_id)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    if(m_decoder_ctrl.audio_stream_id != audio_stream_id)
    {
        m_decoder_ctrl.audio_stream_id = audio_stream_id;
        m_decoder_ctrl.renew = TRUE;
        //m_DecoderCtrl.AudioPlaymode = AUDIO_PLAY_MODE_NONE;
        m_decoder_ctrl.search_time = INVALID_TIME;
        return AF_ERR_PE_OK;
    }
    else
    {
        return AF_ERR_PE_FAIL;
    }
}

static DWORD pechange_sid(int sub_stream_id)
{

#ifndef WIN32
    AF_PE_PRINTF("%s\n", __FUNCTION__);
#endif

    if(m_decoder_ctrl.subp_stream_id != sub_stream_id)
    {
        m_decoder_ctrl.subp_stream_id = sub_stream_id;
        m_decoder_ctrl.renew = TRUE;
        m_decoder_ctrl.search_time = INVALID_TIME;
        return AF_ERR_PE_OK;
    }
    else
    {
        return AF_ERR_PE_FAIL;
    }
}

#if 0
static DWORD pechange_vid(AF_PE_MESSAGE *p_message)
{
    DWORD ret;

    AF_PE_PRINTF("%s\n", __FUNCTION__);

    if(m_decoder_ctrl.video_stream_id != p_message->video_stream_id)
    {
        m_decoder_ctrl.video_stream_id = p_message->video_stream_id;
        ret = update_stream_id();
    }
    else
        ret = AF_ERR_PE_OK;

    return ret;
}

static DWORD pechange_sid(AF_PE_MESSAGE *p_message)
{
    DWORD ret;

    AF_PE_PRINTF("%s: old id = 0x%x, new id = 0x%x\n", \
        __FUNCTION__, m_decoder_ctrl.subp_stream_id, p_message->subp_stream_id);

    if(m_decoder_ctrl.subp_stream_id != p_message->subp_stream_id)
    {
        m_decoder_ctrl.subp_stream_id = p_message->subp_stream_id;
        ret = update_stream_id();
    }
    else
        ret = AF_ERR_PE_OK;

    return ret;
}

static DWORD perst_play_point(AF_PE_MESSAGE *p_message)
{
    DWORD ret = AF_ERR_PE_OK;

    if(pe_dec_set_play_point0(&p_message->start_point, &p_message->end_point, TRUE) == FALSE)
    {
        AF_PE_FWASSERT(0);
        ret = AF_ERR_NOT_AVAILABLE;
    }

    return ret;
}
#endif

/*
static AF_PE_PLAY_STATE get_cur_play_state(void)
{
    return m_CurPlaystate;
}
*/

/*
static int get_cur_play_speed(void)
{
    return m_CurPlaySpeed;
}
*/

AF_PE_PLAY_STATE get_next_play_state(void)
{
    return m_nxt_playstate;
}

#ifndef WIN32
DWORD mpg_cmd_play_proc(void)
{
    DWORD ret =0;

   AF_PE_PRINTF("Into %s\n", __func__);
   
    if(m_decoder_ctrl.renew == TRUE)
    {
        return AF_ERR_PE_FAIL;
    }
    ENTER_DEC_CMD();
    ret = penormal_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;
}

DWORD mpg_cmd_pause_proc(void)
{
    DWORD ret = 0;
    AF_PE_PRINTF("Into %s\n", __func__);

    ENTER_DEC_CMD();
    ret = pepause_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;
}

DWORD mpg_cmd_resume_proc(void)
{
    DWORD ret = 0;
   AF_PE_PRINTF("Into %s\n", __func__);

    ENTER_DEC_CMD();
    ret = pestep_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;
}

DWORD mpg_cmd_ff_proc(void)
{
    DWORD ret = 0;

    ENTER_DEC_CMD();
    ret = pefast_forward_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;

}

DWORD mpg_cmd_fb_proc(void)
{
    DWORD ret = 0;

    ENTER_DEC_CMD();
    ret = pefast_reverse_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;

}

DWORD mpg_cmd_slow_proc(void)
{
    DWORD ret = 0;

    ENTER_DEC_CMD();
    ret = peslow_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;

}

DWORD mpg_cmd_stop_proc(int stop_type)
{
    DWORD ret = 0;
    MP_MEDIA_TYPE playing_type;
    struct vpo_device *vpo_dev = NULL;
    AF_PE_PRINTF("Into %s\n", __func__);
    
	/*eric.cai */
    // video_engine_tell_pecache_closing();
    playing_type = get_current_media_type();
    switch (stop_type)  {
    case 0: // stop video playing
        if(playing_type != MP_MEDIA_VIDEO) {
            libc_printf("stop video player but video player not running %d\n", playing_type);                
            return -1;
        }
        break;
    case 1: // stop music playing
        if(playing_type != MP_MEDIA_MUSIC) {
            libc_printf("stop music player but music player not running %d\n", playing_type);
            return -1;
        }
        break;
        
    default:
        libc_printf("Warning: stop type %d was unknow\n", stop_type);
        break;
    }      
    
LBL_STOP:
    ENTER_DEC_CMD();
    ret = pestop_ply();
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    else
    {
        if (g_video_dec_running)
        {
            vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
            if (vpo_dev)
                vpo_win_onoff(vpo_dev, FALSE);
#ifdef DUAL_VIDEO_OUTPUT
            vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
            if (vpo_dev)
                vpo_win_onoff(vpo_dev, FALSE);    // avoid green screen
#endif
        }
    }
    LEAVE_DEC_CMD();

    while(g_video_dec_running)
    {
        osal_task_sleep(1);
        if (m_nxt_playstate != pestop)
            goto LBL_STOP;
    }
    AF_PE_PRINTF("Exit %s\n", __func__);

    return ret;

}

DWORD mpg_cmd_resume_stop_proc(void)
{
    DWORD ret = 0;
    DWORD cur_play_state = 0;
    DWORD next_play_state = 0;
    DWORD cur_play_speed = 0;
    DWORD search_time = 0;

    ENTER_DEC_CMD();
    cur_play_state = m_cur_playstate;
    search_time = INVALID_TIME;
    cur_play_speed = 1;
    next_play_state = peresume_stop;
    if(p_dec_func_table)
    {
        if((*p_dec_func_table)[cur_play_state][next_play_state](&m_decoder_ctrl, cur_play_speed, search_time))
        {
            ret = peresume_stop_ply();
        }
        else
        {
            m_decoder_ctrl.renew = FALSE;
        }
    }

    LEAVE_DEC_CMD();
    return ret;

}

DWORD mpg_cmd_search_proc(DWORD search_time)
{
    DWORD ret = 0;
    
    ENTER_DEC_CMD();
    ret = peseek_ply(search_time);
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
        g_seek_time = (g_seek_time == INVALID_TIME) ? search_time : g_seek_time;
    }
    LEAVE_DEC_CMD();
    return ret;
}


DWORD mpg_cmd_search_ms_proc(DWORD search_ms_time)
{
    DWORD ret = 0;

    ENTER_DEC_CMD();
    ret = peseek_ply(search_ms_time | 0x80000000);
    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }
    LEAVE_DEC_CMD();
    return ret;
}

//DWORD mpg_cmd_change_audio_track(INT32 *aud_pid);
DWORD mpg_cmd_change_prog (int prog_id)
{
    DWORD ret = AF_ERR_PE_FAIL;
    DEC_STREAM_INFO stream_info;

    ENTER_DEC_CMD();

    MEMSET(&stream_info,0,sizeof(DEC_STREAM_INFO));
    mpg_file_get_stream_info(&stream_info);

    if (stream_info.prog_num > 1)
    {       
        if ( stream_info.cur_prog_id > (MAX_PROG_TIME_NUM - 1) )     //Fix bug:mp ts change prog subtitle can not show.
        {
            ASSERT(0);
        }

        /* Modify by chen 2014-08-15,   for removing warning :  Avoid accessing arrays out of bounds (BD-PB-ARRAY-1) */
        if (stream_info.cur_prog_id < MAX_PROG_TIME_NUM)
        {
            prog_time[stream_info.cur_prog_id] = mpgfile_decoder_get_play_time();
            if (((UINT32)prog_id < stream_info.prog_num) && (prog_id < MAX_PROG_TIME_NUM))
            {
                 ret = pechange_pro_id(prog_id, prog_time[prog_id]);
            }
            else
            {
#ifndef WIN32
                AF_PE_PRINTF("\n%s, prog_id = %d , pechange_pro_id  fail!\n", __FUNCTION__, prog_id);
#endif                
            }
        }
        else
        {
#ifndef WIN32
            AF_PE_PRINTF("\n%s, stream_info.cur_prog_id = %d , out of bound error!\n", __FUNCTION__, stream_info.cur_prog_id);
#endif
        }
    }
    LEAVE_DEC_CMD();

//    mpg_cmd_change_audio_track(&ch0);
    return ret;
}

/*
static DWORD mpg_cmd_change_subt_lang (int subt_lang_id)
{
    DWORD ret = AF_ERR_PE_FAIL;
    DEC_STREAM_INFO stream_info;

    ENTER_DEC_CMD();
    MpgFileGetStreamInfo(&stream_info);

    if(stream_info.SubStreamNum > 1)
    {
        if(subt_lang_id < stream_info.SubStreamNum)
             ret = PEChangeSubtID(subt_lang_id);
    }
    LEAVE_DEC_CMD();

    return ret;
}
*/

DWORD mpg_cmd_change_audio_track(INT32 *aud_pid)
{
    DWORD ret = AF_ERR_PE_FAIL;
    DEC_STREAM_INFO stream_info;

    if (NULL == aud_pid)
    {
        return ret;
    }

    ENTER_DEC_CMD();

    MEMSET(&stream_info,0,sizeof(DEC_STREAM_INFO));
    mpg_file_get_stream_info(&stream_info);

    if(stream_info.audio_stream_num > 1)
    {
        if(*aud_pid > (INT32)stream_info.audio_stream_num)
        {
             *aud_pid = 1;
        }
        ret = pechange_aid(*aud_pid);
    }

    LEAVE_DEC_CMD();

    return ret;
}

DWORD mpg_cmd_change_subtitle(INT32 sub_pid)
{
    DWORD ret = AF_ERR_PE_FAIL;
    DEC_STREAM_INFO stream_info;

    ENTER_DEC_CMD();

    MEMSET(&stream_info,0,sizeof(DEC_STREAM_INFO));
    mpg_file_get_stream_info(&stream_info);
    if ((sub_pid >= 0) && (sub_pid <= (INT32)stream_info.sub_stream_num))
    {
        ret = pechange_sid(sub_pid);
    }
    else
    {
#ifndef WIN32
        AF_PE_PRINTF("\n%s, sub_pid = %d ,  out of bound error!\n", __FUNCTION__, sub_pid);
#endif
    }
    LEAVE_DEC_CMD();

    return ret;
}

#endif

/**********************TEST code for controlling mpeg/AVI trick mode***************/
#if 0//def WIN32
DWORD s_time = 5;
BOOL set_ctrl_cmd(DWORD msg_code, DWORD time)
{
    BOOL ret = TRUE;
    DEC_STREAM_INFO stream_info;
    ENTER_DEC_CMD();

    if(1)
    {
        switch(msg_code)
        {
            case V_KEY_PLAY:
                penormal_ply();
                break;
            case V_KEY_PAUSE:
                if(m_cur_playstate == pepause_step)
                {
                    pestep_ply();
                }
                else
                {
                    pepause_ply();
                    //Delay for a while to show "Pause" OSD on PC version
                    decoder_sleep(100);
                }
                break;
            case V_KEY_FF: //FF
                pefast_forward_ply();
                break;
            case V_KEY_FB: //FB
                pefast_reverse_ply();
                break;
            case V_KEY_SLOW: //Slow
                peslow_ply();
                break;
            case V_KEY_STOP:
                pestop_ply();
                break;
            case V_KEY_SEEK:
                peseek_ply(time);
                break;
            case V_KEY_CHGAUD:
                MEMSET(&stream_info,0,sizeof(DEC_STREAM_INFO));
                mpg_file_get_stream_info(&stream_info);
                if(stream_info.audio_stream_num)
                {
                    if(stream_info.cur_audio_stream_id < stream_info.audio_stream_num)
                        ret = pechange_aid(stream_info.cur_audio_stream_id+1);
                    else
                        ret = pechange_aid(1);
                }
                if(ret == AF_ERR_PE_OK)
                    peseek_ply(mpgfile_decoder_get_play_time());
                break;
        }
    }

    if(p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
        ret = TRUE;
    }
    else
        ret = FALSE;

    LEAVE_DEC_CMD();
    return ret;

}
#endif
/**********************end of TEST code for controlling mpeg trick mode********/

/*----------------------------------------------------
Name:
    GetCtrlCmd
Description:
    Get control command sent by Firmware, this function
    is called by decoder and decoder should call this function
    2 to 3 times at least in order to resond command in time.
Parameters:
    IN:

    OUT:
        *decoder_ctrl: decoder control structure pointer
Return:
    TRUE: there is new command coming, otherwise, FALSE
------------------------------------------------------*/
BOOL get_ctrl_cmd(t2decoder_control *decoder_ctrl)
{
    BOOL ret = 0;

    ENTER_DEC_CMD();
    if (m_decoder_ctrl.renew)
    {
        MEMCPY(decoder_ctrl,  &m_decoder_ctrl, sizeof(t2decoder_control));
/*
            AF_PE_PRINTF("video_playmode = %d, audio_playmode = %d, scr_ratio = %d, searchtime = 0x%x\n", \
                m_decoder_ctrl.video_playmode, m_decoder_ctrl.audio_playmode,    m_decoder_ctrl.scr_ratio, m_decoder_ctrl.search_time);
*/
        m_cur_playstate = m_nxt_playstate;
        m_decoder_ctrl.renew = FALSE;
        m_search_time = INVALID_TIME;
        ret = TRUE;
    }
    else
        ret = FALSE;
    LEAVE_DEC_CMD();
    return ret;
}

/* User can set the video play speed based on the input parameter : speed , such as  PE_SPEED_FASTFORWARD_2*/
DWORD mpg_cmd_set_speed(UINT32 speed)
{
    DWORD ret = 0;

    /* Check if the input parameter UINT32 speed is leagal */
    if ((PE_PLAY_SPEED_BEGIN > speed ) || (PE_PLAY_SPEED_END <= speed))
    {
        /* ileagal input parameter */
        ret = 1;
        return ret;
    }

    ENTER_DEC_CMD();

    if (PE_SPEED_NORMAL == speed)
    {
        /* normal play speed */
        penormal_ply();
    }
    else if ((PE_SPEED_FASTFORWARD_2 <= speed ) && (PE_SPEED_FASTFORWARD_24 >= speed))
    {
        /* fast forward */

        m_search_time = INVALID_TIME;

        switch (speed)
        {
            case PE_SPEED_FASTFORWARD_2:
                m_cur_play_speed = 2;
                m_nxt_playstate = peffx2;
                break;

            case PE_SPEED_FASTFORWARD_4:
                m_cur_play_speed = 3;
                m_nxt_playstate = peffx4;
                break;

            case PE_SPEED_FASTFORWARD_8:
                m_cur_play_speed = 4;
                m_nxt_playstate = peffx8x16x32;
                break;

            case PE_SPEED_FASTFORWARD_16:
                m_cur_play_speed = 5;
                m_nxt_playstate = peffx8x16x32;
                break;

            case PE_SPEED_FASTFORWARD_24:
                m_cur_play_speed = 6;
                m_nxt_playstate = peffx8x16x32;
                break;

            default:
                break;
        }
    }
    else if ((PE_SPEED_FASTREWIND_2 <= speed) && (PE_SPEED_FASTREWIND_24 >= speed))
    {
        /* fast rewind */
        m_search_time = INVALID_TIME;
        m_decoder_ctrl.play_dir = BACKWARD_PLAY;
        switch(speed)
        {
            case PE_SPEED_FASTREWIND_2:
                m_cur_play_speed = 2;
                break;

            case PE_SPEED_FASTREWIND_4:
                m_cur_play_speed = 3;
                break;

            case PE_SPEED_FASTREWIND_8:
                m_cur_play_speed = 4;
                break;

            case PE_SPEED_FASTREWIND_16:
                m_cur_play_speed = 5;
                break;

            case PE_SPEED_FASTREWIND_24:
                m_cur_play_speed = 6;
                break;

            default:
                break;
        }
        m_nxt_playstate = perev;
    }
    else if ((PE_SPEED_SLOWFORWARD_2 <= speed) && (PE_SPEED_SLOWFORWARD_8 >= speed))
    {
        /* slow forward */
        m_search_time = INVALID_TIME;
        switch (speed)
        {
            case PE_SPEED_SLOWFORWARD_2:
                m_cur_play_speed = -1;
                break;

            case PE_SPEED_SLOWFORWARD_4:
                m_cur_play_speed = -2;
                break;

            case PE_SPEED_SLOWFORWARD_8:
                m_cur_play_speed = -3;
                break;

            default:
                break;
        }
        m_nxt_playstate = peslow;
    }
    else
    {
        ret = 1;
    }

    if (p_dec_func_table)
    {
        (*p_dec_func_table)[m_cur_playstate][m_nxt_playstate](&m_decoder_ctrl, m_cur_play_speed, m_search_time);
    }

    LEAVE_DEC_CMD();
    return ret;
}

