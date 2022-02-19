
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>

#include "uf_hls.h"

#ifdef HLS_SUPPORT

#define HTTP_COMMON_LINE_LENGTH 1024
#define HLS_NETWORK_CONN_TIMEOUT    5
#define HLS_NETWORK_TIMEOUT     30
#define IPTVMODE(uf) (uf->mode[0]==0)

extern void hls_close(HLSContext *context);
extern int hls_stream_seek(HLSContext *context, long time);
extern UINT32 hls_get_duration(HLSContext *context);

/////////////////////////////////////////////////////////////////////
//for HLS parse, ts segment download

static UINT32 uf_hls_get_content_length(char *url)
{
    CURL     *curl = NULL;
    uint32_t content_length = 0;
    CURLcode ulret = 0;

    curl = curl_easy_init();
    if (NULL == curl)
    {
        return content_length;
    }
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    ulret = curl_easy_perform(curl);
    if (0 == ulret)
    {
        ulret = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, (double *)&content_length);
        UF_DEBUG("%s(), content length:%d\n", __FUNCTION__, content_length);
    }
    curl_easy_cleanup(curl);
    
    if (ulret || (-1 == content_length))
    {
        content_length = 0;
    }

    return content_length;
}

static int uf_hls_set_segment_url(urlfile *file, HLSContext *context)
{
    int ret = -1;
    static int cnt = 0;
    uint32_t content_length = 0;
    uf_hlspriv *priv = NULL;
    char httprange[64] = {0};

    if ((NULL == file) || (NULL == context))
    {
        return -1;
    }

    priv = (uf_hlspriv *)file->private;
    do
    {
        ret = hls_get_segment_url(context, file->url);
        if (cnt ++ > 200)
        {
            cnt = 0;
            UF_DEBUG("waiting update m3u8... ret=%d\n", ret);
        }
    }
    while ((URL_SEGMENT_NO == ret) && (file->state != UF_CLOSE));

    if (URL_SEGMENT_NORMAL == ret)
    {
        if (context->is_do_seek)
        {
            context->is_do_seek = 0;
            #ifdef DSC_SUPPORT
            if (context->is_encrypted)
            {
                //encrypted stream DO NOT seek to bytes, just seek to ts segment.
                //Reason: sometimes the TS steam is encrypted by whole TS segment, not by a 
                // TS package(188 bytes), so we only can dencrypt whole TS segment
                context->enc_len = 0;
                context->cur_size = 0;
            }
            else
            #endif
            {
                content_length = uf_hls_get_content_length(file->url);
                if (content_length > 0)
                {
                    priv->ts_size = content_length;
                    priv->range_start = priv->ts_size * priv->ts_time / priv->ts_duration;
                    MEMSET(httprange, 0, sizeof(httprange));
                    sprintf(httprange, "%lld-", priv->range_start);
                    curl_easy_setopt(file->curl, CURLOPT_RANGE, httprange);
                    UF_DEBUG("http range:%s, size = %d, ts_time=%d, ts_duration=%d\n", \
                        httprange, priv->ts_size, priv->ts_time, priv->ts_duration);

                }
            }
        }

        curl_easy_setopt(file->curl, CURLOPT_URL, file->url);
        curl_easy_setopt(file->curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    return ret;
}

static int uf_hls_init(urlfile *file)
{
    int ret = -1;
    uf_hlspriv *priv = (uf_hlspriv *)file->private;

    if (NULL == priv)
    {
        priv = MALLOC(sizeof(uf_hlspriv));
        if (priv == NULL)
        {
            return -1;
        }
        MEMSET(priv, 0, sizeof(uf_hlspriv));
        file->private = priv;
    }

    priv->ts_size = 0;
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    if (priv->context)
    {
        if(priv->range_start)
        {
            char httprange[64] = {0};
            MEMSET(httprange, 0, sizeof(httprange));
            sprintf(httprange, "%lld-", priv->range_start);
            curl_easy_setopt(file->curl, CURLOPT_RANGE, httprange);
            UF_DEBUG("Breakpoint continuingly, http range:%s\n", httprange);
            curl_easy_setopt(file->curl, CURLOPT_URL, file->url);
            ret = URL_SEGMENT_NORMAL;
        }
        else
        {
            ret = uf_hls_set_segment_url(file, priv->context);
        }
    }
    else
    {
        if (file->url)
        {
            priv->context = hls_open(file);
            //video_set_hls_type(1);
            if (NULL != priv->context)
            {
                file->size = -1;
                priv->retry_connect = 10;
                ret = uf_hls_set_segment_url(file, priv->context);
            }
        }
    }
    if (URL_SEGMENT_NORMAL == ret)
    {
        file->doffset = priv->range_start;
        ret = CURLE_OK;
    }
    else if (URL_SEGMENT_FINISH == ret)
    {
        ret = CURLE_OK;
        UF_LOCK_READ(file);
        file->size = file->cur_pos + rbuffer_dlen(&file->buffer);
        UF_UNLOCK_READ(file);
        uf_state(file, UF_DONE);
        UF_DEBUG("%s(), URL_SEGMENT_FINISH\n", __FUNCTION__);
    }
    else if ((URL_SEGMENT_EXIT == ret))
    {
        //to finish ts segment download
        UF_LOCK_READ(file);
        file->size = file->cur_pos + rbuffer_dlen(&file->buffer);
        UF_UNLOCK_READ(file);
        //file->cur_pos = file->size;
        uf_state(file, UF_DONE);
        UF_DEBUG("%s(), URL_SEGMENT_EXIT\n", __FUNCTION__);
    }
    priv->timer = osal_get_tick();
    priv->range_start = 0;
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYHOST, 0L);

    return ret;

}

void uf_hls_release(urlfile *file)
{
    uf_hlspriv *priv = NULL;
    
    if (file && file->private)
    {
        priv = (uf_hlspriv *)file->private;
        if(priv->m3u_curl)
            curl_easy_cleanup(priv->m3u_curl);
        hls_close(priv->context);
        file->private = NULL;
        priv->context = NULL;
        if (priv->ref.buff)
        {
            FREE(priv->ref.buff);
            priv->ref.buff = NULL;
        }
        FREE(priv);
    }
    //video_set_hls_type(0);
}
// so far hls only support time seek.
static int uf_hls_seek(urlfile *file, off_t offset, int fromwhere)
{
    uf_hlspriv *priv = NULL;

    priv = (uf_hlspriv *)file->private;
    switch (fromwhere)
    {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            break;
        case SEEK_END:
            break;
    }
    if (priv->context && (!priv->context->is_continue))
    {
        return hls_stream_seek(priv->context, (long)offset);
    }
    return -1;
}

static void uf_hls_retproc(urlfile *file, CURLcode ret)
{
    uf_hlspriv *priv = (uf_hlspriv *)file->private;;
    
    UF_DEBUG("ret:%d\n", ret);
    switch (ret)
    {
        case CURLE_OK:
            // Not Set UF_DONE, for may there are other segment ts
            // to download.
            //uf_state(file, UF_DONE);
            break;
        case CURLE_ABORTED_BY_CALLBACK: //user abort.
            break;
        case CURLE_WRITE_ERROR: //user abort.
        case CURLE_GOT_NOTHING:
            break;

        case CURLE_COULDNT_CONNECT:
        case CURLE_COULDNT_RESOLVE_HOST:
            if (IPTVMODE(file) && (file->size || priv->retry_connect-- > 0)) //need reconnect again
            {
                priv->range_start = file->doffset;
                uf_state(file, UF_ROPEN);
                break;
            }
            
        case CURLE_RECV_ERROR:
        case CURLE_PARTIAL_FILE: //Breakpoint continuingly
            priv->range_start = file->doffset;
            uf_state(file, UF_ROPEN);
            break;
        default:
            uf_state(file, UF_ERROR);
            break;
    }
}

static size_t uf_hls_recvd(void *file, void *ptr, size_t size)
{
    urlfile *uf = (urlfile *)file;
    HLSContext* context = NULL;    
    uf_hlspriv *priv = NULL;
    int descrambled_len = 0;
    int ret = 0;
    const UINT32 second_ticks = 1000;
    UINT32 time_elapse = 0;

    priv = uf->private;
    context = priv->context;
    if ((NULL == context) || (uf == NULL))
    {
        return 0;
    }

    // re count and statistics the download speed.
    if (context->dl_count > 0)
    {
        context->dl_count += size;
        time_elapse = osal_get_tick() - context->dl_start_tick;
        if (time_elapse >= second_ticks)
        {
            context->dl_speed = context->dl_count * 8;
            UF_DEBUG("%s(), dl speed: %d, count: %d\n", __FUNCTION__, context->dl_speed, context->dl_count);
            context->dl_count= 0;
        }
    }
    else
    {
        context->dl_count += size;
        context->dl_start_tick = osal_get_tick();
    }
    
    priv->timer = osal_get_tick();
    #ifdef DSC_SUPPORT
    if (context->is_encrypted)
    {
        if (!context->is_do_seek)
        {
            //descramble AES data
            descrambled_len = hls_des_data(context, ptr, size);
            if (descrambled_len > 0 && !context->is_do_seek)
            {
                uf_store_data(uf, context->dec_buf, context->dec_len);
            }
        }
        uf->doffset += size;
        return size;
    }
    #endif
    return 0;
}

static int uf_hls_ioctl(void *file, UINT32 cmd, UINT32 param)
{
    int ret = 0;
    urlfile *uf = NULL;
    uf_hlspriv *priv = NULL;
    UINT32 total_time = 0;

    uf = (urlfile *)file;
    if (uf == NULL || uf->private == NULL)
    {
        return ret;
    }
    priv = (uf_hlspriv *)uf->private;
    switch (cmd)
    {
        case UF_IOCTL_GET_TOTALTIME:
            if (priv->context)
            {
                total_time = hls_get_duration(priv->context);
                *((UINT32 *)param) = total_time;
            }
            else
            {
                *((UINT32 *)param) = 0;
            }
            break;
        case UF_IOCTL_GET_SEEKTYPE:
            *((UINT32 *)param) = priv->context->is_continue ? 0 : 2;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;


}

static int uf_hls_progcb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int ret = 0;
    urlfile *file = (urlfile *)clientp;
    uf_hlspriv *priv = NULL;
    HLSContext* context = NULL; 

    priv = (uf_hlspriv *)file->private;
   context = priv->context;
    if (NULL == context) 
    {
        return 0;
    }
	
#ifdef DSC_SUPPORT
    if (!context->is_encrypted)
#endif
    {
        if ((osal_get_tick() - priv->timer) > (10 * 1000))
        {
            priv->range_start = file->doffset;
            UF_DEBUG("no data, need retry!\n");
            return 1;
        }
    }
    ret = (file->state < UF_NONE);
    return ret;
}

ufprotocol uf_hls =
{
    .next = NULL,
    .name = "hls",
    .init = uf_hls_init,
    .release = uf_hls_release,
    .seek = uf_hls_seek,
    .ioctl = uf_hls_ioctl,
    .curl_retproc = uf_hls_retproc,
    .curl_progcb = uf_hls_progcb,
    .data_recved = uf_hls_recvd,
};

#endif
