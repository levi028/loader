/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: subt_osd.c
   *
   *    Description: The file is mainly to show DVB subtitle on OSD.
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
#include "lib_subt_internal.h"
//#include "osd_lib.h"

struct subt_config_par g_subt_config_par;
BOOL g_subt_init = FALSE;

void osd_subt_enter(void)
{
    if( FALSE== g_subt_init)
    {
        return;
    }
    return g_subt_config_par.osd_subt_enter();
}

void osd_subt_leave(void)
{
    if( FALSE== g_subt_init)
    {
        return;
    }
    return g_subt_config_par.osd_subt_leave();
}

void osd_subt_set_clut(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t)
{
    if( FALSE== g_subt_init)
    {
        return;
    }
    return g_subt_config_par.osd_subt_set_clut(entry_id, y, cb, cr, t);
}

void osd_subt_clear_page(void)
{
    if( FALSE== g_subt_init)
    {
        return;
    }
    return g_subt_config_par.osd_subt_clear_page();
}

INT32 osd_subt_create_region(UINT8 region_idx,struct osdrect *rect)
{
    if( (FALSE== g_subt_init)||(NULL==rect)||(MAX_REGION_IN_PAGE<=region_idx))
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_create_region(region_idx,rect);
}

INT32 osd_subt_delete_region(UINT8 region_idx)
{
    if( (FALSE== g_subt_init)||(MAX_REGION_IN_PAGE<=region_idx))
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_delete_region(region_idx);
}
INT32 osd_subt_region_show(UINT8 region_idx,UINT8 on)
{
    if( (FALSE== g_subt_init)||(MAX_REGION_IN_PAGE<=region_idx)||((1!=on)&&(0!=on)))
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_region_show(region_idx, on);
}

INT32 osd_subt_draw_pixel(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data)
{
    if( (FALSE== g_subt_init)||(MAX_REGION_IN_PAGE<=region_idx)||(0xffff<=x)
		||(0xffff<=y)/*||(0xff<=data)*/)
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_draw_pixel(region_idx, x, y, data);
}

BOOL osd_region_is_created(UINT8 region_idx)
{
    if( (FALSE== g_subt_init)||(MAX_REGION_IN_PAGE<=region_idx))
    {
        return 0;
    }
    return g_subt_config_par.osd_region_is_created(region_idx);
}

//void osd_subt_draw_pixelmap(struct OSDRect rect, UINT8* data)
//{
//    if(g_subt_init == FALSE)
//        lib_subt_init();
//    return g_subt_config_par.osd_subt_draw_pixelmap(rect, data);
//}

INT32 osd_subt_get_region_addr(UINT8 region_idx,UINT16 y, UINT32 *addr)
{
    if( (FALSE== g_subt_init)||(MAX_REGION_IN_PAGE<=region_idx)||(0xffff<=y)
		||(NULL==addr))
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_get_region_addr(region_idx, y, addr);
}

void osd_subt_update_clut(void)
{
    if( FALSE== g_subt_init)
    {
        return;
    }
    return g_subt_config_par.osd_subt_update_clut();
}

INT32 osd_subt_display_define(struct sdec_display_config *cfg)
{
    if( (FALSE== g_subt_init)||(NULL==cfg))
    {
        return 0;
    }
    return g_subt_config_par.osd_subt_display_define(cfg);
}

INT32 osd_subt_clear_osd_screen(BOOL flag)
{
    if(g_subt_init == FALSE)
        return 0;
    return g_subt_config_par.osd_subt_clear_osd_screen(flag);
}
