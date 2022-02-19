/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpspectrum.h
*
*    Description: spectrum analysis
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_MPSPECTRUM_H_
#define _WIN_MPSPECTRUM_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER         spectrum_con;

extern PROGRESS_BAR        spectrum_bar_0;
extern BITMAP            spectrum_peak_0;

extern PROGRESS_BAR        spectrum_bar_1;
extern BITMAP            spectrum_peak_1;

extern PROGRESS_BAR        spectrum_bar_2;
extern BITMAP            spectrum_peak_2;

extern PROGRESS_BAR        spectrum_bar_3;
extern BITMAP            spectrum_peak_3;

extern PROGRESS_BAR        spectrum_bar_4;
extern BITMAP            spectrum_peak_4;

extern PROGRESS_BAR        spectrum_bar_5;
extern BITMAP            spectrum_peak_5;

extern PROGRESS_BAR        spectrum_bar_6;
extern BITMAP            spectrum_peak_6;

void update_bars(UINT32 param);
BOOL open_spectrum_dev(void);
BOOL close_spectrum_dev(void);

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

