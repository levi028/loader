/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_auto_update_tp.c
*
*    Description: auto search program when detect the TP server has been changed
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef AUTO_UPDATE_TPINFO_SUPPORT
#include <types.h>
#include <osal/osal_timer.h>
#include <api/libsi/sie_monitor.h>
#include <api/libpub/lib_frontend.h>
#include <api/libsi/si_module.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsi/si_tdt.h>
#include <api/libtsi/psi_db.h>
#include <hld/decv/decv.h>
#include <api/libpub/lib_device_manage.h>
#include <sys_memmap.h>
#include <api/libsi/si_eit.h>

#include "control.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "si_auto_update_tp.h"
#include "win_password.h"
#include "key.h"
#include "ctrl_play_channel.h"
#include "ap_ctrl_display.h"
#include "win_disk_operate.h"
#include "win_mute.h"
//#define AUTO_UPDATE_DEBUG
#ifdef AUTO_UPDATE_DEBUG
#define AUTP_PRINTF libc_printf
#else
#define AUTP_PRINTF(...) do{}while(0)
#endif

#define COUNT_TIMES 3

enum
{
    GET_CRC_VALUE_VARIABLE = 0x02000000UL,
};

struct new_prog
{
    UINT32 prog_id;
    UINT8 av_flag;
};

static struct new_prog new_progs[PROGRAM_NUM];
static UINT32 new_prog_num = 0;

#ifdef _LCN_ENABLE_
static UINT16 lcn_value = 0xFFFF;
#endif

#ifdef AUDIO_DESCRIPTION_SUPPORT
static BOOL ad_is_open = FALSE;
#endif

static T_NODE cur_sim_tnode;
static BOOL is_open_notice = FALSE;
static BOOL play_back_flag = FALSE;
static BOOL stream_change  = FALSE;
static UINT32 old_crc       = 0xFFFFFFFF;
static UINT8 count         = 0;
static UINT8 package_cnt   = 0;
static struct ts_route_info ts_route;

static void play_default_program(void);
static void del_progs_under_tp(void);

static void reset_variable(void)
{
    MEMSET(new_progs, 0, PROGRAM_NUM * sizeof(struct new_prog));

    #ifdef _LCN_ENABLE_
    lcn_value = 0xFFFF;
    #endif

    new_prog_num = 0;
    stream_change = FALSE;
    play_back_flag = FALSE;
}

void set_old_crc_value(void)
{
    old_crc = cur_sim_tnode.crc_h8 << 24 | cur_sim_tnode.crc_t24;
}

BOOL get_stream_change_flag(void)
{
    return stream_change;
}

static void save_new_progs(UINT32 prog_id, UINT8 av_flag)
{
    if(prog_id != 0x1FFF)
    {
        new_progs[new_prog_num].prog_id = prog_id;
        new_progs[new_prog_num].av_flag = av_flag;
        ++new_prog_num;
    }
}

static void srch_handler(UINT8 aue_type, UINT32 aue_value)
{
    UINT32 msg_type = 0;
    UINT32 msg_code = 0;

    if((ASM_TYPE_PROGRESS == aue_type) && (AS_PROGRESS_SCANOVER == aue_value))
    {
        msg_type = CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO;
        msg_code = CTRL_MSG_SUBTYPE_CMD_PLAY_CHANNEL;
        ap_send_msg(msg_type, msg_code, TRUE);
    }
}

static INT32 tp_search_set_param(T_NODE *t_node, struct as_service_param *as_param)
{
    struct as_service_param *param = as_param;
	#ifndef VFB_SUPPORT
    	struct vdec_device *vdec = NULL;
    #endif
    struct vdec_io_get_frm_para vfrm_param;
    UINT32 search_mode = 0;
    UINT32 prog_type   = 0;
    UINT32 addr        = 0;
    UINT32 len         = 0;
    INT32  ret         = 0;
    T_NODE cur_t_node;

    if((NULL == t_node) || (NULL == as_param))
    {
        return ERR_FAILUE;
    }
    else
    {
        MEMSET(&vfrm_param, 0, sizeof(struct vdec_io_get_frm_para));
        MEMSET(&cur_t_node, 0, sizeof(T_NODE));
        MEMCPY(&cur_t_node, t_node, sizeof(T_NODE));
        recreate_tp_view(VIEW_SINGLE_SAT, 1);

        switch(cur_t_node.ft_type)
        {
            case FRONTEND_TYPE_ISDBT:
            case FRONTEND_TYPE_T:
                {
                    ret = lookup_node(TYPE_TP_NODE, &cur_t_node, cur_t_node.sat_id);
                    if (ret != SUCCESS)
                    {
                        add_node(TYPE_TP_NODE, cur_t_node.sat_id, &cur_t_node);
                        update_data();
                    }

                    param->as_frontend_type = cur_t_node.ft_type;
                    param->as_method = AS_METHOD_FREQ_BW;
                    param->as_sat_cnt = 1;
                    param->sat_ids[0] = cur_t_node.sat_id;  // MAX_SAT_NUM-1;//1;
                }
                break;
            case FRONTEND_TYPE_S:
                {
                #ifdef NEW_DEMO_FRAME
                    param->as_frontend_type = FRONTEND_TYPE_S;
                    param->as_sat_cnt = 1;
                    param->sat_ids[0] = cur_t_node.sat_id;
                #else
                    param->as_sat_id = cur_t_node.sat_id;
                #endif
                    param->as_method = AS_METHOD_TP;
                #ifdef PLSN_SUPPORT
                    param->super_scan = 0;
                    param->pls_num = cur_t_node.pls_num;
                #endif
                }
                break;
            case FRONTEND_TYPE_C:
                {
                    param->as_method = AS_METHOD_TP;
                    param->as_sat_cnt = 1;
                    param->sat_ids[0] = cur_t_node.sat_id;
                    param->as_frontend_type = FRONTEND_TYPE_C;
                    param->ft_count=0;
                }
                break;
            default:
                break;
        }

        param->as_p_add_cfg = PROG_ADD_REPLACE_OLD;
        prog_type = P_SEARCH_TV | P_SEARCH_RADIO;
        search_mode = P_SEARCH_FTA | P_SEARCH_SCRAMBLED;
        param->as_prog_attr = prog_type | search_mode;
        param->as_from = cur_t_node.tp_id;
        param->as_to = 0;
        param->as_handler = srch_handler;

        update_data();

        #if 1//def VFB_SUPPORT
        addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
        addr &= 0x8fffffff;
        len = __MM_AUTOSCAN_DB_BUFFER_LEN;
        #else
        if(cur_t_node.ft_type != FRONTEND_TYPE_C)
        {
            addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
            addr &= 0x8fffffff;
            len = __MM_AUTOSCAN_DB_BUFFER_LEN;
        }
        else
        {
            vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
            vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
            vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
            addr = vfrm_param.t_frm_info.u_y_addr;
            len = (vfrm_param.t_frm_info.u_height * vfrm_param.t_frm_info.u_width * 3) / 2;

    	    if(((UINT32)NULL == addr) || (addr > 0x8fffffff)
                || (len > __MM_AUTOSCAN_DB_BUFFER_LEN)
                || (len <= 0))
    	    {
    		    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
                addr &= 0x8fffffff;
                len = __MM_AUTOSCAN_DB_BUFFER_LEN;
    		    //libc_printf("addr: 0x%x,len: 0x%x",addr,len);
    	    }
        }
        #endif

        db_search_init((UINT8 *)addr, len);

        return SUCCESS;
    }
}

static void stop_all_pvr(void)
{
#if 0//def MULTI_DESCRAMBLE
    struct ts_route_info ts_route_local;
    INT32 ts_route_id = 0;
    int dmx_index = 0;
#endif

    UINT8 cur_rec_num = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    P_NODE pnode;

    pvr_info  = api_get_pvr_info();
    cur_rec_num = pvr_info->rec_num;
    MEMSET(&pnode, 0, sizeof(P_NODE));

    if(cur_rec_num > 0)
    {
        api_stop_play_record(FALSE);

        if(1 == pvr_info->rec[0].record_chan_flag)
        {
            api_stop_record( pvr_info->rec[0].record_handle, 1);
        }

        if(1 == pvr_info->rec[1].record_chan_flag)
        {
            api_stop_record( pvr_info->rec[1].record_handle, 1);
        }
    }
}

static BOOL check_pvr_state(void)
{
    BOOL   ret    = FALSE;
    UINT8  i      = 0;
    UINT8  num    = 0;
    UINT32 rec_id = 0;
    pvr_play_rec_t *pvr_info = NULL;
    P_NODE pnode;

    pvr_info  = api_get_pvr_info();
    num = pvr_info->rec_num;
    MEMSET(&pnode, 0, sizeof(P_NODE));

    if(num > 0)
    {
        for(i = 0; i < num; ++i)
        {
            if(1 == pvr_info->rec[i].record_chan_flag)
            {
                rec_id = pvr_info->rec[i].record_chan_id;
                get_prog_by_id(rec_id, &pnode);

                if(pnode.tp_id != cur_sim_tnode.tp_id)
                {
                    ret = TRUE;
                }
            }
        }
    }

    return ret;
}

static BOOL check_timer(UINT32 prog_id)
{
    P_NODE pnode;

    MEMSET(&pnode, 0, sizeof(P_NODE));

    if(DB_SUCCES == get_prog_by_id(prog_id, &pnode))
    {
        if(pnode.tp_id == cur_sim_tnode.tp_id)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

static void clear_all_timer(void)
{
    int               i         = 0;
    UINT16            total     = 0;
    TIMER_SET_CONTENT *timer    = NULL;
    SYSTEM_DATA       *sys_data = NULL;

    sys_data = sys_data_get();
    total = sys_data->timer_set.common_timer_num + sys_data->timer_set.record_timer_num;
    for(i = 0; i < total; ++i)
    {
        timer = &(sys_data->timer_set.timer_content[i]);
        if(check_timer(timer->wakeup_channel))
        {
            if(TIMER_STATE_READY == timer->wakeup_state)
            {
                timer->timer_mode = TIMER_MODE_OFF;
            }
        }
    }

    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SYSDATA_SAVE,0,FALSE);
    return ;
}

#ifdef AUTO_UPDATE_DEBUG
static void stop_time_shift(void)
{
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if( pvr_info->tms_enable)
    {
        api_stop_tms_record();
    }
}
#endif

static INT32 prepare_tp_scan(T_NODE *t_node, struct as_service_param *as_param)
{
    if((NULL == t_node) || (NULL == as_param))
    {
        return ERR_FAILUE;
    }
    else
    {
        stop_all_pvr();
        clear_all_timer();
        pid_callback_register(save_new_progs);

        if(SUCCESS == tp_search_set_param(t_node, as_param))
        {
            return SUCCESS;
        }
        else
        {
            return ERR_FAILUE;
        }
    }
}

static INT32 start_tp_scan(T_NODE *t_node)
{
    struct as_service_param as_param;
    INT32 ret  = 0;

    MEMSET(&as_param, 0, sizeof(struct as_service_param));
    if(NULL == t_node)
    {
        return ERR_FAILUE;
    }
    else
    {
        ret = prepare_tp_scan(t_node, &as_param);

        if(ret != SUCCESS)
        {
            return ERR_FAILUE;
        }
        else
        {
            #ifdef FSC_SUPPORT
            fsc_control_play_stop_all(1);
            #endif
            ap_clear_all_message();
            epg_off();
            epg_reset();
            epg_release();
            as_service_start(&as_param);
            return SUCCESS;
        }
    }
}

static BOOL ap_filter_specific_msg(void)
{
    OSAL_ER         ret_val       = -1;
    control_msg_t msg;
    UINT32         msg_size      = 0;
    UINT32       begin_time   = 0;
    UINT32       cur_time     = 0;
    UINT32       timeout      = 100;
    UINT32       waiting_time = 120;
    BOOL         ret          = FALSE;
    BOOL         old_value    = FALSE;

    MEMSET(&msg, 0, sizeof(control_msg_t));
    old_value  = ap_enable_key_task_get_key(TRUE);
    begin_time = osal_get_time();

    while(1)
    {
        ret_val = ap_receive_msg(&msg,(INT32 *)&msg_size,timeout);

        if (ret_val != RET_SUCCESS)
        {
            continue;
        }

        if(CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO == msg.msg_type)
        {
            auto_update_tpinfo_msg_proc(msg.msg_type, msg.msg_code);
            ret = TRUE;
            break;
        }

        cur_time = osal_get_time();

        if(cur_time - begin_time > waiting_time)
        {
            ret = FALSE;
            break;
        }
    }

    old_value = ap_enable_key_task_get_key(old_value);

    return ret;
}


static void win_popup_notice_menu(void)
{
    if(menu_stack_get_top() != NULL)
    {
        menu_stack_pop_all();
    }
    ap_clear_all_menus();

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    #ifdef SD_UI
    win_compopup_set_frame(110, 200, 700, 100);
    #else
    win_compopup_set_frame(200, 250, 600, 150);
    #endif
    win_compopup_set_msg_ext("Auto update TP information, please wait!",NULL,0);
    win_compopup_open();
    is_open_notice = TRUE;

    api_set_system_state(SYS_STATE_SEARCH_PROG);
    ap_filter_specific_msg();
}

static BOOL win_popup_enquire_menu(void)
{
    UINT8   back_saved = 0;
    PRESULT ret = 0;

    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_msg("Program under current TP has changed!\nSelect [YES] will STOP all Record and\
        update program.\nSelect [NO] continue record without update!",NULL,0);

    win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
    ret = win_compopup_open_ext(&back_saved);
    if(WIN_POP_CHOICE_YES == ret)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL check_each_program(UINT32 monitor_id)
{
    DB_VIEW *cur_view = NULL;
    BOOL flag = FALSE;
    UINT16 old_view_type = 0xFFFF;
    UINT32 old_param = 0xFFFFFFFF;
    INT16 prog_num_cur = 0;
    INT16 prog_num_db = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    prog_num_cur = sim_get_prog_number(monitor_id);
    cur_view = (DB_VIEW *)get_cur_view();
    old_view_type = cur_view->cur_filter_mode;
    old_param = cur_view->view_param;

    pvr_info = api_get_pvr_info();
    if(((NULL!= pvr_info)&&(pvr_info->rec_num>0))
        &&(VIEW_SPECAIL_PIP== cur_view->cur_filter_mode) ) //dual
    {
        //flag = FALSE;
        return FALSE;
    }

    if((cur_view->cur_filter_mode != (VIEW_SINGLE_TP | PROG_TVRADIO_MODE)) ||
    (((VIEW_SINGLE_TP|PROG_TVRADIO_MODE)==cur_view->cur_filter_mode)&&(cur_view->view_param!=cur_sim_tnode.tp_id)))
    {
        recreate_prog_view(VIEW_SINGLE_TP | PROG_TVRADIO_MODE, cur_sim_tnode.tp_id);
        flag = TRUE;
        cur_view_type = 0;  //force to create view,fix bug 25233
    }
    prog_num_db = get_prog_num(VIEW_SINGLE_TP | PROG_TVRADIO_MODE, cur_sim_tnode.tp_id);

    if(flag)
    {
        recreate_prog_view(old_view_type, old_param);
        flag = FALSE;
    }

    if(prog_num_cur > prog_num_db)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL check_front_end_type(UINT8 nim_id, T_NODE *tnode)
{
    UINT32 ft_type = 0;
    struct nim_device *nim = NULL;
    struct nim_config config;

    if(NULL == tnode)
    {
        return FALSE;
    }

    MEMSET(&config, 0, sizeof(struct nim_config));

    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    ft_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
    dev_get_nim_config(nim, ft_type, &config);

    if(FRONTEND_TYPE_S == ft_type)
    {
        if((config.xpond.s_info.tp_id == tnode->tp_id) &&
            (config.xpond.s_info.frq == tnode->frq) &&
            (config.xpond.s_info.sym == tnode->sym) &&
            (config.xpond.s_info.pol == tnode->pol) &&
            (config.xpond.s_info.type == tnode->ft_type)
#ifdef MULTISTREAM_SUPPORT
            && (config.xpond.s_info.isid == tnode->plp_id)
#endif
            )
        {
            return TRUE;
        }
        else
        {
           return FALSE;
        }
    }
    else if((FRONTEND_TYPE_T == ft_type)||(FRONTEND_TYPE_ISDBT == ft_type))
    {
        if((config.xpond.t_info.tp_id == tnode->tp_id) &&
           (config.xpond.t_info.frq == tnode->frq) &&
            (config.xpond.t_info.type == tnode->ft_type) &&
            (config.xpond.t_info.sym == tnode->sym) &&
            (config.xpond.t_info.band_width == (UINT32)(tnode->bandwidth * 1000)))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(FRONTEND_TYPE_C == ft_type)
    {
        if((config.xpond.c_info.tp_id == tnode->tp_id) &&
            (config.xpond.c_info.frq == tnode->frq) &&
            (config.xpond.c_info.sym == tnode->sym))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

static BOOL check_crc_value(UINT8 *data, INT32 length, T_NODE *tnode, UINT8 dmx_id_p, UINT8 dmx_id_r, UINT8 nim_id)
{
    UINT32  crcdb     = 0;
    OSAL_ID flag_id   = OSAL_INVALID_ID;
    UINT32  flag      = 0;
    UINT32  cur_crc   = 0;
    UINT32  *crc_addr = NULL;

    if((NULL == data) || (NULL==tnode))
    {
        return FALSE;
    }
    crc_addr = (UINT32 *)(data + (length - PSI_SECTION_CRC_LENGTH));
    cur_crc = (UINT32)(*crc_addr);
    crcdb = tnode->crc_h8 << 24 | tnode->crc_t24;

    if(!check_front_end_type(nim_id, tnode))
    {
        return FALSE;
    }

    osal_flag_wait(&flag, flag_id, GET_CRC_VALUE_VARIABLE, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);

    if(old_crc == cur_crc)
    {
        if(COUNT_TIMES == package_cnt)
        {
            if((cur_crc != crcdb) && (dmx_id_p == dmx_id_r))
            {
                package_cnt = 0;
                return TRUE;
            }
        }

        ++package_cnt;
        return FALSE;
    }
    else
    {
        package_cnt = 0;
    }

    osal_flag_clear(flag_id, GET_CRC_VALUE_VARIABLE);
    osal_flag_set(flag_id, GET_CRC_VALUE_VARIABLE);

    if(cur_crc != crcdb)
    {
        if(dmx_id_p == dmx_id_r)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

}
#ifdef _BC_CA_NEW_
static void stop_bc_service(struct ts_route_info *pts_route)
{
    UINT8 dmx_index = 0;
    UINT32 sim_id = 0;
    P_NODE      temp_node;
    if(NULL == pts_route)
    {
        return ;
    }

    get_prog_by_id(pts_route->prog_id, &temp_node);
    bc_stop_descrambling(temp_node.prog_number);
    api_mcas_stop_service_multi_des(temp_node.prog_number);


}
#endif

static BOOL launch_tp_scan(struct ts_route_info *ts_route)
{
    UINT32 msg_type = 0;
    UINT32 msg_code = 0;

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    POBJECT_HEAD topmenu = menu_stack_get_top();
#endif
    pvr_play_rec_t *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    #ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    if(topmenu == (POBJECT_HEAD)&g_win2_epg)
    {
        set_old_crc_value();
        return FALSE;
    }
    #endif

    #ifdef AUDIO_DESCRIPTION_SUPPORT
    ad_is_open = sys_data_get_ad_service();
    if(ad_is_open)
    {
        sys_data_set_ad_service(FALSE);
    }
    #endif

    if(!play_back_flag)
    {
        api_osd_mode_change(OSD_NO_SHOW);

        #ifdef _BC_CA_NEW_
	    stop_bc_service(ts_route);
	    #endif


        api_sim_stop(ts_route);
        set_old_crc_value();

        msg_type = CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO;
        msg_code = CTRL_MSG_SUBTYPE_CMD_START_TP_SCAN;
        ap_send_msg(msg_type, msg_code, TRUE);

        if((PVR_STATE_TMS_PLAY ==  pvr_info->pvr_state) || (PVR_STATE_REC_PLAY ==  pvr_info->pvr_state))
        {
            play_back_flag = TRUE;
        }

        return TRUE;
    }
    else
    {
        play_back_flag = FALSE;
        return FALSE;
    }
}

void update_crc_value(UINT8 *data, UINT32 length, T_NODE *node)
{
    UINT8 crch8 = 0;
    UINT32 crct24 = 0;
    UINT32 crcValue = 0;
    UINT32 *crcAddr = NULL;
    DB_VIEW *cur_view = NULL;
    UINT16 old_view_type = 0xFFFF;
    UINT32 old_param = 0xFFFFFFFF;

    if(NULL == data)
    {
        return ;
    }

    crcAddr = (UINT32 *)(data + (length - PSI_SECTION_CRC_LENGTH));
    crcValue = (UINT32)(*crcAddr);
    crch8 = crcValue >> 24;
    crct24 = crcValue & 0x00FFFFFF;

    cur_view = (DB_VIEW *)get_cur_view();
    old_view_type = cur_view->cur_filter_mode;
    old_param = cur_view->view_param;

    node->crc_h8 = crch8;
    node->crc_t24 = crct24;

    recreate_tp_view(VIEW_SINGLE_SAT, node->sat_id);
    modify_tp(node->tp_id, node);
    update_data();
    recreate_prog_view(old_view_type, old_param);
}

static INT32 process_table_change(INT32 ts_route_id, UINT8 *section,struct ts_route_info *ts_route,P_NODE *p_node,
    struct sim_cb_param *sim_param,INT32 length, T_NODE *t_node)
{
    UINT16 pmt_pid         = 0;
    UINT16 cur_ts_id       = 0;
    UINT16 dmx_idx         = 0;
    UINT32 new_sim_id      = 0;
    UINT32 match_cnt       = 0;
    struct dmx_device *dmx = NULL;

    if((NULL == section)||(NULL == ts_route)||(NULL==p_node)||(NULL==sim_param))
    {
        return ERR_FAILUE;
    }

    // do not auto tp update while recording.
    // sim_pat_callback is not reentrant, it just work around
    if(api_pvr_is_record_active())
        return ERR_FAILUE;

    match_cnt = sim_get_prog_pmt_pid(sim_param->sim_id, p_node->prog_number, &pmt_pid);
    if(0 == match_cnt)
    {
        stream_change = TRUE;
        if(launch_tp_scan(ts_route))
        {
            AUTP_PRINTF("[%s]: matchCnt = %d, launch TP scan\n", __FUNCTION__, match_cnt);
            return SUCCESS;
        }
        else
        {
            return ERR_FAILUE;
        }
    }
    else if(1 == match_cnt)
    {
        cur_ts_id = (section[3]<<8)|section[4];

        if(cur_ts_id == cur_sim_tnode.t_s_id)
        {
            if(p_node->pmt_pid != pmt_pid)
            {
                if(ts_route->dmx_sim_info[0].sim_type != MONITE_TB_PMT)
                {
                    return ERR_FAILUE;
                }

                if(ts_route->dmx_sim_info[0].sim_id != INVALID_SIM_ID)
                {
                    if(sim_stop_monitor(ts_route->dmx_sim_info[0].sim_id) != SUCCESS)
                    {
                        AUTP_PRINTF("\tCan not stop monitor!\n");
                    }
                    ts_route->dmx_sim_info[0].sim_id = INVALID_SIM_ID;
                }

                p_node->pmt_pid = pmt_pid;
                modify_prog(p_node->prog_id, p_node);
                update_data();

                new_sim_id = INVALID_SIM_ID;
                dmx_idx = sim_param->dmx_idx;

                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX,dmx_idx);
                new_sim_id = sim_start_monitor(dmx, MONITE_TB_PMT, p_node->pmt_pid, p_node->prog_number);

                if(INVALID_SIM_ID == new_sim_id)
                {
                    AUTP_PRINTF("\nStart new monitor failue!\n");
                    return ERR_FAILUE;
                }
                sim_register_ncb(new_sim_id, api_sim_callback);

                ts_route->dmx_sim_info[0].sim_id = new_sim_id;
                ts_route->dmx_sim_info[0].sim_pid = pmt_pid;
                ts_route_update(ts_route_id, ts_route);

                return SUCCESS;
            }
            else
            {
                stream_change = TRUE;
                if(check_each_program(sim_param->sim_id))
                {
                    if(launch_tp_scan(ts_route))
                    {
                        return SUCCESS;
                    }
                    else
                    {
                        return ERR_FAILUE;
                    }
                }
	            else
	            {
	            	  update_crc_value(section, length, t_node);
	            }
            }
            }
            else
            {
                stream_change = TRUE;
                if(launch_tp_scan(ts_route))
                {
                    AUTP_PRINTF("[%s]: cur_ts_id:%d, db_ts_id:%d launch TP scan\n",
                        __FUNCTION__, cur_ts_id, cur_sim_tnode.t_s_id);
                    return SUCCESS;
                }
                else
                {
                    return ERR_FAILUE;
                }
            }
    }

    return ERR_FAILUE;
}

static BOOL need_to_check_pat_crc(void)
{
    sys_state_t sys_state = SYS_STATE_NORMAL;
    POBJECT_HEAD topmenu = menu_stack_get_top();
    BOOL is_msg_popup = FALSE;
    UINT8 disk_operate_opened = 0;
#ifdef CAS9_V6
    BOOL is_capin_opened = FALSE;
#endif
    BOOL bcheck = FALSE;

    sys_state = api_get_system_state();
    is_msg_popup = win_msg_popup_opend();
    disk_operate_opened = is_disk_operate_opened();
#ifdef CAS9_V6
    is_capin_opened = api_is_capin_opened();
#endif

    if(NULL == topmenu)
    {
        if(SYS_STATE_TEXT == sys_state)
        {
            bcheck = FALSE;
        }
        else if(disk_operate_opened)
        {
            bcheck = FALSE;
        }
        else if(is_msg_popup)
        {
            bcheck = FALSE;
        }
        #ifdef CAS9_V6
        else if(is_capin_opened)
        {
            bcheck = FALSE;
        }
        #endif
        else
        {
            bcheck = TRUE;
        }
    }
    else
    {
        if(topmenu == (POBJECT_HEAD)&g_win2_light_chanlist)
        {
            bcheck = TRUE;
        }
        else if(topmenu == (POBJECT_HEAD)&g_win2_progname)
        {
            bcheck = TRUE;
        }
        #ifdef MP_PREVIEW_SWITCH_SMOOTHLY
        else if(topmenu == (POBJECT_HEAD)&g_win2_epg)
        {
            bcheck = TRUE;
        }
        #endif
        else
        {
            bcheck = FALSE;
        }
    }

    return bcheck;
}

INT32 sim_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    P_NODE p_node;
    T_NODE t_node;

    struct sim_cb_param *sim_param = NULL;
    UINT8 ignore_data = 4;

    INT32 ts_route_id = 0;
    INT32 ret = 0;
    BOOL check_ret = FALSE;

    RET_CODE get_route_ret = 0;
    pvr_play_rec_t *pvr_info = NULL;
    __MAYBE_UNUSED__ POBJECT_HEAD topmenu = menu_stack_get_top();

    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

    pvr_info  = api_get_pvr_info();
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    if(topmenu != (POBJECT_HEAD)&g_win2_epg)
    {
#endif
        if(count < ignore_data)
        {
            ++count;
            return RET_FAILURE;
        }
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    }
#endif

    if((NULL == section) || (0 == param))
    {
        return RET_FAILURE;
    }

    sim_param = (struct sim_cb_param*)param;
    if(MONITE_TB_PAT == sim_param->table)
    {
        count = 0;
        if((PVR_STATE_TMS_PLAY == pvr_info->pvr_state) || (PVR_STATE_REC_PLAY == pvr_info->pvr_state))
        {
            get_route_ret = ts_route_get_by_type(TS_ROUTE_BACKGROUND, (UINT32 *)&ts_route_id, &ts_route);
        }
        else
        {
            get_route_ret = ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route);
        }

        if(get_route_ret != RET_FAILURE)
        {
            if((SUCCESS==get_prog_by_id(ts_route.prog_id, &p_node)) && (SUCCESS==get_tp_by_id(p_node.tp_id, &t_node)))
            {
                MEMCPY(&cur_sim_tnode, &t_node, sizeof(T_NODE));

                if(need_to_check_pat_crc())
                {
                   check_ret = check_crc_value(section, length, &t_node, (UINT8)sim_param->dmx_idx,
                       ts_route.dmx_id, ts_route.nim_id);
                   if(check_ret)
                   {
                        if(get_pwd_is_open())
                        {
                            ap_send_key(V_KEY_EXIT, FALSE);
                            ap_send_key(V_KEY_EXIT, FALSE);
                            ap_send_key(V_KEY_EXIT, FALSE);
                        }

                        if(NULL != sim_param)
                        {
                            ret = process_table_change(ts_route_id, section, &ts_route, &p_node, sim_param,
                            	length, &t_node);
                            return ret;
                        }
                    }
                }
            }
        }
    }

   return ERR_FAILUE;
}

static BOOL is_new_prog(UINT32 prog_id)
{
    INT32 i = 0;
    INT32 j = new_prog_num - 1;
    INT32 mid = 0;

    if(new_progs[i].prog_id== prog_id)
    {
        return TRUE;
    }

    if((j>=0) && (new_progs[j].prog_id == prog_id))
    {
        return TRUE;
    }

    while(i <= j)
    {
        mid = i + (j - i)/2;

        if(new_progs[mid].prog_id == prog_id)
        {
            return TRUE;
        }
        else
        {
            if(prog_id < new_progs[mid].prog_id)
            {
                j = mid -1;
            }
            else if(prog_id > new_progs[mid].prog_id)
            {
                i = mid + 1;
            }
        }
    }

    return FALSE;
}

static BOOL is_new_prog_saved(void)
{
    P_NODE pnode;

    if(new_prog_num > 0)
    {
        MEMSET(&pnode, 0, sizeof(P_NODE));

        if(1 == new_prog_num)
        {
            if(DB_SUCCES == get_prog_by_id(new_progs[0].prog_id, &pnode))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            if((DB_SUCCES == get_prog_by_id(new_progs[0].prog_id, &pnode)) &&
                (DB_SUCCES == get_prog_by_id(new_progs[new_prog_num - 1].prog_id, &pnode)))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

#ifdef _LCN_ENABLE_
void change_node_lcn(UINT8 flag, UINT8 prog_num)
{
    INT32 i = 0;
    P_NODE pnode;
    MEMSET(&pnode, 0, sizeof(P_NODE));

    for(i = 0; i < prog_num; ++i)
    {
        if(get_prog_at(i, &pnode) == DB_SUCCES)
        {
            if(pnode.lcn_true == FALSE && (flag ? (pnode.LCN > lcn_value) : 1))
            {
                pnode.LCN = lcn_value;
                modify_prog(pnode.prog_id, &pnode);
                ++lcn_value;
            }
        }
    }

    sort_prog_node(PROG_LCN_SORT);
    update_data();
}
#endif

static UINT8 is_same_tp(UINT32 tp_id1,UINT32 tp_id2)
{
#if defined(DVBT2_SUPPORT) && defined(SELECT_HIGHER_QUALITY_SERVICE)
	T_NODE t_node1;
    T_NODE t_node2;
    INT32 ret = 0;
#endif

    if(tp_id1 == tp_id2)
    {
        return TRUE;
    }

#if defined(DVBT2_SUPPORT) && defined(SELECT_HIGHER_QUALITY_SERVICE)
    MEMSET(&t_node1,0,sizeof(t_node1));
    MEMSET(&t_node2,0,sizeof(t_node2));
    ret = get_tp_by_id(tp_id1,&t_node1);
    if(ret == DB_SUCCES)
    {
        ret = get_tp_by_id(tp_id2,&t_node2);
        if(ret == DB_SUCCES)
        {
            if((t_node2.frq == t_node1.frq) \
                &&(t_node2.bandwidth == t_node1.bandwidth))
            {
                return TRUE;
            }
        }
    }
#endif

    return FALSE;
}

static void del_progs_from_view(UINT16 prognum)
{
    int  n    = prognum - 1;
    BOOL flag = FALSE;
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));

    while(n >= 0)
    {
        if(DB_SUCCES == get_prog_at(n, &p_node))
        {
            if(is_same_tp(p_node.tp_id,cur_sim_tnode.tp_id) && (!is_new_prog(p_node.prog_id)))
            {
                del_prog_at((UINT16)n);
                flag = TRUE;

                #ifdef _LCN_ENABLE_
                if(p_node.lcn_true == FALSE)
                {
                    if(p_node.LCN < lcn_value)
                    {
                        lcn_value = p_node.LCN;
                    }
                }
                #endif
            }
        }
        --n;
    }

    if(flag)
    {
        update_data();
    }
}

#ifdef _LCN_ENABLE_
void modify_lcn()
{
    UINT16 number = 0;

    if((sys_data_get_lcn()) && (lcn_value != 0xFFFF))
    {
        recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
        number = get_prog_num(VIEW_ALL | TV_CHAN, 0);
        if(number > 0)
        {
            change_node_lcn(TV_CHAN, number);
        }

        recreate_prog_view(VIEW_ALL | RADIO_CHAN, 0);
        number = get_prog_num(VIEW_ALL | RADIO_CHAN, 0);
        if(number > 0)
        {
            change_node_lcn(RADIO_CHAN, number);
        }
    }
}
#endif

static BOOL swap(UINT8 i, UINT8 j)
{
    struct new_prog temp;

    temp = new_progs[i];
    new_progs[i] = new_progs[j];
    new_progs[j] = temp;

    return TRUE;
}

static void sort_new_prog(void)
{
    UINT8 i    = 0;
    UINT8 j    = 0;
    BOOL  flag = FALSE;

    for(i = 0; i < new_prog_num - 1; ++i)
    {
        flag = TRUE;

        for(j = 0; j < new_prog_num - i -1; ++j)
        {
            if(new_progs[j].prog_id > new_progs[j+1].prog_id)
            {
                flag = !(swap(j,j+1));
            }
        }

        if(flag)
        {
            break;
        }
    }
}

static void del_progs_under_tp(void)
{
    UINT8  only_two = 2;
    UINT16 prog_num = 0;

    if(!is_new_prog_saved())
    {
        return ;
    }

    if(new_prog_num > 1)
    {
        if((only_two == new_prog_num) && (new_progs[0].prog_id > new_progs[1].prog_id))
        {
            swap(0,1);
        }
        else
        {
            sort_new_prog();
        }
    }

    recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
    prog_num = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE, 0);
    if(prog_num > 0)
    {
        del_progs_from_view(prog_num);
    }

    recreate_prog_view(VIEW_ALL | RADIO_CHAN, 0);
    prog_num = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE, 0);
    if(prog_num > 0)
    {
        del_progs_from_view(prog_num);
    }

#ifdef _LCN_ENABLE_
    modify_lcn();
#endif

}

static UINT32 get_last_pid_in_cur_group(UINT16 index)
{
    P_NODE pnode;

    MEMSET(&pnode, 0, sizeof(P_NODE));

    if(DB_SUCCES == get_prog_at(index, &pnode))
    {
        return pnode.prog_id;
    }
    else
    {
        return 0xFFFFFFFF;
    }
}

static UINT32 get_a_new_prog(UINT8 av_flag)
{
    UINT16 prog_num = 0;
    UINT32 sele_pid = 0xFFFFFFFF;

    if(TV_CHAN == av_flag)
    {
        prog_num = get_prog_num(VIEW_ALL | PROG_TV_MODE, 0);
        if(prog_num - 1 >= 0)
        {
            sele_pid =  get_last_pid_in_cur_group(prog_num - 1);
        }
        else
        {
            ap_send_key(V_KEY_MENU,FALSE);
        }
    }
    else if(RADIO_CHAN == av_flag)
    {
        prog_num = get_prog_num(VIEW_ALL | PROG_RADIO_MODE, 0);
        if(prog_num - 1 >= 0)
        {
            sele_pid =  get_last_pid_in_cur_group(prog_num - 1);
        }
        else
        {
            ap_send_key(V_KEY_MENU,FALSE);
        }
    }
    else
    {
        sele_pid = 0xFFFFFFFF;
    }

    return sele_pid;
}

static void play_default_program(void)
{
#ifdef SUPPORT_CAS9
    struct nim_device *nim = NULL;
    UINT32 nim_sub_type = 0;
    struct nim_config config;
#endif
    UINT32 prog_id  = 0;
    UINT32 grp_idx  = 0;
    UINT16 channel  = 0;
    UINT8  av_flag  = 0;
    UINT8  prog_num = 0;
    INT32  ret      = 0;
	if(0 == ret)
	{
		;
	}
	#ifdef SELECT_HIGHER_QUALITY_SERVICE
    P_NODE p_node;
	#endif

    ret = recreate_tp_view(VIEW_ALL | PROG_TVRADIO_MODE, 0);
    reset_group();
    grp_idx = sys_data_get_cur_group_index();
    sys_data_check_channel_groups();
    sys_data_set_cur_group_index(grp_idx);

    av_flag = sys_data_get_cur_chan_mode();
    prog_num = sys_data_get_sate_group_num(av_flag);
    if(0 == prog_num)
    {
        av_flag = (TV_CHAN == av_flag)? RADIO_CHAN : TV_CHAN;
        sys_data_set_cur_chan_mode(av_flag);
    }
    sys_data_change_group(grp_idx);
    sys_data_save(1);

    prog_id = get_a_new_prog(av_flag);
#ifdef SUPPORT_CAS9 //bug4759 need call <api_mcas_start_transponder>
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, cur_tuner_idx);
    nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
    MEMSET(&config, 0, sizeof(struct nim_config));
    dev_get_nim_config(nim, nim_sub_type, &config);
    if(FRONTEND_TYPE_C == nim_sub_type)
    {
        config.xpond.c_info.frq = 0;
    }
    frontend_set_nim(nim, NULL, &config.xpond, 1);
    key_set_signal_check_flag(SIGNAL_CHECK_PAUSE);
#endif

#ifdef SELECT_HIGHER_QUALITY_SERVICE
    ret = get_prog_by_id(prog_id,&p_node);
    filter_same_service_for_single_tp(1,p_node.tp_id);
    //prog_id = get_a_new_prog(av_flag);
    prog_id = get_corresponding_prog_id();
    if(!prog_id)
    {
        prog_id = get_a_new_prog(av_flag);
    }
    sys_data_change_group(grp_idx);
#endif

#ifdef FSC_SUPPORT
    fsc_control_play_stop_all(1);
#endif

    if(prog_id != 0xFFFFFFFF)
    {
        api_play_channel(prog_id, TRUE, FALSE, TRUE);
    }
    else
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        api_play_channel(channel, TRUE, FALSE, FALSE);
    }
    reset_variable();

}

static void do_update_process(void)
{
    INT32 ret = 0;
    UINT32 msg_type = 0;
    UINT32 msg_code = 0;
    
    //subt_show_onoff(FALSE);
    #ifdef CI_SUPPORT
    win_ci_dlg_close();
    set_ci_dlg_openning(TRUE);
    #endif

    ret = start_tp_scan(&cur_sim_tnode);

    win_popup_notice_menu();

    if(ret != SUCCESS)
    {
        //play the default channel.
        msg_type = CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO;
        msg_code = CTRL_MSG_SUBTYPE_CMD_PLAY_CHANNEL;
        ap_send_msg(msg_type, msg_code, TRUE);
    }
}
#ifdef SUPPORT_CAS9
#ifdef MULTI_DESCRAMBLE
static void stop_ca_service(struct ts_route_info *pts_route)
{
    UINT8 dmx_index = 0;
    UINT32 sim_id = 0;

    if(NULL == pts_route)
    {
        return ;
    }

    //MEMSET(&ts_route_local, 0, sizeof(struct ts_route_info));
    for(dmx_index = 0; dmx_index < 3;dmx_index++)
    {
        api_mcas_stop_transponder_multi_des(dmx_index);
    }
    dmx_index = pts_route->dmx_id;
    sim_id = pts_route->dmx_sim_info[dmx_index].sim_id;
    api_mcas_stop_service_multi_des(pts_route->prog_id,sim_id);
}
#else
static void stop_ca_service(struct ts_route_info *pts_route)
{
    UINT8 dmx_index = 0;
    UINT32 sim_id = 0;

    if(NULL == pts_route)
    {
        return ;
    }

    api_mcas_stop_transponder();

    dmx_index = pts_route->dmx_id;
    sim_id = pts_route->dmx_sim_info[dmx_index].sim_id;
    api_mcas_stop_service(sim_id);
}
#endif
#endif

void auto_update_tpinfo_msg_proc(UINT32 msg_type,UINT32 msg_code)
{
    BOOL hint_sate = FALSE;

    if(CTRL_MSG_SUBTYPE_CMD_UPDATE_TPINFO == msg_type)
    {
        if(CTRL_MSG_SUBTYPE_CMD_START_TP_SCAN == msg_code)
        {
            api_pvr_tms_proc(FALSE); //Stop tms before TP update scan

            #if defined(SUPPORT_CAS9)
            stop_ca_service(&ts_route);
            #endif
            api_stop_play_prog(&ts_route);
            if(check_pvr_state())
            {
                if(win_popup_enquire_menu())
                {
                    do_update_process();
                }
                else
                {
                    #ifdef AUDIO_DESCRIPTION_SUPPORT
                    if(ad_is_open)
                    {
                        sys_data_set_ad_service(TRUE);
                    }
                    #endif
                }
            }
            else
            {
                do_update_process();
            }

        }
        else if(CTRL_MSG_SUBTYPE_CMD_PLAY_CHANNEL == msg_code)
        {
            as_service_stop();
            db_search_exit();
            del_progs_under_tp();
            pid_callback_unregister();

            epg_init(SIE_EIT_WHOLE_TP, (UINT8 *)__MM_EPG_BUFFER_START, __MM_EPG_BUFFER_LEN, ap_epg_call_back);

            if(get_tdt_parsing())
            {
                set_tdt_parsing(FALSE);
                start_tdt();
            }

            api_set_system_state(SYS_STATE_NORMAL);
            if(is_open_notice)
            {
                win_compopup_close();
                osd_clear_screen();
                is_open_notice = FALSE;
            }
            show_mute_on_off();
            hint_sate = get_rec_hint_state();
            show_rec_hint_osdon_off(hint_sate);

            #ifdef AUDIO_DESCRIPTION_SUPPORT
            if(ad_is_open)
            {
                sys_data_set_ad_service(TRUE);
            }
            #endif

            #ifdef CI_SUPPORT
            set_ci_dlg_openning(FALSE);
            #endif

            play_default_program();
        }
    }
}

#endif

