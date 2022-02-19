/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mobile_input.c
*
*    Description: The file is used for software keyboard by remote
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>

#include "mobile_input.h"

#define MOBILE_ACT_CNT   (sizeof(act_chars)/sizeof(act_chars[0]))

UINT32 mobile_key_caps_flag = 0;/* 0-lower case 1-up case */

static UINT32 mobile_key_press_cnt = 0;
static UINT8  mobile_pre_act = 0;


static act_map_chars_t act_chars[] =
{
    {VACT_NUM_0,   ACT_TYPE_CHAR,  4,  " _-0"},
    {VACT_NUM_1,   ACT_TYPE_CHAR,  9,  "#.,*?!/@1"},
    {VACT_NUM_2,   ACT_TYPE_CHAR,  4,  "ABC2"},
    {VACT_NUM_3,   ACT_TYPE_CHAR,  4,  "DEF3"},
    {VACT_NUM_4,   ACT_TYPE_CHAR,  4,  "GHI4"},
    {VACT_NUM_5,   ACT_TYPE_CHAR,  4,  "JKL5"},
    {VACT_NUM_6,   ACT_TYPE_CHAR,  4,  "MNO6"},
    {VACT_NUM_7,   ACT_TYPE_CHAR,  5,  "PQRS7"},
    {VACT_NUM_8,   ACT_TYPE_CHAR,  4,  "TUV8"},
    {VACT_NUM_9,   ACT_TYPE_CHAR,  5,  "WXYZ9"},
    {VACT_EDIT_LEFT,    ACT_TYPE_LEFT,  0,  NULL},
    {VACT_EDIT_RIGHT,   ACT_TYPE_RIGHT, 0,  NULL},

    {VACT_CAPS,     ACT_TYPE_CAPS,  0,  NULL},
    {VACT_DEL,      ACT_TYPE_DEL,  0,  NULL},
};

//mobile input type
mobile_input_type_t mobile_input_type =
{
    MOBILE_INPUT_NORMAL,
    MOBILE_CAPS_INIT_LOW,
    17,
    0,
    NULL
};
void mobile_input_init(EDIT_FIELD *edf,mobile_input_type_t *type)
{
    mobile_key_press_cnt = 0;
    mobile_pre_act  = VACT_PASS;

    if(NULL == edf)
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    if(NULL != type)
    {
        mobile_input_type.type       = type->type;
        mobile_input_type.caps_flag      = type->caps_flag;
        mobile_input_type.maxlen         = type->maxlen;
        mobile_input_type.fixlen_flag = type->fixlen_flag;
        mobile_input_type.callback   = type->callback;
    }
    else
    {
        mobile_input_type.type       = MOBILE_INPUT_NORMAL;
        mobile_input_type.caps_flag      = MOBILE_CAPS_INIT_LOW;
        mobile_input_type.maxlen         = 17;
        mobile_input_type.fixlen_flag = 0;
        mobile_input_type.callback   = NULL;

    }

    if((MOBILE_CAPS_INIT_LOW == mobile_input_type.caps_flag)
        || (MOBILE_CAPS_ALWAYS_LOW == mobile_input_type.caps_flag))
    {
        mobile_key_caps_flag = 0;
    }
    else
    {
        mobile_key_caps_flag = 1;
    }

    edf->b_cursor = 0;
    edf->b_max_len = mobile_input_type.maxlen;
}


PRESULT mobile_input_proc(EDIT_FIELD *edf, VACTION vact, 
    UINT32 __MAYBE_UNUSED__ key, 
    UINT32 __MAYBE_UNUSED__ para)
{
    PRESULT ret = PROC_LOOP;
    UINT32 i = 0;
    UINT32 cur_max_pos = 0;
    ACT_TYPE act_type = ACT_TYPE_CHAR;
    act_map_chars_t *pact_map = NULL;
    char    ch = 0;
    UINT8   bstyle = 0;

    if(NULL == edf)
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return ret;
    }

    bstyle = edf->b_style;
    if((!(bstyle&EDIT_STATUS)) && (bstyle&SELECT_EDIT_MODE))
    {
        return PROC_PASS;
    }

    cur_max_pos = com_uni_str_len(edf->p_string);
    if(cur_max_pos >=  edf->b_max_len)
    {
        cur_max_pos = edf->b_max_len - 1;
    }
    if(edf->b_cursor > cur_max_pos)
    {
        edf->b_cursor = cur_max_pos;
    }
    for(i = 0;i<MOBILE_ACT_CNT;i++)
    {
        pact_map = &act_chars[i];
        if(pact_map->act == vact)
        {
            break;
        }
    }

    if(MOBILE_ACT_CNT == i)
    {
        return PROC_PASS;
    }

    act_type = pact_map->acttype;

    switch(act_type)
    {
    case ACT_TYPE_CHAR:
        if((VACT_PASS != mobile_pre_act)   && (vact != mobile_pre_act ))
        {
            mobile_key_press_cnt = 0;
            if (edf->b_cursor < cur_max_pos )
            {
                edf->b_cursor++;
            }
        }
        mobile_pre_act = vact;

        mobile_key_press_cnt %= pact_map->map_char_count;
        ch = pact_map->map_chars[mobile_key_press_cnt];

        if(MOBILE_INPUT_NORMAL == mobile_input_type.type)
        {

            mobile_key_press_cnt++;
            mobile_key_press_cnt %= pact_map->map_char_count;
        }
        else // MOBILE_INPUT_HEX
        {
            if( ((ch>='0') && (ch<='9')) || ((ch>='a') && (ch<='f')) || ((ch>='A') && (ch<='F')) )
            {
                mobile_key_press_cnt++;
                mobile_key_press_cnt %= pact_map->map_char_count;
            }
            else
            {
                for(i=0;i<pact_map->map_char_count;i++)
                {
                    ch = pact_map->map_chars[ (i + mobile_key_press_cnt) % pact_map->map_char_count];
                    if( ((ch>='0') && (ch<='9')) || ((ch>='a') && (ch<='f')) || ((ch>='A') && (ch<='F')) )
                    {
                        mobile_key_press_cnt++;
                        mobile_key_press_cnt %= pact_map->map_char_count;
                        break;
                    }
                }

                if(i == pact_map->map_char_count)
                    {
                    return PROC_PASS;
                    }
            }

        }

        if(0 == mobile_key_caps_flag )   /* Lower Case*/
        {
            if((ch>='A') && (ch<='Z'))
            {
                ch = ch - 'A' + 'a';
            }
        }
        else if(1 == mobile_key_caps_flag )  /* Up Case*/
        {
            if((ch>='a') && (ch<='z'))
            {
                ch = ch - 'a' + 'A';
            }
        }
        set_uni_str_char_at(edf->p_string,ch,edf->b_cursor);
        if(edf->b_cursor == cur_max_pos)
            {
            edf->p_string[edf->b_cursor + 1] = 0;
            }
        if(NULL != mobile_input_type.callback)
            {
            mobile_input_type.callback((UINT8*)(edf->p_string));
            }
        break;
    case ACT_TYPE_LEFT:
        if(edf->b_cursor > 0)
            {
            edf->b_cursor--;
            }
        else
            {
            edf->b_cursor = cur_max_pos;
            }

        mobile_key_press_cnt = 0;
        mobile_pre_act = VACT_PASS;
        break;
    case ACT_TYPE_RIGHT:
        if(edf->b_cursor < cur_max_pos)
            {
            edf->b_cursor++;
            }
        else
            {
            edf->b_cursor = 0;
            }
        mobile_key_press_cnt = 0;
        mobile_pre_act = VACT_PASS;
        break;
    case ACT_TYPE_CAPS:
        if((MOBILE_CAPS_ALWAYS_LOW != mobile_input_type.caps_flag) \
            && (MOBILE_CAPS_ALWAYS_UP !=mobile_input_type.caps_flag))
            {
            mobile_key_caps_flag = (0 == mobile_key_caps_flag)? 1 : 0;
            }
        break;
    case ACT_TYPE_DEL:
        if(cur_max_pos > 0)
        {
            if( !mobile_input_type.fixlen_flag)
            {
                for(i=edf->b_cursor;i<cur_max_pos;i++)
                    {
                    edf->p_string[i] = edf->p_string[i+1];
                    }
                edf->p_string[cur_max_pos] = 0;
                mobile_key_press_cnt = 0;
                mobile_pre_act = VACT_PASS;
                if(NULL != mobile_input_type.callback)
                    {
                    mobile_input_type.callback((UINT8*)(edf->p_string));
                    }
            }
        }
        break;
    default:
        break;
    }

    osd_track_object((POBJECT_HEAD)edf, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

//end:
    return ret;
}

UINT8  mobile_input_get_caps(void)
{
    return mobile_key_caps_flag ;
}

void mobile_input_set_caps(UINT32 flag)
{
    mobile_key_caps_flag=flag;
}

