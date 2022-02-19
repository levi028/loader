/******************************************************
mxl_api.c
----------------------------------------------------
Rf IC control functions

<Revision History>
'11/10/11 : OKAMOTO    Implement "MxL_PowerUpSettings".
'11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix".
'11/06/22 : OKAMOTO    AGC mode selection.
'11/03/16 : OKAMOTO    Select IDAC setting in "MxL_Tuner_RFTune".
'11/03/16 : OKAMOTO    Select IDAC setting.
'11/02/22 : OKAMOTO    IF out selection.
'11/02/14 : OKAMOTO    Correct build error. in C++.
'11/02/09 : OKAMOTO    Correct build error.
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/

#ifndef __MxL_API_H
#define __MxL_API_H

#include <types.h>
#include "mxl_common.h"

/* '11/11/24 : OKAMOTO    Correct header file name. ("mxl_user_define.h"->"mxL_user_define.h")*/
#include "mxL_user_define.h"

/* '11/02/22 : OKAMOTO    IF out selection. */
/******************************/
/*    MxL IF OUT message        */
/******************************/
typedef enum{
    MxL_IF_OUT_1=0,
    MxL_IF_OUT_2,
    MxL_IF_OUT_MAX,
}MxL_IF_OUT;

/* '11/06/22 : OKAMOTO    AGC mode selection. */
/******************************/
/*    MxL IF AGC message        */
/******************************/
typedef enum{
    MxL_AGC_MODE_1=0,
    MxL_AGC_MODE_2,
    MxL_AGC_MODE_MAX,
}MxL_AGC_MODE;

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
**
**  Name: MxL_Set_Register
**
**  Description:    Write one register to MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**                    RegAddr                - Register address to be written
**                    RegData                - Data to be written
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_SET_REG if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Set_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 RegData);

/******************************************************************************
**
**  Name: MxL_Get_Register
**
**  Description:    Read one register from MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**                    RegAddr                - Register address to be read
**                    RegData                - Pointer to register read
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_GET_REG if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Get_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 *RegData);

/******************************************************************************
**
**  Name: MxL_Tuner_Init
**
**  Description:    MxL Tuner Initialization
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_INIT if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Tuner_Init(MxLxxxRF_TunerConfigS* );

/******************************************************************************
**
**  Name: MxL_Tuner_RFTune
**
**  Description:    Frequency tunning for channel
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_RFTUNE if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS*);
//MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS*, UINT32 RF_Freq_Hz, MxLxxxRF_BW_MHz BWMHz);

/******************************************************************************
**
**  Name: MxL_Soft_Reset
**
**  Description:    Software Reset the MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Soft_Reset(MxLxxxRF_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Standby
**
**  Description:    Enter Standby Mode
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Stand_By(MxLxxxRF_TunerConfigS*);


/* '11/10/11 : OKAMOTO    Implement "MxL_PowerUpSettings". */
/******************************************************************************
**
**  Name: MxL_PowerUpSettings
**
**  Description:    Enables all the sections in the main path.
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_PowerUpSettings(MxLxxxRF_TunerConfigS* myTuner);

/******************************************************************************
**
**  Name: MxL_Wakeup
**
**  Description:    Wakeup from Standby Mode (Note: This function call RF_Tune after wake up.)
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Wake_Up(MxLxxxRF_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Check_ChipVersion
**
**  Description:    Return the MxL Tuner Chip ID
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**                    myChipVersion        - MxL_ChipVersion
**  Returns:        MxL_ERR_MSG
**
******************************************************************************/
MxL_ERR_MSG MxL_Check_ChipVersion(MxLxxxRF_TunerConfigS*, MxLxxxRF_ChipVersion*);

/******************************************************************************
**
**  Name: MxL_RFSynth_Lock_Status
**
**  Description:    RF synthesizer lock status of MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**                    isLock                - Pointer to Lock Status
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_RFSynth_Lock_Status(MxLxxxRF_TunerConfigS* , BOOL* isLock);

/******************************************************************************
**
**  Name: MxL_REFSynth_Lock_Status
**
**  Description:    REF synthesizer lock status of MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxLxxxRF_TunerConfigS
**                    isLock                - Pointer to Lock Status
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_REFSynth_Lock_Status(MxLxxxRF_TunerConfigS* , BOOL* isLock);

/******************************************************************************
**
**  Name: MxL_Check_RF_Input_Power
**
**  Description:    Reads the RF Input power from MxL Tuner
**
**  Parameters:        myTuner                - Pointer to MxL30xRF_TunerConfigS
**                    RF_Input_Level        - Pointer to return value, RF input level
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
//MxL_ERR_MSG MxL_Check_RF_Input_Power(MxLxxxRF_TunerConfigS* myTuner, REAL32* RF_Input_Level);
MxL_ERR_MSG MxL_Check_RF_Input_Power(MxLxxxRF_TunerConfigS* myTuner, INT32* RF_Input_Level);


/* '11/02/22 : OKAMOTO    IF out selection. */
/*====================================================*
    MxL_if_out_select
   --------------------------------------------------
    Description     Control bits for IF1/2 output enabling.
    Argument        DeviceAddr    - MxL Tuner Device address
                    if_out    IF output
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_if_out_select(UINT8 DeviceAddr, MxL_IF_OUT if_out, UINT32 tuner_id);

/* '11/06/22 : OKAMOTO    AGC mode selection. */
/*====================================================*
    MxL_agc_mode_select
   --------------------------------------------------
    Description     External VAGC pin selection, only work for DIG_AGCSEL=1.
    Argument        DeviceAddr    - MxL Tuner Device address
                    agc_mode_RegData
                    (0= Select VAGC1 (Default), 1= Select VAGC2)
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_mode_select(UINT8 DeviceAddr, MxL_AGC_MODE agc_mode, UINT32 tuner_id);

/* '11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix". */
/*====================================================*
    MxL_agc_gain_fix
   --------------------------------------------------
    Description     Fix AGC gain.
    Argument        DeviceAddr    - MxL Tuner Device address
                    bFixGain(TRUE:Fix gain , FALSE:Not fix)
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_gain_fix(UINT8 DeviceAddr, BOOL bFixGain, UINT32 tuner_id);

#ifdef __cplusplus
}
#endif

#endif /*__MxL_API_H*/

