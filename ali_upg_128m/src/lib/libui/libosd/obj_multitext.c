/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_multitext.c
*
*    Description: multitext object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"

#define C_LINE_GAP  4
#define BUG_LIBC    PRINTF    //libc_printf  
#define RIGHT_LIBC  PRINTF  //libc_printf
//#define BLD_DEBUG

#define osd_multi_text_param_extend(_ptr, _l,_t,_lidx,_minfo,_linec,_x,_r_flag,_newline,\
    _left,_right,_bottom,param) \
    do{ \
        _ptr    = ((MULTITEXT_PARAM *)param)->p_draw_str; \
        _l      = ((MULTITEXT_PARAM *)param)->pl; \
        _t      = ((MULTITEXT_PARAM *)param)->pt; \
        _lidx   = ((MULTITEXT_PARAM *)param)->plidx; \
        _minfo  = ((MULTITEXT_PARAM *)param)->ptxtinfo; \
        _linec  = ((MULTITEXT_PARAM *)param)->p_line_count; \
        _x      = ((MULTITEXT_PARAM *)param)->x; \
        _r_flag = ((MULTITEXT_PARAM *)param)->reverse_flag; \
        _newline= ((MULTITEXT_PARAM *)param)->p_new_line; \
        _left   = ((MULTITEXT_PARAM *)param)->left; \
        _right  = ((MULTITEXT_PARAM *)param)->right; \
        _bottom = ((MULTITEXT_PARAM *)param)->bottom; \
    }while(0)

#define osd_multi_text_param_make(_ptr, _l,_t,_lidx,_minfo,_linec,_x,_r_flag,_newline,\
    _left,_right,_bottom,param) \
    do{ \
        ((MULTITEXT_PARAM *)param)->p_draw_str        = _ptr; \
        ((MULTITEXT_PARAM *)param)->pl              = _l; \
        ((MULTITEXT_PARAM *)param)->pt              = _t; \
        ((MULTITEXT_PARAM *)param)->plidx           = _lidx; \
        ((MULTITEXT_PARAM *)param)->ptxtinfo        = _minfo; \
        ((MULTITEXT_PARAM *)param)->p_line_count      = _linec; \
        ((MULTITEXT_PARAM *)param)->x               = _x; \
        ((MULTITEXT_PARAM *)param)->reverse_flag    = _r_flag; \
        ((MULTITEXT_PARAM *)param)->p_new_line        = _newline; \
        ((MULTITEXT_PARAM *)param)->left            = _left; \
        ((MULTITEXT_PARAM *)param)->right           = _right; \
        ((MULTITEXT_PARAM *)param)->bottom          = _bottom; \
    }while(0)


struct str_new_line
{
    UINT16 word_cnts;
    UINT16 line_cnts;            //Total Lines After Rebuild
    UINT16 real_last_line;
    UINT16 word_len_buf[200];
    UINT16 word_wbuf[200];
};

typedef struct
{
    UINT8       *p_draw_str;
    UINT16      *pl;
    UINT16      *pt;
    INT16       *plidx;
    mtxtinfo_t  *ptxtinfo;
    UINT16      *p_line_count;
    UINT32      *p_new_line;
    UINT16      height;
    UINT16      left;
    UINT16      right;
    UINT16      bottom;
    BOOL        reverse_flag;
    UINT16      x;
}MULTITEXT_PARAM;

//---------------------------- PRIVATE VARIABLES ----------------------------//
#ifdef BIDIRECTIONAL_OSD_STYLE
UINT16 last_line_width;
#endif
static UINT16 ori_newline   = 0;
static UINT16 line_width[MAX_LINE_NUM] = {0};
static struct str_new_line arabic_new_str;
static UINT8 rebuild_str[4096];
static char word_end_char[] =
{
    ' ', ',', '.', ':', ';', '?', '!', '/', '-', '(', ')',
    '{', '}', '[', ']', '|', '=', '+', '&', '@', '$', '_',
};
static UINT16 osd_rebuild_str_ext(PMULTI_TEXT p_ctrl, UINT8 *p_str, UINT8 *p_rebuild_str);
static UINT16 osd_rebuild_long_str(PMULTI_TEXT p_ctrl, UINT8 *p_str, UINT8 *p_rebuild_str,
    struct str_new_line *new_line);
static BOOL osd_check_rebuild_str(UINT8 *p_str);

static BOOL IS_WORD_END(UINT16  wc)
{
    UINT32      i   = 0;
    UINT32      n   = 0;

    n = sizeof(word_end_char);
    for (i = 0; i < n; i++)
    {
        if (wc == (UINT16)word_end_char[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------- PRIVATE FUNCTIONS ----------------------------//
/*********************************************/

UINT16 osd_get_word_len(UINT8 *p_text, UINT8 font, UINT16 *wordwidth, UINT16 *wordheight , UINT16 width)
{
    UINT16      wc      = 0;
    UINT16      wordlen = 0;
    UINT16      w       = 0;
    UINT16      h       = 0;
    UINT16      mh      = 0;
    UINT16      ww      = 0;
    UINT32      cnt     = 0;
    UINT16      font_w  = 0;
    UINT16      font_h  = 0;
    struct thai_cell    cell;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devancell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	
    BOOL ret = FALSE;

    if((NULL == p_text) || (NULL == wordwidth) || (NULL == wordheight))
    {
        return 0;
    }
    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    while (0 != (wc = com_mb16to_word(p_text)))
    {
        if (IS_NEWLINE(wc))
        {
            *wordwidth = ww;
            *wordheight = mh;
            return wordlen;
        }
        else if (((wc > 0x4e00) && (wc < 0x9f45)) ||
                 ((wc > 0x3000) && (wc < 0x303f)) ||
                 ((wc > 0xff00) && (wc < 0xffef)) ||
                 ((wc > 0x2e80) && (wc < 0x2eff)))//chinese
        {
            if (0 == font_w)
            {
                font_w = 36;
                font_h = 36;
                ret = osd_get_char_width_height(wc, font, &font_w, &font_h);
                if(!ret)
                {
                    return 0;
                }
                //libc_printf("font_w = %d,font_h = %d\n",font_w,font_h);
            }
            ww += font_w;
            mh = font_h;
            wordlen += 1;
            p_text += 2;
            break;
        }
        else if (IS_WORD_END(wc) && wordlen > 0)
        {
            break;
        }
        w = 10;
        h = 0;
        cnt = 2;
        if (is_thai_unicode(wc))
        {
            cnt = thai_get_cell(p_text, &cell);
            if (0 == cnt)
            {
                break;
            }

            ret = osd_get_thai_cell_width_height(&cell, font, &w, &h);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
#ifdef HINDI_LANGUAGE_SUPPORT
		else if (is_devanagari(wc))
		{
			MEMSET(&devancell, 0x00, sizeof(devancell));
			if (!(cnt = get_devanagari_cell(p_text, &devancell)))
			{
				break;
			}
			ret = osd_get_devanagari_width_height(&devancell, font, &w, &h);
			if (!ret)
			{
				ali_trace(&ret);
			}
		}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT		
		else if (is_telugu(wc))
		{
			MEMSET(&telcell, 0x00, sizeof(telcell));
			if (!(cnt = get_telugu_cell(p_text, &telcell)))
			{
				break;
			}
			ret = osd_get_telugu_width_height(&telcell, font, &w, &h);
			if (!ret)
			{
				ali_trace(&ret);
			}
		}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT		
		else if (is_bengali(wc))
		{
			MEMSET(&bencell, 0x00, sizeof(bencell));
			if (!(cnt = get_bengali_cell(p_text, &bencell)))
			{
				break;
			}
			ret = osd_get_bengali_width_height(&bencell, font, &w, &h);
			if (!ret)
			{
				ali_trace(&ret);
			}
		}
#endif		
        else
        {
            ret = osd_get_char_width_height(wc , font, &w, &h);
            if(!ret)
            {
                ali_trace(&ret);
            }
        }
        if (mh < h)
        {
            mh = h;
        }
        if ((ww + w) <= width)
        {
            ww += w;
        }
        else
        {
            break;
        }
        wordlen += cnt / 2;
        p_text += cnt;

        if (IS_WORD_END(wc))
        {
            break;
        }
    }
    *wordwidth = ww;
    *wordheight = mh;

    return wordlen;
}

UINT16 osd_get_text_total_line(UINT8 *p_text, UINT8 font, UINT16 width,
    INT16 lineidx, UINT8 **line_str, UINT8 *linesheght)
{
    UINT16      l       = 0;
    UINT16      wc      = 0;
    UINT16      wordlen = 0;
    UINT16      chl     = 0;
    UINT16      wordw   = 0;
    UINT16      wordh   = 0;
    UINT16      line    = 0;

    if (NULL == p_text)
    {
        return 0;
    }

    l = 0;
    chl = 0;
    wc = com_mb16to_word(p_text);
    while ((0 != wc) && (line < MAX_LINE_NUM))
    {
        if ((0 == l) && (line == lineidx)  && (NULL != line_str))
        {
            *line_str = p_text;
        }

        if (IS_NEWLINE(wc))
        {
            if (0 == chl)
            {
                chl = OSD_FONT_HEIGHT;
            }
            if (NULL != linesheght)
            {
                linesheght[line] = chl;
            }
            line_width[line] = l;
            line++;
            l = 0;
            chl = 0;
            p_text += 2;
        }
        else
        {
            wordlen = osd_get_word_len(p_text, font, &wordw, &wordh, width);
            if (wordlen > 0)
            {
                if (0 == chl)
                {
                    chl = wordh;
                }

                /*If first char of the line is a space, ignore it.*/
                //                if(l == 0 && wordlen==1 && wc == ' ')
                //               {
                //                    pText += 2;
                //                }
                //                else if(l + wordw <= width || l == 0)
                if ((l + wordw <= width) || (0 == l))
                {
                    l += wordw;
                    p_text += wordlen * 2;
                }
                else    /* Next line*/
                {
                    if (chl < wordh)
                    {
                        chl = wordh;
                    }
                    if (0 == chl)
                    {
                        chl = OSD_FONT_HEIGHT;
                    }
                    if (NULL != linesheght)
                    {
                        linesheght[line] = chl;
                    }
                    line_width[line] = l;
                    line++;
                    l = 0;
                    chl = 0;
                }
            }
        }

        wc = com_mb16to_word(p_text);
        if ((0 != l) && (0 == wc))
        {
            if (0 == chl)
            {
                chl = OSD_FONT_HEIGHT;
            }
            if (NULL != linesheght)
            {
                linesheght[line] = chl;
            }
            line_width[line] = l;
            line++;
        }
    }

    return line;
}

static void osd_get_top_line(PMULTI_TEXT p_ctrl, UINT8 *p_str, INT16 lineidx,
    UINT8 **line_str, UINT8 *linesheght)
{
    UINT16      l       = 0;
    UINT16      wc      = 0;
    UINT16      wordlen = 0;
    UINT16      chl     = 0;
    UINT16      wordw   = 0;
    UINT16      wordh   = 0;
    UINT16      line    = 0;

    if(NULL == p_str)
    {
        return ;
    }
    wc = com_mb16to_word(p_str);
    while ((0 != wc) && (line < MAX_LINE_NUM))
    {
        if ((0 == l) && (line == lineidx)  && (NULL != line_str))
        {
            *line_str = p_str;
        }

        if (IS_NEWLINE(wc))
        {
            if (0 == chl)
            {
                chl = OSD_FONT_HEIGHT;
            }
            if (NULL != linesheght)
            {
                linesheght[line] = chl;
            }
            line++;
            RIGHT_LIBC("line=%d \n", line);
            l = 0;
            chl = 0;
            p_str += 2;
        }
        else
        {
            wordlen = osd_get_word_len(p_str, p_ctrl->head.b_font, &wordw, &wordh,
                                     p_ctrl->rc_text.u_width);
            if (wordlen > 0)
            {
                if (0 == chl)
                {
                    chl = wordh;
                }
                if ((l + wordw <= p_ctrl->rc_text.u_width) || (0 == l))
                {
                    l += wordw;
                    p_str += wordlen * 2;
                }
                else    /* Next line*/
                {
                    if (chl < wordh)
                    {
                        chl = wordh;
                    }
                    if (0 == chl)
                    {
                        chl = OSD_FONT_HEIGHT;
                    }
                    if (NULL != linesheght)
                    {
                        linesheght[line] = chl;
                    }
                    line++;
                    l = 0;
                }
            }
        }

        wc = com_mb16to_word(p_str);

        if ((0 != l) && (0 == wc))
        {
            if (0 == chl)
            {
                chl = OSD_FONT_HEIGHT;
            }
            if (NULL != linesheght)
            {
                linesheght[line] = chl;
            }
            line++;
        }
    }
}


void osd_get_mtext_infor(PMULTI_TEXT p_ctrl, mtxtinfo_t *p_info)
{
    PTEXT_CONTENT   p_text       = NULL;
    UINT16          width       = 0;
    UINT16          height      = 0;
    UINT16          total_lines = 0;
    UINT16          page_lines  = 0;
    UINT16          lines       = 0;
    INT16           lidx        = 0;
    UINT32          i           = 0;
    UINT8           *p_str       = NULL;

    if((NULL == p_ctrl) || (NULL == p_info))
    {
        return ;
    }
    width  = p_ctrl->rc_text.u_width;
    page_lines = 0;

    for (i = 0; i < p_ctrl->b_count; i++)
    {
        p_text = &p_ctrl->p_text_table[i];

        if (NULL == p_text)
        {
            continue;
        }

        if ((UINT8)STRING_ID == p_text->b_text_type)
        {
            p_str = osd_get_unicode_string(p_text->text.w_string_id);
        }
        else
        {
            p_str = (UINT8 *)p_text->text.p_string;
        }
        if (NULL == p_str)
        {
            continue;
        }

        lidx = (INT16)p_info->topline_idx - (INT16)total_lines;
        if (FALSE != osd_check_rebuild_str(p_str))
        {
            MEMSET(rebuild_str, 0, 4096);
            lines = osd_rebuild_str_ext(p_ctrl, p_str, rebuild_str);
            osd_get_top_line(p_ctrl, rebuild_str, lidx, &p_info->topline_pstr,
                           &p_info->line_height[total_lines]);
            RIGHT_LIBC("topline_pstr=0x%08x \n", p_info->topline_pstr);
        }
        else
        {
            lines = osd_get_text_total_line(p_str, p_ctrl->head.b_font, width, lidx,
                                         &p_info->topline_pstr, &p_info->line_height[total_lines]);
        }

        if ((p_info->topline_idx >= total_lines) && (p_info->topline_idx < lines + total_lines))
        {
            p_info->topline_tblidx = i;
        }
        total_lines += lines;
    }

    /* Total lines */
    p_info->total_lines = total_lines;

    /* Get the total logical height.*/
    p_info->total_height = 0;
    for (i = 0; i < total_lines; i++)
    {
        p_info->total_height += p_info->line_height[i];
        if (i > 0)
        {
            p_info->total_height += C_LINE_GAP;
        }
    }

    /* One page's height */
    p_info->page_height = p_ctrl->rc_text.u_height;

    if (p_info->topline_idx >= total_lines)
    {
        p_info->topline_idx = 0;
    }

#if 0
    /* Check the topline_idx */
CHECK_TOP_LINE:
    height = 0;
    for (i = p_info->topline_idx; i < total_lines; i++)
    {
        height += p_info->line_height[i];
        if (i != p_info->topline_idx)
        {
            height += C_LINE_GAP;
        }
    }
    if ((p_info->topline_idx > 0) && (height < p_info->page_height))
    {
        if (height + p_info->line_height[total_lines - 1] +
                C_LINE_GAP <= p_info->page_height)
        {
            p_info->topline_idx -= 1;
            goto CHECK_TOP_LINE;
        }
    }
#endif

    /* Get Current Page's lines */
    height = 0;
    for (i = 0; i < total_lines; i++)
    {
        height += p_info->line_height[i];
        if (i != p_info->topline_idx)
        {
            height += C_LINE_GAP;
        }
        if (height > p_info->page_height)
        {
            break;
        }
    }

    page_lines = i;
    p_info->page_lines  = page_lines;
}

#if 0
BOOL osd_rebuild_str(PMULTI_TEXT p_ctrl, mtxtinfo_t *p_info, UINT8 *p_str, UINT8 *p_rebuild_str)
{

    UINT16 l;
    UINT16 wc, wordlen;
    UINT16 wordw, wordh;
    UINT16 line = 0;
    UINT16 str_len_buf[200], str_word_wbuf[200];
    UINT16 str_line_flag[MAX_LINE_NUM];
    UINT16 cnt_buf = 0, cnt_line = 0;
    UINT16 str_len = 0;
    UINT16 str_len_last = 0;
    UINT8 *p_str_tmp;
    UINT16 j = 0, k = 0, m = 0, n = 0;
    if (1 == p_info->total_lines)
    {
        return FALSE;
    }

    l = 0;
    p_str_tmp = p_str;
    wc = com_mb16to_word(p_str_tmp);
    while (0 != wc)
    {
        if (IS_NEWLINE(wc))
        {
            p_str_tmp += 2;
        }
        else
        {
            wordlen = osd_get_word_len(p_str_tmp, p_ctrl->head.b_font, &wordw,
                                     &wordh, p_ctrl->rc_text.u_width);
            if (wordlen > 0)
            {
                j = cnt_buf;
                str_len_buf[j] = wordlen;
                str_word_wbuf[j] = wordw;
                p_str_tmp += wordlen * 2;
                cnt_buf++;
            }
        }

        wc = com_mb16to_word(p_str_tmp);
    }
    j = cnt_buf;
    k = 0;
    while (j--)
    {

        if (l + str_word_wbuf[j] > p_ctrl->rc_text.u_width)
        {
            str_line_flag[cnt_line++] = k * 2;
            l = str_word_wbuf[j];
            k = str_len_buf[j];
        }
        else
        {
            l += str_word_wbuf[j];
            k += str_len_buf[j];
        }
    }

    j = cnt_buf;
    while (j--)
    {
        str_len += str_len_buf[j] * 2;
    }
    j = cnt_line;
    k = 0;
    while (j--)
    {
        k += str_line_flag[j];
    }
    str_len_last = str_len - k;
    j = 0;//cntLine;
    if (0 == cnt_line)
    {
        return FALSE;
    }
    while (++j)
    {
        if (j > cnt_line)
        {
            break;
        }
        k = str_line_flag[j - 1];
        while (k)
        {
            p_rebuild_str[str_line_flag[j - 1] - k + m + n * 2] = p_str[str_len - k - m];
            k--;
        }
        m += str_line_flag[j - 1];
        p_rebuild_str[m + n * 2] = '\0';
        p_rebuild_str[m + n * 2 + 1] = '\n';
        n++;
    }


    if (str_len_last)
    {
        j = str_len_last;
        while (j--)
        {
            p_rebuild_str[str_len - j - 1 + n * 2] = p_str[str_len_last - j - 1];
        }
        p_rebuild_str[str_len + n * 2] = '\0';
        p_rebuild_str[str_len + n * 2 + 1] = '\0';

    }
    return TRUE;
}
#endif

static UINT16 osd_rebuild_str_ext(PMULTI_TEXT p_ctrl, UINT8 *p_str, UINT8 *p_rebuild_str)
{
    UINT16      wc          = 0;
    UINT16      wordlen     = 0;
    UINT16      wordw       = 0;
    UINT16      wordh       = 0;
    UINT16      j           = 0;
    UINT8       *p_str_tmp    = NULL;
    UINT16      ori_linelength = 0;
    UINT16      b_i          = 0;
    UINT16      str_offset  = 0;

    if((NULL == p_str) || (NULL == p_rebuild_str))
    {
        return 0;
    }
    MEMSET(&arabic_new_str, 0, sizeof(arabic_new_str));
    ori_newline = 0;
    p_str_tmp = p_str;

    wc = com_mb16to_word(p_str_tmp);
    while (0 != wc)
    {
        if (IS_NEWLINE(wc))
        {
            BUG_LIBC("ori new line wc=0x%04x\n", wc);
            p_str_tmp += 2;
            //Save
            j = arabic_new_str.word_cnts;
            arabic_new_str.real_last_line = 0;
            str_offset += osd_rebuild_long_str(p_ctrl, p_str + ori_linelength,
                                             p_rebuild_str + str_offset, &arabic_new_str);
            BUG_LIBC("Rebuild  str_offset=%d \n", str_offset);

            for (b_i = 0; b_i < j; b_i++)
            {
                ori_linelength += arabic_new_str.word_len_buf[b_i] * 2;
            }
            ori_linelength += 2;    //Include '\n'

            ori_newline += arabic_new_str.line_cnts;

            MEMSET(&arabic_new_str, 0, sizeof(arabic_new_str));
            BUG_LIBC("ori_linelength=%d str_offset=%d\n", ori_linelength,str_offset);
        }
        else
        {
            if(NULL == p_ctrl)
            {
                return 0;
            }
            wordlen = osd_get_word_len(p_str_tmp, p_ctrl->head.b_font, &wordw, &wordh, p_ctrl->rc_text.u_width);
            if (wordlen > 0)
            {
                j = arabic_new_str.word_cnts;
                arabic_new_str.word_len_buf[j] = wordlen;
                arabic_new_str.word_wbuf[j] = wordw;
                p_str_tmp += wordlen * 2;
                arabic_new_str.word_cnts++;
            }
            else if( 0 == wordlen)
            {    
                //exit because of err in osd_get_word_len, avoid dead loop;
                break;
            }
        }

        wc = com_mb16to_word(p_str_tmp);
    }

    BUG_LIBC("Rebuild Last Line\n");
    arabic_new_str.real_last_line = 1;
    str_offset = osd_rebuild_long_str(p_ctrl, p_str + ori_linelength, p_rebuild_str + str_offset, &arabic_new_str);
    if(!str_offset)
    {
        return 0;
    }
    ori_newline += arabic_new_str.line_cnts;
    BUG_LIBC("Rebuild Last str_offset=%d ori_newline=%d \n", str_offset, ori_newline);

#ifdef BLD_DEBUG
    //UINT16 bJ = 0;
    //libc_printf("After Build \n");
    //while (1)
    //{
    //    wc = ComMB16ToWord(pRebuildStr + bJ);

    //    libc_printf("wc=0x%04x ", wc);
    //    bJ += 2;
    //    if (0 == (bJ % 16))
    //    {
    //        libc_printf("\n ");
    //    }
    //    if (0x000a == wc)
    //    {
    //        libc_printf("\n change flag bJ=%d \n ", bJ);
    //    }
    //    if (0 == wc)
    //    {
    //        libc_printf("\n end flag bJ=%d \n ", bJ);
    //        break;
    //    }
    //}
#endif

    return ori_newline;
}


static UINT16 osd_rebuild_long_str(PMULTI_TEXT p_ctrl, UINT8 *p_str,
    UINT8 *p_rebuild_str, struct str_new_line *new_line)
{
    UINT16      l           = 0;
    UINT16      j           = 0;
    UINT16      k           = 0;
    UINT16      m           = 0;
    UINT16      n           = 0;
    UINT16      cnt_line     = 0;
    UINT16      str_len      = 0;
    UINT16      str_len_last  = 0;
    UINT16      length_re    = 0;
    UINT16      str_line_flag[MAX_LINE_NUM];

#ifdef DEBUG_LASTL
    UINT16      lastlinewidth_tmp = 0;
    UINT16      ori_lastlinewidth = 0;

#endif

    if((NULL == p_ctrl) || (NULL == p_str) || (NULL == p_rebuild_str) || (NULL == new_line))
    {
        return 0;
    }
    MEMSET(str_line_flag, 0, sizeof(UINT16)*ARRAY_SIZE(str_line_flag));
    j = new_line->word_cnts;
    BUG_LIBC(">>> j=%d \n", j);
    while (j > 0)
    {
        j -= 1;
        if (l + new_line->word_wbuf[j] > p_ctrl->rc_text.u_width)
        {
            line_width[ori_newline + cnt_line] = l;
            RIGHT_LIBC("Width=%d lineth=%d \n", l, ori_newline + cnt_line);

            str_line_flag[cnt_line] = k * 2;
            cnt_line++;
            BUG_LIBC(">>> StrLineFlag[%d]=%d j=%d \n", cnt_line, k * 2, j);
            l = new_line->word_wbuf[j];
            k = new_line->word_len_buf[j];
            BUG_LIBC(">>> l=%d k=%d \n", l, k);
        }
        else
        {
            l += new_line->word_wbuf[j];
            k += new_line->word_len_buf[j];
        }
    }
    line_width[ori_newline + cnt_line] = l;
    RIGHT_LIBC("Width=%d last lineth=%d \n", l, ori_newline + cnt_line);

    //Need Change Line?
    if (0 == cnt_line)
    {
        str_len_last = k * 2;
        //For Debug Double CHeck last line width,If OK Pls Mark.
#ifdef DEBUG_LASTL
        lastlinewidth_tmp = 0;
        for (j = 0; j < str_len_last; j++)
        {
            lastlinewidth_tmp += new_line->word_wbuf[j + k];
        }
        BUG_LIBC("0__Lastlinewidth_tmp=%d ori_Lastlinewidth=%d\n",
                 lastlinewidth_tmp, ori_lastlinewidth);
#endif

        for (j = 0; j < str_len_last; j++)
        {
            p_rebuild_str[j] = p_str[j];
        }

        length_re += str_len_last;

        //Add "\n" Or '\0'
        p_rebuild_str[length_re] = '\0';
        if (new_line->real_last_line)
        {
            p_rebuild_str[length_re + 1] = '\0';
        }
        else
        {
            p_rebuild_str[length_re + 1] = '\n';
        }

        length_re += 2;
        new_line->line_cnts = 1;
        BUG_LIBC("0__LengthRe=%d LineCnts=%d \n", length_re, new_line->line_cnts);
        return length_re;
    }

    //Get Total Length
    j = new_line->word_cnts;
    while (j > 0)
    {
        j -= 1;
        str_len += new_line->word_len_buf[j] * 2;
    }
    length_re += str_len;

    //The Last Line's word num.
    new_line->line_cnts = cnt_line + 1;
    j = cnt_line;
    k = 0;
    BUG_LIBC(">>> cntLine=%d \n", cnt_line);
    while (j > 0)
    {
        j -= 1;
        k += str_line_flag[j];
    }
    BUG_LIBC(">>> StrLen=%d k=%d \n", str_len, k);
    str_len_last = str_len - k;
    //For Debug Double CHeck last line width,If OK Pls Mark.
#ifdef DEBUG_LASTL
    lastlinewidth_tmp = 0;
    BUG_LIBC("StrLenLast=%d k=%d\n", str_len_last, k);
    for (j = 0; j < str_len_last; j++)
    {
        lastlinewidth_tmp += new_line->word_wbuf[j + k];
    }
    BUG_LIBC("M__Lastlinewidth_tmp=%d ori_Lastlinewidth=%d\n",
             lastlinewidth_tmp, ori_lastlinewidth);
#endif

    //String Rebuild except the last line.
    j = 0;
    while (++j)
    {
        if (j > cnt_line)
        {
            break;
        }
        k = str_line_flag[j - 1];
        BUG_LIBC("M__start copy to offset=%d \n", str_line_flag[j - 1] - k + m + n * 2);
        while (k)
        {
            p_rebuild_str[str_line_flag[j - 1] - k + m + n * 2] = p_str[str_len - k - m];
            k--;
        }
        m += str_line_flag[j - 1];
        BUG_LIBC("M__Add Line end=%d offset=%d\n", j, m + 2 * n);
        p_rebuild_str[m + n * 2] = '\0';
        p_rebuild_str[m + n * 2 + 1] = '\n';
        length_re += 2;
        n++;
    }

    //If With Last line,Rebuild The Last Line.
    if (str_len_last)
    {
        j = str_len_last;
        BUG_LIBC("M__Add Line end offset=%d StrLenLast=%d \n",
                 str_len - j - 1 + n * 2, j);
        while (j--)
        {
            p_rebuild_str[str_len - j - 1 + n * 2] = p_str[str_len_last - j - 1];
        }
        p_rebuild_str[str_len + n * 2] = '\0';

        if (new_line->real_last_line)
        {
            p_rebuild_str[str_len + n * 2 + 1] = '\0';
        }
        else
        {
            BUG_LIBC("M__Add last Line end offset=%d\n", str_len + n * 2);
            p_rebuild_str[str_len + n * 2 + 1] = '\n';
        }
    }

    length_re += 2;
    BUG_LIBC("M__LengthRe=%d LineCnts=%d \n", length_re, new_line->line_cnts);
    return length_re;
}


static BOOL osd_check_rebuild_str(UINT8 *p_str)
{
    UINT16      wc      = 0;
    UINT16      cnt1    = 0;
    UINT16      cnt2    = 0;

#ifdef BLD_DEBUG
    UINT16      b_i      = 0;

#endif
    if(NULL == p_str)
    {
        return FALSE;
    }
    MEMSET(line_width, 0, MAX_LINE_NUM);

    while (1)
    {
        wc = com_mb16to_word(p_str);

#ifdef BLD_DEBUG
        libc_printf("wc=0x%04x ", wc);
        b_i++;
        if (0 == (b_i % 8))
        {
            libc_printf("\n ");
        }
#endif

        if (0 == wc)
        {
            if (cnt1 || cnt2)
            {
                if (cnt1 >= cnt2)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            break;
        }
        else
        {
            //Arabic/farsi unicode
            if (((wc >= 0x600) && (wc <= 0x6ff))
                    || ((wc >= 0x750) && (wc <= 0x77f))
                    || ((wc >= 0x8a0) && (wc <= 0x8ff))
                    || ((wc >= 0xfb50) && (wc <= 0xfdff))
                    || ((wc >= 0xfe70) && (wc <= 0xfeff)))
            {
                cnt1++;
            }
            else
            {
                if (0x0020 != wc)
                {
                    cnt2++;
                }
            }

            p_str += 2;
        }
    }
    return FALSE;
}
//---------------------------- PUBLIC FUNCTIONS -----------------------------//

// public
// if not support malloc, it can only set string_id or unicode_string
void osd_set_multi_text_content(PMULTI_TEXT p_ctrl, PTEXT_CONTENT p_text_table)
{
    if(NULL == p_ctrl)
    {
        return ;
    }
    p_ctrl->p_text_table = p_text_table;
}
// private

static UINT8 *osd_draw_multi_text_inner(MULTITEXT_PARAM *p_param,PMULTI_TEXT p_ctrl,UINT16 width,
    UINT8 b_style_idx,struct osdrect *r,LPVSCR p_vscr)
{
    UINT16          wc          = 0;
    UINT32          bg_color     = 0;
    UINT8           font        = 0;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT16          *l          = NULL;
    UINT16          *t          = NULL;
    INT16           *lidx       = NULL;
    mtxtinfo_t      *mtxtinfo   = NULL;
    UINT16          * __MAYBE_UNUSED__ line_count  = NULL;
    UINT32          *b_new_line   = NULL;
    BOOL            reverse_flag= FALSE;
    UINT16          x           = 0;
    UINT8           *p_draw_str   = NULL;
    UINT16          wordlen     = 0;
    UINT16          wordw       = 0;
    UINT16          wordh       = 0;
    UINT32          k           = 0;
    UINT16          left        = 0;
    UINT16          right       = 0;
    UINT16          bottom      = 0;
    UINT32          cnt         = 0;
    UINT16          charh       = 0;
    UINT16          charw       = 0;

#ifdef BIDIRECTIONAL_OSD_STYLE
    BOOL            mirror_flag = g_osd_rsc_info.osd_get_mirror_flag();
#endif
    struct thai_cell    cell;
#ifdef HINDI_LANGUAGE_SUPPORT
	struct devanagari_cell devancell;
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
	struct telugu_cell telcell;
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
	struct bengali_cell bencell;
#endif	

    if((NULL == p_param) || (NULL == r) || (NULL == p_ctrl) || (NULL == p_vscr))
    {
        return NULL;
    }
    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    osd_multi_text_param_extend(p_draw_str,l,t,lidx,mtxtinfo,line_count,x,reverse_flag ,\
                    b_new_line,left,right,bottom,p_param);
    font        = p_ctrl->head.b_font;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    bg_color     = lp_win_sty->w_bg_idx;
    if (NULL != p_vscr->lpb_scr)
    {
        bg_color = C_NOSHOW;
    }
    while (1)
    {
        wc = com_mb16to_word(p_draw_str);
        if (0 == wc)
        {
            /* If the last character is not a new line
            character, then set next line from the start.*/
            if (!(*b_new_line))
            {
                *l = x;
                *t += mtxtinfo->line_height[*lidx] + C_LINE_GAP;
#ifdef BIDIRECTIONAL_OSD_STYLE
                (*line_count) ++;
                if (reverse_flag || mirror_flag)
                {
                    *l -=  r->u_width - line_width[*lidx];

                    //if (line_count == mtxtinfo.total_lines)
                    //{
                    //    *l -= r.u_width - last_line_width;
                    //}
                }
#endif
            }
            break;
        }
        else if (IS_NEWLINE(wc)) /* New line characters */
        {
            *l = x;
            *t += mtxtinfo->line_height[*lidx] + C_LINE_GAP;
            *lidx += 1;
            p_draw_str += 2;
            wordlen = 0;
            *b_new_line = 1;
#ifdef BIDIRECTIONAL_OSD_STYLE
            *line_count ++;
            if (reverse_flag || mirror_flag)
            {
#if 1
                *l -=  r->u_width - line_width[*lidx];
#else
                if (line_count == mtxtinfo.total_lines)
                {
                    *l -= r->u_width - last_line_width;
                }
#endif
            }
#endif
        }
        else
        {
            /* Get a word */
            wordlen = osd_get_word_len(p_draw_str, font, &wordw, &wordh, width);
            if ((wordlen > 0) && (*l != x))
            {
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (((!(reverse_flag || mirror_flag)) && (*l + wordw > right))\
                        || (((reverse_flag || mirror_flag)) && (*l < left + wordw)))
#else
                if ((!reverse_flag && (*l + wordw > right)) || (reverse_flag && (*l < left + wordw)))
#endif
                {
                    *l = x;
                    *t += mtxtinfo->line_height[*lidx] + C_LINE_GAP;
                    *lidx += 1;
                    *b_new_line = 1;
#ifdef BIDIRECTIONAL_OSD_STYLE
                    *line_count ++;
#endif
                }
            }
            else
            {
                *b_new_line = 0;
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (reverse_flag || mirror_flag)
                {
                    *l -=  r->u_width - line_width[*lidx];
                }
#endif
            }
        }

        if (*t > bottom)
        {
            break;
        }
        /* Draw the word's every characters.*/
        for (k = 0; k < wordlen; k++)
        {
            wc = com_mb16to_word(p_draw_str);
            if (is_thai_unicode(wc))
            {
                cnt = thai_get_cell(p_draw_str, &cell);
                if (0 == cnt)
                {
                    break;
                }
                if ((*t + mtxtinfo->line_height[*lidx]) <= bottom)
                {
                    charw = osd_draw_thai_cell(*l, *t , lp_win_sty->w_fg_idx, bg_color, &cell, font, p_vscr);
                }
                *l += charw;

                p_draw_str += cnt;
                k += cnt / 2 - 1;
            }
#ifdef HINDI_LANGUAGE_SUPPORT
			else if (is_devanagari(wc))
			{
				MEMSET(&devancell, 0x00, sizeof(devancell));
				if (!(cnt = get_devanagari_cell(p_draw_str, &devancell)))
				{
					break;
				}

				if ((*t + mtxtinfo->line_height[*lidx]) <= bottom)
				{
					charw = osd_draw_devanagari(*l, *t, lp_win_sty->w_fg_idx, bg_color, &devancell, font, p_vscr);
				}
				*l += charw;

				p_draw_str += cnt;
				k += (cnt / 2 - 1);
			}
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT			
			else if (is_telugu(wc))
			{
				MEMSET(&telcell, 0x00, sizeof(telcell));
				if (!(cnt = get_telugu_cell(p_draw_str, &telcell)))
				{
					break;
				}

				if ((*t + mtxtinfo->line_height[*lidx]) <= bottom)
				{
					charw = osd_draw_telugu(*l, *t, lp_win_sty->w_fg_idx, bg_color, &telcell, font, p_vscr);
				}
				*l += charw;

				p_draw_str += cnt;
				k += (cnt / 2 - 1);
			}
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT			
			else if (is_bengali(wc))
			{
				MEMSET(&bencell, 0x00, sizeof(bencell));
				if (!(cnt = get_bengali_cell(p_draw_str, &bencell)))
				{
					break;
				}

				if ((*t + mtxtinfo->line_height[*lidx]) <= bottom)
				{
					charw = osd_draw_bengali(*l, *t, lp_win_sty->w_fg_idx, bg_color, &bencell, font, p_vscr);
				}
				*l += charw;

				p_draw_str += cnt;
				k += (cnt / 2 - 1);
			}
#endif			
            else
            {
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (reverse_flag || mirror_flag)
#else
                if (reverse_flag)
#endif
                {
                    osd_get_char_width_height(wc, font, &charw, &charh);
                    *l -= charw;
                }
                if ((*t + mtxtinfo->line_height[*lidx]) <= bottom)
                {
                    charw = osd_draw_char(*l, *t , lp_win_sty->w_fg_idx, bg_color, wc, font, p_vscr);
                }
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (!(reverse_flag || mirror_flag))
#else
                if (!reverse_flag)
#endif
                {
                    *l += charw;
                }
                p_draw_str += 2;
            }
        }
    }
    return p_draw_str;
}

static void osd_get_draw_origination_point(mtxtinfo_t *mtxtinfo,UINT8 font,UINT16 height,
    UINT8 h_aligin,UINT8 v_align,UINT16 width,UINT16 *ox,UINT16 *oy)
{
    UINT16          max_w        = 0;
    UINT16          max_h        = 0;

    if((NULL == mtxtinfo) || (NULL == ox) || (NULL == oy))
    {
        return;
    }

    if (1 == mtxtinfo->total_lines)
    {
        max_w = osd_multi_font_lib_str_max_hw(mtxtinfo->topline_pstr, font, &max_w, &max_h, 0);
        if (max_w < width)
        {
            if (C_ALIGN_RIGHT == h_aligin)
            {
                *ox = width - max_w;
            }
            else if (C_ALIGN_CENTER == h_aligin)
            {
                *ox = (width - max_w) >> 1;
            }
        }
    }
    if (C_ALIGN_TOP == v_align)
    {
        oy = 0;
    }
    else if (C_ALIGN_BOTTOM == v_align)
    {
        if (mtxtinfo->total_height < height)
        {
            *oy = height - mtxtinfo->total_height;
        }
        else
        {
            *oy = 0;
        }
    }
    else
    {
        if (mtxtinfo->total_height < height)
        {
            *oy = (height - mtxtinfo->total_height) >> 1;
        }
        else
        {
            *oy = 0;
        }
    }

}

void osd_draw_multi_text_cell(PMULTI_TEXT p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32      __MAYBE_UNUSED__    bg_color     = 0;
    UINT8           b_align      = 0;
    UINT8           h_aligin     = 0;
    UINT8           v_align      = 0;
    PTEXT_CONTENT   p_text       = NULL;
    UINT16          width       = 0;
    UINT16          height      = 0;
    UINT16          left        = 0;
    UINT16          right       = 0;
    UINT16          bottom      = 0;
    INT16           lidx        = 0;
    UINT32          i           = 0;
    UINT8           *p_str       = NULL;
    UINT8           *p_draw_str   = NULL;
    UINT8           font        = 0;
    UINT16          ox          = 0;
    UINT16          oy          = 0;
    UINT16          x           = 0;
    UINT16          y           = 0;
    UINT16          l           = 0;
    UINT16          t           = 0;
    UINT32          b_new_line    = 0;
    BOOL            reverse_flag = FALSE;

#ifdef BIDIRECTIONAL_OSD_STYLE
    UINT16          line_count = 1;
    BOOL            mirror_flag = g_osd_rsc_info.osd_get_mirror_flag();
#endif
    struct thai_cell    cell;
    mtxtinfo_t          mtxtinfo;
    struct osdrect      r;
    MULTITEXT_PARAM     m_param;

    if(NULL == p_ctrl)
    {
        return ;
    }
    MEMSET(&m_param, 0x00, sizeof(MULTITEXT_PARAM));
    MEMSET(&cell, 0x00, sizeof(struct thai_cell));
    MEMSET(&mtxtinfo, 0x00, sizeof(mtxtinfo_t));
    osd_set_rect(&r, 0, 0, 0, 0);
    objframe    = &p_ctrl->head.frame;
    font        = p_ctrl->head.b_font;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    bg_color     = lp_win_sty->w_bg_idx;
    b_align      = p_ctrl->b_align;
    h_aligin     = GET_HALIGN(b_align);
    v_align      = GET_VALIGN(b_align);

    p_vscr = osd_draw_object_frame(objframe, b_style_idx);
    if(NULL == p_vscr)
    {
        ASSERT(0);
        return;
    }
    if (NULL != p_vscr->lpb_scr)
    {
        bg_color = C_NOSHOW;
    }

    r = *objframe;
    r.u_left += p_ctrl->rc_text.u_left;
    r.u_top  += p_ctrl->rc_text.u_top;
    r.u_width    = p_ctrl->rc_text.u_width;
    r.u_height   = p_ctrl->rc_text.u_height;
    width  = r.u_width;
    height = r.u_height;

GET_MULTITEXT_INFOR:
    mtxtinfo.topline_idx = (UINT8)p_ctrl->n_line;
    mtxtinfo.topline_pstr = NULL;
    osd_get_mtext_infor(p_ctrl, &mtxtinfo);
    if ((NULL == mtxtinfo.topline_pstr) && (0 != p_ctrl->n_line))
    {
        p_ctrl->n_line = 0;
        goto GET_MULTITEXT_INFOR;
    }
    if (NULL == mtxtinfo.topline_pstr)
    {
        return;
    }

    ox = 0;
    osd_get_draw_origination_point(&mtxtinfo,font,height,h_aligin,v_align,width,&ox,&oy);
    x = r.u_left + ox;
    y = r.u_top + oy;
    l = x;
    t = y;
    bottom = r.u_top + height;
    right  = r.u_left + width;
    left = r.u_left;

    lidx  = mtxtinfo.topline_idx ;
    b_new_line = 0;

    for (i = 0; i < p_ctrl->b_count; i++)
    {
        /* Draw from the top line*/
        if (i < mtxtinfo.topline_tblidx)
        {
            continue;
        }

        p_text = &p_ctrl->p_text_table[i];
        if (NULL == p_text)
        {
            continue;
        }

        reverse_flag = FALSE;
        if (STRING_ID == p_text->b_text_type)
        {
            p_str = osd_get_unicode_string(p_text->text.w_string_id);
        }
        else
        {
            p_str = (UINT8 *)p_text->text.p_string;
            if (STRING_REVERSE == p_text->b_text_type)
            {
                reverse_flag = TRUE;
            }
        }
        if (NULL == p_str)
        {
            continue;
        }

        if (i == mtxtinfo.topline_tblidx)   /* The top line's string pointer.*/
        {
            p_draw_str = mtxtinfo.topline_pstr;
        }
        else
        {
            p_draw_str = p_str;
        }

#ifdef BIDIRECTIONAL_OSD_STYLE
        if (reverse_flag || mirror_flag)
#else
        if (reverse_flag)
#endif
        {
            //force to right align
            x = r.u_left + r.u_width;
            l = x;
			
#ifdef BIDIRECTIONAL_OSD_STYLE
            RIGHT_LIBC("reverse_flag=%d mirror_flag= %d \n", reverse_flag,
                       mirror_flag);
#endif
            RIGHT_LIBC("lineWidth[%d] = %d \n", lidx, line_width[lidx]);
            l -=  r.u_width - line_width[lidx];

        }
#ifdef BIDIRECTIONAL_OSD_STYLE

        osd_multi_text_param_make(p_draw_str,&l,&t,&lidx,&mtxtinfo,&line_count,x, \
                    reverse_flag,&b_new_line,left,right,bottom,&m_param);
#else
        osd_multi_text_param_make(p_draw_str,&l,&t,&lidx,&mtxtinfo,NULL,x, \
                    reverse_flag,&b_new_line,left,right,bottom,&m_param);

#endif
        p_draw_str = osd_draw_multi_text_inner(&m_param,p_ctrl,width,b_style_idx,&r,p_vscr);
        if(NULL == p_draw_str)
        {
            break;
        }
        if (t >= bottom)
        {
            break;
        }
    }
    p_vscr->update_pending = 1;
    // TODO : Draw scroll bar
    if (NULL != p_ctrl->scroll_bar)
    {
        osd_set_scroll_bar_max(p_ctrl->scroll_bar, mtxtinfo.total_lines);
        osd_set_scroll_bar_page(p_ctrl->scroll_bar, mtxtinfo.page_lines);
        osd_set_scroll_bar_pos(p_ctrl->scroll_bar, mtxtinfo.topline_idx);
        osd_draw_object((POBJECT_HEAD)p_ctrl->scroll_bar, C_DRAW_SIGN_EVN_FLG);
    }
}

VACTION osd_multi_text_key_map(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;

    switch (key)
    {
        case V_KEY_UP:
            action = VACT_CURSOR_UP;
            break;
        case V_KEY_P_UP:
            action = VACT_CURSOR_PGUP;
            break;
        case V_KEY_DOWN:
            action = VACT_CURSOR_DOWN;
            break;
        case V_KEY_P_DOWN:
            action = VACT_CURSOR_PGDN;
            break;
        default:
            break;
    }

    return action;
}

PRESULT osd_multi_text_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT         result      = PROC_LOOP;
    PMULTI_TEXT     p_ctrl       = NULL;
    INT32           step        = 0;
    UINT16          total_line  = 0;
    UINT16          page_line   = 0;
    INT16           line_num    = 0;
    BOOL            exit_flag   = FALSE;
    BOOL            scroll_flag = FALSE;
    VACTION         action      = VACT_PASS;
    UINT32          vkey        = V_KEY_NULL;
    BOOL            b_continue   = FALSE;
    mtxtinfo_t      mtxt_info;

    if (NULL == p_obj)
    {
        return result;
    }

    MEMSET(&mtxt_info, 0x00, sizeof(mtxtinfo_t));
    p_ctrl = (PMULTI_TEXT)p_obj;
    if (MSG_TYPE_KEY == msg_type)
    {
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }

    GET_MULTITEXT_INFOR:
        mtxt_info.topline_idx = p_ctrl->n_line;
        osd_get_mtext_infor(p_ctrl, &mtxt_info);
        if ((mtxt_info.total_lines > 0) && (NULL == mtxt_info.topline_pstr))
        {
            p_ctrl->n_line = 0;
            goto GET_MULTITEXT_INFOR;
        }

        p_ctrl->n_line = mtxt_info.topline_idx;

        line_num = p_ctrl->n_line;
        page_line  = mtxt_info.page_lines;
        total_line = mtxt_info.total_lines;

        scroll_flag = FALSE;
        switch (action)
        {
            case VACT_CURSOR_UP:
                step = -1;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_DOWN:
                step = 1;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_PGUP:
                step = -page_line;
                scroll_flag = TRUE;
                break;
            case VACT_CURSOR_PGDN:
                step = page_line;
                scroll_flag = TRUE;
                break;
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION,(action << 16) | vkey, param1);
                if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
                {
                    return result;
                }
                break;
        }

        if (scroll_flag)
        {
            scroll_flag = FALSE;
            if (0 == total_line)
            {
                return PROC_LOOP;
            }
            line_num += step;

            if (line_num < 0)
            {
                line_num = 0;
            }

            if (line_num >= total_line)
            {
                line_num -= step;
            }

            if (p_ctrl->n_line == (UINT16)line_num)
            {
                return PROC_LOOP;
            }

            result = OSD_SIGNAL(p_obj, EVN_PRE_CHANGE, (UINT32)&line_num, 0);
            if (result != PROC_PASS)
            {
                goto CHECK_LEAVE;
            }

            p_ctrl->n_line = (UINT16)line_num;
            osd_track_object(p_obj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

            OSD_SIGNAL(p_obj, EVN_POST_CHANGE, line_num, 0);
            /*
            if(Result != PROC_PASS)
                goto CHECK_LEAVE;
               */
            result = PROC_LOOP;
        }


        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(osd_msg_type==MSG_TYPE_EVNT)
    {
        result = OSD_SIGNAL(p_obj, msg_type, msg, param1);
    }

CHECK_LEAVE:
    if (!(EVN_PRE_OPEN == msg_type))
    {
        CHECK_LEAVE_RETURN(result, p_obj);
    }

EXIT:
    return result;
}


