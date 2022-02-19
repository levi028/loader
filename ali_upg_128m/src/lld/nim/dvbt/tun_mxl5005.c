/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL5005.h
*
*    Description:    Source file of MAXLINEAR MxL5005 TUNER.
*    History:
*        Date          Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  8.1.2007            PengHui          Ver 0.1         Create file.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_tuner.h>

#include "tun_mxl5005.h"

#if ((SYS_TUN_MODULE == MXL5005) || (SYS_TUN_MODULE == ANY_TUNER))
#define NIM_PRINTF(...)

#undef BURST_PAIR_SZ
#define BURST_PAIR_SZ   7

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;
static Tuner_struct MXL5005_Tuner[MAX_TUNER_SUPPORT_NUM];

/*****************************************************************************
* INT32 tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner mxl5005 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl5005_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    //*tuner_id = tuner_cnt;
    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl5005_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_mxl5005_status(UINT32 tuner_id, UINT8 *lock)
{
    *lock = 0x01;
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl5005_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth        : channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const    : AGC time constant
*  Parameter4: UINT8 *data        :
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_mxl5005_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    int ret;
    *data = 0;

    unsigned long dwRF_Freq_KHz;
    unsigned char bBandWidthMHz;
    UINT32 crystal_kHz;
    UINT16 dwIF_Freq_KHz;
    UINT8 dw_TOP;
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

//joey 20080521. for mxl5003 N-1 ACI issue. add 166k offset at Lo.
    dwRF_Freq_KHz = freq+166;

    bBandWidthMHz = bandwidth;
    crystal_kHz= ptrTuner_Config->c_tuner_crystal;
    if(crystal_kHz < 1000)
        crystal_kHz = crystal_kHz * 1000;

    dwIF_Freq_KHz=ptrTuner_Config->w_tuner_if_freq;
    dw_TOP=ptrTuner_Config->c_tuner_agc_top;

    ret = Set_MXL5005_Channel(&MXL5005_Tuner[tuner_id],crystal_kHz, dwIF_Freq_KHz,dw_TOP, dwRF_Freq_KHz,bBandWidthMHz,tuner_id);


    return ret;
}


int  Write_I2C(UINT8 *Reg_Addr, UINT8 *Reg_Data, int BytesNo,UINT32 tuner_id)
{
    int status = 0;
    int i,j;
    _u8 szBuffer[BURST_PAIR_SZ*2 + 1];

    UINT16 BurstPairs, RemainLen;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config = tuner_dev_cfg[tuner_id];

    if ( !BytesNo ) return 0;

    RemainLen = BytesNo % BURST_PAIR_SZ;
    if ( RemainLen )
    {
        BurstPairs = BytesNo / BURST_PAIR_SZ;
    }
    else
    {
        BurstPairs = BytesNo / BURST_PAIR_SZ - 1;
        RemainLen = BURST_PAIR_SZ ;
    }

    for ( i = 0 ; i < BurstPairs ; i ++ )
    {
        for ( j = 0 ; j < BURST_PAIR_SZ ; j ++ )
        {
            szBuffer[j*2]   = Reg_Addr[i * BURST_PAIR_SZ + j];
            szBuffer[j*2+1] = Reg_Data[i * BURST_PAIR_SZ + j];
        }
        status+= ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, szBuffer, BURST_PAIR_SZ*2);
    }

    for ( i = 0 ; i < RemainLen ; i ++ )
    {
        szBuffer[i*2]   = Reg_Addr[BurstPairs * BURST_PAIR_SZ + i];
        szBuffer[i*2+1] = Reg_Data[BurstPairs * BURST_PAIR_SZ + i];
    }
    szBuffer[i*2] = 254;

    status += ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, szBuffer, RemainLen*2+1);

    return status;

}

#endif
