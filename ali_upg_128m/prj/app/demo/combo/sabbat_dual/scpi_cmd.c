/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: scpi_cmd.c
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
#include "scpi_cmd.h"

ST_DBG_CMD g_st_dbg_cmdstr;

ST_DBG_CMD_NODE g_st_dbg_cmd_node[USER_CMD_COUNT_MAX+1]=
{
	{"help","list all cmd instroduce;",str_cmd_help},
	{"setdbgprint","set module debug printf level,0 is no print;",str_cmd_set_dbg_print},
	{"rdreg","read reg;",str_cmd_read_reg},
	{"wtreg","write reg;",str_cmd_write_reg},			
	{"rdmem","read memory;",str_cmd_read_mem},
	{"wtmem","write memory;",str_cmd_write_mem},
	{"taskinfo","get system task status information;",str_cmd_get_task_info},
	{"mallocdump","take memory space for module dump;",str_cmd_get_dump_mem},
	{"freedump","release memory space for module dump;",str_cmd_release_dump_mem},
	{"NULL","NULL",NULL}	
};

/*-------------------------------------------------------------------------
Function Name: StrCmdHelp
Function Description: list all support command.
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/

UINT32 str_cmd_help(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 i=0;
	UINT32 j=0;
	UINT32 i_space_len=0;
	UINT32 ui_total_len=0;
	UINT32 ui_rtn=-1;
	
	UINT32 i_cnt=sizeof(g_st_dbg_cmd_node)/sizeof(ST_DBG_CMD_NODE);
	
	for(i=0;((i<i_cnt)
		&&(0!=strcmp(g_st_dbg_cmd_node[i].ac_cmd_id,"NULL"))
		&&(0!=strcmp(g_st_dbg_cmd_node[i].ac_cmd_id,"")));i++)
	{
		ui_total_len += strlen("\r\n")+ strlen(g_st_dbg_cmd_node[i].ac_cmd_id);
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}
		strcat(sz_out_put,"\r\n");
		strcat(sz_out_put,g_st_dbg_cmd_node[i].ac_cmd_id);

		i_space_len=(UART_CMD_ID_LEN_MAX-strlen(g_st_dbg_cmd_node[i].ac_cmd_id))>0
			?(UART_CMD_ID_LEN_MAX-strlen(g_st_dbg_cmd_node[i].ac_cmd_id)):0;
		
		ui_total_len += i_space_len;
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}
		
		for(j=0;j<i_space_len;j++)
		{
			strcat(sz_out_put," ");
		}

		ui_total_len += strlen(g_st_dbg_cmd_node[i].ac_cmd_instuction);
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}	
		
		strcat(sz_out_put,g_st_dbg_cmd_node[i].ac_cmd_instuction);
		ui_rtn=0;
	}

END_CMD_PROC:

	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: StrCmdSetDbgPrint
Function Description: set soft module debug print level,0 is no print .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_set_dbg_print(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mdl_id=0;
	UINT32 ui_prt_level=0;
	//UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=2))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mdl_id)))
	{
		return -1;
	}

	if((1!=sscanf(argv[1],"%lu",&ui_prt_level)))
	{
		return -1;
	}		

	if(0!=drv_set_module_print_level(ui_mdl_id,ui_prt_level))
	{
		return -1;
	}
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set modlue [%lu] to printf level [%lu]",ui_mdl_id,ui_prt_level);
	return 0;
}

/*-------------------------------------------------------------------------
Function Name: StrCmdGetDumpMem
Function Description: get memory space for dump .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_get_dump_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mdl_id=0;
	UINT32 ui_mem_len=0;
	UINT32 ui_addr=0;
	//UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=2))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mdl_id)))
	{
		return -1;
	}

	if((1!=sscanf(argv[1],"%lu",&ui_mem_len)))
	{
		return -1;
	}		

	if(0!=drv_get_dump_mem(ui_mdl_id,ui_mem_len,&ui_addr))
	{
		return -1;
	}
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"get memory len [0x%08lx] at addr[0x%08lx] for module [%lu] ",ui_mem_len,ui_addr,ui_mdl_id);
	
	return 0;
}
/*-------------------------------------------------------------------------
Function Name: StrCmdReleaseDumpMem
Function Description: release memory space for dump .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_release_dump_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mdl_id=0;
	//UINT32 ui_mem_len=0;
	//UINT32 ui_addr=0;
	//UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=1))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mdl_id)))
	{
		return -1;
	}

	if(0!=drv_release_dump_mem(ui_mdl_id))
	{
		return -1;
	}
	
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"release dump memory for module [%lu] ",ui_mdl_id);
	
	return 0;
}

//extern TCB *g_tcb_table;
/*-------------------------------------------------------------------------
Function Name: StrCmdGetTaskInfo
Function Description: get task run status info .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_get_task_info(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	//UINT32 i=0;
	//UINT32 j=0;
	//INT32 i_space_len=0;
	//UINT32 ui_total_len=0;
	UINT32 ui_rtn=-1;
	//CHAR ac_tmp[UART_CMD_OUTPUT_LEN_MAX]={0};
	//CHAR ac_task_name[4]={0};
	//UINT32 i_cnt=0;
	#if 0
	for(i=0;((g_tcb_table[i].name[0]!=0));i++)
	{
		for(j=0;j<3;j++)
		{
			ac_task_name[j]=g_tcb_table[i].name[j];
		}
		
		ui_total_len +=strlen("NAME=\r\n") + strlen(ac_task_name);
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}
		sprintf(sz_out_put,"NAME=%s\r\n",ac_task_name);


		ui_total_len +=strlen("SP=[XXXXXXXX][XXXXXXXX][XXXXXXXX]\r\n");
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}
		sprintf(ac_tmp,"SP=[%08x][%08x][%08x]\r\n",g_tcb_table[i].sp,g_tcb_table[i].stackbegin,g_tcb_table[i].stackend);
		strcat(sz_out_put,ac_tmp);

		ui_total_len +=strlen("STATUS=[XXXX]\r\n");
		if(ui_total_len>UART_CMD_OUTPUT_LEN_MAX)
		{
			ui_rtn=-1; 
			goto END_CMD_PROC;
		}
		sprintf(ac_tmp,"STATUS=[%d]\r\n",g_tcb_table[i].state);
		strcat(sz_out_put,ac_tmp);
		ui_rtn=0;
	}

END_CMD_PROC:
	#endif
	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: StrCmdReadMem
Function Description: read memory  .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_read_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mode=0;
	UINT32 ui_addr=0;
	UINT32 ui_val=0;
	UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=2))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mode)))
	{
		return -1;
	}

	if(0!=drv_str2hex(argv[1],&ui_addr))
	{
		if((1!=sscanf(argv[1],"%lu",&ui_addr)))
		{
			return -1;
		}		
	}


	if(0!=drv_read_mem(ui_mode,ui_addr,&ui_val))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%02lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_16:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%04lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_32:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%08lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return ui_rtn;
}
/*-------------------------------------------------------------------------
Function Name: StrCmdWriteMem
Function Description: write memory  .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_write_mem(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mode=0;
	UINT32 ui_addr=0;
	UINT32 ui_val=0;
	UINT32 ui_rtn=0;

	if(0 == ui_rtn)
	{
		;
	}
	if((NULL==argc)||(NULL==argv)||(*argc!=3))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mode)))
	{
		return -1;
	}
	
	if(0!=drv_str2hex(argv[1],&ui_addr))
	{
		if((1!=sscanf(argv[1],"%lu",&ui_addr)))
		{
			return -1;
		}		
	}
	
	if(0!=drv_str2hex(argv[2],&ui_val))
	{
		if((1!=sscanf(argv[2],"%lu",&ui_val)))
		{
			return -1;
		}		
	}

	if(0!=drv_write_mem(ui_mode,ui_addr,ui_val))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%02lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_16:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%04lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_32:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%08lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return 0;
}

/*-------------------------------------------------------------------------
Function Name: StrCmdReadReg
Function Description: read registor  .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_read_reg(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mode=0;
	UINT32 ui_addr=0;
	UINT32 ui_val=0;
	UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=2))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mode)))
	{
		return -1;
	}

	if(0!=drv_str2hex(argv[1],&ui_addr))
	{
		if((1!=sscanf(argv[1],"%lu",&ui_addr)))
		{
			return -1;
		}		
	}

	if(0!=drv_read_reg(ui_mode,ui_addr,&ui_val))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%02lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_16:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%04lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_32:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Addr:[0x%08lx]=[0x%08lx]",ui_addr,ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return ui_rtn;
}

/*-------------------------------------------------------------------------
Function Name: StrCmdWriteReg
Function Description: write registor  .
Author: ray.gong (2012-11-10)
Input parameter: NONE;
				 
Output parameter:NONE
Return Value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
Modify history:
-------------------------------------------------------------------------*/
UINT32 str_cmd_write_reg(UINT32 *argc,CHAR **argv,CHAR *sz_out_put)
{
	UINT32 ui_mode=0;
	UINT32 ui_addr=0;
	UINT32 ui_val=0;
	UINT32 ui_rtn=-1;
	
	if((NULL==argc)||(NULL==argv)||(*argc!=3))
	{
		return -1;
	}

	if((1!=sscanf(argv[0],"%lu",&ui_mode)))
	{
		return -1;
	}

	if(0!=drv_str2hex(argv[1],&ui_addr))
	{
		if((1!=sscanf(argv[1],"%lu",&ui_addr)))
		{
			return -1;
		}		
	}
	
	if(0!=drv_str2hex(argv[2],&ui_val))
	{
		if((1!=sscanf(argv[2],"%lu",&ui_val)))
		{
			return -1;
		}		
	}


	if(0!=drv_write_reg(ui_mode,ui_addr,ui_val))
	{
		return -1;
	}

	switch(ui_mode)
	{
		case BIT_8:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%02lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_16:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%04lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		case BIT_32:
			snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"Write Addr:[0x%08lx]=[0x%08lx] Successed!",ui_addr,ui_val);
			ui_rtn=0;
			break;
		default:
			ui_rtn=-1;
			break;
	}

	return ui_rtn;
}


