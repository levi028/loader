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

#include <types.h>
#include <sys_config.h>

#include <api/libpub27/lib_chchg.h>

#include <api/libtsi/db_3l.h>
#if( SYS_PROJECT_FE == PROJECT_FE_ATSC )
#include <api/libtsi/atsc_search.h>
#else
#include <api/libtsi/p_search.h>
#endif
#ifdef    ali_s00_m3330_model2
#define    SCAN_ALL_TP_START        0x00000001
#define    SCAN_ALL_TP_STOP        0x00000010
#define    SCAN_ALL_TP_EXIT        0x00000100
#define    SCAN_ALL_TP_WAIT        0x00001000
#define    PARSING_PROG_START        0x00001000
#endif


#ifndef SYS_PSEARCH_SCAN_TP_ALL
#define SYS_PSEARCH_SCAN_TP_ALL        SYS_FUNC_OFF
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) ||(SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
//usage_type
//for DVBT, SN 20050524
#define AUTOSCAN            0x00
#define CHANSCAN            0x01
#define CHANCHG                0x02
#define AERIALTUNE            0x03    //50914-01Angus

enum
{
    PROG_SYS_ATSC,
    PROG_SYS_CABLE_STD,
    PROG_SYS_CABLE_HRC,
    PROG_SYS_CABLE_IRC,
    PROG_FAC_ATSC,
    PROG_SYS_NTSC,
};

UINT8 pid_search_check(PROG_INFO *p_info);

#ifdef DUPLICATE_CHANNEL_CHECK // hbchen for french spec (duplicate service and same lcn handle)
#ifdef DUP_CHAN_CMP
#if (DUP_CHAN_CMP == DUP_CHAN_CMP_BER) // GMI John pad that for using
#define LCN_VAL_TYPE    UINT32
#define LCN_VAL_ERR     0xFFFFFFFF
#elif (DUP_CHAN_CMP == DUP_CHAN_CMP_SNR)
#define LCN_VAL_TYPE    UINT8
#define LCN_VAL_ERR     0xFF
#elif (DUP_CHAN_CMP == DUP_CHAN_CMP_AGC)
//#define LCN_VAL_TYPE    UINT16
#define LCN_VAL_TYPE    UINT8
//#define LCN_VAL_ERR     0xFFFF
#define LCN_VAL_ERR     0xFF
#else // original code setting
#define LCN_VAL_TYPE    UINT8
#define LCN_VAL_ERR     0xFF
#endif // #if (DUP_CHAN_CMP == DUP_CHAN_CMP_BER)
#else//#ifdef DUP_CHAN_CMP
#define LCN_VAL_TYPE    UINT8
#define LCN_VAL_ERR     0xFF
#endif//#ifdef DUP_CHAN_CMP
#endif // #ifdef DUPLICATE_CHANNEL_CHECK

#endif //#if( SYS_PROJECT_FE == PROJECT_FE_DVBT )



/*
 * name        : AS_METHOD_xxx <enum>
 * description    : auto-scan service method.
 *          any value >=0 means fix-scan's starting index value.
 */
enum
{
    AS_METHOD_NVOD            = -12,
    AS_METHOD_MULTI_FFT         = -11,

    AS_METHOD_FREQ_BW        = -10,

    AS_METHOD_MULTI_NIT        = -9,
    AS_METHOD_MULTI_TP        = -8,
    AS_METHOD_NIT_TP        = -7,
    AS_METHOD_NIT            = -6,
    AS_METHOD_TP_PID        = -5,
    AS_METHOD_SAT_TP        = -4,
    AS_METHOD_TP            = -3,
    AS_METHOD_BLIND            = -2,
    AS_METHOD_FFT            = -2,
    AS_METHOD_TRADITON        = -1,
    AS_METHOD_FIXSCAN         = 0, /* or any value that >0 */
};

/*
 * name        : AS_SGN_xxx <marco set>
 * description    : auto-scan signals for callback function from lib_nim.
 */
enum
{
    AS_SGN_UNLOCKED            = 0,
    AS_SGN_LOCKED            = 1,
    AS_SGN_STOP            = 2,
    AS_SGN_INIT            = 3,
    AS_SGN_CRNUM            = 4,
};

/*
 * name        : AS_PROGRESS_xxx
 * description    : auto-scan callback report progress value with magic values.
 */
enum
{
    AS_PROGRESS_SCANSTART        = 0,
    AS_PROGRESS_SCANOVER        = 100,
    AS_PROGRESS_STOFULL        = 150,
    AS_PROGRESS_NIT_VER_CHANGE    = 160,
    AS_PROGRESS_NIT            = 200,
};

/*
 * name        : ASM_TYPE_xxx
 * description    : auto-scan sending message type.
 */
enum
{
    ASM_TYPE_PROGRESS        = 0xA500,
    ASM_TYPE_ADD_PROGRAM        = 0xA501,
    ASM_TYPE_ADD_TRANSPONDER    = 0xA502,
    ASM_TYPE_INC_TRANSPONDER    = 0xA503,
    ASM_TYPE_SATELLITE        = 0xA504,
    ASM_TYPE_NIT_VER_CHANGE        = 0xA505,

    ASM_TYPE_SCANNING_INFO        = 0xA506,//SN for reporting message to UI when not locking

    ASM_TYPE_INC_PROGRAM        = 0xA507,
    ASM_TYPE_DUP_PROGRAM        = 0xA508,
    ASM_TYPE_SMANT_SEEKING    = 0xA509,
    ASM_TYPE_COMMIT_DEL    = 0xA50A,
};

enum
{
    KU_LNB_22K_EDGE_FREQUENCY    = 11700,
};

#if (SYS_SDRAM_SIZE>2) && (SYS_PROJECT_FE != PROJECT_FE_ATSC)
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
#define AS_SUPPORT_MULTISAT
#endif
#define AS_SUPPORT_NIT
#define AS_SUPPORT_PRESET
#endif

/*
 * name        : as_message
 * description    : auto-scan message encoder/decoder data structure.
 */
union as_message
{
    struct
    {
        UINT16 asm_type;    /* should be ASM_TYPE_xxxx */
        UINT16 asm_value;
    } decode;
    UINT32 msg_encode;
};

/*
 * name        : as_update_event_t <typedef>
 * description    : update event method types.
 * parameter    : 2
 *    IN    UINT16 aue_type:    see ASM_TYPE_xxx
 *    IN    UINT16 aue_value:    the event value code.
 * return value : void.
 * comment    : this function will be provided by application!
 */
typedef void (*as_update_event_t)(UINT16 aue_type, UINT32 aue_value); //ATSC

/*
 * name        : as_service_param <struct>
 * description    : service parameter for starting an auto-scan
 * parameter    : 4
 *    UINT16    as_sat_id:    the satellite ID, use for LNB&DeSEqC information.
 *
 *    INT16 as_method:    the auto-scan method.
 *                - AS_METHOD_FFT    : FFT based spectrum fastscan.
 *                - AS_METHOD_TRAD: traditional auto-scan.
 *                - value >= 0    : fixscan starting index.
 *
 *    INT16 as_index:        - reserved for AS_METHOD_FFT & AS_METHOD_TRAD
 *                - fixscan ending index (not including)
 *    as_service_update_t
 *      as_update_callback    - callback function to update UI progress &/|
 *                  new transponder detection.
 */
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
    UINT8 as_prog_attr;    /* P_SEARCH_FTA or P_SEARCH_ALL */
    UINT8 as_p_add_cfg;    /* see db_3l.h PROG_ADD_xxxx for detail.*/
                 /* PROG_ADD_REPLACE_OLD          : new replace old.
                 * PROG_ADD_DIRECTLY          : add directly.
                 * PROG_ADD_PRESET_REPLACE_NEW    : preset replace new.
                 * MARCOs are strongly recommended.
                 */
    INT32 as_method;    /* see AS_METHOD_xxx */
    UINT16 as_from;        /* low frequency value, default is 950 */
    UINT16 as_to;        /* high frequency value, default is 2150 */
                /* also for DVB-C network search(1) or not(0) */
    UINT16 as_pid_list[3];
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
    UINT32 as_sym;        /* for DVB-C */
    UINT8 as_constellation;    /* for DVB-C */
#endif
#ifdef AS_SUPPORT_MULTISAT
    UINT16 *as_sat_ids;
#endif
    as_update_event_t as_handler;
};
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBC)
struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
    UINT16 as_prog_attr;    /* P_SEARCH_FTA or P_SEARCH_ALL */
    UINT16 as_p_add_cfg;    /* see db_3l.h PROG_ADD_xxxx for detail.*/
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
    UINT32 as_sym;        /* for DVB-C */
    UINT8 as_constellation;    /* for DVB-C */

    as_update_event_t as_handler;
};
#else

#ifdef SUPPORT_FAST_FFT
typedef enum
{
    NORMAL_SCAN = 0x01,
    FAST_SCAN,
} FAST_SCAN_MODE;
#endif

struct as_service_param
{
    UINT16 as_sat_id;    /* satellite ID */
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
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
//#if(SYS_PROJECT_FE == PROJECT_FE_DVBT)  //SN for DVB-T
    /*pointer array to store the parameters of a group of continuous bands.*/
    band_param    *band_group_param;//[MAX_BAND_COUNT];
    UINT8    scan_cnt;            /*the total group number of continuous bands*/
    /*bandwidth of a channel in DVB-T system, input by UI according to the country selected*/
    UINT32    bandwidth;
    UINT8    band_type;            /*band_type parsed from *band_group_param bit 31*/
//#endif
//#ifdef SMART_ANT_SUPPORT
    BOOL    smtenna_enable;
//#endif
    as_update_event_t as_handler;
#ifdef SUPPORT_FAST_FFT
    FAST_SCAN_MODE fast_scan_mode;
#endif

};
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/*
 * name        : as_service_register
 * description    : register auto-scan service task, but not run it.
 * parameter    : 0
 * return value    : INT32
 *    SUCCESS    : the auto-scan task is created.
 *    other    : the auto-scan task could not be created. (already created?)
 */
INT32 as_service_register();

#if 0
/*
 * name        : as_service_unregister
 * description    : un-register auto-scan service task.
 * parameter    : 0
 * return value : INT32
 *    SUCCESS    : the auto-scan task is deleted.
 *    other    : the auto-scan task could not be deleted. (not created or has been running)
 */
INT32 as_service_unregister();
#endif

/*
 * name        : as_service_start
 * description    : start an auto-scan according to the parameter asp.
 *          see <as_service_param for detail>
 * parameter    : 1
 *    IN    struct as_service_param *asp;
 * return value    : INT32
 *    SUCCESS    : the auto-scan task is started.
 *    other    : the auto-scan task failed to start. (not created or has been running)
 * notice    : this function is non-block.
 */
INT32 as_service_start(struct as_service_param *asp);

/*
 * name        : as_service_save_tmp
 * description    : save tmp information.
 * parameter    : 3
 *    IN    UINT8 *data:    the content.
 *    IN    UINT32 len:    content length.
 *    IN    INT32 block:    the saving should work in block mode (!=0 )or not(0).
 * return value : INT32
 *    SUCCESS    : the tmp information is saved.
 *    other    : the data is not saved.
 */
INT32 as_service_save_tmp(UINT8 *data, UINT32 len, INT32 block);

/*
 * name        : as_service_stop
 * description    : stop an on-going auto-scan.
 * parameter    : 1
 *    IN    as_service_event_t as_stop_event: - NULL block call.
 *                             - other non-block call.
 * notice    : if the as_stop_event is not NULL, it will be called when
 *          the autoscan really stopped.
 */
INT32 as_service_stop(/*as_service_event_t as_stop_event*/);

//hbchen 20060912 modify for french spec (duplicate service and same lcn handle)
#ifdef DUPLICATE_CHANNEL_CHECK
//void set_signal_quality(UINT32 tp_id, UINT8 ber);//old
//UINT8 get_signal_quality(UINT32 tp_id);//old
void free_signal_table();
void set_signal_quality(UINT32 tp_id, LCN_VAL_TYPE ber);
LCN_VAL_TYPE get_signal_quality(UINT32 tp_id);
#endif

#ifdef    ali_s00_m3330_model2
INT32 as_tp_scan_stop();
INT32 as_tp_scan_exit();
#endif

/*
 * name        : sat2antenna
 * description    : transfer sat parm to antenna struct.
 * parameter    : 3
 *    IN    S_NODE *sat:    the sat pointer that the prog belongs to.
 *    OUT    antenna:    frontend param.
 * return value : void
 */
void sat2antenna(S_NODE *sat, struct cc_antenna_info *antenna);

#ifdef __cplusplus
}
#endif

#endif /* _LIB_AS_H */
