/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: scpi_cmd_drv.c
*   
*    Description: 
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <bus/dog/dog.h>
#include <hld/hld_dev.h>
#include "scpi.h"
#include "scpi_cmd_drv.h"


/*-------------------------------------------------------------------------
Function Name: drvStr2Hex
Function Description: format string like "0x123456789" to hex  .
Author: ray.gong (2012-11-10)
Input parameter: szInput:input string;
				 puiVal:point to the value after format 
				 
Output parameter:
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 drv_str2hex(CHAR *sz_input,UINT32 *pui_val)
{
	UINT32 i=0;
	//UINT32 ui_rtn=-1;
	UINT32 ui_val_tmp=0;
	//UINT32 ui_data_type=0;
	UINT32 ui_pow_val=0;
	
	if((NULL==sz_input)||(strlen(sz_input)<3))
	{
		return -1;
	}

	if(((sz_input[0]!='0'))||((sz_input[1]!='x')&&(sz_input[1]!='X')))
	{
		return -1;
	}

	for(i=0;i<strlen(sz_input)-2;i++)
	{
		if((sz_input[2+i]>='0')&&(sz_input[2+i]<='9'))
		{
			//uiPowVal=drvPower(16,(strlen(szInput)-3-i));
			ui_pow_val=1<<(4*(strlen(sz_input)-3-i));
			if((UINT32)-1==ui_pow_val)
			{
				return -1;
			}				
			ui_val_tmp+=(sz_input[2+i]-'0')*ui_pow_val;  
		}
		else if((sz_input[2+i]>='a')&&(sz_input[2+i]<='f'))
		{
			//uiPowVal=drvPower(16,(strlen(szInput)-3-i));
			ui_pow_val=1<<(4*(strlen(sz_input)-3-i));
			if((UINT32)-1==ui_pow_val)
			{
				return -1;
			}				
			ui_val_tmp+=(sz_input[2+i]-'a'+10)*ui_pow_val;  		}
		else if((sz_input[2+i]>='A')&&(sz_input[2+i]<='F'))
		{
			//uiPowVal=drvPower(16,(strlen(szInput)-3-i));
			ui_pow_val=1<<(4*(strlen(sz_input)-3-i));
			if((UINT32)-1==ui_pow_val)
			{
				return -1;
			}				
			ui_val_tmp+=(sz_input[2+i]-'A'+10)*ui_pow_val;  		
		}
		else
		{
			return -1;
		}
	}

	*pui_val=ui_val_tmp;
	return 0;
}


/*-------------------------------------------------------------------------
Function Name: drvReadMem
Function Description: read memory   .
Author: ray.gong (2012-11-10)
Input parameter: uiMode:0:8bit once read;1:16bit once read;2:32bit once read;
				 uiAddr:memory address 
				 
Output parameter:
				puiVal:point to the read value of uiAddr
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 drv_read_mem(UINT32 ui_mode,UINT32 ui_addr,UINT32 *pui_val)
{
	UINT32 ui_tmp=0;
	UINT32 ui_rtn=-1;
	
	if((0==ui_addr)||(NULL==pui_val)||(ui_mode>BIT_32))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			ui_tmp=*(UINT8 *)(ui_addr);
			ui_rtn=0;
			break;
		case BIT_16:
			ui_tmp=*(UINT16 *)(ui_addr);
			ui_rtn=0;
			break;
		case BIT_32:
			ui_tmp=*(UINT32 *)(ui_addr);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	if(0==ui_rtn)
	{
		*pui_val=ui_tmp;
	}
	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: drvWriteMem
Function Description: read MEMORY   .
Author: ray.gong (2012-11-10)
Input parameter: uiMode:0:8bit once read;1:16bit once read;2:32bit once read;
				 uiAddr:registor address 
				 uiVal:the value want to be write to address uiAddr
				 
Output parameter:NONE
				
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 drv_write_mem(UINT32 ui_mode,UINT32 ui_addr,UINT32 ui_val)
{
	//UINT32 ui_tmp=0;
	UINT32 ui_rtn=-1;
	
	if((0==ui_addr)||(ui_mode>BIT_32))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			*(UINT8 *)(ui_addr)=(UINT8)(ui_val&0xff);
			ui_rtn=0;
			break;
		case BIT_16:
			*(UINT16 *)(ui_addr)=(UINT16)(ui_val&0xffff);
			ui_rtn=0;
			break;
		case BIT_32:
			*(UINT32 *)(ui_addr)=(UINT32)(ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: drvReadReg
Function Description: read registor   .
Author: ray.gong (2012-11-10)
Input parameter: uiMode:0:8bit once read;1:16bit once read;2:32bit once read;
				 uiAddr:registor address 
				 
Output parameter:
				puiVal:point to the read value of uiAddr
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 drv_read_reg(UINT32 ui_mode,UINT32 ui_addr,UINT32 *pui_val)
{
	UINT32 ui_tmp=0;
	UINT32 ui_rtn=-1;
	
	if((0==ui_addr)||(NULL==pui_val)||(ui_mode>BIT_32))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			ui_tmp=*(volatile UINT8 *)(ui_addr);
			ui_rtn=0;
			break;
		case BIT_16:
			ui_tmp=*(volatile UINT16 *)(ui_addr);
			ui_rtn=0;
			break;
		case BIT_32:
			ui_tmp=*(volatile UINT32 *)(ui_addr);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	if(0==ui_rtn)
	{
		*pui_val=ui_tmp;
	}
	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: drvWriteReg
Function Description: read registor   .
Author: ray.gong (2012-11-10)
Input parameter: uiMode:0:8bit once read;1:16bit once read;2:32bit once read;
				 uiAddr:registor address 
				 uiVal:the value want to be write to address uiAddr
				 
Output parameter:NONE
				
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/

UINT32 drv_write_reg(UINT32 ui_mode,UINT32 ui_addr,UINT32 ui_val)
{
	//UINT32 ui_tmp=0;
	UINT32 ui_rtn=-1;
	
	if((0==ui_addr)||(ui_mode>BIT_32))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			*(volatile UINT8 *)(ui_addr)=(UINT8)(ui_val&0xff);
			ui_rtn=0;
			break;
		case BIT_16:
			*(volatile UINT16 *)(ui_addr)=(UINT16)(ui_val&0xffff);
			ui_rtn=0;
			break;
		case BIT_32:
			*(volatile UINT32 *)(ui_addr)=(UINT32)(ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return ui_rtn;
}

UINT32 drv_get_dump_mem(module_id mdl_id,UINT32 ui_mem_len,UINT32 *pui_mem_addr)
{
	UINT32 ui_rtn=-1;
#if 0
	switch(mdl_id)
	{
		case MDL_DECA:
			*pui_mem_addr=drv_deca_dump_init(ui_mem_len);
			break;
		case MDL_SND:
			*pui_mem_addr=drv_snd_dump_init(ui_mem_len);
			break;
		default:
			return -1;
	}

	if((-1!=*pui_mem_addr)&&(0!=*pui_mem_addr))
	{
		ui_rtn=0;
	}
	else
	{
		ui_rtn=-1;
	}
#endif
	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: drvReleaseDumpMem
Function Description: release the memory for module dump after end dump operate   .
Author: ray.gong (2012-11-10)
Input parameter: MdlId:see data type ModuleID
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 drv_release_dump_mem(module_id mdl_id)
{
#if 0
	switch(mdl_id)
	{
		case MDL_DECA:
			drv_deca_dump_debug_release();
			break;
		case MDL_SND:
			drv_snd_dump_debug_release();
			break;
		default:
			return -1;
	}
#endif
	return 0;
}
