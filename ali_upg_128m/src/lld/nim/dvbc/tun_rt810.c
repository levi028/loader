/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_rt810.c
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

#include "tun_rt810.h"
#include "RT810.h"


#define RT810_PRINTF(...)    do{}while(0) //libc_printf

static struct QAM_TUNER_CONFIG_EXT * rt810_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 rt810_tuner_cnt = 0;
static UINT8 rt810_reg_ini[2]={0,0};
static UINT8 rt810_bw[2]={0,0};


INT32 tun_rt810_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL) || (rt810_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

    //reopen process;
    if(ptrTuner_Config->c_tuner_reopen & 0xff)
    {
        tuner_dev_ptr=rt810_dev_id[*tuner_id];
        tuner_dev_ptr->c_tuner_freq_param=(ptrTuner_Config->c_tuner_reopen & 0xFF00) >> 8;
        return SUCCESS;
    }

    tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
    if((void*)tuner_dev_ptr == NULL)
        return ERR_FAILUE;

    MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));

    rt810_dev_id[rt810_tuner_cnt] = tuner_dev_ptr;
    *tuner_id = rt810_tuner_cnt;

    rt810_reg_ini[rt810_tuner_cnt]=0;        // status bit for initialized rt810 register
    rt810_tuner_cnt ++;

    return SUCCESS;
}

INT32 tun_rt810_status(UINT32 tuner_id, UINT8 *lock)
{

    if ((tuner_id>=rt810_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    // rt810 has no PLL lock indicator, only need to wait 100ms, enough for lock
    osal_task_sleep(100);

    //result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
    *lock = 1;

    return SUCCESS;
}

/*
static UINT8 DVBC_Arry[50] = {0x03, 0x5B, 0x04, 0x92, 0x05, 0xC4, 0x06, 0x14, 0x07, 0x34,
                           //0x03        0x04        0x05        0x06        0x07
                       0x08, 0x42, 0x09, 0x40, 0x0A, 0x47, 0x0B, 0x00, 0x0C, 0x90,
                           //0x08        0x09        0x0A        0x0B        0x0C
                       0x0D, 0x83, 0x0E, 0x63, 0x0F, 0x88, 0x10, 0x95, 0x11, 0x03,
                           //0x0D        0x0E        0x0F        0x10        0x11
                       0x12, 0x40, 0x13, 0x00, 0x14, 0xCB, 0x15, 0xEB, 0x16, 0x51,
                           //0x12        0x13        0x14        0x15        0x16
                       0x17, 0x0F, 0x18, 0xC5, 0x19, 0x42, 0x1A, 0x90, 0x1B, 0x00};
                           //0x17        0x18        0x19        0x1A        0x1B
*/

typedef enum _BW_Type
{
    BW_6M = 0,
    BW_7M,
    BW_8M
}BW_Type;


INT32 tun_rt810_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{

    INT32 result = 0;
    UINT8 bandwidth = 0;
    UINT8 v_freq=DVB_C;


    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=rt810_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = rt810_dev_id[tuner_id];
    if(tuner_dev_ptr->c_tuner_freq_param ==0x00)
        v_freq=0x09;
    else
        v_freq=tuner_dev_ptr->c_tuner_freq_param;


    if(0==rt810_reg_ini[tuner_id])
    {

        RT810_Init(); // direct setting. should be same with the tuner driver.
        //RT810_GPIO(HI_SIG);
        rt810_bw[tuner_id] = 0;

        rt810_reg_ini[tuner_id]=1;
    }

    bandwidth = 6;

    if (bandwidth != rt810_bw[tuner_id]) // do standard setting.
    {
        result = RT810_Standard(v_freq);

        rt810_bw[tuner_id] = bandwidth;
    }

    RT810_Set_Info tun_info;
    tun_info.InputMode = AIR_IN;
    tun_info.RT_Input = SIGLE_IN;
    tun_info.RT810_Standard = v_freq;
    tun_info.RF_KHz = freq;

    result |= RT810_SetFrequency(tun_info);

    return SUCCESS;

}

/*
void RT810_Delay_MS(int ms)
{
    osal_task_sleep(ms);
}
*/

void RT810_Delay_MS(int ms)
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
#define BURST_SZ 14

typedef struct _I2C_OP_TYPE
{
    UINT8 DevAddr;
    UINT8 Data[50];
    UINT8 Len;
}I2C_OP_TYPE;

RT810_ErrCode write_i2c(I2C_OP_TYPE *I2C_Info)
{
    INT32 result = 0;
    UINT8 data[BURST_SZ]; // every time, write 14 byte..

    INT32 RemainLen, BurstNum;
    INT32 i,j;

    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    tuner_dev_ptr = rt810_dev_id[0];


    //UINT8 u8_add = I2C_Info->DevAddr;
    UINT8 u8_add = 0x34;
    UINT8 len = I2C_Info->Len;
    UINT8 *buff = I2C_Info->Data;

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
            data[j]   = buff[i * BURST_SZ + j ];
        }

        result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, BURST_SZ);
    }



    for ( i = 0 ; i < RemainLen ; i++ )
    {
        data[i]   = buff[BurstNum * BURST_SZ + i ];
    }
    result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, RemainLen);
    //libc_printf("rt810 i2c write: %d !\n", result);

    if (result == SUCCESS)
    {
        return RT_Success;
    }
    else
    {
        return RT_Fail;
    }
}

RT810_ErrCode read_i2c(I2C_LEN_TYPE *I2C_Info)
{
    //UINT8 data[4]; // every time, write 14 byte..
    INT32 result = 0;

    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    tuner_dev_ptr = rt810_dev_id[0];

    //UINT8 u8_add = I2C_Info->DevAddr | 0x01;
    UINT8 u8_add = 0x34 | 0x01;
    UINT8 len = I2C_Info->Len;
    UINT8 *data = I2C_Info->Data;

    if (len > 4)
    {
        return RT_Fail;
    }

    result |= i2c_scb_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);

    //libc_printf("rt810 i2c read: %d !\n", result);

    if (result == SUCCESS)
    {
        return RT_Success;
    }
    else
    {
        return RT_Fail;
    }
}


RT810_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info)
{
    I2C_OP_TYPE tmp_info;
    UINT8 i;

    for (i=0;i< I2C_Info->Len;i++)
    {
        tmp_info.Data[i+1] = I2C_Info->Data[i];
    }
    tmp_info.DevAddr = I2C_Info->DevAddr;
    tmp_info.Len = I2C_Info->Len+1;

    return write_i2c(&tmp_info);
}

UINT8 RT810_Convert(UINT8 InvertNum)
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

    //libc_printf("rt810 convert: from 0x%2x to 0x%2x !\n", InvertNum, ReturnNum);

    return ReturnNum;
}

RT810_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info)
{
    INT32 result;
    UINT8 tmp_cnt;
    I2C_Info->RegAddr  = 0x00;

    result = read_i2c(I2C_Info);

    if(RT_Success != result)
        return RT_Fail;

    for(tmp_cnt = 0;tmp_cnt < I2C_Info->Len;tmp_cnt ++)
    {
         I2C_Info->Data[tmp_cnt] = RT810_Convert(I2C_Info->Data[tmp_cnt]);
    }

    return RT_Success;
}

RT810_ErrCode I2C_Write(I2C_TYPE *I2C_Info)
{
    I2C_OP_TYPE tmp_info;

    tmp_info.DevAddr = I2C_Info->DevAddr;
    tmp_info.Len       = 2;
    tmp_info.Data[0]      = I2C_Info->RegAddr;
    tmp_info.Data[1]      = I2C_Info->Data;

    return write_i2c(&tmp_info);
}

