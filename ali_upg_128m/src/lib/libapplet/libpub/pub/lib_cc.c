/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_cc.c
*
* Description:
*     process the program channel change
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/cic/cic.h>
#include <api/libci/ci_plus.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_device_manage.h>

#include <bus/tsi/tsi.h>
#include<api/libsi/lib_epg.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/lib_nvod.h>
#include <api/libsi/sie_monitor.h>
#include <hld/sto/sto.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#include <asm/chip.h>

#include <api/libtsg/lib_tsg.h>

#ifdef MULTI_CAS
#include <api/libcas/mcas.h>
#endif

#ifdef CC_USE_TSG_PLAYER
#include <api/libpvr/lib_pvr.h>
#endif

#if (defined(_MHEG5_V20_ENABLE_) )
#include <mh5_api/mh5_api.h>
#endif

#ifdef SUPPORT_CAS9
#include <api/libcas/cas9/cas9.h>
#endif
#include <api/libpub/lib_cc.h>
#include "lib_cc_inner.h"


/*******************************************************
* macro define
********************************************************/
#define NULL_PID 8191
#define MAX_VALID_PID 8190
#define CC_PRINTF   PRINTF//libc_printf
//#define CHANNEL_CHANGE_VIDEO_FAST_SHOW

#ifdef CI_SERVICE_SHUNNING_DEBUG_PRINT
#define CI_SHUNNING_DEBUG libc_printf
#else
#define CI_SHUNNING_DEBUG(...) do{} while(0)
#endif


/*******************************************************
* structure and enum ...
********************************************************/
#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)

typedef struct bc_cc
{
   cc_stop_transponder bc_stop_transponder;
   cc_stop_service bc_stop_service;
   cc_get_prog_record_flag bc_get_prog_record_flag;
   cc_stop_descrambling bc_stop_descrambling;
}BC_CB;

#endif

/*******************************************************
* global and local variable
********************************************************/
//ci device & service data
#ifdef MULTI_CAS
//a temp solution to make sure api_mcas_start_transponder()
//is called pair with api_mcas_stop_transponder()
BOOL mcas_xpond_stoped = FALSE;
#endif

#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)
static BC_CB bc_cc_cb={NULL,NULL,NULL,NULL};
#ifdef BC_PVR_SUPPORT
UINT8 g_pre_nim_id = 0;
#endif
#endif

BOOL pub_blk_scrn = FALSE;
ID l_cc_sema_id = OSAL_INVALID_ID;

static struct ci_device_list ci_dev_list;
static struct ci_service_info ci_service;
INT32 cc_cmd_cnt = 0;        //play channel cmd counter for CI

/*******************************************************
* local function declare
********************************************************/
#ifdef PUB_PORTING_PUB29
  typedef INT32 (*cc_notify_callback)(UINT32 param);
  cc_notify_callback post_play_callback = NULL;
  static UINT32 l_cc_crnt_state = CC_STATE_STOP;
  #if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
  static struct cc_xpond_info play_xpond_info;
  #endif
#else
  static BOOL cc_aerial_signal_monitor(struct ft_frontend *ft);
#endif

static UINT32 ciplus_case_idx = 0;

/*****************************************************
*internal functions
******************************************************/

/* BEGIN: tsg ci patch code */

#if defined(SUPPORT_BC) || defined(SUPPORT_BC_STD)
void bc_cc_init(cc_stop_transponder f_stop_tp, cc_stop_service f_stop_service, cc_get_prog_record_flag f_get_rec,
    cc_stop_descrambling f_stop_descram)
{
    bc_cc_cb.bc_stop_transponder=f_stop_tp;
    bc_cc_cb.bc_stop_service=f_stop_service;
    bc_cc_cb.bc_get_prog_record_flag=f_get_rec;
    bc_cc_cb.bc_stop_descrambling=f_stop_descram;
}

#endif

#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
static UINT32   mpg2_h264_flag = 0;         //0: no change; 1:mpg2->h264 or h264->mpg2
#endif


#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
static UINT32 cc_begin_tick = 0;
static UINT32 cc_prev_tick = 0;
#endif

static BOOL is_vdec_first_shown = FALSE;

/* Switch Audio PID *
 * For example, one program may have several language streams,
 * when switch another language stream,
 * this function will be called by upper layer. */
static void cc_switch_apid(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    UINT32 cmd_bits = cmd;

    if ((NULL == es) || (NULL == dev_list))
    {
        ASSERT(0);
        return;
    }
    if(es->a_pid != 0)
    {
        cmd_bits = CC_CMD_STOP_AUDIO|CC_CMD_DMX_CHG_APID|CC_CMD_START_AUDIO;
    }
    else
    {
        cmd_bits = CC_CMD_NULL;
    }
    cc_driver_act(cmd_bits, es, dev_list, NULL);
}

/* Pause Video, don't Pause Audio */
static void cc_pause_video(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    UINT32 cmd_bits = cmd;

	if((NULL !=es) &&(NULL !=dev_list))
	{
	    cmd_bits = CC_CMD_PAUSE_VIDEO;//CC_CMD_STOP_VIDEO;
	    cc_driver_act(cmd_bits,es, dev_list, NULL);
    }
}

/* Resume Video*/
static void cc_resume_video(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    UINT32 cmd_bits = cmd;

	if((NULL != es)&&(NULL!=dev_list))
	{
	    cmd_bits = CC_CMD_START_VIDEO;
	    cc_driver_act(cmd_bits,es, dev_list, NULL);
    }
}

/* First Video Frame Already Show */
void cb_vdec_first_show(UINT32 param1, UINT32 param2)
{
    is_vdec_first_shown = TRUE;
#ifdef CHANCHG_VIDEOTYPE_SUPPORT
    cc_backup_free();
#endif
#ifdef CHCHG_TICK_PRINTF
    extern UINT32 vdec_start_tick;
    if(vdec_start_tick)
    {
        UINT32 temp_tick = osal_get_tick();
        libc_printf("####CHCHG %s():line%d, tick=%d, tick used=%d\n", \
            __FUNCTION__,__LINE__, temp_tick, temp_tick-vdec_start_tick);
        vdec_start_tick = 0;
    }
#endif
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
    UINT32 ciplus_tmp_tick = osal_get_tick();
    cc_get_ciplus_tick(&cc_begin_tick, &cc_prev_tick);
    libc_printf("# First Picture Show (@%s)\n", __FUNCTION__);
    libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", ciplus_tmp_tick, \
        ciplus_tmp_tick-cc_begin_tick, ciplus_tmp_tick-cc_prev_tick);
    cc_prev_tick = ciplus_tmp_tick;
#endif
}

/* Check whether First Vedio Frame Already Show or not */
static BOOL ck_vdec_first_show(void)
{
    return is_vdec_first_shown;
}

/* Reset First Vedio Frame not Show */
static void reset_vdec_first_show(void)
{
    is_vdec_first_shown = FALSE;
}

//0225 ECM ISSUE, DON'T STOP SERVICE.
/* Process Before Stop Channel */
static void cc_pre_stop_channel(UINT32 cmd_bits, struct dmx_device *dmx,struct cc_es_info *es, UINT8 frontend_change)
{
#ifndef NEW_DEMO_FRAME //temp solution for pip pvr new si monitor
    UINT16 prog_pos = 0xffff;
#else
  #if defined(CONAX_NEW_PUB) || (defined(_MHEG5_V20_ENABLE_) \
        && !defined(NEW_DEMO_FRAME))
    UINT16 prog_pos = 0xffff;
  #endif
  #if(CAS_TYPE==CAS_C2300A3||CAS_TYPE == CAS_C1900A|CAS_TYPE == CAS_C2300A \
    ||CAS_TYPE==CAS_C1200A)
    UINT16 prog_pos = 0xffff;
  #endif
#endif        
    T_NODE tp;
    INT32 ret = !SUCCESS;
#ifdef MULTI_DESCRAMBLE
    UINT8 index __MAYBE_UNUSED__ =0;
#endif        

    if ((NULL == dmx) || (NULL == es))
    {
        return;
    }
    MEMSET(&tp, 0, sizeof(tp));
    //Stop CA
//    prog_pos = 0xffff;
    CC_PRINTF("CC Pre Stop Channel 1 --- Stop CA\n");
#if 0//def SUPPORT_GACAS	//Ben 180727#1
 	gacas_mcas_stop_service();
#endif
#ifdef MULTI_CAS
  #ifdef NEW_DEMO_FRAME
    #ifdef COMBO_CA_PVR
    if((0 == ((dmx->type) & HLD_DEV_ID_MASK)) && (ts_route_check_recording_by_nim_id(0) != RET_SUCCESS))
    #endif
    {
    		#ifdef _C0700A_VSC_ENABLE_
        if(1)
        #else
        //0225 ECM ISSUE, DON'T STOP SERVICE.
        if(cmd_bits & CC_CMD_STOP_MCAS_SERVICE )
        #endif	
        {
      #ifdef MULTI_DESCRAMBLE
      #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
        if(FALSE == api_mcas_get_prog_record_flag(es->prog_id))
      #endif
        {
            api_mcas_stop_service_multi_des(es->prog_id,es->sim_id);
        }
        #elif defined(SUPPORT_C0200A)
        INT8 session_id = 0xff;
        session_id = nvcak_search_session_by_prog(es->prog_id);
        if (FALSE == nvcak_get_prog_record_flag(es->prog_id))
        {
            //libc_printf("%s nvcak stop play channel, session_id = %d.\n", __FUNCTION__, session_id);
            nvcak_stop_dsc_program(session_id);
        }
        #else
        #if (CAS_TYPE == CAS_C1700A)
            api_mcas_stop_service();
        #else
            api_mcas_stop_service(es->sim_id);
        #endif
      #endif
        }
        else
        {
            //FSC will stop preplay without CC_CMD_STOP_MCAS_SERVICE, but main is playing.
        #ifdef FSC_SUPPORT
            api_mcas_enable_ecm_cmd(1);
        #else  
            //normal.
            #ifdef CAS9_V6//disable ecm send in menu
                api_mcas_enable_ecm_cmd(0);
            #endif    
        #endif
        }
    }
  #else
    api_mcas_stop_service();
  #endif
    if(frontend_change)
    {
    #if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
        api_mcas_stop_transponder(es->sim_id);
    #else
      #ifdef MULTI_DESCRAMBLE
        index = (dmx->type)&HLD_DEV_ID_MASK;
        //libc_printf("%s dmx_index=%d\n",__FUNCTION__,index);
        if(FALSE == api_mcas_get_prog_record_flag(es->prog_id))
        {
            api_mcas_stop_transponder_multi_des(index);//vicky201410_miss_sync E-tree
        }
        #elif defined(SUPPORT_C0200A)
        UINT8 dmx_id = (dmx->type)&HLD_DEV_ID_MASK - 1;
        //libc_printf("%s nvcak stop transponder, dmx_id = %d.\n", __FUNCTION__, dmx_id);
        if (nvcak_check_dmx_has_rec_channel(dmx_id) == -1)
        {
            nvcak_stop_transponder(dmx_id);
        }
        #else
            api_mcas_stop_transponder();
        #ifdef SUPPORT_CAS_A
            sie_stop_emm_service();
        #endif
      #endif
    #endif
        mcas_xpond_stoped = TRUE;
    }
#endif

#if defined(SUPPORT_BC_STD) //CAS_CM_ENABLE
    if((NULL!=bc_cc_cb.bc_stop_service) && (NULL!=bc_cc_cb.bc_stop_transponder))
    {
    //libc_printf("cc_pre_stop_channel: api_mcas_stop_service\n");
        bc_cc_cb.bc_stop_service();
    if(frontend_change)
    {
    //libc_printf("cc_pre_stop_channel: api_mcas_stop_transponder\n");
            bc_cc_cb.bc_stop_transponder();
        }
    }
#elif defined(SUPPORT_BC)
  #ifdef BC_PVR_SUPPORT
    if( (NULL!=bc_cc_cb.bc_get_prog_record_flag) && (FALSE == bc_cc_cb.bc_get_prog_record_flag(es->service_id)) )
  #endif
    {
        if((NULL!=bc_cc_cb.bc_stop_descrambling) && (NULL!=bc_cc_cb.bc_stop_service))
        {
            bc_cc_cb.bc_stop_descrambling(es->service_id);
            bc_cc_cb.bc_stop_service(es->service_id);
        if(frontend_change)
        {
            #ifdef BC_PVR_SUPPORT
            UINT8 dmx_id = g_pre_nim_id;
            #else
            UINT8 dmx_id = ((dmx->type)&HLD_DEV_ID_MASK) - 1;
            #endif
                if(NULL!=bc_cc_cb.bc_stop_transponder)
                {
            //libc_printf("cc_pre_stop_channel: api_mcas_stop_transponder\n");
                    bc_cc_cb.bc_stop_transponder(dmx_id);
                }
            }
        }
    }
#endif

    //SI Monitor Off
    CC_PRINTF("CC Pre Stop Channel 3 --- SI Monitor Off\n");
#ifndef NEW_DEMO_FRAME //temp solution for pip pvr new si monitor
    prog_pos = get_prog_pos(es->prog_id);
    si_monitor_off(prog_pos);
#else
    if (es->sim_onoff_enable)
    {
        cc_sim_stop_monitor(es);
    }
  #if defined(CONAX_NEW_PUB) || (defined(_MHEG5_V20_ENABLE_) \
        && !defined(NEW_DEMO_FRAME))
    prog_pos = get_prog_pos(es->prog_id);
    si_monitor_off(prog_pos);
  #endif
//PVR defined NEW_DEMO_FRAME, call cc_sim_stop_monitor(),
//but CAS need si_monitor_off()
  #if(CAS_TYPE==CAS_C2300A3||CAS_TYPE == CAS_C1900A|CAS_TYPE == CAS_C2300A \
    ||CAS_TYPE==CAS_C1200A)
    prog_pos = get_prog_pos(es->prog_id);
    si_monitor_off(prog_pos);
  #endif

#endif

    //NVOD
    CC_PRINTF("CC Pre Stop Channel 4 --- NVOD Save\n");
    if((SERVICE_TYPE_NVOD_TS == es->service_type) && (frontend_change))
    {
        ret = get_tp_by_id(es->tp_id, &tp);
        if (DB_SUCCES != ret)
        {
            CC_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
        }
        nvod_saveorload_tp_eit(0, &tp);
    }

    //EPG Off
    //recommend: after si_monitor, for it maybe spend much time
    CC_PRINTF("CC Pre Stop Channel 2 --- EPG Off\n");
//reduce the space for ota
#ifndef _BUILD_OTA_E_
    #ifdef FSC_SUPPORT
    if(es->epg_enable)
    {
        epg_off();        
    }
    #else  
    epg_off();
    #endif
#endif

    CC_PRINTF("CC Pre Stop Channel 5 --- Leave Pre Stop Channel\n");
}

#if (defined(PUB_PORTING_PUB29)) && ((SYS_PROJECT_FE == PROJECT_FE_ATSC)||( SYS_PROJECT_FE == PROJECT_FE_DVBT ))
static void cc_lock_program(UINT32 cur_channel)
{
    P_NODE p_node;
    struct ft_frontend ft;
    struct cc_param param;
    BOOL ci_start_service = FALSE;
    UINT32 cmd_bits = 0;
    struct nim_config config;

    monitor_rst();//clear dm counter

    MEMSET(&p_node, 0, sizeof(p_node));
    MEMSET(&ft, 0, sizeof(struct ft_frontend));
    if(get_prog_by_id(cur_channel, &p_node) != SUCCESS)
    {
        return FALSE;
    }

    l_cc_crnt_state = (p_node.av_flag)? CC_STATE_PLAY_TV:CC_STATE_PLAY_RADIO;
#ifdef HDTV_SUPPORT
    cmd_bits = CC_CMD_CLOSE_VPO|CC_CMD_STOP_AUDIO;
#else
    cmd_bits = CC_CMD_CLOSE_VPO|CC_CMD_STOP_AUDIO|CC_CMD_FULL_FB;
#endif

    if(!prt_pre_play_channel(&p_node, &ft, &param, &ci_start_service, FALSE))
    {
        return FALSE;
    }

    if(ft.nim != NULL)
    {
        dev_get_nim_config(ft.nim, ft.xpond.common.type, &config);
        if(MEMCMP(&ft.xpond, &config.xpond, sizeof(union ft_xpond)))
        {
            cmd_bits |= CC_CMD_SET_XPOND;
        }
    }

#if(SYS_PROJECT_FE == PROJECT_FE_ATSC)
//libc_printf("cur_channel[%x]\n",cur_channel);
    UINT16 p_idx= get_prog_pos(cur_channel);
//libc_printf("p_idx[%x]\n",p_idx);

    if (pre_tp_id != l_cc_cmd.pids.tp_id)
    {
        psip_monitor_stop();
    }
    else
    {
        psip_pmt_monitor_off(p_idx);
    }
#endif

#if(SYS_PROJECT_FE == PROJECT_FE_ATSC)
    if(FALSE == get_vct_changing_flag())
#endif
    {
        cc_driver_act(cmd_bits, &(param.es), &(param.dev_list), &ft);
    }
#if(SYS_PROJECT_FE == PROJECT_FE_ATSC)
    pre_tp_id = l_cc_cmd.pids.tp_id;
    psip_monitor_start((UINT32)pre_tp_id, p_idx);
#endif

}
#endif

#ifndef PUB_PORTING_PUB29
static BOOL cc_aerial_signal_monitor(struct ft_frontend *ft)
{
    T_NODE t_node;
#if (defined(SUPPORT_FREQ_CONVERT) || defined(EZ_ADD_SUPPORT)) 
    UINT32 center_freq=0;
#endif
    INT32 ret = 1;

    if(NULL == ft)
    {
        return FALSE;
    }

    MEMSET(&t_node, 0, sizeof(T_NODE));
//    center_freq=0;
   //  center_freq = sfreq + ((bandwidth*1000)>>1);
    ret = nim_io_control(ft->nim, NIM_DRIVER_STOP_ATUOSCAN, 0);
    if (SUCCESS != ret)
    {
        CC_PRINTF("%s(): failed at line:%d\n", __FUNCTION__, __LINE__);
    }
    //api_nim_reset_autoscan();     //50929 yuchun
#ifdef SUPPORT_FREQ_CONVERT
    as_freq_convert(&center_freq);
#endif
#ifdef EZ_ADD_SUPPORT
    psip_monitor_stop();
#endif
    frontend_tuning(ft->nim, NULL, &ft->xpond, 1);
/*
    api_nim_channel_change(
        center_freq,
        bandwidth,
        0,
        0,
        0,
        0,
        0x03, //AERIALTUNE,
        1,
        0);
*/
#ifdef EZ_ADD_SUPPORT
    // manual scan cannot enter due to will auto scan in
    if (center_freq && (NULL == menu_stack_get_top()))
    {
        //soc_printf("psip_start\n");

        if ((SUCCESS == get_tp_by_id(pre_tp_id, &t_node))
            && (center_freq == t_node.frq))
        {
            if (TRUE == get_vct_changing_flag())
            {
                set_vct_changing_flag(FALSE);
            }
            return TRUE;
        }
        // 080225 Xavier avoid tp_id check error when bigger than 57
        psip_monitor_start(center_freq, 0xffff);
    }
#endif

    return TRUE;
}

/* Show Background Logo */
static void cc_show_logo(UINT32 cmd, struct cc_logo_info *plogo, struct cc_device_list *dev_list)
{
    INT32 ret = -1;
    INT32 ret_code = RET_FAILURE;
    UINT8 i = 0;
    UINT8 j = 0;
#ifdef DUAL_ENABLE
    UINT8 *logo_buf = NULL;
#else
    void *write_buff = NULL;
#endif
    UINT32 cmd_bits = 0;

    UINT32 size_got = 0;
    struct vdec_io_reg_callback_para tpara;
    struct cc_es_info es;// = {0};
    struct vdec_status_info decv_status;// = {0};
    const UINT8 delay_cnt = 100;
    UINT32 size_threshold = 32;
    const UINT32 vbv_request_cnt = 2;

    if ((NULL == dev_list) || (UINT_MAX == cmd))
    {
        return;
    }
    MEMSET(&tpara, 0, sizeof(struct vdec_io_reg_callback_para));
    MEMSET(&es, 0, sizeof(struct cc_es_info));
    MEMSET(&decv_status, 0, sizeof(struct vdec_status_info));
    if(NULL == dev_list->dmx)
    {
        ret_code = vdec_io_control(dev_list->vdec, VDEC_IO_GET_STATUS, (UINT32)(&decv_status));
        if((RET_SUCCESS != ret_code) || (VDEC27_STARTED == decv_status.u_cur_status))
        {
            return;
        }
    }

    CC_PRINTF("CC Show Logo 1 --- Driver Act\n");
    if(sys_ic_get_chip_id() != ALI_S3281)
    {
        vdec_start(dev_list->vdec);//fix pause issue
    }

    cmd_bits = cmd | CC_CMD_STOP_VIDEO | CC_CMD_STOP_DMX |CC_CMD_STOP_AUDIO;
    cc_driver_act(cmd_bits, &es, dev_list, NULL);

    CC_PRINTF("CC Show Logo 2 --- Show Logo\n");
    if((plogo != NULL) && (plogo->addr != NULL) && (plogo->size > 0) && (plogo->sto != NULL))
    {
    #ifdef DUAL_ENABLE
        logo_buf = (UINT8 *)MALLOC(plogo->size);
        if (NULL == logo_buf)
        {
            return;
        }
    #endif
        reset_vdec_first_show();
        tpara.e_cbtype = VDEC_CB_FIRST_SHOWED;
        tpara.p_cb = cb_vdec_first_show;
        vdec_io_control(dev_list->vdec, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        vdec_io_control(dev_list->vdec, VDEC_SET_DMA_CHANNEL, DMA_INVALID_CHA);
        vdec_start(dev_list->vdec);
    #ifdef ADJUST_SPECIAL_LOGO_QUALITY
        struct vpo_device *vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        vpo_ioctl(vpo_dev, VPO_IO_CHANGE_YC_INIT_PHASE, 1);
    #endif
        for(i = 0;i < vbv_request_cnt; i++)
        {
#ifndef HDTV_SUPPORT
    #ifdef DUAL_ENABLE
            if(((UINT32)plogo->addr&0x80000000)||((UINT32)plogo->addr&0xA0000000))
            {
                osal_cache_flush(plogo->addr,plogo->size);
                vdec_copy_data((UINT32)dev_list->vdec, (UINT32)plogo->addr,plogo->size,&size_got);
            }
            else
            {
                #ifdef _NV_PROJECT_SUPPORT_
                MEMCPY((UINT8 *)logo_buf, plogo->addr, plogo->size);
                #else
                sto_get_data(plogo->sto, (UINT8 *)logo_buf, plogo->addr, plogo->size);
                #endif
                osal_cache_flush(logo_buf,plogo->size);
                vdec_copy_data((UINT32)dev_list->vdec, (UINT32)logo_buf,plogo->size,&size_got);
            }
    #else
            vdec_vbv_request((void *)dev_list->vdec, plogo->size, \
                             &write_buff, &size_got, (struct control_block *)NULL);
            #ifdef _NV_PROJECT_SUPPORT_
            MEMCPY((UINT8 *)write_buff, plogo->addr, size_got);
            #else
            sto_get_data(plogo->sto, (UINT8 *)write_buff, plogo->addr, size_got);
            #endif
            vdec_vbv_update((void *)dev_list->vdec, size_got);
    #endif
#else
    #ifdef DUAL_ENABLE
            if (0 == i)
            {
                #ifdef _NV_PROJECT_SUPPORT_
                MEMCPY((UINT8 *)logo_buf, plogo->addr, plogo->size);
                #else
                ret = sto_get_data(plogo->sto, (UINT8 *)logo_buf, (UINT32)(plogo->addr), plogo->size);
                if(ret > 0)
                #endif
                {
                    osal_cache_flush(logo_buf,plogo->size);
                    vdec_copy_data((UINT32)dev_list->vdec,(UINT32)logo_buf,plogo->size,&size_got);
                }
            }
            else if(size_got >= size_threshold)
            {
                MEMSET(logo_buf, 0, size_threshold);
                osal_cache_flush(logo_buf,size_threshold);
                vdec_copy_data((UINT32)dev_list->vdec,(UINT32)logo_buf,size_threshold,&size_got);
            }

    #else
            vdec_vbv_request((void *)dev_list->vdec, plogo->size, \
                             &write_buff, &size_got, (struct control_block *)NULL);
            if (0 == i)
            {
                #ifdef _NV_PROJECT_SUPPORT_
                MEMCPY((UINT8 *)write_buff, plogo->addr, size_got);
                #else
                sto_get_data(plogo->sto, (UINT8 *)write_buff, (UINT32)plogo->addr, size_got);
                #endif
            }
            else if (size_got >= size_threshold)
            {
                MEMSET(write_buff, 0, size_threshold);
                size_got = size_threshold;
            }
            vdec_vbv_update((void *)dev_list->vdec, size_got);
    #endif
#endif
        }
        osal_task_sleep(10);
        j = 0;
        while((!ck_vdec_first_show()) && (j<delay_cnt))
        {
            osal_task_sleep(10);
            j++;
        }
#ifdef DUAL_ENABLE
        FREE(logo_buf);
        logo_buf = NULL;
#endif
        vdec_stop(dev_list->vdec, FALSE, FALSE);
#ifdef ADJUST_SPECIAL_LOGO_QUALITY
        vpo_ioctl(vpo_dev, VPO_IO_CHANGE_YC_INIT_PHASE, 0);
#endif

    }

#ifdef PUB_PORTING_PUB29
    l_cc_crnt_state = CC_STATE_STOP;
#endif
    CC_PRINTF("CC Show Logo 3 --- Leave Show Logo\n");
}


/* Implement CC Command */
static void cc_cmd_implement(struct libpub_cmd *cmd)
{
    UINT32 cmd_finish_bit = 0;
    struct ft_frontend *ft = NULL;
    struct cc_es_info *es = NULL;
    struct cc_device_list *dev_list = NULL;
    struct cc_logo_info *plogo = NULL;
    struct cc_param *param = NULL;
    BOOL   bret = FALSE;
#ifdef FSC_SUPPORT    
    UINT32 cc_cmd_bits = 0;
#endif
    
    if (NULL == cmd)
    {
        return;
    }
    switch(cmd->cmd_type)
    {
    case LIBPUB_CMD_CC_PLAY:
        ft = (struct ft_frontend*)cmd->data1;
        param = (struct cc_param*)cmd->data2;
        //Show Black Screen
        if(1 == cmd->para1)
        {
            cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        }

        CC_PRINTF("%s(): call cc_play_channel(),tick=%d\n", __FUNCTION__, osal_get_tick());
        cc_play_channel(cmd->cmd_bit, ft, param);
        cmd_finish_bit = LIBPUB_FLG_CC_PLAY_BIT;
        break;
    case LIBPUB_CMD_CC_STOP:
        es = (struct cc_es_info *)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        //Show Black Screen
        if(1 == cmd->para1)
        {
            cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        }

        cc_stop_channel(cmd->cmd_bit, es, dev_list);
        cmd_finish_bit = LIBPUB_FLG_CC_STOP_BIT;
        break;
    case LIBPUB_CMD_CC_PAUSE_VIDEO:
        es = (struct cc_es_info *)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        cc_pause_video(cmd->cmd_bit, es, dev_list);
        cmd_finish_bit = LIBPUB_FLG_CC_PAUSE_BIT;
        break;
    case LIBPUB_CMD_CC_RESUME_VIDEO:
        es = (struct cc_es_info *)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        cc_resume_video(cmd->cmd_bit,es, dev_list);
        cmd_finish_bit = LIBPUB_FLG_CC_PAUSE_BIT;
        break;
    case LIBPUB_CMD_CC_SWITCH_APID:
        es = (struct cc_es_info *)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        cc_switch_apid(cmd->cmd_bit, es, dev_list);
        cmd_finish_bit = LIBPUB_FLG_CC_SWITCH_APID_BIT;
        break;
    case LIBPUB_CMD_CC_SWITCH_ACH:
        es = (struct cc_es_info *)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        cmd->cmd_bit = CC_CMD_AUDIO_SWITCH_CHAN;
        cc_driver_act(cmd->cmd_bit, es, dev_list, NULL);
        cmd_finish_bit = LIBPUB_FLG_CC_SWITCH_ACH_BIT;
        break;
    case LIBPUB_CMD_CC_SHOW_LOGO:
        plogo = (struct cc_logo_info*)cmd->data1;
        dev_list = (struct cc_device_list *)cmd->data2;
        cc_show_logo(cmd->cmd_bit, plogo, dev_list);
        cmd_finish_bit = LIBPUB_FLG_CC_LOGO_BIT;
        break;
#ifdef CC_USE_TSG_PLAYER
    case LIBPUB_CMD_CC_START_PATCH:
        cc_start_patch(cmd->para1);
        cmd_finish_bit = LIBPUB_FLG_CC_PATCH_BIT;
        break;
#endif
    case LIBPUB_CMD_CC_AERIAL_CHG_CH:
    #ifndef PUB_PORTING_PUB29
        bret = cc_aerial_signal_monitor((struct ft_frontend*)cmd->para1);
        if(FALSE == bret)
        {
            ali_trace(&bret);
        }
    #endif
        break;
#if (defined(PUB_PORTING_PUB29)) && ((SYS_PROJECT_FE == PROJECT_FE_ATSC)||( SYS_PROJECT_FE == PROJECT_FE_DVBT ))
    case LIBPUB_CMD_CC_LOCK:
        cc_lock_program(cmd->para1);
        cmd_finish_bit = MID27_FLG_CC_LOCK_BIT;
        break;
#endif

#ifdef FSC_SUPPORT
    case LIBPUB_CMD_CC_FSC_PRE_PLAY:
    	if(cmd->para1 == 1)
    	{
        		cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    	}
        MEMCPY(&ft,cmd->data1,sizeof(ft));
        MEMCPY(&param,cmd->data2,sizeof(param));         
        FSC_CC_PRINTF("I come here %d \n",__LINE__);
    	FSC_CC_PRINTF("%s: call FSC_PRE_PLAY \n", __FUNCTION__); 
        //FSC pre play don't need to enabel ttx/subt.
        param->es.not_oprate_subt_ttx = 1;
        param->es.epg_enable = 0;
    	cc_play_channel(cmd->cmd_bit, ft, param);
        param->es.not_oprate_subt_ttx = 0;
        cmd_finish_bit = LIBPUB_FLG_CC_PLAY_BIT;
        break;

    case LIBPUB_CMD_CC_FSC_MAIN_PLAY:
    	//Show Black Screen
    	if(cmd->para1 == 1)
    	{
        		cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    	}
        MEMCPY(&ft,cmd->data1,sizeof(ft));
        MEMCPY(&param,cmd->data2,sizeof(param));         
        FSC_CC_PRINTF("I come here %d \n",__LINE__);
    	FSC_CC_PRINTF("%s: call FSC_MAIN_PLAY \n", __FUNCTION__); 
        set_fsc_vdec_first_show(param->dev_list.vdec);
        param->es.epg_enable = 1;
    	cc_play_channel(cmd->cmd_bit, ft, param);
        cmd_finish_bit = LIBPUB_FLG_CC_PLAY_BIT;
        break;

    case LIBPUB_CMD_CC_FSC_STOP:
        MEMCPY(&ft,cmd->data1,sizeof(ft));
        MEMCPY(&param,cmd->data2,sizeof(param));
    	//Show Black Screen
    	if(cmd->para1 == 1)
    	{
        		cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    	}
        cmd->cmd_bit |= CC_CMD_FSC_STOP; 
        FSC_CC_PRINTF("I come here %d \n",__LINE__);
    	FSC_CC_PRINTF("%s: call FSC_STOP_PLAY \n", __FUNCTION__);               
        if(param != NULL)
        {
    	    cc_stop_channel(cmd->cmd_bit,&(param->es), &(param->dev_list));            
        }
    	cmd_finish_bit = LIBPUB_FLG_CC_STOP_BIT;
        break;
        
    case LIBPUB_CMD_CC_FSC_CHANNEL_CHANGE:
        FSC_CC_PRINTF("I come here %d \n",__LINE__);
    	FSC_CC_PRINTF("%s: call FSC_CHANNEL_CHANGE \n", __FUNCTION__);
        MEMCPY(&ft,cmd->data1,sizeof(ft));
        MEMCPY(&param,cmd->data2,sizeof(param));

        //Show Black Screen
    	if(cmd->para1 == 1)
    	{
        		cc_cmd_bits |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    	}
        fsc_channel_chanege_act(ft,param,cmd->para2,cc_cmd_bits);
        cmd_finish_bit = LIBPUB_FLG_CC_PLAY_BIT;
        break;
        
#endif
    default:
        break;
    }

    if(1 == cmd->sync)
    {
        libpub_declare_cmd_finish(cmd_finish_bit);
    }
}

#ifdef PVR_DYNAMIC_PID_CHANGE_TEST
void cc_set_h264_chg_flag(UINT32 value)
{
    mpg2_h264_flag = value;

    // libc_printf("%s : mpg2_h264_flag(%d)\n",__FUNCTION__,mpg2_h264_flag);
}

UINT32 cc_get_h264_chg_flag(void)
{
    // libc_printf("%s : mpg2_h264_flag(%d)\n",__FUNCTION__,mpg2_h264_flag);
    return mpg2_h264_flag;
}
#endif

void cc_set_case_idx(UINT32 case_idx)
{
    ciplus_case_idx = case_idx;
}
UINT32 cc_get_case_idx(void)
{
    return ciplus_case_idx;
}
/* Process Before Play Channel */
void cc_pre_play_channel(struct dmx_device *dmx, struct cc_es_info *es, struct ft_ts_route *ts_route,
    UINT8 frontend_change)
{    
    T_NODE tp;
    struct ci_service_info service;
#ifndef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
    UINT16 prog_pos = 0xffff;    
#else
  #ifndef CI_PLUS_PVR_SUPPORT
    #if defined(CONAX_NEW_PUB) || (defined(_MHEG5_V20_ENABLE_) && !defined(NEW_DEMO_FRAME))
        UINT16 prog_pos = 0xffff;
    #endif
    //PVR defined NEW_DEMO_FRAME, call cc_sim_stop_monitor(),
    //but CAS need si_monitor_off()
    #if(CAS_TYPE==CAS_C2300A3||CAS_TYPE == CAS_C1900A||CAS_TYPE == CAS_C2300A||CAS_TYPE==CAS_C1200A)
        UINT16 prog_pos = 0xffff;
    #endif
  #endif //CI_PLUS_PVR_SUPPORT
#endif //NEW_DEMO_FRAME

    if ((NULL == dmx) || (NULL == es) || (NULL == ts_route))
    {
        //ASSERT(0);
        return;
    }

//    prog_pos = 0xffff;
#if (defined(_MHEG5_V20_ENABLE_) )
    mheg5_receiver_stop_applications();
#endif
    //Stop CA
    CC_PRINTF("CC Pre Play Channel 1 --- Stop CA\n");

    MEMSET(&tp, 0, sizeof(T_NODE));
    MEMSET(&service, 0, sizeof(struct ci_service_info));
    //SI Monitor Off
    CC_PRINTF("CC Pre Play Channel 3 --- SI Monitor Off\n");
#ifndef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
    prog_pos = get_prog_pos(es->prog_id);
        si_monitor_off(prog_pos);
  #if(defined(_SERVICE_NAME_UPDATE_ENABLE_)\
    ||defined(_MULTILINGAL_SERVICE_NAME_ENABLE_)\
    ||defined(_PREFERRED_NAME_ENABLE_)\
    ||defined(_SHORT_SERVICE_NAME_ENABLE_))
    #if (SYS_PROJECT_FE != PROJECT_FE_ATSC)
      sdt_monitor_off();
    #endif // (SYS_PROJECT_FE != PROJECT_FE_ATSC)
  #endif

  #ifdef SUPPORT_FRANCE_HD
    nit_monitor_off();
  #endif // SUPPORT_FRANCE_HD
#else
  #ifdef AUTO_OTA
    ota_monitor_off();
  #endif
    if (es->sim_onoff_enable)
    {
        cc_sim_stop_monitor(es);
    }
  #ifndef CI_PLUS_PVR_SUPPORT
    #if defined(CONAX_NEW_PUB) || (defined(_MHEG5_V20_ENABLE_) && !defined(NEW_DEMO_FRAME))
    prog_pos = get_prog_pos(es->prog_id);
        si_monitor_off(prog_pos);
    #endif
//PVR defined NEW_DEMO_FRAME, call cc_sim_stop_monitor(),
//but CAS need si_monitor_off()
    #if(CAS_TYPE==CAS_C2300A3||CAS_TYPE == CAS_C1900A||CAS_TYPE == CAS_C2300A||CAS_TYPE==CAS_C1200A)
        prog_pos = get_prog_pos(es->prog_id);
        si_monitor_off(prog_pos);
    #endif
  #endif //CI_PLUS_PVR_SUPPORT
#endif //NEW_DEMO_FRAME

#ifdef MULTI_CAS
  #ifdef NEW_DEMO_FRAME
    #ifdef COMBO_CA_PVR
    if((0 == ((dmx->type) & HLD_DEV_ID_MASK)) && (ts_route_check_recording_by_nim_id(0) != RET_SUCCESS))
    #endif
    {
        #if (!defined(MULTI_DESCRAMBLE)) && (!defined(SUPPORT_C0200A))
        #if (CAS_TYPE == CAS_C1700A)
        #else
        api_mcas_stop_service(es->sim_id);
        #endif
        #endif
    }
  #else  //NEW_DEMO_FRAME
    api_mcas_stop_service();
  #endif //NEW_DEMO_FRAME
    if(frontend_change)
    {
  #if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
        api_mcas_stop_transponder(es->sim_id);
  #else
    #ifdef MULTI_DESCRAMBLE
        //if the program is recording,don't stop descramble service
        if(FALSE == api_mcas_get_prog_record_flag(es->prog_id))
        {
            api_mcas_stop_transponder_multi_des(ts_route->dmx_id-1);
        }
        #elif defined(SUPPORT_C0200A)
        //libc_printf("%s stop transponder, dmx = %d.\n", __FUNCTION__, ts_route->dmx_id-1);
        if (nvcak_check_dmx_has_rec_channel(ts_route->dmx_id - 1) == -1)
        {
            nvcak_stop_transponder(ts_route->dmx_id - 1);
        }
    #else
      #if (CAS_TYPE == CAS_C1700A)
        #else
        api_mcas_stop_transponder();
      #endif
    #endif //MULTI_DESCRAMBLE
        mcas_xpond_stoped = TRUE;
    }
  #endif//(defined(GET_CAT_BY_SIM))
#endif//MULTI_CAS

#if defined(SUPPORT_BC_STD) //CAS_CM_ENABLE
    if((NULL!=bc_cc_cb.bc_stop_service) && (NULL!=bc_cc_cb.bc_stop_transponder))
    {
        //libc_printf("cc_pre_stop_channel: api_mcas_stop_service\n");
        bc_cc_cb.bc_stop_service();
    if(frontend_change)
    {
        //libc_printf("cc_pre_stop_channel: api_mcas_stop_transponder\n");
            bc_cc_cb.bc_stop_transponder();
        }
    }
#elif defined(SUPPORT_BC)
    if(frontend_change)
    {
        UINT8 dmx_id = 0;

        #ifdef BC_PVR_SUPPORT
        libc_printf("%s: stop transponder(nim_id = %d)\n", __FUNCTION__, g_pre_nim_id);
        dmx_id = g_pre_nim_id;
        #endif
        if(NULL!=bc_cc_cb.bc_stop_transponder)
        {
            bc_cc_cb.bc_stop_transponder(dmx_id);
        }
    }
#endif

    //NVOD
    CC_PRINTF("CC Pre Play Channel 4 --- NVOD Save\n");
    if((SERVICE_TYPE_NVOD_TS==es->service_type)&&(frontend_change))
    {
        get_tp_by_id(es->tp_id, &tp);
        nvod_saveorload_tp_eit(0, &tp);
    }

    //CI, avoid not descramble issue when cc frequently
    CC_PRINTF("CC Pre Play Channel 5 --- Stop CI\n");

#ifdef CI_PLUS_SUPPORT
    ci_stop_sdt_filter();
#endif

#if ((!defined CI_ALLPASS_ENABLE))
    if (ts_route->ci_enable)
#endif
    {
        service.monitor_id = es->sim_id;
#ifdef CI_SUPPORT
    #ifdef  CI_ALLPASS_ENABLE
        api_ci_stop_service(NULL, &service, 0x03);
    #else
        api_ci_stop_service(NULL, &service, ts_route->ci_slot_mask);
    #endif
#endif
    }
#ifdef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
    if(!es->not_oprate_subt_ttx)
    {
    #if (SUBTITLE_ON == 1)
        subt_unregister(es->sim_id);
    #endif
    #if (TTX_ON == 1)
        ttx_unregister(es->sim_id);
    #endif
    #if (ISDBT_CC == 1)
        isdbtcc_unregister(es->sim_id);
    #endif
    }
#endif

    //EPG Off
    //recommend: after si_monitor, for it maybe spend much time
    CC_PRINTF("CC Pre Play Channel 2 --- EPG Off\n");
//reduce the space for ota
#ifndef _BUILD_OTA_E_
    #ifdef FSC_SUPPORT
    if(es->epg_enable)
    {
        epg_off();        
    }
    #else  
    epg_off();
    #endif
#endif

#ifdef CI_SUPPORT
    api_ci_mutex_lock();
#endif
    CC_PRINTF("CC Pre Play Channel 6 --- Leave Pre Play Channel\n");
}

/* Process After Play Channel */
void cc_post_play_channel(struct nim_device *nim, struct dmx_device *dmx,
    struct ft_ts_route *ts_route, struct cc_es_info *es, UINT8 frontend_change)
{
    T_NODE tp;
    P_NODE pnode;    
    INT32 ret = RET_FAILURE;
#ifdef CI_PLUS_SUPPORT
    enum ci_protect_mode prot_mode = CI_PROTECTED_IN_ACTIVE;
#endif 

    if ((NULL==nim)||(NULL == dmx) || (NULL == es) || (NULL == ts_route))
    {
        ASSERT(0);
        return;
    }

    MEMSET(&tp, 0, sizeof(T_NODE));
    MEMSET(&pnode, 0, sizeof(P_NODE));
//    prot_mode = CI_PROTECTED_IN_ACTIVE;
#ifdef PUB_PORTING_PUB29
    if(NULL != post_play_callback && es)
    {
        post_play_callback(es->prog_id);
    }
#endif

#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
    UINT32 ciplus_tmp_tick = osal_get_tick();
    cc_get_ciplus_tick(&cc_begin_tick, &cc_prev_tick);
    libc_printf("# Current @%s\n",__FUNCTION__);
    libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", ciplus_tmp_tick, \
            ciplus_tmp_tick-cc_begin_tick, \
            ciplus_tmp_tick-cc_prev_tick);
    cc_prev_tick = ciplus_tmp_tick;
#endif
    //Start CA
    CC_PRINTF("CC Post Play Channel 1 --- Start CA\n");

    //EPG On
    CC_PRINTF("CC Post Play Channel 2 --- EPG On\n");
//reduce the space for ota
#ifndef _BUILD_OTA_E_
    if (STATUS_ON == epg_get_status())
    {
        #ifdef FSC_SUPPORT
        if(es->epg_enable)
        {
            epg_off();
        }
        #else       
        // off epg before epg_on, to ensure open epg on correct service.
        epg_off();
        #endif
    }
  #ifdef _INVW_JUICE
    epg_on_by_inview(dmx, es->sat_id, es->tp_id, es->service_id, 0);
  #else
    #ifdef FSC_SUPPORT
    if(es->epg_enable)
    {
        epg_on_ext(dmx, es->sat_id, es->tp_id, es->service_id); 
    }
    #else
    epg_on_ext(dmx, es->sat_id, es->tp_id, es->service_id);
    #endif
  #endif // _INVW_JUICE

  #ifdef _C0200A_CA_ENABLE_
    if (STATUS_ON == nvcak_eit_get_status())
    {
        // off eit before eit_on, to ensure open eit on correct service.
        nvcak_eit_off();
    }
    nvcak_eit_on(dmx, es->sat_id, es->tp_id, es->service_id);

    if (STATUS_ON == nvcak_sdt_get_status())
    {
        // off eit before eit_on, to ensure open eit on correct service.
        nvcak_sdt_off();
    }
    nvcak_sdt_on(dmx, es->sat_id, es->tp_id, es->service_id);
  #endif

#endif // _BUILD_OTA_E_
    
    //SI Monitor On
    CC_PRINTF("CC Post Play Channel 3 --- SI Monitor On\n");

    //NVOD
    CC_PRINTF("CC Post Play Channel 4 --- NVOD Load\n");
    if((SERVICE_TYPE_NVOD_TS==es->service_type)&&(frontend_change))
    {
        ret = get_tp_by_id(es->tp_id, &tp);
        if (RET_SUCCESS != ret)
        {
            return;
        }
        nvod_saveorload_tp_eit(1, &tp);
    }

    //CI
    CC_PRINTF("CC Post Play Channel 5 --- Start CI\n");

    ret = get_prog_by_id(es->prog_id, &pnode);
    if (RET_SUCCESS != ret)
    {
        return;
    }
    //free prog ts bypass ci slot
    if(!pnode.ca_mode)
    {
    #if ((defined(CI_SUPPORT)) && (!defined(CI_PLUS_SUPPORT))&&(!defined CI_ALLPASS_ENABLE))
        api_set_nim_ts_type(2, ts_route->ci_slot_mask);
    #endif
    }
    //CI, avoid not descramble issue when cc frequently
#ifdef CI_SUPPORT
    api_ci_mutex_unlock();
#endif

#if ((defined CI_ALLPASS_ENABLE))
    if (FLAG_TRUE)
#else
  #ifdef CI_PLUS_SUPPORT
    if (ts_route->ci_enable) //We need pass CAM for ALL time when CI PLUS enable
                             //So we just bypass the CAM when do auto scan
  #else   //CI
    if (ts_route->ci_enable && (pnode.ca_mode))
  #endif
#endif
    {
        CC_PRINTF("\nCI Start\n");

        ci_dev_list.nim_dev = nim;
        ci_dev_list.dmx_dev = dmx;
        ci_service.prog_id = es->prog_id;
        ci_service.video_pid = es->v_pid;
        ci_service.audio_pid = es->a_pid;
        ci_service.ttx_pid = es->ttx_pid;
        ci_service.subt_pid = es->subt_pid;
        ci_service.monitor_id = es->sim_id;

#if (!defined(CC_USE_TSG_PLAYER) && defined(CI_SUPPORT))
    #if ((defined CI_ALLPASS_ENABLE))
        api_ci_start_service(&ci_dev_list, &ci_service, 0x03);
    #else

      #ifdef CI_PLUS_SUPPORT
        CI_SHUNNING_DEBUG("# check pnode @%s\n", __FUNCTION__);
        CI_SHUNNING_DEBUG("#    pnode.prog_number = %d\n", pnode.prog_number);
        CI_SHUNNING_DEBUG("#    pnode.shunning_protected = %d\n", \
            pnode.shunning_protected);
        if (1 == pnode.shunning_protected)
        {
            prot_mode = ci_service_shunning(pnode.tp_id, pnode.prog_number);
        }
        else
        {
            prot_mode = CI_PROTECTED_IN_ACTIVE;
        }

        if(prot_mode == CI_PROTECTED_IN_ACTIVE)
        {
            api_ci_start_service(&ci_dev_list, &ci_service, ts_route->ci_slot_mask);
        }
        else
        {
            api_ci_stop_service(NULL, &ci_service, ts_route->ci_slot_mask);
            api_set_nim_ts_type(2, ts_route->ci_slot_mask);
        }
      #else //CI
        api_ci_start_service(&ci_dev_list, &ci_service, ts_route->ci_slot_mask);
      #endif
    #endif

#endif
    }
    else
    {
        CC_PRINTF("\nCI Not Start\n");
    }
#ifdef CI_PLUS_SERVICESHUNNING
    ci_start_sdt_filter(es->tp_id);
#endif

#ifdef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
    if(!es->not_oprate_subt_ttx)
    {
    #if (SUBTITLE_ON == 1)
        subt_register(es->sim_id);
    #endif
    #if (TTX_ON == 1)
        ttx_register(es->sim_id);
    #endif

    #if (ISDBT_CC == 1)
        isdbtcc_register(es->sim_id);
    #endif
    }
#endif

#if (defined(_MHEG5_V20_ENABLE_) )
    UINT16 prog_pos = 0xffff;

    prog_pos = get_prog_pos(es->prog_id);
    mheg5_receiver_start_applications((prog_pos>0xFFF0)?(UINT32)si_get_cur_channel():prog_pos);
#endif

    CC_PRINTF("CC Post Play Channel 6 --- Leave Post Play Channel\n");
}


#if ((defined(CAS9_V6) && defined(MULTI_DESCRAMBLE)) || defined(CAS9_PVR_SCRAMBLED) || defined(BC_PVR_SUPPORT))
UINT8 nim_change_flag = FALSE;
static UINT8 get_nim_change_flag(void)
{
    return nim_change_flag;
}

#ifdef BC_PVR_SUPPORT
void set_nim_change_flag(UINT8 flag, UINT8 pre_nim_id)
{
    //libc_printf("set_nim_change_flag(%d, %d)\n", flag, pre_nim_id);

    g_pre_nim_id = pre_nim_id;
    nim_change_flag = flag;
}
#else
void set_nim_change_flag(UINT8 flag)
{
    nim_change_flag = flag;
}
#endif
#endif

UINT32 cc_play_set_cmdbits(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param)
{
    UINT32 hde_mode = 0;
    UINT32 cmd_bits = cmd;
    struct cc_es_info *es = NULL;
    UINT16 video_pid = 0xffff;
    UINT16 audio_pid = 0xffff;
    P_NODE p_node;
    struct nim_config config;

    if ((NULL == ft) || (NULL == param) || (NULL == &param->es))
    {
        ASSERT(0);
        return cmd_bits;
    }

    es = &param->es;
    video_pid = es->v_pid & 0x1fff;
    audio_pid = es->a_pid & 0x1fff;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&config, 0, sizeof(struct nim_config));
    //Check Hde Mode
    hde_mode = hde_get_mode();
#if ((defined(HDTV_SUPPORT)) && (!defined(CHANCHG_VIDEOTYPE_SUPPORT)))
    if(VIEW_MODE_MULTI == hde_mode)
    {
        cmd_bits &= ~CC_CMD_CLOSE_VPO;
    }
    if (cmd_bits & CC_CMD_CLOSE_VPO)
    {
        cc_enable_fill_blk_screen(FALSE); // fill black screen is slow
    }
    else
    {
        cc_enable_fill_blk_screen(TRUE);
    }
#else
    if((VIEW_MODE_MULTI == hde_mode))/*(hde_mode == VIEW_MODE_PREVIEW) ||*/ 
    {
        cmd_bits &= ~CC_CMD_CLOSE_VPO;
    }
#endif

    if(es->blk_enable)
    {
        cmd_bits |= CC_CMD_FILL_FB;
    }

        //Check Video ES
#if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_) )
    cmd_bits |= CC_CMD_STOP_VIDEO;
    if((es->service_type) &&(video_pid != 0)&&(video_pid != NULL_PID))
    {
        cmd_bits |= CC_CMD_START_VIDEO;
    }
    else
    {
        cmd_bits |= CC_CMD_STOP_VIDEO;
        //if(hde_get_mode() != VIEW_MODE_MULTI)
        //  cmd_bits |= CC_CMD_STOP_VIDEO_VPO;
    }
#else
    cmd_bits |= CC_CMD_STOP_VIDEO;
    if((0 != video_pid) && (NULL_PID != video_pid))
    {
        cmd_bits |= CC_CMD_START_VIDEO;
#ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
        cmd_bits |= CC_CMD_INFO_VPO_CHANCHNG;
#endif
    }
    else
    {
        if(SUCCESS==get_prog_by_id(es->prog_id, &p_node))
        {
            if(p_node.av_flag)
            {
                cmd_bits |= CC_CMD_STOP_VIDEO;
            }
        }
    }
#endif

        //Check Audio ES
#if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_) )
    cmd_bits |= CC_CMD_STOP_AUDIO;
    if((0 != audio_pid) && (NULL_PID != audio_pid))
    {
        cmd_bits |= CC_CMD_START_AUDIO;
    }

    if(((0 == video_pid) || (NULL_PID == video_pid)) && (0 != audio_pid) && (audio_pid < MAX_VALID_PID))
    {
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    }
    if( ((0 == video_pid) || (NULL_PID == video_pid)) && ((0 == audio_pid) || (NULL_PID == audio_pid)))
    {
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    }
#else
    cmd_bits |= CC_CMD_STOP_AUDIO;
    if((audio_pid != 0) && (audio_pid != NULL_PID))
    {
        cmd_bits |= CC_CMD_START_AUDIO;
    }
    //If Radio Channel, don't Close Vpo or Fill Video Frame Buffer
    if(((0 == video_pid) || (NULL_PID == video_pid)) && (audio_pid != 0) && (audio_pid < MAX_VALID_PID))
    {
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    }
#endif

#ifdef FSC_SUPPORT
    if((param->is_fsc_play) && (param->fsc_play_mode == TS_ROUTE_MAIN_PLAY))
    {
        if(param->dmx_info.dmx_av_mode == FCC_AUDIO_ENABLE)
        {
            cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);;
        }
    }
#endif

    cmd_bits |= CC_CMD_STOP_DMX|CC_CMD_START_DMX;

    cmd_bits |= CC_CMD_START_SI_MONITOR;

    cmd_bits |= CC_CMD_STOP_MCAS_SERVICE;

    if(ft->nim != NULL)
    {
        //Judge whether antenna and transponder need change or not
        dev_get_nim_config(ft->nim, ft->xpond.common.type, &config);
        if(1 == ft->antenna.antenna_enable)
        {
            //do it in driver act, for: set front after dmx&dec closed
            //frontend_set_antenna(ft->nim, &ft->antenna, &ft->xpond, 1);

            if((ft->xpond.s_info.pol != config.xpond.s_info.pol)
                ||(MEMCMP(&ft->antenna, &config.antenna, \
                sizeof(struct ft_antenna))))
            {
                cmd_bits |= CC_CMD_SET_ANTENNA;
            }
        }
        if(MEMCMP(&ft->xpond, &config.xpond, sizeof(union ft_xpond)))
        {
            //xpond_change = 1;
            cmd_bits |= CC_CMD_SET_XPOND;
            //need send channel change command to UNICABLE LNB
            if(LNB_CTRL_UNICABLE == ft->antenna.lnb_type)
            {
                cmd_bits |= CC_CMD_SET_ANTENNA;
            }
        }
        else if(TRUE == frontend_check_nim_param(ft->nim, &ft->xpond, &ft->antenna))
        {
            //xpond_change = 1;
            cmd_bits |= CC_CMD_SET_XPOND;
        }
#if (defined(PUB_PORTING_PUB29)) && ((SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC)) )
        play_xpond_info.frq = ft->xpond.t_info.frq;
        play_xpond_info.sym = ft->xpond.t_info.sym;
        play_xpond_info.pol = 0;
        play_xpond_info.guard_interval = ft->xpond.t_info.guard_intl;
        play_xpond_info.FFT = ft->xpond.t_info.fft_mode;
        play_xpond_info.modulation = ft->xpond.t_info.modulation;
        play_xpond_info.bandwidth = ft->xpond.t_info.band_width/1000;
        play_xpond_info.inverse = ft->xpond.t_info.inverse;
#endif

        #if ((defined(CAS9_V6) && defined(MULTI_DESCRAMBLE)) || defined(CAS9_PVR_SCRAMBLED) || defined(BC_PVR_SUPPORT))
        if(TRUE == get_nim_change_flag())
        {
            cmd_bits |= CC_CMD_SET_ANTENNA;
        }
        #endif
    }
    return cmd_bits;
}

#ifdef PUB_PORTING_PUB29
UINT8 get_cc_crnt_state(void)
{
    return l_cc_crnt_state;
}
#endif

/* Stop Channel *
 * For Example, when enter main menu, this function will be
 *called by upper layer
*/
void cc_stop_channel(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    UINT32 cmd_bits = cmd;
    UINT32 hde_mode = 0;

    if ((NULL == es) || (NULL == dev_list))
    {
        return;
    }

#if (defined(_MHEG5_V20_ENABLE_) )
    mheg5_receiver_stop_applications();
#endif

    CC_PRINTF("CC Stop Channel 1 --- Set Cmd Bits\n");
    /*Check Hde Mode to Determine Whether Close Vpo*/
    hde_mode = hde_get_mode();

#if defined(HDTV_SUPPORT) && !defined(CHANCHG_VIDEOTYPE_SUPPORT)
    if(VIEW_MODE_MULTI ==hde_mode)
    {
        cmd_bits &= ~CC_CMD_CLOSE_VPO;
    }
    if (cmd_bits & CC_CMD_CLOSE_VPO)
    {
        cc_enable_fill_blk_screen(FALSE); // fill black screen is slow
    }
    else
    {
        cc_enable_fill_blk_screen(TRUE);
    }
#else
    if((VIEW_MODE_MULTI == hde_mode))/*(hde_mode == VIEW_MODE_PREVIEW) || */
    {
        cmd_bits &= ~CC_CMD_CLOSE_VPO;
    }
#endif

    // For Invalid-Program, Audio and Video PID are both 0
    if((0 == es->v_pid) && (0 == es->a_pid))
    {
        cmd_bits |= (CC_CMD_STOP_VIDEO | CC_CMD_STOP_AUDIO);
    }

    //Check Video ES
    if(es->v_pid != 0)
    {
        cmd_bits |= CC_CMD_STOP_VIDEO;
    }

    //Check Audio ES
    if(es->a_pid != 0)
    {
        cmd_bits |= CC_CMD_STOP_AUDIO;
        //If Radio Channel, don't Close Vpo or Fill Video Frame Buffer
        if(0 == es->v_pid)
        {
            cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        }
    }
#if !(defined(SUPPORT_HW_CONAX)|defined(MULTI_CAS))
  #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    cmd_bits |= CC_CMD_STOP_DMX;

    //Process Before Stop Channel
    CC_PRINTF("CC Stop Channel 2 --- Pre Stop Channel\n");
    //0225 ECM ISSUE, DON'T STOP SERVICE.
    cc_pre_stop_channel(cmd_bits,dev_list->dmx, es, 0);
  #else
    cmd_bits |= CC_CMD_STOP_DMX;
    CC_PRINTF("CC Stop Channel 2 --- Pre Stop Channel\n");
    //0225 ECM ISSUE, DON'T STOP SERVICE.
    cc_pre_stop_channel(cmd_bits,dev_list->dmx, es, 1);
  #endif
#else //hwca && swca
    cmd_bits |= CC_CMD_DMX_DISABLE;
    //0225 ECM ISSUE, DON'T STOP SERVICE.
    cc_pre_stop_channel(cmd_bits,dev_list->dmx, es, 0);
#endif

#ifdef FSC_SUPPORT    
    struct cc_param *pcc_param = container_of(es, struct cc_param,es);
    UINT8 stop_mode = pcc_param->stop_mode;
    if((cmd_bits & CC_CMD_FSC_STOP) && (1 == fsc_stop_mode_get() || stop_mode))
    {
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_STOP_AUDIO|CC_CMD_STOP_VIDEO| \
                CC_CMD_START_AUDIO|CC_CMD_START_VIDEO|CC_CMD_FILL_FB);
    }

    fsc_channel_stop(cmd_bits, es, dev_list);
#endif
    //Stop Channel
    CC_PRINTF("CC Stop Channel 3 --- Driver Act\n");
    cc_driver_act(cmd_bits, es, dev_list, NULL);
    CC_PRINTF("CC Stop Channel 4 --- Leave Stop Channel\n");

#ifdef PUB_PORTING_PUB29
    l_cc_crnt_state = CC_STATE_STOP;
#endif
}


/*****************************************************
*external APIs
******************************************************/
void chchg_init(void)
{
    BOOL ret_bool = FALSE;

#ifdef CHCHG_TICK_PRINTF
    struct vdec_io_reg_callback_para tpara;
    struct vdec_device *v_dec = NULL;
    struct vdec_device *avc_dec = NULL;
    v_dec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    avc_dec = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");

    MEMSET(&tpara, 0, sizeof(tpara));
#endif

    l_cc_sema_id = osal_semaphore_create(1);
    if (OSAL_INVALID_ID == l_cc_sema_id)
    {
        CC_PRINTF("Create l_cc_sema_id failed!\n");
    }

    ret_bool = libpub_cmd_handler_register(LIBPUB_CMD_CC, cc_cmd_implement);
    ASSERT(ret_bool);
    ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_CC_PLAY);
    ASSERT(ret_bool);
    ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_CC_STOP);
    ASSERT(ret_bool);

#ifdef CHCHG_TICK_PRINTF
    reset_vdec_first_show();
    tpara.e_cbtype = VDEC_CB_FIRST_SHOWED;
    tpara.p_cb = cb_vdec_first_show;
    vdec_io_control(v_dec, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
    vdec_io_control(avc_dec, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
#endif

#ifdef CC_USE_TSG_PLAYER
    cc_tsg_task_init();
#endif

#if defined(FSC_SUPPORT)
    cc_backup_mutex_init();
    //cc_backup_dma_init(TRUE);
#endif
}

INT32 chchg_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn)
{
    struct libpub_cmd cmd;
    INT32 ret = RET_FAILURE;

    if ((NULL == ft) || (NULL == param))
    {
        return RET_FAILURE;
    }

    MEMSET(&cmd, 0, sizeof(struct libpub_cmd));
    osal_task_dispatch_off();
    cc_cmd_cnt++;
    osal_task_dispatch_on();

    ENTER_PUB_API();

    cmd.cmd_type = LIBPUB_CMD_CC_PLAY;
#ifdef STAR_ADV_SUPPORT
    if(FLAG_TRUE)
#else
    if(VIEW_MODE_MULTI == hde_get_mode())
#endif
    {
        cmd.sync = 1;
    }
    else
    {
        cmd.sync = 0;
    }
    if(blk_scrn)
    {
        cmd.para1 = 1;
    }
    
    ASSERT(LIBPUB_CMD_DATA_SIZE >= sizeof(struct ft_frontend));
    ASSERT(LIBPUB_CMD_DATA2_SIZE >= sizeof(struct cc_param));
    
    MEMCPY(cmd.data1, ft, sizeof(struct ft_frontend));
    MEMCPY(cmd.data2, param, sizeof(struct cc_param));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    if(cmd.sync)
    {
        ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_PLAY_BIT, OSAL_WAIT_FOREVER_TIME);
    }

    LEAVE_PUB_API();
    return ret;
}

INT32 chchg_stop_channel(struct cc_es_info *es, struct cc_device_list *dev_list, BOOL blk_scrn)
{
    struct libpub_cmd cmd;
    INT32 ret = RET_FAILURE;

    if((NULL == es) || (NULL == dev_list))
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));
    ENTER_PUB_API();
    cmd.cmd_type = LIBPUB_CMD_CC_STOP;
    cmd.para1 = blk_scrn;
    cmd.sync = 1;
    MEMCPY(cmd.data1, es, sizeof(struct cc_es_info));
    MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_STOP_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

INT32 chchg_pause_video(struct vdec_device *vdec)
{
    struct libpub_cmd cmd;
    struct cc_device_list dev_list;
    INT32 ret = RET_FAILURE;

    if(NULL == vdec)
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));
    MEMSET(&dev_list, 0, sizeof(dev_list));

    ENTER_PUB_API();
    cmd.cmd_type = LIBPUB_CMD_CC_PAUSE_VIDEO;
    cmd.sync = 1;
    dev_list.vdec = vdec;
    MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_PAUSE_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}


INT32 chchg_resume_video(struct vdec_device *vdec)
{
    struct libpub_cmd cmd;
    struct cc_device_list dev_list;
    INT32 ret = RET_FAILURE;

    if(NULL == vdec)
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));
    MEMSET(&dev_list, 0, sizeof(dev_list));

    ENTER_PUB_API();
    cmd.cmd_type = LIBPUB_CMD_CC_RESUME_VIDEO;
    cmd.sync = 1;
    dev_list.vdec = vdec;
    MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_PAUSE_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

INT32 chchg_switch_audio_pid(struct cc_es_info *es, struct cc_device_list *dev_list)
{
    struct libpub_cmd cmd;
    INT32 ret = !SUCCESS;

    if((NULL == es) || (NULL == dev_list))
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));

    ENTER_PUB_API();
    cmd.cmd_type = LIBPUB_CMD_CC_SWITCH_APID;
    cmd.sync = 1;
    MEMCPY(cmd.data1, es, sizeof(struct cc_es_info));
    MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_SWITCH_APID_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

INT32 chchg_switch_audio_chan(struct snd_device *snd, UINT8 audio_chan)
{
    struct libpub_cmd cmd;
    struct cc_device_list dev_list;
    struct cc_es_info *es = NULL;
    INT32 ret = RET_FAILURE;

    if((NULL == snd)||((UINT8)-1== audio_chan))
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));
    MEMSET(&dev_list, 0, sizeof(dev_list));

    ENTER_PUB_API();

    cmd.cmd_type = LIBPUB_CMD_CC_SWITCH_ACH;
    cmd.sync = 1;
    dev_list.snd_dev = snd;
    es = (struct cc_es_info *)(cmd.data1);
    es->a_ch = audio_chan;
    MEMCPY(cmd.data2, &dev_list, sizeof(dev_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_SWITCH_ACH_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

INT32 chchg_show_logo(struct cc_logo_info *plogo, struct cc_device_list *dev_list)
{
    struct vdec_device *vdec = (struct vdec_device *)get_selected_decoder();
    struct libpub_cmd cmd;
    INT32 ret = RET_FAILURE;

    if((NULL == plogo) || (NULL == dev_list))
    {
        return RET_FAILURE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));

    ENTER_PUB_API();

    if(vdec != NULL)
    {
        /*we must stop video decoder first, before showing logo*/
        vdec_stop(vdec, FALSE, FALSE);
    }
    
    cmd.cmd_type = LIBPUB_CMD_CC_SHOW_LOGO;
    cmd.sync = 1;
    MEMCPY(cmd.data1, plogo, sizeof(struct cc_logo_info));
    MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret=libpub_wait_cmd_finish(LIBPUB_FLG_CC_LOGO_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}

#ifdef CC_USE_TSG_PLAYER
INT32 chchg_start_patch(UINT8 slot_mask)
{
    struct libpub_cmd cmd;
    INT32 ret = RET_FAILURE;

    MEMSET(&cmd, 0, sizeof(cmd));

    ENTER_PUB_API();

    cmd.cmd_type = LIBPUB_CMD_CC_START_PATCH;
    cmd.sync = 1;
    cmd.para1 = (UINT32)slot_mask;
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    ret = libpub_wait_cmd_finish(LIBPUB_FLG_CC_PATCH_BIT, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    return ret;
}
#endif


//a temp solution for comping failure!
//#ifndef CI_SUPPORT
#if 0
void api_ci_start_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
}
void api_ci_stop_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
}
void api_ci_switch_aid(UINT32 prog_id, UINT16 pid)
{
}
#endif

#ifdef CI_PLUS_SERVICESHUNNING
void chchg_operate_ci(UINT32 msg, UINT8 slot_mask)
{
    enum ci_protect_mode prot_mode = CI_PROTECTED_IN_ACTIVE;
    P_NODE pnode;

    if (SUCCESS != get_prog_by_id(ci_service.prog_id, &pnode))
    {
        return;
    }

    MEMSET(&pnode, 0, sizeof(P_NODE));
    CI_SHUNNING_DEBUG("# check pnode.shunning_protected @%s\n", __FUNCTION__);
    CI_SHUNNING_DEBUG("#       pnode.shunning_protected = %d\n", pnode.shunning_protected);
    if (1 == pnode.shunning_protected)
    {
        // then: check current cam is shun or not
        prot_mode = ci_service_shunning(pnode.tp_id, pnode.prog_number);
    }
    else
    {
        // play service immediately, and start SDT monitor
        prot_mode = CI_PROTECTED_IN_ACTIVE;
    }

    if((CI_PROTECTED_IN_ACTIVE == prot_mode) || api_ci_camup_in_progress())
    {
        api_ci_start_service(&ci_dev_list, &ci_service, slot_mask);
    }
    else
    {
        api_ci_stop_service(NULL, &ci_service, slot_mask);
        // We can't bypass CAM here
#ifndef CI_PLUS_SUPPORT
        api_set_nim_ts_type(2, slot_mask);
#endif
    }
}
#endif

INT32 cc_get_cmd_cnt(void)
{
    return cc_cmd_cnt;
}

void cc_set_cmd_cnt(INT32 cnt)
{
	if(-1 == cnt)
	{
		return; 
	}
    cc_cmd_cnt = cnt;
}

void uich_chg_set_video_type(UINT32 flag)
{
    pub_blk_scrn = flag? FALSE : TRUE;
}


BOOL uich_chg_aerial_signal_monitor(struct ft_frontend *ft)
{
    struct libpub_cmd cmd;
    //struct cc_device_list dev_list;
    INT32 ret = RET_FAILURE;

    if (NULL == ft)
    {
        return FALSE;
    }
    MEMSET(&cmd, 0, sizeof(cmd));

    ENTER_PUB_API();
    cmd.cmd_type = LIBPUB_CMD_CC_AERIAL_CHG_CH;
    cmd.para1 = (UINT32)ft;
    //cmd.para1 = sfreq;
    //cmd.para2 = bandwidth;
    //MEMSET(&dev_list, 0, sizeof(dev_list));
    //dev_list.vdec = vdec;
    //MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    ret = libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);

    LEAVE_PUB_API();
    /*struct mid27_cmd this_cmd;

    ENTER_CC_API();

    api_nim_stop_autoscan();    //50916-01Angus

    this_cmd.cmd_type = MID27_CMD_CC_AERIAL_CHG_CH;
    this_cmd.para32 = sfreq;
    this_cmd.para16 = bandwidth;

    mid27_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME);

    LEAVE_CC_API();*/
    if (E_OK == ret)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif


