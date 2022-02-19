/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r836.c
*
*    Description:    This file contains alpstdae basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu		 Ver 0.1	Create file.
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

#include "tun_r836.h"
#include "R836.h"

//#if ((SYS_TUN_MODULE == R836) || (SYS_TUN_MODULE == ANY_TUNER))
#if 1


#define R840_PRINTF(...)	do{}while(0) //libc_printf

static struct COFDM_TUNER_CONFIG_EXT * r836_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 r836_tuner_cnt = 0;
static UINT8 r836_reg_ini[2]={0,0};
static UINT32 tmp_rf_freq[2] = {0,0};

INT32 tun_r836_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (r836_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));	
	
	r836_dev_id[r836_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = r836_tuner_cnt;

	r836_reg_ini[r836_tuner_cnt]=0;		// status bit for initialized r836 register
	r836_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_r836_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=r836_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = r836_dev_id[tuner_id];

	// r836 has no PLL lock indicator, only need to wait 100ms, enough for lock
	osal_task_sleep(100);

	//result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = 1;

	return SUCCESS;
}

INT32 tun_r836_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)	
{	

	INT32 result;

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=r836_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = r836_dev_id[tuner_id];


	if(0==r836_reg_ini[tuner_id])
	{

		R840_Init(); // direct setting. should be same with the tuner driver.
		//R840_GPIO(HI_SIG);

		r836_reg_ini[tuner_id]=1;
	}

	//joey, 20120110, rt810 not support 6 & 7/8 sametime.
	//now, only support 7M/8M filter.
	R840_Set_Info tmp_r840_info;

	switch (bandwidth)
	{
		case 6:
			tmp_r840_info.R840_Standard =R840_DVB_T2_6M;
			break;

		case 7:
			tmp_r840_info.R840_Standard =R840_DVB_T2_7M;
			break;

		case 8:
			tmp_r840_info.R840_Standard =R840_DVB_T2_8M;
			break;

		default:
			R840_PRINTF("Error! bandwidth=%d\n",bandwidth);   
			break;
	}

	tmp_r840_info.RF_KHz = freq;
	tmp_r840_info.R840_LT = LT_OFF;
	tmp_r840_info.R840_ClkOutMode = CLK_OUT_OFF;
	tmp_r840_info.R840_IfAgc_Select = IF_AGC1;
		
	R840_SetPllData(tmp_r840_info);

	tmp_rf_freq[tuner_id] = freq;

	return SUCCESS;

}

INT32 tun_r836_get_rf_level(UINT32 tuner_id, UINT16 *rf_level)// return level in dbuV.
{
	INT32 result;
	UINT16 RF_gain, IF_gain;
	INT32 RF_RSSI, IF_RSSI, RSSI;

	R840_RF_Gain_Info R840_rf_gain;

	//read RF_gain from tuner
	R840_GetRfGain(&R840_rf_gain);
	RF_gain = R840_rf_gain.RF_gain_comb;

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

INT32 tun_r836_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
	
    switch(cmd)
    {
/*    
        case NIM_TUNER_POWER_CONTROL:
		if (TRUE == (*param))
		{
			R840_LoopThrough_Type tmp_r836_lt;
			tmp_r836_lt = LT_OFF;
			
            		R840_Standby(tmp_r836_lt);
		}
            break;
*/

        case NIM_TUNER_GET_AGC:
		return tun_r836_get_rf_level(tuner_id, (UINT16 *)param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}


/*
void R840_Delay_MS(int ms)
{
	osal_task_sleep(ms);
}
*/

void R840_Delay_MS(int ms)
{
	int i = 0;
	for (i=0; i<=ms; i++)
	{
		osal_delay(1000);		
	}
}

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

//joey, 20111207, for R836.
typedef struct _I2C_OP_TYPE
{
	//UINT8 DevAddr;
	//UINT8 Data[50];
	UINT8 Data[51]; // 1 more than 50.
	UINT8 Len;
}I2C_OP_TYPE;

R840_ErrCode write_i2c(I2C_OP_TYPE *I2C_Info)
{
	INT32 result = 0;
	UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = r836_dev_id[0];


	//UINT8 u8_add = I2C_Info->DevAddr;
	UINT8 u8_add = 0x34;
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
	//libc_printf("r836 i2c write: %d !\n", result);

	if (result == SUCCESS)
	{
		return RT_Success;
	}
	else
	{
		return RT_Fail;
	}
}

R840_ErrCode read_i2c(I2C_LEN_TYPE *I2C_Info)
{
	//UINT8 data[4]; // every time, write 14 byte..
	INT32 i,j;

	INT32 result = 0;

	struct COFDM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = r836_dev_id[0];
	
	//UINT8 u8_add = I2C_Info->DevAddr | 0x01;
	UINT8 u8_add = 0x34 | 0x01;
	UINT8 len = I2C_Info->Len;
	UINT8 *data = I2C_Info->Data;

//joey, 20111207, for R836.
	if (len > 8)
	{
		return RT_Fail;
	}

	result |= i2c_scb_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);

/*
	libc_printf("r836 i2c read: %d !\n", result);

	for (i=0;i<len;i++)
	{
		libc_printf("r836 data[%d]: 0x%x !\n", i, data[i]);
		
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


R840_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info)
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
	
	return write_i2c(&tmp_info);
}

UINT8 R840_Convert(UINT8 InvertNum)
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

	//libc_printf("r836 convert: from 0x%2x to 0x%2x !\n", InvertNum, ReturnNum);

	return ReturnNum;
}

R840_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info)
{
	INT32 result;
	UINT8 tmp_cnt;
	I2C_Info->RegAddr  = 0x00;

	result = read_i2c(I2C_Info);

	if(RT_Success != result)
		return RT_Fail;

	for(tmp_cnt = 0;tmp_cnt < I2C_Info->Len;tmp_cnt ++)
	{
		 I2C_Info->Data[tmp_cnt] = R840_Convert(I2C_Info->Data[tmp_cnt]);
	}
	
	return RT_Success;
}

R840_ErrCode I2C_Write(I2C_TYPE *I2C_Info)
{
	I2C_OP_TYPE tmp_info;

	//tmp_info.DevAddr = I2C_Info->DevAddr;
	tmp_info.Len       = 2;
	tmp_info.Data[0]      = I2C_Info->RegAddr;
	tmp_info.Data[1]      = I2C_Info->Data;

	return write_i2c(&tmp_info);
}

#endif
