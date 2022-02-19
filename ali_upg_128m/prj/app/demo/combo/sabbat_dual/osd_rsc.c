/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_rsc.c
*
*    Description: This file contains some functions to get osd resource by analyzing raw data.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include "sys_config.h"
#include "api/libc/printf.h"
#include "api/libc/string.h"
#include "api/libc/alloc.h"
#include "osal/osal.h"

#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <asm/chip.h>
#include "osd_config.h"
#include "osd_rsc.h"

#include "images.id"
#ifndef _BUILD_OTA_E_
#define MULTI_MENU_LANG 1
//#define FONT_1BIT_RSC_SUPPORT
#endif
//#define SCALE_FONT_SUPPORT    // only use first set font.
#ifdef OSD_16BIT_SUPPORT
    #ifndef SD_UI
    #include "rsc/bmp_16/bitmaps_array.h"
    
    #ifdef FONT_1BIT_RSC_SUPPORT
    #include "rsc/font/fonts_array_1bit.h"
    #endif    
    #if 0//def FONT_1BIT_SUPPORT    
    #define font_asc     font_asc_1bit
    #define font_lat1    font_lat1_1bit
    #define font_late     font_late_1bit
    #define font_grk    font_grk_1bit
    #define font_cry    font_cry_1bit
    #define font_arb    font_arb_1bit
    #else
    #include "rsc/font_4bit/fonts_array.h"	
	#include "rsc/font/fonts_array_1bit.h"
    #endif
    #else
    #include "rsc/bmp_sd_16/bitmaps_array.h"
    #include "rsc/font_sd/fonts_array.h"
    #include "rsc/font_sd/fonts_array_1bit.h"
    #endif
#else
#if 0//def SD_UI
#include "rsc/bmp_sd_8/bitmaps_array.h"
#include "rsc/font_sd/fonts_array.h"
#include "rsc/font_sd/fonts_array_1bit.h"
//#include "rsc/font/fonts_array_1bit.h"

//#define font_asc     font_asc_1bit
//#define font_lat1    font_lat1_1bit
//#define font_late     font_late_1bit
//#define font_grk    font_grk_1bit
//#define font_cry    font_cry_1bit
//#define font_arb    font_arb_1bit

#else
#include "rsc/bmp/bitmaps_array.h"
#include "rsc/font_4bit/fonts_array.h"
#include "rsc/font/fonts_array_1bit.h"
/*
#define font_asc     font_asc_1bit
#define font_lat1    font_lat1_1bit
#define font_late     font_late_1bit
#define font_grk    font_grk_1bit
#define font_cry    font_cry_1bit
#define font_arb    font_arb_1bit
*/
#endif

#endif

#include "rsc/str/strings_array.h"
#include "rsc/pal/palettes_array.h"
#include "rsc/wstyle/winstyles_array.h"

#include <api/libchar/lib_char.h>

#define RSC_PRINTF(...)

#ifdef OSD_16BIT_SUPPORT
#define FONT_BMP_LEN  1280*720*2//1024*100
#else
#define FONT_BMP_LEN  1280*720*4//1024*100
#endif
#define BITS_PER_PIXEL_4 4

#define SUPPORT_MULTI_TASK

static UINT8 bitmap_buffer[FONT_BMP_LEN]={0};

struct font_size_scale
{
    UINT32 w_mul;
    UINT32 w_div;
    UINT32 h_mul;
    UINT32 h_div;
};

#ifdef SCALE_FONT_SUPPORT
static struct font_size_scale font_size_ary[FONT_MAX_SIZE] =
{
    {1,1,  1,1},
    {1,1,  1,1},
    {3,2,  3,2},
    {1,1,  1,1},
};
#endif

static ID_RSC    g_lang_env = 0;
static UINT8    cur_wstyle = 0;
static const char *cur_string = NULL;
#ifdef SUPPORT_MULTI_TASK
static UINT8 *font_bitmap[SYS_OS_TASK_NUM] = {NULL};
#else
static UINT8 font_bitmap[FONT_BMP_LEN] = {0};
#endif

#ifdef BIDIRECTIONAL_OSD_STYLE
static BOOL mirror_flag = FALSE;
#endif
static UINT8 font_bitmap_thai[1024*12]={0};
static UINT32 thai_font_buf[1024*12] ={0};
/*******************************************************************/
/*        Local function defintion                                 */
/*******************************************************************/
#ifdef SCALE_FONT_SUPPORT
static UINT8 get_font_size_index(UINT16 w_lib_class)
{
    unsigned char fontsize = 0;

    fontsize = w_lib_class&0xFF;
    if(fontsize > FONT_MAX_SIZE )
    {
        fontsize = FONT_MAX_SIZE-1;
    }
    return fontsize;
}

static void osd_set_one_pix(UINT8 *buf, UINT32 pos, UINT8 n, UINT32 data)
{
    UINT8 clu = 0;
    UINT32 byte_pos = 0;
    const UINT8 four_bits_per_pix = 4;

    static UINT8 mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    if(NULL == buf)
    {
        return ;
    }
    if(four_bits_per_pix == n)
    {
        clu = *(buf + (pos >> 1));
        if (0 == (pos & 1))
        {
            clu = (clu&0x0f)|((data&0x0f)<<4);
        }
        else
        {
            clu = (clu&0xf0)|((data&0x0f));
        }
        *(buf + (pos >> 1)) = clu;
    }
    else if(1 == n)
    {
        byte_pos = pos >> 3;
        clu = buf[byte_pos];
        clu |= mask[pos & 7];
        buf[byte_pos] = clu;
    }
}

static UINT32 osd_get_one_pix(const UINT8 *buf, UINT32 pos, UINT8 n)
{
    UINT8 clu = 0;
    UINT32 ret = 0;
    static UINT8 mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    if(NULL == buf)
    {
        return 0;
    }
    switch(n)
    {
    case 1:
        clu = *(buf + (pos >> 3));
        ret = (clu & mask[pos & 7]);
        break;
    case 4;
        clu = *(buf + (pos >> 1));
        if (0 == (pos & 1))
        {
            clu >>= 4;
        }
        ret = clu & 0x0f;
        break;
    case 8:
        ret = buf[pos];
        break;
    case 16:
        ret = *(UINT16* )(buf+(pos<<1));
        break;
    case 32:
        ret = *(UINT32* )(buf+(pos<<2));
        break;
    default :
        break;
    }
    return ret;
}


static UINT32 xy2pos(UINT32 x, UINT32 y, UINT32 width, UINT32 bpp)
{
    UINT32 pos = 0;
    UINT32 dw_per_line = 0;
    UINT32 byte_per_line = 0;
    UINT32 pos_per_line = 0;
    UINT32 stride = 0;

    if( 1 == bpp)
    {
        stride = (width+7)/8;
        pos_per_line = stride*8;
    }
    else if( BITS_PER_PIXEL_4 == bpp)
    {
        #if 1
        dw_per_line = (width*bpp+31)/32;
        byte_per_line = dw_per_line*4;
        pos_per_line = byte_per_line*2;
        #else
        stride = (width*4+7)/8;
        pos_per_line = stride*2;
        #endif
    }


    pos = x*pos_per_line + y;
    return pos;
}

static BOOL osd_scale_char_matrix(UINT16 w_lib_class, UINT32 width, UINT32 height, const font_t *font, \
                                OBJECTINFO* p_object_info, UINT8 *font_buffer, UINT8 *bmp_buffer, UINT8 bpp)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 pos = 0;
    UINT32 one_bit = 0;
    UINT32 x0 = 0;
    UINT32 y0 = 0;
    UINT32 out_pos = 0;
    UINT32 out_width = 0;
    UINT32 out_height = 0;
    UINT32 cur_width = 0;
    UINT32 cur_height = 0;
    UINT32 inc_w = 0;
    UINT32 inc_h = 0;
    UINT32 fontsize = get_font_size_index(w_lib_class);

    if((NULL == font) || (NULL == p_object_info) || (NULL == font_buffer) || (NULL == bmp_buffer))
    {
        return FALSE;
    }
    if( font_size_ary[fontsize].w_mul == font_size_ary[fontsize].w_div &&
        font_size_ary[fontsize].h_mul == font_size_ary[fontsize].h_div )
    {
        return FALSE;
    }
    out_width = width * font_size_ary[fontsize].w_mul/font_size_ary[fontsize].w_div;
    out_height = height * font_size_ary[fontsize].h_mul/font_size_ary[fontsize].h_div;


    inc_w = (width<<16) / out_width;
    inc_h = (height<<16) / out_height;

    UINT32 buffer_len = (out_width * out_height) + 7;
    if(buffer_len>FONT_BMP_LEN/2)
    {
        ASSERT(0);
    }
    MEMSET(bmp_buffer, 0, buffer_len);
    cur_height = 0;
    for(i = 0; i < out_height; i++)
    {
        cur_width = 0;
        for (j = 0; j < out_width; j++)
        {
            x0 = (cur_height>>16);
            y0 = (cur_width>>16);
            pos = xy2pos(x0, y0, width, bpp);
            out_pos = xy2pos(i, j, out_width, bpp);
            UINT32 one_bit = osd_get_one_pix(font_buffer, pos, bpp);
            if (one_bit)
            {
                osd_set_one_pix(bmp_buffer, out_pos, bpp, one_bit);
            }
            cur_width += inc_w;
            //libc_printf("i=%d:j=%d:pos=%d,out_pos=%d,one_bit=%d\n", i, j, pos,out_pos,one_bit);
        }
        cur_height += inc_h;
    }
    p_object_info->m_obj_attr.m_w_height = out_height;
    p_object_info->m_obj_attr.m_w_width= out_width;
    p_object_info->m_obj_attr.m_w_actual_width *= font_size_ary[fontsize].w_mul/font_size_ary[fontsize].w_div;
    return TRUE;
}
#endif

// Font search
static BOOL osdsearch(UINT16 u_start ,UINT16 u_end, UINT16 u_aim, UINT16 *p_tab, UINT16 *u_idx )
{
    UINT16        start = u_start;
    UINT16        end = u_end;
    UINT16        mid_value = 0;

    if((NULL == p_tab) || (NULL == u_idx))
    {
        return FALSE;
    }
    while(start <= end)
    {
        mid_value = p_tab[(start + end + 1)/2];

        if((start == end) && ( u_aim != mid_value))
        {
            return FALSE;
        }
        if(u_aim > mid_value)
        {
            start = (end + start+1)/2;
        }
        else if(u_aim < mid_value)
        {
            end = (end + start)/2;
        }
        else
        {
            *u_idx = (start + end + 1)/2;
            return TRUE;
        }
    }

    return FALSE;
}



static BOOL osdsearch_font(font_t *font, UINT16 *p_idx)
{
    UINT16        u_idx = 0;

    if((NULL == font) || (NULL == p_idx))
    {
        return FALSE;
    }
    u_idx = *p_idx;

    if(u_idx <  font->u_idx_tab[0])
    {
        return FALSE;
    }
    return osdsearch(0, (font->u_font_cnt  - 1), u_idx, (UINT16*)font->u_idx_tab, p_idx);
}

static const font_t *get_font3(UINT16 w_lib_class,UINT16 *w_index)
{    
    font_t *font = NULL;
#if 1//def FONT_1BIT_RSC_SUPPORT
    UINT16 lib_type = 0;
    UINT16 __MAYBE_UNUSED__ font_idx = 0;
    unsigned char fontsize = 0;    

    fontsize = w_lib_class & 0x00FF;
    lib_type = w_lib_class & 0xFF00;

//    if(fontsize!=0)        PRINTF("!!!!!!!!!!!!!fontsize=%d\n",fontsize);
    if(fontsize >= FONT_MAX_SIZE )
    {
        fontsize = FONT_MAX_SIZE-1;
    }
#ifdef SCALE_FONT_SUPPORT
    fontsize = 0;
#endif

    switch(lib_type)
    {
    case LIB_FONT_DEFAULT:
        font = &font_asc_1bit[fontsize];
        break;
#if(MULTI_MENU_LANG)
    case LIB_FONT_LATIN1:
        font = &font_lat1_1bit[fontsize];
        break;
    case LIB_FONT_LATINE:
        font = &font_late_1bit[fontsize];
        break;
#if 1
    case LIB_FONT_GREEK:
        font = &font_grk_1bit[fontsize];
        break;
    case LIB_FONT_CYRILLIC:
        font = &font_cry_1bit[fontsize];
        break;
#endif
   
    case LIB_FONT_ARABIC:
        font = &font_arb_1bit[fontsize];
           break;
#if 0
    case LIB_FONT_MASSCHAR1:
        font = &font_msc[fontsize];
        break;
#endif
#endif
    default:
            return NULL;
    }

    if(NULL == w_index)
    {
        return NULL ;
    }
    if(NULL == font->u_idx_tab)
    {
        if(*w_index > font->u_font_cnt )
        {
            return NULL;
        }
    }
    else
    {
        font_idx = *w_index;

        switch(lib_type)
        {
        case LIB_FONT_DEFAULT:
        case LIB_FONT_LATIN1:
        case LIB_FONT_LATINE:
        case LIB_FONT_ARABIC:
            break;
        default:
            break;
        }

        if(FALSE == osdsearch_font(font,w_index))
        {
             return NULL;
        }
    }
#endif
    return font;
}

static const font_t *get_font_1bit(UINT16 w_lib_class,UINT16 *w_index)
{
    UINT16 wc = 0;

    if(NULL == w_index)
    {
        return NULL;
    }
    wc = *w_index;
    const font_t *pfont = get_font3(w_lib_class, &wc);

    if (NULL == pfont)
    {
        wc = '-';
        pfont = get_font3(w_lib_class, &wc);
    }
    *w_index = wc;

    return pfont;
}


static const font_t *get_font2(UINT16 w_lib_class,UINT16 *w_index)
{
    unsigned char fontsize = 0;
    font_t *font = NULL;
    UINT16 lib_type = 0;
    UINT16 __MAYBE_UNUSED__ font_idx = 0;

    fontsize = w_lib_class & 0x00FF;
    lib_type = w_lib_class & 0xFF00;

//    if(fontsize!=0)        PRINTF("!!!!!!!!!!!!!fontsize=%d\n",fontsize);
    if(fontsize >= FONT_MAX_SIZE )
    {
        fontsize = FONT_MAX_SIZE-1;
    }
#ifdef SCALE_FONT_SUPPORT
    fontsize = 0;
#endif

    switch(lib_type)
    {
    case LIB_FONT_DEFAULT:
        font = &font_asc[fontsize];
        break;
#if(MULTI_MENU_LANG)
    case LIB_FONT_LATIN1:
        font = &font_lat1[fontsize];
        break;
    case LIB_FONT_LATINE:
        font = &font_late[fontsize];
        break;
#if 1
    case LIB_FONT_GREEK:
        font = &font_grk[fontsize];
        break;
    case LIB_FONT_CYRILLIC:
        font = &font_cry[fontsize];
        break;
#endif

    case LIB_FONT_ARABIC:
        font = &font_arb[fontsize];
        break;

#ifdef OSD_16BIT_SUPPORT
#if((!defined FONT_1BIT_SUPPORT) && (!defined SD_UI))
    case LIB_FONT_THAI:
        font = &font_thai[fontsize];
        break;
#endif
#endif

#if 0
    case LIB_FONT_MASSCHAR1:
        font = &font_msc[fontsize];
        break;
#endif

#ifdef CAS9_V6 //char_selected
case LIB_FONT_LETTERLIKE:
    font = &font_letterlike[fontsize];
    break;

case LIB_FONT_LATINEXTB:
    font = &font_latinextb[fontsize];
    break;

case LIB_FONT_LATINEXTA:
    font = &font_latinexta[fontsize];
    break;
#endif

#endif
    default:
            return NULL;
    }
    if(NULL == w_index)
    {
        return NULL;
    }
    if(NULL == font->u_idx_tab)
    {
        if(*w_index > font->u_font_cnt )
        {
            return NULL;
        }
    }
    else
    {
        font_idx = *w_index;

        switch(lib_type)
        {
        case LIB_FONT_DEFAULT:
        case LIB_FONT_LATIN1:
        case LIB_FONT_LATINE:
        case LIB_FONT_ARABIC:
            break;
        default:
            break;
        }

        if(FALSE == osdsearch_font(font,w_index))
        {
             return NULL;
        }
    }

    return font;

}

static const font_t *get_font(UINT16 w_lib_class,UINT16 *w_index)
{
    if(NULL == w_index)
    {
        return NULL;
    }

    UINT16 wc = *w_index;
    const font_t *pfont = get_font2(w_lib_class, &wc);

    if (NULL == pfont)
    {
        wc = '-';
        pfont = get_font2(w_lib_class, &wc);
    }
    *w_index = wc;

    return pfont;
}

// BMP decoder
static unsigned long bmp_rle_decode(const unsigned char *rle_bmp_data,unsigned long data_size, unsigned char *bmp_data)
{
    unsigned long i = 0;
    unsigned long out_size = 0;
    unsigned char k = 0;
    unsigned char size = 0;
    const UINT8 max_num_same_symbol = 0x80;

    if((NULL == rle_bmp_data) || (NULL == bmp_data))
    {
        return 0;
    }
    do
    {
        size = *(rle_bmp_data + i);
        if(size<max_num_same_symbol)
        {
            for(k=0;k<size;k++)
            {
                bmp_data[out_size + k] = rle_bmp_data[i + 1];
            }
            i+= 2;
        }
        else
        {
            size -= max_num_same_symbol;
            for(k=0;k<size;k++)
            {
                bmp_data[out_size + k] = rle_bmp_data[i + 1 + k];
            }
            i+= size + 1;
        }

        out_size += size;
    }while(i!=data_size);

    return out_size;
}

static UINT32 bmp_rle16_decode(unsigned char *rle16_bmp_data,unsigned long data_size, unsigned char *bmp_data)
{
    unsigned long i = 0;
    unsigned long out_size = 0;
    unsigned char k = 0;
    unsigned char size = 0;
    const UINT8 max_num_same_symbol = 0x80;

    if((NULL == rle16_bmp_data) || (NULL == bmp_data))
    {
        return 0;
    }
    do
    {
        size = *(rle16_bmp_data + i);
        if(size<max_num_same_symbol)
        {
            for(k=0;k<size;k++)
            {
                bmp_data[out_size + k*2] = rle16_bmp_data[i + 1];
                bmp_data[out_size + k*2 + 1] = rle16_bmp_data[i + 2];
            }
            i+= 3;
        }
        else
        {
            size -= max_num_same_symbol;
            for(k=0;k<size;k++)
            {
                bmp_data[out_size + k*2] = rle16_bmp_data[i + 1 + k*2];
                bmp_data[out_size + k*2 + 1] = rle16_bmp_data[i + 2 + k*2];
            }
            i+= size*2 + 1;
        }

        out_size += size*2;
    }while(i!=data_size);

    return out_size;
}

static long bmp_rle32_decode(BYTE *in_buf,unsigned long buflen,BYTE  *out_buf,BYTE byte_of_pixel)
{
    unsigned char *p_buf = NULL;
    unsigned char *p_out_buf = NULL;
    long n_data_len = 0;
    long n_pixel_data_len = 0;
    long ret = 0;
    int  i = 0;
    const UINT8 threshold_byte_per_pixel = 3;

    if((NULL == in_buf) || (NULL == out_buf))
    {
        return -1;
    }
    p_buf = in_buf;
    p_out_buf = out_buf;
    n_data_len = (long)buflen;
    if(byte_of_pixel < threshold_byte_per_pixel)
    {
        return (-1);
    }
    while(n_data_len > 0)
    {
        if(*p_buf & 0x80) //
        {
            //n_pixel_data_len = (long)(*p_buf & ~0x80) * byte_of_pixel;
            n_pixel_data_len = (long)(*p_buf & ~0x80) << 2;
            p_buf++;
            for(i = 0;i < n_pixel_data_len; i++)
            {
                *p_out_buf = *p_buf;
                p_out_buf++;
                p_buf++;
            }
            n_data_len -= n_pixel_data_len + 1;


        }
        else
        {
            //n_pixel_data_len = (long)(*p_buf & ~0x80) * byte_of_pixel;
            n_pixel_data_len = (long)(*p_buf & ~0x80) << 2;
            p_buf++;
            for(i = 0;i < n_pixel_data_len; i++)
            {
                //*p_out_buf++ = *(p_buf + i%byte_of_pixel);
                *p_out_buf = *(p_buf + (i & 0x03));
                p_out_buf++;
            }
            p_buf += byte_of_pixel;
            n_data_len -= byte_of_pixel + 1;

        }
        ret += n_pixel_data_len;
    }
    return ret;
}
//
static ID_RSC osdext_get_font_lib_id(UINT16 env_id)
{
    lp_lang_table    p_lang_tab =(lp_lang_table) g_lang_rsc_map_table;
    UINT8            i = 0;

    for (i=0; i<SIZE_LANG_TABLE; i++)
    {
        if (p_lang_tab->lang_id == env_id)
        {
            return p_lang_tab->font_id;
        }
        else
        {
            p_lang_tab++;
        }
    }
    return LIB_FONT_DEFAULT;
}
/*******************************************************************/
/*        Local function defintion                                 */
/*******************************************************************/

UINT16 osd_get_lang_environment(void)
{
    return g_lang_env;
}

BOOL osd_set_lang_environment(UINT16 langclass)
{
    ID_RSC            font_rsc_id = 0;

    if(langclass >= SIZE_LANG_TABLE)
    {
        langclass = 0;
    }
    g_lang_env = langclass;
    font_rsc_id = osdext_get_msg_lib_id(langclass);

    switch(font_rsc_id)
    {

#if(MULTI_MENU_LANG)
    case LIB_STRING_FRENCH:
        cur_string = (char *)French_strs_array;
        PRINTF("========Change to LIB_STRING_FRENCH=========\n");
        break;
    case LIB_STRING_GERMAN:
        cur_string = (char *)German_strs_array;
        PRINTF("========Change to LIB_STRING_GERMAN=========\n");
        break;
    case LIB_STRING_ITALY:
        cur_string = (char *)Italian_strs_array;
        PRINTF("========Change to LIB_STRING_ITALY=========\n");
        break;

    case LIB_STRING_RUSSIAN:
        cur_string = (char *)Russia_strs_array;
        PRINTF("========Change to LIB_STRING_RUSSIAN=========\n");
        break;

    case LIB_STRING_SPANISH:
        cur_string = (char *)Spanish_strs_array;
        break;
    case LIB_STRING_TURKISH:
        cur_string = (char *)Turkish_strs_array;
        break;
    case LIB_STRING_PORTUGUESE:
        cur_string = (char *)Portuguese_strs_array;
        break;
    case LIB_STRING_POLISH:
        cur_string = (char *)Polish_strs_array; 
        break;
    case LIB_STRING_ARABIC:
        cur_string = (char *)Arabic_strs_array;
        break;
    case LIB_STRING_THAI:
        cur_string =(char *) Thai_strs_array;
        break;
	case LIB_STRING_PERSIAN:
    	cur_string =(char *)Persian_strs_array;
    	break;
#ifdef HINDI_LANGUAGE_SUPPORT
	case LIB_STRING_HINDI:
		cur_string = (char *)hindi_strs_array;
		break;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
	case LIB_STRING_TELUGU:
		cur_string = (char *)telugu_strs_array;
		break;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
	case LIB_STRING_BENGALI:
		cur_string = (char *)bengali_strs_array;
		break;
#endif		
#endif
    case LIB_STRING_ENGLISH:
    default:
        cur_string = (char *)English_strs_array;
        PRINTF("========Change to LIB_STRING_ENGLISH=========\n");
        break;
    }
#ifdef BIDIRECTIONAL_OSD_STYLE
    switch(font_rsc_id)
    {
#if(MULTI_MENU_LANG    )
    case LIB_STRING_ARABIC:
#ifdef PERSIAN_SUPPORT
    case LIB_STRING_PERSIAN:
#endif
        mirror_flag = TRUE;
        break;
#endif
    default:
        mirror_flag = FALSE;
        break;
    }
#endif

    return TRUE;
}

#ifdef BIDIRECTIONAL_OSD_STYLE
static void check_mirror_flag(UINT16 w_lib_class, UINT16 u_index, OBJECTINFO* p_object_info)
{
    UINT16    w_class = w_lib_class&0xF000;

    // check for security
    // if the lib is one of the following two,pObjectInfo maynot be a empty value
    if(((LIB_FONT == w_class)||(LIB_ICON == w_class))&&(NULL == p_object_info))
    {
        return;
    }
    if(LIB_ICON == w_class)
    {
        if (TRUE == osd_get_mirror_flag())
        {
            UINT16 tmp_index = u_index;
            if ((IM_CHANNELLIST_TV == tmp_index) || (IM_CHANNELLIST_RADIO == tmp_index)
                || (IM_CHANNELLIST_MEDIAPLAY == tmp_index) || (IM_CHANNELLIST_PVR == tmp_index)
                || (IM_ORANGE_ARROW_S == tmp_index) || (IM_EPG == tmp_index) || (IM_INFORMATION_PVR== tmp_index)
                || (IM_VOLUME == tmp_index)
                || (IM_INFORMATION_TV == tmp_index) || (IM_INFORMATION_RADIO == tmp_index)
                || (IM_SATELLITE == tmp_index) || (IM_RECEIVE == tmp_index)
                || (IM_SATELLITE_LINE_1 == tmp_index) || (IM_SATELLITE_LINE_2 == tmp_index)
                || (IM_SATELLITE_LINE_3 == tmp_index) || (IM_SATELLITE_LINE_4 == tmp_index)
                || (IM_SATELLITE_LINE_5 == tmp_index) || (IM_MEDIAPLAYER == tmp_index)
                || (IM_MEDIAPLAYER_ON == tmp_index)||(IM_MEDIAPLAYER_SELECT == tmp_index)
                || (IM_CHANNEL == tmp_index) || (IM_CHANNEL_ON == tmp_index) || (IM_CHANNEL_SELECT == tmp_index)
                || (IM_INSTALLATION==tmp_index)||(IM_INSTALLATION_ON==tmp_index)||(IM_INSTALLATION_SELECT==tmp_index)
                || (IM_SYSTEM == tmp_index) || (IM_SYSTEM_ON == tmp_index) || (IM_SYSTEM_SELECT == tmp_index)
                || (IM_TOOLS == tmp_index) || (IM_TOOLS_ON == tmp_index) || (IM_TOOLS_SELECT == tmp_index)
#ifdef RECORD_SUPPORT
                || (IM_PVR == tmp_index) || (IM_PVR_ON == tmp_index) || (IM_PVR_SELECT == tmp_index)
#endif
            )
                p_object_info->m_obj_attr.m_mirror_flag = FALSE;
            else
            {
                p_object_info->m_obj_attr.m_mirror_flag = TRUE;
            }
        }
        else
        {
            p_object_info->m_obj_attr.m_mirror_flag = FALSE;
        }
    }
}
#endif

UINT8 *osd_get_thai_font_data(UINT16 w_lib_class,struct thai_cell *cell,OBJECTINFO *p_object_info)
{
    const font_t *font[3] ={NULL};
    UINT16 w_index[3] = {0};
    UINT16 stride = 0;  
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT32 font_width = 0;
    UINT32 font_height = 0;
    UINT32 font_max_width = 0;
    UINT32 *pu32 = NULL;
    UINT32 size = 0;
    UINT32 data_offset = 0;
    UINT32 u32 = 0;
    UINT32 u32_offset = 0;
    UINT32 cur_pix = 0;
    UINT32 dest_pix = 0;
#ifndef GE_DRAW_OSD_LIB
    UINT32 dest_offset = 0;
    UINT8 *pb_data_buf = NULL;
    UINT8 *font_byte=NULL;
#endif
    UINT32  __MAYBE_UNUSED__ dest_byte_offset = 0;
    UINT32 dest_bit_offset = 0;
    UINT32 *font_word = NULL;
    UINT32 *font_buf  = NULL;

    if((NULL == p_object_info) || (NULL == cell))
    {
        return NULL;
    }
    RSC_PRINTF("Call %s:\n",__FUNCTION__);

    p_object_info->m_w_obj_class=w_lib_class;

    for(i=0; i<cell->char_num; i++)
    {
        w_index[i] = cell->unicode[i];

        font[i] = get_font(w_lib_class,&w_index[i]);
        if(NULL == font[i])
        {
            return NULL;
        }
        if(font_max_width < font[i]->width[w_index[i]])
        {
            font_max_width = font[i]->width[w_index[i]];
            font_height =  font[i]->height;
        }
    }

    p_object_info->m_obj_attr.m_w_width = (font_max_width+7)/8*8;
    p_object_info->m_obj_attr.m_w_height = font_height;
    p_object_info->m_obj_attr.m_w_actual_width = font_max_width;
    p_object_info->m_obj_attr.m_b_color    = 1;
    stride = (font_max_width+7)/8;

    MEMSET(font_bitmap_thai,0,font_height*stride);

    font_buf = thai_font_buf;//MALLOC(((font_height*font_max_width+31)/32)*4);
    MEMSET(font_buf, 0, ((font_height*font_max_width+31)/32)*4);

    for(i=0; i<cell->char_num; i++)
    {
        // get wIndex font offset
        data_offset = 0;
        for(j=0;j<w_index[i];j++)
        {
            font_width = font[i]->width[j];
            size = (font_width * font_height + 31)>>5;//32;
            data_offset += size;
        }
        pu32 = (UINT32*)(font[i]->data + data_offset);

        font_width = font[i]->width[w_index[i]];
        u32 = 0;
        cur_pix = 0;
        for(j=0; j<font_height; j++)
        {
            dest_byte_offset = 0;
            for(k=0; k<font_width; k++)
            {
                u32_offset =  cur_pix & 0x1F;
                if(0 == u32_offset)
                {
                    u32     = *(pu32 + (cur_pix>>5));
                }

                dest_pix = j*font_max_width+font_max_width-font_width+k;
                dest_bit_offset = dest_pix & 0x1F;
                if( (0 == dest_bit_offset) ||  (0 == k))
                {
                    font_word = (UINT32*)&font_buf[dest_pix>>5];
                }
                if(u32 & (0x80000000>>u32_offset) )
                {
                    *font_word |= (0x80000000>> dest_bit_offset);
                }
                cur_pix++;
            }
        }
    }

    pu32 = font_buf;
#ifdef GE_DRAW_OSD_LIB
    if(osd_get_vscr_state())
    {
        osal_cache_flush((UINT8 *)pu32,font_height*font_max_width);
    }
    return (UINT8 *)pu32;
#else
    font_width = font_max_width;
    cur_pix = 0;
    dest_offset = 0;
    u32 = 0;
    for(i=0;i<font_height;i++,dest_offset+=stride)
    {
        dest_byte_offset = 0;
        for(j=0;j<font_width;j++,cur_pix++)
        {
            u32_offset =  cur_pix & 0x1F;
            if(0 == u32_offset)
            {
                u32     = *(pu32 + (cur_pix>>5));
            }
            dest_bit_offset = j & 0x07;
            if( 0 == dest_bit_offset)
            {
                font_byte = &font_bitmap_thai[dest_offset + dest_byte_offset++];
            }
            if(u32 & (0x80000000>>u32_offset) )
            {
                *font_byte |= (0x80>> dest_bit_offset);
            }
        }
    }

    //FREE(font_buf);
 
    pb_data_buf = font_bitmap_thai;
    RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d OK!\n",w_lib_class,w_index);

    return pb_data_buf;
#endif

}

#ifdef HINDI_LANGUAGE_SUPPORT
#if(MULTI_MENU_LANG)
BOOL osd_get_devanagari_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
	UINT8 fontsize = 0;
	font_t *font = NULL;
	
	if (!pWt || !pHt)
		return FALSE;
	
    fontsize = w_lib_class & 0x00FF;
	font = &font_devanagari[fontsize];
	*pWt= font->width[group_index];
	*pHt = font->height;

	return TRUE;
}

UINT8 *osd_get_devanagari_data(UINT16 w_lib_class, struct devanagari_cell *pCell, OBJECTINFO *pObjInfo)
{
	if (!pCell || !pObjInfo)
		return NULL;
	
    const font_t *font = NULL;
	const UINT32 *pData = NULL;
	UINT8 *pFontByte = NULL;
	UINT8 fontsize = 0;
	UINT8 pixelbits = 0;
	UINT32 taskid = 0;
	UINT32 fontheight = 0;
	UINT32 fontwidth = 0;
	UINT32 dataoffset = 0;	
	UINT16 chidx = pCell->group_idx;
	UINT16 stride = 0;
	UINT32 i = 0, j = 0;
	
    unsigned long u32 = 0;
    unsigned long u32_offset = 0;
    unsigned long cur_pix = 0;
    unsigned long dest_offset = 0;
    unsigned long dest_byte_offset = 0;
    unsigned long dest_bit_offset = 0;
    const unsigned long *pu32 = NULL;
	

    fontsize = w_lib_class & 0x00FF;
	font = &font_devanagari[fontsize];
    fontheight = font->height;
	pObjInfo->m_w_obj_class = w_lib_class;
    pObjInfo->m_obj_attr.m_w_width = ((font->width[chidx] + 7) / 8) * 8;
    pObjInfo->m_obj_attr.m_w_height = font->height;
    pObjInfo->m_obj_attr.m_w_actual_width = font->width[chidx];
//    pObjInfo->m_obj_attr.m_b_color = (font->bit_cnt > 0) ? font->bit_cnt : 1;
	pObjInfo->m_obj_attr.m_b_color = 1;
    pixelbits = pObjInfo->m_obj_attr.m_b_color;
    pObjInfo->m_obj_attr.m_w_width *= pixelbits;

#ifdef SUPPORT_MULTI_TASK
    taskid = osal_task_get_current_id();
    if (!font_bitmap[taskid - 1])
    {
        font_bitmap[taskid - 1] = (UINT8 *)MALLOC(FONT_BMP_LEN);
        if (!font_bitmap[taskid - 1])
        {
            return NULL;
        }
    }
	
    if (osd_get_vscr_state())
    {
        MEMSET(font_bitmap[taskid - 1], 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
    }
#else
    MEMSET(font_bitmap, 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
#endif

	for (i = 0; i < chidx; i++)
	{
		fontwidth = font->width[i];
		dataoffset += ((fontheight * fontwidth * pixelbits + 31) >> 5);
	}
	pData = font->data + dataoffset;

	dataoffset = 0;
	if (osd_get_vscr_state())
	{
        fontwidth = font->width[chidx];
        stride = (fontwidth+7)/8*pixelbits;
        cur_pix = 0;
        dest_offset = 0;
        u32 = 0;
        for(i=0;i<fontheight;i++)
        {
            dest_byte_offset = 0;
            for(j=0;j<fontwidth*pixelbits;j++) // total bits of a line
            {
                u32_offset = cur_pix & 0x1F;   // 2^5 = 32
                if(0 == u32_offset)  // multiple of 4 bytes
                {
                    u32 = *(pData + (cur_pix>>5));
                }
                dest_bit_offset = j & 0x07;	// 2^3 = 8
                if(0 == dest_bit_offset)   // integral bytes
                {
#ifdef SUPPORT_MULTI_TASK
                    pFontByte = font_bitmap[taskid - 1] + (dest_offset + dest_byte_offset);
                    dest_byte_offset++;
#else
                    pFontByte = &font_bitmap[dest_offset + dest_byte_offset++];
#endif
                }
                if(u32 & (0x80000000>>u32_offset) )
                {
                    *pFontByte |= (0x80>> dest_bit_offset);
                }
                cur_pix++;
            }
            dest_offset += stride;
        }
	}
	
#ifdef SUPPORT_MULTI_TASK
	pFontByte = font_bitmap[taskid - 1];
#else
	pFontByte = font_bitmap;
#endif

#ifdef GE_DRAW_OSD_LIB
        pFontByte = (UINT8 *)pData;
#endif

	return pFontByte;
}
#else
BOOL osd_get_devanagari_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
    return FALSE;
}

UINT8 *osd_get_devanagari_data(UINT16 w_lib_class, struct devanagari_cell *pCell, OBJECTINFO *pObjInfo)
{
    return NULL;
}
#endif
#endif

#ifdef TELUGU_LANGUAGE_SUPPORT
#if(MULTI_MENU_LANG)
BOOL osd_get_telugu_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
	UINT8 fontsize = 0;
	font_t *font = NULL;
	
	if (!pWt || !pHt)
		return FALSE;
	
    fontsize = w_lib_class & 0x00FF;
	font = &font_telugu[fontsize];
	*pWt= font->width[group_index];
	*pHt = font->height;

	return TRUE;
}

UINT8 *osd_get_telugu_data(UINT16 w_lib_class, struct telugu_cell *pCell, OBJECTINFO *pObjInfo)
{
	if (!pCell || !pObjInfo)
		return NULL;
	
    const font_t *font = NULL;
	const UINT32 *pData = NULL;
	UINT8 *pFontByte = NULL;
	UINT8 fontsize = 0;
	UINT8 pixelbits = 0;
	UINT32 taskid = 0;
	UINT32 fontheight = 0;
	UINT32 fontwidth = 0;
	UINT32 dataoffset = 0;	
	UINT16 chidx = pCell->group_idx;
	UINT16 stride = 0;
	UINT32 i = 0, j = 0;
	
    unsigned long u32 = 0;
    unsigned long u32_offset = 0;
    unsigned long cur_pix = 0;
    unsigned long dest_offset = 0;
    unsigned long dest_byte_offset = 0;
    unsigned long dest_bit_offset = 0;
    const unsigned long *pu32 = NULL;
	

    fontsize = w_lib_class & 0x00FF;
	font = &font_telugu[fontsize];
    fontheight = font->height;
	pObjInfo->m_w_obj_class = w_lib_class;
    pObjInfo->m_obj_attr.m_w_width = ((font->width[chidx] + 7) / 8) * 8;
    pObjInfo->m_obj_attr.m_w_height = font->height;
    pObjInfo->m_obj_attr.m_w_actual_width = font->width[chidx];
//    pObjInfo->m_obj_attr.m_b_color = (font->bit_cnt > 0) ? font->bit_cnt : 1;
	pObjInfo->m_obj_attr.m_b_color = 1;
    pixelbits = pObjInfo->m_obj_attr.m_b_color;
    pObjInfo->m_obj_attr.m_w_width *= pixelbits;

#ifdef SUPPORT_MULTI_TASK
    taskid = osal_task_get_current_id();
    if (!font_bitmap[taskid - 1])
    {
        font_bitmap[taskid - 1] = (UINT8 *)MALLOC(FONT_BMP_LEN);
        if (!font_bitmap[taskid - 1])
        {
            return NULL;
        }
    }
	
    if (osd_get_vscr_state())
    {
        MEMSET(font_bitmap[taskid - 1], 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
    }
#else
    MEMSET(font_bitmap, 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
#endif

	for (i = 0; i < chidx; i++)
	{
		fontwidth = font->width[i];
		dataoffset += ((fontheight * fontwidth * pixelbits + 31) >> 5);
	}
	pData = font->data + dataoffset;

	dataoffset = 0;
	if (osd_get_vscr_state())
	{
        fontwidth = font->width[chidx];
        stride = (fontwidth+7)/8*pixelbits;
        cur_pix = 0;
        dest_offset = 0;
        u32 = 0;
        for(i=0;i<fontheight;i++)
        {
            dest_byte_offset = 0;
            for(j=0;j<fontwidth*pixelbits;j++) // total bits of a line
            {
                u32_offset = cur_pix & 0x1F;   // 2^5 = 32
                if(0 == u32_offset)  // multiple of 4 bytes
                {
                    u32 = *(pData + (cur_pix>>5));
                }
                dest_bit_offset = j & 0x07;	// 2^3 = 8
                if(0 == dest_bit_offset)   // integral bytes
                {
#ifdef SUPPORT_MULTI_TASK
                    pFontByte = font_bitmap[taskid - 1] + (dest_offset + dest_byte_offset);
                    dest_byte_offset++;
#else
                    pFontByte = &font_bitmap[dest_offset + dest_byte_offset++];
#endif
                }
                if(u32 & (0x80000000>>u32_offset) )
                {
                    *pFontByte |= (0x80>> dest_bit_offset);
                }
                cur_pix++;
            }
            dest_offset += stride;
        }
	}
	
#ifdef SUPPORT_MULTI_TASK
	pFontByte = font_bitmap[taskid - 1];
#else
	pFontByte = font_bitmap;
#endif

#ifdef GE_DRAW_OSD_LIB
        pFontByte = (UINT8 *)pData;
#endif

	return pFontByte;
}

#else

BOOL osd_get_telugu_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
	return FALSE;
}
UINT8 *osd_get_telugu_data(UINT16 w_lib_class, struct telugu_cell *pCell, OBJECTINFO *pObjInfo)
{
	return NULL;
}
#endif
#endif

#ifdef BENGALI_LANGUAGE_SUPPORT
#if(MULTI_MENU_LANG)
BOOL osd_get_bengali_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
	UINT8 fontsize = 0;
	font_t *font = NULL;
	
	if (!pWt || !pHt)
		return FALSE;
	
    fontsize = w_lib_class & 0x00FF;
	font = &font_bengali[fontsize];
	*pWt= font->width[group_index];
	*pHt = font->height;

	return TRUE;
}

UINT8 *osd_get_bengali_data(UINT16 w_lib_class, struct bengali_cell *pCell, OBJECTINFO *pObjInfo)
{
	if (!pCell || !pObjInfo)
		return NULL;
	
    const font_t *font = NULL;
	const UINT32 *pData = NULL;
	UINT8 *pFontByte = NULL;
	UINT8 fontsize = 0;
	UINT8 pixelbits = 0;
	UINT32 taskid = 0;
	UINT32 fontheight = 0;
	UINT32 fontwidth = 0;
	UINT32 dataoffset = 0;	
	UINT16 chidx = pCell->group_idx;
	UINT16 stride = 0;
	UINT32 i = 0, j = 0;
	
    unsigned long u32 = 0;
    unsigned long u32_offset = 0;
    unsigned long cur_pix = 0;
    unsigned long dest_offset = 0;
    unsigned long dest_byte_offset = 0;
    unsigned long dest_bit_offset = 0;
    const unsigned long *pu32 = NULL;
	

    fontsize = w_lib_class & 0x00FF;
	font = &font_bengali[fontsize];
    fontheight = font->height;
	pObjInfo->m_w_obj_class = w_lib_class;
    pObjInfo->m_obj_attr.m_w_width = ((font->width[chidx] + 7) / 8) * 8;
    pObjInfo->m_obj_attr.m_w_height = font->height;
    pObjInfo->m_obj_attr.m_w_actual_width = font->width[chidx];
//    pObjInfo->m_obj_attr.m_b_color = (font->bit_cnt > 0) ? font->bit_cnt : 1;
	pObjInfo->m_obj_attr.m_b_color = 1;
    pixelbits = pObjInfo->m_obj_attr.m_b_color;
    pObjInfo->m_obj_attr.m_w_width *= pixelbits;

#ifdef SUPPORT_MULTI_TASK
    taskid = osal_task_get_current_id();
    if (!font_bitmap[taskid - 1])
    {
        font_bitmap[taskid - 1] = (UINT8 *)MALLOC(FONT_BMP_LEN);
        if (!font_bitmap[taskid - 1])
        {
            return NULL;
        }
    }
	
    if (osd_get_vscr_state())
    {
        MEMSET(font_bitmap[taskid - 1], 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
    }
#else
    MEMSET(font_bitmap, 0, fontheight * pObjInfo->m_obj_attr.m_w_width/(8/pixelbits));
#endif

	for (i = 0; i < chidx; i++)
	{
		fontwidth = font->width[i];
		dataoffset += ((fontheight * fontwidth * pixelbits + 31) >> 5);
	}
	pData = font->data + dataoffset;

	dataoffset = 0;
	if (osd_get_vscr_state())
	{
        fontwidth = font->width[chidx];
        stride = (fontwidth+7)/8*pixelbits;
        cur_pix = 0;
        dest_offset = 0;
        u32 = 0;
        for(i=0;i<fontheight;i++)
        {
            dest_byte_offset = 0;
            for(j=0;j<fontwidth*pixelbits;j++) // total bits of a line
            {
                u32_offset = cur_pix & 0x1F;   // 2^5 = 32
                if(0 == u32_offset)  // multiple of 4 bytes
                {
                    u32 = *(pData + (cur_pix>>5));
                }
                dest_bit_offset = j & 0x07;	// 2^3 = 8
                if(0 == dest_bit_offset)   // integral bytes
                {
#ifdef SUPPORT_MULTI_TASK
                    pFontByte = font_bitmap[taskid - 1] + (dest_offset + dest_byte_offset);
                    dest_byte_offset++;
#else
                    pFontByte = &font_bitmap[dest_offset + dest_byte_offset++];
#endif
                }
                if(u32 & (0x80000000>>u32_offset) )
                {
                    *pFontByte |= (0x80>> dest_bit_offset);
                }
                cur_pix++;
            }
            dest_offset += stride;
        }
	}
	
#ifdef SUPPORT_MULTI_TASK
	pFontByte = font_bitmap[taskid - 1];
#else
	pFontByte = font_bitmap;
#endif

#ifdef GE_DRAW_OSD_LIB
        pFontByte = (UINT8 *)pData;
#endif

	return pFontByte;
}

#else

BOOL osd_get_bengali_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt)
{
	return FALSE;
}
UINT8 *osd_get_bengali_data(UINT16 w_lib_class, struct bengali_cell *pCell, OBJECTINFO *pObjInfo)
{
	return NULL;
}
#endif
#endif

static void osd_get_bmp_data(UINT8 *pb_data_buf, bitmap_infor_t *bmpinfo)
{
    const UINT8 bit_count_32 = 32;
    const UINT8 bit_count_16 = 16;

    if((NULL == pb_data_buf) || (NULL == bmpinfo))
    {
        return ;
    }
    if(bit_count_32 == bmpinfo->bit_count) //4 == (bmpinfo->bmp_size /(bmpinfo->h*bmpinfo->w)))
    {
 
    #ifdef GE_DRAW_OSD_LIB
        VSCR src_vscr,dst_vscr;
        dst_vscr.lpb_scr     = pb_data_buf;
        src_vscr.lpb_scr     = (UINT8 *)(bmpinfo->data);
        src_vscr.b_color_mode = osd_get_cur_color_mode();//OSD_HD_ARGB8888
        src_vscr.v_r.u_width  = bmpinfo->w;
        src_vscr.v_r.u_height = bmpinfo->h;
        src_vscr.v_r.u_top    = 0;
        src_vscr.v_r.u_left   = 0;
        lib_ge_vscr_draw_color(&dst_vscr,&src_vscr,bmpinfo->bmp_size,bmpinfo->bit_count,&src_vscr.v_r,NULL,0,LIB_GE_DRAW_RLE_DECODE);
    #else
        if((UINT32)bmp_rle32_decode((BYTE *)bmpinfo->data,bmpinfo->data_size,pb_data_buf,4)!=bmpinfo->bmp_size)
        {
            pb_data_buf = NULL;
        }
    #endif        
       
    }
    else if(bit_count_16 == bmpinfo->bit_count )////2 == (bmpinfo->bmp_size /(bmpinfo->h*bmpinfo->w)))
    {
    #ifdef GE_DRAW_OSD_LIB
        VSCR src_vscr,dst_vscr;
        dst_vscr.lpb_scr     = pb_data_buf;
        src_vscr.lpb_scr     = (UINT8 *)(bmpinfo->data);
        src_vscr.b_color_mode = osd_get_cur_color_mode();//OSD_HD_ARGB1555;
        src_vscr.v_r.u_width  = bmpinfo->w;
        src_vscr.v_r.u_height = bmpinfo->h;
        src_vscr.v_r.u_top    = 0;
        src_vscr.v_r.u_left   = 0;
        lib_ge_vscr_draw_color(&dst_vscr,&src_vscr,bmpinfo->bmp_size,bmpinfo->bit_count,&src_vscr.v_r,NULL,0,LIB_GE_DRAW_RLE_DECODE);
    #else
        if(bmp_rle16_decode((UINT8 *)bmpinfo->data,bmpinfo->data_size,pb_data_buf) !=bmpinfo->bmp_size)
        {
                ASSERT(0);
                pb_data_buf = NULL;
        }
    #endif
    }
    else
    {
        if(bmp_rle_decode(bmpinfo->data,bmpinfo->data_size,pb_data_buf)!=bmpinfo->bmp_size)
        {
            //FREE(pb_data_buf);
            pb_data_buf = NULL;
        }
    }
}

#ifdef SCALE_FONT_SUPPORT
static void osd_scale_font(UINT16 w_lib_class, UINT32 font_width, UINT32 font_height, const font_t *font, \
        OBJECTINFO* p_object_info, UINT8 bit_per_pix, UINT8 *pb_data_buf)
{
    UINT8 *scale_data_buffer = NULL;

#ifdef SUPPORT_MULTI_TASK
    scale_data_buffer = font_bitmap[tsk_id - 1]+FONT_BMP_LEN/2;
    if(osd_scale_char_matrix(w_lib_class, font_width, font_height, font, p_object_info, font_bitmap[tsk_id - 1], \
                            scale_data_buffer, bit_per_pix))
    {
        pb_data_buf = scale_data_buffer;
    }
#else
    scale_data_buffer = font_bitmap+FONT_BMP_LEN/2;
    if(osd_scale_char_matrix(w_lib_class, font_width, font_height, font, p_object_info, font_bitmap, \
                            scale_data_buffer, bit_per_pix))
    {
        pb_data_buf = scale_data_buffer;
    }
#endif
}
#endif

UINT8 *osd_get_rsc_obj_data(UINT16 w_lib_class,UINT16 u_index,OBJECTINFO *p_object_info)
{
    UINT16 w_index = 0;
    UINT16 w_class = 0;
    UINT16 stride = 0;
    const unsigned char *str_array = NULL;
    const font_t *font = NULL;
    bitmap_infor_t *bmpinfo = NULL;
    UINT8 *pb_data_buf = NULL;
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long font_width = 0;
    unsigned long font_height = 0;
    const unsigned long *pu32 = NULL;
    unsigned long offset = 0;
    unsigned long size = 0;
    unsigned long data_offset = 0;
    unsigned long u32 = 0;
    unsigned long u32_offset = 0;
    unsigned long cur_pix = 0;
    unsigned long dest_offset = 0;
    unsigned long dest_byte_offset = 0;
    unsigned long dest_bit_offset = 0;
    unsigned char *font_byte = NULL;
    unsigned long str_num = 0;
    UINT32   tsk_id = 0;
    UINT8 bit_per_pix = 0;

    if(NULL == p_object_info)
    {
        return NULL;
    }
    p_object_info->m_w_obj_class=w_lib_class;
    w_index=u_index;
    w_class = w_lib_class&0xF000;
    if(LIB_ICON == w_class)
    {
        w_index -= 0x8001;
        RSC_PRINTF("Get LIB_ICON: 0x%04x,wIndex=%d\n",w_lib_class,w_index);
        if(w_index>=sizeof(bitmaps_infor)/sizeof(bitmaps_infor[0]))
        {
            return NULL;
        }
        bmpinfo = &bitmaps_infor[w_index];
        p_object_info->m_obj_attr.m_w_width        = bmpinfo->bmp_size/bmpinfo->h;
        p_object_info->m_obj_attr.m_w_height       = bmpinfo->h;
        p_object_info->m_obj_attr.m_w_actual_width = bmpinfo->w;
        //p_object_info->m_obj_attr.m_b_color = (bmpinfo->bmp_size/(bmpinfo->h*bmpinfo->w))<<3;
        p_object_info->m_obj_attr.m_b_color = bmpinfo->bit_count;
#ifdef BIDIRECTIONAL_OSD_STYLE
        check_mirror_flag(w_lib_class,u_index, p_object_info);
#endif
#ifdef SUPPORT_MULTI_TASK
        tsk_id = osal_task_get_current_id();
        if(NULL == font_bitmap[tsk_id - 1])
        {
            font_bitmap[tsk_id - 1] = bitmap_buffer;//(UINT8 *)MALLOC(FONT_BMP_LEN);
            if(NULL == font_bitmap[tsk_id - 1])
            {
                return NULL;
            }
        }
        if(bmpinfo->bmp_size > FONT_BMP_LEN)
        {
            return NULL;
        }
        else
        {
            pb_data_buf = font_bitmap[tsk_id - 1];
        }
#else
        if(bmpinfo->bmp_size > sizeof(font_bitmap) )
        {
            return NULL;
        }
        else
        {
            pb_data_buf = font_bitmap;
        }
#endif
        if(pb_data_buf!=NULL)
        {
            osd_get_bmp_data(pb_data_buf,bmpinfo);
        }
    }
    else if(LIB_STRING == w_class)
    {
        w_index -= 1;
        str_array = (unsigned char *)cur_string;
        RSC_PRINTF("0x%02x,0x%02x,0x%02x \n",str_array[0],str_array[1],str_array[2]);
        str_num = (str_array[1]<<8) + str_array[2];
        RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,str_num=%d\n",w_lib_class,w_index,str_num);
        if(w_index >= str_num)
        {
            return NULL;
        }
        offset = (str_array[3 + 2*w_index]<<8) + str_array[3 + 2*w_index + 1];
        pb_data_buf = (UINT8*)&str_array[offset];
        RSC_PRINTF("Get LIB_STRING:0x%04x,wIndex=%d,offset=%d,pbDatabuf=0x%x\n",w_lib_class,w_index,offset,pb_data_buf);
    }
    else if(LIB_FONT == w_class)
    {
        RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d\n",w_lib_class,w_index);
        font = get_font(w_lib_class,&w_index);
        if(NULL == font)
        {
            return NULL;
        }
        p_object_info->m_obj_attr.m_w_width     = (font->width[w_index]+7)/8*8;
        p_object_info->m_obj_attr.m_w_height    = font->height;
        font_height = font->height;
        p_object_info->m_obj_attr.m_w_actual_width = font->width[w_index];
        p_object_info->m_obj_attr.m_b_color     = 1;
        if(font->bit_cnt)
        {
            p_object_info->m_obj_attr.m_b_color = font->bit_cnt;
        }
        bit_per_pix = p_object_info->m_obj_attr.m_b_color;
        p_object_info->m_obj_attr.m_w_width *= bit_per_pix;
    #ifdef SUPPORT_MULTI_TASK
        tsk_id = osal_task_get_current_id();
        if(NULL== font_bitmap[tsk_id - 1])
        {
            font_bitmap[tsk_id - 1] = (UINT8 *)MALLOC(FONT_BMP_LEN);
            if(NULL == font_bitmap[tsk_id - 1])
            {
                return NULL;
            }
        }
        if(osd_get_vscr_state())
        {
            MEMSET(font_bitmap[tsk_id - 1],0,font_height * p_object_info->m_obj_attr.m_w_width/(8/bit_per_pix));
        }
    #else
        MEMSET(font_bitmap,0,font_height * p_object_info->m_obj_attr.m_w_width/(8/bit_per_pix));
    #endif
        data_offset = 0;
        for(i=0;i<w_index;i++)
        {
            font_width = font->width[i];
            size = (font_width * font_height * bit_per_pix + 31)>>5;//32;
            data_offset += size;
        }
        pu32 = font->data + data_offset;
        if(osd_get_vscr_state())
        {
            font_width = font->width[w_index];
            stride = (font_width+7)/8*bit_per_pix;
            cur_pix = 0;
            dest_offset = 0;
            RSC_PRINTF("%c,font_width=%d,font_height=%d,stride=%d,data_offset=%d\n",
                        w_index,font_width,font_height,stride,data_offset);
            u32 = 0;
            for(i=0;i<font_height;i++)
            {
                dest_byte_offset = 0;
                for(j=0;j<font_width*bit_per_pix;j++)
                {
                    u32_offset = cur_pix & 0x1F;
                    if(0 == u32_offset)
                    {
                        u32 = *(pu32 + (cur_pix>>5));
                    }
                    dest_bit_offset = j & 0x07;
                    if(0 == dest_bit_offset)
                    {
#ifdef SUPPORT_MULTI_TASK
                        font_byte = font_bitmap[tsk_id - 1] + (dest_offset + dest_byte_offset);
                        dest_byte_offset++;
#else
                        font_byte = &font_bitmap[dest_offset + dest_byte_offset++];
#endif
                    }
                    if(u32 & (0x80000000>>u32_offset) )
                    {
                        *font_byte |= (0x80>> dest_bit_offset);
                    }
                    cur_pix++;
                }
                dest_offset += stride;
            }
        }
#ifdef SUPPORT_MULTI_TASK
        pb_data_buf = font_bitmap[tsk_id - 1];
#else
        pb_data_buf = font_bitmap;
#endif
#ifdef SCALE_FONT_SUPPORT
        osd_scale_font(w_lib_class,font_width,font_height,font,p_object_info,bit_per_pix,pb_data_buf);
#endif
#ifdef GE_DRAW_OSD_LIB
        if(osd_get_cur_color_mode() != OSD_256_COLOR)
        pb_data_buf = (UINT8 *)pu32;
#endif
        RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d OK!\n",w_lib_class,w_index);
    }
    else if(LIB_PALLETE == w_class)
    {
        RSC_PRINTF("Get LIB_PALLETE: 0x%04x,wIndex=%d\n",w_lib_class,w_index);
        w_index &= 0x0F;
        if(w_index>=sizeof(palette_array)/sizeof(palette_array[0]))
        {
            w_index = 0;
        }
        pb_data_buf = palette_array[w_index];
    }
    else
    {
        RSC_PRINTF("Get Unkown data: 0x%04x,wIndex=%d\n",w_lib_class,w_index);
        return NULL;
    }
    return pb_data_buf;
}

UINT8 *osd_get_rsc_obj_data_ext(UINT16 w_lib_class, UINT16 u_index, OBJECTINFO *p_object_info,
                             UINT8 *font_bitmap_ext,UINT32 font_bitmap_size)
{
    UINT16 w_index = 0;
    UINT16 w_class = 0;
    UINT16 stride = 0;

    const unsigned char *str_array = NULL;

    const font_t *font = NULL;
    bitmap_infor_t *bmpinfo = NULL;

    UINT8 *pb_data_buf = NULL;
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long font_width = 0;
    unsigned long font_height = 0;
    const unsigned long *pu32 = NULL;
    unsigned long offset = 0;
    unsigned long size = 0;
    unsigned long data_offset = 0;
    unsigned long u32 = 0;
    unsigned long u32_offset = 0;
    unsigned long cur_pix = 0;
    unsigned long dest_offset = 0;
    unsigned long dest_byte_offset = 0;
    unsigned long dest_bit_offset = 0;
    unsigned char *font_byte = NULL;
    unsigned long str_num = 0;

    if((!p_object_info) || (!font_bitmap_ext))
    {
        return NULL;
    }
    RSC_PRINTF("Call %s:\n", __FUNCTION__);
    p_object_info->m_w_obj_class = w_lib_class;
    w_index = u_index;
    w_class = w_lib_class &0xF000;

    if (LIB_ICON == w_class)
    {
        w_index -= 0x8001;

        RSC_PRINTF("Get LIB_ICON: 0x%04x,wIndex=%d\n", w_lib_class, w_index);

        if (w_index >= sizeof(bitmaps_infor) / sizeof(bitmaps_infor[0]))
        {
            return NULL;
        }

        bmpinfo = &bitmaps_infor[w_index];

        p_object_info->m_obj_attr.m_w_width = bmpinfo->bmp_size / bmpinfo->h;
        p_object_info->m_obj_attr.m_w_height = bmpinfo->h;
        p_object_info->m_obj_attr.m_w_actual_width = bmpinfo->w;
        p_object_info->m_obj_attr.m_b_color = 8;

    #ifdef BIDIRECTIONAL_OSD_STYLE
        check_mirror_flag(w_lib_class,u_index, p_object_info);
#endif
        //        pb_data_buf = MALLOC(bmpinfo->bmp_size);
        //if (bmpinfo->bmp_size > sizeof(font_bitmap_ext))
        if (bmpinfo->bmp_size > font_bitmap_size)
        {
            return NULL;
        }
        else
        {
            pb_data_buf = font_bitmap_ext;
        }
        if (pb_data_buf != NULL)
        {
            if (bmp_rle_decode(bmpinfo->data, bmpinfo->data_size, pb_data_buf) != bmpinfo->bmp_size)
            {
                //FREE(pb_data_buf);
                pb_data_buf = NULL;
            }
            //else    printf("MALLOC 0X%08X(%dK)id=%d\t",pb_data_buf,bmpinfo->bmp_size/1024,wIndex);
        }

        //    pb_data_buf = bmpinfo->data;

    }
    else if (LIB_STRING == w_class)
    {
        w_index -= 1;
        str_array = (unsigned char *)cur_string;
        RSC_PRINTF("0x%02x,0x%02x,0x%02x \n", str_array[0], str_array[1], str_array[2]);

        str_num = (str_array[1] << 8) + str_array[2];

        RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,str_num=%d\n", w_lib_class, w_index, str_num);
        if (w_index >= str_num)
        {
            return NULL;
        }

        offset = (str_array[3+2 * w_index] << 8) + str_array[3+2 * w_index + 1];

        pb_data_buf = (UINT8*) &str_array[offset];
        RSC_PRINTF("Get LIB_STRING:0x%04x,wIndex=%d,offset=%d,pbDatabuf=0x%x\n",w_lib_class,w_index,offset,pb_data_buf);

    }
    else if (LIB_FONT == w_class)
    {
        RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d\n", w_lib_class, w_index);

        font = get_font_1bit(w_lib_class, &w_index);
        if (NULL == font)
        {
            return NULL;
        }

        p_object_info->m_obj_attr.m_w_width = (font->width[w_index] + 7) / 8 * 8;
        p_object_info->m_obj_attr.m_w_height = font->height;
        font_height = font->height;
        p_object_info->m_obj_attr.m_w_actual_width = font->width[w_index];
        p_object_info->m_obj_attr.m_b_color = 1;
        MEMSET(font_bitmap_ext, 0, font_height *p_object_info->m_obj_attr.m_w_width / 8);
        data_offset = 0;
        for (i = 0; i < w_index; i++)
        {
            font_width = font->width[i];
            size = (font_width *font_height + 31) >> 5; //32;
            data_offset += size;
        }

        pu32 = font->data + data_offset;
        font_width = font->width[w_index];
        stride = (font_width + 7) / 8;
        cur_pix = 0;
        dest_offset = 0;
        RSC_PRINTF("%c,font_width=%d,font_height=%d,stride=%d,data_offset=%d\n", w_index, \
                    font_width, font_height, stride, data_offset);
        u32 = 0;
        for (i = 0; i < font_height; i++)
        {
            dest_byte_offset = 0;
            for (j = 0; j < font_width; j++)
            {
                u32_offset = cur_pix &0x1F;
                if (0 == u32_offset)
                {
                    u32 = *(pu32 + (cur_pix >> 5));
                }
                dest_bit_offset = j &0x07;
                if (0 == dest_bit_offset)
                {
                    font_byte = &font_bitmap_ext[dest_offset + dest_byte_offset];
                    dest_byte_offset++;
                }
                if (u32 &(0x80000000 >> u32_offset))
                {
                    *font_byte |= (0x80 >> dest_bit_offset);
                }
                cur_pix++;
            }
            dest_offset += stride;
        }

        pb_data_buf = font_bitmap_ext;
        RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d OK!\n", w_lib_class, w_index);

    }
    else if (LIB_PALLETE == w_class)
    {
        RSC_PRINTF("Get LIB_PALLETE: 0x%04x,wIndex=%d\n", w_lib_class, w_index);
        w_index &= 0x0F;
        if (w_index >= sizeof(palette_array) / sizeof(palette_array[0]))
        {
            w_index = 0;
        }
        pb_data_buf = palette_array[w_index];
    }
    else
    {
        RSC_PRINTF("Get Unkown data: 0x%04x,wIndex=%d\n", w_lib_class, w_index);
        return NULL;
    }

    return pb_data_buf;
}


BOOL osd_get_obj_info(UINT16 w_lib_class,UINT16 u_index,OBJECTINFO *p_object_info)
{
    UINT16    w_class = 0;
    UINT16    w_index = 0;
    const font_t *font = NULL;
    bitmap_infor_t *bmpinfo  = NULL;

    if (NULL == p_object_info)
    {
        return FALSE;
    }
    RSC_PRINTF("Call %s:\n",__FUNCTION__);

    p_object_info->m_w_obj_class = w_lib_class;


    // the content dwIndex points to is UNICODE.
    w_index= u_index;

    // ADD operation help to Get the high-level resource-type
    // add 0x80 to in case of so many string type
    w_class=w_lib_class&0xF000;

    // check for security
    // if the lib is one of the following two,pObjectInfo maynot be a empty value
    #if 0
    if(((LIB_FONT == w_class)||(LIB_ICON == w_class))&&(NULL == p_object_info))
    {
        return 0;
    }
    #endif
    if(LIB_FONT == w_class)
    {
        font = get_font(w_lib_class,&w_index);
        if(NULL == font)
        {
            return FALSE;
        }

#ifdef SCALE_FONT_SUPPORT
        UINT32 fontsize = get_font_size_index(w_lib_class);
        UINT32 font_width = font->width[w_index];
        font_width = font_width*font_size_ary[fontsize].w_mul/font_size_ary[fontsize].w_div;
        p_object_info->m_obj_attr.m_w_width  = (font_width + 7)/8;
        p_object_info->m_obj_attr.m_w_height = font->height*font_size_ary[fontsize].h_mul/font_size_ary[fontsize].h_div;
        p_object_info->m_obj_attr.m_w_actual_width = font->width[w_index]*font_size_ary[fontsize].w_mul/
                                                font_size_ary[fontsize].w_div;
#else
        p_object_info->m_obj_attr.m_w_width     = (font->width[w_index]+7)/8;
        p_object_info->m_obj_attr.m_w_height    = font->height;
        p_object_info->m_obj_attr.m_w_actual_width = font->width[w_index];
#endif
        p_object_info->m_obj_attr.m_b_color    = 1;
    }
    else if(LIB_ICON == w_class)
    {
        w_index -= 0x8001;

        if(w_index>=sizeof(bitmaps_infor)/sizeof(bitmaps_infor[0]))
        {
            return FALSE;
        }
        bmpinfo = &bitmaps_infor[w_index];
        p_object_info->m_obj_attr.m_w_width     = bmpinfo->bmp_size/bmpinfo->h;
        p_object_info->m_obj_attr.m_w_height     = bmpinfo->h;
        p_object_info->m_obj_attr.m_w_actual_width = bmpinfo->w;
        //p_object_info->m_obj_attr.m_b_color    = 8;
        //p_object_info->m_obj_attr.m_b_color = (bmpinfo->bmp_size/(bmpinfo->h*bmpinfo->w))<<3;
        p_object_info->m_obj_attr.m_b_color = bmpinfo->bit_count;

    #ifdef BIDIRECTIONAL_OSD_STYLE
        check_mirror_flag(w_lib_class,u_index, p_object_info);
    #endif
    }
    else
    {

        RSC_PRINTF("Get Unkown infor: 0x%04x,wIndex=%d\n",w_lib_class,w_index);
        return FALSE;
    }
    RSC_PRINTF("Get INFOR: 0x%04x,wIndex=%d\n",w_lib_class,w_index);

    return TRUE;
}

BOOL       osd_release_obj_data(UINT8 *lp_data_buf,lp_objectinfo p_object_info)
{
    return TRUE;
}

/**************************************************************************************
* Function Name:    osd_get_default_font_lib
*
* Description:        Get a character's FONT Library ID.
*        
* Arguments:
*        u_char:     IN;        The character UNICODE.
*
* Return value:
*        ID_RSC:        The font Library ID of the caracter
*
* NOTES:    Its detecting order is: ASCII char, MASS char, the language we currently used.
*
**************************************************************************************/
ID_RSC osd_get_default_font_lib(UINT16 u_char)
{
    UINT8 hbyte = 0;
    UINT8  __MAYBE_UNUSED__ lbyte = 0;

    const UINT8 basic_ascii_max_value   = 0x7F;
    const UINT8 ext_ascii_max_value        = 0xFF;
#ifdef CAS9_V6 //char_selected
    const UINT16 latin_ext_a_unicode_min_value = 0x0100;
    const UINT16 latin_ext_a_unicode_max_value = 0x017F;

    const UINT16 latin_ext_b_unicode_min_value = 0x0180;
    const UINT16 latin_ext_b_unicode_max_value = 0x024F;
#endif
    const UINT16 latine_unicode_max_value = 0x01FF;

    const UINT8 greek_unicode_hbyte     = 0x03;
    const UINT8 cyrillic_unicode_hbyte  = 0x04;
#ifdef CAS9_V6 //char_selected
    const UINT16 letterlike_unicode_min_value = 0x2100;
    const UINT16 letterlike_unicode_max_value = 0x214F;
#endif
    const UINT16 thai_unicode_min_value = 0x0E00;
    const UINT16 thai_unicode_max_value = 0x0E7F;

    const UINT8 masschar1_unicode_hbyte_min_value = 0xE0;
    const UINT8 masschar1_unicode_lbyte_max_value = 0xF8;

    const UINT16 arabic_unicode_min_value = 0x0600;
    const UINT16 arabic_unicode_max_value = 0x06FF;

    const UINT16 arabic_presentation_a_min_value = 0xFB50;
    const UINT16 arabic_presentation_a_max_value = 0xFDFF;

    const UINT16 arabic_presentation_b_min_value = 0xFE80;
    const UINT16 arabic_presentation_b_max_value = 0xFEFC;

#ifdef HINDI_LANGUAGE_SUPPORT
	const UINT16 devanagari_unicode_min = 0x0900;
	const UINT16 devanagari_unicode_max = 0x097F;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
	const UINT16 telugu_unicode_min = 0x0C00;
	const UINT16 telugu_unicode_max = 0x0C7F;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	const UINT16 bengali_unicode_min = 0x0980;
	const UINT16 bengali_unicode_max = 0x09FF;
#endif	

    hbyte = (UINT8)(u_char>>8);
    lbyte = (UINT8)u_char;

    if(u_char <= basic_ascii_max_value)
    {
        return LIB_FONT_DEFAULT;
    }
    else if(u_char <= ext_ascii_max_value)
    {
        return LIB_FONT_LATIN1;
    }
#ifdef CAS9_V6 //char_selected
    else if((u_char >= latin_ext_a_unicode_min_value)&&(u_char <= latin_ext_a_unicode_max_value))//unicode:0x0100~0x017F
    {
        return LIB_FONT_LATINEXTA;
    }
    else if((u_char >= latin_ext_b_unicode_min_value)&&(u_char <= latin_ext_b_unicode_max_value))//unicode:0x0180~0x024F
    {
        return LIB_FONT_LATINEXTB;
    }
#endif
    else if(u_char < latine_unicode_max_value)
    {
        return LIB_FONT_LATINE;
    }
    else if(hbyte == greek_unicode_hbyte)
    {
        return LIB_FONT_GREEK;
    }
    else if(hbyte == cyrillic_unicode_hbyte)
    {
        return LIB_FONT_CYRILLIC;
    }
#ifdef CAS9_V6 //char_selected
    else if((u_char >= letterlike_unicode_min_value)&&(u_char <= letterlike_unicode_max_value))//unicode: 0x2100~0x214F
    {
        return LIB_FONT_LETTERLIKE;
    }
#endif
    //else if(hbyte == 0x06|| (hbyte == 0xFE && (lbyte>=0x80 && lbyte<=0xFC) ) )
    //lwj for persion support
    else if(((u_char >= arabic_unicode_min_value)&&(u_char <= arabic_unicode_max_value))||
            ((u_char >= arabic_presentation_a_min_value)&&(u_char <= arabic_presentation_a_max_value))||
            ((u_char >= arabic_presentation_b_min_value)&&(u_char <= arabic_presentation_b_max_value)))
    {
        return LIB_FONT_ARABIC;
    }
#ifdef HINDI_LANGUAGE_SUPPORT
	else if (u_char >= devanagari_unicode_min && u_char <= devanagari_unicode_max)
	{
		return LIB_FONT_DEVANAGARI;
	}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	else if (u_char >= telugu_unicode_min && u_char <= telugu_unicode_max)
	{
		return LIB_FONT_TELUGU;
	}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	else if (u_char >= bengali_unicode_min && u_char <= bengali_unicode_max)
	{
		return LIB_FONT_BENGALI;
	}
#endif	
    else if((u_char >= thai_unicode_min_value) && (u_char <= thai_unicode_max_value))
    {
        return LIB_FONT_THAI;
    }
    else if((hbyte >= masschar1_unicode_hbyte_min_value) && (hbyte <= masschar1_unicode_lbyte_max_value))
    {
        return LIB_FONT_MASSCHAR1;
    }
    else
    {
        return osdext_get_font_lib_id(osd_get_lang_environment());
    }

}


ID_RSC osdext_get_msg_lib_id(UINT16 env_id)
{
    lp_lang_table    p_lang_tab = (lp_lang_table)g_lang_rsc_map_table;
    UINT8            i = 0;

    for (i=0; i<SIZE_LANG_TABLE; i++)
    {
        if (p_lang_tab->lang_id == env_id)
        {
            return p_lang_tab->str_id;
        }
        else
        {
            p_lang_tab++;
        }
    }
    return LIB_STRING_ENGLISH;
}

PWINSTYLE osdext_get_win_style(UINT8 b_idx)
{
    if (b_idx < SIZE_WINSTYLE)
    {
        return winstyle_array[cur_wstyle] + b_idx;
    }
    else
    {
        return winstyle_array[cur_wstyle] +0;    // Error Handle
    }
}

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
void osdext_set_win_style(UINT8 b_idx, UINT32 b_color, UINT32 f_color)
{
    PWINSTYLE t_winstyle = NULL;

    t_winstyle = winstyle_array[cur_wstyle] + b_idx;
    if( b_idx != WSTL_OSM_POP_UP_TEXT_FG_BLACK)
    {
        t_winstyle->w_bg_idx = b_color;
    }
    t_winstyle->w_fg_idx = f_color;

//    memcpy(winstyle_array[cur_wstyle] + b_idx, winstyle, sizeof(WINSTYLE));
}
#endif

#ifdef BIDIRECTIONAL_OSD_STYLE
BOOL osd_get_mirror_flag(void)
{
    return mirror_flag;
}
#endif

#ifdef FONT_1BIT_RSC_SUPPORT
font_t *osd_get_font_array_1bit(void)
{
    return (font_t *)font_array_1bit;
}
font_subt_t *osd_get_font_array_1bit_subt(void)
{
    return (font_subt_t *)font_array_1bit_subt;
}
#else
font_t *osd_get_font_array_1bit(void)
{
    return NULL;
}

font_subt_t *osd_get_font_array_1bit_subt(void)
{
    return NULL;
}
#endif

