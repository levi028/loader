/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL5007.c
*
*    Description:    Source file of MAXLINEAR MxL5007 TUNER.
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
#include "tun_mxl5007.h"
#include <hld/nim/nim_dev.h>

#if ((SYS_TUN_MODULE == MXL5007) || (SYS_TUN_MODULE == ANY_TUNER))


//#define MXL_DEBUG

#define NIM_PRINTF(...)

#define MXL_DVBT_DEBUG  0
#if(MXL_DVBT_DEBUG)
#define NIM_MXL5007_PRINTF  soc_printf
#else
#define NIM_MXL5007_PRINTF(...)
#endif

struct COFDM_TUNER_CONFIG_EXT stMxL5007_Config[2];
static MxL5007_TunerConfigS Mxl5007_TunerConfig[2];
static BOOL bMxl_Tuner_Inited[2];
static UINT32 tuner_cnt = 0;

#ifdef MXL_DEBUG
UINT8 reg_dump[256];
#endif
/*****************************************************************************
* INT32 tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner mxl5007 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl5007_init_ISDBT_mode(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    MxL5007_ChipVersion chip_ver=0;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    MEMCPY(&stMxL5007_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));

    Mxl5007_TunerConfig[tuner_cnt].tuner_id = tuner_cnt;
    Mxl5007_TunerConfig[tuner_cnt].I2C_Addr = (MxL5007_I2CAddr)(stMxL5007_Config[tuner_cnt].c_tuner_base_addr);
    Mxl5007_TunerConfig[tuner_cnt].Mode = MxL_MODE_ISDBT;
    Mxl5007_TunerConfig[tuner_cnt].IF_Diff_Out_Level = -8;
    Mxl5007_TunerConfig[tuner_cnt].Xtal_Freq = (MxL5007_Xtal_Freq)(stMxL5007_Config[tuner_cnt].c_tuner_crystal*1000);
    Mxl5007_TunerConfig[tuner_cnt].IF_Freq = (MxL5007_IF_Freq)(stMxL5007_Config[tuner_cnt].w_tuner_if_freq*1000);
    Mxl5007_TunerConfig[tuner_cnt].IF_Spectrum = MxL_NORMAL_IF;
    Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_ENABLE;
    Mxl5007_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_7;
    if(tuner_cnt == 0)
    {
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_ENABLE;
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_0;
    }
    else
    {
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_DISABLE;
    }

    bMxl_Tuner_Inited[tuner_cnt] = FALSE;
    if( MxL_Tuner_Init(&Mxl5007_TunerConfig[tuner_cnt]) != MxL_OK )
        return ERR_FAILUE;
    bMxl_Tuner_Inited[tuner_cnt] = TRUE;

    if(tuner_id)
        *tuner_id = tuner_cnt;

    chip_ver=MxL_Check_ChipVersion(&Mxl5007_TunerConfig[tuner_cnt]);
    NIM_MXL5007_PRINTF("chip_ver=%x\n",chip_ver);

    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner mxl5007 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl5007_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    MxL5007_ChipVersion chip_ver=0;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    MEMCPY(&stMxL5007_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));

    Mxl5007_TunerConfig[tuner_cnt].tuner_id = tuner_cnt;
    Mxl5007_TunerConfig[tuner_cnt].I2C_Addr = (MxL5007_I2CAddr)(stMxL5007_Config[tuner_cnt].c_tuner_base_addr);
    Mxl5007_TunerConfig[tuner_cnt].Mode = MxL_MODE_DVBT;
    Mxl5007_TunerConfig[tuner_cnt].IF_Diff_Out_Level = -8;
    Mxl5007_TunerConfig[tuner_cnt].Xtal_Freq = (MxL5007_Xtal_Freq)(stMxL5007_Config[tuner_cnt].c_tuner_crystal*1000);
    Mxl5007_TunerConfig[tuner_cnt].IF_Freq = (MxL5007_IF_Freq)(stMxL5007_Config[tuner_cnt].w_tuner_if_freq*1000);
    Mxl5007_TunerConfig[tuner_cnt].IF_Spectrum = MxL_NORMAL_IF;
    Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_ENABLE;
    Mxl5007_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_7;
    if(tuner_cnt == 0)
    {
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_ENABLE;
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Amp = MxL_CLKOUT_AMP_0;
    }
    else
    {
        Mxl5007_TunerConfig[tuner_cnt].ClkOut_Setting = MxL_CLKOUT_DISABLE;
    }

    bMxl_Tuner_Inited[tuner_cnt] = FALSE;
    if( MxL_Tuner_Init(&Mxl5007_TunerConfig[tuner_cnt]) != MxL_OK )
        return ERR_FAILUE;
    bMxl_Tuner_Inited[tuner_cnt] = TRUE;

    if(tuner_id)
        *tuner_id = tuner_cnt;

    chip_ver=MxL_Check_ChipVersion(&Mxl5007_TunerConfig[tuner_cnt]);
    NIM_MXL5007_PRINTF("chip_ver=%x\n",chip_ver);

    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl5007_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_mxl5007_status(UINT32 tuner_id, UINT8 *lock)
{
    BOOL IfLock;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    MxL_RFSynth_Lock_Status(&Mxl5007_TunerConfig[tuner_id], &IfLock);

    //*lock = 1;
    *lock = ( IfLock == TRUE ) ? 1 : 0;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl5007_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_mxl5007_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    unsigned long dwRF_Freq_KHz;
    MxL5007_BW_MHz mxl_BW;
    MxL_ERR_MSG Status;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    dwRF_Freq_KHz = freq;
    mxl_BW = (MxL5007_BW_MHz)bandwidth;

    if(  (Status = MxL_Tuner_RFTune(&Mxl5007_TunerConfig[tuner_id], dwRF_Freq_KHz*1000, mxl_BW)) != MxL_OK )
    {
        return ERR_FAILUE;
    }

//?#define MXL_DEBUG
#ifdef MXL_DEBUG
    UINT8 i;
    for(i = 0; i < 255; i++)
    {
        //soc_printf(&Mxl5007_TunerConfig[tuner_id], i, &reg_dump[i]);
    }
#endif
    return SUCCESS;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                                                           //
//                            Tuner Functions                                   //
//                                                                           //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MxL_ERR_MSG MxL_Set_Register(MxL5007_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 RegData)
{
    UINT32 Status=0;
    UINT8 pArray[2];
    pArray[0] = RegAddr;
    pArray[1] = RegData;
    Status = MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2, myTuner->tuner_id);
    if(Status) return MxL_ERR_SET_REG;

    return MxL_OK;

}

MxL_ERR_MSG MxL_Get_Register(MxL5007_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 *RegData)
{
    if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, RegAddr, RegData, myTuner->tuner_id))
        return MxL_ERR_GET_REG;
    return MxL_OK;

}

void ASSERT_REG0()
{
    UINT8 data;
    MxL_Get_Register(&Mxl5007_TunerConfig[0], 0, &data);

    if(data != 0x3f)
    {
        ASSERT(0);
    }
}

MxL_ERR_MSG MxL_Soft_Reset(MxL5007_TunerConfigS* myTuner)
{
    UINT32 Status=0;
    UINT8 reg_reset;
    reg_reset = 0xFF;
    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, &reg_reset, 1, myTuner->tuner_id))
        return MxL_ERR_OTHERS;
    return MxL_OK;
}

MxL_ERR_MSG MxL_Loop_Through_On(MxL5007_TunerConfigS* myTuner, MxL5007_LoopThru isOn)
{
    UINT8 pArray[2];    // a array pointer that store the addr and data pairs for I2C write

    pArray[0]=0x04;
    if(isOn)
     pArray[1]= 0x01;
    else
     pArray[1]= 0x0;

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Stand_By(MxL5007_TunerConfigS* myTuner)
{
    UINT8 pArray[4];    // a array pointer that store the addr and data pairs for I2C write

    pArray[0] = 0x01;
    pArray[1] = 0x0;
    pArray[2] = 0x0F;
    pArray[3] = 0x0;

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 4 ,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Wake_Up(MxL5007_TunerConfigS* myTuner)
{
    UINT8 pArray[2];    // a array pointer that store the addr and data pairs for I2C write

    pArray[0] = 0x01;
    pArray[1] = 0x01;

    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    if(MxL_Tuner_RFTune(myTuner, myTuner->RF_Freq_Hz, myTuner->BW_MHz))
        return MxL_ERR_RFTUNE;

    return MxL_OK;
}

MxL_ERR_MSG MxL_Tuner_Init(MxL5007_TunerConfigS* myTuner)
{
    UINT8 pArray[MAX_ARRAY_SIZE];    // a array pointer that store the addr and data pairs for I2C write
    UINT32 Array_Size;                            // a integer pointer that store the number of element in above array

    //Soft reset tuner
    if(MxL_Soft_Reset(myTuner))
        return MxL_ERR_INIT;

    MxL_Delay(10);
    //perform initialization calculation
    MxL5007_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, myTuner->IF_Diff_Out_Level, (UINT32)myTuner->Xtal_Freq,
                (UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp);

    //perform I2C write here
    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
        return MxL_ERR_INIT;

    if(MxL_Loop_Through_On(myTuner, TRUE))
        return MxL_ERR_INIT;

    return MxL_OK;
}


MxL_ERR_MSG MxL_Tuner_RFTune(MxL5007_TunerConfigS* myTuner, UINT32 RF_Freq_Hz, MxL5007_BW_MHz BWMHz)
{
    UINT32 Status=0;
    UINT8 pArray[MAX_ARRAY_SIZE];    // a array pointer that store the addr and data pairs for I2C write
    UINT32 Array_Size;                            // a integer pointer that store the number of element in above array

    //Store information into struc
    myTuner->RF_Freq_Hz = RF_Freq_Hz;
    myTuner->BW_MHz = BWMHz;

    //perform Channel Change calculation
    MxL5007_RFTune(pArray,&Array_Size,RF_Freq_Hz,BWMHz);

    //perform I2C write here
    if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
        return MxL_ERR_RFTUNE;

    //wait for 3ms
    MxL_Delay(3);

    return MxL_OK;
}

MxL5007_ChipVersion MxL_Check_ChipVersion(MxL5007_TunerConfigS* myTuner)
{
    UINT8 Data;
    if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, 0xD3, &Data, myTuner->tuner_id))
        return MxL_GET_ID_FAIL;

    switch(Data)
    {
    case 0x41: return MxL_5007T_V4; break;
    default: return MxL_UNKNOWN_ID;
    }
}

MxL_ERR_MSG MxL_RFSynth_Lock_Status(MxL5007_TunerConfigS* myTuner, BOOL* isLock)
{
    UINT8 Data;
    *isLock = FALSE;
    if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, 0xD8, &Data,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

    NIM_MXL5007_PRINTF("RFSynth_Lock=%x\n",Data);//RF_SYN_RDY_RB <3:2>,  REF_SYN_RDY_RB <1:0>
    Data &= 0x0C;
    if (Data == 0x0C)
        *isLock = TRUE;  //RF Synthesizer is Lock
    return MxL_OK;
}

MxL_ERR_MSG MxL_REFSynth_Lock_Status(MxL5007_TunerConfigS* myTuner, BOOL* isLock)
{
    UINT8 Data;
    *isLock = FALSE;
    if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, 0xD8, &Data,myTuner->tuner_id))
        return MxL_ERR_OTHERS;

//    soc_printf("mxl5007 RFSynth_Lock=%x\n",Data);
    Data &= 0x03;

    if (Data == 0x03)
    {
        *isLock = TRUE;   //REF Synthesizer is Lock

    }
    return MxL_OK;
}


/******************************************************************************
**
**  Name: MxL_I2C_Write
**
**  Description:    I2C write operations
**
**  Parameters:
**                    DeviceAddr    - MxL5007 Device address
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
#if defined(BURST_PAIR_SZ)
#error " BURST_PAIR_SZ already defined!"
#else
//#define BURST_PAIR_SZ   6
#define BURST_SZ 6
#endif
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count, UINT32 tuner_id)
{

    int status = 0;
    int i,j;
    UINT8 szBuffer[BURST_SZ];
    UINT16 BurstNum, RemainLen;

//    typedef enum
//    {
//        MxL_I2C_ADDR_96 = 192 ,
//        MxL_I2C_ADDR_97 = 194 ,
//        MxL_I2C_ADDR_98 = 196 ,
//        MxL_I2C_ADDR_99 = 198
//    } MxL5007_I2CAddr ;
//
//    if(sys_ic_is_M3101()!= 1)
//    {
//        stMxL5007_Config[tuner_id].i2c_type_id=0;
//        stMxL5007_Config[tuner_id].c_tuner_base_addr=MxL_I2C_ADDR_96;
//    }

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

        status+= stMxL5007_Config[tuner_id].tuner_write(stMxL5007_Config[tuner_id].i2c_type_id, stMxL5007_Config[tuner_id].c_tuner_base_addr, szBuffer, BURST_SZ);
        if(status!=0)
        {
            NIM_MXL5007_PRINTF("MXL5007 WR1 err=%d\n",status);
            ;
        }
    }



    for ( i = 0 ; i < RemainLen ; i++ )
    {
        szBuffer[i]   = pArray[BurstNum * BURST_SZ + i ];         // addr
        //szBuffer[i+1] = pArray[BurstPairs * BURST_PAIR_SZ + i+1 ];   // value
        // libc_printf("%x  %x \n", szBuffer[i] , szBuffer[i+1]  );
    }

    status += stMxL5007_Config[tuner_id].tuner_write(stMxL5007_Config[tuner_id].i2c_type_id, stMxL5007_Config[tuner_id].c_tuner_base_addr, szBuffer, RemainLen);
    if(status!=0)
    {
            NIM_MXL5007_PRINTF("MXL5007 WR2 err=%d\n",status);
            ;
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
**                    DeviceAddr    - MxL5007 Device address
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
    UINT8 Read_Cmd[2];

    /* read step 1. accroding to mxl5007 driver API user guide. */
    Read_Cmd[0] = 0xFB;
    Read_Cmd[1] = Addr;
    stMxL5007_Config[tuner_id].tuner_write(stMxL5007_Config[tuner_id].i2c_type_id, stMxL5007_Config[tuner_id].c_tuner_base_addr, Read_Cmd, 2);

    stMxL5007_Config[tuner_id].tuner_read(stMxL5007_Config[tuner_id].i2c_type_id, stMxL5007_Config[tuner_id].c_tuner_base_addr, mData, 1);

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

INT32 tun_mxl5007_powcontrol(UINT32 tuner_id, UINT8 stdby)
{
    MxL_ERR_MSG Status;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    if (stdby)
    {
        //libc_printf("start standby mode!\n");
        if ((Status = MxL_Stand_By(&Mxl5007_TunerConfig[tuner_id]))!=MxL_OK)
        {
            //libc_printf("standy mode setting fail!\n");
            return ERR_FAILUE;
        }
    }
    else
    {
        //libc_printf("start wakeup mode!\n");
        if ((Status = MxL_Wake_Up(&Mxl5007_TunerConfig[tuner_id]))!=MxL_OK)
        {
            //libc_printf("wakeup mode setting fail!\n");
            return ERR_FAILUE;
        }
    }

    return SUCCESS;
}

INT32 tun_mxl5007_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;

#if 0
    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;
    if ( bMxl_Tuner_Inited[tuner_id] == FALSE )
    {
        if(MxL_Tuner_Init(&Mxl5007_TunerConfig[tuner_id]) != MxL_OK )
            return ERR_FAILUE;
        bMxl_Tuner_Inited[tuner_id] = TRUE;
    }
#endif

    switch(cmd)
    {
        case NIM_TUNER_POWER_CONTROL:
            tun_mxl5007_powcontrol(tuner_id, param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}


#endif
