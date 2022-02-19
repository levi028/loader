/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: nim_st6110.c
 *
 *  Description: This file contains STV6110 basic function in LLD.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.11,7      wattliu     0.1.000  Initial
 *
 ****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_stv6110.h"

#define STV6110_PRINTF(...)
//#define STV6110_PRINTF    libc_printf

#ifndef I2C_FOR_STV6110
    #define I2C_FOR_STV6110 I2C_TYPE_SCB0
#endif
#define STV6110_CRY 16

static unsigned char init_data[] = {0x00, 0x07,0x11,0xdc,0x85,0x17,0x07,0xec,0x1e};

struct QPSK_TUNER_CONFIG_EXT * stv6110_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 stv6110_tuner_cnt = 0;

#if 1//(SYS_TUN_MODULE == STV6110)
INT32 nim_stv6110_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	INT32 result = -1;
    struct QPSK_TUNER_CONFIG_EXT * stv6110_ptr = NULL;


	if ((ptrTuner_Config == NULL)||(stv6110_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    stv6110_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!stv6110_ptr)
          return ERR_FAILUE;
    MEMCPY(stv6110_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    stv6110_dev_id[stv6110_tuner_cnt] = stv6110_ptr;
    *tuner_id = stv6110_tuner_cnt;
    stv6110_tuner_cnt++;
    if ((result = i2c_write(stv6110_ptr->i2c_type_id, stv6110_ptr->c_tuner_base_addr, init_data, 9)) != SUCCESS)
    {

        return result;
    }

    return SUCCESS;
}

INT32 nim_stv6110_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	//UINT16 divider =0;
	UINT32 bs = 0;//i = 0,k = 0,j=0 ;
	UINT16 n_div = 0;
	//UINT8 temp =0,p_div =0, r_div =0, f_step =0, 
    UINT8 cf =0;
	//UINT8  m_cBR = 0, m_cBF = 0, m_cReg7 = 0, m_cReg13 =0;	
	INT32 iFbw_3db = 0;  //m_iFbw_3db = 0;	
	UINT8 data[16] = {0};//data_1 = 0,data_cp[20] = {0},data_tmp[20] = {0};
	//float fTemp = 0;
	UINT8 result = 0;

    struct QPSK_TUNER_CONFIG_EXT * stv6110_ptr = NULL;
	if((tuner_id>=stv6110_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    stv6110_ptr = stv6110_dev_id[tuner_id];
    data[0] = 0x00;

        if (freq < 950)
        freq = 950;
        else if (freq > 2160)
        freq = 2160;

    if (sym ==0)
        bs = 45000;
    else if (sym > 45000)
        bs = 45000;
    else
        bs = sym;


    data[1] = 0x07;

    data[2] = 0x14;

    n_div = freq;
    data[3] = n_div & 0xff;
    if(freq<1024)
		data[4] = (0x50 | ((n_div>>8) & 0x0f));
    else if(freq<1300)
		data[4] = (0x70 | ((n_div>>8) & 0x0f));
    else if(freq<2048)
		data[4] = (0x80 | ((n_div>>8) & 0x0f));
    else
		data[4] = (0xa0 | ((n_div>>8) & 0x0f));

    iFbw_3db = bs*135 / 200;
    iFbw_3db = iFbw_3db*130/100;
    if (bs<6500)  iFbw_3db = iFbw_3db + 3000;
    iFbw_3db = iFbw_3db + 2000; 
    iFbw_3db = iFbw_3db*108/100;
    if (iFbw_3db< 5000)   iFbw_3db =  5000;
    if (iFbw_3db>36000)   iFbw_3db = 36000;

    cf = iFbw_3db/1000 - 5;

    if(cf >0x1f)
        cf = 0x1f;
    data[5] = cf & 0xff;
    data[6] = 0x06;
    data[7] = 0x00;
    data[8] = 0x00;

    if ((result = i2c_write(stv6110_ptr->i2c_type_id, stv6110_ptr->c_tuner_base_addr, data, 7)) != SUCCESS)
    {

        return result;
    }
    return SUCCESS;
}

INT32 nim_stv6110_status(UINT32 tuner_id,UINT8 *lock)
{
	INT32 result = -1;
	UINT8 data = 0;

    if(lock == NULL)
        return RET_FAILURE;
    struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if((tuner_id>=stv6110_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }

    data = 0x05;
    tuner_dev_ptr = stv6110_dev_id[tuner_id];
    if ((result = i2c_write_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1,1)) == SUCCESS)
    {

        *lock = data &0x01;
    }

    return result;
}

INT32 nim_stv6110_get_info(void)
{
    INT32 result = 0;
	UINT8 data[16] ={0};
    UINT8 rd_data = 0;

    /*nim s3501 write. I2C through s3501 , init s3501*/
    data[0] = 0xc8;
    data[1] = 0xc0;
    i2c_write(I2C_FOR_S3501, 0x66, data, 2);

    data[0] = 0xcb;
    data[1] = 0x54;
    i2c_write(I2C_FOR_S3501, 0x66, data, 2);

    data[0] = 0xb3;
    i2c_write_read(I2C_FOR_S3501, 0x66, data, 1, 1);

    data[1] = data[0] | 0x14;
    data[0] = 0xb3;
    i2c_write(I2C_FOR_S3501, 0x66, data, 2);
       osal_delay(10000);

    /*stv6110 init*/
     i2c_write(I2C_TYPE_SCB0, 0xC0, init_data, 9);
    osal_delay(10000);

    /*stv6110 read*/
    rd_data = 0x05;
    if ((result = (i2c_write_read(I2C_TYPE_SCB0, 0xC0, &rd_data, 1 , 1)) ) == SUCCESS)
    {
        //libc_printf("%s->i2c_write_read success. result:%d, rd_data:%#x\n", __FUNCTION__, result, rd_data);
        rd_data = 0x05;
        if ((result = (i2c_write_read(I2C_TYPE_SCB0, 0xC0, &rd_data, 1 , 1)) ) == SUCCESS)
        {
            //libc_printf("%s->i2c_write_read success. result:%d, rd_data:%#x\n", __FUNCTION__, result, rd_data);
            if((rd_data&0x01) ==0)
            {
                //libc_printf("rd_data&0x01 ==0, return 0\n");
                return 0;
            }
            else
            {
                //libc_printf("rd_data&0x01 !=0, return 1\n");
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

}


#endif
