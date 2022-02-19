/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_epg.h
*
*    Description: EPG library
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __LIB_EPG_H__
#define __LIB_EPG_H__

#include <sys_config.h>
#include <types.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/si_config.h>

//libg_epg config
//#define EPG_MULTI_TP
//#define EPG_OTH_SCH_SUPPORT
//#define PARENTAL_SUPPORT

#if (defined(_MHEG5_SUPPORT_) || defined(_ISDBT_ENABLE_))
#define COMPONENT_SUPPORT
#define MAX_TEXT_CHAR_LEN  64
#define MAX_EPG_COMPONENT_COUNT 10
#define MAX_EPG_NIBBLES_NUM  4
#endif

//#define EPG_CACHE_ENABLE
//#define EPG_ONLY_FUTURE_EVENT
#ifndef SCHEDULE_TABLE_ID_NUM
#ifndef EPG_2DAYS
#define SCHEDULE_TABLE_ID_NUM   4 /*(0x50~0x53 or 0x60~0x63)*/
#else
#define SCHEDULE_TABLE_ID_NUM   1
#endif
#endif
#ifndef MAX_EPG_LANGUAGE_COUNT
    #define MAX_EPG_LANGUAGE_COUNT  3
#endif

#ifdef DTG_PVR
#ifndef MAX_EVENT_CRID_NUM
#define MAX_EVENT_CRID_NUM 4
#endif
#define EPG_MULTI_TP
#define EPG_OTH_SCH_SUPPORT  // with p/f update problem in DTG_PVR; need open "EPG_ONLY_OTH_SCH_SUPPORT"
#define EPG_ONLY_OTH_SCH_SUPPORT
#endif

//for most tp, 64 is enough. but some maybe > 64
#ifdef EPG_MULTI_TP
#ifdef DVBC_INDIA
#define TP_MAX_SERVICE_COUNT    300 //192
#else
#define TP_MAX_SERVICE_COUNT    192
#endif
#else
#define TP_MAX_SERVICE_COUNT    96  //64
#endif
#define MAX_ACTIVE_SERVICE_CNT  5
//end config

#ifdef _INVW_JUICE
typedef enum
{
    EIT_ALL = 0,                // Pid 0x12, table id 0x4E - 0x6F

    EIT_NOWNEXT,                // Pid 0x12, table id 0x4E - 0x4F
    EIT_NOWNEXT_ACTUAL,         // Pid 0x12, table id 0x4E
    EIT_NOWNEXT_OTHER,          // Pid 0x12, table id 0x4F

    EIT_SCHEDULE,               // Pid 0x12, table id 0x50 - 0x6F
    EIT_SCHEDULE_ACTUAL,        // Pid 0x12, table id 0x50 - 0x5F
    EIT_SCHEDULE_OTHER          // Pid 0x12, table id 0x60 - 0x6F
}EIT_TYPE;

#endif
enum NODE_STATUS
{
    NODE_FREE   = 0,
    NODE_ACTIVE = 1,
    NODE_DIRTY  = 2,
};

enum EPG_STATUS
{
    STATUS_NOT_READY   = 0,
    STATUS_OFF         = 1,
    STATUS_ON          = 2,
};

enum DB_EVENT_TYPE
{
    PRESENT_EVENT   = 0,
    FOLLOWING_EVENT = 1,
    SCHEDULE_EVENT  = 2,
    EPG_FULL_EVENT  = 0xFF
};

enum DB_EVENT_TYPE_MASK
{
    PRESENT_EVENT_MASK   = (0X01<<PRESENT_EVENT),
    FOLLOWING_EVENT_MASK = (0X01<<FOLLOWING_EVENT),
    SCHEDULE_EVENT_MASK  = (0X01<<SCHEDULE_EVENT),
};

#ifdef DTG_PVR
enum epg_search_condition
{
    SEARCH_BY_EVENT_ID = 1,  // start from 1 differ from normal epg OP:catch p/f and sch.
    SEARCH_BY_PROG_CRID_PARTLY,//ignore imi
    SEARCH_BY_PROG_CRID_FULL,
    SEARCH_BY_SERIES_CRID,
    SEARCH_BY_RECOMM_CRID,
};
#endif

struct TIME
{
    UINT8 hour;//: 5;      //0-23
    UINT8 minute;//: 6;    //0-59
    UINT8 second;//: 6;    //0-59
};// __attribute__((packed));

//epg event fixed length node
struct DB_NODE
{
#ifdef EPG_MULTI_TP
    UINT32 tp_id;
#endif
    UINT16 service_id;
    UINT16 event_id;
#if 1//def _PREFERRED_NAME_ENABLE_
    UINT8 name_id;
#endif

    UINT8 tab_num: 4;       //table_id & 0x0F
    UINT8 reserved: 4;

    UINT8 event_type: 2;    //DB_EVENT_TYPE
    UINT8 status: 2;        //NODE_STATUS

    UINT8  nibble: 4;       //content descriptor: content_nibble1

#ifdef PARENTAL_SUPPORT
//  struct EPG_MULTI_RATING
//  {
//      UINT8 country_code[3];  //ISO 3166
        UINT8 rating: 4;        //0-0xF, parent rating descriptor: rating & 0x0F
//  } parental[1];
#endif

#if (defined(COMPONENT_SUPPORT) || defined(_ISDBT_ENABLE_))
    UINT8 component_cnt;
    struct EPG_MULTI_COMPONENT
    {
	    UINT8 stream_content;    //Component descriptor
	    UINT8 component_type;       //
	    UINT8 component_tag;
	    UINT8 language_code[3];
	    UINT8 text_char[MAX_TEXT_CHAR_LEN];
    }comp[MAX_EPG_COMPONENT_COUNT];
#endif
#if (defined(_ISDBT_ENABLE_))
    UINT8 audio_com_tag;
#endif

    UINT16 mjd_num;
    struct TIME time;
    struct TIME duration;

//  UINT8  lang_cnt;
    struct EPG_MULTI_LANG
    {
        UINT8 *text_char;   //[event name] + [short event text char]
        UINT16 text_length;
//      UINT8 lang_code[3]; //ISO_639_language_code of event name, etc.
        UINT16 lang_code2;  //!!!NOTE: converted from 3 byte of ISO_639_language_code
#ifdef EPG_MULTI_TP
        UINT8 *ext_text_char; //[extend event text char]
        UINT16 ext_text_length;
#endif
    }lang[MAX_EPG_LANGUAGE_COUNT];
#ifdef DTG_PVR
    struct EPG_EIT_CID
    {
        UINT8 crid_type ;
        UINT8 crid_length ;
        UINT8 *crid_byte ;
    }eit_cid_desc[MAX_EVENT_CRID_NUM];
#endif
    /*
    {
        UINT8 event_name_length;
        UINT8 event_name[0];
        UINT8 short_text_length;
        UINT8 short_text_char[0];
        UINT16 extend_text_length;
        UINT8 extend_text_char[0];
    }
    */

#ifdef ISDBT_EPG
    UINT16 ref_service_id;
    UINT16 ref_event_id;
#endif

#ifdef _MHEG5_SUPPORT_ 
    UINT8 free_ca_mod;
    UINT8 nibble_cnt;
    UINT8 content_nibbles[MAX_EPG_NIBBLES_NUM];
#endif

} __attribute__((packed));

typedef struct DB_NODE eit_event_info_t;

//active service info
struct ACTIVE_SERVICE_INFO
{
    UINT32 tp_id;
    UINT16 service_id;
};

struct reserve_condition
{
    UINT32 tp_list[MAX_ACTIVE_SERVICE_CNT];
    INT32 tp_cnt;
    struct ACTIVE_SERVICE_INFO service_list[MAX_ACTIVE_SERVICE_CNT];
    INT32 service_cnt;
};

typedef void (*EIT_CALL_BACK)(UINT32 tp_id, UINT16 service_id, UINT8 event_type);

#define active_service_t    ACTIVE_SERVICE_INFO
#define api_epg_set_active_service  epg_set_active_service

#ifdef __cplusplus
extern "C" {
#endif

INT32 epg_init(UINT8 mode, UINT8* buf, UINT32 len, EIT_CALL_BACK call_back);
INT32 epg_release();
INT32 epg_reset();

void epg_on(UINT16 sat_id, UINT32 tp_id, UINT16 service_id);
void epg_on_ext(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id);
void epg_off();
void epg_off_ext();
enum EPG_STATUS epg_get_status();

//UINT32 epg_get_cur_tp_id();
//UINT16 epg_get_cur_service_id();

struct DB_NODE *epg_get_schedule_event(INT32 index);
struct DB_NODE *epg_get_service_event(UINT32 tp_id, UINT16 service_id, UINT8 event_type,
                            date_time *start_dt, date_time *end_dt, INT32 *num, BOOL update);

INT32 eit_compare_time(date_time *d1, date_time *d2);
date_time *get_event_start_time(struct DB_NODE *event, date_time *start_dt);
date_time *get_event_end_time(struct DB_NODE *event, date_time *end_dt);

UINT8* lang2tolang3(UINT16 lang_code2); //2 byte to 3 byte
void epg_api_set_select_time(date_time *cur_dt, date_time *start_dt, date_time *end_dt);
INT32 epg_set_active_service(struct ACTIVE_SERVICE_INFO *service, UINT8 cnt);
BOOL  epg_check_active_service(UINT32 tp_id, UINT16 service_id);
INT32 retrieve_eit_pf(UINT32 tp_id, UINT16 service_id);
UINT32 epg_get_cur_tp_id(void);
void pvr_epg_on(void *dmx, UINT16 service_id);
void epg_enter_mutex(void);
void epg_leave_mutex(void);

#ifdef _INVW_JUICE
void epg_init_inview();
void epg_on_by_inview(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id, EIT_TYPE type);
#endif

#ifdef __cplusplus
}
#endif

#endif //__LIB_EPG_H__

