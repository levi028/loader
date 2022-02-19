/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osddrv.c
*
*    Description: This file contains api functions of osd driver
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <api/libc/string.h>

#ifdef GE_SIMULATE_OSD
#include <hld/ge/ge.h>
#endif
#include <hld/gma/gma.h>
#define USER_WIDTH_PARA1 1280
#define USER_WIDTH_PARA2 720

#ifdef GE_SIMULATE_OSD
#define GMA_SW_LAYER_ID          ((UINT32)dev->priv)
#define MAX_DEV_NUM		2
#define GE_SIMU_MUTEX_LOCK()     osal_mutex_lock(dev->sema_opert_osd, OSAL_WAIT_FOREVER_TIME)
#define GE_SIMU_MUTEX_UNLOCK()   osal_mutex_unlock(dev->sema_opert_osd)
#define m_cmd_list               m_osd_cmd_list[GMA_SW_LAYER_ID]

static struct ge_device *m_osddrv_ge_dev;
static ge_cmd_list_hdl m_osd_cmd_list[2];
static const ge_scale_param_t m_gma_scale_param[] =
{
    {LINE_720_25, 720, 576, 1280, 720 },
    {LINE_720_30, 720, 480, 1280, 720 },
    {LINE_1080_25, 720, 576, 1920, 1080 },
    {LINE_1080_30, 720, 480, 1920, 1080 },
};

static const ge_scale_param_t m_gma_scale_param_src_1280x720[] =
{
    {PAL,  1280, 720, 720, 576 },
    {NTSC, 1280, 720, 720, 480 },
    {LINE_720_25, 1, 1, 1, 1 },
    {LINE_720_30, 1, 1, 1, 1 },
    {LINE_1080_25, 1280, 720, 1920, 1080 },
    {LINE_1080_30, 1280, 720, 1920, 1080 },
};

static const ge_scale_param_t m_gma_multiview[]=
{
    {PAL,  1, 384, 1, 510 },
    {NTSC, 1, 384, 1, 426},
};

//static int m_osd_gma_attach_flag = 0;
static struct osd_private_gma *m_osd_gma_priv[MAX_DEV_NUM] = {NULL, NULL};

static void osddrv_attach_gma_layer(struct ge_device *ge_dev, UINT32 osd_layer_id)
{
	struct ge_device *dev = (struct ge_device *)ge_dev;
	struct osd_private_gma *priv;

	if(dev == NULL)
	{
		OSD_PRF("malloc osd dev fail\n");
		return;
	}

	priv = MALLOC(sizeof(*priv));
	if(priv == NULL)
	{
		OSD_PRF("malloc osd priv fail\n");
	}

	MEMSET((void *)priv, 0, sizeof(*priv));
    if(0 == osd_layer_id)
        dev->gma0_priv = (void *)priv;
    else if(1 == osd_layer_id)
        dev->gma1_priv = (void *)priv;

    /*priv->gma_dev = dev_get_by_id(HLD_DEV_TYPE_GMA, osd_layer_id);
	if(priv->gma_dev == NULL)
	{
		OSD_PRF("get gma device fail\n");
		return ;
	}

	priv->index = osd_layer_id;*/
    m_osd_gma_priv[osd_layer_id] = priv;
	//m_osd_gma_attach_flag = 1;
}

void osd_dev_api_attach(struct ge_device *ge_dev, UINT32 osd_layer_id)
{
    struct osd_device *dev = NULL;
    const char *name = NULL;
    UINT32 max_layer = 3;

    if (ge_dev)
    {
        m_osddrv_ge_dev = ge_dev;
    }

    if (osd_layer_id >= max_layer)
    {
        return;
    }

    if (0 == osd_layer_id)
    {
        name = "OSD_DEV_0";
    }
    else if (1 == osd_layer_id)
    {
        name = "OSD_DEV_1";
    }
    else
    {
        name = "OSD_DEV_2";
    }

    dev = (struct osd_device *)dev_alloc((INT8 *)name,HLD_DEV_TYPE_OSD,sizeof(struct osd_device));
    if (NULL == dev)
    {
        return ;
    }

    dev->priv = (void *)osd_layer_id;
    dev->next = NULL;
    dev->flags = 0;
    dev->sema_opert_osd = osal_mutex_create();
    if(OSAL_INVALID_ID == dev->sema_opert_osd)
    {
        PRINTF("OSD Can not create mutex!!!\n");
        ASSERT(0);
    }

    if (dev_register(dev) != RET_SUCCESS)
    {
        dev_free(dev);
        return ;
    }

    osddrv_attach_gma_layer(ge_dev, osd_layer_id);
    return ;
}

static enum GE_PIXEL_FORMAT osddrv_color_mode_to_ge(UINT8 color_mode)
{
    enum GE_PIXEL_FORMAT color_format = GE_PF_CLUT8;

    switch (color_mode)
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

static enum GMA_FORMAT get_gma_format(enum osdcolor_mode eMode)
{
	enum GMA_FORMAT format = GMA_FORMAT_CLUT8;

	switch(eMode)
	{
		case OSD_256_COLOR:
			format = GMA_FORMAT_CLUT8;
			break;
		case OSD_HD_ARGB1555:
			format = GMA_FORMAT_ARGB1555;
			break;
		case OSD_HD_RGB565:
			format = GMA_FORMAT_RGB555;
			break;
		case OSD_HD_ARGB8888:
			format = GMA_FORMAT_ARGB8888;
			break;
	    case OSD_HD_ARGB4444:
			format = GMA_FORMAT_ARGB4444;
		default:
			break;
	}

	return format;
}

static enum GMA_FORMAT get_gma_3921_format(enum GE_PIXEL_FORMAT eMode)
{
	enum GMA_FORMAT format = GMA_FORMAT_CLUT8;

	switch(eMode)
	{
		case GE_PF_CLUT8:
			format = GMA_FORMAT_CLUT8;
			break;
		case GE_PF_ARGB1555:
			format = GMA_FORMAT_ARGB1555;
			break;
		case GE_PF_RGB565:
			format = GMA_FORMAT_RGB555;
			break;
		case GE_PF_ARGB8888:
			format = GMA_FORMAT_ARGB8888;
			break;
		default:
			break;
	}

	return format;
}


static int osddrv_open_gma(struct ge_device *dev, UINT32 layer_id, struct osdpara *pOpenPara)
{
    INT32 result = RET_FAILURE;
    struct osd_private_gma *priv = NULL;
    GMA_LAYER_PARS layer_pars;
    struct gma_device *gma_dev = NULL;

    if(dev)
    {
        if(0 == layer_id)
        {
            priv = (struct osd_private_gma *)dev->gma0_priv;
        }
        else
        {
            priv = (struct osd_private_gma *)dev->gma1_priv;
        }
    }

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");
		return RET_SUCCESS;
	}
    priv->gma_dev = (struct gma_device *)dev_get_by_id(HLD_DEV_TYPE_GMA, layer_id);
	if(priv->gma_dev == NULL)
	{
		OSD_PRF("get gma device fail\n");
		return RET_FAILURE;
	}

	priv->index = layer_id;
	priv->auto_clear_region = TRUE;

	if(priv->open)
	{
		OSD_PRF("osd_gma is already opend\n ");
		return RET_SUCCESS;
	}

    if(pOpenPara)
	{
		MEMCPY((void *)&(priv->default_para), pOpenPara, sizeof(*pOpenPara));
	}
	else
	{
		priv->default_para.e_mode = OSD_HD_ARGB1555;
		priv->default_para.u_galpha = 0xFF;
		priv->default_para.u_galpha_enable = 0;
		priv->default_para.u_pallette_sel = 0;
	}

	if(priv->default_para.e_mode == OSD_256_COLOR)
	{
		priv->trans_color = 0xFF;
		priv->pallette_size = 256;
		priv->pallette_start = MALLOC(priv->pallette_size * 4);
		if(priv->pallette_start == NULL)
		{
			OSD_PRF("malloc pallette buffer fail\n");
			return RET_FAILURE;
		}

		MEMSET((void *)priv->pallette_start, 0, priv->pallette_size * 4);
	}
	else
	{
		priv->trans_color = 0;
	}

    MEMSET((void *)&layer_pars, 0, sizeof(layer_pars));
	layer_pars.format = get_gma_format(priv->default_para.e_mode);
	layer_pars.global_alpha_enable = 0;
	layer_pars.global_alpha_value = 0xFF;
	layer_pars.max_size.w = OSD_GMA_MAX_WIDTH;
	layer_pars.max_size.h = OSD_GMA_MAX_HEIGHT;
    gma_dev = priv->gma_dev;
    if(gma_dev->open)
    {
	    result = gma_dev->open(gma_dev, &layer_pars);
    }
    if(RET_FAILURE == result)
    {
		OSD_PRF("open gma fail\n");
		return RET_FAILURE;
	}

	priv->open = 1;
	return RET_SUCCESS;

}
static UINT32 osddrv_get_pitch(UINT8 color_mode, UINT16 width)
{
    switch (color_mode)
    {
        case OSD_HD_ARGB1555:
        case OSD_HD_ARGB4444:
            width <<= 1;
            break;
        case osd_hd_aycb_cr8888:
        case OSD_HD_ARGB8888:
            width <<= 2;
            break;
        case OSD_256_COLOR:
        default:
            break;
    }

    return width;
}

static UINT32 osddrv_get_pitch_by_ge_format(UINT8 ge_pixel_fmt, UINT16 width)
{
    switch (ge_pixel_fmt)
    {
        case GE_PF_ARGB1555:
        case GE_PF_ARGB4444:
            width <<= 1;
            break;
        case GE_PF_RGB888:
        case GE_PF_ARGB8888:
            width <<= 2;
            break;
        case GE_PF_CLUT8:
        case GE_PF_CK_CLUT8:
        default:
            break;
    }

    return width;
}

#endif

RET_CODE osddrv_open(HANDLE hdev,struct osdpara *open_para)
{
    INT32 result = RET_FAILURE;
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;

    ge_open(m_osddrv_ge_dev);
    osddrv_open_gma(m_osddrv_ge_dev, GMA_SW_LAYER_ID, open_para);//////////////////////////////////////////////

    if (0 == m_cmd_list)
    {
        m_cmd_list = ge_cmd_list_create(m_osddrv_ge_dev, 2);
    }

    if (open_para)
    {
        ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, 0, &region_param);

        region_param.galpha_enable = open_para->u_galpha_enable;
        region_param.global_alpha = open_para->u_galpha;
        region_param.pallette_sel = open_para->u_pallette_sel;
        region_param.color_format = osddrv_color_mode_to_ge(open_para->e_mode);

        ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, 0, &region_param);
    }

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_open(hdev, open_para);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }

    /* If openned already, exit */
    if(dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("osddrv_open: warning - device %s openned already!\n", dev->name);
        return RET_SUCCESS;
    }

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev,open_para);
    }

    /* Setup init work mode */
    if (RET_SUCCESS == result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    }

    return result;
}

#ifdef GE_SIMULATE_OSD
extern RET_CODE ge_gma_close(struct ge_device *dev, UINT32 layer_id);
#endif

RET_CODE osddrv_close(HANDLE hdev)
{
    INT32 result = RET_FAILURE;
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_cmd_list_destroy(m_osddrv_ge_dev, m_cmd_list);
    ge_gma_close(m_osddrv_ge_dev, GMA_SW_LAYER_ID);
    m_cmd_list = 0;

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_close(hdev);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("osddrv_close: warning - device %s closed already!\n", dev->name);
        return RET_SUCCESS;
    }

    /* Stop device */
    if (dev->close)
    {
        result = dev->close(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}

__ATTRIBUTE_RAM_
RET_CODE osddrv_io_ctl(HANDLE hdev,UINT32 cmd,UINT32 param)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    switch (cmd)
    {
        case OSD_IO_SET_GLOBAL_ALPHA:
            cmd = GE_IO_SET_GLOBAL_ALPHA;
            break;
        case OSD_IO_SET_TRANS_COLOR:
            cmd = GE_IO_SET_TRANS_COLOR;
            break;
        case OSD_IO_ENABLE_ANTIFLICK:
            cmd = GE_IO_ENABLE_ANTIFLICK;
            param = TRUE;
            break;
        case OSD_IO_DISABLE_ANTIFLICK:
            cmd = GE_IO_ENABLE_ANTIFLICK;
            param = FALSE;
            break;
        case OSD_IO_SET_AUTO_CLEAR_REGION:
            cmd = GE_IO_SET_AUTO_CLEAR_REGION;
            break;
        case OSD_IO_GET_ON_OFF:
            cmd = GE_IO_GET_LAYER_ON_OFF;
            break;
         case OSD_IO_SET_ENHANCE_PAR:

             cmd = GE_IO_SET_ENHANCE_PAR;

             break;
        case OSD_IO_CREATE_REGION:
        case OSD_IO_MOVE_REGION:
        {
            pcosd_region_param posd_rgn_info = (pcosd_region_param)param;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (NULL == posd_rgn_info)
            {
                return RET_FAILURE;
            }
            ge_rgn_info.color_format = posd_rgn_info->color_format;
            ge_rgn_info.galpha_enable = posd_rgn_info->galpha_enable;
            ge_rgn_info.global_alpha = posd_rgn_info->global_alpha;
            ge_rgn_info.pallette_sel = posd_rgn_info->pallette_sel;
            ge_rgn_info.region_x = posd_rgn_info->region_x;
            ge_rgn_info.region_y = posd_rgn_info->region_y;
            ge_rgn_info.region_w = posd_rgn_info->region_w;
            ge_rgn_info.region_h = posd_rgn_info->region_h;
            ge_rgn_info.bitmap_addr = posd_rgn_info->bitmap_addr;
            ge_rgn_info.pixel_pitch = posd_rgn_info->pixel_pitch;
            ge_rgn_info.bitmap_x = posd_rgn_info->bitmap_x;
            ge_rgn_info.bitmap_y = posd_rgn_info->bitmap_y;
            ge_rgn_info.bitmap_w = posd_rgn_info->bitmap_w;
            ge_rgn_info.bitmap_h = posd_rgn_info->bitmap_h;
            if (OSD_IO_CREATE_REGION == cmd)
            {
                ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID,posd_rgn_info->region_id, &ge_rgn_info);
            }
            else
            {
                ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            }
            return ret;
        }
        case OSD_IO_GET_REGION_INFO:
        {
            posd_region_param posd_rgn_info = (posd_region_param)param;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (NULL == posd_rgn_info)
            {
                return RET_FAILURE;
            }
            ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            posd_rgn_info->color_format = ge_rgn_info.color_format;
            posd_rgn_info->galpha_enable = ge_rgn_info.galpha_enable;
            posd_rgn_info->global_alpha = ge_rgn_info.global_alpha;
            posd_rgn_info->pallette_sel = ge_rgn_info.pallette_sel;
            posd_rgn_info->region_x = ge_rgn_info.region_x;
            posd_rgn_info->region_y = ge_rgn_info.region_y;
            posd_rgn_info->region_w = ge_rgn_info.region_w;
            posd_rgn_info->region_h = ge_rgn_info.region_h;
            posd_rgn_info->bitmap_addr = ge_rgn_info.bitmap_addr;
            posd_rgn_info->pixel_pitch = ge_rgn_info.pixel_pitch;
            posd_rgn_info->bitmap_x = ge_rgn_info.bitmap_x;
            posd_rgn_info->bitmap_y = ge_rgn_info.bitmap_y;
            posd_rgn_info->bitmap_w = ge_rgn_info.bitmap_w;
            posd_rgn_info->bitmap_h = ge_rgn_info.bitmap_h;
            return ret;
        }
        case OSD_IO_GET_DISPLAY_RECT:
        {
            struct osdrect *posd_rect = (struct osdrect *)param;
            ge_rect_t ge_rect;
            RET_CODE ret;
            if (NULL == posd_rect)
            {
                return RET_FAILURE;
            }
            ret = ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_IO_GET_DISPLAY_RECT, (UINT32)&ge_rect);
            posd_rect->u_left = ge_rect.left;
            posd_rect->u_top = ge_rect.top;
            posd_rect->u_width = ge_rect.width;
            posd_rect->u_height = ge_rect.height;
            return ret;
        }
        case OSD_IO_SET_DISPLAY_RECT:
        {
            struct osdrect *posd_rect = (struct osdrect *)param;
            ge_rect_t ge_rect;
            RET_CODE ret;
            if (NULL == posd_rect)
            {
                return RET_FAILURE;
            }
            ge_rect.left = posd_rect->u_left;
            ge_rect.top = posd_rect->u_top;
            ge_rect.width = posd_rect->u_width;
            ge_rect.height = posd_rect->u_height;
            ret = ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_IO_SET_DISPLAY_RECT, (UINT32)&ge_rect);
            return ret;
        }
        default:
            return RET_SUCCESS;
            break;
    }
    return ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, cmd, param);
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_io_ctl(hdev, cmd, param);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->ioctl)
    {
        return dev->ioctl(dev, cmd, param);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_get_para(HANDLE hdev,struct osdpara *para)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_get_para(hdev, para);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->get_para)
    {
        return dev->get_para(dev, para);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_show_on_off(HANDLE hdev,UINT8 on_off)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_show_onoff(m_osddrv_ge_dev, GMA_SW_LAYER_ID, on_off);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_show_on_off(hdev, on_off);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->show_onoff)
    {
        return dev->show_onoff(dev, on_off);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_set_pallette(HANDLE hdev,UINT8 *pallette, UINT16 color_n,UINT8 type)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    static const ge_pal_attr_t m_rgb_pal_attr =
    {GE_PAL_RGB, GE_RGB_ORDER_ARGB, GE_ALPHA_RANGE_0_255, GE_ALPHA_POLARITY_0};

    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_set_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, \
                   pallette, color_n, (OSDDRV_RGB == type) ? &m_rgb_pal_attr : NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_set_pallette(hdev, pallette, color_n, type);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->set_pallette)
    {
        return dev->set_pallette(dev, pallette,color_n,type);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_get_pallette(HANDLE hdev,UINT8 *pallette,UINT16 color_n,UINT8 type)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_get_pallette(m_osddrv_ge_dev,GMA_SW_LAYER_ID, pallette, color_n, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_get_pallette(hdev, pallette, color_n, type);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->get_pallette)
    {
        return dev->get_pallette(dev, pallette,color_n,type);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_modify_pallette(HANDLE hdev,UINT8 index,UINT8 y,UINT8 cb,UINT8 cr,UINT8 k)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_modify_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, index, k, y, cb, cr, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_modify_pallette(hdev, index, y, cb, cr, k);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->modify_pallette)
    {
        return dev->modify_pallette(dev, index,y,cb,cr,k);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_create_region(HANDLE hdev,UINT8 region_id,struct osdrect *rect,struct osdpara *open_para)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();
    if ((NULL == open_para) && (NULL == rect))
    {
        ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, NULL);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    if (NULL == open_para)
    {
        ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);
        region_param.region_x = rect->u_left;
        region_param.region_y = rect->u_top;
        region_param.region_w = rect->u_width;
        region_param.region_h = rect->u_height;

        region_param.bitmap_addr = 0;
        region_param.pixel_pitch = 0;
    }
    else
    {
        region_param.region_x = rect->u_left;
        region_param.region_y = rect->u_top;
        region_param.region_w = rect->u_width;
        region_param.region_h = rect->u_height;

        region_param.galpha_enable = open_para->u_galpha_enable;
        region_param.global_alpha = open_para->u_galpha;
        region_param.pallette_sel = open_para->u_pallette_sel;
        region_param.bitmap_addr = 0;
        region_param.pixel_pitch = 0;
        region_param.color_format = osddrv_color_mode_to_ge(open_para->e_mode);
    }

    ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_create_region(hdev, region_id, rect, open_para);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->create_region)
    {
        return dev->create_region(dev, region_id,rect,open_para);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_delete_region(HANDLE hdev,UINT8 region_id)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_delete_region(hdev, region_id);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->delete_region)
    {
        return dev->delete_region(dev, region_id);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_set_region_pos(HANDLE hdev,UINT8 region_id, struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();

    ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);

    region_param.region_x = rect->u_left;
    region_param.region_y = rect->u_top;
    region_param.region_w = rect->u_width;
    region_param.region_h = rect->u_height;

    RET_CODE ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_set_region_pos(hdev, region_id, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->set_region_pos)
    {
        return dev->set_region_pos(dev, region_id,rect);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_get_region_pos(HANDLE hdev,UINT8 region_id,struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();

    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);

    rect->u_left = region_param.region_x;
    rect->u_top = region_param.region_y;
    rect->u_width = region_param.region_w;
    rect->u_height = region_param.region_h;
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_get_region_pos(hdev, region_id, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->get_region_pos)
    {
         return dev->get_region_pos(dev, region_id,rect);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_region_show(HANDLE hdev,UINT8 region_id,BOOL on)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_show_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, on);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_show(hdev, region_id, on);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_show)
    {
        return dev->region_show(dev,region_id,on);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_region_write_by_surface(HANDLE hdev,UINT8 region_id, UINT8 *src_data,
                              struct osdrect *dest_rect, struct osdrect *src_rect, UINT32 pitch)
{
#ifndef GE_SIMULATE_OSD
    struct osd_device *dev = (struct osd_device *)hdev;

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_write_by_surface)
    {
        return dev->region_write_by_surface(dev, region_id,src_data,dest_rect,src_rect, pitch);
    }
#endif
    return !RET_SUCCESS;
}

RET_CODE osddrv_region_write(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(vscr->b_color_mode, vscr->v_r.u_width);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->u_left - vscr->v_r.u_left;
    ptn_y = rect->u_top - vscr->v_r.u_top;
    osal_cache_flush(vscr->lpb_scr + byte_pitch * ptn_y, byte_pitch * rect->u_height);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(vscr->b_color_mode);
    base_addr.base_address = (UINT32)vscr->lpb_scr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = vscr->v_r.u_width;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, rect->u_width, rect->u_height);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_left, rect->u_top);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, ptn_x, ptn_y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_write(hdev, region_id, vscr, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_write)
    {
        return dev->region_write(dev, region_id,vscr,rect);
    }

    return  !RET_SUCCESS;
}
RET_CODE osddrv_region_read(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(vscr->b_color_mode, vscr->v_r.u_width);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->u_left - vscr->v_r.u_left;
    ptn_y = rect->u_top - vscr->v_r.u_top;
    osal_cache_invalidate(vscr->lpb_scr + byte_pitch * ptn_y, byte_pitch * rect->u_height);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(vscr->b_color_mode);
    base_addr.base_address = (UINT32)vscr->lpb_scr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = vscr->v_r.u_width;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_PRIM_DISABLE);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_DST, &base_addr);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, ptn_x, ptn_y);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_width, rect->u_height);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_SRC, rect->u_left, rect->u_top);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_read(hdev, region_id, vscr, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_read)
    {
        return dev->region_read(dev, region_id,vscr,rect);
    }

    return  !RET_SUCCESS;
}
RET_CODE osddrv_region_fill(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
       ge_unlock(m_osddrv_ge_dev);
       GE_SIMU_MUTEX_UNLOCK();
       return ret;
    }
    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, m_cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, color_data);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_left, rect->u_top);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_width, rect->u_height);
    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);

    ge_cmd_list_end(m_osddrv_ge_dev, m_cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_fill(hdev, region_id, rect, color_data);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_fill)
    {
        return dev->region_fill(dev, region_id,rect,color_data);
    }

    return !RET_SUCCESS;
}
//#ifdef BIDIRECTIONAL_OSD_STYLE
RET_CODE osddrv_region_write_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct osdrect rr;

    ret = osddrv_get_region_pos((HANDLE)dev, region_id, &rr);
    if(ret != RET_SUCCESS)
    {
        return ret;
    }

    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(vscr->b_color_mode, vscr->v_r.u_width);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->u_left - vscr->v_r.u_left;
    ptn_y = rect->u_top - vscr->v_r.u_top;
    osal_cache_flush(vscr->lpb_scr + byte_pitch * ptn_y, byte_pitch * rect->u_height);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(vscr->b_color_mode);
    base_addr.base_address = (UINT32)vscr->lpb_scr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = vscr->v_r.u_width;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, rect->u_width, rect->u_height);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, (rr.u_width - rect->u_left - rect->u_width), rect->u_top);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, ptn_x, ptn_y);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_PTN, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_write_inverse(hdev, region_id, vscr, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_write_inverse)
    {
        return dev->region_write_inverse(dev, region_id,vscr,rect);
    }

    return  !RET_SUCCESS;
}
RET_CODE osddrv_region_read_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct osdrect rr;

    ret = osddrv_get_region_pos((HANDLE)dev, region_id, &rr);
    if(ret != RET_SUCCESS)
    {
        return ret;
    }

    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(vscr->b_color_mode, vscr->v_r.u_width);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->u_left - vscr->v_r.u_left;
    ptn_y = rect->u_top - vscr->v_r.u_top;
    osal_cache_invalidate(vscr->lpb_scr + byte_pitch * ptn_y, byte_pitch * rect->u_height);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(vscr->b_color_mode);
    base_addr.base_address = (UINT32)vscr->lpb_scr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = vscr->v_r.u_width;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_PRIM_DISABLE);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_DST, &base_addr);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, ptn_x, ptn_y);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_width, rect->u_height);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_SRC, (rr.u_width - rect->u_left - rect->u_width), rect->u_top);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_SRC, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_read_inverse(hdev, region_id, vscr, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_read_inverse)
    {
        return dev->region_read_inverse(dev, region_id,vscr,rect);
    }

    return  !RET_SUCCESS;
}
RET_CODE osddrv_region_fill_inverse(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct osdrect rr;

    ret = osddrv_get_region_pos((HANDLE)dev, region_id, &rr);
    if(ret != RET_SUCCESS)
    {
        return ret;
    }

    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
       ge_unlock(m_osddrv_ge_dev);
       GE_SIMU_MUTEX_UNLOCK();
       return ret;
    }
    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, m_cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, color_data);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, (rr.u_width - rect->u_left - rect->u_width), rect->u_top);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->u_width, rect->u_height);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_PTN, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);
    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);

    ge_cmd_list_end(m_osddrv_ge_dev, m_cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_fill_inverse(hdev, region_id, rect, color_data);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;

    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_fill_inverse)
    {
        return dev->region_fill_inverse(dev, region_id,rect,color_data);
    }

    return !RET_SUCCESS;
}
//#endif

RET_CODE osddrv_region_write2(HANDLE hdev,UINT8 region_id,UINT8 *src_data,UINT16 src_width,
                           UINT16 src_height,struct osdrect *src_rect, struct osdrect *dest_rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    ge_gma_region_t region_param;

    UINT32 __MAYBE_UNUSED__ vscr_width = 1920, vscr_height = 1080; // get from subtitle
    UINT32 __MAYBE_UNUSED__ reg_width, reg_height;
    GE_SIMU_MUTEX_LOCK();

    ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);
    if (ret != RET_SUCCESS)
    {
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    if (src_width > USER_WIDTH_PARA1)
    {
        vscr_width = 1920;
        vscr_height = 1080;
    }
    else if (src_width > USER_WIDTH_PARA2)
    {
        vscr_width = 1280;
        vscr_height = 720;
    }
    else
    {
        vscr_width = 0;//720;
        vscr_height = 0;//576;
    }

    reg_width = region_param.region_w;
    reg_height = region_param.region_h;

    #if 0
    if ((vscr_width > reg_width) || (vscr_height > reg_height))
    {
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
        ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
        }
        ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, &region_param);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
            return ret;
        }
        ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_VSCALE_TTX_SUBT, PAL);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
        }
    }
    #endif

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(OSD_256_COLOR, src_width);

    osal_cache_flush(src_data + byte_pitch *src_rect->u_top, byte_pitch *src_rect->u_height);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(OSD_256_COLOR);
    base_addr.base_address = (UINT32)src_data;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = src_width;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, src_rect->u_width, src_rect->u_height);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, dest_rect->u_left, dest_rect->u_top);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, src_rect->u_left, src_rect->u_top);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_region_write2(hdev, region_id, src_data, src_width, src_height, src_rect, dest_rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->region_write2)
    {
         return dev->region_write2(dev, region_id, src_data, src_width, src_height, src_rect, dest_rect);
    }

    return  !RET_SUCCESS;
}

RET_CODE osddrv_draw_hor_line(HANDLE hdev, UINT8 region_id, UINT32 x, UINT32 y, UINT32 width, UINT32 color)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_cmd_list_hdl cmd_list = m_cmd_list;
    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);

    RET_CODE ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_id, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;
    }

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, color);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, width, 1);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, x, y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_draw_hor_line(hdev, region_id, x, y, width, color);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->draw_hor_line)
    {
        return dev->draw_hor_line(dev, region_id, x, y, width, color);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_scale(HANDLE hdev,UINT32 scale_cmd,UINT32 scale_param)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    switch (scale_cmd)
    {
        case OSD_SCALE_WITH_PARAM:
            scale_cmd = GE_SET_SCALE_PARAM;
            break;
        case OSD_VSCALE_OFF:
            scale_cmd = GE_VSCALE_OFF;
            break;
        case OSD_VSCALE_TTX_SUBT:
            scale_cmd = GE_VSCALE_TTX_SUBT;
            break;
        case OSD_HDUPLICATE_ON:
            scale_cmd = GE_H_DUPLICATE_ON_OFF;
            scale_param = TRUE;
            break;
        case OSD_HDUPLICATE_OFF:
            scale_cmd = GE_H_DUPLICATE_ON_OFF;
            scale_param = FALSE;
            break;
        case OSD_OUTPUT_720:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_SOURCE_NTSC == scale_param)
            {
                scale_param = (UINT32)&m_gma_scale_param[1];
            }
            else
            {
                scale_param = (UINT32)&m_gma_scale_param[0];
            }
            break;
        case OSD_OUTPUT_1080:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_SOURCE_NTSC == scale_param)
            {
                scale_param = (UINT32)&m_gma_scale_param[3];
            }
            else
            {
                scale_param = (UINT32)&m_gma_scale_param[2];
            }
            break;
        case OSD_SET_SCALE_MODE:
            scale_cmd = GE_SET_SCALE_MODE;
            break;
        case OSD_HDVSCALE_OFF:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_SOURCE_NTSC == scale_param)
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[1];
            }
            else
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[0];
            }
            break;
    case OSD_HDOUTPUT_720:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_SOURCE_NTSC == scale_param)
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[3];
            }
            else
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[2];
            }
            break;
    case OSD_HDOUTPUT_1080:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_SOURCE_NTSC == scale_param)
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[5];
            }
            else
            {
                scale_param = (UINT32)&m_gma_scale_param_src_1280x720[4];
            }
            break;
    case OSD_VSCALE_DVIEW:
            scale_cmd = GE_SET_SCALE_PARAM;
            if (OSD_PAL == *(enum osdsys *)scale_param)
            {
                scale_param = (UINT32)&m_gma_multiview[0];
            }
            else
            {
                scale_param = (UINT32)&m_gma_multiview[1];
            }
            break;
        default:
        GE_SIMU_MUTEX_UNLOCK();
           return RET_SUCCESS;
            break;
    }
    RET_CODE ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, scale_cmd, scale_param);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_scale(hdev, scale_cmd, scale_param);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->scale)
    {
        return dev->scale(dev, scale_cmd, scale_param);
    }

    return  !RET_SUCCESS;
}
RET_CODE osddrv_set_clip(HANDLE hdev,enum clipmode clipmode, struct osdrect *rect)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    enum GE_CLIP_MODE ge_clip_mode;
    GE_SIMU_MUTEX_LOCK();

    if (CLIP_INSIDE_RECT == clipmode)
    {
        ge_clip_mode = GE_CLIP_INSIDE;
    }
    else if (CLIP_OUTSIDE_RECT == clipmode)
    {
        ge_clip_mode = GE_CLIP_OUTSIDE;
    }
    else
    {
        ge_clip_mode = GE_CLIP_DISABLE;
    }

    ge_gma_set_region_clip_rect(m_osddrv_ge_dev, GMA_SW_LAYER_ID, \
            rect->u_left, rect->u_top, rect->u_width, rect->u_height);

    RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, ge_clip_mode);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_set_clip(hdev, clipmode, rect);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->set_clip)
    {
        return dev->set_clip(dev, clipmode,rect);
    }

    return !RET_SUCCESS;
}
RET_CODE osddrv_clear_clip(HANDLE hdev)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_CLIP_DISABLE);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_clear_clip(hdev);
    }
#endif

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }
    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->clear_clip)
    {
        return dev->clear_clip(dev);
    }

    return !RET_SUCCESS;
}

RET_CODE osddrv_get_region_addr(HANDLE hdev,UINT8 region_idx, UINT16 y, UINT32 *addr)
{
    struct osd_device *dev = (struct osd_device *)hdev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_idx, &region_param);

    UINT32 base_addr = region_param.bitmap_addr;
    base_addr += y * osddrv_get_pitch_by_ge_format(region_param.color_format, region_param.pixel_pitch);
                           // todo: calc according to color_format

    *addr = base_addr;
    GE_SIMU_MUTEX_UNLOCK();

    return ret;
#endif

#if (defined _M3821_ || defined _M3505_)
    if((ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()))
    {
        return gmadrv_get_region_addr(hdev, region_idx, y, addr);
    }
#endif

    if (NULL == dev)
    {
        *addr = 0;
        return RET_SUCCESS;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return RET_FAILURE;
    }

    if (dev->get_region_addr)
    {
        return dev->get_region_addr(dev,region_idx, y, addr);
    }

    return !RET_SUCCESS;
}

