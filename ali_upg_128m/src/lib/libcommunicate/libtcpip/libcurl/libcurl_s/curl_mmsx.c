#include "setup.h"
#include "api/libcurl/curl.h"

#ifdef USE_LIBMXXXMX

#include "curl_mmsx.h"
#include "urldata.h"
#include "nonblock.h" /* for curlx_nonblock */
#include "progress.h" /* for Curl_pgrsSetUploadSize */
#include "transfer.h"
#include "warnless.h"
#include "libmms/mmsx.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include <api/libcurl/mprintf.h>

#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"

#ifdef _WIN32
#define setsockopt(a,b,c,d,e) (setsockopt)(a,b,c,(const char *)d,(int)e)
#define SET_RCVTIMEO(tv,s)   int tv = s*1000
#else
#define SET_RCVTIMEO(tv,s)   int tv = s*1000  	// NOTE: need modify it when upgrade libcurl version
												//struct timeval tv = {s,0}
#endif
static CURLcode curl_mmsxsetupconnect(struct connectdata *conn);
static CURLcode curl_mmsxdo(struct connectdata *conn, bool *done);
static CURLcode curl_mmsxdone(struct connectdata *conn, CURLcode, bool premature);
static CURLcode curl_mmsxconnect(struct connectdata *conn, bool *done);
static CURLcode curl_mmsxdisconnect(struct connectdata *conn, bool dead);
static Curl_recv curl_mmsxrecv;

const struct Curl_handler Curl_handler_mxxxm = 
{
    "mms",                               /* scheme */
    curl_mmsxsetupconnect,               /* setup_connection */
    curl_mmsxdo,                              /* do_it */
    curl_mmsxdone,                            /* done */
    ZERO_NULL,                            /* do_more */
    ZERO_NULL,                         /* connect_it */
    ZERO_NULL,                            /* connecting */
    ZERO_NULL,                            /* doing */
    ZERO_NULL,                            /* proto_getsock */
    ZERO_NULL,                            /* doing_getsock */
    ZERO_NULL,                            /* domore_getsock */
    ZERO_NULL,                            /* perform_getsock */
    curl_mmsxdisconnect,                      /* disconnect */
    ZERO_NULL,                            /* readwrite */
    PORT_MXXXM,                            /* defport */
    CURLPROTO_MXXXM,                       /* protocol */
    PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_mxxxmh = 
{
    "mmsh",                               /* scheme */
    curl_mmsxsetupconnect,                           /* setup_connection */
    curl_mmsxdo,                              /* do_it */
    curl_mmsxdone,                            /* done */
    ZERO_NULL,                            /* do_more */
    ZERO_NULL,                         /* connect_it */
    ZERO_NULL,                            /* connecting */
    ZERO_NULL,                            /* doing */
    ZERO_NULL,                            /* proto_getsock */
    ZERO_NULL,                            /* doing_getsock */
    ZERO_NULL,                            /* domore_getsock */
    ZERO_NULL,                            /* perform_getsock */
    curl_mmsxdisconnect,                      /* disconnect */
    ZERO_NULL,                            /* readwrite */
    PORT_MXXXMH,                            /* defport */
    CURLPROTO_MXXXMH,                       /* protocol */
    PROTOPT_NONE                          /* flags*/
};

#define MMSX_BANDWIDE_DEFAULT 1024 * 128
#if 0
static CURLcode curl_mmsxconnect(struct connectdata *conn, bool *done)
{
    int slresult = 0;
    int slbandwidth = 0;
    mmsx_t *pmmsx = NULL;
    sys_sem_t mmsxseeksem = NULL;

    lprintf("curl_mmsxconnect:enter....\r\n");
    SET_RCVTIMEO(tv,10);
    slbandwidth = MMSX_BANDWIDE_DEFAULT;
    
    pmmsx = mmsx_connect(NULL, NULL, conn->data->change.url, slbandwidth);
    if (NULL == pmmsx)
    {
        *done = false;
        lprintf("curl_mmsxconnect:call mmsx_connect return null.\r\n");
        return CURLE_FAILED_INIT;
    }

    conn->proto.generic = pmmsx;
    closesocket(conn->sock[FIRSTSOCKET]);
    conn->recv[FIRSTSOCKET] = curl_mmsxrecv;

    *done = TRUE;
    if (NULL != pmmsx->connection)
    {
        conn->sock[FIRSTSOCKET] = pmmsx->connection->s;
    }
    else
    {
        conn->sock[FIRSTSOCKET] = pmmsx->connection_h->s;
    }

    return CURLE_OK;
}

static CURLcode curl_mmsxdo(struct connectdata *conn, bool *done)
{
    int slseekable = 0;
    unsigned int ulfilesize = 0;
    double dtimelength = 0.0;
    mmsx_t *pmmsxt = NULL;
    struct SessionHandle *pSessionHandleTemp = NULL;

    lprintf("curl_mmsxdo:enter....\r\n");
    pmmsxt = conn->proto.generic;
    if (NULL == pmmsxt)
    {
        lprintf("curl_mmsxdo:get pmmsxt null pointer.\r\n");
        return CURLE_MXXXMX_GETPOINTER_ERROR;
    }
    pSessionHandleTemp = conn->data;
    if (NULL == pSessionHandleTemp)
    {
        lprintf("curl_mmsxdo:get pSessionHandleTemp null pointer.\r\n");
        return CURLE_MXXXMX_GETPOINTER_ERROR;
    }

    if (((pmmsxt->connection) && (pmmsxt->connection->seekable == 1))||
       ((pmmsxt->connection_h) && (pmmsxt->connection_h->seekable == 1)))
    {
        slseekable = 2;
    }
    curl_easy_setopt(pSessionHandleTemp, CURLOPT_SEEK_INFO, slseekable);
    
    ulfilesize = mmsx_get_length(pmmsxt);
    dtimelength = mmsx_get_time_length(pmmsxt);
    lprintf("filesize=%d,seekable=%d,dtimelength=%f.\r\n",ulfilesize,slseekable,dtimelength);
    if (slseekable != 0)
    {
        curl_easy_setopt(pSessionHandleTemp, CURLOPT_TIME_LENGTH, dtimelength);
        Curl_setup_transfer(conn, FIRSTSOCKET,ulfilesize, FALSE, NULL, -1, NULL);    
    }
    else
    {
        curl_easy_setopt(pSessionHandleTemp, CURLOPT_TIME_LENGTH, 0.0);
        Curl_setup_transfer(conn, FIRSTSOCKET,-1, FALSE, NULL, -1, NULL);
    }
    
    *done = TRUE;
    return CURLE_OK;  
}
#endif

static CURLcode curl_mmsxsetupconnect(struct connectdata *conn)
{
    int slresult = 0;
    int slbandwidth = 0;
    mmsx_t *pmmsx = NULL;

    lprintf("curl_mmsxsetupconnect:enter....\r\n");
    SET_RCVTIMEO(tv,10);
    slbandwidth = MMSX_BANDWIDE_DEFAULT;
    
    pmmsx = mmsx_connect(NULL, NULL, conn->data->change.url, slbandwidth);
    if (NULL == pmmsx)
    {
        lprintf("curl_mmsxconnect:call mmsx_connect return null.\r\n");
        return CURLE_FAILED_INIT;
    }

    conn->proto.generic = pmmsx;
    if(conn->sock[FIRSTSOCKET] != CURL_SOCKET_BAD)
        closesocket(conn->sock[FIRSTSOCKET]);

    if (NULL != pmmsx->connection)
    {
        conn->sock[FIRSTSOCKET] = pmmsx->connection->s;
    }
    else
    {
        conn->sock[FIRSTSOCKET] = pmmsx->connection_h->s;
    }

    return CURLE_OK;
}

static CURLcode curl_mmsxconnect(struct connectdata *conn, bool *done)
{
    lprintf("curl_mmsxconnect:enter....\r\n");

    return CURLE_OK;
}

static CURLcode curl_mmsxdo(struct connectdata *conn, bool *done)
{
    int slseekable = 0;
    unsigned int ulfilesize = 0;
    double dtimelength = 0.0;
    mmsx_t *pmmsxt = NULL;
    struct SessionHandle *pSessionHandleTemp = NULL;

    lprintf("curl_mmsxdo:enter....\r\n");
    pmmsxt = conn->proto.generic;
    if (NULL == pmmsxt)
    {
        lprintf("curl_mmsxdo:get pmmsxt null pointer.\r\n");
        return CURLE_MXXXMX_GETPOINTER_ERROR;
    }
    pSessionHandleTemp = conn->data;
    if (NULL == pSessionHandleTemp)
    {
        lprintf("curl_mmsxdo:get pSessionHandleTemp null pointer.\r\n");
        return CURLE_MXXXMX_GETPOINTER_ERROR;
    }

    if (((pmmsxt->connection) && (pmmsxt->connection->seekable == 1))||
       ((pmmsxt->connection_h) && (pmmsxt->connection_h->seekable == 1)))
    {
        slseekable = 2;
    }
    curl_easy_setopt(pSessionHandleTemp, CURLOPT_SEEK_INFO, slseekable);
    
    ulfilesize = mmsx_get_length(pmmsxt);
    dtimelength = mmsx_get_time_length(pmmsxt);
    lprintf("filesize=%d,seekable=%d,dtimelength=%f.\r\n",ulfilesize,slseekable,dtimelength);
    if (slseekable != 0)
    {
        curl_easy_setopt(pSessionHandleTemp, CURLOPT_TIME_LENGTH, dtimelength);
        Curl_setup_transfer(conn, FIRSTSOCKET,ulfilesize, FALSE, NULL, -1, NULL);    
    }
    else
    {
        curl_easy_setopt(pSessionHandleTemp, CURLOPT_TIME_LENGTH, 0.0);
        Curl_setup_transfer(conn, FIRSTSOCKET,-1, FALSE, NULL, -1, NULL);
    }

    conn->recv[FIRSTSOCKET] = curl_mmsxrecv;
    *done = TRUE;
    return CURLE_OK;  
}

static CURLcode curl_mmsxdone(struct connectdata *conn, CURLcode status,bool premature)
{
    (void)conn; /* unused */
    (void)status; /* unused */
    (void)premature; /* unused */

    lprintf("curl_mmsxdone:enter....\r\n");
    return CURLE_OK;
}

static CURLcode curl_mmsxdisconnect(struct connectdata *conn,bool dead_connection)
{
    mmsx_t *pmmsxt = NULL;

    lprintf("curl_mmsxdisconnect:enter....\r\n");
    pmmsxt = conn->proto.generic;
    if(pmmsxt) 
    {
        conn->proto.generic = NULL;
        mmsx_close(pmmsxt);
        conn->sock[FIRSTSOCKET]=CURL_SOCKET_BAD;
        conn->sock[SECONDARYSOCKET]=CURL_SOCKET_BAD;
    }
        
    return CURLE_OK;
}

static ssize_t curl_mmsxrecv(struct connectdata *conn, int sockindex, char *buf,size_t len, CURLcode *err)
{
    int slresult = 0;
    int nread = -1;
    unsigned int ulseekflag = 0;
    double dseekposition = 0.0;
    off_t sllcurrentpos = 0;
    mmsx_t *pmmsxt = NULL;
    struct SessionHandle *pSessionHandleTemp = NULL;  

    pmmsxt = conn->proto.generic;
    if (NULL == pmmsxt)
    {
        lprintf("curl_mmsxrecv:get pmmsxt null pointer.\r\n");
        *err = CURLE_MXXXMX_GETPOINTER_ERROR;
        return nread;
    }

    pSessionHandleTemp = conn->data;
    if (NULL == pSessionHandleTemp)
    {
        lprintf("curl_mmsxrecv:get pSessionHandleTemp null pointer.\r\n");
        *err = CURLE_MXXXMX_GETPOINTER_ERROR;
        return nread;
    }

    curl_easy_getinfo(pSessionHandleTemp, CURLINFO_SEEK_FLAG, &ulseekflag);
    if (1 == ulseekflag)
    {
        curl_easy_getinfo(pSessionHandleTemp, CURLINFO_SEEK_TIME, &dseekposition);
        if (0 == mmsx_time_seek(NULL,pmmsxt,dseekposition))
        {
            lprintf("curl_mmsxrecv:call mmsx_time_seek error.\r\n");
            *err = CURLE_MXXXMX_SEEK_ERROR;
            return nread;
        }
        sllcurrentpos = mmsx_get_current_pos(pmmsxt);
        curl_easy_setopt(pSessionHandleTemp,CURLOPT_CURRENT_POS,&sllcurrentpos);
        curl_easy_setopt(pSessionHandleTemp,CURLOPT_SEEK_FLAG,0);
        lprintf("mmsxrecv:seektime=%f,sllcurrentpos=%lld.\r\n",dseekposition,sllcurrentpos);      
    }
    
    nread = mmsx_read(NULL, pmmsxt, buf, curlx_uztosi(len));
    if (0 == nread)
    {
        if (((pmmsxt->connection) && (pmmsxt->connection->eos == 1))||
           ((pmmsxt->connection_h) && (pmmsxt->connection_h->eos == 1)))
        {
            conn->data->req.size = conn->data->req.bytecount;
        }
        else 
        {
            lprintf("curl_mmsxrecv:call mmsx_read error.\r\n");
            *err = CURLE_MXXXMX_READ_ERROR;
        }
    }

    return nread;
}
#endif 

