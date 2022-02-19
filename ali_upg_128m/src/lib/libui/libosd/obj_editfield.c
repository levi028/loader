/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_editfield.c
*    Description: editfield object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"
#include "obj_editfield_inner.h"
#define EDF_PRINTF  PRINTF//soc_printf
//---------------------------- PRIVATE VARIABLES ----------------------------//
static UINT8 g_time_mode = 0;    //0 for AM ; 1 for PM
static char *m_fmt_dt[] =
{
    "Y4M2D2","Y2M2D2","M2D2Y4","M2D2Y2","D2M2Y4","h2m2s2","h2m2","m2s2",
};
static char *m_sep = ".-/:";
static BOOL key_input_changed = FALSE; // only for pattern == 'm'
static UINT32 convert_ip2int(UINT16 *p_string);
static void convert_value2string(PEDIT_FIELD p_ctrl, UINT16 *p_string, UINT32 dw_value, PEDIT_PATTERN p_pattern);
static PRESULT notify_change_event(PEDIT_FIELD p_ctrl, UINT16 *p_text, PEDIT_PATTERN p_pattern);
//---------------------------- PRIVATE FUNCTIONS ----------------------------//
static UINT8 get_time_mode(void)
{
    return g_time_mode;
}

static void set_time_mode(UINT8 val)
{
    g_time_mode = val;
}

static BOOL is_edit_string(UINT8 b_tag)
{
    return (('p' == b_tag) || ('s' == b_tag) || ('m' == b_tag));
}

static void get_pattern(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    char            *pcs_string  = NULL ;
    char            *pfmt       = NULL;
    UINT32          i           = 0;
    UINT32          j           = 0;
    UINT8           b_len = 0;

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return ;
    }
    pcs_string = p_ctrl->pcs_pattern;
    MEMSET(p_pattern, 0, sizeof(EDIT_PATTERN));
    p_pattern->b_len = p_ctrl->b_max_len;
    p_pattern->b_max_len = p_ctrl->b_max_len;
    if (!pcs_string)
    {
        return;
    }
    switch (pcs_string[0])
    {
        case 'f':
            p_pattern->f.b_sub = pcs_string[2] - '0';
            OSD_ASSERT(p_pattern->f.b_sub);
            p_pattern->b_len = pcs_string[1] - '0' + p_pattern->f.b_sub;
            p_pattern->w_cursor_map |= (1 << p_pattern->f.b_sub);
            p_pattern->b_max_len = p_pattern->b_len + 1;
            p_pattern->b_tag = 'f';
            break;
        case 'p':
            p_pattern->p.b_init   = pcs_string[1];
            p_pattern->p.b_mask   = pcs_string[2];
            p_pattern->b_len      = pcs_string[3] - '0';
            p_pattern->b_max_len   = p_pattern->b_len;
            p_pattern->b_tag      = 'p';
            break;
        case 'r':
            p_pattern->w_cursor_map = 0;
            p_pattern->r.n_max = 0;
            p_pattern->r.n_min = 0;
            for (i = 1; pcs_string[i] != '~'; i++)
            {
                p_pattern->r.n_min = p_pattern->r.n_min * 10 + (pcs_string[i] - '0');
            }
            for (j = i + 1; pcs_string[j]; j++)
            {
                p_pattern->r.n_max = p_pattern->r.n_max * 10 + (pcs_string[j] - '0');
            }
            p_pattern->b_len = j - i - 1;
            p_pattern->b_max_len = p_pattern->b_len;
            p_pattern->b_tag = 'r';
            break;
#if EDITFIELD_SUPPORT_TIME
        case 't':
        case 'T':
        case 'd':
            if (('t' == pcs_string[0]) || ('T' == pcs_string[0]))
            {
                p_pattern->dt.b_format = pcs_string[1] - '0' + C_TIME_FMT_OFFWSET;
                p_pattern->dt.b_sep = C_TIME_SEP;
            }
            else
            {
                p_pattern->dt.b_format = pcs_string[1] - '0';
                p_pattern->dt.b_sep = pcs_string[2] - '0';
            }
            pfmt = m_fmt_dt[p_pattern->dt.b_format];
            b_len = 0;
            p_pattern->b_len = 0;
            for (i = 0; i < STRLEN(pfmt); i += 2)
            {
                b_len = pfmt[i + 1] - '0';
                if (i)
                {
                    p_pattern->w_cursor_map <<= (b_len + 1);
                    p_pattern->w_cursor_map |= (1 << b_len);
                }
                p_pattern->b_len += b_len;
            }
            p_pattern->b_max_len = p_pattern->b_len + (((UINT8)i) / 2 - 1);
            if ('T' == pcs_string[0])
            {
                p_pattern->b_tag = 'T';
                p_pattern->w_cursor_map <<= 3; //plus three char. '<space>' 'A/P' 'M'
                p_pattern->w_cursor_map |= 5; //allow to change for second char 'A/P'
                p_pattern->b_max_len += 3;
            }
            else
            {
                p_pattern->b_tag = 't';
            }
            break;
#endif
        case 's':
            p_pattern->s.p_string = p_ctrl->p_string;
            p_pattern->b_len = 0;
            for (i = 1; pcs_string[i]; i++)
            {
                p_pattern->b_len = p_pattern->b_len * 10 + (pcs_string[i] - '0');
            }
            p_pattern->b_max_len = p_pattern->b_len;
            p_pattern->b_tag = 's';
            break;
        case 'i':
            p_pattern->w_cursor_map = 0x888;
            p_pattern->ip.b_sep = pcs_string[1] - '0';
            p_pattern->b_tag = 'i';
            p_pattern->b_len = 15;
            p_pattern->b_max_len = p_pattern->b_len;
            break;
        case 'm':
            p_pattern->m.b_init   = pcs_string[1];
            p_pattern->m.b_mask   = pcs_string[2];
            p_pattern->b_len = 0;
            for (i = 3; pcs_string[i]; i++)
            {
                p_pattern->b_len = p_pattern->b_len * 10 + (pcs_string[i] - '0');
            }
            if (p_pattern->b_len > p_ctrl->b_max_len)
            {
                p_pattern->b_len = p_ctrl->b_max_len;
            }
            p_pattern->b_max_len   = p_pattern->b_len;
            p_pattern->b_tag      = 'm';
            break;
        default :
            break;
    }
    return ;
}

/*  Get cursor type in edit mode.
    'p' - CURSOR_NO
    'r' - CURSOR_NO / CURSOR_NORMAL / CURSOR_SPECIAL
    others('f', 't' and 'd') CURSOR_NORMAL*/
static UINT32 get_edit_cursor_type(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32  cursor_type = 0;

    if(NULL == p_pattern)
    {
        return 0;
    }
    switch (p_pattern->b_tag)
    {
        case 'r':
            if(NULL == p_ctrl)
            {
                return 0;
            }
            cursor_type = p_ctrl->b_cursor_mode;
            break;
        case 'm':
        case 's':
            if(NULL == p_ctrl)
            {
                return 0;
            }
            if (CURSOR_NO != p_ctrl->b_cursor_mode)
            {
                cursor_type = CURSOR_NORMAL;
            }
            else
            {
                cursor_type = p_ctrl->b_cursor_mode;
            }
            break;
        case 'f':
        case 'i':
#if EDITFIELD_SUPPORT_TIME
        case 't':
        case 'd':
        case 'T':
#endif
            cursor_type = CURSOR_NORMAL;
            break;
        case 'p':
            cursor_type = CURSOR_NO;
            break;
        default:
            cursor_type = CURSOR_NO;
            break;
    }
    return cursor_type;
}

//Get cursor type in focus mode.
static UINT32 get_focus_cursor_type(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32      cursor_type = 0;
    UINT32      mode        = 0;
    UINT32      status      = 0;

    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    cursor_type = get_edit_cursor_type(p_ctrl, p_pattern);
    if (CURSOR_NORMAL == cursor_type)
    {
        if ((EDIT_STATUS ==  status)|| (NORMAL_EDIT_MODE ==  mode))
        {
            return CURSOR_NORMAL;
        }
    }
    else if (CURSOR_SPECIAL ==  cursor_type)
    {
        if (EDIT_STATUS == status)
        {
            return CURSOR_SPECIAL;
        }
    }
    return CURSOR_NO;
}

static BOOL check_focus_can_input09(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32      mode    = 0;
    UINT32      status  = 0;

    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    if ((EDIT_STATUS == status) || (NORMAL_EDIT_MODE == mode))
    {
        return TRUE;
    }
    else    /* SELECT_EDIT_MODE && status != EDIT_STATUS*/
    {
        return FALSE;
    }
}

static BOOL check_focus_can_addsub(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32      cursor_type = 0;
    UINT32     __MAYBE_UNUSED__  mode        = 0;
    UINT32      status      = 0;

    if((NULL == p_ctrl)||(NULL == p_pattern))
    {
        return FALSE;
    }
    
    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    
    if (p_pattern->b_tag != 'r')
    {
        return FALSE;
    }
    cursor_type = p_ctrl->b_cursor_mode;
    if (CURSOR_NO == cursor_type)
    {
        return TRUE;
    }
    else if (CURSOR_NORMAL == cursor_type)
    {
        return FALSE;
    }
    else    // CURSOR_SPECIAL
    {
        return (SELECT_STATUS == status) ? TRUE : FALSE;
    }
}

static void check_cursor_pos(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32      cursor_type = 0;
    INT32       i           = 0;
    INT8        cursor      = 0;
    UINT8       b_max_len     = 0;
    UINT8       str_len      = 0;

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return ;
    }
    b_max_len = p_pattern->b_max_len;
    if ('p' == p_pattern->b_tag)
    {
        b_max_len += 1;
    }
    else if (('s' == p_pattern->b_tag) || ('m' == p_pattern->b_tag))
    {
        str_len =  com_uni_str_len(p_ctrl->p_string);
        if (str_len < b_max_len)
        {
            str_len = str_len + 1;
        }
    }
    if (p_ctrl->b_cursor >= b_max_len)
    {
        p_ctrl->b_cursor = 0;
    }
    cursor_type = get_edit_cursor_type(p_ctrl, p_pattern);
    if (cursor_type != CURSOR_NORMAL)
    {
        return;
    }
    if ((p_pattern->b_tag != 's') && (p_pattern->b_tag != 'm'))
    {
        cursor = p_ctrl->b_cursor;
        for (i = 0; i < p_pattern->b_max_len; i++)
        {
            cursor = p_ctrl->b_cursor;
            if (p_pattern->w_cursor_map & (1 << (p_pattern->b_max_len - cursor - 1)))
            {
                if (cursor < (p_pattern->b_max_len - 1))
                {
                    cursor++;
                }
                else
                {
                    cursor = 0;
                }
            }
        }
        p_ctrl->b_cursor = cursor;
    }
    return ;
}
static BOOL check_time_input_value(PEDIT_FIELD p_ctrl,PEDIT_PATTERN p_pattern)
{
    UINT8 time_type = 0;
    BOOL ret = FALSE;
    UINT32 value = 0;
    UINT32 __MAYBE_UNUSED__ h_value = 0;
    UINT32 m_value = 0;    
    UINT32 s_value = 0;
    UINT16  __MAYBE_UNUSED__ h_str[2] = {0,0};
    UINT16 m_str[2] = {0,0};
    UINT16 s_str[2] = {0,0};
    if(p_pattern->b_tag != 'T' && p_pattern->b_tag != 't')
    {
        return FALSE;
    }
    time_type = p_pattern->dt.b_format - C_TIME_FMT_OFFWSET;
    value = com_uni_str2int(p_ctrl->p_string);
    switch(time_type)
    {
    case 0 :
        if(p_pattern->b_max_len != 8)
        {
            return FALSE;
        }
        h_value = value/10000;
        m_value = (value%10000)/100;
        s_value = value%100;        
//        if(p_pattern->b_tag == 't')
//        {
//            if(h_value/10 > 2)
//            {
//                h_value = h_value%10 + 20;
//            }
//            else if(h_value/10 == 2)
//            {
//                if(h_value%10 > 3)
//                {
//                    h_value = 23;
//                }
//            }
//        }
//        else(p_pattern->b_tag == 'T')
//        {
//            if(h_value/10 > 1)
//            {
//                h_value = h_value%10 + 10;
//            }
//            else if(h_value/10 == 1)
//            {
//                if(h_value%10 > 1)
//                {
//                    h_value = 11;
//                }
//            }            
//        }
        if(m_value/10 >= 6)
        {
            m_value = m_value%10 + 50;
        }
        if(s_value/10 >= 6)
        {
            s_value = s_value%10 + 50;
        }
        //com_int2uni_str(h_str, h_value, 2);
        com_int2uni_str(m_str, m_value, 2);
        com_int2uni_str(s_str, s_value, 2);
        //p_ctrl->p_string[0] = h_str[0];
        //p_ctrl->p_string[1] = h_str[1];
        p_ctrl->p_string[3] = m_str[0];
        p_ctrl->p_string[4] = m_str[1];
        p_ctrl->p_string[6] = s_str[0];
        p_ctrl->p_string[7] = s_str[1];
        ret = TRUE;
        break;        
    case 1 :
        if(p_pattern->b_max_len != 5)
        {
            return FALSE;
        }
        h_value = value/100;
        m_value = value%100; 
//        if(p_pattern->b_tag == 't')
//        {
//            if(h_value/10 > 2)
//            {
//                h_value = h_value%10 + 20;
//            }
//            else if(h_value/10 == 2)
//            {
//                if(h_value%10 > 3)
//                {
//                    h_value = 23;
//                }
//            }
//        }
//        else(p_pattern->b_tag == 'T')
//        {
//            if(h_value/10 > 1)
//            {
//                h_value = h_value%10 + 10;
//            }
//            else if(h_value/10 == 1)
//            {
//                if(h_value%10 > 1)
//                {
//                    h_value = 11;
//                }
//            }            
//        }
        if(m_value/10 >= 6)
        {
            m_value = m_value%10 + 50;
        }
        //com_int2uni_str(h_str, h_value, 2);
        com_int2uni_str(m_str, m_value, 2);
        //pCtrl->pString[0] = h_str[0];
        //pCtrl->pString[1] = h_str[1];
        p_ctrl->p_string[3] = m_str[0];
        p_ctrl->p_string[4] = m_str[1];           
        ret = TRUE;             
        break;
    case 2 :
        if(p_pattern->b_max_len != 5)
        {
            return FALSE;
        }
        m_value = value/100;
        s_value = value%100;
        if(m_value/10 >= 6)
        {
            m_value = m_value%10 + 50;
        }
        if(s_value/10 >= 6)
        {
            s_value = s_value%10 + 50;
        } 
        com_int2uni_str(s_str, s_value, 2);
        com_int2uni_str(m_str, m_value, 2);
        p_ctrl->p_string[0] = m_str[0];
        p_ctrl->p_string[1] = m_str[1];
        p_ctrl->p_string[3] = s_str[0];
        p_ctrl->p_string[4] = s_str[1];  
        ret = TRUE;             
        break;        
    default :
        break;
    }
    return ret;
}

static INT8 get_next_cursor_pos(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern, INT8 step)
{
    INT8        cursor  = 0;
    UINT8       b_max_len = 0;
    UINT8       str_len  = 0;

    if(NULL == p_pattern)
    {
        return -1;
    }
    b_max_len = p_pattern->b_max_len;
    if ('p' == p_pattern->b_tag)
    {
        b_max_len += 1;
    }
    else if (('s' == p_pattern->b_tag) || ('m' == p_pattern->b_tag))
    {
        if (NULL == p_ctrl)
        {
            return -1;
        }
        str_len =  com_uni_str_len(p_ctrl->p_string);
        if (str_len < b_max_len)
        {
            str_len = str_len + 1;
        }
    }
    if(b_max_len > 0)
    {
        if (NULL == p_ctrl)
        {
            return -1;
        }
        cursor = (p_ctrl->b_cursor + step + b_max_len) % b_max_len;
    }
    if (!(('s' == p_pattern->b_tag) && ('p' == p_pattern->b_tag)))
    {
        if (p_pattern->w_cursor_map & (1 << (p_pattern->b_max_len - cursor - 1)))
        {
            cursor += step;
        }
        if ('T' == p_pattern->b_tag)
        {
            if (cursor >= p_pattern->b_max_len)
            {
                cursor = 0;
            }
        }
    }
    return cursor;
}

static BOOL check_number_value(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32     __MAYBE_UNUSED__ mode    = 0;
    UINT32    __MAYBE_UNUSED__  status  = 0;
    UINT32      dval    = 0;
    UINT32      value   = 0;
    UINT32      valbak  = 0;
    UINT32      min     = 0;
    UINT32      max     = 0;
    UINT16      ws_text[C_MAX_EDIT_LENGTH];

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return FALSE;
    }
    MEMSET(ws_text, 0x00, sizeof(ws_text));
    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    dval = osd_get_edit_field_default_val(p_ctrl);

    if (('r' == p_pattern->b_tag) && (CURSOR_SPECIAL == p_ctrl->b_cursor_mode))
    {
        value = com_uni_str2int(p_ctrl->p_string);
        //OSD_GetEditFieldIntValue(pCtrl,&value);
        min = p_pattern->r.n_min;
        max = p_pattern->r.n_max;
        valbak = p_ctrl->valbak;
        if ((value < min) || (value > max))
        {
            if (FORCE_TO_VALBAK == dval)
            {
                value = valbak;
            }
            else
            {
                if ((valbak >= min) && (valbak <= max))
                {
                    value = valbak;
                }
                else
                {
                    value = (value < min) ? min : max;
                }
            }
            convert_value2string(p_ctrl, ws_text, value, p_pattern);
            notify_change_event(p_ctrl, ws_text, p_pattern);
            return TRUE;
        }
    }
    return FALSE;
}

static void check_number_edit_mode(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32   __MAYBE_UNUSED__   mode    = 0;
    UINT32      status  = 0;
    UINT8       len     = 0;
    BOOL ret = FALSE;

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return ;
    }
    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    if (('r' == p_pattern->b_tag) && (EDIT_STATUS == status) && (CURSOR_SPECIAL == p_ctrl->b_cursor_mode))
    {
        len = com_uni_str_len(p_ctrl->p_string);
        if (len >=  p_pattern->b_max_len)
        {
            p_ctrl->b_style &= ~EDIT_STATUS;
            ret = check_number_value(p_ctrl, p_pattern);
            if(!ret)
            {
                return;
            }
        }
    }
}

static void get_edit_text(PEDIT_FIELD p_ctrl, UINT16 *p_buf, int n_size, PEDIT_PATTERN pattern)
{
    UINT8   len = 0;

    if((NULL == p_ctrl) || (NULL == pattern) || (n_size == 0))
    {
        return ;
    }
    switch (pattern->b_tag)
    {
        case 'p':   // password
            if(p_ctrl->b_cursor>(n_size-1))
            {
                //libc_printf("get_edit_text1: b_cursor > n_size, %d, %d\n", p_ctrl->b_cursor, n_size);
                len = n_size -1;
                com_uni_str_mem_set(p_buf, pattern->p.b_mask, len);//p_ctrl->b_cursor);
            }
            else
            {
                len = p_ctrl->b_cursor;
                com_uni_str_mem_set(p_buf, pattern->p.b_mask, len);//p_ctrl->b_cursor);
                com_uni_str_mem_set(&p_buf[p_ctrl->b_cursor], pattern->p.b_init, pattern->b_len - p_ctrl->b_cursor);
                com_uni_str_mem_set(&p_buf[pattern->b_len], ' ', 1);
            }
            break;
        case 'm':
            len = com_uni_str_len(p_ctrl->p_string);
            if (p_ctrl->b_cursor > len)
            {
                p_ctrl->b_cursor = len - 1;
                ASSERT(0);
            }

            if(len>(n_size -1))
            {
                //libc_printf("get_edit_text2: len > n_size, %d, %d\n", len, n_size);
                com_uni_str_mem_set(p_buf, pattern->m.b_mask, n_size -1);
            }
            else
            {
                com_uni_str_mem_set(p_buf, pattern->m.b_mask, len);
                if (key_input_changed)
                {
                    p_buf[p_ctrl->b_cursor] = p_ctrl->p_string[p_ctrl->b_cursor];
                }
                com_uni_str_mem_set(&p_buf[len], ' ', 1);
            }
            break;
        default:
            com_uni_str_copy_ex(p_buf, p_ctrl->p_string, n_size - 1);
            break;
    }
}

static UINT32 check_range_value(INT32 value, INT32 n_min, INT32 n_max, BOOL f_loop)
{
    if (value < n_min)
    {
        value = f_loop ? n_max : n_min;
    }
    else if (value > n_max)
    {
        value = f_loop ? n_min : n_max;
    }
    return value;
}

static BOOL get_value(PEDIT_FIELD p_ctrl, UINT32 *p_value)
{
    UINT32          value   = 0;
    UINT32          dval    = 0;
    EDIT_PATTERN    pattern;

    if((NULL == p_ctrl) || (NULL == p_value))
    {
        return FALSE;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    get_pattern(p_ctrl, &pattern);
    if (('p' == pattern.b_tag) || ('s' == pattern.b_tag) || ('m' == pattern.b_tag))
    {
        return FALSE;
    }
    if ('i' == pattern.b_tag)
    {
        *p_value = convert_ip2int(p_ctrl->p_string);
    }
    else
    {
        *p_value = com_uni_str2int(p_ctrl->p_string);
    }
    if ('P' == (UINT8)(p_ctrl->p_string[6] >> 8))
    {
        set_time_mode(1);    //ui set am/pm, not decide by value.  0 for am, 1 for pm
    }
    if ('r' == pattern.b_tag)
    {
    CHECK_RANGE:
        value = check_range_value(*p_value, pattern.r.n_min, pattern.r.n_max, FALSE);
        if (value != *p_value)
        {
            value = p_ctrl->valbak;
            *p_value = value;
            dval = osd_get_edit_field_default_val(p_ctrl);
            if (dval != FORCE_TO_VALBAK)
            {
                goto CHECK_RANGE;
            }
        }
    }
    return TRUE;
}

static BOOL convert_ip2int2(UINT16 *p_string, UINT32 *p_value)
{
    //strings: "192.168.9.1"
    UINT32      i       = 0;
    UINT32      j       = 0;
    UINT32      data    = 0;
    UINT32      temp    = 0;
    UINT32      pos[4]  = {0};
    char        asc_str[32] = {0};

    if(NULL == p_value)
    {
        return FALSE;
    }
    com_uni_str_to_asc((UINT8 *)p_string, asc_str);
    data = 0;
    for (i = 0, j = 0; i < STRLEN(asc_str); i++)
    {
        if (!IS_NUMBER_CHAR(asc_str[i]))
        {
            pos[j] = i;
            j++;
        }
    }
    ASSERT(3 == j);
    for (i = 0, j = 0, data = 0; i < 4; i++)
    {
        temp = ATOI(&asc_str[j]);
        if (temp > 0xff)
        {
            *p_value = temp;
            return FALSE;
        }
        data += (temp << ((3 - i) * 8));
        j = pos[i] + 1;
    }
    *p_value = data;
    return TRUE;
}

static UINT32 convert_ip2int(UINT16 *p_string)
{
    UINT32 value = 0;

    if (!convert_ip2int2(p_string, &value))
    {
        value = 0;
    }
    return value;
}

static UINT32 auto_correct_ip(UINT16 *p_string, UINT16 pos, UINT8 num)
{
    UINT32  value   = 0;
    UINT32  offset  = pos % 4;
    UINT32  tens    = 0;
    const   UINT32   m_const_3   = 3;
    const   UINT32   m_const_5   = 5;

    if (!convert_ip2int2(p_string, &value))
    {
        if (0 == offset)
        {
            if ((value / 100) > m_const_3)// error, do nothing.
            {
                return FALSE;
            }
            else
            {
                set_uni_str_char_at(p_string, '0', pos + 1);
                set_uni_str_char_at(p_string, '0', pos + 2);
            }
        }
        else if (1 == offset)
        {
            tens = (value % 100) / 10;
            if (tens > m_const_5)  // error, do nothing.
            {
                return FALSE;
            }
            else
            {
                set_uni_str_char_at(p_string, '0', pos + 1);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static void convert_value2string(PEDIT_FIELD p_ctrl, UINT16 *p_string, UINT32 dw_value, PEDIT_PATTERN p_pattern)
{
    UINT32          i       = 0;
    UINT32          j       = 0;
    UINT32          len     = 0;
    UINT32          cnt     = 0;
    UINT32          dw_temp  = 1;
    UINT16          w_sep    = 0;
    UINT32          status  = osd_get_edit_field_status(p_ctrl);
    const UINT32    m_const_3 = 3;
    const UINT32    m_const_4 = 4;
    UINT32          dw_quotient  = 0;
    UINT32          dw_remainder = 0;
    UINT8           b_sub        = 0;
    char            *pfmt       = 0;

    if((NULL == p_pattern) || (NULL == p_string))
    {
        return ;
    }
#if EDITFIELD_SUPPORT_TIME
    if (('t' == p_pattern->b_tag) || ('T' == p_pattern->b_tag))
    {
        pfmt = m_fmt_dt[p_pattern->dt.b_format];

        for (i = 0; i < p_pattern->b_len; i++)
        {
            dw_temp *= 10;
        }
        p_string[0] = 0;
        w_sep = (UINT16)(m_sep[p_pattern->dt.b_sep]);
        for (i = 0, len = 0; i < STRLEN(pfmt); i += 2)
        {
            cnt = pfmt[i + 1] - '0';
            for (j = 0; j < cnt; j++)
            {
                dw_temp /= 10;
            }
            if(dw_temp > 0)
            {
                len += com_int2uni_str(&p_string[len], dw_value / dw_temp, cnt);
            }
            if (dw_temp > 1)
            {
                set_uni_str_char_at(&p_string[len], w_sep, 0);
                len++;
            }
            if(dw_temp > 0)
            {
                dw_value %= dw_temp;
            }
        }
        if ('T' == p_pattern->b_tag)
        {
            set_uni_str_char_at(&p_string[len + 2], 'M', 0);
            set_uni_str_char_at(&p_string[len], ' ', 0); //space
            if(NULL == p_ctrl)
            {
                return ;
            }
            if ((SELECT_STATUS == status) || ((EDIT_STATUS == status) && (p_ctrl->b_cursor == (len + 1))))
            {
                set_uni_str_char_at(&p_string[len + 1], (get_time_mode() ? 'P' : 'A'), 0);
            }
            len += 3;
        }
        p_string[len] = 0;
        len++;
    }
    else if ('i' == p_pattern->b_tag)
#else
    if ('i' == p_pattern->b_tag)
#endif
    {
        w_sep = (UINT16)(m_sep[p_pattern->ip.b_sep]);
        for (i = 0, len = 0; i < m_const_4; i++)
        {
            len += com_int2uni_str(&p_string[len], (UINT8)(dw_value >> ((3 - i) * 8)), 3);
            if (i < m_const_3)
            {
                set_uni_str_char_at(&p_string[len], w_sep, 0);
                len++;
            }
        }
        p_string[len] = 0;
        len++;
    }
    else
    {
        if ('f' == p_pattern->b_tag)
        {
            b_sub = p_pattern->f.b_sub;
        }
        for (i = 0; i < b_sub; i++)
        {
            dw_temp *= 10;
        }
        dw_quotient = dw_value / dw_temp;
        dw_remainder = dw_value % dw_temp;
        len = p_pattern->b_len - b_sub;
        if ('r' == p_pattern->b_tag)
        {
            if(NULL == p_ctrl)
            {
                return ;
            }
            if ((CURSOR_NO == p_ctrl->b_cursor_mode)|| (CURSOR_SPECIAL == p_ctrl->b_cursor_mode))
            {
                len = 0;
            }
        }
        com_int2uni_str(p_string, dw_quotient, len);
        if (b_sub)
        {
            len = com_uni_str_len(p_string);
            set_uni_str_char_at(p_string, '.', len);
            len++;
            com_int2uni_str(&(p_string[len]), dw_remainder, b_sub);
        }
    }
}

static PRESULT notify_change_event(PEDIT_FIELD p_ctrl, UINT16 *p_text, PEDIT_PATTERN p_pattern)
{
    PRESULT     result  = PROC_PASS;
    UINT32      dw_value = 0;

    if(NULL == p_pattern)
    {
        return PROC_PASS;
    }
    if (('p' == p_pattern->b_tag) || ('s' == p_pattern->b_tag) || ('m' == p_pattern->b_tag))
    {
        result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_PRE_CHANGE, (UINT32)p_text, 0);
        if (PROC_PASS == result)
        {
            if(NULL == p_ctrl)
            {
                return PROC_PASS;
            }
            //com_uni_str_copy(p_ctrl->p_string, p_text);
            com_uni_str_copy_ex(p_ctrl->p_string, p_text, p_ctrl->b_max_len -1);
            result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_POST_CHANGE, (UINT32)p_text, 0);
        }
    }
    else
    {
        if ('i' == p_pattern->b_tag)
        {
            if (!convert_ip2int2(p_text, &dw_value))
            {
                if(NULL == p_ctrl)
                {
                    return PROC_PASS;
                }
                result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_DATA_INVALID,
                                    (UINT32)&dw_value, convert_ip2int(p_ctrl->p_string));
                return result;
            }
        }
        else
        {
            dw_value = com_uni_str2int(p_text);
        }
        result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_PRE_CHANGE,(UINT32)&dw_value, 0);
        if (PROC_PASS == result)
        {
            if(NULL == p_ctrl)
            {
                return PROC_PASS;
            }
            convert_value2string(p_ctrl, p_ctrl->p_string, dw_value, p_pattern);
            result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_POST_CHANGE, dw_value, 0);
        }
		if(p_pattern->b_tag == 'T' || p_pattern->b_tag == 't')
        {
            check_time_input_value(p_ctrl, p_pattern);
        }
        check_number_edit_mode(p_ctrl, p_pattern);
    }
    return result;
}

static void move_edit_cursor(PEDIT_FIELD p_ctrl, INT8 step, PEDIT_PATTERN p_pattern)
{
    PRESULT     result  = PROC_PASS;
    INT8        cursor  = 0;

    cursor = get_next_cursor_pos(p_ctrl, p_pattern, step);
    if(-1 == cursor)
    {
        return;
    }
    result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_CURSOR_PRE_CHANGE,(UINT32)&cursor, 0);
    if (PROC_PASS == result)
    {
        if(NULL == p_ctrl)
        {
            return ;
        }
        p_ctrl->b_cursor = cursor;
        result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_CURSOR_POST_CHANGE, cursor, 0);
        if ((result != PROC_PASS) && (result != PROC_LOOP)&& (result != PROC_LEAVE))
        {
            return;
        }
        osd_track_object((POBJECT_HEAD)p_ctrl, C_UPDATE_ALL/*C_MOVE_CURSOR*/);
    }
}

static PRESULT change_edit_value(PEDIT_FIELD p_ctrl, INT8 num, PEDIT_PATTERN p_pattern, BOOL f_move_cursor)
{
    PRESULT     result  = PROC_LOOP;
    UINT        str_len  = 0;
    UINT32      dw_value = 0;
    UINT16      ws_text[C_MAX_EDIT_LENGTH];

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return PROC_LOOP;
    }
    MEMSET(ws_text, 0x00, sizeof(ws_text));
    str_len = com_uni_str_len(p_ctrl->p_string);
    if (('p' == p_pattern->b_tag) && (p_ctrl->b_cursor == p_pattern->b_max_len))
    {
        p_ctrl->b_cursor = 0;
    }
    if (p_pattern->b_tag != 'p')    //make more key can be password
    {
        num = (num + 10) % 10;
    }
    //com_uni_str_copy(ws_text, p_ctrl->p_string);
    com_uni_str_copy_ex(ws_text, p_ctrl->p_string, C_MAX_EDIT_LENGTH-1);
    if (('r' == p_pattern->b_tag) && (f_move_cursor) && (p_ctrl->b_cursor) && (CURSOR_SPECIAL == p_ctrl->b_cursor_mode))
    {
        dw_value = com_uni_str2int(ws_text);
        if (0 == dw_value)
        {
            p_ctrl->b_cursor = 0;
        }
    }
    if ('T' == p_pattern->b_tag)
    {
        if ('P' == get_uni_str_char_at(ws_text, p_ctrl->b_cursor))
        {
            set_time_mode(0);
        }
        else if ('A' == get_uni_str_char_at(ws_text, p_ctrl->b_cursor))
        {
            set_time_mode(1);
        }
        else
        {
            set_uni_str_char_at(ws_text, (num + '0'), p_ctrl->b_cursor);
        }
    }
    else
    {
        set_uni_str_char_at(ws_text, (num + '0'), p_ctrl->b_cursor);
    }
    if ('p' == p_pattern->b_tag)
    {
        ws_text[p_ctrl->b_cursor + 1] = 0;
        f_move_cursor = TRUE;
    }
    else if (('s' == p_pattern->b_tag) || ('m' == p_pattern->b_tag))
    {
        if (p_ctrl->b_cursor == str_len)
        {
            if (str_len < p_ctrl->b_max_len)
            {
                ws_text[p_ctrl->b_cursor + 1] = 0;
            }
        }
    }
    else if ('r' == p_pattern->b_tag)
    {
        if (p_ctrl->b_cursor_mode != CURSOR_NORMAL)
        {
            ws_text[p_ctrl->b_cursor + 1] = 0;
        }
    }
    else if ('i' == p_pattern->b_tag)
    {
        auto_correct_ip(ws_text, p_ctrl->b_cursor, num + '0');
    }
    result = notify_change_event(p_ctrl, ws_text, p_pattern);
    if (PROC_PASS == result)
    {
        if (('p' == p_pattern->b_tag) && ((p_ctrl->b_cursor + 1) >= p_pattern->b_max_len))
        {
            p_ctrl->b_cursor = 0;
            f_move_cursor = FALSE;
        }
        osd_track_object((POBJECT_HEAD)p_ctrl, C_UPDATE_CONTENT);
        if (f_move_cursor)
        {
            move_edit_cursor(p_ctrl, 1, p_pattern);
        }
    }
    return result;
}

static void step_edit_value(PEDIT_FIELD p_ctrl, INT8 num, PEDIT_PATTERN p_pattern)
{
    UINT32      value       = 0;
    UINT32      new_value    = 0;
    PRESULT     result      = 0;

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return ;
    }
    if ('i' == p_pattern->b_tag)
    {
        value = convert_ip2int(p_ctrl->p_string);
    }
    else
    {
        value = com_uni_str2int(p_ctrl->p_string);
    }
    new_value = value + num;
    new_value = check_range_value(new_value, p_pattern->r.n_min, p_pattern->r.n_max, TRUE);
    result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_PRE_CHANGE, (UINT32)&new_value, value);
    if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
    {
        return;
    }
    convert_value2string(p_ctrl, p_ctrl->p_string, new_value, p_pattern);
    osd_track_object((POBJECT_HEAD)p_ctrl, C_UPDATE_CONTENT);
    result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_POST_CHANGE, new_value, value);
    if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
    {
        return;
    }
}

static void devide_edit_value(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern)
{
    UINT32      value       = 0;
    UINT32      new_value    = 0;
    PRESULT     result      = 0;

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return ;
    }
    if ('i' == p_pattern->b_tag)
    {
        value = convert_ip2int(p_ctrl->p_string);
    }
    else
    {
        value = com_uni_str2int(p_ctrl->p_string);
    }
    new_value = value / 10;
    if (value == new_value)
    {
        return;
    }
    result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_PRE_CHANGE, (UINT32)&new_value, value);
    if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
    {
        return;
    }
    convert_value2string(p_ctrl, p_ctrl->p_string, new_value, p_pattern);
    if (p_ctrl->b_cursor > 0)
    {
        p_ctrl->b_cursor -= 1;
    }
    result = OSD_SIGNAL((POBJECT_HEAD)p_ctrl, EVN_POST_CHANGE, new_value, value);
    if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
    {
        return;
    }
    osd_track_object((POBJECT_HEAD)p_ctrl, C_UPDATE_CONTENT);
}

static BOOL backup_restore_value(PEDIT_FIELD p_ctrl, PEDIT_PATTERN p_pattern, BOOL b_backup)
{

    if((NULL == p_ctrl) || (NULL == p_pattern))
    {
        return FALSE;
    }
    /* Only number, date and time can backup or restore */
    if ((p_pattern->b_tag != 'r')&& (p_pattern->b_tag != 'd')&& (p_pattern->b_tag != 'i')
         &&(p_pattern->b_tag != 't')&& (p_pattern->b_tag != 'f')&& (p_pattern->b_tag != 'T'))
    {
        return FALSE;
    }
    if (b_backup)
    {
        if ('i' == p_pattern->b_tag)
        {
            p_ctrl->valbak = convert_ip2int(p_ctrl->p_string);
        }
        else
        {
            p_ctrl->valbak = com_uni_str2int(p_ctrl->p_string);
        }
        EDF_PRINTF("Backup value : %d \n", p_ctrl->valbak);
    }
    else
    {
        convert_value2string(p_ctrl, p_ctrl->p_string, p_ctrl->valbak, p_pattern);
        EDF_PRINTF("Restore value : %d \n", p_ctrl->valbak);
    }
    return TRUE;
}

static void transfer_time_mode(PEDIT_FIELD p_ctrl, UINT32 *dw_value)
{
    UINT32      i       = 0;
    UINT32      j       = 0;
    UINT32      dw_temp  = 1;
    UINT32    __MAYBE_UNUSED__  len     = 0;
    UINT32      cnt     = 0;
    UINT8       timemode = 0;
    char        *pfmt   = NULL;
    EDIT_PATTERN pattern;
    const UINT32 twelve_hour = 12;

    if(NULL == dw_value)
    {
        return ;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    get_pattern(p_ctrl, &pattern);
    if ('T' == pattern.b_tag)
    {
        pfmt = m_fmt_dt[pattern.dt.b_format];
        timemode = (get_time_mode()) ? 1: 0;
        for (i = 0; i < pattern.b_len; i++)
        {
            dw_temp *= 10;
        }
        for (i = 0, len = 0; i < STRLEN(pfmt); i += 2)
        {
            cnt = pfmt[i + 1] - '0';
            for (j = 0; j < cnt; j++)
            {
                dw_temp /= 10;
            }
            if ('h' == pfmt[i])
            {
                if (1 == timemode)
                {
                    if ((dw_temp >0) && (*dw_value / dw_temp) > twelve_hour) //pm
                    {
                        *dw_value = *dw_value - (12 * dw_temp);
                    }
                }
                else
                {
                    if ((dw_temp >0) && (0 == (*dw_value / dw_temp))) //am
                    {
                        *dw_value = *dw_value + (12 * dw_temp);
                    }
                }
                break;
            }
        }
    }
}

BOOL osd_set_edit_field_content(PEDIT_FIELD p_ctrl, UINT32 string_type, UINT32 dw_value)
{
    UINT32  len     = 0;
    UINT8   *p_str   = NULL;
    EDIT_PATTERN pattern;
    UINT32 ret = 0;

    if(NULL == p_ctrl)
    {
        return FALSE;
    }
    switch (string_type)
    {
    case STRING_ID:
        p_str = osd_get_unicode_string(dw_value);
        len = com_uni_str_len_ext(p_str);
        if((len + 1) > (p_ctrl->b_max_len))
            MEMCPY((UINT8 *)p_ctrl->p_string, p_str, p_ctrl->b_max_len*2);
        else
            MEMCPY((UINT8 *)p_ctrl->p_string, p_str, (len + 1) * 2);
        break;
    case STRING_ANSI:
        //com_asc_str2uni((UINT8 *)dw_value, p_ctrl->p_string);
        ret = com_asc_str2uni_ex((UINT8 *)dw_value, p_ctrl->p_string, p_ctrl->b_max_len*2-1);
        if(!ret)
        {
            return FALSE;
        }
        break;
    case STRING_UNICODE:
        //com_uni_str_copy(p_ctrl->p_string, (UINT16 *)dw_value);
        com_uni_str_copy_ex(p_ctrl->p_string, (UINT16 *)dw_value, p_ctrl->b_max_len*2-1);
        break;
    case STRING_NUMBER:
    {
        MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
        get_pattern(p_ctrl, &pattern);
        transfer_time_mode(p_ctrl, &dw_value);
        convert_value2string(p_ctrl, p_ctrl->p_string, dw_value, &pattern);
    }
    break;
    default:
        return FALSE;
    }
    return TRUE;
}

BOOL osd_set_edit_field_prefix(PEDIT_FIELD p_ctrl, UINT32 string_type, UINT32 dw_value)
{
    UINT32  len     = 0;
    UINT8   *p_str   = NULL;
    UINT32 ret = 0;
    
    if(NULL == p_ctrl)
    {
        return FALSE;
    }
    switch (string_type)
    {
        case STRING_ID:
            p_str = osd_get_unicode_string(dw_value);
            len = com_uni_str_len_ext(p_str);

            if((len + 1) > (p_ctrl->b_max_len))
                MEMCPY((UINT8 *)p_ctrl->p_string, p_str, p_ctrl->b_max_len*2);
            else
                MEMCPY((UINT8 *)p_ctrl->p_string, p_str, (len + 1) * 2);
            break;
        case STRING_ANSI:
            //com_asc_str2uni((UINT8 *)dw_value, p_ctrl->p_prefix);
            ret = com_asc_str2uni_ex((UINT8 *)dw_value, p_ctrl->p_prefix, p_ctrl->b_max_len*2-1);
            if(!ret)
            {
                return FALSE;
            }
            break;
        case STRING_UNICODE:
            //com_uni_str_copy(p_ctrl->p_prefix, (UINT16 *)dw_value);
            com_uni_str_copy_ex(p_ctrl->p_prefix, (UINT16 *)dw_value, p_ctrl->b_max_len-1);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

BOOL osd_set_edit_field_suffix(PEDIT_FIELD p_ctrl, UINT32 string_type, UINT32 dw_value)
{
    UINT32  len     = 0;
    UINT8   *p_str   = NULL;
    UINT32 ret = 0;
    
    if(NULL == p_ctrl)
    {
        return FALSE;
    }
    switch (string_type)
    {
        case STRING_ID:
            p_str = osd_get_unicode_string(dw_value);
            len = com_uni_str_len_ext(p_str);

            if((len + 1) > (p_ctrl->b_max_len))
                MEMCPY((UINT8 *)p_ctrl->p_string, p_str, p_ctrl->b_max_len*2);
            else
                MEMCPY((UINT8 *)p_ctrl->p_string, p_str, (len + 1) * 2);
            break;
        case STRING_ANSI:
            //com_asc_str2uni((UINT8 *)dw_value, p_ctrl->p_suffix);
            ret = com_asc_str2uni_ex((UINT8 *)dw_value, p_ctrl->p_suffix, p_ctrl->b_max_len*2-1);
            if(!ret)
            {
                return FALSE;
            }
            break;
        case STRING_UNICODE:
            //com_uni_str_copy(p_ctrl->p_suffix, (UINT16 *)dw_value);
            com_uni_str_copy_ex(p_ctrl->p_suffix, (UINT16 *)dw_value, p_ctrl->b_max_len-1);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

UINT32 osd_get_edit_field_content(PEDIT_FIELD p_ctrl)
{
    EDIT_PATTERN    pattern;

    if(NULL == p_ctrl)
    {
        return 0;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    get_pattern(p_ctrl, &pattern);
    if (('p' == pattern.b_tag) || ('s' == pattern.b_tag) || ('m' == pattern.b_tag))
    {
        return (UINT32)p_ctrl->p_string;
    }
    else if ('i' == pattern.b_tag)
    {
        return convert_ip2int(p_ctrl->p_string);
    }
    else
    {
        return com_uni_str2int(p_ctrl->p_string);
    }
}

BOOL osd_get_edit_field_int_value(PEDIT_FIELD p_ctrl, UINT32 *p_value)
{
    EDIT_PATTERN pattern;

    if(NULL == p_ctrl)
    {
        return FALSE;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    get_pattern(p_ctrl, &pattern);
    if (('r' == pattern.b_tag) || ('f' == pattern.b_tag) || ('t' == pattern.b_tag))
    {
        *p_value = com_uni_str2int(p_ctrl->p_string);
    }
    else if ('i' == pattern.b_tag)
    {
        *p_value = convert_ip2int(p_ctrl->p_string);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

BOOL osd_get_edit_field_time_date(PEDIT_FIELD p_ctrl, date_time *dt)
{
    UINT32          i           = 0;
    UINT32          pos         = 0;
    UINT32          cnt         = 0;
    UINT16          *p_string    = NULL;
    char            *pfmt       = NULL;
    char            f           = 0;
    UINT32          val         = 0;
    const UINT32    m_const_12   = 12;
    EDIT_PATTERN    pattern;

    if((NULL == p_ctrl) || (NULL == dt))
    {
        return FALSE;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    get_pattern(p_ctrl, &pattern);
    if (('t' != pattern.b_tag) && ('T' != pattern.b_tag))
    {
        return FALSE;
    }
    pfmt = m_fmt_dt[pattern.dt.b_format];
    p_string = p_ctrl->p_string;
    for (i = 0; i < STRLEN(pfmt); i += 2)
    {
        f = pfmt[i];
        cnt = (UINT32)(pfmt[i + 1] - '0');
        if (!com_uni_str2int_ext(&p_string[pos], cnt, &val))
        {
            return FALSE;
        }
        switch (f)
        {
            case 'Y':
                dt->year = (UINT16)val;
                break;
            case 'M':
                dt->month = (UINT8)val;
                break;
            case 'D':
                dt->day = (UINT8)val;
                break;
            case 'h':
                if ('T' == pattern.b_tag)
                {
                    if (get_time_mode()) //pm
                    {
                        if (val != m_const_12)
                        {
                            val = val + m_const_12;
                        }
                    }
                    else
                    {
                        if (m_const_12 == val)
                        {
                            val = 0;
                        }
                    }
                }
                dt->hour = (UINT8)val;
                break;
            case 'm':
                dt->min = (UINT8)val;
                break;
            case 's':
                dt->sec = (UINT8)val;
                break;
            default:
                return FALSE;
        }
        pos += cnt + 1;
    }
    return TRUE;
}

void osd_draw_edit_field_cell(PEDIT_FIELD p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw)
{
    LPVSCR          p_vscr       = NULL;
    struct osdrect  *objframe   = NULL;
    PWINSTYLE       lp_win_sty    = NULL;
    UINT32  __MAYBE_UNUSED__        fg_color     = 0;
    UINT8           *p_str       = NULL;
    UINT8           b_cursor     = 0;
    UINT32          draw_type   = 0;
    UINT8           pos         = 0;
    UINT32          cursor_mode  = 0;
    UINT32          shift_chars  = 0;
    UINT16          total_w      = 0;
    UINT16          part_w       = 0;
    UINT16          ch_h         = 0;
    UINT16          ch_w         = 0;
    UINT16          w_max_height  = 0;
    UINT16          w_ypos       = 0;
    UINT16          w_xpos       = 0;
    UINT8           b_assign_type = 0;
    UINT32          h_aligin     = 0;
    UINT32          v_align      = 0;
    EDIT_PATTERN    pattern;
    struct osdrect  r;
    UINT16          ws_text[C_MAX_EDIT_LENGTH * 2];

    if (NULL == p_ctrl)
    {
        return;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    osd_set_rect(&r, 0, 0, 0, 0);
    MEMSET(ws_text, 0x00, sizeof(ws_text));
    b_assign_type = p_ctrl->b_align;
    h_aligin     = GET_HALIGN(b_assign_type);
    v_align      = GET_VALIGN(b_assign_type);
    objframe    = &p_ctrl->head.frame;
    lp_win_sty    = g_osd_rsc_info.osd_get_win_style(b_style_idx);
    fg_color     = lp_win_sty->w_fg_idx;
    p_vscr = osd_draw_object_frame(objframe, b_style_idx);
    get_pattern(p_ctrl, &pattern);
    draw_type = osd_get_draw_type(n_cmd_draw);
    if (C_DRAW_TYPE_HIGHLIGHT == draw_type)
    {
        cursor_mode = get_focus_cursor_type(p_ctrl, &pattern);
    }
    else
    {
        cursor_mode = CURSOR_NO;
    }
    b_cursor = p_ctrl->b_cursor + 1;
    ws_text[0] = 0;
    if (p_ctrl->p_prefix != NULL)
    {
        //com_uni_str_copy(ws_text, p_ctrl->p_prefix);
        com_uni_str_copy_ex(ws_text, p_ctrl->p_prefix, C_MAX_EDIT_LENGTH * 2-1);
        pos = com_uni_str_len(ws_text);
    }
    get_edit_text(p_ctrl, &ws_text[pos], ARRAY_SIZE(ws_text) - pos, &pattern);
    if (CURSOR_SPECIAL == cursor_mode)
    {
        pos = com_uni_str_len(ws_text);
        set_uni_str_char_at(ws_text, '_', pos);
        ws_text[pos + 1] = 0;
    }
    if (p_ctrl->p_suffix != NULL)
    {
        pos = com_uni_str_len(ws_text);
        //com_uni_str_copy(&ws_text[pos], p_ctrl->p_suffix);
        com_uni_str_copy_ex(&ws_text[pos], p_ctrl->p_suffix, C_MAX_EDIT_LENGTH * 2-1-pos);
    }
    p_str = (UINT8 *)ws_text;
    //draw string and cusor here
    r = *objframe;
    r.u_left += p_ctrl->b_x;
    r.u_top  += p_ctrl->b_y;
    r.u_width -= p_ctrl->b_x << 1;
    r.u_height -= p_ctrl->b_y << 1;
    /* Make 's' cursor show */
    if ((('s' == pattern.b_tag) || ('m' == pattern.b_tag)) && (CURSOR_NORMAL == cursor_mode))
    {
        total_w = osd_multi_font_lib_str_max_hw(p_str, p_ctrl->head.b_font, &w_max_height, &ch_w, b_cursor);
        while ((r.u_width < total_w) && (shift_chars < p_ctrl->b_cursor))
        {
            shift_chars++;
            total_w = osd_multi_font_lib_str_max_hw(p_str + shift_chars * 2,
                                              p_ctrl->head.b_font, &w_max_height, &ch_w, b_cursor - shift_chars);
        }
    }
    p_str = (UINT8 *)(ws_text + shift_chars);
    osd_draw_text(&r, p_str, lp_win_sty->w_fg_idx, p_ctrl->b_align,p_ctrl->head.b_font, p_vscr);
    if ((CURSOR_NORMAL == cursor_mode) && (p_str != NULL))
    {
        total_w = osd_multi_font_lib_str_max_hw(p_str, p_ctrl->head.b_font, &w_max_height, &ch_w, 0);
        pos = (p_ctrl->p_prefix != NULL) ? com_uni_str_len(p_ctrl->p_prefix) : 0;
        part_w = osd_multi_font_lib_str_max_hw(p_str, p_ctrl->head.b_font, &ch_h, &ch_w, b_cursor + pos - shift_chars);
        w_ypos = r.u_top;
        w_xpos = r.u_left;
        if (r.u_height > w_max_height)
        {
            if (C_ALIGN_VCENTER == v_align)
            {
                w_ypos += (r.u_height - w_max_height) >> 1;
            }
            else if (C_ALIGN_BOTTOM == v_align)
            {
                w_ypos += r.u_height - w_max_height;
            }
        }
        //  Adjust X Coordinate
        //if(!(bAssignType&C_ALIGN_LEFT))
        {
            if (C_ALIGN_CENTER == h_aligin)
            {
#ifdef BIDIRECTIONAL_OSD_STYLE
                if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
                {
                    w_xpos -= (r.u_width - total_w) >> 1;
                }
                else
                {
                    if (total_w < r.u_width)
                    {
                        w_xpos += (r.u_width - total_w) >> 1;
                    }
                }

#else
                if (total_w < r.u_width)
                {
                    w_xpos += (r.u_width - total_w) >> 1;
                }
#endif
            }
            else if (C_ALIGN_RIGHT == h_aligin)
            {
                if (total_w < r.u_width)
                {
                    w_xpos += r.u_width - total_w - 4;
                }
            }
        }
#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        {
            w_xpos += (r.u_width > total_w) ? (r.u_width - total_w) : 0; //-4;
        }
#endif
        if (0 == com_uni_str_len((UINT16 *)p_str))
        {
            ch_w = 10;
            ch_h = 12;
        }
        else if ((UINT8)(b_cursor - shift_chars) > com_uni_str_len((UINT16 *)p_str))
        {
            w_xpos += part_w;
            ch_w = 10;
        }
        else
        {
            w_xpos += part_w - ch_w;
        }
        w_ypos += ch_h - 2;

#ifdef BIDIRECTIONAL_OSD_STYLE
        if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
        {
            w_xpos = r.u_left + (r.u_width - (w_xpos - r.u_left) - ch_w);
        }
#endif
        osd_draw_hor_line(w_xpos, w_ypos, ch_w, lp_win_sty->w_fg_idx, p_vscr);
        osd_draw_hor_line(w_xpos, w_ypos + 1, ch_w, lp_win_sty->w_fg_idx, p_vscr);
    }
}

/*SELECT_EDIT_MODE map*/
static VACTION osd_edit_field_key_map1(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;
    PEDIT_FIELD p_ctrl = (PEDIT_FIELD)p_obj;
    UINT8 b_style = p_ctrl->b_style;

    if (b_style & EDIT_STATUS)
    {
        switch (key)
        {
            case V_KEY_UP:
                action = VACT_EDIT_INCREASE_;
                break;
            case V_KEY_DOWN:
                action = VACT_EDIT_DECREASE_;
                break;
            case V_KEY_LEFT:
                action = VACT_EDIT_LEFT;
                break;
            case V_KEY_RIGHT:
                action = VACT_EDIT_RIGHT;
                break;
            case V_KEY_0:
            case V_KEY_1:
            case V_KEY_2:
            case V_KEY_3:
            case V_KEY_4:
            case V_KEY_5:
            case V_KEY_6:
            case V_KEY_7:
            case V_KEY_8:
            case V_KEY_9:
                action = key - V_KEY_0 + VACT_NUM_0;
                break;
            case V_KEY_ENTER:
                action = VACT_EDIT_ENTER;
                break;
            case V_KEY_MENU:
            case V_KEY_EXIT:
                action = VACT_EDIT_CANCEL_EXIT;
                break;
/*          case V_KEY_CAPS:
                Action = VACT_CAPS;
                break;*/
            default:
                break;
        }
    }
    else // SELECT_STATUS
    {
        switch (key)
        {
            case V_KEY_ENTER:
                //Action = VACT_INPUT;
                break;
            case V_KEY_LEFT:
                action = VACT_EDIT_LEFT;
                break;
            case V_KEY_RIGHT:
                action = VACT_EDIT_RIGHT;
                break;
            default:
                break;
        }
    }
    return action;
}

/* NORMAL_EDIT_MODE map*/
static VACTION osd_edit_field_key_map2(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;
    PEDIT_FIELD p_ctrl = (PEDIT_FIELD)p_obj;
    UINT8 b_style = p_ctrl->b_style;

    if (b_style & EDIT_STATUS)
    {
        switch (key)
        {
            case V_KEY_0:
            case V_KEY_1:
            case V_KEY_2:
            case V_KEY_3:
            case V_KEY_4:
            case V_KEY_5:
            case V_KEY_6:
            case V_KEY_7:
            case V_KEY_8:
            case V_KEY_9:
                action = key - V_KEY_0 + VACT_NUM_0;
                break;
            case V_KEY_ENTER:
                action = VACT_ENTER;
                break;
            case V_KEY_MENU:
            case V_KEY_EXIT:
                action = VACT_EDIT_CANCEL_EXIT;
                break;
            case V_KEY_LEFT:
                action = VACT_EDIT_LEFT;
                break;
/*
            case V_KEY_CAPS:
                Action = VACT_CAPS;
                break;
*/
            default:
                break;
        }
    }
    else // SELECT_STATUS
    {
        switch (key)
        {
            case V_KEY_LEFT:
                action = VACT_EDIT_LEFT;
                break;
            case V_KEY_RIGHT:
                action = VACT_EDIT_RIGHT;
                break;
            case V_KEY_0:
            case V_KEY_1:
            case V_KEY_2:
            case V_KEY_3:
            case V_KEY_4:
            case V_KEY_5:
            case V_KEY_6:
            case V_KEY_7:
            case V_KEY_8:
            case V_KEY_9:
                action = key - V_KEY_0 + VACT_NUM_0;
                break;
            default:
                break;
        }
    }
    return action;
}


VACTION osd_edit_field_key_map(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;
    PEDIT_FIELD p_ctrl = (PEDIT_FIELD)p_obj;
    UINT8 b_style = p_ctrl->b_style;

    if (SELECT_EDIT_MODE == (b_style & SELECT_EDIT_MODE))
    {
        action = osd_edit_field_key_map1(p_obj, key);
    }
    else // if(bStyle&NORMAL_EDIT_MODE)
    {
        action = osd_edit_field_key_map2(p_obj, key);
    }

    return action;
}
static PRESULT osd_edit_field_num_internal_proc(POBJECT_HEAD p_obj, PEDIT_PATTERN p_pattern,
                                            UINT32 status, UINT32 action, UINT32 v_key)
{
    PRESULT         result      = PROC_PASS;
    PEDIT_FIELD     p_ctrl       = NULL;
    INT8            num         = 0;

    if ((NULL == p_obj) || (NULL == p_pattern))
    {
        result = PROC_LOOP;
        return result;
    }
    p_ctrl = (PEDIT_FIELD)p_obj;
    if (!check_focus_can_input09(p_ctrl, p_pattern))
    {
        EDF_PRINTF("Can't int put 0~9\n");
        result = PROC_PASS;
        return result;
    }
    if (status != EDIT_STATUS)
    {
        EDF_PRINTF(" 0 ~ 9: Change from SELECT_STATUS->EDIT_STATUS\n");
        p_ctrl->b_style |= EDIT_STATUS;

        if ('r' == p_pattern->b_tag)
        {
            if (p_ctrl->b_cursor_mode != CURSOR_NORMAL)
            {
                p_ctrl->b_cursor = 0;
            }
        }
        backup_restore_value(p_ctrl, p_pattern, TRUE);
    }
    check_cursor_pos(p_ctrl, p_pattern);
    if ((VACT_EDIT_PASSWORD == action) && ('p' == p_pattern->b_tag))
    {
        num = v_key - V_KEY_0;
    }
    //10~ store vkey to make more key can be password except 0~9
    else
    {
        num = action - VACT_NUM_0;    //0~9
    }
    result = change_edit_value(p_ctrl, num, p_pattern, TRUE);
    return result;
}

static PRESULT osd_edit_field_left_right_internal_proc(POBJECT_HEAD p_obj, PEDIT_PATTERN p_pattern,
                                                  UINT32 status, UINT32 action)
{
    PRESULT         result      = PROC_PASS;
    PEDIT_FIELD     p_ctrl       = NULL;
    INT8            num         = 0;
    INT8            step        = 0;

    if ((NULL == p_obj) || (NULL == p_pattern))
    {
        result = PROC_LOOP;
        return result;
    }
    p_ctrl = (PEDIT_FIELD)p_obj;
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == g_osd_rsc_info.osd_get_mirror_flag())
    {
        if (VACT_EDIT_LEFT == action)
        {
            action = VACT_EDIT_RIGHT;
        }
        else if (VACT_EDIT_RIGHT == action)
        {
            action = VACT_EDIT_LEFT;
        }
    }
#endif
    if (CURSOR_NORMAL == get_focus_cursor_type(p_ctrl, p_pattern))
    {
        /* Moving cursor */
        if (VACT_EDIT_LEFT == action)
        {
            step = -1;
        }
        else
        {
            step = 1;
        }
        check_cursor_pos(p_ctrl, p_pattern);
        move_edit_cursor(p_ctrl, step, p_pattern);
    }
    else if (check_focus_can_addsub(p_ctrl, p_pattern))
    {
        /* Increase / Decrease */
        if (VACT_EDIT_LEFT == action)
        {
            num = -1;
        }
        else
        {
            num = 1;
        }

        step_edit_value(p_ctrl, num, p_pattern);
    }
    else if (('r' == p_pattern->b_tag) && (CURSOR_SPECIAL == p_ctrl->b_cursor_mode) && (EDIT_STATUS == status))
    {
        if (VACT_EDIT_LEFT == action)// Delete last char for number input
        {
            devide_edit_value(p_ctrl, p_pattern);
        }
        else  // Restore to previous value( exit current input)
        {
            backup_restore_value(p_ctrl, p_pattern, FALSE);
            p_ctrl->b_style &= ~EDIT_STATUS;
            osd_track_object(p_obj, C_UPDATE_CONTENT);
        }
    }
    else
    {
        result = PROC_PASS;
    }
    return result;
}

PRESULT osd_edit_field_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
    PRESULT         result      = PROC_LOOP;
    PEDIT_FIELD     p_ctrl       = NULL;
    INT8            num         = 0;
    UINT32          dw_value     = 0;
    UINT32          evn_param    = 0;
    UINT32          mode        = 0;
    UINT32          status      = 0;
    BOOL            exit_flag   = FALSE;
    VACTION         action      = VACT_PASS;
    UINT32          vkey        = V_KEY_NULL;
    BOOL            b_continue   = FALSE;
    EDIT_PATTERN    pattern;
    UINT16          ws_text[C_MAX_EDIT_LENGTH] = {0};

    if (NULL == p_obj)
    {
        return result;
    }
    MEMSET(&pattern, 0x00, sizeof(EDIT_PATTERN));
    p_ctrl = (PEDIT_FIELD)p_obj;
    mode   = osd_get_edit_field_mode(p_ctrl);
    status = osd_get_edit_field_status(p_ctrl);
    if (MSG_TYPE_KEY == msg_type)
    {
        key_input_changed = TRUE;
        result = osd_obj_common_proc(p_obj, msg, param1, &vkey, &action, &b_continue);
        key_input_changed = FALSE;
        if (!b_continue)
        {
            goto CHECK_LEAVE;
        }
        result = PROC_LOOP;
        get_pattern(p_ctrl, &pattern);
        switch (action)
        {
            case VACT_EDIT_ENTER:   /* Enter Edit mode */
                if ((SELECT_EDIT_MODE == mode) && (SELECT_STATUS == status))
                {
                    p_ctrl->b_style |= EDIT_STATUS;
                    EDF_PRINTF("VACT_INPUT : Change from SELECT_STATUS->EDIT_STATUS\n");
                    p_ctrl->b_cursor = 0;
                    /* Only 'r', 'd' and 't' can backup current value.*/
                    backup_restore_value(p_ctrl, &pattern, TRUE);
                    osd_track_object(p_obj, C_UPDATE_CONTENT);
                }
                else
                {
                    result = PROC_PASS;
                }
                break;
            case VACT_EDIT_CANCEL_EXIT: /* Exit Edit mode without save */
                if ((EDIT_STATUS == status) && ((SELECT_EDIT_MODE == mode)
                    || (('r' == pattern.b_tag) && (CURSOR_SPECIAL == p_ctrl->b_cursor_mode))))

                {
                    EDF_PRINTF("VACT_CLEAR: Change from EDIT_STATUS->SELECT_STATUS\n");
                    p_ctrl->b_style &= ~EDIT_STATUS;
                    if (backup_restore_value(p_ctrl, &pattern, FALSE))
                    {
                        MEMSET(ws_text, 0x00, sizeof(ws_text));
                        //com_uni_str_copy(ws_text, p_ctrl->p_string);
                        com_uni_str_copy_ex(ws_text, p_ctrl->p_string, C_MAX_EDIT_LENGTH-1);
                        notify_change_event(p_ctrl, ws_text, &pattern);
                    }
                    osd_track_object(p_obj, C_UPDATE_CONTENT);
                }
                else
                {
                    result = PROC_PASS;
                }
                break;
            case VACT_EDIT_SAVE_EXIT:   // Exit Edit mode with save
                if (((EDIT_STATUS == status) && ((SELECT_EDIT_MODE == mode) || ('r' == pattern.b_tag)))
                    || (NORMAL_EDIT_MODE == mode) || ('r' == pattern.b_tag))
                {
                    EDF_PRINTF("VACT_ENTER : Change from EDIT_STATUS->SELECT_STATUS\n");
                    if (!get_value(p_ctrl, &dw_value))
                    {
                        evn_param = (UINT32)p_ctrl->p_string;
                    }
                    else
                    {
                        evn_param = dw_value;
                    }
                    if (PROC_PASS == OSD_SIGNAL(p_obj, EVN_ENTER, evn_param, 0))
                    {
                        p_ctrl->b_style &= ~EDIT_STATUS;
                        //hb 20081204, edit might reset value after
                        // OSD_SIGNAL call back
                        get_value(p_ctrl, &dw_value);
                        if (!is_edit_string(pattern.b_tag))
                        {
                            convert_value2string(p_ctrl, p_ctrl->p_string, dw_value, &pattern);
                        }
                        osd_track_object(p_obj, C_UPDATE_CONTENT);
                    }
                }
                else
                {
                    result = PROC_PASS;
                }
                break;
            case VACT_EDIT_LEFT:
            case VACT_EDIT_RIGHT:
                result = osd_edit_field_left_right_internal_proc(p_obj, &pattern, status, action);
                break;
            case VACT_EDIT_INCREASE_:
            case VACT_EDIT_DECREASE_:  // add or minus the current cursor digit
                if (get_focus_cursor_type(p_ctrl, &pattern) != CURSOR_NORMAL)
                {
                    result = PROC_PASS;
                    break;
                }
                if (mode != EDIT_STATUS)
                {
                    EDF_PRINTF(" VACT_EDIT_INCREASE_/VACT_EDIT_DECREASE_: ");
                    EDF_PRINTF("Change from SELECT_STATUS->EDIT_STATUS\n");
                    p_ctrl->b_style |= EDIT_STATUS;
                    backup_restore_value(p_ctrl, &pattern, TRUE);
                }

                check_cursor_pos(p_ctrl, &pattern);
                num = get_uni_str_char_at(p_ctrl->p_string, p_ctrl->b_cursor);
                if (0 == num)
                {
                    EDF_PRINTF("GetUniStrCharAt() failed!\n");
                }
                num -= '0';
                if (VACT_EDIT_INCREASE_ == action)
                {
                    num = (num + 1) % 10;
                }
                else
                {
                    num = (num - 1 + 10) % 10;
                }
                result = change_edit_value(p_ctrl, num, &pattern, FALSE);
                break;
            case VACT_NUM_0:
            case VACT_NUM_1:
            case VACT_NUM_2:
            case VACT_NUM_3:
            case VACT_NUM_4:
            case VACT_NUM_5:
            case VACT_NUM_6:
            case VACT_NUM_7:
            case VACT_NUM_8:
            case VACT_NUM_9:
            case VACT_EDIT_PASSWORD:
                result = osd_edit_field_num_internal_proc(p_obj, &pattern, status, action, vkey);
                break;
            case VACT_CLOSE:
            CLOSE_OBJECT:
                result = osd_obj_close(p_obj, C_CLOSE_CLRBACK_FLG);
                exit_flag = TRUE;
                break;
            default:
                result = OSD_SIGNAL(p_obj, EVN_UNKNOWN_ACTION, (action << 16) | vkey, param1);
                if ((result != PROC_PASS) && (result != PROC_LOOP) && (result != PROC_LEAVE))
                {
                    return result;
                }
                break;
        }
        if (exit_flag)
        {
            goto EXIT;
        }
    }
    else// if(MSG_TYPE_EVNT ==msg_type)
    {
        result = OSD_SIGNAL(p_obj, msg_type, msg, param1);
        if ((PROC_PASS == result) && ((EVN_FOCUS_PRE_LOSE == msg_type)
                                      || (EVN_PARENT_FOCUS_PRE_LOSE == msg_type)))
        {
            /* Clear EDIT_STATUS flag when losing focus. */
            if (EDIT_STATUS == status)
            {
                p_ctrl->b_style &= ~EDIT_STATUS;
            }

            get_pattern(p_ctrl, &pattern);
            check_number_value(p_ctrl, &pattern);
        }
    }
CHECK_LEAVE:
    if (!(EVN_PRE_OPEN == msg_type))
    {
        CHECK_LEAVE_RETURN(result, p_obj);
    }
EXIT:
    return result;
}

