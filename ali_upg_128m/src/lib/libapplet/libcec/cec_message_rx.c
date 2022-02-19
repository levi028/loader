/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message_rx.c
*
* Description:
*     Receiving & Processing HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "cec_private.h"
#include "cec_message_rx.h"
#include "cec_common_str.h"

extern void ali_trace(void *pmsg);

void CEC_SWAP8(UINT8 *p)
{
    UINT8 tmp = 0;

    if(!p)
    {
        return;
    }
    tmp = *(p + 1);
    *(p + 1) = *p;
    *p = tmp;
}

E_CEC_MSG_ERRNO rx_cec_msg_image_view_on(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address!=CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_IMAGE_VIEW_ON]);
            // Ignore the MSG
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_IMAGE_VIEW_ON], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_IMAGE_VIEW_ON], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_text_view_on(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address!=CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_TEXT_VIEW_ON]);
            // Ignore the MSG
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_TEXT_VIEW_ON], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_TEXT_VIEW_ON], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_active_source(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address,
    UINT8 *param, UINT16 param_length)
{
    UINT16 physical_address = 0;
    
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_TWO == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Physical Address=0x%02X%02X]\n", source_address ,
                    cec_opcode_str[OPCODE_ACTIVE_SOURCE], param[0], param[1]);

                // Got ActiveSource from other source, should become inactive source state
                if(api_cec_get_source_active_state()==CEC_SOURCE_STATE_ACTIVE)
                {
                    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
                }
                //update active source info (physical & logical address)
                physical_address = ((param[0]<<8)|(param[1]));
                api_cec_update_bus_active_source_info(source_address, physical_address);
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_ACTIVE_SOURCE], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address, \
               cec_opcode_str[OPCODE_ACTIVE_SOURCE], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_ACTIVE_SOURCE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_request_active_source(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_REQUEST_ACTIVE_SOURCE]);

            // replay Active Source if in active if in Active Source state
            if(api_cec_get_source_active_state()==CEC_SOURCE_STATE_ACTIVE)
            {
                ap_cec_msg_active_source();
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_REQUEST_ACTIVE_SOURCE], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_REQUEST_ACTIVE_SOURCE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }            
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_set_stream_path(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_TWO == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Physical Address=0x%02X%02X]\n", source_address, \
               cec_opcode_str[OPCODE_SET_STREAM_PATH], param[0], param[1]);

            // ? got set stream path, destination is our STB, should become active source.
            if(    api_cec_get_device_physical_address() == ((UINT16) (param[0]<<8) | (param[1])))
            {
                api_cec_set_source_active_state(CEC_SOURCE_STATE_ACTIVE);
                ap_cec_msg_active_source();
            }
            else
            {
                /* The Destination is NOT our STB*/
                api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_SET_STREAM_PATH], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_SET_STREAM_PATH], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_inactive_source(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address!=CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_TWO ==  param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Physical Address=0x%02X%02X]\n", source_address, \
               cec_opcode_str[OPCODE_INACTIVE_SOURCE], param[0], param[1]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_INACTIVE_SOURCE], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_INACTIVE_SOURCE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_routing_change(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_FOUR == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Original Address=0x%02X%02X][New Address=0x%02X%02X]\n", source_address,
               cec_opcode_str[OPCODE_ROUTING_CHANGE], param[0], param[1], param[2], param[3]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_ROUTING_CHANGE], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_ROUTING_CHANGE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_routing_information(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_TWO == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Physical Address=0x%02X%02X]\n", source_address, \
               cec_opcode_str[OPCODE_ROUTING_INFORMATION], param[0], param[1]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_ROUTING_INFORMATION], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_ROUTING_INFORMATION], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_system_standby(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(CEC_MSG_PARAM_LEN_ZERO == param_length)
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_SYSTEM_STANDBY]);
        b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, OPCODE_SYSTEM_STANDBY, param,
          param_length);
        if(b_ret)
        {
            ali_trace(&b_ret);
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address,  \
          cec_opcode_str[OPCODE_SYSTEM_STANDBY], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_get_cec_version(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address,  cec_opcode_str[OPCODE_GET_CEC_VERSION]);

            /*    Directly addressed    */
            // response <CEC Version>
            ap_cec_msg_cec_version( source_address );
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_GET_CEC_VERSION], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GET_CEC_VERSION], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_give_physical_address(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GIVE_PHYSICAL_ADDR]);

            // response <Report Physical Address>
            ap_cec_msg_report_physical_address();
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_GIVE_PHYSICAL_ADDR], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GIVE_PHYSICAL_ADDR], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_get_menu_language(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GET_MENU_LANGUAGE]);
            //CEC v1.4 New Spec, response with Feature Abort
            ap_cec_msg_feature_abort(source_address, OPCODE_GET_MENU_LANGUAGE, REASON_REFUSED);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_GET_MENU_LANGUAGE], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GET_MENU_LANGUAGE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_cec_version(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(CEC_MSG_PARAM_LEN_ONE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [CEC Version=0x%02X]\n", source_address,  \
               cec_opcode_str[OPCODE_CEC_VERSION], param[0]);
                // store the CEC version info?
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address,  \
               cec_opcode_str[OPCODE_CEC_VERSION], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address,  \
          cec_opcode_str[OPCODE_CEC_VERSION], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_report_physical_address(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT16    physical_address = 0;
    
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_THREE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Physical Address=0x%02X%02X][Device Type=0x%02X]\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_PHYSICAL_ADDR], param[0], param[1], param[2]);

            // store the Physical Address info?
            physical_address = ((param[0]<<8)|(param[1]));
            api_cec_update_bus_physical_address_info(source_address, physical_address);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_REPORT_PHYSICAL_ADDR], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_REPORT_PHYSICAL_ADDR], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_set_menu_language(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(source_address==CEC_LA_TV)
        {
            if(CEC_MSG_PARAM_LEN_THREE == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                CEC_PRINTF("RecvFrom(0x%X): <Set Menu Language> [Lang=0x%02X%02X%02X(\"%c%c%c\")]\n", source_address, \
                    cec_opcode_str[OPCODE_SET_MENU_LANGUAGE], param[0], param[1],param[2],param[0], param[1], param[2]);

                // Set Langugage code ISO/FDIS 639-2, http://www.loc.gov/standards/iso639-2/langhome.html
                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, OPCODE_SET_MENU_LANGUAGE,
                param, param_length);
                if(!b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <Set Menu Language>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_SET_MENU_LANGUAGE], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <Set Menu Language>, ERROR! (source_address=0x%X) \n", source_address, \
               cec_opcode_str[OPCODE_SET_MENU_LANGUAGE], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <Set Menu Language>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_SET_MENU_LANGUAGE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_give_tuner_device_status(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_ONE == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Status Request=0x%02X]\n", source_address, \
                    cec_opcode_str[OPCODE_GIVE_TUNER_DEVICE_STATUS], param[0]);

                /*
                    [Status Request]:     1 = "On",  2 = "Off", 3 = "Once"
                */
                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
                OPCODE_GIVE_TUNER_DEVICE_STATUS, param, param_length);
                if(b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_GIVE_TUNER_DEVICE_STATUS], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address,
               cec_opcode_str[OPCODE_GIVE_TUNER_DEVICE_STATUS], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GIVE_TUNER_DEVICE_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_select_digital_service(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_SEVEN == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Digital Service ID=0x%02X %02X %02X %02X %02X %02X %02X]\n", \
                    source_address, cec_opcode_str[OPCODE_SELECT_DIGITAL_SERVICE], param[0], param[1], param[2], \
                    param[3], param[4], param[5], param[6]);

                /* Little-Endian to Big-Endian */
                CEC_SWAP8(param+1);
                CEC_SWAP8(param+3);
                CEC_SWAP8(param+5);
                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, \
                    OPCODE_SELECT_DIGITAL_SERVICE, param, param_length);
                if(b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_SELECT_DIGITAL_SERVICE], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address, \
               cec_opcode_str[OPCODE_SELECT_DIGITAL_SERVICE], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_SELECT_DIGITAL_SERVICE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_select_analog_service(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_FOUR == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Analog Bcast Type=0x%02X][Analog Freq=0x%02X%02X]\
                    [Bcast System=0x%02X]\n", source_address, cec_opcode_str[OPCODE_SELECT_ANALOG_SERVICE], param[0],
                    param[1], param[2], param[3]);

                // Do NOT accept such functionality in our STB
                ap_cec_msg_feature_abort(source_address, OPCODE_SELECT_ANALOG_SERVICE, REASON_REFUSED);
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_SELECT_ANALOG_SERVICE], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address, \
               cec_opcode_str[OPCODE_SELECT_ANALOG_SERVICE], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_SELECT_ANALOG_SERVICE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_tuner_step_decrement(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_ZERO == param_length)
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address,cec_opcode_str[OPCODE_TUNER_STEP_DECREMENT]);

                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
                    OPCODE_TUNER_STEP_DECREMENT, param, param_length);
                if(b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
                    cec_opcode_str[OPCODE_TUNER_STEP_DECREMENT], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address, \
               cec_opcode_str[OPCODE_TUNER_STEP_DECREMENT], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_TUNER_STEP_DECREMENT], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_tuner_step_increment(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_ZERO == param_length)
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n",source_address, cec_opcode_str[OPCODE_TUNER_STEP_INCREMENT]);
                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
                    OPCODE_TUNER_STEP_INCREMENT, param, param_length);
                if(b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address,
                    cec_opcode_str[OPCODE_TUNER_STEP_INCREMENT], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X) \n", source_address,
               cec_opcode_str[OPCODE_TUNER_STEP_INCREMENT], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address,
          cec_opcode_str[OPCODE_TUNER_STEP_INCREMENT], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_tuner_device_status(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    // store the query result if STB indeed issued an query, otherwise drop it!
    if(    dest_address == api_cec_get_device_logical_address() )
    {
        if(CEC_MSG_PARAM_LEN_EIGHT == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Digital Tuner Device Info=0x%02X %02X %02X %02X %02X %02X %02X %02X]\n", \
               source_address, cec_opcode_str[OPCODE_TUNER_DEVICE_STATUS], param[0], param[1], param[2], param[3],
               param[4], param[5], param[6], param[7]);
            // store digital tuner status info?
        }
        else if(CEC_MSG_PARAM_LEN_FIVE == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Analog Tuner Device Info=0x%02X %02X %02X %02X %02X %02X %02X %02X]\n", \
               source_address, cec_opcode_str[OPCODE_TUNER_DEVICE_STATUS],param[0],param[1],param[2],param[3],param[4]);
            // store analog tuner status info?
            return CEC_MSG_OK;
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_TUNER_DEVICE_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_TUNER_DEVICE_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_device_vendor_id(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address==CEC_LA_BROADCAST)
    {
        if(CEC_MSG_PARAM_LEN_THREE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Vendor ID=0x%02X%02X%02X]\n", source_address, \
               cec_opcode_str[OPCODE_DEVICE_VENDOR_ID], param[0], param[1], param[2]);
            // store vendor id?
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_DEVICE_VENDOR_ID], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=%X) \n", source_address, \
          cec_opcode_str[OPCODE_DEVICE_VENDOR_ID], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_give_device_vendor_id(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GIVE_DEVICE_VENDOR_ID]);

            // Reply <Device Vendor ID>, [Vendor ID] = 3bytes (24bit)
            ap_cec_msg_device_vendor_id();
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_GIVE_DEVICE_VENDOR_ID], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GIVE_DEVICE_VENDOR_ID], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_vendor_command(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())        // Directly addressed
    {
        if((param_length <= CEC_MSG_PARAM_LEN_FOURTEEN) && (param_length >= CEC_MSG_PARAM_LEN_ONE))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Vendor Data=", source_address, cec_opcode_str[OPCODE_VENDOR_COMMAND]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("0x%02X ", param[i]);
            }
            CEC_PRINTF("]\n");
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
            cec_opcode_str[OPCODE_VENDOR_COMMAND], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_VENDOR_COMMAND], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_vendor_command_with_id(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())        // Directly addressed
    {
        if(param_length <= CEC_MSG_PARAM_LEN_FOURTEEN)
        {
            if(!param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            //Vendor ID= 3 byte, Vendor Data <=11 byte

            CEC_PRINTF("RecvFrom(0x%X): <%s> [Vendor ID=0x%02X%02X%02X] [Vendor data=", source_address, \
            cec_opcode_str[OPCODE_VENDOR_COMMAND_WITH_ID], param[0], param[1], param[2]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("0x%02X ", param[i]);
            }
            CEC_PRINTF("]\n");
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
            cec_opcode_str[OPCODE_VENDOR_COMMAND_WITH_ID], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_VENDOR_COMMAND_WITH_ID], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_vendor_remote_button_down(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_FOURTEEN >= param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X):<%s> [RC Code=",source_address,cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_DOWN]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("0x%02X ", param[i]);
            }
            CEC_PRINTF("]\n");
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
            cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_DOWN], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_DOWN], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_vendor_remote_button_up(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_FOURTEEN >= param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [RC Code=",source_address,cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_UP]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("0x%02X ", param[i]);
            }
            CEC_PRINTF("]\n");
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
            cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_UP], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_VENDOR_REOMTE_BUTTON_UP], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

