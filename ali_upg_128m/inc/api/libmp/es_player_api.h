/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: es_player_api.h
*
*    Description: This file contains APIs for es player.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __ES_PLAYER_API_H_
#define __ES_PLAYER_API_H_
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)     (sizeof(a) / sizeof((a)[0]))
#endif
#define VPKT_HDR_SBM_IDX  0
#define VPKT_DATA_SBM_IDX 1
#define DISP_IN_SBM_IDX   2
#define APKT_HDR_SBM_IDX  3
#define APKT_DATA_SBM_IDX 4
#define DECA_OUT_SBM_IDX  5
#define SND_IN_SBM_IDX    6

//#define AV_NOPTS_VALUE    INT64_C(0x8000000000000000)

enum
{
    AV_SYNC_NONE,
    AV_SYNC_AUDIO,
    AV_SYNC_VIDEO,
    AV_SYNC_EXTERNAL, /* synchronize to an external clock */
};

typedef struct ali_video_decoder_status_s
{
    UINT32 buffer_size; /// size of internal buffer used for PES/ES gathering
    UINT32 buffer_used; /// used space in internal buffer, app will use it to control speed of pushing data
    INT64 last_pts;     /// 90kHz, 33 bits, -1 if unknown
    UINT32 frames_decoded;
    UINT32 frames_displayed;
    INT32 width;      /// -1 if unknown
    INT32 height;     /// -1 if unknown
    INT32 fps;        /// -1 if unknown, units can be decided by ALi
    INT8  interlaced; /// -1 if unknown
    UINT32 decode_error;
}ali_video_decoder_status;

typedef struct ali_sbm_info_s
{
    void *decoder_start;  //frame buffer
    UINT32 decv_buf_size;

    void *mem_start; //sbm buffer
    UINT32 mem_size;

	void *priv_buf_addr;
	UINT32 priv_buf_size;
	void  *pdec_init_par;
}ali_sbm_info;

typedef struct ali_video_config_s
{
    INT32 codec_tag;

    INT32 frame_rate;

    /**
     * some codecs need / can use extradata like Huffman tables.
     */
    UINT8 *extradata;
    INT32 extradata_size;

    /**
     * picture width / height.
     */
    INT32 width, height;

    /**
     * sample aspect ratio (0 if unknown)
     * That is the width of a pixel divided by the height of the pixel.
     */
    struct av_rational sample_aspect_ratio;

    INT32 preview;
    struct av_rect src_rect;
    struct av_rect dst_rect;
    INT32 sbm_mode;
}ali_video_config;

typedef struct ali_audio_config_s
{
    INT32 codec_id;

     /* audio only */
    INT32 sample_rate; ///< samples per second
    INT32 channels;    ///< number of audio channels

    /**
    * bits per sample/pixel from the demuxer (needed for huffyuv).
    */
    INT32 bits_per_coded_sample;

    /**
     * the average bitrate
     */
    INT32 bit_rate;		
    INT32 av_sync_mode;
    INT32 av_sync_unit;
	UINT8 *extradata;
    UINT32 extradata_size;
    /**
     * number of bytes per packet if constant and known or 0
     * Used by some WAV based audio codecs.
     */
    INT32 block_align;
}ali_audio_config;

/** Open video decoder with given codec.
 *  \param decv_config The param which will be used to initialize decoder.
 *  \param pe_info   share memory addr and decoder addr
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_open(ali_video_config *decv_config, ali_sbm_info pe_info);

/** Close given decoder instance.
 *  \param decv_config  The param which will be used to initialize decoder.
 */
void ali_video_decoder_close(ali_video_config *decv_config);

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_get_status(ali_video_config *decv_config, ali_video_decoder_status *status);

/** Enable/disable STC sync.
 *  With sync enabled, decoder displays video frames according to pts/stc difference.
 *  With sync disabled, decoder displays video frames according to vsync.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_set_sync(ali_video_config *decv_config, BOOL enable);

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_pause(ali_video_config *decv_config, BOOL pause_decode, BOOL pause_display);
int ali_video_decoder_step(ali_video_config *decv_config);

int ali_video_decoder_set_slow_ratio(ali_video_config *decv_config, int src_ratio);



int ali_video_decore_ioctl(ali_video_config *decv_config, int cmd, void *param1, void *param2);
int ali_audio_decore_ioctl(ali_audio_config *deca_config,int cmd, void *param1, void *param2);

/** Push next ES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_write_header(const struct av_packet *pkt_hdr);

/** Push next fragment of ES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_video_decoder_write(const UINT8* buf, UINT32 size);

/** Open audio decoder with given codec.
 *  \param deca_config The param which will be used to initialize decoder.
 *  \param pe_info   share memory addr and decoder addr.
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_open(ali_audio_config *deca_config, ali_sbm_info pe_info);

/** Close given decoder instance.
 *  \param deca_config The param which will be used to initialize decoder.
 */
void ali_audio_decoder_close(ali_audio_config *deca_config);

/** Pause decoding audio frames / playing audio samples.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_pause(ali_audio_config *deca_config, BOOL pause_decode, BOOL pause_display);

/** Push next ES packet header to ali audio decoder.
 *  \param p_pkt_header, contains pts, frame size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_write_header(const struct av_packet* p_pkt_header);

/** Push fragment of ES packet to audio decoder.
 *  \param buf  buffer with fragment of one ES packet.
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_audio_decoder_write(const UINT8* buf, UINT32 size);

#ifdef __cplusplus
}
#endif
#endif
