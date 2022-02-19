/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pe.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIBMP_PE_H__
#define __LIBMP_PE_H__

#include <api/libimagedec/imagedec.h>
#include <api/libfs2/types.h>
//Current supporte media stream type in decoders
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>
#include <osal/osal.h>
#include <hld/decv/decv.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define UNKNOWN_STREAM                     0
#define MPEG_12_FILE_STREAM             1
#define AVI_STREAM                         2
#define MKV_STREAM                         3
#define RMVB_STREAM                     4

//define the callback function for APP
#define MP_FS_MOUNT                    (1 << 0)
#define MP_FS_UNMOUNT                (1 << 1)
#define MP_MUSIC_PLAYBACK_END        (1 << 2)
#define MP_IMAGE_PLAYBACK_END        (1 << 3)
#define MP_IMAGE_DECODE_PROCESS     (1 << 4)
#define MP_FS_IDE_MOUNT_NAME        (1 << 5)
#define MP_FS_USB_MOUNT_NAME        (1 << 6)
#define MP_VIDEO_PLAYBACK_END       (1 << 7)
#define MP_IMAGE_DECODER_ERROR        (1 << 8)
#define MP_VIDEO_BUILD_IDX_TBL      (1 << 9)
#define MP_VIDEO_PARSE_END          (1 << 10)
#define MP_VIDEO_DECODER_ERROR        (1 << 11)
#define MP_DRM_NOT_AUTHORIZED       (1 << 12)
#define MP_DRM_RENTAL_EXPIRED       (1 << 13)
#define MP_DRM_VIEW_RENTAL          (1 << 14)
#define MP_DRM_VERSION_ERR          (1 << 15)
#define MP_DRM_ICT_ENABLE           (1 << 16)
#define MP_BUFFER_PERCENT           (1 << 17)
#define MP_PREVIEW_VIDEO_PLAYBACK_END (1 <<18)
#define MP_MUSIC_PLAYBACK_BEGIN		(1 << 19)

// param in mp_callback_func(MP_VIDEO_BUILD_IDX_TBL, param)
#define MP_VIDEO_BUILD_IDX_TBL_BEGIN    0
#define MP_VIDEO_BUILD_IDX_TBL_END      1

/* param in mp_callback_func(MP_VIDEO_DECODER_ERROR, param),
   and in mp_callback_func(MP_VIDEO_PARSE_END, param);  */
#define MP_DERR_OK                      0
#define MP_DERR_UNSUPPORTED_VIDEO_CODEC (1<<0)
#define MP_DERR_UNSUPPORTED_AUDIO_CODEC (1<<1)
#define MP_DERR_UNSUPPORTED_RESOLUTION  (1<<2)
#define MP_DERR_UNSUPPORTED_FRAME_RATE  (1<<3)
#define MP_DERR_TOO_BIG_INTERLEAVE      (1<<4)
#define MP_DERR_MP4_DRM_AUTH            (1<<5)
#define MP_DERR_ENGINE_TASK             (1<<6)
#define MP_DERR_UNKNOWN                 0xffffffff

#define PE_CACHE_NUM    2

#define PE_CACHE_CLOSED     0
#define PE_CACHE_OPENED     1
#define PE_CACHE_CLOSING    2                               //!< Pe cache is closing but not closed.

#define PE_FILE_READING     0
#define PE_FILE_FAILED      1
#define PE_FILE_EOF         2

#define PE_SEEK_TYPE_DISABLE 0                              //!< Indicating current pe cache can not support seek operation.
#define PE_SEEK_TYPE_BYTE    1                              //!< Indicating current pe cache support bye seek operation.
#define PE_SEEK_TYPE_TIME    2                              //!< Indicating current pe cache support time seek operation.
#define PE_PREVIOUS_BUFFER_SIZE 10240 //10K

#define PE_NET_FILE_TYPE_URL 0
#define PE_ALI_SPECIAL_STREAM 1

#define PE_NET_FILE_TYPE_MAX 2

#define MUSIC_PE_CACHE_SIZE_MAX    (512*1024)

#define FD_PATH_HEADER "/flshchk/"   //path header of flash data.
/*use for network optimization*/
#define MP_SET_NETWORK_OTHER_INFO (0x01)					//!< Get some information when mp3 network is playing



/*end of the network command*/

typedef void(*mp_callback_func)(unsigned long type, unsigned long param);
typedef void(*ali2dto3d_callback_func)(unsigned char *pic_out_buf, unsigned char *pic_in_buf,
                    int pic_width, int pic_height, int output_format,
                    int user_depth, int user_shift, int user_vip);
#ifdef PE_CALLBACK_TO_APP_ENABLE
typedef void(*pccache_callback_func)(UINT8 type, UINT8 param);
#endif
//define function point for pe_cache use
typedef  FILE * (* pe_cache_open_fp)(const char *,
                                                                unsigned char* ,
                                                                unsigned int,
                                                                unsigned int );//filename, buf, cache length, offset
typedef size_t (* pe_cache_read_fp)(void *, size_t, size_t, FILE *);
typedef int(* pe_cache_seek_fp)(FILE *, off_t, int);
typedef int (* pe_cache_feof_fp)(FILE*);
typedef off_t (* pe_cache_ftell_fp)(FILE *);
typedef int (* pe_cache_fclose_fp)(FILE *);

typedef struct pe_cache
{
    UINT16 internal_cache;   //:1;
    FILE *fp;
    UINT32 block_size;

#ifdef ENABLE_PE_FLAG
    UINT32 req_size;
    ID flag;
#endif
    //add function point  for pe_cache callback
    pe_cache_read_fp file_read;
    pe_cache_seek_fp file_seek;
    pe_cache_feof_fp file_eof;
    pe_cache_ftell_fp file_tell;
    pe_cache_fclose_fp file_close;
#ifdef LOG_PE_CACHE
    UINT32 read_delay;
    UINT32 read_count;
    UINT32 read_tick;
    UINT32 read_size;
#endif
    off_t   file_size;
    INT32   file_duration;
	int     netfile;
    INT32   seek_type;
    int     open_success;
	UINT8	sgdma_id;
}pe_cache, *p_pe_cache;

typedef struct pe_cache_ex
{
    UINT8    status;
    UINT8    sub_status;
    ID        mutex;
    UINT8 *    cache_buff;
    UINT32    cache_size;
    UINT32    data_len;
    UINT32    rd_pos;
    UINT32    wr_pos;
    off_t        file_offset;
}pe_cache_ex;

#ifdef DUAL_ENABLE
#define PE_CACHE_CMD_NULL  0
#define PE_CACHE_CMD_OPEN  1
#define PE_CACHE_CMD_CLOSE 2
#define PE_CACHE_CMD_SEEK  3
#define PE_CACHE_CMD_FILE_SIZE      4
#define PE_CACHE_CMD_FILE_DURATION  5
#define PE_CACHE_CMD_FULL_USE       6
#define PE_CACHE_CMD_GET_SEEK_TYPE  7

#define PE_CACHE_CMD_STATUS_NEW 1
#define PE_CACHE_CMD_STATUS_IMPLEMENTED 2

typedef struct
{
    UINT32 status     :2;
    UINT32 type       :6;
    UINT32 reserved   :24;
    UINT32 param[4];
}pe_cache_cmd;
#endif

struct pe_feature_cfg
{
    UINT32 chip;

    UINT32 using_adpcm;
    UINT32 has_dview;  // has divew same as vpo of M3327C
    UINT32 has_pip;        // has pip same as vpo of M3329E
    UINT32 has_sca;        // has dview scaler as vpo of M3101C
};

struct pe_image_cfg
{

    // dst frame info
    UINT32 frm_y_addr;
    UINT32 frm_y_size;
    UINT32 frm_c_addr;
    UINT32 frm_c_size;
    UINT32 frm2_y_addr;
    UINT32 frm2_y_size;
    UINT32 frm2_c_addr;
    UINT32 frm2_c_size;
    UINT32 frm_mb_type;
    UINT32 frm3_y_addr;
    UINT32 frm3_y_size;
    UINT32 frm3_c_addr;
    UINT32 frm3_c_size;
    UINT32 frm4_y_addr;
    UINT32 frm4_y_size;
    UINT32 frm4_c_addr;
    UINT32 frm4_c_size;

    // buf for dec internal usage
    UINT8 *decoder_buf;
    UINT32 decoder_buf_len;

    //callback function for application
    mp_callback_func mp_cb;
    ali2dto3d_callback_func ali_pic_2dto3d;
};

struct img_callback
{
    mp_callback_func mp_cb;
};

struct pe_music_cfg
{
    //buff
    UINT32 pcm_out_buff;
    UINT32 pcm_out_buff_size;
    UINT32 processed_pcm_buff;
    UINT32 processed_pcm_buff_size;

    //callback function for application
    mp_callback_func mp_cb;
    UINT32 pe_cache_buf_start_addr;
    UINT32 pe_cache_buf_len;
    UINT32 reserved[8];    //// use in the future, should init to zero
};

struct pe_video_cfg
{
    mp_callback_func mp_cb;
    UINT32 decoder_buf;
    UINT32 decoder_buf_len;
    UINT32 mm_vbv_len;
    UINT32 reserved;    // struct pe_video_cfg_extra *. set to 0 if no pe_video_cfg_extra
    UINT32 set_sbm_size; //!<set sbm buffer size , set 0 default
    UINT32 set_start_play_time_ms; //!<set current file start playing time.
    BOOL   disable_seek;
    BOOL   new_retry_mechanism;
    BOOL   dynamic_resolution;      //!< One flag used to indicate wehter support fluent resolution switch or not.
    BOOL   show_first_before_net_bufferring;
};

struct pe_video_cfg_extra
{
    UINT32 frame_buf_base;
    UINT32 frame_buf_len;
    UINT16 max_width_support, max_height_support;   // if 0, determined by frame_buf_len
    UINT32 reserved[8];    // use in the future, should set to 0
};

typedef struct
{
    struct pe_image_cfg image;
    struct pe_music_cfg music;
    struct pe_video_cfg video;
}pe_config, *p_pe_config;

typedef struct
{
    pe_cache_open_fp file_open;
    pe_cache_read_fp file_read;
    pe_cache_seek_fp file_seek;
    pe_cache_feof_fp file_eof;
    pe_cache_ftell_fp file_tell;
    pe_cache_fclose_fp file_close;
}pe_cache_cb_fp;

typedef struct
{
    int width;
    int height;
    char *buffer;
}st_image_buffer;

struct argb_color
{
    int b;
    int g;
    int r;
    int alpha;
};

int pe_init(struct pe_music_cfg *pe_music_cfg, struct pe_image_cfg *pe_image_cfg, struct pe_video_cfg *pe_video_cfg);
int pe_cleanup();


//============================================================================
// image structure and functions

struct image_info
{
    unsigned long    fsize;
    unsigned long    width;
    unsigned long    height;
    unsigned long    bbp;
};


struct image_display_t
{
    unsigned char    decode_mode;
    unsigned char    show_mode;
    unsigned char    vpo_mode;
    unsigned char    rotate;

    //rect for source
    unsigned short    src_left;
    unsigned short    src_top;
    unsigned short    src_width;
    unsigned short    src_height;

    //rect for display
    unsigned short    dest_left;
    unsigned short    dest_top;
    unsigned short    dest_width;
    unsigned short    dest_height;

    struct image_slideshow_effect *effect;

    UINT8    *y_addr;
    UINT32    y_len;
    UINT8     *c_addr;
    UINT32    c_len;

    UINT32    width;
    UINT32    height;

    UINT8    sample_format;
    //callback function
    mp_callback_func mp_cb;
};


//slideshow effect
struct image_slideshow_effect
{

    enum IMAGE_SHOW_MODE mode;
    union
    {
        imagedec_show_shutters     shuttles_param;
        imagedec_show_brush     brush_param;
        imagedec_show_slide     slide_param;
        imagedec_show_show_random
                                random_param;
        imagedec_show_fade         fade_param;
    }mode_param;

};


int image_abort();
int image_rotate(unsigned char rotate_angle);
int image_decode(char *file, int mode, int left, int top, int width, int height, unsigned char rotate);
int image_decode_ex(char *file, int mode, int left, int top,
                                    int width, int height,
                                    unsigned char rotate, struct image_slideshow_effect *effect);
int image_display(char * file, struct image_display_t * pdisplay);
int image_get_info(char *filename, struct image_info *info );
void image_restore_vpo_rect();
int image_zoom(struct rect *dst_rect, struct rect *src_rect);
int image_cleanup(void);


//============================================================================
// music structure and functions

typedef enum
{
    mp3,
    bye1,
    aac,
    rm
}music_type;

/*! @enum
@brief
   Define the enum type for the interface : DWORD mpg_cmd_set_speed(UINT32 speed)
   User can set the video play speed based on the enum type definition
*/
typedef enum
{
    PE_PLAY_SPEED_BEGIN = 1,

    //!< normal play speed
    PE_SPEED_NORMAL = PE_PLAY_SPEED_BEGIN,

    //!< fast forward
    PE_SPEED_FASTFORWARD_2,    	 //!< 2 times fast forward.
    PE_SPEED_FASTFORWARD_4,      //!< 4 times fast forward.
    PE_SPEED_FASTFORWARD_8,      //!< 8 times fast forward.
    PE_SPEED_FASTFORWARD_16,     //!< 16 times fast forward.
    PE_SPEED_FASTFORWARD_24,     //!< 24 times fast forward.

    //!< fast rewind
    PE_SPEED_FASTREWIND_2,       //!< 2 times fast rewind.
    PE_SPEED_FASTREWIND_4,       //!< 4 times fast rewind.
    PE_SPEED_FASTREWIND_8,       //!< 8 times fast rewind.
    PE_SPEED_FASTREWIND_16,      //!< 16 times fast rewind.
    PE_SPEED_FASTREWIND_24,      //!< 24 times fast rewind.

    //!< slow forward
    PE_SPEED_SLOWFORWARD_2,      //!< 2 times slow forward.
    PE_SPEED_SLOWFORWARD_4,      //!< 4 times slow forward.
    PE_SPEED_SLOWFORWARD_8,      //!< 8 times slow forward

    //!< not support slow rewind
    PE_SPEED_SLOWREWIND_2,       //!< 2 times slow rewind.
    PE_SPEED_SLOWREWIND_4,       //!< 4 times slow rewind.
    PE_SPEED_SLOWREWIND_8,       //!< 8 times slow rewind.
    PE_PLAY_SPEED_END
} AF_PE_PLAY_SPEED;

typedef struct
{
    char title[30];         /*标题*/
    char artist[30];        /*作者*/
    char album[30];         /*专集*/
    char year[4];           /*出品年代*/
    char comment[30];       /*备注*/
    char genre;             /*类型*/
    char track;
    unsigned long time;
    unsigned long file_length;
    unsigned int bitrate;
    unsigned int samplerate;
    unsigned int channel_number;
    unsigned long detail_flag;
}music_info;


typedef struct _DECODER_INFO
{
    unsigned long    bit_rate;                //bps
    unsigned long    sample_rate;            //KHz
    unsigned long    channel_mode;
}decoder_info;

int music_play(char *filename);
void music_seek(int time);
DWORD music_stop(void);
int music_pause(void);
DWORD music_get_time(void);
void music_set_eq(int on, float preamp, float *bands);
int music_get_song_info(char * filename, music_info *music_info);
int music_get_decoder_info(char * filename, decoder_info *decoder_info);

void music_engine_set_disable_seek(UINT8 disable);

/*!
@fn     int music_set_other_info(UINT32 cmd, UINT32 param1)
@brief  Music sets some other information.
@param[in]  cmd   		The IO cmd
@param[in]  param1    	The param match the cmd
@return    RET_CODE
@retval    0          Operated successfully.
@retval    non-zero   Failed to operate.
*/
int music_set_other_info(UINT32 cmd, UINT32 param1);

#define PE_CACHE_AUTO_THRESHOLD     0xFFFFFFFF
int pe_cache_init();
int pe_cache_open(char *filename, UINT8 *cache_buff, UINT32 cache_size, UINT32 block_size);
UINT32 pe_cache_close(int cache_id);
UINT32 pe_cache_read(int cache_id, UINT8 *buff, UINT32 size, UINT32 threshold);
UINT32 pe_cache_get(int cache_id, UINT8 **ptr, UINT32 size, UINT32 threshold);
void pe_cache_invalidate(int cache_id, UINT8 *ptr, UINT32 size);
int pe_cache_seek(int cache_id, off_t offset, int where);
int pe_cache_check_eof(int cache_id);
UINT32 pe_cache_get_data_len(int cache_id);
off_t pe_cache_tell(int cache_id);
off_t pe_cache_filesize(int cache_id);
INT32 pe_cache_file_duration(int cache_id);
void pe_cache_full_use(int cache_id);
off_t pe_cache_file_offset(int cache_id);


int video_set_output(enum VDEC_OUTPUT_MODE eoutmode, struct vdec_pipinfo *pinitinfo);
void video_set_disable_seek(UINT8 disable);
int video_dec_file(char *path, BOOL preview);

#ifdef SEE_CPU
int see_pe_cache_release(void);
int see_pe_cache_init(pe_cache_ex *info, pe_cache_cmd *buf, ID mutex);
#endif

#ifdef __cplusplus
}
#endif
#endif// __LIBMP_PE_H__



