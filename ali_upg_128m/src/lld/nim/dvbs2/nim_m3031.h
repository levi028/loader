#ifndef __NIM_M3031_H
#define __NIM_M3031_H
//#define AVOID_PLL_M_SPUR
//#define AVOID_LPF_WIDEST
//#include "types.h"
#include <types.h>
#include <hld/nim/nim_tuner.h>
#include <osal/osal.h>

#define MAX_TUNER_SUPPORT_NUM 2
//struct QPSK_TUNER_CONFIG_EXT
//{
//	UINT16 wTuner_Crystal;			/* Tuner Used Crystal: in KHz unit */
//	UINT8  cTuner_Base_Addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
//	UINT8  cTuner_Out_S_D_Sel;		/* Tuner Output mode Select: 1 --> Single end, 0 --> Differential */
//	UINT32 i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/	
//};

//#define M3031_XTAL_FREQ_MHZ	27
#define ERR_FAIL			-1
#define M3031_GAIN_ERROR	-111111
#define LT_GAIN_DISABLE		-111
//#define LT_GAIN_AUTO		100
#define M3031_XTAL_FREQ_MHZ 27 //must be 27
#define FAST_RFAGC_SPEED	0 //use for fast RF AGC locking
//#define SLOW_RFAGC_SPEED	13 //use after AGC is possibly locked

#define FAST_RFAGC_WAIT_MS	100 //wait time (ms) for RFAGC to settle in fast mode
#define CHIP_ID_M3031B	0x02
#define M3031B_OLD 0
#define M3031B_NEW 1


#define os_get_tick_count 		OS_GetTickCount 

#ifdef __cplusplus
extern "C"
{
#endif

//int i2c_write(UINT32 i2c_type_id, UINT8 i2c_base_addr, UINT8 *pData, UINT32 length);
//int i2c_read(UINT32 i2c_type_id, UINT8 i2c_base_addr, UINT8 *pData, UINT32 length);
//void osal_task_sleep(UINT32 ms);

INT32 nim_m3031_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptr_tuner_config);
INT32 nim_m3031_control(UINT32 tuner_id, UINT32 freq, UINT32 bb_sym);
INT32 nim_m3031_status(UINT32 tuner_id, UINT8 *lock);
INT32 nim_m3031_gain(UINT32 tuner_id, UINT32 demod_agc);
INT32 nim_m3031_lt_gain(UINT32 tuner_id, INT32 lt_gain);
INT32 nim_m3031_standby(UINT32 tuner_id);
INT32 nim_m3031b_command(UINT32 tuner_id, INT32 cmd, INT32 *param);


#ifdef __cplusplus
}
#endif

#endif  /* __LLD_C3031_H__ */

