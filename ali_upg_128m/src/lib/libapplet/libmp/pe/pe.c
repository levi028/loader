  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pe.c
*
*    Description: This file describes play engine operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <asm/chip.h>
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
#include <api/libimagedec/imagedec.h>
#include <api/libmp/pe.h>
#ifdef NETWORK_SUPPORT
#include <api/libnet/libnet.h>
#include <api/libtcpip/lwip/inet.h>
#endif

#include "music/plugin.h"
#include "music/music_engine.h"
#include "image/image_engine.h"
#include "video_advanced/mp_player.h"
#include "video_advanced/avformat.h"
#ifdef MP4_CONTAINER
#include "av_player/av_player.h"
#endif
#include "play_engine_common.h"

#define PE_DBG_PRINTF libc_printf

#define ENABLE_PE_CACHE
#define MIN_FILE_NAME_LEN_WITH_EXT      3

#ifdef MAX_FILE_NAME_SIZE
#undef MAX_FILE_NAME_SIZE
#endif

#define MAX_FILE_NAME_SIZE      (2048+4)

__MAYBE_UNUSED__ static struct pe_feature_cfg g_mp_feature_cfg = {0, 0, 0, 0, 0}; /* */
static UINT8 pe_init_time = 0;
static struct pe_music_cfg g_pe_music_cfg = {0, 0, 0, 0, NULL, 0, 0, {0}};    //for release the memory in pe_cleanup

static char m_pe_file_name_buf[MAX_FILE_NAME_SIZE] = {0};

#ifdef MORETV_PLUGIN_SUPPORT
UINT8 pe_inited = 0;
struct pe_music_cfg pe_music_cfg_bk;
struct pe_image_cfg pe_image_cfg_bk;
struct pe_video_cfg pe_video_cfg_bk;
#endif

//################################################################################
//## wrapper for music engine
int music_play(char *filename)
{
    if(!filename)
    {
        libc_printf("%s: filename is null \n", __FUNCTION__);
        return -1;
    }

    PE_DBG_PRINTF("Into %s\n", __func__);

    MEMSET(m_pe_file_name_buf, 0, MAX_FILE_NAME_SIZE);
    strncpy(m_pe_file_name_buf, filename, MAX_FILE_NAME_SIZE - 1);
    m_pe_file_name_buf[MAX_FILE_NAME_SIZE - 1] = 0;

   return mp_play(m_pe_file_name_buf, 2, 0, MP_MEDIA_MUSIC);

}

void music_seek(int time)
{
  static long long last_play_time = 0;
  static long long last_seek_time = 0;
  long long now_time, seek_time;
  long long total_time = 0;

  now_time = (int)mp_get_current_time();
	total_time = mp_get_total_time();

  PE_DBG_PRINTF("%s: now: %llu, lp: %llu, ls: %llu\n", __func__, now_time, last_play_time, last_seek_time);
  if(now_time == last_play_time)
  {
    seek_time = last_seek_time + time;
  }
  else
  {
    seek_time = now_time + time;
  }
  //The value of "seek_time" maybe a negative number when pressing "FB", so it need to set to 0 immediately.
  //When compare the value of "total_time" and "seek_time", it will make mistake because the two values
  //have different types. The "last_seek_time" will be go from zero to total_time.
   if(seek_time < 0)
    seek_time = 0;

	if (total_time && (total_time < seek_time))
	{
		last_seek_time = total_time;
		last_play_time = now_time;
		return;
	}


  mpg_cmd_search_proc(seek_time);
  last_seek_time = seek_time;
  last_play_time = now_time;

  return;
}

DWORD music_stop(void)
{
   PE_DBG_PRINTF("Into %s\n", __func__);
   return mpg_cmd_stop_proc(1);
}

int music_pause(void)
{
    PE_DBG_PRINTF("Into %s\n", __func__);
    return mp_toggle_pause_music();
}

DWORD music_get_time(void)
{
//    PE_DBG_PRINTF("Into %s\n", __func__);
   return mp_get_current_time();
}

void music_set_eq(int on, float preamp, float *bands)
{
    PE_DBG_PRINTF("Into %s\n", __func__);
    if(bands)
        music_engine_set_eq(on, preamp, bands);
}

void music_engine_set_disable_seek(UINT8 disable )
{
    PE_DBG_PRINTF("Into %s\n", __func__);
    mp_set_disable_seek(disable, MP_MEDIA_MUSIC);
}

UINT8 music_engine_get_disable_seek(void)
{
    return mp_get_disable_seek(MP_MEDIA_MUSIC);
}

int music_get_song_info(char *filename, music_info *music_info)
{
    if((!filename) ||(!music_info))
    {
        ASSERT(0);
        libc_printf("%s: filename or music_info is null \n", __FUNCTION__);
        return -1;
    }
    return mp_get_song_info(filename, music_info);
}


int music_get_decoder_info(char *filename, decoder_info *decoder_info)
{
    if((!filename) || (!decoder_info))
    {
        ASSERT(0);
        libc_printf("%s: filename or decoder_info is null \n", __FUNCTION__);
        return -1;
    }
    return mp_get_decoder_info(filename, decoder_info);
}


//################################################################################
//## wrapper for image engine
static int image_init(struct pe_image_cfg *pe_image_cfg)
{
    int ret = -1;

    if(!pe_image_cfg)
    {
         libc_printf("%s: pe_image_cfg is null \n", __FUNCTION__);
         return -1;
    }
    ret = image_engine_init(pe_image_cfg);
    return ret;
}

void image_restore_vpo_rect(void)
{
    struct rect        main_src_rect;
    struct rect        main_dst_rect;
    struct vpo_device *pvpo_sd = NULL;

    MEMSET(&main_src_rect, 0, sizeof(struct rect));
    MEMSET(&main_dst_rect, 0, sizeof(struct rect));
    vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),FALSE);
    pvpo_sd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    if(NULL != pvpo_sd)
    {
        vpo_win_onoff(pvpo_sd, FALSE);    // avoid green screen
    }
    osal_task_sleep(50);
    //restore the vop
    main_src_rect.u_start_x = 0;
    main_src_rect.u_start_y = 0;
    main_dst_rect.u_start_x = 0;
    main_dst_rect.u_start_y = 0;
    main_src_rect.u_width = PICTURE_WIDTH;
    main_src_rect.u_height = PICTURE_HEIGHT;
    main_dst_rect.u_width = SCREEN_WIDTH;
    main_dst_rect.u_height = SCREEN_HEIGHT;
    vpo_zoom((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),&main_src_rect,&main_dst_rect);
    if(NULL != pvpo_sd)
    {
        vpo_zoom((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1),&main_src_rect,&main_dst_rect);
    }

}
int image_cleanup(void)
{
    int ret = -1;

    //cleanup the image decoder
	ret = image_engine_abort();
    if(ret<0)
        ali_trace(&ret);
    ret = image_engine_cleanup();
    return ret;
}

int image_abort(void)
{
    int ret = -1;

    ret = image_engine_abort();
    return ret;
}

int image_rotate(unsigned char rotate_angle)
{
    int ret = -1;

    ret = image_engine_rotate(rotate_angle);
    return ret;
}

int image_zoom(struct rect *dstrect, struct rect *srcrect)
{
    int ret = -1;

    if((!dstrect) || (srcrect == NULL))
    {
        libc_printf("%s: dstrect is null \n", __FUNCTION__);
        return -1;
    }
    ret = image_engine_zoom(dstrect, srcrect);
    return ret;
}

int image_2d_to_3d_swap(int display_type)
{
    int ret = -1;

    ret = image_engine_2d_to_3d_swap(display_type);
    return ret;
}

/*
int image_3d_user_option(int output_format, int user_depth, int user_shift, int user_vip)
{
    int ret;
    struct image_3d_para para;
    para.output_format = output_format;
    para.user_depth = user_depth;
    para.user_shift = user_shift;
    para.user_vip = user_vip;
    ret = image_engine_3d_user_option(&para);

    return ret;
}*/

int image_set_disp_param(int display_type, int output_format, int user_depth, int user_shift, int user_vip)
{
    int ret = -1;
    struct image_3d_para para;

    MEMSET(&para, 0, sizeof(struct image_3d_para));
    para.output_format = output_format;
    para.user_depth = user_depth;
    para.user_shift = user_shift;
    para.user_vip = user_vip;
    ret = image_engine_set_disp_param(display_type, &para);
    return ret;
}

extern int image_engine_decode_opentv(char *filename, UINT32 file_addr, UINT32 file_len, int scale_coef, struct image_config *cfg);
int image_decode_opentv(char *file, UINT32 file_addr, UINT32 file_len, int mode, int left, int top, int width, int height, unsigned char rotate, int scale_coef)
{
	int ret;
	struct image_config cfg;
	MEMSET(&cfg, 0, sizeof(cfg));

	if(mode == IMAGEDEC_REAL_SIZE)
	{
		cfg.decode_mode = IMAGEDEC_REAL_SIZE;
		cfg.src_top = top;
		cfg.src_left = left;
		cfg.src_width= width;
		cfg.src_height = height;
		cfg.rotate = ANG_ORI;
	}
	else if(mode == IMAGEDEC_THUMBNAIL)
	{
		cfg.decode_mode = IMAGEDEC_THUMBNAIL;
		cfg.dest_top = top;
		cfg.dest_left = left;
		cfg.dest_width= width;
		cfg.dest_height = height;
		cfg.rotate = ANG_ORI;
	}
    else if (mode == IMAGEDEC_MULTI_PIC)
    {
        cfg.decode_mode = IMAGEDEC_MULTI_PIC;
		cfg.dest_top = top;
		cfg.dest_left = left;
		cfg.dest_width= width;
		cfg.dest_height = height;
		cfg.rotate = ANG_ORI;
	}
	else if(mode == IMAGEDEC_FULL_SRN)
	{
		cfg.decode_mode = IMAGEDEC_FULL_SRN;
		cfg.dest_top = top;
		cfg.dest_left = left;
		cfg.dest_width= width;
		cfg.dest_height = height;
		cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
	}
	else if(mode == IMAGEDEC_SIZEDEFINE)
	{
		cfg.decode_mode = IMAGEDEC_SIZEDEFINE;
		cfg.dest_top = top;
		cfg.dest_left = left;
		cfg.dest_width= width;
		cfg.dest_height = height;
		cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
	}
	else
	{
		cfg.decode_mode = IMAGEDEC_FULL_SRN;
		cfg.dest_top = top;
		cfg.dest_left = left;
		cfg.dest_width= width;
		cfg.dest_height = height;
		cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
	}

	ret = image_engine_decode_opentv(file, file_addr, file_len, scale_coef, &cfg);

	return ret;


}

int image_decode(char *file, int mode, int left, int top, int width, int height, unsigned char rotate)
{
    int ret = -1;
    struct image_config cfg;

    if(!file)
    {
        libc_printf("%s: file is null \n", __FUNCTION__);
        return -1;
    }
    MEMSET(&cfg, 0, sizeof(cfg));
    switch(mode)
    {
       case IMAGEDEC_REAL_SIZE:
        cfg.decode_mode = IMAGEDEC_REAL_SIZE;
        cfg.src_top = top;
        cfg.src_left = left;
        cfg.src_width= width;
        cfg.src_height = height;
        cfg.rotate = ANG_ORI;
        break;
       case IMAGEDEC_THUMBNAIL:
        cfg.decode_mode = IMAGEDEC_THUMBNAIL;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = ANG_ORI;
        break;
       case IMAGEDEC_MULTI_PIC:
        cfg.decode_mode = IMAGEDEC_MULTI_PIC;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = ANG_ORI;
        break;
       case IMAGEDEC_FULL_SRN:
        cfg.decode_mode = IMAGEDEC_FULL_SRN;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
        break;
       case IMAGEDEC_SIZEDEFINE:
        cfg.decode_mode = IMAGEDEC_SIZEDEFINE;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
        break;
       default:
        cfg.decode_mode = IMAGEDEC_FULL_SRN;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
        break;
    }

    ret = image_engine_decode(file, &cfg);

    return ret;
}

int image_decode_ex(char *file, int mode, int left, int top, int width, int height,
                                         unsigned char rotate, struct image_slideshow_effect *effect)
{
    int ret = -1;
    struct image_config cfg;

    MEMSET(&cfg, 0, sizeof(cfg));
    if((!file) || (!effect))
    {
        libc_printf("%s: file is null \n", __FUNCTION__);
        return -1;
    }
    switch(mode)
    {
       case IMAGEDEC_REAL_SIZE:
        cfg.decode_mode = IMAGEDEC_REAL_SIZE;
        cfg.src_top = top;
        cfg.src_left = left;
        cfg.src_width= width;
        cfg.src_height = height;
        cfg.rotate = ANG_ORI;
        break;
       case IMAGEDEC_THUMBNAIL:
        cfg.decode_mode = IMAGEDEC_THUMBNAIL;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = ANG_ORI;
        break;
       case IMAGEDEC_MULTI_PIC:
        cfg.decode_mode = IMAGEDEC_MULTI_PIC;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = ANG_ORI;
        break;
       default:
        cfg.decode_mode = IMAGEDEC_FULL_SRN;
        cfg.dest_top = top;
        cfg.dest_left = left;
        cfg.dest_width= width;
        cfg.dest_height = height;
        cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;
        cfg.effect = effect;
        break;
    }

    ret = image_engine_decode(file, &cfg);
    return ret;
}

int image_display(char *file, struct image_display_t *pdisplay)
{
    int ret = -1;

    if((!file)||(!pdisplay))
    {
        libc_printf("%s: file or pdisplay is null\n", __FUNCTION__);
        return -1;
    }
    pdisplay->dest_top = 0;
    pdisplay->dest_left = 0;
    pdisplay->dest_width = 720;
    pdisplay->dest_height = 576;

    struct image_config *cfg = (struct image_config *)pdisplay;

    ret = image_engine_decode(file, cfg);
    return ret;
}

int image_get_info(char *filename, struct image_info *info )
{
    int ret = -1;

    if((!filename) ||(!info))
    {
        ASSERT(0);
        libc_printf("%s: filename is null \n", __FUNCTION__);
        return -1;
    }
    ret = image_engine_get_info(filename, info);
    return ret;
}

/*
int image_ioctl (char *filename, UINT32 cmd, UINT32 para)
{
    int ret = 0;

    //ret = image_engine_ioctl(filename, cmd, para);

    return ret;
}*/

/*
int pe_cfg(struct pe_feature_cfg *pcfg)
{
    if(!pcfg)
    {
        return -1;
    }

    MEMCPY((void*)&g_mp_feature_cfg, pcfg ,sizeof(struct pe_feature_cfg));

    return 0;
}
*/

//################################################################################
//## wrapper for video engine
// UI is calling the following 3 APIs now, so we must implement them here.
DWORD mpgfile_decoder_get_play_time()
{
    return mp_get_current_time();
}
DWORD mpgfile_decoder_get_play_time_ms()
{
    __MAYBE_UNUSED__ int ret = 0;

    return mp_get_current_time_ms();
}

DWORD mpg_file_get_chapter_info(PDEC_CHAPTER_INFO p_dec_chapter_info)
{
    __MAYBE_UNUSED__ int ret = 0;

    /* Check if the input parameter p_dec_chapter_info is legal */
    if (NULL == p_dec_chapter_info)
    {
        PE_DBG_PRINTF("\n%s:%d,  p_dec_chapter_info = NULL,  Illegal!\n", __func__, __LINE__);
        return RET_FAILURE;
    }

    return avplayer_get_chapter_info(p_dec_chapter_info);
}

//API
DWORD mpg_file_get_stream_info(PDEC_STREAM_INFO p_dec_stream_info)
{
    DWORD ret = 0;

    /* Check if the input parameter p_dec_stream_info is legal */
    if (NULL == p_dec_stream_info)
    {
        PE_DBG_PRINTF("\n%s:%d,  p_dec_stream_info = NULL,  Illegal!\n", __func__, __LINE__);
        return RET_FAILURE;
    }

    MEMSET(p_dec_stream_info, 0, sizeof (DEC_STREAM_INFO));

    mp_get_stream_info(NULL, (void *)p_dec_stream_info);
    if (!p_dec_stream_info->width)
    {
        p_dec_stream_info->width = 720;
    }

    if (!p_dec_stream_info->height)
    {
        p_dec_stream_info->height = 576;
    }

    return ret;
}

int mpg_file_get_media_type()
{
    int media_type;

    media_type = get_current_media_type();

    return media_type;
}

int mpgget_total_play_time()
{
    DEC_STREAM_INFO dec_stream_info;
    UINT32 try_cnt = 0;
    UINT32 max_try_cnt = 500;

    while(1 == mpg_file_get_stream_info(&dec_stream_info))    //Not got info
    {
        try_cnt++;
        osal_task_sleep(1);
        if (try_cnt > max_try_cnt)
        {
            break;
        }
    }
    if (0 == mpg_file_get_stream_info(&dec_stream_info))  // Get info ok
    {
        return dec_stream_info.total_time;// timescale:second
    }
    return 0;
}

DWORD mpgget_total_play_time_ms()
{
    __MAYBE_UNUSED__ int ret = 0;
    __MAYBE_UNUSED__ DWORD    time_ms = 0;

    return mp_get_total_time();
}

void video_set_disable_seek(UINT8 disable)
{
    mp_set_disable_seek(disable, MP_MEDIA_VIDEO);
}

UINT8 video_get_disable_seek(void)
{
    return mp_get_disable_seek(MP_MEDIA_VIDEO);
}

int video_decode(char *file, UINT8 video_stream_type, enum snd_dup_channel audio_channel, BOOL preview)
{
    /*
        Note:
        video_decode() run in CPU, video_engine_decode() is a remote call in dual-cpu architecture.
        video_engine_decode() need [char *file] be global memory. malloc it here is a general
        method for all projects, include single-cpu, dual-cpu, dual-cpu with hw security enabled.
        Q: why not malloc it in video_engine_decode()?
        A: video_engine_decode() run in SEE, CPU cannot read SEE memory
                                                                       -- Jerry Long on 2010-07-08
    */
    if(!file)
    {
        ASSERT(0);
        libc_printf("%s: file is null\n", __FUNCTION__);
        return -1;
    }

    MEMSET(m_pe_file_name_buf, 0, MAX_FILE_NAME_SIZE);
    strncpy(m_pe_file_name_buf, file, MAX_FILE_NAME_SIZE - 1);
    m_pe_file_name_buf[MAX_FILE_NAME_SIZE - 1] = 0;

	return mp_play(m_pe_file_name_buf, audio_channel, preview, MP_MEDIA_VIDEO);

}

int video_dec_file(char *path, BOOL preview)
{
    if(!path)
    {
        ASSERT(0);
        libc_printf("%s: path is null\n", __FUNCTION__);
        return -1;
    }
#ifdef SUPPORT_ES_PLAYER
    int ret = -1;

    if((ret = network_stream_test(path, preview)) > 0)
        return ret;
#endif

#ifdef SUPPORT_MPEG4_TEST
    return video_decode(path, UNKNOWN_STREAM, SND_DUP_NONE, preview);
#else
    return video_decode(path, MPEG_12_FILE_STREAM, SND_DUP_NONE, preview);
#endif
}



#ifdef SUPPORT_MPEG4_TEST
int video_set_output(enum VDEC_OUTPUT_MODE eoutmode, struct vdec_pipinfo *pinitinfo)
{
    if(!pinitinfo)
    {
        ASSERT(0);
        libc_printf("%s: pinitinfo is null\n", __FUNCTION__);
        return -1;
    }
    return video_engine_set_output(eoutmode, pinitinfo);
}
#endif

/*
struct pe_music_cfg
{
    //buff
    UINT32 pcm_out_buff; // ylm :not used
    UINT32 pcm_out_buff_size; // ylm :not used
    UINT32 processed_pcm_buff; // ylm :not used
    UINT32 processed_pcm_buff_size; // ylm :not used

    //callback function for application
    mp_callback_func mp_cb;
    UINT32 pe_cache_buf_start_addr;
    UINT32 pe_cache_buf_len;
    UINT32 reserved[8];    //// use in the future, should init to zero
}; ylm: pcm_out_buff transfer to SEE?
*/

//########################################################################################
//## PE functions
int pe_init(struct pe_music_cfg *pe_music_cfg, struct pe_image_cfg *pe_image_cfg, struct pe_video_cfg *pe_video_cfg)
{
    int image_ret = -1;
    int mp_ret = -1;

#if defined(NETWORK_SUPPORT) || defined(SUPPORT_ES_PLAYER)//net work
    pe_cache_cb_fp fp;
#endif

    if((NULL==pe_music_cfg)||(NULL==pe_image_cfg)||(NULL==pe_video_cfg))
    {
        return -1;
    }

    //MEMSET(&g_pe_music_cfg,0,sizeof(pe_music_cfg));
    MEMSET(&g_pe_music_cfg, 0x0, sizeof (g_pe_music_cfg));

    //if app dont init this addr,then default config add by system
    if((0==pe_music_cfg->pe_cache_buf_start_addr)||(0==pe_music_cfg->pe_cache_buf_len))
    {
        pe_music_cfg->pe_cache_buf_len=MUSIC_PE_CACHE_SIZE_MAX;
        pe_music_cfg->pe_cache_buf_start_addr= (UINT32)MALLOC(pe_music_cfg->pe_cache_buf_len);
        if(0==pe_music_cfg->pe_cache_buf_start_addr)
        {
            return -1;
        }
        g_pe_music_cfg.pe_cache_buf_start_addr=pe_music_cfg->pe_cache_buf_start_addr;
        g_pe_music_cfg.pe_cache_buf_len=pe_music_cfg->pe_cache_buf_len;
    }


#ifdef MORETV_PLUGIN_SUPPORT
    //bk params
    MEMCPY(&pe_music_cfg_bk, pe_music_cfg, sizeof(struct pe_music_cfg));
    MEMCPY(&pe_image_cfg_bk, pe_image_cfg, sizeof(struct pe_image_cfg));
    MEMCPY(&pe_video_cfg_bk, pe_video_cfg, sizeof(struct pe_video_cfg));
    pe_inited = 1;
#endif

    //init the image decoder
    image_ret = image_init(pe_image_cfg);

    mp_ret = mp_init(pe_video_cfg, pe_music_cfg);

#ifdef MP4_CONTAINER // ylm: need suppport???
    av_player_config player_config;
    MEMSET((void *)&player_config, 0, sizeof(av_player_config));
    player_config.buf_start1 =AVP_BUF1_ADDR;
    player_config.buf_size1 = AVP_BUF1_LEN;//0x1400000;//AVP_BUF2_LEN;//#define AVP_BUF1_LEN   0xF00000 //15M
    player_config.buf_start2 = AVP_BUF2_ADDR;
    player_config.buf_size2 = AVP_BUF2_LEN;
    avp_cache_init(__MM_MP_BUFFER_ADDR);//
    av_player_init(&player_config);
    g_mp4_av_player_handle = av_player_create();
#endif

#ifdef ENABLE_PE_CACHE
	if(0 != pe_cache_init())
        return -1;

#ifdef NETWORK_SUPPORT//net work
    MEMSET(&fp, 0, sizeof(pe_cache_cb_fp));
    fp.file_open = url_open;
    fp.file_close = (pe_cache_fclose_fp)url_close;
    fp.file_eof = NULL;
    fp.file_read = url_read;
    fp.file_seek = NULL;
    fp.file_tell = NULL;
    pe_cache_register_net_fp(fp,PE_NET_FILE_TYPE_URL);
#endif

#ifdef SUPPORT_ES_PLAYER
    MEMSET(&fp, 0, sizeof(pe_cache_cb_fp));
    fp.file_open = ali_stream_open;
    fp.file_close = ali_stream_close;
    fp.file_eof = ali_stream_end;
    fp.file_read = ali_stream_read;
    fp.file_seek = ali_stream_seek;
    fp.file_tell = ali_stream_tell;
    pe_cache_register_net_fp(fp,PE_ALI_SPECIAL_STREAM);
#endif

#endif

	if((image_ret == 0) && (mp_ret == 0))
    {
        pe_init_time++;
    }
    return mp_ret;
}

int pe_cleanup(void)
{
    //release the image decoder
    //don't release pe when usb plug out
    PE_DBG_PRINTF("Into %s\n", __func__);

    if(1 == pe_init_time)
    {
        int ret = -1;

        /* stop video play*/
        mpg_cmd_stop_proc(0);

        /* stop music play*/
        mpg_cmd_stop_proc(1);

		ret = image_cleanup();
        if(ret < 0)
            ali_trace(&ret);

        pe_init_time = 0;

        mp_cleanup();

	    ret = pe_cache_release();
        if(ret < 0)
            ali_trace(&ret);
    }
    else
    {
        if(pe_init_time != 0)
        {
            pe_init_time--;
        }
        //do nothing some module still need pe
    }

    if(0 != g_pe_music_cfg.pe_cache_buf_start_addr)
    {
        FREE((void *)g_pe_music_cfg.pe_cache_buf_start_addr);

        g_pe_music_cfg.pe_cache_buf_len = 0;
        g_pe_music_cfg.pe_cache_buf_start_addr = 0;
    }
    return RET_SUCCESS;
}

#ifdef MORETV_PLUGIN_SUPPORT
int pe_reinit()
{
    pe_init(&pe_music_cfg_bk, &pe_image_cfg_bk, &pe_video_cfg_bk);
    return 0;
}
#endif

int get_file_name_ext(const char *filename, char *pext, int ext_size)
{
    int file_name_len = 0;
    int i = 0;

    if((!filename) || (!pext) || (ext_size<=1))
    {
        ASSERT(0);
        libc_printf("%s: filename or pext is null\n", __FUNCTION__);
        return 0;
    }
    file_name_len = strlen(filename);
    if (file_name_len <= MIN_FILE_NAME_LEN_WITH_EXT)
    {
        *pext = 0;
        return 0;
    }
    for ( i = file_name_len; i; i--)
    {
        if (filename[i-1] == '.')
        {
            break;
        }
    }

    if (i == 0)
    {
        *pext = 0;
        return 0;
    }

    strncpy(pext, &filename[i], ext_size - 1);
	pext[ext_size-1] = 0;
    for (i = 0; pext[i]&& (i< ext_size); i++)
    {
        if ((pext[i] >= 'A') && (pext[i] <= 'Z'))
        {
            pext[i] |= 0x20;
        }
    }

    return i;
}

