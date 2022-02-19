/**
 * Copyright (C) 2005 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmhalFrontEndtypes.h
 *
 * \version       1
 *
 * \date          %date_modified%
 *
 * \brief         Describe briefly the purpose of this file.
 *
 * REFERENCE DOCUMENTS :
 *
 * Detailled description may be added here.
 *
 * \section info Change Information
 *
 * \verbatim
   Date          Modified by CRPRNr  TASKNr  Maintenance description
   -------------|-----------|-------|-------|-----------------------------------
   19-May-2005  | B.GUILLOT |       | 6884  |
   \endverbatim
 *
*/


#ifndef TMHALFRONTENDTYPES_H
#define TMHALFRONTENDTYPES_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/
#include "tmtypes.h"


#ifdef __cplusplus
extern "C"
{
#endif



/*============================================================================*/
/*                       MACRO DEFINITION                                     */
/*============================================================================*/
#define TMBSL_ERR_TUNER_BASE                0x00
#define TMBSL_DEMOD_BER_NOT_READY           0x7fffffff

#define TMBSL_MAX_CHANNEL_FRONTEND          0x08

#ifndef NULL
    #define NULL (void *)0
#endif

/*============================================================================*/
/*                       ENUM OR TYPE DEFINITION                              */
/*============================================================================*/
/* standard*/
typedef enum _tmhalFrontEndStandard_t {
    tmhalFrontEndStandardDSS = 0,
    tmhalFrontEndStandardDVBS,
    tmhalFrontEndStandardBSD,
    tmhalFrontEndStandardDVBT,
    tmhalFrontEndStandardDVBC,
    tmhalFrontEndStandardDVBH,
    tmhalFrontEndStandardMax
} tmhalFrontEndStandard_t, *ptmhalFrontEndStandard_t;

/* spectral inversion*/
typedef enum _tmhalFrontEndSpectrum_t {
    tmhalFrontEndSpectrumAuto = 0,
    tmhalFrontEndSpectrumNormal,
    tmhalFrontEndSpectrumInverted,
    tmhalFrontEndSpectralInvMax
} tmhalFrontEndSpectrum_t, *ptmhalFrontEndSpectrum_t;

/* modulation*/
typedef enum _tmhalFrontEndModulation_t {
    tmhalFrontEndModulationAuto = 0,
    tmhalFrontEndModulationBpsk,
    tmhalFrontEndModulationQpsk,
    tmhalFrontEndModulationQam4,
    tmhalFrontEndModulationPsk8,
    tmhalFrontEndModulationQam16,
    tmhalFrontEndModulationQam32,
    tmhalFrontEndModulationQam64,
    tmhalFrontEndModulationQam128,
    tmhalFrontEndModulationQam256,
    tmhalFrontEndModulationQam512,
    tmhalFrontEndModulationQam1024,
    tmhalFrontEndModulationMax
} tmhalFrontEndModulation_t, *ptmhalFrontEndModulation_t;

/* viterbi rate*/
typedef enum _tmhalFrontEndDepuncRate_t {
    tmhalFrontEndDepuncRateAuto = 0,
    tmhalFrontEndDepuncRate12,
    tmhalFrontEndDepuncRate23,
    tmhalFrontEndDepuncRate34,
    tmhalFrontEndDepuncRate45,
    tmhalFrontEndDepuncRate56,
    tmhalFrontEndDepuncRate67,
    tmhalFrontEndDepuncRate78,
    tmhalFrontEndDepuncRate89,
    tmhalFrontEndDepuncRateMax
} tmhalFrontEndDepuncRate_t, *ptmhalFrontEndDepuncRate_t;

/* turbo code*/
typedef enum _tmhalFrontEndTurboCode_t {
    tmhalFrontEndTurboCodeDisabled = 0,
    tmhalFrontEndTurboCode1,
    tmhalFrontEndTurboCode2,
    tmhalFrontEndTurboCode3,
    tmhalFrontEndTurboCode4,
    tmhalFrontEndTurboCodeMax
} tmhalFrontEndTurboCode_t, *ptmhalFrontEndTurboCode_t;

/* tone burst*/
typedef enum _tmhalFrontEndToneBurst_t {
    tmhalFrontEndToneBurstDisabled = 0,
    tmhalFrontEndToneBurstSatA,
    tmhalFrontEndToneBurstSatB,
    tmhalFrontEndToneBurstMax
} tmhalFrontEndToneBurst_t, *ptmhalFrontEndToneBurst_t;

/* frequency offset*/
typedef enum _tmhalFrontEndRfOffset_t {
    tmhalFrontEndRfOffsetAuto = 0,
    tmhalFrontEndRfOffsetNull,
    tmhalFrontEndRfOffsetPlus125,
    tmhalFrontEndRfOffsetMinus125,
    tmhalFrontEndRfOffsetPlus166,
    tmhalFrontEndRfOffsetMinus166,
    tmhalFrontEndRfOffsetPlus333,
    tmhalFrontEndRfOffsetMinus333,
    tmhalFrontEndRfOffsetPlus500,
    tmhalFrontEndRfOffsetMinus500,
    tmhalFrontEndRfOffsetMax
} tmhalFrontEndRfOffset_t, *ptmhalFrontEndRfOffset_t;

/* guard interval*/
typedef enum _tmhalFrontEndGuardInterval_t {
    tmhalFrontEndGuardIntervalAuto = 0,
    tmhalFrontEndGuardInterval32,
    tmhalFrontEndGuardInterval16,
    tmhalFrontEndGuardInterval8,
    tmhalFrontEndGuardInterval4,
    tmhalFrontEndGuardIntervalMax
} tmhalFrontEndGuardInterval_t, *ptmhalFrontEndGuardInterval_t;

/* fast Fourrier transform size*/
typedef enum _tmhalFrontEndFft_t {
    tmhalFrontEndFftAuto = 0,
    tmhalFrontEndFft2K,
    tmhalFrontEndFft8K,
    tmhalFrontEndFft4K,
    tmhalFrontEndFftMax
} tmhalFrontEndFft_t, *ptmhalFrontEndFft_t;

/* hierarchy*/
typedef enum _tmhalFrontEndHierarchy_t {
    tmhalFrontEndHierarchyAuto = 0,
    tmhalFrontEndHierarchyNo,
    tmhalFrontEndHierarchyAlpha1,
    tmhalFrontEndHierarchyAlpha2,
    tmhalFrontEndHierarchyAlpha4,
    tmhalFrontEndHierarchyMax
} tmhalFrontEndHierarchy_t, *ptmhalFrontEndHierarchy_t;

/* priority*/
typedef enum _tmhalFrontEndPriority_t {
    tmhalFrontEndPriorityAuto = 0,
    tmhalFrontEndPriorityHigh,
    tmhalFrontEndPriorityLow,
    tmhalFrontEndPriorityMax
} tmhalFrontEndPriority_t, *ptmhalFrontEndPriority_t;

/* status of the manager function*/
typedef enum _tmhalFrontEndManagerStatus_t {
    tmhalFrontEndManagerStatusCarrierOk = 0,
    tmhalFrontEndManagerStatusNoCarrier,
    tmhalFrontEndManagerStatusNotCompleted,
    tmhalFrontEndManagerStatusMax
} tmhalFrontEndManagerStatus_t, *ptmhalFrontEndManagerStatus_t;

/* Fine (but slow) autoscan or normal autoscan*/
typedef enum _tmhalFrontEndAutoScanMode_t
{
    tmhalFrontEndAutoScanNone = 0,
    tmhalFrontEndAutoScanAverage,
    tmhalFrontEndAutoScanFine
} tmhalFrontEndAutoScanMode_t;

typedef enum _tmhalFrontEndBandType_t
{
    tmhalFrontEndBandTypeDefault = 0,
    tmhalFrontEndBandTypeSECAM_L,
    tmhalFrontEndBandTypeMax
} tmhalFrontEndBandType_t;

typedef enum _tmhalFrontEndSatPolarVoltage_t
{
    /** 0V for Analog swith, 0 volts is used     */
    tmhalFrontEndSat0V_E,
    /** 14V for the vertical polarisation usually     */
    tmhalFrontEndSat14V_E,
    /** 18V for the horizontal polarisation usually   */
    tmhalFrontEndSat18V_E
}tmhalFrontEndSatPolarVoltage_t;

typedef enum _tmhalFrontEndTunerMode_t
{
    tmhalFrontEndTunerAnalogMode_E,
    tmhalFrontEndTunerDigitalMode_E,
    tmhalFrontEndTunerModeMax_E
}tmhalFrontEndTunerMode_t;

/* Status of the carrier phase lock loop */
typedef enum _tmhalFrontEndCarrierStatus_t
{
      /** statuc unknown*/
      tmhalFrontEndCarrierUnknow = 0,
      /** statuc loop locked*/
      tmhalFrontEndCarrierLock,
      /** loop not locked */
      tmhalFrontEndCarrierNoLock,
      tmhalFrontEndCarrierMax
}tmhalFrontEndCarrierStatus_t;

/* Pio control */
typedef enum _tmhalFrontEndPioCtrl_t
{
      /** Pio as Input */
      tmhalFrontEndPioAsInput = 0,
      /** Pio as output */
      tmhalFrontEndPioAsOutput,
      tmhalFrontEndPioAsInOutMax
} tmhalFrontEndPioCtrl_t;
/* config items*/
typedef enum _tmhalFrontEndConfig_t {
    tmhalFrontEndBoard = 0,
    tmhalFrontEndStandard,
    tmhalFrontEndLPSEL = 6,
    tmhalFrontEndInit = 100,
    tmhalFrontEndI2cSwitch,
    tmhalFrontEndCheckIdentity,
    tmhalFrontEndReadSync,
    tmhalFrontEndReadIQ,
    tmhalFrontEndReadAgc,
    tmhalFrontEndReadTVBER,       /* = 106 - BSD*/
    tmhalFrontEndReadTCPTU,
    tmhalFrontEndReadTMCC,
    tmhalFrontEndReadCBER,        /* = 109 - DVB-T*/
    tmhalFrontEndReadConfidence,
    tmhalFrontEndReadRepImpul,
    tmhalFrontEndReadRepFreq,
    tmhalFrontEndReadCellId,
    tmhalFrontEndReadDspVersion,
    tmhalFrontEndCarrierHigh,
    tmhalFrontEndCarrierLow,
    tmhalFrontEndCarrierStep,
    tmhalFrontEndCarrierCurrent,
    tmhalFrontEndStartIQ,
    tmhalFrontEndStopIQ,
    tmhalFrontEndStartRepImpul,
    tmhalFrontEndStopRepImpul,
    tmhalFrontEndReadACI,
    tmhalFrontEndReadCCI,
    tmhalFrontEndReadLEE,
    tmhalFrontEndReadCRC,
    tmhalFrontEndReadMSE,            /* = 127 - DVB-C*/
    tmhalFrontEndReadCOEF,
    tmhalFrontEndStartEqual,
    tmhalFrontEndStopEqual,
    tmhalFrontEndReadRSSI = 138,        /*138*/
    tmhalFrontEndRFTone,
    tmhalFrontEndRFToneIndex,
    tmhalFrontEndToneSearchState,
    tmhalFrontEndNbOfUnit = 200,
    tmhalFrontEndOutOfGIEcho,            /*DVB_T*/
    tmhalFrontEndDSPDownload,
    tmhalFrontEndDSPloadFromHost,
    tmhalFrontEndDSPloadFromEEPROM,
    tmhalFrontEndEnable4k,
    tmhalFrontEndModePer,
    tmhalFrontEndGiveConfigToTuner,
    tmhalFrontEndConfigAttenuator,
    tmhalFrontEndAGCBusEnable,
    tmhalFrontEndAGCExt,
    tmhalFrontEndAGCGainIndex,
    tmhalFrontEndAGCGainValue,
    tmhalFrontEndLOFreqIndex,
    tmhalFrontEndLOFreqValue,
    tmhalFrontEndXtalMode,
    tmhalFrontEndEnableLNA,
    tmhalFrontEndCutOffIndex,
    tmhalFrontEndTunerBandwidth = 300,
    tmhalFrontEndTunerLVL,
    tmhalFrontEndBand,
    tmhalFrontEndRF,
    tmhalFrontEndID,
    tmhalFrontEndPOR,
    tmhalFrontEndReadFEL,
    tmhalFrontEndFCC,
    tmhalFrontEndTEMP,
    tmhalFrontEndRFAGC,
    tmhalFrontEndIFAGC,
    tmhalFrontEndAGCandTEMP,
    tmhalFrontEndPowerState,
    tmhalFrontEndSensibility,
    tmhalFrontEndReadCarrier,
    tmhalFrontEndBandAuto = 330,
    tmhalFrontEndManualBandWidth,
    tmhalFrontEndReadInit = 372,
    tmhalFrontEndStandardMode = 400,
    tmhalFrontEndDemodAdd = 500,
    tmhalFrontEndDemodFct,

    tmhalFrontEndMapNb,         /* for 8275AHN, 8274AHN  */
    tmhalFrontEndMapIndex,
    tmhalFrontEndMapFreq,
    tmhalFrontEndMapParam,
    tmhalFrontEndATC,                /* used by TD1316 */
    tmhalFrontEndTOP,                /* used by TD1316 */
    tmhalFrontEndReadID,
    tmhalFrontEndXTALL,
    tmhalFrontEndPLLMFACTOR,
    tmhalFrontEndPLLNFACTOR,
    tmhalFrontEndPLLPFACTOR,
    tmhalFrontEndBERDEPTH,
    tmhalFrontEndIQSWAPPED,
    tmhalFrontEndEXTOFFSET,
    tmhalFrontEndOFFSET125K,
    tmhalFrontEndIF,
    tmhalFrontEndWATCHDOG,
    tmhalFrontEndNBRETRY,
    tmhalFrontEndPULSEKILLER,
    tmhalFrontEndXTALL100PPM,
    tmhalFrontEndAGCTUN,
    tmhalFrontEndAGCPOLATUN,
    tmhalFrontEndAGCPOLAIF,
    tmhalFrontEndAGCTUNMAX,
    tmhalFrontEndAGCTUNMIN,
    tmhalFrontEndAGCIFMAX,
    tmhalFrontEndAGCIFMIN,
    tmhalFrontEndOUT1POCLK,
    tmhalFrontEndOUT1PARASER,
    tmhalFrontEndOUT1SLSB,        /* MSBLSB, configure in serial mode if  MSB or LSB first */
    tmhalFrontEndOUT1TSMODE,      /* Transport stream mode A, B, C... */
    tmhalFrontEndOUT1PARADIV,
    tmhalFrontEndOUT2POCLK,
    tmhalFrontEndOUT2PARASER,
    tmhalFrontEndOUT2SLSB,
    tmhalFrontEndOUT2TSMODE,
    tmhalFrontEndOUT2PARADIV,
    tmhalFrontEndSRMax,          /* Sat SR config */
    tmhalFrontEndSRMin,
    tmhalFrontEndSRMaxMargin,
    tmhalFrontEndSRMinMargin,
    tmhalFrontEndSEARCHRANGE,
    tmhalFrontEndPOLAAGC,
    tmhalFrontEndDeltaThreshold,
    tmhalFrontEndZIFTuner,
    tmhalFrontEndFC,                /* tmbsl8262 */
    tmhalFrontEndBBGAIN,
    tmhalFrontEndRFATT,
    tmhalFrontEndPDPLL,
    tmhalFrontEndPDZIF,
    tmhalFrontEndPDLOOPT,
    tmhalFrontEndPDRSSI,
    tmhalFrontEndPDLNA,
    tmhalFrontEndREFDIVIDER,
    tmhalFrontEndCALMANUAL,
    tmhalFrontEndCPCURSEL,
    tmhalFrontEndCPHIGH,
    tmhalFrontEndPORT1,
    tmhalFrontEndFSampling,
    tmhalFrontEndDVBMCNS,       /* TDA10023 */
    tmhalFrontEndIFMax,
    tmhalFrontEndIFMin,
    tmhalFrontEndTunMax,
    tmhalFrontEndTunMin,
    tmhalFrontEndAGCThreshold,
    tmhalFrontEndEqualType,
    tmhalFrontEndPolaPWM1,
    tmhalFrontEndPolaPWM2,
    tmhalFrontEndBERWindow,
    tmhalFrontEndClockSetRange,
    tmhalFrontEndSWDyn,
    tmhalFrontEndSWStep,
    tmhalFrontEndJQAMMode,
    tmhalFrontEndJQAMReQTS,
    tmhalFrontEndJQAMReQON,
    tmhalFrontEndJQAMGetReg5,
    tmhalFrontEndJQAMGetTSStatus,
    tmhalFrontEndJQAMGetRCStatus,
    tmhalFrontEndJQAMGetTSON,
    tmhalFrontEndCPUMP,          /* Cdm1316 */
    tmhalFrontEndGiveCfgToTuner,
    tmhalFrontEndPioInOut1,
    tmhalFrontEndPioValue1,
    tmhalFrontEndPioInOut2,
    tmhalFrontEndPioValue2,
    tmhalFrontEndPioInOut3,
    tmhalFrontEndPioValue3,
    tmhalFrontEndSetAntennaPower = 600,     /* Control GPIO1 in open-drain for Vestel application */
    tmhalFrontEndZapMode = 601,
    tmhalFrontEndChannelInfo,
    tmhalFrontEndTimeErr,
    tmhalFrontEndFreqErr,
    tmhalFrontEndEnableI2cSwitch,
    tmhalFrontEndReadFreqFilter,
    tmhalFrontEndReadChannelType,
    tmhalFrontEndReadCCIDetection,
    tmhalFrontEndReadDopplerDetection,
    tmhalFrontEndReadCumulatedHS,
    tmhalFrontEndReadACIDetection,
    tmhalFrontEndReadPKUse,
    tmhalFrontEndReadIIRTimeInter,
    tmhalFrontEndReadEchoType,
    tmhalFrontEndReadChangeDetect,
    tmhalFrontEndReadNBPath,
    tmhalFrontEndReadChannelLength,
    tmhalFrontEndEnableIQDetect,
    tmhalFrontEndIFTuner = 619,
    tmhalFrontEndIFDemod,
    tmhalFrontEndDemodReadFct,
    tmhalFrontEndAutoOffsetLock,
    tmhalFrontEndPLLSTEP,
    tmhalFrontEndBANDAUTO,
    tmhalFrontEndMANUALBANDWIDTH,
    tmhalFrontEndREADINIT,
    tmhalFrontEndISMK3,
    tmhalFrontEndDiversitySetMaster,
    tmhalFrontEndDiversitySetBridge,
    tmhalFrontEndDiversitySetSlave,
    tmhalFrontEndICMODE  = 631,
    tmhalFrontEndSACLKOUTPUTMODE,
    tmhalFrontEndBANDEXT

} tmhalFrontEndConfig_t, *ptmhalFrontEndConfig_t;

/* system function*/
typedef struct _tmhalFrontEndBslSystemFunc_t
{
    UInt32    (*SY_GetTickTime)    (void);
    UInt32    (*SY_GetTickPeriod)    (void);
    UInt32    (*SY_GetCommErrors)    (void);
    Bool    (*SY_Write)            (UInt32  uAddress,
                                 UInt32  uSubAddress,
                                 UInt32  uNbData,
                                 UInt32* pDataBuff);
    Bool    (*SY_WriteBit)        (UInt32  uAddress,
                                 UInt32  uSubAddress,
                                 UInt32  uMaskValue,
                                 UInt32  uValue);
    Bool    (*SY_Read)            (UInt32  uAddress,
                                 UInt32  uSubAddress,
                                 UInt32  uNbData,
                                 UInt32* pDataBuff);
    Bool    (*SY_SetDeviceAdd)        (UInt32 DeviceId,
                                     UInt32 NewAdd);
    Bool    (*SY_SetErrorEnable)    (Bool ErrorEnable);
    Bool    (*SY_SetUrt)            (void);
    Bool    (*SY_GetMatLog)            (char deviceName[],int WriteOrRead,UInt8 TabDevice[25][150]);
} tmhalFrontEndBslSystemFunc_t, *ptmhalFrontEndBslSystemFunc_t;


/***********************************************************************************************/
/* Those types are used with new bsl arch */

typedef struct _tmhalFrontEndbslSrvDebugFunc_t
{
    void        (*print)(const char*, ...);
} tmhalFrontEndbslSrvDebugFunc_t;


typedef struct _tmhalFrontEndbslSrvTimeFunc_t
{
    UInt32      (*Get)(void);           /* return current time value in ms */
    UInt32      (*Wait)(UInt32 t);      /* wait t ms without blocking sceduler, warning this function
                                           don't schedule others frontend instance */
} tmhalFrontEndbslSrvTimFunc_t;


typedef struct _tmhalFrontEndbslSrvMathFunc_t
{
    Float32     (*log10)(Float32 x);
    Float32     (*pow)(Float32 x, Int32 y);
} tmhalFrontEndbslSrvMathFunc_t;

typedef struct _tmhalFrontEndbslSrvTunerFunc_t
{
    UInt32      (*Init)(tmUnitSelect_t TunerUnit, ...);
    UInt32      (*Read)(tmUnitSelect_t FrontEndUnit, UInt8 uSubAddress, UInt32 uNbData, UInt8* pDataBuff);
    UInt32      (*Write)(tmUnitSelect_t FrontEndUnit, UInt8 uSubAddress, UInt32 uNbData, UInt8* pDataBuff, Bool bIndex);
    UInt32      (*SetRf)(tmUnitSelect_t FrontEndUnit, UInt32 uRf);
    UInt32      (*GetRf)(tmUnitSelect_t FrontEndUnit, UInt32* puRf);
    UInt32      (*SetPowerState)(tmUnitSelect_t FrontEndUnit, tmPowerState_t ePowerState);
    UInt32      (*SetCfg)(tmUnitSelect_t FrontEndUnit, tmhalFrontEndConfig_t uItemId, UInt32 uValue);
    UInt32      (*GetCfg)(tmUnitSelect_t FrontEndUnit, tmhalFrontEndConfig_t uItemId, UInt32* puValue);
    UInt32      (*GetRssi)(tmUnitSelect_t FrontEndUnit, Int8* pRssi, UInt32* puAgc1, UInt32* puAgc2, UInt32* puAdc);
} tmhalFrontEndbslSrvTunerFunc_t;

typedef struct _tmhalFrontEndbslSrvDemodFunc_t
{
    UInt32      (*Read)(tmUnitSelect_t FrontEndUnit, UInt8 uSubAddress, UInt32 uNbData, UInt8* pDataBuff);
    UInt32      (*Write)(tmUnitSelect_t FrontEndUnit, UInt8 uSubAddress, UInt32 uNbData, UInt8* pDataBuff);
    UInt32      (*WriteBit)(tmUnitSelect_t FrontEndUnit, UInt8 uSubAddress, UInt8 uMask, UInt8 uData);
} tmhalFrontEndbslSrvDemodFunc_t;


typedef struct _tmhalFrontEndbslSrvFunc_t
{
    tmhalFrontEndbslSrvDemodFunc_t  sDemod;
    tmhalFrontEndbslSrvTunerFunc_t  sTuner;
    tmhalFrontEndbslSrvTimFunc_t    sTime;
    tmhalFrontEndbslSrvMathFunc_t   sMath;
    tmhalFrontEndbslSrvDebugFunc_t  sDebug;
} tmhalFrontEndbslSrvFunc_t;

typedef struct _tmhalFrontEndAntennaDiv_t
{
    UInt32              nbSlave;
    tmUnitSelect_t      SlaveId[TMBSL_MAX_CHANNEL_FRONTEND];
} tmhalFrontEndAntennaDiv_t;
/***********************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* TMHALFRONTENDTYPES_H */

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/


