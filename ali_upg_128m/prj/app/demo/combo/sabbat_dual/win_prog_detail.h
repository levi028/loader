/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_detail.h
*
*    Description:   The realize of program detail info
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PROGDETAIL_H_
#define _WIN_PROGDETAIL_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern void show_firmware_info(void);
extern BOOL check_magic_key_firmware_info(UINT32 code);
extern TEXT_FIELD txt_string3;
extern TEXT_FIELD txt_string4;
extern TEXT_FIELD txt_tpinfo;
extern TEXT_FIELD txt_pidinfo;
extern TEXT_FIELD txt_string1;

extern TEXT_FIELD txt_progname;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
extern TEXT_FIELD txt_satname;
extern TEXT_FIELD txt_string2;
#endif

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif
