/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ttx_init.c
*
*    Description: The file is mainly to init teletext engine.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libtsi/si_types.h>
//#include <api/libtsi/si_table.h>

#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include "lib_ttx_internal.h"


//extern struct ttx_config_par g_ttx_pconfig_par;
//extern UINT8 transparent_level;
//extern struct dmx_device *dmx_dev;
//extern UINT8 ttx_eng_state;
//extern UINT8 transparent_value[TTX_TRANSPARENT_LEVEL];
//extern UINT8 *subpage_p26_nation;


__ATTRIBUTE_REUSE_
void  ttxeng_init(void)
{
     ttx_eng_para_init();

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT)
    g_ttx_pconfig_par.osd_layer_id = 0;
#else
    g_ttx_pconfig_par.osd_layer_id = 1;
#endif

#ifdef    SUPPORT_ERASE_UNKOWN_PACKET
    g_ttx_pconfig_par.erase_unknown_packet = TRUE;
#else
    g_ttx_pconfig_par.erase_unknown_packet = FALSE;
#endif

#ifdef TTX_SUB_PAGE
    g_ttx_pconfig_par.ttx_sub_page = TRUE;
    g_ttx_pconfig_par.ttx_subpage_addr = __MM_TTX_SUB_PAGE_BUF_ADDR;
#else
    g_ttx_pconfig_par.ttx_sub_page = FALSE;
    g_ttx_pconfig_par.ttx_subpage_addr = 0;
#endif

#ifdef SUPPORT_PACKET_26
    g_ttx_pconfig_par.parse_packet26_enable = TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_g2 = get_char_from_g2;

    //if(TRUE == g_ttx_pconfig_par.ttx_sub_page)
    //{
	#ifdef TTX_SUB_PAGE
        if(RET_FAILURE==ttx_alloc_subpage_p26_naton_buffer())
        {
            return;//ASSERT(0);
        }
	#endif
    //}
#else
    g_ttx_pconfig_par.parse_packet26_enable = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_g2 = NULL;
#endif

#ifdef USER_FAST_TEXT
    g_ttx_pconfig_par.user_fast_text = TRUE;
#else
    g_ttx_pconfig_par.user_fast_text = FALSE;
#endif

#ifdef NO_TTX_DESCRIPTOR
    g_ttx_pconfig_par.no_ttx_descriptor = TRUE;
#else
    g_ttx_pconfig_par.no_ttx_descriptor = FALSE;
#endif

#if (SYS_SDRAM_SIZE == 2)
    g_ttx_pconfig_par.sys_sdram_size_2m = TRUE;
    g_ttx_pconfig_par.ttx_vscrbuf =
    (UINT8 *)(((UINT32)(__MM_TTX_TMP_BUF_ADDR) & 0x1fffffff) | 0x80000000);
#else
    g_ttx_pconfig_par.sys_sdram_size_2m = FALSE;
    g_ttx_pconfig_par.ttx_vscrbuf = ttx_vscr_buf_8m;
#endif

#ifdef HDTV_SUPPORT
    g_ttx_pconfig_par.hdtv_support_enable = TRUE;
#else
    g_ttx_pconfig_par.hdtv_support_enable = FALSE;
#endif

#ifdef TTX_CYRILLIC_1_SUPPORT
    g_ttx_pconfig_par.ttx_cyrillic_1_support = TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_1 = get_char_from_cyrillic_1;
#else
    g_ttx_pconfig_par.ttx_cyrillic_1_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_1 = NULL;
#endif

#ifdef TTX_CYRILLIC_2_SUPPORT
    g_ttx_pconfig_par.ttx_cyrillic_2_support = TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_2 = get_char_from_cyrillic_2;
#else
    g_ttx_pconfig_par.ttx_cyrillic_2_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_2 = NULL;
#endif

#ifdef TTX_CYRILLIC_3_SUPPORT
    g_ttx_pconfig_par.ttx_cyrillic_3_support = TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_3 = get_char_from_cyrillic_3;
#else
    g_ttx_pconfig_par.ttx_cyrillic_3_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_3 = NULL;
#endif

#ifdef TTX_GREEK_SUPPORT
    g_ttx_pconfig_par.ttx_greek_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_greek = get_char_from_greek;
#else
    g_ttx_pconfig_par.ttx_greek_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_greek = NULL;
#endif

#ifdef TTX_ARABIC_SUPPORT
    g_ttx_pconfig_par.ttx_arabic_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_arabic = get_char_from_arabic;
#else
    g_ttx_pconfig_par.ttx_arabic_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_arabic = NULL;
#endif

#ifdef TTX_HEBREW_SUPPORT
    g_ttx_pconfig_par.ttx_hebrew_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_hebrew = get_char_from_hebrew;
#else
    g_ttx_pconfig_par.ttx_hebrew_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_hebrew = NULL;
#endif

#if defined(SUPPORT_PACKET_26) && defined(TTX_CYRILLIC_G2_SUPPORT)
    g_ttx_pconfig_par.ttx_cyrillic_g2_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_g2 = get_char_from_g2_cyrillic;
#else
    g_ttx_pconfig_par.ttx_cyrillic_g2_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_cyrillic_g2 = NULL;
#endif

#if defined(SUPPORT_PACKET_26) && defined(TTX_GREEK_G2_SUPPORT)
    g_ttx_pconfig_par.ttx_greek_g2_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_greek_g2 = get_char_from_g2_greek;
#else
    g_ttx_pconfig_par.ttx_greek_g2_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_greek_g2 = NULL;
#endif

#if defined(SUPPORT_PACKET_26) && defined(TTX_ARABIC_G2_SUPPORT)
    g_ttx_pconfig_par.ttx_arabic_g2_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_arabic_g2 = get_char_from_g2_arabic;
#else
    g_ttx_pconfig_par.ttx_arabic_g2_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_arabic_g2 = NULL;
#endif

#if defined(SUPPORT_PACKET_26) && defined(TTX_G3_SUPPORT)
    g_ttx_pconfig_par.ttx_g3_support= TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_g3 = get_char_from_g3;
#else
    g_ttx_pconfig_par.ttx_g3_support = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_g3 = NULL;
#endif

#if (TTX_COLOR_NUMBER==16)
    g_ttx_pconfig_par.ttx_color_number = 16;
    g_ttx_pconfig_par.ttx_pallette = (UINT8*)ttx_pallette_16;
    g_ttx_pconfig_par.ttx_drawchar = ttx_draw_char_16;
#elif (TTX_COLOR_NUMBER==256)
    g_ttx_pconfig_par.ttx_color_number = 256;
    g_ttx_pconfig_par.ttx_pallette = (UINT8*)ttx_pallette_256;
    g_ttx_pconfig_par.ttx_drawchar = ttx_draw_char_256;
#endif

}

