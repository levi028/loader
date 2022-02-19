/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_tdccg0x1f.c
*
*    Description:    This file contains tdcc_g0x1f basic function in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  20081028        Trueve Hu         Ver 0.1    Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_tdccg0x1f.h"


#define TDCCG0X1F_PRINTF(...)    do{}while(0) //libc_printf

//static UINT32 tun_tdccg0x1f_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * tdccg0x1f_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 tdccg0x1f_tuner_cnt = 0;


INT32 tun_tdccg0x1f_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
    /* Check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL) || (tdccg0x1f_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

    tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
    if((void*)tuner_dev_ptr == NULL)
        return ERR_FAILUE;

    MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));

    tdccg0x1f_dev_id[tdccg0x1f_tuner_cnt] = tuner_dev_ptr;
    *tuner_id = tdccg0x1f_tuner_cnt;

    tdccg0x1f_tuner_cnt++;

    return SUCCESS;
}

INT32 tun_tdccg0x1f_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data =0;

    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=tdccg0x1f_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = tdccg0x1f_dev_id[tuner_id];

    result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
    *lock = (UINT8)((data>>6)&0x01);

    return result;
}

INT32 tun_tdccg0x1f_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{

    INT32   result;
    UINT16 Npro;
    UINT32 OscFreq;
    UINT8   RefDivRatio,TunCrystal;
	//UINT8   StepFreq;
    UINT8   RS, ATP, BS;
    UINT8   data[5];


    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=tdccg0x1f_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = tdccg0x1f_dev_id[tuner_id];


    TDCCG0X1F_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);


//==========Calculate control bytes=============
    OscFreq = freq + tuner_dev_ptr->w_tuner_if_freq;
    //StepFreq = tuner_dev_ptr->c_tuner_step_freq;
    RefDivRatio = tuner_dev_ptr->c_tuner_ref_div_ratio;
    TunCrystal = tuner_dev_ptr->c_tuner_crystal;

    Npro=OscFreq * RefDivRatio / (TunCrystal * 1000);

    if (RefDivRatio == 24) //166.667
        RS = 0x00;
    else if (RefDivRatio == 28)//142.857
        RS = 0x01;
    else if (RefDivRatio == 80) //50
        RS = 0x02;
    else if (RefDivRatio == 64) // 62.5
        RS = 0x03;
    else if (RefDivRatio == 128) // 31.25
        RS = 0x04;
    else if (RefDivRatio == 50) //80
        RS = 0x05;
    else
        RS = 0x03;

    //ATP = 0x02; //Normal op
    //ATP = 0x07; //Dual AGC
    ATP = tuner_dev_ptr->c_tuner_agc_top;
#if 0
    if ((freq >= 48000) && (freq < 100000))
    {
        BS = 0x01;
        //CP = 0x02;
    }
    else if ((freq >= 100000) && (freq < 145000))
    {
        BS = 0x01;
        //CP = 0x03;
    }
    else if  ((freq >= 145000) && (freq < 300000))
    {
        BS = 0x02;
        //CP = 0x02;
    }
    else if  ((freq >= 300000) && (freq < 430000))
    {
        BS = 0x02;
        //CP = 0x03;
    }
    else if  ((freq >= 430000) && (freq < 700000))
    {
        BS = 0x08;
        //CP = 0x02;
    }
    else if  ((freq >= 700000) && (freq < 862000))
    {
        BS = 0x08;
        //CP = 0x03;
    }
    else
    {
        BS = 0x08;
        //CP = 0x02;
    }
#endif
    if ((freq >= 48000) && (freq < 148000))
    {
        BS = 0x01;
    }
    else if  ((freq >= 148000) && (freq < 430000))
    {
        BS = 0x02;
    }
    else
    {
        BS = 0x08;
    }



//==========Evaluated to control bytes=============
    data[0] = (UINT8)((Npro>>8)&0x7F);
    data[1] = (UINT8)(Npro&0xFF);
    data[2] = 0x80 | ((ATP<<3)&0x38) | (RS&0x07);
    data[3] = BS;// | ((CP<<6)&0xC0);
    data[4] = 0xc2;

//==========Write to tuner register===============
//       if(AGC_Time_Const == SLOW_TIMECST_AGC)
    /* Trueve 081128, open this delay according to Joey's experiment result */
          osal_task_sleep(50);

       result = ERR_FAILUE;

    if(_i2c_cmd==_1ST_I2C_CMD)
    {
        result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data,5);
    }

     return result;
}

