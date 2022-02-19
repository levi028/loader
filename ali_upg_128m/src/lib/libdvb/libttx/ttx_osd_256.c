/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: ttx_osd_256.c

   *    Description:define function which used by OSD to show ttx
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include "lib_ttx_internal.h"

//extern UINT32* find_char(UINT16 charset, UINT8 character, UINT8 p26_char_set);

const UINT8 ttx_pallette_256[4*256] = {
                /*front*/
                0x10, 0x80, 0x80, 0x0f,    //black
                0x50, 0x5b, 0xec, 0x0f,    //red
                0x8f, 0x37, 0x24, 0x0f,    //green
                0xd0, 0x12, 0x90, 0x0f,    //yellow
                0x27, 0xec, 0x6e, 0x0f,    //blue
                0x68, 0xc7, 0xda, 0x0f,    //magenta
                0xa7, 0xa3, 0x12, 0x0f,    //cyan
                0xea, 0x7f, 0x7f, 0x0f,    //white
                /*back*/
                0x10, 0x80, 0x80, 0x0f,    //black
                0x50, 0x5b, 0xec, 0x0f,    //red
                0x8f, 0x37, 0x24, 0x0f,    //green
                0xd0, 0x12, 0x90, 0x0f,    //yellow
                0x27, 0xec, 0x6e, 0x0f,    //blue
                0x68, 0xc7, 0xda, 0x0f,    //magenta
                0xa7, 0xa3, 0x12, 0x0f,    //cyan
                0xea, 0x7f, 0x7f, 0x0f,    //white

                0, 0, 0, 0,    //transparent
				0x10, 0x80, 0x80, 0x0f,	//black-instead
                0x50, 0x5b, 0xec, 0x0f,	//red-instead
				0x8f, 0x37, 0x24, 0x0f,	//green-instead
				0xd0, 0x12, 0x90, 0x0f,	//yellow-instead
				0x27, 0xec, 0x6e, 0x0f,	//blue-instead
				0x68, 0xc7, 0xda, 0x0f,	//magenta-instead
				0xa7, 0xa3, 0x12, 0x0f,	//cyan-instead
				0xea, 0x7f, 0x7f, 0x0f,	//white-instead
};

BOOL check_fg_bg_same(UINT8 fg_color,UINT8 bg_color)
{
	BOOL flag=FALSE;
	
	if((TTX_F_BLACK==fg_color)&&(TTX_B_BLACK==bg_color))
		flag=TRUE;
	else if((TTX_F_RED==fg_color)&&(TTX_B_RED==bg_color))
		flag=TRUE;
	else if((TTX_F_GREEN==fg_color)&&(TTX_B_GREEN==bg_color))
		flag=TRUE;
	else if((TTX_F_YELLOW==fg_color)&&(TTX_B_YELLOW==bg_color))
		flag=TRUE;
	else if((TTX_F_BLUE==fg_color)&&(TTX_B_BLUE==bg_color))
		flag=TRUE;
	else if((TTX_F_MAGENTA==fg_color)&&(TTX_B_MAGENTA==bg_color))
		flag=TRUE;
	else if((TTX_F_CYAN==fg_color)&&(TTX_B_CYAN==bg_color))
		flag=TRUE;
	else if((TTX_F_WHITE==fg_color)&&(TTX_B_WHITE==bg_color))
		flag=TRUE;
	else
		flag=FALSE;
	
	return flag;	
}


static UINT8 ttx_color_256_set_data(UINT32 *char_addr,UINT32 charmask,UINT8 fg_color,UINT8 bg_color)
{
    UINT8 data = 0;

    if((fg_color>7)||((bg_color<8)||(bg_color>15))||(NULL == char_addr))
    {
        return 0xFF;
    }

    if(*char_addr & charmask)
    {
       data = fg_color;
    }
    else
    {
       data = bg_color;
    }
    return data;
}

void ttx_draw_char_256(UINT16 x, UINT16 y, UINT16 charset, UINT8 fg_color,UINT8 bg_color,
    UINT8 double_width,UINT8 double_height,UINT8 character, UINT8 p26_char_set)
{ 
    struct osdrect r;
    UINT8 *p_data = NULL;
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 data1 = 0;
	UINT8 data2=0;
	UINT8 data3=0;
	UINT8 data4=0;
    UINT8 zoomkind = 0;
    UINT16 bit_count = 0;
    UINT32 charmask = 0;
    UINT32 *char_addr = NULL;
    INT16 dw = 0;
    INT16 dh = 0;
	UINT8 ts_color = 0;
	UINT8 m=0,n=0;	
	BOOL flag=FALSE;
	int bit_count_mod14=0;
	
    MEMSET(&r,0x0,sizeof(struct osdrect));

	

	if(sys_ic_get_chip_id() == ALI_S3281)
	{
		flag=check_fg_bg_same(fg_color,bg_color);
	
		for(m=TTX_F_BLACK,ts_color=TTX_TRANSFOR_COLOR;m<TTX_B_BLACK;m++,ts_color++)
		{	 		
	 		if(fg_color==m)
			{
				for(n=TTX_B_BLACK;n<TTX_TRANSPARENT;n++,ts_color++)
	 			{
					if(bg_color==n)
					{				
					
						break;
					}
					else
						continue;
			
				}					
				
				break;						
			}
			else
				continue;
			
		}
	}
	

	
    if(ttx_show_on!=TRUE)
    {
        return;
    }
    if((character < TTX_CHARACTER_SPACE )||(0xffff == charset)\
        ||(fg_color>7)||((bg_color<8)||(bg_color>15))||(0xff == p26_char_set))
    {
        return;
    }
    if(((x%TTX_CHAR_W) != 0) ||((y%TTX_CHAR_H) != 0) ||((x+TTX_CHAR_W) > osd_ttx_width))
    {
        return ;
    }
    if((TRUE== get_ttxmenu_status()) && (TRUE ==get_ttxinfo_status()))
    {
        if((y+TTX_CHAR_H) > (osd_ttx_height+TTX_CHAR_H))
        {
            return;
        }
    }
    else
    {
        if((y+TTX_CHAR_H) > osd_ttx_height)
        {
            return;
        }
    }
    if(double_width && (x>=TTX_MAX_COL_IF_DOUBLE*TTX_CHAR_W))
    {
        double_width = 0;
    }
    if(double_height && (y>=TTX_MAX_ROW*TTX_CHAR_H))
    {
        double_height = 0;
    }
    if(double_width)
    {
        //PRINTF("---------TTX_DrawChar(), OSD_STRIDE = %d \n", OSD_STRIDE);
        dw = 28;
        if(double_height) //vertical zoom double
        {
            dh = 40;
            zoomkind = 4;
        }
        else        // vertical normal
        {
            dh = 20;
            zoomkind = 3;
        }
    }
    else    //horizion normal
    {
        //PRINTF("---------TTX_DrawChar(), OSD_STRIDE = %d \n", OSD_STRIDE);
        dw = 14;
        if(double_height)    //vertical zoom  double
        {
            dh = 40;
            zoomkind = 2;
        }
        else        //vertical normal
        {
            dh = 20;
            zoomkind = 1;
        }
    }
    r.u_left = x + osd_ttx_xoffset;
    r.u_top = y + osd_ttx_yoffset;
    r.u_width = dw;
    r.u_height = dh;
    ttx_vscr.v_r.u_left = r.u_left;
    ttx_vscr.v_r.u_top = r.u_top;
    ttx_vscr.v_r.u_width = TTX_CHAR_W*2;
    ttx_vscr.v_r.u_height = TTX_CHAR_H*2;
    ttx_vscr.lpb_scr = g_ttx_pconfig_par.ttx_vscrbuf;
    p_data = ttx_vscr.lpb_scr;
    bit_count = 0;
    char_addr = find_char( charset, character, p26_char_set);
    if(NULL == char_addr)
    {
        return;
    }
    switch(zoomkind)
    {
          case 1:            //vertical normal , horizine normal
               for(i = 0 ;i < 9;i++)
               {
                   charmask = 0x80000000;
                   for(j = 0 ;j < 32 ;j++)
                   {
                      if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)
                      {//char draw finished
                          break;
                      }
                      if((bit_count!=0) && (0 ==bit_count %14))
                      {//one row finished, change to next
                          p_data += 14;
                      }
                      data1 = ttx_color_256_set_data(char_addr,charmask, fg_color,bg_color);
					  if((sys_ic_get_chip_id() == ALI_S3281)&&(get_ttx_is_mosiac()==FALSE)&&(y<TTX_MAX_ROW*TTX_CHAR_H)&&(!flag))							
					  {
					   		bit_count_mod14 = bit_count %14;
							   
							if(bit_count_mod14 > 2)
							{
							        //check left and modify
								data2 = *(p_data-1);
								if((data2 == fg_color) && (data1 == bg_color))
								{
								    *(p_data-1) = ts_color;
								} 
								
								data2 = *(p_data-1);
								data3 = *(p_data-2);
								if ((data3 == fg_color) && (data2 == bg_color))
								{
								    *(p_data-2) = ts_color;
								}
								
								data3 = *(p_data-2);
								data4 = *(p_data-3);
								if ((data4 == fg_color) && (data3 == bg_color))
								{
								    *(p_data-3) = ts_color;
								}
								
								if ((data4 == bg_color) && (data3 == fg_color) && (data2 == fg_color) && (data1 == fg_color))
								{
								    *(p_data-2) = ts_color;
								}
								
								//check right boundary
								if(bit_count_mod14 == 13)
								{
								    if(data1 == fg_color) 
								    {
								        data1 = ts_color;
								    }
								}
							} 
							else if (bit_count_mod14 == 0)
							{
							        //check left boundary
								if(data1 == fg_color) 
								{
								    	data1 = ts_color;
								}
							}
					  }
                      *p_data = data1;
                      p_data++;
                      bit_count++;
                      charmask = charmask>>1;
                   }
                   char_addr += 1;
                }
                break;
            case 2:            //vertical zoom  double, horizine normal
                for(i = 0 ;i < 9;i++)
                {
                    charmask = 0x80000000;
                    for(j = 0;j < 32;j++)
                    {
                        if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)
                        {
                           break;
                        }
                        if((bit_count!=0) && (0 ==bit_count %14))
                        {
                           p_data += 14*3;
                        }
                        data1 = ttx_color_256_set_data(char_addr,charmask, fg_color,bg_color);
						if((sys_ic_get_chip_id() == ALI_S3281)&&(get_ttx_is_mosiac()==FALSE)&&(y<24*TTX_CHAR_H)&&(!flag))						
					    {
							bit_count_mod14 = bit_count %14;
							    
							if(bit_count_mod14 > 2)
							{
							        //check left and modify
								data2 = *(p_data-1);
								if((data2 == fg_color) && (data1 == bg_color))
								{
								    *(p_data-1) = ts_color;
								    *(p_data-1+28) = ts_color;
								} 
								
								data2 = *(p_data-1);
								data3 = *(p_data-2);
								if ((data3 == fg_color) && (data2 == bg_color))
								{
								    *(p_data-2) = ts_color;
								    *(p_data-2+28) = ts_color;
								}
								
								data3 = *(p_data-2);
								data4 = *(p_data-3);
								if ((data4 == fg_color) && (data3 == bg_color))
								{
								    *(p_data-3) = ts_color;
								    *(p_data-3+28) = ts_color;
								}
								
								if ((data4 == bg_color) && (data3 == fg_color) && (data2 == fg_color) && (data1 == fg_color))
								{
								    *(p_data-2) = ts_color;
								    *(p_data-2+28) = ts_color;
								}
								
								//check right boundary
								if(bit_count_mod14 == 13)
								{
								    if(data1 == fg_color) 
								    {
								        data1 = ts_color;
								    }
								}
						   } 
						   else if (bit_count_mod14 == 0)
						   {
							        //check left boundary
								if(data1 == fg_color) 
								{
									data1 = ts_color;
								}
						   }
					    }
                        *p_data = data1;
                        *(p_data + 14*2) = *p_data;

                        p_data++;
                        bit_count++;
                        charmask = charmask>>1;
                    }
                    char_addr += 1;
               }
               break;
           case 3:         //horizine zoom  double, vertical normal
               for(i = 0 ;i < 9;i++)
               {
                   charmask = 0x80000000;
                   for(j=0;j<32;j++)
                   {
                       if(bit_count >= TTX_CHAR_W * TTX_CHAR_H)
                       {
                          break;
                       }
                       data1 = ttx_color_256_set_data(char_addr,charmask, fg_color,bg_color);

                       *p_data = data1;
                       p_data++;
                       *p_data = data1;
                       p_data++;
                       bit_count++;
                       charmask = charmask>>1;
                    }
                    char_addr += 1;
                }
                break;
            case 4:             //horizine zoom  double, vertical double
                for(i = 0 ;i < 9;i++)
                {
                    charmask = 0x80000000;
                    for(j = 0;j < 32;j++)
                    {
                        if(bit_count == TTX_CHAR_W * TTX_CHAR_H)
                        {
                            break;
                        }
                        if((bit_count!=0) && (0 ==bit_count % 14))
                        {
                            p_data += 14*2;
                        }
                        data1 = ttx_color_256_set_data(char_addr,charmask, fg_color,bg_color);
                        *p_data = data1;
                        *(p_data + 14*2) = data1;
                        *(p_data + 14*2+1) = data1;
                        p_data++;
                        *p_data = data1;
                        p_data++;
                        charmask = charmask>>1;
                        bit_count += 1;
                    }
                    char_addr += 1;
                }
                break;
        default:
            break;
    }
    if(RET_SUCCESS != osddrv_region_write((HANDLE)g_ttx_osd_dev, m_b_cur_region, &ttx_vscr, &r))
    {
        TTX_PRINTF("error\n");
    }
}
