/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_ctrl_bar_draw.h

*    Description: The drawing flows of the BMP\TEXT\ICON in pvr ctrl bar
                  will be defined here.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_PVR_CTRL_BAR_DRAW_H_
#define _WIN_PVR_CTRL_BAR_DRAW_H_

#include "win_pvr_ctrl_bar_basic.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RECORD_BAR_NUM    (sizeof(record_bars)/sizeof(record_bar_t))
#define PLAY_BAR_WIDTH (record_bars[0].width)
#include "win_pvr_ctrl_bar_basic.h"
void win_pvr_get_play_2ndtime(char *string, int str_size);
void win_pvr_get_play_1sttime(char *string, int str_size);
UINT32 win_pvr_get_play_pos(void);
void win_pvr_ctlbar_draw_infor(void);
void win_pvr_ctlbar_draw(void);
void api_set_pvr_status_icons(void);


#ifdef __cplusplus
}
#endif

#endif //_WIN_PVR_CTRL_BAR_DRAW_H_
