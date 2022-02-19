/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ttx_remote.c
*
*    Description: The file is mainly to define the remote call function that
     will be used by TTX.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libtsi/si_types.h>
//#include <api/libtsi/si_table.h>

#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>

#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#endif

#define LIBSUBT_PRINTF     PRINTF

#if 0//def DUAL_ENABLE
#include <modules.h>

enum LIB_TTX_FUNC
{
    FUNC_LIB_TTX_INIT = 0,
    func_lib_ttx_attach,
    func_lib_ttx_osd_set_size,
};

#ifdef _LIB_TTX_REMOTE
static UINT32 desc_ttx_eng_attach[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ttx_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};
void ttxeng_init(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_TTX_MODULE<<24) |(0<<16)|FUNC_LIB_TTX_INIT, NULL);
}

void  ttxeng_attach(struct ttx_config_par *pconfig_par)
{
    jump_to_func(NULL,os_hld_caller,pconfig_par,(LIB_TTX_MODULE<<24)|(1<<16)|func_lib_ttx_attach,desc_ttx_eng_attach);
}

void ttx_osd_set_size(UINT32 width,UINT32 height)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_TTX_MODULE<<24)|(2<<16) |func_lib_ttx_osd_set_size, NULL);
}
#else
void  ttxeng_init(void);
void  ttxeng_attach(struct ttx_config_par *pconfig_par);

static UINT32 lib_ttx_entry[] =
{
    (UINT32)ttxeng_init,
    (UINT32)ttxeng_attach,
    (UINT32)ttx_osd_set_size,
};

void lib_ttx_callee(UINT8 *msg)
{
    if(NULL == msg)
    {
        return;
    }
    os_hld_callee((UINT32)lib_ttx_entry, msg);
}
#endif

#endif

