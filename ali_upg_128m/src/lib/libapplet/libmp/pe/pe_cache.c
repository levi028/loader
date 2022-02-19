  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pe_cache.c
*
*    Description: This file describes play engine cache operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <modules.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <osal/osal_cache.h>
#include <osal/osal_mutex.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <api/libimagedec/imagedec.h>
#include <api/libmp/pe.h>
#ifdef SUPPORT_ES_PLAYER
#include <api/libstream/libstream.h>
int g_libstream_enable = 0;
#endif

#include "music/plugin.h"
#include "music/music_engine.h"
#include "image/image_engine.h"
#include "video_advanced/mp_player.h"
#ifdef MP4_CONTAINER
#include "av_player/av_player.h"
extern F_HANDLE m_mp4_test_file;
#endif
#include "play_engine_common.h"
#include "pe_flash.h"

#include <api/libcurl/urlfile.h>
#include <bus/dma/dma.h>

#ifdef DLNA_DMP_SUPPORT
extern char *dmp_get_res_uri(char *file_name);
#endif

extern void libnet_calc_speed(UINT32 bytes);
extern off_t get_content_size_api(FILE *stream);
int pe_cache_get_seektype(int cache_id);

#if 0
#define PECACHE_PRINTF    libc_printf
#else
#define PECACHE_PRINTF(...)    do{}while(0)
#endif

#define MAX_GET_DATA_FROM_PE_CACHE_TIMES        100
#define HTTP_HEADER_LEN         7
#define DLNA_DMP_HEADER_LEN     12

//#define NETWORK_DISABLE_SEEK
#define PE_CACHE_READ_TMO 1000

#ifdef  NETWORK_DISABLE_SEEK   
static volatile UINT8 m_seek_type = 0xFF;
#else
static volatile UINT8 m_seek_type = 0xFF;
#endif

#ifdef PE_CALLBACK_TO_APP_ENABLE
UINT8 app_run_callback=0;
#endif

static char gs_url_content_uri[256] = {0};
static off_t gs_url_content_siz = -1;

// set 0, no seek; 1, byte seek(normal); 2 time seek.
void network_set_seek_type(UINT8 seek_type)
{
#ifdef  NETWORK_DISABLE_SEEK   
    m_seek_type = 0;
#else
    m_seek_type = seek_type;
#endif
}

// return 0, no seek; 1, byte seek(normal); 2 time seek.
UINT8 network_get_seek_type(void)
{
    return m_seek_type;
}

UINT8 network_is_disable_seek(void)
{
    //current only byte seek we can support
    if ((1 == m_seek_type)||(2 == m_seek_type))
    {
        return 0;
    }
    else
    {
        return 1;
    }

    return 1;// default DISABLE SEEK
}


off_t network_get_url_content_size(char *url)
{
    if(!url)
    {
        libc_printf("%s: url is null\n", __FUNCTION__);
        return -1;
    }
    if (strcmp(url, gs_url_content_uri) == 0)
        return gs_url_content_siz;

    return -1;
}

void network_set_url_content_size(char* url, off_t size)
{
    if(!url)
    {
        libc_printf("%s: url is null\n", __FUNCTION__);
        return ;
    }
    strcpy(gs_url_content_uri, url);
	gs_url_content_uri[255] = 0;
    gs_url_content_siz = size;
}
#define PE_CACHE_NUM    2
#ifndef MAX_FILE_NAME_SIZE
#define MAX_FILE_NAME_SIZE                1024
#endif
#ifndef MAX_DIR_NAME_SIZE
#define MAX_DIR_NAME_SIZE                MAX_FILE_NAME_SIZE
#endif
#ifndef FULL_PATH_SIZE
#define FULL_PATH_SIZE                    (MAX_DIR_NAME_SIZE + MAX_FILE_NAME_SIZE + 1)
#endif
//#define LOG_PE_CACHE
//#define ENABLE_PE_FLAG
#define PE_FLAG_DATA_READY  0x00000001
#define PE_PREVIOUS_BUFFER_SIZE 10240
UINT32 pe_pre_buffer_size[PE_CACHE_NUM] = {10240, 10240};
#ifdef PE_CALLBACK_TO_APP_ENABLE        
pccache_callback_func app_callback;
#endif

#if 0//def  DUAL_ENABLE
#define PE_CACHE_MUTEX_CREATE osal_dual_mutex_create
#define PE_CACHE_MUTEX_LOCK osal_dual_mutex_lock
#define PE_CACHE_MUTEX_UNLOCK osal_dual_mutex_unlock
#define PE_CACHE_MUTEX_DELETE osal_dual_mutex_delete
#else
#define PE_CACHE_MUTEX_CREATE osal_mutex_create
#define PE_CACHE_MUTEX_LOCK osal_mutex_lock
#define PE_CACHE_MUTEX_UNLOCK osal_mutex_unlock
#define PE_CACHE_MUTEX_DELETE osal_mutex_delete
#endif

//static UINT32 pe_pre_buffer_size[PE_CACHE_NUM] = {10240, 10240}; /* */
#ifdef PE_CALLBACK_TO_APP_ENABLE
pccache_callback_func app_callback = NULL;
#endif

static pe_cache_cb_fp pe_cache_net_fp[PE_NET_FILE_TYPE_MAX];
static volatile ID pe_cache_mutex = OSAL_INVALID_ID;
//static volatile pe_cache *pe_cache_info = NULL; //[PE_CACHE_NUM];
static volatile pe_cache pe_cache_info[PE_CACHE_NUM];
//static volatile pe_cache_ex * pe_cache_info_ex = NULL;
//static pe_cache pe_cache_info[PE_CACHE_NUM];
static pe_cache_ex *pe_cache_info_ex = NULL;
static ID pe_task_id = OSAL_INVALID_ID;
static int pe_cache_count = 0;
static int pe_cache_release_flag = 0;
static pe_cache_cb_fp *g_reg_pecache_cb_hdl = NULL;
int pe_cache_use_sgdma = 0;
#if 0//def DUAL_ENABLE
static volatile pe_cache_cmd *pe_cache_cmd_buf = NULL;
#endif

#define RBUFF_PTR_ADD(P,S,O) ((P)+(O)<(S)?(P)+(O):(P)+(O)-(S))
#define RBUFF_PTR_DEC(P,S,O) ((P)-(O)>=0?(P)-(O):(S)+(P)-(O))

UINT32 pe_cache_get_data_len(int cache_id);



#if 0 //def  DUAL_ENABLE
static void pe_cache_process_cmd(void)
{
    char *filename = NULL;
    UINT8 *cache_buff = 0;
    UINT32 cache_size = 0;
    UINT32 block_size = 0;
    int cache_id = 0;
    off_t offset = 0;
    UINT32 *offset_p = NULL;
    int where = 0;
    int ret = 0;
    off_t file_size = 0;
    INT32 file_duration = 0;

    PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
    if(pe_cache_mutex != INVALID_ID && pe_cache_cmd_buf && pe_cache_cmd_buf->status == PE_CACHE_CMD_STATUS_NEW)
    {
        switch(pe_cache_cmd_buf->type)
        {
            case PE_CACHE_CMD_OPEN:
                filename = (char *)(pe_cache_cmd_buf->param[0]);
                cache_buff = (UINT8 *)(pe_cache_cmd_buf->param[1]);
                cache_size = pe_cache_cmd_buf->param[2];
                block_size = pe_cache_cmd_buf->param[3];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                cache_id = pe_cache_open(filename, cache_buff, cache_size, block_size);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->param[0] = cache_id;
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
             case PE_CACHE_CMD_CLOSE:
                cache_id = pe_cache_cmd_buf->param[0];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                pe_cache_close(cache_id);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
             case PE_CACHE_CMD_SEEK:
                cache_id = pe_cache_cmd_buf->param[0];
                offset_p = (UINT32 *)(&offset);
                *offset_p = pe_cache_cmd_buf->param[1];
                *(offset_p+1) = pe_cache_cmd_buf->param[2];
                where = pe_cache_cmd_buf->param[3];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                ret = pe_cache_seek(cache_id, offset, where);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->param[0] = ret;
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
            case PE_CACHE_CMD_FILE_SIZE:
                cache_id = pe_cache_cmd_buf->param[0];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                file_size = pe_cache_filesize(cache_id);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->param[0] = (UINT32)(file_size>>32);
                pe_cache_cmd_buf->param[1] = (UINT32)(file_size & 0xFFFFFFFF);
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
            case PE_CACHE_CMD_FILE_DURATION:
                cache_id = pe_cache_cmd_buf->param[0];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                file_duration = pe_cache_file_duration(cache_id);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->param[0] = (UINT32)file_duration;
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
            case PE_CACHE_CMD_FULL_USE:
                cache_id = pe_cache_cmd_buf->param[0];
                PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                pe_cache_full_use(cache_id);
                PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
            case PE_CACHE_CMD_GET_SEEK_TYPE:
                cache_id = pe_cache_cmd_buf->param[0];
                pe_cache_cmd_buf->param[0] = (UINT32)pe_cache_get_seektype(cache_id);
                pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
                break;
             default:
                break;
        }
    }
    PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
}
#endif

static int data_copy(int cache_id, void *dest, void *src, unsigned int len)
{
    unsigned int   ret = 0;
    unsigned int   dma_xfer_id = 0;

    if (cache_id < 0 || cache_id >= PE_CACHE_NUM) { return 0; }

    if (DMA_INVALID_CHA != pe_cache_info[cache_id].sgdma_id && len > 1024)
    {
        osal_cache_flush(src, len);
        dma_xfer_id = dma_copy(pe_cache_info[cache_id].sgdma_id, src, dest, len, 0);
        if (DMA_INVALID_XFR_ID != dma_xfer_id)
        {
            dma_wait(dma_xfer_id, DMA_WAIT_MODE_DEFALT);
            osal_cache_invalidate(dest, len);
            ret = len;
            //libc_printf("PeCache SGDMA success! [src:%p, dest:%p, len:%d]\n", src, dest, len);
        }
#if 0
        else
        {
            libc_printf("%s:%d: dma_copy failed!\n", __FUNCTION__, __LINE__);
        }
#endif
    }
	if(ret == len)
		return ret;
	else
	{
		MEMCPY(dest, src, len);
		return len;
	}
    //return ret == len ? ret : (int)MEMCPY(dest, src, len);
}

static int is_netfile(char *url)
{
    int ret = 0;
    ret = (strstr(url, "://") != NULL);
    ret |= (strstr(url, "dmpfs") != NULL);
    return ret;
}

static char *pe_cache_realpath(char *path)
{
#ifdef DLNA_SUPPORT
    char *real_uri = dlnaPlayerGetURI();
    // it should be effective in DMP or DMR
    if(real_uri && ((MEMCMP(path, "/dmpfs", 6) == 0) || (M3503Stream_get_dmr_active())))
    {
        return real_uri;
    }
    else
#endif
    {
        return path;
    }
}

static void pe_cache_task(UINT32 para1, UINT32 para2)
{
    int i = 0;
    unsigned int uldosleep = 2/*10*/;   //Some files(wav) will be distortion if this value is more than 5, so set it 2. --by william.zeng 20151026
    pe_cache    pc;
    pe_cache_ex pc_ex;
    size_t bytes_read = 0;
    size_t bytes_returned = 0;
    //UINT32 buf_cal_tick = 0;

    MEMSET(&pc, 0x0, sizeof (pc));
    MEMSET(&pc_ex, 0x0, sizeof (pc_ex));
    while(!pe_cache_release_flag)
    {
        uldosleep = 2/*10*/;  //Some files(wav) will be distortion if this value is more than 5, so set it 2. --by william.zeng 20151026
#if 0//def  DUAL_ENABLE
        pe_cache_process_cmd();
#endif
        for(i=0; i<PE_CACHE_NUM && !pe_cache_release_flag; i++)
        {
            bytes_returned = 0;
            PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
            if(PE_CACHE_OPENED == pe_cache_info_ex[i].status)
            {
                PE_CACHE_MUTEX_LOCK(pe_cache_info_ex[i].mutex, OSAL_WAIT_FOREVER_TIME);
          MEMCPY(&pc, (const void *)&pe_cache_info[i], sizeof(pe_cache));
                MEMCPY(&pc_ex, (const void *)&pe_cache_info_ex[i], sizeof(pe_cache_ex));
                PE_CACHE_MUTEX_UNLOCK(pe_cache_info_ex[i].mutex);

                #ifdef NETWORK_SUPPORT
                if(pe_cache_info[i].netfile && pc.file_read == (pe_cache_read_fp)uf_read)
                {
                    if(pe_cache_info[i].file_size == 0)
                    {
                        if (uf_ioctl((urlfile *)pc.fp, UF_IOCTL_GET_FILESIZE, (UINT32)&pe_cache_info[i].file_size) == -1)
                        {
                            pe_cache_info[i].file_size = -1;
                        }
                    }
                    if (pe_cache_info[i].file_duration == 0 && pe_cache_info[i].file_size != 0)
                    {
                        if(uf_ioctl((urlfile *)pc.fp, UF_IOCTL_GET_TOTALTIME, (UINT32)&pe_cache_info[i].file_duration)==-1)
                        {
                            pe_cache_info[i].file_duration = -1;
                        }
                    }
                }
                #endif
                //if((pc.data_len < pc.cache_size) && (PE_FILE_READING == pc.sub_status))
                //pe_cache have a previous buffer can't be over write
                if((pc_ex.data_len < (pc_ex.cache_size - pe_pre_buffer_size[i])) \
                    && (PE_FILE_READING == pc_ex.sub_status))
                {
                    if(pc_ex.wr_pos >= pc_ex.rd_pos)
                    {
                        bytes_read = pc_ex.cache_size - pc_ex.wr_pos;
                        if(pc_ex.wr_pos == 0)
						{
						    bytes_read--;
						}
                    }
                    else
                    {
                        //bytes_read = pc.rd_pos - pc.wr_pos;
                        //pe_cache have a previous buffer can't be over write
                        bytes_read = pc_ex.rd_pos - pc_ex.wr_pos - pe_pre_buffer_size[i];
                    }
                    if(bytes_read > pc.block_size)
                    {
                        bytes_read = pc.block_size;
                    }
                    bytes_returned = pc.file_read(&pc_ex.cache_buff[pc_ex.wr_pos], bytes_read, 1, pc.fp);
#if 0//def DUAL_ENABLE
                    osal_cache_flush(&pc_ex.cache_buff[pc_ex.wr_pos], bytes_read);
#endif

                    PE_CACHE_MUTEX_LOCK(pe_cache_info_ex[i].mutex, OSAL_WAIT_FOREVER_TIME);

                    bytes_returned = bytes_returned>0 ? bytes_returned : 0;
                    if (pc.file_tell)
                    {
                              pe_cache_info_ex[i].file_offset = pc.file_tell(pc.fp);
                    }
                    else
                    {
                           pe_cache_info_ex[i].file_offset +=bytes_returned;
                    }
                    if(!bytes_returned)
                    {
                        if(pc.file_eof!=NULL) 
                        {
                            if (pc.file_eof(pc.fp)) 
                            {
                                pe_cache_info_ex[i].sub_status = PE_FILE_EOF;
                            }
                            else
                            {
                                if (!pe_cache_info[i].netfile)
                                {
                                    pe_cache_info_ex[i].sub_status = PE_FILE_FAILED;
                                }
                            }
                        }
                        else
                        {
                               pe_cache_info_ex[i].sub_status =PE_FILE_EOF;
                        }
                    }
                    else
                    {
                        int seektype = pc.file_seek ? PE_SEEK_TYPE_BYTE : PE_SEEK_TYPE_DISABLE;
                        #if defined(NETWORK_SUPPORT) && !defined(_INVW_JUICE)
                        if(pe_cache_info[i].netfile)
                            uf_ioctl((urlfile *)pc.fp, UF_IOCTL_GET_SEEKTYPE, (UINT32)&seektype);
                        #endif
                        pe_cache_info_ex[i].sub_status = PE_FILE_READING;
                        pe_cache_info[i].seek_type = seektype;
                        pe_cache_info[i].open_success = 1;
                        pe_cache_info_ex[i].wr_pos += (UINT32)bytes_returned;
                        if(pe_cache_info_ex[i].wr_pos >= pe_cache_info_ex[i].cache_size)
                        {
                            pe_cache_info_ex[i].wr_pos -= pe_cache_info_ex[i].cache_size;
                        }
                        pe_cache_info_ex[i].data_len += (UINT32)bytes_returned;
                        uldosleep = 1;
                    }
                    
#ifdef ENABLE_PE_FLAG
                    if(pe_cache_info[i].req_size)
                    {
                        pe_cache_info[i].req_size = 0;
                        osal_flag_set(pe_cache_info[i].flag, PE_FLAG_DATA_READY);
                    }
#endif
                    PE_CACHE_MUTEX_UNLOCK(pe_cache_info_ex[i].mutex);
                }
                #ifdef NETWORK_SUPPORT
                if(pe_cache_info[i].netfile)
                {
                    libnet_calc_speed(bytes_returned);
                    #if 0
                    if(osal_get_tick()-buf_cal_tick>1000)
                    {
                        libc_printf("PeCache Data:%2d%%\r",(100*pe_cache_info_ex[i].data_len)/pe_cache_info_ex[i].cache_size);
                        buf_cal_tick=osal_get_tick();
                    }
                    #endif
                }
                #endif
            }
        PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
        }
        osal_task_sleep(uldosleep);
    }
    pe_task_id = OSAL_INVALID_ID;
    PECACHE_PRINTF("%s exit!\n", __FUNCTION__);
}

int pe_cache_init(void)
{
    int i = 0;
    OSAL_T_CTSK t_ctsk;
    __MAYBE_UNUSED__ int ret = -1;

    if(pe_task_id != OSAL_INVALID_ID)
    {
        return 0;
    }

    MEMSET(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
    if(pe_cache_info_ex == NULL)
    {
#if 0//def  DUAL_ENABLE
        pe_cache_info_ex = (pe_cache_ex *)malloc_sm(sizeof(pe_cache_ex)*PE_CACHE_NUM);
#else
        pe_cache_info_ex = (pe_cache_ex *)MALLOC(sizeof(pe_cache_ex)*PE_CACHE_NUM);
#endif
        if(pe_cache_info_ex == NULL)
        {
            ASSERT(0);
            return -1;
        }
        MEMSET((void *)pe_cache_info_ex, 0, (sizeof(pe_cache_ex)*PE_CACHE_NUM));
        for(i=0; i<PE_CACHE_NUM; i++)
        {
            pe_cache_info_ex[i].mutex = INVALID_ID;
        }
    }

    for(i=0; i<PE_CACHE_NUM; i++)
    {
        if (pe_cache_info_ex[i].mutex != INVALID_ID)
        {
            PE_CACHE_MUTEX_DELETE(pe_cache_info_ex[i].mutex);
            pe_cache_info_ex[i].mutex = INVALID_ID;
        }    
    }
    
    MEMSET((void *)pe_cache_info, 0, (sizeof(pe_cache)*PE_CACHE_NUM));
    MEMSET((void *)pe_cache_info_ex, 0, (sizeof(pe_cache_ex)*PE_CACHE_NUM));
    for(i=0; i<PE_CACHE_NUM; i++)
    {
        pe_cache_info_ex[i].status = PE_CACHE_CLOSED;
        pe_cache_info_ex[i].mutex = PE_CACHE_MUTEX_CREATE();
        if(pe_cache_info_ex[i].mutex == OSAL_INVALID_ID)
            return -1;
#ifdef ENABLE_PE_FLAG
        pe_cache_info[i].flag = osal_flag_create(0);
        if(pe_cache_info[i].flag == OSAL_INVALID_ID)
            return -1;
#endif
    }
    for(i=0; i<PE_NET_FILE_TYPE_MAX;i++)
    {//init callback for net file
        pe_cache_net_fp[i].file_open = NULL;
        pe_cache_net_fp[i].file_read = NULL;
        pe_cache_net_fp[i].file_seek = NULL;
        pe_cache_net_fp[i].file_eof = NULL;
        pe_cache_net_fp[i].file_tell = NULL;
        pe_cache_net_fp[i].file_close = NULL;
    }
    if(pe_cache_mutex == INVALID_ID)
    {
        pe_cache_mutex = PE_CACHE_MUTEX_CREATE();
    }

    ASSERT(pe_cache_mutex != INVALID_ID);

#if 0//def  DUAL_ENABLE
    if(pe_cache_cmd_buf == NULL)
    {
        pe_cache_cmd_buf = (pe_cache_cmd *)malloc_sm(sizeof(pe_cache_cmd));
        if(pe_cache_cmd_buf == NULL)
        {
            ASSERT(0);
            return -1;
        }

        pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
        pe_cache_cmd_buf->type = PE_CACHE_CMD_NULL;
    }
    else
    {
        ASSERT(0);
    }
    ret = video_engine_pe_cache_init(pe_cache_info_ex, pe_cache_cmd_buf, pe_cache_mutex);
    if(ret!=TRUE)
        return -1;
#endif

    if(pe_task_id == OSAL_INVALID_ID)
    {
        t_ctsk.stksz = 0x2000;
        t_ctsk.quantum    = 4;//10//4
        t_ctsk.itskpri    = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL//OSAL_PRI_HIGH
        t_ctsk.name[0]    = 'P';
        t_ctsk.name[1]    = 'E';
        t_ctsk.name[2]    = 'C';
        t_ctsk.task = (FP)pe_cache_task;
        pe_cache_release_flag = 0;
        pe_task_id = osal_task_create(&t_ctsk);
        ASSERT(pe_task_id != OSAL_INVALID_ID);
    }
    else
    {
        ASSERT(0);
    }

    return 0;
}

int pe_cache_release(void)
{
    int i = 0;
    __MAYBE_UNUSED__ int ret = -1;

#if 0
    // bug : playing video, plug out usb device, system reset.
    // solution:
    // don't free pe_cache_info, becasue pe_cache_close will use this structure.

    if(pe_cache_info)
    {
        for(i=0; i<PE_CACHE_NUM; i++)
        {
            PE_CACHE_MUTEX_DELETE(pe_cache_info[i].mutex);
        }
#ifdef  DUAL_ENABLE
        free_sm(pe_cache_info, sizeof(pe_cache)*PE_CACHE_NUM);
#else
        FREE(pe_cache_info);
#endif
        pe_cache_info = NULL;


    }
    else
    {
        ASSERT(0);
    }
#endif

    if(pe_task_id != OSAL_INVALID_ID)
    {
#if 0
        osal_task_delete(pe_task_id);
        pe_task_id = OSAL_INVALID_ID;
#else
        PECACHE_PRINTF("waitting for pe cache task exit...\n");
        pe_cache_release_flag = 1;
        while(pe_task_id != OSAL_INVALID_ID)
        {
            osal_task_sleep(10);
        }
#endif
    }
    
#if 0//def DUAL_ENABLE
    if(pe_cache_cmd_buf)
    {
        if(pe_cache_mutex != OSAL_INVALID_ID)
        {
            PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
        }
        free_sm((UINT32 *)pe_cache_cmd_buf, sizeof(pe_cache_cmd));
        pe_cache_cmd_buf = NULL;
        if(pe_cache_mutex != OSAL_INVALID_ID)
        {
            PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
        }
    }
    /*
    else
    {
        //ASSERT(0);
    }*/

    // Roman add at 20100705 to fix bug:
    // jpeg+mp3 burning test crash.
    ret = video_engine_pe_cache_release();
    if(ret!=TRUE)
        return -1;
#endif
    if(pe_cache_mutex != OSAL_INVALID_ID)
    {
        PE_CACHE_MUTEX_DELETE(pe_cache_mutex);
        pe_cache_mutex = OSAL_INVALID_ID;
    }

    // avoid plug-out USB system wait when playing Video
    if (pe_cache_info_ex)
    {
        for(i=0; i<PE_CACHE_NUM; i++)
        {
            pe_cache_info_ex[i].sub_status = PE_FILE_FAILED;
        }
    }
    return 0;
}

void pe_cache_register_net_fp(pe_cache_cb_fp net_cb_fp,int type)
{
    if(type >=PE_NET_FILE_TYPE_MAX)
    {
        return;
    }
    pe_cache_net_fp[type].file_open= net_cb_fp.file_open;
    pe_cache_net_fp[type].file_close = net_cb_fp.file_close;
    pe_cache_net_fp[type].file_eof = net_cb_fp.file_eof;
    pe_cache_net_fp[type].file_read = net_cb_fp.file_read;
    pe_cache_net_fp[type].file_seek = net_cb_fp.file_seek;
    pe_cache_net_fp[type].file_tell = net_cb_fp.file_tell;
}

#ifdef PE_CALLBACK_TO_APP_ENABLE
UINT8 app_run_callback=0;
void pe_cache_register_callback_func(pccache_callback_func callback)
{
    if(callback != NULL)
        app_callback=callback;
}

#endif
/*
void pe_cache_cleanup()
{
    int i;

    for(i=0; i<PE_CACHE_NUM; i++)
    {
        pe_cache_info[i].status = PE_CACHE_CLOSED;
        osal_mutex_delete(pe_cache_info[i].mutex);
#ifdef ENABLE_PE_FLAG
        osal_flag_delete(pe_cache_info[i].flag);
#endif
    }

    osal_mutex_delete(pe_cache_mutex);
    pe_cache_mutex = INVALID_ID;

    osal_task_delete(pe_task_id);
    pe_task_id = OSAL_INVALID_ID;
}
*/

int pe_cache_open(char *filename, UINT8 *cache_buff, UINT32 cache_size, UINT32 block_size)
{
    int i = 0;
    int cache_id = -1;
    char *file_path = filename;
    int ret = -1;

	PECACHE_PRINTF("pe cache open: [%s]\n", filename);

    if(!filename)
    {
        libc_printf("%s: filename is null\n", __FUNCTION__);
        return  -1;
    }
    PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
    for(i=0; i<PE_CACHE_NUM; i++)
    {
        if(PE_CACHE_CLOSED == pe_cache_info_ex[i].status)
        {
#ifdef SUPPORT_ES_PLAYER
            g_libstream_enable = 0;
#endif
            file_path = pe_cache_realpath(filename);

            if(NULL == cache_buff)
            {
                cache_buff = (UINT8 *)MALLOC(cache_size);
                if(NULL == cache_buff)
                {
                    PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
                    PECACHE_PRINTF("%s:%d: malloc failed!\n", __FUNCTION__,__LINE__);
                    return -1;
                }
                pe_cache_info[i].internal_cache = 1;
            }
            else
            {
                pe_cache_info[i].internal_cache = 0;
            }
            pe_cache_info[i].file_size = 0;
            pe_cache_info[i].netfile = is_netfile(file_path);
            pe_cache_info[i].file_duration = 0;
            pe_cache_info[i].sgdma_id = DMA_INVALID_CHA;

            if(g_reg_pecache_cb_hdl)
            {
                pe_cache_info[i].file_read = g_reg_pecache_cb_hdl->file_read;
                pe_cache_info[i].file_seek = g_reg_pecache_cb_hdl->file_seek;
                pe_cache_info[i].file_eof = g_reg_pecache_cb_hdl->file_eof;
                pe_cache_info[i].file_tell = g_reg_pecache_cb_hdl->file_tell;
                pe_cache_info[i].file_close = g_reg_pecache_cb_hdl->file_close;
                pe_cache_info[i].seek_type = 0;
                pe_cache_info[i].open_success = 1;
                pe_pre_buffer_size[i] = 10240;
                pe_cache_info[i].fp = g_reg_pecache_cb_hdl->file_open(file_path, cache_buff,cache_size,0);
                if(NULL == pe_cache_info[i].fp)
                {
                    if(pe_cache_info[i].internal_cache)
                        FREE(cache_buff);
                    break;
                }
                pe_cache_info[i].file_seek(pe_cache_info[i].fp, 0, SEEK_END);
                pe_cache_info[i].file_size = pe_cache_info[i].file_tell(pe_cache_info[i].fp);
                pe_cache_info[i].file_seek(pe_cache_info[i].fp, 0, SEEK_SET);
            }
            else if (pe_cache_info[i].netfile)
            {//Net file
#ifdef NETWORK_SUPPORT
               pe_cache_cb_fp fp;
               //libnet_set_seek_option(file_path);
               fp.file_open = (pe_cache_open_fp)uf_open;
               fp.file_close = (pe_cache_fclose_fp)uf_close;
               fp.file_read = (pe_cache_read_fp)uf_read;
               fp.file_tell = (pe_cache_ftell_fp)uf_tell;
               fp.file_eof = (pe_cache_feof_fp)uf_eof;
               fp.file_seek = (pe_cache_seek_fp)uf_seek;
               pe_cache_register_net_fp(fp,i);        
            
#ifdef _INVW_JUICE
                pe_pre_buffer_size[i] = 10240;
#else
                pe_pre_buffer_size[i] = 2*1024*1024;
                if(cache_size <= pe_pre_buffer_size[i])
                {
                    pe_pre_buffer_size[i] = block_size;//cache_size/2;
                }
#endif
#ifdef _INVW_JUICE
                            //file_path, buf, cache length, offset
                pe_cache_info[i].fp = pe_cache_net_fp[PE_NET_FILE_TYPE_URL].file_open(file_path,cache_buff,0x120000,0);
#else
                pe_cache_info[i].fp = (FILE *)uf_open(file_path, 0);
#endif
                if(NULL == pe_cache_info[i].fp)
                {
                    if(pe_cache_info[i].internal_cache)
                    {
                        FREE(cache_buff);
                        cache_buff = NULL;
                    }
                    break;
                }
#ifdef _INVW_JUICE
                pe_cache_info[i].file_read = pe_cache_net_fp[PE_NET_FILE_TYPE_URL].file_read;//url_read;
#else
                pe_cache_info[i].file_read = pe_cache_net_fp[i].file_read;//url_read;
#endif

                pe_cache_info[i].file_seek = pe_cache_net_fp[i].file_seek;//NULL;//url_seek
                pe_cache_info[i].file_eof =  pe_cache_net_fp[i].file_eof;//NULL;
                pe_cache_info[i].file_tell = pe_cache_net_fp[i].file_tell;//NULL;//url_tell

#ifdef _INVW_JUICE
                pe_cache_info[i].file_close = pe_cache_net_fp[PE_NET_FILE_TYPE_URL].file_close;//url_close
#else
                pe_cache_info[i].file_close = pe_cache_net_fp[i].file_close;//url_close
#endif                
                pe_cache_info[i].seek_type = 0;
                pe_cache_info[i].open_success = 0;
#endif
            }
#ifdef SUPPORT_ES_PLAYER
            else if(1 == is_ali_stream(file_path))
            {
                //libc_printf("%s : cache size %d block size %d\n", __FUNCTION__, cache_size, block_size);
                g_libstream_enable = 1;
                pe_pre_buffer_size[i] = cache_size;//10240;
                pe_cache_info[i].fp = pe_cache_net_fp[PE_ALI_SPECIAL_STREAM].file_open(file_path
                    , NULL, 0, 0);
                if(NULL == pe_cache_info[i].fp)
                {
                    if(pe_cache_info[i].internal_cache)
                    {
                        FREE(cache_buff);
                        cache_buff = NULL;
                    }
                    break;
                }
                pe_cache_info[i].seek_type = 1;
                pe_cache_info[i].open_success = 1;
                pe_cache_info[i].file_read = pe_cache_net_fp[PE_ALI_SPECIAL_STREAM].file_read;
                pe_cache_info[i].file_seek = pe_cache_net_fp[PE_ALI_SPECIAL_STREAM].file_seek;
                pe_cache_info[i].file_eof = NULL;
                pe_cache_info[i].file_tell = pe_cache_net_fp[PE_ALI_SPECIAL_STREAM].file_tell;
                pe_cache_info[i].file_close = pe_cache_net_fp[PE_ALI_SPECIAL_STREAM].file_close;
                //pe_cache_info_ex[i].file_size = (long long)LS_END_OFFSET;
            }            
#endif        
            else
            {//HDD file
                pe_cache_info[i].seek_type = 1;
                pe_cache_info[i].open_success = 1;
                pe_pre_buffer_size[i] = 10240;
                pe_cache_info[i].fp = fopen(file_path, "rb");
                if(NULL == pe_cache_info[i].fp)
                {
                    if(pe_cache_info[i].internal_cache)
                    FREE(cache_buff);
                    cache_buff = NULL;
                    break;
                }
                pe_cache_info[i].file_read = fread;
                pe_cache_info[i].file_seek = fseek;
                pe_cache_info[i].file_eof = feof;
                pe_cache_info[i].file_tell = ftell;
                pe_cache_info[i].file_close = fclose;
                ret = fseek(pe_cache_info[i].fp, 0, SEEK_END);
                if(ret<0)
                    ali_trace(&ret);
                pe_cache_info[i].file_size = ftell(pe_cache_info[i].fp);
                ret = fseek(pe_cache_info[i].fp, 0, SEEK_SET);
                if(ret<0)
                    ali_trace(&ret);
            }
        #ifdef MP4_CONTAINER
        m_mp4_test_file = pe_cache_info[i].fp;
        #endif
            pe_cache_info_ex[i].file_offset =0;//init file offset
            pe_cache_info_ex[i].cache_buff = cache_buff;
            pe_cache_info_ex[i].cache_size = cache_size;//(cache_size/block_size)*block_size;
            pe_cache_info[i].block_size = block_size;
            pe_cache_info_ex[i].data_len = 0;
            pe_cache_info_ex[i].rd_pos = 0;
            pe_cache_info_ex[i].wr_pos = 0;
            pe_cache_info_ex[i].status = PE_CACHE_OPENED;
            pe_cache_info_ex[i].sub_status = PE_FILE_READING;
#ifdef PE_CALLBACK_TO_APP_ENABLE
         if(app_callback !=NULL)
                 pe_cache_info[i].callback= app_callback;
#endif
#ifdef ENABLE_PE_FLAG
            pe_cache_info[i].req_size = 0;
#endif
#ifdef LOG_PE_CACHE
            pe_cache_info[i].read_delay = 0;
            pe_cache_info[i].read_count = 0;
#endif
         pe_cache_count++;
            cache_id = i;
            break;
        }
    }
    PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);

    if(cache_id != -1 && pe_cache_use_sgdma)
    {
        osal_task_dispatch_off();
        pe_cache_info[cache_id].sgdma_id = dma_open(DMA_CHAANY, 0, NULL);
        osal_task_dispatch_on();
        PECACHE_PRINTF("pe cache [%d] dma channel:%d\n", cache_id, pe_cache_info[cache_id].sgdma_id);
    }
    PECACHE_PRINTF("pe cache open:%d\n", cache_id);
    return cache_id;
}

//important!!!must set the cache_id to -1 and set cache_buff to NULL, after call pe_cache_close
UINT32 pe_cache_close(int cache_id)
{
    if((cache_id < 0) ||(cache_id >= PE_CACHE_NUM))
    {
        return 0;
    }

    PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
    if(PE_CACHE_OPENED == pe_cache_info_ex[cache_id].status || PE_CACHE_CLOSING == pe_cache_info_ex[cache_id].status)
    {
        //if(pe_cache_info[cache_id].fp &&pe_cache_info[cache_id].file_close!=NULL)//yuliang_net
        // fclose(pe_cache_info[cache_id].fp);
        if(pe_cache_info[cache_id].fp)
        {
            pe_cache_info[cache_id].file_close(pe_cache_info[cache_id].fp);
            pe_cache_info[cache_id].fp = NULL;
        }
        if(pe_cache_info[cache_id].internal_cache)
        {
            FREE(pe_cache_info_ex[cache_id].cache_buff);
            pe_cache_info_ex[cache_id].cache_buff = NULL;
        }
        pe_cache_info_ex[cache_id].status = PE_CACHE_CLOSED;
        pe_cache_info[cache_id].file_size = 0;
        pe_cache_info[cache_id].file_duration = 0;
        pe_cache_info[cache_id].open_success = 0;
        if (pe_cache_info[cache_id].sgdma_id != DMA_INVALID_CHA)
        {
            osal_task_dispatch_off();
            dma_close(pe_cache_info[cache_id].sgdma_id);
            osal_task_dispatch_on();
        }
        pe_cache_count --;
    }
    m_seek_type = 0xFF;
    PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
    PECACHE_PRINTF("pe cache close:%d\n", cache_id);
    return 0;
}

/*
void pe_cache_reset_info()
{
#ifdef LOG_PE_CACHE
    int i;
    for(i=0; i<PE_CACHE_NUM; i++)
    {
        pe_cache_info[i].read_delay = 0;
        pe_cache_info[i].read_count = 0;
    }
#endif
}
*/

/*
void pe_cache_print_info()
{
#ifdef LOG_PE_CACHE
    int i;
    for(i=0; i<PE_CACHE_NUM; i++)
        libc_printf("cache_id=%d, delay=%d, count=%d\n", i, pe_cache_info[i].read_delay, pe_cache_info[i].read_count);
#endif
}
*/

UINT32 pe_cache_read(int cache_id, UINT8 *buff, UINT32 size, UINT32 threshold)
{
    UINT32 bytes_returned = 0;
    volatile pe_cache_ex *pc_ex = NULL; 
#ifdef PE_CALLBACK_TO_APP_ENABLE
    UINT8 get_data_times=0;
#endif
    UINT32 tick_begin = 0;

    if(!buff)
    {
        libc_printf("%s: buff is null\n", __FUNCTION__);
        return -1;
    }
    ASSERT((UINT32)cache_id < PE_CACHE_NUM);
    if (cache_id <0 ||cache_id >= PE_CACHE_NUM)
    {
        return 0;
    }

    pc_ex = &pe_cache_info_ex[cache_id];  
    if(PE_CACHE_AUTO_THRESHOLD == threshold)
    {
        threshold = (pe_cache_count > 1) ? (pc_ex->cache_size - pe_pre_buffer_size[cache_id]) : 0;
    }

#ifdef LOG_PE_CACHE
    UINT32 delta = osal_get_tick() - pc->read_tick;
    if(delta)
        PECACHE_PRINTF("%s[%d]: size=0x%x, threshold=0x%x, data_len=0x%x, speed=%d KB/s\n", __FUNCTION__, cache_id, 
                    size, threshold, pe_cache_get_data_len(cache_id), pc->read_size/delta);
    pc->read_size = size;
    UINT32 ts = osal_get_tick();
#endif

    if(size > threshold)
    {
        threshold = size;
    }
    tick_begin = osal_get_tick();
#ifdef ENABLE_PE_FLAG
    if(threshold > pe_cache_get_data_len(cache_id) && !pe_cache_check_eof(cache_id))
    {
        UINT32 flgptn = 0;
        BOOL wait_data = TRUE;
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        if(PE_FILE_FAILED == pc_ex->sub_status)
        {
            PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
            return 0;
        }
        pc->req_size = size;
        PE_CACHE_MUTEX_UNLOCK(pc->mutex);
        osal_flag_wait(&flgptn, pc->flag, PE_FLAG_DATA_READY, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
    }
#else
    while((threshold > pe_cache_get_data_len(cache_id)) && (!pe_cache_check_eof(cache_id)))
    {
#ifdef PE_CALLBACK_TO_APP_ENABLE
            if((pc->callback != NULL)&&(get_data_times > MAX_GET_DATA_FROM_PE_CACHE_TIMES))
            {
                get_data_times=0;
                app_run_callback=1;
                pc->callback(1,1);
            }
            osal_task_sleep(1);
        if(pc->callback != NULL)
            get_data_times++;
#else
        if ((osal_get_tick() - tick_begin) > PE_CACHE_READ_TMO)
        {
            if((PE_FILE_EOF == pc_ex->sub_status) && (0 == pe_cache_get_data_len(cache_id)) )
                return 0x7FFFFFFF;
            //PECACHE_PRINTF("pe_cache_read see, wait data time out\n");
            return 0;
        }

        osal_task_sleep(1);
#endif
    }
#endif

    if(PE_CACHE_CLOSING == pe_cache_info_ex[cache_id].status)
    {
        PECACHE_PRINTF("pe cache closing\n");
        return 0x7FFFFFFF;
    }

#ifdef PE_CALLBACK_TO_APP_ENABLE
    if(1 == app_run_callback)
    {
            pc->callback(0,0);
            app_run_callback=0;
    }
#endif
#ifdef LOG_PE_CACHE
    pc->read_delay += osal_get_tick() - ts;
    pc->read_count++;
#endif

    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        do
        {
            if(PE_FILE_FAILED == pc_ex->sub_status)
            {
                break;
            }

            if(pc_ex->data_len)
            {
                if(pc_ex->rd_pos < pc_ex->wr_pos)
                {
                    bytes_returned = pc_ex->wr_pos - pc_ex->rd_pos;
                    if(bytes_returned > size)
                    {
                        bytes_returned = size;
                    }
					#if 1
                    MEMCPY(buff, &pc_ex->cache_buff[pc_ex->rd_pos], bytes_returned);
                    #else
                    data_copy(cache_id, buff, &pc_ex->cache_buff[pc_ex->rd_pos], bytes_returned);
                    #endif
                    pc_ex->rd_pos += bytes_returned;
                }
                else
                {
                    bytes_returned = pc_ex->cache_size - pc_ex->rd_pos;
                    if(bytes_returned > size)
                    {
                        bytes_returned = size;
                    }
                    #if 1
                    MEMCPY(buff, &pc_ex->cache_buff[pc_ex->rd_pos], bytes_returned);
                    #else
                    data_copy(cache_id, buff, &pc_ex->cache_buff[pc_ex->rd_pos], bytes_returned);
                    #endif
                    pc_ex->rd_pos += bytes_returned;
                    if(pc_ex->rd_pos >= pc_ex->cache_size)
                    {
                        pc_ex->rd_pos = 0;
                    }

                    size -= bytes_returned;
                    if(size)
                    {
                        if(size > pc_ex->wr_pos)
                        {
                            size = pc_ex->wr_pos;
                        }
                        #if 1                        
                        MEMCPY(&buff[bytes_returned], pc_ex->cache_buff, size);
                        #else
                        data_copy(cache_id, &buff[bytes_returned], pc_ex->cache_buff, size);
                        #endif
                        pc_ex->rd_pos = size;
                        bytes_returned += size;
                    }
                }
                pc_ex->data_len -= bytes_returned;
            }
        }while(0);
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }

#ifdef LOG_PE_CACHE
    delta = osal_get_tick() - ts;
    if(delta)
        PECACHE_PRINTF("read_speed=%d KB/s, read_time=%d\n", bytes_returned/delta, delta);
    else
        PECACHE_PRINTF("read_speed=[MAX]\n");
    pc->read_tick = osal_get_tick();
#endif

    return bytes_returned;
}

UINT32 pe_cache_get(int cache_id, UINT8 **ptr, UINT32 size, UINT32 threshold)
{
    UINT32 bytes_returned = 0;
    UINT32 tick_begin = 0;
    volatile pe_cache_ex *pc_ex = NULL;

    if(!ptr)
    {
        libc_printf("%s: ptr is null \n", __FUNCTION__);
        return 0;
    }
    
    if (cache_id <0 || cache_id >= PE_CACHE_NUM)
    {
        return 0;
    }
    pc_ex = &pe_cache_info_ex[cache_id];
	ASSERT((UINT32)cache_id < PE_CACHE_NUM);
    if(PE_CACHE_AUTO_THRESHOLD == threshold)
    {
        threshold = (pe_cache_count > 1) ? pc_ex->cache_size : 0;
    }

#ifdef LOG_PE_CACHE
    UINT32 delta = osal_get_tick() - pc->read_tick;
    if(delta)
        PECACHE_PRINTF("%s[%d]: size=0x%x, threshold=0x%x, data_len=0x%x, speed=%d KB/s\n", __FUNCTION__, cache_id, 
                    size, threshold, pe_cache_get_data_len(cache_id), pc->read_size/delta);
    pc->read_size = size;
    UINT32 ts = osal_get_tick();
#endif

    if(size > threshold)
    {
        threshold = size;
    }
    tick_begin = osal_get_tick();

#ifdef ENABLE_PE_FLAG
    if(threshold > pe_cache_get_data_len(cache_id) && !pe_cache_check_eof(cache_id))
    {
        UINT32 flgptn = 0;
        BOOL wait_data = TRUE;
        PE_CACHE_MUTEX_LOCK(pc->mutex, OSAL_WAIT_FOREVER_TIME);
        if(PE_FILE_FAILED == pc->sub_status)
        {
            PE_CACHE_MUTEX_UNLOCK(pc->mutex);
            return 0;
        }
        pc->req_size = size;
        PE_CACHE_MUTEX_UNLOCK(pc->mutex);
        osal_flag_wait(&flgptn, pc->flag, PE_FLAG_DATA_READY, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
    }
#else
    while((threshold > pe_cache_get_data_len(cache_id)) && (!pe_cache_check_eof(cache_id)))
    {
        if ((osal_get_tick() - tick_begin) > PE_CACHE_READ_TMO)
        {
            if((PE_FILE_EOF == pc_ex->sub_status) && (0 == pe_cache_get_data_len(cache_id)) )
                return 0x7FFFFFFF;
            //PECACHE_PRINTF("pe_cache_get see, wait data time out\n");
            return 0;
        }
        
        osal_task_sleep(1);
    }
#endif

    if(PE_CACHE_CLOSING == pe_cache_info_ex[cache_id].status)
    {
        PECACHE_PRINTF("pe cache closing\n");
        return 0x7FFFFFFF;
    }

#ifdef LOG_PE_CACHE
    pc->read_delay += osal_get_tick() - ts;
    pc->read_count++;
#endif

    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        if((pc_ex->sub_status != PE_FILE_FAILED) && pc_ex->data_len)
        {
            if(pc_ex->rd_pos < pc_ex->wr_pos)
            {
                bytes_returned = pc_ex->wr_pos - pc_ex->rd_pos;
            }
            else
            {
                bytes_returned = pc_ex->cache_size - pc_ex->rd_pos;
            }

            if(bytes_returned > size)
            {
                bytes_returned = size;
            }
            *ptr = &pc_ex->cache_buff[pc_ex->rd_pos];
        }
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }

#ifdef LOG_PE_CACHE
    delta = osal_get_tick() - ts;
    if(delta)
        PECACHE_PRINTF("get_speed=%d KB/s, read_time=%d\n", bytes_returned/delta, delta);
    else
        PECACHE_PRINTF("get_speed=[MAX]\n");
    pc->read_tick = osal_get_tick();
#endif

    return bytes_returned;
}

void pe_cache_invalidate(int cache_id, UINT8 *ptr, UINT32 size)
{
    volatile pe_cache_ex *pc_ex = NULL;
	
	if(cache_id <0 || cache_id >= PE_CACHE_NUM) return;

    if(!ptr)
    {
        libc_printf("%s: ptr is null \n", __FUNCTION__);
        return ;
    }
    ASSERT((cache_id >= 0) && (cache_id < PE_CACHE_NUM));
    pc_ex = &pe_cache_info_ex[cache_id];
    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        ASSERT(pc_ex->rd_pos == ((UINT32)ptr - (UINT32)pc_ex->cache_buff));
        ASSERT(size <= pc_ex->data_len);
        if(pc_ex->data_len)
        {
            ASSERT(((pc_ex->rd_pos >= pc_ex->wr_pos) && (size <= (pc_ex->cache_size - pc_ex->rd_pos))) || \
                   (size <= (pc_ex->wr_pos - pc_ex->rd_pos)));
            pc_ex->rd_pos += size;
            if(pc_ex->rd_pos >= pc_ex->cache_size)
            {
                pc_ex->rd_pos = 0;
            }
            pc_ex->data_len -= size;
        }
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }
}

static int pe_cache_seek_in_buffer(int cache_id, off_t offset)
{
    int ret = 0;
    __MAYBE_UNUSED__ p_pe_cache pc = NULL;
    pe_cache_ex *pc_ex = NULL;

    if(cache_id < 0 || cache_id >= PE_CACHE_NUM) 
    {
        return -1;
    }
    
    pc = (p_pe_cache)&pe_cache_info[cache_id];
    pc_ex = &pe_cache_info_ex[cache_id];
    
    PECACHE_PRINTF("%s:%d: offset=%lld\r\n",__FUNCTION__,__LINE__, offset);
    if(offset >= 0)
    {
        pc_ex->data_len -= offset;
        pc_ex->rd_pos = RBUFF_PTR_ADD(pc_ex->rd_pos, pc_ex->cache_size, offset);
    }
    else if(offset < 0) 
    {
        pc_ex->data_len += (-1* offset);
        pc_ex->rd_pos = RBUFF_PTR_DEC(pc_ex->rd_pos, pc_ex->cache_size, -1*offset);
    }
    else
        ret = -1;
    PECACHE_PRINTF("%s:ret:%d, offset: %lld, new rd_pos:%d\n", __FUNCTION__, ret, offset, pc_ex->rd_pos);
    return ret;
}

static int pe_cache_real_seek(int cache_id, off_t offset, int where)
{
    int ret = -1;
    p_pe_cache pc = (p_pe_cache)&pe_cache_info[cache_id];
    pe_cache_ex *pc_ex = &pe_cache_info_ex[cache_id];
	PECACHE_PRINTF("PC->Fileseek = %d\n",pc->file_seek);
    if(pc->file_seek)
    {
        ret = pc->file_seek(pc->fp, offset, where);
        pc_ex->data_len = 0;
        pc_ex->rd_pos = 0;
        pc_ex->wr_pos = 0;                        
    }
    if (pc->file_tell != NULL)
    {
        pc_ex->file_offset = pc->file_tell(pc->fp);
    }
    pc_ex->sub_status = PE_FILE_READING;
    PECACHE_PRINTF("%s: offset: %lld, where:%d, new offset:%lld\n", __FUNCTION__, offset, where, pc_ex->file_offset);
    return ret;
}

int pe_cache_seek(int cache_id, off_t offset, int where)
{
    int ret = -1;
    p_pe_cache pc = NULL;
	pe_cache_ex *pc_ex = NULL;
    off_t cache_cur_offset = 0;
    off_t cache_start_offset = 0;
    off_t cache_end_offset = 0;
    UINT8 ucseektype = 0;;

    if(cache_id <0 || cache_id >= PE_CACHE_NUM) return -1;
    ASSERT((UINT32)cache_id < PE_CACHE_NUM);
    pc = (p_pe_cache)&pe_cache_info[cache_id];
    pc_ex = &pe_cache_info_ex[cache_id];

    ucseektype = pe_cache_get_seektype(cache_id);
    
    PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
    if(PE_CACHE_OPENED == pc_ex->status && PE_FILE_FAILED != pc_ex->sub_status)
    {
        cache_end_offset = pc_ex->file_offset;
        if(cache_end_offset < 0 ) cache_end_offset = 0;
        cache_cur_offset = cache_end_offset - (off_t)(pc_ex->data_len);
        if(cache_cur_offset < 0 ) cache_cur_offset = 0;
        if(pc_ex->rd_pos > pc_ex->wr_pos)
        {
            cache_start_offset = cache_cur_offset - (pc_ex->rd_pos - pc_ex->wr_pos);
        }
        else 
        {
            cache_start_offset = cache_cur_offset - pc_ex->rd_pos;
        }
        if(cache_start_offset < 0 ) cache_start_offset = 0;

        #ifdef SUPPORT_ES_PLAYER
        if(g_libstream_enable == 1)
            pe_pre_buffer_size[cache_id] = 10240;
        #endif

        PECACHE_PRINTF("\n%s:%d: newoff:%lld, where:%d [start:%lld, end:%lld, cur:%lld]\n",__FUNCTION__,__LINE__, 
            offset, where, cache_start_offset, cache_end_offset, cache_cur_offset);
        switch(where)
        {
            case SEEK_SET:
                PECACHE_PRINTF("SEEK_SET: start:%lld, cur:%lld, end:%lld, new offset:%lld\n", 
                    cache_start_offset, cache_cur_offset, cache_end_offset, offset);
                if(offset >= cache_cur_offset && offset < cache_end_offset)
                {
                    ret = pe_cache_seek_in_buffer(cache_id, offset-cache_cur_offset);
                }
                else if(offset >= cache_start_offset && offset < cache_cur_offset)
                {
                    ret = pe_cache_seek_in_buffer(cache_id, offset-cache_cur_offset);
                }
                else if(pc->open_success == 0) // connection is running
                {
                    ret = 0;
                }
                else if(ucseektype)
                {
                    ret = pe_cache_real_seek(cache_id, offset, where);
                }
                break;
            case SEEK_CUR:
                PECACHE_PRINTF("SEEK_CUR: start:%lld, cur:%lld, end:%lld, new offset:%lld\n", 
                    cache_start_offset, cache_cur_offset, cache_end_offset, offset);
                if((offset >=0) && (offset <= pc_ex->data_len))
                {
                    ret = pe_cache_seek_in_buffer(cache_id, offset);
                }
                else if((offset<0) && ((-1*offset) <= cache_cur_offset - cache_start_offset))
                {
                    ret = pe_cache_seek_in_buffer(cache_id, offset);
                }
                else if(ucseektype)
                {
                    offset -= (cache_end_offset - cache_cur_offset);
                    ret = pe_cache_real_seek(cache_id, offset, where);
                }
                break;
            case SEEK_END:
            case 3: //time seek
                PECACHE_PRINTF("offset: %lld, where: %d\n", offset, where);
                if(ucseektype)
                    ret = pe_cache_real_seek(cache_id, offset, where);
                break;
            default:
                break;
        }            
    }
    PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
    PECACHE_PRINTF("%s:%d: ret=%d\r\n",__FUNCTION__,__LINE__, ret);
    return ret;
}

int pe_cache_rollback(int cache_id, off_t offset, int where)
{
    int ret = -1;
    pe_cache_ex *pc_ex = NULL;
    if(cache_id <0 || cache_id >= PE_CACHE_NUM) return -1;

    ASSERT((UINT32)cache_id < PE_CACHE_NUM);

    pc_ex = &pe_cache_info_ex[cache_id];

    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        switch(where)
        {
            case SEEK_SET:
                if(pc_ex->rd_pos <= pc_ex->wr_pos)
                {
                    if(offset < pc_ex->rd_pos)
                    {
                        pc_ex->data_len += (pc_ex->rd_pos - offset);
                    }
                    else
                    {
                        pc_ex->data_len -= (offset - pc_ex->rd_pos);
                    }
                    pc_ex->rd_pos = offset;
                    ret = 0;
                }
                else
                {
                    ret = -1;
                }
                break;
            default:
                ret = -1;
                break;
        }
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }

    return ret;
}

int pe_cache_check_eof(int cache_id)
{
    int result = 1;
    __MAYBE_UNUSED__ p_pe_cache pc = (p_pe_cache)&pe_cache_info[cache_id];
    pe_cache_ex *pc_ex = &pe_cache_info_ex[cache_id];

    if((cache_id <0) || (cache_id >= PE_CACHE_NUM))
    {
        return 1;
    }
    pc_ex = &pe_cache_info_ex[cache_id];
    if(PE_FILE_FAILED == pc_ex->sub_status)
    {
        return 1;
    }

    ASSERT((UINT32)cache_id < PE_CACHE_NUM);
    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        if(PE_FILE_READING == pc_ex->sub_status)
        {
            result = 0;
        }
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }
    return result;
}

UINT32 pe_cache_get_data_len(int cache_id)
{
    volatile pe_cache_ex *pc_ex = NULL;
    UINT32 data_len = 0;

	if(cache_id <0 || cache_id >= PE_CACHE_NUM) return 0;
    pc_ex = &pe_cache_info_ex[cache_id];
	
    ASSERT((UINT32)cache_id < PE_CACHE_NUM);
    if(PE_CACHE_OPENED == pc_ex->status)
    {
        PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
        data_len = pc_ex->data_len;
        PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
    }
    return data_len;
}

off_t pe_cache_tell(int cache_id)
{
    p_pe_cache pc = NULL;
    volatile pe_cache_ex *pc_ex = NULL;
    off_t offset = 0;

	if(cache_id <0 || cache_id >= PE_CACHE_NUM) return -1;
    pc = (p_pe_cache)&pe_cache_info[cache_id];
    pc_ex = &pe_cache_info_ex[cache_id];
    if(pc->file_tell ==NULL)
    {
      return offset;//don't return tell when play net work stream
    }
    else
    {
        ASSERT((UINT32)cache_id < PE_CACHE_NUM);
        if(PE_CACHE_OPENED == pc_ex->status)
        {
            PE_CACHE_MUTEX_LOCK(pc_ex->mutex, OSAL_WAIT_FOREVER_TIME);
            offset = pc_ex->file_offset - (off_t)(pc_ex->data_len);
            PE_CACHE_MUTEX_UNLOCK(pc_ex->mutex);
        }
        return offset;
    }
}

void pe_cache_set_closing(int cache_id)
{
    if(pe_cache_info_ex && (cache_id >= 0 && cache_id < PE_CACHE_NUM))
    {
        PE_CACHE_MUTEX_LOCK(pe_cache_mutex, OSAL_WAIT_FOREVER_TIME);
        if(pe_cache_info_ex[cache_id].status == PE_CACHE_OPENED)
        {
            pe_cache_info_ex[cache_id].status = PE_CACHE_CLOSING;
        }
        PE_CACHE_MUTEX_UNLOCK(pe_cache_mutex);
    }
}

off_t pe_cache_filesize(int cache_id)
{
    off_t size = 0;
    if(cache_id >= 0 && cache_id < PE_CACHE_NUM)
    {
        size = pe_cache_info[cache_id].file_size;
    }
    PECACHE_PRINTF("pe_cache_filesize:%lld\n", size);
    return size;
}

// Get File's durations in second, return -1 if no duration information.
INT32 pe_cache_file_duration(int cache_id)
{
    INT32 duration = 0;
    if(cache_id >= 0 && cache_id < PE_CACHE_NUM)
    {
        duration = pe_cache_info[cache_id].file_duration;
    }
    return duration;
}

void pe_cache_full_use(int cache_id)
{
    if(cache_id >= 0 && cache_id < PE_CACHE_NUM)
    {
        pe_pre_buffer_size[cache_id] = 1;
    }
    PECACHE_PRINTF("pe_cache_full_use ...\n");
}

int pe_cache_get_seektype(int cache_id)
{
    int seektype = 0;
    if(cache_id >= 0 && cache_id < PE_CACHE_NUM)
    {
        seektype = pe_cache_info[cache_id].seek_type;
    }
    return (m_seek_type == 0xFF ? seektype : m_seek_type);
}

off_t pe_cache_file_offset(int cache_id)
{
    if(pe_cache_info_ex && (cache_id >= 0 && cache_id < PE_CACHE_NUM)
        && (pe_cache_info_ex[cache_id].status == PE_CACHE_OPENED))
    {
        return pe_cache_info_ex[cache_id].file_offset;
    }
    return 0;
}

void pecache_reg_filehdl(pe_cache_cb_fp *hdl)
{
    g_reg_pecache_cb_hdl = hdl;
}

int pecache_enable_sgdma()
{
    pe_cache_use_sgdma = 1;
#ifdef NETWORK_SUPPORT
    uf_enable_sgdma();
#endif
	return 0;
}
