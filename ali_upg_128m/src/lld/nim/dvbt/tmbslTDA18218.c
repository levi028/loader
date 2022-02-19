//-----------------------------------------------------------------------------
// $Header: 
// (C) Copyright 2007-2008 NXP Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of NXP Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of NXP
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than NXP Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmbslTDA18218.c
//
// DESCRIPTION:  Function for the Hybrid Cable silicon tuner TDA18218
//
// DOCUMENT REF: 
//
// NOTES:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//

#ifdef _WIN32
#ifndef NXPFE
#include "math.h"
#include "OAIDL.h"
#include "stdlib.h"
#endif //NXPFE

#include "tmtypes.h"
#ifndef NXPFE
#include "SystemFuncURT.h"
#endif //NXPFE
#endif

#ifdef NXPFE
#include "dvp.h"
#include "tmhalFrontEndtypes.h"
#endif //NXPFE

#include "tmhalFrontEndtypes.h"

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//
#include "tmbslTuner.h"
#include "tmbslTDA18218local.h"

#if (SYS_TUN_MODULE == TDA18218)
#define TDA18218_STATIC_INSTANCE
#include "tmbslTDA18218StaticInstance.h"


#ifndef NXPFE
//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// Global data:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// Internal Prototypes:
//-----------------------------------------------------------------------------
//
tmErrorCode_t
tmTDA18218Init (
                ptmTDA18218Object_t    pObj    // I: Tuner Object
                );
Bool 
TDA18218InitTick(
                 ptmTDA18218Object_t    pObj,        // I: Tuner unit number
                 UInt16                    wTime        // I: time to wait for
                 );
Bool 
TDA18218WaitTick(
                 ptmTDA18218Object_t    pObj        // I: Tuner unit number
                 );
Bool 
tmTDA18218Write(
                ptmTDA18218Object_t        pObj,        // I: Tuner unit number
                UInt8                    uSubAddress,// I: sub address
                UInt8                    uNbData        // I: nb of data
                );
Bool 
tmTDA18218Read(
               ptmTDA18218Object_t        pObj,        // I: Tuner unit number
               UInt8                    uSubAddress,// I: sub address
               UInt8                    uNbData        // I: nb of data
               );
Bool 
tmTDA18218Wait(
               ptmTDA18218Object_t        pObj,        //  I: Tuner unit number
               UInt16                    wTime        //  I: time to wait for
               );

tmErrorCode_t
tmTDA18218SetStandardMode(
                          ptmTDA18218Object_t            pObj,        // I: tuner Object
                          tmTDA18218StandardMode_t    StandardMode// I: Standard mode of this device
                             );
tmErrorCode_t
tmTDA18218GetStandardMode(
                          ptmTDA18218Object_t            pObj,            // I: tuner Object
                          tmTDA18218StandardMode_t    *pStandardMode    // I: Standard mode of this device
                             );
tmErrorCode_t
tmTDA18218SetPowerState(
                        ptmTDA18218Object_t        pObj,        // I: tuner Object
                        tmTDA18218PowerState_t    powerState    // I: Power state of this device
                        );
tmErrorCode_t
tmTDA18218GetPowerState(
                        ptmTDA18218Object_t        pObj,        // I: tuner Object
                        tmTDA18218PowerState_t    *pPowerState// I: Power state of this device
                        );

Bool
TMBSL_CLASS tmTDA18218GetMAINPLLParams(
                                       ptmTDA18218Object_t        pObj,            // I: tuner Object
                                       UInt32                    uLO,            // I: local oscillator frequency in Hz
                                       UInt8                    *uPreScaler,    // I/O: PreScaler
                                       UInt8                    *uPostDiv        // I/O: PostDiv
                                       );

Bool
tmTDA18218ReadLO(
                 ptmTDA18218Object_t        pObj,            // I: tuner Object
                 UInt32                    *uLO            // O: local oscillator frequency in hertz
                    );


Bool
tmTDA18218CalcMAINPLL (
                       ptmTDA18218Object_t    pObj,        // I: tuner Object
                       UInt32                    uLO            // I: local oscillator frequency in Hz
                          );

Bool
tmTDA18218WriteMAINPLL (
                        ptmTDA18218Object_t        pObj,        // I: tuner Object
                        UInt32                    uLO,        // I: local oscillator frequency in Hz
                        Bool                    bI2CEnabled    // I: I2C enable flag
                        );

Bool
tmTDA18218CalcCALPLL (
                      ptmTDA18218Object_t        pObj,        // I: tuner Object
                      UInt32                    uLO            // I: local oscillator frequency in Hz
                         );

Bool
tmTDA18218SetApplicationType (
                              ptmTDA18218Object_t pObj,
                              tmTDA18218ApplicationType_t applicationType
                                 );

Bool
tmTDA18218WriteAGC1_TOP (
                         ptmTDA18218Object_t            pObj
                            );

Bool
tmTDA18218WriteAGC2_TOP (
                         ptmTDA18218Object_t            pObj,
                         UInt8                    uBPIndex
                         );

Bool
tmTDA18218ResetAGC (
                    ptmTDA18218Object_t pObj
                    );

Bool 
tmTDA18218WriteAllPlus (
                        ptmTDA18218Object_t pObj
                        );

Bool 
tmTDA18218CalPOR (
                  ptmTDA18218Object_t pObj
                  );

Bool 
tmTDA18218CalAV (
                 ptmTDA18218Object_t pObj
                 );

Bool
tmTDA18218GetRSSI (
                   ptmTDA18218Object_t            pObj,
                   UInt32*                        pRSSI
                   );

Bool 
tmTDA18218SetRFIO(
                  ptmTDA18218Object_t pObj, 
                  tmTDA18218RFIOConfig_t Index
                  );

Bool 
tmTDA18218GetRFIO(
                  ptmTDA18218Object_t pObj, 
                  tmTDA18218RFIOConfig_t *pIndex
                  );

Bool
tmTDA18218IRCalAtHighFreq(
                          ptmTDA18218Object_t    pObj
                             );

Bool
tmTDA18218IRCalibration(
                        ptmTDA18218Object_t    pObj
                        );

Bool
tmTDA18218WriteTrimIRCal(
                         ptmTDA18218Object_t    pObj
                            );

tmErrorCode_t
tmTDA18218ThermometerRead(
                          ptmTDA18218Object_t        pObj,        // I: Tuner unit number
                          Int8*                    puValue        // O: Read thermometer value
                             );


tmErrorCode_t
TMBSL_CLASS allocInstance (
                           UInt32                    DeviceUnit,    // I: Tuner unit number
                           pptmTDA18218Object_t    ppDrvObject    // I: Tuner Object
                           );
tmErrorCode_t
TMBSL_CLASS deAllocInstance (        
                             UInt32                    DeviceUnit    // I: Tuner unit number
                             );
tmErrorCode_t
TMBSL_CLASS getInstance (
                         tmUnitSelect_t            DeviceUnit,    // I: Tuner unit number
                         pptmTDA18218Object_t        ppDrvObject    // I: Tuner Object
                         );
#endif // NXPFE

#ifndef NXPFE
Bool m_tmTDA18218IRCalDebug = False;
Bool m_tmTDA18218ResetAGCDebug = False;
Bool m_tmTDA18218SetRFDebug = False;
Bool m_tmTDA18218CalAVDebug = False;
Bool m_tmTDA18218AGC1WorkAround = False;
Bool m_tmTDA18218RSSIInverted = False;
Bool m_tmTDA18218ForceES3 = False;
#endif // NXPFE

//-----------------------------------------------------------------------------
// Exported functions:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218Init:
//
// DESCRIPTION: create an instance of a TDA18218 Tuner
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TM_OK
//  
// NOTES:
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(Init)
(
 tmUnitSelect_t     TunerUnit,    //  I: Tuner unit number
 tmbslTuParam_t     Param         //  I: setup parameters
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    //----------------------
    // initialize the Object
    //----------------------
    // pObj initialization
    getInstance(TunerUnit, &pObj);

    if (pObj == Null)
    {
        ret = allocInstance(TunerUnit, &pObj);
        if (ret != TM_OK || pObj == Null)
            return TMBSL_ERR_TUNER_NOT_INITIALIZED;

        // initialize the Object by default values
        pObj->SystemFunc = Param.systemFunc;
        pObj->uHwAddress = Param.uHwAddress;
        pObj->init = True;
        return TM_OK;
    }

    // return if already initialized
    if (pObj->init == True)
    {
        return TM_OK;
    }
    else {
        // initialize the Object by default values
        pObj->SystemFunc = Param.systemFunc;
        pObj->uHwAddress = Param.uHwAddress;
        pObj->init = True;
        return TM_OK;
    }

    return TM_OK;
}


//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218DeInit:
//
// DESCRIPTION: destroy an instance of a TDA18218 Tuner
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:
//-----------------------------------------------------------------------------
//
tmErrorCode_t 
TMBSL_CLASS TMBSL_FUNC(DeInit)
(
 tmUnitSelect_t  TunerUnit     //  I: Tuner unit number
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    //----------------------
    // De-initialize the Object
    //----------------------
    // pObj initialization
    getInstance(TunerUnit, &pObj);

    // return if already initialized
    if (pObj == Null || pObj->init == False)
        return TM_OK;

    //----------------
    // Deinit the Object
    //----------------
    // initialize the Object
    if (pObj != Null)
    {
        pObj->init = False;
        ret = deAllocInstance(TunerUnit);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetSWVersion:
//
// DESCRIPTION: Return the version of this device
//
// RETURN:      TM_OK
//
// NOTES:       Values defined in the tmTDA18218local.h file
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetSWVersion)
(
 ptmSWVersion_t     pSWVersion        //  I: Receives SW Version 
 )
{
    pSWVersion->compatibilityNr = tmTDA18218_BSL_COMP_NUM;
    pSWVersion->majorVersionNr  = tmTDA18218_BSL_MAJOR_VER;
    pSWVersion->minorVersionNr  = tmTDA18218_BSL_MINOR_VER;

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218SetPowerState:
//
// DESCRIPTION: Set the power state of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(SetPowerState)
(
 tmUnitSelect_t            TunerUnit,        //  I: Tuner unit number
 tmTDA18218PowerState_t    powerState        //  I: Power state of this device
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // set power state
    return tmTDA18218SetPowerState(pObj, powerState);
}

//-------------------------------------------------------------------------------------
// FUNCTION:    TDA18218SetPowerState:
//
// DESCRIPTION: Set the power state of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218SetPowerState(
                                    ptmTDA18218Object_t        pObj,        // I: tuner Object
                                    tmTDA18218PowerState_t    powerState    // I: Power state of this device
                                    )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    switch (powerState)
    {
    case tmTDA18218PowerNormalMode:                    
        // set SM bit
        pObj->I2CMap.uBx17.bF.SM = 0x00;
        break;

    case tmTDA18218PowerStandbyMode:
        // set SM bit
        pObj->I2CMap.uBx17.bF.SM = 0x01;
        break;

    default:
        // power state not supported
        return TM_ERR_NOT_SUPPORTED;
    }

    // store powerstate 
    pObj->curPowerState = powerState;

    if (tmTDA18218Write(pObj,0x17,1) != True)    
        return False;

    return TM_OK;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetPowerState:
//
// DESCRIPTION: Get the power state of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetPowerState)
(
 tmUnitSelect_t                TunerUnit,        //  I: Tuner unit number
 tmTDA18218PowerState_t    *pPowerState    //  O: Power state of this device
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // get power state
    return tmTDA18218GetPowerState(pObj, pPowerState);
}

//-------------------------------------------------------------------------------------
// FUNCTION:    TDA18218GetPowerState:
//
// DESCRIPTION: Get the power state of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218GetPowerState(
                                    ptmTDA18218Object_t        pObj,        // I: tuner Object
                                    tmTDA18218PowerState_t    *pPowerState// I: Power state of this device
                                    )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    //----------------------
    // get the value
    //----------------------
    if (pObj->I2CMap.uBx17.bF.SM == 0x00)
        *pPowerState = tmTDA18218PowerNormalMode;
    else
        *pPowerState = tmTDA18218PowerStandbyMode;

    return TM_OK;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218SetStandardMode:
//
// DESCRIPTION: Set the standard mode of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(SetStandardMode)
(
 tmUnitSelect_t                    TunerUnit,        //  I: Tuner unit number
 tmTDA18218StandardMode_t        StandardMode    //  I: Standard mode of this device
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // set standard mode
    return tmTDA18218SetStandardMode(pObj, StandardMode);
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218SetStandardMode:
//
// DESCRIPTION: Set the standard mode of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218SetStandardMode(
                                      ptmTDA18218Object_t            pObj,        // I: tuner Object
                                      tmTDA18218StandardMode_t    StandardMode//  I: Standard mode of this device
                                      )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;


    // set uIF & Std
    switch (StandardMode)
    {
    case tmTDA18218DigitalChannel_6MHz:
        pObj->ChannelType = tmTDA18218DigitalChannel;
        pObj->I2CMap.uBx1B.bF.pdAGC2a = 1;
        pObj->I2CMap.uBx18.bF.pdAGC2b = 0;
        pObj->I2CMap.uBx18.bF.PD_RFAGC_Det = 1;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4 = 0;
        pObj->I2CMap.uBx17.bF.PD_RFAGC_Ifout = 0;
        pObj->I2CMap.uBx2D.bF.RFcal_modeOFF = 1;
        pObj->I2CMap.uBx1B.bF.LP_Fc = 0;
        pObj->Config.uIF = 3000000;//-35000;
        break;

    case tmTDA18218DigitalChannel_7MHz:
        pObj->ChannelType = tmTDA18218DigitalChannel;
        pObj->I2CMap.uBx1B.bF.pdAGC2a = 1;
        pObj->I2CMap.uBx18.bF.pdAGC2b = 0;
        pObj->I2CMap.uBx18.bF.PD_RFAGC_Det = 1;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4 = 0;
        pObj->I2CMap.uBx17.bF.PD_RFAGC_Ifout = 0;
        pObj->I2CMap.uBx2D.bF.RFcal_modeOFF = 1;
        pObj->I2CMap.uBx1B.bF.LP_Fc = 1; //0;
        pObj->Config.uIF = 5000000;
#ifdef DVBT
        pObj->Config.uIF = 3500000;//-35000;
#endif
        break;

    case tmTDA18218DigitalChannel_8MHz:
        pObj->ChannelType = tmTDA18218DigitalChannel;
        pObj->I2CMap.uBx1B.bF.pdAGC2a = 1;
        pObj->I2CMap.uBx18.bF.pdAGC2b = 0;
        pObj->I2CMap.uBx18.bF.PD_RFAGC_Det = 1;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4 = 0;
        pObj->I2CMap.uBx17.bF.PD_RFAGC_Ifout = 0;
        pObj->I2CMap.uBx2D.bF.RFcal_modeOFF = 1;
        pObj->I2CMap.uBx1B.bF.LP_Fc =  2; // 1; // 2;
        pObj->Config.uIF = 5000000;
#ifdef DVBT
        pObj->Config.uIF = 4000000;	// -35000;
#endif
        break;

    case tmTDA18218DigitalChannel_9MHz:
        pObj->ChannelType = tmTDA18218DigitalChannel;
        pObj->I2CMap.uBx1B.bF.pdAGC2a = 1;
        pObj->I2CMap.uBx18.bF.pdAGC2b = 0;
        pObj->I2CMap.uBx18.bF.PD_RFAGC_Det = 1;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3 = 0;
        pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4 = 0;
        pObj->I2CMap.uBx17.bF.PD_RFAGC_Ifout = 0;
        pObj->I2CMap.uBx2D.bF.RFcal_modeOFF = 1;
        pObj->I2CMap.uBx1B.bF.LP_Fc = 3;
        pObj->Config.uIF = 5000000;
        break;

    default:
        // standard mode not supported
        return TM_ERR_NOT_SUPPORTED;
    }

    // write standard
    if (tmTDA18218Write(pObj,0x1A,3) != True)    
        return False;
    if (tmTDA18218Write(pObj,0x17,2) != True)    
        return False;
    if (tmTDA18218Write(pObj,0x2D,1) != True)    
        return False;

    // update standard mode 
    pObj->StandardMode = StandardMode;

    // update channel type
    pObj->ChannelType = tmTDA18218DigitalChannel;

    return TM_OK;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetStandardMode:
//
// DESCRIPTION: Get the standard mode of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetStandardMode)
(
 tmUnitSelect_t                    TunerUnit,        //  I: Tuner unit number
 tmTDA18218StandardMode_t        *pStandardMode    //  O: Standard mode of this device
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // get standard mode
    return tmTDA18218GetStandardMode(pObj, pStandardMode);
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetStandardMode:
//
// DESCRIPTION: Get the standard mode of this device.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218GetStandardMode
(
 ptmTDA18218Object_t            pObj,            // I: tuner Object
 tmTDA18218StandardMode_t    *pStandardMode    //  O: Standard mode of this device
 )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_BAD_PARAMETER;

    //----------------------
    // get the value
    //----------------------

    *pStandardMode = pObj->StandardMode;

    return TM_OK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218Write:
//
// DESCRIPTION: Write in the Tuner.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TMBSL_ERR_IIC_ERR
//              TM_OK
//
// NOTES:      uIndex: IN parameter containing the HW Sub Address as specified in 
//               the datasheet (i.e. looks like 0xX0)      
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(Write)
(
 tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
 UInt32              uIndex,         //  I: Start index to write
 UInt32              uNbBytes,       //  I: Number of bytes to write
 UInt32*             puBytes         //  I: Pointer on an array of bytes
 )
{

    UInt32    uCounter;
    UInt8*    pI2CMap;

    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // test the parameters. uIndex is: 0x0X
    if ((uIndex + uNbBytes) > TDA18218_NB_BYTES)
        return TMBSL_ERR_TUNER_BAD_PARAMETER;

    // pI2CMap initialization
    pI2CMap = &(pObj->I2CMap.uBx00.ID_byte) + uIndex;

    //--------------
    // set the value
    //--------------    
    // save the values written in the Tuner
    for (uCounter = 0; uCounter < uNbBytes; uCounter++)
    {
        *pI2CMap = (UInt8)(puBytes[uCounter]);
        pI2CMap ++;
    }

    // write in the Tuner
    if (tmTDA18218Write(pObj,(UInt8)(uIndex),(UInt8)(uNbBytes)) != True)
        return TMBSL_ERR_IIC_ERR;

    return TM_OK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218WriteBit:
//
// DESCRIPTION: Write in the Tuner.
//
// RETURN:      TM_ERR_NOT_SUPPORTED
//
// NOTES:
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(WriteBit)
(
 tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
 UInt32              uIndex,         //  I: Start index to write
 UInt32              uBitMask,       //  I: bit mask
 UInt32              uBitValue       //  I: bit value
 )
{
    return TM_ERR_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218Read:
//
// DESCRIPTION: Read in the Tuner.
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TMBSL_ERR_TUNER_BAD_PARAMETER
//              TMBSL_ERR_IIC_ERR
//              TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(Read)
(
 tmUnitSelect_t      TunerUnit,      //  I: Tuner unit number
 UInt32              uIndex,         //  I: Start index to read
 UInt32              uNbBytes,       //  I: Number of bytes to read
 UInt32*             puBytes         //  I: Pointer on an array of bytes
 )
{
    UInt32    uCounter;
    UInt8*    pI2CMap;

    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // test the parameters
    if (uNbBytes > TDA18218_NB_BYTES)
        return TMBSL_ERR_TUNER_BAD_PARAMETER;

    // test uIndex
    if (uIndex == (UInt32)(-1))
        uIndex = 0;

    // pI2CMap initialization
    pI2CMap = &(pObj->I2CMap.uBx00.ID_byte) + uIndex;

    //--------------
    // get the value
    //--------------
    // read from the Tuner
    if (tmTDA18218Read(pObj,(UInt8)(uIndex),(UInt8)(uNbBytes)) != True)
        return TMBSL_ERR_IIC_ERR;

    // copy readen values to puBytes
    for (uCounter = 0; uCounter < uNbBytes; uCounter++)
    {
        *puBytes = (UInt32)(*pI2CMap);
        pI2CMap ++;
        puBytes ++;
    }
    return TM_OK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218SetConfig:
//
// DESCRIPTION: Set the Config of the TDA18218
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_ERR_NOT_SUPPORTED
//              TMBSL_ERR_TUNER_BAD_PARAMETER
//              TM_OK
//
// NOTES:
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(SetConfig)
(
 tmUnitSelect_t     TunerUnit,  //  I: TunerUnit number
 UInt32             uItemId,    //  I: Identifier of the item to modify
 UInt32             uValue      //  I: Value to set for the Config item
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    //--------------
    // set the value
    //--------------
    switch((tmTDA18218cfgIndex_t)uItemId)
    {
#ifdef DVBT
    case tmTDA18218BOARD:
        tmTDA18218SetApplicationType(pObj, tmTDA18218HybridApplication);
        //break;
#endif

    case tmTDA18218FEINIT:        
        tmTDA18218Init(pObj);
        tmTDA18218SetRFIO(pObj, tmTDA18218Master_RFin_LTOonLT);
        break;

    case tmTDA18218RF:
        // set RF frequency 
        TMBSL_FUNC(SetRf)(pObj->eUnit, pObj->Config.uRF);
        break;

    case tmTDA18218IF:
        // set IF frequency 
        pObj->Config.uIF = uValue;    
        break;

        //case tmTDA18218TMMODE:
        //// set TMMODE
        //pObj->Config.uTMMODE = uValue;
        //// set TM_ON
        //pObj->I2CMap.TM_ON = (UInt8)uValue;
        //// write byte THERMO_BYTE
        //if (tmTDA18218Write(pObj,0x01,1) != True)
        //return TMBSL_ERR_IIC_ERR;
        //break;

        //case tmTDA18218PLMODE:
        //// set uPLMODE
        //pObj->Config.uPLMODE = uValue;
        //// set Dis_Power_level
        //pObj->I2CMap.Dis_Power_level = (UInt8)uValue;
        //// write byte EASY_PROG_BYTE_1
        //if (tmTDA18218Write(pObj,0x03,1) != True)
        //return TMBSL_ERR_IIC_ERR;
        //break;

    case tmTDA18218POWERSTATE:
        tmTDA18218SetPowerState (pObj, (tmTDA18218PowerState_t)uValue);
        break;

    case tmTDA18218STANDARDMODE:
        tmTDA18218SetStandardMode (pObj, (tmTDA18218StandardMode_t)uValue);
        break;

    case tmTDA18218CHANNELTYPE:
        pObj->ChannelType = (tmTDA18218ChannelType_t)uValue;
        break;

    case tmTDA18218DEVICETYPE:
        pObj->DeviceType = (tmTDA18218DeviceType_t)uValue;
        break;

    case tmTDA18218APPLICATIONTYPE:
        tmTDA18218SetApplicationType(pObj, (tmTDA18218ApplicationType_t)uValue);
        break;

    case tmTDA18218RFIOConfig:
        tmTDA18218SetRFIO(pObj, (tmTDA18218RFIOConfig_t)uValue);
        break;

    case tmTDA18218_IRCalWanted_LTviaAGC1b:
        break;

    case tmTDA18218_IRCalImage_LTviaAGC1b:
        break;

    case tmTDA18218_RFCalPOR_LTviaHZ:
        break;

    case tmTDA18218_RFCalAV_LTviaHZ:
        break;

    case tmTDA18218_SoftReset:
        pObj->I2CMap.uBx19.bF.hc_mode = 5;
        if (tmTDA18218Write(pObj,0x19,1) != True)    return TMBSL_ERR_IIC_ERR;
        break;

    case tmTDA18218_RSSI:
        //pObj->I2CMap.uBx19.bF.hc_mode = 6;
        //pObj->I2CMap.pdAGC2_DETECT4 = 1;
        //pObj->I2CMap.AGC_On = 0;
        //pObj->I2CMap.RFcal_modeOFF = 0;
        //pObj->I2CMap.H3H5_Gain_3dB = 0;
        //if (tmTDA18218Write(pObj,0x19,1) != True)    return TMBSL_ERR_IIC_ERR;
        //if (tmTDA18218Write(pObj,0x1C,1) != True)    return TMBSL_ERR_IIC_ERR;
        //if (tmTDA18218Write(pObj,0x2D,1) != True)    return TMBSL_ERR_IIC_ERR;
        //if (tmTDA18218Write(pObj,0x1A,1) != True)    return TMBSL_ERR_IIC_ERR;
        break;

    case tmTDA18218READINIT:
        // read bytes 0x00 to 0x3A
        //if (tmTDA18218Read (pObj,0x00,59) != True)
        //    return TMBSL_ERR_IIC_ERR;

        if (uValue == 1)
        {
            // initialize device
            if(tmTDA18218Init(pObj) == TMBSL_ERR_IIC_ERR)
                return TMBSL_ERR_IIC_ERR;
        }
        break;

    default:
        return TM_FALSE;
    }

    return TM_OK;
}


//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetConfig:
//
// DESCRIPTION: Get the Config of the TDA18218
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetConfig)
(
 tmUnitSelect_t     TunerUnit,  //  I: Tuner unit number
 UInt32             uItemId,    //  I: Identifier of the item to get value
 UInt32*            puValue     //  I: Address of the variable to output the Config item value
 )
{
    //UInt32 uCounter = 0;

    ptmTDA18218Object_t        pObj = Null;
    //UInt32                    uByte=0; 
    tmErrorCode_t            ret  = TM_OK;
    //------------------------------
    // test input parameters
    //------------------------------
    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    //--------------
    // get the value
    //--------------
    switch((tmTDA18218cfgIndex_t)uItemId)
    {
    case tmTDA18218BOARD:
        // get board
        *puValue = pObj->Config.uBoard;
        break;

    case tmTDA18218STATUSBYTE:
        // read bytes 0x00 to 0x3A
       // if (tmTDA18218Read(pObj,0x00,59) != True)
       //     return TMBSL_ERR_IIC_ERR;
        break;

    case tmTDA18218RF:
        // get RF frequency
        *puValue = pObj->Config.uRF;
        break;

    case tmTDA18218IF:
        // get IF frequency
        *puValue = pObj->Config.uIF;
        break;

    case tmTDA18218_RSSI:
        // get RSSI
        tmTDA18218GetRSSI(pObj, puValue);
        break;

#ifdef DVBT
    //case tmhalRFCALOK:
    //case tmhalIRCALOK:
#endif
    case tmTDA18218MAINLOCK:
        // get MAINLOCK value
        *puValue = pObj->I2CMap.uBx01.bF.LO_Lock;
        break;

    case tmTDA18218POWERSTATE:
        // get POWERSTATE
        tmTDA18218GetPowerState(pObj, (ptmTDA18218PowerState_t)puValue);
        break;

    case tmTDA18218STANDARDMODE:
        // get STANDARDMODE
        tmTDA18218GetStandardMode(pObj, (ptmTDA18218StandardMode_t)puValue);
        break;

    case tmTDA18218CHANNELTYPE:
        *puValue = (UInt32)pObj->ChannelType;
        break;

    case tmTDA18218DEVICETYPE:
        *puValue = (tmTDA18218DeviceType_t)pObj->DeviceType;
        break;

    case tmTDA18218APPLICATIONTYPE:
        *puValue = (tmTDA18218ApplicationType_t)pObj->ApplicationType;
        break;

    case tmTDA18218RFIOConfig:
        // get RFIO
        tmTDA18218GetRFIO(pObj, (ptmTDA18218RFIOConfig_t)puValue);
        break;

    default:
        return TM_ERR_BAD_PARAMETER;
    }

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218SetRf:
//
// DESCRIPTION: Calculate i2c I2CMap & write in TDA18218
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TMBSL_ERR_TUNER_BAD_PARAMETER
//              TMBSL_ERR_IIC_ERR
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(SetRf)
(
 tmUnitSelect_t        TunerUnit,  //  I: Tuner unit number
 UInt32                uRF            //  I: RF frequency in hertz
 )
{    
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;
    UInt32 uLO;

    //------------------------------
    // test input parameters
    //------------------------------
    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // Config.uRF update
    pObj->Config.uRF = uRF;

    uLO = uRF + pObj->Config.uIF;

    if ((uRF > 170000000) && (uRF <= 600000000))
    {
        pObj->I2CMap.uBx29.bF.IR_Freq = 2;
    }
    else if ((uRF > 600000000) && (uRF <= 1002000000))
    {
            pObj->I2CMap.uBx29.bF.IR_Freq = 3;        
    }
    else
    {
        pObj->I2CMap.uBx29.bF.IR_Freq = 0;
    }
    if (tmTDA18218Write(pObj,0x29,1) != True)    return TM_FALSE;

    // Write MAIN PLL
    tmTDA18218WriteMAINPLL(pObj, uLO, True);

    // Band pass filters

    if ((uLO <=188000000))  {pObj->I2CMap.uBx1A.bF.BP_Filter = 3;}
    else if ((188000000< uLO) && (uLO <=253000000))  {pObj->I2CMap.uBx1A.bF.BP_Filter = 4;}
    else if ((253000000< uLO) && (uLO <=343000000))  {pObj->I2CMap.uBx1A.bF.BP_Filter = 5;}
    else if ((343000000< uLO))  {pObj->I2CMap.uBx1A.bF.BP_Filter = 6;}
    if (tmTDA18218Write(pObj,0x1A,1) != True)    return TM_FALSE;

    if (tmTDA18218WriteAGC2_TOP(pObj, pObj->I2CMap.uBx1A.bF.BP_Filter)!= True)    return TM_FALSE;

    // AGC2 fast reset

    pObj->I2CMap.uBx24.bF.AGC2_bypass = 1;
    if (tmTDA18218Write(pObj,0x24,1) != True)    return TM_FALSE;
    tmTDA18218Wait(pObj, 7);
    pObj->I2CMap.uBx24.bF.AGC2_bypass = 0;
    if (tmTDA18218Write(pObj,0x24,1) != True)    return TM_FALSE;

    // return value
    return TM_OK;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetRf:
//
// DESCRIPTION: Get the frequency programmed in the tuner
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       The value returned is the one stored in the Object
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetRf)
(
 tmUnitSelect_t        TunerUnit,        //  I: Tuner unit number
 UInt32*                pRF                //  O: RF frequency in hertz
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //------------------------------
    // test input parameters
    //------------------------------
    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    //----------------------
    // get the value
    //----------------------
    // the read function can't be used
    *pRF = pObj->Config.uRF;

    return TM_OK;
}

//-------------------------------------------------------------------------------------
// FUNCTION:   tmbslTDA18218GetRSSI:
//
// DESCRIPTION: Launch and read the RSSI
//
// RETURN:      true or false
//
// NOTES:  
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetRSSI)
(
 tmUnitSelect_t        TunerUnit,
 UInt32*                pRSSI)

{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //------------------------------
    // test input parameters
    //------------------------------
    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    //----------------------
    // get the value
    //----------------------
    if (tmTDA18218GetRSSI(pObj, pRSSI) != True)
        return TMBSL_ERR_IIC_ERR;

    return TM_OK;
}

//-----------------------------------------------------------------------------
// Internal functions:
//-----------------------------------------------------------------------------
//
#ifndef NXPFE
//-----------------------------------------------------------------------------
// FUNCTION:    allocInstance:
//
// DESCRIPTION: allocate new instance
//
// RETURN:      
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS allocInstance (
                           UInt32                    DeviceUnit,    // I: Device unit number
                           pptmTDA18218Object_t    ppDrvObject    // I: Device Object
                           )
{ 
    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (DeviceUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // return value
    return getInstance(DeviceUnit, ppDrvObject);
}

//-----------------------------------------------------------------------------
// FUNCTION:    deAllocInstance:
//
// DESCRIPTION: deallocate instance
//
// RETURN:      always TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS deAllocInstance (
                             UInt32                    DeviceUnit    // I: Device unit number
                             )
{     
    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (DeviceUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // return value
    return TM_OK;
}

//-----------------------------------------------------------------------------
// FUNCTION:    getInstance:
//
// DESCRIPTION: get the instance
//
// RETURN:      always True
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS getInstance (
                         tmUnitSelect_t            DeviceUnit,    // I: Device unit number
                         pptmTDA18218Object_t        ppDrvObject    // I: Device Object
                         )
{     
    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (DeviceUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // get instance
    *ppDrvObject = &gTDA18218Instance[DeviceUnit];

    // return value
    return TM_OK;
}
#endif // NXPFE

//-----------------------------------------------------------------------------
// FUNCTION:    TDA18218Init:
//
// DESCRIPTION: initialization of the Tuner
//
// RETURN:      always True
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218Init (
                            ptmTDA18218Object_t    pObj
                            )
{     
    //tmErrorCode_t            ret  = TM_OK;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // tmTDA18218DigitalOnlyApplication
    // force channel type to digital
    pObj->ChannelType = tmTDA18218DigitalChannel;

    // write all initialization bytes
    pObj->I2CMap.uBx05.Read_byte_5 = 0x00;            // 0x05 byte
    pObj->I2CMap.uBx06.Read_byte_6 = 0x00;            // 0x06 byte
    pObj->I2CMap.uBx07.PSM_byte_1 = 0xD0;                // 0x07 byte
    pObj->I2CMap.uBx08.Main_Divider_byte_1 = 0x00;    // 0x08 byte
    pObj->I2CMap.uBx09.Main_Divider_byte_2 = 0x40;    // 0x09 byte
    pObj->I2CMap.uBx0A.Main_Divider_byte_3 = 0x00;    // 0x0A byte
    pObj->I2CMap.uBx0B.Main_Divider_byte_4 = 0x00;    // 0x0B byte
    pObj->I2CMap.uBx0C.Main_Divider_byte_5 = 0x07;    // 0x0C byte
    pObj->I2CMap.uBx0D.Main_Divider_byte_6 = 0xFF;    // 0x0D byte
    pObj->I2CMap.uBx0E.Main_Divider_byte_7 = 0x84;    // 0x0E byte
    pObj->I2CMap.uBx0F.Main_Divider_byte_8 = 0x09;    // 0x0F byte
    pObj->I2CMap.uBx10.Cal_Divider_byte_1 = 0x00;        // 0x10 byte
    pObj->I2CMap.uBx11.Cal_Divider_byte_2 = 0x13;        // 0x11 byte
    pObj->I2CMap.uBx12.Cal_Divider_byte_3 = 0x00;        // 0x12 byte
    pObj->I2CMap.uBx13.Cal_Divider_byte_4 = 0x00;        // 0x13 byte
    pObj->I2CMap.uBx14.Cal_Divider_byte_5 = 0x01;        // 0x14 byte
    pObj->I2CMap.uBx15.Cal_Divider_byte_6 = 0x84;        // 0x15 byte
    pObj->I2CMap.uBx16.Cal_Divider_byte_7 = 0x09;        // 0x16 byte
    pObj->I2CMap.uBx17.Power_Down_byte_1 = 0xF0;        // 0x17 byte
    pObj->I2CMap.uBx18.Power_Down_byte_2 = 0x19;        // 0x18 byte
    pObj->I2CMap.uBx19.Xtout_Byte = 0x01;	//modify for power saving, close the xout20090615,	0x0A;                // 0x19 byte

    switch(pObj->DeviceType)
    {
    default:
    case tmTDA18218MasterDeviceWithoutLT:
        break;

    case tmTDA18218MasterDeviceWithLT:
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 0x00;

        break;
    }

    pObj->I2CMap.uBx1A.IF_Byte_1 = 0xAE;	//0x8E;                // 0x1A byte
    pObj->I2CMap.uBx1B.IF_Byte_2 = 0x69;                // 0x1B byte
    pObj->I2CMap.uBx1C.AGC2b_byte = 0x98;                // 0x1C byte
    pObj->I2CMap.uBx1D.PSM_byte_2 = 0x00;                // 0x1D byte
    pObj->I2CMap.uBx1E.PSM_byte_3 = 0x00;                // 0x1E byte
    pObj->I2CMap.uBx1F.PSM_byte_4 = 0x58;                // 0x1F byte Killer_on PRCR CFR_STB#40
    pObj->I2CMap.uBx20.AGC1_byte_1 = 0x10;            // 0x20 byte
    pObj->I2CMap.uBx21.AGC1_byte_2 = 0x40;            // 0x21 byte
    pObj->I2CMap.uBx22.AGC1_byte_3 = 0x8C;            // 0x22 byte
    pObj->I2CMap.uBx23.AGC2_byte_1 = 0x00;            // 0x23 byte
    pObj->I2CMap.uBx24.AGC2_byte_2 = 0x0C;            // 0x24 byte
    pObj->I2CMap.uBx25.Analog_AGC_byte = 0x48;        // 0x25 byte
    pObj->I2CMap.uBx26.RC_byte = 0x85;                // 0x26 byte
    pObj->I2CMap.uBx27.RSSI_byte = 0xC9;                // 0x27 byte
    pObj->I2CMap.uBx28.IR_CAL_byte_1 = 0xA7;            // 0x28 byte
    pObj->I2CMap.uBx29.IR_CAL_byte_2 = 0x00;            // 0x29 byte
    pObj->I2CMap.uBx2A.IR_CAL_byte_3 = 0x00;            // 0x2A byte
    pObj->I2CMap.uBx2B.IR_CAL_byte_4 = 0x00;            // 0x2B byte
    pObj->I2CMap.uBx2C.RF_Cal_byte_1 = 0x30;            // 0x2C byte
    pObj->I2CMap.uBx2D.RF_Cal_byte_2 = 0x81;            // 0x2D byte
    pObj->I2CMap.uBx2E.RF_Cal_byte_3 = 0x80;            // 0x2E byte
    pObj->I2CMap.uBx2F.RF_Cal_byte_4 = 0x00;            // 0x2F byte
    pObj->I2CMap.uBx30.RF_Cal_byte_5 = 0x39;            // 0x30 byte
    pObj->I2CMap.uBx31.RF_Cal_byte_6 = 0x00;            // 0x31 byte
    pObj->I2CMap.uBx32.RF_Cal_byte_7 = 0x8A;            // 0x32 byte
    pObj->I2CMap.uBx33.RF_Cal_byte_8 = 0x00;            // 0x33 byte
    pObj->I2CMap.uBx34.RF_Cal_byte_9 = 0x00;            // 0x34 byte
    pObj->I2CMap.uBx35.RF_Cal_byte_10 = 0x00;            // 0x35 byte
    pObj->I2CMap.uBx36.RF_Cal_RAM_byte_1 = 0x00;        // 0x36 byte
    pObj->I2CMap.uBx37.RF_Cal_RAM_byte_2 = 0x00;        // 0x37 byte
    pObj->I2CMap.uBx38.Debug_byte = 0x00;                // 0x38 byte
    pObj->I2CMap.uBx39.Fmax_byte_1 = 0xF6;            // 0x39 byte
    pObj->I2CMap.uBx3A.Fmax_byte_2 = 0xF6;            // 0x3A byte
   // if (tmTDA18218Write(pObj, 0x05, 27) != True)    return TM_FALSE;
	if (tmTDA18218Write(pObj, 0x05, 10) != True)    return TM_FALSE;
if (tmTDA18218Write(pObj, 0x0F, 10) != True)    return TM_FALSE;
if (tmTDA18218Write(pObj, 0x19, 7) != True)    return TM_FALSE;
    if (tmTDA18218Write(pObj, 0x22, 1) != True)    return TM_FALSE;
    if (tmTDA18218Write(pObj, 0x22, 1) != True)    return TM_FALSE;
    //if (tmTDA18218Write(pObj, 0x24, 23) != True)    return TM_FALSE;
	if (tmTDA18218Write(pObj, 0x24, 10) != True)    
if (tmTDA18218Write(pObj, 0x2E, 10) != True)    
if (tmTDA18218Write(pObj, 0x38, 3) != True)

    // launch msm
    pObj->I2CMap.uBx05.bF.msm_rst = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return TM_FALSE;
    pObj->I2CMap.uBx05.bF.msm_rst = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return TM_FALSE;
    pObj->I2CMap.uBx05.bF.launch_msm = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return TM_FALSE;
    pObj->I2CMap.uBx05.bF.launch_msm = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return TM_FALSE;

    // call IR calibration function
    tmTDA18218IRCalibration(pObj);

    // call reset agc function
    tmTDA18218ResetAGC(pObj);

    // return value
    return TM_OK;
}


//-----------------------------------------------------------------------------
// FUNCTION:    TDA18218InitTick
//
// DESCRIPTION: this function will delay for the number of millisecond
//
// RETURN:      nothing
//
// NOTES:       
//-----------------------------------------------------------------------------
//
Bool 
TMBSL_CLASS tmTDA18218InitTick(
                               ptmTDA18218Object_t    pObj,        // I: Tuner unit number
                               UInt16                    wTime        // I: time to wait for
                                  )
{
    UInt32 uCurrentTick;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // get current tick
    uCurrentTick = _SYSTEMFUNC.SY_GetTickTime();

    // Calculate end tick
    pObj->uTickEnd = (UInt32)wTime;
    pObj->uTickEnd += _SYSTEMFUNC.SY_GetTickPeriod()/2;
    pObj->uTickEnd /= _SYSTEMFUNC.SY_GetTickPeriod();
    pObj->uTickEnd += uCurrentTick;

    // always add 1 because of rounding issue
    if (wTime)
        pObj->uTickEnd++;

    // test overflow
    if (pObj->uTickEnd < uCurrentTick)
        return False;
    else
        return True;
}

//-----------------------------------------------------------------------------
// FUNCTION:    TDA18218WaitTick
//
// DESCRIPTION: this function will block for the number of millisecond
//
// RETURN:      True if time has elapsed else False
//
// NOTES:       
//-----------------------------------------------------------------------------
//
Bool 
TMBSL_CLASS tmTDA18218WaitTick(
                               ptmTDA18218Object_t    pObj        // I: Tuner unit number
                                  )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // test if time has elapsed
    if (_SYSTEMFUNC.SY_GetTickTime() >= pObj->uTickEnd)
        return True;
    else
        return False;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    TDA18218Write
//
// DESCRIPTION: This function writes I2C data in the Tuner
//
// RETURN:      True or False
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
Bool 
TMBSL_CLASS tmTDA18218Write(
                            ptmTDA18218Object_t    pObj,        // I: Tuner unit number
                            UInt8                    uSubAddress,// I: sub address
                            UInt8                    uNbData        // I: nb of data
                            )
{
    Bool    I2CCommResult = TM_FALSE;

    UInt8    uCounter;
    UInt8*    pI2CMap;
    UInt32    WriteBuffer[TDA18218_NB_BYTES] = {0};
    UInt32*    pWriteBuffer;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // pI2CMap & pWriteBuffer initialization
    pI2CMap = &(pObj->I2CMap.uBx00.ID_byte);
    pWriteBuffer = WriteBuffer;

    // copy I2CMap data in WriteBuffer
    for (uCounter = 0; uCounter < TDA18218_NB_BYTES; uCounter++)
    {
        switch(uCounter)
        {
        case 0x1B:
            *pWriteBuffer = (UInt32)((*pI2CMap & 0xBF) | (pObj->Config.uRF_IF_Det << 6));
            break;        
        default:
            *pWriteBuffer = (UInt32)(*pI2CMap);
            break;
        }        
        pWriteBuffer ++;
        pI2CMap ++;
    }

    // write data in the Tuner
    I2CCommResult = _SYSTEMFUNC.SY_Write(pObj->uHwAddress, 
        uSubAddress,
        uNbData,
        &(WriteBuffer[uSubAddress]) );

    // return value
    return I2CCommResult;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218Read
//
// DESCRIPTION: This function reads I2C data from the Tuner
//
// RETURN:      True or False
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
Bool 
TMBSL_CLASS tmTDA18218Read(
                           ptmTDA18218Object_t    pObj,        // I: Tuner unit number
                           UInt8                    uSubAddress,// I: sub address
                           UInt8                    uNbData        // I: nb of data
                              )
{
    Bool    I2CCommResult = TM_FALSE;

    UInt8    uCounter;
    UInt8*    pI2CMap;
    UInt32    ReadBuffer[TDA18218_NB_BYTES] = {0};

    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // pI2CMap & pReadBuffer initialization
    pI2CMap = &(pObj->I2CMap.uBx00.ID_byte);

     // to read the AGC1 & AGC2 gain setting
     //if (pObj->I2CMap.uBx00.bF.ID >= tmTDA18218_ID_ES3)
     //{
     //    tmTDA18218Write(pObj,0x03,2);
     //}

    // read data from the Tuner
    I2CCommResult = _SYSTEMFUNC.SY_Read(pObj->uHwAddress, I2C_NO_READ_SUB_ADDRESS, uNbData, ReadBuffer);

    if(I2CCommResult)
    {
        // copy read data in I2CMap
        for (uCounter = 0; uCounter < uNbData; uCounter++)
        {
            switch(uCounter)
            {
            case 0x00:
                //pObj->I2CMap.uBx00.ID_byte = (!m_tmTDA18218ForceES3) ? (UInt8)ReadBuffer[uCounter] : tmTDA18218_ID_ES3;
                pObj->I2CMap.uBx00.ID_byte = (UInt8)ReadBuffer[uCounter];
                break;
            case 0x1B:
                pObj->I2CMap.uBx1B.IF_Byte_2 = (UInt8)((ReadBuffer[uCounter] & 0xBF) | (pObj->Config.uRF_IF_Det << 6));
                break;
            case 0x20:
                pObj->I2CMap.uBx20.AGC1_byte_1 = (UInt8)((ReadBuffer[uCounter] & 0x08) | (pObj->I2CMap.uBx20.AGC1_byte_1 & 0xF7));
                break;
            case 0x21:
                pObj->I2CMap.uBx21.AGC1_byte_2 = (UInt8)((ReadBuffer[uCounter] & 0xC0) | (pObj->I2CMap.uBx21.AGC1_byte_2 & 0x3F));
                break;
            case 0x23: 
                break;
            default:
                pI2CMap[uCounter] = (UInt8)(ReadBuffer[uCounter]);
                break;
            }
        }
    }

    // return value
    return I2CCommResult;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    TDA18218Wait
//
// DESCRIPTION: This function waits for requested time
//
// RETURN:      True or False
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
Bool 
TMBSL_CLASS tmTDA18218Wait(
                           ptmTDA18218Object_t        pObj,        //  I: Tuner unit number
                           UInt16                    wTime        //  I: time to wait for
                              )
{
    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // wait wTime ms
#ifndef NXPFE
    tmTDA18218InitTick(pObj,wTime);
    while (tmTDA18218WaitTick(pObj) == False);
#else // NXPFE
    Wait(wTime);
#endif // NXPFE

    // Return value
    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218GetMAINPLLParams:
//
// DESCRIPTION: Get Prescaler & PostDiv corresponding to uLo
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(GetMAINPLLParams)
(
 tmUnitSelect_t            TunerUnit,        // I: Tuner unit number
 UInt32                    uLO,            // I: local oscillator frequency in Hz
 UInt8                    *uPreScaler,    // I/O: PreScaler
 UInt8                    *uPostDiv        // I/O: PostDiv
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // set standard mode
    return tmTDA18218GetMAINPLLParams(pObj, uLO, uPreScaler, uPostDiv)?TM_OK:TMBSL_ERR_TUNER_NOT_INITIALIZED;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218GetMAINPLLParams:
//
// DESCRIPTION: Get the MAIN fractional PLL settings
//
// RETURN:      True or False
//
// NOTES:       This function doesn't write in the tuner
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218GetMAINPLLParams (
                                        ptmTDA18218Object_t        pObj,            // I: tuner Object
                                        UInt32                    uLO,            // I: local oscillator frequency in Hz
                                        UInt8                    *uPreScaler,    // I/O: PreScaler
                                        UInt8                    *uPostDiv        // I/O: PostDiv
                                        )
{    
    UInt8    uCounter = 0;

    // search for MAIN_Post_Div corresponding to uLO
    uCounter = 0;
    do uCounter ++;
    while (uLO > pObj->Config.MAIN_PLL_Map[uCounter - 1].uLO_Max && uCounter < tmTDA18218_MAIN_PLL_MAP_NB_ROWS);

    //*uPreScaler = (UInt8)((pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div)&0x03 + 8);
    //*uPostDiv = 1 << (((((UInt8)pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div)&0x30) >> 4) - 1);

    *uPreScaler = (pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div)&0x07;
    *uPreScaler += 8;

    *uPostDiv = (pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div)&0x70;

    *uPostDiv = *uPostDiv >> 4;
    *uPostDiv = 1 << (*uPostDiv - 1);

    //*uPostDiv = 1 << (((((UInt8)pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div)&0x70) >> 4) - 1);


    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA18218ReadLO:
//
// DESCRIPTION: Get Prescaler & PostDiv corresponding to uLo
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS TMBSL_FUNC(ReadLO)
(
 tmUnitSelect_t            TunerUnit,        // I: Tuner unit number
 UInt32                    *uLO            // O: local oscillator frequency in hertz
 )
{
    ptmTDA18218Object_t        pObj = Null;
    tmErrorCode_t            ret  = TM_OK;

    //----------------------
    // test input parameters
    //----------------------
    // test the max number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    ret = getInstance(TunerUnit, &pObj);
    if (ret != TM_OK)
        return ret;

    // test the Object
    if (pObj == Null || pObj->init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // set standard mode
    return tmTDA18218ReadLO(pObj, uLO)?TM_OK:TMBSL_ERR_TUNER_NOT_INITIALIZED;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218GetMAINPLLParams:
//
// DESCRIPTION: Read Lo
//
// RETURN:      True or False
//
// NOTES:       This function doesn't write in the tuner
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218ReadLO
(
 ptmTDA18218Object_t        pObj,            // I: tuner Object
 UInt32                    *uLO            // O: local oscillator frequency in hertz
 )
{    
    if (pObj->I2CMap.uBx07.bF.Freq_prog_En == 1)
    {
        *uLO = ((pObj->I2CMap.uBx0A.LO_Frac_31_to_24 << 12) + (pObj->I2CMap.uBx0B.LO_Frac_23_to_16 << 4) + pObj->I2CMap.uBx0C.bF.LO_Frac_15_to_12)*1000;

        return True;
    }

    return False;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218CalcMAINPLL:
//
// DESCRIPTION: Calculate the MAIN fractional PLL settings
//
// RETURN:      True or False
//
// NOTES:       This function doesn't write in the tuner
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218CalcMAINPLL (
                                   ptmTDA18218Object_t    pObj,        // I: tuner Object
                                   UInt32                    uLO            // I: local oscillator frequency in Hz
                                      )
{    
    UInt8    uCounter = 0;
    UInt32    uDiv;

    if (pObj->I2CMap.uBx07.bF.Freq_prog_En == 1) // auto
    {
        // convert LO in KHz
        uLO /= 1000;

        // update & write LO_Frac_Auto [31..0]
        pObj->I2CMap.uBx0A.LO_Frac_31_to_24 = (UInt8)(uLO >> 12);
        pObj->I2CMap.uBx0B.LO_Frac_23_to_16 = (UInt8)(uLO >> 4);
        pObj->I2CMap.uBx0C.bF.LO_Frac_15_to_12 = ((UInt8)(uLO)); 
    }
    else
    { 
        //manual
        // search for MAIN_Post_Div corresponding to uLO
        uCounter = 0;
        do uCounter ++;
        while (uLO > pObj->Config.MAIN_PLL_Map[uCounter - 1].uLO_Max && uCounter < tmTDA18218_MAIN_PLL_MAP_NB_ROWS);
        pObj->I2CMap.uBx08.Main_Divider_byte_1 = ((UInt8)pObj->Config.MAIN_PLL_Map[uCounter - 1].uPost_Div);

        //// force LO_Sn05
        //pObj->I2CMap.uBx0F.bF..LO_Sn05 = 0;

        // calculate MAIN_Div
        uDiv = (((UInt32)(pObj->Config.MAIN_PLL_Map[uCounter - 1].uDiv) * (uLO / 1000)) << 7) / 125;
        pObj->I2CMap.uBx09.bF.LO_Int = (UInt8)(uDiv >> 16) & 0x7F;
        pObj->I2CMap.uBx0A.LO_Frac_31_to_24 = (UInt8)(uDiv >> 13) & 0x07;
        pObj->I2CMap.uBx0B.LO_Frac_23_to_16 = (UInt8)(uDiv >> 5);
        pObj->I2CMap.uBx0C.LO_Frac_15_to_8 = ((UInt8)(uDiv) & 0xF8);
        pObj->I2CMap.uBx0D.LO_Frac_7_to_0 = 0x01;
    }

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218WriteMAINPLL:
//
// DESCRIPTION: Write the MAIN fractional PLL settings
//
// RETURN:      True or False
//
// NOTES:      
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218WriteMAINPLL (
                                    ptmTDA18218Object_t        pObj,        // I: tuner Object
                                    UInt32                    uLO,        // I: local oscillator frequency in Hz
                                    Bool                    bI2CEnabled    // I: I2C enable flag
                                    )
{    
    if (pObj->I2CMap.uBx07.bF.Freq_prog_En == 1) // auto
    {
        uLO /= 1000;

        // update & write LO_Frac_Auto [31..0]
        pObj->I2CMap.uBx0A.LO_Frac_31_to_24 = (UInt8)(uLO >> 12);
        pObj->I2CMap.uBx0B.LO_Frac_23_to_16 = (UInt8)(uLO >> 4);
        pObj->I2CMap.uBx0C.bF.LO_Frac_15_to_12 = ((UInt8)(uLO)); 
        if (bI2CEnabled)
        {
            if (tmTDA18218Write(pObj,0x0A,4) != True)
                return False;
        }

        // update & write Freq_prog_Start to 1
        pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 0x01;
        if (bI2CEnabled)
        {
            if (tmTDA18218Write(pObj,0x0F,1) != True)
                return False;
        }

        // update & write Freq_prog_Start to 0
        pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 0x00;
        if (bI2CEnabled)
        {
            if (tmTDA18218Write(pObj,0x0F,1) != True)
                return False;
        }
    }
    else // manual
    {        
        // call tmTDA18218CalcMAINPLL function
        tmTDA18218CalcMAINPLL(pObj, uLO);

        // write main pll bytes
        if (bI2CEnabled)
        {
            if (tmTDA18218Write(pObj,0x08,6) != True)
                return False;
        }
    }
    //wait 5ms
    //tmTDA18218Wait(5);

    //return value
    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218CalcCALPLL:
//
// DESCRIPTION: Calculate the CAL fractionnal PLL settings
//
// RETURN:      True or False
//
// NOTES:       This function doesn't write in the tuner
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218CalcCALPLL (
                                  ptmTDA18218Object_t        pObj,        // I: tuner Object
                                  UInt32                    uLO            // I: local oscillator frequency in Hz
                                     )
{    
    UInt8    uCounter = 0;
    UInt32    uDiv;

    // search for CAL_Post_Div corresponding to uLO
    do uCounter ++;
    while (uLO > pObj->Config.CAL_PLL_Map[uCounter - 1].uLO_Max && uCounter < tmTDA18218_CAL_PLL_MAP_NB_ROWS);
    pObj->I2CMap.uBx10.Cal_Divider_byte_1 = (UInt8)pObj->Config.CAL_PLL_Map[uCounter - 1].uPost_Div; 

    // calculate CAL_Div
    uDiv = (((UInt32)(pObj->Config.CAL_PLL_Map[uCounter - 1].uDiv) * (uLO / 1000)) << 7) / 125;
    pObj->I2CMap.uBx11.CAL_Int = (UInt8)(uDiv >> 16);
    pObj->I2CMap.uBx12.bF.CAL_Frac_22_to_16 = (UInt8)(uDiv >> 9) & 0x7F;
    pObj->I2CMap.uBx13.CAL_Frac_15_to_8 = (UInt8)(uDiv >> 1);
    pObj->I2CMap.uBx14.CAL_Frac_7_to_0 = ((UInt8)(uDiv) & 0x80) << 7 | (pObj->I2CMap.uBx14.CAL_Frac_7_to_0 & 0x7F);

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218SetApplicationType:
//
// DESCRIPTION:
//
// RETURN:      True or False
//
// NOTES:       This function doesn't write in the tuner
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218SetApplicationType (
    ptmTDA18218Object_t pObj,
    tmTDA18218ApplicationType_t applicationType
    )
{
    UInt8* uAGC1_TOP_Map = Null;
    UInt8* uAGC2_TOP_Map = Null;
    UInt8 i = 0;

    // AGC1_TOP_Map uGu, uGd, uAd15, uAd12, uAd9, uAd6, uAu3, uAu6, uAu9, uAu12
    static UInt8 AGC1_TOP_Map[] = {0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // AGC2_TOP_Map uGu2, uGd2, uGu3, uGd3, uGu4, uGd4
    static UInt8 AGC2_TOP_Map[] = {    
    0x02, 0x01, 0x17, 0x16, 0x0C, 0x0B, //filtre 0
    0x02, 0x01, 0x17, 0x16, 0x0C, 0x0B, //filtre 1
    0x02, 0x01, 0x17, 0x16, 0x0C, 0x0B, //filtre 2
    0x02, 0x01, 0x15, 0x14, 0x0C, 0x0B, //filtre 3
    0x12, 0x11, 0x7, 0x6, 0x0C, 0x0B, //filtre 4
    0x02, 0x01, 0x13, 0x12, 0x0C, 0x0B, //filtre 5
    0x02, 0x01, 0x09, 0x08, 0x0C, 0x0B, //filtre 6
    0x02, 0x01, 0x09, 0x08, 0x0C, 0x0B };//filtre 7

	// update channel type
    pObj->ChannelType = tmTDA18218DigitalChannel;

    uAGC1_TOP_Map = AGC1_TOP_Map;
    uAGC2_TOP_Map = AGC2_TOP_Map;

    // set AGC1 TOP map
    pObj->Config.AGC1_TOP_Map[0].uGu = uAGC1_TOP_Map[0];
    pObj->Config.AGC1_TOP_Map[0].uGd = uAGC1_TOP_Map[1];
    pObj->Config.AGC1_TOP_Map[0].uAd15 = uAGC1_TOP_Map[2];
    pObj->Config.AGC1_TOP_Map[0].uAd12 = uAGC1_TOP_Map[3];
    pObj->Config.AGC1_TOP_Map[0].uAd9 = uAGC1_TOP_Map[4];
    pObj->Config.AGC1_TOP_Map[0].uAd6 = uAGC1_TOP_Map[5];
    pObj->Config.AGC1_TOP_Map[0].uAu3 = uAGC1_TOP_Map[6];
    pObj->Config.AGC1_TOP_Map[0].uAu6 = uAGC1_TOP_Map[7];
    pObj->Config.AGC1_TOP_Map[0].uAu9 = uAGC1_TOP_Map[8];
    pObj->Config.AGC1_TOP_Map[0].uAu12 = uAGC1_TOP_Map[9];

    // set AGC2 TOP map
    for (i = 0; i < tmTDA18218_AGC2_TOP_MAP_NB_ROWS; i++)
    {
        pObj->Config.AGC2_TOP_Map[i].uGu2 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+0];
        pObj->Config.AGC2_TOP_Map[i].uGd2 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+1];
        pObj->Config.AGC2_TOP_Map[i].uGu3 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+2];
        pObj->Config.AGC2_TOP_Map[i].uGd3 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+3];
        pObj->Config.AGC2_TOP_Map[i].uGu4 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+4];
        pObj->Config.AGC2_TOP_Map[i].uGd4 = uAGC2_TOP_Map[i*tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS+5];
    }

    pObj->ApplicationType = applicationType;

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218WriteAGC1_TOP:
//
// DESCRIPTION: Send AGC1 TOP values from the table AGC1_TOP_Map in tmTDA18218
//
// RETURN:      True or False
//
// NOTES:   for multiFE    
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218WriteAGC1_TOP (
                                     ptmTDA18218Object_t            pObj
                                        )
{    
    //aud_sel           1    1   1    1   0      0   0    0
    //au_ptr            2    3   1    0   0   1   2    3
    //ES2        Gu Gd Ad15  X   Ad12 Ad9 Au6 Au9 Au12  X
    //ES3        Gu Gd Ad15 Ad12 Ad9  Ad6 Au3 Au6 Au9  Au12

    //ES2-Hy    12  8  0    -     0    0   0   0   3    -
    //ES2-Donly    14 11  0    -     0    0   0   0   3    -
    //ES3-Hy    12  8  0    0    0    0   0   0   3    3
    //ES3-Donly    14 11  0    0    0    0   0   0   3    3

    // write Gu
    pObj->I2CMap.uBx20.bF.AGC1_Gup_sel = 1;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx21.bF.AGC1_Gud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uGu;
    if (tmTDA18218Write(pObj,0x21,1) != True) return False;

    // write Gd
    pObj->I2CMap.uBx20.bF.AGC1_Gup_sel = 0;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx21.bF.AGC1_Gud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uGd;
    if (tmTDA18218Write(pObj,0x21,1) != True) return False;

    // write Au aud_sel au_ptr
    // write Au 0 3
    //for ES3 compatible ES2
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 0;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=3;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAu12;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    // ************* SKip top Au3 for 18218  PR/CR CFR_STB#86
    //// write Au 0 0
    //pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 0;
    //pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=0;
    //if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    ////for ES2
    ////pObj->I2CMap.AGC1_aud = pObj->Config.AGC1_TOP_Map[0].uAu6;
    ////for ES3 compatible ES2
    //pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAu3;
    //if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    // *********************************************************

    // write Au 0 1
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 0;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=1;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    //for ES2
    //pObj->I2CMap.AGC1_aud = pObj->Config.AGC1_TOP_Map[0].uAu9;
    //for ES3 compatible ES2
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAu6;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    // write Au 0 2
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 0;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=2;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    //for ES2
    //pObj->I2CMap.AGC1_aud = pObj->Config.AGC1_TOP_Map[0].uAu12;
    //for ES3 compatible ES2
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAu9;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    // write Ad 1 3
    //for ES3 compatible ES2
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 1;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=3;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    //for ES3 compatible ES2
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAd12;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    // *************  SKip top uAd6 for 18218  PR/CR CFR_STB#86
    //// write Ad 1 0
    //pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 1;
    //pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=0;
    //if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    ////for ES2
    ////pObj->I2CMap.AGC1_aud = pObj->Config.AGC1_TOP_Map[0].uAd9;
    ////for ES3 compatible ES2
    //pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAd6;
    //if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    // *********************************************************

    // write Ad 1 1
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 1;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=1;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    //for ES2
    //pObj->I2CMap.AGC1_aud = pObj->Config.AGC1_TOP_Map[0].uAd12;
    //for ES3 compatible ES2        
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAd9;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    // write Ad 1 2
    pObj->I2CMap.uBx1F.bF.AGC1_aud_sel = 1;
    pObj->I2CMap.uBx1F.bF.AGC1_au_ptr=2;
    if (tmTDA18218Write(pObj,0x1F,1) != True) return False;
    //for ES2
    //for ES3 compatible ES2        
    pObj->I2CMap.uBx20.bF.AGC1_aud = (UInt8)pObj->Config.AGC1_TOP_Map[0].uAd15;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218WriteAGC2_TOP:
//
// DESCRIPTION: Send AGC2b TOP values from the table AGC2_TOP_Map in tmTDA18218
//
// RETURN:      True or False
//
// NOTES:    for MultiFE   
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218WriteAGC2_TOP (
                                     ptmTDA18218Object_t        pObj,
                                     UInt8                    uBPIndex
                                        )
{
    // write Gu2
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 1;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 1;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGu2;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    // write Gu3
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 2;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 1;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGu3;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    // write Gu4
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 3;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 1;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGu4;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    // write Gd2
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 1;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 0;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGd2;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    // write Gd3
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 2;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 0;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGd3;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    // write Gd4
    pObj->I2CMap.uBx20.bF.AGC2_RAM_sel = 3;
    pObj->I2CMap.uBx20.bF.AGC2_Gup_sel = 0;
    if (tmTDA18218Write(pObj,0x20,1) != True) return False;
    pObj->I2CMap.uBx23.bF.AGC2_Gud = (UInt8)pObj->Config.AGC2_TOP_Map[uBPIndex].uGd4;
    if (tmTDA18218Write(pObj,0x23,1) != True) return False;

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218ResetAGC:
//
// DESCRIPTION: resets AGC1 & AGC2 
//
// RETURN:   true or false       
//
// NOTES:  for MultiFE
//-------------------------------------------------------------------------------------
Bool
TMBSL_CLASS tmTDA18218ResetAGC (
                                ptmTDA18218Object_t pObj
                                )
{
//#pragma region TDA18218_DEBUG
    UInt8 tmp_AGC_On = 0;
    UInt8 tmp_pdAGC2_DETECT2 = 0;
    UInt8 tmp_pdAGC2_DETECT3 = 0;
    UInt8 tmp_pdAGC2_DETECT4 = 0;

    // store values
    tmp_AGC_On = pObj->I2CMap.uBx1C.bF.AGC_On;
    tmp_pdAGC2_DETECT2 = pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2;
    tmp_pdAGC2_DETECT3 = pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3;
    tmp_pdAGC2_DETECT4 = pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4;

    // force values
    pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT2 = (pObj->ChannelType == tmTDA18218AnalogChannel) ? 1 : 0;
    pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT3 = (pObj->ChannelType == tmTDA18218AnalogChannel) ? 1 : 0;
    pObj->I2CMap.uBx1C.bF.pdAGC2_DETECT4 = (pObj->ChannelType == tmTDA18218AnalogChannel) ? 1 : 0;
//#pragma endregion

    // start AGC clock        
    pObj->I2CMap.uBx1C.bF.AGC_On = 1;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;

    // configure AGC1 (bypass on; speed = 62,5 KHz)
    pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
    if (tmTDA18218Write(pObj,0x18,1) != True)    return False;
    pObj->I2CMap.uBx22.bF.AGC1_bypass = 1;
    pObj->I2CMap.uBx22.bF.AGC1_Speed_Up = 1;
    pObj->I2CMap.uBx22.bF.AGC1_Speed_Do = 1;
    pObj->I2CMap.uBx22.bF.AGC1_loop_off = 0;
    if (tmTDA18218Write(pObj,0x22,1) != True)    return False;
    pObj->I2CMap.uBx1F.bF.AGC1_Speed = 1;   //@1F
    if (tmTDA18218Write(pObj,0x1F,1) != True)    return False;

    //if(pObj->ChannelType == tmTDA18218DigitalChannel)
    //{
    //    // configure AGC2 (bypass on = digital only)
    //    pObj->I2CMap.uBx24.bF.AGC2_bypass = 1;
    //    pObj->I2CMap.uBx24.bF.AGC2_Speed_Up = 1;
    //    pObj->I2CMap.uBx24.bF.AGC2_Speed_Do = 1;
    //    pObj->I2CMap.uBx24.bF.AGC2_loop_off = 0;
    //}
    //else
    //{
    //    pObj->I2CMap.uBx24.bF.AGC2_loop_off = 1;
    //}

    if (tmTDA18218Write(pObj,0x24,1) != True)    return False;

    // stop AGC clock
    pObj->I2CMap.uBx1C.bF.AGC_On = 0;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;

    // write AGC1 tops
    tmTDA18218WriteAGC1_TOP(pObj);

    // write AGC2 tops
    tmTDA18218WriteAGC2_TOP(pObj,6);

    // start AGC clock
    pObj->I2CMap.uBx1C.bF.AGC_On = 1;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
    tmTDA18218Wait(pObj,1);

    // configure AGC1 (bypass off; speed = 62,5 KHz)
    pObj->I2CMap.uBx22.bF.AGC1_bypass = 0;
    if (tmTDA18218Write(pObj,0x22,1) != True)    return False;

    tmTDA18218Wait(pObj,300);

    return True;
}

Bool 
TMBSL_CLASS tmTDA18218WriteAllPlus (
                                    ptmTDA18218Object_t pObj
                                    )
{
    Bool    I2CCommResult = TM_FALSE;

    UInt8    uCounter;
    UInt8*    pI2CMap;
    UInt32    WriteBuffer[TDA18218_NB_BYTES] = {0};

    // test the Object
    if (pObj == Null || pObj->init == False)
        return False;

    // pI2CMap & pWriteBuffer initialization
    pI2CMap = &(pObj->I2CMap.uBx00.ID_byte);

    // copy I2CMap data in WriteBuffer
    for (uCounter = 0; uCounter < 31; uCounter++)
    {
        WriteBuffer[uCounter] = (UInt32)(pI2CMap[uCounter+28]);
    }

    WriteBuffer[31] = (UInt32)(0xFF);

    for (uCounter = 32; uCounter < 37; uCounter++)
        WriteBuffer[uCounter] = (UInt32)(pI2CMap[uCounter-32]);

    WriteBuffer[36] = (WriteBuffer[36] & 0xFC) | pObj->Config.uAGC1_Init;

    // write data in the Tuner
    I2CCommResult = _SYSTEMFUNC.SY_Write(pObj->uHwAddress, 0x1C, 37, WriteBuffer);

    return I2CCommResult;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218CalPOR:
//
// DESCRIPTION: RF tracking filter POR calibration
//
// RETURN:   true or false       
//
// NOTES:
//-------------------------------------------------------------------------------------
Bool 
TMBSL_CLASS tmTDA18218CalPOR (
                              ptmTDA18218Object_t pObj
                                 )
{
    UInt8 updAGC1b;
    UInt8 uRFSW_AGC1b_out;

//#pragma region TDA18218_DEBUG
    UInt8 bTempRSSI = 0;
    UInt8 bTempCAL_Int = 0;

    // store values
    bTempRSSI = pObj->I2CMap.uBx27.RSSI_byte;
    bTempCAL_Int = pObj->I2CMap.uBx11.CAL_Int;

    // force values
    pObj->I2CMap.uBx27.RSSI_byte = 0xDE;            // @ 0x27
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;
    pObj->I2CMap.uBx11.CAL_Int = 0x13;            // @ 0x11
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
//#pragma endregion

    ///////////////////////////// switch off AGC1b     
    // store pdAGC1b current value
    updAGC1b = pObj->I2CMap.uBx17.bF.pdAGC1b;
    // update & write pdAGC1b to 1
    pObj->I2CMap.uBx17.bF.pdAGC1b = 0x01;
    if (tmTDA18218Write(pObj, 0x17, 1) != True)    return False;

    ///////////////////////////// switch off RFSW_AGC1b_out     
    // store RFSW_AGC1b_out current value
    uRFSW_AGC1b_out = pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out;
    // update & write RFSW_AGC1b_out to 0
    pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0x00;
    if (tmTDA18218Write(pObj, 0x18, 1) != True)    return False;

    ///////////////////////////// clear flags
    // update & write FlagClear to 1
    pObj->I2CMap.uBx05.bF.FlagClear = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;
    // update & write FlagClear to 0
    pObj->I2CMap.uBx05.bF.FlagClear = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;
    // select RFCalPOR mode
    pObj->I2CMap.uBx19.bF.hc_mode = 0x03;
    if (tmTDA18218Write(pObj, 0x19, 1) != True)    return False;

    ///////////////////////////// launch msm
    // update & write launch_msm to 1
    pObj->I2CMap.uBx05.bF.launch_msm = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;
    // update & write launch_msm to 0
    pObj->I2CMap.uBx05.bF.launch_msm = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;

    ///////////////////////////// wait 200 ms
    tmTDA18218Wait(pObj, 150);

    ///////////////////////////// restore AGC1b
    // update & write pdAGC1b to previous value
    pObj->I2CMap.uBx17.bF.pdAGC1b = updAGC1b;
    if (tmTDA18218Write(pObj, 0x17, 1) != True)    return False;

    // update & write RFSW_AGC1b_out to previous value
    pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = uRFSW_AGC1b_out;
    if (tmTDA18218Write(pObj, 0x18, 1) != True)    return False;

//#pragma region TDA18218_DEBUG
    // Restore values
    pObj->I2CMap.uBx27.RSSI_byte = bTempRSSI;
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;
    pObj->I2CMap.uBx11.CAL_Int = bTempCAL_Int;
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
//#pragma endregion

    ///////////////////////////// select Normal mode
    pObj->I2CMap.uBx19.bF.hc_mode = 0x00;
    if (tmTDA18218Write(pObj, 0x19, 1) != True)    return False;

    ///////////////////////////// launch msm
    // update & write launch_msm to 1
    pObj->I2CMap.uBx05.bF.launch_msm = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;
    // update & write launch_msm to 0
    pObj->I2CMap.uBx05.bF.launch_msm = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1) != True)    return False;

    //read ?

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218CalAV:
//
// DESCRIPTION: RF tracking filter AV calibration
//
// RETURN:   true or false       
//
// NOTES:
//-------------------------------------------------------------------------------------
Bool 
TMBSL_CLASS tmTDA18218CalAV (
                             ptmTDA18218Object_t pObj
                                )
{
    UInt8 bTempo = 0;

//#pragma region TDA18218_DEBUG
    UInt8 bTempRSSI = 0;
    UInt8 bTempCAL_Int = 0;
    UInt8 bTempAGC2b_byte = 0;

    // store values
    bTempRSSI = pObj->I2CMap.uBx27.RSSI_byte;
    bTempCAL_Int = pObj->I2CMap.uBx11.CAL_Int;
    bTempAGC2b_byte = pObj->I2CMap.uBx1C.AGC2b_byte;

    // force values
    pObj->I2CMap.uBx27.RSSI_byte = 0xDE;            // @ 0x27
    if (tmTDA18218Write(pObj,0x27,1) != True) return False;
    pObj->I2CMap.uBx11.CAL_Int = 0x13;            // @ 0x11
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
//#pragma endregion

    ///////////////////////////// clear flags
    // update & write FlagClear to 1
    pObj->I2CMap.uBx05.bF.FlagClear = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;
    // update & write FlagClear to 0
    pObj->I2CMap.uBx05.bF.FlagClear = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;

    ///////////////////////////// Work around for ES2
    pObj->I2CMap.uBx28.bF.Force_Lock = 0x01;
    if (tmTDA18218Write(pObj, 0x28, 1)!= True)    return False;

    ///////////////////////////// select RFCalAV mode
    pObj->I2CMap.uBx19.bF.hc_mode = 0x04;
    if (tmTDA18218Write(pObj, 0x19, 1)!= True)    return False;

    ///////////////////////////// launch msm
    // update & write launch_msm to 1
    pObj->I2CMap.uBx05.bF.launch_msm = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;
    // update & write launch_msm to 0
    pObj->I2CMap.uBx05.bF.launch_msm = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;

    ///////////////////////////// wait 
    bTempo = 3 + (2 * pObj->I2CMap.uBx32.bF.NmeasAV);
    tmTDA18218Wait(pObj, (UInt16)bTempo);

    ///////////////////////////// select Normal mode
    pObj->I2CMap.uBx19.bF.hc_mode = 0x00;
    if (tmTDA18218Write(pObj, 0x19, 1)!= True)    return False;

    // Speed up the RFAGC setting
    pObj->I2CMap.uBx1C.bF.pulse_up_auto = 0;
    pObj->I2CMap.uBx1C.bF.pulse_up_width = 0;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;

    ///////////////////////////// launch msm
    // update & write launch_msm to 1
    pObj->I2CMap.uBx05.bF.launch_msm = 0x01;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;
    // update & write launch_msm to 0
    pObj->I2CMap.uBx05.bF.launch_msm = 0x00;
    if (tmTDA18218Write(pObj, 0x05, 1)!= True)    return False;
    // a waiting time of 11ms to pre-position RFAGC (first call to CALAV), done in Setrf
    // no waiting time if the RFAGC is already pre-positioned (second call to CALAV)

    // Work around for ES2
    pObj->I2CMap.uBx28.bF.Force_Lock = 0x00;
    if (tmTDA18218Write(pObj, 0x28, 1)!= True)    return False;

//#pragma region TDA18218_DEBUG
    // Restore values
    pObj->I2CMap.uBx1C.AGC2b_byte = bTempAGC2b_byte;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
    pObj->I2CMap.uBx27.RSSI_byte = bTempRSSI;
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;
    pObj->I2CMap.uBx11.CAL_Int = bTempCAL_Int;
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
//#pragma endregion

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:   tmTDA18218GetRSSI:
//
// DESCRIPTION: Launch and read the RSSI
//
// RETURN:      true or false
//
// NOTES:  
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218GetRSSI (
                               ptmTDA18218Object_t            pObj,
                               UInt32*                        pRSSI)
{
    UInt8 bTempAGC = 0;
    UInt8 bTempRSSI_byte = 0;

    Int32 TempRSSI=0;
    //Int32 CorrAGC1[4] = {3, 6, 9, 15};

    //Init before RSSI
    // Read AGC2 before writing it on I2C bus
    bTempAGC = pObj->I2CMap.uBx1C.bF.AGC_On;        
    pObj->I2CMap.uBx1C.bF.AGC_On = 0;                // @ 0x1C
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;

    // Store RSSI_byte
    bTempRSSI_byte = pObj->I2CMap.uBx27.RSSI_byte;
    pObj->I2CMap.uBx27.RSSI_byte = 0xC9; // @ 0x27
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;

    pObj->I2CMap.uBx05.bF.FlagClear = 1;            // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx19.bF.hc_mode = 6;                // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;

    pObj->I2CMap.uBx05.bF.FlagClear = 0;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 1;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 0;
    // do not send, launch_msm returns to 0 automatically in the IC

    tmTDA18218Wait(pObj, 15);

    pObj->I2CMap.uBx19.bF.hc_mode = 0;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;

    // restore RSSI_byte
    pObj->I2CMap.uBx27.RSSI_byte = bTempRSSI_byte;
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;

    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;
    if (pObj->I2CMap.uBx05.bF.FlagMSM_OK != True)        
    {
        pObj->I2CMap.uBx1C.bF.AGC_On = bTempAGC;        // @ 0x1C
        if (!m_tmTDA18218AGC1WorkAround)
        {
            if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
        }
        else
        {
            if (tmTDA18218WriteAllPlus(pObj) != True)    return False;
        }
        return False;
    }
    if (pObj->I2CMap.uBx04.bF.RSSI_Alarm == True)        
    {
        pObj->I2CMap.uBx1C.bF.AGC_On = bTempAGC;        // @ 0x1C
        if (!m_tmTDA18218AGC1WorkAround)
        {
            if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
        }
        else
        {
            if (tmTDA18218WriteAllPlus(pObj) != True)    return False;
        }
        return False;
    }

    pObj->I2CMap.uBx1C.bF.AGC_On = bTempAGC;        // @ 0x1C
    if (!m_tmTDA18218AGC1WorkAround)
    {
        if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;
    }
    else
    {
        if (tmTDA18218WriteAllPlus(pObj) != True)    return False;
    }

    if (m_tmTDA18218RSSIInverted)
    {
        TempRSSI = pObj->I2CMap.uBx02.RSSI;
    }
    else
    {
        TempRSSI = (pObj->I2CMap.uBx02.bF.RSSI_0 << 7) |
            (pObj->I2CMap.uBx02.bF.RSSI_1 << 6) |
            (pObj->I2CMap.uBx02.bF.RSSI_2 << 5) |
            (pObj->I2CMap.uBx02.bF.RSSI_3 << 4) |
            (pObj->I2CMap.uBx02.bF.RSSI_4 << 3) |
            (pObj->I2CMap.uBx02.bF.RSSI_5 << 2) |
            (pObj->I2CMap.uBx02.bF.RSSI_6 << 1) |
            (pObj->I2CMap.uBx02.bF.RSSI_7 << 0);
    }

    *pRSSI = TempRSSI;

    return True;

}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218SetRFIO:
//
// DESCRIPTION: Set the input and output configuration of first tuner amplifier
//
// RETURN:   true or false       
//
// NOTES:  for internal  
//-------------------------------------------------------------------------------------
Bool 
TMBSL_CLASS tmTDA18218SetRFIO(
                              ptmTDA18218Object_t pObj, 
                              tmTDA18218RFIOConfig_t Index
                                 )
{
    switch(Index) 
    {
    case tmTDA18218Single_HZin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Single_HZin_LTOonMTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Single_HZin_LTOonLT_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Single_RFin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Single_RFin_LTOonLT:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Single_Rfin_LTOonMTO_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Master_HZin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Master_HZin_LTOonMTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Master_HZin_LTOonLT_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Master_RFin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Master_RFin_LTOonLT:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Master_Rfin_LTOonMTO_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 0;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 1;
        break;

    case tmTDA18218Slave_M_HZin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Slave_M_HZin_LTOonMTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Slave_M_HZin_LTOonLT_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 0;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Slave_M_RFin_noLTO:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 1;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Slave_M_RFin_LTOonLT:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 0;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 1;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;

    case tmTDA18218Slave_M_Rfin_LTOonMTO_test:
        pObj->I2CMap.uBx17.bF.pdHZlna = 0;
        pObj->I2CMap.uBx17.bF.pdAGC1a = 1;
        pObj->I2CMap.uBx17.bF.pdAGC1b = 1;
        pObj->I2CMap.uBx17.bF.pdLT = 1;
        pObj->I2CMap.uBx18.bF.pdDETECT1 = 0;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_Hzin = 1;
        pObj->I2CMap.uBx18.bF.RFSW_MTO_LT_RFin = 0;
        pObj->I2CMap.uBx18.bF.RFSW_Buf1_out = 1;
        pObj->I2CMap.uBx18.bF.RFSW_AGC1b_out = 0;
        break;
    default:
        break;
    }

    // write bytes 0x17 to 0x18
    if (tmTDA18218Write(pObj,0x17,2) != True)    
        return False;

    // store RFIO config
    pObj->RFIOConfig = Index;

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmTDA18218GetRFIO:
//
// DESCRIPTION: Get the input and output configuration of first tuner amplifier
//
// RETURN:   true or false       
//
// NOTES:  for internal  
//-------------------------------------------------------------------------------------
Bool 
TMBSL_CLASS tmTDA18218GetRFIO(
                              ptmTDA18218Object_t pObj, 
                              tmTDA18218RFIOConfig_t *pIndex
                                 )
{
    // get RFIO config
    *pIndex = pObj->RFIOConfig;

    return True;
}


//-------------------------------------------------------------------------------------
// FUNCTION:   tmTDA18218IRCalibration:
//
// DESCRIPTION: Launch the procedure of IR calibration. It should be done once at the power up of the tuner
//
// RETURN:      True or False
//
// NOTES:  
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218IRCalibration (ptmTDA18218Object_t    pObj)
{
    UInt32 uRF;
    //UInt8 Index = 0;

    // force values
    //Init before calibration
    pObj->I2CMap.uBx27.RSSI_byte = 0xDE;            // @ 0x27
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;
    pObj->I2CMap.uBx17.Power_Down_byte_1 = 0xF8;    // @ 0x17
    if (tmTDA18218Write(pObj,0x17,1) != True)    return False;
    pObj->I2CMap.uBx18.Power_Down_byte_2 = 0x0F;    // @ 0x18
    if (tmTDA18218Write(pObj,0x18,1) != True)    return False;
    pObj->I2CMap.uBx1C.AGC2b_byte = 0x8B;            // @ 0x1C
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;

    // 1st step IRCAL wanted
    pObj->I2CMap.uBx29.bF.IR_Freq = 2;            // @ 0x29
    if (tmTDA18218Write(pObj,0x29,1) != True)    return False;
    pObj->I2CMap.uBx19.bF.hc_mode = 1;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;        
    pObj->I2CMap.uBx11.CAL_Int = 0x13;        // @ 0x11, 19x64kHz = 1,216 MHz
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
    uRF = (pObj->Config.IR_Map[1].uRF_Max);
    pObj->I2CMap.uBx0A.LO_Frac_31_to_24 = (UInt8)(uRF >> 12);
    pObj->I2CMap.uBx0B.LO_Frac_23_to_16 = (UInt8)(uRF >> 4);
    pObj->I2CMap.uBx0C.bF.LO_Frac_15_to_12 = (UInt8)(uRF);
    if (tmTDA18218Write(pObj,0x0A,3) != True)    return False;
    pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 1; // @ 0x0F
    if (tmTDA18218Write(pObj,0x0F,1) != True)    return False;
    tmTDA18218Wait(pObj, 5);

    pObj->I2CMap.uBx05.bF.FlagClear = 1;            // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.FlagClear = 0;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 1;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 0;
    tmTDA18218Wait(pObj, 1);
    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;
    if (pObj->I2CMap.uBx05.bF.FlagMSM_OK != 1) return False;

    // Image
    pObj->I2CMap.uBx11.CAL_Int = (UInt8)0xED;        // @ 0x11, -19x64kHz = -1,216 MHz
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
    pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 1; // @ 0x0F
    if (tmTDA18218Write(pObj,0x0F,1) != True)    return False;
    tmTDA18218Wait(pObj, 5);
    pObj->I2CMap.uBx19.bF.hc_mode = 2;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;

    pObj->I2CMap.uBx05.bF.FlagClear = 1;            // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;        
    pObj->I2CMap.uBx05.bF.FlagClear = 0;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 1;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 0;
    tmTDA18218Wait(pObj, 40);
    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;
    if (pObj->I2CMap.uBx05.bF.FlagMSM_OK != 1) return False;

    // 2nd step IRCAL wanted
    pObj->I2CMap.uBx29.bF.IR_Freq = 3;            // @ 0x29
    if (tmTDA18218Write(pObj,0x29,1) != True)    return False;
    pObj->I2CMap.uBx19.bF.hc_mode = 1;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;
    pObj->I2CMap.uBx11.CAL_Int = 0x13;        // @ 0x11, 19x64kHz = 1,216 MHz
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
    uRF = (pObj->Config.IR_Map[2].uRF_Max);
    pObj->I2CMap.uBx0A.LO_Frac_31_to_24 = (UInt8)(uRF >> 12);
    pObj->I2CMap.uBx0B.LO_Frac_23_to_16 = (UInt8)(uRF >> 4);
    pObj->I2CMap.uBx0C.bF.LO_Frac_15_to_12 = (UInt8)(uRF); 
    if (tmTDA18218Write(pObj,0x0A,3) != True)    return False;
    pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 1; // @ 0x0F
    if (tmTDA18218Write(pObj,0x0F,1) != True)    return False;
    tmTDA18218Wait(pObj, 5);

    pObj->I2CMap.uBx05.bF.FlagClear = 1;            // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.FlagClear = 0;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 1;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 0;
    tmTDA18218Wait(pObj, 1);
    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;
    if (pObj->I2CMap.uBx05.bF.FlagMSM_OK != 1) return False;

    // Image
    pObj->I2CMap.uBx11.CAL_Int = (UInt8)0xED;        // @ 0x11, -19x64kHz = -1,216 MHz
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
    pObj->I2CMap.uBx0F.bF.Freq_prog_Start = 1; // @ 0x0F
    if (tmTDA18218Write(pObj,0x0F,1) != True)    return False;
    tmTDA18218Wait(pObj, 5);
    pObj->I2CMap.uBx19.bF.hc_mode = 2;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;

    pObj->I2CMap.uBx05.bF.FlagClear = 1;            // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.FlagClear = 0;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 1;        // @ 0x05
    if (tmTDA18218Write(pObj,0x05,1) != True)    return False;
    pObj->I2CMap.uBx05.bF.launch_msm = 0;
    tmTDA18218Wait(pObj, 40);
    if (tmTDA18218Read(pObj,0x00,7) != True)    return False;
    if (pObj->I2CMap.uBx05.bF.FlagMSM_OK != 1)            return False;    

    pObj->I2CMap.uBx19.bF.hc_mode = 0;            // @ 0x19
    if (tmTDA18218Write(pObj,0x19,1) != True)    return False;

    // restore values
     pObj->I2CMap.uBx27.RSSI_byte = 0xC9;            // @ 0x27
    if (tmTDA18218Write(pObj,0x27,1) != True)    return False;
    pObj->I2CMap.uBx11.CAL_Int = 0x13;
    pObj->I2CMap.uBx17.Power_Down_byte_1 = 0xF0;
    pObj->I2CMap.uBx18.Power_Down_byte_2 = 0x19;
    pObj->I2CMap.uBx1C.AGC2b_byte = 0x98;
    if (tmTDA18218Write(pObj,0x11,1) != True)    return False;
    if (tmTDA18218Write(pObj,0x17,2) != True)    return False;
    if (tmTDA18218Write(pObj,0x1C,1) != True)    return False;

/*
    // Save initial calibration coefficient
    pObj->I2CMap.uBx29.bF.IR_Freq = 3; 
    if (tmTDA18218Write(pObj,0x29,1) != True)    return False;
    for (Index=0 ;Index<4; Index++) {
        pObj->I2CMap.uBx2A.bF.Ptr_Rx_Read = Index;
        if (tmTDA18218Write(pObj, 0x2A, 1) != True)    return False;
        if (tmTDA18218Read(pObj, 0x00, 59) != True)    return False;
        pObj->Config.IRCoefHighF[Index] = pObj->I2CMap.uBx2B.bF.Rx_W;
    }
*/
    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:   tmTDA18218WriteIRCal:
//
// DESCRIPTION: Write the trim values of IR cal
//
// RETURN:      true or false
//
// NOTES:  
//-------------------------------------------------------------------------------------
//
Bool
TMBSL_CLASS tmTDA18218WriteTrimIRCal (ptmTDA18218Object_t    pObj)
{
    pObj->I2CMap.uBx2B.bF.Rx_ptrW = 0;
    if (tmTDA18218Write(pObj,0x2A,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_W = (UInt8)pObj->Config.IR_Rx_Map[0].uR2I;    
    if (tmTDA18218Write(pObj,0x2B,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_ptrW = 1;
    if (tmTDA18218Write(pObj,0x2A,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_W = (UInt8)pObj->Config.IR_Rx_Map[0].uR2Q;
    if (tmTDA18218Write(pObj,0x2B,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_ptrW = 2;    
    if (tmTDA18218Write(pObj,0x2A,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_W = (UInt8)pObj->Config.IR_Rx_Map[0].uR3IQ;
    if (tmTDA18218Write(pObj,0x2B,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_ptrW = 3;
    if (tmTDA18218Write(pObj,0x2A,1) != True)    return False;
    pObj->I2CMap.uBx2B.bF.Rx_W = (UInt8)pObj->Config.IR_Rx_Map[0].uR3InQ;
    if (tmTDA18218Write(pObj,0x2B,1) != True)    return False;

    return True;
}

//-------------------------------------------------------------------------------------
// FUNCTION:    TDA18218ThermometerRead:
//
// DESCRIPTION: Read die temperature
//
// RETURN:      TMBSL_ERR_IIC_ERR
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t
TMBSL_CLASS tmTDA18218ThermometerRead
(
 ptmTDA18218Object_t        pObj,        // I: Tuner unit number
 Int8*                    puValue        // O: Read thermometer value
 )
{
    //tmErrorCode_t ret = TMBSL_ERR_IIC_ERR;

    UInt32 uCounter = 0;

    // test the Object
    if (pObj == Null)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // switch thermometer on
    pObj->I2CMap.uBx1D.bF.TM_ON = 0x01;

    // write byte THERMO_BYTE
    if (tmTDA18218Write(pObj, 0x1D, 1) != True)
        return TMBSL_ERR_IIC_ERR;

    // read byte THERMO_BYTE
    if (tmTDA18218Read (pObj, 0x00, 7) != True)
        return TMBSL_ERR_IIC_ERR;

    // set & write TM_Range
    if (((pObj->I2CMap.uBx01.bF.TM_D == 0x00) && (pObj->I2CMap.uBx1D.bF.TM_Range == 0x01)) || ((pObj->I2CMap.uBx01.bF.TM_D == 0x08) && (pObj->I2CMap.uBx1D.bF.TM_Range == 0x00)))
    {
        pObj->I2CMap.uBx1D.bF.TM_Range = !(pObj->I2CMap.uBx1D.bF.TM_Range);

        // write byte THERMO_BYTE
        if (tmTDA18218Write(pObj, 0x1D, 1) != True)
            return TMBSL_ERR_IIC_ERR;

        // wait 10ms
        if (tmTDA18218Wait(pObj,1) == False)
            return TM_FALSE;   

        // read byte THERMO_BYTE
        if (tmTDA18218Read (pObj, 0x00, 7) != True)
            return TMBSL_ERR_IIC_ERR;
    }

    // search for TMVALUE corresponding to TM_D
    uCounter = 0;
    do uCounter ++;
    while (pObj->I2CMap.uBx01.bF.TM_D > pObj->Config.THERMOMETER_Map[uCounter - 1].uTM_D && uCounter < tmTDA18218_THERMOMETER_NB_ROWS);

    // get TMVALUE value
    if (pObj->I2CMap.uBx1D.bF.TM_Range == 0x00)
        *puValue = (UInt8)pObj->Config.THERMOMETER_Map[uCounter - 1].uTM_60_92;
    else
        *puValue = (UInt8)pObj->Config.THERMOMETER_Map[uCounter - 1].uTM_92_122;

    // switch thermometer off
    pObj->I2CMap.uBx1D.bF.TM_ON = 0x00;

    // write byte THERMO_BYTE
    if (tmTDA18218Write(pObj, 0x1D, 1) != True)
        return TMBSL_ERR_IIC_ERR;

    return TM_OK;
}

#ifdef DVBT
#ifndef NXPFE
#ifdef _WIN32
//-------------------------------------------------------------------------------------
// FUNCTIONS ADDED FOR FRONTEND EVALUATION APPLICATION IN EXPERT MODE
//
// All these functions must be removed from reference code before sending
// to customer
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbInitDriver
//
// DESCRIPTION: Initialize the fields of SystemFunc of the 
//                gTDA182I1Instance[TunerUnit] with the address from SystemFunc.dll
//                Open IIC communication
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//                TM_FALSE
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbInitDriver)
(
 tmUnitSelect_t        TunerUnit,            //  I: Tuner unit number
 UInt32                uHwAddress,            //  I: Tuner unit I2C address
 UInt32                uLptPort,            //  I: I2C interface LPT port
 UInt32                uI2cSpeed,            //  I: I2C interface speed
 LPSTR*                uParamTunerNumber    //  I: Tuner unit number
 )
{
    ptmTDA18218Object_t        pObj;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // pObj initialization
    pObj = &gTDA18218Instance[TunerUnit];

    //----------------------
    // init I2C map 
    //----------------------
    //// read bytes 0x00 to 0x0F
    //if (TDA18218Read (pObj,0x00,16) != True)
    //    return TMBSL_ERR_IIC_ERR;

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbDeInitDriver
//
// DESCRIPTION: DeInitialize gTDA182I1Instance[TunerUnit]
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbDeInitDriver)
(
 tmUnitSelect_t        TunerUnit        //  I: Tuner unit number
 )
{
    ptmTDA18218Object_t        pObj;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // test the Object
    if (gTDA18218Instance[TunerUnit].init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // pObj initialization
    pObj = &gTDA18218Instance[TunerUnit];

    //---------------------------------
    // De-initialize the Object
    //---------------------------------
    pObj->init = False;

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbSetUrt
//
// DESCRIPTION: Show URT configuration panel
//
// RETURN:        TM_OK
//                TM_FALSE
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbSetUrt)
(
 tmUnitSelect_t        TunerUnit        //  I: Tuner unit number
 )
{    
    ptmTDA18218Object_t        pObj;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // test the Object
    if (gTDA18218Instance[TunerUnit].init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // pObj initialization
    pObj = &gTDA18218Instance[TunerUnit];

    // open URT configuration panel
    if (_SYSTEMFUNC.SY_SetUrt() == False)
        return TM_FALSE;

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbSetErrorEnable
//
// DESCRIPTION: Show URT configuration panel
//
// RETURN:        TM_OK
//                TM_FALSE
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbSetErrorEnable)
(
 tmUnitSelect_t        TunerUnit,        //  I: Tuner unit number
 UInt32                ErrorEnable        //  I: URT error enable value        
 )
{    
    ptmTDA18218Object_t        pObj;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // test the Object
    if (gTDA18218Instance[TunerUnit].init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // pObj initialization
    pObj = &gTDA18218Instance[TunerUnit];

    // open URT configuration panel
    if (_SYSTEMFUNC.SY_SetErrorEnable((Bool)ErrorEnable) == False)
        return TM_FALSE;

    return TM_OK;
}


//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbSetConfig:
//
// DESCRIPTION: Set the Config of the TDA182I1
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_ERR_NOT_SUPPORTED
//              TMBSL_ERR_TUNER_BAD_PARAMETER
//              TM_OK
//
// NOTES:
//-----------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbSetConfig)
(
 tmUnitSelect_t     TunerUnit,  //  I: TunerUnit number
 UInt32             uItemId,    //  I: Identifier of the item to modify
 UInt32             uValue      //  I: Value to set for the Config item
 )
{
    // return value    
    return TMBSL_CLASS TMBSL_FUNC(SetConfig)(TunerUnit,uItemId,uValue);
}


//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbGetConfig:
//
// DESCRIPTION: Get the Config of the TDA182I1
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_OK
//
// NOTES:       
//-----------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbGetConfig)
(
 tmUnitSelect_t     TunerUnit,  //  I: Tuner unit number
 UInt32             uItemId,    //  I: Identifier of the item to get value
 UInt32*            puValue     //  I: Address of the variable to output the Config item value
 )
{
    // return value    
    return TMBSL_CLASS TMBSL_FUNC(GetConfig)(TunerUnit,uItemId,puValue);
}

//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbSetRf:
//
// DESCRIPTION: Calculate i2c I2CMap & write in TDA182I1
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TMBSL_ERR_TUNER_BAD_PARAMETER
//              TMBSL_ERR_IIC_ERR
//              TM_OK
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbSetRf)
(
 tmUnitSelect_t        TunerUnit,  //  I: Tuner unit number
 UInt32                uRF            //  I: RF frequency in hertz
 )
{    
    ptmTDA18218Object_t        pObj;

    //------------------------------
    // test input parameters
    //------------------------------
    // test the instance number
    if (TunerUnit > TDA18218_MAX_UNITS)
        return TMBSL_ERR_TUNER_BAD_UNIT_NUMBER;

    // test the Object
    if (gTDA18218Instance[TunerUnit].init == False)
        return TMBSL_ERR_TUNER_NOT_INITIALIZED;

    // pObj initialization
    pObj = &gTDA18218Instance[TunerUnit];

    // return value    
    return TMBSL_CLASS TMBSL_FUNC(SetRf)(TunerUnit,uRF);
}


//-----------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbGetSWVersion:
//
// DESCRIPTION: Return the version of this device
//
// RETURN:      TM_OK
//
// NOTES:       Values defined in the tmTDA182I1local.h file
//-----------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbGetSWVersion)
(
 ptmSWVersion_t     pSWVersion        //  I: Receives SW Version 
 )
{
    pSWVersion->compatibilityNr = tmTDA18218_BSL_COMP_NUM;
    pSWVersion->majorVersionNr  = tmTDA18218_BSL_MAJOR_VER;
    pSWVersion->minorVersionNr  = tmTDA18218_BSL_MINOR_VER;

    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbGetI2CLog
//
// DESCRIPTION: get I2C log data
//
// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_ERR_IIC_ERR
//              TM_FALSE
//              TM_OK
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbGetI2CLog)
(
 tmUnitSelect_t      TunerUnit,        //  I: Tuner unit number
 SAFEARRAY            **Table            //  Table containing a structure
 )    
{    
    // return value    
    return TM_FALSE;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbClearLog:
//
// DESCRIPTION: 

// RETURN:      TMBSL_ERR_TUNER_BAD_UNIT_NUMBER
//              TMBSL_ERR_TUNER_NOT_INITIALIZED
//              TM_FALSE
//              TM_OK
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbClearLog)
(
 tmUnitSelect_t        TunerUnit    //  I: Tuner unit number
 )
{    
    // return value
    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbWrite
//
// DESCRIPTION: This function writes I2C data
//
// RETURN:      True or False
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbWrite)
(
 tmUnitSelect_t            TunerUnit,    // I: Tuner unit number
 UInt32                    uSubAddress,// I: sub address
 UInt32                    uNbData        // I: nb of data
 )
{
    // return value
    return TM_OK;
}


//-------------------------------------------------------------------------------------
// FUNCTION:    tmbslTDA182I1VbRead
//
// DESCRIPTION: This function reads I2C data
//
// RETURN:      True or False
//
// NOTES:       
//-------------------------------------------------------------------------------------
//
tmErrorCode_t __stdcall
TMBSL_CLASS TMBSL_FUNC(VbRead)
(
 tmUnitSelect_t            TunerUnit,    // I: Tuner unit number
 UInt32                    uSubAddress,// I: sub address
 UInt32                    uNbData        // I: nb of data
 )
{
    // return value
    return TM_OK;
}
#endif //_WIN32
#endif // NXPFE

#endif
#endif // (SYS_TUN_MODULE == TDA18218)

