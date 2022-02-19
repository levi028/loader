/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: subt_osd_no_bl.c
   *
   *    Description: The file is mainly to show DVB SUBTITLE bitmap on OSD.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>
#include <api/libsubt/lib_subt.h>
#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/dis/vpo.h>
#include "lib_subt_internal.h"

#define SUBT_PRINTF(...) do{}while(0)
//#define SUBT_PRINTF    libc_printf
#define SUBT_MAX_PALLETE_SIZE 256

#define ENTER_SUBT_OSD_API()    {m_in_subt_decoding = TRUE;}
#define LEAVE_SUBT_OSD_API()    {m_in_subt_decoding = FALSE;} //{osal_semaphore_release(l_subtosd_sema_id);}

static UINT32 g_ps_free = 0;
static struct osd_device *osd_dev = NULL;
static UINT8 *m_region_data_nobl[MAX_REGION_IN_PAGE];
static struct osdrect m_subt_region_nobl[MAX_REGION_IN_PAGE];
static BOOL m_used_region_nobl[MAX_REGION_IN_PAGE];
static BOOL m_shown_region_nobl[MAX_REGION_IN_PAGE];
static UINT16 m_total_height = 0;
static UINT16 m_subt_region_w = 0;
static UINT16  m_subt_region_h = 0;
static BOOL m_in_subt_decoding = FALSE;
static UINT16 region_offset_v = 0;
static unsigned char subt_palette[SUBT_MAX_PALLETE_SIZE][4];
static BOOL g_update_palette = FALSE;
static UINT32 subt_region_type = 0xFFFFFFFF;

static BOOL page_delete_flag=FALSE;
static BOOL page_clear_flag=FALSE;
BOOL g_subt_show_on = FALSE;

static INT32 osd_region_show(UINT8 region_idx, UINT8 on)
{
    struct osdrect r1;
    struct osdrect r2;
    struct osdrect r3;	
    struct osdrect rect;
	UINT32 u_src_width = 0;
    UINT32 u_src_height = 0;
    UINT32 vscr_width = 1920;
    UINT32 vscr_height = 1080;

    INT32 ret = ERR_FAILUE;
    UINT16 tmp = 0;
    UINT16 backup_old_pos = 0;

    if(region_idx >= MAX_REGION_IN_PAGE)
    {
        return RET_FAILURE;
    }
    MEMSET(&r1,0,sizeof(struct osdrect));
    MEMSET(&r2,0,sizeof(struct osdrect));
	MEMSET(&r3,0,sizeof(struct osdrect));
    MEMSET(&rect,0,sizeof(struct osdrect));
	u_src_width = m_subt_region_nobl[region_idx].u_left + m_subt_region_nobl[region_idx].u_width;
    u_src_height = m_subt_region_nobl[region_idx].u_top + m_subt_region_nobl[region_idx].u_height;

    if ((u_src_width > DVB_HD_SUBT_OSD_WIDTH) || (u_src_height > DVB_HD_SUBT_OSD_HEIGHT))
    {
        vscr_width = 1920;
        vscr_height = 1080;
    }
    else if ((u_src_width > DVB_SD_SUBT_OSD_WIDTH) || (u_src_height > DVB_SD_SUBT_OSD_HEIGHT))
    {
        vscr_width = 1280;
        vscr_height = 720;
    }
    else
    {
        vscr_width = 720;
        vscr_height = 576;
    }
	r3.u_left=0;
	r3.u_top=0;
	r3.u_height=vscr_height;
	r3.u_width=vscr_width;

    if(m_used_region_nobl[region_idx])
    {
          MEMCPY(&rect,&(m_subt_region_nobl[region_idx]),sizeof(struct osdrect));

        g_subt_config_par.subt_hor_offset = 0;
        g_subt_config_par.subt_ver_offset = 0;
        g_subt_config_par.subt_width = m_subt_region_w;
        g_subt_config_par.subt_height = m_subt_region_h;

        if(rect.u_left >= g_subt_config_par.subt_hor_offset)
        {
            r1.u_left = 0;
            r2.u_left = rect.u_left -g_subt_config_par.subt_hor_offset;
            r1.u_width = (rect.u_left+rect.u_width> g_subt_config_par.subt_hor_offset+g_subt_config_par.subt_width)? \
                ((g_subt_config_par.subt_hor_offset+g_subt_config_par.subt_width>rect.u_left)?
                (g_subt_config_par.subt_hor_offset+g_subt_config_par.subt_width-rect.u_left):0):rect.u_width;
            r2.u_width = r1.u_width;
        }
        else
        {
            r1.u_left = g_subt_config_par.subt_hor_offset-rect.u_left ;
            r2.u_left = 0;
            r1.u_width = (rect.u_left+rect.u_width> g_subt_config_par.subt_hor_offset+g_subt_config_par.subt_width)? \
                (g_subt_config_par.subt_width):((rect.u_left+rect.u_width>g_subt_config_par.subt_hor_offset)?
                (rect.u_left+rect.u_width-g_subt_config_par.subt_hor_offset):0);
            r2.u_width = r1.u_width;
        }
        if((TRUE == g_subt_config_par.hd_subtitle_support) && (rect.u_width<=ATSC_SUBT_OSD_WIDTH))
        {
            if(((ATSC_SUBT_OSD_WIDTH-r2.u_width)/DIVIDE_HALF)>g_subt_config_par.subt_hor_offset)
            {
                r2.u_left +=  g_subt_config_par.subt_hor_offset;
            }
            else
            {
                r2.u_left += ((ATSC_SUBT_OSD_WIDTH-r2.u_width)/DIVIDE_HALF);
            }
        }
        if(TRUE== g_subt_config_par.user_force_pos )                // User forcely set position of subtitle show
        {
            if((g_subt_config_par.user_force_set_hor_offset != 0xFF) &&
                (g_subt_config_par.user_force_set_hor_offset > g_subt_config_par.subt_ver_offset))
            {
                tmp = g_subt_config_par.user_force_set_hor_offset - g_subt_config_par.subt_hor_offset;
                if(tmp > r2.u_left)
                {
                    r2.u_left = tmp;
                }
                if(r2.u_left + r2.u_width > m_subt_region_w)
                {
                    r2.u_width = m_subt_region_w - r2.u_left;
                }
            }
        }

        if(rect.u_top >= g_subt_config_par.subt_ver_offset)
        {
            r1.u_top = 0;
            r2.u_top = rect.u_top -g_subt_config_par.subt_ver_offset;
            r1.u_height = (rect.u_top+rect.u_height> g_subt_config_par.subt_ver_offset+g_subt_config_par.subt_height)? \
                ((g_subt_config_par.subt_ver_offset+g_subt_config_par.subt_height>rect.u_top)?
                (g_subt_config_par.subt_ver_offset+g_subt_config_par.subt_height-rect.u_top):0):rect.u_height;
            r2.u_height = r1.u_height;
        }
        else
        {
            r1.u_top = g_subt_config_par.subt_ver_offset-rect.u_top ;
            r2.u_top = 0;
            r1.u_height = (rect.u_top+rect.u_height> g_subt_config_par.subt_ver_offset+g_subt_config_par.subt_height)? \
                (g_subt_config_par.subt_height):((rect.u_top+rect.u_height>g_subt_config_par.subt_ver_offset)?
                (rect.u_top+rect.u_height-g_subt_config_par.subt_ver_offset):0);
            r2.u_height = r1.u_height;
        }
        if((TRUE==g_subt_config_par.hd_subtitle_support)&&(rect.u_height<=ATSC_SUBT_OSD_HEIGHT))
        {
            if(((ATSC_SUBT_OSD_HEIGHT-r2.u_height)/DIVIDE_HALF)>g_subt_config_par.subt_ver_offset)
            {
                r2.u_top +=  g_subt_config_par.subt_ver_offset;
            }
            else
            {
                r2.u_top += ((ATSC_SUBT_OSD_HEIGHT-r2.u_height)/DIVIDE_HALF);
            }
        }

        backup_old_pos = r2.u_top;
        if( TRUE==g_subt_config_par.user_force_pos )
        {
            if( 0== region_idx )           // Only first region use pos that user set
        {
            if((g_subt_config_par.user_force_set_ver_offset != 0xFF) &&
                (g_subt_config_par.user_force_set_ver_offset > g_subt_config_par.subt_ver_offset))
            {
                tmp = g_subt_config_par.user_force_set_ver_offset - g_subt_config_par.subt_ver_offset;
                if(tmp > r2.u_height)
                {
                    r2.u_top = tmp;
                }
                if(r2.u_top + r2.u_height > m_subt_region_h)
                {
                    r2.u_height = m_subt_region_h - r2.u_top;
                }
                region_offset_v = r2.u_top - backup_old_pos;         // calculate offset of first region
                }
            }
            else                                    // left regions will use region offset
            {
                r2.u_top = r2.u_top + region_offset_v;               // The left regions except first region offset
                if(r2.u_top + r2.u_height > m_subt_region_h)
                {
                    r2.u_height = m_subt_region_h - r2.u_top;
                }
            }
        }

        if(on)
        {
            if(( FALSE== g_subt_config_par.hd_subtitle_support) && (rect.u_width>DVB_SD_SUBT_OSD_WIDTH))
            {
                return RET_FAILURE;
            }
            //m_ShownRegion_nobl[region_idx] = TRUE;
            if((region_idx==0)&&(page_delete_flag)&&(!page_clear_flag))
				ret = osddrv_region_fill((HANDLE)osd_dev,0,&r3,OSD_TRANSPARENT_COLOR);
            ret = osddrv_region_write2((HANDLE)osd_dev,0,m_region_data_nobl[region_idx],rect.u_width,rect.u_height,&r1,&r2);
        }
        else
        {
             //m_ShownRegion_nobl[region_idx] = FALSE;
               ret = osddrv_region_fill((HANDLE)osd_dev,0,&r2,OSD_TRANSPARENT_COLOR);
        }
    }
    return ret;
}

static INT32 osd_region_fill_function(UINT8 region_idx,UINT8 u_color_data)
{
    if((region_idx >= MAX_REGION_IN_PAGE)||(0xff!=u_color_data))
    {
        return RET_FAILURE;
    }
    MEMSET(m_region_data_nobl[region_idx], u_color_data,
        (m_subt_region_nobl[region_idx].u_width*m_subt_region_nobl[region_idx].u_height));

    if(m_shown_region_nobl[region_idx])
    {
        return osd_region_show(region_idx, TRUE);
    }

    return RET_SUCCESS;
}


static UINT8 osd_subt_del_region(UINT8 region_idx)
{
    //INT32 func_ret = 0 ;

    if(region_idx >= MAX_REGION_IN_PAGE)
    {
    	SUBT_PRINTF("region_id is too large: %d\n",region_idx);
        return SUBT_RET_FAILURE;
    }
    if(!m_used_region_nobl[region_idx])
    {
        SUBT_PRINTF("Delete Null region %d\n",region_idx);
        return SUCCESS;
    }
    SUBT_PRINTF("Delete region %d\n",region_idx);
	page_clear_flag=FALSE;
    MEMSET(m_region_data_nobl[region_idx], OSD_TRANSPARENT_COLOR,(m_subt_region_nobl[region_idx].u_width*m_subt_region_nobl[region_idx].u_height));
    m_used_region_nobl[region_idx] = FALSE;
    m_shown_region_nobl[region_idx] = FALSE;
    g_ps_free -= m_subt_region_nobl[region_idx].u_width*m_subt_region_nobl[region_idx].u_height;
    m_total_height -= m_subt_region_nobl[region_idx].u_height;

    return SUCCESS;
}

static UINT8 osd_subt_clear_region(UINT8 region_idx)
{
    INT32 func_ret = 0 ;

    if(region_idx >= MAX_REGION_IN_PAGE)
    {
    	SUBT_PRINTF("region_id is too large: %d\n",region_idx);
        return SUBT_RET_FAILURE;
    }
    if(!m_used_region_nobl[region_idx])
    {
        SUBT_PRINTF("Delete Null region %d\n",region_idx);
        return SUCCESS;
    }
	page_clear_flag=TRUE;
    SUBT_PRINTF("Delete region %d\n",region_idx);
    func_ret = osd_region_fill_function(region_idx, OSD_TRANSPARENT_COLOR);
	if(RET_FAILURE==func_ret)
	 	return SUBT_RET_FAILURE;
    m_used_region_nobl[region_idx] = FALSE;
    m_shown_region_nobl[region_idx] = FALSE;
    g_ps_free -= m_subt_region_nobl[region_idx].u_width*m_subt_region_nobl[region_idx].u_height;
    m_total_height -= m_subt_region_nobl[region_idx].u_height;

    return SUCCESS;
}

BOOL osd_region_is_created_no_bl(UINT8 region_idx)
{

    ENTER_SUBT_OSD_API();
    if( (FALSE== g_subt_show_on)||(MAX_REGION_IN_PAGE<=region_idx))
    {
        LEAVE_SUBT_OSD_API();
        return FALSE;
    }
    LEAVE_SUBT_OSD_API();
    return m_used_region_nobl[region_idx];
}

static BOOL osd_subt_scale_no_bl(UINT16 screen_width)
{
    enum osdsys  e_osd_sys = OSD_PAL;
    enum tvsystem tvsys = PAL;
    RET_CODE ret_func = 0;
    UINT32 param = 0;

	if(1920<screen_width)
    {
        return FALSE;
    }
     ret_func = vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),
        VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
	 
	 if(RET_FAILURE==ret_func)
	 	return FALSE;

#ifdef HDTV_SUPPORT
    if((  NTSC==tvsys)||( PAL_M==tvsys)||( PAL_60==tvsys)||( NTSC_443==tvsys)||
        ( LINE_720_30==tvsys)||( LINE_1080_30==tvsys))
    {
        e_osd_sys = OSD_NTSC;
    }
    else
    {
        e_osd_sys = OSD_PAL;
    }
#else
    if(( NTSC==tvsys)||( PAL_M==tvsys)||( PAL_60==tvsys)||( NTSC_443==tvsys))
    {
        e_osd_sys = OSD_NTSC;
    }
    else
    {
        e_osd_sys = OSD_PAL;
    }
#endif

    if(g_subt_config_par.osd_get_scale_para != NULL) //true color hd osd
    {
        param = g_subt_config_par.osd_get_scale_para(tvsys, screen_width);
        ret_func =osddrv_scale((HANDLE)osd_dev,OSD_SCALE_WITH_PARAM,param);
		if(RET_FAILURE==ret_func)
	 		return FALSE;
    }
    else
    {
        ret_func =osddrv_scale((HANDLE)osd_dev,OSD_VSCALE_TTX_SUBT,(UINT32)(&e_osd_sys));
		if(RET_FAILURE==ret_func)
	 		return FALSE;
    }

    return TRUE;
}

static BOOL osd_subt_adjust_region_no_bl(UINT8 region_idx, struct osdrect *p_region)
{
    RET_CODE ret = 0;
    UINT32 u_src_width = 0;
    UINT32 u_src_height = 0;
    UINT32 vscr_width = 1920;
    UINT32 vscr_height = 1080;
	struct osdrect rect;
    osd_region_param region_param;
	BOOL ret_func = 0;
	
    if((region_idx >= MAX_REGION_IN_PAGE)||(NULL==p_region))
    {
        return FALSE;
    }

    MEMSET(&region_param,0,sizeof(osd_region_param));
	MEMSET(&rect,0,sizeof(struct osdrect));
	
	
    u_src_width = p_region->u_left + p_region->u_width;
    u_src_height = p_region->u_top + p_region->u_height;

    if ((u_src_width > DVB_HD_SUBT_OSD_WIDTH) || (u_src_height > DVB_HD_SUBT_OSD_HEIGHT))
    {
        vscr_width = 1920;
        vscr_height = 1080;
    }
    else if ((u_src_width > DVB_SD_SUBT_OSD_WIDTH) || (u_src_height > DVB_SD_SUBT_OSD_HEIGHT))
    {
        vscr_width = 1280;
        vscr_height = 720;
    }
    else
    {
        vscr_width = 720;
        vscr_height = 576;
    }
	rect.u_left=0;
	rect.u_top=0;
	rect.u_height=vscr_height;
	rect.u_width=vscr_width;

    if ((vscr_width > m_subt_region_w )||( vscr_height > m_subt_region_h))
    {
        region_param.region_id = region_idx;

        ret = osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_GET_REGION_INFO, (UINT32)&region_param);
        if (ret != RET_SUCCESS)
        {
            return FALSE;
        }

        region_param.region_x = 0;
        region_param.region_y = 0;
        region_param.region_w = vscr_width;
        region_param.region_h = vscr_height;
        region_param.bitmap_x = 0;
        region_param.bitmap_y = 0;
        region_param.bitmap_w = vscr_width;
        region_param.bitmap_h = vscr_height;
        region_param.pixel_pitch = vscr_width;
        region_param.bitmap_addr = 0;
        ret = osddrv_delete_region((HANDLE)osd_dev, region_idx);
        if (ret != RET_SUCCESS)
        {
            // SDBBP();
            return FALSE;
        }
        ret = osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_CREATE_REGION, (UINT32)&region_param);
        if (ret != RET_SUCCESS)
        {
            // SDBBP();
            return FALSE;
        }
        m_subt_region_w = vscr_width;
        m_subt_region_h = vscr_height;

        ret_func=osd_subt_scale_no_bl(m_subt_region_w);
		if(ret_func!=TRUE)
		{
			return FALSE;
		}
		ret=osddrv_region_fill((HANDLE)osd_dev,0,&rect,OSD_TRANSPARENT_COLOR);
		if (ret != RET_SUCCESS)
        {
           
            return FALSE;
        }
		ret=osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_ON);
		if (ret != RET_SUCCESS)
        {
           
            return FALSE;
        }
    }

    return TRUE;
}

__ATTRIBUTE_RAM_
INT32 osd_subt_create_region_no_bl(UINT8 region_idx, struct osdrect *p_region)
{
    UINT8 i = 0;
    UINT8 ret_func = 0;
    //RET_CODE ret_func1 = 0;


    ENTER_SUBT_OSD_API();
    if( (FALSE== g_subt_show_on)||(region_idx >= MAX_REGION_IN_PAGE)||(NULL == p_region))
    {
        LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    SUBT_PRINTF("create region %d, x:%d,y:%d,w:%d,h:%d",region_idx,p_region->u_left,
        p_region->u_top,p_region->u_width,p_region->u_height);

    if ((g_subt_config_par.hd_subtitle_support) && (sys_ic_get_chip_id() >= ALI_S3602F))
    {
        ret_func = osd_subt_adjust_region_no_bl(region_idx, p_region);
		if(FALSE==ret_func)
		{	
			LEAVE_SUBT_OSD_API();
        	return ERR_FAILUE;
		}
    }

    // delete overlap region
    for(i = 0; i < MAX_REGION_IN_PAGE; i++)
    {

        if(m_used_region_nobl[i])
        {
           // if(m_SubtRegion_nobl[i].uTop+m_SubtRegion_nobl[i].uHeight-1 >= p_region->uTop)
           //Some special stream may have 1 pixel region overlap, but they must be showed.
           if(m_subt_region_nobl[i].u_top+m_subt_region_nobl[i].u_height-1 > p_region->u_top)
            {
                 if((m_subt_region_nobl[i].u_top <= p_region->u_top) ||
                     (m_subt_region_nobl[i].u_top <= (p_region->u_top+p_region->u_height-1)))
                 {
                    SUBT_PRINTF("region overlap\n");
                    //ret_func = osd_subt_del_region(i);
                     p_region->u_top=m_subt_region_nobl[i].u_top+m_subt_region_nobl[i].u_height;
                 }
            }
        }
    }

    if(m_total_height+p_region->u_height >= g_subt_config_par.max_subt_height)
    {
        SUBT_PRINTF("Total Height overflow\n");
        for(i = 0; i < MAX_REGION_IN_PAGE; i++)
        {
            if(m_used_region_nobl[i])
            {
                ret_func = osd_subt_del_region(i);
				if(SUCCESS!=ret_func)
				{	
					LEAVE_SUBT_OSD_API();
					return ERR_FAILUE;
				}
            }
        }
    }

    if(m_total_height+p_region->u_height >= g_subt_config_par.max_subt_height)
    {
        SUBT_PRINTF("too big subt region\n");
        LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    if(m_used_region_nobl[region_idx])
    {
        SUBT_PRINTF("region %d already exists, delete it\n",region_idx);
        ret_func = osd_subt_del_region(region_idx);
		if(SUCCESS!=ret_func)
		{	
			LEAVE_SUBT_OSD_API();
			return ERR_FAILUE;
		}
    }

    //To fix bug reported by E-TEK, subtitle's width is not 16-byte align
    p_region->u_width = ((p_region->u_width+0xf)&0xfffffff0);

    /*if(region_idx==0)
        m_RegionData_nobl[region_idx] = g_ps_buf_addr;
    else
    {
        m_RegionData_nobl[region_idx] = m_RegionData_nobl[region_idx-1]+
        (m_SubtRegion_nobl[region_idx-1].uWidth*m_SubtRegion_nobl[region_idx-1].uHeight);
        if( m_RegionData_nobl[region_idx]+(p_region->uWidth*p_region->uHeight)>g_ps_buf_addr+g_ps_buf_len )
        {
            m_RegionData_nobl[region_idx] = g_ps_buf_addr;
            SUBT_PRINTF("no enough buf to create region, width:%d,height:%d",p_region->uWidth,p_region->uHeight);
            return ERR_FAILUE;
        }
    }*/
    if(g_ps_free+p_region->u_width*p_region->u_height > g_subt_config_par.g_ps_buf_len)
    {
        m_region_data_nobl[region_idx] = g_subt_config_par.g_ps_buf_addr;
        SUBT_PRINTF("no enough buf to create region, width:%d,height:%d",p_region->u_width,p_region->u_height);
        return ERR_FAILUE;
    }
    m_region_data_nobl[region_idx] = g_subt_config_par.g_ps_buf_addr+g_ps_free;
    g_ps_free += p_region->u_width*p_region->u_height;
    m_used_region_nobl[region_idx] = TRUE;
    m_shown_region_nobl[region_idx] = FALSE;
    m_subt_region_nobl[region_idx].u_left = p_region->u_left;
    m_subt_region_nobl[region_idx].u_width = p_region->u_width;
    m_subt_region_nobl[region_idx].u_top = p_region->u_top;
    m_subt_region_nobl[region_idx].u_height = p_region->u_height;
    m_total_height += m_subt_region_nobl[region_idx].u_height;
    //ret_func1 = osd_region_fill_function(region_idx, OSD_TRANSPARENT_COLOR);
	MEMSET(m_region_data_nobl[region_idx], OSD_TRANSPARENT_COLOR,(m_subt_region_nobl[region_idx].u_width*m_subt_region_nobl[region_idx].u_height));
    LEAVE_SUBT_OSD_API();
    return SUCCESS;
}

void osd_subt_enter_no_bl(void)
{
    struct osdpara    t_open_para;
    struct osdrect t_open_rect;
    struct osdrect r;

   // enum osdsys  e_osd_sys = OSD_PAL;
    enum tvsystem tvsys = PAL;
    int i = 0;
    RET_CODE ret_func = 0;
    BOOL ret_func1 = TRUE;
    struct vpo_device *vpo_dev = NULL;

    MEMSET(&t_open_para,0,sizeof(struct osdpara));
    MEMSET(&t_open_rect,0,sizeof(struct osdrect));
    MEMSET(&r,0,sizeof(struct osdrect));
    if( TRUE== g_subt_show_on)
    {
        return;
    }

    if((NULL==g_subt_config_par.g_ps_buf_addr) || ( 0== g_subt_config_par.g_ps_buf_len))
    {
        return;
    }

    SUBT_PRINTF("lib subtitle enter.\n");

    //soc_printf("osd_subt_enter_no_bl\n");
    vpo_dev = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
	if(NULL==vpo_dev)
	{
		return;
	}
    ret_func =vpo_ioctl(vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}

#if 0//def HDTV_SUPPORT
    if(( NTSC== tvsys)||( PAL_M==tvsys)||(PAL_60==tvsys)||( NTSC_443==tvsys)||
        ( LINE_720_30==tvsys)||( LINE_1080_30==tvsys))
    {
        e_osd_sys = OSD_NTSC;
    }
    else
    {
        e_osd_sys = OSD_PAL;
    }
//#else
    if(( NTSC==tvsys)||( PAL_M==tvsys)||( PAL_60==tvsys)||(NTSC_443==tvsys))
    {
        e_osd_sys = OSD_NTSC;
    }
    else
    {
        e_osd_sys = OSD_PAL;
    }
#endif

#if (COLOR_N==4)
    t_open_para.e_mode = OSD_4_COLOR;
#elif (COLOR_N==16)
    t_open_para.e_mode = OSD_16_COLOR;
#elif (COLOR_N==256)
    t_open_para.e_mode = OSD_256_COLOR;
#endif
    t_open_para.u_galpha_enable = 0;
    t_open_para.u_galpha = 0x0f;

    m_subt_region_w = 720;
    m_subt_region_h = 576;

    if(g_subt_config_par.hd_subtitle_support)
    {
        t_open_rect.u_left = 0;//SUBT_HOR_OFFSET;
        t_open_rect.u_width = m_subt_region_w; // SUBT_WIDTH;
        t_open_rect.u_top = 0;//SUBT_VER_OFFSET;
        t_open_rect.u_height = m_subt_region_h; // SUBT_HEIGHT;

        r.u_left = 0;
        r.u_top = 0;
        r.u_width =t_open_rect.u_width;
        r.u_height =t_open_rect.u_height;
    }
    else
    {
        t_open_rect.u_left = g_subt_config_par.subt_hor_offset;
        t_open_rect.u_width = g_subt_config_par.subt_width;
        t_open_rect.u_top = g_subt_config_par.subt_ver_offset;
        t_open_rect.u_height = g_subt_config_par.subt_height;

        r.u_left = 0;
        r.u_top = 0;
        r.u_width =g_subt_config_par.subt_width;
        r.u_height =g_subt_config_par.subt_height;
    }

    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, g_subt_config_par.osd_layer_id);
	if(osd_dev==NULL)
	{
		return;
	}
    ret_func =osddrv_close((HANDLE)osd_dev);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    ret_func =osddrv_open((HANDLE)osd_dev, &t_open_para);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    osal_task_sleep(20);

    //if(g_subt_config_par.hd_subtitle_support)
    //    OSDDrv_IoCtl((HANDLE)osd_dev, OSD_IO_SET_SCREEN_WIDTH, 720);

    ret_func =osddrv_create_region((HANDLE)osd_dev, 0, &(t_open_rect), NULL);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    ret_func =osddrv_modify_pallette((HANDLE)osd_dev, OSD_TRANSPARENT_COLOR, 0x10, 0x80, 0x80, 0);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    ret_func =osddrv_region_fill((HANDLE)osd_dev,0,&r, OSD_TRANSPARENT_COLOR);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}

    ret_func1 = osd_subt_scale_no_bl(m_subt_region_w);
	if(ret_func1!=TRUE)
	{
		return;
	}
    ret_func =osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_ON);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    g_update_palette = FALSE;

    //OSDDrv_IoCtl(osd_dev, OSD_IO_SUBT_RESOLUTION, OSD_SUBT_RESO_720X576);
    subt_region_type = 0xFFFFFFFF;
    g_subt_show_on = TRUE;
    MEMSET(subt_palette,0,256*4);
    m_total_height = 0;
    MEMSET(g_subt_config_par.g_ps_buf_addr,OSD_TRANSPARENT_COLOR,g_subt_config_par.g_ps_buf_len);
    for(i = 0; i < MAX_REGION_IN_PAGE; i++)
    {
        m_region_data_nobl[i] = g_subt_config_par.g_ps_buf_addr;
        m_used_region_nobl[i] = FALSE;
        m_shown_region_nobl[i] = FALSE;
        MEMSET(&(m_subt_region_nobl[i]),0,sizeof(struct osdrect));
    }
    g_ps_free = 0;
    //LEAVE_SUBT_OSD_API();
}


void osd_subt_leave_no_bl(void)
{
    //int i;
    RET_CODE ret_func = 0;

    while( TRUE== m_in_subt_decoding)
    {
        os_task_sleep(1);
    }

    if( FALSE== g_subt_show_on)
    {
        return;
    }

    g_subt_show_on = FALSE;

    SUBT_PRINTF("lib subtitle leave.\n");

    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,g_subt_config_par.osd_layer_id);
	if(osd_dev==NULL)
	{
		return;
	}
    ret_func = osddrv_show_on_off((HANDLE)osd_dev, OSDDRV_OFF);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    ret_func = osddrv_delete_region((HANDLE)osd_dev, 0);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}
    ret_func = osddrv_close((HANDLE)osd_dev);
	if(ret_func!=RET_SUCCESS)
	{
		return;
	}

    //LEAVE_SUBT_OSD_API();
}

void osd_subt_set_clut_no_bl(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t)
{
    ENTER_SUBT_OSD_API();
    //RET_CODE ret_func = 0;
    //if(entry_id >= SUBT_MAX_PALLETE_SIZE)
    //{
    //    return ;
    //}
/*    
    if((0x80>entry_id)||(0xff==y)||(0xff==cb)||(0xff==cr)||(0xf<t))
	{
		LEAVE_SUBT_OSD_API();
		return;
	}
*/	
    if(TRUE == g_subt_show_on)
    {
        //OSDDrv_ModifyPallette((HANDLE)osd_dev, entry_id, y,cb,cr,t);
        subt_palette[entry_id][0]=y;
        subt_palette[entry_id][1]=cb;
        subt_palette[entry_id][2]=cr;
        subt_palette[entry_id][3]=t;
        //if( (entry_id+1)%16 == 0)
        //    OSDDrv_SetPallette((HANDLE)osd_dev, subt_palette, 256,OSDDRV_YCBCR);
        g_update_palette = TRUE;

    }
    LEAVE_SUBT_OSD_API();
}

void osd_subt_update_clut_no_bl(void)
{
    RET_CODE ret_func = 0;

    if(g_update_palette)
    {
        ret_func = osddrv_set_pallette((HANDLE)osd_dev, (UINT8 *)subt_palette, 256,OSDDRV_YCBCR);
		ret_func=(RET_SUCCESS==ret_func)?RET_SUCCESS:RET_FAILURE;
        g_update_palette = FALSE;
    }

}

__ATTRIBUTE_RAM_
void osd_subt_clear_page_no_bl(void)
{
    UINT8 i = 0;
    UINT8 ret_func = 0;

    ENTER_SUBT_OSD_API();

    if( FALSE== g_subt_show_on)
    {
        LEAVE_SUBT_OSD_API();
        return;
    }
    SUBT_PRINTF("%s\n", __FUNCTION__);
    for(i=0;i<MAX_REGION_IN_PAGE;i++)
    {
        if((m_used_region_nobl[i]) && (m_shown_region_nobl[i]))
        {
            SUBT_PRINTF("%s: show off region(%d).\n", __FUNCTION__, i);
            //OSDDrv_RegionShow((HANDLE)osd_dev,i,0);
            ret_func =osd_subt_clear_region(i);
			ret_func=(SUCCESS==ret_func)?SUCCESS:SUBT_RET_FAILURE;
        }
        else
        {
            SUBT_PRINTF("%s: show off NULL region(%d).\n", __FUNCTION__, i);
        }
    }
    LEAVE_SUBT_OSD_API();
}

__ATTRIBUTE_RAM_
INT32 osd_subt_delete_region_no_bl(UINT8 region_idx)
{
    INT32 ret = ERR_FAILUE;

    ENTER_SUBT_OSD_API();
    if(( FALSE== g_subt_show_on)||(region_idx >= MAX_REGION_IN_PAGE))
    {
        ret = ERR_FAILUE;
    }
    else
    {
        SUBT_PRINTF("%s, region_idx = %d\n", __FUNCTION__, region_idx);
        ret = osd_subt_del_region(region_idx);
    }
    LEAVE_SUBT_OSD_API();
    return ret;
}

__ATTRIBUTE_RAM_
INT32 osd_subt_region_show_no_bl(UINT8 region_idx,UINT8 on)
{
    INT32 ret = ERR_FAILUE;

    ENTER_SUBT_OSD_API();
    if(region_idx >= MAX_REGION_IN_PAGE)
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }
    if( FALSE== g_subt_show_on)
    {
        ret = ERR_FAILUE;
    }
    else
    {
        SUBT_PRINTF("%s, region_idx = %d, on = %d\n", __FUNCTION__, region_idx, on);

        if(m_used_region_nobl[region_idx])
        {
            if(on && (FALSE== m_shown_region_nobl[region_idx] ))
            {
                m_shown_region_nobl[region_idx] = TRUE;
                ret = osd_region_show(region_idx,on);
            }
            else if((!on) &&( TRUE ==m_shown_region_nobl[region_idx] ))
            {
                m_shown_region_nobl[region_idx] = FALSE;
                ret = osd_region_show(region_idx,on);
            }
            else if((on) && (m_shown_region_nobl[region_idx]))
            {
                //force show a region to fix bug21520, because ge can not scale one pixel
                //libc_printf("force show this region\n");
                m_shown_region_nobl[region_idx] = TRUE;
                ret = osd_region_show(region_idx,on);
            }
        }
        else
        {
            SUBT_PRINTF("%s: show NULL region(%d).\n", __FUNCTION__, region_idx);
            ret = ERR_FAILUE;
        }
    }
    LEAVE_SUBT_OSD_API();
    return ret;
}

__ATTRIBUTE_RAM_
INT32 osd_subt_draw_pixel_no_bl(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data)
{
    INT32 ret = 0;


    ENTER_SUBT_OSD_API();
    if((region_idx >= MAX_REGION_IN_PAGE) /*||(0x80>data)*/)
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }
    if( FALSE== g_subt_show_on)
    {
        ret = ERR_FAILUE;
    }
    else
    {
        if(m_used_region_nobl[region_idx])
        {
            if((x >= m_subt_region_nobl[region_idx].u_width) || (y >= m_subt_region_nobl[region_idx].u_height))
            {
                SUBT_PRINTF("%s: out of region(%d), x = %d, y = %d, width = %d, height = %d.\n", \
                    __FUNCTION__, region_idx, x, y, m_subt_region_nobl[region_idx].u_width,
                    m_subt_region_nobl[region_idx].u_height);
                ret = ERR_FAILUE;
            }
            else
            {
                *(m_region_data_nobl[region_idx]+x+y*m_subt_region_nobl[region_idx].u_width) = data;
                ret = SUCCESS;
            }

        }
        else
        {
            SUBT_PRINTF("%s: draw NULL region(%d).\n", __FUNCTION__, region_idx);
            ret =  ERR_FAILUE;
        }
    }
    LEAVE_SUBT_OSD_API();
    return ret;
}
// -- for sdk
//void osd_subt_draw_pixelmap_no_bl(struct OSDRect rect, UINT8* data)
//{
//}

INT32 osd_subt_get_region_addr_no_bl(UINT8 region_idx,UINT16 y, UINT32 *addr)
{
    INT32 ret = SUCCESS;

    ENTER_SUBT_OSD_API();
    if((region_idx >= MAX_REGION_IN_PAGE)||(NULL==addr))
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    if( FALSE== g_subt_show_on)
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    if(!m_used_region_nobl[region_idx])
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    if(y >= m_subt_region_nobl[region_idx].u_height)
    {
    	LEAVE_SUBT_OSD_API();
        return ERR_FAILUE;
    }

    *addr = (UINT32)(m_region_data_nobl[region_idx]+y*(m_subt_region_nobl[region_idx].u_width));

    LEAVE_SUBT_OSD_API();
    return ret;
}

INT32 osd_subt_display_define_no_bl(struct sdec_display_config *cfg)
{
    UINT32 region_type = 0xFFFFFFFF;
    INT32 ret_func = 0;

    if((g_subt_show_on != TRUE)||(NULL == cfg))
    {
        return 0;
    }

    if(cfg->display_height <= SUBT_DISPLAY_HEIGHT_480)
    {
        region_type = OSD_SUBT_RESO_720X480;
    }
    else if(cfg->display_height <= SUBT_DISPLAY_HEIGHT_576)
    {
        region_type = OSD_SUBT_RESO_720X576;
    }
    else if(cfg->display_height <= SUBT_DISPLAY_HEIGHT_720)
    {
        region_type = OSD_SUBT_RESO_1280X720;
    }
    else if(cfg->display_height <= SUBT_DISPLAY_HEIGHT_1080)
    {
        region_type = OSD_SUBT_RESO_1920X1080;
    }

    if((region_type != 0xFFFFFFFF) && (region_type != subt_region_type))
    {
        subt_region_type = region_type;
        osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,g_subt_config_par.osd_layer_id);
        ret_func =osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_SUBT_RESOLUTION, subt_region_type);
		return ret_func;
        SUBT_PRINTF("\nset osd type %d .     ",subt_region_type);
    }
    return 0;
}
INT32 osd_subt_clear_osd_screen_no_bl(BOOL flag)
{

    INT32 ret = ERR_FAILUE;
    ENTER_SUBT_OSD_API();
    if(g_subt_show_on == FALSE)
    {
    	LEAVE_SUBT_OSD_API();    
    	ret = ERR_FAILUE;	    
    }	
    page_delete_flag=flag;
    ret=SUCCESS;
    LEAVE_SUBT_OSD_API();
    return ret;

}
