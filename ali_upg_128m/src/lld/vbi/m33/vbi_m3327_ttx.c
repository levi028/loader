/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327_ttx.c

   *    Description:define the function to decode ttx packet
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
//#include <api/libttx/lib_ttx.h>


#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <api/libsi/si_tdt.h>
#include <api/libttx/ttx_osd.h>
#include "vbi_buffer.h"
#include "vbi_m3327.h"
#include "vbi_m3327_internal.h"

#ifdef TTX_BY_OSD
//#define MAX_PAGE_FILTER_NUM 5 //cloud
#define TTX_SPEC600_PAGE_ID 600

UINT8 g_packet_exist[8][25] = {{0}};


static UINT8 g_default_g0_set =0xff;

static UINT8 last_temp_data[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
// Used for fixing stream error of time show
static UINT8 last_temp_data2[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
// Backup data for next compare
static BOOL page_update_too_quick = FALSE;


static UINT8 page_start[8]={0,0,0,0,0,0,0,0};
static UINT8 g0_set[8]={0,0,0,0,0,0,0,0} ;
static UINT8 second_g0_set[8]={0,0,0,0,0,0,0,0};
static UINT8 g_transfer_mode = 1;
//UINT32 last_tmp_tick = 0xFFFFFFFF;
static struct PBF_CB *p_cur_wr_cb[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static struct PBF_CB *p_dec_rd_cb = NULL;
static UINT16 rd_page = 0xffff;
static struct PBF_CB *last_p_cur_wr_cb[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UINT8 last_mag =0;//last mag,0~7
static UINT8 transfer_mode =0;//transfer mode,(1:serial mode---0:parallel mode)
static UINT8 last_transfer_mode =0;

//init it in vbi_m3327_init()

static UINT8 last_row = 0;
static UINT8 last_col = 0;//to store row and col for more than 13 x/26 packet
static UINT16 last_p26_page_id=0;
static UINT16 last_p26_sub_page_id=0;
static BOOL b_packet26=FALSE;

static UINT8 last_packet[8]={0,0,0,0,0,0,0,0};
static UINT16 last_page_id[8]={0,0,0,0,0,0,0,0};

static UINT16 last_sub_page_id[8]= {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff};
static UINT8 b_save_subpage=FALSE;

static UINT8 last_page_number[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};//page uints+page tens
static BOOL b_show_page=FALSE;
static BOOL erase_page=FALSE;
static BOOL b_change_mag = FALSE;
//for some special stream have a page serial mode and a page parallel mode
//static BOOL bCheck_p26 = FALSE;
//for stream: x/n,x/26,x/n+1.....
static UINT16 g_inital_page = 0xFF;
static UINT16 last_erase_bit[2] = {0,0};
static UINT8 inital_page_status = 2;//TTXENG_TEXT;
static UINT16 page_transfered_num = 0;
//cloud, to judge when to filter blank header
static UINT16 first_ttx_page = 0xff;//cloud, first non-ttxsubtitle page id
static UINT32 page_rec_data[8] = {1,1,1,1,1,1,1,1};
//#define PRINTF_PAGE_PACKET
#ifdef PRINTF_PAGE_PACKET
static UINT16 g_debug_page_id = 0xffff; //printf special page packet
static UINT8 g_debug_line = 0xFF; //debug special packet
static UINT8 g_debug_p26 = 0;    //debug 26 packet of special page
static UINT8 g_disable_p26 = 0;//disable p26
#endif


#ifdef KAON_TTX_TIME
void set_kaon_ttx_time(UINT8 * temp)
{
    date_time dt;
    get_local_time(&dt);
        temp[0] = dt.hour/10 + '0';
        temp[1] = dt.hour % 10 + '0';
        temp[2] = ':';
        temp[3] = dt.min /10 + '0';
        temp[4] = dt.min% 10 + '0';
        temp[5] = ':';
        temp[6] = dt.sec / 10 + '0';
        temp[7] = dt.sec % 10 + '0';
}
#endif

static const UINT8 cor[8] =
{
    0x40, 0x10, 4, 8, 1, 0x20, 0x80, 2
};

static INLINE UINT8 hamming_8_4(UINT8 in)
{
    UINT32 a = 0;
    UINT32 b = 0;
    UINT32 c = 0;
    UINT32 d = 0;

    if(TRUE == g_vbi27_pconfig_par->hamming_8_4_enable)
    {
        d = in;
        d ^= d >> 4;
        d ^= d >> 2;
        d ^= d >> 1;

        if (!(d & 1))
        {
            a = in & 0xC5;
            a ^= a >> 4;
            a ^= a >> 2;
            a ^= a >> 1;
            b = in & 0x71;
            b ^= b >> 4;
            b ^= b >> 2;
            b ^= b << 1;
            c = in & 0x5C;
            c ^= c >> 4;
            c ^= c << 2;
            c ^= c >> 1;
            in ^= cor[(c & 4) | (b & 2) | (a & 1)];
        }
    }

    return in;
}
static INLINE void hamming_24_18(UINT8 in1,UINT8 in2,UINT8 in3,UINT8 *out1,UINT8 *out2,UINT8 *out3)
{
    //if(TRUE == g_vbi27_pconfig_par->hamming_24_16_enable)
    //{
        //reserved for feature usage
    //    ;
    //}
    *out1 = in1;
    *out2 = in2;
    *out3 = in3;
}

static INLINE UINT16 hex2dec(UINT16 x)
{ //0x0000 - 0x0799    =>  100 - 899
    UINT16 y = 0;
    UINT8 units=(x&0x000f);
    UINT8 tens=(x&0x00f0)>>4;
    UINT8 hunds=(x&0x0f00)>>8;

    if(MAX_TENS_NUM < tens /*||units>9*/)
    {
        return 0xffff;
    }
    y = units + tens*10 + hunds*100;
    if(TTX_PAGE_MIN_NUM > y)
    {
        y += 800;
    }
    if(TTX_PAGE_MAX_NUM < y)
    {
        y = 0xffff;
    }
    return y;
}

static INLINE UINT8 odd_dec(UINT8  data)
{
    UINT8 result=0;
    UINT8 cnt=0;
    UINT8 i = 0;

    for(i=0;i<8;i++)
    {
        if(data&(1<<i))
        {
            cnt++;
            result |= 1<<(7-i);
        }
    }

    if(!(cnt%TTX_NORMAL_2))
    {
         return 0x20;
    }
    /*
    result = ((data&0x80)>>7) + ((data&0x40)>>5) + \
    ((data&0x20)>>3) + ((data&0x10)>>1) \
        + ((data&0x08)<<1) + ((data&0x04)<<3) + ((data&0x02)<<5) ;
    */
     result = result&0x7f;
     return result;
}

static INLINE UINT16 x_mag(UINT8 addr1)
{
    UINT16 result = 0;

    result = ((addr1&0x40)>>6) + ((addr1&0x10)>>3) + (addr1&0x04);
    return result;
}

static INLINE UINT8 x_packet(UINT8 addr1,UINT8 addr2)
{
    UINT16 result = 0;

    result = ((addr2&0x40)>>5) + ((addr2&0x10)>>2) + ((addr2&0x04)<<1) + ((addr2&0x01)<<4) + (addr1&0x01);
    return result;
}

static INLINE void x_page_cs(UINT8 *in, struct header *head)
{
    UINT8 page_unit = 0;
    UINT8 page_tens = 0;
    UINT8 s1 = 0;
    UINT8 s2 = 0;
    UINT8 s3 = 0;
    UINT8 s4 = 0;
    UINT8 i = 0;
    UINT8 a[8]={0,0,0,0,0,0,0,0};

    for(i=0;i<8;i++)
    {
        a[i]=hamming_8_4(in[i]);
    }

    page_unit = ((a[0]&0x40)>>6)+((a[0]&0x10)>>3)+(a[0]&0x04)+((a[0]&0x01)<<3);
    page_tens = ((a[1]&0x40)>>6)+((a[1]&0x10)>>3)+(a[1]&0x04)+((a[1]&0x01)<<3);
    s1 =  ((a[2]&0x40)>>6) + ((a[2]&0x10)>>3) + (a[2]&0x04) + ((a[2]&0x01)<<3);
    s2 =  ((a[3]&0x40)>>6) + ((a[3]&0x10)>>3) + (a[3]&0x04);
    s3 =  ((a[4]&0x40)>>6) + ((a[4]&0x10)>>3) + (a[4]&0x04) + ((a[4]&0x01)<<3);
    s4 =  ((a[5]&0x40)>>6) + ((a[5]&0x10)>>3) ;

//    PRINTF("page_tens == %x  page_unit == %x \n",page_tens,page_unit);

    head->page = (page_tens<<4) + page_unit;
//    PRINTF("page == %x  \n",head->page);
    head->s1= s1;
    head->s2= s2;
    head->s3= s3;
    head->s4= s4;
    head->control_bits=((a[3]&0x01)<<4) +((a[5]&0x04)<<3)+((a[5]&0x01)<<6)+((a[6]&0x40)<<1)
                       +((a[6]&0x10)<<4)  +((a[6]&0x04)<<7) +((a[6]&0x01)<<10)+((a[7]&0x40)<<5)
                       +((a[7]&0x10)<<10) +((a[7]&0x04)<<11) +((a[7]&0x01)<<12) ;

}

static INLINE UINT8 x_packet_30(UINT8 *in, struct header *head)
{
    UINT8 page_unit = 0;
    UINT8 page_tens = 0;
    //UINT8 s1;
   // UINT8 s2;
   // UINT8 s3;
   // UINT8 s4;
    UINT8 m1 = 0;
    UINT8 m2 = 0;
    UINT8 m3 = 0;
    UINT8 mag = 0;
    UINT8 i = 0;
    UINT8 a[8]={0,0,0,0,0,0,0,0};

    for(i=0;i<8;i++)
    {
        a[i]=hamming_8_4(in[i]);
    }

    //libc_printf("a[0]=0x%x,a[1]=0x%x\n",a[0],a[1]);
    //same as page header
    page_unit = ((a[0]&0x40)>>6) +((a[0]&0x10)>>3)+(a[0]&0x04)+((a[0]&0x01)<<3);
    page_tens = ((a[1]&0x40)>>6) +((a[1]&0x10)>>3)+(a[1]&0x04)+((a[1]&0x01)<<3);

    m1 = a[3]&0x01;
    m2 = a[5]&0x04;
    m3 = a[5]&0x01;
    mag = (m1)+(m2<<1)+(m3<<2);
    //libc_printf("page_tens == %x  page_unit == %x,\n",page_tens,page_unit);
    head->page = (page_tens<<4) + page_unit;

    return mag;

}

static INLINE UINT16 x_link(UINT8 *in ,UINT8 cur_mag)
{
    UINT8 mag = 0;
    UINT8 page_unit = 0;
    UINT8 page_tens = 0;
//    UINT8 s1 = 0;
//    UINT8 s2 = 0;
//    UINT8 s3 = 0;
//    UINT8 s4 = 0;
    UINT16 page_id = 0;
    UINT8 m1 = 0;
    UINT8 m2 = 0;
    UINT8 m3 = 0;
    UINT8 i = 0;
    UINT8 a[6]={0,0,0,0,0,0};

    for(i=0;i<6;i++)
    {
        a[i]=hamming_8_4(in[i]);
    }

    page_unit = ((a[0]&0x40)>>6)+((a[0]&0x10)>>3)+(a[0]&0x04)+((a[0]&0x01)<<3);
    page_tens = ((a[1]&0x40)>>6)+((a[1]&0x10)>>3)+(a[1]&0x04)+((a[1]&0x01)<<3);

    m1 = (a[3]&0x01);
    m2 = (a[5]&0x04)>>2;
    m3 = (a[5]&0x01);

    mag=(m3<<2) +(m2<<1) +(m1) ;//m3m2m1
    //mag= (mag+cur_mag)%8;
    /*if(mag>=cur_mag)
        mag -= cur_mag;
    else
        mag = cur_mag-mag;*/
    mag = mag^cur_mag;


//    s1 =  ((a[2]&0x40)>>6) + ((a[2]&0x10)>>3) + (a[2]&0x04) + ((a[2]&0x01)<<3);
//    s2 =  ((a[3]&0x40)>>6) + ((a[3]&0x10)>>3) + (a[3]&0x04);
//    s3 =  ((a[4]&0x40)>>6) + ((a[4]&0x10)>>3) + (a[4]&0x04) + ((a[4]&0x01)<<3);
//    s4 =  ((a[5]&0x40)>>6) + ((a[5]&0x10)>>3) ;

    page_id = hex2dec((mag<<8) + (page_tens<<4) + page_unit);  // 1-8
    //VBI_PRINTF("link page %d of page %d  =  %d \n",n, cb->page_id, page_id);
    return page_id;

}

static INLINE INT32  ttx_mem_cpy(UINT8 *dst,UINT8 *src,INT32  len)
{
    INT32 dw_count = 0;
    INT32 ret = 0;

     // Need not updata time
    if(TTX_NORMAL_0==MEMCMP(last_temp_data2,src,TTX_NORMAL_8))
    {
        return ret;
    }
    else                                            // Update time
    {
        MEMCPY(last_temp_data2,src,8);
    }
    //for(dw_count=0;dw_count<len;dw_count++)
    dw_count=0;
    while(dw_count<len)
    {
        if(*(dst+dw_count) != *(src+dw_count))
        {
            *(dst+dw_count) = *(src+dw_count);
            ret = -1;
        }
        dw_count++;
    }
    return ret;
}


#if 0
// this table generates the parity checks for hamm24/18 decoding.
// bit 0 is for test A, 1 for B, ...
// thanks to R. Gancarz for this fine table *g*

static char hamm24par[3][256] =
{
    { // parities of first byte
     0, 33, 34,  3, 35,  2,  1, 32, 36,  5,  6, 39,  7, 38, 37,  4,
    37,  4,  7, 38,  6, 39, 36,  5,  1, 32, 35,  2, 34,  3,  0, 33,
    38,  7,  4, 37,  5, 36, 39,  6,  2, 35, 32,  1, 33,  0,  3, 34,
     3, 34, 33,  0, 32,  1,  2, 35, 39,  6,  5, 36,  4, 37, 38,  7,
    39,  6,  5, 36,  4, 37, 38,  7,  3, 34, 33,  0, 32,  1,  2, 35,
     2, 35, 32,  1, 33,  0,  3, 34, 38,  7,  4, 37,  5, 36, 39,  6,
     1, 32, 35,  2, 34,  3,  0, 33, 37,  4,  7, 38,  6, 39, 36,  5,
    36,  5,  6, 39,  7, 38, 37,  4,  0, 33, 34,  3, 35,  2,  1, 32,
    40,  9, 10, 43, 11, 42, 41,  8, 12, 45, 46, 15, 47, 14, 13, 44,
    13, 44, 47, 14, 46, 15, 12, 45, 41,  8, 11, 42, 10, 43, 40,  9,
    14, 47, 44, 13, 45, 12, 15, 46, 42, 11,  8, 41,  9, 40, 43, 10,
    43, 10,  9, 40,  8, 41, 42, 11, 15, 46, 45, 12, 44, 13, 14, 47,
    15, 46, 45, 12, 44, 13, 14, 47, 43, 10,  9, 40,  8, 41, 42, 11,
    42, 11,  8, 41,  9, 40, 43, 10, 14, 47, 44, 13, 45, 12, 15, 46,
    41,  8, 11, 42, 10, 43, 40,  9, 13, 44, 47, 14, 46, 15, 12, 45,
    12, 45, 46, 15, 47, 14, 13, 44, 40,  9, 10, 43, 11, 42, 41,  8
    }, { // parities of second byte
     0, 41, 42,  3, 43,  2,  1, 40, 44,  5,  6, 47,  7, 46, 45,  4,
    45,  4,  7, 46,  6, 47, 44,  5,  1, 40, 43,  2, 42,  3,  0, 41,
    46,  7,  4, 45,  5, 44, 47,  6,  2, 43, 40,  1, 41,  0,  3, 42,
     3, 42, 41,  0, 40,  1,  2, 43, 47,  6,  5, 44,  4, 45, 46,  7,
    47,  6,  5, 44,  4, 45, 46,  7,  3, 42, 41,  0, 40,  1,  2, 43,
     2, 43, 40,  1, 41,  0,  3, 42, 46,  7,  4, 45,  5, 44, 47,  6,
     1, 40, 43,  2, 42,  3,  0, 41, 45,  4,  7, 46,  6, 47, 44,  5,
    44,  5,  6, 47,  7, 46, 45,  4,  0, 41, 42,  3, 43,  2,  1, 40,
    48, 25, 26, 51, 27, 50, 49, 24, 28, 53, 54, 31, 55, 30, 29, 52,
    29, 52, 55, 30, 54, 31, 28, 53, 49, 24, 27, 50, 26, 51, 48, 25,
    30, 55, 52, 29, 53, 28, 31, 54, 50, 27, 24, 49, 25, 48, 51, 26,
    51, 26, 25, 48, 24, 49, 50, 27, 31, 54, 53, 28, 52, 29, 30, 55,
    31, 54, 53, 28, 52, 29, 30, 55, 51, 26, 25, 48, 24, 49, 50, 27,
    50, 27, 24, 49, 25, 48, 51, 26, 30, 55, 52, 29, 53, 28, 31, 54,
    49, 24, 27, 50, 26, 51, 48, 25, 29, 52, 55, 30, 54, 31, 28, 53,
    28, 53, 54, 31, 55, 30, 29, 52, 48, 25, 26, 51, 27, 50, 49, 24
    }, { // parities of third byte
    63, 14, 13, 60, 12, 61, 62, 15, 11, 58, 57,  8, 56,  9, 10, 59,
    10, 59, 56,  9, 57,  8, 11, 58, 62, 15, 12, 61, 13, 60, 63, 14,
     9, 56, 59, 10, 58, 11,  8, 57, 61, 12, 15, 62, 14, 63, 60, 13,
    60, 13, 14, 63, 15, 62, 61, 12,  8, 57, 58, 11, 59, 10,  9, 56,
     8, 57, 58, 11, 59, 10,  9, 56, 60, 13, 14, 63, 15, 62, 61, 12,
    61, 12, 15, 62, 14, 63, 60, 13,  9, 56, 59, 10, 58, 11,  8, 57,
    62, 15, 12, 61, 13, 60, 63, 14, 10, 59, 56,  9, 57,  8, 11, 58,
    11, 58, 57,  8, 56,  9, 10, 59, 63, 14, 13, 60, 12, 61, 62, 15,
    31, 46, 45, 28, 44, 29, 30, 47, 43, 26, 25, 40, 24, 41, 42, 27,
    42, 27, 24, 41, 25, 40, 43, 26, 30, 47, 44, 29, 45, 28, 31, 46,
    41, 24, 27, 42, 26, 43, 40, 25, 29, 44, 47, 30, 46, 31, 28, 45,
    28, 45, 46, 31, 47, 30, 29, 44, 40, 25, 26, 43, 27, 42, 41, 24,
    40, 25, 26, 43, 27, 42, 41, 24, 28, 45, 46, 31, 47, 30, 29, 44,
    29, 44, 47, 30, 46, 31, 28, 45, 41, 24, 27, 42, 26, 43, 40, 25,
    30, 47, 44, 29, 45, 28, 31, 46, 42, 27, 24, 41, 25, 40, 43, 26,
    43, 26, 25, 40, 24, 41, 42, 27, 31, 46, 45, 28, 44, 29, 30, 47
    }
};

// table to extract the lower 4 bit from hamm24/18 encoded bytes

static char hamm24val[256] =
{
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15,
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15
};



// mapping from parity checks made by table hamm24par to error
// results return by hamm24.
// (0 = no error, 0x0100 = single bit error, 0x1000 = double error)

static short hamm24err[64] =
{
    0x0000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
};

// mapping from parity checks made by table hamm24par to faulty bit
// in the decoded 18 bit word.

static int hamm24cor[64] =
{
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00001, 0x00000, 0x00002, 0x00004, 0x00008,
    0x00000, 0x00010, 0x00020, 0x00040, 0x00080, 0x00100, 0x00200, 0x00400,
    0x00000, 0x00800, 0x01000, 0x02000, 0x04000, 0x08000, 0x10000, 0x20000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

int
hamm24(UINT8 *p, int *err)
{
    int e;
    int x;
    UINT8 tmp, tmp1, i, j;

    for(j = 0; j < 3; j++)
    {
        tmp1 = *(p+j);
        tmp = 0;
        for(i = 0; i < 8; i++)
            tmp |= (((tmp1 >> (7-i))&1)<<i);
        *(p+j) = tmp;
    }

    e = hamm24par[0][p[0]] ^ hamm24par[1][p[1]] ^ hamm24par[2][p[2]];
    x = hamm24val[p[0]] + p[1] % 128 * 16 + p[2] % 128 * 2048;

    *err += hamm24err[e];
    return x ^ hamm24cor[e];
}
#endif

UINT16 get_first_ttx_page(void)
{
    return first_ttx_page;
}

UINT16 get_inital_page(void)//cloud
{
    return g_inital_page;
}

UINT8 get_inital_page_status(void)
{
    return inital_page_status;
}

BOOL get_b_save_subpage(void)
{
    return b_save_subpage;
}

UINT32 get_packet26_nation_map(UINT16 page, UINT16 line, UINT16 i)
{//use this function to replace GET_P26_NATION_MAP to avoid bug, cloud
    UINT32 pos = 0;
    UINT32 ret = 0;

    if((TTX_PAGE_MIN_NUM > page)|| (TTX_PAGE_MAX_NUM < page))
    {
        return 0;
    }
    pos = ((page-100)*(25*20)+line*20+(i>>1));

    ret = ((g_ttx_p26_nation[pos]>>((i&1)*4))&0xf);
    return ret;
}

static void erase_unknown_packet1(UINT8 mag)
{
     UINT8 i = 0;

      //if((1==last_pCurWrCB[mag]->MagazineSerial)
      //&& (1==last_pCurWrCB[mag]->ErasePage) && (1==last_erase_bit[0]) &&
      //(last_pCurWrCB[mag]->page_id == last_erase_bit[1]))
      //{
      //    ;
      //}
      //else
      if(!((1==last_p_cur_wr_cb[mag]->magazine_serial)
             &&(1==last_p_cur_wr_cb[mag]->erase_page) && (1==last_erase_bit[0])
             &&(last_p_cur_wr_cb[mag]->page_id == last_erase_bit[1])))
      {
          for(i=1;i<24;i++)
          {
              if(0==g_packet_exist[mag][i])
              {
                  //if(last_pCurWrCB[temp_mag]->page_id==102)
                  //   libc_printf("g_packet_exist[temp_mag][%d]=%d\n",
                  //i,g_packet_exist[temp_mag][i]);
                  if(((0==last_p_cur_wr_cb[mag]->magazine_serial)&&(TTX_SPEC600_PAGE_ID!=last_p_cur_wr_cb[mag]->page_id))
                        || (1==last_p_cur_wr_cb[mag]->magazine_serial))
                  {
                    //cloud
                      MEMSET(last_p_cur_wr_cb[mag]->buf_start+i*40, 0x20, 40);
                      p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
                  }
              }
          }
      }
}

static BOOL whether_to_update_ttx_page(UINT8 mag)
{
    UINT8 j =0;
    UINT8 i =0;
    BOOL ret= FALSE;

    for(i=1;i<24;i++)
    {
        if(1 != g_packet_exist[mag][i])
        {
            if((TRUE == g_vbi27_pconfig_par->parse_packet26_enable) && (TTX_PACKET_26==g_packet_exist[mag][i]))
            {
                g_packet_exist[mag][i]=1;
                j++;
            }
            else if(0==g_packet_exist[mag][i])
            {
                if((1==last_p_cur_wr_cb[mag]->erase_page)||(1==last_p_cur_wr_cb[mag]->subtitle))
                    MEMSET(last_p_cur_wr_cb[mag]->buf_start+i*40, 0x20, 40);//cloud
                //pCBPageUpdate(last_pCurWrCB[temp_mag]->page_id,i);
            }
        }
        else
        {
            j++;
        }
    }
    if(j>0)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    return ret;

}

static void whether_to_erase_page(UINT8 mag)
{
    UINT8 i = 0;

    for(i=1;i<24;i++)
    {
        if(1==g_packet_exist[mag][i])
        {
            erase_page=TRUE;
            break;
        }
    }
    if(TTX_MAX_ROW==i)
    {
        erase_page=FALSE;
    }
}


static void erase_unknown_packet(UINT8 mag)
{
     UINT8 i = 0;

     //if((1==last_pCurWrCB[mag]->MagazineSerial) &&
     //   (1==last_pCurWrCB[mag]->ErasePage) && (1==last_erase_bit[0]) &&
     //   (last_pCurWrCB[mag]->page_id == last_erase_bit[1]))
     //{
    //     ;
    // }
     //else
     if(!((1==last_p_cur_wr_cb[mag]->magazine_serial) &&
        (1==last_p_cur_wr_cb[mag]->erase_page) && (1==last_erase_bit[0]) &&
        (last_p_cur_wr_cb[mag]->page_id == last_erase_bit[1])))
     {
          for(i=1;i<24;i++)
          {
             if(0==g_packet_exist[mag][i])
             {
                 if((0!= last_p_cur_wr_cb[mag]->sub_page_id) || (1==last_p_cur_wr_cb[mag]->subtitle))//cloud
                 {
                      MEMSET(last_p_cur_wr_cb[mag]->buf_start+i*40, 0x20, 40);
                      p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
                 }
              }
           }
     }
}

static void ttx_update_page(UINT8 mag)
{
    UINT8 i = 0;
    UINT32 tmp_tick = 0;
    struct PBF_CB *tmp_cb = NULL;
    UINT32 last_tmp_tick  = 0;

    // only check current showing page
    if((last_p_cur_wr_cb[mag]->page_id == cur_page) && ( 0xffff==wait_page ))
    {
        tmp_tick = osal_get_tick();
        if((RET_SUCCESS == pbf_wr_req(last_p_cur_wr_cb[mag]->page_id, &tmp_cb))&& (NULL!= tmp_cb))
        {
            last_tmp_tick = tmp_cb->page_update_tick;

            if(1 != last_p_cur_wr_cb[mag]->subtitle)
            { // Now we only check normal ttx
                if(TTX_PAGE_CHANGE_TIME > (tmp_tick - last_tmp_tick))
                {
                    page_update_too_quick = TRUE;
                }
                else
                {
                    page_update_too_quick = FALSE;
                }
               // if(page_update_too_quick == TRUE)
               // {
               //     ;
                   // if page update too quick we do nothing.
              //  }
                if(1)//(FALSE == page_update_too_quick)
                {
                    for(i=1;i<24;i++)
                    {
                         p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
                    }
                 }
                 tmp_cb->page_update_tick = tmp_tick;
              }
              else
              {
                for(i=1;i<24;i++)
                {
                    p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
                }
              }
        }
        else
        {
           for(i=1;i<24;i++)
           {
               p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
           }
        }
     }
     else
     {
         for(i=1;i<24;i++)
         {
            p_cbpage_update(last_p_cur_wr_cb[mag]->page_id,i);
         }
     }
}

static void parse_ttx_time_filling(UINT8 mag,UINT8 *ptr,UINT8 *data,UINT8 *temp_data)
{
    UINT8 i = 0;
    UINT8 tmp = 0;

    if((0xffff != rd_page) && (0 == p_cur_wr_cb[mag]->suppress_header) && (0 == p_cur_wr_cb[mag]->interrupted_sequence) )
        //&& pCurWrCB[mag]->ErasePage!=1)
    {
        if((1==transfer_mode) || ((0==transfer_mode) && (1==mag)))
        {
            ptr = p_dec_rd_cb->buf_start+32;
#ifndef KAON_TTX_TIME
            for(i=0;i<8;i++)
            {
                temp_data[i] = odd_dec(*(data +32+i));
            }
             // ':'error position adjust
            if((':' != temp_data[2])&& (':' == temp_data[5]))
            {
                tmp = temp_data[2];
                temp_data[2] = temp_data[5];
                temp_data[5] = tmp;
            }
             // ':'error position adjust
            //if(temp_data[2]!=':' || temp_data[5]!=':')
            //{
            //    temp_data[2] = temp_data[5] = ':';
            //}
#else
            set_kaon_ttx_time(temp_data);
#endif

        if(ttx_mem_cpy(ptr,temp_data,TTX_NORMAL_8) != TTX_NORMAL_0)
        {
             // Last byte of temp_data is space and
             //last_temp_data[7] contain valid data
              if((0x20==temp_data[7]) && (0xff!=last_temp_data[7]))
              {
                  #if 0
                  // Calculate data will be shown
                  if( 0x39== last_temp_data[7])
                      temp_data[7] = 0x30;
                  else
                      temp_data[7] = last_temp_data[7] + 1;
                  ptr[7] = temp_data[7];
                 #else  //mask flicker
                 temp_data[7]=last_temp_data[7];
                 ptr[7] = temp_data[7];
                 #endif
              }
              // Backup cur temp_data for next use
             MEMCPY(last_temp_data,temp_data,8);
             if(p_cbpage_update!=NULL)
             {
                if(0 == p_dec_rd_cb->subtitle)
                {
                     p_cbpage_update(p_dec_rd_cb->page_id,0x00);
                }
             }
             //if(pCBPageUpdate!=NULL)
                 //pCBPageUpdate(pDecRdCB->page_id,0x00);
           }
        }
    }
}

static void ttx_set_cur_page_cb(UINT8 mag,struct header *head,UINT16 page_id,UINT16 sub_page_id)
{
    p_cur_wr_cb[mag]->sub_page_id = sub_page_id;//cloud
    p_cur_wr_cb[mag]->page_id = page_id;
    p_cur_wr_cb[mag]->erase_page= (0x0010==(head->control_bits & 0x0010))?0x01:0x00;
    p_cur_wr_cb[mag]->update_indicator= (0x0100==(head->control_bits & 0x0100))?0x01:0x00;
    p_cur_wr_cb[mag]->nation_option= (head->control_bits)>>12 ;
    p_cur_wr_cb[mag]->suppress_header = (0x0080==(head->control_bits & 0x0080))?0x01:0x00;
    p_cur_wr_cb[mag]->inhibit_display = (0x0400==(head->control_bits & 0x0400))?0x01:0x00;
    p_cur_wr_cb[mag]->magazine_serial = (0x0800==(head->control_bits & 0x0800))?0x01:0x00;
    p_cur_wr_cb[mag]->newsflash= (0x0020==(head->control_bits & 0x0020))?0x01:0x00;
    p_cur_wr_cb[mag]->subtitle= (0x0040==(head->control_bits & 0x0040))?0x01:0x00;
    p_cur_wr_cb[mag]->interrupted_sequence= (0x0200==(head->control_bits & 0x0200))?0x01:0x00;
    p_cur_wr_cb[mag]->g0_set = g0_set[mag];
    p_cur_wr_cb[mag]->second_g0_set = second_g0_set[mag];
}

// added for clean ttx_subt line that is not needed

#ifdef PRINTF_PAGE_PACKET
static void parse_ttx_data_debug1(UINT8 packet,UINT8 mag)
{
    if((packet !=TTX_PACKET_0) && (mag<TTX_TOTAL_MAGZINE) &&
        (last_p_cur_wr_cb[mag]!=NULL) && (last_p_cur_wr_cb[mag]->page_id==g_debug_page_id))
    {
        libc_printf("P%d, ", packet);
        if(g_debug_line == packet)
        {
            soc_printf(" ");//for debug line of special page
        }
    }
}

static void parse_ttx_data_debug2(UINT16 page_id)
{
     if(page_id == g_debug_page_id)
     {
        libc_printf("\nPg %d S[%d]:", page_id, transfer_mode);
     }
}

static void parse_ttx_data_debug3(UINT8 mag,UINT8 row,UINT8 col,UINT8 p26_data)
{
    if(g_debug_p26&&(last_p_cur_wr_cb[mag]->page_id==g_debug_page_id))
    {
        libc_printf("G3(%d,%d:%d),",row,col,p26_data);
    }
}

static void parse_ttx_data_debug4(UINT8 mag,UINT8 row,UINT8 col,UINT8 p26_data)
{
    if(g_debug_p26&&(last_p_cur_wr_cb[mag]->page_id==g_debug_page_id))
    {
         libc_printf("G2(%d,%d:%d),",row,col,p26_data);
    }
}

static void parse_ttx_data_debug5(UINT8 mag,UINT8 row,UINT8 col,UINT8 p26_data)
{
    if(g_debug_p26&&(last_p_cur_wr_cb[mag]->page_id==g_debug_page_id))
    {
        libc_printf("DM(%d,%d:%d),",row,col,p26_data);
    }
}
#endif

static void ttx_parse_set_b_showpage(UINT8 mag,UINT8 page_number,UINT8 subtitle)
{
   UINT8 i = 0;
   UINT8 j = 0;

   if(1 == transfer_mode)
   {
       if((page_number!=last_page_number[mag]) || (mag!=last_mag) ||(1==subtitle))
       {
           if((0xf!=last_mag) && (1==page_start[last_mag]))
           {
               page_start[last_mag]=0;
               b_show_page=TRUE;
           }
       }
       page_start[mag]=1;//begin to parse a new page
   }
   else
   {
       if(0xf==last_mag)
       {
           page_start[mag]=1;
       }
       else if((mag!=last_mag) && (1 == last_transfer_mode))
       {
           for(i=0,j=0;i<25;i++)
           {
               if(1==g_packet_exist[last_mag][i])
               {
                   j++;
               }
           }
           //if last mag page have 1 line data,
           //the page should be showed. guoping fix
           //fulan bug 2008-10-30,stream file
           //12633H22000 K-TV TTX 29E can not display *.ts
           if(TTX_NORMAL_2 <= j)
           {
               b_show_page=TRUE;
               b_change_mag =TRUE;
           }
       }
       else if(/*page_number!=last_page_number[mag]&&*/(1==page_start[mag]) && (0xff!=last_page_number[mag]))
       {
           page_start[mag]=1;//old page end,begin to receive new page
           if(((0==last_page_number[mag]) ||( 0xff==last_page_number[mag]))&& (0==last_packet[mag]))
           {
               b_show_page=FALSE;
           }
           else
           {
               b_show_page=TRUE;
           }
       }
       else if((page_number==last_page_number[mag]) && (1==page_start[mag])&& (1==subtitle))
       {
           b_show_page=TRUE;
       }
       page_start[mag]=1;
   }
}

static void ttx_display_last_page(UINT8 temp_mag,UINT8 mag)
{//display last page
    BOOL ret = 0;
    UINT8 i = 0;

    if(1 == transfer_mode)
    {
        temp_mag=last_mag;
    }
    else
    {
        temp_mag=mag;
    }
    if((TRUE==b_change_mag) && (0==transfer_mode))
    {
        temp_mag=last_mag;
        b_change_mag = FALSE;
    }
    if(0==last_p_cur_wr_cb[temp_mag]->valid)
    {
        page_transfered_num++;//cloud
    }
    last_p_cur_wr_cb[temp_mag]->valid = 1;
    // move here from pbf_wr_req(),
    //to make sure cb of the page finish.
    last_p_cur_wr_cb[temp_mag]->complement_line = 0;
    if(NULL!=p_cbpage_update)
    {

        if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
        {
            b_save_subpage=TRUE;
        }

        ret = whether_to_update_ttx_page(temp_mag);

        if(TRUE == ret)
        {
            ttx_update_page(temp_mag);
        }

        if(1==last_p_cur_wr_cb[temp_mag]->pack24_exist)
        {
            p_cbpage_update(last_p_cur_wr_cb[temp_mag]->page_id,24);
        }
        for(i=1;i<24;i++)
        {
            if(1==g_packet_exist[temp_mag][i])
            {
                erase_page=TRUE;
                break;
            }
        }
        if(TTX_MAX_ROW==i)
        {
            erase_page=FALSE;
        }
        if((TRUE == g_vbi27_pconfig_par->erase_unknown_packet)
            && (TRUE == erase_page) && (1!=last_p_cur_wr_cb[temp_mag]->subtitle))
        {
            erase_unknown_packet1(temp_mag);
        }
        if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
        {
            b_save_subpage=FALSE;
        }
    }
    b_show_page=FALSE;
    if(last_p_cur_wr_cb[temp_mag]->subtitle!=1)
    {
        last_erase_bit[0]= last_p_cur_wr_cb[temp_mag]->erase_page;
        last_erase_bit[1]= last_p_cur_wr_cb[temp_mag]->page_id;
    }
}

static void ttx_avoid_page_refresh_quick(UINT8 mag)
{
    UINT8 i =0;

    if((TRUE == g_vbi27_pconfig_par->user_fast_text) &&
        (0 ==p_cur_wr_cb[mag]->pack24_exist))
    {
        if(( 0x01==p_cur_wr_cb[mag]->erase_page )&&
            ((FALSE == g_vbi27_pconfig_par->erase_unknown_packet)||(1==p_cur_wr_cb[mag]->subtitle)))
        {//avoid refresh too quickly
            for(i=0;i<24;i++)
            {
                if(p_cbpage_update!=NULL)
                {
                    p_cbpage_update(p_cur_wr_cb[mag]->page_id,i);
                }
            }
        }
        else
        {
            if(p_cbpage_update!=NULL)
            {
                p_cbpage_update(p_cur_wr_cb[mag]->page_id,0);
            }
        }
    }
    else
    {
        //in case page that need Erase, for avoid flash,
        //show not erase osd directly only need reset page memory

        //if(( 0x01==pCurWrCB[mag]->ErasePage )&&
        //(/*(g_vbi27_pconfig_par->erase_unknown_packet == FALSE)||*/
        //  (1==pCurWrCB[mag]->Subtitle)))
        if(( 0x01==p_cur_wr_cb[mag]->erase_page )&&(1==p_cur_wr_cb[mag]->subtitle))
        {
            for(i=0;i<25;i++)
            {
                if(p_cbpage_update!=NULL)
                {
                    p_cbpage_update(p_cur_wr_cb[mag]->page_id,i);
                }
            }
        }
        else
        {
            if(p_cbpage_update!=NULL)
            {
                p_cbpage_update(p_cur_wr_cb[mag]->page_id,0);
            }
        }
    }
}

static void ttx_save_last_page_data(UINT8 mag,UINT8 page_number)
{
    UINT8 i = 0;

    if(( 0x01==p_cur_wr_cb[mag]->erase_page ) || (0x01==p_cur_wr_cb[mag]->subtitle )
        ||(last_page_id[mag] != p_cur_wr_cb[mag]->page_id) )
    {
        for(i=0;i<25;i++)
        {
            g_packet_exist[mag][i]=0;
        }
    }
    p_cur_wr_cb[mag]->pack24_exist = 0;
    last_page_number[mag] = page_number;//cloud
    last_p_cur_wr_cb[mag] = p_cur_wr_cb[mag];//cloud
    last_mag = mag;//cloud
    last_page_id[mag]= p_cur_wr_cb[mag]->page_id;
    last_sub_page_id[mag]=p_cur_wr_cb[mag]->sub_page_id;
    last_transfer_mode =transfer_mode;
    //for(i=0;i<25;i++)
    //    g_packet_exist[mag][i]=0;
}

static void parse_ttx_time_filling1(UINT8 mag,struct header *head,
    UINT8 *temp_data,UINT8 *data)
{
    UINT8 suppress_header = 0;
    UINT8 *ptr = NULL;
    UINT8 i = 0;
    UINT8 tmp = 0;

    suppress_header = (0x0080 == (head->control_bits & 0x0080))?0x01:0x00;
   // eras_page = ((head->control_bits & 0x0010)==0x0010)?0x01:0x00;//cloud
    if((0 == p_dec_rd_cb->suppress_header) && (0 == suppress_header) )//&& 0 == ErasePage)
    {
        if((1==transfer_mode) || ((0==transfer_mode) && (1==mag)))
        {
            ptr = p_dec_rd_cb->buf_start+32;
#ifndef KAON_TTX_TIME
            for(i=0;i<8;i++)
            {
                temp_data[i] = odd_dec(*(data +32+i));
            }

            // ':'error position adjust
            if((':' != temp_data[2]) && (':' == temp_data[5]))
            {
                tmp = temp_data[2];
                temp_data[2] = temp_data[5];
                temp_data[5] = tmp;
            }
              // ':'error position adjust
            //if(temp_data[2]!=':' || temp_data[5]!=':')

            //{

                //temp_data[2] = temp_data[5] = ':';

            //}
#else
            set_kaon_ttx_time(temp_data);
#endif
            if(ttx_mem_cpy(ptr,temp_data,TTX_NORMAL_8) != TTX_NORMAL_0)
            {
                // Last byte of temp_data is space and
                // last_temp_data[7] contain valid data
                // Calculate data will be shown
                if((0x20==temp_data[7]) && (0xff != last_temp_data[7]))
                {                            
                    if( 0x39==last_temp_data[7] )
                    {
                        temp_data[7] = 0x30;
                    }
                    else
                    {
                        temp_data[7] = last_temp_data[7] + 1;
                    }
                    ptr[7] = temp_data[7];
                }
                MEMCPY(last_temp_data,temp_data,8);
                // Backup cur temp_data for next use
                if(p_cbpage_update!=NULL)
                {
                    p_cbpage_update(p_dec_rd_cb->page_id,0x00);
                }
            }
        }
    }
}

static UINT8 ttx_set_show_page(struct header *head,UINT8 mag)
{
   //if( ( 0x3== head->s4) && (0xF== head->s3)&&(0x7==head->s2)&&(0xF== head->s1))
   //{
   //    return 0;// no FF-3F7F transmit
   //}
   //cloud
   transfer_mode = (0x0800 == (head->control_bits & 0x0800))?0x01:0x00;
   if(transfer_mode!=g_transfer_mode)
   {
       return 0;
   }
   if((1== page_start[mag]) && (0 == transfer_mode))
   {
       b_show_page=TRUE;
   }
   else if((1 == page_start[last_mag]) && (1 == transfer_mode))
   {
       b_show_page=TRUE;
   }
   else
   {
       b_show_page=FALSE;
   }
   return 1;
}

static void ttx_display_last_page1(UINT8 temp_mag,UINT8 mag)
{  //display last page
    BOOL ret = 0;
    UINT8 i = 0;

   // BOOL ttx_subt_update = FALSE;
    if(1 == transfer_mode)
    {
        temp_mag=last_mag;
    }
    else
    {
        temp_mag=mag;
    }
    if(0==last_p_cur_wr_cb[temp_mag]->valid)
    {
        page_transfered_num++;//cloud
    }
    last_p_cur_wr_cb[temp_mag]->valid = 1;
    // move here from pbf_wr_req(),
    //to make sure cb of the page finish.
    last_p_cur_wr_cb[temp_mag]->complement_line = 0;
    if(p_cbpage_update!=NULL)
    {
        if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
        {
            b_save_subpage=TRUE;
        }

        ret = whether_to_update_ttx_page(temp_mag);

        //if((TRUE ==ret)&&
        //    ((1==last_p_cur_wr_cb[temp_mag]->update_indicator )
         //   ||(1==last_p_cur_wr_cb[temp_mag]->subtitle)))
         if((TRUE==ret)&&((0!=last_p_cur_wr_cb[temp_mag]->sub_page_id)||(1==last_p_cur_wr_cb[temp_mag]->subtitle)||((last_p_cur_wr_cb[temp_mag]->page_id == cur_page) && (wait_page == 0xffff))))
        {
            for(i=1;i<24;i++)
            {
                p_cbpage_update(last_p_cur_wr_cb[temp_mag]->page_id,i);
            }
        }
        if(1==last_p_cur_wr_cb[temp_mag]->pack24_exist)
        {
            p_cbpage_update(last_p_cur_wr_cb[temp_mag]->page_id,24);
        }

        whether_to_erase_page(temp_mag);
        if((TRUE == g_vbi27_pconfig_par->erase_unknown_packet) && (erase_page==TRUE) &&
            (last_p_cur_wr_cb[temp_mag]->subtitle!=1))
        {
            erase_unknown_packet(temp_mag);
        }
        if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
        {
            b_save_subpage=FALSE;
            last_sub_page_id[temp_mag]= last_p_cur_wr_cb[temp_mag]->sub_page_id;
        }
        last_page_id[temp_mag]=last_p_cur_wr_cb[temp_mag]->page_id;
    }
    page_start[temp_mag]= 0; //as terminator,last page finished
    b_show_page = FALSE;
    if(last_p_cur_wr_cb[temp_mag]->subtitle!=1)
    {
        last_erase_bit[0]= last_p_cur_wr_cb[temp_mag]->erase_page;
        last_erase_bit[1]= last_p_cur_wr_cb[temp_mag]->page_id;
    }
}

static UINT8 parse_ttx_packet26(UINT8 mag,UINT8 *data)
{
   UINT8 ham1 = 0;
   UINT8 ham2 = 0;
   UINT8 ham3 = 0;
   UINT8 p26_address = 0;
   UINT8 p26_mode = 0;
   UINT8 p26_data = 0;
   UINT8 p26_nation = 0;
   UINT8 row = 0;
   UINT8 col=0;
   UINT8 i = 0;
   UINT8 j = 0;

   if((mag<=TTX_MAX_MAGZINE_NUM )&& (1 == page_start[mag]) &&
      (1==page_rec_data[mag]))// && packet >= last_line[mag])
   {
       if((mag != last_mag) && (1==transfer_mode))
       {
           return 0 ;
       }
       #ifdef PRINTF_PAGE_PACKET
       if(g_disable_p26&&(last_p_cur_wr_cb[mag]->page_id==g_debug_page_id))
       {
           return 0;
       }
       #endif
       for(i=1,j=0;i<=24;i++)//to fix bug in 29_07_04_TVC_TS.ts
       {
           if(1==g_packet_exist[mag][i])
           {
               j++;
           }
       }
        if((1==j) && (1==g_packet_exist[mag][j]))
        {
            g_packet_exist[mag][j]=0;
        }

       for(i = 0; i < 13; i++)
       {
            hamming_24_18(*(data+1+i*3),*(data+1+i*3+1),*(data+1+i*3+2),&ham1,&ham2,&ham3);
            p26_address = ((ham1&0x20)>>5) + ((ham1&0x08)>>2) +(ham1&0x04) + ((ham1&0x02)<<2)
                                + ((ham2&0x80)>>3) + ((ham2&0x40)>>1);
            p26_mode = ((ham2&0x20)>>5) + ((ham2&0x10)>>3) + ((ham2&0x08)>>1)
                         + ((ham2&0x04)<<1) + ((ham2&0x02)<<3);
            p26_data = ((ham3&0x80)>>7) + ((ham3&0x40)>>5) + ((ham3&0x20)>>3) + ((ham3&0x10)>>1)
                            + ((ham3&0x08)<<1) + ((ham3&0x04)<<3) + ((ham3&0x02)<<5);

            p26_data = p26_data&0x7F;//cloud
            if((0==i)&&(last_p_cur_wr_cb[mag]->page_id==last_p26_page_id)&& (b_packet26==TRUE)//cloud
            &&((TRUE == g_vbi27_pconfig_par->ttx_sub_page) && (last_p_cur_wr_cb[mag]->sub_page_id==last_p26_sub_page_id)))
            {
                col=last_col;
                row=last_row;
                b_packet26=FALSE;
            }
            if (p26_address < TTX_MAX_COLUMN)
            {
                col = p26_address;
                if(TTX_TRIPLET_MAX_NUM==i)
                {
                    last_row = row;
                    last_col = col;//cloud
                    //cloud
                    last_p26_page_id = last_p_cur_wr_cb[mag]->page_id;
                    if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
                    {
                        last_p26_sub_page_id =last_p_cur_wr_cb[mag]->sub_page_id;
                    }
                    b_packet26 = TRUE;
                }
                // col functions
                switch (p26_mode)
                {
                case 2://G3 set
                        //reuse font to save code size,cloud
                    if((VBI_PACKET26_VALUE_1==p26_data) || (VBI_PACKET26_VALUE_2==p26_data))
                    {
                        SET_P26_NATION_MAP(last_p_cur_wr_cb[mag]->page_id, row, col, 0xf);
                        if(VBI_PACKET26_VALUE_1==p26_data)
                        {
                            p26_data = 0x2d;
                        }
                        else
                        {
                            p26_data = 0x2e;
                        }
                        p26_data=p26_data&0x7F;//cloud
                        //g_ttx_p26_data[row*40+col] = p26_data;
                        *(last_p_cur_wr_cb[mag]->buf_start + row*40+col) =p26_data;
                        g_packet_exist[mag][row] = 26;
                        #ifdef PRINTF_PAGE_PACKET
                        parse_ttx_data_debug3(mag,row,col,p26_data);
                        #endif
                    }
                    break;
                case 15: // char from G2 set
                    if (row <= TTX_MAX_ROW)
                    {
                        if((p26_data < TTX_NORMAL_127) &&(row!=TTX_NORMAL_0))
                        {
                            //if(row!=0)//cloud
                            SET_P26_NATION_MAP(last_p_cur_wr_cb[mag]->page_id, row, col, 0xf);
                            p26_data=p26_data&0x7F;//cloud
                            //g_ttx_p26_data[row*40+col] = p26_data;
                            *(last_p_cur_wr_cb[mag]->buf_start + row*40+col) = p26_data;
                            g_packet_exist[mag][row] = 26;
                        }
                        #ifdef PRINTF_PAGE_PACKET
                        parse_ttx_data_debug4(mag,row,col,p26_data);
                        #endif
                    }
                    break;
                case 16:case 17:case 18:case 19:case 20:case 21:case 22:case 23://just for logiscope
                case 24:case 25:case 26:case 27:case 28:case 29:case 30:case 31:
                    // char from G0 set with diacritical mark
                    if ((row <= TTX_MAX_ROW) && (row!=TTX_NORMAL_0))
                    {
                        p26_nation = p26_mode - 16;//cloud
                        SET_P26_NATION_MAP(last_p_cur_wr_cb[mag]->page_id,row, col, p26_nation);//cloud
                        p26_data=p26_data|0x80;//cloud
                        //g_ttx_p26_data[row*40+col] = p26_data;//cloud
                        *(last_p_cur_wr_cb[mag]->buf_start + row*40+col) =p26_data;
                        g_packet_exist[mag][row] = 26;
                        #ifdef PRINTF_PAGE_PACKET
                        parse_ttx_data_debug4(mag,row,col,p26_data);
                        #endif
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                // row functions
                if (0 == (p26_address -= 40))
                {
                    p26_address = 24;
                }
                switch (p26_mode)
                {
                case 1: // full row color
                    row = p26_address;
                    break;
                case 4: // set active position
                    row = p26_address;
                    if(TTX_MAX_COLUMN > p26_data)
                    {
                        col = p26_data;//cloud
                    }
                    if(TTX_TRIPLET_MAX_NUM==i)
                    {
                        last_row = row;
                        last_col = col;//cloud
                        last_p26_page_id = last_p_cur_wr_cb[mag]->page_id;
                        if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
                        {
                            last_p26_sub_page_id = last_p_cur_wr_cb[mag]->sub_page_id;
                        }
                        b_packet26 = TRUE;
                    }
                    break;
                case 7: // address row 0 (+ full row color)
                    if(TTX_LAST_ROW_NUM == p26_address)
                    {
                        row = 0;
                    }
                    break;
                case 31:
                    //Termination Marker
                    if (TTX_LAST_ROW_NUM == p26_address)
                    {
                        last_packet[mag]=26;
                        return 0;
                    }
                    break;
                default:
                    break;
                }
            }
         }
   }
   return 1;
}

static UINT8 parse_ttx_packet28(UINT8 mag,UINT8 *data)
{
    UINT8 ham1 = 0;
    UINT8 ham2 = 0;
    UINT8 ham3 = 0;
    UINT8 designation = 0;

    if(1 == page_start[mag])
    {
        if((mag != last_mag) && (1==transfer_mode))
        {
            return 0;
        }
        ham1 = hamming_8_4(*data);
        designation = ((ham1&0x40)>>6) + ((ham1&0x10)>>3) + (ham1&0x04) + ((ham1&0x01)<<3);
        if((designation != TTX_NORMAL_0) && (designation != TTX_NORMAL_4))
        {
            return 0;
        }
        hamming_24_18(*(data+1),*(data+2),*(data+3),&ham1,&ham2,&ham3);
        last_p_cur_wr_cb[mag]->g0_set = ((ham2&0x10)>>4) + ((ham2&0x08)>>2)+((ham2&0x04))+((ham2&0x02)<<2)
                                     +((ham3&0x80)>>3)+((ham3&0x40)>>1)+((ham3&0x20)<<1);
        last_p_cur_wr_cb[mag]->second_g0_set = ((ham3&0x10)>>4)+((ham3&0x08)>>2)+((ham3&0x04)<<1)+((ham3&0x02)<<2);
        hamming_24_18(*(data+4),*(data+5),*(data+6),&ham1,&ham2,&ham3);
        last_p_cur_wr_cb[mag]->second_g0_set += ((ham1&0x20)>>1) + ((ham1&0x08)<<2) +((ham1&0x04)<<4);
    }
    return 1;
}

static UINT8 parse_ttx_packet29(UINT8 mag,UINT8 *data)
{
    UINT8 ham1 = 0;
    UINT8 ham2 = 0;
    UINT8 ham3 = 0;
    UINT8 designation = 0;

    if(1 == page_start[mag])
    {
       if((mag != last_mag) && (1==transfer_mode))
       {
            return 0 ;
       }
       ham1 = hamming_8_4(*data);
       designation = ((ham1&0x40)>>6) + ((ham1&0x10)>>3) + (ham1&0x04) + ((ham1&0x01)<<3);
       if((designation != TTX_NORMAL_0) && (designation != TTX_NORMAL_4))
       {
           return 0;
       }
       hamming_24_18(*(data+1),*(data+2),*(data+3),&ham1,&ham2,&ham3);
       last_p_cur_wr_cb[mag]->g0_set = ((ham2&0x10)>>4) + ((ham2&0x08)>>2) +((ham2&0x04))+((ham2&0x02)<<2)
                                       +((ham3&0x80)>>3)+((ham3&0x40)>>1)+((ham3&0x20)<<1);
       g0_set[mag] = last_p_cur_wr_cb[mag]->g0_set;
       last_p_cur_wr_cb[mag]->second_g0_set = ((ham3&0x10)>>4)+((ham3&0x08)>>2)+((ham3&0x04)<<1)+((ham3&0x02)<<2);
       hamming_24_18(*(data+4),*(data+5),*(data+6),&ham1,&ham2,&ham3);
       last_p_cur_wr_cb[mag]->second_g0_set += ((ham1&0x20)>>1) + ((ham1&0x08)<<2) +((ham1&0x04)<<4);
       second_g0_set[mag] = last_p_cur_wr_cb[mag]->second_g0_set;
    }
    return 1;
}

static UINT8 ttx_parse_head_para(UINT8 mag,UINT8 *data,struct header *head,
    UINT8 *page_number,UINT16 *page_id,UINT16 *sub_page_id,UINT8 *subtitle)
{
//    UINT8 i = 0;

    page_rec_data[mag] = 1;
    *page_number = hex2dec(head->page)-800;
    *page_id = hex2dec( (mag<<8) + head->page); //100 ~899,0~99->800~899
    if( (0xffff== *page_id)&&(head->page!=0xEE))
    {
        page_rec_data[mag] = 0;
        return 0;
    }
    //interrupted_sequence= ((head.control_bits & 0x0200)==0x0200)?0x01:0x00;
    *subtitle= (0x0040==(head->control_bits & 0x0040))?0x01:0x00;
    transfer_mode = (0x0800==(head->control_bits & 0x0800))?0x01:0x00;
    //1:serial mode---0:parallel mode
    g_transfer_mode = transfer_mode;
    #ifdef PRINTF_PAGE_PACKET
    parse_ttx_data_debug2(*page_id);
    #endif
    if(TRUE == g_vbi27_pconfig_par->ttx_sub_page)
    {
        *sub_page_id = hex2dec(((head->s2)<<4) + head->s1)-800;//cloud,0~79
        if(TTX_SUB_PAGE_MAX_NUM < *sub_page_id)
        {
            return 0;
        }
    }
    #if 0
    for(i=8;i<32;i++)
    {
        if(0x20 != odd_dec(*(data +i)))
        {
            break;
        }
    }
    //filter blank packet 0
    if((TTX_NORMAL_32==i) && (TTX_NORMAL_1 != *subtitle))
    {
        return 0;
    }
    #endif
    return 1;
}

static void ttx_color_link(UINT8 mag,UINT8 i,UINT16 link_page_id)
{
    switch(i)
    {
       case 0:
           last_p_cur_wr_cb[mag]->link_red =  link_page_id;
           break;
       case 1:
           last_p_cur_wr_cb[mag]->link_green= link_page_id;
           break;
       case 2:
           last_p_cur_wr_cb[mag]->link_yellow= link_page_id;
           break;
       case 3:
           last_p_cur_wr_cb[mag]->link_cyan = link_page_id;
           break;
       case 4:
           last_p_cur_wr_cb[mag]->link_next= link_page_id;
           break;
       case 5:
           last_p_cur_wr_cb[mag]->link_index = link_page_id;
           break;
       default:
            break;
    }
}

static UINT8 ttx_parse_packet_0(UINT8 mag,UINT8 *data)
{
    UINT8 i = 0;
    UINT8 func_ret = 0;
    UINT8 temp_mag=0;
    UINT16 page_id=0;
    UINT16 sub_page_id=0xffff; //cloud
    struct header head;
    UINT8 *ptr = NULL;
    UINT8 page_number = 0xff;//current page uints+page tens
    UINT8 subtitle  = 0;
    UINT8 temp_data[40]={0};

    if(mag>TTX_MAX_MAGZINE_NUM)
    {
        return 0;
    }
    x_page_cs(data,&head);
    if(head.page != 0xff)  //
    {
        func_ret = ttx_parse_head_para(mag,data,&head,&page_number,&page_id,&sub_page_id,&subtitle);
        if(0 == func_ret)
        {
            return 0;
        }
        ttx_parse_set_b_showpage(mag,page_number,subtitle);
        if(TRUE == b_show_page)
        {
            ttx_display_last_page(temp_mag,mag);
        }
        //mag0 <-> page 800-899
        if(RET_FAILURE == pbf_wr_req(page_id , &p_cur_wr_cb[mag]))
        {
            return 0;//ASSERT(0);
        }
        if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
        {
            MEMSET(g_ttx_p26_nation+(page_id-100)*(25*40/2),0x00,(25*40/2));//cloud
        }
        ttx_set_cur_page_cb(mag,&head,page_id,sub_page_id);
        if((0xff != g_inital_page)&&(p_cur_wr_cb[mag]->page_id == g_inital_page))
        {
            if(1==p_cur_wr_cb[mag]->subtitle)
            {
                inital_page_status = 1;//TTXENG_SUBT;
            }
            else
            {
                inital_page_status = 2;//TTXENG_TEXT;
            }
        }
        if(1 != p_cur_wr_cb[mag]->subtitle)
        {
            first_ttx_page = p_cur_wr_cb[mag]->page_id;
        }
        parse_ttx_time_filling(mag,ptr,data,temp_data);
        if((0x01 == p_cur_wr_cb[mag]->erase_page) &&
            ((0!=p_cur_wr_cb[mag]->sub_page_id)||(1==p_cur_wr_cb[mag]->subtitle)))
        {
            MEMSET(p_cur_wr_cb[mag]->buf_start, 0x20, 1000);
            p_cur_wr_cb[mag]->pack24_exist = 0;
            p_cur_wr_cb[mag]->complement_line = 0;
        }
        ptr = p_cur_wr_cb[mag]->buf_start;
        *ptr++ = 0x20;
        *ptr++ = (UINT8)(page_id/100) + 0x30;
        *ptr++ = (UINT8)((page_id%100)/10) + 0x30;
        *ptr++ = (UINT8)((page_id%100)%10) + 0x30;
        *ptr++ = 0x20;
        *ptr++ = 0x20;
        *ptr++ = 0x20;
        *ptr++ = 0x20;
        for(i=0;i<32;i++)
        {
            *ptr++ = odd_dec(*(data+8+i));
        }
        ttx_avoid_page_refresh_quick(mag);
        ttx_save_last_page_data(mag,page_number);
    }
    else //if(0xff == head.page)
    {
        page_rec_data[mag] = 0;
        #ifdef PRINTF_PAGE_PACKET
        void parse_ttx_data_debug2(page_id);
        #endif
        func_ret = ttx_set_show_page(&head,mag);
        if(0 == func_ret)
        {
            return 0;
        }
        if(TRUE == b_show_page)
        {
            ttx_display_last_page1(temp_mag,mag);
        }
        last_page_number[mag] = 0xff;//cloud
        last_mag = mag;//cloud
        if(0xffff== rd_page)
        {
            return 0;
        }
        parse_ttx_time_filling1(mag,&head ,temp_data,data);
    }
    return 1;
}

void parse_ttx_data(UINT8 *data) //
{
    UINT8 i = 0;
    //UINT8 temp_mag=0;
    UINT8 addr1 = 0;
    UINT8 addr2 = 0;
    UINT8 packet = 0;
    UINT8 mag = 0;
    UINT8 abs_mag = 0;
    UINT16 page_id=0;
    UINT16 link_page_id = 0;
    //UINT16 sub_page_id=0xffff; //cloud
    struct header head;
    UINT8 *ptr = NULL;
    //UINT8 temp_data[40]={0};
    UINT8 last_data[42]={0};//for repeated same packet of same mag.
    //UINT8 page_number = 0xff;//current page uints+page tens
    //UINT8 subtitle  = 0;
    struct PBF_CB *check_cb = NULL;
    UINT8 func_ret = 0;

    if(0 == MEMCMP(data,last_data,TTX_NORMAL_42))
    {
        return ;
    }
    else
    {
        MEMCPY(last_data,data,42);
    }
    addr1 = hamming_8_4(*data++); //
    addr2 = hamming_8_4(*data++); //
    //mag = x_mag(addr1,addr2); // make sure 0 -- 7
    mag = x_mag(addr1); // make sure 0 -- 7
    packet = x_packet(addr1,addr2);
    //////////////////////////////////////////////////////////////////////////
#ifdef PRINTF_PAGE_PACKET
    void parse_ttx_data_debug1(packet, mag);
#endif
    switch(packet)
    {
        case 0:
            func_ret = ttx_parse_packet_0(mag,data);
            if(0 == func_ret)
            {
                return;
            }
            break;
        case 27:
            if(1 == page_start[mag])
            {
                if((mag != last_mag) && (1==transfer_mode))
                {
                    return ;
                }
                data++;//designation = Ham_dec(*data++);
                for(i=0;i<6;i++)
                {
                    link_page_id = x_link(data,mag);
                    data += 6;
                    ttx_color_link(mag,i,link_page_id);
                }
            }
            break;
        case  1:case  2:case  3:case  4:case  5:case  6:case  7:case  8://just for logiscope
        case  9:case 10:case 11:case 12:case 13:case 14:case 15:case 16:
        case 17:case 18:case 19:case 20:case 21:case 22:case 23:case 24:
            if(mag>TTX_MAX_MAGZINE_NUM)
            {
                return;
            }
            if((1 == page_start[mag]) && (1==page_rec_data[mag]))// && packet >= last_line[mag])
            {
                if((mag != last_mag) && (1==transfer_mode))
                {
                    return ;
                }
                g_packet_exist[mag][packet]=1;//cloud
                if(TTX_MAX_ROW == packet)
                {
                    last_p_cur_wr_cb[mag]->pack24_exist = 1;
                }
                ptr = last_p_cur_wr_cb[mag]->buf_start + packet*40;
                for(i=0;i<40;i++)
                {
                    if ((TRUE == g_vbi27_pconfig_par->parse_packet26_enable) &&
                    (((1 == *(ptr+i)>>7)&&(0 != get_packet26_nation_map(last_p_cur_wr_cb[mag]->page_id, packet, i)))||
                    ((0 == *(ptr+i)>>7)&&(0xf == get_packet26_nation_map(last_p_cur_wr_cb[mag]->page_id, packet, i)))))
                    {
                        ;
                    }
                    else
                    {
                        *(ptr+i)= odd_dec(*(data+i));
                    }
                }
            }
            break;
       case 28:
            func_ret = parse_ttx_packet28(mag,data);
            if(0 == func_ret)
            {
                return;
            }
            break;
       case 29:
            func_ret = parse_ttx_packet29(mag,data);
            if(0 == func_ret)
            {
                return;
            }
            break;
        case 26:
            if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
            {
                func_ret = parse_ttx_packet26(mag,data);
                if(0 == func_ret)
                {
                    return;
                }
            }
            break;
        case 30:
            data++;
            abs_mag = x_packet_30(data,&head);
            if(0xff != head.page)
            {//100 ~899,0~99->800~899
                page_id = hex2dec( (abs_mag<<8) + head.page);
                if((0xffff== page_id) ||((page_id!=100) && (pbf_rd_req(page_id, &check_cb)!=RET_SUCCESS)) )
                {
                    return;
                }
                g_inital_page = page_id;
            }
            break;
        default:
            break;
    }
    if(TTX_PACKET_29 >= packet)
    {
        last_packet[mag]=packet;
    }
    return ;
}

INT32 ttx_m3327_request_page(struct vbi_device *dev, UINT16 page_id , struct PBF_CB **cb )
{
    INT32 ret = RET_FAILURE;
    if(NULL == dev)
    {
        return ret;
    }

    ret = pbf_rd_req(page_id, cb) ;

    if(RET_SUCCESS == ret)
    {
        p_dec_rd_cb = *cb;
        rd_page = p_dec_rd_cb->page_id;
    }
    return ret;
}
INT32 ttx_m3327_request_page_up(struct vbi_device *dev,UINT16 page_id , struct PBF_CB **cb )
{
    INT32 ret = RET_FAILURE;
    if(NULL == dev)
    {
        return ret;
    }

    ret =pbf_rd_req_up(page_id, cb);
    if(RET_SUCCESS == ret)
    {
        p_dec_rd_cb = *cb;
        rd_page = p_dec_rd_cb->page_id;
    }
    return ret;
    
}
INT32 ttx_m3327_request_page_down(struct vbi_device *dev, UINT16 page_id , struct PBF_CB **cb )
{
    INT32 ret = RET_FAILURE;

    if(NULL == dev)
    {
        return ret;
    }

    ret =pbf_rd_req_down(page_id, cb);
    if(RET_SUCCESS == ret)
    {
        p_dec_rd_cb = *cb;
        rd_page = p_dec_rd_cb->page_id;
    }
    return ret;
}
void ttx_m3327_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
    if(NULL == dev)
    {
        return;
    }
    g_default_g0_set = default_g0_set;
}

/*
    NAME             :     vbi_m3327_init_ttx_decoder(void)
    DESCRIPTION        :      init the teletext decoder
    PARAMETERS        :    NONE
    RETURN            :    void
*/
void vbi_m3327_init_ttx_decoder(void)
{
    int i = 0;

    p_cbpage_update = NULL;
    rd_page = 0xffff;

    for(i=0;i<8;i++)
    {
        page_start[i] = 0;
        g0_set[i] = g_default_g0_set<<3;
        second_g0_set[i]= 0;//g0_set[i];
        //last_line[i] = 0;
    }
    transfer_mode = 1;
    last_transfer_mode =1;
    last_mag = 0xf;//1;
    b_show_page=FALSE;//cloud
    g_inital_page = 0xFF;//cloud
    last_p26_page_id = 0;
    last_p26_sub_page_id = 0;
    page_transfered_num = 0; //cloud
    first_ttx_page = 0xff;
    ttx_subt_alarm_id = OSAL_INVALID_ID;
//    page_erase = FALSE;
//  last_tmp_tick = osal_get_tick();
}

void vbi_enable_ttx_by_osd(struct vbi_device* pdev)
{
    if(NULL == pdev)
    {
        return;
    }
    if(NULL == pdev->priv)
    {
        return;
    }
    struct vbi_m3327_private *priv = (struct vbi_m3327_private *)(pdev->priv);

    pdev->request_page = ttx_m3327_request_page;
    pdev->request_page_up = ttx_m3327_request_page_up;
    pdev->request_page_down = ttx_m3327_request_page_down;
    pdev->default_g0_set = ttx_m3327_default_g0_set;

    priv->ttx_by_osd = TRUE;
    priv->init_ttx_decoder = vbi_m3327_init_ttx_decoder;
}

void vbi_disable_ttx_by_osd(void)//add for 3101
{
    struct vbi_device *vbi_dev = NULL;
    struct vbi_m3327_private *priv = NULL;

    vbi_dev = (struct vbi_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_VBI);
    if(NULL==vbi_dev)
    {
        ASSERT(0);
        /* by saicheong for secure coding, BD-PB-NP-1 */
        libc_printf("critical: pointer parameter is NULL!(%s %d)\n", __FILE__, __LINE__);
        return ;
    }
    priv = (struct vbi_m3327_private *)(vbi_dev->priv);

    vbi_dev->request_page = NULL;
    vbi_dev->request_page_up = NULL;
    vbi_dev->request_page_down = NULL;
    vbi_dev->default_g0_set = NULL;

    priv->ttx_by_osd = FALSE;
    priv->init_ttx_decoder = NULL;
}
#endif
