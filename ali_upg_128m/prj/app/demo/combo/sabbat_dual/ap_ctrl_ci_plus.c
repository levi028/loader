/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ap_ctrl_ci_plus.c
 *
 *    Description: This source file is CI PLUS application process.
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
#include <api/libsi/sie_monitor.h>
#include <bus/tsi/tsi.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_desc_sat_delivery.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/psi_pat.h>
#include <api/libsi/psi_pmt.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_utility.h>
#include "copper_common/com_api.h"
#include "control.h"
#include "menus_root.h"
#ifdef CI_SUPPORT
#include "win_ci_common.h"
#endif
#include "ap_ctrl_ci.h"
#include "ap_ctrl_dual_ci.h"
#include "ap_ctrl_display.h"
#include "ap_ctrl_ci_plus.h"


#define SIGNAL_MAX_CHECK_CNT 6

struct si_descriptor
{
    UINT8   tag;
    UINT8   len;
    UINT8   data[0];
};

#if (defined CI_SUPPORT)
UINT32  bak_ci_caumup_prog_id = 0;
UINT8   do_not_enable_analogoutput = 0;
UINT8   ciplus_analog_output_status = 1;
#endif

#ifdef CI_PLUS_SUPPORT
static UINT8    ci_tuned_table_buff[1024];


/**
 * Check and update service shunning information into database when receive new sdt.
 * @return TRUE, current service have been updated.
 *         FALSE, current service not update.
 */
static BOOL ap_ci_update_shunning_info(UINT8 slot_mask)
{
    UINT32  serv_cnt,
            i,
            k;
    UINT16  *serv_list;
    UINT8   shunning_protected,
            update_done;
    BOOL    ret = FALSE;
    P_NODE  pnode;
    P_NODE  playing_pnode;
    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    CI_SHUNNING_DEBUG("# %s\n", __FUNCTION__);
    serv_cnt = api_ci_get_sdt_service_count();
    if (serv_cnt == 0)
    {
        return ret;
    }

    serv_list = (UINT16 *)MALLOC(sizeof(UINT16) * serv_cnt);
    api_ci_get_sdt_service_list(serv_list, serv_cnt);

    /* update db */
    k = 0;
    CI_SHUNNING_DEBUG("#\t Check service count = %d\n", serv_cnt);
    while (SUCCESS == get_prog_at(k, &pnode))
    {
        ap_get_playing_pnode(&playing_pnode);
        CI_SHUNNING_DEBUG("#\t Check pnode.tp_id = %d (@%s)\n", pnode.tp_id, __FUNCTION__);
        CI_SHUNNING_DEBUG("#\t Check playing_pnode.tp_id = %d (@%s)\n", playing_pnode.tp_id, __FUNCTION__);
        if (pnode.tp_id == playing_pnode.tp_id)
        {
            for (i = 0; i < serv_cnt; i++)
            {
                CI_SHUNNING_DEBUG("#\t Check pnode.prog_number = %d (@%s)\n", pnode.prog_number, __FUNCTION__);
                CI_SHUNNING_DEBUG("#\t Check serv_list[i] = %d (@%s)\n", serv_list[i], __FUNCTION__);
                if (pnode.prog_number == serv_list[i])
                {
                    CI_SHUNNING_DEBUG("#\t YES, found one\n");
                    if (CI_PROTECTED_ACTIVE == ci_service_shunning(pnode.tp_id, pnode.prog_number))
                    {
                        shunning_protected = 1;
                        CI_SHUNNING_DEBUG("#\t YES, this service need protected\n");
                    }
                    else
                    {
                        shunning_protected = 0;
                        CI_SHUNNING_DEBUG("#\t NO, this service no need protected\n");
                    }

                    if (pnode.shunning_protected != shunning_protected)
                    {
                        pnode.shunning_protected = shunning_protected;
                        modify_prog(pnode.prog_id, &pnode);
                        update_data();
                        if (pnode.prog_number == playing_pnode.prog_number)
                        {
                            ret = TRUE;
                        }

                        CI_SHUNNING_DEBUG("#\t Update service = %d\n", pnode.prog_number);
                    }

                    serv_list[i] = 0xFFFF;
                }
            }

            update_done = 1;
            for (i = 0; i < serv_cnt; i++)
            {
                if (serv_list[i] != 0xFFFF)
                {
                    update_done = 0;
                }
            }
        }

        if (update_done)
        {
            break;
        }

        k++;
    }

    CI_SHUNNING_DEBUG("# After while loop\n");
    return ret;
}

static INT32 ap_ci_parse_tp_info(UINT8 *data, INT32 data_len, T_NODE *t_node)
{
    INT32                   i = 0;
    struct si_descriptor    *desc = NULL;
    UINT32                  polar = 0;
    INT32                   ret = CI_TUNED_SUCCESS;
    P_NODE                  playing_pnode;
    struct satellite_delivery_system_descriptor     *sat = NULL;
    struct cable_delivery_system_descriptor         *cab = NULL;

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    libc_printf("%s:---->\n", __FUNCTION__);
    for (i = 0; i < data_len; i += sizeof(struct si_descriptor) + desc->len)
    {
        desc = (struct si_descriptor *)(data + i);
        if (desc->tag == SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR)
        {
            libc_printf("%s:come to s.\n", __FUNCTION__);

            sat = (struct satellite_delivery_system_descriptor *)desc;
            polar = sat->polarization;
            t_node->pol = polar < SAT_POL_CIRCLE_LEFT ? polar : polar - 2;
            t_node->frq = bcd2integer(sat->frequency, 8, 0) / 100;
            t_node->sym = bcd2integer(sat->symbol_rate, 7, 0) / 10;
            t_node->fec_inner = sat->fec_inner;
            t_node->sat_id = playing_pnode.sat_id;
            libc_printf("plo=%d\nfrp=%d\nsym=%d\nFEC=%d\n", t_node->pol, t_node->frq, t_node->sym, t_node->fec_inner);
            break;
        }

        if (desc->tag == CABLE_DELIVERY_SYSTEM_DESCRIPTOR)
        {
            libc_printf("%s:come to C.\n", __FUNCTION__);

            cab = (struct cable_delivery_system_descriptor *)desc;
            t_node->frq = bcd2integer(cab->frequency, 8, 0) / 100;  //cab->frequency;
            t_node->sym = bcd2integer(cab->symbol_rate, 7, 0) / 10; //cab->symbol_rate;
            t_node->fec_inner = cab->modulation + 3;    //cab->FEC_inner;
            t_node->ft_type = FRONTEND_TYPE_C;
            t_node->sat_id = playing_pnode.sat_id;
            libc_printf("frp=%d\nsym=%d\nmodulation=%d\n", t_node->frq, t_node->sym, t_node->fec_inner);
            break;
        }

        if (desc->tag == TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR)
        {
            //to do ...
            break;
        }
    }

    return ret;
}

/* search given tp's detail information from the NIT buffer*/
static INT32 ap_ci_search_node(UINT8 *buff, INT32 buff_len, UINT16 orig_network_id, UINT16 ts_id, T_NODE *t_node)
{
    INT32   ret = CI_TUNED_FAILURE;
    INT32   desc_len = 0;
    INT32   loop_len = 0;

    desc_len = ((buff[8] & 0x0F) << 8) | buff[9];
    buff += (10 + desc_len);
    loop_len = ((buff[0] & 0x0F) << 8) | buff[1];
    buff += 2;

    while (loop_len > 0)
    {
        desc_len = ((buff[4] & 0x0F) << 8) | buff[5];

        if ((((buff[0] << 8) | buff[1]) == ts_id) && (((buff[2] << 8) | buff[3]) == orig_network_id))
        {
            MEMSET(t_node, 0, sizeof(T_NODE));
            t_node->network_id = orig_network_id;
            t_node->t_s_id = ts_id;
            ret = ap_ci_parse_tp_info(buff + 6, desc_len, t_node);
            break;
        }

        buff += (6 + desc_len);
        loop_len -= (6 + desc_len);
    }

    return ret;
}

/*
 * request the NIT from stream
 * and find given tp's information
 */
static INT32 ap_ci_find_tp_in_nit(UINT16 network_id, UINT16 orig_network_id, UINT16 ts_id, T_NODE *t_node)
{
    INT32               ret = RET_FAILURE;
    UINT8               section_num = 0;
    UINT8               last_section_num = 0;
    struct dmx_device   *dmx_dev = NULL;
    struct get_section_param    sr_request;
    struct restrict             sr_restrict;

    if ((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
    {
        CI_DEBUG("ap_ci_find_tp_in_nit: get DMX failure.\n");
        return CI_TUNED_FAILURE;
    }

    libc_printf("ap_ci_find_tp_in_nit(0x%x, %d, %d).\n", network_id, orig_network_id, ts_id);

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));
    sr_request.buff = ci_tuned_table_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = PSI_NIT_PID;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 10000;
    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[3] = 0xff;
    sr_restrict.mask[4] = 0xff;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_NIT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;
    sr_restrict.value[0][3] = network_id >> 8;
    sr_restrict.value[0][4] = (UINT8) network_id & 0xFF;
    section_num = 0;
    do
    {
        sr_restrict.value[0][6] = section_num;
        if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
        {
            libc_printf("ap_ci_find_tp_in_nit: can't get NIT section from stream.\n");
            ret = CI_TUNED_FAILURE;
            break;
        }

        last_section_num = ci_tuned_table_buff[7];
        libc_printf("  Current section num = %d, last_section_num = %d\n", section_num, last_section_num);
        ret = ap_ci_search_node(ci_tuned_table_buff, 1024, orig_network_id, ts_id, t_node);
        if (ret == CI_TUNED_SUCCESS)
        {
            break;
        }

        section_num++;
    }
    while (section_num <= last_section_num);
    if (ret != CI_TUNED_SUCCESS)
    {
        section_num = 0;
        sr_restrict.value[0][0] = PSI_NIT_TABLE_ID + 1;
        do
        {
            sr_restrict.value[0][6] = section_num;
            if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
            {
                libc_printf("ap_ci_find_tp_in_nit: can't get NIT section from stream2.\n");
                ret = CI_TUNED_FAILURE;
                break;
            }

            last_section_num = ci_tuned_table_buff[7];
            libc_printf("  Current section num = %d, last_section_num = %d\n", section_num, last_section_num);
            ret = ap_ci_search_node(ci_tuned_table_buff, 1024, orig_network_id, ts_id, t_node);
            if (ret == CI_TUNED_SUCCESS)
            {
                break;
            }

            section_num++;
        }
        while (section_num <= last_section_num);
        if (ret != CI_TUNED_SUCCESS)
        {
            CI_DEBUG("ap_ci_find_tp_in_nit: can't find needed TP in NIT.\n");
        }
    }

    return ret;
}

static UINT32 ci_search_tuned_service(struct dmx_device *dmx_dev, UINT32 tp_id, UINT16 service_id, UINT32 *new_prog_id)
{
    INT32                       ret = RET_FAILURE;
    UINT8                       section_num = 0;
    UINT8                       last_section_num = 0;
    UINT16                      pmt_pid = INVALID_PID;
    struct get_section_param    sr_request;
    struct restrict             sr_restrict;
    struct section_parameter    sect_param;
    struct pat_section_info     pat_info;
    struct program_map          *p_maps = NULL;
    DB_VIEW                     *last_view = NULL;
    UINT16                      last_filter_mode = 0;
    UINT32                      last_view_param = 0;

    libc_printf("***** Come here:ci_search_tuned_service *******\n");

    /* 1. Get PAT */
    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));
    sr_request.buff = ci_tuned_table_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = PSI_PAT_PID;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 3000;
    sr_restrict.mask_len = 7;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_PAT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;
    section_num = 0;
    p_maps = MALLOC(sizeof(struct program_map) * PSI_MODULE_MAX_PROGRAM_NUMBER);
    if (p_maps == NULL)
    {
        return !SUCCESS;
    }

    MEMSET(p_maps, 0, sizeof(sizeof(struct program_map) * PSI_MODULE_MAX_PROGRAM_NUMBER));
    do
    {
        sr_restrict.value[0][6] = section_num;
        if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
        {
            CI_DEBUG("%s: can't get PAT section from stream.\n", __FUNCTION__);
            if (NULL != p_maps)
            {
                free(p_maps);
            }
            return CI_TUNED_FAILURE;
        }

        libc_printf("*****Found PAT *******\n");
        last_section_num = ci_tuned_table_buff[7];
        MEMSET(&pat_info, 0, sizeof(pat_info));
        sect_param.priv = (void *) &pat_info;
        pat_info.map = p_maps;
        pat_info.max_map_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
        ret = psi_pat_parser(ci_tuned_table_buff, 1024, &sect_param);
        if (ret == SUCCESS)
        {
            break;
        }

        section_num++;
    }
    while (section_num <= last_section_num);
    FREE(p_maps);
    p_maps = NULL;

    /* 1.1 Find service's PMT pid */
    UINT8   i = 0;
    for (i = 0; i < pat_info.map_counter; i++)
    {
        libc_printf("pat_info.map[i].pm_number = %d\n", pat_info.map[i].pm_number);
        libc_printf("pat_info.map[i].pm_pid = %d\n", pat_info.map[i].pm_pid);
        if (pat_info.map[i].pm_number == service_id)
        {
            pmt_pid = pat_info.map[i].pm_pid;
            libc_printf("*****Find service's PMT pid = %d*******\n", pmt_pid);
            break;
        }
    }

    if (i == pat_info.map_counter)
    {
        CI_DEBUG("*****Can not Find service's PMT pid = %d*******\n", __FUNCTION__);
        return !SUCCESS;
    }

    /* 2. Get PMT */
    PROG_INFO   pg_info;
    P_NODE      p_node;

    MEMSET(&sr_request, 0, sizeof(struct get_section_param));
    MEMSET(&sr_restrict, 0, sizeof(struct restrict));
    MEMSET(&pg_info, 0, sizeof(PROG_INFO));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    sr_request.buff = ci_tuned_table_buff;
    sr_request.buff_len = 1024;
    sr_request.crc_flag = 1;
    sr_request.pid = pmt_pid;
    sr_request.mask_value = &sr_restrict;
    sr_request.wai_flg_dly = 3000;
    sr_restrict.mask_len = 6;
    sr_restrict.value_num = 1;
    sr_restrict.mask[0] = 0xff;
    sr_restrict.mask[1] = 0x80;
    sr_restrict.mask[6] = 0xff;
    sr_restrict.value[0][0] = PSI_PMT_TABLE_ID;
    sr_restrict.value[0][1] = 0x80;

    //    sr_request.pid = pmt_pid;
    //    sr_restrict.value[0][0] = PSI_PMT_TABLE_ID;
    section_num = 0;
    do
    {
        sr_restrict.value[0][6] = section_num;
        CI_DEBUG("%s: Going to get PMT section from stream.\n", __FUNCTION__);
        if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
        {
            CI_DEBUG("%s: can't get PMT section from stream.\n", __FUNCTION__);
            return CI_TUNED_FAILURE;
        }

        CI_DEBUG("%s: get PMT section from stream success!!.\n", __FUNCTION__);
        last_section_num = ci_tuned_table_buff[7];
        MEMSET(&pg_info, 0, sizeof(pg_info));
        ret = psi_pmt_parser(ci_tuned_table_buff, &pg_info, PSI_MODULE_COMPONENT_NUMBER);
        pg_info.prog_number = service_id;
        pg_info.pmt_pid = pmt_pid;
        libc_printf("CA Count = %d\n", pg_info.ca_count);
        dvb_to_unicode("NoName", STRLEN("NoName"), (UINT16 *)pg_info.service_name, MAX_SERVICE_NAME_LENGTH, 0);
        dvb_to_unicode("NoName", STRLEN("NoName"), (UINT16 *)pg_info.service_provider_name, MAX_SERVICE_NAME_LENGTH, 0);
        libc_printf("*****psi_pmt_parser=%d*******\n", ret);
        psi_pg2db(&p_node, &pg_info);
        if (pg_info.ca_count > 0)
        {
            p_node.ca_mode = 1;
        }
        else
        {
            p_node.ca_mode = 0;
        }

        if (ret == SUCCESS)
        {
            break;
        }

        section_num++;
    }
    while (section_num <= last_section_num);

    /* 3. save Program */
    last_view = (DB_VIEW *)get_cur_view();
    last_filter_mode = last_view->cur_filter_mode;
    last_view_param = last_view->view_param;
    libc_printf("***going to add_node!***\n");
    libc_printf("v pid = %d\n", p_node.video_pid);
    libc_printf("a pid = %d\n", p_node.audio_pid[0]);
    if (last_view->cur_type != TYPE_PROG_NODE)
    {
        recreate_prog_view(VIEW_ALL, 0);
        ret = add_node(TYPE_PROG_NODE, tp_id, &p_node);
        if (ret == SUCCESS)
        {
            libc_printf("***add_node success!***\n");
            update_data();
        }

        //        recreate_prog_view(last_filter_mode, last_view_param);
    }
    else
    {
        ret = add_node(TYPE_PROG_NODE, tp_id, &p_node);
        if (ret == SUCCESS)
        {
            libc_printf("***add_node success2!***\n");
            update_data();
        }
    }

    *new_prog_id = p_node.prog_id;
    libc_printf("*** going to return prog_id = %d!***\n", *new_prog_id);
    return ret;
}

/*For CI+ CAM UpGrade Function 2012/03/16*/
INT32 ap_ci_tune_service(void)
{
    UINT8               slot = 0;
    UINT16              network_id = 0;
    UINT16              org_network_id = 0;
    UINT16              ts_id = 0;
    UINT16              nim_id = 0;
    UINT16              service_id;
    UINT32              prog_id = 0;
    UINT32              tp_id = 0;
    T_NODE              t_tuned_node;
    T_NODE              t_cur_node;
    S_NODE              s_tuned_node;
    P_NODE              p_cur_node;
    P_NODE              playing_pnode;
    struct ft_antenna   antenna;
    union ft_xpond      xpond_info;
    struct nim_device   *nim = NULL;
    INT32               ret = 0;
    UINT8               signal_lock = 0;

    MEMSET(&playing_pnode, 0, sizeof(P_NODE));
    sys_data_get_curprog_info(&p_cur_node);
    bak_ci_caumup_prog_id = p_cur_node.prog_id;
    get_tp_by_id(p_cur_node.tp_id, &t_cur_node);    //Does this function work for dvbc?

    /* step 1. Get service's detail information */
    api_ci_get_hc_id(&slot, &network_id, &org_network_id, &ts_id, &service_id);

    /* step 2. find this service in DB or not */
    prog_id = api_ci_get_prog_id_by_ft_type(network_id, org_network_id, ts_id, service_id, t_cur_node.ft_type);
    if (prog_id != 0)
    {
        /* Yes, the service in DB, tune to the service directly, and return */
        CI_DEBUG("*****Yes, find the service in DB*******\n");
        api_play_channel(prog_id, TRUE, FALSE, TRUE);
        return SUCCESS;
    }

    ap_get_playing_pnode(&playing_pnode);

    /* step 3. service not in DB, check TP in DB or not firstly */
    tp_id = api_ci_tuned_in_db_by_ft_type();
    libc_printf("**TP is in DB? tp_id=%d\n", tp_id);
    if (tp_id == 0)
    {
        /* The TP of the service going to tune not in DB, here should find this TP's detail information */
        if (CI_TUNED_FAILURE == ap_ci_find_tp_in_nit(network_id, org_network_id, ts_id, &t_tuned_node))
        {
            /* Info User: Can't find needed TP, tune service fail. */
            libc_printf("*****Find tp in nit failure*******\n");
            api_ci_set_cur_progid(slot, bak_ci_caumup_prog_id);
            api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
            return !SUCCESS;
        }
        else
        {
            /* Got tp information from NIT, save to TP DB */
            libc_printf("*****Find tp in nit success*******\n");

            DB_VIEW *last_view;
            UINT16  last_filter_mode;
            UINT32  last_view_param;
            last_view = (DB_VIEW *)get_cur_view();
            last_filter_mode = last_view->cur_filter_mode;
            last_view_param = last_view->view_param;
            if (last_view->cur_type != TYPE_TP_NODE)
            {
                recreate_tp_view(VIEW_SINGLE_SAT, playing_pnode.sat_id);
                ret = add_node(TYPE_TP_NODE, playing_pnode.sat_id, (void *) &t_tuned_node);
                if (ret == SUCCESS)
                {
                    libc_printf("%s: Add TP node in DB success!\n", __FUNCTION__);
                    update_data();
                }

                recreate_prog_view(last_filter_mode, last_view_param);
                tp_id = t_tuned_node.tp_id;
            }
        }
    }

    if (tp_id == 0)
    {
        api_ci_set_cur_progid(slot, bak_ci_caumup_prog_id);
        api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
        return !SUCCESS;
    }

    /* OK, here can ensure the TP we need in DB. */
    /* step 4. Check need tune frontend or not */
    if (tp_id != playing_pnode.tp_id)
    {
        // Change frontend
        nim_id = s_tuned_node.tuner1_valid ? 0 : (s_tuned_node.tuner2_valid ? 1 : 0);
        get_sat_by_id(t_tuned_node.sat_id, &s_tuned_node);
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);

        UINT32  nim_sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        if (nim_sub_type == FRONTEND_TYPE_S)
        {
#ifdef DVBS_SUPPORT
            sat2antenna(&s_tuned_node, &antenna);
#endif
            xpond_info.s_info.type = FRONTEND_TYPE_S;
            xpond_info.s_info.frq = t_tuned_node.frq;
            xpond_info.s_info.sym = t_tuned_node.sym;
            xpond_info.s_info.pol = t_tuned_node.pol;
            xpond_info.s_info.tp_id = t_tuned_node.tp_id;
            xpond_info.s_info.fec_inner = t_tuned_node.fec_inner;
        }
        else if (nim_sub_type == FRONTEND_TYPE_C)
        {
            xpond_info.c_info.type = FRONTEND_TYPE_C;
            xpond_info.c_info.frq = t_tuned_node.frq;
            xpond_info.c_info.sym = t_tuned_node.sym;
            xpond_info.c_info.modulation = t_tuned_node.fec_inner;
            xpond_info.c_info.tp_id = t_tuned_node.tp_id;
        }
        else if (nim_sub_type == FRONTEND_TYPE_T)
        {
            //to do...
        }

        frontend_tuning(nim, &antenna, &xpond_info, 1);

        // check signal lock or not
        UINT8   lock_cnt = 0;
        while (lock_cnt < SIGNAL_MAX_CHECK_CNT)
        {
            nim_get_lock(nim, &signal_lock);
            if (signal_lock == FALSE)
            {
                osal_task_sleep(500);
            }
            else
            {
                break;
            }

            lock_cnt++;
        }

        if (signal_lock != TRUE)
        {
            libc_printf("signal_lock missed!\n");

            //api_play_channel(prev_channel);
            api_ci_set_cur_progid(slot, bak_ci_caumup_prog_id);
            api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
            return !SUCCESS;
        }
    }

    /* step 5. OK, now we at the correct TP, try to get PAT, PMT */
    struct dmx_device   *dmx;
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);  // default 0?
    if (ci_search_tuned_service(dmx, tp_id, service_id, &prog_id) == SUCCESS)
    {
        libc_printf("*****Find service success!*******\n");
        api_play_channel(prog_id, TRUE, FALSE, TRUE);
    }
    else
    {
        api_ci_set_cur_progid(slot, bak_ci_caumup_prog_id);
        api_play_channel(bak_ci_caumup_prog_id, TRUE, FALSE, TRUE);
    }

    return SUCCESS;
}
#endif

#ifdef CI_PLUS_SUPPORT
void ap_ci_playch_callback(UINT32 prog_id)
{
    /* The parameters may be different according to different project configuration */
    api_play_channel(prog_id, TRUE, FALSE, TRUE);
}

void ap_ci_camup_progress_callback(int percent)
{
    //...
    static int  first_percent = 0;
    if (percent < 0)
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_CI_UPG, percent, TRUE);
        app_disable_while_ci_upgrade = FALSE;
        first_percent = 0;
        return;
    }

    if ((first_percent == 0) && (percent > 0) && !api_ci_camup_timeout())
    {
        first_percent = percent;
    }

    if (percent == first_percent)
    {
        if (TRUE)
        {
            //stop any recordings
            api_stop_play_record(FALSE);
            api_pvr_tms_proc(FALSE);
            api_stop_record(0, 1);
            api_stop_record(0, 1);

            //stop MP  stop channel
            api_stop_play(FALSE);

            //disable key respond and message handle(prevent other menus popup)
            app_disable_while_ci_upgrade = TRUE;
        }
    }

    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_CI_UPG, percent, TRUE);

    if (percent >= CI_CAM_UPGRADE_PROGRESS_DONE)
    {
        app_disable_while_ci_upgrade = FALSE;
        first_percent = 0;
    }

    return;
}
#endif

