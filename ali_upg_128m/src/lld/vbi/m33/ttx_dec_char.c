/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: ttx_dec_char.c

   *    Description:define the function using to parse the ttx data
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include "vbi_m3327.h"
#include "vbi_m3327_internal.h"
#include <hld/osd/osddrv_dev.h>
#include "../../../lib/libdvb/libttx/lib_ttx_internal.h"

#define TTXDEC_PRINTF PRINTF
//#define TTXDEC_PRINTF libc_printf
#ifdef SEE_CPU
extern OSAL_ID TTX_SEM_ID;
#define ENTER_TTX_ENG() \
osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME)
#define LEAVE_TTX_ENG() osal_semaphore_release(TTX_SEM_ID)
#else
#define ENTER_TTX_ENG()
#define LEAVE_TTX_ENG()
#endif
extern BOOL b_ttx_subt_deal;
#ifdef TTX_FLASH_FUNC

static VSCR TTX_Flash_Vscr ;
UINT8 ttx_flash_test_num = 0;
static OSAL_ID ttx_flash_task_id = 0;
UINT8 g_ttx_flash_buf[11264*2];//14*20*40*double_height=11200
extern UINT32 osd_ttx_height;
extern UINT32 osd_ttx_startcol;
extern UINT32 osd_ttx_startrow;
ID ttx_flash_mutex = 0;

UINT16 last_cur_subpage = 0xffff;
UINT16 last_cur_page =0xffff;

typedef struct ttx_flash
{   
    UINT32 left;
    UINT32 top;
    UINT32 width;
    UINT32 height;
    UINT8  fg_color;
    UINT8 bg_color;

    struct ttx_flash *next;
}ttx_flash_node; 

ttx_flash_node *ttx_flash_list=NULL;

void ttx_delete_whole()
{
    //libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
    ttx_flash_node *p;
    MEMSET(g_ttx_flash_buf,0x00,11264*2);
    osal_mutex_lock(ttx_flash_mutex, OSAL_WAIT_FOREVER_TIME);
    while(ttx_flash_list)
    {
        p=ttx_flash_list;
        ttx_flash_list=p->next;
        free(p);
    }
     osal_mutex_unlock(ttx_flash_mutex);
}


static UINT8 ttx_front_color_change(UINT8 fg_color)
{
    UINT8 ret =0 ;
    switch(fg_color)
    {
        case TTX_B_BLACK:
            ret = TTX_BC_BLACK;
        break;
        case TTX_B_RED:
            ret = TTX_BC_RED;
        break;
        case TTX_B_GREEN:
            ret = TTX_BC_GREEN;
        break;
        case TTX_B_YELLOW:
            ret = TTX_BC_YELLOW;
        break;
        case TTX_B_BLUE:
            ret = TTX_BC_BLUE;
        break;
        case TTX_B_MAGENTA:
            ret = TTX_BC_MAGENTA;
        break;
        case TTX_B_CYAN:
            ret = TTX_BC_CYAN;
        break;
        case TTX_B_WHITE:
            ret = TTX_BC_WHITE;
        break;
    }
    return ret;
}

int ttx_flash_get_node(ttx_flash_node *pre, ttx_flash_node *node)
{
    ttx_flash_node *ptr = ttx_flash_list;
    int ret = 0;
    if((ttx_flash_list == NULL) || (node == NULL) || (pre == NULL)) return 0;
    osal_mutex_lock(ttx_flash_mutex, OSAL_WAIT_FOREVER_TIME);
    if(pre->height)
    {
        while(ptr)
        {
            if(ptr == pre->next)
            {
                memcpy(node, ptr, sizeof(ttx_flash_node));
                ret = 1;
                break;
            }
            ptr=ptr->next;
        }
        if(NULL == ptr)
            ret = 0;
    }
    else
    {
        memcpy(node, ttx_flash_list, sizeof(ttx_flash_node));
        ret = 1;
    }
    osal_mutex_unlock(ttx_flash_mutex);
    return ret;
}


void ttx_flash_task()
{
    
    INT32 i =0;
    struct osdrect r;
    UINT8 fc_color = 0;
    ttx_flash_node p_node;



    while(1)
    {
        MEMSET(&p_node, 0, sizeof(p_node));

        
        while(ttx_flash_get_node(&p_node, &p_node))
        {
            TTX_Flash_Vscr.v_r.u_left=  p_node.left;
            TTX_Flash_Vscr.v_r.u_top =  p_node.top;
            TTX_Flash_Vscr.v_r.u_width =  p_node.width;
            TTX_Flash_Vscr.v_r.u_height =  p_node.height;
            TTX_Flash_Vscr.lpb_scr= g_ttx_flash_buf;
            r.u_left=  p_node.left;
            r.u_top=  p_node.top;
            r.u_width=  p_node.width;
            r.u_height=  p_node.height;

            MEMSET(g_ttx_flash_buf,0x00,11264*2);
            osddrv_region_read((HANDLE)g_ttx_osd_dev,m_b_cur_region,&TTX_Flash_Vscr,&r);

            fc_color = ttx_front_color_change( p_node.bg_color);
            for(i=0;i<TTX_Flash_Vscr.v_r.u_width*TTX_Flash_Vscr.v_r.u_height;++i)
            {
                if(g_ttx_flash_buf[i]<=TTX_F_WHITE)
                {
                    
                    g_ttx_flash_buf[i]= fc_color;   
                }
            }
            osddrv_region_write((HANDLE)g_ttx_osd_dev, m_b_cur_region, &TTX_Flash_Vscr, &r);
         }
        osal_task_sleep(500);
        MEMSET(&p_node, 0, sizeof(p_node));
        while(ttx_flash_get_node(&p_node, &p_node))
        {
            TTX_Flash_Vscr.v_r.u_left=  p_node.left;
            TTX_Flash_Vscr.v_r.u_top =  p_node.top;
            TTX_Flash_Vscr.v_r.u_width =  p_node.width;
            TTX_Flash_Vscr.v_r.u_height =  p_node.height;

            TTX_Flash_Vscr.lpb_scr = g_ttx_flash_buf;
			
            r.u_left=  p_node.left;
            r.u_top=  p_node.top;
            r.u_width=  p_node.width;
            r.u_height=  p_node.height;

            osddrv_region_read((HANDLE)g_ttx_osd_dev,m_b_cur_region,&TTX_Flash_Vscr,&r);

            for(i=0;i<TTX_Flash_Vscr.v_r.u_width*TTX_Flash_Vscr.v_r.u_height;++i)
            {
                if(g_ttx_flash_buf[i]>= TTX_BC_BLACK)
                    g_ttx_flash_buf[i]=  p_node.fg_color;
            }
            osddrv_region_write((HANDLE)g_ttx_osd_dev, m_b_cur_region, &TTX_Flash_Vscr, &r);
        }
        osal_task_sleep(500);
    }
    
}

INT32 ttx_flash_task_creat()//(struct vbi_device *dev)
{
	
	OSAL_T_CTSK t_ctsk;

	t_ctsk.task = (FP)ttx_flash_task;
	t_ctsk.para1 = 0;
	t_ctsk.para2 = 0;
	t_ctsk.stksz = 0x0C00;
	t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL; 

#ifdef _TDS_INFO_MONITOR
    t_ctsk.name[0] = 'T';//TTX
    t_ctsk.name[1] = 'F';//FLASH
    t_ctsk.name[2] = 'T';//TASK
#endif
	ttx_flash_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID==ttx_flash_task_id)
	{
	       // VBI_PRINTF("Can not create VBIDec_Task!!!\n");
		 ASSERT(0);
	}

	return RET_SUCCESS;
}


#endif

struct line_ctrl_bl
{
    UINT8 draw_next_line;
    UINT8 double_line;
    UINT8 alpha_mosiac;
    UINT8 fg_color;
    UINT8 pre_bg_color;
    UINT8 bg_color;
    UINT8 width;
    UINT8 height;
    UINT8 conceal;
    UINT8 continous;
    UINT8 hold_mosiac;
    UINT8 real_alpha_mosiac;
    UINT8 real_mosiac;
    UINT8 last_mosiac;
    UINT8 real_fg_color;
    UINT8 real_bg_color;
    UINT8 real_width;
    UINT8 real_height;
    UINT8  alpha1_g1;
    UINT8  alpha2_g1;
    UINT8  alpha_g2;
    UINT8 clear_extra_data;
    UINT16 alpha;
    UINT16 alpha1;
    UINT16 alpha2;
    UINT16 mosiac;
};

extern UINT8 seperate_ttxsubtitle_mode;
extern UINT8 mosaic_mode;

static BOOL ttxsubt_double_line_show = FALSE;
static BOOL b_false = FALSE;//avoid a compiler bug
static BOOL b_upd_subpage_p26 = FALSE;//flag when to use subpage_p26_nation
static UINT8 keystate =0;//only use in function below

//#define PRINTF_PAGE_PACKET
#ifdef PRINTF_PAGE_PACKET
static UINT16 draw_page_id = 0xffff; //page id
static UINT8 draw_page_line = 0xff;//page line
static UINT8 draw_page_col = 0xff;//page col
#endif

#if 1
static void start_end_byte_data(UINT8 skip_start8byte,UINT8 skip_end8byte,
    struct PBF_CB *cb,UINT8 line_num,UINT8 **data,UINT8 *start_byte,UINT8 *end_byte)
{
    if((NULL == data)||(NULL == cb)||(NULL == start_byte)||(NULL == end_byte)||(line_num>TTX_MAX_ROW))
    {
        return;
    }
    if(1 == skip_start8byte)
    {
        *start_byte = 8;
        *data = cb->buf_start + (40*line_num) +8;
    }
    else
    {
        *start_byte = 0;
        *data = cb->buf_start + (40*line_num);
    }
    if(1 == skip_end8byte)
    {
        *end_byte = 32;
    }
    else
    {
        *end_byte = 40;
    }

}
#endif

static UINT8 ttx_parse_line_mag(struct PBF_CB *cb)
{
    UINT8 mag = 0;

    if(TTX_TOTAL_MAGZINE==cb->page_id/100)
    {
        mag=0;
    }
    else
    {
        mag=cb->page_id/100;
    }
    return mag;
}

static void ttx_erase_unuseful_packet24(struct PBF_CB *cb,UINT8 *data,UINT8 line_num)
{
    UINT8 j = 0;
    UINT8 i = 0;

    if((TTX_MAX_ROW==line_num) && (1==cb->pack24_exist))
    {//cloud, if packet24 only have some no useful data, erase it
        for(i=0;i<40;i++)
        {
          if(*(data+i)>VBI_CHARACTER_SPACE)
          {
            j++;
          }
        }
        if(j<TTX_NORMAL_8)
        {
            MEMSET(data,0x20,40);
        }
    }
}

static void ttx_clean_line_real(UINT8 mag)
{
    UINT8 i = 0;
    UINT8 j = 0;

    for(i=1,j=0;i<24;i++)
    {
        if(1==g_packet_exist[mag][i])
        {
            if(TTX_NORMAL_0==j)
            {
                ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset,osd_ttx_width,i*TTX_CHAR_H,TTX_TRANSPARENT);//(0,i-1);
            }
            else
            {
                if(i-j>TTX_NORMAL_2)
                {
                    ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+(j+2)*TTX_CHAR_H,osd_ttx_width,
                        (i-j-2)*TTX_CHAR_H,TTX_TRANSPARENT);//(j+2,i-1);
                }
            }
            j=i;
        }
    }
    if((TTX_MAX_ROW==i) && (j<TTX_LAST_ROW_NUM))
    {
        if(j!=TTX_NORMAL_0)
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+(j+2)*TTX_CHAR_H,
                osd_ttx_width,(24-j-2)*TTX_CHAR_H,TTX_TRANSPARENT);//(j+2,24);
        }
        else
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset,osd_ttx_width,25*TTX_CHAR_H,TTX_TRANSPARENT);//(0,24);
        }
        ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+24*TTX_CHAR_H,osd_ttx_width,
            TTX_CHAR_H,TTX_TRANSPARENT);//line24;
        if(TRUE == g_ttx_pconfig_par.ttx_sub_page)
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+25*TTX_CHAR_H,osd_ttx_width,
                TTX_CHAR_H,TTX_TRANSPARENT);//line25;
        }
    }
}

static UINT8 ttx_clean_line(struct PBF_CB *cb,UINT8 *box_display,UINT8 *box_start,UINT8 mag,UINT8 line_num)
{
    if((NULL == cb)||(NULL == box_display)||(NULL == box_start))
    {
        return 0;
    }
    if((cb->subtitle)||(cb->newsflash))
    {
        if((0 == seperate_ttxsubtitle_mode)||(FALSE == get_ttxmenu_status()))
        {
            *box_display = 1;
            *box_start = 0;
            if(0 == line_num)
            {
                #if 0
                if((1==cb->erase_page)&&(1==cb->subtitle))
                {
                   ttx_set_screen_color(TTX_TRANSPARENT);
                }
                #endif
                //goto show_pageheader;//cloud
               // if(/*cb->SuppressHeader==1 &&*/ cb->InterruptedSequence==1 &&
                //    ((cb->ErasePage==0 && cb->MagazineSerial==1) ||
                 //   (cb->ErasePage==1 && cb->MagazineSerial==0)))
                //{//cloud,some streams transfer ttx-subtitle part by part
                  //like mutiplex c32_001.ts
               //    ;
               // }
               // else
                if(!((1==cb->interrupted_sequence)&&(((0==cb->erase_page) && (1==cb->magazine_serial)) ||
                ((1==cb->erase_page) && (0==cb->magazine_serial)))))
                {
                    screen_transparent_flag = 0;
                }
                return 0;
            }
            if(0 == screen_transparent_flag /*|| page_erase == TRUE*/)
            {
                if(TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number )
                {
                    ttx_set_back_alpha(transparent_value[TTX_TRANSPARENT_LEVEL-1]);
                    if((0==cb->magazine_serial) && (1==cb->subtitle) && (1==cb->suppress_header) && (0==cb->erase_page))
                    { //only clear the line which have no data to avoid refresh too quickly
                         ttx_clean_line_real(mag);
                    }
                    else
                    {
                        ttx_set_screen_color(TTX_B_BLACK);
                    }
                }
                //Ryan
                if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
                {
                    //TTX_SetScreenColor(TTX_TRANSPARENT);
                    if(1==cb->subtitle)
                    { //only clear the line which have no data to avoid refresh too quickly
                       if(b_ttx_subt_deal == FALSE)
                          ttx_clean_line_real(mag);
                    }
                    else
                    {
                        ttx_set_screen_color(TTX_TRANSPARENT);
                    }
                }
                screen_transparent_flag = 1;
                screen_transparent_flag2 = 1;
            }
        }
    }
    else
    {
        *box_display = 0;
        if(1 == screen_transparent_flag2)
        {
            if(TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number)
            {
                ttx_set_back_alpha(transparent_value[transparent_level]);
            }
            if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
            {
                ttx_set_screen_color(TTX_B_BLACK);
            }
            screen_transparent_flag2 = 0;
        }
    }
    return 1;
}

static void ttx_clean_extra_line(UINT8 line_num)
{
    if(line_num >TTX_MAX_COLUMN)
    {
        return;
    }
    if((TRUE==get_ttxmenu_status())&& (TRUE==get_ttxinfo_status()) )
    {
        if(TTX_MAX_ROW ==line_num)
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+26*TTX_CHAR_H,16,TTX_CHAR_H,TTX_TRANSPARENT);
            ttx_clear_screen(osd_ttx_xoffset+TTX_CHAR_W*39,osd_ttx_yoffset+26*TTX_CHAR_H,32,
                           TTX_CHAR_H,TTX_TRANSPARENT);//clear extra blank
        }
        show_ttx_info();
    }
}

static void ttx_change_header(struct PBF_CB *cb)
{
    //some stream have wrong header, force it to 1 ,cloud
    UINT8 i = 0;
    if(NULL == cb)
    {
        return;
    }
    for(i=0;i<g_subt_page_num;i++)
    {
       if((g_subt_page[i].page == cb->page_id) && (cb->subtitle!=1) && (g_subt_page[i].page!=g_init_page[i].page))
       {
           cb->subtitle=1;
       }
    }
}

#ifdef PRINTF_PAGE_PACKET
static void ttx_parse_line_print_data(struct PBF_CB *cb,UINT8 *data,UINT8 line_num,UINT8 j)
{
    if((NULL == cb)||(NULL == data)||(line_num > TTX_MAX_ROW)||(j> TTX_MAX_COLUMN))
    {
        return;
    }
    if( cb->page_id == draw_page_id)
    {
        soc_printf(" ");
        if(line_num == draw_page_line)
        {
            soc_printf("%x",(*data));
            if(j == draw_page_col)
            {
                soc_printf(" ",(*data));
            }
        }
    }
}
#endif

static void set_end_page_line_num(struct PBF_CB *cb,UINT8 *data,UINT8 line_num)
{
    if((NULL == cb)||(NULL == data)||(line_num > TTX_MAX_ROW))
    {
        return;
    }
    if((1 == cb->subtitle) && (line_num<=end_page_line_num))
    {
        b_useful_data = check_displayable_char(data,40);
        if(b_useful_data)
        {
            end_page_line_num = line_num;
        }
    }
}

static UINT8 ttx_set_real_mosiac(UINT8 hold_mosiac,UINT8 last_mosiac)
{
    UINT8 real_mosiac = 0;

    if(0 == hold_mosiac)
    {
        real_mosiac = 0x20;
    }
    else
    {
        real_mosiac = last_mosiac;
    }
    return real_mosiac;
}

static void parse_line_set_packet_exsit(UINT8 mag,UINT8 line_num,UINT8 real_height)
{
    if((mag > TTX_MAX_MAGZINE_NUM) ||(line_num > TTX_MAX_ROW) )
    {
        return;
    }
    if(g_ttx_pconfig_par.erase_unknown_packet==TRUE)
    {
        g_packet_exist[mag][line_num] = 1;
        if(1 == real_height)
        {
           g_packet_exist[mag][line_num+1] = 1;
        }
    }
    return;
}

static void parse_line_set_packet_exsit1(UINT8 mag,UINT8 line_num,UINT8 real_height)
{
    if((mag > TTX_MAX_MAGZINE_NUM) ||(line_num > TTX_MAX_ROW) )
    {
        return;
    }
    if(g_ttx_pconfig_par.erase_unknown_packet==TRUE)
    {
        g_packet_exist[mag][line_num+1] = 1;
        if( 1== real_height)
        {
            g_packet_exist[mag][line_num+2] = 1;
        }
    }
    return;
}

static void ttx_alpha_set(struct PBF_CB *cb,struct line_ctrl_bl *line_ctrl)
{
    if((NULL == line_ctrl)||(NULL == cb))
    {
        return;//Invalid param
    }
   language_select(cb->g0_set>>3,cb->nation_option,&(line_ctrl->alpha1_g1));
   language_select(cb->second_g0_set>>3,cb->second_g0_set&7,&(line_ctrl->alpha2_g1));
   language_select_g2(cb->g0_set>>3, cb->nation_option, &(line_ctrl->alpha_g2));
   line_ctrl->alpha1 = line_ctrl->alpha1_g1 | (line_ctrl->alpha_g2<<8);
   line_ctrl->alpha2 = line_ctrl->alpha2_g1 | (line_ctrl->alpha_g2<<8);
   line_ctrl->alpha = line_ctrl->alpha1;
}

static void ttx_parse_line_param(struct line_ctrl_bl *line_ctrl)
{
    if(NULL == line_ctrl)
    {
        return;//Invalid param
    }
    line_ctrl->alpha_mosiac = 1;
    line_ctrl->real_alpha_mosiac = line_ctrl->alpha_mosiac;
    line_ctrl->fg_color = TTX_F_WHITE;
    line_ctrl->real_fg_color = line_ctrl->fg_color;
    line_ctrl->pre_bg_color = TTX_B_WHITE;
    line_ctrl->bg_color = TTX_B_BLACK;
    line_ctrl->real_bg_color = line_ctrl->bg_color;
    line_ctrl->width = 0;
    line_ctrl->real_width = line_ctrl->width;
    line_ctrl->height = 0;
    line_ctrl->real_height = line_ctrl->height;
    line_ctrl->conceal = 0;
    line_ctrl->continous = 1;
    line_ctrl->hold_mosiac = 0;
    line_ctrl->real_mosiac =0x20;
    line_ctrl->last_mosiac =0x20;
}

static UINT8 parse_line_ctrl_word(struct line_ctrl_bl *line_ctrl,struct PBF_CB *cb,
    UINT8 tempdata, UINT8 *box_start,UINT8 line_num,UINT8 j,UINT8 *data,UINT8 *end_byte)
{//Colin

    if((NULL == line_ctrl)||(NULL == box_start)||(j>TTX_MAX_COLUMN)||(line_num > TTX_MAX_ROW))
    {
        return 0xFF;//Invalid param
    }
    switch(tempdata)
    {
    case 0x00://Black
    case 0x01://Red
    case 0x02://Green
    case 0x03://Yellow
    case 0x04://Blue
    case 0x05://Magenta
    case 0x06://Cyan
    case 0x07://White,  set fg color
            line_ctrl->hold_mosiac = 0;
            line_ctrl->alpha_mosiac = 1;
            line_ctrl->fg_color = tempdata;
            line_ctrl->pre_bg_color = line_ctrl->fg_color+8;
            if(1==line_ctrl->real_alpha_mosiac  )//alpha
            {
                line_ctrl->real_mosiac = 0x20;
            }
    break;
    case 0x0A://End Box
            *box_start = 0;
    if(((TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number) && (1==cb->subtitle) && (1==cb->suppress_header)
            &&(0==cb->erase_page)) && ((0==cb->magazine_serial)||((1==cb->magazine_serial)
            && (1==cb->update_indicator) &&(1==cb->interrupted_sequence))))
        //cloud, for stream 2010-1-28-12-37-205500-7-T.ts,need to erase lefted space
    {
            ttx_clear_screen((j+1)*TTX_CHAR_W+osd_ttx_xoffset,line_num*TTX_CHAR_H+osd_ttx_yoffset,
                (40-j)*TTX_CHAR_W,TTX_CHAR_H*2,TTX_B_BLACK);
    }
        //Ryan
    if((TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number) && (1==cb->subtitle )&& (1==cb->suppress_header) )
    {
        line_ctrl->clear_extra_data =0;
        if((0==cb->erase_page) && ((0==cb->magazine_serial)|| ((1==cb->magazine_serial) &&
                      (1==cb->update_indicator) && (1==cb->interrupted_sequence))))//cloud
        {
                    line_ctrl->clear_extra_data =1;
        }
        if((1==cb->erase_page) && (0==cb->magazine_serial) && (1==cb->interrupted_sequence))
        {
                    line_ctrl->clear_extra_data =1;
        }

        if(line_ctrl->clear_extra_data)
        {
                    ttx_clear_screen((j+1)*TTX_CHAR_W+osd_ttx_xoffset,line_num*TTX_CHAR_H+osd_ttx_yoffset,
                        (40-j)*TTX_CHAR_W,TTX_CHAR_H*2,TTX_TRANSPARENT);
                    if(  TTX_NORMAL_22==line_num)
                    {
                        ttxsubt_double_line_show = FALSE;
                    }
                }
        }
    break;
    case 0x0B://Start Box
        //if(prev_data == 0x0b || check_displayable_char(&tempdata, end_byte-j))
        if(check_displayable_char(data, (*end_byte)-j))
        {
                    *box_start = 1;
        }
        if((TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number) && (0==cb->magazine_serial) &&
                    (1==cb->subtitle) && (1==cb->suppress_header) && (0==cb->erase_page))
        {
                    ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+line_num*TTX_CHAR_H,
                        (j+1)*TTX_CHAR_W,TTX_CHAR_H*2,TTX_B_BLACK);
        }
        //Ryan
        if((TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)&& (0==cb->magazine_serial) && (1==cb->subtitle)
                    &&(1==cb->suppress_header)&& ((0==cb->erase_page) ||
                    ((1==cb->erase_page) && (1==cb->interrupted_sequence))))
        {
           
            if(b_ttx_subt_deal == FALSE)
            {
                    if(j<TTX_MAX_COL_IF_DOUBLE)
                    {
                        ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+line_num*TTX_CHAR_H,
                        (j+1)*TTX_CHAR_W,TTX_CHAR_H*2,TTX_TRANSPARENT);
                    }
                    else
                    {
                        ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+line_num*TTX_CHAR_H,
                        osd_ttx_width,TTX_CHAR_H*2,TTX_TRANSPARENT);//cloud, remove extra blank
                    }
            }
            
        }
    break;
    case 0x0c://Normal Size
        line_ctrl->hold_mosiac = 0;
        if( 1== line_ctrl->height)
        {
                    line_ctrl->width = 0x00;
                    line_ctrl->height=line_ctrl->width;
                    line_ctrl->draw_next_line = 1;
        }
        else
        {
                    line_ctrl->width = 0x00;
        }
        line_ctrl->real_width =line_ctrl->width;
        line_ctrl->real_height =line_ctrl->height;
        if( 1== line_ctrl->real_alpha_mosiac)//alpha
        {
                    line_ctrl->real_mosiac = 0x20;
        }
    break;
    case 0x0d://Double Height
        line_ctrl->hold_mosiac = 0;
        if(0 == line_ctrl->double_line)
        {
                    cb->complement_line |= (1<<(line_num+1));
                    line_ctrl->double_line = 1;
                    line_ctrl->draw_next_line = 1;
                    line_ctrl->alpha = line_ctrl->alpha1;//cloud,avoid change alpha twice
                    return 3;
                    //goto LineStart;
        }
        line_ctrl->height = 0x01;
        if(  1==line_ctrl->real_alpha_mosiac)//alpha
        {
                    line_ctrl->real_mosiac = 0x20;
        }
    break;
    case 0x0e://Double Width
        line_ctrl->hold_mosiac = 0;
        line_ctrl->width = 0;//0x01;  we don't support Double Width now
        if( 1== line_ctrl->real_alpha_mosiac)//alpha
        {
                    line_ctrl->real_mosiac = 0x20;
        }
    break;
    case 0x0f://Double Size
        line_ctrl->hold_mosiac = 0;
        if(0 == line_ctrl->double_line)
        {
                    cb->complement_line |= (1<<(line_num+1));
                    line_ctrl->double_line = 1;
                    line_ctrl->draw_next_line = 1;
                    line_ctrl->alpha = line_ctrl->alpha1;//cloud,avoid change alpha twice
                    return 3;
                    //goto LineStart;
        }
        line_ctrl->height=0x1;//width = 0x01;we don't support Double Width now
        line_ctrl->width =0;
        if( 1== line_ctrl->real_alpha_mosiac)//alpha
        {
                    line_ctrl->real_mosiac = 0x20;
        }
    break;
    case 0x10://Mosaics Black
    case 0x11://Mosaics Red
    case 0x12://Mosaics Green
    case 0x13://Mosaics Yellow
    case 0x14://Mosaics Blue
    case 0x15://Mosaics Magenta
    case 0x16://Mosaics Cyan
    case 0x17://Mosaics White,  Mosaic Colour Codes
        line_ctrl->alpha_mosiac = 0;
        line_ctrl->fg_color = *data - 0x10;
        line_ctrl->pre_bg_color = line_ctrl->fg_color+8;
    break;
    case 0x18://Conceal
        line_ctrl->conceal = 1;
    break;
    case 0x19://Contiguous Mosaic Graphics
        line_ctrl->continous = 0x01;
    break;
    case 0x1a://Separated Mosaic Graphics
        line_ctrl->continous = 0x00;
    break;
    case 0x1b://ESC
        line_ctrl->alpha = (line_ctrl->alpha==line_ctrl->alpha1)?line_ctrl->alpha2:line_ctrl->alpha1;
    break;
    case 0x1c://Black Background
        line_ctrl->bg_color = TTX_B_BLACK;
        line_ctrl->real_bg_color =line_ctrl->bg_color;
    break;
    case 0x1d://New Background
        line_ctrl->bg_color = line_ctrl->pre_bg_color;
        line_ctrl->real_bg_color =line_ctrl->bg_color;
    break;
    case 0x1e://Hold Mosaics
        line_ctrl->hold_mosiac = 1;
        line_ctrl->real_mosiac = line_ctrl->last_mosiac;
    break;
    case 0x1f://Release Mosaics
        line_ctrl->hold_mosiac = 0;
    break;
    default :
    break;
    }
    return 1;
}

void parse_line(struct PBF_CB *cb, UINT8 line_num,UINT8 skip_start8byte,UINT8 skip_end8byte)
{
    struct line_ctrl_bl line_ctrl;
    UINT8 j = 0;
    UINT8 mag = 0;
    UINT8 start_byte = 0;
    UINT8 end_byte = 0;
    UINT8 *data =NULL;
    UINT8 tempdata = 0;
    UINT8 box_display = 0;
    UINT8 box_start = 0;
    UINT8 display_enable = 1;
    UINT8 double_width_hold = 0;
//    UINT8 prev_data = 0x20;
    UINT8 p26_char_set =0;
    UINT8 ret = 0;
    UINT16 char_set = 0;

	UINT8 ttx_flash_flag =0;
    UINT8 ttx_flash_start_column = 0; 
	
    UINT32 u_left = 0;
    UINT32 u_top = 0;
    UINT32 u_height = 0;
    ttx_flash_node *p_node= NULL;
    MEMSET(&line_ctrl,0,sizeof(struct line_ctrl_bl));
    line_ctrl.alpha = 1;
    line_ctrl.mosiac=0xb0;

    if(NULL == cb)
    {
        return;
    }
    mag = ttx_parse_line_mag(cb);
    ttx_alpha_set(cb,&line_ctrl);
    if(((line_num != 0) &&(1 == cb->inhibit_display))||((1<<line_num)&cb->complement_line))
    {
        return;
    }
    cb->complement_line &= ~(1<<(line_num+1));
line_start:
    ttx_parse_line_param(&line_ctrl);
    if(( 1== line_ctrl.double_line) && (TTX_MAX_ROW == line_num))
    {
        line_ctrl.double_line = 0;
        return;
    }
    start_end_byte_data(skip_start8byte,skip_end8byte,cb,line_num,&data,&start_byte,&end_byte);
    ttx_change_header(cb);
    ret = ttx_clean_line(cb,&box_display,&box_start,mag,line_num);
    if(0 == ret)
    {
        return;
    }
    ttx_clean_extra_line(line_num);
//show_pageheader:
    if((TRUE==g_ttx_pconfig_par.ttx_sub_page) &&(TRUE==b_upd_page)&&(0==skip_end8byte)&&(0==line_num))
    {//only update time with style
        start_byte=31;
        if(1==skip_start8byte)
        {
            data += 23;
        }
        else
        {
            data += 31;
        }
        while(*(data-1)<0x20)
        {
            data--;
            start_byte--;
        }
    }
    if((1 ==cb->subtitle) && (1 == seperate_ttxsubtitle_mode) && (TRUE== get_ttxmenu_status()))
    {
        if(line_num!=0)
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset,osd_ttx_width,26*TTX_CHAR_H,TTX_B_BLACK);//(0,25);
        }
        return;
    }
    ttx_erase_unuseful_packet24(cb,data,line_num);
    set_end_page_line_num(cb,data,line_num);
    for(j=start_byte;j<end_byte;j++,data++)
    {
#ifdef PRINTF_PAGE_PACKET
         ttx_parse_line_print_data(cb,data,line_num,j);
#endif
        tempdata=(*data)&0x7F;
        display_enable = (1 == box_display)?box_start:1;
        if (tempdata < VBI_CHARACTER_SPACE)
        {
            line_ctrl.real_mosiac = ttx_set_real_mosiac(line_ctrl.hold_mosiac,line_ctrl.last_mosiac);
            ret = parse_line_ctrl_word(&line_ctrl,cb,tempdata,&box_start,line_num,j,data,&end_byte);
#ifdef TTX_FLASH_FUNC

			if(tempdata==8)
			{
				ttx_flash_flag = 1;//value 1 means have start flag
				ttx_flash_start_column = j;
				if(ttx_flash_test_num==0)
				{
					ttx_flash_task_creat();
					ttx_flash_test_num = 1;
				}		 
			}					
			else if(tempdata==9)
		
				ttx_flash_flag = 2;//value 2 means have end flag
						
#endif
            if(VBI_RET_LOOP_BEFORE == ret)
            {
               goto line_start;
            }

            if( 0== double_width_hold)
            {
                if(0== line_ctrl.real_alpha_mosiac)
                {
                    if( 1==display_enable)
                    {
                        g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, line_num*TTX_CHAR_H,
                        line_ctrl.mosiac, line_ctrl.real_fg_color, line_ctrl.real_bg_color,
                        line_ctrl.real_width, line_ctrl.real_height, line_ctrl.real_mosiac, 0);
                        parse_line_set_packet_exsit(mag,line_num,line_ctrl.real_height);
                    }
                }
                else
                {
                    if( 1==display_enable)
                    {
                        g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, line_num*TTX_CHAR_H, line_ctrl.mosiac,
                        line_ctrl.real_fg_color, line_ctrl.real_bg_color, line_ctrl.real_width,
                        line_ctrl.real_height, 0x20, 0);
                        parse_line_set_packet_exsit(mag,line_num,line_ctrl.real_height);
                    }
                }
                if(( 1== line_ctrl.draw_next_line)&&( 1==display_enable))
                {
                    g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, (line_num+1)*TTX_CHAR_H,
                    line_ctrl.mosiac, line_ctrl.real_fg_color, line_ctrl.real_bg_color,
                    line_ctrl.real_width, line_ctrl.real_height, 0x20,0);
                    parse_line_set_packet_exsit1(mag,line_num,line_ctrl.real_height);
                }
            }
            else
            {
                double_width_hold = 0;
            }
        }
        else if ((tempdata >= 0x20) && (tempdata < 0x80))
        {
            if(1== line_ctrl.alpha_mosiac )
            {
                char_set = line_ctrl.alpha;
            }
            else
            {
                if((0x40==tempdata)||((tempdata>=0x5b)&&(tempdata<=0x5f)))
                {
                    char_set = line_ctrl.alpha;
                }
                else
                {
                    char_set = line_ctrl.mosiac;
                }
                if(1== line_ctrl.hold_mosiac)
                {
                    if((tempdata<0x41) || (tempdata>0x5a))
                    {
                        line_ctrl.last_mosiac = tempdata;
                    }
                }
                else
                {
                    line_ctrl.last_mosiac = tempdata;
                }
            }
            if(1==display_enable)
            {
                if(g_ttx_pconfig_par.parse_packet26_enable==TRUE)
                {
                    if((FALSE == b_upd_subpage_p26) || (FALSE == g_ttx_pconfig_par.ttx_sub_page))
                    {
                        p26_char_set = get_packet26_nation_map(cb->page_id, line_num, j);
                    }
                    else
                    {
                        p26_char_set = get_subpage_nation_map(cb->sub_page_id, line_num, j);
                    }
                }
                mosaic_mode = line_ctrl.continous;
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, line_num*TTX_CHAR_H, char_set,
                line_ctrl.real_fg_color, line_ctrl.real_bg_color, line_ctrl.real_width,
                line_ctrl.real_height, *data, p26_char_set);
                parse_line_set_packet_exsit(mag,line_num,line_ctrl.real_height);
            }
            if(( 1== line_ctrl.draw_next_line)&&( 1==display_enable))
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W,
                (line_num+1)*TTX_CHAR_H, line_ctrl.mosiac, line_ctrl.real_fg_color,
                line_ctrl.real_bg_color, line_ctrl.real_width, line_ctrl.real_height, 0x20,0);
                parse_line_set_packet_exsit1(mag,line_num,line_ctrl.real_height);
            }
            if( 1==line_ctrl.real_width)
            {
                double_width_hold = 1;
            }
		#ifdef TTX_FLASH_FUNC	
			if((TRUE == ttx_show_on)&&(1==ttx_flash_flag)&&(NULL == p_node)&&(j>=ttx_flash_start_column)&&((last_cur_page!=cur_page)||(last_cur_subpage!=cur_subpage)))
            {
                p_node=(ttx_flash_node *)malloc(sizeof(ttx_flash_node));
                if(NULL == p_node)
                {
                }
                else
                {
                	
					
                    u_left =j*TTX_CHAR_W+osd_ttx_xoffset;
                    u_top = line_num*TTX_CHAR_H+osd_ttx_yoffset;
                    if(line_ctrl.real_height== 1) 
                        u_height = TTX_CHAR_H*2;
                    else
                        u_height = TTX_CHAR_H;
                   osal_mutex_lock(ttx_flash_mutex, OSAL_WAIT_FOREVER_TIME);
                    p_node->left =u_left;
                    p_node->top = u_top;
                    p_node->width = TTX_CHAR_W;
                    p_node->height = u_height;
                    p_node->fg_color =line_ctrl.real_fg_color;
                    p_node->bg_color = line_ctrl.real_bg_color;
                    p_node->next = ttx_flash_list;
                    ttx_flash_list = p_node;
                    p_node = NULL;
                    osal_mutex_unlock(ttx_flash_mutex);
                }
            }
		#endif
        }
        else
        {
        	return;
        }
        //prev_data = tempdata;
        line_ctrl.real_alpha_mosiac = line_ctrl.alpha_mosiac;
        line_ctrl.real_fg_color = line_ctrl.fg_color;
        line_ctrl.real_bg_color =line_ctrl.bg_color;
        line_ctrl.real_width =line_ctrl.width;
        line_ctrl.real_height =line_ctrl.height;
        if(1==line_ctrl.height)
        {
            line_ctrl.draw_next_line = 0;
        }
    }
}

static UINT8 ttx_send_num_key( struct vbi_device *ttx_dev,TTX_KEYTYPE key )
{
    UINT8 i = 0;
    UINT8 ch = 0;
    UINT16 left = 0;
    UINT16 top = 0;
//    INT32 ret_func = 0;
    INT32 ret = 0;

    if((NULL == ttx_dev)|| (key > TTX_KEY_NORESPONSE))
    {
        return 0xFF;
    }
    ch = key2char(key);
    if((TTX_NORMAL_0==keystate) && (ch >0x30) && (ch<0x39))
    {
        //TTX_SetRegionColor(14,0,42,20,TTX_WHITE);
        left = TTX_CHAR_W;
        top = 0;
        req_page = (ch-0x30)*100;
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE, TTX_B_BLACK,0,0,ch,0);
        left = TTX_CHAR_W*2;
        ch = '-';
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE, TTX_B_BLACK,0,0,ch,0);
        left = TTX_CHAR_W*3;
        ch = '-';
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE, TTX_B_BLACK,0,0,ch,0);
        keystate= 1;
        return 2;
        //goto EXIT_POINT;
    }
    else if(TTX_NORMAL_1 == keystate)
    {
        //TTX_SetRegionColor(28,0,28,20,TTX_WHITE);
        left = TTX_CHAR_W*2;
        top = 0;
        req_page += ((ch-0x30)*10);
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        left = TTX_CHAR_W*3;
        ch = '-';
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        keystate= 2;
        return 2;
       // goto EXIT_POINT;
    }
    else if(TTX_NORMAL_2 == keystate)
    {
        //TTX_SetRegionColor(42,0,14,20,TTX_WHITE);
        left = TTX_CHAR_W*3;
        top = 0;
        req_page += (ch-0x30);
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        keystate= 0;
    }
    else
    {
        return 2;
        //goto EXIT_POINT;
    }
        //LIBTTX_PRINTF("2. ttx_request_page %d \n",req_page);
    if(req_page!=0xffff)
    {
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            clear_sub_page_cb();
        }
        ret = ttx_request_page(ttx_dev,req_page , &p_cur_rd_cb );
        if (RET_SUCCESS == ret)
        {
        #ifdef TTX_FLASH_FUNC
        	ttx_delete_whole();
            same_cur_subpage = 0xffff;
		#endif
            wait_page =0xffff;
            cur_page = p_cur_rd_cb->page_id;
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                cur_subpage = p_cur_rd_cb->sub_page_id;
                b_upd_page = FALSE;//
            }
            if(((p_cur_rd_cb->subtitle)||(p_cur_rd_cb->newsflash)) && ((0==seperate_ttxsubtitle_mode)
                ||(FALSE==get_ttxmenu_status())))
            {
                ttx_set_screen_color(TTX_TRANSPARENT);
                //MEMSET(last_draw_data, 0xFF, 24*40);
            }
            else
            {
                if(TTX_COLOR_NUMBER_256 == g_ttx_pconfig_par.ttx_color_number)
                {
                    ttx_clear_screen(osd_ttx_xoffset, osd_ttx_yoffset+TTX_CHAR_H, osd_ttx_width,
                        TTX_CHAR_H*24,TTX_B_BLACK);
                    //MEMSET(last_draw_data, 0xFF, 24*40);
                }
            }
            parse_line(p_cur_rd_cb,0,0,1);
            for(i=1;i<25;i++)
            {
                parse_line(p_cur_rd_cb,i,0,0);
            }
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb, 0, 24,FALSE);
            }
        }
        else
        {
            //LIBTTX_PRINTF("ttx_request_page failure!\n");
            wait_page = req_page;
        }
    }
    if(TRUE == g_ttx_pconfig_par.ttx_sub_page)
    {
        show_subpage_status();
    }
    return 2;
    //goto EXIT_POINT;
}

static UINT8 ttx_send_up_key(struct vbi_device *ttx_dev)
{
    UINT8 i = 0;
    INT32 ret = RET_FAILURE;

    if(NULL == ttx_dev)
    {
        return 0xFF;//ERROR
    }

    keystate= 0;
    if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
    {
        clear_sub_page_cb();
    }
    if(wait_page!=0xffff)
    {
        ret = ttx_request_page_up(ttx_dev,wait_page , &p_cur_rd_cb );
    }
    else
    {
        ret = ttx_request_page_up(ttx_dev,cur_page , &p_cur_rd_cb );
    }

    if (RET_SUCCESS == ret)
    {
	#ifdef TTX_FLASH_FUNC
		ttx_delete_whole();
        same_cur_subpage = 0xffff;
	#endif
        wait_page =0xffff;
        cur_page = p_cur_rd_cb->page_id;
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            cur_subpage = p_cur_rd_cb->sub_page_id;
            b_upd_page = FALSE;//
        }
        if(((p_cur_rd_cb->subtitle)||(p_cur_rd_cb->newsflash)) && ((0 == seperate_ttxsubtitle_mode)
            ||(FALSE == get_ttxmenu_status())))
        {
            ttx_set_screen_color(TTX_TRANSPARENT);
            //MEMSET(last_draw_data, 0xFF, 24*40);
        }
        else
        {
            if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
            {
                ttx_clear_screen(osd_ttx_xoffset, osd_ttx_yoffset+TTX_CHAR_H,osd_ttx_width,TTX_CHAR_H*24,TTX_B_BLACK);
                //MEMSET(last_draw_data, 0xFF, 24*40);
            }
        }
        p_cur_rd_cb->complement_line = 0; //Ryan
        parse_line(p_cur_rd_cb,0,0,1);
        for(i=1;i<25;i++)
        {
            parse_line(p_cur_rd_cb,i,0,0);
        }

        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            ret = set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb, 0, 24, FALSE);
        }
    }
    else
    {
        wait_page = req_page;
    }
    if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
    {
        show_subpage_status();
    }
    return 2;
}

static UINT8 ttx_send_down_key(struct vbi_device *ttx_dev)
{
    INT32 ret = 0;
    UINT8 i = 0;

    if(NULL == ttx_dev)
    {
        return 0xFF;//ERROR
    }
    keystate= 0;
    //LIBTTX_PRINTF("KEY_DOWN %d \n", cur_page);
    if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
    {
        clear_sub_page_cb();
    }
    if(wait_page!=0xffff)
    {
        ret = ttx_request_page_down(ttx_dev,wait_page , &p_cur_rd_cb );
    }
    else
    {
        ret = ttx_request_page_down(ttx_dev,cur_page , &p_cur_rd_cb );
    }

    if (RET_SUCCESS == ret)
    {
        //LIBC_PRINTF("page_down Success!\n");
	#ifdef TTX_FLASH_FUNC
		ttx_delete_whole();
        same_cur_subpage = 0xffff;
	#endif
        wait_page =0xffff;
        cur_page = p_cur_rd_cb->page_id;
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            cur_subpage = p_cur_rd_cb->sub_page_id;
            b_upd_page = FALSE;//
        }
        if(((p_cur_rd_cb->subtitle)||(p_cur_rd_cb->newsflash)) && ((0 == seperate_ttxsubtitle_mode)
            ||(FALSE == get_ttxmenu_status())))
        {
            ttx_set_screen_color(TTX_TRANSPARENT);
            //MEMSET(last_draw_data, 0xFF, 24*40);
        }
        else
        {
            if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
            {
                ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+TTX_CHAR_H,osd_ttx_width,TTX_CHAR_H*24,TTX_B_BLACK);
                //MEMSET(last_draw_data, 0xFF, 24*40);
            }
        }
        p_cur_rd_cb->complement_line = 0; //Ryan
        parse_line(p_cur_rd_cb,0,0,1);
        for(i=1;i<25;i++)
        {
            parse_line(p_cur_rd_cb,i,0,0);
        }
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            ret = set_sub_page_cb(p_cur_rd_cb->sub_page_id,p_cur_rd_cb,0,24,FALSE);
        }
    }
    else
    {
        //LIBC_PRINTF("page_down fail!\n");
        wait_page = req_page;
    }
    if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
    {
        show_subpage_status();
    }
    return 2;
    //goto EXIT_POINT;
}

static UINT8 ttx_send_right_key(void)
{
    UINT8 subpage = 0;
    UINT8 i = 0;
    INT32 ret = RET_FAILURE;

    //ttx_gen_char_code();
    keystate= 0;
    //LIBC_PRINTF("TTX_KEY_RIGHT, WaitPg=%d,WaitSubPG=%d,cur_pg=%d,cur_subpg=%d\n", WaitPage,
    //WaitSubPage,cur_page,cur_subpage);
    if((0xffff == cur_subpage) || (TTX_SUB_PAGE_MAX_NUM == cur_subpage))
    {
        return 2;
        //goto EXIT_POINT;
    }
    subpage = cur_subpage;
    ret = ttx_request_sub_page_up(cur_page , subpage, &p_cur_rd_cb );
    if (RET_SUCCESS == ret)
    {
        //LIBC_PRINTF("subpage_up Success!\n");
        #ifdef TTX_FLASH_FUNC
        	ttx_delete_whole();
		#endif
        b_upd_subpage_p26 = TRUE;
        wait_sub_page =0xffff;
        cur_page = p_cur_rd_cb->page_id;
        cur_subpage = p_cur_rd_cb->sub_page_id;
        if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_clear_screen(osd_ttx_xoffset,
            osd_ttx_yoffset+TTX_CHAR_H,osd_ttx_width,
            TTX_CHAR_H*24,TTX_B_BLACK);
            //MEMSET(last_draw_data, 0xFF, sizeof(last_draw_data));
        }
        parse_line(p_cur_rd_cb,0,0,1);
        for(i=1;i<25;i++)
        {
            //LIBC_PRINTF("sub_page_up,i=%d\n",i);
            parse_line(p_cur_rd_cb,i,0,0);
        }
        wait_page = p_cur_rd_cb->page_id;
        b_upd_subpage_p26 = FALSE;
        show_subpage_status();
    }
    else
    {
        //LIBC_PRINTF("subpage_up Failure!\n");
        if((subpage < TTX_SUB_PAGE_MAX_NUM) && (subpage >0))
        {
            wait_sub_page=subpage+1;
        }
    }
    if(cur_subpage!=0)
    {
        b_upd_page = TRUE;
    }
    else
    {
        b_upd_page = FALSE;
    }
    //LIBC_PRINTF("TTX_KEY_RIGHT,WaitSubPage=%d\n",WaitSubPage);
    return 2;
}

static UINT8 ttx_send_left_key(void)
{
    UINT8 subpage = 0;
    UINT8 i = 0;
    INT32 ret = RET_FAILURE;

    keystate= 0;
    //LIBC_PRINTF("WaitPg=%d,WaitSubPg=%d,cur_pg=%d,cur_subpg=%d\n",WaitPage,WaitSubPage,cur_page,cur_subpage);
    if((0==cur_subpage )||(0xffff==cur_subpage)||((TTX_F_RED==status_line_data[2])&&(0x41== status_line_data[3])))
    {
        return 2;
    }
   // LIBC_PRINTF("begin to subpage down!!!\n");
    subpage = cur_subpage;
    ret = ttx_request_sub_page_down(cur_page, subpage , &p_cur_rd_cb );

    if (RET_SUCCESS == ret)
    {
        //LIBC_PRINTF("subpage_down Success!\n");
        #ifdef TTX_FLASH_FUNC
        	ttx_delete_whole();
		#endif
        b_upd_subpage_p26 = TRUE;
        wait_sub_page =0xffff;
        cur_page = p_cur_rd_cb->page_id;
        cur_subpage = p_cur_rd_cb->sub_page_id;
        if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+TTX_CHAR_H,
            osd_ttx_width,TTX_CHAR_H*24,TTX_B_BLACK);
            //MEMSET(last_draw_data, 0xFF, 24*40);
        }
        parse_line(p_cur_rd_cb,0,0,1);
        for(i=1;i<25;i++)
        {
            //LIBC_PRINTF("sub_page_down,i=%d\n",i);
            parse_line(p_cur_rd_cb,i,0,0);
        }
        wait_page = p_cur_rd_cb->page_id;
        b_upd_subpage_p26 = FALSE;
        show_subpage_status();
        if(cur_subpage!=0)
        {
            b_upd_page = TRUE;
        }
        else
        {
            b_upd_page = FALSE;
        }
    }
    else
    {
        //LIBC_PRINTF("subpage_down Failure!\n");
        wait_sub_page =0xffff;
        wait_page = p_cur_rd_cb->page_id;
        b_upd_page = FALSE;
    }
    //LIBC_PRINTF("TTX_KEY_LEFT,WaitSubPage=%d\n",WaitSubPage);
    return 2;
}

static UINT8 ttx_send_color_key(TTX_KEYTYPE key)
{
    if(TTX_KEY_RED == key)
    {
        keystate= 0;
        if(cur_page != 0xffff)
        {
            //LIBTTX_PRINTF("red, pCurRdCB->page_id = %d \n",pCurRdCB->page_id);
            req_page = p_cur_rd_cb->link_red;
            if(0xffff== req_page)
            {
                if((TRUE==g_ttx_pconfig_par.user_fast_text) &&(0 == p_cur_rd_cb->pack24_exist)
                    &&(queue_tag>=TTX_QUEUE_TAG_1))
                {
                    req_page = page_queue[0];
                }
                else
                {
                    return 2;//goto EXIT_POINT;
                }

            }
        }
        else
        {
            return 2;//goto EXIT_POINT;
        }

    }
    else if(TTX_KEY_GREEN == key)
    {
        keystate= 0;
        if(cur_page != 0xffff)
        {
            //LIBTTX_PRINTF("green, pCurRdCB->page_id = %d \n",pCurRdCB->page_id);
            req_page = p_cur_rd_cb->link_green;
            if( 0xffff== req_page)
            {
                if((TRUE==g_ttx_pconfig_par.user_fast_text) &&(0 == p_cur_rd_cb->pack24_exist)
                    &&(queue_tag>TTX_QUEUE_TAG_1))
                {
                    req_page = page_queue[1];
                }
                else
                {
                    return 2;//goto EXIT_POINT;
                }
            }
        }
        else
        {
            return 2;//goto EXIT_POINT;
        }

    }
    else if(TTX_KEY_YELLOW == key)
    {
        keystate= 0;
        if(cur_page != 0xffff)
        {
            //LIBTTX_PRINTF("yellow, pCurRdCB->page_id = %d \n",pCurRdCB->page_id);
            req_page = p_cur_rd_cb->link_yellow;
            if( 0xffff== req_page)
            {
                if((TRUE==g_ttx_pconfig_par.user_fast_text) &&(0 == p_cur_rd_cb->pack24_exist)
                    &&(queue_tag>TTX_QUEUE_TAG_2))
                {
                    req_page = page_queue[2];
                }
                else
                {
                    return 2;//goto EXIT_POINT;
                }
            }
        }
        else
        {
           return 2;// goto EXIT_POINT;
        }
     }
    else//key == TTX_KEY_CYAN
    {
        keystate= 0;
        if(cur_page != 0xffff)
        {
            //LIBTTX_PRINTF("cyan, pCurRdCB->page_id = %d \n",pCurRdCB->page_id);
            req_page = p_cur_rd_cb->link_cyan;
            if( 0xffff== req_page)
            {
                if((TRUE==g_ttx_pconfig_par.user_fast_text) &&(0 == p_cur_rd_cb->pack24_exist)
                    &&(queue_tag>TTX_QUEUE_TAG_3))
                {
                    req_page = page_queue[3];
                }
                else
                {
                    return 2;//goto EXIT_POINT;
                }
            }
        }
        else
        {
            return 2;//goto EXIT_POINT;
        }
    }
    return 0;
}

static void ttx_send_inc_dec_key(TTX_KEYTYPE key)
{
    if(TTX_KEY_INC1==key)
    {
        keystate= 0;
        if(wait_page!=0xffff)
        {
            req_page = (899==wait_page)?100:(wait_page+1);
        }
        else if(cur_page != 0xffff)
        {
            req_page = (899==cur_page)?100:(cur_page+1);
        }
    }
    else if(TTX_KEY_DEC1 == key)
    {
        keystate= 0;
        if(wait_page!=0xffff)
        {
            req_page = (100==wait_page)?899:(wait_page-1);
        }
        else if(cur_page != 0xffff)
        {
            req_page = (100==cur_page)?899:(cur_page-1);
        }
    }
    else if(TTX_KEY_INC100 == key)
    {
        keystate= 0;
        if(wait_page!=0xffff)
        {
            req_page = (wait_page>799)?(wait_page-700):(wait_page+100);
        }
        else if(cur_page != 0xffff)
        {
            req_page = (cur_page>799)?(cur_page-700):(cur_page+100);
        }
    }
    else //key == TTX_KEY_DEC100
    {
        keystate= 0;
        if(wait_page!=0xffff)
        {
            req_page = (wait_page<200)?(wait_page+700):(wait_page-100);
        }
        else if(cur_page != 0xffff)
        {
            req_page = (cur_page<200)?(cur_page+700):(cur_page-100);
        }
    }
}

void ttx_eng_send_key(struct vbi_device *ttx_dev, TTX_KEYTYPE key)
{
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 huds = 0;
    UINT8 tens = 0;
    UINT8 ones = 0;
    UINT8 queue_inc = 0;
    UINT8 func_ret = 0;
    UINT16 left = 0;
    UINT16 top = 0;
    UINT16 last_page = 0;
    UINT16 swap_page = 0;
    INT32 ret = RET_FAILURE;

    if(NULL == ttx_dev)
    {
        return;
    }
    ENTER_TTX_ENG();
    if(TTXENG_OFF == ttx_eng_state)
    {
        LEAVE_TTX_ENG();
        return;
    }
    //osal_task_dispatch_off();
    if(g_ttx_pconfig_par.user_fast_text==TRUE)
    {
        last_page = cur_page;
    }
    if (TTX_KEY_ALPHA == key)
    {
        if(transparent_level >= TTX_TRANSPARENT_LEVEL-1)
        {
            transparent_level = 0;
        }
        else
        {
            transparent_level++;
        }
        ttx_set_back_alpha(transparent_value[transparent_level]);
        goto EXIT_POINT;
    }
    else if((TTX_KEY_NUM0==key)||(TTX_KEY_NUM1==key)||(TTX_KEY_NUM2==key)||(TTX_KEY_NUM3==key)||(TTX_KEY_NUM4==key)||
        (TTX_KEY_NUM5==key)||(TTX_KEY_NUM6==key)||(TTX_KEY_NUM7==key)||(TTX_KEY_NUM8==key)||(TTX_KEY_NUM9==key))
    {
        func_ret = ttx_send_num_key(ttx_dev,key);
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
/*.......................up & dowm ............................*/
    else if(TTX_KEY_UP == key)
    {
        func_ret = ttx_send_up_key(ttx_dev);
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
    else if(TTX_KEY_DOWN == key)
    {
        func_ret = ttx_send_down_key(ttx_dev);
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
/*.......................left & right ............................*/
    else if((TTX_KEY_RIGHT == key) && (TRUE == g_ttx_pconfig_par.ttx_sub_page))
    {
        func_ret = ttx_send_right_key();
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
    else if(TTX_KEY_LEFT == key)
    {
        func_ret = ttx_send_left_key();
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
/*.......................color ............................*/
    else if((TTX_KEY_RED == key)||(TTX_KEY_GREEN == key)||(TTX_KEY_YELLOW == key)||(TTX_KEY_CYAN == key))
    {
        func_ret = ttx_send_color_key(key);
        if(TTX_NORMAL_2 == func_ret)
        {
            goto EXIT_POINT;
        }
    }
    else if((TTX_KEY_INC1 == key)||(TTX_KEY_DEC1 == key) ||(TTX_KEY_INC100 == key) ||(TTX_KEY_DEC100 == key))
    {
        ttx_send_inc_dec_key(key);
    }
    else
    {
        goto EXIT_POINT;
    }
    if(req_page!=0xffff)
    {
        left = 0;
        top = 0;
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,'P',0);
        left = TTX_CHAR_W;
        top = 0;
        huds = (req_page/100);
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,huds+0x30,0);
        left = TTX_CHAR_W*2;
        tens = (req_page-100*huds)/10;
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,tens+0x30,0);
        left = TTX_CHAR_W*3;
        ones = req_page-100*huds-10*tens;
        g_ttx_pconfig_par.ttx_drawchar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ones+0x30,0);

        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            clear_sub_page_cb();
        }
        ret = ttx_request_page(ttx_dev,req_page , &p_cur_rd_cb );
        if (RET_SUCCESS == ret)
        {
            wait_page =0xffff;
            cur_page = p_cur_rd_cb->page_id;
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                cur_subpage = p_cur_rd_cb->sub_page_id;
                b_upd_page = b_false;
            }
            parse_line(p_cur_rd_cb,0,0,1);
            for(i=1;i<25;i++)
            {
                parse_line(p_cur_rd_cb,i,0,0);
            }
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                ret = set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb, 0, 24, FALSE);
            }
        }
        else
        {
            wait_page = req_page;
        }
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            show_subpage_status();
        }
    }
EXIT_POINT:
    if(g_ttx_pconfig_par.user_fast_text==TRUE)
    {
        if(new_history_mode!=TRUE)
        {
            if(last_page != 0xffff)
            {
                queue_inc =1;
                for(i=0;i<queue_tag;i++)
                {
                    if((page_queue[i] == last_page) && (last_page!=cur_page))
                    {
                        for(j=i;j<queue_tag;j++)
                        {
                            swap_page = page_queue[j];
                            page_queue[j] = page_queue[queue_tag-1];
                            page_queue[queue_tag-1] = swap_page;
                        }
                        queue_inc = 0;
                        break;
                    }
                }
                if((0 != queue_inc)&&(last_page!=cur_page))
                {
                    if(TTX_QUEUE_TAG_4 == queue_tag)
                    {
                        page_queue[0] = page_queue[1];
                        page_queue[1] = page_queue[2];
                        page_queue[2] = page_queue[3];
                        page_queue[3] = last_page;
                    }
                    else
                    {
                        page_queue[queue_tag] = last_page;
                        queue_tag++;
                    }
                }
            }
        }
        else
        {
            set_packet24_new(ttx_dev, last_page);
        }
        if(p_cur_rd_cb &&(0 == p_cur_rd_cb->pack24_exist))
        {
            parse_line_ext(p_cur_rd_cb);
        }
     }
    LEAVE_TTX_ENG();
    return;
}
