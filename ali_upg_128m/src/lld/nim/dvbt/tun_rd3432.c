#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>

#include "tun_rd3432.h"

#if ((SYS_TUN_MODULE == RADIO3432) || (SYS_TUN_MODULE == ANY_TUNER))

#define NIM_PRINTF(...)
//#define NIM_PRINTF libc_printf

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT8 tuner_cnt = 0;

/*****************************************************************************
* INT32 tun_rd3432_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner rd3432 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    //tuner_id = tuner_cnt;
    tuner_cnt++;
    NIM_PRINTF("3432 init\n");

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_rd3432_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 *lock)
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

    NIM_PRINTF("result: %d, *lock = %d !\n", result, *lock);
    return result;
}

/*****************************************************************************
* INT32 nim_rd3432_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result,i;
    UINT32 tmp;
    UINT16 Npro;
    UINT8 CP, SP4_3, SP2_0 = 0, T2_0, ATC, R1_0;
    UINT8 ATP2_0,RS2_0;
    UINT8 CP2,CP1_0,AISL,BS4_1;

     struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if(_i2c_cmd==_2ND_I2C_CMD)
        return SUCCESS;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    UINT32 tuner_crystal = ptrTuner_Config->c_tuner_crystal; //khz
    libc_printf("tuner_crystal assert =%d\n",tuner_crystal);
    if(tuner_crystal >= 1000)
    {
        // tuner crystal kHz, trans to Mhz.
        tuner_crystal = tuner_crystal/1000;
    }

    //tuner_crystal=4;
    /*libc_printf("tuner_id=%d\n",tuner_id);
    libc_printf("IF=%d\n",ptrTuner_Config->wTuner_IF_Freq);
    libc_printf("Step_Freq=%d\n",ptrTuner_Config->cTuner_Step_Freq);
    libc_printf("Ref_DivRatio=%d\n",ptrTuner_Config->cTuner_Ref_DivRatio);
    libc_printf("tuner_crystal=%d\n",tuner_crystal);
    libc_printf("freq=%d\n",freq);
    */
    /*N value*/
    //tmp=(UINT32)((freq+ptrTuner_Config->wTuner_IF_Freq+(ptrTuner_Config->cTuner_Step_Freq/2))*1000.0/ptrTuner_Config->cTuner_Step_Freq);
    tmp=((freq+ptrTuner_Config->w_tuner_if_freq+(ptrTuner_Config->c_tuner_step_freq/2))*(ptrTuner_Config->c_tuner_ref_div_ratio))/tuner_crystal;

    Npro=tmp/1000;
    //libc_printf("Tuner Set Freq Npro=%x\n",Npro);

    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) ((Npro)&0xFF);

    /*ATP2_0*/
    ATP2_0=(ptrTuner_Config->c_tuner_agc_top<<3)&0x38;
    /*RS2_0*/

    if (ptrTuner_Config->c_tuner_ref_div_ratio == 24) //step 166.7
    {
        RS2_0 = 0x00;
    }
    else if (ptrTuner_Config->c_tuner_ref_div_ratio == 64) //step 62.5
    {
        RS2_0 = 0x03;
    }
    else
    {
        //libc_printf("R1R0 cal error! \n");
        return ERR_FAILUE;

    }

    data[2]=(UINT8)(0x80 | ATP2_0 | RS2_0);


    /*charge pump*/
    CP2=0;
    CP1_0=0x03;        /*default is 600mA*/



    /*RFAGC input sel*/
    AISL = 1;        // 1: mixer output, 0: if amplifer

    /*band selsect*/
    if (bandwidth<=7)
        BS4_1=0x02;
    else
        BS4_1=0x00;

    data[3]=(UINT8)(((CP1_0<<6)&&0xc0) |((AISL<<5)&&0x20)|BS4_1);
    data[5]=data[3];



    if (FAST_TIMECST_AGC == AGC_Time_Const)
        ATC = 0x01;
    else
        ATC = 0x00;

    data[4]=(UINT8)(0xC0 | ((ATC<<5)&&0x20) | 0x10 | ((CP2<<1)&&0x02) | 0x01);


    /*radio default setting*/
    data[2]=0x80;
    if (freq<=470000)
        data[3]=0x92;
    else
        data[3]=0xd8;

    data[4]=0xd1;
    data[5]=data[3];

    for (i=0;i<5;i++)
        {
        libc_printf("data%d=%x\n",i,data[i]);
        }


    if(ptrTuner_Config->tuner_write!=NULL)
    {

        osal_task_sleep(10);
        if(_i2c_cmd==_1ST_I2C_CMD)
        {
            result=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 5);
            if(result!=0)
                NIM_PRINTF("i2c write result is %d\n",result);
        }
        else if(_i2c_cmd==_2ND_I2C_CMD)
        {
            return SUCCESS;
            data[2]=data[4];
            data[3]=data[5];
            result=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 4);
            if(result!=0)
                NIM_PRINTF("_2nd_i2c  write result is %d\n",result);
        }
        else
        {
            return ERR_FAILUE ;
        }
    }

    if ((AGC_Time_Const == FAST_TIMECST_AGC) && (_i2c_cmd==_2ND_I2C_CMD))
    {
        //just wait once 100ms in ATC = 1 state.
        osal_task_sleep(100);
    }

    return SUCCESS;


}

#endif


