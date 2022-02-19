/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ttx_init_ext.c
*
*    Description: The file is mainly to init the teletext engine and set the
     parameter of the OSD for teletext.
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
#include <api/libttx/ttx_osd.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include "lib_ttx_internal.h"


//extern UINT8 transparent_level;
//extern struct dmx_device *dmx_dev;
//extern UINT8 ttx_eng_state;
//extern UINT8 transparent_value[TTX_TRANSPARENT_LEVEL];

//extern const UINT8 ttx_pallette_16[4*16];
//extern const UINT8 ttx_pallette_256[4*256];
//extern UINT8 *subpage_p26_nation;
//extern UINT8 TTX_VscrBuf_8M[ ( ((TTX_CHAR_W*2)>>TTX_FACTOR) ) * TTX_CHAR_H*2];

UINT32 osd_ttx_width = REAL_OSD_TTX_WIDTH ;
//the width of the background of the TTX OSD
UINT32 osd_ttx_height = REAL_OSD_TTX_HEIGHT ;
//the height of the background of the TTX OSD
UINT32 osd_ttx_startcol = ((720 - REAL_OSD_TTX_WIDTH)/2) ;
//the uLeft of the TTX_OSD background according to the screen
UINT32 osd_ttx_startrow =((570 - REAL_OSD_TTX_HEIGHT)/2) ;
//the uTop of the TTX_OSD background according to the screen
UINT32 osd_ttx_xoffset =((REAL_OSD_TTX_WIDTH - (TTX_CHAR_W*40))/2) ;
//the uLeft of the region which show the real data of the ttx
//according to the TTX_OSD background
#ifdef TTX_SUB_PAGE
UINT32 osd_ttx_yoffset =((REAL_OSD_TTX_HEIGHT - (TTX_CHAR_H*26))/2) ;
//the uTop of  the region which show the real data of the ttx
//according to the TTX_OSD background
#else
UINT32 osd_ttx_yoffset =((REAL_OSD_TTX_HEIGHT - (TTX_CHAR_H*25))/2);
#endif
void ttx_osd_set_size(UINT32 width,UINT32 height);
static void ttx_osd_set_parameter(UINT32 width,UINT32 height);
extern ID ttx_flash_mutex;


__ATTRIBUTE_REUSE_
void  ttxeng_attach(struct ttx_config_par *pconfig_par)
{
#ifdef TTX_BY_OSD

    BOOL bl_addr_range_legal = TRUE;

    ttx_eng_para_init();
    MEMSET(&g_ttx_pconfig_par,0,sizeof(struct ttx_config_par));

    if(NULL == pconfig_par)
    {
        return;//ASSERT(0);
    }
    else
    {
		#if (defined(_M3715C_) || defined(_M3823C_)) 
		if(pconfig_par->ttx_sub_page )
		{
		    pconfig_par->ttx_subpage_addr = __MM_TTX_SUB_PAGE_BUF_ADDR;
		}
	    else
		{
		    pconfig_par->ttx_subpage_addr = 0;
	    }
		#endif
        /* Addr legal check */
        bl_addr_range_legal  = osal_check_address_range_legal((void *)(pconfig_par->ttx_vscrbuf), 0);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)(pconfig_par->ttx_subpage_addr), 0);
        if (FALSE == bl_addr_range_legal)
        {
            //libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return;
        }

        if(1!=pconfig_par->osd_layer_id)
        {
            return;
        }

        MEMCPY(&g_ttx_pconfig_par, pconfig_par, sizeof(struct ttx_config_par));
    }

//reserved following macros to save code size
#ifdef SUPPORT_PACKET_26
    g_ttx_pconfig_par.parse_packet26_enable = TRUE;
    g_ttx_pconfig_par.get_ttxchar_from_g2 = get_char_from_g2;

    if(TRUE== g_ttx_pconfig_par.ttx_sub_page )
    {
        subpage_p26_nation = (UINT8 *)MALLOC(25*40*40*sizeof(UINT8));
        if(NULL == subpage_p26_nation)
        {
             return;//ASSERT(0);
        }
        MEMSET(subpage_p26_nation, 0, (25*40*40)*sizeof(UINT8));
    }
#else
    g_ttx_pconfig_par.parse_packet26_enable = FALSE;
    g_ttx_pconfig_par.get_ttxchar_from_g2 = NULL;
#endif

#if (SYS_SDRAM_SIZE == 2)
    g_ttx_pconfig_par.ttx_vscrbuf =
    (UINT8 *)(((UINT32)(__MM_TTX_TMP_BUF_ADDR) & 0x1fffffff) | 0x80000000);
#else
    g_ttx_pconfig_par.ttx_vscrbuf = ttx_vscr_buf_8m;
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
#if 1
 //libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
   if( 0 == ttx_flash_mutex )
   {
       if((ttx_flash_mutex = osal_mutex_create()) == OSAL_INVALID_ID)
       {
           ASSERT(0);
       }
   }
 #endif
#endif

}


static void ttx_osd_set_parameter(UINT32 width,UINT32 height)
{
    if((width>REAL_OSD_TTX_WIDTH)||(height>REAL_OSD_TTX_HEIGHT))
    {
        return;
    }
    osd_ttx_startcol = (720- width)/2;
    osd_ttx_startrow = (570 - height)/2;
    osd_ttx_xoffset = (width - (TTX_CHAR_W*40))/2;

    #ifdef TTX_SUB_PAGE
        osd_ttx_yoffset = (height - (TTX_CHAR_H*26))/2;
    #else
        osd_ttx_yoffset = (height - (TTX_CHAR_H*25))/2;
    #endif
}

void ttx_osd_set_size(UINT32 width,UINT32 height)
{
    UINT8 b_valid_width=0;
    UINT8 b_valid_height=0;
    UINT32 width1=0;
    UINT32 height1=0;

    b_valid_width = ((0==width%2)?1:0);
    b_valid_height = ((0==height%2)?1:0);
    if(1==b_valid_width)
    {
        width1 = width;
    }
    else
    {
        width1 = width+1;
    }

    if(1==b_valid_height)
    {
        height1 = height;
    }
    else
    {
        height1 = height+1;
    }

    if((width1 >= REAL_OSD_TTX_WIDTH) && (width1 <= TTX_MAX_OSD_WIDTH))
    {
        osd_ttx_width =width1 ;
    }
    else if(width1 < REAL_OSD_TTX_WIDTH)
    {
        osd_ttx_width = REAL_OSD_TTX_WIDTH ;
    }
    else
    {
        osd_ttx_width = 720 ;
    }

    if((height1 >= REAL_OSD_TTX_HEIGHT) && (height1 <=TTX_MAX_OSD_HEIGHT))
    {
        osd_ttx_height = height1 ;
    }
    else if(height1 < REAL_OSD_TTX_HEIGHT)
    {
        osd_ttx_height = REAL_OSD_TTX_HEIGHT;
    }
    else
    {
        osd_ttx_height = 570 ;
    }
    ttx_osd_set_parameter(osd_ttx_width,osd_ttx_height);
}

