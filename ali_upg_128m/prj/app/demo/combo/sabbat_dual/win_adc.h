/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_adc.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _WIN_ADC_H_
#define _WIN_ADC_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER     adc_item1;        /* frequency*/
extern CONTAINER     adc_item2;        /* start */
extern CONTAINER     adc_item3;

extern TEXT_FIELD     adc_txt1;
extern TEXT_FIELD     adc_txt2;
extern TEXT_FIELD     adc_txt3;
extern TEXT_FIELD     adc_txt4;

extern EDIT_FIELD     adc_edt1;
extern MULTISEL     adc_sel1;

extern TEXT_FIELD adc_txt_progress;

extern PROGRESS_BAR adc_bar_progress;

#ifdef __cplusplus
}
#endif

#endif //_WIN_ADC_H_


