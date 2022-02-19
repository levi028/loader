#include <basic_types.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>

#include <api/libisdbtcc/lib_isdbtcc.h>
#include <api/libisdbtcc/isdbtcc_osd.h>
#include <hld/osd/osddrv.h>

//#define LIBSUBT_PRINTF     PRINTF

#ifdef DUAL_ENABLE
#include <modules.h>

enum LIB_ISDBT_FUNC{
    FUNC_LIB_ISDBT_INIT = 0,
    FUNC_OSD_ISDBT_ENTER,
    FUNC_OSD_ISDBT_LEAVE,
    FUNC_LIB_ISDBT_ATTACH,

};

#ifndef _LIB_ISDBTCC_REMOTE
//void lib_isdbtcc_init();
//void osd_isdbtcc_enter();
//void osd_isdbtcc_leave();
void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par);

UINT32 lib_isdbt_entry[] =
{
    (UINT32)lib_isdbtcc_init,
    (UINT32)osd_isdbtcc_enter,
    (UINT32)osd_isdbtcc_leave,
    (UINT32)lib_isdbtcc_attach,
};

void lib_isdbtcc_callee(UINT8 *msg)
{
	if(NULL==msg)
	{
		return;
	}
    os_hld_callee((UINT32)lib_isdbt_entry, msg);
}

#endif

#ifdef _LIB_ISDBTCC_REMOTE
static UINT32 desc_cc_fg_para[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct isdbtcc_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

void lib_isdbtcc_init()
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_LIB_ISDBT_INIT, NULL);
}

void osd_isdbtcc_enter()
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_ENTER, NULL);
}

void osd_isdbtcc_leave()
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_LEAVE, NULL);
}

void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_LIB_ISDBT_ATTACH, desc_cc_fg_para);
}
#endif
#endif

