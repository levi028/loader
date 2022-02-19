/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie.c
*
*    Description: get SI data from DMX and dispatch it to upper layer application
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libc/list.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <api/libc/fast_crc.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_module.h>
#include <hld/dmx/dmx.h>

#define DUPLICATE_PID_SUPPORT
#define SIE_DEBUG_LEVEL             0
#define DBG_PRINTF(...)             do{}while(0)
#if (SIE_DEBUG_LEVEL>1)
#include <api/libc/printf.h>
#define SIE_PRINTF              libc_printf
#define RINGBUF_PRINTF          libc_printf
#elif (SIE_DEBUG_LEVEL==1)
#include <api/libc/printf.h>
#define SIE_PRINTF              PRINTF
#define RINGBUF_PRINTF          libc_printf
#else
#define SIE_PRINTF(...)             do{}while(0)
#define RINGBUF_PRINTF(...)         do{}while(0)
#endif

enum
{
    SIE_MODULE_TASK_QUANTUM         = 7,
    SIE_MODULE_TASK_STACKSIZE       = 0x3000,
    SIE_MODULE_TASK_PRIORITY        = OSAL_PRI_NORMAL,
};

enum
{
    SIE_FLAG_TASK_ABORTING          = 0x80000000UL,
    SIE_FLAG_TASK_ABORTED           = 0x40000000UL,
    SIE_FLAG_ENGINE_CLOSING         = 0x20000000UL,
    SIE_FLAG_ENGINE_CLOSED          = 0x10000000UL,
    SIE_FLAG_ENGINE_ENABLE          = 0x08000000UL,
    SIE_FLAG_MUTEX                  = 0x04000000UL,
    SIE_FLAG_RING_MUTEX             = 0x02000000UL,
};

enum
{
    SIE_FLT_FREE    = 0x00,
    SIE_FLT_IDLE    = 0x01,
    SIE_FLT_ACTIVE  = 0x02,
    SIE_FLT_RUNNING = 0x03,
};

//#define SIE_RINGBUF_RESERVE_LEN   0x1000

#define MAX_DMX_NUM     DMX_HW_SW_REAL_NUM

struct sie_request
{
    //struct list_head list;         //the double-linked list management.
    UINT8 dmx_filter;                //the demux filter number in demux device.
    UINT8 status;
    UINT16 max_sec_length;           //the maximum section length, normally 1024 or 4096.
    UINT16 reserved_len;             //ring buffer reserved for dmx.
    UINT32 timeout;                  //in mSecond, OSAL_WAIT_FOREVER_TIME for cyclic.
    UINT32 read;                     //the read pointer for ring buffer.
    UINT32 write;                    //the write pointer for ring buffer.
    //struct list_head *target;      //targeting linked list.
    struct get_section_param gsp;    //the param that demux need for a filter.
    struct si_filter_t filter;       //the filter structure provided to module API.
    struct si_filter_param fparam;
    UINT32 buf_len;                  //the total ring buffer length.
    UINT8 *buffer;                   //the ring buffer starting address.
    UINT8 ring_buffer_mode;          //0 - when ring buffer full, stop dmx getting section data. (for EPG)
                                     //1 - when ring buffer full, discard the oldest data. (for CA)
};

/* notice   : this structure is not packed, so some stuff byte might exist between 2 sect_buffers.*/
struct sect_buffer
{
    UINT16 section_length;  //the section length that follows.
    UINT16 hit_reason;      //the hit reason demux provides, along with SIE reason.
    UINT8 section[0];       //the section content.
};

struct sie_dmx_info
{
    OSAL_ID mutex_task;       //mutex of this sie task.
    OSAL_ID sie_ring_mutex;   //ring buffer mutex only for ring_buffer_mode 1.
    INT32 mutex_cnt;          //counter of mutex_task.
    OSAL_ID sie_flag;         //flag of this sie task.
    UINT32 status;            //the module status, inited, released, opened...
    UINT32 filter;            //the global filter status for demux device, only 32-way for maximum.
    UINT32 request_nr;        //the module's maximum concurrent request number, <= 32.
    void *addr;               //request list addr.
    struct dmx_device *dmx;   //the demultiplex device handle.

    /* when call external parser to parse a section of data,the parser is not mutex protected,
     * so use a tmp_buffer to avoid crash issue.*/
    UINT8 *tmp_buffer;

    OSAL_ID task_id;          //id of this sie task.
};

struct dmx_poll
{
    UINT32 mask;      //the mask bits the caller interested in.
    UINT32 timeout;   //the polling timeout value in mSecond.
};

#ifdef AUTO_OTA
typedef enum
{
    OTASRCH_USER_ABORT      = 0x80000000UL,
    OTASRCH_USER_ABORT_ACK  = 0x40000000UL,
    OTASRCH_END             = 0x20000000UL,
}ENUM_OTASRCH_FLAG;

static OSAL_ID m_otasrch_flgid = OSAL_INVALID_ID;
#endif

typedef INT32(*si_filter_op_t)(struct si_filter_t *, struct si_filter_param *);

static struct sie_dmx_info sie_info[MAX_DMX_NUM];
static OSAL_ID sie_module_flag = OSAL_INVALID_ID;

/*only ring buffer mode 1 need use this mutex*/
static __inline__ void sie_enter_ring_mutex(struct sie_dmx_info *info)
{
    UINT32 flag = 0;
    INT32 rst = 0;

    //osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_RING_MUTEX,OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
    rst = osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_RING_MUTEX,OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);

    if(E_FAILURE == rst)
    {
        PRINTF("osal_flag_wait() failed!\n");
    }
    info->sie_ring_mutex = osal_task_get_current_id(); //record which task get this ring_mutex,used for debug
}
static __inline__ void sie_leave_ring_mutex(struct sie_dmx_info *info)
{
    if(NULL == info)
    {
        return ;
    }

    osal_flag_set(info->sie_flag, SIE_FLAG_RING_MUTEX);
    info->sie_ring_mutex = osal_task_get_current_id();
}

static __inline__ void sie_enter_mutex(struct sie_dmx_info *info)
{
    UINT32 flag = 0;
    INT32  rst  = 0;

    if(NULL == info)
    {
        return ;
    }

    if (info->mutex_task != osal_task_get_current_id())
    {
        //osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_MUTEX, OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
        rst = osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_MUTEX,OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);

        if(E_FAILURE == rst)
        {
            PRINTF("osal_flag_wait() failed!\n");
        }
        info->mutex_task = osal_task_get_current_id();
    }
    osal_task_dispatch_off();
    ++info->mutex_cnt;
    osal_task_dispatch_on();
}

static __inline__ void sie_leave_mutex(struct sie_dmx_info *info)
{
    if(NULL == info)
    {
        return ;
    }

    osal_task_dispatch_off();
    if (0 == (--info->mutex_cnt))
    {
        info->mutex_task = OSAL_INVALID_ID;
        osal_flag_set(info->sie_flag, SIE_FLAG_MUTEX);
    }
    osal_task_dispatch_on();
}

static BOOL treat_return(sie_status_t ret,struct sect_buffer *buffer,struct sie_dmx_info *info,
    struct sie_request *request)
{
    INT32 ret2 = 0;

    //sie_enter_mutex(info);
    if ((sie_started==ret)&&((NULL==buffer)||(buffer->hit_reason&SIE_REASON_RINGBUF_FULL)))
    {
        if (SIE_FLT_ACTIVE == request->status)
        {
            dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request->dmx_filter);
            info->filter &= (~(1<<request->dmx_filter));
            request->gsp.continue_get_sec = 1;
            if (request->fparam.timeout != OSAL_WAIT_FOREVER_TIME)
            {
                request->timeout = osal_get_tick()+request->fparam.timeout;
            }
            else
            {
                request->timeout = OSAL_WAIT_FOREVER_TIME;
            }

			request->status = SIE_FLT_RUNNING;		//20151109: avoid multi thread issue

            sie_leave_mutex(info);
            // unlock here, because it will cause deadlock when ca playback(deadlock with on_dmx_event:sie_enter_mutex)
            ret2 = dmx_async_req_section(info->dmx, &request->gsp, &request->dmx_filter);
            sie_enter_mutex(info);
            if (ret2 !=RET_SUCCESS)
            {
                SIE_PRINTF("    ##request filter for pid[0x%x] fail!\n", request->gsp.pid);
                request->status = SIE_FLT_FREE;
                //sie_leave_mutex(info);
                return FALSE;
            }

			//20151109 begin:
			//FIXME: fatal error occurred during call dmx_async_req_section(),
			//       this request's status changed!
			if (request->status == SIE_FLT_RUNNING)
			{
				request->status = SIE_FLT_ACTIVE;
			}
			else
			{
				SIE_PRINTF("Warning: request(pid=0x%x) status(=%d) changed while re-request dmx filter!!!\n",pid,request->status);
				dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request->dmx_filter);
				request->status = SIE_FLT_FREE;
				return FALSE;
			}
			//20151109 end.

            //debug ring buffer only
            //if ((buffer != NULL) && (buffer->hit_reason&SIE_REASON_RINGBUF_FULL))
            {
                DBG_PRINTF("    ##request filter[%d],pid[0x%x],timeout[%d] continue to get section!\n",
                    request->dmx_filter, request->gsp.pid, request->timeout);
            }
            info->filter |= (1<<request->dmx_filter);
        }

    }
    else if ((sie_freed == ret)&&(SIE_FLT_ACTIVE == request->status))
    {
        dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request->dmx_filter);
        SIE_PRINTF("##filter[%d],pid[0x%x] freed!\n",request->dmx_filter,request->gsp.pid);
        info->filter &= (~(1<<request->dmx_filter));
        request->status = SIE_FLT_FREE;
    }
    //sie_leave_mutex(info);

    return TRUE;
}

static sie_status_t dispatch_section(struct sie_dmx_info *info, struct sie_request *request, UINT32 mask, UINT8 reason)
{
    sie_status_t ret = 0;
    struct sect_buffer *buffer = NULL;
    struct sie_request t_request;
    UINT16 section_len = 0;
    UINT16 hit_reason  = 0;
    UINT16 section_cnt = 0;
    UINT32 read_addr   = 0;
    UINT32 write_addr  = 0;

    if(NULL == request)
    {
        return sie_freed;
    }

    MEMSET(&t_request, 0, sizeof(struct sie_request));
    MEMCPY(&t_request, request, sizeof(struct sie_request));

    DBG_PRINTF("\n##%s():\n", __FUNCTION__);
    if(SIE_REASON_FILTER_TIMEOUT == reason)
    {
        request->status = SIE_FLT_RUNNING;
        sie_leave_mutex(info);
        ret = t_request.fparam.section_parser(t_request.gsp.pid,&t_request.filter,SIE_REASON_FILTER_TIMEOUT,NULL,0);
        SIE_PRINTF("##request filter[%d],pid[0x%x] time out,tick=%d!\n",
            request->dmx_filter,request->gsp.pid,osal_get_tick());
        sie_enter_mutex(info);
        if(SIE_FLT_RUNNING == request->status)
        {
            request->status = SIE_FLT_ACTIVE;
        }
        else
        {
            return sie_freed;
        }
    }
    else if((SIE_REASON_FILTER_HIT == reason)&&(request->read != request->write))
    {
        if (request->buf_len <= (request->reserved_len+sizeof(struct sect_buffer)+request->max_sec_length))
        {
            if (request->gsp.sec_tbl_len > SI_LONG_SECTION_LENGTH)
            {
#if (SIE_DEBUG_LEVEL > 0)
                SIE_PRINTF("%s: sec len %d > max length %d\n",__FUNCTION__,
                    request->gsp.sec_tbl_len,SI_LONG_SECTION_LENGTH);
                ASSERT(0);
#endif
            }
            else
            {
                MEMCPY(info->tmp_buffer, request->gsp.buff, request->gsp.sec_tbl_len);

                request->status = SIE_FLT_RUNNING;
                sie_leave_mutex(info);
                ret = t_request.fparam.section_parser(t_request.gsp.pid,
                &t_request.filter, t_request.gsp.sec_hit_num, info->tmp_buffer,
                t_request.gsp.sec_tbl_len);

                sie_enter_mutex(info);
                if(SIE_FLT_RUNNING == request->status)
                {
                    request->status = SIE_FLT_ACTIVE;
                }
                else
                {
                    return sie_freed;
                }
            }
            request->write = 0;
        }
#ifndef SIE_MINIMUM_MODEL
        else
        {
            if(0 == request->ring_buffer_mode)
            {
                section_cnt = 2;
            }
            else
            {
                section_cnt = 1;
            }

            if(1 == request->ring_buffer_mode)
            {
                sie_enter_ring_mutex(info);
            }

            /* here process data in ring buffer */
            while ((request->read!=request->write)&&(section_cnt))
            {
                buffer = (struct sect_buffer *)(request->buffer+request->read);
                DBG_PRINTF("    ****r: 0x%X, len: %d\n",request->read,buffer->section_length);
                if (buffer->section_length > SI_LONG_SECTION_LENGTH)
                {
#if (SIE_DEBUG_LEVEL > 0)
                    RINGBUF_PRINTF("sie: sec len %d > max sec len %d!\n",buffer->section_length,SI_LONG_SECTION_LENGTH);
                    ASSERT(0);
#endif
                }
                else
                {
                    if (SI_LONG_SECTION_LENGTH < buffer->section_length) // avoid MEMCPY overflow
                    {
                        ASSERT(0);
                        return sie_freed;
                    }
                    MEMCPY(info->tmp_buffer, buffer->section, buffer->section_length);
                    section_len = buffer->section_length;
                    hit_reason = buffer->hit_reason;

                    request->status = SIE_FLT_RUNNING;
                    sie_leave_mutex(info);
                    //do NOT protect callback to avoid dead lock!
                    ret = t_request.fparam.section_parser(t_request.gsp.pid,&t_request.filter,
                                hit_reason, info->tmp_buffer,section_len);
                    sie_enter_mutex(info);

                    if(SIE_FLT_RUNNING == request->status)
                    {
                        request->status = SIE_FLT_ACTIVE;
                    }
                    else
                    {
                        if(1 == t_request.ring_buffer_mode)
                        {
                            sie_leave_ring_mutex(info);
                        }
                        return sie_freed;
                    }
                }

                if (ret != sie_started)
                {
                    RINGBUF_PRINTF("    ****ret: %d, request status: %d\n",ret,request->status);
                    break;
                }

                request->read += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);
                DBG_PRINTF("    ****ring buffer read move to 0x%x!\n", request->read);
                --section_cnt;

                read_addr = (request->read+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len);
                if (read_addr > request->buf_len)
                {
                    request->read = 0;
                    RINGBUF_PRINTF("    ****ring buffer read rewind to 0!\n");

                    //rewind write!!!
                   write_addr=(request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len);
                   if (((buffer->hit_reason&SIE_REASON_RINGBUF_FULL) != 0) && (write_addr > request->buf_len))
                   {
                       RINGBUF_PRINTF("    &&&&ring buffer write rewind to 0!\n");
                       request->write = 0;
                   }
                }

                //sie_leave_mutex(info);

            }//end of while

            if(1 == request->ring_buffer_mode)
            {
                sie_leave_ring_mutex(info);
            }

            //read pointer != write, ringbuf still has sections not parsed, wake up sie to run
            if(request->read != request->write)
            {
                dmx_io_control(info->dmx, DMX_WAKEUP_SIAE, 1);
            }
        }
#endif
    }

    if(!treat_return(ret, buffer, info, request))
    {
        ret = sie_freed;
    }

    return ret;
}

static UINT32 inspect_active(struct sie_dmx_info *info)
{
    UINT32 cnt                  = 0;
    UINT32 ret                  = 0;
    UINT32 value                = 0;
    UINT32 tick                 = 0;
    UINT32 interval             = OSAL_WAIT_FOREVER_TIME;
    UINT8  dispatch_flag        = 0;
    UINT8  i                    = 0;
    UINT8  dispatch_reason      = 0;
    sie_status_t status         = sie_invalid;
    struct sie_request *request = NULL;
    struct dmx_poll poll;

    if(NULL == info)
    {
        return interval;
    }

    poll.timeout = 0;
    cnt = 0;
    DBG_PRINTF("\ninspect_active():mask=0x%x,tick=%d!\n",info->filter,osal_get_tick());
    request = (struct sie_request *)info->addr;

    for(i = 0; i < info->request_nr; i++)
    {
        sie_enter_mutex(info);
        if((SIE_FLT_ACTIVE == request[i].status)&&(SIE_STATUS_OPENED == info->status))
        {
            dispatch_flag = 1;
            poll.mask = 1<<request[i].dmx_filter;
            value = dmx_io_control(info->dmx, IO_ASYNC_POLL, (UINT32)&poll);
            cnt++;
            //timeout
            if(request[i].timeout <= osal_get_tick())
            {
                dispatch_reason = SIE_REASON_FILTER_TIMEOUT;
            }
            /*section hit.for ring buf mode, can not poll the section hit status,
            must rely on read and write pointer.            */
            else if((poll.mask&value)||(/*request[i].ring_buffer_mode==1&&*/(request[i].read!=request[i].write)))
            {
                dispatch_reason = SIE_REASON_FILTER_HIT;
            }
            //filter not hit, and not timeout
            else
            {
                if ((request[i].timeout!=OSAL_WAIT_FOREVER_TIME)&&(request[i].timeout<interval))
                {
                    interval = request[i].timeout;
                }

                dispatch_flag = 0;
            }

            //filter hit or time out, dispatch
            if(1 == dispatch_flag)
            {
                //sie_leave_mutex(info);
                status = dispatch_section(info, &request[i], poll.mask, dispatch_reason);
                //sie_enter_mutex(info);
                if ((sie_started == status)&&(SIE_FLT_ACTIVE == request[i].status))
                {
                    //find the minimum timeout(it's time point and not interval)
                    if ((request[i].timeout!=OSAL_WAIT_FOREVER_TIME)&&(request[i].timeout<interval))
                    {
                        interval = request[i].timeout;
                    }
                }
            }

        }
        sie_leave_mutex(info);
    }
    DBG_PRINTF("    poll fiter cnt=%d!\n", cnt);

    if(OSAL_WAIT_FOREVER_TIME == interval)
    {
        return interval;
    }

    tick = osal_get_tick();
    ret = (interval>tick)? interval-tick : 0;
    DBG_PRINTF("tick=%d, of all active filters min timeout=%d,next poll interval=%d!\n", tick, interval,ret);
    return ret;
}

static void sie_main(struct sie_dmx_info *info)
{
    UINT8  i      = 0;
//    UINT32 filter = 0xFFFFFFFF;
    UINT32 flag   = 0xFFFFFFFF;
    struct sie_request *request = NULL;
    struct dmx_poll poll;

    if(NULL == info)
    {
        return ;
    }

    poll.mask = 0xFFFFFFE0;
    poll.timeout = OSAL_WAIT_FOREVER_TIME;

    while(1)
    {
        //task will suspend here until request hit, timeout, or DMX_WAKEUP_SIAE set.
        dmx_io_control(info->dmx, IO_ASYNC_POLL, (UINT32)(&poll));
        /* clear up the trigger, since the task is waked up. */
        dmx_io_control(info->dmx, DMX_WAKEUP_SIAE, 0);

        if (OSAL_E_OK == osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_ENGINE_CLOSING,OSAL_TWF_ANDW, 0))
        {
            sie_enter_mutex(info);
            request = (struct sie_request *)info->addr;
            for(i = 0;i < info->request_nr; i++)
            {
                if(SIE_FLT_ACTIVE == request[i].status)
                {
                    dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request[i].dmx_filter);
                }
            }
            info->filter = 0;
            sie_leave_mutex(info);
            break;
        }
        //remain time
        poll.timeout = inspect_active(info);
        //get newest active filter bitmap, because new filter maybe enabled during inspect_active.
        poll.mask = info->filter;
        osal_task_sleep(1);
    }
    osal_flag_clear(info->sie_flag, SIE_FLAG_ENGINE_CLOSING);
    osal_flag_set(info->sie_flag, SIE_FLAG_ENGINE_CLOSED);
}

static void sie_task(UINT32 param1, __MAYBE_UNUSED__ UINT32 param2)
{
    INT32  ret_wait = 0;
    UINT32 flag     = 0xFFFFFFFF;

    param2 = 0;

    while(1)
    {
        //wait main task to enable the task
        //osal_flag_wait(&flag, sie_info[param1].sie_flag,SIE_FLAG_ENGINE_ENABLE,OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        ret_wait = osal_flag_wait(&flag, sie_info[param1].sie_flag,
                SIE_FLAG_ENGINE_ENABLE, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);

        if(E_FAILURE == ret_wait)
        {
            PRINTF("osal_flag_wait() failed!\n");
        }

        //clear this flag
        osal_flag_clear(sie_info[param1].sie_flag, SIE_FLAG_ENGINE_ENABLE);

        //enter main function
        sie_main(&sie_info[param1]);
    }
}

static void init_requests(void *addr, void *buffer, INT32 nr)
{
    INT32 i = 0;
    struct sie_request *req = NULL;

    if((NULL == addr) || (NULL == buffer))
    {
        return ;
    }

    req = (struct sie_request *)addr;

    for(i=0; i < nr; i++)
    {
        req->status = SIE_FLT_FREE;
        if (NULL != buffer)
        {
            req->buffer = buffer+SI_SHORT_SECTION_LENGTH*i;
        }
        ++req;
    }
}

static ID_THREAD sie_create_task(INT32 dmx_idx)
{
    OSAL_T_CTSK task;
    ID_THREAD task_id = OSAL_INVALID_ID;

    MEMSET(&task, 0, sizeof(OSAL_T_CTSK));
    if (OSAL_INVALID_ID == sie_info[dmx_idx].sie_flag)
    {
        sie_info[dmx_idx].sie_flag = osal_flag_create(SIE_FLAG_MUTEX|SIE_FLAG_RING_MUTEX);
        if (OSAL_INVALID_ID == sie_info[dmx_idx].sie_flag)
        {
            SIE_PRINTF("%s: flag create failure!\n", __FUNCTION__);
            return OSAL_INVALID_ID;
        }
    }

    sie_info[dmx_idx].mutex_task = OSAL_INVALID_ID;
    sie_info[dmx_idx].mutex_cnt  = 0;
#if SIE_DEBUG_LEVEL>0
    task.name[0] = 'S';
    task.name[1] = 'I';
    task.name[2] = 'E';
#endif
    task.itskpri = SIE_MODULE_TASK_PRIORITY;
    task.task    = sie_task;
    task.quantum = SIE_MODULE_TASK_QUANTUM;
    task.stksz   = SIE_MODULE_TASK_STACKSIZE;
    task.para1   = dmx_idx;

    task_id = osal_task_create(&task);
    if (OSAL_INVALID_ID == task_id)
    {
        osal_flag_delete(sie_info[dmx_idx].sie_flag);
        sie_info[dmx_idx].sie_flag = OSAL_INVALID_ID;
        SIE_PRINTF("%s: task create failure!\n", __FUNCTION__);
        return OSAL_INVALID_ID;
    }

    sie_info[dmx_idx].task_id = task_id;
    sie_info[dmx_idx].status = SIE_STATUS_INITED;

    return task_id;
}

INT32 sie_open_dmx(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen)
{
    INT32  i = -1;
    UINT32 flag = 0xFFFFFFFF;
    ID_THREAD task_id = OSAL_INVALID_ID;

    if ((NULL == dmx)||(max_filter_nr <= 0)||(buflen < 0))
    {
        SIE_PRINTF("%s: parameter invalid!\n", __FUNCTION__);
        return SI_EINVAL;
    }
    if ((NULL != buffer)&&(buflen<max_filter_nr*SI_SHORT_SECTION_LENGTH))
    {
        SIE_PRINTF("%s: buffer length not enough!\n", __FUNCTION__);
        return SI_EINVAL;
    }

    //get index of dmx
    i = (dmx->type)&HLD_DEV_ID_MASK;
    ASSERT(i < MAX_DMX_NUM);
    if (i >= MAX_DMX_NUM)
    {
        return SI_EINVAL;
    }

    if(E_FAILURE == osal_flag_wait(&flag, sie_module_flag, SIE_FLAG_MUTEX,
            OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME) )
    {
        PRINTF("osal_flag_wait() failed!\n");
    }
    if(SIE_STATUS_RELEASED == sie_info[i].status)
    {
        //create SIE task
        task_id = sie_create_task(i);
        sie_info[i].tmp_buffer = MALLOC(SI_LONG_SECTION_LENGTH);
        if(NULL == sie_info[i].tmp_buffer || task_id == OSAL_INVALID_ID)
        {
            osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
            return SI_EINVAL;
        }
    }

    if(SIE_STATUS_INITED == sie_info[i].status)
    {
        sie_enter_mutex(&sie_info[i]);
        sie_info[i].dmx = dmx;
        sie_info[i].request_nr = max_filter_nr;
        sie_info[i].addr = MALLOC(max_filter_nr*sizeof(struct sie_request));
        if (NULL == sie_info[i].addr)
        {
            sie_leave_mutex(&sie_info[i]);
            SIE_PRINTF("%s: memory alloc failure!\n", __FUNCTION__);
            osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
            return SI_EINVAL;
        }

        MEMSET(sie_info[i].addr, 0, max_filter_nr*sizeof(struct sie_request));
        init_requests(sie_info[i].addr, buffer, max_filter_nr);
        sie_info[i].status = SIE_STATUS_OPENED;
        sie_leave_mutex(&sie_info[i]);
        //enable SIE task
        osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_ENABLE);
        osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
        return SI_SUCCESS;
    }
    else
    {
        SIE_PRINTF("%s: creating SIE task failed!\n", __FUNCTION__);
        osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
        return SI_EINVAL;
    }
}

INT32 sie_close_dmx(struct dmx_device *dmx)
{
    UINT32 flag     = 0;
    INT32  i        = -1;
    INT32  ret_wait = -1;

    if (NULL == dmx)
    {
        SIE_PRINTF("%s: dmx is NULL!\n", __FUNCTION__);
        return SI_EINVAL;
    }

    i = (dmx->type)&HLD_DEV_ID_MASK;
    ASSERT(i < MAX_DMX_NUM);
    if (i >= MAX_DMX_NUM)
    {
        return SI_EINVAL;
    }

    sie_enter_mutex(&sie_info[i]);
    if (SIE_STATUS_OPENED == sie_info[i].status)
    {
        sie_info[i].status = SIE_STATUS_INITED; //set status to closed mode
        sie_leave_mutex(&sie_info[i]);
        //inform SIE task to close all filters on the DMX and then release SIE task
        osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSING);
        dmx_io_control(sie_info[i].dmx, DMX_WAKEUP_SIAE, 1);
        //wait until all filters are closed
        //osal_flag_wait(&flag, sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED,OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME);

        ret_wait = osal_flag_wait(&flag, sie_info[i].sie_flag,
            SIE_FLAG_ENGINE_CLOSED,OSAL_TWF_ANDW,OSAL_WAIT_FOREVER_TIME);
        if(E_FAILURE == ret_wait)
        {
            PRINTF("osal_flag_wait() failed!\n");
        }
        //free resource
        if (sie_info[i].addr)
        {
            FREE(sie_info[i].addr);
            sie_info[i].addr = NULL;
        }

        osal_flag_clear(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED);
        return SI_SUCCESS;
    }
    else
    {
        sie_leave_mutex(&sie_info[i]);
        SIE_PRINTF("%s: module not in SIE_STAT_OPENED stat!\n", __FUNCTION__);
        return SI_EINVAL;
    }
}

#ifdef AUTO_OTA
INT32 si_create_otasrch_flag()
{
    if(m_otasrch_flgid == OSAL_INVALID_ID)
    {
        m_otasrch_flgid = osal_flag_create(0);
        if (OSAL_INVALID_ID == m_otasrch_flgid)
        {
//          ASSERT(0);
            return ERR_FAILUE;
        }
    }

    return SUCCESS;
}

INT32 si_delete_otasrch_flag()
{
    INT32 ret = SUCCESS;

    if(m_otasrch_flgid != OSAL_INVALID_ID)
    {
        ret = osal_flag_delete(m_otasrch_flgid);
        m_otasrch_flgid = OSAL_INVALID_ID;
    }

    return ret;
}

void clear_stop_otasrch_cmd()
{
    osal_flag_clear(m_otasrch_flgid, OTASRCH_USER_ABORT);
}

void set_stop_otasrch_ack()
{
    osal_flag_set(m_otasrch_flgid, OTASRCH_USER_ABORT_ACK);
}

INT32 check_stop_otasrch_cmd()
{
    UINT32 flag = 0;
    INT32  flg_wait_ret = OSAL_E_OK;

    flg_wait_ret = osal_flag_wait(&flag, m_otasrch_flgid, OTASRCH_USER_ABORT, OSAL_TWF_ORW, 0);
    return flg_wait_ret;
}

void set_otasrch_end()
{
    osal_flag_set(m_otasrch_flgid, OTASRCH_END);
}

void set_stop_otasrch_cmd()
{
    osal_flag_set(m_otasrch_flgid, OTASRCH_USER_ABORT);
}

INT32 wait_stop_otasrch_ack()
{
    UINT32 flag = 0;

    if(SUCCESS == osal_flag_wait(&flag, m_otasrch_flgid,
        OTASRCH_USER_ABORT_ACK|OTASRCH_END, OSAL_TWF_ORW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME))
    {
        if(SUCCESS == osal_flag_wait(&flag, m_otasrch_flgid,OTASRCH_USER_ABORT, OSAL_TWF_ORW|OSAL_TWF_CLR, 0))
        {
        }

        return SUCCESS;
    }
    return ERR_FAILUE;
}

INT32 si_stop_channel_otasrch(BOOL b_blocking, INT32 engine_id)
{
    set_stop_otasrch_cmd();

    if(b_blocking)
    {
        wait_stop_otasrch_ack();
    }

    return SUCCESS;
}
#endif

INT32 sie_module_init(void)
{
    INT32 i = 0;

    if (OSAL_INVALID_ID == sie_module_flag)
    {
        sie_module_flag = osal_flag_create(SIE_FLAG_MUTEX);
        if (OSAL_INVALID_ID == sie_module_flag)
        {
            SIE_PRINTF("%s: flag create failure!\n", __FUNCTION__);
            return SI_SBREAK;
        }
    }

    for(i=0; i<MAX_DMX_NUM; i++)
    {
        sie_info[i].mutex_task     = OSAL_INVALID_ID;
        sie_info[i].sie_ring_mutex = OSAL_INVALID_ID;
        sie_info[i].mutex_cnt      = 0;
        sie_info[i].sie_flag       = OSAL_INVALID_ID;
        sie_info[i].status         = SIE_STATUS_RELEASED;
    }

#ifdef AUTO_OTA
    si_create_otasrch_flag();
#endif

    return SI_SUCCESS;
}

static void on_dmx_event(struct get_section_param *gsp)
{
#ifndef SIE_MINIMUM_MODEL
    BOOL ring_buffer_full      = FALSE;
    struct sect_buffer *buffer = NULL;
#endif
    INT32  discard_length  = -1;
    INT32  dmx_idx         = 0xFFFFFFFF;
    UINT32 hit_reason      = 0;
    UINT32 read_addr       = 0;
    UINT32 write_addr      = 0;
    INT32  next_sec_length = 0;
    struct sie_request *request = NULL;

    if(NULL == gsp)
    {
        return ;
    }

    request = list_entry(gsp, struct sie_request, gsp);  /* get the request by it's memeber gsp pointer */

    /*for non-async request status not started, it will be just ignored */
    dmx_idx = request->filter.dmx->type & HLD_DEV_ID_MASK;

    if ((SIE_FLT_ACTIVE != request->status) && (SIE_FLT_RUNNING != request->status))
    {
        gsp->continue_get_sec = 0;     /* this filter is no longer active. */
        SIE_PRINTF("%s: entering a non-active filter!\n", __FUNCTION__);
        return;
    }

    /* check CRC for those needed. */
    hit_reason = gsp->sec_hit_num;
    if (request->fparam.attr[hit_reason]&SI_ATTR_HAVE_CRC32)
    {
        if (mg_fcs_decoder(gsp->buff, gsp->sec_tbl_len) != 0)
        {
            gsp->continue_get_sec = 1;
            SIE_PRINTF("%s: failed to pass CRC32!\n",__FUNCTION__);
            return;
        }
    }
    /*
     *1.call the filter's on_event if there is one,
     *  it will return the recording or non-recording selection,
     *  otherwise, regard it as a recording reply.
     *
     *2.for filter without pipeline buffer, a recording reply
     *  will convert the filter into idle status and trigger the callback.
     *  for filter with pipeline buffer, a record reply will be put into
     *  the pipeline buffer and wakeup the SI egine to examine the data.
     *  for pipeline buffer that are space-full, it is regarded as a
     *  conventional filter.
     */
    DBG_PRINTF("\n****%s(): get section pid[0x%x]!tick=%d!\n", __FUNCTION__,gsp->pid,osal_get_tick());
    if (NULL == request->fparam.section_event)
    {
        gsp->continue_get_sec = 0;
    }
    else
    {
        if (FALSE == request->fparam.section_event(gsp->pid,&request->filter,hit_reason, gsp->buff,gsp->sec_tbl_len))
        {
            return;
        }
    }

#ifndef SIE_MINIMUM_MODEL
    if (request->buf_len>(request->reserved_len+request->max_sec_length+sizeof(struct sect_buffer)))
    {
        /* figure out the current section buffer */
        buffer = (struct sect_buffer *)(request->buffer+request->write);
        buffer->hit_reason = (UINT8)hit_reason;
        buffer->section_length = gsp->sec_tbl_len;

        if (buffer->section_length > SI_LONG_SECTION_LENGTH)
        {
#if (SIE_DEBUG_LEVEL > 0)
            SIE_PRINTF("%s: sec len %d > max length %d\n",__FUNCTION__,buffer->section_length,SI_LONG_SECTION_LENGTH);
            ASSERT(0);
#endif
            return;
        }

        //copy section from dmx working buff, check buffer boundary
        MEMCPY((UINT8 *)(request->buffer+request->write+sizeof(struct sect_buffer)), gsp->buff, buffer->section_length);

        /* 4 byte aligned for the next sect_buffer */
        request->write += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);
        DBG_PRINTF("    ~~~~ring buffer current write=0x%x!\n", request->write);

        /* rewind control */
        if ((request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len)
        {
            if (0 < request->read)
            {
                request->write = 0;
                RINGBUF_PRINTF("    ~~~~ring buffer write rewind to 0!\n");
            }
            else
            {
                //ring buffer full
                //1. stop dmx, 2. let dispatch process read, 3. need rewind at dispatch_section()
                ring_buffer_full = TRUE;
                RINGBUF_PRINTF("    ~~~~read = 0 when write rewind!!!\n");
            }
        }

        next_sec_length = (request->write+sizeof(struct sect_buffer)+request->max_sec_length);
        if (ring_buffer_full || ((request->write<request->read) && ((UINT32)next_sec_length >= request->read)))
        {
          /*no space for the next section arriving,so this filter will be closed and wait for read pointer to proceed!*/
            if(0 == request->ring_buffer_mode)
            {
                RINGBUF_PRINTF("    ~~~~ring buffer full,stop filter[%d] for pid[0x%x]!\n",
                    request->dmx_filter,request->gsp.pid);
                buffer->hit_reason |= SIE_REASON_RINGBUF_FULL;
                gsp->continue_get_sec = 0;
                dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
            }
            else
            {
                RINGBUF_PRINTF("    ~~~~buf full\n");
                dmx_idx = request->filter.dmx->type&HLD_DEV_ID_MASK;

                sie_enter_ring_mutex(&sie_info[dmx_idx]);

                while (discard_length < request->max_sec_length)
                {
                    buffer = (struct sect_buffer *)(request->buffer+request->read);
                    RINGBUF_PRINTF("    ~~~~ring buffer current read=0x%x!\n", request->read);

                    //need protect this line of code
                    request->read += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);
                    discard_length += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);

                    RINGBUF_PRINTF("    ~~~~ring buffer read move to 0x%x!\n", request->read);
                    read_addr=(request->read+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len);
                    if (read_addr > request->buf_len)
                    {
                        request->read = 0;
                        RINGBUF_PRINTF("    ~~~~ring buffer read rewind to 0!\n");
                    }
                }

                write_addr=(request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len);
                if (write_addr > request->buf_len)
                {
                    //request->read shall not be zero!
                    request->write = 0;
                    RINGBUF_PRINTF("    ~~~~ring buffer write rewind to 0!\n");
                }
                sie_leave_ring_mutex(&sie_info[dmx_idx]);

                dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
             }
        }
        else
        {
//          buffer = (struct sect_buffer *)(request->buffer+request->write);
            dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
        }
    }
    else
#endif
    {
        sie_enter_mutex(&sie_info[dmx_idx]);
        request->write = gsp->sec_tbl_len;
        gsp->continue_get_sec = 0;
        sie_leave_mutex(&sie_info[dmx_idx]);
    }
}

static struct si_filter_t *__alloc_filter(struct sie_dmx_info *info, UINT16 pid, UINT8 *buffer, UINT32 length,
    UINT16 max_sec_length, UINT8 mode)
{
    struct sie_request *req = NULL;
    struct sie_request *request = NULL;
    UINT8 *addr = NULL;
    UINT8 i     = 0;
    UINT8 flag  = 0;

    if(NULL == info)
    {
        return NULL;
    }

    req = (struct sie_request *)info->addr;

    //check PID of request, no duplicate PID is allowed.
    for(i = 0;i < info->request_nr; i++)
    {
        if((SIE_FLT_FREE == req[i].status) && (0 == flag))
        {
            request = &req[i];
            MEMSET(&req[i].gsp, 0, sizeof(struct get_section_param));
            flag = 1;
        }
#ifndef DUPLICATE_PID_SUPPORT
        if(((SIE_FLT_ACTIVE==req[i].status)||(SIE_FLT_RUNNING == req[i].status)) && (req[i].gsp.pid==pid))
        {
            SIE_PRINTF("%s(): same pid 0x%x already active!\n", __FUNCTION__, pid);
            return NULL;
        }
#endif
    }

    if (NULL == request)
    {
        SIE_PRINTF("%s(): no free request left!\n", __FUNCTION__);
        return NULL;
    }

    if (NULL != buffer)
    {
        MEMSET(request, 0, sizeof(struct sie_request));
        request->buffer = buffer;
        request->reserved_len = max_sec_length;
#ifndef SIE_MINIMUM_MODEL
        //request buf long enough, use ring buf mode
        if(length > (request->reserved_len+max_sec_length+sizeof(struct sect_buffer)))
        {
            addr = buffer+length-request->reserved_len;
        }
        else
#endif
        {
            addr = buffer;
        }
    }
    else    //in search mode, each request allocated 1024 byte buffer from vbv
    {
        if(NULL == request->buffer)
        {
            SIE_PRINTF("%s(): request buf NULL!\n", __FUNCTION__);
            return NULL;
        }
        addr = request->buffer;
        MEMSET(request, 0, sizeof(struct sie_request));
        request->buffer = addr;
        request->reserved_len = max_sec_length;
    }

    request->max_sec_length       = max_sec_length;
    request->status               = SIE_FLT_IDLE;
    request->buf_len              = length;
    request->read                 = 0;
    request->write                = 0;
    request->filter.dmx           = info->dmx;
    request->ring_buffer_mode     = mode;
    struct get_section_param *gsp = &request->gsp;

    gsp->pid              = pid;
    gsp->buff             = addr;
    gsp->buff_len         = request->max_sec_length;
    gsp->continue_get_sec = 1;
    gsp->mask_value       = &request->fparam.mask_value;
    gsp->get_sec_cb       = on_dmx_event;

    return &request->filter;
}

static BOOL filter_sanity_check(struct si_filter_t *filter, UINT8 check_status)
{
    struct sie_request *request = NULL;

    if(NULL == filter)
    {
        return FALSE;
    }

    request = list_entry(filter, struct sie_request, filter);

    if(request->status & check_status)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static INT32 __config_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
    struct sie_request *request = NULL;

    if(NULL == filter)
    {
        return SI_EINVAL;
    }

    if(FALSE == filter_sanity_check(filter, SIE_FLT_IDLE|SIE_FLT_ACTIVE))
    {
        SIE_PRINTF("%s: filter not in active/idle status!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    request = list_entry(filter, struct sie_request, filter);
    MEMCPY(&request->fparam, fparam, sizeof(struct si_filter_param));

    return SI_SUCCESS;
}

static INT32 __copy_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
    struct sie_request *request = NULL;

    if(NULL == filter)
    {
        return SI_EINVAL;
    }

    if(FALSE == filter_sanity_check(filter, SIE_FLT_IDLE|SIE_FLT_ACTIVE))
    {
        SIE_PRINTF("%s: filter not in active/idle status!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    request = list_entry(filter, struct sie_request, filter);
    MEMCPY(fparam, &request->fparam, sizeof(struct si_filter_param));

    return SI_SUCCESS;
}

static INT32 __enable_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
    struct sie_request *request = NULL;
    struct dmx_device *dmx = NULL;
    UINT32 dmx_idx = 0xFFFFFFFF;

    if(FALSE == filter_sanity_check(filter, SIE_FLT_IDLE))
    {
        SIE_PRINTF("%s(): filter for pid 0x%x not in idle status!\n",__FUNCTION__,request->gsp.pid);
        return SI_EINVAL;
    }

    request = list_entry(filter, struct sie_request, filter);
    dmx = filter->dmx;
    dmx_idx = (dmx->type)&HLD_DEV_ID_MASK;

    if (request->fparam.timeout != OSAL_WAIT_FOREVER_TIME)
    {
    #ifdef TFN_SCAN
        request->timeout = request->fparam.timeout+osal_get_tick()+4000;
    #else
        request->timeout = request->fparam.timeout+osal_get_tick();
    #endif
    }
    else
    {
        request->timeout = OSAL_WAIT_FOREVER_TIME;
    }

    request->gsp.wai_flg_dly = OSAL_WAIT_FOREVER_TIME;
    request->status = SIE_FLT_ACTIVE;

    if(RET_SUCCESS != dmx_async_req_section(dmx, &request->gsp, &request->dmx_filter))
    {
        SIE_PRINTF("%s(): req dmx filter for pid 0x%x failed!\n",__FUNCTION__,request->gsp.pid);
        request->status = SIE_FLT_FREE;
        MEMSET(&request->gsp, 0, sizeof(struct get_section_param));

        return SI_EINVAL;
    }

    sie_info[dmx_idx].filter |= 1<<request->dmx_filter;
    SIE_PRINTF("\n%s(): filter[%d] for pid[0x%x], timeout=%d, tick=%d!\n",__FUNCTION__, request->dmx_filter,
        request->gsp.pid, request->timeout,osal_get_tick());
    dmx_io_control(dmx, DMX_WAKEUP_SIAE, 1);

    return SI_SUCCESS;
}


#ifndef SIE_MINIMUM_MODEL
static INT32 __get_pid(struct si_filter_t *filter, struct si_filter_param *param)
{
    struct sie_request *request = NULL;

    if (filter != NULL)
    {
        request = list_entry(filter, struct sie_request, filter);
        return request->gsp.pid;
    }
    else
    {
        return 0x1FFF;
    }
}
#endif

static INT32 filter_operation(si_filter_op_t operation, struct si_filter_t *filter,struct si_filter_param *fparam)
{
    INT32  ret      = 0;
    INT32  ret_wait = 0;
    UINT32 flag     = 0;
    UINT32 dmx_idx  = 0;

    dmx_idx = (filter->dmx->type)&HLD_DEV_ID_MASK;
    if (SIE_STATUS_OPENED != sie_info[dmx_idx].status)
    {
        SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
        return SI_EINVAL;
    }
    sie_enter_mutex(&sie_info[dmx_idx]);
    ret_wait = osal_flag_wait(&flag, sie_info[dmx_idx].sie_flag,
        SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING,OSAL_TWF_ORW, 0);
    if (ret_wait != OSAL_E_OK)
    {
        ret = operation(filter, fparam);
    }
    else
    {
        ret = SI_UBREAK;
    }

    sie_leave_mutex(&sie_info[dmx_idx]);
    return ret;
}

struct si_filter_t *sie_alloc_filter_ext(struct dmx_device *dmx, UINT16 pid, UINT8 *buffer, UINT32 length,
    UINT16 max_sec_length, UINT8 mode)
{
    UINT32 flag     = 0xFFFFFFFF;
    INT32  i        = -1;
    INT32  ret_wait = -1;
    struct si_filter_t *ret = NULL;

    ASSERT(dmx != NULL);
    if(dmx != NULL)
    {
        i = (dmx->type)&HLD_DEV_ID_MASK;
    }

    ASSERT(i < MAX_DMX_NUM);
    if ((i < 0) || (i >= MAX_DMX_NUM))
    {
        return NULL;
    }

    if (SIE_STATUS_OPENED != sie_info[i].status)
    {
        SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
        return NULL;
    }
    else if (length < max_sec_length)
    {
        SIE_PRINTF("%s: insufficent buffer length %x!\n", __FUNCTION__,length);
        return NULL;
    }

    sie_enter_mutex(&sie_info[i]);
    ret_wait = osal_flag_wait(&flag, sie_info[i].sie_flag,
        SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING,OSAL_TWF_ORW,0);
    if (ret_wait != OSAL_E_OK)
    {
        ret = __alloc_filter(&sie_info[i], pid, buffer, length, max_sec_length, mode);
    }
    else
    {
        ret = NULL;
    }

    sie_leave_mutex(&sie_info[i]);

    return ret;
}

INT32 sie_enable_filter(struct si_filter_t *filter)
{
    return filter_operation(__enable_filter, filter, NULL);
}

static INT32 _sie_abort(INT32 dmx_idx, UINT8 **buff, UINT16 pid, struct restrict *mask_value)
{
    UINT8 i = 0;
    struct sie_request *request = NULL;

    sie_enter_mutex(&sie_info[dmx_idx]);
    request = (struct sie_request *)sie_info[dmx_idx].addr;

    for(i = 0;i < sie_info[dmx_idx].request_nr; i++)
    {
        if((SIE_FLT_ACTIVE == request[i].status) || (SIE_FLT_RUNNING == request[i].status))
        {
            if((pid == request[i].gsp.pid)&&((NULL == mask_value)||
                (0==MEMCMP(mask_value, &(request[i].fparam.mask_value), sizeof(struct restrict)))))
            {
                dmx_io_control(sie_info[dmx_idx].dmx, IO_ASYNC_CLOSE, request[i].dmx_filter);
                sie_info[dmx_idx].filter &= (~(1<<request[i].dmx_filter));
                SIE_PRINTF("%s: flt[%d] for pid[0x%x] closed!\n",__FUNCTION__,request[i].dmx_filter,request[i].gsp.pid);
                SIE_PRINTF("%s: r %d, w %d\n",__FUNCTION__,request[i].read,request[i].write);

                request[i].status = SIE_FLT_FREE;

                if(buff)
                {
                    *buff = request[i].buffer;
                }
                break;
            }
        }
    }

    sie_leave_mutex(&sie_info[dmx_idx]);
    return SI_SUCCESS;
}

INT32 sie_abort_ext(struct dmx_device *dmx, UINT8 **buff, UINT16 pid, struct restrict *mask_value)
{
    INT32 dmx_idx = 0xFFFFFFFF;

    if (NULL == dmx)
    {
        SIE_PRINTF("%s: dmx is NULL!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    dmx_idx = (dmx->type)&HLD_DEV_ID_MASK;
    ASSERT(dmx_idx < MAX_DMX_NUM);
    if (dmx_idx >= MAX_DMX_NUM)
    {
        return SI_EINVAL;
    }

    if (SIE_STATUS_OPENED != sie_info[dmx_idx].status)
    {
        SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    return _sie_abort(dmx_idx, buff, pid, mask_value);
}

INT32 sie_abort_filter(struct si_filter_t *filter)
{
    INT32 dmx_idx = 0;
    struct dmx_device *dmx = NULL;
    struct sie_request *request = NULL;

    if(NULL == filter)
    {
        return SI_EINVAL;
    }

    dmx = filter->dmx;

    if (NULL == dmx)
    {
        SIE_PRINTF("%s: dmx is NULL!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    dmx_idx = (dmx->type)&HLD_DEV_ID_MASK;
    if(dmx_idx >= MAX_DMX_NUM)
    {
        SIE_PRINTF("%s: dmx_idx is invalid ,the filter maybe be destroyed !\n",__FUNCTION__);
        return SI_EINVAL;
    }

    if (SIE_STATUS_OPENED != sie_info[dmx_idx].status)
    {
        SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
        return SI_EINVAL;
    }

    request = list_entry(filter, struct sie_request, filter);

    sie_enter_mutex(&sie_info[dmx_idx]);

    if(request->status != SIE_FLT_FREE)
    {
        dmx_io_control(dmx, IO_ASYNC_CLOSE, request->dmx_filter);

        sie_info[dmx_idx].filter &= (~(1<<request->dmx_filter));
        request->status = SIE_FLT_FREE;
    }

    SIE_PRINTF("%s: flt[%d] for pid[0x%x] closed!\n",__FUNCTION__,request->dmx_filter,request->gsp.pid);

    MEMSET(&request->gsp, 0, sizeof(struct get_section_param));

    sie_leave_mutex(&sie_info[dmx_idx]);

    return SI_SUCCESS;

}

#ifndef SIE_MINIMUM_MODEL
INT32 sie_query_stat(struct dmx_device *dmx)
{
    INT32 i = 0;

    ASSERT(dmx != NULL);
    if(NULL != dmx)
    {
        i = (dmx->type)&HLD_DEV_ID_MASK;
    }

    return sie_info[i].status;
}

INT32 sie_get_pid(struct si_filter_t *filter)
{
    return filter_operation(__get_pid, filter, NULL);
}
#endif

INT32 sie_config_filter(struct si_filter_t *filter,struct si_filter_param *fparam)
{
    if ((!fparam) || (!fparam->section_parser))
    {
        return SI_EINVAL;
    }
    else
    {
        return filter_operation(__config_filter, filter, fparam);
    }
}

INT32 sie_copy_config(struct si_filter_t *filter,struct si_filter_param *fparam)
{
    if (!fparam)
    {
        return SI_EINVAL;
    }
    else
    {
        return filter_operation(__copy_filter, filter, fparam);
    }
}

INT32 sie_open(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen)
{
    return sie_open_dmx(dmx, max_filter_nr, buffer, buflen);
}

INT32 sie_close(void)
{
    UINT32 flag = 0xFFFFFFFF;
    INT32 i = 0;

    for(i=0; i<MAX_DMX_NUM; i++)
    {
        sie_enter_mutex(&sie_info[i]);
        if (SIE_STATUS_OPENED == sie_info[i].status)
        {
            sie_info[i].status = SIE_STATUS_INITED; //set status to closed mode
            //inform SIE task to close all filters on the DMX and then release SIE task
            osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSING);
            dmx_io_control(sie_info[i].dmx, DMX_WAKEUP_SIAE, 1);
            osal_cache_flush(&sie_info[i],sizeof(sie_info[i]));
            sie_leave_mutex(&sie_info[i]);

            //wait until all filters are closed
            osal_flag_wait(&flag, sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED,OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME);
            //free resource
            if (sie_info[i].addr)
            {
                FREE(sie_info[i].addr);
                sie_info[i].addr = NULL;
            }

            osal_flag_clear(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED);
        }
        else
        {
            sie_leave_mutex(&sie_info[i]);
        }
    }

    return SI_SUCCESS;
}

struct si_filter_t *sie_alloc_filter(UINT16 pid, UINT8 *buffer, UINT32 length,UINT16 max_sec_length)
{
    struct si_filter_t *ret = NULL;
    UINT32 flag = 0xFFFFFFFF;
    INT32 i = 0;

    for(i=0; i<MAX_DMX_NUM; i++)
    {
        if(SIE_STATUS_OPENED == sie_info[i].status)
        {
            break;
        }
    }

    if(MAX_DMX_NUM == i)
    {
        SIE_PRINTF("%s: dmx NULL\n",__FUNCTION__);
        return NULL;
    }
    else if (length<max_sec_length)
    {
        SIE_PRINTF("%s: insufficent buffer length %x!\n", __FUNCTION__,length);
        return NULL;
    }

    sie_enter_mutex(&sie_info[i]);
    if (osal_flag_wait(&flag, sie_info[i].sie_flag,
        SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING, OSAL_TWF_ORW, 0)!=OSAL_E_OK)
    {
        ret = __alloc_filter(&sie_info[i], pid, buffer, length, max_sec_length, 0);
    }
    else
    {
        SIE_PRINTF("%s: sie closed!\n",__FUNCTION__);
        ret = NULL;
    }

    sie_leave_mutex(&sie_info[i]);

    return ret;
}

INT32 sie_abort(UINT16 pid, struct restrict *mask_value)
{
    INT32 dmx_idx = 0xFFFFFFFF;

    for(dmx_idx=0; dmx_idx < MAX_DMX_NUM; dmx_idx++)
    {
        if(SIE_STATUS_OPENED == sie_info[dmx_idx].status)
        {
            break;
        }
    }

    if(MAX_DMX_NUM == dmx_idx)
    {
        return SI_EINVAL;
    }

    return _sie_abort(dmx_idx, NULL, pid, mask_value);
}

#if (SIE_DEBUG_LEVEL > 1)
static void sie_set_continue(struct si_filter_t *filter, UINT8 flag)
{
    struct sie_request *request = NULL;

    if(NULL == filter)
    {
        return ;
    }

    request = list_entry(filter, struct sie_request, filter);
    request->gsp.continue_get_sec = flag;
}
#endif

