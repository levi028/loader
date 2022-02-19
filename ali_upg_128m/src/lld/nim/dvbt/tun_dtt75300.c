#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_dtt75300.h"

#if ((SYS_TUN_MODULE == DTT75300) || (SYS_TUN_MODULE == ANY_TUNER)) //for dvb-t tuner 75300/01.


#define NIM_PRINTF(...)

static struct COFDM_TUNER_CONFIG_EXT stDTT75300_Config[2];
static UINT8 tuner_dtt75300_cnt = 0;


/*****************************************************************************
* INT32 tun_DTT75300_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner DTT75300 Initialization
*
* Arguments:
* Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_dtt75300_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    MEMCPY(&stDTT75300_Config[tuner_dtt75300_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    tuner_dtt75300_cnt++;
    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_DTT75300_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_dtt75300_status(UINT32 tuner_id,UINT8 *lock)
{
    INT32 result;
    UINT8 data;

    result = stDTT75300_Config[tuner_id].tuner_read(I2C_FOR_TUNER,stDTT75300_Config[tuner_id].c_tuner_base_addr, &data, 1);
    *lock = (UINT8)((data>>6)&0x01);

    return result;
}

/*****************************************************************************
* INT32 tun_DTT75300_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_dtt75300_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result;
    UINT16 Npro;    
    UINT8 C1_0, R4_0, BS1_0, P3_0, LO1_0, ATC;

    /*data byte*/
    Npro=(UINT16)(((freq+stDTT75300_Config[tuner_id].w_tuner_if_freq+(stDTT75300_Config[tuner_id].c_tuner_step_freq/2))*((stDTT75300_Config[tuner_id].c_tuner_crystal*1000)/stDTT75300_Config[tuner_id].c_tuner_step_freq))/(stDTT75300_Config[tuner_id].c_tuner_crystal*1000.0) + 0.5);

    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) (Npro&0xFF);

    /*control byte*/
    if ((freq <= 236000) || (freq>536000 && (freq<536000)))
        C1_0 = 0x01;
    else if (((236000 < freq) && (freq <= 263000)) ||((536000 < freq) && (freq <= 811000)))
        C1_0 = 0x02;
    else if ((811000 < freq) && (freq <= 898000))
        C1_0 = 0x03;
    else
        C1_0 = 0x01;

    R4_0 = 0x13; //this should just for divider=24, 166.6K step.

    data[2] = (UINT8)(0x80 | ((C1_0<<5)&0x60) | (R4_0&0x1f));

    if (bandwidth == 8)
    {
        BS1_0 = 0x02;
    }
    else
    {
        BS1_0 = 0x01;
    }

    if (freq <470000)
    {
        P3_0 = 0x09;
    }
    else
    {
        P3_0 = 0x00;
    }

    data[3] = (UINT8)(0x30 | ((BS1_0<<6)&0xc0) | (P3_0&0x0f));

    LO1_0 = 0x01;

    if (FAST_TIMECST_AGC == AGC_Time_Const)
        ATC = 0x00;
    else
        ATC = 0x01;

    data[4] = (UINT8)(((LO1_0<<6)&0xc0) | ((ATC<<5)&0x20) | 0x11);//IFE = 1, AT2_0 = 1;

    data[5] = 0x90;

    if (SLOW_TIMECST_AGC == AGC_Time_Const)
        osal_task_sleep(100); // when change ATC to normal state, wait for about 100ms for channel aquistion.

    if(stDTT75300_Config[tuner_id].tuner_write!=NULL)
    {
        if(_i2c_cmd == _1ST_I2C_CMD)
        {
            stDTT75300_Config[tuner_id].tuner_write(stDTT75300_Config[tuner_id].i2c_type_id,stDTT75300_Config[tuner_id].c_tuner_base_addr, data, 6);
        }
        else
        {
            return ERR_FAILUE ;
        }
    }
    return SUCCESS;
}
#endif
