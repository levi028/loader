#include <sys_config.h>
#include <api/libc/alloc.h>
#include <string.h>
#include <stdio.h>
#include <api/libcurl/urlfile.h>
#include "uf_playlist.h"
#include "uf_playlist_proc.h"
#include "uf_asxparser.h"
#include "uf_refparser.h"
#include "uf_m3uparser.h"
#include "uf_plsparser.h"
#include "uf_smilparser.h"
#include "uf_nscparser.h"
#include "uf_http.h"
#include "uf_hls.h"
#include "uf_mms.h"
#include "uf_util.h"
#include "uf_xspfparser.h"

static BOOL _uf_is_asx_playlist(urlfile *file, char *data, int len)
{
    int i = 0;
    char *p = NULL;
    __MAYBE_UNUSED__ char debug[10] = {0};

    for (i = 0; i < 9; i++)
    {
        debug[i] = data[i];
    }
    UF_DEBUG("%s:%s\n", __FUNCTION__, debug);

    p = data;

    for (i = 0; i < len; i++)
    {
        if (*p == '<')
        {
            break;
        }
        p++;
    }

    if (i == len)
    {
        return FALSE;
    }

    p++;
    i++;

    for (; i < len; i++)
    {
        if (*p != ' ')
        {
            break;
        }
        p++;
    }
    if (((*p == 'A') || (*p == 'a'))
            && ((*(p + 1) == 'S') || (*(p + 1) == 's'))
            && ((*(p + 2) == 'X') || (*(p + 2) == 'x')))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL _uf_is_mmsref(urlfile *file, char *data, int len)
{
    if (uf_strstr(data, len, "[Reference]") && uf_strstr(data, len, "asf"))
    {
        return TRUE;
    }
    return FALSE;
}
static BOOL _uf_is_xspf_playlist(urlfile *file, char *data, int len)
{
    if (uf_strstr(data, len, "xspf"))
    {
        return TRUE;
    }
    return FALSE;
}

static BOOL _uf_is_pls_playlist(urlfile *file, char *data, int len)
{
    if (uf_strstr(data, len, "[playlist]"))
    {
        return TRUE;
    }
    return FALSE;
}
static BOOL _uf_is_smil_playlist(urlfile *file, char *data, int len)
{
    char *ptr = NULL;

    ptr = uf_strstr(data, len, "<?xml");
    if (ptr == NULL) { ptr = uf_strstr(data, len, "<!DOCTYPE smil"); }

    if (ptr)
    {
        if (uf_strstr(data, len, "<smil")
                || uf_strstr(data, len, "<?wpl")
                || uf_strstr(data, len, "(smil-document"))
        {
            return TRUE;
        }
    }
    return FALSE;
}
static BOOL _uf_is_nsc_playlist(urlfile *file, char *data, int len)
{
    if (uf_strstr(data, len, "[Address]"))
    {
        return TRUE;
    }
    return FALSE;
}

static BOOL _uf_is_m3u_playlist(urlfile *file, char *data, int len)
{
    if (strncmp(data, "#EXTM3U", 7))
    {
        return FALSE;
    }

    if (uf_strstr(data, len, "#EXT-X-STREAM-INF:")
            || uf_strstr(data, len, "#EXT-X-TARGETDURATION:")
            || uf_strstr(data, len, "#EXT-X-MEDIA-SEQUENCE:"))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL _uf_is_hls(urlfile *file, char *data, int len)
{
    /* Require #EXTM3U at the start, and either one of the ones below somewhere for a proper match. */
    if (strncmp(data, "#EXTM3U", 7))
    {
        return FALSE;
    }
    if (uf_strstr(data, len, "#EXT-X-STREAM-INF:")
            || uf_strstr(data, len, "#EXT-X-TARGETDURATION:")
            || uf_strstr(data, len, "#EXT-X-MEDIA-SEQUENCE:"))
    {
        return TRUE;
    }

    return FALSE;
}

int uf_store_to_switch_buffer(urlfile *file, char *data, int len)
{
#define MAX_PLAYLIST_SIZE 0x100000
    struct uf_httppriv *priv = NULL;

    if (file == NULL || data == NULL || len == 0)
    {
        return 0;
    }

    uf_state(file, UF_TRANS);
    priv = (struct uf_httppriv *)file->private;
    if (priv->switch_buf.buff == NULL)
    {
        curl_easy_getinfo(file->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &priv->switch_buf.size);
        if (priv->switch_buf.size <= 0)
        {
            priv->switch_buf.size = (512 * 1024) - 1;
        }
        else if (priv->switch_buf.size > MAX_PLAYLIST_SIZE)
        {
            UF_DEBUG("Playlist size too big:0x%d, max support %d\n", priv->switch_buf.size, MAX_PLAYLIST_SIZE);
            return -1;
        }

        priv->switch_buf.buff = MALLOC(priv->switch_buf.size + 1);
        if (priv->switch_buf.buff == NULL)
        {
            return -1;
        }
        MEMSET(priv->switch_buf.buff, 0, priv->switch_buf.size + 1);
    }

    if (priv->switch_buf.len + len <= priv->switch_buf.size)
    {
        MEMCPY(priv->switch_buf.buff + priv->switch_buf.len, data, len);
        priv->switch_buf.len += len;
    }

    return len;
}

void uf_playlist_detect(void *file, void *ptr, size_t size)
{
    urlfile *uf = (urlfile *)file;
    struct uf_httppriv *priv = NULL;

    priv = (struct uf_httppriv *)uf->private;
    if (_uf_is_hls(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_HLS;
    }
    else if (_uf_is_m3u_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_M3U;
    }
    else if (_uf_is_pls_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_PLS;
    }
    else if (_uf_is_nsc_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_NSC;
    }
    else if (_uf_is_smil_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_SMIL;
    }
    else if (_uf_is_asx_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_ASX;
    }
    else if (_uf_is_xspf_playlist(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_XSPF;
    }
    else if (_uf_is_mmsref(uf, ptr, size))
    {
        priv->pl_type = HTTP_PLAYLIST_MMSREF;
    }
}


static BOOL _switch_to_new_url(urlfile *file, char *url)
{
    ufprotocol *proto = NULL;

    if (NULL == url)
    {
        return FALSE;
    }

    strncpy(file->url, url, UF_URL_LEN - 1);

    // release old proto
    file->proto->release(file);

    proto = uf_findproto(url);

    // attach new proto
    file->proto = proto;

    return TRUE;
}

static int _uf_http_to_mms(urlfile *file)
{
    struct uf_httppriv *httppriv = (struct uf_httppriv *)file->private;
    ufprotocol *proto = NULL;
    uf_mmspriv *mmspriv = NULL;

    if (httppriv->pl_type == HTTP_PLAYLIST_MMSREF)
    {
        UF_DEBUG("\n%s\n", httppriv->switch_buf.buff);
        proto = uf_findproto("mms");
        if (proto == NULL)
        {
            uf_state(file, UF_ERROR);
            return 1;
        }
        mmspriv = MALLOC(sizeof(uf_mmspriv));
        if (mmspriv == NULL)
        {
            uf_state(file, UF_ERROR);
            return 1;
        }

        MEMSET(mmspriv, 0, sizeof(uf_mmspriv));
        mmspriv->ref.buff = httppriv->switch_buf.buff;
        mmspriv->ref.len = httppriv->switch_buf.len;
        mmspriv->ref.size = httppriv->switch_buf.size;

        UF_LOCK_PROTO(file);
        httppriv->switch_buf.buff = NULL;
        file->proto->release(file);
        file->proto = proto;
        file->private = mmspriv;
        UF_UNLOCK_PROTO(file);
        uf_state(file, UF_ROPEN);
        UF_DEBUG("switch to mms ...\n");
        return 1;
    }
    return 0;
}

static int _uf_http_to_hls(urlfile *file)
{
    ufprotocol *proto = NULL;
    struct uf_httppriv *httppriv = (struct uf_httppriv *)file->private;
    uf_hlspriv *hlspriv = NULL;
    //char *path = NULL;

    if (httppriv->pl_type == HTTP_PLAYLIST_HLS)
    {
        proto = uf_findproto("hls");
        if (proto == NULL)
        {
            uf_state(file, UF_ERROR);
            return 1;
        }
        hlspriv = MALLOC(sizeof(uf_hlspriv));
        if (hlspriv == NULL)
        {
            uf_state(file, UF_ERROR);
            return 1;
        }
        MEMSET(hlspriv, 0, sizeof(uf_hlspriv));

        hlspriv->ref.buff = httppriv->switch_buf.buff;
        hlspriv->ref.len = httppriv->switch_buf.len;
        hlspriv->ref.size = httppriv->switch_buf.size;
        hlspriv->timer = httppriv->timer;

        UF_LOCK_PROTO(file);
        hlspriv->m3u_curl = file->curl;
        file->curl = NULL;
        httppriv->switch_buf.buff = NULL;
        file->proto->release(file);
        file->proto = proto;
        file->private = hlspriv;
        UF_UNLOCK_PROTO(file);
        uf_state(file, UF_ROPEN);
        UF_DEBUG("switch to HLS ...\n");
        return 1;
    }
    return 0;
}

BOOL uf_playlist_proccess(urlfile *file)
{
    uf_playlist_t *playlist = NULL;
    struct uf_httppriv *priv = NULL;
    uf_playlist_entry entry;
    INT32  entry_cnt = 0;

    priv = (struct uf_httppriv *)file->private;
    if (priv->pl_type == HTTP_PLAYLIST_HLS)
    {
        _uf_http_to_hls(file);
        return TRUE; /* HLS no need notify application. */
    }
    else if (priv->pl_type == HTTP_PLAYLIST_M3U)
    {
        playlist = m3u_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }
    else if (priv->pl_type == HTTP_PLAYLIST_PLS)
    {
        playlist = pls_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }
    else if (priv->pl_type == HTTP_PLAYLIST_MMSREF)
    {
        _uf_http_to_mms(file);
        return TRUE;
    }
    else if (priv->pl_type == HTTP_PLAYLIST_NSC)
    {
        playlist = nsc_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }
    else if (priv->pl_type == HTTP_PLAYLIST_SMIL)
    {
        playlist = smil_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }
    else if (priv->pl_type == HTTP_PLAYLIST_ASX)
    {
        playlist = asx_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }
    else if (priv->pl_type == HTTP_PLAYLIST_XSPF)
    {
        playlist = xspf_parser_build_tree(priv->switch_buf.buff, priv->switch_buf.len);
    }

    if (NULL == playlist)
    {
        return FALSE;
    }

    uf_playlist_set_cur_list(playlist);

    /* Notify application that we got one playlist, application can get this list from API. */
    libnet_notify(NET_MSG_DOWNLOAD_PLAYLIST, 0);

    /* Not let's play the first item in playlist. */
    entry_cnt = uf_playlist_get_cur_list_entry_cnt();
    if (entry_cnt > 0)
    {
        UF_DEBUG("Playlist entry count = %d\n", entry_cnt);
        if (FALSE == uf_playlist_get_cur_list_entry(0, &entry))
        {
            UF_DEBUG("Get first entry from playlist fail\n");
            return FALSE;
        }

        UF_DEBUG("First entry: Title %s\nURL %s\n", entry.title, entry.link);
        if (FALSE == _switch_to_new_url(file, entry.link))
        {
            return FALSE;
        }
        uf_state(file, UF_ROPEN);

        return TRUE;
    }

    return FALSE;
}

