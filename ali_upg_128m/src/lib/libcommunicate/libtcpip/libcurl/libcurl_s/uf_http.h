#ifndef _UF_HTTP_H__
#define _UF_HTTP_H__

#include <basic_types.h>
#include "uf_playlist_proc.h"


struct uf_httppriv
{
    off_t   range_start; // for seek/reopen
    UINT32  timer;
    int     seek_type;     // 0: byte seek, 1: time seek.
    int     break_cont;    // breakpoint continuingly, 0: disable, 1: enable, default is 1;

    http_playlist_type  pl_type; // identify URL playlist content is HLS, ASX or others.
    http_switch_buf     switch_buf;

    int     retry_connect;  //try to connect if connect fail.
    int     dorun;
    struct  curl_slist *headerlist;
};


#endif//_UF_HTTP_H__

