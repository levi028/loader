/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_char_language.c
*
*    Description: implement osd draw speical language(Inherited from osd lib).
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include "osd_lib_internal.h"
#include "osd_char_language.h"


#ifdef PERSIAN_SUPPORT
//-----------------------------------------------------------
typedef struct line_information
{
    UINT8   line_number;
    UINT16  start_offset;
    UINT16  end_offset;
} LINE_INFO;

enum
{
    ENGLISH_ENV = 0,
#ifdef CHINESE_SUPPORT
    CHINESE_ENV,
#endif
    FRANCE_ENV,
    GERMANY_ENV ,
    ITALY_ENV,
    SPANISH_ENV,
    PORTUGUESE_ENV,
    RUSSIAN_ENV,
    TURKISH_ENV,
    POLISH_ENV,
    ARABIC_ENV,
#ifdef PERSIAN_SUPPORT
    PERSIAN_ENV,
#endif
    ENDLANG_ENV
};

const unsigned short arbic_position_index[] = /*first,last,midile,alone*/
{
    0x621, 0x622, 0x623, 0x624, 0x625, 0x626, 0x627, 0x628, 0x629, 0x62A, 0x62B,
    0x62C, 0x62D, 0x62E, 0x62F, 0x630, 0x631, 0x632, 0x633, 0x634, 0x635, 0x636,
    0x637, 0x638, 0x639, 0x63A, 0x63B, 0x63C, 0x63D, 0x63E, 0x63F, 0x640, 0x641,
    0x642, 0x643, 0x644, 0x645, 0x646, 0x647, 0x648, 0x649, 0x64A, //for arabic
    0x67E, 0x686, 0x698, 0x6AF, 0x6A9,  //for Persian 4 extended Char
    0x06cc,	

};
#define ARB_POS_INDX_NUM  (sizeof(arbic_position_index)/sizeof(arbic_position_index[0]))


const unsigned short arbic_convert_order_special_ascii_index[] =
{
    //  0x22,0x20,//0x0D,0x0A,
    0xFEF5, 0xFEF6,
    0xFEF7, 0xFEF8,
    0xFEF9, 0xFEFA,
    0xFEFB, 0xFEFC,
    //add arabic digital number 0 1 2 3 4 5 6 7 8 9
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};
#define ARB_COVT_ORD_SP_ASC_INDX	(sizeof(arbic_convert_order_special_ascii_index)/sizeof(arbic_convert_order_special_ascii_index[0]))

const unsigned short arbic_combing_char_index[] =
{
    0xFEF5, 0xFEF6,
    0xFEF7, 0xFEF8,
    0xFEF9, 0xFEFA,
    0xFEFB, 0xFEFC,
};
#define ARB_COMB_CHAR_INDX	(sizeof(arbic_combing_char_index)/sizeof(arbic_combing_char_index[0]))

const unsigned short arbic_convert_order_special_ascii_punctuation_index[] =
{
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
    0x2C, 0x2D, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x5E,
    0x5F, 0x60, 0x7B, 0x7C, 0x7D, 0x7E,
    0x60C,

};
#define ARB_COVT_ORD_SP_ASC_PUNC_INDX	(sizeof(arbic_convert_order_special_ascii_punctuation_index)/sizeof(arbic_convert_order_special_ascii_punctuation_index[0]))

const unsigned short arbic_position[][4] = /*last,first,midile,alone*/
{
    { 0xfe80, 0xfe80, 0xfe80, 0xfe80}, /*0x0621*/
    { 0xfe82, 0xfe81, 0xfe82, 0xfe81}, /*0x0622*/
    { 0xfe84, 0xfe83, 0xfe84, 0xfe83},/*0x0623*/
    { 0xfe86, 0xfe85, 0xfe86, 0xfe85},/*0x0624*/
    { 0xfe88, 0xfe87, 0xfe88, 0xfe87},/*0x0625*/
    { 0xfe8a, 0xfe8b, 0xfe8c, 0xfe89},/*0x0626*/
    { 0xfe8e, 0xfe8d, 0xfe8e, 0xfe8d},/*0x0627*/
    { 0xfe90, 0xfe91, 0xfe92, 0xfe8f},/*0x0628*/
    { 0xfe94, 0xfe93, 0xfe93, 0xfe93},/*0x0629*/
    { 0xfe96, 0xfe97, 0xfe98, 0xfe95},/*0x062A*/
    { 0xfe9a, 0xfe9b, 0xfe9c, 0xfe99},/*0x062B*/
    { 0xfe9e, 0xfe9f, 0xfea0, 0xfe9d},/*0x062C*/
    { 0xfea2, 0xfea3, 0xfea4, 0xfea1},/*0x062D*/
    { 0xfea6, 0xfea7, 0xfea8, 0xfea5},/*0x062E*/
    { 0xfeaa, 0xfea9, 0xfeaa, 0xfea9},/*0x062F*/
    { 0xfeac, 0xfeab, 0xfeac, 0xfeab},/*0x0630*/
    { 0xfeae, 0xfead, 0xfeae, 0xfead},/*0x0631*/
    { 0xfeb0, 0xfeaf, 0xfeb0, 0xfeaf},/*0x0632*/
    { 0xfeb2, 0xfeb3, 0xfeb4, 0xfeb1},/*0x0633*/
    { 0xfeb6, 0xfeb7, 0xfeb8, 0xfeb5},/*0x0634*/
    { 0xfeba, 0xfebb, 0xfebc, 0xfeb9},/*0x0635*/
    { 0xfebe, 0xfebf, 0xfec0, 0xfebd},/*0x0636*/
    { 0xfec2, 0xfec3, 0xfec4, 0xfec1},/*0x0637*/
    { 0xfec6, 0xfec7, 0xfec8, 0xfec5},/*0x0638*/
    { 0xfeca, 0xfecb, 0xfecc, 0xfec9},/*0x0639*/
    { 0xfece, 0xfecf, 0xfed0, 0xfecd},/*0x063A*/
    { 0x63b, 0x63b, 0x63b, 0x63b},       /*0x063B*/
    { 0x63c, 0x63c, 0x63c, 0x63c},       /*0x063C*/
    { 0x63d, 0x63d, 0x63d, 0x63d},       /*0x063D*/
    { 0x63e, 0x63e, 0x63e, 0x63e},       /*0x063E*/
    { 0x63f, 0x63f, 0x63f, 0x63f},       /*0x063F*/
    { 0x640, 0x640, 0x640, 0x640},       /*'-'*//*0x0640*/
    { 0xfed2, 0xfed3, 0xfed4, 0xfed1},/*0x0641*/
    { 0xfed6, 0xfed7, 0xfed8, 0xfed5},/*0x0642*/
    { 0xfeda, 0xfedb, 0xfedc, 0xfed9},/*0x0643*/
    { 0xfede, 0xfedf, 0xfee0, 0xfedd},/*0x0644*/
    { 0xfee2, 0xfee3, 0xfee4, 0xfee1},/*0x0645*/
    { 0xfee6, 0xfee7, 0xfee8, 0xfee5},/*0x0646*/
    { 0xfeea, 0xfeeb, 0xfeec, 0xfee9},/*0x0647*/
    { 0xfeee, 0xfeed, 0xfeee, 0xfeed},/*0x0648*/
    { 0xfef0, 0xfeef, 0xfef0, 0xfeef},/*0x0649*/
    { 0xfef2, 0xfef3, 0xfef4, 0xfef1}, /*0x064a*/

    { 0xFB57, 0xFB59, 0xFB59, 0xFB57},/*0x67E*/
    { 0xFB7B, 0xFB7C, 0xFB7D, 0xFB7A,},/*0x686*/
    { 0xFB8B, 0x698  , 0xFB8B, 0x698},/*0x698*/
    { 0xFB93, 0xFB94, 0xFB95, 0x6AF},/*0x6AF*/
    { 0xFB8F, 0xFB90, 0xFB91, 0x6A9},/*0x6A9*/
    { 0xfbfd, 0xfbfe, 0xfbff, 0xfbfc},  /*0x06cc*/
    //total 48...
};
#if 0//for arabic
const unsigned short the_set1[23] =
{
    //for Front link.---vision right
    0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
    0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
    0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626
};
const unsigned short the_set2[35] =
{
    //for behind link.---vision Left
    0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
    0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
    0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626,
    0x627, 0x623, 0x625, 0x622, 0x62f, 0x630, 0x631, 0x632,
    0x648, 0x624, 0x629, 0x649
};
#else//for persian.

const unsigned short the_set1[] =
{
    //for Front link.---vision left
    0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
    0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
    0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626,
    0x627, 0x623, 0x625, 0x622, 0x62f, 0x630, 0x631, 0x632,
    0x648, 0x624, 0x629, 0x649, 0x640,
    0x67E, 0x686, 0x698, 0x6AF, 0x6A9, //for persian
     0x6CC,
    0xFEF6, 0xFEF8, 0xFEFA, 0xFEFC
};
#define SET1_NUM	(sizeof(the_set1)/sizeof(the_set1[0]))

const unsigned short the_set2[] =
{
    //for behind link.---vision right
    0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
    0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
    0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626,
    /*0x622,*/ 0x623, 0x625,/* 0x627, 0x62f, 0x630, 0x631, 0x632,
    0x648,*/ 0x624, 0x629, 0x649, 0x640,
    0x67E, 0x686,  0x6AF, 0x6A9, //for persian.
    0x6CC,
    //0x698//for persian
};
#define SET2_NUM	(sizeof(the_set2)/sizeof(the_set2[0]))

#endif
const unsigned short the_set3[] = {0x622, 0x623, 0x625, 0x627};
#define SET3_NUM	(sizeof(the_set3)/sizeof(the_set3[0]))
const unsigned short persian_extend[] =
{0x67E, 0x686, 0x698, 0x6AF, 0x6A9,0x6CC}; //extend 4 char in persian support
#define PER_EX_NUM	(sizeof(persian_extend)/sizeof(persian_extend[0]))
const unsigned short persian_subscript[] =
{0x64B, 0x64C, 0x64D, 0x64E, 0x64F, 0x650, 0x651, 0x652,};
#define PER_SUB_NUM	(sizeof(persian_subscript)/sizeof(persian_subscript[0]))

//extend 4 char in persian support
const unsigned short arabic_specs[][2] =
{
    {0xFEF5, 0xFEF6},
    {0xFEF7, 0xFEF8},
    {0xFEF9, 0xFEFA},
    {0xFEFB, 0xFEFC},
};

//For ZERO WIDTH NON-JOINER
const unsigned short the_zwnj_set[] = {0x200C};
#define ZWNJ_NUM	(sizeof(the_zwnj_set)/sizeof(the_zwnj_set[0]))


static UINT16   str_bak[1024] = {0};

#define BUF_SEC  29
//#define PRNT_BUF


static OSAL_ID xformer_cb_sem = OSAL_INVALID_ID;
extern UINT16 osd_get_lang_environment(void);

BOOL is_arabic_extend_set(unsigned short unicode)
{
    if ((unicode <= 0x6fe) && (unicode >= 0x660) && (0x698 != unicode))
    {
        return TRUE;
    }
    else if ((unicode <= 0x39) && (unicode >= 0x30)) //arabic digital num 0~9
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
BOOL is_arabic_set(unsigned short unicode, unsigned short *set)
{
    INT16 set_len = 0;
    INT16 i = 0;

    //SetLen=sizeof(Set)/2;
    if (the_set1 == set)
    {
        set_len = SET1_NUM;
    }
    else if (the_set2 == set)
    {
        set_len = SET2_NUM;
    }
    else if (the_set3 == set)
    {
        set_len = SET3_NUM;
    }
    else if (persian_extend == set)
    {
        set_len = PER_EX_NUM;
    }
    else if (arbic_position_index == set)
    {
        set_len = ARB_POS_INDX_NUM;
    }
    else if (arbic_convert_order_special_ascii_index == set)
    {
        set_len = ARB_COVT_ORD_SP_ASC_INDX;
    }
    else if (arbic_convert_order_special_ascii_punctuation_index == set)
    {
        set_len = ARB_COVT_ORD_SP_ASC_PUNC_INDX;
    }
    else if (arbic_combing_char_index == set)
    {
        set_len = ARB_COMB_CHAR_INDX;
    }
    else if (persian_subscript == set)
    {
        set_len = PER_SUB_NUM;
    }
    else if(the_zwnj_set==set)
    {
	set_len = ZWNJ_NUM;
    }   
    while (i < set_len)
    {
        if (unicode == set[i])
        {
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

void xformer_capture_semaphore(void)
{

    if (OSAL_INVALID_ID == xformer_cb_sem)
    {
        xformer_cb_sem = osal_semaphore_create(1);
    }
    if (OSAL_INVALID_ID != xformer_cb_sem)
    {
        osal_semaphore_capture(xformer_cb_sem, OSAL_WAIT_FOREVER_TIME);
    }
}

void xformer_release_semaphore(void)
{

    if (xformer_cb_sem != OSAL_INVALID_ID)
    {
        osal_semaphore_release(xformer_cb_sem);
    }
}

#if (1)
void arabic_unistr_xformer(unsigned short *uni_str, BOOL convert_flag, BOOL mb_flag)
{
    #define first       1////0
    #define last        0///1
    #define middle  2
    #define alone   3

    //SYSTEM_DATA* p_sys_data;
    INT16   uni_str_len=0;
    INT16 i=0;
    INT16 j=0;
    INT16 xformer_pos = 0;
    INT16 connect_pos=0;
    INT16 k=0;
    INT16 m=0;
    INT16 n=0;
    INT16 temp=0;
    //INT16 flag=0;
    INT16 t=0;
    INT16 xformer_pos_array[4] = {alone, last, first, middle};
    //{Alone,First,Last,Middle};
    LINE_INFO  line_info[16];//={{0}};
    ID_RSC      rsc_id=0;

#ifdef PRNT_BUF
    UINT32 sub_time_bk = GetCurSubTime();	
#endif

    
    rsc_id = osd_get_lang_environment();
    if ((rsc_id == PERSIAN_ENV) || (rsc_id == ARABIC_ENV)) //for  arabic and persian .
    {
        i = 0;
        memset((void *)(&(line_info[0])),0x00,sizeof(line_info[0]) * 16);
        while (uni_str[i])
        {
            if (mb_flag)
                uni_str[i] = ((uni_str[i] << 8) & 0xFF00) | \
                            ((uni_str[i] >> 8) & 0xFF); //MB to LB
            i++;
        }
        if (0 == i)
        {
            return;
        }
        uni_str_len = i;

        #ifdef PRNT_BUF
        if(BUF_SEC == sub_time_bk)
        {
            libc_printf("1 \n");
            libc_printf("%d buf: ",uni_str_len);
            for(i=0;i<uni_str_len;i++)
            {
            	libc_printf("0x%04x ",uni_str[i]);
            }
            libc_printf(" \n");
        }
        #endif
        
        //if(UniStrLen>511)
        //  return;
        //do not transfer sftring for too long sub_string for some time.
        //for(i=0;i<1024;i++)
        //  StrBak[i]=0;
        //step1 : follow rule2-connecting   rule.
        for (i = 0, j = 0; i < uni_str_len; i++)
        {
            if (is_arabic_set(uni_str[i], (unsigned short *)persian_subscript))
            {
                continue;//eat subscript char.
            }
            else if (0x644 == uni_str[i])
            {
                if (is_arabic_set(uni_str[i + 1], (unsigned short *)the_set3))
                {
                    if (i)
                    {
                        if (is_arabic_set(uni_str[i - 1], (unsigned short *)the_set2))
                        {
                            connect_pos = 1;
                        }
                        else
                        {
                            connect_pos = 0;
                        }
                    }
                    else
                    {
                        connect_pos = 0;
                    }
                    k = 0;
                    while (1)
                    {
                        if (uni_str[i + 1] == the_set3[k])
                        {
                            break;
                        }
                        k++;
                    }
                    //UniStr[512+j]=arabic_specs[k][connect_pos];
                    //choose connecting Char to replace 2-combing chars
                    str_bak[j] = arabic_specs[k][connect_pos];
                    //choose connecting Char to replace 2-combing chars
                    i++;
                }
                else
                    //UniStr[512+j]=UniStr[i];//bak-copy unistr to 512B Address
                {
                    str_bak[j] = uni_str[i];    //bak-copy unistr to 512B Address
                }
            }
            else
            {
                //UniStr[512+j]=UniStr[i];//bak-copy unistr to 512B Address
                str_bak[j] = uni_str[i]; //bak-copy unistr to 512B Address
            }
            j++;
        }
        if (j < uni_str_len) //copy back string after connecting form 512 to 0
        {
            for (i = 0; i <= j; i++)
            {
                if (i == j)
                {
                    uni_str[i] = 0;    //set NULL ending  flag
                }
                else
                    //UniStr[i]=UniStr[i+512];
                {
                    uni_str[i] = str_bak[i];
                }
            }
        }

        //step 2: follow rule 1--xformer rule.
        uni_str_len = j;

        #ifdef PRNT_BUF
        if(BUF_SEC == sub_time_bk)
        {
            libc_printf("2 \n");
            libc_printf("%d buf: ",uni_str_len);
            for(i=0;i<uni_str_len;i++)
            {
                libc_printf("0x%04x ",uni_str[i]);
            }
            libc_printf(" \n");
        }
        #endif

#if 1
        //-----------------------------------------------------
        //3Convert continous Persian string except Standard Ascii string Uni-Bytes.
        for (i = 0; i < 16; i++)
        {
            line_info[i].line_number = 0xFF;
            line_info[i].start_offset = line_info[i].end_offset = 0;
        }
        i = j = 0;
        line_info[0].line_number = 0;
        line_info[0].start_offset = 0;
        line_info[0].end_offset = 0;
        while (i < uni_str_len)
        {
            if ((0x0D == uni_str[i]) && (0x0A == uni_str[i + 1]))
            {
                line_info[j].line_number = j;
                line_info[j].end_offset = i - 1;
                j++;
                line_info[j].line_number = j;
                line_info[j].start_offset = i + 2;
                i += 2;
            }
            else
            {
                i++;
            }
        }
        line_info[j].end_offset = uni_str_len - 1;

        for (i = 0; i <= j; i++)
        {
            k = line_info[i].start_offset;
            while (k <= line_info[i].end_offset)
            {
                if (((uni_str[k] >= 0x60D) && (uni_str[k] <= 0x6fe)) || \
                        is_arabic_set(uni_str[k], \
                                      (unsigned short *)arbic_combing_char_index))
                {
                    line_info[i].start_offset = k;
                    break;
                }
                else
                {
                    k++;
                }
            }
            //that means current line is include arabic/persian code.
            if ((k - 1) != line_info[i].end_offset)
            {
                k = line_info[i].end_offset;
                while (k > line_info[i].start_offset)
                {
                    if (((uni_str[k] >= 0x60D) && (uni_str[k] <= 0x6fe)) || \
                            is_arabic_set(uni_str[k], \
                                          (unsigned short *)arbic_combing_char_index))
                    {
                        line_info[i].end_offset = k;
                        break;
                    }
                    else
                    {
                        k--;
                    }
                }
                //convert arabic string segments
                k = line_info[i].start_offset;
                while (k <= line_info[i].end_offset)
                {
                    if (((uni_str[k] >= 0x60D) && (uni_str[k] <= 0x6fe)) || \
                            is_arabic_set(uni_str[k], (unsigned short *)\
                                          arbic_convert_order_special_ascii_index) || \
                            is_arabic_set(uni_str[k],(unsigned short *)the_zwnj_set)||	\
                            is_arabic_set(uni_str[k], (unsigned short *)\
                                          arbic_convert_order_special_ascii_punctuation_index))
                    {
                        m = n = k;
                        while ((k <= line_info[i].end_offset) &&  \
                                ((uni_str[k] >= 0x60D && uni_str[k] <= 0x6fe) || \
                                 is_arabic_set(uni_str[k], (unsigned short *)\
                                 arbic_convert_order_special_ascii_index) || \
                                 is_arabic_set(uni_str[k],(unsigned short *)the_zwnj_set)|| \
                                 is_arabic_set(uni_str[k], (unsigned short *)\
                                               arbic_convert_order_special_ascii_punctuation_index)))
                        {
                            n = k;
                            k++;
                        }
                        for (t = m; t < (((n - m + 1) >> 1) + m); t++)
                        {
                            temp = uni_str[t];
                            uni_str[t] = uni_str[n + m - t];
                            uni_str[n + m - t] = temp;
                        }
                    }
                    else
                    {
                        k++;
                    }
                }
            }
        }
        #ifdef PRNT_BUF
        if(BUF_SEC == sub_time_bk)
        {
            libc_printf("3 \n");
            libc_printf("%d buf: ",uni_str_len);
            for(i=0;i<uni_str_len;i++)
            {
                libc_printf("0x%04x ",uni_str[i]);
            }
            libc_printf(" \n");
        }
        #endif

        //--------------------------------------------------------------

        //3when exit multi line string,that means "0x000D,0x000A"
        //must be set as milestones for each line that it converts each line by.
        //3so we should do something as below to finish the converting.
        if (convert_flag)
        {
            i = 0;
            j = k = 0;
            while (i < uni_str_len)
            {
                if ((0x0D == uni_str[i]) && (0x0A == uni_str[i + 1]))
                {
                    k = i - 1;
                    for (m = j; m <= k; m++)
                    {
                        temp = m - j;
                        //UniStr[512+UniStrLen-i+temp]=UniStr[m];
                        str_bak[uni_str_len - i + temp] = uni_str[m];
                    }
                    //UniStr[512+UniStrLen-i-1]=0x0A;
                    //UniStr[512+UniStrLen-i-2]=0x0D;
                    str_bak[uni_str_len - i - 1] = 0x0A;
                    str_bak[uni_str_len - i - 2] = 0x0D;
                    j = i + 2;
                    i += 2;
                }
                else
                {
                    i++;
                }
            }
            for (m = 0; m < (uni_str_len - j); m++)
            {
                //UniStr[512+m]=UniStr[j+m];
                str_bak[m] = uni_str[j + m];
            }
            //copy back from address 512 to 0
            for (i = 0; i < uni_str_len; i++)
            {
                //UniStr[i]=UniStr[i+512];
                uni_str[i] = str_bak[i];
            }
        }
#endif

        #ifdef PRNT_BUF
        if(BUF_SEC == sub_time_bk)
        {
            libc_printf("4 \n");
            libc_printf("%d buf: ",uni_str_len);
            for(i=0;i<uni_str_len;i++)
            {
                libc_printf("0x%04x ",uni_str[i]);
            }
            libc_printf(" \n");
        }
        #endif


        //--------------------------------------------------------------
        //      for(i=0;i<UniStrLen;i++)CharDebug[i]=UniStr[i];
        for (i = 0; i < uni_str_len; i++)
        {
            if (((uni_str[i] >= 0x621) && (uni_str[i] <= 0x64A)) || \
                    is_arabic_set(uni_str[i], (unsigned short *)persian_extend))
            {
                if (0 == i)
                {
                    if (is_arabic_set(uni_str[i + 1], (unsigned short *)the_set2) || \
                            is_arabic_extend_set(uni_str[i + 1]))
                    {
                        xformer_pos = last;
                    }
                    else
                    {
                        xformer_pos = alone;
                    }
                }
                else if (i <= uni_str_len - 2)
                {
                    temp = 0;
                    if (is_arabic_set(uni_str[i - 1], (unsigned short *)the_set1) || \
                            is_arabic_extend_set(uni_str[i - 1]))
                    {
                        temp |= 0x02;    //0x02;
                    }
                    if (is_arabic_set(uni_str[i + 1], (unsigned short *)the_set2) || \
                            is_arabic_extend_set(uni_str[i + 1]))
                    {
                        temp |= 0x01;    //0x01;
                    }
                    //XformerPos:0--Alone    1--Last  2--First  3--Middle
                    xformer_pos = xformer_pos_array[temp];
                }
                else if (uni_str_len - 1 == i)
                {
                    if (is_arabic_set(uni_str[i - 1], (unsigned short *)the_set1) || \
                            is_arabic_extend_set(uni_str[i - 1]))
                    {
                        xformer_pos = first;
                    }
                    else
                    {
                        xformer_pos = alone;
                    }
                }
                //              CharDebug[i]|=(XformerPos<<12);
                //j=UniStr[i]-0x621;
                j = 0;
                while (1) //calculate char index
                {
                    if (uni_str[i] == arbic_position_index[j])
                    {
                        break;
                    }
                    j++;
                }
                //UniStr[i+512]=Arbic_Position[j][XformerPos];
                str_bak[i] = arbic_position[j][xformer_pos];
            }
            else
            {
                //UniStr[i+512]=UniStr[i];
                str_bak[i] = uni_str[i];
            }
        }
        for (i = 0; i < uni_str_len; i++)
        {
            uni_str[i] = str_bak[i];
            if (mb_flag)
                uni_str[i] = ((uni_str[i] << 8) & 0xFF00) | \
                            ((uni_str[i] >> 8) & 0xFF); //MB to LB
        }

        #ifdef PRNT_BUF
	if(BUF_SEC == sub_time_bk)
	{
		libc_printf("5 \n");
		libc_printf("%d buf: ",uni_str_len);
		for(i=0;i<uni_str_len;i++)
		{
			libc_printf("0x%04x ",uni_str[i]);
		}
		libc_printf(" \n");
	}
	#endif
	
	k=0;
	for(i=0;i<uni_str_len;i++)
	{
		if(is_arabic_set(uni_str[i],(unsigned short *)the_zwnj_set))
		{
			for(j=i; j<uni_str_len; j++)
			{
				uni_str[j] = uni_str[j+1];
			}
			k++;
		}
	}
	uni_str_len -= k;

	#ifdef PRNT_BUF
	if(BUF_SEC == sub_time_bk)
	{
		libc_printf("5.1 \n");
		libc_printf("%d buf: ",uni_str_len);
		for(i=0;i<uni_str_len;i++)
		{
			libc_printf("0x%04x ",uni_str[i]);
		}
		libc_printf(" \n");
	}		
#endif
		

        //      for(i=0;i<UniStrLen;i++)CharDebug[i+256]=UniStr[i];
        if (convert_flag) //this flag depending on direction of OSD show
        {
            for (i = 0; i < (uni_str_len >> 1); i++)
            {
                k = uni_str[i];
                uni_str[i] = uni_str[uni_str_len - i - 1];
                uni_str[uni_str_len - i - 1] = k;
            }
        }

    	#ifdef PRNT_BUF
	if(BUF_SEC == sub_time_bk)
	{
		libc_printf("6 \n");
		libc_printf("%d buf: ",uni_str_len);
		for(i=0;i<uni_str_len;i++)
		{
			libc_printf("0x%04x ",uni_str[i]);
		}
		libc_printf(" \n");
	}
	#endif

    }
    #undef first
    #undef last
    #undef middle
    #undef alone
}
#else

extern BOOL sub_flag;
void arabic_unistr_xformer(unsigned short *uni_str, BOOL convert_flag, BOOL mb_flag)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;
    UINT32 line_offset = 0;
    UINT32 cur_line_words = 0;
    BOOL switch_line_fg = FALSE;
    BOOL is_sub_str_fg = FALSE;
    UINT16  uni_tmp = 0;
    ID_RSC      rsc_id = 0;

    if(NULL == uni_str)
    {
        return ;
    }
    rsc_id = osd_get_lang_environment();
    if ((PERSIAN_ENV == rsc_id) || (ARABIC_ENV == rsc_id)) //for  arabic and persian .
    {
        //--judge if the string is include original persian string.
        /*
        i=j=0;
        while(UniStr[j])
        {
            i++;
            if(!(UniStr[j]>=0x60B&&UniStr[j]<=0x6fe))
                j++;
        }
        if(i==j)return;*/
        //--------
        while (uni_str[i])
        {
            if (mb_flag)
            {
                uni_str[i] = ((uni_str[i] << 8) & 0xFF00) | ((uni_str[i] >> 8) & 0xFF); //MB to LB
            }
            str_bak[i] = uni_str[i];
            i++;
            if (!(uni_str[j] >= 0x60B && uni_str[j] <= 0x6fe))
            {
                j++;
            }
        }
        str_bak[i] = 0x0;
        if (!i)
        {
            return;
        }
        if (i == j)
        {
            goto NOT_PROCESS;
        }

        //2step1: unify 0x0D,0x0A to 0x0
        is_sub_str_fg = FALSE;
        for (j = 0; j <= i; j++)
        {
            if ((0x0D == str_bak[j]) || (0x0A == str_bak[j]))
            {
                is_sub_str_fg = TRUE;
                str_bak[j] = 0;
            }
        }
        //2step2: process persian string line by line
        line_offset = 0;
        switch_line_fg = TRUE;
        for (j = 0; j <= i; j++)
        {
            if ((0 == str_bak[j]) && switch_line_fg)
            {
                sub_flag = TRUE;
                persian_process(str_bak + line_offset, j - line_offset);
                sub_flag    = FALSE;
                if (convert_flag)
                {
                    //start
                    //re-cacaulate cur line words
                    cur_line_words = j - line_offset;
                    k = 0;
                    while (0x0 == str_bak[j - k - 1])
                    {
                        cur_line_words--;
                        k++;
                    }
                    k = 0;
                    //end
                    while (k < (cur_line_words / 2))
                    {
                        uni_tmp = str_bak[line_offset + k] ;
                        str_bak[line_offset + k] = str_bak[line_offset + cur_line_words - 1 - k];
                        str_bak[line_offset + cur_line_words - 1 - k] = uni_tmp;
                        k++;
                    }
                }
                switch_line_fg = FALSE;
            }
            else
            {
                if ((0 != str_bak[j]) && (!switch_line_fg))
                {
                    line_offset = j;
                    switch_line_fg = TRUE;
                }
            }
        }
        //2step3: copyback persian string after processing
        switch_line_fg = FALSE;
        for (j = 0, k = 0; j <= i; j++)
        {
            if (0 != str_bak[j])
            {
                uni_str[k] = str_bak[j];
                k++;
                switch_line_fg = FALSE;
            }
            else
            {
                if (!switch_line_fg & is_sub_str_fg)
                {
                    uni_str[k] = 0x0A;
                    k++;
                    switch_line_fg = TRUE;
                }
            }
        }
        uni_str[k] = 0x0;

        //2step4: MB.LB restore.
    NOT_PROCESS:
        i = 0;
        while (uni_str[i])
        {
            if (mb_flag)
            {
                uni_str[i] = ((uni_str[i] << 8) & 0xFF00) | ((uni_str[i] >> 8) & 0xFF); //MB to LB
            }
            i++;
        }

    }
}
#endif
#endif

