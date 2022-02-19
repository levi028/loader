/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_cc_dec.c

   *    Description: PSIP parsing engine to monitor and parse tables in streams
   *  History:
   *      Date         Author          Version   Comment
   *      ====         ======         =======   =======
   *  1.  ----     Steve Lee         0.1.000       Initial
   *  2.  2007.        HB Chen           0.2.000       modify, rename
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/
/*******************
* INCLUDE FILES    *
********************/
#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <api/libclosecaption/atsc_cc_init.h>
#include <api/libclosecaption/closecaption_osd.h>
#include <api/libclosecaption/atsc_dtvcc_dec.h>
#include "lib_closecaption_internal.h"

#if 1//(SYS_CHIP_MODULE!=ALI_M3327C)&&((SYS_PROJECT_FE == PROJECT_FE_ATSC)||(SYS_PROJECT_FE == PROJECT_FE_DVBS))

/*******************
* LOCAL MACROS     *
********************/
#if 1
#define CC_PRINTF(...) //soc_printf
#else
#define CC_PRINTF libc_printf   //vicky20110124
#endif

#if 1
#define CCDEC1_PRINTF(...)//    soc_printf
#else
#define CCDEC1_PRINTF   libc_printf     //vicky20110124
#endif

#define EAS_PRINTF(...)

#define     INFO_TEXT       0X00
#define     CAPTION_ROLL_UP 0X01
#define     CAPTION_POP_ON  0X02
#define     CAPTION_PAINT_ON    0X03

#define RESUME_CAPTION_LOADING      0X0
#define BACKSPACE                   0X1
#define DELETE_TO_END_OF_ROW        0X4
#define ROLLUP_CAPTION_2ROW         0X5
#define ROLLUP_CAPTION_3ROW         0X6
#define ROLLUP_CAPTION_4ROW         0X7
#define FLASH_ON                    0X8
#define RESUME_DIRECT_CAPTIONING    0X9
#define TEXT_RESTART                0XA
#define RESUME_TEXT_DISPLAY         0XB
#define ERASE_DISPLAYED_MEMORY      0XC
#define CARRIAGE_RETURN             0XD
#define ERASE_NONDISPLAYED_MEMORY   0XE
#define END_OF_CAPTION              0XF

#define TAB_OFFSET_1                0X1
#define TAB_OFFSET_2                0X2
#define TAB_OFFSET_3                0X3

//rating_system
#define MPAA 0x0
#define US_TV 0x1
#define MPA3 0x2
#define CAN_ENG 0x3
#define CAN_FRE 0x4
//MPAA rating
#define MPAA_NA 0x0
#define MPAA_G 0x1
#define MPAA_PG 0x2
#define MPAA_PG13 0x3
#define MPAA_R 0x4
#define MPAA_NC17 0x5
#define MPAA_X 0x6
#define MPAA_NR 0x7
//US_TV rating
#define TV_NONE 0x0
#define TV_Y 0x1
#define TV_Y7 0x2
#define TV_G 0x3
#define TV_PG 0x4
#define TV_14 0x5
#define TV_MA 0x6

#define LINE21_DEPTH    1024
#define CC_RETURN_CONTINUE 1
#define CC_RETURN_SUCCESS 0

/*******************
* LOCAL TYPEDEFS   *
********************/
struct flash_data_cell
{
    UINT8 start_x;
    UINT8 y;
    UINT8 end_x;
    UINT8 fg_color;
    UINT8 bg_color;
    UINT8 underline;
    UINT8 italics;
    UINT8 string[32];
};

/*******************
* STATIC DATA      *
********************/

#ifdef CC_MONITOR_CC
unsigned char bsn_cc[33]; // total + 2 * 16
unsigned char bcontrol=FALSE;
static unsigned int w_cmdcnt=0;
static unsigned int w_prevcnt=0;
static UINT8 bfirst=TRUE;
static UINT32 dwprevtime =0;
static UINT8 btimes=0;
#endif
__MAYBE_UNUSED__ static UINT16 *line21_data;
__MAYBE_UNUSED__ static UINT8 *cc_field_data;
__MAYBE_UNUSED__ static INT32 line21_wr=0;
__MAYBE_UNUSED__ static INT32 line21_rd=0;
__MAYBE_UNUSED__ static INT32 line21_cnt=0;
__MAYBE_UNUSED__ static BOOL pre_g_disable_process_cc=TRUE;
__MAYBE_UNUSED__ static BOOL chan1=FALSE;
__MAYBE_UNUSED__ static BOOL chan2=FALSE;
__MAYBE_UNUSED__ static BOOL displayed_char=FALSE;
__MAYBE_UNUSED__ static BOOL flag_end_of_caption=FALSE;
__MAYBE_UNUSED__ static BOOL flash_on=FALSE;//flash control
__MAYBE_UNUSED__ static BOOL flash_flag=FALSE;//flash control
__MAYBE_UNUSED__ static BOOL b_turn_on_text=FALSE;
__MAYBE_UNUSED__ static UINT8 flash_idx=0;
__MAYBE_UNUSED__ static UINT8 flash_data_idx=0;
__MAYBE_UNUSED__ static BOOL flag_edm=FALSE;//Erased Displayed Memory
__MAYBE_UNUSED__ static UINT16 no_vbi_rating_count=0;
__MAYBE_UNUSED__ static UINT8 vbi_diff_count=0;
__MAYBE_UNUSED__ static UINT8 g_pre_vbi27_cc_by_osd = 0;
__MAYBE_UNUSED__ static UINT8 underline=0;
__MAYBE_UNUSED__ static UINT8 draw_black=0;
__MAYBE_UNUSED__ static UINT8 pre_pos_y=0;
__MAYBE_UNUSED__ static UINT8 pos_x=0;
__MAYBE_UNUSED__ static UINT8 pos_y=0;
__MAYBE_UNUSED__ static UINT8 color=0;
__MAYBE_UNUSED__ static UINT8 italics=0;
__MAYBE_UNUSED__ static UINT8 g_cc_mode = 0xff;
__MAYBE_UNUSED__ static UINT8 pop_on_string[15][34];
__MAYBE_UNUSED__ static UINT8 pop_on_string_color[15][34];//color
__MAYBE_UNUSED__ static UINT8 pop_on_string_italic[15][34];//italic
__MAYBE_UNUSED__ static UINT8 pop_on_string_underline[15][34];//underline
__MAYBE_UNUSED__ static UINT8 pop_on_valid[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
__MAYBE_UNUSED__ static UINT8 record_pop_on_valid[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//
__MAYBE_UNUSED__ static UINT8 pop_on_startx[15];
__MAYBE_UNUSED__ static UINT8 pop_on_endx[15];
__MAYBE_UNUSED__ static INT8 g_start_row=0;
__MAYBE_UNUSED__ static UINT8 g_dest_row=0;
__MAYBE_UNUSED__ static UINT8 g_num_rollup=0;
__MAYBE_UNUSED__ static UINT16 flash_count=0;
__MAYBE_UNUSED__ static UINT16 cc_parsing_count=0; //check_cc_valid_count=0
__MAYBE_UNUSED__ static BOOL vchip_flag=FALSE;
__MAYBE_UNUSED__ static BOOL xds_flag=FALSE;
__MAYBE_UNUSED__ static BOOL flash_show_on=FALSE;
__MAYBE_UNUSED__ static BOOL cgmsa_flag=FALSE; //for cgmsa
//EAS======================================
static BOOL eas_text_on = FALSE;
static UINT16 eas_len = 0;
static UINT8 eas_text[MAX_EAS_LEN];
__MAYBE_UNUSED__ static struct flash_data_cell flash_data[20];
__MAYBE_UNUSED__ static UINT8 push_count=0;
__MAYBE_UNUSED__ static UINT16 pre_data=0xffff;
__MAYBE_UNUSED__ static UINT16 pre_data_1=0xffff;

/**********************
* FUNCTION PROTOTYPES *
***********************/
/********************************************
* NAME: vbi_line21_push
*       push user data for cc decode
*
* Returns : BOOL
*              1        : TRUE
*              0        : FALSE
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 data                   IN  video_user_data cc part
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
BOOL vbi_line21_push(UINT16 data)
{
    line21_data=(UINT16*)g_atsc_cc_config_par.bs_buf_addr;
    if(line21_cnt>=LINE21_DEPTH)
    {
        //soc_printf("full\n");
        return FALSE;
    }
    if(clean_cc_buffer)
    {
        if(push_count < CC_PUSH_TOTAL_COUNT)
        {
            line21_data[line21_wr] = 0;
            push_count++;
            //libc_printf("%d",push_count);
        }
        else
        {
            push_count = 0;
            clean_cc_buffer=FALSE;
        }
    }
    else
    {
        line21_data[line21_wr] = data;
    }
    line21_wr = (line21_wr>=LINE21_DEPTH-1)?0: line21_wr+1;
    line21_cnt++;
    return TRUE;
}
/********************************************
* NAME: cc_push_field
*       push filed of user data for cc decode
*
* Returns : BOOL
*              1        : TRUE
*              0        : FALSE
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 data                    IN  corresponding field information of video_user_data for ccdecode
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
BOOL cc_push_field(UINT8 field)
{
    if(line21_cnt>=LINE21_DEPTH)
    {
        return FALSE;
    }
    cc_field_data=(UINT8*)g_atsc_cc_config_par.bs_buf_addr+0x800; //shift 2KB //vicky20110128
    cc_field_data[line21_wr] = field;
    return TRUE;

}
/********************************************
* NAME: vbi_line21_pop
*       pop user data for cc decode
*
* Returns : BOOL
*              1        : TRUE
*              0        : FALSE
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 data                   IN/OUT  video_user_data cc part
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
BOOL vbi_line21_pop(UINT16 *data)
{
    if((0==line21_cnt)|| (NULL == data))
    {
        return FALSE;
    }
    line21_data=(UINT16*)g_atsc_cc_config_par.bs_buf_addr; //vicky20110128
    *data = line21_data[line21_rd];
    line21_rd = (line21_rd>=LINE21_DEPTH-1)?0: line21_rd+1;
    line21_cnt--;
    return TRUE;
}
/********************************************
* NAME: cc_pop_field
*       pop field of user data for cc decode
*
* Returns : BOOL
*              1        : TRUE
*              0        : FALSE
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 data                   IN/OUT  corresponding field information of video_user_data for ccdecode
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
BOOL cc_pop_field(UINT8 *field)//for know filed 1 or field 2
{
    if((0==line21_cnt)||(NULL == field))
    {
        return FALSE;
    }

    cc_field_data=(UINT8*)g_atsc_cc_config_par.bs_buf_addr+0x800; //shift 2KB
    *field = cc_field_data[line21_rd];

    return TRUE;
}
#ifdef  CC_BY_OSD   //vicky20101229 dbg
static OSAL_ID cc_clear_alarm_id = OSAL_INVALID_ID;

/********************************************
*
* Returns : void
* Parameter     				Flow    Description
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void delete_cc_clear_timer(void)
{
	//libc_printf("[CC_DEBUG] delete_cc_clear_timer\n");
    if (cc_clear_alarm_id != OSAL_INVALID_ID)
    {
		osal_timer_delete(cc_clear_alarm_id);
		cc_clear_alarm_id = OSAL_INVALID_ID;
    }
}


/********************************************
* NAME: cc_clear_timeout_handle
*       clear cc string if there is no data in 16s
*
* Returns : void
* Parameter     				Flow    Description
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
static void cc_clear_timeout_handle(void)
{
    //libc_printf("[CC_DEBUG] cc_clear_timeout_handle\n");
	cc_fill_char(0,0,35,15,0xff);
	delete_cc_clear_timer();
}




/********************************************
* NAME: create_cc_clear_timer
*
* Returns : void
* Parameter     				Flow    Description
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void create_cc_clear_timer(void)
{
	delete_cc_clear_timer();
	
    OSAL_T_CTIM     t_dalm;
	t_dalm.callback = (TMR_PROC)cc_clear_timeout_handle;
    t_dalm.type = TIMER_ALARM;
    t_dalm.time  = 16000;

    cc_clear_alarm_id = osal_timer_create(&t_dalm);
}

/********************************************
* NAME: cc_clear_timer_set
*       set cc clear timer(ms)
*
* Returns : void
* Parameter     				Flow    Description
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
static void cc_clear_timer_set(unsigned long time)
{
	if(cc_clear_alarm_id == OSAL_INVALID_ID)
	{
		create_cc_clear_timer();
	}
	else
	{
		osal_timer_set(cc_clear_alarm_id, time);
	}
}

/********************************************
* NAME: miscellaneous_command
*       cc miscellaneous command decode
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 data                   IN  video user data cc part
* ------------------------------------------------------------------------------
* Additional information:
* plz refer to EIA/CEA-608-B, FCCˇs rule in 47CFR15.119
******************************************/
static void cc_row_up_caption_two(void)
{
    BOOL dec_depth= FALSE;

    if(g_cc_mode==CAPTION_POP_ON)
    {
        cc_fill_char(0,0,34,15,0xff);
    }
    if((g_num_rollup!=CLOSE_CAPTION_NORMAL_2) && ( CAPTION_ROLL_UP == g_cc_mode) &&
        (g_num_rollup>CLOSE_CAPTION_NORMAL_1))
    {
        dec_depth=TRUE;
    }
    g_num_rollup=2;
    color=0;
    if(!b_ccrecv_pac)
    {
        pos_x=0;
        pos_y=14;
        g_start_row = 13;
    }
    else
    {
        g_start_row = pos_y-g_num_rollup+1;
        if(g_start_row<0)
        {
            g_start_row++;
        }
    }
    if(dec_depth)
    {
        if(g_start_row>CLOSE_CAPTION_NORMAL_1)
        {
            cc_fill_char(0,g_start_row-1,35,1,0xff);
        }
        if(g_start_row>CLOSE_CAPTION_NORMAL_2)
        {
            cc_fill_char(0,g_start_row-2,35,1,0xff);
        }
    }
    g_cc_mode = CAPTION_ROLL_UP;
}

static void cc_row_up_caption_three(void)
{
    BOOL dec_depth= FALSE;

    if(g_cc_mode==CAPTION_POP_ON)
    {
        cc_fill_char(0,0,34,15,0xff);
    }
    if((g_num_rollup!=CLOSE_CAPTION_NORMAL_3) && (g_cc_mode == CAPTION_ROLL_UP) &&
        ( g_num_rollup>CLOSE_CAPTION_NORMAL_2))
    {
        dec_depth=TRUE;
    }

    g_num_rollup=3;
    color=0;
    if(!b_ccrecv_pac)
    {
        pos_x=0;
        pos_y=14;
        g_start_row = 12;
    }
    else
    {
        g_start_row = pos_y-g_num_rollup+1;//0-3+1=-2
        if(g_start_row<0)
        {
            g_start_row++;
        }
        if(g_start_row<0)
        {
            g_start_row++;
        }
    }
    if(dec_depth)
    {
        if(g_start_row>1)
        {
            cc_fill_char(0,g_start_row-1,35,1,0xff);
        }
    }
    g_cc_mode = CAPTION_ROLL_UP;
}

static void cc_row_up_caption_four(void)
{
    if(g_cc_mode==CAPTION_POP_ON)
    {
        cc_fill_char(0,0,34,15,0xff);
    }
    g_num_rollup=4;
    color=0;
    if(!b_ccrecv_pac)
    {
        pos_x=0;
        pos_y=14;
        g_start_row = 11;
    }
    else
    {
        g_start_row = pos_y-g_num_rollup+1;
        if(g_start_row<0)
        {
            g_start_row++;
        }
        if(g_start_row<0)
        {
            g_start_row++;
        }
        if(g_start_row<0)
        {
            g_start_row++;
        }
    }

    g_num_rollup=4;
    g_cc_mode = CAPTION_ROLL_UP;
}

static void cc_resume_caption_loading(void)
{
    if(flag_end_of_caption)
    {
        MEMSET(record_pop_on_valid,0x00,15);
        MEMSET(pop_on_string,0x00,15*34);
        MEMSET(pop_on_string_color,0x00,15*34);
        MEMSET(pop_on_string_italic,0x00,15*34);
        MEMSET(pop_on_string_underline,0x00,15*34);
        MEMSET(pop_on_valid,0x00,15);
        MEMSET(pop_on_startx,0x00,15);
        MEMSET(pop_on_endx,0x00,15);
        flag_end_of_caption=FALSE;
    }
    //pos_x=0;pos_y=14;color=0;
    flag_edm=FALSE;
    g_cc_mode = CAPTION_POP_ON;
}

static void cc_carriage_return_internal(void)
{
    pos_x=0;
    if(g_cc_mode==INFO_TEXT)
    {
        if(*ccg_vbi27_cc_by_osd > CLOSE_CAPTION_NORMAL_4)
        {
            pos_x=1;
            if(pos_y<CC_MAX_LINE)
            {
                pos_y++;
            }
            else
            {
                cc_carriage_return(7,14,0,8,TRUE);//TRUE for Text
            }
        }
        g_cc_mode=0xff;
    }
    else
    {
        if(*ccg_vbi27_cc_by_osd <= CLOSE_CAPTION_NORMAL_4)
        {
            cc_carriage_return(g_start_row,pos_y,0,8,FALSE);
        }
    }
}

static void cc_erase_non_display_memory(void)
{
    if(flash_idx!=0)
    {
        MEMSET(&flash_data, 0 , sizeof(struct flash_data_cell)*20);
    }
    flash_flag=FALSE;
    flash_on=FALSE;
    flash_idx=0;
    flash_data_idx=0;
    flag_edm=FALSE;
    MEMSET(record_pop_on_valid,0x00,15);
    MEMSET(pop_on_string,0x00,15*34);
    MEMSET(pop_on_string_color,0x00,15*34);
    MEMSET(pop_on_string_italic,0x00,15*34);
    MEMSET(pop_on_string_underline,0x00,15*34);
    MEMSET(pop_on_valid,0x00,15);
    MEMSET(pop_on_startx,0x00,15);
    MEMSET(pop_on_endx,0x00,15);
}

static void cc_erase_display_memory(void)
{
    if(flash_idx!=0)
    {
        MEMSET(&flash_data, 0 , sizeof(struct flash_data_cell)*20);
    }
    flag_edm=TRUE;
    flash_flag=FALSE;
    flash_on=FALSE;
    flash_idx=0;
    flash_data_idx=0;
    b_turn_on_text=FALSE;
    cc_fill_char(0,0,35,15,0xff);
}

static void miscellaneous_command(UINT16 data)
{
    UINT8 row = 0;
    UINT8 col = 0;
    UINT8 cmd2 = 0;

    cmd2 = (data&0xf);

    if( 0x04 ==((data>>CLOSE_CAPTION_NORMAL_8)&0x06) )
    {
        switch(cmd2)
        {
            case TEXT_RESTART:
            case RESUME_TEXT_DISPLAY:
                CCDEC1_PRINTF("<RESUME_TEXT_DISPLAY>\n");
                if(*ccg_vbi27_cc_by_osd>CLOSE_CAPTION_NORMAL_4)
                {
                   // g_cc_mode = INFO_TEXT;
                }
                break;
            case ROLLUP_CAPTION_2ROW:
                CCDEC1_PRINTF("<ROLLUP_CAPTION_2ROW>\n");
                cc_row_up_caption_two();
                break;
            case ROLLUP_CAPTION_3ROW:
                CCDEC1_PRINTF("<ROLLUP_CAPTION_3ROW>\n");
                cc_row_up_caption_three();
                break;
            case ROLLUP_CAPTION_4ROW:
                CCDEC1_PRINTF("<ROLLUP_CAPTION_4ROW>\n");
                cc_row_up_caption_four();
                break;
            case RESUME_CAPTION_LOADING:
                CCDEC1_PRINTF("<RESUME_CAPTION_LOADING>\n");
                cc_resume_caption_loading();
                break;

            case RESUME_DIRECT_CAPTIONING:
                CCDEC1_PRINTF("<RESUME_DIRECT_CAPTIONING>\n");
                pos_x=0;
                pos_y=14;
                color=0;
                g_cc_mode = CAPTION_PAINT_ON;
                draw_black=1;
                break;

            case CARRIAGE_RETURN:
                CCDEC1_PRINTF("<CARRIAGE_RETURN>\n");
                //soc_printf("<CARRIAGE_RETURN>\n");
                if(0xff == g_cc_mode)
                {
                    break;
                }
                cc_carriage_return_internal();
                break;

            case END_OF_CAPTION:
                CCDEC1_PRINTF("<END_OF_CAPTION>\n");
                //soc_printf("<END_OF_CAPTION>\n");
                flag_end_of_caption=TRUE;
                if((g_cc_mode != INFO_TEXT) && (g_cc_mode != CAPTION_POP_ON) &&
                    (CAPTION_ROLL_UP==g_cc_mode||CAPTION_PAINT_ON==g_cc_mode))
                {
                    cc_draw_char1(pos_x, pos_y,color, 0xff,0,0,0,0,0x20);//pre should be 0x7f//hbchen
                    cc_fill_char(0,0,35,15,0xff);
                    break;
                }
                if( 0xff==g_cc_mode)
                {
                    break;
                }
                cc_fill_char(0,0,35,15,0xff);
                UINT8 i=0;

                for(i=0;i<5;i++)
                {
                    osal_delay((WORD)1000000);
                }

                if(*ccg_vbi27_cc_by_osd <= CLOSE_CAPTION_NORMAL_4)
                {
                    for(row=0;row<15;row++)
                    {
                        if(1 == pop_on_valid[row] )
                        {
                            cc_draw_char1(pop_on_startx[row], row,0, 0xff,0,0,0,0,0x20);//pre should be 0x7f
                            for(col=pop_on_startx[row];col<pop_on_endx[row];col++)
                            {
                                cc_draw_char1(col+1, row,pop_on_string_color[row][col], 0xff,0,0,
                                pop_on_string_underline[row][col],pop_on_string_italic[row][col],
                                pop_on_string[row][col]);
                            }
                            cc_draw_char1(pop_on_endx[row]+1, row,0, 0xff,0,0,0,0,0x20);//aft
                            //pop_on_valid[row] = 0;
                        }
                    }
                }
                if(flag_edm)
                {
                    MEMSET(record_pop_on_valid,0x00,15);//
                    MEMSET(pop_on_string,0x00,15*34);
                    MEMSET(pop_on_string_color,0x00,15*34);
                    MEMSET(pop_on_string_italic,0x00,15*34);
                    MEMSET(pop_on_string_underline,0x00,15*34);
                    MEMSET(pop_on_valid,0x00,15);
                    MEMSET(pop_on_startx,0x00,15);
                    MEMSET(pop_on_endx,0x00,15);
                    flag_edm=FALSE;
                }
                pos_x=0;
                pos_y=14;
                color=0;
                //g_cc_mode = 0xff;
                break;
            case BACKSPACE:
                CCDEC1_PRINTF("<BACKSPACE>\n");
                if(0xff == g_cc_mode)
                {
                    break;
                }
                cc_fill_char(--pos_x,pos_y,1,1,0xff);
                break;
            case DELETE_TO_END_OF_ROW:
                CCDEC1_PRINTF("<DELETE_TO_END_OF_ROW>\n");
                //soc_printf("D,%d\n",change_row);
                if(0xff == g_cc_mode)
                {
                    break;
                }
                cc_fill_char(pos_x,pos_y,34-pos_x,1,0xff);
                if((g_cc_mode != INFO_TEXT) && b_ccrecv_pac &&( *ccg_vbi27_cc_by_osd < 0x05))
                {
                    cc_draw_char1(pos_x, pos_y,color, 0xff,0,0,0,0,0x20);//aft
                }
                break;
            case ERASE_DISPLAYED_MEMORY:
                CCDEC1_PRINTF("<ERASE_DISPLAYED_MEMORY>\n");
                cc_erase_display_memory();
				delete_cc_clear_timer();
                break;
            case ERASE_NONDISPLAYED_MEMORY:
                CCDEC1_PRINTF("<ERASE_NONDISPLAYED_MEMORY>\n");
                cc_erase_non_display_memory();
                break;
            case FLASH_ON:
                CCDEC1_PRINTF("<FLASH_ON>\n");
                flash_flag=TRUE;
                flash_on=TRUE;
                flash_data[flash_idx].start_x=pos_x;
                flash_data[flash_idx].y=pos_y;
                flash_data[flash_idx].fg_color=color;
                flash_data[flash_idx].bg_color=0x07;//CC_BLACK;
                flash_data[flash_idx].underline=underline;
                flash_data[flash_idx].italics=italics;
                flash_idx++;
                draw_black=1;
                break;
            default:
                break;
        }
    }
    else
    {
        switch(cmd2)
        {
            case TAB_OFFSET_1:
                pos_x+=1;
                break;
            case TAB_OFFSET_2:
                pos_x+=2;
                break;
            case TAB_OFFSET_3:
                pos_x+=3;
                break;
            default:
                break;
         }
    }
}

#ifdef CC_MONITOR_CC    //vicky0110216
void add_cc(void)
{
    w_cmdcnt++;
    if(CC_WIN_CMD_MAX_CNT==w_cmdcnt)
    {
        //libc_printf("Add cc cnt overflow, divide 2\n");
        w_cmdcnt>>=1;
        w_prevcnt>>=1;
    }
}
void gather_cc(void)
{
    if(w_cmdcnt>0)
    {
        bcontrol=TRUE;
    }
    else
    {
        bcontrol=FALSE;
    }
}
void remove_cc(void)
{
    if(w_cmdcnt<=w_prevcnt)
    {
        w_cmdcnt=0;
    }
    w_prevcnt=w_cmdcnt;
}

void scan_cc(void)
{
    if(bfirst)
    {
        dwprevtime=os_get_tick_count();
        w_prevcnt=0;
        w_cmdcnt=0;
        bcontrol=FALSE;
        bfirst=FALSE;
    }
    DWORD dwtime=os_get_tick_count();

    if((dwtime>dwprevtime)&&((dwtime-dwprevtime)>g_atsc_cc_config_par.w_polling_time))
    {
        btimes++;
        if(btimes==g_atsc_cc_config_par.b_ccscan)
        {
            btimes=0;
            remove_cc();
        }
        dwprevtime=dwtime;
        gather_cc();
        //libc_printf("CC Control Exist(%d)-",bcontrol);
    }
}
#endif
//end


static UINT8 cc_set_pos_y(UINT8 pos_data)
{
    UINT8 pos_y = 0;

    switch(pos_data)
    {
        case 0x2:
            pos_y = 0;
        break;
        case 0x3:
            pos_y = 1;
        break;
        case 0x4:
            pos_y = 2;
        break;
        case 0x5:
            pos_y = 3;
        break;
        case 0xA:
            pos_y = 4;
        break;
        case 0xB:
            pos_y = 5;
        break;
        case 0xC:
            pos_y = 6;
        break;
        case 0xD:
            pos_y = 7;
        break;
        case 0xE:
            pos_y = 8;
        break;
        case 0xF:
            pos_y = 9;
        break;
        case 0x0:
            pos_y = 10;
        break;
        case 0x6:
            pos_y = 11;
        break;
        case 0x7:
            pos_y = 12;
        break;
        case 0x8:
            pos_y =13;
        break;
        case 0x9:
            pos_y =14;
        break;
        default:
            pos_y = 0;
        break;
    }
    return pos_y;
}

static void cc_set_globle_param(UINT32 vbi_rating_temp)
{
    if(ccpre_vbi_rating!=vbi_rating_temp)
    {
        if(CLOSE_CAPTION_NORMAL_0==ccpre_vbi_rating)//vbi_rating_temp!=0
        {
            ccvbi_rating=vbi_rating_temp;
            ccpre_vbi_rating=ccvbi_rating;
            vbi_diff_count=0;
        }
        else if(CLOSE_CAPTION_NORMAL_3==vbi_diff_count)
        {
            vbi_diff_count=0;
            ccvbi_rating=vbi_rating_temp;
            ccpre_vbi_rating=ccvbi_rating;
        }
        else
        {
            vbi_diff_count++;
            ccvbi_rating=ccpre_vbi_rating;
        }
    }
    else
    {
        vbi_diff_count=0;
        ccvbi_rating=vbi_rating_temp;
        ccpre_vbi_rating=ccvbi_rating;
    }
}

static UINT8 cc_set_rating_system(UINT8 byte)
{
    UINT8 rating_system=0xff;

    if(0x18==(byte&0x18))
    {
        rating_system = CAN_ENG;//or CAN_FRE;//un support
    }
    else if(0x00==(byte&0x18))
    {
        rating_system = MPAA;
    }
    else if(0x08==(byte&0x18))
    {
        rating_system = US_TV;
    }
    else
    {
        rating_system = 0xff;
    }
    return rating_system;
}

static void cc_init_flash_param(void)
{
    UINT8 idx=0;
    UINT8 i=0;
  
    flash_show_on=TRUE;
    for(idx=0;idx<flash_idx;idx++)
    {
        for(i=0;i<flash_data[idx].end_x;i++)
        {
            if((flash_data[idx].string[i] >= 0x20) && (flash_data[idx].string[i]<=0x8f))
            {
                cc_draw_char1(flash_data[idx].start_x+i+1, flash_data[idx].y,
                flash_data[idx].fg_color, 0x07,0,0, flash_data[idx].underline,
                flash_data[idx].italics,flash_data[idx].string[i]);
            }
        }
    }
}

static void cc_init_param(void)
{
    UINT8 idx=0;

    flash_count=0;
    if(ccg_disable_process_cc==TRUE)
    {
        b_turn_on_text=FALSE;
        b_ccrecv_pac=b_turn_on_text;
        flash_flag=b_ccrecv_pac;
        g_pre_vbi27_cc_by_osd=0;
        if(flash_idx!=0)
            MEMSET(&flash_data, 0 , sizeof(struct flash_data_cell)*20);
        //return;
    }
    if( flash_flag)
    {
        if(flash_show_on==FALSE)
        {
            cc_init_flash_param();
        }
        else
        {
            flash_show_on=FALSE;
            for(idx=0;idx<flash_idx;idx++)
            {
                if(flash_data[idx].end_x!=0)
                {
                    cc_fill_char(flash_data[idx].start_x, flash_data[idx].y,
                        flash_data[idx].end_x,1,0x07);
                }
                else
                {
                    continue;
                }
            }
        }
    }
}

static UINT32 cc_set_vbi_rating_temp(UINT8 rating_system,UINT8 byte1,UINT8 byte2)
{
    UINT32 vbi_rating_temp=0;
    UINT8 rating=0xff;

    switch(rating_system)
    {
        case MPAA:
            CC_PRINTF("MPAA\n");
            rating=byte1&0x07;
            vbi_rating_temp |= ((rating+1)<<19);// ==>(rating+1) different between 608b and eia766-a
    #if 0
            switch(rating)
            {
            case MPAA_NA:
                vbi_rating_temp |= (1<<25);
                break;
            case MPAA_G:
                vbi_rating_temp |= (1<<18);
                break;
            case MPAA_PG:
                vbi_rating_temp |= (1<<19);
                break;
            case MPAA_PG13:
                vbi_rating_temp |= (1<<20);
                break;
            case MPAA_R:
                vbi_rating_temp |= (1<<21);
                break;
            case MPAA_NC17:
                vbi_rating_temp |= (1<<22);
                break;
            case MPAA_X:
                vbi_rating_temp |= (1<<23);
                break;
            case MPAA_NR:
                vbi_rating_temp |= (1<<24);
                break;
            }
    #endif
            break;
        case MPA3:
            CC_PRINTF("MPA3\n");
                            //ccvbi_rating=0;
            break;
        case US_TV:
            CC_PRINTF("US_TV\n");

            rating = byte2&0x07;
            switch(rating)
            {
                case TV_NONE:
                    CC_PRINTF("TV_NONE\n");
                    vbi_rating_temp |= (1<<18);
                    break;
                case TV_Y:
                    CC_PRINTF("TV_Y\n");
                    vbi_rating_temp |= 0x1;
                    break;
                case TV_Y7:
                    CC_PRINTF("TV_Y7\n");
                    vbi_rating_temp |= (1<<1);
                    if(byte2&0x20)
                    {
                        vbi_rating_temp |= (1<<2);
                    }
                    break;
                case TV_G:
                    CC_PRINTF("TV_G\n");
                    vbi_rating_temp |= (1<<3);
                    break;
                case TV_PG:
                    CC_PRINTF("TV_PG\n");
                    vbi_rating_temp |= (1<<4);
                    if(byte2&0x20)
                    {
                        vbi_rating_temp |= (1<<5);//v
                    }
                    if(byte2&0x10)
                    {
                        vbi_rating_temp |= (1<<6);//s
                    }
                    if(byte2&0x08)
                    {
                        vbi_rating_temp |= (1<<7);//l
                    }
                    if(byte1&0x20)
                    {
                        vbi_rating_temp |= (1<<8);//d
                    }
                    break;
                case TV_14:
                    CC_PRINTF("TV_14\n");
                    vbi_rating_temp |= (1<<9);
                    if(byte2&0x20)
                    {
                        vbi_rating_temp |= (1<<10);//v
                    }
                    if(byte2&0x10)
                    {
                        vbi_rating_temp |= (1<<11);//s
                    }
                    if(byte2&0x08)
                    {
                        vbi_rating_temp |= (1<<12);//l
                    }
                    if(byte1&0x20)
                    {
                        vbi_rating_temp |= (1<<13);//d
                    }
                    break;
                case TV_MA:
                    CC_PRINTF("TV_MA\n");
                    vbi_rating_temp |= (1<<14);
                    if(byte2&0x20)
                    {
                        vbi_rating_temp |= (1<<15);//v
                    }
                    if(byte2&0x10)
                    {
                        vbi_rating_temp |= (1<<16);//s
                    }
                    if(byte2&0x08)
                    {
                        vbi_rating_temp |= (1<<17);//l
                    }
                    break;
                default:
                    CC_PRINTF("no rating\n");
                    //ccvbi_rating=0;
                    break;
            }
            break;
        case CAN_ENG:
            CC_PRINTF("CAN_ENG\n");
            //ccvbi_rating=0;
            break;
        case CAN_FRE:
            CC_PRINTF("CAN_FRE\n");
            //ccvbi_rating=0;
            break;
        default:
            CC_PRINTF("other rating\n");
            //ccvbi_rating=0;
            break;
    }
    return vbi_rating_temp;
}

static void cc_init_param1(void)
{
    if(CC_MAX_PARSE_COUNT==cc_parsing_count)
    {
        cc_parsing_count=0;
        //ccvbi_rating=0;
        if((*ccg_vbi27_cc_by_osd != CLOSE_CAPTION_NORMAL_0) &&
            (*ccg_vbi27_cc_by_osd < CLOSE_CAPTION_NORMAL_5))//text don't clean
        {
            cc_fill_char(0,0,35,15,0xff);
        }

    }
}

static void cc_init_param2(void)
{
    if(CC_NO_VBI_RATING_MAX_CNT==no_vbi_rating_count) //fix bug 02569
    {
        ccvbi_rating=0;
        ccpre_vbi_rating=0;
        //libc_printf("clean vbi_rating\n");
    }
}

static UINT8 cc_preamble_address(UINT16 data,UINT8 byte1,UINT8 byte2)
{
    UINT8 pos_data = 0;

    if(0x00==(byte1&0x08))
    {
        chan1=TRUE;
    }
    else
    {
        chan1=FALSE;
    }

    if(  !(chan1==(0x01==(*ccg_vbi27_cc_by_osd&0x01)) )  )
    {
        return CC_RETURN_CONTINUE;
        //continue;
    }
    //soc_printf("PAC\n");
    CCDEC1_PRINTF("\nPAC: %3x - %3x\n",byte1,byte2);

    if(flash_on)
    {
        flash_data_idx=0;
        flash_on=FALSE;
    }

    b_ccrecv_pac=TRUE;
    flag_edm=FALSE;
    displayed_char=FALSE;
    xds_flag=FALSE;

    if(byte2&0x01)
    {
        underline=1;
    }
    else
    {
        underline=0;
    }
    pos_data = ((data&0x0700)>>7)|((data&0x20)>>5);
    pos_y = cc_set_pos_y(pos_data);
    if(CAPTION_ROLL_UP == g_cc_mode)
    {
        //pos_y=14;//base row
        if( g_num_rollup > (pos_y+1) )
        {
            pos_y=g_num_rollup-1;
        }
        if((pre_pos_y!=0) && (pos_y!=pre_pos_y))
        {
            //CC_FillChar(0,0,34,15,0xff);
            g_dest_row=pos_y-g_num_rollup+1;
            cc_move_intact(g_start_row, g_dest_row, g_num_rollup);
            g_start_row=g_dest_row;
        }
        pre_pos_y=pos_y;
    }

    italics = 0;
    color = 0;
    if(0x10==(data&0x10))//indent
    {
        pos_x = ((data&0x0f)>>1)*4;
    }
    else
    {
        pos_x = 0;
        if(7==((data&0x0f)>>1 ))
        {
            italics = 1;
        }
        else
        {
            color = (data&0x0f)>>1;
        }
    }
    draw_black=1;
    CCDEC_PRINTF("pre:pos_x =%d,pos_y=%d,color=%d\n",pos_x,pos_y,color);
    return CC_RETURN_SUCCESS;
}

static UINT8 cc_midrow_code(UINT16 data,UINT8 byte1,UINT8 byte2)
{
    if(CC_MIDROW_CODE_1==byte1)
    {
        chan1=TRUE;
    }
    else
    {
        chan1=FALSE;
    }

    if(  !(chan1==(0x01==(*ccg_vbi27_cc_by_osd&0x01)) )  )
    {
        return CC_RETURN_CONTINUE;
    }
    //soc_printf("Mid row\n");
    CCDEC1_PRINTF("\nMid row: %3x - %3x\n",byte1,byte2);
    if(flash_on)
    {
        //flash_data[flash_idx++].end_x=pos_x;
        flash_data_idx=0;
        flash_on=FALSE;
    }
    xds_flag=FALSE;
    flag_edm=FALSE;

    if(CAPTION_POP_ON==g_cc_mode)
    {
        if(0==pop_on_valid[pos_y])
        {
            pop_on_valid[pos_y] = 1;
            pop_on_startx[pos_y] = pos_x;
        }
        pos_x = (pos_x>32)?32:pos_x;//hbchen
        pop_on_string[pos_y][pos_x++] =  0x20;
    }
    else //CAPTION_PAINT_ON or Roll up
    {
        if((*ccg_vbi27_cc_by_osd <= 0x4) &&(g_cc_mode != INFO_TEXT) &&
            b_ccrecv_pac && ((g_cc_mode ==CAPTION_ROLL_UP)||(g_cc_mode ==CAPTION_PAINT_ON)))
        {
            cc_draw_char1(pos_x++, pos_y,color, 0xff,0,0,0,0,0x20);
        }
    }
    italics = 0;;
    if(byte2&0x01)
    {
        underline=1;
    }
    else
    {
        underline=0;
    }

    if(7==((data&0x0f)>>1 ))
    {
        italics = 1;
    }
    else
    {
        color = (data&0x0f)>>1;
    }
    CCDEC_PRINTF("mid:pos_x =%d,pos_y=%d,color=%d\n",pos_x,pos_y,color);
    return CC_RETURN_SUCCESS;
}

static UINT8 cc_miscellaneous_code(UINT16 data,UINT8 byte1,UINT8 byte2)
{
    if((0x15==byte1) || (0x14==byte1) || (0x17==byte1))
    {
        chan1=TRUE;
    }
    else
    {
        chan1=FALSE;
    }
    if(!(chan1==(0x01==(*ccg_vbi27_cc_by_osd&0x01))))
    {
        return CC_RETURN_CONTINUE;
    }
    if(flash_on)
    {
        //flash_data[flash_idx++].end_x=pos_x;
        flash_data_idx=0;
        flash_on=FALSE;
    }
    xds_flag=FALSE;

    CCDEC1_PRINTF("\nmis: %3x - %3x\n",byte1,byte2);
    miscellaneous_command(data);
    //draw_black=1;
    return CC_RETURN_SUCCESS;
}

//static void cc_set_string_caption_pop(UINT16 data,UINT8 byte1,UINT8 byte2,BOOL special_char)
static void cc_set_string_caption_pop(UINT8 byte1,UINT8 byte2,BOOL special_char)
{
    //if(byte1== 0x89) continue;
    if(byte1!= 0x89)
    {
        if(0==pop_on_valid[pos_y])
        {
            pop_on_valid[pos_y] = 1;
            pop_on_startx[pos_y] = pos_x;
        }
        //pos_x = (pos_x>32)?32:pos_x;
        if(1==pop_on_valid[pos_y])
        {
             if(pop_on_startx[pos_y] > pos_x)
             {
                pop_on_startx[pos_y] = pos_x;
             }
        }
    }
    pos_x = (pos_x>32)?32:pos_x;//hbchen
    //CCDEC_PRINTF("%c",(data>>8)&0xff);
    if((byte1>=0x20) && (byte1 <=0xcf))//0X80-0X8F : special char
    {
        pop_on_string_color[pos_y][pos_x] =  color;
        pop_on_string_italic[pos_y][pos_x] =  italics;
        pop_on_string_underline[pos_y][pos_x] =  underline;
        pop_on_string[pos_y][pos_x++] =  byte1;//(data>>8)&0xff;
    }
    //pos_x = (pos_x>32)?32:pos_x;
    if(FALSE==special_char)
    {
       // CCDEC_PRINTF("%c",data&0xff);
        if((byte2>=0x20) && (byte2 <=0x7f))
        {
            pos_x = (pos_x>32)?32:pos_x;
            pop_on_string_color[pos_y][pos_x] = color;
            pop_on_string_italic[pos_y][pos_x] = italics;
            pop_on_string_underline[pos_y][pos_x] = underline;
            pop_on_string[pos_y][pos_x++] =  byte2;//data&0xff;
        }
        else if(byte2==CLOSE_CAPTION_NORMAL_0)
        {
            pop_on_string[pos_y][pos_x] =  0x20;
        }
    }
    if(1==pop_on_valid[pos_y])
    {
        if(pop_on_endx[pos_y] < pos_x)
        {
            pop_on_endx[pos_y] = pos_x;
        }
    }
}


static UINT8 cc_string_code(UINT16 data,UINT8 byte1,UINT8 byte2,UINT8  field)
{
    BOOL special_char=FALSE;
    BOOL drawed_char=FALSE;

    if(xds_flag && (CC_FIELD_2==field))
    {
        return CC_RETURN_CONTINUE;
    }
    if((FALSE==b_ccrecv_pac) && (g_cc_mode != INFO_TEXT))
    {
        return CC_RETURN_CONTINUE;
    }
    if((g_cc_mode !=INFO_TEXT) && ( *ccg_vbi27_cc_by_osd > 0x04 ) )
    {
         return CC_RETURN_CONTINUE;
    }
    if((INFO_TEXT==g_cc_mode) && ( *ccg_vbi27_cc_by_osd <= 0x04 ) )
    {
         return CC_RETURN_CONTINUE;
    }
    if(  !(chan1==(0x01==(*ccg_vbi27_cc_by_osd&0x01)) )  )
    {
         return CC_RETURN_CONTINUE;
    }
    if(INFO_TEXT==g_cc_mode)
    {
        if((b_turn_on_text==FALSE) && (ccg_disable_process_cc==FALSE))
        {
            cc_fill_char(0,7,35,8,0x7);
            pos_x=1;
            pos_y=7;
            color=0;
            b_turn_on_text=TRUE;
        }
    }
    CCDEC1_PRINTF("%c%c",byte1,byte2);
    if( ((0x11==byte1 || 0x19==byte1) && (byte2 >= 0x30 && byte2<=0x3f))|| \
    ((0x12==byte1 || 0x1A==byte1) && (byte2 >= 0x20 && byte2<=0x3f))|| \
    ((0x13==byte1 || 0x1B==byte1) && (byte2 >= 0x20 && byte2<=0x3f)) )//for special char
    {
        if(data == pre_data)
        {
            pre_data=0xffff;
            pre_data_1=data;
            return CC_RETURN_CONTINUE;
        }

        if((0x12==byte1) || (0x1A==byte1)||(0x13==byte1) || (0x1B==byte1))
        {
            if(pos_x!=0)
            {
                pos_x=pos_x-1;
            }
        }
        special_char=TRUE;
        if((0x11==byte1) || (0x19==byte1))
        {
            byte1=byte2+0x50;//byte2=0x30~0x3f //80~8f
        }
        else if((0x12==byte1) || (0x1A==byte1))
        {
            byte1=byte2+0x70;//byte2=0x20~0x3f // 90~af
        }
        else if((0x13==byte1) || (0x1B==byte1))
        {
            byte1=byte2+0x90;//byte2=0x20~0x3f // b0~cf
        }
        else
        {
            byte1=byte2+0x50;
        }
        if(g_cc_mode != CAPTION_POP_ON)
        {
            if((0x1139==pre_data_1) || (0x1939==pre_data_1))
            {
                if((0x1139==data) || (0x1939==data))
                {
                    byte1=byte2+0x50;
                }
                else
                {
                    byte1=byte2+0x50;
                    if(g_cc_mode != INFO_TEXT)
                    {
                        cc_draw_char1(pos_x-1, pos_y,color, 0xff,0,0,0,0,0x20);
                    }
                }
            }
            else
            {
                if((0x1139==data|| 0x1939==data)&&(TRUE==displayed_char))
                {
                    byte1=0x20;
                }
                else if((0x1139==data) || (0x1939==data))
                {
                    //byte1=byte2+0x50;
                    draw_black=0;
                }
            }
        }
    }
    // normal but pre_data are transparent, then fill the transparent solid
    else if((0x1139==pre_data_1) || (0x1939==pre_data_1))
    {

        if(g_cc_mode != CAPTION_POP_ON)
        {
            cc_draw_char1(pos_x-1, pos_y,color, 0xff,0,0,0,0,0x20);
        }

    }

    if(0xff==g_cc_mode)
    {
        return CC_RETURN_CONTINUE;
    }
    if( ((byte1 < 0x20) || (byte1 > 0x8f)) && ((byte2 < 0x20) || (byte2 > 0x7f)) )// 0x7f
    {
        return CC_RETURN_CONTINUE;
    }
    if(( CC_FIELD_2==field) && ( (CLOSE_CAPTION_NORMAL_0==(*ccg_vbi27_cc_by_osd)) ||
        (CLOSE_CAPTION_NORMAL_1==(*ccg_vbi27_cc_by_osd)) || (CLOSE_CAPTION_NORMAL_2==(*ccg_vbi27_cc_by_osd)) ||
        (CLOSE_CAPTION_NORMAL_5==(*ccg_vbi27_cc_by_osd)) ||(CLOSE_CAPTION_NORMAL_6==(*ccg_vbi27_cc_by_osd))))
    {
        return CC_RETURN_CONTINUE;
    }
    if( (1==field) && ((CLOSE_CAPTION_NORMAL_0==(*ccg_vbi27_cc_by_osd)) ||
        (CLOSE_CAPTION_NORMAL_3==(*ccg_vbi27_cc_by_osd)) ||(CLOSE_CAPTION_NORMAL_4==(*ccg_vbi27_cc_by_osd)) ||
       ( CLOSE_CAPTION_NORMAL_7==(*ccg_vbi27_cc_by_osd)) ||(CLOSE_CAPTION_NORMAL_8==(*ccg_vbi27_cc_by_osd))))
    {
        return CC_RETURN_CONTINUE;
    }

    //cc_parsing_count=0;//if not string for a while, clean screan
    cc_clear_timer_set(16000);
    displayed_char=TRUE;
    if(CAPTION_POP_ON==g_cc_mode)
    {
        //cc_set_string_caption_pop(data,byte1,byte2,special_char);
        cc_set_string_caption_pop(byte1,byte2,special_char);
    }
    else
    {
        if(flash_on)
        {
            flash_data[flash_idx-1].string[flash_data_idx++]=byte1;
            flash_data[flash_idx-1].string[flash_data_idx++]=byte2;
            flash_data[flash_idx-1].end_x=flash_data_idx+1;
        }
        if((g_cc_mode != INFO_TEXT) && (1==draw_black))
        {
            cc_draw_char1(pos_x++, pos_y,color, 0xff,0,0,0,0,0x20);
        }
        pos_x = (pos_x>32)?32:pos_x;

        if((byte1>=0x20) && (byte1 <= 0xcf))//0X80-0X8F : special char
        {
            cc_draw_char1(pos_x++, pos_y,color, 0xff,0,0,underline,italics,byte1);
            if(byte1!=0x89)
            {
                drawed_char=TRUE;
            }
        }
        if(FALSE==special_char)
        {
            if((byte2>=0x20) && (byte2 <=0x7f))
            {
                pos_x = (pos_x>32)?32:pos_x;
                cc_draw_char1(pos_x++, pos_y,color, 0xff,0,0,underline,italics,byte2);
                drawed_char=TRUE;
            }
        }
        if((g_cc_mode != INFO_TEXT) && drawed_char)
        {
            cc_draw_char1(pos_x, pos_y,color, 0xff,0,0,0,0,0x20);
        }
    }
    draw_black=0;
    return CC_RETURN_SUCCESS;
}

/********************************************
* NAME: cc_parsing
*       cc decode main function
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:
* plz refer to EIA/CEA-608-B, FCCˇs rule in 47CFR15.119
* This function will alwyas running and decoding accroding to user cc setting
* 1 flashing
* 2 pop user data
* 3 XDS decode: always decode
* 4 CMD decode: depend on g_disable_process_cc
* 5 String decode: depend on g_disable_process_cc and CMD decode
******************************************/
void cc_parsing(void)
{
    UINT16 data=0;
    UINT32 vbi_rating_temp=0;
    UINT8  field=0;
    UINT8 byte1=0;
    UINT8 byte2=0;
    UINT8 idx=0;
    UINT8 rating_system=0xff;
    //UINT8 rating=0xff;

    #ifdef CC_MONITOR_CC
    scan_cc();
    #endif
    flash_count++;
    if(DTVCC_TOTAL_FLASH_ID==flash_count)
    {
        cc_init_param();
    }
    //cc_parsing_count++;
    //cc_init_param1();
    no_vbi_rating_count++;
    cc_init_param2();
    while(TRUE ==cc_pop_field(&field))
    {
        if(FALSE == vbi_line21_pop(&data))
        {
            continue;
        }
        byte1=(data>>8)&0xff;
        byte2 = (data&0xff);
        //soc_printf("%3x %3x\n",byte1,byte2);
        if(vchip_flag && (CC_FIELD_2==field) && (byte1>0x0f))
        {
            //if((byte1>=0x01) && (byte1<=0x0f) && (CC_FIELD_2==field))
           // {
           //     continue;
            //}
            ccvbi_rating=0;
            vbi_rating_temp=0;
            rating_system=0xff;
            //rating=0xff;
            no_vbi_rating_count=0;
            CC_PRINTF("v-chip data: %d - %d \n",byte1,byte2);
            vchip_flag = FALSE;
            //handle_608-vchip-Content advisory
            rating_system = cc_set_rating_system(byte1);
            if(0xff==rating_system)
            {
                CC_PRINTF("rating_system is 0xff\n");
            }
            vbi_rating_temp =cc_set_vbi_rating_temp(rating_system,byte1,byte2);
            cc_set_globle_param(vbi_rating_temp);
            continue;
        }
        #if 1 //for cgmsa
        if(cgmsa_flag && (CC_FIELD_2==field) && (byte1>0x0f))
        {
            ccvbi_cgmsa = byte1; //bit4~3 CGMS-A bit1~2 APS
            cgmsa_flag = FALSE;
        }
        #endif
        if((byte1>=0x01) && (byte1<=0x0f) && (CC_FIELD_2==field))//Extended Data Services
        {

            //soc_printf("XDS: %3x - %3x \n",byte1,byte2);
            xds_flag=TRUE;
            if((0x01==byte1) && (0x05==byte2))
            {
                vchip_flag = TRUE;
                no_vbi_rating_count=0;
            }
            #if 1 //for cgmsa
            if((0x01==byte1) && (0x08==byte2))
            {
                cgmsa_flag = TRUE;
            }
            #endif
            if( ((0x09==byte1) && (0x01==byte2)) || ((0x09==byte1) && (0x02==byte2)))
            {
                //CC_PRINTF(rate_logo_pos+8, rate_logo_pos+2,0, 0xff,0,0,0,0,0x41);//discover EAS
                //CC_PRINTF(rate_logo_pos+9, rate_logo_pos+3,0, 0xff,0,0,0,0,0x45);//discover EAS
                //CC_PRINTF(rate_logo_pos+10, rate_logo_pos+4,0, 0xff,0,0,0,0,0x53);//discover EAS
            }

            if(CC_CGMSA_INVALID_FLAG== byte1)
            {
                xds_flag=FALSE;
                vchip_flag=FALSE;
                cgmsa_flag=FALSE;//for cgmsa
            }
            //soc_printf("xds_flag= %d - %3x \n",xds_flag,byte1);
            continue;

        }
        else if((byte1>=0x10&&byte1<=0x1f)&&(!(byte2>0x2f&&byte2<0x40)) && byte2>=0x20&&byte2<=0x7f && \
            (!((byte1==0x12 || byte1==0x13|| byte1==0x1A|| byte1==0x1B) && (/*byte2>0x1f&&*/byte2<0x40)  )))
        {   //(data<0x2000)//control data
        //#ifdef CC_MONITOR_CC
        //    add_cc();
       // #endif
            ccvbi_cc=TRUE;
            if( (CC_FIELD_2==field) && (CLOSE_CAPTION_NORMAL_0==*ccg_vbi27_cc_by_osd ||
                CLOSE_CAPTION_NORMAL_1==*ccg_vbi27_cc_by_osd || CLOSE_CAPTION_NORMAL_2==*ccg_vbi27_cc_by_osd ||
                CLOSE_CAPTION_NORMAL_5==*ccg_vbi27_cc_by_osd || CLOSE_CAPTION_NORMAL_6==*ccg_vbi27_cc_by_osd) )
            {
                continue;
            }
            if( (CC_FIELD_1==field) && (CLOSE_CAPTION_NORMAL_0==*ccg_vbi27_cc_by_osd ||
                CLOSE_CAPTION_NORMAL_3==*ccg_vbi27_cc_by_osd ||CLOSE_CAPTION_NORMAL_4==*ccg_vbi27_cc_by_osd ||
                CLOSE_CAPTION_NORMAL_7==*ccg_vbi27_cc_by_osd || CLOSE_CAPTION_NORMAL_8==*ccg_vbi27_cc_by_osd) )
            {
                continue;
            }
            pos_x = (pos_x>32)?32:pos_x;
            if(data == pre_data)
            {
                pre_data=0xffff;
                pre_data_1=data;
                continue;
            }
            //================don't handle cc data decision
            if( *ccg_vbi27_cc_by_osd !=  g_pre_vbi27_cc_by_osd)
            {
                g_pre_vbi27_cc_by_osd=*ccg_vbi27_cc_by_osd;
                b_turn_on_text=FALSE;
                b_ccrecv_pac=b_turn_on_text;
                chan2=b_ccrecv_pac;
                chan1=chan2;
                g_cc_mode = 0xff;
                cc_fill_char(0,0,35,15,0xff);
				delete_cc_clear_timer();
                for(idx=0;idx<15;idx++)
                {
                    pop_on_valid[idx]=0;
                }
            }

            if(TRUE==ccg_disable_process_cc)
            {
                b_turn_on_text=FALSE;
                b_ccrecv_pac=b_turn_on_text;
                flash_flag=b_ccrecv_pac;
                g_cc_mode = 0xff;
                //cc_parsing_count=0;
                //check_cc_valid_count=0;
                cc_clear_timer_set(16000);
                if(flash_idx!=0)
                {
                    MEMSET(&flash_data, 0 , sizeof(struct flash_data_cell)*20);
                }
                pre_g_disable_process_cc = ccg_disable_process_cc;
                continue;
            }
            if(pre_g_disable_process_cc!=ccg_disable_process_cc)
            {
                MEMSET(line21_data,0x00,LINE21_DEPTH);
                line21_rd=0;
                line21_wr=0;
                line21_cnt=0;
                MEMSET(cc_field_data,0x00,LINE21_DEPTH);
                data=0x0;
                byte2 = 0x0;
                byte1 = byte2;
                pre_g_disable_process_cc = ccg_disable_process_cc;
            }
            if( (byte2 >=0x40)  && (byte2 <= 0x7f) )//preamble address code
            {
                if(CC_RETURN_CONTINUE == cc_preamble_address(data,byte1,byte2))
                {
                    continue;
                }
            }
            else if((byte1==CC_MIDROW_CODE_1) || (byte1==CC_MIDROW_CODE_2))//Midrow code
            {
                if(CC_RETURN_CONTINUE == cc_midrow_code(data,byte1,byte2))
                {
                    continue;
                }
            }
            else if((0x15==byte1) || (0x14==byte1) || (0x17==byte1)||
                (0x1c==byte1)|| (0x1f==byte1) || (0x1d==byte1))  //0x1d for CC4 //Miscellaneous code
            {
                if(CC_RETURN_CONTINUE == cc_miscellaneous_code(data,byte1,byte2))
                {
                    continue;
                }
            }
        }
        else//string data
        {

            #ifdef CC_MONITOR_CC
                add_cc();
            #endif
            if(CC_RETURN_CONTINUE == cc_string_code(data,byte1,byte2,field))
            {
                continue;
            }
        }
        pre_data = data;
        pre_data_1 = data;
    }
}
#endif

void eas_on_off(BOOL on_off, UINT8 *text, UINT16 len)
{
    eas_text_on = on_off;
    EAS_PRINTF("eas = %d\n",eas_text_on);
    if (text)
    {
        eas_len = len;
        if (len > MAX_EAS_LEN)
            ASSERT(0);

        MEMCPY(eas_text, text, len);
    }
    else
    {
        line21_data=(UINT16*)g_atsc_cc_config_par.bs_buf_addr;
        cc_field_data=(UINT8*)g_atsc_cc_config_par.bs_buf_addr+0x800; //shift 2KB
        MEMSET(line21_data,0x00,LINE21_DEPTH);
        line21_rd=0;
        line21_wr=0;
        line21_cnt=0;
        MEMSET(cc_field_data,0x00,LINE21_DEPTH);
        eas_len = 0;
        MEMSET(eas_text,0,MAX_EAS_LEN);
    }
}
#ifdef  CC_BY_OSD
void eas_text_scroll(void)
{
    UINT16 i = 0;
    UINT16 j = 0;
    UINT16 k=0;

    while(eas_text_on)
    {
        i =0;
        while(i<(eas_len+OSD_EAS_CHAR_NUM))
        {
            if (i<OSD_EAS_CHAR_NUM)
            {
                k = i;
                j = 0;
            }
            else
            {
                k = OSD_EAS_CHAR_NUM-1;
                j = i -(OSD_EAS_CHAR_NUM-1);
            }
            do
            {
                if (!eas_text_on)
                {
                    goto EAS_EXIT;
                }
                eas_draw_char(OSD_EAS_CHAR_NUM-k-1, 0,EAS_FG_COLOR, EAS_BG_COLOR,0,0,0,eas_text[j]);

                j++;
                k--;
                osal_task_sleep(3);
            }while(j<=i);
            eas_update_line();
            i++;

        }
    }
EAS_EXIT:
    return;
}
#endif
#endif

