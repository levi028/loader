#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>    //51117-01Angus
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_DTF8570.h"

#if ((SYS_TUN_MODULE == DTF8570) || (SYS_TUN_MODULE == ANY_TUNER))

#define NIM_PRINTF(...)// soc_printf //      // (...)

struct COFDM_TUNER_CONFIG_EXT stDTF8570_Config[2];    //51117-01Angus
static UINT8 dtf8570_tuner_cnt = 0;

/*****************************************************************************
* INT32 nim_DTF8570_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner DTF8570 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_DTF8570_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    MEMCPY(&stDTF8570_Config[dtf8570_tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    dtf8570_tuner_cnt++;
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_DTF8570_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 nim_DTF8570_status(UINT32 tuner_id,UINT8 *lock)
{
    INT32 result;
    UINT8 data;

    result = stDTF8570_Config[tuner_id].tuner_read(stDTF8570_Config[tuner_id].i2c_type_id,stDTF8570_Config[tuner_id].c_tuner_base_addr, &data, 1);
    *lock = ((data & 0x40) >> 6);

   //     DVBT_PRINTF(" DTF8570 lock=%x\n ",*lock );
    return result;
}

/*****************************************************************************
* INT32 nim_DTF8570_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 nim_DTF8570_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    UINT32 tmp;
    UINT16 Npro;
    INT32 result,err;

     /*data byte*/
    tmp=((freq+stDTF8570_Config[tuner_id].w_tuner_if_freq+(stDTF8570_Config[tuner_id].c_tuner_step_freq/2))*((stDTF8570_Config[tuner_id].c_tuner_crystal*1000)/stDTF8570_Config[tuner_id].c_tuner_step_freq))/stDTF8570_Config[tuner_id].c_tuner_crystal;

        Npro=tmp/1000;
    NIM_PRINTF("Tuner Set Freq Npro=%d\n",Npro);
    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) ((Npro)&0xFF);

     /*control byte*/
    if ((150000 <= freq) && (freq < 510000))       data[2] =0xb4;
    else if ((510000 <= freq) && (freq <= 735000))  data[2] =0xbc;////0xf4;//
    else if ((735000 <freq) && (freq <= 835000))    data[2]=0xf4;
    else if ((835000 < freq) && (freq <= 890000))   data[2]= 0xfc;
    else
    {
                   if(stDTF8570_Config[tuner_id].tuner_write!=NULL)    //51226-01Angus
                   {
                       data[2] =0xb4;
                       data[3] = 0x01;
                       stDTF8570_Config[tuner_id].tuner_write(stDTF8570_Config[tuner_id].i2c_type_id,stDTF8570_Config[tuner_id].c_tuner_base_addr, data, 4);
                   }
                   NIM_PRINTF("FAIL INPUT FREQ. Npro MSb=0x%x , Npro LSb=0x%x\n",data[0],data[1]);
                   return ERR_FAILUE ;
    }

        /*band selection*/
             if ((40000 <= freq) && (freq < 150000))    data[3] = 0x01;
        else if ((150000 <= freq) && (freq < 455000))   data[3] = 0x02;
       else if ((455000 <= freq) && (freq <= 865000))  data[3] = 0x0C;
        else return ERR_FAILUE ;

       /*mode select*/
      if ((150000 <= freq) && (freq < 510000))       data[4] =0x9c;
    else if ((510000 <= freq) && (freq <= 735000))  data[4] =0x9c;////0xf4;//
    else if ((735000 <freq) && (freq <= 835000))    data[4]=0xdc;
    else if ((835000 < freq) && (freq <= 890000))   data[4]= 0xdc;
    else return ERR_FAILUE ;

    /*auxiliary byte*/
    data[5] =data[5]&0xf0;
    data[5] =( AGC_Time_Const<<7)|(stDTF8570_Config[tuner_id].c_tuner_agc_top << 4);

    if(stDTF8570_Config[tuner_id].tuner_write!=NULL)
    {
        if(_i2c_cmd == _1ST_I2C_CMD)
        {
            err=stDTF8570_Config[tuner_id].tuner_write(stDTF8570_Config[tuner_id].i2c_type_id,stDTF8570_Config[tuner_id].c_tuner_base_addr, data, 4);
            NIM_PRINTF("1st tuner_err=%x\n",err);

        }
        else if(_i2c_cmd == _2ND_I2C_CMD)
        {
            data[2]=data[4];
            data[3]=data[5];
            err=stDTF8570_Config[tuner_id].tuner_write(stDTF8570_Config[tuner_id].i2c_type_id,stDTF8570_Config[tuner_id].c_tuner_base_addr, data, 4);
            NIM_PRINTF("2nd tuner_err=%x\n",err);
        }
        else
        {
            return ERR_FAILUE ;
        }
    }

    if ((AGC_Time_Const == FAST_TIMECST_AGC) && (_i2c_cmd == _2ND_I2C_CMD))
    {
        //just wait once 100ms in ATC = 1 state.
        osal_task_sleep(100);
    }

    return SUCCESS;


}

#endif
