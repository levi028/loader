/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_alpstdae.c
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

#include "tun_alpstdae.h"


#define ALPSTDAE_PRINTF(...)    do{}while(0) //libc_printf

//static UINT32 tun_alpstdae_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * alpstdae_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 alpstdae_tuner_cnt = 0;


INT32 tun_alpstdae_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL) || (alpstdae_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

    tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
    if((void*)tuner_dev_ptr == NULL)
        return ERR_FAILUE;

    MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));

    alpstdae_dev_id[alpstdae_tuner_cnt] = tuner_dev_ptr;
    *tuner_id = alpstdae_tuner_cnt;

    alpstdae_tuner_cnt ++;

    return SUCCESS;
}

INT32 tun_alpstdae_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data =0;

    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=alpstdae_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = alpstdae_dev_id[tuner_id];

    result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
    *lock = (UINT8)((data>>6)&0x01);

    return result;
}

INT32 tun_alpstdae_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{

    INT32   result;
    UINT16 Npro;
    UINT32 OscFreq;
    UINT8   RefDivRatio,TunCrystal;
	//UINT8   StepFreq;
    UINT8   RS, AGST, BS_CP;
    UINT8   data[5];


    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if ((tuner_id>=alpstdae_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    tuner_dev_ptr = alpstdae_dev_id[tuner_id];


    ALPSTDAE_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);


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
    else
        RS = 0x03;

    //AGST = 0x03; //Normal op
    //AGST = 0x00; //Dual AGC
    AGST = tuner_dev_ptr->c_tuner_agc_top;

    if ((freq >= 47000) && (freq < 125000))
        BS_CP = 0xa0;
    else if ((freq >= 125000) && (freq < 366000))
         BS_CP = 0xa2;
    else if  ((freq >= 366000) && (freq < 622000))
         BS_CP = 0x68;
    else if  ((freq >= 622000) && (freq < 726000))
         BS_CP = 0xa8;
    else if  ((freq >= 726000) && (freq < 862000))
         BS_CP = 0xe8;
    else
        BS_CP = 0x68;

//==========Evaluated to control bytes=============
    data[0] = (UINT8)((Npro>>8)&0x7F);
    data[1] = (UINT8)(Npro&0xFF);
    data[2] = 0x80 | ((AGST&0x07)<<3) | (RS&0x07);
    data[3] = BS_CP;
    data[4] = 0xc6;

//==========Write to tuner register===============
       if(AGC_Time_Const == SLOW_TIMECST_AGC)
          osal_task_sleep(50);

       result = ERR_FAILUE;

    if(_i2c_cmd==_1ST_I2C_CMD)
    {
        result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data,5);
    }

     return result;
}

