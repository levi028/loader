/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 tp scan function such
*		as auto_scan channel_change
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#ifndef __LLD_NIM_DVBS_C3505_SCAN_TP_H__
#define __LLD_NIM_DVBS_C3505_SCAN_TP_H__

extern UINT16 config_data;
extern INT32 fft_energy_1024[1024];
extern INT32 fft_energy_1024_tmp[1024];
extern INT32 frequency_est[TP_MAXNUM];
extern INT32 symbol_rate_est[TP_MAXNUM];
extern INT32 tp_number;
extern INT32 *channel_spectrum;
extern INT32 *channel_spectrum_tmp;
extern INT32 last_tuner_if;
extern INT32 chlspec_num;
extern INT32 called_num;
extern INT32 final_est_freq;
extern INT32 final_est_sym_rate;
extern INT32 max_fft_energy;

#define LOOP_PERIOD 25
#define USER_STOP -44

#define COMPEN_MULTISWITCH_DELAY 800    // For some multiswitch,  it need more time to make signal steady. Unit is ms

struct nim_c3505_frequency_seg_params
{
    UINT32 seg_spectrum_start;    // segment spectrum start freq
    UINT32 seg_spectrum_end;      // segment spectrum end freq

    UINT32 fft_energy_seg_sum;    // sum of segment spectrum fft energy 
    UINT32 fft_energy_seg_avg;    // average of segment spectrum fft energy 
    UINT32 fft_energy_seg_max;    // max of segment spectrum fft energy 

    UINT32 no_tp_flag;            // 0 = there is no tp in tihs frequency rang 
};

struct nim_c3505_wideband_params
{
    UINT32 fft_energy_sum;    // sum of segment spectrum fft energy 
    UINT32 fft_energy_avg;    // average of segment spectrum fft energy 
    UINT32 fft_energy_max;    // max of segment spectrum fft energy 
};

struct nim_c3505_autoscan_params
{
    struct nim_c3505_private *priv;
    struct NIM_AUTO_SCAN *pst_auto_scan;
    UINT32 start_t;
    UINT32 temp_t;
    UINT32 end_t;
    INT32 adc_sample_freq;
    INT32 success;
    UINT32 fft_freq;
    UINT8 reg0a_data_bak;                                   // used to restore register 0a value 
    struct nim_c3505_wideband_params wideband_para;         // used to filter false TP for reduce time of autoscan
    struct nim_c3505_frequency_seg_params head_seg_para;    // used to filter false TP for reduce time of autoscan
    struct nim_c3505_frequency_seg_params tail_seg_para;    // used to filter false TP for reduce time of autoscan
    UINT8 freq_band;    // For dvb-s/s2, 0 = default, 1 = C band, 2 = Ku band
};


//------------------Scan Function -----------------------//
INT32 nim_c3505_channel_change(struct nim_device *dev, struct NIM_CHANNEL_CHANGE * pst_channel_change);
INT32 nim_c3505_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 change_type, struct nim_dvbs_isid *p_isid);
INT32 nim_c3505_autoscan_find_acm_tp(struct nim_device *dev, UINT32 lock_tp_idx, UINT32 cur_freq, UINT32 cur_sym);
INT32 nim_c3505_autoscan_init(struct nim_device *dev, struct nim_c3505_autoscan_params *params);
INT32 nim_c3505_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3505_autoscan_params *params);
INT32 nim_c3505_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params);
INT32 nim_c3505_autoscan_try_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params);
INT32 nim_c3505_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan);
INT32 nim_c3505_adc2mem_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 dram_len);
INT32 nim_c3505_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, INT32 *cap_buffer);
INT32 nim_c3505_channel_search(struct nim_device *dev, UINT32 cr_num);
INT32 nim_c3505_soft_search_init( struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param);
INT32 nim_c3505_soft_search(struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param);
INT32 nim_c3505_fft(struct nim_device *dev, UINT32 startFreq);
INT32 nim_c3505_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq);
INT32 nim_c3505_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr);

//Nim Autosearch,R2FFT 
extern INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *SymbolRate_est, INT32 *m_IF_freq, INT32 *m_ch_number);
extern void nim_s3501_smoothfilter(void);
extern void nim_s3501_median_filter(INT32 FLength, INT32 *Fdata, INT32 Scan_mode);
extern INT32 nim_s3501_search_tp(INT32 chlspec_num, INT32 *channel_spectrum,UINT32 sfreq,UINT32 adc_sample_freq,INT32 loop);

extern INT32 nim_s3501_fft_wideband_scan(struct nim_device *dev,UINT32 tuner_if, UINT32 adc_sample_freq);
//extern INT32 nim_s3501_fft_wideband_scan_m3031(struct nim_device *dev, UINT32 tuner_if, UINT32 adc_sample_freq);

extern void R2FFT(INT32 *FFT_I_1024, INT32 *FFT_Q_1024);
//demux signal pass
extern INT32 nim_s3501_autoscan_signal_input(struct nim_device *dev, UINT8 s_Case);
extern plsn_state nim_c3505_try_plsn(struct nim_device *dev, UINT8 *index);
extern INT32 nim_c3505_get_search_timeout(struct nim_device *dev, UINT32 *single_timeout, UINT32 *total_timeout);

#endif	// __LLD_NIM_DVBS_C3505_SCAN_TP_H__ */
