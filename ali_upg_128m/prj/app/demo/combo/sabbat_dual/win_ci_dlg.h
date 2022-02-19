/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_dlg.h
*
*    Description: The realize the function of CI dlg
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_CI_DLG_H_
#define _WIN_CI_DLG_H_
//win_ci_dlg.h
#ifdef __cplusplus
extern "C"{
#endif

#ifdef CI_SUPPORT
BOOL is_ci_dlg_openning(void);
void set_ci_dlg_openning( BOOL show);
void win_ci_dlg_close(void);
void win_ci_dlg_show_status(UINT32 msg_code);
UINT8 win_ci_dlg_get_status(void);
#else
UINT8 win_ci_dlg_get_status(void);
#endif

#ifdef __cplusplus
 }
#endif
#endif//_WIN_CI_DLG_H_

