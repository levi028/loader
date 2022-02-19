/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_sharp6158.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  4.27.2006        Joey Gao        Ver 0.1       Create file.
*
*****************************************************************************/

#ifndef __LLD_NIM_SHARP6158_H__
#define __LLD_NIM_SHARP6158_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

#include "MN_DMD_device.h"

#if 0
typedef struct _SHARP6158_LOCK_INFO
{
    UINT8   FECRates;
    UINT8   Modulation;
    UINT8   Mode;
    UINT8   Guard;
    UINT8   status;
    UINT8   Hierarchy;
    UINT8   Spectrum;
    UINT8   ChannelBW;
    UINT32  Frequency;
    INT32   FreqOffset;
}SHARP6158_LOCK_INFO, *PSHARP6158_LOCK_INFO;

typedef struct
{
    struct COFDM_TUNER_CONFIG_API tc;
    DMD_PARAMETER_t* param;

    UINT32 tuner_id;

    // internal variables
    SHARP6158_LOCK_INFO lock_info;

    OSAL_ID flag_id;
    OSAL_ID i2c_mutex_id;

#if 0
    UINT8 i2c_reg_page;

    UINT32 autoscan_stop_flag           :1;
    UINT32 reserved                     :31;

    UINT32 snr_ber;
    UINT32 snr_per;
    UINT32 per_tot_cnt;

    UINT16 tuner_if_freq;

    // for RSSI display
    nim_ad_gain_table_t rf_ad_gain_table;
    nim_ad_gain_table_t if_ad_gain_table;

    UINT16 rec_agc; //when flag can't be wait, return the latest valid value.
    UINT8 rec_snr;
    UINT32 rec_ber_cnt;

    UINT8 work_status;
    UINT8 tps_cnt;
#endif

}NIM_SHARP6158_PRIVATE, *PNIM_SHARP6158_PRIVATE;
#endif

static INT32 nim_sharp6158_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);

INT32 nim_sharp6158_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig);

#endif  /* __LLD_NIM_SHARP6158_H__ */

