/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_list_internal.c
*
*    Description: To realize the UI for user view & edit the channel
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <api/libosd/osd_lib.h>
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "string.id"
#include "copper_common/system_data.h"
#include "win_chan_list.h"
#include "win_chan_list_internal.h"
#include "win_password.h"
#include "win_rename.h"
#include "platform/board.h"
#include "key.h"
#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif

OBJLIST    chlst_ol;
TEXT_FIELD   chlist_infor_txt;
TEXT_FIELD   chlst_preview_txt;
BITMAP       chlist_infor_bmp;
CONTAINER  chlst_btn_con;

UINT8  chan_edit_grop_idx    = 0;    /*  See CHAN_EDIT_GRP_T : index to chan_edit_grps[] */
UINT8  chan_edit_type_btnidx = 0;    /* See CHAN_EDIT_TYPE_T */
UINT16 move_channel_idx      = 0;
BOOL   pwd_valid_status      = FALSE;
UINT32 play_chan_id          = 0;
UINT8  m_sorted_flag         = 0;
UINT32 del_channel_idx[MAX_PROG_NUM/32 + 1] = {0};
#ifdef CAS9_V6
extern void check_last_live_uri();
#endif

static void gen_fft_modulation_str(T_NODE *t_node, char *str, int str_buf_size)
{
    char       *fft           = NULL;
    char       *modulation    = NULL;

    fft = "";
    switch(t_node->FFT)
    {
        case 1:
            fft = "1K";
            break;
        case 2:
            fft = "2K";
            break;
        case 4:
            fft = "4K";
            break;
        case 8:
            fft = "8K";
            break;
        case 16:
            fft = "16K";
            break;
        case 32:
            fft = "32K";
            break;
        default:
            break;
    }

    modulation = "";
    switch(t_node->modulation)
    {
        case 2:
            modulation = "DQPSK";
            break;
        case 4:
            modulation = "QPSK";
            break;
        case 16:
            modulation = "16QAM";
            break;
        case 64:
            modulation = "64QAM";
            break;
        case 65: //(64+1)
            modulation = "256QAM";
            break;
        default:
            break;
    }

    snprintf(str,str_buf_size,"%lu/%s/%s", t_node->frq, fft, modulation);
#ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == osd_get_mirror_flag())
    {
         snprintf(str,str_buf_size,"%s/%s/%d",modulation , fft, t_node->frq);
    }
#endif
}

static void gen_gi_fec_str(T_NODE *t_node, char *str)
{
    char       *gi            = NULL;
    char       *fec           = NULL;

    gi = "";   // gi
    switch(t_node->guard_interval)
    {
        case 4:
            gi = "1/4";
            break;
        case 8:
            gi = "1/8";
            break;
        case 16:
            gi = "1/16";
            break;
        case 32:
            gi = "1/32";
            break;
        case 128:
            gi = "1/128";
            break;
        case 147:
            gi = "19/128";
            break;
        case 19:
            gi = "19/256";
            break;
        default:
            break;
    }

    // fec
    fec = "";
    switch(t_node->fec_inner)
    {
        case 0:
            fec = "1/2";
            break;
        case 1:
            fec = "2/3";
            break;
        case 2:
            fec = "3/4";
            break;
        case 3:
            fec = "5/6";
            break;
        case 4:
            fec = "7/8";
            break;
        case 5:
            fec = "3/5";
            break;
        case 6:
            fec = "4/5";
            break;
        default:
            break;
    }

    snprintf(str, 50, "gi: %s  fec: %s", gi, fec);
}

static void set_tnode_tuner_param(struct nim_device *priv_nim_dev, T_NODE *t_node)
{
    UINT8      nim_fft        = 0;
    UINT8      nim_modulation = 0;
    UINT8      nim_gi         = 0;
    UINT8      nim_fec        = 0;

    if(priv_nim_dev->get_fftmode)
    {
        priv_nim_dev->get_fftmode(priv_nim_dev,&nim_fft);
        t_node->FFT= nim_fft;
    }
    if(priv_nim_dev->get_modulation)
    {
        priv_nim_dev->get_modulation(priv_nim_dev,&nim_modulation);
        t_node->modulation = nim_modulation;
    }
    if(priv_nim_dev->get_guard_interval)
    {
        priv_nim_dev->get_guard_interval(priv_nim_dev,&nim_gi);
        t_node->guard_interval = nim_gi;
    }
    if(priv_nim_dev->get_fec)
    {
        priv_nim_dev->get_fec(priv_nim_dev,&nim_fec);
        t_node->fec_inner = nim_fec;
    }
}

static void win_chlst_switch_channel_del_flag(UINT32 chan_idx)
{
    UINT32  windex    = 0;
    UINT8   bshiftbit = 0;

    windex      = chan_idx / 32;
    bshiftbit   = chan_idx % 32;
    del_channel_idx[windex] ^= (0x00000001<<bshiftbit);
}

void win_chlist_set_btn_display(BOOL update)
{
    win_chlist_set_btn_display2(update, 0xff);
}

void win_chlist_set_btn_display2(BOOL update, UINT8 mode)
{
    UINT32          i            = 0;
    UINT8           btn_colidx   = 0;
    UINT8           btn_index    = 0;
    UINT16          btn_left     = 0;
    UINT16          btn_gap      = 0;
    CONTAINER       *btn_con     = NULL;
    TEXT_FIELD      *btn_txt     = NULL;
    TEXT_FIELD      *btn_txt_one = NULL;
    chan_edit_grp_t *pcheditgrp  = NULL;

    btn_con = &chlst_btn_con;
    // to avoid the OSD flash
    switch(mode)
    {
#ifdef _INVW_JUICE
    case VACT_LOCK:
        btn_index = 0;
        break;
    case VACT_MOVE:
        btn_index = 1;
        break;
    case VACT_ADD:
        btn_index = 1;
        break;
    case VACT_SORT:
        btn_index = 0;
        break;
    case VACT_EDIT:
        btn_index = 1;
        break;
    case VACT_DEL:
        btn_index = 2;
        break;

#else
    case VACT_FAV:
        btn_index = 0;
        break;
    case VACT_LOCK:
        btn_index = 1;
        break;
    case VACT_SKIP:
        btn_index = 2;
        break;
    case VACT_MOVE:
        btn_index = 3;
        break;
    case VACT_ADD:
        btn_index = 1;
        break;
    case VACT_SORT:
        btn_index = 0;
        break;
//    case VACT_EDIT:
//      btn_index = 1;
//        break;
    case VACT_DEL:
        btn_index = 1;
        break;
#endif
    default:
        break;
    }

    pcheditgrp = chan_edit_grps[chan_edit_grop_idx];
    for(i=0;i<(UINT32)(pcheditgrp->btn_cnt - 1);i++)
    {
        btn_txt = btn_txtobj[i];
        osd_set_objp_next(btn_txt, btn_txtobj[i + 1]);
    }
    btn_txt = btn_txtobj[i];
    osd_set_objp_next(btn_txt, NULL);

    if(5 == pcheditgrp->btn_cnt)
    {
        btn_left = CH_5BTN_L;
        btn_gap  = CH_5BTN_GAP;
    }
    else if(4 == pcheditgrp->btn_cnt)
    {
        btn_left = CH_4BTN_L;
        btn_gap  = CH_4BTN_GAP;
    }
    else //if(pcheditgrp->btn_cnt == 3)
    {
        btn_left = CH_3BTN_L;
        btn_gap  = CH_3BTN_GAP;
    }

    btn_txt = (TEXT_FIELD*)osd_get_container_next_obj(btn_con);
    for(i=0;i<pcheditgrp->btn_cnt;i++)
    {
        btn_txt->b_align = C_ALIGN_CENTER;
        if((pcheditgrp->btn_idx[i] == chan_edit_type_btnidx) && (chan_edit_type_btnidx!= CHAN_EDIT_NONE))
        {
            btn_colidx = pcheditgrp->btn_colidx[2*i + 1];
            btn_txt->b_align |= C_ALIGN_TOP;
        }
        else
        {
            btn_colidx = pcheditgrp->btn_colidx[2*i + 0];
            btn_txt->b_align |= C_ALIGN_BOTTOM;
        }
        osd_set_text_field_content(btn_txt, STRING_ID, (UINT32)pcheditgrp->btn_stridx[i]);
        btn_txt->head.frame.u_left = btn_left + i*CH_BTN_W;
        if(i > 0)
        {
            btn_txt->head.frame.u_left += btn_gap * i;
        }
        osd_set_color(btn_txt, btn_colidx, btn_colidx, btn_colidx, btn_colidx);

        if(i == btn_index)
        {
            btn_txt_one = btn_txt;
        }
        btn_txt = (TEXT_FIELD*)osd_get_objp_next(btn_txt);
    }

    if(update)
    {
        if(0xff == mode)    // update all
        {
            osd_draw_object((POBJECT_HEAD)btn_con, C_UPDATE_ALL);
        }
        else
        {
            if(btn_txt_one)
            {
                osd_draw_object((POBJECT_HEAD)btn_txt_one, C_UPDATE_ALL);
            }
        }
    }
}

void vact_enter_proc(UINT16 sel, UINT16 ch_cnt, OBJLIST *ol, COM_POP_LIST_PARAM_T *param, OSD_RECT *rect)
{
    UINT8    i              = 0;
    UINT8    *chan_name     = NULL;
    UINT16   play_chan_pos  = 0;
    UINT32   changed        = 0;
    UINT32   flag           = 0;
    P_NODE   p_node;
    UINT8 favset[MAX_FAVGROUP_NUM] = {0};

    MEMSET(&p_node, 0, sizeof(P_NODE));
    if(CHAN_EDIT_NONE == chan_edit_type_btnidx)
    {
#if(CHAN_NORMAL_PLAY_TYPE == 0)
        #ifdef PREVIEW_SHOW_LOGO
        win_show_logo();
        #else
        win_chlst_play_channel(sel);
        #endif
#endif
    }
    else
    {
        get_prog_at(sel,&p_node);

        switch(chan_edit_type_btnidx)
        {
        case CHAN_EDIT_FAV:
            for(i=0;i<MAX_FAVGROUP_NUM;i++)
            {
                favset[i] = (p_node.fav_group[0]>>i) & 0x01;
            }
            param->id = 0;
            param->cur = 0;
            param->selecttype = POP_LIST_MULTISELECT;
            param->select_array = favset;
            #ifndef SD_UI
            osd_set_rect(rect,615, 277, 320,380); // 450
            #else
            osd_set_rect(rect,CH_5BTN_L, CH_BTN_T + CH_BTN_H, 300,300);
            #endif
            win_com_open_sub_list(POP_LIST_TYPE_FAVSET,rect,param);
            changed = 0;
            for(i=0;i<MAX_FAVGROUP_NUM;i++)
            {
                if(win_comlist_ext_check_item_sel(i))
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }
                if(favset[i] !=  flag)
                {
                    changed = 1;
                    p_node.fav_group[0] &= ~(0x1<<i);
                    p_node.fav_group[0] |= (flag<<i);
                }
            }

            if(changed)
            {
                modify_prog(p_node.prog_id, &p_node);
            }
            break;
        case CHAN_EDIT_LOCK:
            if(!pwd_valid_status)
            {//lock password verified flag,when not verified,pop-up pwd windows
                if(!win_pwd_open(NULL,0))
                {
                    break;
                }
                else
                {
                    pwd_valid_status = TRUE;
                }
            }
            p_node.lock_flag ^= 1;
            modify_prog(p_node.prog_id, &p_node);
#ifdef MULTIFEED_SUPPORT
            multifeed_set_parentlock( p_node.sat_id, p_node.tp_id, p_node.prog_number, p_node.lock_flag );
#endif
            break;
        case CHAN_EDIT_SKIP:
            p_node.skip_flag ^= 1;
            modify_prog(p_node.prog_id, &p_node);     
            break;
        case CHAN_EDIT_MOVE:
            if(move_channel_idx > ch_cnt)
            {
                move_channel_idx = sel;
            }
            else
            {
                if(move_channel_idx != sel)
                {
                    move_prog(sel,move_channel_idx);
                    play_chan_pos = get_prog_pos(play_chan_id);
                    osd_set_obj_list_single_select(ol, play_chan_pos);
                }
                move_channel_idx = INVALID_POS_NUM;
            }
            break;
        case CHAN_EDIT_EDIT:
            if (m_sorted_flag)
            {
                if (!win_chlist_save_sorted_prog())
                {
                    break;
                }
            }
            get_prog_at(sel,&p_node);
            if( win_chanrename_open(p_node.prog_id,&chan_name))
            {
                com_uni_str_copy_char(p_node.service_name,chan_name);
                modify_prog(p_node.prog_id, &p_node);
            }
            break;
        case CHAN_EDIT_DEL:
            if (m_sorted_flag)
            {
                if (!win_chlist_save_sorted_prog())
                {
                    break;
                }
            }
            win_chlst_switch_channel_del_flag(sel);
            break;
        default:
            break;
        }

        osd_track_object((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

void vact_del_proc(chan_edit_grp_t *pcheditgrp, VACTION act, COM_POP_LIST_PARAM_T *param,
    OSD_RECT *rect, OBJLIST *ol)
{
    UINT8    i             = 0;
    UINT8    sort_flag     = 0;
    UINT16   sel           = 0;
    UINT32   prog_id_cur   = 0;
    UINT32   prog_id_play  = 0;
    P_NODE   p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    if(CHAN_EDIT_NONE == chan_edit_type_btnidx)
    {
        for(i=0;i<pcheditgrp->btn_cnt;i++)
        {
            if(act == pcheditgrp->btn_act[i])
            {
                chan_edit_type_btnidx = pcheditgrp->btn_idx[i];// CHAN_EDIT_FAV + act - VACT_FAV;
                break;
            }
        }
    }
    else
    {
        chan_edit_type_btnidx = CHAN_EDIT_NONE;
    }
    //win_chlist_set_btn_display2(TRUE,act);
    if(chan_edit_type_btnidx != CHAN_EDIT_NONE)
    {
         win_chlist_set_btn_display2(TRUE,act);
    }
    else
    {
        win_chlist_set_btn_display(TRUE);
    }

    if(((VACTION)VACT_SORT == act) || ((VACTION)VACT_ADD == act))
    {
        if((VACTION)VACT_SORT == act)
        {
            param->cur = 0xFFFF;
            param->selecttype = POP_LIST_SINGLESELECT;
	     //由于修改了节目编辑中BTN的位置
	     //所以在进行SORT时需要修改排序选择列表的位置
           // osd_set_rect(rect,CH_5BTN_L, CH_BTN_T + CH_BTN_H + 20, 320,260);
            osd_set_rect(rect,615, 277 , 320,260);
            sel = win_com_open_sub_list(POP_LIST_TYPE_SORTSET,rect,param);
            switch(sel)
            {
                case 0:
                    sort_flag = PROG_NAME_SORT;
                    break;
                case 1:
                    sort_flag = PROG_NAME_SORT_EXT;
                    break;
                case 2:
                    sort_flag = PROG_FTA_SORT_EXT;//PROG_FTA_SORT;
                    break;
                case 3:
                    sort_flag = PROG_LOCK_SORT;
                    break;
                case 4:
                    sort_flag = PROG_DEFAULT_SORT;
                    break;
                default:
                    sort_flag = 0xFF;
                    break;
            }
        
            if(sort_flag != 0xFF)
            {
                /* Clear Delete flags, as delete flag is not sort.*/
                win_chlst_clear_channel_del_flags();

                get_prog_at(osd_get_obj_list_cur_point(ol),&p_node);
                prog_id_cur  = p_node.prog_id;

                get_prog_at(osd_get_obj_list_single_select(ol),&p_node);
                prog_id_play = p_node.prog_id;

                sort_prog_node(sort_flag);
                sel = get_prog_pos(prog_id_cur);

                if(INVALID_POS_NUM == sel)
                {
                    sel = 0;
                }
                osd_set_obj_list_top(ol, sel / osd_get_obj_list_page(ol) * osd_get_obj_list_page(ol));
                osd_set_obj_list_cur_point(ol, sel);
                osd_set_obj_list_new_point(ol, sel);

                sel = get_prog_pos(prog_id_play);
                if(INVALID_POS_NUM == sel)
                {
                    sel = 0;
                }
                osd_set_obj_list_single_select(ol, sel);
                osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

                sys_data_set_cur_group_channel(sel);
                m_sorted_flag = TRUE;
            }
        }

        chan_edit_type_btnidx = CHAN_EDIT_NONE;
        win_chlist_set_btn_display(TRUE);
    }
}

UINT16 list_index_2_channel_index(OBJLIST *ol,UINT16 list_idx)
{
    char   flag      = 0;
    UINT16 start_idx = 0;
    UINT16 end_idx   = 0;
    UINT16 chan_idx  = 0;
    UINT16 new_point = 0;

    /* When moving, the channel displayer order is not same as normal */
    if((CHAN_EDIT_MOVE == chan_edit_type_btnidx) && (move_channel_idx!=INVALID_POS_NUM))
    {
        new_point = osd_get_obj_list_new_point(ol);//ol->wNewPoint;
        start_idx = (new_point>move_channel_idx) ? move_channel_idx : new_point;
        end_idx   = (new_point>move_channel_idx) ? new_point : move_channel_idx;

        if((list_idx<start_idx) || (list_idx> end_idx))
        {
            flag = 0;
        }
        else
        {
            if(new_point==move_channel_idx)
            {
                flag = 0;
            }
            else if(new_point<move_channel_idx)
            {
                if((list_idx>new_point) && (list_idx<=move_channel_idx))
                {
                    flag = -1;
                }
            }
            else
            {
                if((list_idx>=move_channel_idx) && (list_idx<new_point))
                {
                    flag = 1;
                }
            }
        }

        if(list_idx==new_point)  /* foucs item show the channel in moving */
        {
            chan_idx = move_channel_idx;
        }
        else
        {
            chan_idx = list_idx + flag;
        }
    }
    else    /* not in moving state*/
    {
        chan_idx = list_idx;
    }

   return chan_idx;

}

BOOL win_chlist_save_sorted_prog(void)
{
    BOOL               changed    = FALSE;
    UINT8              back_saved = 0;
    win_popup_choice_t choice     = 0;

    if (m_sorted_flag)
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL, NULL, RS_MSG_ARE_YOU_SURE_TO_SAVE);
        choice = win_compopup_open_ext(&back_saved);
        if(WIN_POP_CHOICE_YES == choice)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg(NULL, NULL, RS_MSG_WAIT_FOR_SAVING);
            win_compopup_open_ext(&back_saved);
            update_data();
#ifdef _INVW_JUICE
            osal_delay_ms(500);
            inview_update_database();  //v0.1.4
#endif
            win_chlst_clear_channel_del_flags();
            sys_data_check_channel_groups();

            changed = TRUE;
            m_sorted_flag = 0;

            win_compopup_smsg_restoreback();
        }
    }
    else
    {
        changed = TRUE;
    }

    return changed;
}

void win_chlst_play_channel(UINT16 chan_idx)
{
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    osd_set_text_field_content(&chlst_preview_txt, STRING_ID,0);
    win_chlst_draw_preview_window_ext();/*Clear signal status,before change channel*/

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    clear_switch_back_channel();
#endif

#ifdef SAT2IP_CLIENT_SUPPORT
//  get_sat_by_id(p_node.sat_id, &s_node);
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {      
        win_satip_stop_play_channel();
        satip_clear_task_status();
    }
#endif

    api_play_channel(chan_idx, TRUE, TRUE/*chan_edit_grop_idx== CHAN_EDIT_GRP_NOMAL*/,FALSE);

#ifdef SAT2IP_CLIENT_SUPPORT
    if (api_cur_prog_is_sat2ip())//(1 == s_node.sat2ip_flag)
    {
        win_satip_play_channel(chan_idx, FALSE, 0, 0);
    }
#endif

    get_prog_at(chan_idx,&p_node);
    play_chan_id = p_node.prog_id;

    win_chlst_draw_preview_window();

#ifdef CAS9_V6
    //force update URI info.
    check_last_live_uri();
#endif    
}

//节目编辑界面不需要显示过多的信息
//故将部分多余的显示信息注释掉
void win_chlst_draw_channel_infor(void)
{
    OBJLIST    *ol            = NULL;
    TEXT_FIELD *txt           = NULL;
    UINT16     chan_idx       = 0;
    char       str[50]        = {0};
    UINT8 str_audio_type[32]  = {0};
    UINT16     audiopid       = 0;
    UINT16     ttxpid         = 0;
    UINT16     subtpid        = 0;
    UINT32     audio_pid_idx  = 0;
    UINT16     top_idx        = 0;
    struct nim_device *priv_nim_dev = NULL;
    S_NODE s_node;
    T_NODE t_node;
    P_NODE p_node;

    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    ol = &chlst_ol;
//    txt = &chlist_infor_txt;
    chan_idx = list_index_2_channel_index(ol,osd_get_obj_list_cur_point(ol));

    get_prog_at(chan_idx, &p_node);
    get_tp_by_id(p_node.tp_id,&t_node);
    get_sat_by_id(t_node.sat_id, &s_node);

    top_idx = 0;
    if ((FRONTEND_TYPE_T == t_node.ft_type) || (FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        if ((FRONTEND_TYPE_T == board_get_frontend_type(0)) || (FRONTEND_TYPE_ISDBT == board_get_frontend_type(0)))
        {
            priv_nim_dev = g_nim_dev;
        }
        else
        {
            priv_nim_dev = g_nim_dev2;
        }

        if(priv_nim_dev != NULL)
        {
            set_tnode_tuner_param(priv_nim_dev, &t_node);
        }
    }

#ifdef SD_UI
    txt->head.frame.u_width = txt->head.frame.u_width - 50;
#endif

    //satellite name
#if 0
    if (t_node.t2_signal)
    {
        snprintf(str,50,"%s","DVBT2");
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else
    {
      #ifndef DB_USE_UNICODE_STRING
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)s_node.sat_name);
      #else
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)s_node.sat_name);
      #endif
    }

    txt->head.frame.u_top = INFO_TXT_T;
    top_idx++;
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
#endif

    //Channel name
#if 0
#ifndef DB_USE_UNICODE_STRING
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)p_node.service_name);
#else
    osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)p_node.service_name);
#endif
    txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
    top_idx++;
    osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
#endif

#ifdef SD_UI
    txt->head.frame.u_width = txt->head.frame.u_width + 50;
    if(FRONTEND_TYPE_C == t_node.ft_type)  //TP
    {
        snprintf(str,50,"%lu  %lu  %s", t_node.frq, t_node.sym, qam_table[t_node.fec_inner - 4]);
    }
    else if(FRONTEND_TYPE_S == t_node.ft_type)
    {
        snprintf(str,50,"%lu/%s/%lu", t_node.frq, (0 == t_node.pol) ? "H" : "V",t_node.sym);
    }
    else if((FRONTEND_TYPE_T == t_node.ft_type)||(FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        gen_fft_modulation_str(&t_node, str, 50);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
    top_idx ++;
    osd_draw_object( (POBJECT_HEAD)txt,C_UPDATE_ALL);
#else
    char strqam[20] = {0};

    if(FRONTEND_TYPE_C == t_node.ft_type)
    {
       snprintf(str,50,"%d  %d  %s", (int)t_node.frq*10, (int)t_node.sym,qam_table[t_node.fec_inner - 4]);
      // snprintf(strqam, 20,"%s", qam_table[t_node.fec_inner - 4]);
    }
    else if(FRONTEND_TYPE_S == t_node.ft_type )
    {
        snprintf(str, 50,"%d/%s/%d", (int)t_node.frq, (0 == t_node.pol) ? "H" : "V",(int)t_node.sym);
    #ifdef MULTISTREAM_SUPPORT
        if (1 == t_node.t2_profile)
        {
            snprintf(str + STRLEN(str), 50 - STRLEN(str), "/%c%2d", 'M', t_node.plp_id);
        }
        else
        {
            snprintf(str + STRLEN(str), 50 - STRLEN(str), "/%c", 'S');
        }
    #endif
    }
    else if((FRONTEND_TYPE_T == t_node.ft_type)||(FRONTEND_TYPE_ISDBT == t_node.ft_type))
    {
        gen_fft_modulation_str(&t_node, str, 50);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
//    txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
     txt->head.frame.u_top = INFO_TXT_T; 	
    top_idx++;
    osd_draw_object( (POBJECT_HEAD)txt,C_UPDATE_ALL);
    //Qam
#if 0
    if(FRONTEND_TYPE_C == t_node.ft_type)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)strqam);
        txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
        top_idx++;
        osd_draw_object( (POBJECT_HEAD)txt,C_UPDATE_ALL);
    }
#endif
#endif

    //PID
#if 0
    audiopid = p_node.audio_pid[0];
    ttxpid = p_node.teletext_pid;
    subtpid = p_node.subtitle_pid;

    get_ch_pids(&p_node,&audiopid,&ttxpid,&subtpid,&audio_pid_idx);

    if(0 == p_node.audio_count)
    {
        snprintf(str, 50,"PID: %d/No Audio/%d",p_node.video_pid,p_node.pcr_pid);
    }
    else
    {
        key_check_ddplus_stream_changed(&audiopid);
        if(AUDIO_TYPE_TEST(audiopid,AC3_DES_EXIST))
        {
            strncpy((char *)str_audio_type, "Dolby D", (32-1));
        }
        else if(AUDIO_TYPE_TEST(audiopid,EAC3_DES_EXIST))
        {
            strncpy((char *)str_audio_type, "Dolby D+", (32-1));
        }
        else if(AUDIO_TYPE_TEST(audiopid,AAC_DES_EXIST))
        {
            strncpy((char *)str_audio_type, "AAC", (32-1));
        }
        else if(AUDIO_TYPE_TEST(audiopid ,ADTS_AAC_DES_EXIST))
        {
            strncpy((char *)str_audio_type, "AAC", (32-1));
        }
        else
        {
            MEMSET(str_audio_type, 0, sizeof(str_audio_type));
        }
        snprintf(str,50,"PID: %d/%d%s/%d",p_node.video_pid,audiopid & 0x1FFF,str_audio_type, p_node.pcr_pid);
    }
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
    top_idx++;
    osd_draw_object( (POBJECT_HEAD)txt,C_UPDATE_ALL);

     if((FRONTEND_TYPE_T == t_node.ft_type)||(FRONTEND_TYPE_ISDBT == t_node.ft_type))
     {
         gen_gi_fec_str(&t_node, str);
         osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
         txt->head.frame.u_top = INFO_TXT_T + INFO_TXT_H*top_idx;
         top_idx++;
         osd_draw_object((POBJECT_HEAD)txt,C_UPDATE_ALL);
     }

    osd_draw_object( (POBJECT_HEAD)&chlist_infor_bmp,C_UPDATE_ALL);
#endif 
}

void win_chlst_get_preview_rect(UINT16 *x,UINT16 *y, UINT16 *w, UINT16 *h)
{
    struct osdrect osd_rect;
    UINT16 left          = 0;
    UINT16 top           = 0;
    UINT16 width         = 0;
    UINT16 height        = 0;
    INT32  left_offset   = 0;
    INT32  top_offset    = 0;
    INT32  width_offset  = 0;
    INT32  height_offset = 0;
    UINT8  tv_mode       = 0;

    tv_mode = api_video_get_tvout();
    MEMSET(&osd_rect, 0, sizeof(struct osdrect));
    osd_get_rect_on_screen(&osd_rect);
#ifndef SD_UI
    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;
    left = (PREVIEW_L + osd_rect.u_left) * 720 / 1280;
    width = PREVIEW_W * 720 / 1280;

    if((TV_MODE_720P_50 == tv_mode) || (TV_MODE_1080I_25 == tv_mode)
       || (TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode)
       || (TV_MODE_1080P_25 == tv_mode) || (TV_MODE_1080P_50 == tv_mode)
       || (TV_MODE_1080P_24 == tv_mode)
       || (TV_MODE_PAL_N == tv_mode))
    {
        top = (PREVIEW_T + osd_rect.u_top) * 576 / 720;
        height = PREVIEW_H * 576 / 720;
    }
    else if((TV_MODE_720P_60 == tv_mode) || (TV_MODE_480P == tv_mode)
        || (TV_MODE_1080I_30 == tv_mode) || (TV_MODE_NTSC358 == tv_mode)
        || (TV_MODE_1080P_30 == tv_mode) || (TV_MODE_1080P_60 == tv_mode)
        || (TV_MODE_NTSC443 == tv_mode)|| (TV_MODE_PAL_M == tv_mode ))
    {
        top = (PREVIEW_T + osd_rect.u_top) * 480 / 720;
        height  = PREVIEW_H * 480 / 720;
    }

    top_offset = BORDER_WIDHT;//+6;
    height_offset =  (BORDER_WIDHT /*+ 2*/)*2;

    *x = left + left_offset;//osd_rect.uLeft + left + left_offset;
    *y = top + top_offset;//osd_rect.uTop + top + top_offset;
    *w = width - width_offset;
    *h = height - height_offset;

#ifdef BIDIRECTIONAL_OSD_STYLE

    if (TRUE == osd_get_mirror_flag())
    {
        *x = 720 - *x - *w;
    }
#endif
#else

    left_offset = BORDER_WIDHT;
    width_offset = BORDER_WIDHT*2;//12;
    top_offset = BORDER_WIDHT;//12;
    height_offset =  BORDER_WIDHT*4;//25;

    if((TV_MODE_576P == tv_mode) || (TV_MODE_PAL == tv_mode))
    {
        left    = PREVIEW_L;
        top     = PREVIEW_T;
        width   = PREVIEW_W;
        height  = PREVIEW_H;
    }
    else if((TV_MODE_480P == tv_mode) || (TV_MODE_NTSC358 == tv_mode))
    {
        top     = (PREVIEW_T ) * 480 / 576;//+ osd_rect.uTop
        height  = PREVIEW_H * 480 / 576 -5 ;
        left    = PREVIEW_L;//+osd_rect.uLeft
        width   = PREVIEW_W;
    }
    else
    {
        left    = PREVIEW_L;
        top     = PREVIEW_T;
        width   = PREVIEW_W;
        height  = PREVIEW_H;
    }

    *x = (osd_rect.u_left + left + left_offset);
    *y = (osd_rect.u_top + top + top_offset);// + tb_w;// + tb_w;
    *w = (width - width_offset);
    *h = (height - height_offset);
#endif
}


