/*****************************************************************************
*    Copyright (C)2016 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r850.c
*
*    Description:    This file contains tuner basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20160106		Joey.Gao		 Ver 1.0	Create file.
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

#include "tun_r850.h"
#include "R850.h"

#if ((SYS_TUN_MODULE == R850) || (SYS_TUN_MODULE == ANY_TUNER))

extern UINT16 R850_Xtal;	
#if 1
#define R850_PRINTF(...)	do{}while(0) 
#else
#define R850_PRINTF(...)	libc_printf
#endif
static struct COFDM_TUNER_CONFIG_EXT * r850_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 r850_tuner_cnt = 0;
static UINT8 r850_reg_ini[2]={0,0};
static UINT32 tmp_rf_freq[2] = {0,0};

//joey, 20150116, for R850 ISDB mode support.
static UINT8 r850_work_mode[2]={0,0}; //0: DVBT2(DVB-T), 1: ISDB-T.
static R850_Standard_Type r850_cur_std[2]={0,0}; // used for rssi cal.
//joey, 20150313, for R850 RSSI get affect the tuner performance.
static UINT8 r850_rssi_rdy[2]={0,0}; //0: ready, 1: not ready..

INT32 tun_r850_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (r850_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));	
	
	r850_dev_id[r850_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = r850_tuner_cnt;

	r850_reg_ini[r850_tuner_cnt]=0;		// status bit for initialized r850 register
	r850_work_mode[r850_tuner_cnt] = 0; // initial as "DVB-T2/T combo" mode.
	r850_tuner_cnt ++;

	return SUCCESS;
}

//joey, 20150116, for R850 ISDB mode support.
INT32 tun_r850_isdbt_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (r850_tuner_cnt >= MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));	
	
	r850_dev_id[r850_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = r850_tuner_cnt;

	r850_reg_ini[r850_tuner_cnt]=0;		// status bit for initialized r850 register
	r850_work_mode[r850_tuner_cnt] = 1; // initial as "ISDB-T" mode.
	
	r850_tuner_cnt ++;

	return SUCCESS;
}
//leo,20160317,for R850 DVBC mode support
INT32 tun_r850_dvbc_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (r850_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;
	tuner_dev_ptr->c_tuner_base_addr = (UINT8)ptrTuner_Config->c_tuner_base_addr;
	tuner_dev_ptr->c_tuner_crystal= ptrTuner_Config->c_tuner_crystal;
	tuner_dev_ptr->i2c_type_id =  (UINT32)ptrTuner_Config->i2c_type_id;
	tuner_dev_ptr->w_tuner_if_freq = (UINT16)ptrTuner_Config->w_tuner_if_freq;
	R850_Xtal = ((UINT16)(ptrTuner_Config->c_tuner_crystal))*1000;
	//MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	r850_dev_id[r850_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = r850_tuner_cnt;

	r850_reg_ini[r850_tuner_cnt]=0;		// status bit for initialized r850 register
	r850_work_mode[r850_tuner_cnt] = 2; // initial as "dvbc" mode.
	
	r850_tuner_cnt ++;

	return SUCCESS;
}
INT32 tun_r850_status(UINT32 tuner_id, UINT8 *lock)
{

	if ((tuner_id >= r850_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	// r850 has no PLL lock indicator, only need to wait 100ms, enough for lock
	osal_task_sleep(100);

	//result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = 1;

	return SUCCESS;
}
INT32 tun_r850_dvbc_control(UINT32 tuner_id, UINT32 freq, UINT32 bandwidth, UINT8 agc_time_const, UINT8 _i2c_cmd)
{	

	if ((tuner_id>=r850_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	//joey, 20120110, rt810 not support 6 & 7/8 sametime.
	//now, only support 7M/8M filter.
	R850_Set_Info tmp_r850_info;

	//joey, 20150116, for R850 ISDB mode support.
	if(2 == r850_work_mode[tuner_id])//DVBC mode
	{
		tmp_r850_info.R850_Standard = R850_DVB_C_8M;
	}
	else
	{
		R850_PRINTF("Error! not dvbc work mode\n");
		return ERR_FAILUE;
	}
	
	if ((0==r850_reg_ini[tuner_id]) || (tmp_r850_info.R850_Standard != r850_cur_std[tuner_id]))
	{
		r850_rssi_rdy[tuner_id] = 0; // any time re-init the tuner, the RSSI function maybe illigal.
		R850_IMR_done_flag[tuner_id] = FALSE;
		R850_Initial_done_flag[tuner_id] = FALSE;
			
		R850_Init(tuner_id, tmp_r850_info.R850_Standard); // direct setting. should be same with the tuner driver.
		//R850_GPIO(HI_SIG);

		r850_reg_ini[tuner_id]=1;
	}	
	
	r850_cur_std[tuner_id] = tmp_r850_info.R850_Standard;
	
	tmp_r850_info.RF_KHz = freq;
	tmp_r850_info.R850_LT = LT_OFF;
	tmp_r850_info.R850_ClkOutMode = CLK_OUT_OFF;
		
	R850_SetPllData(tuner_id, tmp_r850_info);//set the tv format and frenquency point

	tmp_rf_freq[tuner_id] = freq;

//joey, 20150313, for R850 RSSI get affect the tuner performance.
	if (0 == r850_rssi_rdy[tuner_id])
	{
		r850_rssi_rdy[tuner_id] = 1;
	}

	return SUCCESS;

}	
INT32 tun_r850_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)	
{	

	if ((tuner_id >= r850_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	//joey, 20120110, rt810 not support 6 & 7/8 sametime.
	//now, only support 7M/8M filter.
	R850_Set_Info tmp_r850_info;

	//joey, 20150116, for R850 ISDB mode support.
	if (1 == r850_work_mode[tuner_id]) // ISDB-T mode.
	{
		switch (bandwidth)
		{
			case 6:
				tmp_r850_info.R850_Standard =R850_ISDB_T_IF_5M;
				break;

			case 7:
				tmp_r850_info.R850_Standard =R850_DVB_T_7M_IF_5M;
				break;

			case 8:
				tmp_r850_info.R850_Standard =R850_DVB_T_8M_IF_5M;
				break;

			default:
				tmp_r850_info.R850_Standard =R850_DVB_T_8M_IF_5M;
				R850_PRINTF("Error! bandwidth=%d\n",bandwidth);   
				break;
		}
	}
	else if(0 == r850_work_mode[tuner_id])
	{
		switch (bandwidth)
		{
			case 6:
				tmp_r850_info.R850_Standard =R850_DVB_T2_6M_IF_5M;
				break;

			case 7:
				tmp_r850_info.R850_Standard =R850_DVB_T2_7M_IF_5M;
				break;

			case 8:
				tmp_r850_info.R850_Standard =R850_DVB_T2_8M_IF_5M;
				break;

			default:
				tmp_r850_info.R850_Standard =R850_DVB_T2_8M_IF_5M;
				R850_PRINTF("Error! bandwidth=%d\n",bandwidth);   
				break;
		}
	}
	else
	{
		R850_PRINTF("Error! workmode is not dvbt/isdbt\n");
		return ERR_FAILUE;
	}

	if ((0 == r850_reg_ini[tuner_id]) || (tmp_r850_info.R850_Standard != r850_cur_std[tuner_id]))
	{
		r850_rssi_rdy[tuner_id] = 0; // any time re-init the tuner, the RSSI function maybe illigal.
		R850_IMR_done_flag[tuner_id] = FALSE;
		R850_Initial_done_flag[tuner_id] = FALSE;
			
		R850_Init(tuner_id, tmp_r850_info.R850_Standard); // direct setting. should be same with the tuner driver.
		//R850_GPIO(HI_SIG);

		r850_reg_ini[tuner_id]=1;
	}	
	
	r850_cur_std[tuner_id] = tmp_r850_info.R850_Standard;
	
	tmp_r850_info.RF_KHz = freq;
	tmp_r850_info.R850_LT = LT_OFF;
	tmp_r850_info.R850_ClkOutMode = CLK_OUT_OFF;
		
	R850_SetPllData(tuner_id, tmp_r850_info);

	tmp_rf_freq[tuner_id] = freq;

//joey, 20150313, for R850 RSSI get affect the tuner performance.
	if (0 == r850_rssi_rdy[tuner_id])
	{
		r850_rssi_rdy[tuner_id] = 1;
	}

	return SUCCESS;

}

INT32 tun_r850_get_rf_level(UINT32 tuner_id, UINT16 *rf_level)// return level in dbuV.
{

	//UINT16 RF_gain, IF_gain;
	//INT32 RF_RSSI, IF_RSSI, 
	INT32 RSSI;

	//R850_RF_Gain_Info R850_rf_gain;

//joey, 20150313, for R850 RSSI get affect the tuner performance.
	if (1 == r850_rssi_rdy[tuner_id])
	{
		R850_GetTotalRssi(tuner_id, tmp_rf_freq[tuner_id], r850_cur_std[tuner_id], &RSSI);
		//joey, 20160107, for R850 ISDB mode support.  and add offset adjustment.
		RSSI = (RSSI + 107) + 7;
	}
	else
	{
		RSSI = 50;
	}
/*	
	{
		//read RF_gain from tuner
		R850_GetRfGain(&R850_rf_gain);
		RF_gain = R850_rf_gain.RF_gain_comb;

		//read IF_gain from demod	
		IF_gain = *rf_level;

		//RSSI
		if(tmp_rf_freq[tuner_id] < 200000)   //RF<200MHz
		{
			if(RF_gain < 608)
			{
				RF_RSSI = (-104*RF_gain - 2752)/1000;
				IF_RSSI = 0;
			}
			else
			{
				RF_RSSI = 0;
				IF_RSSI = (-96*IF_gain - 43927)/1000;
			}
		}
		else if(tmp_rf_freq[tuner_id] < 350000)   //RF<350MHz
		{
			if(RF_gain < 608)
			{
				RF_RSSI = (-104*RF_gain + 0)/1000;
				IF_RSSI = 0;
			}
			else
			{
				RF_RSSI = 0;
				IF_RSSI = (-94*IF_gain - 49500)/1000;
			}
		}
		else if(tmp_rf_freq[tuner_id] < 500000)   //RF<500MHz
		{
			if(RF_gain < 608)
			{
				RF_RSSI = (-99*RF_gain + 5000)/1000;
				IF_RSSI = 0;
			}
			else
			{
				if(IF_gain<400)
					IF_RSSI = (-99*IF_gain - 37000)/1000;
				else if(IF_gain<800)
					IF_RSSI = (-28*IF_gain - 65000)/1000;
				else
					IF_RSSI = (-11*IF_gain-79000)/1000;

				RF_RSSI = 0;
			}
		}
		else if(tmp_rf_freq[tuner_id] < 800000)   //RF<800MHz
		{
			if(RF_gain < 608)
			{
				RF_RSSI = (-88*RF_gain + 4000)/1000;
				IF_RSSI = 0;
			}
			else
			{
				if(IF_gain<400)
					IF_RSSI = (-95*IF_gain - 35379)/1000;
				else if(IF_gain<800)
					IF_RSSI = (-18*IF_gain - 67346)/1000;
				else
					IF_RSSI = (-39*IF_gain-46654)/1000;

				RF_RSSI = 0;
			}
		}
		else //RF>=800M
		{
			if(RF_gain < 608)
			{
				RF_RSSI = (-88*RF_gain + 7000)/1000;
				IF_RSSI = 0;
			}
			else
			{
				if(IF_gain<400)
					IF_RSSI = (-95*IF_gain - 31379)/1000;
				else if(IF_gain<800)
					IF_RSSI = (-18*IF_gain - 65346)/1000;
				else
					IF_RSSI = (-39*IF_gain-44654)/1000;

				RF_RSSI = 0;
			}
		}

		RSSI = RF_RSSI+IF_RSSI+107;
	}
*/
	if (RSSI > 255)
	{
		*rf_level = 255;
	}
	else if (RSSI < 0)
	{
		*rf_level = 0;
	}
	else
	{
		*rf_level = RSSI;
	}

	return SUCCESS;
}

INT32 tun_r850_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
	
    switch(cmd)
    {
  
/*    
   		case NIM_TUNER_POWER_CONTROL:
		if (TRUE == (*param))
		{
			R850_LoopThrough_Type tmp_r850_lt;
			tmp_r850_lt = LT_OFF;
			
            		R850_Standby(tuner_id, tmp_r850_lt);
		}
            break;
*/
        case NIM_TUNER_GET_AGC:
		return tun_r850_get_rf_level(tuner_id, (UINT16 *)param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}


/*
void R850_Delay_MS(int ms)
{
	osal_task_sleep(ms);
}
*/


//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 WriteLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
#define BURST_SZ 13

//joey, 20111207, for R850.
typedef struct _I2C_OP_TYPE
{
	//UINT8 DevAddr;
	//UINT8 Data[50];
	UINT8 Data[51]; // 1 more than 50.
	UINT8 Len;
}I2C_OP_TYPE;

R850_ErrCode write_i2c(UINT32 tuner_id, I2C_OP_TYPE *I2C_Info)
{
	INT32 result = 0;
	UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = r850_dev_id[tuner_id];


//joey, 20150119, for R850 address selection.
	//UINT8 u8_add = I2C_Info->DevAddr;
	//UINT8 u8_add = 0x34;
	UINT8 u8_add = tuner_dev_ptr->c_tuner_base_addr;

	UINT8 len = (I2C_Info->Len)-1;
	UINT8 *buff = &(I2C_Info->Data[1]);

	RemainLen = len % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = len / BURST_SZ; 
	}
	else
	{
		BurstNum = len / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			data[j+1]   = buff[i * BURST_SZ + j ];
		}

		data[0] = I2C_Info->Data[0] + BURST_SZ*i;
		result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, BURST_SZ+1);
	}

	

	for ( i = 0 ; i < RemainLen ; i++ )
	{
		data[i+1]   = buff[BurstNum * BURST_SZ + i ];
	}

	data[0] = I2C_Info->Data[0] + BURST_SZ*BurstNum;
	
	result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, RemainLen+1);
	//libc_printf("r850 i2c write: %d !\n", result);

	if (result == SUCCESS)
	{
		return RT_Success;
	}
	else
	{
		return RT_Fail;
	}
}

R850_ErrCode read_i2c(UINT32 tuner_id, I2C_LEN_TYPE *I2C_Info)
{
	//UINT8 data[4]; // every time, write 14 byte..
	//INT32 i,j;

	INT32 result = 0;

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = r850_dev_id[tuner_id];
	
//joey, 20150119, for R850 address selection.
	//UINT8 u8_add = I2C_Info->DevAddr | 0x01;
	//UINT8 u8_add = 0x34 | 0x01;
	UINT8 u8_add = tuner_dev_ptr->c_tuner_base_addr | 0x01;

	UINT8 len = I2C_Info->Len;
	UINT8 *data = I2C_Info->Data;

//joey, 20111207, for R850.
	if (len > 8)
	{
		return RT_Fail;
	}

//joey, 20150119, for R850 read function fix a potetial bug.
	//result |= i2c_scb_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);
	result |= i2c_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);

/*
	libc_printf("r850 i2c read: %d !\n", result);

	for (i=0;i<len;i++)
	{
		libc_printf("r850 data[%d]: 0x%x !\n", i, data[i]);
		
	}
*/

	if (result == SUCCESS)
	{
		return RT_Success;
	}
	else
	{
		return RT_Fail;
	}
}


R850_ErrCode I2C_Write_Len(UINT32 tuner_id, I2C_LEN_TYPE *I2C_Info)
{
	I2C_OP_TYPE tmp_info;
	UINT8 i;

	for (i=0;i< I2C_Info->Len;i++)
	{
		tmp_info.Data[i+1] = I2C_Info->Data[i];
	}
	//tmp_info.DevAddr = I2C_Info->DevAddr;
	tmp_info.Data[0] = I2C_Info->RegAddr;
	tmp_info.Len = I2C_Info->Len+1;
	
	return write_i2c(tuner_id, &tmp_info);
}

UINT8 R850_Convert(UINT8 InvertNum)
{
	UINT8 ReturnNum = 0;
	UINT8 AddNum    = 0x80;
	UINT8 BitNum    = 0x01;
	UINT8 CuntNum   = 0;

	for(CuntNum = 0;CuntNum < 8;CuntNum ++)
	{
		if(BitNum & InvertNum)
			ReturnNum += AddNum;

		AddNum = AddNum >> 1;
		BitNum = BitNum << 1;
	}

	//libc_printf("r850 convert: from 0x%2x to 0x%2x !\n", InvertNum, ReturnNum);

	return ReturnNum;
}

R850_ErrCode I2C_Read_Len(UINT32 tuner_id, I2C_LEN_TYPE *I2C_Info)
{
	INT32 result;
	UINT8 tmp_cnt;
	I2C_Info->RegAddr  = 0x00;

	result = read_i2c(tuner_id, I2C_Info);

	if(RT_Success != result)
		return RT_Fail;

	for(tmp_cnt = 0;tmp_cnt < I2C_Info->Len;tmp_cnt ++)
	{
		 I2C_Info->Data[tmp_cnt] = R850_Convert(I2C_Info->Data[tmp_cnt]);
	}
	
	return RT_Success;
}

R850_ErrCode I2C_Write(UINT32 tuner_id, I2C_TYPE *I2C_Info)
{
	I2C_OP_TYPE tmp_info;

	//tmp_info.DevAddr = I2C_Info->DevAddr;
	tmp_info.Len       = 2;
	tmp_info.Data[0]      = I2C_Info->RegAddr;
	tmp_info.Data[1]      = I2C_Info->Data;

	return write_i2c(tuner_id, &tmp_info);
}

#endif