#ifndef _HEAD_FILE_H_
#define _HEAD_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>

#include <osal/osal.h>
#include <hal/hal_mem.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <bus/sci/sci.h>
#include <bus/tsi/tsi.h>
#include <bus/pinmux/pinmux.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <hld/nim/nim_dev.h>

#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/vbi/vbi.h>
#include <hld/sdec/sdec.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv_avc.h>
#include <hld/decv/decv.h>
#include <hld/osd/osddrv.h>
#include <hld/rfm/rfm.h>
#include <api/libc/alloc.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#include <hld/decv/decv_hevc.h>


#ifdef CI_SUPPORT
#include <hld/cic/cic.h>
#endif
#if defined(_BC_CA_STD_ENABLE_) || defined(_BC_CA_ENABLE_)
#include <hld/smc/smc.h>
#include <bus/otp/otp.h>
#endif
#ifdef MULTI_CAS
#include <hld/smc/smc.h>
#include <api/libcas/mcas.h>
#include <bus/otp/otp.h>
#endif
#include <hld/scart/scart.h>

#include <api/libtsi/db_3l.h>
#include <api/libnim/lib_nim.h>
#include <hld/nim/nim_tuner.h>
#include <api/libsubt/lib_subt.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/subt_osd.h>
#include <sys_parameters.h>
#include <api/libosd/osd_lib.h>
#ifdef HDTV_SUPPORT
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif
#include <hld/crypto/crypto.h>

#ifdef NETWORK_SUPPORT
#include <hld/net/net.h>
#endif
#if defined(_MHEG5_V20_ENABLE_)
#include <mh5_api/mh5_api.h>
#endif

#ifdef HDCP_IN_FLASH
#include "hdcp_des.h"
#endif
#include <hld/dsc/dsc.h>
#ifdef CI_PLUS_SUPPORT
#include <bus/otp/otp.h>
#endif
#if (ISDBT_CC == 1)
#include <hld/sdec/sdec.h>
#include <api/libisdbtcc/isdbtcc_osd.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

#ifdef __cplusplus
}
#endif

#endif

