#ifndef _NIM_CXD2856_MONITOR_H_
#define _NIM_CXD2856_MONITOR_H_
INT32 nim_cxd2856_get_lock(struct nim_device *dev, UINT8 *lock);

INT32 nim_cxd2856_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);

INT32 nim_cxd2856_get_freq(struct nim_device *dev, UINT32 *freq);

INT32 nim_cxd2856_get_modulation(struct nim_device *dev, UINT8 *modulation);

INT32 nim_cxd2856_get_SSI(struct nim_device *dev, UINT8 *ssi);

INT32 nim_cxd2856_get_SQI(struct nim_device *dev, UINT8 *sqi);

INT32 nim_cxd2856_get_per(struct nim_device *dev, UINT32 *per);

INT32 nim_cxd2856_get_ber(struct nim_device *dev, UINT32 *ber);

INT32 nim_cxd2856_get_cn(struct nim_device *dev, UINT16 *cn);

INT32 nim_cxd2856_get_rf_level(struct nim_device *dev, UINT16 *rf_level);
#endif
