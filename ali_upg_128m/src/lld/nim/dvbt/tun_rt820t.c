/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_rt820.c
*
*    Description:    This file contains alpstdae basic function in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  20080520        Trueve Hu         Ver 0.1    Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_rt820t.h"
#include "R820T.h"

#if ((SYS_TUN_MODULE == RT820T) || (SYS_TUN_MODULE == ANY_TUNER))


#define R828_PRINTF(...)    do{}while(0) //libc_printf

static struct COFDM_TUNER_CONFIG_EXT * rt820_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 rt820_tuner_cnt = 0;
static UINT8 rt820_reg_ini[2]={0,0};
static UINT8 rt820_bw[2]={0,0};


INT32 tun_rt820_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL) || (rt820_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

    tuner_dev_ptr = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if((void*)tuner_dev_ptr == NULL)
        return ERR_FAILUE;

    MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));

    rt820_dev_id[rt820_tuner_cnt] = tuner_dev_ptr;
    *tuner_id = rt820_tuner_cnt;

    rt820_reg_ini[rt820_tuner_cnt]=0;        // status bit for initialized rt820 register
    rt820_tuner_cnt ++;

    return SUCCESS;
}

INT32 tun_rt820_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data =0;

    struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=rt820_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = rt820_dev_id[tuner_id];

    // rt820 has no PLL lock indicator, only need to wait 100ms, enough for lock
    osal_task_sleep(100);

    //result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
    *lock = 1;

    return SUCCESS;
}

INT32 tun_rt820_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{

    INT32 result;

    struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=rt820_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = rt820_dev_id[tuner_id];


    if(0==rt820_reg_ini[tuner_id])
    {

        R828_Init(); // direct setting. should be same with the tuner driver.
        //R828_GPIO(HI_SIG);
        rt820_bw[tuner_id] = 0;

        rt820_reg_ini[tuner_id]=1;
    }

    //joey, 20120110, rt810 not support 6 & 7/8 sametime.
    //now, only support 7M/8M filter.
    R828_Set_Info tmp_rt828_info;
    tmp_rt828_info.RF_KHz = freq;
    tmp_rt828_info.RT_InputMode = AIR_IN;
    tmp_rt828_info.RT_Input = SIGLE_IN;

    // joey, 2012-3-26. for RT820T 8M/7M and 6M filter change.
        if (tuner_dev_ptr->w_tuner_if_freq > 4000) // RT820 8M/ 7M mode. should be 4570.
    {
        if (8 == bandwidth) // 8M mode.
        {
            tmp_rt828_info.R828_Standard=DVB_T_8M;

            R828_SetStandard(DVB_T_8M);
            R828_SetFrequency(tmp_rt828_info, NORMAL_MODE);

        }
        else // 7M mode.
        {
            tmp_rt828_info.R828_Standard=DVB_T_7M_2;

            R828_SetStandard(DVB_T_7M_2);
            R828_SetFrequency(tmp_rt828_info, NORMAL_MODE);
        }
    }
    else    // should be 3570, for 6M mode.
    {
        tmp_rt828_info.R828_Standard=DVB_T_6M;

        R828_SetStandard(DVB_T_6M);
        R828_SetFrequency(tmp_rt828_info, NORMAL_MODE);
    }

    return SUCCESS;

}

/*
void R828_Delay_MS(int ms)
{
    osal_task_sleep(ms);
}
*/

void R828_Delay_MS(int ms)
{
    int i = 0;
    for (i=0; i<=ms; i++)
    {
        osal_delay(1000);
    }
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              :
//* Input Parameters    :     tmUnitSelect_t tUnit
//*                         UInt32 AddrSize,
//*                         UInt8* pAddr,
//*                         UInt32 WriteLen,
//*                         UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
#define BURST_SZ 13

//joey, 20111207, for RT820C/R828.
typedef struct _I2C_OP_TYPE
{
    //UINT8 DevAddr;
    //UINT8 Data[50];
    UINT8 Data[51]; // 1 more than 50.
    UINT8 Len;
}I2C_OP_TYPE;

R828_ErrCode write_i2c(I2C_OP_TYPE *I2C_Info)
{
    INT32 result = 0;
    UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

    INT32 RemainLen, BurstNum;
    INT32 i,j;

    struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    tuner_dev_ptr = rt820_dev_id[0];


    //UINT8 u8_add = I2C_Info->DevAddr;
    UINT8 u8_add = 0x34;
    UINT8 len = (I2C_Info->Len)-1;
    UINT8 *buff = &(I2C_Info->Data[1]);

    RemainLen = len % BURST_SZ; 
    if (RemainLen)
    {
        BurstNum = len / BURST_SZ;
    }
    else
    {
        BurstNum = len / BURST_SZ - 1;
        RemainLen = BURST_SZ;
    }

    for ( i = 0 ; i < BurstNum; i ++ )
    {
        for ( j = 0 ; j < BURST_SZ ; j++  )
        {
            data[j+1]   = buff[i * BURST_SZ + j ];
        }

        data[0] = I2C_Info->Data[0] + BURST_SZ*i;
        result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, BURST_SZ+1);
    }



    for ( i = 0 ; i < RemainLen ; i++ )
    {
        data[i+1]   = buff[BurstNum * BURST_SZ + i ];
    }

    data[0] = I2C_Info->Data[0] + BURST_SZ*BurstNum;

    result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, RemainLen+1);
    //libc_printf("rt820 i2c write: %d !\n", result);

    if (result == SUCCESS)
    {
        return RT_Success;
    }
    else
    {
        return RT_Fail;
    }
}

R828_ErrCode read_i2c(I2C_LEN_TYPE *I2C_Info)
{
    //UINT8 data[4]; // every time, write 14 byte..
    INT32 i,j;

    INT32 result = 0;

    struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    tuner_dev_ptr = rt820_dev_id[0];

    //UINT8 u8_add = I2C_Info->DevAddr | 0x01;
    UINT8 u8_add = 0x34 | 0x01;
    UINT8 len = I2C_Info->Len;
    UINT8 *data = I2C_Info->Data;

//joey, 20111207, for RT820C(R828 support).
    if (len > 8)
    {
        return RT_Fail;
    }

    result |= i2c_scb_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);

/*
    libc_printf("rt820 i2c read: %d !\n", result);

    for (i=0;i<len;i++)
    {
        libc_printf("rt820 data[%d]: 0x%x !\n", i, data[i]);

    }
*/

    if (result == SUCCESS)
    {
        return RT_Success;
    }
    else
    {
        return RT_Fail;
    }
}


R828_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info)
{
    I2C_OP_TYPE tmp_info;
    UINT8 i;

    for (i=0;i< I2C_Info->Len;i++)
    {
        tmp_info.Data[i+1] = I2C_Info->Data[i];
    }
    //tmp_info.DevAddr = I2C_Info->DevAddr;
    tmp_info.Data[0] = I2C_Info->RegAddr;
    tmp_info.Len = I2C_Info->Len+1;

    return write_i2c(&tmp_info);
}

UINT8 R828_Convert(UINT8 InvertNum)
{
    UINT8 ReturnNum = 0;
    UINT8 AddNum    = 0x80;
    UINT8 BitNum    = 0x01;
    UINT8 CuntNum   = 0;

    for(CuntNum = 0;CuntNum < 8;CuntNum ++)
    {
        if(BitNum & InvertNum)
            ReturnNum += AddNum;

        AddNum = AddNum >> 1;
        BitNum = BitNum << 1;
    }

    //libc_printf("rt820 convert: from 0x%2x to 0x%2x !\n", InvertNum, ReturnNum);

    return ReturnNum;
}

R828_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info)
{
    INT32 result;
    UINT8 tmp_cnt;
    I2C_Info->RegAddr  = 0x00;

    result = read_i2c(I2C_Info);

    if(RT_Success != result)
        return RT_Fail;

    for(tmp_cnt = 0;tmp_cnt < I2C_Info->Len;tmp_cnt ++)
    {
         I2C_Info->Data[tmp_cnt] = R828_Convert(I2C_Info->Data[tmp_cnt]);
    }

    return RT_Success;
}

R828_ErrCode I2C_Write(I2C_TYPE *I2C_Info)
{
    I2C_OP_TYPE tmp_info;

    //tmp_info.DevAddr = I2C_Info->DevAddr;
    tmp_info.Len       = 2;
    tmp_info.Data[0]      = I2C_Info->RegAddr;
    tmp_info.Data[1]      = I2C_Info->Data;

    return write_i2c(&tmp_info);
}

#endif
