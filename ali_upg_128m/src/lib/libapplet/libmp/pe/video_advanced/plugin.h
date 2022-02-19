#ifndef __PLUGIN_PICTURE_H__
#define __PLUGIN_PICTURE_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "t2mpgdec.h"

/*General PE errors*/
#define AF_ERR_PE_OK                    0
#define AF_ERR_NOT_AVAILABLE            0xffffffff
#define AF_ERR_PE_FAIL                  0xfffffffe

#define AF_C_MAXOP        9

/* return value for video container init and decode:
   See pfn_decoder_init and pfn_decoder_run
*/
#define VIDEO_CON_FILE_PLAY_FAILED  FALSE   // corrupt file
#define VIDEO_CON_FILE_PLAY_OK      TRUE    // file play OK
#define VIDEO_CON_FILE_UNKNOWN      2       // file type unknown

#define MP_FILE_PLAY_FAILED  0   // corrupt file
#define MP_FILE_PLAY_OK      1    // file play OK
#define MP_FILE_UNKNOWN      2       // file type unknown

#define MUSIC_IS_OUR_FILE       1
#define MUSIC_NOT_OUR_FILE      0
#define MUSIC_OPEN_FILE_FAIL    -1


/*Container IO Control Command*/
#define CONTAINER_IO_CONTROL_BASE                 0xffff
#define CONTAINER_IO_CONTROL_GET_TIME_MS        (CONTAINER_IO_CONTROL_BASE+1)
#define CONTAINER_IO_CONTROL_EN_AC3_BS_MODE     (CONTAINER_IO_CONTROL_BASE+2)
#define CONTAINER_IO_CONTROL_GET_CHAPTER_INFO   (CONTAINER_IO_CONTROL_BASE+3)
#define CONTAINER_IO_CONTROL_GET_TOTALTIME_MS   (CONTAINER_IO_CONTROL_BASE+4)
#define CONTAINER_IO_CONTROL_SET_AVSYNC_DELAY   (CONTAINER_IO_CONTROL_BASE+5)
#define CONTAINER_IO_CONTROL_EN_MUTE_FIRST        (CONTAINER_IO_CONTROL_BASE+6)

/*Structure definition*/
typedef BOOL (*paf_pe_function)(t2decoder_control *p_decoder_ctrl, int play_speed, DWORD search_time);
typedef paf_pe_function af_pe_function_table[AF_C_MAXOP][AF_C_MAXOP];
typedef af_pe_function_table *paf_pe_function_table;

typedef enum
{
  pestop      = 0,
  peplay      = 1,
  pepause_step = 2,
  peffx2      = 3,
  peffx4      = 4,
  peffx8x16x32 = 5,
  peslow      = 6,
  perev       = 7,
  peresume_stop = 8
}AF_PE_PLAY_STATE, *PAF_PE_PLAY_STATE;

//add by mark
typedef struct
{
    UINT32 buffer_addr;
    UINT32 buffer_size;
} PE_CACHE_BUFFER;
//end

// video(container) plugin interface
typedef struct
{
    struct pe_video_cfg video;
    struct pe_music_cfg music;
}MP_CONFIG;

#ifndef WIN32
#define _avdmx_plugin(x) const AVDMX_PLUGIN x __attribute__ ((section(".video.plugin")))
#else
#define _avdmx_plugin(x) AVDMX_PLUGIN x
#endif

/*******************es player plugin***************************/
// return 0 success, otherwise failed
typedef int (*pfn_video_decoder_ioctl)(void *phandle, int cmd, void *param1, void *param2);
// return 0 success, otherwise failed
typedef int (*pfn_audio_decoder_ioctl)(int cmd, void *param1, void *param2);

typedef struct
{
    pfn_video_decoder_ioctl es_player_video_ctrl;
    pfn_audio_decoder_ioctl es_player_audio_ctrl;
} ES_PLAYER_PLUGIN;

#ifndef WIN32
    #define _esplayer_input_plugin(x) const ES_PLAYER_PLUGIN x __attribute__ ((section(".es_player.plugin")))
#else
    #define _esplayer_input_plugin(x) ES_PLAYER_PLUGIN x
#endif


typedef int (*pfn_av_decoder_install)(void);
typedef int (*pfn_av_decoder_uninstall)(void);
typedef struct
{
    const char               *name;
    pfn_av_decoder_install   avcodec_install;
    pfn_av_decoder_uninstall avcodec_uninstall;
} AVCODEC_PLUGIN;

#define _avcodec_plugin(x) const AVCODEC_PLUGIN x __attribute__ ((section(".avcodec.plugin")))

// get file ext name, in lower case, return the length of the file ext name
int get_file_name_ext(const char *filename, char *pext, int ext_size);

#ifdef __cplusplus
}
#endif

#endif// __PLUGIN_PICTURE_H__


