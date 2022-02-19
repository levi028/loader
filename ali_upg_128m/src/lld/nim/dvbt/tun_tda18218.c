/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_tda18218.c
*
*    Description:    This file contains tda18252 basic function in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <stdio.h>
#include <hld/nim/nim_dev.h>

#include "tun_tda18218.h"

#include "tmtypes.h"
#include "dvp.h"
#include "tmhalFEtypes.h"
#include "tmbslTDA18218.h"
#include "tmbslTDA18218local.h"
#include "tmbslTDA18218StaticInstance.h"
//#include "SystemFuncURT.h"
//#include "tmbslTunerTypes.h"

#if ((SYS_TUN_MODULE == TDA18218)||(SYS_TUN_MODULE == ANY_TUNER))

#define TDA18218_DEBUG_FLAG    0
#if(TDA18218_DEBUG_FLAG)
#define TDA18218_PRINTF soc_printf
#else
#define TDA18218_PRINTF(...)
#endif

static UINT32 tun_tda18218_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * tda18218_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT8 tda18218_tuner_cnt = 0;
/*
#define MAX_I2C_DATA_LEN 16
unsigned int tun_tda18218write(UINT32  uAddress,UINT32  uSubAddress, UINT32  uNbData,UINT32* pDataBuff)
unsigned int tda18218_write(UINT8 dev_addr, UINT8 reg_addr, UINT32 len, UINT32 *data)
{
    UINT8  i, j, tmp, buffer[MAX_I2C_DATA_LEN];
    UINT32 i2c_id = dev_addr >> 8;
    INT32 err;

   // libc_printf("tda18218_write:dev_addr->0x%x, reg_addr->0x%x, %d\n", dev_addr, reg_addr, len);
    if(len > (MAX_I2C_DATA_LEN-1))
    {
        tmp = len/15;
        buffer[0] = reg_addr;

        for(i=0;i<tmp;i++)
        {
            buffer[0] += 15*i;//start register address
            for(j = 1;j < 16;j++)
            {
                buffer[j] = (UINT8)data[15*i+j-1];
            }
            err = i2c_scb_write(i2c_id, dev_addr, buffer, 16);
            if(err < 0)
                SDBBP();
        }

        buffer[0] += 15*i;//start register address
        tmp = len%15;
        for(j = 1;j < tmp;j++)
        {
            buffer[j] = (UINT8)data[15*i+j-1];
        }
        err = i2c_scb_write(i2c_id, dev_addr, buffer, tmp+1);
        if(err < 0)
            SDBBP();
    }
    else
    {
        len++;
        buffer[0] = reg_addr;
        for(j = 1;j < len;j++)
        {
            buffer[j] = (UINT8)data[j-1];
        }
        err = i2c_scb_write(i2c_id, dev_addr, buffer, len);
        if(err < 0)
            SDBBP();
    }
    return  err;
}
*/
unsigned int    tun_tda18218write    (UINT32  uAddress,UINT32  uSubAddress, UINT32  uNbData,UINT32* pDataBuff)
{
    INT32 result;
    UINT8 data[uNbData+1];

    UINT8 u8_add;
    UINT8 u8_subadd;
    INT32 len;

    u8_add       = (UINT8)(uAddress & 0xff);
    u8_subadd = (UINT8)(uSubAddress & 0xff);
    len = (INT32)uNbData;

    int i;
    for (i=0;i<uNbData;i++)
        {
        data[i+1]=(UINT8)((*(pDataBuff+i))& 0xff);
        }
    data[0]=u8_subadd;

#if 0//def GPIO_I2C_CONTROL_TUNER
    result = i2c_write(I2C_TYPE_GPIO, u8_add, data,len+1);
#else
     result = i2c_scb_write(0, u8_add, data, len+1);
    if(result!=0)
    {
        TDA18218_PRINTF("TDA18218_WR=%d\n",result);
        ;
    }
#endif

    if (result == SUCCESS)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }

    return result;
}

unsigned int    tun_tda18218read(UINT32  uAddress, UINT32  uSubAddress,UINT32  uNbData,UINT32* pDataBuff)
{
    INT32 result;
    UINT8 data[59];

    UINT8 u8_add;
    UINT8 u8_subadd;
    INT32 len;

    u8_add       = (UINT8)(uAddress & 0xff);
    u8_subadd = (UINT8)(uSubAddress & 0xff);
    len = (INT32)uNbData+(INT32)u8_subadd;

#if 0//def GPIO_I2C_CONTROL_TUNER
    result = i2c_read(I2C_TYPE_GPIO,u8_add,&data,len);
#else
    result = i2c_scb_read(0, u8_add, &data, len);
    if(result!=0)
    {
        TDA18218_PRINTF("TDA18218_RD=%d\n",result);
        ;
    }
#endif

    Int i;
    for (i=0;i<uNbData;i++)
        {
        *(pDataBuff+i)=data[uSubAddress+i];
        }
    //TDA18252_PRINTF("%s : result %d !!\n", __FUNCTION__,result);
    if (result == SUCCESS)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;

}



INT32 tun_tda18218_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL) || (tda18218_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

    tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
    if((void*)tuner_dev_ptr == NULL)
        return ERR_FAILUE;

    MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));

    tda18218_dev_id[tda18218_tuner_cnt] = tuner_dev_ptr;
    //tuner_id = tda18218_tuner_cnt;

    tda18218_tuner_cnt ++;

    TDA18218_PRINTF("%s start !\n", __FUNCTION__);

    return SUCCESS;
}

INT32 tun_tda18218_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    tmErrorCode_t err;
    UINT32 data[59];


    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=tda18218_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = tda18218_dev_id[tuner_id];

    //err=tmbslTDA18218GetConfig(tuner_id,tmTDA18218STATUSBYTE,data);

    tun_tda18218read(0xc0,0,2,data);        //added for debug
    TDA18218_PRINTF("reg[0]=%2x\n",data[0]);
    TDA18218_PRINTF("reg[1]=%2x\n",data[1]);
    //err = 1;                                //added for debug
/*
    int i;
    libc_printf("Start pintf tuner!!!!!\n");
    for (i=0;i<59;i++)
        TDA18218_PRINTF("reg%d=%2x\n",i,data[i]);
    libc_printf("End pintf tuner!!!!!\n");
*/
    TDA18218_PRINTF("LO Lock Status: %d !!\n", (((UINT8)(data[1]&0xff))&0x40)>>6);
    //TDA18218_PRINTF("%s ::AGC2 Status: %d !!\n", __FUNCTION__,((UINT8)(data[17]&0xff))&0x01);

    //err|=tmbslTDA18252GetConfig(tuner_id,tmTDA18252TUNEROK,data);

    //TDA18218_PRINTF("%s ::Tuner status: %d !!\n", __FUNCTION__,(UINT8)(data[0]&0xff));

    *lock=(((UINT8)(data[1]&0xff))&0x40)>>6;
    TDA18218_PRINTF("TDA18218_LK=%d\n",*lock);
    /*if (err !=0)
        {
        result= ERR_FAILED;
        libc_printf("failed to read tuner\n");
        }
    else
        {
        result = SUCCESS;
        }
    */

    return SUCCESS;
}

INT32 get_t_period()
{
return 1;
}

INT32 tun_tda18218_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const, UINT8 *data, UINT8 _i2c_cmd)
{

    INT32   result;
    tmErrorCode_t err;
    static INT8 tun_status=0;

    if (0==tun_status)
    {
    static tmbslTuParam_t Param;
    static tmhalFEBslSystemFunc_t systemFunc;


    systemFunc.SY_GetTickTime        =    osal_get_tick;
    systemFunc.SY_GetTickPeriod        =    get_t_period;
    //systemFunc.SY_TimerStart        =    NULL;
    //systemFunc.SY_TimerWait        =    NULL;
    systemFunc.SY_GetCommErrors    =    NULL;
    systemFunc.SY_Write            =    tun_tda18218write;
    systemFunc.SY_WriteBit            =    NULL;//tun_tda18252writebit;
    systemFunc.SY_Read            =    tun_tda18218read;
    systemFunc.SY_SetDeviceAdd        =    NULL;
    systemFunc.SY_SetErrorEnable    =    NULL;
    systemFunc.SY_SetUrt            =    NULL;
    systemFunc.SY_GetMatLog        =    NULL;

    Param.uHwAddress    =    0xc0;
    Param.systemFunc    =    systemFunc;


    err    =    tmbslTDA18218Init(tuner_id,Param);
    err|=    tmbslTDA18218SetConfig(tuner_id,tmTDA18218FEINIT,0);

    //Set the power State in mode normal
    tmTDA18218PowerState_t powerState =tmTDA18218PowerNormalMode;
    err|=    tmbslTDA18218SetPowerState(tuner_id, powerState);

    tun_status =1;
    }


    if (8==bandwidth)    // 8M channel
    {
        err|=    tmbslTDA18218SetStandardMode(tuner_id,tmTDA18218DigitalChannel_8MHz);
        libc_printf("Tuner select 8M\n");
    }
    else if (7==bandwidth)    // 7M channel
    {
        err|= tmbslTDA18218SetStandardMode(tuner_id,tmTDA18218DigitalChannel_7MHz);
    }
    else            // 6M channel
    {
        err|= tmbslTDA18218SetStandardMode(tuner_id,tmTDA18218DigitalChannel_6MHz);
    }
    //Set Freq to tuner
    unsigned long    FreqHz    =    freq*1000;
    err|=    tmbslTDA18218SetRf(tuner_id,FreqHz);
    osal_task_sleep(200);
    //TDA18218_PRINTF("%s Freq=%d Hz::ERR: %d !!\n", __FUNCTION__,FreqHz,err);

    if(freq==722000)
    {
        osal_task_sleep(1);    // add for debug
    }

    if (err !=0)
    {
        result= ERR_FAILED;
    }
    else
    {
        result = SUCCESS;
    }

    return result;

}

INT32 tun_tda18218_powcontrol(UINT32 tuner_id,UINT8 stdby)
{

    INT32   result;
    tmErrorCode_t err;

    if (stdby)
    {
        //libc_printf("start standby mode!\n");
        err = tmbslTDA18218SetConfig(tuner_id, tmTDA18218POWERSTATE, tmTDA18218PowerStandbyMode);
    }
    else
    {
        //libc_printf("start wakeup mode!\n");
        err =    tmbslTDA18218SetPowerState(tuner_id, tmTDA18218PowerNormalMode);
    }

    if (err !=0)
    {
        //libc_printf("tda18218 set power failed!\n");
        result= ERR_FAILED;
    }
    else
    {
        result = SUCCESS;
    }

    return result;

}
INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;

    switch(cmd)
    {
        case NIM_TUNER_POWER_CONTROL:
            tun_tda18218_powcontrol(tuner_id, param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}
#endif

