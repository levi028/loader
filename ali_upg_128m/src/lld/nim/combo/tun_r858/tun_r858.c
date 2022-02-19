/*****************************************************************************
*    Copyright (C)2016 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r858.c
*
*    Description:    This file contains tuner basic function in LLD. 
*    History:
*           Date              Author           Version          Reason
*	    =======   =====	==    =========   =========
*	1.  20160513	   Robin.gan 	      Ver 1.0	       Create file.
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

#include "tun_r858.h"
#include "R858.h"

#if 0
#define R858_PRINTF			libc_printf
#else
#define R858_PRINTF(...)	do{}while(0) 
#endif

#define MAX_TUNER_SUPPORT_NUM 2

static struct QAM_TUNER_CONFIG_EXT r858_dev_id[MAX_TUNER_SUPPORT_NUM];
static UINT32 r858_tuner_cnt = 0;

extern UINT8 R858_ADDRESS[R858_MAX_NUM][2];

QAM_MODE_STR g_tuner_qam_mode = QAM_MODE_J83AC;

UINT32 g_cur_freq = 0;

UINT8 R858_Convert(UINT8 InvertNum)
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
	
	return ReturnNum;
}

R858_ErrCode r858_i2c_write(UINT32 tuner_id, I2C_TYPE *I2C_Info)
{
	UINT8 data[56];
	int i2c_result;		
	struct QAM_TUNER_CONFIG_EXT * r858_ptr = NULL;

	r858_ptr = &r858_dev_id[tuner_id];

	data[0] = I2C_Info->RegAddr;
	data[1] = I2C_Info->Data;

	//R858_PRINTF("[%s %d]r858_ptr->i2c_type_id=0x%x, DevAddr=0x%x, RegAddr=0x%x\n", __FUNCTION__, __LINE__, r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data[0]);
	i2c_result = i2c_write(r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data, 2);
	if (SUCCESS != i2c_result)
	{
		R858_PRINTF("[%s %d]I2c_write error, i2c_result=%d!\n", __FUNCTION__, __LINE__, i2c_result);
		return RT_Fail;
	}
	
	return RT_Success;	
}

R858_ErrCode r858_i2c_write_len(UINT32 tuner_id, I2C_LEN_TYPE *I2C_Info)
{
	UINT8 data[56];
	UINT32 len_done = 0; //have writen length
	UINT8 length;
	int i2c_result;	
	struct QAM_TUNER_CONFIG_EXT * r858_ptr = NULL;

	r858_ptr = &r858_dev_id[tuner_id];	
	
	length = I2C_Info->Len;			//the length need to write
	data[0] = I2C_Info->RegAddr;	//the reg addr

	//R858_PRINTF("[%s %d]r858_ptr->i2c_type_id=0x%x, DevAddr=0x%x, RegAddr=0x%x, length=%d\n", __FUNCTION__, __LINE__, r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data[0], length);
	while((len_done+15)<length)//the i2c can only read/write 16 bytes one time
	{
		MEMCPY(&data[1], &(I2C_Info->Data[len_done]), 15);
		i2c_result = i2c_write(r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data, 16);
		len_done += 15;
		data[0] += 15;//the new reg addr
		if(SUCCESS != i2c_result)
		{
			R858_PRINTF("[%s %d]I2c_write error, i2c_result=%d!\n", __FUNCTION__, __LINE__, i2c_result);
			return RT_Fail;
		}
	}
	
	MEMCPY(&data[1], &(I2C_Info->Data[len_done]), length-len_done);
	i2c_result = i2c_write(r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data, length-len_done+1);
	if(SUCCESS != i2c_result)
	{
		R858_PRINTF("[%s %d]I2c_write error!\n", __FUNCTION__, __LINE__);
		return RT_Fail;
	}
	
	return RT_Success;	
}

R858_ErrCode r858_i2c_read_len(UINT32 tuner_id, I2C_LEN_TYPE *I2C_Info)
{
	UINT8 data[56];
	UINT8 length;
	UINT8 i;
	int i2c_result;
	struct QAM_TUNER_CONFIG_EXT * r858_ptr = NULL;

	r858_ptr = &r858_dev_id[tuner_id];	
	data[0] = 0x00;			//the Reg addr,  it must read from 0x00 for r858
	length = I2C_Info->Len;	//the length need to write	
	
	i2c_result = i2c_write_read(r858_ptr->i2c_type_id, I2C_Info->I2cAddr, data, 1, length);
	if(SUCCESS != i2c_result)
	{
		R858_PRINTF("[%s %d]I2c_write_read error, i2c_result=%d!\n", __FUNCTION__, __LINE__, i2c_result);
		return RT_Fail;
	}
	
	for (i=0; i<length; i++)
	{
		 I2C_Info->Data[i] = R858_Convert(data[i]);
	}
	
	return RT_Success;
}

static UINT8 init_r858_flag = 0;
INT32 tun_r858_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * r858_ptr = NULL;
	R858_Set_Info r858_info;
	UINT8 addr;
	UINT8 cur_tuner_id;	

 	g_tuner_qam_mode = ptrTuner_Config->qam_mode;
	
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (r858_tuner_cnt > MAX_TUNER_SUPPORT_NUM))
	{
		R858_PRINTF("[%s %d](ptrTuner_Config == NULL) || (r858_tuner_cnt >= MAX_TUNER_SUPPORT_NUM)\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}	

	if (ptrTuner_Config->c_tuner_reopen)
	{
		cur_tuner_id = *tuner_id;
	}
	else
	{
		cur_tuner_id = r858_tuner_cnt;
	}		

	r858_ptr = &r858_dev_id[cur_tuner_id];	
	MEMCPY(r858_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	addr = ptrTuner_Config->c_tuner_base_addr;

	if ((0x14==addr)||(0x54==addr)||(0x94==addr)||(0xD4==addr))//judge which tuner is used according to I2c addr
	{		
		R858_ADDRESS[R858_NUM1][R858_TUNER1] = addr;
		R858_ADDRESS[R858_NUM1][R858_TUNER2] = addr + 0x20;		
	}
	else if ((0x34==addr)||(0x74==addr)||(0xB4==addr)||(0xF4==addr))
	{
		R858_ADDRESS[R858_NUM1][R858_TUNER1] = addr - 0x20;
		R858_ADDRESS[R858_NUM1][R858_TUNER2] = addr;		
	}
	else
	{
		R858_PRINTF("[%s %d]tuner I2C addr error, pls check it!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}

	if (!init_r858_flag)
	{
		init_r858_flag = 1;
		r858_info.R858_Standard = R858_DVB_C_8M_IF_5M;
		r858_info.R858_Standard_2 = R858_DVB_C_8M_IF_5500;
		R858_PRINTF("[%s %d]R858_Init_ALL!\n", __FUNCTION__, __LINE__);
		R858_Init_ALL(R858_NUM1, r858_info);
	}
	
	if (!ptrTuner_Config->c_tuner_reopen)
    {
    	*tuner_id = r858_tuner_cnt;	
        r858_tuner_cnt++;
    }

	R858_PRINTF("[%s %d]*tuner_id=%d, r858_tuner_cnt=%d!\n", __FUNCTION__, __LINE__, *tuner_id, r858_tuner_cnt);
	return SUCCESS;
}

INT32 tun_r858_status(UINT32 tuner_id, UINT8 *lock)
{
	if ((tuner_id >= r858_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
	{
		R858_PRINTF("[%s %d]tuner_id=%d, (tuner_id >= r858_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM)\n", __FUNCTION__, __LINE__, tuner_id);
		return ERR_FAILUE;
	}
		
	//osal_task_sleep(100);//this is not need sleep, because lock is always 1.
	
	*lock = 1;			

	return SUCCESS;
}

INT32 tun_r858_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 cmd)	
{	
	struct QAM_TUNER_CONFIG_EXT * r858_ptr = NULL;
	R858_Set_Info r858_info;
	UINT8 addr;
	R858_ErrCode result;	
	
	if ((tuner_id >= r858_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
	{
		R858_PRINTF("[%s %d]tuner_id=%d, (tuner_id >= r858_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM)\n", __FUNCTION__, __LINE__, tuner_id);
		return ERR_FAILUE;
	}
	
	r858_ptr = &r858_dev_id[tuner_id];

	g_cur_freq = freq;//get rf level will use this param
	r858_info.RF_KHz = freq;
	r858_info.R858_LT = LT_ON;
	r858_info.R858_ClkOutMode = CLK_OUT_ON;
	if (QAM_MODE_J83B == g_tuner_qam_mode)
	{
		r858_info.R858_Standard = R858_J83B_IF_4000;
		r858_info.R858_Standard_2 = R858_J83B_IF_4300;	
	}
	else
	{
		r858_info.R858_Standard = R858_DVB_C_8M_IF_5M;
		r858_info.R858_Standard_2 = R858_DVB_C_8M_IF_5500;		
	}
	
	R858_PRINTF("[%s %d]tuner_id=%d, r858_info.RF_KHz=%d!\n", __FUNCTION__, __LINE__, tuner_id, r858_info.RF_KHz);		
	addr = r858_ptr->c_tuner_base_addr;
	
	if ((0x14==addr)||(0x54==addr)||(0x94==addr)||(0xD4==addr))
	{
		result = R858_SetPllData(R858_NUM1, R858_TUNER1, r858_info);		
		if (RT_Success != result)
		{
			R858_PRINTF("[%s %d]R858_SetPllData fail!\n", __FUNCTION__, __LINE__);
			return ERR_FAILUE;
		}		
	}
	else if ((0x34==addr)||(0x74==addr)||(0xB4==addr)||(0xF4==addr))
	{
		result = R858_SetPllData(R858_NUM1, R858_TUNER2, r858_info);
		if (RT_Success != result)
		{
			R858_PRINTF("[%s %d]R858_SetPllData fail!\n", __FUNCTION__, __LINE__);
			return ERR_FAILUE;
		}		
	}	
	else
	{
		R858_PRINTF("[%s %d]tuner I2C addr error, pls check it!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}
	
	return SUCCESS;

}

INT32 tun_r858_get_rf_level(UINT32 tuner_id, INT32 *rf_level)
{		
	UINT8 addr;
	R858_ErrCode result;
	INT32 tmp = 0;
	
	addr = r858_dev_id[tuner_id].c_tuner_base_addr;

	if ((0x14==addr)||(0x54==addr)||(0x94==addr)||(0xD4==addr))
	{
		result = R858_GetTotalRssi(R858_NUM1, R858_TUNER1, g_cur_freq, rf_level);		
		if (RT_Success != result)
		{
			R858_PRINTF("[%s %d]R858_GetTotalRssi fail!\n", __FUNCTION__, __LINE__);
			return ERR_FAILUE;
		}		
	}
	else if ((0x34==addr)||(0x74==addr)||(0xB4==addr)||(0xF4==addr))
	{
		result = R858_GetTotalRssi(R858_NUM1, R858_TUNER2, g_cur_freq, rf_level);		
		if (RT_Success != result)
		{
			R858_PRINTF("[%s %d]R858_GetTotalRssi fail!\n", __FUNCTION__, __LINE__);
			return ERR_FAILUE;
		}		
	}	
	else
	{
		R858_PRINTF("[%s %d]tuner I2C addr error, pls check it!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}
	//change rf_level to 0.1dbm
	tmp = *rf_level;
	*rf_level = tmp *(-10);
	R858_PRINTF("[%s %d]rf_level = %d\n", __FUNCTION__, __LINE__,*rf_level);
	return SUCCESS;
}

INT32 tun_r858_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = ERR_FAILUE;

	switch(cmd)
    {
        case NIM_TUNER_GET_AGC:
			return tun_r858_get_rf_level(tuner_id, (INT32 *)param);
            break;
		case NIM_TUNER_GET_RF_POWER_LEVEL:
			return tun_r858_get_rf_level(tuner_id, (INT32 *)param);
            break;
		case NIM_TURNER_SET_STANDBY:
			break;
			
        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}
