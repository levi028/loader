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
#include <hld/decv/decv_media_player.h>

#include <api/libmp/pe.h>
#include "mp_ctrl.h"
#include "mp_player.h"
#include "avformat.h"
#include "mp_media_analy.h"

#define NEW_ARCH
#define NEED_CHECK

#ifdef MP_DEBUG
#define MPG_PRINTF    libc_printf
#else
#define MPG_PRINTF(...)    do{}while(0)          //libc_printf
#endif
#define MP_PLAY_FLAG_PENDING  0xff    // Didnot try this contaier
#define MP_PLAY_FLAG_TRYING   0xfe    // Trying this contaier
#define MP_PLAY_FLAG_OK       VIDEO_CON_FILE_PLAY_OK // This contaier OK

#define MAX_VIDEO_PLUGIN_SUPPORT    9
#define MAX_AUDIO_PLUGIN_SUPPORT    8
#define MAX_DMX_PLUGIN_SUPPORT    32
#define FIRST_CACHE_READ_SIZE       0x400  // for network seek more fast
#define MPGFIEL_CACHE_BLK_SIZE      0x20000

#define FIRST_CACHE_READ_SIZE2       0x4000//0xd0000
#define MPGFIEL_CACHE_BLK_SIZE2     0x800//0x20000
#define MPGFILE_CACHE_SIZE          0x120000

extern void av_register_codec_parser(AVCodecParser *parser);
extern int alip_get_header(const UINT8 *buf);
extern int aac_get_header(const UINT8 *buf);
extern int ogg_get_header(const UINT8 *buf);
extern int mp3_get_packet_size(const UINT8 *buf,int size);

#define REGISTER_PARSER(X, x)                                           \
    {                                                                   \
        extern AVCodecParser ff_##x##_parser;                           \
        av_register_codec_parser(&ff_##x##_parser);                 \
    }

static int m_cache_id = -1;

static t2dec_other_buf m_other_buffer[MAX_OTHER_BUF_NO];
static BYTE m_dec_err_msg = DERR_OK;
static BOOL m_first_cache_read = TRUE;
DEC_INIT_PAR m_dec_init_par;
static t2frame_buffer m_frame_buffer[MAX_FRAME_BUF_NO];
//static struct pe_video_cfg  m_pe_video_cfg;
static struct pe_video_cfg_extra m_pe_video_cfg_extra;
static PE_CACHE_BUFFER m_cache_buffer;
static enum VDEC_OUTPUT_MODE m_vdec_outmode = RESERVED_MODE;
static struct vdec_pipinfo m_vdec_config;
static BOOL m_vdec_set_output_flg = FALSE;

static int m_cache_first_read_size = 0;
static int m_cache_blk_size = 0;
int g_libstream_enable = 0;
static BOOL http_type = FALSE;
static int   m_current_video_plugin = 0;
static UINT8 m_video_plugin_play_flag[MAX_DMX_PLUGIN_SUPPORT];


static af_pe_function_table  *m_pfn_decoder_ctrl[MAX_VIDEO_PLUGIN_SUPPORT];

extern UINT32 _video_init_struct_start;
extern UINT32 _video_init_struct_end;
extern AVDMX_PLUGIN *av_demuxer;

static char *g_mp_file_name = NULL; // ylm, need???
static int   m_dmx_plugin_count = 0;
static MP_CONFIG  m_pe_media_cfg;
static UINT8 g_mp_disable_seek[3] = {FALSE, FALSE, FALSE};
static AVDMX_PLUGIN  avdmx_plugin[MAX_DMX_PLUGIN_SUPPORT];
static MP_PRIVATE g_mp_private;
static char music_dmx[MAX_AUDIO_PLUGIN_SUPPORT][10] =
    {"aac","ac3","bye1","flac","mp3","ogg","wav", "none"}; //lpcm shoud not be place in the list.
static char video_dmx[MAX_VIDEO_PLUGIN_SUPPORT][14] =
    {"avi","bye3","flv","matroska,webm","mp4mov","mpeg","mpegts","rm","alip"};

enum e_video_stream_type
{
    STREAM_TYPE_AVI = 0,
	STREAM_TYPE_BYE2,
    STREAM_TYPE_FLV,
    STREAM_TYPE_MKV,
    STREAM_TYPE_MP4,
    STREAM_TYPE_MPG,
    STREAM_TYPE_TS,
    STREAM_TYPE_RMVB,
    STREAM_TYPE_ALIP,
};

enum e_music_stream_type
{
    STREAM_TYPE_AAC = 0,
    STREAM_TYPE_AC3,
    STREAM_TYPE_BYE1,
    STREAM_TYPE_FLAC,
    STREAM_TYPE_MP3,
    STREAM_TYPE_OGG,
    STREAM_TYPE_WAV,
};

extern void pe_cache_set_closing(int cache_id);
extern void avplayer_set_music_pause(int pause);
extern int pe_cache_get_seektype(int cache_id);
static BOOL is_music_dmx(const char * dmx_name)
{
    int i;
    for( i = 0; i < MAX_AUDIO_PLUGIN_SUPPORT; i++) {
        if(strstr(dmx_name, music_dmx[i])){
           return 1;
        }
    }
    return 0;
}

static BOOL is_video_dmx(const char * dmx_name)
{
    int i;
    for(i = 0; i < MAX_VIDEO_PLUGIN_SUPPORT; i++) {
        if(strstr(dmx_name, video_dmx[i])){
           return 1;
        }
    }
    return 0;
}

static BOOL is_video_NetPlay(void)
{
	return http_type;
}

static void Set_video_Play_Mode(BOOL Cur_http_type)
{
	http_type = Cur_http_type;
}



paf_pe_function_table video_get_ctrl_table(UINT8 stream_type)
{
    return m_pfn_decoder_ctrl[stream_type];
}

DWORD mpg_set_avsync_delay(UINT32 stream_type, INT32 time_ms)
{
   return mp_ioctl(CONTAINER_IO_CONTROL_SET_AVSYNC_DELAY, stream_type, time_ms);
}

// A general API for PE
RET_CODE video_decoder_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2)
{
    return mp_ioctl(io_cmd, param1, param2);
}

/*----------------------------------------------------
Name:
    DecoderReadData
Description:
    Decoder call this function to read data
Parameters:
    IN:
        buf: the address that data to be read to
        size: the size of the data to be read in byte,
    OUT:
Return:
    The data size has been read
------------------------------------------------------*/
static DWORD decoder_read_data(BYTE *buf, DWORD size)
{
    DWORD temp_size = 0;
    DWORD offset = 0;
    DWORD read_size = 0;
    DWORD total_size = size;

    /* Check if the input parameter buf is legal */
    if (NULL == buf)
    {
        MPG_PRINTF("\n%s:%d,  buf = NULL,  Illegal!\n", __func__, __LINE__);
        return RET_FAILURE;
    }

    while(total_size > 0)
    {
        if(total_size > (m_cache_buffer.buffer_size/2))
    {
            temp_size = (m_cache_buffer.buffer_size/2);
    }
        else
    {
            temp_size = total_size;
    }

    if(m_first_cache_read == TRUE)
        {
            m_first_cache_read = FALSE;
            read_size = pe_cache_read(m_cache_id, buf+offset, temp_size, m_cache_first_read_size);
            if(read_size > temp_size)
            {
                return read_size;
            }
            total_size -= read_size;
            offset += read_size;
        }
        else
        {
            read_size = pe_cache_read(m_cache_id, buf+offset, temp_size, temp_size);
            if(read_size > temp_size)
            {
                return read_size;
            }
            total_size -= read_size;
            offset += read_size;
        }
        if (pestop == get_next_play_state())
        {
			 libc_printf("%s, stop play\n", __func__);
            return 0;
        }
        if(/*read_size >= temp_size || */ pe_cache_check_eof(m_cache_id) || 0 == total_size)
        {
            return (size - total_size);
        }
    }

    return 0;
}

/*----------------------------------------------------
Name:
    DecoderReadData
Description:
    Decoder call this function to read data
Parameters:
    IN:
        buf: the address that data to be read to
        size: the size of the data to be read in byte,
    OUT:
Return:
    The data size has been read
------------------------------------------------------*/
/*
static DWORD DecoderReadDataFast(BYTE *buf, DWORD size)
{
    return pe_cache_read(m_cache_id, buf, size, size);
}
*/
/*----------------------------------------------------
Name:
    DecoderSeekData
Description:
    Decoder call this function to seek data in media stream.
Parameters:
    IN:
    offset: Number of bytes from "origin".
    origin: Initial postion which has 3 cases:
    SEEK_CUR: Seek from current postion
    SEEK_END: Seek from the end
    SEEK_SET: Seek from the beginning
    OUT:
Return:
    Return TRUE if success, otherwise, FALSE.
------------------------------------------------------*/
static BOOL decoder_seek_data(long long offset, DWORD origin)
{

    if (pestop == get_next_play_state())
    {
        return FALSE;
    }

    if(!pe_cache_seek(m_cache_id, (long long)offset, origin))
    {
        m_first_cache_read = TRUE;
        return TRUE;
    }
    else
        return FALSE;
}

static BOOL decoder_roll_back_data(long long offset, DWORD origin)
{
    if (get_next_play_state() == pestop)
	{
        return FALSE;
    }

	if(!pe_cache_seek(m_cache_id, (long long)offset, origin))
    {
        return TRUE;
    }
    else
    {
    	libc_printf("---------->pe_cache_seek fail\n");
        return FALSE;
    }
}

static INT64 decoder_get_file_size(void)
{
    INT64 i64_file_size;

    i64_file_size = pe_cache_filesize(m_cache_id);
    //libc_printf("%s, %x\n", __FUNCTION__, i64_file_size);
    if(i64_file_size == -1)
    {
        return 0;
    }
    else
    {
        return i64_file_size;
    }
}


static BOOL decoder_stop_play(void)
{
    if(pestop == get_next_play_state())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static INT64 decoder_tell_pos(void)
{
    return pe_cache_tell(m_cache_id);
}

INT32 decoder_get_seek_type(void)
{
    return pe_cache_get_seektype(m_cache_id);
}

UINT32 decoder_get_cache_size(void)
{
	return pe_cache_get_data_len(m_cache_id);
}

UINT32 decoder_check_end(void)
{
	return pe_cache_check_eof(m_cache_id);
}


/* For link issue, here define some NULL function for temp, later, the code should be clean */
/* end of NULL function */
BOOL g_video_dec_running = FALSE;
//static mp_callback_func func_mp_cb;

BOOL is_video_decoder_running(void)
{
    return g_video_dec_running;
}

int video_engine_set_output(enum VDEC_OUTPUT_MODE e_out_mode, struct vdec_pipinfo *p_init_info)
{
    /* Check if the input parameter p_init_info is legal */
    if (NULL == p_init_info)
    {
        MPG_PRINTF("\n%s:%d,  p_init_info = NULL,  Illegal!\n", __func__, __LINE__);
        return -1;
    }

    /* Check if the input parameter e_out_mode is legal */
    if (e_out_mode >= VDEC_OUTPUT_MODE_END)
    {
        MPG_PRINTF("\n%s:%d,  e_out_mode >= VDEC_OUTPUT_MODE_END,  Illegal!\n", __func__, __LINE__);
        return -1;
    }

    m_vdec_outmode = e_out_mode;
    m_vdec_config = *p_init_info;

    if (is_video_decoder_running())
    {
       mp_ioctl(VDEC_IO_SET_SCALE_MODE, m_vdec_outmode, (UINT32)&m_vdec_config);
    }
	else
		m_vdec_set_output_flg = TRUE;

    return 0 ;
}

int video_engine_set_start_time(UINT32 start_play_time_ms)
{
	int ret = -1;
    if (!is_video_decoder_running())
    {
     	m_pe_media_cfg.video.set_start_play_time_ms = start_play_time_ms;
		ret = 0;
    }
	else
	{
	    m_pe_media_cfg.video.set_start_play_time_ms = 0;
	}
    return ret;
}

static UINT8 video_get_container(enum e_video_stream_type stream_type)
{
    UINT8 i = 0;
    UINT8 type = 0;

    for(i=0; i<m_dmx_plugin_count; i++)
    {
		if(!strcmp(avdmx_plugin[i].name, &video_dmx[stream_type][0]))
        {
            type = i;
            return type;
        }
    }
    return -1;
}

static UINT8 video_analy(const UINT8 *buf)
{
	if(mkv_get_header(buf))
    {
        return video_get_container(STREAM_TYPE_MKV);
    }
    else if(flv_get_header(buf))
    {
        return video_get_container(STREAM_TYPE_FLV);
    }
    else if(alip_get_header(buf))
    {
       return video_get_container(STREAM_TYPE_ALIP);
    }
    else if(avi_get_header(buf))
    {
        return video_get_container(STREAM_TYPE_AVI);
    }
    else if(mp4_get_header(buf))
    {
        return video_get_container(STREAM_TYPE_MP4);
    }
    else if(bye3_get_header(buf))
    {
        return video_get_container(STREAM_TYPE_BYE2);
    }
    else if(ts_get_packet_size(buf, 1024) > 0)
    {
        return video_get_container(STREAM_TYPE_TS);
    }
    else if(ps_get_header(buf) > 0)
    {
        return video_get_container(STREAM_TYPE_MPG);
    }    
    return -1;
}

static UINT8 music_get_container(enum e_music_stream_type stream_type)
{
    UINT8 i = 0;
    UINT8 type = 0;

    for(i=0; i<m_dmx_plugin_count; i++)
    {
	  if(!strcmp(avdmx_plugin[i].name, &music_dmx[stream_type][0]))
        {
            type = i;
            return type;
        }
    }
    return -1;
}

static UINT8 music_analy(const UINT8 *buf)
{
	if(aac_get_header(buf))
    {
        return music_get_container(STREAM_TYPE_AAC);
    }
    else if(mp3_get_packet_size(buf,6144))
    {
        return music_get_container(STREAM_TYPE_MP3);
    }
    else if(ogg_get_header(buf))
    {
        return music_get_container(STREAM_TYPE_OGG);
    }
    
    return -1;
}

static UINT8 video_engine_get_type(UINT8 *type)
{
    UINT8 video_file_type = 0;
    UINT8 music_file_type = 0;
    char *data = malloc(6144);

    if(NULL == data)
         return 0;

    MEMSET(data, 0, 6144);
    decoder_read_data((BYTE *)data, 6144);
    video_file_type = video_analy((const UINT8 *)data);
    music_file_type = music_analy((const UINT8 *)data);
    free(data);
    data = NULL;

    if(video_file_type == 0xFF && music_file_type == 0xFF)
        return 0;
    else
        *type = (video_file_type == 0xFF) ?  music_file_type : video_file_type;
    return 1;
}


static int network_check_container(UINT8 *dmx_type)
{
	int ret = -1;
	m_video_plugin_play_flag[*dmx_type] = MP_PLAY_FLAG_TRYING;
    if(video_engine_get_type(dmx_type))
    {
        MPG_PRINTF("%s, %d, %s\n", __FUNCTION__, *dmx_type, avdmx_plugin[*dmx_type].name);
        m_current_video_plugin = *dmx_type;
        m_video_plugin_play_flag[*dmx_type] = MP_PLAY_FLAG_TRYING;
		av_demuxer = &avdmx_plugin[m_current_video_plugin];
		ret = 0;
        if(is_music_dmx(av_demuxer->name))
            music_engine_set_disable_seek(TRUE);
    }
    else
            music_engine_set_disable_seek(FALSE);
    decoder_roll_back_data(0, SEEK_SET);
	return ret;
}

static int retry_container(mp_callback_func func_mp_cb, int *idx, int media_type, UINT8 *dmx_type)
{
    int tmp_status=-1;
    BOOL try_other_container = FALSE;
    int i = 0;
    int ret = 2;

    osal_task_dispatch_off();

    tmp_status = get_next_play_state();

    if (((MP_FILE_UNKNOWN == m_video_plugin_play_flag[*dmx_type])
        || (MP_FILE_PLAY_FAILED == m_video_plugin_play_flag[*dmx_type]))
        && ((peplay == tmp_status) || (pepause_step == tmp_status)))
    {
        try_other_container = TRUE;
    }

    if (try_other_container)
    {
        for (i = *idx; i < m_dmx_plugin_count; i++)
        {
            if (MP_PLAY_FLAG_PENDING == m_video_plugin_play_flag[i]) {
                if ((media_type == MP_MEDIA_VIDEO) &&
                    (is_video_dmx(avdmx_plugin[i].name)))
                {
                    *idx = i;
                    break;
                }
                else if ((media_type == MP_MEDIA_MUSIC) &&
                    (is_music_dmx(avdmx_plugin[i].name)))
                {
                    *idx = i;
                    break;
                }
                m_video_plugin_play_flag[i] = MP_PLAY_FLAG_TRYING;
            }
            else
            {
                continue;
            }
        }
        if (i >= m_dmx_plugin_count)
        {
            osal_task_dispatch_on();
            if (func_mp_cb != NULL)
            {
                func_mp_cb(MP_VIDEO_PARSE_END, MP_DERR_UNKNOWN);
                func_mp_cb(MP_VIDEO_PLAYBACK_END, 0);
            }
        }
        else
        {
            m_video_plugin_play_flag[i] = MP_PLAY_FLAG_TRYING;
            *dmx_type = i;
			av_demuxer = &avdmx_plugin[i];
            osal_task_dispatch_on();

            if(is_video_NetPlay() && m_pe_media_cfg.video.new_retry_mechanism)
            {
                ret = 0;
            }
            else
            {
                ret = 1;
            }
            libc_printf("Try demux %s, ret %d\n", av_demuxer->name, ret);
        }
    }
    else
    {
        osal_task_dispatch_on();
    }

    return ret;
}

static AVDMX_PLUGIN * get_current_dmx_plugin(void)
{
	return av_demuxer;
}

MP_MEDIA_TYPE get_current_media_type(void)
{
    return g_mp_private.media_type	;
}

static int mp_register_codec_parser()
{
    /* parsers */
    REGISTER_PARSER(AAC,                aac);
    REGISTER_PARSER(AAC_LATM,           aac_latm);
    REGISTER_PARSER(AC3,                ac3);
    REGISTER_PARSER(FLAC,               flac);
    REGISTER_PARSER(MPEGAUDIO,          mpegaudio);
    REGISTER_PARSER(PCMBLUERAY,         pcmblueray);
	REGISTER_PARSER(DVDPCM,             dvdpcm);
//    REGISTER_PARSER(VORBIS,             vorbis);
	return 0;
}

extern UINT32 _avcodec__start;
extern UINT32 _avcodec__end;
static void mp_load_avcodec_plugin(void)
{
    AVCODEC_PLUGIN *avcodec_plugin = NULL;
    UINT32 avcodec_start = (UINT32)(&_avcodec__start);
    UINT32 avcodec_end   = (UINT32)(&_avcodec__end);

    if(avcodec_start == avcodec_end)
        return;

    avcodec_plugin = (AVCODEC_PLUGIN *)avcodec_start;
    while((UINT32)avcodec_plugin < avcodec_end)
    {
        libc_printf("Install avcodec plugin %s\n", avcodec_plugin->name);
        avcodec_plugin->avcodec_install();
        avcodec_plugin++;
    }
}

static int mp_load_dmx_plugin(MP_CONFIG *pmedia_cfg)
{
	int cnt = 0;
	int i;

    MPG_PRINTF("\n Into %s:%d\n", __func__, __LINE__);

    if (m_dmx_plugin_count <= 0) {
        UINT32 video_init_struct_start = (UINT32)(&_video_init_struct_start);
        UINT32 video_init_struct_end = (UINT32)(&_video_init_struct_end);

        AVDMX_PLUGIN *tmp = (AVDMX_PLUGIN *)(video_init_struct_start);
        // load plugin
        while((UINT32)tmp < video_init_struct_end)
        {
            if (cnt >= MAX_DMX_PLUGIN_SUPPORT)
            {
                // OMG! Too many video plugins.
                ASSERT(0);
                break;
            }
            avdmx_plugin[cnt] = *tmp;
            tmp++;
            cnt++;
        }
        m_dmx_plugin_count = cnt;
    }

	//init plugin
    for (i = 0; i < m_dmx_plugin_count; i++)
    {
		if(avdmx_plugin[i].init)
          avdmx_plugin[i].init(pmedia_cfg);
    }

    return 0;
}



static BOOL mp_is_netfile(char *url)
{
    if(strstr(url, "://") ||strstr(url, "dmpfs"))
		return TRUE;

    return FALSE;
}

static void mp_init_param(UINT8 preview, enum snd_dup_channel audio_channel,mp_callback_func func_mp_cb, BOOL disable_seek)
{
    m_dec_init_par.frame_buffer = m_frame_buffer;
    m_dec_init_par.other_buffer = m_other_buffer;
    m_dec_init_par.decoder_read_data = decoder_read_data;
    m_dec_init_par.decoder_seek_data = decoder_seek_data;
    m_dec_init_par.decoder_tell_pos = decoder_tell_pos;
    m_dec_init_par.get_ctrl_cmd = get_ctrl_cmd;
    m_dec_init_par.decoder_stop_play = decoder_stop_play;
    m_dec_init_par.audio_channel = audio_channel;
    m_dec_init_par.mp_callback_func = func_mp_cb;
	m_dec_init_par.decoder_get_seek_type = decoder_get_seek_type;
	m_dec_init_par.decoder_roll_back_data= decoder_roll_back_data;
	m_dec_init_par.decoder_get_file_size = decoder_get_file_size;
	m_dec_init_par.decoder_get_cache_size = decoder_get_cache_size;
	m_dec_init_par.decoder_check_end = decoder_check_end;

    if(!preview)
    {
        m_dec_init_par.preview = FALSE;
    }
    else
    {
        m_dec_init_par.preview = TRUE;
    }

    #ifndef _USE_32M_MEM_
    if(is_video_NetPlay())
    {
       m_dec_init_par.disable_seek = disable_seek;

        if (m_dec_init_par.disable_seek != 0)
        {
            MPG_PRINTF("\n Disable network mediaplay seek function \n");
        }

		if ((!MEMCMP(g_mp_file_name,"mms://",6))
        || (!MEMCMP(g_mp_file_name,"mmsh://",7))
        || (!MEMCMP(g_mp_file_name,"rtsp://",7))
        || (!MEMCMP(g_mp_file_name,"rtmp",4)))
        {
            m_dec_init_par.is_mms_network_play = TRUE;
        }
        else
        {
            m_dec_init_par.is_mms_network_play = FALSE;
        }

        m_dec_init_par.new_retry_mechanism = m_pe_media_cfg.video.new_retry_mechanism;
		m_dec_init_par.is_network_play = TRUE;
		m_dec_init_par.show_first_before_net_bufferring = m_pe_media_cfg.video.show_first_before_net_bufferring;
    }
    else
    #endif
    {
        m_dec_init_par.disable_seek = FALSE;//USB鍙互seek
        m_dec_init_par.new_retry_mechanism = FALSE;
		m_dec_init_par.set_start_play_time_ms = m_pe_media_cfg.video.set_start_play_time_ms;
		m_dec_init_par.is_network_play = FALSE;
		m_dec_init_par.is_mms_network_play = FALSE;
		m_dec_init_par.show_first_before_net_bufferring = FALSE;
    }
}

void mp_set_disable_seek(UINT8 disable, MP_MEDIA_TYPE media_type)
{
    g_mp_disable_seek[media_type] = disable;
}

UINT8 mp_get_disable_seek(MP_MEDIA_TYPE media_type)
{
    return  g_mp_disable_seek[media_type];
}

AVDMX_PLUGIN *mp_find_dmx_plugin(char *file, int *idx)
{
    int i;
    MPG_PRINTF("\n Into %s:%d\n", __func__, __LINE__);

    for (i = 0; i < m_dmx_plugin_count; i++)
    {
        if (1 == avdmx_plugin[i].read_probe(file))
        {
#ifdef NEED_CHECK
            m_video_plugin_play_flag[i] = MP_PLAY_FLAG_TRYING; // ylm???
#endif

#ifndef _USE_32M_MEM_
            if(strcmp(avdmx_plugin[i].name, "ALI Media File Decoder") == 0)
                g_libstream_enable = 1;
            else
#endif
            g_libstream_enable = 0;
            *idx = i;
            MPG_PRINTF("\n find dmx %s\n", avdmx_plugin[i].name);
            return &avdmx_plugin[i];
        }
    }

	return NULL;
}

int mp_get_decoder_info(char *filename, decoder_info *decoder_info)
{
	AVDMX_PLUGIN *dmx=get_current_dmx_plugin();
	if(dmx == NULL)
		return -1;

	if(dmx->get_decoder_info)
		dmx->get_decoder_info(filename, decoder_info);
	else
	{
        decoder_info->bit_rate = 0;
        decoder_info->sample_rate = 0;
        decoder_info->channel_mode = 0;
	}

	return 0;
}

int mp_get_stream_info(char *filename, void *media_info)
{
	int ret = -1, idx;
	AVDMX_PLUGIN *plugin=NULL;
	DEC_STREAM_INFO *p_media_info=media_info;

    if(filename == NULL)
    {
     	plugin = get_current_dmx_plugin();
    }
    else
    {
     	plugin = mp_find_dmx_plugin(filename, &idx);
    }
    if(plugin == NULL)
    {
    	libc_printf("no dmx, av_demuxer=0x%x. \n", get_current_dmx_plugin());
    	return -1;
    }

    if(plugin->get_stream_info != NULL)
        ret = plugin->get_stream_info(filename, media_info);

	//get the music information again if it's necessary
	if((0 == p_media_info->total_time)||(0 == p_media_info->audio_bitrate)){
		music_info music_inf;
		if((NULL != plugin->get_song_info)&&(NULL != filename)){
			ret = plugin->get_song_info(filename, &music_inf);
			if(0 < ret){
				p_media_info->total_time = music_inf.time;
				p_media_info->audio_bitrate = music_inf.bitrate;
			}
		}
	}

    return ret;
}

int mp_get_song_info(char *filename, void *music_info)
{
	int ret= -1, idx;
	AVDMX_PLUGIN *plugin=NULL;

    if(filename == NULL)
    {
     	plugin = get_current_dmx_plugin();
    }
    else
    {
     	plugin = mp_find_dmx_plugin(filename, &idx);
    }
    if(plugin == NULL)
    {
    	libc_printf("no dmx, av_demuxer=0x%x. \n", get_current_dmx_plugin());
    	return -1;
    }

    if(plugin->get_song_info != NULL)
        ret = plugin->get_song_info(filename, music_info);

    return  ret;
}

DWORD mp_get_current_time()
{
	return avplayer_get_play_time()/1000;
}

DWORD mp_get_current_time_ms()
{
	return avplayer_get_play_time();
}

int mp_init(struct pe_video_cfg *pe_video_cfg, struct pe_music_cfg *pe_music_cfg)
{

    MPG_PRINTF("\n Into %s:%d\n", __func__, __LINE__);
    if(NULL == pe_video_cfg || NULL == pe_music_cfg)
    {
    	return -1;
    }

    if(NULL == pe_video_cfg->mp_cb|| NULL == pe_music_cfg->mp_cb)
    {
        MPG_PRINTF("\n%s:%d,  pe_video_cfg->mp_cb = NULL,  Illegal!\n", __func__, __LINE__);
        return -1;
    }

    #ifdef _M3715C_
    pe_video_cfg->decoder_buf = __MM_SEE_MP_BUF_ADDR;
    pe_video_cfg->decoder_buf_len = __MM_SEE_PRIVATE_TOP_ADDR - __MM_SEE_MP_BUF_ADDR;//__MM_SEE_MP_BUF_LEN;
    pe_video_cfg->mm_vbv_len = __MM_SEE_VBV_LEN;
    #endif

    /* Addr legal check */
    if(FALSE == osal_check_address_range_legal((void *)pe_video_cfg->decoder_buf, pe_video_cfg->decoder_buf_len))
    {
       MPG_PRINTF("%s:%d Addr Illegal!\n",__func__,__LINE__);
       return -1;
    }

	memcpy(&m_pe_media_cfg.video, pe_video_cfg, sizeof(struct pe_video_cfg));
	memcpy(&m_pe_media_cfg.music, pe_music_cfg, sizeof(struct pe_music_cfg));

    if (m_pe_media_cfg.video.reserved != 0)
    {
        m_pe_video_cfg_extra = *(struct pe_video_cfg_extra *)(m_pe_media_cfg.video.reserved);
        m_pe_media_cfg.video.reserved = (UINT32)(&m_pe_video_cfg_extra);
    }

	//load plugin
	mp_load_dmx_plugin(&m_pe_media_cfg);
    mp_load_avcodec_plugin();
    mp_register_codec_parser();

    if (OSAL_INVALID_ID == dec_cmd_sema_id)
    {
        dec_cmd_sema_id = osal_semaphore_create(1);
	     if (OSAL_INVALID_ID == dec_cmd_sema_id)
	    {
	        MPG_PRINTF("Create dec_cmd_sema_id failed!\n");
	        ASSERT(0);
	        return -1;
	    }
   }

    if ((enum VDEC_OUTPUT_MODE)RESERVED_MODE == m_vdec_outmode) //ylm ???
    {
        m_vdec_outmode = MP_MODE;
        m_vdec_config.src_rect.u_start_x = 0;
        m_vdec_config.src_rect.u_start_y = 0;
        m_vdec_config.src_rect.u_width = PICTURE_WIDTH;
        m_vdec_config.src_rect.u_height = PICTURE_HEIGHT;
        m_vdec_config.dst_rect = m_vdec_config.src_rect;
    }

	return 0;
}


void mp_cleanup(void)
{
    int i = 0;

    if(m_dmx_plugin_count <= 0)
    {
        libc_printf("%s: dmx plugin not exist.\n", __func__);
        return;
    }

    for (i = 0; i < m_dmx_plugin_count; i++)
    {
		if(avdmx_plugin[i].cleanup)
            avdmx_plugin[i].cleanup();
    }
}

int mp_process_task(UINT32 para1, UINT32 para2)
{
    UINT8 stream_type = (para2 >>16)&0xff;
    enum snd_dup_channel audio_channel = (para2 >>8)&0xff;
    UINT8 preview = para2&0xff;
    int  init_ok = 0, search_index = 0;
    BOOL decode_ok = 0;
    BOOL ret = 0;
    struct vpo_device *vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    mp_callback_func func_mp_cb = (mp_callback_func)para1;
    PE_CACHE_BUFFER cache_buf;
	struct pe_video_cfg video_cfg = m_pe_media_cfg.video;
    struct pe_video_cfg *pvideo_cfg = &video_cfg;
	struct pe_video_cfg_extra *extra_buf = NULL;
	PE_CACHE_BUFFER tmp_buff={0,0};

    cache_buf.buffer_addr = 0;
    cache_buf.buffer_size = MPGFILE_CACHE_SIZE;

    MPG_PRINTF("\n In to %d\n", __LINE__);
	if(pvideo_cfg->reserved != 0)
        extra_buf = (struct pe_video_cfg_extra *)pvideo_cfg->reserved;

    if(is_video_NetPlay() && pvideo_cfg->new_retry_mechanism /*&& g_mp_private.media_type == MP_MEDIA_VIDEO*/)
    {
		if(extra_buf)
		{
			if(0 != extra_buf->reserved[5])
			{
				tmp_buff.buffer_size = extra_buf->reserved[5];
				tmp_buff.buffer_addr =  pvideo_cfg->decoder_buf & 0x8FFFFFFF;
			}
		}
		if(0 == tmp_buff.buffer_size)
		{
			tmp_buff.buffer_size = 0x600000;// 8M for net file.
			tmp_buff.buffer_addr =  pvideo_cfg->decoder_buf & 0x8FFFFFFF;
		}
		pvideo_cfg->decoder_buf += tmp_buff.buffer_size;
     	pvideo_cfg->decoder_buf_len -= tmp_buff.buffer_size;
        m_cache_buffer = tmp_buff;
        m_cache_id = pe_cache_open(g_mp_file_name, (UINT8 *)(tmp_buff.buffer_addr),
                                 tmp_buff.buffer_size, m_cache_blk_size);
        if(m_cache_id == -1)
        {
            avplayer_rls_init_param(pvideo_cfg, &m_dec_init_par, &m_cache_buffer);
            MPG_PRINTF("%s:%d, create pe cache failed\n", __func__, __LINE__);
            func_mp_cb(MP_VIDEO_DECODER_ERROR, MP_DERR_ENGINE_TASK);
            goto LBL_TASK_EXIT2;
        }

LBL_TRY_VIDEO_DECODER_2:
        MPG_PRINTF("t2VideoDec_Task: %d\n", stream_type);

        g_video_dec_running = TRUE;
        pestream_start(stream_type);
		if( 0 > network_check_container(&stream_type))
		{
			MPG_PRINTF("network_check_container failed\n", __func__, __LINE__);
                    goto LBL_TASK_EXIT2;
		}
        mp_init_param(preview, audio_channel, func_mp_cb, mp_get_disable_seek(g_mp_private.media_type));
        ret = avplayer_get_init_param(pvideo_cfg, g_mp_private.media_type, &m_dec_init_par, &m_cache_buffer);
        if (!ret)
        {
            MPG_PRINTF("%s:%d, get_init_param failed\n", __func__, __LINE__);
            func_mp_cb(MP_VIDEO_DECODER_ERROR, MP_DERR_ENGINE_TASK);
            goto LBL_TASK_EXIT2;
        }
        MPG_PRINTF("After get_init_param()\n");
        MPG_PRINTF("m_pe_media_cfg: %x, %d\n", pvideo_cfg->decoder_buf, pvideo_cfg->decoder_buf_len);
        MPG_PRINTF("m_cache_buffer: %x, %d\n", m_cache_buffer.buffer_addr, m_cache_buffer.buffer_size);
    }
    else
    {
LBL_TRY_VIDEO_DECODER:
        MPG_PRINTF("\n In to %d\n", __LINE__);

        g_video_dec_running = TRUE;
        pestream_start(stream_type);
        mp_init_param(preview, audio_channel, func_mp_cb, mp_get_disable_seek(g_mp_private.media_type));

		//ylm: 1. get cache buffer and other buffer
        ret = avplayer_get_init_param(pvideo_cfg, g_mp_private.media_type, &m_dec_init_par, &cache_buf);
        if (!ret)
        {
            MPG_PRINTF("%s:%d, get_init_param failed\n", __func__, __LINE__);
            func_mp_cb(MP_VIDEO_DECODER_ERROR, MP_DERR_ENGINE_TASK);
            goto LBL_TASK_EXIT2;
        }
        MPG_PRINTF("\n In to %d\n", __LINE__);

		//ylm: 2. open cache
        m_cache_buffer = cache_buf;
        m_cache_id = pe_cache_open(g_mp_file_name, (UINT8 *)(cache_buf.buffer_addr),
                                 cache_buf.buffer_size, m_cache_blk_size);
        if(m_cache_id == -1)
        {
            avplayer_rls_init_param(pvideo_cfg, &m_dec_init_par, &m_cache_buffer);
            MPG_PRINTF("%s:%d, create pe cache failed\n", __func__, __LINE__);
            func_mp_cb(MP_VIDEO_DECODER_ERROR, MP_DERR_ENGINE_TASK);
            goto LBL_TASK_EXIT2;
        }
    }
    MPG_PRINTF("\n In to %d\n", __LINE__);

    if(!m_vdec_config.src_rect.u_width || !m_vdec_config.src_rect.u_height) // ylm: ???
    {
        m_vdec_config.src_rect.u_width = PICTURE_WIDTH;
        m_vdec_config.src_rect.u_height = PICTURE_HEIGHT;
        m_vdec_config.dst_rect = m_vdec_config.src_rect;
    }

	//ylm: 3. set scale mode
	if(m_vdec_set_output_flg)
    	avplayer_decoder_ioctl(VDEC_IO_SET_SCALE_MODE, m_vdec_outmode, (UINT32)&m_vdec_config);

    m_first_cache_read = TRUE;

   //ylm: 4. init decoder
    MPG_PRINTF("\n In to %d\n", __LINE__);

    init_ok = avplayer_decoder_init(&m_dec_init_par);

    MPG_PRINTF("\n In to %d\n", __LINE__);
    m_video_plugin_play_flag[stream_type] = init_ok;
    if (init_ok == MP_FILE_PLAY_OK)
    {
    	m_vdec_set_output_flg = FALSE;
        m_dec_err_msg = DERR_OK;
    }
    else if (init_ok == MP_FILE_UNKNOWN)
    {
        m_dec_err_msg = DERR_OK;
        MPG_PRINTF("%s:%d, file type unknown\n", __func__, __LINE__);
        goto EXIT_T2TASK;
    }
    else
    {
        m_dec_err_msg = avplayer_decoder_get_last_err_msg();
        MPG_PRINTF("%s:%d, decode init fail\n", __func__, __LINE__);
        goto EXIT_T2TASK;
    }

#if 0
    if(m_decallback.video_mpcall_back.request_callback)
    {
        vpo_win_mode(vpo_dev, VPO_MAINWIN, &m_decallback.video_mpcall_back, NULL);
    }
#endif
	if(is_video_NetPlay() && pvideo_cfg->new_retry_mechanism)
	{
		pe_cache_full_use(m_cache_id);
	}

    MPG_PRINTF("\n In to %d\n", __LINE__);
    mpg_cmd_search_proc(0);
    if (init_ok == MP_FILE_PLAY_OK)
    {
		// ylm: 5. start to decoder
		func_mp_cb(MP_MUSIC_PLAYBACK_BEGIN, 0);
		
        MPG_PRINTF("\n In to %s:%d\n", __func__, __LINE__);
        decode_ok = avplayer_decoder_decode(func_mp_cb);
        m_video_plugin_play_flag[stream_type] = decode_ok;
        if (MP_FILE_PLAY_FAILED == decode_ok)
        {
            m_dec_err_msg = avplayer_decoder_get_last_err_msg();
            MPG_PRINTF("%s:%d, decoder error!(%d)\n", __func__, __LINE__, m_dec_err_msg);
        }
        else if (MP_FILE_UNKNOWN == decode_ok)
        {
            m_dec_err_msg = DERR_OK;
            MPG_PRINTF("%s:%d, file type unknown\n", __func__, __LINE__);
        }
        else
        {
            m_dec_err_msg = DERR_OK;
            MPG_PRINTF("%s:%d, decode finish\n", __func__, __LINE__);
        }
    }
    MPG_PRINTF("\n In to %s:%d\n", __func__, __LINE__);

//    mpg_file_get_stream_info(&dec_stream_info); //ylm ???
EXIT_T2TASK:
    avplayer_decoder_reset(preview);
    if(is_video_NetPlay() && pvideo_cfg->new_retry_mechanism)
    {
         ret =  decoder_roll_back_data(0, SEEK_SET);//decoder_seek_data(0, SEEK_SET);
         if (FALSE == ret)
         {
             MPG_PRINTF("\n%s:%d,  decoder_seek_data  fail !\n", __func__, __LINE__);
         }
    }
#ifdef SUPPORT_MPEG4_TEST
    if(DERR_MP4_NO_INDEX == (avplayer_decoder_get_last_err_msg()))
    {
        //For no idx avi file, re-run it without first seek -- Michael
        ret = decoder_seek_data(0, SEEK_SET);
        if (FALSE == ret)
        {
            MPG_PRINTF("\n%s:%d,  decoder_seek_data  fail !\n", __func__, __LINE__);
        }
        avplayer_decoder_init(&m_dec_init_par);
        avplayer_decoder_decode(func_mp_cb);
        avplayer_decoder_reset(preview);
    }
#endif

    if(is_video_NetPlay() && pvideo_cfg->new_retry_mechanism)
    {
        ;
    }
    else
    {
        pe_cache_close(m_cache_id);
        /*eric.cai */
        m_cache_id = -1;
    }

    avplayer_rls_init_param(pvideo_cfg, &m_dec_init_par, &m_cache_buffer);
    h264_decoder_select(0, m_dec_init_par.preview);
    if(0 == g_libstream_enable)
    {
        if (vpo_dev && g_mp_private.media_type == MP_MEDIA_VIDEO)
        {
            vpo_win_onoff(vpo_dev, FALSE);
        }
    }
//#if 0 // disable retry, should modify probe
    ret = retry_container(func_mp_cb, &search_index, g_mp_private.media_type, &stream_type);
    if(0 == ret)
    {
        goto LBL_TRY_VIDEO_DECODER_2;
    }
    else if(1 == ret)
    {
        goto LBL_TRY_VIDEO_DECODER;
    }
//#endif

LBL_TASK_EXIT2:
    if(is_video_NetPlay() && pvideo_cfg->new_retry_mechanism && m_cache_id != -1)
    {
        MPG_PRINTF("http_type: %d pe cache close %d\n", http_type, m_cache_id);
        pe_cache_close(m_cache_id);
        /*eric.cai */
        m_cache_id = -1;
    }
    g_video_dec_running = FALSE;
    p_dec_func_table = NULL;
    Set_video_Play_Mode(FALSE);
    m_vdec_set_output_flg = FALSE;
	return 0;
}

int mp_play(char *file, enum snd_dup_channel audio_channel, BOOL preview, MP_MEDIA_TYPE media_type)
{
    OSAL_T_CTSK t_ctsk;
    OSAL_ID    tsk_id = OSAL_INVALID_ID;
	int dmx_type;

    /* Check if the input parameter video_file is legal */
    if (NULL == file)
    {
        MPG_PRINTF("\n%s:%d,  file = NULL,  Illegal!\n", __func__, __LINE__);
        return -1;
    }

    /* Check if the input parameter audio_channel is legal */
    if (media_type == MP_MEDIA_VIDEO && audio_channel >= SND_DUP_CHANNEL_END)
    {
        MPG_PRINTF("\n%s:%d,  audio_channel >= SND_DUP_END,  Illegal!\n", __func__, __LINE__);
        return -1;
    }

    MPG_PRINTF("Into %s: %s\n", __func__, file);

	Set_video_Play_Mode(mp_is_netfile(file));

#ifdef NEED_CHECK
    while(g_video_dec_running == TRUE)  // ylm??? 为什么需要，看起来是让dec_task结束
    {
        osal_task_sleep(1);
    }
#endif

    MEMSET(m_video_plugin_play_flag, MP_PLAY_FLAG_PENDING, sizeof(m_video_plugin_play_flag));

    if((av_demuxer = mp_find_dmx_plugin(file, &dmx_type)) == NULL)
    {
        libc_printf("%s:%d: Could not find demux! File:%s\n", __func__, __LINE__, file);
        av_demuxer = &avdmx_plugin[0];
        //return -1;
    }

    if(1 == g_libstream_enable)
    {
        m_cache_first_read_size = FIRST_CACHE_READ_SIZE2;
        m_cache_blk_size = MPGFIEL_CACHE_BLK_SIZE2;
    }
    else
    {
        m_cache_first_read_size = FIRST_CACHE_READ_SIZE;
        m_cache_blk_size = MPGFIEL_CACHE_BLK_SIZE;
    }

    MPG_PRINTF("es player enable %d\n", g_libstream_enable);
	g_mp_private.toggle_pause = FALSE;
	g_mp_private.media_type = media_type;

    // Note: make sure video_file point to global memroy already -- Jerry Long on 2010-07-08
    //STRCPY(m_video_file_name, video_file);
    g_mp_file_name= file;

    //create the decode task
#ifdef SUPPORT_MPEG4_TEST
#ifdef SW_DECODE
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
#else
    t_ctsk.itskpri = OSAL_PRI_HIGH;
#endif
#else
    t_ctsk.itskpri = OSAL_PRI_HIGH;
#endif
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)((media_type == MP_MEDIA_VIDEO)?m_pe_media_cfg.video.mp_cb:m_pe_media_cfg.music.mp_cb);
    t_ctsk.para2 = ((dmx_type<<16)|(audio_channel<<8)|preview);
    t_ctsk.name[0] = 'M';
    t_ctsk.name[1] = 'P';
    t_ctsk.name[2] = 'E';
    t_ctsk.task = (FP)mp_process_task;
    tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == tsk_id)
    {
        return -1;
    }

    return 0;
}

int mp_pause_av(BOOL pause_decode, BOOL pause_display)
{
    if(get_current_media_type() == MP_MEDIA_VIDEO/*need to check current status*/)
    {
        ali_video_decoder_pause(NULL, pause_decode,pause_display);
        ali_audio_decoder_pause(NULL, pause_decode,pause_display);
		if(!pause_decode && !pause_display)
		{
			ali_video_decoder_set_slow_ratio(NULL, 0);
		}
    }

    return 0;
}

int mp_step_av()
{
    if(get_current_media_type() == MP_MEDIA_VIDEO/*need to check current status*/)
    {
        ali_video_decoder_step(NULL);
    }

    return 0;
}

int mp_slow_av(int src_ratio)
{
    if(get_current_media_type() == MP_MEDIA_VIDEO/*need to check current status*/)
    {
    	ali_video_decoder_pause(NULL, FALSE,FALSE);
        ali_audio_decoder_pause(NULL, FALSE,FALSE);
        ali_video_decoder_set_slow_ratio(NULL, src_ratio);
    }

    return 0;
}

int mp_toggle_pause_music()
{
    int ret = -1;
	BOOL *pause = &g_mp_private.toggle_pause;

	*pause = !*pause;

    if(get_current_media_type() == MP_MEDIA_MUSIC/*need to check current status*/) {
        ret = ali_audio_decoder_pause(NULL, *pause,*pause);
    }

    if(ret == 0) {
        avplayer_set_music_pause(*pause);
    }

    return ret;
}

DWORD mp_get_total_time()
{
	return avplayer_get_duration();
}

int mp_stop()
{
	return 0;
}

int mp_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2)
{
	return avplayer_decoder_ioctl(io_cmd, param1, param2);
}

void video_engine_tell_pecache_closing(void)
{
    pe_cache_set_closing(m_cache_id);
}
