/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_nim.h
*
* Description:
*     This file contains all NIM library define.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef    __LIB_NIM_H__
#define __LIB_NIM_H__

#include <types.h>
#include <retcode.h>
#include <sys_config.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 api_nim_register(void);
INT32 api_nim_unregister(void);
UINT8 api_nim_get_lock(void);
UINT32 api_nim_get_freq(void);
UINT8 api_nim_get_snr(void);
UINT8 api_nim_get_fec(void);
void api_nim_stop_autoscan(void);
void api_nim_reset_autoscan(void);

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)

    INT32 api_nim_get_fft_result(UINT32 freq, UINT32* start_adr);
    INT32 api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar,
            UINT32 freq, UINT32 sym, UINT8 fec));
    INT32 api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec);
    UINT32 api_nim_get_ber(void);
    UINT32 api_nim_get_sym(void);
    UINT8 api_nim_get_agc(void);

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ATSC)||(SYS_PROJECT_FE == PROJECT_FE_ISDBT)

INT32 api_nim_disable();//Sam_chen 20050616
INT32 api_nim_channel_change(UINT32 freq, UINT32 bandwidth, UINT8 guard_interval,
        UINT8 fft_mode,UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);
INT32 api_nim_channel_search(UINT32 freq,UINT32 bandwidth,UINT8 guard_interval,
        UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type,
        UINT8 inverse,UINT16 freq_offset, UINT8 priority);
UINT8 api_nim_get_fftmode(void);
UINT8 api_nim_get_gi(void);
UINT8 api_nim_get_modulation(void);
UINT8 api_nim_get_freqinv(void);
UINT16 api_nim_get_agc(void);
INT8 api_nim_get_priority(void);
INT8 api_nim_hier_mode(void);

#ifdef SMART_ANT_SUPPORT
typedef void (*sm_status_update)(UINT16 value);

INT8 api_nim_set_smartenna(UINT8 position,UINT8 gain,    UINT8 pol,UINT8 channel);
INT8 api_nim_get_smartenna_setting(UINT8 *p_position,UINT8 *p_gain,UINT8 *p_pol,UINT8 *p_channel);
INT8 api_nim_get_smartenna_metric(UINT8 metric,UINT16 *p_metric);
INT32  api_nim_auto_seek_smartenna(UINT32 freq, UINT16 *antval,UINT8 *detect, sm_status_update status_update );

UINT16 api_nim_get_vsb_agc(void);
UINT16 api_nim_get_vsb_snr(void);
UINT32 api_nim_get_vsb_per(void);
#endif


#if (SYS_SDRAM_SIZE == 8 || GET_BER == SYS_FUNC_ON )
UINT32 api_nim_get_ber(void);
#endif

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar,
    UINT32 freq, UINT32 sym, UINT8 fec));
#if 0
INT32 api_nim_autoscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar,
    UINT32 freq, UINT32 sym, UINT8 fec));
#endif
INT32 api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec);
UINT32 api_nim_get_ber(void);
UINT32 api_nim_get_sym(void);
UINT32 api_nim_get_freq(void);
UINT8 api_nim_get_agc(void);
#endif




#ifdef __cplusplus
}
#endif

#endif  /* __LIB_NIM_H__ */
