/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_play_si_proc.h
 *
 *    Description: This head file contains play channel's SI process APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __CTRL_PLAY_SI_PROC_H_
#define __CTRL_PLAY_SI_PROC_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include <api/libsi/sie_monitor.h>
#include "./copper_common/com_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

INT32   api_sim_callback_for_playback(UINT32 param);;
INT32   api_sim_callback(UINT32 param);
INT32   api_sim_pat_callback(UINT32 param);
UINT32  api_sim_start(struct dmx_device *dmx, enum MONITE_TB sim_type, UINT32 tp_id, UINT32 prog_id,
                      UINT32 prog_number, UINT16 sim_pid, INT8 b_force, sim_notify_callback sim_callback);
void    api_sim_stop(struct ts_route_info *p_ts_route);

#ifdef __cplusplus
}
#endif
#endif
