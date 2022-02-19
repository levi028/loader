/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_dual_ci.c
 *
 *    Description: This source file is DUAL CI application process.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/decv.h>
#include <api/libsi/sie_monitor.h>
#include <bus/tsi/tsi.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "menus_root.h"
#ifdef CI_SUPPORT
#include "win_ci_common.h"
#endif
#include "ap_ctrl_ci.h"
#include "ap_ctrl_dual_ci.h"
#include "ap_ctrl_display.h"


#define MAX_TS_ROUTE_PER_DMX    2
#define MAX_CI_MSG_DELAY_TIME   70000


typedef struct _ci_d_table
{
    UINT8   priority[2];
} CI_TABLE;

typedef enum _slot_priority
{
    CI_SLOT_PRI_INVALID = -1,
    CI_SLOT_PRI_0 = 0,
    CI_SLOT_PRI_1 ,
    CI_SLOT_PRI_2 ,
    CI_SLOT_PRI_3 ,
    CI_SLOT_PRI_4 ,
    CI_SLOT_PRI_5 ,
    CI_SLOT_PRI_6 ,
} CI_SLOT_PRI_E;
typedef struct _ci_slot_priority
{
    UINT32  prog_id;
    UINT32  route_id;
    UINT8   dmx_id;
    INT8    s0_pri;
    INT8    s1_pri;
} slot_pri_t;

#if (defined CI_SUPPORT)
static CI_TABLE      g_ci_table[5];
INT8                 main_original_slot = -1;
INT8                 pip_original_slot = -1;
static INT8          pb_original_slot = -1;


static void priority_cacu(INT8 *hold_route_id, slot_pri_t *main, slot_pri_t *pip, slot_pri_t *pb, slot_pri_t *bg)
{
    UINT8 slot_id = 0;
    UINT8 route_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route2;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();

    for (slot_id = 0; slot_id <2; slot_id++)
    {
        for (route_id = 0; route_id < CC_MAX_TS_ROUTE_NUM; route_id++)
        {
            MEMSET(&ts_route, 0, sizeof(ts_route));

            if (RET_FAILURE == ts_route_get_by_id(route_id, &ts_route))
            {
                PRINTF("ts_route_get_by_id() failed!\n");
            }

            if (ts_route.enable != 1)
            {
                continue;
            }

            if (ts_route.ca_slot_ready[slot_id] != 0)
            {
                g_ci_table[route_id].priority[slot_id] += 1;
                if (ts_route.type == TS_ROUTE_MAIN_PLAY)
                {
                    g_ci_table[route_id].priority[slot_id] += 4;
                }
                if (((TS_ROUTE_PLAYBACK == ts_route.type)) && (ts_route.dmx_id < SW_DMX_ID))
                {
                    g_ci_table[route_id].priority[slot_id] += 4;  //Ts_gen playback
                }
                g_ci_table[route_id].priority[slot_id] += 2 * (ts_route.type == TS_ROUTE_PIP_PLAY);
                if (((slot_id == 0) ? (ts_route.cia_used == 1) : (ts_route.cib_used == 1)))
                {
                    g_ci_table[route_id].priority[slot_id] += 1;

                    hold_route_id[slot_id] = route_id;
                }
            }
            else
            {
                g_ci_table[route_id].priority[slot_id] = 0;
                if ((slot_id == 0) ? (ts_route.cia_used == 1) : (ts_route.cib_used == 1))
                {
                    hold_route_id[slot_id] = route_id;
                }
            }

            CI2_PRINTF(" %d ", g_ci_table[route_id].priority[slot_id]);
            if (ts_route.type == TS_ROUTE_MAIN_PLAY)
            {
                main->route_id = route_id;
                main->prog_id = ts_route.prog_id;
                main->dmx_id = ts_route.dmx_id;

                // For scramble mode
                if (!((sys_data_get_scramble_record_mode() == 0) && (api_pvr_get_rec_num_by_dmx(main->dmx_id) != 0)))
                {
                    if ((slot_id == 0))
                    {
                        main->s0_pri= g_ci_table[route_id].priority[slot_id];
                    }
                    else
                    {
                        main->s1_pri = g_ci_table[route_id].priority[slot_id];
                    }
                }
            }
            else if (ts_route.type == TS_ROUTE_PIP_PLAY)
            {
                pip->dmx_id = ts_route.dmx_id;
                MEMSET(&ts_route2, 0, sizeof(ts_route2));
                if (RET_SUCCESS != ts_route_get_by_type(TS_ROUTE_BACKGROUND, NULL, &ts_route2))
                {
                    PRINTF("ts_route_get_by_type() failed!\n");
                }

                if (sys_data_get_scramble_record_mode() == 0)
                {
                    if (( pvr_info->rec_num == 0)
                    || (( pvr_info->rec_num != 0)
                       && (ts_route.is_recording == 0)
                       && (ts_route2.dmx_id != ts_route.dmx_id))
                    || (( pvr_info->rec_num != 0)
                       && (ts_route.is_recording == 1)
                       && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route2) != RET_FAILURE)
                       && (ts_route.prog_id == ts_route2.prog_id)))
                    {
                        pip->route_id = route_id;
                        pip->prog_id = ts_route.prog_id;
                        if ((slot_id == 0))
                        {
                            pip->s0_pri = g_ci_table[route_id].priority[slot_id];
                        }
                        else
                        {
                            pip->s1_pri = g_ci_table[route_id].priority[slot_id];
                        }
                    }
                }
                else    //free mode
                {
                    pip->route_id = route_id;
                    pip->prog_id = ts_route.prog_id;
                    if ((slot_id == 0))
                    {
                        pip->s0_pri = g_ci_table[route_id].priority[slot_id];
                    }
                    else
                    {
                        pip->s1_pri = g_ci_table[route_id].priority[slot_id];
                    }
                }
            }
            else if ((TS_ROUTE_PLAYBACK == ts_route.type) && (ts_route.dmx_id < SW_DMX_ID)) //TS_gen
            {
                pb->route_id = route_id;
                pb->prog_id = ts_route.prog_id;
                if ((slot_id == 0))
                {
                    pb->s0_pri = g_ci_table[route_id].priority[slot_id];
                }
                else
                {
                    pb->s1_pri = g_ci_table[route_id].priority[slot_id];
                }
            }
            else if ((ts_route.type == TS_ROUTE_BACKGROUND))
            {
                bg->route_id = route_id;
            }
        }

        CI2_PRINTF("\n");
    }
}

static void select_ci_retry(slot_pri_t *main, slot_pri_t *pip, slot_pri_t *pb, slot_pri_t *bg)
{
    struct io_param param;
    UINT16          PID[2];
    P_NODE          p_node;
    RET_CODE        dmx_state;
    UINT8           scramble_type;
    UINT8           scrm_flag = 0;
    INT8            temp_priority = 0;
    struct list_info play_pvr_info;

    api_get_play_pvr_info(&play_pvr_info);
    // main_pic
    if ((main->route_id != INVALID_ID) || (pip->route_id != INVALID_ID))
    {
        if (main->prog_id != 0)
        {
            get_prog_by_id(main->prog_id, &p_node);
            PID[0] = p_node.video_pid;
            PID[1] = p_node.audio_pid[p_node.cur_audio];
        }
        else if (pb->route_id != INVALID_ID) //playback
        {
            PID[0] = play_pvr_info.pid_v;
            PID[1] = play_pvr_info.pid_a;
        }

        MEMSET(&param, 0, sizeof(param));
        param.io_buff_in = (UINT8 *) &PID;
        param.io_buff_out = (UINT8 *) &scramble_type;
        dmx_state = RET_FAILURE;
        if (0 == main->dmx_id)
        {
            dmx_state = dmx_io_control(g_dmx_dev, IS_AV_SCRAMBLED_EXT, ((UINT32) & param));
        }
        else
        {
            dmx_state = dmx_io_control(g_dmx_dev2, IS_AV_SCRAMBLED_EXT, ((UINT32) & param));
        }
        CI2_PRINTF("%s dmx_state %d, scramble_type %d\n", __FUNCTION__, dmx_state, scramble_type);

        scrm_flag = (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID);
        if ((dmx_state == RET_SUCCESS) && ((scramble_type & scrm_flag) != 0x00))
        {
            temp_priority = main->s0_pri;
            main->s0_pri = main->s1_pri;
            main->s1_pri = temp_priority;
        }
    }

    //pip_pic
    if (pip->route_id != INVALID_ID)
    {
        if ((pip->prog_id != 0) && (get_prog_by_id(pip->prog_id, &p_node) == DB_SUCCES))
        {
            PID[0] = p_node.video_pid;
            PID[1] = p_node.audio_pid[p_node.cur_audio];
            MEMSET(&param, 0, sizeof(param));
            param.io_buff_in = (UINT8 *) &PID;
            param.io_buff_out = (UINT8 *) &scramble_type;
            dmx_state = RET_FAILURE;
            if (pip->dmx_id == 0)
            {
            dmx_state = dmx_io_control(g_dmx_dev, IS_AV_SCRAMBLED_EXT, ((UINT32) & param));
            }
            else
            {
            dmx_state = dmx_io_control(g_dmx_dev2, IS_AV_SCRAMBLED_EXT, ((UINT32) & param));
            }
            CI2_PRINTF("%s dmx_state %d, scramble_type %d\n", __FUNCTION__, dmx_state, scramble_type);
            scrm_flag = (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID);
            if ((dmx_state == RET_SUCCESS) && ((scramble_type & scrm_flag) != 0x00))
            {
                temp_priority = pip->s0_pri;
                pip->s0_pri = pip->s1_pri;
                pip->s1_pri = temp_priority;
            }
        }
    }
}

RET_CODE api_select_2ci(INT8 *need_ci_route_id, INT8 *hold_ci_route_id, BOOL retry)
{
#ifndef _BUILD_OTA_E_
    UINT8 record_mode = 0; // 0: record to scramble ; 1: record to fta mode
    UINT8 slot_id = 0;
    UINT8 route_id = 0;
    INT8  pri_max = 0;
    INT8  temp_id = 0;
    INT8  p = 0;
    pvr_record_t    *rec;
    slot_pri_t      main;
    slot_pri_t      pip;
    slot_pri_t      pb; // playback
    slot_pri_t      bg; // background
    RET_CODE        dmx_state;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route2;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    CI2_PRINTF("g_ci_table:\n");
    MEMSET(&main, 0xff, sizeof(slot_pri_t));
    MEMSET(&pip, 0xff, sizeof(slot_pri_t));
    MEMSET(&pb, 0xff, sizeof(slot_pri_t));
    MEMSET(&bg, 0xff, sizeof(slot_pri_t));
    main.dmx_id = 0;
    pip.dmx_id = 0;
    priority_cacu(hold_ci_route_id, &main, &pip, &pb, &bg);

    if (main.route_id == INVALID_ID)
    {
        main.route_id = pb.route_id;
        main.s0_pri = pb.s0_pri;
        main.s1_pri = pb.s1_pri;
    }

    if (retry)
    {
        select_ci_retry(&main, &pip, &pb, &bg);
    }

    // add for both play
    if (pip.dmx_id == 0)
    {
        dmx_state = dmx_io_control(g_dmx_dev, IS_BOTH_SLOT_PLAY, 0);
    }
    else
    {
        dmx_state = dmx_io_control(g_dmx_dev2, IS_BOTH_SLOT_PLAY, 0);
    }
    if (RET_SUCCESS == dmx_state)
    {
        if ((main.s0_pri != CI_SLOT_PRI_0) || (main.s1_pri != CI_SLOT_PRI_0))
        {
            pip.s0_pri = CI_SLOT_PRI_0;
            pip.s1_pri = CI_SLOT_PRI_0;
        }
    }

    if ((CI_SLOT_PRI_5 == main.s0_pri) && (CI_SLOT_PRI_5 == main.s1_pri))
    {
        if ((main.prog_id == pvr_info->rec[0].record_chan_id) && (main.prog_id != 0))
        {
            ( pvr_info->rec[0].ci_id == 0) ? (main.s0_pri++) : (main.s1_pri++);
        }
        else if ((main.prog_id == pvr_info->rec[1].record_chan_id) && (main.prog_id != 0))
        {
            ( pvr_info->rec[1].ci_id == 0) ? (main.s0_pri++) : (main.s1_pri++);
        }
    }

    if ((CI_SLOT_PRI_3 == pip.s0_pri) && (CI_SLOT_PRI_3== pip.s1_pri))
    {
        if ((pip.prog_id == pvr_info->rec[0].record_chan_id) && (pip.prog_id != 0))
        {
            ( pvr_info->rec[0].ci_id == 0) ? (pip.s0_pri++) : (pip.s1_pri++);
        }
        else if ((pip.prog_id == pvr_info->rec[1].record_chan_id) && (pip.prog_id != 0))
        {
            ( pvr_info->rec[1].ci_id == 0) ? (pip.s0_pri++) : (pip.s1_pri++);
        }
    }

    if ((CI_SLOT_PRI_6 == main.s0_pri) || (CI_SLOT_PRI_6 == main.s1_pri))         //(6,x)&(x,6)
    {
        if (CI_SLOT_PRI_6 == main.s0_pri)
        {
            need_ci_route_id[0] = main.route_id;
            if (main.prog_id != pip.prog_id)
            {
                if (pip.s1_pri > CI_SLOT_PRI_0)
                {
                    need_ci_route_id[1] = pip.route_id;
                }
                else
                {
                     need_ci_route_id[1] = hold_ci_route_id[1];
                }
            }
        }
        else
        {
            need_ci_route_id[1] = main.route_id;
            if (main.prog_id != pip.prog_id)
            {
                if (pip.s0_pri > CI_SLOT_PRI_0)
                {
                    need_ci_route_id[0] = pip.route_id;
                }
                else
                {
                    need_ci_route_id[0] = hold_ci_route_id[0];
                }
            }
        }
    }
    else if ((CI_SLOT_PRI_5 == main.s0_pri) || (CI_SLOT_PRI_5 == main.s1_pri))    //(5,5)&(5,x)&(x,5)
    {
        if ((CI_SLOT_PRI_5 == main.s0_pri) && (CI_SLOT_PRI_5 == main.s1_pri))     //(5,5)
        {
            CI2_PRINTF("Same card!\n");
            if (pb.route_id == INVALID_ID)
            {
                if ((CI_SLOT_PRI_INVALID == pip.s0_pri)
                && (CI_SLOT_PRI_INVALID== pip.s1_pri)) //whatever background exist ( M )
                {
                    if (-1 == main_original_slot)
                    {
                        //CI2_PRINTF("( M )\n");
                        if (-1 == hold_ci_route_id[0])
                        {
                            need_ci_route_id[0] = main.route_id;
                            need_ci_route_id[1] = hold_ci_route_id[1];
                        }
                        else
                        {
                            need_ci_route_id[1] = main.route_id;
                            need_ci_route_id[0] = hold_ci_route_id[0];
                        }
                    }
                    else
                    {
                        if (-1 == bg.route_id)
                        {
                            CI2_PRINTF("( M )\n");
                            if (main_original_slot == 0)
                            {
                                need_ci_route_id[0] = main.route_id;
                                if (pip.s1_pri > CI_SLOT_PRI_0)
                                {
                                    need_ci_route_id[1] = pip.route_id;
                                }
                                else
                                {
                                    need_ci_route_id[1] = hold_ci_route_id[1];
                                }
                            }
                            else if (1 == main_original_slot)
                            {
                                need_ci_route_id[1] = main.route_id;
                                if (pip.s0_pri > CI_SLOT_PRI_0)
                                {
                                    need_ci_route_id[0] = pip.route_id;
                                }
                                else
                                {
                                    need_ci_route_id[0] = hold_ci_route_id[0];
                                }
                            }
                        }
                        else
                        {
                            CI2_PRINTF(" ( M+BG )\n");

                            record_mode = sys_data_get_scramble_record_mode();
                            rec = api_pvr_get_rec(1);
                            if ((rec != NULL)
                            && (((rec->ci_id != main_original_slot) && (record_mode == 1))
                               || ((rec->ci_id == main_original_slot) && (record_mode == 0))))
                            {
                                if (main_original_slot == 0)
                                {
                                    need_ci_route_id[0] = main.route_id;
                                    if (pip.s1_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[1] = pip.route_id;
                                    }
                                    else
                                    {
                                         need_ci_route_id[1] = hold_ci_route_id[1];
                                    }
                                }
                                else if (1 == main_original_slot)
                                {
                                    need_ci_route_id[1] = main.route_id;
                                    if (pip.s0_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[0] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[0] = hold_ci_route_id[0];
                                    }
                                }
                            }
                            else
                            {
                                if (main_original_slot == 0)
                                {
                                    need_ci_route_id[1] = main.route_id;
                                    if (pip.s0_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[0] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[0] = hold_ci_route_id[0];
                                    }
                                }
                                else if (1 == main_original_slot)
                                {
                                    need_ci_route_id[0] = main.route_id;
                                    if (pip.s1_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[1] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[1] = hold_ci_route_id[1];
                                    }
                                }
                            }
                        }
                    }
                }
                else if ((pip.s0_pri != CI_SLOT_PRI_INVALID)
                     || (pip.s1_pri != CI_SLOT_PRI_INVALID))    //(M + PIP)
                {
                    if (-1 == main_original_slot)
                    {
                        CI2_PRINTF("(M + PIP)\n");
                        if ((CI_SLOT_PRI_4 == pip.s0_pri))
                        {
                            need_ci_route_id[0] = pip.route_id;
                            need_ci_route_id[1] = main.route_id;
                        }
                        else if (CI_SLOT_PRI_4 == pip.s1_pri)
                        {
                            need_ci_route_id[1] = pip.route_id;
                            need_ci_route_id[0] = main.route_id;
                        }
                        else if ((CI_SLOT_PRI_3 == pip.s0_pri) || (CI_SLOT_PRI_3 == pip.s1_pri))
                        {
                            need_ci_route_id[0] = main.route_id;
                            if (main.prog_id != pip.prog_id)
                            {
                                if (pip.s0_pri > CI_SLOT_PRI_0)
                                {
                                    need_ci_route_id[1] = pip.route_id;
                                }
                                else
                                {
                                    need_ci_route_id[1] = hold_ci_route_id[1];
                                }
                            }
                        }
                    }
                    else
                    {
                        if (-1 == bg.route_id)  //(M + PIP )
                        {
                            CI2_PRINTF("(M + PIP)\n");
                            if (main_original_slot == 0)
                            {
                                need_ci_route_id[0] = main.route_id;
                                if (pip.s1_pri > CI_SLOT_PRI_0)
                                {
                                    need_ci_route_id[1] = pip.route_id;
                                }
                                else
                                {
                                    need_ci_route_id[1] = hold_ci_route_id[1];
                                }
                            }
                            else if (1 == main_original_slot)
                            {
                                need_ci_route_id[1] = main.route_id;
                                if (pip.s0_pri > CI_SLOT_PRI_0)
                                {
                                    need_ci_route_id[0] = pip.route_id;
                                }
                                else
                                {
                                    need_ci_route_id[0] = hold_ci_route_id[0];
                                }
                            }
                        }
                        else    //(M + PIP + BG) ( M+BG )
                        {
                            CI2_PRINTF("(M + PIP+ BG)\n");
                            rec = api_pvr_get_rec(1);
                            record_mode = sys_data_get_scramble_record_mode();
                            if ((rec != NULL)
                            && (((rec->ci_id != main_original_slot) && (record_mode == 1))
                                || ((rec->ci_id == main_original_slot) && (record_mode == 0))))
                            {
                                if (main_original_slot == 0)
                                {
                                    need_ci_route_id[0] = main.route_id;
                                    if (pip.s1_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[1] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[1] = hold_ci_route_id[1];
                                    }
                                }
                                else if (1 == main_original_slot)
                                {
                                    need_ci_route_id[1] = main.route_id;
                                    if (pip.s0_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[0] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[0] = hold_ci_route_id[0];
                                    }
                                }
                            }
                            else
                            {
                                if (main_original_slot == 0)
                                {
                                    need_ci_route_id[1] = main.route_id;
                                    if (pip.s0_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[0] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[0] = hold_ci_route_id[0];
                                    }
                                }
                                else if (1 == main_original_slot)
                                {
                                    need_ci_route_id[0] = main.route_id;
                                    if (pip.s1_pri > CI_SLOT_PRI_0)
                                    {
                                        need_ci_route_id[1] = pip.route_id;
                                    }
                                    else
                                    {
                                        need_ci_route_id[1] = hold_ci_route_id[1];
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else    // for playback mode
            {
                if (-1 == bg.route_id)  // menu_playback  ( PB )
                {
                    CI2_PRINTF("( PB )\n");
                    if (-1 == pb_original_slot)
                    {
                        if (-1 == hold_ci_route_id[0])
                        {
                            need_ci_route_id[0] = pb.route_id;
                            need_ci_route_id[1] = hold_ci_route_id[1];
                        }
                        else
                        {
                            need_ci_route_id[1] = pb.route_id;
                            need_ci_route_id[0] = hold_ci_route_id[0];
                        }
                    }
                    else
                    {
                        if (pb_original_slot == 0)
                        {
                            need_ci_route_id[0] = pb.route_id;
                            need_ci_route_id[1] = hold_ci_route_id[1];
                        }
                        else
                        {
                            need_ci_route_id[1] = pb.route_id;
                            need_ci_route_id[0] = hold_ci_route_id[0];
                        }
                    }
                }
                else
                {
                    // reference main_original_slot
                    if (-1 == pb_original_slot)
                    {
                        pb_original_slot = main_original_slot;
                    }

                    if (-1 == pb_original_slot)
                    {
                        if (-1 == hold_ci_route_id[0])
                        {
                            need_ci_route_id[0] = pb.route_id;
                            need_ci_route_id[1] = hold_ci_route_id[1];
                        }
                        else
                        {
                            need_ci_route_id[1] = pb.route_id;
                            need_ci_route_id[0] = hold_ci_route_id[0];
                        }
                    }
                    else if ((CI_SLOT_PRI_INVALID == pip.s0_pri)
                         && (CI_SLOT_PRI_INVALID == pip.s1_pri))
                    {
                        if ((pb.route_id != -1)
                        && ( pvr_info->tms_chan_id == pb.prog_id)
                        && (pb.prog_id != 0))   // for tms tsgen plack (PB + BG)
                        {
                            CI2_PRINTF("( PB + BG(tms) )\n");
                            if (hold_ci_route_id[0] != -1)
                            {
                                // tms playback use original ci
                                need_ci_route_id[0] = pb.route_id;

                                // to judge orininal ts route use other ci
                                need_ci_route_id[1] = -1;
                                if (g_ci_table[hold_ci_route_id[0]].priority[1] > 0)
                                {
                                    need_ci_route_id[1] = hold_ci_route_id[0];
                                }
                            }
                            else if (hold_ci_route_id[1] != -1)
                            {
                                need_ci_route_id[1] = pb.route_id;
                                need_ci_route_id[0] = -1;
                                if (g_ci_table[hold_ci_route_id[1]].priority[0] > 0)
                                {
                                    need_ci_route_id[0] = hold_ci_route_id[1];
                                }
                            }
                        }
                        else    // info playback & menu playback (PB + BG)
                        {
                            CI2_PRINTF("( PB + BG (info + menu))\n");
                            rec = api_pvr_get_rec(1);
                            record_mode = sys_data_get_scramble_record_mode();
                            if ((rec != NULL)
                            && (((rec->ci_id != pb_original_slot) && (record_mode == 1)) /*record to free mode*/
                               || ((rec->ci_id == pb_original_slot) && (record_mode == 0))))/*record scramble mode*/
                            {
                                if (pb_original_slot == 0)
                                {
                                    need_ci_route_id[0] = pb.route_id;
                                    (pip.s1_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[1] = pip.route_id)
                                        : (need_ci_route_id[1] = hold_ci_route_id[1]);
                                }
                                else if (1 == pb_original_slot)
                                {
                                    need_ci_route_id[1] = pb.route_id;
                                    (pip.s0_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[0] = pip.route_id)
                                        : (need_ci_route_id[0] = hold_ci_route_id[0]);
                                }
                            }
                            else
                            {
                                if (pb_original_slot == 0)
                                {
                                    need_ci_route_id[1] = pb.route_id;
                                    (pip.s0_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[0] = pip.route_id)
                                        : (need_ci_route_id[0] = hold_ci_route_id[0]);
                                }
                                else if (1 == pb_original_slot)
                                {
                                    need_ci_route_id[0] = pb.route_id;
                                    (pip.s1_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[1] = pip.route_id)
                                        : (need_ci_route_id[1] = hold_ci_route_id[1]);
                                }
                            }
                        }
                    }
                    else    //PB + PIP + BG
                    {
                        CI2_PRINTF("( PB + PIP + BG)\n");
                        rec = api_pvr_get_rec(1);
                        record_mode = sys_data_get_scramble_record_mode();
                        if ((rec != NULL)
                        && (((rec->ci_id != pb_original_slot) && (record_mode == 1))
                           || ((rec->ci_id == pb_original_slot) && (record_mode == 0))))
                        {
                            if (pb_original_slot == 0)
                            {
                                need_ci_route_id[0] = pb.route_id;
                                (pip.s1_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[1] = pip.route_id)
                                    : (need_ci_route_id[1] = hold_ci_route_id[1]);
                            }
                            else if (1 == pb_original_slot)
                            {
                                need_ci_route_id[1] = pb.route_id;
                                (pip.s0_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[0] = pip.route_id)
                                    : (need_ci_route_id[0] = hold_ci_route_id[0]);
                            }
                        }
                        else
                        {
                            if (0 == pb_original_slot)
                            {
                                need_ci_route_id[1] = pb.route_id;
                                (pip.s0_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[0] = pip.route_id)
                                    : (need_ci_route_id[0] = hold_ci_route_id[0]);
                            }
                            else if (1 == pb_original_slot)
                            {
                                need_ci_route_id[0] = pb.route_id;
                                (pip.s1_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[1] = pip.route_id)
                                    : (need_ci_route_id[1] = hold_ci_route_id[1]);
                            }
                        }
                    }
                }
            }
        }
        else if (CI_SLOT_PRI_5 == main.s0_pri)
        {
            need_ci_route_id[0] = main.route_id;
            (pip.s1_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[1] = pip.route_id)
                : (need_ci_route_id[1] = hold_ci_route_id[1]);
        }
        else if (CI_SLOT_PRI_5 == main.s1_pri)
        {
            need_ci_route_id[1] = main.route_id;
            (pip.s0_pri > CI_SLOT_PRI_0) ? (need_ci_route_id[0] = pip.route_id)
                : (need_ci_route_id[0] = hold_ci_route_id[0]);
        }
    }
    else
    {
        if ((CI_SLOT_PRI_4 == pip.s0_pri) //(4,x)
        || ((pip.s0_pri == CI_SLOT_PRI_3) && (pip.s1_pri == CI_SLOT_PRI_0)))   //(3,0)
        {
            need_ci_route_id[0] = pip.route_id;
            need_ci_route_id[1] = hold_ci_route_id[1];
        }
        else if ((CI_SLOT_PRI_4 == pip.s1_pri)
             || ((CI_SLOT_PRI_0 == pip.s0_pri) && (CI_SLOT_PRI_3 == pip.s1_pri)))      //(x,4)&(0,3)
        {
            need_ci_route_id[1] = pip.route_id;
            need_ci_route_id[0] = hold_ci_route_id[0];
        }
        else if ((CI_SLOT_PRI_3 == pip.s0_pri) && (CI_SLOT_PRI_3 == pip.s1_pri))  //(3,3)
        {
            rec = api_pvr_get_rec(1);
            if (-1 == pip_original_slot)
            {
                if ((NULL == rec) || ((-1 == rec->ci_id) && (NULL != rec)))
                {
                    need_ci_route_id[0] = pip.route_id;
                    need_ci_route_id[1] = hold_ci_route_id[1];
                }
                else
                {
                    if (rec->ci_id == 0)
                    {
                        need_ci_route_id[0] = hold_ci_route_id[0];
                        need_ci_route_id[1] = pip.route_id;
                    }
                    else    //(rec->ci_ic == 1)
                    {
                        need_ci_route_id[0] = pip.route_id;
                        need_ci_route_id[1] = hold_ci_route_id[1];
                    }
                }
            }
            else
            {
                record_mode = sys_data_get_scramble_record_mode();
                if ((rec != NULL)
                && (((rec->ci_id != pip_original_slot) && (record_mode == 1))
                   || ((rec->ci_id == pip_original_slot) && (record_mode == 0))))
                {
                    if (pip_original_slot == 0)
                    {
                        need_ci_route_id[0] = pip.route_id;
                        need_ci_route_id[1] = hold_ci_route_id[1];
                    }
                    else if (1 == pip_original_slot)
                    {
                        need_ci_route_id[1] = pip.route_id;
                        need_ci_route_id[0] = hold_ci_route_id[0];
                    }
                }
                else        // (pip_original_slot != -1)
                {
                    if (0 == pip_original_slot)
                    {
                        need_ci_route_id[1] = pip.route_id;
                        need_ci_route_id[0] = hold_ci_route_id[0];
                    }
                    else if (1 == pip_original_slot)
                    {
                        need_ci_route_id[0] = pip.route_id;
                        need_ci_route_id[1] = hold_ci_route_id[1];
                    }
                }
            }
        }
        else    // for main and pip both dont use CI                    //(0,0)
        {
            need_ci_route_id[0] = hold_ci_route_id[0];
            need_ci_route_id[1] = hold_ci_route_id[1];
        }
    }

    CI2_PRINTF("slot 0: need_ci_route_id %d, hold_ci_route_id %d\n", need_ci_route_id[0], hold_ci_route_id[0]);
    CI2_PRINTF("slot 1: need_ci_route_id %d, hold_ci_route_id %d\n", need_ci_route_id[1], hold_ci_route_id[1]);
#endif
}

void ap_2ci_message_proc(UINT32 msg_type, UINT32 msg_code)
{
#ifndef _BUILD_OTA_E_
    INT8                    pri_max,
                            temp_id,
                            p,
                            main_route_id = -1,
                            pip_route_updated,
                            pip_dmx_id,
                            pip_route_id = -1,
                            pb_route_id = -1,
                            bg_route_id = -1,
                            need_ci_route_id[2],
                            hold_ci_route_id[2];

    UINT32                  main_prog_id = 0,
                            pip_prog_id = 0;
    UINT8                   dmx_route_num;
    UINT16                  dmx_routes[2];
    BOOL                    start_pip_vdec = TRUE;
    struct ts_route_info    ts_route,
                            ts_route2,
                            ts_route3;
    P_NODE                  p_node;
    RET_CODE                ret;
    UINT8                   ts_id,
                            dmx_id = 0,
                            tsi_id,
                            ci_mode,
                            slot_id;
    BOOL                    real_operate_ci,
                            retry;
    pvr_record_t            *rec;
    UINT32                  delay_array[] = { 5000, 5000, 6000, 6000, 16000, 16000, 64000, 64000, 70000, 70000 };
    static UINT8            index = 0;
    UINT8                   scramble_type;
    UINT8                   scrm_flag = 0;
    RET_CODE                dmx_state = RET_FAILURE;
    UINT8                   m_r_slot = 0,
                            p_r_slot = 0,
                            pb_r_slot = 0;
    UINT8                   record_mode = 0;
    struct dmx_device       *dmx = NULL;
    struct vdec_device      *decv = NULL;
    struct nim_device       *nim = NULL;
    struct list_info play_pvr_info;

    api_get_play_pvr_info(&play_pvr_info);
    ret = RET_SUCCESS;
    CI_PATCH_TRACE("Enter %s(%d)\n", __FUNCTION__, msg_code);

    // 1. non-delay msg
    // 2. if any prog use ci but not worked
    MEMSET(g_ci_table, 0, sizeof(g_ci_table));
    need_ci_route_id[0] = -1;
    need_ci_route_id[1] = -1;
    hold_ci_route_id[0] = -1;
    hold_ci_route_id[1] = -1;
    if (ret == RET_SUCCESS)
    {
        if (((msg_code != OP_CI_DELAY_MSG) && (msg_code != NOT_OP_CI))
        || (delay_array[++index] >= MAX_CI_MSG_DELAY_TIME))
        {
            index = 0;
        }

        retry = 0;
        if ((delay_array[index] != delay_array[index - 1]) && (index > 0))
        {
            retry = 1;
            ts_route_make_ca_slot_info();

            //CI2_PRINTF("make_ca_slot_info\n");
        }

        api_select_2ci(need_ci_route_id, hold_ci_route_id, retry);
        if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route) != RET_FAILURE)
        {
            main_prog_id = ts_route.prog_id;
        }

        if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, NULL, &ts_route) != RET_FAILURE)
        {
            pip_prog_id = ts_route.prog_id;
        }

        if (ts_route_get_by_type(TS_ROUTE_PLAYBACK, NULL, &ts_route) != RET_FAILURE)
        {
            pb_route_id = ts_route.id;
        }

        pip_route_updated = 0;
        /*CONFIG_SLOT_NUM*/
        for (slot_id = 0; slot_id <2; slot_id++)
        {
            MEMSET(&ts_route, 0, sizeof(ts_route));
            rec = api_pvr_get_rec(1);
            record_mode = sys_data_get_scramble_record_mode();
            if (((-1 == need_ci_route_id[slot_id]) && (-1 == hold_ci_route_id[slot_id]))
            || ((1 == record_mode)
               && (rec != NULL)
               && (rec->ci_id == slot_id)
               && (need_ci_route_id[slot_id] == pb_route_id)))
            {
                continue;
            }

            MEMSET(&ts_route, 0, sizeof(ts_route));

            if (RET_FAILURE == ts_route_get_by_id(hold_ci_route_id[slot_id], &ts_route))
            {
                PRINTF("ts_route_get_by_id() failed!\n");
            }

            MEMSET(&ts_route2, 0, sizeof(ts_route2));

            //ts_route_get_by_id(need_ci_route_id[slot_id], &ts_route2);
            if (RET_FAILURE == ts_route_get_by_id(need_ci_route_id[slot_id], &ts_route2))
            {
                PRINTF("ts_route_get_by_id() failed!\n");
            }

            real_operate_ci = 1;
            //stop using ci
            if ((hold_ci_route_id[slot_id] != -1)
            && (!((ts_route.type == TS_ROUTE_PIP_PLAY) && (pip_route_updated))))  //update pip route once
            {
                // different route may use the same ci, not delete ci flag
                if (!((hold_ci_route_id[slot_id] != need_ci_route_id[slot_id])
                    && (need_ci_route_id[slot_id] != -1)
                    && (ts_route.prog_id == ts_route2.prog_id)))
                {
                    (slot_id == 0) ? (ts_route.cia_used = 0) : (ts_route.cib_used = 0);
                    ts_route_update(ts_route.id, &ts_route);
                    if (ts_route.type == TS_ROUTE_MAIN_PLAY)
                    {
                        m_r_slot = 0;
                    }
                    else if (ts_route.type == TS_ROUTE_PIP_PLAY)
                    {
                        p_r_slot = 0;
                    }

                    if ((real_operate_ci) && ((slot_id == 0)))
                    {
                        api_operate_ci(0, ((ts_route.cib_used << 1) | ts_route.cia_used),
                                      ts_route.dmx_sim_info[0].sim_id, NULL, NULL, 0, 0, 0);
                    }
                }
            }

            //start to use ci
            if ((need_ci_route_id[slot_id] != -1)
            && (!((ts_route2.type == TS_ROUTE_PIP_PLAY) && (pip_route_updated)))) //update pip route once
            {
                MEMSET(&p_node, 0, sizeof(p_node));
                get_prog_by_id(ts_route2.prog_id, &p_node);
                if (ts_route2.ca_slot_ready[slot_id] != 0)
                {
                    if (sys_data_get_ci_mode() == CI_SERIAL_MODE)
                    {
                        if (slot_id == 0)
                        {
                            ts_route2.cia_used = 1;
                        }
                        else
                        {
                            ts_route2.cib_used = 1;
                        }

                        ts_route2.cia_included = 1;
                        ts_route2.cib_included = 1;
                        ts_route2.ts_id = TSI_TS_A;
                        ts_route2.state |= TS_ROUTE_STATUS_USE_CI_PATCH;
                        g_cam_ci_info[slot_id].ts_id = TSI_TS_A;
                    }
                    else    // paralllel mode
                    if ((main_prog_id == pip_prog_id) && (pip_prog_id != 0))
                    {
                        MEMSET(&ts_route3, 0, sizeof(ts_route3));
                        if (ts_route2.type == TS_ROUTE_PIP_PLAY)
                        {
                            if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route3) != RET_SUCCESS)
                            {
                                PRINTF("ts_route_get_by_type() failed!\n");
                            }
                        }
                        else if (ts_route2.type == TS_ROUTE_MAIN_PLAY)
                        {
                            if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, NULL, &ts_route3) != RET_SUCCESS)
                            {
                                PRINTF("ts_route_get_by_type() failed!\n");
                            }
                        }

                        struct io_param     param;
                        UINT16              PID[2];
                        P_NODE              p_node;

                        get_prog_by_id(ts_route3.prog_id, &p_node);
                        PID[0] = p_node.video_pid;
                        PID[1] = p_node.audio_pid[p_node.cur_audio];
                        MEMSET(&param, 0, sizeof(param));
                        param.io_buff_in = (UINT8 *) &PID;
                        param.io_buff_out = (UINT8 *) &scramble_type;

                        dmx = g_dmx_dev2;
                        if (ts_route2.dmx_id == 0)
                        {
                            dmx = g_dmx_dev;
                        }
                        dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, ((UINT32) & param));

                        scrm_flag = VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL
                                    |AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID;
                        if ((ts_route3.enable) && ((dmx_state == RET_FAILURE)
                        || ((dmx_state == RET_SUCCESS) && (scramble_type & scrm_flag == 0x00))))
                        {
                            ts_route2.ts_id = ts_route3.ts_id;
                        }
                        else
                        {
                            ts_route2.ts_id = (slot_id == 0) ? TSI_TS_A : TSI_TS_B;
                            ts_route_check_dmx_route(ts_route3.dmx_id, &dmx_route_num, dmx_routes);

                            if ((TS_ROUTE_PIP_PLAY == ts_route3.type) && (dmx_route_num < MAX_TS_ROUTE_PER_DMX))
                            {
                                ts_route3.ts_id = ts_route2.ts_id;
                                ts_route3.cia_used = (ts_route3.ts_id == TSI_TS_A) ? 1 : 0;
                                ts_route3.cia_included = (ts_route3.ts_id == TSI_TS_A) ? 1 : 0;
                                ts_route3.cib_used = (ts_route3.ts_id == TSI_TS_A) ? 0 : 1;
                                ts_route3.cib_included = (ts_route3.ts_id == TSI_TS_A) ? 0 : 1;
                                ts_route_update(ts_route3.id, &ts_route3);
                                pip_route_updated = 1;
                                pip_original_slot = ts_route3.ts_id - 1;
                                p_r_slot = ts_route3.ts_id;
                            }
                        }
                    }
                    else
                    {
                        ts_route2.ts_id = (slot_id == 0) ? TSI_TS_A : TSI_TS_B;
                    }

                    if (sys_data_get_ci_mode() != CI_SERIAL_MODE)
                    {
                        ts_route2.cia_used = (ts_route2.ts_id == TSI_TS_A) ? 1 : 0;
                        ts_route2.cia_included = (ts_route2.ts_id == TSI_TS_A) ? 1 : 0;
                        g_cam_ci_info[slot_id].ts_id = ts_route2.ts_id;
                        ts_route2.cib_used = (ts_route2.ts_id == TSI_TS_A) ? 0 : 1;
                        ts_route2.cib_included = (ts_route2.ts_id == TSI_TS_A) ? 0 : 1;
                    }

                    ts_route_update(ts_route2.id, &ts_route2);
                    if (ts_route2.type == TS_ROUTE_MAIN_PLAY)
                    {
                        main_original_slot = ts_route2.ts_id - 1;
                        m_r_slot = ts_route2.ts_id;
                    }
                    else if (ts_route2.type == TS_ROUTE_PIP_PLAY)
                    {
                        pip_original_slot = ts_route2.ts_id - 1;
                        p_r_slot = ts_route2.ts_id;
                    }
                    else if (ts_route2.type == TS_ROUTE_PLAYBACK)
                    {
                        pb_original_slot = ts_route2.ts_id - 1;
                        pb_r_slot = ts_route2.ts_id;
                    }

                    //check TS, but not operate ci
                    api_check_ts_by_dmx(ts_route2.dmx_id, ts_route2.ts_id, ts_route2.tp_id, TRUE, TRUE, FALSE);
                    api_check_ts_by_dmx(1 - ts_route2.dmx_id, ts_route2.ts_id, ts_route2.tp_id, FALSE, TRUE, FALSE);
                    if (real_operate_ci)
                    {
                        if (sys_data_get_ci_mode() == CI_SERIAL_MODE)
                        {
#ifdef CC_USE_TSG_PLAYER
                            if (ts_route2.type != TS_ROUTE_PLAYBACK)
                            {
                                tsi_select(TSI_TS_B, ts_route_get_nim_tsiid(0));
                                tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
                            }

#else
                            tsi_select(TSI_TS_A, ts_route2.tsi_id);
                            tsi_dmx_src_select(((ts_route2.dmx_id == 0) ? (TSI_DMX_0) : (TSI_DMX_1)), TSI_TS_A);
#endif

                            CI2_PRINTF("Start ci slot %d <-> ts %d \n", slot_id, ts_route2.id);
                            if ((TS_ROUTE_PLAYBACK == ts_route2.type) && (ts_route2.dmx_id < SW_DMX_ID))    //TSG gen
                            {
                                dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route2.dmx_id);
                                api_operate_ci(1, ((ts_route2.cib_used << 1) | ts_route2.cia_used),
                                               ts_route2.dmx_sim_info[0].sim_id, NULL, dmx,
                                               play_pvr_info.index, play_pvr_info.pid_v, play_pvr_info.pid_a);
                            }
                            else
                            {
                                dmx = dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route2.nim_id);
                                api_operate_ci(1, ((ts_route2.cib_used << 1) | ts_route2.cia_used),
                                               ts_route2.dmx_sim_info[0].sim_id, dmx,
                                               dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route2.dmx_id),
                                               ts_route2.prog_id, p_node.video_pid,
                                               p_node.audio_pid[p_node.cur_audio]);
                            }
                        }
                        else        // paralllel mode
                        {
                            UINT8   new_ts_id;

                            if ((ts_route2.tsi_id == ts_route_get_nim_tsiid(0))
                            || (ts_route2.tsi_id == ts_route_get_nim_tsiid(1)))
                            {
                                new_ts_id = ts_route2.tsi_id;
                            }
                            else
                            {
                                new_ts_id = TSI_SPI_TSG;
                            }

                            ts_id = TSI_TS_B;
                            if (slot_id == 0)
                            {
                                ts_id = TSI_TS_A;
                            }
                            tsi_select(ts_id, new_ts_id);
                            dmx_id = TSI_DMX_1;
                            if (ts_route2.dmx_id == 0)
                            {
                                dmx_id = TSI_DMX_0;
                            }
                            tsi_dmx_src_select(dmx_id, ts_route2.ts_id);

                            decv = g_decv_dev2;
                            if (ts_route2.type == TS_ROUTE_MAIN_PLAY)
                            {
                                decv = g_decv_dev;
                            }
                            if ((ts_route2.dmx_id < SW_DMX_ID)
                            && (ts_route.enable)
                            && (ts_route2.enable)
                            && (!((ts_route.prog_id == ts_route2.prog_id)
                                 && (hold_ci_route_id[slot_id] == need_ci_route_id[slot_id]))))
                            {
                                vdec_io_control( decv, VDEC_IO_REST_VBV_BUF, 0);
                            }

                            scrm_flag = VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL
                                        |AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID;
                            if ((main_prog_id == pip_prog_id)
                            && (pip_prog_id != 0)
                            && (ts_route3.enable)
                            && (ts_route3.type == TS_ROUTE_PIP_PLAY)
                            && ((dmx_state == RET_SUCCESS) && (scramble_type & scrm_flag != 0x00)))
                            {
                                dmx_id = TSI_DMX_1;
                                if (ts_route3.dmx_id == 0)
                                {
                                    dmx_id = TSI_DMX_0;
                                }
                                tsi_dmx_src_select(dmx_id, ts_route3.ts_id);

                                decv = g_decv_dev2;
                                if (ts_route3.type == TS_ROUTE_MAIN_PLAY)
                                {
                                    decv = g_decv_dev;
                                }
                                if (ts_route3.dmx_id < SW_DMX_ID)
                                {
                                    vdec_io_control( decv, VDEC_IO_REST_VBV_BUF, 0);
                                }
                            }

                            //CI2_PRINTF("Start ci slot %d <-> ts %d \n",slot_id,ts_route2.id);
                            if ((TS_ROUTE_PLAYBACK == ts_route2.type) && (ts_route2.dmx_id < SW_DMX_ID))
                            {   //TSG gen
                                dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route2.dmx_id);
                                api_operate_ci(1, ((ts_route2.cib_used << 1) + ts_route2.cia_used),
                                               ts_route2.dmx_sim_info[0].sim_id, NULL, dmx,
                                               play_pvr_info.index, play_pvr_info.pid_v, play_pvr_info.pid_a);
                            }
                            else
                            {
                                nim = dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route2.nim_id);
                                dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route2.dmx_id);
                                api_operate_ci(1, ((ts_route2.cib_used << 1) + ts_route2.cia_used),
                                               ts_route2.dmx_sim_info[0].sim_id, nim, dmx, ts_route2.prog_id,
                                               p_node.video_pid, p_node.audio_pid[p_node.cur_audio]);
                            }
                        }
                    }

                    if (ts_route2.type == TS_ROUTE_PLAYBACK)
                    {
                        set_ci_delay_msg(osal_get_tick(), delay_array[0]);
                        CI2_PRINTF("delay time %d\n", delay_array[index]);
                    }
                    else
                    {
                        set_ci_delay_msg(osal_get_tick(), delay_array[index]);
                        CI2_PRINTF("delay time %d\n", delay_array[index]);
                    }
                }
            }
        }

        if ((-1 == need_ci_route_id[0]) && (-1 == need_ci_route_id[1]))
        {
            stop_ci_delay_msg();
            CI2_PRINTF("stop send delay MSG \"CTRL_MSG_SUBTYPE_CMD_OPERATE_CI\"\n");
        }
    }
    else
    {
        stop_ci_delay_msg();
        CI2_PRINTF("stop send delay MSG \"CTRL_MSG_SUBTYPE_CMD_OPERATE_CI\"\n");
    }

#ifdef PIP_SUPPORT
    tsi_check_dmx_src(1, &ts_id, &tsi_id, &ci_mode);
    CI2_PRINTF("dmx 0, ts %d, tsi %d, ci %d\n", ts_id, tsi_id, ci_mode);
    tsi_check_dmx_src(2, &ts_id, &tsi_id, &ci_mode);
    CI2_PRINTF("dmx 1, ts %d, tsi %d, ci %d\n", ts_id, tsi_id, ci_mode);
#endif
#endif
    CI_PATCH_TRACE("Exit %s(%d)\n", __FUNCTION__, msg_code);
}
#endif

