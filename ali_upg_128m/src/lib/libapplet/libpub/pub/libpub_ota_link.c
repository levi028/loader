#include <osal/osal_task.h>
#include <api/libc/printf.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libtsi/db_3l.h>
#include <api/libnim/lib_nim.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/decv/vdec_driver.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca_dev.h>
#include <api/libc/string.h>

#include "lib_pub_inner.h"
#ifdef _BUILD_OTA_E_
void monitor_switch(UINT32 onoff)
{    
}

void monitor_proc(void)
{
}

void monitor_rst(void)
{
}

void monitor_init(UINT32 interval, libpub_monitor_callback func)
{
}

void mv_init(void)
{
}

INT32 si_monitor_on(UINT32 index)
{
	return ERR_FAILUE;
}

void sdt_monitor_on(UINT32 index)
{
}
#endif
