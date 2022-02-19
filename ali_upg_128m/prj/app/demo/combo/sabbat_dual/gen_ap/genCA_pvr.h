/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: genCA_pvr.h
*
*    Description: This file contains the callback functions for re-encryption of transport stream
                 and encryption and HMAC of related metadata
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifndef __GEN_CA_PVR_H__
#define __GEN_CA_PVR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include <api/libpvr/lib_pvr_eng.h>
#include <api/libpvr/lib_pvr.h>

#ifdef _C0700A_VSC_ENABLE_
#define GEN_CA_PVR_RSC_REC_NUM            1
#else
#define GEN_CA_PVR_RSC_REC_NUM            PVR_MAX_REC_NUM
#endif
struct channel_handle
{
	UINT8 state;
	UINT32 prog_id;
	UINT32 handle;
	UINT32 last_time_change_key;//for Ali internal test
};
/*****************************************************************************
 * Function: gen_ca_set_dsc_for_live_play
 * Description:
 *    when pvr stop record or timeshift to live, need to set dsc again
 *
 * Input:
 *      Para 1: UINT32 dmx_id , DMX id for living play
 *      Para 2: UINT32 stream_id , decrypt stream id for living play
 *
 * Output:
 *      None
 *
 * Returns: None
 *
*****************************************************************************/
void gen_ca_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id);
/*****************************************************************************
 * Function: get_dsc_pid_multi_des
 * Description:
 *    gets the specified program's descrambling pids.
 *
 * Input:
 *      Para 2: UINT16 pid_number, Pid number want to get
 *      Para 3: UINT32 prog_id, Specify which program
 *
 * Output:
 *      Para 1: UINT16 *pid_list, A pointer to pid list
 *
 * Returns: UINT16 pid_number, Real output pid number
 *
*****************************************************************************/
UINT16 get_dsc_pid_multi_des(UINT16 *pid_list, UINT16 pid_number,UINT32 prog_id);
/*****************************************************************************
 * Function: gen_ca_pvr_replay_time_search
 * Description:
 *    when playback in jump mode , get correct key data
 * Input:
 *        Para 1: UINT32 time, jump time
 * Output:
 *        None
 *
 * Returns:   TRUE : success , FALSE : failure
 *
*****************************************************************************/
BOOL gen_ca_pvr_replay_time_search(UINT32 time);
/*****************************************************************************
 * Function: gen_ca_pvr_set_key_info
 * Description:
 *    set crypto device parameter to use even or odd key
 * Input:
 *        Para 1: UINT32 prog_id, record channel id
 *        Para 2: UINT8 *keydata, encrypt key data for generating recording key
 *        Para 3: struct gen_ca_crypto_info crypto_info, crypto information
 * Output:
 *        None
 *
 * Returns:   TRUE : success , FALSE : failure
 *
*****************************************************************************/
BOOL gen_ca_pvr_set_key_info(UINT32 prog_id , UINT8 *keydata , struct gen_ca_crypto_info crypto_info);
/*****************************************************************************
 * Function: gen_ca_pvr_rec_config
 * Description:
 *    configure crypto for re-encrypt ts, and encrypt key
 * Input:
 *        Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 * Output:
 *        Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int gen_ca_pvr_rec_config(pvr_crypto_general_param *rec_param);
/*****************************************************************************
 * Function:gen_ca_pvr_playback_config
 * Description:
 *    configure crypto for decrypt ts and decrypt key
 * Input:
 *        Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *        Para 2: INT8 timeshift_flag, Indicate timeshift or not
 *
 * Output:
 *      Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int gen_ca_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag);
/*****************************************************************************
 * Function: gen_ca_pvr_rec_stop
 * Description:
 *    Release recording resource when stop record
 * Input:
 *        Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int gen_ca_pvr_rec_stop(pvr_crypto_general_param *rec_param);
/*****************************************************************************
 * Function: gen_ca_pvr_playback_stop
 * Description:
 *    Release playback resource when stop playback
 * Input:
 *        Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int gen_ca_pvr_playback_stop(pvr_crypto_general_param *play_param);
/*****************************************************************************
 * Function: gen_ca_set_dsc_for_live_play
 * Description:
 *    Set DSC for live stream
 * Input:
 *        Para 1: UINT16 dmx_id, dmx id of live stream
 *        Para 2: UINT32 stream_id, stream id of live stream
 *
 * Output:
 *      None
 *
 * Returns:   None
 *
*****************************************************************************/
//void cas9_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id);
/*****************************************************************************
 * Function: gen_ca_pvr_init
 * Description:
 *    Initialize pvr module
 * Input:
 *        None
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
INT32 gen_ca_task_init(void);
int gen_ca_pvr_init(void);
/*****************************************************************************
 * Function: gen_ca_crypto_data
 * Description:
 *    Encrypt/Decrypt meta data
 * Input:
 *        Para 1: pvr_crypto_data_param *cp, a sturcutre pointer of pvr crypto data
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int gen_ca_crypto_data(pvr_crypto_data_param *cp);

#ifdef __cplusplus
}
#endif

#endif /* __GEN_CA_PVR_H__ */

