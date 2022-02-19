/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpzoom.h
*
*    Description: Mediaplayer volume and banance setup UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MPZOOM_H_
#define _WIN_MPZOOM_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER g_win_mpzoom;
extern BITMAP win_mpzoom_bmp;
extern BITMAP win_mpzoom_larrow;
extern BITMAP win_mpzoom_tarrow;
extern BITMAP win_mpzoom_rarrow;
extern BITMAP win_mpzoom_darrow;
extern TEXT_FIELD win_mpzoom_title;

void win_mpzoom_open(void);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

