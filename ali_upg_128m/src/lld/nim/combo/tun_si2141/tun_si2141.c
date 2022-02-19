/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:    tun_si2141.c
*
*    Description:    This file contains alpstdae basic function in LLD. 
*    History:
*           Date            Author        Version          Reason
*	    ===================================================
*	1.  20150723	    Joey.Gao	Ver 1.0	       Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include <hld/nim/nim_dev.h>

#include "Si2141_44_24_L2_API.h"
#include "tun_si2141.h"

#if ((SYS_TUN_MODULE == SI2141) || (SYS_TUN_MODULE == ANY_TUNER))

#if 0
#define SI2141_PRINTF  libc_printf
#else
#define SI2141_PRINTF(...)	do{}while(0)
#endif

//static struct COFDM_TUNER_CONFIG_EXT * si2141_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};

//for si2141 porting part usage.
typedef struct _si2141_config_t
{
	UINT16  		cTuner_Crystal;
	UINT8  		cTuner_Base_Addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */	
	UINT16 		wTuner_IF_Freq;
	UINT32 		i2c_type_id;

 	UINT8   invert_spectrum;
	UINT8   modulation;
	
}SI2141_CONFIG_T;

static SI2141_CONFIG_T  si2141_config[MAX_TUNER_SUPPORT_NUM] = {{0,0,0,0,0,0}};

//for si2141 core driver usage.
static L1_Si2141_44_24_Context  si2141_context[MAX_TUNER_SUPPORT_NUM] = {};
static UINT32 si2141_tuner_cnt = 0;
static UINT8 si2141_reg_ini[MAX_TUNER_SUPPORT_NUM]={0};
static UINT8 si2141_rssi_rdy[MAX_TUNER_SUPPORT_NUM]={0}; //0: not ready, 1: ready..

INT32 tun_si2141_init_isdbt(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (si2141_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	si2141_config[si2141_tuner_cnt].cTuner_Base_Addr=(UINT8)(ptrTuner_Config->c_tuner_base_addr);
	si2141_config[si2141_tuner_cnt].cTuner_Crystal=(UINT16)ptrTuner_Config->c_tuner_crystal;
	si2141_config[si2141_tuner_cnt].i2c_type_id=(UINT32)ptrTuner_Config->i2c_type_id;
	si2141_config[si2141_tuner_cnt].wTuner_IF_Freq=(UINT16)ptrTuner_Config->w_tuner_if_freq;

	si2141_config[si2141_tuner_cnt].invert_spectrum= Si2141_44_24_DTV_MODE_PROP_INVERT_SPECTRUM_NORMAL;
	si2141_config[si2141_tuner_cnt].modulation= Si2141_44_24_DTV_MODE_PROP_MODULATION_ISDBT;

	si2141_reg_ini[si2141_tuner_cnt]=0;		// status bit for initialized si2141 register

	*tuner_id = si2141_tuner_cnt;

	si2141_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_si2141_init_dvbt(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (si2141_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	si2141_config[si2141_tuner_cnt].cTuner_Base_Addr=(UINT8)(ptrTuner_Config->c_tuner_base_addr);
	si2141_config[si2141_tuner_cnt].cTuner_Crystal=(UINT16)ptrTuner_Config->c_tuner_crystal;
	si2141_config[si2141_tuner_cnt].i2c_type_id=(UINT32)ptrTuner_Config->i2c_type_id;
	si2141_config[si2141_tuner_cnt].wTuner_IF_Freq=(UINT16)ptrTuner_Config->w_tuner_if_freq;

	si2141_config[si2141_tuner_cnt].invert_spectrum= Si2141_44_24_DTV_MODE_PROP_INVERT_SPECTRUM_NORMAL;
	si2141_config[si2141_tuner_cnt].modulation= Si2141_44_24_DTV_MODE_PROP_MODULATION_DVBT;

	si2141_reg_ini[si2141_tuner_cnt]=0;		// status bit for initialized si2141 register

	*tuner_id = si2141_tuner_cnt;

	si2141_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_si2141_status(UINT32 tuner_id, UINT8 *lock)
{
	if ((tuner_id>=si2141_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	osal_task_sleep(100);

	//for tuner check lock of si2141, similar to the get rssi. but we don't do so comlexity here.
	*lock = 1;

	return SUCCESS;
}

INT32 tun_si2141_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)	
{	
	SI2141_CONFIG_T * si2141_config_ptr = NULL;
	L1_Si2141_44_24_Context * si2141_context_ptr = NULL;
	UINT8 tmp_bw = 0;

	if ((tuner_id>=si2141_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	si2141_config_ptr = &(si2141_config[tuner_id]);
	si2141_context_ptr = &(si2141_context[tuner_id]);
	
	if(0==si2141_reg_ini[tuner_id])
	{
		//Like software level initial, memory object initial.
		Si2141_44_24_L1_API_Init(si2141_context_ptr, (int)(si2141_config_ptr->cTuner_Base_Addr));

		//To set correct I2C connection type.
		CONNECTION_TYPE con_type = CUSTOMER;
		L0_Connect(si2141_context_ptr->i2c, con_type);
			
		//SW default part.
		//Direct sw default.prepare for software initial of parameter part. Not same of the HW(register) default part
		si2141_context_ptr->cmd->power_up.clock_mode =  Si2141_44_24_POWER_UP_CMD_CLOCK_MODE_XTAL;
		si2141_context_ptr->cmd->power_up.en_xout    =  Si2141_44_24_POWER_UP_CMD_EN_XOUT_DIS_XOUT;
		si2141_context_ptr->cmd->config_clocks.clock_mode = Si2141_44_24_CONFIG_CLOCKS_CMD_CLOCK_MODE_XTAL;

		//Indirect sw default part. just writing the above setting by function call.
		if (NO_Si2141_44_24_ERROR != Si2141_44_24_Init(si2141_context_ptr))
		{
			//error message.
			return ERR_FAILURE;
		}

		//To re-write the above default, we can change the new sw default here.
		//Update IF frequency according to upper layer setting.
		si2141_context_ptr->prop->dtv_lif_freq.offset = si2141_config_ptr->wTuner_IF_Freq; // typical is 5000.

		if (NO_Si2141_44_24_ERROR != Si2141_44_24_L1_SetProperty2(si2141_context_ptr, Si2141_44_24_DTV_LIF_FREQ_PROP))
		{
			//error message.
			return ERR_FAILURE;
		}
	 
		
		si2141_reg_ini[tuner_id]=1;
	}

	switch (bandwidth)
	{
		case 6:
			tmp_bw = Si2141_44_24_DTV_MODE_PROP_BW_BW_6MHZ;
			break;

		case 7:
			tmp_bw = Si2141_44_24_DTV_MODE_PROP_BW_BW_7MHZ;
			break;

		case 8:
			tmp_bw = Si2141_44_24_DTV_MODE_PROP_BW_BW_8MHZ;
			break;

		default:
			tmp_bw = Si2141_44_24_DTV_MODE_PROP_BW_BW_8MHZ;
			SI2141_PRINTF("Error! bandwidth=%d\n",bandwidth);   
			break;
	}
		
	Si2141_44_24_DTVTune (si2141_context_ptr, freq*1000, tmp_bw, si2141_config_ptr->modulation, si2141_config_ptr->invert_spectrum);

	if (0 == si2141_rssi_rdy[tuner_id])
	{
		si2141_rssi_rdy[tuner_id] = 1;
	}

	return SUCCESS;

}

INT32 tun_si2141_get_rf_level(UINT32 tuner_id, UINT16 *rf_level)// return level in dbuV.
{
	INT16 tmp_rssi = 0;
	UINT16 tmp_rf_value = 0;
	L1_Si2141_44_24_Context * si2141_context_ptr = NULL;

	static UINT8 tmp_last_rf_value = 0;
	
	if ((tuner_id>=si2141_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	if (0 == si2141_rssi_rdy[tuner_id]) // Tuner not tuned yet, rssi is meanless.
	{
		*rf_level = 50; // just set to middle value. means around 50 dbuV, meanless.
		return SUCCESS;
	}

	si2141_context_ptr = &(si2141_context[tuner_id]);

	if (NO_Si2141_44_24_ERROR == Si2141_44_24_L1_TUNER_STATUS (si2141_context_ptr)) // success. 
	{
		tmp_rssi = si2141_context_ptr->rsp->tuner_status.rssi;

		tmp_rssi = tmp_rssi + 107;

		if (tmp_rssi > 255)
		{
			tmp_rf_value = 255;
		}
		else if (tmp_rssi < 0)
		{
			tmp_rf_value = 0;
		}
		else
		{
			tmp_rf_value = tmp_rssi;
		}
		
		tmp_last_rf_value = tmp_rf_value; //update the last success value.
		
	}
	else
	{
		//something wrong. the rssi value abnormal.
		tmp_rf_value = tmp_last_rf_value;
	}

	*rf_level = tmp_rf_value;
	
	return SUCCESS;
}

INT32 tun_si2141_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
	
    switch(cmd)
    {
/*    
        case NIM_TUNER_POWER_CONTROL:
		if (TRUE == (*param))
		{
			R840_LoopThrough_Type tmp_si2141_lt;
			tmp_si2141_lt = LT_OFF;
			
            		R840_Standby(tmp_si2141_lt);
		}
            break;
*/

        case NIM_TUNER_GET_AGC:
		return tun_si2141_get_rf_level(tuner_id, (UINT16 *)param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}

#if 1
//Tuner driver I2C requirement implementation.

/************************************************************************************************************************
  system_wait function
  Use:        current system wait function
              Used to wait for time_ms milliseconds while doing nothing
  Parameter:  time_ms the wait duration in milliseconds
  Returns:    The current system time in milliseconds
  Porting:    Needs to use the final system call for time retrieval
************************************************************************************************************************/
int system_wait(int time_ms)
{
	osal_task_sleep(time_ms);
	return time_ms;
}

/************************************************************************************************************************
  system_time function
  Use:        current system time retrieval function
              Used to retrieve the current system time in milliseconds
  Returns:    The current system time in milliseconds
  Porting:    Needs to use the final system call
************************************************************************************************************************/
int system_time(void)        
{
	UINT32 tmp_ms_time;
	tmp_ms_time = osal_get_tick();
	return (int)tmp_ms_time;
}

int tun_si2141_read_i2c(L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes, unsigned char *pucDataBuffer)
{
	INT32 result = 0;
	UINT8 i = 0;
	UINT32 tmp_i2c_type_id;

	//this part for multi tuner support, to reconganize the i2c_type_id automatically.
	for(i=0;i<si2141_tuner_cnt;i++)
	{
		if (si2141_context[i].i2c == i2c) //memory address match.
		{
			break;
		}
	}

	if (i<si2141_tuner_cnt) // means address match success.
	{
		tmp_i2c_type_id = si2141_config[i].i2c_type_id;
	}
	else
	{
		return 0; // not match the i2c obj, something error happen.
	}
	
	result = i2c_read(tmp_i2c_type_id, i2c->address, pucDataBuffer, iNbBytes);

	if (SUCCESS == result)
	{
		return iNbBytes;
	}
	else
	{
		return 0;
	}
}

int tun_si2141_write_i2c(L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes, unsigned char *pucDataBuffer)
{
	INT32 result = 0;
	UINT8 i = 0;
	UINT32 tmp_i2c_type_id;

	//this part for multi tuner support, to reconganize the i2c_type_id automatically.
	for(i=0;i<si2141_tuner_cnt;i++)
	{
		if (si2141_context[i].i2c == i2c) //memory address match.
		{
			break;
		}
	}

	if (i<si2141_tuner_cnt) // means address match success.
	{
		tmp_i2c_type_id = si2141_config[i].i2c_type_id;
	}
	else
	{
		return 0; // not match the i2c obj, something error happen.
	}
	
	result = i2c_write(tmp_i2c_type_id, i2c->address, pucDataBuffer, iNbBytes);

	if (SUCCESS == result)
	{
		return iNbBytes;
	}
	else
	{
		return 0;
	}
}

#endif

#endif
