#ifndef __TUN_CDT_9VM80_H__
#define __TUN_CDT_9VM80_H__

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

#define TUNER_DTVIF_FREQ                36130
#define TUNER_ATVIF_FREQ                    38900
#define TUNER_FMIF_FREQ                33300
#define FREQSTEP                        166667

#define    VL_START       44000
#define    VL_END         160000
#define    VH_START    160000
#define    VH_END        446000
#define    U_START        446000
#define    U_END        865000

typedef enum
{
    PAL,
    NTSC,
    SECAM,
    FM
}PSYSTEM;

typedef enum
{
    MN,
    BG,
    DK,
    I,
    L,
    LL,
    FMS
}SSYSTEM;

////////////////////////////
// DTV Control Byte 1
////////////////////////////
#define DREF_DIV            0x06 //0x16
#define DATP_BIT            0x05
#define DRS_BIT                 0x00 //(REF_DIV : RS_BIT) 0X06:0X00; 0X07:0X01; 0X08:0X02; 0X16:0X03; 0X32:0X04; 0X20:0X05
#define DTUNER_CTL1_BYTE    (0x80)|(DATP_BIT << 3)|(DRS_BIT)

////////////////////////////
// ATV Control Byte 1
////////////////////////////
#define AREF_DIV            0x20
#define AATP_BIT            0x05
#define ARS_BIT                 0x05 //(REF_DIV : RS_BIT) 0X06:0X00; 0X07:0X01; 0X08:0X02; 0X16:0X03; 0X32:0X04; 0X20:0X05
#define ATUNER_CTL1_BYTE    (0x80)|(AATP_BIT << 3)|(ARS_BIT)

////////////////////////////
// DTV Control Byte 2
////////////////////////////
#define DATC_BIT                0x00
#define DMODE_BIT            0x01
#define DDISGCA_BIT            0x00
#define DIFOA_BIT                0x00 //0: Dif 1:Aif
#define DCP2_BIT                0x00
#define DXLO_BIT                0x01 //0: XTALOUT EN ; 1:XTALOUT DIS
#define DTUNER_CTL2_BYTE    (0xC0) |(DATC_BIT << 5) |(DMODE_BIT << 4) | (DDISGCA_BIT << 3) |( DIFOA_BIT << 2) | (DCP2_BIT << 1) | (DXLO_BIT)

////////////////////////////
// ATV Control Byte 2
////////////////////////////
#define AATC_BIT                0x00
#define AMODE_BIT            0x01
#define ADISGCA_BIT            0x00
#define AIFOA_BIT                0x01 //0: Dif 1:Aif
#define ACP2_BIT                0x00
#define AXLO_BIT                0x01 //0: XTALOUT EN ; 1:XTALOUT DIS
#define ATUNER_CTL2_BYTE    (0xC0) |(AATC_BIT << 5) |(AMODE_BIT << 4) | (ADISGCA_BIT << 3) |( AIFOA_BIT << 2) | (ACP2_BIT << 1) | (AXLO_BIT)

////////////////////////////
// Band Switching Byte
////////////////////////////
#define CP_BIT                0x01
#define P5_BIT                0x00
#define TUNER_VL_BAND        0x01
#define TUNER_VH_BAND        0x02
#define TUNER_UH_BAND        0x08
#define TUNER_BS_BYTE        (CP_BIT << 6) |(P5_BIT << 4) | 0x00

#if 0
////////////////////////////
// System switch Byte
////////////////////////////
#define SWITCHDATAPAL        0x56
#define ADJUSTDATAPAL        0x70
#define SWITCHDATANTSC        0x56
#define ADJUSTDATANTSC        0x30
#define SWITCHDATASECAM    0xc6
#define ADJUSTDATASECAM    0x70
#define SWITCHDATAFM        0x0C//FMSYSTEMAUTOMUTE 0x0e
#define ADJUSTDATAFM        0x6e//0xee 6e:audio gain 0db ee:-6db

////////////////////////////
// sound control byte
////////////////////////////
#define SOUNDSYSBG        0x49
#define SOUNDSYSI        0x4a
#define SOUNDSYSDK        0x4b
#define SOUNDSYSMN        0x08
#define SOUNDSYSL        0x0a
#define SOUNDSYSFM        0x08

////////////////////////////////
// SN761640 Read Mode
////////////////////////////////
#define TN_CB2_ATC        0x20
#define TN_CB2_MODE        0x10
#define TN_CB2_DISCGA    0x08
#define TN_CB2_IFDA        0x04
#define TN_CB2_CP2        0x02
#define TN_CB2_XLO          0x01

////////////////////////////////
// TDA6509 Read Mode
////////////////////////////////
#define TN_READ_POR    0x80
#define TN_READ_FL        0x40
#define TN_READ_ADC3    0x04
#define TN_READ_ADC2    0x02
#define TN_READ_ADC1    0x01
#define TN_AFCADC_STATUS  (TN_READ_ADC3 | TN_READ_ADC2 |TN_READ_ADC1)

////////////////////////////////
// TDA9883 Read Mode
////////////////////////////////
#define TN_READ_AFCWIN     0x80
#define TN_READ_VIFL           0x40
#define TN_READ_FMIFL        0x20
#define TN_READ_AFC4         0x10
#define TN_READ_AFC3         0x08
#define TN_READ_AFC2         0x04
#define TN_READ_AFC1         0x02
#define TN_READ_PONR         0x01
#endif

#define TN_READ_AFC    (TN_READ_AFC1 | TN_READ_AFC2 | TN_READ_AFC3 | TN_READ_AFC4)

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




INT32 tun_cdt_9VM80_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_cdt_9VM80_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)    ;
INT32 tun_cdt_9VM80_status(UINT32 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_CDT_9VM80_H__ */
