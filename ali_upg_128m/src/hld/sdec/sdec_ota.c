/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: sdec_ota.c

   *    Description:define the ota function using by subtitle decoder
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

INT32 sdec_open(struct sdec_device *dev)
{
    return 0;
}

INT32 sdec_close(struct sdec_device *dev)
{
    return 0;
}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id, UINT16 ancillary_page_id)
{
    return 0;
}

INT32  sdec_stop(struct sdec_device *dev)
{
    return 0;
}

INT32  sdec_pause(struct sdec_device *dev)
{
    return 0;
}

INT32 sdec_attach(struct sdec_feature_config *cfg_param)
{
    return 0;
}

void subt_hld_disply_bl_init(struct sdec_device *dev)
{
}
