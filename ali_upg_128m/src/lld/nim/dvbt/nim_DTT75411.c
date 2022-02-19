/*****************************************************************************
*    Copyright (C)2008 Aov Corporation. All Rights Reserved.
*
*    File:    DTT75411_Tuner
*
*    Description:    DTT75411_Tuner driver.
*    History:
*           Date            Athor        Version          Reason
*        =======   =====      ========    =========
*    1.    2008         Steven        Ver 0.1        Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_DTT75411.h"

#if ((SYS_TUN_MODULE == DTT75411) || (SYS_TUN_MODULE == ANY_TUNER))
#define NIM_PRINTF(...)
#define ALI_PRINTF(...)



struct COFDM_TUNER_CONFIG_EXT stDTT75411_Config[2];
static UINT8 dtt75411_tuner_cnt = 0;


INT32 nim_DTT75411_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    MEMCPY(&stDTT75411_Config[dtt75411_tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    dtt75411_tuner_cnt++;
    return SUCCESS;
}

INT32 nim_DTT75411_status(UINT32 tuner_id,UINT8 *lock)
{
    INT32 result;
    UINT8 data;
    stDTT75411_Config[tuner_id].i2c_type_id=0;
    //result = stDTT75411_Config[tuner_id].Tuner_Read(stDTT75411_Config[tuner_id].i2c_type_id,stDTT75411_Config[tuner_id].c_tuner_base_addr, &data, 1);
    result = stDTT75411_Config[tuner_id].tuner_read(1,stDTT75411_Config[tuner_id].c_tuner_base_addr, &data, 1);
    *lock = ((data & 0x40) >> 6);
    NIM_PRINTF(" data=%x,tuner lk=%d\n",data,*lock);

    return result;
}


INT32 nim_DTT75411_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth, UINT8 AGC_Time_Const, UINT8 *data, UINT8 _i2c_cmd)
{
    UINT32 tmp;
    UINT16 Npro;
    INT32 result,err;
tuner_id=0;
NIM_PRINTF("tuner freq!=%d\n",freq);
 // DTT75411's control byte [Begin]
    //tmp=((freq+stDTT75411_Config.wTuner_IF_Freq+(stDTT75411_Config.cTuner_Step_Freq/2))*((stDTT75411_Config.cTuner_Crystal*1000)/stDTT75411_Config.cTuner_Step_Freq))/stDTT75411_Config.cTuner_Crystal;
    freq += stDTT75411_Config[tuner_id].w_tuner_if_freq; //Change freq from input channel center frequency to oscillator frequency
    tmp=((freq+(stDTT75411_Config[tuner_id].c_tuner_step_freq/2))*((stDTT75411_Config[tuner_id].c_tuner_crystal*1000)/stDTT75411_Config[tuner_id].c_tuner_step_freq))/stDTT75411_Config[tuner_id].c_tuner_crystal;
    //Chales@20080512: modify DTT75411's control byte [End]

    Npro=tmp/1000;

    if ((83000 <= freq) && (freq <=905000))
    {
        data[0] = (UINT8) ((Npro>>8)&0x7F);
        data[1] = (UINT8) ((Npro)&0xFF);
        data[2] = 0x88;
    }
    else
    {
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
    }

      // control byte
    if ((83000 <= freq) && (freq < 121000))
        data[3] = 0x01;//0x11;
        else if ((121000 <= freq) && (freq < 141000))
        data[3] = 0x41;//0x51;
       else if ((141000 <= freq) && (freq <= 166000))
        data[3] = 0x81;//0x91;
       else if ((166000 <= freq) && (freq <= 182000))
        data[3] = 0xC1;//0xD1;
       else if ((182000 <= freq) && (freq <= 286000))
        data[3] = 0x02;//0x14;
       else if ((286000 <= freq) && (freq <= 386000))
        data[3] = 0x42;//0x54;
       else if ((386000 <= freq) && (freq <= 446000))
        data[3] = 0x82;//0x94;
       else if ((446000 <= freq) && (freq <= 466000))
        data[3] = 0xC2;//0xD4;
       else if ((466000 <= freq) && (freq <= 506000))
        data[3] = 0x08;
       else if ((506000 <= freq) && (freq <= 761000))
        data[3] = 0x48;
       else if ((761000 <= freq) && (freq <= 846000))
        data[3] = 0x88;
       else if ((846000 <= freq) && (freq <= 905000))
        data[3] = 0xC8;
        else
            return ERR_FAILUE ;

    if(stDTT75411_Config[tuner_id].tuner_write!=NULL)
    {
		if( ( _i2c_cmd ==_1ST_I2C_CMD) || (_i2c_cmd==_2ND_I2C_CMD) )
		{
		    if(AGC_Time_Const == FAST_TIMECST_AGC)
		        data[4] = 0xE3;
		    else
		    {
		       data[4] = 0xC3;
		     osal_task_sleep(100);
		    }

			INT32 err = 0;
			stDTT75411_Config[tuner_id].i2c_type_id=0;
		    err = stDTT75411_Config[tuner_id].tuner_write(stDTT75411_Config[tuner_id].i2c_type_id, stDTT75411_Config[tuner_id].c_tuner_base_addr, data, 5);
		    ALI_PRINTF("%x,%x,%x,%x,%x\n",data[0],data[1],data[2],data[3],data[4]);
		    if(err!=0)
		    {
		        ALI_PRINTF("tuner wr fail!!%d\n",err);
		    }
		}
		else
		{
		    return ERR_FAILUE ;
		}
     }

     return SUCCESS;
}

#endif

