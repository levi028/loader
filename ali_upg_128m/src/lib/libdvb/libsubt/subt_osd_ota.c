/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: subt_osd_ota.c
   *
   *    Description: The file is mainly for OTA module.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>
#include <api/libsubt/lib_subt.h>
#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/dis/vpo.h>
//#include "osd_lib.h"

void osd_subt_set_clut(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t)
{
}

void osd_subt_clear_page(void)
{
}

INT32 osd_subt_create_region(UINT8 region_idx,struct osdrect *rect)
{
    return 0;
}

INT32 osd_subt_delete_region(UINT8 region_idx)
{
    return 0;
}
INT32 osd_subt_region_show(UINT8 region_idx,UINT8 on)
{
    return 0;
}

INT32 osd_subt_draw_pixel(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data)
{
    return 0;
}

BOOL osd_region_is_created(UINT8 region_idx)
{
    return TRUE;
}

//void osd_subt_draw_pixelmap(struct OSDRect rect, UINT8* data)
//{
//    if(g_subt_init == FALSE)
//        lib_subt_init();
//    return g_subt_config_par.osd_subt_draw_pixelmap(rect, data);
//}

INT32 osd_subt_get_region_addr(UINT8 region_idx,UINT16 y, UINT32 *addr)
{
    return 0;
}

void osd_subt_update_clut(void)
{
}

INT32 osd_subt_display_define(struct sdec_display_config *cfg)
{
    return 0;
}

