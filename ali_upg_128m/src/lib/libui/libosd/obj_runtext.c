/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_runtext.c
*
*    Description: implement runner text object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"
#include "obj_runtext.h"

#ifdef TXT_RUNNER_SUPPORT
#define RUNER_OBJ_INDEX_0   0
#define RUNER_OBJ_INDEX_1   1
#define RUNER_OBJ_INDEX_2   2

static struct RUNTXTMAP osd_runer_txt_obj1;
static struct RUNTXTMAP osd_runer_orginal_txt_obj1;
static struct RUNTXTMAP osd_runer_txt_obj2;
static struct RUNTXTMAP osd_runer_orginal_txt_obj2;
static struct RUNTXTMAP osd_runer_txt_obj3;
static struct RUNTXTMAP osd_runer_orginal_txt_obj3;
static struct RUNTXTMAP osd_running_obj;

static UINT16  uoffset_xvalue      = 0;
static UINT16  obj_run_mode        = 0;
static UINT8   runner_obj_change_value = 0;
static UINT8   total_runner_obj_value  = 0;
static UINT16  obj_run_total_strlen    = 0;

static struct osdrect runner_size;
static struct osdrect runner_size1;
static struct osdrect runner_size2;
static struct osdrect runner_size3;

static UINT16 osd_draw_char_ext2(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch,
                                UINT8 font, LPVSCR p_vscr);

void set_osd_txt_runner_obj(struct RUNTXTMAP p_obj1, struct RUNTXTMAP p_obj2,struct RUNTXTMAP p_obj3)
{
    if (NULL != p_obj1.run_head)
    {
        total_runner_obj_value = 1;
        MEMCPY(&osd_runer_txt_obj1, &p_obj1, sizeof(struct RUNTXTMAP));
    }

    if (NULL != p_obj2.run_head)
    {
        total_runner_obj_value = 2;
        MEMCPY(&osd_runer_txt_obj2, &p_obj2, sizeof(struct RUNTXTMAP));
    }

    if (NULL != p_obj3.run_head)
    {
        total_runner_obj_value = 3;
        MEMCPY(&osd_runer_txt_obj3, &p_obj3, sizeof(struct RUNTXTMAP));
    }

}

void close_osd_txt_runner_obj(void)
{
    uoffset_xvalue = 0;
    obj_run_mode = 0;
    obj_run_total_strlen = 0;
    runner_obj_change_value = 0;
    total_runner_obj_value = 0;

    MEMSET(&osd_runer_txt_obj1, 0, sizeof(struct RUNTXTMAP));
    MEMSET(&osd_runer_txt_obj2, 0, sizeof(struct RUNTXTMAP));
    MEMSET(&osd_runer_txt_obj3, 0, sizeof(struct RUNTXTMAP));
}

static POBJECT_HEAD get_osd_txt_runner_obj(UINT8 index)
{
    if (RUNER_OBJ_INDEX_0 == index)
    {
        return osd_runer_txt_obj1.run_head;
    }
    else if (RUNER_OBJ_INDEX_1 == index)
    {
        return osd_runer_txt_obj2.run_head;
    }
    else if (RUNER_OBJ_INDEX_2 == index)
    {
        return osd_runer_txt_obj3.run_head;
    }
    else
    {
        return osd_runer_txt_obj1.run_head;
    }
}

static void get_osd_txt_runner_text_field_width(UINT8 index, struct osdrect *p_rect1)
{
    if(NULL == p_rect1)
    {
        return ;
    }
    switch(index)
    {
    case RUNER_OBJ_INDEX_1:
        MEMCPY(p_rect1, &(osd_runer_txt_obj2.run_head->frame), sizeof(struct osdrect));
        break;
    case RUNER_OBJ_INDEX_2:
        MEMCPY(p_rect1, &(osd_runer_txt_obj3.run_head->frame), sizeof(struct osdrect));
        break;
    default:
        MEMCPY(p_rect1, &(osd_runer_txt_obj1.run_head->frame), sizeof(struct osdrect));
        break;
    }
}

static void get_osd_txt_runner_map(UINT8 index, struct RUNTXTMAP *p_obj1)
{
    if(NULL == p_obj1)
    {
        return ;
    }
    switch(index)
    {
        case RUNER_OBJ_INDEX_1:
            {
                MEMCPY(p_obj1, &osd_runer_txt_obj2, sizeof(struct RUNTXTMAP));
            }
            break;
        case RUNER_OBJ_INDEX_2:
            {
                MEMCPY(p_obj1, &osd_runer_txt_obj3, sizeof(struct RUNTXTMAP));
            }
            break;
        default:
            {
                MEMCPY(p_obj1, &osd_runer_txt_obj1, sizeof(struct RUNTXTMAP));
            }
            break;
    }

}

static void set_osd_txt_runner_offset_value(UINT8 index, UINT16 value)
{
    switch(index)
    {
        case RUNER_OBJ_INDEX_1:
            {
                osd_runer_txt_obj2.offset_value = value;
            }
            break;
        case RUNER_OBJ_INDEX_2:
            {
                osd_runer_txt_obj3.offset_value = value;
            }
            break;
        default:
            {
                osd_runer_txt_obj1.offset_value = value;
            }
            break;
    }

}

static UINT8 osd_draw_txt_runner(struct RUNTXTMAP p_obj1, struct RUNTXTMAP p_obj2,
    struct RUNTXTMAP p_obj3)
{
    POBJECT_HEAD        oh          = NULL;
    OBJECT_HEAD         *p_foucs_obj  = NULL;
    PTEXT_FIELD         *p_txt       = NULL;
    UINT8               b_focus_id    = 0;
    UINT32              draw_type    = 0;
    UINT32              n_cmd_draw    = 0;
    UINT8               result      = FALSE;

    if (NULL == p_obj1.run_head)
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    if (TRUE == result)
    {
        result = FALSE;
        set_osd_txt_runner_obj(p_obj1, p_obj2, p_obj3);
        if (NULL != p_obj1.run_head)
        {
            MEMCPY(&runner_size1, &(osd_runer_txt_obj1.run_head->frame), sizeof(struct osdrect));
            if ((osd_runer_txt_obj1.total_strlen) > (runner_size1.u_width))
            {
                return TRUE;
            }
        }
        if (NULL != p_obj2.run_head)
        {
            MEMCPY(&runner_size2, &(osd_runer_txt_obj2.run_head->frame), sizeof(struct osdrect));
            if ((osd_runer_txt_obj2.total_strlen) > (runner_size2.u_width))
            {
                return TRUE;
            }
        }
        if (NULL != p_obj3.run_head)
        {
            MEMCPY(&runner_size3, &(osd_runer_txt_obj3.run_head->frame), sizeof(struct osdrect));
            if ((osd_runer_txt_obj3.total_strlen) > (runner_size3.u_width))
            {
                return TRUE;
            }
        }
    }
    return result;

}

static UINT8 osd_draw_con_txt_runner(struct RUNTXTMAP p_obj)
{
    POBJECT_HEAD        oh          = NULL;
    OBJECT_HEAD         *p_foucs_obj  = NULL;
    PTEXT_FIELD         runer_string = NULL;
    UINT8               b_focus_id    = 0;
    UINT32              draw_type    = 0;
    UINT32              n_cmd_draw    = 0;
    UINT8               result      = FALSE;
    UINT16              u_max_height  = 0;
    UINT16              u_max_width   = 0;
    struct RUNTXTMAP    temp_p_obj;
    struct RUNTXTMAP    temp_p_obj2;
    struct RUNTXTMAP    temp_p_obj3;

    //Set_OSD_TXT_Runner_Obj(pObj);
    MEMSET(&temp_p_obj, 0, sizeof(struct RUNTXTMAP));
    MEMSET(&temp_p_obj2, 0, sizeof(struct RUNTXTMAP));
    MEMSET(&temp_p_obj3, 0, sizeof(struct RUNTXTMAP));

    MEMCPY(&temp_p_obj, &p_obj, sizeof(struct RUNTXTMAP));
#if 1
    p_foucs_obj = osd_get_focus_object((POBJECT_HEAD)temp_p_obj.run_head);
    draw_type = C_DRAW_TYPE_NORMAL;
    n_cmd_draw = C_UPDATE_ALL;
    osd_set_draw_type(n_cmd_draw, draw_type);

    while (OT_TEXTFIELD != p_foucs_obj->b_type)
    {
        b_focus_id = osd_get_focus_id(p_foucs_obj);
        p_foucs_obj   = osd_get_object(p_foucs_obj, b_focus_id);
        if (NULL == p_foucs_obj)
        {
            break;
        }
    }
#endif
    if (NULL == p_foucs_obj)
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    if (TRUE == result)
    {
        temp_p_obj.run_head = (POBJECT_HEAD)p_foucs_obj;
        runer_string = (PTEXT_FIELD)temp_p_obj.run_head;
        temp_p_obj.total_strlen = osd_multi_font_lib_str_max_hw(runer_string->p_string,
                                                          runer_string->head.b_font, &u_max_height, &u_max_width, 0);
        set_osd_txt_runner_obj(temp_p_obj, temp_p_obj2, temp_p_obj3);
        MEMCPY(&runner_size1, &(osd_runer_txt_obj1.run_head->frame),sizeof(struct osdrect));
        if ((temp_p_obj.total_strlen) < (runner_size1.u_width))
        {
            result = FALSE;
        }
    }
    return result;

}


static UINT16 osd_draw_text_ext2(struct osdrect *p_rect, UINT8 *p_text, UINT32 color,
    UINT8 assign_style, UINT8 font_size, LPVSCR p_vscr)
{
    UINT16      u_max_height      = 0;
    UINT16      u_det_height      = 0;
    UINT16      u_max_width       = 0;
    UINT16      chw             = 0;
    UINT16      chh             = 0;
    UINT16      u_len            = 0;
    UINT16      u_str_len         = 0;
    UINT16      u_xpos           = 0;
    UINT16      u_ypos           = 0;
    UINT16      u_count          = 0;
    UINT16      u_str            = 0 ;	
	UINT32	cnt = 0;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devancell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	

    if (color & C_NOSHOW)
    {
        return 0;
    }
    if ((NULL == p_rect)|| (NULL == p_text))
    {
        return 0;
    }

    u_str_len = osd_multi_font_lib_str_max_hw(p_text, font_size, &u_max_height, &u_max_width, 0);
    if (!(u_str_len * u_max_height * u_max_width))
    {
        return 0;
    }
    u_xpos = p_rect->u_left;
    u_ypos = p_rect->u_top;
    //  Adjust Y Coordinate
    if (p_rect->u_height > u_max_height)
    {
        if (C_ALIGN_VCENTER == GET_VALIGN(assign_style))
        {
            u_ypos += (p_rect->u_height - u_max_height) >> 1;
        }
        else if (C_ALIGN_BOTTOM == GET_VALIGN(assign_style))
        {
            u_ypos += p_rect->u_height - u_max_height;
        }
    }
    //  Adjust X Coordinate
    if (u_str_len < p_rect->u_width)
    {
        if (C_ALIGN_CENTER == GET_HALIGN(assign_style))
        {
            u_xpos += (p_rect->u_width - u_str_len) >> 1;
        }
        else if (C_ALIGN_RIGHT == GET_HALIGN(assign_style))
        {
            u_xpos += p_rect->u_width - u_str_len - 4;
        }
    }

    while (0 != (u_str = com_mb16to_word(p_text)))  // == NULL?
    {
        if (is_thai_unicode(u_str))
        {
            struct thai_cell cell;

            cnt = thai_get_cell(p_text, &cell);
            if (0 == cnt)
            {
                break;
            }

            osd_get_thai_cell_width_height(&cell, font_size, &chw, &chh);
            if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
            {
                break;
            }

            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_thai_cell(u_xpos, u_ypos, color, C_NOSHOW, &cell, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += cnt;
            u_count++;
        }
#ifdef HINDI_LANGUAGE_SUPPORT
		else if (is_devanagari(u_str))
		{
			MEMSET(&devancell, 0x00, sizeof(devancell));
			if (!(cnt = get_devanagari_cell(p_text, &devancell)))
			{
				break;
			}
			
			osd_get_devanagari_width_height(&devancell, font_size, &chw, &chh);
			
			if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
			{
				break;
			}
			if (chh < u_max_height)
			{
				u_ypos += ((u_max_height - chh) >> 1);
			}

			u_xpos += osd_draw_devanagari(u_xpos, u_ypos, color, C_NOSHOW, &devancell, font_size, p_vscr);
			if (chh < u_max_height)
			{
				u_ypos -= ((u_max_height - chh) >> 1);
			}

			p_text += cnt;
			u_count++;
		}	
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT			
		else if (is_telugu(u_str))
		{
			MEMSET(&telcell, 0x00, sizeof(telcell));
			if (!(cnt = get_telugu_cell(p_text, &telcell)))
			{
				break;
			}
			
			osd_get_telugu_width_height(&telcell, font_size, &chw, &chh);
			
			if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
			{
				break;
			}
			if (chh < u_max_height)
			{
				u_ypos += ((u_max_height - chh) >> 1);
			}

			u_xpos += osd_draw_telugu(u_xpos, u_ypos, color, C_NOSHOW, &telcell, font_size, p_vscr);
			if (chh < u_max_height)
			{
				u_ypos -= ((u_max_height - chh) >> 1);
			}

			p_text += cnt;
			u_count++;
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT				
		else if (is_bengali(u_str))
		{
			MEMSET(&bencell, 0x00, sizeof(bencell));
			if (!(cnt = get_bengali_cell(p_text, &bencell)))
			{
				break;
			}
			
			osd_get_bengali_width_height(&bencell, font_size, &chw, &chh);
			
			if (u_xpos + chw > p_rect->u_left + p_rect->u_width)
			{
				break;
			}
			if (chh < u_max_height)
			{
				u_ypos += ((u_max_height - chh) >> 1);
			}

			u_xpos += osd_draw_bengali(u_xpos, u_ypos, color, C_NOSHOW, &bencell, font_size, p_vscr);
			if (chh < u_max_height)
			{
				u_ypos -= ((u_max_height - chh) >> 1);
			}

			p_text += cnt;
			u_count++;
		}
#endif		
        else
        {
            osd_get_char_width_height(u_str, font_size, &chw, &chh);

            if (chh < u_max_height)
            {
                u_ypos += (u_max_height - chh) >> 1;
            }
            u_xpos += osd_draw_char_ext2(u_xpos, u_ypos, color, C_NOSHOW, u_str, font_size, p_vscr);
            if (chh < u_max_height)
            {
                u_ypos -= (u_max_height - chh) >> 1;
            }
            p_text += 2;
            u_count++;
        }

    }
    return u_count;
}


static void osd_draw_text_field_cell_ext(PTEXT_FIELD p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect *objframe    = NULL;
    //ID_RSC        RscId;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32          fg_color     = 0;
    UINT8           *p_str        = NULL;
    OBJECTINFO      rsc_lib_info;
    struct osdrect  r;

    if(NULL == p_ctrl)
    {
        return ;
    }
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    //RscId         = OSDExt_GetMsgLibId(OSD_GetLangEnvironment());
    if (p_ctrl->p_string != NULL)
    {
        p_str = (UINT8 *)p_ctrl->p_string;
    }
    else
    {
        //g_osd_info.osd_get_rsc_data(RscId, pCtrl->wStringID, &RscLibInfo);
        p_str = osd_get_unicode_string(p_ctrl->w_string_id);
    }

    p_vscr = osd_draw_object_frame(objframe, b_style_idx);
    r = *objframe;
    if (!(p_ctrl->b_align & C_ALIGN_SCROLL)) //Use bX as the offset of string.
    {
        r.u_left += p_ctrl->b_x;
        r.u_width -= p_ctrl->b_x << 1;
    }
    r.u_top  += p_ctrl->b_y;
    r.u_height -= p_ctrl->b_y << 1;
    osd_draw_text_ext2(&r, p_str, lp_win_sty->w_fg_idx, p_ctrl->b_align, p_ctrl->head.b_font, p_vscr);


}

static void osd_get_text_field_width(PTEXT_FIELD p_ctrl1, struct osdrect *p_rect1, PTEXT_FIELD p_ctrl2,
    struct osdrect *p_rect2, PTEXT_FIELD p_ctrl3, struct osdrect *p_rect3)
{
    struct osdrect  *objframe   = NULL;
    struct osdrect  r;

    MEMSET(&r,0x00,sizeof(struct osdrect));
    if (NULL == p_ctrl1)
    {
        return;
    }

    objframe    = &p_ctrl1->head.frame;
    r = *objframe;
    MEMCPY(p_rect1, &r, sizeof(struct osdrect));

    if (p_ctrl2 != NULL)
    {
        objframe    = &p_ctrl2->head.frame;
        r = *objframe;
        MEMCPY(p_rect2, &r, sizeof(struct osdrect));
    }

    if (p_ctrl3 != NULL)
    {
        objframe    = &p_ctrl3->head.frame;
        r = *objframe;
        MEMCPY(p_rect3, &r, sizeof(struct osdrect));
    }
}



static UINT16 osd_draw_char_ext2(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch,
    UINT8 font, LPVSCR p_vscr)
{
    UINT16          height      = 0;
    UINT16          width       = 0;
    BOOL            b_get_vscr    = FALSE;
    ID_RSC          rsc_lib_id    = 0;
    UINT8           *p_bit_map_data = NULL;
    UINT32          stride      = 0;
    struct osdrect  r;
    VSCR            vscr;
    OBJECTINFO      obj_info;

    if (FALSE == osd_get_char_width_height(ch, font, &width, &height))
    {
        return 0;
    }
    rsc_lib_id = g_osd_rsc_info.osd_get_font_lib(ch) | font;

    //get vscr if necessary
    if ((NULL == p_vscr) || (NULL == p_vscr->lpb_scr))
    {
        b_get_vscr = osdget_local_vscr(&vscr, x, y, width, height);
        if (!b_get_vscr)
        {
            return 0;
        }
    }
    else
    {
        MEMCPY(&vscr, p_vscr, sizeof(VSCR));
    }
    //backgroud
    if (!(C_NOSHOW & bg_color) && b_get_vscr)
    {
        osd_draw_fill(x, y, width, height, bg_color, &vscr);
    }
    // Load bitmap
    p_bit_map_data = (UINT8 *)g_osd_rsc_info.osd_get_rsc_data(rsc_lib_id, ch, &obj_info);
    if (NULL != p_bit_map_data)
    {
        if (osd_color_mode_is_clut(vscr.b_color_mode))
        {
            get_osd_txt_runner_text_field_width(runner_obj_change_value, &runner_size);

            if ((x - uoffset_xvalue) < (runner_size.u_left))
            {
                if (0 == obj_run_mode)
                {
                    libc_printf("0r_x=>%d\n", x - uoffset_xvalue - width + runner_size.u_width);
                    if ((x - uoffset_xvalue - width + runner_size.u_width) >= (runner_size.u_left))
                    {
                        osd_set_rect(&r, x - uoffset_xvalue - width +runner_size.u_width, y, width, height);
                        osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
                    }
                    else
                    {
                        libc_printf("aaaa \n");
                    }
                }
                else if (1 == obj_run_mode)
                {
                    if ((uoffset_xvalue > obj_run_total_strlen)
                            && ((x - uoffset_xvalue + runner_size.u_width +
                                 obj_run_total_strlen)
                                < (runner_size.u_width + runner_size.u_left))
                            && ((x - uoffset_xvalue + runner_size.u_width +
                                 obj_run_total_strlen - 32) > (runner_size.u_left)))
                    {
                        osd_set_rect(&r, x - uoffset_xvalue + runner_size.u_width +
                                    obj_run_total_strlen - 32, y, width, height);
                        osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
                    }
                    else
                    {
                        libc_printf("bbb \n");
                    }
                }
            }
            else
            {
                libc_printf("1r_x=>%d\n", x - uoffset_xvalue);
                osd_set_rect(&r, x - uoffset_xvalue, y, width, height);
                osddraw_bmp(p_bit_map_data, &obj_info, &vscr, &r, rsc_lib_id, fg_color);
            }

            if (0 == obj_run_mode)
            {
                if (uoffset_xvalue >= runner_size.u_width)
                {
                    uoffset_xvalue = 0;    //runner_size.uWidth;
                }
            }
            else if (1 == obj_run_mode)
            {
                if (uoffset_xvalue >= (runner_size.u_width + obj_run_total_strlen - width))
                {
                    uoffset_xvalue = 0;    //runner_size.uWidth;
                }
            }
        }
        else
        {
            stride = width + 7;
            stride >>= 3;
            draw_char_matrix(x, y, width, height, p_bit_map_data, stride, C_MIXER, fg_color, &vscr);
        }
    }
    if (b_get_vscr)
    {
        osdlib_region_write(&vscr, &vscr.v_r);
    }
    return width;
}

void start_txt_runner(void)
{
    UINT8               b_style_idx   = 0;
    UINT8               b_focus_id    = 0;
    UINT32              draw_type    = 0;
    UINT32              n_cmd_draw    = 0;
    UINT32              vscr_idx    = OSAL_INVALID_ID;
    struct osdrect      *objframe   = NULL;
    UINT32              mem_size_r  = 0;

    POBJECT_HEAD        oh          = NULL;
    OBJECT_HEAD         *p_foucs_obj  = NULL;
    PTEXT_FIELD         p_ctrl       = NULL;
    PWINSTYLE           lp_win_sty    = NULL;
    UINT32              fg_color     = 0;
    UINT8               *p_str       = NULL;
    VSCR                *p_vscr      = NULL;

    vscr_idx = osal_task_get_current_id();
    if ((OSAL_INVALID_ID == vscr_idx) || (vscr_idx > OSD_DRAW_TASK_MAX))
    {
        ASSERT(0);
    }

    runner_obj_change_value += 1;
    if (runner_obj_change_value >= total_runner_obj_value)
    {
        runner_obj_change_value = 0;
    }
    get_osd_txt_runner_map(runner_obj_change_value, &osd_running_obj);
    uoffset_xvalue = (osd_running_obj.offset_value + osd_running_obj.offset_step);
    obj_run_mode = osd_running_obj.run_obj_mode;
    obj_run_total_strlen = osd_running_obj.total_strlen;

    p_foucs_obj = (OBJECT_HEAD *)get_osd_txt_runner_obj(runner_obj_change_value);
    n_cmd_draw = C_UPDATE_ALL;
#if 1
    draw_type = C_DRAW_TYPE_NORMAL;

    b_style_idx = p_foucs_obj->style.b_hlidx;

    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_bg_idx;

    p_vscr = osd_get_vscr(&p_foucs_obj->frame, OSD_REQBUF);
    osd_draw_frame(&p_foucs_obj->frame, fg_color, p_vscr);
#endif
    b_style_idx = p_foucs_obj->style.b_hlidx;

    osd_draw_text_field_cell_ext((PTEXT_FIELD)p_foucs_obj, b_style_idx, n_cmd_draw);
    p_vscr = osd_get_task_vscr(vscr_idx);

    if ((NULL != p_vscr) && (NULL != p_vscr->lpb_scr))
    {
        p_vscr->update_pending = 1;
    }

    osd_update_vscr(p_vscr);
    set_osd_txt_runner_offset_value(runner_obj_change_value, uoffset_xvalue);

}
#endif

