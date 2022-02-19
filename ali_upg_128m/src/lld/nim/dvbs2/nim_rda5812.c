/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_rda5812.c
*
*    Description:    This file contains RDA-RDA5812 tuner basic function.
*    History:
*           Date                Athor                Version                  Reason
*        ========        ====            ==========    ===    =========    =================
*    1.  2009-7-25        Douglass Yan        Ver 0.1            Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_rda5812.h"

#ifndef I2C_FOR_RDA5812
#define I2C_FOR_RDA5812 I2C_TYPE_SCB0
#endif
#define RDA5812_PRINTF(...)
//#define RDA5812_PRINTF soc_printf

#define RDA5812_I2C_ADDR     0x18



#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_RDA5812, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_RDA5812, Addr, lpbData, 1)

//static unsigned char default_value[11] = {0x58, 0xc0, 0x0c, 0x00, 0x05, 0x30, 0x00, 0x04, 0xed, 0x09, 0x0a};


struct QPSK_TUNER_CONFIG_EXT * rda5812_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 rda5812_tuner_cnt = 0;

INT32 rda5812_read(UINT32 i2c_type_id, UINT8 base_addr, UINT8 reg,UINT8* buff,UINT8 len)
{
    if((buff == NULL) || (len == 0))
        return RET_FAILURE;
    buff[0] = reg;
    return i2c_write_read(i2c_type_id, base_addr, buff, 1, len);
}


INT32 rda5812_write(UINT32 i2c_type_id, UINT8 base_addr, UINT8 reg,UINT8* buff,UINT8 len)
{
	UINT8  i=0, buffer[12]={0};
    if(buff == NULL)
        return RET_FAILURE;

    if (len > 11)
    {
        return ERR_FAILUE;
    }
    buffer[0] = reg;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = buff[i];
    }

    return i2c_write(i2c_type_id, base_addr, buffer, len+1);
}

INT32 rda5812_write_reg(UINT32 i2c_type_id, UINT8 base_addr, UINT8 reg,UINT8 val,UINT8 len)
{
	UINT8  i=0, buffer[2]={0};

    if (len > 1)
    {
        return ERR_FAILUE;
    }
    buffer[0] = reg;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = val;
    }

    return i2c_write(i2c_type_id, base_addr, buffer, len+1);
}

/*****************************************************************************
* INT32 nim_rda5812_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner init operation
*
* Arguments:
*  Parameter1: UINT32* tuner_id                                : Tuner driver id
*  Parameter2: struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config        : Tuner config setting
*
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_rda5812_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//INT32 result = -1;

    struct QPSK_TUNER_CONFIG_EXT * rda5812_ptr = NULL;

	if ((ptrTuner_Config == NULL)||(rda5812_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    rda5812_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!rda5812_ptr)
          return ERR_FAILUE;
    MEMCPY(rda5812_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    rda5812_dev_id[rda5812_tuner_cnt] = rda5812_ptr;
    *tuner_id = rda5812_tuner_cnt;
    rda5812_tuner_cnt++;

	osal_delay(1000); // wait 1 ms

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x04, 0x04, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x04, 0x05, 1);

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x30, 0x60, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x31, 0x04, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x38, 0x03, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x3A, 0x06, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x3B, 0x6B, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x44, 0x55, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x7C, 0xC4, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x39, 0xBA, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x3E, 0x83, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x41, 0xA2, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x4F, 0x07, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x53, 0xAC, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x67, 0x1C, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x68, 0x81, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x69, 0x47, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x6B, 0x18, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x6C, 0xC6, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x6D, 0x4B, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x71, 0x8E, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x05, 0x10, 1);
    //rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x05, 0x63, 1);

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x15, 0x10, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x16, 0x30, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x17, 0x34, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x18, 0x3C, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x19, 0x3E, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1A, 0x3F, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1B, 0x3F, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x06, 0x1B, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1C, 0x5C, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1D, 0x50, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1E, 0x8F, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x1F, 0x4E, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x20, 0x8D, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x21, 0x53, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x22, 0x90, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x23, 0x53, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x24, 0x8D, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x25, 0x49, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x26, 0x73, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x27, 0x55, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x28, 0x80, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x29, 0x7F, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x2A, 0xB2, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x81, 0x96, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x82, 0xCF, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x83, 0xCC, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x84, 0xCE, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x85, 0x9D, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x86, 0x88, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x87, 0xA8, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x88, 0x0E, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x89, 0x00, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x8A, 0x0D, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x8B, 0x0F, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x8C, 0x1A, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x8D, 0x28, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x8E, 0x55, 1);

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x91, 0x1B, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x92, 0xFF, 1);

	osal_delay(1000); // wait 1 ms

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x07, 0x07, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x08, 0x68, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x09, 0x4b, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x0a, 0xda, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x0b, 0x0a, 1);
    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x04, 0x83, 1);

	osal_delay(10000); // wait 10 ms

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_rda5812_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 tuner_id    : Tuner driver id
*  Parameter2: UINT32 freq        : Synthesiser programmable divider
*  Parameter3: UINT8 sym        : symbol rate
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 nim_rda5812_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	UINT8 data[4]={0};
	UINT32 Rs=0, BW=0;
	//INT32 result = -1;
	unsigned long temp_rda = 0,temp_rda1 =0;
	//UINT32 i = 0;

    struct QPSK_TUNER_CONFIG_EXT * rda5812_ptr = NULL;
	if((tuner_id>=rda5812_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    rda5812_ptr = rda5812_dev_id[tuner_id];

    rda5812_write_reg(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x04, 0x81, 1);

    //rda5812_ptr->w_tuner_crystal=27000;
    temp_rda=8;
    temp_rda1=2097144;
    // freq setting, {07H[5:0], 08H[7:0],09H[7:0],0aH[7:0]} = dec2bin( f * (2^21) / f_xtal )
    temp_rda1 =(unsigned long)(freq*(temp_rda1/27));
    temp_rda = temp_rda1+ (temp_rda*freq)/27;

    data[3] =(UINT8)( temp_rda&0x0ff);
    temp_rda>>=8;
    data[2] =(UINT8)( temp_rda&0x0ff);
    temp_rda>>=8;
    data[1] =(UINT8)( temp_rda&0x0ff);
    temp_rda>>=8;
    data[0] =(UINT8)( temp_rda&0x0ff);

    if (rda5812_write(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR , 0x07, data, 4)!=SUCCESS){
        return ERR_FAILUE;
    }

    // BW setting, 0bh[5:0] (from 4M to 40M, step 1M).
    Rs = sym;
    if (Rs == 0)
        Rs = 45000;

    BW=Rs;

    unsigned char  bw_bit=0x00;
    bw_bit=(unsigned char)((BW*135/200+4000)/1000);
    bw_bit&=0x3f;

    if (rda5812_write(rda5812_ptr->i2c_type_id, RDA5812_I2C_ADDR, 0x0b, &bw_bit, 1)!=SUCCESS)
        return ERR_FAILUE;

    // set enable=1(04H,bit7), set rxon=1(04H,bit1)
    data[0] = 0xa3;    // enable AGC, 04H bit2
    if (rda5812_write(rda5812_ptr->i2c_type_id,RDA5812_I2C_ADDR,0x04,data,1)!=SUCCESS)
        return ERR_FAILUE;

    // wait 50ms to perform self calibration
    osal_delay(50);

    osal_delay(5);

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_rda5812_status(UINT32 tuner_id,UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT32 tuner_id    :Tuner driver id
*  Parameter2: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_rda5812_status(UINT32 tuner_id,UINT8 *lock)
{
    INT32 result = SUCCESS;
	//UINT8 data;

    if(lock == NULL)
        return RET_FAILURE;
	//struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if((tuner_id>=rda5812_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }

    /* Because rda5812 doesn't has this flag,return 1 directly */
    *lock = 1;
    return result;
}
