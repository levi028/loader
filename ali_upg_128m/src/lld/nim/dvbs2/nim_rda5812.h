#ifndef __LLD_RDA5812_H
#define __LLD_RDA5812_H

#include <types.h>
#include <hld/nim/nim_tuner.h>



#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_rda5812_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_rda5812_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_rda5812_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif
