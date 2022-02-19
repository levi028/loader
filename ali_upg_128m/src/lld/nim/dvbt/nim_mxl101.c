/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    This file contains m3327 basic function in LLD.
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    4.14.2005        Sam Chen      Ver 0.1       Create file.
*
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>
#include "nim_mxl101.h"
//#include"MxL101SF_PhyCtrlApi.h"
//#if(SYS_DEM_MODULE==MXL101 )
extern DWORD OS_GetTime(void);

#define MXL101_DEBUG_FLAG    0
#if(MXL101_DEBUG_FLAG)
    #define NIM_MXL101_PRINTF  libc_printf
    #define NIM_PRINTF         libc_printf
#else
    #define NIM_MXL101_PRINTF(...)
    #define NIM_PRINTF(...)
#endif

//#define soc_printf(...)
#define Delay_Number    1
#define WAIT_Number    1
#define SYS_WAIT_MS(x) osal_delay((x*1000*WAIT_Number))//osal_task_sleep(x*WAIT_Number)    //osal_delay((x*1000))

#define abs(x)    (x<0) ? -x : x
#define sign(y)    (y>0) ? 1 : -1

#define MXL101_rule
#define Mode_a


#define    NIM_MXL101_FLAG_ENABLE             0x00000100    //050513 Tom
#define NIM_MXL101_SCAN_END                        0x00000001
#define NIM_MXL101_CHECKING            0x00000002

ID f_MXL101_tracking_task_id = OSAL_INVALID_ID;

ID      nim_MXL101_flag[2] = {OSAL_INVALID_ID,OSAL_INVALID_ID};
struct MXL101_Lock_Info                 *MXL101_CurChannelInfo[2];
__MAYBE_UNUSED__ static struct MXL101_Demod_Info                *MXL101_COFDM_Info[2];

INT8 MXL101_autoscan_stop_flag = 0;    //50622-01Angus
ID f_IIC_Sema_ID[2] = {OSAL_INVALID_ID,OSAL_INVALID_ID};
struct nim_device *nim_mxl101[2] = {NULL,NULL};

static UINT32 tuner_cnt = 0;
/**************************************************************************************

****************************************************************************************/
#define SYS_TS_INPUT_INTERFACE   TSI_SPI_1
#define WAIT_TUNER_LOCK 200
#define WAIT_OFDM_LOCK  200
#define WAIT_FEC_LOCK     512

static UINT32 MXL101_ber = 0;
static UINT32 MXL101_per = 0;
static UINT32 MXL101_per_tot_cnt = 0;

__MAYBE_UNUSED__ static char nim_MXL101_name[HLD_MAX_NAME_SIZE] = "NIM_COFDM_0";


//#define MXL101_CHIP_ADRRESS 0xC0
#define MXL101_CHIP_ADRRESS 0xC6
/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ReadRegister
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|                 8/22/2008
--|
--| DESCRIPTION   : This function reads register data of the provided-address
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ReadRegister(UINT32 tuner_id,UINT8 regAddr, UINT8 *dataPtr)
{
    struct nim_MXL101_private *priv = (struct nim_MXL101_private *)nim_mxl101[tuner_id]->priv;
    struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    MXL_STATUS status = MXL_TRUE;
    INT32 err = 0;

    // User should implememnt his own I2C read register routine corresponding to
    // his hardaware.

    UINT8 Read_Cmd[2]; 
    osal_semaphore_capture(f_IIC_Sema_ID[tuner_id],TMO_FEVR);
    /* read step 1. accroding to mxl5007 driver API user guide. */
    Read_Cmd[0] = 0xFB;
    Read_Cmd[1] = regAddr;

//    status=i2c_write(0, MXL101_CHIP_ADRRESS, Read_Cmd, 2);
//    status=i2c_read(0, MXL101_CHIP_ADRRESS, dataPtr, 1);
    err = i2c_write(cfg->i2c_type_id, cfg->i2c_base_addr, Read_Cmd, 2);
    if(err!=0)
    {
        NIM_MXL101_PRINTF("MxL Reg write : status  %d Addr - 0x%x, data - 0x%x\n",status, regAddr, *dataPtr);
        status = MXL_FALSE;
    }
    else
    {
        err = i2c_read(cfg->i2c_type_id, cfg->i2c_base_addr, dataPtr, 1);
        if(err!=0)
        {
            NIM_MXL101_PRINTF("MxL Reg Read : status  %d Addr - 0x%x, data - 0x%x\n",status, regAddr, *dataPtr);
            status = MXL_FALSE;
        }
    }
    osal_semaphore_release(f_IIC_Sema_ID[tuner_id]);

    return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_WriteRegister
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function writes the provided value to the specified
--|                 address.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_WriteRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 regData)
{
    struct nim_MXL101_private *priv = (struct nim_MXL101_private *)nim_mxl101[tuner_id]->priv;
    struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    MXL_STATUS status = MXL_TRUE;
    INT32 err = 0;
    UINT8 Write_Cmd[2];

    osal_semaphore_capture(f_IIC_Sema_ID[tuner_id],TMO_FEVR);

    Write_Cmd[0] = regAddr;
    Write_Cmd[1] = regData;
    // User should implememnt his own I2C write register routine corresponding to
    // his hardaware.
    //status = i2c_write(0, MXL101_CHIP_ADRRESS, Write_Cmd, 2);
    err = i2c_write(cfg->i2c_type_id, cfg->i2c_base_addr, Write_Cmd, 2);
    if(err!=0)
    {
        NIM_MXL101_PRINTF("MxL Reg Write : status %d Addr - 0x%x, data - 0x%x\n", status,regAddr, regData);
        status = MXL_FALSE;
    }
    osal_semaphore_release(f_IIC_Sema_ID[tuner_id]);
    return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_Sleep
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/31/2009
--|
--| DESCRIPTION   : This function will cause the calling thread to be suspended
--|                 from execution until the number of milliseconds specified by
--|                 the argument time has elapsed
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_Sleep(UINT16 TimeinMilliseconds)
{
  MXL_STATUS status = MXL_TRUE;

  // User should implememnt his own sleep routine corresponding to
  // his Operating System platform.
  osal_task_sleep(TimeinMilliseconds);

  //NIM_MXL101_PRINTF("Ctrl_Sleep : %d msec's\n", TimeinMilliseconds);
  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_GetTime
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/05/2009
--|
--| DESCRIPTION   : This function will return current system's timestamp in
--|                 milliseconds resolution.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_GetTime(UINT32 *TimeinMilliseconds)
{
  //*TimeinMilliseconds = current systems timestamp in milliseconds.
  // User should implement his own routine to get current system's timestamp in milliseconds.
  *TimeinMilliseconds=osal_get_tick();
  return MXL_TRUE;
}


/**********************************/
//MxL101SF_PhyCfg.cpp
REG_CTRL_INFO_T MxL_101OverwriteDefault[] =
{
  {0xC8, 0xFF, 0x40},
  {0x8D, 0x01, 0x01},
  {0x42, 0xFF, 0x33},
  {0x71, 0xFF, 0x66},
  {0x72, 0xFF, 0x01},
  {0x73, 0xFF, 0x00},
  {0x74, 0xFF, 0xC2},
  {0x71, 0xFF, 0xE6},
  {0x83, 0xFF, 0x64},
  {0x85, 0xFF, 0x64},
  {0x88, 0xFF, 0xF0},
  {0x6F, 0xFF, 0xB7},
  {0x98, 0xFF, 0x40},
  {0x99, 0xFF, 0x18},
  {0xE0, 0xFF, 0x00},
  {0xE1, 0xFF, 0x10},
  {0xE2, 0xFF, 0x91},
  {0xE4, 0xFF, 0x3F},
  {0xE9, 0xFF, 0xFF},
  {0xEA, 0xFF, 0x07},
  {0xB1, 0xFF, 0x00},
  {0xB2, 0xFF, 0x09},
  {0xB3, 0xFF, 0x1F},
  {0xB4, 0xFF, 0x1F},
  {0x00, 0xFF, 0x01},
  {0xE2, 0xFF, 0x02},
  {0x81, 0xFF, 0x05},
  {0xF4, 0xFF, 0x07},
  {0xD4, 0x1F, 0x2F},
  {0xD6, 0xFF, 0x0C},
  {0xB8, 0xFF, 0x42},
  {0xBF, 0xFF, 0x1C},
  {0x82, 0xFF, 0xC6},
  {0x84, 0xFF, 0x8C},
  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0x6C},
  {0x87, 0xFF, 0x01},
  {0x00, 0xFF, 0x00},
  {0, 0, 0}
};

REG_CTRL_INFO_T MxL_101SuspendMode[] =
{
  {0x1C, 0xFF, 0x00},
  {0x04, 0xFF, 0x02},
  {0x02, 0xFF, 0x00},
  {0x01, 0xFF, 0x00},
  {0x06, 0xFF, 0x00},
  {0x09, 0xFE, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_101WakeUp[] =
{
  {0x04, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_MpegDataOutToTSIF[] =
{
  {0x17, 0xC0, 0xC0},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_MpegDataIn[] =
{
  {0x17, 0xC0, 0x40},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_PhySoftReset[] =
{
  {0xFF, 0xFF, 0x00},
  {0x02, 0xFF, 0x01},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_TunerDemodMode[] =
{
  {0x03, 0xFF, 0x01},
  {0x7D, 0x40, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_TunerMode[] =
{
  {0x03, 0xFF, 0x00},
  {0x7D, 0x40, 0x40},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_TopMasterEnable[] =
{
  {0x01, 0xFF, 0x01},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_TopMasterDisable[] =
{
  {0x01, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_PhyTuneRF[] =
{
  {0x1D, 0x7F, 0x00},
  {0x1E, 0xFF, 0x00},
  {0x1F, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_StartTune[] =
{
  {0x00, 0xFF, 0x01},
  {0x81, 0x1C, 0x04},
  {0x83, 0x1C, 0x0F},
  {0x85, 0x1C, 0x02},
  {0x00, 0xFF, 0x00},
  {0x1C, 0xFF, 0x01},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_IrqClear[] =
{
  {0x0E, 0xFF, 0xFF},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_ResetPerCount[] =
{
  {0x20, 0x01, 0x01},
  {0x20, 0x01, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_CableSettings[] =
{
  {0x0A, 0x10, 0x10},
  {0x0A, 0x0F, 0x04},
  {0x3F, 0x3F, 0x01},
  {0x44, 0xF0, 0x60},
  {0x46, 0xF0, 0x10},
  {0x48, 0xF0, 0x70},
  {0x48, 0x0F, 0x0C},
  {0x0D, 0x03, 0x02},
  {0x4D, 0xFF, 0x40},
  {0x69, 0x01, 0x01},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_EnableCellId[] =
{
  {0x00, 0xFF, 0x01},
  {0x7E, 0xFF, 0x05},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_DisableCellId[] =
{
  {0x00, 0xFF, 0x01},
  {0x7E, 0xFF, 0x04},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_EnableChanScan[] =
{
  {0x00, 0xFF, 0x01},
  {0x81, 0xFF, 0x04},
  {0xF4, 0xFF, 0x03},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_DisableChanScan[] =
{
  {0x00, 0xFF, 0x01},
  {0x81, 0xFF, 0x05},
  {0xF4, 0xFF, 0x07},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_EnableHighTempRange[] =
{
  {0x00, 0xFF, 0x01},
  {0x6C, 0xFF, 0xAC},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_DisableHighTempRange[] =
{
  {0x00, 0xFF, 0x01},
  {0x6C, 0xFF, 0x2C},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_Config6MHzBandwidth[] =
{
  {0x00, 0xFF, 0x01},
  {0x84, 0xFF, 0xA1},
  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0x93},
  {0x87, 0xFF, 0x01},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_Config7MHzBandwidth[] =
{
  {0x00, 0xFF, 0x01},
  {0x84, 0xFF, 0x96},
  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0xA0},
  {0x87, 0xFF, 0x01},
  {0x00, 0xFF, 0x00},
  {0,    0,    0}
};

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ProgramRegisters
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|
--| DESCRIPTION   : This function writes multiple registers with provided data array.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ProgramRegisters(UINT32 tuner_id, PREG_CTRL_INFO_T ctrlRegInfoPtr)
{
  UINT8 status = MXL_TRUE;
  UINT16 i;
  UINT8 tmp;

  for (i = 0; ctrlRegInfoPtr[i].regAddr|ctrlRegInfoPtr[i].mask|ctrlRegInfoPtr[i].data; i++)
  {
    // Check if partial bits of register were updated
    if (ctrlRegInfoPtr[i].mask != 0xFF)
    {
      status = Ctrl_ReadRegister(tuner_id, ctrlRegInfoPtr[i].regAddr, &tmp);
      if (status != MXL_TRUE) break;;
    }

    tmp &= ~ctrlRegInfoPtr[i].mask;
    tmp |= ctrlRegInfoPtr[i].data;

    status |= Ctrl_WriteRegister(tuner_id, ctrlRegInfoPtr[i].regAddr, tmp);
    if (status != MXL_TRUE) break;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_PhyTune
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|                 5/4/2010 - Mahee
--|
--| DESCRIPTION   : Tune to the specified RF frequency with bandwidth
--|
--|                 The provided input frequency and bandwidth's unit is MHz.
--|                 Changed to increase frequency resolution.
--|
--| RETURN VALUE  : PREG_CTRL_INFO_T
--|
--|---------------------------------------------------------------------------*/

PREG_CTRL_INFO_T Ctrl_PhyTune(UINT32 frequency, UINT8 bandwidth)
{
  UINT32 freq;
  UINT8 filt_bw;

  // Set Channel Bandwidth
  switch (bandwidth)
  {
    case 6:
      filt_bw = 21;
      break;

    case 7:
      filt_bw = 42;
      break;

    case 8:
      filt_bw = 63;
      break;

    default:
      filt_bw = 0x63;
      break;
  }

  // Calculate RF Channel
  frequency /= 15625;

  // Do round
  freq = frequency;

  // Abort Tune. This control is in the arrary
  // Set Bandwidth
  MxL_PhyTuneRF[0].data = filt_bw;

  // Set RF
  MxL_PhyTuneRF[1].data = (freq & 0xFF);       // Fractional part
  MxL_PhyTuneRF[2].data = (freq >> 8 ) & 0xFF; // Integer part

  // Start Tune. This control is in the arrary
  return MxL_PhyTuneRF;
}
//MxL101SF_PhyCfg.cpp end

//MxL101SF_PhyCtrlApi.c
const UINT8 MxLCtrlVersion[] = {6, 3, 15, 1};

// Variable to store tuned frequency for RSSI calculation
UINT32 MxLTunedFreq = 0;

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_SoftResetDevice
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 9/8/2008
--|
--| DESCRIPTION   : By writing any value into address 0xFF (AIC), all control
--|                 registers are initialized to the default value.
--|                 AIC - Address Initiated Command
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_SoftResetDevice(UINT32 tuner_id)
{
  return Ctrl_ProgramRegisters(tuner_id, MxL_PhySoftReset);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_GetChipInfo
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 2/7/2008
--|
--| DESCRIPTION   : This function returns MxL101SF Chip Id and version information.
--|                 Chip Id of MxL101SF has 0x61
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_GetChipInfo(UINT32 tuner_id,PMXL_DEV_INFO_T DevInfoPtr)
{
  UINT8 status = MXL_FALSE;
  UINT8 readBack;

  status = Ctrl_ReadRegister(tuner_id, CHIP_ID_REG, &readBack);
  DevInfoPtr->DevId = readBack;

  status |= Ctrl_ReadRegister(tuner_id, TOP_CHIP_REV_ID_REG, &readBack);
  DevInfoPtr->DevVer = (readBack & 0x0F);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_SetMxLDeviceMode
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|
--| DESCRIPTION   : MxL101SF has a built-in RF Tuner in addtion to DVB-T BB block
--|                 In tuner only mode, Digial BB will be disabled.
--|                 MXL_TUNER_MODE : Tuner mode, MXL_SOC_MODE : SOC mode
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_SetMxLDeviceMode(UINT32 tuner_id, PMXL_DEV_MODE_CFG_T DevModePtr)
{
  UINT8 status = MXL_FALSE;

  // Set Device mode
  if (DevModePtr->DeviceMode == MXL_SOC_MODE)
    status = Ctrl_ProgramRegisters(tuner_id, MxL_TunerDemodMode);
  else if (DevModePtr->DeviceMode == MXL_TUNER_MODE)
    status = Ctrl_ProgramRegisters(tuner_id, MxL_TunerMode);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TopMasterControl
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 4/17/2009
--|
--| DESCRIPTION   : Power up Tuner
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TopMasterControl(UINT32 tuner_id, PMXL_TOP_MASTER_CFG_T TopMasterCtrlPtr)
{
  UINT8 status = MXL_FALSE;

  if (TopMasterCtrlPtr->TopMasterEnable == MXL_DISABLE)
    status = Ctrl_ProgramRegisters(tuner_id, MxL_TopMasterDisable);
  else
    status = Ctrl_ProgramRegisters(tuner_id, MxL_TopMasterEnable);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_InitTunerDemod
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : Initializing Tuner and Demod block of MxL101SF
--|                 before tuning to a certain frequency.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_InitTunerDemod(UINT32 tuner_id)
{
  return Ctrl_ProgramRegisters(tuner_id, MxL_101OverwriteDefault);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ControlMpegOutput
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 05/22/2010
--|
--| DESCRIPTION   : Enable TSIF for MPEG-2 data output
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ControlMpegOutput(UINT32 tuner_id, PMXL_TS_CTRL_T TsOutCtrlPtr)
{
  UINT8 status;
  UINT8 regData = 0;

  if (TsOutCtrlPtr->TsCtrl == MXL_ON) status = Ctrl_ProgramRegisters(tuner_id, MxL_MpegDataOutToTSIF);
  else status = Ctrl_ProgramRegisters(tuner_id, MxL_MpegDataIn);

  status |= Ctrl_ReadRegister(tuner_id,0x04, &regData);

  if (TsOutCtrlPtr->TsDriveStrength == TS_DRIVE_STRENGTH_2X) regData |= 0x01;
  else regData &= ~0x01;

  status |= Ctrl_WriteRegister(tuner_id, 0x04, regData);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigMpegOut
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|
--| DESCRIPTION   : For Transport stream output through ASI(MPEG TS) interface
--|                 the following parameters are needed to configure after or before
--|                 demod lock.
--|                  TS output mode : Seral or Parallel
--|                  CLK Phase : Normal or Inverted
--|                  MPEG Valid Polarity : Low or High
--|                  MPEG Sync Polarity  : Low or High
--|                  MPEG CLK Frequency  : 0 - 36.571429MHz
--|                                      : 1 - 2.285714MHz
--|                                      : 2 - 4.571429MHz
--|                                      : 3 - 6.857143MHz
--|                                      : 4 - 9.142857MHz
--|                                      : 5 - 13.714286MHz
--|                                      : 6 - 18.285714MHz
--|                                      : 7 - 27.428571MHz
--|                 * MxL101SF Feature
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigMpegOut(UINT32 tuner_id, PMXL_MPEG_CFG_T MpegOutCfgPtr)
{
  UINT8 status = MXL_TRUE;
  UINT8 mode, tmp;

  mode = 0xC0; // MPEG Out

  // Configure MPEG Clock phase
  if (MpegOutCfgPtr->MpegClkPhase == MPEG_CLK_IN_PHASE)
    mode &= ~V6_INVERTED_CLK_PHASE;
  else
    mode |= V6_INVERTED_CLK_PHASE;;

  if (MpegOutCfgPtr->MpegClkFreq > MPEG_CLOCK_27_428571MHz)
    MpegOutCfgPtr->MpegClkFreq = MPEG_CLOCK_27_428571MHz;

  mode |= (((UINT8)MpegOutCfgPtr->MpegClkFreq) << 2);

  status = Ctrl_WriteRegister(tuner_id, V6_MPEG_OUT_CLK_INV_REG, mode);

  // Configure data input mode, MPEG Valid polarity, MPEG Sync polarity
  // Get current configuration
  status |= Ctrl_ReadRegister(tuner_id,V6_MPEG_OUT_CTRL_REG, &mode);
  mode &= 0xF0;

  // Data Input mode
  if (MpegOutCfgPtr->SerialOrPar == MPEG_DATA_PARALLEL)
  {
    mode |= V6_MPEG_DATA_PARALLEL;
  }
  else
  {
    mode |= V6_MPEG_DATA_SERIAL;

    // If serial interface is selected, configure MSB or LSB order in transmission
    status |= Ctrl_ReadRegister(tuner_id,V6_MPEG_INOUT_BIT_ORDER_CTRL_REG, &tmp);

    if (MpegOutCfgPtr->LsbOrMsbFirst == MPEG_SERIAL_MSB_1ST)
      tmp |= V6_MPEG_SER_MSB_FIRST;
    else
      tmp &= ~V6_MPEG_SER_MSB_FIRST; // LSB First

    status |= Ctrl_WriteRegister(tuner_id, V6_MPEG_INOUT_BIT_ORDER_CTRL_REG, tmp);
  }

  // MPEG Sync polarity
  if (MpegOutCfgPtr->MpegSyncPol == MPEG_CLK_IN_PHASE)
    mode &= ~V6_INVERTED_MPEG_SYNC;
  else
    mode |= V6_INVERTED_MPEG_SYNC;

  // MPEG Valid polarity
  if (MpegOutCfgPtr->MpegValidPol == MPEG_CLK_IN_PHASE)
    mode &= ~V6_INVERTED_MPEG_VALID;
  else
    mode |= V6_INVERTED_MPEG_VALID;

  status |= Ctrl_WriteRegister(tuner_id, V6_MPEG_OUT_CTRL_REG, mode);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigDevPowerSavingMode
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/20/2009
--|
--| DESCRIPTION   : This function configures Standby mode and Sleep mode to
--|                 control power consumption.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigDevPowerSavingMode(UINT32 tuner_id, PMXL_PWR_MODE_CFG_T PwrModePtr)
{
  UINT8 status = MXL_FALSE;

  switch (PwrModePtr->PowerMode)
  {
    case STANDBY_ON:
      status = Ctrl_ProgramRegisters(tuner_id, MxL_TopMasterDisable);
      status |= Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0x01);
      status |= Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0x00);
      break;

    case SLEEP_ON:
      status = Ctrl_ProgramRegisters(tuner_id, MxL_101SuspendMode);
      break;

    case STANDBY_OFF:
      status = Ctrl_ProgramRegisters(tuner_id, MxL_TopMasterEnable);
      status = Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0x01);
      break;

    case SLEEP_OFF:
      status = Ctrl_ProgramRegisters(tuner_id, MxL_101WakeUp);
      status |= MxL101SF_API_SoftResetDevice(tuner_id);
      status |= MxL101SF_API_InitTunerDemod(tuner_id);
      break;

    default:
      break;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ChanTuneCfg
--|
--| AUTHOR        : Mahendra Kondur - updated new topmastercontrol function name
--|
--| DATE CREATED  : 9/4/2010
--|
--| DESCRIPTION   : This function shall be called during by MxL101SF_API_TuneRF
--|                 API to perform necessary channel tunning optimization.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ChanTuneCfg(UINT32 tuner_id, PMXL_TUNER_CHAN_CONFIG_T ChanCfgPtr)
{
  UINT8 status = MXL_TRUE;
  UINT8 regData;
  UINT32 cpStartTime;
  UINT32 cpEndTime;
  MXL_DEMOD_LOCK_STATUS_T cpLock;
  UINT8 FlagCheck = 0;
  UINT8 cpLockFlag = 0;

  Ctrl_GetTime(&cpStartTime);
  cpEndTime = cpStartTime;

  ChanCfgPtr->Timeout = 768;

  while ((cpEndTime - ChanCfgPtr->StartTime) < ChanCfgPtr->Timeout)
  {
    // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
    status |= Ctrl_ReadRegister(tuner_id, 0x25, &regData);
    ChanCfgPtr->AgcGain = regData;
    regData = 0;
    status |= Ctrl_ReadRegister(tuner_id, 0x26, &regData);
    ChanCfgPtr->AgcGain |= (regData & 0x1F) << 8;

    if (ChanCfgPtr->TpsCellId)
    {
      if (ChanCfgPtr->AgcGain <= 6208) ChanCfgPtr->Timeout = 2560;
      else ChanCfgPtr->Timeout = 1536;
    }

    status = MxLWare_API_GetDemodStatus(tuner_id, MXL_DEMOD_CP_LOCK_REQ, &cpLock);

    if (cpLock.Status == MXL_LOCKED)
    {
      if (!cpLockFlag)
      {
        cpLockFlag = 1;

        status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x01);
        status |= Ctrl_ReadRegister(tuner_id, 0x81, &regData);
        regData = regData & 0xE3;
        status |= Ctrl_WriteRegister(tuner_id, 0x81, regData);
        status |= Ctrl_WriteRegister(tuner_id, 0x83, 0x1F);
        status |= Ctrl_WriteRegister(tuner_id, 0x85, 0x06);
        status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x00);
      }

      if (ChanCfgPtr->TpsCellId)
      {
        if ((cpEndTime - cpStartTime) > 620)
          break;
      }
      else // end of TPS Cell ID check
      {
        break;
      }
    }
    else // CP lock else
    {
      if (!FlagCheck)
      {
        if ((cpEndTime - ChanCfgPtr->StartTime) >= 768)
        {
          FlagCheck = 1;

          if (ChanCfgPtr->AgcGain <= 6208)
          {
            status |= Ctrl_WriteRegister(tuner_id,START_TUNE_REG, 0);

            status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x01);
            status |= Ctrl_ReadRegister(tuner_id, 0x81, &regData);
            regData = regData & 0xE3;
            status |= Ctrl_WriteRegister(tuner_id, 0x81, regData);
            status |= Ctrl_WriteRegister(tuner_id, 0x83, 0x1F);
            status |= Ctrl_WriteRegister(tuner_id, 0x85, 0x06);
            status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x00);

            status |= Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 1);

          }
        }
      }

      if (ChanCfgPtr->TpsCellId)
        Ctrl_GetTime(&cpStartTime);

    } // End of CP lock

    Ctrl_GetTime(&cpEndTime);
  }

  if (ChanCfgPtr->TpsCellId)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_DisableCellId);
  else
  {
    if ((FlagCheck == 0) && ((cpEndTime - ChanCfgPtr->StartTime) >= 768))
    {
      if (ChanCfgPtr->AgcGain <= 6208)
      {
        status |= Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0);

        status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x01);
        status |= Ctrl_ReadRegister(tuner_id, 0x81, &regData);
        regData = regData & 0xE3;
        status |= Ctrl_WriteRegister(tuner_id, 0x81, regData);
        status |= Ctrl_WriteRegister(tuner_id, 0x83, 0x1F);
        status |= Ctrl_WriteRegister(tuner_id, 0x85, 0x06);
        status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x00);

        status |= Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 1);
      }
    }
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TuneRF
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur - updated new topmastercontrol function name
--|
--| DATE CREATED  : 1/15/2008
--|                 10/23/2009
--|                 11/20/2009 - Settings for TPS Cell ID
--|                 9/4/2010 - Break the function into addition API
--|                            (MxL101SF_API_ChanTuneCfg) for tunning optimization
--|
--| DESCRIPTION   : After power-on initialization, when channel frequency has
--|                 changed, this function shall be called
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TuneRF(UINT32 tuner_id, PMXL_RF_TUNE_CFG_T TuneParamPtr)
{
  PREG_CTRL_INFO_T ctrlArrayPtr;
  MXL_TUNER_CHAN_CONFIG_T ChanTuneCfg;
  UINT8 status = MXL_FALSE;
  UINT8 mxl_mode;
  UINT8 regData = 0;
  UINT32 StartTime;
  UINT16 TimeOut = 768;

  // Stop Tune
  status = Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0);

  // Check Device mode
  status |= Ctrl_ReadRegister(tuner_id, MXL_MODE_REG, &mxl_mode);

  // Configure settings for 6 MHz Bandwidth
  if (TuneParamPtr->Bandwidth == 6)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_Config6MHzBandwidth);

  // Configure settings for 7 MHz Bandwidth
  if (TuneParamPtr->Bandwidth == 7)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_Config7MHzBandwidth);

  // Fill out registers for channel tune
  ctrlArrayPtr = Ctrl_PhyTune(TuneParamPtr->Frequency, TuneParamPtr->Bandwidth);

  if (!ctrlArrayPtr) return MXL_FALSE;

  // Program registers
  status |= Ctrl_ProgramRegisters(tuner_id, ctrlArrayPtr);

  // Enable TPS Cell ID feature
  if (TuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_EnableCellId);

  // Start tune
  status |= Ctrl_ProgramRegisters(tuner_id, MxL_StartTune);

  if ((mxl_mode & MXL_DEV_MODE_MASK) == MXL_SOC_MODE)
  {
    // Get timestamp
    Ctrl_GetTime(&StartTime);

    // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
    status |= Ctrl_ReadRegister(tuner_id, 0x25, &regData);
    ChanTuneCfg.AgcGain = regData;
    regData = 0;
    status |= Ctrl_ReadRegister(tuner_id, 0x26, &regData);
    ChanTuneCfg.AgcGain |= (regData & 0x1F) << 8;

    ChanTuneCfg.TpsCellId = TuneParamPtr->TpsCellIdRbCtrl;
    ChanTuneCfg.StartTime = StartTime;
    ChanTuneCfg.Timeout = TimeOut;

    MxL101SF_API_ChanTuneCfg(tuner_id, &ChanTuneCfg);
  }

  MxLTunedFreq = TuneParamPtr->Frequency;
  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TuneRFExt
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 7/26/2010
--|
--| DESCRIPTION   : Optional Tune RF API for better performance under higher
--|                 temperature range.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/
/*
MXL_STATUS MxL101SF_API_TuneRFExt(PMXL_RF_TUNE_CFG_T TuneParamPtr)
{
  PREG_CTRL_INFO_T ctrlArrayPtr;
  MXL_TUNER_CHAN_CONFIG_T ChanTuneCfg;
  UINT8 status = MXL_FALSE;
  UINT8 mxl_mode;
  UINT8 regData = 0;
  UINT32 StartTime;
  UINT16 TimeOut = 768;

  // Stop Tune
  status = Ctrl_WriteRegister(START_TUNE_REG, 0);

  // Check Device mode
  status |= Ctrl_ReadRegister(MXL_MODE_REG, &mxl_mode);

  // Configure settings for 6 MHz Bandwidth
  if (TuneParamPtr->Bandwidth == 6)
    status |= Ctrl_ProgramRegisters(MxL_Config6MHzBandwidth);

  // Configure settings for 7 MHz Bandwidth
  if (TuneParamPtr->Bandwidth == 7)
    status |= Ctrl_ProgramRegisters(MxL_Config7MHzBandwidth);

  // Fill out registers for channel tune
  ctrlArrayPtr = Ctrl_PhyTune(TuneParamPtr->Frequency, TuneParamPtr->Bandwidth);

  if (!ctrlArrayPtr) return MXL_FALSE;

  // Program registers
  status |= Ctrl_ProgramRegisters(ctrlArrayPtr);

  // Enable TPS Cell ID feature
  if (TuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE)
    status |= Ctrl_ProgramRegisters(MxL_EnableCellId);

  // Start tune
  status |= Ctrl_ProgramRegisters(MxL_EnableHighTempRange);
  status |= Ctrl_ProgramRegisters(MxL_StartTune);

  // Get timestamp
  Ctrl_GetTime(&StartTime);

  // Sleep for 32 msec's
  Ctrl_Sleep(32);

  status |= Ctrl_ProgramRegisters(MxL_DisableHighTempRange);

  if ((mxl_mode & MXL_DEV_MODE_MASK) == MXL_SOC_MODE)
  {
    // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
    status |= Ctrl_ReadRegister(0x25, &regData);
    ChanTuneCfg.AgcGain = regData;
    regData = 0;
    status |= Ctrl_ReadRegister(0x26, &regData);
    ChanTuneCfg.AgcGain |= (regData & 0x1F) << 8;

    ChanTuneCfg.TpsCellId = TuneParamPtr->TpsCellIdRbCtrl;
    ChanTuneCfg.StartTime = StartTime;
    ChanTuneCfg.Timeout = TimeOut;

    MxL101SF_API_ChanTuneCfg(&ChanTuneCfg);
  }

  MxLTunedFreq = TuneParamPtr->Frequency;
  return (MXL_STATUS)status;
}
*/
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetSNR
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur - Support for Interger calculation of SNR
--|
--| DATE CREATED  : 1/24/2008
--|                 1/12/2010
--|
--| DESCRIPTION   : This function returns SNR(Signal to Noise Ratio).
--|                 SNR is calculated as follows after reading 10bit register
--|                 Folting-point calculation:
--|                  SNR = 10 * SNR_REG_VALUE / 64 - 2.5  dB
--|                 Integer calculation:
--|                  10000 x SNR = 1563 x SNR_REG_VALUE - 25000  dB
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetSNR(UINT32 tuner_id, PMXL_DEMOD_SNR_INFO_T SnrPtr)
{
  UINT8 status = MXL_FALSE;
  UINT32 snrCalc;
  UINT16 snrData;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_SNR_RB_LSB_REG, &rawData);
  snrData = rawData;

  status |= Ctrl_ReadRegister(tuner_id, V6_SNR_RB_MSB_REG, &rawData);
  snrData |= (rawData & 0x03) << 8;

  if (status == MXL_TRUE)
  {
    snrCalc =  CALCULATE_SNR(snrData);
    SnrPtr->SNR = snrCalc;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetBER
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur - Support for Interger calculation of BER
--|
--| DATE CREATED  : 1/24/2008
--|                 2/24/2010
--|
--| DESCRIPTION   : This function returns BER(Bit Error Ratio).
--|                 Floating-point calculation
--|                  BER = (AVG_ERRORS x 4)/(N_ACCUMULATE x 64 x 188 x 8)
--|                 Integer calculation
--|                  1e10 x BER = (AVG_ERRORS x 4 x 3247)
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetBER(UINT32 tuner_id, PMXL_DEMOD_BER_INFO_T BerInfoPtr)
{
  UINT8 status;
  UINT32 berCalc;
  UINT32 n_accumulate;
  UINT16 avg_errors;
  UINT8 rawData;

  avg_errors = 0;
  status = Ctrl_ReadRegister(tuner_id, V6_RS_AVG_ERRORS_LSB_REG, &rawData);
  avg_errors = rawData;

  status |= Ctrl_ReadRegister(tuner_id, V6_RS_AVG_ERRORS_MSB_REG, &rawData);
  avg_errors |= (rawData << 8);

  status |= Ctrl_ReadRegister(tuner_id, V6_N_ACCUMULATE_REG, &rawData);
  n_accumulate = rawData;

  berCalc = CALCULATE_BER(avg_errors, n_accumulate);

  if (status == MXL_TRUE)
  {
    BerInfoPtr->BER = berCalc;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetSyncLockStatus
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 5/6/2009
--|
--| DESCRIPTION   : This function return SYNC_LOCK Status
--|                 if 1, Locked
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetSyncLockStatus(UINT32 tuner_id, PMXL_DEMOD_LOCK_STATUS_T SyncLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_SYNC_LOCK_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if ((rawData & SYNC_LOCK_MASK) >> 4)
      SyncLockPtr->Status = MXL_LOCKED;
    else
      SyncLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetRsLockStatus
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 4/30/2008
--|
--| DESCRIPTION   : This function returns RS Lock status
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetRsLockStatus(UINT32 tuner_id, PMXL_DEMOD_LOCK_STATUS_T RsLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_RS_LOCK_DET_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if ((rawData & RS_LOCK_DET_MASK) >> 3)
      RsLockPtr->Status = MXL_LOCKED;
    else
      RsLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetCpLockStatus
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 4/30/2008
--|
--| DESCRIPTION   : This function returns CP Lock status
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetCpLockStatus(UINT32 tuner_id, PMXL_DEMOD_LOCK_STATUS_T CpLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_CP_LOCK_DET_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if((rawData & V6_CP_LOCK_DET_MASK) >> 2)
      CpLockPtr->Status = MXL_LOCKED;
    else
      CpLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodResetIrqStatus
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|                 9/09/2008
--|
--| DESCRIPTION   : This function clears IRQ status registers
--|                 Writing 0xFF to this register will clear
--|                 the previous status
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodResetIrqStatus(UINT32 tuner_id)
{
  return Ctrl_ProgramRegisters(tuner_id, MxL_IrqClear);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsCodeRate
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function returns code rate of TPS parameters
--|                 bit<2:0> - 000:1/2, 001:2/3, 010:3/4, 011:5/6, 100:7/8
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsCodeRate(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsCodeRatePtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_CODE_RATE_TPS_REG, &rawData);

  if (status == MXL_TRUE)
    TpsCodeRatePtr->TpsInfo = rawData & V6_CODE_RATE_TPS_MASK;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsHierarchy
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function returns Hiearchy information of TPS parameters
--|                 bit<6:4> - 000:Non hierarchy, 001:1, 010:2, 011:4
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsHierarchy(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsHierarchyPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_TPS_HIERACHY_REG, &rawData);

  if (status == MXL_TRUE)
    TpsHierarchyPtr->TpsInfo = (rawData & V6_TPS_HIERARCHY_INFO_MASK) >> 6;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigStreamPriority
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 1/14/2010
--|
--| DESCRIPTION   : This API will config High priority or Low priority stream
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigStreamPriority(UINT32 tuner_id, PMXL_DEMOD_TS_PRIORITY_CFG_T TsPriorityPtr)
{
  UINT8 status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_TPS_HPORLP_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if (TsPriorityPtr->StreamPriority == LP_STREAM)
      rawData = (rawData | 0x80);
    else
      rawData = (rawData & 0x7F);

    status |= Ctrl_WriteRegister(tuner_id, V6_TPS_HPORLP_REG, rawData);
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetHierarchicalAlphaValue
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 1/14/2010
--|
--| DESCRIPTION   : This function returns Hierarchical Alpha of TPS parameters
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetHierarchicalAlphaValue(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsAlphaPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_TPS_ALPHA_REG, &rawData);

  if (status == MXL_TRUE)
    TpsAlphaPtr->TpsInfo = (rawData & V6_TPS_ALPHA_MASK);

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsConstellation
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function return Constellation status bit information
--|                 Constellation, 00 : QPSK, 01 : 16QAM, 10:64QAM
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsConstellation(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsConstellationPtr)
{
  MXL_STATUS status;
  UINT8 tpsParams;

  status = Ctrl_ReadRegister(tuner_id, V6_MODORDER_TPS_REG, &tpsParams);

  if (status == MXL_TRUE)
    TpsConstellationPtr->TpsInfo = (tpsParams & V6_PARAM_CONSTELLATION_MASK) >> 4 ;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsFftMode
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function return FFT Mode status bit information
--|                 FFT Mode, 00:2K, 01:8K, 10:4K
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsFftMode(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsFftModePtr)
{
  MXL_STATUS status;
  UINT8 tpsParams;

  status = Ctrl_ReadRegister(tuner_id, V6_MODE_TPS_REG, &tpsParams);

  if (status == MXL_TRUE)
    TpsFftModePtr->TpsInfo = (tpsParams & V6_PARAM_FFT_MODE_MASK) >> 2 ;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsGuardInterval
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : This function return GI status bit information
--|                 00:1/32, 01:1/16, 10:1/8, 11:1/4
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsGuardInterval(UINT32 tuner_id, PMXL_DEMOD_TPS_INFO_T TpsGIPtr)
{
  MXL_STATUS status;
  UINT8 tpsParams;

  status = Ctrl_ReadRegister(tuner_id, V6_CP_TPS_REG, &tpsParams);

  if (status == MXL_TRUE)
    TpsGIPtr->TpsInfo = (tpsParams & V6_PARAM_GI_MASK) >> 4 ;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsLock
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/29/2008
--|
--| DESCRIPTION   : This function returns Tps lock status bit information.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsLock(UINT32 tuner_id, PMXL_DEMOD_LOCK_STATUS_T TpsLockPtr)
{
  MXL_STATUS status;
  UINT8 tpsParams;

  status = Ctrl_ReadRegister(tuner_id, V6_TPS_LOCK_REG, &tpsParams);

  if (status == MXL_TRUE)
  {
    if((tpsParams & V6_PARAM_TPS_LOCK_MASK) >> 6)
      TpsLockPtr->Status = MXL_LOCKED;
    else
      TpsLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetPacketErrorCount
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 3/13/2008
--|
--| DESCRIPTION   : This function returns TS Packet error count.
--|
--|                  PER Count = FEC_PER_COUNT * (2 ** (FEC_PER_SCALE * 4))
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetPacketErrorCount(UINT32 tuner_id, PMXL_DEMOD_PEC_INFO_T PecInfoPtr)
{
  UINT8 status;
  UINT32 fec_per_count, fec_per_scale;
  UINT8 rawData;

  // FEC_PER_COUNT Register
  status = Ctrl_ReadRegister(tuner_id, V6_FEC_PER_COUNT_REG, &rawData);
  fec_per_count = rawData;

  // FEC_PER_SCALE Register
  status |= Ctrl_ReadRegister(tuner_id, V6_FEC_PER_SCALE_REG, &rawData);

  rawData &= V6_FEC_PER_SCALE_MASK;
  rawData *= 4;

  fec_per_scale = 1 << rawData;

  fec_per_count *= fec_per_scale;

  PecInfoPtr->PEC = fec_per_count;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodResetPacketErrorCount
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 3/13/2008
--|
--| DESCRIPTION   : This function resets TS Packet error count.
--|
--|                 After setting 7th bit of V5_PER_COUNT_RESET_REG,
--|                 it should be reset to 0.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodResetPacketErrorCount(UINT32 tuner_id)
{
  return Ctrl_ProgramRegisters(tuner_id, MxL_ResetPerCount);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerGetLockStatus
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/3/2008
--|
--| DESCRIPTION   : This function provides RF synthesizer lock status
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerGetLockStatus(UINT32 tuner_id, PMXL_TUNER_LOCK_STATUS_T TunerLockStatusPtr)
{
  MXL_STATUS status;
  UINT8 data;

  // Initialize
  TunerLockStatusPtr->RefSynthLock = MXL_UNLOCKED;
  TunerLockStatusPtr->RfSynthLock = MXL_UNLOCKED;

  status = Ctrl_ReadRegister(tuner_id, V6_RF_LOCK_STATUS_REG, &data);

  if (status == MXL_TRUE)
  {
    if ((data & 3) == 3)
      TunerLockStatusPtr->RefSynthLock = MXL_LOCKED;;  // Locked

    if ((data & 0xc) == 0xc)
      TunerLockStatusPtr->RfSynthLock = MXL_LOCKED;;  // Locked
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerSetIFOutputFreq
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/7/2008
--|                 2/24/2010
--|
--| DESCRIPTION   : In Tuner only mode of MxL101SF, this function configures
--|                 IF output frequency
--|                 Support for IF Frequency in integer format
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerSetIFOutputFreq(UINT32 tuner_id, PMXL_TUNER_IF_FREQ_T IfFreqParamPtr)
{
  UINT8 status;
  UINT8 control;
  UINT16 IFFCW;

  // Set IF polarity
  if (IfFreqParamPtr->IF_Polarity == TUNER_NORMAL_IF_SPECTRUM)
    control = TUNER_NORMAL_IF_SPECTRUM;
  else
    control = TUNER_INVERT_IF_SPECTRUM;

  if (IfFreqParamPtr->IF_Index <= IF_OTHER_35MHZ_45MHZ)
    control |= (UINT8)IfFreqParamPtr->IF_Index;

  status = Ctrl_WriteRegister(tuner_id, V6_TUNER_IF_SEL_REG, control);

  if (IfFreqParamPtr->IF_Index == IF_OTHER_12MHZ)
  {
    control = 0x08;
    IfFreqParamPtr->IF_Freq = (IfFreqParamPtr->IF_Freq/108) * 4096;
    IfFreqParamPtr->IF_Freq += 500000;
    IfFreqParamPtr->IF_Freq /= 1000000;
    IFFCW = (UINT16)(IfFreqParamPtr->IF_Freq);
  }
  else if (IfFreqParamPtr->IF_Index == IF_OTHER_35MHZ_45MHZ)
  {
    control = 0x08;
    IfFreqParamPtr->IF_Freq = (IfFreqParamPtr->IF_Freq/216) * 4096;
    IfFreqParamPtr->IF_Freq += 500000;
    IfFreqParamPtr->IF_Freq /= 1000000;
    IFFCW = (UINT16)(IfFreqParamPtr->IF_Freq);
  }
  else
  {
    control = 0;
    IFFCW = 0;
  }

  control |= (IFFCW >> 8);
  status |= Ctrl_WriteRegister(tuner_id, V6_TUNER_IF_FCW_BYP_REG, control);

  control = IFFCW & 0x00FF;
  status |= Ctrl_WriteRegister(tuner_id, V6_TUNER_IF_FCW_REG, control);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerLoopThruControl
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : If loop through mode is enabled, RF signal from the antenna
--|                 is looped through to an external demodulator.
--|                  0 : Disable, 1: Enable,
--|                 API is reorganised according to the MxL101SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerLoopThruControl(UINT32 tuner_id,MXL_BOOL EnableDisable)
{
  MXL_STATUS status = MXL_TRUE;
  UINT8 control;

  if (EnableDisable == MXL_DISABLE) control = 0; // Disable
  else control = 1;             // Enable

  status = Ctrl_WriteRegister(tuner_id,V6_TUNER_LOOP_THRU_CTRL_REG, control);

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_XtalSelect
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : Select XTAL frequency of CLK out
--|                 4 : 24MHz, 8 : 28.8MHz, 7 : 27 MHz, 12 : 48 MHz
--|                 API is reorganised according to the MxL1x1SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_XtalSelect(UINT32 tuner_id, MXL_XTAL_FREQ_E XtalFreq)
{
  UINT8 status = MXL_TRUE;

  status = Ctrl_WriteRegister(tuner_id,V6_DIG_CLK_FREQ_SEL_REG, (UINT8)XtalFreq);
  status |= Ctrl_WriteRegister(tuner_id,V6_DIG_RFREFSELECT_REG, ((UINT8)XtalFreq)|0xA0);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_XtalClkOutGain
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : If Xtal Clock out is enabled, then a valid clk out gain value
--|                 should be programmed to the chip.
--|                 API is reorganised according to the MxL1x1SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_XtalClkOutGain(UINT32 tuner_id,MXL_XTAL_CLK_OUT_GAIN_E ClkOutGain)
{
  MXL_STATUS Status = MXL_TRUE;

  if (ClkOutGain < CLK_OUT_NA)
    Status = Ctrl_WriteRegister(tuner_id,V6_XTAL_CLK_OUT_GAIN_REG, (UINT8)ClkOutGain);

  return Status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_XtalClkOutControl
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : XTAL Clock out control
--|                  0 : Disable, 1: Enable,
--|                 API is reorganised according to the MxL1x1SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_XtalClkOutControl(UINT32 tuner_id, MXL_BOOL EnableDisable)
{
  UINT8 status = MXL_TRUE;
  UINT8 control;

  status = Ctrl_ReadRegister(tuner_id, V6_DIG_XTAL_ENABLE_REG, &control);

  if (EnableDisable == MXL_DISABLE) control &= 0xDF; // Disable
  else control |= 0x20;             // Enable

  status |= Ctrl_WriteRegister(tuner_id, V6_DIG_XTAL_ENABLE_REG, control);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_XtalBiasControl
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : 0 : 200uA, 1 : 575 uA, ...
--|                 API is reorganised according to the MxL1x1SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_XtalBiasControl(UINT32 tuner_id, MXL_XTAL_BIAS_E XtalBias)
{
  MXL_STATUS status = MXL_TRUE;
  UINT8 control;

  control = 0x10;          // Xtal Enable

  if (XtalBias < XTAL_BIAS_NA)
  {
    control |= (UINT8)XtalBias;
    status = Ctrl_WriteRegister(tuner_id, V6_DIG_XTAL_BIAS_REG, control);
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_XtalCapControl
--|
--| AUTHOR        : Brenndon Lee
--|                 Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 8/4/2009
--|
--| DESCRIPTION   : XTAL Clock Cap control
--|                 0 : 10pF, 1 : 1 pF, ...
--|                 API is reorganised according to the MxL1x1SF control structure
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_XtalCapControl(UINT32 tuner_id, UINT8 XtalCapacitor)
{
  MXL_STATUS status = MXL_TRUE;

  if ((XtalCapacitor < 26) || (XtalCapacitor == 0x3F))
    status = Ctrl_WriteRegister(tuner_id, V6_XTAL_CAP_REG, XtalCapacitor);

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigXtalSettings
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/4/2009
--|
--| DESCRIPTION   : This API shall configure XTAL settings for MxL101SF
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigXtalSettings(UINT32 tuner_id,PMXL_XTAL_CFG_T XtalCfgPtr)
{
  UINT8 status;

  status = MxL101SF_API_TunerLoopThruControl(tuner_id,XtalCfgPtr->LoopThruEnable);
  status |= MxL101SF_API_XtalSelect(tuner_id,XtalCfgPtr->XtalFreq);
  status |= MxL101SF_API_XtalClkOutGain(tuner_id,XtalCfgPtr->XtalClkOutGain);
  status |= MxL101SF_API_XtalClkOutControl(tuner_id,XtalCfgPtr->XtalClkOutEnable);
  status |= MxL101SF_API_XtalBiasControl(tuner_id,XtalCfgPtr->XtalBiasCurrent);
  status |= MxL101SF_API_XtalCapControl(tuner_id,XtalCfgPtr->XtalCap);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_GetTunerSignalStrength
--|
--| AUTHOR        : Mahendra
--|
--| DATE CREATED  : 10/20/2009
--|
--| DESCRIPTION   : This API shall give MxL101SF's RF Signal Strength in dBm unit
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_GetTunerSignalStrength(UINT32 tuner_id, PMXL_SIGNAL_STATS_T SigQualityPtr)
{
  UINT8 status;
  UINT8 regData;
  UINT16 rxPwr;
  SINT16 rfPwrAdj = 0;

  status = Ctrl_WriteRegister(tuner_id,0x00, 0x02);

  status |= Ctrl_ReadRegister(tuner_id, V6_DIG_RF_PWR_LSB_REG, &regData);
  rxPwr = regData;

  status |= Ctrl_ReadRegister(tuner_id, V6_DIG_RF_PWR_MSB_REG, &regData);
  rxPwr |= (regData & 0x07) << 8;
  MxLTunedFreq = MxLTunedFreq/1000000;

  if (MxLTunedFreq <= 131) rfPwrAdj = - 4;
  else if (MxLTunedFreq <= 143) rfPwrAdj = 40;
  else if (MxLTunedFreq <= 296) rfPwrAdj = 36;
  else if (MxLTunedFreq <= 308) rfPwrAdj = 41;
  else if (MxLTunedFreq <= 320) rfPwrAdj = 44;
  else if (MxLTunedFreq <= 332) rfPwrAdj = 52;
  else if (MxLTunedFreq <= 422) rfPwrAdj = 39;
  else if (MxLTunedFreq <= 506) rfPwrAdj = 33;
  else if (MxLTunedFreq <= 566) rfPwrAdj = 25;
  else if (MxLTunedFreq <= 650) rfPwrAdj = 20;
  else if (MxLTunedFreq <= 800) rfPwrAdj = 14;
  else if (MxLTunedFreq <= 860) rfPwrAdj = 21;
  else if (MxLTunedFreq > 860) rfPwrAdj = 29;

  SigQualityPtr->SignalStrength = ((rxPwr + rfPwrAdj) / 8) - 119;

  status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x00);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigCableSettings
--|
--| AUTHOR        : Mahendra
--|
--| DATE CREATED  : 11/05/2009
--|
--| DESCRIPTION   : This API shall configure DVBC settings on MxL101SF
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigCableSettings(UINT32 tuner_id)
{
  return Ctrl_ProgramRegisters(tuner_id, MxL_CableSettings);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigGPOPins
--|
--| AUTHOR        : Mahendra
--|
--| DATE CREATED  : 10/02/2009
--|
--| DESCRIPTION   : API to confugure GPIO Pins of MxL101SF
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigGPOPins(UINT32 tuner_id, PMXL_DEV_GPO_CFG_T GpoPinCfgPtr)
{
  UINT8 status = MXL_FALSE;
  UINT8 regData = 0;
  UINT8 gpoMask = 0;

  status = Ctrl_ReadRegister(tuner_id, V6_GPO_CTRL_REG, &regData);

  switch(GpoPinCfgPtr->GpoPinId)
  {
    case MXL_GPO_0:
      gpoMask = V6_GPO_0_MASK;
      break;

    case MXL_GPO_1:
      gpoMask = V6_GPO_1_MASK;
      break;

    default:
      break;
  }

  if (GpoPinCfgPtr->GpoPinCtrl)
    regData |= gpoMask;
  else
    regData &= ~gpoMask;

  status |= Ctrl_WriteRegister(tuner_id, V6_GPO_CTRL_REG, regData);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetCellId
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 11/19/2009
--|
--| DESCRIPTION   : This API Enables TPS Cell ID Feature
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetCellId(UINT32 tuner_id, PMXL_DEMOD_CELL_ID_INFO_T tpsCellId)
{
  UINT8 status;
  UINT8 regData;
  UINT16 CellId;

  status = Ctrl_WriteRegister(tuner_id, 0x00, 0x02);

  status |= Ctrl_ReadRegister(tuner_id, 0x98, &regData);
  CellId = regData;

  status |= Ctrl_ReadRegister(tuner_id, 0x99, &regData);
  CellId |= regData << 8;

  tpsCellId->TpsCellId = CellId;

  status |= Ctrl_WriteRegister(tuner_id, 0x00, 0x00);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_GetAGCLock
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2010
--|
--| DESCRIPTION   : This function return AGC_LOCK status bit information
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_GetAGCLock(UINT32 tuner_id,PMXL_DEMOD_LOCK_STATUS_T AgcLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_AGC_LOCK_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if((rawData & V6_AGC_LOCK_MASK) >> 5)
      AgcLockPtr->Status = MXL_LOCKED;
    else
      AgcLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_GetFECLock
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2010
--|
--| DESCRIPTION   : This function return FEC_LOCK status bit information
--|                 if set to 1, FEC_LOCK
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_GetFECLock(UINT32 tuner_id, PMXL_DEMOD_LOCK_STATUS_T FecLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_IRQ_STATUS_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if((rawData & IRQ_MASK_FEC_LOCK) >> 4)
      FecLockPtr->Status = MXL_LOCKED;
    else
      FecLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigSpectrum
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 4/19/2010
--|
--| DESCRIPTION   : This function will configures MxL101SF to process inverted
--|                 signal.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigSpectrum(UINT32 tuner_id, PMXL_DEMOD_SPECTRUM_CFG_T SpectrumCfgPtr)
{
  UINT8 status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(tuner_id, V6_SPECTRUM_CTRL_REG, &rawData);

  if (status == MXL_TRUE)
  {
    if(SpectrumCfgPtr->SpectrumCfg == MXL_ENABLE)
      rawData &= ~V6_SPECTRUM_MASK;
    else
      rawData |= V6_SPECTRUM_MASK;

    status |= Ctrl_WriteRegister(tuner_id, V6_SPECTRUM_CTRL_REG, rawData);
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigRsPktThreshold
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 09/09/2010
--|
--| DESCRIPTION   : This function will configures RS packet threshold required
--|                 by the demod to lock a valid channel
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigRsPktThreshold(UINT32 tuner_id, PMXL_DEMOD_RS_PKT_THRESH_T RsPktThreshCfgPtr)
{
  return Ctrl_WriteRegister(tuner_id, RS_PKT_THRESH_REG, RsPktThreshCfgPtr->RsPktThresh);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigChannelScan
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 2/22/2010
--|
--| DESCRIPTION   : This function Enables or Disables Channel Scan functionality
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigChannelScan(UINT32 tuner_id, PMXL_TUNER_CHAN_SCAN_CFG_T ChanScanCfgPtr)
{
  UINT8 Status = MXL_FALSE;

  if (ChanScanCfgPtr->ChanScanCfg == MXL_ENABLE)
    Ctrl_ProgramRegisters(tuner_id, MxL_EnableChanScan);
  else
    Ctrl_ProgramRegisters(tuner_id, MxL_DisableChanScan);

  return (MXL_STATUS)Status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigFreqOffsetSearchRange
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/31/2010
--|
--| DESCRIPTION   : This function increases frequency offset search  range
--|                 during channel scan operation.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigFreqOffsetSearchRange(UINT32 tuner_id, PMXL_TUNER_FREQ_OFFSET_CFG_T FreqOffsetRangeCfgPtr)
{
  UINT8 Status = MXL_FALSE;
  UINT8 rawData = 0;

  Status = Ctrl_ReadRegister(tuner_id, FREQ_OFFSET_SRCH_RANGE_REG, &rawData);

  if (FreqOffsetRangeCfgPtr->MaxFreqOffsetRangeCfg == MXL_ENABLE) rawData |= 0x07;
  else rawData &= ~0x07;

  Status |= Ctrl_WriteRegister(tuner_id, FREQ_OFFSET_SRCH_RANGE_REG, rawData);

  return (MXL_STATUS)Status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_GetChannelOffset
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 2/22/2010
--|
--| DESCRIPTION   : This function returns offset for a channel
--|                 Frequency offset will be valid only if FEC lock has been achieved
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_GetChannelOffset(UINT32 tuner_id, PMXL_TUNER_CHAN_OFFSET_T ChanOffsetPtr)
{
  UINT8 status;
  UINT8 tmpData;
  UINT8 rawData0;
  UINT8 rawData1;
  SINT32 readValue;
  SINT32 freqOffset = 0;

  status = Ctrl_WriteRegister(tuner_id, 0, 2);

  // Read 0x9B registe first, otherwise V6_FREQ_OFFSET_LSB_REG/V6_FREQ_OFFSET_LSB_REG will return 0
  status |= Ctrl_ReadRegister(tuner_id, 0x9B, &tmpData);
  status |= Ctrl_ReadRegister(tuner_id, V6_FREQ_OFFSET_LSB_REG, &rawData0);

  status |= Ctrl_ReadRegister(tuner_id, V6_FREQ_OFFSET_MSB_REG, &rawData1);
  status |= Ctrl_WriteRegister(tuner_id, 0, 0);

  readValue = rawData0 + (rawData1 * 256);

  if (readValue < 512) freqOffset = readValue;
  else freqOffset = -(1024 - readValue + 1);

  ChanOffsetPtr->ChanOffset = (freqOffset * 2048) + 2048;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_CheckChannel
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 02/22/2010
--|                 05/01/2010
--|                 09/04/2010 - Updated channel scan to handle bad signal
--|                              conditions
--|
--| DESCRIPTION   : This API will check for presence of singnal at different lock
--|                 levels (i.e CP, TPS, RS, FEC) for a given frequency.
--|                 - Removed checking for FEC lock.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_CheckChannel(UINT32 tuner_id, PMXL_TUNER_CHECK_CHAN_REQ_T ChanScanCtrlPtr)
{
  PREG_CTRL_INFO_T ctrlArrayPtr;
  UINT8 status = MXL_FALSE;
  UINT8 mxl_mode;
  UINT8 regData = 0;
  UINT32 StartTime;
  UINT32 EndTime;
  UINT16 TimeOut = 768;
  MXL_DEMOD_LOCK_STATUS_T  cpLockStatus;
  MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
  MXL_DEMOD_LOCK_STATUS_T  rsLockStatus;
  UINT16 AgcGainValue = 0;
  MXL_TUNER_CHAN_CONFIG_T ChanTuneCfg;

  // Stop Tune
  status = Ctrl_WriteRegister(tuner_id, START_TUNE_REG, 0);

  // Check Device mode
  status |= Ctrl_ReadRegister(tuner_id, MXL_MODE_REG, &mxl_mode);

  if (ChanScanCtrlPtr->Bandwidth == 6)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_Config6MHzBandwidth);

  if (ChanScanCtrlPtr->Bandwidth == 7)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_Config7MHzBandwidth);

  // Fill out registers for channel tune
  ctrlArrayPtr = Ctrl_PhyTune(ChanScanCtrlPtr->Frequency, ChanScanCtrlPtr->Bandwidth);

  if (!ctrlArrayPtr) return MXL_FALSE;

  // Program registers
  status |= Ctrl_ProgramRegisters(tuner_id, ctrlArrayPtr);

  // Enable TPS Cell ID feature
  if (ChanScanCtrlPtr->TpsCellIdRbCtrl == MXL_ENABLE)
    status |= Ctrl_ProgramRegisters(tuner_id, MxL_EnableCellId);

//  if (ChanScanCtrlPtr->IncreaseMaxTempRange == MXL_ON)
//    status |= Ctrl_ProgramRegisters(MxL_EnableHighTempRange);

  // Start tune
  status |= Ctrl_ProgramRegisters(tuner_id, MxL_StartTune);

  Ctrl_GetTime(&StartTime);

  // Sleep for 32 msec's
 // if (ChanScanCtrlPtr->IncreaseMaxTempRange == MXL_ON)
 // {
 //   Ctrl_Sleep(32);
 //   status |= Ctrl_ProgramRegisters(MxL_DisableHighTempRange);
 // }

  if ((mxl_mode & MXL_DEV_MODE_MASK) == MXL_SOC_MODE)
  {
    // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
    status |= Ctrl_ReadRegister(tuner_id, 0x25, &regData);
    AgcGainValue = regData;
    regData = 0;
    status |= Ctrl_ReadRegister(tuner_id, 0x26, &regData);
    AgcGainValue |= (regData & 0x1F) << 8;

    ChanTuneCfg.TpsCellId = ChanScanCtrlPtr->TpsCellIdRbCtrl;
    ChanTuneCfg.StartTime = StartTime;
    ChanTuneCfg.AgcGain = AgcGainValue;
    ChanTuneCfg.Timeout = TimeOut;

    MxL101SF_API_ChanTuneCfg(tuner_id, &ChanTuneCfg);
  }

  // Reset IRQ status
  status = MxL101SF_API_DemodResetIrqStatus(tuner_id);
  ChanScanCtrlPtr->ChanPresent = MXL_FALSE;

  // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
  status |= Ctrl_ReadRegister(tuner_id, 0x25, &regData);
  AgcGainValue = regData;
  regData = 0;
  status |= Ctrl_ReadRegister(tuner_id, 0x26, &regData);
  AgcGainValue |= (regData & 0x1F) << 8;

  if (AgcGainValue <= 6208)
  {
    if (ChanScanCtrlPtr->TpsCellIdRbCtrl == MXL_ENABLE) TimeOut = 2560;
    else TimeOut = 2048;
  }
  else
  {
    if (ChanScanCtrlPtr->TpsCellIdRbCtrl == MXL_ENABLE) TimeOut = 1536;
    else TimeOut = 512;
  }

  Ctrl_GetTime(&EndTime);

  while ((EndTime - StartTime) < TimeOut)
  {
    // Check if channel is available. Start with CP Lock
    status |= MxL101SF_API_DemodGetCpLockStatus(tuner_id, &cpLockStatus);
    if (cpLockStatus.Status == MXL_LOCKED) break;

    Ctrl_GetTime(&EndTime);
  }

  if (ChanScanCtrlPtr->ChanScanCtrl == MXL_BREAK_AT_CP_LOCK)
  {
    // Break if interested to check channel status only till CP Lock
    if (cpLockStatus.Status == MXL_LOCKED)
      ChanScanCtrlPtr->ChanPresent = MXL_TRUE;

    goto EXIT;
  }

  if (cpLockStatus.Status == MXL_LOCKED)
  {
    // Read AGC Gain value 13-bit value spread across 0x25 & 0x26 registers
    status |= Ctrl_ReadRegister(tuner_id, 0x25, &regData);
    AgcGainValue = regData;
    regData = 0;
    status |= Ctrl_ReadRegister(tuner_id, 0x26, &regData);
    AgcGainValue |= (regData & 0x1F) << 8;

    // CP is locked, check TPS Lock
    if (AgcGainValue <= 6208)
    {
      if (ChanScanCtrlPtr->TpsCellIdRbCtrl == MXL_ENABLE) TimeOut = 3072;
      else TimeOut = 2560;
    }
    else
    {
      if (ChanScanCtrlPtr->TpsCellIdRbCtrl == MXL_ENABLE) TimeOut = 2048;
      else TimeOut = 1024;
    }

    Ctrl_GetTime(&EndTime);

    while ((EndTime - StartTime) < TimeOut)
    {
      status |= MxL101SF_API_DemodGetTpsLock(tuner_id, &tpsLockStatus);
      if (tpsLockStatus.Status == MXL_LOCKED) break;

       Ctrl_GetTime(&EndTime);
    }

    if (ChanScanCtrlPtr->ChanScanCtrl == MXL_BREAK_AT_TPS_LOCK)
    {
      // Break if interested to check channel status only till TPS Lock
      if (tpsLockStatus.Status == MXL_LOCKED)
        ChanScanCtrlPtr->ChanPresent = MXL_TRUE;

      goto EXIT;
    }

    if (tpsLockStatus.Status == MXL_LOCKED)
    {
      // TPS is locked, check RS Lock
      TimeOut = 5000;
      Ctrl_GetTime(&EndTime);

      while ((EndTime - StartTime) < TimeOut) // < 5 secs
      {
        status |= MxL101SF_API_DemodGetRsLockStatus(tuner_id, &rsLockStatus);
        if (rsLockStatus.Status == MXL_LOCKED) break;

         Ctrl_GetTime(&EndTime);
      }

      if (ChanScanCtrlPtr->ChanScanCtrl == MXL_BREAK_AT_RS_LOCK)
      {
        // Break if interested to check channel status only till RS Lock
        if (rsLockStatus.Status == MXL_LOCKED)
          ChanScanCtrlPtr->ChanPresent = MXL_TRUE;

        goto EXIT;
      }
      if (rsLockStatus.Status == MXL_LOCKED)
      {
        ChanScanCtrlPtr->ChanPresent = MXL_TRUE;
      } // RS Lock
    } // TPS Lock
  } // CP Lock

EXIT:

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDevice
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The general device configuration shall be handled
--|                 through this API
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDevice(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEV_SOFT_RESET_CFG:
      status = MxL101SF_API_SoftResetDevice(tuner_id);
      break;

    case MXL_DEV_OPERATIONAL_MODE_CFG:
      status = MxL101SF_API_SetMxLDeviceMode(tuner_id, (PMXL_DEV_MODE_CFG_T)ParamPtr);
      break;

    case MXL_DEV_XTAL_SETTINGS_CFG:
      status = MxL101SF_API_ConfigXtalSettings(tuner_id, (PMXL_XTAL_CFG_T)ParamPtr);
      break;

    case MXL_DEV_101SF_OVERWRITE_DEFAULTS_CFG:
      status = MxL101SF_API_InitTunerDemod(tuner_id);
      break;

    case MXL_DEV_101SF_POWER_MODE_CFG:
      status = MxL101SF_API_ConfigDevPowerSavingMode(tuner_id, (PMXL_PWR_MODE_CFG_T)ParamPtr);
      break;

    case MXL_DEV_MPEG_OUT_CFG:
      status = MxL101SF_API_ConfigMpegOut(tuner_id, (PMXL_MPEG_CFG_T)ParamPtr);
      break;

    case MXL_DEV_TS_CTRL_CFG:
      status = MxL101SF_API_ControlMpegOutput(tuner_id, (PMXL_TS_CTRL_T)ParamPtr);
      break;

    case MXL_DEV_GPO_PINS_CFG:
      status = MxL101SF_API_ConfigGPOPins(tuner_id, (PMXL_DEV_GPO_CFG_T)ParamPtr);
      break;

    case MXL_DEV_CABLE_CFG:
      status = MxL101SF_API_ConfigCableSettings(tuner_id);
      break;

    default:
      break;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDeviceStatus
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The general device inquiries shall be handled
--|                 through this API
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDeviceStatus(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEV_ID_VERSION_REQ:
      status = MxL101SF_API_GetChipInfo(tuner_id, (PMXL_DEV_INFO_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDemodStatus
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The demod specific inquiries shall be handled
--|                 through this API
--|                 - Support for MXL_DEMOD_TPS_HIERARCHICAL_ALPHA_REQ
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDemodStatus(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEMOD_SNR_REQ:
      status = MxL101SF_API_DemodGetSNR(tuner_id, (PMXL_DEMOD_SNR_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_BER_REQ:
      status = MxL101SF_API_DemodGetBER(tuner_id, (PMXL_DEMOD_BER_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_CODE_RATE_REQ:
      status = MxL101SF_API_DemodGetTpsCodeRate(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_HIERARCHY_REQ:
      status = MxL101SF_API_DemodGetTpsHierarchy(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_CONSTELLATION_REQ:
      status = MxL101SF_API_DemodGetTpsConstellation(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_FFT_MODE_REQ:
      status = MxL101SF_API_DemodGetTpsFftMode(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_HIERARCHICAL_ALPHA_REQ:
      status = MxL101SF_API_DemodGetHierarchicalAlphaValue(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_GUARD_INTERVAL_REQ:
      status = MxL101SF_API_DemodGetTpsGuardInterval(tuner_id, (PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_LOCK_REQ:
      status = MxL101SF_API_DemodGetTpsLock(tuner_id, (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_CELL_ID_REQ:
      status = MxL101SF_API_DemodGetCellId(tuner_id, (PMXL_DEMOD_CELL_ID_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_PACKET_ERROR_COUNT_REQ:
      status = MxL101SF_API_DemodGetPacketErrorCount(tuner_id, (PMXL_DEMOD_PEC_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_SYNC_LOCK_REQ:
      status = MxL101SF_API_DemodGetSyncLockStatus(tuner_id, (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_RS_LOCK_REQ:
      status = MxL101SF_API_DemodGetRsLockStatus(tuner_id, (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_CP_LOCK_REQ:
      status = MxL101SF_API_DemodGetCpLockStatus(tuner_id, (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_FEC_LOCK_REQ:
      status = MxL101SF_API_GetFECLock(tuner_id, (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDemod
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 03/12/2010
--|                 04/20/2010
--|                 09/09/2010
--|
--| DESCRIPTION   : The demod block specific configuration shall be handled
--|                 through this API
--|                 - Support for MXL_DEMOD_STREAM_PRIORITY_CFG
--|                 - Support for MXL_DEMOD_SPECTRUM_CFG
--|                 - Support for MXL_DEMOD_RS_PKT_THRESH_CFG
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDemod(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEMOD_RESET_IRQ_CFG:
      status = MxL101SF_API_DemodResetIrqStatus(tuner_id);
      break;

    case MXL_DEMOD_RESET_PEC_CFG:
      status = MxL101SF_API_DemodResetPacketErrorCount(tuner_id);
      break;

    case MXL_DEMOD_TS_PRIORITY_CFG:
      status = MxL101SF_API_ConfigStreamPriority(tuner_id, (PMXL_DEMOD_TS_PRIORITY_CFG_T)ParamPtr);
      break;

    case MXL_DEMOD_SPECTRUM_CFG:
      status = MxL101SF_API_ConfigSpectrum(tuner_id, (PMXL_DEMOD_SPECTRUM_CFG_T)ParamPtr);
      break;

    case MXL_DEMOD_RS_PKT_THRESH_CFG:
      status = MxL101SF_API_ConfigRsPktThreshold(tuner_id, (PMXL_DEMOD_RS_PKT_THRESH_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigTuner
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 02/22/2010
--|
--| DESCRIPTION   : The tuner block specific configuration shall be handled
--|                 through this API
--|                 Support for MXL_TUNER_CHAN_SCAN_CFG
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigTuner(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_TUNER_TOP_MASTER_CFG:
      status = MxL101SF_API_TopMasterControl(tuner_id, (PMXL_TOP_MASTER_CFG_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_TUNE_CFG:
      status = MxL101SF_API_TuneRF(tuner_id, (PMXL_RF_TUNE_CFG_T)ParamPtr);
      break;

/*
    case MXL_TUNER_CHAN_TUNE_EXT_CFG:
      status = MxL101SF_API_TuneRFExt((PMXL_RF_TUNE_CFG_T)ParamPtr);
      break;
*/

    case MXL_TUNER_IF_OUTPUT_FREQ_CFG:
      status = MxL101SF_API_TunerSetIFOutputFreq(tuner_id, (PMXL_TUNER_IF_FREQ_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_SCAN_CFG:
      status = MxL101SF_API_ConfigChannelScan(tuner_id, (PMXL_TUNER_CHAN_SCAN_CFG_T)ParamPtr);
      break;

    case MXL_TUNER_FREQ_OFFSET_SRCH_RANGE_CFG:
      status = MxL101SF_API_ConfigFreqOffsetSearchRange(tuner_id, (PMXL_TUNER_FREQ_OFFSET_CFG_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetTunerStatus
--|
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 02/22/2010
--|
--| DESCRIPTION   : The tuner specific inquiries shall be handled
--|                 through this API
--|                 Support for MXL_TUNER_CHAN_OFFSET_REQ &
--|                 MXL_TUNER_CHECK_CHAN_REQ
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetTunerStatus(UINT32 tuner_id, MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_TUNER_LOCK_STATUS_REQ:
      status = MxL101SF_API_TunerGetLockStatus(tuner_id, (PMXL_TUNER_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_TUNER_SIGNAL_STRENGTH_REQ:
      status = MxL101SF_API_GetTunerSignalStrength(tuner_id, (PMXL_SIGNAL_STATS_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_OFFSET_REQ:
      status = MxL101SF_API_GetChannelOffset(tuner_id, (PMXL_TUNER_CHAN_OFFSET_T)ParamPtr);
      break;

   case MXL_TUNER_CHECK_CHAN_STATUS_REQ:
      status = MxL101SF_API_CheckChannel(tuner_id, (PMXL_TUNER_CHECK_CHAN_REQ_T)ParamPtr);
      break;

   default:
      break;
  }

  return status;
}
//MxL101SF_PhyCtrlApi.c END

static INT32 f_MXL101_read(UINT tuner_id,UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    struct nim_MXL101_private *priv = (struct nim_MXL101_private *)nim_mxl101[tuner_id]->priv;
    struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    INT32 err;

    osal_semaphore_capture(f_IIC_Sema_ID[tuner_id],TMO_FEVR);

    data[0] = reg_add;
    //err = i2c_write_read(0, dev_add, data, 1, len);
    err = i2c_write_read(cfg->i2c_type_id, cfg->i2c_base_addr, data, 1, len);
    if(err!=0)
    {
        NIM_MXL101_PRINTF("nim_MXL101_read: err = %d ,reg_add=%x!\n", err,reg_add);
    }
    //        osal_delay(1000);

    osal_semaphore_release(f_IIC_Sema_ID[tuner_id]);
    return err;
}

static INT32 f_MXL101_write(UINT tuner_id,UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    struct nim_MXL101_private *priv = (struct nim_MXL101_private *)nim_mxl101[tuner_id]->priv;
    struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    UINT8  i, buffer[8];
    INT32 err;

    osal_semaphore_capture(f_IIC_Sema_ID[tuner_id],TMO_FEVR);
    if (len > 7)
    {
            osal_semaphore_release(f_IIC_Sema_ID[tuner_id]);
            return ERR_FAILUE;
    }
    buffer[0] = reg_add;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = data[i];
    }

    //err = i2c_write(0, dev_add, buffer, len + 1);
    err = i2c_write(cfg->i2c_type_id, cfg->i2c_base_addr, buffer, len + 1);
    if(err!=0)
    {
        NIM_MXL101_PRINTF("nim_MXL101_write: err = %d !\n", err);
    }
    osal_semaphore_release(f_IIC_Sema_ID[tuner_id]);
    return  err;
}

void f_MXL101_dump_register(struct nim_device *dev)
{
        UINT16  i=0;
        UINT8 data[5];
        struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
        for(i=0;i<=0xff;i++)
        {
            f_MXL101_read(priv_mem->tuner_id,dev->base_addr,(UINT8)i,data,1);
             NIM_MXL101_PRINTF("reg=0x%x, data=0x%x\n",i,data[0]);
             osal_delay(100);
        }

}
/*****************************************************************************
* INT32  f_MXL101_attach()
* Description: MXL101 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
INT32 f_MXL101_attach(char *name,struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner)    //51117-01Angus
{

    struct nim_device *dev;
    struct nim_MXL101_private * priv_mem;

    if ((ptrCOFDM_Tuner == NULL))
    {
        NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (struct nim_MXL101_private *)MALLOC(sizeof(struct nim_MXL101_private));
    if ((void*)priv_mem == NULL)
    {
        dev_free(dev);

        NIM_MXL101_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    MEMCPY((void*)&(priv_mem->Tuner_Control), (void*)ptrCOFDM_Tuner, sizeof(struct COFDM_TUNER_CONFIG_API));
    priv_mem->tuner_id = tuner_cnt;
    dev->priv = (void*)priv_mem;

    tuner_cnt++;

    /* Function point init */

    dev->base_addr = priv_mem->Tuner_Control.ext_dm_config.i2c_base_addr;
    dev->init = f_MXL101_attach;
    dev->open = f_MXL101_open;
    dev->stop = f_MXL101_close;//Sam
    dev->do_ioctl = f_MXL101_ioctl;//nim_m3327_ioctl;//050810 yuchun for stop channel search
    dev->do_ioctl_ext = f_MXL101_ioctl_ext;
    dev->get_lock = f_MXL101_get_lock;
    dev->get_freq = f_MXL101_get_freq;
    dev->get_fec = f_MXL101_get_code_rate;
    dev->get_agc = f_MXL101_get_AGC;
    dev->get_snr = f_MXL101_get_SNR;
    dev->get_ber = f_MXL101_get_BER;

    dev->disable=f_MXL101_disable;//Sam
    dev->channel_change = f_MXL101_channel_change;
    dev->channel_search = (void *)f_MXL101_channel_search;//NULL;//f_MXL101_channel_search;//nim_m3327_channel_search;
    dev->get_guard_interval = f_MXL101_get_GI;
    dev->get_fftmode = f_MXL101_get_fftmode;
    dev->get_modulation = f_MXL101_get_modulation;
    dev->get_spectrum_inv = f_MXL101_get_specinv;
//    dev->get_HIER= f_MXL101_get_hier_mode;
//    dev->get_priority=f_MXL101_priority;
    dev->get_freq_offset =    f_MXL101_get_freq_offset;

    f_IIC_Sema_ID[priv_mem->tuner_id]=osal_semaphore_create(1);

    /*if((((struct nim_MXL101_private*)dev->priv)->Tuner_Control.tuner_config.cChip)!=Tuner_Chip_QUANTEK )
    {
        ptrCOFDM_Tuner->tuner_config.Tuner_Write=NULL;
        ptrCOFDM_Tuner->tuner_config.Tuner_Read=NULL;
    }*/

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    nim_mxl101[priv_mem->tuner_id] = dev;

    if (((struct nim_MXL101_private*)dev->priv)->Tuner_Control.nim_tuner_init != NULL)
    {
        if((((struct nim_MXL101_private*)dev->priv)->Tuner_Control.tuner_config.c_chip)==TUNER_CHIP_QUANTEK || (((struct nim_MXL101_private*)dev->priv)->Tuner_Control.tuner_config.c_chip)==TUNER_CHIP_MAXLINEAR)
        {
        //_MXL101_PassThrough(dev,1);


            if (((struct nim_MXL101_private*)dev->priv)->Tuner_Control.nim_tuner_init(&((struct nim_MXL101_private*)dev->priv)->tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            //if (((struct nim_MXL101_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(&tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");

                //MXL101_PassThrough(dev,0);

                return ERR_NO_DEV;
            }

        //_MXL101_PassThrough(dev,0);
        }
        else
        {
            if (((struct nim_MXL101_private*)dev->priv)->Tuner_Control.nim_tuner_init(&(priv_mem->tuner_id), &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");

                return ERR_NO_DEV;
            }

        }

    }


    return SUCCESS;
}


/*****************************************************************************
* INT32 f_MXL101_open(struct nim_device *dev)
* Description: MXL101 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_open(struct nim_device *dev)
{

    //UINT8    data = 0;
    //UINT8    tdata[5] ;
    //UINT8    lock = 0;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    struct COFDM_TUNER_CONFIG_API * config_info = &(priv_mem->Tuner_Control);


    NIM_PRINTF("f_MXL101_open\n");

    nim_MXL101_flag[priv_mem->tuner_id] = osal_flag_create(NIM_MXL101_FLAG_ENABLE);//050513 Tom
#if 1

    if (nim_MXL101_flag[priv_mem->tuner_id]==OSAL_INVALID_ID)
        return ERR_FAILUE;
    MXL101_CurChannelInfo[priv_mem->tuner_id] = (struct MXL101_Lock_Info *)MALLOC(sizeof(struct MXL101_Lock_Info));
    if (MXL101_CurChannelInfo[priv_mem->tuner_id] == NULL)
    {
        NIM_PRINTF("f_MXL101_open MALLOC fail!\n");
        return ERR_FAILUE;
    }
    MEMSET(MXL101_CurChannelInfo[priv_mem->tuner_id], 0, sizeof(struct MXL101_Lock_Info));
#endif

  //UINT32 loop;
  MXL_DEV_INFO_T mxlDevInfo;
  MXL_XTAL_CFG_T mxlXtalCfg;
  MXL_DEV_MODE_CFG_T mxlDevMode;
  MXL_MPEG_CFG_T mxlMpegOutCfg;
  MXL_TOP_MASTER_CFG_T mxlTopMasterCfg;
  //MXL_RF_TUNE_CFG_T mxlChanCfg;
  //MXL_DEMOD_LOCK_STATUS_T rsLockStatus;
  //MXL_DEMOD_SNR_INFO_T Snr;
  //MXL_DEMOD_BER_INFO_T Ber;
  //MXL_SIGNAL_STATS_T SigStrength;
  //MXL_DEMOD_CELL_ID_INFO_T TpsCellIdInfo;
  MXL_TUNER_FREQ_OFFSET_CFG_T FreqOffsetRangeCfg;
  //MXL_TS_CTRL_T mxlTscfg;


  // Open LPT driver for I2C communcation
  //Ctrl_I2cConnect(99);

  // 1. Do SW Reset
  MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_SOFT_RESET_CFG, NULL);

  // 2. Read Back chip id and version
  //    Expecting CHIP ID = 0x61, Version = 0x6
  MxLWare_API_GetDeviceStatus(priv_mem->tuner_id, MXL_DEV_ID_VERSION_REQ, &mxlDevInfo);
  NIM_MXL101_PRINTF("MxL101SF : ChipId = 0x%x, Version = 0x%x\n", mxlDevInfo.DevId, mxlDevInfo.DevVer);
//Device Identification shall be 0x61, and Version shall be 0x6
  // 3. Init Tuner and Demod
  MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_101SF_OVERWRITE_DEFAULTS_CFG, NULL);

  // Step 4
  // Enable Loop Through if needed
  // Enable Clock Out and configure Clock Out gain if needed
  // Configure MxL101SF XTAL frequency
  // Configure XTAL Bias value if needed

  // Xtal Capacitance value must be configured in accordance
  // with XTAL datasheet’s requirement.
  mxlXtalCfg.XtalFreq = XTAL_24MHz;
  mxlXtalCfg.LoopThruEnable = MXL_ENABLE;
  mxlXtalCfg.XtalBiasCurrent = XTAL_BIAS_NA;
  mxlXtalCfg.XtalCap = 0x12;
  mxlXtalCfg.XtalClkOutEnable = MXL_ENABLE;
  mxlXtalCfg.XtalClkOutGain = CLK_OUT_NA;
  MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_XTAL_SETTINGS_CFG, &mxlXtalCfg);

  // 5. Set Baseband mode, SOC or Tuner only mode
  mxlDevMode.DeviceMode = MXL_SOC_MODE;
  MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_OPERATIONAL_MODE_CFG, &mxlDevMode);

  // 6. Configure MPEG Out
  // CLK, MPEG_CLK_INV, Polarity of MPEG Valid/MPEG Sync

  if (NIM_COFDM_TS_SSI == config_info->ts_mode)
  {
    //For output SSI.
    mxlMpegOutCfg.SerialOrPar = MPEG_DATA_SERIAL;
    mxlMpegOutCfg.MpegClkFreq = MPEG_CLOCK_36_571429MHz;
    mxlMpegOutCfg.LsbOrMsbFirst = MPEG_SERIAL_MSB_1ST;
  }
  else
  {
    //For output SPI.
    mxlMpegOutCfg.SerialOrPar = MPEG_DATA_PARALLEL;
    mxlMpegOutCfg.MpegClkFreq = MPEG_CLOCK_9_142857MHz;
//    mxlMpegOutCfg.MpegClkFreq = MPEG_CLOCK_36_571429MHz;
  }

  if (config_info->ext_dm_config.lock_polar_reverse)
      mxlMpegOutCfg.MpegClkPhase =MPEG_CLK_INVERTED;
  else
      mxlMpegOutCfg.MpegClkPhase =MPEG_CLK_IN_PHASE;

  mxlMpegOutCfg.MpegSyncPol = MPEG_CLK_IN_PHASE;
  mxlMpegOutCfg.MpegValidPol = MPEG_CLK_IN_PHASE;
  MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_MPEG_OUT_CFG, &mxlMpegOutCfg);

//  if (NIM_COFDM_TS_SSI != config_info->ts_mode)
  {
    // Turn on TS
//    mxlTscfg.TsCtrl = MXL_ON;
//    MxLWare_API_ConfigDevice(MXL_DEV_TS_CTRL_CFG,&mxlTscfg );
//    NIM_MXL101_PRINTF("init 7 step: ret_status = 0x%x\n", ret_status);
  }

  // Enable Top Master Control
  mxlTopMasterCfg.TopMasterEnable = MXL_ENABLE;
  MxLWare_API_ConfigTuner(priv_mem->tuner_id, MXL_TUNER_TOP_MASTER_CFG, &mxlTopMasterCfg);

  FreqOffsetRangeCfg.MaxFreqOffsetRangeCfg = MXL_ENABLE;
  MxL101SF_API_ConfigFreqOffsetSearchRange(priv_mem->tuner_id, &FreqOffsetRangeCfg);

  f_MXL101_power_control(dev, FALSE);

  osal_flag_set(nim_MXL101_flag[priv_mem->tuner_id],NIM_MXL101_SCAN_END);


        return SUCCESS;
}
/*****************************************************************************
* void  f_MXL101_get_chip_id(struct nim_device *dev)
* Description:  get  MXL101chip_id
*
* Arguments: struct nim_device *dev*
*
* Return Value:
*****************************************************************************/

void f_MXL101_get_chip_id(struct nim_device *dev)
{
    //UINT8 data[1];

}


/*****************************************************************************
* INT32  f_MXL101_hw_init()
* Description: MXL101 initialization (set initial register)
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/

void f_MXL101_hw_init(struct nim_device *dev)
{
    //UINT8 data[5];
    //int ret;
    //UINT16 i;
    //struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    //struct COFDM_TUNER_CONFIG_API * config_info = &(priv_mem->Tuner_Control);


    return;

}

void f_MXL101_PassThrough(struct nim_device *dev,BOOL Open)
{
    //UINT8 data;
}

/*****************************************************************************
* INT32 f_MXL101_close(struct nim_device *dev)
* Description: MXL101 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_close(struct nim_device *dev)
{
    //UINT8 data = 0;

    f_MXL101_power_control(dev, TRUE);

    return SUCCESS;
}



/*****************************************************************************
* INT32 f_MXL101_disable(struct nim_device *dev)
* Description: MXL101 disable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_MXL101_disable(struct nim_device *dev)
{
    //UINT8 data = 0;

    return SUCCESS;
}
/*****************************************************************************
* INT32 f_MXL101_channel_search(struct nim_device *dev, UINT32 freq);

* Description: MXL101 channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency*
* Return Value: INT32
*****************************************************************************/
//050803 yuchun  add
static INT32 f_MXL101_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority)
{
    //UINT8     i,data[1];
    INT32    chsearch=ERR_FAILED; //,center_freq;
    //INT32 freq_offset_reg;

    // UINT32    Center_NPRO,Search_NPRO,tmp_freq;
    //UINT32 First_Frequency;
    //UINT8 num,freq_step;
    //UINT8 j=0,k = 0;
    //INT32 get_freq_offset;
    //INT32 remainder;
    //UINT16 temp_Step_Freq;

    chsearch=f_MXL101_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);
    return chsearch;
}


static INT32 f_MXL101_freq_offset(struct nim_device *dev, UINT32 bandwidth, UINT8 fft_mode, INT32 *freq_offset)
{
    //UINT8 data[4];
    //UINT8 mode;
    //UINT32 freq_offset_tmp = 0;

     *freq_offset=0;
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_MXL101_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type,UINT8 inverse);

* Description: stv0360 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency
*  Parameter3: UINT32 bandwidth
*  Parameter4: UINT8  guard_interval
*  Parameter5: UINT8  fft_mode
*  Parameter6: UINT8  modulation
*  Parameter7: UINT8  fec
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority)
{

    __MAYBE_UNUSED__ UINT8    data[6];    //51117-01Angus
    //UINT32    temp;
    UINT8    ret_flag = SCAN_TIMEOUT;// LOCK_OK;
    TMO     start_time, temp_time;
    struct nim_MXL101_private *dev_priv;    //51117-01Angus
    UINT8 code_rate;//Tuner_Retry=0,Can_Tuner_Retry=0;
    //UINT8 lock=0;
    MXL_RF_TUNE_CFG_T mxlChanCfg;
    UINT8 rsLockStatus;
    //MXL_DEMOD_CELL_ID_INFO_T TpsCellIdInfo;


    dev_priv = (struct nim_MXL101_private *)dev->priv;

    UINT32 tuner_id = dev_priv->tuner_id;

    osal_flag_clear(nim_MXL101_flag[tuner_id], NIM_MXL101_CHECKING);

    NIM_MXL101_PRINTF("freq %d, bandwidth %d\n",freq,bandwidth);
    /* NIM_PRINTF("freq %d, bandwidth %d,guard_interval %d, fft_mode %d,modulation %d,fec %d,usage_type %d,\
        inverse %d\n ",freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse);*/

    osal_flag_clear(nim_MXL101_flag[tuner_id], NIM_MXL101_SCAN_END);

    MXL101_CurChannelInfo[tuner_id]->Frequency = freq;
    MXL101_CurChannelInfo[tuner_id]->ChannelBW = bandwidth;

    dev_priv = dev->priv;

    // 8. Tune RF with channel frequency and bandwidth
      mxlChanCfg.Bandwidth = bandwidth;
      mxlChanCfg.Frequency =freq*1000;
      mxlChanCfg.TpsCellIdRbCtrl = MXL_DISABLE;  // Enable TPS Cell ID feature
      MxLWare_API_ConfigTuner(tuner_id, MXL_TUNER_CHAN_TUNE_CFG, &mxlChanCfg);


    /*-------------------------------------------------------------------------*/
    /* (4)WAIT SYMBOL_LOCK                         */
    /*-------------------------------------------------------------------------*/

    start_time = OS_GetTime();
    while(1)
    {
        //MxLWare_API_GetDemodStatus(MXL_DEMOD_TPS_CELL_ID_REQ, &TpsCellIdInfo);
        //osal_task_sleep(1000);
        Ctrl_ReadRegister(tuner_id,V6_RS_LOCK_DET_REG, &rsLockStatus);
         NIM_MXL101_PRINTF(" LK data=%x\n",rsLockStatus);

        if ((rsLockStatus & RS_LOCK_DET_MASK) >> 3)
        {
            ret_flag = LOCK_OK;
            break;
        }
        osal_task_sleep(50);

        //For fixed the bug: Fail to search program if frequency offset exceed 100KHz, but the signal quality bar is shown good.
        temp_time = OS_GetTime();
        if (temp_time < start_time) //for avoid overflow.
            start_time = temp_time;
        temp_time -= start_time;
        if( temp_time > 1)  //Time out after 1 second.
        {
            ret_flag = SCAN_TIMEOUT;
            NIM_MXL101_PRINTF("MXL101_SYMBOL_NO_LOCK data[0] =0x%x ,ret=%d \n", data[0],ret_flag);
            osal_flag_set(nim_MXL101_flag[tuner_id],NIM_MXL101_SCAN_END);
            return ERR_FAILED;
        }
    }


    /*-------------------------------------------------------------------------*/
    /* (7)Status update:                            */
    /*-------------------------------------------------------------------------*/

    if (ret_flag == LOCK_OK  || ret_flag == TPS_UNLOCK || ret_flag == FEC_UNLOCK)
    {

        MXL101_CurChannelInfo[tuner_id]->Frequency = freq;
        MXL101_CurChannelInfo[tuner_id]->ChannelBW = (UINT8)bandwidth;

        f_MXL101_getinfo(dev, &code_rate, &guard_interval, &fft_mode, &modulation);

        //MT352_CurChannelInfo->HPRates = code_rate;
        MXL101_CurChannelInfo[tuner_id]->Guard= guard_interval;
        MXL101_CurChannelInfo[tuner_id]->Mode= fft_mode;
        MXL101_CurChannelInfo[tuner_id]->Modulation= modulation;
        MXL101_CurChannelInfo[tuner_id]->FECRates = code_rate;

        MXL101_CurChannelInfo[tuner_id]->Hierarchy = f_MXL101_hier_mode(dev);
        MXL101_CurChannelInfo[tuner_id]->Priority=priority;
        MXL101_CurChannelInfo[tuner_id]->lock_status=1;

        NIM_MXL101_PRINTF("SUCCESS end_time = %d\n",osal_get_tick()-start_time);

        osal_flag_set(nim_MXL101_flag[tuner_id],NIM_MXL101_SCAN_END);
        return SUCCESS;
    }
    else
    {
          MXL101_CurChannelInfo[tuner_id]->lock_status=0;
        NIM_MXL101_PRINTF("ERR_FAILED end_time = %d ret=%d\n",osal_get_tick()-start_time,ret_flag);

        osal_flag_set(nim_MXL101_flag[tuner_id],NIM_MXL101_SCAN_END);
        return ERR_FAILED;
    }
}



static INT32 f_MXL101_getinfo(struct nim_device *dev, UINT8 *code_rate, UINT8 *guard_interval, UINT8 *fft_mode, UINT8 *modulation)
{
//    UINT8 data[2];
//    UINT16 tdata;
//    UINT16 i;
struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
  //MXL_STATUS status = MXL_TRUE;

  //MXL_TUNER_CHAN_SCAN_CFG_T mxlChanScanCfg;
  MXL_TUNER_CHECK_CHAN_REQ_T mxl101sfChanScan;

  MXL_DEMOD_SNR_INFO_T Snr;
  MXL_DEMOD_BER_INFO_T Ber;
  MXL_DEMOD_TPS_INFO_T TpsData;

  MXL_TUNER_CHAN_OFFSET_T mxlChanOffset;
  MXL_DEMOD_CELL_ID_INFO_T TpsCellIdInfo;
  MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;

     // Check untill RS lock is achived for each channel frequency
    mxl101sfChanScan.ChanScanCtrl = MXL_BREAK_AT_CP_LOCK; // MXL_FULL_MODE

    // Enable TPS Cell Id feature if required, else Disable it
    mxl101sfChanScan.TpsCellIdRbCtrl = MXL_DISABLE;

    // Frequency of the channel
    mxl101sfChanScan.Frequency = MXL101_CurChannelInfo[priv_mem->tuner_id]->Frequency;

    // Bandwidth for the channel
    mxl101sfChanScan.Bandwidth = MXL101_CurChannelInfo[priv_mem->tuner_id]->ChannelBW;

    NIM_MXL101_PRINTF("Scanning for %d Hz, bandwidth= %d .....\n", mxl101sfChanScan.Frequency, mxl101sfChanScan.Bandwidth);

   // API to check for the presence of the channel
    // If the channel is detected then "mxl101sfChanScan.ChanPresent"
    // will contain value "MXL_TRUE"
//    status = MxLWare_API_GetTunerStatus(MXL_TUNER_CHECK_CHAN_STATUS_REQ, &mxl101sfChanScan);

    // Check if Channel is detected or not
//    if (mxl101sfChanScan.ChanPresent == MXL_TRUE)
    {
      // If Channel is detected record all the necessary
      // staus of the channel or MxL101SF

      // Before recording channel status make sure TPS Lock is present
      MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
      if (tpsLockStatus.Status == MXL_LOCKED)
      {
        NIM_MXL101_PRINTF ("\n Frequency Detected : %d Hz\n", mxl101sfChanScan.Frequency);

        // Record Frequency offset
        MxLWare_API_GetTunerStatus(priv_mem->tuner_id, MXL_TUNER_CHAN_OFFSET_REQ, &mxlChanOffset);
        NIM_MXL101_PRINTF ("Frequency Offset : %d Hz\n", mxlChanOffset.ChanOffset); 

        // SNR
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_SNR_REQ, &Snr);
        NIM_MXL101_PRINTF ("SNR : %f dB\n", (REAL32)(Snr.SNR/10000));

        // BER
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_BER_REQ, &Ber);
        NIM_MXL101_PRINTF ("BER : %f\n", (REAL32)(Ber.BER/100));

        // TPS Cell Id
        if (mxl101sfChanScan.TpsCellIdRbCtrl == MXL_ENABLE)
        {
          MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CELL_ID_REQ, &TpsCellIdInfo);
          NIM_MXL101_PRINTF ("TPS Cell ID : %d\n", TpsCellIdInfo.TpsCellId);
        }

        // TPS Code Rate
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CODE_RATE_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_MXL101_PRINTF ("TPS Code Rate : 1/2\n");
            *code_rate =FEC_1_2;
            break;

          case 1:
            NIM_MXL101_PRINTF ("TPS Code Rate : 2/3\n");
            *code_rate =FEC_2_3;
            break;

          case 2:
            NIM_MXL101_PRINTF ("TPS Code Rate : 3/4\n");
            *code_rate =FEC_3_4;
            break;

          case 3:
            NIM_MXL101_PRINTF ("TPS Code Rate : 5/6\n");
            *code_rate =FEC_5_6;
            break;

          case 4:
            NIM_MXL101_PRINTF ("TPS Code Rate : 7/8\n");
            *code_rate =FEC_7_8;
            break;

          default:
            *code_rate = 0; /* error */
            NIM_MXL101_PRINTF ("TPS Code Rate : INVALID CR!\n");
        }

        // TPS Constellation
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CONSTELLATION_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_MXL101_PRINTF ("TPS Constellation : QPSK\n");
            * modulation = TPS_CONST_QPSK;
            break;

          case 1:
            NIM_MXL101_PRINTF ("TPS Constellation : 16QAM\n");
            * modulation = TPS_CONST_16QAM;
            break;

          case 2:
            NIM_MXL101_PRINTF ("TPS Constellation : 64QAM\n");
            * modulation = TPS_CONST_64QAM;
            break;

          default:
            NIM_MXL101_PRINTF ("TPS Constellation : INVALID TPS CONSTELLATION!\n");
            * modulation = 0xff; // error
        }

        // TPS GI
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_GUARD_INTERVAL_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/32\n");
            *guard_interval = guard_1_32;
            break;

          case 1:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/16\n");
            *guard_interval = guard_1_16;
            break;

          case 2:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/8\n");
            *guard_interval = guard_1_8;
            break;

          case 3:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/4\n");
            *guard_interval = guard_1_4;
            break;

          default:
            *guard_interval = 0xff; /* error */
            NIM_MXL101_PRINTF ("TPS Code Rate : INVALID GUARD INTERVAL!\n");
        }

        // FFT
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_FFT_MODE_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/32\n");
            *fft_mode = MODE_2K;
            break;

          case 1:
            NIM_MXL101_PRINTF ("TPS Guard Interval : 1/16\n");
            *fft_mode = MODE_8K;
            break;

          default:
            NIM_MXL101_PRINTF ("FFT : INVALID !\n");
            *fft_mode = 0xff; // error
        }

      } // End of if (tpsLockStatus.Status == MXL_LOCKED)
    } // End of if (mxl101sfChanScan.ChanPresent == MXL_TRUE)

    return SUCCESS;
}



/************************Zarlink Semiconductor*********************************
*   Name:Tnim_PostViterbiBER
*   Purpose:calculates Post-viterbi BER
*   Remarks:formula is RS_BERCNT/(RS_ERRPER*1024*204*8)
*           the result is multiplied by 100E6 to get an integer value, the
*            formula becomes
*            BER = 100e6/(1024*204*8) *RS_BERCNT/RS_ERRPER = 60*RS_BERCNT/RS_ERRPER
*                    RS_ERRPER=77
*   Inputs:
*   Outputs:  BER * 100E6 , or -1 if an error occurs
*******************************************************************************/


INT32 MXL101_PostViterbi_BER_Read(struct nim_device *dev)
{
     // UINT8  Interrupt_2;

         return 0;
}



/*****************************************************************************
* INT32 f_MXL101_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/

static INT32 f_MXL101_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 rawData=0;
    OSAL_ER    result;
       UINT32    flgptn;
    //static UINT32 dd_time=0;
    //UINT8 ber_vld;
    //UINT32 m_vbber, m_per;
    struct nim_MXL101_private *dev_priv;

    dev_priv = (struct nim_MXL101_private *)dev->priv;
    UINT32 tuner_id = dev_priv->tuner_id;

    result = osal_flag_wait(&flgptn,nim_MXL101_flag[tuner_id], NIM_MXL101_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("lock result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            //libc_printf("Get LOCK Fail, Wait ChannelChg Complete!\n");
        *lock=0xff;
    }
    else
    {
        Ctrl_ReadRegister(tuner_id,V6_RS_LOCK_DET_REG, &rawData);
         //soc_printf(" LK data=%x\n",rawData);

        if ((rawData & RS_LOCK_DET_MASK) >> 3)
        {
                    *lock = 1;
                  MXL101_CurChannelInfo[tuner_id]->lock_status=1;

             }
            else
          {
                      *lock = 0;
                 MXL101_CurChannelInfo[tuner_id]->lock_status=0;
        }
      }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_get_freq(struct nim_device *dev, UINT32 *freq)
* Read M3327 frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *freq            :
*
* Return Value: void
*****************************************************************************/
static INT32 f_MXL101_get_freq(struct nim_device *dev, UINT32 *freq)
{
    //INT32 freq_off=0;
    //UINT8  data[3];
    //INT32  tdata=0;
    //    OSAL_ER    result;
    //    UINT32    flgptn;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
        //*freq=MXL101_CurChannelInfo->Frequency+MXL101_CurChannelInfo->FreqOffset;
        *freq=MXL101_CurChannelInfo[priv_mem->tuner_id]->Frequency;
        NIM_MXL101_PRINTF("freq=%d KHz \n", *freq);

    return SUCCESS;

}

/*****************************************************************************
* INT32 f_MXL101_get_code_rate(struct nim_device *dev, UINT8* code_rate)
* Description: Read MXL101 code rate
*   FEC status (b6-b4)                      code rate                 return value
*    0                    1/2            1
*    1                    2/3            4
*    2                    3/4            8
*    3                    5/6            16
*    5                    7/8            32
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
    MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    MXL_DEMOD_TPS_INFO_T TpsData;
//    *code_rate = MXL101_CurChannelInfo->FECRates;
   struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
    if (tpsLockStatus.Status != MXL_LOCKED)
        return ERR_FAILURE;

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CODE_RATE_REQ, &TpsData);
    switch(TpsData.TpsInfo)
    {
      case 0:
        NIM_MXL101_PRINTF ("TPS Code Rate : 1/2\n");
        *code_rate =FEC_1_2;
        break;

      case 1:
        NIM_MXL101_PRINTF ("TPS Code Rate : 2/3\n");
        *code_rate =FEC_2_3;
        break;

      case 2:
        NIM_MXL101_PRINTF ("TPS Code Rate : 3/4\n");
        *code_rate =FEC_3_4;
        break;

      case 3:
        NIM_MXL101_PRINTF ("TPS Code Rate : 5/6\n");
        *code_rate =FEC_5_6;
        break;

      case 4:
        NIM_MXL101_PRINTF ("TPS Code Rate : 7/8\n");
        *code_rate =FEC_7_8;
        break;

      default:
        NIM_MXL101_PRINTF ("TPS Code Rate : INVALID CR!\n");
        return ERR_FAILURE; /* error */
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_get_GI(struct nim_device *dev, UINT8 *guard_interval)
* Description: Read MXL101 guard interval
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* guard_interval
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    MXL_DEMOD_TPS_INFO_T TpsData;
//    *guard_interval = MXL101_CurChannelInfo->Guard;
   struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
    if (tpsLockStatus.Status != MXL_LOCKED)
        return ERR_FAILURE;

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_GUARD_INTERVAL_REQ, &TpsData);
    switch(TpsData.TpsInfo)
    {
      case 0:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/32\n");
        *guard_interval = guard_1_32;
        break;

      case 1:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/16\n");
        *guard_interval = guard_1_16;
        break;

      case 2:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/8\n");
        *guard_interval = guard_1_8;
        break;

      case 3:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/4\n");
        *guard_interval = guard_1_4;
        break;

      default:
        NIM_MXL101_PRINTF ("TPS Code Rate : INVALID GUARD INTERVAL!\n");
        return ERR_FAILURE;
    }

    return SUCCESS;
}


/*****************************************************************************
* INT32 f_MXL101_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
* Description: Read MXL101 fft_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* fft_mode
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    MXL_DEMOD_TPS_INFO_T TpsData;
//    *fft_mode = MXL101_CurChannelInfo->Mode;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
    if (tpsLockStatus.Status != MXL_LOCKED)
        return ERR_FAILURE;

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_FFT_MODE_REQ, &TpsData);
    switch(TpsData.TpsInfo)
    {
      case 0:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/32\n");
        *fft_mode = MODE_2K;
        break;

      case 1:
        NIM_MXL101_PRINTF ("TPS Guard Interval : 1/16\n");
        *fft_mode = MODE_8K;
        break;

      default:
        NIM_MXL101_PRINTF ("FFT : INVALID !\n");
        return ERR_FAILURE; // error
    }

    return SUCCESS;

}


/*****************************************************************************
* INT32 f_MXL101_get_modulation(struct nim_device *dev, UINT8 *modulation)
* Description: Read MXL101 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* modulation
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_MXL101_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    MXL_DEMOD_TPS_INFO_T TpsData;
//    *modulation = MXL101_CurChannelInfo->Modulation;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
    if (tpsLockStatus.Status != MXL_LOCKED)
        return ERR_FAILURE;

    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CONSTELLATION_REQ, &TpsData);
    switch(TpsData.TpsInfo)
    {
      case 0:
        NIM_MXL101_PRINTF ("TPS Constellation : QPSK\n");
        * modulation = TPS_CONST_QPSK;
        break;

      case 1:
        NIM_MXL101_PRINTF ("TPS Constellation : 16QAM\n");
        * modulation = TPS_CONST_16QAM;
        break;

      case 2:
        NIM_MXL101_PRINTF ("TPS Constellation : 64QAM\n");
        * modulation = TPS_CONST_64QAM;
        break;

      default:
        NIM_MXL101_PRINTF ("TPS Constellation : INVALID TPS CONSTELLATION!\n");
        return ERR_FAILURE; // error
    }
    return SUCCESS;

}

/*****************************************************************************
* INT32 f_MXL101_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    //UINT8    data;
    UINT32    flgptn;
    OSAL_ER    result;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    result = osal_flag_wait(&flgptn,nim_MXL101_flag[priv_mem->tuner_id], NIM_MXL101_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
            //NIM_PRINTF("Get Spectrum Inv Fail, Wait ChannelChg Complete!\n");
        *Inv=0;
    }
    else
    {
        //f_MXL101_read(dev->base_addr, REGMXL101_STATUS_0  ,&data,1);
        *Inv=0;// 1:inv 0:non-inv
        //soc_printf("Spectral_Inv=0x%x\n",    *Inv);
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_AGC(struct nim_device *dev, UINT8 *agc)
{
     //UINT8        data[2];
     UINT16        SignalStrength=0;
     OSAL_ER    result;
     UINT32        flgptn;
    //struct nim_MXL101_private *dev_priv;
      UINT8 status;
      UINT8 regData;
      UINT16 rxPwr;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    result = osal_flag_wait(&flgptn,nim_MXL101_flag[priv_mem->tuner_id], NIM_MXL101_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("AGC result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get AGC Fail, Wait ChannelChg Complete!\n");
        *agc = 0;
    }
    else
    {

        if( MXL101_CurChannelInfo[priv_mem->tuner_id]->lock_status==1)
        {
             status = Ctrl_WriteRegister(priv_mem->tuner_id, 0x00, 0x02);

              status |= Ctrl_ReadRegister(priv_mem->tuner_id, V6_DIG_RF_PWR_LSB_REG, &regData);
              rxPwr = regData;

              status |= Ctrl_ReadRegister(priv_mem->tuner_id, V6_DIG_RF_PWR_MSB_REG, &regData);
              rxPwr |= (regData & 0x07) << 8;

            NIM_MXL101_PRINTF("rxPwr=%d \n",rxPwr);


              SignalStrength = (UINT32) (rxPwr / 8) +10;

            if(SignalStrength>100)
                SignalStrength=100;

            NIM_MXL101_PRINTF("%d \n",SignalStrength);

              status |= Ctrl_WriteRegister(priv_mem->tuner_id, 0x00, 0x00);

             *agc=SignalStrength;
        }
        else
        {
            *agc=0;
        }
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *snr
*
* Return Value: * snr

*****************************************************************************/
static INT32 f_MXL101_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    //UINT8    data;
    OSAL_ER result;
    UINT32     flgptn;

    MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    MXL_DEMOD_SNR_INFO_T Snr;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    result = osal_flag_wait(&flgptn,nim_MXL101_flag[priv_mem->tuner_id], NIM_MXL101_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("SNR result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
        NIM_PRINTF("Get SNR Fail, Wait ChannelChg Complete!\n");
        *snr = 0;
        return ERR_FAILURE;
    }
    else
    {
        MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
        if (tpsLockStatus.Status == MXL_LOCKED)
        {
            MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_SNR_REQ, &Snr);
            //NIM_MXL101_PRINTF ("SNR : %f dB\n", (REAL32)(Snr.SNR/10000));
            //(REAL32)Snr.SNR shall be conver to (UINT8)snr.

            *snr=Snr.SNR/10000;    //dB: about 0~33.
            *snr = ((*snr)<<1) + ((*snr)>>1) + ((*snr)>>2); //Conver dB to 0~100: dB*(2+0.5+0.25)
            if (*snr>100)
                *snr = 100;
        }
        else
        {
            *snr=0;
        }
    }
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_MXL101_get_BER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    MXL_DEMOD_BER_INFO_T Ber;
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_BER_REQ, &Ber);
       *vbber = Ber.BER;
    return SUCCESS;
}

static INT32 f_MXL101_power_control(struct nim_device *dev,UINT8 StandbyMode)
{
    //INT32 result = ERR_FAILUE;
    MXL_PWR_MODE_CFG_T Param;
   struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);

    if (StandbyMode)
    {
        Param.PowerMode = STANDBY_ON;
        MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_101SF_POWER_MODE_CFG, &Param);
//        libc_printf("tuner standby.\n");
    }
    else
    {
        Param.PowerMode = STANDBY_OFF;
        MxLWare_API_ConfigDevice(priv_mem->tuner_id, MXL_DEV_101SF_POWER_MODE_CFG, &Param);
//        libc_printf("tuner wakeup.\n");
    }

    return SUCCESS;

}

//INT32 f_MXL101_command(struct nim_device *dev, INT32 cmd, UINT32 param)
//{
//    INT32 ret = SUCCESS;
//
//    switch(cmd)
//    {
//        case NIM_TUNER_POWER_CONTROL:
//            ret = f_MXL101_power_control(dev, param);
//            ret = ERR_FAILUE;
//            break;
//
//        default:
//            ret = ERR_FAILUE;
//            break;
//    }
//
//    return ret;
//}

static INT32 f_MXL101_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)    //50622-01Angus
{
    static UINT32 rec_ber_cnt = 0;
    nim_rec_performance_t * p_nim_rec_performance;
    INT32 ret_val = ERR_FAILUE;

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            MXL101_autoscan_stop_flag = param;
        break;

        case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
            p_nim_rec_performance = (nim_rec_performance_t *)param;
            f_MXL101_get_lock(dev, &(p_nim_rec_performance->lock));

            if (p_nim_rec_performance->lock == 1)
            {
                if (rec_ber_cnt !=MXL101_per_tot_cnt)
                {
                    rec_ber_cnt = MXL101_per_tot_cnt;
                    p_nim_rec_performance->ber = MXL101_ber;
                    p_nim_rec_performance->per = MXL101_per;
                    p_nim_rec_performance->valid = 1;
                }
                else
                {
                    p_nim_rec_performance->valid = 0;
                }
            }

            ret_val = SUCCESS;

            break;
        case NIM_DRIVER_GET_AGC:
            return f_MXL101_get_AGC(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_BER:
            return f_MXL101_get_BER(dev, (UINT32 *)param);
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return f_MXL101_get_GI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_FFT_MODE:
            return f_MXL101_get_fftmode(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_MODULATION:
            return f_MXL101_get_modulation(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_SPECTRUM_INV:
            return f_MXL101_get_specinv(dev, (UINT8 *)param);
            break;

        default:
        break;
    }
    return SUCCESS;
}

static INT32 f_MXL101_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list)    //50622-01Angus
{
    struct NIM_CHANNEL_CHANGE *change_para;
    struct NIM_CHANNEL_SEARCH *search_para;
    switch (cmd)
    {
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            return f_MXL101_channel_change(dev, change_para->freq, change_para->bandwidth, \
                change_para->guard_interval, change_para->fft_mode, change_para->modulation, \
                change_para->fec, change_para->usage_type, change_para->inverse, change_para->priority);
            break;
        case NIM_DRIVER_CHANNEL_SEARCH:
            search_para = (struct NIM_CHANNEL_SEARCH *)param_list;
            return f_MXL101_channel_search(dev, search_para->freq, search_para->bandwidth, \
                search_para->guard_interval, search_para->fft_mode, search_para->modulation, \
                search_para->fec, search_para->usage_type, search_para->inverse, \
                search_para->freq_offset, search_para->priority);
            break;
        default:
            break;
    }
    return SUCCESS;
}


#if 0
void f_MXL101_tracking_task(struct nim_device *dev)
{
    UINT32     flgptn;
    UINT8 data[1];
    TMO     start_time, temp_time;


    while(1)
    {
        osal_flag_wait(&flgptn,nim_MXL101_flag, NIM_MXL101_CHECKING, OSAL_TWF_ANDW,OSAL_WAIT_FOREVER_TIME);
        osal_flag_clear(nim_MXL101_flag,NIM_MXL101_CHECKING);


//        f_MXL101_channel_change(dev,  MXL101_CurChannelInfo->Frequency, MXL101_CurChannelInfo->ChannelBW,
//                    MXL101_CurChannelInfo->Guard, MXL101_CurChannelInfo->Mode, MXL101_CurChannelInfo->Modulation, MXL101_CurChannelInfo->FECRates, MODE_CHECKING, 0);

        osal_flag_clear(nim_MXL101_flag,NIM_MXL101_SCAN_END);

        data[0]=0x10 ;
        f_MXL101_write(dev->base_addr,REGMXL101_TUNERGO,data,1);

        start_time = osal_get_tick();
        while(1)
        {
            f_MXL101_read(dev->base_addr,REGMXL101_STATUS_0 , data, 1);
            if ((data[0] & MXL101_OFDM_LOCK_FLAG) == MXL101_OFDM_LOCK_FLAG)
            {
                break;
            }
            osal_task_sleep(20);

            temp_time = osal_get_tick();

            if(temp_time >= start_time ?  ( temp_time - start_time > 512) : (start_time - temp_time < 0xFFFFFFFF - 512))
            {
                osal_flag_set(nim_MXL101_flag, NIM_MXL101_CHECKING);
                break;
            }
        }
        osal_flag_set(nim_MXL101_flag,NIM_MXL101_SCAN_END);
        osal_task_sleep(50);
    }
}


static INT32 f_MXL101_tracking_task_init(struct nim_device *dev)
{
    ER    ret_val;
    OSAL_T_CTSK    t_ctsk;

#if (SYS_SDRAM_SIZE == 2)
    t_ctsk.stksz    = 0x200;
#else
    t_ctsk.stksz    = 0x400;
#endif
    t_ctsk.quantum= 10;
    t_ctsk.itskpri    = OSAL_PRI_NORMAL;
    t_ctsk.task = (FP)f_MXL101_tracking_task;
    t_ctsk.para1 = (UINT32)dev;

    f_MXL101_tracking_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == f_MXL101_tracking_task_id)
    {
        NIM_PRINTF("create_task nim_MXL101_tracking_task_id failed\n");
        return FALSE;
    }
    return TRUE;
}
#endif
/*****************************************************************************
* INT32 f_MXL101_get_freq_offset(struct nim_device *dev,INT32 *freq_offset)
* Description: Read MXL101 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: INT32 *freq_offset
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_MXL101_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)//051222 yuchun
{
        OSAL_ER    result;
        UINT32    flgptn;
        struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
        result = osal_flag_wait(&flgptn,nim_MXL101_flag[priv_mem->tuner_id], NIM_MXL101_SCAN_END, OSAL_TWF_ANDW,0);
       // libc_printf("offset result = %d\r\n",result);

    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get guard_interval Fail, Wait ChannelChg Complete!\n");
//        *guard_interval=0;
        *freq_offset=0xffffffff;//050725 yuchun
    }
        else
        {
            f_MXL101_freq_offset(dev, MXL101_CurChannelInfo[priv_mem->tuner_id]->ChannelBW, MXL101_CurChannelInfo[priv_mem->tuner_id]->Mode, freq_offset);
       }

//       soc_printf(" offset =%d\n", *freq_offset );
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_hier_mode(struct nim_device *dev)
* Description: Read MXL101 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_hier_mode(struct nim_device *dev)
{
UINT8 data = 0;
UINT8 hier;

// f_MXL101_read(dev->base_addr, REGMXL101_TPSRECEIVED ,&data,1);

    data=(data&TPS_HIERMODE)>>2;
    switch(data)
    {
        case 0 : hier=HIER_NONE;
        break;
        case 1 :hier=HIER_1;
        break;
        case 2 : hier=HIER_2;
        break;
        case 3 : hier=HIER_4;
        break;
        default :
                hier=HIER_NONE;
        break; /* error */
    }
        return hier;
}


/*****************************************************************************
* INT32 f_MXL101_get_hier_mode(struct nim_device *dev,UINT8*hier)
* Description: Read MXL101 hier_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8*hier
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_MXL101_get_hier_mode(struct nim_device *dev, UINT8*hier)
{
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    *hier= MXL101_CurChannelInfo[priv_mem->tuner_id]->Hierarchy;
     //soc_printf(" hier a %d\n", *hier);
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_MXL101_priority(struct nim_device *dev,UINT8*priority)
* Description: Read MXL101 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: UINT8*priority
*
* Return Value: INT32
*****************************************************************************/

INT8 f_MXL101_priority(struct nim_device *dev, UINT8*priority)
{
    struct nim_MXL101_private * priv_mem = (struct nim_MXL101_private *)(dev->priv);
    *priority= MXL101_CurChannelInfo[priv_mem->tuner_id]->Priority;
    return SUCCESS;
}


//#endif
