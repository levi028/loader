/******************************************************************************
 * ALi (Zhuhai) Corporation, All Rights Reserved. 2014 Copyright (C)
 *
 * File: urlfile.c
 *
 * Description: -
 *     Download data from URL
 * History
 *       Date            Author             Version           Comment
 *     2014/02/18        Doy.Dong              1              create
 *
 ******************************************************************************/
#include "setup.h"
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>
#include <bus/dma/dma.h>
#define IPTVMODE(uf) (uf->mode[0]==0)

static ufprotocol *g_uf_support_proto = NULL;  /*supported protocol list*/
static int  uf_use_sgdma = 0;

/*uper case --> lower case*/
static void _uf_lowcase(char *url, char *proto, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        if (url[i] >= 'A' && url[i] <= 'Z') { proto[i] = url[i] + 32; }
        else { proto[i] = url[i]; }
    }
}

static void _uf_release(urlfile *file)
{
    if (file)
    {
        if (file->proto && file->proto->release) { file->proto->release(file); }
        if (file->curl) { curl_easy_cleanup(file->curl); }
        if (file->r_lock != OSAL_INVALID_ID) { osal_mutex_delete(file->r_lock); }
        if (file->w_lock != OSAL_INVALID_ID) { osal_mutex_delete(file->w_lock); }
        if (file->s_lock != OSAL_INVALID_ID) { osal_mutex_delete(file->s_lock); }
        if (file->p_lock != OSAL_INVALID_ID) { osal_mutex_delete(file->p_lock); }
        rbuffer_destroy(&file->buffer);
        if (file->buffer.rdma_id != DMA_INVALID_CHA)
        {
            osal_task_dispatch_off();
            dma_close(file->buffer.rdma_id);
            osal_task_dispatch_on();
        }
        if (file->buffer.wdma_id != DMA_INVALID_CHA)
        {
            osal_task_dispatch_off();
            dma_close(file->buffer.wdma_id);
            osal_task_dispatch_on();
        }
        FREE(file);
    }
}

static urlfile *_uf_create(char *url, char *mode)
{
    urlfile *file = NULL;
    ufprotocol *proto = NULL;

    if (STRLEN(url) > UF_URL_LEN - 1)
    {
        UF_DEBUG("URL is too long![%s]\n", url);
        return NULL;
    }

    /*match protocol*/
    proto = uf_findproto(url);
    if (proto == NULL)
    {
        libc_printf("**Can not match a protocol for (%s)\n", url);
    }

    file = MALLOC(sizeof(urlfile));
    if (file == NULL)
    {
        UF_DEBUG("malloc fail [%s]\n", url);
        return NULL;
    }

    MEMSET(file, 0, sizeof(urlfile));
    file->r_lock = OSAL_INVALID_ID;
    file->w_lock = OSAL_INVALID_ID;
    file->s_lock = OSAL_INVALID_ID;
    file->p_lock = OSAL_INVALID_ID;
    file->taskid = OSAL_INVALID_ID;

    file->r_lock = osal_mutex_create();
    file->w_lock = osal_mutex_create();
    file->s_lock = osal_mutex_create();
    file->p_lock = osal_mutex_create();
    if (file->r_lock == OSAL_INVALID_ID || file->w_lock == OSAL_INVALID_ID
            || file->s_lock == OSAL_INVALID_ID || file->p_lock == OSAL_INVALID_ID)
    {
        _uf_release(file);
        UF_DEBUG("osal_mutex_create fail [%s]\n", url);
        return NULL;
    }

    if (!rbuffer_init(&file->buffer, NULL, UF_BUFFER_LEN)) //128Kbyte buffer for each urlfile.
    {
        _uf_release(file);
        UF_DEBUG("rbuffer_init fail [%s]\n", url);
        return NULL;
    }

    file->size = 0; //default size is 0.
    if (mode) { strncpy(file->mode, mode, UF_MODE_LEN); }
    file->proto = proto;
    strncpy(file->url, url, UF_URL_LEN - 1);
    if (IPTVMODE(file) && uf_use_sgdma)
    {
        osal_task_dispatch_off();
        file->buffer.rdma_id = dma_open(DMA_CHAANY, 0, NULL);
        file->buffer.wdma_id = dma_open(DMA_CHAANY, 0, NULL);
        osal_task_dispatch_on();
        UF_DEBUG("rbuffer read  dma channle:%d\n", file->buffer.rdma_id);
        UF_DEBUG("rbuffer write dma channle:%d\n", file->buffer.wdma_id);
    }
    return file;
}

static void _uf_idle(urlfile *file)
{
    if (file->proto && file->proto->idle)
    {
        file->proto->idle(file);
    }
}

static int _uf_curl_debug(CURL *curl, curl_infotype itype, char *pData, size_t size, void *userdata)
{
    if (itype == CURLINFO_TEXT)
    {
        UF_DEBUG("[TEXT]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_IN)
    {
        UF_DEBUG("[HEADER_IN]%s\n", pData);
    }
    /*
    else if (itype == CURLINFO_HEADER_OUT)
    {
        UF_DEBUG("[HEADER_OUT]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_IN)
    {
        UF_DEBUG("[DATA_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_OUT)
    {
        UF_DEBUG("[DATA_OUT]%s\n", pData);
    }
    */
    return 0;
}

static size_t _uf_write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t ret = 0;
    size_t count = size * nmemb;
    urlfile *file = (urlfile *)userdata;

    if (file == NULL) { return 0; }
    if (file->proto && file->proto->data_recved)
    {
        ret = file->proto->data_recved(file, ptr, count);
        if (ret)
        {
            UF_DEBUG("%s: data has been received\n", __FUNCTION__);
            return ret;
        }
    }

    if (file->size == 0)
    {
        curl_easy_getinfo(file->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file->size);
    }
    ret = uf_store_data(file, ptr, count);
    return ret;
}

static void _curl_retproc(urlfile *file, CURLcode ret)
{
    if (file == NULL) { return; }

    if (file->proto && file->proto->curl_retproc)
    {
        file->proto->curl_retproc(file, ret);
    }
    else //default curl ret process.
    {
        UF_DEBUG("(%d) ret:%d\n", file->taskid, ret);
        switch (ret)
        {
            case CURLE_OK:
                uf_state(file, UF_DONE);
                break;
            case CURLE_ABORTED_BY_CALLBACK:
                break;
            case CURLE_WRITE_ERROR:
                break;
            default:
                uf_state(file, UF_ERROR);
                break;
        }
    }
}

static int _curl_progcb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int ret = 0;
    urlfile *file = (urlfile *)clientp;

    if (file->proto && file->proto->curl_progcb)
    {
        ret |= file->proto->curl_progcb(clientp, dltotal, dlnow, ultotal, ulnow);
    }
    ret |= (file->state < UF_NONE || file->state == UF_ROPEN);
    return ret;
}

static int _curl_init(urlfile *file)
{
    int ret = CURLE_OK;
    if (file == NULL || file->curl == NULL) { return 1; }

    /*set common parameters*/
    curl_easy_reset(file->curl);
    ret |= curl_easy_setopt(file->curl, CURLOPT_URL, file->url);
    ret |= curl_easy_setopt(file->curl, CURLOPT_FILE, file);
    ret |= curl_easy_setopt(file->curl, CURLOPT_NOPROGRESS, 0);
    ret |= curl_easy_setopt(file->curl, CURLOPT_PROGRESSFUNCTION, _curl_progcb);
    ret |= curl_easy_setopt(file->curl, CURLOPT_PROGRESSDATA, (void *)file);
    ret |= curl_easy_setopt(file->curl, CURLOPT_DEBUGFUNCTION, _uf_curl_debug);
    ret |= curl_easy_setopt(file->curl, CURLOPT_VERBOSE, 1);
    /*
    if (file->mode == 'w')
    {
        ret |= curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        ret |= curl_easy_setopt(curl, CURLOPT_READFUNCTION, _uf_read_data);
    }
    else
    */
    {
        ret |= curl_easy_setopt(file->curl, CURLOPT_WRITEFUNCTION, _uf_write_data);
    }

    /*set special parameters by protocol*/
    if (file->proto && file->proto->init)
    {
        ret |= file->proto->init(file);
    }

    return ret;
}

static void _uf_trans_task(UINT32 param1, UINT32 param2)
{
    urlfile *file = (urlfile *)param1;

    if (file == NULL) { return; }
    uf_state(file, UF_OPEN);

    while (file->state != UF_CLOSE)
    {
        switch (file->state)
        {
            case UF_ROPEN:
                uf_state(file, UF_OPEN);
                break;
            case UF_OPEN:
            case UF_TRANS:
                if (file->curl == NULL)
                {
                    file->curl = curl_easy_init();
                }
                if (file->curl == NULL)
                {
                    uf_state(file, UF_ERROR);
                    UF_DEBUG("curl_easy_init fail!\n");
                    break;
                }
                if (_curl_init(file) != CURLE_OK)
                {
                    uf_state(file, UF_ERROR);
                    UF_DEBUG("curl init fail!\n");
                    break;
                }
                _curl_retproc(file, curl_easy_perform(file->curl));
                break;
            case UF_CLOSE:
                break;
            case UF_DONE:
            case UF_ERROR:
                _uf_idle(file);
            default:
                osal_task_sleep(100);
                break;
        }
    }
    UF_DEBUG("uf exit(%d)\n", file->taskid);
    _uf_release(file);
}

/******************************************************************************
 * Function: uf_store_data
 * Description: -
 *    store data to file internal buffer.
 *    It will block task until all data has been stored in buffer or urlfile
 *    state has been changed.
 ******************************************************************************/
size_t uf_store_data(urlfile *file, char *data, size_t len)
{
    size_t wlen = 0;
    size_t ret = 0;

    if (file == NULL || data == NULL) { return ret; }
#if UFDEBUG
    if (file->open_time > 0)
    {
        UF_DEBUG("open url:%s use time:%dms\n", file->url, osal_get_tick() - file->open_time);
        file->open_time = 0;
    }
#endif

    if (file->state == UF_OPEN)
    {
        uf_state(file, UF_TRANS);
    }

    while (UF_TRANS == file->state && ret < len)
    {
        wlen = 0;

        UF_LOCK_WRITE(file);
        if (UF_TRANS == file->state)
        {
            wlen = rbuffer_write(&file->buffer, data + ret, len - ret);
            ret += wlen;
        }
        UF_UNLOCK_WRITE(file);

        if (wlen == 0 && UF_TRANS == file->state)
        {
            osal_task_sleep(5);
        }
    }

    file->doffset += ret;
    return ret;
}

void uf_state(urlfile *file, UF_STATE state)
{
    if (file == NULL) { return; }

    osal_mutex_lock(file->s_lock, OSAL_WAIT_FOREVER_TIME);
    switch (state)
    {
        case UF_ROPEN:
            if (file->state == UF_OPEN || file->state == UF_TRANS
                    || file->state == UF_DONE || file->state == UF_ERROR)
            {
                file->state = state;
            }
            break;
        case UF_OPEN:
            if (file->state == UF_NONE || file->state == UF_ROPEN)
            {
                file->state = state;
            }
            break;
        case UF_TRANS:
            if (file->state == UF_OPEN)
            {
                file->state = state;
            }
            break;
        case UF_CLOSE:
            if (file->state != UF_NONE)
            {
                file->state = state;
            }
            break;
        case UF_ERROR:
            if (file->state == UF_OPEN || file->state == UF_TRANS)
            {
                file->state = state;
            }
            break;
        case UF_DONE:
            if (file->state == UF_TRANS)
            {
                file->state = state;
            }
            break;
        default:
            break;
    }
    osal_mutex_unlock(file->s_lock);
}

urlfile *uf_open(char *url, char *mode)
{
    OSAL_T_CTSK t_ctsk;
    urlfile *file = NULL;
    ID tskid = OSAL_INVALID_ID;

    if (url == NULL)
    {
        return NULL;
    }

    file = _uf_create(url, mode);
    if (file == NULL) { return NULL; }
#if UFDEBUG
    file->open_time = osal_get_tick();
#endif
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x8000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)file;
    t_ctsk.para2 = 0;
    t_ctsk.name[0] = 'U';
    t_ctsk.name[1] = 'F';
    t_ctsk.name[2] = 'T';
    t_ctsk.task = (FP)_uf_trans_task;
    tskid = osal_task_create(&t_ctsk);
    if (tskid == OSAL_INVALID_ID)
    {
        _uf_release(file);
        return NULL;
    }

    while (file->state == UF_NONE) //Wait for the task to run
    {
        osal_task_sleep(10);
    }

    file->taskid = tskid;
    UF_DEBUG("url:%s(%d)\n", url, tskid);
    return file;
}

void uf_close(urlfile *file)
{
    if (file == NULL) { return; }
    UF_DEBUG("uf_close(%d)\n", file->taskid);
    uf_state(file, UF_CLOSE);
}

size_t uf_read(void *ptr, size_t size, size_t nmemb, urlfile *file)
{
    size_t ret = 0;

    if (file == NULL || ptr == NULL) { return 0; }

    UF_LOCK_READ(file);
    ret = rbuffer_read(&file->buffer, ptr, size * nmemb);
    file->cur_pos += ret;
    UF_UNLOCK_READ(file);
#if 0
    if (ret > 0) { UF_DEBUG("%d\n", ret); }
#endif
    return ret;
}

size_t uf_write(void *ptr, size_t size, size_t nmemb, urlfile *file)
{
    libc_printf("unsupport!!\n");
    return 0;
}

int uf_seek(urlfile *file, off_t offset, int whence)
{
    int ret = -1;

    if (file == NULL || file->state <= UF_NONE)
    {
        return -1;
    }
    UF_LOCK_READ(file);//block read.
    UF_LOCK_PROTO(file);
    if (file->proto && file->proto->seek)
    {
        ret = file->proto->seek(file, offset, whence);
    }
    UF_UNLOCK_PROTO(file);
    UF_UNLOCK_READ(file);
    UF_DEBUG("uf_seek [%lld:%d] ret:%d\n", offset, whence, ret);
    return ret;
}

off_t uf_tell(urlfile *file)
{
    if (file == NULL)
    {
        return -1;
    }
    return file->cur_pos;
}

int uf_eof(urlfile *file)
{
    int eof = 1;

    if (file == NULL) { return eof; }

    if (file->state > UF_NONE)
    {
        eof = (file->size > 0 && file->cur_pos >= file->size);
    }
    else if (file->state == UF_ERROR)
    {
        eof = (file->cur_pos >= file->doffset);
    }
#if 1
    if (eof) { UF_DEBUG("url:%s EOF[%lld:%lld:%d]\n", file->url, file->cur_pos, file->size, file->state); }
#endif
    return eof;
}

int uf_ioctl(urlfile *file, UINT32 cmd, UINT32 param)
{
    int ret = -1;

    if (file == NULL || file->state <= UF_NONE)
    {
        return ret;
    }

    UF_LOCK_PROTO(file);
    if (file->proto && file->proto->ioctl)
    {
        ret = file->proto->ioctl(file, cmd, param);
    }
    UF_UNLOCK_PROTO(file);

    if (ret == -1 && cmd == UF_IOCTL_GET_FILESIZE)
    {
        *((off_t *)param) = file->size;
        ret = 0;
    }
    return ret;
}

ufprotocol *uf_findproto(char *url)
{
    int len = 0;
    char s[UF_PROTO_LEN];
    ufprotocol *proto = g_uf_support_proto;

    if (url == NULL) { return NULL; }
    while (proto)
    {
        MEMSET(s, 0, sizeof(s));
        len = STRLEN(proto->name);
        len = (len > UF_PROTO_LEN ? UF_PROTO_LEN : len);
        _uf_lowcase(url, s, len);
        if (MEMCMP(proto->name, s, len) == 0)
        {
            break;
        }
        proto = proto->next;
    }
    return proto;
}

static void uf_regproto(ufprotocol *proto)
{
    ufprotocol *last = g_uf_support_proto;

    if (proto == NULL) { return; }
    proto->next = NULL;
    if (last == NULL)
    {
        g_uf_support_proto = proto;
    }
    else
    {
        while (last->next)
        {
            if (STRCMP(proto->name, last->name) == 0) //exist
            {
                return;
            }
            last = last->next;
        }
        if (STRCMP(proto->name, last->name) == 0) //exist
        {
            return;
        }
        last->next = proto;
    }
}

void uf_protoinit(void)
{
    __MAYBE_UNUSED__ extern ufprotocol uf_http;
    __MAYBE_UNUSED__ extern ufprotocol uf_rtmp;
    __MAYBE_UNUSED__ extern ufprotocol uf_rtsp_live555;
    __MAYBE_UNUSED__ extern ufprotocol uf_mms;
    __MAYBE_UNUSED__ extern ufprotocol uf_hls;
    //extern ufprotocol uf_rtp;
    //extern ufprotocol uf_airplay_img;

    uf_regproto(&uf_http);
#ifdef USE_LIBRTMP
    uf_regproto(&uf_rtmp);
#endif
#ifdef RTSP_SUPPORT
    uf_regproto(&uf_rtsp_live555);//uf_rtsp
#endif
    uf_regproto(&uf_mms);
#ifdef HLS_SUPPORT
    uf_regproto(&uf_hls);
#endif
#ifdef ALICAST_SUPPORT
    uf_regproto(&uf_rtp);
#endif
    //uf_regproto(&uf_airplay_img);
}

void uf_enable_sgdma(void)
{
    uf_use_sgdma = 1;
}

