/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_pip.c
 *
 *    Description: This source file is PIP application process functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/dmx/dmx.h>
#include "ctrl_pip.h"
#include "pvr_control.h"
#include "win_com.h"
#include "./copper_common/dev_handle.h"
#include "win_com_popup.h"
#include "ctrl_util.h"
#include "control.h"
#include "ap_ctrl_ci.h"
#include "pvr_ctrl_basic.h"
#include "key.h"
#include <api/libdb/db_interface.h>

#define TUNER_INVALID_ID    0xFF
#if 1//def PIP_SUPPORT
static UINT32               main_screen_tp_id = 0;
static UINT8                pip_view_mode = 0;
static UINT8                playback_enable = 0;
static UINT32               tp_id1 = 0;
static UINT32               tp_id2 = 0;
static UINT8                main_screen_dmx_id = 0;
static INT8                 tms_dmx_id = 0;
static UINT32               tuner1_state = 0;
static UINT32               tuner1_tp_id = 0;
static UINT32               tuner2_state = 0;
static UINT32               tuner2_tp_id = 0;
static UINT8                main_screen_tuner_id = 0;
static UINT32               main_screen_tuner_state = 0;
static UINT8                pip_screen_tuner_id = 0;
static UINT32               pip_screen_tuner_state = 0;
static UINT8                pip_screen_dmx_id = 0;
static UINT32               pip_screen_tp_id = 0;
#ifdef PIP_SUPPORT
static BOOL                 is_pip_swapped = FALSE; //used for get default dmx to vdec0
static INT8                 main_pic_tuner_id;
static BOOL                 is_pip_chgch_flag = FALSE;
static P_NODE               cur_pip_prog;
#endif
static INT8                 record_dmx_id1 = 0;
static INT8                 record_dmx_id2 = 0;
static INT8                 nim_id1 = 0;
static INT8                 nim_id2 = 0;
static UINT8                ts_gen_enable = 0;
static BOOL                 get_nim1_cfg_success = FALSE;
static BOOL                 get_nim2_cfg_success = FALSE;
static struct nim_config    nim_cfg;
static struct nim_config    nim_cfg2;
static BOOL                 is_pip_view = FALSE;

extern UINT8                cur_view_type;          // need check again
#if (defined CI_SUPPORT)
extern INT8                 main_original_slot;
extern INT8                 pip_original_slot;
#endif

#endif

#ifdef PIP_SUPPORT
void ap_pip_set_prog(P_NODE *ptr_node)
{
    MEMCPY(&cur_pip_prog, ptr_node, sizeof(P_NODE));
}

P_NODE *ap_pip_get_prog(void)
{
    UINT8                   pip_tuner_idx = 0;
    UINT8                   pip_dmx_idx = 0;
    struct ts_route_info    ts_route,
                            ts_route_m,
                            ts_route_pb;
    INT32                   ts_route_id = 0;
    INT32                   ts_route_id_m = 0;
    INT32                   ts_route_id_pb = 0;
    UINT8                   dmx_id = 0;
    struct dmx_device       *dmx = NULL;
    struct dmx_config       dmx_cfg,
                            dmx_cfg2;
    UINT32                  dmx_state = DEV_DMX_STATUS_PLAY;
    UINT32                  dmx_tp_id = 0;
    UINT8                   node_pos = 0;
    P_NODE                  p_node;
    P_NODE                  playing_pnode;
    pvr_record_t            *rec = NULL;
    UINT32                  tp_id1 = 0;
    UINT32                  tp_id2 = 0;
    INT8                    nim_id1 = -1;
    INT8                    nim_id2 = -1;
    UINT8                   rec_num1 = 0,
                            rec_num2 = 0;
    UINT8                   route_num = 0;
    UINT8                   ts_gen = 0;
    UINT16                  route_index[2];
    UINT32                  sub_type = 0;
    INT32                   ret = RET_FAILURE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    if (get_prog_num(VIEW_ALL | PROG_TV_MODE, 0) == 0)
    {
        return NULL;
    }

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    if ((cur_pip_prog.prog_id == 0) || (DB_SUCCES != get_prog_by_id(cur_pip_prog.prog_id, &p_node)))
    {
        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
        if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE))
        {
            dmx_id = 1 - ts_route.dmx_id;
            dmx = (dmx_id == 0) ? g_dmx_dev : ((dmx_id == 1) ? g_dmx_dev2 : NULL);
            if (dmx)
            {
                dev_get_device_status(HLD_DEV_TYPE_DMX, 0, dmx, &dmx_state);
                dev_get_dmx_config(dmx, dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX), &dmx_cfg);
                dmx_tp_id = dmx_cfg.tp_id;
                if (dmx_state == DEV_DMX_STATUS_RECORD)
                {
                    get_prog_by_id(dmx_cfg.dmx_sim_info[0].channel_id, &p_node);
                    ap_pip_set_prog(&p_node);
                }
                else
                {
                    ap_pip_set_prog(&playing_pnode);
                }
            }
        }
        else
        {
            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            if (ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route) != RET_FAILURE)
            {
                ap_pip_set_prog(&playing_pnode);
            }
            else
            {
                return NULL;
            }
        }
    }
    else if (cur_pip_prog.prog_id != 0)
    {
        SYSTEM_DATA *sys_data = NULL;

        sys_data = sys_data_get();
        if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            return &cur_pip_prog;
        }

        if (sys_data->antenna_connect_type == ANTENNA_CONNECT_SINGLE)
        {
            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
            {
                if (((ts_route.nim_id == 0) && (cur_pip_prog.tp_id != playing_pnode.tp_id)) || (ts_route.nim_id != 0))
                {
                    ap_pip_set_prog(&playing_pnode);
                    return &cur_pip_prog;
                }
                else if ((PVR_MAX_RECORDER == api_pvr_get_rec_num_by_dmx(1 - ts_route.dmx_id)))
                {
                    rec = NULL;
                    rec = api_pvr_get_rec_by_dmx(1 - ts_route.dmx_id, 1);
                    if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                    {
                        ap_pip_set_prog(&p_node);
                    }
                }
            }
            else if (ts_route_get_by_type(TS_ROUTE_BACKGROUND, &ts_route_id, &ts_route) != RET_FAILURE)
            {
                get_prog_by_id(ts_route.prog_id, &p_node);
                ap_pip_set_prog(&p_node);
            }
        }
        else if (sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
        {
            T_NODE  t_node_pip;
            T_NODE  t_node_m;

            MEMSET(&t_node_pip, 0, sizeof(T_NODE));
            MEMSET(&t_node_m, 0, sizeof(T_NODE));
            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            MEMSET(&ts_route_pb, 0, sizeof(struct ts_route_info));
            if (((cur_pip_prog.tp_id != playing_pnode.tp_id)
                  && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE))
            || (( pvr_info->rec != 0)
                && (ts_route_get_by_type(TS_ROUTE_BACKGROUND, &ts_route_id, &ts_route) != RET_FAILURE)
                && (ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route_pb) != RET_FAILURE)
                && ((cur_pip_prog.tp_id != ts_route.tp_id)
                   || (( pvr_info->tms_r_handle != NULL)
                      && (ts_route_pb.dmx_id < 2)
                      && ((cur_pip_prog.prog_id != pvr_info->tms_chan_id)
                         || (NULL == api_pvr_get_rec_by_prog_id(cur_pip_prog.prog_id, NULL)))))
                )   // only for 2rec on diff dmx pip want to use the dmx of main_pic
             || ((cur_pip_prog.tp_id == playing_pnode.tp_id)
                && (((cur_pip_prog.tuner1_valid == playing_pnode.tuner1_valid)
                     && (cur_pip_prog.tuner2_valid == playing_pnode.tuner2_valid))
                || ((cur_pip_prog.tuner1_valid == 0) && (cur_pip_prog.tuner2_valid == 0)))))
            {
                pvr_record_t    *rec = NULL;

                if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) == RET_FAILURE)
                {
                    ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route);
                }

                rec = api_pvr_get_rec_by_dmx(1 - ts_route.dmx_id, 1);
                if ((rec != NULL)
                && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES)
                && ((cur_pip_prog.tp_id != p_node.tp_id)
                   || (( pvr_info->tms_r_handle != NULL)
                      && ((cur_pip_prog.prog_id != pvr_info->tms_chan_id)
                         || (NULL == api_pvr_get_rec_by_prog_id(cur_pip_prog.prog_id, NULL))))))
                {
                    ap_pip_set_prog(&p_node);
                }
                else if ((SUCCESS == get_tp_by_id(p_node.tp_id, &t_node_pip))
                && (SUCCESS == get_tp_by_id(playing_pnode.tp_id, &t_node_m))
                && (t_node_m.pol != t_node_pip.pol))
                {
                    ap_pip_set_prog(&playing_pnode);
                }
            }
            else
            {
                //FTA tms play + TSG playback + want PIP, then open background FTA
                if (( pvr_info->tms_r_handle != 0) && api_pvr_check_tsg_state())
                {
                    ap_pip_set_prog(&playing_pnode);
                }
            }
        }
        else    //ANTENNA_CONNECT_DUAL_DIFF
        {
            /*
                            main dmx        pip dmx                tp        nim        backgroud-rec    case
                            dmx 0/1            dmx 1/0                =        =        no                default
                                                                !=        =        no                main-pic
                                                                ?        !=        no                default

                            dmx 0/1            dmx 1/0                =        =        yes                default
                                                                !=        =        yes                rec
                                                                ?        !=        yes    (1rec)        default
                                                                ?        !=        yes  (2rec)        rec

                                            (dmx0 tp, dmx1 tp)    pip_tp
                            dmx 2            (0,0)                                                default

                                    .---------(!0,0)                tp1        tp !=    nim =            rec
                                   /                                tp1        tp !=    nim !=            default
                                  /                                tp1        tp =                        default
                                 /                                tp2                                default
                                /
                            1 or 2rec              (0,!0)                tp2        tp !=    nim =            rec
                                \                                tp2        tp !=    nim !=            default
                                 \                                tp2        tp =                        default
                                  \                                tp1                                default
                                   \
                                     \________(!0,!0)                tp1 or tp2                        default
                                                                other                            rec on tp1

                        */
            //#define get_prog_debug
            S_NODE  s_node;

            get_sat_by_id(cur_pip_prog.sat_id, &s_node);
            pip_tuner_idx = (s_node.tuner1_valid == 1) ? 0 : ((s_node.tuner2_valid == 1) ? 1 : TUNER_INVALID_ID);
            if (TUNER_INVALID_ID == pip_tuner_idx)
            {
                return &cur_pip_prog;
            }


            sub_type = dev_get_device_sub_type(g_dmx_dev, HLD_DEV_TYPE_DMX);
            ret = dev_get_dmx_config(g_dmx_dev, sub_type, &dmx_cfg);
            if (SUCCESS == ret)
            {
                tp_id1 = dmx_cfg.tp_id;
            }
            else
            {
                tp_id1 = 0;
            }

            sub_type = dev_get_device_sub_type(g_dmx_dev2, HLD_DEV_TYPE_DMX);
            ret = dev_get_dmx_config(g_dmx_dev2, sub_type, &dmx_cfg2);
            if (SUCCESS == ret)
            {
                tp_id2 = dmx_cfg2.tp_id;
            }
            else
            {
                tp_id2 = 0;
            }

            ts_route_check_dmx_route(0, &route_num, route_index);
            if (route_num > 0)
            {
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

                //ts_route_get_by_id(route_index[0], &ts_route);
                if (RET_FAILURE == ts_route_get_by_id(route_index[0], &ts_route))
                {
                    PRINTF("ts_route_get_by_id() failed!\n");
                }

                nim_id1 = ts_route.nim_id;
            }
            else
            {
                nim_id1 = -1;
            }

            ts_route_check_dmx_route(1, &route_num, route_index);
            if (route_num > 0)
            {
                MEMSET(&ts_route, 0, sizeof(struct ts_route_info));

                //ts_route_get_by_id(route_index[0], &ts_route);
                if (RET_FAILURE == ts_route_get_by_id(route_index[0], &ts_route))
                {
                    PRINTF("ts_route_get_by_id() failed!\n");
                }

                nim_id2 = ts_route.nim_id;
            }
            else
            {
                nim_id2 = -1;
            }

            rec_num1 = api_pvr_get_rec_num_by_dmx(0);
            rec_num2 = api_pvr_get_rec_num_by_dmx(1);
            MEMSET(&ts_route_m, 0, sizeof(struct ts_route_info));
            if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id_m, &ts_route_m) != RET_FAILURE)
            {
                pip_dmx_idx = 1 - ts_route_m.dmx_id;
                if (( pvr_info->rec_num == 0)
                && (pip_tuner_idx != 255)
                && (pip_tuner_idx == ts_route_m.nim_id)
                && ((cur_pip_prog.tp_id != ts_route_m.tp_id)))
                {
                    ap_pip_set_prog(&playing_pnode);
                }
                else if (1 == pvr_info->rec_num)
                {
                    if ((pip_dmx_idx == 0)
                    && (rec_num1 == 1)
                    && (((pip_tuner_idx == nim_id1) && (cur_pip_prog.tp_id != tp_id1))
                       || (pip_tuner_idx != nim_id1)))
                    {
                        rec = NULL;
                        rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                        if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                        {
                            ap_pip_set_prog(&p_node);
                        }
                    }
                    else if ((1 == pip_dmx_idx)
                         && (1 == rec_num2)
                         && (((pip_tuner_idx == nim_id2) && (cur_pip_prog.tp_id != tp_id2))
                            || (pip_tuner_idx != nim_id2)))
                    {
                        rec = NULL;
                        rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                        if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                        {
                            ap_pip_set_prog(&p_node);
                        }
                    }
                }
                else if (PVR_MAX_RECORDER == pvr_info->rec_num)
                {
                    if (PVR_MAX_RECORDER == api_pvr_get_rec_num_by_dmx(pip_dmx_idx))
                    {
                        if ((NULL == api_pvr_get_rec_by_prog_id(cur_pip_prog.prog_id, &node_pos)))
                        {
                            rec = NULL;
                            rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                            if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                            {
                                ap_pip_set_prog(&p_node);
                            }
                        }
                    }
                    else if ((1 == api_pvr_get_rec_num_by_dmx(ts_route_m.dmx_id))
                         && (1 == api_pvr_get_rec_num_by_dmx(pip_dmx_idx)))
                    {
                        if (((cur_pip_prog.tp_id != ((pip_dmx_idx == 0) ? tp_id1 : tp_id2))
                             && (pip_tuner_idx == ((pip_dmx_idx == 0) ? nim_id1 : nim_id2)))
                        || (pip_tuner_idx != ((pip_dmx_idx == 0) ? nim_id1 : nim_id2)))
                        {
                            rec = NULL;
                            rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                            if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                            {
                                ap_pip_set_prog(&p_node);
                            }
                        }
                    }
                }
            }
            else
            {
                MEMSET(&ts_route_pb, 0, sizeof(struct ts_route_info));
                ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id_pb, &ts_route_pb);
                if ((ts_route_pb.dmx_id < SW_DMX_ID))
                {
                    ts_gen = 1;
                }
                else
                {
                    ts_gen = 0;
                }

                if ((0 == pvr_info->rec_num) /* && (ts_gen == 1)*/
                     && ( pvr_info->tms_r_handle != NULL))
                {
                    //deal with ts-gen playback and no record but has tms record!
                    if (RET_FAILURE != ts_route_get_by_type(TS_ROUTE_BACKGROUND, &ts_route_id, &ts_route))
                    {
                        if (((ts_route.tp_id != cur_pip_prog.tp_id) && (1 == ts_gen))
                        || ((ts_route.tp_id != cur_pip_prog.tp_id) && (ts_route.nim_id == pip_tuner_idx)))
                        {
                            get_prog_by_id(ts_route.prog_id, &p_node);
                            ap_pip_set_prog(&p_node);
                        }
                    }
                }
                else if (1 == pvr_info->rec_num)
                {
                    if ((1 == rec_num1)
                    && (((cur_pip_prog.tp_id != tp_id1) && (tp_id1 != 0) && (pip_tuner_idx == nim_id1))
                       || ((ts_gen) && ((pip_tuner_idx != nim_id1)))))
                    {
                        rec = NULL;
                        rec = api_pvr_get_rec_by_dmx(0, 1);
                        if ((rec != NULL) && (DB_SUCCES == get_prog_by_id(rec->record_chan_id, &p_node)))
                        {
                            ap_pip_set_prog(&p_node);
                        }
                    }
                    else if ((1 == rec_num2)
                         && (((cur_pip_prog.tp_id != tp_id2) && (tp_id2 != 0) && (pip_tuner_idx == nim_id2))
                            || ((ts_gen) && ((pip_tuner_idx != nim_id1)))))
                    {
                        rec = NULL;
                        rec = api_pvr_get_rec_by_dmx(1, 1);
                        if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                        {
                            ap_pip_set_prog(&p_node);
                        }
                    }
                    else
                    {
                        //dmx0 recording, pip in dmx0's tuner but different TP; pip in dmx0 but different tuner. or
                        //dmx1 recording, pip in dmx1's tuner but different TP; pip in dmx1 but different tuner.
                        if (((1 == rec_num1)
                             && (((pip_tuner_idx == nim_id1) && (cur_pip_prog.tp_id != tp_id1))
                             || ((0 == pip_dmx_idx) && (pip_tuner_idx != nim_id1))))
                        || ((1 == rec_num2)
                           && (((pip_tuner_idx == nim_id2) && (cur_pip_prog.tp_id != tp_id2))
                              || ((1 == pip_dmx_idx) && (pip_tuner_idx != nim_id2)))))
                        {
                            ap_pip_set_prog(&playing_pnode);
                        }
                    }
                }
                else if (PVR_MAX_RECORDER == pvr_info->rec_num)
                {
                    if (cur_pip_prog.tp_id == tp_id1)
                    {
                        pip_dmx_idx = 0;
                    }
                    else if (cur_pip_prog.tp_id == tp_id2)
                    {
                        pip_dmx_idx = 1;
                    }

                    if ((((PVR_MAX_RECORDER == rec_num1)
                          && (NULL == api_pvr_get_rec_by_prog_id(cur_pip_prog.prog_id, &node_pos))
                          && (cur_pip_prog.tp_id == tp_id1))
                        || ((PVR_MAX_RECORDER == rec_num2)
                           && (NULL == api_pvr_get_rec_by_prog_id(cur_pip_prog.prog_id, &node_pos))
                           && (cur_pip_prog.tp_id == tp_id2)))
                    || (ts_gen == 1)
                    && (((PVR_MAX_RECORDER == rec_num1) && (cur_pip_prog.tp_id != tp_id1))
                       || ((PVR_MAX_RECORDER == rec_num2) && (cur_pip_prog.tp_id != tp_id2))))
                    {
                        rec = NULL;
                        if (ts_gen != 1)
                        {
                            rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                        }
                        else
                        {
                            // for  BUG18975
                            rec = api_pvr_get_rec(1);
                        }

                        if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                        {
                            ap_pip_set_prog(&p_node);
                        }
                    }
                    else if ((1 == api_pvr_get_rec_num_by_dmx(0)) && (1 == api_pvr_get_rec_num_by_dmx(1)))
                    {
                        if (((cur_pip_prog.tp_id != ((pip_dmx_idx == 0) ? tp_id1 : tp_id2))
                             && (pip_tuner_idx == ((pip_dmx_idx == 0) ? nim_id1 : nim_id2))))
                        {
                            rec = NULL;
                            rec = api_pvr_get_rec_by_dmx(pip_dmx_idx, 1);
                            if ((rec != NULL) && (get_prog_by_id(rec->record_chan_id, &p_node) == DB_SUCCES))
                            {
                                ap_pip_set_prog(&p_node);
                            }
                        }
                    }
                }
            }
        }

        //ap_pip_check_set_prog(&cur_pip_prog);
    }

    return &cur_pip_prog;
}

RET_CODE ap_pip_check_set_prog(P_NODE *ptr_node)
{
    P_NODE  p_node;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    if (get_prog_by_id(ptr_node->prog_id, &p_node) != DB_SUCCES)
    {
        get_prog_at(0, &p_node);
        ap_pip_set_prog(&p_node);

        //libc_printf("Cerrect prog id from %d to%d \n",ptr_node->prog_id,p_node.prog_id);
    }

    //libc_printf("pass prog  id %d\n",ptr_node->prog_id);
}

void ap_pip_exit(void)
{
    struct ts_route_info    ts_route;
    INT32                   ts_route_id;
    struct dmx_device       *dmx = NULL;
    UINT32                  sim_id;
    UINT8                   main_dmx_id = 0;
    sys_state_t             system_state = SYS_STATE_INITIALIZING;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    // I'm not sure this var is global or local, old code no this info
    // Here just for compile.
    BOOL                    pip_opened_after_playback = TRUE;
#ifdef DVR_PVR_SUPPORT
    UINT8                   rec_pos;
    pvr_record_t            *rec = api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos);
#endif
    system_state = api_get_system_state();
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) == RET_FAILURE)
    {
        if (system_state == SYS_STATE_PIP)
        {
            pip_exit();
            pip_opened_after_playback = FALSE;
        }

        return;
    }

    //pip_slide_inout(2);
#ifdef DVR_PVR_SUPPORT
    //stop tms record on pip dmx!!
    if ((!pip_opened_after_playback)
    && ( pvr_info->tms_r_handle != NULL)
    && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) == ts_route.dmx_id))
    {
        api_pvr_tms_proc(FALSE);
    }
#endif
    pip_exit();
    pip_opened_after_playback = FALSE;

    //update pip ts route!
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
    {
        if (ts_route.state & TS_ROUTE_STATUS_RECORD)    //change to background record!
        {
            ts_route.type = TS_ROUTE_BACKGROUND;
            ts_route.state &= ~TS_ROUTE_STATUS_PLAY;
            ts_route_update(ts_route_id, &ts_route);
        }
        else
        {
            //BUG19476
            //if (ts_route.prog_id != progs_use_same_sim_id)
            api_sim_stop(&ts_route);
            ts_route_delete(ts_route_id);
        }
    }

    if ((sys_data_get_normal_tp_switch())
#ifdef DVR_PVR_SUPPORT
    && (!pvr_info->rec_num)
#endif
    )
    {
        sys_data_change_normal_tp_view();
        sys_data_set_normal_tp_switch(FALSE);
    }

    return;
}

BOOL ap_pip_check_swap(void)
{
    BOOL    ret = TRUE;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
#ifdef DVR_PVR_SUPPORT
    if ((ap_pvr_set_state() == PVR_STATE_REC_PLAY)
    || ( pvr_info->pvr_state == PVR_STATE_REC_PVR_PLAY)
    || ( pvr_info->pvr_state == PVR_STATE_TMS_PLAY)
    || ( pvr_info->pvr_state == PVR_STATE_UNREC_PVR_PLAY))
    {
        ret = FALSE;
    }
#endif
    return ret;
}

BOOL ap_get_pip_swapped(void)
{
    return is_pip_swapped;
}

void ap_set_pip_swapped(BOOL is_swap)
{
    is_pip_swapped = is_swap;
}

void ap_pip_swap(BOOL video_swap)
{
    P_NODE                  p_node;
    P_NODE                  playing_pnode;
    UINT32                  sim_id = INVALID_SIM_ID;
    UINT32                  temp;
    UINT8                   temp_id;
    INT32                   ts_route_id1;
    INT32                   ts_route_id2;
    struct ts_route_info    ts_route1;
    struct ts_route_info    ts_route2;
    struct snd_device       *snd = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    BOOL                    start_pip_vdec = TRUE;
    BOOL                    is_swapped = FALSE;
    UINT8                   main_dmx_id = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    BOOL *need_check_tms = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    need_check_tms = api_get_need_check_tms();
    *need_check_tms = FALSE;         //avoid scramble<->fta cause tms start record p_node mistake
    if (!api_pvr_check_dmx_canuseci(TRUE, 0) || !api_pvr_check_dmx_canuseci(TRUE, 1))
    {
#ifdef _INVW_JUICE
        if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
        {
#endif
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(100, 260, 400, 50);
            win_compopup_set_msg_ext("SCRAMBLED RECORD, CAN NOT SWAP!", NULL, 0);
            win_compopup_open();
            osal_task_sleep(2000);
            win_compopup_close();
#ifdef _INVW_JUICE
        }
#endif
        return;
    }

    ap_get_playing_pnode(&playing_pnode);
    MEMCPY(&p_node, &playing_pnode, sizeof(P_NODE));
    MEMCPY(&playing_pnode, &cur_pip_prog, sizeof(P_NODE));
    cur_view_type = 0;              //force to create view
    sys_data_change_normal_tp(&playing_pnode);
    MEMCPY(&cur_pip_prog, &p_node, sizeof(P_NODE));
    cur_view_type = 0;              //force to create view
    sys_data_change_pip_tp(&cur_pip_prog);
    pvr_info->play.play_chan_id = playing_pnode.prog_id;
    cur_view_type = 0;

    //    pip_swap(TRUE);
    deca_stop(g_deca_dev, 0, ADEC_STOP_IMM);
    vdec_stop(g_decv_dev, TRUE, TRUE);
    vdec_stop(g_decv_dev2, TRUE, TRUE);
    MEMSET(&ts_route1, 0, sizeof(struct ts_route_info));
    if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id1, &ts_route1) != RET_FAILURE))
    {
        if (ts_route1.dmx_sim_info[0].used && (ts_route1.dmx_sim_info[0].sim_type == MONITE_TB_PMT))
        {
#if (SUBTITLE_ON == 1)
            subt_unregister(ts_route1.dmx_sim_info[0].sim_id);
#endif
#if (TTX_ON == 1)
            ttx_unregister(ts_route1.dmx_sim_info[0].sim_id);
#endif
#if (ISDBT_CC == 1)
            isdbtcc_unregister(ts_route1.dmx_sim_info[0].sim_id);
#endif
        }

        subt_show_onoff(FALSE);
    }

    if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id1, &ts_route1) != RET_FAILURE)
    {
        dmx_io_control(ts_route1.dmx_id ? g_dmx_dev2 : g_dmx_dev, DMX_PIP_SWAP, video_swap);
    }

    vdec_start(g_decv_dev);

    //    vdec_start(l_decv_dev2);
    deca_io_control(g_deca_dev, DECA_SET_STR_TYPE, AUDIO_MPEG2);
    deca_start(g_deca_dev, 0);
    chchg_switch_audio_chan(snd, playing_pnode.audio_channel);

    //update ts_route
    MEMSET(&ts_route1, 0, sizeof(struct ts_route_info));
    MEMSET(&ts_route2, 0, sizeof(struct ts_route_info));
    if ((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id1, &ts_route1) != RET_FAILURE)
    && (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id2, &ts_route2) != RET_FAILURE))
    {
        if (ts_route2.dmx_sim_info[0].used && (ts_route2.dmx_sim_info[0].sim_type == MONITE_TB_PMT))
        {
#if (SUBTITLE_ON == 1)
            subt_register(ts_route2.dmx_sim_info[0].sim_id);
            subt_show_onoff(TRUE);
#endif
#if (TTX_ON == 1)
            ttx_register(ts_route2.dmx_sim_info[0].sim_id);
#endif
#if (ISDBT_CC == 1)
            isdbtcc_register(ts_route2.dmx_sim_info[0].sim_id);
            isdbtcc_show_onoff(TRUE);
#endif

            //api_osd_layer2_onoff(OSDDRV_ON);
        }

        ts_route1.type = TS_ROUTE_PIP_PLAY;
        ts_route1.vdec_id = 1;
        ts_route1.vpo_id = 1;
        ts_route2.type = TS_ROUTE_MAIN_PLAY;
        ts_route2.vdec_id = 0;
        ts_route2.vpo_id = 0;
        api_dynamic_pid_db_for_swap();
        if ((DUAL_CI_SLOT == g_ci_num) && ((ts_route1.stream_ca_mode) || (ts_route2.stream_ca_mode)))
        {
            UINT8   temp = main_original_slot;
            main_original_slot = pip_original_slot;
            pip_original_slot = main_original_slot;
        }

        if ((sys_data_get_ci_mode() == CI_SERIAL_MODE)
        && (ts_route1.stream_ca_mode == 1) && (ts_route2.stream_ca_mode == 1))
        {
            ts_route2.cia_used = 1; //for 2-CI mode
            ts_route1.cia_used = 0;
            if (ts_route1.prog_id != ts_route2.prog_id)
            {
                if (RET_SUCCESS == ap_ci_switch(&ts_route1, &ts_route2, &start_pip_vdec))
                {
                    temp_id = ts_route2.ts_id;
                    ts_route2.ts_id = ts_route1.ts_id;
                    ts_route1.ts_id = temp_id;
                }
                else
                {
                    ts_route2.cia_used = 0;
                    ts_route1.cia_used = 0;
                }
            }
        }

        ts_route_update(ts_route_id1, &ts_route1);
        ts_route_update(ts_route_id2, &ts_route2);
    }

    if (start_pip_vdec)     // to avoid pip scrambled prog and mosaic after swap!
    {
        vdec_start(g_decv_dev2);
    }

    if (get_pause_state() == TRUE)
    {
        set_pause_on_off(TRUE);
    }

    is_swapped = ap_get_pip_swapped();
    ap_set_pip_swapped(!is_swapped);
    main_dmx_id = ap_get_main_dmx();
    if (RET_FAILURE == dmx_io_control(((main_dmx_id == 0) ? g_dmx_dev : g_dmx_dev2), IS_BOTH_SLOT_PLAY, 0))
    {
        main_dmx_id = 1 - main_dmx_id;
    }

    ap_set_main_dmx(FALSE, main_dmx_id);
    *need_check_tms = TRUE;  //enable tms check on new main screen!
    if (DUAL_CI_SLOT == g_ci_num)
    {
        stop_ci_delay_msg();
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OPERATE_CI, OP_CI_TS_TYPE_CHG, FALSE);
    }
}

void ap_pip_set_chgch(BOOL value)
{
    is_pip_chgch_flag = value;
    return;
}

BOOL ap_pip_check_chgch(void)
{
    return is_pip_chgch_flag;
}

BOOL api_pip_check_stream_route(UINT8 nim_id, UINT8 dmx_id, UINT32 tp_id)
{
    BOOL                ret = TRUE;
    struct nim_device   *nim = NULL;
    struct dmx_device   *dmx = NULL;
    struct nim_config   nim_cfg;
    struct dmx_config   dmx_cfg;
    UINT32              tuner_state = DEV_FRONTEND_STATUS_PLAY;
    UINT32              tuner_tp_id = 0;
    UINT32              dmx_state = DEV_DMX_STATUS_PLAY;
    UINT32              dmx_tp_id = 0;
    UINT32              nim_sub_type;
    UINT32              dmx_sub_type;
    nim = (nim_id == 0) ? g_nim_dev : ((nim_id == 1) ? g_nim_dev2 : NULL);
    dmx = (dmx_id == 0) ? g_dmx_dev : ((dmx_id == 1) ? g_dmx_dev2 : NULL);
    if (nim && dmx)
    {
        nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        dmx_sub_type = dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX);
        dev_get_device_status(HLD_DEV_TYPE_NIM, nim_sub_type, nim, &tuner_state);
        dev_get_device_status(HLD_DEV_TYPE_DMX, dmx_sub_type, dmx, &dmx_state);
        dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
        tuner_tp_id = nim_cfg.xpond.s_info.tp_id;
        dev_get_dmx_config(dmx, dmx_sub_type, &dmx_cfg);
        dmx_tp_id = dmx_cfg.tp_id;
        if (((tuner_state == DEV_FRONTEND_STATUS_RECORD) && (tuner_tp_id != tp_id))
        || ((dmx_state == DEV_DMX_STATUS_RECORD)
           && ((dmx_tp_id != tp_id) || (api_pvr_get_rec_num_by_dmx(dmx_id) > 1))))
        {
            ret = FALSE;
        }
    }

    return ret;
}

BOOL api_pip_check_pip_view(void)
{
    return is_pip_view;
}
#endif
#if 1//def ENHANCE_PIP_VIEW
#ifdef DB_SUPPORT_HMAC
void api_pip_filter_view(UINT8 chan_mode)
{
    UINT32  i = 0;
    UINT32  last_pos = 0;
    UINT32  prog_num = 0;
    P_NODE  p_node;

    //UINT32 tick1 = osal_get_tick();
    
    last_pos = 0;
    prog_num = get_prog_num(TYPE_PROG_NODE | PROG_ALL_MODE, 0);
    for (i = 0; i < prog_num; i++)
    {
        get_prog_at_ncheck(last_pos, &p_node);
        if (p_node.av_flag != chan_mode)
        {
            del_prog_by_id_from_view(p_node.prog_id);
        }
        else
        {
            last_pos++;
        }
    }

    //UINT32 tick2 = osal_get_tick();
    //libc_printf("tt = %d \n",tick2 - tick1);
}
#else
void api_pip_filter_view(UINT8 chan_mode)
{
    UINT32  i = 0;
    UINT32  last_pos = 0;
    UINT32  prog_num = 0;
    P_NODE  p_node;


    last_pos = 0;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    prog_num = get_prog_num(TYPE_PROG_NODE | PROG_ALL_MODE, 0);
    for (i = 0; i < prog_num; i++)
    {
        get_prog_at(last_pos, &p_node);
        if (p_node.av_flag != chan_mode)
        {
            del_prog_by_id(p_node.prog_id);
        }
        else
        {
            last_pos++;
        }
    }

    db_unsave_data_changed();
}
#endif

BOOL api_check_view(pip_db_info *ap_pip)
{
    BOOL        ret = FALSE;
    UINT32      node_tp_id = ap_pip->tp_id;
    UINT8       node_pol = ap_pip->pol;
    UINT32      node_prog_id = ap_pip->pg_id;
    BOOL        node_in_tuner1 = ap_pip->tun1_val;
    BOOL        node_in_tuner2 = ap_pip->tun2_val;
    UINT8       node_tuner_id = 0;
    UINT8       rec_pos = 0;
    SYSTEM_DATA *sys_data = NULL;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    sys_data = sys_data_get();

    if ((node_in_tuner1 == FALSE) && (node_in_tuner2 == FALSE))
    {
        node_in_tuner1 = TRUE;
    }

    if (node_in_tuner1)
    {
        node_tuner_id = 0;
    }
    else if (node_in_tuner2)
    {
        node_tuner_id = 1;
    }

#ifdef DVR_PVR_SUPPORT
    if (( pvr_info->pvr_state == PVR_STATE_UNREC_PVR_PLAY) && (api_pvr_get_back2menu_flag() == TRUE))
    {
        ret = TRUE;
        goto RETURN;
    }
#endif
    if (sys_data->antenna_connect_type == ANTENNA_CONNECT_SINGLE)
    {
        /*
                    PLay                        :    main-pic:  all;          pip:  TP
                    1Rec                    :    main-pic:  TP;        pip:    TP
                    2Rec    in main            :    main-pic:  TP;        pip:    TP
                            in pip                :    main-pic:  TP;        pip:    TP
                            in main&pip            :    main-pic:  TP;        pip:    TP
                */
        if (main_screen_tp_id == 0)
        {
            struct ts_route_info    ts_route;

            MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
            MEMSET(&ts_route, 0x0, sizeof(ts_route));

            //ts_route_get_by_type(TS_ROUTE_BACKGROUND, NULL, &ts_route);
            if (RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_BACKGROUND, NULL, &ts_route))
            {
                PRINTF("ts_route_get_by_type() failed!\n");
            }

            main_screen_tp_id = ts_route.tp_id;
        }

        if (((pip_view_mode == 0)
             && ((node_tp_id == main_screen_tp_id)
                || ((node_tp_id != main_screen_tp_id) && ( pvr_info->rec_num == 0))// for BUG18967
             || ((0 == main_screen_tp_id)
                && ( pvr_info->rec_num != 0)
                && (playback_enable == 0)
                && ((node_tp_id == tp_id1) && (tp_id1 != 0)))))
        || ((pip_view_mode == 1)
             && (((playback_enable)
                  && (((node_tp_id == tp_id1) && (tp_id1 != 0))
                     || ((node_tp_id == tp_id2) && (tp_id2 != 0))))
             || ((!playback_enable) && (node_tp_id == main_screen_tp_id)))))
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }
    else if (sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF)
    {
//Tm = main-pic tuner  To = other tuner
//Tr  = Rec tuner
/*
    Play                        :    main-pic:  T1   all     +    T2  alll;    pip-pic:      Tm  TP      +     To  all
    1 REC        Main Rec        :    main-pic:  Tr   TP    +     To  all;     pip-pic:      Tr   TP       +     To  all
                Pip     Rec        :    main-pic:  Tr   TP     +     To  all;    pip-pic:      Tm     TP    +    Tr   TP
                BK     Rec        :    main-pic:  Tr   TP     +     To  all;    pip-pic:      Tm     TP    +    Tr   TP
    2 REC        main        :    main-pic:  Tr   TP     +     To  all;    pip-pic:      Tr   TP       +     To  all
                pip            :    main-pic:  Tr   TP     +     To  all;    pip-pic:    Tm     TP    +    Tr   TP(2 rec-channels)
                main(pip)        :    main-pic:  Tr   TP     +     To  all;    pip-pic:    Tr     TP    +    To  all
                main&pip  rec    :    main-pic:  Tr1 TP     +     Tr2 TP;      pip-pic:     Tr1    TP    +    Tr2 TP
                BK  rec        :    main-pic:  Tr   TP     +     To  TP;  pip-pic:     Tm    TP    +    Tr   TP
                main(pip)&BK    :    main-pic:  Tr1 TP     +     Tr2 TP;      pip-pic:     Tr1    TP    +    Tr2 TP
*/
        if (SW_DMX_ID== main_screen_dmx_id)    // playbackmode without using ts_gen
        {
            //fourth, to create view
            if ((((tms_dmx_id == 0) || (record_dmx_id1 == 0) || (record_dmx_id2 == 0))
                && (tp_id1 != 0)
                && (pip_view_mode == 1)
                && (node_tp_id != tp_id1)
                && (node_tuner_id == nim_id1))
            || (((tms_dmx_id == 1) || (record_dmx_id1 == 1) || (record_dmx_id2 == 1))
                && (tp_id2 != 0)
                && (pip_view_mode == 1)
                && (node_tp_id != tp_id2)
                && (node_tuner_id == nim_id2)))
            {
                ret = FALSE;
            }
            else
            {
                ret = TRUE;
            }
        }
        else    //normal play + ts_gen
        {
            if ((tuner1_state != DEV_FRONTEND_STATUS_RECORD) && (tuner2_state != DEV_FRONTEND_STATUS_RECORD))   //Play
            {
                //deal with ts-gen playback and no record but has tms record!
                if (1 == pip_view_mode)
                {
                    if (ts_gen_enable)
                    {
                        if ( pvr_info->tms_r_handle != 0)
                        {
                            if ((node_tp_id == tp_id1) || (node_tp_id == tp_id2))
                            {
                                ret = TRUE;
                            }
                            else
                            {
                                ret = FALSE;
                            }
                        }
                        else
                        {
                            ret = TRUE;
                        }
                    }
                    else if ((node_tp_id != main_screen_tp_id) && (node_tuner_id == main_screen_tuner_id))
                    {
                        ret = FALSE;
                    }
                    else
                    {
                        ret = TRUE;
                    }
                }
                else
                {
                    ret = TRUE;
                }
            }
            else if (1 == pvr_info->rec_num) // 1 rec
            {
                if (1 == api_pvr_get_rec_num_by_dmx(main_screen_dmx_id))
                {
                    if (((pip_view_mode == 0)
                        && (node_tp_id != main_screen_tp_id)
                        && (node_tuner_id == main_screen_tuner_id))
                    || ((pip_view_mode == 1)
                       && (node_tp_id != main_screen_tp_id)
                       && (node_tuner_id == main_screen_tuner_id)))
                    {
                        ret = FALSE;
                    }
                    else
                    {
                        ret = TRUE;
                    }
                }
                else if (1 == api_pvr_get_rec_num_by_dmx(pip_screen_dmx_id))
                {
                    if (((pip_view_mode == 0)
                         && (node_tp_id != pip_screen_tp_id)
                         && (node_tuner_id == pip_screen_tuner_id))
                    || ((pip_view_mode == 1)
                         && (((node_tp_id != pip_screen_tp_id)
                              && ((playback_enable && ts_gen_enable)
                              || ((!playback_enable) && (node_tp_id != main_screen_tp_id))
                              || ((node_tuner_id == pip_screen_tuner_id)
                                 && (playback_enable) && (!ts_gen_enable))))
                         || ((node_prog_id != pvr_info->tms_chan_id)
                              && (NULL == api_pvr_get_rec_by_prog_id(node_prog_id, &rec_pos))
                              && ( pvr_info->tms_r_handle != 0)
                              && (playback_enable && ts_gen_enable)))))   // for TS gen
                    {
                        ret = FALSE;
                    }
                    else
                    {
                        ret = TRUE;
                    }
                }
                else if (main_screen_dmx_id == pip_screen_dmx_id)   // support main & pip at same dmx!
                {
                    if (pip_view_mode == 0)
                    {
                        if (1 == sys_data_get_scramble_record_mode())
                        {
                            if ((node_tp_id != ((main_screen_dmx_id == 0) ? tp_id2 : tp_id1))
                            && (node_tuner_id == (1 - main_screen_tuner_id)))
                            {
                                ret = FALSE;
                            }
                            else
                            {
                                ret = TRUE;
                            }
                        }
                        else    // for sramble mode
                        {
                            if ((node_tp_id != ((main_screen_dmx_id == 0) ? tp_id1 : tp_id2))
                            && (node_tuner_id == (main_screen_tuner_id)))
                            {
                                ret = FALSE;
                            }
                            else
                            {
                                ret = TRUE;
                            }
                        }
                    }
                    else
                    {
                        if (1 == sys_data_get_scramble_record_mode())
                        {
                            if (((node_tp_id == ((main_screen_dmx_id == 0) ? tp_id2 : tp_id1))
                                 || ((node_tuner_id == pip_screen_tuner_id)
                                    && ((playback_enable && ts_gen_enable)
                                       || ((!playback_enable) && (node_tp_id == main_screen_tp_id))))))
                            {
                                ret = TRUE;
                            }
                            else
                            {
                                ret = FALSE;
                            }
                        }
                        else    // for sramble mode
                        {
                            if (((node_tp_id == ((main_screen_dmx_id == 0) ? tp_id1 : tp_id2))
                                 || ((node_tuner_id == pip_screen_tuner_id)
                                    && ((playback_enable && ts_gen_enable)
                                       || ((!playback_enable) && (node_tp_id == main_screen_tp_id))))))
                            {
                                ret = TRUE;
                            }
                            else
                            {
                                ret = FALSE;
                            }
                        }
                    }
                }
            }
            else if (PVR_MAX_RECORDER == pvr_info->rec_num) // 2 rec
            {
                struct ts_route_info ts_route;
                MEMSET(&ts_route,0,sizeof(ts_route));

                if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route) != RET_SUCCESS)
                {
                    PRINTF("ts_route_get_by_type() failed!\n");
                }

                if (main_screen_dmx_id == pip_screen_dmx_id)
                {
                    if (0 == api_pvr_get_rec_num_by_dmx(main_screen_dmx_id))
                    {
                        if (pip_view_mode == 0)
                        {
                            // filter out diff TP and same Tuner with record TP!
                            if ((node_tp_id != ((main_screen_dmx_id == 0) ? tp_id2 : tp_id1))
                            && (node_tuner_id == pvr_r_get_nim_id( pvr_info->rec[0].record_handle)))
                            {
                                ret = FALSE;
                            }
                            else
                            {
                                ret = TRUE;
                            }
                        }

                        if (1 == pip_view_mode)
                        {
                            if ((node_tp_id == ((main_screen_dmx_id == 0) ? tp_id1 : tp_id2))
                            || (NULL != api_pvr_get_rec_by_prog_id(node_prog_id, &rec_pos)))
                            {
                                ret = TRUE;
                            }
                            else
                            {
                                ret = FALSE;
                            }
                        }
                    }
                    else if (1 == api_pvr_get_rec_num_by_dmx(main_screen_dmx_id))
                    {
                        if (((pip_view_mode == 0) && ((node_tp_id == tp_id1) || (node_tp_id == tp_id2)))
                        || ((pip_view_mode == 1)
                             && (((main_screen_dmx_id == 0)
                                   ? ((((ts_route.is_recording == 1) && (node_tp_id == tp_id1)))
                                     || (node_tp_id == tp_id2))
                                   : ((((ts_route.is_recording == 1) && (node_tp_id == tp_id2)))
                                     || (node_tp_id == tp_id1)))
                             || (NULL != api_pvr_get_rec_by_prog_id(node_prog_id, &rec_pos)))))
                        {
                            ret = TRUE;
                        }
                        else
                        {
                            ret = FALSE;
                        }
                    }
                    else if (PVR_MAX_RECORDER == api_pvr_get_rec_num_by_dmx(main_screen_dmx_id))
                    {
                        if ((node_tp_id != main_screen_tp_id) && (node_tuner_id == main_screen_tuner_id))
                        {
                            ret = FALSE;
                        }
                        else
                        {
                            ret = TRUE;
                        }
                    }
                }
                else
                {
                    if (PVR_MAX_RECORDER == api_pvr_get_rec_num_by_dmx(main_screen_dmx_id))
                    {
                        if (((pip_view_mode == 0)
                            && (node_tp_id != main_screen_tp_id)
                            && (node_tuner_id == main_screen_tuner_id))
                        || ((pip_view_mode == 1)
                           && (node_tp_id != main_screen_tp_id)
                           && (node_tuner_id == main_screen_tuner_id)))
                        {
                            ret = FALSE;
                        }
                        else
                        {
                            ret = TRUE;
                        }
                    }
                    else if (PVR_MAX_RECORDER == api_pvr_get_rec_num_by_dmx(pip_screen_dmx_id))
                    {
                        if (pip_view_mode == 0)
                        {
                            if ((node_tp_id != pip_screen_tp_id) && (node_tuner_id == pip_screen_tuner_id))
                            {
                                ret = FALSE;
                            }
                            else
                            {
                                ret = TRUE;
                            }
                        }

                        if (1 == pip_view_mode)
                        {
                            if (((node_tp_id == main_screen_tp_id) && (main_screen_tp_id != 0))
                            || (NULL != api_pvr_get_rec_by_prog_id(node_prog_id, &rec_pos)))
                            {
                                ret = TRUE;
                            }
                            else
                            {
                                ret = FALSE;
                            }
                        }
                    }
                    else if ((1 == api_pvr_get_rec_num_by_dmx(record_dmx_id1))
                         && (1 == api_pvr_get_rec_num_by_dmx(record_dmx_id2)))
                    {
                        // pip can not play same TP progs with main because main dmx already has 2 prog
                        // (1 record, and 1 main play, NOW not consident this 2 is same prog!)
                        if (((pip_view_mode == 0)
                             && ((node_tp_id == main_screen_tp_id) || (node_tp_id == pip_screen_tp_id)))
                        || ((pip_view_mode == 1)
                           && (((main_screen_dmx_id == 0)
                                ? ((((ts_route.is_recording == 1) && (node_tp_id == tp_id1)))
                                  || (node_tp_id == tp_id2))
                                : ((((ts_route.is_recording == 1) && (node_tp_id == tp_id2)))
                                  || (node_tp_id == tp_id1)))
                           || (NULL != api_pvr_get_rec_by_prog_id(node_prog_id, &rec_pos)))))
                        {
                            ret = TRUE;
                        }
                        else
                        {
                            ret = FALSE;
                        }
                    }
                }
            }
        }
    }
    else if (sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {
        if (((1 == pip_view_mode)
             && ((get_nim1_cfg_success
                  && (1 == nim_cfg.antenna.antenna_enable)
                  && (node_pol != nim_cfg.xpond.s_info.pol)
                  && (0 == main_screen_tuner_id))
                || (get_nim2_cfg_success
                   && (1 == nim_cfg2.antenna.antenna_enable)
                   && (node_pol != nim_cfg2.xpond.s_info.pol)
                   && (1 == main_screen_tuner_id))))
        || ((0 == pip_view_mode)
             && (((NULL != api_pvr_get_rec_by_dmx(0, 1))
                  && (1 == nim_cfg.antenna.antenna_enable)
                  && (((FRONTEND_TYPE_S == nim_cfg.xpond.common.type)
                      || (FRONTEND_TYPE_S == nim_cfg2.xpond.common.type))
                     ?(((0 == nim_id1)
                       ? (node_pol != nim_cfg.xpond.s_info.pol)
                       : (node_pol != nim_cfg2.xpond.s_info.pol)))
                     : 1))
               || ((NULL != api_pvr_get_rec_by_dmx(1, 1))
                    && (1 == nim_cfg2.antenna.antenna_enable)
                    && (((FRONTEND_TYPE_S == nim_cfg.xpond.common.type)
                        || (FRONTEND_TYPE_S == nim_cfg2.xpond.common.type))
                       ?(((0 == nim_id1)
                         ? (node_pol != nim_cfg.xpond.s_info.pol)
                         : (node_pol != nim_cfg2.xpond.s_info.pol)))
                       : 1)))))
        {
            ret = FALSE;
        }

        /*
            Play            :    main-pic: all;  pip-pic: all
            Main Rec        :    main-pic: all;      pip-pic: all
            Pip     Rec        :    main-pic: all;  pip-pic: TP
            mian&pip  rec    :    main-pic: TP;  pip-pic: TP
        */
        else if (((pip_view_mode == 0)
                  && ((node_tp_id == main_screen_tp_id)
                       || ((( pvr_info->rec_num < PVR_MAX_RECORDER)
                           || (api_pvr_get_rec_num_by_dmx(0) != 1)
                           || ((node_tp_id == tp_id1)
                           || (node_tp_id == tp_id2))))))
             || ((pip_view_mode == 1)
                  && (((!ts_gen_enable)
                       && (playback_enable)
                       && (( pvr_info->rec_num < PVR_MAX_RECORDER)
                          || (api_pvr_get_rec_num_by_dmx(0) != 1)
                          || (((node_tp_id == tp_id1) && (tp_id1 != 0))
                          || ((node_tp_id == tp_id2) && (tp_id2 != 0)))))
                     || ((ts_gen_enable)
                          && (playback_enable)
                          && ((( pvr_info->rec_num == 0)
                              && (((node_tp_id == tp_id1)&& (tp_id1 != 0))
                                 || ((node_tp_id == tp_id2) && (tp_id2 != 0))))
                             || (( pvr_info->rec_num == 1)
                                 && ((NULL != api_pvr_get_rec_by_prog_id(node_prog_id, NULL))
                                    || (( pvr_info->tms_r_handle != 0) && ( pvr_info->tms_chan_id == node_prog_id))
                                    || (( pvr_info->tms_r_handle == 0)
                                       && (((node_tp_id == tp_id1) && (tp_id1 != 0))
                                          || ((node_tp_id == tp_id2) && (tp_id2 != 0))))))))
                     || ((!playback_enable)
                        && ((node_tp_id == pip_screen_tp_id)
                           || ((node_tp_id != pip_screen_tp_id)
                              && (api_pvr_get_rec_num_by_dmx(pip_screen_dmx_id) == 0)))))))
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

RETURN:
    return ret;
}

INT32 api_pip_create_view(UINT8 mode, UINT16 __MAYBE_UNUSED__ flag)  // mode: 0--full screen view, 1--PIP screen view
{
    INT32                   ret = -1;
    struct nim_config       config;
    pip_db_info             filter_buf;
    INT32                   ts_route_id1 = 0;
    INT32                   ts_route_id2 = 0;
    INT32                   ts_route_id_pb = 0;
    struct ts_route_info    ts_route1;
    struct ts_route_info    ts_route2;
    struct ts_route_info    ts_route_pb;
    UINT8                   route_num = 0;
    UINT8                   i = 0;
    UINT16                  route_index[2] = { 0 };
    UINT32                  nim_sub_type = 0;
    UINT32                  nim2_sub_type = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&config, 0x0, sizeof(config));
    MEMSET(&filter_buf, 0x0, sizeof(filter_buf));
    MEMSET(&ts_route1, 0x0, sizeof(ts_route1));
    MEMSET(&ts_route2, 0x0, sizeof(ts_route2));
    MEMSET(&ts_route_pb, 0x0, sizeof(ts_route_pb));
    if (cur_view_type == (mode + 1))
    {   // same as last view, no need create again
        return 0;
    }

    if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id1, &ts_route1) != RET_SUCCESS)
    {
        PRINTF("ts_route_get_by_type() failed!\n");
    }

    ts_route_get_by_type(TS_ROUTE_PIP_PLAY, (UINT32 *)&ts_route_id2, &ts_route2);
    ts_route_get_by_type(TS_ROUTE_PLAYBACK, (UINT32 *)&ts_route_id_pb, &ts_route_pb);
    is_pip_view = (mode == 1) ? TRUE : FALSE;
    MEMSET(&filter_buf, 0, sizeof(filter_buf));
    pip_view_mode = mode;
    nim_sub_type = dev_get_device_sub_type(g_nim_dev, HLD_DEV_TYPE_NIM);
    if (nim_sub_type > FRONTEND_TYPE_ISDBT)
    {
        PRINTF("PLEASE check the type, it may be wrong!\n");
    }

    dev_get_device_status(HLD_DEV_TYPE_NIM, nim_sub_type, g_nim_dev, &tuner1_state);
    dev_get_nim_config(g_nim_dev, nim_sub_type, &config);
    tuner1_tp_id = config.xpond.s_info.tp_id;
    if (DUAL_TUNER == g_tuner_num)
    {
        nim2_sub_type = dev_get_device_sub_type(g_nim_dev2, HLD_DEV_TYPE_NIM);
        if (nim2_sub_type > FRONTEND_TYPE_ISDBT)
        {
            PRINTF("PLEASE check the type, it may be wrong!\n");
        }

        dev_get_device_status(HLD_DEV_TYPE_NIM, nim2_sub_type, g_nim_dev2, &tuner2_state);
        dev_get_nim_config(g_nim_dev2, nim2_sub_type, &config);
        tuner2_tp_id = config.xpond.s_info.tp_id;
    }

    if (ts_route1.enable)
    {
        main_screen_tuner_id = ts_route1.nim_id;
        main_screen_tuner_state = (main_screen_tuner_id == 0) ? tuner1_state : tuner2_state;
        main_screen_dmx_id = ts_route1.dmx_id;
        main_screen_tp_id = ts_route1.tp_id;
    }
    else if ((ts_route_pb.enable))  // ts_gen
    {
        main_screen_tuner_id = ts_route_pb.nim_id;
        main_screen_tuner_state = (main_screen_tuner_id == 0) ? tuner1_state : tuner2_state;
        main_screen_dmx_id = ts_route_pb.dmx_id;
        main_screen_tp_id = ts_route_pb.tp_id;
    }

    if (ts_route_pb.enable)
    {
        playback_enable = 1;
        if (ts_route_pb.dmx_id < SW_DMX_ID)
        {
            ts_gen_enable = 1;
        }
        else
        {
            ts_gen_enable = 0;
        }
    }
    else
    {
        playback_enable = 0;
        ts_gen_enable = 0;
    }

    if (ts_route2.enable)
    {
        pip_screen_tuner_id = ts_route2.nim_id;
        pip_screen_tuner_state = (pip_screen_tuner_id == 0) ? tuner1_state : tuner2_state;
        pip_screen_dmx_id = ts_route2.dmx_id;
        pip_screen_tp_id = ts_route2.tp_id;
    }
    else
    {
        MEMSET(&ts_route2, 0, sizeof(struct ts_route_info));
        ts_route_check_dmx_route(1 - main_screen_dmx_id, &route_num, route_index);
        if ((route_num > 0))
        {
            for (i = 0; i < route_num; i++)
            {
                //ts_route_get_by_id(route_index[i], &ts_route2);
                if (RET_FAILURE == ts_route_get_by_id(route_index[i], &ts_route2))
                {
                    PRINTF("ts_route_get_by_id() failed!\n");
                }

                if ((ts_route2.enable) && (ts_route2.is_recording) && (ts_route2.type == TS_ROUTE_BACKGROUND))
                {
                    pip_screen_tuner_id = ts_route2.nim_id;
                    pip_screen_tuner_state = (pip_screen_tuner_id == 0) ? tuner1_state : tuner2_state;
                    pip_screen_dmx_id = ts_route2.dmx_id;
                    pip_screen_tp_id = ts_route2.tp_id;
                    break;
                }
            }
        }
        else
        {
            pip_screen_tuner_id = main_screen_tuner_id;
            pip_screen_tuner_state = (pip_screen_tuner_id == 0) ? tuner1_state : tuner2_state;
            pip_screen_dmx_id = 1 - main_screen_dmx_id;
            pip_screen_tp_id = main_screen_tp_id;
        }
    }

    MEMSET(&nim_cfg, 0, sizeof(struct nim_config));
    MEMSET(&nim_cfg2, 0, sizeof(struct nim_config));
    get_nim1_cfg_success = (SUCCESS == dev_get_nim_config(g_nim_dev, nim_sub_type, &nim_cfg));
    get_nim2_cfg_success = (SUCCESS == dev_get_nim_config(g_nim_dev2, nim2_sub_type, &nim_cfg2));

    //libc_printf("nim1 pol %d, nim2 pol %d\n",nim_cfg.antenna.pol,nim_cfg2.antenna.pol);
    // first, to get dmx_id of tms, record
    tms_dmx_id = ( pvr_info->tms_r_handle != 0) ? (pvr_r_get_dmx_id( pvr_info->tms_r_handle)) : (-1);
    record_dmx_id1 = (pvr_info->rec[0].record_handle != 0) ? (pvr_r_get_dmx_id( pvr_info->rec[0].record_handle)):(-1);
    record_dmx_id2 = (pvr_info->rec[1].record_handle != 0) ? (pvr_r_get_dmx_id( pvr_info->rec[1].record_handle)):(-1);

    //second, to get tps of two dmxes
    struct dmx_config   dmx_cfg;
    struct dmx_config   dmx_cfg2;

    if (SUCCESS == dev_get_dmx_config(g_dmx_dev, dev_get_device_sub_type(g_dmx_dev, HLD_DEV_TYPE_DMX), &dmx_cfg))
    {
        tp_id1 = dmx_cfg.tp_id;
    }
    else
    {
        tp_id1 = 0;
    }

    if (SUCCESS == dev_get_dmx_config(g_dmx_dev2, dev_get_device_sub_type(g_dmx_dev2, HLD_DEV_TYPE_DMX), &dmx_cfg2))
    {
        tp_id2 = dmx_cfg2.tp_id;
    }
    else
    {
        tp_id2 = 0;
    }

    // third, to get nims of two dmxes
    ts_route_check_dmx_route(0, &route_num, route_index);
    if (route_num > 0)
    {
        MEMSET(&ts_route1, 0, sizeof(struct ts_route_info));

        //ts_route_get_by_id(route_index[0], &ts_route1);
        if (RET_FAILURE == ts_route_get_by_id(route_index[0], &ts_route1))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        //nim used by dmx0
        nim_id1 = ts_route1.nim_id;

    }
    else
    {
        nim_id1 = -1;
    }

    ts_route_check_dmx_route(1, &route_num, route_index);
    if (route_num > 0)
    {
        MEMSET(&ts_route2, 0, sizeof(struct ts_route_info));

        //ts_route_get_by_id(route_index[0], &ts_route2);
        if (RET_FAILURE == ts_route_get_by_id(route_index[0], &ts_route2))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        //nim used by dmx1
        nim_id2 = ts_route2.nim_id;

    }
    else
    {
        nim_id2 = -1;
    }

    //UINT32 ptm = osal_get_tick();
    ret = recreate_special_pip_view(TYPE_PROG_NODE, (UINT32*)api_check_view, (UINT32*)&filter_buf);
    api_pip_filter_view(sys_data_get_cur_chan_mode());
    cur_view_type = mode + 1;

    return ret;
}
#endif

#ifdef PIP_SUPPORT
void ap_play_chan_pip_check(UINT16 chan_idx, BOOL b_force, BOOL is_pip_chgch)
{
    SYSTEM_DATA *sys_data = NULL;
    INT32       retval = RET_FAILURE;
    T_NODE      t_node_n;
    T_NODE      t_node_m;
    sys_state_t system_state = SYS_STATE_INITIALIZING;
    P_NODE      p_node;
    P_NODE      playing_pnode;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route_bk;
    INT32                   ts_route_id = 0;

    get_prog_at(chan_idx, &p_node);
    system_state = api_get_system_state();
    ap_get_playing_pnode(&playing_pnode);

    if (sys_data_get_pip_support() && is_pip_chgch && b_force)
    {

        ap_pip_set_prog(&p_node);
        sys_data_set_pip_group_channel(chan_idx);
        retval = ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route);
        if ((retval != RET_FAILURE)
        && pvr_info->tms_r_handle
        && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) == (frontend.ts_route.dmx_id - 1))
        && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) != ts_route.dmx_id))
        {
            //stop tms record in pip dmx while there is main play!
            api_pvr_tms_proc(FALSE, FALSE);
        }
    }

    //for dual same
    sys_data = sys_data_get();
    if (sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME)
    {

        MEMSET(&t_node_n, 0, sizeof(T_NODE));
        MEMSET(&t_node_m, 0, sizeof(T_NODE));
        if ((SUCCESS == get_tp_by_id(p_node.tp_id, &t_node_n))
        && (SUCCESS == get_tp_by_id(playing_pnode.tp_id, &t_node_m))
        && (!is_pip_chgch)
        && (t_node_m.pol != t_node_n.pol)
        && (system_state == SYS_STATE_PIP))
        {
            ap_pip_exit();
            api_set_system_state(SYS_STATE_NORMAL);
        }
    }
}
#endif

