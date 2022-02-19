#include <sys_config.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>
#include "uf_rtsp.h"
#include "curl_base64.h"

#define UF_RTSP_DEBUG

#define MSRTP_SBIT(byte) ((byte)&0x80)>>7
#define MSRTP_LBIT(byte) ((byte)&0x40)>>6
#define MSRTP_RBIT(byte) ((byte)&0x20)>>5
#define MSRTP_DBIT(byte) ((byte)&0x10)>>4
#define MSRTP_IBIT(byte) ((byte)&0x08)>>3
#define MSRTP_LEN_OFFSET(byte_addr) ((*byte_addr)<<16|(*(byte_addr+1))<<8|(*(byte_addr+2)))


#ifdef UF_RTSP_DEBUG
    #define UF_RTSP_PRINT if (g_uf_rtsp_debug)libc_printf
#else
    #define UF_RTSP_PRINT(...) do{}while(0)
#endif

static INT32 g_uf_rtsp_debug = 0;

static INT32 gs_rtsp_udp_port       = UF_RTP_REVEIVE_PORT;
static INT32 gs_rtsp_interleaved_ch = -1;
static int uf_rtsp_rtp_task_create(track_media *track, rtp_write_data write_cb);
extern void network_set_seek_type(UINT8 seek_type);

static BOOL new_udp_port_pair(INT32 *rtp,INT32 *rtcp)
{
    BOOL ret = FALSE;

    if ((NULL == rtp) || (NULL == rtcp))
    {
        return ret;
    }

    *rtp = gs_rtsp_udp_port++;
    *rtcp = gs_rtsp_udp_port++;

    ret = TRUE;
    
    return ret;
}

static BOOL new_interleaved_ch_pair(INT32 *ch1, INT32 *ch2)
{
    BOOL ret = FALSE;

    if ((NULL == ch1) || (NULL == ch2))
    {
        return ret;
    }

    *ch1 = gs_rtsp_interleaved_ch++;
    *ch2 = gs_rtsp_interleaved_ch++;
    
    ret = TRUE;

    return ret;
}

static BOOL uf_rtsp_param_init(urlfile *file)
{
    uf_rtsp_connection  *prtsp_conn = NULL;

    if (NULL == file)
    {
        UF_RTSP_PRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return FALSE;
    }

    prtsp_conn = MALLOC(sizeof(uf_rtsp_connection));
    if (NULL == prtsp_conn)
    {
        UF_RTSP_PRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return FALSE;
    }
    MEMSET(prtsp_conn,0,sizeof(uf_rtsp_connection));
    

    prtsp_conn->resp_buf = MALLOC(RTSP_CMD_BUFSIZE);
    if (NULL == prtsp_conn->resp_buf)
    {
        FREE(prtsp_conn);
        UF_RTSP_PRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return FALSE;
    }
    MEMSET(prtsp_conn->resp_buf,0,RTSP_CMD_BUFSIZE);
    prtsp_conn->resp_buf_siz = RTSP_CMD_BUFSIZE;
    prtsp_conn->read_idx = 0;
    prtsp_conn->write_idx = 0;
    prtsp_conn->state = UF_RTSP_SESSION_NONE;

    file->private = (void*)prtsp_conn;

    if (gs_rtsp_interleaved_ch == -1)
    {
        gs_rtsp_interleaved_ch = 0;
    }
    if ((gs_rtsp_udp_port == 0) || (gs_rtsp_udp_port > UF_RTP_REVEIVE_PORT_MAX))
    {
        gs_rtsp_udp_port = UF_RTP_REVEIVE_PORT;
    }

    return TRUE;
}

static void uf_rtsp_param_free(urlfile *file)
{
    uf_rtsp_connection  *prtsp_conn = NULL;

    if ((NULL == file)||(NULL == file->private))
    {
        UF_RTSP_PRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return;
    }

    prtsp_conn = (uf_rtsp_connection*)file->private;
    
    if (prtsp_conn->resp_buf)
    {
        FREE(prtsp_conn->resp_buf);
        prtsp_conn->resp_buf = NULL;
    }

    if (file->private)
    {
        FREE(file->private);
        file->private = NULL;
    }

    gs_rtsp_interleaved_ch = 0;

}

static UF_RTSP_RETURN_CODE uf_rtsp_get_ms_stream_head(urlfile *file, char *data)
{
    unsigned int ret = 0;
    unsigned int ulheadlen = 0;
    char *start_pos = NULL;
    char *end_pos = NULL;
    char *head_data = NULL;

    start_pos = strstr(data,"base64");
    if (NULL == start_pos)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return UF_RTSP_CMDDATA_NOFIND;
    }
    start_pos = start_pos + 7;

    end_pos = strstr(start_pos,"\r\n");
    if (NULL == start_pos)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return UF_RTSP_CMDDATA_NOFIND;
    }
    UF_RTSP_PRINT("%s:%d,%d.\n",__FUNCTION__,*start_pos,*end_pos);
    *end_pos = '\0';

    ret = Curl_base64_decode(start_pos,(unsigned char **)(&head_data),(size_t *)&ulheadlen);
    if (UF_RTSP_OK != ret)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        if(head_data) FREE(head_data);
        return UF_RTSP_CMDDATA_NOFIND;
    }

    ret = uf_store_data(file, head_data,ulheadlen);
    if (ulheadlen != ret)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        if(head_data) FREE(head_data);
        return UF_RTSP_CMDDATA_NOFIND;
    }
    if(head_data) FREE(head_data);
    return UF_RTSP_OK;
}

static BOOL uf_rtsp_sdp_parse(urlfile *file)
{
    char *psdp = NULL;
    char *ptmp = NULL;
    char strtmp[32] = {0};
    INT32 i = 0;
    uf_rtsp_connection  *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;
    
    if (NULL == conn)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    psdp = (char*)conn->resp_buf;

AGAIN:
    psdp = strstr(psdp, "v=");
    if (NULL == psdp)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return FALSE;
    }
    ptmp = psdp+3;

    if ((*ptmp != 0x0D) && (*(ptmp+1) != 0x0A))
    {
        goto AGAIN;
    }

    psdp = ptmp + 2;

    // find range
    ptmp = strstr(psdp, "a=range:npt=");
    if (ptmp)
    {
        psdp = ptmp;
        ptmp += strlen("a=range:npt=");
        conn->media_info.range[0] = 0.0;
        conn->media_info.range[1] = 0.0;
        //libc_printf("#%s\n", ptmp);
        if ((*ptmp != 'n') && (*(ptmp+1) != 'o')) // check npt=now-
        {
            sscanf(ptmp, "%lf-%lf", &conn->media_info.range[0], &conn->media_info.range[1]);
        }
        else
        {
            conn->media_info.range[0] = 0.0;
            conn->media_info.range[1] = 0.0;
        }
        if (conn->media_info.range[1] > 0.0)
        {
            conn->media_info.seekable = TRUE;
            UF_RTSP_PRINT("SEEKABLE TRUE\n");
        }
        else 
        {
            conn->media_info.seekable = FALSE;
            UF_RTSP_PRINT("SEEKABLE FALSE\n");
        }
        conn->media_info.range_play = 0.0; // always play from 0.0
        //libc_printf("Range %f-%f\n", pinfo->media_info.range[0], pinfo->media_info.range[1]);

        ptmp = strstr(ptmp, "\r\n");
        psdp = ptmp + 2;
    }

    conn->media_info.track_num = 0;
    conn->media_info.setup_idx = 0;
    memset(conn->media_info.tracks, 0, MAX_TRACK_MEDIA_NUM*sizeof(track_media));
    while ((ptmp = (strstr(psdp, "m=")))) 
    {
        // m=<media> <port> <proto> <fmt>
        if (NULL == ptmp)
        {
            break;
        }

        psdp = ptmp + 2;
        ptmp = psdp;
        memset(strtmp, 0, 32);
        i = 0;

        while ((*ptmp != ' ') && (i<31)) 
        {
            strtmp[i++] = *ptmp;
            ptmp++;
        }

        i = conn->media_info.track_num;
        if (0 == strcmp(strtmp, "audio"))
        {
            conn->media_info.tracks[i].media_type = SDP_M_AUDIO;
        }
        else if (0 == strcmp(strtmp, "video"))
        {
            conn->media_info.tracks[i].media_type = SDP_M_VIDEO;
        }
        //else if (0 == strcmp(strtmp, "application"))
        //{
        //    conn->media_info.tracks[i].media_type = SDP_M_APPLICATION;
        //}
        else 
        {
            UF_RTSP_PRINT("Other media type: [%s]\n", strtmp);
            continue;
        }

        // skip port
        ptmp++;
        while (*ptmp != ' ') ptmp++;
        // skip proto
        ptmp++;
        while (*ptmp != ' ') ptmp++;

        // get fmt
        ptmp++;
        sscanf(ptmp, "%d", (int *)(&conn->media_info.tracks[i].fmt));

        // get name
        psdp = ptmp;
        ptmp = strstr(psdp, "a=control:");
        if (ptmp)
        {
            ptmp += strlen("a=control:");
            sscanf(ptmp, "%s", conn->media_info.tracks[i].name);
        }
        UF_RTSP_PRINT("TRACK  %s, fmt %d\n", conn->media_info.tracks[i].name, conn->media_info.tracks[i].fmt);
        
        conn->media_info.tracks[i].write_fp = file;
        if ((conn->media_info.track_num+1) >= MAX_TRACK_MEDIA_NUM)
            break;

        conn->media_info.track_num++;
    };

    return TRUE;
}

static UF_RTSP_USR_ACTION uf_rtsp_get_user_cmd(urlfile *file)
{
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    return conn->action;
}

static void uf_rtsp_set_user_cmd(urlfile *file, UF_RTSP_USR_ACTION action)
{
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    conn->action = action;
}

static size_t uf_rtsp_resp_buf_write(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    urlfile *file = NULL;
    uf_rtsp_connection  *conn = NULL;
    size_t cnt = 0;
    //unsigned int i = 0;    
    unsigned int ultotallen = 0;

    file = (urlfile *)userdata;
    if ((NULL == file)||(NULL == file->private))
    {
        return 0;
    }

    cnt = size * nmemb;
    conn = (uf_rtsp_connection*)file->private;
    if (NULL == conn->resp_buf)
    {
        return 0;
    }
    
    ultotallen = conn->write_idx + cnt;
    if (ultotallen > conn->resp_buf_siz)
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return 0;
    }

    MEMCPY(&(conn->resp_buf[conn->write_idx]), (UINT8*)ptr, cnt);
    conn->write_idx += cnt;
    
    return cnt;
}

static void uf_rtsp_resp_buf_reset(urlfile *file)
{
    uf_rtsp_connection  *conn = NULL;

    if ((NULL == file)||(NULL == file->private))
    {
        return;
    }

    conn = (uf_rtsp_connection*)file->private;
    if (NULL == conn->resp_buf)
    {
        return;
    }

    MEMSET(conn->resp_buf,0,conn->resp_buf_siz);
    conn->write_idx = 0;
    conn->read_idx = 0;

}
static UF_RTSP_RETURN_CODE uf_rtsp_resp_buf_parse(urlfile *file, long req_type)
{
    unsigned int ulrtspver = 0;
    unsigned int ulrtspsub = 0;
    unsigned int ulrtspcode = 0;
    char *acrtspstatus[50] = {0};
    unsigned int ulretcode = 0;
    uf_rtsp_connection  *conn = NULL;

    if ((NULL == file)||(NULL == file->private))
    {
        return UF_RTSP_PARAM_ERROR;
    }

    conn = (uf_rtsp_connection*)file->private;
    if (NULL == conn->resp_buf)
    {
        return UF_RTSP_PARAM_ERROR ;
    }


    if (conn->write_idx == 0)
    {
        UF_RTSP_PRINT("%s:%d:no cmd data.\n",__FUNCTION__,__LINE__);
        return UF_RTSP_READCMDDATA_ERROR;
    }

    UF_RTSP_PRINT("parse=========\n%s.\n",conn->resp_buf);
    ulretcode = sscanf((char *)conn->resp_buf, "RTSP/%d.%d %d %50[^\015\012]", 
        &ulrtspver, &ulrtspsub,&ulrtspcode, (char *)acrtspstatus);
    
    if (4 != ulretcode) 
    {
        UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return UF_RTSP_READCMDDATA_ERROR;
    }

     if ((ulrtspcode < 200)||(ulrtspcode >= 300))
    {
        UF_RTSP_PRINT("%s:%d:%d error.\n",__FUNCTION__,__LINE__,ulrtspcode);
        return UF_RTSP_CMDCODE_ERROR;
    }

    switch(req_type)
    {
        case CURL_RTSPREQ_OPTIONS:
            // check if server is ms rtsp server
            if (NULL != strstr((char *)conn->resp_buf, "Server: WMSer"))
            {
                conn->rtsp_type = UF_RTSP_TYPE_MS;
            }
            else 
            {
                conn->rtsp_type = UF_RTSP_TYPE_STD;
            }
            break;
        case CURL_RTSPREQ_DESCRIBE:
            uf_rtsp_sdp_parse(file);

            break;
        case CURL_RTSPREQ_SETUP:
            break;
        case CURL_RTSPREQ_PLAY:
            break;
        case CURL_RTSPREQ_PAUSE:
            break;
        case CURL_RTSPREQ_TEARDOWN:
            break;
        case CURL_RTSPREQ_GET_PARAMETER:
            break;
        case CURL_RTSPREQ_SET_PARAMETER:
            break;
        case CURL_RTSPREQ_RECEIVE:
            break;
        default:
            break;
    }
    return UF_RTSP_OK;
}


static int uf_rtsp_interleave_data_write(void *data, unsigned int ulsize, unsigned int ulnmemb, void *userdata)
{
    UINT8 byte = 0;
    unsigned int ullength = 0;
    unsigned int ulwritesize = 0;
    __MAYBE_UNUSED__ unsigned int ulcountbuf = 0;
    //unsigned int ulcountfile = 0;
    UINT8 *pbyte = NULL;
    __MAYBE_UNUSED__ UINT32 len_offset = 0;
    urlfile *file = NULL;
    //uf_rtsp_connection *conn= NULL;
    UF_RTSP_USR_ACTION action = UF_USR_ACTION_NONE;
    
    file = (urlfile *)userdata;
    if ((NULL == file)||(NULL == file->private))
    {
        return 0;
    }
    
    action = uf_rtsp_get_user_cmd(file);
    if (UF_USR_ACTION_SEEK == action)
    {
        // if user seek, just return quickly.(need check, maybe no need proccess here.)
        return ulsize*ulnmemb;
    }
    
    pbyte = (UINT8*)data;
    byte = *(UINT8*)pbyte;

    ullength = 0;
    if (byte == '$')
    {
        pbyte += 16; //rtsp interleave header
        ullength += 16;
        byte = *(UINT8*)pbyte;
    }
    len_offset = MSRTP_LEN_OFFSET((pbyte+1));
    //libc_printf("%02X %02X %02X\n", *(pbyte+1), *(pbyte+2), *(pbyte+3));
    //libc_printf("RTP payload S %d L %d R %d D %d I %d  len/offset %d  rtpLen %d\n", 
    //    MSRTP_SBIT(byte), MSRTP_LBIT(byte), MSRTP_RBIT(byte), 
    //    MSRTP_DBIT(byte), MSRTP_IBIT(byte), len_offset, ulsize*ulnmemb);

    ullength += 4;
    if (MSRTP_RBIT(byte))
        ullength += 4;
    
    if (MSRTP_DBIT(byte))
        ullength += 4;
    
    if (MSRTP_IBIT(byte))
        ullength += 4;
    
    pbyte = (UINT8 *)data;
    pbyte = pbyte + ullength;
    ulwritesize = ulsize * ulnmemb - ullength;
    ulcountbuf = uf_store_data(file, (char *)pbyte, ulwritesize);

    return ulsize*ulnmemb;
}

static BOOL generate_udp_transport(track_media *track, char *transport, INT32 len)
{
    BOOL ret = FALSE;
    INT32 rtp_port, rtcp_port;
    
    ret = new_udp_port_pair(&rtp_port, &rtcp_port);
    snprintf(transport, len, "RTP/AVP;unicast;client_port=%d-%d", (int)rtp_port, (int)rtcp_port);
    track->rtp_type = UF_RTP_TYPE_UDP;
    track->port_ch[0] = rtp_port;
    track->port_ch[1] = rtcp_port;
    return ret;
}

static BOOL generate_interleave_transport(track_media *track, char *transport, INT32 len)
{
    BOOL ret = FALSE;
    INT32 ch1, ch2;

    ret = new_interleaved_ch_pair(&ch1, &ch2);
    snprintf(transport, len, "RTP/AVP/TCP;unicast;interleaved=%d-%d", (int)ch1, (int)ch2);
    track->rtp_type = UF_RTP_TYPE_INTERLEAVED;
    track->port_ch[0] = ch1;
    track->port_ch[1] = ch2;

    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_get_next_transport(urlfile *file, char *transport, UINT32 len)
{
    uf_rtsp_connection  *conn = NULL;
    track_media           *track = NULL;

    conn = (uf_rtsp_connection *)file->private;

    track = &conn->media_info.tracks[conn->media_info.setup_idx];
    
    if (UF_RTSP_TYPE_MS == conn->rtsp_type)
    {
        generate_interleave_transport(track, transport, len);
    }
    else
    {
        // first try UDP connection, if failure, try interleave next time.
        if (track->rtp_type == UF_RTP_TYPE_NONE)
        {
            generate_udp_transport(track, transport, len);
        }
        else
        {
            generate_interleave_transport(track, transport, len);
        }
    }

    return UF_RTSP_OK;
}

static UF_RTSP_RETURN_CODE uf_rtsp_get_next_track_url(urlfile *file, char *track_url, UINT32 len)
{
    int                 i = 0;
    uf_rtsp_connection  *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    i = conn->media_info.setup_idx;
    MEMSET(track_url,0,len);
    MEMCPY(track_url, file->url, STRLEN(file->url));
    strcat(track_url, "/");
    strcat(track_url, conn->media_info.tracks[i].name);
    UF_RTSP_PRINT("track_url:%s.\n",track_url);

    return UF_RTSP_OK;
}

static BOOL check_interleaved_connection(urlfile *file)
{
    uf_rtsp_connection  *conn = NULL;
    int                 i = 0;

    conn = (uf_rtsp_connection*)file->private;

    for (i=0; i<conn->media_info.track_num; i++)
    {
        if (conn->media_info.tracks[i].rtp_type == UF_RTP_TYPE_INTERLEAVED)
        {
            return TRUE;
        }
    }

    return FALSE;
}


/* uf_rtsp_progress_cb
 *
Use to exit anytime when user abort this session.
 */
static int uf_rtsp_progress_cb(void *pfile, double dltotal, double dlnow, double total, double now)
{
    UF_RTSP_USR_ACTION  action = UF_USR_ACTION_NONE;
    urlfile             *file = NULL;

    file = (urlfile *)pfile;

    action = uf_rtsp_get_user_cmd(file);
    if (UF_USR_ACTION_ABORT == action)
    {
        // abort current curl connection immediatly.
        return 1;
    }

    return 0;
}

static void uf_rtsp_curl_init(urlfile *file)
{
    
    if (file->curl == NULL)
    {
        file->curl = curl_easy_init();
    }

    curl_easy_reset(file->curl);
}

static void uf_rtsp_curl_setopt(urlfile *file, char *track_url, char *transport, char *range, long req_type)
{
    BOOL    interleaved = FALSE;

    if (CURL_RTSPREQ_SETUP == req_type)
    {
        curl_easy_setopt(file->curl, CURLOPT_URL, (UINT32)track_url);    
        curl_easy_setopt(file->curl, CURLOPT_RTSP_STREAM_URI, (UINT32)track_url);
        curl_easy_setopt(file->curl, CURLOPT_RTSP_TRANSPORT, (UINT32)transport);
    }
    else
    {
        curl_easy_setopt(file->curl, CURLOPT_URL, (UINT32)file->url);    
        curl_easy_setopt(file->curl, CURLOPT_RTSP_STREAM_URI, file->url);
    }
    curl_easy_setopt(file->curl, CURLOPT_USERAGENT, (UINT32)UF_RTSP_CLIENT_AGENT);
    curl_easy_setopt(file->curl, CURLOPT_FILE, file);
    curl_easy_setopt(file->curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(file->curl, CURLOPT_PROGRESSFUNCTION, uf_rtsp_progress_cb);
    curl_easy_setopt(file->curl, CURLOPT_PROGRESSDATA, (void *)file);
    curl_easy_setopt(file->curl, CURLOPT_WRITEFUNCTION, uf_rtsp_resp_buf_write);
    if (CURL_RTSPREQ_PLAY == req_type)
    {
        curl_easy_setopt(file->curl, CURLOPT_RANGE, (UINT32)range);
    }
    else
    {
        curl_easy_setopt(file->curl, CURLOPT_RANGE, (UINT32)NULL);
    }
    if ((CURL_RTSPREQ_PLAY == req_type) || (CURL_RTSPREQ_RECEIVE == req_type))
    {
        interleaved = check_interleaved_connection(file);
        if (interleaved)
        {
            curl_easy_setopt(file->curl, CURLOPT_INTERLEAVEFUNCTION, uf_rtsp_interleave_data_write);
            curl_easy_setopt(file->curl, CURLOPT_INTERLEAVEDATA, (void*)file);
        }
    }
    curl_easy_setopt(file->curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(file->curl, CURLOPT_RTSP_REQUEST, req_type);
}

/* uf_rtsp_cmd_option
 * RTSP Option command use to check which commands the server support,
 * and use as heartbeat in RTSP STD.
 */
static UF_RTSP_RETURN_CODE uf_rtsp_cmd_option(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);
    
    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_OPTIONS);
    
    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send option fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_OPTIONS);
f_exit:
    return ret;
}


/* uf_rtsp_cmd_describe
 * RTSP Describe command use to get stream's describe information
 */
static UF_RTSP_RETURN_CODE uf_rtsp_cmd_describe(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);
    
    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_DESCRIBE);

    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send describe fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_DESCRIBE);
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_setup(urlfile *file, char *track_url, char *transport)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);
    
    uf_rtsp_curl_setopt(file, track_url, transport, NULL, CURL_RTSPREQ_SETUP);

    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send setup fail, curl_ret %d\nURL:%s\n\n", __FUNCTION__, __LINE__, curl_ret, track_url);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_SETUP);
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_setup_tracks(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    uf_rtsp_connection *conn = NULL;
    track_media *track = NULL;
    int         count = 0;
    UINT16      i = 0;
    char        track_url[256] = {0};
    char        transport[64] = {0};

    conn = (uf_rtsp_connection *)file->private;

    if (NULL == conn)
    {
        UF_RTSP_PRINT("%s, L:%d, rtsp connection NULL\n", __FUNCTION__, __LINE__);
        goto f_exit;
    }
    
    count = conn->media_info.track_num;
    conn->media_info.setup_idx = 0;
    
    for (i=0; i<count; i++)
    {
        track = &conn->media_info.tracks[i];
        conn->media_info.setup_idx = i;
        uf_rtsp_get_next_track_url(file, track_url, 256);
        uf_rtsp_get_next_transport(file, transport, 64);
        ret = uf_rtsp_cmd_setup(file, track_url, transport);
        
        if (UF_RTSP_OK != ret)
        {
            UF_RTSP_PRINT("%s, L:%d, setup track error\n", __FUNCTION__, __LINE__);
            if (UF_RTP_TYPE_INTERLEAVED == track->rtp_type)
            {
                // can not play this stream.
                UF_RTSP_PRINT("%s, L:%d, CAN NOT PLAY TRACK\n", __FUNCTION__, __LINE__);
                goto f_exit;
            }

            // previous setup use RTP/UDP, fail; this time try interleaved/TCP
            uf_rtsp_get_next_transport(file, transport, 64); // generate interleaved transport.
            ret = uf_rtsp_cmd_setup(file, track_url, transport);
            
            if (UF_RTSP_OK != ret)
            {
                // can not play this stream.
                UF_RTSP_PRINT("%s, L:%d, CAN NOT PLAY TRACK\n", __FUNCTION__, __LINE__);
                goto f_exit;
            }
        }
        
        conn->state = UF_RTSP_SESSION_SETUPED;

        if (UF_RTP_TYPE_UDP == track->rtp_type)
        {
            // start up RTP task
            ret = uf_rtsp_rtp_task_create(track, uf_store_data);
            if (UF_RTSP_OK != ret)
            {
                UF_RTSP_PRINT("%s, L:%d create RTP task failed.\n",__FUNCTION__,__LINE__,ret);
                goto f_exit;
            }
        }
    }

f_exit:

    return ret;
}


static UF_RTSP_RETURN_CODE uf_rtsp_cmd_play(urlfile *file)
{
    uf_rtsp_connection *conn = NULL;
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode            curl_ret = CURLE_OK;
    char                range[32] = {0};

    conn = (uf_rtsp_connection *)file->private;
    
    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);

            
    snprintf(range, 31, "npt=%0.3f-", conn->media_info.range_play);
    uf_rtsp_curl_setopt(file, NULL, NULL, range, CURL_RTSPREQ_PLAY);

    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send CURL_RTSPREQ_PLAY fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_PLAY);
    
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_teardown(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_OK;
    __MAYBE_UNUSED__ CURLcode curl_ret = CURLE_OK;

    
    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_TEARDOWN);

    // Yes, we don't care the return value here, just exit as soon as possible.
    curl_ret = curl_easy_perform(file->curl);

    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_recieve(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_RECEIVE);

    //UF_RTSP_PRINT("feedback recieved\n");
    curl_ret = curl_easy_perform(file->curl);
    //UF_RTSP_PRINT("feedback finished\n");
    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send CURL_RTSPREQ_RECEIVE fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = UF_RTSP_OK;
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_pause(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);
    
    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_PAUSE);

    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send CURL_RTSPREQ_PAUSE fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_PAUSE);
    
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_cmd_set_parameter(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    CURLcode curl_ret = CURLE_OK;

    // Reset response buffer.
    uf_rtsp_resp_buf_reset(file);
    
    uf_rtsp_curl_setopt(file, NULL, NULL, NULL, CURL_RTSPREQ_SET_PARAMETER);

    curl_ret = curl_easy_perform(file->curl);

    if (CURLE_OK != curl_ret)
    {
        UF_RTSP_PRINT("%s,L:%d, send CURL_RTSPREQ_SET_PARAMETER fail, curl_ret %d\n", __FUNCTION__, __LINE__, curl_ret);
        goto f_exit;
    }

    ret = uf_rtsp_resp_buf_parse(file, CURL_RTSPREQ_SET_PARAMETER);
    
f_exit:
    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_send_heartbeat(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    if (UF_RTSP_TYPE_MS == conn->rtsp_type)
    {
        ret = uf_rtsp_cmd_set_parameter(file);
    }
    else
    {
        ret = uf_rtsp_cmd_option(file);
    }
    
    conn->last_heartbeat_time = osal_get_tick();


    return ret;
}

static BOOL check_heartbeat_time(urlfile *file)
{
    BOOL    ret = FALSE;
    UINT32  tick = 0;
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection*)file->private;

    tick = osal_get_tick();
    if ((tick - conn->last_heartbeat_time) > UF_RTSP_HEARTBEAT_TIME)
    {
        ret = TRUE;
    }
    
    return ret;
}

static BOOL uf_rtsp_check_seekable(urlfile *file)
{
    BOOL ret = FALSE;
    uf_rtsp_connection *conn = NULL;

    if (file)
    {
        conn = (uf_rtsp_connection*)file->private;

        if (conn->media_info.seekable)
        {
            ret = TRUE;
        }
    }

    return ret;
}

static UF_RTSP_RETURN_CODE uf_rtsp_session_maintain(urlfile *file)
{
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;
    UF_RTSP_USR_ACTION  action = UF_USR_ACTION_NONE;
    uf_rtsp_connection  *conn = NULL;
    BOOL    interleaved = FALSE;

    conn = (uf_rtsp_connection*)file->private;

    conn->state = UF_RTSP_SESSION_PLAYED;
    
    while (1)
    {
        // if user no action, return UF_USR_ACTION_NONE
        action = uf_rtsp_get_user_cmd(file);

        if (UF_USR_ACTION_ABORT == action)
        {
            break;
        }
        else if ((UF_USR_ACTION_NONE != action) && (UF_USR_ACTION_LAST != action))
        {
            if (UF_USR_ACTION_SEEK == action)
            {
                // Need PUASE first, then PLAY with range
                ret = uf_rtsp_cmd_pause(file);
                conn->state = UF_RTSP_SESSION_PAUSED;
                if (UF_RTSP_OK == ret)
                {
                    ret = uf_rtsp_cmd_play(file);
                    conn->state = UF_RTSP_SESSION_PLAYED;
                }
            }
            else if (UF_USR_ACTION_PAUSE == action)
            {
                // From PLAY to PAUSE
                if (UF_RTSP_SESSION_PLAYED == conn->state)
                {
                    ret = uf_rtsp_cmd_pause(file);
                    conn->state = UF_RTSP_SESSION_PAUSED;
                }
            }
            else if (UF_USR_ACTION_RESUME == action)
            {
                // From PAUSE to PLAY
                if (UF_RTSP_SESSION_PAUSED == conn->state)
                {
                    ret = uf_rtsp_cmd_play(file);
                    conn->state = UF_RTSP_SESSION_PLAYED;
                }
            }

            // clear user cmd after executed.
            uf_rtsp_set_user_cmd(file, UF_USR_ACTION_NONE);
            
            if (UF_RTSP_OK != ret)
            {
                UF_RTSP_PRINT("%s, L:%d maintain error\n", __FUNCTION__, __LINE__);

                // return from this function
                break;
            }
            continue;
        }
        
        if (check_heartbeat_time(file))
        {
            ret = uf_rtsp_send_heartbeat(file);
        }
        
        interleaved = check_interleaved_connection(file);
        
        if (interleaved)
        {
            ret = uf_rtsp_cmd_recieve(file);
        }
        else
        {
            osal_task_sleep(100);
        }
    }

    return ret;
}

static void uf_rtsp_rtp_data_task(track_media *track, rtp_write_data write_cb)
{
    UINT16 cur_seq_num = 0;
    UINT16 last_seq_num = 0;
    int ret = -1;
    __MAYBE_UNUSED__ UINT32 write_cnt = 0;
    UINT32 blockflag = 1;
    UINT32 read_size = 0;
    INT32  rtp_sock = -1;
    fd_set rfd;
    UINT8 *rtp_data_buf = NULL;
    struct timeval timeout;    
    struct sockaddr_in addr;
    socklen_t addr_size = 0;
    uf_rtp_header *rtp_header = NULL;
    
    if ((NULL == track) || (NULL == write_cb))
    {
        UF_RTSP_PRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return;
    }

    track->state = UF_RTP_TASK_RUNNING;
    UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, track->state);

    rtp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == rtp_sock)
    {
        UF_RTSP_PRINT("%s:%d create socket error.\n",__FUNCTION__,__LINE__);
        return;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(track->port_ch[0]);
    if (-1 == bind(rtp_sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        closesocket(rtp_sock);
        rtp_sock = -1;
        UF_RTSP_PRINT("%s:%d bind error.\n",__FUNCTION__,__LINE__);
        return;
    }
    ioctlsocket(rtp_sock, FIONBIO, &blockflag);
    
    rtp_data_buf = (UINT8*)MALLOC(UF_RTP_RECEIVEBUF_SIZE);
    if (NULL == rtp_data_buf)
    {
        UF_RTSP_PRINT("%s, malloc rtp buffer fail.\n", __FUNCTION__);
        closesocket(rtp_sock);
        rtp_sock = -1;
        return ;
    }
    while ((track->state > UF_RTP_TASK_NONE) && (track->state < UF_RTP_TASK_STOP))
    {                
        timeout.tv_sec = 0;
        timeout.tv_usec = 500*1000;
        FD_ZERO(&rfd);
        FD_SET(rtp_sock, &rfd);
        
        ret = select(rtp_sock+1, &rfd, 0, 0, &timeout);
        if (ret > 0)
        {
            read_size = recvfrom(rtp_sock, rtp_data_buf, UF_RTP_RECEIVEBUF_SIZE,
                                 0, (struct sockaddr *)&addr, &addr_size);
            if (read_size <= 0)
            {
                UF_RTSP_PRINT("rtp no receive!\n");
                continue;
            }

            rtp_header = (uf_rtp_header *)rtp_data_buf;
            cur_seq_num = (unsigned short)(ntohs(rtp_header->seqnum));
            if (cur_seq_num != (last_seq_num+ 1))
            {
                UF_RTSP_PRINT("lost packet:%d,%d.\n",last_seq_num, cur_seq_num);
            }
            last_seq_num = cur_seq_num;

			write_cnt = write_cb(track->write_fp, (char *)&rtp_data_buf[12],read_size-12);
            //if ((read_size-12) != write_cnt)
            //{
            //    UF_RTSP_PRINT("store data error:%d.\n",write_cnt);
            //    osal_task_sleep(100);
            //} 
        }            
        else if (-1 == ret)
        {
            UF_RTSP_PRINT("error.\r\n");
            osal_task_sleep(20);
        }
        else if (0 == ret)
        {
            UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, track->state);
            UF_RTSP_PRINT("timeout.\r\n");
            osal_task_sleep(300);
        }
    }

    closesocket(rtp_sock);  

    track->state = UF_RTP_TASK_EXIT;
    UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, track->state);
    UF_RTSP_PRINT("RTP task exit task id[%d]\n", osal_task_get_current_id());
    FREE(rtp_data_buf);

    return;
}

static int uf_rtsp_rtp_task_create(track_media *track, rtp_write_data write_cb)
{
    OSAL_T_CTSK t_ctsk;
    OSAL_ID tsk_id;

    if ((NULL == write_cb)||(NULL == track))
    {
        UF_RTSP_PRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return UF_RTSP_PARAM_ERROR;
    }
    
    track->state = UF_RTP_TASK_CREATE;
    UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, track->state);
    
    t_ctsk.itskpri = OSAL_PRI_NORMAL;//OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = 0;//(UINT32)rsrc;
    t_ctsk.para2 = 0;//(UINT32)cb;
    t_ctsk.name[0] = 'R';
    t_ctsk.name[1] = 'T';
    t_ctsk.name[2] = 'P';
    t_ctsk.para1  = (UINT32)track;
    t_ctsk.para2  = (UINT32)write_cb;
    t_ctsk.task = (FP)uf_rtsp_rtp_data_task;
    tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == tsk_id)
    {
        track->state = UF_RTP_TASK_NONE;
        UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, track->state);
        UF_RTSP_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return UF_RTSP_CREATETASK_ERROR;
    }
    
    
    return UF_RTSP_OK;
}

static void uf_rtsp_rtp_data_task_stop(urlfile *file)
{
    UINT16 i = 0;
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    for (i=0; i<conn->media_info.track_num; i++)
    {
        if (UF_RTP_TASK_RUNNING == conn->media_info.tracks[i].state) 
        {
            conn->media_info.tracks[i].state = UF_RTP_TASK_STOP;
            UF_RTSP_PRINT("set track i=%d stop\n", i);
            while (UF_RTP_TASK_EXIT != conn->media_info.tracks[i].state) 
            {
                UF_RTSP_PRINT("%s L: %d, state = %d\n", __FUNCTION__, __LINE__, conn->media_info.tracks[i].state);
                UF_RTSP_PRINT("wait exit i = %d\n", i);
                osal_task_sleep(100);
            }
            conn->media_info.tracks[i].state = UF_RTP_TASK_NONE;
        }
    }

}

static void uf_rtsp_main_task(UINT32 param1, UINT32 param2)
{
    urlfile *file = NULL;
    uf_rtsp_connection *conn = NULL;
    UF_RTSP_RETURN_CODE ret = UF_RTSP_ERROR;

    file = (urlfile *)param1;
    
    if ((NULL == file) || (NULL == file->private))
    {
        UF_RTSP_PRINT("%s,L:%d parameter error, rtsp main task exit\n", __FUNCTION__, __LINE__);
        ASSERT(0);
        return;
    }

    conn = (uf_rtsp_connection*)file->private;

    conn->state = UF_RTSP_SESSION_CONNECTING;
    
    uf_rtsp_curl_init(file);
    
    ret = uf_rtsp_cmd_option(file);
    if (UF_RTSP_OK != ret)
    {
        UF_RTSP_PRINT("%s, L:%d, first OPTION fail\n", __FUNCTION__, __LINE__);
        goto f_exit;
    }

    ret = uf_rtsp_cmd_describe(file);
    if (UF_RTSP_OK != ret)
    {
        UF_RTSP_PRINT("%s, L:%d, describe fail\n", __FUNCTION__, __LINE__);
        goto f_exit;
    }
    
    if (UF_RTSP_TYPE_MS == conn->rtsp_type)
    {
        ret = uf_rtsp_get_ms_stream_head(file, (char *)conn->resp_buf);
        if (UF_RTSP_OK != ret)
        {
            UF_RTSP_PRINT("%s:%d.\n",__FUNCTION__,__LINE__);
            goto f_exit;
        }
    }
    if (uf_rtsp_check_seekable(file))
    {
        network_set_seek_type(2); // vod, set time seek
    }
    else
    {
        network_set_seek_type(0); // live, set disable seek
    }

    ret = uf_rtsp_cmd_setup_tracks(file);
    if (UF_RTSP_OK != ret)
    {
        UF_RTSP_PRINT("%s, L:%d, setup tracks fail\n", __FUNCTION__, __LINE__);
        goto f_exit;
    }

    ret = uf_rtsp_cmd_play(file);
    if (UF_RTSP_OK != ret)
    {
        UF_RTSP_PRINT("%s, L:%d, play fail\n", __FUNCTION__, __LINE__);
        goto f_exit;
    }

    // play success, let's maintain this session.
    ret = uf_rtsp_session_maintain(file);
    
f_exit:

    // stop RTP task
    uf_rtsp_rtp_data_task_stop(file);
    
    if ((conn->state > UF_RTSP_SESSION_CONNECTING)
    && (conn->state < UF_RTSP_SESSION_DISCONNECT))
    {
        ret = uf_rtsp_cmd_teardown(file);
    }

    if (UF_RTSP_OK != ret)
    {
        // set state to let transfer task exit
        uf_state(file, UF_ERROR);
    }

    conn->state = UF_RTSP_SESSION_DISCONNECT;
    
}

/* uf_rtsp_check_exit
 * Check RTSP main task is exit or not.
 */
static BOOL uf_rtsp_check_exit(urlfile *file)
{
    BOOL    ret = FALSE;
    uf_rtsp_connection *conn = NULL;

    conn = (uf_rtsp_connection *)file->private;

    if (UF_RTSP_SESSION_DISCONNECT == conn->state)
    {
        ret = TRUE;
    }
    
    return ret;
}

/* uf_rtsp_init
 * This callback will startup RTSP main Task, and won't return until play end or aborted.
 */
static int uf_rtsp_init(urlfile *file)
{
    OSAL_T_CTSK         t_ctsk;
    ID                  tskid = OSAL_INVALID_ID;
    int                 ret = CURLE_FAILED_INIT;
    __MAYBE_UNUSED__ uf_rtsp_connection  *conn = NULL;

    if (NULL != file->private)
    {
        return ret;
    }
    if (FALSE == uf_rtsp_param_init(file))
    {
        return ret;
    }

    conn = (uf_rtsp_connection *)file->private;
    // let's startup RTSP main task.
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x8000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)file;
    t_ctsk.para2 = 0;
    t_ctsk.name[0] = 'R';
    t_ctsk.name[1] = 'M';
    t_ctsk.name[2] = 'T';
    t_ctsk.task = (FP)uf_rtsp_main_task;
    tskid = osal_task_create(&t_ctsk);
    
    if (tskid == OSAL_INVALID_ID)
    {
        return ret;
    }

    // set this state to ensure can uf_store_data
    uf_state(file, UF_OPEN);
    file->size = -1;

    // now wait user abort or connection fail
    while (FALSE == uf_rtsp_check_exit(file))
    {
        if (UF_CLOSE == file->state)
        {
            // user abort
            uf_rtsp_set_user_cmd(file, UF_USR_ACTION_ABORT);
        }
        
        osal_task_sleep(100);
    }

    // Init always return FAIL to trigger _uf_trans_task exit, 
    // because when execute this line, RTSP play already teardown or fail.
    
    return ret; 
}


void uf_rtsp_release(urlfile *file)
{
    uf_rtsp_param_free(file);
    UF_RTSP_PRINT("%s\n\n", __FUNCTION__);
}


static int uf_rtsp_seek(urlfile *file, off_t offset, int fromwhere)
{
    BOOL    seekable = FALSE;
    INT32   ret = 0;
    uf_rtsp_connection *conn = NULL;
    
    if(file == NULL || file->private == NULL)
    {
        return ret;
    }

    if (fromwhere != 3)
    {
        ret = -1;
        return ret;
    }
    
    seekable = uf_rtsp_check_seekable(file);
    if (seekable == FALSE)
    {
        ret = -1;
        return ret;
    }

    conn = (uf_rtsp_connection*)file->private;

    rbuffer_clean(&file->buffer);

    conn->media_info.range_play = (double)offset;

    uf_rtsp_set_user_cmd(file, UF_USR_ACTION_SEEK);
    
    UF_RTSP_PRINT("%s, ret = %d\n", __FUNCTION__, ret);
    return ret;
}


static int uf_rtsp_ioctl(void *file, UINT32 cmd, UINT32 param)
{
    int ret = 0;
    uf_rtsp_connection  *conn = NULL;
    urlfile *uf = (urlfile *)file;

    if(uf == NULL || uf->private == NULL)
    {
        return ret;
    }
    conn = (uf_rtsp_connection*)(((urlfile *)file)->private);
    // Connect not success yet.
    if (conn->state < UF_RTSP_SESSION_SETUPED)
    {
        return 0;
    }
    
    switch (cmd)
    {
        case UF_IOCTL_MP_PAUSE:
            uf_rtsp_set_user_cmd(file, UF_USR_ACTION_PAUSE);
            break;
        case UF_IOCTL_MP_RESUME:
            uf_rtsp_set_user_cmd(file, UF_USR_ACTION_RESUME);
            break;
        case UF_IOCTL_GET_TOTALTIME:
            if (conn->media_info.range[1]>0.0)
            {
                *((INT32*)param) = (INT32)conn->media_info.range[1];
            }
            else
            {
                *((INT32*)param) = 0;
            }
            break;
        case UF_IOCTL_GET_FILESIZE:
            *((INT32*)param) = -1;
            break;
        case UF_IOCTL_GET_SEEKTYPE:
            *((UINT32 *)param) = conn->media_info.seekable ? 2 : 0;
            break;
        default:
            ret = -1;
            break;
    }
    
    return ret;
}

/* Because RTSP implement running in itself task, so here 
 * curl_retproc curl_progcb data_recved set to NULL.  */
ufprotocol uf_rtsp =
{
    .next = NULL,
    .name = "rtsp",
    .init = (void *)uf_rtsp_init,
    .release = (void *)uf_rtsp_release,
    .seek = (void *)uf_rtsp_seek,
    .ioctl = (void *)uf_rtsp_ioctl,
    .curl_retproc = NULL,
    .curl_progcb = NULL,
    .data_recved = NULL,
};

