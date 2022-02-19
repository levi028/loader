/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ge_osd.c
*
*    Description: GE draw OSD resource.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <api/libosd/osd_lib.h>


#ifdef GE_DRAW_OSD_LIB

#define LIB_CMD_SIZE        4096
#define FONT_BMP_SIZE       20480

#ifdef OSD_16BIT_SUPPORT
#define BMP_BUFFER          1280*720*2
#else
#define BMP_BUFFER          1280*720*4//204800
#endif
#define ATTR_ALIGN_32   __attribute__((aligned(32)))

#if 0
#define LIB_DE_DEBUG(...)            do{}while(0)
#else
#define LIB_DE_DEBUG           libc_printf
#endif




#define LIB_GE_HANDLE   (g_lib_ge_dev)
#define argb1555_get_r(x)    ((x>>10) & 0x1F)
#define argb1555_get_g(x)    ((x>>5) & 0x1F)
#define argb1555_get_b(x)    (x & 0x1F)

#define argb4444_get_a(x)    ((x >> 12) & 0xF)
#define argb4444_get_r(x)    ((x>>8) & 0xF)
#define argb4444_get_g(x)    ((x>>4) & 0xF)
#define argb4444_get_b(x)    (x & 0xF)
#define ARGB1555_2_8888(x)   ((((x&0x8000) == 0x8000?0xFF:0x00)<<24)| \
                              ((argb1555_get_r(x)<<3)<<16) |\
                              ((argb1555_get_g(x)<<3)<<8)|(argb1555_get_b(x)<<3))

#define ARGB4444_2_8888(x)   (((argb4444_get_a(x)<<4)<<24)|((argb4444_get_r(x)<<4)<<16) \
                             |((argb4444_get_g(x)<<4)<<8)|(argb4444_get_b(x)<<4))


/*
#define GE_SET_BASE_M(pbase,address,pitch, base_sel, format, decoder,flags) \
    do{ \
        ((ge_base_addr_t *)(pbase))->base_address = (UINT32)(address); \
        ((ge_base_addr_t *)(pbase))->pixel_pitch = (UINT16)(pitch); \
        ((ge_base_addr_t *)(pbase))->base_addr_sel = \
                                                     (enum GE_BASE_ADDR_SEL)(base_sel); \
        ((ge_base_addr_t *)(pbase))->color_format = \
                                                    (enum GE_PIXEL_FORMAT)(format); \
        ((ge_base_addr_t *)(pbase))->data_decoder= \
                                                   (enum GE_DATA_DECODER)(decoder); \
        ((ge_base_addr_t *)(pbase))->modify_flags = (UINT16)(flags); \
    }while(0)
*/

#ifdef _INVW_JUICE
// Inview: to prevent build warning messages
#undef NULL
#define NULL            ((void *)0)
#endif

static UINT32  font_pal[256] ATTR_ALIGN_32; // 3 pallette must be 8 byte aligne
static UINT8   *font_buffer        = NULL;
static UINT8   *mask_data_buffer   = NULL;
static UINT8   *font_warp          = NULL;
static UINT8   *bitmap_buffer      = NULL;

static BOOL    b_start_draw        = FALSE;
static UINT32  trans_color_low = 0x00000000;
static UINT32  trans_color_high = 0x00FFFFFF;
static ge_cmd_list_hdl     g_ge_osd_cmd_hdl = 0;
static struct ge_device    *g_lib_ge_dev = NULL;
static const UINT8         const_pixel_4_bits = 4;
static const UINT8         const_pixel_1_bits = 1;

RET_CODE lib_ge_draw_open(struct ge_device *ge_dev)
{
    if (ge_dev)
    {
        g_lib_ge_dev = ge_dev;
    }
    else
    {
        return -1;
    }
    MEMSET(font_pal,0x00,sizeof(font_pal));
    g_ge_osd_cmd_hdl = ge_cmd_list_create(ge_dev, LIB_CMD_SIZE);
#ifdef _INVW_JUICE
    if (NULL == font_buffer)
    {
        font_buffer = (UINT8 *)MALLOC(FONT_BMP_SIZE);

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    	if (NULL == font_buffer)
	    {
            return -1;
	    }
    }

    if (NULL == mask_data_buffer)
    {
        mask_data_buffer = (UINT8 *)MALLOC(FONT_BMP_SIZE);

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    	if (NULL == mask_data_buffer)
	    {
            return -1;
	    }
    }

    if (NULL == font_warp)
    {
        font_warp = (UINT8 *)MALLOC(FONT_BMP_SIZE);

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    	if (NULL == font_warp)
	    {
            return -1;
	    }
    }

    if (NULL == bitmap_buffer)
    {
        bitmap_buffer = (UINT8 *)MALLOC(BMP_BUFFER);

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    	if (NULL == bitmap_buffer)
	    {
            return -1;
	    }
    }
#else
    font_buffer      = (UINT8 *)MALLOC(FONT_BMP_SIZE);

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == font_buffer)
    {
        return -1;
    }

    mask_data_buffer = (UINT8 *)MALLOC(FONT_BMP_SIZE);

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == mask_data_buffer)
    {
        FREE(font_buffer);
        return -1;
    }
   
    font_warp = (UINT8 *)MALLOC(FONT_BMP_SIZE);

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == font_warp)
    {
        FREE(font_buffer);
        FREE(mask_data_buffer);
        return -1;
    }
    
//此处一开始就malloc一块内存，且中途不释放，这样对内存会
//造成浪费，故在这屏蔽，等到需要用到bitmap_buffer时再临时分配
    #if 1
    bitmap_buffer    = (UINT8 *)MALLOC(BMP_BUFFER);

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == bitmap_buffer)
    {
        FREE(font_buffer);
        FREE(mask_data_buffer);
        FREE(font_warp);        
        return -1;
    }
    #endif
#endif
    return 0;
}


static struct ge_device *_lib_get_ge_device(void)
{
    return g_lib_ge_dev;
}

static UINT32 _lib_ge_get_n_bit(const UINT8 *buf, UINT32 pos, UINT32 n)
{
    UINT8  clu = 0;
    UINT32 ret = 0;

    if (NULL == buf)
    {
        return 0;
    }
    switch (n)
    {
        case PIXEL4BIT:
            clu = *(buf + (pos >> 1));
            if (0 == (pos & 1))
            {
                clu >>= 4;
            }
            ret =  clu & 0x0f;
            break;
        case PIXEL1BIT:
            clu = *(buf + (pos >> 2));
            clu = clu >> (7 - (pos & 0x07));
            ret =  clu & 0x01;
            break;
        case PIXEL2BIT:
            clu = *(buf + (pos >> 2));
            clu = clu >> ((3 - (pos & 0x03)) * 2);
            ret =  clu & 0x03;
            break;
        case PIXEL8BIT:
            ret = buf[pos];
            break;
        case PIXEL16BIT:
            ret = *(UINT16 *)(buf + (pos << 1));
            break;
        case PIXEL32BIT:
            ret = *(UINT32 *)(buf + (pos << 2));
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}


static BOOL lib_ge_get_draw_state(void)
{
    return b_start_draw;
}

void lib_ge_draw_start(void)
{
    b_start_draw = TRUE;
    ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
}



void lib_ge_draw_end(void)
{
    ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
    ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    b_start_draw = FALSE;
}


static enum GE_PIXEL_FORMAT ge_draw_color_mode_to_ge(UINT8 b_color_mode)
{
    enum GE_PIXEL_FORMAT color_format = GE_PF_CLUT8;

    switch (b_color_mode)
    {
        case OSD_256_COLOR:
            color_format = GE_PF_CLUT8;
            break;
        case OSD_HD_ARGB1555:
            color_format = GE_PF_ARGB1555;
            break;
        case OSD_HD_ARGB4444:
            color_format = GE_PF_ARGB4444;
            break;
        case osd_hd_aycb_cr8888:
        case OSD_HD_ARGB8888:
            color_format = GE_PF_ARGB8888;
            break;
        default:
            break;
    }

    return color_format;
}

static UINT32 ge_draw_get_pitch_by_ge_format(UINT8 ge_pixel_fmt, UINT16 u_width)
{
    switch (ge_pixel_fmt)
    {
        case GE_PF_ARGB1555:
        case GE_PF_ARGB4444:
            u_width <<= 1;
            break;
        case GE_PF_RGB888:
        case GE_PF_ARGB8888:
            u_width <<= 2;
            break;
        case GE_PF_CLUT8:
        case GE_PF_CK_CLUT8:
        default:
            break;
    }

    return u_width;
}

static void ge_set_ptn_addr_pitch(UINT32 cmd_pos, UINT32 addr, UINT32 format_and_pitch, enum GE_DATA_DECODER decoder)
{
    const UINT8         u_shift_bits  = 16;
    enum GE_PIXEL_FORMAT format     = GE_PF_ARGB8888;
    ge_base_addr_t      base_addr_dst;

    if (format_and_pitch >> u_shift_bits)
    {
        format = (enum GE_PIXEL_FORMAT)(format_and_pitch >> u_shift_bits);
    }
    MEMSET(&base_addr_dst,0x00,sizeof(ge_base_addr_t));
    base_addr_dst.base_address = addr;
    base_addr_dst.pixel_pitch = (UINT16)format_and_pitch;
    base_addr_dst.data_decoder = decoder;
    base_addr_dst.color_format = format;
    base_addr_dst.modify_flags = GE_BA_FLAG_ADDR | GE_BA_FLAG_PITCH | GE_BA_FLAG_FORMAT;

    ge_set_base_addr(LIB_GE_HANDLE, cmd_pos, GE_PTN, &base_addr_dst);
}

static UINT8 *lib_ge_decode_bitmap(UINT8 *dest_buf, sgt_raw_bitmap *bmp_res, BOOL is_rle)
{
    UINT32          cmd_hdl         = 0;
    ge_base_addr_t  src_base_addr;

    if((NULL == dest_buf)||(NULL == bmp_res))
    {
        return NULL;
    }
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    src_base_addr.color_format = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(bmp_res->color_mode);
    src_base_addr.base_address = (UINT32)dest_buf;
    src_base_addr.data_decoder = GE_DECODER_DISABLE;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.pixel_pitch = (bmp_res->bitmap_size / bmp_res->height) / (bmp_res->bps / 8);
    src_base_addr.modify_flags = GE_BA_FLAG_ALL;
    //osal_cache_flush(dest_buf->dest_buf, byte_pitch * rect->uHeight);
    osal_cache_invalidate(dest_buf, bmp_res->bitmap_size);

    if (is_rle)
    {
        if (!lib_ge_get_draw_state())
        {
            ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
        }
        cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);

        ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
        ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);
        ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &src_base_addr);


        ge_set_ptn_addr_pitch(cmd_hdl, (UINT32)(bmp_res->data),
                              (bmp_res->bitmap_size / bmp_res->height) / (bmp_res->bps / 8) |
                              (src_base_addr.color_format << 16), GE_DECODER_RLE);

        ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, 0, 0);
        ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, src_base_addr.pixel_pitch,  bmp_res->height);

        ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);


        if (!lib_ge_get_draw_state())
        {
            ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
            ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
        }

    }
    return dest_buf;
}

static UINT8 *lib_ge_copy_bitmap(sgt_raw_bitmap *src_res, sgt_raw_bitmap *dst_res,
    ge_rect_t *src_rect, ge_rect_t *dst_rect)
{
    UINT32      cmd_hdl     = 0;
    UINT32      stride      = 0;
    UINT32         src_stride  = 0;
    INT32       width       = 0;
    INT32       height      = 0;
    INT32       top         = 0;
    INT32       bottom      = 0;
    INT32       left        = 0;
    INT32       right       = 0;
    INT32       off_x       = 0;
    INT32       off_y       = 0;
    enum GE_PIXEL_FORMAT  ge_pixel = GE_PF_ARGB1555;
    enum GE_PIXEL_FORMAT  src_ge_pixel = GE_PF_ARGB1555;
    ge_rect_t   rect_set;
    ge_base_addr_t src_base_addr;
    ge_base_addr_t dst_base_addr;

    //cmd_base = ge_cmd_list_get_base(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);

    if ((NULL == src_res) || (NULL == dst_res) || (NULL== src_rect) || (NULL == dst_rect))
    {
        return NULL;
    }
    MEMSET(&rect_set,0x00,sizeof(ge_rect_t));
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }
    top     = dst_rect->top;
    bottom  = dst_rect->top  + dst_rect->height;
    left    = dst_rect->left;
    right   = dst_rect->left + dst_rect->width;

    width = src_rect->width;
    height  = src_rect->height;

    src_base_addr.base_address  = (UINT32)src_res->data;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.color_format  =(enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    src_base_addr.data_decoder  = GE_DECODER_DISABLE;
    src_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    src_base_addr.pixel_pitch   = src_res->width;
    ge_pixel = ge_draw_color_mode_to_ge(dst_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, dst_res->width);
    src_ge_pixel = ge_draw_color_mode_to_ge(src_res->color_mode);
    src_stride   = ge_draw_get_pitch_by_ge_format(src_ge_pixel,src_res->width);
    osal_cache_flush(dst_res->data, stride * dst_res->height);
    osal_cache_invalidate(dst_res->data, stride * dst_res->height);
    osal_cache_flush(src_res->data,src_stride*src_res->height);   //20131118 add for 32 bit UI
    osal_cache_invalidate(src_res->data,src_stride*src_res->height);

    off_y  = src_rect->top;

    for (top = dst_rect->top; top < bottom; top += height)
    {
        if ((top + height) > bottom)
        {
            if (top != dst_rect->top)
            {
                top = bottom - height;
            }
            else
            {
                height = bottom - top;
            }
        }
        rect_set.top    = top;
        rect_set.height = height;

        off_x = src_rect->left;
        for (left = dst_rect->left; left < right; left += width)
        {
            if ((left + width) > right)
            {
                if (left != dst_rect->left)
                {
                    left = right - width;
                }
                else
                {
                    width = right - left;
                }
            }
            rect_set.left  = left;
            rect_set.width = width;
            cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);

            dst_base_addr.base_address = (UINT32)(dst_res->data);
            dst_base_addr.data_decoder = GE_DECODER_DISABLE;
            dst_base_addr.base_addr_sel = GE_BASE_ADDR1;
            dst_base_addr.pixel_pitch =  dst_res->width;
            dst_base_addr.modify_flags = GE_BA_FLAG_ALL;
            dst_base_addr.color_format =(enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(dst_res->color_mode);

            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &src_base_addr);

            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, width, height);
            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, width, height);

            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, rect_set.left, rect_set.top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, off_x, off_y);
            ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
            off_x += width;

        }
        off_y += height;
    }

    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }
    return (UINT8 *)dst_res;
}

static void lib_ge_copy_bitmap_key_color(sgt_raw_bitmap *src_res, sgt_raw_bitmap *dst_res,
    ge_rect_t *rect, INT32 pixel_pitch, UINT32 bg_color)
{
    UINT32      cmd_hdl     = 0;
    UINT32      stride      = 0;
    UINT32         src_stride  = 0;
    INT32       width       = 0;
    INT32       height      = 0;
    INT32       top         = 0;
    INT32       bottom      = 0;
    INT32       left        = 0;
    INT32       right       = 0;
    enum GE_PIXEL_FORMAT  ge_pixel = GE_PF_ARGB1555;
    enum GE_PIXEL_FORMAT  src_ge_pixel = GE_PF_ARGB1555;
    ge_rect_t   rect_set;
    ge_base_addr_t src_base_addr;
    ge_base_addr_t dst_base_addr;

    if((NULL == src_res) || (NULL == dst_res) || (NULL == rect))
    {
        return ;
    }
    MEMSET(&rect_set,0x00,sizeof(ge_rect_t));
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }
    top     = rect->top;
    bottom  = rect->top  + rect->height;
    left    = rect->left;
    right   = rect->left + rect->width;
    width = src_res->width;
    height  = src_res->height;
    src_base_addr.base_address  = (UINT32)src_res->data;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.color_format  =(enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    src_base_addr.data_decoder  = GE_DECODER_DISABLE;
    src_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    src_base_addr.pixel_pitch   =((src_res->bitmap_size / src_res->height) / (src_res->bps / 8));

    dst_base_addr.base_address = (UINT32)(dst_res->data);
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR1;

    if (pixel_pitch != 0)
    {
        dst_base_addr.pixel_pitch = pixel_pitch;
    }
    else
    {
        dst_base_addr.pixel_pitch = dst_res->width;
    }
    dst_base_addr.modify_flags = GE_BA_FLAG_ALL;
    dst_base_addr.color_format =(enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(dst_res->color_mode);

    ge_pixel = ge_draw_color_mode_to_ge(dst_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, dst_res->width);
    src_ge_pixel = ge_draw_color_mode_to_ge(src_res->color_mode);
    src_stride   = ge_draw_get_pitch_by_ge_format(src_ge_pixel,src_res->width);


    osal_cache_flush(dst_res->data, stride * dst_res->height);
    osal_cache_invalidate(dst_res->data, stride * dst_res->height);
    osal_cache_flush(src_res->data,src_stride*src_res->height);    //20131118 add for 32 bit UI
    osal_cache_invalidate(src_res->data,src_stride*src_res->height);

    for (top = rect->top; top < bottom; top += height)
    {
        if ((top + height) > bottom)
        {
            if (top != rect->top)
            {
                top = bottom - height;
            }
            else
            {
                height = bottom - top;
            }
        }
        rect_set.top    = top;
        rect_set.height = height;

        for (left = rect->left; left < right; left += width)
        {
            if ((left + width) > right)
            {
                if (left != rect->left)
                {
                    left = right - width;
                }
                else
                {
                    width = right - left;
                }
            }
            rect_set.left  = left;
            rect_set.width = width;
            //cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,GE_PRIM_DISABLE);
            cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);

            dst_base_addr.base_addr_sel = GE_BASE_ADDR2;
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &dst_base_addr);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &src_base_addr);

            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);

            ge_set_colorkey_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_PTN_POST_CLUT);
            ge_set_rgb_expansion(LIB_GE_HANDLE,cmd_hdl,GE_PTN,GE_RGB_EXPAN_PAD0_TO_LSB);
            //ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_R, GE_CKEY_MATCH_IN_RANGE);
            //ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_G, GE_CKEY_MATCH_IN_RANGE);
            //ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_B, GE_CKEY_MATCH_IN_RANGE);

            ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_R, GE_CKEY_ALWAYS_MATCH);
            ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_G, GE_CKEY_ALWAYS_MATCH);
            ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_B, GE_CKEY_ALWAYS_MATCH);

            ge_set_colorkey_match_mode(LIB_GE_HANDLE, cmd_hdl, GE_CKEY_CHANNEL_A, GE_CKEY_MATCH_IN_RANGE);
            ge_set_colorkey_range(LIB_GE_HANDLE, cmd_hdl,trans_color_low,trans_color_high);

            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, width, height);
            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, width, height);

            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, left, top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 0, 0);
            ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

        }

    }
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }
}


static void lib_ge_copy_bitmap_alpha_blending(sgt_raw_bitmap *src_res, sgt_raw_bitmap *dst_res,
    ge_rect_t *rect, UINT32 pixel_pitch, UINT32 bg_color)
{
    UINT32      cmd_hdl     = 0;
    UINT32      stride      = 0;
    INT32       width       = 0;
    INT32       height      = 0;
    INT32       top         = 0;
    INT32       bottom      = 0;
    INT32       left        = 0;
    INT32       right       = 0;
    enum GE_PIXEL_FORMAT  ge_pixel = GE_PF_ARGB1555;
    ge_rect_t rect_set;
    ge_base_addr_t src_base_addr;
    ge_base_addr_t dst_base_addr;
    ge_base_addr_t bln_base_addr;

    if((NULL == src_res) || (NULL == dst_res) || (NULL == rect))
    {
        return ;
    }
    //cmd_base = ge_cmd_list_get_base(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
    MEMSET(&rect_set,0x00,sizeof(ge_rect_t));
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&bln_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }

    top     = rect->top;
    bottom  = rect->top  + rect->height;
    left    = rect->left;
    right   = rect->left + rect->width;
    width = src_res->width;
    height  = src_res->height;
    src_base_addr.base_address  = (UINT32)src_res->data;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.color_format  = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    src_base_addr.data_decoder  = GE_DECODER_DISABLE;
    src_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    src_base_addr.pixel_pitch   =((src_res->bitmap_size / src_res->height) / (src_res->bps / 8));

    dst_base_addr.base_address = (UINT32)(dst_res->data);
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR1;
    if (pixel_pitch != 0)
    {
        dst_base_addr.pixel_pitch = pixel_pitch;
    }
    else
    {
        dst_base_addr.pixel_pitch = dst_res->width;
    }

    dst_base_addr.modify_flags = GE_BA_FLAG_ALL;
    dst_base_addr.color_format = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(dst_res->color_mode);

    //ge_set_base_addr(LIB_GE_HANDLE, cmd_base, GE_DST, &dst_base_addr);

    //dst_base_addr.base_addr_sel = GE_BASE_ADDR2;
    //ge_set_base_addr(LIB_GE_HANDLE, cmd_base, GE_SRC, &dst_base_addr);

    //ge_set_base_addr(LIB_GE_HANDLE,cmd_base,GE_PTN,&src_base_addr);

    //ge_set_rgb_order(LIB_GE_HANDLE, cmd_base, GE_DST, GE_RGB_ORDER_ARGB);
    //ge_set_rgb_order(LIB_GE_HANDLE, cmd_base, GE_SRC, GE_RGB_ORDER_ARGB);
    //ge_set_rgb_order(LIB_GE_HANDLE, cmd_base, GE_PTN, GE_RGB_ORDER_ARGB);


    ge_pixel = ge_draw_color_mode_to_ge(dst_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, dst_res->width);



    osal_cache_flush(dst_res->data, stride * dst_res->height);
    osal_cache_invalidate(dst_res->data, stride * dst_res->height);

    ge_pixel = ge_draw_color_mode_to_ge(src_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, src_res->width);
    //ckey_buffer = MALLOC(stride*src_res->height);
    osal_cache_flush(src_res->data,stride*src_res->height);    //20131118 add for 32 bit UI
    osal_cache_invalidate(src_res->data,stride*src_res->height);
    //osal_cache_invalidate(ckey_buffer,stride*src_res->height);

    //MEMSET(ckey_buffer,00,stride*src_res->height);
    //osal_cache_flush(ckey_buffer,stride*src_res->height);
    bln_base_addr.color_format  = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    bln_base_addr.base_addr_sel = GE_BASE_ADDR2;
    bln_base_addr.data_decoder  = GE_DECODER_DISABLE;
    bln_base_addr.base_address  = (UINT32)bitmap_buffer;
    bln_base_addr.pixel_pitch   = src_res->width;
    bln_base_addr.modify_flags  = GE_BA_FLAG_ALL;



    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_PRIM_DISABLE);
    ge_set_src_mode(LIB_GE_HANDLE, cmd_hdl, GE_SRC_DIRECT_FILL);
    ge_set_rgb_expansion(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_EXPAN_PAD0_TO_LSB);
    ge_set_color_format(LIB_GE_HANDLE, cmd_hdl, bln_base_addr.color_format);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &bln_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &bln_base_addr);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
    ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, (bg_color  & 0x7FFFFFF));
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, 0, 0);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, src_res->width, src_res->height);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

#if 1
    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,
                           GE_DRAW_BITMAP_ALPHA_BLENDING);

    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &bln_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &bln_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &src_base_addr);

    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);

    ge_set_global_alpha(LIB_GE_HANDLE, cmd_hdl, 0xFF);
    //ge_set_global_alpha_mode(gelib_dev,cmd_hdl,0);
    ge_set_global_alpha_sel(LIB_GE_HANDLE, cmd_hdl, GE_USE_GALPHA);
    //ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, bg_color & 0x7FFFFFF);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, 00, 00);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 00, 00);

    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, src_res->width, src_res->height);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, src_res->width, src_res->height);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
#endif


#if 1
    for (top = rect->top; top < bottom; top += height)
    {
        if ((top + height) > bottom)
        {
            if (top != rect->top)
            {
                top = bottom - height;
            }
            else
            {
                height = bottom - top;
            }
        }
        rect_set.top    = top;
        rect_set.height = height;

        for (left = rect->left; left < right; left += width)
        {
            if ((left + width) > right)
            {
                if (left != rect->left)
                {
                    left = right - width;
                }
                else
                {
                    width = right - left;
                }
            }
            rect_set.left  = left;
            rect_set.width = width;
            //cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE,g_ge_osd_cmd_hdl,
            //                      GE_PRIM_DISABLE);
            cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &bln_base_addr);

            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);


            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, width, height);
            //ge_set_xy(LIB_GE_HANDLE,cmd_hdl,GE_DST,left - dst_res->left,top - dst_res->top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, left, top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 0, 0);
            ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

        }

    }
#endif
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }

}

static void lib_ge_copy_bitmap_alpha_blending_no_bg(sgt_raw_bitmap *src_res, sgt_raw_bitmap *dst_res,
    ge_rect_t *rect, UINT32 pixel_pitch)
{
    UINT32      cmd_hdl     = 0;
    UINT32      stride      = 0;
    INT32       width       = 0;
    INT32       height      = 0;
    INT32       top         = 0;
    INT32       bottom      = 0;
    INT32       left        = 0;
    INT32       right       = 0;
    enum GE_PIXEL_FORMAT  ge_pixel = GE_PF_ARGB1555;
    ge_rect_t rect_set;
    ge_base_addr_t src_base_addr;
    ge_base_addr_t dst_base_addr;
    ge_base_addr_t bln_base_addr;

    if((NULL == src_res) || (NULL == dst_res) || (NULL == rect))
    {
        return ;
    }
    //cmd_base = ge_cmd_list_get_base(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
    MEMSET(&rect_set,0x00,sizeof(ge_rect_t));
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&bln_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }

    top     = rect->top;
    bottom  = rect->top  + rect->height;
    left    = rect->left;
    right   = rect->left + rect->width;
    width = src_res->width;
    height  = src_res->height;

    bln_base_addr.base_address  = (UINT32)src_res->data;
    bln_base_addr.base_addr_sel = GE_BASE_ADDR;
    bln_base_addr.color_format  = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    bln_base_addr.data_decoder  = GE_DECODER_DISABLE;
    bln_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    bln_base_addr.pixel_pitch   =((src_res->bitmap_size / src_res->height) / (src_res->bps / 8));

    
    //src_base_addr.base_address  = (UINT32)src_res->data;
    //src_base_addr.base_addr_sel = GE_BASE_ADDR;
    //src_base_addr.color_format  = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    //src_base_addr.data_decoder  = GE_DECODER_DISABLE;
    //src_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    //src_base_addr.pixel_pitch   =((src_res->bitmap_size / src_res->height) / (src_res->bps / 8));

    dst_base_addr.base_address = (UINT32)(dst_res->data);
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR1;
    if (pixel_pitch != 0)
    {
        dst_base_addr.pixel_pitch = pixel_pitch;
    }
    else
    {
        dst_base_addr.pixel_pitch = dst_res->width;
    }

    dst_base_addr.modify_flags = GE_BA_FLAG_ALL;
    dst_base_addr.color_format = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(dst_res->color_mode);

    ge_pixel = ge_draw_color_mode_to_ge(dst_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, dst_res->width);
    //osal_cache_flush(dst_res->data, stride * dst_res->height);
    osal_cache_invalidate(dst_res->data, stride * dst_res->height);

    ge_pixel = ge_draw_color_mode_to_ge(src_res->color_mode);
    stride   = ge_draw_get_pitch_by_ge_format(ge_pixel, src_res->width);
    //ckey_buffer = MALLOC(stride*src_res->height);
    osal_cache_flush(src_res->data,stride*src_res->height);    //20131118 add for 32 bit UI
    osal_cache_invalidate(src_res->data,stride*src_res->height);
    
    //bln_base_addr.color_format  = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(src_res->color_mode);
    //bln_base_addr.base_addr_sel = GE_BASE_ADDR2;
    //bln_base_addr.data_decoder  = GE_DECODER_DISABLE;
    //bln_base_addr.base_address  = (UINT32)bitmap_buffer;
    //bln_base_addr.pixel_pitch   = src_res->width;
    //bln_base_addr.modify_flags  = GE_BA_FLAG_ALL;



    //cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_PRIM_DISABLE);
    //ge_set_src_mode(LIB_GE_HANDLE, cmd_hdl, GE_SRC_DIRECT_FILL);
    //ge_set_rgb_expansion(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_EXPAN_PAD0_TO_LSB);
    //ge_set_color_format(LIB_GE_HANDLE, cmd_hdl, bln_base_addr.color_format);
    //ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &bln_base_addr);
    //ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &bln_base_addr);
    //ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    //ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
    //ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, (bg_color  & 0x7FFFFFF));
    //ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, 0, 0);
    //ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, src_res->width, src_res->height);
    //ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
    
#if 0
    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,
                           GE_DRAW_BITMAP_ALPHA_BLENDING);

    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &bln_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &bln_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &src_base_addr);

    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);

    ge_set_global_alpha(LIB_GE_HANDLE, cmd_hdl, 0xFF);
    //ge_set_global_alpha_mode(gelib_dev,cmd_hdl,0);
    ge_set_global_alpha_sel(LIB_GE_HANDLE, cmd_hdl, GE_USE_GALPHA);
    //ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, bg_color & 0x7FFFFFF);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, 00, 00);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 00, 00);

    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, src_res->width, src_res->height);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, src_res->width, src_res->height);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
#endif


#if 1
    for (top = rect->top; top < bottom; top += height)
    {
        if ((top + height) > bottom)
        {
            if (top != rect->top)
            {
                top = bottom - height;
            }
            else
            {
                height = bottom - top;
            }
        }
        rect_set.top    = top;
        rect_set.height = height;

        for (left = rect->left; left < right; left += width)
        {
            if ((left + width) > right)
            {
                if (left != rect->left)
                {
                    left = right - width;
                }
                else
                {
                    width = right - left;
                }
            }
            rect_set.left  = left;
            rect_set.width = width;
#if 0
            cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &bln_base_addr);

            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);


            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, width, height);
            //ge_set_xy(LIB_GE_HANDLE,cmd_hdl,GE_DST,left - dst_res->left,top - dst_res->top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, left, top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 0, 0);
            ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
#else
            cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,
                                   GE_DRAW_BITMAP_ALPHA_BLENDING);

            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &dst_base_addr);
            ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &bln_base_addr);

            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
            ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);

            ge_set_global_alpha(LIB_GE_HANDLE, cmd_hdl, 0xFF);
            //ge_set_global_alpha_mode(gelib_dev,cmd_hdl,0);
            ge_set_global_alpha_sel(LIB_GE_HANDLE, cmd_hdl, GE_USE_GALPHA);
            //ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, bg_color & 0x7FFFFFF);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, left, top);
            ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 00, 00);

            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, width,  height);
            ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, width, height);
            ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

#endif


        }

    }
#endif
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }

}



static void lib_ge_draw_line(UINT8   *buffer , INT32 left, INT32 width,
                             UINT32 draw_color, UINT32 pixel_pitch, UINT8 color_mode)
{
    enum GE_PIXEL_FORMAT __MAYBE_UNUSED__ color_format = GE_PF_CLUT8;

    if(NULL == buffer)
    {
        return ;
    }
    color_format = (enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(color_mode);
}

static void lib_ge_draw_windows(sgt_raw_bitmap *dst_res, ge_rect_t *rect,
    UINT32 pixel_pitch, UINT32 draw_color, UINT32 frame_color)
{
    UINT32      stride      = 0;
    UINT32      cmd_hdl     = 0;
    INT32       __MAYBE_UNUSED__ offset_top  = 0;
    INT32       __MAYBE_UNUSED__ offset_left = 0;
    ge_base_addr_t dst_base_addr;

    if((NULL == dst_res) || (NULL == rect))
    {
        return ;
    }
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }

    dst_base_addr.base_address   = (UINT32)dst_res->data;
    dst_base_addr.base_addr_sel  = GE_BASE_ADDR;
    dst_base_addr.color_format   =(enum GE_PIXEL_FORMAT)ge_draw_color_mode_to_ge(dst_res->color_mode);
    dst_base_addr.data_decoder   = GE_DECODER_DISABLE;


    if (pixel_pitch != 0)
    {
        dst_base_addr.pixel_pitch    = pixel_pitch;
    }
    else
    {
        dst_base_addr.pixel_pitch    = dst_res->width;
    }

    stride = ge_draw_get_pitch_by_ge_format(dst_base_addr.color_format, rect->width);

    offset_top  = rect->top  - dst_res->top;
    offset_left = rect->left  - dst_res->left;

    if(GE_PF_ARGB1555 == ge_draw_color_mode_to_ge(dst_res->color_mode)
        || GE_PF_ARGB4444 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
    //osal_cache_flush(dst_res->data+(stride *offset_top),stride*rect->height);
    //osal_cache_invalidate(dst_res->data + (stride * offset_top), stride*rect->height);
    osal_cache_flush(dst_res->data, stride * rect->height);
    osal_cache_invalidate(dst_res->data, stride * rect->height);
    }
    else  //if(GE_PF_ARGB8888 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
        osal_cache_flush(dst_res->data,stride*dst_res->height);
        osal_cache_invalidate(dst_res->data,stride*dst_res->height);
    }
    dst_base_addr.modify_flags   = GE_BA_FLAG_ALL;

    //cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,GE_DRAW_RECT_FRAME_FILL);
    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_FILL_RECT_DRAW_COLOR);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_color_format(LIB_GE_HANDLE, cmd_hdl, dst_base_addr.color_format);
    ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, frame_color); // red
    ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, draw_color);
    //ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, offset_left, offset_top);

    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, rect->left, rect->top);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, rect->width, rect->height);

    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }

}


static void lib_ge_draw_4bit_font(sgt_raw_bitmap *dst_res, ge_rect_t *rect, UINT8 *font_data,
    INT32 pixel_pitch, UINT32 draw_color, UINT32 back_color, UINT8 bps)
{
    enum GE_PIXEL_FORMAT __MAYBE_UNUSED__ color_format = GE_PF_CLUT8;
    UINT32          stride      = 0;
    UINT32          cmd_hdl     = 0;
    UINT32          height      = 0;
    ge_base_addr_t  ptn_base_addr;
    ge_base_addr_t  dst_base_addr;
    ge_base_addr_t  msk_base_addr;
    ge_base_addr_t  src_base_addr;
    ge_rect_t       rect_font;

    if (bps != const_pixel_4_bits)
    {
        return;
    }
    if ((NULL == dst_res) || (NULL== rect) || (NULL == font_data))
    {
        ASSERT(0);
        return;
    }
    MEMSET(&ptn_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&msk_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&src_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&rect_font,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }
    //2 generate pallete
    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,
                           GE_FILL_RECT_DRAW_COLOR);
    dst_base_addr.base_address  = (UINT32)font_pal;
    dst_base_addr.pixel_pitch   = 1 << bps;
    dst_base_addr.color_format  = GE_PF_ARGB8888;
    dst_base_addr.data_decoder  = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.modify_flags  = GE_BA_FLAG_ALL;
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_color_format(LIB_GE_HANDLE, cmd_hdl, dst_base_addr.color_format);
    if(GE_PF_ARGB1555 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
    ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, ARGB1555_2_8888(draw_color));
    ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, ARGB1555_2_8888(draw_color));
    }
    else if(GE_PF_ARGB4444 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
        ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, ARGB4444_2_8888(draw_color));
        ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, ARGB4444_2_8888(draw_color));
    }
    else   //20131118 add for 32 bit UI
    {
        ge_set_draw_color(LIB_GE_HANDLE, cmd_hdl, draw_color);
        ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, draw_color);
    }
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST, 0, 0);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, dst_base_addr.pixel_pitch, 1);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    rect_font.top       = 0;
    rect_font.height    = 0;
    rect_font.width     = rect->width;
    rect_font.height    = rect->height;
    osal_cache_invalidate(font_warp, FONT_BMP_SIZE);
    color_format = GE_PF_CLUT4;

    //2 byte swap of big-endian.
    src_base_addr.base_address  = (UINT32)font_data;
    src_base_addr.pixel_pitch   = 1;
    src_base_addr.color_format  = GE_PF_ARGB8888;
    src_base_addr.data_decoder  = GE_DECODER_DISABLE;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    dst_base_addr.base_address  = (UINT32)font_buffer;
    dst_base_addr.pixel_pitch   = 1;
    dst_base_addr.color_format  = GE_PF_ARGB8888;
    dst_base_addr.data_decoder  = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR1;
    dst_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_PRIM_DISABLE);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &src_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_byte_endian(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_BYTE_ENDIAN_BIG);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, 00, 00);
    height = (rect->width * rect->height * bps + 31) >> 5;
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, 1, height);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    //2    transform clut4 to     argb1555
    ptn_base_addr.base_address  = (UINT32)font_buffer;
    ptn_base_addr.pixel_pitch   =  rect->width * 2;
    ptn_base_addr.color_format  = GE_PF_CLUT4;
    ptn_base_addr.data_decoder  = GE_DECODER_DISABLE;
    ptn_base_addr.base_addr_sel = GE_BASE_ADDR1;
    ptn_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    dst_base_addr.base_address  = (UINT32)font_warp;
    dst_base_addr.pixel_pitch   = rect->width * 2;
    dst_base_addr.color_format  = ge_draw_color_mode_to_ge(dst_res->color_mode);
    dst_base_addr.data_decoder  = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &ptn_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_clut_addr(LIB_GE_HANDLE, cmd_hdl, (UINT32)(&font_pal[0]));
    ge_set_clut_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_RGB_ORDER_ARGB);
    ge_set_clut_mode(LIB_GE_HANDLE, cmd_hdl, GE_CLUT_COLOR_EXPANSION, FALSE);
    ge_set_clut_update(LIB_GE_HANDLE, cmd_hdl, TRUE);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, 00, 00);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, ptn_base_addr.pixel_pitch , (rect->height) / 2);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, ptn_base_addr.pixel_pitch , (rect->height) / 2);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    //2 generate mask A8
    ptn_base_addr.base_address  = (UINT32)font_buffer;
    ptn_base_addr.pixel_pitch   =  rect->width * 2;
    ptn_base_addr.color_format  = GE_PF_CLUT4;
    ptn_base_addr.data_decoder  = GE_DECODER_DISABLE;
    ptn_base_addr.base_addr_sel = GE_BASE_ADDR1;
    ptn_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    dst_base_addr.base_address  = (UINT32)mask_data_buffer;
    dst_base_addr.pixel_pitch   = rect->width * 4;
    dst_base_addr.color_format  = GE_PF_CLUT4;
    dst_base_addr.data_decoder  = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_BITMAP);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &ptn_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_font_stretch(LIB_GE_HANDLE, cmd_hdl, 1, 0);
    ge_set_subbyte_endian(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_SUBBYTE_RIGHT_PIXEL_LSB);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, 00, 00);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, ptn_base_addr.pixel_pitch ,(rect->height) / 2);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST, ptn_base_addr.pixel_pitch * 2 , (rect->height) / 2);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    ptn_base_addr.base_address  = (UINT32)font_warp;
    ptn_base_addr.pixel_pitch   = rect->width;
    ptn_base_addr.color_format  = ge_draw_color_mode_to_ge(dst_res->color_mode);
    ptn_base_addr.data_decoder  = GE_DECODER_DISABLE;
    ptn_base_addr.base_addr_sel = GE_BASE_ADDR1;
    ptn_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    dst_base_addr.base_address  = (UINT32)dst_res->data;
    dst_base_addr.pixel_pitch   = rect->width;
    dst_base_addr.color_format  = ge_draw_color_mode_to_ge(dst_res->color_mode);
    dst_base_addr.data_decoder  = GE_DECODER_DISABLE;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    msk_base_addr.base_address  = (UINT32)mask_data_buffer;
    msk_base_addr.pixel_pitch   = rect->width;
    msk_base_addr.color_format  = GE_PF_MASK_A8;
    msk_base_addr.data_decoder  = GE_DECODER_DISABLE;
    msk_base_addr.base_addr_sel = GE_BASE_ADDR4;
    msk_base_addr.modify_flags  = GE_BA_FLAG_ALL;

    if (pixel_pitch != 0)
    {
        dst_base_addr.pixel_pitch = pixel_pitch;
    }
    else
    {
        dst_base_addr.pixel_pitch = rect->width;
    }


    stride = ge_draw_get_pitch_by_ge_format(dst_base_addr.color_format,dst_base_addr.pixel_pitch);


    osal_cache_invalidate(dst_res->data + stride * rect->top, stride * rect->height);

    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,
                           GE_DRAW_BITMAP_ALPHA_BLENDING);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &ptn_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    dst_base_addr.base_addr_sel = GE_BASE_ADDR3;
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_SRC, &dst_base_addr);
    ge_set_base_addr(LIB_GE_HANDLE, cmd_hdl, GE_MSK, &msk_base_addr);
    ge_set_msk_mode(LIB_GE_HANDLE, cmd_hdl, GE_MSK_ENABLE);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_MSK, 0, 0);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_MSK, rect->width, rect->height);

    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, rect->left, rect->top);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, rect->width, rect->height);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 0, 0);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_PTN, rect->width, rect->height);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_SRC, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);

    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }
    return ;
}

static void lib_ge_draw_1bit_font(sgt_raw_bitmap *dst_res, ge_rect_t *rect, UINT8 *font_data,
                                  INT32 pixel_pitch, UINT32 draw_color, UINT32 back_color, INT8 bps)
{
    UINT32          stride      = 0;
    UINT32          cmd_hdl     = 0;
    ge_base_addr_t  ptn_base_addr;
    ge_base_addr_t  dst_base_addr;

    if (bps > 1)
    {
        return;
    }
    if ((NULL == dst_res) || (NULL == rect) || (NULL == font_data))
    {
        ASSERT(0);
        return;
    }
    MEMSET(&ptn_base_addr,0x00,sizeof(ge_base_addr_t));
    MEMSET(&dst_base_addr,0x00,sizeof(ge_base_addr_t));
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_new(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_COMPILE);
    }
    ///////

    dst_base_addr.color_format = ge_draw_color_mode_to_ge(dst_res->color_mode);
    dst_base_addr.base_address = (UINT32)dst_res->data;
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.pixel_pitch = pixel_pitch;
    dst_base_addr.modify_flags = GE_BA_FLAG_ALL;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR1;

    ptn_base_addr.color_format = ge_draw_color_mode_to_ge(dst_res->color_mode);
    ptn_base_addr.base_address = (UINT32)font_data;
    ptn_base_addr.data_decoder = GE_DECODER_DISABLE;
    ptn_base_addr.pixel_pitch = rect->width;
    ptn_base_addr.modify_flags = GE_BA_FLAG_ALL;
    ptn_base_addr.base_addr_sel = GE_BASE_ADDR2;

    stride = ge_draw_get_pitch_by_ge_format(dst_base_addr.color_format,dst_base_addr.pixel_pitch);
    osal_cache_invalidate(dst_res->data + stride * rect->top, stride * rect->height);

    //if (back_color & C_MIX_BG)
    //{
    //    back_color &= ~0x40000000;
    //    cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl,GE_DRAW_FONT_FILL_BACK_COLOR);
    //}
    //else
    {
        cmd_hdl = ge_cmd_begin(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, GE_DRAW_FONT);
    }


    ge_set_operating_entity(LIB_GE_HANDLE, cmd_hdl, GE_DST, &dst_base_addr);
    ge_set_operating_entity(LIB_GE_HANDLE, cmd_hdl, GE_PTN, &ptn_base_addr);
    ge_set_subbyte_endian(LIB_GE_HANDLE, cmd_hdl, GE_PTN,GE_SUBBYTE_RIGHT_PIXEL_LSB);

    if(GE_PF_ARGB1555 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
    ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, ARGB1555_2_8888(back_color));
    ge_set_font_color(LIB_GE_HANDLE, cmd_hdl, ARGB1555_2_8888(draw_color));
    }
    else if(GE_PF_ARGB4444 == ge_draw_color_mode_to_ge(dst_res->color_mode))
    {
        ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, ARGB4444_2_8888(back_color));
        ge_set_font_color(LIB_GE_HANDLE,cmd_hdl,ARGB4444_2_8888(draw_color));
    }
    else  //20131118 add for 32 bit UI
    {
        ge_set_back_color(LIB_GE_HANDLE, cmd_hdl, back_color);
        ge_set_font_color(LIB_GE_HANDLE,cmd_hdl,draw_color);
    }
    ge_set_font_data_format(LIB_GE_HANDLE, cmd_hdl, GE_FONT_DATA_DWORD);
    ge_set_byte_endian(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_BYTE_ENDIAN_BIG);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_PTN, 0, 0);
    ge_set_xy(LIB_GE_HANDLE, cmd_hdl, GE_DST_SRC, rect->left, rect->top);
    ge_set_wh(LIB_GE_HANDLE, cmd_hdl, GE_DST_PTN, rect->width, rect->height);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_DST, GE_RGB_ORDER_ARGB);
    ge_set_rgb_order(LIB_GE_HANDLE, cmd_hdl, GE_PTN, GE_RGB_ORDER_ARGB);
    ge_cmd_end(LIB_GE_HANDLE, cmd_hdl);
    ///////
    if (!lib_ge_get_draw_state())
    {
        ge_cmd_list_end(LIB_GE_HANDLE, g_ge_osd_cmd_hdl);
        ge_cmd_list_start(LIB_GE_HANDLE, g_ge_osd_cmd_hdl, TRUE, 1);
    }

    return ;
}



/*******************************************************************************
*explaination
*as follow function:OSD Vscr translate ge draw.
*
*
*
*
*
*******************************************************************************/

RET_CODE lib_ge_vscr_draw_color(VSCR *dst_pvr, VSCR *src_pvr, UINT32 src_size, UINT32 src_bps,
    struct osdrect *rect, DRAW_COLOR *color, INT32 scr_pitch, UINT32 d_type)
{
    UINT8           *p_ret  = NULL;
    sgt_raw_bitmap    bmp_dst;
    sgt_raw_bitmap    bmp_rsc;
    ge_rect_t       ge_rect;

    if ((NULL == dst_pvr) || (NULL== rect))
    {
        ASSERT(0);
        return RET_FAILURE;
    }
    MEMSET(&bmp_dst,0x00,sizeof(sgt_raw_bitmap));
    MEMSET(&bmp_rsc,0x00,sizeof(sgt_raw_bitmap));
    MEMSET(&ge_rect,0x00,sizeof(ge_rect_t));
    bmp_dst.color_mode  = dst_pvr->b_color_mode;
    bmp_dst.data        = dst_pvr->lpb_scr;
    bmp_dst.height      = dst_pvr->v_r.u_height;
    bmp_dst.width       = dst_pvr->v_r.u_width;
    bmp_dst.left        = dst_pvr->v_r.u_left;
    bmp_dst.top         = dst_pvr->v_r.u_top;

    ge_rect.height      = rect->u_height;
    ge_rect.width       = rect->u_width;
    ge_rect.top         = rect->u_top;
    ge_rect.left        = rect->u_left;


    switch (d_type)
    {
        case LIB_GE_DRAW_WINDOW:
            if (NULL == color)
            {
                ASSERT(0);
                return RET_FAILURE;
            }
            lib_ge_draw_windows(&bmp_dst, &ge_rect, scr_pitch, color->fg_color,color->pen_color);
            break;
        case LIB_GE_DRAW_ALPHA_BLEEDING:
        case LIB_GE_DRAW_KEY_COLOR:
        case LIB_GE_DRAW_ALPHA_BLEEDING_NO_BG:
            if ((NULL == src_pvr) || (NULL == rect) || (NULL == color))
            {
                ASSERT(0);
                return RET_FAILURE;
            }

            bmp_rsc.bitmap_size = src_size;
            bmp_rsc.bps         = src_bps;
            bmp_rsc.width       = src_pvr->v_r.u_width;
            bmp_rsc.height      = src_pvr->v_r.u_height;
            bmp_rsc.left        = 0;
            bmp_rsc.top         = 0;
            bmp_rsc.data        = src_pvr->lpb_scr;
            bmp_rsc.color_mode  = src_pvr->b_color_mode;
            if (LIB_GE_DRAW_KEY_COLOR == d_type)
            {
                lib_ge_copy_bitmap_key_color(&bmp_rsc, &bmp_dst, &ge_rect, scr_pitch, color->bg_color);
            }
            else if (LIB_GE_DRAW_ALPHA_BLEEDING == d_type)
            {
                lib_ge_copy_bitmap_alpha_blending(&bmp_rsc, &bmp_dst, &ge_rect, scr_pitch, color->bg_color);
            }
            else if(LIB_GE_DRAW_ALPHA_BLEEDING_NO_BG == d_type)
            {
                lib_ge_copy_bitmap_alpha_blending_no_bg(&bmp_rsc, &bmp_dst, &ge_rect, scr_pitch);
            }
            break;
        case LIB_GE_DRAW_RLE_DECODE:
            bmp_rsc.bitmap_size = src_size;
            bmp_rsc.bps         = src_bps;
            bmp_rsc.width       = src_pvr->v_r.u_width;
            bmp_rsc.height      = src_pvr->v_r.u_height;
            bmp_rsc.left        = src_pvr->v_r.u_left;
            bmp_rsc.top         = src_pvr->v_r.u_top;
            bmp_rsc.data        = src_pvr->lpb_scr;
            bmp_rsc.color_mode  = src_pvr->b_color_mode;
            if (NULL == dst_pvr->lpb_scr)
            {
                ASSERT(0);
            }
            p_ret = lib_ge_decode_bitmap(dst_pvr->lpb_scr, &bmp_rsc, TRUE);
            if(NULL == p_ret)
            {
                return RET_FAILURE;
            }
            break;
        default:
            break;
    }

    return RET_SUCCESS;
}

RET_CODE lib_ge_vscr_draw_font(VSCR *pvr, struct osdrect *rect, UINT8 *font_data,
    INT32 scr_pitch, UINT8 bps, UINT32 fg_color, UINT32 bg_color)
{
    sgt_raw_bitmap  bmp_dst;
    ge_rect_t     ge_rect;

    if((NULL == pvr) || (NULL == rect) || (NULL == font_data))
    {
        return RET_FAILURE;
    }
    MEMSET(&bmp_dst,0x00,sizeof(sgt_raw_bitmap));
    MEMSET(&ge_rect,0x00,sizeof(ge_rect_t));
    bmp_dst.color_mode  = pvr->b_color_mode;
    bmp_dst.data        = pvr->lpb_scr;
    bmp_dst.height      = pvr->v_r.u_height;
    bmp_dst.width       = pvr->v_r.u_width;
    bmp_dst.left        = pvr->v_r.u_left;
    bmp_dst.top         = pvr->v_r.u_top;

    ge_rect.height      = rect->u_height;
    ge_rect.width       = rect->u_width;
    ge_rect.top         = rect->u_top;
    ge_rect.left        = rect->u_left;

    if (bps > const_pixel_4_bits)
    {
        return RET_FAILURE;
    }
    else
    {
        if (const_pixel_4_bits == bps)
        {
            lib_ge_draw_4bit_font(&bmp_dst, &ge_rect, font_data, scr_pitch, fg_color, bg_color, bps);
        }
        else if (const_pixel_1_bits == bps)
        {
            lib_ge_draw_1bit_font(&bmp_dst, &ge_rect, font_data, scr_pitch,fg_color, bg_color, bps);
        }
    }
    return RET_SUCCESS;
}

RET_CODE lib_ge_vscr_copy_bitmap(VSCR *dst_pvr, VSCR *src_pvr,struct osdrect *dst_rect,
    struct osdrect *src_rect)
{
    RET_CODE        ret     = RET_SUCCESS;
    UINT8           *p_ret  = NULL;
    sgt_raw_bitmap    bmp_dst;
    sgt_raw_bitmap    bmp_rsc;
    ge_rect_t       ge_src_rect;
    ge_rect_t       ge_dst_rect;

    if ((NULL == dst_pvr) || (NULL == src_pvr) || (NULL == src_rect) || (NULL == dst_rect))
    {
        ret = RET_FAILURE;
    }
    else
    {
        MEMSET(&bmp_dst,0x00,sizeof(sgt_raw_bitmap));
        MEMSET(&bmp_rsc,0x00,sizeof(sgt_raw_bitmap));
        MEMSET(&ge_src_rect,0x00,sizeof(ge_rect_t));
        MEMSET(&ge_dst_rect,0x00,sizeof(ge_rect_t));
        bmp_dst.data       = dst_pvr->lpb_scr;
        bmp_dst.color_mode = dst_pvr->b_color_mode;
        bmp_dst.top        = 0;
        bmp_dst.left       = 0;
        bmp_dst.width      = dst_pvr->v_r.u_width;
        bmp_dst.height     = dst_pvr->v_r.u_height;

        ge_dst_rect.left   = dst_rect->u_left;
        ge_dst_rect.top    = dst_rect->u_top;
        ge_dst_rect.width  = dst_rect->u_width;
        ge_dst_rect.height = dst_rect->u_height;

        bmp_rsc.data       = src_pvr->lpb_scr;
        bmp_rsc.color_mode = src_pvr->b_color_mode;
        bmp_rsc.top        = 0;
        bmp_rsc.left       = 0;
        bmp_rsc.width      = src_pvr->v_r.u_width;
        bmp_rsc.height     = src_pvr->v_r.u_height;

        ge_src_rect.left   = src_rect->u_left;
        ge_src_rect.top    = src_rect->u_top;
        ge_src_rect.width  = src_rect->u_width;
        ge_src_rect.height = src_rect->u_height;

        p_ret = lib_ge_copy_bitmap(&bmp_rsc, &bmp_dst, &ge_src_rect, &ge_dst_rect);
        if(NULL == p_ret)
        {
            ret = RET_FAILURE;
        }
    }
    return ret;
}

#endif

