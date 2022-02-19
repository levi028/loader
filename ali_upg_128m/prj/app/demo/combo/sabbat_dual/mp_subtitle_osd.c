/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mp_subtitle_osd.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>

#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/osd/osddrv.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common.h>

#include <api/libmp/mp_subtitle_api.h>

#include "mp_subtitle_osd.h"

#include "copper_common/system_data.h"
#include "osd_rsc.h"

/**********************Global  Variables***********************************/
#define MP_SUBTITLE_VALUE 256
#define MP_SUBTITLE_NUMBER 96
#define MP_SUBTITLE_COUNT 48
#define MP_SUBTITLE_NUM 4
#define MP_SUBTITLE_STR_LEN 1023
//define the Global  Variables
static const unsigned char mp_subtitle_pallette_256[4 * 256] = {
    /*front */
    0x10, 0x80, 0x80, 0x0f,    //black
    0x50, 0x5b, 0xec, 0x0f,    //red
    0x8f, 0x37, 0x24, 0x0f,    //green
    0xd0, 0x12, 0x90, 0x0f,    //yellow
    0x27, 0xec, 0x6e, 0x0f,    //blue
    0x68, 0xc7, 0xda, 0x0f,    //magenta
    0xa7, 0xa3, 0x12, 0x0f,    //cyan
    0xea, 0x7f, 0x7f, 0x0f,    //white
    /*back */
    0xea, 0x7f, 0x7f, 0x0f,    //white
    0x50, 0x5b, 0xec, 0x0f,    //red
    0x8f, 0x37, 0x24, 0x0f,    //green
    0xd0, 0x12, 0x90, 0x0f,    //yellow
    0x27, 0xec, 0x6e, 0x0f,    //blue
    0x68, 0xc7, 0xda, 0x0f,    //magenta
    0xa7, 0xa3, 0x12, 0x0f,    //cyan
    0x10, 0x80, 0x80, 0x0f,    //black

    0, 0, 0, 0,            //transparent
};

//for persian subtitle 20130618
#ifdef PERSIAN_SUPPORT
UINT16 str_keep[1024]={0};
UINT16 str_cnt=0;
#endif

#ifdef PERSIAN_SUPPORT
extern void xformer_capture_semaphore(void);
extern void xformer_release_semaphore(void);
extern void arabic_unistr_xformer(unsigned short* uni_str,BOOL convert_flag,BOOL mb_flag);
#endif

/**Static  Function Declare**/
//static BOOL mp_subtitle_scale_no_bl(HANDLE osd_dev, UINT16 screen_width);
static void mp_subtitle_osd_clean_screen(mp_subtitle_osd * subtitle_osd);
static void mp_subtitle_osd_change_pallette
    (mp_subtitle_osd * subtitle_osd, unsigned char *pallette);
static void mp_subtitle_osd_draw_string
    (mp_subtitle_osd * subtitle_osd, unsigned short x,
     unsigned short y, unsigned char fg_color, unsigned short *string);
static void mp_subtitle_osd_draw_image(mp_subtitle_osd * subtitle_osd,
                       unsigned short x, unsigned short y,
                       unsigned short width,
                       unsigned short height,
                       unsigned char *image);
static void mp_subtitle_osd_get_rects_cross(const struct osdrect *backr,
                        const struct osdrect *forgr,
                        struct osdrect *relativr);
static BOOL mp_subtitle_osd_bitmap_format_transfer(bit_map_t * bmpdest,
                           bit_map_t * bmpsrc,
                           UINT32 transcolor,
                           UINT32 forecolor);
static void mp_subtitle_osd_icon_char_data2bitmap(FONTICONPROP * pobjattr,
                          UINT8 * pdtabitmap,
                          bit_map_t * bmp,
                          ID_RSC rsclibid);
#if 0
static void mp_subtitle_osd_draw_char(mp_subtitle_osd * subtitle_osd,
                      unsigned short x, unsigned short y,
                      unsigned char fg_color,
                      unsigned short character);
#endif
#ifndef MP_SUBTITLE_SETTING_SUPPORT
static void mp_subtitle_osd_draw_bmp(mp_subtitle_osd * subtitle_osd,
                     UINT8 * pdtabitmap,
                     OBJECTINFO * rsclibinfo, LPVSCR pbvscr,
                     struct osdrect *r, ID_RSC rsclibid,
                     UINT32 dwcolorflag);
static void mp_subtitle_osd_draw_char2(mp_subtitle_osd * subtitle_osd,
                       unsigned char *char_addr,
                       OBJECTINFO * object_info,
                       struct osdrect *r, ID_RSC resource_id,
                       unsigned char fg_color);
#else
static void mp_subtitle_osd_draw_bmp3(mp_subtitle_osd * subtitle_osd,
                      UINT8 * pdtabitmap,
                      OBJECTINFO * rsclibinfo, LPVSCR pbvscr,
                      struct osdrect *r, ID_RSC rsclibid,
                      UINT32 dwcolorflag, UINT32 dwbgcolorflag);
static void mp_subtitle_osd_draw_char3(mp_subtitle_osd * subtitle_osd,
                       unsigned char *char_addr,
                       OBJECTINFO * object_info,
                       struct osdrect *r, ID_RSC resource_id,
                       unsigned char fg_color,
                       unsigned char bg_color);
#endif

#if 0
#endif
/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_create
//
// Description:
//  This function is sample callback function.
//  The subtitle APIs will call this callback
//  function to create a subtitle osd region.
//    Customer who has different osd functions should implement it by yourself.
//
// Arguments:
//      subtitle_osd - Pointer to pointer a struct contains osd infomation
//
// Return Value:
//      1 - Success
//      -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
int mp_subtitle_osd_create(mp_subtitle_osd **psubtitle_osd)
{
    struct osdpara osd_open_para;
    struct osdrect osd_open_rect;
    struct osdrect r;
    enum tvsystem tv_sys = PAL;
    enum osdsys eosdsys = OSD_PAL;
    mp_subtitle_osd *subtitle_osd = NULL;

    MEMSET(&osd_open_para, 0 ,sizeof(osd_open_para));
    MEMSET(&osd_open_rect, 0 ,sizeof(osd_open_rect));
    MEMSET(&r, 0 ,sizeof(r));
    MP_SUB_DEBUG("====>>mp_subtitle_osd_create()\n");

    if (!psubtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_create: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_create()\n");
    return -1;
    }

    subtitle_osd = (mp_subtitle_osd *) MALLOC(sizeof (mp_subtitle_osd));
    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_create: Malloc failed!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_create()\n");
    return -1;
    }

    MEMSET(subtitle_osd, 0, sizeof (mp_subtitle_osd));

    subtitle_osd->osd_dev = dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    if (NULL == subtitle_osd->osd_dev)
    {
    FREE(subtitle_osd);
    MP_SUB_DEBUG("mp_subtitle_osd_create: Get osd_dev failed!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_create()\n");
    return -1;
    }

    osddrv_show_on_off((HANDLE) subtitle_osd->osd_dev, OSDDRV_OFF);

    osd_open_para.e_mode = OSD_256_COLOR;
    osd_open_para.u_galpha_enable = FALSE;
    osd_open_para.u_galpha = 0x0f;
    osd_open_para.u_pallette_sel = 0;

    osddrv_close((HANDLE) subtitle_osd->osd_dev);
    osddrv_open((HANDLE) subtitle_osd->osd_dev, &osd_open_para);
    osal_task_sleep(20);

    osd_open_rect.u_left = 0;    //MP_SUBTITLE_OSD_STARTCOL;
    osd_open_rect.u_width = MP_SUBTITLE_OSD_WIDTH;
    osd_open_rect.u_top = 0;    //MP_SUBTITLE_OSD_STARTROW;
    osd_open_rect.u_height = MP_SUBTITLE_OSD_HEIGHT;

    subtitle_osd->region = 0;
    osddrv_create_region((HANDLE) subtitle_osd->osd_dev,
            subtitle_osd->region, &(osd_open_rect), NULL);
    osddrv_set_pallette((HANDLE) subtitle_osd->osd_dev,
               (unsigned char *)(mp_subtitle_pallette_256),
               256, OSDDRV_YCBCR);

    r.u_left = 0;
    r.u_top = 0;
    r.u_width = osd_open_rect.u_width;
    r.u_height = osd_open_rect.u_height;
    osddrv_region_fill((HANDLE) subtitle_osd->osd_dev,
              subtitle_osd->region, &r, 16);

    vpo_ioctl((struct vpo_device *)dev_get_by_type
          (NULL, HLD_DEV_TYPE_DIS),
          VPO_IO_GET_OUT_MODE, (UINT32) (&tv_sys));
    if ((NTSC == tv_sys) || (PAL_M == tv_sys)
    || (PAL_60 == tv_sys) || (NTSC_443 == tv_sys))
    {
    eosdsys = OSD_NTSC;
    }
    else
    {
    eosdsys = OSD_PAL;
    }
    osddrv_scale((HANDLE) subtitle_osd->osd_dev,
         OSD_VSCALE_TTX_SUBT, (unsigned long)(&eosdsys));
    //mp_subtitle_scale_no_bl((HANDLE)subtitle_osd->osd_dev,
    //MP_SUBTITLE_OSD_WIDTH);

    osddrv_show_on_off((HANDLE) subtitle_osd->osd_dev, OSDDRV_ON);
    osddrv_region_show((HANDLE) subtitle_osd->osd_dev, subtitle_osd->region,
              TRUE);

    *psubtitle_osd = subtitle_osd;

    MP_SUB_DEBUG("<<====mp_subtitle_osd_create()\n");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_destroy
//
// Description:
//  This function is sample callback function.
//  The subtitle APIs will call this callback
//  function to delete a subtitle osd region.
//    Customer who has different osd functions should implement it by yourself.
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//
// Return Value:
//      1 - Success
//      -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
int mp_subtitle_osd_destroy(mp_subtitle_osd  *subtitle_osd)
{
    struct osdrect r;

    MEMSET(&r, 0 ,sizeof(r));
    MP_SUB_DEBUG("====>>mp_subtitle_osd_destroy()\n");

    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_destroy: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_destroy()\n");
    return -1;
    }

    r.u_left = 0;
    r.u_top = 0;
    r.u_width = MP_SUBTITLE_OSD_WIDTH;
    r.u_height = MP_SUBTITLE_OSD_HEIGHT;

    osddrv_region_fill((HANDLE) subtitle_osd->osd_dev,
              subtitle_osd->region, &r, 16);

    osddrv_delete_region((HANDLE) subtitle_osd->osd_dev, subtitle_osd->region);

    //OSDDrv_Scale((HANDLE)osd_dev,OSD_VSCALE_OFF,(unsigned long)(&eosdsys));
    osddrv_show_on_off((HANDLE) subtitle_osd->osd_dev, OSDDRV_OFF);

    FREE(subtitle_osd);

    MP_SUB_DEBUG("<<====mp_subtitle_osd_destroy()\n");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_clean_screen
//
// Description:
//  This function is sample callback function.
//  The subtitle APIs will call this callback
//  function to control  subtitle osd.
//  It will use this callback to clean screen,
//   change palette, draw string and draw image at now.
//  Customer who has different osd functions should implement it by yourself.
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      control_code - Control code
//      parameters - Depends on control_code
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_osd_control(mp_subtitle_osd  *subtitle_osd,
                 long control_code, long parameters)
{
    long ret = 0;
    mp_subtitle_draw_string_parameters *str_para = NULL;
    mp_subtitle_draw_image_parameters *img_para = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_osd_control()\n");

    ret = -1;
    switch (control_code)
    {
    case MP_SUBTITLE_CONTROL_CLEAN_SCREEN:
    {
        MP_SUB_DEBUG("mp_subtitle_osd_control:\
                     MP_SUBTITLE_CONTROL_CLEAN_SCREEN\n");
        mp_subtitle_osd_clean_screen(subtitle_osd);
        ret = 1;
        break;
    }

    case MP_SUBTITLE_CHANGE_PALLETE:
    {
        MP_SUB_DEBUG("mp_subtitle_osd_control: \
                     MP_SUBTITLE_CHANGE_PALLETE\n");
        mp_subtitle_osd_change_pallette(subtitle_osd,
                        (unsigned char *)parameters);
        ret = 1;
        break;
    }

    case MP_SUBTITLE_DRAW_STRING:
    {
        MP_SUB_DEBUG("mp_subtitle_osd_control: MP_SUBTITLE_DRAW_STRING\n");
        str_para = (mp_subtitle_draw_string_parameters *)parameters;
        if (str_para)
        {
        mp_subtitle_osd_draw_string(subtitle_osd,
                        str_para->x, 410, str_para->fg_color,
                        str_para->string);
        ret = 1;
        }
        break;
    }

    case MP_SUBTITLE_DRAW_IMAGE:
    {
        MP_SUB_DEBUG("mp_subtitle_osd_control: MP_SUBTITLE_DRAW_IMAGE\n");
        img_para = (mp_subtitle_draw_image_parameters *)parameters;
        if (img_para)
        {
        if (MP_SUBTITLE_OSD_HEIGHT < img_para->y + img_para->height)
        {
            if (OSD_SCREEN_HEIGHT < img_para->height)
            {
            img_para->height = 576;
            }
            img_para->y = (576 - img_para->height) / 2;
            img_para->y &= ~1;
        }
        mp_subtitle_osd_draw_image(subtitle_osd,
                       img_para->x, img_para->y, img_para->width,
                       img_para->height, img_para->image);
        ret = 1;
        }
        break;
    }

    default:
    {
        MP_SUB_DEBUG("mp_subtitle_osd_control: Unknown control code!\n");
        break;
    }

    }

    MP_SUB_DEBUG("<<====mp_subtitle_osd_control()\n");

    return ret;
}

#if 0
/****Internal Static  Function ****/

static BOOL mp_subtitle_scale_no_bl(HANDLE osd_dev, UINT16 screen_width)
{
    enum tvsystem tvsys;

    vpo_ioctl((struct vpo_device *)dev_get_by_type
          (NULL, HLD_DEV_TYPE_DIS), VPO_IO_GET_OUT_MODE, (UINT32) (&tvsys));

    UINT32 param = ap_get_osd_scale_param(tvsys, screen_width);

    osddrv_scale(osd_dev, OSD_SCALE_WITH_PARAM, param);

    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_clean_screen
//
// Description:
//      Clear subtitle osd region
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_clean_screen(mp_subtitle_osd  *subtitle_osd)
{
    struct osdrect r;

    MEMSET(&r, 0 ,sizeof(r));
    MP_SUB_DEBUG("====>>mp_subtitle_osd_destroy()\n");

    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_destroy: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_destroy()\n");
    return;
    }

    r.u_left = 0;
    r.u_top = 0;
    r.u_width = MP_SUBTITLE_OSD_WIDTH;
    r.u_height = MP_SUBTITLE_OSD_HEIGHT;

    osddrv_region_fill((HANDLE) subtitle_osd->osd_dev,
              subtitle_osd->region, &r, 16);

}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_change_pallette
//
// Description:
//      Change subtitle osd region pallette
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_change_pallette
    (mp_subtitle_osd  *subtitle_osd, unsigned char *pallette)
{
    MP_SUB_DEBUG("====>>mp_subtitle_osd_change_pallette()\n");

    if ((!subtitle_osd) || (!pallette))
    {
    MP_SUB_DEBUG("mp_subtitle_osd_change_pallette: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_change_pallette()\n");
    return;
    }

    osddrv_set_pallette((HANDLE) subtitle_osd->osd_dev,
               pallette, 256, OSDDRV_YCBCR);

    MP_SUB_DEBUG("<<====mp_subtitle_osd_change_pallette()\n");
    return;
}

#ifdef MP_SUBTITLE_SETTING_SUPPORT
/////////////////////////////////////////////////////////////////////////////
//change system data color to subtitle color, according to the subt pallete
//      SUBT_COLOR_BLACK = 0, ==> 0
//      SUBT_COLOR_RED = 1, ==>1
//      SUBT_COLOR_GREEN = 2, ==>2
//      SUBT_COLOR_YELLOW = 3, ==>3
//      SUBT_COLOR_BLUE = 4, ==>4
//      SUBT_COLOR_WHITE = 5, ==>7
//      SUBT_COLOR_TRANSPARENT = 6, ==>16
/////////////////////////////////////////////////////////////////////////////
static INT8 sys_color_2_subt_color(INT8 sys_color)
{
    INT8 subt_color = 0;

    if (SUBT_COLOR_WHITE == sys_color)
    {
    subt_color = 7;
    }
    else if (SUBT_COLOR_TRANSPARENT == sys_color)
    {
    subt_color = 16;
    }
    else
    {
    subt_color = sys_color;
    }

    return subt_color;
}

/////////////////////////////////////////////////////////////////////////////
//change system data y position to subtitle y position
//system data y:0~100
//subtitle data y:-50~50
/////////////////////////////////////////////////////////////////////////////
static INT8 sys_yoffset_2_subt_yoffset(INT8 sys_yoffset)
{
    INT8 subt_yoffset = 0;

    subt_yoffset = 50 - sys_yoffset;

    return subt_yoffset;
}
#endif

static void subtitle_osd_draw_string_impl(mp_subtitle_osd  *subtitle_osd,
                       unsigned short x, unsigned short y,long *line_total_width,long total_height,UINT8 *width2,
                      unsigned char fg_color,
                      unsigned short *string)
{
    unsigned short uni_char = 0;
    ID_RSC resource_id = 0;
    OBJECTINFO object_info;
    unsigned char *char_addr = NULL;
    short width = 0;
    short height = 0;
    long line_width = 0;
    long current_height = 0;
    struct osdrect r;
    //UINT8 i1 = 0;
    UINT8 i2 = 0;
    UINT16 temp = 0;
    UINT16 temp_y = 0;
    //unsigned short *stringcopy = NULL;
    short lines = 0;
    long line_start_width = 0;
    ID_RSC rscid = 0;
    long width_temp = 0;
    long x_offset = 80;

    MEMSET(&object_info, 0 ,sizeof(object_info));
    MEMSET(&r, 0 ,sizeof(r));
    rscid = osd_get_lang_environment();

#ifdef MP_SUBTITLE_SETTING_SUPPORT
    SYSTEM_DATA *sys_data = sys_data_get();
    INT8 font_size = sys_data->ext_subt_param.font_size;
    INT8 bg_color = sys_color_2_subt_color(sys_data->ext_subt_param.bg_color);
    INT8 y_offset = sys_yoffset_2_subt_yoffset(sys_data->ext_subt_param.y_offset);

#endif

    while (*string)
    {
        uni_char = *string;
        if ((0x000D == uni_char) || (0x000A == uni_char) || (L'|' == uni_char))    //cmx0815
        {
            if ((0x000A == uni_char) || (L'|' == uni_char))    //cmx0815
            {
                lines++;
                line_width = 0;
                current_height += height;
                if (current_height > MP_SUBTITLE_NUMBER)
                {
                    break;
                }
            }
            string++;
            continue;
        }

        resource_id = osd_get_default_font_lib(uni_char);

#ifdef MP_SUBTITLE_SETTING_SUPPORT
        resource_id |= font_size;
#endif

        char_addr =
            (unsigned char *)osd_get_rsc_obj_data_ext(resource_id, uni_char,
                                   &object_info,
                                   (UINT8 *)subtitle_osd->font_buffer,
                                   sizeof
                                   (subtitle_osd->font_buffer));
        if (NULL == char_addr)
        {
            string++;
            MP_SUB_DEBUG("mp_subtitle_osd_draw_char: We can find this char!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
            continue;
        }

        width = object_info.m_obj_attr.m_w_actual_width;
        height = object_info.m_obj_attr.m_w_height;
        if (( /*sys_data->lang.OSD_lang */ PERSIAN_ENV == rscid)
#ifdef HEBREW_SUPPORT
            || ( /*sys_data->lang.OSD_lang */ HEBREW_ENV == rscid)
#endif
            )
        {
            line_start_width = (MP_SUBTITLE_OSD_WIDTH - (x + line_total_width[current_height / 24])) /
                2 + line_total_width[current_height / 24];    //???
        }
        else
        {
            line_start_width =
                (MP_SUBTITLE_OSD_WIDTH - line_total_width[lines]) / 2;
        }

        //set mp_subtitle_vscr size
        r.u_left = line_start_width + line_width;
        temp_y = (UINT32) MP_SUBTITLE_OSD_YOFFSET_TEMP;
        temp_y = temp_y >> 1;
        //r.uLeft = x + line_width + MP_SUBTITLE_OSD_XOFFSET;
#ifdef MP_SUBTITLE_SETTING_SUPPORT
        r.u_top = MP_SUBTITLE_OSD_HEIGHT - temp_y -
            total_height + current_height - y_offset;
#else
        r.u_top = y + current_height + temp_y;
#endif
        r.u_width = width;
        r.u_height = height;
        if (( /*sys_data->lang.OSD_lang */ PERSIAN_ENV == rscid)
#ifdef HEBREW_SUPPORT
            || ( /*sys_data->lang.OSD_lang */ HEBREW_ENV == rscid)
#endif
            )
        {
            //for persian subtitle 20130618
            //if((width2[i2+1]>1)&&(width2[i2+1]<24))
            temp = i2 + 1;
            if ((width2[temp] > NUM_ONE)
                && (width2[temp] < MP_SUBTITLE_COUNT /*24 */ ))
            {
                line_width -= width2[i2 + 1];
            }
            else
            {
                line_width -= 20;
            }

        }
        else
        {
            line_width += width;
        }
#ifdef _M3383_SABBAT_
        width_temp = (MP_SUBTITLE_OSD_WIDTH - 2 * x - 50);
        if (line_width > width_temp)
#else
            //if (line_width > (MP_SUBTITLE_OSD_WIDTH - 2*x))
        width_temp = (MP_SUBTITLE_OSD_WIDTH - x_offset);
        if (line_width > width_temp)
#endif
        {
            lines++;
            line_width = 0;
            current_height += height;
            line_start_width =
                (MP_SUBTITLE_OSD_WIDTH - line_total_width[lines]) / 2;
            r.u_left = line_start_width + line_width;
            temp_y = (UINT32) MP_SUBTITLE_OSD_YOFFSET_TEMP;
            temp_y = temp_y >> 1;
#ifdef MP_SUBTITLE_SETTING_SUPPORT
            r.u_top = MP_SUBTITLE_OSD_HEIGHT - temp_y
                - total_height + current_height - y_offset;
#else
            r.u_top = y + current_height + temp_y;
#endif
            line_width += width;
            if (current_height > MP_SUBTITLE_NUMBER)
            {
                break;
            }

        }

#ifdef MP_SUBTITLE_SETTING_SUPPORT
        mp_subtitle_osd_draw_char3(subtitle_osd,
                       char_addr, &object_info, &r, resource_id,
                       fg_color, bg_color);
#else
        mp_subtitle_osd_draw_char2(subtitle_osd,
                       char_addr, &object_info, &r, resource_id,
                       fg_color);
#endif

        string++;
        i2++;
        temp = i2 + 1;
        if (temp >= MP_SUBTITLE_VALUE)
        {
            break;        //joey.che  
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_string
//
// Description:
//      Draw string on subtitle osd region
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      x - Offset x
//      y - Offset y
//      fg_color - String color
//      string - String
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_draw_string(mp_subtitle_osd  *subtitle_osd,
                    unsigned short x, unsigned short y,
                    unsigned char fg_color,
                    unsigned short *string)
{
    unsigned short uni_char = 0;
    ID_RSC resource_id = 0;
    OBJECTINFO object_info;
    unsigned char *char_addr = NULL;
    UINT8 width2[256] = { 0 };
    short height = 0;
    long line_width = 0;
    long current_height = 0;
    UINT8 i1 = 0;
    unsigned short *stringcopy = NULL;
    long line_total_width[5] = { 0 };
    short lines = 0;
    ID_RSC rscid = 0;
    long x_offset = 80;
    const UINT8 line_cnt = 5;

	if(0 == rscid)
	{
		;
	}
	if(0 == current_height)
	{
		;
	}
	if(0 == line_width)
	{
		;
	}

	MEMSET(&object_info, 0 ,sizeof(object_info));
    rscid = osd_get_lang_environment();

#ifdef MP_SUBTITLE_SETTING_SUPPORT
    long total_height = 0;    //for string total height
    SYSTEM_DATA *sys_data = sys_data_get();
    INT8 font_size = sys_data->ext_subt_param.font_size;

    fg_color = sys_color_2_subt_color(sys_data->ext_subt_param.fg_color);
#endif

#ifdef PERSIAN_SUPPORT
    xformer_capture_semaphore();
    //for persian subtitle 20130618
    str_cnt = 0;
    while ((string[str_cnt]) && (MP_SUBTITLE_STR_LEN > str_cnt))
    {
        str_keep[str_cnt] = string[str_cnt];
        str_cnt++;
    }
    str_keep[str_cnt] = 0x0;
    arabic_unistr_xformer(string, TRUE, FALSE);    //handle arabic and persian string
    xformer_release_semaphore();
#endif

    stringcopy = string;
    MP_SUB_DEBUG("====>>mp_subtitle_osd_draw_string()\n");

    if ((!subtitle_osd) || (!string))
    {
        MP_SUB_DEBUG("mp_subtitle_osd_draw_string: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_string()\n");
        return;
    }

    line_width = 0;
    height = 24;
    current_height = 0;

    for (lines = 0; lines < 5; lines++)
    {
        line_total_width[lines] = 0;
    }

    lines = 0;

    while (*stringcopy)
    {
        uni_char = *stringcopy;
        if ((0x000D == uni_char) || (0x000A == uni_char) || (L'|' == uni_char))    //cmx0815
        {
            if ((0x000A == uni_char) || (L'|' == uni_char))    //cmx0815
            {
                lines++;

#ifdef MP_SUBTITLE_SETTING_SUPPORT
                total_height += height;
#endif
                if (lines > MP_SUBTITLE_NUM)
                {
                    break;
                }
            }
            stringcopy++;
            continue;
        }

        resource_id = osd_get_default_font_lib(uni_char);

#ifdef MP_SUBTITLE_SETTING_SUPPORT
        resource_id |= font_size;
#endif
        char_addr =
            (unsigned char *)osd_get_rsc_obj_data_ext(resource_id, uni_char,
                                   &object_info,
                                   (UINT8 *)subtitle_osd->font_buffer,
                                   sizeof
                                   (subtitle_osd->font_buffer));
        if (NULL == char_addr)
        {
            stringcopy++;
            MP_SUB_DEBUG("mp_subtitle_osd_draw_char: We can find this char!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
            continue;
        }

#ifdef MP_SUBTITLE_SETTING_SUPPORT
        height = object_info.m_obj_attr.m_w_height;
#endif

        width2[i1] = object_info.m_obj_attr.m_w_actual_width;
        line_total_width[lines] += object_info.m_obj_attr.m_w_actual_width;
        if (line_total_width[lines] > (MP_SUBTITLE_OSD_WIDTH - x_offset))
        {
            line_total_width[lines] -= object_info.m_obj_attr.m_w_actual_width;
            lines++;

#ifdef MP_SUBTITLE_SETTING_SUPPORT
            total_height += height;
#endif
            if (lines < line_cnt)// avoid line_total_width overflow
            {
                line_total_width[lines] += object_info.m_obj_attr.m_w_actual_width;
            }
        }

        stringcopy++;
        i1++;
    }

#ifdef MP_SUBTITLE_SETTING_SUPPORT
    total_height += height;
#endif

    //for persian subtitle 20130618
#ifdef PERSIAN_SUPPORT
    stringcopy = string;
#endif

    lines = 0;


    subtitle_osd_draw_string_impl(subtitle_osd, x, y,line_total_width,total_height,width2,fg_color,string);

    //for persian subtitle 20130618
#ifdef PERSIAN_SUPPORT
    string = stringcopy;
    str_cnt = 0;
    while ((str_keep[str_cnt]) && (str_cnt < MP_SUBTITLE_STR_LEN))
    {
        string[str_cnt] = str_keep[str_cnt];
        str_cnt++;
    }
    string[str_cnt] = 0x0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_image
//
// Description:
//      Draw image on subtitle osd region
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      x - Offset x
//      y - Offset y
//      width - Image width
//      height - Image height
//      image - Image
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_draw_image(mp_subtitle_osd  *subtitle_osd,
                       unsigned short x, unsigned short y,
                       unsigned short width,
                       unsigned short height,
                       unsigned char *image)
{
    VSCR mp_subtitle_vscr;
    struct osdrect r;
    UINT16 temp = 0;
    MEMSET(&mp_subtitle_vscr, 0 ,sizeof(mp_subtitle_vscr));
    MEMSET(&r, 0 ,sizeof(r));

    MP_SUB_DEBUG("====>>mp_subtitle_osd_draw_string()\n");

    if ((!subtitle_osd) || (!image))
    {
    MP_SUB_DEBUG("mp_subtitle_osd_draw_string: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_string()\n");
    return;
    }

#ifdef MP_SUBTITLE_SETTING_SUPPORT
//modify y_offset start
    SYSTEM_DATA *sys_data = sys_data_get();
    INT32 y_offset = 0;

    y_offset = sys_yoffset_2_subt_yoffset(sys_data->ext_subt_param.y_offset);
//-50~50 => -100~100
//modify end
#endif

    r.u_left = x;
    temp = (UINT32) MP_SUBTITLE_OSD_YOFFSET_TEMP;
    temp = temp >> 1;
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    r.u_top = MP_SUBTITLE_OSD_HEIGHT - temp - height - y_offset;
#else
    r.u_top = y;
#endif
    r.u_width = width;
    r.u_height = height;

    //MEMSET(image, 1, spu->image_size);

    mp_subtitle_vscr.v_r.u_left = r.u_left;
    mp_subtitle_vscr.v_r.u_top = r.u_top;
    mp_subtitle_vscr.v_r.u_width = r.u_width;
    mp_subtitle_vscr.v_r.u_height = r.u_height;
    mp_subtitle_vscr.lpb_scr = image;

    // write Vscr data to osd frame buffer
    osddrv_region_write((HANDLE) subtitle_osd->osd_dev,
               subtitle_osd->region, &mp_subtitle_vscr, &r);

}

#ifndef MP_SUBTITLE_SETTING_SUPPORT
static UINT8 get_one_bit(const UINT8  *buf, UINT32 pos)
{
    UINT8 clu = 0;
    UINT8 mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    clu = *(buf + (pos >> 3));
    return (clu & mask[pos & 7]);
}
static void draw_sub_char_matrix(mp_subtitle_osd  *subtitle_osd,
                  UINT32 x, UINT32 y, UINT32 width, UINT32 height,
                  const UINT8  *ptr, UINT32 stride, UINT32 bg,
                  UINT32 fg)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 x0 = 0;
    UINT32 actual_width = (width >> 16) & 0xFFFF;
    UINT32 pos = stride >> 16;
    UINT32 one_bit = 0;
    UINT32 next_bit = 0;
    UINT32 num = 0;

    width &= 0xFFFF;
    if ((!actual_width) || (actual_width > width))
    {
    actual_width = width;
    }

    stride &= 0xffff;

    for (i = 0; i < height; i++, y++)
    {
    x0 = x;
    if (stride)
    {
        pos = 0;
    }

    one_bit = 0;
    next_bit = get_one_bit(ptr, pos++);

    j = 0;
    while (j < actual_width)
    {
        one_bit = next_bit;
        num = 0;

#if 1
        do
        {
        num++;
        j++;
        if (j >= actual_width)
        {
            break;
        }
        next_bit = get_one_bit(ptr, pos++);
        }
        while (next_bit == one_bit);
#else
        pos--;
        for (; j < width; j++, num++)
        {
        next_bit = get_one_bit(ptr, pos++);
        if (next_bit != one_bit)
            break;
        }
#endif
        if (one_bit)
        {
        osddrv_draw_hor_line((HANDLE) subtitle_osd->osd_dev,
                   (UINT8) subtitle_osd->region, x0, y, num,
                   fg);
        }
        else
        {
        if (0 == (bg & C_MIXER))
        {
            osddrv_draw_hor_line((HANDLE) subtitle_osd->osd_dev,
                       (UINT8) subtitle_osd->region, x0, y, num,
                       bg);
        }
        }
        x0 += num;
    }
    pos += (width - actual_width);
    ptr += stride;
    }
}
#endif
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_get_rects_cross
//
// Description:
//      Borrowed from osd library. Thank you, Xian_Zhou
//
// Arguments:
//      ?
//
// Return Value:
//      1 - Success
//      0 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_get_rects_cross(const struct osdrect *backr,
                        const struct osdrect *forgr,
                        struct osdrect *relativr)
{
    short ltx = 0;
    short lty = 0;
    short rbx = 0;
    short rby = 0;
    short dw = 0;
    short dh = 0;

    ltx = forgr->u_left >= backr->u_left ? forgr->u_left : backr->u_left;
    lty = forgr->u_top >= backr->u_top ? forgr->u_top : backr->u_top;
    rbx = (forgr->u_left + forgr->u_width) <=
    (backr->u_left + backr->u_width) ?
    (forgr->u_left + forgr->u_width) : (backr->u_left + backr->u_width);
    rby = (forgr->u_top + forgr->u_height) <=
    (backr->u_top + backr->u_height) ?
    (forgr->u_top + forgr->u_height) : (backr->u_top + backr->u_height);

    dw = rbx > ltx ? (rbx - ltx) : 0;
    dh = rby > lty ? (rby - lty) : 0;
    relativr->u_width = dw;
    relativr->u_height = dh;
    relativr->u_left = ltx;
    relativr->u_top = lty;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_bitmap_format_transfer
//
// Description:
//      Borrowed from osd library. Thank you, Xian_Zhou
//
// Arguments:
//      ?
//
// Return Value:
//      1 - Success
//      0 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static BOOL mp_subtitle_osd_bitmap_format_transfer(bit_map_t  *bmpdest,
                           bit_map_t  *bmpsrc,
                           UINT32 transcolor,
                           UINT32 forecolor)
{
    struct osdrect destrect;
    struct osdrect srcrect;
    struct osdrect crossrect;
    UINT16 i = 0;
    UINT16 j = 0;
    UINT8 maskdest = 0;
    UINT8 masksrc = 0;
    UINT8 backbyte = 0;
    UINT8 forebyte = 0;
    UINT8 forebits = 0;
    UINT8 fbytepixoffset = 0;
    UINT8 bbytepixoffset = 0;
    UINT8 pixsperbytedest = 0;
    UINT8 pixsperbytesrc = 0;
    UINT32 flinestartpixsoffset = 0;
    UINT32 blinestartpixsoffset = 0;
    UINT32 blinebyteoffset = 0;
    UINT8 *pforglinedata = NULL;
    UINT8 *pbacklinedata = NULL;
    UINT8 *pfgcolor = NULL;
    BOOL bmix = 0;
    UINT8 btranscolor = transcolor;
    UINT8 btranscolortmp = transcolor;
    UINT8 bforecolor = forecolor;

    if ((NULL == bmpsrc->p_data ) || (NULL == bmpdest->p_data))
    {
    return FALSE;
    }

    MEMSET(&destrect, 0x0, sizeof (destrect));
    MEMSET(&srcrect, 0x0, sizeof (srcrect));
    MEMSET(&crossrect, 0x0, sizeof (crossrect));
    destrect.u_left = bmpdest->u_left;
    destrect.u_top = bmpdest->u_top;
    destrect.u_width = bmpdest->u_width;
    destrect.u_height = bmpdest->u_height;
    srcrect.u_left = bmpsrc->u_left;
    srcrect.u_top = bmpsrc->u_top;
    srcrect.u_width = bmpsrc->u_width;
    srcrect.u_height = bmpsrc->u_height;

    // Get the cross rectangle of source and dest bitmap.
    mp_subtitle_osd_get_rects_cross(&destrect, &srcrect, &crossrect);

    // If the cross rectange is emplty return.
    if (0 == (crossrect.u_width * crossrect.u_height))
    {
    return FALSE;
    }

    maskdest = (2 << (bmpdest->bits_per_pix - 1)) - 1;
    masksrc = (2 << (bmpsrc->bits_per_pix - 1)) - 1;
    btranscolor &= masksrc;
    bforecolor &= maskdest;

    // If tansparenct color is not same with forground color,
    // the mixing is ture, else not mixing.
    if (bmpsrc->bits_per_pix < bmpdest->bits_per_pix)
    {
    btranscolor = 0;
    if (btranscolortmp != bforecolor)
    {
        bmix = TRUE;
    }
    else
    {
        bmix = FALSE;
    }
    }
    else
    {
    if (btranscolor != bforecolor)    // || masksrc==0xFF)
    {
        bmix = TRUE;
    }
    else
    {
        bmix = FALSE;
    }
    }

    // pixs per byte
    pixsperbytedest = 8 / bmpdest->bits_per_pix;
    pixsperbytesrc = 8 / bmpsrc->bits_per_pix;
    // The dest copy data area every line's start pix index in the first byte.
    blinestartpixsoffset = crossrect.u_left % pixsperbytedest;
    flinestartpixsoffset = (crossrect.u_left - srcrect.u_left) % pixsperbytesrc;
    // Decide what data are copyed to the dest bitmap
    if (bmpsrc->bits_per_pix < bmpdest->bits_per_pix)
    {
        pfgcolor = &bforecolor;    // 1bit per pix -- font data
        //if (NULL != bmpdest->pData)
        //{
            pbacklinedata = bmpdest->p_data + (crossrect.u_left - destrect.u_left) *
                bmpdest->bits_per_pix / 8 + bmpdest->stride * (crossrect.u_top - destrect.u_top);
            pforglinedata = bmpsrc->p_data + (crossrect.u_left - srcrect.u_left) * bmpsrc->bits_per_pix / 8 +
                bmpsrc->stride * (crossrect.u_top - srcrect.u_top);
            for (i = 0; i < crossrect.u_height; i++)    // every line
            {
            for (j = 0; j < crossrect.u_width; j++)    // every pix
            {
                forebyte =
                *(pforglinedata +
                  (flinestartpixsoffset + j) / pixsperbytesrc);
                fbytepixoffset =
                (pixsperbytesrc - 1 -
                 (flinestartpixsoffset +
                  j) % pixsperbytesrc) * bmpsrc->bits_per_pix;
                // Foreground pix of j from flinestartpixsoffset's
                // data offset in the forebyte
                forebits = (forebyte >> fbytepixoffset) & masksrc;
                *(pbacklinedata + j) = btranscolortmp;    //16;
                if ((!bmix) || (forebits != btranscolor))
                {
                blinebyteoffset =
                    (blinestartpixsoffset + j) / pixsperbytedest;

                bbytepixoffset =
                    (pixsperbytedest - 1 -
                     (blinestartpixsoffset +
                      j) % pixsperbytedest) * bmpdest->bits_per_pix;
                // Background pix of j from flinestartpixsoffset's data offset in the backbyte
                backbyte = *(pbacklinedata + blinebyteoffset);
                // Background byte of pix j from blinestartpixsoffset
                backbyte &= ~(maskdest << bbytepixoffset);
                backbyte |= *pfgcolor << bbytepixoffset;
                *(pbacklinedata + blinebyteoffset) = backbyte;
                }

            }
            pbacklinedata += bmpdest->stride;
            pforglinedata += bmpsrc->stride;
            }
        //}
    }
    else
    {
        pfgcolor = &forebits;    // icon data
        //if (NULL != bmpdest->pData)
        //{
            pbacklinedata = bmpdest->p_data + (crossrect.u_left - destrect.u_left)
            * bmpdest->bits_per_pix / 8
            + bmpdest->stride * (crossrect.u_top - destrect.u_top);
            pforglinedata = bmpsrc->p_data +
            (crossrect.u_left - srcrect.u_left) * bmpsrc->bits_per_pix / 8
            + bmpsrc->stride * (crossrect.u_top - srcrect.u_top);
            for (i = 0; i < crossrect.u_height; i++)    // every line
            {
            for (j = 0; j < crossrect.u_width; j++)    // every pix
            {
                if ((maskdest == masksrc) && (0xFF == maskdest))
                {
                forebyte = *(pforglinedata + flinestartpixsoffset + j);
                if ((!bmix) || (forebyte != btranscolor))
                {
                    *(pbacklinedata +
                      blinestartpixsoffset + j) = forebyte;
                }
                continue;
                }

                forebyte = *(pforglinedata +
                     (flinestartpixsoffset + j) / pixsperbytesrc);
                //Foreground byte of pix j from flinestartpixsoffset /
                fbytepixoffset = (pixsperbytesrc - 1 -
                          (flinestartpixsoffset +
                           j) % pixsperbytesrc) *
                bmpsrc->bits_per_pix;
                // Foreground pix of j from flinestartpixsoffset's
                // data offset in the forebyte /
                forebits = (forebyte >> fbytepixoffset) & masksrc;

                if ((!bmix) || (forebits != btranscolor))
                {
                blinebyteoffset = (blinestartpixsoffset + j) / pixsperbytedest;
                bbytepixoffset = (pixsperbytedest - 1 - (blinestartpixsoffset + j) % pixsperbytedest) * bmpdest->bits_per_pix;
                // Background pix of j from flinestartpixsoffset's
                // data offset in the backbyte /
                backbyte = *(pbacklinedata + blinebyteoffset);
                // Background byte of pix j from blinestartpixsoffset
                backbyte &= ~(maskdest << bbytepixoffset);
                backbyte |= *pfgcolor << bbytepixoffset;
                *(pbacklinedata + blinebyteoffset) = backbyte;
                }
            }
            pbacklinedata += bmpdest->stride;
            pforglinedata += bmpsrc->stride;
            }
        //}
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_icon_char_data2bitmap
//
// Description:
//      Borrowed from osd library. Thank you, Xian_Zhou
//
// Arguments:
//      ?
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_icon_char_data2bitmap
    (FONTICONPROP  *pobjattr, UINT8  *pdtabitmap, bit_map_t  *bmp,
     ID_RSC rsclibid)
{

    bmp->u_width = pobjattr->m_w_actual_width;
    //rsclibinfo.m_objAttr.m_wActualWidth
    bmp->u_height = pobjattr->m_w_height;

    if (LIB_FONT == (rsclibid & 0xF000))
    {
    if (LIB_FONT_MASSCHAR == (rsclibid & 0xFF00))
    {
        bmp->stride = pobjattr->m_w_width;
    }
    else
    {
        bmp->stride = pobjattr->m_w_width * pobjattr->m_b_color / 8;
    }
    }
    else
    {
    bmp->stride = pobjattr->m_w_width;
    }

    bmp->p_data = pdtabitmap;
    bmp->bits_per_pix = pobjattr->m_b_color;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_bmp
//
// Description:
//      Borrowed from osd library. Thank you, Xian_Zhou
//
// Arguments:
//      ?
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
#ifndef MP_SUBTITLE_SETTING_SUPPORT
static void mp_subtitle_osd_draw_bmp(mp_subtitle_osd  *subtitle_osd,
                     UINT8  *pdtabitmap,
                     OBJECTINFO  *rsclibinfo, LPVSCR pbvscr,
                     struct osdrect *r, ID_RSC rsclibid,
                     UINT32 dwcolorflag)
{
    UINT32 btranscolor = 0;
    UINT32 bforgcolor = 0;
    bit_map_t srcbmp;

    MEMSET(&srcbmp, 0, sizeof (bit_map_t));

    if ((NULL == pbvscr) || (NULL == pbvscr->lpb_scr))
    {
    return;
    }

    bforgcolor = GET_COLOR_IDX(dwcolorflag);
    srcbmp.u_left = r->u_left;
    srcbmp.u_top = r->u_top;
    mp_subtitle_osd_icon_char_data2bitmap
    (&rsclibinfo->m_obj_attr, pdtabitmap, &srcbmp, rsclibid);
    srcbmp.u_width = r->u_width > srcbmp.u_width ? srcbmp.u_width : r->u_width;
    srcbmp.u_height = r->u_height > srcbmp.u_height ? srcbmp.u_height : r->u_height;

#if 0
    btranscolor = 0;
    destbmp.u_left = pbvscr->v_r.u_left;
    destbmp.u_top = pbvscr->v_r.u_top;
    destbmp.u_width = pbvscr->v_r.u_width;
    destbmp.u_height = pbvscr->v_r.u_height;
    destbmp.p_data = pbvscr->lpb_scr;
    destbmp.stride = OSD_GET_VSCR_STIDE(pbvscr);
    destbmp.bits_per_pix = BIT_PER_PIXEL;

    mp_subtitle_osd_bitmap_format_transfer
    (&destbmp, &srcbmp, btranscolor, bforgcolor);
#else
    btranscolor = 255;
    draw_sub_char_matrix(subtitle_osd, srcbmp.u_left,
              srcbmp.u_top, srcbmp.u_width, srcbmp.u_height,
              srcbmp.p_data, srcbmp.stride, btranscolor, bforgcolor);
#endif
}

#else
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_bmp3
//
// Description:
//      Borrowed from osd library. Thank you, Xian_Zhou
//
// Arguments:
//      ?
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////

static void mp_subtitle_osd_draw_bmp3(mp_subtitle_osd  *subtitle_osd,
                      UINT8  *pdtabitmap,
                      OBJECTINFO  *rsclibinfo, LPVSCR pbvscr,
                      struct osdrect *r, ID_RSC rsclibid,
                      UINT32 dwcolorflag, UINT32 dwbgcolorflag)
{
    UINT32 btranscolor = 0;
    UINT32 bforgcolor = 0;
    bit_map_t destbmp;
    bit_map_t srcbmp;

    MEMSET(&destbmp, 0, sizeof (bit_map_t));
    MEMSET(&srcbmp, 0, sizeof (bit_map_t));
    if ((NULL == pbvscr) || (NULL == pbvscr->lpb_scr))
    {
    return;
    }

    bforgcolor = GET_COLOR_IDX(dwcolorflag);
    srcbmp.u_left = r->u_left;
    srcbmp.u_top = r->u_top;
    mp_subtitle_osd_icon_char_data2bitmap
    (&rsclibinfo->m_obj_attr, pdtabitmap, &srcbmp, rsclibid);
    srcbmp.u_width = r->u_width > srcbmp.u_width ? srcbmp.u_width : r->u_width;
    srcbmp.u_height = r->u_height > srcbmp.u_height ? srcbmp.u_height : r->u_height;

#if 1
    btranscolor = dwbgcolorflag;    //0;
    destbmp.u_left = pbvscr->v_r.u_left;
    destbmp.u_top = pbvscr->v_r.u_top;
    destbmp.u_width = pbvscr->v_r.u_width;
    destbmp.u_height = pbvscr->v_r.u_height;
    destbmp.p_data = pbvscr->lpb_scr;
    destbmp.stride = OSD_GET_VSCR_STIDE(pbvscr);
    destbmp.bits_per_pix = BIT_PER_PIXEL;

    mp_subtitle_osd_bitmap_format_transfer
    (&destbmp, &srcbmp, btranscolor, bforgcolor);
#else
    btranscolor = dwbgcolorflag;
    //libc_printf("srcbmp.uLeft=%d,srcbmp.uTop=%d,
    //srcbmp.uWidth=%d,srcbmp.uHeight=%d\n",
    //srcbmp.uLeft,srcbmp.uTop,srcbmp.uWidth,srcbmp.uHeight);
    draw_sub_char_matrix(subtitle_osd, srcbmp.u_left,
              srcbmp.u_top, srcbmp.u_width, srcbmp.u_height,
              srcbmp.p_data, srcbmp.stride, btranscolor, bforgcolor);
#endif
}
#endif

#if 0
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_char
//
// Description:
//      Draw char on subtitle osd region, for debug
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      x - Offset x
//      y - Offset y
//      width - Char width
//      height - Char height
//      character - Char
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_draw_char(mp_subtitle_osd  *subtitle_osd,
                      unsigned short x, unsigned short y,
                      unsigned char fg_color,
                      unsigned short character)
{
    //unsigned char                     *pData=NULL;
    struct osdrect r;
    short width = 0;
    short height = 0;

    //unsigned char                     charset_idx, char_idx;
    //unsigned long                     charmask;
    //unsigned short                    bit_count;
    //unsigned char                     i, j, k;
    //unsigned char                     data1, data2;

    UINT16 temp_x = 0;
    UINT16 temp_y = 0;
    unsigned char *char_addr = NULL;
    VSCR mp_subtitle_vscr;
    ID_RSC resource_id = 0;
    OBJECTINFO object_info;

    MEMSET(&r, 0, sizeof (struct osdrect));
    MEMSET(&mp_subtitle_vscr, 0, sizeof (VSCR));
    MEMSET(&object_info, 0, sizeof (OBJECTINFO));

    MP_SUB_DEBUG("====>>mp_subtitle_osd_draw_char()\n");

    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_draw_char: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
    return;
    }
/*
    if((x%MP_SUBTITLE_OSD_CHAR_W) != 0 ||\
      (y%MP_SUBTITLE_OSD_CHAR_H) != 0 ||\
       x+MP_SUBTITLE_OSD_CHAR_W > MP_SUBTITLE_OSD_WIDTH)
    {
        MP_SUB_DEBUG("mp_subtitle_osd_draw_char: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
        return ;
    }

    if(y+MP_SUBTITLE_OSD_CHAR_H > MP_SUBTITLE_OSD_HEIGHT)
    {
        MP_SUB_DEBUG("mp_subtitle_osd_draw_char: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
        return;
    }
*/

    //character = ComMB16ToUINT16((char*)(&character));
    resource_id = osd_get_default_font_lib(character);
    char_addr = (unsigned char *)osd_get_rsc_obj_data
    (resource_id, character, &object_info);
    if (char_addr == NULL)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_draw_char: We can find this char!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
    return;
    }

    width = object_info.m_obj_attr.m_w_actual_width;
    height = object_info.m_obj_attr.m_w_height;

    //set mp_subtitle_vscr size
    temp_x = (UINT16) MP_SUBTITLE_OSD_XOFFSET_TEMP;
    temp_x = temp_x >> 1;
    temp_y = (UINT16) MP_SUBTITLE_OSD_YOFFSET_TEMP;
    temp_y = temp_y >> 1;
    //set mp_subtitle_vscr size
    r.u_left = x + temp_x;
    r.u_top = y + temp_y;
    r.u_width = width;
    r.u_height = height;

    mp_subtitle_vscr.v_r.u_left = r.u_left;
    mp_subtitle_vscr.v_r.u_top = r.u_top;
    mp_subtitle_vscr.v_r.u_width = width;
    mp_subtitle_vscr.v_r.u_height = height;
    mp_subtitle_vscr.lpb_scr = subtitle_osd->vscr_buffer;

    //mp_subtitle_osd_draw_bmp(char_addr, &object_info,&mp_subtitle_vscr, &r, resource_id, fg_color);
    mp_subtitle_osd_draw_bmp(subtitle_osd, char_addr,
                 &object_info, &mp_subtitle_vscr, &r, resource_id,
                 fg_color);

    // write Vscr data to osd frame buffer
    osddrv_region_write((HANDLE) subtitle_osd->osd_dev,
               subtitle_osd->region, &mp_subtitle_vscr, &r);

    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
}
#endif
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_char2
//
// Description:
//      Draw char on subtitle osd region
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      char_addr - Bitmap of the char
//      object_info - ?
//      r - Pointer to struct OSDRect
//      resource_id - Resource in project's resource folder
//      fg_color - Color of the char
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
#ifndef MP_SUBTITLE_SETTING_SUPPORT
static void mp_subtitle_osd_draw_char2(mp_subtitle_osd  *subtitle_osd,
                       unsigned char *char_addr,
                       OBJECTINFO  *object_info,
                       struct osdrect *r, ID_RSC resource_id,
                       unsigned char fg_color)
{
    VSCR mp_subtitle_vscr;

    MP_SUB_DEBUG("====>>mp_subtitle_osd_draw_char()\n");

    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_draw_char: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
    return;
    }

    mp_subtitle_vscr.v_r.u_left = r->u_left;
    mp_subtitle_vscr.v_r.u_top = r->u_top;
    mp_subtitle_vscr.v_r.u_width = r->u_width;
    mp_subtitle_vscr.v_r.u_height = r->u_height;
    mp_subtitle_vscr.b_color_mode = OSD_256_COLOR;
    mp_subtitle_vscr.lpb_scr = subtitle_osd->vscr_buffer;
    MEMSET(subtitle_osd->vscr_buffer, 0, 100 * 100);

    mp_subtitle_osd_draw_bmp(subtitle_osd,
                 char_addr, object_info, &mp_subtitle_vscr,
                 r, resource_id, fg_color);

    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
}

#else
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_osd_draw_char3
//
// Description:
//      Draw char on subtitle osd region
//
// Arguments:
//      subtitle_osd - Pointer to a struct contains osd infomation
//      char_addr - Bitmap of the char
//      object_info - ?
//      r - Pointer to struct OSDRect
//      resource_id - Resource in project's resource folder
//      fg_color - Color of the char
//      bg_color - Color of background
//
// Return Value:
//      None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_osd_draw_char3(mp_subtitle_osd  *subtitle_osd,
                       unsigned char *char_addr,
                       OBJECTINFO  *object_info,
                       struct osdrect *r, ID_RSC resource_id,
                       unsigned char fg_color,
                       unsigned char bg_color)
{
    VSCR mp_subtitle_vscr;

    MEMSET(&mp_subtitle_vscr, 0 ,sizeof(mp_subtitle_vscr));
    MP_SUB_DEBUG("====>>mp_subtitle_osd_draw_char()\n");

    if (!subtitle_osd)
    {
    MP_SUB_DEBUG("mp_subtitle_osd_draw_char: Invalid Parameters!\n");
    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
    return;
    }

    mp_subtitle_vscr.v_r.u_left = r->u_left;
    mp_subtitle_vscr.v_r.u_top = r->u_top;
    mp_subtitle_vscr.v_r.u_width = r->u_width;
    mp_subtitle_vscr.v_r.u_height = r->u_height;
    mp_subtitle_vscr.b_color_mode = OSD_256_COLOR;
    mp_subtitle_vscr.lpb_scr = (UINT8 *)subtitle_osd->vscr_buffer;
    MEMSET(subtitle_osd->vscr_buffer, 0, 100 * 100);

    //mp_subtitle_osd_draw_bmp(char_addr, object_info,
    //&mp_subtitle_vscr, r, resource_id, fg_color);
    mp_subtitle_osd_draw_bmp3(subtitle_osd, char_addr,
                  object_info, &mp_subtitle_vscr, r, resource_id,
                  fg_color, bg_color);

    // write Vscr data to osd frame buffer
    osddrv_region_write((HANDLE) subtitle_osd->osd_dev,
               subtitle_osd->region, &mp_subtitle_vscr, r);

    MP_SUB_DEBUG("<<====mp_subtitle_osd_draw_char()\n");
}
#endif
