/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
   disclosed to unauthorized individual.
*    File: es_player_interface.c
*
*    Description: This file describes es_player_interface operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <hld/decv/decv.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca.h>
#include <hld/decv/decv_media_player.h>
#include <api/libmp/es_player_api.h>
#include <hld/sbm/sbm.h>
#include "video_advanced/mp_player.h"
#include "video_advanced/mp_dbg.h"


#if 1
#define ES_PLAYER_PRINTF   libc_printf
#else
#define ES_PLAYER_PRINTF(...)
#endif

struct ali_decv_private
{
    void *dec_handle;
    unsigned int codec_id;
    int vpkt_data_fd;
    int vpkt_hdr_fd;
    int vsink_in_fd;
    int vpkt_data_mode;
    int vpkt_hdr_mode;
    int vsink_in_mode;
};

struct ali_deca_private
{
    void *dec_handle;
    unsigned int codec_id;
    int apkt_data_fd;
    int apkt_hdr_fd;
    int adec_out_fd;
    int asink_in_fd;
    int apkt_data_mode;
    int apkt_hdr_mode;
    int adec_out_mode;
    int asink_in_mode;
};


struct ali_decore_buffer
{
    UINT32 vpkt_hdr_addr;
    UINT32 vpkt_hdr_size;
    UINT32 vpkt_data_addr;
    UINT32 vpkt_data_size;
    UINT32 vdec_out_addr;
    UINT32 vdec_out_size;
    UINT32 vsink_in_addr;
    UINT32 vsink_in_size;
    UINT32 apkt_hdr_addr;
    UINT32 apkt_hdr_size;
    UINT32 apkt_data_addr;
    UINT32 apkt_data_size;
    UINT32 adec_out_addr;
    UINT32 adec_out_size;
    UINT32 asink_in_addr;
    UINT32 asink_in_size;
    UINT32 frm_buf_addr;
    UINT32 frm_buf_size;
    UINT32 pcm_buf_addr;
    UINT32 pcm_buf_size;
};


static struct ali_decv_private *decvp = NULL;
static struct ali_deca_private *decap = NULL;

#ifdef MP_DEBUG
UINT32 g_dbg_mp_option=0;
UINT32 g_dbg_apkt_cnt, g_dbg_vpkt_cnt;
UINT32 g_dbg_total_vpkt_size,g_dbg_total_apkt_size;
UINT32 g_dbg_first_vpkt_tick=0;
FILE *file_audio_dump;
FILE *file_video_dump;
#endif

static int ali_decore_alloc_buffer(ali_sbm_info pe_info, struct ali_decore_buffer *decore_buffer)
{ // ylm: allocate sbm buffer/video decoder buffer
    UINT32 mp_mem_addr = 0;
    UINT32 mp_mem_size = 0;
    UINT32 video_frm_addr = 0;
    UINT32 video_frm_size = 0;
    UINT32 run_addr = 0;
    UINT32 run_size = 0;

	if((decore_buffer == NULL) || (pe_info.decoder_start == NULL) || (pe_info.mem_start == NULL))
    {
        return -1;
    }

    MEMSET(decore_buffer, 0, sizeof(struct ali_decore_buffer));

    video_frm_addr  = (UINT32)pe_info.decoder_start; //decoder buffer
    video_frm_size  = pe_info.decv_buf_size;
    mp_mem_addr     = (UINT32)pe_info.mem_start;  // sbm buffer
    mp_mem_size     = pe_info.mem_size;

    if(mp_mem_addr == (video_frm_addr + video_frm_size - mp_mem_size))
    {
        run_size = mp_mem_size - video_frm_size;
        run_addr = mp_mem_addr;
    }
    else
    {
        run_size = mp_mem_size;
        run_addr = mp_mem_addr;
    }

    if(run_size >= 0xE00000)
    {
        decore_buffer->vpkt_data_size = 0xA00000;
        decore_buffer->apkt_data_size = 0x300000;
    }
    else if(run_size >= 0xC00000)
    {
        decore_buffer->vpkt_data_size = 0x900000;
        decore_buffer->apkt_data_size = 0x280000;
    }
    else if(run_size >= 0xA00000)
    {
        decore_buffer->vpkt_data_size = 0x780000;
        decore_buffer->apkt_data_size = 0x200000;
    }
    else if(run_size >= 0x800000)
    {
        decore_buffer->vpkt_data_size = 0x600000;
        decore_buffer->apkt_data_size = 0x180000;
    }
	else if(run_size >= 0x600000)
    {
		decore_buffer->vpkt_data_size = 0x430000;
        decore_buffer->apkt_data_size = 0x150000;
	}
	else if(run_size >= 0x400000)
	{
		decore_buffer->vpkt_data_size = 0x280000;
        decore_buffer->apkt_data_size = 0x100000;
	}
	else 
	{
		decore_buffer->vpkt_data_size = 0x1A0000;
        decore_buffer->apkt_data_size = 0xE0000;
    }

    decore_buffer->pcm_buf_addr = 0;
    decore_buffer->pcm_buf_size = 0;
    decore_buffer->frm_buf_addr = video_frm_addr;  //ylm???, not be transfered to see.
    decore_buffer->frm_buf_size = video_frm_size;
    decore_buffer->vdec_out_size = 0;//128 * sizeof(struct av_frame);
    decore_buffer->vsink_in_size = 0;//128 * sizeof(struct av_frame);
    decore_buffer->adec_out_size = 0;//128 * sizeof(struct audio_frame);
    decore_buffer->vpkt_hdr_size = 4096 * sizeof(struct av_packet);
    decore_buffer->apkt_hdr_size = 1024 * sizeof(struct av_packet);
    decore_buffer->asink_in_size = 128 * sizeof(struct audio_frame);

    if(decore_buffer->vpkt_hdr_size + decore_buffer->vpkt_data_size
            + decore_buffer->vdec_out_size + decore_buffer->vsink_in_size
            + decore_buffer->apkt_hdr_size + decore_buffer->apkt_data_size
            + decore_buffer->adec_out_size + decore_buffer->asink_in_size > run_size)
    {
        ES_PLAYER_PRINTF("decore not enough memory %d\n", run_size);
        return -1;
    }

    decore_buffer->vpkt_data_addr = run_addr;
    run_addr += decore_buffer->vpkt_data_size;
    run_size -= decore_buffer->vpkt_data_size;
    decore_buffer->vpkt_hdr_addr = run_addr;
    run_addr += decore_buffer->vpkt_hdr_size;
    run_size -= decore_buffer->vpkt_hdr_size;
    decore_buffer->vdec_out_addr = run_addr;
    run_addr += decore_buffer->vdec_out_size;
    run_size -= decore_buffer->vdec_out_size;
    decore_buffer->vsink_in_addr = run_addr;
    run_addr += decore_buffer->vsink_in_size;
    run_size -= decore_buffer->vsink_in_size;
    decore_buffer->apkt_data_addr = run_addr;
    run_addr += decore_buffer->apkt_data_size;
    run_size -= decore_buffer->apkt_data_size;
    decore_buffer->apkt_hdr_addr = run_addr;
    run_addr += decore_buffer->apkt_hdr_size;
    run_size -= decore_buffer->apkt_hdr_size;
    decore_buffer->adec_out_addr = run_addr;
    run_addr += decore_buffer->adec_out_size;
    run_size -= decore_buffer->adec_out_size;
    decore_buffer->asink_in_addr = run_addr; // ylm??? not used
    run_addr += decore_buffer->asink_in_size;
    run_size -= decore_buffer->asink_in_size;

    return 0;

}

int ali_video_decore_ioctl(ali_video_config *decv_config, int cmd, void *param1, void *param2)
{
    int ret = -1;

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("decoder has not been opened\n");
        return -1;
    }

    switch(cmd)
    {
        case VDEC_CMD_INIT:
            ret = es_player_decv_ioctl(NULL, cmd, (void *)&decvp->dec_handle, param2);
            break;
        case VDEC_CMD_EXTRA_DADA:
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_SW_RESET:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
		case VDEC_STEP_DISPLAY:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
        case VDEC_CFG_DECODE_MODE:
        case VDEC_CFG_DISPLAY_RECT:
		case VDEC_SET_DISPLAY_SLOW_RATIO:
        case VDEC_CMD_GET_STATUS:
        case VDEC_DQ_GET_COUNT:
        case VDEC_SET_MALLOC_DONE:
        case VDEC_GET_MEM_RANGE:
            ret = es_player_decv_ioctl(decvp->dec_handle, cmd, param1, param2);
            break;
        default:
            ret = -1;
            break;
    }

    if(ret != VDEC_SUCCESS)
    {
        ES_PLAYER_PRINTF("video decore ioctl fail %d, cmd: %d\n", ret, cmd);
        ret = -1;
    }

    return ret;
}

/** Open video decoder with given codec.
 *  \param decv_config The param which will be used to initialize decoder.
 *  \param pe_info   share memory addr and decoder addr
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_open(ali_video_config *decv_config, ali_sbm_info pe_info)
{
    struct ali_decore_buffer decore_buffer;
    struct sbm_config sbm_info;
    vdec_init vdec_init;
    int pkt_sbm_idx = 0;
    int dec_out_sbm_idx = 0;
    int display_sbm_idx = 0;
    int ret = -1;
    enum video_decoder_type select = MPEG2_DECODER;
	PDEC_INIT_PAR pdec_init_par = (PDEC_INIT_PAR)pe_info.pdec_init_par;

    MEMSET(&decore_buffer, 0, sizeof(struct ali_decore_buffer));
    MEMSET(&sbm_info, 0, sizeof(struct sbm_config));
    MEMSET(&vdec_init, 0, sizeof(vdec_init));

    if((decvp != NULL) || (decv_config == NULL))
    {
        ES_PLAYER_PRINTF("decoder has been inited\n");
        return ret;
    }

    if(ali_decore_alloc_buffer(pe_info, &decore_buffer) < 0)
    {
        return -1;
    }

    decvp = MALLOC(sizeof(struct ali_decv_private));

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("malloc decoder private fail\n");
        return -1;
    }

    MEMSET(decvp, 0, sizeof(struct ali_decv_private));

    if(decv_config->codec_tag == rv30 || decv_config->codec_tag == rv40)
        decvp->codec_id = rmvb;
    else
        decvp->codec_id = decv_config->codec_tag;

    MEMSET(&sbm_info, 0, sizeof(sbm_info));
    MEMSET(&vdec_init, 0, sizeof(vdec_init));

    sbm_info.buffer_addr = decore_buffer.vpkt_data_addr;
    sbm_info.buffer_size = decore_buffer.vpkt_data_size;
    sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = 128*1024;
    sbm_info.wrap_mode = SBM_MODE_PACKET;
    sbm_info.lock_mode = SBM_SPIN_LOCK;
    decvp->vpkt_data_mode = SBM_MODE_PACKET;

    ret = ali_sbm_create(VPKT_DATA_SBM_IDX, sbm_info);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore create sbm fail\n");
        goto fail;
    }

    decvp->vpkt_data_fd = 1;
    sbm_info.buffer_addr = decore_buffer.vpkt_hdr_addr;
    sbm_info.buffer_size = decore_buffer.vpkt_hdr_size;
    sbm_info.reserve_size = 10*sizeof(struct av_packet);
    sbm_info.wrap_mode = SBM_MODE_NORMAL;
    sbm_info.lock_mode = SBM_SPIN_LOCK;
    decvp->vpkt_hdr_mode = SBM_MODE_NORMAL;

    ret = ali_sbm_create(VPKT_HDR_SBM_IDX, sbm_info);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore create sbm fail\n");
        goto fail;
    }

    decvp->vpkt_hdr_fd = 1;

	ret = ali_sbm_reset(VPKT_DATA_SBM_IDX);
    if(ret<0)
        ali_trace(&ret);

	ret = ali_sbm_reset(VPKT_HDR_SBM_IDX);
    if(ret<0)
        ali_trace(&ret);


    if((decvp->codec_id == h264) || (decvp->codec_id == mpg2)
        || (decvp->codec_id == hevc))
    {
        switch(decvp->codec_id)
        {
            case mpg2:
                select = MPEG2_DECODER;
                break;
            case h264:
                select = H264_DECODER;
                break;
            case hevc:
                select = H265_DECODER;
                break;
            default:
                break;
        }

        video_decoder_select(select, decv_config->preview);
        vdec_start(get_selected_decoder());
    }

	vdec_init.phw_mem_cfg = (vdec_hwmem_config *)&pdec_init_par->hwmem;
    vdec_init.pfrm_buf = (vdec_fbconfig *)pe_info.decoder_start;
    vdec_init.decode_mode = decv_config->sbm_mode;
    vdec_init.fmt_in.fourcc = decvp->codec_id;
    vdec_init.fmt_in.frame_rate = decv_config->frame_rate*10;
    vdec_init.fmt_in.pic_width = decv_config->width;
    vdec_init.fmt_in.pic_height = decv_config->height;
    vdec_init.fmt_in.pixel_aspect_x = decv_config->sample_aspect_ratio.num;
    vdec_init.fmt_in.pixel_aspect_y = decv_config->sample_aspect_ratio.den;
    vdec_init.fmt_out.frame_rate = decv_config->frame_rate*10;
    vdec_init.fmt_out.pic_width = decv_config->width;
    vdec_init.fmt_out.pic_height = decv_config->height;
    vdec_init.fmt_out.pixel_aspect_x = decv_config->sample_aspect_ratio.num;
    vdec_init.fmt_out.pixel_aspect_y = decv_config->sample_aspect_ratio.den;
    vdec_init.preview = decv_config->preview;
	vdec_init.dec_buf_addr = (UINT32)pe_info.decoder_start;
	vdec_init.dec_buf_size = pe_info.decv_buf_size;
	vdec_init.priv_buf_addr = (UINT32)pe_info.priv_buf_addr;
	vdec_init.priv_buf_size = pe_info.priv_buf_size;

    if((decvp->codec_id == h264) && (decv_config->extradata_size > 0) && (decv_config->extradata))
    {
        vdec_init.decoder_flag |= VDEC_FLAG_AVC1_FORMAT;
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_CMD_INIT, NULL, &vdec_init);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore init fail\n");
        goto fail;
    }

    pkt_sbm_idx = (VPKT_HDR_SBM_IDX<<16)|VPKT_DATA_SBM_IDX;
    dec_out_sbm_idx = -1; /* dont need frame out to user */
    ret = ali_video_decore_ioctl(decv_config, VDEC_CFG_VIDEO_SBM_BUF, (void *)pkt_sbm_idx, (void *)dec_out_sbm_idx);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore set video sbm fail\n");
        goto fail;
    }

    display_sbm_idx = -1;//DISP_IN_SBM_IDX;
    ret = ali_video_decore_ioctl(decv_config, VDEC_CFG_DISPLAY_SBM_BUF,
    		(void *)display_sbm_idx, (void *)display_sbm_idx);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore set display sbm fail\n");
        goto fail;
    }

    /* decode extradata */
    if((decv_config->extradata_size > 0) && decv_config->extradata)
    {
        ES_PLAYER_PRINTF("video decode extradata: 0x%x %d\n", decv_config->extradata, decv_config->extradata_size);
		osal_cache_flush(decv_config->extradata, decv_config->extradata_size);
        ret = ali_video_decore_ioctl(decv_config, VDEC_CMD_EXTRA_DADA,
        		decv_config->extradata, (void *)decv_config->extradata_size);
        if(ret != 0 )
        {
            ES_PLAYER_PRINTF("video decore extradata fail: %d\n",ret);
            if((ret == VDEC_FORBIDDEN_DECODE) || (ret == VDEC_NO_MEMORY))
            {
                goto fail;
            }
        }
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_CFG_DISPLAY_RECT,
                &(decv_config->src_rect), &(decv_config->dst_rect));
    if(ret != 0)
    {
    	ES_PLAYER_PRINTF("decore set display rect fail: %d\n",ret);
    }

#ifdef MP_DEBUG
if(g_dbg_mp_option & MP_DBG_DUMP_VIDEO)
{
    fs_remove("/mnt/uda1/video_dump.es");
    file_video_dump = fopen("/mnt/uda1/video_dump.es", "wb+");
    if(!file_video_dump)
        libc_printf("Error: open video_dump.es failed\n");
   	 else
        libc_printf("open video_dump.es success\n");
}
     g_dbg_vpkt_cnt=0;
     g_dbg_total_vpkt_size=0;
     g_dbg_first_vpkt_tick =0;
#endif

    return 0;
fail:

    if(decvp && (decvp->vpkt_data_fd > 0))
    {
		ret = ali_sbm_destroy(VPKT_DATA_SBM_IDX, decvp->vpkt_data_mode);
        if(ret<0)
            ali_trace(&ret);
    }

    if(decvp && (decvp->vpkt_hdr_fd > 0))
    {
		ret = ali_sbm_destroy(VPKT_HDR_SBM_IDX, decvp->vpkt_hdr_mode);
        if(ret<0)
            ali_trace(&ret);
    }

    if(decvp != NULL)
    {
        FREE(decvp);
        decvp = NULL;
    }

    return -1;

}

/** Close given decoder instance.
 *  \param decv_config The param which will be used to initialize decoder.
 */
void ali_video_decoder_close(ali_video_config *decv_config)
{
    int ret = 0;

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("video decoder has been released\n");
        return;
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_CMD_RELEASE, NULL, NULL);
    if(ret < 0)
    {
        libc_printf("video decoder release fail\n");
    }

    if((decvp->codec_id == h264) || (decvp->codec_id == mpg2))
    {
        vdec_stop(get_selected_decoder(), 0, 0);
        h264_decoder_select(0, 0);
    }

    if(decvp->codec_id == hevc)
    {
        vdec_stop(get_selected_decoder(), 0, 0);
        h264_decoder_select(0, 0);
    }

	ret = ali_sbm_destroy(VPKT_DATA_SBM_IDX, decvp->vpkt_data_mode);
    if(ret < 0)
        ali_trace(&ret);

	ret = ali_sbm_destroy(VPKT_HDR_SBM_IDX, decvp->vpkt_hdr_mode);
    if(ret < 0)
        ali_trace(&ret);

#ifdef MP_DEBUG
if(g_dbg_mp_option & MP_DBG_DUMP_VIDEO)
{
    if(file_video_dump!= NULL)
    {
		int ret;
       fflush(file_video_dump);

       ret =  fclose(file_video_dump);
       if(ret != 0)
           libc_printf("close video_dump.es failed, ret %d\n", ret);

        fs_sync("/mnt/uda1");
    }
}

if(g_dbg_mp_option & MP_DBG_VIDEO_ABR)
{
    libc_printf("Main2See: average video transfer rate(b/s): %ld\n", (UINT32)(((UINT64)g_dbg_total_vpkt_size<<3)*(UINT64)1000/(osal_get_tick() - g_dbg_first_vpkt_tick)));
}

#endif

    FREE(decvp);
    decvp = NULL;

}

/** Push next PES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_write_header(const struct av_packet *pkt_hdr)
{
    UINT32 write_size = 0;
 
    if((decvp == NULL) || (!pkt_hdr))
    {
        ES_PLAYER_PRINTF("write video packet header fail \n");
        return -1;
    }

    write_size = ali_sbm_write(VPKT_HDR_SBM_IDX, (const char *)pkt_hdr, sizeof(struct av_packet));

    if(write_size == sizeof(struct av_packet))
    {
#ifdef MP_DEBUG		
    	if(g_dbg_mp_option & MP_DBG_VPTS)
    	{
    		static uint64 last_valid_vpts;
	        if(pkt_hdr->pts == AV_NOPTS_VALUE)
	        {
	            libc_printf("no Vpts\n");
				if(pkt_hdr->pts != AV_NOPTS_VALUE)
					 last_valid_vpts = pkt_hdr->pts; //Just for clear warning
	        }
	        else
	        {
	            libc_printf("Vpts=%lld, diff=%d\n", pkt_hdr->pts, pkt_hdr->pts - last_valid_vpts);
	            last_valid_vpts = pkt_hdr->pts;
	        }
    	}
    	g_dbg_vpkt_cnt++;
#endif
        return 0;
    }

    return -1;
}

/** Push next fragment of PES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_write(const UINT8 *buf, UINT32 size)
{
    UINT32 write_size = 0;

    if((decvp == NULL) || (!buf))
    {
        ES_PLAYER_PRINTF("write video packet data fail \n");
        return -1;
    }

#ifdef MP_DEBUG
	if(g_dbg_mp_option & MP_DBG_DUMP_VIDEO)
	{
		if(file_video_dump)
		{
			fwrite(buf, 1, size, file_video_dump);
		}
	}

	if(g_dbg_mp_option & MP_DBG_VIDEO_ABR)
	{
	    g_dbg_total_vpkt_size += size;
	    if(g_dbg_first_vpkt_tick == 0)
	        g_dbg_first_vpkt_tick = osal_get_tick();
	}

#endif

    write_size = ali_sbm_write(VPKT_DATA_SBM_IDX, (const char *)buf, size);

    if(write_size == size)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}




/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_pause(ali_video_config *decv_config, BOOL pause_decode, BOOL pause_display)
{
    int ret = -1;

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("pause video decoder fail \n");
        return -1;
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_CMD_PAUSE_DECODE, (void *)pause_decode, (void *)pause_display);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore pause video fail\n");
        return -1;
    }

    return 0;
}


/** displaying video frames.
 *  \param step displaying
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_step(ali_video_config *decv_config)
{
    BOOL step_decode = 1;
    BOOL step_display = 1;
    int ret = -1;


    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("step video display fail \n");
        return -1;
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_STEP_DISPLAY, (void *)step_decode, (void *)step_display);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("display step video frame fail\n");
        return -1;
    }

    return 0;
}

/** displaying video frames.
 *  \param slow displaying
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_set_slow_ratio(ali_video_config *decv_config, int src_ratio)
{
	int ret = -1;

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("slow video display fail \n");
        return -1;
    }

    ret = ali_video_decore_ioctl(decv_config, VDEC_SET_DISPLAY_SLOW_RATIO, (void *)src_ratio, (void *)src_ratio);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("slow display video frame fail\n");
        return -1;
    }

    return 0;
}



/** Enable/disable STC sync.
 *  With sync enabled, decoder displays video frames according to pts/stc difference.
 *  With sync disabled, decoder displays video frames according to vsync.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_set_sync(ali_video_config *decv_config, BOOL enable)
{
    int ret = -1;
    int av_sync_mode = 0;
    int av_sync_unit = 1;

    if(decvp == NULL)
    {
        ES_PLAYER_PRINTF("set avsync fail \n");
        return -1;
    }

    av_sync_mode = enable?AV_SYNC_AUDIO:AV_SYNC_NONE;
    ret = ali_video_decore_ioctl(decv_config, VDEC_CFG_SYNC_MODE, &av_sync_mode, &av_sync_unit);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore set sync mode fail: %d\n",ret);
        return -1;
    }

    return 0;
}




/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_get_status(ali_video_config *decv_config, ali_video_decoder_status *status)
{
    struct vdec_decore_status decore_status;
    int ret = -1;


	if((decvp == NULL) || (status == NULL))
    {
        ES_PLAYER_PRINTF("get video decoder status fail \n");
        return -1;
    }

    MEMSET(&decore_status, 0, sizeof(decore_status));

    ret = ali_video_decore_ioctl(decv_config, VDEC_CMD_GET_STATUS, &decore_status, NULL);

    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore get status fail: %d\n",ret);
        return -1;
    }

    status->width = decore_status.pic_width;
    status->height = decore_status.pic_height;
    status->fps = decore_status.frame_rate;
    status->interlaced = decore_status.interlaced_frame;
    status->frames_decoded = decore_status.frames_decoded;
    status->frames_displayed = decore_status.frames_displayed;
    status->last_pts = decore_status.frame_last_pts;
    status->buffer_size = decore_status.buffer_size;
    status->buffer_used = decore_status.buffer_used;
    status->decode_error=decore_status.decode_error;
    return 0;
}

int ali_audio_decore_ioctl(ali_audio_config *deca_config, int cmd, void *param1, void *param2)
{
    int ret = -1;

    if(decap == NULL)
    {
        ES_PLAYER_PRINTF("decoder has not been opened\n");
        return -1;
    }

    switch(cmd)
    {
        case DECA_DECORE_INIT:
            ret = es_player_deca_ioctl(cmd, param1, param2);
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
        case DECA_DECORE_SET_BASE_TIME:
        case DECA_DECORE_PAUSE_DECODE:
        case DECA_DECORE_GET_PCM_TRD:
        case DECA_DECORE_GET_REMAIN_SAMPLE:
            ret = es_player_deca_ioctl(cmd, param1, param2);
            break;
        default:
            ret = -1;
            break;
    }

    if(ret != RET_SUCCESS)
    {
        ES_PLAYER_PRINTF("audio decore ioctl fail\n");
        ret = -1;
    }
    return ret;
}

/** Open audio decoder with given codec.
 *  \param codec_id  codec to be used

 *  \param deca_config The param which will be used to initialize decoder.

 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_open(ali_audio_config *deca_config, ali_sbm_info pe_info)

{
    struct ali_decore_buffer decore_buffer;
    struct audio_config audio_init;
    struct sbm_config sbm_info;
    unsigned int pcm_buf_addr = 0;
    unsigned int pcm_buf_size = 0;
    int ret = -1;
    int reset = 0;

    MEMSET(&decore_buffer, 0, sizeof(struct ali_decore_buffer));
    MEMSET(&sbm_info, 0, sizeof(struct sbm_config));
    MEMSET(&audio_init, 0, sizeof(struct audio_config));
     if((decap != NULL) || (deca_config == NULL))

    {
        ES_PLAYER_PRINTF("decoder has been inited\n");

        return ret;
    }
    if(ali_decore_alloc_buffer(pe_info, &decore_buffer) < 0)
    {
        return -1;
    }

    decap = MALLOC(sizeof(struct ali_deca_private));

    if(decap == NULL)
    {
        ES_PLAYER_PRINTF("malloc decoder private fail\n");
        return -1;
    }

    MEMSET(decap, 0, sizeof(struct ali_deca_private));

    decap->codec_id = deca_config->codec_id;

    //snd_set_volume((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SUB_OUT, 40);

    pcm_buf_size = decore_buffer.pcm_buf_size;
    pcm_buf_addr = decore_buffer.pcm_buf_addr;

    sbm_info.buffer_size = decore_buffer.apkt_data_size;
    sbm_info.buffer_addr = decore_buffer.apkt_data_addr;
    sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = 128*1024;
    sbm_info.wrap_mode = SBM_MODE_PACKET;
    sbm_info.lock_mode = SBM_SPIN_LOCK;

    decap->apkt_data_mode = SBM_MODE_PACKET;

    ret = ali_sbm_create(APKT_DATA_SBM_IDX, sbm_info);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore create sbm fail\n");
        goto fail;
    }

    decap->apkt_data_fd = 1;

    sbm_info.buffer_size = decore_buffer.apkt_hdr_size;
    sbm_info.buffer_addr = decore_buffer.apkt_hdr_addr;
    sbm_info.reserve_size = 10*sizeof(struct av_packet);
    sbm_info.wrap_mode = SBM_MODE_NORMAL;
    sbm_info.lock_mode = SBM_SPIN_LOCK;

    decap->apkt_hdr_mode = SBM_MODE_NORMAL;

    ret = ali_sbm_create(APKT_HDR_SBM_IDX, sbm_info);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("decore create sbm fail\n");
        goto fail;
    }

    decap->apkt_hdr_fd = 1;

	ret = ali_sbm_reset(APKT_DATA_SBM_IDX);
    if(ret<0)
        ali_trace(&ret);

	ret = ali_sbm_reset(APKT_HDR_SBM_IDX);
    if(ret<0)
        ali_trace(&ret);

    audio_init.decode_mode = 1;
    audio_init.sync_mode = deca_config->av_sync_mode;
    audio_init.sync_unit = deca_config->av_sync_unit;
    audio_init.deca_input_sbm = (APKT_HDR_SBM_IDX<<16)|APKT_DATA_SBM_IDX;
    audio_init.deca_output_sbm = -1;
    audio_init.snd_input_sbm = -1;
    audio_init.codec_id = decap->codec_id;
    audio_init.bits_per_coded_sample = deca_config->bits_per_coded_sample;
    audio_init.sample_rate = deca_config->sample_rate;
    audio_init.channels = deca_config->channels;
    audio_init.bit_rate = deca_config->bit_rate;
    audio_init.block_align = deca_config->block_align;
    audio_init.pcm_buf_size = pcm_buf_size;
    audio_init.pcm_buf = pcm_buf_addr;
	if(deca_config->extradata != NULL )
	{
		struct av_packet pkt_info;
		pkt_info.data = deca_config->extradata;
		pkt_info.size = deca_config->extradata_size;

		ali_audio_decoder_write(deca_config->extradata, deca_config->extradata_size);
		ali_audio_decoder_write_header((void*)&pkt_info)	;
	   ES_PLAYER_PRINTF("Set extra data, size = %d\n", deca_config->extradata_size);
	}
	else
		audio_init.extra_data[0] = 0xFF;


    if(!audio_init.sample_rate || !audio_init.channels)
    {
	   ES_PLAYER_PRINTF("sample rate/channels is invaild, sr/ch = %d/%d\n", audio_init.sample_rate, audio_init.channels);
	   goto fail;
    }
	//pcm buffer,pcm size
	ES_PLAYER_PRINTF("Open: sr/ch = %d/%d, id %x, block size %d\n", audio_init.sample_rate, audio_init.channels, audio_init.codec_id, audio_init.block_align);
    ret = ali_audio_decore_ioctl(deca_config, DECA_DECORE_INIT, &audio_init, &reset);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("deca decore init fail\n");
        goto fail;
    }

#if 0
    /* decode extradata */
    if((deca_config->extradata_size > 0) && deca_config->extradata)
    {
        ES_PLAYER_PRINTF("decode extradata: 0x%x %d\n", deca_config->extradata, deca_config->extradata_size);
        ret = ali_video_decore_ioctl(deca_config, VDEC_CMD_EXTRA_DADA,
        		decv_config->extradata, (void *)decv_config->extradata_size);
        if(ret != 0 )
        {
            ES_PLAYER_PRINTF("decore extradata fail: %d\n",ret);
            if((ret == VDEC_FORBIDDEN_DECODE) || (ret == VDEC_NO_MEMORY))
            {
                goto fail;
            }
        }
    }
#endif

#ifdef MP_DEBUG
if(g_dbg_mp_option & MP_DBG_DUMP_AUDIO)
{
    fs_remove("/mnt/uda1/audio_dump.es");
    file_audio_dump = fopen("/mnt/uda1/audio_dump.es", "wb+");
    if(!file_audio_dump)
        libc_printf("Error: open audio_dump.es failed\n");
   	 else
        libc_printf("open audio_dump.es success\n");
}

     g_dbg_apkt_cnt=0;
#endif
    return 0;

fail:

    if(decap && (decap->apkt_data_fd > 0))
    {
		ret = ali_sbm_destroy(APKT_DATA_SBM_IDX, decap->apkt_data_mode);
        if(ret<0)
            ali_trace(&ret);
    }

    if(decap && (decap->apkt_hdr_fd > 0))
    {
		ret = ali_sbm_destroy(APKT_HDR_SBM_IDX, decap->apkt_hdr_mode);
        if(ret<0)
            ali_trace(&ret);
    }

    if(decap != NULL)
    {
        FREE(decap);
        decap = NULL;
    }

    return -1;

}

/** Close given decoder instance.
 *  \param deca_config The param which will be used to initialize decoder.
 */
void ali_audio_decoder_close(ali_audio_config *deca_config)
{
    int ret = 0;

    if(decap == NULL)
    {
        ES_PLAYER_PRINTF("audio decoder has been released\n");
        return;
    }

    ret = ali_audio_decore_ioctl(deca_config, DECA_DECORE_RLS, NULL, NULL);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("audio decoder release fail\n");
        return ;
    }

	ret = ali_sbm_destroy(APKT_DATA_SBM_IDX, decap->apkt_data_mode);
    if(ret<0)
        ali_trace(&ret);

	ret = ali_sbm_destroy(APKT_HDR_SBM_IDX, decap->apkt_hdr_mode);
    if(ret<0)
        ali_trace(&ret);


    FREE(decap);

    decap = NULL;

#ifdef MP_DEBUG
if(g_dbg_mp_option & MP_DBG_DUMP_AUDIO)
{
    if(file_audio_dump!= NULL)
    {
		int ret;
       fflush(file_audio_dump);

       ret =  fclose(file_audio_dump);
       if(ret != 0)
           libc_printf("close audio_dump.es failed, ret %d\n", ret);

        fs_sync("/mnt/uda1");
    }
}
#endif
    return;
}




/** Pause decoding audio frames / playing audio samples.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_pause(ali_audio_config *deca_config, BOOL pause_decode, BOOL pause_display)

{
    int ret = -1;

    if(decap == NULL)
    {
        ES_PLAYER_PRINTF("pause audio decoder fail \n");
        return -1;
    }

    ret = ali_audio_decore_ioctl(deca_config, DECA_DECORE_PAUSE_DECODE, &pause_decode, &pause_display);
    if(ret < 0)
    {
        ES_PLAYER_PRINTF("deca decore pause fail\n");
        return -1;
    }

    return 0;
}


/** Push next PES packet header to ali audio decoder. ES mode need not write this header.
 *  \param p_pkt_header, contains pts, frame size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_write_header(const struct av_packet *p_pkt_header)
{
    unsigned int write_size = 0;    
    if((decap == NULL) ||(!p_pkt_header))
    {
        ES_PLAYER_PRINTF("write audio packet header fail \n");
        return -1;
    }


    write_size = ali_sbm_write(APKT_HDR_SBM_IDX, (const char *)p_pkt_header, sizeof(struct av_packet));

    if(write_size != sizeof(struct av_packet))
    {
        return -1;
    }

 #ifdef MP_DEBUG
    if(g_dbg_mp_option & MP_DBG_APTS)
    {   
    	static uint64 last_valid_apts;
    	if(p_pkt_header->pts == AV_NOPTS_VALUE)
        {
            libc_printf("no Apts\n");
			if(p_pkt_header->pts != AV_NOPTS_VALUE)
					 last_valid_apts = p_pkt_header->pts; //Just for clear warning
        }
        else
        {
            libc_printf("Apts=%lld, diff=%d\n", p_pkt_header->pts, p_pkt_header->pts - last_valid_apts);
            last_valid_apts = p_pkt_header->pts;
        }
    }
    g_dbg_apkt_cnt++;
#endif
    return 0;
}

/** Push fragment of PES packet to audio decoder, or ES data (see \e ali_audio_decoder_set_pes_mode).
 *  \param buf  buffer with fragment of one PES packet, or some ES data.
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_write(const UINT8 *buf, UINT32 size)
{
    unsigned int write_size = 0;

    if((decap == NULL)||(!buf))
    {
        ES_PLAYER_PRINTF("write audio packet data fail \n");
        return -1;
    }
#ifdef MP_DEBUG
	if(g_dbg_mp_option & MP_DBG_DUMP_AUDIO)
	{
		if(file_audio_dump)
		{
			fwrite(buf, 1, size, file_audio_dump);
		}
	}
#endif

    write_size = ali_sbm_write(APKT_DATA_SBM_IDX, (const char *)buf, size);
    if(write_size != size)
    {
        return -1;
    }

    return 0;
}


