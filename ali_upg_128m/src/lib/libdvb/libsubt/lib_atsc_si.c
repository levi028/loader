/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_atsc_si.c
*
*    Description: The file is to init the buffer of ATSC SUBTITLE and send
*    the section to ATSC SUBTITLE decoder.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <api/libc/list.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libsi/si_module.h>
#include <api/libsi/sie_monitor.h>
#include <api/libsubt/lib_subt.h>
#include "lib_subt_internal.h"
#include <api/libsubt/subt_osd.h>

#define SUBT_ATSC_SECTION_SIZE    (4096+2048)
//#define SUBT_ATSC_PRINTF    libc_printf
#define SUBT_ATSC_PRINTF(...)

#ifndef DUAL_ENABLE

static UINT8 lib_subt_atsc_section_buffer[SUBT_ATSC_SECTION_SIZE];
static UINT16 lib_subt_atac_pid=0x1fff;
static UINT16 lib_subt_atsc_dmx_filter = 0xffff;
static struct si_filter_t *sfilter_ecm = NULL;
static struct si_filter_param fparam_ecm;

#endif
static UINT8 *g_sec_buf = NULL; // cpu use

typedef unsigned char uint8_t;

void lib_subt_atsc_attach(void *p_config)
{
    struct atsc_subt_config_par *par = NULL;

    if(NULL == p_config)
    {
        return;
    }
    par=(struct atsc_subt_config_par*)p_config;

    g_sec_buf=par->sec_buf_addr;
    //libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
    //libc_printf("%x-g_sec_buf(%x), len(%d)\n",
    //(UINT8*)p_config,par->sec_buf_addr,par->sec_buf_len);
    lib_subt_atsc_set_buf(p_config);
}
#ifndef DUAL_ENABLE

static sie_status_t lib_subt_atsc_section_cb(UINT16 pid, struct si_filter_t *filter,
    UINT8 reason,UINT8 *data, INT32 length)
{
    //DWORD dwt1=0;
    //DWORD dwt2=0;
    UINT16 ret_func = 0;
	filter=NULL;
	reason=0;
	
	if(length<0||0xffffffff<=(UINT32)length)
		return sie_invalid;
    if((pid != lib_subt_atac_pid)||(NULL == data))
    {
        return sie_started;
    }

    //SUBT_ATSC_PRINTF("data %x  len =%d\n",data,length);

    #ifdef _LIB_SUBT_REMOTE
    if(NULL==g_sec_buf)
    {
        libc_printf("Oh!Oh! UI should call lib_subt_atsc_attach() first\n");
        return sie_started;
    }
    //MEMCPY((UINT8*)g_sec_buf,data,length);
    lib_subt_copy_data((UINT32)data,length);

    //SUBT_ATSC_PRINTF("g_sec_buf %x  len =%d\n",g_sec_buf,length);
    //dwt1=osal_get_tick();    //milliseconds
    ret_func =lib_subt_atsc_stream_identify(length,g_sec_buf);
    //dwt2=osal_get_tick(); //milliseconds
    //libc_printf("Run as _LIB_SUBT_REMOTE(%d)\n",dwt2-dwt1);
    #else
    //dwt1=osal_get_tick();    //milliseconds
    ret_func =lib_subt_atsc_stream_identify(length,data);
    //dwt2=osal_get_tick();    //milliseconds
    //libc_printf("Run as not _LIB_SUBT_REMOTE(%d)\n",dwt2-dwt1);
    #endif

    return sie_started;
}

static BOOL lib_subt_atsc_dmx_event(UINT16 pid, struct si_filter_t *si_filter,
    UINT8 reason, UINT8 *data, INT32 len)
{
	pid=0;
	si_filter=NULL;
	reason=0;
	data=NULL;
	len=0;
    return TRUE;
}

/*******************************************************************************
    Construct filter params to filter given pid stream type.
    Things need to do:
        Construct filter params;
        allocate filter description structure; config and enable.
 ******************************************************************************/
void lib_subt_atsc_start_filter(UINT16 dmx_id,int pid)
{
    INT32 ret_func = 0;
    INT32 ret = 0;
    struct dmx_device *dmx_dev = NULL;
	if(dmx_id>=0xffff||((pid <0x20) || ATSC_SUBT_INVALID_PID== pid  ))
		return ;
    if (ATSC_SUBT_INVALID_PID==lib_subt_atac_pid)
    {
        lib_subt_atsc_dmx_filter = 0xffff;
        ret = RET_SUCCESS;

        MEMSET(&fparam_ecm, 0, sizeof(struct si_filter_param));
        fparam_ecm.timeout = OSAL_WAIT_FOREVER_TIME;
        fparam_ecm.attr[0]=SI_ATTR_HAVE_HEADER;
        fparam_ecm.mask_value.mask_len = 1;
        fparam_ecm.mask_value.mask[0] = 0xFF;
        fparam_ecm.mask_value.mask[1] = 0x00;
        fparam_ecm.mask_value.mask[2] = 0x00;
        fparam_ecm.mask_value.mask[3] = 0x00;

        fparam_ecm.mask_value.value_num = 1;
        fparam_ecm.mask_value.value[0][0] = 0xC6;
        fparam_ecm.mask_value.value[0][1] = 0x00;
        fparam_ecm.mask_value.value[0][2] = 0x00;
        fparam_ecm.mask_value.value[0][3] = 0x00;

        fparam_ecm.section_event = lib_subt_atsc_dmx_event;
        fparam_ecm.section_parser = (si_handler_t)lib_subt_atsc_section_cb;

        //     si_monitor_register_pmt_cb(BC2_ecm_cb);
        dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
        sfilter_ecm = sie_alloc_filter_ext(dmx_dev,pid, lib_subt_atsc_section_buffer,
         sizeof(lib_subt_atsc_section_buffer),1024,0);//2048

        if(NULL == sfilter_ecm)
        {
           //SIEM_PRINTF("%s(): alloc filter failed!\n",__FUNCTION__);
           return ;
        }

        ret_func = sie_config_filter(sfilter_ecm, &fparam_ecm);
        ret = sie_enable_filter(sfilter_ecm);

        if(SUCCESS == ret)
        {
           lib_subt_atac_pid=pid;
           //lib_subt_atsc_dmx_filter = sie_get_dmx_filter_nubmer(sfilter_ecm,NULL);
        }
        SUBT_ATSC_PRINTF("[%s] sie_enable_filter ret: %d\n",__FUNCTION__,ret);
    }
}

void lib_subt_atsc_stop_filter(UINT16 dmx_id)
{
    UINT16 lib_subt_atac_pid1 = 0;
    struct dmx_device *dmx_dev = NULL;
	if(dmx_id>=0xffff)
		return ;
    if ( ATSC_SUBT_INVALID_PID== lib_subt_atac_pid)
    {
        return;
    }
    lib_subt_atac_pid1 = lib_subt_atac_pid;
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    if(SI_SUCCESS != sie_abort_ext(dmx_dev,NULL,lib_subt_atac_pid1,NULL))
    {
        ASSERT(FALSE);
    }
    lib_subt_atac_pid = 0x1fff;
    return;
}
#endif
/* // no one calls
INT32 lib_subt_atsc_set_cw(struct dmx_device * dev,UINT8 cw_type, UINT32 * cw)
{
    return dmx_cfg_cw(dev,lib_subt_atsc_dmx_filter,cw_type, cw);
}
*/

