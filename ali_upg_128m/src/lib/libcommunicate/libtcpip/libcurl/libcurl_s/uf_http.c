
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>
#include "uf_http.h"
#include "uf_util.h"

#define IPTVMODE(uf) (uf->mode[0]==0)
#define LOWCASE(c)   (((c)>='A'&&(c) <= 'Z')?(c)+32:(c))

static int _strncaschr(char *s, int n, char c)
{
    char c1 = 0, c2 = 0;
    INT32 i = 0;

    c1 = LOWCASE(c);
    for (i = 0; i < n; i++)
    {
        c2 = LOWCASE(s[i]);
        if (c1 == c2) { return 1; }
    }
    return 0;
}

static size_t uf_http_header(char *buffer, size_t size, size_t nitems, void *outstream)
{
    struct uf_httppriv *priv = NULL;
    urlfile *file = (urlfile *)outstream;
    int  http_code = 0;

    if (file && file->private)
    {
        priv = (struct uf_httppriv *)file->private;
        if (uf_strstr(buffer, size * nitems, "Accept-Ranges: "))
        {
            priv->seek_type = 1;
        }
    }

    curl_easy_getinfo(file->curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400) //response error
    {
        UF_DEBUG("HTTP Response Error Code:%d\n", http_code);
        return 0;
    }

    return size * nitems;
}

static int uf_http_init(urlfile *file)
{
    struct uf_httppriv *priv = NULL;
    char httprange[128];

    if (file == NULL) { return 1; }
    priv = (struct uf_httppriv *)file->private;
    if (priv == NULL)
    {
        priv = MALLOC(sizeof(struct uf_httppriv));
        if (priv == NULL) {return 1;}
        MEMSET(priv, 0, sizeof(struct uf_httppriv));
        priv->break_cont = IPTVMODE(file);
        priv->retry_connect = 10;
        file->private = priv;
        priv->dorun = !_strncaschr(file->mode, UF_MODE_LEN, 'c');
        priv->headerlist = curl_slist_append(priv->headerlist, "Icy-MetaData: 0");
    }
    if (priv->range_start > 0) //set http range.
    {
        MEMSET(httprange, 0, 128);
        sprintf(httprange, "%lld-", priv->range_start);
        curl_easy_setopt(file->curl, CURLOPT_RANGE, httprange);
        UF_DEBUG("http range:%s\n", httprange);
    }
    curl_easy_setopt(file->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(file->curl, CURLOPT_HEADERFUNCTION, uf_http_header);
    curl_easy_setopt(file->curl, CURLOPT_WRITEHEADER, (void *)file);
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(file->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    if (IPTVMODE(file))
    {
        curl_easy_setopt(file->curl, CURLOPT_USERAGENT, "VLC/2.1.5");
    }
    else
    {
        curl_easy_setopt(file->curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; CIBA)");
    }
    if (priv->headerlist)
    {
        curl_easy_setopt(file->curl, CURLOPT_HTTPHEADER, priv->headerlist);
    }
    priv->timer = osal_get_tick();
    file->doffset = priv->range_start;
    UF_LOCK_READ(file);
    file->cur_pos = priv->range_start;
    UF_UNLOCK_READ(file);
    priv->range_start = 0;

    //wait user set UF_IOCTL_DORUN command if mode is 'c'
    while (!priv->dorun)
    {
        osal_task_sleep(1000);
    }
    return 0;
}

static void uf_http_release(urlfile *file)
{
    struct uf_httppriv *priv = (struct uf_httppriv *)file->private;
    if (priv)
    {
        file->private = NULL;
        if (priv->switch_buf.buff)
        {
            FREE(priv->switch_buf.buff);
        }
        if (priv->headerlist)
        {
            curl_slist_free_all(priv->headerlist);
        }
        FREE(priv);
    }
}

static int uf_http_seek(urlfile *file, off_t offset, int whence)
{
    int ret = 0;
    struct uf_httppriv *priv = NULL;



    if (file == NULL) { return -1; }
    priv = (struct uf_httppriv *)file->private;
    if (priv == NULL || file->size <= 0) { return -1; }

    switch (whence)
    {
        case SEEK_SET:
            if (offset >= 0 && offset < file->size)
            {
                priv->range_start = offset;
                file->cur_pos = priv->range_start;
                UF_LOCK_WRITE(file);
                uf_state(file, UF_ROPEN);      //reopen the url.
                rbuffer_clean(&file->buffer); //clean old data
                UF_UNLOCK_WRITE(file);
            }
            else if (offset == file->size)
            {
                UF_LOCK_WRITE(file); //block the _uf_trans_task
                uf_state(file, UF_DONE); //do nothing
                rbuffer_clean(&file->buffer);
                UF_UNLOCK_WRITE(file);
                file->cur_pos = file->size; //just set cur_pos = file size.
                UF_DEBUG("seek to end!\n");
            }
            else
            {
                ret = -1; /*invalid offset.*/
            }
            break;
        case SEEK_CUR:
            if (offset + file->cur_pos > file->size)
            {
                ret = -1; /*invalid offset.*/
            }
            else if (offset + file->cur_pos == file->size)
            {
                UF_LOCK_WRITE(file); //block the _uf_trans_task
                uf_state(file, UF_DONE);
                rbuffer_clean(&file->buffer);
                UF_UNLOCK_WRITE(file);
                file->cur_pos = file->size; //just set cur_pos = file size.
                UF_DEBUG("seek to end!\n");
            }
            else
            {
                priv->range_start = file->cur_pos + offset;
                file->cur_pos = priv->range_start;
                UF_LOCK_WRITE(file); //block the _uf_trans_task
                uf_state(file, UF_ROPEN);      //reopen the url.
                rbuffer_clean(&file->buffer);  //clean old data
                UF_UNLOCK_WRITE(file);
            }
            break;
        case SEEK_END:
            UF_LOCK_WRITE(file);
            uf_state(file, UF_DONE);  //do nothing
            rbuffer_clean(&file->buffer);
            UF_UNLOCK_WRITE(file);
            file->cur_pos = file->size; //just set cur_pos = file size.
            UF_DEBUG("seek to end!\n");
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

static void uf_http_retproc(urlfile *file, CURLcode ret)
{
    struct uf_httppriv *priv = NULL;
    int    http_code = 0;

    if (file == NULL  || file->private == NULL) { return; }

    UF_DEBUG("%s: ret:%d (%lld:%lld)\n", __FUNCTION__, ret, file->size, file->doffset);
    curl_easy_getinfo(file->curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400)
    {
        uf_state(file, UF_ERROR); //error occurred
        return;
    }

    priv = (struct uf_httppriv *)file->private;
    switch (ret)
    {
        case CURLE_OK:
            if ((priv->pl_type > HTTP_PLAYLIST_NONE) && (priv->pl_type < HTTP_PLAYLIST_MAX))
            {
                if (uf_playlist_proccess(file))
                {
                    break; // we need reopen, break to avoid set to UF_DONE
                }
            }

            if (file->size == -1 && priv->break_cont) // livestream url, so need reopen it.
            {
                uf_state(file, UF_ROPEN);
                break;
            }

            if (file->size > 0 && file->doffset < file->size && priv->break_cont) //Breakpoint continuingly
            {
                priv->range_start = file->doffset;
                uf_state(file, UF_ROPEN);
                break;
            }

            uf_state(file, UF_DONE);
            break;

        case CURLE_ABORTED_BY_CALLBACK:
            break;
        case CURLE_WRITE_ERROR:
            break;
        case CURLE_GOT_NOTHING:
            break;

        case CURLE_RECV_ERROR:
        case CURLE_PARTIAL_FILE: //Breakpoint continuingly
            if (priv->break_cont)
            {
                priv->range_start = file->doffset;
                uf_state(file, UF_ROPEN);
                break;
            }

        case CURLE_COULDNT_CONNECT:
        case CURLE_COULDNT_RESOLVE_HOST:
            if (IPTVMODE(file) && (file->size || priv->retry_connect-- > 0)) //need reconnect again
            {
                priv->range_start = file->doffset;
                uf_state(file, UF_ROPEN);
                break;
            }

        default:
            uf_state(file, UF_ERROR);
            break;
    }
}

static size_t uf_http_recvd(void *file, void *ptr, size_t size)
{
    urlfile *uf = (urlfile *)file;
    struct uf_httppriv *priv = NULL;

    if (uf == NULL) { return 0; }
    priv = (struct uf_httppriv *)uf->private;
    priv->timer = osal_get_tick();

    /*normal mode: needn't parse the conntext.*/
    if (!IPTVMODE(uf)) { priv->pl_type = HTTP_PLAYLIST_MAX; }
    if (priv->pl_type == HTTP_PLAYLIST_NONE)
    {
        /* Maybe this connection is HLS, Playlist, so need check first. */
        uf_playlist_detect(uf, ptr, size);
        /* No need check anymore if have checked that not one of identified playlist. */
        if (priv->pl_type == HTTP_PLAYLIST_NONE)
        {
            priv->pl_type = HTTP_PLAYLIST_MAX;
        }
    }

    /* Downloaded content is playlist, just store in switch buffer, waiting to be process. */
    if ((priv->pl_type > HTTP_PLAYLIST_NONE) && (priv->pl_type < HTTP_PLAYLIST_MAX))
    {
        return uf_store_to_switch_buffer(uf, ptr, size);
    }
	/*
    if (memcmp(ptr, "ICY 200 OK", 10) == 0)
    {
        return switch_2_http_icy(file, ptr, size);
    }
	*/
    if (uf->size == 0)
    {
        curl_easy_getinfo(uf->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &uf->size);
        if (uf->size == 0) { uf->size = -1; }
    }

    return 0;
}

static int uf_http_progcb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int ret = 0;
    urlfile *file = (urlfile *)clientp;
    struct uf_httppriv *priv = (struct uf_httppriv *)file->private;

    //IPTV mode, need retry to get data
    if (IPTVMODE(file) && osal_get_tick() - priv->timer > 5 * 1000 && file->doffset > 0)
    {
        if (file->size > 0)
        {
            priv->range_start = file->doffset;
        }
        else
        {
            priv->range_start = 0;
        }
        UF_DEBUG("no data, need retry!\n");
        return 1;
    }
    ret = (file->state < UF_NONE);
    return ret;
}


static int uf_http_ioctl(void *file, UINT32 cmd, UINT32 param)
{
    int ret = 0;
    urlfile *uf = (urlfile *)file;
    struct uf_httppriv *priv = NULL;

    if (uf == NULL || uf->private == NULL)
    {
        return -1;
    }
    priv = (struct uf_httppriv *)uf->private;
    switch (cmd)
    {
        case UF_IOCTL_GET_SEEKTYPE:
            if (uf->size == -1) { priv->seek_type = 0; }
            *((int *)param) =  priv->seek_type;
            break;
        case UF_IOCTL_BREAKPOINT_CONTINUINGLY:
            priv->break_cont = param;
            break;
        case UF_IOCTL_DORUN:
            priv->dorun = 1;
            break;
        case UF_IOCTL_GET_CURL:
            param = (UINT32)uf->curl;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}


ufprotocol uf_http =
{
    .next = NULL,
    .name = "http",
    .init = (void *)uf_http_init,
    .release = (void *)uf_http_release,
    .seek = (void *)uf_http_seek,
    .ioctl = uf_http_ioctl,
    .curl_retproc = (void *)uf_http_retproc,
    .curl_progcb = uf_http_progcb,
    .data_recved = uf_http_recvd,
};

