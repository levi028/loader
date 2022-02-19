
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include "uf_mms.h"
#include "uf_util.h"

extern void network_set_seek_type(UINT8 seek_type);

static int _mms_get_url(uf_mmspriv *priv, int idx)
{
    char *ptr1 = NULL, *ptr2 = NULL;
    char tmp[32];
    int  cpylen = 0;

    MEMSET(priv->ref.url, 0, 1024);
    MEMSET(tmp, 0, 32);
    sprintf(tmp, "Ref%d=", idx + 1);
    ptr1 = uf_strstr(priv->ref.buff, priv->ref.len, tmp);
    if (ptr1)
    {
        ptr1 += STRLEN(tmp);
        ptr1 = uf_strstr(ptr1, priv->ref.len - (ptr1 - priv->ref.buff), "://");
        if (ptr1 == NULL) { return 1; }
        STRCPY(priv->ref.url, "mmsh");
        ptr2 = uf_strstr(ptr1, priv->ref.len - (ptr1 - priv->ref.buff), "\r\n");
        cpylen = (ptr2 == NULL) ? STRLEN(ptr1) : ptr2 - ptr1;
        cpylen = cpylen > 1020 ? 1020 : cpylen;
        strncpy(priv->ref.url + 4, ptr1, cpylen);
        UF_DEBUG("get mms URL:%s\n", priv->ref.url);
        return 0;
    }
    return 1;
}

int mms_init(urlfile *file)
{
    int ret = 0;
    uf_mmspriv *priv = (uf_mmspriv *)file->private;

    if (priv == NULL)
    {
        priv = MALLOC(sizeof(uf_mmspriv));
        if (priv == NULL) { return 1; }
        MEMSET(priv, 0, sizeof(uf_mmspriv));
        file->private = priv;
    }
    if (priv->dtimeoffset > 0)
    {
        ret |= curl_easy_setopt(file->curl, CURLOPT_SEEK_FLAG, 1);
        ret |= curl_easy_setopt(file->curl, CURLOPT_SEEK_TIME, priv->dtimeoffset);
        UF_DEBUG("seek to %f\n", priv->dtimeoffset);
    }
    else
    {
        ret |= curl_easy_setopt(file->curl, CURLOPT_SEEK_FLAG, 0);
        ret |= curl_easy_setopt(file->curl, CURLOPT_SEEK_TIME, 0);
    }

    if (priv->ref.buff) //switch from http
    {
        ret |= _mms_get_url(priv, priv->ref.idx);
        ret |= curl_easy_setopt(file->curl, CURLOPT_URL, priv->ref.url);
    }
    return ret;
}

void mms_release(urlfile *file)
{
    uf_mmspriv *priv = (uf_mmspriv *)file->private;
    if (priv)
    {
        file->private = NULL;
        if (priv->ref.buff)
        {
            FREE(priv->ref.buff);
            priv->ref.buff = NULL;
        }
        FREE(priv);
    }
}

static int mms_seek(urlfile *file, off_t offset, int fromwhere)
{
    int ret = -1;
    uf_mmspriv *priv = (uf_mmspriv *)file->private;
    if (fromwhere == 3)
    {
        UF_LOCK_WRITE(file);
        priv->dtimeoffset = offset;
        file->cur_pos = 0;
        uf_state(file, UF_ROPEN); //reopen
        rbuffer_clean(&file->buffer);
        UF_UNLOCK_WRITE(file);
        UF_DEBUG("time offset:%lld\n", offset);
        ret = 0;
    }
    return ret;
}

static void uf_mms_retproc(urlfile *file, CURLcode ret)
{
    uf_mmspriv *priv = (uf_mmspriv *)file->private;
    UF_DEBUG("%s: ret:%d\n", __FUNCTION__, ret);
    switch (ret)
    {
        case CURLE_OK:
        case CURLE_MXXXMX_SEEK_ERROR:
            if (file->size == -1)
            {
                uf_state(file, UF_ROPEN);
                break;
            }
            uf_state(file, UF_DONE);
            break;

        case CURLE_ABORTED_BY_CALLBACK: //user abort.
            break;
        case CURLE_WRITE_ERROR: //user abort.
            break;

        case CURLE_FAILED_INIT:
            if (priv->ref.buff)
            {
                priv->ref.idx++;
                uf_state(file, UF_ROPEN);
                break;
            }
        default:
            uf_state(file, UF_ERROR);
            break;
    }
}

static size_t uf_mms_recvd(void *file, void *ptr, size_t size)
{
    off_t sllcurrentpos = 0;
    urlfile *uf = (urlfile *)file;
    uf_mmspriv *priv = (uf_mmspriv *)uf->private;

    if (uf == NULL) { return 0; }
    if (uf->size == 0) // get file size.
    {
        curl_easy_getinfo(uf->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &uf->size);
    }
    if (priv->dtimelength == 0)
    {
        curl_easy_getinfo(uf->curl, CURLINFO_SEEK_INFO, &priv->ulseekinfo);
        curl_easy_getinfo(uf->curl, CURLINFO_TIME_LENGTH, &priv->dtimelength);
        network_set_seek_type(priv->ulseekinfo);
    }
    if (uf->cur_pos == 0)
    {
        curl_easy_getinfo(uf->curl, CURLINFO_CURRENT_POS, &sllcurrentpos);
        UF_LOCK_READ(uf);
        uf->cur_pos = sllcurrentpos;
        UF_UNLOCK_READ(uf);
        UF_DEBUG("seek type:%d\n", priv->ulseekinfo);
    }
    return 0;
}

static void mms_ioctl(void *file, UINT32 cmd, UINT32 param)
{
    __MAYBE_UNUSED__ int ret = 0;
    urlfile *uf = NULL;
    uf_mmspriv *priv = NULL;

    uf = (urlfile *)file;
    if (uf == NULL || uf->private == NULL)
    {
        return ;//-1;
    }
    priv = (uf_mmspriv *)uf->private;
    switch (cmd)
    {
        case UF_IOCTL_GET_TOTALTIME:
            *((UINT32 *)param) = (UINT32)priv->dtimelength;
            break;
        case UF_IOCTL_GET_SEEKTYPE:
            *((UINT32 *)param) = priv->ulseekinfo ? 2 : 0;
            break;
        default:
            ret = -1;
            break;
    }
    return ;//ret;


}

ufprotocol uf_mms =
{
    .next = NULL,
    .name = "mms",
    .init = (void *)mms_init,
    .release = (void *)mms_release,
    .seek = (void *)mms_seek,
    .ioctl = (void *)mms_ioctl,
    .curl_retproc = (void *)uf_mms_retproc,
    .curl_progcb = NULL,
    .data_recved = uf_mms_recvd,
};

