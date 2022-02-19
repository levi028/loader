/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_dtvcc_dec.c

   *    Description: This file includes UI control task framework
   *    History:
   *      Date                Author             Version       Comment
   *      ====                ======             =======       =======
   *      1.  2007.08.08      Xing lee            For ATSC project
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
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <math.h>
#include <api/libclosecaption/atsc_cc_init.h>
#include <api/libclosecaption/atsc_dtvcc_dec.h>
#include <api/libclosecaption/closecaption_osd.h>
#include "lib_closecaption_internal.h"
BOOL cc_block_start=FALSE;//a patch for sarnoff c13, c14
BOOL g_dtvcc_war=FALSE;
BOOL cr_b2t=FALSE;
BOOL last_block_byte=FALSE;
BOOL defined_windows=FALSE;
UINT8 dtvcc_pos_x=0;
UINT8 dtvcc_pos_y=0;
UINT8 g_dtv_cc_service=0;
#ifdef CC_MONITOR_CS
//#define PRINTSN
static unsigned char bsprev[63]={0};
static unsigned char bscur[63]={0};
unsigned char bsn_cc[33]={0}; // total + 2 * 16
#endif


#if 0
static UINT16 line21_data_dtvcc[LINE21_DEPTH_DTVCC];
#else
__MAYBE_UNUSED__ static UINT16 *line21_data_dtvcc = NULL;
#endif
__MAYBE_UNUSED__ static UINT8 dtvcc_service_buff[DEPTH_SERVICE_BUFF] = {0};
__MAYBE_UNUSED__ static INT32 line21_wr_dtvcc=0;
__MAYBE_UNUSED__ static INT32 line21_rd_dtvcc=0;
__MAYBE_UNUSED__ static INT32 line21_cnt_dtvcc=0;
__MAYBE_UNUSED__ INT16 wr_dtvcc=0;
__MAYBE_UNUSED__ INT16 rd_dtvcc=0;
__MAYBE_UNUSED__ INT16 cnt_dtvcc=0;
__MAYBE_UNUSED__ UINT8 g_cc_windows_cur_win=0;
BOOL  first_defined =FALSE;
/* foreground FLASHING */
BOOL fgflashing = FALSE;
BOOL bgflashing = FALSE;

 UINT8 dtvcc_flash_idx = 0;
 UINT8 dtvcc_flash_data_idx = 0;
/* foreground FLASHING */
DWORD dtvcc_timestamp=0;
DWORD g_cc_delay= 0;
BOOL CMD_DELAY=FALSE;
UINT16 pop_on_string_dtvcc[8][15][36];
UINT8 pop_on_string_dtvcc_fg[8][15][36];//fg color
UINT8 pop_on_string_dtvcc_bg[8][15][36];//bg color
UINT8 pop_on_string_dtvcc_ud[8][15][36];//under line
UINT8 pop_on_string_dtvcc_itl[8][15][36];//italic
UINT8 pop_on_valid_dtvcc[8][15];
UINT8 pop_on_startx_dtvcc[8][15];
UINT8 pop_on_endx_dtvcc[8][15];
__MAYBE_UNUSED__ static UINT8 g_start_row_dtvcc=0;
__MAYBE_UNUSED__ static UINT8 window_pre_param[8][2];//now record x,y
__MAYBE_UNUSED__ struct dtvcc_flash_data_cell dtvcc_flash_data[20];
/* foreground FLASHING */
struct ccwindow m_ccwindow[CC_MAX_REGION_NUM];

__MAYBE_UNUSED__ static UINT8 bfirst=TRUE;
__MAYBE_UNUSED__ static UINT32 dwprevtime = 0;
__MAYBE_UNUSED__ static UINT8 btimes=0;
__MAYBE_UNUSED__ static UINT16 dtvcc_parsing_count=0;
__MAYBE_UNUSED__ static UINT8 old_seq_number=0xff;
__MAYBE_UNUSED__ static UINT16 dtvcc_flash_count=0;
__MAYBE_UNUSED__ static BOOL g_dtvcc_flash_show_on=FALSE;
__MAYBE_UNUSED__ static int cc_set_color_i=0;
__MAYBE_UNUSED__ static BOOL  windows_exist = FALSE;
__MAYBE_UNUSED__ static UINT8 cc_def_windows_id=0;
__MAYBE_UNUSED__ static int cc_def_win_i = 0;
__MAYBE_UNUSED__ static int cc_clear_win_i=0;

#ifdef    CC_BY_OSD
static const UINT8 rgb_pallette[9][4] = {
                /*front*/
                {0x02, 0x02, 0x02, 0x0f},    //white
                {0x00, 0x02, 0x00, 0x0f},    //green
                {0x00, 0x00, 0x02, 0x0f},    //blue
                {0x00, 0x02, 0x02, 0x0f},    //cyan
                {0x02, 0x00, 0x00, 0x0f},       //red
                {0x02, 0x02, 0x00, 0x0f},    //yellow
                {0x02, 0x00, 0x02, 0x0f},    //magenta
                {0x00, 0x00, 0x00, 0x0f},    //black
                /*back*/
                {0, 0, 0, 0}    //transparent
};

static UINT8 get_service_number(void);

/****************************************************************************
*  Function: TransRGBtoIndex
*
*  Description: Transfer RGB valuse to ALi palette table index
*
****************************************************************************/
UINT8 trans_rgbto_index(UINT8 r,UINT8 g,UINT8 b)
{
    UINT8 i = 0;

    for(i=0;i<8;i++)
    {
        rgbsoc_printf("[%d][%d][%d] \n ",r,g,b);
        rgbsoc_printf("[%d][0%d][0%d][0%d] \n ",i,rgb_pallette[i][0],
            rgb_pallette[i][1],rgb_pallette[i][2]);
        if((r==rgb_pallette[i][0]) && (g==rgb_pallette[i][1]) &&(b==rgb_pallette[i][2]))
        {
            return i;
        }
    }
    return 0x09;

}
/****************************************************************************
*  Function: push_service_data
*
*  Description:
*
****************************************************************************/
INT16 push_service_data(UINT8 data)
{
    if(cnt_dtvcc>=DEPTH_SERVICE_BUFF)
    {
        xsoc_printf("sfull\n");
        return cnt_dtvcc;
    }
    xsoc_printf("s ( %x \n" ,data);
    dtvcc_service_buff[wr_dtvcc] = data;
    wr_dtvcc = (wr_dtvcc>=DEPTH_SERVICE_BUFF-1)?0: wr_dtvcc+1;
    cnt_dtvcc++;
    return cnt_dtvcc;
}
/****************************************************************************
*  Function: pop_service_data
*
*  Description:
*              
****************************************************************************/
INT16 pop_service_data(UINT8 *data)
{
    if((0==cnt_dtvcc) ||(NULL == data))
    {
        xsoc_printf("sempty \n");
        return cnt_dtvcc;
    }
    *data = dtvcc_service_buff[rd_dtvcc];
    xsoc_printf("s)=%x\n" ,*data);
    rd_dtvcc= (rd_dtvcc>=DEPTH_SERVICE_BUFF-1)?0: rd_dtvcc+1;
    cnt_dtvcc--;
    return cnt_dtvcc;
}
/****************************************************************************
*  Function: vbi_line21_push_dtvcc
*
*  Description: Push dtvcc data into line21_data_dtvcc[] buffer
*
****************************************************************************/
BOOL vbi_line21_push_dtvcc(UINT16 data)
{
    if(line21_cnt_dtvcc>=LINE21_DEPTH_DTVCC)
    {
        xsoc_printf("full\n");
        xsoc_printf("dtvcc full...\n"); //vicky20110210 dbg
        line21_cnt_dtvcc = 0;  //for ATSC cc
        return FALSE;
    }
    xsoc_printf(" ( %x \n" ,data);
    line21_data_dtvcc=(UINT16*)((UINT8*)g_atsc_cc_config_par.bs_buf_addr+0xC00);
    //shift 3KB
    line21_data_dtvcc[line21_wr_dtvcc] = data;
    line21_wr_dtvcc = (line21_wr_dtvcc>=LINE21_DEPTH_DTVCC-1)?0: line21_wr_dtvcc+1;
    line21_cnt_dtvcc++;
    return TRUE;
}
/****************************************************************************
*  Function: vbi_line21_pop_dtvcc
*
*  Description: Pop dtvcc data from line21_data_dtvcc[] buffer
*
****************************************************************************/
BOOL vbi_line21_pop_dtvcc(UINT16 *data)
{
    if((0==line21_cnt_dtvcc) ||(NULL == data))
    {
        xsoc_printf("empty \n");
        return FALSE;
    }
    line21_data_dtvcc=(UINT16*)((UINT8*)g_atsc_cc_config_par.bs_buf_addr+0xC00);
    //shift 3KB
    *data = line21_data_dtvcc[line21_rd_dtvcc];
    xsoc_printf(")=%x\n" ,*data);
    line21_rd_dtvcc= (line21_rd_dtvcc>=LINE21_DEPTH_DTVCC-1)?0: line21_rd_dtvcc+1;
    line21_cnt_dtvcc--;
    return TRUE;
}
/****************************************************************************
*  Function: miscellaneous_command_dtvcc
*
*  Description: decoder miscellaneous control code
*
****************************************************************************/
void miscellaneous_command_dtvcc(UINT8 data)
{
    UINT8 col = 0;
    UINT8 win_posy_plus_row_cnt=0;

    switch(data)
    {
        case CARRIAGE_RETURN:
            cc_printf("<CARRIAGE_RETURN>g_start_row_dtvcc =%d\n",g_start_row_dtvcc);
            /*
            soc_printf("CC_WindowsScrollDir = %x\n",
            m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsScrollDir);
            soc_printf("m_CCWindow[gCC_WindowsCurWin].\
            CSetWinAtt.CC_WindowsScrollDir = %x\n",m_CCWindow\
            [gCC_WindowsCurWin].CSetWinAtt.CC_WindowsScrollDir);
            */
            switch(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir)
            {
                case 0:
                    xsoc_printf("L->R\n");
                    if(dtvcc_pos_x-1 < (m_ccwindow[g_cc_windows_cur_win].pos_x))
                    {
                        dtvcc_carriage_return(m_ccwindow[g_cc_windows_cur_win].pos_y,\
                        (m_ccwindow[g_cc_windows_cur_win].pos_y+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont),\
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                        m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,\
                        m_ccwindow[g_cc_windows_cur_win].pos_x,m_ccwindow[g_cc_windows_cur_win].pos_x+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                        m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir);
                    }
                    else
                    {
                        dtvcc_pos_x--;
                    }
                    break;
                case 1:
                    xsoc_printf("R->L\n");
                    if(dtvcc_pos_x+1 > (m_ccwindow[g_cc_windows_cur_win].pos_x+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont))
                    {
                        dtvcc_carriage_return(m_ccwindow[g_cc_windows_cur_win].pos_y,\
                            (m_ccwindow[g_cc_windows_cur_win].pos_y+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont),\
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,\
                            m_ccwindow[g_cc_windows_cur_win].pos_x, m_ccwindow[g_cc_windows_cur_win].pos_x+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir);
                    }
                    else
                    {
                        dtvcc_pos_x++;
                    }
                    break;
                case 2:
                    xsoc_printf("T->B\n");
                    if(dtvcc_pos_y-1 < (m_ccwindow[g_cc_windows_cur_win].pos_y))
                    {
                        dtvcc_carriage_return(m_ccwindow[g_cc_windows_cur_win].pos_y,\
                            (m_ccwindow[g_cc_windows_cur_win].pos_y+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont),\
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,\
                            m_ccwindow[g_cc_windows_cur_win].pos_x,
                            m_ccwindow[g_cc_windows_cur_win].pos_x+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir);
                    }
                    else
                    {
                        dtvcc_pos_y--;
                    }
                    break;
                case 3:
                    xsoc_printf("B->T\n");
                    win_posy_plus_row_cnt =
                        (m_ccwindow[g_cc_windows_cur_win].pos_y+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont)>14?\
                        14:(m_ccwindow[g_cc_windows_cur_win].pos_y+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont);
                    if(dtvcc_pos_y+1 > win_posy_plus_row_cnt)
                    {
                        dtvcc_carriage_return(m_ccwindow[g_cc_windows_cur_win].pos_y,\
                            (m_ccwindow[g_cc_windows_cur_win].pos_y+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont),\
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,\
                            m_ccwindow[g_cc_windows_cur_win].pos_x, m_ccwindow[g_cc_windows_cur_win].pos_x+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir);

                        for(col=0;col<35;col++)
                        {
                            pop_on_string_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y][col] = 0;
                            pop_on_string_dtvcc_fg[g_cc_windows_cur_win][dtvcc_pos_y][col]=0;
                            pop_on_string_dtvcc_bg[g_cc_windows_cur_win][dtvcc_pos_y][col]=0;
                            pop_on_string_dtvcc_ud[g_cc_windows_cur_win][dtvcc_pos_y][col]=0;
                            pop_on_string_dtvcc_itl[g_cc_windows_cur_win][dtvcc_pos_y][col]=0;
                        }
                    }
                    else
                    {
                        dtvcc_pos_y++;
                    }
                    break;
                default:
                    xsoc_printf("Default: L->R\n");
                    if(dtvcc_pos_y+1 > (m_ccwindow[g_cc_windows_cur_win].pos_y+
                        m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont))
                    {
                        dtvcc_carriage_return(m_ccwindow[g_cc_windows_cur_win].pos_y,\
                           (m_ccwindow[g_cc_windows_cur_win].pos_y+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont),\
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,\
                            m_ccwindow[g_cc_windows_cur_win].pos_x, m_ccwindow[g_cc_windows_cur_win].pos_x+
                            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont,\
                            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir);
                    }
                    else
                    {
                        dtvcc_pos_y++;
                    }
                break;
            }
            switch(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir)
            {
                case 0://L->R
                    dtvcc_pos_x= m_ccwindow[g_cc_windows_cur_win].pos_x;
                    break;
                case 1://R->L
                    dtvcc_pos_x= m_ccwindow[g_cc_windows_cur_win].pos_x+\
                        (UINT8)m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont;
                    break;
                case 2://T->B
                    dtvcc_pos_y=(UINT8)m_ccwindow[g_cc_windows_cur_win].pos_y;
                    break;
                case 3://B->T
                    dtvcc_pos_y=(UINT8)m_ccwindow[g_cc_windows_cur_win].pos_y+\
                        (UINT8)m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont;
                    break;
                default://L->R
                    dtvcc_pos_x= m_ccwindow[g_cc_windows_cur_win].pos_x;
                    break;
            }
/* foreground FLASHING */
            dtvcc_flash_idx++;
            if(dtvcc_flash_idx>=DTVCC_MAX_FLASH_ID)
            {
                dtvcc_flash_idx=19;
            }
            dtvcc_flash_data_idx = 0;
/* foreground FLASHING */
            break;

        case END_OF_CAPTION:
            //libc_printf("<END_OF_CAPTION>\n");
            if(last_block_byte && (m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible))
            {
                //libc_printf("\n<END_OF_CAPTION>\n");
                display_popup_window(g_cc_windows_cur_win,dtvcc_pos_y , dtvcc_pos_y);
            }
            //Display_Popup_Window(gCC_WindowsCurWin,dtvcc_pos_y , dtvcc_pos_y);
            break;
        case BACKSPACE:
            cc_printf("<BACKSPACE>\n");
            if(1==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)
            {
                cc_fill_char(--dtvcc_pos_x,dtvcc_pos_y,1,1,0xff);
            }
            else
            {
                dtvcc_pos_x--;
            }
            break;
        case DELETE_TO_END_OF_ROW:
            cc_printf("<DELETE_TO_END_OF_ROW>\n");
            cc_fill_char(0,dtvcc_pos_y,35,1,0xff);
            break;
        default:
            break;
    }
}
/****************************************************************************
*  Function: cc_parsing_dtvcc
*
*  Description: Parsing dtvcc data to Caption Channel Packet
*
****************************************************************************/
#ifdef CC_MONITOR_CS
void add_sn(unsigned char bno)
{
    if (0==bno)
    {
        return;
    }
    bno--;
    bscur[bno]++;
    if(0xFF==bscur[bno])
    {
        //libc_printf("Add sn cnt overflow, divide 2\n");
        bsprev[bno]>>=1;
        bscur[bno]>>=1;
    }
}
void gather_sn(void)
{
    unsigned char bcnt=0;
    unsigned char bi = 0;

    #if 0
    bcnt=4;
    bsn_cc[0]=bcnt;
    bsn_cc[1]=1;
    bsn_cc[2]=2;
    bsn_cc[3]=4;
    bsn_cc[4]=8;
    #else
    for(bi=0;bi<63;bi++)
    {
        if(bscur[bi])
        {
            bsn_cc[++bcnt]=bi+1;
            /*
            if(bcnt>16)
            {
                libc_printf("Service Number over flow\n");
                break;
            }
            */
        }
    }
    bsn_cc[0]=bcnt;
    #endif
}
void remove_sn(void)
{
    unsigned char bi = 0;

    for(bi=0;bi<63;bi++)
    {
        if(bscur[bi]<=bsprev[bi])
        {
            bscur[bi]=0;
        }
        bsprev[bi]=bscur[bi];
    }
}

void scan_sn(void)
{
    unsigned char *p_bsprev = bsprev;
    unsigned char *p_bscur = bscur;
    unsigned char *p_bsn = bsn_cc;

    if(bfirst)
    {
        dwprevtime=os_get_tick_count();
        MEMSET(p_bsprev,0,63);//MEMSET(bsprev,0,63);
        MEMSET(p_bscur,0,63);//MEMSET(bscur,0,63);
        MEMSET(p_bsn,0,33);//MEMSET(bsn_cc,0,33);
        bfirst=FALSE;
    }

    DWORD dwtime=os_get_tick_count();

    if((dwtime>dwprevtime)&&(dwtime-dwprevtime)>g_atsc_cc_config_par.w_polling_time)
    {
        //libc_printf("Ms(%d)\n",OS_GetTickCount());
        btimes++;
        if(btimes==g_atsc_cc_config_par.b_dtvccscan)
        {
            btimes=0;
            remove_sn();
        }
        dwprevtime=dwtime;
        gather_sn();

        #if 0
        unsigned char bi=0;
        libc_printf("SN Record-");
        for(bi=0;bi<63;bi++)
        {
            if(bscur[bi])
                libc_printf("%d(%d),",bi+1,bscur[bi]);
        }
        libc_printf("\n");
        #else
        /*
        libc_printf("Total(%d)-",bsn_cc[0]);
        if(bsn_cc[0]>=1)
        {
            for(bi=1;bi<=bsn_cc[0];bi++)
            {
                libc_printf("[%d],",bsn_cc[bi]);
            }
        }
        libc_printf("\n");
        */
        #endif

    }
}
#endif



static void dtvcc_draw_char(BOOL *dtvcc_flash_show_on)
{
    UINT8 idx = 0;
    UINT8 i = 0;

    if(NULL == dtvcc_flash_show_on)
    {
        libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return;
    }

    if(*dtvcc_flash_show_on==FALSE)
    {
        *dtvcc_flash_show_on=TRUE;
        for(idx=0;idx<dtvcc_flash_idx;idx++)
        {
            for(i=0;i<dtvcc_flash_data[idx].end_x;i++)
            {
                cc_draw_char1(dtvcc_flash_data[idx].start_x+i,
                    dtvcc_flash_data[idx].y, dtvcc_flash_data[idx].fg_color,
                    dtvcc_flash_data[idx].bg_color,0,0,dtvcc_flash_data[idx].underline,
                    dtvcc_flash_data[idx].italics,dtvcc_flash_data[idx].string[i]);
            }
        }
    }
    else
    {
        *dtvcc_flash_show_on=FALSE;
        if(( TRUE == fgflashing)&&(FALSE == bgflashing))
        {
            for(idx=0;idx<dtvcc_flash_idx;idx++)
            {
                for(i=0;i<dtvcc_flash_data[idx].end_x;i++)
                {
                    cc_draw_char1(dtvcc_flash_data[idx].start_x+i,\
                        dtvcc_flash_data[idx].y,  dtvcc_flash_data[idx].bg_color,
                        dtvcc_flash_data[idx].bg_color,0,0,\
                        dtvcc_flash_data[idx].underline, dtvcc_flash_data[idx].italics,\
                        dtvcc_flash_data[idx].string[i]);
                }
            }
        }
        else if(( FALSE == fgflashing)&&(TRUE == bgflashing))
        {
            for(idx=0;idx<dtvcc_flash_idx;idx++)
            {
                //if(DTVCC_flash_data[idx].end_x!=0)
                for(i=0;i<dtvcc_flash_data[idx].end_x;i++)
                {
                    cc_draw_char1(dtvcc_flash_data[idx].start_x+i,dtvcc_flash_data[idx].y,
                        dtvcc_flash_data[idx].fg_color,
                        m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,0,0,
                        dtvcc_flash_data[idx].underline, dtvcc_flash_data[idx].italics,
                        dtvcc_flash_data[idx].string[i]);
                }
                //m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsAttFillColor
            }
        }
        else if((TRUE == fgflashing)&&( TRUE == bgflashing))
        {
            for(idx=0;idx<dtvcc_flash_idx;idx++)
            {
                //if(DTVCC_flash_data[idx].end_x!=0)
                for(i=0;i<dtvcc_flash_data[idx].end_x;i++)
                {
                    cc_draw_char1(dtvcc_flash_data[idx].start_x+i,dtvcc_flash_data[idx].y,
                        m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,
                       m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color,0,0,
                       dtvcc_flash_data[idx].underline, dtvcc_flash_data[idx].italics,
                       dtvcc_flash_data[idx].string[i]);
                }
            }
        }
    }
}

static UINT8 dtvcc_pop_err(UINT16 *data)
{
     while((vbi_line21_pop_dtvcc(data) == FALSE) || (ccg_disable_process_cc!=0))
     {
         psoc_printf("PopDTVCC error\n");
         osal_task_sleep(1);
         if(ccg_disable_process_cc)
         {
             cc_cmd_reset();
             return 0;
         }
     }
     return 1;
}

static UINT8 dtvcc_parsing_data1(UINT16 *data,UINT8 *byte1,UINT8 *byte2,int *packet_size,int *block_size,
    int *service_number,BOOL *b_first_byte,struct service_block *sb,struct extend_service_block *esb)
{
    *block_size = *block_size-2;//block_size-=2;
    *packet_size = *packet_size-2;//packet_size-=2;
    *byte1=(*data&0xff00)>>8;
    *byte2=*data&0x00ff;
    if(( 0x07==sb->service_number ) && (*b_first_byte ==TRUE))
    {
        //service_number=esb.extended_service_number=(byte1&0x3f);
        esb->extended_service_number=(*byte1&0x3f);
        *service_number=esb->extended_service_number;
        #ifdef CC_MONITOR_CS
            if(*service_number > CLOSE_CAPTION_NORMAL_6)
            {
                add_sn(*service_number);
                #ifdef PRINTSN
                    libc_printf("2.Service(%d)\n",*service_number); //vicky20110210
                #endif
            }
        #endif
        *block_size+=1;
        psoc_printf(" esb.extended_service_number=%x\n",esb->extended_service_number);
        *b_first_byte=FALSE;
        last_block_byte=FALSE;
    }
    else
    {
        if(*service_number ==get_service_number())
        {
            dtvcc_parsing_count=0;
            cc_coding_layer_parsing(*byte1);
        }
    }
    if(*service_number ==get_service_number())
    {
        dtvcc_parsing_count=0;
        if(0==*block_size)
        {
            last_block_byte=TRUE;
        }
        cc_coding_layer_parsing(*byte2);
    }
    if(0==*packet_size)
    {
        psoc_printf(" ===============end packet end0===============\n");
        return 0;
    }
    return 1;
}

static UINT8 dtvcc_parsing_data2(  UINT16 *data,UINT8 *byte1,UINT8 *byte2,int *packet_size,int *block_size,
    int *service_number,BOOL *b_first_byte,struct service_block *sb,struct extend_service_block *esb)
{
    if(*data!=0xffff)
    {
        *packet_size = *packet_size -2;//packet_size-=2;
        psoc_printf("=======================>3packet_size = %d\n",*packet_size);
        *byte1=(*data&0xff00)>>8;
        *byte2=*data&0x00ff;
        //block_size=esb.block_size=sb.block_size=byte1&0x1F;
        sb->block_size=*byte1&0x1F;
        esb->block_size=sb->block_size;
        *block_size=esb->block_size;
        //service_number=esb.service_number=sb.service_number=(byte1&0xE0) >> 5;
        sb->service_number=(*byte1&0xE0) >> 5;
        esb->service_number=sb->service_number;
        *service_number=esb->service_number;

        psoc_printf(" 3service_number=%x,block_size=%x\n",*service_number,*block_size );
        if((0==*block_size) && ( 0==*service_number))
        {
              psoc_printf(" ===============end packet end4===============\n");
            return 0;
        }

         if(0x07==sb->service_number )
         {
             *block_size+=1;
             esb->extended_service_number=(*byte2&0x3f);
             *service_number=esb->extended_service_number;
             #ifdef CC_MONITOR_CS
                 if(*service_number > CLOSE_CAPTION_NORMAL_6)
                 {
                     add_sn(*service_number);
                     #ifdef PRINTSN
                         libc_printf("6.Service(%d)\n",*service_number); //vicky20110210
                     #endif
                 }
             #endif

             *b_first_byte=FALSE; //xing: for Sarnoff bitstream18 service even error
             return 2;
         }
         else
         {
             #ifdef CC_MONITOR_CS
                 add_sn(*service_number);
                 #ifdef PRINTSN
                     libc_printf("5.Service(%d)\n",*service_number); //vicky20110210
                 #endif
             #endif
         }
         *block_size-=1;
         if(*service_number ==get_service_number())
         {
             dtvcc_parsing_count=0;
             cc_coding_layer_parsing(*byte2);

         }
         if(0==*packet_size)
         {
             psoc_printf(" ===============end packet end5===============\n");
             return 0;
         }
         return 1;
    }
    else
    {
         return 1;
    }
}

static UINT8 dtvcc_parsing_data3(UINT16 *data,UINT8 *byte1,UINT8 *byte2,int *packet_size,int *block_size,
    int *service_number,BOOL *b_first_byte,struct service_block *sb,struct extend_service_block *esb)
{
    *block_size=0;
    *packet_size = *packet_size - 2;//packet_size-=2;
    *byte1=(*data&0xff00)>>8;
    *byte2=*data&0x00ff;
    if(( 0x07== sb->service_number) && (*b_first_byte ==TRUE))
    {
        //service_number=esb.extended_service_number=(byte1&0x3f);
        esb->extended_service_number=(*byte1&0x3f);
        *service_number=esb->extended_service_number;
        #ifdef CC_MONITOR_CS
            if(*service_number > CLOSE_CAPTION_NORMAL_6)
            {
                add_sn(*service_number);
                #ifdef PRINTSN
                    libc_printf("3.Service(%d)\n",*service_number); //vicky20110210
                #endif
            }
        #endif
        *block_size+=1;
        psoc_printf(" 2esb.extended_service_number=%x\n",esb->extended_service_number);
        *b_first_byte=FALSE;
        last_block_byte=FALSE;
        if(*service_number ==get_service_number())
        {
            dtvcc_parsing_count=0;
            cc_coding_layer_parsing(*byte2);
        }
    }
    else
    {
        if(*service_number ==get_service_number())
        {
            dtvcc_parsing_count=0;
            last_block_byte=TRUE;
            cc_coding_layer_parsing(*byte1);
        }
        psoc_printf(" ---------------end block 1------------------\n");
        //block_size=esb.block_size=sb.block_size=byte2&0x1F;
        sb->block_size=*byte2&0x1F;
        esb->block_size=sb->block_size;
        *block_size=esb->block_size;
        //service_number=esb.service_number=sb.service_number=(byte2&0xE0) >> 5;
        sb->service_number=(*byte2&0xE0) >> 5;
        esb->service_number=sb->service_number;
        *service_number=esb->service_number;
        psoc_printf(" service_number=%x,block_size=%x\n",*service_number,*block_size );
        if((0==*block_size) && (0==*service_number))
        {
            psoc_printf(" ===============end packet end11===============\n");
            return 0;
        }
    }
    if(0==*packet_size)
    {
        psoc_printf(" ===============end packet end1===============\n");
        return 0;
    }
    return 1;
}

static void dtvcc_parsing_data4(BOOL *cancel_dealy)
{
    #ifdef CC_MONITOR_CS
    scan_sn();
    #endif
    if(ccg_disable_process_cc)
    {
        cc_cmd_reset();
    }
    dtvcc_flash_count++;
    if(DTVCC_TOTAL_FLASH_ID==dtvcc_flash_count)
    {
        dtvcc_flash_count=0;
        if( (fgflashing || bgflashing) && dtvcc_flash_idx<DTVCC_TOTAL_FLASH_ID)
        {
            dtvcc_draw_char(&g_dtvcc_flash_show_on);
        }
    }
    dtvcc_parsing_count++;
    if(DTVCC_MAX_PARSE_COUNT==dtvcc_parsing_count)
    {
        dtvcc_parsing_count=0;
        if(g_dtv_cc_service > 0)//text don't clean
        {
            cc_cmd_reset();
        }

    }
    if((g_cc_delay <= (osal_get_tick()- dtvcc_timestamp)) && CMD_DELAY)
    {
        *cancel_dealy=TRUE;
        g_cc_delay=0;
        dtvcc_timestamp=0;
    }
}

static UINT8 dtvcc_parsing_data5(int *block_size,int *service_number,
    int *packet_size,struct caption_channel_packet *t_ccp,BOOL *b_first_byte)
{
    if((0==*block_size) && ( 0==*service_number))
    {
        psoc_printf(" ===============end packet end0===============\n");
        return 0;
    }
    (*packet_size)=(*packet_size)-2;
    if(t_ccp->sequence_number != 0x00)
    {
        if(((old_seq_number + 1) != t_ccp->sequence_number) &&
            (old_seq_number != t_ccp->sequence_number)) //N=N will be ignored
        {
            psoc_printf("Reset 1 old_seq_number=%d ,sequence_number=%d\n",
            old_seq_number,t_ccp->sequence_number);//Reset
            old_seq_number =t_ccp->sequence_number ;
            cc_cmd_reset();
            return 0;
        }
    }
    else
    {
        if((old_seq_number!= 0x03) && (old_seq_number!=0))//0=0 will be ignored
        {
            psoc_printf("Reset 2 old_seq_number=%d ,sequence_number=%d\n",
                old_seq_number,t_ccp->sequence_number);//Reset
            old_seq_number =     t_ccp->sequence_number ;
            cc_cmd_reset();
            return 0;
        }
    }
    old_seq_number = t_ccp->sequence_number ;
    if( 0x07==*service_number )
    {
        *b_first_byte=TRUE;
    }
    else
    {
        #ifdef CC_MONITOR_CS
            add_sn(*service_number);
            #ifdef PRINTSN
                libc_printf("1.Service(%d)\n",*service_number); //vicky20110210
            #endif
        #endif
    }
    return 1;
}


static UINT8 dtvcc_parsing_data6(UINT16 *data,UINT8 *byte1,UINT8 *byte2,int *packet_size,int *block_size,
    int *service_number,BOOL *b_first_byte,struct service_block *sb,struct extend_service_block *esb)
{
    UINT8 func_ret = 0;

    if( 0== (*block_size % 2))
    {
        func_ret = dtvcc_pop_err(data);
        if(CC_FUNC_RET_RETURN == func_ret)
        {
            return 0;
        }
        if(*data!=0xffff)
        {
            func_ret = dtvcc_parsing_data1(data,byte1,
                byte2,packet_size,block_size,
                service_number,b_first_byte,sb,esb);
            if(0 == func_ret)
            {
                return 0;
            }

        }
    }
    else if((1 == (*block_size % 2)) && (1== *block_size ))
    {
        func_ret = dtvcc_pop_err(data);
        if(CC_FUNC_RET_RETURN == func_ret)
        {
            return 0;
        }
        if(*data!=0xffff)
        {
            func_ret = dtvcc_parsing_data3(data,byte1,
                 byte2,packet_size,block_size,
                service_number,b_first_byte,sb,esb);
            if(CC_FUNC_RET_RETURN == func_ret)
            {
                return 0;
            }
            if(0x07== sb->service_number)
            {
                *b_first_byte=TRUE;
                return 2;
            }
        }
    }
    else
    {
        func_ret = dtvcc_pop_err(data);
        if(CC_FUNC_RET_RETURN == func_ret)
        {
            return 0;
        }
        if(*data!=0xffff)
        {
            last_block_byte=FALSE;
            (*block_size)=(*block_size)-2;
            (*packet_size) = (*packet_size) -2;//packet_size-=2;
            *byte1=(*data&0xff00)>>8;
            *byte2=(*data&0x00ff);
            if(( 0x07== sb->service_number) && (*b_first_byte ==TRUE))
            {
                esb->extended_service_number=(*byte1&0x3f);
                *service_number=esb->extended_service_number;
                #ifdef CC_MONITOR_CS
                    if(*service_number > CLOSE_CAPTION_NORMAL_6)
                    {
                        add_sn(*service_number);
                        #ifdef PRINTSN
                            libc_printf("4.Service(%d)\n",*service_number); //vicky20110210
                        #endif
                    }
                #endif
                (*block_size)=(*block_size)+1;
                psoc_printf(" 2esb.extended_service_number=%x\n",esb->extended_service_number);
                *b_first_byte=FALSE;
            }
            else
            {
                if(*service_number ==get_service_number())
                {
                    dtvcc_parsing_count=0;
                    cc_coding_layer_parsing(*byte1);
                }
            }
            if(*service_number ==get_service_number())
            {
                dtvcc_parsing_count=0;
                cc_coding_layer_parsing(*byte2);
            }
            if(0==*packet_size)
            {
                psoc_printf(" ===============end packet end2===============\n");
                return 0;
            }
        }
    }
    return 1;
}

void cc_parsing_dtvcc(void)
{
    UINT16 data = 0;
    UINT8 byte1 = 0;
    UINT8 byte2 = 0;
    int packet_size=0xff;
    int block_size=0;
    int service_number=0;
    struct service_block sb;
    struct extend_service_block esb;
    struct caption_channel_packet t_ccp;
    BOOL b_first_byte=TRUE;
    BOOL cancel_dealy=FALSE;
    UINT8 func_ret = 0;

    MEMSET(&sb,0,sizeof(struct service_block));
    MEMSET(&esb,0,sizeof(struct extend_service_block));
    MEMSET(&t_ccp,0,sizeof(struct caption_channel_packet));
    dtvcc_parsing_data4(&cancel_dealy);
    while((vbi_line21_pop_dtvcc(&data)==TRUE ||cancel_dealy) && (ccg_disable_process_cc!=0))
    {

        if(cancel_dealy)
        {
            data=0x8e;
            cancel_dealy=FALSE;
            cc_coding_layer_parsing(0x8e);
        }
        if(0xffff ==data )
        {
            psoc_printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
            func_ret = dtvcc_pop_err(&data);
            if(0 == func_ret)
            {
                return;
            }
            if(data != 0xffff)
            {
                byte1=(data&0xff00)>>8;
                byte2=(data&0x00ff);
                t_ccp.sequence_number = ((byte1 & 0xC0)>>6);
                t_ccp.packet_size=(byte1 & 0x3F);
                packet_size = (t_ccp.packet_size == 0)? 128:(t_ccp.packet_size * 2);
                sb.block_size=byte2&0x1F;
                esb.block_size=sb.block_size;
                block_size=esb.block_size;
                sb.service_number=(byte2&0xE0) >> 5;
                esb.service_number=sb.service_number;
                service_number=esb.service_number;
                psoc_printf(" ==packet_size=%x,sequence_number=%x==\n",packet_size,t_ccp.sequence_number );
                psoc_printf(" service_number=%x,block_size=%x\n",service_number,block_size );
                func_ret = dtvcc_parsing_data5(&block_size,&service_number,&packet_size,&t_ccp,&b_first_byte);
                if(0 == func_ret)
                {
                    return;
                }

LOOPBLOCK:
                while(packet_size>0)
                {
                    psoc_printf("=======================>packet_size = %d\n",packet_size);
                    last_block_byte=FALSE;
                    while(block_size>0)
                    {
                        cc_block_start=TRUE;
                        psoc_printf("=======================>21packet_size = %d block size=%d\n",
                                packet_size,block_size);
                        func_ret = dtvcc_parsing_data6(&data,&byte1,&byte2,&packet_size,&block_size,
                        &service_number,&b_first_byte,&sb,&esb);
                        if(CC_FUNC_RET_RETURN == func_ret)
                        {
                            return;
                        }
                        else if(CC_FUNC_RET_LOOP == func_ret)
                        {
                            goto LOOPBLOCK;
                        }

                    }
                    psoc_printf(" ---------------end block 0------------------ block_size=%d\n",block_size);
                    if(packet_size>0)
                    {
                        cc_block_start=FALSE;
                        psoc_printf("=======================>2packet_size = %d\n",packet_size);
                        func_ret = dtvcc_pop_err(&data);
                        if(CC_FUNC_RET_RETURN == func_ret)
                        {
                            return;
                        }

                        func_ret = dtvcc_parsing_data2(&data,&byte1,
                            &byte2,&packet_size,&block_size,
                            &service_number,&b_first_byte,&sb,&esb);
                        if(CC_FUNC_RET_RETURN == func_ret)
                        {
                            return;
                        }
                        if(CC_FUNC_RET_LOOP == func_ret)
                        {
                            goto LOOPBLOCK;
                        }

                    }
                }
            }
        }
        else
        {
            if(ccg_disable_process_cc)
            {
                cc_cmd_reset();
                return;
            }
        }
    }
}
/****************************************************************************
*  Function: Get_Service_number
*
*  Description: Get Service number from UI
*
****************************************************************************/
static UINT8 get_service_number(void)
{
    UINT8 res = g_dtv_cc_service;

    return res;
}
/****************************************************************************
*  Function: Display_Popup_Window
*
*  Description: Display text ( left justify , right justify or middle justify)
*
****************************************************************************/
void display_popup_window(UINT8 windows_id, UINT8 line_start, UINT8 line_end)
{

    UINT8 row = 0;
    UINT8 col = 0;
    UINT8 cur_pos = 0;
    UINT8 end_x_window  = 0;
    UINT8 start_x_window  = 0;
    UINT8 start_x_text  = 0;
    UINT8 end_x_text  = 0;

    for(row=line_start;row<=line_end;row++)
    {
        end_x_window = m_ccwindow[windows_id].cdef_win.cc_windows_column_cont + m_ccwindow[windows_id].pos_x;
        start_x_window = m_ccwindow[windows_id].pos_x;
        start_x_text= pop_on_startx_dtvcc[windows_id][row];
        end_x_text=pop_on_endx_dtvcc[windows_id][row];
        if( CLOSE_CAPTION_NORMAL_1==pop_on_valid_dtvcc[windows_id][row] )
        {
            switch(m_ccwindow[windows_id].cset_win_att.cc_windows_justify)
            {
                case 1:
                    cur_pos = end_x_window -(end_x_text - start_x_text  );
                break;
                case 2:
                    cur_pos = ((end_x_window -start_x_window+1)/2) -((end_x_text - start_x_text+1 ) / 2);
                    cur_pos++;
                break;
                default:
                    cur_pos = start_x_text;
                break;
            }
                //soc_printf(" \nrow =%d  pop_on_startx_dtvcc[WindowsID][row]=%x,
                //pop_on_endx_dtvcc[WindowsID][row]=%x\n",
                //row,pop_on_startx_dtvcc[WindowsID][row],pop_on_endx_dtvcc[WindowsID][row]);
                //soc_printf(" \n cur_pos =%d \n");
                for(col=start_x_text;col<end_x_text;col++,cur_pos++)
                {
                    cc_draw_char1(cur_pos, row,pop_on_string_dtvcc_fg[windows_id][row][col],\
                        (m_ccwindow[windows_id].cset_pen_color.bg_color==0x8||
                        m_ccwindow[windows_id].cset_pen_color.bg_color>16)?
                        (UINT8) m_ccwindow[windows_id].cset_win_att.cc_windows_att_fill_color:
                        m_ccwindow[windows_id].cset_pen_color.bg_color,\
                    0,0,pop_on_string_dtvcc_ud[windows_id][row][col],pop_on_string_dtvcc_itl[windows_id][row][col],\
                    pop_on_string_dtvcc[windows_id][row][col]);
                        xsoc_printf("%c",(pop_on_string_dtvcc[windows_id][row][col])&0xff);
                        xsoc_printf("Display Pop-On Data\n");
                }
        }
    }
}
/****************************************************************************
*  Function: CC_CMD_SetPenColor
*
*  Description: Assign styles to a dynamic preset style number P.78
*
****************************************************************************/

BOOL cc_cmd_set_pen_color(UINT8 param)
{
    UINT8 r_temp = 0;
    UINT8 g_temp = 0;
    UINT8 b_temp = 0;

    switch(cc_set_color_i)
    {
        case 1:
            r_temp=(R(param) == 2 ) ? 2 :( (R(param) < 2 ) ? 0 : 2);
            g_temp=(G(param) == 2 ) ? 2 :( (G(param) < 2 ) ? 0 : 2);
            b_temp=(B(param) == 2 ) ? 2 :( (B(param) < 2 ) ? 0 : 2);
            //m_CCWindow[gCC_WindowsCurWin].CSetPenColor.FG_Color=TransRGBtoIndex(r_temp,g_temp,b_temp);
            m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_opacity=(param>>6)&0x03;
            /* foreground FLASHING */
            if(CLOSE_CAPTION_NORMAL_3==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_opacity) // Transparent
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color=trans_rgbto_index(r_temp,g_temp,b_temp)+2*9;
            }
            else if(CLOSE_CAPTION_NORMAL_2==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color=trans_rgbto_index(r_temp,g_temp,b_temp)+1*9;
            }
            else if(CLOSE_CAPTION_NORMAL_1==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color=trans_rgbto_index(r_temp,g_temp,b_temp);
                //DTVCC_flash_data_idx = 0;
                fgflashing = TRUE;
                //DTVCC_flash_idx++;
            }
            else if(0==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color=trans_rgbto_index(r_temp,g_temp,b_temp);
            }
            /* foreground FLASHING */
            //m_CCWindow[gCC_WindowsCurWin].CSetPenColor.FG_Color = Param; //fg ARGB
        break;
        case 2:
            r_temp=(R(param) == 2 ) ? 2 :( (R(param) < 2 ) ? 0 : 2);
            g_temp=(G(param) == 2 ) ? 2 :( (G(param) < 2 ) ? 0 : 2);
            b_temp=(B(param) == 2 ) ? 2 :( (B(param) < 2 ) ? 0 : 2);
            //m_CCWindow[gCC_WindowsCurWin].CSetPenColor.BG_Color=TransRGBtoIndex(r_temp,g_temp,b_temp);
            m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_opacity=(param>>6)&0x03;
            /* foreground FLASHING */
            if(CLOSE_CAPTION_NORMAL_3==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_opacity)
            {// Transparent -> As window's color
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color=trans_rgbto_index(r_temp,g_temp,b_temp)+2*9;
            }
            else if(CLOSE_CAPTION_NORMAL_2==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color=trans_rgbto_index(r_temp,g_temp,b_temp)+1*9;
            }
            else if(CLOSE_CAPTION_NORMAL_1==m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color=trans_rgbto_index(r_temp,g_temp,b_temp);
                //DTVCC_flash_data_idx = 0;
                bgflashing = TRUE;
                //DTVCC_flash_idx++;
            }
            else if(0 == m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_opacity)
            {
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color=trans_rgbto_index(r_temp,g_temp,b_temp);
            }

            if((fgflashing == TRUE)||(bgflashing == TRUE))
            {
                dtvcc_flash_data_idx = 0;
                dtvcc_flash_idx++;
                if(dtvcc_flash_idx>=DTVCC_MAX_FLASH_ID)
                {
                    dtvcc_flash_idx=19;
                }
            }
            /* foreground FLASHING */
        break;
        case 3:
            r_temp=(R(param) == 2 ) ? 2 :( (R(param) < 2 ) ? 0 : 2);
            g_temp=(G(param) == 2 ) ? 2 :( (G(param) < 2 ) ? 0 : 2);
            b_temp=(B(param) == 2 ) ? 2 :( (B(param) < 2 ) ? 0 : 2);
            m_ccwindow[g_cc_windows_cur_win].cset_pen_color.ed_color= trans_rgbto_index(r_temp,g_temp,b_temp);
            xsoc_printf("=CC_CMD_SetPenColor =\n");
            rgbsoc_printf("fg=%d,P=%x,R=%d,G=%d,B=%d\n",
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color,param,r_temp,g_temp,b_temp);
            rgbsoc_printf("Bg=%d,P=%x,R=%d,G=%d,B=%d\n",
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color,param,r_temp,g_temp,b_temp);
            cc_set_color_i = 0;
            return TRUE;
        default:
            break;
    }
    cc_set_color_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_DefineWindows
*
*  Description: Create window and set initial parameters  P.63
*
****************************************************************************/

static void dtvcc_set_cc_window_xy(UINT8 windows_id)
{
    switch(m_ccwindow[windows_id].cdef_win.cc_windows_anchor_point)
    {
    case 1:
        if((m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2))>0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2);
        }
    break;
    case 2:
        if((m_ccwindow[windows_id].pos_x - m_ccwindow[windows_id].cdef_win.cc_windows_column_cont) >0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x - m_ccwindow[windows_id].cdef_win.cc_windows_column_cont;
        }
    break;
    case 3:
        if((m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2)) >0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2);
        }
    break;
    case 4:
        if((m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2))>0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2);
        }
        if((m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2))>0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2);
        }
    break;
    case 5:
        if((m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2))>0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - (m_ccwindow[windows_id].cdef_win.cc_windows_row_cont/2);
        }
        if((m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont))>0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x - (m_ccwindow[windows_id].cdef_win.cc_windows_column_cont);
        }
    break;
    case 6:
        if((m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont)>0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont;
        }
    break;
    case 7:
        if((m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont)>0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont;
        }
        if((m_ccwindow[windows_id].pos_x -( m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2))>0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x -( m_ccwindow[windows_id].cdef_win.cc_windows_column_cont/2);
        }
        //g_start_row_dtvcc = m_CCWindow[gCC_WindowsCurWin].pos_y-
        //m_CCWindow[WindowsID].CDefWin.CC_WindowsRowCont;
    break;
    case 8:
        if((m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont)>0)
        {
            m_ccwindow[windows_id].pos_y =
                m_ccwindow[windows_id].pos_y - m_ccwindow[windows_id].cdef_win.cc_windows_row_cont;
        }
        if((m_ccwindow[windows_id].pos_x - m_ccwindow[windows_id].cdef_win.cc_windows_column_cont)>0)
        {
            m_ccwindow[windows_id].pos_x =
                m_ccwindow[windows_id].pos_x - m_ccwindow[windows_id].cdef_win.cc_windows_column_cont;
        }
        //g_start_row_dtvcc = m_CCWindow[gCC_WindowsCurWin].pos_y-
        //m_CCWindow[WindowsID].CDefWin.CC_WindowsRowCont;
    break;
    default:
        break;
    }
}

BOOL cc_cmd_define_windows(UINT8 param)
{
    switch(cc_def_win_i)
    {
        case 0:
            cr_b2t=FALSE;
            cc_def_windows_id= param & 0x07;
            if(( 0x0f==m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_id )||(g_cc_windows_cur_win != cc_def_windows_id))
            {
                windows_exist=FALSE;
            }
            else
            {
                windows_exist=TRUE;
            }          
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_id=cc_def_windows_id;
            g_cc_windows_cur_win = cc_def_windows_id;
        break;
        case 1:
            if(windows_exist)
            {
                if((m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_lock != (( param >> 0x03 ) & 0x01)) ||\
                    (m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_lock != (( param >> 0x04 ) & 0x01)) ||\
                    (m_ccwindow[cc_def_windows_id].cdef_win.cc_windowsvisible != (( param >> 0x05 ) & 0x01)))
                {
                    cr_b2t=TRUE;
                }
            }
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_priority = param & 0x07; //P
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_lock = ( param >> 0x03 ) & 0x01;//CL
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_lock = (param>> 0x04) & 0x01; //RL
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windowsvisible = ( param >> 0x05 ) & 0x01;//V
            if (1 == m_ccwindow[cc_def_windows_id].cdef_win.cc_windowsvisible)
            {
                m_ccwindow[cc_def_windows_id].cc_windows_display_win = TRUE;
                m_ccwindow[cc_def_windows_id].cc_windows_hide_win = FALSE;
            }
            else
            {
                m_ccwindow[cc_def_windows_id].cc_windows_display_win = FALSE;
                m_ccwindow[cc_def_windows_id].cc_windows_hide_win = TRUE;
            }
        break;
        case 2:
            if(windows_exist)
            {
                if((m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_rel_pos != (( param >> 0x07) & 0x01)) ||\
                    (m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_ver != (param & 0x7f)))
                {
                    cr_b2t=TRUE;
                }
            }
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_rel_pos=( param >> 0x07) & 0x01;//RP
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_ver =  param & 0x7f ;//AV
            if(1==m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_rel_pos)
            {
                m_ccwindow[cc_def_windows_id].pos_y=
                    m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_ver* 15/100 ;
            }
            else
            {
                m_ccwindow[cc_def_windows_id].pos_y=
                    m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_ver* 15/74 ;
            }
        break;
        case 3:
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_hor=  param ;//AH
            if(g_dtvcc_war)
            {
                if(1 == m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_rel_pos)
                {
                    m_ccwindow[cc_def_windows_id].pos_x=
                        m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_hor* 33/99;
                }
                else
                {
                    m_ccwindow[cc_def_windows_id].pos_x=
                        m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_hor* 33/209;
                }
            }
            else
            {
                if(1 == m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_rel_pos)
                {
                    m_ccwindow[cc_def_windows_id].pos_x=
                        m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_hor* 33/99 ;
                }
                else
                {
                    m_ccwindow[cc_def_windows_id].pos_x=
                        m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_hor* 33/159 ;
                }
            }
        break;
        case 4:
            if(windows_exist)
            {
                if(m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_cont != ( param & 0x0f))
                {
                    cr_b2t=TRUE;
                }
            }
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_cont=  param & 0x0f;//RC
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_anchor_point=  (param >> 0x04 ) & 0x0f;//AP
        break;
        case 5:
            if((windows_exist)&&(m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont != (param & 0x3f)))
            {
                    cr_b2t=TRUE;
            }
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont = (param & 0x3f)> 0x1f ? 0x21: (param & 0x3f);
            dtvcc_set_cc_window_xy(cc_def_windows_id);
            g_start_row_dtvcc = m_ccwindow[cc_def_windows_id].pos_y ;
            if(m_ccwindow[cc_def_windows_id].pos_y>CC_MAX_LINE)
            {
                m_ccwindow[cc_def_windows_id].pos_y=14;
            }
            if(windows_exist == FALSE)
            {
                dtvcc_pos_x= m_ccwindow[cc_def_windows_id].pos_x;
                dtvcc_pos_y= m_ccwindow[cc_def_windows_id].pos_y;
                if(m_ccwindow[cc_def_windows_id].cdef_win.cc_windowsvisible)
                {
                    first_defined =TRUE;
                }
            }
        break;
        case 11:
            if((m_ccwindow[cc_def_windows_id].pos_x +
                m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont)>CLOSE_CAPTION_NORMAL_35)
            {
                m_ccwindow[cc_def_windows_id].pos_x =
                    ((35-1-m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont)>CLOSE_CAPTION_NORMAL_0) ? \
                (35-1-m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont) : 0;
            }
            if(windows_exist)
            {
                if((window_pre_param[cc_def_windows_id][0]!=m_ccwindow[cc_def_windows_id].pos_x)||\
                    (window_pre_param[cc_def_windows_id][1]!=m_ccwindow[cc_def_windows_id].pos_y))
                {
                    cr_b2t=TRUE;
                    if(1== m_ccwindow[cc_def_windows_id].cdef_win.cc_windowsvisible )
                    {
                        cc_move_intact(window_pre_param[cc_def_windows_id][1],\
                            m_ccwindow[cc_def_windows_id].pos_y, \
                            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_cont);
                        dtvcc_pos_x= m_ccwindow[cc_def_windows_id].pos_x;
                        dtvcc_pos_y= m_ccwindow[cc_def_windows_id].pos_y;
                    }
                }
            }
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_pen_style_id =  param & 0x07;
            m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_win_style_id =  (param >> 0x03) & 0x07;
            if(!((0 == m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_pen_style_id)&&(TRUE == windows_exist)))
            {
                cc_set_pen_style_id(cc_def_windows_id, m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_pen_style_id);
            }

            if(!((0 == m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_win_style_id)&&(TRUE == windows_exist)))
            {
               cc_set_window_style_id(cc_def_windows_id,m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_win_style_id);
            }
               cc_set_window_style_id(cc_def_windows_id,m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_win_style_id);
            if(0==m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_lock)
            {
                m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_column_cont=35-m_ccwindow[cc_def_windows_id].pos_x-1;
            }
            if(0==m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_lock)
            {
                if((m_ccwindow[cc_def_windows_id].pos_y+
                    m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_cont)>CC_MAX_LINE)
                {
                    m_ccwindow[cc_def_windows_id].cdef_win.cc_windows_row_cont=15-m_ccwindow[cc_def_windows_id].pos_y-1;
                }
            }
            window_pre_param[cc_def_windows_id][0]=m_ccwindow[cc_def_windows_id].pos_x;
            window_pre_param[cc_def_windows_id][1]=m_ccwindow[cc_def_windows_id].pos_y;
            fgflashing = FALSE;
            bgflashing = FALSE;
            dtvcc_flash_idx = 0;
            dtvcc_flash_data_idx = 0;
            MEMSET(&dtvcc_flash_data, 0 , sizeof(struct dtvcc_flash_data_cell)*20);
            cc_def_win_i = 0;
            defined_windows=TRUE;
            return TRUE;
        default:
        break;
    }
    cc_def_win_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_ClearWindows
*
*  Description:   Specify current wndow ID  P.62
*
****************************************************************************/
void cc_cmd_current_windows(UINT8 param)
{
    if(g_cc_windows_cur_win != (param & 0x07))
    {
        g_cc_windows_cur_win=param & 0x07;
        dtvcc_pos_x=m_ccwindow[g_cc_windows_cur_win].pos_x;
        dtvcc_pos_y=m_ccwindow[g_cc_windows_cur_win].pos_y;
    }
    if((1==(param & 0x07))&& m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)
    {
        display_popup_window(g_cc_windows_cur_win,dtvcc_pos_y , dtvcc_pos_y);
    }

    //libc_printf("    CC_CMD_CurrentWindows = %d\n",gCC_WindowsCurWin);
}
/****************************************************************************
*  Function: CC_CMD_ClearWindows
*
*  Description:   Clear Text from a set of windows P.68
*
****************************************************************************/

BOOL cc_cmd_clear_windows(UINT8 param)
{
    UINT16 mask_bit  = 0;
    UINT8 cnt_bit = 0;
    UINT8 cur_window_id =0;
    UINT8 width=0;
    UINT8 high=0;
    UINT8 window_clear_win = 0;
    UINT8 row=0;
    UINT8 col=0;

    if(1==cc_clear_win_i)
    {
        window_clear_win = param; //
        cc_clear_win_i=0;
        for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
        {
            cnt_bit++;
            if(window_clear_win & mask_bit)
            {
                cur_window_id = cnt_bit-1;//20070717 Xing
                xsoc_printf("        gCC_WindowsCurWin=%d\n",g_cc_windows_cur_win);
                xsoc_printf("        clear p=%d,%d\n",param,cur_window_id);
                //pos_y=m_CCWindow[cur_window_id].pos_y;
                //pos_x=m_CCWindow[cur_window_id].pos_x;
                width = m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont;
                high= m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont;
                //m_CCWindow[cur_window_id].CC_WindowsDisplayWin= FALSE;
                xsoc_printf("x=%x,y=%x,width=%x,high=%x\n",x,y,width,high);
                cc_fill_char((UINT16)m_ccwindow[cur_window_id].pos_x,
                    (UINT16)m_ccwindow[cur_window_id].pos_y,\
                    (UINT16)(width +1),(UINT16)(high+1),\
                    (m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible)?\
                    m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color:0xff);
#if 1
                for(row=0;row<15;row++)
                {
                    if(1 == pop_on_valid_dtvcc[cur_window_id][row])
                    {
                        xsoc_printf(" row =%d =>\n",row);
                        for(col=pop_on_startx_dtvcc[cur_window_id][row];
                        col<pop_on_endx_dtvcc[cur_window_id][row];col++)
                        {
                            pop_on_string_dtvcc[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_fg[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_bg[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_ud[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_itl[cur_window_id][row][col]=0;
                        }
                    }
                    pop_on_valid_dtvcc[cur_window_id][row] = 0x00;
                }
#endif
                xsoc_printf("CC_CMD_ClearWindows\n");

            }
        }
        //libc_printf("CC_CMD_ClearWindows\n");
        return TRUE;
    }
    cc_clear_win_i++;
    return FALSE;
}

/********************************************
* NAME: dtvccparsing_task
*       DTVcc task
*
* Returns : void
*
* Parameter     				Flow    Description
* ------------------------------------------------------------------------------
* UINT32 param1				IN	no use
* UINT32 param2				IN	no use
* ------------------------------------------------------------------------------
******************************************/
void dtvccparsing_task(UINT32 param1,UINT32 param2)
{
	dtvcc_init();
	while(1) // run forever when task exist.
	{
		osal_task_sleep(5); 
		
		#ifdef CC_MONITOR_CS	
			if (1)
		#else	
			if((g_dtv_cc_service != 0) && (0==*ccg_vbi27_cc_by_osd))//for DTVCC
		#endif
			{
				cc_parsing_dtvcc();
			}
	}
}

#endif

