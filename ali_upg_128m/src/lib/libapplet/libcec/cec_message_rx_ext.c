/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_message_rx_ext.c
*
* Description:
*     Another file for Receiving & Processing HDMI CEC messages.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "cec_private.h"
#include "cec_message_rx.h"
#include "cec_common_str.h"
#include "cec_message_tx.h"

extern void ali_trace(void *pmsg);

/*******************************************************************************
*    Local function implement
*******************************************************************************/

/************************************************************************************************************
* Rx_CEC_MSG_CDC_Message
************************************************************************************************************/
static E_CEC_MSG_ERRNO rx_cdc_hec_msg_inquire_state(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_FOUR == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HEC_MSG: <%s> [Physical Address=0x%02X%02X][Physical Address=0x%02X%02X]\n", \
          cdc_opcode_str[CDC_HEC_INQUIRE_STATE], param[0], param[1], param[2], param[3]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR!(param_length=%d)\n", cdc_opcode_str[CDC_HEC_INQUIRE_STATE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_report_state(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_FIVE == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HEC_MSG: <%s> [Physical Address=0x%02X%02X][HEC State=0x%02X][HEC Support Field=0x%02X%02X]\n",\
          cdc_opcode_str[CDC_HEC_REPORT_STATE], param[0], param[1], param[2], param[3], param[4]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_REPORT_STATE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_set_state_adjacent(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_THREE == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HEC_MSG: <%s> [Physical Address=0x%02X%02X][HEC Set State=0x%02X]\n",
          cdc_opcode_str[CDC_HEC_SET_STATE_ADJACENT], param[0], param[1], param[2]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_SET_STATE_ADJACENT], \
          param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_set_state(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_ELEVEN == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HEC_MSG: <%s> [Physical Address=0x%02X%02X][Physical Address=0x%02X%02X][HEC Set State=0x%02X]\
          [Physical Address=0x%02X%02X][Physical Address=0x%02X%02X][Physical Address=0x%02X%02X]\n",
          cec_opcode_str[CDC_HEC_SET_STATE], param[0], param[1], param[2], param[3], param[4], param[5], param[6],
          param[7], param[8], param[9], param[10]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_SET_STATE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_request_deactivation(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_SIX == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HEC_MSG: <%s> [Physical Address=0x%02X%02X][Physical Address=0x%02X%02X]\
          [Physical Address=0x%02X%02X]\n", cdc_opcode_str[CDC_HEC_REQUEST_DEACTIVATION], param[0], param[1], param[2],\
          param[3], param[4], param[5]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_REQUEST_DEACTIVATION], \
          param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_notify_alive(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_ZERO == param_length)
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s> [None]\n", cdc_opcode_str[CDC_HEC_NOTIFY_ALIVE]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_NOTIFY_ALIVE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hec_msg_discover(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_ZERO == param_length)
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s> [None]\n", cdc_opcode_str[CDC_HEC_DISCOVER]);
    }
    else
    {
        CEC_PRINTF("CDC_HEC_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HEC_DISCOVER], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hpd_msg_set_state(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_ONE == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HPD_MSG: <%s> [Input port number=0x%X][HPD State=0x%X]\n", cdc_opcode_str[CDC_HPD_SET_STATE], \
          (param[0]>>4)&0xF, (param[0])&0xF);
    }
    else
    {
        CEC_PRINTF("CDC_HPD_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HPD_SET_STATE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO rx_cdc_hpd_msg_report_state(UINT8 *param, UINT8 param_length)
{
    if(CEC_MSG_PARAM_LEN_ONE == param_length)
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("CDC_HPD_MSG: <%s> [HPD State=0x%X][CDC_HPD_Errot_Code=0x%X]\n",
          cdc_opcode_str[CDC_HPD_REPORT_STATE], (param[0]>>4)&0xF, (param[0])&0xF);
    }
    else
    {
        CEC_PRINTF("CDC_HPD_MSG: <%s>, ERROR! (param_length=%d)\n", cdc_opcode_str[CDC_HPD_REPORT_STATE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }
    return CEC_MSG_OK;
}

static E_CEC_MSG_ERRNO _cec_msg_cdc_message_parser(E_CDC_OPCODE cdc_opcode, UINT8 *param, UINT8 param_length)
{
    E_CEC_MSG_ERRNO    tresult = CEC_MSG_UNKNOWN;

    switch(cdc_opcode)
    {
        case CDC_HEC_INQUIRE_STATE:
            tresult = rx_cdc_hec_msg_inquire_state(param, param_length);
            break;
        case CDC_HEC_REPORT_STATE:
            tresult = rx_cdc_hec_msg_report_state(param, param_length);
            break;
        case CDC_HEC_SET_STATE_ADJACENT:
            tresult = rx_cdc_hec_msg_set_state_adjacent(param, param_length);
            break;
        case CDC_HEC_SET_STATE    :
            tresult = rx_cdc_hec_msg_set_state(param, param_length);
            break;
        case CDC_HEC_REQUEST_DEACTIVATION    :
            tresult = rx_cdc_hec_msg_request_deactivation(param, param_length);
            break;
        case CDC_HEC_NOTIFY_ALIVE:
            tresult = rx_cdc_hec_msg_notify_alive(param, param_length);
            break;
        case CDC_HEC_DISCOVER:
            tresult = rx_cdc_hec_msg_discover(param, param_length);
            break;
        case CDC_HPD_SET_STATE:
            tresult = rx_cdc_hpd_msg_set_state(param, param_length);
            break;
        case CDC_HPD_REPORT_STATE:
            tresult = rx_cdc_hpd_msg_report_state(param, param_length);
            break;
        default:
            CEC_PRINTF("[LIB_CEC]: New CEC_CDC Command is Found, need to update LIB_CEC to support this command!\n", \
               cdc_opcode);
            tresult = CEC_MSG_OK;
            break;
    }
    if(tresult != CEC_MSG_OK)
    {
        return tresult;
    }
    return tresult;
}

E_CEC_MSG_ERRNO rx_cec_msg_set_osd_string(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())    // Directly addressed to me
    {
        if((param_length <= CEC_MSG_PARAM_LEN_FOURTEEN) && (param_length >= CEC_MSG_PARAM_LEN_TWO))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Display Control=0x%02X][OSD String=\"", source_address, \
               cec_opcode_str[OPCODE_SET_OSD_STRING], param[0]);
            for(i=1; i<param_length; i++)
            {
                CEC_PRINTF("%c", param[i]);
            }
            CEC_PRINTF("\"]\n");
            // Disp Contro = 1 byte, OSD String <=13 bytes
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
            cec_opcode_str[OPCODE_SET_OSD_STRING], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
        cec_opcode_str[OPCODE_SET_OSD_STRING], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_give_osd_name(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(dest_address == api_cec_get_device_logical_address())
    {
        if(source_address!=CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_ZERO == param_length)
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GIVE_OSD_NAME]);
                ap_cec_msg_set_osd_name( source_address, (UINT8 *)"ALi STB" );
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                    cec_opcode_str[OPCODE_GIVE_OSD_NAME], param_length);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X)\n", source_address, \
               cec_opcode_str[OPCODE_GIVE_OSD_NAME], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
          cec_opcode_str[OPCODE_GIVE_OSD_NAME], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_set_osd_name(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if(dest_address == api_cec_get_device_logical_address())    // Directly addressed to me
    {
        if((param_length<=CEC_MSG_PARAM_LEN_FOURTEEN) && (param_length>=CEC_MSG_PARAM_LEN_ONE))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [OSD Name=\"", source_address, cec_opcode_str[OPCODE_SET_OSD_NAME]);

            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("%c", param[i]);
            }
            CEC_PRINTF("\"]\n");
            //  OSD Name <=14 bytes
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_SET_OSD_NAME], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address, \
        cec_opcode_str[OPCODE_SET_OSD_NAME], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_menu_request(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if( dest_address == api_cec_get_device_logical_address() )
    {
        if( source_address != CEC_LA_BROADCAST)
        {
            if(CEC_MSG_PARAM_LEN_ONE == param_length)
            {
                if (NULL == param)
                {
                    return CEC_MSG_ERR_PARAM_VALUE;
                }
                //    [Menu Request Type]: 0 "Activated", 1 "Deactivated", 2 "Query"
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Menu Request Type=0x%02X]\n", source_address, \
                cec_opcode_str[OPCODE_MENU_REQUEST], param[0]);
                b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
                    OPCODE_MENU_REQUEST, param, param_length);
                if(b_ret)
                {
                    ali_trace(&b_ret);
                }
            }
            else
            {
                // strange situation , ignore this message
                CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                    cec_opcode_str[OPCODE_MENU_REQUEST], param_length);
                //Tx_CEC_MSG_Feature_Abort(source_address, opcode, REASON_INVALID_OPRAND);
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (source_address=0x%X)\n", source_address, \
                cec_opcode_str[OPCODE_MENU_REQUEST], source_address);
            return CEC_MSG_ERR_SRC_ADDR;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_MENU_REQUEST], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_menu_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )
    {
        if(CEC_MSG_PARAM_LEN_ONE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Menu State=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_MENU_STATUS], param[0]);
            // Store the menu status information?
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_MENU_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_MENU_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_user_control_pressed(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if( dest_address == api_cec_get_device_logical_address() )
    {
        if(CEC_MSG_PARAM_LEN_ONE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [UI Command=%s(0x%02X)]\n", source_address, \
               cec_opcode_str[OPCODE_USER_CTRL_PRESSED], cec_key_str[param[0]], param[0]);

            // Record the pressed key and callback to UI layer when get release message.
            // Need to Consider SAC CECT: 11.2.15-8
            api_cec_set_remote_passthrough_key(param[0]);
            b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, OPCODE_USER_CTRL_PRESSED,
               param, param_length);
            if(b_ret)
            {
                ali_trace(&b_ret);
            }
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_USER_CTRL_PRESSED], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_USER_CTRL_PRESSED], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_user_control_released(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    
    if( dest_address == api_cec_get_device_logical_address() )
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_USER_CTRL_RELEASED]);
            api_cec_set_remote_passthrough_key(0xFFFF);
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_USER_CTRL_RELEASED], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_USER_CTRL_RELEASED], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_give_power_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GIVE_POWER_STATUS]);
            ap_cec_msg_report_power_status(source_address );
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_GIVE_POWER_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address, OPCODE_GIVE_POWER_STATUS, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_GIVE_POWER_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_report_power_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )
    {
        if(CEC_MSG_PARAM_LEN_ONE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Power Status=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_REPORT_POWER_STATUS], param[0]);

            // store the power status
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_REPORT_POWER_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_POWER_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_feature_abort(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    CEC_FEATURE_ABORT_INFO    feature_abort_info;
    BOOL b_ret = FALSE;
    
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_TWO == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [opcode=%s][operand=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_FEATURE_ABORT], cec_opcode_str[param[0]], param[1]);

            // store the feature abort result
            feature_abort_info.source_address=source_address;
            feature_abort_info.dest_address=dest_address;
            feature_abort_info.opcode=param[0];
            feature_abort_info.reason=param[1];
            b_ret = api_cec_set_last_received_feature_abort_info(&feature_abort_info);
            if(!b_ret)
            {
                ali_trace(&b_ret);
            }
        }
        else
        {
            // strange situation , ignore this message
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
                cec_opcode_str[OPCODE_FEATURE_ABORT], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_FEATURE_ABORT], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_abort(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, UINT8 *param, \
    UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_ABORT]);

            //Reply with <Feature Abort> directly, don't via queue
            tx_cec_msg_feature_abort(source_address,  OPCODE_ABORT, REASON_UNRECOGNIZE_OPCODE);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s?, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_ABORT], param_length);
            ap_cec_msg_feature_abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, cec_opcode_str[OPCODE_ABORT], \
            dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_give_audio_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_give_audio_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_GIVE_AUDIO_STATUS]);
            // this is a message for AMP, since we are STB, so drop this message.

            b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, OPCODE_GIVE_AUDIO_STATUS,
                param, param_length);
            if(b_ret)
            {
                ali_trace(&b_ret);
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address,
               cec_opcode_str[OPCODE_GIVE_AUDIO_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message 
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address,
            cec_opcode_str[OPCODE_GIVE_AUDIO_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_give_system_audio_mode_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_give_system_audio_mode_status(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
 
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_ZERO == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, \
               cec_opcode_str[OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS]);
            // this is a message for AMP, since we are STB, so drop this message.

            api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
                OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS, param, param_length);

        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS], param_length);
            ap_cec_msg_feature_abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

//NOT YET VERIFIED
/************************************************************************************************************
* rx_cec_msg_report_audio_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_report_audio_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(CEC_MSG_PARAM_LEN_ONE == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Audio Status=%d]\n", source_address, \
               cec_opcode_str[OPCODE_REPORT_AUDIO_STATUS], param[0]);

            api_cec_set_system_audio_status(param[0]);

            CEC_PRINTF("-AMP Mute=%d\n",     api_cec_get_system_audio_status() &0x80);
            CEC_PRINTF("-AMP Volumn=%d\n",    api_cec_get_system_audio_status() &0x7F);
        }
        else
        {

            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REPORT_AUDIO_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_AUDIO_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_report_short_audio_descriptor
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_report_short_audio_descriptor(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if((CEC_MSG_PARAM_LEN_THREE == param_length) || (CEC_MSG_PARAM_LEN_SIX == param_length) \
          ||(CEC_MSG_PARAM_LEN_NINE == param_length) || (CEC_MSG_PARAM_LEN_TWELVE == param_length))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Short Audio Descriptor = \n", source_address, \
               cec_opcode_str[OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR]);

            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("0x%02X ", param[i]);
            }
            CEC_PRINTF("]\n");
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address,
               cec_opcode_str[OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_request_short_audio_descriptor
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_request_short_audio_descriptor(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length <= CEC_MSG_PARAM_LEN_FOUR)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s>\n", source_address, cec_opcode_str[OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("-AFC[%d]=\"%s\"\n", i, cec_short_audio_desc_str[param[i]]);
            }

            // If the requested AFC is NOT support in STB, we should reply with <Feature Abort>
            ap_cec_msg_feature_abort(source_address, OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR, REASON_INVALID_OPRAND);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_system_audio_mode
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_system_audio_mode(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if(param_length == CEC_MSG_PARAM_LEN_ONE)
    {
        BOOL b_ret = FALSE;
        
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        CEC_PRINTF("RecvFrom(0x%X): <%s> [System Audio Status=%s]\n", source_address, \
          cec_opcode_str[OPCODE_SET_SYSTEM_AUDIO_MODE], (param[0])?"On":"Off");

        b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address, OPCODE_SET_SYSTEM_AUDIO_MODE,
          param, param_length);
         if(b_ret)
         {
            ali_trace(&b_ret);
         }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s, ERROR! (param_length=%d)\n", source_address, \
          cec_opcode_str[OPCODE_SET_SYSTEM_AUDIO_MODE], param_length);
        return CEC_MSG_ERR_PAYLOAD_LENGTH;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_system_audio_mode_request
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_system_audio_mode_request(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    BOOL b_ret = FALSE;
    
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if((param_length == CEC_MSG_PARAM_LEN_TWO) || (param_length== CEC_MSG_PARAM_LEN_ZERO))
        {
            b_ret = api_cec_system_call(CEC_SYS_CALL_BY_CEC_OPCODE, source_address, dest_address,
               OPCODE_SYSTEM_AUDIO_MODE_REQUEST, param, param_length);
            if(b_ret)
            {
                ali_trace(&b_ret);
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_REQUEST], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_REQUEST], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_system_audio_mode_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_system_audio_mode_status(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [System Audio Status=%s]\n", source_address, \
               cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_STATUS], (param[0])?"On":"Off");
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SYSTEM_AUDIO_MODE_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
*    <START> The Following CEC MSG are used for Command Recognition only, whichi ain't support in  ALI STB.
************************************************************************************************************/

/************************************************************************************************************
* rx_cec_msg_record_off
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_record_off(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(0 == param_length)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_RECORD_OFF]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_RECORD_OFF], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_RECORD_OFF], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_record_on
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_record_on(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;

    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if((param_length >= CEC_MSG_PARAM_LEN_ONE) && (param_length <= CEC_MSG_PARAM_LEN_EIGHT))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Record Source Type=0x%02X][Record Source Data=0x%02X]\n", source_address,\
               cec_opcode_str[OPCODE_RECORD_ON], param[0]);

            if(param_length == CEC_MSG_PARAM_LEN_ONE)
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Record Source Type=0x%02X]\n", source_address, \
                    cec_opcode_str[OPCODE_RECORD_ON], param[0]);
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Record Source Type=0x%02X][Record Source Data=0x%02X][CDC Param=", \
                    source_address, cec_opcode_str[OPCODE_RECORD_ON], param[0]);

                for(i=1; i<param_length-1; i++)
                {
                    CEC_PRINTF("0x%02X ", param[i]);
                }
                CEC_PRINTF("]\n");
            }
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_RECORD_ON], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_RECORD_ON], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_record_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_record_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if (NULL == param)
        {
            return CEC_MSG_ERR_PARAM_VALUE;
        }
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Record Status Info=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_RECORD_STATUS], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_RECORD_STATUS], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_RECORD_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_record_tv_screen
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_record_tv_screen(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_RECORD_TV_SCREEN]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_RECORD_TV_SCREEN], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_RECORD_TV_SCREEN], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_clear_analogue_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_clear_analogue_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ELEVEN)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][Analogue Broadcast Type=0x%02X]\
               [Analogue Frequence=0x%02X %02X][Broadcast System=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_CLEAR_ANALOG_TIMER], param[0], param[1], param[2], param[3], param[4], param[5], \
               param[6], param[7], param[8], param[9], param[10]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_CLEAR_ANALOG_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_CLEAR_ANALOG_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_clear_digital_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_clear_digital_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_FOURTEEN)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][Digital Service ID=0x%02X %02X %02X %02X %02X %02X \
               %02X]\n", source_address, cec_opcode_str[OPCODE_CLEAR_DIGITAL_TIMER], param[0], param[1], param[2], \
               param[3], param[4], param[5], param[6], param[7], param[8], param[9], param[10], param[11], param[12], \
               param[13]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_CLEAR_DIGITAL_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_CLEAR_DIGITAL_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_clear_external_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_clear_external_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_NINE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][External Source Specifier=0x%02X]\
               [External Plug=0x%02X]\n", source_address, cec_opcode_str[OPCODE_CLEAR_EXTERNAL_TIMER], param[0], \
               param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8]);
        }
        else if (param_length == CEC_MSG_PARAM_LEN_TEN)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PAYLOAD_LENGTH;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][External Source Specifier=0x%02X]\
               [External Physical Address=0x%02X %02X]\n", source_address, \
               cec_opcode_str[OPCODE_CLEAR_EXTERNAL_TIMER], param[0], param[1], param[2], param[3], param[4], \
               param[5], param[6], param[7], param[8], param[9]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_CLEAR_EXTERNAL_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_CLEAR_EXTERNAL_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_analogue_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_analogue_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ELEVEN)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][Analogue Broadcast Type=0x%02X]\
               [Analogue Frequence=0x%02X %02X][Broadcast System=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_SET_ANALOG_TIMER], param[0], param[1], param[2], param[3], param[4], param[5], \
               param[6], param[7], param[8], param[9], param[10]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SET_ANALOG_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SET_ANALOG_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_digital_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_digital_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_FOURTEEN)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X]\
               [Digital Service ID=0x%02X %02X %02X %02X %02X %02X %02X]\n", source_address, \
               cec_opcode_str[OPCODE_SET_DIGITAL_TIMER], param[0], param[1], param[2], param[3], param[4], \
               param[5], param[6], param[7], param[8], param[9], param[10], param[11], param[12], param[13]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SET_DIGITAL_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SET_DIGITAL_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_external_timer
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_external_timer(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_NINE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][External Source Specifier=0x%02X]\
               [External Plug=0x%02X]\n", source_address, cec_opcode_str[OPCODE_SET_EXTERNAL_TIMER], param[0], \
               param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8]);
        }
        else if (CEC_MSG_PARAM_LEN_TEN == param_length)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Day of Month=0x%02X][Month of Year=0x%02X][Start Time=0x%02X %02X]\
               [Duration=0x%02X %02X][Recording Sequence=0x%02X][External Source Specifier=0x%02X]\
               [External Physical Address=0x%02X %02X]\n", source_address, cec_opcode_str[OPCODE_SET_EXTERNAL_TIMER], \
               param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8], param[9]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SET_EXTERNAL_TIMER], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SET_EXTERNAL_TIMER], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_timer_program_title
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_timer_program_title(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    UINT8 i = 0;

    if( dest_address == api_cec_get_device_logical_address())        // Directly addressed
    {
        if((param_length >= CEC_MSG_PARAM_LEN_ONE) && (param_length <= CEC_MSG_PARAM_LEN_FOURTEEN))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Program Title=\"\n", source_address, \
               cec_opcode_str[OPCODE_SET_TIMER_PROGRAM_TITLE]);
            for(i=0; i<param_length; i++)
            {
                CEC_PRINTF("%c", param[i]);
            }
            CEC_PRINTF("\"]\n");
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SET_TIMER_PROGRAM_TITLE], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SET_TIMER_PROGRAM_TITLE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_timer_cleared_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_timer_cleared_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Timer Cleared Status Data=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_TIMER_CLEARED_STATUS], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_TIMER_CLEARED_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_TIMER_CLEARED_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_timer_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_timer_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Timer Status Data=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_TIMER_STATUS], param[0]);
        }
        else if(param_length == CEC_MSG_PARAM_LEN_THREE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Timer Status Data=0x%02X 0x%02X 0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_TIMER_STATUS], param[0], param[1], param[2]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_TIMER_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_TIMER_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_deck_control
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_deck_control(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed            
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Deck Control_Mode=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_DECK_CONTROL], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_DECK_CONTROL], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_DECK_CONTROL], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_deck_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_deck_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Deck Info=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_DECK_STATUS], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_DECK_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message 
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_DECK_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_give_deck_status
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_give_deck_status(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Status Request=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_GIVE_DECK_STATUS], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_GIVE_DECK_STATUS], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_GIVE_DECK_STATUS], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_play
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_play(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, UINT8 *param, \
    UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Play Mode=0x%02X]\n",source_address,cec_opcode_str[OPCODE_PLAY],param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_PLAY], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_PLAY], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_set_audio_rate
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_set_audio_rate(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_ONE)
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            CEC_PRINTF("RecvFrom(0x%X): <%s> [Audio Rate=0x%02X]\n", source_address, \
               cec_opcode_str[OPCODE_SET_AUDIO_RATE], param[0]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_SET_AUDIO_RATE], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_SET_AUDIO_RATE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_initiate_arc
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_initiate_arc(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_INITIATE_ARC]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_INITIATE_ARC], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_INITIATE_ARC], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_report_arc_initiated
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_report_arc_initiated(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_REPORT_ARC_INITIATED]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REPORT_ARC_INITIATED], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_ARC_INITIATED], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_report_arc_terminated
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_report_arc_terminated(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_REPORT_ARC_TERMINATED]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REPORT_ARC_TERMINATED], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REPORT_ARC_TERMINATED], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_request_arc_initiation
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_request_arc_initiation(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_REQUEST_ARC_INITIATION]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REQUEST_ARC_INITIATION], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REQUEST_ARC_INITIATION], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_request_arc_termination
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_request_arc_termination(E_CEC_LOGIC_ADDR source_address, \
    E_CEC_LOGIC_ADDR dest_address, UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length==CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_REQUEST_ARC_TERMINATION]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_REQUEST_ARC_TERMINATION], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_REQUEST_ARC_TERMINATION], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

/************************************************************************************************************
* rx_cec_msg_terminate_arc
************************************************************************************************************/
E_CEC_MSG_ERRNO rx_cec_msg_terminate_arc(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    if( dest_address == api_cec_get_device_logical_address() )        // Directly addressed
    {
        if(param_length == CEC_MSG_PARAM_LEN_ZERO)
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s> [None]\n", source_address, cec_opcode_str[OPCODE_TERMINAE_ARC]);
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d)\n", source_address, \
               cec_opcode_str[OPCODE_TERMINAE_ARC], param_length);
            //AP_CEC_MSG_Feature_Abort(source_address,  OPCODE_ABORT, REASON_INVALID_OPRAND);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        // strange situation , ignore this message
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X)\n", source_address, \
            cec_opcode_str[OPCODE_TERMINAE_ARC], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }

    return CEC_MSG_OK;
}

E_CEC_MSG_ERRNO rx_cec_msg_cdc_message(E_CEC_LOGIC_ADDR source_address, E_CEC_LOGIC_ADDR dest_address, \
    UINT8 *param, UINT16 param_length)
{
    UINT8    i = 0;
    UINT8 cdc_param_length = 0;
    UINT8 cdc_opcode = 0;
    UINT8 *cdc_param = NULL;        
    E_CEC_MSG_ERRNO  t_result = CEC_MSG_UNKNOWN;
    
    if(dest_address!=CEC_LA_BROADCAST)
    {
        if((param_length <= CEC_MSG_PARAM_LEN_FOURTEEN) && (param_length >= CEC_MSG_PARAM_LEN_THREE))
        {
            if (NULL == param)
            {
                return CEC_MSG_ERR_PARAM_VALUE;
            }
            if(param_length == CEC_MSG_PARAM_LEN_THREE)
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Initiator Phy. Address=0x%02X %02X][CDC Opcode=0x%02X]\n", \
                    source_address, cec_opcode_str[OPCODE_CDC_MESSAGE], param[0], param[1], param[2]);
            }
            else
            {
                CEC_PRINTF("RecvFrom(0x%X): <%s> [Initiator Phy. Address=0x%02X %02X][CDC Opcode=0x%02X][CDC Param=",\
                    source_address, cec_opcode_str[OPCODE_CDC_MESSAGE], param[0], param[1], param[2]);

                for(i=3; i<param_length-3; i++)
                {
                    CEC_PRINTF("0x%02X ", param[i]);
                }
                CEC_PRINTF("]\n");
            }

            cdc_opcode        = param[2];
            cdc_param         = param+3;
            cdc_param_length    = param_length-3;
            t_result = _cec_msg_cdc_message_parser(cdc_opcode, cdc_param, cdc_param_length);
            if(CEC_MSG_OK != t_result)
            {
                ali_trace(&t_result);
            }
            // Ignore the MSG
        }
        else
        {
            CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (param_length=%d) \n", source_address, \
               cec_opcode_str[OPCODE_CDC_MESSAGE], param_length);
            return CEC_MSG_ERR_PAYLOAD_LENGTH;
        }
    }
    else
    {
        CEC_PRINTF("RecvFrom(0x%X): <%s>, ERROR! (dest_address=0x%X) \n", source_address,
          cec_opcode_str[OPCODE_CDC_MESSAGE], dest_address);
        return CEC_MSG_ERR_DST_ADDR;
    }
    return CEC_MSG_OK;
}
/************************************************************************************************************
*    <END> The Following CEC MSG are used for Command Recognition only, whichi ain't support in  ALI STB.
************************************************************************************************************/

