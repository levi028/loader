/** @file       sat2ip.h
 *  @brief      Define datatypes that SAT2IP server module used, and declare APIs.
 *
 *  @copyright  ALi Corporation, All Rights Reserved.
 */

#ifndef _ALI_SAT2IP_H_
#define _ALI_SAT2IP_H_
#include <types.h>
#include <api/libhttpserver/http_server.h>

/**
 *  @brief  Define Demodulator type that SAT2IP support.
 */
typedef enum
{
    SAT2IP_DVBS_DEMODULATOR = 0,
    SAT2IP_DVBC_DEMODULATOR = 1,
    SAT2IP_DVBT_DEMODULATOR = 2,
    SAT2IP_ISDBT_DEMODULATOR = 3,
    SAT2IP_DEMODULATOR,
}DEMO_TYPE;

/**
 *  @brief  Define the type that STB filter stream from DMX.
 */
typedef enum
{
    /** Filter stream by program, means the whole service will filter out. */
    SAT2IP_PROGRAM_RECORD = 0,
    /** Filter stream by PID list, default support this type. */
    SAT2IP_PID_RECORD,
    /** Whole TP stream will filter out. */
    SAT2IP_TP_RECORD,
    /** MAX value*/
    SAT2IP_RECORD_MAX,
}STREAM_RECORD_TYPE;

/**
 *  @brief  Define messages that libprovider send to AP layer.
 */
typedef enum
{
    /** Message that libprovider call filter one program. ####### SHOULD BE DELETED*/
    SAT2IP_ACTION_REQUEST_REC_PROGRAM = 0,
    /** Message that libprovider send filter out whole TP.*/
    SAT2IP_ACTION_REQUEST_REC_TP,
    /** Message that libprovider send filter by PID list. */
    SAT2IP_ACTION_REQUEST_REC_PID,
    /** #########SHOULD BE DELETED*/
    SAT2IP_ACTION_STOP_RECORD,
    /** Message that libprovider send, request all channel list. */
    SAT2IP_ACTION_REQUEST_DB_ALL,
    /** Message that libprovider send, abort channel list transport. */
    SAT2IP_ACTION_DB_ABORT,
}SAT2IP_ACTION_TYPE;


/**
 *  @brief  Define error code that provider set.
 */
typedef enum
{
    /** No error encounter. */
    SAT2IP_RET_OK       = 0,
    /** Unknown error. */
    SAT2IP_RET_UNKNOWN  = -1,
    /** Error. */
    SAT2IP_RET_ERROR    = -2,
    /** TP not exist or TP number error. */
    SAT2IP_RET_TPERROR  = -3,
    /** Satellite ID error. */
    SAT2IP_RET_SATERROR = -4,
    /** PID error. */
    SAT2IP_RET_PIDERROR = -5,
    /** Src ID error. */
    SAT2IP_RET_SRC_ERROR = -6,
    /** Hardware resource not enough. */
    SAT2IP_RET_SRC_LACK  = -7,
    /** Tuner resource not enough. */
    SAT2IP_RET_TUNER_LACK  = -8,
    /** Crypto resource not enough. */
    SAT2IP_RET_CRYPT_LACK  = -9,
    /** Connection is over max limit. */
    SAT2IP_RET_MAX_LIMIT  = -10,
}RETURN_REQUEST;

/**
 *  @brief  Modulation system ID.
 */
enum sat2ip_msys {
    msys_unknown = 0,
    msys_dvbs = 1,
    msys_dvbs2 = 2,
    msys_dvbt = 3,
    msys_dvbt2 = 4,
    msys_dvbc = 5,
    msys_isdbt = 6,
    msys_max,
};

/**
 *  @brief  Bandwide id.
 */
enum sat2ip_bw {
    bw_unknown = 0,
    bw_6m = 1,
    bw_7m = 2,
    bw_8m = 3,
};

/**
 *  @brief  DVB-T2 SISO, MISO id.
 */
enum sat2ip_sm {
    sm_siso = 0,    /**< SISO mode. */
    sm_miso = 1,    /**< MISO mode. */
};

/**
 *  @brief  Modulation type ID, DVBT/C ISDBT use.
 */
enum sat2ip_mtype {
    mtype_unknown = 0,
    mtype_qpsk = 0x10,
    mtype_8psk = 0x11,
    mtype_4qam = 0x12,
    mtype_16qam= 0x13,
    mtype_32qam= 0x14,
    mtype_64qam= 0x15,
    mtype_128qam= 0x16,    
    mtype_256qam= 0x17,
};

/**
 *  @brief  DVBS/S2 Transponder polarisation ID.
 */
enum sat2ip_polarisation {
    polar_unknown = 0,
    polar_horizontal = 0x20,
    polar_vertical   = 0x21,
    polar_circular_left = 0x22,
    polar_circular_right = 0x23,
};

/**
 *  @brief  DVBS2 pilot signal on off.
 */
enum sat2ip_pilots {
    pilots_unknown = 0,
    pilots_off = 0x30,
    pilots_on = 0x31,
};


/**
 *  @brief  Satellite information from request structure.
 */
struct sat2ip_sat
{
    INT32 src;          /**< Signal source number, src attribute's value in request QUERY. */
    INT32 fe;           /**< frontend number, fe attribute's value in request QUERY. */
    INT32 sat_index;    /**< Satellite ID, alisatid attribute's value in request QUERY. */
    BOOL  is_ali_client;/**< ALi client flag, server will set this flag. */
};

/**
 *  @brief  Transponder parameter from request.
 */
struct sat2ip_tp
{
    INT32 freq;         /**< Frequency of TP. */
    INT32 pol;          /**< Polarisation parameter of TP, DVBS/S2 use. */
    INT32 symb_rate;    /**< Symbol rate of TP, DVBS/S2 use. */
    DEMO_TYPE dm_type;  /**< Demodulator type. */
    char  mod_way;      /**< Modulator type value. */
    INT8 is_t2;         /**< T2 flag. */
    INT8 t2_system_id;  /**< T2 system ID. */
    INT8 plp_id;        /**< T2 PLP ID. */
    UINT32 tp_id;       /**< T2 TP ID. */
};


/** 
 *  @brief      Start RTSP server for SAT2IP/PVR2IP.
 *  @details    This server only for SAT2IP/PVR2IP, can not streaming USB disk file.
 *              Use port 554, and not support Multicast, HTTP-interleaved transport.
 */
void rtsp_server_start(void);

/** 
 *  @brief      Stop RTSP server running.
 *  @details    It will stop RTSP server running, and exit.
 */
void rtsp_server_stop(void);

/** 
 *  @brief      Just set RTSP server stop accept new connection.
 *  @details    Server still running, but not accept new session.
 */
void rtsp_server_pause(void);

/** 
 *  @brief      Resume running status from PAUSE.
 *  @details    Server resume from PAUSE status, accept new connection request again.
 */
void rtsp_server_resume(void);

/** 
 *  @brief      Disconnect rtsp streaming session.
 *  @details    In some cases STB resource not available, streaming source cut-off,
 *              rtsp streaming session also need disconnect.
 *  @param[in]  type    0: All types session; 1: SAT2IP session; 2: PVR2IP session.
 *  @return     0: Success, Failure when other values.
 */
int  rtsp_session_disconnect_by_type(int type);

/** 
 *  @brief      Start HTTP server for SAT2IP/PVR2IP.
 *  @details    This HTTP server use for get channel list(pvr list) from STB, and also
 *              can be use for SAT2IP streaming. Currently not support for PVR streaming.
 *              This HTTP server use port:8080.
 */
void http_server_start(void);

/** 
 *  @brief      Stop HTTP server for SAT2IP/PVR2IP.
 *  @details    All URL request related to SAT2IP/PVR2IP will not process anymore after stop,
 *              but the server does not stop running, if you going to stop it, you need call
 *              http_mg_server_stop().
 */
void http_server_stop(void);

/** 
 *  @brief      Register new HTTP URL process function to HTTP server after server started.
 *  @details    When register one 'name_xx' and 'func' for SAT2IP, client can request by URL
 *              'http:x.x.x.x:8080/method/sat2ip/name_xx'. If HTTP server recieve such request,
 *              it will forward this request to 'func', 'func' process and response it.
 *              Currently HTTP server default register 4 URLs internal:
 *              "http://x.x.x.x:8080/method/sat2ip/get_channel_list" - get channel list from STB.
 *              "http://x.x.x.x:8080/method/sat2ip/play_live_stream" - Play channel stream by HTTP.
 *              "http://x.x.x.x:8080/method/pvr2ip/get_pvr_list" - Get shared PVR list when enable PVR2IP.
 *              "http://x.x.x.x:8080/method/pvr2ip/get_pvr_user_data" - Get private PVR user data.
 *  @param[in]  type    1: method for SAT2IP, auto add dir "/method/sat2ip/" before 'name' to consist URI;
 *                      2: method for PVR2IP, auto add dir "/method/pvr2ip/" before 'name' to consist URI.
 *  @param[in]  name    Method name to consist URI.
 *  @param[in]  func    The process function when URI be accessed.
 *  @return     TRUE when success; FALSE when failure.
 */
BOOL http_server_method_reg(int type, char *name, http_mg_method_t func);

/** 
 *  @brief      Unregister HTTP URL process function from HTTP Server.
 *  @details    After unregister, all related requests from client will not be processed.
 */
void http_server_method_unreg(int type, char *name);

#endif //_ALI_SAT2IP_H_

