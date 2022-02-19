/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_search.h
*
*    Description: The function of search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_SEARCH_H_
#define _WIN_SEARCH_H_

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_frontend.h>
#include <hld/decv/decv.h>
#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>
#include <bus/tsi/tsi.h>
#include <hld/nim/nim_dev.h>
#include <api/libsi/si_tdt.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#include "key.h"
#include "power.h"

#define DBG_PRINTF PRINTF//soc_printf
#if 0
#define BC_API_PRINTF        libc_printf
#else
#define BC_API_PRINTF(...)    do{}while(0)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

extern TEXT_FIELD   srch_tv_back;
extern TEXT_FIELD   srch_radio_back;
extern TEXT_FIELD   srch_tp_back;
extern TEXT_FIELD   srch_line_h;
extern TEXT_FIELD   srch_line_v;
extern TEXT_FIELD   srch_info;
extern PROGRESS_BAR srch_progress_bar;
extern TEXT_FIELD   srch_progress_txt;
extern BITMAP       srch_bmp;
extern CONTAINER    srch_info_ex;
extern TEXT_FIELD   srch_info_txt_ex;
extern TEXT_FIELD   srch_info_line;
extern BOOL is_auto_scan;
extern void win_search_set_pid_param(UINT32 tp,\
            UINT32 vpid,UINT32 apid,UINT32 ppid);
extern void win_search_set_autoscan_param(void);
extern void win_search_set_param(struct as_service_param *param);
extern void set_auto_scan_update_mode(BOOL mode) ;
#ifdef SHOW_WELCOME_FIRST
extern void get_search_param(struct as_service_param *asp);
extern void get_search_prog_cnt(UINT32 *ptv_num,UINT32 *prad_num);
extern void save_search_prog_cnt(UINT32 tv_num,UINT32 rad_num);
#endif

#ifdef SELECT_HIGHER_QUALITY_SERVICE
void filter_same_service(UINT16 sat_id,UINT32 stp_id);
void set_current_tp(UINT32 ctp_id);
#endif

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif
