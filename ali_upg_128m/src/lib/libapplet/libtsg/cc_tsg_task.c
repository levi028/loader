/*
 * Copyright (C) ALi Corp. 2008,
 *
 * File name    : cc_tsg_task.c
 * Description    : Channel Change TSG task impelementation file.
 *
 * History
 *
 *   Date    Author        Comment
 *   ========    ===========    ================================================
 *
 */
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libnim/lib_nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <bus/tsi/tsi.h>
#include <bus/tsg/tsg.h>
#include <api/libpvr/lib_pvr.h>
#include <api/libtsg/lib_tsg.h>

//#include "pvr_eng_inc.h"
#include "cc_tsg_task.h"
//#ifdef CC_USE_TSG_PLAYER

enum CC_REC_STATE
{
    CC_REC_FREE = 0,
    CC_REC_PLAY_ALONE,
    CC_REC_PLAY_AND_REC,
    CC_REC_REC_ALONE,
};
#define HD_MPEG2_BITRATE        (1024*1024*6)    //1Mbytes/s

#define CC_TSG_PRINTF(...)
//#define CC_TSG_PRINTF           libc_printf

#define CC_TSG_RUN_IN_DMX_TASK

#define ENTER_CC_TSG_API()    osal_semaphore_capture(m_cc_tsg_sema_id, TMO_FEVR)
#define LEAVE_CC_TSG_API()    osal_semaphore_release(m_cc_tsg_sema_id)


struct dmx_device *m_tsg_dmx_dev;
static struct dmx_device *m_tsg_dmx_dev2;
static struct deca_device *m_tsg_deca_dev;

static OSAL_ID m_cc_tsg_sema_id = OSAL_INVALID_ID;
static OSAL_ID m_cc_tsg_task_id = OSAL_INVALID_ID;
static OSAL_ID g_cc_tsg_flagid = OSAL_INVALID_ID;

static UINT8 m_cc_tsg_task_running = 0;

static UINT8 m_tsg_task_exit_safely;
static UINT32 m_tsg_dmx1_bitrate;

#ifdef CC_TSG_DETECT_BITRATE
static UINT8 m_tsg_speed_set_flag;
static UINT32 m_tsg_dmx1_packet_num;
static UINT32 m_tsg_dmx1_start_time;
static UINT32 m_tsg_dmx1_start_num;
static UINT32 m_tsg_dmx1_start_time2;
#endif

static UINT32 m_cc_tsg_rec_handle;
static UINT32 m_cc_tsg_rec_hdl;
UINT32 m_cc_tsg_prog_id;
static UINT16 m_cc_tsg_video_pid;
static UINT16 m_cc_tsg_audio_pid;
static UINT8 m_cc_tsg_av_flag;

#define MAX_TSG_BUF_NUM         8 //(16 * 8)
#define C_TSG_SEND_PAD_NUM      8
#define C_TSG_SEND_PKT_NUM      32

#define CC_TSG_STREAM_BUF_LEN     ((CC_DMX_PKT_NUM + CC_PAD_PKT_NUM) * C_TS_PKT_SIZE * MAX_TSG_BUF_NUM)
#define CC_TSG_REC_BUF_LEN        (CC_DMX_PKT_NUM * C_TS_PKT_SIZE * MAX_TSG_BUF_NUM)
#define CC_TSG_SEND_BUF_LEN        ((C_TSG_SEND_PAD_NUM + C_TSG_SEND_PKT_NUM) * C_TS_PKT_SIZE)

//ATTR_ALIGN_8
//static UINT8 m_cc_ts_stream_buf[CC_TSG_STREAM_BUF_LEN];
static UINT8 *m_cc_ts_stream_buf = NULL;

#ifdef CC_TSG_USE_REC_BUF
//ATTR_ALIGN_8
//static UINT8 m_tsg_rec_stream_buf[CC_DMX_PKT_NUM * C_TS_PKT_SIZE * MAX_TSG_BUF_NUM];
static UINT8 *m_tsg_rec_stream_buf = NULL;
#endif

#ifdef CC_TSG_SPLIT_BUF
//ATTR_ALIGN_8
//static UINT8 m_cc_tsg_send_buf[CC_TSG_SEND_BUF_LEN];
static UINT8 *m_cc_tsg_send_buf = NULL;
#endif

typedef struct _cc_tsg_buf_t
{
    UINT8 *tsg_buf;
    UINT8 *dmx_buf;
    UINT16 pkt_num;
    UINT32 handle;
    UINT32 offset;
} cc_tsg_buf_t, *pcc_tsg_buf_t;

static cc_tsg_buf_t m_tsg_buf_list[MAX_TSG_BUF_NUM];
static UINT8 m_tsg_buf_pos_write, m_tsg_buf_pos_read;
static volatile UINT8 m_tsg_record_buf_pos;

RET_CODE cc_tsg_check_buf_busy(UINT32 buf_addr,UINT32 buf_len)
{
        UINT32 tmo_cnt =0;
        UINT32 tsg_buf_busy;
        while(tmo_cnt<100)
        {
            tsg_buf_busy = tsg_check_buf_busy(buf_addr, buf_len);
            if(tsg_buf_busy != RET_SUCCESS)
                return !RET_SUCCESS;
            osal_task_sleep(1);
            tmo_cnt++;
 //           libc_printf("tsg_buf full\n");
        }
//        libc_printf("tsg err\n");
        return RET_SUCCESS;
}

#ifdef CC_TSG_SPLIT_BUF
BOOL __cc_tsg_send_one_unit(const UINT8 *tsg_buf, const UINT8 *pkt_buf, UINT32 tsg_num, UINT32 pkt_num)
{
    UINT32 buffer_empty = 0;
    UINT32 tsg_remain_buf;
    UINT32 __MAYBE_UNUSED__ tsg_id ;

    dmx_io_control(m_tsg_dmx_dev, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
    tsg_remain_buf = tsg_check_remain_buf();
    while(buffer_empty < tsg_num+tsg_remain_buf)
    {
        if (m_cc_tsg_task_running == 0)
            return FALSE;

        if (m_cc_tsg_rec_handle == 0)
        {
            CC_TSG_PRINTF("tsg send exit because record stopped!\n");
            return FALSE;
        }

        osal_task_sleep(10);
        dmx_io_control(m_tsg_dmx_dev, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
        tsg_remain_buf = tsg_check_remain_buf();
    }

    UINT32 param[2];
    param[0] = (UINT32)(pkt_buf);
    param[1] = pkt_num;
    dmx_io_control(m_tsg_dmx_dev, TSG_PLAYBACK_SYNC, (UINT32)&param[0]);
    //CC_TSG_PRINTF(".");
    osal_cache_flush((void *)tsg_buf, tsg_num*C_TS_PKT_SIZE);
    tsg_id = tsg_transfer((void*)(tsg_buf), tsg_num, FALSE);

    //osal_task_sleep(1); // 10
    //tsg_wait(tsg_id);

    return TRUE;
}

BOOL cc_tsg_send_data(const UINT8 *buf, UINT32 pkt_num)
{
    UINT32 unit_size, unit_num;
    UINT8 *pkt_addr = m_cc_tsg_send_buf + C_TSG_SEND_PAD_NUM * C_TS_PKT_SIZE;

    while (pkt_num)
    {
        if (pkt_num < C_TSG_SEND_PKT_NUM)
            unit_num = pkt_num;
        else
            unit_num = C_TSG_SEND_PKT_NUM;

        unit_size = unit_num * C_TS_PKT_SIZE;


        cc_tsg_check_buf_busy((UINT32) m_cc_tsg_send_buf,(C_TSG_SEND_PAD_NUM +C_TSG_SEND_PKT_NUM)* C_TS_PKT_SIZE);
        MEMCPY(pkt_addr, buf, unit_size);
        if (!__cc_tsg_send_one_unit(m_cc_tsg_send_buf, pkt_addr, C_TSG_SEND_PAD_NUM + unit_num, unit_num))
            return FALSE;

        buf += unit_size;
        pkt_num -= unit_num;
    }
    //osal_task_sleep(1);
    return TRUE;
}
#endif

BOOL __cc_tsg_buf_is_empty(void)
{
    BOOL ret;

    ENTER_CC_TSG_API();
    ret = (m_tsg_buf_pos_write == m_tsg_buf_pos_read);
    LEAVE_CC_TSG_API();

    return ret;
}

static BOOL __cc_tsg_buf_is_full(void)
{
    BOOL ret = FALSE;

    ENTER_CC_TSG_API();

    do
    {
        if (m_tsg_buf_pos_write == m_tsg_buf_pos_read)
            break;
        if ((m_tsg_buf_pos_write + 1) == m_tsg_buf_pos_read)
        {
            ret = TRUE;
            break;
        }

        if (m_tsg_buf_pos_write == (MAX_TSG_BUF_NUM - 1) && m_tsg_buf_pos_read == 0)
        {
            ret = TRUE;
            break;
        }

    } while (0);
    if(FALSE == ret)
    {
            cc_tsg_check_buf_busy((UINT32)m_tsg_buf_list[m_tsg_buf_pos_write].tsg_buf, (CC_DMX_PKT_NUM + CC_PAD_PKT_NUM) * C_TS_PKT_SIZE);
    }

    LEAVE_CC_TSG_API();

    return ret;
}

static void cc_tsg_fill_null_pkt(UINT8 *ptr)
{
    UINT32 i, j;
    for(i = 0; i < CC_PAD_PKT_NUM; i++, ptr += C_TS_PKT_SIZE)
    {
        ptr[0] = 0x47;
        ptr[1] = 0x1f;
        ptr[2] = 0xff;
        ptr[3] = (i&0xf)|0x10;
        for(j = 4; j < C_TS_PKT_SIZE; j++)
            *(ptr+j) = 0;
    }
}

static void cc_tsg_buf_init(void)
{
    UINT32 i;

    if(m_cc_ts_stream_buf == NULL)
    {
        m_cc_ts_stream_buf = MALLOC(CC_TSG_STREAM_BUF_LEN+0x20);
        m_cc_ts_stream_buf = (UINT8 *)(((UINT32)m_cc_ts_stream_buf+0x10)&0xfffffff0);// 16 bytes aline;
    }
#ifdef CC_TSG_USE_REC_BUF
    if(m_tsg_rec_stream_buf == NULL)
    {
        m_tsg_rec_stream_buf = MALLOC(CC_TSG_REC_BUF_LEN+0x20);
        m_tsg_rec_stream_buf = (UINT8 *)(((UINT32)m_cc_tsg_send_buf+0x10)&0xfffffff0);// 16 bytes aline;
    }
#endif
    if(m_cc_tsg_send_buf == NULL)
    {
        m_cc_tsg_send_buf = MALLOC(CC_TSG_SEND_BUF_LEN+0x20);
        m_cc_tsg_send_buf = (UINT8 *)(((UINT32)m_cc_tsg_send_buf+0x10)&0xfffffff0);// 16 bytes aline;
    }

    UINT8 *buf = m_cc_ts_stream_buf;
    for (i = 0; i < MAX_TSG_BUF_NUM; i++)
    {
        m_tsg_buf_list[i].tsg_buf = buf;
        m_tsg_buf_list[i].dmx_buf = buf + CC_PAD_PKT_NUM * C_TS_PKT_SIZE;
        m_tsg_buf_list[i].pkt_num = CC_DMX_PKT_NUM;
        cc_tsg_fill_null_pkt(buf);

        buf += (CC_DMX_PKT_NUM + CC_PAD_PKT_NUM) * C_TS_PKT_SIZE;
    }
    m_tsg_buf_pos_write = 0;
    m_tsg_buf_pos_read = 0;
    m_tsg_record_buf_pos = 0;

#ifdef CC_TSG_SPLIT_BUF
    MEMCPY(m_cc_tsg_send_buf, m_cc_ts_stream_buf, C_TSG_SEND_PAD_NUM * C_TS_PKT_SIZE);
#endif
}

static pcc_tsg_buf_t cc_dmx_get_free_buf(UINT8 **pbuf, UINT32 *size)
{
    UINT32 flgptn;
    UINT32 cnt = 0;

    while (__cc_tsg_buf_is_full()&&cnt<10)
    {
        //DBG_STR("CC TSG buffer FULL <<<<");
        CC_TSG_PRINTF("<");
        cnt++;
        osal_flag_wait(&flgptn, g_cc_tsg_flagid, CC_TSG_BUF_FREE_FLAG, OSAL_TWF_ANDW|OSAL_TWF_CLR, 10);
    }

    UINT32 i = m_tsg_buf_pos_write;

    *pbuf = m_tsg_buf_list[i].dmx_buf;
    *size = CC_DMX_PKT_NUM * C_TS_PKT_SIZE;

    return &m_tsg_buf_list[i];
}

static UINT32 cc_tsg_get_ready_buf(UINT8 **pbuf)
{
    UINT32 flgptn;

    while (__cc_tsg_buf_is_empty())
    {
        //CC_TSG_PRINTF(">");
        osal_flag_wait(&flgptn, g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG, OSAL_TWF_ANDW|OSAL_TWF_CLR, 100);

        if (m_cc_tsg_task_running == 0)
            return 0;
    };

    UINT32 i = m_tsg_buf_pos_read;

    *pbuf = m_tsg_buf_list[i].tsg_buf;
    return m_tsg_buf_list[i].pkt_num;
}

INT32 cc_tsg_request_cache_buffer(UINT32 handle, UINT8 **addr, UINT32 length)
{
    UINT32 size = 0;

    cc_dmx_get_free_buf(addr, &size);

    //if (size == length)
    {
        return size;
    }

    return 0;
}

//#define CC_TSG_CHECK_PKT_COUNTER2

struct ts_pkt_header{
    UINT8 sync_word;

    UINT8 pid_msb5 : 5;
    UINT8 tp_priority:1;
    UINT8 plu_start_indi:1;
    UINT8 tp_err_indi:1;

    UINT8 pid_lsb8;

    UINT8  conti_conter:4;
    UINT8 adapt_field_ctrl:2;
    UINT8 tp_scramb_ctrl:2;
};

INT32 cc_tsg_update_cache_buffer(UINT32 handle, UINT32 size, UINT16 offset)
{
    //ASSERT(size == CC_DMX_PKT_NUM * C_TS_PKT_SIZE);
    UINT32 pkt_num = size / C_TS_PKT_SIZE;
    UINT32 buf_pos;
    UINT8 *dmx_buf;
    UINT32 tsg_remain_buf;
    UINT32 __MAYBE_UNUSED__ tsg_id;

    //ASSERT(handle);
    if (handle == 0)
    {
        return RET_FAILURE;
    }

    ENTER_CC_TSG_API();

    if (handle != CC_TSG_PLAY_HANDLE)
        m_tsg_task_exit_safely = 0;

    buf_pos = m_tsg_buf_pos_write;

    // To avoid ASSERT(0) in tsg driver
    if (pkt_num & 1)
    {
        dmx_buf = m_tsg_buf_list[m_tsg_buf_pos_write].dmx_buf;
        dmx_buf += size;
        // put an empty packet on the tail
        MEMCPY(dmx_buf, m_tsg_buf_list[m_tsg_buf_pos_write].tsg_buf, C_TS_PKT_SIZE);
        pkt_num++;
    }

    m_tsg_buf_list[m_tsg_buf_pos_write].pkt_num = pkt_num;
    m_tsg_buf_list[m_tsg_buf_pos_write].handle = handle;
    m_tsg_buf_list[m_tsg_buf_pos_write].offset = offset;

#ifdef CC_TSG_USE_REC_BUF
    if (handle != CC_TSG_PLAY_HANDLE)
    {
        dmx_buf = m_tsg_buf_list[m_tsg_buf_pos_write].dmx_buf;
        UINT8 *rec_buf = m_tsg_rec_stream_buf + (CC_DMX_PKT_NUM * C_TS_PKT_SIZE) * m_tsg_buf_pos_write;
        MEMCPY(rec_buf, dmx_buf, CC_DMX_PKT_NUM * C_TS_PKT_SIZE);
    }
#endif

    if (m_tsg_buf_pos_write == MAX_TSG_BUF_NUM - 1)
        m_tsg_buf_pos_write = 0;
    else
        m_tsg_buf_pos_write++;

    LEAVE_CC_TSG_API();

    if (pvr_r_get_record_all())
        goto LABEL_SKIP_TSG_PLAY;

#ifdef CC_TSG_RUN_IN_DMX_TASK

    if(m_cc_tsg_av_flag)
    {
        UINT32 len_with_pad;
        UINT32 buffer_empty = 0;
        //UINT32 pvr_err_count;
        UINT32 param[2];
        UINT32 tsg_buf_near_full;

           while(m_tsg_buf_pos_read!=m_tsg_buf_pos_write)
              {
            if (m_cc_tsg_task_running == 0)
                break;

                if(m_tsg_buf_pos_write<m_tsg_buf_pos_read)
                {
                    if(m_tsg_buf_pos_write+2>=MAX_TSG_BUF_NUM)
                    {
                        tsg_buf_near_full =1;
                    }
                    else
                    {
                        if(m_tsg_buf_pos_write+2<m_tsg_buf_pos_read)
                        {
                            tsg_buf_near_full =0;
                        }
                        else
                        {
                            tsg_buf_near_full =1;
                        }
                    }
                }
                else
                {
                    if(m_tsg_buf_pos_write+2<MAX_TSG_BUF_NUM)
                    {
                        tsg_buf_near_full =0;
                    }
                    else
                    {
                        if((m_tsg_buf_pos_write+2-MAX_TSG_BUF_NUM)<m_tsg_buf_pos_read)
                        {
                            tsg_buf_near_full =0;
                        }
                        else
                        {
                            tsg_buf_near_full =1;
                        }
                    }
                }
        len_with_pad = m_tsg_buf_list[m_tsg_buf_pos_read].pkt_num + CC_PAD_PKT_NUM;

        dmx_io_control(m_tsg_dmx_dev, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
        tsg_remain_buf = tsg_check_remain_buf();
        if((buffer_empty < len_with_pad+tsg_remain_buf)&&(0==tsg_buf_near_full))
        {
                break;
        }
        if(1 == tsg_buf_near_full)
              {
                    while(buffer_empty < len_with_pad+tsg_remain_buf)
                    {
                        osal_task_sleep(10);
                       dmx_io_control(m_tsg_dmx_dev, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
                       tsg_remain_buf = tsg_check_remain_buf();
                    }
              }
           buf_pos =  m_tsg_buf_pos_read;
        param[0] = (UINT32)(m_tsg_buf_list[buf_pos].dmx_buf);
        param[1] =  m_tsg_buf_list[m_tsg_buf_pos_read].pkt_num;
        dmx_io_control(m_tsg_dmx_dev, TSG_PLAYBACK_SYNC, (UINT32)&param[0]);
        //CC_TSG_PRINTF(".");
//#define CC_TSG_CHECK_PKT_COUNTER
#ifdef CC_TSG_CHECK_PKT_COUNTER
        UINT8 * buf = m_tsg_buf_list[buf_pos].tsg_buf;
        int j;
        for(j = 0; j < len_with_pad; j++,buf+=188)
        {
            if((buf[1]&0x1f)==(m_cc_tsg_video_pid >> 8) && buf[2]==(m_cc_tsg_video_pid&0xff))
            {
                static UINT8 conti_conter = 0xff;
                    struct ts_pkt_header * ts_header;
                    ts_header= (struct ts_pkt_header * )buf;
                    if(((conti_conter+1)&0x0f)!=ts_header->conti_conter)
                    {
                        if(0xff!=conti_conter)
                        {
                            libc_printf("22dis\n");
                        }
                    }
                    conti_conter = ts_header->conti_conter;
            }
        }
#endif

        osal_cache_flush((m_tsg_buf_list[buf_pos].tsg_buf),len_with_pad*C_TS_PKT_SIZE);
        tsg_id = tsg_transfer((void*)(m_tsg_buf_list[buf_pos].tsg_buf), len_with_pad, FALSE);
            ENTER_CC_TSG_API();

            if (m_tsg_buf_pos_read == MAX_TSG_BUF_NUM - 1)
                m_tsg_buf_pos_read = 0;
            else
                m_tsg_buf_pos_read++;

            LEAVE_CC_TSG_API();

            osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_FREE_FLAG);
        //osal_task_sleep(10); // 10
        //tsg_wait(tsg_id);
        }
    }
    else
    {
        cc_tsg_send_data(m_tsg_buf_list[buf_pos].dmx_buf, pkt_num);
    }
#endif

LABEL_SKIP_TSG_PLAY:

    dmx_buf = NULL;

#ifdef CC_TSG_CHECK_PKT_COUNTER2
    UINT8 *buf = m_tsg_buf_list[buf_pos].dmx_buf;
    UINT32 j;
    for(j = 0; j < pkt_num; j++,buf+=188)
    {
        if((buf[1]&0x1f)==(m_cc_tsg_video_pid >> 8) && buf[2]==(m_cc_tsg_video_pid&0xff))
        {
            static UINT8 conti_conter = 0xff;
                struct ts_pkt_header * ts_header;
                ts_header= (struct ts_pkt_header * )buf;
                if(((conti_conter+1)&0x0f)!=ts_header->conti_conter)
                {
                    if(0xff!=conti_conter)
                    {
                        libc_printf("22dis\n");
                    }
                }
                conti_conter = ts_header->conti_conter;
        }
        else if((buf[1]&0x1f)==(m_cc_tsg_audio_pid >> 8) && buf[2]==(m_cc_tsg_audio_pid&0xff))
        {
            static UINT8 conti_conter = 0xff;
                struct ts_pkt_header * ts_header;
                ts_header= (struct ts_pkt_header * )buf;
                if(((conti_conter+1)&0x0f)!=ts_header->conti_conter)
                {
                    if(0xff!=conti_conter)
                    {
                        libc_printf("A22dis\n");
                    }
                }
                conti_conter = ts_header->conti_conter;
        }
        else if((buf[1]&0x1f)==0x02 && buf[2]==0x40)
        {
            pkt_num = 1000;
        }
    }
#endif

    osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG);

    return RET_SUCCESS;
}

static void __cc_tsg_write_one_package(void)
{
    UINT8 *buf = NULL;
    UINT32 len;// len_with_pad;
    //UINT8 *rec_buf = NULL;
    //UINT32 rec_buf_len, rec_buf_pos;
    UINT32 __MAYBE_UNUSED__ rec_hdl;

    len = cc_tsg_get_ready_buf(&buf);
    if (len == 0)
        return;

    rec_hdl = m_tsg_buf_list[m_tsg_buf_pos_read].handle;

#if 0
    if (rec_hdl != CC_TSG_PLAY_HANDLE)
    {
        if (recorder_get_cc_state(rec_hdl) == CC_REC_PLAY_ALONE)
            rec_hdl = CC_TSG_PLAY_HANDLE;
    }


    if (rec_hdl != CC_TSG_PLAY_HANDLE)
    {
        len = len * C_TS_PKT_SIZE;
        rec_buf_len = _recorder_request_cache_buffer(rec_hdl, &rec_buf, len);
        if ((UINT32)rec_buf_len >= len && rec_buf)
        {
            MEMCPY(rec_buf, m_tsg_buf_list[m_tsg_buf_pos_read].dmx_buf, len);
            _recorder_update_cache_buffer(rec_hdl, len, m_tsg_buf_list[m_tsg_buf_pos_read].offset);
            //pvr_err_count = 0;
        }
        else
        {
            CC_TSG_PRINTF(",");
            //ASSERT(0);
        }
    }
#endif
    ENTER_CC_TSG_API();

    if (m_tsg_buf_pos_read == MAX_TSG_BUF_NUM - 1)
        m_tsg_buf_pos_read = 0;
    else
        m_tsg_buf_pos_read++;

    LEAVE_CC_TSG_API();
}

static void c_tsg_task_proc(UINT32 param1, UINT32 param2)
{
    UINT32 flgptn;
    UINT8 *buf = NULL;
    UINT32 __MAYBE_UNUSED__ len, len_with_pad;
    UINT32 __MAYBE_UNUSED__ buffer_empty = 0;
    struct dmx_device __MAYBE_UNUSED__ *dmx;
    //UINT8 *rec_buf = NULL;
    //UINT32 rec_buf_len, rec_buf_pos;
    //UINT32 tsg_id, rec_hdl;
    UINT32 __MAYBE_UNUSED__ pvr_err_count;
    UINT32 __MAYBE_UNUSED__ tsg_remain_buf;

    while(1)
    {
        //DBG_STR("wait CC_TSG_TSK_RUN_FLAG");
        osal_flag_wait(&flgptn, g_cc_tsg_flagid, CC_TSG_TSK_RUN_FLAG, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
        //DBG_STR("got  CC_TSG_TSK_RUN_FLAG");

        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        pvr_err_count = 0;

        while (m_cc_tsg_task_running)
        {
#ifdef CI_SLOT_DYNAMIC_DETECT
            cc_tsg_ci_slot_detect();
#endif
              len = cc_tsg_get_ready_buf(&buf);
            if (len == 0)
                break;

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
            cc_tsg_ci_slot_set_scramb_flag(buf, len, m_cc_tsg_video_pid, m_cc_tsg_audio_pid);
#endif

#ifndef CC_TSG_RUN_IN_DMX_TASK
#ifndef CC_TSG_SPLIT_BUF
            len_with_pad = len + CC_PAD_PKT_NUM;
            dmx_io_control(dmx, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
            tsg_remain_buf = tsg_check_remain_buf();
            while(buffer_empty < len_with_pad+tsg_remain_buf)
            {
                if (m_cc_tsg_task_running == 0)
                    break;

                osal_task_sleep(50);
                dmx_io_control(dmx, CHECK_DMX_REMAIN_BUF, (UINT32)&buffer_empty);
                tsg_remain_buf = tsg_check_remain_buf();
            }
#else
            cc_tsg_send_data(buf, len);
#endif
#endif
            if (m_cc_tsg_task_running == 0)
                break;

#ifdef CC_TSG_DETECT_BITRATE
            if (m_tsg_speed_set_flag == 0)
            {
                m_tsg_dmx1_packet_num += len;
                if (m_tsg_dmx1_start_time == 0)
                {
                    m_tsg_dmx1_start_time = osal_get_tick();
                    m_tsg_dmx1_start_num = 0;
                }
                else
                {
                    UINT32 duration = osal_get_tick() - m_tsg_dmx1_start_time;
                    if (duration >= 2000)
                    {
                        if (m_tsg_dmx1_start_num == 0)
                        {
                            m_tsg_dmx1_start_time2 = m_tsg_dmx1_start_time + duration;
                            m_tsg_dmx1_start_num = m_tsg_dmx1_packet_num;
                        }
                        else
                        {
                            if (duration >= 5000)
                            {
                                UINT32 bitrate = m_tsg_dmx1_packet_num - m_tsg_dmx1_start_num;
                                duration -= (m_tsg_dmx1_start_time2 - m_tsg_dmx1_start_time);
                                bitrate *= 188;
                                bitrate /= duration;
                                bitrate *= 8192;
                                m_tsg_dmx1_bitrate = bitrate;
                                cc_tsg_task_set_speed(bitrate);
                                m_tsg_speed_set_flag = 1;
                            }
                        }
                    }
                }
            }
#endif
#ifdef CI_SLOT_DYNAMIC_DETECT
            cc_tsg_ci_slot_data_received(len);
#endif
#ifndef CC_TSG_RUN_IN_DMX_TASK
#ifndef CC_TSG_SPLIT_BUF
            UINT32 param[2];
            param[0] = (UINT32)(m_tsg_buf_list[m_tsg_buf_pos_read].dmx_buf);
            param[1] = len;
            dmx_io_control(dmx, TSG_PLAYBACK_SYNC, (UINT32)&param[0]);
            //CC_TSG_PRINTF(".");
            osal_cache_flush(buf, len_with_pad*C_TS_PKT_SIZE);
            tsg_id = tsg_transfer((void*)(buf), len_with_pad, FALSE);

            //osal_task_sleep(10); // 10
#endif
#endif

#if 0
            rec_hdl = m_tsg_buf_list[m_tsg_buf_pos_read].handle;
            //ASSERT(rec_hdl);
            if (rec_hdl == CC_TSG_PLAY_HANDLE)
                pvr_err_count = 0;
            else
            {
                if (recorder_get_cc_state(rec_hdl) == CC_REC_PLAY_ALONE)
                    rec_hdl = CC_TSG_PLAY_HANDLE;
            }

            if (rec_hdl != CC_TSG_PLAY_HANDLE && pvr_err_count < 2)
            {
#if (CC_DMX_PKT_NUM == 256)
                len = len * C_TS_PKT_SIZE;
                rec_buf_len = _recorder_request_cache_buffer(rec_hdl, &rec_buf, len);
                if ((UINT32)rec_buf_len >= len && rec_buf)
                {
                    MEMCPY(rec_buf, m_tsg_buf_list[m_tsg_buf_pos_read].dmx_buf, len);
                       _recorder_update_cache_buffer(rec_hdl, len, m_tsg_buf_list[m_tsg_buf_pos_read].offset);
                    pvr_err_count = 0;
                }
                else
                {
                    CC_TSG_PRINTF(",");
                    pvr_err_count++;
                    //ASSERT(0);
                }
#else
                len = len * C_TS_PKT_SIZE;
                if (m_tsg_record_buf_pos == 0)
                {
                    rec_buf_len = _recorder_request_cache_buffer(rec_hdl, &rec_buf, 256 * C_TS_PKT_SIZE);
                    if ((UINT32)rec_buf_len == 256 * C_TS_PKT_SIZE && rec_buf)
                    {
                        MEMCPY(rec_buf, m_tsg_buf_list[m_tsg_buf_pos_read].dmx_buf, len);
                        m_tsg_record_buf_pos++;
                        rec_buf_pos = len;
                        if (m_tsg_record_buf_pos == (256/CC_DMX_PKT_NUM))
                        {
                            m_tsg_record_buf_pos = 0;
                            _recorder_update_cache_buffer(rec_hdl, rec_buf_pos, m_tsg_buf_list[m_tsg_buf_pos_read].offset);
                        }
                        pvr_err_count = 0;
                    }
                    else
                    {
                        CC_TSG_PRINTF(",");
                        pvr_err_count++;
                        //ASSERT(0);
                    }
                }
                else
                {
                    MEMCPY(rec_buf + rec_buf_pos, m_tsg_buf_list[m_tsg_buf_pos_read].dmx_buf, len);
                    rec_buf_pos += len;
                    m_tsg_record_buf_pos++;
                    if (m_tsg_record_buf_pos == (256/CC_DMX_PKT_NUM))
                    {
                        m_tsg_record_buf_pos = 0;
                        _recorder_update_cache_buffer(rec_hdl, rec_buf_pos, m_tsg_buf_list[m_tsg_buf_pos_read].offset);
                    }
                }
#endif
            }
#endif

#ifndef CC_TSG_RUN_IN_DMX_TASK
#ifndef CC_TSG_SPLIT_BUF
            //tsg_wait(tsg_id);
            ENTER_CC_TSG_API();

            if (m_tsg_buf_pos_read == MAX_TSG_BUF_NUM - 1)
                m_tsg_buf_pos_read = 0;
            else
                m_tsg_buf_pos_read++;

            LEAVE_CC_TSG_API();

            osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_FREE_FLAG);
#endif
#else
            osal_task_sleep(1);
#endif

        }
        osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_FREE_FLAG);
    }
}

void cc_tsg_task_init(void)
{
    OSAL_T_CTSK        t_ctsk;

    m_tsg_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    m_tsg_dmx_dev2 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    m_tsg_deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);

    if (OSAL_INVALID_ID == m_cc_tsg_sema_id)
    {
        m_cc_tsg_sema_id = osal_semaphore_create(1);
        ASSERT(m_cc_tsg_sema_id != OSAL_INVALID_ID);
    }
#ifdef CI_SLOT_DYNAMIC_DETECT
    cc_tsg_ci_slot_init();
#endif
    if(g_cc_tsg_flagid == OSAL_INVALID_ID)
    {
        g_cc_tsg_flagid = osal_flag_create(0);
        ASSERT(g_cc_tsg_flagid != OSAL_INVALID_ID)
    }

//    if(pvr_get_with_tsg_using_status() == 0)
//        return RET_SUCCESS;

    if (m_cc_tsg_task_id == OSAL_INVALID_ID)
    {
        t_ctsk.stksz    =   0x1000;
        t_ctsk.quantum  =   5;
        t_ctsk.itskpri  =   OSAL_PRI_HIGH;
        t_ctsk.name[0]  =   'T';
        t_ctsk.name[1]  =   'S';
        t_ctsk.name[2]  =   'C';
        t_ctsk.task = (FP)c_tsg_task_proc;
        t_ctsk.para1 = 0;
        t_ctsk.para2 = 0;

        m_cc_tsg_task_id = osal_task_create(&t_ctsk);
        ASSERT(m_cc_tsg_task_id != OSAL_INVALID_ID);

        m_cc_tsg_task_running = 0;
    }

    cc_tsg_buf_init();

     return;
}

BOOL cc_tsg_task_using_tsg_is_running(void)
{
    return (m_cc_tsg_task_running);
}

BOOL cc_tsg_task_wait_empty(UINT32 handle)
{

    if(pvr_get_with_tsg_using_status() == 0)
        return TRUE;

    if (handle == m_cc_tsg_rec_hdl)
    {
        while (m_cc_tsg_task_running && (m_tsg_task_exit_safely == 0) && !__cc_tsg_buf_is_empty())
        {
            osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG);
            osal_task_sleep(5);
        }
        if (m_cc_tsg_task_running == 0 && (m_tsg_task_exit_safely == 0) && !__cc_tsg_buf_is_empty())
        {
            __cc_tsg_write_one_package();
        }
        m_tsg_task_exit_safely = 1;
    }
    return TRUE;
}

RET_CODE cc_tsg_task_start(UINT32 prog_id)
{
    if(pvr_get_with_tsg_using_status() == 0)
        return RET_SUCCESS;

    if (prog_id == 0)
        return RET_FAILURE;

    if (m_cc_tsg_task_running == 0)
    {
        CC_TSG_PRINTF("%s %d\n", __FUNCTION__, osal_get_tick());

        m_tsg_dmx1_bitrate = HD_MPEG2_BITRATE + 1;

#ifdef CC_TSG_DETECT_BITRATE
        cc_tsg_task_set_speed(HD_MPEG2_BITRATE + 1);

        m_tsg_dmx1_start_time = 0;
        m_tsg_dmx1_start_time2 = 0;
        m_tsg_dmx1_packet_num = 0;
        m_tsg_dmx1_start_num = 0;
        m_tsg_speed_set_flag = 0;
#else
#ifdef PLAYER_DETECT_BITRATE
        cc_tsg_task_set_speed(HD_MPEG2_BITRATE + 1);
#endif
#endif

        deca_io_control(m_tsg_deca_dev, SET_PASS_CI, TRUE);

        dmx_io_control(m_tsg_dmx_dev, IO_STREAM_DISABLE, (UINT32)NULL);
        //dmx_io_control(m_tsg_dmx_dev, IO_DMX_RESET, (UINT32)NULL);
//#ifdef CI_SLOT_DYNAMIC_DETECT
        cc_tsi_dmx_config(TSI_DMX_0, TRUE, TRUE);
//#endif
        dmx_io_control(m_tsg_dmx_dev, SET_TSG_PLAYBACK, (UINT32)TRUE);
#ifdef CI_SLOT_DYNAMIC_DETECT
        cc_tsg_set_tsg_play_flag(TRUE);
#endif
        m_tsg_buf_pos_write = 0;
        m_tsg_buf_pos_read = 0;
        m_tsg_record_buf_pos = 0;

        m_cc_tsg_task_running = 1;
        m_tsg_task_exit_safely = 1;
#ifdef CI_SLOT_DYNAMIC_DETECT
        cc_tsg_ci_slot_reset();
        cc_tsg_ci_slot_start();
#endif
        tsg_start(0);   //use the default bitrate
        osal_flag_set(g_cc_tsg_flagid, CC_TSG_TSK_RUN_FLAG);

        m_cc_tsg_rec_handle = pvr_start_memory_record(m_tsg_dmx_dev2, prog_id);
        m_cc_tsg_rec_hdl = m_cc_tsg_rec_handle;
        if (m_cc_tsg_rec_handle == 0)
        {
            cc_tsg_task_stop();
            return RET_FAILURE;
        }
        dmx_io_control(m_tsg_dmx_dev, IO_STREAM_ENABLE, (UINT32)NULL);
#ifdef CI_SLOT_DYNAMIC_DETECT
        if (m_cc_tsg_prog_id == prog_id)
            cc_tsg_ci_slot_set_ca_pmt_received(1);
        else
#endif
            m_cc_tsg_prog_id = prog_id;
//#ifdef CI_SLOT_DYNAMIC_DETECT
        cc_tsg_task_set_pause_count(0);
//#endif
    }
    return RET_SUCCESS;
}

RET_CODE cc_tsg_task_stop0(BOOL stop_it)
{
    RET_CODE ret = RET_SUCCESS;
    UINT32 i = m_cc_tsg_task_running;
    UINT32 time_out;

    if (stop_it)
    {
        m_tsg_task_exit_safely = 1;
    }

    while (!__cc_tsg_buf_is_empty())
    {
        //osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG);
        osal_task_sleep(5);
    }

    if (m_cc_tsg_rec_handle)
    {
        ret = pvr_stop_memory_record(m_tsg_dmx_dev2, (UINT32)m_cc_tsg_rec_handle);
#if 0
        enum CC_REC_STATE cc_rec_state = recorder_get_cc_state(m_cc_tsg_rec_handle);
        if (cc_rec_state == CC_REC_FREE)
#endif
        {
            m_tsg_task_exit_safely = 1;
            m_cc_tsg_task_running = 0;
        }
        m_cc_tsg_rec_handle = 0;
    }

    if (m_cc_tsg_task_running)
    {
        while (!__cc_tsg_buf_is_empty())
        {
            //osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG);
            osal_task_sleep(5);
        }

        time_out = 0;
        while (!stop_it && !m_tsg_task_exit_safely)
        {
            if (time_out >= 200)
            {
                m_tsg_task_exit_safely = 1;
                break;
            }
            //osal_flag_set(g_cc_tsg_flagid, CC_TSG_BUF_READY_FLAG);
            osal_task_sleep(5);
            time_out++;
        }

        osal_task_dispatch_off();
        m_cc_tsg_task_running = 0;
        osal_task_dispatch_on();
        CC_TSG_PRINTF("%s\n", __FUNCTION__);
    }
    osal_flag_clear(g_cc_tsg_flagid, CC_TSG_TSK_RUN_FLAG);
    tsg_stop();

    if (i)
    {
        dmx_io_control(m_tsg_dmx_dev, IO_STREAM_DISABLE, (UINT32)NULL);
        //dmx_io_control(m_tsg_dmx_dev, IO_DMX_RESET, (UINT32)NULL);
        dmx_io_control(m_tsg_dmx_dev, SET_TSG_PLAYBACK, (UINT32)FALSE);
#ifdef CI_SLOT_DYNAMIC_DETECT
        cc_tsg_set_tsg_play_flag(FALSE);
#endif
        cc_tsi_dmx_config(TSI_DMX_0, FALSE, FALSE);
//#endif
        deca_io_control(m_tsg_deca_dev, SET_PASS_CI, FALSE);
    }

    if (stop_it)
        m_cc_tsg_rec_hdl = 0;

    return ret;
}

RET_CODE cc_tsg_task_stop(void)
{
    if(pvr_get_with_tsg_using_status() == 0)
        return RET_SUCCESS;
//#ifdef CI_SLOT_DYNAMIC_DETECT
    cc_tsg_task_set_pause_count(0);
//#endif
    m_tsg_task_exit_safely = 1;
    cc_tsg_task_stop0(TRUE);
    m_cc_tsg_prog_id = 0; // reset it
    return  RET_SUCCESS;
}

UINT8* cc_tsg_task_get_buffer(void)
{
    if (m_cc_tsg_task_id == OSAL_INVALID_ID)
        cc_tsg_buf_init();

    return m_cc_ts_stream_buf;
}

UINT32 cc_tsg_task_get_speed(void)
{
    if(pvr_get_with_tsg_using_status() == 0)
        return 0;

    return m_tsg_dmx1_bitrate;
}

#if 0
BOOL cc_tsg_task_reset_dmx(void)
{
    CC_TSG_PRINTF("%s\n", __FUNCTION__);

    dmx_io_control(m_tsg_dmx_dev, IO_STREAM_DISABLE, 0);
    osal_task_sleep(150);
    dmx_io_control(m_tsg_dmx_dev, IO_STREAM_ENABLE, 0);
    //api_send_ca_pmt_auto();

    return TRUE;
}
#endif

BOOL cc_tsg_task_set_pids(UINT16 vpid, UINT16 apid, UINT8 av_flag)
{
    m_cc_tsg_video_pid = vpid & 0x1fff;
    m_cc_tsg_audio_pid = apid & 0x1fff;
    m_cc_tsg_av_flag = av_flag;
    return TRUE;
}

#if (defined(CC_TSG_DETECT_BITRATE) || defined(PLAYER_DETECT_BITRATE))

#if (CC_PAD_PKT_NUM == 256)
UINT8 g_tsg_clock_slow = 0x0E;  // 3C 0x5a * 4 / 6
UINT8 g_tsg_clock_fast = 0x0E;  // 0x0E is recommanded by Norman Yang
#elif (CC_PAD_PKT_NUM == 128)
UINT8 g_tsg_clock_slow = 0x40;  // 0x5a * 4 / 6
UINT8 g_tsg_clock_fast = 0x0E;  // 0x0E is recommanded by Norman Yang
#elif (CC_PAD_PKT_NUM == 64)
UINT8 g_tsg_clock_slow = 0x48;  // 0x5a * 4 / 6
UINT8 g_tsg_clock_fast = 0x0E;  // 0x0E is recommanded by Norman Yang
#else
UINT8 g_tsg_clock_slow = 0x58;
UINT8 g_tsg_clock_fast = 0x18;
#endif

void cc_tsg_task_set_speed(UINT32 bit_rate)
{
    //patch IC (M3602B, S3602D) CI bug. some CI card cann't work at fast speed mode.
    //dynamic adjust the speed,
    CC_TSG_PRINTF("tsg task speed %d\n", bit_rate);

    UINT8 new_value;

    if(bit_rate > HD_MPEG2_BITRATE)
    {
        new_value = g_tsg_clock_fast; // fast speed.
    }
    else
    {
        new_value = g_tsg_clock_slow; // slow speed.
    }

    tsg_set_clk(new_value);
}
#endif

//#endif
