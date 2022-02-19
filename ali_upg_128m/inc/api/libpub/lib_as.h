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
#ifndef _LIB_AS_H
#define _LIB_AS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/p_search.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libpub/lib_frontend.h>


#ifdef PUB_PORTING_PUB29
#define CHANCHG     0x02
#endif


/* description  : auto-scan signals for callback function from lib_nim.*/
#define AS_SGN_UNLOCKED     0
#define AS_SGN_LOCKED       1
#define AS_SGN_STOP         2
#define AS_SGN_INIT             3
#define AS_SGN_CRNUM        4
#define AS_SET_UNICABLE     5   //Unicable

/*description   : auto-scan callback report progress value with magic values.*/
#define AS_PROGRESS_SCANSTART       0
#define AS_PROGRESS_SCANOVER        100
#define AS_PROGRESS_STOFULL     150
#define AS_PROGRESS_NIT_VER_CHANGE  160
#define AS_PROGRESS_NIT         200
//progress=210 means star NIT search mode update
#define AS_PROGRESS_SEARCH_MODE_UPDATE          210
//progress=0xf0 means nit tp count
#define AS_PROGRESS_NIT_TP_CNT  240

/*description   : auto-scan sending message type.*/
#define ASM_TYPE_PROGRESS               0x00
#define ASM_TYPE_ADD_PROGRAM        0x01
#define ASM_TYPE_ADD_TRANSPONDER    0x02
#define ASM_TYPE_INC_TRANSPONDER    0x03
#define ASM_TYPE_SATELLITE              0x04
#define ASM_TYPE_NIT_VER_CHANGE 0x05
#define ASM_TYPE_SCANNING_INFO  0x06//SN for reporting message to UI when not locking
#define ASM_TYPE_INC_PROGRAM        0x07
#define ASM_TYPE_NIT_TP_CNT         0x08
#define ASM_TYPE_SEARCH_MODE_CHANGE     0x09


#define KU_LNB_22K_EDGE_FREQUENCY   11720


//#if ((SYS_PROJECT_FE == PROJECT_FE_DVBC) && defined(DVBC_COUNTRY_BAND_SUPPORT))
#define MAX_FRONTEND_PARAM  2
//#endif

enum
{
    AS_METHOD_NIT_TP_EX = -16,
#ifdef AUTO_OTA
    AS_METHOD_SSU_SEARCH        =-15,
#endif
    AS_METHOD_DATA      = -14,
    AS_METHOD_NVOD_MULTI_TP = -13,
    AS_METHOD_NVOD      = -12,
    AS_METHOD_MULTI_FFT     = -11,
    AS_METHOD_FREQ_BW   = -10,
    AS_METHOD_MULTI_NIT = -9,
    AS_METHOD_MULTI_TP  = -8,
    AS_METHOD_NIT_TP        = -7,
    AS_METHOD_NIT           = -6,
    AS_METHOD_TP_PID        = -5,
    AS_METHOD_SAT_TP        = -4,
    AS_METHOD_TP            = -3,
    AS_METHOD_FFT           = -2,
    AS_METHOD_TRADITON  = -1,
    AS_METHOD_FIXSCAN    = 0,
};


/*description   : auto-scan message encoder/decoder data structure.*/
union as_message
{
    struct
    {
        UINT32 asm_type : 8;    /* should be ASM_TYPE_xxxx */
        UINT32 asm_value : 24;
    } decode;
    UINT32 msg_encode;
};

/* description  : update event method types.
 * parameter    : 2
 *aue_type: see ASM_TYPE_xxx
 *aue_value:    the event value code.
 * comment  : this function will be provided by application!
 */
typedef void (*as_update_event_t)(UINT8 aue_type, UINT32 aue_value);


union as_frontend_param
{
    struct
    {
        UINT8 constellation;
        UINT32 sym;
    } c_param;
    struct
    {
        UINT32  bandwidth;
        UINT8   band_type;
    } t_param;
};



struct as_service_param
{
    UINT8 as_frontend_type;
    UINT8 reserve;
    UINT32 as_prog_attr;//P_SEARCH_FTA or P_SEARCH_ALL
    UINT8 as_p_add_cfg;
    INT32 as_method;// see AS_METHOD_xxx

    UINT16 as_sat_cnt;
    UINT32 as_from;
    UINT32 as_to;
    UINT16 as_pid_list[3];
    as_update_event_t as_handler;
    UINT16 sat_ids[256];

#ifdef COMBOUI
    union as_frontend_param ft[MAX_FRONTEND_PARAM];
    /*pointer array to store the parameters of a group of continuous bands.*/
    band_param  *band_group_param;//[MAX_BAND_COUNT];
    UINT8   scan_cnt;
    UINT8 ft_count;
#else


#if ((SYS_PROJECT_FE == PROJECT_FE_DVBC) && defined(DVBC_COUNTRY_BAND_SUPPORT))
    union as_frontend_param ft[MAX_FRONTEND_PARAM];
    UINT8 ft_count;
#else
    union as_frontend_param ft;
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) \
    || ((SYS_PROJECT_FE == PROJECT_FE_DVBC) && defined(DVBC_COUNTRY_BAND_SUPPORT))
    //[MAX_BAND_COUNT];   /*pointer array to store the parameters of a group of continuous bands.*/
    band_param  *band_group_param;
    UINT8   scan_cnt;
#endif
#endif

#ifdef MULTISTREAM_SUPPORT
    UINT8 ms_enable;    // if multistream search enable: 0:diable, 1:enable
    UINT8 scan_type;    // 0: normal (signle stream) scan; 1: multistream scan
#endif

#ifdef PLSN_SUPPORT
    UINT8 super_scan;
    UINT32 pls_num;
#endif
};



/*description   : register auto-scan service task, but not run it.*/
INT32 as_service_register(void);
/*description   : start an auto-scan according to the parameter asp.*/
INT32 as_service_start(struct as_service_param *asp);
/* description  : stop an on-going auto-scan.*/
INT32 as_service_stop(void);
INT32 as_service_query_stat(void);
//void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
BOOL as_prog_pid_search(struct dmx_device *dmx,UINT8 av_mode,
    UINT16 v_pid, UINT16 a_pid,UINT16 pcr_pid);

#ifdef AUTO_OTA
INT32 as_otasearch_service_stop(BOOL b_blocking);
#endif

void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);

#ifdef __cplusplus
}
#endif

#endif /* _LIB_AS_H */
