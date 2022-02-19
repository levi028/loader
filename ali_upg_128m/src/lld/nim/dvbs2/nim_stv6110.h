#ifndef __LLD_STV6110_H__
#define __LLD_STV6110_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#define REF_OSC_FREQ    16000

#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_stv6110_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_stv6110_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_stv6110_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif
#endif  /* __LLD_STV6110_H__ */

