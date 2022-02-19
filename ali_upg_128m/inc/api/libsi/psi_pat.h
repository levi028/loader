/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: psi_pat.h
*
*    Description: PAT table parser
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __PSI_PAT_H__
#define __PSI_PAT_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"

struct pat_section_info
{
    INT16 max_map_nr;
    INT16 map_counter;
    UINT16 ts_id;
    UINT16 nit_pid;
    struct extension_info ext[1];
    struct program_map *map;
    UINT32 crcvalue;//auto update tp information
};

#ifdef __cplusplus
extern "C" {
#endif

INT32 psi_pat_parser(UINT8 *pat, INT32 len, struct section_parameter *param);

#ifdef __cplusplus
}
#endif

#endif /* __PSI_PAT_H__ */
