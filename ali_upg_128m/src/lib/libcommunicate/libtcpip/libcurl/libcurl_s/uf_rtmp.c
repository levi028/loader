#include "setup.h"

#ifdef USE_LIBRTMP

#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include "librtmp/rtmp.h"
#include "uf_util.h"

struct uf_rtmppriv
{
    ID     seek_lock;
    INT32  seek_time;    /* seek cmd time param */
    UINT32 seek_enable;  /* enable or not */
    RTMP  *rtmp;         /* rtmp handle */
    UINT8  reopen_seek;  /*1:reopen seek; 0: normal seek*/
    UINT8  reservd[3];
};

int rtmp_init(urlfile *file)
{
    struct uf_rtmppriv *priv = NULL;
    int live = 0;

    if (file == NULL || file->curl == NULL) { return 1; }

    priv = (struct uf_rtmppriv *)file->private;
    if (priv == NULL)
    {
        priv = MALLOC(sizeof(struct uf_rtmppriv));
        if (priv == NULL) { return 1;}
        file->private = priv;
        MEMSET(priv, 0, sizeof(struct uf_rtmppriv));
        priv->seek_lock = osal_mutex_create();
        if (priv->seek_lock == OSAL_INVALID_ID) { return 1; }
    }
    file->size = -1;
    priv->rtmp = NULL;
    live = uf_strstr(file->url, STRLEN(file->url), "live") != NULL ? 1 : 0;
    curl_easy_setopt(file->curl, CURLOPT_RTMP_LIVESTREAM, live);
    curl_easy_setopt(file->curl, CURLOPT_RTMP_TIMEOUT, 300);
    curl_easy_setopt(file->curl, CURLOPT_NOT_STRIP_URLPARAM, 1);    
    return 0;
}

void rtmp_release(urlfile *file)
{
    struct uf_rtmppriv *priv = NULL;
    if (file->private)
    {
        priv = (struct uf_rtmppriv *)file->private;
        if (priv->seek_lock != OSAL_INVALID_ID) { 
            osal_mutex_delete(priv->seek_lock); 
        }
        FREE(file->private);
        file->private = NULL;
    }
}

static int rtmp_seek(urlfile *file, off_t offset, int fromwhere)
{
    UINT32 wait_tims = 0;
    struct uf_rtmppriv *priv = (struct uf_rtmppriv *)file->private;

    /**
     * UF_DONE: librtmp & libcurl exit
     * RTMP_STATUS_IDLE: librtmp exit but libcurl not exit yet.
     */
    if (file->state == UF_DONE || rtmp_get_status(priv->rtmp) == RTMP_STATUS_IDLE)
    {
        UF_LOCK_WRITE(file);
        uf_state(file, UF_ROPEN);
        file->size = -1;
        rbuffer_clean(&file->buffer);
        UF_UNLOCK_WRITE(file);
    }
    priv->seek_time = (int)offset;

    osal_mutex_lock(priv->seek_lock, OSAL_WAIT_FOREVER_TIME);
    priv->seek_enable = 1;
    file->doffset = 0;
    rbuffer_clean(&file->buffer);
    osal_mutex_unlock(priv->seek_lock);

    return 0;
}

static void uf_rtmp_done(urlfile *file)
{
    struct uf_rtmppriv *priv = (struct uf_rtmppriv *)file->private;
    if (priv->rtmp && rtmp_get_status(priv->rtmp) == RTMP_STATUS_IDLE)
    {
        do
        {
            if (rbuffer_dlen(&file->buffer) == 0)
            {
                file->size = file->cur_pos;
                break;
            }
            osal_task_sleep(10);
        }
        while (file->state == UF_TRANS);
        uf_state(file, UF_DONE);
    }
}

static void uf_rtmp_retproc(urlfile *file, CURLcode ret)
{
    struct uf_rtmppriv *priv = (struct uf_rtmppriv *)file->private;

    UF_DEBUG("%s: ret:%d\n", __FUNCTION__, ret);
    switch (ret)
    {
        case CURLE_OK:
            uf_rtmp_done(file);
            break;
        case CURLE_ABORTED_BY_CALLBACK: //user abort.
            uf_rtmp_done(file);
            break;
        case CURLE_WRITE_ERROR: //user abort.
            uf_rtmp_done(file);
            break;
        default:
            uf_state(file, UF_ERROR);
            break;
    }
}

static size_t uf_rtmp_recvd(void *file, void *ptr, size_t size)
{
    size_t  wlen = size;
    urlfile *uf = (urlfile *)file;
    struct uf_rtmppriv *priv = (struct uf_rtmppriv *)uf->private;

    if (uf == NULL || priv == NULL)
    {
        return 0;
    }

    if (uf->size == 0) // get file size.
    {
        curl_easy_getinfo(uf->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &uf->size);
    }
    if (priv->rtmp == 0)
    {
        curl_easy_getinfo(uf->curl, CURLINFO_GENERIC_PROTO, &priv->rtmp);
    }

    if (priv->seek_enable == 1 && rtmp_get_status(priv->rtmp) == RTMP_STATUS_IDLE)
    {
        return -1;
    }
    /*
     * reopen seek, need wait some data then seek.
     * normal seek, can do seek im.
     */
    while (wlen > 0 && uf->state == UF_TRANS)
    {
        osal_mutex_lock(priv->seek_lock, OSAL_WAIT_FOREVER_TIME);

        if (1 == priv->seek_enable)
        {
            if (rtmp_get_status(priv->rtmp) == RTMP_STATUS_PLAYING)
            {
                if (uf->doffset > 0x2000)
                {
                    wlen = 0;
                    priv->seek_enable = 0;
                    uf->doffset = 0;
                    rbuffer_clean(&uf->buffer);
                    RTMP_Seek(priv->rtmp, priv->seek_time);
                }
            }
            else
            {
                wlen = 0;
            }
        }

        if (0 == priv->seek_enable && wlen > 0 && rtmp_get_status(priv->rtmp) == RTMP_STATUS_PLAYING)
        {
            if (wlen < rbuffer_flen(&uf->buffer))
            {
                wlen -= uf_store_data(file, ptr + (size - wlen), wlen);
            }
        }
        else
        {
            uf->doffset += wlen;
            wlen = 0;
        }
        osal_mutex_unlock(priv->seek_lock);

        if (wlen > 0) { osal_task_sleep(5); }
    }

    return size - wlen;
}

static int uf_rtmp_ioctl(void *file, UINT32 cmd, UINT32 param)
{
    int ret = 0;
    struct uf_rtmppriv *priv = NULL;
    urlfile *uf = (urlfile *)file;

    if (uf == NULL || uf->private == NULL) { return -1; }

    priv = (struct uf_rtmppriv *)uf->private;
    switch (cmd)
    {
        case UF_IOCTL_GET_SEEKTYPE:
            *((UINT32 *)param) = RTMP_GetDuration(priv->rtmp) ? 2 : 0;
            ret = 0;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

static int uf_rtmp_progcb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int ret = 0;
    struct uf_rtmppriv *priv = NULL;
    urlfile *file = (urlfile *)clientp;

    if (file == NULL || file->private == NULL) { return 0; }
    priv = (struct uf_rtmppriv *)file->private;
    ret = (priv->rtmp && rtmp_get_status(priv->rtmp) == RTMP_STATUS_IDLE);
    return ret;
}

ufprotocol uf_rtmp =
{
    .next = NULL,
    .name = "rtmp",
    .init = rtmp_init,
    .release = rtmp_release,
    .seek = rtmp_seek,
    .ioctl = uf_rtmp_ioctl,
    .curl_retproc = uf_rtmp_retproc,
    .curl_progcb = uf_rtmp_progcb,
    .data_recved = uf_rtmp_recvd,
};

#endif

