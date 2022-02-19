#ifndef __VIDEO_ENGINE_H__
#define __VIDEO_ENGINE_H__

#include <api/libmp/pe.h>
#include <hld/snd/snd_dev.h>
#include "plugin.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	MP_MEDIA_VIDEO,
	MP_MEDIA_MUSIC,
	MP_MEDIA_OTHER,
}MP_MEDIA_TYPE;

typedef struct
{
	BOOL is_network_stream;
	BOOL disable_seek;
	char *file_name;
    MP_MEDIA_TYPE media_type;
    BOOL toggle_pause;
}MP_PRIVATE;

#define MPGFILE_DATA_BUFFER_ID        0
#define SYSTEM_BUFFER_ID            1

extern UINT32 _adec_init_struct_start;
extern UINT32 _adec_init_struct_end;
extern BOOL g_video_dec_running;

int video_engine_init(struct pe_video_cfg *pe_video_cfg);

#ifdef DUAL_ENABLE
#ifndef _LIB_PE_ADV_VIDEO_ENGINE_REMOTE
int video_engine_decode(char *video_file, UINT8 video_stream_type, enum snd_dup_channel audio_channel, BOOL preview);
int video_engine_set_output(enum VDEC_OUTPUT_MODE e_out_mode, struct vdec_pipinfo *p_init_info);
DWORD mpg_cmd_stop_proc(int stop_type);
DWORD mpg_cmd_set_speed(UINT32 speed);
int video_engine_pe_cache_init(pe_cache_ex *pe_cache_info_ex, pe_cache_cmd *pe_cache_cmd_buf, ID pe_cache_mutex);
int video_engine_pe_cache_release();
void video_engine_tell_pecache_closing();

RET_CODE es_player_decv_ioctl(void *phandle, int cmd, void *param1, void *param2);
RET_CODE es_player_deca_ioctl(int cmd, void *param1, void *param2);
RET_CODE video_decoder_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2);

#endif
#endif

int video_engine_set_start_time(UINT32 start_play_time_ms);
paf_pe_function_table video_get_ctrl_table(UINT8 stream_type);


void mp_set_disable_seek(UINT8 disable, MP_MEDIA_TYPE media_type);
UINT8 mp_get_disable_seek(MP_MEDIA_TYPE media_type);
int mp_get_stream_info(char *filename, void *media_info);
int mp_get_song_info(char *filename, void *music_info);
DWORD mp_get_current_time();
DWORD mp_get_current_time_ms();
int mp_init(struct pe_video_cfg *pe_video_cfg, struct pe_music_cfg *pe_music_cfg);
void mp_cleanup();
int mp_play(char *file, enum snd_dup_channel audio_channel, BOOL preview, MP_MEDIA_TYPE media_type);
DWORD mp_get_total_time();
int mp_stop();
int mp_seek();
int mp_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2);
int mp_toggle_pause_music();
int mp_pause_av(BOOL pause_decode, BOOL pause_display);
int mp_step_av();
int mp_slow_av(int src_ratio);
BOOL is_video_decoder_running(void);
MP_MEDIA_TYPE get_current_media_type(void);
int mp_get_decoder_info(char *filename, decoder_info *decoder_info);

#ifdef __cplusplus
}
#endif

#endif //__VIDEO_ENGINE_H__

