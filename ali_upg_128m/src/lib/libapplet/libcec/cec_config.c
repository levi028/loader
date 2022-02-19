/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_config.c
*
* Description:
*     Set/Get HDMI CEC global configuration.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

/*******************
* INCLUDE FILES    *
********************/
#include <osal/osal.h>
//#include <api/libcec/lib_cec.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include "cec_private.h"

#include <api/libosd/osd_lib.h>    // due to com_api.h used the OSD typedef


static CEC_CONFIG         cec_config;


BOOL api_cec_clear_last_received_feature_abort_info(void)
{
    MEMSET(&cec_config.cec_feature_abort_info, 0, sizeof(CEC_FEATURE_ABORT_INFO));
    return TRUE;
}

BOOL api_cec_set_last_received_feature_abort_info(P_CEC_FEATURE_ABORT_INFO p_cec_feature_abort_info)
{
    if(p_cec_feature_abort_info)
    {
        MEMCPY(&cec_config.cec_feature_abort_info, p_cec_feature_abort_info, sizeof(CEC_FEATURE_ABORT_INFO));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL api_cec_get_last_received_feature_abort_info(P_CEC_FEATURE_ABORT_INFO p_cec_feature_abort_info)
{
    CEC_FEATURE_ABORT_INFO temp_cec_feature_abort_info;

    MEMSET(&temp_cec_feature_abort_info, 0, sizeof(CEC_FEATURE_ABORT_INFO));

    if(p_cec_feature_abort_info)
    {

        if(0==MEMCMP(&cec_config.cec_feature_abort_info, &temp_cec_feature_abort_info, sizeof(CEC_FEATURE_ABORT_INFO)))
        {
            //last received cec feature abort info is NOT exist
            return FALSE;
        }
        else
        {

            MEMCPY(p_cec_feature_abort_info, &cec_config.cec_feature_abort_info, sizeof(CEC_FEATURE_ABORT_INFO));
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


void api_cec_set_remote_passthrough_key(UINT16 remote_cec_key)
{
    cec_config.remote_passthrough_key= remote_cec_key;
}

UINT16 api_cec_get_remote_passthrough_key(void)
{
    return cec_config.remote_passthrough_key;
}

void api_cec_set_system_audio_mode_status(BOOL bflag)
{
    cec_config.system_audio_mode_status = bflag;
}

BOOL api_cec_get_system_audio_mode_status(void)
{
    return cec_config.system_audio_mode_status;
}


void api_cec_set_system_audio_status(UINT8 status)
{
    cec_config.system_audio_status= status;
}

UINT8 api_cec_get_system_audio_status(void)
{
    return cec_config.system_audio_status;
}

void api_cec_set_remote_control_passthrough_feature(BOOL bflag)
{
    cec_config.cec_feature_rcp_enable = bflag;
}

BOOL api_cec_get_remote_control_passthrough_feature(void)
{
    return cec_config.cec_feature_rcp_enable;
}


void api_cec_set_system_audio_control_feature(BOOL bflag)
{
    cec_config.cec_feature_sac_enable = bflag;
}

BOOL api_cec_get_system_audio_control_feature(void)
{
    return cec_config.cec_feature_sac_enable;
}


void api_cec_set_func_enable(BOOL bflag)
{
    cec_config.cec_func_enable = bflag;
    api_set_hdmi_cec_onoff(bflag);
}

BOOL api_cec_get_func_enable_flag(void)
{
    return cec_config.cec_func_enable;
}


void api_cec_set_osd_popup_enable(BOOL bflag)
{
    cec_config.cec_osd_popup_enable= bflag;
}

BOOL api_cec_get_osd_popup_enable_flag(void)
{
    return cec_config.cec_osd_popup_enable;
}


void api_cec_set_source_active_state(E_CEC_SOURCE_STATE bflag)
{
    cec_config.source_active_state= bflag;
}

E_CEC_SOURCE_STATE api_cec_get_source_active_state(void)
{
    return cec_config.source_active_state;
}


void api_cec_set_device_standby_mode(E_CEC_STANDBY_MODE mode)
{
    cec_config.cec_device_standby_mode= mode;
}

E_CEC_STANDBY_MODE api_cec_get_device_standby_mode(void)
{
    return cec_config.cec_device_standby_mode;
}


BOOL api_cec_add_tuner_status_subscriber(E_CEC_LOGIC_ADDR subscriber)
{
    cec_config.tuner_status_subscribers    |= 1<<subscriber;
    return TRUE;
}

BOOL api_cec_remove_tuner_status_subscriber(E_CEC_LOGIC_ADDR subscriber)
{
    cec_config.tuner_status_subscribers    &= ~(1<<subscriber);

    return TRUE;
}

BOOL api_cec_get_tuner_status_single_subscriber_status(E_CEC_LOGIC_ADDR subscriber)
{
    return (cec_config.tuner_status_subscribers&(1<<subscriber));
}

UINT16 api_cec_get_tuner_status_all_subscriber_status(void)
{
    return cec_config.tuner_status_subscribers;
}


void api_cec_set_device_logical_address(E_CEC_LOGIC_ADDR logical_address)
{
    cec_config.device_logical_address = logical_address;
}


E_CEC_LOGIC_ADDR api_cec_get_device_logical_address(void)
{
    return cec_config.device_logical_address;
}

void api_cec_set_device_physical_address(UINT16 address)
{
    cec_config.device_physical_address = address;
}

UINT16 api_cec_get_device_physical_address(void)
{
    return cec_config.device_physical_address;
}


E_CEC_MENU_STATE api_cec_get_menu_active_state(void)
{
    return cec_config.menu_active_state;
}


void api_cec_set_menu_active_state(E_CEC_MENU_STATE menu_state)
{
    cec_config.menu_active_state = menu_state;
}

void api_cec_update_bus_physical_address_info(E_CEC_LOGIC_ADDR logical_address, UINT16 physical_address)
{
    cec_config.bus_physical_address[logical_address]=physical_address;
}


UINT16 api_cec_get_bus_physical_address_info(E_CEC_LOGIC_ADDR logical_address)
{
    return cec_config.bus_physical_address[logical_address];
}


BOOL api_cec_update_bus_active_source_info(E_CEC_LOGIC_ADDR logical_address, UINT16 physical_address)
{
    cec_config.bus_active_source_logical_address=logical_address;
    cec_config.bus_physical_address[logical_address]=physical_address;
    return TRUE;
}



void api_cec_set_channel_number_format(E_CHANNEL_NUM_FORAMT ch_num_fmt)
{
    cec_config.channel_num_format= ch_num_fmt;
}


E_CHANNEL_NUM_FORAMT api_cec_get_channel_number_format(void)
{
    return cec_config.channel_num_format;
}


void api_cec_set_service_id_method(E_SERVICE_ID_METHOD service_id_method)
{
    cec_config.service_id_method= service_id_method;
}


E_SERVICE_ID_METHOD api_cec_get_service_id_method(void)
{
    return cec_config.service_id_method;
}


BOOL api_cec_check_retrieved_valid_physical_address(void)
{
    return ((cec_config.device_physical_address!=INVALID_CEC_PHYSICAL_ADDRESS)?1:0);
}

BOOL api_cec_register_system_callback_function(CEC_EVENT_CB_FUNC_T cb_func_ptr)
{
    if(cb_func_ptr)
    {
        cec_config.event_callback=cb_func_ptr;
        return TRUE;
    }
    else
    {
        cec_config.event_callback=NULL;
        return FALSE;
    }
}

BOOL api_cec_system_call(E_CEC_SYS_CALL_TYPE request_type, E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, E_CEC_OPCODE feature_opcode, UINT8 *param, UINT8 length)
{
    if(cec_config.event_callback)
    {
        cec_config.event_callback(request_type, source_address, dest_address, feature_opcode, param, length);
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}

void api_cec_set_device_vendor_id(UINT32 vendor_id)
{
    cec_config.device_vendor_id = vendor_id&0xFFFFFF;
}

UINT32 api_cec_get_device_vendor_id(void)
{
    return cec_config.device_vendor_id;
}

void api_cec_set_active_source_logical_address(E_CEC_LOGIC_ADDR logical_address)
{
    cec_config.bus_active_source_logical_address = logical_address;
}

E_CEC_LOGIC_ADDR api_cec_get_bus_active_source_logical_address(void)
{
    return cec_config.bus_active_source_logical_address;
}

