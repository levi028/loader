/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: ttx_dec.c

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

#ifdef TTX_BY_OSD
#define TTXDEC_PRINTF PRINTF

struct ttx_config_par g_ttx_pconfig_par;// ={0,};
struct PBF_CB *p_cur_rd_cb = NULL;
struct t_ttx_lang g_init_page[TTX_SUBT_LANG_NUM] = {{0,0,{0},0},};
struct t_ttx_lang g_subt_page[TTX_SUBT_LANG_NUM] = {{0,0,{0},0},};

//extern UINT8 *g_ttx_p26_nation;
extern UINT32 osd_ttx_width;
extern UINT32 osd_ttx_xoffset;
extern UINT32 osd_ttx_yoffset;
#ifdef TTX_FLASH_FUNC

UINT16 same_cur_subpage = 0xffff;
extern UINT16 last_cur_subpage;
extern UINT16 last_cur_page;

#endif

#ifdef SEE_CPU
OSAL_ID TTX_SEM_ID = INVALID_ID;
#define ENTER_TTX_ENG() \
osal_semaphore_capture(TTX_SEM_ID,OSAL_WAIT_FOREVER_TIME)
#define LEAVE_TTX_ENG() osal_semaphore_release(TTX_SEM_ID)
#else
#define ENTER_TTX_ENG()
#define LEAVE_TTX_ENG()
#endif
OSAL_ID    ttx_subt_alarm_id = OSAL_INVALID_ID;    // used for ttx subt clear

UINT8 ttx_eng_state = TTXENG_OFF;
UINT8 transparent_level = 0;
UINT8 transparent_value[TTX_TRANSPARENT_LEVEL] = {0};
UINT8 *subpage_p26_nation = NULL;//to store subpage p26 naton map
UINT8 queue_tag = 0;
UINT8 g_subt_page_num = 0;
UINT8 screen_transparent_flag=0;
UINT8 screen_transparent_flag2=0;
UINT8 status_line_data[40];//to show subpage status
UINT8 mosaic_mode = 1;//1:continue mode,  0:separate mode
//UINT8 last_draw_data[24][40] = {{0}};//line1 - line24, not include line 0
UINT8 seperate_ttxsubtitle_mode = 0;

UINT16 cur_page = 0xffff;
UINT16 wait_page = 0xffff; //..100 - 899
UINT16 req_page = 100; //..100 - 899
UINT16 page_queue[4] = {0};
UINT16 wait_sub_page = 0;//0...79
UINT16 cur_subpage = 0;//0...79

UINT32 end_page_line_num = 24;

BOOL b_upd_page = FALSE;
BOOL new_history_mode = FALSE;//
BOOL b_useful_data  = FALSE;

static struct PBF_CB * g_sub_pages[80] = {NULL,};//store all subpages of one page
static UINT8 last_line_id = 0;
static UINT8 g_current_language[3] = {0};
static UINT32 g_dsg_font_idx = 0xFFFFFFFF;
static UINT32 last_temp_tick = 0;
static UINT32 end_line_of_the_page = 0;

static UINT8 link_line_data[40] ={0};
UINT8 last_page_data[1000] = {0};
BOOL last_page_useful_line[25] = {FALSE};
BOOL b_ttx_subt_deal = FALSE;


/*
force ttx use font from special index for
some special stream without lang info, cloud
#define CALC_NATION_POS(subpage, line, i)\
((subpage)*(25*20)+line*20+(i>>1))
#define GET_SUBPAGE_NATION_MAP(subpage, line, i)\
((subpage_p26_nation[CALC_NATION_POS(subpage, line, i)]>>((i&1)*4))&0xf)
*/


static BOOL data_part_same_flag(UINT8* data1,UINT8* data2,UINT32 length)
{
    UINT8 i = 0;

    for(i=0;i<length;++i)
    {
        if(*(data1+i)>0x20)
            break;
    }

    if(40 == i)
        return FALSE;
    
    for(i=0;i<length;++i)
    {
        if(*(data1+i)==*(data2+i))
        {
            continue;
        }
        else
            break;
    }
    //libc_printf("i= %d\n",i);
    for(;i<length;++i)
    {
        if((*(data1+i)>0x20) && (*(data1+i)<0x80))
        {
            //libc_printf("LEAVE FALSE %s():\n",__FUNCTION__);
            return FALSE;
        }
        else
            continue;
    }
    //libc_printf("LEAVE TRUE %s():\n",__FUNCTION__);
    return TRUE;

}

void ttx_eng_init_para_set(void)
{
    UINT32 i = 0;
    transparent_level = 0;

    for(i=0;i<TTX_TRANSPARENT_LEVEL;i++)
    {
        transparent_value[i] = 15-i*TTX_TRANSPARENT_STEP;
    }
     ttx_eng_state = TTXENG_OFF;

     return;
}

INT32 alloc_subpage_p26_naton_buffer(void)
{
        subpage_p26_nation = (UINT8 *)MALLOC(25*40*40*sizeof(UINT8));
        if(NULL == subpage_p26_nation)
        {
            return RET_FAILURE;
        }
        MEMSET(subpage_p26_nation, 0, (25*40*40)*sizeof(UINT8));
        return RET_SUCCESS;
}

UINT32 get_subpage_nation_map(UINT16 subpage, UINT16 line, UINT16 i)
{//use this function to replace GET_SUBPAGE_NATION_MAP to avoid bug, cloud
    UINT32 pos = 0;
    UINT32 ret = 0;
   

    if((subpage>=TTX_SUB_PAGE_TOTAL_NUM)||(line > TTX_MAX_ROW)||(i>TTX_MAX_COLUMN))
    {
        return 0;
    }
    pos = ((subpage)*(25*20)+line*20+(i>>1));
    ret = ((subpage_p26_nation[pos]>>((i&1)*4))&0xf);
    return ret;
}
void show_subpage_status();
void show_ttx_info();
void ttx_eng_set_seperate_ttxsubt_mode(struct vbi_device *ttx_dev, UINT8 mode)
{
    if((NULL == ttx_dev)||((TRUE != mode)&&(FALSE != mode)))
    {
        return;
    }
    seperate_ttxsubtitle_mode = mode;
}
void ttx_eng_set_ttx_history_mode(struct vbi_device *ttx_dev, BOOL mode)
{
    if((NULL == ttx_dev)||((TRUE != mode)&&(FALSE != mode)))
    {
        return;
    }
    new_history_mode = mode;
}

void check_seperate_ttxsubt_mode(UINT8 *mode)
{
    if(NULL == mode)
    {
        return;
    }
    *mode = seperate_ttxsubtitle_mode;
}

void ttx_eng_set_ttx_dsg_font(UINT32 idx)
{
    if(0 == idx)
    {
        return;
    }
    g_dsg_font_idx = idx;
}
static void init_sub_page_cb(void)
{
    UINT32 i = 0;

    for(i=0;i<80;i++)
    {
        {
            g_sub_pages[i] = (struct PBF_CB *)(g_ttx_pconfig_par.ttx_subpage_addr  + 1040*i);
            g_sub_pages[i]->valid = 0x00;
            g_sub_pages[i]->page_id = 0xffff;
            g_sub_pages[i]->sub_page_id = 0xffff;//cloud
            g_sub_pages[i]->buf_start = (UINT8*)(g_ttx_pconfig_par.ttx_subpage_addr  + 1040*i + 40);
            g_sub_pages[i]->link_red = 0xffff;
            g_sub_pages[i]->link_green = 0xffff;
            g_sub_pages[i]->link_yellow = 0xffff;
            g_sub_pages[i]->link_cyan = 0xffff;
            g_sub_pages[i]->complement_line = 0;
        }

        MEMSET(g_sub_pages[i]->buf_start, 0x20, 1000);
    }
    //LIBC_PRINTF("finish of init_SubPageCB\n");
    //MEMSET(empty_line, 0x20, 40);
    return ;
}
void clear_sub_page_cb(void)
{
    UINT32 i = 0;

    for(i=0;i<80;i++)
    {
        if(1 ==g_sub_pages[i]->valid)
        {
            MEMSET(g_sub_pages[i]->buf_start, 0x20, 1000);
            g_sub_pages[i]->valid =0;
        }
    }
    //LIBC_PRINTF("finish of clear_SubPageCB\n");
    return;
}
INT32 set_sub_page_cb(UINT16 sub_page_id, struct PBF_CB *cb, UINT8 start_line_num, UINT8 end_line_num, BOOL show_status)
{
    if(NULL == cb)
    {
        return 0;
    }
    if((sub_page_id>TTX_SUB_PAGE_MAX_NUM) || (0==cb->valid))
    {
        return 0;
    }
    if(start_line_num>end_line_num)
    {
        return 0;
    }
    g_sub_pages[sub_page_id]->page_id = cb->page_id;
    g_sub_pages[sub_page_id]->sub_page_id = cb->sub_page_id;
    g_sub_pages[sub_page_id]->link_red = cb->link_red;
    g_sub_pages[sub_page_id]->link_green = cb->link_green;    
    g_sub_pages[sub_page_id]->link_yellow = cb->link_yellow;
    g_sub_pages[sub_page_id]->link_cyan = cb->link_cyan;
    g_sub_pages[sub_page_id]->link_next = cb->link_next;
    g_sub_pages[sub_page_id]->link_index = cb->link_index;
    g_sub_pages[sub_page_id]->erase_page = cb->erase_page;
    g_sub_pages[sub_page_id]->newsflash = cb->newsflash;
    g_sub_pages[sub_page_id]->subtitle = cb->subtitle;
    g_sub_pages[sub_page_id]->suppress_header = cb->suppress_header;
    g_sub_pages[sub_page_id]->update_indicator = cb->update_indicator;
    g_sub_pages[sub_page_id]->interrupted_sequence = cb->interrupted_sequence;
    g_sub_pages[sub_page_id]->inhibit_display = cb->inhibit_display;
    g_sub_pages[sub_page_id]->magazine_serial = cb->magazine_serial;
    g_sub_pages[sub_page_id]->nation_option = cb->nation_option;
    g_sub_pages[sub_page_id]->g0_set = cb->g0_set;
    g_sub_pages[sub_page_id]->second_g0_set = cb->second_g0_set;
    g_sub_pages[sub_page_id]->pack24_exist = cb->pack24_exist;
    g_sub_pages[sub_page_id]->complement_line =cb->complement_line;
    MEMSET(g_sub_pages[sub_page_id]->buf_start+start_line_num*40, 0x20, (end_line_num-start_line_num+1)*40);
    MEMCPY(g_sub_pages[sub_page_id]->buf_start+start_line_num*40,
          cb->buf_start+start_line_num*40, (end_line_num-start_line_num+1)*40);
    //MEMSET(g_SubPages[sub_page_id]->buf_start+line_num*40, 0x20, 40);//cloud
    //MEMCPY(g_SubPages[sub_page_id]->buf_start+line_num*40,
    //cb->buf_start+line_num*40, 40);
    if( TRUE== g_ttx_pconfig_par.parse_packet26_enable)
    {
        //MEMCPY(subpage_p26_nation+(cb->sub_page_id)*(25*20)+line_num*20,
        //g_ttx_p26_nation+((cb->page_id)-100)*(25*20)+line_num*20, 20);//cloud

        MEMCPY(subpage_p26_nation+(cb->sub_page_id)*(25*20)+start_line_num*20,
        g_ttx_p26_nation+((cb->page_id)-100)*(25*20)+start_line_num*20, (end_line_num-start_line_num+1)*20);
    }
    if(0 != end_line_num)
    {
        g_sub_pages[sub_page_id]->valid = cb->valid;
    }
    if((0==cb->subtitle) && show_status)
    {
        show_subpage_status();
    }
    return 1;
}
INT32 ttx_request_sub_page_up(UINT16 page_id , UINT16 sub_page_id, struct PBF_CB **cb )
{
    if(NULL == cb)
    {
        return RET_FAILURE;
    }
    if((page_id<TTX_PAGE_MIN_NUM) || (page_id>TTX_PAGE_MAX_NUM))
    {
        return RET_FAILURE;
    }
    if(sub_page_id>=TTX_SUB_PAGE_MAX_NUM)
    {
        return RET_FAILURE;
    }
    while(sub_page_id < TTX_SUB_PAGE_MAX_NUM)
    {
        sub_page_id++;
        if((1==g_sub_pages[sub_page_id]->valid)&&(g_sub_pages[sub_page_id]->page_id == page_id))
        {
            *cb=g_sub_pages[sub_page_id];
            return RET_SUCCESS;
        }
    }
    return RET_FAILURE;
}
INT32 ttx_request_sub_page_down(UINT16 page_id , UINT16 sub_page_id, struct PBF_CB **cb )
{
    if(NULL == cb)
    {
        return RET_FAILURE;
    }
    if((page_id<TTX_PAGE_MIN_NUM) || (page_id>TTX_PAGE_MAX_NUM))
    {
        return RET_FAILURE;
    }
    if(sub_page_id>TTX_SUB_PAGE_MAX_NUM)
    {
        return RET_FAILURE;
    }
    while((sub_page_id <=TTX_SUB_PAGE_MAX_NUM) && (sub_page_id >TTX_SUB_PAGE_MIN_NUM))
    {
        sub_page_id--;
        if((1==g_sub_pages[sub_page_id]->valid)&&(g_sub_pages[sub_page_id]->page_id == page_id))
        {
            *cb=g_sub_pages[sub_page_id];
            return RET_SUCCESS;
        }
    }
    //if(TTX_SUB_PAGE_MIN_NUM==sub_page_id)   //always TRUE
    //{
        sub_page_id=cur_subpage;
        cur_subpage=0;
        show_subpage_status();
        cur_subpage=sub_page_id;
    //}
    return RET_FAILURE;
}

static INT32 show_subpage_proc(void)
{
    UINT8 j = 0;
    UINT8 i = 0;
    UINT8 max_subpage=0;
    UINT8 show_page=0;

    j = 79;
    while(j>0)
    {
        if((g_sub_pages[j]->page_id==cur_page) && (1==g_sub_pages[j]->valid))
        {
            max_subpage=j;
            j=1;
        }
        j--;
    }
    MEMSET(status_line_data,0x20,sizeof(status_line_data));
    if(1==g_sub_pages[max_subpage]->subtitle)//|| g_SubPages[max_subpage]->Newsflash==1)
        //not show subpage status when have subtitle
    {
        return RET_FAILURE;    //subtitle no subpage status!
    }
    //LIBC_PRINTF("cur_subpage=%d,cur_page=%d,max_subpage=%d\n",cur_subpage,cur_page,max_subpage);
    show_page = cur_subpage/11;
    j=show_page*11;
    if(cur_subpage>TTX_NORMAL_10)
    {
        status_line_data[0] = 0x2D;
        status_line_data[1] = 0x29;
    }
    if((max_subpage/11)>(cur_subpage/11))
    {
        status_line_data[37] = TTX_F_WHITE;
        status_line_data[38] = 0x28;
        status_line_data[39] = 0x2B;
    }
    for(i=0;(j<(show_page*11+11))&&(j<=max_subpage);j++,i++)
    {
        if(0==show_page)
        {
            if(0==cur_subpage)
            {
                status_line_data[2] = TTX_F_RED;
            }
            else
            {
                status_line_data[2] = TTX_F_WHITE;
            }
            status_line_data[3] = 0x41;
            status_line_data[4] = 0x55;
            status_line_data[5] = 0x54;
            status_line_data[6] = 0x4F;
            if(j!=0)
            {
                if(j==cur_subpage)
                {
                    status_line_data[4+i*3] = TTX_F_RED;
                }
                else if((g_sub_pages[j]->page_id==cur_page)&&(1==g_sub_pages[j]->valid))
                {
                    status_line_data[4+i*3] = TTX_F_WHITE;
                }
                else
                {
                    status_line_data[4+i*3] = TTX_F_BLACK;
                }
                status_line_data[5+i*3] = (UINT8)(j/10) + 0x30;
                status_line_data[6+i*3] = (UINT8)(j%10) + 0x30;
            }
        }
        else
        {
                if(j==cur_subpage)
                {
                    status_line_data[2+i*3] = TTX_F_RED;
                }
                else if((g_sub_pages[j]->page_id==cur_page)&&(1==g_sub_pages[j]->valid))
                {
                    status_line_data[2+i*3] = TTX_F_WHITE;
                }
                else
                {
                    status_line_data[2+i*3] = TTX_F_BLACK;
                }
                status_line_data[3+i*3] = (UINT8)(j/10) + 0x30;
                status_line_data[4+i*3] = (UINT8)(j%10) + 0x30;
        }
    }
    return RET_SUCCESS;
}

void show_subpage_status(void)
{
    UINT8 j = 0;
    UINT8 *data = NULL;
    UINT8 char_set = 0;
    UINT8 alpha_mosiac  = 1;
    UINT8 fg_color  = TTX_F_WHITE;
    UINT8 pre_bg_color = TTX_B_WHITE;
    UINT8 bg_color  = TTX_B_BLACK;
    UINT8 width  = 0;
    UINT8 height  = 0;
//    UINT8 conceal  = 0;
//    UINT8 continous  = 1;
    UINT8 hold_mosiac = 0;
    UINT8 real_alpha_mosiac = 0 ;
    UINT8 real_mosiac = 0x20;
    UINT8 last_mosiac = 0x20;
    UINT8 real_fg_color = 0;
    UINT8 real_bg_color = 0;
    UINT8 real_width = 0;
    UINT8 real_height = 0;
    UINT8 alpha=0x01;
    UINT8 mosiac=0xb0;

    real_alpha_mosiac = alpha_mosiac;
    real_fg_color = fg_color ;
    real_bg_color = bg_color;
    real_width = width;
    real_height = height;
    data = status_line_data;

    if((0xffff==cur_page ) ||(0xffff== cur_subpage ))
    {
        return;
    }
    if(RET_SUCCESS!=show_subpage_proc())
    {
        return;
    }
//showline:
    for(j=0;j<40;j++,data++)
    {
        if(*data < VBI_CHARACTER_SPACE)
        {
            if( 0==hold_mosiac )
            {
                real_mosiac = 0x20;
            }
            else
            {
                real_mosiac = last_mosiac;
            }
            switch(*data)
            {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    alpha_mosiac = 1;
                    fg_color = *data;
                    pre_bg_color = fg_color+8;
                    if(1 == real_alpha_mosiac)//alpha
                    {
                        real_mosiac = 0x20;
                    }
                    break;
                case 0x0c:
                case 0x0d:
                case 0x0e:
                case 0x0f:
                    break;
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    alpha_mosiac = 0;
                    fg_color = *data - 0x10;
                    pre_bg_color = fg_color+8;
                    break;
                case 0x18:
                    //conceal = 1;
                    break;
                case 0x19:
                    //continous = 0x01;
                    break;
                case 0x1a:
                    //continous = 0x00;
                    break;
                case 0x1b:
                    break;
                case 0x1c:
                    bg_color = TTX_B_BLACK;
                    real_bg_color =bg_color;
                    break;
                case 0x1d:
                    bg_color = pre_bg_color;
                    real_bg_color =bg_color;
                    break;
                case 0x1e:
                    hold_mosiac = 1;
                    real_mosiac = last_mosiac;
                    break;
                case 0x1f:
                    hold_mosiac = 0;
                    break;
                default :
                    break;
            }
            if(0==real_alpha_mosiac)
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 25*TTX_CHAR_H, mosiac, real_fg_color, real_bg_color,
                    real_width,real_height, real_mosiac, 0);
            }
            else
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 25*TTX_CHAR_H, mosiac, real_fg_color, real_bg_color,
                    real_width,real_height, 0x20, 0);
            }
        }
        else if (*data <= VBI_CHARACTER_MAX_VALUE)
        {
            if(1 == alpha_mosiac)
            {
                char_set = alpha;
            }
            else
            {
                char_set = mosiac;
                last_mosiac = *data;
            }
            g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 25*TTX_CHAR_H, char_set, real_fg_color, real_bg_color,
                real_width,real_height, *data, 0);
        }
        else
        {
            return;//if the data is suitable for spec, it will not go to the "else"
        }
        real_alpha_mosiac = alpha_mosiac;
        real_fg_color = fg_color;
        real_bg_color =bg_color;
        real_width =width;
        real_height =height;
    }
	last_cur_subpage = cur_subpage;
}

UINT8 key2char(TTX_KEYTYPE key)
{
    switch(key)
    {
        case TTX_KEY_NUM0 :
            return 0x30;
        case TTX_KEY_NUM1 :
            return 0x31;
        case TTX_KEY_NUM2 :
            return 0x32;
        case TTX_KEY_NUM3 :
            return 0x33;
        case TTX_KEY_NUM4 :
            return 0x34;
        case TTX_KEY_NUM5 :
            return 0x35;
        case TTX_KEY_NUM6 :
            return 0x36;
        case TTX_KEY_NUM7 :
            return 0x37;
        case TTX_KEY_NUM8 :
            return 0x38;
        case TTX_KEY_NUM9 :
            return 0x39;
        default :
            return 0x00;
    }
}
static UINT8 _language_from_iso639(UINT8 *language, UINT8 nation)
{
    INT16 ret = -1 ;

    if((TTX_NORMAL_0==MEMCMP(language,"cze",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"CZE",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"ces",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"CES",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"slk",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"SLK",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"slo",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"SLO",TTX_LANG_LEN)))
    {
        ret = 0;
    }
    //else if(MEMCMP(language,"ENG",TTX_LANG_LEN)==0)
        //||MEMCMP(language,"eng",TTX_LANG_LEN)==0)
    //    return 1;
    else if((TTX_NORMAL_0==MEMCMP(language,"est",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"EST",TTX_LANG_LEN)))
    {
        ret = 2;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"fre",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"FRE",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"fra",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"FRA",TTX_LANG_LEN)))
    {
        ret = 3;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"ger",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"GER",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"deu",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"DEU",TTX_LANG_LEN)))
    {
        ret = 4;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"ita",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"ITA",TTX_LANG_LEN)))
    {
        ret = 5;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"lit",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"LIT",TTX_LANG_LEN)))
    {
        ret = 6;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"pol",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"POL",TTX_LANG_LEN)))
    {
        ret = 7;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"por",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"POR",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"spa",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"SPA",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"esl",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"ESL",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"cat",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"CAT",TTX_LANG_LEN)))
    {
        ret = 8;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"rum",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"RUM",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"ron",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"RON",TTX_LANG_LEN)))
    {
        ret = 9;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"slv",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"scc",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"srp",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"scr",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"hrv",TTX_LANG_LEN)))
    {
        ret = 10;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"swe",TTX_LANG_LEN)) ||(TTX_NORMAL_0==MEMCMP(language,"SWE",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"sve",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"SVE",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"fin",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"FIN",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"hun",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"HUN",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"dan",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"DAN",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"nor",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"dut",TTX_LANG_LEN)))
    {
        ret = 11;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"tur",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"TUR",TTX_LANG_LEN)))
    {
        ret = 12;
    }
    else if(TTX_NORMAL_0==(MEMCMP(language,"rus",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"RUS",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"bul",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"BUL",TTX_LANG_LEN)))
    {
        ret = 0x20;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"ukr",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"UKR",TTX_LANG_LEN)))
    {
        ret = 0x30;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"grc",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"GRC",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"gre",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"GRE",TTX_LANG_LEN))
        ||(TTX_NORMAL_0==MEMCMP(language,"ell",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"ELL",TTX_LANG_LEN)))
    {
        ret = 0x40;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"ara",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"ARA",TTX_LANG_LEN)))
    {
        ret = 0x50;
    }
    else if((TTX_NORMAL_0==MEMCMP(language,"heb",TTX_LANG_LEN))||(TTX_NORMAL_0==MEMCMP(language,"HEB",TTX_LANG_LEN)))
    {
        ret = 0x60;
    }
    else
    {
        //LIBTTX_PRINTF("_language_from_iso639: current language -> %c%c%c\n",language[0],language[1],language[2]);
        //return default alpha
        switch (nation)
        {
            case 0:
                ret = 1;
                break;   //English
            case 1:
                ret = 4;
                break;  //German
            case 2:
                ret = 11;
                break;   //Swedish/Finnish/Hungarian
            case 3:
                ret = 5;
                break;    //Italian
            case 4:
                ret = 3;
                break;    //French
            case 5:
                ret = 8;
                break;    //Portuguese/Spanish
            case 6:
                ret = 0;
                break;    //Czech/Slovak
            case 7:
                if(LATIN_ARABIC==g_dsg_font_idx)
                {
                     ret = 0x50;
                 }//Arabic
                else if(LATIN_GREEK==g_dsg_font_idx)
                {
                     ret = 0x40;
                 } //Greek
                else
                {
                     ret = 9;
                }  //Rumanian
                break;
            default:
                ret = 1;
                break;    //English
        }
    }

   return ret;
}
static UINT8 language_from_iso639(UINT8 nation)
{
    UINT32 i = 0;

    if(0xFF == nation)
    {
       return 0; 
    }

    //for ttx_subtitle, need check its own language
    if ((p_cur_rd_cb != NULL) && p_cur_rd_cb->subtitle)
    {
        for (i=0; i<g_subt_page_num; i++)
        {
            if (g_subt_page[i].page == cur_page)
            {
                return _language_from_iso639(g_subt_page[i].lang, nation);
            }
        }
    }
    return _language_from_iso639(g_current_language, nation);
}
void language_select(UINT8 g0_set,UINT8 nation,UINT8 *alpha)
{
    if(NULL == alpha)
    {
        return;
    }
    //libc_printf("g0_set=%d,nation=%d,g_current_language=%s\n", g0_set,nation,g_current_language);
    switch(nation)
    {
        case 0:
            if((G0_SET_0==g0_set)||(G0_SET_2==g0_set)||(G0_SET_8==g0_set))
            {
                *alpha = 1;        //English
            }
            else if(G0_SET_1==g0_set)    //Polish
            {
                *alpha = 7;
            }
            else if(G0_SET_4==g0_set)    //[Cyrillic-1:Serbian/Croatian]
            {
                *alpha = 0x10;
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 1:
            if((G0_SET_0==g0_set)||(G0_SET_1==g0_set)||(G0_SET_2==g0_set)||(G0_SET_4==g0_set))
            {
                *alpha = 4;        //German
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 2:
			if((TTX_NORMAL_0==MEMCMP(g_current_language,"dan",TTX_LANG_LEN) ) ||
				(TTX_NORMAL_0==MEMCMP(g_current_language,"DAN",TTX_LANG_LEN)))
			{
			
				*alpha = 13;	//Danish
				break;
			}
            if((G0_SET_0==g0_set)||(G0_SET_1==g0_set)||(G0_SET_2==g0_set))
            {
                *alpha = 11;        //Swedish/Finnish/Hungarian
            }
            if(G0_SET_4==g0_set)
            {
                *alpha = 2;        //Estonian
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 3:
            if((TTX_NORMAL_0==MEMCMP(g_current_language,"lit",TTX_LANG_LEN))||
                (TTX_NORMAL_0==MEMCMP(g_current_language,"LIT",TTX_LANG_LEN)))
            {
                *alpha = 6;    //Lettish/Lithuanian
                break;
            }

            if((G0_SET_0==g0_set)||(G0_SET_1==g0_set)||(G0_SET_2==g0_set))
            {
                *alpha = 5;        //Italian
            }
            else if(G0_SET_4==g0_set)
            {
                *alpha = 6;        //Lettish/Lithuanian
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 4:
            if((G0_SET_0==g0_set)||(G0_SET_1==g0_set)||(G0_SET_2==g0_set)||(G0_SET_8==g0_set))
            {
                *alpha = 3;        //French
            }
            else if(G0_SET_4==g0_set)
            {
                *alpha = 0x20;    //[Cyrillic-2:Russian/Bulgarian]
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 5:
            if((G0_SET_0==g0_set)||(G0_SET_2==g0_set))
            {
                *alpha = 8;        //Portuguese/Spanish
            }
            else if(G0_SET_3==g0_set)
            {
                *alpha = 0x0A;    //Serbian/Croatian/Slovenian
            }
            else if(G0_SET_4==g0_set)
            {
                *alpha = 0x30;    //[Cyrillic-3:Ukrainian]
            }
            else if(0xA==g0_set)
            {
                *alpha = 0x60;    //[Hebrew]
            }
            else
            {
                *alpha = language_from_iso639(nation);
            }
            break;
        case 6:
            if((G0_SET_0==g0_set)||(G0_SET_1==g0_set)||(G0_SET_4==g0_set))
            {
                *alpha = 0;        //Czech/Slovak
            }
            else if((G0_SET_2==g0_set) || (G0_SET_6==g0_set))
            {
                *alpha = 12;        //Turkish
            }
            else
            {
                if((TTX_NORMAL_0==MEMCMP(g_current_language,"xxx",TTX_LANG_LEN))
                    &&(TRUE==g_ttx_pconfig_par.no_ttx_descriptor))
                {
                    *alpha = 0;//Czech/Slovak
                }
                else
                {
                    *alpha = language_from_iso639(nation);
                }
            }
            break;
        case 7:
            if(G0_SET_3==g0_set)
            {
                *alpha = 9;        //Rumanian
            }
            else if(G0_SET_6==g0_set)
            {
                *alpha = 0x40;    //[Greek]
            }
            else if((G0_SET_8==g0_set) || (0xA==g0_set))
            {
                *alpha = 0x50;    //[Arabic]
            }
            else
            {
                if((TTX_NORMAL_0==MEMCMP(g_current_language,"xxx",TTX_LANG_LEN))
                    &&(TRUE==g_ttx_pconfig_par.no_ttx_descriptor))
                {
                    if(LATIN_ARABIC == g_dsg_font_idx)
                    {
                        *alpha = 0x50;    //[Arabic]
                    }
                    else
                    {
                        *alpha = 0x40;//[Greek]
                    }
                }
                else
                {
                    *alpha = language_from_iso639(nation);
                }
            }
            break;
        default :
            *alpha = language_from_iso639(nation);
            break;
    }
    //libc_printf("language_select: alpha = 0x%X\n",*alpha);
    return;
}
static UINT8 g2_language_from_iso639(UINT8 nation)
{
    UINT8 lan = 0;

    if(0xFF == nation)
    {
        return 0;
    }
    lan = language_from_iso639(nation);

    if(lan < VBI_CHARSET_CYRILLIC_1)
    {
        lan = 0;
    }
    else if((VBI_CHARSET_CYRILLIC_1 == lan) || (VBI_CHARSET_CYRILLIC_2 == lan) ||(VBI_CHARSET_CYRILLIC_3 == lan))
    {
        lan = 0x10;
    }
    else if(VBI_CHARSET_GREEK == lan)
    {
        lan = 0x20;
    }
    else if(VBI_CHARSET_ARABIC == lan)
    {
        lan = 0x30;
    }
    else if(VBI_CHARSET_HEBREW == lan)
    {
        lan = 0x40;
    }
    else
    {
        lan = 0;
    }
    return lan;
}
void language_select_g2(UINT8 g0_set,UINT8 nation,UINT8 *alpha)
{
    if(NULL == alpha)
    {
        return;
    }
    *alpha = 0;
    switch(g0_set)
    {
        case 0:
        case 2:
            if(TTX_NATION_7 == nation)
            {
                *alpha = g2_language_from_iso639(nation);
            }
            else
            {
                *alpha = 0;
            }
            break;
        case 1:
            if((TTX_NATION_5 == nation) || (TTX_NATION_7 == nation))
            {
                *alpha = g2_language_from_iso639(nation);
            }
            else
            {
                *alpha = 0;
            }
            break;
        case 3:
            if((TTX_NATION_5 == nation) || (TTX_NATION_7 == nation))
            {
                *alpha = 0;
            }
            else
            {
                *alpha = g2_language_from_iso639(nation);
            }
            break;
        case 6:
            if(TTX_NATION_6 == nation)
            {
                *alpha = 0;
            }
            else if(TTX_NATION_7 == nation)
            {
                *alpha = 0x20;//Greek G2
            }
            else
            {
                *alpha = g2_language_from_iso639(nation);
            }
            break;
        case 4:
            if((TTX_NATION_1==nation)||(TTX_NATION_2==nation)||(TTX_NATION_3==nation)||(TTX_NATION_6==nation))
            {
                *alpha = 0;
            }
            else if((TTX_NATION_0 == nation)||(TTX_NATION_4 == nation)||(TTX_NATION_5 == nation))
            {
                *alpha = 0x10;//Cyrillic G2
            }
            else
            {
                *alpha = g2_language_from_iso639(nation);
            }
            break;
        case 8:
            *alpha = 0x30;//Arabic G2
            break;
        case 10:
            if((TTX_NATION_5 == nation)||(TTX_NATION_7 == nation))
            {
                *alpha = 0x30;
            }
            break;
        default:
            *alpha = g2_language_from_iso639(nation);
            break;
    }
    return;
}
//check if has displayable character
//for "0x0b x x x 0x0a", if has no displayable character
//between 0x0b and 0x0a, do not show anything
BOOL check_displayable_char(UINT8 *data, UINT8 len)
{
    UINT32 i = 0;

    if(len>40)
    {
        return FALSE;
    }

    for (i=0; i<len; i++)
    {
        if ((data[i] > 0x20) && (data[i] < 0x80))
        {
            return TRUE;
        }
    }
    return FALSE;
}
UINT8 get_mosaic_mode(void)
{
    return mosaic_mode;
}
//extern UINT8 g_packet_exist[8][25];
//#define PRINTF_PAGE_PACKET
//extern BOOL page_erase;

static void parse_line_num_0(struct PBF_CB *p_cb)
{
    UINT8 i = 0;
    if(NULL == p_cb)
    {
        return;
    }
    if((TTXENG_TEXT == ttx_eng_state) && (1!=p_cb->interrupted_sequence))
    {
        if((NULL==p_cur_rd_cb)||(1!=p_cur_rd_cb->newsflash))
        {
            for(i=8;i<32;i++)
            {
                if(*(p_cb->buf_start+i)!=0x20)
                {
                    break;
                }
            }
            if(i!=TTX_NORMAL_32)//filter blank packet 0
            {
               parse_line(p_cb,0,1,0);
            }
        }
    }
}

static void ttx_set_link_line_data1(void)
{
    MEMSET(link_line_data,0x20,sizeof(link_line_data));
    link_line_data[0] = 0x01;
    link_line_data[1] = 0x1d;
    link_line_data[2] = 0x07;
    link_line_data[4] = (UINT8)(page_queue[0]/100) + 0x30;
    link_line_data[5] = (UINT8)((page_queue[0]%100)/10) + 0x30;
    link_line_data[6] = (UINT8)((page_queue[0]%100)%10) + 0x30;
    link_line_data[9] = 0x02;
    link_line_data[10] = 0x1d;
    link_line_data[11] = 0x04;
    link_line_data[13]= (UINT8)(page_queue[1]/100) + 0x30;
    link_line_data[14]= (UINT8)((page_queue[1]%100)/10) + 0x30;
    link_line_data[15] = (UINT8)((page_queue[1]%100)%10) + 0x30;
    link_line_data[19] = 0x03;
    link_line_data[20] = 0x1d;
    link_line_data[21] = 0x04;
    link_line_data[23] = (UINT8)(page_queue[2]/100) + 0x30;
    link_line_data[24] = (UINT8)((page_queue[2]%100)/10) + 0x30;
    link_line_data[25] = (UINT8)((page_queue[2]%100)%10) + 0x30;
    link_line_data[29] = 0x06;
    link_line_data[30] = 0x1d;
    link_line_data[31] = 0x04;
    if((0xffff==page_queue[3]) && (new_history_mode==TRUE))
    {
        link_line_data[32] = (UINT8)'v';
        link_line_data[33] = (UINT8)'o';
        link_line_data[34] = (UINT8)'i';
        link_line_data[35] = (UINT8)'d';
    }
    else
    {
        link_line_data[33] = (UINT8)(page_queue[3]/100) + 0x30;
        link_line_data[34] = (UINT8)((page_queue[3]%100)/10) + 0x30;
        link_line_data[35] = (UINT8)((page_queue[3]%100)%10) + 0x30;
    }
}

static void ttx_set_link_line_data2(void)
{
    MEMSET(link_line_data,0x20,sizeof(link_line_data));
    link_line_data[0] = 0x01;
    link_line_data[1] = 0x1d;
    link_line_data[2] = 0x07;
    link_line_data[6] = (UINT8)(page_queue[0]/100) + 0x30;
    link_line_data[7] = (UINT8)((page_queue[0]%100)/10) + 0x30;
    link_line_data[8] = (UINT8)((page_queue[0]%100)%10) + 0x30;
    link_line_data[14] = 0x02;
    link_line_data[15] = 0x1d;
    link_line_data[16] = 0x04;
    link_line_data[20]= (UINT8)(page_queue[1]/100) + 0x30;
    link_line_data[21]= (UINT8)((page_queue[1]%100)/10) + 0x30;
    link_line_data[22] = (UINT8)((page_queue[1]%100)%10) + 0x30;
    link_line_data[28] = 0x03;
    link_line_data[29] = 0x1d;
    link_line_data[30] = 0x04;
    link_line_data[34] = (UINT8)(page_queue[2]/100) + 0x30;
    link_line_data[35] = (UINT8)((page_queue[2]%100)/10) + 0x30;
    link_line_data[36] = (UINT8)((page_queue[2]%100)%10) + 0x30;
}

void parse_line_ext(struct PBF_CB *cb)
{
    UINT8 j = 0;
    UINT8 *data = NULL ;
    UINT8 char_set = 0;
    UINT8 alpha_mosiac  = 0;
    UINT8 fg_color  = 0;
    UINT8 pre_bg_color = 0;
    UINT8 bg_color  = 0;
    UINT8 width  = 0;
    UINT8 height  = 0;
//    UINT8 conceal  = 0;
//    UINT8 continous  = 0;
    UINT8 hold_mosiac = 0;
    UINT8 real_alpha_mosiac  = 0;
    UINT8 real_mosiac = 0;
    UINT8 last_mosiac = 0;
    UINT8 real_fg_color = 0;
    UINT8 real_bg_color = 0;
    UINT8 real_width = 0;
    UINT8 real_height = 0;
    UINT8 alpha=0x01;
    UINT8 mosiac=0xb0;

    if((NULL == cb) || (1 == cb->inhibit_display))
    {
        return;
    }
    alpha_mosiac = 1;
    real_alpha_mosiac = alpha_mosiac;
    fg_color = TTX_F_WHITE;
    real_fg_color = fg_color;
    pre_bg_color = TTX_B_WHITE;
    bg_color = TTX_B_BLACK;
    real_bg_color = bg_color;
    width = 0;
    real_width = width ;
    height = 0;
    real_height = height;
//    conceal = 0;
//    continous = 1;
    hold_mosiac = 0;
    real_mosiac =0x20;
    last_mosiac =0x20;
    if((cb->subtitle)||(cb->newsflash))
    {
        return;
    }
    for(j=0;j<80;j++)
    {
        if((1==g_sub_pages[j]->valid)&&(1==g_sub_pages[j]->pack24_exist))
        {
            return;
        }
    }

    data = link_line_data;
    if( TTX_QUEUE_TAG_4==queue_tag)
    {
        ttx_set_link_line_data1();
    }
    else if(TTX_QUEUE_TAG_3 == queue_tag)
    {
        ttx_set_link_line_data2();
    }
    else if(TTX_QUEUE_TAG_2 == queue_tag)
    {
        MEMSET(link_line_data,0x20,40);
        link_line_data[0] = 0x01;
        link_line_data[1] = 0x1d;
        link_line_data[2] = 0x07;
        link_line_data[9] = (UINT8)(page_queue[0]/100) + 0x30;
        link_line_data[10] = (UINT8)((page_queue[0]%100)/10) + 0x30;
        link_line_data[11] = (UINT8)((page_queue[0]%100)%10) + 0x30;
        link_line_data[21] = 0x02;
        link_line_data[22] = 0x1d;
        link_line_data[23] = 0x04;
        link_line_data[29]= (UINT8)(page_queue[1]/100) + 0x30;
        link_line_data[30]= (UINT8)((page_queue[1]%100)/10) + 0x30;
        link_line_data[31] = (UINT8)((page_queue[1]%100)%10) + 0x30;
    }
    else if(TTX_QUEUE_TAG_1 == queue_tag)
    {
        MEMSET(link_line_data,0x20,40);
        link_line_data[0] = 0x01;
        link_line_data[1] = 0x1d;
        link_line_data[2] = 0x07;
        link_line_data[19] = (UINT8)(page_queue[0]/100) + 0x30;
        link_line_data[20] = (UINT8)((page_queue[0]%100)/10) + 0x30;
        link_line_data[21] = (UINT8)((page_queue[0]%100)%10) + 0x30;
    }

    for(j=0;j<40;j++,data++)
    {
        if (*data < VBI_CHARACTER_SPACE)
        {
            if(0 == hold_mosiac)
            {
                real_mosiac = 0x20;
            }
            else
            {
                real_mosiac = last_mosiac;
            }
            switch(*data)
            {
                case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:
                    alpha_mosiac = 1;
                    fg_color = *data;
                    pre_bg_color = fg_color+8;
                    if(1 == real_alpha_mosiac)//alpha
                    {
                        real_mosiac = 0x20;
                    }
                    break;
                case 0x0c:
                    break;
                case 0x0d:
                    break;
                case 0x0e:
                    break;
                case 0x0f:
                    break;
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    alpha_mosiac = 0;
                    fg_color = *data - 0x10;
                    pre_bg_color = fg_color+8;
                    break;
                case 0x18:
                    //conceal = 1;
                    break;
                case 0x19:
                    //continous = 0x01;
                    break;
                case 0x1a:
                    //continous = 0x00;
                    break;
                case 0x1b:
                    break;
                case 0x1c:
                    bg_color = TTX_B_BLACK;
                    real_bg_color =bg_color;
                    break;
                case 0x1d:
                    bg_color = pre_bg_color;
                    real_bg_color =bg_color;
                    break;
                case 0x1e:
                    hold_mosiac = 1;
                    real_mosiac = last_mosiac;
                    break;
                case 0x1f:
                    hold_mosiac = 0;
                    break;
                default :
                    break;
            }
            if(0 == real_alpha_mosiac)
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 24*TTX_CHAR_H, mosiac, real_fg_color,
                    real_bg_color, real_width,real_height, real_mosiac,0);
            }
            else
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 24*TTX_CHAR_H, mosiac, real_fg_color,
                    real_bg_color, real_width,real_height, 0x20,0);
            }
        }
        else if (*data <= VBI_CHARACTER_MAX_VALUE)
        {
            if(1 == alpha_mosiac)
            {
                char_set = alpha;
            }
            else
            {
                char_set = mosiac;
                last_mosiac = *data;
            }
            g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 24*TTX_CHAR_H, char_set, real_fg_color,
                real_bg_color, real_width,real_height, *data,0);
        }
        else
        {
            return;
        }
        real_alpha_mosiac = alpha_mosiac;
        real_fg_color = fg_color;
        real_bg_color =bg_color;
        real_width =width;
        real_height =height;
    }
}

void show_ttx_info(void)
{
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 *data = NULL ;
    UINT8 char_set = 0;
    UINT8 alpha_mosiac  = 0;
    UINT8 fg_color  = 0;
    UINT8 pre_bg_color = 0;
    UINT8 bg_color  = 0;
    UINT8 width  = 0;
    UINT8 height  = 0;
//    UINT8 conceal  = 0;
//    UINT8 continous  = 0;
    UINT8 hold_mosiac = 0;

    UINT8 real_alpha_mosiac = 0 ;
    UINT8 real_mosiac = 0;
    UINT8 last_mosiac = 0;
    UINT8 real_fg_color = 0;
    UINT8 real_bg_color = 0;
    UINT8 real_width = 0;
    UINT8 real_height = 0;
    UINT8 ttx_info[40] = {0};
    UINT8 alpha=0x01;
    UINT8 mosiac=0xb0;

    alpha_mosiac = 1;
    real_alpha_mosiac = alpha_mosiac ;
    fg_color = TTX_F_WHITE;
    real_fg_color = fg_color ;
    pre_bg_color = TTX_B_WHITE;
    bg_color = TTX_TRANSPARENT;//TTX_B_BLACK;
    real_bg_color = bg_color;
    width = 0;
    real_width = width ;
    height = 0;
    real_height = height;
    //conceal = 0;
    //continous = 1;
    hold_mosiac = 0;
    real_mosiac =0x20;
    last_mosiac =0x20;
    for(i=0;i<15;i++)
    {
        ttx_info[i]=0x20;
    }

    for(i=24;i<40;i++)
    {
         ttx_info[i]=0x20;
    }
    ttx_info[15]=TTX_F_RED;
    ttx_info[16]='T';
    ttx_info[17]='e';
    ttx_info[18]='l';
    ttx_info[19]='e';
    ttx_info[20]='t';
    ttx_info[21]='e';
    ttx_info[22]='x';
    ttx_info[23]='t';
    data=ttx_info;
    for(j=0;j<40;j++,data++)
    {
        if (*data < VBI_CHARACTER_SPACE)
        {
            if(0 == hold_mosiac)
            {
                real_mosiac = 0x20;
            }
            else
            {
                real_mosiac = last_mosiac;
            }
            switch(*data)
            {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    alpha_mosiac = 1;
                    fg_color = *data;
                    pre_bg_color = fg_color+8;
                    if(1 == real_alpha_mosiac)//alpha
                    {
                        real_mosiac = 0x20;
                    }
                    break;
                case 0x0c:
                    break;
                case 0x0d:
                    break;
                case 0x0e:
                    break;
                case 0x0f:
                    break;
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    alpha_mosiac = 0;
                    fg_color = *data - 0x10;
                    pre_bg_color = fg_color+8;
                    break;
                case 0x18:
                    //conceal = 1;
                    break;
                case 0x19:
                    //continous = 0x01;
                    break;
                case 0x1a:
                    //continous = 0x00;
                    break;
                case 0x1b:
                    break;
                case 0x1c:
                    bg_color = TTX_B_BLACK;
                    real_bg_color =bg_color;
                    break;
                case 0x1d:
                    bg_color = pre_bg_color;
                    real_bg_color =bg_color;
                    break;
                case 0x1e:
                    hold_mosiac = 1;
                    real_mosiac = last_mosiac;
                    break;
                case 0x1f:
                    hold_mosiac = 0;
                    break;
                default :
                    break;
            }
            if(0 == real_alpha_mosiac)
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 26*TTX_CHAR_H, mosiac, real_fg_color,
                    real_bg_color,real_width,real_height, real_mosiac,0);
            }
            else
            {
                g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 26*TTX_CHAR_H, mosiac, real_fg_color,
                    real_bg_color, real_width,real_height, 0x20,0);
            }
        }
        else if (*data <= VBI_CHARACTER_MAX_VALUE)
        {
            if(1 == alpha_mosiac)
            {
                char_set = alpha;
            }
            else
            {
                char_set = mosiac;
                last_mosiac = *data;
            }
            g_ttx_pconfig_par.ttx_drawchar(j*TTX_CHAR_W, 26*TTX_CHAR_H, char_set, real_fg_color,
                real_bg_color, real_width,real_height, *data,0);
        }
        else
        {
            return;//Commonly will not go to this "else"
        }
            
        real_alpha_mosiac = alpha_mosiac;
        real_fg_color = fg_color;
        real_bg_color =bg_color;
        real_width =width;
        real_height =height;
    }
}
void set_packet24_new(struct vbi_device *ttx_dev, UINT16 last_page_num)
{
    INT32 ret = RET_FAILURE;
    struct PBF_CB *temp_cb=NULL;

    if((NULL == ttx_dev)||((last_page_num<TTX_PAGE_MIN_NUM)||\
        ((last_page_num>TTX_PAGE_MAX_NUM)&&(0xFFFF!=last_page_num))))
    {
        return;
    }
    ret = ttx_request_page_up(ttx_dev,cur_page, &temp_cb);
    if (RET_SUCCESS == ret)
    {
        page_queue[0] = temp_cb->page_id;
    }
    else
    {
        page_queue[0] = cur_page;
    }
    ret = ttx_request_page_up(ttx_dev,page_queue[0], &temp_cb);
    if (RET_SUCCESS == ret)
    {
        page_queue[1] = temp_cb->page_id;
    }
    else
    {
        page_queue[1] = page_queue[0];
    }
    ret = ttx_request_page_up(ttx_dev,page_queue[1], &temp_cb);
    if (RET_SUCCESS == ret)
    {
        page_queue[2] = temp_cb->page_id;
    }
    else
    {
        page_queue[2] = page_queue[1];
    }
    queue_tag = 4;
    page_queue[3] = last_page_num;
}


#if 1
static void ttx_subt_page_timeout_handler(void)
{
    if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
        ttx_set_screen_color(TTX_TRANSPARENT);
    if(ttx_subt_alarm_id != OSAL_INVALID_ID)
    {
        osal_timer_delete(ttx_subt_alarm_id);
        ttx_subt_alarm_id = OSAL_INVALID_ID;
    }
}


static void ttx_subt_timer_start(void)
{
  OSAL_T_CTIM ttx_subt_timer;

  if(ttx_subt_alarm_id != OSAL_INVALID_ID)
  {
    osal_timer_delete(ttx_subt_alarm_id);
    ttx_subt_alarm_id = OSAL_INVALID_ID;
  }
  ttx_subt_timer.type = TIMER_ALARM;
  ttx_subt_timer.time = 5000;//2000;
  ttx_subt_timer.callback = (TMR_PROC)ttx_subt_page_timeout_handler;
  ttx_subt_alarm_id = osal_timer_create(&ttx_subt_timer);
}
#endif

INT32 ttx_eng_open(struct vbi_device *ttx_dev, UINT8 subtitle,UINT16 page_num)
{//to create and start task or wake up a task
    UINT8 i = 0;
    UINT8 ch = 0;
    UINT16 left = 0;
    UINT16 top = 0;
//    INT32 ret_func = -1;
    INT32 ret = -1;
    if((NULL == ttx_dev)||((page_num<TTX_PAGE_MIN_NUM)||(page_num>TTX_PAGE_MAX_NUM)))
    {
        return !RET_SUCCESS;
    }

    //LIBC_PRINTF("TTXEng OPEN : subtitle = %d,page_num = %d\n",
    //subtitle,page_num);
    ENTER_TTX_ENG();
    screen_transparent_flag=0;
    screen_transparent_flag2=0;
    if(g_ttx_pconfig_par.user_fast_text==TRUE)
    {
        page_queue[0] = 0xffff;
        page_queue[1] = 0xffff;
        page_queue[2] = 0xffff;
        page_queue[3] = 0xffff;
        queue_tag = 0;
        MEMSET(link_line_data,0x20,40);
    }
    req_page = page_num;
    wait_page = req_page;
    p_cur_rd_cb = NULL;
    cur_page = 0xffff;
    if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
    {
        b_upd_page = FALSE;//
        cur_subpage = 0xffff;
        wait_sub_page = 0xffff;//cloud
        init_sub_page_cb();//cloud
    }
    if(subtitle != 1)
    {
        if(TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_set_back_alpha(transparent_value[transparent_level]);
            ttx_set_screen_color(TTX_B_BLACK);
        }
        if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_set_screen_color(TTX_B_BLACK);
        }
        top = 0;
        //left = 0;
        //ch = 'P';
        //ENG_PRINTF("=> %c \n", ch);
        //TTX_DrawChar(left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch);
        left = TTX_CHAR_W;
        //ch = '1';
        ch = (UINT8)(req_page/100) + 0x30;
        //ENG_PRINTF("=> %c \n", ch);
        g_ttx_pconfig_par.ttx_drawchar
                (left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        left = TTX_CHAR_W*2;
        //ch = '0';
        ch = (UINT8)((req_page%100)/10) + 0x30;
        //ENG_PRINTF("=> %c \n", ch);
        g_ttx_pconfig_par.ttx_drawchar
                (left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        left = TTX_CHAR_W*3;
        //ch = '0';
        ch = (UINT8)((req_page%100)%10) + 0x30;
        //ENG_PRINTF("=> %c \n", ch);
        g_ttx_pconfig_par.ttx_drawchar
                (left,top,0x01,TTX_F_WHITE,TTX_B_BLACK,0,0,ch,0);
        ret = ttx_request_page(ttx_dev,req_page , &p_cur_rd_cb );
        if (RET_SUCCESS == ret)
        {
            wait_page =0xffff;
            cur_page = p_cur_rd_cb->page_id;

            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                cur_subpage = p_cur_rd_cb->sub_page_id;
            }
            parse_line(p_cur_rd_cb,0,0,1);
            for(i=1;i<25;i++)
            {
                parse_line(p_cur_rd_cb,i,0,0);
            }
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                //for(i=0;i<25;i++)
                set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb, 0, 24, FALSE);
                show_subpage_status();
            }
        }
         }
    else
    {
        if(TTX_COLOR_NUMBER_16==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_set_back_alpha(transparent_value[TTX_TRANSPARENT_LEVEL-1]);
            ttx_set_screen_color(TTX_B_BLACK);
        }
        if(TTX_COLOR_NUMBER_256==g_ttx_pconfig_par.ttx_color_number)
        {
            ttx_set_screen_color(TTX_TRANSPARENT);
        }
        ret = ttx_request_page(ttx_dev,req_page , &p_cur_rd_cb );
        if ((RET_SUCCESS == ret)&&(p_cur_rd_cb->page_id == req_page))
        {
            wait_page =0xffff;
            cur_page = p_cur_rd_cb->page_id;
            if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
            {
                cur_subpage = p_cur_rd_cb->sub_page_id;
            }
            for(i=1;i<25;i++)
            {
                parse_line(p_cur_rd_cb,i,0,0);
            }
        }
    }
    ttx_eng_state = subtitle;

    if((g_ttx_pconfig_par.user_fast_text==TRUE) && (new_history_mode==TRUE))
    {
        set_packet24_new(ttx_dev, 0xffff);
        if(p_cur_rd_cb && (0 == p_cur_rd_cb->pack24_exist))
        {
            parse_line_ext(p_cur_rd_cb);
        }
    }
    if((TRUE == get_ttxmenu_status()) && (TRUE == get_ttxinfo_status()))
    {
        show_ttx_info();
    }
    //libc_printf("TTXEng_Open finish\n");
    LEAVE_TTX_ENG();
    return RET_SUCCESS;
}
void  ttx_eng_close(struct vbi_device *ttx_dev)
{
    if(NULL == ttx_dev)
    {
        return;
    }
    //libc_printf("--TTXEng_Close--\n");
    ENTER_TTX_ENG();
    ttx_eng_state = TTXENG_OFF;
	#ifdef TTX_FLASH_FUNC
		ttx_delete_whole();
	#endif
    LEAVE_TTX_ENG();
}



void ttx_eng_update_page(struct vbi_device *ttx_dev, UINT16 page_id, UINT8 line_id)
{
    struct PBF_CB *p_cb = NULL;
    UINT8 i = 0;
    UINT16 mag = 0;
    UINT8 *data = NULL;
    INT32 ret = -1;
	INT32 j=0;
//    INT32 ret_func = -1;
    UINT8 double_height = 0;

    if(NULL == ttx_dev)
    {
        return;
    }
    //ttx_dev = (struct vbi_device *)dev_get_by_name("VBI_M3327_0");
    if(TTXENG_OFF == ttx_eng_state)
    {
        return;
    }
    //osal_task_dispatch_off();
    if(TTX_TOTAL_MAGZINE==page_id/100)
    {
        mag=0;
    }
    else
    {
        mag=page_id/100;
    }
    if (((page_id == cur_page)&&(0xffff ==wait_page)) && (NULL != p_cur_rd_cb))
    {//diplaying & not waiting
        if(g_ttx_pconfig_par.ttx_sub_page==TRUE)
        {
            if(0 == line_id)
            {
                parse_line(p_cur_rd_cb,0,1,0);
                set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb, 0, 0, FALSE);
                end_line_of_the_page = 0;
                for(i=1;i<25;i++)
                {
                    if(1 == g_packet_exist[mag][i])
                    {
                        end_line_of_the_page = i;
                    }
                }
				if(1== p_cur_rd_cb->subtitle)
                {//Colin:some stream may insert NULL page to refresh the subtitle
                    for(j=40;j<1000;++j)
                    {
                        if(*(p_cur_rd_cb->buf_start+j)>0x20)
                            break;
                    }
                    //libc_printf("line %d,j = %d\n",__LINE__,j);
                    if(j==1000)
                        ttx_set_screen_color(TTX_TRANSPARENT); 
                }
            }
            //LIBC_PRINTF("cur_page=%d,cur_subpage=%d,bUpdPage=%d,WaitSubPage=%d,
            //pCurRdCB->sub_page_id=%d\n",cur_page,cur_subpage,bUpdPage,
            //WaitSubPage,pCurRdCB->sub_page_id);


            for( i=0;i<=24;i++)
            {
            	data = p_cur_rd_cb->buf_start + (40*i);
            	if(check_displayable_char(data,40))
            		break;
            }
            if((i>24)&&(1!= p_cur_rd_cb->subtitle))
            {
            	
            	return;
            }

            if((0!=line_id)&& (get_b_save_subpage()))
            {
                set_sub_page_cb(p_cur_rd_cb->sub_page_id, p_cur_rd_cb,line_id, line_id, FALSE);
            }


            if(((FALSE==b_upd_page) && ( 0xffff== wait_sub_page)) ||(wait_sub_page == p_cur_rd_cb->sub_page_id) ||
                (cur_subpage == p_cur_rd_cb->sub_page_id))
            {
                cur_subpage = p_cur_rd_cb->sub_page_id;
			#ifdef TTX_FLASH_FUNC
               // the same curpage needn't delete the ttx_flash_list link table;
               // the ttx_flash_list will be delete when the subpage changed
               if(same_cur_subpage == 0xffff)//pCurRdCB->sub_page_id)
               {
                   
                   same_cur_subpage = p_cur_rd_cb->sub_page_id;
               }
               else
               {
                   if(same_cur_subpage != p_cur_rd_cb->sub_page_id)
                   {                   	   
                       ttx_delete_whole();
                       same_cur_subpage = p_cur_rd_cb->sub_page_id;
                   }
               }
            #endif
                if((line_id >0) && (line_id<=TTX_MAX_ROW) )
                {
                    if((1 == p_cur_rd_cb->subtitle)&&(cur_page==p_cur_rd_cb->page_id))
                    {
                        UINT32 temp_tick = 0;

                        data = p_cur_rd_cb->buf_start + (40*line_id);
                        b_useful_data= check_displayable_char(data,40);
                        if(TRUE == b_useful_data)
                        {
                            ttx_subt_timer_start();
                            temp_tick = osal_get_tick();
                            if(temp_tick - last_temp_tick>1500)
                            {
                	  		  	 
                               //libc_printf("clear screen3\n");
                               ttx_set_screen_color(TTX_TRANSPARENT); 
                            }
                            last_temp_tick = temp_tick; 
                        }
                       
                        if((TRUE==b_useful_data)&&((FALSE==last_page_useful_line[line_id])||((TRUE==last_page_useful_line[line_id])&&(MEMCMP(data,&last_page_data[line_id*40],40)!=0))))
                        {
                            for(i=0;i<40;++i)
                            {
                                if(*(data+i)==0x0d)
                                {
                                    double_height = 2;
									break;//@Ziv fix #39736 show subt overlap
                                }
                                else
                                    continue;
                            }
                            if(40 == i)
                                double_height = 1;
                            b_ttx_subt_deal = TRUE;
                            if(FALSE == data_part_same_flag(&last_page_data[line_id*40],data,40))
                                ttx_clear_screen(osd_ttx_xoffset,osd_ttx_yoffset+line_id*TTX_CHAR_H,osd_ttx_width,double_height*TTX_CHAR_H,TTX_TRANSPARENT);
                            parse_line(p_cur_rd_cb,line_id,0,0);
                            b_ttx_subt_deal = FALSE;
                           last_page_useful_line[line_id] = TRUE;
                           MEMCPY(&last_page_data[line_id*40],data,40);
                           b_useful_data = FALSE;
                        }
                        else
                        {
							if((MEMCMP(data,&last_page_data[line_id*40],40)!=0))
                               MEMSET(&last_page_data[line_id*40],00,40);
                            last_page_useful_line[line_id] = FALSE;
                            b_useful_data = FALSE;
                            return;
                        }
    		    	 }
                     else   //not ttx subtitle
    		    	     parse_line(p_cur_rd_cb,line_id,0,0);
    		    }
            }
            //the stream will send a page with no useful data after
            //sending a page with useful data.
            //the screen must be cleaned when receiving
            //the page with no useful data.
            if(((INT8)line_id>=0) && (line_id<=TTX_MAX_ROW))
            {
                if(line_id<last_line_id)
                {
                    show_subpage_status();
                }
                last_line_id = line_id;
            }
		
            if(line_id ==end_line_of_the_page)
            {
                last_cur_page = cur_page;
            }
        }
        else
        {
            if(0 == line_id)
            {
                parse_line(p_cur_rd_cb,0,1,0);
            }
            else
            {
                parse_line(p_cur_rd_cb,line_id,0,0);
            }
        }
    }
    else if (page_id == wait_page)
    {//Waiting meet
        ret = ttx_request_page(ttx_dev,wait_page , &p_cur_rd_cb );

        if (RET_SUCCESS == ret)
        {
            wait_page =0xffff;
            if(NULL == p_cur_rd_cb)
            {
                return;
            }
            cur_page = p_cur_rd_cb->page_id;
            parse_line(p_cur_rd_cb,0,1,0);
            if(((FALSE == b_upd_page)&&(TRUE == g_ttx_pconfig_par.ttx_sub_page))
                ||(FALSE == g_ttx_pconfig_par.ttx_sub_page))
            {
                for(i=1;i<25;i++)
                {
                    parse_line(p_cur_rd_cb,i,0,0);
                }
                if(TRUE == g_ttx_pconfig_par.ttx_sub_page)
                {
                    show_subpage_status();
                }
                if((p_cur_rd_cb)&&(0==p_cur_rd_cb->pack24_exist)&&(TRUE == g_ttx_pconfig_par.user_fast_text))
                {
                   parse_line_ext(p_cur_rd_cb);
                }
            }
        }
    }
    else if(wait_page != 0xffff)
    {//waiting not meet
        if(0 == line_id)
        {
            ret = ttx_request_page(ttx_dev,page_id , &p_cb);
            if(NULL == p_cb)
            {
                return;
            }
            if(RET_SUCCESS == ret)
            {
                parse_line_num_0(p_cb);
            }
        }
    }
    return;
}
#ifdef SEE_CPU
extern struct vbi_device * g_vbi_device;
void ttx_eng_update_page_cb(UINT16 param1,UINT8 param2)
{
    if(((param1<TTX_PAGE_MIN_NUM)||(param1>TTX_PAGE_MAX_NUM))||(param2>TTX_MAX_ROW))
    {
        return;
    }
    ENTER_TTX_ENG();
    ttx_eng_update_page(g_vbi_device, param1, param2);
    LEAVE_TTX_ENG();
}
#endif
void  ttx_eng_show_onoff(struct vbi_device *ttx_dev, BOOL b_on)
{
    if((NULL == ttx_dev)||((TRUE != b_on)&&(FALSE != b_on)))
    {
        return;
    }
    ENTER_TTX_ENG();
    if(b_on == TRUE)
    {
        //MEMSET(last_draw_data, 0xFF, 24*40);
		MEMSET(last_page_data, 0x00, 1000);
        MEMSET(last_page_useful_line,0x00,25);
        ttx_osd_enter();
    }
    else
    {
        ttx_osd_leave();
    }
    LEAVE_TTX_ENG();
}

#if 0
static UINT8 ttx_eng_get_state2(void)      // internal use
{
    return ttx_eng_state;
}
#endif

UINT8 ttx_eng_get_state(struct vbi_device *ttx_dev)
{
    if(NULL == ttx_dev)
    {
        return 0xFF;//Invalid value
    }
    return ttx_eng_state;
}

void ttx_eng_update_init_page(struct vbi_device *ttx_dev, UINT8 num, UINT32 page_addr)
{
    if((NULL == ttx_dev)||(num > TTX_SUBT_LANG_NUM)||( 0 == page_addr))
    {
        return;
    }
    MEMCPY((void *)(g_init_page), (void *)(page_addr), sizeof(struct t_ttx_lang)*num);
}
void ttx_eng_update_subt_page(struct vbi_device *ttx_dev, UINT8 num, UINT32 page_addr)
{
    if((NULL == ttx_dev)||(num > TTX_SUBT_LANG_NUM)||( 0 == page_addr))
    {
        return;
    }
    MEMCPY((void *)(g_subt_page), (void *)(page_addr), sizeof(struct t_ttx_lang)*num);
    g_subt_page_num = num;
}

void ttx_eng_set_cur_language(UINT32 language)
{
    if(0 == language)
    {
        return;
    }
    MEMCPY((void *)g_current_language, (void *)(language), 3);
}
#endif
