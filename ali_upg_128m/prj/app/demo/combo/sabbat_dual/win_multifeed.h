/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_multifeed.h
*
*    Description: multi_feed menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MULTIFEED_H_
#define _WIN_MULTIFEED_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MULTIFEED_SUPPORT
extern CONTAINER g_win_multifeed;

extern TEXT_FIELD multifeed_title;

extern OBJLIST  multifeed_ol;
extern SCROLL_BAR   multifeed_scb;
extern BITMAP   multifeed_mark;


extern CONTAINER multifeed_item1;
extern CONTAINER multifeed_item2;
extern CONTAINER multifeed_item3;
extern CONTAINER multifeed_item4;
extern CONTAINER multifeed_item5;
extern CONTAINER multifeed_item6;

extern TEXT_FIELD multifeed_txt1;
extern TEXT_FIELD multifeed_txt2;
extern TEXT_FIELD multifeed_txt3;
extern TEXT_FIELD multifeed_txt4;
extern TEXT_FIELD multifeed_txt5;
extern TEXT_FIELD multifeed_txt6;
#endif


#ifdef __cplusplus
}
#endif

#endif

