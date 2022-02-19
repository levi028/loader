/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_task.c
*
* Description:
*     Build HDMI CEC task.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

/*******************
* INCLUDE FILES    *
********************/
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
//#include <api/libcec/lib_cec.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <string.h>

#include "cec_private.h"

#define CEC_PROC_PRINTF    libc_printf
#define CEC_TASK_MBF_SIZE    60

/*******************
* STATIC DATA      *
********************/
static OSAL_ID                 cec_msgbf_id = OSAL_INVALID_ID;
static OSAL_ID                 cec_cmdbf_id = OSAL_INVALID_ID;


/************************************************************************************************************
* callback_hdmi_cec_receive_message
* register to HDMI driver to receive CEC message.
* receive message from HDMI then enqueue this message to cec_ctrl_task process.
************************************************************************************************************/
static void callback_hdmi_cec_receive_message (UINT8 *message, UINT8 message_length)
{
    CEC_RX_MSG cec_msg;

    if (NULL == message)
    {
        return;
    }

    cec_msg.length    = message_length;
    cec_msg.data        = message;

    if( osal_msgqueue_send(cec_msgbf_id,    &cec_msg,    sizeof(CEC_RX_MSG), 0) != OSAL_E_OK )
    {
        libc_printf(" osal_msgqueue_send fail\n");
    }
}


INT32 ap_cec_send_command_message(E_CEC_AP_CMD_TYPE command_type, E_CEC_LOGIC_ADDR logical_address, UINT8 *param)
{
    CEC_CMD command;

      MEMSET(&command, 0, sizeof(CEC_CMD));
    command.type        =     command_type;
    command.dest_addr    =    logical_address;

    if(api_cec_check_retrieved_valid_physical_address()==FALSE)
    {
        libc_printf("ap_cec_send_command_message(): CEC Doesn't have a valid physical address! (drop CEC_CMD)\n");
        return !SUCCESS;
    }

    switch(command_type)
    {

        case CEC_CMD_IMAGE_VIEW_ON:
            break;
        case CEC_CMD_TEXT_VIEW_ON:
            break;
        case CEC_CMD_ACTIVE_SOURCE:
          break;
        case CEC_CMD_INACTIVE_SOURCE:
            break;
        case CEC_CMD_REQUEST_ACTIVE_SOURCE:
            break;
        case CEC_CMD_SYSTEM_STANDBY:
            break;
        case CEC_CMD_CEC_VERSION:
            break;
        case CEC_CMD_GET_CEC_VERSION:
            break;
        case CEC_CMD_GIVE_PHYSICAL_ADDRESS:
            break;
        case CEC_CMD_REPORT_PHYSICAL_ADDRESS:
            break;
        case CEC_CMD_GET_MENU_LANGUAGE:
            break;
        case CEC_CMD_SET_MENU_LANGUAGE:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_MENU_LANGUAGE));
            }
            break;
        case CEC_CMD_POLLING_MESSAGE:
            break;
        case CEC_CMD_GIVE_TUNER_DEVICE_STATUS:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_STATUS_REQUEST));
            }
            break;
        case CEC_CMD_TUNER_STEP_INCREMENT:
            break;
        case CEC_CMD_TUNER_STEP_DECREMENT:
            break;
        case CEC_CMD_TUNER_DEVICE_STATUS:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_TUNER_DEVICE_INFO));
            }
            break;
        case CEC_CMD_GIVE_DEVICE_VENDOR_ID:
            break;
        case CEC_CMD_DEVICE_VENDOR_ID:
            break;
        case CEC_CMD_SET_OSD_STRING:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_OSD_STRING));
            }
            break;
        case CEC_CMD_GIVE_OSD_NAME:
            break;
        case CEC_CMD_SET_OSD_NAME:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_OSD_NAME));
            }
            break;
        case CEC_CMD_MENU_STATUS:
            if(param)
            {
                command.params[0] = param[0];
            }
            break;
        case CEC_CMD_GIVE_DEVICE_POWER_STATUS:
            break;
        case CEC_CMD_REPORT_POWER_STATUS:
            break;
        case CEC_CMD_FEATURE_ABORT:
            if(param)
            {
                MEMCPY(command.params, param, sizeof(CEC_FEATURE_ABORT));
            }
            break;
        case CEC_CMD_ABORT:
            break;
        case CEC_CMD_SYSTEM_AUDIO_MODE_REQUEST:
            if(param)
            {
                command.params[0] = param[0];
            }
            break;
        case CEC_CMD_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            break;
        case CEC_CMD_USER_CONTROL_PRESSED:
            if(param)
            {
                command.params[0] = param[0];
            }
            break;
        case CEC_CMD_USER_CONTROL_RELEASED:
            break;
        case CEC_CMD_SYSTEM_AUDIO_MODE_STATUS:
            break;
        case CEC_CMD_SET_SYSTEM_AUDIO_MODE:
            if(param)
            {
                command.params[0] = param[0];
            }
            break;
        case CEC_CMD_REPORT_AUDIO_STATUS:
            if(param)
            {
                command.params[0] = param[0];
            }
            break;
        default:
            libc_printf(" [!] CEC Command Not Supported!(command_type=%d)\n", command_type);
            return !SUCCESS;
    }

    if( osal_msgqueue_send(    cec_cmdbf_id,    &command,    sizeof(CEC_CMD), 0) != OSAL_E_OK )
    {
        libc_printf(" osal_msgqueue_send fail\n");
    }

    return SUCCESS;
}



/************************************************************************************************************
* cec_control_task
* lib_cec module control task
************************************************************************************************************/
static void cec_control_task(UINT32 param1, UINT32 param2)
{

    OSAL_T_CMBF     t_cmbf;
    CEC_RX_MSG        msg;
    CEC_CMD            cmd;
    UINT32          msgsize = 0;
    UINT32          cmdsize = 0;
    //UINT32          tx_cnt = 0;
    //UINT32          rx_cnt = 0;
    const UINT32    recv_timeout = 100;

    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);

    if(hdmi_dev == NULL)
        ASSERT(0);
    
    MEMSET(&msg, 0, sizeof(CEC_RX_MSG));
    MEMSET(&cmd, 0, sizeof(CEC_CMD));
    /* Create CEC Message Buffer */
    t_cmbf.bufsz         =     sizeof(CEC_RX_MSG) * CEC_TASK_MBF_SIZE;
    t_cmbf.maxmsz    =    sizeof(CEC_RX_MSG);

    cec_msgbf_id = osal_msgqueue_create(&t_cmbf);

    if (OSAL_INVALID_ID == cec_msgbf_id)
    {
        libc_printf("Can not create CEC message buffer!!!\n");
        ASSERT(0);
    }

    /* Create CEC Command Buffer */
    t_cmbf.bufsz =  sizeof(CEC_CMD) * CEC_TASK_MBF_SIZE;
    t_cmbf.maxmsz = sizeof(CEC_CMD);
    cec_cmdbf_id = osal_msgqueue_create(&t_cmbf);

    if (OSAL_INVALID_ID == cec_cmdbf_id)
    {
        libc_printf("Can not create CEC command buffer!!!\n");
        ASSERT(0);
    }
    /********************************************************/

    api_cec_set_device_physical_address(api_get_physical_address());
    if(api_cec_check_retrieved_valid_physical_address())
    {
        libc_printf("CEC_CTRL_TASK(1st): Physical = 0x%04X\n", api_get_physical_address());
        cec_act_logical_address_allocation();
    }

    hdmi_dev->io_control(hdmi_dev,HDMI_CMD_REG_CALLBACK, HDMI_CB_CEC_MESSAGE, (UINT32)callback_hdmi_cec_receive_message);


    //tx_cnt = 1;
    //rx_cnt = 1;
    while(1)
    {

        if(api_cec_get_func_enable_flag())
        {
            // Check receive message queue from HDMI Low level driver
            if(osal_msgqueue_receive(&msg, (INT *)&msgsize, cec_msgbf_id, recv_timeout) == E_OK)
            {

                if(api_cec_check_retrieved_valid_physical_address())
                {
                    // process the cec message
                    cec_msg_proc(&msg);
                }
                else
                {
                    // drop the cec message
                    libc_printf("cec_ctrl_task: drop current cec message!\n");
                }
            }

            //Check receive command queue from UI application
            if(osal_msgqueue_receive(&cmd, (INT *)&cmdsize, cec_cmdbf_id, recv_timeout) == E_OK)
            {
                if(api_cec_check_retrieved_valid_physical_address())
                {
                    // process the cec command
                    cec_cmd_proc(&cmd);
                }
                else
                {
                    // drop the cec command
                    libc_printf("cec_ctrl_task: drop current cec command!\n");
                }
            }
        }
        osal_task_sleep(10);
    }

}


RET_CODE cec_module_init(CEC_EVENT_CB_FUNC_T cb_func_ptr)
{
    OSAL_T_CTSK cec_task_param ;
    UINT16 i = 0;
    BOOL b_ret = FALSE;
    CEC_PRINTF("cec_module_init(cb_func_ptr=0x%08X)\n", cb_func_ptr);

    /********************************************************/
    /* CEC Config Initialized                                    */
    /********************************************************/
    api_cec_set_device_logical_address(CEC_LA_BROADCAST);
    api_cec_set_device_physical_address(INVALID_CEC_PHYSICAL_ADDRESS);
    api_cec_set_device_vendor_id(STB_VENDOR_ID);
    api_cec_set_service_id_method(SERVICE_BY_DIGITAL_ID);
    api_cec_set_channel_number_format(CHANNEL_NUM_1_PART);

    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
    api_cec_set_menu_active_state(CEC_MENU_STATE_UNINITED);

    for(i=CEC_LA_TV; i<CEC_LA_BROADCAST; i++)
    {
        api_cec_update_bus_physical_address_info(i, INVALID_CEC_PHYSICAL_ADDRESS);
    }
    api_cec_set_remote_control_passthrough_feature(FALSE);
    api_cec_set_system_audio_control_feature(TRUE);
    api_cec_set_func_enable(TRUE);
    api_cec_set_osd_popup_enable(TRUE);
    api_cec_set_device_standby_mode(CEC_LOCAL_STANDBY_MODE);
    b_ret = api_cec_register_system_callback_function(cb_func_ptr);
    if(FALSE == b_ret)
    {
        libc_printf("cec register system_cb failed\n");
        return ERR_FAILUE;
    }
    /********************************************************/

    cec_task_param.task = cec_control_task;
    cec_task_param.itskpri=OSAL_PRI_NORMAL-1;
    cec_task_param.quantum=10;
    cec_task_param.stksz=0x1000;
    cec_task_param.para1=0;
    cec_task_param.para2=0;
    cec_task_param.name[0] = 'C';
    cec_task_param.name[1] = 'E';
    cec_task_param.name[2] = 'C';

    if(OSAL_INVALID_ID == osal_task_create(&cec_task_param))
    {
        libc_printf("cre_tsk cec_task_param failed\n");
        return ERR_FAILUE;
    }

    return RET_SUCCESS;
}

