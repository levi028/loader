//-----------------------------------------------------------------------------
// $Header:
// (C) Copyright 2001 NXP Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of NXP Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of NXP
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than NXP Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmhalFEtypes.h
//
// DESCRIPTION:  define the common structures, enum and functions needed
//               for the front-end device
//
// DOCUMENT REF: BSL DemodSat - OLB
//
// NOTES:
//-----------------------------------------------------------------------------
//
#ifndef _tmhalFEtypes_H //-----------------
#define _tmhalFEtypes_H

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include "tmtypes.h"

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//

#ifdef __cplusplus
extern "C"
{
#endif



//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//

#define TMBSL_ERR_TUNER_BASE    0x00

// standard
typedef enum _tmhalFEStandard_t {
    tmhalFEStandardDSS = 0,
    tmhalFEStandardDVBS,
    tmhalFEStandardBSD,
    tmhalFEStandardDVBT,
    tmhalFEStandardDVBC,
    tmhalFEStandardDVBH,
    tmhalFEStandardAnalogDVBT,
    tmhalFEStandardMax
} tmhalFEStandard_t, *ptmhalFEStandard_t;

// spectral inversion
typedef enum _tmhalFESpectrum_t {
    tmhalFESpectrumAuto = 0,
    tmhalFESpectrumNormal,
    tmhalFESpectrumInverted,
    tmhalFESpectralInvMax
} tmhalFESpectrum_t, *ptmhalFESpectrum_t;

// modulation
typedef enum _tmhalFEModulation_t {
    tmhalFEModulationAuto = 0,
    tmhalFEModulationBpsk,
    tmhalFEModulationQpsk,
    tmhalFEModulationQam4,
    tmhalFEModulationPsk8,
    tmhalFEModulationQam16,
    tmhalFEModulationQam32,
    tmhalFEModulationQam64,
    tmhalFEModulationQam128,
    tmhalFEModulationQam256,
    tmhalFEModulationQam512,
    tmhalFEModulationQam1024,
    tmhalFEModulationMax
} tmhalFEModulation_t, *ptmhalFEModulation_t;

// viterbi rate
typedef enum _tmhalFEDepuncRate_t {
    tmhalFEDepuncRateAuto = 0,
    tmhalFEDepuncRate12,
    tmhalFEDepuncRate23,
    tmhalFEDepuncRate34,
    tmhalFEDepuncRate45,
    tmhalFEDepuncRate56,
    tmhalFEDepuncRate67,
    tmhalFEDepuncRate78,
    tmhalFEDepuncRate89,
    tmhalFEDepuncRateMax
} tmhalFEDepuncRate_t, *ptmhalFEDepuncRate_t;

// turbo code
typedef enum _tmhalFETurboCode_t {
    tmhalFETurboCodeDisabled = 0,
    tmhalFETurboCode1,
    tmhalFETurboCode2,
    tmhalFETurboCode3,
    tmhalFETurboCode4,
    tmhalFETurboCodeMax
} tmhalFETurboCode_t, *ptmhalFETurboCode_t;

// tone burst
typedef enum _tmhalFEToneBurst_t {
    tmhalFEToneBurstDisabled = 0,
    tmhalFEToneBurstSatA,
    tmhalFEToneBurstSatB,
    tmhalFEToneBurstMax
} tmhalFEToneBurst_t, *ptmhalFEToneBurst_t;

// frequency offset
typedef enum _tmhalFERfOffset_t {
    tmhalFERfOffsetAuto = 0,
    tmhalFERfOffsetNull,
    tmhalFERfOffsetPlus125,
    tmhalFERfOffsetMinus125,
    tmhalFERfOffsetPlus166,
    tmhalFERfOffsetMinus166,
    tmhalFERfOffsetPlus333,
    tmhalFERfOffsetMinus333,
    tmhalFERfOffsetPlus500,
    tmhalFERfOffsetMinus500,
    tmhalFERfOffsetMax
} tmhalFERfOffset_t, *ptmhalFERfOffset_t;

// guard interval
typedef enum _tmhalFEGuardInterval_t {
    tmhalFEGuardIntervalAuto = 0,
    tmhalFEGuardInterval32,
    tmhalFEGuardInterval16,
    tmhalFEGuardInterval8,
    tmhalFEGuardInterval4,
    tmhalFEGuardIntervalMax
} tmhalFEGuardInterval_t, *ptmhalFEGuardInterval_t;

// fast Fourrier transform size
typedef enum _tmhalFEFft_t {
    tmhalFEFftAuto = 0,
    tmhalFEFft2K,
    tmhalFEFft8K,
    tmhalFEFft4K,
    tmhalFEFftMax
} tmhalFEFft_t, *ptmhalFEFft_t;

// hierarchy
typedef enum _tmhalFEHierarchy_t {
    tmhalFEHierarchyAuto = 0,
    tmhalFEHierarchyNo,
    tmhalFEHierarchyAlpha1,
    tmhalFEHierarchyAlpha2,
    tmhalFEHierarchyAlpha4,
    tmhalFEHierarchyMax
} tmhalFEHierarchy_t, *ptmhalFEHierarchy_t;

// priority
typedef enum _tmhalFEPriority_t {
    tmhalFEPriorityAuto = 0,
    tmhalFEPriorityHigh,
    tmhalFEPriorityLow,
    tmhalFEPriorityMax
} tmhalFEPriority_t, *ptmhalFEPriority_t;

// status of the manager function
typedef enum _tmhalFEManagerStatus_t {
    tmhalFEManagerStatusCarrierOk = 0,
    tmhalFEManagerStatusNoCarrier,
    tmhalFEManagerStatusNotCompleted,
    tmhalFEManagerStatusMax
} tmhalFEManagerStatus_t, *ptmhalFEManagerStatus_t;

// Fine (but slow) autoscan or normal autoscan
typedef enum _tmhalFEAutoScanMode_t
{
    tmhalFEAutoScanAverage = 0,
    tmhalFEAutoScanFine
} tmhalFEAutoScanMode_t;

typedef enum _tmhalFEBandType_t
{
    tmhalFEBandTypeDefault = 0,
    tmhalFEBandTypeSECAM_L,
    tmhalFEBandTypeMax
} tmhalFEBandType_t;

// config items
typedef enum _tmhalFEConfig_t {
    tmhalFEBoard = 0,
    tmhalFEStandard,
    tmhalFELPSEL = 6,
    tmhalFEInit = 100,
    tmhalFEI2cSwitch,
    tmhalFECheckIdentity,
    tmhalFEReadSync,
    tmhalFEReadIQ,
    tmhalFEReadAgc,
    tmhalFEReadTVBER,       // = 106 - BSD
    tmhalFEReadTCPTU,
    tmhalFEReadTMCC,
    tmhalFEReadCBER,        // = 109 - DVB-T
    tmhalFEReadConfidence,
    tmhalFEReadRepImpul,
    tmhalFEReadRepFreq,
    tmhalFEReadCellId,
    tmhalFEReadDspVersion,
    tmhalFECarrierHigh,
    tmhalFECarrierLow,
    tmhalFECarrierStep,
    tmhalFECarrierCurrent,
    tmhalFEStartIQ,
    tmhalFEStopIQ,
    tmhalFEStartRepImpul,
    tmhalFEStopRepImpul,
    tmhalFEReadACI,
    tmhalFEReadCCI,
    tmhalFEReadLEE,
    tmhalFEReadCRC,
    tmhalFEReadMSE,            // = 127 - DVB-C
    tmhalFEReadCOEF,
    tmhalFEStartEqual,
    tmhalFEStopEqual,
    tmhalFEReadRSSI = 138,        //138
    tmhalFENbOfUnit = 200,
    tmhalFEOutOfGIEcho,          //DVB_T
    tmhalFEDSPDownload,
    tmhalFEDSPloadFromHost,
    tmhalFEDSPloadFromEEPROM,
    tmhalFEEnable4k,
    tmhalFEModePer,
    tmhalFEGiveCfgToTuner,
    tmhalFEAGCBusEnable,
    tmhalFEAGCExt,
    tmhalFEAGCGainIndex,
    tmhalFEAGCGainValue,
    tmhalFELOFreqIndex,
    tmhalFELOFreqValue,
    tmhalFEXtalMode,
    tmhalFEEnableLNA,
    tmhalFECutOffIndex,
    tmhalFEGiveCfgToDemodNum,
    tmhalFETunerBandwidth = 300,
    tmhalFETunerLVL,
    tmhalFEBand,
    tmhalFERF,
    tmhalFEID,
    tmhalFEPOR,
    tmhalFELOCK,
    tmhalFEFCC,
    tmhalFETEMP,
    tmhalFERFAGC,
    tmhalFEIFAGC,
    tmhalFEAGCandTEMP,
    tmhalFEPowerState,
    tmhalFESensibility,
    tmhalFESRMin,
    tmhalFESRMax,
    tmhalFEStandardMode = 400,
    tmhalFESound,
    tmhalFEDemodStatus,
    tmhalFEReadAFC,
    tmhalFEReadHlock,
    tmhalFEReadVlock,
    tmhalFEReadDIFAGC,
    tmhalFEReadTIFAGC,
    tmhalFEICIdentity,
    tmhalFESetFI,
    tmhalFESetGPI0Analog,
    tmhalFEDemodHwAdd = 500,        // SW Patch to allow the TDA8275A to control Demod PIO for LNA control
    tmhalFEDemodWriteBitFct
} tmhalFEConfig_t, *ptmhalFEConfig_t;

// system function
typedef struct _tmhalFEBslSystemFunc_t
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
    Bool    (*SY_SetDeviceAdd)        (char deviceName[],
                                     UInt32 NewAdd);
    Bool    (*SY_SetErrorEnable)    (Bool ErrorEnable);
    Bool    (*SY_SetUrt)            (void);
    Bool    (*SY_GetMatLog)            (char deviceName[],int WriteOrRead,UInt8 TabDevice[25][150]);
} tmhalFEBslSystemFunc_t, *ptmhalFEBslSystemFunc_t;

#ifdef __cplusplus
}
#endif

#endif // _tmhalFEtypes_H
