/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: sdec_remote.c

   *    Description:define the remote call function using by subtitle decoder
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>


#ifdef DUAL_ENABLE
#include <modules.h>

enum HLD_SDEC_FUNC
{
    FUNC_SDEC_OPEN = 0,
    FUNC_SDEC_CLOSE,
    FUNC_SDEC_START,
    FUNC_SDEC_STOP,
    FUNC_SDEC_PAUSE,
    FUNC_SDEC_ATTACH,
    FUNC_SDEC_HLD_DISPLAY_INIT,
};

#ifndef _HLD_SDEC_REMOTE
UINT32 hld_sdec_entry[] =
{
    (UINT32)sdec_open,
    (UINT32)sdec_close,
    (UINT32)sdec_start,
    (UINT32)sdec_stop,
    (UINT32)sdec_pause,
    (UINT32)sdec_attach,
    (UINT32)subt_hld_disply_bl_init,
};

void hld_sdec_callee(UINT8 *msg)
{
	if(NULL==msg)
	{
		return;
	}
    os_hld_callee((UINT32)hld_sdec_entry, msg);
}
#endif

#ifdef _HLD_SDEC_REMOTE

static UINT32 desc_cfg_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct sdec_feature_config)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};


/*
 *     Name        :   sdec_open()
 *    Description    :   Open a sdec device
 *    Parameter    :    struct sdec_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 sdec_open(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_OPEN, NULL);
}

/*
 *     Name        :   sdec_close()
 *    Description    :   Close a sdec device
 *    Parameter    :    struct sdec_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 sdec_close(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_CLOSE, NULL);
}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(3<<16)|FUNC_SDEC_START, NULL);
}

INT32  sdec_stop(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_STOP, NULL);

}

INT32  sdec_pause(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_PAUSE, NULL);
}

INT32 sdec_attach(struct sdec_feature_config *cfg_param)
{
    jump_to_func(NULL, os_hld_caller, cfg_param, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_ATTACH, desc_cfg_param);
}

void subt_hld_disply_bl_init(struct sdec_device *dev)
{

    jump_to_func(NULL, os_hld_caller, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_HLD_DISPLAY_INIT, NULL);
}


#if 0 //this function be done in lib_subt.c
INT32  sdec_showonoff(struct sdec_device *dev,BOOL b_on)
{
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return RET_FAILURE;
    }

    if (dev->showonoff)
    {
        return dev->showonoff(dev,b_on);
    }
    return RET_FAILURE;
}
#endif

#endif

#endif

