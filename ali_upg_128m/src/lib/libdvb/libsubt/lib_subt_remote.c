/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_remote.c
   *
   *    Description: The file is mainly to define the remote call function for
        DVB subtitle.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <osal/osal.h>
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
#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#endif

#include "lib_subt_internal.h"

//#define LIBSUBT_PRINTF     PRINTF

#ifdef DUAL_ENABLE
#include <modules.h>

enum LIB_SUBT_FUNC
{
    FUNC_LIB_SUBT_ATTACH = 0,
    FUNC_OSD_SUBT_ENTER,
    FUNC_OSD_SUBT_LEAVE,
// add for DCII Subt (also called SCTE Subt) , North American Cable market
    FUNC_LIB_SUBT_ATSC_SET_BUF,
    FUNC_LIB_ATSC_STREAM_IDENTIFY,
    FUNC_LIB_SUBT_ATSC_CREATE_TASK,
    FUNC_LIB_SUBT_ATSC_TERMINATE_TASK,
    FUNC_LIB_SUBT_ATSC_CLEAN_UP,
    FUNC_LIB_SUBT_ATSC_DELETE_TIMER,
    FUNC_LIB_SUBT_ATSC_SHOW_ONOFF,
    FUNC_LIB_SUBT_ATSC_HCENTER_ON_OFF,
    //FUNC_LIB_SUBT_ATSC_SHIFT_Y_SET,
    FUNC_LIB_SUBT_ATSC_FORCE_SHIFT_Y_SET,
    FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE_ON_OFF,
    FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE__COLOR_CHAR_SET,
    FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE__COLOR_OUTLINE_SET,
    FUNC_LIB_SUBT_DVB_AUTO_TOP_POS_ONOFF,
    FUNC_LIB_SUBT_DVB_SHIFT_Y_SET,
    FUNC_LIB_SUBT_SHIFT_TIME_SET,
    FUNC_LIB_SUBT_DVB_BG_COLOR_SET,
    FUNC_LIB_SUBT_DVB_CHAR_COLOR_SET,
    FUNC_LIB_SUBT_COPY_DATA,
    FUNC_LIB_SUBT_ATSC_SECTION_PARSE_CREATE_TASK,
	FUNC_LIB_SUBT_ATSC_SECTION_PARSE_TERMINATE_TASK,
};
#ifndef _LIB_SUBT_REMOTE

void lib_subt_attach(struct subt_config_par *psubt_config_par);
void osd_subt_enter();
void osd_subt_leave();

// add for DCII Subt (also called SCTE Subt) , North American Cable market
#if 1
void lib_subt_atsc_set_buf(struct atsc_subt_config_par* p_config);
#else
void lib_subt_atsc_set_buf(void* p_config);
#endif
UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data);
BOOL lib_subt_atsc_create_task();
//BOOL lib_subt_atsc_terminate_task();
void lib_subt_atsc_clean_up();
void lib_subt_atsc_delete_timer();
void lib_subt_atsc_show_onoff(BOOL onoff);
void lib_subt_atsc_hcenter_on_off(BOOL b_on_off);
//void lib_subt_atsc_shift_y_set(INT32 nShiftY);
void lib_subt_atsc_force_shift_y_set(BOOL b_user_force,INT32 n_shift_y);
void lib_subt_atsc_color_exchange_on_off(BOOL b_on_off);
void lib_subt_atsc_color_exchange__color_char_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k);
void lib_subt_atsc_color_exchange__color_outline_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k);
void lib_subt_dvb_auto_top_pos_onoff(BOOL b_on_off,UINT16 us_top_max,UINT16 us_top_save);
void lib_subt_dvb_shift_y_set(INT32 n_shift_y);
void lib_subt_shift_time_set(INT32 n_shift_t,BOOL b_on_off);
void lib_subt_dvb_bg_color_set(BOOL b_background,UINT32 b_color);
void lib_subt_dvb_char_color_set(BOOL b_char,UINT32 b_color);
void lib_subt_copy_data(UINT32 data, UINT32 length);
//BOOL lib_subt_atsc_section_parse_create_task(void);
BOOL lib_subt_atsc_section_parse_terminate_task(void);

static UINT32 lib_subt_entry[] =
{
    (UINT32)lib_subt_attach,
    (UINT32)osd_subt_enter,
    (UINT32)osd_subt_leave,
    (UINT32)lib_subt_atsc_set_buf,
    (UINT32)lib_subt_atsc_stream_identify,
    (UINT32)lib_subt_atsc_create_task,
    (UINT32)lib_subt_atsc_terminate_task,
    (UINT32)lib_subt_atsc_clean_up,
    (UINT32)lib_subt_atsc_delete_timer,
    (UINT32)lib_subt_atsc_show_onoff,
    (UINT32)lib_subt_atsc_hcenter_on_off,
    //(UINT32)lib_subt_atsc_shift_y_set,
    (UINT32)lib_subt_atsc_force_shift_y_set,
    (UINT32)lib_subt_atsc_color_exchange_on_off,
    (UINT32)lib_subt_atsc_color_exchange__color_char_set,
    (UINT32)lib_subt_atsc_color_exchange__color_outline_set,
    (UINT32)lib_subt_dvb_auto_top_pos_onoff,
    (UINT32)lib_subt_dvb_shift_y_set,
    (UINT32)lib_subt_shift_time_set,
    (UINT32)lib_subt_dvb_bg_color_set,
    (UINT32)lib_subt_dvb_char_color_set,
    (UINT32)lib_subt_copy_data,
    (UINT32)lib_subt_atsc_section_parse_create_task,
	(UINT32)lib_subt_atsc_section_parse_terminate_task,
};

void lib_subt_callee(UINT8 *msg)
{
	if(NULL==msg)
	{
		return;
	}
    os_hld_callee((UINT32)lib_subt_entry, msg);
}

#endif

#ifdef _LIB_SUBT_REMOTE
static UINT32 desc_lib_subt_attach[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct subt_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};
static UINT32 desc_lib_subt_atsc_set_buf[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct atsc_subt_config_par)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

void lib_subt_attach(struct subt_config_par *psubt_config_par)
{
    jump_to_func(NULL, os_hld_caller, psubt_config_par, (LIB_SUBT_MODULE<<24)|          \
        (1<<16)|FUNC_LIB_SUBT_ATTACH, desc_lib_subt_attach);
}

void osd_subt_enter(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_OSD_SUBT_ENTER, NULL);
}

void osd_subt_leave(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_OSD_SUBT_LEAVE, NULL);
}

#if 1

#if 1
void lib_subt_atsc_set_buf(struct atsc_subt_config_par *p_config)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|      \
        (1<<16)|FUNC_LIB_SUBT_ATSC_SET_BUF, desc_lib_subt_atsc_set_buf);
}
#else
void lib_subt_atsc_set_buf(void* p_config)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|
        (1<<16)|FUNC_LIB_SUBT_ATSC_SET_BUF, NULL);
}
#endif
UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_ATSC_STREAM_IDENTIFY, NULL);
}

BOOL lib_subt_atsc_create_task(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_LIB_SUBT_ATSC_CREATE_TASK, NULL);
}

BOOL lib_subt_atsc_terminate_task(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_LIB_SUBT_ATSC_TERMINATE_TASK, NULL);
}

void lib_subt_atsc_clean_up(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_LIB_SUBT_ATSC_CLEAN_UP, NULL);
}

void lib_subt_atsc_delete_timer(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (0<<16)|FUNC_LIB_SUBT_ATSC_DELETE_TIMER, NULL);
}

void lib_subt_atsc_show_onoff(BOOL onoff)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (1<<16)|FUNC_LIB_SUBT_ATSC_SHOW_ONOFF, NULL);
}

void lib_subt_atsc_hcenter_on_off(BOOL b_on_off)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (1<<16)|FUNC_LIB_SUBT_ATSC_HCENTER_ON_OFF, NULL);
}
/*
void lib_subt_atsc_shift_y_set(INT32 nShiftY)
{
    jump_to_func(NULL, OS_hld_caller, null, (LIB_SUBT_MODULE<<24)|
    (1<<16)|FUNC_LIB_SUBT_ATSC_SHIFT_Y_SET, NULL);
}
*/
void lib_subt_atsc_force_shift_y_set(BOOL b_user_force,INT32 n_shift_y)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_SUBT_ATSC_FORCE_SHIFT_Y_SET, NULL);
}
void lib_subt_atsc_color_exchange_on_off(BOOL b_on_off)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|      \
        (1<<16)|FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE_ON_OFF, NULL);
}

void lib_subt_atsc_color_exchange__color_char_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|      \
        (4<<16)|FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE__COLOR_CHAR_SET, NULL);
}

void lib_subt_atsc_color_exchange__color_outline_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|      \
        (4<<16)|FUNC_LIB_SUBT_ATSC_COLOR_EXCHANGE__COLOR_OUTLINE_SET, NULL);
}

void lib_subt_dvb_auto_top_pos_onoff(BOOL b_on_off,UINT16 us_top_max,UINT16 us_top_save)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (3<<16)|FUNC_LIB_SUBT_DVB_AUTO_TOP_POS_ONOFF, NULL);
}
BOOL lib_subt_atsc_section_parse_create_task(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_SECTION_PARSE_CREATE_TASK, NULL);
}

BOOL lib_subt_atsc_section_parse_terminate_task(void)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_SECTION_PARSE_TERMINATE_TASK, NULL);
}

void lib_subt_dvb_shift_y_set(INT32 n_shift_y)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (1<<16)|FUNC_LIB_SUBT_DVB_SHIFT_Y_SET, NULL);
}

void lib_subt_shift_time_set(INT32 n_shift_t,BOOL b_on_off)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_SUBT_SHIFT_TIME_SET, NULL);
}
void lib_subt_dvb_bg_color_set(BOOL b_background,UINT32 b_color)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_SUBT_DVB_BG_COLOR_SET, NULL);
}
void lib_subt_dvb_char_color_set(BOOL b_char,UINT32 b_color)
{
    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_SUBT_DVB_CHAR_COLOR_SET, NULL);
}

void lib_subt_copy_data(UINT32 data, UINT32 length)
{
	if(0xffffffff<=data||0xffffffff<=length)
	{
		return;
	}
    osal_cache_flush((UINT8*)data, length);

    jump_to_func(NULL, os_hld_caller, null, (LIB_SUBT_MODULE<<24)| (2<<16)|FUNC_LIB_SUBT_COPY_DATA, NULL);
}
#endif

#endif

#endif

