/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mcas.h
*
*    Description: This file defines the API of CA function

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _MCAS_H
#define _MCAS_H

#include <types.h>
#ifdef SUPPORT_HW_CONAX
    #include <api/libtsi/db_3l.h>
    #include <api/libttx/lib_ttx.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MCAS_STATE_UNREGISTER   0x00
#define MCAS_STATE_IDLE     0x01
#define MCAS_STATE_ECM          0x02
#define MCAS_STATE_CAT          0x04
#define MCAS_STATE_EMM      0x08
#define MCAS_STATE_SERVICE_START    0x10

typedef enum
{
    MCAS_DISP_NO_MSG = 0,
    MCAS_DISP_CARD_IN,
    MCAS_DISP_CARD_OUT,
    MCAS_DISP_CARD_INITED,
    MCAS_DISP_CARD_NONE,
    MCAS_DISP_PIN_WRONG,
    MCAS_DISP_INPUT_PIN,
    MCAS_DISP_VIEW_PIN,
    MCAS_DISP_ORDER_PIN,
    MCAS_DISP_FINGER_IND,
    MCAS_DISP_FINGER_CLR,
    MCAS_DISP_HOST_MESSAGE,
    MCAS_DISP_HOST_MSGCLR,
    MCAS_DISP_MAT_CHANGE,
    MCAS_DISP_NO_ACCESS,
    MCAS_DISP_GEO_BLACKOUT,
    MCAS_DISP_NO_ACCESS_NET,
    MCAS_DISP_CARD_NON_EXIST,
    MCAS_DISP_CARD_INCOMPATIBLE,
    MCAS_DISP_INTERFACE_ERR,
    MCAS_DISP_CARD_ERROR,
    MCAS_DISP_NO_ACCESS_CLR,
    MCAS_DISP_NO_ACCESS_ERROR_CODE,
    MCAS_DISP_NO_ACCESS_NOT_SUBSCRIBED,
    MCAS_DISP_NO_SERVICE,
    MCAS_DISP_FINGER_PVR_IND,
    MCAS_DISP_CARD_INCOMPATIBLE_CONAX,
    MCAS_DISP_NO_CARD_CLR,
    MCAS_DISP_FINGER_ORIGINAL_IND,
#ifdef CAS9_V6
    MCAS_DISP_URI_UPDATED=0x80,
    MCAS_DISP_CHECK_CACHED_PIN,
    MCAS_DISP_CAS9_CARD_IN,
    MCAS_DISP_PLAYBACK_URI_UPDATED,
    MCAS_DISP_CHECK_REC_LIST,
#endif
    MCAS_DISP_VSC_SYSTEM_ERROR,
    MCAS_DISP_VSC_NOT_PERSO,
    MCAS_DISP_VSC_VERSION_ERROR,
} mcas_disp_type_t;


/************************
mcas working status
*************************/

typedef enum
{
    MCAS_NULL = 0,
    MCAS_STOP,
    MCAS_START_ECM,     /*change to a scrambled channel*/
    MCAS_GOT_ECM,
    MCAS_START_CAT,     /*change to one TP and request CAT*/
    MCAS_GOT_CAT,
    MCAS_START_EMM,     /*change to one TP and request EMM*/
    MCAS_GOT_EMM,
    MCAS_CARD_IN,
    MCAS_CARD_OUT,
    MCAS_CARD_UPDATE,
    MCAS_INPUT_PIN,
    MCAS_VIEW_PIN,
    MCAS_ORDER_PIN,
    MCAS_FINGER_IND,
    MCAS_FINGER_CLR,
    MCAS_HOST_MESSAGE,
    MCAS_MAT_CHANGE,
    MCAS_NO_ACCESS,
    MCAS_GEO_BLACKOUT,
    MCAS_NO_ACCESS_NET,
    MCAS_CARD_NON_EXIST,
    MCAS_CARD_INCOMPATIBLE,
    MCAS_INTERFACE_ERR,
    MCAS_CARD_ERROR,
    MCAS_NO_ACCESS_CLR,
    MCAS_NO_ACCESS_ERROR_CODE,
    MCAS_UPDATE_PID,
#ifdef CAS9_V6
    MCAS_URI_UPDATED=0x80,
    MCAS_CHECK_CACHED_PIN,
    MCAS_CAS9_CARD_IN,
    MCAS_PLAYBACK_URI_UPDATED,
    MCAS_CHECK_REC_LIST,
#endif
    MCAS_VSC_SYSTEM_ERROR,
    MCAS_VSC_NOT_PERSO,
    MCAS_VSC_VERSION_ERROR,
} mcas_msg_type_t;

typedef struct
{
    mcas_msg_type_t type;
    UINT32 param;
} mcas_msg_t;

typedef void (*mcas_disp_func_t)(mcas_disp_type_t type, UINT32 param);
typedef void (*mcas_apdu_cb)(UINT8 apdu_id, UINT8 apdu_len, UINT8 *apdu_data);//old API, should use mcas_apdu_cb_ext
typedef void (*mcas_apdu_cb_ext)(UINT32 apdu_id, UINT32 apdu_sub_id, UINT32 apdu_len, UINT8 *apdu_data);  //for User defined APDU

//data_type = 0, mean emm original data from stream.
//data_type =1 , mean output data of emm from card.
typedef void (*mcas_special_cb)(UINT8 data_type,UINT8 *data,UINT32 data_len);  
typedef int (*key_store_op_cb)(UINT32 on_off);  

#ifdef CAS9_V6
struct mcas_config_par
{
	UINT32 pre_cw_valid :1 ;
    UINT32 bg_timer_rec :1 ;
    UINT32 rec_emm_fp   :1 ;
    UINT32 mail_msg_sp_operate:1;
    UINT32 keystore_hmac_enable:1;
    UINT32 reserved : 27;
	
    mcas_apdu_cb_ext apdu_cb;
    mcas_special_cb sp_cb;
};
#endif

#if(CAS_TYPE!=CAS_C2000A)
#ifndef _C0700A_VSC_ENABLE_
typedef struct
{
    OSAL_ID thread_id;
    OSAL_ID msgque_id;
    UINT16 state;
    mcas_disp_func_t disp_callback;
} mcas_t;
#endif
#endif

typedef struct
{
    UINT16 pid;
    UINT16 ca_system_id;
    struct restrict filter_restrict;
    UINT8 dmx_id;       //indicate the filter belong to which dmx
    struct si_filter_t *sfilter;
} cas_filter_t;

typedef struct struct_ca_chunk_id
{
    UINT32 ca_date_msgfp_id;
    UINT32 key_store_id;
    UINT32 reserved1;
    UINT32 reserved2;
    UINT32 reserved3;
}CA_CHUNK_ID_S,*P_CA_CHUNK_ID_S;


#define MAX_CA_AUDIO_CNT 16
typedef struct
{
    UINT16 prog_number;

    UINT16 video_pid;

    UINT8  cur_audio;
    UINT16  audio_count;
    UINT16 audio_pid[MAX_CA_AUDIO_CNT];

    UINT16 subtitle_pid;
    UINT16 teletext_pid;
} ca_service_info;

//NEW_TIMER_REC
typedef struct
{
    ca_service_info ca_info;
    UINT32 prog_id;    
} ca_pvr_service_info;

/* NOR FLASH or NAND FLASH */
void api_mcas_init_sto_type(UINT32 type);

/* Export function for root */

/*****************************************************************************
 * Function: api_mcas_register
 * Description:
 *      This function registers CA lib module.
 * Input:
 *      Para 1: mcas_disp_func_t mcas_disp_func, A callback function pointer to send ca task message to UI
 *
 * Output:
 *      None
 *
 * Returns:
 *      RET_SUCCESS(0): Successful
 *      RET_FAILURE(-1): Fail
 *
*****************************************************************************/
INT32 api_mcas_register(mcas_disp_func_t mcas_disp_func);

/*****************************************************************************
 * Function: api_mcas_unregister
 * Description:
 *      This function unregisters CA lib module.
 *
 * Input: None
 *
 * Output: None
 *
 * Returns:
 *      RET_SUCCESS(0): Successful
 *
*****************************************************************************/
INT32 api_mcas_unregister(void);

//User defined APDU
/*****************************************************************************
 * Function: api_mcas_reg_user_def_apdu_cb
 * Description: This function registers "user define apdu" callback to CA lib module.
 * Input:     Para 1: mcas_apdu_cb  cb, A callback function pointer for UI to process user defined apdu
 * Output:   None
 * Returns: 
 *      RET_SUCCESS(0): Successful
 *      RET_FAILURE(-1): Fail
*****************************************************************************/
INT32 api_mcas_reg_user_def_apdu_cb(mcas_apdu_cb cb);

/*****************************************************************************
 * Function: api_mcas_unreg_user_def_apdu_cb
 * Description: This function unregisters "user define apdu" callback to CA lib module.
 * Input:     None
 * Output:   None
 * Returns: 
 *      RET_SUCCESS(0): Successful
 *      RET_FAILURE(-1): Fail
*****************************************************************************/
INT32 api_mcas_unreg_user_def_apdu_cb();

UINT8 ca_set_chunkid(P_CA_CHUNK_ID_S p_chunkid);
/* Export function for channal change */
#ifdef SUPPORT_2CA
#ifdef NEW_DEMO_FRAME
INT32 c0700a_mcas_start_service(UINT16 channel_index, UINT32 mon_id);
INT32 c0700a_mcas_stop_service(UINT32 mon_id);
#else
INT32 c0700a_mcas_start_service(UINT16 channel_index);
INT32 c0700a_mcas_stop_service(void);
#endif
INT32 c0700a_mcas_start_transponder(void);
INT32 c0700a_mcas_stop_transponder(void);
#endif

#ifdef NEW_DEMO_FRAME
/*****************************************************************************
 * Function: api_mcas_start_service
 * Description:
 *      This function starts ca task to descramble channel with index "channel_index"
 * Input:
 *      Para 1: UINT32 channel_index, Index of channel for scramble
 *      Para 2: UINT32 mon_id, PMT monitor id
 *
 * Output:
 *      None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *      !SUCCESS: Fail
 *
*****************************************************************************/
INT32 api_mcas_start_service(UINT32 channel_index, UINT32 mon_id);

/*****************************************************************************
 * Function: api_mcas_start_service_multi_des
 * Description:
 *      This function starts ca task to descramble channel with index "channel_index"
 * Input:
 *      Para 1: UINT32 channel_index, Index of channel for scramble
 *      Para 2: UINT32 mon_id, PMT monitor id
 *      Para 3: UINT8 dmx, Index of DMX
 *
 * Output:
 *      None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *      !SUCCESS: Fail
 *
*****************************************************************************/
INT32 api_mcas_start_service_multi_des(UINT32 channel_index, UINT32 mon_id, UINT8 dmx);

/*****************************************************************************
 * Function: api_mcas_stop_service
 * Description:
 *      This function stops ca task to descramble channel
 * Input:
 *      Para 1: UINT32 mon_id, PMT monitor id
 *
 * Output:
 *      None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *      !SUCCESS: Fail
 *
*****************************************************************************/
INT32 api_mcas_stop_service(UINT32 mon_id);

/*****************************************************************************
 * Function: api_mcas_stop_service_multi_des
 * Description:
 *      This function stops ca task to descramble channel
 * Input:
 *      Para 1: UINT32 prog_id, Specify which program to stop
 *      Para 2: UINT32 mon_id, PMT monitor id of the program
 *
 * Output:
 *      None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *      !SUCCESS: Fail
 *
*****************************************************************************/
INT32 api_mcas_stop_service_multi_des(UINT32 prog_id,UINT32 mon_id);
#else
INT32 api_mcas_start_service(UINT32 channel_index);
INT32 api_mcas_stop_service(void);
#endif

/*****************************************************************************
 * Function: api_mcas_start_transponder
 * Description:
 *      This function is used to start ca task to monitor current transponder's SI info, like CAT.
 *
 * Input: None
 *
 * Output: None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *
*****************************************************************************/
INT32 api_mcas_start_transponder(void);

/*****************************************************************************
 * Function: api_mcas_start_transponder_multi_des
 * Description:
 *      This function is used to start ca task to monitor the specified transponder's SI info, like CAT.
 * 
 * Input:
 *      Para 1: UINT8 dmx_id, Dmx index, specify which transponder
 *
 * Output: None
 *
 * Returns:
 *      SUCCESS(0): Successful, others: Fail
 *
*****************************************************************************/
INT32 api_mcas_start_transponder_multi_des(UINT8 dmx_id);

/*****************************************************************************
 * Function: api_mcas_stop_transponder
 * Description:
 *      This function is used to stop ca task to monitor current transponder's SI info.
 *
 * Input: None
 *
 * Output: None
 *
 * Returns:
 *      SUCCESS(0): Successful, others: Fail
 *
*****************************************************************************/
INT32 api_mcas_stop_transponder(void);

/*****************************************************************************
 * Function: api_mcas_stop_transponder_multi_des
 * Description:
 *      This function is used to stop monitoring the specified transponder's SI info.
 *
 * Input: 
 *      Para 1: UINT8 dmx_id, DMX index, specify which transponder
 *
 * Output: None
 *
 * Returns:
 *      SUCCESS(0): Successful
 *      Others: Fail
 *
*****************************************************************************/
INT32 api_mcas_stop_transponder_multi_des(UINT8 dmx_id);

#ifdef SUPPORT_HW_CONAX
#ifndef SUPPORT_CAS9
INT32 mcas_set_prog_info(P_NODE *p_node);   //Shall use ca_set_prog_info() instead in Cas9 V5,V6
#endif
#endif

void api_mcas_get_emm_pid(UINT16 * pidlist,UINT8 pid_len,UINT8 * len,UINT8 dmx);
INT32 api_mcas_request_tdt(void);
void api_mcas_get_playback_utc(UINT8 * time,UINT8 len);
void api_mcas_stop_playback_tdt(void);
UINT8 api_mcas_playback_utc_init(void);
//used by CAS9_PVR_SCRAMBLED solution only
UINT8 api_mcas_get_playback_utc_init(void);
void api_mcas_set_playback_utc_init(UINT8 flag);

/*****************************************************************************
 * Function: api_mcas_get_prog_record_flag
 * Description:
 *      This function is used to check whether the program is recording
 *
 * Input:
 *      Para 1: UINT32 prog_id, Program id
 *
 * Output: None
 *
 * Returns:
 *      TRUE(1): Recording
 *      FALSE(0): Non-recording
 *
*****************************************************************************/
INT8 api_mcas_get_prog_record_flag(UINT32 prog_id);

/*****************************************************************************
 * Function: api_mcas_set_prog_record_flag
 * Description:
 *      This function is used to mark the program record or not
 *
 * Input:
 *      Para 1: UINT32 prog_id, Program id
 *      Para 2: UINT8 flag, TRUE(0) : record ; FALSE(0) : not record
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_mcas_set_prog_record_flag(UINT32 prog_id, UINT8 flag);

/*****************************************************************************
 * Function: api_mcas_get_session_by_prog
 * Description:
 *      This function is used to search the session according to prog_id.
 *
 * Input:
 *      Para 1: UINT32 prog_id, Program id
 *
 * Output: None
 *
 * Returns: INT8
 *      >0, Valid smart card session id
 *      -1, No valid smart card session for this program
 *
*****************************************************************************/
INT8 api_mcas_get_session_by_prog(UINT32 prog_id);

/*****************************************************************************
 * Function: api_mcas_set_pre_cw_valid
 * Description:
 *      This function is used to skip load empty control word if current ECM is rejected.
 *      Notice: This feature is disabled by default.
 *      To turn on, need to invoke this API. (Timing: before api_mcas_register())
 *
 * Input:
 *      Para 1: UINT8 bon, Enable keep previous control word
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_mcas_set_pre_cw_valid(UINT8 bon);

#if defined(CAS9_V6) && defined(CAS9_PVR_SID)

//NEW_TIMER_REC
/*****************************************************************************
 * Function: api_mcas_set_bg_timer_rec
 * Description: 
 *      This function is used to support schedule record in the background, not effect user's tv watching 
 *      Notice: This feature is disabled by default. 
 *      To turn on, need to invoke this API. (Timing: before api_mcas_register())
 * Input: Para 1: UINT8 bon, Enable supprt schedule recording in the background
 * Output: None
 * Returns: None
*****************************************************************************/
void api_mcas_set_bg_timer_rec(UINT8 bon);

/*****************************************************************************
 * Function: api_mcas_get_rec_sid_by_prog
 * Description:
 *      This function searches smart card session id (for recording usage), according to specified program.
 *      When search fail and TRUE "fcreate": allocate one new smart card session for specified program
 *      Notice: Invoke it when start recording. (Timing: before api_mcas_get_ts_stream_id_by_sid())
 *
 * Input:
 *      Para 1: UINT32 prog_id, Specify which program to search
 *      Para 2: BOOL fcreate, FALSE: Search only, TRUE: Allocate one session when search fail
 *
 * Output: None
 *
 * Returns: INT8
 *      >0, Valid Smart card session id
 *      -1, Fail (search fail or allocate fail), No valid smart card session to record this program
 *
*****************************************************************************/
INT8 api_mcas_get_rec_sid_by_prog(UINT32 prog_id,BOOL fcreate);

/*****************************************************************************
 * Function: api_mcas_get_ts_stream_id_by_sid
 * Description:
 *      This function searches [ts mode] stream id (for recording usage), according to specified session.
 *      When search fail and TRUE "fcreate": allocate one new [ts mode] stream id.
 *      Notice: Invoke it when start recording. (Timing: after api_mcas_get_rec_sid_by_prog())
 *
 * Input:
 *      Para 1: UINT8 bsid, Specify which smart card session to search
 *      Para 2: BOOL fcreate,FALSE: Search only, TRUE: Allocate one when search fail
 *
 * Output: None
 *
 * Returns: UINT16
 *      INVALID_DSC_STREAM_ID(255), Fail (search fail or allocate fail)
 *      Others, Valid [ts mode] stream id
 *
*****************************************************************************/
UINT16 api_mcas_get_ts_stream_id_by_sid(UINT8 bsid,BOOL fcreate);

/*****************************************************************************
 * Function: api_mcas_free_ts_stream_id_by_sid
 * Description:
 *      This function is used to search and release [ts mode] stream id (for recording purpose),
 *      according to specified session.
 *      Notice: Invoke it when stop recording.(Timing: before api_mcas_free_rec_sid())
 *
 * Input:
 *      Para 1: UINT8 bsid, Specify which smart card session to search
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_mcas_free_ts_stream_id_by_sid(UINT8 bsid);

/*****************************************************************************
 * Function: api_mcas_free_rec_sid
 * Description:
 *      This function is used to release specified smart card session.
 *      Notice: Invoke it when stop recording. (Timing: after api_mcas_free_ts_stream_id_by_sid())
 *
 * Input:
 *      Para 1: UINT8 bsid, Specify which smart card session
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_mcas_free_rec_sid(UINT8 bsid);
#endif

/*****************************************************************************
 * Function: api_set_ca_living_dmx
 * Description:
 *      For AP to pass dmx id and program id of front live play program to ca lib
 *
 * Input:
 *      Para 1: UINT8 dmx_id, Index of DMX
 *      Para 2: UINT32 prog_id, Program id
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_set_ca_living_dmx(UINT8 dmx_id,UINT32 prog_id);

/*****************************************************************************
 * Function: api_get_ca_living_dmx
 * Description:
 *      Get dmx id of front live play program
 *
 * Input: None
 *
 * Output: None
 *
 * Returns: UINT8, Index of DMX
 *
*****************************************************************************/
UINT8 api_get_ca_living_dmx(void);

/*****************************************************************************
 * Function: api_set_ca_playback_dmx
 * Description:
 *      For AP to pass rec type and dmx id of play recording to ca lib
 *
 * Input:
 *      Para 1: UINT8 rec_type,
 *          0:re-encription; 1:original stream;2:FTA (only for re-encription)
 *      Para 2: UINT8 dmx_id, index of DMX
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_set_ca_playback_dmx(UINT8 rec_type,UINT8 dmx_id);

/*****************************************************************************
 * Function: api_set_ca_record_type
 * Description:
 *      For AP to pass rec type and dmx id of recording to ca lib
 *
 * Input:
 *      Para 1: UINT8 rec_type,
 *          0:re-encription; 1:original stream;2:FTA (only for re-encription)
 *      Para 2: UINT8 dmx_id, index of DMX
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_set_ca_record_type(UINT8 rec_type,UINT8 dmx_id);

#ifdef CAS9_VSC
/*****************************************************************************
 * Function: api_vsc_lib_start
 * Description:
 *      This function is used only for CONAX cardless solution.
 *      This functionality of this API is to trigger CA lib which a pseudo card inserted.
 *
 * Input:
 *      Para 1: mcas_msg_type_t type, Message send to CA lib
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_vsc_lib_start (mcas_msg_type_t type);
#endif

#ifdef CAS9_V6
/*****************************************************************************
 * Function: api_mcas_set_cached_pin
 * Description:
 *      For AP to pass cached ca pin code to ca lib
 *
 * Input:
 *      Para 1: UINT8 *ca_pin, pointer of pin code (i.e. 4 byte of data)
 *      Para 2: UINT8 digit, must be SC_PIN_DIGIT(4)
 *
 * Output: None
 *
 * Returns: None
 *
*****************************************************************************/
void api_mcas_set_cached_pin(UINT8 *ca_pin,UINT8 digit);

/*****************************************************************************
 * Function: api_mcas_enable_ecm_cmd
 * Description: For AP to enable/disable send ecm to card, ex: in
              main menu, disable ecm, leave menu, enable ecm
 * Input: 
 *      Para 1: UINT8 enable
 * Output: None
 * Returns: None
*****************************************************************************/
void api_mcas_enable_ecm_cmd(UINT8 enable);

/*****************************************************************************
 * Function: api_mcas_get_last_start_chan_idx
 * Description: get last started channel index
 * Input: None.
 * Output: None
 * Returns: channel index
*****************************************************************************/
UINT32 api_mcas_get_last_start_chan_idx();

/*****************************************************************************
 * Function: api_mcas_get_csa_id_by_sid
 * Description: get the corresponding csa dev id according the session id
 * Input: session id.
 * Output: None
 * Returns: csa dev id
*****************************************************************************/
UINT32 api_mcas_get_csa_id_by_sid(UINT8 sid_sc);

/*****************************************************************************
 * Function: api_mcas_config
 * Description: config CA lib
 * Input: mcas_config_par.
 * Output: None
 * Returns: RET_CODE
*****************************************************************************/
RET_CODE api_mcas_config(struct mcas_config_par *pconfig_par);
#endif

#ifdef SUPPORT_HW_CONAX
void ca_notify_ttx_pid(enum ttx_event event, UINT32 pid);
#endif

void api_mcas_config_keystore_write_cb(key_store_op_cb *cb);

#ifdef __cplusplus
}
#endif

#endif


