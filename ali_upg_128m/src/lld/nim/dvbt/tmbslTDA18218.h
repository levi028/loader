//-----------------------------------------------------------------------------
// $Header:
// (C) Copyright 2007 NXP Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of NXP Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of NXP
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than NXP Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmbslTDA18218.h
//
// DESCRIPTION:  define the object for the TDA18218
//
// DOCUMENT REF: DVP Software Coding Guidelines v1.14
//               DVP Board Support Library Architecture Specification v0.5
//
// NOTES:
//-----------------------------------------------------------------------------
//
#ifndef _TMBSL_TDA18218_H //-----------------
#define _TMBSL_TDA18218_H

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include "tmhalFrontEndtypes.h"
//#include "tmbslTunerTypes.h"
#include "tmbslTuner.h"
#include "dvp.h"

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//

#ifndef NXPFE
#ifdef __cplusplus
extern "C"
{
#endif
#endif // NXPFE

    //-----------------------------------------------------------------------------
    // Types and defines:
    //-----------------------------------------------------------------------------
    //

    typedef enum _tmTDA18218cfgIndex_t {
        tmTDA18218BOARD = 0,
        tmTDA18218STANDARD = 1,
        tmTDA18218FEINIT = 100,
        tmTDA18218IDENTITY = 102,
        tmTDA18218GETNBOFUNIT = 200,
        tmTDA18218BANDWIDTH = 300,
        tmTDA18218RFLVL,
        tmTDA18218RFBAND,
        tmTDA18218RF,
        tmTDA18218ID,
        tmTDA18218POR,
        tmTDA18218MAINLOCK,
        tmTDA18218PLRFAGC = 310,
        tmTDA18218POWERSTATE = 312,
        tmTDA18218TMMODE = 350,
        tmTDA18218TMVALUE,
        tmTDA18218RFIOConfig,
        tmTDA18218_IRCalWanted_LTviaAGC1b,
        tmTDA18218_IRCalImage_LTviaAGC1b,
        tmTDA18218_RFCalPOR_LTviaHZ,
        tmTDA18218_RFCalAV_LTviaHZ,
        tmTDA18218_SoftReset,
        tmTDA18218_RSSI,
        tmTDA18218PLMODE,
        tmTDA18218PLAGC1,
        tmTDA18218PLAGC2,
        tmTDA18218RFCALOK,
        tmTDA18218IRCALOK,
        tmTDA18218BPFILTER,
        tmTDA18218GAINTAPER,
        tmTDA18218LPFC,
        tmTDA18218FMRFN,
        tmTDA18218XTOUT,
        tmTDA18218IFLEVEL,
        tmTDA18218EXTENDEDREG,
        tmTDA18218CALPOSTDIV,
        tmTDA18218READINIT,
        tmTDA18218CALDIV,
        tmTDA18218MAINPOSTDIV,
        tmTDA18218MAINDIV,
        tmTDA18218MAPNAME,
        tmTDA18218MAPCOLUMN,
        tmTDA18218MAPROW,
        tmTDA18218MAPVALUE,
        tmTDA18218STATUSBYTE = 380,
        tmTDA18218PROTOCOLSTEP,
        tmTDA18218PROTOCOLWAIT,
        tmTDA18218STANDARDMODE = 400,
        tmTDA18218CHANNELTYPE,
        tmTDA18218DEVICETYPE,
        tmTDA18218APPLICATIONTYPE,      
        tmTDA18218IF = 517,
    } tmTDA18218cfgIndex_t;

    typedef enum _tmTDA18218PowerState_t {
        tmTDA18218PowerNormalMode,                        // Device normal mode
        tmTDA18218PowerStandbyMode,                        // Device standby mode
        tmTDA18218PowerNotSupported,                    // Not supported power mode
    } tmTDA18218PowerState_t, *ptmTDA18218PowerState_t;

    typedef enum _tmTDA18218StandardMode_t {
#ifdef NXPFE   // Multi-FE compatibility
        tmTDA18218StandardNotUsed,
#endif
        tmTDA18218DigitalChannel_6MHz,
        tmTDA18218DigitalChannel_7MHz,
        tmTDA18218DigitalChannel_8MHz,
        tmTDA18218DigitalChannel_9MHz,
        tmTDA18218StandardNotSupported,                    // Not supported standard
    } tmTDA18218StandardMode_t, *ptmTDA18218StandardMode_t;

#ifndef NXPFE
    tmErrorCode_t
        tmbslTDA18218Init(
        tmUnitSelect_t        TunerUnit,    //  I: Tuner unit number
        tmbslTuParam_t        Param         //  I: setup parameters
        );
    tmErrorCode_t
        tmbslTDA18218DeInit (
        tmUnitSelect_t        TunerUnit     //  I: Tuner unit number
        );
    tmErrorCode_t
        tmbslTDA18218GetSWVersion (
        ptmSWVersion_t        pSWVersion        //  I: Receives SW Version
        );
    tmErrorCode_t
        tmbslTDA18218SetPowerState (
        tmUnitSelect_t            TunerUnit,        //  I: Tuner unit number
        tmTDA18218PowerState_t    powerState        //  I: Power state of this device
        );
    tmErrorCode_t
        tmbslTDA18218GetPowerState (
        tmUnitSelect_t            TunerUnit,        //  I: Tuner unit number
        tmTDA18218PowerState_t    *pPowerState    //  O: Power state of this device
        );
    tmErrorCode_t
        tmbslTDA18218SetStandardMode (
        tmUnitSelect_t                    TunerUnit,        //  I: Tuner unit number
        tmTDA18218StandardMode_t        StandardMode    //  I: Standard mode of this device
        );
    tmErrorCode_t
        tmbslTDA18218GetStandardMode (
        tmUnitSelect_t                    TunerUnit,        //  I: Tuner unit number
        tmTDA18218StandardMode_t        *pStandardMode    //  O: Standard mode of this device
        );
    tmErrorCode_t
        tmbslTDA18218Write (
        tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
        UInt32              uIndex,         //  I: Start index to write
        UInt32              uNbBytes,       //  I: Number of bytes to write
        UInt32*             puBytes         //  I: Pointer on an array of bytes
        );
    tmErrorCode_t
        tmbslTDA18218WriteBit (
        tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
        UInt32              uIndex,         //  I: Start index to write
        UInt32              uBitMask,       //  I: bit mask
        UInt32              uBitValue       //  I: bit value
        );
    tmErrorCode_t
        tmbslTDA18218Read (
        tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
        UInt32              uIndex,         //  I: Start index to read
        UInt32              uNbBytes,       //  I: Number of bytes to read
        UInt32*             puBytes         //  I: Pointer on an array of bytes
        );
    tmErrorCode_t
        tmbslTDA18218SetConfig(
        tmUnitSelect_t        TunerUnit,  //  I: TunerUnit number
        UInt32                uItemId,    //  I: Identifier of the item to modify
        UInt32                uValue      //  I: Value to set for the config item
        );
    tmErrorCode_t
        tmbslTDA18218GetConfig(
        tmUnitSelect_t        TunerUnit,  //  I: Tuner unit number
        UInt32                uItemId,    //  I: Identifier of the item to modify
        UInt32*                puValue     //  I: Value to set for the config item
        );
    tmErrorCode_t
        tmbslTDA18218SetRf(
        tmUnitSelect_t        TunerUnit,  //  I: Tuner unit number
        UInt32                uRF            //  I: RF frequency in hertz
        );
    tmErrorCode_t
        tmbslTDA18218GetRf(
        tmUnitSelect_t        TunerUnit,        //  I: Tuner unit number
        UInt32*                pRF                //  O: RF frequency in hertz
        );

    tmErrorCode_t
        tmbslTDA18218ReadLO
        (
        tmUnitSelect_t            TunerUnit,        // I: Tuner unit number
        UInt32                    *uLO            // O: local oscillator frequency in hertz
        );

    tmErrorCode_t
        tmbslTDA18218GetMAINPLLParams
        (
        tmUnitSelect_t            TunerUnit,        // I: Tuner unit number
        UInt32                    uLO,            // I: local oscillator frequency in hertz
        UInt8                    *uPreScaler,    // I/O: PreScaler
        UInt8                    *uPostDiv        // I/O: PostDiv
        );
#endif // NXPFE


#ifndef NXPFE
#ifdef __cplusplus
}
#endif
#endif // NXPFE

#endif // TM<MODULE>_H //---------------
