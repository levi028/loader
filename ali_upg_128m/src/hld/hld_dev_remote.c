/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: hld_dev_remote.c
*
*    Description:    This file contains all functions definations of
*                     HLD device management.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <hld/hld_dev.h>
#include <hld/decv/decv.h>
#include <osal/osal.h>
#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <api/libimagedec/imagedec.h>
#include <api/libmp/pe.h>
#include <hld/deca/deca.h>
#include <hld/dmx/dmx.h>
#include <hld/sdec/sdec.h>
#include <hld/pmu/pmu.h>
#ifdef DUAL_ENABLE
#include <modules.h>
#endif
/*******************************************************************************************************************/

/*
**ADD for PMU RPC.
*/
UINT32 pmu_params_package[] =
{
	/*desc of pointer para.*/
	1, DESC_OUTPUT_STRU(0, sizeof(struct pmu_smc_call_para)),
	1, DESC_P_PARA(0, 3, 0),
	0,
	0,
};

void remote_call_see_execute(UINT32 mcu_src_addr, UINT32 mcu_code_len,
	UINT32 pmu_rpc_no, struct pmu_smc_call_para *pmu_params);
#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
extern void pmu_enter_standby(UINT32 mcu_src_addr, UINT32 mcu_code_len,
	struct pmu_smc_call_para *pmu_params);
extern void pmu_mcu_uart_enable(void);
extern void main_get_mcu_time(struct pmu_smc_call_para *pmu_para);
extern unsigned char get_powerup_type(void);
extern int mcu_set_wakeup_time(struct pmu_smc_call_para *pmu_rpc_para);
extern int mcu_set_current_time(struct pmu_smc_call_para *pmu_rpc_para);
extern void set_panel_power_key(unsigned char panel_key);
extern void set_ir_power_key(unsigned int ir_key1, unsigned int ir_key2);
#endif
/*******************************************************************************************************************/

//#ifdef MAIN_CPU
#if !defined(DUAL_ENABLE) || defined(MAIN_CPU)
#include <hal/hal_gpio.h>

extern void ct1642_multi_gpio_set(INT8 gpio_num, UINT8 flag);
void hld_snd_mute_by_gpio(UINT32 gpio_num, UINT32 flag, int is_by_ext_gpio)
{
	if (!is_by_ext_gpio) {
		HAL_GPIO_BIT_DIR_SET(gpio_num, HAL_GPIO_O_DIR);
		HAL_GPIO_BIT_SET(gpio_num, (UINT8)flag);
	}
#ifndef _BOOT_AV
	else {
		ct1642_multi_gpio_set((INT8)gpio_num, (UINT8)flag);
	}
#endif

	if (flag) {
            osal_task_sleep(20);
    }
}
#endif

#ifdef DUAL_ENABLE
#ifdef MAIN_CPU
struct vdec_callback g_vdec_cb;
struct vpo_callback g_vpo_cb;
struct snd_callback g_snd_cb;
struct img_callback g_img_cb;
struct img_callback g_vde_cb;
struct img_callback g_mus_cb;
struct deca_callback g_deca_cb;
struct snd_moniter_callback g_snd_moniter_cb;
struct deca_data_callback g_deca_data_cb;
#else
#include <stdlib.h>
#include <string.h>
extern ID g_hsr_thread;
#endif

#define DRAM_SPLIT_CTRL_BASE 0xb8041000
#define PVT_S_ADDR 0x10

#ifdef MAIN_CPU
static void hld_vdec_cb(UINT32 u_param);
static void hld_vpo_cb(UINT32 u_param);
static void hld_vpo_hdmi_cb(UINT32 u_param);
static void hld_snd_hdmi_cb(UINT32 u_param);
static void hld_snd_spec_cb(UINT32 u_param);
static void hld_img_cb(unsigned long type, unsigned long param);
static void hld_vde_cb(unsigned long type, unsigned long param);
static void hld_mus_cb(unsigned long type, unsigned long param);
static void hld_deca_cb(UINT32 u_param);
static void hld_cache_flush(UINT32 in, UINT32 data_length);
static void hld_cache_inv(UINT32 in, UINT32 data_length);
static void hld_vdec_spec_cb(UINT32 u_param);
static void hld_snd_moniter_cb(UINT32 u_param);
static void hld_vdec_info_cb(UINT32 u_param);
static void hld_dmx_spec_cb(UINT32 u_param);
static void hld_subtitle_cb(UINT32 uparam);
static INT8 hld_ts_data_cb(UINT32 uparam);
static void remote_hld_dev_see_printf_to_main_with_rpc(struct see_printf_to_main_with_rpc_msg *msg);
#endif

extern void osd_subt_enter(void);
extern void osd_subt_leave(void);

#ifdef SEE_CPU
INT8 ts_see2main_ret = 0;
#endif

#if ((!defined _M3515_SECURE_BOOT_ENABLE_)||(!defined _BL_ENABLE_))
static RET_CODE remote_hld_notify_see_trig(__attribute__((unused))UINT32 param)
{
    return RET_FAILURE;
}

#else
UINT32 remote_hld_notify_see_trig(UINT32 param);
#endif

enum HLD_DECA_FUNC
{
	FUNC_HLD_DEV_ADD = 0,
	FUNC_HLD_DEV_REMOVE,
	FUNC_HLD_MEM_CPY,
	FUNC_HLD_SEE_INIT,
	FUNC_CPY_FROM_PRIV_MEM,
	FUNC_HLD_NOTIFY_SEE_TRIG,
	FUNC_HLD_OTP_GET_MUTEX,
	FUNC_HLD_EN_HD_DECODER,
	FUNC_HLD_PMU_RPC,
	FUNC_HLD_VDEC_CB,
	FUNC_HLD_VPO_CB,
	FUNC_HLD_VPO_HDMI_CB,
	FUNC_HLD_SND_HDMI_CB,
	FUNC_HLD_SND_SPEC_CB,
	FUNC_HLD_IMG_CB,
	FUNC_HLD_VDE_CB,
	FUNC_HLD_MUS_CB,
	FUNC_HLD_DECA_CB,
	FUNC_HLD_CACHE_FLUSH,
	FUNC_HLD_CACHE_INV,
	FUNC_HLD_VDEC_SPEC_CB,
	FUNC_HLD_SND_MONITER_CB,
	FUNC_HLD_VDEC_INFO_CB,
	FUNC_HLD_DMX_SPEC_CB,
	FUNC_HLD_SUBTITLE_CB,
	FUNC_HLD_TS_DATA_CB,
	FUNC_HLD_DEV_SEE_PRINTF_TO_MAIN_WITH_RPC,
	FUNC_HLD_SND_MUTE_BY_GPIO,
};

static UINT32 hld_dev_entry[] =
{
	(UINT32)remote_hld_dev_add,
	(UINT32)remote_hld_dev_remove,
	(UINT32)remote_hld_dev_memcpy,
	(UINT32)remote_hld_see_init,
	(UINT32)remote_cpy_from_priv_mem,
	(UINT32)remote_hld_notify_see_trig,
	(UINT32)remote_otp_get_mutex,
	(UINT32)remote_enable_hd_decoder,
	(UINT32)remote_call_see_execute,
#ifdef MAIN_CPU
	(UINT32)hld_vdec_cb,
	(UINT32)hld_vpo_cb,
	(UINT32)hld_vpo_hdmi_cb,
	(UINT32)hld_snd_hdmi_cb,
	(UINT32)hld_snd_spec_cb,
	(UINT32)hld_img_cb,
	(UINT32)hld_vde_cb,
	(UINT32)hld_mus_cb,
	(UINT32)hld_deca_cb,
	(UINT32)hld_cache_flush,
	(UINT32)hld_cache_inv,
	(UINT32)hld_vdec_spec_cb,
	(UINT32)hld_snd_moniter_cb,
	(UINT32)hld_vdec_info_cb,
	(UINT32)hld_dmx_spec_cb,
	(UINT32)hld_subtitle_cb,
	(UINT32)hld_ts_data_cb,
	(UINT32)remote_hld_dev_see_printf_to_main_with_rpc,
	(UINT32)hld_snd_mute_by_gpio,
#endif
};

static UINT32 desc_hld_dev[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct hld_device)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

#ifdef MAIN_CPU
static void hld_vdec_cb(UINT32 uparam)
{
    UINT8 type = uparam&0xff;
    UINT8 uparam1 = (uparam>>8)&0xff;
    UINT8 uparam2 = (uparam>>16)&0xff;

    switch(type)
    {
        case VDEC_CB_FIRST_SHOWED:
            if(g_vdec_cb.pcb_first_showed)
            {
                g_vdec_cb.pcb_first_showed(uparam1, uparam2);
            }
            break;
        case VDEC_CB_MODE_SWITCH_OK:
            if(g_vdec_cb.pcb_mode_switch_ok)
            {
                g_vdec_cb.pcb_mode_switch_ok(uparam1, uparam2);
            }
            break;
        case VDEC_CB_BACKWARD_RESTART_GOP: // used for advance play
            if(g_vdec_cb.pcb_backward_restart_gop)
            {
                g_vdec_cb.pcb_backward_restart_gop(uparam1, uparam2);
            }
            break;
        case VDEC_CB_FIRST_HEAD_PARSED:
            if(g_vdec_cb.pcb_first_head_parsed)
            {
                g_vdec_cb.pcb_first_head_parsed(uparam1, uparam2);
            }
            break;
        case VDEC_CB_MONITOR_FRAME_VBV:
            if(g_vdec_cb.pcb_new_frame_coming)
            {
                g_vdec_cb.pcb_new_frame_coming(uparam1, uparam2);
            }
            break;
        case VDEC_CB_MONITOR_VDEC_START:
            if(g_vdec_cb.pcb_vdec_start)
            {
                g_vdec_cb.pcb_vdec_start(uparam1, uparam2);
            }
            break;
        case VDEC_CB_MONITOR_VDEC_STOP:
            if(g_vdec_cb.pcb_vdec_stop)
            {
                g_vdec_cb.pcb_vdec_stop(uparam1, uparam2);
            }
            break;
        case VDEC_CB_FIRST_I_DECODED:
            if(g_vdec_cb.pcb_first_i_decoded)
            {
                g_vdec_cb.pcb_first_i_decoded(uparam1, uparam2);
            }
            break;
        case VDEC_CB_ERROR:
            if(g_vdec_cb.pcb_vdec_error)
            {
                g_vdec_cb.pcb_vdec_error((uparam>>8), 0);
            }
            break;
        case VDEC_CB_STATE_CHANGED:
            if(g_vdec_cb.pcb_vdec_state_changed)
            {
                g_vdec_cb.pcb_vdec_state_changed(uparam1, 0);
            }
            break;
		case VDEC_CB_DECODER_FINISH:
            if(g_vdec_cb.pcb_vdec_decoder_finish)
            {
                g_vdec_cb.pcb_vdec_decoder_finish(uparam1, uparam2);
            }
            break;
        case VDEC_CB_MALLOC_DONE:
            if (g_vdec_cb.pcb_vdec_malloc_done)
            {
                g_vdec_cb.pcb_vdec_malloc_done(uparam, 0);
            }
            break;
		case VDEC_CB_MONITOR_GOP:
            if(g_vdec_cb.pcb_vdec_monitor_gop)
            {
                g_vdec_cb.pcb_vdec_monitor_gop(uparam1, uparam2);
            }
            break;
        case VDEC_CB_FRAME_DISPLAYED:
            if(g_vdec_cb.pcb_frame_displayed)
            {
                g_vdec_cb.pcb_frame_displayed(uparam1, uparam2);
            }
            break;
        default:
            break;
    }
}

void hld_vdec_spec_cb(UINT32 uparam)
{
    if(g_vdec_cb.pcb_vdec_user_data_parsed)
    {
        g_vdec_cb.pcb_vdec_user_data_parsed(uparam, 0);
    }
}

void hld_vdec_info_cb(UINT32 uparam)
{
    if(g_vdec_cb.pcb_vdec_info_changed)
    {
        g_vdec_cb.pcb_vdec_info_changed(uparam, 0);
    }
}

void hld_dmx_spec_cb(UINT32 uparam)
{
    //struct dmx_data_see2main_param *dmx_see_user_data = (struct dmx_data_see2main_param *)uparam;

    dmx_get_see_data((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),uparam);
}

INT8 hld_ts_data_cb(UINT32 uparam)
{
#if(SUBTITLE_ON == 1)
    struct ts_see2main_data_cb_new_s *see2main_info_new = NULL;
    UINT8 *data_subt_buff = NULL;
    struct control_block *tmp_ctrl_block_p = NULL;
    UINT32 tmp_data_size = 0;

    void *subt_rq_buff = NULL;
    UINT32 subt_rq_buff_size = 0;
    INT32 sbut_ret = 0;
    struct sdec_device *subt_dev = NULL;
    UINT8 subt_pkt_cnt = 0;
    UINT8 i = 0;

    subt_dev = (struct sdec_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_SDEC);

    see2main_info_new = (struct ts_see2main_data_cb_new_s *)(uparam);
    subt_pkt_cnt = see2main_info_new->subt_pkt_num;

    //libc_printf("cnt:%u\n",subt_pkt_cnt);
    for(i=0;i<subt_pkt_cnt;i++)
    {
        tmp_ctrl_block_p = &(see2main_info_new->subt_pkt[i].ctrl_blk);
        tmp_data_size = see2main_info_new->subt_pkt[i].subt_pkt_size;
        data_subt_buff = see2main_info_new->subt_pkt[i].subt_pkt_buff;

        while(tmp_data_size > 0)
        {
            do
            {
                sbut_ret = sdec_request_write(subt_dev,tmp_data_size,(void**)(&subt_rq_buff),(UINT32*)(&subt_rq_buff_size),tmp_ctrl_block_p);
                if(RET_SUCCESS != sbut_ret)         /** subt buffer rq fail. **/
                {
                    libc_printf("fl\n");
                    if(RET_STA_ERR != sbut_ret)     /** subt not stopped **/
                    {
                        /** sleep?? **/
                    }
                    else
                    {
                        /** sleep?? **/
                    }
                }
                else
                {
                    /** Need to process(reset) pts_valid flag **/
                    break;
                }
            }while(1);

            /** Need to process data_continue flag. **/
            if(subt_rq_buff_size >= tmp_data_size) /** Enough buffer. **/
            {
                MEMCPY(subt_rq_buff,data_subt_buff,tmp_data_size);
                sdec_update_write(subt_dev,tmp_data_size);
                tmp_data_size = 0;
            }
            else
            {
                /** bufer not enough. **/
                MEMCPY(subt_rq_buff,data_subt_buff,subt_rq_buff_size);
                sdec_update_write(subt_dev,subt_rq_buff_size);

                tmp_data_size -= subt_rq_buff_size;
                data_subt_buff += subt_rq_buff_size;
            }
        }

    }
#endif
    return 1;
}



void hld_subtitle_cb(UINT32 uparam)
{
    #if(SUBTITLE_ON == 1)
    struct subtitle_cb_info_s subt_cb_info;
    UINT8 func_type_tmp = 0;
    struct sdec_device *subt_dev = NULL;

    subt_dev = (struct sdec_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_SDEC);

    MEMCPY(&subt_cb_info,(struct subtitle_cb_info_s *)uparam,sizeof(struct subtitle_cb_info_s));
    func_type_tmp = subt_cb_info.func_type;

    switch(func_type_tmp)
    {
        case SDEC_START_T:{
            sdec_start(subt_dev,subt_cb_info.param2,subt_cb_info.param3);
            // SDBBP();
            /** call sdec_start. **/
            break;
        }
        case SDEC_STOP_T:{
            sdec_stop(subt_dev);
            /** call sdec_stop. **/
            break;
        }
        case SDEC_OSD_ENTER_T:{
            osd_subt_enter();
            /** call osd_sdec_enter. **/
            break;
        }
        case SDEC_OSD_LEAVE_T:{
            osd_subt_leave();
            /** call osd_sdec_leave. **/
            break;
        }
        default:{
        }
    };
    #endif
}

static void hld_vpo_cb(UINT32 uparam)
{
    UINT8 type = uparam&0xff;
    UINT8 uparam1 = (uparam>>8)&0xff;

    switch(type)
    {
        case VPO_CB_SRCASPRATIO_CHANGE:
            if(g_vpo_cb.p_src_asp_ratio_change_call_back)
            {
                g_vpo_cb.p_src_asp_ratio_change_call_back(uparam1);
            }
            break;
        case VPO_CB_REPORT_TIMECODE:
            if(g_vpo_cb.report_timecode_callback)
            {
                g_vpo_cb.report_timecode_callback(uparam1);
            }
            break;
        default:
            break;
    }
}

static void hld_vpo_hdmi_cb(UINT32 uparam)
{
    if(g_vpo_cb.phdmi_callback)
    {
        g_vpo_cb.phdmi_callback(uparam);
    }
}

static void hld_snd_hdmi_cb(UINT32 uparam)
{
    if(g_snd_cb.phdmi_snd_cb)
    {
        g_snd_cb.phdmi_snd_cb(uparam);
    }
}

static void hld_snd_spec_cb(UINT32 uparam)
{
    if(g_snd_cb.spec_call_back)
    {
        g_snd_cb.spec_call_back((INT32 *)uparam);
    }
}

static void hld_img_cb(unsigned long type, unsigned long param)
{
    if(g_img_cb.mp_cb)
    {
        g_img_cb.mp_cb(type, param);
    }
}

static void hld_vde_cb(unsigned long type, unsigned long param)
{
    if(g_vde_cb.mp_cb)
    {
        g_vde_cb.mp_cb(type, param);
    }
}

static void hld_mus_cb(unsigned long type, unsigned long param)
{
    if(g_mus_cb.mp_cb)
    {
        g_mus_cb.mp_cb(type, param);
    }
}

static void hld_deca_cb(UINT32 uparam)
{
    struct deca_io_reg_callback_para *deca_moniter_cb_dlts = (struct deca_io_reg_callback_para *)uparam;

    switch(deca_moniter_cb_dlts->e_cbtype)
    {
        case DECA_CB_MONITOR_NEW_FRAME:
            if(g_deca_cb.pcb_output_new_frame)
            {
                g_deca_cb.pcb_output_new_frame((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_MONITOR_START:
            if(g_deca_cb.pcb_deca_start)
            {
                g_deca_cb.pcb_deca_start((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_MONITOR_STOP:
            if(g_deca_cb.pcb_deca_stop)
            {
                g_deca_cb.pcb_deca_stop((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_MONITOR_DECODE_ERR:
            if(g_deca_cb.pcb_deca_decode_err)
            {
                g_deca_cb.pcb_deca_decode_err((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_MONITOR_OTHER_ERR:
            if(g_deca_cb.pcb_deca_other_err)
            {
                g_deca_cb.pcb_deca_other_err((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_STATE_CHANGED:
            if(g_deca_cb.pcb_deca_state_change)
            {
                g_deca_cb.pcb_deca_state_change((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        case DECA_CB_ASE_DATA_END:
            if(g_deca_cb.pcb_deca_ase_data_end)
            {
                g_deca_cb.pcb_deca_ase_data_end((UINT32)deca_moniter_cb_dlts, (UINT32)NULL);
            }
            break;
        default:
            break;
    }

    return ;
}

static void hld_snd_moniter_cb(UINT32 uparam)
{
    struct snd_io_reg_callback_para *snd_moniter_cb_dlts = (struct snd_io_reg_callback_para *)uparam;

    switch(snd_moniter_cb_dlts->e_cbtype)
    {
        case SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD:
            if(g_snd_moniter_cb.pcb_output_remain_data_below_threshold)
            {
                g_snd_moniter_cb.pcb_output_remain_data_below_threshold(snd_moniter_cb_dlts);
            }
            break;
        case SND_CB_MONITOR_OUTPUT_DATA_END:
            if(g_snd_moniter_cb.pcb_output_data_end)
            {
                g_snd_moniter_cb.pcb_output_data_end(snd_moniter_cb_dlts);
            }
            break;
        case SND_CB_MONITOR_ERRORS_OCCURED:
            if(g_snd_moniter_cb.pcb_output_errors_occured)
            {
                g_snd_moniter_cb.pcb_output_errors_occured(snd_moniter_cb_dlts);
            }
            break;
        case SND_CB_FIRST_FRAME_OUTPUT:
            if(g_snd_moniter_cb.pcb_first_frame_output)
            {
                g_snd_moniter_cb.pcb_first_frame_output(snd_moniter_cb_dlts);
            }
            break;
        default:
            break;
    }
    return ;
}

static void hld_cache_flush(UINT32 in, UINT32 data_length)
{
    if ( !osal_dual_is_private_address ( (void *)in ) )
    {
        osal_cache_flush ( (void *)in, data_length );
    }
}
static void hld_cache_inv(UINT32 in, UINT32 data_length)
{
    if ( !osal_dual_is_private_address ( (void *)in) )
    {
        osal_cache_invalidate ( (void *)in, data_length );
    }
}

/*
* main cpu trig the see
*/
UINT32 hld_main_notify_see_trig(UINT32 param)
{

    jump_to_func(NULL, os_hld_caller, param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_NOTIFY_SEE_TRIG, NULL);
}
void remote_hld_dev_see_printf_to_main_with_rpc(struct see_printf_to_main_with_rpc_msg *msg)
{
   libc_printf("<see>%s", msg->str);
}
#endif

void hld_base_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_dev_entry, msg);
}

INT32 hld_dev_add_remote(__attribute__((unused))struct hld_device *dev, __attribute__((unused))UINT32 dev_addr)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_DEV_ADD, desc_hld_dev);
}

INT32 hld_dev_remove_remote(__attribute__((unused))struct hld_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_DEV_REMOVE, desc_hld_dev);
}

void hld_dev_memcpy(__attribute__((unused))void *dest, __attribute__((unused))const void *src, __attribute__((unused))unsigned int len)
{
    jump_to_func(NULL, os_hld_caller, dest, (HLD_BASE_MODULE<<24)|(3<<16)|FUNC_HLD_MEM_CPY, NULL);
}
/*
static void copy_from_priv_mem(void *dest, UINT32 data_len)
{
    UINT32 desc[] = {1, DESC_OUTPUT_STRU(0, 64),1, DESC_P_PARA(0, 0, 0), 0, 0,};

    jump_to_func(NULL, OS_hld_caller, dest, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_CPY_FROM_PRIV_MEM, desc);
}
*/

void hld_otp_get_mutex(__attribute__((unused))UINT32 flag)
{
    jump_to_func(NULL, os_hld_caller, flag, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_OTP_GET_MUTEX, NULL);
}

void hld_dev_see_init(__attribute__((unused))void *addr)
{
    jump_to_func(NULL, os_hld_caller, addr, (HLD_BASE_MODULE << 24) | (1 << 16)| FUNC_HLD_SEE_INIT, NULL);
}

void hld_enable_hd_decoder(__attribute__((unused))UINT32 enable)
{
    jump_to_func(NULL, os_hld_caller, enable, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_EN_HD_DECODER, NULL);
}

/********************************************************
*Function: Interface for MAIN cpu to setup pmu rpc process.
*Notice: This API only can be called by MAIN cpu.
********************************************************/
void hld_pmu_remote_call(__attribute__((unused))UINT32 mcu_src_addr, __attribute__((unused))UINT32 mcu_code_len,
	__attribute__((unused))UINT32 pmu_rpc_no, __attribute__((unused))struct pmu_smc_call_para *pmu_params)
{
#ifdef PMU_STANDBY_DEBUG_EN
	libc_printf("\nFunction:%s, Line:%d", __FUNCTION__, __LINE__);
	/*Dump all PMU RPC params.*/
	libc_printf("\npmu_params->smc_current_year_h: 0x%02X", pmu_params->smc_current_year_h);
	libc_printf("\npmu_params->smc_current_year_l: 0x%02X", pmu_params->smc_current_year_l);
	libc_printf("\npmu_params->smc_current_month: 0x%02X", pmu_params->smc_current_month);
	libc_printf("\npmu_params->smc_current_day: 0x%02X", pmu_params->smc_current_day);
	libc_printf("\npmu_params->smc_current_hour: 0x%02X", pmu_params->smc_current_hour);
	libc_printf("\npmu_params->smc_current_min: 0x%02X", pmu_params->smc_current_min);
	libc_printf("\npmu_params->smc_current_sec: 0x%02X", pmu_params->smc_current_sec);

	libc_printf("\npmu_params->smc_wakeup_month: 0x%02X", pmu_params->smc_wakeup_month);
	libc_printf("\npmu_params->smc_wakeup_day: 0x%02X", pmu_params->smc_wakeup_day);
	libc_printf("\npmu_params->smc_wakeup_hour: 0x%02X", pmu_params->smc_wakeup_hour);
	libc_printf("\npmu_params->smc_wakeup_min: 0x%02X", pmu_params->smc_wakeup_min);
	libc_printf("\npmu_params->smc_wakeup_sec: 0x%02X", pmu_params->smc_wakeup_sec);

	libc_printf("\npmu_params->pmu_powerup_type: 0x%02X", pmu_params->pmu_powerup_type);
	libc_printf("\npmu_params->panel_power_key: 0x%02X", pmu_params->panel_power_key);
	libc_printf("\npmu_params->show_panel_type: 0x%02X", pmu_params->show_panel_type);
	libc_printf("\npmu_params->reserved_flag1: 0x%02X", pmu_params->reserved_flag1);

	libc_printf("\npmu_params->reserved_flag2: 0x%02X", pmu_params->reserved_flag2);
	libc_printf("\npmu_params->reserved_flag3: 0x%02X", pmu_params->reserved_flag3);
	libc_printf("\npmu_params->reserved_flag4: 0x%02X", pmu_params->reserved_flag4);
	libc_printf("\npmu_params->reserved_flag5: 0x%02X", pmu_params->reserved_flag5);

	libc_printf("\npmu_params->reserved_flag6: 0x%02X", pmu_params->reserved_flag6);
	libc_printf("\npmu_params->reserved_flag7: 0x%02X", pmu_params->reserved_flag7);
	libc_printf("\npmu_params->reserved_flag8: 0x%02X", pmu_params->reserved_flag8);
	libc_printf("\npmu_params->reserved_flag9: 0x%02X", pmu_params->reserved_flag9);

	libc_printf("\npmu_params->ir_power_key1: 0x%08X", pmu_params->ir_power_key1);
	libc_printf("\npmu_params->text_seg_start_addr: 0x%08X", pmu_params->text_seg_start_addr);
	libc_printf("\npmu_params->text_seg_end_addr: 0x%08X", pmu_params->text_seg_end_addr);
	libc_printf("\npmu_params->mcu_code_start_phys: 0x%08X", pmu_params->mcu_code_start_phys);

	libc_printf("\npmu_params->mcu_code_len: 0x%08X", pmu_params->mcu_code_len);
	libc_printf("\npmu_params->ir_power_key2: 0x%08X", pmu_params->ir_power_key2);
	libc_printf("\npmu_params->wdt_reboot_flag: 0x%08X", pmu_params->wdt_reboot_flag);
	libc_printf("\npmu_params->kernel_str_params_addr: 0x%08X", pmu_params->kernel_str_params_addr);

	libc_printf("\npmu_params->kernel_str_params_len: 0x%08X", pmu_params->kernel_str_params_len);
	libc_printf("\npmu_params->mcu_uart_support: 0x%08X", pmu_params->mcu_uart_support);
	libc_printf("\npmu_params->reserved_flag10: 0x%08X", pmu_params->reserved_flag10);
	libc_printf("\npmu_params->reserved_flag11: 0x%08X", pmu_params->reserved_flag11);

	libc_printf("\npmu_params->reserved_flag12: 0x%08X", pmu_params->reserved_flag12);
	libc_printf("\npmu_params->reserved_flag13: 0x%08X", pmu_params->reserved_flag13);
	libc_printf("\npmu_params->reserved_flag14: 0x%08X", pmu_params->reserved_flag14);
	libc_printf("\npmu_params->reserved_flag15: 0x%08X", pmu_params->reserved_flag15);

	libc_printf("\npmu_params->reserved_flag16: 0x%08X", pmu_params->reserved_flag16);
	libc_printf("\npmu_params->reserved_flag17: 0x%08X", pmu_params->reserved_flag17);
	libc_printf("\npmu_params->reserved_flag18: 0x%08X", pmu_params->reserved_flag18);
	libc_printf("\npmu_params->reserved_flag19: 0x%08X", pmu_params->reserved_flag19);

	libc_printf("\npmu_params->mcu_write_data: 0x%08X", pmu_params->mcu_write_data);
	libc_printf("\npmu_params->mcu_read_data: 0x%08X", pmu_params->mcu_read_data);
	libc_printf("\npmu_params->mcu_rw_offset: 0x%08X", pmu_params->mcu_rw_offset);
	libc_printf("\nFunction:%s, Line:%d\n", __FUNCTION__, __LINE__);
#endif

	jump_to_func(NULL, os_hld_caller, mcu_src_addr,
		(HLD_BASE_MODULE<<24) | (4<<16) | FUNC_HLD_PMU_RPC, pmu_params_package);
}

/********************************************************
*Function: Interface for SEE cpu to complete pmu function.
*Notice: This API only can be called by SEE cpu.
********************************************************/
#define PMU_READ8(i) (*(volatile UINT8 *)(i))
#define PMU_WRITE8(i, d) (*(volatile UINT8 *)(i)) = (d)
#define PMU_READ16(i) (*(volatile UINT16 *)(i))
#define PMU_WRITE16(i, d) (*(volatile UINT16 *)(i)) = (d)
#define PMU_READ32(i) (*(volatile UINT32 *)(i))
#define PMU_WRITE32(i, d) (*(volatile UINT32 *)(i)) = (d)
#define RTC_WRITE32(id, base_add, data) (*(volatile UINT32*)(base_add+id)) = (data)
#define RTC_WRITE16(id, base_add, data) (*(volatile UINT16*)(base_add+id)) = (data)
#define RTC_WRITE8(id, base_add, data) (*(volatile UINT8*)(base_add+id)) = (data)
#define RTC_READ32(id, base_add) (*(volatile UINT32*)(base_add+id))
#define RTC_READ16(id, base_add) (*(volatile UINT16*)(base_add+id))
#define RTC_READ8(id, base_add) (*(volatile UINT8*)(base_add+id))
#define PMU_SET_BIT(address, bit) (*(volatile unsigned char *)(address) |= ((0x1<<(bit))))
#define PMU_READ_BIT(address, bit) ((*(volatile unsigned char *)(address))&(0x1<<(bit)))

#define ALI_PMU_BASE_ADDR (0xB8050000)
#define PANELWAKEUP_KEY_ADDR (0xB8053FFF)
#define SHOW_TYPE_SRAM (0xB8053FFF)
#define PANNEL_POWER_STATUS (0xB8053FFE)
#define EXIT_STANDBY_TYPE_REG (0xB8053FFD)
#define S3922_MCU_RESET_REG (0xB8000278)
#define C3922_MCU_RESET_REG (0xB8082200)
#define M3702_MCU_RESET_REG (0xB8082200)
#define S3922_MCU_RESET_ENABLE (1<<1)
#define C3922_MCU_RESET_ENABLE (1<<8)
#define M3702_MCU_RESET_ENABLE (1<<8)
#define S3922_IP_RESET_ENABLE (1<<0)
#define C3922_IP_RESET_ENABLE (1<<0)
#define M3702_MCU_RESET_DISABLE (~(1<<8))
#define S3922_MCU_RESET_DISABLE (~(1<<1))
#define C3922_MCU_RESET_DISABLE (~(1<<8))
#define S3922_IP_RESET_DISABLE (~(1<<0))
#define C3922_IP_RESET_DISABLE (~(1<<0))
#define ALI_WDT_REBOOT (0x52454254)/*0x52-'R', 0x45-'E', 0x42-'B', 0x54-'T'.*/
#define ALI_PMU_GPIO_CTRL_REG1 (0xB805C057)
#define ALI_PMU_GPIO_CTRL_REG2 (0xB805C05F)

void remote_call_see_execute(__attribute__((unused))UINT32 mcu_src_addr, __attribute__((unused))UINT32 mcu_code_len,
	UINT32 pmu_rpc_no, struct pmu_smc_call_para *pmu_params)
{
#ifdef PMU_STANDBY_DEBUG_EN
	libc_printf("\nFunction:%s, Line:%d", __FUNCTION__, __LINE__);
	/*Dump all PMU RPC params.*/
	libc_printf("\npmu_params->smc_current_year_h: 0x%02X", pmu_params->smc_current_year_h);
	libc_printf("\npmu_params->smc_current_year_l: 0x%02X", pmu_params->smc_current_year_l);
	libc_printf("\npmu_params->smc_current_month: 0x%02X", pmu_params->smc_current_month);
	libc_printf("\npmu_params->smc_current_day: 0x%02X", pmu_params->smc_current_day);
	libc_printf("\npmu_params->smc_current_hour: 0x%02X", pmu_params->smc_current_hour);
	libc_printf("\npmu_params->smc_current_min: 0x%02X", pmu_params->smc_current_min);
	libc_printf("\npmu_params->smc_current_sec: 0x%02X", pmu_params->smc_current_sec);

	libc_printf("\npmu_params->smc_wakeup_month: 0x%02X", pmu_params->smc_wakeup_month);
	libc_printf("\npmu_params->smc_wakeup_day: 0x%02X", pmu_params->smc_wakeup_day);
	libc_printf("\npmu_params->smc_wakeup_hour: 0x%02X", pmu_params->smc_wakeup_hour);
	libc_printf("\npmu_params->smc_wakeup_min: 0x%02X", pmu_params->smc_wakeup_min);
	libc_printf("\npmu_params->smc_wakeup_sec: 0x%02X", pmu_params->smc_wakeup_sec);

	libc_printf("\npmu_params->pmu_powerup_type: 0x%02X", pmu_params->pmu_powerup_type);
	libc_printf("\npmu_params->panel_power_key: 0x%02X", pmu_params->panel_power_key);
	libc_printf("\npmu_params->show_panel_type: 0x%02X", pmu_params->show_panel_type);
	libc_printf("\npmu_params->reserved_flag1: 0x%02X", pmu_params->reserved_flag1);

	libc_printf("\npmu_params->reserved_flag2: 0x%02X", pmu_params->reserved_flag2);
	libc_printf("\npmu_params->reserved_flag3: 0x%02X", pmu_params->reserved_flag3);
	libc_printf("\npmu_params->reserved_flag4: 0x%02X", pmu_params->reserved_flag4);
	libc_printf("\npmu_params->reserved_flag5: 0x%02X", pmu_params->reserved_flag5);

	libc_printf("\npmu_params->reserved_flag6: 0x%02X", pmu_params->reserved_flag6);
	libc_printf("\npmu_params->reserved_flag7: 0x%02X", pmu_params->reserved_flag7);
	libc_printf("\npmu_params->reserved_flag8: 0x%02X", pmu_params->reserved_flag8);
	libc_printf("\npmu_params->reserved_flag9: 0x%02X", pmu_params->reserved_flag9);

	libc_printf("\npmu_params->ir_power_key1: 0x%08X", pmu_params->ir_power_key1);
	libc_printf("\npmu_params->text_seg_start_addr: 0x%08X", pmu_params->text_seg_start_addr);
	libc_printf("\npmu_params->text_seg_end_addr: 0x%08X", pmu_params->text_seg_end_addr);
	libc_printf("\npmu_params->mcu_code_start_phys: 0x%08X", pmu_params->mcu_code_start_phys);

	libc_printf("\npmu_params->mcu_code_len: 0x%08X", pmu_params->mcu_code_len);
	libc_printf("\npmu_params->ir_power_key2: 0x%08X", pmu_params->ir_power_key2);
	libc_printf("\npmu_params->wdt_reboot_flag: 0x%08X", pmu_params->wdt_reboot_flag);
	libc_printf("\npmu_params->kernel_str_params_addr: 0x%08X", pmu_params->kernel_str_params_addr);

	libc_printf("\npmu_params->kernel_str_params_len: 0x%08X", pmu_params->kernel_str_params_len);
	libc_printf("\npmu_params->mcu_uart_support: 0x%08X", pmu_params->mcu_uart_support);
	libc_printf("\npmu_params->reserved_flag10: 0x%08X", pmu_params->reserved_flag10);
	libc_printf("\npmu_params->reserved_flag11: 0x%08X", pmu_params->reserved_flag11);

	libc_printf("\npmu_params->reserved_flag12: 0x%08X", pmu_params->reserved_flag12);
	libc_printf("\npmu_params->reserved_flag13: 0x%08X", pmu_params->reserved_flag13);
	libc_printf("\npmu_params->reserved_flag14: 0x%08X", pmu_params->reserved_flag14);
	libc_printf("\npmu_params->reserved_flag15: 0x%08X", pmu_params->reserved_flag15);

	libc_printf("\npmu_params->reserved_flag16: 0x%08X", pmu_params->reserved_flag16);
	libc_printf("\npmu_params->reserved_flag17: 0x%08X", pmu_params->reserved_flag17);
	libc_printf("\npmu_params->reserved_flag18: 0x%08X", pmu_params->reserved_flag18);
	libc_printf("\npmu_params->reserved_flag19: 0x%08X", pmu_params->reserved_flag19);

	libc_printf("\npmu_params->mcu_write_data: 0x%08X", pmu_params->mcu_write_data);
	libc_printf("\npmu_params->mcu_read_data: 0x%08X", pmu_params->mcu_read_data);
	libc_printf("\npmu_params->mcu_rw_offset: 0x%08X", pmu_params->mcu_rw_offset);
	libc_printf("\nFunction:%s, Line:%d\n", __FUNCTION__, __LINE__);
#endif

	if(STB_ENTER_PMU_STANDBY == pmu_rpc_no)
	{
		/*Clear pmu wakeup type flag.*/
		PMU_WRITE8(EXIT_STANDBY_TYPE_REG, 0x0);

		/*Enter standby.*/
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		pmu_enter_standby(mcu_src_addr, mcu_code_len, pmu_params);
		#endif
	}
	else if(ENABLE_MCU_UART == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		pmu_mcu_uart_enable();
		#endif
	}
	else if(GET_PMU_MCU_TIME == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		main_get_mcu_time(pmu_params);/*system_pmu_init will call it*/
		#endif
	}
	else if(SUSPEND_MCU == pmu_rpc_no)
	{
		if(ALI_C3922 == sys_ic_get_chip_id())
		{
			PMU_WRITE32(C3922_MCU_RESET_REG, PMU_READ32(C3922_MCU_RESET_REG) | C3922_MCU_RESET_ENABLE);
		}
		else if(ALI_C3702 == sys_ic_get_chip_id())
		{
			PMU_WRITE32(M3702_MCU_RESET_REG, PMU_READ32(M3702_MCU_RESET_REG) | M3702_MCU_RESET_ENABLE);
		}
		PMU_WRITE8(ALI_PMU_GPIO_CTRL_REG1, 0);
		PMU_WRITE8(ALI_PMU_GPIO_CTRL_REG2, 0);
	}
	else if(SET_SHOW_PANEL_TYPE == pmu_rpc_no)
	{
		PMU_WRITE8(PANELWAKEUP_KEY_ADDR, pmu_params->show_panel_type);
	}
	else if(GET_POWERUP_TYPE == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		pmu_params->pmu_powerup_type = get_powerup_type();
		#endif
	}
	else if(SET_WAKEUP_TIME == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		mcu_set_wakeup_time(pmu_params);
		#endif
	}
	else if(SET_CURRENT_TIME == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		mcu_set_current_time(pmu_params);
		#endif
	}
	else if(SET_PANEL_POWER_KEY == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		set_panel_power_key(pmu_params->panel_power_key);
		#endif
	}
	else if(SET_IR_POWER_KEY == pmu_rpc_no)
	{
		#if((!defined(_M3503D_)) && (!defined(_M3711C_)))
		set_ir_power_key(pmu_params->ir_power_key1, pmu_params->ir_power_key2);
		#endif
	}
	else if(SAVE_WDT_REBOOT_FLAG == pmu_rpc_no)
	{
		PMU_WRITE32(WDT_REBOOT_FLAG_SAVE_ADDR, pmu_params->wdt_reboot_flag);
	}
	else if(GET_WDT_REBOOT_FLAG == pmu_rpc_no)
	{
		pmu_params->wdt_reboot_flag = PMU_READ32(WDT_REBOOT_FLAG_READ_ADDR);
	}
	else if(GET_WDT_REBOOT_FLAG_FIRST == pmu_rpc_no)
	{
		PMU_WRITE32(WDT_REBOOT_FLAG_READ_ADDR, PMU_READ32(WDT_REBOOT_FLAG_SAVE_ADDR));
		pmu_params->wdt_reboot_flag = PMU_READ32(WDT_REBOOT_FLAG_READ_ADDR);
	}
	else if(RPC_MCU_WRITE8 == pmu_rpc_no)
	{
		PMU_WRITE8(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset, (unsigned char)(pmu_params->mcu_write_data));
	}
	else if(RPC_MCU_WRITE16 == pmu_rpc_no)
	{
		PMU_WRITE16(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset, (unsigned short)(pmu_params->mcu_write_data));
	}
	else if(RPC_MCU_WRITE32 == pmu_rpc_no)
	{
		PMU_WRITE32(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset, pmu_params->mcu_write_data);
	}
	else if(RPC_MCU_READ8 == pmu_rpc_no)
	{
		pmu_params->mcu_read_data = PMU_READ8(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset);
	}
	else if(RPC_MCU_READ16 == pmu_rpc_no)
	{
		pmu_params->mcu_read_data = PMU_READ16(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset);
	}
	else if(RPC_MCU_READ32 == pmu_rpc_no)
	{
		pmu_params->mcu_read_data = PMU_READ32(ALI_PMU_BASE_ADDR+pmu_params->mcu_rw_offset);
	}
	else
	{
		libc_printf("\n[%s ::: %d], Invalid rpc No. %d!\n", __FUNCTION__, __LINE__, pmu_rpc_no);
	}
}

//This function set data into private memory
//in_data: the address for the data which need set into the private memory
//data_len: the Max length can support 64bytes
/*
static UINT32 store_data(UINT8 *in_data,UINT32 data_len)
{
    UINT32 priv_base_addr=0;
    UINT8 *dest = NULL;
    UINT32 max_len = 64;

    priv_base_addr = *(volatile UINT32 *)(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR);
    if((0 == priv_base_addr) || (NULL == in_data))
    {
        //libc_printf("the private memory not set!\n");
        return 0;
    }
    else
    {
        if(data_len > max_len)
        {
            data_len= 64;
        }
        priv_base_addr |=(0xa<<28);
        //libc_printf("MEMCPY: dest=0x%x,src=0x%x,len=%d\n", (priv_base_addr+0x80),in_data,data_len);
        dest = (UINT8 *)(priv_base_addr+0x80);
        osal_cache_flush((void*)(in_data), data_len);
        if(*(unsigned char *)(in_data+data_len-1) != \
         *(volatile unsigned char*)((UINT32)(in_data+data_len-1)|0xa0000000))
        {
            //libc_printf("len=%d\n", data_len);
            //make sure data is flushed into cache before send to SEE
            ASSERT(0);
        }
        //libc_printf("hld_dev_memcpy:dest=0x%x,src=0x%x,len=%d\n",
        //dest, ((UINT32)(in_data)&0xfffffff)|0xa0000000, data_len);
          hld_dev_memcpy((void *)dest, (void *)(((UINT32)in_data&0x0fffffff)|0xa0000000), data_len);

        return data_len;
    }
}
*/
//This function get some data from private memory
//out_data: the address for the data which need get from the private memory
//data_len: the Max length can support 64bytes
/*
static UINT32 get_data(UINT8 *out_data,UINT32 data_len)
{
    UINT32 max_len = 64;

    if(NULL != out_data)
    {
        copy_from_priv_mem(out_data,data_len);
        if(data_len > max_len)
        {
            data_len= 64;
        }
        return data_len;
    }
    return RET_FAILURE;
}
*/
#ifdef SEE_CPU
UINT32 desc_vpo_hdmi_cb[] =
{ //desc of pointer para
//  1, DESC_OUTPUT_STRU(0, sizeof(struct de2Hdmi_video_infor)),
  1, DESC_STATIC_STRU(0, sizeof(struct de2hdmi_video_infor)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_snd_hdmi_cb[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct snd2hdmi_audio_infor)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_vdec_cb[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct user_data_pram)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_snd_moniter_cb[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_io_reg_callback_para)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_vdec_info_cb[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vdec_info_cb_param)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};
UINT32 desc_dmx_cb[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct dmx_data_see2main_param)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_subt_cb[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct subtitle_cb_info_s)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_ts_see2main_subt_cb[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct ts_see2main_data_cb_new_s)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

#if 0
void hld_cpu_callback(UINT32 u_param)
{
    jump_to_func(NULL, os_hld_caller, u_param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VDEC_CB, NULL);
}

void hld_cpu_vpo_callback(UINT32 u_param)
{
    jump_to_func(NULL, os_hld_caller, u_param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VPO_CB, NULL);
}

void hld_cpu_hdmi_callback(UINT32 u_param)
{
    jump_to_func(NULL, os_hld_caller, u_param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VPO_HDMI_CB, desc_vpo_hdmi_cb);
}

void hld_cpu_snd_callback(UINT32 u_param)
{
    jump_to_func(NULL, os_hld_caller, u_param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SND_HDMI_CB, desc_snd_hdmi_cb);
}

#else

void hld_cpu_os_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VDEC_CB, NULL);
}

void hld_cpu_os_vpo_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VPO_CB, NULL);
}

void hld_cpu_os_hdmi_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VPO_HDMI_CB, desc_vpo_hdmi_cb);
}

void hld_cpu_os_vdec_spec_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VDEC_SPEC_CB, desc_vdec_cb);
}

void hld_cpu_os_vdec_info_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_VDEC_INFO_CB, desc_vdec_info_cb);
}

void hld_cpu_os_dmx_spec_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_DMX_SPEC_CB, desc_dmx_cb);
}
void hld_cpu_os_snd_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SND_HDMI_CB, desc_snd_hdmi_cb);
}

void hld_cpu_os_subt_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SUBTITLE_CB, desc_subt_cb);
}


INT8 hld_cpu_os_ts_see2main_subt_callback1(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_TS_DATA_CB, desc_ts_see2main_subt_cb);
}

void hld_cpu_os_ts_see2main_subt_callback(UINT32 uparam)
{
    ts_see2main_ret = hld_cpu_os_ts_see2main_subt_callback1(uparam);
}


void hld_cpu_os_snd_spec_callback(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SND_SPEC_CB, NULL);
}

void hld_cpu_img_cb(__attribute__((unused))unsigned long type, __attribute__((unused))unsigned long param)
{
    jump_to_func(NULL, os_hld_caller, type, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_IMG_CB, NULL);
}

void hld_cpu_vde_cb(__attribute__((unused))unsigned long type, __attribute__((unused))unsigned long param)
{
    jump_to_func(NULL, os_hld_caller, type, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_VDE_CB, NULL);
}

void hld_cpu_mus_cb(__attribute__((unused))unsigned long type, __attribute__((unused))unsigned long param)
{
    jump_to_func(NULL, os_hld_caller, type, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_MUS_CB, NULL);
}

void hld_cpu_os_deca_cb(__attribute__((unused))UINT32 uparam)
{
    UINT32 desc_deca_moniter_cb[] =
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct deca_io_reg_callback_para)),
      1, DESC_P_PARA(0, 0, 0),
      //desc of pointer ret
      0,
      0,
    };

    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_DECA_CB, desc_deca_moniter_cb);
}

void hld_cpu_os_snd_moniter_cb(__attribute__((unused))UINT32 uparam)
{
    jump_to_func(NULL, os_hld_caller, uparam, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SND_MONITER_CB, desc_snd_moniter_cb);
}

typedef void (*run_loop_request_entry)(void *param);

typedef struct run_loop {
	ID sem;
	ID mutex;
	ID thread;
	char name[3];

	run_loop_request_entry request_entry;
	void *param;
} run_loop_t;

static void run_loop_thread_entry(DWORD param1, __attribute__((unused))DWORD param2)
{
	run_loop_t *run_loop = (run_loop_t *)param1;
	run_loop_request_entry request_entry;
	void *param;
	
	while (1) {
		osal_semaphore_capture(run_loop->sem, OSAL_WAIT_FOREVER_TIME);

		osal_mutex_lock(run_loop->mutex, OSAL_WAIT_FOREVER_TIME);
		request_entry = run_loop->request_entry;
		param = run_loop->param;
		osal_mutex_unlock(run_loop->mutex);

		request_entry(param);
	}
}

static int run_loop_thread_create(run_loop_t *run_loop)
{
	OSAL_T_CTSK t_ctsk;

	memset(&t_ctsk, 0x0, sizeof (t_ctsk));
	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 5;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	memcpy(t_ctsk.name, run_loop->name, 3);
	t_ctsk.para1 = (UINT32)run_loop;
	t_ctsk.task = run_loop_thread_entry;
	
	run_loop->thread = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == run_loop->thread) {
		return -1;
	}

	return 0;
}

run_loop_t *run_loop_create(const char *name)
{
	run_loop_t *run_loop;
	
	run_loop = (run_loop_t *)malloc(sizeof (*run_loop));
	if (NULL == run_loop) {
		goto fail_to_alloc_ctx;
	}

	run_loop->sem = osal_semaphore_create(1);
	if (OSAL_INVALID_ID == run_loop->sem) {
		goto fail_to_create_sem;
	}
	osal_semaphore_capture(run_loop->sem, OSAL_WAIT_FOREVER_TIME);

	run_loop->mutex = osal_mutex_create();
	if (OSAL_INVALID_ID == run_loop->mutex) {
		goto fail_to_create_mutex;
	}
	
	memcpy(run_loop->name, name, 3);
	if (run_loop_thread_create(run_loop)) {
		goto fail_to_create_thread;
	}

	return run_loop;

fail_to_create_thread:
	libc_printf("Fail to create run loop thread\n");
	osal_mutex_delete(run_loop->mutex);
fail_to_create_mutex:
	libc_printf("Fail to create run loop mutex\n");
	osal_semaphore_delete(run_loop->sem);
fail_to_create_sem:
	libc_printf("Fail to create run loop sem\n");
	free(run_loop);
fail_to_alloc_ctx:
	libc_printf("Fail to create run loop ctx\n");
	return NULL;
}

int run_loop_request_post(run_loop_t *run_loop, run_loop_request_entry request_entry, void *param)
{
	if (NULL == run_loop) {
		return -1;
	}

	osal_mutex_lock(run_loop->mutex, OSAL_WAIT_FOREVER_TIME);
	run_loop->request_entry = request_entry;
	run_loop->param = param;
	osal_mutex_unlock(run_loop->mutex);
	
	osal_semaphore_release(run_loop->sem);
	return 0;
}

static void snd_mute_do_rpc(__attribute__((unused))UINT32 gpio_num, __attribute__((unused))UINT32 flag, __attribute__((unused))int is_by_ext_gpio)
{
	jump_to_func(NULL, os_hld_caller, gpio_num, (HLD_BASE_MODULE<<24)|(3<<16)|FUNC_HLD_SND_MUTE_BY_GPIO, NULL);
}

static void snd_mute_do_rpc_in_run_loop(void *param)
{
	UINT32 gpio_num = (UINT32)param;
	UINT32 flag = (UINT32)param;
	int is_by_ext_gpio = (int)param;
	snd_mute_do_rpc(gpio_num & 0x3FFFFFFF, (flag >> 31) & 0x1, (is_by_ext_gpio >> 30) & 0x1);
}

static run_loop_t *s_run_loop = NULL;

void hld_cpu_os_snd_mute_by_gpio(UINT32 gpio_num, UINT32 flag, int is_by_ext_gpio)
{
	if (g_hsr_thread == osal_task_get_current_id()) {
		if (NULL == s_run_loop) {
			s_run_loop = run_loop_create("mut");
			if (NULL == s_run_loop) {
				return;
			}
		}
		run_loop_request_post(s_run_loop, snd_mute_do_rpc_in_run_loop, \
				(void *)((gpio_num & 0x3FFFFFFF) | ((flag & 0x1)<< 31) | ((is_by_ext_gpio & 0x1) << 30)));
	} else {
    	snd_mute_do_rpc(gpio_num, flag, is_by_ext_gpio);
	}
}
void hld_cpu_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_callback, uparam);
}

void hld_cpu_vpo_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_vpo_callback, uparam);
}

void hld_cpu_hdmi_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_hdmi_callback, uparam);
}

void hld_cpu_vdec_spec_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_vdec_spec_callback, uparam);
}

void hld_cpu_vdec_info_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_vdec_info_callback, uparam);
}

void hld_cpu_dmx_spec_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_dmx_spec_callback, uparam);
}

void hld_cpu_subt_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_subt_callback, uparam);
}

void hld_cpu_ts_see2main_subt_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_ts_see2main_subt_callback, uparam);
}

void hld_cpu_snd_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_snd_callback, uparam);
}

void hld_cpu_snd_spec_callback(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_snd_spec_callback, uparam);
}

void hld_cpu_deca_cb(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_deca_cb, uparam);
}

void hld_cpu_snd_moniter_cb(UINT32 uparam)
{
    os_remote_cb_send_msg(hld_cpu_os_snd_moniter_cb, uparam);
}

void hld_cpu_cache_flush(__attribute__((unused))UINT32 in, __attribute__((unused))UINT32 data_length)
{
    jump_to_func(NULL, os_hld_caller, in, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_CACHE_FLUSH, NULL);
}

void hld_cpu_cache_inv(__attribute__((unused))UINT32 in, __attribute__((unused))UINT32 data_length)
{
    jump_to_func(NULL, os_hld_caller, in, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_CACHE_INV, NULL);
}

UINT32 desc_see_printf_to_main_with_rpc_msg[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct see_printf_to_main_with_rpc_msg)),
  1, DESC_P_PARA(0, 0, 0),
  0,
  0,
};
void see_printf_to_main_with_rpc(__attribute__((unused))struct see_printf_to_main_with_rpc_msg *msg)
{
    jump_to_func(NULL, os_hld_caller, msg, ((HLD_BASE_MODULE<<24)|(1<<16)) | FUNC_HLD_DEV_SEE_PRINTF_TO_MAIN_WITH_RPC, desc_see_printf_to_main_with_rpc_msg);
}

#endif

#endif



#endif

