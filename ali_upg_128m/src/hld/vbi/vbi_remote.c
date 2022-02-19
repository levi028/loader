/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_remote.c

   *    Description:define the remote call uisng by VBI under dual_cpu
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>

#if 0//def DUAL_ENABLE
#include <modules.h>

enum HLD_VBI_FUNC
{
    FUNC_VBI_OPEN = 0,
    FUNC_VBI_CLOSE,
    FUNC_VBI_IO_CONTROL,
    FUNC_VBI_SET_OUTPUT,
    FUNC_VBI_START,
    FUNC_VBI_STOP,
    FUNC_VBI_DEFAULT_GO_SET,
    FUNC_VBI_ATTACH,
    FUNC_VBI_HLD_ENABLE_TTX_BY_OSD,
    FUNC_VBI_HLD_ENABLE_TRANSFER,

    FUNC_VBI_TTX_ENG_PARA_INIT,
    FUNC_VBI_TTX_CHECK_SEPERATE_TTXSUBT_MODE,
    FUNC_VBI_TTX_ALLOC_P26_BUFFER,

};

#ifndef _HLD_VBI_REMOTE
UINT32 hld_vbi_entry[] =
{
    (UINT32)vbi_open,
    (UINT32)vbi_close,
    (UINT32)vbi_ioctl,
    (UINT32)vbi_setoutput,
    (UINT32)vbi_start,
    (UINT32)vbi_stop,
    (UINT32)ttx_default_g0_set,
    (UINT32)vbi_attach,
    (UINT32)vbi_hld_enable_ttx_by_osd,
    (UINT32)enable_hld_vbi_transfer,
    (UINT32)ttx_eng_para_init,
    (UINT32)ttx_check_seperate_ttxsubt_mode,
    (UINT32)ttx_alloc_subpage_p26_naton_buffer,
};

void hld_vbi_callee(UINT8 *msg)
{
    if(NULL == msg)
    {
        return;
    }
    os_hld_callee((UINT32)hld_vbi_entry, msg);
}
#endif

#ifdef _HLD_VBI_REMOTE

static UINT32 desc_cfg_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vbi_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vbi_p_uint32[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vbi_io_ctrl_p_uint32[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};

static UINT32 desc_vbi_io_ctrl_ttx_page_info[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ttx_page_info)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,
  0,
};


/*
 *     Name        :   vbi_open()
 *    Description    :   Open a vbi device
 *    Parameter    :    struct vbi_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 vbi_open(struct vbi_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_OPEN, NULL);
}

/*
 *     Name        :   vbi_close()
 *    Description    :   Close a vbi device
 *    Parameter    :    struct vbi_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 vbi_close(struct vbi_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_CLOSE, NULL);
}


/*
 *     Name        :   vbi_io_control()
 *    Description    :   vbiel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param)
{
    UINT32 *desc = NULL;

    switch(cmd)
    {
        case IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT:
        case IO_VBI_WORK_MODE_HISTORY :
        case IO_VBI_ENGINE_OPEN:
        case IO_VBI_ENGINE_CLOSE:
        case IO_VBI_ENGINE_UPDATE_PAGE:
        case IO_VBI_ENGINE_SHOW_ON_OFF:
        case IO_VBI_ENGINE_SEND_KEY:
        case IO_VBI_ENGINE_SET_CUR_LANGUAGE:
        case IO_VBI_SELECT_OUTPUT_DEVICE:
        default:
            desc = NULL;
            break;
        case IO_VBI_ENGINE_GET_STATE:
            desc = desc_vbi_io_ctrl_p_uint32;
            break;
        case IO_VBI_ENGINE_UPDATE_INIT_PAGE:
        case IO_VBI_ENGINE_UPDATE_SUBT_PAGE:
            desc = desc_vbi_io_ctrl_ttx_page_info;
            break;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(3<<16)|FUNC_VBI_IO_CONTROL, desc);

}

void vbi_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_SET_OUTPUT, desc_vbi_p_uint32);
}

INT32 vbi_start(struct vbi_device *dev,t_ttxdec_cbfunc p_cb_func)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_START, NULL);
}

INT32 vbi_stop(struct vbi_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_STOP, NULL);
}

void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_DEFAULT_GO_SET, NULL);
}

INT32 vbi_attach(struct vbi_config_par *cfg_param)
{
    jump_to_func(NULL, os_hld_caller, cfg_param, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_ATTACH, desc_cfg_param);
}

void vbi_hld_enable_ttx_by_osd(struct vbi_device *pdev)
{
    jump_to_func(NULL, os_hld_caller, pdev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_HLD_ENABLE_TTX_BY_OSD, NULL);
}

void enable_hld_vbi_transfer(BOOL enable)
{
    jump_to_func(NULL, os_hld_caller, enable, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_HLD_ENABLE_TRANSFER, NULL);
}

void ttx_eng_para_init(void)
{
    jump_to_func(NULL, os_hld_caller, NULL, (HLD_VBI_MODULE<<24)|(0<<16)|FUNC_VBI_TTX_ENG_PARA_INIT, NULL);
}

void ttx_check_seperate_ttxsubt_mode(UINT8 *mode)
{
    jump_to_func(NULL,os_hld_caller,mode,(HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_TTX_CHECK_SEPERATE_TTXSUBT_MODE, NULL);
}


INT32 ttx_alloc_subpage_p26_naton_buffer(void)
{
    jump_to_func(NULL, os_hld_caller, NULL, (HLD_VBI_MODULE<<24)|(0<<16)|FUNC_VBI_TTX_ALLOC_P26_BUFFER, NULL);
}

//For S3602F, below 2 functions shouldnt be called from CPU, leaving them is just for link issue when link to old DMX
INT32 vbi_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,
                    UINT32 *pu_size_got, struct control_block *p_data_cb)
{
    ASSERT(0);
    return -1;
}

void vbi_update_write(void *pdev, UINT32 u_data_size)
{
    ASSERT(0);
}

#endif

#endif

