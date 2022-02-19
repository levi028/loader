//-----------------------------------------------------------------------------
// $Header:
// (C) Copyright 2001 Philips Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of Philips Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of Philips
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than Philips Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmbslTuner.h
//
// DESCRIPTION:  This header file describes definitions, types, and functions
//               exported by the Board Support Library (BSL) tuner part.
//
// DOCUMENT REF: DVP Software Coding Guidelines v1.14
//               DVP Board Support Library Architecture Specification v0.5
//
// NOTES:        <File notes>
//-----------------------------------------------------------------------------
//
#ifndef _TMBSL_TUNER_H_
#define _TMBSL_TUNER_H_


//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include "tmhalFEtypes.h"

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

/* -------------------------------- types -----------------------------------------*/

/*
 * structures of this type are used by the tmdlTu device library to initialize
 * the tuner board support library component.
 */

typedef struct _tmbslTuParam_t
{
    unsigned long             uHwAddress;
    tmhalFEBslSystemFunc_t    systemFunc;
} tmbslTuParam_t, *ptmbslTuParam_t;

/*
 * a structure of this type is used to describe the capabilities of the
 * tuner
 */
typedef struct _tmbslTuConfig_t
{
    // initialisation
    tmErrorCode_t (*initFunc) (             tmUnitSelect_t tunerUnit,
                                            tmbslTuParam_t sParam);
    tmErrorCode_t (*deinitFunc) (           tmUnitSelect_t tunerUnit);
    tmErrorCode_t (*getSWVersionFunc) (     tmSWVersion_t* pSWVersion);
    tmErrorCode_t (*setPowerStateFunc) (    tmUnitSelect_t tunerUnit,
                                            tmPowerState_t powerState);
    tmErrorCode_t (*getPowerStateFunc) (     tmUnitSelect_t tunerUnit,
    tmPowerState_t* pPowerState);

    // configuration
    tmErrorCode_t (*setCfgFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32 uItemId,
                                            UInt32 uValue);
    tmErrorCode_t (*getCfgFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32 uItemId,
                                            UInt32* puValue);

    // read-write into registers
    tmErrorCode_t (*writeFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32 uAddress,
                                            UInt32 uNbData,
                                            UInt32* puData);
    tmErrorCode_t (*writeBitFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32 uAddress,
                                            UInt32 uBitMask,
                                            UInt32 uData);
    tmErrorCode_t (*readFunc) (             tmUnitSelect_t tunerUnit,
                                            UInt32 uAddress,
                                            UInt32 uNbData,
                                            UInt32* puData);
    // Tuner programming
    tmErrorCode_t (*setRfFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32 uRF);
    tmErrorCode_t (*getRfFunc) (            tmUnitSelect_t tunerUnit,
                                            UInt32* puRF);
    tmErrorCode_t (*getRssiFunc) (            tmUnitSelect_t  TunerUnit,
                                            Int8*           pRssi,
                                            UInt32*         puAgc1,
                                            UInt32*         puAgc2,
                                            UInt32*         puAdc);
} tmbslTuConfig_t, *ptmbslTuConfig_t;

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif /* _TMBSLTU_H_ */
