#ifndef _T2DECODER_H
#define _T2DECODER_H

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <api/libmp/media_player_api.h>

#define MAX_FRAME_BUF_NO    6
#define MAX_OTHER_BUF_NO    10


#define INVALID_TIME 0xffffffff
#define INVALID_STREAM_ID 0xffff

#ifndef SEEK_SET
#define SEEK_SET    0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif
#ifndef SEEK_END
#define SEEK_END    2
#endif

#define AUDIO_WAITRETRY                    0
#define AUDIO_SUCCESS                    1
#define AUDIO_NOTSUPPORTED                2

//normal: normal play mode
//skip_b: only skip b frame in FF mode
//skip_b_p: skip both b and p in FF mode
//step: step or pause mode
//stop: stop mode
#define VIDEO_PLAY_MODE_NORMAL            0
#define VIDEO_PLAY_MODE_SKIP_B            1
#define VIDEO_PLAY_MODE_SKIP_B_P        2
#define VIDEO_PLAY_MODE_STEP            3
#define VIDEO_PLAY_MODE_STOP             4
#define VIDEO_FLUSH_BUFFER              5
#define VIDEO_PLAY_MODE_RESUME_STOP     6

//none: no any operation to audio which means audio keeps previous state
//on: decoder audio on
//off: decoder audio off
#define AUDIO_PLAY_MODE_NONE            0
#define AUDIO_PLAY_MODE_ON                1
#define AUDIO_PLAY_MODE_OFF                2

#define FORWARD_PLAY            1
#define BACKWARD_PLAY           2

#define DERR_OK                      0
#define DERR_MP4_NO_INDEX            18

typedef struct
{
    BOOL renew; //this indictates that any field of "t2DecoderControl" is changed
    BYTE video_playmode; //Nomal, skip_b, skip_b_p
    BYTE audio_playmode; //mute, no_mute
    int scr_ratio; //0: normal speed: 1: 2X slow 2: 4X slow play 3: 8X slow play 4: 16X slow play 5: 32X slow play
    WORD video_stream_id; //video stream ID to be decoded
    WORD audio_stream_id; //audiostream ID to be decoded
    WORD subp_stream_id; //SP stream ID to be decoded
    BYTE stream_type;
    BYTE play_dir; //Play direction(forward or backward)
    DWORD search_time;

    BOOL  subt_onoff;
    WORD  prog_id;
    WORD  subt_lang_id;

}t2decoder_control;

typedef struct
{
    DWORD fbstride;
    DWORD fbmax_height;
    BYTE *frm_buf;  //used only for SW_DECODE
    BYTE *disp_buf; //In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
}t2frame_buffer;

typedef struct
{
    BYTE *buf;
    DWORD size;
}t2dec_other_buf;

typedef struct
{
    UINT32 fb_y[MAX_FRAME_BUF_NO];
    UINT32 fb_c[MAX_FRAME_BUF_NO];
    UINT32 dv_y[MAX_FRAME_BUF_NO];
    UINT32 dv_c[MAX_FRAME_BUF_NO];
    UINT32 fb_max_stride;
    UINT32 fb_max_height;
    UINT32 dv_max_stride;
    UINT32 dv_max_height;
    INT32 fb_num;
    UINT32 neighbor_mem;
    UINT32 colocate_mem;
    UINT32 cmdq_base;
    UINT32 cmdq_len;
    UINT32 vbv_start;
    UINT32 vbv_len;
}hwmem_config;


typedef struct
{
#ifndef WIN32
    enum snd_dup_channel audio_channel;
#endif
    t2frame_buffer *frame_buffer;
    t2dec_other_buf *other_buffer;	
    BOOL preview;
    BOOL is_network_play; 
    DWORD (*decoder_read_data)(BYTE *buf, DWORD size);
    BOOL (*decoder_seek_data)(INT64 offset, DWORD origin);
    INT64 (*decoder_tell_pos)(void);
    BOOL (*get_ctrl_cmd)(t2decoder_control *decoder_ctrl);
    BOOL (*decoder_stop_play)(void);
    void(*mp_callback_func)(unsigned long type, unsigned long param);
    BOOL disable_seek;  //add by mark;
    BOOL new_retry_mechanism;
	hwmem_config hwmem;
	UINT32 set_start_play_time_ms;
	INT32 (*decoder_get_seek_type)(void);
	BOOL (*decoder_roll_back_data)(INT64 offset, DWORD origin);
	INT64 (*decoder_get_file_size)(void);
	UINT32 (*decoder_get_cache_size)(void);
	UINT32 (*decoder_check_end)(void);
	BOOL is_mms_network_play;
	BOOL uis_hls_type;
	BOOL show_first_before_net_bufferring;
}DEC_INIT_PAR, *PDEC_INIT_PAR;


/*-------------------------------------------------------------------
Name:
    DecoderSleep
Description:
    Decoder call this function to sleep which will suspend decoder task
    immidiately and come back to task after "n" ms
Parameters:
    [IN]
        n: the time that decoder will be suspended for in ms
    [OUT]
Return:
-------------------------------------------------------------------*/
void decoder_sleep(int n);

/*----------------------------------------------------
Name:
    t2AudioSelectSource
Description:
    Select audio source type.
Parameters:
    IN:
        id: audio stream id.
        0xC0-0xDF: MPEG1 layer1/2/3 audio stream
        0x80-0x87: AC3 audio stream
        0x88-0x8F: DTS audio stream
        0x90-0x97: SDDS audio stream
        0xA0-0xA7: LPCM audio stream
        0xB0: CDDA(PCM)
        0x70-0x7f: BYE1 stream
        0x60-0x6f: DTSCD stream    special case, select and send PCM stream, but no annlog output

        Note: later, we will merge mp2 and mp3 audio stream
        into mpeg audio stream
    OUT:
Return:
    TRUE, if success, otherwise, FALSE
------------------------------------------------------*/
BOOL t2audio_select_source(DWORD id,int audio_codec_tag, int audio_channels,
                int audio_sample_rate, int audio_byte_rate,
                int audio_block_size, int audio_sample_per_block, int audio_bits_per_sample,
                int audio_encode_opt, unsigned char* extra_data);

/*----------------------------------------------------
Name:
    t2OutputAudio
Description:
    Output audio stream.
Parameters:
    IN:
        buf: the pointer to the audio stream to be output
        size: the audio stream size to be output
    OUT:
Return:
    AUDIO_WAITRETRY        : Waits for re-send the audio stream,
                            this case, the buffer is full
    AUDIO_SUCCESS        : Sent successful
    AUDIO_NOTSUPPORTED    : Audio stream is not supported.

------------------------------------------------------*/
int t2output_audio(BYTE *buf, DWORD size);

/*----------------------------------------------------
Name:
    t2StopAudio
Description:
    Stop audio playback
Parameters:
    IN:
    OUT:
Return:
------------------------------------------------------*/
void t2stop_audio(void);

DWORD t2audio_get_time(void);
void t2audio_set_time(DWORD rtime);
DWORD t2audio_buffered_remain(void);
BOOL t2audio_config(DWORD fs, WORD bitres, WORD nch);
DWORD mp4_multu64div(UINT32 v1, UINT32 v2, UINT32 v3);

#define C_MPEG4_DECODER_NO      -1
#define C_XD_LICENSE_NO       0
#define C_XD_LICENSE_YES      1
#define C_MPEG4_LICENSE_YES     2

/* return -1: no mpeg4 decoder
   return 0 : has mpeg4 decoder, but not xd certified
   return 1 : has mpeg4 decoder, and pass xd certified */
int check_xd_enable();

#endif  // _T2DECODER_H

