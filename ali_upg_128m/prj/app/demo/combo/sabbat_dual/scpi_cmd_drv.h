/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: scpi_cmd_drv.h
*   
*    Description: 
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SCPI_CMD_DRV__
#define __SCPI_CMD_DRV__

#ifdef __cplusplus
extern "C"
{
#endif

extern UINT32 drv_str2hex(CHAR *sz_input,UINT32 *pui_val);
extern UINT32 drv_read_mem(UINT32 ui_mode,UINT32 ui_addr,UINT32 *pui_val);
extern UINT32 drv_read_reg(UINT32 ui_mode,UINT32 ui_addr,UINT32 *pui_val);
extern UINT32 drv_write_mem(UINT32 ui_mode,UINT32 ui_addr,UINT32 ui_val);
extern UINT32 drv_write_reg(UINT32 ui_mode,UINT32 ui_addr,UINT32 ui_val);
extern UINT32 drv_get_dump_mem(module_id mdl_id,UINT32 ui_mem_len,UINT32 *pui_mem_addr);
extern UINT32 drv_release_dump_mem(module_id mdl_id);

#ifdef __cplusplus
}
#endif

#endif
