/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: arabic_convert.c
 *
 *    Description: This source file for process arabic/hebrew/persian charactor
      to unicode.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include "char_map.h"
#include "iso_5937.h"
#define MAX_CHARS       1024 * 4
#define MAX_WORDS       1024
#define BIDI_NULL       0
#define BIDI_LTR        1
#define BIDI_RTL        2
#define BIDI_SPC        3
#define JOIN_NONE       0
#define JOIN_RIGT       1
#define JOIN_LEFT       2
#define JOIN_DUAL       3
#define ARABIC_OTHER    0
#define ARABIC_ALEF     1
#define ARABIC_LAM      2
#define ARABIC_WAW      ARABIC_OTHER
#define ARABIC_YEH      ARABIC_OTHER
#define ARABIC_BEH      ARABIC_OTHER
#define ARABIC_TEH      ARABIC_OTHER
#define ARABIC_HAH      ARABIC_OTHER
#define ARABIC_DAL      ARABIC_OTHER
#define ARABIC_REH      ARABIC_OTHER
#define ARABIC_SEEN     ARABIC_OTHER
#define ARABIC_SAD      ARABIC_OTHER
#define ARABIC_TAH      ARABIC_OTHER
#define ARABIC_AIN      ARABIC_OTHER
#define ARABIC_FEH      ARABIC_OTHER
#define ARABIC_QAF      ARABIC_OTHER
#define ARABIC_KAF      ARABIC_OTHER
#define ARABIC_MEEM     ARABIC_OTHER
#define ARABIC_NOON     ARABIC_OTHER
#define ARABIC_HEH      ARABIC_OTHER
#define ARABIC_NOJOIN   ARABIC_OTHER

/////////FOR Persian
#define ARABIC_PEH      ARABIC_OTHER
#define ARABIC_SEH      ARABIC_OTHER
#define ARABIC_JEEM     ARABIC_OTHER
#define ARABIC_CHEH     ARABIC_OTHER
#define ARABIC_KHEH     ARABIC_OTHER
#define ARABIC_ZAL      ARABIC_OTHER
#define ARABIC_ZEH      ARABIC_OTHER
#define ARABIC_JHEH     ARABIC_OTHER
#define ARABIC_SHEEN    ARABIC_OTHER
#define ARABIC_ZAD      ARABIC_OTHER
#define ARABIC_ZAH      ARABIC_OTHER
#define ARABIC_GHAIN    ARABIC_OTHER
#define ARABIC_GHAF     ARABIC_OTHER
#define ARABIC_GAF      ARABIC_OTHER
#define ARABIC_VAV      ARABIC_OTHER

/////////////////////////
#define ARABIC_CHAR     0
#define PERSIAN_CHAR    1
#define MAX_INFO        (sizeof(arabic_code_info) / sizeof(arabic_code_info[0]))
#define MAX_PER_INFO    (sizeof(persian_code_info) / sizeof(persian_code_info[0]))
static UINT8    ara_per_switch = 0;
struct arabic_join
{
    unsigned short  unicode;
    unsigned short  present;
    unsigned char   join;
    unsigned char   group;
};

static unsigned char    group_last = 0;
static unsigned int segments = 0;
static unsigned short   *str_out = NULL;

//////////////////FOR ARABIC///////////////////
static const struct arabic_join arabic_code_info[] =
{
    { 0x0621, 0xFE80, JOIN_NONE, ARABIC_NOJOIN },
    { 0x0622, 0xFE81, JOIN_RIGT, ARABIC_ALEF },
    { 0x0623, 0xFE83, JOIN_RIGT, ARABIC_ALEF },
    { 0x0624, 0xFE85, JOIN_RIGT, ARABIC_WAW },
    { 0x0625, 0xFE87, JOIN_RIGT, ARABIC_ALEF },
    { 0x0626, 0xFE89, JOIN_DUAL, ARABIC_YEH },
    { 0x0627, 0xFE8D, JOIN_RIGT, ARABIC_ALEF },
    { 0x0628, 0xFE8F, JOIN_DUAL, ARABIC_BEH },
    { 0x0629, 0xFE93, JOIN_RIGT, ARABIC_TEH },
    { 0x062A, 0xFE95, JOIN_DUAL, ARABIC_BEH },
    { 0x062B, 0xFE99, JOIN_DUAL, ARABIC_BEH },
    { 0x062C, 0xFE9D, JOIN_DUAL, ARABIC_HAH },
    { 0x062D, 0xFEA1, JOIN_DUAL, ARABIC_HAH },
    { 0x062E, 0xFEA5, JOIN_DUAL, ARABIC_HAH },
    { 0x062F, 0xFEA9, JOIN_RIGT, ARABIC_DAL },
    { 0x0630, 0xFEAB, JOIN_RIGT, ARABIC_DAL },
    { 0x0631, 0xFEAD, JOIN_RIGT, ARABIC_REH },
    { 0x0632, 0xFEAF, JOIN_RIGT, ARABIC_REH },
    { 0x0633, 0xFEB1, JOIN_DUAL, ARABIC_SEEN },
    { 0x0634, 0xFEB5, JOIN_DUAL, ARABIC_SEEN },
    { 0x0635, 0xFEB9, JOIN_DUAL, ARABIC_SAD },
    { 0x0636, 0xFEBD, JOIN_DUAL, ARABIC_SAD },
    { 0x0637, 0xFEC1, JOIN_DUAL, ARABIC_TAH },
    { 0x0638, 0xFEC5, JOIN_DUAL, ARABIC_TAH },
    { 0x0639, 0xFEC9, JOIN_DUAL, ARABIC_AIN },
    { 0x063A, 0xFECD, JOIN_DUAL, ARABIC_AIN },
    { 0x0640, 0x0640, JOIN_DUAL, ARABIC_NOJOIN },
    { 0x0641, 0xFED1, JOIN_DUAL, ARABIC_FEH },
    { 0x0642, 0xFED5, JOIN_DUAL, ARABIC_QAF },
    { 0x0643, 0xFED9, JOIN_DUAL, ARABIC_KAF },
    { 0x0644, 0xFEDD, JOIN_DUAL, ARABIC_LAM },
    { 0x0645, 0xFEE1, JOIN_DUAL, ARABIC_MEEM },
    { 0x0646, 0xFEE5, JOIN_DUAL, ARABIC_NOON },
    { 0x0647, 0xFEE9, JOIN_DUAL, ARABIC_HEH },
    { 0x0648, 0xFEED, JOIN_RIGT, ARABIC_WAW },
    { 0x0649, 0xFEEF, JOIN_DUAL, ARABIC_YEH },
    { 0x064A, 0xFEF1, JOIN_DUAL, ARABIC_YEH },
    { 0x067E, 0xFB56, JOIN_DUAL, ARABIC_BEH },
    { 0x0686, 0xFB7A, JOIN_DUAL, ARABIC_HAH },
    { 0x06A9, 0xFB8E, JOIN_DUAL, ARABIC_GAF },
    { 0x06AF, 0xFB92, JOIN_DUAL, ARABIC_GAF },
    { 0x06CA, 0xFED5, JOIN_DUAL, ARABIC_OTHER },
    { 0x06CC, 0xFBFC, JOIN_DUAL, ARABIC_YEH },
    { 0x200C, 0x200C, JOIN_NONE, ARABIC_NOJOIN },
    { 0x200D, 0x200D, JOIN_DUAL, ARABIC_NOJOIN },
};

//////////////////FOR PERSIAN///////////////////
static const struct arabic_join persian_code_info[] =
{	
    	{ 0x0621, 0xFE80, JOIN_NONE, ARABIC_NOJOIN },
	{ 0x0622, 0xFE81, JOIN_RIGT, ARABIC_ALEF}, 	
	{ 0x0623, 0xFE83, JOIN_RIGT, ARABIC_ALEF}, 	
	{ 0x0624, 0xFE85, JOIN_RIGT, ARABIC_WAW}, 
	{ 0x0625, 0xFE87, JOIN_RIGT, ARABIC_ALEF}, 	
	{ 0x0626, 0xFE89, JOIN_DUAL, ARABIC_YEH}, 
	
	{ 0x0627, 0xFE8D, JOIN_RIGT, ARABIC_ALEF}, ///???
//	{ 0x0622, 0xFE8D, JOIN_RIGT, ARABIC_ALEF}, ///???
////////////////////////////////////////////////////	
	{ 0x0628, 0xFE8F, JOIN_DUAL, ARABIC_BEH},
	{ 0x0629, 0xFE93, JOIN_RIGT, ARABIC_TEH },
	{ 0x062A, 0xFE95, JOIN_DUAL, ARABIC_TEH},
	{ 0x062B, 0xFE99, JOIN_DUAL, ARABIC_SEH},
	{ 0x062C, 0xFE9D, JOIN_DUAL, ARABIC_JEEM},
	{ 0x062D, 0xFEA1, JOIN_DUAL, ARABIC_HEH}, 
	{ 0x062E, 0xFEA5, JOIN_DUAL, ARABIC_KHEH},
	{ 0x062F, 0xFEA9, JOIN_RIGT, ARABIC_DAL},
	{ 0x0630, 0xFEAB, JOIN_RIGT, ARABIC_ZAL},
	{ 0x0631, 0xFEAD, JOIN_RIGT, ARABIC_REH},
	{ 0x0632, 0xFEAF, JOIN_RIGT, ARABIC_ZEH},
	{ 0x0633, 0xFEB1, JOIN_DUAL, ARABIC_SEEN},
	{ 0x0634, 0xFEB5, JOIN_DUAL, ARABIC_SHEEN},
	{ 0x0635, 0xFEB9, JOIN_DUAL, ARABIC_SAD}, 
	{ 0x0636, 0xFEBD, JOIN_DUAL, ARABIC_ZAD},
	{ 0x0637, 0xFEC1, JOIN_DUAL, ARABIC_TAH},
	{ 0x0638, 0xFEC5, JOIN_DUAL, ARABIC_ZAH},
	{ 0x0639, 0xFEC9, JOIN_DUAL, ARABIC_AIN},
	{ 0x063A, 0xFECD, JOIN_DUAL, ARABIC_GHAIN},
	{ 0x0640, 0x0640, JOIN_DUAL, ARABIC_NOJOIN },//cmx add 3.26
	{ 0x0641, 0xFED1, JOIN_DUAL, ARABIC_FEH},
	{ 0x0642, 0xFED5, JOIN_DUAL, ARABIC_GHAF},

	{ 0x0643, 0xFED9, JOIN_DUAL, ARABIC_KAF }, 	
	{ 0x0649, 0xFEEF, JOIN_DUAL, ARABIC_YEH },	
	{ 0x064A, 0xFEF1, JOIN_DUAL, ARABIC_YEH },	
	
	{ 0x0644, 0xFEDD, JOIN_DUAL, ARABIC_LAM},
	{ 0x0645, 0xFEE1, JOIN_DUAL, ARABIC_MEEM},
	{ 0x0646, 0xFEE5, JOIN_DUAL, ARABIC_NOON },
	{ 0x0647, 0xFEE9, JOIN_DUAL, ARABIC_HEH},	
	{ 0x0648, 0xFEED, JOIN_RIGT, ARABIC_VAV},
	{ 0x067E, 0xFB56, JOIN_DUAL, ARABIC_PEH},
	{ 0x0686, 0xFB7A, JOIN_DUAL, ARABIC_CHEH},
	{ 0x0698, 0xFB8A, JOIN_RIGT, ARABIC_JHEH},	
	{ 0x06A9, 0xFB8E, JOIN_DUAL, ARABIC_KAF}, 
	{ 0x06AF, 0xFB92, JOIN_DUAL, ARABIC_GAF},	
	{ 0x06CC, 0xFEEF, JOIN_DUAL, ARABIC_YEH},

//	{ 0x0622, 0xFE8E, JOIN_RIGT, ARABIC_NOJOIN},
//	{ 0x06C0, 0xFBA4, JOIN_RIGT, ARABIC_NOJOIN },
//	{ 0xFEFB, 0xFEFB, JOIN_RIGT, ARABIC_NOJOIN },
//	{ 0x0623, 0xFE83, JOIN_RIGT, ARABIC_NOJOIN },
};

static unsigned short           *word_pos[MAX_WORDS] = { 0 };
static unsigned char            word_dir[MAX_WORDS] = { 0 };
static unsigned short           str_buf[MAX_CHARS] = { 0 };
static unsigned short last_code=0;
static unsigned short next_code=0;

static unsigned int get_dir(unsigned short code)
{
    unsigned int    dir = 0;

    if ((0x0009 == code) || (0x0020 == code))
    {
        dir = BIDI_SPC;
    }
    else if ((0x200C == code) || (0x200D == code))
    {
        dir = BIDI_RTL;
    }

    /* Arabic */
    else if (((0x0600 <= code) && (code <= 0x06FF))
         || ((0x0750 <= code) && (code <= 0x077F))
         || ((0xFB50 <= code) && (code <= 0xFDFF))
         || ((0xFE70 <= code) && (code <= 0xFEFE)))
    {
        dir = BIDI_RTL;
    }

    /* Hebrew */
    else if (((0x0590 <= code) && (code <= 0x05FF)) || ((0xFB00 <= code) && (code <= 0xFB4F)))
    {
        dir = BIDI_RTL;
    }

    /* change some punctuation characters direction
        '(' & ')' & '<' & '>' & '[' & ']' & '{' & '}' & '!' & '?' */
    else if ((0x0028 == code) || (0x0029 == code) || (0x002F == code) 
         || (0x005C == code)   || (0x003C == code)|| (0x003E == code) 
         || (0x005B == code) || (0x005D == code) || (0x007B == code)
         || (0x007D == code) || (0x003A == code)|| (0x002D == code) 
         || (0x0021 == code) || (0x003F == code)) 
    {
        dir = BIDI_RTL;
    }
    else
    {
        dir = BIDI_LTR;
    }

    if( (0x30<=last_code) && (last_code<= 0x39) && (0x30<=next_code) && (next_code<= 0x39))
    {
        if(0x003A == code)
        {
            dir = BIDI_LTR;
        }
    }

    return dir;
}

static void merge_seg(unsigned int idx, unsigned char dir)
{
    unsigned int    i = 0;

    for (i = idx + 1; i <= segments - 1; i++)
    {
        word_pos[i] = word_pos[i + 1];
        word_dir[i] = word_dir[i + 1];
    }

    word_dir[idx] = dir;
    segments--;
}

static void absorb_spc(unsigned char dir)
{
    unsigned int    i = 0;

    while (i < segments)
    {
        if (word_dir[i] == dir)
        {
            while ((i > 0) && (BIDI_SPC == word_dir[i - 1]))
            {
                --i;
                merge_seg(i, dir);
                if ((i > 0) && (word_dir[i - 1] == word_dir[i]))
                {
                    --i;
                    merge_seg(i, dir);
                }
            }

            while ((i < segments - 1) && (BIDI_SPC == word_dir[i + 1]))
            {
                merge_seg(i, dir);
                if ((i < segments - 1) && (word_dir[i] == word_dir[i + 1]))
                {
                    merge_seg(i, dir);
                }
            }
        }

        i++;
    }
}


static void dir_split(unsigned short **str, unsigned short *end)
{
    unsigned short  code = 0;
    unsigned short  *begin = NULL;
    unsigned int    dir = 0;
    unsigned int    dir_last = 0;
    unsigned int    stage = 0;

    if((!str) || (!end))
    {
        return;
    }
    begin = *str;
    dir_last = BIDI_NULL;
    segments = 0;
    code = *begin;
    stage = 0;

    last_code = 0;
    next_code = 0;
    
    while (begin < end)
    {

        if (CHAR_CARRIAGE_RETURN == code)
        {
            stage = 1;
        }
        else if ((CHAR_LINE_FEED == code) && (1 == stage))
        {
            break;
        }
        else
        {
            stage = 0;
        }
        
        last_code = *(begin-1);
	next_code = *(begin+1);
        dir = get_dir(code);
        if (dir != dir_last)
        {
            dir_last = dir;
            word_pos[segments] = begin;
            word_dir[segments] = dir;
            segments++;
        }
        ++begin;
        code = *begin;
        if (segments >= MAX_WORDS)
        {
            PRINTF("\n segments = %d\n", segments);
            segments = MAX_WORDS;
            break;
        }
    }

    if (begin < end)
    {
        begin--;
    }

    word_pos[segments] = begin;
    word_dir[segments] = BIDI_NULL;
    *str = begin;
    absorb_spc(BIDI_RTL);
    absorb_spc(BIDI_LTR);
}

static void latin_line(unsigned short *begin, unsigned short *end)
{
    if((!begin) || (!end))
    {
        return;
    }
    while (begin <= end)
    {
        *str_out = *begin;
        str_out++;
        begin++;
    }
}

static struct arabic_join *seek_info(unsigned short code)
{
    unsigned int    i = 0;

    if (ARABIC_CHAR == ara_per_switch)
    {
        for (i = 0; i < MAX_INFO; i++)
        {
            if (arabic_code_info[i].unicode == code)
            {
                break;
            }
        }

        return i == MAX_INFO ? NULL : (struct arabic_join *) &arabic_code_info[i];
    }
    else if (PERSIAN_CHAR == ara_per_switch)
    {
        for (i = 0; i < MAX_PER_INFO; i++)
        {
            if (persian_code_info[i].unicode == code)
            {
                break;
            }
        }

        return (MAX_PER_INFO == i) ? NULL : (struct arabic_join *) &persian_code_info[i];
    }

    return NULL;
}

static unsigned short get_ligature(unsigned short present)
{
    switch (present)
    {
    case 0xFE82:
        return 0xFEF5;

    case 0xFE84:
        return 0xFEF7;

    case 0xFE88:
        return 0xFEF9;

    case 0xFE8E:
        return 0xFEFB;

    default:
        break;
    }

    return 0x0020;
}

static void out_char(unsigned short code, unsigned char mode)
{
    unsigned char       group = 0;
    unsigned short      present = 0;
    struct arabic_join  *code_info = NULL;

    code_info = seek_info(code);
    if (NULL == code_info)
    {
        *str_out = code;
        str_out++;
        group_last = ARABIC_OTHER;
        return;
    }

    group = code_info->group;
    if ((ARABIC_LAM == group) && (ARABIC_ALEF == group_last))
    {
        str_out--;
        present = get_ligature(*str_out);
        mode &= 1;
    }
    else
    {
        present = code_info->present;
    }

    group_last = group;
    if ((present != 0x200C) && (present != 0x200D))
    {
        *str_out = present + mode;
        str_out++;
    }
}

static void arabic_line(unsigned short *end, unsigned short *begin)
{
    unsigned short      code = 0;
    unsigned short      code_last = 0;
    unsigned char       join = 0;
    unsigned char       join_last = 0;
    unsigned char       mode = 0;
    unsigned char       mode_last = 0;
    struct arabic_join  *info = NULL;

    if((!end) || (!begin))
    {
        return;
    }
    group_last = ARABIC_OTHER;
    code_last = 0;
    join_last = JOIN_NONE;
    mode_last = JOIN_NONE;
    while (end >= begin)
    {
        //      mode_last = JOIN_NONE;
        info = seek_info(code = *end);
        join = (NULL == info) ? JOIN_NONE : info->join;
        if ((join_last & JOIN_RIGT) && (join & JOIN_LEFT))
        {
            mode_last |= JOIN_RIGT;
            mode = JOIN_LEFT;
        }
        else
        {
            mode = JOIN_NONE;
        }

        if (code_last)
        {
            if (PERSIAN_CHAR == ara_per_switch)
            {
                if ((0x06cc == code_last) && (mode_last > 1))
                {
                    out_char(code_last, 2 + mode_last);
                }
                //else if ((0x0640 == code_last) && (mode_last > 1))
                //{
                //    out_char(code_last, 0); //no xferomer add on 20130618
                //}
                else
                {
                    out_char(code_last, mode_last);
                }
            }
            else
            {
                out_char(code_last, mode_last);
            }
        }

        mode_last = mode;
        code_last = code;
        join_last = join;
        end--;
    }

    if (code_last)
    {
        if (PERSIAN_CHAR == ara_per_switch)
        {
            if ((0x06cc == code_last) && (mode_last > 1))
            {
                out_char(code_last, 2 + mode_last);
            }
            else
            {
                out_char(code_last, mode_last);
            }
        }
        else
        {
            out_char(code_last, mode_last);
        }
    }
}

void hebrew_punctuation_process(unsigned short *str, unsigned int size)
{
    unsigned int    i = 0;

    if (NULL == str)
    {
        return;
    }
    for (i = 0; i < size; i++)
    {
        if (CHAR_LEFT_RND_BRACKET == str[i])
        {
            // '('
            str[i] = CHAR_RIGHT_RND_BRACKET;
        }
        else if (CHAR_RIGHT_RND_BRACKET == str[i])
        {
            // ')'
            str[i] = CHAR_LEFT_RND_BRACKET;
        }
        else if ((CHAR_LEFT_ANG_BRACKET == str[i])
             || (CHAR_LEFT_SQU_BRACKET == str[i])
             || (CHAR_RIGHT_OPEN_BRACKET == str[i]))
        {
            // '<' & '[' & '{'
            str[i] += 2;
        }
        else if ((CHAR_RIGHT_ANG_BRACKET == str[i])
             || (CHAR_RIGHT_SQU_BRACKET == str[i])
             || (CHAR_RIGHT_CLOSE_BRACKET == str[i]))
        {
            // '<' & '[' & '{'
            str[i] -= 2;
        }
    }
}

void arabic_process(unsigned short *str, unsigned int size)
{
    unsigned int    len = 0;
    unsigned short  *ptr = NULL;
    unsigned short  *begin = NULL;
    unsigned short  *end = NULL;

    ara_per_switch = ARABIC_CHAR;
    if ((0 == size) || (NULL == str))
    {
        return;
    }

    ptr = str;
    str_out = str_buf;
    if (size >= MAX_CHARS)
    {
        size = MAX_CHARS - 1;
        str[MAX_CHARS - 1] = 0x0000;
    }

    PRINTF("\nstr size = %d\n", size);

    dir_split(&str, str + size);

    while (segments)
    {
        end = word_pos[segments] - 1;
        segments--;
        begin = word_pos[segments];
        if (BIDI_LTR == word_dir[segments])
        {
            latin_line(begin, end);
        }
        else
        {
            arabic_line(end, begin);
        }
    }

    len = str_out - str_buf;
    MEMCPY(ptr, str_buf, len * 2);
    ptr[len] = 0;
}

void persian_process(unsigned short *str, unsigned int size)
{
    unsigned int    len = 0;
    unsigned short  *ptr = NULL;
    unsigned short  *begin = NULL;
    unsigned short  *end = NULL;

    ara_per_switch = PERSIAN_CHAR;
    if ((0 == size) || (NULL == str))
    {
        return;
    }

    ptr = str;
    str_out = str_buf;
    if (size >= MAX_CHARS)
    {
        size = MAX_CHARS - 1;
        str[MAX_CHARS - 1] = 0x0000;
    }

    PRINTF("\nstr size = %d\n", size);

    dir_split(&str, str + size);

    while (segments)
    {
        end = word_pos[segments] - 1;
        segments--;
        begin = word_pos[segments];
        if (BIDI_LTR == word_dir[segments])
        {
            latin_line(begin, end);
        }
        else
        {
            arabic_line(end, begin);
        }
    }
    
    len = str_out - str_buf;
    MEMCPY(ptr, str_buf, len * 2);

}
