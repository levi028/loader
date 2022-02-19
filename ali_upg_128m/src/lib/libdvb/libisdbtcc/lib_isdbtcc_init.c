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

extern struct isdbtcc_config_par g_isdbtcc_config_par;//
extern BOOL g_isdbtcc_init;

//#if (ISDBT_CC==1)
void lib_isdbtcc_init()
{
    g_isdbtcc_config_par.isdbtcc_osd_layer_id = 0;
    g_isdbtcc_init = TRUE;
}
//#endif
