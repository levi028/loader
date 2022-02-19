/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 hardware operate function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/


#ifndef __LLD_NIM_DVBS_C3505_HW_CFG_H__
#define __LLD_NIM_DVBS_C3505_HW_CFG_H__


#define NIM_GET_DWORD(i)	(*(volatile UINT32 *)(i))
#define NIM_SET_DWORD(i,d)  (*(volatile UINT32 *)(i)) = (d)

#define NIM_GET_WORD(i) 	(*(volatile UINT16 *)(i))
#define NIM_SET_WORD(i,d)   (*(volatile UINT16 *)(i)) = (d)

#define NIM_GET_BYTE(i) 	(*(volatile UINT8 *)(i))
#define NIM_SET_BYTE(i,d)   (*(volatile UINT8 *)(i)) = (d)

INT32 nim_c3505_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_c3505_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);

INT32 nim_c3505_set_err(struct nim_device *dev);
INT32 nim_c3505_get_err(struct nim_device *dev);
INT32 nim_c3505_clear_err(struct nim_device *dev);

INT32 nim_c3505_sym_config(struct nim_device *dev, UINT32 sym);
INT32 nim_c3505_interrupt_mask_clean(struct nim_device *dev);
//---------------Demodulator configuration Function --------------//
INT32 nim_c3505_interrupt_clear(struct nim_device *dev);
INT32 nim_c3505_set_demod_ctrl(struct nim_device *dev, UINT8 c_Value);
INT32 nim_c3505_set_hw_timeout(struct nim_device *dev, UINT16 time_thr);
INT32 nim_c3505_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner);
INT32 nim_c3505_after_reset_set_param(struct nim_device *dev);
INT32 nim_c3505_set_adc(struct nim_device *dev);
INT32 nim_c3505_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel);
INT32 nim_c3505_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate);
INT32 nim_c3505_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_Case, UINT8 frz_agc1);
INT32 nim_c3505_set_rs(struct nim_device *dev, UINT32 rs);
INT32 nim_c3505_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_c3505_set_fc_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs);
INT32 nim_c3505_set_rs_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs);
INT32 nim_c3505_cfg_tuner_get_freq_err(struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param);
INT32 nim_c3505_set_freq_offset(struct nim_device *dev, INT32 delfreq);
//INT32 nim_c3505_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_Freq);
//INT32 nim_c3505_freq_offset_reset(struct nim_device *dev, UINT8 low_sym);
//INT32 nim_c3505_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq);
INT32 nim_c3505_cr_setting(struct nim_device *dev, UINT8 s_Case);
INT32 nim_c3505_tr_cr_setting(struct nim_device *dev, UINT8 s_Case);
INT32 nim_c3505_cr_adaptive_initial (struct nim_device *dev);
INT32 nim_c3505_cr_adaptive_configure (struct nim_device *dev, UINT32 sym);
INT32 nim_c3505_cr_tab_init(struct nim_device *dev);
INT32 nim_c3505_cr_new_tab_init(struct nim_device *dev);
INT32 nim_c3505_cr_new_adaptive_unlock_monitor(struct nim_device *dev);
INT32 nim_c3505_cr_new_modcod_table_init(struct nim_device *dev,UINT32 sym);
INT32 nim_c3505_cr_adaptive_method_choice(struct nim_device *dev, UINT8 choice_type);
INT32 nim_c3505_nframe_step_tso_setting(struct nim_device *dev,UINT32 sym_rate,UINT8 s_Case);
INT32 nim_c3505_fec_set_ldpc(struct nim_device *dev, UINT8 s_Case, UINT8 c_ldpc, UINT8 c_fec);
INT32 nim_c3505_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode);
INT32 nim_c3505_tso_off (struct nim_device *dev);
INT32 nim_c3505_tso_on (struct nim_device *dev);
INT32 nim_c3505_set_tso_clk (struct nim_device *dev, struct nim_tso_cfg *tso_cfg);
INT32 nim_c3505_tso_dummy_off (struct nim_device *dev);
INT32 nim_c3505_tso_dummy_on (struct nim_device *dev);
INT32 nim_c3505_diseqc_initial(struct nim_device *dev);
INT32 nim_c3505_diseqc_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt);
INT32 nim_c3505_diseqc2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt);
INT32 nim_c3505_fec_set_demap_noise(struct nim_device *dev);
INT32 nim_c3505_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag);
INT32 nim_c3505_module_demap_flt_ini(struct nim_device *dev);
INT32 nim_c3505_set_12v(struct nim_device *dev, UINT8 flag);
INT32 nim_c3505_fft_set_para(struct nim_device *dev);
INT32 nim_c3505_get_bypass_buffer(struct nim_device *dev);
INT32 nim_c3505_module_demap_flt_set(struct nim_device *dev, UINT8 flt_en, UINT8 flt_modu_type, UINT8 flt_frame_type, UINT8 flt_pilot, UINT16 flt_code_rate);
INT32 nim_c3505_module_cci_ini(struct nim_device *dev);	
INT32 nim_c3505_module_cci_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3505_module_diseqc_cmd_queue_ini(struct nim_device *dev);
INT32 nim_c3505_module_diseqc_cmd_queue_set(struct nim_device *dev, UINT8 diseqc_que_cmd);
INT32 nim_c3505_set_map_beta(struct nim_device *dev, UINT8 index, UINT8 frame_mode);
INT32 nim_c3505_set_demap_noise(struct nim_device *dev,UINT8 index, UINT8 frame_mode);
INT32 nim_c3505_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode);
INT32 nim_c3505_acm_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3505_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid);
INT32 nim_c3505_set_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid);
INT32 nim_c3505_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid);
INT32 nim_c3505_task_tso_setting(struct nim_device * dev,UINT32 sym_rate,UINT8 s_case);
INT32 nim_c3505_s2_ldpc_est_ber_enable(struct nim_device *dev);
INT32 nim_c3505_s2_ldpc_est_ber_disable(struct nim_device *dev);
INT32 nim_c3505_module_demap_flt_rpt(struct nim_device *dev, UINT16 *demap_flt_status);
INT32 nim_c3505_module_cci_rpt(struct nim_device *dev, UINT16 *cci_rpt_freq, UINT8 *cci_rpt_level, UINT8 *cci_rpt_lock, UINT8 s_Case);
INT32 nim_c3505_module_diseqc_cmd_queue_rpt(struct nim_device *dev);
INT32 nim_c3505_hw_check(struct nim_device *dev);
INT32 nim_c3505_hw_init(struct nim_device *dev);
INT32 nim_c3505_set_plsn(struct nim_device *dev);
UINT32 nim_c3505_get_plsn(struct nim_device *dev);
INT32 nim_c3505_close_ppll(struct nim_device *dev);
INT32 nim_c3505_open_ppll(struct nim_device *dev);
INT32 nim_c3505_adapt_ppll_mode(struct nim_device *dev);
INT32 nim_c3505_set_work_mode(struct nim_device *dev, UINT8 work_mode);
INT32 nim_c3505_set_map_beta_extra(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 state);
INT32 nim_c3505_auto_adaptive(struct nim_device *dev);
INT32 nim_c3505_mon_fake_lock(struct nim_device *dev);
INT32 nim_c3505_lock_unlock_adaptive(struct nim_device *dev);
INT32 nim_c3505_askey_patch_in_set_tp(struct nim_device *dev, UINT8 enable, struct nim_c3505_tp_scan_para *tp_scan_param);
INT32 nim_c3505_set_cr_prs_table(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 state);


#endif	// __LLD_NIM_DVBS_C3505_HW_CFG_H__ */
