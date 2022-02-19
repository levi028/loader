/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL5005.h
*
*    Description:    Header file of MAXLINEAR MxL5005 TUNER.
*    History:
*           Date          Author                 Version          Reason
*        ============    =============    =========    =================
*   4.17.2008         David.Deng          Ver 0.1         Create file.
*****************************************************************************/

#ifndef __TUN_MxL301_H__
#define __TUN_MxL301_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

#include "mxl301rf.h"
#include "mxl_common.h"

#if 0
/******************************************************************************
**
**  Name: MxL_Set_Register
**
**  Description:    Write one register to MxL301
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
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
**  Description:    Read one register from MxL301
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
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
**  Description:    MxL301 Initialization
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
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
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
**                    RF_Freq_Hz            - RF Frequency in Hz
**                    BWMHz                - Bandwidth 6, 7 or 8 MHz
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_RFTUNE if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_Soft_Reset
**
**  Description:    Software Reset the MxL301 Tuner
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Soft_Reset(MxLxxxRF_TunerConfigS* myTuner);

/******************************************************************************
**
**  Name: MxL_Check_ChipVersion
**
**  Description:    Return the MxL301 Chip ID
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
**
**  Returns:        MxL_ChipVersion
**
******************************************************************************/
MxL_ERR_MSG MxL_Check_ChipVersion(MxLxxxRF_TunerConfigS*);

/******************************************************************************
**
**  Name: MxL_RFSynth_Lock_Status
**
**  Description:    RF synthesizer lock status of MxL301
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
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
**  Description:    REF synthesizer lock status of MxL301
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
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
**  Description:    Calculate RF Input Power Level in dBm
**
**  Parameters:        myTuner                - Pointer to MxL301_TunerConfigS
**                    RF_Input_Level        - Integer Pointer to RF_Input_Level in dBm
**
**  Returns:        MxL_ERR_MSG            - MxL_OK if success
**                                        - MxL_ERR_OTHERS if fail
**
******************************************************************************/
MxL_ERR_MSG MxL_Check_RF_Input_Power(MxLxxxRF_TunerConfigS* , SINT32 *RF_Input_Level);
/******************************************************************************
**
**  Name: MxL_I2C_Write
**
**  Description:    I2C write operations
**
**  Parameters:
**                    DeviceAddr    - MxL301 Device address
**                    pArray        - Write data array pointer
**                    count        - total number of array
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count, UINT32 tuner_id);

/******************************************************************************
**
**  Name: MxL_I2C_Read
**
**  Description:    I2C read operations
**
**  Parameters:
**                    DeviceAddr    - MxL301 Device address
**                    Addr        - register address for read
**                    *Data        - data return
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, UINT32 tuner_id);

/******************************************************************************
**
**  Name: MxL_Delay
**
**  Description:    Delay function in milli-second
**
**  Parameters:
**                    mSec        - milli-second to delay
**
**  Returns:        0
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
void MxL_Delay(UINT32 mSec);
#endif
INT32 tun_mxl301_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl301_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_mxl301_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_mxl301_powcontrol(UINT32 tuner_id, UINT8 stdby);
INT32 tun_mxl301_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif  /* __TUN_MxL5005_H__ */



