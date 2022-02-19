/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_ge_draw_ctrl.c
*
*    Description: control GE draw.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>

#define  OSD_DRAW_TASK  45
static INT32    record[OSD_DRAW_TASK] = {0};
static BOOL     b_g_is_vscr = TRUE;
static BOOL     vscr_show_enable = TRUE;

void osd_ge_draw_init(UINT32 handle, BOOL b_vscr)
{
    UINT32  task_id     = 0;
    UINT8   *reg_buf    = NULL;

    task_id = osal_task_get_current_id();

    osal_task_dispatch_off();
    b_g_is_vscr = b_vscr;
    osal_task_dispatch_on();

    if (!b_vscr)
    {
        if (osd_get_task_vscr_buffer(task_id) != NULL)
        {
            osd_task_buffer_free(task_id, osd_get_task_vscr_buffer(task_id));
        }
        osddrv_get_region_addr(handle, 0, 0, (UINT32 *)(&reg_buf));
        osd_task_buffer_init(task_id, reg_buf);
    }
}

BOOL osd_get_vscr_state(void)
{
    return b_g_is_vscr;
}

BOOL osd_get_vscr_show_enable(void)
{
    return vscr_show_enable;
}

void osd_set_vscr_show_enable(BOOL state)
{
    vscr_show_enable = state;
}

void osd_draw_recode_ge_cmd_start(void)
{
    INT32 id = (INT32)osal_task_get_current_id();

    if ((INVALID_ID == id) || (id >= OSD_DRAW_TASK))
    {
        ASSERT(0);
        return;
    }

#ifdef GE_DRAW_OSD_LIB
    if (0 == record[id])
    {
        if (!osd_get_vscr_state())
        {

            lib_ge_draw_start();

        }
    }
#endif
    osal_task_dispatch_off();
    record[id]++;
    osal_task_dispatch_on();

}

void osd_draw_recode_ge_cmd_stop(void)
{
    INT32 id = (INT32)osal_task_get_current_id();

    if ((INVALID_ID == id) || (id >= OSD_DRAW_TASK))
    {
        ASSERT(0);
        return;
    }

    osal_task_dispatch_off();
    record[id]--;
    osal_task_dispatch_on();

    if (record[id] <= 0)
    {
#ifdef GE_DRAW_OSD_LIB
        if (!osd_get_vscr_state())
        {

            lib_ge_draw_end();

        }
#endif
        record[id] = 0;
    }

}


