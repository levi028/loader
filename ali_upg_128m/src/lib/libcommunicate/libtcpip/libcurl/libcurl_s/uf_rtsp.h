#ifndef _UF_RTSP_H__
#define _UF_RTSP_H__

#include <basic_types.h>
#include <sys_config.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/urlfile.h>

#define MAX_TRACK_MEDIA_NUM     5
#define UF_RTP_REVEIVE_PORT     4670
#define UF_RTP_REVEIVE_PORT_MAX 4690

#define RTSP_CMD_BUFSIZE        12000
#define UF_RTSP_HEARTBEAT_TIME  15000
#define UF_RTP_RECEIVEBUF_SIZE  2048
#define UF_RTSP_CLIENT_AGENT    "ALi RTSP Client."

typedef enum UF_RTSP_TYPE
{
    UF_RTSP_TYPE_STD = 0,   // standard RTSP
    UF_RTSP_TYPE_MS,        // ms rtsp usually play with TCP interleave 
} UF_RTSP_TYPE;

typedef enum SDP_MEDIA_TYPE
{
    SDP_M_NONE = 0,
    SDP_M_AUDIO,
    SDP_M_VIDEO,
    SDP_M_TEXT,
    SDP_M_APPLICATION,
    SDP_M_MESSAGE,
    SDP_M_LAST,
} SDP_MEDIA_TYPE;

typedef enum UF_RTP_TYPE
{
    UF_RTP_TYPE_NONE = 0,
    UF_RTP_TYPE_UDP,
    UF_RTP_TYPE_INTERLEAVED,
    UF_RTP_TYPE_LAST,
} UF_RTP_TYPE;


typedef enum UF_RTSP_RETURN_CODE
{
    UF_RTSP_OK = 0,
    UF_RTSP_ERROR,
    UF_RTSP_MALLOC_ERROR,
    UF_RTSP_PARAM_ERROR,
    UF_RTSP_CREATETASK_ERROR,
    UF_RTSP_GETINFO_ERROR,
    UF_RTSP_CMDSPACE_ERROR,
    UF_RTSP_CMDCODE_ERROR,
    UF_RTSP_READCMDDATA_ERROR,
    UF_RTSP_CMDDATA_NOFIND,
    UF_RTSP_CONNECT_TIMEOUT,
    UF_RTSP_OPTION_ERROR,
    UF_RTSP_DESCRIBE_ERROR,
    UF_RTSP_SETUP_ERROR,
    UF_RTSP_PLAY_ERROR,
    UF_RTSP_TEARDOWN_ERROR,

    UF_RTSP_MAX_ERROR,
}UF_RTSP_RETURN_CODE;

typedef enum UF_RTSP_USR_ACTION
{
    UF_USR_ACTION_NONE = 0,
    UF_USR_ACTION_PAUSE,
    UF_USR_ACTION_RESUME,
    UF_USR_ACTION_SEEK,
    UF_USR_ACTION_ABORT,
    UF_USR_ACTION_LAST,
}UF_RTSP_USR_ACTION;

typedef enum UF_RTP_TASK_STATE
{
    UF_RTP_TASK_NONE = 0,
    UF_RTP_TASK_CREATE,
    UF_RTP_TASK_RUNNING,
    UF_RTP_TASK_STOP,
    UF_RTP_TASK_EXIT,
    UF_RTP_TASK_LAST,
}UF_RTP_TASK_STATE;

typedef enum UF_RTSP_SESSION_STATE
{
    UF_RTSP_SESSION_NONE = 0,
    UF_RTSP_SESSION_CONNECTING,
    UF_RTSP_SESSION_SETUPED,
    UF_RTSP_SESSION_PLAYED,
    UF_RTSP_SESSION_PAUSED,
    UF_RTSP_SESSION_DISCONNECT,
    UF_RTSP_SESSION_LAST,
}UF_RTSP_SESSION_STATE;

typedef struct track_media
{
    SDP_MEDIA_TYPE  media_type; // m=
    char            name[32];
    UINT16          fmt;
    UF_RTP_TYPE     rtp_type;
    INT32           port_ch[2];
    UF_RTP_TASK_STATE   state;
    urlfile         *write_fp;
} track_media;

typedef struct rtsp_media_info
{
    double      range[2];
    double      range_play;
    UINT16      setup_idx;
    UINT16      track_num;
    track_media tracks[MAX_TRACK_MEDIA_NUM];
    BOOL        seekable;
} uf_rtsp_media_info;


typedef struct uf_rtsp_connection
{
    UINT32                  last_heartbeat_time;
    UINT32                  resp_buf_siz;
    UINT32                  write_idx;
    UINT32                  read_idx;
    UINT8                   *resp_buf;
    UF_RTSP_TYPE            rtsp_type;
    uf_rtsp_media_info      media_info;
    UF_RTSP_SESSION_STATE   state;
    UF_RTSP_USR_ACTION      action;
}uf_rtsp_connection;

typedef struct uf_rtp_header
{ 
	UINT16	cc        : 4; 
	UINT16	extension : 1; 
	UINT16	padding   : 1; 
	UINT16	version   : 2; 
	UINT16	payload   : 7; 
	UINT16	marker    : 1; 
	UINT16	seqnum; 
	UINT32	timestamp; 
	UINT32	ssrc; 
}uf_rtp_header; 

typedef size_t (*rtp_write_data)(urlfile *file, char *data, size_t len);

#endif

