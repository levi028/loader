/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_preplay_channel.h
 *
 *    Description: This head file contains APIs use pre-play channel process.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __CTRL_PREPLAY_CHANNEL_H_
#define __CTRL_PREPLAY_CHANNEL_H_
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include <api/libsi/sie_monitor.h>
#include <hld/dmx/dmx_dev.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

BOOL    api_pre_play_channel(P_NODE *p_node, struct ft_frontend *ft, struct cc_param *param,
                             BOOL *ci_start_service, BOOL bk_play);


#ifdef __cplusplus
}
#endif
#endif
