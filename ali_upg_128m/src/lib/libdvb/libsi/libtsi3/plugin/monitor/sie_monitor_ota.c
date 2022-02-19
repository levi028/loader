/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_monitor_ota.c
*
*    Description: SI monitor adding AUTO OTA character
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/sie_monitor.h>
#include <api/libsi/si_nit.h>
#include <api/libota/lib_ota.h>
#include <api/libpub/lib_frontend.h>
#include "sie_monitor_core.h"

#ifdef AUTO_OTA
#define SIM_INVALID_MON_ID  0xFFFFFFFF  //invalid si monitor object ID

struct NIT_TABLE_INFO
{
    struct nit_section_info nit;
};

UINT8 otapmt_slot_on = 0;
UINT32 otapmt_monitor_id = SIM_INVALID_MON_ID;
struct NIT_TABLE_INFO nit_table_info;
struct extension_info ota_nit_ext;
UINT8 nit_flag[PSI_MODULE_MSECTION_NUMBER>>3] = {0};
static UINT16 m_ota_cur_chan_index = 0xffff;
static UINT32 nit_monitor_id = 0xffffffff;
static INT8 linkagetype09_exist = 0;    //default:not exist;
static AUTO_OTA_INFO m_ota_info;
static BOOL m_ota_upgrade_found = FALSE;

//changed by Iris for dvbs and dvbc auto ota
static SERVICEPID_INFO ota_servicepid_info = {
    .service_id = 0xFFFF,
    .service_pmtpid = 0x1FFF,
    .ssuinfo = {
    .s_ssuinfo = {
        .frequency = 0x0,
        .symbol= 0x0,
        .polar=0x0,
        .ssu_pid = 0xFFFF,
        .ssu_sat_id=0x0,
    },
    .t_ssuinfo = {
        .frequency = 0x0,
        .bandwidth= 0x0,
        .ssu_pid = 0xFFFF,
    },
    .c_ssuinfo = {
        .frequency = 0x0,
        .symbol= 0x0,
        .qam=0x0,
        .ssu_pid = 0xFFFF,
    },
    }
};

void set_ota_upgrade_found(BOOL set)
{
    m_ota_upgrade_found = set;
}

BOOL get_ota_upgrade_found(void)
{
    return m_ota_upgrade_found;
}

UINT16 si_get_ota_serviceid()
{
    return ota_servicepid_info.service_id;
}

void si_set_ota_serviceid(UINT16 serviceid)
{
    ota_servicepid_info.service_id = serviceid;
}

UINT16 si_get_ota_servicepmtpid()
{
    return ota_servicepid_info.service_pmtpid;
}

void si_set_ota_servicepmtpid(UINT16 pmtpid)
{
    ota_servicepid_info.service_pmtpid = pmtpid;
}

#if(defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
UINT16 si_get_ota_ssupid_t()
{
    return ota_servicepid_info.ssuinfo.t_ssuinfo.ssu_pid;
}

void si_set_ota_ssupid_t(UINT16 ssupid)
{
    ota_servicepid_info.ssuinfo.t_ssuinfo.ssu_pid = ssupid;
}

UINT32 si_get_ota_bandwidth()
{
    return ota_servicepid_info.ssuinfo.t_ssuinfo.bandwidth;
}

void si_set_ota_bandwidth(UINT32 bandwidth)
{
    ota_servicepid_info.ssuinfo.t_ssuinfo.bandwidth = bandwidth;
}

UINT32 si_get_ota_freq_t()
{
    return ota_servicepid_info.ssuinfo.t_ssuinfo.frequency;
}

void si_set_ota_freq_t(UINT32 frequency)
{
    ota_servicepid_info.ssuinfo.t_ssuinfo.frequency = frequency;
}

INT32 si_get_ota_ssuinfo_t(union ssu_info *pinfo)
{
    if(NULL == pinfo)
    {
        return -1;
    }

    pinfo->t_ssuinfo.frequency = ota_servicepid_info.ssuinfo.t_ssuinfo.frequency;
    pinfo->t_ssuinfo.bandwidth = ota_servicepid_info.ssuinfo.t_ssuinfo.bandwidth;
    pinfo->t_ssuinfo.ssu_pid = ota_servicepid_info.ssuinfo.t_ssuinfo.ssu_pid;
}
#endif

#if(defined(DVBS_SUPPORT))
UINT16 si_get_ota_ssusatid()
{
    return ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_sat_id;
}

void si_set_ota_ssusatid(UINT16 ssu_sat_id)
{
    ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_sat_id = ssu_sat_id;
}

UINT16 si_get_ota_ssupid_s()
{
    return ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_pid;
}

void si_set_ota_ssupid_s(UINT16 ssupid)
{
    ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_pid = ssupid;
}

UINT32 si_get_ota_symbol_s()
{
    return ota_servicepid_info.ssuinfo.s_ssuinfo.symbol;
}

void si_set_ota_symbol_s(UINT32 symbol)
{
    ota_servicepid_info.ssuinfo.s_ssuinfo.symbol = symbol;
}

UINT32 si_get_ota_freq_s()
{
    return ota_servicepid_info.ssuinfo.s_ssuinfo.frequency;
}

void si_set_ota_freq_s(UINT32 frequency)
{
    ota_servicepid_info.ssuinfo.s_ssuinfo.frequency = frequency;
}

UINT32 si_get_ota_polar()
{
    return ota_servicepid_info.ssuinfo.s_ssuinfo.polar;
}

void si_set_ota_polar(UINT8 polar)
{
    ota_servicepid_info.ssuinfo.s_ssuinfo.polar = polar;
}

INT32 si_get_ota_ssuinfo_s(union ssu_info *pinfo)
{
    if(NULL == pinfo)
    {
        return -1;
    }

    pinfo->s_ssuinfo.frequency=ota_servicepid_info.ssuinfo.s_ssuinfo.frequency;
    pinfo->s_ssuinfo.symbol = ota_servicepid_info.ssuinfo.s_ssuinfo.symbol;
    pinfo->s_ssuinfo.polar = ota_servicepid_info.ssuinfo.s_ssuinfo.polar;
    pinfo->s_ssuinfo.ssu_pid = ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_pid;
    pinfo->s_ssuinfo.ssu_sat_id = ota_servicepid_info.ssuinfo.s_ssuinfo.ssu_sat_id;
}
#endif

#if(defined(DVBC_SUPPORT))
UINT16 si_get_ota_ssupid_c()
{
    return ota_servicepid_info.ssuinfo.c_ssuinfo.ssu_pid;
}

void si_set_ota_ssupid_c(UINT16 ssupid)
{
    ota_servicepid_info.ssuinfo.c_ssuinfo.ssu_pid = ssupid;
}

UINT32 si_get_ota_symbol_c()
{
    return ota_servicepid_info.ssuinfo.c_ssuinfo.symbol;
}

void si_set_ota_symbol_c(UINT32 symbol)
{
    ota_servicepid_info.ssuinfo.c_ssuinfo.symbol = symbol;
}

UINT32 si_get_ota_freq_c()
{
    return ota_servicepid_info.ssuinfo.c_ssuinfo.frequency;
}

void si_set_ota_freq_c(UINT32 frequency)
{
    ota_servicepid_info.ssuinfo.c_ssuinfo.frequency = frequency;
}

UINT32 si_get_ota_modulation_c()
{
    return ota_servicepid_info.ssuinfo.c_ssuinfo.qam;
}

void si_set_ota_modulation_c(UINT8 modulation)
{
    ota_servicepid_info.ssuinfo.c_ssuinfo.qam = modulation;
}

INT32 si_get_ota_ssuinfo_c(union ssu_info *pinfo)
{
    if(NULL == pinfo)
    {
        return -1;
    }

    pinfo->s_ssuinfo.frequency =ota_servicepid_info.ssuinfo.s_ssuinfo.frequency;
    pinfo->c_ssuinfo.frequency=ota_servicepid_info.ssuinfo.c_ssuinfo.frequency;
    pinfo->c_ssuinfo.symbol = ota_servicepid_info.ssuinfo.c_ssuinfo.symbol;
    pinfo->c_ssuinfo.qam = ota_servicepid_info.ssuinfo.c_ssuinfo.qam;
    pinfo->c_ssuinfo.ssu_pid = ota_servicepid_info.ssuinfo.c_ssuinfo.ssu_pid;
}
#endif

void si_set_ota_config(UINT16 hw_model, UINT16 hw_ver, UINT32 oui, UINT16 sw_model, UINT16 ota_sw_version)
{
    m_ota_info.hw_model = hw_model;
    m_ota_info.hw_ver = hw_ver;
    m_ota_info.oui = oui;
    m_ota_info.sw_model = sw_model;
    m_ota_info.ota_sw_version = ota_sw_version;
}

UINT32 si_get_otaconfig_oui()
{
    return m_ota_info.oui;
}

void si_get_otaconfig(AUTO_OTA_INFO *potainfo)
{
    if(NULL == potainfo)
    {
        return ;
    }

    MEMCPY(potainfo, &m_ota_info, sizeof(AUTO_OTA_INFO));
}

void set_linkagetype09_exist(INT8 exist)
{
    linkagetype09_exist = exist;
}

INT8 get_linkagetype09_exist()
{
    return linkagetype09_exist;
}

INT32 nit_linkage_deschandle(UINT8 tag, UINT8 len, UINT8 *pdata, void *ppriv)
{
    if((0x4A != tag) || (NULL == pdata) || (NULL == ppriv))
    {
        libc_printf("%s[si_monitor.c] : linkage tag wrong!\n", __FUNCTION__);
        ASSERT(0);
    }

    struct nit_section_info *n_info = (struct nit_section_info *)ppriv;
    struct NIT_FIRSTLOOP_INFO *pinfo = &n_info->p_firstloopinfo;
    UINT8 linkage_type = pdata[6];
    UINT8 ota_linkage = 0x09;
    UINT8 another_linkage = 0x0A;
    UINT32 oui = 0;
    UINT32 orig_oui = 0;
    UINT16 orig_network_id = 0;
    INT32 ret = SI_SKIP;

    if(ota_linkage == linkage_type)//local stream for OTA
    {
        oui = (pdata[8]<<16 | pdata[9]<<8 | pdata[10]);//ref to ETSI TS 102 006 Page10
        orig_network_id = (pdata[2]<<8 | pdata[3]);
        if(FRANCE_HD_NETWORK_ID == orig_network_id)
        {
            orig_oui = 0x00015A;
        }
        else
        {
            orig_oui = si_get_otaconfig_oui();
        }
        if(oui == orig_oui)
        {
            pinfo->ssu_location_info.linkage_type = 0x09;//0x09:local, 0x0A:another ts
            pinfo->ssu_location_info.des_tsid = (pdata[0]<<8 | pdata[1]);
            pinfo->ssu_location_info.des_orig_netid = (pdata[2]<<8 | pdata[3]);
            pinfo->ssu_location_info.des_serviceid = (pdata[4]<<8 | pdata[5]);
            pinfo->ssu_location_info.OUI = (pdata[8]<<16 | pdata[9]<<8 | pdata[10]);//ref to ETSI TS 102 006 Page10
            pinfo->ssu_location_info.table_type = 0x01;
        }
        ret = SI_SUCCESS;
        set_linkagetype09_exist(1);
    }
    else if(another_linkage == linkage_type)
    {//in one network desc loop , maybe exist two different linkage desc,
        if(ota_linkage != pinfo->ssu_location_info.linkage_type)
        { //so when 0x09 linkage type exist, need not parse 0x0A linkage type desc.
            pinfo->ssu_location_info.linkage_type = 0x0A; //0x09:local, 0x0A:another ts
            pinfo->ssu_location_info.des_tsid = (pdata[0]<<8 | pdata[1]);
            pinfo->ssu_location_info.des_orig_netid = (pdata[2]<<8 | pdata[3]);
            pinfo->ssu_location_info.des_serviceid =0;
            pinfo->ssu_location_info.OUI = 0;
            pinfo->ssu_location_info.table_type = pdata[7];//ref to ETSI TS 102 006 Page10
            ret = SI_SUCCESS;
        }
        set_linkagetype09_exist(0);
    }
    else
    {
        set_linkagetype09_exist(0);
    }
    return ret;
}

static INT8 si_get_service_pid(UINT8 *pbuf, UINT16 serviceid, UINT16 *pservicepid)
{
    UINT16 prognum = 0;

    if((NULL == pbuf) || (NULL == pservicepid))
    {
        return ERR_FAILUE;
    }

    prognum = pbuf[0]<<8 | pbuf[1];

    if(serviceid == prognum)
    {
        *pservicepid = (pbuf[2]&0x1F)<<8 | pbuf[3];
        return SUCCESS;
    }

    pservicepid = NULL;
    return ERR_FAILUE;
}

/*******************************************************************************
*   AUTO OTA callback function
*******************************************************************************/
static BOOL nit_event(UINT8 *buffer)
{
    if(NULL == buffer)
    {
        return FALSE;
    }

    BOOL need_reset = FALSE;
    struct section_header *sh = (struct section_header *)buffer;
    UINT8 section_number = sh->section_number;
    UINT8 last_section_number = sh->last_section_number;

    //this section first hit, need set table sections hit flag bitmap
    if (0 == ota_nit_ext.hit)
    {
        ota_nit_ext.version = sh->version_number;
        ota_nit_ext.last_section_number = last_section_number;
        //nit_ext.hit++;
        need_reset = TRUE;
    }
    //this section not first hit, but table version changed,need reget the table sections
    else if (sh->version_number != ota_nit_ext.version)
    {
        ota_nit_ext.version = sh->version_number;
        need_reset = TRUE;
    }
    //this section not first hit, not need reget
    else
    {
        need_reset = FALSE;
    }
    //set table sections hit flag bitmap
    if (TRUE == need_reset)
    {
        MEMSET(ota_nit_ext.flag, 0xFF, (last_section_number>>3)+1);
        ota_nit_ext.hit = 0;
    }
    //this section not the one wanted, return false
    else if (!(ota_nit_ext.flag[section_number>>3]&(1<<(section_number&7))))
    {
        return FALSE;
    }

    ota_nit_ext.flag[section_number>>3] &= ~(1<<(section_number&7));
    ota_nit_ext.hit++;

    return TRUE;
}

static INT32 ota_pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
    struct prog_info *prog = NULL;
    INT32 otaret = 0;
    struct section_param sec_param;
    P_NODE p_node;
    T_NODE t_node;
    S_NODE s_node;

    if(section == NULL)
    {
        return !SI_SUCCESS;
    }

    prog =(struct nit_section_info*)MALLOC(sizeof(struct prog_info));
    if(NULL == prog)
    {
        return !SI_SUCCESS;
    }

    MEMSET(prog, 0, sizeof(struct prog_info));
    MEMSET(&sec_param, 0, sizeof(struct section_param));
    prog->prog_number = (section[3]<<8) | section[4];

    if (psi_pmt_parser(section, prog,PSI_MODULE_COMPONENT_NUMBER) == SI_SUCCESS)
    {
        if(prog->p_esloopinfo.pmt_ssu_info.ssu_pid != 0)
        {
            if(si_get_otaconfig_oui() != prog->p_esloopinfo.pmt_ssu_info.m_otainfo.oui
//              && OUI_SAGEM != pmt_section_info.p_esloopinfo->pmt_ssu_info.m_otainfo.oui
                )
            {
            }
            else
            {
#ifdef DVBS_SUPPORT
                if(si_get_ota_ssupid_s() != prog->p_esloopinfo.pmt_ssu_info.ssu_pid)
                {
                    si_set_ota_ssupid_s(prog->p_esloopinfo.pmt_ssu_info.ssu_pid);
                }
#endif
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
                if(si_get_ota_ssupid_t() != prog->p_esloopinfo.pmt_ssu_info.ssu_pid)
                {
                    si_set_ota_ssupid_t(prog->p_esloopinfo.pmt_ssu_info.ssu_pid);
                }
#endif
                sec_param.section_type = 0x00;
                otaret = si_private_sec_parsing_start(0,prog->p_esloopinfo.pmt_ssu_info.ssu_pid,
                    parse_dsmcc_sec, (void*)(&sec_param));

                if(otaret == OTA_SUCCESS)
                {
                    if (get_prog_at(m_ota_cur_chan_index, &p_node) != SUCCESS)
                    {
                        return SI_SUCCESS;
                    }

                    get_sat_by_id(p_node.sat_id,&s_node);
                    get_tp_by_id(p_node.tp_id, &t_node);

                #if ((defined(DVBT_SUPPORT))||(defined(ISDBT_SUPPORT)))
                    if(( (FRONTEND_TYPE_T == t_node.ft_type) || (FRONTEND_TYPE_ISDBT== t_node.ft_type)) 
                        && ((t_node.frq != si_get_ota_freq_t()) || (t_node.bandwidth != si_get_ota_bandwidth())))
                    {
                        si_set_ota_freq_t(t_node.frq);
                        si_set_ota_bandwidth(t_node.bandwidth*1000);
                    }
                #endif
                #if (defined(DVBS_SUPPORT))
                    if(( FRONTEND_TYPE_S == t_node.ft_type) 
                        && ((t_node.frq != si_get_ota_freq_s()) || (t_node.sym != si_get_ota_symbol_s())))
                    {
                        si_set_ota_freq_s(t_node.frq);
                        si_set_ota_symbol_s(t_node.sym);
                        si_set_ota_polar(t_node.pol);
                        si_set_ota_ssusatid(t_node.sat_id);
                    }
                #endif
                #if(defined(DVBC_SUPPORT))
                    if(( FRONTEND_TYPE_C == t_node.ft_type)&&
                        ((t_node.frq != si_get_ota_freq_c()) || (t_node.sym != si_get_ota_symbol_c())))
                    {
                        si_set_ota_freq_c(t_node.frq);
                        si_set_ota_symbol_c(t_node.sym);
                        si_set_ota_modulation_c(t_node.fec_inner);
                    }
                #endif
                    set_ota_upgrade_found(TRUE);
                    sim_stop_monitor(otapmt_monitor_id);
                    otapmt_slot_on = 0;
                }
            }
        }
    }
    FREE(prog);
    prog = NULL;

    return SI_SUCCESS;
}

static INT32 ota_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
    UINT8 *prog_map = NULL;
    INT32 prog_map_len = 0;
    UINT16 ota_pmt_pid = 0x1FFF;
    UINT16 ota_serviceid = si_get_ota_serviceid();
    struct dmx_device *dmx = NULL;

    if (section != NULL)
    {
        prog_map_len = (((section[1]&0xF)<<8)|section[2]) - 9;
        prog_map = section + 8;

        /* get OTA PMT pid */
        while (prog_map_len > 0)
        {
            if (SUCCESS == si_get_service_pid(prog_map, ota_serviceid, &ota_pmt_pid))
            {
                if ((0x1FFF != (ota_pmt_pid&0x1FFF)) && (ota_pmt_pid != si_get_ota_servicepmtpid()))
                {
                    si_set_ota_servicepmtpid(ota_pmt_pid);

                    if (1 == otapmt_slot_on)
                    {
                        sim_stop_monitor(otapmt_monitor_id);
                    }
                    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
                    otapmt_monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, ota_pmt_pid, ota_serviceid);
                    otapmt_slot_on = 1;
                    sim_register_scb(otapmt_monitor_id, ota_pmt_callback, NULL);
                }
            }

            prog_map_len -= 4;
            prog_map += 4;
        }
    }

    return SI_SUCCESS;
}

INT32 auto_ota_nit_callback(UINT8 *section, INT32 length, UINT32 param)
{
    UINT32 orig_oui = 0;
    struct dmx_device *dmx = NULL;
    struct section_parameter sec_param;
    struct nit_section_info *nit_info = NULL;
    struct NITSSU_INFO *pssuinfo = NULL;
    P_NODE p_node;
    T_NODE t_node;

    struct desc_table ota_nit_loop1[] = {
        {
        .tag = NETWORK_NAME_DESCRIPTOR,
        .bit = NIT_NETWORKNAME_EXIST_BIT,
        .parser = NULL,
        },
        {
        .tag = LINKAGE_DESCRIPTOR,
        .bit = NIT_LINKAGE_EXIST_BIT,
        .parser = nit_linkage_deschandle,
        },
    };

    if(!nit_event(section))
    {
        return SI_SUCCESS;
    }

    nit_info = &(nit_table_info.nit);

    MEMSET(&sec_param, 0, sizeof(struct section_parameter));
    MEMSET(nit_info, 0, sizeof(struct nit_section_info));

    sec_param.priv = (void*)nit_info;
    nit_info->lp1_nr = ARRAY_SIZE(ota_nit_loop1);
    nit_info->loop1 = ota_nit_loop1;
    nit_info->lp2_nr = 0;

    if(si_nit_parser(section, length, &sec_param) == SI_SUCCESS)
    {
        pssuinfo = &nit_info->p_firstloopinfo.ssu_location_info;
        if(0x09 == pssuinfo->linkage_type)
        {
            if (get_prog_at(m_ota_cur_chan_index, &p_node) != SUCCESS)
            {
                return SI_SUCCESS;
            }

            get_tp_by_id(p_node.tp_id, &t_node);

            #if 1  //def SUPPORT_FRANCE_HD
            if(FRANCE_HD_NETWORK_ID == pssuinfo->des_orig_netid)
            {
                orig_oui = 0x00015A;
            }
            else
            {
                orig_oui = si_get_otaconfig_oui();
            }
            if(pssuinfo->OUI != orig_oui)
            #else
            if(pssuinfo->OUI != si_get_otaconfig_oui())
            #endif
            {
            }
            else
            {
                si_set_ota_serviceid(pssuinfo->des_serviceid);

                if (pat_monitor_id == SIM_INVALID_MON_ID)
                {
                    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
                    pat_monitor_id = sim_start_monitor(dmx, MONITE_TB_PAT, PSI_PAT_PID, 0);
                }
                sim_register_scb(pat_monitor_id, ota_pat_callback, NULL);
            }
        }
    }

    if(ota_nit_ext.hit > ota_nit_ext.last_section_number)
    {
       ota_nit_ext.hit = 0;
    }

    return SI_SUCCESS;
}

void ota_monitor_on(UINT32 index)
{
    struct dmx_device *dmx = NULL;

    /*start monitoring*/
    if (nit_monitor_id == SIM_INVALID_MON_ID)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
        nit_monitor_id = sim_start_monitor(dmx, MONITE_TB_NIT, PSI_NIT_PID, 0);
    }

    /*register callback function*/
    sim_register_scb(nit_monitor_id, auto_ota_nit_callback, NULL);
    m_ota_cur_chan_index = index;

    ota_nit_ext.flag = nit_flag;
    ota_nit_ext.hit = 0;
}

void ota_monitor_off()
{
    if (nit_monitor_id != SIM_INVALID_MON_ID)
    {
        sim_unregister_scb(nit_monitor_id, auto_ota_nit_callback);
        sim_stop_monitor(nit_monitor_id);
        nit_monitor_id = SIM_INVALID_MON_ID;
    }
    if (pat_monitor_id != SIM_INVALID_MON_ID)
    {
        sim_unregister_scb(pat_monitor_id, ota_pat_callback);
        sim_stop_monitor(pat_monitor_id);
        pat_monitor_id = SIM_INVALID_MON_ID;
    }
    if (otapmt_monitor_id != SIM_INVALID_MON_ID)
    {
        sim_unregister_scb(otapmt_monitor_id, ota_pmt_callback);
        sim_stop_monitor(otapmt_monitor_id);
        otapmt_monitor_id = SIM_INVALID_MON_ID;
    }

    if (1 == otapmt_slot_on)
    {
        sim_stop_monitor(otapmt_monitor_id);
        otapmt_monitor_id = SIM_INVALID_MON_ID;
        otapmt_slot_on = 0;
        m_ota_cur_chan_index = 0xffff;
    }
}

#endif

