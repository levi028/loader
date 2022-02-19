/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_init_ext.c
   *
   *    Description: The file is mainly to init the parameter of DVB subtitle.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
//#include <api/libtsi/si_table.h>

#include <api/libsubt/lib_subt.h>
#include <api/libsubt/subt_osd.h>
#include <hld/sdec/sdec.h>
#include <hld/osd/osddrv.h>
#include"lib_subt_internal.h"

void lib_subt_attach(struct subt_config_par *psubt_config_par)
{
    BOOL bl_addr_range_legal = TRUE;

    if(NULL == psubt_config_par)
    {

        return;
    }
    else
    {
        /* Addr legal check */
		#if (defined(_M3715C_) || defined(_M3823C_)) 
    	psubt_config_par->g_ps_buf_addr = (UINT8*)__MM_SUB_PB_START_ADDR;
    	psubt_config_par->g_ps_buf_len =  __MM_SUB_PB_LEN;	
   		#endif
        bl_addr_range_legal  = osal_check_address_range_legal(psubt_config_par->g_ps_buf_addr,
                                                              psubt_config_par->g_ps_buf_len);
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return;
        }
        MEMCPY(&g_subt_config_par, psubt_config_par, sizeof(struct subt_config_par));
    }
	if(g_subt_config_par.cc_by_osd)
		g_subt_config_par.cc_by_osd=0;
	if(g_subt_config_par.cc_by_vbi)
		g_subt_config_par.cc_by_vbi=0;
	if(g_subt_config_par.max_subt_height!=576)
		g_subt_config_par.max_subt_height=576;
	if((g_subt_config_par.osd_layer_id!=0)&&(g_subt_config_par.osd_layer_id!=1))
		g_subt_config_par.osd_layer_id=1;
	if(g_subt_config_par.speed_up_subt_enable!=FALSE)
		g_subt_config_par.speed_up_subt_enable=FALSE;
	if(g_subt_config_par.sys_sdram_size>=0xff)
		g_subt_config_par.sys_sdram_size=0;
	if(g_subt_config_par.user_force_pos==TRUE)
		g_subt_config_par.user_force_pos=FALSE;
	if(!g_subt_config_par.user_force_pos)
	{
		g_subt_config_par.user_force_set_hor_offset=0;
		g_subt_config_par.user_force_set_ver_offset=0;
	}
    if(g_subt_config_par.hd_subtitle_support)
    {
        g_subt_config_par.subt_width = 1840;
        g_subt_config_par.subt_height = 1024;
        g_subt_config_par.subt_hor_offset = (1920-g_subt_config_par.subt_width)/2;
        g_subt_config_par.subt_ver_offset =(1080-g_subt_config_par.subt_height)/2;
    }
    else
    {
        g_subt_config_par.subt_width = 720;//SUBT_WIDTH;
        g_subt_config_par.subt_height = 576;//SUBT_HEIGHT;
        g_subt_config_par.subt_hor_offset = 0;//SUBT_HOR_OFFSET;
        g_subt_config_par.subt_ver_offset = 0;//SUBT_VER_OFFSET;
    }
	 if(TRUE==g_subt_config_par.osd_blocklink_enable )
		g_subt_config_par.osd_blocklink_enable =FALSE;
#if 0
    if(0)//TRUE==g_subt_config_par.osd_blocklink_enable )
    {
        g_subt_config_par.osd_subt_enter = osd_subt_enter_2m;
        g_subt_config_par.osd_subt_leave = osd_subt_leave_2m;
        g_subt_config_par.osd_subt_set_clut = osd_subt_set_clut_2m;
        g_subt_config_par.osd_subt_clear_page = osd_subt_clear_page_2m;
        g_subt_config_par.osd_subt_create_region = osd_subt_create_region_2m;
        g_subt_config_par.osd_subt_delete_region = osd_subt_delete_region_2m;
        g_subt_config_par.osd_subt_draw_pixel = osd_subt_draw_pixel_2m;
        g_subt_config_par.osd_region_is_created = osd_region_is_created_2m;
//      g_subt_config_par.osd_subt_draw_pixelmap = osd_subt_draw_pixelmap_2m;
        g_subt_config_par.osd_subt_region_show = osd_subt_region_show_2m;
        g_subt_config_par.osd_subt_get_region_addr = osd_subt_get_region_addr_2m;
        //g_subt_config_par.osd_subt_update_clut = osd_subt_update_clut_2m;
        g_subt_config_par.osd_subt_display_define = osd_subt_display_define_2m;

    }
    else
#endif
    {
        g_subt_config_par.osd_subt_enter = osd_subt_enter_no_bl;
        g_subt_config_par.osd_subt_leave = osd_subt_leave_no_bl;
        g_subt_config_par.osd_subt_set_clut = osd_subt_set_clut_no_bl;
        g_subt_config_par.osd_subt_clear_page = osd_subt_clear_page_no_bl;
        g_subt_config_par.osd_subt_create_region = osd_subt_create_region_no_bl;
        g_subt_config_par.osd_subt_delete_region = osd_subt_delete_region_no_bl;
        g_subt_config_par.osd_subt_draw_pixel = osd_subt_draw_pixel_no_bl;
        g_subt_config_par.osd_region_is_created = osd_region_is_created_no_bl;
		g_subt_config_par.osd_subt_draw_pixelmap = NULL;
        g_subt_config_par.osd_subt_region_show = osd_subt_region_show_no_bl;
        g_subt_config_par.osd_subt_get_region_addr = osd_subt_get_region_addr_no_bl;
        g_subt_config_par.osd_subt_update_clut = osd_subt_update_clut_no_bl;
        g_subt_config_par.osd_subt_display_define = osd_subt_display_define_no_bl;
		g_subt_config_par.osd_subt_clear_osd_screen = osd_subt_clear_osd_screen_no_bl;

    }

    g_subt_init = TRUE;
    LIBSUBT_PRINTF("%s : block_link[%d], OSD[%d], HD[%d]!\n",__FUNCTION__,
    g_subt_config_par.osd_blocklink_enable,g_subt_config_par.osd_layer_id,
    g_subt_config_par.hd_subtitle_support);
}


