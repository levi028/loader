#ifndef _UF_PLAYLIST_PROC_H_
#define _UF_PLAYLIST_PROC_H_

#include <api/libcurl/urlfile.h>

typedef enum http_playlist_type
{
    HTTP_PLAYLIST_NONE = 0,
    HTTP_PLAYLIST_HLS,
    HTTP_PLAYLIST_ASX,
    HTTP_PLAYLIST_MMSREF,
    HTTP_PLAYLIST_M3U,
    HTTP_PLAYLIST_PLS,
    HTTP_PLAYLIST_NSC,
    HTTP_PLAYLIST_SMIL,
    HTTP_PLAYLIST_XSPF,
    HTTP_PLAYLIST_MAX,
} http_playlist_type;

typedef struct http_switch_buf
{
    char *buff;
    long long   size;
    long long   len;

    char  url[UF_URL_LEN];
} http_switch_buf;


int uf_store_to_switch_buffer(urlfile *file, char *data, int len);

void uf_playlist_detect(void *file, void *ptr, size_t size);

BOOL uf_playlist_proccess(urlfile *file);


#endif

