#include <sys_config.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>
#include <api/live555/live555.h>
#include "uf_rtsp_live555.h"

#ifdef RTSP_SUPPORT

#define UF_LIVE555_DEBUG 0

#ifdef UF_LIVE555_DEBUG
    #define UF_LIVE555_PRINT if (g_uf_live555_debug)libc_printf
#else
    #define UF_LIVE555_PRINT(...) do{}while(0)
#endif
#define UF_RTSP_WAIT_TMO 15*1000

//static UF_RTSP_USR_ACTION uf_rtsp_live555_get_user_cmd(urlfile *file);
//static void uf_rtsp_live555_set_user_cmd(urlfile *file, UF_RTSP_USR_ACTION action);
static void uf_rtsp_live555_work_task(UINT32 param1, UINT32 param2);
static void uf_rtsp_live555_wait_exit(urlfile *file);

static INT32 g_uf_live555_debug = 1;//0;
#if 1// dubug
static int g_dump_switch = 0;
static int g_dump_file_num = 0;
#endif
static BOOL uf_rtsp_priv_init(urlfile *file)
{
    uf_rtsp_private *priv= NULL;

#if 1// dubug
    char dump_file_name[64] = {0};
#endif
    if (NULL == file)
    {
        UF_LIVE555_PRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return FALSE;
    }

    priv = MALLOC(sizeof(uf_rtsp_private));
    if (NULL == priv)
    {
        UF_LIVE555_PRINT("%s:%d:malloc error.\n", __FUNCTION__,__LINE__);
        return FALSE;
    }
    MEMSET(priv,0,sizeof(uf_rtsp_private));

    #if 1// debug
    if (g_dump_switch)
    {
        sprintf(dump_file_name, "/mnt/uda1/rtsp_es_play_dump_%d.esx", g_dump_file_num);
        priv->dump = fopen(dump_file_name, "wb");
        sprintf(dump_file_name, "/mnt/uda1/rtsp_es_play_dump_%d.log", g_dump_file_num);
        priv->log =  fopen(dump_file_name, "wb");
        sprintf(dump_file_name, "/mnt/uda1/rtsp_es_play_dump_video_%d.es", g_dump_file_num);
        priv->dump_es_v= fopen(dump_file_name, "wb");
        sprintf(dump_file_name, "/mnt/uda1/rtsp_es_play_dump_video_%d.alip", g_dump_file_num);
        priv->dump_es_a= fopen(dump_file_name, "wb");
        
        g_dump_file_num++;
    }
    #endif

    priv->last_data_tick = osal_get_tick();

    file->private = (void*)priv;
    
    return TRUE;
}

static void uf_rtsp_priv_free(urlfile *file)
{
    uf_rtsp_private *priv = NULL;

    if ((NULL == file)||(NULL == file->private))
    {
        UF_LIVE555_PRINT("%s:%d:param error.\n", __FUNCTION__,__LINE__);
        return;
    }

    priv = (uf_rtsp_private*)file->private;
    
    if (file->private)
    {
        FREE(file->private);
        file->private = NULL;
    }

}
static int uf_rtsp_live555_init(urlfile *file)
{
    OSAL_T_CTSK t_ctsk;
    ID          tskid = OSAL_INVALID_ID;
    int         ret = CURLE_FAILED_INIT;  // always return fail

    if (NULL != file->private)
    {
        return ret;
    }

    if (FALSE == uf_rtsp_priv_init(file))
    {
        return ret;
    }

    // let's startup RTSP work task.
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x8000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)file;
    t_ctsk.para2 = 0;
    t_ctsk.name[0] = 'R';
    t_ctsk.name[1] = 'W';
    t_ctsk.name[2] = 'T';
    t_ctsk.task = (FP)uf_rtsp_live555_work_task;
    tskid = osal_task_create(&t_ctsk);
    
    if (tskid == OSAL_INVALID_ID)
    {
        return ret;
    }

    // set this state to ensure can uf_store_data
    uf_state(file, UF_OPEN);

    // OK, just dive into loop to wait play finished.
    // All work will do by uf_rtsp_live555_work_task
    uf_rtsp_live555_wait_exit(file);

    // Init always return FAIL to trigger _uf_trans_task exit, 
    // because when execute this line, RTSP play already teardown or fail.
    
    return ret; 
}

static void uf_rtsp_live555_release(urlfile *file)
{
    if (file == NULL)
    {
        return ;
    }

    free(file->private);
    file->private = NULL;
}

static int uf_rtsp_live555_seek(urlfile *file, off_t offset, int fromwhere)
{
    int ret = 0;
    uf_rtsp_private *priv = NULL;

    priv = file->private;
    
    ret = rtsp_live555_seek(priv->client, (double)offset);

    return ret;
}

static int uf_rtsp_live555_ioctl(urlfile *file, UINT32 cmd, UINT32 param)
{
    int ret = 0;
    double dura = 0.0;
    UINT32 total_time = 0.0;
    uf_rtsp_private *priv = NULL;

    priv = file->private;
    
    if (priv == NULL)
    {
        return 0;
    }

    switch (cmd)
    {
        case UF_IOCTL_MP_PAUSE:
            ret = rtsp_live555_pause(priv->client);
            break;
        case UF_IOCTL_MP_RESUME:
            ret = rtsp_live555_resume(priv->client, (double)param);
            break;
        case UF_IOCTL_GET_FILESIZE:
            // if you want to get total time, have to set ret = 0, and file size not eq zero, don't ask why.
            ret = 0;
            file->size = -1;
            *((off_t *)param) = file->size;
            break;
        case UF_IOCTL_GET_TOTALTIME:
            if (rtsp_live555_get_duration(priv->client, &dura))
            {
                total_time = dura;
                *((UINT32 *)param) = total_time; //  in seconds
                //libc_printf("%s, total_time %d\n", __FUNCTION__, total_time);
            }
            ret = 0; // we can not return -1
            break;
        default:
            ret = -1;
            break;
    }
    
    return ret;
}


static void uf_rtsp_live555_wait_exit(urlfile*file)
{
    uf_rtsp_private *priv = NULL;
    BOOL bstopped = FALSE;
    BOOL bwait_data_tmo = FALSE;
    BOOL ret = FALSE;
    double dura = 0.0;

    priv = file->private;

    // Just wait one moment to ensure work task is running
    osal_task_sleep(100);
    
    // Now wait user abort or connection fail
    while (TRUE == priv->b_running)
    {
        // rtsp work task still working
        // 1. just check is there any one set UF_CLOSE to kill this play
        // 2. if UF_CLOSE set, stop rtsp session and let rtsp work task exit        
        if ( bwait_data_tmo ||((bstopped == FALSE) && (UF_CLOSE == file->state)))
        {
            if (priv->client)
            {
                rtsp_live555_stop(priv->client); // set signal to exit rtsp work task
                rtsp_live555_close(priv->client);
                priv->client = NULL;
                bstopped = TRUE; // already stopped, don't stop again.
                UF_LIVE555_PRINT("Call rtsp_live555_close because USER stop\n");
            }
        }
        osal_task_sleep(200);

        dura = 0.0;
        ret = rtsp_live555_get_duration(priv->client, &dura);
        if ((ret = TRUE) && (dura > 0.0))
        {
            if ((priv->b_pause==FALSE) && ((osal_get_tick()-priv->last_data_tick)>UF_RTSP_WAIT_TMO))
            {
                bwait_data_tmo = TRUE;
            }
        }
    }
    
#if 1//debug
    if (priv->dump)
    {
        fclose(priv->dump);
    }
    if (priv->log)
    {
        fclose(priv->log);
    }
    if (priv->dump_es_v)
    {
        fclose(priv->dump_es_v);
    }
    if (priv->dump_es_a)
    {
        fclose(priv->dump_es_a);
    }
#endif
    UF_LIVE555_PRINT("%s exit\n", __FUNCTION__);
}

static int uf_rtsp_live555_log_cb(void *priv, unsigned char *data, unsigned long size)
{
    urlfile *file = (urlfile*)priv;
    uf_rtsp_private *uf_priv = (uf_rtsp_private*)file->private;
    
    #if 1//debug
    if (uf_priv && uf_priv->log)
    {
        fwrite(data, 1, size, uf_priv->log);
    }
    #endif
    return size;
}
static int uf_rtsp_live555_data_got(void *priv, unsigned char *data, unsigned long size)
{
    urlfile *file = (urlfile*)priv;
    uf_rtsp_private *uf_priv = (uf_rtsp_private*)file->private;


    #if 1//debug
    if (uf_priv && uf_priv->dump)
    {
        fwrite(data, 1, size, uf_priv->dump);
        UF_LIVE555_PRINT("x");
    }
    #endif
    uf_priv->last_data_tick = osal_get_tick(); // update the got data time
    return uf_store_data(file, data, size);

    return size;
}
static int uf_rtsp_live555_video_es_data_got(void *priv, unsigned char *data, unsigned long size)
{
    urlfile *file = (urlfile*)priv;
    uf_rtsp_private *uf_priv = (uf_rtsp_private*)file->private;


    #if 1//debug
    if (uf_priv && uf_priv->dump_es_v)
    {
        fwrite(data, 1, size, uf_priv->dump_es_v);
        UF_LIVE555_PRINT("x");
    }
    #endif

    return size;
}
static int uf_rtsp_live555_video_ali_es_data_got(void *priv, unsigned char *data, unsigned long size)
{
    urlfile *file = (urlfile*)priv;
    uf_rtsp_private *uf_priv = (uf_rtsp_private*)file->private;


    #if 1//debug
    if (uf_priv && uf_priv->dump_es_a)
    {
        fwrite(data, 1, size, uf_priv->dump_es_a);
        UF_LIVE555_PRINT("a");
    }
    #endif

    return size;
}

static void uf_rtsp_live555_work_task(UINT32 param1, UINT32 param2)
{
    urlfile *file = NULL;
    uf_rtsp_private *priv = NULL;
    int ret = -1;
    double dura = 0.0;
    
    file = (urlfile *)param1;

    if ((NULL == file) || (NULL == file->private))
    {
        UF_LIVE555_PRINT("%s,L:%d parameter error, rtsp main task exit\n", __FUNCTION__, __LINE__);
        return;
    }
    
    priv = (uf_rtsp_private*)file->private;

    // set flag to tell waiting loop that work task running
    priv->b_running = TRUE;

    // Keep client handle
    priv->client = rtsp_live555_open(file->url);

    if (priv->client)
    {
        if (rtsp_live555_get_duration(priv->client, &dura) && (dura > 0.0))
        {
            network_set_seek_type(2); // vod, set time seek
        }
        else
        {
            network_set_seek_type(0); // live, set disable seek
        }
        rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_PRIV_DATA, (void*)file);
        rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_DATA_CALLBACK, (void*)uf_rtsp_live555_data_got);
        rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_DEBUG_CALLBACK, (void*)uf_rtsp_live555_log_cb);
        rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_DB_V_DATA_CALLBACK, (void*)uf_rtsp_live555_video_es_data_got);
        //rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_DB_ALIP_V_DATA_CALLBACK, (void*)uf_rtsp_live555_video_ali_es_data_got);
        rtsp_live555_set_opt(priv->client, RTSP_LIVE555_SET_WAIT_DATA_TIMEOUT, (void*)(5*1000000)); // wait 5s timeout

        // Dive into play loop
        ret = rtsp_live555_play(priv->client);
    }
    
    if (ret < 0)
    {
        if (priv->client)
        {
            rtsp_live555_close(priv->client);
            priv->client = NULL;
            UF_LIVE555_PRINT("Call rtsp_live555_close because play error.\n");
        }
        // set state to let transfer task exit
        uf_state(file, UF_ERROR);
    }

    UF_LIVE555_PRINT("uf_rtsp_live555_work_task exit###########################\n");
    priv->b_running = FALSE;

}

#if 1
urlfile g_test_file;
static void _rtsp_test_task(UINT32 param1, UINT32 param2)
{
    uf_rtsp_live555_init(&g_test_file);

    uf_rtsp_live555_release(&g_test_file);

}

BOOL rtsp_test_open(char *url)
{
    OSAL_T_CTSK t_ctsk;
    ID tskid = OSAL_INVALID_ID;

    memset(&g_test_file, 0, sizeof(urlfile));
    strcpy(g_test_file.url, url);

    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x8000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)&g_test_file;
    t_ctsk.para2 = 0;
    t_ctsk.name[0] = 'U';
    t_ctsk.name[1] = 'F';
    t_ctsk.name[2] = 'T';
    t_ctsk.task = (FP)_rtsp_test_task;
    tskid = osal_task_create(&t_ctsk);
    if (tskid == OSAL_INVALID_ID)
    {
        return FALSE;
    }

    return TRUE;
}


void rtsp_test_close()
{
    g_test_file.state = UF_CLOSE;
}

#endif

/* RTSP is base on LIVE555, we have to merge it to our PE/URLFILE architecture,
   so when play RTSP url, it will do some curl init, but doesn't use it.*/
ufprotocol uf_rtsp_live555 =
{
    .next = NULL,
    .name = "rtsp",
    .init = uf_rtsp_live555_init,
    .release = uf_rtsp_live555_release,
    .seek = uf_rtsp_live555_seek,
    .ioctl = uf_rtsp_live555_ioctl,
    .curl_retproc = NULL,
    .curl_progcb = NULL,
    .data_recved = NULL,
};

#endif 

