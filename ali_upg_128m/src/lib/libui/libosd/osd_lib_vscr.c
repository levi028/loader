/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_vscr.c
*
*    Description: implement VSCR relative function.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef PC_SIMULATE
#include <libosd/osd_lib.h>
#include <libosd/osd_common_draw.h>
#include <string.h>
#include <osd_rsc.h>
#else
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <api/libc/string.h>
#include "osd_lib_internal.h"
#endif
#include <api/libchar/lib_char.h>



static UINT8        *osd_vscr_buffer[OSD_DRAW_TASK_MAX];
static VSCR         g_vscr[OSD_DRAW_TASK_MAX];
VSCR g_vscr_bak;
OSAL_ID sema_vscr_bak = OSAL_INVALID_ID;
#define VSCR_BAK_LOCK()     osal_mutex_lock(sema_vscr_bak, OSAL_WAIT_FOREVER_TIME)
#define VSCR_BAK_UNLOCK()   osal_mutex_unlock(sema_vscr_bak)

////////////////////////////////////////////////////////////////////////
// virtual screen related functions

UINT32 OSD_GET_VSCR_STIDE(VSCR *p_vscr)
{
    if(NULL == p_vscr)
    {
        return 0;
    }
    return osd_get_pitch_color_mode(p_vscr->b_color_mode, p_vscr->v_r.u_width);
}

UINT32 OSD_GET_VSCR_SIZE(struct osdrect *p_rect, enum osdcolor_mode b_color_mode)
{
    UINT32 pitch = 0;

    if(NULL == p_rect)
    {
        return 0;
    }
    pitch = osd_get_pitch_color_mode(b_color_mode, p_rect->u_width);
    return (pitch * p_rect->u_height);
}

/*   ***********************************************************************/
/*   This function used to write data from pVscr2 to g_vscr_bak.
/*   Don't REMOVE or MODIFY it, but can understand it
/*   ADD TIME: 2016.9.6              ---- dean.zhang
/*   ***********************************************************************/   

RET_CODE vscr_data_backup(VSCR *pVscr1,VSCR *pVscr2)
{
       RET_CODE ret = RET_SUCCESS;

	#ifdef OSD_16BIT_SUPPORT
	UINT16* dst=NULL;
	UINT16* src=NULL;
	#else	 
	UINT32* dst=NULL;
	UINT32* src=NULL;
	#endif
	struct osdrect rc_src;
	struct osdrect rc_dst;
	INT32 i,j;
	INT32 offset_src;
	INT32 offset_dst;

       VSCR_BAK_LOCK();

       pVscr1->b_block_id = pVscr2->b_block_id;
       pVscr1->b_color_mode = pVscr2->b_color_mode;
       pVscr1->b_draw_mode = pVscr2->b_draw_mode;
       pVscr1->update_pending = pVscr2->update_pending;

	rc_dst.u_left		= pVscr1->v_r.u_left;
	rc_dst.u_top		= pVscr1->v_r.u_top;
	rc_dst.u_width		= pVscr1->v_r.u_width;
	rc_dst.u_height		= pVscr1->v_r.u_height;
	
	rc_src.u_left		= pVscr2->v_r.u_left;
	rc_src.u_top		= pVscr2->v_r.u_top;
	rc_src.u_width		= pVscr2->v_r.u_width;
	rc_src.u_height		= pVscr2->v_r.u_height;
	

       if (!osd_rect_in_rect(&rc_dst, &rc_src))
       {
            VSCR_BAK_UNLOCK();
            return RET_FAILURE;
       }
	#ifdef OSD_16BIT_SUPPORT
	src = (UINT16*)pVscr2->lpb_scr;
	dst = (UINT16*)pVscr1->lpb_scr;
	#else              
	src	= (UINT32*)pVscr2->lpb_scr;
    dst	= (UINT32*)pVscr1->lpb_scr;
	#endif

	offset_src = 0;
	offset_dst = (rc_src.u_top-rc_dst.u_top)*rc_dst.u_width + (rc_src.u_left-rc_dst.u_left);
	
	src += offset_src;
	dst	+= offset_dst;
	for(i=0; i<rc_src.u_height; i++)
	{
         
	#ifdef OSD_16BIT_SUPPORT
        MEMCPY((UINT8 *)dst, (UINT8 *)src, rc_src.u_width*2);
	#else
        MEMCPY((UINT8 *)dst, (UINT8 *)src, rc_src.u_width*4);
    #endif
		src += rc_src.u_width;
		dst += rc_dst.u_width;
	}	
    
       VSCR_BAK_UNLOCK();
       return ret;
}

/*   ***********************************************************************/
/*   This function used to write data from g_vscr_bak to pVscr1.
/*   Don't REMOVE or MODIFY it, but can understand it
/*   ADD TIME: 2016.9.6              ---- dean.zhang
/*   ***********************************************************************/   

RET_CODE vscr_data_restore(VSCR *pVscr1,VSCR *pVscr2, BOOL update)
{
       RET_CODE ret = RET_SUCCESS;
#ifdef OSD_16BIT_SUPPORT
	UINT16* dst=NULL;
	UINT16* src=NULL;
#else
	UINT32* dst=NULL;
	UINT32* src=NULL;
#endif
	struct osdrect rc_src;
	struct osdrect rc_dst;
	INT32 i,j;
	INT32 offset_src;
	INT32 offset_dst;
    UINT32 ptn_x, ptn_y;
    
       VSCR_BAK_LOCK();

	rc_dst.u_left		= pVscr1->v_r.u_left;
	rc_dst.u_top		= pVscr1->v_r.u_top;
	rc_dst.u_width		= pVscr1->v_r.u_width;
	rc_dst.u_height		= pVscr1->v_r.u_height;
	
	rc_src.u_left		= pVscr2->v_r.u_left;
	rc_src.u_top		= pVscr2->v_r.u_top;
	rc_src.u_width		= pVscr2->v_r.u_width;
	rc_src.u_height		= pVscr2->v_r.u_height;
	
	if (!osd_rect_in_rect(&rc_src, &rc_dst))
	{
	    VSCR_BAK_UNLOCK();
	    return RET_FAILURE;
	}
	#ifdef OSD_16BIT_SUPPORT
	src	= (UINT16*)pVscr2->lpb_scr;
    dst	= (UINT16*)pVscr1->lpb_scr;	
	#else	
	src	= (UINT32*)pVscr2->lpb_scr;
    dst	= (UINT32*)pVscr1->lpb_scr;
	#endif
	offset_src = (rc_dst.u_top-rc_src.u_top)*rc_src.u_width + (rc_dst.u_left-rc_src.u_left);
	offset_dst = 0;
	
	src += offset_src;
	dst	+= offset_dst;
	for(i=0; i<rc_dst.u_height; i++)
	{
              /*
		for(j=0;j<rc_dst.u_width;j++)
		{
			dst[j] = src[j];
		}
		*/
	#ifdef OSD_16BIT_SUPPORT
	    MEMCPY((UINT8 *)dst, (UINT8 *)src, rc_dst.u_width*2);
	#else
        MEMCPY((UINT8 *)dst, (UINT8 *)src, rc_dst.u_width*4);
    #endif
		src += rc_src.u_width;
		dst += rc_dst.u_width;
	}	
    //used to write back data cache to DRAM by given the address and byte length
       if (update)
	#ifdef OSD_16BIT_SUPPORT
            osal_cache_flush(pVscr1->lpb_scr, pVscr1->v_r.u_width*pVscr1->v_r.u_height*2);
	#else
            osal_cache_flush(pVscr1->lpb_scr, pVscr1->v_r.u_width*pVscr1->v_r.u_height*4);
    #endif
       
       VSCR_BAK_UNLOCK();
       return ret;
}

void osd_update_vscr(VSCR *p_vscr)
{
    if ((NULL != p_vscr) && (NULL != p_vscr->lpb_scr))
    {
#ifdef SUPPORT_DRAW_EFFECT
        if (p_vscr->update_pending)
        {
            if (p_vscr_head != NULL)
            {
                osd_draw_mode(p_vscr, p_vscr_head);
                p_vscr_head = NULL;
            }
            else
            {
                osdlib_region_write(p_vscr, &p_vscr->v_r);
                p_vscr->update_pending = 0;
                p_vscr->b_draw_mode  = 0;
            }
        }
#else
        if (p_vscr->update_pending)
        {
            osdlib_region_write(p_vscr, &p_vscr->v_r);
            p_vscr->update_pending = 0;
        }
#endif
    }
}

void osd_update_vscr_bakeup(void)
{
    osdlib_region_write(&g_vscr_bak, &g_vscr_bak.v_r);
}
void osd_update_vscr_ext(VSCR *p_vscr, UINT8 region_id)
{
    if ((NULL != p_vscr) && (p_vscr->lpb_scr != NULL))
    {
        if (p_vscr->update_pending)
        {
#ifdef BIDIRECTIONAL_OSD_STYLE
            if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
            {
                osddrv_region_write_inverse(osd_get_cur_device_handle(), region_id, p_vscr, &p_vscr->v_r);
            }
            else
#endif
                osddrv_region_write(osd_get_cur_device_handle(), region_id, p_vscr, &p_vscr->v_r);
            p_vscr->update_pending = 0;
        }
    }
}


void osd_init_vscr_color(LPVSCR p_gvscr, UINT32 color)
{
    UINT16      ture_color  = color;
    UINT32      i           = 0;
    UINT16      *p          = NULL;
    UINT32      size        = 0;
    UINT8       clut_color  = 0;
	UINT32      pixel_size = 0;

    if ((NULL == p_gvscr) || (NULL == p_gvscr->lpb_scr))
    {
        return;
    }

    size = OSD_GET_VSCR_SIZE(&p_gvscr->v_r, p_gvscr->b_color_mode);
	pixel_size = osd_get_pixel_size(p_gvscr->b_color_mode);
	
	if(0 == pixel_size)
	{
		return;
	}
	
    if (osd_color_mode_is_clut(p_gvscr->b_color_mode))
    {
        clut_color = color;
        MEMSET(p_gvscr->lpb_scr, clut_color, size);
    }
    else if (osd_color_mode_is16bit(p_gvscr->b_color_mode))
    {
    	p   = (UINT16 *)p_gvscr->lpb_scr;
        for (i = 0; i < size/pixel_size; i++)
        {
            *p = ture_color;
            p++;
        }
    }
    else
    {
    	p   = (UINT16 *)p_gvscr->lpb_scr;
        for (i = 0; i < size/pixel_size; i++)
        {
            *p = ture_color;
            p++;
        }
    }
}



BOOL osdget_local_vscr(LPVSCR lp_vscr, UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
    UINT32          vscr_idx = OSAL_INVALID_ID;

    if(NULL == lp_vscr)
    {
        return FALSE;
    }
    /*MultiTask OSD draw supported*/
    vscr_idx = (UINT32)osal_task_get_current_id();

    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        return FALSE;
    }

    osd_set_rect(&lp_vscr->v_r, x, y, w, h);
    //lpVscr->bColorMode = g_osd_region_info.tOpenPara.eMode;
    lp_vscr->b_color_mode = osd_get_cur_color_mode();
    if (OSD_GET_VSCR_SIZE(&lp_vscr->v_r, lp_vscr->b_color_mode)
            > OSD_VSRC_MEM_MAX_SIZE)
    {
        lp_vscr->lpb_scr = NULL;
        return FALSE;
    }

    if ((g_vscr[vscr_idx - 1].update_pending)
            && (NULL != g_vscr[vscr_idx - 1].lpb_scr))
    {
        osdlib_region_write(&g_vscr[vscr_idx - 1], &g_vscr[vscr_idx - 1].v_r);
        g_vscr[vscr_idx - 1].update_pending = FALSE;
        g_vscr[vscr_idx - 1].lpb_scr = NULL;
    }

    lp_vscr->lpb_scr = osd_vscr_buffer[vscr_idx - 1];
    osd_set_rect2(&g_vscr[vscr_idx - 1].v_r,  &lp_vscr->v_r);
    lp_vscr->update_pending = FALSE;

    ASSERT(lp_vscr->lpb_scr);
    //if(OSD_INVALID_REGION != g_osd_region_info.region_id)
    if (OSD_INVALID_REGION != osd_get_cur_region())
    {
        osdlib_region_read(lp_vscr, &lp_vscr->v_r);
    }

    return TRUE;
}


VSCR *osd_get_vscr(struct osdrect *p_rect, UINT32 b_flag)
{
    VSCR            *p_gvscr     = NULL;
    UINT32          vscr_idx    = OSAL_INVALID_ID ;
    UINT32          mem_size_r  = 0;
    struct osdrect  corner;

    osd_set_rect(&corner, 0, 0, 0, 0);
#ifdef GE_DRAW_OSD_LIB
    vscr_idx = (UINT32)osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return NULL;
    }

    p_gvscr = &g_vscr[vscr_idx - 1];
    p_gvscr->b_color_mode = osd_get_cur_color_mode();
    mem_size_r = OSD_GET_VSCR_SIZE(p_rect, p_gvscr->b_color_mode);

    if (p_gvscr->lpb_scr) /* Global virtual screen is valid. */
    {
        if (osd_rect_in_rect(&p_gvscr->v_r, p_rect)) /* Global's virtual screen cover the area that requied */
        {
            return p_gvscr;
        }
        else    /* Global's virtual screen can't cover the area that requied */
        {
            osd_update_vscr(p_gvscr);
            p_gvscr->lpb_scr = NULL;
        }
    }

    osd_set_rect2(&p_gvscr->v_r, p_rect);
    if (OSD_DIRDRAW == b_flag)
    {
        p_gvscr->lpb_scr = NULL;
    }
    else
    {
        if (mem_size_r > OSD_VSRC_MEM_MAX_SIZE)
        {
            p_gvscr->lpb_scr = NULL;
        }
        else
        {
            p_gvscr->lpb_scr = osd_vscr_buffer[vscr_idx - 1];
        }
    }

    if (p_gvscr->lpb_scr != NULL)
    {
        if (osd_get_vscr_state())
        {
            if (b_flag & OSD_GET_BACK)
            {
                osdlib_region_read(p_gvscr, p_rect);

            }
            else if (b_flag & OSD_GET_CORNER)
            {
                if(NULL == p_rect)
                {
                    return NULL;
                }
                osd_set_rect(&corner, p_rect->u_left, p_rect->u_top, CIRCLE_PIX * 2, p_rect->u_height);
                osdlib_region_read(p_gvscr, &corner);

                corner.u_left = p_rect->u_left + p_rect->u_width - CIRCLE_PIX * 2;
                osdlib_region_read(p_gvscr, &corner);

            }
            else
            {
                MEMSET(p_gvscr->lpb_scr, OSD_TRANSPARENT_COLOR_BYTE, mem_size_r);
            }
        }
        osd_region_param    region;
        UINT32    __MAYBE_UNUSED__    pitch_size;
        if (RET_FAILURE == osddrv_io_ctl(osd_get_cur_device_handle(), OSD_IO_GET_REGION_INFO, (UINT32)&region))
        {
            p_gvscr->lpb_scr = NULL;
        }
        else
        {
            pitch_size = OSD_GET_VSCR_STIDE(p_gvscr);

        }


    }

    p_gvscr->update_pending = FALSE;

#else

    vscr_idx = (UINT32)osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
        return NULL;
    }

    p_gvscr = &g_vscr[vscr_idx - 1];
    p_gvscr->b_color_mode = osd_get_cur_color_mode();
    mem_size_r = OSD_GET_VSCR_SIZE(p_rect, p_gvscr->b_color_mode);

    if (p_gvscr->lpb_scr) /* Global virtual screen is valid. */
    {
        if (osd_rect_in_rect(&p_gvscr->v_r, p_rect)) /* Global's virtual screen cover the area that requied */
        {
            return p_gvscr;
        }
        else    /* Global's virtual screen can't cover the area that requied */
        {
            osd_update_vscr(p_gvscr);
            p_gvscr->lpb_scr = NULL;
        }
    }

    osd_set_rect2(&p_gvscr->v_r, p_rect);
    if (OSD_DIRDRAW == b_flag)
    {
        p_gvscr->lpb_scr = NULL;
    }
    else
    {
        if (mem_size_r > OSD_VSRC_MEM_MAX_SIZE)
        {
            p_gvscr->lpb_scr = NULL;
        }
        else
        {
            p_gvscr->lpb_scr = osd_vscr_buffer[vscr_idx - 1];
        }
    }

    if (p_gvscr->lpb_scr != NULL)
    {
        if (b_flag & OSD_GET_BACK)
        {
            osdlib_region_read(p_gvscr, p_rect);
        }
        else if (b_flag & OSD_GET_CORNER)
        {
            osd_set_rect(&corner, p_rect->u_left, p_rect->u_top, CIRCLE_PIX * 2, p_rect->u_height);
            osdlib_region_read(p_gvscr, &corner);

            corner.u_left = p_rect->u_left + p_rect->u_width - CIRCLE_PIX * 2;
            osdlib_region_read(p_gvscr, &corner);
        }
        else
        {
            MEMSET(p_gvscr->lpb_scr, OSD_TRANSPARENT_COLOR_BYTE, mem_size_r);
        }
    }

    p_gvscr->update_pending = FALSE;

#endif
    return p_gvscr;
}



void osd_golobal_vscr_init(void)
{
    return; //Task should manage the buffer itself.
#if 0
    UINT8   i = 0;
    MEMSET(g_vscr, 0x0, sizeof(g_vscr));
    for (i = 0; i < OSD_DRAW_TASK_MAX; i++)
    {
        if (NULL != osd_vscr_buffer[i])
        {
            osd_vscr_buffer[i] = NULL;
        }
    }
#endif
}

void osd_task_buffer_init(ID task_id, UINT8 *p_config)
{
    memset(&g_vscr_bak, 0, sizeof(VSCR));
    g_vscr_bak.v_r.u_width = 1280;
    g_vscr_bak.v_r.u_height = 720;
    //g_vscr_bak.lpb_scr = (UINT8 *)MALLOC(OSD_VSRC_MEM_MAX_SIZE);
    //memset(g_vscr_bak.lpb_scr, 0, OSD_VSRC_MEM_MAX_SIZE);

    sema_vscr_bak = osal_mutex_create();
    
    if ((OSAL_INVALID_ID == task_id) || (task_id > OSD_DRAW_TASK_MAX))
    {
        return;/*Invalid id or id overflow max supported*/
    }

    if (osd_vscr_buffer[task_id - 1] != NULL)
    {
        return;/*If memory have been alloced*/
    }
    if (p_config != NULL)
    {
        /*Memory managed by user*/
        osd_vscr_buffer[task_id - 1] = (UINT8 *)(((UINT32)p_config & 0x0fffffff) | 0x80000000);
    }
    else
    {
        osd_vscr_buffer[task_id - 1] = (UINT8 *)MALLOC(OSD_VSRC_MEM_MAX_SIZE);
    }
    if (NULL == osd_vscr_buffer[task_id - 1])
    {
        ASSERT(0);/*Malloc osd VSCR buffer failure,draw osd will abnormal,so assert here*/
    }
#if 0
    else
    {
        /*Set as transparent color first*/
        //MEMSET(osd_vscr_buffer[task_id - 1],OSD_TRANSPARENT_COLOR,OSD_VSRC_MEM_MAX_SIZE);
    }
#endif
}

void osd_task_buffer_free(ID task_id, UINT8 *p_config)
{
    UINT32  i = 0;

    if (((OSAL_INVALID_ID == task_id) && (NULL == p_config))
            || ((task_id != OSAL_INVALID_ID) && (task_id > OSD_DRAW_TASK_MAX)))
    {
        return;/*Invalid id or id overflow max supported*/
    }

    if (p_config != NULL)
    {
        for (i = 0; i < OSD_DRAW_TASK_MAX; i++)
        {
            if (p_config == osd_vscr_buffer[i])
            {
                /*User managed memory should be freed by calling function,here only reset buffer pointers*/
                osd_vscr_buffer[i] = NULL;
            }
        }
    }
    else
    {
        /*Lib managed memory is freed by itself*/
        if (NULL != osd_vscr_buffer[task_id - 1])
        {
            FREE(osd_vscr_buffer[task_id - 1]);
            osd_vscr_buffer[task_id - 1] = NULL;
        }
    }
}

UINT8 *osd_get_task_vscr_buffer(ID task_id)
{
    if ((OSAL_INVALID_ID == task_id)|| (task_id > OSD_DRAW_TASK_MAX))
    {
        return NULL;/*Invalid id or id overflow max supported*/
    }

    return (osd_vscr_buffer[task_id - 1]);
}

LPVSCR osd_get_task_vscr(ID task_id)
{
    if ((OSAL_INVALID_ID == task_id)|| (task_id > OSD_DRAW_TASK_MAX))
    {
        return NULL;/*Invalid id or id overflow max supported*/
    }

    return &(g_vscr[task_id - 1]);
}




