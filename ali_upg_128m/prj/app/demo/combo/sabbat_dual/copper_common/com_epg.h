/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: com_epg.h
*
*    Description: The common function of EPG
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _EPG_DEMO_H_
#define _EPG_DEMO_H_

#ifdef __cplusplus
extern "C"
{
#endif

//#include <api/libtsi/sec_eit.h>
#include <api/libsi/lib_epg.h>

//eit_event_info_t* epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type,date_time *dt,INT32 *event_num);
eit_event_info_t* epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type,date_time *start_dt,date_time *end_dt,\
    INT32 *event_num, BOOL update);
UINT8* epg_get_event_name(eit_event_info_t *ep,INT32* len);

INT32 epg_get_event_start_time(eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_end_time(eit_event_info_t *ep,UINT8* str);
UINT8 *epg_get_event_language(eit_event_info_t *ep);
UINT8 epg_get_event_language_count(eit_event_info_t *ep);
UINT8 epg_get_event_language_select(eit_event_info_t *ep);
void epg_set_event_langcode_select(UINT16 lang_code2);
INT32 epg_get_event_content_type(eit_event_info_t* ep,UINT8 *str,INT32 str_buf_size, INT32 *len);

UINT32 epg_get_event_all_short_detail(eit_event_info_t *ep,UINT16 *short_buf,UINT32 max_len);
UINT32 epg_get_event_all_extented_detail(eit_event_info_t *ep,UINT16 *extented_buf,UINT32 max_len);
/*
UINT8* epg_get_event_id(eit_event_info_t *ep,UINT8* str,INT32* len);
INT32 epg_get_event_detail_flag(eit_event_info_t *ep);
UINT8* epg_get_event_detail(eit_event_info_t *ep,INT32* len);
UINT8* epg_get_event_extented_detail(eit_event_info_t *ep,INT32 *len,INT32 idx);
UINT8* epg_get_event_short_detail(eit_event_info_t *ep,INT32 *len,INT32 idx);
INT32 epg_get_event_extented_desc_number(eit_event_info_t *ep);
INT32 epg_get_event_short_desc_number(eit_event_info_t *ep);
INT32 epg_get_event_start_time(eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_end_time(eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_time(int nType, eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_rating(eit_event_info_t* ep,UINT8* str,INT32* len);
INT32 epg_get_event_content_type(eit_event_info_t* ep,UINT8 *str,INT32* len);
INT32 epg_get_event_ca_id(eit_event_info_t *ep,UINT8 *str,INT32* len);
INT32 epg_get_event_component(eit_event_info_t *ep,UINT8 *str,INT32 *len);
*/

#ifdef __cplusplus
 }
#endif

#endif/*_EPG_DEMO_H_*/


