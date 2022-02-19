/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2009 Copyright (C)
*
*    File:    ci_api.c
*
*    Description:    This file contains all IO control interfaces for AP.
*
*    History:
*            Date          Athor      Version    Comments
*   =====   ========      =======      =========     =================
*      1.     2009.10.21    Steven     Ver 0.1    Create file.
*
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include "ci_stack.h"
#ifdef CI_SUPPORT

extern struct ci_slot_control ci_control;
extern void ci_set_nim_ts_type(struct nim_device* nim_dev, struct dmx_device* dmx_dev,
                    UINT8 type, UINT8 slot);
extern void ci_set_pmt_cmd(UINT32 prog_id, BOOL resend, BOOL update, int slot);


//CI IO control api
RET_CODE api_ci_io_ctrl(int slot, enum ci_io_ctrl_cmd cmd, UINT32 param)
{
    ASSERT(slot >= 0 && slot < CI_MAX_SLOT_NUM);

    APPL_PRINTF("api_ci_io_ctrl: slot[%d], cmd[%d], param[%X]\n",slot,cmd,param);
    switch(cmd)
    {
        case CI_IO_CTRL_PASS_CAM:    //set TS pass CAM
            ci_set_nim_ts_type(ci_control.slot_info[slot].nim_dev,
                            ci_control.slot_info[slot].dmx_dev,
                            3, slot);
            break;
        case CI_IO_CTRL_BYPASS_CAM:    //set TS bypass CAM
            ci_set_nim_ts_type(ci_control.slot_info[slot].nim_dev,
                            ci_control.slot_info[slot].dmx_dev,
                            2, slot);
            break;
        case CI_IO_CTRL_RESEND_CA_PMT:    //resend ca pmt to CAM if CAM works abnormal
            if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0)
            {
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, TRUE, FALSE, slot);
                api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
            }
            else
            {
                APPL_PRINTF("api_ci_io_ctrl: cmd[%d] error - no pmt!\n",cmd);
            }
            break;
        case CI_IO_CTRL_SWITCH_AID:    //switch audio pid
            if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0)
            {
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX] = param;
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, TRUE, FALSE, slot);
                api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
            }
            else
            {
                APPL_PRINTF("api_ci_io_ctrl: cmd[%d] error - no pmt!\n",cmd);
            }
            break;
        case CI_IO_CTRL_SWITCH_TTX_PID:    //switch ttx pid
            if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0)
            {
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_TTX_INDEX] = param;
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, TRUE, FALSE, slot);
                api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
            }
            else
            {
                APPL_PRINTF("api_ci_io_ctrl: cmd[%d] error - no pmt!\n",cmd);
            }
            break;
        case CI_IO_CTRL_SWITCH_SUBT_PID:    //switch subt pid
            if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0)
            {
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_SUBT_INDEX] = param;
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, TRUE, FALSE, slot);
                api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
            }
            else
            {
                APPL_PRINTF("api_ci_io_ctrl: cmd[%d] error - no pmt!\n",cmd);
            }
            break;
        default:
            APPL_PRINTF("api_ci_io_ctrl: not supported cmd[%d]!\n",cmd);
            return ERR_FAILURE;
            break;
    }

    return SUCCESS;
}
#else    //CI_SUPPORT
RET_CODE api_ci_io_ctrl(int slot, enum ci_io_ctrl_cmd cmd, UINT32 param)
{
    return SUCCESS;
}
#endif

