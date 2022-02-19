/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: epg_common.h
*
*    Description: include common macro and parameter declaration
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _EPG_COMMON_H_
#define _EPG_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <api/libc/printf.h>
#include <api/libsi/si_tdt.h>

#define EPG_PRINT               PRINTF
#define EIT_PRINT               PRINTF
#define DB_PRINT                PRINTF
#define X_PRINT                 PRINTF
#define MSG_PRINT               PRINTF

//#define SCHEDULE_TABLE_ID_NUM     4 /*(0x50~0x53 or 0x60~0x63)*/
#define SCHEDULE_DAY            (SCHEDULE_TABLE_ID_NUM*4)

#define SI_EIT_SECTION_LEN      4096
//#define PSI_EIT_PID               0x0012

#define EPGDB_FLAG_MUTEX        0x80000000UL
#define ALLOC_FLAG_MUTEX        0x40000000UL
#define EPG_FLAG_MUTEX          0x20000000UL

#define SI_EIT_TABLE_ID_APF     0x4E
#define SI_EIT_TABLE_ID_OPF     0x4F

#define SI_EIT_TABLE_ID_ASCH    0x50
#define SI_EIT_TABLE_ID_OSCH    0x60

#define IS_SCH(tid)         ((SI_EIT_TABLE_ID_ASCH == (tid&0xf0))||(SI_EIT_TABLE_ID_OSCH == (tid&0xf0)))
#define IS_OTH_SCH(tid)     (SI_EIT_TABLE_ID_OSCH == (tid&0xf0))
#define IS_PF(tid)          ((SI_EIT_TABLE_ID_APF == tid)||(SI_EIT_TABLE_ID_OPF == tid))
#define IS_OTH_PF(tid)      (SI_EIT_TABLE_ID_OPF == tid)

#ifdef DTG_PVR
#define PROG_CRID       0x31
#define SERIES_CRID     0x32
#define RECOMM_CRID     0x33

#define PROG_CRID_MASK   0x1
#define SERIES_CRID_MASK 0x2
#define RECOMM_CRID_MASK 0x4
#endif

enum EPG_XMEM_STATUS
{
    STATUS_AVAILABLE   = 0,
    STATUS_UNAVAILABLE = -1,
};
//event condition struct for creating view
//example:
//1. if need create view including present & following events, set event_mask to be
//  (PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK)
//2. if need create view of schedule events, set event_mask to be
//  SCHEDULE_EVENT_MASK
struct view_condition
{
    UINT32 tp_id;
    UINT16 service_id;
    UINT8  event_mask;  //PRESENT_EVENT_MASK,FOLLOWING_EVENT_MASK,SCHEDULE_EVENT_MASK

#ifdef DTG_PVR
    UINT8 search_condition;
    UINT16 event_id;
    struct SEARCH_EVENT_CRID
    {
        UINT8 crid_length;
        UINT8 crid_byte[32];
    }event_crid[2];
#endif

    date_time start_dt; //time restriction for sch events
    date_time end_dt;
};

#ifdef __cplusplus
}
#endif

#endif

