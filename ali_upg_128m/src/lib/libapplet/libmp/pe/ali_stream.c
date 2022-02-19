  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ali_stream.c
*
*    Description: This file describes ali stream operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <api/libstream/libstream.h>
#include <api/libmp/pe.h>
#include <hld/snd/snd.h>

//#include "video_advanced/video_engine.h"
#include "play_engine_common.h"
#include "video_advanced/mp_player.h"

#if 1
#define PRF libc_printf
#else
#define PRF(...)    do{}while(0)
#endif

#define FLAG_SIZE        18

static int _last_tick = -1;
static int _time_cnt = 0;
static int _time_out = 10000;
static char ali_media_path[] = "/es_stream.ali";              /* */
static LS_HANDLE m_ls_hdl = 0xFFFFFFFF;//-1;                                   /* */
static struct ls_config m_ls_cfg = {0, 0, NULL};                                      /* */
static struct ls_prog_header m_prog_header1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};               /* */
static struct ls_prog_vide_header m_prog_vide_header = {0, 0, 0, 0, 0, 0, 0, 0, 0}; /* */
static struct ls_prog_header m_prog_header2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};               /* */
static struct ls_prog_audi_header m_prog_audi_header = {0, 0, 0, 0}; /* */
static int m_play_mode = 0;
static int m_avm_playing = 0; // 0 : stop 1: playing 2 : pause
static int m_avm_advertisement = 0;
static int m_avm_video_init = 0;
static int m_avm_sample_index[2] = {0, 0};
static int m_avm_backup = 0;
static int m_avm_last_playtime = 0;
static int m_avm_empty_cnt = 0;
static int m_sample_index[2] = {0, 0};

FILE *ali_stream_open(const char *path, unsigned char *buffer, unsigned int buf_len, unsigned int offset)
{
    FILE *fp = NULL;

    m_ls_hdl = libstream_open(&m_ls_cfg);
    if(m_ls_hdl == 0)
    {
        PRF("%s : fail\n", __FUNCTION__);
        return NULL;
    }

    libstream_set_prog_info(m_ls_hdl, (void *)&m_prog_header1);
    if(m_play_mode == 0)
    {
        libstream_set_prog_info(m_ls_hdl, (void *)&m_prog_header2);
    }
    libstream_control(m_ls_hdl, LS_CONTROL_START, 0);

    fp = (FILE *)m_ls_hdl;
    return fp;
}

size_t ali_stream_close(FILE *fp)
{
    if(m_ls_hdl != (LS_HANDLE)-1)
    {
        libstream_control(m_ls_hdl, LS_CONTROL_STOP, 0);
        libstream_close(m_ls_hdl);

        m_ls_hdl = -1;
    }

    return 0;
}

int ali_stream_end(FILE *stream)
{
    if(!stream)
    {
        libc_printf("%s : stream is null\n", __FUNCTION__);
        return -1;
    }
    int ret = libstream_control((LS_HANDLE)stream, LS_CONTROL_CHECK_END, 0);

    if(1 == ret)
    {
        libc_printf("%s : end flag %d\n", __FUNCTION__, ret);
    }
    return (ret == 1 ? 1 : 0);
}

/*
int ali_stream_play_mode(int mode)
{
    m_play_mode = mode;

    return 1;
}
*/

static void stop_ali_stream(void)
{
    libstream_control(m_ls_hdl, LS_CONTROL_SET_END, 0);
}

size_t ali_stream_read(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    LS_HANDLE ls_hdl = (LS_HANDLE)stream;
    int left_size = 0;
    int read_size = 0;
    int ret = 0;

    if((!ptr) || (!stream))
    {
        libc_printf("%s : ptr or stream is null\n", __FUNCTION__);
        return 0;
    }
    //libc_printf("%s : size %d\n", __FUNCTION__, size);
    left_size = size * nmemb;
    while(left_size > 0)
    {
        ret = libstream_fread(ls_hdl, ptr, left_size);
        if(ret < 0)
        {
            if(1 == ali_stream_end(stream))
            {
                return 0;
            }

            //libc_printf("%s : wait new data comming\n", __FUNCTION__);
            osal_task_sleep(1);
#if 0
            time_out--;
            if(time_out < 0)
            {
                libc_printf("%s : no data comming, timeout happen\n", __FUNCTION__);
                goto EXIT;
            }
#endif          
            continue;
        }
        read_size += ret;
        left_size -= ret;
        ptr += ret;

    }
//EXIT:
    return read_size;
}

int ali_stream_seek(FILE *stream, off_t offset, int whence)
{
    LS_HANDLE ls_hdl = (LS_HANDLE)stream;
    long long ret = 0;

    if(!stream)
    {
        libc_printf("%s : stream is null\n", __FUNCTION__);
        return -1;
    }
    if(1 == ali_stream_end(stream))
    {
        return -1;
    }

    ret = libstream_fseek(ls_hdl, whence, offset);

    return (int)ret;
}

off_t ali_stream_tell(FILE *stream)
{
    if(!stream)
    {
        libc_printf("%s : stream is null\n", __FUNCTION__);
        return -1;
    }

    if(1 == ali_stream_end(stream))
    {
        return -1;
    }

    return libstream_ftell((LS_HANDLE)stream);
}

static int strcmp_c(const char *s1, const char *s2)
{
    int i = 0;
    char c1 = 0;
    char c2 = 0;

    if(!s1)
    {
        libc_printf("%s : s1 is null\n", __FUNCTION__);
        return -1;
    }
    if(!s2)
    {
        libc_printf("%s : s2 is null\n", __FUNCTION__);
        return -1;
    }
    for (i = 0, c1 = *s1, c2 = *s2; (c1 != '\0') && (c2 != '\0'); i++)
    {
        c1 = *(s1 + i);
        c2 = *(s2 + i);
        if ((c1 >= 'A') && (c1 <='Z'))
        {
            c1 += 'a' - 'A';
        }
        if ((c2 >= 'A') && (c2 <='Z'))
        {
            c2 += 'a' - 'A';
        }
        if (c1 != c2)
        {
            break;
        }
    }

    return (int)c1 - (int)c2;
}

int is_ali_stream(char *file)
{
    int size = 0;
    int i = 0;

    if(!file)
    {
        libc_printf("%s : file is null\n", __FUNCTION__);
    }
    size = STRLEN(file);
    for(i = 0;i < size;i++)
    {
        if(file[i] == '.')
        {
            break;
        }
    }

    if(i >= size)
    {
        return -1;
    }

    i++;
    if(strcmp_c(file + i, "ALI"))
    {
        //libc_printf("%s : it is not ali file\n", __FUNCTION__);
        return -1;
    }

    return 1;
}

int is_es_stream(char *file)
{
	UINT32 size = STRLEN(file);	
    UINT32 i = 0;

    if(!file)
    {
        libc_printf("%s : file is null\n", __FUNCTION__);
    }
    size = STRLEN(file);
    for(i = 0;i < size;i++)
    {
        if(file[i] == '.')
        {
            break;
        }
    }

    if(i >= size)
    {
        return -1;
    }

    i++;
    if(strcmp_c(file + i, "ES"))
    {
        //libc_printf("%s : it is not ali file\n", __FUNCTION__);
        return -1;
    }

    return 1;
}

static void start_player(int preview)
{
    if(m_avm_playing == 0)
    {
        video_decode(ali_media_path, UNKNOWN_STREAM, SND_DUP_NONE,preview);

        while(m_ls_hdl == (LS_HANDLE)-1)
        {
            libc_printf("%s : wait until ls is opened\n", __FUNCTION__);
            osal_task_sleep(1);
        }
        osal_task_sleep(10);
        m_avm_playing = 1;
        m_avm_sample_index[0] = 0;
        m_avm_sample_index[1] = 0;
        m_avm_advertisement = 0;
        m_avm_empty_cnt = 0;
        m_avm_backup = 0;
        m_avm_last_playtime = -1;
        m_avm_video_init = 0;
        libc_printf("%s : line %d\n", __FUNCTION__, __LINE__);
    }
}

static void stop_player(void)
{
    struct vdec_status_info status;
    RET_CODE ret = RET_FAILURE;
    int cnt = 0;

    if((m_avm_playing == 1) || (m_avm_playing == 2))
    {
        stop_ali_stream();

        while(!pe_cache_check_eof(0))
        {
            libc_printf("%s : line %d\n", __FUNCTION__, __LINE__);
            osal_task_sleep(10);
        }

        while(1)//cnt < 10)
        {
            MEMSET((void *)&status, 0, sizeof(status));
            ret = video_decoder_ioctl(VDEC_IO_GET_STATUS, (unsigned long)&status, 0);
            libc_printf("%s : get status %d ret %d w %d h %d\n"
                , __FUNCTION__, status.u_cur_status, ret, status.pic_width, status.pic_height);
            if(ret == RET_SUCCESS)
            {
                if((status.u_cur_status == VDEC_DECODING) || (status.u_cur_status == VDEC_CLOSED))
                {
                    libc_printf("wait the decoder init finished\n");
                    break;
                }
            }
            osal_task_sleep(1);
            cnt++;
        }

        libc_printf("%s : line %d\n", __FUNCTION__, __LINE__);

        mpg_cmd_stop_proc(0);
        while(m_ls_hdl != (LS_HANDLE)-1)
        {
            libc_printf("%s : wait player stop\n", __FUNCTION__);
            osal_task_sleep(1);
        }
        osal_task_sleep(10);
        m_avm_playing = 0;
    }
}

static void send_frame(int type, void *buffer, int size, UINT32 pts)
{
    struct ls_data_header data_header;
    int ret = 0;

    if(!buffer)
    {
        libc_printf("%s: buffer is null:\n", __FUNCTION__);
        return;
    }
    MEMSET((void *)&data_header, 0, sizeof(data_header));
    data_header.prog_index = type == 0 ? 0 : 1;
    data_header.sample_index = ++m_sample_index[data_header.prog_index];
    data_header.timestamp.flag = ls_presentation_ts;
    data_header.timestamp.value = pts;

    do
    {
        ret = libstream_push_data(m_ls_hdl, buffer, size, (void *)&data_header);
        if(ret < 0)
        {
            //libc_printf("%s : libstream is full\n", __FUNCTION__);
            osal_task_sleep(10);
        }
    }while(ret < 0);
}

// fopen
static FILE *pe_fopen(const char *path, const char *opt)
{
    if((!path) || (!opt))
    {
        libc_printf("%s: path is null:\n", __FUNCTION__);
        return NULL;
    }
    return fopen(path, opt);
}

// fclose
static INT32 pe_fclose(FILE *file)
{
    INT32 ret = -1;

    if (file != NULL)
    {
        ret = fclose(file);
    }
    return ret;
}

static ssize_t pe_fread(UINT8 *buffer, INT32 size, FILE *file)
{
    INT32 ret = 0;

    //libc_printf("file 0x%x, offset %d Q, read %d K\n", file, ((_ftell(file)/1024)-64)/47, size/1024);
    if ((file != NULL) && (buffer != NULL))
    {
        ret = fread(buffer, 1, size, file);
        if (ret <= 0)
        {
            libc_printf("_fread err ret=%d,file = %xh\n", ret, file);
        }
    }
    return ret;
}

static off_t pe_fseek(FILE *file, off_t offset, int whence)
{
    if(file)
    {
        return fseek(file, offset, whence);
    }

    return -1;
}

static void send_stream(int par1, int par2)
{
    if(par1 == 0)
        return;
    char *file = (char *)par1;
    /* simulate the network data push operation */
    unsigned char flag_start[9] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    unsigned char flag[FLAG_SIZE];
    int read_size = 0;
    int ret_size = 0;
    UINT32 pts = 0;
    FILE *in_file = NULL;
    void *buffer = NULL;
    void *buf_video = NULL;
    int buf_size = 0x10000;
    int m = 0;
    int n = 0;
    int run = 0;
    char video_config_data[32];
    int video_config_init = 0;
    int tick = 0;
    unsigned char *buf = NULL;
    void *buf2 = NULL;
	//int au_num = 0;
	UINT32 au_num = 0;
    int size = 0;
	UINT32 i = 0;//int i = 0;

    MEMSET(video_config_data, 0, sizeof(video_config_data));
    MEMSET(flag, 0, sizeof(flag));
    buffer = MALLOC(buf_size);
    if(buffer == NULL)
    {
        libc_printf("malloc operation buffer fail\n");
        return;
    }
    buf_video = MALLOC(buf_size);
    if(buf_video == NULL)
    {
        libc_printf("malloc operation buffer fail\n");
        FREE(buffer);
        buffer = NULL;
        return;
    }
RESTART:
    libc_printf("reset the file handle\n");
    if(in_file != NULL)
    {
        pe_fclose(in_file);
    }
    in_file = (FILE *)pe_fopen(file, "rb");
    if(in_file == NULL)
    {
        libc_printf("open in file fail %x\n", file);
        FREE(buffer);
        buffer = NULL;
        FREE(buf_video);
        buf_video = NULL;
        return;
    }
    pe_fseek(in_file, 0, SEEK_SET);
    while(1)
    {
        tick = osal_get_tick();
        if(-1 == _last_tick)
        {
            _last_tick = tick;
        }
        else
        {
            _time_cnt += tick - _last_tick;
            _last_tick = tick;
            if(_time_cnt >= _time_out)
            {
                _time_cnt = 0;
                _time_out = ((read_tsc()&0x7) + 1) * 1000;
                libc_printf("time out %d\n", _time_out);
                stop_player();
                start_player(0);
            }
        }
        MEMSET(flag, 0, FLAG_SIZE);
        ret_size = pe_fread(flag, FLAG_SIZE, in_file);
        if(ret_size < FLAG_SIZE)
        {
            libc_printf("%s : input file error1 %d\n", __FUNCTION__, __LINE__);
            if(1 == run)
            {
                libc_printf("%s : resend the es file\n", __FUNCTION__);
                goto RESTART;
            }
            break;
        }
        run = 1;
        m = 9;
        n = 0;
        for(n = 0;n < m;n++)
        {
            if(flag[n] != flag_start[n])
            {
                libc_printf("%s : input file error2 %d\n", __FUNCTION__, __LINE__);
                goto RESTART;
            }
        }
        read_size = flag[10] | (flag[11]<<8) | (flag[12]<<16) | (flag[13]<<24);
        ret_size = pe_fread(buffer, read_size, in_file);
        //libc_printf("packet len %x\n", ret_size);
        if(ret_size < read_size)
        {
            libc_printf("%s : input file error3 %d\n", __FUNCTION__, __LINE__);
            goto RESTART;
        }
        pts = flag[14] | (flag[15]<<8) | (flag[16]<<16) | (flag[17]<<24);
        if(0x99 == flag[9])
        {
            if((*(unsigned char *)buffer != 0x00)
                || (*((unsigned char *)buffer + 1) != 0x00)
                || (*((unsigned char *)buffer + 2) != 0x01))
            {
                libc_printf("drop the error video data\n");
                continue;
            }
            if(video_config_init == 0)
            {
                video_config_init = 1;
                MEMCPY(video_config_data, buffer, 29);
            }
            if(m_avm_video_init  == 0)
            {
                m_avm_video_init = 1;
                if((0x00 == *(unsigned char *)buffer)
                      && (*((unsigned char *)buffer + 1) == 0x00)
                      && (*((unsigned char *)buffer + 2) == 0x01)
                      && (*((unsigned char *)buffer + 3) != 0xB0))
                {
                    MEMCPY(buf_video, video_config_data, 29);
                    MEMCPY(buf_video + 29, buffer, ret_size);
                    ret_size += 29;
                    send_frame(0, buf_video, ret_size, pts);
                }
                else
                {
                    send_frame(0, buffer, ret_size, pts);
                }
            }
            else
            {
                send_frame(0, buffer, ret_size, pts);
            }
        }
        else
        {
            buf = (unsigned char *)buffer;
            buf2 = (unsigned char *)buffer;
            au_num = 0;
            size = 0;
            i = 0;
            au_num = ((*buf<<8) | *(buf + 1))>>4;
            buf += 2;
            buf2 += (au_num + 1)<<1;
            for(i = 0;i < au_num;i++)
            {
                if (((UINT32)buf+ 1) > ((UINT32)buffer + buf_size-1)) // assure buf not overflow
                {
                    break;
                }
                size = ((*buf<<8) | *(buf + 1))>>3;
                send_frame(1, buf2, size, pts);
                pts += 1024;
                buf2 += size;
                buf += 2;
            }
        }
    }

    FREE(buffer);
    buffer = NULL;
    FREE(buf_video);
    buf_video = NULL;
    pe_fclose(in_file);
    return;
}

int network_stream_test(char *file, int preview)
{
    OSAL_T_CTSK t_ctsk_sw;
    OSAL_ID tsk_id = OSAL_INVALID_ID;

    if((!file) || (is_es_stream(file) < 0))
    {
        return -1;
    }
    MEMSET(&t_ctsk_sw, 0, sizeof(OSAL_T_CTSK));
    //api_full_screen_play();
    *(UINT32 *)0xB8006300 &= ~1;

    m_sample_index[0] = 0;
    m_sample_index[1] = 0;

    /* libstream open paramters */
    MEMSET((void *)&m_ls_cfg, 0, sizeof(m_ls_cfg));
    m_ls_cfg.cache_size = 0x200000;
    m_ls_cfg.reserved_size = 0x130000;

    /* set video and audio stream info */
    MEMSET((void *)&m_prog_header1, 0, sizeof(m_prog_header1));
    MEMSET((void *)&m_prog_vide_header, 0, sizeof(m_prog_vide_header));
    m_prog_header1.prog_index = 0;
    m_prog_header1.type = LS_DATA_VIDE;
    m_prog_header1.codec_tag = 0x7634706D;
    m_prog_header1.timescale = 90000;
    m_prog_header1.resolution = 24;
    m_prog_header1.private = &m_prog_vide_header;
    m_prog_vide_header.width = 720;
    m_prog_vide_header.height = 576;
    m_prog_vide_header.frame_rate = 30000;

    MEMSET((void *)&m_prog_header2, 0, sizeof(m_prog_header2));
    MEMSET((void *)&m_prog_audi_header, 0, sizeof(m_prog_audi_header));
    m_prog_header2.prog_index = 1;
    m_prog_header2.type = LS_DATA_AUDI;
    m_prog_header2.codec_tag = 0x6134706D;
    m_prog_header2.timescale = 44100;
    m_prog_header2.resolution = 16;
    m_prog_header2.private = &m_prog_audi_header;
    m_prog_audi_header.sample_rate = 44100;
    m_prog_audi_header.sample_size = 1024;
    m_prog_audi_header.bit_rate = 95919;
    m_prog_audi_header.channel_count = 2;

#if 1
    start_player(0);
#else
    /* start the player operation */
    video_decode(ali_media_path, UNKNOWN_STREAM, SND_DUP_NONE, preview);

    while(m_ls_hdl == -1)
    {
        libc_printf("%s : wait until ls is opened\n", __FUNCTION__);
        osal_task_sleep(1);
    }
#endif

    {
        t_ctsk_sw.name[0] = 'S';
        t_ctsk_sw.name[1] = 'T';
        t_ctsk_sw.name[2] = 'R';
        t_ctsk_sw.stksz = 0x4000;
        t_ctsk_sw.quantum = 5;
        t_ctsk_sw.para1 = (int)file;
        t_ctsk_sw.para2 = (int)m_ls_hdl;
        t_ctsk_sw.task = (FP)send_stream;
        t_ctsk_sw.itskpri = OSAL_PRI_NORMAL;
		tsk_id = osal_task_create(&t_ctsk_sw);

        osal_task_sleep(100);
        if(tsk_id == OSAL_INVALID_ID)
            return -1;
    }

    return 1;
}

/*
int pack_ali_stream_by_es_file(char *file)
{
    FILE *out_file = NULL;
    FILE *in_file = NULL;
    int size = STRLEN(file);
    char *new_file = NULL;
    int i = 0;

    for(i = 0;i < size;i++)
    {
        if(file[i] == '.')
            break;
    }

    if(i >= size)
        return -1;

    i++;
    if(strcmp_c(file + i, "ES"))
    {
        libc_printf("%s : it is not es file\n", __FUNCTION__);
        return -1;
    }

    new_file = MALLOC(size + 1);
    MEMCPY(new_file, file, size);
    *(new_file + i) = 'a';
    *(new_file + i + 1) = 'l';
    *(new_file + i + 2) = 'i';
    *(new_file + i + 3) = '\0';
    out_file = (FILE *)_fopen(new_file, "wb");
    if(out_file == NULL)
    {
        libc_printf("open new file fail %s \n", new_file);
        return 1;
    }
    libc_printf("open out file done %s\n", new_file);

    in_file = (FILE *)_fopen(file, "rb");
    if(in_file == NULL)
    {
        libc_printf("open in file fail %x\n", file);
        return 1;
    }

    LS_HANDLE ls_hdl;
    struct ls_config ls_cfg;

    MEMSET((void *)&ls_cfg, 0, sizeof(ls_cfg));
    ls_cfg.cache_size = 0x100000;
    ls_cfg.reserved_size = 0x50000;
    ls_hdl = libstream_open(&ls_cfg);

    {
        struct ls_prog_header prog_header;
        struct ls_prog_audi_header prog_audi_header;
        struct ls_prog_vide_header prog_vide_header;

        MEMSET((void *)&prog_header, 0, sizeof(prog_header));
        MEMSET((void *)&prog_vide_header, 0, sizeof(prog_vide_header));
        prog_header.prog_index = 0;
        prog_header.type = LS_DATA_VIDE;
        prog_header.codec_tag = 0x7634706D;
        prog_header.timescale = 90000;
        prog_header.resolution = 24;
        prog_header.private = &prog_vide_header;
        prog_vide_header.width = 720;
        prog_vide_header.height = 576;
        prog_vide_header.frame_rate = 30000;
        libstream_set_prog_info(ls_hdl, (void *)&prog_header);

        MEMSET((void *)&prog_header, 0, sizeof(prog_header));
        MEMSET((void *)&prog_audi_header, 0, sizeof(prog_audi_header));
        prog_header.prog_index = 1;
        prog_header.type = LS_DATA_AUDI;
        prog_header.codec_tag = 0x6134706D;
        prog_header.timescale = 48000;
        prog_header.resolution = 16;
        prog_header.private = &prog_audi_header;
        prog_audi_header.sample_rate = 48000;
        prog_audi_header.sample_size = 1024;
        prog_audi_header.bit_rate = 95919;
        prog_audi_header.channel_count = 0;
        libstream_set_prog_info(ls_hdl, (void *)&prog_header);
    }

#define FLAG_SIZE        14
    struct ls_data_header data_header;
    unsigned char flag_start[9] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    unsigned char flag[FLAG_SIZE];
    int read_size, ret_size, sample_idx[2];
    int m, n, total_size;

    void *buffer = NULL;
    int buf_size = 0x10000;
    int push_size;
    void *out_buf = NULL;

    out_buf = MALLOC(buf_size);
    if(out_buf == NULL)
    {
        libc_printf("%s : malloc out buf fail\n", __FUNCTION__);
        return 1;
    }

    buffer = MALLOC(buf_size);
    if(buffer == NULL)
    {
        libc_printf("malloc operation buffer fail\n");
        return 1;
    }

    libstream_control(ls_hdl, LS_CONTROL_START, 0);

    fseek(in_file, 0, SEEK_SET);
    sample_idx[0] = sample_idx[1] = 0;
    total_size = 0;
    while(1)
    {
        ret_size = _fread(flag, FLAG_SIZE, in_file);
        if(ret_size < FLAG_SIZE)
        {
            libc_printf("%s : input file error %d\n", __FUNCTION__, __LINE__);
            break;
        }
        m = 0;
        n = 0;
        for(n = 0;n < m;n++)
        {
            if(flag[n] != flag_start[n])
            {
                libc_printf("%s : input file error %d\n", __FUNCTION__, __LINE__);
                return 1;
            }
        }
        read_size = flag[10] | (flag[11]<<8) | (flag[12]<<16) | (flag[13]<<24);
        ret_size = _fread(buffer, read_size, in_file);
        if(ret_size < read_size)
        {
            libc_printf("%s : input file error %d\n", __FUNCTION__, __LINE__);
            return 1;
        }
        MEMSET((void *)&data_header, 0, sizeof(data_header));
        data_header.prog_index = flag[9] == 0x99 ? 0 : 1;
        data_header.sample_index = ++sample_idx[data_header.prog_index];
        data_header.timestamp.flag = LS_NoneTs;
CON:
        push_size = libstream_push_data(ls_hdl, buffer, ret_size, (void *)&data_header);
        if(push_size < 0)
        {
            m = 0;
            while(1)
            {
                ret_size = libstream_fread(ls_hdl, out_buf, buf_size);
                if(ret_size < 0)
                    break;

                m++;
                if(_fwrite(out_buf, ret_size, out_file) < ret_size)
                {
                    libc_printf("%s : write data fail %d\n", __FUNCTION__, __LINE__);
                    return 1;
                }

                total_size += ret_size;

                if(ret_size < buf_size)
                    break;
            }

            if(m > 0)
                goto CON;

            break;
        }
    }

    libc_printf("%s : total1 write size %d\n", __FUNCTION__, total_size);

    while(1)
    {
        ret_size = libstream_fread(ls_hdl, out_buf, buf_size);
        if(ret_size < 0)
            break;

        if(_fwrite(out_buf, ret_size, out_file) < ret_size)
        {
            libc_printf("%s : write data fail %d\n", __FUNCTION__, __LINE__);
            return 1;
        }

        total_size += ret_size;

        if(ret_size < buf_size)
            break;
    }

    libc_printf("%s : total2 write size %d\n", __FUNCTION__, total_size);

    libstream_control(ls_hdl, LS_CONTROL_STOP, 0);

    libstream_close(ls_hdl);

    _fclose(in_file);
    _fclose(out_file);

    _fsync(new_file);
    _fsync("/mnt/uda1");
    FREE(new_file);
    FREE(out_buf);
    FREE(buffer);


    return 1;
}
*/
