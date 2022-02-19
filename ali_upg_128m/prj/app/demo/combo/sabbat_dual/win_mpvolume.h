/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpvolume.h
*
*    Description: Mediaplayer volume and banance setup UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MPVOLUME_H_
#define _WIN_MPVOLUME_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER    g_win_mpvolume;

extern TEXT_FIELD     mpbalance_txt;
extern TEXT_FIELD     mpvolume_txt;
extern PROGRESS_BAR mpvolume_bar;
extern PROGRESS_BAR mpbalance_bar;
extern BITMAP         mpbalance_bmp;

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

