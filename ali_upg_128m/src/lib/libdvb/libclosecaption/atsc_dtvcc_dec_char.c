/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_dtvcc_dec_char.c

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
#include <api/libclosecaption/closecaption_osd.h> //vicky20110128
#include "lib_closecaption_internal.h"


__MAYBE_UNUSED__ static BOOL delayin_delay=FALSE;

#ifdef    CC_BY_OSD
static UINT8  cc_ext_flag=0; // C0 Code Set : 0x03 EXT1 Extended Code
static UINT8  cc_vl_code_flag=0; // C3 Code Set : 0x90~0x9f Variable Length Code
static UINT8  cc_skip=0;
static BOOL b_command_finish = TRUE;
static UINT16 dtvcc_cmd_data=0x0000;
static int cc_delay_i=0;
static int cc_set_loc_i=0;
static int cc_set_atr_i=0;
static int cc_set_win_attr_i=0;
static int cc_tog_win_i=0;
static int cc_tog_win_j=0;
static int cc_hide_win_i=0;
static int cc_hide_win_idx=0;
static int cc_display_win_i=0;
static int cc_display_win_idx=0;
static int cc_del_win_i=0;
#endif
/* foreground FLASHING */

#ifdef    CC_BY_OSD
/****************************************************************************
*  Function: CC_CMD_DisplayWindows
*
*  Description:   Deletes window definitions for a set of windows  P.69
*
****************************************************************************/

static void cc_cmd_del_windows_para(UINT8 j,UINT8 cur_window_id,UINT8 delwin_temp,BOOL *del_win)
{
    if((m_ccwindow[j].cdef_win.cc_windowsvisible!=0) && (j!=cur_window_id) && !delwin_temp)
    {
        if((m_ccwindow[cur_window_id].pos_x<=m_ccwindow[j].pos_x && \
            (m_ccwindow[cur_window_id].pos_x+m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont+1)>
            m_ccwindow[j].pos_x && \
            m_ccwindow[cur_window_id].pos_y<=m_ccwindow[j].pos_y && \
            (m_ccwindow[cur_window_id].pos_y+m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont+1)>
            m_ccwindow[j].pos_y) ||\
            (m_ccwindow[j].pos_x<=m_ccwindow[cur_window_id].pos_x && \
            (m_ccwindow[j].pos_x+m_ccwindow[j].cdef_win.cc_windows_column_cont+1)>
            m_ccwindow[cur_window_id].pos_x && \
            m_ccwindow[j].pos_y<=m_ccwindow[cur_window_id].pos_y && \
            (m_ccwindow[j].pos_y+m_ccwindow[j].cdef_win.cc_windows_row_cont+1)>
            m_ccwindow[cur_window_id].pos_y))
        {
            *del_win=FALSE;
        }
    }
}


BOOL cc_cmd_del_windows(UINT8 param)
{
    UINT8 j=0;
    UINT16 mask_bit = 0x01;
    UINT8 win_del_win = 0;
    UINT8 cnt_bit = 0;
    UINT8 cur_window_id =0;
    UINT8 row = 0;
    UINT8 col = 0;
    UINT8 delwin[8]={0};//the widown will be del
    UINT8 delwin_temp = 0;
    BOOL del_win=TRUE;

    if(1==cc_del_win_i)
    {
        win_del_win = param; //
        //libc_printf("CC_CMD_DelWindows=%x\n",WindowsDelWin);
        if(0xff==win_del_win)
        {
            cc_fill_char(0,0,36,15,0xff);
            //for(cc_del_win_i=0;cc_del_win_i<8;cc_del_win_i++)
            cc_del_win_i=0;
            while(cc_del_win_i<CLOSE_CAPTION_MAX_WIN_NUM)
            {
				if(ccg_disable_process_cc==0)
				{
					return FALSE;
				}
                for(row=0;row<15;row++)//not finish xing
                {
                    for(col=0;col<34;col++)
                    {
                        pop_on_string_dtvcc[cc_del_win_i][row][col]=0;
                        pop_on_string_dtvcc_fg[cc_del_win_i][row][col]=0;
                        pop_on_string_dtvcc_bg[cc_del_win_i][row][col]=0;
                        pop_on_string_dtvcc_ud[cc_del_win_i][row][col]=0;
                        pop_on_string_dtvcc_itl[cc_del_win_i][row][col]=0;
                    }
                    pop_on_valid_dtvcc[cc_del_win_i][row] = 0x00;
                    pop_on_startx_dtvcc[cc_del_win_i][row]= 0x00;
                    pop_on_endx_dtvcc[cc_del_win_i][row]= 0x00;
                }
                MEMSET(&m_ccwindow[cc_del_win_i] , 0 , sizeof(struct ccwindow));
                m_ccwindow[cc_del_win_i].cset_pen_color.fg_color=8;//20070717 Xing
                m_ccwindow[cc_del_win_i].cset_pen_color.bg_color=8;
                m_ccwindow[cc_del_win_i].cset_win_att.cc_windows_att_fill_color=8;
                m_ccwindow[cc_del_win_i].cset_win_att.cc_windows_display_effect=0;
                m_ccwindow[cc_del_win_i].cdef_win.cc_windows_id=0xf;           
                m_ccwindow[cc_del_win_i].cdef_win.cc_windowsvisible = 0;
                m_ccwindow[cc_del_win_i].cc_windows_display_win= FALSE;
                m_ccwindow[cc_del_win_i].cc_windows_hide_win=TRUE;
                cc_del_win_i++;
            }
        }
        else
        {
            for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
            {
                cnt_bit++;
                if(win_del_win & mask_bit)
                {
                    cur_window_id = cnt_bit-1;
                    delwin[cur_window_id]=1;
                    //libc_printf("delwin=%d\n",CurrentWidnowID);
                }
            }
            cnt_bit=0;
            cur_window_id=0;
            for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
            {
               cnt_bit++;
               if(win_del_win & mask_bit)
               {
                    cur_window_id = cnt_bit-1;

                    xsoc_printf("CC_WindowsCurWin=%d\n",g_cc_windows_cur_win);
                    xsoc_printf("CC_CMD_DelWindows p=%d,%d\n",param,cur_window_id);

                    xsoc_printf("CC_CMD_DelWindows \n");
                    for(j=0;j<8;j++)// don't del overlap win
                    {
                        delwin_temp = delwin[j];
                        cc_cmd_del_windows_para(j,cur_window_id,delwin_temp,&del_win);
                    }
                    if(del_win)
                    {
                        cc_fill_char((UINT16)m_ccwindow[cur_window_id].pos_x,
                            (UINT16)m_ccwindow[cur_window_id].pos_y,\
                            (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont +1+1),\
                            (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont+1),0xff);
                    }
                    for(row=0;row<15;row++)//not finish xing
                    {
                        for(col=0;col<34;col++)
                        {
                            pop_on_string_dtvcc[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_fg[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_bg[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_ud[cur_window_id][row][col]=0;
                            pop_on_string_dtvcc_itl[cur_window_id][row][col]=0;
                        }
                        pop_on_valid_dtvcc[cur_window_id][row] = 0x00;
                    }

                    MEMSET(&m_ccwindow[cur_window_id] , 0 , sizeof(struct ccwindow));

                    m_ccwindow[cur_window_id].cset_pen_color.fg_color=8;
                    m_ccwindow[cur_window_id].cset_pen_color.bg_color=8;
                    m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color=8;
                    m_ccwindow[cur_window_id].cset_win_att.cc_windows_display_effect=0;
                    m_ccwindow[cur_window_id].cdef_win.cc_windows_id=0xf;
                    m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible = 0;
                    m_ccwindow[cur_window_id].cc_windows_display_win= FALSE;
                    m_ccwindow[cur_window_id].cc_windows_hide_win=TRUE;
                }
            }
        }
        /* foreground FLASHING */
        fgflashing = FALSE;
        bgflashing = FALSE;
        dtvcc_flash_idx = 0;
        dtvcc_flash_data_idx = 0;
        MEMSET(&dtvcc_flash_data, 0 , sizeof(struct dtvcc_flash_data_cell)*20);
        /* foreground FLASHING */
        cc_del_win_i=0;
        //libc_printf("CC_CMD_DelWindows\n");
        return TRUE;
    }
    cc_del_win_i++;
    return FALSE;
}

 /****************************************************************************
*  Function: CC_CMD_DisplayWindows
*
*  Description:   Cause a set of windows to become visible P.70
*
****************************************************************************/

static UINT8 cc_cmd_display_windows_internal(UINT8 cur_window_id)
{
    UINT8 width=0;
    UINT8 high=0;
    UINT8 opa_step=0;
    UINT8 display_win_step=0;

    if(m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible == 0)
    {
        m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible = 1;
        m_ccwindow[cur_window_id].cc_windows_display_win= TRUE;
        m_ccwindow[cur_window_id].cc_windows_hide_win = FALSE;
        width = m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont;
        high= m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont;
        if(CLOSE_CAPTION_NORMAL_1 == m_ccwindow[cur_window_id].cset_win_att.cc_windows_display_effect)//FADE
        {
        	if(0!=m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
        	{
        		opa_step = 15/m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
        	}
           // for(cc_display_win_idx=0;cc_display_win_idx<
           //     m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
            cc_display_win_idx=0;
            while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                dtvcc_modify_osdopacity((UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color,\
                    opa_step*cc_display_win_idx);
                osal_task_sleep(450);
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                cc_display_win_idx++;
            }
            dtvcc_modify_osdopacity((UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color,0xf);
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
            if(ccg_disable_process_cc)
            {
                return 4;
                //break;
            }
        }
        else if( CLOSE_CAPTION_NORMAL_2==m_ccwindow[cur_window_id].cset_win_att.cc_windows_display_effect )//WIPE
        {
            switch(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_dir)
            {
                case 0:// L->R
                	if(0!=m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                	{
                		display_win_step =
                		(width +1) / m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
                	}
                //for(cc_display_win_idx=0;cc_display_win_idx<
                //    m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
                cc_display_win_idx=0;
                while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                {
                    osal_task_sleep(450);
                    cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                        (UINT16) m_ccwindow[cur_window_id].pos_y,\
                        (UINT16) (display_win_step*cc_display_win_idx),(UINT16) (high+1),\
                        (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                    if(ccg_disable_process_cc)
                    {
                        break;
                    }
                    cc_display_win_idx++;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                break;
                case 1:// R->L
                	if(0!=m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                	{
                		display_win_step =
                        (width +1) / m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
                	}
               // for(cc_display_win_idx=0;cc_display_win_idx<
               //     m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
                cc_display_win_idx=0;
                while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                {
                    osal_task_sleep(450);
                    cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x+width+1-
                        cc_display_win_idx*display_win_step,(UINT16) m_ccwindow[cur_window_id].pos_y,\
                        (UINT16) (display_win_step*cc_display_win_idx),(UINT16) (high+1),\
                        (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                    if(ccg_disable_process_cc)
                    {
                        break;
                    }
                    cc_display_win_idx++;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                break;
                case 2:// T->B
                display_win_step=1;
               // for(cc_display_win_idx=0;cc_display_win_idx<
                //    m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
                cc_display_win_idx=0;
                while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                {
                    osal_task_sleep(450);
                    if(cc_display_win_idx>high+1)
                    {
                        break;
                    }
                    cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                        (UINT16) m_ccwindow[cur_window_id].pos_y,\
                        (UINT16) (width+1),(UINT16) (display_win_step*cc_display_win_idx),\
                        (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                    if(ccg_disable_process_cc)
                    {
                        break;
                    }
                    cc_display_win_idx++;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                break;
                case 3://B->T
                display_win_step=1;
               // for(cc_display_win_idx=0;cc_display_win_idx<
                //    m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
                cc_display_win_idx=0;
                while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                {
                    osal_task_sleep(450);
                    if(cc_display_win_idx>high+1)
                    {
                        break;
                    }
                    cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                        (UINT16) m_ccwindow[cur_window_id].pos_y+high+1-cc_display_win_idx,\
                        (UINT16) (width+1),(UINT16) (display_win_step*cc_display_win_idx),\
                        (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                    if(ccg_disable_process_cc)
                    {
                        break;
                    }
                    cc_display_win_idx++;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                break;
                default://L->R
                	if(0!=m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                	{
                		display_win_step =
                        (width +1) / m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
                	}
               // for(cc_display_win_idx=0;cc_display_win_idx<
               //     m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_display_win_idx++)
                cc_display_win_idx=0;
                while(cc_display_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
                {
                    osal_task_sleep(450);
                    cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                        (UINT16) m_ccwindow[cur_window_id].pos_y,\
                        (UINT16) (display_win_step*cc_display_win_idx),(UINT16) (high+1),\
                        (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                    if(ccg_disable_process_cc)
                    {
                        break;
                    }
                    cc_display_win_idx++;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                break;
            }
        }
        else//SNAP
        {
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
            (UINT16) m_ccwindow[cur_window_id].pos_y,\
            (UINT16) (width +1),\
            (UINT16) (high+1),\
            (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
        }
        first_defined=FALSE;
        display_popup_window(cur_window_id,0,14);
    }
    else
    {
        display_popup_window(cur_window_id,dtvcc_pos_y , dtvcc_pos_y);
    }
    return 0;
}

BOOL cc_cmd_display_windows(UINT8 param)
{
    UINT16 mask_bit  = 0;
    UINT8 cnt_bit = 0;
    UINT8 win_display_win =0;
    UINT8 cur_window_id =0;

    if(1==cc_display_win_i)
    {
        win_display_win = param;
        cc_display_win_i=0;
        for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
        {
           cnt_bit++;
           if(win_display_win & mask_bit)
           {
               cur_window_id = cnt_bit-1;
               xsoc_printf("        CC_CMD_DisplayWindows p=%d,id=%d\n",param,cur_window_id);
               xsoc_printf("disgCC_WindowsCurWin=%d\n",g_cc_windows_cur_win);
               xsoc_printf("Hide=%x,display=%x\n",m_ccwindow[cur_window_id].cc_windows_hide_win ,
                   m_ccwindow[cur_window_id].cc_windows_display_win);
               if(CLOSE_CAPTION_NORMAL_4 ==cc_cmd_display_windows_internal(cur_window_id))
               {
                   break;
               }
           }
        }
        return TRUE;
    }
    cc_display_win_i++;
    return FALSE;
}
 /****************************************************************************
*  Function: CC_CMD_TogWindows
*
*  Description:   Cause a set of windows to become invisible P.71
*
****************************************************************************/

static void cc_cmd_hide_windows_internal(UINT8 cur_window_id)
{
   UINT8 width=0;
   UINT8 high=0;
   UINT8 opa_step=0;
   UINT8 display_win_step=0;

   if(1 == m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible)
   {
        xsoc_printf(" gCC_WindowsCurWin=%d\n",g_cc_windows_cur_win);
        xsoc_printf(" CC_CMD_HideWindows p=%d,id=%d\n",param,cur_window_id);
        m_ccwindow[cur_window_id].cc_windows_display_win=FALSE;
        m_ccwindow[cur_window_id].cc_windows_hide_win = TRUE;
        m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible = 0;
        width = m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont;
        high= m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont;
        if(cc_block_start)
        {
            osal_task_sleep(75);
            cc_block_start=FALSE;
        }
        if(1 == m_ccwindow[cur_window_id].cset_win_att.cc_windows_display_effect)//FADE
        {
        	if(0 != m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
        	{
                opa_step = 15/m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
        	}
            //for(cc_hide_win_idx=0;cc_hide_win_idx<
            //    m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsEffectSpeed;cc_hide_win_idx++)
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                dtvcc_modify_osdopacity(0x7,15-opa_step*cc_hide_win_idx);
                osal_task_sleep(450);
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),\
                    (UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            dtvcc_modify_osdopacity((UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color,0xf);
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),0x08);
        }
        else if(m_ccwindow[cur_window_id].cset_win_att.cc_windows_display_effect ==CLOSE_CAPTION_NORMAL_2)//WIDE
        {
            switch(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_dir)
            {
            case 0:// L->R
            	if(0 != m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            	{
            		display_win_step =(width +1)/m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
            	}
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                osal_task_sleep(450);
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (display_win_step*cc_hide_win_idx),\
                    (UINT16) (high+1),0x8);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),0x8);
            break;
            case 1:// R->L
            	if(0!= m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            	{
            		display_win_step =
                    (width +1) / m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
            	}
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                osal_task_sleep(450);
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x+width+
                    1-cc_hide_win_idx*display_win_step,(UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (display_win_step*cc_hide_win_idx),(UINT16) (high+1),0x8);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>
                    CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                (UINT16) m_ccwindow[cur_window_id].pos_y,\
                (UINT16) (width +1),(UINT16) (high+1),0x8);
            break;
            case 2:// T->B
            display_win_step=1;
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                osal_task_sleep(450);
                if(cc_hide_win_idx>high+1)
                {
                    break;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (width+1),(UINT16) (display_win_step*cc_hide_win_idx),0x8);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>
                    CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),0x8);
            break;
            case 3://B->T
            display_win_step=1;
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                osal_task_sleep(450);
                if(cc_hide_win_idx>high+1)
                {
                    break;
                }
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y+high+1-cc_hide_win_idx,\
                    (UINT16) (width+1),(UINT16) (display_win_step*cc_hide_win_idx),0x8);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>
                    CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),0x8);
            break;
            default://L->R
            	if(0 != m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            	{
            		display_win_step =
            		(width +1) / m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed;
            	}
            cc_hide_win_idx=0;
            while(cc_hide_win_idx<m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed)
            {
                osal_task_sleep(450);
                cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                    (UINT16) m_ccwindow[cur_window_id].pos_y,\
                    (UINT16) (display_win_step*cc_hide_win_idx),(UINT16) (high+1),0x8);
                if(ccg_disable_process_cc)
                {
                    break;
                }
                //patch for DTVCC Sarnoff bitstream C70 Fade 7.5 sec
                if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed>
                    CLOSE_CAPTION_NORMAL_8)
                {
                    break;
                }
                cc_hide_win_idx++;
            }
            cc_fill_char((UINT16) m_ccwindow[cur_window_id].pos_x,\
                (UINT16) m_ccwindow[cur_window_id].pos_y,(UINT16) (width +1),(UINT16) (high+1),0x8);
            break;
            }
        }
        else
        {
            cc_fill_char((UINT16)m_ccwindow[cur_window_id].pos_x,
                (UINT16)m_ccwindow[cur_window_id].pos_y,\
            (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont +1),\
            (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont+1),0xff);
        }
    }
}

BOOL cc_cmd_hide_windows(UINT8 param)
{
    UINT16 mask_bit  = 0;
    UINT8 cnt_bit = 0;
    UINT8 cur_window_id =0;
    UINT8 win_hide_win = 0;

    if(1==cc_hide_win_i)
    {
        win_hide_win = param;
        cc_hide_win_i=0;
        for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
        {
           cnt_bit++;
           cur_window_id = cnt_bit-1;
           if(win_hide_win & mask_bit)
           {
               //CurrentWidnowID = CountBit-1;//20070717 Xing
               cc_cmd_hide_windows_internal(cur_window_id);
                xsoc_printf("        Hide=%x,display=%x\n",m_ccwindow[cur_window_id].cc_windows_hide_win ,
                m_ccwindow[cur_window_id].cc_windows_display_win);
           }
           else if(m_ccwindow[cur_window_id].cset_win_att.cc_windows_justify  != 0)
           {
               cc_fill_char((UINT16)(m_ccwindow[cur_window_id].pos_x),\
                       (UINT16)(dtvcc_pos_y),\
                   (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont +1),\
                   1,(UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
               display_popup_window(cur_window_id,dtvcc_pos_y , dtvcc_pos_y);
           }
           else if((m_ccwindow[cur_window_id].cc_windows_hide_win) &&
            (m_ccwindow[cur_window_id].cdef_win.cc_windows_id==cur_window_id))
           {
               cc_fill_char((UINT16)(m_ccwindow[cur_window_id].pos_x),\
                       (UINT16)(dtvcc_pos_y),\
                   (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont +1),\
                   1,(UINT8) m_ccwindow[cur_window_id].cset_win_att.cc_windows_att_fill_color);
               display_popup_window(cur_window_id,dtvcc_pos_y , dtvcc_pos_y);
           }
        }
        return TRUE;
    }
    cc_hide_win_i++;
    return FALSE;
}

/****************************************************************************
*  Function: CC_CMD_TogWindows
*
*  Description:   Toggles Display/Hide status of a set of windows  P.72
*
****************************************************************************/

BOOL cc_cmd_tog_windows(UINT8 param)
{
    UINT16 mask_bit  = 0;
    UINT8 cnt_bit = 0;
    UINT8 cur_window_id =0;
    UINT8 win_tog_win = 0;
    UINT8 tog_win[8]={0};

    if(1==cc_tog_win_i)
    {
        win_tog_win = param; //
        cc_tog_win_i=0;
        for(mask_bit = 1 ;mask_bit <= 0x80 ; mask_bit = mask_bit << 1)
        {
               cnt_bit++;
               if(win_tog_win & mask_bit)
               {
                   cur_window_id = cnt_bit-1;//20070717 Xing
                   //xsoc_printf("CC_CMD_TogWindows p=%d,%d\n",Param,CurrentWidnowID);
                if(m_ccwindow[cur_window_id].cc_windows_display_win)
                {

                    cc_fill_char((UINT16)m_ccwindow[cur_window_id].pos_x,(UINT16)m_ccwindow[cur_window_id].pos_y,\
                            (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_column_cont +1+1),\
                    (UINT16)(m_ccwindow[cur_window_id].cdef_win.cc_windows_row_cont+1),0xff);
                    xsoc_printf("        Hide=%x,display=%x\n",m_ccwindow[cur_window_id].cc_windows_hide_win ,
                        m_ccwindow[cur_window_id].cc_windows_display_win);
                }
                else
                {
                    tog_win[cur_window_id]=1;//display window and pop-up string
                }
                m_ccwindow[cur_window_id].cc_windows_hide_win = !m_ccwindow[cur_window_id].cc_windows_hide_win;
                m_ccwindow[cur_window_id].cc_windows_display_win= !m_ccwindow[cur_window_id].cc_windows_display_win;
                m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible =
                    !m_ccwindow[cur_window_id].cdef_win.cc_windowsvisible;

                xsoc_printf(" Hide=%x,display=%x\n",m_ccwindow[cur_window_id].cc_windows_hide_win ,
                    m_ccwindow[cur_window_id].cc_windows_display_win);
               }
        }
        cc_tog_win_j=0;
        while(cc_tog_win_j<CLOSE_CAPTION_MAX_WIN_NUM)
        {
            if(1==tog_win[cc_tog_win_j])
            {
                cc_fill_char((UINT16)m_ccwindow[cc_tog_win_j].pos_x,(UINT16)m_ccwindow[cc_tog_win_j].pos_y,\
                (UINT16)(m_ccwindow[cc_tog_win_j].cdef_win.cc_windows_column_cont +1),\
                (UINT16)(m_ccwindow[cc_tog_win_j].cdef_win.cc_windows_row_cont+1),
                m_ccwindow[cc_tog_win_j].cset_win_att.cc_windows_att_fill_color);
                display_popup_window(cc_tog_win_j,0,15);
                //Display_Popup_Window(gCC_WindowsCurWin,pos_y , pos_y);
                first_defined =FALSE;
            }
            cc_tog_win_j++;
        }
        //libc_printf("\nCC_CMD_TogWindows: %x\n",WindowsTogWin);
        return TRUE;
    }
    cc_tog_win_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_SetWinAttr
*
*  Description:   Defines the window styles for the current window P.73
*
****************************************************************************/

BOOL cc_cmd_set_win_attr(UINT8 param)
{
    UINT8 r_temp = 0;
    UINT8 g_temp = 0;
    UINT8 b_temp = 0;

    switch(cc_set_win_attr_i)
    {
         case 1:
            r_temp=(R(param) == 2 ) ? 2 :( (R(param) < 2 ) ? 0 : 2);
            g_temp=(G(param) == 2 ) ? 2 :( (G(param) < 2 ) ? 0 : 2);
            b_temp=(B(param) == 2 ) ? 2 :( (B(param) < 2 ) ? 0 : 2);
            //m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsAttFillColor=TransRGBtoIndex(r_temp,g_temp,b_temp);
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_opacity=A(param);

            if(CLOSE_CAPTION_NORMAL_3  == A(param)) // TRANSPARENT
            {
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color=
                    trans_rgbto_index(r_temp,g_temp,b_temp)+2*9;
            }
            else if( CLOSE_CAPTION_NORMAL_2==  A(param))
            {
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color=
                    trans_rgbto_index(r_temp,g_temp,b_temp)+1*9;
            }
            else
            {
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color=trans_rgbto_index(r_temp,g_temp,b_temp);
            }
            //m_CCWindow[gCC_WindowsCurWin].CSetWinAtt.CC_WindowsAttFillColor= (Param && 0x3f) ; //FARGB
        break;
        case 2:
            //Border type and Border Color
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_border_type=(param  >> 0x06 ) & 0x03;

            r_temp=(R(param) == 2 ) ? 2 :( (R(param) < 2 ) ? 0 : 2);
            g_temp=(G(param) == 2 ) ? 2 :( (G(param) < 2 ) ? 0 : 2);
            b_temp=(B(param) == 2 ) ? 2 :( (B(param) < 2 ) ? 0 : 2);
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_border_color=trans_rgbto_index(r_temp,g_temp,b_temp);
        break;
        case 3:
            //justify , scroll direction , print direction,WordWrap
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify=param  & 0x03;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir=(param>>2)  & 0x03;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir=(param>>4)  & 0x03;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_word_wrap=(param>>5)  & 0x03;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_border_type=\
                ((param  & 0x80)>>0x05) || (m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_border_type&0x03);
            xsoc_printf("\n\nJustify=%d,ScrollDir=%d,PrintDir=%d,WordWrap=%d\n",
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify,\
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_scroll_dir,\
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir,
                m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_word_wrap);
        break;
        case 4:
            //effect speed,effect direction,display effect
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_display_effect = param & 0x03 ;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_dir = (param>>2)&0x03 ;
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_effect_speed = (param>>4)&0x0f;
            //libc_printf("CC_CMD_SetWinAttr\n");
            cc_set_win_attr_i=0;
            return TRUE;
        default:
            break;
    }

    cc_set_win_attr_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_SetPenAtr
*
*  Description:  Assign pen style attributes for the current window P.76
*
****************************************************************************/

BOOL cc_cmd_set_pen_atr(UINT8 param)
{
    if(CLOSE_CAPTION_NORMAL_1==cc_set_atr_i)
    {
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_pen_size = param & 0x03; //FARGB
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_offset = (param >> 2 ) & 0x03;
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_text_tag = (param >> 4) & 0x0f;
        //xsoc_printf("i = 1\n");
    }
    else if(CLOSE_CAPTION_NORMAL_2==cc_set_atr_i)
    {
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics = (param >> 7 ) & 0x01;
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline = (param >> 6) & 0x01;
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_edge_type = (param >> 3) & 0x07;
        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_font_style =  param & 0x07;
        //xsoc_printf("i = 2\n");
        //Border type and Border Color
        if(CLOSE_CAPTION_NORMAL_0==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_lock)
        {
            //if(g_dtvcc_war)
                //m_CCWindow[WindowsID].CDefWin.CC_WindowsColumnCont=42-m_CCWindow[WindowsID].pos_x-1;
            //else
            if(0==m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_pen_size)
            {
                m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont=37-m_ccwindow[g_cc_windows_cur_win].pos_x-1;
            }
        }
        //libc_printf("CC_CMD_SetPenAtr: size:%d\n",m_CCWindow[gCC_WindowsCurWin].CSetPenAtt.CC_WinPenAttPenSize);
        cc_set_atr_i=0;
        return TRUE;
    }
    cc_set_atr_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_SetPenLoc
*
*  Description:  Specifies the pen cursor location within a window (p.79)
*
****************************************************************************/

BOOL cc_cmd_set_pen_loc(UINT8 param)
{
    if(CLOSE_CAPTION_NORMAL_1==cc_set_loc_i)
    {
        m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_row = param & 0x0f;
        //pos_y =  m_CCWindow[gCC_WindowsCurWin].pos_y+m_CCWindow[gCC_WindowsCurWin].CSetPenLoc.PenRow;
    }
    else if(CLOSE_CAPTION_NORMAL_2==cc_set_loc_i)
    {
        m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_column = param & 0x3f;
        cc_set_loc_i=0;

        if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify != CLOSE_CAPTION_NORMAL_0) // not left
        {
            // left-to-right or right-to-left
            if((CLOSE_CAPTION_NORMAL_0==m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir)||\
                (CLOSE_CAPTION_NORMAL_1==m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir))
            {
                dtvcc_pos_y =  m_ccwindow[g_cc_windows_cur_win].pos_y + m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_row;
                dtvcc_pos_x = m_ccwindow[g_cc_windows_cur_win].pos_x;
            }
            else // top-to-bottom or bottom-to-top
            {
                dtvcc_pos_x = m_ccwindow[g_cc_windows_cur_win].pos_x + m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_column;
                dtvcc_pos_y = m_ccwindow[g_cc_windows_cur_win].pos_y;
            }
        }
        else
        {
            dtvcc_pos_y = m_ccwindow[g_cc_windows_cur_win].pos_y + m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_row;
            dtvcc_pos_x = m_ccwindow[g_cc_windows_cur_win].pos_x + m_ccwindow[g_cc_windows_cur_win].cset_pen_loc.pen_column;
        }
        //libc_printf("CC_CMD_SetPenLoc posx=%x posy=%x\n",pos_x,pos_y);
        return TRUE;
    }


    cc_set_loc_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_Delay
*
*  Description:  Delay service data interpretation
*
*  Input: UINT8 Param: tenths of sencods
****************************************************************************/

BOOL cc_cmd_delay(UINT8 param)
{
    if(1==cc_delay_i)
    {
        CMD_DELAY=TRUE;
        g_cc_delay=(param & 0xff)*80 ;  //0~25.5sec gCC_Delay(ms)
        dtvcc_timestamp = osal_get_tick();

        cc_printf("param=%xCC_CMD_Delay%x ,%d \n",param,dtvcc_timestamp,g_cc_delay);
        if(!delayin_delay)
        //wr_dtvcc = rd_dtvcc=cnt_dtvcc=0;
        {
            cnt_dtvcc=0;
            rd_dtvcc=cnt_dtvcc;
            wr_dtvcc = rd_dtvcc;
        }
        cc_delay_i=0;
        return TRUE;
    }
    cc_delay_i++;
    return FALSE;
}
/****************************************************************************
*  Function: CC_CMD_Reset
*
*  Description:  Deleted All window and display screen for resets the caption channel service
*
****************************************************************************/
void cc_cmd_reset(void)
{
    //BOOL ret = TRUE;

    xsoc_printf("CC_CMD_Reset\n");
    cc_fill_char(0,0,36,15,0xff);
    cc_cmd_del_windows(0xFF);
    cc_cmd_del_windows(0xFF);
    defined_windows=FALSE;
    cc_block_start=FALSE;
}
/****************************************************************************
*  Function: cc_coding_layer_parsing
*
*  Description:  interpretation DTVCC command sets (CEA708 page 60)
*   and DTVCC Code Set (CEA708 page29)
*
****************************************************************************/
static UINT8 dtvcc_c0_code_set(UINT8 *data,UINT8 *skip,UINT8 *ext_flag)
{
    // C0 Code Set
    switch(*data)
    {
        case 0x08:
            miscellaneous_command_dtvcc(BACKSPACE);
            cc_printf("BACKSPACE (BS)\n");
        break;
        case 0x0d:
            miscellaneous_command_dtvcc(CARRIAGE_RETURN);
            cc_printf("Carriage Return (CR)\n");
        break;
        case 0x0e:
            miscellaneous_command_dtvcc(DELETE_TO_END_OF_ROW);
            dtvcc_pos_x=m_ccwindow[g_cc_windows_cur_win].pos_x;
            //pos_y=m_CCWindow[gCC_WindowsCurWin].pos_y;
            cc_printf("Horizontal Carriage Return (HCR)%x\n",*data);
        break;
        case 0x0c:
            cc_fill_char((UINT16)0,(UINT16)0,(UINT16)36,(UINT16)15,8);
            dtvcc_pos_x=m_ccwindow[g_cc_windows_cur_win].pos_x;
            dtvcc_pos_y=m_ccwindow[g_cc_windows_cur_win].pos_y;
            cc_printf("Formfeed (FF)\n");
        break;
        case 0x03:
            miscellaneous_command_dtvcc(END_OF_CAPTION);
            cc_printf("End of Text (ETX)\n");
        break;
        case 0x10:
            *ext_flag = TRUE;
            cc_printf("Extended Code (EXT1)\n");
        break;
        default:
            // Unused Codes in Code Set C0
            if((*data <= CC_NO_USEFUL_MAX_DATA))
            {
                return 0;
            }
            else if(( *data >= CC_CTRL_MIN_DATA_SKIP_1 ) && (*data <= CC_CTRL_MAX_DATA_SKIP_1))
            {
                *skip=1;
            }
            else if(( *data >= CC_CTRL_MIN_DATA_SKIP_2 ) && (*data <= CC_CTRL_MAX_DATA_SKIP_2))
            {
                *skip=2;
            }
        break;
    }
    return 1;
}

static UINT8 dtvcc_c1_code_set(UINT8 *data,UINT16 *cmd_data,BOOL *b_cmd_finish)
{
    if(ccg_disable_process_cc)
    {
        cc_cmd_reset();
        return 0;
    }
    // C1 Code Set
    if((((*data >= 0x80) &&(*data <= 0x87)) &&(TRUE== *b_cmd_finish )) ||
        ((0x0001==*cmd_data) && ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0001;
        cc_printf("CurrentW %x W=%x\n", *data, *data&0x07);
        cc_cmd_current_windows(*data);
        *b_cmd_finish=TRUE;
    }
    else if((((*data >= 0x98) &&(*data <= 0x9f)) &&(TRUE== *b_cmd_finish )) ||
        ((*cmd_data==0x0002) && ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0002;
        cc_printf("DefineW %x W=%x\n",*data, *data&0x07);
        *b_cmd_finish=cc_cmd_define_windows(*data);
    }
    else if(((0x88 == *data)&&(TRUE== *b_cmd_finish )) ||  ((0x0004==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0004;
        cc_printf("ClearW %x\n",*data);
        *b_cmd_finish=cc_cmd_clear_windows(*data);
    }
    else if(((0x89== *data)&&(TRUE== *b_cmd_finish )) || ((0x0008==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0008;
        cc_printf("DisplayW %x\n",*data);
        *b_cmd_finish=cc_cmd_display_windows(*data);
    }
    else if(((0x8A== *data)&&(TRUE== *b_cmd_finish )) ||( (0x0010==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0010;
        cc_printf("HideW %x\n",*data);
        *b_cmd_finish=cc_cmd_hide_windows(*data);
    }
    else if(((0x8B== *data)&&(TRUE== *b_cmd_finish )) || ( (0x0020==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0020;
        cc_printf("ToggleW %x\n",*data);
        *b_cmd_finish=cc_cmd_tog_windows(*data);
    }
    else if(((0x8C == *data)&&(TRUE== *b_cmd_finish )) || ( (0x0040==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0040;
        cc_printf("DeleteW %x\n",*data);
        *b_cmd_finish=cc_cmd_del_windows(*data);
    }
    else if(((0x8D== *data)&&(TRUE== *b_cmd_finish )) ||  ((0x0080==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0080;
        cc_printf("Delay data = %x\n",*data);
        *b_cmd_finish=cc_cmd_delay(*data);
    }
    else if(((0x8E== *data)&&(TRUE== *b_cmd_finish )) ||((0x0100==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0100;
        cc_printf("Delay Cancel\n");
        //Display_Popup_Window(gCC_WindowsCurWin,0,15);
        if(m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)
        {
            display_popup_window(g_cc_windows_cur_win,dtvcc_pos_y , dtvcc_pos_y);
        }
        g_cc_delay = 0;
        dtvcc_timestamp=0;
        *b_cmd_finish=TRUE;
    }
    else if(((0x8F== *data)&&(TRUE== *b_cmd_finish )) || ( (0x0200==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0200;
        cc_printf("Reset\n");
        cc_cmd_reset();
        *b_cmd_finish=TRUE;
    }
    else if(((0x90== *data)&&(TRUE== *b_cmd_finish )) ||  ((0x0400==*cmd_data)&& (*b_cmd_finish == FALSE)))
    {
        *cmd_data=0x0400;
        cc_printf("SetPenA %x\n", *data);
        *b_cmd_finish=cc_cmd_set_pen_atr(*data);

    }
    else if(((0x91== *data)&&(TRUE== *b_cmd_finish )) ||  ((0x0800==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x0800;
        cc_printf("SetPenC %x\n", *data);
        *b_cmd_finish=cc_cmd_set_pen_color(*data);

    }
    else if(((0x92== *data)&&(TRUE== *b_cmd_finish )) ||  ((0x1000==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x1000;
        cc_printf("SetPenL %x\n", *data);
        *b_cmd_finish=cc_cmd_set_pen_loc(*data);
       
    }
    else if(((0x97== *data)&&(TRUE== *b_cmd_finish )) ||  ((0x2000==*cmd_data)&& ( FALSE== *b_cmd_finish)))
    {
        *cmd_data=0x2000;
        cc_printf("SetWinA %x\n", *data);
        *b_cmd_finish=cc_cmd_set_win_attr(*data);
    }
    else
    {
        *cmd_data=0x0000;
        cc_printf("No Command\n");
        return 0;
    }
    if(TRUE== *b_cmd_finish )
    {
        *cmd_data=0x0000;
    }
    return 1;
}

static UINT8 dtvcc_string_set(UINT8 *data,UINT16 *cmd_data,UINT8 *ext_flag)
{
    if(ccg_disable_process_cc || (defined_windows==FALSE))//hbchen
    {
        cc_cmd_reset(); //20060831 xing
        return 0;
    }
    *cmd_data=0x0000;
    if(1 == m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)
    {
        if(first_defined)
        {
            if(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color!=CLOSE_CAPTION_NORMAL_8)
            {
            cc_fill_char((UINT8)m_ccwindow[g_cc_windows_cur_win].pos_x,(UINT8)m_ccwindow[g_cc_windows_cur_win].pos_y,\
            (UINT8)(m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont+1),\
            (UINT8)(m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_row_cont+1),
            m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color);
            }
            first_defined=FALSE;
        }
    }
    if( ((CLOSE_CAPTION_NORMAL_1==m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_display_effect)||\
        (CLOSE_CAPTION_NORMAL_2 == m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_display_effect))&&\
        ((CLOSE_CAPTION_NORMAL_1 == m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)&&\
        ((m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify!= CLOSE_CAPTION_NORMAL_1)&&\
        (m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify!= CLOSE_CAPTION_NORMAL_2))))
    {
        if(dtvcc_pos_x>(m_ccwindow[g_cc_windows_cur_win].pos_x +
            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont))
        {
            if((1==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_lock) ||
                (0==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_lock))
            {
                miscellaneous_command_dtvcc(CARRIAGE_RETURN);
            }
            else
            {
                dtvcc_pos_x = m_ccwindow[g_cc_windows_cur_win].pos_x +
                    m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont;
            }
        }
        dtvcc_pos_x = (dtvcc_pos_x>= 35)?35:dtvcc_pos_x;
           xsoc_printf("11fg=%d,bg=%d\n",m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color,
            m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color);
        if(((*data>=0x20 && *data <=0x7f) || (*data>=0xA0))&&(*ext_flag==FALSE))
        {
            cc_draw_char1(dtvcc_pos_x, dtvcc_pos_y,m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color, \
                    (m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color==0x8||
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color>16)?
                    (UINT8) m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color:
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color,\
                    0,0,m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline ,\
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics,*data&0x00ff);
            dtvcc_pos_x++;
        }
        else if(((*data>=0x20 && *data <=0x7f) || (*data>=0xA0 ))&&(*ext_flag==TRUE))
        {
            cc_draw_char1(dtvcc_pos_x, dtvcc_pos_y,m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color, \
                 (m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color==0x8||
                 m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color>16)?
                 (UINT8) m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color:
                 m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color,\
                 0,0,m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline , \
                 m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics,(*data&0x00ff)|0x1000);
             dtvcc_pos_x++;
        }
    }
    else
    {
        if(CLOSE_CAPTION_NORMAL_0== pop_on_valid_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y])
        {
            pop_on_valid_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y] = 0x01;
            pop_on_startx_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y] = dtvcc_pos_x;
            xsoc_printf("pop_on_valid_dtvcc[%x][%x]=%x \n",g_cc_windows_cur_win,dtvcc_pos_y,
                pop_on_valid_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y]);
        }

        if(dtvcc_pos_x>(m_ccwindow[g_cc_windows_cur_win].pos_x +
            m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont))
        {
            if((1==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_lock) ||
                (0==m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_lock))
            {
                miscellaneous_command_dtvcc(CARRIAGE_RETURN);
            }
            else
            {
                dtvcc_pos_x = m_ccwindow[g_cc_windows_cur_win].pos_x +
                    m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windows_column_cont;
            }
        }
        dtvcc_pos_x = (dtvcc_pos_x>=35)?35:dtvcc_pos_x;
        if(((*data>=0x20 && *data <=0x7f)||(*data>=0xA0 ))&&(*ext_flag==FALSE))
        {
            if((1 == m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible) &&\
                (0  == m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify))// Window be displayed
            {
                cc_draw_char1(dtvcc_pos_x, dtvcc_pos_y,m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color, \
                        (m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color==0x8||
                        m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color>16)?
                        (UINT8) m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color:
                        m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color,\
                        0,0,m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline ,\
                        m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics,*data&0x00ff);
            }
            if(fgflashing||bgflashing)
            {
                dtvcc_flash_data[dtvcc_flash_idx-1].string[dtvcc_flash_data_idx]=(*data)&0x00ff;
                dtvcc_flash_data[dtvcc_flash_idx-1].y=dtvcc_pos_y;
                dtvcc_flash_data[dtvcc_flash_idx-1].fg_color=m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color;
                dtvcc_flash_data[dtvcc_flash_idx-1].bg_color=m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color;
                dtvcc_flash_data[dtvcc_flash_idx-1].italics=
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics;
                dtvcc_flash_data[dtvcc_flash_idx-1].underline=
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline;
                dtvcc_flash_data_idx++;
                dtvcc_flash_data[dtvcc_flash_idx-1].end_x=dtvcc_flash_data_idx;
            }
            pop_on_string_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x] =  (*data)&0x00ff;
            pop_on_string_dtvcc_fg[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color;
            pop_on_string_dtvcc_bg[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
                m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color;
            pop_on_string_dtvcc_ud[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
                m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline;
            pop_on_string_dtvcc_itl[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
                m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics;
            switch(m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_print_dir)
            {
                case 0://L->R
                    dtvcc_pos_x++;
                    break;
                case 1://R->L
                    dtvcc_pos_x--;
                    break;
                case 2://T->B
                    dtvcc_pos_y++;
                    break;
                case 3://B->T
                    dtvcc_pos_y--;
                    break;
                default://L->R
                    dtvcc_pos_x++;
                    break;
            }
        }
        else if(((*data>=0x20 && *data <=0x7f)||(*data>=0xA0 ))&&(*ext_flag==TRUE))
        {
            if((1 == m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible)&&\
             (0  == m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify))// Window be displayed
            {
             cc_draw_char1(dtvcc_pos_x, dtvcc_pos_y,m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color, \
                     (m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color==0x8||
                     m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color>16)?
                     (UINT8) m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_att_fill_color:
                     m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color,\
                     0,0,m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline , \
                     m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics,(*data&0x00ff)|0x1000);
            }
            if(fgflashing||bgflashing)
            {
                dtvcc_flash_data[dtvcc_flash_idx-1].string[dtvcc_flash_data_idx]=((*data&0x00ff)|0x1000);
                dtvcc_flash_data[dtvcc_flash_idx-1].y=dtvcc_pos_y;
                dtvcc_flash_data[dtvcc_flash_idx-1].fg_color=m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color;
                dtvcc_flash_data[dtvcc_flash_idx-1].bg_color=m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color;
                dtvcc_flash_data[dtvcc_flash_idx-1].italics=
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics;
                dtvcc_flash_data[dtvcc_flash_idx-1].underline=
                    m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline;
                dtvcc_flash_data_idx++;
                dtvcc_flash_data[dtvcc_flash_idx-1].end_x=dtvcc_flash_data_idx;
            }
            pop_on_string_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x] = ((*data&0x00ff)|0x1000);
            pop_on_string_dtvcc_fg[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
             m_ccwindow[g_cc_windows_cur_win].cset_pen_color.fg_color;
            pop_on_string_dtvcc_bg[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
             m_ccwindow[g_cc_windows_cur_win].cset_pen_color.bg_color;
            pop_on_string_dtvcc_ud[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
              m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_underline;
            pop_on_string_dtvcc_itl[g_cc_windows_cur_win][dtvcc_pos_y][dtvcc_pos_x]=
              m_ccwindow[g_cc_windows_cur_win].cset_pen_att.cc_win_pen_att_italics;
            dtvcc_pos_x++;
        }
        pop_on_endx_dtvcc[g_cc_windows_cur_win][dtvcc_pos_y] = dtvcc_pos_x;

        if((m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_display_effect != CLOSE_CAPTION_NORMAL_0)&&\
            ( CLOSE_CAPTION_NORMAL_1== m_ccwindow[g_cc_windows_cur_win].cdef_win.cc_windowsvisible) &&\
            ( CLOSE_CAPTION_NORMAL_1== m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify||\
             CLOSE_CAPTION_NORMAL_2== m_ccwindow[g_cc_windows_cur_win].cset_win_att.cc_windows_justify))
        {
            display_popup_window(g_cc_windows_cur_win,dtvcc_pos_y , dtvcc_pos_y);
        }
    }
    *ext_flag = FALSE;
    return 1;
}


static void dtvcc_set_parameter(void)
{
     g_cc_delay = 0;
     dtvcc_timestamp=0;
     CMD_DELAY=FALSE;
}

void cc_coding_layer_parsing(UINT8 data)
{
    UINT8 ret = 0;

#ifdef DELAYSERVICE
    UINT8 data2=0;
    BOOL delayflag=FALSE;

    //soc_printf("data=%x,cmd_data=%x,extflag=%d,bCommandFinish=%d\n",data,cmd_data,ExtFlag,bCommandFinish);
    xsoc_printf("Parsing data=%x,Extflag=%d,bCommandFinish=%d,VLCodeFlag=%d,Skip=%d\n",
    data,cc_ext_flag,b_command_finish,cc_vl_code_flag,cc_skip);
delayloop:
    if(g_cc_delay > 0)
    {
        if( (0x8e==data) && ( FALSE== cc_ext_flag) )
        {
            dtvcc_set_parameter();
            //libc_printf("======Delay cancel ========\n");
        }
        else if( 0x8f==data && ( FALSE== cc_ext_flag) )
        {
            dtvcc_set_parameter();
            //wr_dtvcc = rd_dtvcc=cnt_dtvcc=gCC_Delay=0;
            g_cc_delay=0;
            cnt_dtvcc=g_cc_delay;
            rd_dtvcc=cnt_dtvcc;
            wr_dtvcc = rd_dtvcc;
            //xsoc_printf("======Delay Reset========\n");
        }
        else if(push_service_data(data)>=DEPTH_SERVICE_BUFF)
        {
            dtvcc_set_parameter();
            //libc_printf("<push_service_data(data)>127>\n");
        }

        if(g_cc_delay > (osal_get_tick()-  dtvcc_timestamp))
        {
            //libc_printf("(%d > %d-%d)\n",gCC_Delay,osal_get_tick(),timestamp);
            return;
        }
        else
        {
            dtvcc_set_parameter();
        }

    }
    else //if( 0== g_cc_delay)
    {
        CMD_DELAY=FALSE;
    }
    if(pop_service_data(&data2)>1)
    {
        //soc_printf("normal\n");
        data=data2;
        delayflag=TRUE;
        delayin_delay=TRUE;
    }
    else if (delayflag==TRUE)
    {
        data=data2;
        //soc_printf("end %x\n",data);
        delayflag=FALSE;  
        delayin_delay=FALSE;
    }
#endif
    if(cc_skip >0)
    {
        cc_skip--;
        return;
    }

    if( (data<=0x1f) && ( b_command_finish==TRUE) && ( FALSE== cc_ext_flag) &&
        ( FALSE== cc_vl_code_flag))//miscellaneous control code
    {
        ret = dtvcc_c0_code_set(&data,&cc_skip,&cc_ext_flag);
        if(0 == ret)
        {
            return;
        }
    }
    else if(((/*data>=0x00&&*/data<=0x1f)||(data>=0x80&&data<=0x9f)) && ( b_command_finish == TRUE) &&
        (cc_ext_flag == TRUE) && (cc_vl_code_flag == FALSE))
    {
        // Unused Codes in Code Set C2 & C3
        if((data>=0x08)&&(data<=0x0f))
        {
            cc_skip = 1;
        }
        else if((data>=0x10)&&(data<=0x17))
        {
            cc_skip = 2;
        }
        else if((data>=0x18)&&(data<=0x1f))
        {
            cc_skip = 3;
        }
        else if((data>=0x80)&&(data<=0x87))
        {
            cc_skip = 4;
        }
        else if((data>=0x87)&&(data<=0x8f))
        {
            cc_skip = 5;
        }
        else if((data>=0x90)&&(data<=0x9f))
        {
            cc_vl_code_flag = TRUE;
        }

        cc_printf("C2 & C3 Code Skip = %d, VLCodeFlag = %d\n", cc_skip, cc_vl_code_flag);
        cc_ext_flag = FALSE;
    }
    else if(( TRUE == b_command_finish) && ( FALSE== cc_ext_flag) && ( TRUE== cc_vl_code_flag))
    {
        // Unused Codes in Code Set C3 0x90~0x9f
        #if 0
        if( 0x00==(data&0xc0 >> CLOSE_CAPTION_NORMAL_6) )
            cc_printf("Beginning of Command (BOC)\n");
        else if( 0x01==(data&0xc0 >> CLOSE_CAPTION_NORMAL_6) )
            cc_printf("Continuation of Command (COC)\n");
        else if(0x11 == (data&0xc0 >> CLOSE_CAPTION_NORMAL_6))
            cc_printf("End of Command (EOC)\n");
        #endif
        cc_skip = data & 0x3f;
        cc_vl_code_flag = FALSE;
    }
    else if(((data>=0x80&&data<=0x9f) ||( FALSE==b_command_finish)) && (cc_ext_flag == FALSE))//control data
    {
        ret = dtvcc_c1_code_set(&data,&dtvcc_cmd_data,&b_command_finish);
        if(0 == ret)
        {
            return;
        }
    }
    else//string data
    {
        ret = dtvcc_string_set(&data,&dtvcc_cmd_data,&cc_ext_flag);
        if(0 == ret)
        {
            return;
        }
    }
#ifdef DELAYSERVICE
    if(delayflag)
    {
        goto delayloop;
    }
#endif
}
/****************************************************************************
*  Function: CC_Set_Pen_Style_ID
*
*  Description: Predefined Pen Style according Pen Style ID
*
****************************************************************************/
void cc_set_pen_style_id (UINT8 windows_id, UINT8 param)
{
    if (0 == param)
    {
        param = 1;
    }

    m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_pen_size = 1; // STNDR
    switch(param)
    {
        case 1:
            m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_font_style = 0;
        break;
        case 2:
            m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_font_style = 1;
        break;
        case 3:
            m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_font_style = 2;
        break;
        case 4:
        case 6:
            m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_font_style = 3;
        break;
        default:
            m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_font_style = 4;
        break;
     }

    m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_offset = 1; // NORMAL
    m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_italics = 0; // NO
    m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_underline = 0; // NO

    if ((CLOSE_CAPTION_NORMAL_6 == param) || (CLOSE_CAPTION_NORMAL_7 == param))
    {
        m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_edge_type = 3; // UNIFRM
    }
    else
    {
        m_ccwindow[windows_id].cset_pen_att.cc_win_pen_att_edge_type = 1; // NONE
    }

    m_ccwindow[windows_id].cset_pen_color.fg_color = 0;//TransRGBtoIndex(2,2,2); // White
    m_ccwindow[windows_id].cset_pen_color.fg_opacity = 0; // SOLID

    if ((CLOSE_CAPTION_NORMAL_1 == param) || (CLOSE_CAPTION_NORMAL_2 == param) ||
        (CLOSE_CAPTION_NORMAL_3 == param) || (CLOSE_CAPTION_NORMAL_4 == param) ||
        (CLOSE_CAPTION_NORMAL_5 == param))
    {
        m_ccwindow[windows_id].cset_pen_color.bg_color = 7;//TransRGBtoIndex(0,0,0); // Black
    }
    else
    {
        m_ccwindow[windows_id].cset_pen_color.bg_color = 8;
    }

    if ((CLOSE_CAPTION_NORMAL_6 == param) || (CLOSE_CAPTION_NORMAL_7 == param))
    {
        m_ccwindow[windows_id].cset_pen_color.bg_opacity = 3; // TRANSPARENT
    }
    else
    {
        m_ccwindow[windows_id].cset_pen_color.bg_opacity = 0; // SOLID
    }

    if ((CLOSE_CAPTION_NORMAL_6 == param) || (CLOSE_CAPTION_NORMAL_7 == param))
    {
        m_ccwindow[windows_id].cset_pen_color.ed_color = 7;//TransRGBtoIndex(0,0,0); // Black
    }
    else
    {
        m_ccwindow[windows_id].cset_pen_color.ed_color = 8;
    }
}
/****************************************************************************
*  Function: CC_Set_Window_Style_ID
*
*  Description: Predefined window according Style ID
*
****************************************************************************/
void cc_set_window_style_id (UINT8 windows_id, UINT8 param)
{
    if (CLOSE_CAPTION_NORMAL_0 == param)
    {
        param = 1;
    }
    if ((CLOSE_CAPTION_NORMAL_3 == param) || (CLOSE_CAPTION_NORMAL_6 == param))
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_justify = 2; // CENTER
    }
    else
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_justify = 0; // LEFT
    }
    if (CLOSE_CAPTION_NORMAL_7 == param)
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_print_dir = 2; // TOP-TO-BOTTOM
        m_ccwindow[windows_id].cset_win_att.cc_windows_scroll_dir = 1; // RIGHT-TO-LEFT
    }
    else
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_print_dir = 0; // LEFT-TO-RIGHT
        m_ccwindow[windows_id].cset_win_att.cc_windows_scroll_dir = 3; // BOTTOM-TO-TOP
    }
    if ((CLOSE_CAPTION_NORMAL_4 == param) || (CLOSE_CAPTION_NORMAL_5 == param) || (CLOSE_CAPTION_NORMAL_6 == param))
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_word_wrap =0; // YES
    }
    else
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_word_wrap =0; // NO
    }

    m_ccwindow[windows_id].cset_win_att.cc_windows_display_effect = 0; // SNAP

    if ((CLOSE_CAPTION_NORMAL_1 == param) || (CLOSE_CAPTION_NORMAL_3 == param) ||
        (CLOSE_CAPTION_NORMAL_4 == param) || (CLOSE_CAPTION_NORMAL_6 == param) ||
        (CLOSE_CAPTION_NORMAL_7 == param))
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_att_fill_color = 7;//TransRGBtoIndex(0,0,0); // Black
    }
    else
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_att_fill_color = 8;
    }

    if ((CLOSE_CAPTION_NORMAL_2 == param) || (CLOSE_CAPTION_NORMAL_5 == param))
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_att_fill_opacity = 3; // TRANSPARENT
    }
    else
    {
        m_ccwindow[windows_id].cset_win_att.cc_windows_att_fill_opacity = 0; // SOLID
    }
    m_ccwindow[windows_id].cset_win_att.cc_windows_att_border_type = 0; // NONE
}
/****************************************************************************
*  Function: Dtvcc_init
*
*  Description: init all window parameters
*
****************************************************************************/
void dtvcc_init(void)
{
    UINT8 i = 0;

    xsoc_printf("DTVCCINIT\n");
     for( i =0 ; i< 8 ; i++)
    {
        m_ccwindow[i].cc_windows_hide_win =TRUE;
        m_ccwindow[i].cc_windows_display_win = FALSE;
        m_ccwindow[i].cdef_win.cc_windowsvisible = 0;
        m_ccwindow[i].cdef_win.cc_windows_id=0xf;
        m_ccwindow[i].cset_pen_color.fg_color=8;
        m_ccwindow[i].cset_pen_color.bg_color=8;
        m_ccwindow[i].cset_win_att.cc_windows_scroll_dir=3;
        m_ccwindow[i].cset_win_att.cc_windows_print_dir=3;
        m_ccwindow[i].cset_win_att.cc_windows_att_fill_color=8;
    }
}

#endif
