/******************************************************************************
 * ALi (Zhuhai) Corporation, All Rights Reserved. 2014 Copyright (C)
 *
 * File: rbuffer.c
 *
 * Description: -
 *     ring buffer module.
 * History
 *       Date            Author             Version           Comment
 *     2014/02/18        Doy.Dong              1              create
 *
 *     2015/04/29        Doy.Dong              2              add rbuffer_copy function 
 *                                                            and code settling 
 *
 *
 ******************************************************************************/

#include <basic_types.h>
#include <os/tds2/itron.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libcurl/rbuffer.h>
#include <bus/dma/dma.h>

#define RBUFF_PTR_INC(P,S,O) ((P)+(O)<(S)?(P)+(O):(P)+(O)-(S))
#define RBUFF_PTR_DEC(P,S,O) ((P)-(O)>=0?(P)-(O):(S)+(P)-(O))

static int data_copy(unsigned char dma_id, void *dest, void *src, unsigned int len)
{
/*
    int ret = 0;
    if (dest == NULL || src == NULL || len <= 0) { return 0; }
    if (dma_id != DMA_INVALID_CHA && len > 1024)
    {
        unsigned int   dma_xfer_id = 0;
        osal_cache_flush(src, len);
        dma_xfer_id = dma_copy(dma_id, src, dest, len, 0);
        if (DMA_INVALID_XFR_ID != dma_xfer_id)
        {
            dma_wait(dma_xfer_id, DMA_WAIT_MODE_DEFALT);
            osal_cache_invalidate(dest, len);
            ret = len;
        }
#if 0
        else
        {
            libc_printf("%s:%d: dma_copy failed!\n", __FUNCTION__, __LINE__);
        }
#endif
    }
    return ret == len ? ret : MEMCPY(dest, src, len);
    */
    return 1;
}
int rbuffer_init(rbuffer *rb, char *buffer, int size)
{
    if ((NULL == rb) || (size <= 0))
    {
        return 0;
    }

    rb->wr = rb->rd = 0;
    rb->size = size;
    rb->rdma_id = DMA_INVALID_CHA;
    rb->wdma_id = DMA_INVALID_CHA;
    if (buffer)
    {
        rb->buff = buffer;
    }
    if (NULL == rb->buff)
    {
        rb->buff = MALLOC(size);
        rb->allocb = (rb->buff != NULL);
    }
    rb->wpb = 1;
    return (rb->buff != NULL);
}

void rbuffer_destroy(rbuffer *rb)
{
    if (NULL == rb) { return; }

    if (rb->buff && rb->allocb)
    {
        FREE(rb->buff);
        rb->buff = NULL;
    }
}

void rbuffer_set_wpb(rbuffer *rb, int wpb)
{
    if (NULL == rb) { return; }
    if (wpb < 1) { wpb = 1; }
    if (wpb < rb->size)
    {
        rb->wpb = wpb;
    }
}

//unread data length in buffer
int rbuffer_dlen(rbuffer *rb)
{
    int r = 0, w = 0;

    if (NULL == rb) { return 0; }
    r = rb->rd;
    w = rb->wr;
    return (w >= r) ? (w - r) : (w + rb->size - r);
}

//free space length
int rbuffer_flen(rbuffer *rb)
{
    int left = 0;
    int r = 0, w = 0;

    if (NULL == rb) { return 0; }
    r = rb->rd;
    w = rb->wr;
    left = (w >= r) ? (r + rb->size - w) : (r - w);
    left = (left > rb->wpb) ? (left - rb->wpb) : 0;
    return left;
}

//old data length
int rbuffer_olen(rbuffer *rb)
{
    int left = 0;
    int r = 0, w = 0;

    if (NULL == rb) { return 0; }
    r = rb->rd;
    w = rb->wr;
    left = (w >= r) ? (r + rb->size - w) : (r - w);
    left = (left > 1) ? (left - 1) : 0;
    return left;
}

void rbuffer_clean(rbuffer *rb)
{
    if (rb)
    {
        rb->rd = rb->wr = 0;
    }
}

int rbuffer_write(rbuffer *rb, char *data, int len)
{
    int wlen = 0;
    int r = 0;
    int w = 0;

    if ((NULL == rb) || (NULL == rb->buff) || (NULL == data))
    {
        return 0;
    }

    r = rb->rd;
    w = rb->wr;
    wlen = (w >= r) ? (r + rb->size - w) : (r - w); //free space length
    wlen -= rb->wpb;
    if ((wlen <= 0) || (len <= 0))
    {
        return 0;
    }

    wlen = (len > wlen) ? wlen : len;
    if (w < r)
    {
        MEMCPY(rb->buff + w, data, wlen);
    }
    else
    {
        int s = rb->size - w;
        if (wlen <= s)
        {
            MEMCPY(rb->buff + w, data, wlen);
        }
        else
        {
            MEMCPY(rb->buff + w, data, s);
            MEMCPY(rb->buff, data + s, wlen - s);
        }
    }

    rb->wr = RBUFF_PTR_INC(w, rb->size, wlen);
    return wlen;
}

int rbuffer_read(rbuffer *rb, char *out, int len)
{
    int rlen = 0;
    int r = 0, w = 0;

    if ((NULL == rb) || (NULL == rb->buff) || (NULL == out))
    {
        return 0;
    }

    r = rb->rd;
    w = rb->wr;
    rlen = (w >= r) ? (w - r) : (w + rb->size - r); // data length
    rlen = len > rlen ? rlen : len;
    if ((rlen <= 0) || (len <= 0))
    {
        return 0;
    }

    if (r < w)
    {
        MEMCPY(out, rb->buff + r, rlen);
    }
    else
    {
        int s = rb->size - r;
        if (rlen <= s)
        {
            MEMCPY(out, rb->buff + r, rlen);
        }
        else
        {
            MEMCPY(out, rb->buff + r, s);
            MEMCPY(out + s, rb->buff, rlen - s);
        }
    }
    rb->rd = RBUFF_PTR_INC(r, rb->size, rlen);
    return rlen;
}

int rbuffer_seek(rbuffer *rb, int offset)
{
    int ret = 0;

    if ((NULL == rb) || (NULL == rb->buff))
    {
        return -1;
    }
    if (0 == offset)
    {
        return 0;
    }

    if ((offset > 0) && (offset <= rbuffer_dlen(rb)))
    {
        rb->rd = RBUFF_PTR_INC(rb->rd, rb->size, offset);
    }
    else if ((offset < 0) && ((-1 * offset) <= rbuffer_olen(rb)))
    {
        rb->rd = RBUFF_PTR_DEC(rb->rd, rb->size, offset);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

int rbuffer_copy(rbuffer *dst, rbuffer *src)
{
    int wlen = 0;
    int r = 0, w = 0;

    if ((NULL == dst) || (NULL == src) || (NULL == dst->buff) || (NULL == src->buff))
    {
        return 0;
    }

    r = dst->rd;
    w = dst->wr;
    wlen = (w >= r) ? (r + dst->size - w) : (r - w); //free space length
    wlen -= dst->wpb;
    if (wlen <= 0)
    {
        return 0;
    }

    if (w < r) //one piece continuous buffer, just read data once
    {
        wlen = rbuffer_read(src, dst->buff + w, wlen);
    }
    else //uncontinuous buffer, need read twice
    {
        int s = dst->size - w; // the last continuous buffer
        if (wlen <= s) //one piece continuous buffer
        {
            wlen = rbuffer_read(src, dst->buff + w, wlen);
        }
        else //two piece buffer
        {
            int ret = rbuffer_read(src, dst->buff + w, s);
            if (s == ret) // read success, read next.
            {
                ret += rbuffer_read(src, dst->buff, wlen - s);
            }
            wlen = ret;
        }
    }
    dst->wr = RBUFF_PTR_INC(w, dst->size, wlen);
    return wlen;
}

