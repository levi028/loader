/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_monitor.c
*
* Description:
*     This file implement the process monitor
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <osal/osal_task.h>
#include <api/libc/printf.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libtsi/db_3l.h>
#include <api/libnim/lib_nim.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/decv/vdec_driver.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca_dev.h>
#include <api/libc/string.h>

#include "lib_pub_inner.h"

/*******************************************************
* macro define
********************************************************/
#define MONITOR_PRINTF  PRINTF//libc_printf


#define LIBPUB_MONITOR_INTERVAL     50

#define MONITOR_ENTER_MUTEX()   { \
        osal_flag_wait(&lib_monitor_flgptn,libpub_flag_id, \
            LIBPUB_MONITOR_MUTEX,TWF_ORW|TWF_CLR,OSAL_WAIT_FOREVER_TIME); \
						}
#define MONITOR_LEAVE_MUTEX()   osal_flag_set(libpub_flag_id, LIBPUB_MONITOR_MUTEX)


/*******************************************************
* structure and enum ...
********************************************************/


/*******************************************************
* global and local variable
********************************************************/
static UINT32 monitor_check_interval =  0;
static libpub_monitor_callback monitor_callback = NULL;

static INT32 nim_unlock_cnt = 0;
static INT32 monitor_check_cnt = 0;
static UINT32 vdec_frame_idx = 0;

static UINT32 lib_monitor_flgptn = 0;


/*******************************************************
* internal function
********************************************************/
static void check_nim_status(struct nim_device *nim)
{
    UINT8 lock = 0;
    const UINT16 unlock_cnt = 25;

    if (nim)
    {
        nim->get_lock(nim, &lock);
    }
    if(0 == lock)
    {
        nim_unlock_cnt++;
        if(nim_unlock_cnt==unlock_cnt)
        {
            nim_unlock_cnt = 0;
            if(monitor_callback)
            {
                monitor_callback(LIB_MONITOR_NIM_UNLOCK,(UINT32)nim);
            }
            MONITOR_PRINTF("\nnim device 0x%x unlock, tick=%d\n",nim,osal_get_tick());
        }
    }
    else
    {
        nim_unlock_cnt = 0;
    }
}


static void check_decv_status(struct vdec_device *vdec)
{
    struct vdec_status_info vdec_info;// = {0};
    const UINT16 check_count = 100;

	if(NULL == vdec)
	{
		return;
	}
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    monitor_check_cnt++;
    if(monitor_check_cnt==check_count)
    {
        monitor_check_cnt = 0;
        vdec_io_control(vdec, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if(VDEC27_STARTED == vdec_info.u_cur_status)
        {
            if(vdec_info.display_idx==vdec_frame_idx)
            {
                if(monitor_callback)
                {
                    monitor_callback(LIB_MONITOR_VDEC_HOLD, (UINT32)vdec);
                }
                MONITOR_PRINTF("\nvdec device 0x%x hold, tick=%d\n",vdec,osal_get_tick());
            }
        }
        vdec_frame_idx = vdec_info.display_idx;
    }
}


/*******************************************************
* external API
********************************************************/
void monitor_switch_dev_monitor(UINT32 device_flag, UINT8 onoff)
{
	if(LIBPUB_MONITOR_INVALID_NUM != device_flag)
	{
	    if(0 == onoff)
	    {
	        osal_flag_clear(libpub_flag_id,device_flag);
	    }
	    else if(1 == onoff)
	    {
	        osal_flag_set(libpub_flag_id,device_flag);
	    }
	}
}



void monitor_init(UINT32 interval, libpub_monitor_callback func)
{
    if(interval!=0)
    {
        monitor_check_interval = interval;
    }
    else
    {
        monitor_check_interval = LIBPUB_MONITOR_INTERVAL;
    }
    if(func!=NULL)
    {
        monitor_callback = func;
    }
    if(libpub_flag_id!=OSAL_INVALID_ID)
    {
        MONITOR_LEAVE_MUTEX();
    }
}


void  monitor_rst(void)
{
    MONITOR_ENTER_MUTEX();

    monitor_check_cnt = 0;
    vdec_frame_idx = 0;

    MONITOR_LEAVE_MUTEX();

}


void monitor_switch(UINT32 onoff)
{
    if(0 == onoff)
    {
        MONITOR_ENTER_MUTEX();
        monitor_check_cnt = 0;
        nim_unlock_cnt = 0;
        MONITOR_LEAVE_MUTEX();
    }

    monitor_switch_dev_monitor(LIBPUB_MONITOR_CHECK_NIM, onoff);

    monitor_switch_dev_monitor(LIBPUB_MONITOR_CHECK_VDEC, onoff);
}

void monitor_proc(void)
{
    UINT32 flgptn = 0;
    UINT16 device_id = 0;
    struct nim_device *nim = NULL;
    struct vdec_device *vdec = NULL;

    osal_task_sleep(monitor_check_interval);
    MONITOR_ENTER_MUTEX();

    //check nim device
    if(OSAL_E_OK == osal_flag_wait(&flgptn, libpub_flag_id, LIBPUB_MONITOR_CHECK_NIM, OSAL_TWF_ANDW, 0))
    {
        device_id = 0;
        while (1)
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, device_id);
            if (NULL == nim)
            {
                break;
            }
            check_nim_status(nim);
            device_id++;
        }

    }

    //check vdec device
    if (OSAL_E_OK == osal_flag_wait(&flgptn, libpub_flag_id, LIBPUB_MONITOR_CHECK_VDEC, OSAL_TWF_ANDW, 0))
    {

        device_id = 0;
        while (1)
        {
            vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, device_id);
            if (NULL == vdec)
            {
                break;
            }
            check_decv_status(vdec);
            device_id++;
        }
    }

    //check other device if needed


    MONITOR_LEAVE_MUTEX();

}


/*************************************************************************************
*
*                       Porting API For PUB29
*
*************************************************************************************/
#ifdef PUB_PORTING_PUB29


BOOL dm_get_vdec_running()
{
    struct vdec_status_info cur_status;
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
    if((cur_status.u_cur_status!=VDEC_DECODING))//&&(CurStatus.uFirstPicShowed==TRUE))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


#endif


