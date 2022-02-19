#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>

#include <hld/isdbtcc/isdbtcc_dec.h>

#include <math.h>

/////////////////////
#include "isdbtcc_dec_buffer.h"
#include <api/libisdbtcc/lib_isdbtcc.h>

#ifdef DUAL_ENABLE
#include <modules.h>

enum LLD_ISDBTCC_FUNC{
    FUNC_ISDBTCC_DEC_ATTACH = 0,
    FUNC_ISDBTCC_DISPLAY_INIT,
};

#ifndef _LLD_ISDBTCC_REMOTE
INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param);
void isdbtcc_disply_init(struct sdec_device *dev);

UINT32 lld_isdbtcc_entry[] =
{
    (UINT32)isdbtcc_dec_attach,
    (UINT32)isdbtcc_disply_init,
};

void lld_isdbtcc_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)lld_isdbtcc_entry, msg);
}
#endif

#ifdef _LLD_ISDBTCC_REMOTE

static UINT32 desc_cfg_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct sdec_feature_config)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param)
{
    jump_to_func(NULL, os_hld_caller, cfg_param, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DEC_ATTACH, desc_cfg_param);
}

void isdbtcc_disply_init(struct sdec_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DISPLAY_INIT, NULL);
}

//!jary
#if 0
//#ifndef _LLD_SDEC_SW_REMOTE

//For S3602F, below 2 functions shouldnt be called from CPU, leaving them is just for link issue when link to old DMX
INT32 sdec_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got,
    struct control_block *ps_data_cb)

{
    SDBBP();
}

void sdec_update_write(void *pdev,UINT32 u_data_size)
{
    SDBBP();
}
#endif
#endif

#endif

