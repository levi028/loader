/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_monitor_ap.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>


UINT32 g_pvr_pmt_count = 0; // pmt count

INT32 pvr_pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    if (1 == (++g_pvr_pmt_count))
    {
#ifdef _DEBUG_PATCH
        libc_printf("patch -> %s send pmt msg - tick %d\n",__FUNCTION__,osal_get_tick());
#endif
        //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PMT_UPDATE, monitor_id, FALSE);
    }

    return RET_SUCCESS;
}


