/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_prog_name.h
*
*    Description:   The realize of program name
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PROGNAME_H_
#define _WIN_PROGNAME_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
//#include <api/libtsi/si_epg.h>
#include <api/libsi/lib_epg.h>
#include <hld/deca/deca.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#include "ctrl_util.h"
#include "ctrl_play_channel.h"

#include "copper_common/system_data.h"
#include "ap_ctrl_display.h"

#if (_ALI_PLAY_VERSION_ >= 2)
#include "aliplay_service.h"
#endif

#include <api/libosd/osd_lib.h>
#include <api/libclosecaption/lib_closecaption.h>

#include "win_mute.h"
#include "win_pause.h"
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#endif

extern struct channel_info recent_channels[2];
extern OSAL_ID epg_flag;
extern UINT8 show_and_playchannel;
extern BITMAP       prog_bmp;
extern TEXT_FIELD   prog_text;

void change_channel(INT32 shift);
PRESULT win_progname_unkown_act_proc(VACTION act);
void progname_timer_func(UINT unused);
void win_progname_redraw(void);

typedef enum
{
    PROG_BMP_STATIC,
    PROG_BMP_AV_TYPE,
    PROG_BMP_EPG,
    PROG_BMP_TTX,
    PROG_BMP_SUBT,
    PROG_BMP_LOCK,
    PROG_BMP_FAV,
    PROG_BMP_AUDIO_MODE,
    PROG_BMP_CA
} prog_bmp_type;

typedef enum
{
    PROG_TXT_PROG_NAME,
    PROG_TXT_DATE,
    PROG_TXT_TIME,
    PROG_TXT_PROG_NUM,
    PROG_TXT_EPG_NOW,
    PROG_TXT_EPG_NEXT,
#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    PROG_TXT_PROG_GROUP,
#endif
    PROG_TXT_DUAL_MONO,
    PROG_TXT_SUBT_TYPE
} prog_txt_type;

typedef struct
{
    prog_bmp_type bmp_type;
    UINT16 left, top, width, height;
    UINT16 icon;
} prog_bmp_t;

typedef struct
{
    prog_txt_type txt_type;
    UINT16 left, top, width, height;
    UINT8 shidx;
} prog_txt_t;

UINT16 get_cur_icon(UINT16 cur_channel, P_NODE *p_node,prog_bmp_t *p_prog_bmp, UINT8 av_mode);

void win_progname_get_proper_string(UINT16 cur_channel, char *string, int str_size, prog_txt_t *p_prog_txt,
                                    S_NODE *s_node, UINT16 *group_name);

BOOL win_prog_is_epg_update(void);

void win_prog_set_epg_update(BOOL bupdate);

extern char txt_string_time[80];

#define STRING_LENGTH  15

#ifdef __cplusplus
}
#endif

#endif//_WIN_PROGNAME_H_
