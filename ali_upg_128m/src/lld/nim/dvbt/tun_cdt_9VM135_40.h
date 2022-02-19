#ifndef __TUN_CDT_9VM135_40_H__
#define __TUN_CDT_9VM135_40_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif


#define TUNER_DEMOD_ADR                0x86
#define TUNER_SW_SUBADDR                 0x00//switch
#define TUNER_AD_SUBADDR                 0x01//adjust
#define TUNER_DA_SUBADDR                 0x02//data
#define TUNER_DEV_ADR                    0xC2

#define TUNER_IF_FREQ                        36160
#define TUNER_FMIF_FREQ                33300
#define FREQSTEP                        166667

#define NXP_TD1611ALF_INT_AGC    1
//Dword InitTuner(Byte PSystem, Byte SSystem);
//Dword SetTuner(IN  Dword dwFreq/*KHz*/,  IN  Byte  ucBw /*MHz*/, IN Dword dwIF);
//Dword GetTuner(OUT Byte* pArray, IN Byte count);
//Dword GetDemod(OUT Byte* pArray, IN Byte count);
//Dword AutoScanTuner(IN Dword dwStartFreq, IN Dword dwEndFreq);
//Dword AutoScanFMTuner(IN Dword dwStartFreq, IN Dword dwEndFreq);
//Dword GetProgTuner(IN Byte bChannelNum);
//Dword GetProgFMTuner(IN Byte bChannelNum);
//Dword WriteTunerReg(IN Byte* pArray, IN Byte count);
//Dword ReadTunerReg(OUT Byte* pArray, IN Byte count);
//Dword AdjustTuner(void);
//Dword CalibrateTuner(void);




INT32 tun_cdt_9VM135_40_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_cdt_9VM135_40_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)    ;
INT32 tun_cdt_9VM135_40_status(UINT32 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_CDT_9VM135_40_H__ */
