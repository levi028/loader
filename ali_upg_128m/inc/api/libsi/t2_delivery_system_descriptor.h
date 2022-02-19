#ifndef __T2_DELIVERY_SYSTEM_DESCRIPTOR_H__
#define __T2_DELIVERY_SYSTEM_DESCRIPTOR_H__

#include <types.h>
#include <sys_config.h>
#include "si_module.h"
#include "si_nit.h"

#if defined(DVBT2_SUPPORT)

#ifdef __cplusplus
extern "C" {
#endif

INT32 t2_delivery_system_descriptor_parser(UINT8 tag, UINT8 length, UINT8 *data, void *priv);


#ifdef __cplusplus
}
#endif

#endif  //DVBT2_SUPPORT

#endif // __T2_DELIVERY_SYSTEM_DESCRIPTOR_H__

