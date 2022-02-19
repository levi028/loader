/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_bat.h
*
*    Description: parse BAT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SI_BAT_H__
#define __SI_BAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>
#include "si_module.h"
#include <osal/osal.h>

#define MAX_BOUQ_NAME_LENGTH    15
#define BAT_MIN_TIME            1500

struct bat_service_info
{
    UINT16 ts_id;
    UINT16 orig_netid;
    UINT16 bouq_id;
    UINT16 sid;
    UINT16 lcn;
    UINT8 service_type;
    UINT8 vol;
    UINT8 track;
};

struct bat_hitmap
{
    UINT32 all_hit_value;
    UINT32 hit;
    UINT32 parse;
};

struct bat_section_info
{
    //tmp save each section bouquet id
    UINT16 b_id;
    //tmp save one tp's ts_id, origin_netid
    UINT16 ts_id;
    UINT16 orig_netid;

    UINT8 buf[8*PSI_SHORT_SECTION_LENGTH];

    INT16 bouq_cnt;
    struct bat_hitmap hitmap[PSI_MODULE_MAX_BAT_EXT_NUMBER];
    UINT16 bouq_id[PSI_MODULE_MAX_BAT_EXT_NUMBER];
    UINT8 bouq_name[PSI_MODULE_MAX_BAT_EXT_NUMBER][2*(MAX_BOUQ_NAME_LENGTH + 1)];

    INT16 service_cnt;
    struct bat_service_info s_info[PSI_MODULE_MAX_BAT_SINFO_NUMBER];
};

extern ID g_bat_semaphore;

INT32 si_bat_request_start(void);
INT32 si_bat_parser(UINT8 *bat, INT32 len, struct section_parameter *param);
INT32 si_bat_get_info(struct bat_section_info *bsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg);

#ifdef __cplusplus
}
#endif

#endif /* __SI_BAT_H__ */

