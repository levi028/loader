/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_hz6306.c
*
*    Description:    This file contains SHARP BS2S7HZ6306 tuner basic function.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  2006-7-18           Jie Wu         Ver 0.1        Create file.
*     2.  2007-7-03          Dietel Wang                      Standardize interface
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_hz6306.h"

#ifndef I2C_FOR_HZ6303
#define I2C_FOR_HZ6303 I2C_TYPE_SCB0
#endif
#define HZ6306_PRINTF(...)



#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_HZ6303, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_HZ6303, Addr, lpbData, 1)


static unsigned char init_data1[] = { 0x44, 0x7e, 0xe1, 0x42 };
static unsigned char init_data2[] = { 0xe5 };
static unsigned char init_data3[] = { 0xfd, 0x0d };

struct QPSK_TUNER_CONFIG_EXT * hz6306_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 hz6306_tuner_cnt = 0;

INT32 nim_hz6306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	INT32 result = -1;
    struct QPSK_TUNER_CONFIG_EXT * hz6306_ptr = NULL;

	if ((ptrTuner_Config == NULL)||(hz6306_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    hz6306_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!hz6306_ptr)
          return ERR_FAILUE;
    MEMCPY(hz6306_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    hz6306_dev_id[hz6306_tuner_cnt] = hz6306_ptr;
    *tuner_id = hz6306_tuner_cnt;
    hz6306_tuner_cnt++;
    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, init_data1, sizeof(init_data1))) != SUCCESS)

    {
        HZ6306_PRINTF("nim_hz6306_init: I2C write error\n");
        return result;
    }

    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, init_data2, sizeof(init_data2))) != SUCCESS)
        return result;


    osal_delay(10000);

    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, init_data3, sizeof(init_data3))) != SUCCESS)
        return result;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_hz6306_control(UINT32 freq, UINT8 sym, UINT8 cp)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 sym        : symbol rate
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_hz6306_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	UINT8 data[4]={0};
	UINT16 Npro = 0,tmp = 0;
	UINT32 Rs = 0, BW = 0;
	UINT8 Nswa = 0;
    UINT8 LPF = 15;
    UINT8 BA = 1;
    UINT8 DIV = 0;
	INT32 result = -1;

    struct QPSK_TUNER_CONFIG_EXT * hz6306_ptr = NULL;
	if((tuner_id>=hz6306_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    hz6306_ptr = hz6306_dev_id[tuner_id];
#if 0
    UINT32 ratio = 135;
#endif


    Rs = sym;
    if (Rs == 0)
        Rs = 45000;
#if 1
    BW = Rs*135/200;
    BW = BW*130/100;
    if (Rs < 6500)
        BW = BW + 3000;
    BW = BW + 2000;
    BW = BW*108/100;
#else
    if (ratio == 0)
        BW = 34000;
    else
        BW = Rs * ratio / 100;
#endif
    if (BW < 10000)
        BW = 10000;
    if (BW > 34000)
        BW = 34000;

    if (BW <= 10000)
        LPF = 3;
    else if (BW <= 12000)
        LPF = 4;
    else if (BW <= 14000)
        LPF = 5;
    else if (BW <= 16000)
        LPF = 6;
    else if (BW <= 18000)
        LPF = 7;
    else if (BW <= 20000)
        LPF = 8;
    else if (BW <= 22000)
        LPF = 9;
    else if (BW <= 24000)
        LPF = 10;
    else if (BW <= 26000)
        LPF = 11;
    else if (BW <= 28000)
        LPF = 12;
    else if (BW <= 30000)
        LPF = 13;
    else if (BW <= 32000)
        LPF = 14;
    else
        LPF = 15;



    if (freq <= 1154)
        DIV = 1;
    else
        DIV = 0;

    if (freq <= 986)
        BA = 5;
    else if (freq <= 1073)
        BA = 6;
    else if (freq <= 1154)
        BA = 7;
    else if (freq <= 1291)
        BA = 1;
    else if (freq <= 1447)
        BA = 2;
    else if (freq <= 1615)
        BA = 3;
    else if (freq <= 1791)
        BA= 4;
    else if (freq <= 1972)
        BA= 5;
    else //if (freq <= 2150)
        BA = 6;


    tmp = freq * 1000 * 8 / REF_OSC_FREQ;
    Nswa = tmp % 32;
    Npro = tmp / 32;
    HZ6306_PRINTF ("nim_hz6306_control: Npro = %d, Nswa = %d\n", Npro, Nswa);

    data[0] = (UINT8) ((Npro >> 3) & 0x1F);

    data[0] = data[0] | 0x40;
    data[1] = Nswa | (((UINT8)Npro & 0x07) << 5);
    data[2] = 0xE1;
    data[3] = (BA<<5) | (DIV<<1);
    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, data+0, 4)) != SUCCESS)
    {
        HZ6306_PRINTF("nim_hz6306_control: I2C write error\n");
        return result;
    }

    data[2] = 0xE5;
    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, data+2, 1)) != SUCCESS)
        return result;
    osal_delay(10000);

    data[2] |= ((LPF & 0x01) << 4) | ((LPF & 0x02) << 2);
    data[3] |= ((LPF & 0x04) << 1) | ((LPF & 0x08) >> 1);
    if ((result = i2c_write(hz6306_ptr->i2c_type_id, hz6306_ptr->c_tuner_base_addr, data+2, 2)) != SUCCESS)
        return result;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_hz6306_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_hz6306_status(UINT32 tuner_id,UINT8 *lock)
{
	INT32 result = -1;
	UINT8 data = 0;
    if(lock == NULL)
        return RET_FAILURE;

    struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if((tuner_id>=hz6306_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    tuner_dev_ptr = hz6306_dev_id[tuner_id];
    if ((result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1)) == SUCCESS)
    {
        HZ6306_PRINTF("nim_hz6306_status: data = 0x%x\n", data);
        *lock = ((data & 0x40) >> 6);
    }

    return result;
}
