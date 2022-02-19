/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_ttx.h

   *    Description:define the MACRO, the data,structure uisng TELETEXT
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __TTX_ENGINE_H__
#define __TTX_ENGINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <mediatypes.h>
#include "ttx_osd.h"
#include <osal/osal.h>

#define LATIN_0 0
#define LATIN_1 1
#define LATIN_2 2
#define LATIN_3 3
#define LATIN_CYRILLIC 4
#define LATIN_GREEK 6
#define LATIN_ARABIC 8
#define ARABIC_HEBREW 10

#define TTXENG_OFF        0
#define TTXENG_SUBT        1
#define TTXENG_TEXT        2

#define TTX_SUBT_LANG_NUM        11

#define TTX_NORMAL_0 0
#define TTX_NORMAL_0 0
#define TTX_NORMAL_1 1
#define TTX_NORMAL_2 2
#define TTX_NORMAL_3 3
#define TTX_NORMAL_4 4
#define TTX_NORMAL_5 5

#define TTX_TYPE_0 0
#define TTX_TYPE_1 1
#define TTX_TYPE_2 2
#define TTX_TYPE_3 3
#define TTX_TYPE_4 4
#define TTX_TYPE_5 5
#define TTX_REGISTER_NUM 3
#define TTX_PAGE_MIN_NUM 100
#define TTX_PAGE_MAX_NUM 899

#define TTX_MAX_COL_IF_DOUBLE     39
#define TTX_COLOR_NUMBER_256     256
#define TTX_COLOR_NUMBER_16     16
#define TTX_MAX_ROW             24

#define TTX_DSCRIPT_MAX_LENGTH 256

#define TTX_FLASH_FUNC
struct t_ttx_lang
{
    //UINT8 lang_idx;
    UINT16 pid;
    UINT16 page;
    UINT8 lang[3];
    UINT8 ttx_type;
};

enum ttx_event
{
    TTX_PID_UPDATE = 1,
    TTX_PID_ADD,                // Add for SAT2IP
};

typedef void(*TTX_EVENT_CALLBACK)(enum ttx_event event, UINT32 param);

struct ttx_config_par
{
    BOOL erase_unknown_packet;
    BOOL ttx_sub_page;
    BOOL parse_packet26_enable;
    BOOL user_fast_text;
    BOOL no_ttx_descriptor;
    BOOL sys_sdram_size_2m; //true:2M,false:other
    BOOL hdtv_support_enable;
    UINT8 *ttx_vscrbuf;
    UINT8 *ttx_pallette;

    BOOL ttx_cyrillic_1_support;
    BOOL ttx_cyrillic_2_support;
    BOOL ttx_cyrillic_3_support;
    BOOL ttx_greek_support;
    BOOL ttx_arabic_support;
    BOOL ttx_hebrew_support;
    BOOL ttx_cyrillic_g2_support;
    BOOL ttx_greek_g2_support;
    BOOL ttx_arabic_g2_support;
    BOOL ttx_g3_support;

    UINT16  ttx_color_number;

    UINT32  ttx_subpage_addr;

    UINT8 osd_layer_id;

    UINT32 *(*get_ttxchar_from_cyrillic_1)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_cyrillic_2)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_cyrillic_3)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_greek)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_arabic)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_hebrew)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_g2)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_cyrillic_g2)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_greek_g2)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_arabic_g2)(UINT8 charset, UINT8 character, UINT8 i);
    UINT32 *(*get_ttxchar_from_g3)(UINT8 charset, UINT8 character, UINT8 i);
    void (*ttx_drawchar)(UINT16 x, UINT16 y, UINT16 charset, UINT8 fg_color, UINT8 bg_color,
                        UINT8 double_width,UINT8 double_height, UINT8 character, UINT8 p26_char_set);
    UINT32  (*osd_get_scale_para)(enum tvsystem tvsys,INT32 scr_width);
};

extern struct vbi_device *ttx_dev;
extern OSAL_ID TTX_SEM_ID;
//extern UINT8 g_init_page_num;

void  ttxeng_init(void);
void  ttxeng_attach(struct ttx_config_par *pconfig_par);

INT32 ttxeng_open(UINT8 subtitle,UINT16 page_num);
void  ttxeng_close(void);
void  ttxeng_show_on_off(BOOL b_on);
void ttxeng_send_key(TTX_KEYTYPE key);
void ttxeng_default_g0_set(UINT8 g0_set);

UINT32 ttxeng_get_subt_lang(struct t_ttx_lang** list ,UINT8* num);
UINT8 ttxeng_get_cur_subt_lang(void);
INT32 ttxeng_set_subt_lang(UINT8 lang);
UINT32 ttxeng_get_init_lang(struct t_ttx_lang** list ,UINT8* num);
#ifdef TTX_BY_OSD
INT32 ttxeng_set_init_lang(UINT8 lang);
#endif
INT32 ttx_register(UINT32 monitor_id);
INT32 ttx_unregister(UINT32 monitor_id);

void ttx_set_initpage(BOOL enable, UINT16 page_id);
void  ttx_enable(BOOL);
void ttxpvr_set_subt_lang(struct t_ttx_lang* list ,UINT8 num);
void ttxpvr_set_init_lang(struct t_ttx_lang* list ,UINT8 num);
void ttx_pvr_enable(BOOL enable,UINT16 dmx_id);

void ttx_reg_callback(TTX_EVENT_CALLBACK callback);
void ttx_unreg_callback();
BOOL ttx_is_available();
void ttx_osd_set_size(UINT32 width,UINT32 height);
BOOL ttx_check_enable(void);
extern UINT16 get_inital_page();
extern UINT8 get_inital_page_status();
extern UINT16 get_first_ttx_page();
extern INT32 ttx_descriptor(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param);

#ifdef __cplusplus
}
#endif

#endif /*__TTX_ENGINE_H__  */

