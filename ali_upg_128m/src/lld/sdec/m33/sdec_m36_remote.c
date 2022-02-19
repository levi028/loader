/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m36_remote.c
*
*    Description: The file is to define the function to the subtitle decoder
     remote call.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>
#ifdef DUAL_ENABLE
#include <modules.h>
#endif

#include "sdec_buffer.h"
#include "sdec_m3327.h"

#ifdef DUAL_ENABLE

enum LLD_SDEC_SW_FUNC
{
    FUNC_SDEC_M33_ATTACH = 0,
    FUNC_SDEC_SBUT_DISPLAY_BL_INIT,
};

#ifndef _LLD_SDEC_SW_REMOTE

INT32 sdec_m33_attach(struct sdec_feature_config * cfg_param);
void subt_disply_bl_init(struct sdec_device *dev);

static UINT32 lld_sdec_m36_entry[] =
{
    (UINT32)sdec_m33_attach,
    (UINT32)subt_disply_bl_init,
};

void lld_sdec_sw_callee(UINT8 *msg)
{
	if(NULL==msg)
	{
		return;
	}
    os_hld_callee((UINT32)lld_sdec_m36_entry, msg);
}
#endif

#ifdef _LLD_SDEC_SW_REMOTE

static UINT32 desc_cfg_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct sdec_feature_config)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

INT32 sdec_m33_attach(struct sdec_feature_config *cfg_param)
{
    jump_to_func(NULL, os_hld_caller, cfg_param, (LLD_SDEC_SW_MODULE<<24)|
        (1<<16)|FUNC_SDEC_M33_ATTACH, desc_cfg_param);
}

void subt_disply_bl_init(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (LLD_SDEC_SW_MODULE<<24)|(1<<16)|
        FUNC_SDEC_SBUT_DISPLAY_BL_INIT, NULL);
}

//For S3602F, below 2 functions shouldnt be called from CPU,
//leaving them is just for link issue when link to old DMX
INT32 sdec_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got,
    struct control_block *ps_data_cb)

{
    SDEC_ASSERT(0);
    return 0;
}

void sdec_update_write(void *pdev,UINT32 u_data_size)
{
    SDEC_ASSERT(0);
}

#endif

#endif

