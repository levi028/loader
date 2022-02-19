/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_find.h
*
*    Description: The menu when press find botton.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_FIND_H_
#define _WIN_FIND_H_
//win_find.h

#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER g_win_find;
#ifndef KEYBOARD_SUPPORT
extern CONTAINER find_item1;    /* Name */

extern TEXT_FIELD find_title;
extern TEXT_FIELD find_txt1;
extern EDIT_FIELD find_edt1;    /* Name */
#else
extern TEXT_FIELD find_title;
extern TEXT_FIELD find_input;
extern TEXT_FIELD find_txt_char;
#endif

#ifdef __cplusplus
}
#endif

#endif

