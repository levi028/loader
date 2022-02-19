/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL301.c
*
*    Description:    Source file of MAXLINEAR MxL301 TUNER.
*    History:
*           Date          Author                 Version          Reason
*        ============    =============    =========    =================
*   4.17.2008         David.Deng          Ver 0.1         Create file.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

#include "mxl_api.h"
#include "mxL_user_define.h"
#include "mxl301rf.h"
#include "tun_mxl301.h"


#if ((SYS_TUN_MODULE == MXL301) || (SYS_TUN_MODULE == ANY_TUNER))


//#define MXL_DEBUG

#define NIM_PRINTF(...)

#define MXL_DVBT_DEBUG  0
#if(MXL_DVBT_DEBUG)
#define NIM_MXL301_PRINTF  libc_printf
#else
#define NIM_MXL301_PRINTF(...)
#endif

static BOOL run_on_through_mode(UINT32 tuner_id);

struct COFDM_TUNER_CONFIG_EXT stMxL301_Config[MAX_TUNER_SUPPORT_NUM];
static MxLxxxRF_TunerConfigS Mxl301_TunerConfig[MAX_TUNER_SUPPORT_NUM];
DEM_WRITE_READ_TUNER m_ThroughMode[MAX_TUNER_SUPPORT_NUM];
static BOOL bMxl_Tuner_Inited[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;



//void ASSERT_REG0()
//{
//    UINT8 data;
//    MxL_Get_Register(&Mxl301_TunerConfig[0], 0, &data);
//
//    if(data != 0x3f)
//    {
//        ASSERT(0);
//    }
//}



//===================================================================================================
// mxL_user_define.c
//===================================================================================================
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
#define BURST_SZ 6

UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count, UINT32 tuner_id)
{
    int status = 0;
    int i,j;
    UINT8 szBuffer[BURST_SZ];
    UINT16 BurstNum, RemainLen;
/*
    if(sys_ic_is_M3101()!= 1)
    {
        stMxL301_Config[tuner_id].i2c_type_id=0;
        stMxL301_Config[tuner_id].c_tuner_base_addr=MxL_I2C_ADDR_96;
    }
*/
    if ( !count ) return 0;


    RemainLen = count % BURST_SZ;
    if ( RemainLen )
    {
        BurstNum = count / BURST_SZ;
    }
    else
    {
        BurstNum = count / BURST_SZ - 1;
        RemainLen = BURST_SZ;
    }

    for ( i = 0 ; i < BurstNum; i ++ )
    {
        for ( j = 0 ; j < BURST_SZ ; j++  )
        {
            szBuffer[j]   = pArray[i * BURST_SZ + j ];             // addr
            //szBuffer[j+1] = pArray[i * BURST_PAIR_SZ + j +1];    // value

        }

        if (run_on_through_mode(tuner_id))
            status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, stMxL301_Config[tuner_id].c_tuner_base_addr, szBuffer, BURST_SZ, 0, 0);
        else
            status = stMxL301_Config[tuner_id].tuner_write(stMxL301_Config[tuner_id].i2c_type_id, stMxL301_Config[tuner_id].c_tuner_base_addr, szBuffer, BURST_SZ);
        if(status != SUCCESS)
        {
            NIM_MXL301_PRINTF("MXL301 WR1 err=%d\n",status);
            return ERR_FAILUE;
        }
    }

    for ( i = 0 ; i < RemainLen ; i++ )
    {
        szBuffer[i]   = pArray[BurstNum * BURST_SZ + i ];         // addr
        //szBuffer[i+1] = pArray[BurstPairs * BURST_PAIR_SZ + i+1 ];   // value
        // libc_printf("%x  %x \n", szBuffer[i] , szBuffer[i+1]  );
    }

    if (run_on_through_mode(tuner_id))
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, stMxL301_Config[tuner_id].c_tuner_base_addr, szBuffer, RemainLen, 0, 0);
    else
        status = stMxL301_Config[tuner_id].tuner_write(stMxL301_Config[tuner_id].i2c_type_id, stMxL301_Config[tuner_id].c_tuner_base_addr, szBuffer, RemainLen);
    if(status != SUCCESS)
    {
        NIM_MXL301_PRINTF("MXL301 WR2 err=%d\n",status);
        return ERR_FAILUE;
    }

    return status;

}

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
UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, UINT32 tuner_id)
{
    int status = 0;
    UINT8 Read_Cmd[2];

    /* read step 1. accroding to mxl301 driver API user guide. */
    Read_Cmd[0] = 0xFB;
    Read_Cmd[1] = Addr;
    if (run_on_through_mode(tuner_id))
    {
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, stMxL301_Config[tuner_id].c_tuner_base_addr, Read_Cmd, 2, 0, 0);
        if(status != SUCCESS)
            return ERR_FAILUE;
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, stMxL301_Config[tuner_id].c_tuner_base_addr, 0, 0, mData, 1);
        if(status != SUCCESS)
            return ERR_FAILUE;
    }
    else
    {
        stMxL301_Config[tuner_id].tuner_write(stMxL301_Config[tuner_id].i2c_type_id, stMxL301_Config[tuner_id].c_tuner_base_addr, Read_Cmd, 2);
        stMxL301_Config[tuner_id].tuner_read(stMxL301_Config[tuner_id].i2c_type_id, stMxL301_Config[tuner_id].c_tuner_base_addr, mData, 1);
    }

    return SUCCESS;
}


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
void MxL_Delay(UINT32 mSec)
{
    osal_task_sleep(mSec);
}

/* '11/02/22 : OKAMOTO    Read/Write data on specified bit. */
/*====================================================*
    MXL301_register_read_reg_struct
   --------------------------------------------------
    Description     read data on specified bit
    Argument        DeviceAddr    - MxL Tuner Device address
                    in_reg    Register address
                    pRegData    Register data
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
UINT32 MXL301_register_read_reg_struct(UINT8 DeviceAddr, MXL301_REG_STRUCT in_reg, UINT8 *pRegData, UINT32 tuner_id){

    UINT8 nFilter = 0;
    UINT8 nReadData = 0;
    UINT8 i;
    UINT32 Status=0;


    /* Check input value */
    if(in_reg.BottomBit > in_reg.TopBit )
        return MxL_ERR_OTHERS;
    if(in_reg.TopBit > 7)
        return MxL_ERR_OTHERS;

    /* Read register */
    Status = MxL_I2C_Read(DeviceAddr, in_reg.RegAddr, &nReadData, tuner_id);
    if(Status != MxL_OK){
        return Status;
    }

    /* Shift Data */
    nReadData = nReadData >> in_reg.BottomBit;

    /* Make Filter */
    nFilter = 0x00;
    for(i=0; i<(in_reg.TopBit-in_reg.BottomBit+1); i++){
        nFilter |= 1<<i;
    }

    /* Filter read data */
    *pRegData = nFilter & nReadData;

    return MxL_OK;
}

/* '11/02/22 : OKAMOTO    Read/Write data on specified bit. */
/*====================================================*
    MXL301_register_write_reg_struct
   --------------------------------------------------
    Description     write data on specified bit
    Argument        DeviceAddr    - MxL Tuner Device address
                    in_reg    Register address
                    RegData    Register data
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
UINT32 MXL301_register_write_reg_struct(UINT8 DeviceAddr, MXL301_REG_STRUCT in_reg, UINT8 RegData, UINT32 tuner_id){
    UINT8 nFilter = 0;
    UINT8 nWriteData = 0;
    UINT8 nReadData = 0;
    UINT8 i;
    UINT32 Status=0;


    /* Check input value */
    if(in_reg.BottomBit > in_reg.TopBit )
        return MxL_ERR_OTHERS;
    if(in_reg.TopBit > 7)
        return MxL_ERR_OTHERS;
    if( RegData >= ( 1<< (in_reg.TopBit - in_reg.BottomBit+1) ) )
        return MxL_ERR_OTHERS;

    if( (in_reg.TopBit==7) && (in_reg.BottomBit==0) ){
        //Not use filering
        nWriteData = RegData;
    }else{
        /* Read register */
        Status = MxL_I2C_Read(DeviceAddr, in_reg.RegAddr, &nReadData, tuner_id);
        if(Status != MxL_OK){
            return Status;
        }

        /* Make Filter */
        nFilter = 0x00;
        for(i=0; i<in_reg.BottomBit; i++){
            nFilter |= 1<<i;
        }
        for(i=in_reg.TopBit+1; i<8; i++){
            nFilter |= 1<<i;
        }

        /* Filtering */
        nFilter = nFilter & nReadData;


        /* Shift write data*/
        nWriteData = RegData<<in_reg.BottomBit;

        /* Make write data */
        nWriteData = nFilter | nWriteData;
    }

    /* Write register */
    {
        UINT8 Array[2];
        Array[0] = in_reg.RegAddr;
        Array[1] = nWriteData;
        Status = MxL_I2C_Write(DeviceAddr, Array, 2, tuner_id);
    }
    return Status;
}


UINT32 MXL301_register_write_bit_name(UINT8 DeviceAddr, MxL_BIT_NAME bit_name, UINT8 RegData, UINT32 tuner_id){
    const MXL301_REG_STRUCT    DIG_IDAC_CODE    = {0x0D, 5, 0};
    const MXL301_REG_STRUCT    DIG_ENIDAC_BYP    = {0x0D, 7, 7};
    const MXL301_REG_STRUCT    DIG_ENIDAC      = {0x0D, 6, 6};
    const MXL301_REG_STRUCT    DIG_ATTOFF      = {0x0B, 5, 0};
    const MXL301_REG_STRUCT    DIG_ATTON       = {0x0C, 5, 0};
    const MXL301_REG_STRUCT    DIG_IDAC_MODE    = {0x0C, 6, 6};
    const MXL301_REG_STRUCT    FRONT_BO        = {0x78, 5, 0};
    const MXL301_REG_STRUCT    IF1_OFF     = {0x00, 0, 0};
    const MXL301_REG_STRUCT    IF2_OFF     = {0x00, 1, 1};
    const MXL301_REG_STRUCT    MAIN_TO_IF2    = {0x00, 4, 4};
    const MXL301_REG_STRUCT    AGC_MODE    = {0x21, 0, 0};

    /* '11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix". */
    const MXL301_REG_STRUCT    AGC_GAIN_FIX    = {0x15, 3, 2};

    MxL_ERR_MSG Status;
    switch(bit_name){
    case MxL_BIT_NAME_DIG_IDAC_CODE:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_IDAC_CODE, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_DIG_ENIDAC_BYP:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_ENIDAC_BYP, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_DIG_ENIDAC:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_ENIDAC, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_DIG_ATTOFF:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_ATTOFF, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_DIG_ATTON:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_ATTON, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_DIG_IDAC_MODE:
        Status = MXL301_register_write_reg_struct(DeviceAddr, DIG_IDAC_MODE, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_FRONT_BO:
        Status = MXL301_register_write_reg_struct(DeviceAddr, FRONT_BO, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_IF1_OFF:
        Status = MXL301_register_write_reg_struct(DeviceAddr, IF1_OFF, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_IF2_OFF:
        Status = MXL301_register_write_reg_struct(DeviceAddr, IF2_OFF, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_MAIN_TO_IF2:
        Status = MXL301_register_write_reg_struct(DeviceAddr, MAIN_TO_IF2, RegData, tuner_id);
        break;
    case MxL_BIT_NAME_AGC_MODE:
        Status = MXL301_register_write_reg_struct(DeviceAddr, AGC_MODE, RegData, tuner_id);
        break;

    /* '11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix". */
    case MxL_BIT_NAME_AGC_GAIN_FIX:
        Status = MXL301_register_write_reg_struct(DeviceAddr, AGC_GAIN_FIX, RegData, tuner_id);
        break;

    default:
        Status = MxL_ERR_OTHERS;
        break;
    }
    return Status;
}


#if 0

//===================================================================================================
//mxl_api.c
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                                                           //
//                            Tuner Functions                                   //
//                                                                           //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MxL_ERR_MSG MxL_Set_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 RegData)
{
    UINT32 Status=0;
    UINT8 pArray[2];
    pArray[0] = RegAddr;
    pArray[1] = RegData;

    Status = MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id);
    if(Status) return MxL_ERR_SET_REG;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Get_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 *RegData)
{
    if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, RegAddr, RegData,myTuner->tuner_id))
        return MxL_ERR_GET_REG;
    return MxL_OK;

}


MxL_ERR_MSG MxL_Soft_Reset(MxLxxxRF_TunerConfigS* myTuner)
{
    UINT32 Status=0;
    UINT8 reg_reset;
    reg_reset = 0xFF;
    Status = MxL_I2C_Write((UINT8)myTuner->I2C_Addr, &reg_reset, 1,myTuner->tuner_id);
    if(Status)
        return (MxL_ERR_MSG)Status;

    return MxL_OK;
}


MxL_ERR_MSG MxL_Stand_By(MxLxxxRF_TunerConfigS* myTuner)
{
    UINT8 pArray[4];    /* a array pointer that store the addr and data pairs for I2C write    */

    pArray[0] = 0x01;
    pArray[1] = 0x0;
    pArray[2] = 0x13;   //START_TUNE
    pArray[3] = 0x0;    //0:abort.

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 4,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    return MxL_OK;
}

/* '11/10/11 : OKAMOTO    Implement "MxL_PowerUpSettings". */
MxL_ERR_MSG MxL_PowerUpSettings(MxLxxxRF_TunerConfigS* myTuner)
{
    UINT8 pArray[2];    /* a array pointer that store the addr and data pairs for I2C write    */

    pArray[0] = 0x01;
    pArray[1] = 0x01;

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Wake_Up(MxLxxxRF_TunerConfigS* myTuner)
{
#if 0
    UINT8 pArray[2];    /* a array pointer that store the addr and data pairs for I2C write    */

    pArray[0] = 0x01;
    pArray[1] = 0x01;

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
        return MxL_ERR_OTHERS;
#else
    /* '11/10/11 : OKAMOTO    Implement "MxL_PowerUpSettings". */
    MxL_ERR_MSG Status = MxL_PowerUpSettings(myTuner);
    if(Status!=MxL_OK)
        return Status;
#endif
    if(MxL_Tuner_RFTune(myTuner))
        return MxL_ERR_RFTUNE;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Tuner_Init(MxLxxxRF_TunerConfigS* myTuner)
{
    UINT8 pArray[MAX_ARRAY_SIZE];    /* a array pointer that store the addr and data pairs for I2C write */
    UINT32 Array_Size = 0;                /* a integer pointer that store the number of element in above array */

    UINT32 Status;

    /* Soft reset tuner */
    if(MxL_Soft_Reset(myTuner))
        return MxL_ERR_INIT;

    /* perform initialization calculation */
    if(myTuner->TunerID == MxL_TunerID_MxL202RF)
    {
        /* Uncomment section below for MxL202RF */
        /*Status = MxL202RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
                (UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
                (UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->BW_MHz, (UINT8)myTuner->AGC);
        if(Status) //if failure
            return (MxL_ERR_MSG)Status;
        */

        return MxL_ERR_INIT; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
    }
    else if (myTuner->TunerID == MxL_TunerID_MxL301RF)
    {
        /* Uncomment section below for MxL301RF */
        Status = MxL301RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
                (UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
                (UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->AGC, (UINT8)myTuner->IF_Path

                /* '11/10/06 : OKAMOTO    Select AGC external or internal. */
                ,myTuner->bInternalAgcEnable
                );
        if(Status) //if failure
            return (MxL_ERR_MSG)Status;

    }
    else if (myTuner->TunerID == MxL_TunerID_MxL302RF)
    {
        /* Uncomment section below for MxL302RF */
        /* Status = MxL302RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
                (UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
                (UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->AGC, (UINT8)myTuner->IF_Split, (UINT8)myTuner->IF_Path);
        if(Status) //if failure
            return (MxL_ERR_MSG)Status;
        */

        return MxL_ERR_INIT; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
    }

    /* perform I2C write here */
    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
        return MxL_ERR_INIT;

    /* '11/10/06 : OKAMOTO    Control AGC set point. */
    {
        UINT8 pArray[2];
        pArray[0] = 0x2A;
        pArray[1] = myTuner->AGC_set_point;
        if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, sizeof(pArray),myTuner->tuner_id))
            return MxL_ERR_INIT;
    }

    MxL_Delay(1);    /* 1ms delay*/

    return MxL_OK;
}



/* '11/03/16 : OKAMOTO    Select IDAC setting. */
/*====================================================*
    MxL_idac_setting
   --------------------------------------------------
    Description     IDAC setting
    Argument        INT8 DeviceAddr
                    idac_setting    (Select IDAC setting.)
                    idac_hysterisis    (Set hysterisis in auto setting.)
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_idac_setting(UINT8 DeviceAddr, MxL_IDAC_SETTING idac_setting, MxL_IDAC_HYSTERISIS idac_hysterisis, UINT8 dig_idac_code, UINT32 tuner_id)
{
    switch(idac_setting){
    case MxL_IDAC_SETTING_AUTO:
        {
            UINT8 Array[] = {
                0x0D, 0x00,
                0x0C, 0x67,
                0x6F, 0x89,
                0x70, 0x0C,
                0x6F, 0x8A,
                0x70, 0x0E,
                0x6F, 0x8B,
                0x70, 0x10,
            };

            if(idac_hysterisis<0 || idac_hysterisis>=MxL_IDAC_HYSTERISIS_MAX){
                return MxL_ERR_OTHERS;
            }else{
                UINT8 ui8_idac_hysterisis;
                ui8_idac_hysterisis = (UINT8)idac_hysterisis;
                Array[15] = Array[15]+ui8_idac_hysterisis;
            }
            return (MxL_ERR_MSG)MxL_I2C_Write(DeviceAddr, Array, sizeof(Array), tuner_id);
        }
        break;
    case MxL_IDAC_SETTING_MANUAL:
        if(dig_idac_code<0 || dig_idac_code>=63){
            return MxL_ERR_OTHERS;
        }else{
            UINT8 Array[] = {0x0D, 0x0};
            Array[1] = 0xc0 + dig_idac_code;    //DIG_ENIDAC_BYP(0x0D[7])=1, DIG_ENIDAC(0x0D[6])=1
            return (MxL_ERR_MSG)MxL_I2C_Write(DeviceAddr, Array, sizeof(Array), tuner_id);
        }
        break;
    case MxL_IDAC_SETTING_OFF:
        return (MxL_ERR_MSG)MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_DIG_ENIDAC, 0, tuner_id);        //0x0D[6]    0
        break;
    default:
        return MxL_ERR_OTHERS;
    }
}

//MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS* myTuner, UINT32 RF_Freq_Hz, MxLxxxRF_BW_MHz BWMHz)
MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS* myTuner)
{
    UINT8 pArray[MAX_ARRAY_SIZE];    /* a array pointer that store the addr and data pairs for I2C write */
    UINT32 Array_Size = 0;                /* a integer pointer that store the number of element in above array */

    UINT32 Status;
    UINT8 Data1, Data2;
    SINT16 Data;
    UINT8 i;

    //Store information into struc
//    myTuner->RF_Freq_Hz = RF_Freq_Hz;
//    myTuner->BW_MHz = BWMHz;

    /* perform Channel Change calculation */
    if(myTuner->TunerID == MxL_TunerID_MxL202RF)
    {
        /* Uncomment section below for MxL202RF */
        /*Status = MxL202RF_RFTune(pArray,&Array_Size,RF_Freq_Hz,BWMHz, myTuner->Mode);
        if (Status)
            return MxL_ERR_RFTUNE;
        */

        return MxL_ERR_RFTUNE; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
    }
    else if (myTuner->TunerID == MxL_TunerID_MxL301RF)
    {
        /* Uncomment section below for MxL301RF */
        Status = MxL301RF_RFTune(pArray,&Array_Size,myTuner->RF_Freq_Hz,myTuner->BW_MHz, myTuner->Mode);
        if (Status)
            return MxL_ERR_RFTUNE;

    }
    else if (myTuner->TunerID == MxL_TunerID_MxL302RF)
    {
        /* Uncomment section below for MxL302RF */
        /* Status = MxL302RF_RFTune(pArray,&Array_Size,RF_Freq_Hz,BWMHz, myTuner->Mode, myTuner->IF_Split);
        if (Status)
            return MxL_ERR_RFTUNE;
        */

        return MxL_ERR_RFTUNE; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
    }

    /* perform I2C write here */
    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
        return MxL_ERR_RFTUNE;

    MxL_Delay(1); /* Added V9.2.1.0 */

    /* Register read-back based setting for Analog M/N split mode only */
    if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode == MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
    {
        MxL_Get_Register(myTuner, 0xE3, &Data1);
        MxL_Get_Register(myTuner, 0xE4, &Data2);
        Data = ((Data2&0x03)<<8) + Data1;
        if(Data >= 512)
            Data = Data - 1024;

        if(Data < 20)
        {
            MxL_Set_Register(myTuner, 0x85, 0x43);
            MxL_Set_Register(myTuner, 0x86, 0x08);
        }
        else if (Data >= 20)
        {
            MxL_Set_Register(myTuner, 0x85, 0x9E);
            MxL_Set_Register(myTuner, 0x86, 0x0F);
        }

        for(i = 0; i<Array_Size; i+=2)
        {
            if(pArray[i] == 0x11)
                Data1 = pArray[i+1];
            if(pArray[i] == 0x12)
                Data2 = pArray[i+1];
        }
        MxL_Set_Register(myTuner, 0x11, Data1);
        MxL_Set_Register(myTuner, 0x12, Data2);
    }

    if (myTuner->TunerID == MxL_TunerID_MxL301RF || myTuner->TunerID == MxL_TunerID_MxL302RF)
        MxL_Set_Register(myTuner, 0x13, 0x01);  //START_TUNE: start. 4.2.8 Sequencer Settings, MxL301RF_Prog_Guide_Release_9.4.4.0.pdf.

    if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode >= MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
    {
        if(MxL_Set_Register(myTuner, 0x00, 0x01))
            return MxL_ERR_RFTUNE;
    }
    MxL_Delay(30);

    if((myTuner->Mode == MxL_MODE_DVBT) || (myTuner->Mode >= MxL_MODE_ANA_MN))
    {
        if(MxL_Set_Register(myTuner, 0x1A, 0x0D))   //4.2.9 Post-Tune Settings, MxL301RF_Prog_Guide_Release_9.4.4.0.pdf.
            return MxL_ERR_SET_REG;
    }
    if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode >= MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
    {
        if(MxL_Set_Register(myTuner, 0x00, 0x00))
            return MxL_ERR_RFTUNE;
    }

    /* '11/03/16 : OKAMOTO    Select IDAC setting in "MxL_Tuner_RFTune". */
    Status = MxL_idac_setting(myTuner->I2C_Addr, myTuner->idac_setting, myTuner->idac_hysterisis, myTuner->dig_idac_code, myTuner->tuner_id);
    if(Status!=MxL_OK){
        return (MxL_ERR_MSG)Status;
    }

    return MxL_OK;
}

MxL_ERR_MSG MxL_Check_ChipVersion(MxLxxxRF_TunerConfigS* myTuner, MxLxxxRF_ChipVersion* myChipVersion)
{
    UINT8 Data;
    if(MxL_Get_Register(myTuner, 0x17, &Data))
        return MxL_GET_ID_FAIL;

    switch(Data & 0x0F)
    {
    case 0x09: *myChipVersion=MxL_30xRF_V9; break;
    default:
        *myChipVersion=MxL_UNKNOWN_ID;
        return MxL_ERR_UNKNOWN_ID;
    }

    return MxL_OK;
}
MxL_ERR_MSG MxL_RFSynth_Lock_Status(MxLxxxRF_TunerConfigS* myTuner, BOOL* isLock)
{
    UINT8 Data;
    *isLock = FALSE;

    //it's normal, version&0xF == 1001(MxL301RF_V3). MxL301RF_Prog_Guide_Release_9.4.4.0.pdf, p36.
    UINT8 version=0;
    if(MxL_Get_Register(myTuner, 0x17, &version))
    {
        libc_printf("fail to read version \n");
    }
    libc_printf("version = 0x%x \n",version);

    if(MxL_Get_Register(myTuner, 0x16, &Data))
        return MxL_ERR_OTHERS;

    /*
    Data &= 0x0C;
    if (Data == 0x0C)
        *isLock = TRUE;  // RF Synthesizer is Lock
    */

    //Both RF and REF Synthesizer are Locked. MxL301RF_Prog_Guide_Release_9.4.4.0.pdf, p36.
    Data &= 0x0A;
    if (Data == 0x0A)
        *isLock = TRUE;

    return MxL_OK;
}

MxL_ERR_MSG MxL_REFSynth_Lock_Status(MxLxxxRF_TunerConfigS* myTuner, BOOL* isLock)
{
    UINT8 Data;
    *isLock = FALSE;
    if(MxL_Get_Register(myTuner, 0x16, &Data))
        return MxL_ERR_OTHERS;
    Data &= 0x03;
    if (Data == 0x03)
        *isLock = TRUE;   /*REF Synthesizer is Lock */
    return MxL_OK;
}

/* '11/02/22 : OKAMOTO    IF out selection. */
/*====================================================*
    MxL_if_out_select
   --------------------------------------------------
    Description     write data on specified bit
    Argument        DeviceAddr    - MxL Tuner Device address
                    if_out    IF output
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_if_out_select(UINT8 DeviceAddr, MxL_IF_OUT if_out, UINT32 tuner_id)
{
    UINT8 if1_off_RegData = 0;
    UINT8 if2_off_RegData = 0;
    UINT8 main_to_if2_RegData = 0;
//    UINT8 agc_mode_RegData = 0;

    UINT32 Status;

    switch(if_out){
    case MxL_IF_OUT_1:
        if1_off_RegData = 0;
        if2_off_RegData = 1;
        main_to_if2_RegData = 0;
//        agc_mode_RegData = 0;
        break;
    case MxL_IF_OUT_2:
        if1_off_RegData = 1;
        if2_off_RegData = 0;
        main_to_if2_RegData = 1;
//        agc_mode_RegData = 1;
        break;
    default:
        return MxL_ERR_OTHERS;
    }

    Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_IF1_OFF, if1_off_RegData, tuner_id);
    if(Status!=MxL_OK){
        return Status;
    }
    Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_IF2_OFF, if2_off_RegData, tuner_id);
    if(Status!=MxL_OK){
        return Status;
    }
    Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_MAIN_TO_IF2, main_to_if2_RegData, tuner_id);
    if(Status!=MxL_OK){
        return Status;
    }
//    Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_AGC_MODE, agc_mode_RegData, tuner_id);
//    if(Status!=MxL_OK){
//        return Status;
//    }

    return MxL_OK;
}

/* '11/06/22 : OKAMOTO    AGC mode selection. */
/*====================================================*
    MxL_agc_mode_select
   --------------------------------------------------
    Description     External VAGC pin selection, only work for DIG_AGCSEL=1.
    Argument        DeviceAddr    - MxL Tuner Device address
                    agc_mode
                    (0= Select VAGC1 (Default), 1= Select VAGC2)
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_mode_select(UINT8 DeviceAddr, MxL_AGC_MODE agc_mode, UINT32 tuner_id)
{
    UINT32 Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_AGC_MODE, agc_mode, tuner_id);
    if(Status!=MxL_OK){
        return (MxL_ERR_MSG)Status;
    }

    return MxL_OK;
}

/* '11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix". */
/*====================================================*
    MxL_agc_gain_fix
   --------------------------------------------------
    Description     Fix AGC gain.
    Argument        DeviceAddr    - MxL Tuner Device address
                    bFixGain(TRUE:Fix gain , FALSE:Not fix)
    Return Value    UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_gain_fix(UINT8 DeviceAddr, BOOL bFixGain, UINT32 tuner_id)
{
    UINT8    RegData = 0;
    if(bFixGain){
        RegData = 3;
    }
    return (MxL_ERR_MSG)MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_AGC_GAIN_FIX, RegData, tuner_id);
}
#endif



//===================================================================================================
// Ali interface.
//===================================================================================================

static BOOL run_on_through_mode(UINT32 tuner_id)
{
    return (m_ThroughMode[tuner_id].nim_dev_priv && m_ThroughMode[tuner_id].dem_write_read_tuner);
}

static INT32 set_through_mode(UINT32 tuner_id, DEM_WRITE_READ_TUNER *ThroughMode)
{
    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;
      MEMCPY((UINT8*)(&m_ThroughMode[tuner_id]), (UINT8*)ThroughMode, sizeof(DEM_WRITE_READ_TUNER));
    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl301_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner mxl301 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl301_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    MxLxxxRF_ChipVersion chip_ver=0;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    ptrTuner_Config->c_tuner_base_addr = MxL_I2C_ADDR_97;    //It's fixed address in Sharp6158 full nim.
    MEMCPY(&stMxL301_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));

      MEMSET((UINT8*)(&m_ThroughMode[tuner_cnt]), 0, sizeof(DEM_WRITE_READ_TUNER));
    bMxl_Tuner_Inited[tuner_cnt] = FALSE;


    Mxl301_TunerConfig[tuner_cnt].tuner_id = tuner_cnt;
    Mxl301_TunerConfig[tuner_cnt].I2C_Addr = (MxLxxxRF_I2CAddr)(stMxL301_Config[tuner_cnt].c_tuner_base_addr);

    Mxl301_TunerConfig[tuner_cnt].TunerID = MxL_TunerID_MxL301RF;

    /* TV Standard */
    Mxl301_TunerConfig[tuner_cnt].Mode = MxL_MODE_DVBT;

    /* Xtal Frequency */
    Mxl301_TunerConfig[tuner_cnt].Xtal_Freq = MxL_XTAL_24_MHZ;
//        Mxl301_TunerConfig[tuner_id].Xtal_Freq = (MxLxxxRF_Xtal_Freq)(stMxL301_Config[tuner_id].cTuner_Crystal*1000);

    /* IF Frequency */
    Mxl301_TunerConfig[tuner_cnt].IF_Freq = MxL_IF_5_MHZ;//MxL_IF_4_MHZ
//        Mxl301_TunerConfig[tuner_id].IF_Freq = (MxLxxxRF_IF_Freq)(stMxL301_Config[tuner_id].wTuner_IF_Freq*1000);

    /* IF Inversion */
    Mxl301_TunerConfig[tuner_cnt].IF_Spectrum = MxL_NORMAL_IF;

    /* Clock out */
    Mxl301_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_DISABLE;
    Mxl301_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_0;
//        Mxl301_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_ENABLE;
//        Mxl301_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_7;

    Mxl301_TunerConfig[tuner_cnt].Xtal_Cap = MxL_XTAL_CAP_8_PF;
    Mxl301_TunerConfig[tuner_cnt].AGC = MxL_AGC_SEL1; //Set AGC selection
    Mxl301_TunerConfig[tuner_cnt].IF_Split = MxL_IF_SPLIT_DISABLE;
    Mxl301_TunerConfig[tuner_cnt].IF_Path = MxL_IF_PATH1; //Set Tuner IF Output Path

    /* '11/10/06 : OKAMOTO    Control AGC set point. */
    Mxl301_TunerConfig[tuner_cnt].AGC_set_point = 0x93;

    /* '11/10/06 : OKAMOTO    Select AGC external or internal. */
    Mxl301_TunerConfig[tuner_cnt].bInternalAgcEnable = FALSE;//TRUE

    Mxl301_TunerConfig[tuner_cnt].RF_Freq_Hz = 474000*1000; //Hz
    Mxl301_TunerConfig[tuner_cnt].BW_MHz = MxL_BW_8MHz;

    Mxl301_TunerConfig[tuner_cnt].idac_setting = MxL_IDAC_SETTING_OFF;
    Mxl301_TunerConfig[tuner_cnt].idac_hysterisis = MxL_IDAC_HYSTERISIS_0_00DB;
    Mxl301_TunerConfig[tuner_cnt].dig_idac_code = 0;

//    MxL_Check_ChipVersion(&Mxl301_TunerConfig[tuner_cnt],&chip_ver);
//    NIM_MXL301_PRINTF("chip_ver=%x\n",chip_ver);

    if(*tuner_id)
        *tuner_id = tuner_cnt;
    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl301_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_mxl301_status(UINT32 tuner_id, UINT8 *lock)
{
    BOOL IfLock;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    MxL_RFSynth_Lock_Status(&Mxl301_TunerConfig[tuner_id], &IfLock);

    //*lock = 1;
    *lock = ( IfLock == TRUE ) ? 1 : 0;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl301_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth        : channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const    : AGC time constant
*  Parameter4: UINT8 *data        :
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl301_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
//    unsigned long dwRF_Freq_KHz;
//    MxLxxxRF_BW_MHz mxl_BW;
    MxL_ERR_MSG Status;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    Mxl301_TunerConfig[tuner_id].RF_Freq_Hz = freq*1000;
    Mxl301_TunerConfig[tuner_id].BW_MHz = (MxLxxxRF_BW_MHz)bandwidth;

    switch(Mxl301_TunerConfig[tuner_id].BW_MHz)
    {
        case MxL_BW_6MHz:
            Mxl301_TunerConfig[tuner_id].IF_Freq = MxL_IF_4_5_MHZ; //MxL_IF_4_MHZ
            break;
        case MxL_BW_7MHz:
            Mxl301_TunerConfig[tuner_id].IF_Freq = MxL_IF_4_5_MHZ;
            break;
        case MxL_BW_8MHz:
            Mxl301_TunerConfig[tuner_id].IF_Freq = MxL_IF_5_MHZ;
            break;
        default:
            return ERR_FAILUE;
    }
    if((Status = MxL_Tuner_Init(&Mxl301_TunerConfig[tuner_id])) != MxL_OK )
    {
        return ERR_FAILUE;
    }

    if(  (Status = MxL_Tuner_RFTune(&Mxl301_TunerConfig[tuner_id])) != MxL_OK )
    {
        return ERR_FAILUE;
    }

    return SUCCESS;
}


INT32 tun_mxl301_powcontrol(UINT32 tuner_id, UINT8 stdby)
{
    MxL_ERR_MSG Status;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    if (stdby)
    {
        //libc_printf("start standby mode!\n");
        if ((Status = MxL_Stand_By(&Mxl301_TunerConfig[tuner_id]))!=MxL_OK)
        {
            //libc_printf("standy mode setting fail!\n");
            return ERR_FAILUE;
        }
    }
    else
    {
        //libc_printf("start wakeup mode!\n");
        if ((Status = MxL_Wake_Up(&Mxl301_TunerConfig[tuner_id]))!=MxL_OK)
        {
            //libc_printf("wakeup mode setting fail!\n");
            return ERR_FAILUE;
        }
    }

    return SUCCESS;
}

INT32 tun_mxl301_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;

    switch(cmd)
    {
        case NIM_TUNER_SET_THROUGH_MODE:
            ret = set_through_mode(tuner_id, (DEM_WRITE_READ_TUNER *)param);
            break;

        case NIM_TUNER_POWER_CONTROL:
            ret = tun_mxl301_powcontrol(tuner_id, param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}


#endif
