#ifndef LIBNET_RTSP_H
#define LIBNET_RTSP_H

#include <basic_types.h>

#define MAX_SDP_MEDIA_NUM 5
typedef enum 
{
    RTSP_TYPE_STD = 0,  // 
    RTSP_TYPE_MS,  // ms rtsp usually play with TCP interleave 
} RTSP_TYPE;

typedef enum
{
    SDP_M_NONE = 0,
    SDP_M_AUDIO,
    SDP_M_VIDEO,
    SDP_M_TEXT,
    SDP_M_APPLICATION,
    SDP_M_MESSAGE,
    SDP_M_LAST,
} SDP_MEDIA_TYPE;
typedef enum
{
    RTP_TYPE_NONE = 0,
    RTP_TYPE_UDP,
    RTP_TYPE_INTERLEAVED,
    RTP_TYPE_LAST,
} RTP_TYPE;
typedef struct sdp_media
{
    SDP_MEDIA_TYPE  media_type; // m=
    char            name[32];
    UINT16          fmt;
    RTP_TYPE        rtp_type;
    INT32           port_ch[2];
} sdp_media;

typedef struct rtsp_media_info
{
    double range[2];
    UINT32 track_num;
    sdp_media tracks[MAX_SDP_MEDIA_NUM];
    BOOL    seekable;
} rtsp_media_info;

typedef struct rtspinfostr
{
    UINT32 ulcmdbufsize;
    UINT32 ulcmdbufwritepos;
    UINT32 ulcmdbufreadpos;
    UINT8 *pctrackbuf;
    UINT8 *pccmdbuf;
    UINT8 *pcurlsource;
    CURL  *pcurl;
    Rsrc_t *prsrcinfo;
    RTSP_TYPE rtsp_type;
    rtsp_media_info media_info;
}rtspinfostr_type;

typedef struct rtpinfostr
{
    UINT32 ulrtpbufsize;
    UINT8 *pcrtpbuf;
}rtpinfostr_type;

typedef struct rtpheaderstr 
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
}rtpheaderstr_type; 

typedef UINT32 (*libnetrtsp_writedata_callback)(void *ptr, UINT32 size, UINT32 nmemb, void *userdata);

INT32 libnetrtsp_transfer(UINT32 param1,UINT32 param2);

#endif

