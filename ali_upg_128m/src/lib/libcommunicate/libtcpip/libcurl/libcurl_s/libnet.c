
#include <api/libfs2/stdio.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libmp/pe.h>
#include "swapbuf.h"
#include "libnet_internal.h"
#include "libnet_rtsp.h"

#if 0
#define LIBNET_PRINTF    libc_printf
#else
#define LIBNET_PRINTF(...)    do{}while(0)
#endif
extern UINT8 network_is_disable_seek();
libnet_callback libnet_cb;
#define MAX_RSRC_NUM	8
#define DEFAULT_SWAP_BUF_SIZ 0x20000
#define NOR_TO			30
#define PE_TO			(60)
#define MAX_TRY_CONNECTION  3
#define URL_CONNECT_TIMEOUT 20 // seconds
__MAYBE_UNUSED__ static BOOL NET_DOWNLOAD_STATUS=FALSE;
static unsigned long u2b_play_video_handle = 0;
static int  gStop_Rtp=FALSE;;


static RsrcStatus_t tRsrcStatus[MAX_RSRC_NUM];// = {{LIBNET_DOWNLOAD_NA, 0, 0,0, 0, INVALID_ID}};
static BOOL ylibcurl_init = FALSE;
static UINT32 g_rtmp_livestream = FALSE;
static UINT32   url_timeout = 20;
static int   g_network_seek = 0;

void libnet_abort_url_read(BOOL abort);
void libnet_set_seek_option(char *url);
void libnet_network_seek(int enable);
static off_t get_content_size(Rsrc_t * tpRsrc);
static BOOL get_can_get_content_size();
static void set_can_get_content_size(BOOL bCanGet);
static SpeedInfo g_trans_speed = {0,0,0};

//////////
extern void network_set_url_content_size(char* url, off_t size);
extern void network_set_seek_type(UINT8 seek_type);
extern off_t network_get_url_content_size(char *url);
extern UINT8 network_get_seek_type();
//////////
int libnet_init(struct libnet_config *pcfg)
{
	libnet_cb = pcfg->cb;
	ylibcurl_init = FALSE;
    uf_protoinit();
    MEMSET(tRsrcStatus, 0, MAX_RSRC_NUM*sizeof(RsrcStatus_t));
#ifndef LIB_W5300	
	return network_init(pcfg);
#else
	return 0;
#endif	
}

INT32 libnet_rsc_lock(INT32 idx)
{
    INT32 ret = -1;

    if ((idx > 0) && (idx < MAX_RSRC_NUM) && (tRsrcStatus[idx].mutex != OSAL_INVALID_ID))
    {
        ret = osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
    }
    
    return ret;
}

INT32 libnet_rsc_unlock(INT32 idx)
{
    INT32 ret = -1;

    if ((idx > 0) && (idx < MAX_RSRC_NUM) && (tRsrcStatus[idx].mutex != OSAL_INVALID_ID))
    {
        ret = osal_mutex_unlock(tRsrcStatus[idx].mutex);
    }
    
    return ret;
}

void libnet_switch_callback(libnet_callback cb)
{
    libnet_cb = cb;
}

BOOL get_net_dl_status()
{
	return TRUE;
}

void set_net_dl_status(BOOL status)
{

}

BOOL libnet_get_net_dl_status(int idx)
{
    return tRsrcStatus[idx].NetDownLoadStatus;
}

void libnet_set_net_dl_status(int idx, BOOL status)
{
	//NET_DOWNLOAD_STATUS=status;
	tRsrcStatus[idx].NetDownLoadStatus = status;
}

int libnet_get_ydownLoad_status(unsigned int ulindex,unsigned int *pulstatus)
{
    if ((ulindex > 0)||(ulindex >= MAX_RSRC_NUM))
    {
        return -1;
    }

    if (NULL == pulstatus)
    {
        return -1;
    }

    //libc_printf("get status.\r\n");
    osal_mutex_lock(tRsrcStatus[ulindex].mutex, OSAL_WAIT_FOREVER_TIME);
    *pulstatus = tRsrcStatus[ulindex].yDownLoadStatus;
    osal_mutex_unlock(tRsrcStatus[ulindex].mutex);

    return 0;
}

void libnet_set_ydownLoad_status(int idx, DownLoadStatus status)
{
    if(idx >= 0 && idx < MAX_RSRC_NUM)
    {
        switch(status)
        {
            case LIBNET_DOWNLOAD_NA:
            case LIBNET_DOWNLOAD_INIT:
                tRsrcStatus[idx].yDownLoadStatus = status;
                break;
            case LIBNET_DOWNLOAD_START:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus <= LIBNET_DOWNLOAD_FINISH)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            case LIBNET_DOWNLOAD_DOWNLOADING:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus >= LIBNET_DOWNLOAD_START
                    && tRsrcStatus[idx].yDownLoadStatus <= LIBNET_DOWNLOAD_FINISH)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                
                break;
            case LIBNET_DOWNLOAD_WAITDATA:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus == LIBNET_DOWNLOAD_DOWNLOADING)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            case LIBNET_DOWNLOAD_SETSEEK:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus >= LIBNET_DOWNLOAD_START
                    && tRsrcStatus[idx].yDownLoadStatus <= LIBNET_DOWNLOAD_FINISH)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            case LIBNET_DOWNLOAD_FINISH:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus >= LIBNET_DOWNLOAD_START && tRsrcStatus[idx].yDownLoadStatus != LIBNET_DOWNLOAD_ABORT)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;                
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            case LIBNET_DOWNLOAD_ERROR:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus >= LIBNET_DOWNLOAD_START)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;                
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            case LIBNET_DOWNLOAD_ABORT:
                osal_mutex_lock(tRsrcStatus[idx].mutex, OSAL_WAIT_FOREVER_TIME);
                if(tRsrcStatus[idx].yDownLoadStatus >= LIBNET_DOWNLOAD_INIT)
                {
                    tRsrcStatus[idx].yDownLoadStatus = status;                
                }
                osal_mutex_unlock(tRsrcStatus[idx].mutex);
                break;
            default:
                break;
        }
        #if 0
        if (status == LIBNET_DOWNLOAD_ABORT)
        {
            libc_printf("%s:%d:%d:%d.\n", __FUNCTION__,idx, status, tRsrcStatus[idx].yDownLoadStatus);
        }
        #endif
    }
}

/* write_header
** Receive response header, every call by one line
**/
static size_t write_header(char *buffer,size_t size,size_t nitems,void *outstream)
{
    #define MAX_RESP_HEADER_LEN 256
    int k, resp_code;
    char header[MAX_RESP_HEADER_LEN];
    Rsrc_t * tpRsrc = (Rsrc_t*)outstream;// here use file pointer as outstream

    for (k=0; k<size*nitems && k<(MAX_RESP_HEADER_LEN-1); k++) {
        header[k] = buffer[k];
    }
    header[k] = '\0';

    //libc_printf("\n%s: %s\n",__FUNCTION__, header);
    if (strncmp(header, "HTTP/1", strlen("HTTP/1"))== 0) {
        // get response code
        for (k=0; k<size*nitems; k++) {
            if (header[k] == ' ') {
                sscanf(&header[k+1], "%d", &resp_code);
                tpRsrc->headerInfo.response_code = resp_code;
                break;
            }
        }
    }

    if (strncmp(header, "Location:", strlen("Location:")) == 0) {
        // yes, server response me new URL to get resource, we should replay with this URL
        if (tpRsrc->headerInfo.newLocation) {
            k = strlen("Location:");
            if (header[k] == ' ')
                k++;
            STRCPY((char *)tpRsrc->headerInfo.newLocation, &header[k]);
            k = strlen((char *)tpRsrc->headerInfo.newLocation);
            // remove \r\n or just \n
            if ((tpRsrc->headerInfo.newLocation[k-1] == '\n') || (tpRsrc->headerInfo.newLocation[k-1] == '\r'))
                tpRsrc->headerInfo.newLocation[k-1] = 0;
            if ((tpRsrc->headerInfo.newLocation[k-2] == '\n') || (tpRsrc->headerInfo.newLocation[k-2] == '\r'))
                tpRsrc->headerInfo.newLocation[k-2] = 0;
        }
    }
    if (strncmp(header, "Accept-Ranges: none", strlen("Accept-Ranges: none")) == 0) {
        // no seek
        tpRsrc->headerInfo.seek_info = 0;
    }
    if (strncmp(header, "Transfer-Encoding: chunked", strlen("Transfer-Encoding: chunked")) == 0) {
        // chunked, no content length
        tpRsrc->headerInfo.transfer_chunked = 1;
        tpRsrc->headerInfo.seek_info = 0; // yes, chunked transfer, no seek
    }

    // now check if get all header, if all header got, do some action.
    if (size*nitems <= 2 && (header[0] == '\n' || header[1] == '\n')) {
        // header end
        if (tpRsrc->headerInfo.transfer_chunked == 1) {
            tpRsrc->fileSize = 0x1fffffffffffffff;
            network_set_url_content_size(tpRsrc->full_url, 0x1fffffffffffffff);
        }
        if (tpRsrc->headerInfo.seek_info == 0) {
            tpRsrc->notify(NET_MSG_DOWNLOAD_INFO, NET_DOWNLOAD_INFO_NOSEEK);
        }
        if (tpRsrc->headerInfo.response_code == 301 || tpRsrc->headerInfo.response_code == 302) {
            // OK, now replay it.
            // I don't want add new command, here just reuse SEEK
            // If somebody findout some problem here, please add new command
            STRCPY(tpRsrc->full_url, (char *)tpRsrc->headerInfo.newLocation);
            tpRsrc->offset_newval = 0;
            libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_SETSEEK);
            //libc_printf("REPLAY NEW URL\n");
        }
    }

    return size*nitems;
}


/* write_data
** Receive data from server
**/
size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t ret = 0;
    Rsrc_t * tpRsrc = (Rsrc_t *)userdata;
    UINT8 *w_addr = NULL;
    UINT8 *tmp_ptr = (UINT8*)ptr;
    UINT32 w_cnt = 0, remain_data = size*nmemb;
    unsigned int index = 0;
    libnet_callback notify;
    long long param = 0;
    off_t last_yDataCnt;
    off_t _1_percent = 0;
    UINT32 try_cnt = 0;
    unsigned int ulseekinfo = 0xff;
    double  dtimelength = 0;
    off_t sllcurrentpos = 0;
    
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_ABORT
        ||tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_SETSEEK
        ||tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_NA)
    {
        LIBNET_PRINTF("write_data return because seek/abort [%d]\n", tRsrcStatus[tpRsrc->idx].yDownLoadStatus);
        ret = 0;
        goto exit_fn;
    }
    
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_WAITDATA) 
    {
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_DOWNLOADING);
        if (tpRsrc->notify) 
        {
            //libc_printf("NET_MSG_DOWNLOAD_RESUME \n");
            tpRsrc->notify(NET_MSG_DOWNLOAD_RESUME, 0);
        }

    }
    // secure check
    if ((NULL == ptr) || (NULL == tpRsrc))
    {
        ret = 0;
        goto ERR;
    }
    index = tpRsrc->idx;
    notify = libnet_cb;
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_START) {
        // set new state
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_DOWNLOADING);
        
        if ((NULL != tpRsrc->pCurl)
        && ((!MEMCMP(tpRsrc->full_url,"mms://",6))||(!MEMCMP(tpRsrc->full_url,"mmsh://",7))))
        {
            curl_easy_getinfo(tpRsrc->pCurl, CURLINFO_SEEK_INFO, &ulseekinfo);
            curl_easy_getinfo(tpRsrc->pCurl, CURLINFO_TIME_LENGTH, &dtimelength);
            curl_easy_getinfo(tpRsrc->pCurl, CURLINFO_CURRENT_POS, &sllcurrentpos);
            tpRsrc->dtimelength = dtimelength;
        }
		curl_easy_getinfo(tpRsrc->pCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &param);

        //tpRsrc->fileSize = -1;
        LIBNET_PRINTF("%s:%lld:%lld.\r\n",__FUNCTION__,param,get_content_size(tpRsrc));
        if (param < 0 && tpRsrc->fileSize == -1) 
        {
            tpRsrc->fileSize = param = get_content_size(tpRsrc);
        }
        else if (param > 0 && tpRsrc->fileSize == -1) {
	        tpRsrc->fileSize = tpRsrc->offset_newval + param;
        }

        if(tpRsrc->fileSize == -1 && MEMCMP(tpRsrc->full_url,"http",4)==0)
        {
            libnet_network_seek(0);
            libnet_set_seek_option(tpRsrc->full_url);
        }
		set_can_get_content_size(tpRsrc->fileSize > 0);

        if ((!MEMCMP(tpRsrc->full_url,"mms://",6))||(!MEMCMP(tpRsrc->full_url,"mmsh://",7)))
        {
            network_set_seek_type(ulseekinfo);
            tpRsrc->offset = sllcurrentpos;
        }
        
        tpRsrc->yDataCnt = 0;
		if (NULL != notify)
			notify(NET_MSG_DOWNLOAD_START, param);
			
        libnet_set_net_dl_status(index, TRUE);
        LIBNET_PRINTF("%s:seekinfo=%d,fileSize=%lld,time=%f,curpos=%lld.\n", __FUNCTION__, ulseekinfo,tpRsrc->fileSize,dtimelength,sllcurrentpos);
    }
    last_yDataCnt = tpRsrc->yDataCnt;
    _1_percent = tpRsrc->fileSize/100;

    if (tRsrcStatus[tpRsrc->idx].bUsePeCache) {
        // pe cache play media data, download to swap buffer
        do {
            w_cnt = 0;
            w_addr = NULL;
            try_cnt = 0;
            while (1) {
                swapbuf_write_request(tpRsrc->swapbuf, remain_data, &w_addr, &w_cnt);

                if (w_cnt > 0)
                    break;

                // maybe decoder seek again, or close/abort connection
                if (LIBNET_DOWNLOAD_ABORT == tRsrcStatus[index].yDownLoadStatus
                    || LIBNET_DOWNLOAD_SETSEEK == tRsrcStatus[index].yDownLoadStatus){
                    goto exit_fn;
                }
                osal_task_sleep(10);
                try_cnt++;
                libc_printf("F");
                if (try_cnt > 20) {
                    tpRsrc->notify(NET_MSG_DOWNLOAD_SPEED, 0);
                    try_cnt = 0;
                }
            }

            MEMCPY(w_addr, (UINT8*)tmp_ptr, w_cnt);
            swapbuf_write_update(tpRsrc->swapbuf, w_cnt);

            remain_data -= w_cnt;
            tmp_ptr += w_cnt;
            ret += w_cnt;
            tpRsrc->yDataCnt += w_cnt;
        } while (remain_data>0);
    }
    else {
        // download to mem buffer
        if (tpRsrc->buffer == NULL) {
            ret = -1;
            goto ERR;
        }
        if (size*nmemb <= (tpRsrc->buf_len - tpRsrc->yDataCnt)) {
            MEMCPY(tpRsrc->buffer+tpRsrc->yDataCnt, (UINT8*)ptr, size*nmemb);
            ret = size*nmemb;
            tpRsrc->yDataCnt += ret;
        }
        else if ((tpRsrc->buf_len - tpRsrc->yDataCnt)<=0){
            // mem buffer empty
            ret = -1;
            goto ERR;
        }
        else {
            // mem buffer not enough
            MEMCPY(tpRsrc->buffer+tpRsrc->yDataCnt, (UINT8*)ptr, (tpRsrc->buf_len - tpRsrc->yDataCnt));
            ret = (tpRsrc->buf_len - tpRsrc->yDataCnt);
            tpRsrc->yDataCnt += ret;
        }
    }

    if (NULL != notify && _1_percent > 0 && (tpRsrc->yDataCnt/_1_percent > last_yDataCnt/_1_percent))//send a msg every 1 percent
    {
        notify(NET_MSG_DOWNLOAD_PROGRESS, (UINT32)tpRsrc->yDataCnt);
    }
exit_fn:

ERR:
    return ret;
}
/**/

int libnet_param_free(Rsrc_t *p)
{
	//int i = 0;

    tRsrcStatus[p->idx].bUsed = 0;
	libnet_set_ydownLoad_status(p->idx, LIBNET_DOWNLOAD_NA);

    if (NULL != p->full_url)
    {
        FREE(p->full_url);
        FREE(p->headerInfo.newLocation);
        p->headerInfo.newLocation = NULL;
        p->full_url = NULL;
    }

    if (1 == tRsrcStatus[p->idx].bUsePeCache)
    {
        swapbuf_free(p->swapbuf);
        p->swapbuf = NULL;
        tRsrcStatus[p->idx].bUsePeCache = 0;
    }

    return 0;
}


//If the DMP/DMR can not get the length of file, it can not
//download the data from DMS(no matter enable/disable seek).
// so here we add the the patch: if DMS does not provide the
// length of file, we set the default file length, so that the
// libnet can download the file.  latter we should fixed that
//the libnet can download without the length of file.
#define DEFAULT_FILE_LENGTH 0x40000000 // 1G bytes


//if we can not get the network content length,
//so far we can not seek, so we set the "Range" http header
// to 0.
static BOOL m_if_can_get_content_size = TRUE;
static BOOL get_can_get_content_size()
{
    return m_if_can_get_content_size;
}
static void set_can_get_content_size(BOOL bCanGet)
{
    m_if_can_get_content_size = bCanGet;
}


static off_t get_content_size(Rsrc_t * tpRsrc)
{
    off_t fileSize = -1;
    
    if (tpRsrc->fileSize <= 0)
    {
        fileSize = network_get_url_content_size(tpRsrc->full_url);
        if (fileSize <= 0)
        {
            set_can_get_content_size(FALSE);
        }
        return fileSize;
    }

    return tpRsrc->fileSize;
}

off_t get_content_size_api(FILE *stream)
{
    return get_content_size((Rsrc_t *)stream);
}

INT32 get_content_duration_api(FILE *stream)
{
    Rsrc_t * tpRsrc = (Rsrc_t *)stream;

    if (NULL == tpRsrc)
    {
        return -1;
    }
    
    if (tpRsrc->dtimelength > 0.0)
    {
        return (INT32)tpRsrc->dtimelength;
    }
    return -1;
}

/*
*  This function change long long to string
*/
static void lld_to_string(char* display_buffer,off_t x)
{
    int i,j,mid;
    char temp;
    i=0;
    if(x <= 0)
    {
       display_buffer[0] = '0';
       return;
    }
    while(x)
    {
        display_buffer[i++]='0'+x%10;
        x/=10;
    }
    mid = strlen(display_buffer)/2;
    --i;
    for(j = 0;j<mid;++j)
    {
        temp = display_buffer[j];
        display_buffer[j] = display_buffer[i];
        display_buffer[i] = temp;
        --i;
    }
}

static int curl_progress_cb(void *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow)
{
    Rsrc_t * tpRsrc = (	Rsrc_t *)clientp;
    UINT32 cur_tick;
    UINT32 dlcnt = (UINT32)dlnow;
    UINT32 byte_cnt;
    UINT32 time_diff;
    
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_ABORT        
        || tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_SETSEEK
        || tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_NA)
        return 1; // abort waiting write data. libcurl will return imediatly.

    cur_tick = osal_get_tick();

    if (tpRsrc->speedInfo.last_speed_tick == 0) {
        tpRsrc->speedInfo.speed = 0;
        tpRsrc->speedInfo.last_data_cnt = dlcnt;
        tpRsrc->speedInfo.last_speed_tick = cur_tick;
    }
    else if (tpRsrc->speedInfo.last_data_cnt > dlcnt) {

        tpRsrc->speedInfo.speed = 0;
        tpRsrc->speedInfo.last_data_cnt = dlcnt;
        tpRsrc->speedInfo.last_speed_tick = cur_tick;
    }
    else {
        byte_cnt = dlcnt - tpRsrc->speedInfo.last_data_cnt;
        time_diff = cur_tick - tpRsrc->speedInfo.last_speed_tick;
        if (time_diff >= LIBNET_SPEED_CACULATE_TIME) {
            time_diff = time_diff/1000;
            tpRsrc->speedInfo.speed = byte_cnt/time_diff;
            //libc_printf("\ndownload speed %d\n", tpRsrc->speedInfo.speed);
            tpRsrc->notify (NET_MSG_DOWNLOAD_SPEED, tpRsrc->speedInfo.speed);
            tpRsrc->speedInfo.last_data_cnt = dlcnt;
            tpRsrc->speedInfo.last_speed_tick = cur_tick;
        }
    }
    return 0;
}

static int transfer(UINT32 param1, UINT32 param2)
{
    Rsrc_t * tpRsrc = NULL;
    CURL *curl = NULL;
    libnet_callback notify;
    int msg_code = 0;
    CURLcode ret = CURLE_OK;
    int try_conn_cnt = 0;
    unsigned int iUsePeCache = 0;
    long low_speed_time = 0;
    UINT8 ucseektype = 0xff;
    unsigned int ulseeklastflag = 0;
    long long sllcurrentpos = 0;

    char fromWhere[128];
    if (0 == param1)
    {
        LIBNET_PRINTF("Bad Parameter!\n");
        return -1;
    }
    tpRsrc = (Rsrc_t *)param1;
    notify = (libnet_callback)param2;
    low_speed_time = tpRsrc->yTransTime;

    iUsePeCache = tRsrcStatus[tpRsrc->idx].bUsePeCache;
    if (0 == iUsePeCache && NULL == tpRsrc->buffer)
    {
        // pe case, we use swap buffer, however, download to mem case,
        // user must config buffer to libnet
        LIBNET_PRINTF("Invalid Target Address!\n");
        ret = -1;
        goto check_exit;
    }

    LIBNET_PRINTF("Enter %s()\n", __FUNCTION__);
    curl = curl_easy_init();
    if (NULL == curl)
    {
        LIBNET_PRINTF("curl_easy_init failed!\n");
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_ERROR);
        ret = -1;
		goto check_exit;
    }

SEEK_LOOP:
    MEMSET(fromWhere,0,128*sizeof(char));
    curl_easy_setopt(curl, CURLOPT_URL, tpRsrc->full_url);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_header);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void*)tpRsrc);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_FILE, tpRsrc);
    // set Low Speed Limit and Low Speed Timeout to detect NetWork Exception:
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 512);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, low_speed_time); // 10 minutes timeout
    // set connection timeout
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, url_timeout);

    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, curl_progress_cb);

    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)tpRsrc);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);


    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
    curl_easy_setopt(curl,CURLOPT_SEEK_FLAG,0);
    curl_easy_setopt(curl,CURLOPT_SEEK_TIME,0);
    curl_easy_setopt(curl,CURLOPT_CURRENT_POS,&sllcurrentpos);

    curl_easy_setopt(curl, CURLOPT_RTMP_LIVESTREAM, g_rtmp_livestream);
    curl_easy_setopt(curl, CURLOPT_RTMP_TIMEOUT, url_timeout);
    
    // check if this request is SEEK or not
	ucseektype = network_get_seek_type();
    if ((0 !=ucseektype) && ((tpRsrc->offset != 0)||(tpRsrc->ultimeseekflag ==1)))
    {        
        if (1 == ucseektype)
        {
            if (get_can_get_content_size())
                lld_to_string(fromWhere,tpRsrc->offset);
            else
                lld_to_string(fromWhere,0);
            strcat(fromWhere,"-");
            curl_easy_setopt(curl, CURLOPT_RANGE, fromWhere);
        }
        else if (2 == ucseektype)
        {
            //"TimeSeekRange.dlna.org: npt=100";
            curl_easy_setopt(curl,CURLOPT_SEEK_FLAG,1);
            curl_easy_setopt(curl,CURLOPT_SEEK_TIME,tpRsrc->dtimeoffset);
        }
    }
    libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_START);
	LIBNET_PRINTF("Start Data Transfer: url:%s\n", tpRsrc->full_url);

    // now do request, and feed data to write_data
    try_conn_cnt = 0;
    tpRsrc->pCurl = curl;

    tpRsrc->debug_open_time = osal_get_tick();
    do 
    {
        try_conn_cnt++;
    	ret = curl_easy_perform(curl);
	} 
    while(ret == CURLE_COULDNT_CONNECT && try_conn_cnt < MAX_TRY_CONNECTION
        	&& tRsrcStatus[tpRsrc->idx].yDownLoadStatus != LIBNET_DOWNLOAD_NA 
        	&& tRsrcStatus[tpRsrc->idx].yDownLoadStatus != LIBNET_DOWNLOAD_ABORT);

	if(ret == CURLE_PARTIAL_FILE)
	{
		tpRsrc->yDataCnt = 0;
		swapbuf_reset(tpRsrc->swapbuf);
		curl_easy_reset(curl);
		goto SEEK_LOOP;
	}
	
    if (ret != CURLE_OK && LIBNET_DOWNLOAD_SETSEEK > tRsrcStatus[tpRsrc->idx].yDownLoadStatus) 
    {
        // perform return error not because seek
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_ERROR);

        if ((ret == CURLE_OPERATION_TIMEDOUT) || (ret == CURLE_RECV_ERROR))
        {
            libc_printf("NET_MSG_DOWNLOAD_NETERROR \n");
            tpRsrc->notify(NET_MSG_DOWNLOAD_NETERROR, 0);
        }
	}

    if (ret == CURLE_OK && (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_DOWNLOADING
						 || tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_WAITDATA))
    {
        // write data finished because download finish, not SEEK
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_FINISH);
    }
    
check_exit:
    // In pe cache play media case, we can not clean up connection
    // before url_close, because maybe user want to seek later.
    // disable seek case, here wait url_close
    if( 1 == iUsePeCache && tpRsrc->pCurl)
    {
        LIBNET_PRINTF("%s:%d:%d,%lld.\r\n",__FUNCTION__,ret,tRsrcStatus[tpRsrc->idx].yDownLoadStatus,tpRsrc->offset);
        while(LIBNET_DOWNLOAD_ABORT != tRsrcStatus[tpRsrc->idx].yDownLoadStatus)
        {
            if (LIBNET_DOWNLOAD_SETSEEK == tRsrcStatus[tpRsrc->idx].yDownLoadStatus) // need seek.
            {   
                tpRsrc->yDataCnt = 0;
                swapbuf_reset(tpRsrc->swapbuf);
                if (1 == ucseektype)
                {
                    tpRsrc->offset = tpRsrc->offset_newval;
                    if (tpRsrc->offset_newval == get_content_size(tpRsrc))
                    {
                        ulseeklastflag = 1;
                    }
                    else
                    {
                        ulseeklastflag = 0;
                    }
                }
                else if (2 == ucseektype)
                {
                    if ((off_t)tpRsrc->dtimeoffset == (off_t)tpRsrc->dtimelength)
                    {
                        ulseeklastflag = 1;
                    }
                    else
                    {
                        ulseeklastflag = 0;
                    }
                }

                if (ulseeklastflag == 1) 
                {
                    libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_START);
                    // if offset == fileSize, it means seek to file end, no need do new GET request to server.
                    // change status, finish url_seek's waiting.
                }
                else 
                {
                    // do real seek
                    curl_easy_reset(curl);
                    LIBNET_PRINTF("goto seekloop...\r\n");
                    goto SEEK_LOOP;
                }
            }
            // wait SEEK or CLOSE
            //LIBNET_PRINTF(">(%d)", osal_task_get_current_id());
            osal_task_sleep(100);
        }
     }   

	LIBNET_PRINTF("Data Transfer Completed! ret:%d\n", ret);
    // maybe connect error, resource already free here.
    if(tRsrcStatus[tpRsrc->idx].yDownLoadStatus != LIBNET_DOWNLOAD_NA)
    {
        if (tRsrcStatus[tpRsrc->idx].bUsePeCache
            &&((tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_ABORT && ret ==CURLE_WRITE_ERROR)
            || ret == CURLE_OK))
        {
            msg_code = 0;
        }
        else if(CURLE_OK != ret && CURLE_PARTIAL_FILE != ret)
        {
            msg_code = -NET_ERR_HTTP_SERVER_ERROR;
        }
        else 
        {
            msg_code = (int)tpRsrc->yDataCnt;
        }
        
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_FINISH);
    	libnet_set_net_dl_status(tpRsrc->idx, FALSE);

    	if ((NULL != notify)/* && CURLE_OPERATION_TIMEDOUT != ret*/)
    	{
            notify(NET_MSG_DOWNLOAD_PROGRESS, (UINT32)tpRsrc->yDataCnt);
    		notify(NET_MSG_DOWNLOAD_FINISH, (UINT32)msg_code);
    	}
    }

    osal_mutex_lock(tRsrcStatus[tpRsrc->idx].mutex, OSAL_WAIT_FOREVER_TIME);
    if(tpRsrc->pCurl)
    {
    	curl_easy_cleanup(curl);
        tpRsrc->pCurl = NULL;
    }
    // maybe connect error, resource already free here.
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus != LIBNET_DOWNLOAD_NA) 
    {
    	if ((NULL != tpRsrc))
    	{		
    		libnet_param_free(tpRsrc);
    		//FREE(tpRsrc);
    		//tpRsrc = NULL;
    	}	
    }
    osal_mutex_unlock(tRsrcStatus[tpRsrc->idx].mutex);

    //libc_printf("%s, exit task[%d]\n", __FUNCTION__, osal_task_get_current_id());
    return 0;
}

static int create_transfer_task(Rsrc_t *rsrc, libnet_callback cb)
{
	OSAL_T_CTSK t_ctsk;
	OSAL_ID tsk_id;
    TP pftaskentryfunc = NULL;

    libc_printf("%s:%s.\r\n",__FUNCTION__,rsrc->full_url);
    if (!MEMCMP(rsrc->full_url,"rtsp://",7))
    {
        pftaskentryfunc = (void *)libnetrtsp_transfer;
    }
    else
    {
        pftaskentryfunc = (void *)transfer;
    }
    
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.stksz = 0x8000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)rsrc;
	t_ctsk.para2 = (UINT32)cb;
	t_ctsk.name[0] = 'T';
	t_ctsk.name[1] = 'R';
	t_ctsk.name[2] = 'A';
	t_ctsk.task = (TP)pftaskentryfunc;
	tsk_id = osal_task_create(&t_ctsk);
//	LIBNET_PRINTF("tsk_id = %d\n", tsk_id);
	ASSERT(OSAL_INVALID_ID != tsk_id);

    return tsk_id;
}

static Rsrc_t * libnet_param_init(
    char *full_url,
    unsigned char* buffer,
    unsigned int buf_len,
    unsigned int offset,
    unsigned int use_pecache)
{
    int i = 0;
    Rsrc_t * tpRsrc = NULL;

    for (i = 0; i < MAX_RSRC_NUM; i++)
    {
        if (0 == tRsrcStatus[i].bUsed)
        {
            tRsrcStatus[i].bUsed = 1;
            tRsrcStatus[i].NetDownLoadStatus = FALSE;
            libnet_set_ydownLoad_status(i, LIBNET_DOWNLOAD_INIT);
            break;
        }
    }
    if (MAX_RSRC_NUM == i)
    {
        LIBNET_PRINTF("No more libnet resource!\n");
        return 0;
    }

    if(tRsrcStatus[i].mutex == 0)
    {
        if((tRsrcStatus[i].mutex = osal_mutex_create()) == OSAL_INVALID_ID)
        {
            return NULL;
        }
    }

    tpRsrc = &tRsrcStatus[i].tpRsrc;// (Rsrc_t *)MALLOC(sizeof(Rsrc_t));
    MEMSET(tpRsrc, 0x0, sizeof(Rsrc_t));
    tpRsrc->headerInfo.seek_info = 1; // default byte seek
    tpRsrc->full_url = MALLOC(1024);
    tpRsrc->headerInfo.newLocation = MALLOC(1024);
    if (NULL == tpRsrc->full_url)
    {
        tpRsrc = NULL;
        LIBNET_PRINTF("Memory malloc failed!\n");
        return NULL;
    }
    MEMSET(tpRsrc->full_url, 0x0, 1024);

    MEMCPY(tpRsrc->full_url, full_url, STRLEN(full_url));
    tpRsrc->buffer = buffer;  // cache buffer
    tpRsrc->buf_len = buf_len;
    tpRsrc->offset = offset;
    tpRsrc->offset_newval = offset;
    tpRsrc->idx = i;
    tpRsrc->yTransTime = NOR_TO;
    tpRsrc->yDataCnt = 0;
    tpRsrc->ultimeseekflag = 0;
    tpRsrc->dtimelength = 0.0;
    tpRsrc->dtimeoffset = 0.0;
    tpRsrc->fileSize = -1;
    tRsrcStatus[i].bUsePeCache = use_pecache;
    if (1 == tRsrcStatus[i].bUsePeCache)
    {
        // network media play case
        tpRsrc->yTransTime = PE_TO;//15 * 60;
        tpRsrc->swapbuf = swapbuf_new(DEFAULT_SWAP_BUF_SIZ);
    }

    return tpRsrc;
}

static int libcurl_init()
{
    if (FALSE == ylibcurl_init)
    {
        if(CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
        {
            LIBNET_PRINTF("CURL init failed!\n");
            return 0;
        }
        ylibcurl_init = TRUE;
    }

    return 1;
}

/* Full_URL is like "ftp://user:password@192.168.0.1/xxxx", user and password can be NULL*/
unsigned long libnet_download_to_mem(
    char *full_url,
    unsigned char* buffer,
    unsigned int buf_len,
    unsigned int offset)
{
	//int i = 0;
    Rsrc_t * tpRsrc = NULL;

    if (0 == libcurl_init())
        return 0;

    tpRsrc = libnet_param_init(full_url, buffer, buf_len, offset, 0);
    if (NULL == tpRsrc)
        return 0;
    tpRsrc->notify = libnet_cb;
    create_transfer_task(tpRsrc, libnet_cb);

    return (unsigned long)tpRsrc;

}

void libnet_download_abort(unsigned long handle)
{
    Rsrc_t *tpRsrc = (Rsrc_t*)handle;
    __MAYBE_UNUSED__ int idx = 0;

    if(handle == 0)
        return;

	idx = tpRsrc->idx;
    libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_ABORT);
}

unsigned long libnet_download_to_pecache(
    char *full_url,
    unsigned char* buffer,
    unsigned int buf_len,
    unsigned int offset)
{
    //int i = 0;
    Rsrc_t * tpRsrc = NULL;

    if (0 == libcurl_init())
        return 0;

	tpRsrc = libnet_param_init(full_url, buffer, buf_len, offset, 1);
	if (NULL == tpRsrc)
		return 0;
	tpRsrc->notify = libnet_cb;
	if(create_transfer_task(tpRsrc, libnet_cb) == OSAL_INVALID_ID)
	{
        libc_printf("crreat transfer task failed!!!!\n");
        libnet_param_free(tpRsrc);
        tpRsrc = NULL;
	}
	return (unsigned long)tpRsrc;
}

int url_connected(FILE *fp)
{
	Rsrc_t * tpRsrc = (Rsrc_t*)fp;
    return libnet_get_net_dl_status(tpRsrc->idx);
}

FILE *url_open(const char *path, unsigned char* buffer, unsigned int buf_len, unsigned int offset)
{
    FILE *tmp = NULL;
	__MAYBE_UNUSED__ UINT32 time_tick;
	__MAYBE_UNUSED__ BOOL bret =FALSE;
    char * url = NULL;
    Rsrc_t *tpRsrc = NULL;
    libnet_callback notify = NULL;
    // step 1: build a net Transfer-Thread
    //DLNA patch, just use DLNA dmp/dmr URL directory
#ifdef DLNA_SUPPORT
    url = dlnaPlayerGetURI();
#endif
    //
    if (NULL == url)    
    	url = (char *)path;
	/*tpRsrc*/ u2b_play_video_handle = libnet_download_to_pecache(url, buffer, buf_len, offset);
	if (0 != /*tpRsrc*/ u2b_play_video_handle)
	{
		tpRsrc = (Rsrc_t *)u2b_play_video_handle;
		//libc_printf("u2b: 0x%08x\n", tpRsrc);
	}
	else 
    {
		return NULL;
    }

    notify = tpRsrc->notify;
#if 0
	// step 2: wait the Transfer-Thread building status, if build OK, goto step3
	//			else, wait here
	time_tick = osal_get_tick();
	while(osal_get_tick()-time_tick < url_timeout*1000+100)
	{
		if(libnet_get_net_dl_status(tpRsrc->idx)) 
		{
			//url connet is ok, now write_data recieve 
			bret= TRUE;
			break;
		}
		osal_task_sleep(10);

        // maybe connect will fail very very soon
        if (LIBNET_DOWNLOAD_NA == tRsrcStatus[tpRsrc->idx].yDownLoadStatus
            || LIBNET_DOWNLOAD_ABORT == tRsrcStatus[tpRsrc->idx].yDownLoadStatus)
            goto exit_fn;
    }

    // step 3: Malloc a FILE pointer, return this pointer
    if(bret)
    {
		tmp = (FILE *)tpRsrc;
    }
    else
    {
        LIBNET_PRINTF("url_open timeout\n");
	    libnet_download_abort((UINT32)tpRsrc);
        do {
            // here wait transfer task free libnet param.
            osal_task_sleep(200);
        }while(LIBNET_DOWNLOAD_NA != tRsrcStatus[tpRsrc->idx].yDownLoadStatus);
        LIBNET_PRINTF("url_open fail\n");
	}
#else
	tmp = (FILE *)tpRsrc;
#endif

//exit_fn:
    if (tmp == NULL)
        notify(NET_MSG_DOWNLOAD_FINISH, -(NET_ERR_CONNECT_FAILED));
    return tmp;
}
size_t url_read(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = 0;
    UINT32 out_cnt = 0, index = 0;
    UINT8 *out_addr = NULL;
    __MAYBE_UNUSED__ int exit_line = 0;
    Rsrc_t * tpRsrc = (Rsrc_t*)stream;
	__MAYBE_UNUSED__ UINT32 try_cnt = 0;

    if (NULL == ptr)
    {
        //libc_printf("%s Invalid Input Parameter!\n", __FUNCTION__);
        exit_line = __LINE__;
        goto ERR;
    }

    if (0 == stream)
    {
        //libc_printf("Bad u2b play handle!\n");
        exit_line = __LINE__;
        goto ERR;
    }

	index = tpRsrc->idx;
	if (0 == tRsrcStatus[index].bUsed)
	{
		exit_line = __LINE__;
		goto ERR;
	}

    // if get_content_size(tpRsrc) == -1, means didn't get content length 
    // so read until disconnect
    if((-1 != get_content_size(tpRsrc)) 
        && (tpRsrc->offset >= get_content_size(tpRsrc))) {
        ret = 0;
        goto ERR;
    }

    swapbuf_read_request(tpRsrc->swapbuf, size*nmemb, &out_addr, &out_cnt);
    if (out_cnt!=0) {
        MEMCPY((UINT8*)ptr, out_addr, out_cnt);
        swapbuf_read_update(tpRsrc->swapbuf, out_cnt);
        ret = out_cnt;
    }

    tpRsrc->offset += ret;
ERR:
    if (ret == 0 && tRsrcStatus[index].yDownLoadStatus == LIBNET_DOWNLOAD_DOWNLOADING) {
        //libc_printf("%s, no data, ret = 0, tpRsrc->yDataCnt = %lld", __FUNCTION__, tpRsrc->yDataCnt);
        libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_WAITDATA);
        if (tpRsrc->notify) {
            //libc_printf("NET_MSG_DOWNLOAD_WAITDATA \n");
            tpRsrc->notify(NET_MSG_DOWNLOAD_WAITDATA, 0);
        }
    }
    return ret;
}
/*
*/

size_t url_close(FILE * fp)
{
    Rsrc_t * tpRsrc;
    int idx;
    //libc_printf("===url_close===!\n");
    tpRsrc = (Rsrc_t *)fp;
    if (tpRsrc == NULL)
        return -1;

    idx = tpRsrc->idx;

	libnet_download_abort((UINT32)tpRsrc);

	if (0 != u2b_play_video_handle)
	{
		do 
        {
		    // here wait transfer task free libnet param.
			osal_task_sleep(200);
		}while(LIBNET_DOWNLOAD_NA != tRsrcStatus[idx].yDownLoadStatus);
		//libnet_param_free(tpRsrc);
		//tpRsrc = NULL;
		u2b_play_video_handle = 0;
	}
	
	//libc_printf("===url_close Done===!\n");
	return 0;
}

int url_eof(FILE *stream)
{
    int ret = -1;
    UINT8 ucseektype = 0;
    Rsrc_t* tpRsrc = (Rsrc_t*)stream;
    
    if (0 == tpRsrc)
	{
		LIBNET_PRINTF("Bad u2b play handle!\n");
		ret = -1;
        goto exit_fn;
	}
  
    if(LIBNET_DOWNLOAD_NA == tRsrcStatus[tpRsrc->idx].yDownLoadStatus)
    {
        libc_printf("\n%s, eof0\n", __FUNCTION__);
        return 1;
    }

    ucseektype = network_get_seek_type();
    if (0 == ucseektype) 
    {
        // in disable seek case, when in status LIBNET_DOWNLOAD_FINISH, it finish really
        // LIBNET_DOWNLOAD_NA: means network error.
        if ((LIBNET_DOWNLOAD_FINISH == tRsrcStatus[tpRsrc->idx].yDownLoadStatus)
            || (tRsrcStatus[tpRsrc->idx].yDownLoadStatus >= LIBNET_DOWNLOAD_ERROR)) 
        {
            ret = 1;
        }
        else
            ret = 0;
    }
    else if (1 == ucseektype)
    {
        if ((tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_FINISH)
                || tRsrcStatus[tpRsrc->idx].yDownLoadStatus >= LIBNET_DOWNLOAD_ERROR)
        {
            // why here LIBNET_DOWNLOAD_ERROR: because in seekable case, maybe plugin seek to almost end
            ret = 1;
        }
        else 
        {
            ret = 0;
        }
    }
    else
    {
            ret = 0;
    }
    
exit_fn:
    #if 0
    if (ret == 1)
    {
      LIBNET_PRINTF("%s:%d:%d:%lld,%lld.\r\n",__FUNCTION__,ret,tRsrcStatus[tpRsrc->idx].yDownLoadStatus,tpRsrc->offset,get_content_size(tpRsrc));
    }
    #endif
    return ret;
}

off_t url_tell(FILE *stream)
{
    Rsrc_t *tpRsrc = (Rsrc_t *)stream;
    if (0 == tpRsrc)
    {
        LIBNET_PRINTF("Bad u2b play handle!\n");
        return -1;
    }

    LIBNET_PRINTF("url_tell: offset %lld\n",tpRsrc->offset);
    if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus >= LIBNET_DOWNLOAD_ERROR)
        return get_content_size(tpRsrc);

    return tpRsrc->offset;
}


INT32 url_seek(FILE *stream, off_t offset, int fromwhere)
{
    __MAYBE_UNUSED__ INT32 ret = 0;
    Rsrc_t* tpRsrc = (Rsrc_t *)stream;
    off_t fileSize = -1;
    UINT8 ucseektype = 0xff;
    if (0 == tpRsrc)
    {
        LIBNET_PRINTF("Bad u2b play handle!\n");
        return -1;
    }

    LIBNET_PRINTF("%s:%lld:%d.\r\n",__FUNCTION__,offset,fromwhere);
    ucseektype = network_get_seek_type();
    if (1 == ucseektype)
    {
        fileSize = tpRsrc->fileSize; //get_content_size(tpRsrc);

        if(fileSize < 0) return -1; //can not do seek.
        
        if (SEEK_SET == fromwhere) {
            tpRsrc->offset_newval = offset;
            if (tpRsrc->offset_newval > fileSize)
                tpRsrc->offset_newval = fileSize;
            libc_printf("SEEK_SET offset %lld\n", offset);
        }
        else if (SEEK_CUR == fromwhere) {
            tpRsrc->offset_newval = tpRsrc->offset + offset;
            if (tpRsrc->offset_newval > fileSize)
                tpRsrc->offset_newval = fileSize;
            libc_printf("SEEK_CUR offset %lld\n", offset);
        }
        else if (SEEK_END == fromwhere) {
            tpRsrc->offset_newval = fileSize - offset;
            if (tpRsrc->offset_newval < 0)
                tpRsrc->offset_newval = 0;
            libc_printf("SEEK_END offset %lld\n", offset);
        }
        else
        {
            ret = -1;
            goto exit_fn;
        }
    }
    else if (2 == ucseektype)
    {
        if (SEEK_SET== fromwhere)
        {
            if (offset > (off_t)tpRsrc->dtimelength)
            {
                offset = (off_t)tpRsrc->dtimelength;
            }
            tpRsrc->ultimeseekflag = 1;
            tpRsrc->dtimeoffset = (double)offset;
        }
        else
        {
            libc_printf("%s:%d:where=%d error.",__FUNCTION__,__LINE__,fromwhere);
            ret = -1;
            goto exit_fn;
        }
    }
    else
    {
        libc_printf("%s:%d:seektype=%d error.\n", __FUNCTION__,__LINE__,ucseektype);
        ret = -1;
        goto exit_fn;
    }
        
    libnet_set_ydownLoad_status(tpRsrc->idx, LIBNET_DOWNLOAD_SETSEEK);

    while(tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_SETSEEK)
    {
        if (tRsrcStatus[tpRsrc->idx].yDownLoadStatus == LIBNET_DOWNLOAD_ERROR)
            break;
        osal_task_sleep(5);
    }

exit_fn:
    return 0;
}

off_t url_filesize(FILE *stream)
{
    Rsrc_t * tpRsrc = (Rsrc_t*)stream;
    return tpRsrc->fileSize;
}

 void  libnet_set_stop_rtp(int value)
{
    gStop_Rtp= value;

}
 int libnet_get_stop_rtp(void)
{
  return gStop_Rtp;
}

void libnet_set_wait_flag(BOOL set)
{

}

void libnet_abort_url_read(BOOL abort)
{
	if (abort)
	{
		if (0 != u2b_play_video_handle)
			libnet_download_abort(u2b_play_video_handle);

	}
}

void libnet_set_url_timeout(UINT32 timeout)
{
    url_timeout = timeout;
}

int libnet_get_url_timeout(void)
{
   return url_timeout;
}

void libnet_rtmp_livestream(INT32 livestream)
{
    g_rtmp_livestream = (livestream?1:0);
}

void libnet_set_seek_option(char *url)
{
#if 0
    if(g_network_seek == 1)
    {
        //http
        if(MEMCMP(url, "http", 4) == 0)
        {
            video_set_disable_seek(0);
            music_engine_set_disable_seek(0);
            network_set_seek_type(1);
        }
        //mms
        else if(MEMCMP(url, "mms", 3) == 0)
        {
            video_set_disable_seek(1);
            network_set_seek_type(0);
        }
        //RTMP
        else if(MEMCMP(url, "rtmp", 4) == 0)
        {
            video_set_disable_seek(1);
            network_set_seek_type(2);
        }
        //rtsp
        else if(MEMCMP(url, "rtsp", 4) == 0)
        {
            video_set_disable_seek(1);
            network_set_seek_type(2);
        }
        //other uknow
        else
        {
            music_engine_set_disable_seek(1);
            video_set_disable_seek(1);
            network_set_seek_type(0);
        }
    }
    else if(g_network_seek == 0)
    {
        music_engine_set_disable_seek(1);
        video_set_disable_seek(1);
        network_set_seek_type(0);
    }
#else
    video_set_disable_seek(0);
    music_engine_set_disable_seek(0);
#endif
}

void libnet_network_seek(int enable)
{
    g_network_seek = enable;
}

void libnet_notify(UINT32 msg, UINT32 msgcode)
{
    if(libnet_cb) libnet_cb(msg, msgcode);
}

//calculate transfer speed and send speed info msg to App every 1s.
void libnet_calc_speed(UINT32 bytes)
{
    UINT32   speed = 0; //KByte/S
    DWORD tick = osal_get_tick();
    if (g_trans_speed.last_speed_tick == 0)
    {
        g_trans_speed.last_speed_tick = tick;
        g_trans_speed.last_data_cnt = bytes;
    }
    else
    {
        if (tick - g_trans_speed.last_speed_tick >= 1000)
        {
            g_trans_speed.last_data_cnt += bytes;
            speed = g_trans_speed.last_data_cnt/(tick - g_trans_speed.last_speed_tick);
            speed *= 1000;
            g_trans_speed.last_speed_tick = tick;
            g_trans_speed.last_data_cnt = 0;
            libnet_notify(NET_MSG_DOWNLOAD_SPEED, speed);
            //libc_printf("notify speed: %d\n", speed);
        }
        else
        {
            g_trans_speed.last_data_cnt += bytes;
        }
    }
}

#if 1 /* network performance test*/

#define DO_UPLOAD_TEST           // do upload test

#define SPEED_LIMIT          -1       //no speed limit
//#define SPEED_LIMIT        0x1000   //speed limit at 4K

__MAYBE_UNUSED__ static int up_limit_size = (100 * 0x100000);  //default: max upload size is 100MB

static UINT32 _trans_len_  = 0;
static UINT32 _trans_tick_ = 0;
static UINT32 _trans_cnt_  = 500 * 0x100000;

//for usb 3g burnning test, limit the download speed
static void speed_limit(int size)
{
    static UINT32 last_size = 0;
    static UINT32 last_time = 0;
    UINT32 cur_time = 0;

    //maybe restart download
    if (last_size > size || last_time == 0)
    {
        last_size = 0;
        last_time = osal_get_tick();
    }

    //check speed
    cur_time = osal_get_tick();

    if (size - last_size > SPEED_LIMIT && cur_time - last_time < 1000)
    {
        osal_task_sleep(1000 - (cur_time - last_time));
        //for next check
        last_size = size;
        last_time = osal_get_tick();
    }
}

static size_t _data_hdl(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (_trans_tick_ == 0) { _trans_tick_ = osal_get_tick(); }

    _trans_len_ += size * nmemb;

    if (_trans_len_ > _trans_cnt_) { return 0; }

    else { return size * nmemb; }
}

static size_t _data_hdl_null(void *ptr, size_t size, size_t nmemb, void *stream)
{



    return size * nmemb;
}
static int _trans_test(char *url, int upload)
{
    CURL *curl;
    __MAYBE_UNUSED__ CURLcode ret = CURLE_OK;
    UINT32 tick = 0;
    UINT32 speed = 0;


    ///////////////////////////////////////////////////////////////////////////
    //start upload test
    //libc_printf("start upload test: %s\n", tpRsrc->full_url);
    _trans_tick_ = _trans_len_ = 0;


    curl  = curl_easy_init();
    if (!curl) { return 0; }
    curl_easy_setopt(curl, CURLOPT_UPLOAD, upload ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, upload ? _data_hdl : _data_hdl_null);
    //curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)(200 * 0x100000));




    ///////////////////////////////////////////////////////////////////////////
    //start download test
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, upload ? _data_hdl_null : _data_hdl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
    libc_printf("\nStart %s Test ...\n", upload ? "Upload" : "Download");
    curl_easy_perform(curl);
    tick = osal_get_tick();
    curl_easy_cleanup(curl);

    if ((tick - _trans_tick_) > 1000) { speed = (_trans_len_ / 1024) / ((tick - _trans_tick_) / 1000); }
    libc_printf("\n%s Test Complete! => Size:%dKB, Speed:%dKB/s\n\n",
                upload ? "Upload" : "Download", _trans_len_ / 1024, speed);
    return 1;
    }
static int _trans_test_task(UINT32 param1, UINT32 param2)
    {
    Rsrc_t *tpRsrc = (Rsrc_t *)param1;
    libnet_callback notify = (libnet_callback)param2;
    _trans_test(tpRsrc->full_url, 1);
    _trans_test(tpRsrc->full_url, 0);
    if (notify) { notify(NET_MSG_DOWNLOAD_FINISH, 100); }
	return 1;

}

unsigned long libnet_perform_test(char *full_url, unsigned char *buffer,
                               unsigned int buf_len, unsigned int offset)
{
    //int i = 0;
    Rsrc_t *tpRsrc = NULL;
    OSAL_T_CTSK t_ctsk;
    OSAL_ID tsk_id;

    if (0 == libcurl_init())
    {
        return 0;
    }

    tpRsrc = libnet_param_init(full_url, buffer, buf_len, offset, 0);

    if (NULL == tpRsrc)
    {
        return 0;
    }

    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x8000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)tpRsrc;
    t_ctsk.para2 = (UINT32)libnet_cb;
    t_ctsk.name[0] = 'T';
    t_ctsk.name[1] = 'R';
    t_ctsk.name[2] = 'A';
    t_ctsk.task = (FP)_trans_test_task;
    tsk_id = osal_task_create(&t_ctsk);
    ASSERT(OSAL_INVALID_ID != tsk_id);
    return (UINT32)tpRsrc;
}

#endif

