/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_password.h
*
*    Description: password input nemu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PASSWORD_H_
#define _WIN_PASSWORD_H_
#ifdef __cplusplus
extern "C"
{
#endif


extern CONTAINER    win_pwd_con;
extern TEXT_FIELD   win_pwd_title;
extern TEXT_FIELD   win_pwd_char;

typedef enum
{
    WIN_PWD_CHAN = 0,
    WIN_PWD_MENU,
}WIN_PWD_TYPE;

typedef enum
{
    WIN_PWD_INPUT = 0,
    WIN_PWD_EDIT,
}WIN_PWD_INPUT_TYPE;

typedef enum
{
    WIN_PWD_CALLED_UNKOWN = 0,
    WIN_PWD_CALLED_PROG_PLAYING,
    WIN_PWD_CALLED_PVR
}WIN_PWD_CALLED_MODULE;

BOOL win_pwd_open(UINT32* vkey_exist_array,UINT32 vkey_exist_cnt);
BOOL win_pwd_get_input(char* input_pwd);

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
BOOL get_pwd_is_open(void);
#endif

void win_set_pwd_caller(WIN_PWD_CALLED_MODULE pwd_by_rating);
WIN_PWD_CALLED_MODULE win_get_pwd_caller();

#ifdef __cplusplus
}
#endif

#endif//_WIN_PASSWORD_H_

