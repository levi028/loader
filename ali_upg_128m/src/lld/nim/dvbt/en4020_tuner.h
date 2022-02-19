/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrTuner.h
 *
 * @brief Prototypes and definitions needed by entrTuner.c and API's.
 *
 *   $xRev: 11840 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRTUNER_H__
#define __ENTRTUNER_H__

/**********************************************************************************************************************/
#ifndef ENTR_uint8
    #define ENTR_uint8 unsigned char
#endif
#ifndef ENTR_uint16
    #define ENTR_uint16 unsigned int
#endif
#ifndef ENTR_sint16
    #define ENTR_sint16 signed short
#endif
#ifndef ENTR_sint32
    #define ENTR_sint32 signed long
#endif

/**********************************************************************************************************************/
#define ENTR_headerRev "$Rev: 11943 $"
/**********************************************************************************************************************/

#define ENTR_tunerActive_yes 0
#define ENTR_tunerActive_no 1

#define ENTR_tunerCfg_yes 0
#define ENTR_tunerCfg_no 1

#define ENTR_FchanMin 20000000
#define ENTR_FchanMax 900000000

/**********************************************************************************************************************/

typedef enum {
    ENTR_b2wAddress_C0 = 0xC0,
    ENTR_b2wAddress_C4 = 0xC4,
    ENTR_b2wAddress_C6 = 0xC6
} ENTR_b2wAddress;

/* index for entr_freqFxtalHz[] */
typedef enum {
    ENTR_Fxtal_24MHz
} ENTR_Fxtal;

/* index for entr_freqFcompHz[] */
typedef enum {
    ENTR_Fcomp_1MHz
} ENTR_Fcomp;

typedef enum {
    ENTR_ltaEnaSel_off,
    ENTR_ltaEnaSel_on
} ENTR_ltaEnaSel;

/* index for ifAgcRange[2] (entrCfg.c) */
typedef enum {
    ENTR_broadcastMedium_cable,
    ENTR_broadcastMedium_ota
} ENTR_broadcastMedium;

/* index for rfAttRange[3] (entrCfg.c) */
typedef enum {
    ENTR_rfAttMode_seriesPin_shuntPin,
    ENTR_rfAttMode_seriesPin_internalAtt,
    ENTR_rfAttMode_seriesPin_shuntPin_internalAtt
} ENTR_rfAttMode;

typedef enum {
    ENTR_specInvSel_off,
    ENTR_specInvSel_on
} ENTR_specInvSel;

/* used as index in entr_cfgBw(). do not change order */
typedef enum {
    ENTR_ifBw_6MHz,
    ENTR_ifBw_7MHz,
    ENTR_ifBw_8MHz
} ENTR_ifBw;

typedef enum {
    ENTR_ifOutputSel_1,
    ENTR_ifOutputSel_2
} ENTR_ifOutputSel;

typedef enum {
    ENTR_agcCtrlMode_demod,
    ENTR_agcCtrlMode_internal
} ENTR_agcCtrlMode;

/* used as index for rssiPar[] in entrCfg.c */
typedef enum {
    ENTR_modulation_DVB_C,
    ENTR_modulation_J83_Annex_B,
    ENTR_modulation_DVB_T,
    ENTR_modulation_ATSC,
    ENTR_modulation_ISDB_T,
    ENTR_modulation_DMB_T,
    ENTR_modulation_PAL,
    ENTR_modulation_NTSC,
    ENTR_modulation_SECAM,
    ENTR_modulation_SECAM_L
} ENTR_modulation;

typedef enum {
    ENTR_tunerCnt_single,
    ENTR_tunerCnt_dual
} ENTR_tunerCnt;

/* index for rssiLnaGain in entrCfg.c */
typedef enum {
    ENTR_lnaModeStatus_cable,
    ENTR_lnaModeStatus_ota,
    ENTR_lnaModeStatus_ota_ln
} ENTR_lnaModeStatus;

typedef enum {
    ENTR_ifOutPutLevel_neg3p2_dB,
    ENTR_ifOutPutLevel_neg2p8_dB,
    ENTR_ifOutPutLevel_neg2p3_dB,
    ENTR_ifOutPutLevel_neg2p0_dB,
    ENTR_ifOutPutLevel_neg1p6_dB,
    ENTR_ifOutPutLevel_neg1p0_dB,
    ENTR_ifOutPutLevel_neg0p5_dB,
    ENTR_ifOutPutLevel_0_dB_nominal,
    ENTR_ifOutPutLevel_0p6_dB,
    ENTR_ifOutPutLevel_1p2_dB,
    ENTR_ifOutPutLevel_1p8_dB,
    ENTR_ifOutPutLevel_2p5_dB,
    ENTR_ifOutPutLevel_3p3_dB,
    ENTR_ifOutPutLevel_4p0_dB,
    ENTR_ifOutPutLevel_5p0_dB,
    ENTR_ifOutPutLevel_6p0_dB
} ENTR_ifOutPutLevel;

typedef enum {
    ENTR_ok,
    ENTR_err_rw,
    ENTR_err_not_active,
    ENTR_err_not_implemented,
    ENTR_err_invalid_parameter,
    ENTR_err_unknown,
    ENTR_err_ifcsm_timeout,
    ENTR_err_ifcsm_fail,
    ENTR_err_ifAccTop_setTtPwr,
    ENTR_err_tfcSm_timeout,
    ENTR_err_tfcSm_fail,
    ENTR_err_freqPllSm_timeout,
    ENTR_err_freqPll_unlocked,
    ENTR_err_cfgCalDet_timeout,
    ENTR_err_tfSetCh_fail,
    ENTR_err_unsuported_ic_rev
} ENTR_return;

typedef struct
{
    ENTR_uint8                tunerActive;
    ENTR_uint8                tunerCfg;
    ENTR_sint32                FxtalHz;
    ENTR_sint32                FcompHz;
    ENTR_sint32                FchanHz;
    ENTR_ltaEnaSel            ltaEnaSel;
    ENTR_broadcastMedium    broadcastMedium;
    ENTR_rfAttMode            rfAttMode;
    ENTR_specInvSel            specInvSel;
    ENTR_ifBw                ifBw;
    ENTR_sint32                FifHz;
    ENTR_sint32                FifMaxHz;
    ENTR_sint32                FifN;
    ENTR_sint32                FifD;
    ENTR_ifOutputSel        ifOutputSel;
    ENTR_ifOutPutLevel        ifOutPutLevel;
    ENTR_agcCtrlMode        agcCtrlMode;
    ENTR_modulation            modulation;
    ENTR_sint32                FloHz;
    ENTR_sint32                FspinHz;
    ENTR_sint32                FspinMaxHz;
    ENTR_sint32                FspinN;
    ENTR_sint32                FspinD;
    ENTR_uint8                ifBwWd[4];
    ENTR_sint32                ifAccTop[2];
    ENTR_uint8                tffCwMb[5];
    ENTR_uint8                tffCwLb[5];
    ENTR_uint8                tffCwHb[5];
    ENTR_uint8                tfgCwMb[5];
    ENTR_uint8                tfgCwLb[5];
    ENTR_uint8                tfgCwHb[5];
    ENTR_sint32                calTmpr;
    ENTR_sint32                curTemp;
    ENTR_sint32                freqLbMax;
    ENTR_sint32                freqMbMax;
    ENTR_sint16                tfcTtLut[6];
    ENTR_uint8                tfcTtPwrLb[5];
    ENTR_uint8                tfcTtPwrMb[5];
    ENTR_uint8                tfcTtPwrHb[5];
} entr_data;

/******************************************************************************
* tfcTtLut[6] was moved from entrTfCal.c into entr_data to support testing.
* tfcTtPwrLb[5], tfcTtPwrMb[5], tfcTtPwrHb[5] are new and also support testing.
******************************************************************************/

typedef struct
{
    UINT32 i2c_id;
    ENTR_b2wAddress            b2wAddress;
    ENTR_Fxtal                Fxtal;
    ENTR_Fcomp                Fcomp;
    ENTR_sint32                FchanHz;
    ENTR_sint32                FchanErrHz;
    ENTR_ltaEnaSel            ltaEnaSel;
    ENTR_broadcastMedium    broadcastMedium;
    ENTR_rfAttMode            rfAttMode;
    ENTR_specInvSel            specInvSel;
    ENTR_ifBw                ifBw;
    ENTR_sint32                FifHz;
    ENTR_sint32                FifErrHz;
    ENTR_ifOutputSel        ifOutputSel;
    ENTR_ifOutPutLevel        ifOutPutLevel;
    ENTR_agcCtrlMode        agcCtrlMode;
    ENTR_modulation            modulation;
    ENTR_tunerCnt            tunerCnt;
    ENTR_lnaModeStatus        lnaModeStatus;
    ENTR_sint16                RSSI;
    entr_data                tunerData;
} entr_config;

ENTR_return entr_active(entr_config *tunerCfg);
ENTR_return entr_set_chan(entr_config *tunerCfg);
ENTR_return entr_idle(entr_config *tunerCfg);
ENTR_return entr_rssi(entr_config *tunerCfg);
ENTR_return entr_status(entr_config *tunerCfg);
ENTR_return entr_extract_revstr(char revStr[], ENTR_uint16 *revision);
ENTR_return entr_code_revision(ENTR_uint16 *revision);

//ENTR_return entr_user_2wb_wr(ENTR_uint8 b2wAdx, ENTR_uint8 icReg, ENTR_uint8 *data, ENTR_uint8 byteCnt);
//ENTR_return entr_user_2wb_rd(ENTR_uint8 b2wAdx, ENTR_uint8 icReg, ENTR_uint8 *data, ENTR_uint8 byteCnt);
//ENTR_return entr_user_delay(ENTR_uint8 delayMsec);


#endif
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrCfg.h
 *
 * @brief Prototypes and definitions needed by entrCfg.c
 *
 *   $xRev: 11942 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRCFG_H__
#define __ENTRCFG_H__


#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************************************/

#define ENTR_cfgWrReg1 18
#define ENTR_cfgRegCnt 169

#define ENTR_cfgChFltrReg1 49
#define ENTR_cfgChFltrRegCnt 44

/**********************************************************************************************************************/

#define ENTR_cfgIfAccTopFttHz_Ref 93750

#define ENTR_cfgShPinLimitLo_Cablel 188
#define ENTR_cfgShPinLimitHi_Cablel 2
#define ENTR_cfgShPinLimitLo_Ter 132
#define ENTR_cfgShPinLimitHi_Ter 3

/**********************************************************************************************************************/

ENTR_return entr_cfgIcRev (entr_config *tunerCfg);

ENTR_return entr_cfgIc (entr_config *tunerCfg);

ENTR_return entr_cfgBase (entr_config *tunerCfg); /* base cfg for normal operation */
ENTR_return entr_cfgIdle (entr_config *tunerCfg);
ENTR_return entr_cfgIfBwCal (entr_config *tunerCfg);
ENTR_return entr_cfgIfAccTopCal (entr_config *tunerCfg);
ENTR_return entr_cfgTfCal (entr_config *tunerCfg);

/* cfg ic multi-mode settings */
ENTR_return entr_cfgBcMedium (entr_config *tunerCfg);
ENTR_return entr_cfgRfAtten (entr_config *tunerCfg);
ENTR_return entr_cfgBw (entr_config *tunerCfg);
ENTR_return entr_cfgIfOutPutLevel (entr_config *tunerCfg);
ENTR_return entr_cfgMod (entr_config *tunerCfg);
ENTR_return entr_cfgModOrBcMed (entr_config *tunerCfg);

/* misc */
ENTR_return entr_cfgResolveBbErrCondition(entr_config *tunerCfg);
ENTR_return entr_cfgCalDetRead(entr_config *tunerCfg, ENTR_sint32 *Value);
ENTR_return entr_cfgReadTmprAcc(entr_config *tunerCfg, ENTR_sint32 *tmpr);
ENTR_return entr_cfgBbAgcDelay(entr_config *tunerCfg, ENTR_uint8 timeOutMsec);
ENTR_return entr_cfgRssi (entr_config *tunerCfg);
ENTR_return entr_cfgRssiSearchLut (entr_config *tunerCfg, ENTR_sint16 acc, ENTR_sint16 lut[],
                                   ENTR_sint16 lutGainStep, ENTR_sint16 lutMin, ENTR_sint16 lutOffset,
                                   ENTR_sint16 lutLen, ENTR_sint16 *gain);
ENTR_return entr_cfgHystModeOff (entr_config *tunerCfg);
ENTR_return entr_cfgHystModeOn (entr_config *tunerCfg);
/**********************************************************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrFreq.h
 *
 * @brief Prototypes and definitions needed by entrFreq.c
 *
 *   $xRev: 11914 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRFREQ_H__
#define __ENTRFREQ_H__

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************************************/

//#define ENTR_freqFxtalHzMin 20000000
//#define ENTR_freqFxtalHzMax 24000000
//#define ENTR_freqFcompHzMin 250000
//#define ENTR_freqFcompHzMax 2000000
//#define ENTR_freqFspinMaxHzMin 500000
//#define ENTR_freqFifMaxHzMin 50000000

#define ENTR_freqLoSmTimeOutMsec 100

#define ENTR_freqSetLnaModeAccMin 4090

#define ENTR_freqVCO1lowV_max_Hz 867450000
#define ENTR_freqVCO2lowV_max_Hz 930150000
#define ENTR_freqVCO3lowV_max_Hz 1013850000
#define ENTR_freqVCO4lowV_max_Hz 1132150000
#define ENTR_freqVCO5lowV_max_Hz 1215300000
#define ENTR_freqVCO6lowV_max_Hz 1329050000
#define ENTR_freqVCO7lowV_max_Hz 1488300000
#define ENTR_freqVCO8lowV_max_Hz 1594350000
#define ENTR_freqVCO9lowV_max_Hz 1731150000

/**********************************************************************************************************************/

ENTR_return entr_freqSetFxtalAndFcomp(entr_config *tunerCfg);
ENTR_return entr_freqSetFloHz(entr_config *tunerCfg, ENTR_sint32 floHz);
ENTR_return entr_freqSetFspinHz(entr_config *tunerCfg, ENTR_sint32 FspinHz);
ENTR_return entr_freqSetFifHz(entr_config *tunerCfg, ENTR_sint32 FifHz);
ENTR_return entr_freqSetTfBand(entr_config *tunerCfg);
ENTR_return entr_freqSetTtGen(entr_config *tunerCfg);
ENTR_return entr_freqSetLc(entr_config *tunerCfg);
ENTR_return entr_freqSetLnaMode(entr_config *tunerCfg);
ENTR_return entr_freqCfgTtGen(entr_config *tunerCfg, ENTR_uint8 LoDiv);

#ifdef __cplusplus
}
#endif
#endif
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrIfCal.h
 *
 * @brief Prototypes and definitions needed by entrIfCal.c
 *
 *   $xRev: 11439 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRIFCAL_H__
#define __ENTRIFCAL_H__


#ifdef __cplusplus
extern "C" {
#endif

/* used as index in entrIfCal.c */
typedef enum {
    ENTR_ifCalBw_6MHz,
    ENTR_ifCalBw_7MHz,
    ENTR_ifCalBw_8MHz,
    ENTR_ifCalBw_10MHz
} ENTR_ifCalBw;

/**********************************************************************************************************************/

#define ENTR_ifCalN 16
#define ENTR_ifCalD_6MHz 64
#define ENTR_ifCalD_7MHz 64
#define ENTR_ifCalD_8MHz 64
#define ENTR_ifCalD_10MHz 64

/**********************************************************************************************************************/

/* choose such that spin and IF freq can be set! */
#define ENTR_ifFttHz_Ref 3000000
#define ENTR_ifFttHz_6MHz 4204102
#define ENTR_ifFttHz_7MHz 4904297
#define ENTR_ifFttHz_8MHz 5604492
#define ENTR_ifFttHz_10MHz 7004883

#define ENTR_ifAccTopRefN_Cable 323 /* 10 dB backoff */
#define ENTR_ifAccTopRefN_Ter 51 /* 26 dB backoff */
#define ENTR_ifAccTopRefDbits 10

/**********************************************************************************************************************/

ENTR_return enterr_ifCalBW(entr_config *tunerCfg);
ENTR_return entr_ifCalBWSngleFreq(entr_config *tunerCfg, ENTR_sint32 ifCalVref, ENTR_sint32 ifCalD, ENTR_uint8 ifBw);
ENTR_return entr_ifSetBw(entr_config *tunerCfg);
ENTR_return entr_ifTrigSM(entr_config *tunerCfg);
ENTR_return entr_ifSetTtFreq(entr_config *tunerCfg, ENTR_uint8 ifBw);
ENTR_return entr_ifAccTopCal(entr_config *tunerCfg);
ENTR_return entr_ifAccTopCalSingleBckOff(entr_config *tunerCfg, ENTR_uint8 ifAccTopMode, ENTR_sint32 ifAccTopRefD);
ENTR_return entr_ifSetAccTop(entr_config *tunerCfg);

#ifdef __cplusplus
}
#endif
#endif
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrReg.h
 *
 * @brief Prototypes and definitions needed by entrReg.c and most register
 * @brief get/set calls.
 *
 *   $xRev: 11942 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRREG_H__
#define __ENTRREG_H__


#ifdef __cplusplus
extern "C" {
#endif

#define ENTR_dieRev_reg 0
#define ENTR_dieRev_mask 31

#define ENTR_dieCfg_reg 0
#define ENTR_dieCfg_mask 224

#define ENTR_synAnLock_reg 1
#define ENTR_synAnLock_mask 1
#define ENTR_synAnLock_off 0
#define ENTR_synAnLock_on 1

#define ENTR_synDigLock_reg 1
#define ENTR_synDigLock_mask 8
#define ENTR_synDigLock_off 0
#define ENTR_synDigLock_on 8

#define ENTR_synVcoRd_reg 1
#define ENTR_synVcoRd_mask 240

#define ENTR_synVtuneRd_reg 2
#define ENTR_synVtuneRd_mask 15

#define ENTR_tmprRd_reg 3
#define ENTR_tmprRd_mask 127

#define ENTR_agcAccRdLo_reg 7
#define ENTR_agcAccRdLo_mask 255

#define ENTR_agcAccRdHi_reg 8
#define ENTR_agcAccRdHi_mask 255

#define ENTR_agcErrLo_reg 10
#define ENTR_agcErrLo_mask 255

#define ENTR_agcErrHi_reg 11
#define ENTR_agcErrHi_mask 31

#define ENTR_calDetRdLo_reg 12
#define ENTR_calDetRdLo_mask 255

#define ENTR_calDetRdHi_reg 13
#define ENTR_calDetRdHi_mask 127

#define ENTR_calDetDone_reg 13
#define ENTR_calDetDone_mask 128
#define ENTR_calDetDone_notDone 0
#define ENTR_calDetDone_done 128

#define ENTR_ifCapWdRd_reg 14
#define ENTR_ifCapWdRd_mask 31

#define ENTR_ifCalErr_reg 14
#define ENTR_ifCalErr_mask 32
#define ENTR_ifCalErr_noErr 0
#define ENTR_ifCalErr_Err 32

#define ENTR_ifCalDone_reg 14
#define ENTR_ifCalDone_mask 64
#define ENTR_ifCalDone_notDone 0
#define ENTR_ifCalDone_done 64

#define ENTR_tfcCapWdRd_reg 15
#define ENTR_tfcCapWdRd_mask 63

#define ENTR_tfcErr_reg 15
#define ENTR_tfcErr_mask 64
#define ENTR_tfcErr_noErr 0
#define ENTR_tfcErr_err 64

#define ENTR_tfcDone_reg 15
#define ENTR_tfcDone_mask 128
#define ENTR_tfcDone_notDone 0
#define ENTR_tfcDone_done 128

#define ENTR_iAttCtrl_reg 18
#define ENTR_iAttCtrl_mask 1
#define ENTR_iAttCtrl_off 0
#define ENTR_iAttCtrl_on 1

#define ENTR_ltaEna_reg 18
#define ENTR_ltaEna_mask 4
#define ENTR_ltaEna_off 0
#define ENTR_ltaEna_on 4

#define ENTR_lnaMode_reg 18
#define ENTR_lnaMode_mask 24
#define ENTR_lnaMode_Cal 0
#define ENTR_lnaMode_terLoNoise 8
#define ENTR_lnaMode_terNorm 16
#define ENTR_lnaMode_cab 24

#define ENTR_dualChipSel_reg 18
#define ENTR_dualChipSel_mask 32
#define ENTR_dualChipSel_single 0
#define ENTR_dualChipSel_dual 32

#define ENTR_mixMode_reg 19
#define ENTR_mixMode_mask 48
#define ENTR_mixMode_phs8 0
#define ENTR_mixMode_phs4 16
#define ENTR_mixMode_phs2 32
#define ENTR_mixMode_disconSig 48

#define ENTR_synLoDiv_reg 20
#define ENTR_synLoDiv_mask 7
#define ENTR_synLoDiv_reg019Lsbp0Idx0 0
#define ENTR_synLoDiv_div4Phs2 1
#define ENTR_synLoDiv_div16Phs8 2
#define ENTR_synLoDiv_div8Phs4 3
#define ENTR_synLoDiv_reg019Lsbp0Idx4 4
#define ENTR_synLoDiv_reg019Lsbp0Idx5 5
#define ENTR_synLoDiv_div32Phs8 6
#define ENTR_synLoDiv_div64Phs8 7

#define ENTR_ttBufCtrl_reg 20
#define ENTR_ttBufCtrl_mask 128
#define ENTR_ttBufCtrl_off 0
#define ENTR_ttBufCtrl_on 128

#define ENTR_sePinCtrl_reg 21
#define ENTR_sePinCtrl_mask 2
#define ENTR_sePinCtrl_off 0
#define ENTR_sePinCtrl_on 2

#define ENTR_shPinCtrl_reg 21
#define ENTR_shPinCtrl_mask 4
#define ENTR_shPinCtrl_off 0
#define ENTR_shPinCtrl_on 4

#define ENTR_tfBand_reg 22
#define ENTR_tfBand_mask 6
#define ENTR_tfBand_LB 0
#define ENTR_tfBand_MB 2
#define ENTR_tfBand_HB 4
#define ENTR_tfBand_TT 6

#define ENTR_tfAttCw_reg 22
#define ENTR_tfAttCw_mask 120
#define ENTR_tfAttCw_AttCtl_0 0
#define ENTR_tfAttCw_AttCtl_1 8
#define ENTR_tfAttCw_AttCtl_2 16
#define ENTR_tfAttCw_AttCtl_3 24
#define ENTR_tfAttCw_AttCtl_4 32
#define ENTR_tfAttCw_AttCtl_5 40
#define ENTR_tfAttCw_AttCtl_6 48
#define ENTR_tfAttCw_AttCtl_7 56
#define ENTR_tfAttCw_AttCtl_8 64
#define ENTR_tfAttCw_AttCtl_9 72
#define ENTR_tfAttCw_AttCtl_10 80
#define ENTR_tfAttCw_AttCtl_11 88
#define ENTR_tfAttCw_AttCtl_12 96
#define ENTR_tfAttCw_AttCtl_13 104
#define ENTR_tfAttCw_AttCtl_14 112
#define ENTR_tfAttCw_AttCtl_15 120

#define ENTR_tfDeQhb_reg 22
#define ENTR_tfDeQhb_mask 128
#define ENTR_tfDeQhb_Off 0
#define ENTR_tfDeQhb_On 128

#define ENTR_tfDeQmb_reg 23
#define ENTR_tfDeQmb_mask 64
#define ENTR_tfDeQmb_off 0
#define ENTR_tfDeQmb_on 64

#define ENTR_tfDeQlb_reg 23
#define ENTR_tfDeQlb_mask 128
#define ENTR_tfDeQlb_off 0
#define ENTR_tfDeQlb_on 128

#define ENTR_detLvlCtrl_reg 27
#define ENTR_detLvlCtrl_mask 12
#define ENTR_detLvlCtrl_rangeM22M20 0
#define ENTR_detLvlCtrl_rangeM26M23 4
#define ENTR_detLvlCtrl_rangeM30M27 8
#define ENTR_detLvlCtrl_rangeM35M31 12

#define ENTR_tmprCtrl_reg 27
#define ENTR_tmprCtrl_mask 16
#define ENTR_tmprCtrl_TmprPwrOff 0
#define ENTR_tmprCtrl_TmprPwrOn 16

#define ENTR_synCPcur_reg 30
#define ENTR_synCPcur_mask 24
#define ENTR_synCPcur_mA0p4 0
#define ENTR_synCPcur_mA0p8 8
#define ENTR_synCPcur_mA1p6 16
#define ENTR_synCPcur_mA2p0 24

#define ENTR_synN_reg 32
#define ENTR_synN_mask 255

#define ENTR_synA_reg 33
#define ENTR_synA_mask 31

#define ENTR_synR_reg 34
#define ENTR_synR_mask 127

#define ENTR_synNrange_reg 35
#define ENTR_synNrange_mask 3
#define ENTR_synNrange_range0 0
#define ENTR_synNrange_range1 1
#define ENTR_synNrange_range2 2
#define ENTR_synNrange_range3 3

#define ENTR_synSmClk_reg 35
#define ENTR_synSmClk_mask 12
#define ENTR_synSmClk_vcoSM0 0
#define ENTR_synSmClk_vcoSM1 4
#define ENTR_synSmClk_vcoSM2 8
#define ENTR_synSmClk_vcoSM3 12

#define ENTR_synVcoWr_reg 35
#define ENTR_synVcoWr_mask 240
#define ENTR_synVcoWr_VCO0 0
#define ENTR_synVcoWr_VCO1 16
#define ENTR_synVcoWr_VCO2 32
#define ENTR_synVcoWr_VCO3 48
#define ENTR_synVcoWr_VCO4 64
#define ENTR_synVcoWr_VCO5 80
#define ENTR_synVcoWr_VCO6 96
#define ENTR_synVcoWr_VCO7 112
#define ENTR_synVcoWr_VCO8 128

#define ENTR_synSmTrig_reg 36
#define ENTR_synSmTrig_mask 1
#define ENTR_synSmTrig_dis 0
#define ENTR_synSmTrig_ena 1

#define ENTR_ttlnjectPt_reg 40
#define ENTR_ttlnjectPt_mask 2
#define ENTR_ttlnjectPt_Lna 0
#define ENTR_ttlnjectPt_Mix 2

#define ENTR_ttPwr_reg 40
#define ENTR_ttPwr_mask 28
#define ENTR_ttPwr_LnaNeg80MixNeg68dBV 0
#define ENTR_ttPwr_LnaNeg74MixNeg62dBV 4
#define ENTR_ttPwr_LnaNeg68MixNeg56dBV 8
#define ENTR_ttPwr_LnaNeg62MixNeg50dBV 12
#define ENTR_ttPwr_LnaNeg56MixNeg44dBV 16
#define ENTR_ttPwr_LnaNeg50MixNeg38dBV 20
#define ENTR_ttPwr_LnaNeg44MixNeg32dBV 24
#define ENTR_ttPwr_LnaNeg44MixNeg32again 28

#define ENTR_ttFilter_reg 43
#define ENTR_ttFilter_mask 255

#define ENTR_ifCapWdWr_reg 44
#define ENTR_ifCapWdWr_mask 31

#define ENTR_calDetTrig_reg 45
#define ENTR_calDetTrig_mask 1
#define ENTR_calDetTrig_ClearTrigger 0
#define ENTR_calDetTrig_Trigger 1

#define ENTR_calDetCtrl_reg 45
#define ENTR_calDetCtrl_mask 64
#define ENTR_calDetCtrl_B2W 0
#define ENTR_calDetCtrl_SM 64

#define ENTR_ifCalTrig_reg 46
#define ENTR_ifCalTrig_mask 16
#define ENTR_ifCalTrig_ClearTrigger 0
#define ENTR_ifCalTrig_Trigger 16

#define ENTR_ifCalVrefLo_reg 47
#define ENTR_ifCalVrefLo_mask 255

#define ENTR_ifCalVrefHi_reg 48
#define ENTR_ifCalVrefHi_mask 127

#define ENTR_ifCapWdCtrl_reg 48
#define ENTR_ifCapWdCtrl_mask 128
#define ENTR_ifCapWdCtrl_B2W 0
#define ENTR_ifCapWdCtrl_SM 128

#define ENTR_bbHysMaxCnt_reg 104
#define ENTR_bbHysMaxCnt_mask 3
#define ENTR_bbHysMaxCnt_time10p9msecs 0
#define ENTR_bbHysMaxCnt_time21p8msecs 1
#define ENTR_bbHysMaxCnt_time43p7msecs 2
#define ENTR_bbHysMaxCnt_time87p4msecs 3

#define ENTR_bbHysDelta_reg 104
#define ENTR_bbHysDelta_mask 12
#define ENTR_bbHysDelta_window16 0
#define ENTR_bbHysDelta_window64 4
#define ENTR_bbHysDelta_window256 8
#define ENTR_bbHysDelta_window1024 12

#define ENTR_bbHysErrMax_reg 104
#define ENTR_bbHysErrMax_mask 240
#define ENTR_bbHysErrMax_BBErrMx32 0
#define ENTR_bbHysErrMax_BBErrMx64 16
#define ENTR_bbHysErrMax_BBErrMx96 32
#define ENTR_bbHysErrMax_BBErrMx128 48
#define ENTR_bbHysErrMax_BBErrMx160 64
#define ENTR_bbHysErrMax_BBErrMx192 80
#define ENTR_bbHysErrMax_BBErrMx224 96
#define ENTR_bbHysErrMax_BBErrMx256 112
#define ENTR_bbHysErrMax_BBErrMx288 128
#define ENTR_bbHysErrMax_BBErrMx320 144
#define ENTR_bbHysErrMax_BBErrMx352 160
#define ENTR_bbHysErrMax_BBErrMx384 176
#define ENTR_bbHysErrMax_BBErrMx416 192
#define ENTR_bbHysErrMax_BBErrMx448 208
#define ENTR_bbHysErrMax_BBErrMx480 224
#define ENTR_bbHysErrMax_BBErrMx512 240

#define ENTR_rfHysMaxCnt_reg 105
#define ENTR_rfHysMaxCnt_mask 3
#define ENTR_rfHysMaxCnt_time21p8msecs 0
#define ENTR_rfHysMaxCnt_time43p7msecs 1
#define ENTR_rfHysMaxCnt_time87p2msecs 2
#define ENTR_rfHysMaxCnt_time174p4msecs 3

#define ENTR_rfHysDelta_reg 105
#define ENTR_rfHysDelta_mask 12
#define ENTR_rfHysDelta_window16 0
#define ENTR_rfHysDelta_window64 4
#define ENTR_rfHysDelta_window256 8
#define ENTR_rfHysDelta_window1024 12

#define ENTR_rfHysErrMax_reg 105
#define ENTR_rfHysErrMax_mask 240
#define ENTR_rfHysErrMax_RFErrMx32 0
#define ENTR_rfHysErrMax_RFErrMx64 16
#define ENTR_rfHysErrMax_RFErrMx96 32
#define ENTR_rfHysErrMax_RFErrMx128 48
#define ENTR_rfHysErrMax_RFErrMx160 64
#define ENTR_rfHysErrMax_RFErrMx192 80
#define ENTR_rfHysErrMax_RFErrMx224 96
#define ENTR_rfHysErrMax_RFErrMx256 112
#define ENTR_rfHysErrMax_RFErrMx288 128
#define ENTR_rfHysErrMax_RFErrMx320 144
#define ENTR_rfHysErrMax_RFErrMx352 160
#define ENTR_rfHysErrMax_RFErrMx384 176
#define ENTR_rfHysErrMax_RFErrMx416 192
#define ENTR_rfHysErrMax_RFErrMx448 208
#define ENTR_rfHysErrMax_RFErrMx480 224
#define ENTR_rfHysErrMax_RFErrMx512 240

#define ENTR_dcHysMaxCnt_reg 106
#define ENTR_dcHysMaxCnt_mask 3
#define ENTR_dcHysMaxCnt_time10p9msecs 0
#define ENTR_dcHysMaxCnt_time21p8msecs 1
#define ENTR_dcHysMaxCnt_time43p7msecs 2
#define ENTR_dcHysMaxCnt_time87p4msecs 3

#define ENTR_dcHysDelta_reg 106
#define ENTR_dcHysDelta_mask 12
#define ENTR_dcHysDelta_window16 0
#define ENTR_dcHysDelta_window64 4
#define ENTR_dcHysDelta_window256 8
#define ENTR_dcHysDelta_window1024 12

#define ENTR_dcHysADCMax_reg 106
#define ENTR_dcHysADCMax_mask 240
#define ENTR_dcHysADCMax_ADCMx128 0
#define ENTR_dcHysADCMax_ADCMx256 16
#define ENTR_dcHysADCMax_ADCMx384 32
#define ENTR_dcHysADCMax_ADCMx512 48
#define ENTR_dcHysADCMax_ADCMx640 64
#define ENTR_dcHysADCMax_ADCMx768 80
#define ENTR_dcHysADCMax_ADCMx896 96
#define ENTR_dcHysADCMax_ADCMx1024 112
#define ENTR_dcHysADCMax_ADCMx1152 128
#define ENTR_dcHysADCMax_ADCMx1280 144
#define ENTR_dcHysADCMax_ADCMx1408 160
#define ENTR_dcHysADCMax_ADCMx1536 176
#define ENTR_dcHysADCMax_ADCMx1664 192
#define ENTR_dcHysADCMax_ADCMx1792 208
#define ENTR_dcHysADCMax_ADCMx1920 224
#define ENTR_dcHysADCMax_ADCMx2048 240

#define ENTR_ifAgcSrc_reg 122
#define ENTR_ifAgcSrc_mask 32
#define ENTR_ifAgcSrc_useB2wFnlAgc 0
#define ENTR_ifAgcSrc_useDemodFnlAgc 32

#define ENTR_ifNlo_reg 125
#define ENTR_ifNlo_mask 255

#define ENTR_ifNhi_reg 126
#define ENTR_ifNhi_mask 255

#define ENTR_specInv_reg 127
#define ENTR_specInv_mask 8
#define ENTR_specInv_off 0
#define ENTR_specInv_on 8

#define ENTR_reset_reg 129
#define ENTR_reset_mask 1
#define ENTR_reset_Reset 0
#define ENTR_reset_NotReset 1

#define ENTR_digDCbwFst_reg 132
#define ENTR_digDCbwFst_mask 15
#define ENTR_digDCbwFst_BW29p8kHz 0
#define ENTR_digDCbwFst_BW14p9kHz 1
#define ENTR_digDCbwFst_BW7p5kHz 2
#define ENTR_digDCbwFst_BW3p7kHz 3
#define ENTR_digDCbwFst_BW1p9kHz 4
#define ENTR_digDCbwFst_BW933Hz 5
#define ENTR_digDCbwFst_BW466Hz 6
#define ENTR_digDCbwFst_BW233Hz 7
#define ENTR_digDCbwFst_BW117Hz 8
#define ENTR_digDCbwFst_BW58Hz 9
#define ENTR_digDCbwFst_BW29Hz 10
#define ENTR_digDCbwFst_BW14p6Hz 11
#define ENTR_digDCbwFst_BW7p3Hz 12
#define ENTR_digDCbwFst_BW3p6Hz 13
#define ENTR_digDCbwFst_BW1p8Hz 14
#define ENTR_digDCbwFst_BW0p9Hz 15

#define ENTR_digDCbwSlw_reg 132
#define ENTR_digDCbwSlw_mask 240
#define ENTR_digDCbwSlw_BW1p9kHz 0
#define ENTR_digDCbwSlw_BW933Hz 16
#define ENTR_digDCbwSlw_BW466Hz 32
#define ENTR_digDCbwSlw_BW233Hz 48
#define ENTR_digDCbwSlw_BW117Hz 64
#define ENTR_digDCbwSlw_BW58Hz 80
#define ENTR_digDCbwSlw_BW29Hz 96
#define ENTR_digDCbwSlw_BW14p6Hz 112
#define ENTR_digDCbwSlw_BW7p3Hz 128
#define ENTR_digDCbwSlw_BW3p6Hz 144
#define ENTR_digDCbwSlw_BW1p8Hz 160
#define ENTR_digDCbwSlw_BW0p9Hz 176
#define ENTR_digDCbwSlw_BW0p46Hz 192
#define ENTR_digDCbwSlw_BW0p23Hz 208
#define ENTR_digDCbwSlw_BW0p11Hz 224
#define ENTR_digDCbwSlw_BW0p06Hz 240

#define ENTR_IQPhBwFst_reg 133
#define ENTR_IQPhBwFst_mask 15
#define ENTR_IQPhBwFst_BW3p7kHz 0
#define ENTR_IQPhBwFst_BW1p9kHz 1
#define ENTR_IQPhBwFst_BW933Hz 2
#define ENTR_IQPhBwFst_BW466Hz 3
#define ENTR_IQPhBwFst_BW233Hz 4
#define ENTR_IQPhBwFst_BW117Hz 5
#define ENTR_IQPhBwFst_BW58Hz 6
#define ENTR_IQPhBwFst_BW29Hz 7
#define ENTR_IQPhBwFst_BW14p6Hz 8
#define ENTR_IQPhBwFst_BW7p3Hz 9
#define ENTR_IQPhBwFst_BW3p6Hz 10
#define ENTR_IQPhBwFst_BW1p8Hz 11
#define ENTR_IQPhBwFst_BW0p9Hz 12
#define ENTR_IQPhBwFst_BW0p5Hz 13
#define ENTR_IQPhBwFst_BW0p2Hz 14
#define ENTR_IQPhBwFst_BW0p1Hz 15

#define ENTR_IQPhBwSlw_reg 133
#define ENTR_IQPhBwSlw_mask 240
#define ENTR_IQPhBwSlw_BW3p7kHz 0
#define ENTR_IQPhBwSlw_BW1p9kHz 16
#define ENTR_IQPhBwSlw_BW933Hz 32
#define ENTR_IQPhBwSlw_BW466Hz 48
#define ENTR_IQPhBwSlw_BW233Hz 64
#define ENTR_IQPhBwSlw_BW117Hz 80
#define ENTR_IQPhBwSlw_BW58Hz 96
#define ENTR_IQPhBwSlw_BW29Hz 112
#define ENTR_IQPhBwSlw_BW14p6Hz 128
#define ENTR_IQPhBwSlw_BW7p3Hz 144
#define ENTR_IQPhBwSlw_BW3p6Hz 160
#define ENTR_IQPhBwSlw_BW1p8Hz 176
#define ENTR_IQPhBwSlw_BW0p9Hz 192
#define ENTR_IQPhBwSlw_BW0p5Hz 208
#define ENTR_IQPhBwSlw_BW0p2Hz 224
#define ENTR_IQPhBwSlw_BW0p1Hz 240

#define ENTR_IQGainBwFst_reg 134
#define ENTR_IQGainBwFst_mask 15
#define ENTR_IQGainBwFst_BW7p5kHz 0
#define ENTR_IQGainBwFst_BW3p7kHz 1
#define ENTR_IQGainBwFst_BW1p9Hz 2
#define ENTR_IQGainBwFst_BW933Hz 3
#define ENTR_IQGainBwFst_BW466Hz 4
#define ENTR_IQGainBwFst_BW233Hz 5
#define ENTR_IQGainBwFst_BW117Hz 6
#define ENTR_IQGainBwFst_BW58Hz 7
#define ENTR_IQGainBwFst_BW29p1Hz 8
#define ENTR_IQGainBwFst_BW14p6Hz 9
#define ENTR_IQGainBwFst_BW7p3Hz 10
#define ENTR_IQGainBwFst_BW3p6Hz 11
#define ENTR_IQGainBwFst_BW1p8Hz 12
#define ENTR_IQGainBwFst_BW0p9Hz 13
#define ENTR_IQGainBwFst_BW0p5Hz 14
#define ENTR_IQGainBwFst_BW0p2Hz 15

#define ENTR_IQGainBwSlw_reg 134
#define ENTR_IQGainBwSlw_mask 240
#define ENTR_IQGainBwSlw_BW7p5kHz 0
#define ENTR_IQGainBwSlw_BW3p7kHz 16
#define ENTR_IQGainBwSlw_BW1p9Hz 32
#define ENTR_IQGainBwSlw_BW933Hz 48
#define ENTR_IQGainBwSlw_BW466Hz 64
#define ENTR_IQGainBwSlw_BW233Hz 80
#define ENTR_IQGainBwSlw_BW117Hz 96
#define ENTR_IQGainBwSlw_BW58Hz 112
#define ENTR_IQGainBwSlw_BW29p1Hz 128
#define ENTR_IQGainBwSlw_BW14p6Hz 144
#define ENTR_IQGainBwSlw_BW7p3Hz 160
#define ENTR_IQGainBwSlw_BW3p6Hz 176
#define ENTR_IQGainBwSlw_BW1p8Hz 192
#define ENTR_IQGainBwSlw_BW0p9Hz 208
#define ENTR_IQGainBwSlw_BW0p5Hz 224
#define ENTR_IQGainBwSlw_BW0p2Hz 240

#define ENTR_lcCtrl_reg 136
#define ENTR_lcCtrl_mask 8
#define ENTR_lcCtrl_off 0
#define ENTR_lcCtrl_on 8

#define ENTR_lccLo_reg 137
#define ENTR_lccLo_mask 255

#define ENTR_lccMid_reg 138
#define ENTR_lccMid_mask 255

#define ENTR_lccHi_reg 139
#define ENTR_lccHi_mask 63

#define ENTR_lcsLo_reg 140
#define ENTR_lcsLo_mask 255

#define ENTR_lcsMid_reg 141
#define ENTR_lcsMid_mask 255

#define ENTR_lcsHi_reg 142
#define ENTR_lcsHi_mask 63

#define ENTR_rotLo_reg 143
#define ENTR_rotLo_mask 255

#define ENTR_rotMid_reg 144
#define ENTR_rotMid_mask 255

#define ENTR_rotHi_reg 145
#define ENTR_rotHi_mask 15

#define ENTR_agcAccWrLo_reg 146
#define ENTR_agcAccWrLo_mask 255

#define ENTR_agcAccWrHi_reg 147
#define ENTR_agcAccWrHi_mask 255

#define ENTR_agcAccSel_reg 149
#define ENTR_agcAccSel_mask 15
#define ENTR_agcAccSel_BB 0
#define ENTR_agcAccSel_RF 1
#define ENTR_agcAccSel_IDCA 2
#define ENTR_agcAccSel_QDCA 3
#define ENTR_agcAccSel_IDCD 4
#define ENTR_agcAccSel_QDCD 5
#define ENTR_agcAccSel_PhIQ 6
#define ENTR_agcAccSel_AmpIQ 7
#define ENTR_agcAccSel_LineCancelI 8
#define ENTR_agcAccSel_LineCancelQ 9
#define ENTR_agcAccSel_DigitalAGC 10
#define ENTR_agcAccSel_RFDetADC 11
#define ENTR_agcAccSel_InDAGCADC 12
#define ENTR_agcAccSel_RfZeroPwr 13

#define ENTR_agcAccLoad_reg 149
#define ENTR_agcAccLoad_mask 128
#define ENTR_agcAccLoad_disWr 0
#define ENTR_agcAccLoad_enaWr 128

#define ENTR_ifAgcEna_reg 150
#define ENTR_ifAgcEna_mask 1
#define ENTR_ifAgcEna_off 0
#define ENTR_ifAgcEna_on 1

#define ENTR_rfCloseLoop_reg 150
#define ENTR_rfCloseLoop_mask 4
#define ENTR_rfCloseLoop_off 0
#define ENTR_rfCloseLoop_on 4

#define ENTR_bbAdcBckOff_reg 151
#define ENTR_bbAdcBckOff_mask 15
#define ENTR_bbAdcBckOff_BkOffFrmFS_0dB 0
#define ENTR_bbAdcBckOff_BkOffFrmFS_1dB 1
#define ENTR_bbAdcBckOff_BkOffFrmFS_2dB 2
#define ENTR_bbAdcBckOff_BkOffFrmFS_3dB 3
#define ENTR_bbAdcBckOff_BkOffFrmFS_4dB 4
#define ENTR_bbAdcBckOff_BkOffFrmFS_5dB 5
#define ENTR_bbAdcBckOff_BkOffFrmFS_6dB 6
#define ENTR_bbAdcBckOff_BkOffFrmFS_7dB 7
#define ENTR_bbAdcBckOff_BkOffFrmFS_8dB 8
#define ENTR_bbAdcBckOff_BkOffFrmFS_9dB 9
#define ENTR_bbAdcBckOff_BkOffFrmFS_10dB 10
#define ENTR_bbAdcBckOff_BkOffFrmFS_11dB 11
#define ENTR_bbAdcBckOff_BkOffFrmFS_12dB 12
#define ENTR_bbAdcBckOff_BkOffFrmFS_13dB 13
#define ENTR_bbAdcBckOff_BkOffFrmFS_14dB 14
#define ENTR_bbAdcBckOff_BkOffFrmFS_15dB 15

#define ENTR_ifAgcShift_reg 151
#define ENTR_ifAgcShift_mask 112
#define ENTR_ifAgcShift_bbagcshft_0 0
#define ENTR_ifAgcShift_bbagcshft_1 16
#define ENTR_ifAgcShift_bbagcshft_2 32
#define ENTR_ifAgcShift_bbagcshft_3 48
#define ENTR_ifAgcShift_bbagcshft_4 64
#define ENTR_ifAgcShift_bbagcshft_5 80
#define ENTR_ifAgcShift_bbagcshft_6 96
#define ENTR_ifAgcShift_bbagcshft_7 112

#define ENTR_ifVga2Ena_reg 151
#define ENTR_ifVga2Ena_mask 128
#define ENTR_ifVga2Ena_off 0
#define ENTR_ifVga2Ena_on 128

#define ENTR_ifAgcAccTopLo_reg 152
#define ENTR_ifAgcAccTopLo_mask 255

#define ENTR_ifAgcAccTopHi_reg 153
#define ENTR_ifAgcAccTopHi_mask 15

#define ENTR_ifAgcLoopBw_reg 154
#define ENTR_ifAgcLoopBw_mask 7
#define ENTR_ifAgcLoopBw_BW_4p8Hz 0
#define ENTR_ifAgcLoopBw_BW_2p4Hz 1
#define ENTR_ifAgcLoopBw_BW_1p2Hz 2
#define ENTR_ifAgcLoopBw_BW_0p6Hz 3
#define ENTR_ifAgcLoopBw_BW_0p3Hz 4

#define ENTR_rfDetEna_reg 154
#define ENTR_rfDetEna_mask 16
#define ENTR_rfDetEna_off 0
#define ENTR_rfDetEna_on 16

#define ENTR_rfAgcTop_reg 155
#define ENTR_rfAgcTop_mask 15
#define ENTR_rfAgcTop_Vdet_0p1 0
#define ENTR_rfAgcTop_Vdet_0p12 1
#define ENTR_rfAgcTop_Vdet_0p14 2
#define ENTR_rfAgcTop_Vdet_0p16 3
#define ENTR_rfAgcTop_Vdet_0p18 4
#define ENTR_rfAgcTop_Vdet_0p2 5
#define ENTR_rfAgcTop_Vdet_0p22 6
#define ENTR_rfAgcTop_Vdet_0p24 7
#define ENTR_rfAgcTop_Vdet_0p26 8
#define ENTR_rfAgcTop_Vdet_0p28 9
#define ENTR_rfAgcTop_Vdet_0p3 10
#define ENTR_rfAgcTop_Vdet_0p32 11
#define ENTR_rfAgcTop_Vdet_0p34 12
#define ENTR_rfAgcTop_Vdet_0p36 13
#define ENTR_rfAgcTop_Vdet_0p38 14
#define ENTR_rfAgcTop_Vdet_0p4 15

#define ENTR_rfAgcLoopBw_reg 156
#define ENTR_rfAgcLoopBw_mask 112
#define ENTR_rfAgcLoopBw_BW80Hz 0
#define ENTR_rfAgcLoopBw_BW40Hz 16
#define ENTR_rfAgcLoopBw_BW20Hz 32
#define ENTR_rfAgcLoopBw_BW10Hz 48
#define ENTR_rfAgcLoopBw_BW5Hz 64
#define ENTR_rfAgcLoopBw_BW2p5Hz 80
#define ENTR_rfAgcLoopBw_reg156LSBp06 96
#define ENTR_rfAgcLoopBw_reg156LSBp07 112

#define ENTR_rfDetCalAve_reg 156
#define ENTR_rfDetCalAve_mask 128
#define ENTR_rfDetCalAve_off 0
#define ENTR_rfDetCalAve_on 128

#define ENTR_dcILoopBw_reg 157
#define ENTR_dcILoopBw_mask 7
#define ENTR_dcILoopBw_BW400Hz 0
#define ENTR_dcILoopBw_BW200Hz 1
#define ENTR_dcILoopBw_BW100Hz 2
#define ENTR_dcILoopBw_BW50Hz 3
#define ENTR_dcILoopBw_BW25Hz 4
#define ENTR_dcILoopBw_BW12p5Hz 5
#define ENTR_dcILoopBw_BW6p25Hz 6
#define ENTR_dcILoopBw_BW3p125Hz 7

#define ENTR_dcQLoopBw_reg 157
#define ENTR_dcQLoopBw_mask 112
#define ENTR_dcQLoopBw_BW400Hz 0
#define ENTR_dcQLoopBw_BW200Hz 16
#define ENTR_dcQLoopBw_BW100Hz 32
#define ENTR_dcQLoopBw_BW50Hz 48
#define ENTR_dcQLoopBw_BW25Hz 64
#define ENTR_dcQLoopBw_BW12p5Hz 80
#define ENTR_dcQLoopBw_BW6p25Hz 96
#define ENTR_dcQLoopBw_BW3p125Hz 112

#define ENTR_rfDetCalFrc_reg 158
#define ENTR_rfDetCalFrc_mask 1
#define ENTR_rfDetCalFrc_off 0
#define ENTR_rfDetCalFrc_on 1

#define ENTR_rfDetCalAuto_reg 158
#define ENTR_rfDetCalAuto_mask 2
#define ENTR_rfDetCalAuto_off 0
#define ENTR_rfDetCalAuto_on 2

#define ENTR_agcSmEna_reg 158
#define ENTR_agcSmEna_mask 4
#define ENTR_agcSmEna_off 0
#define ENTR_agcSmEna_on 4

#define ENTR_dcHysMode_reg 158
#define ENTR_dcHysMode_mask 8
#define ENTR_dcHysMode_off 0
#define ENTR_dcHysMode_on 8

#define ENTR_rfHysMode_reg 158
#define ENTR_rfHysMode_mask 16
#define ENTR_rfHysMode_off 0
#define ENTR_rfHysMode_on 16

#define ENTR_bbHysMode_reg 158
#define ENTR_bbHysMode_mask 32
#define ENTR_bbHysMode_off 0
#define ENTR_bbHysMode_on 32

#define ENTR_iAttLimitLo_reg 159
#define ENTR_iAttLimitLo_mask 255

#define ENTR_iAttLimitHi_reg 160
#define ENTR_iAttLimitHi_mask 15

#define ENTR_iAttLimitTest_reg 160
#define ENTR_iAttLimitTest_mask 32
#define ENTR_iAttLimitTest_off 0
#define ENTR_iAttLimitTest_on 32

#define ENTR_shPinLimitLo_reg 161
#define ENTR_shPinLimitLo_mask 255

#define ENTR_shPinLimitHi_reg 162
#define ENTR_shPinLimitHi_mask 15

#define ENTR_shPinLimitTest_reg 162
#define ENTR_shPinLimitTest_mask 128
#define ENTR_shPinLimitTest_off 0
#define ENTR_shPinLimitTest_on 128

#define ENTR_sePinLimitLo_reg 163
#define ENTR_sePinLimitLo_mask 255

#define ENTR_sePinLimitHi_reg 164
#define ENTR_sePinLimitHi_mask 15

#define ENTR_sePinLimitTest_reg 164
#define ENTR_sePinLimitTest_mask 128
#define ENTR_sePinLimitTest_off 0
#define ENTR_sePinLimitTest_on 128

#define ENTR_digAgcBckOff_reg 165
#define ENTR_digAgcBckOff_mask 3
#define ENTR_digAgcBckOff_neg8p5dB 0
#define ENTR_digAgcBckOff_neg6dB 1
#define ENTR_digAgcBckOff_neg2p5dB 2
#define ENTR_digAgcBckOff_reg165LSBp03 3

#define ENTR_digAgcEna_reg 166
#define ENTR_digAgcEna_mask 1
#define ENTR_digAgcEna_off 0
#define ENTR_digAgcEna_on 1

#define ENTR_digAgcBW_reg 166
#define ENTR_digAgcBW_mask 112
#define ENTR_digAgcBW_BW22p5Hz 0
#define ENTR_digAgcBW_BW45Hz 16
#define ENTR_digAgcBW_BW90Hz 32
#define ENTR_digAgcBW_BW180Hz 48
#define ENTR_digAgcBW_BW360Hz 64
#define ENTR_digAgcBW_BW720Hz 80
#define ENTR_digAgcBW_BW1440Hz 96
#define ENTR_digAgcBW_BW2880Hz 112

#define ENTR_digAgcInSel_reg 166
#define ENTR_digAgcInSel_mask 128
#define ENTR_digAgcInSel_Translator_Out 0
#define ENTR_digAgcInSel_Interp_Out 128

#define ENTR_ttSrc_reg 167
#define ENTR_ttSrc_mask 1
#define ENTR_ttSrc_TransNCO 0
#define ENTR_ttSrc_SpinNCO 1

#define ENTR_ttEna_reg 167
#define ENTR_ttEna_mask 32
#define ENTR_ttEna_off 0
#define ENTR_ttEna_on 32

#define ENTR_tfcTrig_reg 167
#define ENTR_tfcTrig_mask 64
#define ENTR_tfcTrig_ClearTrig 0
#define ENTR_tfcTrig_Trig 64

#define ENTR_tfcCapWd0_reg 168
#define ENTR_tfcCapWd0_mask 63

#define ENTR_tfcCapWdMax_reg 168
#define ENTR_tfcCapWdMax_mask 64
#define ENTR_tfcCapWdMax_Max31 0
#define ENTR_tfcCapWdMax_Max63 64

#define ENTR_tfcCapCtrl_reg 168
#define ENTR_tfcCapCtrl_mask 128
#define ENTR_tfcCapCtrl_B2WWrite 0
#define ENTR_tfcCapCtrl_SMWrite 128

#define ENTR_ifLevel_reg 175
#define ENTR_ifLevel_mask 15
#define ENTR_ifLevel_FSRngRed_Nom 0
#define ENTR_ifLevel_FSRngRed_neg0p5 1
#define ENTR_ifLevel_FSRngRed_neg1 2
#define ENTR_ifLevel_FSRngRed_neg1p6 3
#define ENTR_ifLevel_FSRngRed_neg2 4
#define ENTR_ifLevel_FSRngRed_neg2p3 5
#define ENTR_ifLevel_FSRngRed_neg2p8 6
#define ENTR_ifLevel_FSRngRed_neg3p2 7
#define ENTR_ifLevel_FSRngRed_pos6 8
#define ENTR_ifLevel_FSRngRed_pos5 9
#define ENTR_ifLevel_FSRngRed_pos4 10
#define ENTR_ifLevel_FSRngRed_pos3p3 11
#define ENTR_ifLevel_FSRngRed_pos2p5 12
#define ENTR_ifLevel_FSRngRed_pos1p8 13
#define ENTR_ifLevel_FSRngRed_pos1p2 14
#define ENTR_ifLevel_FSRngRed_pos0p6 15

#define ENTR_ifOut2_reg 175
#define ENTR_ifOut2_mask 16
#define ENTR_ifOut2_off 0
#define ENTR_ifOut2_on 16

#define ENTR_ifOut1_reg 175
#define ENTR_ifOut1_mask 32
#define ENTR_ifOut1_off 0
#define ENTR_ifOut1_on 32

#define ENTR_shPinDacCtrl_reg 180
#define ENTR_shPinDacCtrl_mask 1
#define ENTR_shPinDacCtrl_off 0
#define ENTR_shPinDacCtrl_on 1

#define ENTR_sePinDacCtrl_reg 180
#define ENTR_sePinDacCtrl_mask 2
#define ENTR_sePinDacCtrl_off 0
#define ENTR_sePinDacCtrl_on 2

#define ENTR_iAttDacCtrl_reg 180
#define ENTR_iAttDacCtrl_mask 4
#define ENTR_iAttDacCtrl_off 0
#define ENTR_iAttDacCtrl_on 4

#define ENTR_ttGenDiv_reg 186
#define ENTR_ttGenDiv_mask 1
#define ENTR_ttGenDiv_off 0
#define ENTR_ttGenDiv_on 1

ENTR_return entr_usrRegWr(entr_config *tunerCfg, ENTR_uint8 reg, ENTR_uint8 mask, ENTR_uint8 value);
ENTR_return entr_usrRegRd(entr_config *tunerCfg, ENTR_uint8 reg, ENTR_uint8 mask, ENTR_uint8 *value);

#ifdef __cplusplus
}
#endif
#endif
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrTfCal.h
 *
 * @brief Prototypes and definitions needed by entrTfCal.c.
 *
 *   $xRev: 11859 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

#ifndef __ENTRTFCAL_H__
#define __ENTRTFCAL_H__


#ifdef __cplusplus
extern "C" {
#endif

#define ENTR_tfTempAcc25Deg = 150;

#define ENTR_tfSmTimeOutMsec 100;

#define ENTR_tfgFmbMaxHz 450000000
#define ENTR_tfgCompMlo 1409306
#define ENTR_tfgCompBlo 1246
#define ENTR_tfgCompMhi 595341
#define ENTR_tfgCompBhi 1280
#define ENTR_tfgD 1024

#define ENTR_tfFreqCntLb 5
#define ENTR_tfFreqCntMb 5
#define ENTR_tfFreqCntHb 5

ENTR_return entr_tfCal(entr_config *tunerCfg);
ENTR_return entr_tfSetTtPwr(entr_config *tunerCfg, ENTR_uint8 *ttPwr);
ENTR_return entr_tfCalOneBand(entr_config *tunerCfg, ENTR_uint8 tfBand, ENTR_uint8 tfcCapWdMax, ENTR_uint8 tfFreqCnt, ENTR_uint8 tfCw0[], ENTR_sint32 tfFreq[], ENTR_uint8 tffCw[], ENTR_uint8 tfgCw[]);
ENTR_return entr_tfGainCal(entr_config *tunerCfg, ENTR_uint8 tfBand, ENTR_uint8 *tfgCw);
ENTR_return entr_tfcTrig(entr_config *tunerCfg);
ENTR_return entr_tfChanChg(entr_config *tunerCfg);
ENTR_return entr_tfChanChgLinInterp(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tfCw[], ENTR_sint16 *cw);
ENTR_return entr_tfChanChgFreq(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tffCw[], ENTR_uint8 cwMax);
ENTR_return entr_tfChanChgGain(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tfgCw[]);

#ifdef __cplusplus
}
#endif
#endif


#define Delay_Number    1
#define WAIT_Number    1

//#define SYS_WAIT_MS(x) osal_delay((x*1000*WAIT_Number))

#define task_delay(x)    //osal_delay(1)

