/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message_tx.c
*
* Description:
*     Processing & Sending HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <bus/hdmi/m36/hdmi_api.h>
#include <string.h>
#include "cec_private.h"
#include "cec_message_tx.h"
#include "cec_common_str.h"
#include "cec_message_rx.h"

/************************************************************************************************************
* tx_cec_msg_image_view_on
************************************************************************************************************/
INT32 tx_cec_msg_image_view_on(void)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | CEC_LA_TV;
    buf[1] = OPCODE_IMAGE_VIEW_ON;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", CEC_LA_TV, cec_opcode_str[OPCODE_IMAGE_VIEW_ON]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_text_view_on
************************************************************************************************************/
INT32 tx_cec_msg_text_view_on(void)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | CEC_LA_TV;
    buf[1] = OPCODE_TEXT_VIEW_ON;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", CEC_LA_TV, cec_opcode_str[OPCODE_TEXT_VIEW_ON]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_active_source
************************************************************************************************************/
INT32 tx_cec_msg_active_source(void)
{
    UINT8 buf[4] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    UINT16 paddr = 0;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | CEC_LA_BROADCAST;
    buf[1] = OPCODE_ACTIVE_SOURCE;

    paddr = api_cec_get_device_physical_address();
    /* Operand: [Physical Address] */
    buf[2] = (UINT8) (paddr >>8);
    buf[3] = (UINT8) (paddr & 0xFF);

    CEC_PRINTF("SendTo(0x%X): <%s> [Physical Address=0x%.2X%.2X]\n", CEC_LA_BROADCAST,
     cec_opcode_str[OPCODE_ACTIVE_SOURCE], buf[2], buf[3]);

    return api_hdmi_cec_transmit(buf, 4);
}

/************************************************************************************************************
* tx_cec_msg_request_active_source
************************************************************************************************************/
INT32 tx_cec_msg_request_active_source(void)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | CEC_LA_BROADCAST;
    buf[1] = OPCODE_REQUEST_ACTIVE_SOURCE;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", CEC_LA_BROADCAST, cec_opcode_str[OPCODE_REQUEST_ACTIVE_SOURCE]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_cec_version
************************************************************************************************************/
INT32 tx_cec_msg_cec_version(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_CEC_VERSION;

    /* Operand: [CEC Version] */
    // version 1.1: 0x00
    // version 1.2: 0x01
    // version 1.2a: 0x02
    // version 1.3: 0x03
    // version 1.3a: 0x04
    // version 1.4: 0x05
    // version 1.4a: 0x05
    buf[2] = CEC_VER_1_4A;

    CEC_PRINTF("SendTo(0x%X): <%s> [CEC Version=0x%02X]\n", dest_address, cec_opcode_str[OPCODE_CEC_VERSION], buf[2]);

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_get_cec_version
************************************************************************************************************/
INT32 tx_cec_msg_get_cec_version(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_GET_CEC_VERSION;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GET_CEC_VERSION]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_give_physical_address
************************************************************************************************************/
INT32 tx_cec_msg_give_physical_address(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_GIVE_PHYSICAL_ADDR;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GIVE_PHYSICAL_ADDR]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_report_physical_address
************************************************************************************************************/
INT32 tx_cec_msg_report_physical_address(void)
{
    UINT8 buf[5] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    UINT16 paddr = 0;
    
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | CEC_LA_BROADCAST;
    buf[1] = OPCODE_REPORT_PHYSICAL_ADDR;

    paddr = api_cec_get_device_physical_address();
    /* Operand: [Physical Address] */
    buf[2] = (UINT8) ( paddr>>8);
    buf[3] = (UINT8) (paddr & 0xFF);

    /* Operand: [Device Type] */
    buf[4] = CEC_DEV_TUNER; // "TV" =0, "Recording Device"=1, "STB"=3, "DVD"=4, "Audio"System"=5.

    CEC_PRINTF("SendTo(0x%X): <%s> [Physical Addr=0x%02x%02x][Device Type=0x%.2x]\n", CEC_LA_BROADCAST,
     cec_opcode_str[OPCODE_REPORT_PHYSICAL_ADDR], buf[2], buf[3], buf[4]);

    return api_hdmi_cec_transmit(buf, 5);
}

/************************************************************************************************************
* tx_cec_msg_get_menu_language
************************************************************************************************************/
INT32 tx_cec_msg_get_menu_language(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_GET_MENU_LANGUAGE;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GET_MENU_LANGUAGE]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_set_menu_language
************************************************************************************************************/
INT32 tx_cec_msg_set_menu_language(E_CEC_LOGIC_ADDR dest_address, P_CEC_MENU_LANGUAGE pmenulang)
{
    UINT8 buf[5] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    if (NULL == pmenulang)
    {
        return -1;
    }
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_SET_MENU_LANGUAGE;

    /* Operand: [Language] */
    buf[2]    =    pmenulang->lang[0];
    buf[3]    =    pmenulang->lang[1];
    buf[4]    =    pmenulang->lang[2];

    CEC_PRINTF("SendTo(0x%X): <%s> [Lang=0x%02X%02X%02X(\"%c%c%c\")]\n", dest_address,
     cec_opcode_str[OPCODE_SET_MENU_LANGUAGE], buf[2], buf[3], buf[4], buf[2], buf[3], buf[4]);

    return api_hdmi_cec_transmit(buf, 5);
}

/************************************************************************************************************
* tx_cec_msg_give_tuner_device_status
************************************************************************************************************/
INT32 tx_cec_msg_give_tuner_device_status(E_CEC_LOGIC_ADDR dest_address, P_CEC_STATUS_REQUEST status_request)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    if (NULL == status_request)
    {
        return -1;
    }
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_GIVE_TUNER_DEVICE_STATUS;

    /* Operand: [Tuner Device Info] */
    buf[2] = status_request->request_type;

    CEC_PRINTF("SendTo(0x%X): <%s>  [Status Request=%d]\n", dest_address,
     cec_opcode_str[OPCODE_GIVE_TUNER_DEVICE_STATUS], buf[2]);

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_tuner_device_status
************************************************************************************************************/
INT32 tx_cec_msg_tuner_device_status(UINT8 dest_address, P_CEC_TUNER_DEVICE_INFO tuner_device_info)
{
    UINT8 buf[10] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    if (NULL == tuner_device_info)
    {
        return -1;
    }
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_TUNER_DEVICE_STATUS;

    /* Operand: [Tuner Device Info] */
    MEMCPY(&buf[2], tuner_device_info, sizeof(CEC_TUNER_DEVICE_INFO));

    /* Little-Endian to Big-Endian */
    CEC_SWAP8(buf+4);
    CEC_SWAP8(buf+6);
    CEC_SWAP8(buf+8);

    CEC_PRINTF("SendTo(0x%X): <%s>  [Tuner Device Info=0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X]\n",
                dest_address, cec_opcode_str[OPCODE_TUNER_DEVICE_STATUS], buf[2], buf[3], buf[4], buf[5], buf[6],
                buf[7], buf[8], buf[9]);

    return api_hdmi_cec_transmit(buf, 10);
}

/************************************************************************************************************
* tx_cec_msg_tuner_step_increment
************************************************************************************************************/
INT32 tx_cec_msg_tuner_step_increment(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_TUNER_STEP_INCREMENT;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s>  [None]\n", dest_address, cec_opcode_str[OPCODE_TUNER_STEP_INCREMENT]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_tuner_step_decrement
************************************************************************************************************/
INT32 tx_cec_msg_tuner_step_decrement(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_TUNER_STEP_DECREMENT;

    /* Operand: [NONE] */
    CEC_PRINTF("SendTo(0x%X): <%s>  [None]\n", dest_address, cec_opcode_str[OPCODE_TUNER_STEP_DECREMENT]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_give_device_vendor_id
************************************************************************************************************/
INT32 tx_cec_msg_give_device_vendor_id(E_CEC_LOGIC_ADDR dest_address)
{
    //<Device Vendor ID>
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_GIVE_DEVICE_VENDOR_ID;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GIVE_DEVICE_VENDOR_ID]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_device_vendor_id
************************************************************************************************************/
INT32 tx_cec_msg_device_vendor_id(void)
{
    //<Device Vendor ID>
    UINT8 buf[5] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    UINT32 vid = 0;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | CEC_LA_BROADCAST;
    buf[1] = OPCODE_DEVICE_VENDOR_ID;

    /* Operand: Vendor ID[3] */
    vid = api_cec_get_device_vendor_id();
    buf[2] = (vid>>16)&0xFF;
    buf[3] = (vid>>8)&0xFF;
    buf[4] = (vid)&0xFF;

    CEC_PRINTF("SendTo(0x%X): <%s> [Vendor ID=0x%06X]\n", CEC_LA_BROADCAST, cec_opcode_str[OPCODE_DEVICE_VENDOR_ID],vid);

    return api_hdmi_cec_transmit(buf, 5);
}

/************************************************************************************************************
* tx_cec_msg_set_osd_string
************************************************************************************************************/
INT32 tx_cec_msg_set_osd_string(E_CEC_LOGIC_ADDR dest_address, P_CEC_OSD_STRING osd_string)
{
    UINT8    buf[16+1] = {0};
    UINT8    str_len = 0;
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    if (NULL == osd_string)
    {
        return -1;
    }
    /* Reset Buffer    */
    MEMSET(buf, 0, sizeof(buf));
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_SET_OSD_STRING;

    /* Operand: [Display Control][OSD String] */
    buf[2] = osd_string->display_control;
    str_len = strlen((const char *)osd_string->string);
    if(str_len > CEC_OSD_STRING_LEN)
    {
        str_len= CEC_OSD_STRING_LEN;
    }
    strncpy((char *)&buf[3], (const char *)osd_string->string, str_len);
    buf[16]='\0';    // for Debug Printf only

    CEC_PRINTF("SendTo(0x%X): <%s> [Display Control=0x%02X][OSD String=\"%s\"]\n", CEC_LA_TV,
     cec_opcode_str[OPCODE_SET_OSD_STRING], buf[2], &buf[3]);

    return api_hdmi_cec_transmit(buf, 3 + str_len);
}

/************************************************************************************************************
* tx_cec_msg_give_osd_name
************************************************************************************************************/
INT32 tx_cec_msg_give_osd_name(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_GIVE_OSD_NAME;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GIVE_OSD_NAME]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_set_osd_name
************************************************************************************************************/
INT32 tx_cec_msg_set_osd_name(E_CEC_LOGIC_ADDR dest_address, P_CEC_OSD_NAME osd_name)
{
    UINT8     buf[16+1];
    UINT8    str_len = 0;
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    if (NULL == osd_name)
    {
        return -1;
    }
    /* Reset Buffer    */
    MEMSET(buf, 0, sizeof(buf));
    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_SET_OSD_NAME;

    /* Operand: [OSD Name] */
    str_len = strlen((const char *)osd_name->name);
    if(CEC_OSD_NAME_LEN < str_len)
    {
        str_len = CEC_OSD_NAME_LEN;
    }
    strncpy((char *)&buf[2],(const char *)osd_name->name, str_len);
    buf[16]='\0';    // for Debug Printf only

    CEC_PRINTF("SendTo(0x%X): <%s> [OSD Name=\"%s\"]\n", dest_address, cec_opcode_str[OPCODE_SET_OSD_NAME], &buf[2]);

    return api_hdmi_cec_transmit(buf, 2+str_len);
}

/************************************************************************************************************
* tx_cec_msg_menu_status
************************************************************************************************************/
INT32 tx_cec_msg_menu_status(E_CEC_LOGIC_ADDR dest_address, E_CEC_MENU_STATE menu_state)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;
    
    /* Header & Opcode */
    buf[0] = (laddr<<4) | dest_address;
    buf[1] = OPCODE_MENU_STATUS;

    /* Operand: [Menu State] */
    buf[2] = menu_state;

    CEC_PRINTF("SendTo(0x%X): <%s> [Menu State=%s]\n", dest_address, cec_opcode_str[OPCODE_MENU_STATUS],
     ((menu_state==CEC_MENU_STATE_ACTIVATE) ? "[Actived]":"[Deactived]"));

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_give_device_power_status
************************************************************************************************************/
INT32 tx_cec_msg_give_device_power_status(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = ( laddr<<4) | dest_address;
    buf[1] = OPCODE_GIVE_POWER_STATUS;

    /* Operand: [None] */

    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_GIVE_POWER_STATUS]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_report_power_status
************************************************************************************************************/
INT32 tx_cec_msg_report_power_status(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_REPORT_POWER_STATUS;

    /* Operand: [Power Status] */
    // On: 0x00
    // Standby 0x01
    // In transition Standby to On: 0x02
    // In transition On to Standby: 0x03
    buf[2] = POW_ON;

    CEC_PRINTF("SendTo(0x%X): <%s> [Power Status=0x%02X]\n", dest_address, cec_opcode_str[OPCODE_REPORT_POWER_STATUS],
     POW_ON);

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_feature_abort
************************************************************************************************************/
INT32 tx_cec_msg_feature_abort(E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode,
    E_CEC_FEATURE_ABORT_REASON abort_reason)
{
    UINT8 buf[4] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    /* Header & Opcode */
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_FEATURE_ABORT;

    /* Operand: [Feature Opcode] */
    buf[2] = feature_opcode;

    /* Operand: [Feature Abort Reason] */
    buf[3] = abort_reason;

    CEC_PRINTF("SendTo(0x%X): <%s> [opcode=%s][reason=0x%02X]\n", dest_address, cec_opcode_str[OPCODE_FEATURE_ABORT],
     cec_opcode_str[feature_opcode], abort_reason);

    return api_hdmi_cec_transmit(buf, 4);
}

/************************************************************************************************************
* tx_cec_msg_abort
************************************************************************************************************/
INT32 tx_cec_msg_abort(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send abort message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_ABORT;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_ABORT]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_user_control_pressed
************************************************************************************************************/
INT32 tx_cec_msg_user_control_pressed(E_CEC_LOGIC_ADDR dest_address, E_CEC_KEY_CODE ui_command)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Pressed message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_USER_CTRL_PRESSED;
    buf[2] = ui_command;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [UI_Command=%s(0x%02X)]\n", dest_address, cec_opcode_str[OPCODE_USER_CTRL_PRESSED],
    cec_key_str[ui_command], ui_command);

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_user_control_released
************************************************************************************************************/
INT32 tx_cec_msg_user_control_released(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_USER_CTRL_RELEASED;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [None]\n", dest_address, cec_opcode_str[OPCODE_USER_CTRL_RELEASED]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_give_audio_status
************************************************************************************************************/
INT32 tx_cec_msg_give_audio_status(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_GIVE_AUDIO_STATUS;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [none]\n", dest_address, cec_opcode_str[OPCODE_GIVE_AUDIO_STATUS]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_give_system_audio_mode_status
************************************************************************************************************/
INT32 tx_cec_msg_give_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[2]= {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS;

    /* Operand: [None] */
    CEC_PRINTF("SendTo(0x%X): <%s> [none]\n", dest_address, cec_opcode_str[OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS]);

    return api_hdmi_cec_transmit(buf, 2);
}

/************************************************************************************************************
* tx_cec_msg_report_audio_status
************************************************************************************************************/
INT32 tx_cec_msg_report_audio_status(E_CEC_LOGIC_ADDR dest_address, UINT8 audio_status)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_REPORT_AUDIO_STATUS;
    buf[2] = audio_status;    // Bit7: Mute Status Bit[6-0]: Volunm 0~100

    /* Operand: [Audio Status] 1 byte, bit 7 mute status, bit6-0 volum status */
    CEC_PRINTF("SendTo(0x%X): <%s> [Audio Status=0x%02X]\n", dest_address, cec_opcode_str[OPCODE_REPORT_AUDIO_STATUS],
    buf[2]);

    return api_hdmi_cec_transmit(buf, 3);
}

/************************************************************************************************************
* tx_cec_msg_request_short_audio_descriptor
************************************************************************************************************/
INT32 tx_cec_msg_request_short_audio_descriptor(E_CEC_LOGIC_ADDR dest_address,
    E_CEC_AUDIO_FORMAT_CODE *p_afc_code, UINT8 afc_num)
{
    UINT8 buf[6] = {0};
    UINT8 i = 0;
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    if (NULL == p_afc_code)
    {
        return -1;
    }
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR;

    if(CEC_MAX_AFC_NUM < afc_num)
    {
        // MAX up to 4 AFC(s)
        afc_num = CEC_MAX_AFC_NUM;
    }

    for(i=0; i<afc_num; i++)
    {
        buf[2+i] = p_afc_code[i]&0x3F;
    }

    /* Operand: [Audio Format ID and Code] 1 byte,  up to 4 AFC, Audio Format ID[7-6],     Audio Format Code[5-0]*/
    CEC_PRINTF("SendTo(0x%X): <%s> ", dest_address, cec_opcode_str[OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR]);

    for(i=0; i<afc_num; i++)
    {
        CEC_PRINTF("AFC[%d]=0x%02X ", i, buf[2+i] );
    }

    CEC_PRINTF("\n");

    return api_hdmi_cec_transmit(buf, 2+afc_num);
}

/************************************************************************************************************
* tx_cec_msg_report_short_audio_descriptor
************************************************************************************************************/
INT32 tx_cec_msg_report_short_audio_descriptor(E_CEC_LOGIC_ADDR dest_address, UINT8 *p_short_audio_desc, \
    UINT8 short_audio_desc_num)
{
    UINT8 buf[14] = {0};
    UINT8 i = 0;
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    if (NULL == p_short_audio_desc)
    {
        return -1;
    }
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR;

    if(CEC_MAX_SHORT_AD_NUM < short_audio_desc_num)
    {
        // MAX up to 4 short audio descriptor(s)
        short_audio_desc_num = CEC_MAX_SHORT_AD_NUM;//4;
    }

    for(i=0; i<short_audio_desc_num; i++)
    {
        buf[2+i*3] = p_short_audio_desc[i*3];
        buf[2+i*3+1] = p_short_audio_desc[i*3+1];
        buf[2+i*3+2] = p_short_audio_desc[i*3+2];
    }

    /* Operand: [Short Audio Descriptor] 3Byte, up to 4 short audio descriptor*/
    CEC_PRINTF("SendTo(0x%X): <%s> ", dest_address, cec_opcode_str[OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR], buf[2]);
    for(i=0; i<short_audio_desc_num; i++)
    {
        CEC_PRINTF("Desc[%d]={0x%02X 0x%02X 0x%02X} ", i, buf[2+i*3], buf[2+i*3+1], buf[2+i*3+2]);
    }
    CEC_PRINTF("\n");

    return api_hdmi_cec_transmit(buf, 2+short_audio_desc_num*3);
}

/************************************************************************************************************
* tx_cec_msg_system_audio_mode_request
************************************************************************************************************/
INT32 tx_cec_msg_system_audio_mode_request(E_CEC_LOGIC_ADDR dest_address, BOOL bsacmodeon)
{
    UINT8    buf[4] = {0};
    UINT8 data_len = 0;
    UINT16    physical_address = 0;
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_SYSTEM_AUDIO_MODE_REQUEST;

    if(bsacmodeon)
    {
        // Enable SAC mode
        physical_address = api_cec_get_bus_physical_address_info(api_cec_get_bus_active_source_logical_address());
        buf[2] = (physical_address>>8)&0xFF;
        buf[3] = physical_address&0xFF;
        data_len = 4;

        /* Operand: [Physical Address] 2 byte */
        CEC_PRINTF("SendTo(0x%X): <%s> [Physical Address=0x%02x%02x]\n", dest_address, \
            cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_REQUEST], buf[2], buf[3]);
    }
    else
    {
        // Disable SAC mode
        data_len = 2;

        /* Operand: [Physical Address] 2 byte */
        CEC_PRINTF("SendTo(0x%X): <%s> [none]\n", dest_address, cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_REQUEST]);
    }

    return api_hdmi_cec_transmit(buf, data_len);
}

/************************************************************************************************************
* tx_cec_msg_system_audio_mode_status
************************************************************************************************************/
INT32 tx_cec_msg_system_audio_mode_status(E_CEC_LOGIC_ADDR dest_address)
{
    UINT8 buf[3] = {0};
    E_CEC_LOGIC_ADDR laddr = CEC_LA_BROADCAST;

    laddr = api_cec_get_device_logical_address();
    // Send User Control Released message
    buf[0] = (laddr <<4) | dest_address;
    buf[1] = OPCODE_SYSTEM_AUDIO_MODE_STATUS;
    buf[2] = api_cec_get_system_audio_mode_status();  // On:1 Off:0

    /* Operand: [System Audio Status] 1 byte */
    CEC_PRINTF("SendTo(0x%X): <%s> [System Audio Status=%s]\n", dest_address, \
        cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_STATUS], (buf[2])?"On":"Off");

    return api_hdmi_cec_transmit(buf, 3);
}

