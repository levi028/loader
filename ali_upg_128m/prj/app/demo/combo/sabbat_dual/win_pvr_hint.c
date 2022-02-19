/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_hint.c

*    Description: recording hint menu, indicate if recording or not.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef DVR_PVR_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif


#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_pvr_hint.h"

#ifndef SD_UI
#ifdef SUPPORT_CAS_A
#define PVR_HINT_L  940//920//880//966//530
#define PVR_HINT_T 110//150//68//108//30
#else
#define PVR_HINT_L 880//966//530
#define PVR_HINT_T 150//68//108//30
#endif
#define PVR_HINT_W 34
#define PVR_HINT_H 25
#else
#define PVR_HINT_L 530//966//530
#define PVR_HINT_T 30//68//108//30
#define PVR_HINT_W 60
#define PVR_HINT_H 32

#endif
#define PVR_HINT_ICON   IM_N_INFORMATION_PVR


#define REC_HINT_STATE        1
#define UNREC_HINT_STATE        0


DEF_BITMAP(win_rec_hint_bmp,NULL,NULL,C_ATTR_ACTIVE,0, 1,0,0,0,0,PVR_HINT_L,PVR_HINT_T,PVR_HINT_W,PVR_HINT_H, \
    WSTL_TRANS_IX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX, NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,PVR_HINT_ICON)


static BOOL cur_rec_hint_state=UNREC_HINT_STATE;



/*******************************************************************************
* Function definition
*******************************************************************************/

/***********************************************/

BOOL get_rec_hint_state(void)
{
    return cur_rec_hint_state;
}

BOOL set_rec_hint_state(BOOL flag)
{
    cur_rec_hint_state = flag;
    return TRUE;
}

void show_rec_hint_osdon_off(UINT8 flag)
{
    OBJECT_HEAD *obj = NULL;

    obj = (OBJECT_HEAD*)&win_rec_hint_bmp;

#ifdef _INVW_JUICE //v0.1.4
    if (!inview_is_app_busy())
    {
        inview_pause();
        if(flag)
        {
            osd_draw_object(obj, C_UPDATE_ALL);
            osal_task_sleep(3000);
        }
        osd_clear_object(obj, 0);
    }
#else
    if(flag)
    {
        osd_draw_object(obj, C_UPDATE_ALL);
    }
    else
    {
        osd_clear_object(obj, 0);
    }
#endif
}

////////////////////////////////////////////////
#endif//#ifdef DVR_PVR_SUPPORT

