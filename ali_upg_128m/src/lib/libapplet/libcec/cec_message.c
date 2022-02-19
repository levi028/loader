/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message.c
*
* Description:
*     Internal functions for processing HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h> // due to com_api.h used the OSD typedef
#include <bus/hdmi/m36/hdmi_api.h>

#include "cec_common_str.h"
#include "cec_private.h"
#include "cec_message_tx.h"
#include "cec_message_rx.h"

static E_CEC_MSG_ERRNO cec_opcode_abort(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
    *   GP: General Protocol
    *      <Feature Abort>
    *      <Abort> Message
    *************************************************************************/
    switch (opcode)
    {
        case OPCODE_FEATURE_ABORT:
            t_result = rx_cec_msg_feature_abort(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_ABORT:
            t_result = rx_cec_msg_abort(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_view_on(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
    *  OTP: One Touch Play (As Follower)
    *
    *  Supported Command:
    *      <Active Source> 1:  please refer to Routing Control (RC) command
    *
    *  Others:
    *      <Image View On>:        drop this message (sent to TV only)
    *      <Text View On> :        drop this message (sent to TV only)
    *
    *************************************************************************/
    switch (opcode)
    {
        case OPCODE_IMAGE_VIEW_ON:
            t_result = rx_cec_msg_image_view_on(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_TEXT_VIEW_ON:
            t_result = rx_cec_msg_text_view_on(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_source(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
    *  RC: Routing Control  (As Follower)
    *
    *  Supported Command:
    *      <Active Source> 2 [Physical Address] :
    *      <Request Active Source> :
    *      <Set Stream Path> :
    *
    *  Others:
    *      <Inactive Source> :     drop this message (sent to TV only)
    *      <Routing Change>    :       drop this message (CEC switch command)
    *      <Routing Information> : drop this message (CEC switch command)
    *
    *************************************************************************/
    switch (opcode)
    {
        case OPCODE_ACTIVE_SOURCE:
            t_result = rx_cec_msg_active_source(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REQUEST_ACTIVE_SOURCE:
            t_result = rx_cec_msg_request_active_source(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_STREAM_PATH:
            t_result = rx_cec_msg_set_stream_path(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_INACTIVE_SOURCE:
            t_result = rx_cec_msg_inactive_source(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_ROUTING_CHANGE:
            t_result = rx_cec_msg_routing_change(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_ROUTING_INFORMATION:
            t_result = rx_cec_msg_routing_information(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
          return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_system_standby(UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  SS: System Standby  (As Follower)
        *
        *  Supported Command:
        *      <Standby> :
        *
        *  Others:
        *
        *
    *************************************************************************/
    t_result = rx_cec_msg_system_standby(source_address, dest_address, param_ptr, param_length);
    if(CEC_MSG_OK != t_result)
    {
          return t_result;
    }
    return t_result;
}

static void cec_opcode_record(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    /*************************************************************************
    *  OTR: One Touch Record  (As Follower)
    *
    *  Supported Command:
    *
    *  Others:
    *      <Record Off> :      reply <Feature Abort>
    *      <Record On> :       reply <Feature Abort>
    *      <Record Status> :       reply <Feature Abort>
    *      <Record TV Screen> :    reply <Feature Abort>
    *
    *************************************************************************/
    // replay <Feature Abort> by default
    // NOT Support in LIB_CEC, add code only for CEC command recognition.
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;
	
    t_result = rx_cec_msg_record_off(source_address, dest_address, param_ptr, param_length);
     if(CEC_MSG_OK != t_result)
     {
           return ;
     }
    ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
    return;
}

static void cec_opcode_timer(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    /*************************************************************************
    *  TP: Timer Programming (As Follower)
    *
    *  Supported Command:
    *
    *  Others:
    *      <Clear Analogue Timer> :    reply <Feature Abort>
    *      <Clear Digital Timer> :     reply <Feature Abort>
    *      <Clear External Timer>: reply <Feature Abort>
    *      <Set Analogue Timer>:       reply <Feature Abort>
    *      <Set Digital Timer> :   reply <Feature Abort>
    *      <Set External Timer> :      reply <Feature Abort>
    *      <Set Timer Program Title> : reply <Feature Abort>
    *      <Timer Cleared Status> :    reply <Feature Abort>
    *      <Timer Status> :            reply <Feature Abort>
    *************************************************************************/
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;
    
    switch (opcode)
    {
        case OPCODE_CLEAR_ANALOG_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_clear_analogue_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_CLEAR_DIGITAL_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_clear_digital_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_CLEAR_EXTERNAL_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_clear_external_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_SET_ANALOG_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_set_analogue_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_SET_DIGITAL_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_set_digital_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_SET_EXTERNAL_TIMER:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_set_external_timer(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_SET_TIMER_PROGRAM_TITLE:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_set_timer_program_title(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_TIMER_CLEARED_STATUS:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result =  rx_cec_msg_timer_cleared_status(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_TIMER_STATUS:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_timer_status(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        default:
            break;
    }
}

static E_CEC_MSG_ERRNO cec_opcode_system_info(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
    *  SI: System Information  (As Follower)
    *
    *  Supported Command:
    *      <Get CEC Version> :         reply <CEC Version>
    *      <Give Physical Address> :       reply <Report Physical Address>
    *      <Get Menu Language> :       reply <Set Menu Language>
    *      <Polling Message> :         reply <Polling Message>
    *
    *  Others:
    *      <CEC Version> [CEC Version]:                            query result from other CEC device
    *      <Report Physical Address> [Physical Addr][Device Type]:     query result from other CEC device
    *      <Set Menu Language> [Language]:                         query result from other CEC device
    *
    *************************************************************************/

    switch (opcode)
    {
        case OPCODE_GET_CEC_VERSION:
            t_result = rx_cec_msg_get_cec_version(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_PHYSICAL_ADDR:
            t_result = rx_cec_msg_give_physical_address(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GET_MENU_LANGUAGE:
            t_result = rx_cec_msg_get_menu_language(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_CEC_VERSION:
            t_result = rx_cec_msg_cec_version(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_PHYSICAL_ADDR:
            t_result = rx_cec_msg_report_physical_address(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_MENU_LANGUAGE:
            t_result = rx_cec_msg_set_menu_language(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static void cec_opcode_deck_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    /*************************************************************************
    *  DC: Deck Control  (As Follower)
    *
    *  Supported Command:
    *
    *  Others:
    *      <Deck Control>
    *      <Deck Status>
    *      <Give Deck Status>
    *      <Play>
    *
    *************************************************************************/
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;
    
    switch (opcode)
    {
        case OPCODE_DECK_CONTROL:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_deck_control(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_DECK_STATUS:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_deck_status(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_GIVE_DECK_STATUS:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_give_deck_status(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        case OPCODE_PLAY:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_play(source_address, dest_address, param_ptr, param_length);
            if(CEC_MSG_OK != t_result)
            {
                return ;
            }
            ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
            break;
        default:
            break;
    }
}

static E_CEC_MSG_ERRNO cec_opcode_tuner_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  TC: Tuner control  (As Follower)
        *
        *  Supported Command:
        *      <Give Tuner Device Status> [Status Request]
        *      <Select Digital Service>  [Digital Service ID]
        *      <Tuner Device Status>   [Tuner Device Info]
        *      <Tuner Step Decrement>
        *      <Tuner Step Increment>
        *
        *  Others:
        *      <Select Analogue Service>
        *
        *************************************************************************/
    switch(opcode)
    {
        case OPCODE_GIVE_TUNER_DEVICE_STATUS:       // Notify UI control task to reply tuner device status
            t_result = rx_cec_msg_give_tuner_device_status(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SELECT_DIGITAL_SERVICE:     // Notify UI control task to tune to select digital service
            t_result = rx_cec_msg_select_digital_service(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SELECT_ANALOG_SERVICE:
            t_result = rx_cec_msg_select_analog_service(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_TUNER_STEP_DECREMENT:   // Notify UI control task to tune to next low channel
            t_result = rx_cec_msg_tuner_step_decrement(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_TUNER_STEP_INCREMENT:   // Notify UI control task to tune to next high channel
            t_result = rx_cec_msg_tuner_step_increment(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_TUNER_DEVICE_STATUS:
            // store the query result if STB indeed issued an query, otherwise drop it!
            t_result = rx_cec_msg_tuner_device_status(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_vendor(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result= CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  VSC: Vendor Specific Commands
        *
        *      <CEC Version> 8  :      refer to System Information  (SI)
        *      <Get CEC Version> 9 :   refer to System Information  (SI)
        *
        *      <Device Vendor ID> [Vendor ID]
        *      <Give Device Vendor ID>
        *      <Vendor Command> [Vendor Specific Data]
        *      <Vendor Command With ID>  [Vendor ID][Vendor Specific Data]
        *      <Vendor Remote Button Down>  [Vendor Specific RC Code]
        *      <Vendor Remote Button Up> [Vendor Specific RC Code]
        *
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_DEVICE_VENDOR_ID:
            t_result = rx_cec_msg_device_vendor_id(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_DEVICE_VENDOR_ID:
            t_result = rx_cec_msg_give_device_vendor_id(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_VENDOR_COMMAND:
            t_result = rx_cec_msg_vendor_command(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_VENDOR_COMMAND_WITH_ID:
            t_result = rx_cec_msg_vendor_command_with_id(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_VENDOR_REOMTE_BUTTON_DOWN:
            t_result = rx_cec_msg_vendor_remote_button_down(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_VENDOR_REOMTE_BUTTON_UP:
            t_result = rx_cec_msg_vendor_remote_button_up(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_osd_display(UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  OD: OSD Display
        *      <Set OSD String> [Display Control][OSD String]  : only sends to TV.
        *************************************************************************/
    t_result = rx_cec_msg_set_osd_string(source_address, dest_address, param_ptr, param_length);
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_osd_transfer(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  DOT: Device OSD Transfer
        *      <Give OSD Name>
        *      <Set OSD Name>  [OSD Name]
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_GIVE_OSD_NAME:  // Reply <Set OSD Name>
            t_result = rx_cec_msg_give_osd_name(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_OSD_NAME:
            t_result = rx_cec_msg_set_osd_name(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_device_menu_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  DMC: Device Menu Control
        *      <Menu Request> [Menu Request Type]
        *      <Menu Status>  [Menu State]
        *      <User Control Pressed> 10   [UI Command]:   refer to Remote Control Passthrough(RCP)
        *      <User Control Released> 11                  refer to Remote Control Passthrough(RCP)
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_MENU_REQUEST:   // Notify UI control task to open/close menu & reply <Menu Status>
            t_result = rx_cec_msg_menu_request(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_MENU_STATUS:
            t_result = rx_cec_msg_menu_status(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_rpc(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  RCP: Remote Control Passthrough
        *      <User Control Pressed> 12  [UI Command]
        *      <User Control Released> 13
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_USER_CTRL_PRESSED:
            t_result = rx_cec_msg_user_control_pressed(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_USER_CTRL_RELEASED: // Notify UI control task passthrough key
            t_result = rx_cec_msg_user_control_released(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_power_status(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  PS: Power STATUS
        *      <Give Device Power Status>
        *      <Report Power Status> [Power Status]
        *
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_GIVE_POWER_STATUS:
            t_result = rx_cec_msg_give_power_status(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_POWER_STATUS:
            t_result = rx_cec_msg_report_power_status(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static E_CEC_MSG_ERRNO cec_opcode_system_audio_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    E_CEC_MSG_ERRNO t_result = CEC_MSG_UNKNOWN;

    /*************************************************************************
        *  SAC: System Audio Control
        *      <Give Audio Status> [None]
        *      <Give System Audio Mode Status> [None]
        *      <Report Audio Status> [Audio Status]
        *      <Set System Audio Mode> [System Audio Status]
        *      <System Audio Mode Request> [Physical Address]
        *      <System Audio Mode Status> [System Audio Status]
        *      <User Control Pressed> [UI Command]
        *      <User Control Released> [None]
        *
        *************************************************************************/
    switch (opcode)
    {
        case OPCODE_GIVE_AUDIO_STATUS:
            t_result = rx_cec_msg_give_audio_status(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            t_result = rx_cec_msg_give_system_audio_mode_status(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_AUDIO_STATUS:
            t_result = rx_cec_msg_report_audio_status(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR: // CEC V1.4 New MSG
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_report_short_audio_descriptor(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR: // CEC V1.4 New MSG
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            t_result = rx_cec_msg_request_short_audio_descriptor(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_SYSTEM_AUDIO_MODE:
            t_result = rx_cec_msg_set_system_audio_mode(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
            t_result = rx_cec_msg_system_audio_mode_request(source_address, dest_address, param_ptr, param_length);
            break;

        case OPCODE_SYSTEM_AUDIO_MODE_STATUS:
            t_result = rx_cec_msg_system_audio_mode_status(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            t_result = CEC_MSG_OK;
            break;
    }
    if(CEC_MSG_OK != t_result)
    {
        return t_result;
    }
    return t_result;
}

static void cec_opcode_audio_rate_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, \
    UINT8 param_length)
{
    /*************************************************************************
        *  ARC: Audio Rate Control
        *      <Set Audio Rate>   [Audio Rate]
        *
        *************************************************************************/

    // NOT Support in LIB_CEC, add code only for CEC command recognition.
    E_CEC_MSG_ERRNO ret = CEC_MSG_UNKNOWN; 
    ret = rx_cec_msg_set_audio_rate(source_address, dest_address, param_ptr, param_length);
     if(ret != CEC_MSG_OK)
     {
             return;
     }
    ap_cec_msg_feature_abort(source_address, opcode, REASON_REFUSED);
}

static void cec_opcode_arcc_audio_return_channel_control(UINT8 opcode, UINT8 source_address, UINT8 dest_address, \
    UINT8 *param_ptr, UINT8 param_length)
{
    /*************************************************************************
        *   ARCC: Audio Return Channel Control (CEC V1.4 New MSG)
        *
        *
        *************************************************************************/
    E_CEC_MSG_ERRNO ret = CEC_MSG_UNKNOWN; 
    
    switch (opcode)
    {
        case OPCODE_INITIATE_ARC:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_initiate_arc(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_ARC_INITIATED:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_report_arc_initiated(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REPORT_ARC_TERMINATED   :
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_report_arc_terminated(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REQUEST_ARC_INITIATION  :
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_request_arc_initiation(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_REQUEST_ARC_TERMINATION:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_request_arc_termination(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_TERMINAE_ARC:
            // NOT Support in LIB_CEC, add code only for CEC command recognition.
            ret = rx_cec_msg_terminate_arc(source_address, dest_address, param_ptr, param_length);
            break;
        default:
            ret = CEC_MSG_OK;
            break;
    }
     if(ret != CEC_MSG_OK)
     {
             return;
     }
}

static void cec_opcode_cdc_message(UINT8 source_address, UINT8 dest_address, UINT8 *param_ptr, UINT8 param_length)
{
    /*************************************************************************
        *   CDC: Capability Discovery and Control Feature (CEC V1.4 New MSG)
        *
        *
        *************************************************************************/

    // NOT Support in LIB_CEC, add code only for CEC command recognition.
    E_CEC_MSG_ERRNO ret = CEC_MSG_UNKNOWN;
    
    ret = rx_cec_msg_cdc_message(source_address, dest_address, param_ptr, param_length);
    if(ret != CEC_MSG_OK)
    {
        return;
    }
}

void cec_msg_proc(P_CEC_RX_MSG msg)
{
    UINT32      i = 0;
    UINT8       header = 0;
    UINT8 source_address = 0;
    UINT8 dest_address = 0;
    UINT8 opcode = 0;
    UINT8 param_length = 0;
    UINT8 *param_ptr=NULL;
    BOOL        broadcast_message = FALSE;
    BOOL ret = FALSE;
    E_CEC_MSG_ERRNO tresult = CEC_MSG_UNKNOWN;

    if (NULL == msg)
    {
        return;
    }
    // Get CEC Header, Initiator Address, Message Type, Opcode and message length.
    header          = msg->data[0];
    source_address  = (header & 0xF0)>>4;
    dest_address        = (header & 0x0F);

    broadcast_message = ((header & 0x0F) == 0x0F) ? TRUE:FALSE;
    
    opcode = msg->data[1];
    param_length = msg->length-2;
    if(param_length)
    {
        param_ptr=&(msg->data[2]);
    }

    /* Filter out any weird message */
    if((!broadcast_message)&&(dest_address != api_cec_get_device_logical_address()))
    {
        CEC_PRINTF("<Error> Receive a None-Broadcast message and its destination is NOT us!\n");
        for(i=0; i<msg->length; i++)
        {
            CEC_PRINTF("0x%.2X ",msg->data[i]);
        }
        CEC_PRINTF("\n");
        return;
    }

    if(1 == msg->length)
    {
        CEC_PRINTF("RecvFrom(0x%X): <Polling Message> [None]\n", source_address);
        return;
    }

    switch(opcode)
    {
        case OPCODE_FEATURE_ABORT:
        case OPCODE_ABORT:
            tresult = cec_opcode_abort(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_IMAGE_VIEW_ON:
        case OPCODE_TEXT_VIEW_ON:
            tresult = cec_opcode_view_on(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_ACTIVE_SOURCE:
        case OPCODE_REQUEST_ACTIVE_SOURCE:
        case OPCODE_SET_STREAM_PATH:
        case OPCODE_INACTIVE_SOURCE:
        case OPCODE_ROUTING_CHANGE:
        case OPCODE_ROUTING_INFORMATION:
            tresult = cec_opcode_source(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SYSTEM_STANDBY:
            tresult = cec_opcode_system_standby(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_RECORD_OFF:
        case OPCODE_RECORD_ON:
        case OPCODE_RECORD_STATUS:
        case OPCODE_RECORD_TV_SCREEN:
            cec_opcode_record(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_CLEAR_ANALOG_TIMER:
        case OPCODE_CLEAR_DIGITAL_TIMER:
        case OPCODE_CLEAR_EXTERNAL_TIMER:
        case OPCODE_SET_ANALOG_TIMER:
        case OPCODE_SET_DIGITAL_TIMER:
        case OPCODE_SET_EXTERNAL_TIMER:
        case OPCODE_SET_TIMER_PROGRAM_TITLE:
        case OPCODE_TIMER_CLEARED_STATUS:
        case OPCODE_TIMER_STATUS:
            cec_opcode_timer(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GET_CEC_VERSION:
        case OPCODE_GIVE_PHYSICAL_ADDR:
        case OPCODE_GET_MENU_LANGUAGE:
        case OPCODE_CEC_VERSION:
        case OPCODE_REPORT_PHYSICAL_ADDR:
        case OPCODE_SET_MENU_LANGUAGE:
            tresult = cec_opcode_system_info(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_DECK_CONTROL:
        case OPCODE_DECK_STATUS:
        case OPCODE_GIVE_DECK_STATUS:
        case OPCODE_PLAY:
            cec_opcode_deck_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_TUNER_DEVICE_STATUS:
        case OPCODE_SELECT_DIGITAL_SERVICE:
        case OPCODE_SELECT_ANALOG_SERVICE:
        case OPCODE_TUNER_STEP_DECREMENT:
        case OPCODE_TUNER_STEP_INCREMENT:
        case OPCODE_TUNER_DEVICE_STATUS:
            tresult = cec_opcode_tuner_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_DEVICE_VENDOR_ID:
        case OPCODE_GIVE_DEVICE_VENDOR_ID:
        case OPCODE_VENDOR_COMMAND:
        case OPCODE_VENDOR_COMMAND_WITH_ID:
        case OPCODE_VENDOR_REOMTE_BUTTON_DOWN:
        case OPCODE_VENDOR_REOMTE_BUTTON_UP:
            tresult = cec_opcode_vendor(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_OSD_STRING: // Reply <Set OSD Name>
            tresult = cec_opcode_osd_display(source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_OSD_NAME:
        case OPCODE_SET_OSD_NAME:
            tresult = cec_opcode_osd_transfer(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_MENU_REQUEST:   // Notify UI control task to open/close menu & reply <Menu Status>
        case OPCODE_MENU_STATUS:
            tresult = cec_opcode_device_menu_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_USER_CTRL_PRESSED:
        case OPCODE_USER_CTRL_RELEASED: // Notify UI control task passthrough key
            tresult = cec_opcode_rpc(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_POWER_STATUS:
        case OPCODE_REPORT_POWER_STATUS:
            tresult = cec_opcode_power_status(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_GIVE_AUDIO_STATUS:
        case OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
        case OPCODE_REPORT_AUDIO_STATUS:
        case OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR:
        case OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR:
        case OPCODE_SET_SYSTEM_AUDIO_MODE:
        case OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
        case OPCODE_SYSTEM_AUDIO_MODE_STATUS:
            tresult = cec_opcode_system_audio_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_SET_AUDIO_RATE:
            cec_opcode_audio_rate_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_INITIATE_ARC:
        case OPCODE_REPORT_ARC_INITIATED:
        case OPCODE_REPORT_ARC_TERMINATED:
        case OPCODE_REQUEST_ARC_INITIATION:
        case OPCODE_REQUEST_ARC_TERMINATION:
        case OPCODE_TERMINAE_ARC:
            cec_opcode_arcc_audio_return_channel_control(opcode, source_address, dest_address, param_ptr, param_length);
            break;
        case OPCODE_CDC_MESSAGE:
            cec_opcode_cdc_message(source_address, dest_address, param_ptr, param_length);
            break;

        /*************************************************************************
        *  ERROR Handling for Un-supported command
        *
        *************************************************************************/
        default:
            // replay <Feature Abort> by default
            CEC_PRINTF("[LIB_CEC]: RecvFrom(0x%X): <%s> | New CEC Command is Found, need to update LIB_CEC to support \
                this command!\n", source_address, cec_opcode_str[opcode]);
            ap_cec_msg_feature_abort(source_address,  opcode, REASON_UNRECOGNIZE_OPCODE);
            break;

    }

    if(api_cec_get_osd_popup_enable_flag())
    {
        switch(opcode)
        {
            // Fix for CEC v1.4, while DUT should quick response <Device Menu Request> message
            case OPCODE_MENU_REQUEST:           //<Device Menu Control> message
            case OPCODE_MENU_STATUS:            //<Device Menu Control> message
            case OPCODE_ABORT:
            case OPCODE_USER_CTRL_PRESSED:
            case OPCODE_USER_CTRL_RELEASED:
                break;
            default:
                //Don't Display OSD when received <Device Menu Control> message
                if(tresult==CEC_MSG_OK)
                {
                    ret = api_cec_system_call(CEC_SYS_CALL_SHOW_OSD_MSG, source_address, dest_address, opcode, param_ptr,
                        param_length);
                    if(ret)
                    {
                        ali_trace(&ret);
                    }
                }
                break;
        }
    }
}

void cec_cmd_proc(P_CEC_CMD cmd)
{
    void *ptr = NULL;

    if (NULL == cmd)
    {
        return;
    }
    ptr = (void*)cmd->params;

    switch(cmd->type)
    {
        case CEC_CMD_IMAGE_VIEW_ON:
            tx_cec_msg_image_view_on();
            break;
        case CEC_CMD_TEXT_VIEW_ON:
            tx_cec_msg_text_view_on();
            break;
        case CEC_CMD_ACTIVE_SOURCE:
            tx_cec_msg_active_source(); // Broadcast Only
            break;
        case CEC_CMD_INACTIVE_SOURCE:
            tx_cec_msg_inactive_source();
            break;
        case CEC_CMD_REQUEST_ACTIVE_SOURCE:
            tx_cec_msg_request_active_source();
            break;
        case CEC_CMD_SYSTEM_STANDBY:
            tx_cec_msg_system_standby(cmd->dest_addr); // Standby All other device?
            break;
        case CEC_CMD_GET_CEC_VERSION:
            tx_cec_msg_get_cec_version(cmd->dest_addr);
            break;
        case CEC_CMD_GIVE_PHYSICAL_ADDRESS:
            tx_cec_msg_give_physical_address(cmd->dest_addr);
            break;
        case CEC_CMD_GET_MENU_LANGUAGE:         // Get TV's Menu language, UI will get callback by set language.
            tx_cec_msg_get_menu_language(cmd->dest_addr);
            break;
        case CEC_CMD_SET_MENU_LANGUAGE:
            tx_cec_msg_set_menu_language(cmd->dest_addr, (P_CEC_MENU_LANGUAGE)cmd->params);
            break;
        case CEC_CMD_GIVE_DEVICE_VENDOR_ID:
            tx_cec_msg_give_device_vendor_id(cmd->dest_addr);
            break;
        case CEC_CMD_SET_OSD_STRING:
            tx_cec_msg_set_osd_string( CEC_LA_TV, (P_CEC_OSD_STRING)cmd->params);
            break;           
        case CEC_CMD_GIVE_OSD_NAME:
            tx_cec_msg_give_osd_name(cmd->dest_addr);
            break;
        case CEC_CMD_MENU_STATUS:
            tx_cec_msg_menu_status(cmd->dest_addr, cmd->params[0]);
            break;
        case CEC_CMD_GIVE_DEVICE_POWER_STATUS:
            tx_cec_msg_give_device_power_status(cmd->dest_addr);
            break;
        case CEC_CMD_ABORT:
            tx_cec_msg_abort(cmd->dest_addr);
            break;
        case CEC_CMD_CEC_VERSION:
            tx_cec_msg_cec_version(cmd->dest_addr);
            break;
        case CEC_CMD_REPORT_PHYSICAL_ADDRESS:
            tx_cec_msg_report_physical_address();
            break;
        case CEC_CMD_POLLING_MESSAGE:
            tx_cec_msg_polling_message(cmd->dest_addr);
            break;
        case CEC_CMD_GIVE_TUNER_DEVICE_STATUS:
            tx_cec_msg_give_tuner_device_status(cmd->dest_addr, (P_CEC_STATUS_REQUEST)cmd->params);
            break;
        case CEC_CMD_TUNER_DEVICE_STATUS:   // Report Tuner Status
            tx_cec_msg_tuner_device_status(cmd->dest_addr, (P_CEC_TUNER_DEVICE_INFO)cmd->params);
            break;
        case CEC_CMD_TUNER_STEP_INCREMENT:
            tx_cec_msg_tuner_step_increment(cmd->dest_addr);
            break;
        case CEC_CMD_TUNER_STEP_DECREMENT:
            tx_cec_msg_tuner_step_decrement(cmd->dest_addr);
            break;
        case CEC_CMD_DEVICE_VENDOR_ID:
            tx_cec_msg_device_vendor_id();
            break;
        case CEC_CMD_SET_OSD_NAME:
            tx_cec_msg_set_osd_name(cmd->dest_addr, (P_CEC_OSD_NAME)cmd->params);
            break;
        case CEC_CMD_REPORT_POWER_STATUS:
            tx_cec_msg_report_power_status(cmd->dest_addr);
            break;
        case CEC_CMD_FEATURE_ABORT:
            tx_cec_msg_feature_abort(cmd->dest_addr, ((P_CEC_FEATURE_ABORT)ptr)->opcode,
               ((P_CEC_FEATURE_ABORT)ptr)->reason);
            break;
        case CEC_CMD_SYSTEM_AUDIO_MODE_REQUEST:
            tx_cec_msg_system_audio_mode_request(cmd->dest_addr, cmd->params[0]);
            break;
        case CEC_CMD_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            tx_cec_msg_give_system_audio_mode_status(cmd->dest_addr);
            break;
        case CEC_CMD_USER_CONTROL_PRESSED:
            tx_cec_msg_user_control_pressed(cmd->dest_addr, cmd->params[0]);
            break;
        case CEC_CMD_USER_CONTROL_RELEASED:
            tx_cec_msg_user_control_released(cmd->dest_addr);
            break;
        case CEC_CMD_SYSTEM_AUDIO_MODE_STATUS:
            tx_cec_msg_system_audio_mode_status(cmd->dest_addr);
            break;
        case CEC_CMD_REPORT_AUDIO_STATUS:
            tx_cec_msg_report_audio_status(cmd->dest_addr, cmd->params[0]);
            break;
        case CEC_CMD_SET_SYSTEM_AUDIO_MODE:
            tx_cec_msg_set_system_audio_mode(cmd->dest_addr, cmd->params[0]);
            break;    
        default:
            CEC_PRINTF("[LIB_CEC]: Unknown CEC_CMD_TYPE(0x%02X) from UI is found,  need to update LIB_CEC to support \
               this command!\n", cmd->type);
            break;
    }
}

