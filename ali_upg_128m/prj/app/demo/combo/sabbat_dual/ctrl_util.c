/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_util.c
 *
 *    Description: This source file contains some application layer's utility
      functions.
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
#include <bus/tsi/tsi.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/scart/scart.h>
#include <hld/ge/ge.h>
#include <api/libsi/sie_monitor.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
#include <bus/usb2/usb.h>
#include <hld/dsc/dsc.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/lib_epg.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_frontend.h>
#include <api/libchunk/chunk.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "ctrl_util.h"
#include "ap_ctrl_ci.h"

static UINT8        main_dmx_id = 0;
static P_NODE       s_playing_pnode ;
static sys_state_t  sys_state = SYS_STATE_INITIALIZING;
static sys_state_t  sys_state_bak = SYS_STATE_INITIALIZING;
static BOOL         b_display_blacked = FALSE;

void ap_get_playing_pnode(P_NODE *pnode)
{
    if (NULL != pnode)
    {
	    MEMCPY(pnode ,&s_playing_pnode,sizeof(P_NODE));
    }
}

void ap_set_playing_pnode(P_NODE *pnode)
{
    if (NULL != pnode)
    {
        MEMCPY(&s_playing_pnode, pnode, sizeof(P_NODE));
    }
}

INT32 get_nim_id(P_NODE *p_r_node)
{
    INT32   ret = 0;
    S_NODE  s_node;

    if (NULL == p_r_node)
    {
        return RET_FAILURE;
    }
    MEMSET(&s_node, 0, sizeof(S_NODE));
    get_sat_by_id(p_r_node->sat_id, &s_node);
    ret = s_node.tuner1_valid ? 1 : (s_node.tuner2_valid ? 2 : 1);
    return ret;
}

BOOL dm_get_vdec_running(void)
{
    struct vdec_status_info  cur_status;

    MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32) & cur_status);
    if ((cur_status.u_cur_status != VDEC_DECODING))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL api_is_playing_tv(void)
{
    if (SCREEN_BACK_VIDEO == screen_back_state)
    {
        if (VIEW_MODE_FULL == hde_get_mode())
        {
            return TRUE;
        }

        if (VIEW_MODE_PREVIEW == hde_get_mode())
        {
            return TRUE;
        }
    }

    return FALSE;
}

void ap_set_main_dmx(BOOL pip_chgch, UINT8 dmx_id)
{
    struct dmx_device   *dmx = NULL;
    BOOL                is_swapped = FALSE;

#ifdef PIP_SUPPORT
    is_swapped = ap_get_pip_swapped();
#endif
    if ((is_swapped && (0 == dmx_id)) || ((!is_swapped) && (1 == dmx_id)))
    {
        if (pip_chgch)
        {
            dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
            osal_task_dispatch_off();
            dmx_io_control(dmx, DMX_PLAY_IN_MAIN_PIC, TRUE);
            dmx = NULL; // reset var
            if (0 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
            }
            else// if (1 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            }

            dmx_io_control(dmx, DMX_PLAY_IN_MAIN_PIC, FALSE);
            osal_task_dispatch_on();
        }
        else
        {
            if (0 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);;
            }
            else// if (1 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            }

            osal_task_dispatch_off();
            dmx_io_control(dmx, DMX_PLAY_IN_MAIN_PIC, FALSE);

            dmx = NULL; // reset var
            if (0 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            }
            else// if (1 == dmx_id)
            {
                dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
            }

            dmx_io_control(dmx, DMX_PLAY_IN_MAIN_PIC, TRUE);
            osal_task_dispatch_on();
        }

        is_swapped = !is_swapped;
#ifdef PIP_SUPPORT
        ap_set_pip_swapped(is_swapped);
#endif
    }

    main_dmx_id = dmx_id;
    return;
}

UINT8 ap_get_main_dmx(void)
{
    return main_dmx_id;
}

#ifndef _BUILD_OTA_E_
BOOL api_check_nim(UINT8 nim_id, UINT32 tp_id)
{
    BOOL                ret = TRUE;
    struct nim_device   *nim = NULL;
    struct nim_config   nim_cfg;
    UINT32              tuner_state = DEV_FRONTEND_STATUS_PLAY;
    UINT32              tuner_tp_id = 0;
    UINT32              nim_sub_type = 0;

    MEMSET(&nim_cfg, 0, sizeof(struct nim_config));

    if (0 == nim_id)
    {
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    }
    else if (1 == nim_id)
    {
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
    }

    if (nim)
    {
        nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        if (nim_sub_type > FRONTEND_TYPE_ISDBT)
        {
            PRINTF("PLEASE check the type, it may be wrong!\n");
        }

        dev_get_device_status(HLD_DEV_TYPE_NIM, nim_sub_type, nim, &tuner_state);
        if (FRONTEND_TYPE_S == nim_sub_type)
        {
            dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
            tuner_tp_id = nim_cfg.xpond.s_info.tp_id;
        }
        else if ((FRONTEND_TYPE_T == nim_sub_type) || (FRONTEND_TYPE_ISDBT == nim_sub_type))
        {
            dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
            tuner_tp_id = nim_cfg.xpond.t_info.tp_id;
        }
        else if (FRONTEND_TYPE_C == nim_sub_type)
        {
            dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
            tuner_tp_id = nim_cfg.xpond.c_info.tp_id;
        }
        else
        {
            tuner_tp_id = tp_id;
        }

        if ((DEV_FRONTEND_STATUS_RECORD == tuner_state) && (tuner_tp_id != tp_id))
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

BOOL api_check_dmx(UINT8 dmx_id, UINT32 tp_id, UINT32 prog_id)
{
    BOOL                ret = TRUE;
    struct dmx_device   *dmx = NULL;
    struct dmx_config   dmx_cfg;
    UINT32              dmx_state = DEV_DMX_STATUS_PLAY;
    UINT32              dmx_tp_id = 0;
    UINT8               dmx_rec_num = 0;
    pvr_record_t        *rec = NULL;
    P_NODE              p_node;
    UINT32              dmx_sub_type = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();

    MEMSET(&dmx_cfg, 0x0, sizeof(dmx_cfg));
    MEMSET(&p_node, 0x0, sizeof(p_node));
    if (0 == dmx_id)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    }
    else if (1 == dmx_id)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
    }

    if (dmx)
    {
        dmx_sub_type = dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX);
        dev_get_device_status(HLD_DEV_TYPE_DMX, dmx_sub_type, dmx, &dmx_state);
        dev_get_dmx_config(dmx, dmx_sub_type, &dmx_cfg);
        dmx_tp_id = dmx_cfg.tp_id;
        dmx_rec_num = api_pvr_get_rec_num_by_dmx(dmx_id);
        if ((dmx_tp_id != tp_id) && (DEV_DMX_STATUS_RECORD == dmx_state))
        {
            return FALSE;
        }
        else if ((dmx_rec_num > 1) && (DEV_DMX_STATUS_RECORD == dmx_state))
        {
            rec = api_pvr_get_rec_by_dmx(dmx_id, 1);
            if ((rec != NULL) && (rec->record_chan_id != prog_id))
            {
                rec = api_pvr_get_rec_by_dmx(dmx_id, 2);
                if ((rec != NULL) && (rec->record_chan_id != prog_id))
                {
                    return FALSE;
                }
            }
        }
        else
        {
            // check this dmx is tms record and other is idle or available(same TP),
            // than return false and use other dmx! And for BUG19395
            rec = api_pvr_get_rec_by_dmx(1 - dmx_id, 1);
            if (rec)
            {
                get_prog_by_id(rec->record_chan_id, &p_node);
            }

            if ( pvr_info->tms_r_handle
            && (pvr_r_get_dmx_id( pvr_info->tms_r_handle) == dmx_id)
            && ((NULL == rec)
               || ((1 == api_pvr_get_rec_num_by_dmx(1 - dmx_id)) && (p_node.tp_id == tp_id))
               || ((2 == api_pvr_get_rec_num_by_dmx(1 - dmx_id))
                  && (p_node.tp_id == tp_id)
                  && (api_pvr_get_rec_by_prog_id(prog_id, NULL) != NULL)))
            && ( pvr_info->play.play_handle != 0)
            && (pvr_p_get_dmx_id( pvr_info->play.play_handle) != (1 - dmx_id)))
            {
                return FALSE;
            }
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

static BOOL select_nim_dual_same(UINT8 *nim_id, UINT32 tp_id)
{
    INT32                   ts_route_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route_pip;
    struct nim_device   *nim = NULL;
    struct nim_config   nim_cfg;
    UINT32              tuner_state = DEV_FRONTEND_STATUS_PLAY;
    UINT32              tuner_tp_id = 0;
    UINT32              nim_sub_type = 0;

    if (NULL == nim_id)
    {
        return FALSE;
    }

    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    MEMSET(&ts_route_pip, 0, sizeof(struct ts_route_info));
    if (TRUE != api_check_nim(*nim_id, tp_id))
    {
        *nim_id = 1 -*nim_id;
        if (TRUE != api_check_nim(*nim_id, tp_id))
        {
            return FALSE;
        }
    }
    else
    {
    	//fix bug#33759 , #34354
        if (0 == *nim_id)
    	{
           nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
    	}
    	else if (1 == *nim_id)
    	{
           nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    	}
	if (nim)
    	{
           nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
           if (nim_sub_type > FRONTEND_TYPE_ISDBT)
           {
               PRINTF("PLEASE check the type, it may be wrong!\n");
           }

           dev_get_device_status(HLD_DEV_TYPE_NIM, nim_sub_type, nim, &tuner_state);
           if (FRONTEND_TYPE_S == nim_sub_type)
           {
               dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
               tuner_tp_id = nim_cfg.xpond.s_info.tp_id;
           }
           else if (FRONTEND_TYPE_T == nim_sub_type)
           {
               dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
               tuner_tp_id = nim_cfg.xpond.t_info.tp_id;
           }
           else if (FRONTEND_TYPE_C == nim_sub_type)
           {
               dev_get_nim_config(nim, nim_sub_type, &nim_cfg);
               tuner_tp_id = nim_cfg.xpond.c_info.tp_id;
           }
           else
           {
               tuner_tp_id = tp_id;
           }
           if ((DEV_FRONTEND_STATUS_RECORD == tuner_state) && (tuner_tp_id == tp_id))
           {
              *nim_id = 1 -*nim_id;
           }
       }
        // if main-pic's or pip-pic's tp is diff with the other's tp , he will change tuner
        if (((ts_route_get_by_type(TS_ROUTE_PIP_PLAY, (UINT32 *)&ts_route_id, &ts_route_pip) != RET_FAILURE)
             && (/*(!is_pip_chgch) && */(tp_id != ts_route_pip.tp_id) && (*nim_id == ts_route_pip.nim_id))))
        {
            *nim_id = 1 -*nim_id;
            if (TRUE != api_check_nim(*nim_id, tp_id))
            {
                // for two records and no main-pic, two nims should be checked
                if (ts_route.enable != 1)
                {
                    *nim_id = 1 -*nim_id;
                    if (TRUE != api_check_nim(*nim_id, tp_id))
                    {
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static BOOL select_nim_dual_diff(UINT8 *nim_id, UINT32 tp_id, BOOL tuner1_valid, BOOL tuner2_valid)
{
    UINT8   invalid_nim_id = 0xFF;

    if (NULL == nim_id)
    {
        return FALSE;
    }

    *nim_id = tuner1_valid ? 0 : (tuner2_valid ? 1 : invalid_nim_id);
    if (*nim_id == invalid_nim_id)
    {
        *nim_id = 0;        //this is just a trick ,just  try itself
    }

    if ((TRUE != api_check_nim(*nim_id, tp_id)))
    {
        *nim_id = 1 -*nim_id;
        if (TRUE != api_check_nim(*nim_id, tp_id))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL select_nim_single(UINT8 *nim_id, UINT32 tp_id, BOOL pip_chgch)
{
    if (NULL == nim_id)
    {
        return FALSE;
    }

    if (TRUE != api_check_nim(*nim_id, tp_id))
    {
        if (pip_chgch && (TRUE != api_check_nim(*nim_id, tp_id)))
        {
            *nim_id = 1 -*nim_id;
            if (TRUE != api_check_nim(*nim_id, tp_id))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL api_select_nim(BOOL pip_chgch, UINT32 tp_id, BOOL tuner1_valid, BOOL tuner2_valid, UINT8 *nim_id)
{
    UINT8                   temp_nimid = 0;
    INT32                   ts_route_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route_pip;
    SYSTEM_DATA             *sys_data = NULL;
    struct nim_device       *nim = NULL;
    T_NODE                  t_node;
    S_NODE                  s_node;
    UINT16                  ft_type_n = 0;
    BOOL                    ret = TRUE;

    if (NULL == nim_id)
    {
        return FALSE;
    }
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route_pip, 0x0, sizeof(ts_route_pip));
    MEMSET(&t_node, 0x0, sizeof(t_node));
    MEMSET(&s_node, 0x0, sizeof(s_node));
    sys_data = sys_data_get();
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
    {
        temp_nimid = ts_route.nim_id;
    }
    else
    {
        temp_nimid = 0;     // default is nim0
    }

    if (SUCCESS != get_tp_by_id(tp_id, &t_node))
    {
        return FALSE;
    }

#ifdef SAT2IP_CLIENT_SUPPORT
    if (SUCCESS != get_sat_by_id(t_node.sat_id, &s_node))
    {
        return FALSE;
    }
    else
    {
        if (1 == s_node.sat2ip_flag)
        {
            // because we have such a global array nim_tsiid[3],
            // and nim_tsiid[2] is just TSG...
            *nim_id = 2;    //3;
            return TRUE;
        }
    }
#endif
//Fix bug#33759 , #34354
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, temp_nimid);
    if (nim)
   {
       ft_type_n = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
       if (t_node.ft_type != ft_type_n)
       {
           temp_nimid = 1 - temp_nimid;
	    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, temp_nimid);
	    if (nim)
   	    {
       	ft_type_n = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
       	if (t_node.ft_type != ft_type_n)
		{
		    return FALSE;
   		}
           }
       }
   }
   else
   {
	return FALSE;
   }
   
#ifdef BC_PVR_SUPPORT
    UINT8   matched_route_num = 0;
    UINT16  matched_route_ids[4] = { 0 };
    INT32   i;
    ts_route_check_tp_route(tp_id, &matched_route_num, matched_route_ids);
    for (i = 0; i < matched_route_num; i++)
    {
        if (RET_FAILURE == ts_route_get_by_id((UINT32) matched_route_ids[i], &ts_route))
        {
            PRINTF("ts_route_get_by_id() failed!\n");
        }

        temp_nimid = ts_route.nim_id;
        break;
    }
#endif
    switch (sys_data->antenna_connect_type)
    {
    // 1. When recording, tp cannt be changed;
    // 2. pip-pic cannt change main-pic's tp when dual-same
    case ANTENNA_CONNECT_DUAL_SAME:
        ret = select_nim_dual_same(&temp_nimid, tp_id);
        break;

    case ANTENNA_CONNECT_DUAL_DIFF:
        ret = select_nim_dual_diff(&temp_nimid, tp_id, tuner1_valid, tuner2_valid);
        break;

    case ANTENNA_CONNECT_SINGLE:
    default:
        ret = select_nim_single(&temp_nimid, tp_id, pip_chgch);
        break;
    }

    *nim_id = temp_nimid;
    return ret;
}

#if (!defined(SUPPORT_TWO_TUNER) && defined(CC_USE_TSG_PLAYER))
static BOOL single_tuner_select_dmx(BOOL pip_chgch, BOOL bk_play, BOOL ca_mode, UINT32 prog_id,
                                 UINT32 tp_id, UINT8 nim_id, UINT8 *dmx_id, UINT8 *dmx_2play)
{
    UINT8   temp_dmxid = (TSI_DMX_0 - 1);

    if (api_check_dmx(temp_dmxid, tp_id, prog_id) != TRUE)
    {
        return FALSE;
    }

    *dmx_id = temp_dmxid;
    return TRUE;
}
#endif

static UINT8 api_get_scramble_rec_dmx(void)
{
    UINT8           ret = 0;
    pvr_record_t    *rec = NULL;

    rec = api_pvr_get_rec(1);
    if ((rec != NULL) && (!rec->ca_mode))
    {
        rec = api_pvr_get_rec(2);
    }

    if ((rec != NULL) && (rec->ca_mode) && (0 == sys_data_get_scramble_record_mode()))
    {
        ret = pvr_r_get_dmx_id(rec->record_handle);
    }
    else
    {
        ret = 0xFF;
    }

    return ret;
}

#ifdef PIP_SUPPORT
/**
*  add for mian&pip play same pip under ca mode for BUG18739
*  return:  1: dmx can use(return TRUE); 2: dmx cannot use(return FALSE)
*/
static BOOL select_dmx_pip_chchg_proc(UINT32 prog_id,UINT8 *temp_dmxid,UINT8 *dmx_2play,UINT32 tp_id)
{
    INT32                   ts_route_id = 0;
    struct ts_route_info    ts_route;
    SYSTEM_DATA             *sys_data = NULL;
    if(NULL == temp_dmxid)
    {
        return FALSE;
    }

    sys_data = sys_data_get();

    //pip change channel
    if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
    {
        *temp_dmxid = ts_route.dmx_id;
        if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
        {
            //check whether can use another dmx, only under dmx2 playback state  OR  same TP as main play!
            if (((ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
                 && (tp_id == ts_route.tp_id))
            || ((ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route) != RET_FAILURE)
               && ((ts_route.dmx_id == SW_DMX_ID) || (sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_SAME))))
            {
                *temp_dmxid = 1 - *temp_dmxid;
                if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                {
                    return FALSE;
                }

                dmx_io_control((*temp_dmxid == 1) ? g_dmx_dev : g_dmx_dev2, IO_STREAM_DISABLE, 0);
            }
        }

        if ((dmx_2play != NULL)
        && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        && (ts_route.dmx_id == *temp_dmxid))
        {
            // for exit both play, while other dmx is idle
            pvr_record_t    *rec = NULL;
            rec = api_pvr_get_rec_by_dmx((1 - (*temp_dmxid)), 1);
            if ((tp_id == ts_route.tp_id) && (rec != NULL))
            {
                *dmx_2play = 1;
            }
            else
            {
                *temp_dmxid = 1 - (*temp_dmxid);
                if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                {
                    return FALSE;
                }

                dmx_io_control((*temp_dmxid == 1) ? g_dmx_dev : g_dmx_dev2, IO_STREAM_DISABLE_EXT, 0);
            }
        }

        // such as playback from dmx patch state then pip chgch!!
        if ((ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        && (*temp_dmxid == ts_route.dmx_id)
        && (RET_SUCCESS == dmx_io_control((ts_route.dmx_id == 0)?g_dmx_dev : g_dmx_dev2, IS_BOTH_SLOT_PLAY, 0)))
        {
            *dmx_2play = 1;
        }
    }
    else            //pip first play! open channel
    {
        //use different dmx from main route!
        if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            *temp_dmxid = 1 - ts_route.dmx_id;
            if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
            {
                return FALSE;
            }
        }
        else if (ts_route_get_by_type(TS_ROUTE_PLAYBACK, &ts_route_id, &ts_route) != RET_FAILURE)
        {
            //open PIP on playback
            if (SW_DMX_ID == ts_route.dmx_id)
            {
                *temp_dmxid = 0;
                if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                {
                    *temp_dmxid = 1 - (*temp_dmxid);
                    if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                    {
                        return FALSE;
                    }
                }
            }
            else
            {
                *temp_dmxid = 1 - ts_route.dmx_id;
                if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                {
                    *temp_dmxid = 1 - (*temp_dmxid);
                    if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
                    {
                        return FALSE;
                    }
                }
            }
        }
        else
        {
            //no main route! error!
            *temp_dmxid = 0;
        }
    }

    return TRUE;
}

/**
* MAIN PIC chang channel cases!check whether need close PIP!
* 1: same tuner, different TP!
* 2: main screen chgch, same dmx with PIP, But with different nim(Dual same)!
* 3: before entering  BOTH_PLAY state when main pic chgch
* 4: After  leaving BOTH_PLAY state when main pic chgch
* 5: forbid entering Main + PIP(with rec) state!
*/
static void select_dmx_check_if_need_close_pip(UINT32 prog_id, UINT32 tp_id, BOOL is_pip_chgch)
{
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;

    MEMSET(&ts_route, 0x0, sizeof(ts_route));

    if (sys_data_get_pip_support()
    && (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
    && (!is_pip_chgch)
    && (((ts_route.nim_id == nim_id) && (ts_route.tp_id != tp_id))
       || ((ts_route.dmx_id == temp_dmxid) && ((ts_route.nim_id != nim_id)))
       || ((NULL != api_pvr_get_rec_num_by_dmx(ts_route.dmx_id))
          && (ts_route.dmx_id == temp_dmxid) && (!ts_route.is_recording))
       || ((ts_route.dmx_id == temp_dmxid) && (ts_route.tp_id == tp_id) && (!is_pip_use_ext))
       || ((ts_route.tp_id != tp_id) && (is_pip_use_ext))
       || ((prog_id == ts_route.prog_id) && (ts_route.is_recording))))
    {
        ap_pip_exit();
        api_set_system_state(SYS_STATE_NORMAL);
    }
}
#endif

#ifdef  CI_SUPPORT
static void select_dmx_check_ci_using(BOOL ca_mode, UINT32 prog_id, UINT32 tp_id, UINT8 *temp_dmxid)
{
    // 1. check ci using
    // 2. Parallel 2CI mode
    P_NODE  p_node;
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&p_node, 0, sizeof(p_node));

    get_prog_by_id(ts_route.prog_id, &p_node);
    if ((!api_pvr_check_dmx_canuseci(ca_mode, ts_route.dmx_id))
    || ((DUAL_CI_SLOT == g_ci_num)
       && (sys_data_get_scramble_record_mode() == 1)
       && ( pvr_info->rec_num == 1)
       && (ts_route.state & TS_ROUTE_STATUS_RECORD)
       && (p_node.ca_mode == 1)
       && (ca_mode == 1)))
    {
        // this dmx is can not pass CI!!
        *temp_dmxid = 1 - (*temp_dmxid);
    }

    RET_CODE ret_code = RET_FAILURE;

    ret_code = dmx_io_control((ts_route.dmx_id == 0) ? g_dmx_dev : g_dmx_dev2, IS_BOTH_SLOT_PLAY, 0);
    //check whether change back to background dmx!
    if ((ts_route_get_by_type(TS_ROUTE_BACKGROUND, &ts_route_id, &ts_route) != RET_FAILURE)
    && (ts_route.dmx_id != *temp_dmxid)
    && (ts_route.tp_id == tp_id)
    && (api_pvr_check_dmx_canuseci(ca_mode, ts_route.dmx_id))
    && ((SINGLE_CI_SLOT == g_ci_num)
       || (!((DUAL_CI_SLOT == g_ci_num)
          && (prog_id == ts_route.prog_id)
          && (sys_data_get_scramble_record_mode() == 1)))))
    {
        if ((ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) == RET_FAILURE)
        || (RET_SUCCESS == ret_code))
        {
            *temp_dmxid = ts_route.dmx_id;
        }
    }
}
#endif

static BOOL select_dmx_main_chchg_proc(UINT8 *temp_dmxid, BOOL ca_mode,UINT32 prog_id, UINT32 tp_id)
{
    SYSTEM_DATA             *sys_data = NULL;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route1;
    UINT8                   pos = 0;
#ifdef MULTI_DESCRAMBLE
    UINT32                  ts_route_id = 0;
#endif
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route1, 0x0, sizeof(ts_route1));
    sys_data = sys_data_get();
    if(NULL == temp_dmxid)
    {
        return FALSE;
    }

    #ifdef NEW_TIMER_REC    
    //Fill correct info to ts_route //vicky201410
    if (RET_FAILURE==ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, NULL, &ts_route))    
    {
        //return FALSE;
    }
    #endif
    
    if (ANTENNA_CONNECT_SINGLE == sys_data->antenna_connect_type)
    {
        if (DUAL_CI_SLOT == g_ci_num)
        {
            *temp_dmxid = api_get_scramble_rec_dmx();
            if ((*temp_dmxid != 0xFF))
            {
                if (ca_mode)
                {
                    *temp_dmxid = 1 - (*temp_dmxid);
                }
            }
            else
            {
                *temp_dmxid = ts_route.dmx_id;
            }
        }
        else    // original policy
        {
            // Single Mode use other dmx firstly, temporally
            MEMSET(&ts_route1, 0, sizeof(struct ts_route_info));
            if ((NULL == api_pvr_get_rec_by_prog_id(prog_id, &pos))
            && (1 == pvr_info->rec_num)
            && (api_pvr_check_dmx_canuseci(ca_mode, 1 - ts_route.dmx_id))
            && (api_pvr_get_rec_by_prog_id(ts_route.prog_id, &pos) != NULL)
            && (0 == sys_data_get_scramble_record_mode())
            && (ca_mode)
            && (ts_route.stream_ca_mode))
            {   // For free mode , to try other dmx; Scamble Mode, to try original dmx
                *temp_dmxid = 1 - ts_route.dmx_id;
            }
            else
            {
                *temp_dmxid = ts_route.dmx_id;
            }
        }
    }
    else
    {
        *temp_dmxid = ts_route.dmx_id;
#ifdef CI_SUPPORT
        // 1. check ci using; 2. Parallel 2CI mode
        select_dmx_check_ci_using(ca_mode, prog_id, tp_id, temp_dmxid);
#endif
#ifdef MULTI_DESCRAMBLE
        //check whether change back to background dmx!
        if ((ts_route_get_by_type(TS_ROUTE_BACKGROUND, &ts_route_id, &ts_route) != RET_FAILURE)
        && (ts_route.dmx_id != *temp_dmxid)
        && (ts_route.tp_id == tp_id))
        {
            *temp_dmxid = ts_route.dmx_id;
        }
#endif
    }

    if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
    {
#ifdef PIP_SUPPORT
        // leave dmx both play state when main chgch to other dmx while pip on
        if (RET_SUCCESS == dmx_io_control((*temp_dmxid == 0) ? g_dmx_dev : g_dmx_dev2, IS_BOTH_SLOT_PLAY, 0))
        {
            ap_pip_exit();
            api_set_system_state(SYS_STATE_NORMAL);
        }
#endif
        *temp_dmxid = 1 - (*temp_dmxid);
        if (TRUE != api_check_dmx(*temp_dmxid, tp_id, prog_id))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL select_dmx(BOOL __MAYBE_UNUSED__ pip_chgch, BOOL bk_play, BOOL ca_mode, UINT32 prog_id,
                      UINT32 tp_id, UINT8 __MAYBE_UNUSED__ nim_id, UINT8 *dmx_id, 
                      UINT8 * dmx_2play __MAYBE_UNUSED__ )
{
    UINT8                   temp_dmxid = 0;
    INT32                   ts_route_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route1;
    UINT8                   dmx_route_num = 0;
    UINT16                  dmx_routes[2] = {0};
    UINT16                  i = 0;
    UINT8                   retry_time = 0;
#ifdef PIP_SUPPORT
    BOOL                    is_pip_chgch = FALSE;
#endif
    P_NODE                  playing_pnode;
    UINT32                  ret = 0;
    pvr_play_rec_t  *pvr_info = NULL;

	if(NULL == pvr_info)
	{
		;
	}
	if (NULL == dmx_id)
    {
        return FALSE;
    }
    pvr_info  = api_get_pvr_info();

#ifdef PIP_SUPPORT
    is_pip_chgch = ap_pip_check_chgch();
#endif
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route1, 0x0, sizeof(ts_route1));
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
#ifdef PIP_SUPPORT
    //add for mian&pip play same pip under ca mode for BUG18739
    if (pip_chgch
        && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, &ts_route_id, &ts_route) != RET_FAILURE) 
        && (prog_id == playing_pnode.prog_id)
        && ( pvr_info->rec_num != 0)
        && (ts_route.stream_ca_mode == 1)
        && (recorder_check_scrambled())
        && (ts_route.stream_ca_mode == 1))
    {
        *dmx_id = ts_route.dmx_id;
        *dmx_2play = 1;

        return TRUE;
    }

    if (pip_chgch)
    {
        ret = select_dmx_pip_chchg_proc(prog_id, &temp_dmxid, dmx_2play, tp_id);
        if(!ret )
        {
            return FALSE;
        }
    }
    else
#endif
    {//main screen change channel
        if (!bk_play)   // main front chgch
        {

            //main change channel
            if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
            {
                ret = select_dmx_main_chchg_proc(&temp_dmxid, ca_mode,prog_id, tp_id);
                if(!ret)
                {
                    return FALSE;
                }
            }
            else    //first main play  OR  replay from playback! open channel
            {
                if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
                {
                    // policy: maintain PIP if can! so try other dmx first!
                    temp_dmxid = 1 - ts_route.dmx_id;
                    if ((TRUE != api_check_dmx(temp_dmxid, tp_id, prog_id))
                    || (TRUE != api_pvr_check_dmx_canuseci(ca_mode, temp_dmxid)))
                    {
                        temp_dmxid = ts_route.dmx_id;
                        if (TRUE != api_check_dmx(temp_dmxid, tp_id, prog_id))
                        {
                            return FALSE;
                        }
                    }
                }
                else
                {
                    // for 2ci back from menu select dmx has no rec, under rec free rec mode
                    if ((DUAL_CI_SLOT == g_ci_num)
                    && (ts_route_get_by_type(TS_ROUTE_BACKGROUND, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
                    && ((ts_route.is_recording)
                       && (ts_route.stream_av_mode)
                       && (ca_mode)
                       && (1 == sys_data_get_scramble_record_mode())))
                    {
#ifdef _CAS9_CA_ENABLE_
                        if (ts_route.prog_id == prog_id)
                        {
                            temp_dmxid = ts_route.dmx_id;
                        }
                        else
#endif
                        {
                            temp_dmxid = 1 - ts_route.dmx_id;
                        }
                    }
                    else
                    {
                        temp_dmxid = 0;
                    }

                    if (TRUE != api_check_dmx(temp_dmxid, tp_id, prog_id))
                    {
                        temp_dmxid = 1 - temp_dmxid;
                        if (TRUE != api_check_dmx(temp_dmxid, tp_id, prog_id))
                        {
                            return FALSE;
                        }
                    }
                }
            }

#ifdef PIP_SUPPORT // MAIN PIC chang channel cases! check whether need close PIP!
            select_dmx_check_if_need_close_pip(prog_id, tp_id, is_pip_chgch);
#endif
        }
        else        //main background play such as timer record play
        {
#ifdef PIP_SUPPORT
            //close pip first!!
            if (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
            {
                ap_pip_exit();
                api_set_system_state(SYS_STATE_NORMAL);
            }
#endif
            if (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
            {
                temp_dmxid = ts_route.dmx_id;
            }
            else
            {
                temp_dmxid = 0;
            }

            ret = FALSE;

            do
            {
                ts_route_check_dmx_route(temp_dmxid, &dmx_route_num, dmx_routes);
                if (1 == dmx_route_num)
                {
                    if ((ts_route_get_by_id(dmx_routes[0], &ts_route) != RET_FAILURE) && (tp_id != ts_route.tp_id))
                    {
                        temp_dmxid = 1 - temp_dmxid;
                        //retry_time++;
                        if (retry_time > 1)
                        {
                            return FALSE;//no enough rsc to use.
                        }
                    }
                }
                else
                {
                    for (i = 0; i < dmx_route_num; i++)
                    {
                        if (ts_route_get_by_id(dmx_routes[i], &ts_route) != RET_FAILURE)
                        {
                            if (ts_route.prog_id == prog_id)
                            {
                                ret = TRUE;
                                break;
                            }
                        }
                    }

                    if (i == dmx_route_num)
                    {
                        temp_dmxid = 1 - temp_dmxid;
                    }
                    if(ret)
                    {
                        break;//while
                    }
                }
            }while(retry_time++ < 1);
        }
    }

#if defined(COMBO_CA_PVR) || defined(BC_PVR_SUPPORT)
    temp_dmxid = nim_id;
#endif
    *dmx_id = temp_dmxid;
    return TRUE;
}

BOOL api_select_dmx(BOOL pip_chgch, BOOL bk_play, BOOL ca_mode, UINT32 prog_id,
                  UINT32 tp_id, UINT8 nim_id, UINT8 *dmx_id, UINT8 *dmx_2play)
{
    BOOL    ret = FALSE;

    if (NULL == dmx_id)
    {
        return ret;
    }

#if (!defined(SUPPORT_TWO_TUNER) && defined(CC_USE_TSG_PLAYER))
    ret = single_tuner_select_dmx(pip_chgch, bk_play, ca_mode, prog_id, tp_id, nim_id, dmx_id, dmx_2play);
#else
    ret = select_dmx(pip_chgch, bk_play, ca_mode, prog_id, tp_id, nim_id, dmx_id, dmx_2play);
#endif
    return ret;
}

/**
*   check_type:  TRUE: check ts_route->type, TS_ROUTE_BACKGROUND.
*                FALSE: don't check.
*/
static void api_check1ci_if_operate_ci(UINT8 ts_id, struct ts_route_info *ts_route,UINT16 id, BOOL check_type)
{
    UINT8 check_route_type = check_type? (ts_route->type!=TS_ROUTE_BACKGROUND):1;

    if ((TSI_TS_A == ts_id)&& (ts_route->cia_used||ts_route->cib_used)&& check_route_type)
    {
        api_operate_ci(0,((ts_route->cib_used << 1) + ts_route->cia_used),
                        ts_route->dmx_sim_info[0].sim_id, NULL, NULL, 0, 0, 0);
        ts_route->cia_used = 0;
        ts_route->cib_used = 0;
        ts_route_update(id, ts_route);
        api_pvr_change_record_mode(ts_route->prog_id);
    }
}

/**
*   check_type:  TRUE: check ts_route->type, TS_ROUTE_BACKGROUND.
*                FALSE: don't check.
*/
static void api_check2ci_if_operate_ci(UINT8 ts_id, struct ts_route_info *ts_route,
                                    UINT16 dmx_route_id, BOOL check_type)
{
    UINT8 check_route_type = check_type? (ts_route->type!=TS_ROUTE_BACKGROUND):1;

    if (NULL == ts_route)
    {
        return ;
    }
    if ((((TSI_TS_A == ts_id) && (ts_route->cia_used))||((TSI_TS_B == ts_id) && (ts_route->cib_used)))
        && check_route_type)
    {
        api_operate_ci(0, ((ts_route->cib_used << 1) + ts_route->cia_used),
            ts_route->dmx_sim_info[0].sim_id, NULL, NULL, ts_route->prog_id, 0, 0);
        (1 == ts_route->ts_id) ? (ts_route->cia_used = 0) : (ts_route->cib_used = 0);
        ts_route_update(dmx_route_id, ts_route);
        api_pvr_change_record_mode(ts_route->prog_id);
    }
}
// check this dmx TS route setting, adjust them
// (1: adjust TS; 2: ci service stop conditionally;
// 3: record change mode conditionally;
// 4: report error conditionally)!
BOOL api_check_ts_by_dmx(UINT8 dmx_id,UINT8 ts_id,UINT32 tp_id,BOOL is_goal_dmx,BOOL scramble,BOOL start_ci_service)
{
    BOOL                    ret = TRUE;
    UINT8                   dmx_route_num = 0;
    UINT16                  dmx_routes[2] = {0};
    UINT16                  i = 0;
    UINT8                   temp_tsid = 0;
    UINT32                  dmx_tp_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route1;

#if (!defined(SUPPORT_TWO_TUNER) && defined(CC_USE_TSG_PLAYER))
    return TRUE;
#else
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route1, 0x0, sizeof(ts_route1));
    ts_route_check_dmx_route(dmx_id, &dmx_route_num, dmx_routes);
    if (dmx_route_num > 0)
    {
        if (!is_goal_dmx)
        {
            dmx_tp_id = 0;
        }

        for (i = 0; i < dmx_route_num; i++)
        {
            if (RET_FAILURE == ts_route_get_by_id(dmx_routes[i], &ts_route))
            {
                ret = FALSE;
                break;
            }

            if (!is_goal_dmx)   //need check tp!
            {
                if (0 == dmx_tp_id)
                {
                    dmx_tp_id = ts_route.tp_id;
                }
                else
                {
                    if (ts_route.tp_id != dmx_tp_id)
                    {
                        ret = FALSE;
                        break;
                    }
                }
            }

            if (is_goal_dmx)
            {
                if (ts_route.ts_id != ts_id)
                {
                    if (ts_route.cia_used || ts_route.cib_used)
                    {
                        api_operate_ci(0, ((ts_route.cib_used << 1) + ts_route.cia_used),
                            ts_route.dmx_sim_info[0].sim_id, NULL, NULL, ts_route.prog_id, 0, 0);
                        if (SINGLE_CI_SLOT == g_ci_num)
                        {
                            ts_route.cia_used = 0;
                            ts_route.cib_used = 0;
                        }
                        else if (DUAL_CI_SLOT == g_ci_num)
                        {
                            (1 == ts_route.ts_id) ? (ts_route.cia_used = 0) : (ts_route.cib_used = 0);
                        }

                        api_pvr_change_record_mode(ts_route.prog_id);
                    }

                    ts_route.ts_id = ts_id;
                    ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
#ifdef CI_SUPPORT
                    if (CI_SERIAL_MODE == sys_data_get_ci_mode())
                    {
                        ts_route.cib_included = ts_route.cia_included;
                    }
                    else
                    {
                        ts_route.cib_included = ((TSI_TS_B == ts_route.ts_id) && (ts_route.ci_mode != 0));
                    }
#endif
                    tsi_dmx_src_select(ts_route.dmx_id + 1, ts_route.ts_id);
                    tsi_select(ts_route.ts_id, ts_route.tsi_id);
                    ts_route_update(dmx_routes[i], &ts_route);
                }
                else
                {
                    if (scramble && (TRUE == start_ci_service)) //CI prog
                    {
                        switch(g_ci_num)
                        {
                            case SINGLE_CI_SLOT: //only 1 ci slot
                                api_check1ci_if_operate_ci(ts_id, &ts_route,dmx_routes[i], TRUE);
                                break;
                            case DUAL_CI_SLOT: // 2 ci slot
                                api_check2ci_if_operate_ci(ts_id, &ts_route,dmx_routes[i], TRUE);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            else
            {
                if (((ts_route.ts_id == ts_id) && (tp_id) && (ts_route.tp_id != tp_id))
                || ((DUAL_CI_SLOT == g_ci_num)
                   && (ts_route.ts_id == ts_id)
                   && (ts_route.tp_id != tp_id)
                   && (ts_route_get_by_type(TS_ROUTE_PLAYBACK, NULL, &ts_route1) != RET_FAILURE)
                   && (ts_route1.dmx_id < 2)))
                {
                    //switch it's TS
                    ts_route.ts_id = 3 - ts_route.ts_id;
                    if (TS_ROUTE_BACKGROUND == ts_route.type)
                    {
                        ts_route.ts_id_recover = 1;
                    }

                    ts_route.cia_included = (TSI_TS_A == ts_route.ts_id);
#ifdef CI_SUPPORT
                    if (sys_data_get_ci_mode() == CI_SERIAL_MODE)
                    {
                        ts_route.cib_included = ts_route.cia_included;
                    }
                    else
                    {
                        ts_route.cib_included = ((ts_route.ts_id == TSI_TS_B) && (ts_route.ci_mode != 0));
                    }
#endif
                    tsi_dmx_src_select(ts_route.dmx_id + 1, ts_route.ts_id);
                    tsi_select(ts_route.ts_id, ts_route.tsi_id);
                    ts_route_update(dmx_routes[i], &ts_route);
                }

                if(scramble && (start_ci_service == TRUE))
                {
                    switch(g_ci_num)
                    {
                        case 1:
                            api_check1ci_if_operate_ci(ts_id, &ts_route, dmx_routes[i], FALSE);
                            break;
                        case 2:
                            api_check2ci_if_operate_ci(ts_id, &ts_route, dmx_routes[i], FALSE);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if ((!is_goal_dmx) && (ret))
        {
            //check whether some dmx to different TS
            temp_tsid = 0;
            for (i = 0; i < dmx_route_num; i++)
            {
                if (ts_route_get_by_id(dmx_routes[i], &ts_route) != RET_FAILURE)
                {
                    if (0 == temp_tsid)
                    {
                        temp_tsid = ts_route.ts_id;
                    }
                    else
                    {
                        if (ts_route.ts_id != temp_tsid)
                        {
                            ret = FALSE;
                            break;
                        }
                    }
                }
                else
                {
                    ret = FALSE;
                    break;
                }
            }
        }
    }

    return ret;
#endif
}

#ifdef CI_SUPPORT
// check if it is pip_chgchannel/main screen mode. if it is, it can not use ci.
static BOOL check_canuseci(BOOL pip_chgch, BOOL *ci_start_service,UINT32 tp_id, UINT8 *temp_tsid)
{
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    UINT32                  dmx_tp_id = 0;

    if ((NULL == ci_start_service) || (NULL == temp_tsid))
    {
        return FALSE;
    }
    MEMSET(&ts_route, 0x0, sizeof(ts_route));

    *ci_start_service = TRUE;
    dmx_tp_id = 0;
    if (pip_chgch)  //can not use CI only when pip_changechannel and main screen use CI
    {
        ts_route_id = ts_route_check_ci(0);
        if (ts_route_id != -1)
        {
            if (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
            {
                if ((ts_route.type != TS_ROUTE_PIP_PLAY) && (1 == ts_route.stream_ca_mode))
                {
                    dmx_tp_id = ts_route.tp_id;
                    *ci_start_service = FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
    }

    *temp_tsid = ((*ci_start_service == TRUE) || (tp_id == dmx_tp_id)) ? TSI_TS_A : TSI_TS_B;

    return TRUE;
}

/**ci serial/parrel mode proc.
*   return value: FALSE: return; TRUE: continue exec.
*/

static BOOL  api_ci_sp_mode_proc(BOOL pip_chgch,UINT8 *temp_tsid, BOOL *ci_start_service, UINT8 dmx_id,
                                        UINT32 tp_id, UINT32 prog_id)
{
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    UINT32                  dmx_tp_id = 0;
    UINT8                   temp_id = 0;
    struct dmx_device       *dmx = NULL;
    struct dmx_config       dmx_cfg;
    pvr_record_t            *rec = NULL;
    UINT8                   rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;


    if ((NULL == temp_tsid)||(NULL == ci_start_service))
    {
        return FALSE;
    }
    pvr_info  = api_get_pvr_info();

    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&dmx_cfg, 0x0, sizeof(dmx_cfg));

    if (sys_data_get_ci_mode() == CI_SERIAL_MODE)
    {
        if (*temp_tsid == TSI_TS_A)
        {
            *ci_start_service = TRUE;

              // can not use CI only when pip_changechannel and
              // main screen use CI or CI used for rec&playback this rec!!
            if (pip_chgch)
            {
                ts_route_id = ts_route_check_ci(0);
                if (ts_route_id != -1)
                {
                    //for BUG19062
                    if ((ts_route_get_by_type(TS_ROUTE_PLAYBACK, NULL, &ts_route) != RET_FAILURE)
                    && (ts_route.stream_ca_mode == 1)
                    && ((ts_route.cia_used == 1) || (ts_route.cib_used == 1))
                    && (ts_route.ts_id == TSI_TS_A)
                    && (ts_route.dmx_id != 2))
                    {
                        *ci_start_service = FALSE;
                        *temp_tsid = TSI_TS_B;
                    }
                    else
                    {
                        MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
                        if (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
                        {
                            if ((1 == ts_route.stream_ca_mode)
                            && ((TS_ROUTE_MAIN_PLAY == ts_route.type)
                               || ((TS_ROUTE_BACKGROUND == ts_route.type)
                                  && (ts_route.is_recording)
                                  && (((PVR_STATE_TMS_PLAY == ap_pvr_set_state())
                                      && (pvr_r_get_channel_id( pvr_info->tms_r_handle) == ts_route.prog_id))
                                     || ((PVR_STATE_REC_PLAY == ap_pvr_set_state())
                                        && (rec = api_pvr_get_rec_by_prog_id(ts_route.prog_id, &rec_pos))
                                        && (rec != NULL)
                                        && (pvr_get_index( pvr_info->play.play_handle)
                                            == pvr_get_index(rec->record_handle)))))))
                            {
                                *ci_start_service = FALSE;
                            }
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }
        else if (TSI_TS_B == *temp_tsid)
        {
            //need to switch with other dmx's routes!
            *temp_tsid = ts_route_check_ts_by_dmx(1 - dmx_id);
            if (TSI_TS_A == *temp_tsid)  //no existed routes including the dmx_id
            {
                *ci_start_service = TRUE;
                if (pip_chgch)          //can not use CI only when pip_changechannel and main screen use CI
                {
                    ts_route_id = ts_route_check_ci(0);
                    if (ts_route_id != -1)
                    {
                        if (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
                        {
                            //for BUG19392
                            if ((ts_route.type == TS_ROUTE_PLAYBACK)
                            || ((ts_route.type == TS_ROUTE_MAIN_PLAY ) && (ts_route.stream_ca_mode == 1)))
                            {
                                *ci_start_service = FALSE;
                            }
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }

                *temp_tsid = (*ci_start_service == TRUE) ? TSI_TS_A : TSI_TS_B;
                if (*temp_tsid == TSI_TS_B)
                {
                    //check whether can use same TS
                    dmx = (dmx_id == 0) ? g_dmx_dev2 : ((dmx_id == 1) ? g_dmx_dev : NULL);
                    if (dmx)
                    {
                        dev_get_dmx_config(dmx, dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX), &dmx_cfg);
                        dmx_tp_id = dmx_cfg.tp_id;
                        if (dmx_tp_id == tp_id)
                        {
                            *temp_tsid = TSI_TS_A;
                        }
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
            else    // NULL / TSI_TS_B
            {
                *temp_tsid = TSI_TS_A;
                *ci_start_service = TRUE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else            // CI parallel mode
    {
        //check whether can use CI under this existed TS routes
        ts_route_id = ts_route_check_ci((*temp_tsid == TSI_TS_A) ? 0 : 1);

        // if the background that later be reactived, no need to check again!
        if ((ts_route_id != -1)
        && (RET_FAILURE != ts_route_get_by_id(ts_route_id, &ts_route))
        && (prog_id != ts_route.prog_id))    //can't use it, need to check aother dmx's CI usage state!
        {
            temp_id = *temp_tsid;
            *temp_tsid = ts_route_check_ts_by_dmx(1 - dmx_id);
            if ((*temp_tsid == 0))
            {
                *temp_tsid = temp_id;    //to use original temp_tsid
            }
            else
            {
                if ( (*temp_tsid == temp_id))    //no routes / equal between dmx!
                {
                    *temp_tsid = 3 - temp_id;
                    *ci_start_service = TRUE;
                }
                else    // not equal between 2 dmx!
                {
                    //check whether can use CI under this existed TS routes
                    ts_route_id = ts_route_check_ci((*temp_tsid == TSI_TS_A) ? 0 : 1);
                    if ((ts_route_id != -1)
                    && (RET_FAILURE != ts_route_get_by_id(ts_route_id, &ts_route))
                    && (ts_route.stream_ca_mode == 1))//can't use it, need to check aother dmx's CI usage state!
                    {
                        *temp_tsid = temp_id;
                        *ci_start_service = FALSE;
                    }
                    else    //can use this CI
                    {
                        *temp_tsid = temp_id;
                        *ci_start_service = TRUE;
                    }
                }
            }
        }
        else    //can use this CI
        {
            *ci_start_service = TRUE;
        }
    }

    return TRUE;
}

/**
*   check whether to drop TS to other, only can under serial CI mode and when main chgch and
*       pip is scrambled and can pass CI!
*/
static void ap_check_if_can_drop_ts(BOOL pip_chgch, UINT32 tp_id,UINT8 *temp_tsid)
{
    struct ts_route_info    ts_route;
    INT32                   ts_route_id = 0;
    UINT32                  dmx_tp_id = 0;
    UINT8                   temp_id = 0;
    P_NODE                  p_node2;

    if (NULL == temp_tsid)
    {
        return;
    }
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&p_node2, 0x0, sizeof(p_node2));

    if ((sys_data_get_ci_mode() == CI_SERIAL_MODE)
    && !pip_chgch
    && (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route) != RET_FAILURE)
    && (ts_route.stream_ca_mode == 1)
    && !(ts_route.cia_used || ts_route.cib_used)
    && api_pvr_check_dmx_canuseci(ts_route.stream_ca_mode, ts_route.dmx_id))
    {
        ts_route_id = ts_route_check_ci(0);
        if (ts_route_id != -1)
        {
            if ((ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
            && (ts_route.cia_used || ts_route.cib_used))
            {
                api_operate_ci(0, ((ts_route.cib_used << 1) + ts_route.cia_used),
                    ts_route.dmx_sim_info[0].sim_id, NULL, NULL, ts_route.prog_id, 0, 0);
                ts_route.cia_used = 0;
                ts_route.cib_used = 0;
                ts_route_update(ts_route_id, &ts_route);
            }
        }

        ts_route_get_by_type(TS_ROUTE_PIP_PLAY, &ts_route_id, &ts_route);
        if (ts_route.ts_id != TSI_TS_A)
        {
            ts_route.ts_id = TSI_TS_A;
            ts_route.cia_included = 1;
            ts_route.cib_included = 1;
            tsi_dmx_src_select(ts_route.dmx_id + 1, ts_route.ts_id);
            tsi_select(ts_route.ts_id, ts_route.tsi_id);
        }

        ts_route.cia_used = 1;
        get_prog_by_id(ts_route.prog_id, &p_node2);
        api_operate_ci(1, ((ts_route.cib_used << 1) + ts_route.cia_used),
            ts_route.dmx_sim_info[0].sim_id, dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route.nim_id),
            dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route.dmx_id), ts_route.prog_id, p_node2.video_pid,
            p_node2.audio_pid[p_node2.cur_audio]);
        vdec_start(g_decv_dev2);    // avoid vdec be in stop state for mosaic bug when swap between 2 $ progs!
        ts_route_update(ts_route_id, &ts_route);
        *temp_tsid = TSI_TS_B;
    }
    else
    {
        //check whether change TP under this TS
        ts_route_id = ts_route_check_ci(0);
        if ((ts_route_id != -1)
        && (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
        && (ts_route.tp_id != tp_id)
        && ((ts_route.is_recording)
           || ((pip_chgch)
              && (ts_route.type == TS_ROUTE_MAIN_PLAY)
              && (ts_route.state & TS_ROUTE_STATUS_PLAY))))
        {
            *temp_tsid = TSI_TS_B;
        }
    }
}

#endif

BOOL api_select_ts(BOOL pip_chgch, BOOL bk_play, BOOL scramble, UINT32 prog_id,
                UINT32 tp_id, UINT8 dmx_id, UINT8 *ts_id, BOOL *ci_start_service)
{
#ifdef CI_SUPPORT
    UINT8                   temp_id = 0;
    UINT8                   dmx_route_num = 0;
    UINT16                  dmx_routes[2] = { 0 };
    pvr_record_t            *rec = NULL;
    UINT8                   rec_pos = 0;
    BOOL                    ret_val = FALSE;
#endif
    UINT8                   temp_tsid = 0;
    UINT32                  dmx_tp_id = 0;
    INT32                   ts_route_id = 0;
    struct ts_route_info    ts_route;
    struct ts_route_info    ts_route1;
    struct dmx_device       *dmx = NULL;
    struct dmx_config       dmx_cfg;
    P_NODE                  p_node2;
    P_NODE                  playing_pnode;

    if ((NULL == ts_id)||(NULL == ci_start_service))
    {
        return FALSE;
    }
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
#if (!defined(SUPPORT_TWO_TUNER) && defined(CC_USE_TSG_PLAYER))
    *ts_id = TSI_TS_B;
    *ci_start_service = scramble?TRUE:FALSE;
    return TRUE;
#else
    MEMSET(&ts_route, 0x0, sizeof(ts_route));
    MEMSET(&ts_route1, 0x0, sizeof(ts_route1));
    MEMSET(&dmx_cfg, 0x0, sizeof(dmx_cfg));
    MEMSET(&p_node2, 0x0, sizeof(p_node2));
    // deal with pip use same TS with main and chgch to other TP, chg pip TS first!!
    if (pip_chgch
    && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
    && (tp_id != ts_route.tp_id)
    && (1 == ts_route.stream_ca_mode)
    && (ts_route_get_by_type(TS_ROUTE_PIP_PLAY, (UINT32 *)&ts_route_id, &ts_route1) != RET_FAILURE)
    && (ts_route1.ts_id == ts_route.ts_id))
    {
        ts_route1.ts_id = 3 - ts_route1.ts_id;
        ts_route_update(ts_route1.id, &ts_route1);
    }

    //add for mian&pip play same pip under ca mode for BUG18739
    if (pip_chgch
    && (ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) != RET_FAILURE)
    && (prog_id == playing_pnode.prog_id)
    && (1 == ts_route.stream_ca_mode))
    {
        *ts_id = ts_route.ts_id;
        return TRUE;
    }
    MEMSET(&ts_route, 0, sizeof(struct ts_route_info));
    if (scramble && !bk_play)       //want to use CI
    {
        //check the TS that already existed in the route including the dmx_id!
        temp_tsid = ts_route_check_ts_by_dmx(dmx_id);
        if (0 == temp_tsid)         //no existed routes including the dmx_id
        {
            //check TS of another dmx_id's route
            temp_tsid = ts_route_check_ts_by_dmx(1 - dmx_id);
            if (TSI_TS_A == temp_tsid)
            {
#ifdef CI_SUPPORT
                if (CI_SERIAL_MODE == sys_data_get_ci_mode())
                {
                    // check if it is pip_chgchannel/main screen mode. if it is, it can not use ci.
                    ret_val = check_canuseci(pip_chgch, ci_start_service, tp_id, &temp_tsid);
                    if(!ret_val)
                    {
                        return FALSE;
                    }
                }
                else
#endif
                {
                    temp_tsid = TSI_TS_B;
                    *ci_start_service = TRUE;
                }
            }
            else
            {
                temp_tsid = TSI_TS_A;
                *ci_start_service = TRUE;
            }
        }
        else    // TSI_TS_A / TSI_TS_B
        {
#ifdef CI_SUPPORT
            ret_val = api_ci_sp_mode_proc(pip_chgch, &temp_tsid, ci_start_service, dmx_id, tp_id, prog_id);
            if(!ret_val)
            {
                return FALSE;
            }
#endif
        }
    }
    else    //select one TS is OK!
    {
        //check the TS that already existed in the route including the dmx_id!
        temp_tsid = ts_route_check_ts_by_dmx(dmx_id);
        if (0 == temp_tsid)     //no existed routes including the dmx_id
        {
            //check TS of another dmx_id's route
            temp_tsid = ts_route_check_ts_by_dmx(1 - dmx_id);
            if (0 == temp_tsid) //no routes!!
            {
                if (scramble)
                {
                    temp_tsid = TSI_TS_A;
                    *ci_start_service = TRUE;
                }
                else
                {
                    temp_tsid = TSI_TS_B;
                    *ci_start_service = FALSE;
                }
            }
            else
            {
                //check another dmx's tp_id
                dmx = (0 == dmx_id) ? g_dmx_dev2 : ((1 == dmx_id) ? g_dmx_dev : NULL);
                if (dmx)
                {
                    dev_get_dmx_config(dmx, dev_get_device_sub_type(dmx, HLD_DEV_TYPE_DMX), &dmx_cfg);
                    dmx_tp_id = dmx_cfg.tp_id;
                    if (dmx_tp_id != tp_id)
                    {
                        temp_tsid = 3 - temp_tsid;
                    }
                }
                else
                {
                    return FALSE;
                }
            }
        }
        else if (TSI_TS_A == temp_tsid)
        {
#ifdef CI_SUPPORT
            // check whether to drop TS to other, only can under serial
            // CI mode and when main chgch and pip is scrambled and can pass CI!
            ap_check_if_can_drop_ts(pip_chgch, tp_id, &temp_tsid);
#endif
        }
        else    //TSI_TS_B
        {
#ifdef CI_SUPPORT
            if (CI_SERIAL_MODE == sys_data_get_ci_mode())
            {
                ts_route_check_dmx_route(dmx_id, &dmx_route_num, dmx_routes);
                if ((dmx_route_num > 0) && (ts_route_get_by_id(dmx_routes[0], &ts_route) != RET_FAILURE))
                {
                    if ((ts_route.tp_id != tp_id)
                    && (ts_route.type != (pip_chgch ? TS_ROUTE_PIP_PLAY : TS_ROUTE_MAIN_PLAY)))
                    {
                        temp_tsid = 3 - temp_tsid;
                    }
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                //check whether change TP under this TS
                ts_route_id = ts_route_check_ci(1);
                if ((ts_route_id != -1)
                && (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE)
                && (ts_route.tp_id != tp_id)
                && ((ts_route.is_recording)
                   || ((pip_chgch)
                      && (ts_route.type == TS_ROUTE_MAIN_PLAY)
                      && (ts_route.state & TS_ROUTE_STATUS_PLAY))))
                {
                    temp_tsid = TSI_TS_A;
                }
            }
#endif
        }
    }
    *ts_id = temp_tsid;
    if (api_check_ts_by_dmx(dmx_id, *ts_id, tp_id, TRUE, scramble, *ci_start_service) != TRUE)
    {
        return FALSE;
    }
    if (api_check_ts_by_dmx(1 - dmx_id, *ts_id, tp_id, FALSE, scramble, *ci_start_service) != TRUE)
    {
        return FALSE;
    }
    return TRUE;
#endif
}

#endif

BOOL api_is_displaying_blackscreen(void)
{
    return b_display_blacked;
}

INT32 api_disp_blackscreen(BOOL black, BOOL force)
{
    RET_CODE            ret_code = RET_FAILURE;
    struct vdec_device  *vdec_handle = NULL;
    struct vpo_device   *vpo_handle = NULL;

	if(NULL == vpo_handle)
	{
		;
	}
	vdec_handle = get_selected_decoder();
    vpo_handle = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    if (b_display_blacked != black || force)
    {
        if (TRUE == black)
        {
            ret_code = vdec_stop(vdec_handle, TRUE, FALSE);

            if (ret_code == RET_BUSY)
            {
                ret_code = vdec_stop(vdec_handle, TRUE, FALSE);
            }
        }
        else
        {
            ret_code = vdec_start(vdec_handle);
        }

        b_display_blacked = black;
    }

    return RET_SUCCESS;
}

sys_state_t api_get_system_state(void)
{
    return sys_state;
}

void api_set_system_state(sys_state_t stat)
{
    sys_state_bak = sys_state;
    sys_state = stat;
}

void api_restore_system_state(void)
{
    sys_state = sys_state_bak;
}

UINT8 lib_nimng_get_nim_play(void)
{
    return 1;
}

UINT8 lib_nimng_get_lnb_ns(void)
{
    return 2;
}

UINT8 get_chunk_add_len(UINT32 id, UINT32 *addr, UINT32 *len)
{
    INT32           ret = 0;
    CHUNK_HEADER    chuck_hdr;

    if ( NULL == addr )
    {
        return 0;
    }
    ret = get_chunk_header(id, &chuck_hdr);
    if (0 == ret)
    {
        return 0;
    }

    *addr = (UINT32) chunk_goto(&id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    *len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    return 1;
}
