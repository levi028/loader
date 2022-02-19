/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 get hardware status function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/


#ifndef __LLD_NIM_DVBS_C3505_MONITOR_H__
#define __LLD_NIM_DVBS_C3505_MONITOR_H__

void nim_c3505_get_fast_lock(struct nim_device *dev, UINT8 *lock);
//---------------Demodulator monitor Function --------------//
INT32 nim_c3505_reg_get_chip_type(struct nim_device *dev);
INT32 nim_c3505_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_c3505_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock);
INT32 nim_c3505_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag);
INT32 nim_c3505_get_demod_gain(struct nim_device *dev, UINT8 *agc);
INT32 nim_c3505_get_tuner_gain(struct nim_device *dev,UINT8 agc, INT32 *agc_tuner);
INT32 nim_c3505_get_dbm_level(struct nim_device *dev,UINT8 agc_demod, INT32 agc_tuner,INT8 *level_db);
INT32 nim_c3505_get_agc_filter(struct nim_device *dev, INT8 *agc);
INT32 nim_c3505_get_agc(struct nim_device *dev, UINT8 *agc);
INT32 nim_c3505_get_agc_dbm(struct nim_device *dev, INT8 *agc_dbm);
INT32 nim_c3505_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_c3505_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_c3505_get_tune_freq(struct nim_device *dev, INT32 *freq);
INT32 nim_c3505_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode);
INT32 nim_c3505_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate);
INT32 nim_c3505_reg_get_map_type(struct nim_device *dev, UINT8 *map_type);
INT32 nim_c3505_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off);
INT32 nim_c3505_reg_get_modcod(struct nim_device *dev, UINT8 *modcod);
INT32 nim_c3505_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 rs, UINT32 *bit_rate);
INT32 nim_c3505_get_bitmode(struct nim_device *dev, UINT8 *bitMode);
INT32 nim_c3505_get_ber(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3505_get_new_ber(struct nim_device *dev, UINT32 *ber);
INT32 nim_c3505_get_per(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3505_get_fer(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3505_get_new_per(struct nim_device *dev, UINT32 *per);
INT32 nim_c3505_get_phase_error(struct nim_device *dev, INT32 *phase_error);
INT32 nim_c3505_get_mer(struct nim_device *dev, UINT32 *mer);
INT32 nim_c3505_get_mer_task(struct nim_device *dev);
INT32 nim_c3505_get_snr(struct nim_device *dev, UINT8 *snr);
INT32 nim_c3505_get_snr_db(struct nim_device *dev, INT16 *snr_db);
INT32 nim_c3505_get_snr_db_new(struct nim_device *dev, INT16 *snr_db);
INT32 nim_c3505_check_ber(struct nim_device *dev, UINT32 *rs_ubc);
UINT8 nim_c3505_get_snr_index(struct nim_device *dev);
INT32 nim_c3505_get_cur_freq(struct nim_device *dev, UINT32 *freq);
UINT8 nim_c3505_get_cr_num(struct nim_device *dev);
INT32 nim_c3505_get_ldpc(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3505_get_ldpc_iter_cnt(struct nim_device *dev, UINT16 *iter_cnt);
INT32 nim_c3505_cr_sw_snr_rpt(struct nim_device *dev);
UINT8 nim_c3505_map_beta_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 *demap_beta_active, UINT8 *demap_beta);
UINT16 nim_c3505_demap_noise_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT16 *demap_noise);
INT32 nim_c3505_reg_get_freqoffset(struct nim_device *dev);
INT32 nim_c3505_get_symbol(struct nim_device *dev, struct nim_get_symbol *p_symbol);
INT32 nim_c3505_get_bb_header_info(struct nim_device *dev);
INT32 nim_c3505_get_channel_info(struct nim_device *dev);
INT32 nim_c3505_get_tp_info(struct nim_device *dev);
INT32 nim_c3505_printf_tp_info(struct nim_device *dev);
INT32 nim_c3505_show_acm_ccm_info(struct nim_device *dev);

#endif	// __LLD_NIM_DVBS_C3505_MONITOR_H__ */
