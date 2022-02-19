/*****************************************************************************
*    Copyright (C)2017 Gospell Corporation. All Rights Reserved.
*
*    File:    tun_atbm253.c
*
*    Description:    APIS file for atbm tuner.
*    History:
*      Date               Athor                Version            Reason
*  =========           =========          =============	     =============
*	1.  2019/09/20	    Liang Ming  	      Ver 0.1		      Create file.
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim_dev.h>
#include "ATBM253Api.h"

#define ATBM_TUNER_DEBUG		0

#if (ATBM_TUNER_DEBUG > 0)
#define ATBM_TUNER_ERROR		libc_printf
#else
#define ATBM_TUNER_ERROR(...)
#endif

struct QAM_TUNER_CONFIG_EXT * atbm253_tuner_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
UINT32 atbm253_tuner_cnt = 0;
UINT8 atbm253_reg_ini[2]={0,0};
extern INT32 atbm253_mutex_create(void);
//extern void gos_gpio_set_tuner_reset(void);

INT32 tun_atbm253_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
//	UINT8 data;
//	INT32 result = 0;
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
//	ATBM253InitConfig_t InitConfig;
//	ATBM253_ERROR_e ret = ATBM253_NO_ERROR;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (atbm253_tuner_cnt >= MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
	{
		ATBM_TUNER_ERROR("%s()----Malloc tuner dev failed\n",__FUNCTION__);
		return ERR_FAILUE;
	}		
	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	//gos_gpio_set_tuner_reset();
	atbm253_tuner_dev_id[atbm253_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = atbm253_tuner_cnt;
	atbm253_reg_ini[atbm253_tuner_cnt]=0;		
	atbm253_tuner_cnt++;    
	atbm253_mutex_create();
	
	return SUCCESS;
}

INT32 tun_atbm253_status(UINT32 tuner_id, UINT8 *lock)
{
//	INT32 result;
//	BOOL lockstatus = 0;
//	ATBM253_ERROR_e ret = ATBM253_NO_ERROR;
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=atbm253_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
	{
		ATBM_TUNER_ERROR("%s()----Tuner id is large\n",__FUNCTION__);
		return ERR_FAILUE;
	}
	tuner_dev_ptr = atbm253_tuner_dev_id[tuner_id];
//	ATBM253GetLockStatus(tuner_id,&lockstatus);
//	*lock = lockstatus;
	*lock = 1;
	ATBM_TUNER_ERROR("%s()----Tuner lock status:%d \n",__FUNCTION__,*lock);
	return SUCCESS;
}

INT32 tun_atbm253_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{	
//	INT32 result;
	UINT32 FreqKHz = freq; 
	UINT32 BandWidthKHz =  8000;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	ATBM253InitConfig_t InitConfig;
	ATBM253_ERROR_e ret = ATBM253_NO_ERROR;
	
	if ((tuner_id>=atbm253_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = atbm253_tuner_dev_id[tuner_id];
	if(0==atbm253_reg_ini[tuner_id])
	{
		ATBM_TUNER_ERROR("%s()----ATBM253 INIT START\n",__FUNCTION__);
		/*Get the default configuration from ATBM253 SDK. */
		if(ATBM253_NO_ERROR != ATBM253DefaultCfgGet(&InitConfig))
		{
			return ERR_FAILUE;
		}
		/*Initialize with custumer parameters.*/
		ret = ATBM253Init(tuner_id,&InitConfig);
		if(ATBM253_NO_ERROR != ret)
		{
			ATBM_TUNER_ERROR("%s()----ATBM253 INIT FAILED  ret %d\n",__FUNCTION__,ret);
			return ERR_FAILUE;
		}
		ATBM_TUNER_ERROR("%s()----ATBM253 INIT OK\n",__FUNCTION__);
		atbm253_reg_ini[tuner_id]=1;
	}
	ATBM_TUNER_ERROR("%s()----TIPS: freqKhz:%d \n",__FUNCTION__,freq);
	ret = ATBM253ChannelTune(tuner_id,ATBM253_SIGNAL_MODE_DVBC,FreqKHz,BandWidthKHz,ATBM253_SPECTRUM_NORMAL);
	if(ATBM253_NO_ERROR != ret)
	{
		ATBM_TUNER_ERROR("%s()----TIPS:Configure Tuner failed \n",__FUNCTION__);
		return ERR_FAILUE;
	}
	return SUCCESS;
}
INT32 tun_atbm253_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
	INT32 ret = SUCCESS;
//	ATBM_S32 ttt = 0;
	switch(cmd)
	{
//		case NIM_TUNER_GET_AGC:
//		case NIM_DRIVER_GET_AGC:
//			ATBM253GetRSSI((ATBM_U32)tuner_id, (ATBM_S32 *)&ttt);
//			libc_printf("ATBM253GetRSSI---[%d][%d]\n",tuner_id,ttt);
//			break;
		default:
			ret = ERR_FAILUE;
			break;
	}
	return ret;
}

