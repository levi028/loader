/*****************************************************************************
*    Copyright (C)2016 Ali Corporation. All Rights Reserved.
*
*    File:    tps65233.c
*
*    Description: tps65233 lnb driver
*    History:
*                   Date                      Athor            Version                 Reason
*        ============       =========     ======     ================
*    1.     2016-03-13          	Robin gan        Ver 1.0                Create file.
*   
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim.h>
#include "lnb_tps65233.h"

#define MAX_LNB_TPS65233   2

/*******************************/
/*VSEL2    VSEL1    VSEL0    LNB (V) */
/*0             0         0           13       */
/*0             0         1          13.4     */
/*0             1         0          13.8     */
/*0             1         1          14.2     */
/*1             0         0           18      */
/*1             0         1          18.6    */
/*1             1         0          19.2    */
/*1             1         1          19.8    */
/******************************/
#define LNB_H_VALUE	0x6
#define LNB_V_VALUE	0x2

#define LNB_I2C_ENABLE		(1<<7)		//I2C control enable
#define LNB_I2C_DISABLE		(~(1<<7))	//I2C control disable

#define LNB_TONE_GATE_ON	(1<<5)		//tone gate on
#define LNB_TONE_GATE_OFF	(~(1<<5))	//tone gate off

#define LNB_OUTPUT_ENABLE	(1<<3)		//lnb output enable
#define LNB_OUTPUT_DISABLE	(~(1<<3))	//lnb output disable


UINT32 lnb_tps65233_cnt = 0;

struct LNB_TPS65233_CONTROL
{
    UINT32 i2c_base_addr;
    UINT32 i2c_type_id;
    OSAL_ID mutex_id;
    OSAL_ID timmer_id;
    UINT8   int_gpio_en;
    UINT8   int_gpio_polar;
    UINT8   int_gpio_num;
    UINT8   current_polar;
    UINT8   power_en;
};

struct LNB_TPS65233_CONTROL lnb_tps65233_array[MAX_LNB_TPS65233] =
{
{0, 0, OSAL_INVALID_ID, OSAL_INVALID_ID,0, 0, 0, 0, 0},
{0, 0, OSAL_INVALID_ID, OSAL_INVALID_ID,0, 0, 0, 0, 0}
};

static int lnb_i2c_write(UINT32 id, UINT32 i2c_base_addr, UINT8 reg_start, UINT8* buff, UINT8 length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;
	
	data[0] = reg_start;

	//because the i2c driver limit, only 16 bytes can be r/w everytime 
	while((rd+15)<length)
	{
		MEMCPY(&data[1], &buff[rd], 15);
		i2c_result = i2c_write(lnb_tps65233_array[id].i2c_type_id, i2c_base_addr, data, 16);
		rd += 15;
		data[0] += 15;
		if(SUCCESS != i2c_result)
		{
			LNB_PRINTF("[%s %d], i2c_write error, id = %d, i2c_result=%d\n", __FUNCTION__, __LINE__, id, i2c_result);
			return i2c_result;
		}
	}	
	MEMCPY(&data[1], &buff[rd], length-rd);
	i2c_result = i2c_write(lnb_tps65233_array[id].i2c_type_id, i2c_base_addr, data, length-rd+1);
	
	return i2c_result;
}

static int lnb_i2c_read(UINT32 id, UINT32 i2c_base_addr, UINT8 reg_start, UINT8* buff, UINT8 length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;	
	data[0] = reg_start;

	i2c_result = i2c_write_read(lnb_tps65233_array[id].i2c_type_id, i2c_base_addr, data, 1, length-rd);	
	if(SUCCESS != i2c_result)
	{
		LNB_PRINTF("[%s %d], i2c_write_read error, id = %d, i2c_result=%d\n", __FUNCTION__, __LINE__, id, i2c_result);
		return i2c_result;
	}
	MEMCPY(&buff[0], &data[0],length);
	
	return i2c_result;
}

INT32 lnb_tps65233_set_pol(UINT32 id, UINT8 param)
{
	INT32 result;
	UINT8 byte;

	result = lnb_i2c_read(id, lnb_tps65233_array[id].i2c_base_addr, REG_CONTROL_1, &byte, 1);
	if (result != SUCCESS)
	{
		LNB_PRINTF("[%s %d], lnb_i2c_read error, result=%d\n", __FUNCTION__, __LINE__, result);
		return result;
	}

	LNB_PRINTF("[%s %d], id = %d, before set reg value=0x%x\n", __FUNCTION__, __LINE__, id, byte);
	
	if(NIM_PORLAR_HORIZONTAL==param)
	{
		byte = (byte&0xF8) | LNB_H_VALUE;
	}
    else
    {
		byte = (byte&0xF8) | LNB_V_VALUE;
    }
	
	if ((result = lnb_i2c_write(id, lnb_tps65233_array[id].i2c_base_addr, REG_CONTROL_1, &byte, 1)) != SUCCESS) 
	{
		LNB_PRINTF("[%s %d], lnb_i2c_write error, result=%d\n", __FUNCTION__, __LINE__, result);
		return result;
	}	

	LNB_PRINTF("[%s %d], after set reg value=0x%x\n", __FUNCTION__, __LINE__, byte);
	
	return SUCCESS;
}

INT32 lnb_tps65233_init(UINT32 id)
{
    INT32 result = SUCCESS;
	UINT8 byte;

	LNB_PRINTF("[%s %d], id = %d, Enter!\n", __FUNCTION__, __LINE__, id);
	result = lnb_i2c_read(id, lnb_tps65233_array[id].i2c_base_addr, REG_CONTROL_1, &byte, 1);
	if (result != SUCCESS)
	{
		LNB_PRINTF("[%s %d], lnb_i2c_read error, result=%d\n", __FUNCTION__, __LINE__, result);
		return result;
	}
	
	byte = byte | LNB_I2C_ENABLE | LNB_TONE_GATE_ON | LNB_OUTPUT_ENABLE;//enable I2C control	
	if ((result = lnb_i2c_write(id, lnb_tps65233_array[id].i2c_base_addr, REG_CONTROL_1, &byte, 1)) != SUCCESS) 
	{
		LNB_PRINTF("[%s %d], lnb_i2c_write error, result=%d\n", __FUNCTION__, __LINE__, result);
		return result;
	}	
	
    if(lnb_tps65233_array[id].mutex_id == OSAL_INVALID_ID)
    {
        lnb_tps65233_array[id].mutex_id = osal_mutex_create();        
    }   

	LNB_PRINTF("[%s %d], Leave!\n", __FUNCTION__, __LINE__);
    return result;
}

INT32 lnb_tps65233_power_en(UINT32 id)
{
    INT32 result = SUCCESS;
	//todo
    return result;
}


INT32 lnb_tps65233_power_off(UINT32 id)
{
    INT32 result = SUCCESS;
	UINT8 byte;
	UINT32 addr;
	
	LNB_PRINTF("[%s %d], id = %d, Enter! \n", __FUNCTION__, __LINE__, id);

	//before call this function, EN pin has been pull low by lnb_ant_power_off function in power.c
	//so we must change the I2C addr according to spec.
	//EN=0, I2C addr=0x60; EN=1, I2C addr=0x61
	addr = lnb_tps65233_array[id].i2c_base_addr - 2;
	
	byte = 0x00;	
	if ((result = lnb_i2c_write(id, addr, REG_CONTROL_1, &byte, 1)) != SUCCESS) 
	{
		LNB_PRINTF("[%s %d], lnb_i2c_write error, id = %d, result=%d\n", __FUNCTION__, __LINE__, id, result);
		return result;
	}	

	LNB_PRINTF("[%s %d], Leave!\n", __FUNCTION__, __LINE__);	
    return result;
}


INT32 lnb_tps65233_control(UINT32 id, UINT32 cmd, UINT32 param)
{
    INT32 result = SUCCESS;
    
    switch(cmd)
    { 
    	case LNB_CMD_ALLOC_ID:
            lnb_tps65233_array[lnb_tps65233_cnt].mutex_id = OSAL_INVALID_ID;
            *((UINT32 *)param) = lnb_tps65233_cnt;//alloc lnb id
            lnb_tps65233_cnt++;
            if (lnb_tps65233_cnt > MAX_LNB_TPS65233)
                result = !SUCCESS;
            break;
			
        case LNB_CMD_INIT_CHIP:
            {
                struct EXT_LNB_CTRL_CONFIG * lnb_config = (struct EXT_LNB_CTRL_CONFIG *)param;
                lnb_tps65233_array[id].i2c_base_addr = lnb_config->i2c_base_addr;
                lnb_tps65233_array[id].i2c_type_id = lnb_config->i2c_type_id;
                lnb_tps65233_array[id].int_gpio_en = lnb_config->int_gpio_en;
                lnb_tps65233_array[id].int_gpio_polar = lnb_config->int_gpio_polar;
                lnb_tps65233_array[id].int_gpio_num = lnb_config->int_gpio_num;
                lnb_tps65233_array[id].current_polar = NIM_PORLAR_HORIZONTAL;
                lnb_tps65233_array[id].mutex_id = OSAL_INVALID_ID;
                lnb_tps65233_array[id].timmer_id = OSAL_INVALID_ID;
                result = lnb_tps65233_init(id);
            }
            break;
			
        case LNB_CMD_SET_POLAR:
            result = lnb_tps65233_set_pol(id, (UINT8)param);
            break;

		case LNB_CMD_POWER_EN:
			lnb_tps65233_power_en(id);
			break;

		case LNB_CMD_POWER_OFF:
			lnb_tps65233_power_off(id);
			break;
			
        default:
            result = !SUCCESS;
            break;
    }
    return result;
}

