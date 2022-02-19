
#ifndef _URL_FILE_H_
#define _URL_FILE_H_
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include "rbuffer.h"

#define UFDEBUG 0
#if UFDEBUG
#define UF_DEBUG(f,...) libc_printf("(%s:%d):"f, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define UF_DEBUG(...)
#endif

#define UF_LOCK_WRITE(uf)   osal_mutex_lock(uf->w_lock, OSAL_WAIT_FOREVER_TIME)
#define UF_UNLOCK_WRITE(uf) osal_mutex_unlock(uf->w_lock)

#define UF_LOCK_READ(uf)    osal_mutex_lock(uf->r_lock, OSAL_WAIT_FOREVER_TIME)
#define UF_UNLOCK_READ(uf)  osal_mutex_unlock(uf->r_lock)

#define UF_LOCK_PROTO(uf)    osal_mutex_lock(uf->p_lock, OSAL_WAIT_FOREVER_TIME)
#define UF_UNLOCK_PROTO(uf)  osal_mutex_unlock(uf->p_lock)

#define UF_PROTO_LEN   32         /*max protocol name length*/
#define UF_URL_LEN     2048       /*max url length*/
#define UF_BUFFER_LEN (128*1024) /*url file internal download buffer size*/
#define UF_TITLE_LEN   128        /* Playlist title max length */
#define UF_MODE_LEN    4          /*url open mode*/

typedef enum
{
    UF_CLOSE  = -2, /*need close, this state only can be set by uf_close*/
    UF_ERROR  = -1, /*error occured*/
    UF_NONE   =  0, /*initial state*/
    UF_ROPEN  =  1, /*reopen*/
    UF_OPEN   =  2, /*start open*/
    UF_TRANS  =  3, /*data transfer*/
    UF_DONE   =  4  /*data transfer complete, it is idle*/
} UF_STATE;

typedef struct _ufprotocol
{
    struct _ufprotocol *next;
    char *name; /*protocol name, must be lower case*/
    
    /*  When do initial the callback will be called, can set some special parameters for protocol in this callback.
        return value:
            0: init success
           !0: init falied, uf->state will be set to UF_ERROR.
    */
    int (*init)(void *file);

    /* Where do uf_close the callback will be called, release protocol private resource in this callback */
    void (*release)(void *file);

    /*  Where do uf_seek the callback will be called, do protocol seek action in this callback
        return value:
             0: seek success.
            -1: seek faile.
    */
    int (*seek)(void *file, off_t offset, int whence);

    /*  process speical command, see UF_IOCTL_xxx.
        return value:
            -1: this cmd not supported by this protocol.
             0: cmd has been proccessed.
    */
    int (*ioctl)(void *file, UINT32 cmd, UINT32 param);

    /* When curl_easy_perform() returned the callback will be called.
       Here, you can check the return value and do something you need.
    */
    void (*curl_retproc)(void *file, CURLcode ret);
    
    /*  When libcurl do progress callback, the callback will be called.
        return value:
            0: libcurl curl_easy_perform() keep running.
            1: libcurl curl_easy_perform() will exit and return CURLE_ABORTED_BY_CALLBACK.
    */
    int (*curl_progcb)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    
    /*  When data is received this callback will be called.
        return value:
            0: data hasn't been processed by this callback, it will be written into urlfile->buffer.
          > 0: data has been processed by this callback, return the length of processed data.
               and the data will not be written to urlfile->buffer.
    */
    size_t (*data_recved)(void *file, void *data, size_t size);
    
    /*  When urlfile->state is UF_DONE/UF_ERROR this callback will be called every 100 ms.
    */
    void (*idle)(void *file);
} ufprotocol;

typedef struct
{
        char      mode[UF_MODE_LEN];    /*url open mode: 0:IPTV, r:read; w:write; c:customize*/
        char      url[UF_URL_LEN];
        off_t     size;    /*initial value is 0, it need be set after connect success. 
                             if the file size is unkown this value need set -1. such as for a live stream URL*/
        off_t     cur_pos; //current read position
        off_t     doffset; //current download offset
        ID        taskid;
        ID        r_lock;  //read mutex
        ID        w_lock;  //write buffer mutex
        ID        s_lock;  //state mutex
        ID        p_lock;  //protocol mutex
        UF_STATE  state;   //url file state
        rbuffer   buffer;  //internal download buffer
        CURL     *curl;    //releated libcurl object, this filed can be changed if you need.
        ufprotocol *proto; //match ufprotocol by URL protocol name
        void     *private; //private data for ufprotocol
#if UFDEBUG        //debug feilds
        UINT32 open_time;
#endif
} urlfile;

typedef struct uf_playlist_entry
{
    char title[UF_TITLE_LEN];
    char link[UF_URL_LEN];
} uf_playlist_entry;

urlfile *uf_open(char *url, char *mode);
void uf_close(urlfile *file);
size_t uf_read(void *ptr, size_t size, size_t nmemb, urlfile *file);
size_t uf_write(void *ptr, size_t size, size_t nmemb, urlfile *file);
int uf_seek(urlfile *file, off_t offset, int whence);
off_t uf_tell(urlfile *file);
int uf_eof(urlfile *file);
int uf_ioctl(urlfile *file, UINT32 cmd, UINT32 param);
void uf_state(urlfile *file, UF_STATE state);
size_t uf_store_data(urlfile *file, char *data, size_t len);
ufprotocol *uf_findproto(char *url);
void uf_protoinit(void);
void uf_enable_sgdma(void);

INT32 uf_playlist_get_cur_list_entry_cnt(void);
BOOL uf_playlist_get_cur_list_entry(UINT32 idx, uf_playlist_entry *entry);

////////////////////////////////////////////////////////////////////////
// urlfile ioctrl commands
#define UF_IOCTL_GET_FILESIZE   0x00000001  // get file size.
#define UF_IOCTL_GET_TOTALTIME  0x00000002  // get total time length
#define UF_IOCTL_MP_PAUSE       0x00000003  // play to pause
#define UF_IOCTL_MP_RESUME      0x00000004  // pause to play
#define UF_IOCTL_GET_SEEKTYPE   0x00000005  // get seek type
#define UF_IOCTL_BREAKPOINT_CONTINUINGLY   0x00000006  // enable/disable Breakpoint continuingly
#define UF_IOCTL_DORUN            0x00000007  //exit wait state and run.if open mode has 'c', you need set this command.
#define UF_IOCTL_GET_CURL       0x00000008  //get internal CURL

#endif
