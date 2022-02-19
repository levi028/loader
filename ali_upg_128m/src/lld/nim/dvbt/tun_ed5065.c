#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_ed5065.h"

#if ((SYS_TUN_MODULE == ED5065) || (SYS_TUN_MODULE == ANY_TUNER))

#define ED5065_DEBUG_FLAG  0
#if(ED5065_DEBUG_FLAG)
#define NIM_PRINTF  soc_printf
#else
#define NIM_PRINTF(...)
#endif

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;

/*****************************************************************************
* INT32 tun_ed5065_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner ed5065 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_ed5065_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    tuner_id = tuner_cnt;
    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_ed5065_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_ed5065_status(UINT8 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];
    result = ptrTuner_Config->tuner_read(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, &data, 1);
    *lock = ((data & 0x40) >> 6);
    NIM_PRINTF("TUNER LK=%d\n",*lock);

    return result;
}

/*****************************************************************************
* INT32 nim_ed5065_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_ed5065_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result=0;
    UINT32 tmp;
    UINT16 Npro;
    UINT8 CP2_0, BS5, BS3_1, ATC, R2_0;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    NIM_PRINTF("tuner_id=%d\n",tuner_id);

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    UINT32 tuner_crystal = ptrTuner_Config->c_tuner_crystal; //khz

    if(tuner_crystal >= 1000)
    {
        // tuner crystal kHz, trans to Mhz.
        tuner_crystal = tuner_crystal/1000;
    }

    NIM_PRINTF("tuner_crystal=%d \n",tuner_crystal);

    //change ref divider according to diff bandwidth.
    /*N value*/
    tmp=((freq+ptrTuner_Config->w_tuner_if_freq+(ptrTuner_Config->c_tuner_step_freq/2))*(ptrTuner_Config->c_tuner_ref_div_ratio))/tuner_crystal;

    Npro=tmp/1000;
    //NIM_PRINTF("Tuner Set Freq Npro=%d\n",Npro);
    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) ((Npro)&0xFF);


    if (ptrTuner_Config->c_tuner_ref_div_ratio == 24) //step 166.7
    {
        R2_0 = 0x02;
    }
    else
    {
        NIM_PRINTF("R1R0 cal error! \n");
        return ERR_FAILUE;

    }

    if (FAST_TIMECST_AGC == AGC_Time_Const)
        ATC = 0x01;
    else
        ATC = 0x0;

    // control byte 1.
    data[2] = (UINT8)(0x80 | 0x40 | 0x08 | (R2_0&0x07));

    /*CP */
    if (freq < 240000 )
    {
        CP2_0 = 0x01;
    }
    else if ((freq < 310000) && (freq >= 240000))
    {
        CP2_0 = 0x02;
    }
    else if ((freq < 380000) && (freq >= 310000))
    {
        CP2_0 = 0x03;
    }
    else if ((freq < 578000) && (freq >= 380000))
    {
        CP2_0 = 0x04;
    }
    else if ((freq < 650000) && (freq >= 578000))
    {
        CP2_0 = 0x05;
    }
    else if ((freq < 746000) && (freq >= 650000))
    {
        CP2_0 = 0x06;
    }
    else if (freq >= 746000)
    {
        CP2_0 = 0x07;
    }
    else
    {
        //libc_printf("CP error! \n");
        return ERR_FAILUE;
    }

    if (freq < 430000)
    {
        BS3_1 = 0x02;
    }
    else
    {
        BS3_1 = 0x04;
    }

    if (bandwidth == 8)
    {
        BS5 = 0x01;
    }
    else
    {
        BS5 = 0x00;
    }

    // control byte 2.
    data[3] = (UINT8)(((CP2_0<<5)&0xe0)  | ((BS5<<4)&0x10) | (BS3_1&0x07));

    data[4] = (UINT8)(0x80 | 0x00 | ((ATC<<3)&0x08) | (ptrTuner_Config->c_tuner_agc_top&0x07));


    if(ptrTuner_Config->tuner_write!=NULL)
    {
        if(_i2c_cmd==_1ST_I2C_CMD)
        {
            result=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 5);
            //NIM_PRINTF("Tuner_I2C_type=%d\n",ptrTuner_Config->i2c_type_id);
            NIM_PRINTF("%x,%x,%x,%x,%x\n",data[0],data[1],data[2],data[3],data[4]);
            if(result!=0)
            {
                NIM_PRINTF("Tuner_W=%d\n",result);
                ;
            }
        }
    }

    if ((AGC_Time_Const == FAST_TIMECST_AGC) && (_i2c_cmd==_1ST_I2C_CMD))
    {
            osal_task_sleep(100);
    }

    return result;


}

#endif




