/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_process_table.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SIE_MONITOR_PROCESS_TABLE__
#define __SIE_MONITOR_PROCESS_TABLE__

#ifdef __cplusplus
extern "C" {
#endif

extern struct PID_TID_PARSER pid_tid_parser[];  //save mappings between pid, table_id and receive function

INT16 get_dmx_index(struct dmx_device *dmx);
void callback_run(section_parse_cb_t *callback_tab, INT max_cnt,UINT8 *buf, INT32 length, UINT32 param);
INT32 register_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback);
INT32 unregister_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback);
void set_table_bit_valid(struct dmx_device *dmx, UINT16 table_id, UINT16 dmx_index, UINT16 pid, UINT32 param);
void set_table_bit_invalid(enum MONITE_TB table, UINT16 dmx_index, UINT16 pid);
void set_monitor_param(UINT16 table_id, UINT32 param, struct si_filter_param *fparam);
UINT32 monitor_id_valid(UINT32 monitor_id);
UINT32 check_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 pid,UINT32 param);

#ifdef __cplusplus
}
#endif

#endif
