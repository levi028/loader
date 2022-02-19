/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_frontend.c
*
* Description:
*     ali STB front end device and ts device manage unit
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <hld/nim/nim_dev.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libtsi/db_3l.h>
#include <api/libc/string.h>
#include <bus/tsi/tsi.h>
#include <api/libdiseqc/lib_diseqc.h>
#include <api/libsi/si_tdt.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_as.h>
#include <api/libpub/lib_device_manage.h>
#include <api/libpub/lib_pub.h>
#include <api/libci/ci_plus.h>

#include <hld/nim/nim.h>

#include "lib_cc_inner.h"
/*******************************************************
* macro define
********************************************************/
#define CC_MAX_TS_ROUTE_NUM 5

#define CC_MAX_TSI_NUM  3  /*max tsi num*/
/*******************************************************
* structure and enum ...
********************************************************/

/*******************************************************
* global and local variable
********************************************************/
static double local_longtitude = 0;
static double local_latitude = 0;
static UINT8 quick_channel_change = 0;

static INT8 nim_tsiid[CC_MAX_TSI_NUM] = {TSI_SPI_0, TSI_SPI_1, TSI_SPI_TSG};
static struct ts_route_info g_ts_route_info[CC_MAX_TS_ROUTE_NUM];
static v12set_callback l_v12set_callback = NULL;

/* Get Number of Valid TS Route */
static UINT8 __ts_route_get_valid(void)
{
    UINT16 i = 0;

    for(i = 0; i < CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(0 == g_ts_route_info[i].enable)
        {
            break;
        }
    }

    return i;
}

UINT8 frontend_set_quick_cc_mode(UINT8 mode)
{
    UINT8 pre_mode = quick_channel_change;
    quick_channel_change = mode;
    return pre_mode;
}

/* Get Lock Flag and Scramble Flag of TS Route */
static void __ts_route_check_flag(struct ts_route_info *ts_route)
{
    struct dmx_device *dmx = NULL;
    struct nim_device *nim = NULL;
//    UINT32 scrabled = 0;
    UINT32 scrable_typ = 0;

    if (NULL == ts_route)
    {
        ASSERT(0);
        return;
    }
    //may have problem!
    //MEMSET(&node, 0, sizeof(P_NODE));
    if(0 == (ts_route->state & TS_ROUTE_STATUS_PLAYACK))
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route->dmx_id);
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route->nim_id);

        if(ts_route->lock_chck_en)
        {
            //Check whether NIM Signal is Locked or not
            nim_get_lock(nim, &ts_route->flag_lock);
        }

#if 0 //def CI_PLUS_SUPPORT
        ts_route->flag_scramble = 0;
        P_NODE node;
        MEMSET(&node, 0, sizeof(P_NODE));
        //Check whether TS Stream in DMX is Scrambled or not and Scamble Type
        if(get_prog_by_id(ts_route->prog_id, &node) != SUCCESS)
        {
            return;
        }

        ts_route->flag_scramble = ts_route_is_av_scrambled(dmx, node.video_pid, node.audio_pid[node.cur_audio]);
#else
        //Check whether TS Stream in DMX is Scrambled or not and Scamble Type
        //scrabled = 0;
        if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)(&scrable_typ)))
        {
            ts_route->flag_scramble = (scrable_typ & (VDE_TS_SCRBL|VDE_PES_SCRBL));
        }
#endif
    }

    return;
}

/*******************************************************
* external APIs
********************************************************/

void uich_chg_set12vset_cb(v12set_callback v12setcb)
{
	if(NULL != v12setcb)
	{
    	l_v12set_callback = v12setcb;
    }
}

// copy from "ci_is_av_scrambled" as a common API
BOOL ts_route_is_av_scrambled(struct dmx_device *dmx, UINT16 v_pid, UINT16 a_pid)
{
    BOOL ret = FALSE;
    RET_CODE dmx_state = RET_FAILURE;
    struct io_param param;
    struct io_param_ex param_ex;
    UINT16 pid[2] = {0,};
    UINT8 scramble_type=0;
    UINT32 chip_id = 0xffffffff;

    MEMSET(&param, 0, sizeof(param));
    MEMSET(&param_ex, 0, sizeof(param_ex));
    if ((dmx != NULL) && (INVALID_PID != v_pid)&&(INVALID_PID != a_pid))
    {
        pid[0] = v_pid;
        pid[1] = a_pid;
        chip_id = sys_ic_get_chip_id();

        switch (chip_id)
        {
            case ALI_M3329E:
                param.io_buff_in = (UINT8 *)pid;
                param.io_buff_out= (UINT8 *)&scramble_type;
                dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, (UINT32)&param);
                break;
            case ALI_S3602F:
            case ALI_S3281:
            case ALI_S3503:
                param_ex.io_buff_in = (UINT8 *)pid;
                param_ex.io_buff_out= (UINT8 *)&scramble_type;
                dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, (UINT32)&param_ex);
                break;
            default:
                //for 3602 do not support IS_AV_SCRAMBLED_EXT
                dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)&scramble_type);
                break;
        }

        //fix issue: for AC3 audio is AUD_STR_INVALID
        if((SUCCESS == dmx_state) && ((scramble_type
            & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL)) != 0))
        {
            ret = TRUE;
        }
    }

    return ret;
}



//set Unicable
void frontend_set_unicable(struct nim_device *nim,struct ft_antenna *antenna,union ft_xpond *xpond)
{   
    UINT8 __MAYBE_UNUSED__ band = 0;
    UINT16 __MAYBE_UNUSED__ freq = 0;
    
    if ((NULL==nim) || (NULL==antenna) || (NULL==xpond))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }

#ifdef SAT2IP_SUPPORT
    if (ts_route_is_sat2ip_dmx(nim))
    {
        return;
    }
#endif

    if (LNB_CTRL_UNICABLE == antenna->lnb_type )
    {   
        if (xpond->s_info.frq > KU_LNB_22K_EDGE_FREQUENCY)
        {
            band = 1;
            freq = antenna->lnb_high;
        }
        else
        {
            band = 0;
            freq = antenna->lnb_low;
        }
        api_diseqc_unicable_channel_change(nim, antenna->unicable_ub, antenna->unicable_freq, \
                            antenna->unicable_pos, \
                            1-xpond->s_info.pol, band, xpond->s_info.frq-freq);
//    osal_task_sleep(50);
    }
}

/* Set NIM Info */
void frontend_set_nim(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set)
{
    struct NIM_CHANNEL_CHANGE nim_param;
    INT32 tuning_frq = 0;
    struct nim_config config;
    struct t_diseqc_info d_info;
    UINT8 pol = 0;
#ifdef MULTISTREAM_SUPPORT
    struct nim_dvbs_isid nim_isid;
#endif

    if ((NULL==nim) || (NULL==xpond))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }

#ifdef SAT2IP_SUPPORT
    if (ts_route_is_sat2ip_dmx(nim))
    {
        return;
    }
#endif

    MEMSET(&nim_param, 0, sizeof(nim_param));
    MEMSET(&d_info, 0, sizeof(d_info));
    MEMSET(&config, 0, sizeof(config));
#ifdef MULTISTREAM_SUPPORT
    MEMSET(&nim_isid, 0, sizeof(struct nim_dvbs_isid));
#endif
//Get Old NIM Config Info
    dev_get_nim_config(nim, xpond->common.type, &config);


/* DVB-S Frontend, Antenna Config is Valid*/
/* Tuning DVB-S NIM */
    //if((antenna != NULL) && (antenna->antenna_enable == 1))
    //change on 2012-02-14 for no signal after auto scan (set win_miscset_save_setting -> antenna->antenna_enable = 0)
    if((FRONTEND_TYPE_S == xpond->common.type) && (antenna != NULL))
    {
        pol = 0 == antenna->pol? (xpond->s_info.pol+1):antenna->pol;
        //if((!l_cc_local_ismutex)||(pAntenna->positioner_type == 0))
        {
            d_info.sat_or_tp = 1;
            d_info.diseqc_type = antenna->di_seq_c_type;
            d_info.diseqc_port = antenna->di_seq_c_port;
            d_info.diseqc11_type = antenna->di_seq_c11_type;
            d_info.diseqc11_port = antenna->di_seq_c11_port;
            if(LNB_CTRL_22K == antenna->lnb_type)
            {
                if(xpond->s_info.frq >= KU_LNB_22K_EDGE_FREQUENCY)
                {
                    d_info.diseqc_k22 = 1;
                }
                else
                {
                    d_info.diseqc_k22 = 0;
                }
            }
            else
            {
                d_info.diseqc_k22 = antenna->k22;
            }

            d_info.diseqc_polar = pol;
            d_info.diseqc_toneburst = antenna->toneburst;   /* 0: off, 1: A, 2: B */
            api_diseqc_operation(nim, &d_info, force_set);
        }

        switch(antenna->lnb_type)
        {
            case LNB_CTRL_22K:
                if(xpond->s_info.frq > KU_LNB_22K_EDGE_FREQUENCY)
                {
                    api_diseqc_set_22k(nim, 1);
                    tuning_frq = xpond->s_info.frq - ((antenna->lnb_high > antenna->lnb_low)
                        ? antenna->lnb_high : antenna->lnb_low);
                }
                else
                {
                    api_diseqc_set_22k(nim, 0);
                    tuning_frq = xpond->s_info.frq - ((antenna->lnb_high > antenna->lnb_low)
                        ? antenna->lnb_low : antenna->lnb_high);
                }
            break;
            case LNB_CTRL_POL:
                pol = ((0 == antenna->pol) || (3 == antenna->pol)) ? (xpond->s_info.pol + 1) : (antenna->pol);
                if(FLAG_TRUE == pol)
                {
                    tuning_frq = antenna->lnb_low - xpond->s_info.frq;
                }
                else
                {
                    tuning_frq = antenna->lnb_high - xpond->s_info.frq;
                }
                break;
            case LNB_CTRL_UNICABLE:
//                    frontend_set_unicable(nim,antenna,xpond);
                tuning_frq = antenna->unicable_freq;
                break;
            case LNB_CTRL_STD:
            default:
                tuning_frq = (((antenna->lnb_low != 0) ? antenna->lnb_low : antenna->lnb_high)) - xpond->s_info.frq;
                break;
        }

        if(tuning_frq < 0)
        {
            tuning_frq = -tuning_frq;
        }

        nim_param.sym = xpond->s_info.sym;
    }
    else
    {
        /* Tuning DVB-C NIM */
        if(FRONTEND_TYPE_C == xpond->common.type)
        {
            tuning_frq = xpond->c_info.frq;
            nim_param.sym = xpond->c_info.sym;
            nim_param.modulation = xpond->c_info.modulation;
        }
        /* Tuning DVB-T NIM */
        else if ((FRONTEND_TYPE_T == xpond->common.type)||(FRONTEND_TYPE_ISDBT == xpond->common.type))
        {
            tuning_frq = xpond->t_info.frq;
            nim_param.sym = xpond->t_info.sym;
            nim_param.fec = xpond->t_info.fec;
            nim_param.guard_interval = xpond->t_info.guard_intl;
            nim_param.fft_mode = xpond->t_info.fft_mode;
            nim_param.modulation = xpond->t_info.modulation;
            nim_param.usage_type = xpond->t_info.usage_type;
            nim_param.inverse = xpond->t_info.inverse;
            nim_param.bandwidth = xpond->t_info.band_width/1000;

            //(DVBT2_SUPPORT)
            nim_param.t2_signal = xpond->t_info.t2_signal;
            nim_param.plp_index = xpond->t_info.plp_index;
            //For play program according to the plp_id, and regardless which plp_index it's broadcasted with.
            nim_param.plp_id = xpond->t_info.plp_id;
            //Keep plp_num for support USAGE_TYPE_NEXT_PIPE_SCAN.
            nim_param.plp_num = xpond->t_info.plp_num;
            nim_param.t2_profile = xpond->t_info.t2_profile;
            
            if((UINT8)USAGE_TYPE_CHANCHG == nim_param.usage_type)//play channle
            {
                nim_param.priority = xpond->t_info.priority;
            }
            else //scan
            {
                nim_param.priority = 0;
            }
        }
    }
    nim_param.freq = tuning_frq;

#ifdef MULTISTREAM_SUPPORT  
    if(xpond->common.type == FRONTEND_TYPE_S)
    {
        nim_param.change_type = xpond->s_info.change_type;
        //libc_printf("%s(), tp_id: %d,change_type: %d, isid: %d\n\n",__FUNCTION__,xpond->s_info.tp_id,
        //                  nim_param.change_type, xpond->s_info.isid);
        if((1 == nim_param.change_type) || (2 == nim_param.change_type))
        {
            nim_isid.isid_write = xpond->s_info.isid;
            nim_isid.isid_bypass = 0;
            nim_isid.isid_mode = 1;
            nim_param.isid = &nim_isid;
            //libc_printf("isid: %d\n",xpond->s_info.isid);
        }       
    }
#endif

#ifdef PLSN_SUPPORT
    nim_param.super_scan = xpond->s_info.super_scan;
    nim_param.plsn_db = xpond->s_info.pls_num;
#endif

     if(nim->do_ioctl_ext)
     {
        #ifdef FSC_SUPPORT
        if(quick_channel_change)
        {
            nim->do_ioctl_ext(nim, NIM_DRIVER_QUICK_CHANNEL_CHANGE, (void *)(&nim_param));
        }
        else
        {
            nim->do_ioctl_ext(nim, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&nim_param));
        }
        #else
            nim->do_ioctl_ext(nim, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&nim_param));
        #endif

        #if 0//def FSC_SUPPORT
        //for support 3503A fsc tuner4 
        struct nim_device *demod_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 8);
        if(NULL != demod_dev)
        {
            demod_dev->do_ioctl_ext(demod_dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&nim_param));
        }
        #endif
    }

#if defined(DVBT2_SUPPORT)
    xpond->t_info.t2_signal = nim_param.t2_signal;
    xpond->t_info.plp_index = nim_param.plp_index;
    xpond->t_info.plp_id = nim_param.plp_id;
    xpond->t_info.t2_system_id = nim_param.t2_system_id;
    xpond->t_info.plp_num = nim_param.plp_num;
    xpond->t_info.t2_profile = nim_param.t2_profile;
#endif

//Store New NIM Config Info
    MEMCPY(&config.xpond, xpond, sizeof(union ft_xpond));
    dev_set_nim_config(nim, xpond->common.type, &config);
}


/* Set Antenna Info */
void frontend_set_antenna(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set)
{
    double __MAYBE_UNUSED__ sat_obit = 0;
    double longtitude = 0;
    double latitude = 0;
    UINT8 tp_pol = 0;
    struct t_diseqc_info d_info;
    struct nim_config config;

    if ((NULL==nim) || (NULL==xpond) || (NULL == antenna))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
#ifdef SAT2IP_SUPPORT
    if (ts_route_is_sat2ip_dmx(nim))
    {
        return;
    }
#endif
    MEMSET(&d_info, 0, sizeof(struct t_diseqc_info));
    MEMSET(&config, 0, sizeof(struct nim_config));
    //if((antenna != NULL) && (antenna->antenna_enable == 1))
    //change on 2012-02-13 for dvbs not set this param (set win_miscset_save_setting -> antenna->antenna_enable=0)
    if((FRONTEND_TYPE_S == xpond->common.type) && (antenna != NULL))
    {

        if (l_v12set_callback != NULL)
        {
            l_v12set_callback(antenna->v12);
        }

    //Get Old NIM Config
        if(SUCCESS != dev_get_nim_config(nim, FRONTEND_TYPE, &config))
        {
            return;
        }

    //Store New NIM Config
        MEMCPY(&config.antenna, antenna, sizeof(struct ft_antenna));
        dev_set_nim_config(nim, FRONTEND_TYPE, &config);

    //Set Diseqc Info and Operate
        tp_pol = xpond->s_info.pol + 1;

        d_info.sat_or_tp = 0;
        d_info.diseqc_type = antenna->di_seq_c_type;
        d_info.diseqc_port = antenna->di_seq_c_port;
        d_info.diseqc11_type = antenna->di_seq_c11_type;
        d_info.diseqc11_port = antenna->di_seq_c11_port;
        d_info.diseqc_toneburst = antenna->toneburst;
        d_info.diseqc_polar = (0 == antenna->pol) ? (tp_pol) : (antenna->pol);
        if(LNB_CTRL_22K == antenna->lnb_type)
        {
            if(xpond->s_info.frq >= KU_LNB_22K_EDGE_FREQUENCY)
            {
                d_info.diseqc_k22 = 1;
            }
            else
            {
                d_info.diseqc_k22 = 0;
            }
        }
        else
        {
            d_info.diseqc_k22 = antenna->k22;
        }

        /* Set Positioner Type Motor */
        d_info.positioner_type = antenna->positioner_type;
        /* Diseqc1.2 Motor */
        if(POSITIONER_TYPE_DISEQC_12 == antenna->positioner_type)
        {
            d_info.position = antenna->position;
        }
        /* Usal Motor */
        else if(POSITIONER_TYPE_DISEQC_13 == antenna->positioner_type)
        {
            sat_obit = api_usals_get_sat_orbit(antenna->longitude);
            chchg_get_local_position(&longtitude, &latitude);
            d_info.wxyz = api_usals_xx_to_wxyz(api_usals_target_formula_calcuate(sat_obit, longtitude, latitude));
        }
        else
        {
            d_info.positioner_type = 0;
        }
    //Operate Diseqc
        api_diseqc_operation(nim, &d_info, force_set);
    //set Unicable LNB if exist
        if (LNB_CTRL_UNICABLE == antenna->lnb_type)
        {
            frontend_set_unicable(nim,antenna,xpond);
        }
    }
}

/* Set Antenna and NIM Info */
void frontend_tuning(struct nim_device *nim, struct ft_antenna *antenna,
    union ft_xpond *xpond, UINT8 force_set)
{
    //antenna may be NULL. 
	if((NULL != nim) && (NULL != xpond))
	{
	    frontend_set_antenna(nim, antenna, xpond, force_set);
	    frontend_set_nim(nim, antenna, xpond, force_set);
	}
}

/* Set TS Route Info */
void frontend_set_ts_route(struct nim_device *nim, struct ft_ts_route *route)
{

    if((NULL == route) || (0 == route->ts_route_enable) ||(NULL == nim))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }

//Set TS Input Source Attribute
    //tsi_mode_set(route->tsiid, route->tsi_mode); // move to system_hw_init
//Select TS Input Source for Output TS of TSI
    tsi_select(route->ts_id, route->tsiid);
//Select TSI Output TS for DMX
    tsi_dmx_src_select(route->dmx_id, route->ts_id);
    //Set TS Pass CI CAM or Bypass
  #if 0
    struct dmx_device *dmx = NULL;

    if(route->ci_enable)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, route->dmx_id);
        //If needed, Set NIM TS Mode and Reset DMX
        if((nim != NULL) && (dmx != NULL))
        {

        }
    }
  #endif
}

/* Check whether NIM Param is changed or not */
BOOL frontend_check_nim_param(struct nim_device *nim, union ft_xpond *xpond, struct ft_antenna *antenna)
{
    BOOL ret = FALSE;
    INT32 freq = 0;
    UINT8 fec = 0;
    UINT32 sym = 0;
    UINT32 tmp = 0;
    UINT8 pol = 0;
    INT32 tuning_frq = 0;
    const INT32 dvbs_freq_low_threshold = 2;
    const INT32 dvbs_freq_high_threshold = 150;
    const INT32 dvbc_freq_threshold = 100;
    const INT32 dvbt_freq_threshold = 100;

    if((NULL == nim) || (NULL == xpond))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return ret;
    }

    switch (xpond->common.type)
    {
        case FRONTEND_TYPE_S:
        {
            if (NULL == antenna)
            {
                PUB_PRINT_LINE("parameter_error!\n");
                return ret;
            }
        //Calculate Tuning Frequency according to Antenna Info
            switch(antenna->lnb_type)
            {
            case LNB_CTRL_22K:
                if(xpond->s_info.frq > KU_LNB_22K_EDGE_FREQUENCY)
                {
                    tuning_frq = xpond->s_info.frq - ((antenna->lnb_high > antenna->lnb_low) \
                        ? antenna->lnb_high : antenna->lnb_low);
                }
                else
                {
                    tuning_frq = xpond->s_info.frq - ((antenna->lnb_high > antenna->lnb_low) \
                        ? antenna->lnb_low : antenna->lnb_high);
                }
            break;
            case LNB_CTRL_POL:
                pol = ((0 == antenna->pol) || (3 == antenna->pol)) ? (xpond->s_info.pol + 1) : (antenna->pol);
                if(FLAG_TRUE == pol)
                {
                    tuning_frq = antenna->lnb_low - xpond->s_info.frq;
                }
                else
                {
                    tuning_frq = antenna->lnb_high - xpond->s_info.frq;
                }
                break;
            case LNB_CTRL_UNICABLE:
                tuning_frq = antenna->unicable_freq;
                break;
            case LNB_CTRL_STD:
            default:
                tuning_frq = (((antenna->lnb_low != 0) ? antenna->lnb_low : antenna->lnb_high)) - xpond->s_info.frq;
                break;
            }

            if(tuning_frq < 0)
            {
                tuning_frq = -tuning_frq;
            }

            //Get NIM Freq
            //param "1" mean that cur freq is that user latest setting, not realy work freq
            freq = nim_io_control(nim, NIM_DRIVER_GET_CUR_FREQ, 1);

            //Check Freq Freq Offset within 2MHz, Same TP
            //NOTE: unsigned subtract
            if (freq >= tuning_frq)
            {
                tmp = freq - tuning_frq;
            }
            else
            {
                tmp = tuning_frq - freq;
            }

            if(tmp > (UINT32)dvbs_freq_low_threshold)
            {
                ret = TRUE;
            }
            else
            {
                if(NULL == nim->do_ioctl)
                {
                    return ret;
                }
                nim->do_ioctl(nim, NIM_DRIVER_GET_SYM, (UINT32)&sym);
                //Sym Rate Offset within 150, Same TP
                //NOTE: unsigned subtract
                if (sym >= xpond->s_info.sym)
                {
                    tmp = sym - xpond->s_info.sym;
                }
                else
                {
                    tmp = xpond->s_info.sym - sym;
                }
                if(tmp > (UINT32)dvbs_freq_high_threshold)
                {
                    ret = TRUE;
                }
            }
        }
        break;
        case FRONTEND_TYPE_C:
            //Get NIM Freq
            if(SUCCESS != nim_get_freq(nim, (UINT32 *)&freq))
            {
                return ret;
            }

            //Freq Offset within 1MHz, Same TP
            if((UINT32)freq >= xpond->c_info.frq)
            {
                tmp = freq - xpond->c_info.frq;
            }
            else
            {
                tmp = xpond->c_info.frq - freq;
            }
            if(tmp > (UINT32)dvbc_freq_threshold)
            {
                ret = TRUE;
            }
            //Check modulation
            else
            {
                if(SUCCESS != nim_get_fec(nim, &fec))
                {
                    return ret;
                }
                if(fec != xpond->c_info.modulation)
                {
                    ret = TRUE;
                }
            }
            break;
	case FRONTEND_TYPE_ISDBT:
        case FRONTEND_TYPE_T:
            if(SUCCESS != nim_get_freq(nim, (UINT32 *)&freq))
            {
                return ret;
            }
            if((UINT32)freq >= xpond->t_info.frq)
            {
                tmp = freq - xpond->t_info.frq;
            }
            else
            {
                tmp = xpond->t_info.frq - freq;
            }

            if(tmp>(UINT32)dvbt_freq_threshold)
            {
                ret = TRUE;

            }
            else
            {
                ret = FALSE;
            }
            break;
        default:
            break;

    }

    return ret;
}

/* Get Status of NIM or DMX Device in TS Route */
void _ts_route_get_dev_state(UINT32 hld_type, void *dev, UINT32 *status)
{
    UINT16 i = 0;
    UINT8 dev_id = 0;

    if((NULL==dev) || (NULL==status))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    switch(hld_type)
    {
    //Get NIM Device Status
    case HLD_DEV_TYPE_NIM:
        dev_id = (((struct nim_device *)dev)->type) & HLD_DEV_ID_MASK;
        *status = DEV_FRONTEND_STATUS_PLAY;
        for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
        {
            if((1 == g_ts_route_info[i].enable) && (g_ts_route_info[i].nim_id == dev_id))
            {
                if(g_ts_route_info[i].state & TS_ROUTE_STATUS_RECORD)
                {
                    *status = DEV_FRONTEND_STATUS_RECORD;
                    break;
                }
            }
        }
        break;
    //Get DMX Device Status
    case HLD_DEV_TYPE_DMX:
        dev_id = (((struct dmx_device *)dev)->type) & HLD_DEV_ID_MASK;
        *status = DEV_DMX_STATUS_PLAY;
        for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
        {
            if((1 == g_ts_route_info[i].enable) && (g_ts_route_info[i].dmx_id == dev_id))
            {
                if(g_ts_route_info[i].state & TS_ROUTE_STATUS_RECORD)
                {
                    *status = DEV_DMX_STATUS_RECORD;
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    return;
}

/* Get DMX Config Info in TS Route */
void _ts_route_get_dmx_config(struct dmx_device *dmx, struct dmx_config *config)
{
    UINT16 i = 0;
    UINT16 j = 0;
    UINT8 dev_id = 0;
    struct ts_route_info *info = NULL;

    if((NULL == dmx) || (NULL==config))
    {
        ASSERT(0);
        return;
    }

    MEMSET(config, 0, sizeof(struct dmx_config));
    dev_id = (dmx->type) & HLD_DEV_ID_MASK;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if((1 == info->enable) && (info->dmx_id == dev_id))
        {
            config->tp_id = info->tp_id;
            if(info->sim_num)
            {
                MEMCPY(&config->dmx_sim_info[j], &(info->dmx_sim_info[0]), info->sim_num * sizeof(struct dmx_sim));
                j += info->sim_num;
            }
        }
    }

    return;
}

/* Create TS Route */
UINT32 ts_route_create(struct ts_route_info *ts_route)
{
    UINT32 ret = 0;

    if (NULL == ts_route)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return ret;
    }

    //Get TS Route Number for Check whether TS Route is Valid or not
    ret = __ts_route_get_valid();
    if(ret >= CC_MAX_TS_ROUTE_NUM)
    {
        return ret;
    }

    //Copy TS Route Info
    MEMSET(&g_ts_route_info[ret], 0, sizeof(struct ts_route_info));
    MEMCPY(&g_ts_route_info[ret], ts_route, sizeof(struct ts_route_info));
    g_ts_route_info[ret].enable = 1;
    g_ts_route_info[ret].id = ret;
    g_ts_route_info[ret].flag_lock = 1;
    g_ts_route_info[ret].lock_chck_en = 1;
    g_ts_route_info[ret].flag_scramble = 0;

    return ret;
}

/* Create TS Route */
UINT32 ts_route_create_ext(struct ts_route_info *ts_route,struct ts_route_config *pconfig)
{
    UINT32 ret = 0;

    if (NULL == ts_route)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return ret;
    }

    ret = ts_route_create(ts_route);
    if(ret >= CC_MAX_TS_ROUTE_NUM)
    {
        return ret;
    }
    
    if(pconfig->lock_chck_en)
    {
        g_ts_route_info[ret].lock_chck_en = 1;
    }
    else
    {
        g_ts_route_info[ret].lock_chck_en = 0;
    }

    return ret;
}

/* Delete TS Route */
RET_CODE ts_route_delete(UINT32 id)
{
    if (id >= CC_MAX_TS_ROUTE_NUM)
    {
        return RET_FAILURE;
    }
    if(g_ts_route_info[id].enable)
    {
        MEMSET(&g_ts_route_info[id], 0, sizeof(struct ts_route_info));
    }
    return RET_SUCCESS;
}

/* Get TS Route Info according to TS Route ID */
UINT8 ts_route_get_num(void)
{
    UINT8 ret = 0;
    UINT16 i = 0;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(1 == g_ts_route_info[i].enable)
        {
            ret++;
        }
    }

    return ret;
}

/* Get TS Route Info according to TS Route ID */
RET_CODE ts_route_get_by_id(UINT32 id, struct ts_route_info *ts_route)
{
    if ((id >= CC_MAX_TS_ROUTE_NUM) || (NULL == ts_route))
    {
        return RET_FAILURE;
    }
  
    MEMCPY(ts_route, &g_ts_route_info[id], sizeof(struct ts_route_info));
	if(1 == ts_route->enable )
	{
		__ts_route_check_flag(ts_route);
	}
    return RET_SUCCESS;
}

/* Get TS Route ID and Info according to TS Route Type */
RET_CODE ts_route_get_by_type(UINT32 type, UINT32 *ts_route_id, struct ts_route_info *ts_route)
{
    RET_CODE ret = RET_FAILURE;
    UINT16 i = 0;
    struct ts_route_info *info = NULL;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if((1 == info->enable) && (info->type == type))
        {
            if(ts_route_id != NULL)
            {
                *ts_route_id = i;
            }
            if(ts_route!= NULL)
            {
                MEMCPY(ts_route, info, sizeof(struct ts_route_info));
                if(ts_route->bstoping == 0)
                {
                    __ts_route_check_flag(ts_route);
                }
            }
            ret = RET_SUCCESS;
            break;
        }
    }

    return ret;
}

/* Get TS Route ID and Info of Prog which is Recording according to Prog ID */
RET_CODE ts_route_get_record(UINT32 prog_id, UINT32 *ts_route_id, struct ts_route_info *ts_route)
{
    RET_CODE ret = RET_FAILURE;
    UINT16 i = 0;
    struct ts_route_info *info = NULL;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if((1 == info->enable) && (1 == info->is_recording) && (info->prog_id == prog_id))
        {
            if(ts_route_id != NULL)
            {
                *ts_route_id = i;
            }
            if(ts_route!= NULL)
            {
                MEMCPY(ts_route, info, sizeof(struct ts_route_info));
                if(ts_route->bstoping == 0)
                {
                    __ts_route_check_flag(ts_route);
                }
            }
            ret = RET_SUCCESS;
            break;
        }
    }

    return ret;
}

/* Update TS Route */
RET_CODE ts_route_update(UINT32 id, struct ts_route_info *ts_route)
{
    if (NULL == ts_route)
    {
        return RET_FAILURE;
    }
    if (id >= CC_MAX_TS_ROUTE_NUM)
    {
        return RET_FAILURE;
    }
    if(1 == g_ts_route_info[id].enable)
    {
        MEMCPY(&g_ts_route_info[id], ts_route, sizeof(struct ts_route_info));
    }
    return RET_SUCCESS;
}

/* Get TS Route ID of TS Route which Use Right CI Slot */
INT16 ts_route_check_ci(UINT8 ci_id)
{
    INT16 ret = -1;
    UINT16 i = 0;
    BOOL ci_serial_mode = FALSE;
    BOOL ci_parallel_mode = FALSE;
    struct ts_route_info *info = NULL;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if(1 == info->enable)
        {
            ci_serial_mode = FALSE;
            ci_parallel_mode = FALSE;
            if ((CI_SERIAL_MODE == info->ci_mode) && ((info->cia_used) || (info->cib_used)))
            {
                ci_serial_mode = TRUE;
            }
            if ((CI_PARALLEL_MODE==info->ci_mode)&&((0==ci_id) ? info->cia_used : ((1==ci_id) ? info->cib_used : 0)))
            {
                ci_parallel_mode = TRUE;
            }

            if (ci_serial_mode || ci_parallel_mode)
            {
                ret = i;
                break;
            }

        }
    }

    return ret;
}

/* Get ID and Number of All TS Routes which Include Right CI Slot and
Get ID of TS Route which Use Right CI Slot Currently */
INT16 ts_route_check_ci_route(UINT8 ci_id, UINT8 *route_num, UINT16 *routes)
{
    INT16 ret = -1;
    UINT16 i = 0;
    BOOL ci_serial_mode = FALSE;
    BOOL ci_parallel_mode = FALSE;
    struct ts_route_info *info = NULL;

    if ((NULL==route_num) || (NULL==routes))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return -1;
    }
    *route_num = 0;
    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if(FLAG_TRUE == info->enable)
        {
            ci_serial_mode = FALSE;
            ci_parallel_mode = FALSE;
            if ((CI_SERIAL_MODE == info->ci_mode) && ((info->cia_included) || (info->cib_included)))
            {
                ci_serial_mode = TRUE;
            }
            if ((CI_PARALLEL_MODE==info->ci_mode)&&((0==ci_id)?info->cia_included:((1==ci_id)?info->cib_included:0)))
            {
                ci_parallel_mode = TRUE;
            }

            if (ci_serial_mode || ci_parallel_mode)
            {
                routes[*route_num] = i;
                (*route_num)++;
                ci_serial_mode = FALSE;
                ci_parallel_mode = FALSE;
                if ((CI_SERIAL_MODE == info->ci_mode) && ((info->cia_used) || (info->cib_used)))
                {
                    ci_serial_mode = TRUE;
                }
                if ((CI_PARALLEL_MODE == info->ci_mode)
                    && ((0==ci_id)?info->cia_included:((1==ci_id)?info->cib_included:0)))
                {
                    ci_parallel_mode = TRUE;
                }

                if (ci_serial_mode || ci_parallel_mode)
                {
                    ret = i;
                }
            }

        }
    }

    return ret;
}

/* Get TS ID of Matched TS Route according to Input DMX ID */
UINT8 ts_route_check_ts_by_dmx(UINT8 dmx_id)
{
    UINT8 ret = 0;
    UINT16 i = 0;

    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(1 == g_ts_route_info[i].enable)
        {
            if(g_ts_route_info[i].dmx_id == dmx_id)
            {
                ret = g_ts_route_info[i].ts_id;
                break;
            }
        }
    }

    return ret;
}

/* Get ID and Number of Matched TS Route according to Input DMX ID */
void ts_route_check_dmx_route(UINT8 dmx_id, UINT8 *route_num, UINT16 *routes)
{
    UINT16 i= 0;

    if ((NULL==route_num) || (NULL==routes))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    *route_num = 0;
    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(1 == g_ts_route_info[i].enable)
        {
            if(g_ts_route_info[i].dmx_id == dmx_id)
            {
                routes[*route_num] = i;
                (*route_num)++;
            }
        }
    }
}

/* Get ID and Number of Matched TS Route according to Input TP ID */
void ts_route_check_tp_route(UINT32 tp_id, UINT8 *route_num, UINT16 *routes)
{
    UINT16 i = 0;

    if ((NULL==route_num) || (NULL==routes))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    *route_num = 0;
    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(1 == g_ts_route_info[i].enable)
        {
            if(g_ts_route_info[i].tp_id == tp_id)
            {
                routes[*route_num] = i;
                (*route_num)++;
            }
        }
    }
}

void ts_route_check_nim_route(UINT32 nim_id, UINT8 *route_num, UINT16 *routes)
{
    UINT16 i = 0;

    if ((NULL==route_num) || (NULL==routes))
    {
        PUB_PRINT_LINE("parameter error!\n");
        return;
    }
    *route_num = 0;
    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        if(1 == g_ts_route_info[i].enable)
        {
            if(g_ts_route_info[i].nim_id == nim_id)
            {
                routes[*route_num] = i;
                (*route_num)++;
            }
        }
    }
}

UINT32 ts_route_get_by_prog_id(UINT32 prog_id, UINT8 dmx_id,  UINT8 is_pip, struct ts_route_info *ts_route)
{
    UINT8 i = 0;
    RET_CODE ret = RET_FAILURE;
    struct ts_route_info *info = NULL;

    if (NULL==ts_route)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return ret;
    }
    for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
    {
        info = &g_ts_route_info[i];
        if(1 == info->enable)
        {
            if((TS_ROUTE_PIP_PLAY == info->type) && (is_pip) && (info->prog_id == prog_id))
            {
                break;
            }
            if((info->type != TS_ROUTE_PIP_PLAY) && (!is_pip) && (info->prog_id == prog_id) && (info->dmx_id == dmx_id))
            {
                break;
            }
        }
    }

    if(i >= CC_MAX_TS_ROUTE_NUM)
    {
        ret = RET_FAILURE;
    }
    else
    {
        MEMCPY(ts_route, &g_ts_route_info[i], sizeof(struct ts_route_info));
        ret = RET_SUCCESS;
    }
    return ret;
}
UINT8 ts_route_set_ci_info(UINT32 sim_id,CA_INFO *ca_info,UINT8 ca_count)
{
    UINT8 i = 0;

    if (NULL==ca_info)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return RET_FAILURE;
    }
    for (i=0;i<5;i++)
    {
        if((g_ts_route_info[i].enable)&&(g_ts_route_info[i].dmx_sim_info[0].sim_id == sim_id))
        {
            MEMCPY(g_ts_route_info[i].ca_info,ca_info,ca_count*sizeof(CA_INFO));
            g_ts_route_info[i].ca_count = ca_count;
            g_ts_route_info[i].ca_info_enable= 1;
            return RET_SUCCESS;
        }
    }
    return RET_FAILURE;
}

UINT8 is_ca_info_in_ts_route(UINT32 sim_id)
{
    UINT8 i = 0;

    for (i=0;i<5;i++)
    {
        if((g_ts_route_info[i].dmx_sim_info[0].sim_id == sim_id)&&(1 == g_ts_route_info[i].ca_info_enable))
        {
            return RET_SUCCESS;
        }
    }
    return RET_FAILURE;
}

UINT8 ts_route_get_by_sim_id(UINT32 sim_id,UINT8 is_pip,    struct ts_route_info *ts_route)
{
    UINT8 i = 0;
    UINT8 j = 0;
    struct ts_route_info *info = NULL;

    if (NULL == ts_route)
    {
        PUB_PRINT_LINE("parameter error!\n");
        return RET_FAILURE;
    }
    for(i=0;i<CC_MAX_TS_ROUTE_NUM;i++)
    {
        info = &g_ts_route_info[i];
        if (!info->enable)
        {
            continue;
        }
        for(j=0;j<DEV_DMX_SIM_CNT>>1;j++)
        {
            if ((info->dmx_sim_info[j].sim_id ==sim_id)\
                &&(TS_ROUTE_PIP_PLAY==info->type)&&(1 == is_pip))
            {
                MEMCPY(ts_route, info ,sizeof(struct ts_route_info));
                return RET_SUCCESS;
            }
            if ((info->dmx_sim_info[j].sim_id ==sim_id)
            && ((TS_ROUTE_MAIN_PLAY == info->type)||(TS_ROUTE_BACKGROUND == info->type))
            && (0 == is_pip))
            {
                MEMCPY(ts_route, info, sizeof(struct ts_route_info));
                return RET_SUCCESS;
            }
        }
    }
    MEMSET(ts_route,0,sizeof(struct ts_route_info));
    return RET_FAILURE;
}
/*
add for media provider
check all record
*/

void ts_route_check_record_route_all(UINT16 *route_num)
{
    INT32 i = 0;
    struct ts_route_info *info = NULL;

    if(route_num != NULL)
    {
        *route_num = 0;
        for(i = 0; i < CC_MAX_TS_ROUTE_NUM; i++)
        {
            info = &g_ts_route_info[i];
            if(1 == info->enable)
            {
                if (((TS_ROUTE_MAIN_PLAY == info->type) || (TS_ROUTE_BACKGROUND == info->type))
                && (info->state & TS_ROUTE_STATUS_RECORD)
                && (info->is_recording))
                {
                    (*route_num)++;
                }
            }
        }
    }
}


#ifdef COMBO_CA_PVR
UINT8 ts_route_check_recording_by_nim_id(UINT32 nim_id)
{
    UINT8 route = 0;
    UINT8 slot = 0;
    struct ts_route_info ts_route;

    for(route=0;route<CC_MAX_TS_ROUTE_NUM;route++)
    {
        MEMSET(&ts_route,0,sizeof(struct ts_route_info));
        if(ts_route_get_by_id(route,&ts_route) == RET_FAILURE)
        {
            continue;
        }
        if(!ts_route.enable)
        {
            continue;
        }
        if(ts_route.nim_id == nim_id && ts_route.is_recording == TRUE && (ts_route.state & TS_ROUTE_STATUS_RECORD))
        {
            //libc_printf("ts_route_check_recording_by_nim_id => RET_SUCCESS\n");
            return RET_SUCCESS;
        }
    }
    //libc_printf("ts_route_check_recording_by_nim_id => RET_FAILURE\n");
    return RET_FAILURE;
}
#endif

INT32 ts_route_get_max_num(void)
{
    return CC_MAX_TS_ROUTE_NUM;
}

INT8 ts_route_get_nim_tsiid(UINT8 nim_id)
{
	if(CC_MAX_TSI_NUM < nim_id)
	{
		return -1;
	}
    return nim_tsiid[nim_id];
}

void ts_route_set_nim_tsiid(UINT8 nim_id, INT8 tsiid)
{
	if(CC_MAX_TSI_NUM > nim_id)
	{
		nim_tsiid[nim_id] = tsiid;	
	} 
}

void chchg_set_local_position(double longtitude,double latitude)
{
    ENTER_PUB_API();
    local_longtitude = longtitude;
    local_latitude = latitude;
    LEAVE_PUB_API();
}

void chchg_get_local_position(double *longtitude,double *latitude)
{
    if ((NULL == longtitude)||(NULL == latitude))
    {
        ASSERT(0);
        return;
    }
    ENTER_PUB_API();
    *longtitude = local_longtitude;
    *latitude = local_latitude;
    LEAVE_PUB_API();
}


#ifdef SAT2IP_SUPPORT
//get sat2ip ts route
BOOL ts_route_get_by_sat2ip(struct ts_route_info *ts_route)
{
    int i = 0;

    if (ts_route)
    {
        for (i = 0; i < CC_MAX_TS_ROUTE_NUM; i ++)
        {
            if (g_ts_route_info[i].enable && g_ts_route_info[i].sat2ip_flag)
            {
                MEMCPY(ts_route, &g_ts_route_info[i], sizeof(struct ts_route_info));
                return TRUE;
            }
        }
    }
    return FALSE;
}

// check if the dmx device is sat2ip nim device.
BOOL ts_route_is_sat2ip_dmx(struct nim_device *nim_dev)
{
    int i = 0;
    struct nim_device *nim = NULL;

    if (NULL == nim_dev)
    {
        return FALSE;
    }
    for (i = 0; i < CC_MAX_TS_ROUTE_NUM; i ++)
    {
        if (g_ts_route_info[i].enable && g_ts_route_info[i].sat2ip_flag)
        {
            nim = dev_get_by_id(HLD_DEV_TYPE_DMX, g_ts_route_info[i].dmx_id);
            if (nim == nim_dev)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#endif

