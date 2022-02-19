/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_ota.c
   *
   *    Description: The file is mainly to make the OTA module.
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

#ifdef _BC_CA_NEW_    
UINT32 subt_get_language(struct t_subt_lang **list ,UINT8 *num)
{
    return 0;
}
#endif

void lib_subt_attach(struct subt_config_par *psubt_config_par)
{

}
#ifndef _BC_CA_NEW_    
void osd_subt_enter(void)
{

}

void osd_subt_leave(void)
{

}
#endif

void lib_subt_atsc_set_buf(struct atsc_subt_config_par *p_config)
{

}

UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
    return 0;
}

BOOL lib_subt_atsc_create_task(void)
{
    return TRUE;
}

BOOL lib_subt_atsc_terminate_task(void)
{
    return TRUE;
}

BOOL lib_subt_atsc_section_parse_terminate_task(void)
{
	return TRUE;
}


void lib_subt_atsc_clean_up(void)
{

}

void lib_subt_atsc_delete_timer(void)
{

}

void lib_subt_atsc_show_onoff(BOOL onoff)
{

}

void lib_subt_atsc_hcenter_on_off(BOOL b_on_off)
{

}

void lib_subt_atsc_force_shift_y_set(BOOL b_user_force,INT32 n_shift_y)
{

}
void lib_subt_atsc_color_exchange_on_off(BOOL b_on_off)
{

}

void lib_subt_atsc_color_exchange__color_char_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{

}

void lib_subt_atsc_color_exchange__color_outline_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{

}

void lib_subt_dvb_auto_top_pos_onoff(BOOL b_on_off,UINT16 us_top_max,UINT16 us_top_save)
{

}

void lib_subt_dvb_shift_y_set(INT32 n_shift_y)
{

}

void lib_subt_shift_time_set(INT32 n_shiftt,BOOL b_on_off)//xuehui
{

}

void lib_subt_dvb_bg_color_set(BOOL b_background,UINT32 n_color)//xuehui#1
{

}
void lib_subt_dvb_char_color_set(BOOL b_char,UINT32 n_color)
{

}
void lib_subt_copy_data(UINT32 data, UINT32 length)
{

}

