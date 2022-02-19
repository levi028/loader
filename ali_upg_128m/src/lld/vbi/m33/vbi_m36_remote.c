/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m36_remote.c

   *    Description:define the remote call function to the VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <hal/hal_common.h>
#ifdef DUAL_ENABLE
#include <modules.h>
#endif
#include "vbi_buffer.h"
#include "vbi_m3327.h"

#ifdef DUAL_ENABLE
    #if 0
    enum LLD_VBI_FUNC
    {
        FUNC_VBI_M33_ATTACH = 0,
        FUNC_VBI_M33_ENABLE_TTX_BY_OSD,
        FUNC_VBI_M33_ENABLE_VBI_TRANSFER,
        FUNC_VBI_M33_GET_INITIAL_PAGE,
        FUNC_VBI_M33_GET_INITIAL_PAGE_STATUS,
        FUNC_VBI_M33_GET_FIRST_TTX_PAGE,
    };
    #endif
enum LLD_VBI_FUNC
{
    FUNC_VBI_M33_SET_OUTPUT = 0,
    FUNC_VBI_M33_SET_VBI_DATA_ADDR,
    FUNC_VBI_M33_SET_RD,
    //FUNC_VBI_M33_MAIN_SET_QUENE_CNT,
    FUNC_VBI_M33_SET_VBI_STATUS,
    FUNC_VBI_M33_SET_VBI_BY_TTX,
    FUNC_SET_TVE_ID,
};
    
#ifndef _LLD_VBI_M33_REMOTE

    #if 0
    void vbi_enable_ttx_by_osd(struct vbi_device*pdev);
    void enable_vbi_transfer(BOOL enable);
    UINT8 get_inital_page_status();
    UINT16 get_inital_page();
    UINT16 get_first_ttx_page();
    INT32 vbi_m33_attach(struct vbi_config_par *config_par);
    #endif
void vbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request);
//void vbi_set_vbi_data_addr(struct vbi_data_array_t *p_vbi_data_array,UINT32 *cnt_add );
//void vbi_queue_set_rd(UINT16 rd);
//void vbi_main_set_quene_cnt(UINT16 cnt);
//void set_vbi_status_to_see(UINT32 status);
//void set_ttx_by_vbi(BOOL b_on);
//void set_tve_id(UINT16 id);


    #if 0
    static UINT32 lld_vbi_m36_entry[] =
    {
        (UINT32)vbi_m33_attach,
        (UINT32)vbi_enable_ttx_by_osd,
        (UINT32)enable_vbi_transfer,
        (UINT32)get_inital_page,
        (UINT32)get_inital_page_status,
        (UINT32)get_first_ttx_page,
    };
    #endif

static UINT32 lld_vbi_m36_entry[] =
{
    (UINT32)vbi_m3327_setoutput,
    (UINT32)vbi_set_vbi_data_addr,
    (UINT32)vbi_queue_set_rd,
    //(UINT32)vbi_main_set_quene_cnt,
    (UINT32)set_vbi_status_to_see,
    (UINT32)set_ttx_by_vbi,
    (UINT32)set_tve_id,
};


void lld_vbi_m33_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)lld_vbi_m36_entry, msg);
}
#endif

#ifdef _LLD_VBI_M33_REMOTE

/*static UINT32 desc_cfg_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vbi_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};*/

static UINT32 desc_vbi_m33_p_uint32[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};

void vbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request)
{
    jump_to_func(NULL, os_hld_caller, dev, (LLD_VBI_M33_MODULE<<24)|(2<<16)|FUNC_VBI_M33_SET_OUTPUT, desc_vbi_m33_p_uint32);
}

void vbi_set_vbi_data_addr(struct vbi_data_array_t *p_vbi_data_array,UINT32 *cnt_add )
{
    jump_to_func(NULL, os_hld_caller, p_vbi_data_array, (LLD_VBI_M33_MODULE<<24) | (2<<16)|FUNC_VBI_M33_SET_VBI_DATA_ADDR, NULL);
}

void vbi_queue_set_rd(UINT16 rd)
{
    jump_to_func(NULL, os_hld_caller, rd, (LLD_VBI_M33_MODULE<<24)|(1<<16) |FUNC_VBI_M33_SET_RD, NULL);
}

void set_vbi_status_to_see(UINT32 status)
{
    jump_to_func(NULL, os_hld_caller, status, (LLD_VBI_M33_MODULE<<24)|(1<<16) |FUNC_VBI_M33_SET_VBI_STATUS, NULL);
}

void set_ttx_by_vbi(BOOL b_on)
{
    jump_to_func(NULL, os_hld_caller, status, (LLD_VBI_M33_MODULE<<24)|(1<<16) |FUNC_VBI_M33_SET_VBI_BY_TTX, NULL);
}
void set_tve_id(UINT16 id)
{
    jump_to_func(NULL, os_hld_caller, rd, (LLD_VBI_M33_MODULE<<24)|(1<<16) |FUNC_SET_TVE_ID, NULL);
}


#if 0
INT32 vbi_m33_attach(struct vbi_config_par *cfg_param)
{
    jump_to_func(NULL, os_hld_caller, cfg_param,(LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ATTACH, desc_cfg_param);
}

void vbi_enable_ttx_by_osd(struct vbi_device*pdev)
{
    jump_to_func(NULL, os_hld_caller, pdev, (LLD_VBI_M33_MODULE<<24) | (1<<16)|FUNC_VBI_M33_ENABLE_TTX_BY_OSD, NULL);
}

void enable_vbi_transfer(BOOL enable)
{
    jump_to_func(NULL, os_hld_caller, enable,(LLD_VBI_M33_MODULE<<24) |(1<<16)|FUNC_VBI_M33_ENABLE_VBI_TRANSFER, NULL);
}

UINT16 get_inital_page(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LLD_VBI_M33_MODULE<<24) |(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE, NULL);
}

UINT8 get_inital_page_status(void)
{
    jump_to_func(NULL, os_hld_caller, null,(LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE_STATUS,NULL);
}

UINT16 get_first_ttx_page(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LLD_VBI_M33_MODULE<<24) |(0<<16)|FUNC_VBI_M33_GET_FIRST_TTX_PAGE, NULL);
}
#endif


#endif

#endif

