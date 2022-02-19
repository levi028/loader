#ifndef _UF_RTSP_LIVE555_H__
#define _UF_RTSP_LIVE555_H__

#include <basic_types.h>
#include <sys_config.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/urlfile.h>



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

typedef struct uf_rtsp_private
{
    void*               client;
    UF_RTSP_USR_ACTION  action;
    BOOL                b_running;
    BOOL                b_pause;
    UINT32              last_data_tick;
#if 1 // debug
    FILE *dump;
    FILE *dump_es_v;
    FILE *dump_es_a;
    FILE *log;
#endif
}uf_rtsp_private;

#endif

