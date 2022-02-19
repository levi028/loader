 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_hdmi_certification.h
*
*    Description:   The realize for hdmi certification test
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_HDMI_CERTIFICATION_H_
#define _WIN_HDMI_CERTIFICATION_H_
#include <sys_config.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifdef CEC_SUPPORT

extern CONTAINER win_hdmi_certify_con;

extern CONTAINER cert_item_con_hdmi_hdcp;
extern CONTAINER cert_item_con_cec_on_off;
extern CONTAINER cert_item_con_cec_rcp_feature;
extern CONTAINER cert_item_con_cec_system_audio_mode;
extern CONTAINER cert_item_con_cec_source_state;
extern CONTAINER cert_item_con_cec_standby_mode;
extern CONTAINER cert_item_con_cec_command_select;
extern CONTAINER cert_item_con_cec_dest_address;
extern CONTAINER cert_item_con_cec_command_send;


extern TEXT_FIELD cert_item_txtname_hdmi_hdcp;
extern TEXT_FIELD cert_item_txtname_cec_on_off;
extern TEXT_FIELD cert_item_txtname_cec_rcp_feature;
extern TEXT_FIELD cert_item_txtname_cec_system_audio_mode;
extern TEXT_FIELD cert_item_txtname_cec_source_state;
extern TEXT_FIELD cert_item_txtname_cec_standby_mode;
extern TEXT_FIELD cert_item_txtname_cec_command_select;
extern TEXT_FIELD cert_item_txtname_cec_dest_address;
extern TEXT_FIELD cert_item_txtname_cec_command_send;

extern MULTISEL cert_item_txtset_hdmi_hdcp;
extern MULTISEL cert_item_txtset_cec_on_off;
extern MULTISEL cert_item_txtset_cec_rcp_feature;
extern MULTISEL cert_item_txtset_cec_system_audio_mode;
extern MULTISEL cert_item_txtset_cec_source_state;
extern MULTISEL cert_item_txtset_cec_standby_mode;
extern MULTISEL cert_item_txtset_cec_command_select;
extern MULTISEL cert_item_txtset_cec_dest_address;

extern MULTI_TEXT hdmi_cec_mtxt;

#endif
#ifdef __cplusplus
 }
#endif
#endif//_WIN_HDMI_CERTIFICATION_H_

