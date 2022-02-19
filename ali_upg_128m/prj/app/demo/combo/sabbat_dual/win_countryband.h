/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_countryband.h
*
*    Description: The realize of country setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_COUNTRYBAND_H_
#define _WIN_COUNTRYBAND_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <api/libosd/osd_lib.h>

extern void get_default_bandparam_qamb(UINT8 index, band_param *buf);

extern PRESULT comlist_country_net_callback(POBJECT_HEAD pobj,\
        VEVENT event, UINT32 param1, UINT32 param2);

extern UINT8 win_country_net_get_num(void);
extern void win_contry_net_set_entry(BOOL auto_scan);
extern BOOL from_auto_scan ;
extern CONTAINER item_country_con;
extern CONTAINER item_network_con;
extern CONTAINER item_button_prev_con;
extern CONTAINER item_button_next_con;

extern TEXT_FIELD item_country_label;
extern TEXT_FIELD item_network_label;

extern TEXT_FIELD item_button_prev_label;
extern TEXT_FIELD item_button_next_label;

extern MULTISEL item_country_sel;
extern MULTISEL item_network_sel;



#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

