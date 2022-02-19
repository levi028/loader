/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message_tx.h
*
* Description:
*     Header file Processing & Sending HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _CEC_MESSAGE_TX_H_
#define _CEC_MESSAGE_TX_H_

#ifdef __cplusplus
extern "C"
{
#endif


INT32 tx_cec_msg_image_view_on(void);
INT32 tx_cec_msg_text_view_on(void);
INT32 tx_cec_msg_active_source(void);
INT32 tx_cec_msg_request_active_source(void);
INT32 tx_cec_msg_cec_version(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_get_cec_version(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_give_physical_address(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_report_physical_address(void);
INT32 tx_cec_msg_get_menu_language(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_set_menu_language(E_CEC_LOGIC_ADDR dest_address, P_CEC_MENU_LANGUAGE p_menu_lang);
INT32 tx_cec_msg_give_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, P_CEC_STATUS_REQUEST status_request);
INT32 tx_cec_msg_tuner_device_status(UINT8 dest_address, P_CEC_TUNER_DEVICE_INFO tuner_device_info);
INT32 tx_cec_msg_tuner_step_increment(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_tuner_step_decrement(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_give_device_vendor_id(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_device_vendor_id(void);
INT32 tx_cec_msg_set_osd_string(E_CEC_LOGIC_ADDR dest_address, P_CEC_OSD_STRING osd_string);
INT32 tx_cec_msg_give_osd_name(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_set_osd_name(E_CEC_LOGIC_ADDR dest_address, P_CEC_OSD_NAME osd_name);
INT32 tx_cec_msg_menu_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_MENU_STATE menu_state);
INT32 tx_cec_msg_give_device_power_status(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_report_power_status(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_feature_abort(E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode,
    E_CEC_FEATURE_ABORT_REASON abort_reason);
INT32 tx_cec_msg_abort(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_user_control_pressed(E_CEC_LOGIC_ADDR dest_address, E_CEC_KEY_CODE ui_command);
INT32 tx_cec_msg_user_control_released(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_give_audio_status(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_give_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address);
INT32 tx_cec_msg_report_audio_status(E_CEC_LOGIC_ADDR dest_address, UINT8 audio_status);
INT32 tx_cec_msg_request_short_audio_descriptor(E_CEC_LOGIC_ADDR dest_address,
    E_CEC_AUDIO_FORMAT_CODE *p_afc_code, UINT8 afc_num);
INT32 tx_cec_msg_report_short_audio_descriptor(E_CEC_LOGIC_ADDR dest_address, UINT8 *p_short_audio_desc, \
    UINT8 short_audio_desc_num);
INT32 tx_cec_msg_system_audio_mode_request(E_CEC_LOGIC_ADDR dest_address, BOOL b_sac_mode_on);
INT32 tx_cec_msg_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address);

#ifdef __cplusplus
 }
#endif

#endif
