/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_atsc_osd.c
   *
   *    Description: The file is mainly to display the ATSC SUBTITLE
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>

#include <api/libsubt/lib_subt.h>
#include <api/libsubt/subt_osd.h>
#include <hld/sdec/sdec.h>

#include <hld/osd/osddrv_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/dis/vpo.h>

#include "lib_atsc_subt.h"
#include "lib_subt_atsc_osd.h"
#include "lib_subt_internal.h"

//#define SUBT_ATSC_OSD_PRINTF libc_printf
#define SUBT_ATSC_OSD_PRINTF(...)

//BOOL shifty_subt_auto = FALSE;
static BOOL b_center = FALSE;
static BOOL user_force = FALSE;
static INT32 shifty_dcii_subt = 0;
static BOOL b_color_exchange = FALSE;
static UINT8 char_y = 0xeb;
static UINT8 char_cb = 0x80;
static UINT8 char_cr = 0x80;
static UINT8 char_k = 0xF;
static UINT8 outline_y = 0x16;
static UINT8 outline_cb = 0x80;
static UINT8 outline_cr = 0x80;
static UINT8 outline_k = 0xF;
//static BOOL osd_region_deleted = TRUE;

struct osd_device *lib_subt_atsc_get_osd_handle(void)
{
    struct osd_device *ret = NULL;

    #ifdef SHOW_BY_CPU
        ret = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    #else
        ret = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,g_subt_config_par.osd_layer_id);
    #endif
    //struct osd_device* ret = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    return ret;
}

RET_CODE lib_subt_atsc_create_region(struct osd_device *subt_atsc_osd_dev,
   struct osdrect *rect, DISPLAY_STANDARD_TYPE type)
{
    enum osdsys  e_osd_sys = OSD_NTSC;
    enum tvsystem tvsys = NTSC;
    RET_CODE ret = SUCCESS;
    UINT32 param = 0;
    RET_CODE ret_func = 0;
    struct vpo_device *vpo_dev = NULL;
    UINT32 screen_width = 0;

    if((NULL == subt_atsc_osd_dev) || (NULL == rect))
    {
        return ERR_FAILURE;
    }

    switch(type)
    {
        default:
            break;
        case TYPE_720_576_25:
            break;
        case TYPE_720_480_30:
        case TYPE_1280_720_60:
        case TYPE_1920_1080_60:
            e_osd_sys = OSD_PAL;
        break;
    }
//    e_osd_sys = PAL;
    do
    {
        //SUBT_ATSC_OSD_PRINTF("__%d__ l %d t%d w %d h %d\n",__LINE__,
        //rect->uLeft,rect->uTop,rect->uWidth,rect->uHeight);
        ret = osddrv_create_region((HANDLE)subt_atsc_osd_dev,0,rect,NULL);
        if(SUCCESS != ret)
        {
            break;
        }
        osd_status = 1;
       // osd_region_deleted = FALSE;
       	vpo_dev =(struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
        ret_func = vpo_ioctl(vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
		if(RET_FAILURE==ret_func)
			return ERR_FAILURE;
#ifdef HDTV_SUPPORT
        if((NTSC==tvsys)||(PAL_M==tvsys)||(PAL_60==tvsys)||
            (NTSC_443==tvsys)||(LINE_720_30==tvsys)||(LINE_1080_30==tvsys))
        {
            e_osd_sys = OSD_NTSC;
        }
        else
        {
            e_osd_sys = OSD_PAL;
        }
#else
        if((NTSC==tvsys)||(PAL_M==tvsys)||(PAL_60==tvsys)||(NTSC_443==tvsys))
            e_osd_sys = OSD_NTSC;
        else
            e_osd_sys = OSD_PAL;
#endif
        if(g_subt_config_par.osd_get_scale_para != NULL) //true color hd osd
        {
        	  switch(type)
				    {
				        default:
				            break;
				        case TYPE_720_576_25:
				            screen_width = 720; 
				            break;
				        case TYPE_720_480_30:	
				            screen_width = 480; 
				            break;
					    case TYPE_1280_720_60:
				         screen_width = 1280; 
				            break;
					    case TYPE_1920_1080_60:
				         screen_width = 1920; 
				            break;
				    }
            param = g_subt_config_par.osd_get_scale_para(tvsys, screen_width);
            ret = osddrv_scale((HANDLE)subt_atsc_osd_dev,OSD_SCALE_WITH_PARAM,param);
        }
        else
        {
            ret = osddrv_scale((HANDLE)subt_atsc_osd_dev,OSD_VSCALE_TTX_SUBT,(UINT32)&e_osd_sys);
        }
        if(SUCCESS != ret)
        {
            break;
        }

        ret = osddrv_show_on_off((HANDLE)subt_atsc_osd_dev,OSDDRV_ON);
        if(SUCCESS != ret)
        {
            break;
        }
    }while(0);
    return ret;
}

//Clear OSD Display (Show Off and delete regionid 0)
RET_CODE lib_subt_atsc_clear_osd(struct osd_device *subt_atsc_osd_dev)
{
    RET_CODE ret = SUCCESS;

    if(NULL == subt_atsc_osd_dev)
    {
        return ERR_FAILURE;
    }

    do
    {
        ret = osddrv_show_on_off((HANDLE)subt_atsc_osd_dev,OSDDRV_OFF);
        if(SUCCESS != ret)
        {
            break;
        }

        ret = osddrv_delete_region((HANDLE)subt_atsc_osd_dev,0);
        if(SUCCESS != ret)
        {
            break;
        }
        osd_status = 0;
    }while(0);
    return ret;
}


//Fill color specified to rectangle specified
void lib_subt_atsc_display(struct osdrect *rect,atsc_subt_bmp_header *bmp_header,UINT16 color)
{
    #ifdef SHOW_BY_CPU

    struct osd_device *subt_atsc_osd_dev= lib_subt_atsc_get_osd_handle();

    #endif
#if 1
    UINT8 *subt_atsc_vscreen =g_subt_config_par.g_ps_buf_addr;
    int x = 0;
    int y = 0;

    if((NULL == rect)||(NULL == bmp_header)||(0xffff<=color))
    {
        return;
    }

    do
    {
        if(rect->u_left + rect->u_width >= g_subt_atsc_rect.u_left+ g_subt_atsc_rect.u_width)
        {
             ;
             SUBT_ATSC_OSD_PRINTF("width error\n");
        }
        if(rect->u_top+ rect->u_height>= g_subt_atsc_rect.u_top+g_subt_atsc_rect.u_height)
        {
             ;
             SUBT_ATSC_OSD_PRINTF("height error\n");
        }

    #ifdef SHOW_BY_CPU
        osddrv_region_fill((HANDLE)subt_atsc_osd_dev,0,rect,color);
    #else
         for(y = rect->u_top; y<rect->u_top + rect->u_height; ++y)
         {
             for(x = rect->u_left; x<rect->u_left + rect->u_width; ++x)
             {
                *(subt_atsc_vscreen+x+y*g_subt_atsc_rect.u_width) = color;
             }
         }
    #endif
    }while(0);
#endif
    return;
}

void lib_subt_atsc_show_screen(void)
{
    UINT8 *subt_atsc_vscreen =g_subt_config_par.g_ps_buf_addr;
    struct osdrect rect;
    struct osdrect rect1;
    UINT16 amend_height_real = 0;
    RET_CODE ret_func = 0;
    struct osd_device *subt_atsc_osd_dev = NULL;

    MEMSET(&rect1,0,sizeof(struct osdrect));
    MEMSET(&rect,0,sizeof(struct osdrect));
    rect.u_top = 0;
    rect.u_left = 0;
    rect.u_height = g_subt_atsc_rect.u_height;
    rect.u_width = g_subt_atsc_rect.u_width;
    amend_height_real = globe_height_real+0xf;
    if(b_center)
    {
        rect1.u_top = 0;
        rect1.u_left =(g_subt_atsc_rect.u_width-globe_width_real)>>1;
        rect1.u_height = amend_height_real;//g_subt_atsc_rect.uHeight;
        rect1.u_width = globe_width_real+0xf;
        if(shifty_dcii_subt<=0)
        {
            rect1.u_top = 0;
        }
        else
        {
            rect1.u_top = shifty_dcii_subt;
        }
        if(FALSE == user_force)
        {
           if(rect1.u_top+amend_height_real>=g_subt_atsc_rect.u_height)
           {
               rect1.u_top = g_subt_atsc_rect.u_height-amend_height_real;
           }
        }
        else
        {
           if(rect1.u_top+amend_height_real>=g_subt_atsc_rect.u_height)
           {
               rect1.u_height = g_subt_atsc_rect.u_height -rect1.u_top;
               if(rect1.u_height<=0)
               {
                   rect1.u_height = 0;
               }
           }
        }
    }
    else
    {
        rect1.u_top = 0;
        rect1.u_left = 0;
        rect1.u_height = g_subt_atsc_rect.u_height;
        rect1.u_width = g_subt_atsc_rect.u_width;
    }
    SUBT_ATSC_OSD_PRINTF("Subt_Area:L(%d),T(%d),W(%d),H(%d)\n",
                        g_subt_atsc_rect.u_left,g_subt_atsc_rect.u_top,
                        g_subt_atsc_rect.u_width,g_subt_atsc_rect.u_height);

    subt_atsc_osd_dev = lib_subt_atsc_get_osd_handle();

#ifdef SHOW_BY_CPU
    //OSDDrv_ShowOnOff((HANDLE)subt_atsc_osd_dev, OSDDRV_ON);
    ret_func = osddrv_region_show((HANDLE)subt_atsc_osd_dev,0,TRUE);
#else
    ret_func = osddrv_region_write2((HANDLE)subt_atsc_osd_dev, 0, subt_atsc_vscreen,
    g_subt_atsc_rect.u_width, g_subt_atsc_rect.u_height,&rect, &rect1);
	ret_func=(ret_func==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
#endif
    return;
}
// get rectangle according to display standard
BOOL lib_subt_atsc_get_osd_rect(DISPLAY_STANDARD_TYPE type, struct osdrect *rect)
{
    BOOL ret = FALSE;
    UINT16 h_limited = 0;
    UINT16 v_limited = 0;

    do
    {
        switch(type)
        {
            default:
                ret = FALSE;
                break;
            case TYPE_720_480_30:
                h_limited = 720;
                v_limited = 480;
                //libc_printf("1\n");
                break;
            case TYPE_720_576_25:
                h_limited = 720;
                v_limited = 576;
                //libc_printf("2\n");
                break;
            case TYPE_1280_720_60:
                h_limited = 1280;
                v_limited = 720;
                //libc_printf("3\n");
                break;
            case TYPE_1920_1080_60:
                h_limited = 1920;
                v_limited = 1080;
                //libc_printf("4\n");
                break;
        }

        if((rect->u_top >= v_limited)||(rect->u_top < 0)|| (rect->u_height< 0) )
        {
           // SDBBP();
		   SUBT_ATSC_OSD_PRINTF("rect's top is invalid\n");
            break;
        }
         if((rect->u_left>= h_limited)|| (rect->u_left < 0) ||(rect->u_width < 0))
        {
           // SDBBP();
		   SUBT_ATSC_OSD_PRINTF("rect's left is invalid\n");
            break;
        }

        if((rect->u_height + rect->u_top > v_limited) &&(rect->u_top < v_limited))
        {
            SUBT_ATSC_OSD_PRINTF("height too large\n");
            rect->u_top = rect->u_top - (rect->u_top + rect->u_height - v_limited);
        }

        if( (rect->u_width+ rect->u_left> h_limited) && (rect->u_left< h_limited))
        {
            SUBT_ATSC_OSD_PRINTF("width too large %d %d %d %d\n",
                rect->u_width,rect->u_left,rect->u_width+rect->u_left,h_limited);
            rect->u_left =rect->u_left-(rect->u_width+ rect->u_left-h_limited)-32;
        }
        if(b_center)
        {
            rect->u_left = 0;
           // rect->u_top = (rect->u_top - 0xf) & 0xfffffff0;
			rect->u_top = (rect->u_top - 0xf) & 0xfff0;
            if(rect->u_top < 0)
            {
                rect->u_top = 0;
            }
            rect->u_width = h_limited - rect->u_left;
            rect->u_width = (rect->u_width + 0xf) & 0xfffffff0;
            rect->u_height= v_limited - rect->u_top;
        }
        else
        {
            rect->u_left = (h_limited - rect->u_width)>>1;
            if(rect->u_left < 0)
            {
                rect->u_left = 0;
            }
            rect->u_left = rect->u_left & 0xfffffff0;

            //rect->u_top = (rect->u_top - 0xf) & 0xfffffff0;
			rect->u_top = (rect->u_top - 0xf) & 0xfff0;
            if(rect->u_top < 0)
            {
                rect->u_top = 0;
            }
            rect->u_width = h_limited - rect->u_left;
            rect->u_width = (rect->u_width + 0xf) & 0xfffffff0;

            rect->u_height= v_limited - rect->u_top;
        }
        if( (rect->u_width+ rect->u_left> h_limited) && (rect->u_left< h_limited))
        {
            rect->u_width = h_limited- rect->u_left;
        }

        if( rect->u_width+ rect->u_left> h_limited)
        {
            break;
        }
        if((rect->u_width < 0) || (rect->u_height < 0) )
        {
            break;
        }
        ret = TRUE;
    }while(0);
    return ret;
}

//Begin:change DC2 Subtitle color and position set of the stream:add by Colin
void lib_subt_atsc_hcenter_on_off(BOOL b_on_off)
{
	if((TRUE!=b_on_off)&&(FALSE!=b_on_off))
	{
		return;
	}
     b_center = b_on_off;
}
/*
void lib_subt_atsc_shift_y_set(INT32 nShiftY)
{
    shifty_dcii_subt = nShiftY;
}
*/
void lib_subt_atsc_force_shift_y_set(BOOL b_user_force,INT32 n_shift_y)
{
	if(((TRUE!=b_user_force)&&(FALSE!=b_user_force))||(1080<=n_shift_y))
	{
		return;
	}	
    user_force = b_user_force;
    shifty_dcii_subt = n_shift_y;
}

void lib_subt_atsc_color_exchange_on_off(BOOL b_on_off)
{
	if((TRUE!=b_on_off)&&(FALSE!=b_on_off))
	{
		return;
	}
    b_color_exchange = b_on_off;
}

void lib_subt_atsc_color_exchange__color_char_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{
	if((0xff<=u_y)||(0xff<=u_cb)||(0xff<=u_cr)||(0xff<=u_k))
	{
		return;
	}
     char_y = u_y;
     char_cb = u_cb;
     char_cr = u_cr;
     char_k = u_k;
}

void lib_subt_atsc_color_exchange__color_outline_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k)
{
	if((0xff<=u_y)||(0xff<=u_cb)||(0xff<=u_cr)||(0xff<=u_k))
	{
		return;
	}
     outline_y = u_y;
     outline_cb = u_cb;
     outline_cr = u_cr;
     outline_k = u_k;
}
//End:change DC2 Subtitle color and position set of the stream

// Set Pallette
RET_CODE lib_subt_atsc_osd_set_pallette
         (struct osd_device *subt_atsc_osd_dev,atsc_subt_bmp_header *bmp_header)
{
    RET_CODE ret = 0;
	if((NULL==subt_atsc_osd_dev)||(NULL==bmp_header))
	{
		return RET_FAILURE;
	}
    do
    {
        /*ret = OSDDrv_SetPallette((HANDLE)subt_atsc_osd_dev,\
                                subt_atsc_palette,256,OSDDRV_YCBCR);
        */
        ret = osddrv_modify_pallette
              ((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_CHAR_COLOR_INDEX,\
                bmp_header->char_color.y_component*8, \
                bmp_header->char_color.cb_component*8,\
                bmp_header->char_color.cr_component*8,\
                bmp_header->char_color.opaque_enable?0xf:8);
        if(SUCCESS!= ret)
        {
            break;
        }
        ret = osddrv_modify_pallette
              ((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_FRAME_COLOR_INDEX,\
                bmp_header->bmp_frame.frame_color.y_component*8,\
                bmp_header->bmp_frame.frame_color.cb_component*8,\
                bmp_header->bmp_frame.frame_color.cr_component*8,\
                bmp_header->bmp_frame.frame_color.opaque_enable?0xf:8);
        if(SUCCESS!= ret)
        {
            break;
        }
        ret = osddrv_modify_pallette
              ((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_OUTLINE_COLOR_INDEX,\
                bmp_header->bmp_outline.outline_color.y_component*8,\
                bmp_header->bmp_outline.outline_color.cb_component*8,\
                bmp_header->bmp_outline.outline_color.cr_component*8,\
                bmp_header->bmp_outline.outline_color.opaque_enable?0xf:8);
        if(SUCCESS!= ret)
        {
            break;
        }
        ret = osddrv_modify_pallette
              ((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_SHADOW_COLOR_INDEX,\
                bmp_header->bmp_drop_shadow.shadow_color.y_component*8,\
                bmp_header->bmp_drop_shadow.shadow_color.cb_component*8,\
                bmp_header->bmp_drop_shadow.shadow_color.cr_component*8,\
                bmp_header->bmp_drop_shadow.shadow_color.opaque_enable?0xf:8);
        if(SUCCESS!= ret)
        {
            break;
        }
        if(b_color_exchange)
        {
            ret = osddrv_modify_pallette((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_CHAR_COLOR_INDEX,
                char_y, char_cb, char_cr,char_k);
            ret = osddrv_modify_pallette((HANDLE)subt_atsc_osd_dev,ATSC_SUBT_OUTLINE_COLOR_INDEX,
                outline_y, outline_cb,outline_cr,outline_k);
        }
    }while(0);
    return ret;
}

