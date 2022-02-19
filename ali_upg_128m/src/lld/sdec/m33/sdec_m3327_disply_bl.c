/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327_display_bl.c
*
*    Description: The file is to decode the data from the subtitle buffer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>
#include <hld/snd/snd.h>

#include "sdec_buffer.h"
#include "sdec_m3327.h"
#include "sdec_m3327_internal.h"

static UINT8 old_region_count = 0;
static UINT16 total_obj_cnt = 0;
static UINT16 drawn_obj_cnt = 0;
static UINT16 total_region_cnt = 0;
static UINT16 drawn_region_cnt = 0;
static BOOL b_auto_top_position_onoff = FALSE;
static UINT16 region_top_max = 576;
static UINT16 region_top_save = 50;
static UINT32 shifty_dvb_subt = 0;
static INT32 subt_shift_time=0;
static BOOL subt_shift_time_onoff = FALSE;
static BOOL g_background= FALSE;
static UINT8 by_value  =0;
static UINT8 bcr_value =0;
static UINT8 bcb_value =0;
static UINT8 bt_value  =0;
static BOOL g_char= FALSE;
static UINT8 by_value_char  =0;
static UINT8 bcr_value_char =0;
static UINT8 bcb_value_char =0;
static UINT8 bt_value_char  =0;
static BOOL is_last_obj_seg = FALSE;//only use in the function below
static UINT8 page_region_count = 0;
static OSAL_ID sdec_alarm_id = OSAL_INVALID_ID;
static UINT32 u_sdec_remain_size = 0;
static UINT8* pu_sdec_start_point = NULL ;

UINT32 page_finished_time=0;
UINT32 end_of_display_page_time=0;
void lib_subt_dvb_auto_top_pos_onoff(BOOL b_on_off,UINT16 us_top_max,UINT16 us_top_save)
{
	if((1080<us_top_max)||(1080<us_top_save))
	{
		return;
	}
    b_auto_top_position_onoff = b_on_off;
    region_top_max = us_top_max;
    region_top_save = us_top_save;
}

void lib_subt_dvb_shift_y_set(INT32 n_shift_y)
{
	if(0>n_shift_y)
	{
		return;
	}
    shifty_dvb_subt = n_shift_y;
}

void lib_subt_shift_time_set(INT32 n_shift_t,BOOL b_on_off)
{
	if(((TRUE!=b_on_off)&&(FALSE!=b_on_off))||(0>n_shift_t))
	{
		return;
	}
    subt_shift_time = n_shift_t;
    subt_shift_time_onoff = b_on_off;
}

void lib_subt_dvb_bg_color_set(BOOL b_background,UINT32 b_color) //xuehui#1
{
	if(0xffffffff<b_color)
	{
		return;
	}	
    g_background = b_background;
    by_value = (UINT8)((b_color>>24)&0xff);//y_value
    bcb_value = (UINT8)((b_color>>16)&0xff);//cb_value
    bcr_value =(UINT8)((b_color>>8)&0xff);//cr_value
    bt_value = (UINT8)(b_color&0xff);//t_value

    //libc_printf("lib_subt_dvb_bg_color_set libsee,by_value =%x,
    //bcb_value =%x,bcr_value =%x,bt_value =%x\n",by_value,bcb_value,bcr_value,bt_value);
}

void lib_subt_dvb_char_color_set(BOOL b_char,UINT32 b_color)
{
	if(0xffffffff<b_color)
	{
		return;
	}
    g_char = b_char;
    by_value_char = (UINT8)((b_color>>24)&0xff);//y_value
    bcb_value_char = (UINT8)((b_color>>16)&0xff);//cb_value
    bcr_value_char =(UINT8)((b_color>>8)&0xff);//cr_value
    bt_value_char = (UINT8)(b_color&0xff);//t_value
}

static void draw_my2bit_pixel(UINT16 pixel_num,UINT8 region_idx,UINT16 *pos_x, UINT16 pos_y,UINT8 wr_data)
{
    UINT32 i = 0;
	if((MAX_REGION_IN_PAGE<=region_idx)||(PALLETTE_OFFSET>wr_data)||(pixel_num>g_region_cb[region_idx].region_width)
		||(*pos_x>g_region_cb[region_idx].region_width)||(pos_y>g_region_cb[region_idx].region_height))
	{
		return;
	}
    for(i=0; i<pixel_num; i++)
    {
        SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
		*pos_x = (*pos_x)+1;
    }
}


static void my2bit_process(UINT8 my2bit,UINT8 region_idx,UINT16 *pos_x, UINT16 pos_y,UINT8 clut_id,
    BOOL b_two_map_four,UINT8 *two_map_four,UINT8 *end_of_2bit_code_string)
{
    UINT16 pixel_num = 0;
    UINT8 pixel_color = 0;
    UINT8 wr_data = 0;
    UINT32 i = 0;
	if((MAX_REGION_IN_PAGE<=region_idx)||(PALLETTE_OFFSET>clut_id)||(0xff<=my2bit)||(NULL==pos_x)
	||(pos_y>g_region_cb[region_idx].region_height)||(NULL==two_map_four)
	||(*pos_x>g_region_cb[region_idx].region_width)
	||(NULL==end_of_2bit_code_string)||((TRUE!=b_two_map_four)&&(FALSE!=b_two_map_four)))
	{
		return;
	}

    if( 0x2== (my2bit&0x2)) // switch_1 = 1
    {
        pixel_num = getnext2bit();
        pixel_num += (3 + ((my2bit&0x1)<<2));

        pixel_color = getnext2bit();
        if(b_two_map_four)
        {
            pixel_color = two_map_four[pixel_color];
        }
        //wr_data =  (clut_id) + ((pixel_color<<2)+((pixel_color&0x01)<<1)+(pixel_color&0x01));
        wr_data = clut_id + pixel_color;
		#if 0
        for(i=0; i<pixel_num; i++)
        {
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
		#else
		draw_my2bit_pixel(pixel_num,region_idx,pos_x,pos_y,wr_data);
		#endif
    }
    else //switch_1 = 0
    {
        if(  0x1== (my2bit&0x1) ) //switch_2 =1
        {
            pixel_color = 0x00;
            if(b_two_map_four)
            {
                pixel_color = two_map_four[pixel_color];
            }
            //wr_data =  (clut_id) + ((pixel_color<<2)+((pixel_color&0x01)<<1)+(pixel_color&0x01));
            wr_data = clut_id + pixel_color;
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
        else//switch_2 =0
        {
            my2bit = getnext2bit();
            switch(my2bit)
            {
            case 0:  //'00' end of coding
                if(cnt2bit != 0)
                {
                    //rle_buf++;
                    //rle_len--;
                    cnt2bit = 0;
                }
                *end_of_2bit_code_string = 0x01;
                break;
            case 1: //'01' two color 0
                pixel_num = 2;

                pixel_color = 0x00;
                if(b_two_map_four)
                {
                    pixel_color = two_map_four[pixel_color];
                }
                //wr_data =  (clut_id) + ((pixel_color<<2)+((pixel_color&0x01)<<1)+(pixel_color&0x01));
                wr_data = clut_id + pixel_color;

                draw_my2bit_pixel(pixel_num,region_idx,pos_x,pos_y,wr_data);
                break;
            case 2: //'10'  4bit len[12-27] + 2bit color
                my2bit  = getnext2bit();
                pixel_num = getnext2bit();
                pixel_num += (12+(my2bit<<2));

                pixel_color =getnext2bit();
                if(b_two_map_four)
                {
                    pixel_color = two_map_four[pixel_color];
                }
                //wr_data =  (clut_id) + ((pixel_color<<2)+((pixel_color&0x01)<<1)+(pixel_color&0x01));
                wr_data = clut_id + pixel_color;
                draw_my2bit_pixel(pixel_num,region_idx,pos_x,pos_y,wr_data);
                break;
            case 3://'11' 8big len[29-284] + 2bit color
                pixel_num = 0;
                for(i=0; i<4; i++)
                {
                    my2bit  = getnext2bit();
                    pixel_num +=  (my2bit    << ((3-i)*2));
                }
                pixel_num +=29;

                pixel_color =getnext2bit();
                if(b_two_map_four)
                {
                    pixel_color = two_map_four[pixel_color];
                }
                //wr_data =  (clut_id) + ((pixel_color<<2)+((pixel_color&0x01)<<1)+(pixel_color&0x01));
                wr_data = clut_id + pixel_color;

                draw_my2bit_pixel(pixel_num,region_idx,pos_x,pos_y,wr_data);

                break;
                default:
                    break;
            }
        }
    }
}

static UINT8 draw_my4bit_pixel(UINT8 my4bit,UINT8 clut_id,UINT8 region_idx,
    UINT16 *pos_x, UINT16 pos_y,UINT8 *end_of_4bit_code_string)
{
    UINT8 pixel_color = 0;
    UINT16 pixel_num = 0;
    UINT8 wr_data = 0;
    UINT32 i = 0;

	if((MAX_REGION_IN_PAGE<=region_idx)||(PALLETTE_OFFSET>clut_id)||(0xff<=my4bit)||(NULL==pos_x)
	||(pos_y>g_region_cb[region_idx].region_height)||(NULL==end_of_4bit_code_string)
	||(*pos_x>g_region_cb[region_idx].region_width))	
	{
		return 0;
	}
    if( 0== my4bit) //0000
    {
        my4bit = getnext4bit();
        if( SUBT_CODE_VALUE_INVALID == my4bit) //RLE length == 0
        {
            return 0;
        }
        if( SUBT_CODE_VALUE_C== my4bit) //00001100
        {
            //SDEC_PRINTF("1 pixel color0 \n");
            pixel_color = 0x00;
            wr_data =  (clut_id) + pixel_color;
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
        else if( SUBT_CODE_VALUE_D== my4bit) //00001101
        {
            //SDEC_PRINTF("2 pixel color0 \n");
            pixel_color = 0x00;
            wr_data =  (clut_id) + pixel_color;
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
            pixel_color = 0x00;
            wr_data =  (clut_id) + pixel_color;
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
        else if((my4bit>SUBT_CODE_VALUE_0) && (my4bit<SUBT_CODE_VALUE_8)) //00000LLL
        {
            pixel_num = 2 + (my4bit&0x07);
            //SDEC_PRINTF("%d pixel color0 \n",pixel_num);
            pixel_color = 0x00;
            for(i=0;i<pixel_num;i++)
            {
                wr_data =  (clut_id) + pixel_color;
                SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
                *pos_x = (*pos_x)+1;
            }
        }
        else if( SUBT_CODE_VALUE_8== (my4bit&0x0c)) //000010LLCCCC
        {
            pixel_num = 4 + (my4bit&0x03);
            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_color = my4bit;
            //SDEC_PRINTF("%d pixel color %x \n",pixel_num,pixel_color);
            for(i=0;i<pixel_num;i++)
            {
                wr_data =  (clut_id) + pixel_color;
                SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
                *pos_x = (*pos_x)+1;
            }
        }
        else if( SUBT_CODE_VALUE_E== my4bit) //00001110
        {
            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_num = 9 + (my4bit&0x0f);
            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_color = my4bit;
            //SDEC_PRINTF("%d pixel color %x \n",pixel_num,pixel_color);
            for(i=0;i<pixel_num;i++)
            {
                wr_data =  (clut_id) + pixel_color;
                SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
                *pos_x = (*pos_x)+1;
            }
        }
        else if( SUBT_CODE_VALUE_F== my4bit) //00001111
        {
            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_num = (my4bit&0x0f)<<4;
            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_num += (my4bit&0x0f);
            pixel_num += 25;

            my4bit = getnext4bit();
            if( SUBT_CODE_VALUE_INVALID== my4bit) //RLE length == 0
            {
                return 0;
            }
            pixel_color = my4bit;
            //SDEC_PRINTF("%d pixel color %x \n",pixel_num,pixel_color);
            for(i=0;i<pixel_num;i++)
            {
                wr_data =  (clut_id) + pixel_color;
                SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
                *pos_x = (*pos_x)+1;
            }
        }
        else if( SUBT_CODE_VALUE_0 == my4bit) //00000000
        {
            cnt4bit = 0; //for byte aligned
            *end_of_4bit_code_string = 0x01;
            //SDEC_PRINTF("end_of_4bit_code_string \n");
        }
        else//051103 yuchun
        {
            return 0;
        }
    }
    else//0001-1111
    {
        pixel_color = my4bit;
        //SDEC_PRINTF("1 pixel color %x \n",pixel_color);
        wr_data =  (clut_id) + pixel_color;
        SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
        *pos_x = (*pos_x)+1;
    }
    return 1;
}


static UINT8 draw_my8bit_pixel(UINT16 my8bit,UINT8 clut_id,UINT8 region_idx,
    UINT16 *pos_x, UINT16 pos_y,UINT8 *end_of_8bit_code_string)
{
    UINT8 pixel_color = 0;
    UINT16 pixel_num = 0;
    UINT8 wr_data = 0;
    UINT32 i = 0;

    if( 0xffff== my8bit) //RLE length == 0
    {
        return 0;
    }
	if((MAX_REGION_IN_PAGE<=region_idx) /*||(PALLETTE_OFFSET>clut_id) */||(NULL==pos_x)
	||(pos_y>g_region_cb[region_idx].region_height)||(NULL==end_of_8bit_code_string)
	||(*pos_x>g_region_cb[region_idx].region_width))	
	{
		return 0;
	}

    if((my8bit>SDEC_NORMAL_0) && (my8bit<SDEC_NORMAL_128)) //00000000 0LLLLLLL
    {
        pixel_num = my8bit;
        pixel_color = 0;
        //SDEC_PRINTF("%d pixel color %x \n",pixel_num,pixel_color);
        for(i=0;i<pixel_num;i++)
        {
            wr_data =  (clut_id) + ((pixel_color&0xf0)>>4);
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
    }
    else if(my8bit>=128+2) ////00000000 1LLLLLLL CCCCCCCC
    {
        pixel_num = my8bit-128;
        my8bit = getnext8bit();
        if( 0xffff== my8bit) //RLE length == 0
        {
            return 0 ;
        }
        pixel_color = my8bit;
        //SDEC_PRINTF("%d pixel color %x \n",pixel_num,pixel_color);
        wr_data =  (clut_id) + ((pixel_color&0xf0)>>4);

        if(0==clut_id)
            wr_data =  pixel_color;
        for(i=0;i<pixel_num;i++)
        {
            SUBT_DRAW_PIXEL(region_idx,*pos_x,pos_y,wr_data);
            *pos_x = (*pos_x)+1;
        }
    }
    else if( SUBT_CODE_VALUE_0== my8bit) //00000000
    {
        *end_of_8bit_code_string = 0x01;
        //SDEC_PRINTF("end_of_8bit_code_string \n");
    }
    else//051103 yuchun
    {
        return 0;
    }
    return 1;
}



__ATTRIBUTE_RAM_
static void draw_object_bl(UINT8 *data, UINT16 len, UINT8 field_polar,
       UINT8 clut_id, UINT16 stride,UINT16 x, UINT16 y, UINT8 region_idx)
{
    UINT16 data_type = 0;
    UINT8 mychar = 0;
    UINT8 my2bit = 0;
    UINT8 my4bit = 0;
    UINT16 my8bit = 0;
    UINT8 end_of_2bit_code_string =0;
    UINT8 end_of_4bit_code_string =0;
    UINT8 end_of_8bit_code_string =0;
    UINT8 pixel_color = 0;
    UINT16 pos_x = 0;
    UINT16 pos_y = 0;
    UINT8 wr_data = 0;
    BOOL b_two_map_four = FALSE;
    UINT8 two_map_four[4];
    UINT8 ret =0;

     MEMSET(two_map_four, 0, 4);
    SDEC_PRINTF("draw_object_bl, stride = %d, x = %d, y = %d\n", stride, x, y);
	if((MAX_REGION_IN_PAGE<=region_idx) /*||(PALLETTE_OFFSET>clut_id)*/||(NULL==data)
	||(y>g_region_cb[region_idx].region_height)||(0xffff<=stride)
	||(x>g_region_cb[region_idx].region_width)||(0xffff<=len)
	||((0!=field_polar)&&(1!=field_polar)))	
	{
		return ;
	}
    cnt2bit = 0;
    cnt4bit = 0;
    if( 0==field_polar ) //top field data
    {
        rle_len = len;
        rle_buf = --data;
        pos_x = x;
        pos_y = y;
    }
    else    //bottom field data
    {
        rle_len = len;
        rle_buf = --data;
        pos_x = x;
        pos_y = y+1;
    }
    while(1) //RLE decode
    {
        data_type = getnext8bit();
        if( 0xffff==data_type ) //RLE length == 0
        {
            return;
        }
       switch(data_type)
       {
         case 0x10:
            end_of_2bit_code_string = 0x00;
            while(!end_of_2bit_code_string)
            {
                my2bit = getnext2bit();
                if( 0xff==my2bit ) //RLE length == 0
                {
                    return;
                }
                if( 0== my2bit) //00
                {
                    my2bit = getnext2bit();
                    if( 0xff==my2bit ) //RLE length == 0
                    {
                        return;
                    }
                    my2bit_process(my2bit,region_idx,&pos_x, pos_y,clut_id,
                        b_two_map_four,two_map_four,&end_of_2bit_code_string);
                }
                else//
                {
                    pixel_color = my2bit;
                    if(b_two_map_four)
                    {
                        pixel_color = two_map_four[pixel_color];
                    }
                    //SDEC_PRINTF("1 pixel color %x \n",pixel_color);
                    wr_data = clut_id + pixel_color;
                    SUBT_DRAW_PIXEL(region_idx,pos_x++,pos_y,wr_data);
                }
            }
        break;
        case 0x11:
            end_of_4bit_code_string = 0x00;
            while(!end_of_4bit_code_string)
            {
                my4bit = getnext4bit();
                if( 0xff== my4bit) //RLE length == 0
                {
                    return;
                }
                ret = draw_my4bit_pixel(my4bit,clut_id,region_idx,&pos_x,pos_y,&end_of_4bit_code_string);
                if(0 == ret)
                {
                    return;
                }
            }
        break;
        case 0x12:
            end_of_8bit_code_string = 0x00;
            while(!end_of_8bit_code_string)
            {
                my8bit = getnext8bit();
                if( 0xffff== my8bit) //RLE length == 0
                {
                    return;
                }
                if( 0== my8bit) //00000000
                {
                    my8bit = getnext8bit();
                    ret = draw_my8bit_pixel(my8bit,clut_id,region_idx,&pos_x,pos_y,&end_of_8bit_code_string);
                    if(0 == ret)
                    {
                        return;
                    }
                }
                else//00000001-11111111
                {
                    pixel_color = my8bit;
                    //SDEC_PRINTF("1 pixel color %x \n",pixel_color);
                    wr_data =  (clut_id) + ((pixel_color&0xf0)>>4);
					if(0==clut_id)
						wr_data =  pixel_color;
                    SUBT_DRAW_PIXEL(region_idx,pos_x++,pos_y,wr_data);
                }
            }
        break;
        case 0x20:
            mychar = getnext8bit();
            if(0xff==mychar)
            {
                return;
            }
            two_map_four[0] = (mychar&0xf0)>>4;
            two_map_four[1] = (mychar&0x0f);
       
            mychar = getnext8bit();
            if(0xff==mychar)
            {
                return;
            }
            two_map_four[2] = (mychar&0xf0)>>4;
            two_map_four[3] = (mychar&0x0f);
            b_two_map_four = TRUE;
        break;
        case 0x21:
        case 0x22:
            return;
        case 0xf0:
            pos_x = x;
            pos_y +=2;
            //pixdata = pixdata_bak + 2*stride;
            //pixdata_bak = pixdata;
            //SDEC_PRINTF("CHANGE LINE \n");
        break;
        default:
            break;
    }
    }//while loop
}

__ATTRIBUTE_RAM_
static UINT8 find_segment_bl(UINT8 *p_data,UINT32 size,UINT8 *sync_byte ,
     UINT8 *segment_type,UINT16 *page_id,UINT16 *segment_length)
{
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;

	if((NULL==p_data)||(NULL==segment_type)||(NULL==sync_byte)
	||(NULL==page_id)||(NULL==segment_length)||(0xffffffff<=size))		
	{
		return 0x00;
	}
    *sync_byte = *p_data++;
    if(0x0f==*sync_byte)
    {
        if(size < SEGMENT_HEADER_LENGTH) //unit header
        {
            return 0x00;
        }
        *segment_type = *p_data++;
        high_byte = *p_data++;
        low_byte = *p_data++;
        *page_id = (high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        *segment_length =  (high_byte<<8) + low_byte;
        //if(*segment_length >8000)
        if(*segment_length >sdec_temp_buf_len)
        {
            //libc_printf("segment_length too big= %d\n", *segment_length);
            *sync_byte = 0xff;
            return 0x01;
        }
        size -= 6;
        if(size  < *segment_length)
        {
            return 0x00;
        }
    }
    return 0x01;

}


static void parse_page_segment_bl(UINT8 *p_data,INT32 segment_len,UINT32 u_pts,UINT8 u_stc_id)
{
    struct page_cb *page_cb = NULL;
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;
    UINT8 mychar = 0;
    INT32 i = 0;
    INT32 j = 0;
     BOOL region_exist = FALSE;

	if((NULL==p_data)||(0>segment_len)||(0xffffffff<=(UINT32)segment_len)
		||(0xffffffff<=u_pts)||(0xf0==u_stc_id))		
	{
		return ;
	}
    SDEC_PRINTF("page segment\n");
    is_last_obj_seg = FALSE;
    page_cb = &g_page_cb;//[g_page_wr_ptr];
    if(subt_shift_time_onoff)
    {
        page_cb->u_pts = u_pts+subt_shift_time;//xuehui
        SDEC_PRINTF("subt_shiftTime 1 =%8x\n",subt_shift_time);
    }
    else
    {
        page_cb->u_pts = u_pts-subt_shift_time;//xuehui
        SDEC_PRINTF("subt_shiftTime 2 =%8x\n",subt_shift_time);
    }
    SDEC_PRINTF("uPTS=%8x\n",u_pts);
    page_cb->u_stcid = u_stc_id;
    page_cb->page_time_out = *p_data++;
    mychar = *p_data++;
    page_cb->page_version_number = (mychar&0xf0)>>4;
    page_cb->page_state = (mychar&0x0c)>>2;
    segment_len -=2;
    page_region_count = 0;
    if((0x01==page_cb->page_state  ) || ( 0x02== page_cb->page_state))
    {
        for(i=0;i<MAX_REGION_IN_PAGE;i++)
        {
            g_region_cb[i].region_id = 0xff;
            g_region_cb[i].region_width = 0;
            g_region_cb[i].region_height = 0;
            for(j=0;j<MAX_OBJECT_IN_REGION;j++)
            {
                g_region_cb[i].object_id[j] = 0xffff;
            }
            //g_region_cb[i].data = sdec_pixel_buf;
        }
        page_cb->end_of_display = 0;
        g_region_idx = 0;
        total_obj_cnt = 0;
        drawn_obj_cnt = 0;
        total_region_cnt = 0;
        drawn_region_cnt = 0;
        //reset reg count and clut count in page refresh or new page--Michael Xie 3/21/207
        page_cb->region_cnt = 0;
        page_cb->clut_cnt = 0;

    }
    else
    {
        page_cb->end_of_display = 0;
		total_obj_cnt = 0;
        drawn_obj_cnt = 0;
    }
    while(segment_len>0)
    {
        page_cb->region_id[page_cb->region_cnt] = *p_data++;
        p_data++;
        high_byte = *p_data++;
        low_byte = *p_data++;
        page_cb->region_x[page_cb->region_cnt] = (high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        page_cb->region_y[page_cb->region_cnt] = (high_byte<<8) + low_byte;
        segment_len -=6;
        //Check if region already exist, in "chan4mar2.mpg"
        //and "chan5west.mpg", region could be sent repeatly --Michael Xie 3/21/207
        region_exist = FALSE;
        for(i = 0; i < page_cb->region_cnt; i++)
        {
            if(page_cb->region_id[page_cb->region_cnt]==page_cb->region_id[i])
            {
                region_exist = TRUE;
                break;
            }
        }
        if( FALSE== region_exist)
        {
            (page_cb->region_cnt)++;
            total_region_cnt++;
        }
        page_region_count++;
        if(page_cb->region_cnt >= MAX_REGION_IN_PAGE)
        {
            SDEC_PRINTF("page_cb->region_cnt >= MAX_REGION_IN_PAGE\n");
            total_region_cnt=MAX_REGION_IN_PAGE;
            return;
        }
    }
    page_released = 0;
}

static void parse_region_segment_bl(UINT8 *p_data,INT32 segment_len)
{
    INT32 i = 0;
    UINT8 region_id = 0;
    UINT8 region_idx = 0;
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;
    UINT8 mychar = 0;
    BOOL region_is_created = TRUE;
    UINT8 object_idx = 0;

    struct osdrect rect;

    MEMSET(&rect, 0, sizeof(struct osdrect));
	
	
    if((SDEC_NORMAL_10==segment_len)||(NULL==p_data))
    {
        return;
    }
    is_last_obj_seg = FALSE;
    SDEC_PRINTF("region segment\n");
    //libc_printf("region segment\n");
    region_id = *p_data++;
    for(region_idx=0;region_idx<MAX_REGION_IN_PAGE;region_idx++)
    {
        if(g_region_cb[region_idx].region_id == region_id)
        {
            break;
        }
    }
	/* by wen for cpptest BD-PB-ARRAY-1 (changed from == to >= )*/
    if(region_idx>=MAX_REGION_IN_PAGE)
    {
        if(g_region_idx>=MAX_REGION_IN_PAGE)
        {
            return;
        }
        else
        {
            region_idx = g_region_idx++;
        }
    }
    g_region_cb[region_idx].region_id = region_id;
    mychar = *p_data++;
    g_region_cb[region_idx].region_version_number = (mychar&0xf0)>>4;
    g_region_cb[region_idx].region_fill_flag = (mychar&0x08)>>3;
    high_byte = *p_data++;
    low_byte = *p_data++;
    g_region_cb[region_idx].region_width = (high_byte<<8) + low_byte;
    high_byte = *p_data++;
    low_byte = *p_data++;
    g_region_cb[region_idx].region_height = (high_byte<<8) + low_byte;
    mychar = *p_data++;
    g_region_cb[region_idx].region_level = (mychar&0xe0)>>5;
    g_region_cb[region_idx].region_depth = (mychar&0x1c)>>2;
    g_region_cb[region_idx].clut_id = *p_data++;
    g_region_cb[region_idx].region_8b_pixel_code = *p_data++;
    mychar = *p_data++;
    g_region_cb[region_idx].region_4b_pixel_code = (mychar&0xf0)>>4;
    g_region_cb[region_idx].region_2b_pixel_code = (mychar&0x0c)>>2;
    object_idx = 0;
    g_region_cb[region_idx].object_cnt = 0;
    segment_len -=10;
    while((segment_len>0) && (object_idx<MAX_OBJECT_IN_REGION))
    {
        high_byte = *p_data++;
        low_byte = *p_data++;
        g_region_cb[region_idx].object_id[object_idx]=(high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        g_region_cb[region_idx].object_type[object_idx] = (high_byte&0xc0)>>6;
        g_region_cb[region_idx].object_provider_flag[object_idx] =(high_byte&0x30)>>4;
        g_region_cb[region_idx].object_x[object_idx] = ((high_byte&0x0f)<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        g_region_cb[region_idx].object_y[object_idx] = ((high_byte&0x0f)<<8) + low_byte;

        if((SDEC_NORMAL_1==g_region_cb[region_idx].object_type[object_idx]) ||
            (SDEC_NORMAL_2==g_region_cb[region_idx].object_type[object_idx]))
        {
            g_region_cb[region_idx].foregroud_pixel_code[object_idx] = *p_data++;
            g_region_cb[region_idx].backgroud_pixel_code[object_idx] = *p_data++;
            segment_len -=8;
        }
        else
        {
            segment_len -=6;
        }
        object_idx++;
        g_region_cb[region_idx].object_cnt++;
		if(segment_len-6<0)
			break;

    }
    total_obj_cnt += g_region_cb[region_idx].object_cnt;
    if(g_sdec_priv->region_is_created)
    {
        region_is_created = g_sdec_priv->region_is_created(region_idx);
    }
    if((!region_is_created)||((0x01==g_page_cb.page_state)&&region_is_created))
    {
        for(i=0;i<g_page_cb.region_cnt;i++)
        {
            if(g_page_cb.region_id[i] == g_region_cb[region_idx].region_id)
            {
                break;
            }
        }
        if(i == g_page_cb.region_cnt)
        {
            return; //this region is not in page.
        }
        rect.u_left= g_page_cb.region_x[i];
        rect.u_top= g_page_cb.region_y[i];
        rect.u_width= g_region_cb[region_idx].region_width;
        rect.u_height= g_region_cb[region_idx].region_height;
        if(b_auto_top_position_onoff)
        {
           rect.u_top=(region_top_max-region_top_save)-((g_page_cb.region_cnt-i)*rect.u_height);
        }
        else
        {
           if (rect.u_top>SDEC_NORMAL_100)
           {
               rect.u_top+=shifty_dvb_subt;
           }
        }
        if(!region_is_created)
        {
            SUBT_CREATE_REGION(region_idx,&rect);
            SUBT_REGION_SHOW(region_idx,0);
        }
        drawn_region_cnt++;
    }

}

static void parse_clut_segment_bl(UINT8 *p_data,INT32 segment_len)
{
    UINT8 clut_id = 0;
    //UINT8 clut_version_number = 0;
    UINT8 clut_enrty_id = 0;
    //UINT8 bit2_clut_flag = 0;
    //UINT8 bit4_clut_flag = 0;
    //UINT8 bit8_clut_flag = 0;
    UINT8 full_range_flag = 0;
    UINT8 y_value = 0;
    UINT8 cr_value = 0;
    UINT8 cb_value = 0;
    UINT8 t_value = 0;
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;
    UINT8 mychar = 0;
    UINT8 cur_clut_cnt = 0;
    INT32 i = 0;
    UINT8 region_depth = g_region_cb[0].region_depth;
    UINT16 max_entry_in_clut;
    UINT16  clut_start = 0,clut_offset = 0;

	if((SDEC_NORMAL_2==segment_len)||(NULL==p_data))
    {
        return;
    }
    SDEC_PRINTF("subt_clut segment\n");
    is_last_obj_seg = FALSE;
    clut_id = *p_data++;
    mychar = *p_data++;
    //clut_version_number = (mychar&0xf0)>>4;
    segment_len -=2;
    cur_clut_cnt = g_page_cb.clut_cnt;
		
    max_entry_in_clut = MAX_ENTRY_IN_CLUT;
    for(i = 0; i < g_page_cb.clut_cnt; i++)
    {
        if(subt_clut_id[i] == clut_id)
        {
            //CLUT_id already exist, then replace it with new one
            cur_clut_cnt = i;
            break;
        }
    }
    if(3==region_depth)
        max_entry_in_clut = 256;
    else
        clut_start = PALLETTE_OFFSET + cur_clut_cnt * MAX_ENTRY_IN_CLUT ;
    while(segment_len>0)
    {
        clut_enrty_id = *p_data++;
        mychar = *p_data++;
       	//bit2_clut_flag = (mychar&0x80)>>7;
        //bit4_clut_flag = (mychar&0x40)>>6;
        //bit8_clut_flag = (mychar&0x20)>>5;
        full_range_flag = (mychar&0x01);
        if( SDEC_NORMAL_1== full_range_flag)
        {
            y_value = *p_data++;
            cr_value = *p_data++;
            cb_value = *p_data++;
            t_value = *p_data++;
            segment_len -=6;
        }
        else
        {
            high_byte = *p_data++;
            low_byte = *p_data++;
            y_value = (high_byte&0xfc);
            cr_value = ((high_byte&0x03)<<6) + ((low_byte&0xc0)>>2);
            cb_value = (low_byte&0x3c)<<2;
            t_value = (low_byte&0x03)<<6;
            segment_len -=4;
        }
        if ((cur_clut_cnt < MAX_REGION_IN_PAGE) && (clut_enrty_id <max_entry_in_clut))
        {
            clut_offset = clut_start + clut_enrty_id;
            subt_clut[clut_offset][0] = y_value;
            subt_clut[clut_offset][1] = cb_value;
            subt_clut[clut_offset][2] = cr_value;
            if((0==y_value))// && (0==cb_value) && (0 == cr_value))
            {
                t_value = 255;
            }
            subt_clut[clut_offset][3] = t_value;
            if(1 == g_background)
            {
                if( 0 ==clut_enrty_id)
                {
                    subt_clut[clut_offset][0] = by_value;
                    subt_clut[clut_offset][1] = bcb_value;
                    subt_clut[clut_offset][2] = bcr_value;
                    subt_clut[clut_offset][3] = bt_value;
                }
            }
            if(1 == g_char)
            {
                if(clut_enrty_id != 0)
                {
                    subt_clut[clut_offset][0] = by_value_char;
                    subt_clut[clut_offset][1] = bcb_value_char;
                    subt_clut[clut_offset][2] = bcr_value_char;
                    subt_clut[clut_offset][3] = bt_value_char;
                }
            }
        }
    }
    sdec_get_clut = 0xff; // 0xff mean that sdec had got clut data
    if (cur_clut_cnt < MAX_REGION_IN_PAGE)
    {
        subt_clut_id[cur_clut_cnt] = clut_id;
        if(cur_clut_cnt == g_page_cb.clut_cnt)
        {
            g_page_cb.clut_cnt++;
        }
    }

}

static void parse_obj_segment_bl(UINT8 *p_data)
{
    INT32 i = 0;
    INT32 j = 0;
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;
    UINT8 mychar = 0;
    //UINT8 object_version_number = 0;
    UINT8 object_coding_method = 0;
    //UINT8 non_modifying_colour_flag = 0;
    UINT16 top_field_data_block_length = 0;
    UINT16 bottom_field_data_block_length = 0;
    UINT8 number_of_codes = 0;
    UINT16 object_id = 0;
    struct page_cb *page_cb = NULL;
    UINT8 region_depth = g_region_cb[0].region_depth;

    SDEC_PRINTF("object segment\n");
	if(NULL==p_data)
    {
        return;
    }
    high_byte = *p_data++;
    low_byte = *p_data++;
    object_id = (high_byte<<8) + low_byte;

    if(0 ==g_region_idx)
    {
        return;
    }
    is_last_obj_seg = TRUE;
    i = 0;
    while(i < g_region_idx)
    {
        for(j=0;j<g_region_cb[i].object_cnt;j++)
        {
            if(g_region_cb[i].object_id[j] == object_id)
            {
                break;
            }
        }
        if(j==g_region_cb[i].object_cnt)//not find in  region i
        {
            i++;
            continue;
        }

        mychar = *p_data++;
        //object_version_number = (mychar&0xf0)>>4;
        object_coding_method = (mychar&0x0c)>>2;
       // non_modifying_colour_flag = (mychar&0x02)>>1;

        if( SDEC_NORMAL_0== object_coding_method)
        {
            drawn_obj_cnt++;
            high_byte = *p_data++;
            low_byte = *p_data++;
            top_field_data_block_length = (high_byte<<8) + low_byte;
            //PRINTF("top len = %x\n",top_field_data_block_length);
            high_byte = *p_data++;
            low_byte = *p_data++;
            bottom_field_data_block_length = (high_byte<<8) + low_byte;
            //PRINTF("bot len = %x\n",bottom_field_data_block_length);
            if( 0== g_sdec_priv->support_hw_decode)
            {
                draw_object_bl(p_data,top_field_data_block_length,0,region_depth==3? 0 : (PALLETTE_OFFSET+ i*16),
                    g_region_cb[i].region_width,g_region_cb[i].object_x[j],g_region_cb[i].object_y[j],i);
            }
            else
            {
                g_sdec_priv->draw_obj_hardware(p_data,top_field_data_block_length,region_depth==3? 0: (PALLETTE_OFFSET+ i*16),
                    i,g_region_cb[i].region_width,0,g_region_cb[i].object_x[j],g_region_cb[i].object_y[j]);
            }

            p_data += top_field_data_block_length;

            if( 0== g_sdec_priv->support_hw_decode)
            {
                draw_object_bl(p_data,bottom_field_data_block_length,1,region_depth==3? 0: (PALLETTE_OFFSET+ i*16),
                    g_region_cb[i].region_width,g_region_cb[i].object_x[j],g_region_cb[i].object_y[j],i);
            }
            else
            {
                g_sdec_priv->draw_obj_hardware(p_data,bottom_field_data_block_length,region_depth==3? 0:(PALLETTE_OFFSET+ i*16),
                    i,g_region_cb[i].region_width,1,g_region_cb[i].object_x[j],g_region_cb[i].object_y[j]);
            }
            p_data += bottom_field_data_block_length;
        }
        else if( SDEC_NORMAL_1== object_coding_method)
        {
            number_of_codes = *p_data++;
            p_data += number_of_codes;
        }
        //To fix subtitle stream that has no "end segment"
        if((total_obj_cnt) && (total_obj_cnt == drawn_obj_cnt) &&
            (total_region_cnt) && (total_region_cnt==drawn_region_cnt))
        {
            is_last_obj_seg = FALSE;
            page_cb = &g_page_cb;//[g_page_wr_ptr];
            page_finished_time=osal_get_tick();
            if(0 == page_region_count)
            {
                page_cb->end_of_display = 1;
            }
            page_released = 0;
            page_finished = 1;
        }
        break;//i++;
    }
}

static void parse_display_segment_bl(UINT8 *p_data)
{
    UINT8 low_byte = 0;
    UINT8 high_byte = 0;
    UINT8 mychar = 0;
    struct page_cb *page_cb = NULL;

    struct sdec_display_config dds_cfg;

    MEMSET(&dds_cfg, 0, sizeof(struct sdec_display_config));
	if(NULL==p_data)
    {
        return;
    }
    mychar = *p_data++;
    dds_cfg.dds_version_number = (mychar&0xf0)>>4;
    dds_cfg.display_window_flag = (mychar&0x08)>>3;
    high_byte = *p_data++;
    low_byte = *p_data++;
    dds_cfg.display_width = (high_byte<<8) + low_byte;
    high_byte = *p_data++;
    low_byte = *p_data++;
    dds_cfg.display_height = (high_byte<<8) + low_byte;
    page_cb = &g_page_cb;
    if(1==dds_cfg.display_window_flag)
    {
        high_byte = *p_data++;
        low_byte = *p_data++;
        dds_cfg.display_window_hor_min = (high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        dds_cfg.display_window_hor_max = (high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        dds_cfg.display_window_ver_min = (high_byte<<8) + low_byte;
        high_byte = *p_data++;
        low_byte = *p_data++;
        dds_cfg.display_window_ver_max = (high_byte<<8) + low_byte;
        page_cb->display_width = dds_cfg.display_window_hor_max-dds_cfg.display_window_hor_min+1;
        page_cb->display_height =dds_cfg.display_window_ver_max-dds_cfg.display_window_ver_min+1;
    }
    else
    {
        page_cb->display_width = dds_cfg.display_width;
        page_cb->display_height = dds_cfg.display_height;
    }

    if(g_sdec_priv->subt_display_define)
    {
        g_sdec_priv->subt_display_define(&dds_cfg);
    }
}



__ATTRIBUTE_RAM_
static void  parse_segment_bl(UINT8 *p_data,UINT8 segment_type,UINT16 page_id,
       INT32 segment_len,UINT32 u_pts, UINT8 u_stc_id)
{
    struct page_cb *page_cb = NULL;

	if((NULL==p_data)||(0x80<segment_type)||(0>segment_len)
		||(0xffffffff<=(UINT32)segment_len)||(0xffffffff<=u_pts)||(0xf0==u_stc_id))
	{
		return;
	}
    if((g_composition_page_id != page_id) && (g_ancillary_page_id != page_id))
    {
        return;
    }
    if(SUBT_PAGE_SEGMENT==segment_type)
    {
        parse_page_segment_bl(p_data,segment_len,u_pts,u_stc_id);
    }
    else if(segment_type==SUBT_REGION_SEGMENT)
    {
        parse_region_segment_bl(p_data,segment_len);
    }
    else if(segment_type==SUBT_CLUT_SEGMENT)
    {
        parse_clut_segment_bl(p_data,segment_len);
    }
    else if(segment_type==SUBT_OBJECT_SEGMENT)
    {
        parse_obj_segment_bl(p_data);
    }
    else if(segment_type==SUBT_DISPLAY_SEGMENT)//display definition
    {
        parse_display_segment_bl(p_data);
    }
    else if((0x80==segment_type) && (( 0== page_region_count) ||(((total_obj_cnt != drawn_obj_cnt) &&
        (total_obj_cnt)) &&((total_region_cnt) && (total_region_cnt!=drawn_region_cnt)))) )
    {
        is_last_obj_seg = FALSE;
        SDEC_PRINTF("end of display segment\n");
        page_cb = &g_page_cb;//[g_page_wr_ptr];
        end_of_display_page_time=osal_get_tick();
        if( 0== page_region_count)
        {
            page_cb->end_of_display = 1;
        }
        page_released = 0;
        page_finished = 1;

    }
    else
    {
        is_last_obj_seg = FALSE;
    }
    return;
}

static void stream_parse_bl_update_data(void)
{
    if( 1==sdec_flag_not_enough_data)
    {
        if(1 == p_sdec_rd_hdr->u_pes_start)
        {
            sdec_sbf_rd_update(u_sdec_remain_size-sdec_temp_size2);
        }
        else if((pu_sdec_start_point == sdec_temp_buf) ||(pu_sdec_start_point > p_sdec_rd_hdr->pu_start_point))
        {
            sdec_copy_to_temp(pu_sdec_start_point,u_sdec_remain_size);
            sdec_sbf_rd_update(u_sdec_remain_size-sdec_temp_size2);
          //TTX_PRINTF("4. rd upd = %d \n",uSDecRemainSize-sdec_temp_size2);
        }
        else
        {
            p_sdec_rd_hdr->u_data_size += u_sdec_remain_size;
            p_sdec_rd_hdr->pu_start_point = pu_sdec_start_point;
        }
        sdec_flag_not_enough_data = 0;
    }
}

__ATTRIBUTE_RAM_
static void stream_parse_bl(void)
{
    UINT8 sync_byte = 0;
    UINT8 segment_type = 0;
    UINT16 page_id = 0;
    UINT16 segment_length = 0;
    UINT8 ret_func =0;
    UINT32 u_size = 0;
    UINT8 *pu_data= NULL;
    UINT32 u_hdr_ptr = 0;
    UINT8 *tmpdata= NULL;
    UINT8 tmpsync_byte = 0;
    UINT8 tmpsegment_type = 0;
    UINT32 tmpsize = 0;
    UINT16 tmppage_id = 0;
    UINT16 tmpsegment_length = 0;
    BOOL stuffing_has_valid_data = FALSE;

    //SDEC_PRINTF("Stream Parse\n");
    while(page_released) //exit when not enough useful data.
    {
        if(p_sdec_rd_hdr->u_data_size== 0)
        {
            if(RET_SUCCESS==sdec_hdr_buf_rd_req(&u_hdr_ptr)  )
            {
                p_sdec_rd_hdr = &sdec_bs_hdr_buf[u_hdr_ptr];
                sdec_hdr_buf_rd_update();
            }
            else
            {
                //SDEC_PRINTF("No Header Data\n");
                break;//return RET_FAILURE;
            }
        }
        stream_parse_bl_update_data();
        u_size = p_sdec_rd_hdr->u_data_size;
        if(sdec_sbf_rd_req(&u_size,&pu_data) != RET_SUCCESS)
        {
            SDEC_PRINTF("Error: No Buffer Data\n");
            SDEC_ASSERT(FALSE);
        }
        else
        {
            if(pu_data!= p_sdec_rd_hdr->pu_start_point)
                SDEC_ASSERT(FALSE);
            // skip 2 byte (data_identifier & sbutitle_stream_id)
            if(1 == p_sdec_rd_hdr->u_pes_start)
            {
                //libc_printf("new pes start\n");
                if(1 == identifier_detected)
                {
find_header:
                    if(*pu_data != 0x00)//sbutitle_stream_id
                    {
                        SDEC_PRINTF("this is not a subtitle stream!\n");
                       /*Steve Lee: consume error pes data for release version*/
                        sdec_sbf_rd_update(u_size);
                        p_sdec_rd_hdr->u_data_size = 0;
                        identifier_detected = 0;
                        continue;
                        //SDEC_ASSERT(FALSE);//break;
                      /*~Steve Lee: consume error pes data for release version*/
                    }
                    sdec_sbf_rd_update(1);
                    p_sdec_rd_hdr->u_data_size -= 1;
                    p_sdec_rd_hdr->pu_start_point += 1;
                    p_sdec_rd_hdr->u_pes_start = 0;
                    identifier_detected = 0;
                }
                else
                {
                    if(*pu_data != 0x20)//data_identifier
                    {
                        SDEC_PRINTF("this is not a subtitle pes!\n");
                       /*Steve Lee: consume error pes data for release version*/
                        sdec_sbf_rd_update(u_size);
                        p_sdec_rd_hdr->u_data_size = 0;
                        continue;
                        //SDEC_ASSERT(FALSE);//break;
                      /*~Steve Lee: consume error pes data for release version*/
                    }
                    sdec_sbf_rd_update(1);
                    p_sdec_rd_hdr->u_data_size -= 1;
                    p_sdec_rd_hdr->pu_start_point += 1;
                    identifier_detected = 1;
                }
                sdec_stuffing_flag = 0;
                sdec_temp_size = 0;  //clear sdec_temp_size for new pes start
                sdec_temp_size2 = 0;
                continue;
            }

            if(sdec_temp_size)
            {
                sdec_temp_size2 = sdec_temp_size;

                //SDEC_PRINTF("1. pu_data = %8x, u_size = %d\n",pu_data,u_size);
                sdec_add_to_temp(pu_data,u_size);
                pu_data = sdec_temp_buf;
                u_size = sdec_temp_size;
                sdec_temp_size = 0;
            }
            else
            {
                sdec_temp_size2 = 0;
            }
            if(1 == sdec_stuffing_flag) //skip stuffing data
            {
                tmpdata = pu_data;
                tmpsize = u_size;
                while(tmpsize)
                {
                    ret_func = find_segment_bl(tmpdata, tmpsize,
                        &tmpsync_byte,&tmpsegment_type, &tmppage_id,
                        &tmpsegment_length);
					
                    if((0x01==ret_func)&&(SUBT_SEGMENT_FIND == tmpsync_byte))
                    {
                        //Found segment in stuffing data
                        stuffing_has_valid_data = TRUE;
                        break;
                    }
                    tmpdata++;
                    tmpsize--;
                }
                if(TRUE == stuffing_has_valid_data)
                {
                    sdec_stuffing_flag = 0;
                    if(!sdec_temp_size2)
                    {
                        sdec_sbf_rd_update(u_size-tmpsize);
                        p_sdec_rd_hdr->u_data_size -=  (u_size-tmpsize);
                        p_sdec_rd_hdr->pu_start_point += (u_size-tmpsize);
                    }
                    else
                    {
                        sdec_temp_size2 -= (u_size-tmpsize);
                    }
                    pu_data= tmpdata;
                    u_size = tmpsize;
                    stuffing_has_valid_data= FALSE;
                }
                else
                {
                    sdec_sbf_rd_update(u_size-sdec_temp_size2);
                    p_sdec_rd_hdr->u_data_size =  0;
                    continue;
                }
            }
            if(find_segment_bl(pu_data, u_size,&sync_byte,&segment_type, &page_id, &segment_length)==0x00)
            {
                //libc_printf("no seg\n");
                u_sdec_remain_size = u_size;
                pu_sdec_start_point = pu_data;//pSDecRdHdr->puSDecStartPoint;
                sdec_flag_not_enough_data = 1;
                p_sdec_rd_hdr->u_data_size = 0;
                continue;
            }
            else
            {
                if( SUBT_SEGMENT_FIND== sync_byte)
                {
                    //SDEC_PRINTF("segment  find! \n");
                    parse_segment_bl((UINT8*)(pu_data+6),segment_type,
                    page_id, (INT32)segment_length,p_sdec_rd_hdr->u_pts,
                    p_sdec_rd_hdr->u_stc_id);
                    if((UINT32)(segment_length +SEGMENT_HEADER_LENGTH) <
                        sdec_temp_size2)
                    {
                        SDEC_PRINTF("Error: unit_len < sdec_temp_size2! \n");
                        SDEC_ASSERT(FALSE);
                    }
                    sdec_sbf_rd_update(segment_length +6-sdec_temp_size2);
                    p_sdec_rd_hdr->u_data_size -=(segment_length +6-sdec_temp_size2);
                    p_sdec_rd_hdr->pu_start_point +=
                        (segment_length +6-sdec_temp_size2);
                }
                else // stuffing data start , so skip this data block left.
                {
                    if((u_size>SDEC_NORMAL_3) && ( SDEC_NORMAL_0==sync_byte )&&
                        (pu_data!=NULL))
                    {
                        if((0x20==*(pu_data-1)) && (0x00==*pu_data) &&
                            (0x0f==*(pu_data+1)))
                        {
                            goto find_header;
                        }
                    }
                    sdec_sbf_rd_update(u_size-sdec_temp_size2);
                    p_sdec_rd_hdr->u_data_size = 0;
                    sdec_stuffing_flag = 1;
                }
            }
        }//sbf_rd_req(SDEC_DATA_BUF_ID) RET_SUCCESS
    }// end while loop for each update write
}

static void page_timeout_handle(__MAYBE_UNUSED__ UINT time)//(void)
{
    //libc_printf("page_timeout_handle\n");
	time=0;
    set_sdec_clear_page(TRUE);
    osal_timer_delete(sdec_alarm_id);
    sdec_alarm_id = OSAL_INVALID_ID;
}

__ATTRIBUTE_RAM_
static void dec_page_bl(struct page_cb *page_cb, UINT8 param)
{
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 region_idx = 0;
    UINT8 region_id = 0;
    OSAL_T_CTIM     t_dalm;
    UINT8 clut_index = 0;
    UINT8 show_region_idx[MAX_REGION_IN_PAGE]={0};
    UINT8 show_region_cnt = 0;
	UINT8 region_depth;
	UINT16  clut_start = 0,clut_offset = 0;
	UINT32 diff_time=0;

    MEMSET(&t_dalm,0,sizeof(OSAL_T_CTIM));

	if((NULL==page_cb)||(0xff<=param))
	{
		return;
	}
    SDEC_PRINTF("dec_page_bl\n");

    if(FALSE == page_finished)
    {
        if((0x02==page_cb->page_state  ) || ( 0x01== page_cb->page_state))
        {
              if((0x01 == page_cb->page_state) && (SDEC_NORMAL_1==param) &&
                (page_cb->region_cnt>SDEC_NORMAL_3)&&
                (page_cb->region_cnt == old_region_count))
            {//to avoid subtitle show time too short in stream
            //TV3_794Mhz_8k_1_4_BW8__FEC_2_3--cloud
                return;
            }
            for(i=0;i<MAX_REGION_IN_PAGE;i++)
            {
            	if(page_cb->region_cnt>0)
                SUBT_DELETE_REGION(i);
            }
        }
        return;
    }

    if (sdec_alarm_id != OSAL_INVALID_ID)
    {
        osal_timer_delete(sdec_alarm_id);
        sdec_alarm_id = OSAL_INVALID_ID;
    }


	if(page_cb->end_of_display == 1)
	{
	
       if((0x01 == page_cb->page_state) && (SDEC_NORMAL_1==param) &&(page_cb->region_cnt>SDEC_NORMAL_3))
       {
       }
       else
       {
       
        	diff_time=end_of_display_page_time-page_finished_time;        	
        	if(diff_time>=1000)
        	{
    			osd_subt_clear_page();
        	}
  	  	}
       	page_finished_time=0;
       	end_of_display_page_time=0;
    }

	region_depth = g_region_cb[0].region_depth;
	if(region_depth!=3)
		clut_start = PALLETTE_OFFSET;
	else
	{
		for(clut_offset=0;clut_offset<256;clut_offset++)
			osd_subt_set_clut(clut_offset,
						subt_clut[clut_offset][0],
						subt_clut[clut_offset][1],
						subt_clut[clut_offset][2],
						15 - (subt_clut[clut_offset][3]>>4));
	}

    for(i=0;i<page_cb->region_cnt;i++)
    {

        region_id = page_cb->region_id[i];

        for(region_idx=0;region_idx<MAX_REGION_IN_PAGE;region_idx++)
        {
            if(g_region_cb[region_idx].region_id == region_id)
            {
                break;
            }
        }
        /* by wen for cpptest BD-PB-ARRAY-1 (changed from == to >= )*/
        if(region_idx>=MAX_REGION_IN_PAGE)
        {
            continue;
        }

        for (clut_index=0; clut_index<MAX_REGION_IN_PAGE; clut_index++)
        {
            if (subt_clut_id[clut_index] == g_region_cb[region_idx].clut_id)
            {
                break;
            }
        }
        if (clut_index >= MAX_REGION_IN_PAGE)
        {
            clut_index = 0;
        }

		if(region_depth!=3)
		{
			for(j=0;j<MAX_ENTRY_IN_CLUT;j++)
//			osd_subt_set_clut(((g_region_cb[region_idx].CLUT_id)*16+PALLETTE_OFFSET) + j,
			{
			clut_offset = clut_start + clut_index*MAX_ENTRY_IN_CLUT + j;
			osd_subt_set_clut((region_idx*16+PALLETTE_OFFSET) + j,
						subt_clut[clut_offset][0],
						subt_clut[clut_offset][1],
						subt_clut[clut_offset][2],
						15 -((subt_clut[clut_offset][3])>>4));
			}
		}
        //SDEC_PRINTF("region_id[%d].data = %8x\n",
        //i,g_region_cb[region_id].data);
        //SUBT_REGION_SHOW(region_idx,1);
        show_region_idx[show_region_cnt] = region_idx;
        show_region_cnt++;
    }
    old_region_count = show_region_cnt;
    if((show_region_cnt>0) && (sdec_get_clut != 0xff))
    {
        sdec_get_clut++;
        if(sdec_get_clut>1)
        {
            sdec_get_clut = 0xff;
            for(i = 0; i < MAX_REGION_IN_PAGE; i++)
            {
				for(j=0;j<MAX_ENTRY_IN_CLUT;j++)
				{
					MEMCPY(subt_clut[PALLETTE_OFFSET + i*MAX_ENTRY_IN_CLUT + j], subt_defalt_clut[j], 4);
				}
            }
        }
    }
	if((0x01 == page_cb->page_state)||(0x02 == page_cb->page_state))
		SUBT_CLEAR_OSD_SCREEN(TRUE);
	else
		SUBT_CLEAR_OSD_SCREEN(FALSE);

    for(i=0; i<show_region_cnt; i++)
    {
        if((0==i)&&(total_obj_cnt>0))//update 1 times
        {
            osd_subt_update_clut();
        }
		if(total_obj_cnt>0)
        SUBT_REGION_SHOW(show_region_idx[i], 1);
    }
    //reset page_cb->region_cnt after clear page-----cloud
    if((1 == page_cb->end_of_display) && (0 == page_cb->page_state))
    {
        page_cb->region_cnt = 0;
    }

    page_cb->display_width = 0;
    page_cb->display_height = 0;

    t_dalm.callback = page_timeout_handle;//cloud
    t_dalm.type = TIMER_ALARM;
    t_dalm.time  = 1000*page_cb->page_time_out;
	if((t_dalm.time==0)||(page_cb->page_time_out>=30))
		 t_dalm.time = 5000;


    //libc_printf("page_time_out=%d\n",page_cb->page_time_out);
    sdec_alarm_id = osal_timer_create(&t_dalm);
}

static void display_entry_bl(UINT8 param)
{
    UINT32 stc_msb32 = 0;
    struct page_cb *page_cb = NULL;
    UINT32 stc_delay = 0;
    struct snd_device *snd_dev = NULL;

    snd_dev = (struct snd_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
    //SDEC_PRINTF("Display Entry\n");
	if(0xff<=param)
	{
		return;
	}
    if(1 == page_released)//if(g_page_rd_ptr==g_page_wr_ptr)
    {
        return;
    }
    else
    {
        page_cb = &g_page_cb;//&g_page_cb[g_page_rd_ptr];
    }

    if( 0== page_finished)
    {
        if( 0xff==page_cb->u_stcid )
        {
            SDEC_PRINTF("Error: stc == 0xff\n");
            stc_msb32 = 0xffffffff;
        }
        else if(get_stc(&stc_msb32, page_cb->u_stcid)!=RET_SUCCESS)
        {
            SDEC_PRINTF("Error: stc NOT init!\n");
            stc_msb32 = 0xffffffff;
        }

        //libc_printf("uPTS=0x%8x ,stc_msb32=0x%8x, stc_delay = 0x%8x \n",
        //page_cb->uPTS,stc_msb32, stc_delay);
        snd_io_control(snd_dev,SND_STC_DELAY_GET,(UINT32)(&stc_delay));
        if(page_cb->u_pts > stc_msb32+0x100000) // too far ,free run
        {
            SDEC_PRINTF("free run\n");
            //libc_printf("free run\n");
            stc_msb32 = 0xffffffff;
        }
        //SDEC_PRINTF("DISP_STC=%8x ,uPTS=%8x \n",stc_msb32,page_cb->uPTS);
        /*"+0x3000, in block link mode, we only use one OSD frame buffer,
        we start decoding after PTS
          is matched, this could cause display subtitle is a bit slower
          than 2 OSD frame buffer solution,
          Therefore, "0x3000" ahead of PTS to start decoding to
          compensate this issue.
          */
        else if(page_cb->u_pts > stc_msb32-stc_delay)//+0x3000)
        {
            //libc_printf("return\n");
            return;
        }
        //libc_printf("ok go\n");

        SDEC_PRINTF("DISP_STC=%8x ,uPTS=%8x \n",stc_msb32,page_cb->u_pts);
    }

    if(TRUE == subt_show_on)
    {
        dec_page_bl(page_cb,param);
    }

    page_released = 1;
    page_finished = 0;
    osal_flag_set(sdec_flag_id,SDEC_MSG_BUFF_AVAILABLE);

}

void subt_disply_bl_init(struct sdec_device *dev)
{
	if(NULL==dev)
	{
		return;
	}
    struct sdec_m3327_private*priv=(struct sdec_m3327_private*)(dev->priv);

    if(priv)
    {
        osal_interrupt_disable();
        priv->stream_parse_cb = stream_parse_bl;
        priv->display_entry_cb = display_entry_bl;
        osal_interrupt_enable();
    }
}


