/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_as.h
*
* Description:
*     auto-scan service implementation for ali chip based IRD.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
 #include <sys_config.h>
#ifdef PUB_PORTING_PUB29


#include <api/libpub/lib_as.h>

#else

#ifndef _LIB_AS_H
#define _LIB_AS_H

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)
#include <api/libtsi/atsc_search.h>
#else
#include <api/libtsi/p_search.h>
#endif
#ifndef SYS_PSEARCH_SCAN_TP_ALL
#define SYS_PSEARCH_SCAN_TP_ALL        SYS_FUNC_OFF
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) ||(SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
//usage_type
//for DVBT, SN 20050524
#define AUTOSCAN    0x00
#define CHANSCAN    0x01
#define CHANCHG        0x02
#define AERIALTUNE    0x03    //50914-01Angus

UINT8 pid_search_check(PROG_INFO *p_info);

#endif

#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)
enum
{
    PROG_SYS_ATSC,
    PROG_SYS_CABLE_STD,
    PROG_SYS_CABLE_HRC,
    PROG_SYS_CABLE_IRC,
    PROG_FAC_ATSC,
    PROG_SYS_NTSC,
};
#endif

enum
{
#ifdef AUTO_OTA
    AS_METHOD_SSU_SEARCH        =-15,
#endif
    AS_METHOD_DATA        = -14,
    AS_METHOD_NVOD_MULTI_TP    = -13,
    AS_METHOD_NVOD        = -12,
    AS_METHOD_MULTI_FFT     = -11,
    AS_METHOD_FREQ_BW    = -10,
    AS_METHOD_MULTI_NIT    = -9,
    AS_METHOD_MULTI_TP    = -8,
    AS_METHOD_NIT_TP        = -7,
    AS_METHOD_NIT            = -6,
    AS_METHOD_TP_PID        = -5,
    AS_METHOD_SAT_TP        = -4,
    AS_METHOD_TP            = -3,
    AS_METHOD_BLIND        = -2,
    AS_METHOD_FFT            = -2,
    AS_METHOD_TRADITON    = -1,
    AS_METHOD_FIXSCAN     = 0,
};


/* description    : auto-scan signals for callback function from lib_nim.*/
#define AS_SGN_UNLOCKED     0
#define AS_SGN_LOCKED       1
#define AS_SGN_STOP            2
#define AS_SGN_INIT             3
#define AS_SGN_CRNUM        4
#define AS_SET_UNICABLE        5    //Unicable

/*description    : auto-scan callback report progress value with magic values.*/
#define AS_PROGRESS_SCANSTART        0
#define AS_PROGRESS_SCANOVER        100
#define AS_PROGRESS_STOFULL        150
#define AS_PROGRESS_NIT_VER_CHANGE    160
#define AS_PROGRESS_NIT            200

/*description    : auto-scan sending message type.*/
#define ASM_TYPE_PROGRESS               0x00
#define ASM_TYPE_ADD_PROGRAM        0x01
#define ASM_TYPE_ADD_TRANSPONDER    0x02
#define ASM_TYPE_INC_TRANSPONDER    0x03
#define ASM_TYPE_SATELLITE              0x04
#define ASM_TYPE_NIT_VER_CHANGE    0x05
#define ASM_TYPE_SCANNING_INFO    0x06//SN for reporting message to UI when not locking
#define ASM_TYPE_INC_PROGRAM        0x07
#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)
#define ASM_TYPE_DUP_PROGRAM            0x08
#define ASM_TYPE_SMANT_SEEKING            0x09
#define ASM_TYPE_COMMIT_DEL             0x0A
#endif

#define KU_LNB_22K_EDGE_FREQUENCY    11700



/*description    : auto-scan message encoder/decoder data structure.*/
union as_message
{
    struct
    {
        UINT32 asm_type : 8;    /* should be ASM_TYPE_xxxx */
        UINT32 asm_value : 24;
    } decode;
    UINT32 msg_encode;
};

/* description    : update event method types.
 * parameter    : 2
 *aue_type:    see ASM_TYPE_xxx
 *aue_value:    the event value code.
 * comment    : this function will be provided by application!
 */
typedef void (*as_update_event_t)(UINT8 aue_type, UINT32 aue_value);


#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
struct as_service_param
{
    struct nim_device *nim;
    struct dmx_device *dmx;
    UINT16 as_sat_id;    /* satellite ID */
    UINT8 as_prog_attr;    /* P_SEARCH_FTA or P_SEARCH_ALL */
    UINT8 as_p_add_cfg;
    INT32 as_method;    /* see AS_METHOD_xxx */
    UINT32 as_from;
    UINT32 as_to;
    UINT16 as_pid_list[3];
    UINT16 *as_sat_ids;

    as_update_event_t as_handler;
};
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBC)
struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
    UINT32 as_prog_attr;
    UINT8 as_p_add_cfg;
    INT32 as_method;    /* see AS_METHOD_xxx */
    UINT32 as_from;
    UINT32 as_to;
    UINT16 as_pid_list[3];
    UINT32 as_sym;        /* for DVB-C */
#ifdef DVBC_AUTOSCAN_MULTI_SYMBOL
    UINT32 as_sym2;        //for DVB-C India
    UINT32 as_sym3;        //for DVB-C India
 #endif
    UINT8 as_constellation;    /* for DVB-C */
    as_update_event_t as_handler;
};
#elif(SYS_PROJECT_FE == PROJECT_FE_ATSC)
struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
#if 0 //(SYS_PROJECT_FE == PROJECT_FE_DVBT)  //atsc_pvr
    UINT8 as_prog_attr;    /* P_SEARCH_FTA or P_SEARCH_ALL */
#endif
    /*PROG_SYS_ATSC or PROG_SYS_CABLE_STD or PRG_SYS_CABLE_HRC or PRG_SYS_CABLE_IRC or PROG_SYS_NTSC*/
    UINT8 as_prog_sys;
    UINT8 as_p_add_cfg;    /* see db_3l.h PROG_ADD_xxxx for detail.*/
                 /* PROG_ADD_REPLACE_OLD          : new replace old.
                 * PROG_ADD_DIRECTLY          : add directly.
                 * PROG_ADD_PRESET_REPLACE_NEW    : preset replace new.
                 * MARCOs are strongly recommended.
                 */
    INT32 as_method;    /* see AS_METHOD_xxx */
    UINT32 as_from;        /* low frequency value, default is 950 */
    UINT32 as_to;        /* high frequency value, default is 2150 */
                /* also for DVB-C network search(1) or not(0) */
    UINT16 as_pid_list[3];
    band_param    *band_group_param;    /*pointer array to store the parameters of a group of continuous bands.*/
    UINT8    scan_cnt;            /*the total group number of continuous bands*/
    /*bandwidth of a channel in DVB-T system, input by UI according to the country selected*/
    UINT32    bandwidth;
    UINT8    band_type;            /*band_type parsed from *band_group_param bit 31*/
    BOOL    smtenna_enable;
    as_update_event_t as_handler;
};
#else
//for DVBT
struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
    UINT32 as_prog_attr;    /* P_SEARCH_FTA or P_SEARCH_ALL */
    UINT8 as_p_add_cfg;
    INT32 as_method;    /* see AS_METHOD_xxx */
    UINT32 as_from;
    UINT32 as_to;
    UINT16 as_pid_list[3];
    band_param    *band_group_param;/*pointer array to store the parameters of a group of continuous bands.*/
    UINT8    scan_cnt;            /*the total group number of continuous bands*/
    UINT32    bandwidth;        /*bandwidth of a channel in DVB-T system, input by UI according to the country selected*/
    UINT8    band_type;        /*band_type parsed from *band_group_param bit 31*/
    as_update_event_t as_handler;
};
#endif

#ifdef BASE_TP_HALF_SCAN_SUPPORT

struct half_scan_param
{
    UINT32 time_out;
    UINT8 as_frontend_type; //FRONTEND_TYPE_S FRONTEND_TYPE_C FRONTEND_TYPE_T
    UINT32 sat_id; //1 for T&C
    UINT32 tp_id; //basal tp already exists in database before search
    as_update_event_t as_event; //notify event
};

#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*description    : register auto-scan service task, but not run it.*/
INT32 as_service_register();
/*description    : start an auto-scan according to the parameter asp.*/
INT32 as_service_start(struct as_service_param *asp);
/* description    : stop an on-going auto-scan.*/
INT32 as_service_stop();
INT32 as_service_query_stat();
BOOL as_prog_pid_search(UINT8 av_mode,UINT16 v_pid, UINT16 a_pid,UINT16 pcr_pid);

#ifdef AUTO_OTA
INT32 as_otasearch_service_stop(BOOL b_blocking);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LIB_AS_H */

#endif
