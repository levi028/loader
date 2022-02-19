/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_quickscan.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/nim/nim.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
#include <api/libdb/db_interface.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_com_menu_define.h"
#include "win_quickscan.h"
#include "win_search.h"
#include "win_countryband.h"
#include "platform/board.h"
#include "win_signal.h"
#include "control.h"

#ifdef ANTENNA_INSTALL_BEEP
#include "beep_mp2.h"
#endif


#define VACT_POP_UP (VACT_PASS + 1)

#ifdef SD_UI
#define TP_UPDATE_TIMER_NAME    "TPCHANGE"
#define TP_UPDATE_TIMER_INTERVAL    600
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER g_win_quickscan;

static VACTION quicksrch_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT quicksrch_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION quicksrch_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT quicksrch_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION quicksrch_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT quicksrch_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION quicksrch_item_edit_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT quicksrch_item_edit_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static void win_qs_set_search_param(void);

#ifdef SD_UI
static void send_tp_turn_msg_by_timer();
#endif
static BOOL  check_is_valid_freq();

#define TXT_BUTTON_IDX   WSTL_BUTTON_02_HD//WSTL_NL_BUTTON_01_HD

#define DVBC_QUICKSCAN_FREQ  0
#define DVBC_QUICKSCAN_SYMBOL    1
#define DVBC_QUICKSCAN_EDIT_LEN  0x06 // add .xx
#define MAX_DVBC_FREQUENCY 87000
#define MIN_DVBC_FREQUENCY 4700
#define MIN_FREQUENCY 4825
#define MAX_FREQUENCY 85875
#define MIN_SYMBOL 1000
#define MAX_SYMBOL 7000
#define QUALITY_LEVEL_ONE 20
#define QUALITY_LEVEL_TWO 40
#define QUALITY_LEVEL_THREE 55
#define QUALITY_LEVEL_FOUR 70

static char win_quicksrch_pat_freq[] = "f32";
//integer width is 5,  fraction width is 2
static char win_quicksrch_pat_sym[] = "s5";
static UINT16 quickscan_edit_str[5][30]={{0,},};
static ID set_frontend = OSAL_INVALID_ID;
static UINT16 scan_model_type_ids[] =
{
    RS_CONSTELLATION_16,
    RS_CONSTELLATION_32,
    RS_CONSTELLATION_64,
    RS_CONSTELLATION_128,
    RS_CONSTELLATION_256,
};

static UINT16 yesno_ids_local[] =
{
    RS_COMMON_NO,
    RS_COMMON_YES,
};

/****/
#define LDEF_CON(root, var_con,nxt_obj,\
    ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    quicksrch_item_keymap,quicksrch_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_TXT_BUTTON(root,var_txt,nxt_obj,ID,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, \
    TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    quicksrch_item_sel_keymap,quicksrch_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, \
    ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, \
    TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    quicksrch_item_edit_keymap,quicksrch_item_edit_callback, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)


#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,\
    var_txt,var_txtset,ID,idu,idd,l,t,w,h,\
    name_id,setstr,pat,sub)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_txtset,\
    l + TXTN_L_OF,t + TXTN_T_OF,\
    TXTN_W,TXTN_H,name_id)    \
    LDEF_EDIT(&var_con,var_txtset,NULL,ID,\
    l + TXTS_L_OF, t + TXTS_T_OF,\
    TXTS_W,TXTS_H,NORMAL_EDIT_MODE, \
    CURSOR_NORMAL, pat,NULL,sub, setstr)


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,\
    var_txt,var_num,ID,idu,idd,l,t,w,h,\
    res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,\
    ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,\
    l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,\
    TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL,ID,\
     l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,\
     TXTS_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT_BUTTON(&var_con,var_txt,NULL,\
    ID,l + TXTN_L_OF,t + TXTN_T_OF,\
    TXTN_W-50,TXTN_H,res_id)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    quicksrch_keymap,quicksrch_callback,  \
    nxt_obj, focus_id,0)


#define WIN g_win_quickscan

#define SCAN_MODE_ID   1
#define CH_NO_ID        2
#define FREQ_ID         3
#define SYM_ID          4
#define QAM_ID          5
#define NIT_ID          6
#define START_ID        7

static POBJECT_HEAD quickscan_items[] =
{
    (POBJECT_HEAD)&quicksrch_item1,
    (POBJECT_HEAD)&quicksrch_item2,
    (POBJECT_HEAD)&quicksrch_item3,
    (POBJECT_HEAD)&quicksrch_item4,
    (POBJECT_HEAD)&quicksrch_item5,
    (POBJECT_HEAD)&quicksrch_item6,
    (POBJECT_HEAD)&quicksrch_item7,
};

#define SCAN_MODE_CNT       (quicksrch_sel1.n_count)
#define SCAN_MODE_IDX       (quicksrch_sel1.n_sel)



/* Should have the same order as stream_iso_639lang_abbr*/
static UINT16 scan_mode_ids[] =
{
    RS_SCAN_MODE_CHL,
    RS_SCAN_MODE_FRE,
};

/*******************************************************************************
*   Component  Object  Define
*******************************************************************************/
LDEF_MENU_ITEM_SEL(WIN, quicksrch_item1,&quicksrch_item2, quicksrch_txt1, \
            quicksrch_sel1, SCAN_MODE_ID, START_ID, CH_NO_ID, CON_L, CON_T + (CON_H + CON_GAP)*0, \
            CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, 0, 2, scan_mode_ids)

LDEF_MENU_ITEM_SEL(WIN, quicksrch_item2,&quicksrch_item3, quicksrch_txt2, quicksrch_sel2, \
            CH_NO_ID, SCAN_MODE_ID, FREQ_ID, CON_L, CON_T + (CON_H + CON_GAP)*1, \
            CON_W, CON_H, RS_INSTALLATION_CHANNEL_NO, STRING_PROC, 0, 3, NULL)

LDEF_MENU_ITEM_EDT(WIN, quicksrch_item3, &quicksrch_item4, quicksrch_txt3, quicksrch_edt3, \
            FREQ_ID, CH_NO_ID, SYM_ID, CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, \
            RS_DISPLAY_FREQUENCY, quickscan_edit_str[0], win_quicksrch_pat_freq, quickscan_edit_str[3])

LDEF_MENU_ITEM_EDT(WIN, quicksrch_item4, &quicksrch_item5, quicksrch_txt4, quicksrch_edt4, \
            SYM_ID, FREQ_ID, QAM_ID, CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,  \
            RS_INFO_SYMBOL, quickscan_edit_str[1], win_quicksrch_pat_sym, quickscan_edit_str[4])

LDEF_MENU_ITEM_SEL(WIN, quicksrch_item5, &quicksrch_item6, quicksrch_txt5, quicksrch_sel5, \
            QAM_ID, SYM_ID, NIT_ID, CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, \
            RS_INSTALLATION_QAM_MODE, STRING_ID, 0, 5, scan_model_type_ids)

LDEF_MENU_ITEM_SEL(WIN, quicksrch_item6, &quicksrch_item7, quicksrch_txt6, quicksrch_sel6,  \
            NIT_ID, QAM_ID, START_ID, CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, \
            RS_INSTALLATION_NIT_SCAN, STRING_ID, 0, 2, yesno_ids_local)

LDEF_MENU_ITEM_TXT(WIN, quicksrch_item7, NULL, quicksrch_txt7, START_ID, NIT_ID, SCAN_MODE_ID, \
                   CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, RS_COMMON_SEARCH)

LDEF_WIN(WIN, &quicksrch_item1, W_L, W_T, W_W, W_H, 1)

//static UINT32 def_freq = 74700;
static UINT32 def_sym = 6875;
static UINT32 def_constellation = QAM64;
static UINT32 ch_no=0;
static UINT32 min_ch_no=0;
static UINT32 max_ch_no=0;

/*******************************************************************************
*   local function definition
*******************************************************************************/
static void get_ch_str(UINT32 ch_no, UINT16 *uni_str)
{
    UINT8 j=0;
    UINT32 freq=0;
    char ch_prefix=0;
    UINT8 ch_show_no=0;
    UINT8 str_buf[10]={0};
    UINT8 tmp_buf[10]={0};
    UINT32 cnt = 0x0;
    UINT32 tmp = 0x1;
    UINT32 __MAYBE_UNUSED__ str_trans = 0;
    UINT8 __MAYBE_UNUSED__ str_ret = 0;
    char ch_str[30]={0};
    int sprintf_ret=0;
    band_param *buf = (band_param *)&u_country_param;

    while((ch_no > buf[j].end_ch_no )&& ( buf[j].end_ch_no != NUM_ZERO))
    {
        j++;
    }
    if(j>MAX_BAND_COUNT-NUM_ONE) //not find
    {
        return;
    }

    freq = buf[j].start_freq + (ch_no- buf[j].start_ch_no)*buf[j].freq_step;
    ch_prefix = buf[j].show_ch_prefix;
    ch_show_no = buf[j].show_ch_no+(ch_no- buf[j].start_ch_no);

    j=2;
    while (j--)  // 2 位小数点
    {
        tmp = tmp * 10;
        cnt++;
    }
    if (tmp > freq)
    {
        com_int2str(str_buf, 0x0, DVBC_QUICKSCAN_EDIT_LEN-1);
    }
    else
    {
        com_int2str(str_buf, freq / tmp, (DVBC_QUICKSCAN_EDIT_LEN-1) < cnt ? 0 : (DVBC_QUICKSCAN_EDIT_LEN-1)-cnt);
        str_ret=com_str_cat(str_buf, (const UINT8*)".");
        com_int2str(tmp_buf, freq % tmp, cnt);
        str_ret=com_str_cat(str_buf, tmp_buf);        
    }

    sprintf_ret=snprintf(ch_str,30, "%c%d(%.6s MHz)",  ch_prefix, ch_show_no, str_buf);
    if(0 == sprintf_ret)
    {
        ali_trace(&sprintf_ret);
    }
    str_trans=com_asc_str2uni((UINT8*)ch_str, uni_str);

}

static UINT32 get_ch_frequency(UINT32 ch_no)
{
    UINT8 j=0;
    UINT32 freq=0;
    band_param *buf = (band_param *)&u_country_param;

    while((ch_no > buf[j].end_ch_no )&& ( buf[j].end_ch_no != NUM_ZERO))
    {
        j++;
    }

    if(j>MAX_BAND_COUNT-NUM_ONE) //not find
    {
        return 0;
    }

    freq = buf[j].start_freq + (ch_no- buf[j].start_ch_no)*buf[j].freq_step;
    return freq;
}

static void win_qs_tuner_frontend(void)
{
    UINT8   dot_pos=0;
    UINT32 freq=0;
    UINT32 symbol=0;
    UINT32 constellation=0;
    union ft_xpond xpond;
    struct nim_device *nim=NULL;

    MEMSET(&xpond, 0, sizeof(union ft_xpond));
    //nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    //add on 2011-11-04 for DVBC tests shoud jude the tuner valid
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, board_get_dvbc_valid_tuner());
    if(NULL != nim)
    {
        if(NUM_ZERO == SCAN_MODE_IDX) //by channel
        {
            freq = get_ch_frequency(ch_no);
        }
        else  // by freq
        {
            //freq
            wincom_mbs_to_i_with_dot(quickscan_edit_str[0], &freq, &dot_pos);
        }

        // SYMBOL
        symbol = wincom_mbs_to_i(quickscan_edit_str[1]);
        // scan mode
        constellation = QAM16 + osd_get_multisel_sel(&quicksrch_sel5);



        //soc_printf("osal_get_tick1 %d:\n", osal_get_tick());

        MEMSET(&xpond, 0, sizeof(xpond));
        xpond.c_info.type = FRONTEND_TYPE_C;
        xpond.c_info.frq = freq;
        xpond.c_info.sym = symbol;
        xpond.c_info.modulation = constellation;
        //frontend_set_nim(nim, NULL, &xpond, 1);
        frontend_tuning(nim, NULL, &xpond, 1);

        //soc_printf("osal_get_tick1 %d:\n", osal_get_tick());

    #if 0
       // nim_channel_change(nim, freq,symbol,constellation);
        if((i_qs_freq != freq) || (i_qs_symbol != symbol) ||\
     (i_qs_mode != constellation))
        {
            i_qs_freq = freq;
            i_qs_symbol = symbol;
            i_qs_mode = constellation;

    //      UIChChgNimcc(freq, symbol, constellation);
        }

        //soc_printf("osal_get_tick2 %d:\n", osal_get_tick());

        osal_task_sleep(300);
        UINT8 lock=0;
        nim_get_lock(nim, &lock);

        //soc_printf("osal_get_tick3 %d:\n", osal_get_tick());

        if(lock)
        {
            def_freq = freq;
            def_sym = symbol;
            def_constellation = constellation;
        }
    #endif
    }
}


//extern void get_default_bandparam(UINT8 index, Band_param *buf);
static void win_qs_load_setting(void)
{

    UINT32 freq=0;
    UINT32 symbol=0;
    UINT32 constellation=0;
    P_NODE p_node;
    T_NODE t_node;
    UINT32 prog_num=0;
    INT8 condition=-1;

    UINT8 cur_chan_mode=0;
    UINT32 channel_number=0;
    UINT32 fi=0;
    BOOL __MAYBE_UNUSED__ flag=FALSE;
    INT32 __MAYBE_UNUSED__ tp_ret=0;
    INT32 __MAYBE_UNUSED__ prog_ret=0;
    SYSTEM_DATA *sys_data=NULL;

    sys_data = sys_data_get();
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    if(sys_data->current_ft_count > NUM_ZERO)
    {
        def_sym = sys_data->current_ft[0].c_param.sym;
        def_constellation = sys_data->current_ft[0].c_param.constellation;
    }

    #ifdef DVBC_MODE_CHANGE
    if(NUM_ZERO == sys_data->dvbc_mode)
    {
        get_default_bandparam(sys_data->country, (band_param *)&u_country_param);
    }
    else
    {
        MEMSET(&u_country_param,0,sizeof(u_country_param));
        get_default_bandparam_qamb(sys_data->country, (band_param *)&u_country_param);
    }
    #else
    get_default_bandparam(sys_data->country, (band_param *)&u_country_param);
    #endif
    //prog_num = get_prog_num(VIEW_ALL | TV_CHAN | RADIO_CHAN , 0);

    cur_chan_mode = sys_data_get_cur_chan_mode();
    prog_num = db_check_prog_exist(VIEW_ALL | cur_chan_mode, 0);//speed UI

    if (NUM_ZERO != prog_num)
    {
        prog_ret=get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node);
        tp_ret=get_tp_by_id(p_node.tp_id,&t_node);


        while((t_node.frq !=NUM_ZERO)&&(u_country_param[fi].start_freq!=NUM_ZERO)&&(fi<MAX_BAND_COUNT))
        {
            if((t_node.frq<=u_country_param[fi].end_freq)&&(t_node.frq>=u_country_param[fi].start_freq))
            {
                channel_number=(u_country_param[fi].start_ch_no);
                if(u_country_param[fi].freq_step!=NUM_ZERO)
                {
                    channel_number+= ((t_node.frq -u_country_param[fi].start_freq)/u_country_param[fi].freq_step);
                }
                break;
            }
            fi++;
        }
    }
    else// if(0 == prog_num)
    {
        ch_no=0;
    }

    if(channel_number != NUM_ZERO)
    {
        ch_no = channel_number;
        freq = t_node.frq;
        symbol = t_node.sym;
        constellation = t_node.fec_inner;
    }
    else
    {
        if(NUM_ZERO == ch_no)
        {
            ch_no=(u_country_param[0].start_ch_no);
            freq = u_country_param[0].start_freq;
        }
        else
        {
            freq = get_ch_frequency( ch_no);
        }
        symbol = def_sym;
        constellation = def_constellation;
    }

    fi=0;
    min_ch_no=u_country_param[0].start_ch_no;
    while((u_country_param[fi].end_ch_no != NUM_ZERO) && fi<MAX_BAND_COUNT)
    {
        /*
        if(uCountryParam[fi].end_ch_no > max_ch_no)
            max_ch_no=uCountryParam[fi].end_ch_no;
        */
        fi++;
    }
    max_ch_no=u_country_param[fi-1].end_ch_no;


    /*ChNo*/


    /* FREQ */
    //wincom_mbs_to_i_with_dot(quickscan_edit_str[0], &freq_def, &dot_pos);
    //if(freq_def==0)
    wincom_i_to_mbs_with_dot(quickscan_edit_str[DVBC_QUICKSCAN_FREQ], freq, DVBC_QUICKSCAN_EDIT_LEN - 1, 0x02);

    flag=osd_set_edit_field_suffix(&quicksrch_edt3, STRING_ANSI, (UINT32)" MHz");
    osd_set_edit_field_style(&quicksrch_edt3,NORMAL_EDIT_MODE | SELECT_STATUS);


    /* SYMBOL */
    flag=osd_set_edit_field_suffix(&quicksrch_edt4, STRING_ANSI, (UINT32)" KBps");
    wincom_i_to_mbs(quickscan_edit_str[DVBC_QUICKSCAN_SYMBOL], symbol, DVBC_QUICKSCAN_EDIT_LEN - 1);
    osd_set_edit_field_style(&quicksrch_edt4,NORMAL_EDIT_MODE | SELECT_STATUS);
    condition=(INT32)constellation - QAM16;
    if(condition<0)
    {
        condition=0;
    }

    /* QAM */
    osd_set_multisel_sel(&quicksrch_sel5, condition);


}


/* switch Scan Mode by Channel or by Frequency */
static void switch_scan_mode_select(BOOL select)
{
    if(select) /* By Frequency */
    {
        if( !osd_check_attr(&quicksrch_item2, C_ATTR_INACTIVE))
        {
            osd_set_attr(&quicksrch_item2, C_ATTR_INACTIVE);
            osd_set_attr(&quicksrch_txt2, C_ATTR_INACTIVE);
            osd_set_attr(&quicksrch_sel2, C_ATTR_INACTIVE);
        }
        if( !osd_check_attr(&quicksrch_item3, C_ATTR_ACTIVE))
        {
            osd_set_attr(&quicksrch_item3, C_ATTR_ACTIVE);
            osd_set_attr(&quicksrch_txt3, C_ATTR_ACTIVE);
            osd_set_attr(&quicksrch_edt3, C_ATTR_ACTIVE);
        }
    }
    else  /* By Channel */
    {
        if( !osd_check_attr(&quicksrch_item2, C_ATTR_ACTIVE))
        {
            osd_set_attr(&quicksrch_item2, C_ATTR_ACTIVE);
            osd_set_attr(&quicksrch_txt2, C_ATTR_ACTIVE);
            osd_set_attr(&quicksrch_sel2, C_ATTR_ACTIVE);
        }
        if( !osd_check_attr(&quicksrch_item3, C_ATTR_INACTIVE))
        {
            osd_set_attr(&quicksrch_item3, C_ATTR_INACTIVE);
            osd_set_attr(&quicksrch_txt3, C_ATTR_INACTIVE);
            osd_set_attr(&quicksrch_edt3, C_ATTR_INACTIVE);
        }
    }
}

static void win_qs_set_mode_string(UINT8 input)
{
    switch_scan_mode_select((0 == input)? 0:1);
    osd_track_object((POBJECT_HEAD)&quicksrch_item1,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&quicksrch_item2,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&quicksrch_item3,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&quicksrch_item4,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&quicksrch_item5,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&quicksrch_item6,C_UPDATE_ALL);
}



/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION quicksrch_item_edit_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION action = VACT_PASS;

    switch (key)
    {
        case V_KEY_LEFT:
            action = VACT_EDIT_LEFT;
            break;
        case V_KEY_RIGHT:
            action = VACT_EDIT_RIGHT;
            break;
        case V_KEY_0:
        case V_KEY_1:
        case V_KEY_2:
        case V_KEY_3:
        case V_KEY_4:
        case V_KEY_5:
        case V_KEY_6:
        case V_KEY_7:
        case V_KEY_8:
        case V_KEY_9:
            action = key - V_KEY_0 + VACT_NUM_0;
            break;
        default:
            break;
    }

    return action;
}
static void chanscan_set_frontend_handler(void)
{
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
}
static PRESULT quicksrch_item_edit_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch(event)
	{
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			//refresh signal
			//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
			api_stop_timer(&set_frontend);
			set_frontend = api_start_timer("SETFE",500,(OSAL_T_TIMER_FUNC_PTR)chanscan_set_frontend_handler);
			break;
	}
	return ret;
}
static VACTION quicksrch_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch (key)
    {
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        case V_KEY_ENTER:
            act = VACT_POP_UP;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;

}


static PRESULT quicksrch_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 __MAYBE_UNUSED__ sel=0;
    UINT8 bid=0;
    UINT16 *uni_str=NULL;
    VACTION unact=0;
    //PRESULT obj_ret = PROC_PASS;
    //#ifndef SD_UI
    //BOOL send_msg_flag=FALSE;
    //#endif
    POBJECT_HEAD item = NULL;

    MEMSET(&item, 0, sizeof(POBJECT_HEAD));
    bid = osd_get_obj_id(pobj);

    switch (event)
    {
        case EVN_PRE_CHANGE:
            sel = *((UINT32*)param1);
            break;

        case EVN_POST_CHANGE:
            if(SCAN_MODE_ID == bid)
            {
                win_qs_load_setting();
                win_qs_set_mode_string(SCAN_MODE_IDX);
                #ifdef SD_UI
                send_tp_turn_msg_by_timer();
				#else
				ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
                #endif
            }
            else if ((QAM_ID == bid))
            {
            #ifndef SD_UI
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
            #else
                send_tp_turn_msg_by_timer();
            #endif
            }
            break;
        case EVN_REQUEST_STRING:
            sel = param1;
            uni_str= (UINT16*)param2;

            if(CH_NO_ID ==bid)
            {
                get_ch_str(ch_no, uni_str);
            }
            break;
        case EVN_UNKNOWN_ACTION:
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            item = quickscan_items[bid - 1];
            if(CH_NO_ID == bid)
            {
                if(VACT_DECREASE == unact)
                {
                    if(ch_no == min_ch_no)
                    {
                        ch_no =max_ch_no;
                    }
                    else
                    {
                        ch_no--;
                    }
                }
                else if(VACT_INCREASE == unact)
                {
                    if(ch_no == max_ch_no)
                    {
                        ch_no = min_ch_no;
                    }
                    else
                    {
                        ch_no++;
                    }
                }
                osd_track_object(item,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                #ifndef SD_UI
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
                #else
                send_tp_turn_msg_by_timer();
                #endif

                ret = PROC_LOOP;
            }
            break;
        default:
            break;
    }
    return ret;
}


static VACTION quicksrch_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch (key)
    {
        case V_KEY_ENTER:
        //case V_KEY_RIGHT:
            act = VACT_ENTER;
            break;
        default:
            act = VACT_PASS;
        break;
    }

    return act;
}

#ifdef ANTENNA_INSTALL_BEEP
static void beep_stop(void)
{
    SYSTEM_DATA* sys_data = NULL;

    sys_data = sys_data_get();
    if(sys_data && sys_data->install_beep)
    {
    #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
        api_audio_stop_tonevoice();
    #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
        api_audio_beep_stop();
    #endif
    }
}
#endif

static PRESULT quicksrch_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{

    PRESULT ret = PROC_PASS;
    UINT8 bid=0;
    VACTION unact=VACT_PASS;
    POBJECT_HEAD submenu = NULL;

    MEMSET(&submenu, 0, sizeof(POBJECT_HEAD));
    bid = osd_get_obj_id(pobj);

    switch (event)
    {
        case EVN_FOCUS_PRE_LOSE:
            if(FREQ_ID == bid)
            {
                if(FALSE == check_is_valid_freq())
                {
                    ret = PROC_LOOP;
                }
            }
            break;

        case EVN_FOCUS_POST_LOSE:
            if ((FREQ_ID == bid) || (SYM_ID == bid))
            {
                #ifndef SD_UI
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
                #else
                send_tp_turn_msg_by_timer();
                #endif
            }

            break;

        case EVN_UNKNOWN_ACTION:
        //case EVN_KEY_GOT:
            unact = (VACTION)(param1 >> 16);
            if ((VACT_ENTER==unact ) &&( START_ID==bid ))
            {
                if( FALSE == check_is_valid_freq() )
                {
                    ret = PROC_LOOP;
                    break;
                }

                win_qs_set_search_param();
                submenu = (POBJECT_HEAD)&g_win2_search;
                if (osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                {
                    menu_stack_push(submenu);
                    #ifdef ANTENNA_INSTALL_BEEP
                    beep_stop();
                    #endif
                }
                ret = PROC_LOOP;
            }
            break;
        default:
            break;
    }

    return ret;

}


static VACTION quicksrch_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

    switch (key)
    {
        case V_KEY_UP:
            act = VACT_CURSOR_UP;
            break;
        case V_KEY_DOWN:
            act = VACT_CURSOR_DOWN;
            break;
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}

static PRESULT quicksrch_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
#ifdef ANTENNA_INSTALL_BEEP
    SYSTEM_DATA* sys_data = NULL;
    static UINT8 tone_voice_init =0;
    UINT8 level =0,lock=0;
    UINT8 __MAYBE_UNUSED__ quality=0 ;
    static UINT32 interval=0;
#endif
    PRESULT ret = PROC_PASS;

#ifdef ANTENNA_INSTALL_BEEP
    sys_data = sys_data_get();
#endif

    switch (event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title ( ( POBJECT_HEAD ) &g_win_quickscan, RS_INSTALLATION_QUICK_SEARCH, 0 );
            win_qs_load_setting();
            osd_set_container_focus((CONTAINER*)pobj,1);
            cur_tuner_idx = board_get_dvbc_valid_tuner();
            break;

        case EVN_POST_OPEN:
#ifdef ANTENNA_INSTALL_BEEP
            tone_voice_init = 0;
#endif
            win_qs_set_mode_string(SCAN_MODE_IDX);

            //win_help_guide_open_ex(pobj,com_guide, 2);
            //win_signal_open(pobj);
            //win_signal_open_ex(pobj,
        //pobj->frame.uLeft+CON_L, pobj->frame.uTop + 350);
        #ifndef SD_UI
            win_signal_open_ex(pobj,CON_L, pobj->frame.u_top + 358);
        #else
            win_signal_open_ex(pobj,CON_L-20, pobj->frame.u_top + 270);
        #endif
        #ifndef SD_UI
            win_qs_tuner_frontend();
        #else
            send_tp_turn_msg_by_timer();
        #endif
            win_signal_refresh();
            //win_signal_update();
            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            #ifdef ANTENNA_INSTALL_BEEP
            beep_stop();
            #endif
            break;
        case EVN_POST_CLOSE:
            //win_help_guide_close();
            win_signal_close();
            break;
        case EVN_MSG_GOT:
            if(CTRL_MSG_SUBTYPE_STATUS_SIGNAL == param1)
            {
                win_signal_refresh();
                //win_signal_update();
#ifdef ANTENNA_INSTALL_BEEP
                if(sys_data->install_beep)
                {
                    level   = (UINT8)(param2>>16);
                    quality = (UINT8)(param2>>8);
                    lock    = (UINT8)(param2>>0);
                #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
                    if(quality<QUALITY_LEVEL_ONE)
                              level = 0;
                    else if(quality<QUALITY_LEVEL_TWO)
                        level = 1;
                    else if(quality<QUALITY_LEVEL_THREE)
                        level = 3;
                    else if(quality <QUALITY_LEVEL_FOUR)
                        level = 5;
                    else
                        level = 7;
                    api_audio_gen_tonevoice(level, tone_voice_init);
                #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
                    if(lock)
                        level = 100;//100/quality;
                    else
                        level = 1;//100/quality;
                    if((!tone_voice_init )||( level!=interval))
                    {
                        if(!tone_voice_init)
                            api_audio_beep_start(beep_bin_array, beep_bin_array_size);
                        interval = level;
                        api_audio_beep_set_interval(interval);
                    }
                #endif
                    tone_voice_init = 1;
                }
#endif
            }
            else if(CTRL_MSG_SUBTYPE_CMD_TP_TURN == param1)
            {
                win_qs_tuner_frontend();
            }
            break;
        default:
            break;
    }

    return ret;
}

static void win_qs_set_search_param(void)
{
    UINT32  search_mode=0;
    UINT32 prog_type=0;
    S_NODE s_node;
    struct as_service_param param;
    struct vdec_device *vdec __MAYBE_UNUSED__=NULL;
    struct vdec_io_get_frm_para vfrm_param;
    UINT32 addr=0;
    UINT32 len=0;
    T_NODE t_node;
    INT32 ret=0;
    UINT8  dot_pos=0;
    UINT32 freq=0;
    UINT32 symbol=0;
    UINT32 constellation=0;
    UINT32 nit_search=0;
    RET_CODE __MAYBE_UNUSED__ vdec_ret=RET_SUCCESS;
    INT32 __MAYBE_UNUSED__ func_ret=0;


    MEMSET(&vfrm_param, 0, sizeof(struct vdec_io_get_frm_para));
    MEMSET(&param, 0, sizeof(struct as_service_param));
    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));

    if(NUM_ZERO==SCAN_MODE_IDX)  //by channel
    {
        freq = get_ch_frequency(ch_no);
    }
    else
    {
        //get the parameters for the search
        //freq
        wincom_mbs_to_i_with_dot(quickscan_edit_str[0], &freq, &dot_pos);

    }
    // SYMBOL
    symbol = wincom_mbs_to_i(quickscan_edit_str[1]);
    // scan mode
    constellation = QAM16 + osd_get_multisel_sel(&quicksrch_sel5);
    //nit search
    nit_search = osd_get_multisel_sel(&quicksrch_sel6);

    //add the TP node
    //recreate_sat_view(VIEW_ALL, 0);
    //add_node(TYPE_SAT_NODE, 0, &s_node);
    s_node.sat_id = 1;
    //update_data();
    t_node.frq = freq;
    t_node.sym = symbol;
    t_node.fec_inner = constellation;
    t_node.sat_id = s_node.sat_id;

    t_node.ft_type = FRONTEND_TYPE_C;

    func_ret=recreate_tp_view( VIEW_SINGLE_SAT, s_node.sat_id);
    ret = lookup_node(TYPE_TP_NODE, &t_node, t_node.sat_id);
    if (ret != SUCCESS)
    {
        func_ret=add_node(TYPE_TP_NODE, t_node.sat_id, &t_node);
        func_ret=update_data();
#ifdef _INVW_JUICE
        osal_delay_ms(500);
        //INVIEW_RefreshServices(FALSE, 1);  //v0.1.4 marked
#endif
    }

    //tv/radio/all
    prog_type = P_SEARCH_TV | P_SEARCH_RADIO;
    search_mode = P_SEARCH_FTA | P_SEARCH_SCRAMBLED;

    param.as_prog_attr = prog_type | search_mode;

    param.as_from = t_node.tp_id;
    param.as_to = 0;
    if (NUM_ZERO == nit_search)
    {
        param.as_method = AS_METHOD_TP;
    }
    else
    {
        param.as_method = AS_METHOD_NIT_TP;
    }

    param.as_sat_cnt = 1;
    param.sat_ids[0] = s_node.sat_id;
    param.as_frontend_type=FRONTEND_TYPE_C;
    //param.ft[0].c_param.sym = symbol;
    //param.ft[0].c_param.constellation = constellation;
    param.ft_count=0;

    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
    param.as_handler = NULL;

    // set param
    win2_search_set_param(&param);

#if 1//def VFB_SUPPORT
    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;
#else
    vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
    vdec_ret=vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
    addr = vfrm_param.t_frm_info.u_y_addr;
    len = (vfrm_param.t_frm_info.u_height * vfrm_param.t_frm_info.u_width * 3) / 2;
#endif
    func_ret=db_search_init((UINT8 *)addr, len);

}

#ifdef SD_UI
static ID tp_update_timer = OSAL_INVALID_ID;
static void tp_update_timer_out(void)
{

    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
}

static void send_tp_turn_msg_by_timer(void)
{
    if(OSAL_INVALID_ID == tp_update_timer)
    {
        tp_update_timer = api_start_timer(TP_UPDATE_TIMER_NAME, \
    TP_UPDATE_TIMER_INTERVAL, (OSAL_T_TIMER_FUNC_PTR)tp_update_timer_out);
    }
    else
    {
        api_stop_timer(&tp_update_timer);
        tp_update_timer = api_start_timer(TP_UPDATE_TIMER_NAME, \
                      TP_UPDATE_TIMER_INTERVAL, (OSAL_T_TIMER_FUNC_PTR)tp_update_timer_out);

    }

}
#endif

static BOOL  check_is_valid_freq(void)
{
    UINT32 freq=0;
    UINT8 dot_pos=0;
    UINT8   back_saved=0;

    win_popup_choice_t __MAYBE_UNUSED__ popup_choice =WIN_POP_CHOICE_YES;

    wincom_mbs_to_i_with_dot(quickscan_edit_str[0], &freq, &dot_pos);
    //libc_printf(" freq = %d ,dot_pos = %d  \n",freq,dot_pos);
    if((freq>MAX_DVBC_FREQUENCY || freq < MIN_DVBC_FREQUENCY) && (NUM_ONE==SCAN_MODE_IDX))
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg("Frequency exceeds the range <47 -- 870>,please set it again!",NULL, 0);
        #ifdef SD_UI
        win_compopup_set_frame(150, 150, 320, 100);
        #else
        win_compopup_set_frame(GET_MID_L(500), 150, 500, 200);
        #endif
        popup_choice=win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();

        return FALSE;
    }

    return TRUE;
}
