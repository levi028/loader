#ifndef __LLD_VZ7803_H
#define __LLD_VZ7803_H

#include <types.h>
#include <hld/nim/nim_tuner.h>

#define REF_OSC_FREQ    4000 /* 4MHZ */

#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_vz7803_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_vz7803_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_vz7803_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_VZ7803_H__ */

