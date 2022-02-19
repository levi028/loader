/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message_api.c
*
* Description:
*     APIs for processing HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <bus/hdmi/m36/hdmi_api.h>
#include <string.h>
#include "cec_private.h"
#include "cec_common_str.h"

/************************************************************************************************************
* ap_cec_msg_image_view_on
************************************************************************************************************/
INT32 ap_cec_msg_image_view_on(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_IMAGE_VIEW_ON, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_text_view_on
************************************************************************************************************/
INT32 ap_cec_msg_text_view_on(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_TEXT_VIEW_ON, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_active_source
************************************************************************************************************/
INT32 ap_cec_msg_active_source(void)
{
    BOOL b_ret = FALSE;
    
    api_cec_set_source_active_state(CEC_SOURCE_STATE_ACTIVE);
    //update active source related info (logical address & physical address)
    b_ret = api_cec_update_bus_active_source_info(api_cec_get_device_logical_address(), api_cec_get_device_physical_address());
    if(!b_ret)
    {
        return !SUCCESS;
    }
    return ap_cec_send_command_message(CEC_CMD_ACTIVE_SOURCE, CEC_LA_BROADCAST, NULL);
}

/************************************************************************************************************
* ap_cec_msg_inactive_source
************************************************************************************************************/
INT32 ap_cec_msg_inactive_source(E_CEC_LOGIC_ADDR dest_address)
{    
    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
    return ap_cec_send_command_message(CEC_CMD_INACTIVE_SOURCE, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_request_active_source
************************************************************************************************************/
INT32 ap_cec_msg_request_active_source(void)
{
    return ap_cec_send_command_message(CEC_CMD_REQUEST_ACTIVE_SOURCE, CEC_LA_BROADCAST, NULL);
}

/************************************************************************************************************
* ap_cec_msg_system_standby
************************************************************************************************************/
INT32 ap_cec_msg_system_standby(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_SYSTEM_STANDBY, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_cec_version
************************************************************************************************************/
INT32 ap_cec_msg_cec_version(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_CEC_VERSION, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_get_cec_version
************************************************************************************************************/
INT32 ap_cec_msg_get_cec_version(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GET_CEC_VERSION, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_give_physical_address
************************************************************************************************************/
INT32 ap_cec_msg_give_physical_address(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GIVE_PHYSICAL_ADDRESS, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_report_physical_address
************************************************************************************************************/
INT32 ap_cec_msg_report_physical_address(void)
{
    return ap_cec_send_command_message(CEC_CMD_REPORT_PHYSICAL_ADDRESS, CEC_LA_BROADCAST, NULL);
}

/************************************************************************************************************
* ap_cec_msg_get_menu_language
************************************************************************************************************/
INT32 ap_cec_msg_get_menu_language(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GET_MENU_LANGUAGE, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_set_menu_language
************************************************************************************************************/
INT32 ap_cec_msg_set_menu_language(E_CEC_LOGIC_ADDR dest_address, UINT8 *iso_639lang_str, UINT8 lang_str_len)
{
    CEC_MENU_LANGUAGE    stmenulanguage;
    UINT8 t_len = sizeof(stmenulanguage.lang);

    MEMSET(&stmenulanguage, 0x0, sizeof(CEC_MENU_LANGUAGE));
    if(!iso_639lang_str)
    {
        return !SUCCESS;
    }

    if (lang_str_len <= t_len)
    {
        MEMCPY(stmenulanguage.lang, iso_639lang_str, lang_str_len);
    }
    return ap_cec_send_command_message(CEC_CMD_SET_MENU_LANGUAGE, dest_address, (UINT8 *)&stmenulanguage);
}

/************************************************************************************************************
* ap_cec_msg_polling_message
************************************************************************************************************/
INT32 ap_cec_msg_polling_message(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_POLLING_MESSAGE, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_give_tuner_device_status
************************************************************************************************************/
INT32 ap_cec_msg_give_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_TUNER_STATUS_REQUEST status_request)
{
    CEC_STATUS_REQUEST ststatusrequest;

    MEMSET(&ststatusrequest, 0x0, sizeof(CEC_STATUS_REQUEST));
    ststatusrequest.request_type    =    status_request;

    return ap_cec_send_command_message(CEC_CMD_GIVE_TUNER_DEVICE_STATUS, dest_address, (UINT8 *)&ststatusrequest);
}

/************************************************************************************************************
* ap_cec_msg_tuner_device_status
************************************************************************************************************/
INT32 ap_cec_msg_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, CEC_TUNER_DEVICE_INFO *tuner_device_info)
{
    return ap_cec_send_command_message(CEC_CMD_TUNER_DEVICE_STATUS, dest_address, (UINT8 *)tuner_device_info);
}

/************************************************************************************************************
* ap_cec_msg_tuner_step_increment
************************************************************************************************************/
INT32 ap_cec_msg_tuner_step_increment(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_TUNER_STEP_INCREMENT, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_tuner_step_decrement
************************************************************************************************************/
INT32 ap_cec_msg_tuner_step_decrement(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_TUNER_STEP_DECREMENT, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_give_device_vendor_id
************************************************************************************************************/
INT32 ap_cec_msg_give_device_vendor_id(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GIVE_DEVICE_VENDOR_ID, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_device_vendor_id
************************************************************************************************************/
INT32 ap_cec_msg_device_vendor_id(void)
{
    return ap_cec_send_command_message(CEC_CMD_DEVICE_VENDOR_ID, CEC_LA_BROADCAST, NULL);
}

/************************************************************************************************************
* ap_cec_msg_set_osd_string
************************************************************************************************************/
INT32 ap_cec_msg_set_osd_string(E_CEC_LOGIC_ADDR dest_address, UINT8 *osd_string)
{
    UINT8                    str_len = 0;
    CEC_OSD_STRING            stosdstring;

    MEMSET(&stosdstring, 0, sizeof(CEC_OSD_STRING));

    if(osd_string)
    {
        str_len = strlen((const char *)osd_string); 

        if(CEC_OSD_STRING_LEN < str_len)
        {
            str_len = CEC_OSD_STRING_LEN;
        }
        strncpy((char *)stosdstring.string,(const char *) osd_string, str_len);

        return ap_cec_send_command_message(CEC_CMD_SET_OSD_STRING, dest_address, (UINT8 *)&stosdstring);
    }

    return RET_SUCCESS;
}

/************************************************************************************************************
* ap_cec_msg_give_osd_name
************************************************************************************************************/
INT32 ap_cec_msg_give_osd_name(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GIVE_OSD_NAME, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_set_osd_name
************************************************************************************************************/
INT32 ap_cec_msg_set_osd_name(E_CEC_LOGIC_ADDR dest_address, UINT8 *osd_name)
{
    UINT8                    str_len = 0;
    CEC_OSD_NAME            stosdname;

    MEMSET(&stosdname, 0, sizeof(CEC_OSD_NAME));

    if(osd_name)
    {
        str_len = strlen((const char *)osd_name);

        if(CEC_OSD_NAME_LEN < str_len)
        {
            str_len = CEC_OSD_NAME_LEN;
        }
        strncpy((char *)stosdname.name, (const char *)osd_name, str_len);

        return ap_cec_send_command_message(CEC_CMD_SET_OSD_NAME, dest_address, (UINT8 *)&stosdname);
    }

    return RET_SUCCESS;
}

/************************************************************************************************************
* ap_cec_msg_menu_status
************************************************************************************************************/
INT32 ap_cec_msg_menu_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_MENU_STATE menu_state)
{
    if(api_cec_get_source_active_state()==CEC_SOURCE_STATE_ACTIVE)
    {
        api_cec_set_menu_active_state(menu_state);
        return ap_cec_send_command_message(CEC_CMD_MENU_STATUS, dest_address, (UINT8 *)&menu_state);
    }
    return RET_SUCCESS;
}

/************************************************************************************************************
* ap_cec_msg_give_device_power_status
************************************************************************************************************/
INT32 ap_cec_msg_give_device_power_status(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_GIVE_DEVICE_POWER_STATUS, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_report_power_status
************************************************************************************************************/
INT32 ap_cec_msg_report_power_status(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_REPORT_POWER_STATUS, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_system_audio_mode_request
************************************************************************************************************/
INT32 ap_cec_msg_system_audio_mode_request(BOOL bonoff)
{
    return ap_cec_send_command_message(CEC_CMD_SYSTEM_AUDIO_MODE_REQUEST, CEC_LA_AUDIO_SYSTEM, (UINT8 *)&bonoff);
}

/************************************************************************************************************
* ap_cec_msg_set_system_audio_mode
************************************************************************************************************/
INT32 ap_cec_msg_set_system_audio_mode(E_CEC_LOGIC_ADDR dest_address, BOOL bonoff)
{
    if(bonoff && (dest_address==CEC_LA_BROADCAST))
    {
        api_cec_set_system_audio_mode_status(TRUE);
    }
    else
    {
        api_cec_set_system_audio_mode_status(FALSE);
    }
    return ap_cec_send_command_message(CEC_CMD_SET_SYSTEM_AUDIO_MODE, dest_address, (UINT8 *)&bonoff);
}

/************************************************************************************************************
* ap_cec_msg_give_system_audio_mode_status
************************************************************************************************************/
INT32 ap_cec_msg_give_system_audio_mode_status(void)
{
    return ap_cec_send_command_message(CEC_CMD_GIVE_SYSTEM_AUDIO_MODE_STATUS, CEC_LA_AUDIO_SYSTEM, NULL);
}

/************************************************************************************************************
* ap_cec_msg_system_audio_mode_status
************************************************************************************************************/
INT32 ap_cec_msg_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_SYSTEM_AUDIO_MODE_STATUS, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_report_audio_status
************************************************************************************************************/
INT32 ap_cec_msg_report_audio_status(E_CEC_LOGIC_ADDR dest_address, UINT8 audio_status)
{
    return ap_cec_send_command_message(CEC_CMD_REPORT_AUDIO_STATUS, dest_address, (UINT8 *)&audio_status);
}

/************************************************************************************************************
* ap_cec_msg_user_control_pressed
************************************************************************************************************/
INT32 ap_cec_msg_user_control_pressed(E_CEC_LOGIC_ADDR dest_address, E_CEC_KEY_CODE cec_key)
{
    return ap_cec_send_command_message(CEC_CMD_USER_CONTROL_PRESSED, dest_address, (UINT8 *)&cec_key);
}

/************************************************************************************************************
* ap_cec_msg_user_control_released
************************************************************************************************************/
INT32 ap_cec_msg_user_control_released(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_USER_CONTROL_RELEASED, dest_address, NULL);
}

/************************************************************************************************************
* ap_cec_msg_feature_abort
************************************************************************************************************/
INT32 ap_cec_msg_feature_abort(E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, \
    E_CEC_FEATURE_ABORT_REASON abort_reason)
{
    CEC_FEATURE_ABORT        stfeatureabort;

    MEMSET(&stfeatureabort, 0x0, sizeof(CEC_FEATURE_ABORT));
    stfeatureabort.opcode        =    feature_opcode;
    stfeatureabort.reason        =    abort_reason;

    return ap_cec_send_command_message(CEC_CMD_FEATURE_ABORT, dest_address, (UINT8 *)&stfeatureabort);
}

/************************************************************************************************************
* ap_cec_msg_abort
************************************************************************************************************/
INT32 ap_cec_msg_abort(E_CEC_LOGIC_ADDR dest_address)
{
    return ap_cec_send_command_message(CEC_CMD_ABORT, dest_address, NULL);
}

/************************************************************************************************************
* tx_cec_msg_inactive_source
************************************************************************************************************/
INT32 tx_cec_msg_inactive_source(void)
{
    UINT8 buf[4] = {0};
    E_CEC_LOGIC_ADDR l_addr = CEC_LA_TV;
    UINT16 p_addr = 0;

    /* Header & Opcode */
    l_addr = api_cec_get_device_logical_address();
    buf[0] = (l_addr<<4) | CEC_LA_TV;
    buf[1] = OPCODE_INACTIVE_SOURCE;

    /* Operand: [Physical Address] */
    p_addr = api_cec_get_device_physical_address();
    buf[2] = (UINT8) (p_addr >>8);
    buf[3] = (UINT8) (p_addr & 0xFF);

    CEC_PRINTF("SendTo(0x%X): <%s> [Physical Address=0x%.2X%.2X]\n", CEC_LA_TV, cec_opcode_str[OPCODE_INACTIVE_SOURCE],
     buf[2], buf[3]);

    return api_hdmi_cec_transmit(buf, 4);
}

/************************************************************************************************************
* tx_cec_msg_system_standby
************************************************************************************************************/
INT32 tx_cec_msg_system_standby(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR l_addr = CEC_LA_TV;
    
    /* Header & Opcode */
    buf[0] = (l_addr <<4) | dest_address;
    buf[1] = OPCODE_SYSTEM_STANDBY;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_SYSTEM_STANDBY]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_polling_message
************************************************************************************************************/
INT32 tx_cec_msg_polling_message(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf = 0;
    E_CEC_LOGIC_ADDR l_addr = CEC_LA_TV;
    
    /* Header & Opcode */
    l_addr = api_cec_get_device_logical_address();
    buf = ( l_addr <<4) | dest_address;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <Polling Message> [None]\n", dest_address);

    return api_hdmi_cec_transmit(&buf, 1);
}

/************************************************************************************************************
* tx_cec_msg_set_system_audio_mode
************************************************************************************************************/
INT32 tx_cec_msg_set_system_audio_mode(E_CEC_LOGIC_ADDR dest_address, BOOL bonoff)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR l_addr = CEC_LA_TV;

    // Send User Control Released message
    l_addr = api_cec_get_device_logical_address();
    buf[0] = (l_addr <<4) | dest_address;
    buf[1] = OPCODE_SET_SYSTEM_AUDIO_MODE;
    buf[2] = bonoff;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [System Audio Status=%s]\n", dest_address, \
        cec_opcode_str[OPCODE_SET_SYSTEM_AUDIO_MODE], (buf[2])?"On":"Off");

    return api_hdmi_cec_transmit(buf, 3);
}

