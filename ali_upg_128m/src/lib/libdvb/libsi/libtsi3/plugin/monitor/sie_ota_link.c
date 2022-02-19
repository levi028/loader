/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_core.c
*
*    Description: main function of SI monitor
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
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/psi_db.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsi/si_sdtt.h>
#include "sie_monitor_core.h"
#include "sie_monitor_process_table.h"

#ifdef _BUILD_OTA_E_

void sim_open_monitor(UINT32 praram)
{
    
}

void sim_close_monitor(UINT32 praram)
{
    
}

UINT32 sim_start_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 sim_pid,UINT32 param)
{
    return -1;
}

INT32 sim_stop_monitor(UINT32 monitor_id)
{
    return ERR_FAILUE;
}

#ifdef _BUILD_LOADER_COMBO_
INT32 sim_register_scb(UINT32 monitor_id, sim_section_callback callback, void *priv)
{
    return ERR_FAILUE;   
}

INT32 sim_unregister_scb(UINT32 monitor_id, sim_section_callback callback)
{
    return ERR_FAILUE;   
}
#endif

#endif
