/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 hardware data capture function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#ifndef __LLD_NIM_DVBS_C3505_DATA_CAP_H__
#define __LLD_NIM_DVBS_C3505_DATA_CAP_H__

//---------------- capture Function --------------//
// added by Gloria.Zhou for TS GS auto cap 2015.12.1
INT32 nim_c3505_tso_bbfrm_cap_mod(struct nim_device *dev);
INT32 nim_c3505_rec_stop(struct nim_device *dev);
INT32 nim_c3505_ts_gs_auto_cap_mod(struct nim_device *dev);
INT32 nim_c3505_tsbb_cap_set(struct nim_device *dev, UINT8 s_case,UINT32 swpidf,UINT32 swpidd);
INT32 nim_c3505_dma_cap_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3505_cap_cfg(struct nim_device *dev,UINT32 dram_len);
INT32 nim_c3505_la_cfg(struct nim_device *dev);
INT32 nim_c3505_normal_tso_set(struct nim_device *dev);
INT32 nim_c3505_adc2mem_entity(struct nim_device *dev,  UINT8 *cap_buffer, UINT32 dram_len,UINT8 cap_src);
void nim_c3505_cap_calculate_energy(struct nim_device *dev);
INT32 nim_c3505_adc2mem_calculate_energy(struct nim_device *dev,UINT8 *cap_buffer,UINT32 dram_len);
void nim_c3505_cap_fft_result_read(struct nim_device *dev);
INT32 nim_c3505_cap_iq_enerage(struct nim_device *dev);
INT32 nim_c3505_cap(struct nim_device *dev, UINT32 startFreq, INT32 *cap_buffer, UINT32 sym);
INT32 nim_c3505_wide_band_scan_open(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq,UINT32 step_freq);
INT32 nim_c3505_wide_band_scan_close(void);
INT32 nim_c3505_wide_band_scan(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq);
INT32 nim_c3505_cap_fre(struct nim_device *dev);

#endif	// __LLD_NIM_DVBS_C3505_DATA_CAP_H__ */
