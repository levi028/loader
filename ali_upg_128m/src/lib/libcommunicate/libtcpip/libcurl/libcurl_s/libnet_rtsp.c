#include <api/libfs2/stdio.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include "swapbuf.h"
#include "libnet_internal.h"
#include "libnet_rtsp.h"

#define LIBNET_RTSPDEBUG
INT32 g_libnet_rtsp_debug = 0;
#ifdef LIBNET_RTSPDEBUG
    #define LIBNET_RTSPPRINT if (g_libnet_rtsp_debug)libc_printf
#else
    #define LIBNET_RTSPPRINT(...) do{}while(0)
#endif

#define NTOH_NUM(DATA) (((DATA & 0x00FF) << 8) | ((DATA & 0xFF00) >> 8))

#define RTSP_TRACKBUF_SIZE    20
#define RTSP_URL_BUFSIZE 256
#define RTSP_CMD_BUFSIZE 12000
#define RTSP_CONNECT_TIME  30
#define RTP_RECEIVEBUF_SIZE 4096
#define RTP_REVEIVE_PORT    4670
#define RTSP_HEARTBEAT_TIME 10000
#define MSRTP_SBIT(byte) ((byte)&0x80)>>7
#define MSRTP_LBIT(byte) ((byte)&0x40)>>6
#define MSRTP_RBIT(byte) ((byte)&0x20)>>5
#define MSRTP_DBIT(byte) ((byte)&0x10)>>4
#define MSRTP_IBIT(byte) ((byte)&0x08)>>3
#define MSRTP_LEN_OFFSET(byte_addr) ((*byte_addr)<<16|(*(byte_addr+1))<<8|(*(byte_addr+2)))

typedef enum
{
    RTSP_OK = 0,
    RTSP_CURLINIT_ERROR,
    RTSP_MALLOC_ERROR,
    RTSP_PARAM_ERROR,
    RTSP_CREATETASK_ERROR,
    RTSP_GETINFO_ERROR,
    RTSP_CMDSPACE_ERROR,
    RTSP_CMDCODE_ERROR,
    RTSP_READCMDDATA_ERROR,
    RTSP_CMDDATA_NOFIND,
    RTSP_CONNECT_TIMEOUT,
    RTSP_OPTION_ERROR,
    RTSP_DESCRIBE_ERROR,
    RTSP_SETUP_ERROR,
    RTSP_PLAY_ERROR,
    RTSP_TEARDOWN_ERROR,

    RTSP_MAX_ERROR
}RTSP_RETURN_CODE;

typedef enum
{
    RTP_STATUS_CLOSE = 0,
    RTP_STATUS_OPEN,
    
    RTP_STATUS_MAX
}RTP_STATUS_CODE;

static INT32 gs_rtsp_udp_port = RTP_REVEIVE_PORT;
static INT32 gs_rtsp_interleave = 0;
static char *gs_rtsp_agent_str = "ALi RTSP Player.";
unsigned short g_ushortseqnum = 0;
unsigned int g_ulrtpstatus = RTP_STATUS_OPEN;
static rtspinfostr_type *gs_rtsp_info = NULL;
static rtpinfostr_type *gs_rtp_info = NULL;
unsigned int g_ulreponseflag = 0;

extern size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata);
extern void libnet_set_ydownLoad_status(int idx, DownLoadStatus status);
extern int libnet_get_ydownLoad_status(int slindex,unsigned int *pulstatus);
extern int libnet_param_free(Rsrc_t *p);
extern void libnet_set_net_dl_status(int idx, BOOL status);
extern CURLcode Curl_base64_decode(const char *src,
                            unsigned char **outptr, size_t *outlen);
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

static BOOL new_interleave_pair(INT32 *ch1, INT32 *ch2)
{
    BOOL ret = FALSE;

    if ((NULL == ch1) || (NULL == ch2))
    {
        return ret;
    }

    *ch1 = gs_rtsp_interleave++;
    *ch2 = gs_rtsp_interleave++;
    
    ret = TRUE;

    return ret;
}

static void rtsp_caculate_download_speed(rtspinfostr_type *pinfo)
{
    UINT32 cur_tick = 0;
    UINT32 dlcnt = 0;
    UINT32 byte_cnt;
    UINT32 time_diff;
    
    cur_tick = osal_get_tick();

    dlcnt = (UINT32)pinfo->prsrcinfo->yDataCnt;

    if (pinfo->prsrcinfo->speedInfo.last_speed_tick == 0) {
        pinfo->prsrcinfo->speedInfo.speed = 0;
        pinfo->prsrcinfo->speedInfo.last_data_cnt = dlcnt;
        pinfo->prsrcinfo->speedInfo.last_speed_tick = cur_tick;
    }
    else if (pinfo->prsrcinfo->speedInfo.last_data_cnt > dlcnt) {

        pinfo->prsrcinfo->speedInfo.speed = 0;
        pinfo->prsrcinfo->speedInfo.last_data_cnt = dlcnt;
        pinfo->prsrcinfo->speedInfo.last_speed_tick = cur_tick;
    }
    else {
        byte_cnt = dlcnt - pinfo->prsrcinfo->speedInfo.last_data_cnt;
        time_diff = cur_tick - pinfo->prsrcinfo->speedInfo.last_speed_tick;
        if (time_diff >= LIBNET_SPEED_CACULATE_TIME) {
            time_diff = time_diff/1000;
            pinfo->prsrcinfo->speedInfo.speed = byte_cnt/time_diff;
            pinfo->prsrcinfo->notify (NET_MSG_DOWNLOAD_SPEED, pinfo->prsrcinfo->speedInfo.speed);
            pinfo->prsrcinfo->speedInfo.last_data_cnt = dlcnt;
            pinfo->prsrcinfo->speedInfo.last_speed_tick = cur_tick;
        }
    }
}

rtspinfostr_type *libnetrtsp_getrtspinfo(void)
{
    return gs_rtsp_info;
}

int libnetrtsp_rtp_setstatus(unsigned int ulstatus)
{
    if (ulstatus >= RTP_STATUS_MAX)
    {
        return RTSP_PARAM_ERROR;
    }
    
    g_ulrtpstatus = ulstatus;
    return RTSP_OK;
}

int libnetrtsp_rtp_getstatus(unsigned int *pulstatus)
{
    if (NULL == pulstatus)
    {
        return RTSP_PARAM_ERROR;
    }

    *pulstatus = g_ulrtpstatus;
    return RTSP_OK;
}

rtpinfostr_type *libnetrtsp_rtp_getinfo(void)
{
    return gs_rtp_info;
}

int libnetrtsp_rtp_cleaninfo(void)
{
    rtpinfostr_type *pstrrtpinfo = gs_rtp_info; 
    
    if (NULL == pstrrtpinfo)
    {
        return RTSP_OK;
    }

    if (pstrrtpinfo->pcrtpbuf)
    {
        FREE(pstrrtpinfo->pcrtpbuf);
    }

    FREE(pstrrtpinfo);
    gs_rtp_info = NULL;

    return RTSP_OK;
}

int libnetrtsp_rtp_init()
{
    char *pctemp = NULL;
    rtpinfostr_type *pstrrtpinfo = NULL;

    libnetrtsp_rtp_setstatus(RTP_STATUS_OPEN);
    pstrrtpinfo = MALLOC(sizeof(rtspinfostr_type));
    if (NULL == pstrrtpinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pstrrtpinfo,0,sizeof(rtspinfostr_type));

    pctemp = MALLOC(RTP_RECEIVEBUF_SIZE);
    if (NULL == pctemp)
    {
        FREE(pstrrtpinfo);
        LIBNET_RTSPPRINT("%s:%d.\n",__FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pctemp,0,RTP_RECEIVEBUF_SIZE);
    pstrrtpinfo->ulrtpbufsize = RTP_RECEIVEBUF_SIZE;
    pstrrtpinfo->pcrtpbuf = pctemp;
    gs_rtp_info = pstrrtpinfo;

    return RTSP_OK;
}

void libnetrtsp_rtp_receivedata(sdp_media *track, libnetrtsp_writedata_callback pfwritedata_callback)
{
    unsigned short ushortseqnum = 0;
    int ret = -1;
    unsigned ulwritecount = 0;
    unsigned int ulrtpstatus = 0;
    unsigned int ulblockflag = 1;
    unsigned int ulreadsize = 0;
    int rtp_sock = -1;
    fd_set rfd;
    char *pcrtpdatabuf = NULL;
    unsigned int ulrtpdatabufsize = 0;
    rtpinfostr_type *pstrrtpinfo = NULL;
    struct timeval timeout;    
    struct sockaddr_in addr;
    socklen_t addr_size = 0;
    rtpheaderstr_type *pstrrtpheader = NULL;
    rtspinfostr_type *pinfo = NULL;
    
    if ((NULL == track) || (NULL == pfwritedata_callback))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return ;//RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return ;//RTSP_GETINFO_ERROR;
    }

    rtp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == rtp_sock)
    {
        LIBNET_RTSPPRINT("%s:%d create socket error.\n",__FUNCTION__,__LINE__);
        return;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(track->port_ch[0]);
    if (-1 == bind(rtp_sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        closesocket(rtp_sock);
        rtp_sock = -1;
        LIBNET_RTSPPRINT("%s:%d bind error.\n",__FUNCTION__,__LINE__);
        return;
    }
    ioctlsocket(rtp_sock, FIONBIO, &ulblockflag);

    pstrrtpinfo = libnetrtsp_rtp_getinfo();
    if (NULL == pstrrtpinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return;
    }

    ulrtpdatabufsize = pstrrtpinfo->ulrtpbufsize;
    pcrtpdatabuf = pstrrtpinfo->pcrtpbuf;
    libnet_set_ydownLoad_status(pinfo->prsrcinfo->idx,LIBNET_DOWNLOAD_START);
    
    while (1)
    {                
        libnetrtsp_rtp_getstatus(&ulrtpstatus);
        if (RTP_STATUS_OPEN == ulrtpstatus)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = 500*1000;
            FD_ZERO(&rfd);
            FD_SET(rtp_sock, &rfd);
            
            ret = select(rtp_sock+1, &rfd, 0, 0, &timeout);
            if (ret > 0)
            {
                ulreadsize = recvfrom(rtp_sock, pcrtpdatabuf,ulrtpdatabufsize,0,(struct sockaddr *)&addr,&addr_size);
                
                if (ulreadsize <= 0)
                {
                    LIBNET_RTSPPRINT("rtp no receive!\n");
                    continue;
                }

                //LIBNET_RTSPPRINT("read=0x%x.\r\n",ulreadsize);
                pstrrtpheader = (rtpheaderstr_type *)pcrtpdatabuf;
                ushortseqnum = (unsigned short)(NTOH_NUM(pstrrtpheader->seqnum));
                if (ushortseqnum != g_ushortseqnum + 1)
                {
                    LIBNET_RTSPPRINT("lost packet:%d,%d.\n",g_ushortseqnum,ushortseqnum);
                }
                g_ushortseqnum = ushortseqnum;

                ulwritecount = pfwritedata_callback(&pcrtpdatabuf[12],ulreadsize-12,1,pinfo->prsrcinfo);
                if ((ulreadsize-12) != ulwritecount)
                {
                    LIBNET_RTSPPRINT("pfwritedata error:%d.\n",ulwritecount);
                } 
                rtsp_caculate_download_speed(pinfo);
            }            
            else if (-1 == ret)
            {
                LIBNET_RTSPPRINT("error.\r\n");
                osal_task_sleep(2);
            }
            else if (0 == ret)
            {
                LIBNET_RTSPPRINT("timeout.\r\n");
            }
        }
        else
        {
            LIBNET_RTSPPRINT("%s:rtp closed.\r\n",__FUNCTION__);
            break;
        }
    }

    closesocket(rtp_sock);  

    LIBNET_RTSPPRINT("RTP task exit task id[%d]\n", osal_task_get_current_id());
    return;
}

void libnetrtsp_rtp_receivetask(sdp_media *track, libnetrtsp_writedata_callback pfwritedata_callback)
{
    unsigned int ulret = 0;

    ulret = libnetrtsp_rtp_init();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d error.\r\n",__FUNCTION__,__LINE__,ulret);
        return;
    }

    libnetrtsp_rtp_receivedata(track, pfwritedata_callback);
    libnetrtsp_rtp_cleaninfo();    
    return;
}

int libnetrtsp_rtp_taskinit(sdp_media *track, libnetrtsp_writedata_callback pfwritedata_callback)
{
    OSAL_T_CTSK t_ctsk;
    OSAL_ID tsk_id;

    if (NULL == pfwritedata_callback)
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }
    
    t_ctsk.itskpri = OSAL_PRI_NORMAL;//OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = 0;//(UINT32)rsrc;
    t_ctsk.para2 = 0;//(UINT32)cb;
    t_ctsk.name[0] = 'R';
    t_ctsk.name[1] = 'T';
    t_ctsk.name[2] = 'P';
    t_ctsk.para1  = (UINT32)track;
    t_ctsk.para2  = (UINT32)pfwritedata_callback;
    t_ctsk.task = (FP)libnetrtsp_rtp_receivetask;
    tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == tsk_id)
    {
        LIBNET_RTSPPRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CREATETASK_ERROR;
    }
    
    return RTSP_OK;
}

unsigned int libnetrtsp_progress_callback(void *fp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    UINT32 status;
    
    //libc_printf("progress callback:enter...\r\n");
    rtspinfostr_type *pinfo = (rtspinfostr_type*)fp;

    if (pinfo && pinfo->prsrcinfo)
    {
        libnet_get_ydownLoad_status(pinfo->prsrcinfo->idx,(unsigned int *)(&status));

        if (status == LIBNET_DOWNLOAD_ABORT)
        {
            LIBNET_RTSPPRINT("%s:abort.\n", __FUNCTION__);
            (void)libnetrtsp_rtp_setstatus(RTP_STATUS_CLOSE);

            return 1; // abort
        }
    }

    rtsp_caculate_download_speed(pinfo);

    return 0;
}

int libnetrtsp_strcat(char *pcdest,char *pcsource)
{
    char *pcdesttemp = NULL;
    
    if ((NULL == pcdest)||(NULL == pcsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pcdesttemp = pcdest;
    while (*pcdesttemp)
    {
        pcdesttemp++;
    }                               /* find end of dst */
    
    while(((*pcdesttemp++) = (*pcsource++))) ;       /* Copy src to end of dst */
    
    return (int)pcdest ;  
}

int libnetrtsp_resp_buf_reset(void)
{
    rtspinfostr_type *pstrrtspinfo = NULL;

    pstrrtspinfo = libnetrtsp_getrtspinfo();
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    MEMSET(pstrrtspinfo->pccmdbuf,0,pstrrtspinfo->ulcmdbufsize);
    pstrrtspinfo->ulcmdbufwritepos = 0;
    pstrrtspinfo->ulcmdbufreadpos = 0;

    return RTSP_OK;
}

unsigned int libnetrtsp_resp_buf_write(void *psourcedata, unsigned int ulsize, unsigned int ulnmemb, void *userdata)
{
    //unsigned int i = 0;    
    unsigned int uldatalen = 0;
    unsigned int ultotallen = 0;
    rtspinfostr_type *pstrrtspinfo = NULL;

    LIBNET_RTSPPRINT("%s:enter...\r\n",__FUNCTION__);
    pstrrtspinfo = libnetrtsp_getrtspinfo();
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }
    
    uldatalen = ulsize * ulnmemb;
    ultotallen = pstrrtspinfo->ulcmdbufwritepos + ulsize * ulnmemb;
    if (ultotallen > pstrrtspinfo->ulcmdbufsize)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDSPACE_ERROR;
    }

    MEMCPY(&(pstrrtspinfo->pccmdbuf[pstrrtspinfo->ulcmdbufwritepos]), psourcedata, uldatalen);
    pstrrtspinfo->ulcmdbufwritepos += uldatalen;
    
    LIBNET_RTSPPRINT("%s:leave...\r\n",__FUNCTION__);
    return uldatalen;
}

int libnetrtsp_resp_buf_find(char *pcstringfind,char *pcdest,unsigned int ullength)
{
    unsigned int ulactlength = 0;
    char *pcstartpos = NULL;
    char *pcendpos = NULL;
    rtspinfostr_type *pstrrtspinfo = NULL;

    if ((NULL == pcstringfind)||(NULL == pcdest))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pstrrtspinfo = libnetrtsp_getrtspinfo();
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    if (pstrrtspinfo->ulcmdbufwritepos == 0)
    {
        LIBNET_RTSPPRINT("%s:%d:no cmd data.\r\n",__FUNCTION__,__LINE__);
        return RTSP_READCMDDATA_ERROR;
    }

    pcstartpos = strstr(pstrrtspinfo->pccmdbuf,pcstringfind);
    if (NULL == pcstartpos)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }

    pcendpos = strstr(pcstartpos,"\r\n");
    if (NULL == pcstartpos)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }

    ulactlength = pcendpos - pcstartpos;
    if (ulactlength > (ullength - 1))
    {
        LIBNET_RTSPPRINT("%s:%d:%d:%d.\r\n",__FUNCTION__,__LINE__,ulactlength,ullength);
        return RTSP_READCMDDATA_ERROR;
    }

    MEMCPY(pcdest,pcstartpos,ulactlength);
    pcdest[ulactlength] = '\0';
    LIBNET_RTSPPRINT("%s:%d:%s.\r\n",__FUNCTION__,__LINE__,pcdest);
    
    return RTSP_OK;
}

BOOL libnetrtsp_check_seekable(rtspinfostr_type *pinfo)
{
    if (NULL == pinfo)
    {
        return FALSE;
    }

    return pinfo->media_info.seekable;
}

int libnetrtsp_interleave_data_write(void *psourcedata, unsigned int ulsize, unsigned int ulnmemb, void *userdata)
{
    UINT8 byte = 0;
    //unsigned char ucflag = 0;
    unsigned int ullength = 0;
    unsigned int ulwritesize = 0;
    unsigned int ulcountbuf = 0;
    //unsigned int ulcountfile = 0;
    UINT8 *pbyte = NULL;
    UINT32 len_offset = 0;
    rtspinfostr_type *pstrrtspinfo = NULL;

    
    //LIBNET_RTSPPRINT("%s:enter...\r\n",__FUNCTION__);
    pstrrtspinfo = libnetrtsp_getrtspinfo();
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    pbyte = (UINT8*)psourcedata;
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
    
    pbyte = (UINT8 *)psourcedata;
    pbyte = pbyte + ullength;
    ulwritesize = ulsize * ulnmemb - ullength;
    ulcountbuf = write_data(pbyte, 1, ulwritesize, pstrrtspinfo->prsrcinfo);

    return ulsize*ulnmemb;
}

static int libnetrtsp_ms_stream_header_write(void *psourcedata, unsigned int ulsize)
{
    unsigned int ulcountbuf = 0;
    //unsigned int ulcountfile = 0;
    rtspinfostr_type *pstrrtspinfo = NULL;

    LIBNET_RTSPPRINT("%s:enter...\r\n",__FUNCTION__);
    pstrrtspinfo = libnetrtsp_getrtspinfo();
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    libnet_set_ydownLoad_status(pstrrtspinfo->prsrcinfo->idx,LIBNET_DOWNLOAD_START);

    ulcountbuf = write_data(psourcedata,1,ulsize,pstrrtspinfo->prsrcinfo);
    if (ulsize != ulcountbuf)
    {
        LIBNET_RTSPPRINT("%s:countbuf=%d error.\r\n",__FUNCTION__,ulcountbuf);
        return RTSP_GETINFO_ERROR;
    }
    
    return ulsize;
}

static int libnetrtsp_get_ms_stream_head(char *pcharsource)
{
    unsigned int ulret = 0;
    unsigned int ulheadlen = 0;
    char *pcstartpos = NULL;
    char *pcendpos = NULL;
    char *pcheaddata = NULL;

    pcstartpos = strstr(pcharsource,"base64");
    if (NULL == pcstartpos)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }
    pcstartpos = pcstartpos + 7;

    pcendpos = strstr(pcstartpos,"\r\n");
    if (NULL == pcstartpos)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }
    LIBNET_RTSPPRINT("%s:%d,%d.\r\n",__FUNCTION__,*pcstartpos,*pcendpos);
    *pcendpos = '\0';

    ulret = Curl_base64_decode(pcstartpos,(unsigned char **)(&pcheaddata),(size_t *)(&ulheadlen));
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }

    ulret = libnetrtsp_ms_stream_header_write(pcheaddata,ulheadlen);
    if (ulheadlen != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_CMDDATA_NOFIND;
    }

    return RTSP_OK;
}

static BOOL rtsp_sdp_parse(void)
{
    char *psdp = NULL;
    char *ptmp = NULL;
    char strtmp[32] = {0};
    INT32 i = 0;
    rtspinfostr_type *pinfo = NULL;

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    psdp = (char*)pinfo->pccmdbuf;

AGAIN:
    psdp = strstr(psdp, "v=");
    if (NULL == psdp)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
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
        pinfo->media_info.range[0] = 0.0;
        pinfo->media_info.range[1] = 0.0;
        //libc_printf("#%s\n", ptmp);
        sscanf(ptmp, "%lf-%lf", &pinfo->media_info.range[0], &pinfo->media_info.range[1]);
        if (pinfo->media_info.range[1] > 0.0)
        {
            pinfo->prsrcinfo->dtimelength = pinfo->media_info.range[1];
            pinfo->media_info.seekable = TRUE;
        }
        else 
        {
            pinfo->media_info.seekable = FALSE;
        }
        pinfo->media_info.range[0] = 0.0; // always play from 0.0
        //libc_printf("Range %f-%f\n", pinfo->media_info.range[0], pinfo->media_info.range[1]);

        ptmp = strstr(ptmp, "\r\n");
        psdp = ptmp + 2;
    }

    pinfo->media_info.track_num = 0;
    memset(pinfo->media_info.tracks, 0, MAX_SDP_MEDIA_NUM*sizeof(sdp_media));
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

        while (*ptmp != ' ') 
        {
            strtmp[i++] = *ptmp;
            ptmp++;
        }

        i = pinfo->media_info.track_num;
        if (0 == strcmp(strtmp, "audio"))
        {
            pinfo->media_info.tracks[i].media_type = SDP_M_AUDIO;
        }
        else if (0 == strcmp(strtmp, "video"))
        {
            pinfo->media_info.tracks[i].media_type = SDP_M_VIDEO;
        }
        else if (0 == strcmp(strtmp, "application"))
        {
            pinfo->media_info.tracks[i].media_type = SDP_M_APPLICATION;
        }
        else 
        {
            libc_printf("Other media type: [%s]\n", strtmp);
        }

        // skip port
        ptmp++;
        while (*ptmp != ' ') ptmp++;
        // skip proto
        ptmp++;
        while (*ptmp != ' ') ptmp++;

        // get fmt
        ptmp++;
        sscanf(ptmp, "%d", (int *)(&pinfo->media_info.tracks[i].fmt));

        // get name
        psdp = ptmp;
        ptmp = strstr(psdp, "a=control:");
        if (ptmp)
        {
            ptmp += strlen("a=control:");
            sscanf(ptmp, "%s", pinfo->media_info.tracks[i].name);
        }

        if ((pinfo->media_info.track_num+1) >= MAX_SDP_MEDIA_NUM)
            break;
        
        pinfo->media_info.track_num++;
    };

    return TRUE;
}

int libnetrtsp_resp_buf_parse(long req_type)
{
    unsigned int ulrtspver = 0;
    unsigned int ulrtspsub = 0;
    unsigned int ulrtspcode = 0;
    char *acrtspstatus[50] = {0};
    unsigned int ulretcode = 0;
    rtspinfostr_type *pinfo = NULL;

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    if (pinfo->ulcmdbufwritepos == 0)
    {
        LIBNET_RTSPPRINT("%s:%d:no cmd data.\r\n",__FUNCTION__,__LINE__);
        return RTSP_READCMDDATA_ERROR;
    }

    LIBNET_RTSPPRINT("%s.\r\n",pinfo->pccmdbuf);
    ulretcode = sscanf(pinfo->pccmdbuf, "RTSP/%d.%d %d %50[^\015\012]", 
        &ulrtspver, &ulrtspsub,&ulrtspcode, (char *)acrtspstatus);
    
    if (4 != ulretcode) 
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_READCMDDATA_ERROR;
    }

     if ((ulrtspcode < 200)||(ulrtspcode >= 300))
    {
        LIBNET_RTSPPRINT("%s:%d:%d error.\r\n",__FUNCTION__,__LINE__,ulrtspcode);
        return RTSP_CMDCODE_ERROR;
    }

    switch(req_type)
    {
        case CURL_RTSPREQ_OPTIONS:
            // check if server is ms rtsp server
            if (NULL != strstr(pinfo->pccmdbuf, "Supported:"))
            {
                pinfo->rtsp_type = RTSP_TYPE_MS;
            }
            else 
            {
                pinfo->rtsp_type = RTSP_TYPE_STD;
            }
            break;
        case CURL_RTSPREQ_DESCRIBE:
            rtsp_sdp_parse();
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
    return RTSP_OK;
}

int libnetrtsp_send_setparameter(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write); 
    curl_easy_setopt(pcurl, CURLOPT_INTERLEAVEFUNCTION, libnetrtsp_interleave_data_write);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_GET_PARAMETER);

    ulret = curl_easy_perform(pcurl);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_TEARDOWN_ERROR;
    }

    return RTSP_OK;
    
}


int libnetrtsp_process_set_parameter(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
    }
    
    ulret = libnetrtsp_send_setparameter(pcurl,pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_SET_PARAMETER);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    } 

    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;}

int libnetrtsp_send_interleave_receive(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    //LIBNET_RTSPPRINT("%s:enter...\r\n",__FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write); 
    curl_easy_setopt(pcurl, CURLOPT_INTERLEAVEFUNCTION, libnetrtsp_interleave_data_write);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_RECEIVE);

    ulret = curl_easy_perform(pcurl);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d: ret = %d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_TEARDOWN_ERROR;
    }

    //LIBNET_RTSPPRINT("%s:leave...\r\n",__FUNCTION__);
    return RTSP_OK;
    
}

int libnetrtsp_send_teardown(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_TEARDOWN);

    ulret = curl_easy_perform(pcurl);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_TEARDOWN_ERROR;
    }

    return RTSP_OK;
}

int libnetrtsp_process_teardown(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
    }
    
    ulret = libnetrtsp_send_teardown(pcurl,pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_TEARDOWN);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    } 

    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;
}

static int libnetrtsp_send_pause(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, NULL);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write);  
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PAUSE);

    ulret = curl_easy_perform(pcurl);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_PLAY_ERROR;
    }

    return RTSP_OK;}

int libnetrtsp_process_pause(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
    }
    
    ulret = libnetrtsp_send_pause(pcurl,pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        //here donot return
        //return ulret;
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_PAUSE);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    } 

    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;}

static int libnetrtsp_send_play(CURL *pcurl,char *pcurlsource,char *pcrange, BOOL interleaved)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    if ((NULL == pcurl)||(NULL == pcurlsource)||(NULL == pcrange))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)pcrange);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write);  
    if (interleaved)
    {
        curl_easy_setopt(pcurl, CURLOPT_INTERLEAVEFUNCTION, libnetrtsp_interleave_data_write);
    }
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PLAY);

    ulret = curl_easy_perform(pcurl);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_PLAY_ERROR;
    }

    return RTSP_OK;
}


int libnetrtsp_process_play(CURL *pcurl,char *pcurlsource, double range_val)
{
    CURLcode ulret = 0;
    char range[64];
    char track_url[256] = {0};
    rtspinfostr_type *pinfo = NULL;
    BOOL interleaved = FALSE;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }
    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    MEMSET(track_url,0,256);
    MEMCPY(track_url,pcurlsource,STRLEN(pcurlsource));
    libnetrtsp_strcat(track_url, "/");  
    LIBNET_RTSPPRINT("playurl:%s\r\n",track_url);

    snprintf(range, 64, "npt=%0.3f-", range_val);

    if (RTSP_TYPE_MS == pinfo->rtsp_type)
    {
        interleaved = TRUE;
    }
    ulret = libnetrtsp_send_play(pcurl,track_url,range, interleaved);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_PLAY);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        libnet_set_ydownLoad_status(pinfo->prsrcinfo->idx,LIBNET_DOWNLOAD_ERROR);
        return ulret;
    }
    libnet_set_ydownLoad_status(pinfo->prsrcinfo->idx,LIBNET_DOWNLOAD_START);
    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;
}


int libnetrtsp_send_setup(CURL *pcurl,char *pcurlsource,char *pctransport)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, (UINT32)pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_USERAGENT, (UINT32)gs_rtsp_agent_str);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_TRANSPORT, (UINT32)pctransport);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SETUP);

    ulret = curl_easy_perform(pcurl);
    if (CURLE_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_SETUP_ERROR;
    }
    
    return RTSP_OK;
}

static BOOL generate_udp_transport(sdp_media *track, char *transport, INT32 len)
{
    BOOL ret = FALSE;
    INT32 rtp_port, rtcp_port;
    
    ret = new_udp_port_pair(&rtp_port, &rtcp_port);
    snprintf(transport, len, "RTP/AVP;unicast;client_port=%d-%d", (int)rtp_port, (int)rtcp_port);
    track->rtp_type = RTP_TYPE_UDP;
    track->port_ch[0] = rtp_port;
    track->port_ch[1] = rtcp_port;
    return ret;
}

static BOOL generate_interleave_transport(sdp_media *track, char *transport, INT32 len)
{
    BOOL ret = FALSE;
    INT32 ch1, ch2;

    ret = new_interleave_pair(&ch1, &ch2);
    snprintf(transport, len, "RTP/AVP/TCP;unicast;interleaved=%d-%d", (int)ch1, (int)ch2);
    track->rtp_type = RTP_TYPE_INTERLEAVED;
    track->port_ch[0] = ch1;
    track->port_ch[1] = ch2;

    return ret;
}

static INT32 rtsp_send_track_setup(CURL *pcurl, char* track_url, char *transport)
{
    INT32 ret = RTSP_SETUP_ERROR;
    
    ret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    ret = libnetrtsp_send_setup(pcurl,track_url,transport);
    if (RTSP_OK != ret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    ret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_SETUP);
    if (RTSP_OK != ret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }  

    return ret;
}

int libnetrtsp_process_setup(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    char track_url[256] = {0};
    char transport[64]={0};
    rtspinfostr_type *pinfo = NULL;
    INT32 i = 0;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }


    if (RTSP_TYPE_MS == pinfo->rtsp_type)
    {
        // MS RTSP usually use interleaved RTSP
        for (i=0; i<pinfo->media_info.track_num; i++)
        {
            if (pinfo->media_info.tracks[i].media_type == SDP_M_APPLICATION)
            {
                continue;
            }
            MEMSET(track_url,0,256);
            MEMCPY(track_url,pcurlsource,STRLEN(pcurlsource));
            libnetrtsp_strcat(track_url, "/");
            libnetrtsp_strcat(track_url, pinfo->media_info.tracks[i].name);
            LIBNET_RTSPPRINT("track_url:%s.\r\n",track_url);

            generate_interleave_transport(&pinfo->media_info.tracks[i], transport, 64);
            if (RTSP_OK != rtsp_send_track_setup(pcurl, track_url, transport))
            {
                LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
                return ulret;
            }
        }
    }
    else
    {
        // STD RTSP first use RTP/UDP to setup, if fail, then use interleaved
        for (i=0; i<pinfo->media_info.track_num; i++)
        {
            if ((pinfo->media_info.tracks[i].media_type == SDP_M_AUDIO)
            || (pinfo->media_info.tracks[i].media_type == SDP_M_VIDEO))
            {
                MEMSET(track_url,0,256);
                MEMCPY(track_url,pcurlsource,STRLEN(pcurlsource));
                libnetrtsp_strcat(track_url, "/");
                libnetrtsp_strcat(track_url, pinfo->media_info.tracks[i].name);
                LIBNET_RTSPPRINT("track_url:%s.\n",track_url);

                generate_udp_transport(&pinfo->media_info.tracks[i], transport, 64);
                if (RTSP_OK != rtsp_send_track_setup(pcurl, track_url, transport))
                {
                    LIBNET_RTSPPRINT("track_url:%s. UDP fail, try interleaved\n",track_url);
                    generate_interleave_transport(&pinfo->media_info.tracks[i], transport, 64);
                    if (RTSP_OK != rtsp_send_track_setup(pcurl, track_url, transport))
                    {
                        LIBNET_RTSPPRINT("%s:%d:%d.\n",__FUNCTION__,__LINE__,ulret);
                        return ulret;
                    }
                }
                else
                {
                    #if 1
                    ulret = libnetrtsp_rtp_taskinit(&pinfo->media_info.tracks[i], write_data);
                    if (RTSP_OK != ulret)
                    {
                        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
                        return ulret;
                    }
                    osal_task_sleep(10);
                    #endif
                    // currently just support one RTP track, so break here
                    break;
                }
            }
        }
    }


    
    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;
}

int libnetrtsp_send_describe(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;


    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }
    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)pcurlsource);    
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, pcurlsource);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);

    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_DESCRIBE);

    ulret = curl_easy_perform(pcurl);
    if (CURLE_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return RTSP_DESCRIBE_ERROR;
    }

    return RTSP_OK;
}

int libnetrtsp_process_describe(CURL *pcurl,char *pcurlsource)
{
    CURLcode ulret = 0;
    //char *pcfindstring = NULL;
    rtspinfostr_type *pinfo = NULL;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }
    
    ulret = libnetrtsp_send_describe(pcurl,pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_DESCRIBE);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.",__FUNCTION__,__LINE__,ulret);
        return ulret;
    } 

    if (RTSP_TYPE_MS == pinfo->rtsp_type)
    {
        ulret = libnetrtsp_get_ms_stream_head(pinfo->pccmdbuf);
        if (RTSP_OK != ulret)
        {
            LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
            return RTSP_READCMDDATA_ERROR;
        }
    }

    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;
}

int libnetrtsp_send_option(CURL *pcurl, char *url)
{
    CURLcode ulret = 0;
    rtspinfostr_type *pinfo = NULL;

    if ((NULL == pcurl)||(NULL == url))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    curl_easy_setopt(pcurl, CURLOPT_URL, (UINT32)url);    
    curl_easy_setopt(pcurl, CURLOPT_RTSP_STREAM_URI, url);
    curl_easy_setopt(pcurl, CURLOPT_TIMEOUT, RTSP_CONNECT_TIME);
    curl_easy_setopt(pcurl, CURLOPT_RANGE, (UINT32)NULL);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSFUNCTION, libnetrtsp_progress_callback);
    curl_easy_setopt(pcurl, CURLOPT_PROGRESSDATA, (void*)pinfo);
    curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(pcurl, CURLOPT_HEADER, 1);
    curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, libnetrtsp_resp_buf_write);
    curl_easy_setopt(pcurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_OPTIONS);

    ulret = curl_easy_perform(pcurl);
    if (CURLE_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        if (CURLE_COULDNT_CONNECT == ulret)
        {
            return RTSP_CONNECT_TIMEOUT;
        }
        return RTSP_OPTION_ERROR;
    }

    return RTSP_OK;
}

int libnetrtsp_process_option(CURL *pcurl,char *pcurlsource)
{
    unsigned int ulret = 0;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }
    
    ulret = libnetrtsp_send_option(pcurl,pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    ulret = libnetrtsp_resp_buf_parse(CURL_RTSPREQ_OPTIONS);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.\r\n",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }  

    LIBNET_RTSPPRINT("%s:leave...\n", __FUNCTION__);
    return RTSP_OK;
}

int libnetrtsp_cleanrtspinfo(void)
{
    rtspinfostr_type *pstrrtspinfo = gs_rtsp_info; 

    if (NULL == pstrrtspinfo)
    {
        return RTSP_OK;
    }

    if (pstrrtspinfo->pcurlsource)
    {
        FREE(pstrrtspinfo->pcurlsource);
    }

    if (pstrrtspinfo->pccmdbuf)
    {
        FREE(pstrrtspinfo->pccmdbuf);
    }

    if (pstrrtspinfo->pctrackbuf)
    {
        FREE(pstrrtspinfo->pctrackbuf);
    }

    FREE(pstrrtspinfo);
    gs_rtsp_info = NULL;

    return RTSP_OK;
}

int libnetrtsp_cleanrsrcsource(Rsrc_t *prsrcinfo)
{
    int status = -1;
    
    if (NULL == prsrcinfo)
    {
        return RTSP_OK;
    }

    libnet_rsc_lock(prsrcinfo->idx);

    libnet_get_ydownLoad_status(prsrcinfo->idx, &status);
    if (status != LIBNET_DOWNLOAD_NA) 
    {
        if ((NULL != prsrcinfo))
        {        
            libnet_param_free(prsrcinfo);
        }    
    }
    libnet_rsc_unlock(prsrcinfo->idx);

    return RTSP_OK;
}

int libnetrtsp_init(CURL *pcurl,Rsrc_t *prsrcinfo)
{
    char *pctemp = NULL;
    rtspinfostr_type *pstrrtspinfo = NULL;

    if ((NULL == pcurl)||(NULL == prsrcinfo))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pstrrtspinfo = MALLOC(sizeof(rtspinfostr_type));
    if (NULL == pstrrtspinfo)
    {
        LIBNET_RTSPPRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pstrrtspinfo,0,sizeof(rtspinfostr_type));

    pctemp = MALLOC(RTSP_CMD_BUFSIZE);
    if (NULL == pctemp)
    {
        FREE(pstrrtspinfo);
        LIBNET_RTSPPRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pctemp,0,RTSP_CMD_BUFSIZE);
    pstrrtspinfo->pccmdbuf = pctemp;
    pstrrtspinfo->ulcmdbufsize = RTSP_CMD_BUFSIZE;
    pstrrtspinfo->ulcmdbufreadpos = 0;
    pstrrtspinfo->ulcmdbufwritepos = 0;

    pctemp = MALLOC(RTSP_URL_BUFSIZE);
    if (NULL == pctemp)
    {
        FREE(pstrrtspinfo->pccmdbuf);
        FREE(pstrrtspinfo);      
        LIBNET_RTSPPRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pctemp,0,RTSP_URL_BUFSIZE);
    pstrrtspinfo->pcurlsource = pctemp;
    MEMCPY(pstrrtspinfo->pcurlsource, prsrcinfo->full_url, STRLEN(prsrcinfo->full_url));

    pctemp = MALLOC(RTSP_TRACKBUF_SIZE);
    if (NULL == pctemp)
    {
        FREE(pstrrtspinfo->pccmdbuf);
        FREE(pstrrtspinfo->pcurlsource);
        FREE(pstrrtspinfo);
        LIBNET_RTSPPRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return RTSP_MALLOC_ERROR;
    }
    MEMSET(pctemp,0,RTSP_TRACKBUF_SIZE);
    pstrrtspinfo->pctrackbuf = pctemp;

    pstrrtspinfo->pcurl = pcurl;
    pstrrtspinfo->prsrcinfo = prsrcinfo;
    gs_rtsp_info = pstrrtspinfo;

    gs_rtsp_interleave = 0;
    gs_rtsp_udp_port = RTP_REVEIVE_PORT;
    
    return RTSP_OK;
}

int libnetrtsp_maintain(CURL *pcurl,char *pcurlsource)
{
    unsigned int ulstatus = 0;
    unsigned int ulindex = 0;
    unsigned int ulret = 0;
    unsigned int uloldtick = 0;
    unsigned int ulnewtick = 0;
    rtspinfostr_type *pinfo = NULL;
    //INT32        i = 0;
    
    if ((NULL == pcurl)||(NULL == pcurlsource))
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\r\n",__FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }
    
    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    ulret = libnetrtsp_resp_buf_reset();
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d.",__FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    ulindex = pinfo->prsrcinfo->idx;

    uloldtick = osal_get_tick();
    BOOL heart_beat_flag = FALSE;
    while (1)
    {
        libnet_get_ydownLoad_status(ulindex,&ulstatus);

        if (ulstatus == LIBNET_DOWNLOAD_ABORT)
        {
            LIBNET_RTSPPRINT("%s:abort.\n", __FUNCTION__);
            libnetrtsp_rtp_setstatus(RTP_STATUS_CLOSE);
            break;
        }
        if (ulstatus == LIBNET_DOWNLOAD_SETSEEK)
        {
            if (libnetrtsp_check_seekable(pinfo))
            {
                libnetrtsp_process_pause(pcurl, pcurlsource);
                libnetrtsp_process_play(pcurl, pcurlsource, pinfo->prsrcinfo->dtimeoffset);
            }
            else
            {
                LIBNET_RTSPPRINT("Can not seek, ERROR\n");
                libnet_set_ydownLoad_status(ulindex,LIBNET_DOWNLOAD_ERROR);
            }
            continue;
        }
        heart_beat_flag = FALSE;
        ulnewtick = osal_get_tick();
        if (ulnewtick - uloldtick > RTSP_HEARTBEAT_TIME)
        {
            uloldtick = ulnewtick;
            heart_beat_flag = TRUE;
        }

        if (RTSP_TYPE_MS == pinfo->rtsp_type)
        {
            if (heart_beat_flag)
            {
                ulret = libnetrtsp_process_set_parameter(pcurl, pcurlsource);
                if (RTSP_OK != ulret)
                {
                    LIBNET_RTSPPRINT("%s:%d:%d error.\n", __FUNCTION__,__LINE__,ulret);
                }
                libc_printf("%s  send set parametter OK\n", __FUNCTION__);
            }
            ulret = libnetrtsp_send_interleave_receive(pcurl, pcurlsource);

            if (RTSP_OK != ulret)
            {
                LIBNET_RTSPPRINT("%s:%d:%d send receive error.\n", __FUNCTION__,__LINE__,ulret);
            }
        }
        else
        {
            if (heart_beat_flag)
            {
                ulret = libnetrtsp_send_option(pcurl, pcurlsource);
                if (RTSP_OK != ulret)
                {
                    LIBNET_RTSPPRINT("%s:%d:%d error.\n", __FUNCTION__,__LINE__,ulret);
                }
            }
        }

    }

    ulret = libnetrtsp_process_teardown(pcurl, pcurlsource);
    if (RTSP_OK != ulret)
    {
        LIBNET_RTSPPRINT("%s:%d:%d error.\n", __FUNCTION__,__LINE__,ulret);
        return ulret;
    }

    libnet_set_ydownLoad_status(pinfo->prsrcinfo->idx,LIBNET_DOWNLOAD_FINISH);

    return RTSP_OK;
}

int libnetrtsp_startrtsp(CURL *pcurl)
{
    unsigned int ulret = 0;
    rtspinfostr_type *pinfo = NULL;
    
    if (NULL == pcurl)
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    pinfo = libnetrtsp_getrtspinfo();
    if (NULL == pinfo)
    {
        LIBNET_RTSPPRINT("%s:%d.\r\n",__FUNCTION__,__LINE__);
        return RTSP_GETINFO_ERROR;
    }

    ulret = libnetrtsp_process_option(pinfo->pcurl,pinfo->pcurlsource);
    if (RTSP_OK != ulret)
    {
        goto ERROR_EXIT;
    }

    ulret = libnetrtsp_process_describe(pinfo->pcurl,pinfo->pcurlsource);
    if (RTSP_OK != ulret)
    {
        goto ERROR_EXIT;
    }

    if (libnetrtsp_check_seekable(pinfo))
    {
        network_set_seek_type(2); // vod, set time seek
    }
    else
    {
        network_set_seek_type(0); // live, set disable seek
    }
    
    ulret = libnetrtsp_process_setup(pinfo->pcurl,pinfo->pcurlsource);
    if (RTSP_OK != ulret)
    {
        goto ERROR_EXIT;
    }

    ulret = libnetrtsp_process_play(pinfo->pcurl,pinfo->pcurlsource, 0.0);
    if (RTSP_OK != ulret)
    {
        goto ERROR_EXIT;
    }

    ulret = libnetrtsp_maintain(pinfo->pcurl,pinfo->pcurlsource);
    if (RTSP_OK != ulret)
    {
        goto ERROR_EXIT;
    }

    libnetrtsp_rtp_setstatus(RTP_STATUS_CLOSE);
    return RTSP_OK;

ERROR_EXIT:
    LIBNET_RTSPPRINT("%s:%d:%d error.\n", __FUNCTION__,__LINE__,ulret);

    (void)libnetrtsp_send_teardown(pinfo->pcurl,pinfo->pcurlsource);
    libnetrtsp_rtp_setstatus(RTP_STATUS_CLOSE);
    return ulret;
}

INT32 libnetrtsp_transfer(UINT32 param1,UINT32 param2)
{
    unsigned int ulret = 0;
    CURL *pcurl = NULL;
    Rsrc_t *prsrcinfo = NULL;

    LIBNET_RTSPPRINT("%s:enter...\n", __FUNCTION__);
    if (0 == param1)
    {
        LIBNET_RTSPPRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return RTSP_PARAM_ERROR;
    }

    prsrcinfo = (Rsrc_t *)param1;
    curl_global_init(CURL_GLOBAL_ALL);    
    pcurl = curl_easy_init();
    if (NULL == pcurl)
    {
        LIBNET_RTSPPRINT("curl_easy_init failed!\n");
        libnet_set_ydownLoad_status(prsrcinfo->idx, LIBNET_DOWNLOAD_ERROR);
        return RTSP_CURLINIT_ERROR;
    }

    ulret = libnetrtsp_init(pcurl,prsrcinfo);
    if (RTSP_OK != ulret)
    {
        goto RTSP_CHECK_EXIT;
    }

    ulret = libnetrtsp_startrtsp(pcurl);

RTSP_CHECK_EXIT:
    LIBNET_RTSPPRINT("%s:%d:%d.\n", __FUNCTION__,__LINE__,ulret);
    libnetrtsp_cleanrtspinfo();
    curl_easy_cleanup(pcurl);
    libnetrtsp_cleanrsrcsource(prsrcinfo);
    
    return ulret;
}


