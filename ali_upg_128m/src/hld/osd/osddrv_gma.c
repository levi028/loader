#if (defined _M3821_ || defined _M3505_ || defined _M3702_)

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/dis/vpo.h>

#include <hld/gma/gma.h>

#if 1
#define PRF(...)	do{}while(0)
#else
#define PRF libc_printf
#endif

#define DBG_PRF(opt, fmt, args...)\
	do\
	{\
		PRF(fmt, args);\
	}while(0)


#if 0
#define OSD_PRF(fmt, args...)\
	DBG_PRF(GMA_DBG_FLAG_DEFAULT, "OSD_gma %s->%s: L %d : "fmt"\n", __FILE__, __FUNCTION__\
	, __LINE__, ##args)
#endif

#define MAX_DEV_NUM		2


/*struct osd_region_par
{
	int valid;
	struct OSDRect reg_rect;
	int bpp;

	struct OSDPara para;

	void *mem_start;
	int mem_size;
	int pitch;
};

struct osd_private_gma
{
	int index;
	int open;
	int show_on;

	struct OSDPara default_para;
	struct gma_device *gma_dev;
	UINT32 trans_color;

	struct osd_region_par region[MAX_REGION_NUM];

	void *pallette_start;
	int pallette_size;
	int pallette_type;
};*/

static struct osd_device *m_osd_gma_dev[MAX_DEV_NUM]= {NULL, NULL};
static struct osd_private_gma *m_osd_gma_priv[MAX_DEV_NUM] = {NULL, NULL};

static const osd_scale_param m_osd_scale_map_576[] =
{
    {PAL, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_NC, 1,1,1,1/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 1,6,1,5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 9, 4, 16, 5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 9, 4, 16, 5/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 3,8,8,15/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 3,8,8,15/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

static const osd_scale_param m_osd_scale_map_720[] =
{
    {PAL, 16,5,9,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 16,5,9,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_NC, 16,5,9,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 16,3,9,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 1, 1, 1, 1},
    {LINE_720_30, 1, 1, 1, 1},

    {LINE_1080_25, 2,2,3,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 2,2,3,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

static const osd_scale_param m_osd_scale_map_1080[] =
{
    {PAL, 8*2,15,3*2,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N, 8*2,15,3*2,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_NC, 8*2,15,3*2,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC, 8*2,9,3*2,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443, 8*2,9,3*2,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60, 8*2,9,3*2,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M, 8*2,9,3*2,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25, 3, 3, 2, 2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30, 3, 3, 2, 2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};
static enum GE_PIXEL_FORMAT gmadrv_color_mode_to_ge(UINT8 bColorMode)
{
    enum GE_PIXEL_FORMAT color_format = GE_PF_CLUT8;

    switch (bColorMode)
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


static UINT32 get_screen_pitch(UINT32 region_width)
{
	if (region_width <= 720)
		return 720;

	if (region_width <= 1280)
		return 1280;

	return 1920;
}

static UINT32 osd_gma_get_screen_pitch (struct osd_private_gma *priv)
{
	int pitch = 720;
	int i = 0;

	for(i = 0;i < MAX_REGION_NUM;i++)
	{
		if(priv->region[i].valid)
		{
			if(pitch < priv->region[i].pitch)
				pitch = priv->region[i].pitch;
		}
	}

	return get_screen_pitch(pitch);
}

static pcosd_scale_param osd_gma_get_scale_param(enum tvsystem eTVMode, INT32 nScreenWidth)
{
	UINT32 i,size_576,size_720,size_1080,size;
	const osd_scale_param  *p_ui_map;

	if (eTVMode == LINE_1080_24 || eTVMode == LINE_1152_ASS || eTVMode == LINE_1080_ASS || eTVMode == LINE_1080_50)
		eTVMode = LINE_1080_25;
	else if (eTVMode == LINE_1080_60)
		eTVMode = LINE_1080_30;

	size_720  = ARRAY_SIZE(m_osd_scale_map_720);
	size_1080 = ARRAY_SIZE(m_osd_scale_map_1080);
	size_576 = ARRAY_SIZE(m_osd_scale_map_576);
	if(nScreenWidth == 1280)
	{
		size = size_720;
		p_ui_map = m_osd_scale_map_720;
	}
	else if(nScreenWidth == 1920)
	{
		size = size_1080;
		p_ui_map = m_osd_scale_map_1080;
	}
	else // if(nScreenWidth == 720)
	{
		//ASSERT(0);
		size = size_576;
		p_ui_map = m_osd_scale_map_576;
	}

	for (i = 0; i < size; i++)
	{
		if (p_ui_map[i].tv_sys == eTVMode)
		{
			return &(p_ui_map[i]);
		}
	}

	// default to PAL
	return &(p_ui_map[0]);
}

static enum tvsystem osd_gma_get_tv_system(void)
{
	enum tvsystem eTVMode = PAL;

	RET_CODE ret = vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32)&eTVMode);

	if (ret == RET_SUCCESS)
	{
		if (eTVMode == LINE_1080_24 || eTVMode == LINE_1152_ASS || eTVMode == LINE_1080_ASS || eTVMode == LINE_1080_50)
			eTVMode = LINE_1080_25;
		else if (eTVMode == LINE_1080_60)
			eTVMode = LINE_1080_30;
	}

	return eTVMode;
}

static int m_osd_gma_attach_flag = 0;

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
		default:
			break;
	}

	return format;
}

static int get_gma_bpp(enum osdcolor_mode eMode)
{
	int bpp = 1;

	switch(eMode)
	{
		case OSD_256_COLOR:
			bpp = 1;
			break;
		case OSD_HD_ARGB1555:
			bpp = 2;
			break;
		case OSD_HD_RGB565:
			bpp = 2;
			break;
		case OSD_HD_ARGB8888:
			bpp = 4;
			break;
		default:
			break;
	}

	return bpp;
}

static void rect_set(INT16 dst_width,
					INT16 dst_height,
					UINT8* dst_buf,
					struct osdrect *dst_rect,
					UINT32  data,
					UINT32 pitch,
					UINT32 bpp)
{
	UINT16 i,j;
//	UINT32 *buf32 = NULL;
//	UINT16 *buf16 = NULL;

	dst_buf += (pitch)*(dst_rect->u_top )+(dst_rect->u_left * bpp);

	for(i = 0; i<dst_rect->u_height;i++)
	{
		for(j = 0;j<dst_rect->u_width;j++)
		{
			if(bpp == 1)
			{
				*(dst_buf + j) = data & 0xFF;
			}
			else if(bpp == 2)
			{
				*(dst_buf + 2 * j) = data & 0xFF;
				*(dst_buf + 2 * j + 1) = (data>>8) & 0xFF;
			}
			else if(bpp == 4)
			{
				*(dst_buf + 4 * j) = data & 0xFF;
				*(dst_buf + 4 * j + 1) = (data>>8) & 0xFF;
				*(dst_buf + 4 * j + 2) = (data>>16) & 0xFF;
				*(dst_buf + 4 * j + 3) = (data>>24) & 0xFF;
			}
		}

		osal_cache_flush(dst_buf, bpp * dst_rect->u_width);

		dst_buf += pitch;
	}
}

static void rect_cpy(UINT16 dst_width,
					UINT16 dst_height,
					UINT8* dst_buf,
					UINT16 src_width,
					UINT16 src_height,
					UINT8* src_buf,
					struct osdrect *dst_rect,
					struct osdrect *src_rect,
					UINT32 pitch_dst,
					UINT32 pitch_src,
					UINT32 bpp)
{

	UINT16 i;
	UINT32 	uSrcBufOffset,uDstBufOffset;

	UINT32 len = dst_rect->u_width * bpp;

	uSrcBufOffset = (pitch_src * src_rect->u_top )+(src_rect->u_left * bpp);
	uDstBufOffset = (pitch_dst * dst_rect->u_top )+(dst_rect->u_left * bpp);

	for(i = 0; i<dst_rect->u_height;i++)
	{
		if(!((UINT32)(dst_buf+uDstBufOffset)&0x3) && !((UINT32)(src_buf+uSrcBufOffset)&0x3) && (0x03 <len))
		{
			UINT32 dwCount, dwLen;
			UINT32 *DestBuf,*SrcBuf;
			UINT8 * DestBufByte, *SrcBufByte;

			dwLen = (len&0xfffffffc)>>2;
			DestBuf=(UINT32 *)(dst_buf+uDstBufOffset);
			SrcBuf=(UINT32 *)(src_buf+uSrcBufOffset);
			for(dwCount=0;dwCount<dwLen;dwCount++)
			{
				*(DestBuf+dwCount)=*(SrcBuf+dwCount);
			}
			dwLen = len&0x3;
			DestBufByte = (UINT8 *)(dst_buf + uDstBufOffset + (dwCount<<2));
			SrcBufByte = (UINT8 *)(src_buf + uSrcBufOffset +(dwCount<<2));
			for(dwCount=0;dwCount<dwLen;dwCount++)
			{
				*(DestBufByte+dwCount)=*(SrcBufByte+dwCount);
			}
		}
		else if(!((UINT32)(dst_buf+uDstBufOffset)&0x1) && !((UINT32)(src_buf+uSrcBufOffset)&0x1) && (0x01 < len))
		{
			UINT32 wCount, wLen;
			UINT16 *DestBuf,*SrcBuf;
			UINT8 * DestBufByte, *SrcBufByte;

			wLen = (len&0xfffffffe)>>1;
			DestBuf=(UINT16 *)(dst_buf+uDstBufOffset);
			SrcBuf=(UINT16 *)(src_buf+uSrcBufOffset);
			for(wCount=0;wCount<wLen;wCount++)
			{
				*(DestBuf+wCount)=*(SrcBuf+wCount);
			}
			wLen = len&0x1;
			DestBufByte = (UINT8 *)(dst_buf + uDstBufOffset +(wCount<<1));
			SrcBufByte = (UINT8 *)(src_buf + uSrcBufOffset +(wCount<<1));
			for(wCount=0;wCount<wLen;wCount++)
			{
				*(DestBufByte+wCount)=*(SrcBufByte+wCount);
			}
		}
		else
		{
			MEMCPY(dst_buf+uDstBufOffset, src_buf+uSrcBufOffset, len);
		}

		osal_cache_flush(dst_buf+uDstBufOffset, len);

		uSrcBufOffset += pitch_src;
		uDstBufOffset += pitch_dst;
	}
}

#if 0
static int recreate_region(HANDLE hDev, pcosd_region_param pregion_param)
{
	struct osdrect rect;

	if(pregion_param->region_id >= MAX_REGION_NUM)
		goto FAIL;

	gmadrv_delete_region(hDev, pregion_param->region_id);

	rect.u_left = pregion_param->region_x;
	rect.u_top = pregion_param->region_y;
	rect.u_width = pregion_param->region_w;
	rect.u_height = pregion_param->region_h;
	if(gmadrv_create_region(hDev, pregion_param->region_id, &rect, NULL) != RET_SUCCESS)
		goto FAIL;

	return 1;

FAIL:
	return -1;
}
#endif

RET_CODE gmadrv_open(HANDLE hDev, struct osdpara *pOpenPara)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	GMA_LAYER_PARS layer_pars;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open)
	{
		OSD_PRF("osd_gma is already opend\n ");

		result = RET_SUCCESS;
		goto EXIT;
	}

	if(pOpenPara)
	{
		MEMCPY((void *)&(priv->default_para), pOpenPara, sizeof(*pOpenPara));
	}
	else
	{
		priv->default_para.e_mode = OSD_256_COLOR;
		priv->default_para.u_galpha = 0xFF;
		priv->default_para.u_galpha_enable = 0;
		priv->default_para.u_pallette_sel = 0;
	}

	if(priv->default_para.e_mode == OSD_256_COLOR)
	{
		priv->pallette_size = 256;
		priv->pallette_start = MALLOC(priv->pallette_size * 4);
		if(priv->pallette_start == NULL)
		{
			OSD_PRF("malloc pallette buffer fail\n");

			goto EXIT;
		}

		MEMSET((void *)priv->pallette_start, 0, priv->pallette_size * 4);
	}

	priv->gma_dev = (struct gma_device *)dev_get_by_id(HLD_DEV_TYPE_GMA, priv->index);
	if(priv->gma_dev == NULL)
	{
		OSD_PRF("get gma device fail\n");

		result = RET_FAILURE;
		goto EXIT;
	}

	MEMSET((void *)&layer_pars, 0, sizeof(layer_pars));
	layer_pars.format = get_gma_format(priv->default_para.e_mode);
	layer_pars.global_alpha_enable = 0;
	layer_pars.global_alpha_value = 0xFF;
	layer_pars.max_size.w = 720;
	layer_pars.max_size.h = 576;
	if(priv->gma_dev->open(priv->gma_dev, &layer_pars) != RET_SUCCESS)
	{
		OSD_PRF("open gma fail\n");

		result = RET_FAILURE;
		goto EXIT;
	}

	priv->open = 1;

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_close(HANDLE hDev)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	int i = 0;

	if(dev)
		priv = (struct osd_private_gma *)dev->priv;

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma device is already closed\n");

		result = RET_SUCCESS;
		goto EXIT;
	}

	if(priv->show_on)
	{
		priv->gma_dev->show(priv->gma_dev, 0);

		priv->show_on = 0;
	}

	for(i = 0;i < MAX_REGION_NUM;i++)
	{
		if(priv->region[i].valid)
		{
			gmadrv_delete_region(hDev, i);
		}
	}

	priv->gma_dev->close(priv->gma_dev);

	if(priv->pallette_start)
	{
		FREE(priv->pallette_start);

		priv->pallette_start = NULL;
	}

	result = RET_SUCCESS;

	priv->open = 0;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_io_ctl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	posd_region_param pregion_param = NULL;
    struct region_pars pars;
//    RET_CODE ret;
    struct osd_region_par *region = NULL;

	if(dev)
		priv = (struct osd_private_gma *)dev->priv;

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	OSD_PRF("osd_gma io cmd 0x%08x par 0x%08x\n", dwCmd, dwParam);

	switch(dwCmd)
	{
		case OSD_IO_SET_TRANS_COLOR:
			priv->trans_color = dwParam;
			break;
		case OSD_IO_SET_GLOBAL_ALPHA:
			result = priv->gma_dev->io_control(priv->gma_dev, GMA_IO_SET_GLOBAL_ALPHA, (dwParam & 0xFF));
			break;
		/*case OSD_IO_CREATE_REGION:
			pregion_param = (pcosd_region_param)dwParam;
			if(recreate_region(hDev, pregion_param) < 0)
			{
				OSD_PRF("recreate region fail\n");

				goto EXIT;
			}
			break;*/
        case OSD_IO_GET_REGION_INFO:

            pregion_param = (posd_region_param)dwParam;
            if (pregion_param == NULL)
                return RET_FAILURE;
            result = priv->gma_dev->get_region_info(priv->gma_dev, pregion_param->region_id, &pars);

            pregion_param->color_format = gmadrv_color_mode_to_ge(priv->default_para.e_mode);
        	pregion_param->galpha_enable = pars.global_alpha_enable;
        	pregion_param->global_alpha = pars.global_alpha_value;
        	pregion_param->pallette_sel = priv->default_para.u_pallette_sel;
        	pregion_param->region_x = pars.rect.x;
        	pregion_param->region_y = pars.rect.y;
        	pregion_param->region_w = pars.rect.w;
        	pregion_param->region_h = pars.rect.h;
        	pregion_param->bitmap_addr = pars.dmem_start;
        	pregion_param->pixel_pitch = pars.line_length;
        	pregion_param->bitmap_x = pars.rect.x;
        	pregion_param->bitmap_y = pars.rect.y;
        	pregion_param->bitmap_w = pars.rect.w;
        	pregion_param->bitmap_h = pars.rect.h;
			break;
        case OSD_IO_CREATE_REGION:
            pregion_param = (posd_region_param)dwParam;
            if (pregion_param == NULL)
            {
                return RET_FAILURE;
            }
            result = priv->gma_dev->io_control(priv->gma_dev, GMA_IO_CREATE_REGION, dwParam);
            priv->region[pregion_param->region_id].valid = 1;
            result = priv->gma_dev->get_region_info(priv->gma_dev, pregion_param->region_id, &pars);

            region = priv->region + pregion_param->region_id;
        	region->valid = 1;
        	region->bpp = get_gma_bpp(priv->default_para.e_mode);
        	region->mem_size = pars.dmem_len;
        	region->mem_start = (void *)((pars.dmem_start & 0x3FFFFFFF) | 0x80000000);
        	region->pitch = pars.line_length;
        	region->reg_rect.u_left = pars.rect.x;
        	region->reg_rect.u_top = pars.rect.y;
        	region->reg_rect.u_width = pars.rect.w;
        	region->reg_rect.u_height = pars.rect.h;

            break;
		default:
			OSD_PRF("don't support cmd 0x%08x\n", dwCmd);
			goto EXIT;
			break;
	}

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_get_para(HANDLE hDev,struct osdpara* ptPara)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (ptPara == NULL))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	MEMCPY((void *)ptPara, (void *)&priv->default_para, sizeof(priv->default_para));

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_show_on_off(HANDLE hDev,UINT8 uOnOff)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
		priv = (struct osd_private_gma *)dev->priv;

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}
	priv->show_on = (uOnOff) ? 1 : 0;
	result = priv->gma_dev->show(priv->gma_dev, priv->show_on);

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}
static int osd_clip(int x, int lower, int upper)
{
    return (x<lower) ? lower : (x>upper) ? upper : x;
}

static void osd_yuv2rgb_convert(UINT8 *bgra, const UINT8 *yuva, BOOL bVideoMatrix, BOOL bBt709)
{
    int uR,uG,uB;
    UINT8 Y, U, V;

    Y = yuva[0];
    U = yuva[1];
    V = yuva[2];

    if(!bVideoMatrix)
    {
        Y = osd_clip(Y,16,235);
        U = osd_clip(U,16,240);
        V = osd_clip(V,16,240);
        if(!bBt709)
        {
            uR = (298*(Y-16)+0*(U-128)+409*(V-128)+128)/256;
            uG= (298*(Y-16)-100*(U-128)-208*(V-128)+128)/256;
            uB= (298*(Y-16)+517*(U-128)+0*(V-128)+128)/256;
        }
        else
        {
            uR = (298*(Y-16)+0*(U-128)+459*(V-128)+128)/256;
            uG= (298*(Y-16)-54*(U-128)-136*(V-128)+128)/256;
            uB= (298*(Y-16)+541*(U-128)+0*(V-128)+128)/256;
        }
    }
    else
    {
        // Y = osd_clip(Y,0,255);
        // U = osd_clip(U,0,255);
        // V = osd_clip(V,0,255);
        if(!bBt709)
        {
            uR = (256*(Y)+0*(U-128)+351*(V-128)+128)/256;
            uG= (256*(Y)-86*(U-128)-179*(V-128)+128)/256;
            uB= (256*(Y)+444*(U-128)+0*(V-128)+128)/256;
        }
        else
        {
            uR = (256*(Y)+0*(U-128)+394*(V-128)+128)/256;
            uG= (256*(Y)-47*(U-128)-117*(V-128)+128)/256;
            uB= (256*(Y)+464*(U-128)+0*(V-128)+128)/256;
        }
    }

    bgra[2] = osd_clip(uR,0,255);
    bgra[1] = osd_clip(uG,0,255);
    bgra[0] = osd_clip(uB,0,255);
    bgra[3] = (yuva[3] << 4) + yuva[3];

}

RET_CODE gmadrv_set_pallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	struct pallette_pars pallette;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (pPallette == NULL) || (wColorN > priv->pallette_size))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

    UINT32 i = 0;
    if(bType == OSDDRV_YCBCR)
	{
        for (i = 0; i < wColorN; i++)
        {
            osd_yuv2rgb_convert(priv->pallette_start + i * 4, pPallette + i * 4, TRUE, FALSE);
        }
	}
    else
    {
        MEMCPY((void *)priv->pallette_start, pPallette, wColorN * 4);
    }

    priv->pallette_type = OSDDRV_RGB;
	pallette.pallette_buf = (UINT32)priv->pallette_start;
	pallette.alpha_level = GMA_PLT_ALPHA_256;
	pallette.color_num = wColorN;
	pallette.type = GMA_PLT_RGB;

	result = priv->gma_dev->set_pallette(priv->gma_dev, &pallette);

	OSD_PRF("pallette buf 0x%08x type %d num %d\n", (int)pPallette, bType, wColorN);

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_get_pallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (pPallette == NULL) || (wColorN > priv->pallette_size) )
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(bType == OSDDRV_YCBCR)
	{
		UINT8 *src = (UINT8 *)priv->pallette_start;
		UINT8 *dst = pPallette;
		int i = 0;

		for(i = 0;i < wColorN;i++)
		{
			*dst = *(src + i + 2);
			*(dst + 1) = *(src + i + 1);
			*(dst + 2) = *src;
			*(dst + 3) = (*(src + i + 3)>>3) & 0x0F;
		}
	}
	else
	{
		MEMCPY(pPallette, (void *)priv->pallette_start, wColorN * 4);
	}

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_modify_pallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	struct pallette_pars pallette;
    UINT8 pdst[4] = {0};
    UINT32 i = uIndex;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}
    i <<= 2;
    //pdst = (UINT8 *)priv->pallette_start + i;
    pdst[3] = uK;
    pdst[0] = uY;
    pdst[1] = uCb;
    pdst[2] = uCr;
	//*(((UINT32 *)priv->pallette_start) + uIndex) = (((uK<<3) & 0x78) +  ((uK>>1) & 0x07)<<24)
	//	|(uY<<16) | (uCb<<8) | uCr;
    osd_yuv2rgb_convert(priv->pallette_start + i, pdst, TRUE, FALSE);
	pallette.pallette_buf = (UINT32)priv->pallette_start;
	pallette.alpha_level = GMA_PLT_ALPHA_256;
	pallette.color_num = priv->pallette_size;
	pallette.type = GMA_PLT_RGB;
	result = priv->gma_dev->set_pallette(priv->gma_dev, &pallette);

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_create_region(HANDLE hDev,UINT8 uRegionId,struct osdrect* rect,struct osdpara*pOpenPara)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	struct osd_region_par *region = NULL;
	struct region_pars gma_region_pars;
    UINT32 mem_base = 0;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (rect == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device or argument fail\n");

		goto EXIT;
	}


	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(uRegionId >= MAX_REGION_NUM)
	{
		OSD_PRF("region id is unvalid value %d\n", uRegionId);

		goto EXIT;
	}

	if(priv->region[uRegionId].valid)
	{
		OSD_PRF("this reigon %d has been created\n", uRegionId);

		result = RET_SUCCESS;

		goto EXIT;
	}

	MEMSET((void *)&gma_region_pars, 0, sizeof(gma_region_pars));
	gma_region_pars.dmem_len = rect->u_width * rect->u_height * get_gma_bpp(priv->default_para.e_mode);
    gma_region_pars.dmem_start = mem_base& 0x3fffffff;
	gma_region_pars.global_alpha_enable = 0;
	gma_region_pars.global_alpha_value = 0xFF;
	gma_region_pars.rect.x = rect->u_left;
	gma_region_pars.rect.y = rect->u_top;
	gma_region_pars.rect.w = rect->u_width;
	gma_region_pars.rect.h = rect->u_height;
	gma_region_pars.line_length = rect->u_width * get_gma_bpp(priv->default_para.e_mode);
	result = priv->gma_dev->create_region(priv->gma_dev, uRegionId, &gma_region_pars);
	if(result != RET_SUCCESS)
	{
		OSD_PRF("gma create region fail\n");

		goto EXIT;
	}
    priv->region[uRegionId].valid = 1;
	result = priv->gma_dev->get_region_info(priv->gma_dev, uRegionId, &(gma_region_pars));
	if((result != RET_SUCCESS)
		|| (gma_region_pars.dmem_len == 0)
		|| (gma_region_pars.dmem_start == 0)
		|| (gma_region_pars.line_length == 0)
		|| (gma_region_pars.dmem_start & 0x20000000))
	{
		OSD_PRF("gma get info fail or some pars fail dmem_len 0x%08x dmem_start 0x%08x line_length %d\n"
			, gma_region_pars.dmem_len, gma_region_pars.dmem_start, gma_region_pars.line_length);

		goto EXIT;
	}

	region = priv->region + uRegionId;
	region->valid = 1;
	region->bpp = get_gma_bpp(priv->default_para.e_mode);
	region->mem_size = gma_region_pars.dmem_len;
	region->mem_start = (void *)((gma_region_pars.dmem_start & 0x3FFFFFFF) | 0x80000000);
	region->pitch = gma_region_pars.line_length;
	region->reg_rect.u_left = rect->u_left;
	region->reg_rect.u_top = rect->u_top;
	region->reg_rect.u_width = rect->u_width;
	region->reg_rect.u_height = rect->u_height;

	OSD_PRF("gma region done dev_id %d region_id %d mem_start 0x%08x pitch 0x%08x\n"
		, priv->index, uRegionId, region->mem_start, region->pitch);

	{
		UINT32 fill_color = 0;
		struct osdrect write_rect;
		struct osd_region_par *osd_reg_par = priv->region + uRegionId;

		if(priv->default_para.e_mode == OSD_256_COLOR)
		{
			fill_color = priv->trans_color;
		}

		write_rect.u_left = 0;
		write_rect.u_top = 0;
		write_rect.u_width = rect->u_width;
		write_rect.u_height = rect->u_height;
		rect_set(osd_reg_par->reg_rect.u_width, osd_reg_par->reg_rect.u_height, osd_reg_par->mem_start
			,&write_rect,fill_color, osd_reg_par->pitch, osd_reg_par->bpp);
	}

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_delete_region(HANDLE hDev,UINT8 uRegionId)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region %d has been delete\n", uRegionId);

		result = RET_SUCCESS;

		goto EXIT;
	}

	priv->gma_dev->delete_region(priv->gma_dev, uRegionId);

	priv->region[uRegionId].valid = 0;

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_set_region_pos(HANDLE hDev,UINT8 uRegionId,struct osdrect *rect)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	struct region_pars gma_region_pars;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (rect == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device or argument fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

	MEMSET((void *)&gma_region_pars, 0, sizeof(gma_region_pars));
	priv->gma_dev->get_region_info(priv->gma_dev, uRegionId, &gma_region_pars);
	gma_region_pars.rect.x = rect->u_left;
	gma_region_pars.rect.y = rect->u_top;
	result = priv->gma_dev->set_region_info(priv->gma_dev, uRegionId, &gma_region_pars);
	if(result != RET_SUCCESS)
	{
		OSD_PRF("set gma region info fail\n");

		goto EXIT;
	}

	priv->region[uRegionId].reg_rect.u_left = rect->u_left;
	priv->region[uRegionId].reg_rect.u_top = rect->u_top;
	priv->region[uRegionId].reg_rect.u_width = rect->u_width;
	priv->region[uRegionId].reg_rect.u_height = rect->u_height;

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_get_region_pos(HANDLE hDev,UINT8 uRegionId,struct osdrect* rect)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (rect == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device or argument fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

	rect->u_left = priv->region[uRegionId].reg_rect.u_left;
	rect->u_top = priv->region[uRegionId].reg_rect.u_top;
	rect->u_width = priv->region[uRegionId].reg_rect.u_width;
	rect->u_height = priv->region[uRegionId].reg_rect.u_height;

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_region_show(HANDLE hDev,UINT8 uRegionId,BOOL bOn)
{
	return RET_FAILURE;
}

RET_CODE gmadrv_region_write(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct osdrect *rect)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

	{
		struct osdrect src, dst;
		struct osd_region_par *osd_reg_par = priv->region + uRegionId;

		src.u_left = rect->u_left - pVscr->v_r.u_left;
		src.u_top = rect->u_top - pVscr->v_r.u_top;
		src.u_width = rect->u_width;
		src.u_height = rect->u_height;
		dst.u_left = rect->u_left;
		dst.u_top = rect->u_top;
		dst.u_width = rect->u_width;
		dst.u_height = rect->u_height;
		rect_cpy(osd_reg_par->reg_rect.u_width, osd_reg_par->reg_rect.u_height, osd_reg_par->mem_start
			, pVscr->v_r.u_width, pVscr->v_r.u_height, pVscr->lpb_scr
			, &dst, &src, osd_reg_par->pitch
			, osd_reg_par->bpp * pVscr->v_r.u_width, osd_reg_par->bpp);
	}

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_region_read(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct osdrect *rect)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

	{
		struct osdrect src, dst;
		struct osd_region_par *osd_reg_par = priv->region + uRegionId;

		src.u_left = rect->u_left;
		src.u_top = rect->u_top;
		src.u_width = rect->u_width;
		src.u_height = rect->u_height;
		dst.u_left = rect->u_left - pVscr->v_r.u_left;
		dst.u_top = rect->u_top - pVscr->v_r.u_top;
		dst.u_width = rect->u_width;
		dst.u_height = rect->u_height;

		rect_cpy(pVscr->v_r.u_width,pVscr->v_r.u_height,pVscr->lpb_scr,
			osd_reg_par->reg_rect.u_width, osd_reg_par->reg_rect.u_height, osd_reg_par->mem_start
			,&dst,&src, osd_reg_par->bpp * pVscr->v_r.u_width, osd_reg_par->pitch, osd_reg_par->bpp);
	}


	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

RET_CODE gmadrv_region_fill(HANDLE hDev,UINT8 uRegionId,struct osdrect *rect, UINT32 uColorData)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}


	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

	{
		struct osd_region_par *osd_reg_par = priv->region + uRegionId;

		rect_set(osd_reg_par->reg_rect.u_width, osd_reg_par->reg_rect.u_height, osd_reg_par->mem_start
			,rect,uColorData, osd_reg_par->pitch, osd_reg_par->bpp);
	}

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

//#ifdef BIDIRECTIONAL_OSD_STYLE
RET_CODE gmadrv_region_write_inverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct osdrect *rect)
{
	return  RET_FAILURE;
}

RET_CODE gmadrv_region_read_inverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct osdrect *rect)
{
	return  RET_FAILURE;
}

RET_CODE gmadrv_region_fill_inverse(HANDLE hDev,UINT8 uRegionId,struct osdrect *rect, UINT32 uColorData)
{
	return  RET_FAILURE;
}
//#endif

RET_CODE gmadrv_region_write2(HANDLE hDev,UINT8 uRegionId,UINT8* pSrcData,UINT16 uSrcWidth,UINT16 uSrcHeight,struct osdrect *pSrcRect,struct osdrect *pDestRect)
{
	INT32 result = RET_SUCCESS;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (uRegionId >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[uRegionId].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", uRegionId);

		goto EXIT;
	}

    struct osdrect src, dst;
	struct osd_region_par *osd_reg_par = priv->region + uRegionId;

    src.u_left = pSrcRect->u_left;
	src.u_top = pSrcRect->u_top;
	src.u_width = pSrcRect->u_width;
	src.u_height = pSrcRect->u_height;
	dst.u_left = pDestRect->u_left;
	dst.u_top = pDestRect->u_top;
	dst.u_width = pDestRect->u_width;
	dst.u_height = pDestRect->u_height;
	rect_cpy(osd_reg_par->reg_rect.u_width, osd_reg_par->reg_rect.u_height, osd_reg_par->mem_start
		, pSrcRect->u_width, pSrcRect->u_height, pSrcData
		, &dst, &src, osd_reg_par->pitch
		, osd_reg_par->bpp * pSrcRect->u_width, osd_reg_par->bpp);
    EXIT:
	OSD_PRF("OSDDrv_RegionWrite2\n");
	return result;

}

RET_CODE gmadrv_draw_hor_line(HANDLE hDev, UINT8 uRegionId, UINT32 x, UINT32 y, UINT32 width, UINT32 color)
{
    OSD_PRF("OSDDrv_DrawHorLine no support!!!!!!!!!!!!!\n");
	return  RET_FAILURE;
}

RET_CODE gmadrv_scale(HANDLE hDev,UINT32 uScaleCmd,UINT32 uScaleParam)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;
	struct scale_pars scale_info;
	pcosd_scale_param pscale_param = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if(priv == NULL)
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	MEMSET((void *)&scale_info, 0, sizeof(scale_info));
	switch(uScaleCmd)
	{
		case OSD_VSCALE_OFF:
			scale_info.h_dst = 1;
			scale_info.v_dst = 1;
			scale_info.h_src = 1;
			scale_info.v_src = 1;
			break;
		case OSD_VSCALE_TTX_SUBT:
		{
			int screen_width, tv_sys;

			screen_width = osd_gma_get_screen_pitch(priv);
			tv_sys = osd_gma_get_tv_system();
			pscale_param = osd_gma_get_scale_param(tv_sys, screen_width);
			scale_info.scale_type = 1;//GMA_SCALE_FILTER;
			scale_info.h_dst = pscale_param->h_mul;
			scale_info.h_src = pscale_param->h_div;
			scale_info.v_dst = pscale_param->v_mul;
			scale_info.v_src = pscale_param->v_div;
			break;
		}
		case OSD_SCALE_WITH_PARAM:
			pscale_param = (pcosd_scale_param)uScaleParam;
			if(pscale_param == NULL)
			{
				goto EXIT;
			}

			if((pscale_param->h_div == 0)
				|| (pscale_param->h_mul == 0)
				|| (pscale_param->v_div == 0)
				|| (pscale_param->v_mul == 0))
			{
				goto EXIT;
			}

            scale_info.scale_type = 1;//GMA_SCALE_FILTER;
			scale_info.h_dst = pscale_param->h_mul;
			scale_info.h_src = pscale_param->h_div;
			scale_info.v_dst = pscale_param->v_mul;
			scale_info.v_src = pscale_param->v_div;
			break;
		default:
			OSD_PRF("invalid scale cmd %d\n", uScaleCmd);
			goto EXIT;
	}
    if(priv->gma_dev->scale)
	    result = priv->gma_dev->scale(priv->gma_dev, &scale_info);

EXIT:
	OSD_PRF("gma scale result is %d\n", result);
	return result;
}

RET_CODE gmadrv_set_clip(HANDLE hDev,enum clipmode clipmode,struct osdrect *pRect)
{
    SDBBP();
	return  RET_FAILURE;
}
RET_CODE gmadrv_clear_clip(HANDLE hDev)
{
    SDBBP();
	return  RET_FAILURE;
}

RET_CODE gmadrv_get_region_addr(HANDLE hDev,UINT8 region_idx,UINT16 y, UINT32 *addr)
{
	INT32 result = RET_FAILURE;
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private_gma *priv = NULL;

	if(dev)
	{
		priv = (struct osd_private_gma *)dev->priv;
	}

	if((priv == NULL) || (addr == NULL) || (region_idx >= MAX_REGION_NUM))
	{
		OSD_PRF("osd_gma device fail\n");

		goto EXIT;
	}

	if(priv->open == 0)
	{
		OSD_PRF("osd_gma is not opened\n");

		goto EXIT;
	}

	if(priv->region[region_idx].valid == 0)
	{
		OSD_PRF("this region is unvalid %d\n", region_idx);

		goto EXIT;
	}

	*addr = (UINT32 )(priv->region[region_idx].mem_start + y * priv->region[region_idx].pitch);

	result = RET_SUCCESS;

EXIT:
	OSD_PRF("result is %d\n", result);
	return result;
}

#ifdef SEE_CPU
/*if someone remove these module from see:LLD_OSD_M31,LLD_OSD_M36F,LLD_GMA_M3921,LLD_GMA_M3921,LLD_OSD_M36F, maybe a
compile erro with gma_attach_m36f, please change "if 0" to "if 1"*/
#if 1  /////here
int gma_attach_m36f(int layerNum, UINT32 mem_base, UINT32 mem_size)
{
    return 0;
}
#endif

void osddrv_attach_gma(int layerNum, UINT32 mem_base, UINT32 mem_size)
#else
void gmadrv_attach(int layerNum, UINT32 mem_base, UINT32 mem_size)
#endif
{
	struct osd_device *dev;
	struct osd_private_gma *priv = NULL;
#ifdef SEE_CPU
	char m_osd_name[MAX_DEV_NUM][HLD_MAX_NAME_SIZE] = {
		 	{'O'-'/','S'-'/','D'-'/','_'-'/','S'-'/','E'-'/','E'-'/','_'-'/','0'-'/', '\0'},
		  	{'O'-'/','S'-'/','D'-'/','_'-'/','S'-'/','E'-'/','E'-'/','_'-'/','1'-'/', '\0'},
		 };
	int i = 0;
	int j = 0;
	for (i = 0; i < MAX_DEV_NUM; i++)
	{
		for (j = 0; j < HLD_MAX_NAME_SIZE; j++)
		{
			if (m_osd_name[i][j] != '\0')
			{
				m_osd_name[i][j] += '/';
			}
		}
	}
#else
	char m_osd_name[MAX_DEV_NUM][HLD_MAX_NAME_SIZE]
		= {"OSD_DEV_0", "OSD_DEV_1"};
#endif


	dev = (struct osd_device *)dev_alloc(m_osd_name[layerNum],HLD_DEV_TYPE_OSD,sizeof(struct osd_device));
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

	priv->index = layerNum;
	dev->priv = (void *)priv;
	dev->next = NULL;
	dev->flags = 0;
	STRCPY(dev->name, m_osd_name[layerNum]);

	if(dev_register(dev) != RET_SUCCESS)
	{
		OSD_PRF("register osd dev fail\n");
		return;
	}

	m_osd_gma_dev[layerNum] = dev;
	m_osd_gma_priv[layerNum] = priv;

    gma_attach_m36f(layerNum, mem_base, mem_size);
	m_osd_gma_attach_flag = 1;
}

void OSDDrv_Dettach_Gma(void)
{
	int i = 0;

	if(m_osd_gma_attach_flag == 0)
		return;

	for(i = 0;i < MAX_DEV_NUM;i++)
	{
		if(m_osd_gma_priv[i])
			FREE((void *)m_osd_gma_priv[i]);

		if(m_osd_gma_dev[i])
			dev_free((void *)m_osd_gma_dev[i]);
	}

	m_osd_gma_attach_flag = 0;
}

#if 0
void ts_osd_set_auto_scale_par(int *dst_w, int *dst_h)
{
	struct vpo_io_get_info vpo_info;

	vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32)(&vpo_info));

	switch(vpo_info.tvsys)
	{
		case PAL:
		case PAL_N:
		case PAL_NC:
		case LINE_576P_50_VESA:
			*dst_w = 720;
			*dst_h = 576;
			break;
		case NTSC:
		case NTSC_443:
		case PAL_60:
		case PAL_M:
			*dst_w = 720;
			*dst_h = 480;
			break;
		case LINE_720_25:
		case LINE_720_30:
		case LINE_720P_60_VESA:
			*dst_w = 1280;
			*dst_h = 720;
			break;
		case LINE_1080_25:
		case LINE_1080_30:
		case LINE_1080_24:
		case LINE_1080_50:
		case LINE_1080_60:
		case LINE_1080P_60_VESA:
			*dst_w = 1920;
			*dst_h = 1080;
			break;
		default:
			return -1;
	}

	OSD_PRF("auto scale tv sys %d pro %d dst_w %d dst_h %d\n", vpo_info.tvsys, vpo_info.bprogressive, *dst_w, *dst_h);
}

void OSD_Gma_test(void)
{
	struct osd_device *dev = NULL;
	struct OSDPara para;
	struct OSDRect rect;

	OSDDrv_Attach_Gma();

	dev = m_osd_gma_dev[1];

	OSDDrv_Close((HANDLE)dev);

	memset((void *)&para, 0, sizeof(para));
	para.eMode = OSD_256_COLOR;
	para.uGAlpha = 0x7f;
	para.uGAlphaEnable = 0;
	para.uPalletteSel = 0;
	OSDDrv_Open((HANDLE)dev, &para);

	memset((void *)&rect, 0, sizeof(rect));
	rect.uLeft = 0;
	rect.uTop = 0;
	rect.uWidth = 1920;
	rect.uHeight = 1080;
	OSDDrv_CreateRegion((HANDLE)dev, 0, &rect, NULL);

	memset((void *)&rect, 0, sizeof(rect));
	rect.uLeft = 0;
	rect.uTop = 0;
	rect.uWidth = 1920;
	rect.uHeight = 1080;
	OSDDrv_RegionFill((HANDLE)dev, 0, &rect, 0x00);

	{
		osd_scale_param scale_param;
		int dst_w, dst_h;
		int scale_flag_h = 0, scale_flag_v = 0;

		ts_osd_set_auto_scale_par(&dst_w, &dst_h);

		scale_param.tv_sys = OSD_PAL;
		scale_param.h_div = (unsigned short)rect.uWidth;
		scale_param.v_div = (unsigned short)rect.uHeight;
		if(scale_flag_h)
			scale_param.h_mul = (unsigned short)dst_w;
		else
			scale_param.h_mul = (unsigned short)rect.uWidth;

		if(scale_flag_v)
			scale_param.v_mul = (unsigned short)dst_h;
		else
			scale_param.v_mul = (unsigned short)rect.uHeight;

		if(scale_flag_h || scale_flag_v)
			OSDDrv_Scale((HANDLE)dev, OSD_SCALE_WITH_PARAM, &scale_param);
	}

	{
		unsigned int pallette[256];

		memset(pallette, 0, 1024);

		pallette[0] = 0xFFFF0000;
		pallette[1] = 0xFF0000FF;

		OSDDrv_SetPallette((HANDLE)dev, pallette, 256, OSDDRV_RGB);
	}

	memset((void *)&rect, 0, sizeof(rect));
	rect.uLeft = 100;
	rect.uTop = 100;
	rect.uWidth = 200;
	rect.uHeight = 200;
	OSDDrv_RegionFill((HANDLE)dev, 0, &rect, 1);

	OSDDrv_ShowOnOff((HANDLE)dev, 1);
}
#endif

#endif

