#include <sys_config.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>
#include "en4020_tuner.h"

#if ((SYS_TUN_MODULE == EN4020)||(SYS_TUN_MODULE == ANY_TUNER))




/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrCfg.c
 *
 * @brief Methods for configuring IC.
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


/**********************************************************************************************************************
*
* cfg for ifCal:
*  rfAtt in max aten
*  lna in lb
*  tfdeQ on
*  tfAtt at min gain
*  ttSrc trans
*  ttPwr -50 dBV @ mixer
*  ifVga2 on
*  calDetCtrl b2w
*  ifCapWdCtrl b2w
*  ifCapWd 0
*
**********************************************************************************************************************/

ENTR_uint8 entr_cfgIfCalRegs[ENTR_cfgRegCnt] = {
    0x03,    0x0E,    0x82,    0x06,    0x86,    0x40,    0x00,    0x99,    0x48,    0x08,
    0x00,    0x00,    0x13,    0x03,    0x43,    0x08,    0x18,    0x29,    0x7A,    0x05,
    0x84,    0x15,    0x1A,    0x1C,    0x00,    0x00,    0x00,    0x08,    0x01,    0x00,
    0x00,    0x0B,    0xFD,    0xD0,    0xC8,    0x12,    0x3A,    0xE3,    0xA7,    0x22,
    0x00,    0x80,    0x00,    0xD9,    0xBF,    0x4D,    0x3F,    0x2C,    0x00,    0xF7,
    0x00,    0xD0,    0x3F,    0xAF,    0x3E,    0x34,    0x00,    0xD0,    0x01,    0xC9,
    0x3F,    0x72,    0x3D,    0x3A,    0x00,    0xD4,    0x03,    0xC4,    0x3F,    0x54,
    0x39,    0x3D,    0x00,    0x78,    0x14,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x08,    0x00,    0x00,    0x00,    0x00,    0x00,    0xA0,    0x54,    0x98,    0x00,
    0x00,    0x11,    0x11,    0x11,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x30,    0x80,    0xC0,    0x00,    0x00,    0x00,    0x04,    0x00,
    0x00,    0x45,    0xFA,    0x80,    0xBB,    0xBB,    0xBB,    0xBB,    0x01,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x04,    0x00,    0x00,
    0x00,    0x00,    0xFF,    0xF6,    0x00,    0x0C,    0x38,    0xA8,    0xA0,    0xAA,
    0x40,    0x00,    0x20,    0x00,    0x80,    0x00,    0x80,    0x09,    0x03,    0x14,
    0xFF,    0x07,    0x00,    0x00,    0x00,    0x00,    0x00,    0x80,    0x7B,    0x80,
    0x00,    0xF0,    0x3F,    0x0F,    0x01,    0x60,    0x70,    0x09,    0x08
};

/**********************************************************************************************************************
*
* base configuratin, esentialy atsc operation with:
*  rfAtt in max aten
*  dac1 and dac2 off
*
* called at end of entr_active(). final cfg done in entr_set_chan()
*
**********************************************************************************************************************/

ENTR_uint8 entr_cfgBaseRegs[ENTR_cfgRegCnt] = {
    0x0B,    0x0E,    0x02,    0x06,    0x86,    0x40,    0x00,    0x99,    0x48,    0x08,
    0x00,    0x00,    0x13,    0x03,    0x43,    0x08,    0x18,    0x29,    0x7A,    0x05,
    0x84,    0x15,    0x1A,    0x1C,    0x00,    0x00,    0x00,    0x08,    0x01,    0x00,
    0x00,    0x0B,    0xFD,    0xD0,    0xC8,    0x12,    0x3A,    0xE3,    0xA7,    0x22,
    0x00,    0x80,    0x00,    0xD9,    0xBF,    0x4D,    0x3F,    0x2C,    0x00,    0xF7,
    0x00,    0xD0,    0x3F,    0xAF,    0x3E,    0x34,    0x00,    0xD0,    0x01,    0xC9,
    0x3F,    0x72,    0x3D,    0x3A,    0x00,    0xD4,    0x03,    0xC4,    0x3F,    0x54,
    0x39,    0x3D,    0x00,    0x78,    0x14,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x08,    0x00,    0x00,    0x00,    0x00,    0x00,    0xA0,    0x54,    0x98,    0x00,
    0x00,    0x11,    0x11,    0x11,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x30,    0x80,    0xC0,    0x00,    0x00,    0x00,    0x04,    0x00,
    0x00,    0x05,    0xFA,    0x80,    0xBB,    0xBB,    0xBB,    0xBB,    0x07,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x04,    0x00,    0x00,
    0x00,    0x00,    0xFF,    0xC6,    0x00,    0x0C,    0x39,    0xC8,    0xA0,    0xAA,
    0x44,    0x00,    0x20,    0x00,    0x80,    0x00,    0x80,    0x09,    0x03,    0x14,
    0x7F,    0x07,    0x00,    0x00,    0x00,    0x00,    0x00,    0x80,    0x7B,    0x80,
    0x00,    0xF0,    0x3F,    0x0F,    0x01,    0x60,    0x70,    0x09,    0x08
};

/**********************************************************************************************************************
*
* power on reset condition.
*
**********************************************************************************************************************/

ENTR_uint8 entr_cfgIdleRegs[ENTR_cfgRegCnt] = {
    0x00,    0x00,    0x01,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x01,    0x00,    0x00,    0x00,    0x00,    0x00,    0x0C,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x20,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x01,
    0x80,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,
    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00,    0x00
};

/**********************************************************************************************************************
*
* channel filter registers
*
**********************************************************************************************************************/

ENTR_uint8 entr_chFltr_6MHz[ENTR_cfgChFltrRegCnt] = {
    0x05,    0xFF,    0xE8,    0xE4,    0x09,    0x1D,    0xF1,    0xD3,    0x11,    0x00,
    0x40,    0x00,    0xED,    0xBF,    0xA6,    0x3F,    0x16,    0x00,    0x7B,    0x00,
    0xE8,    0x3F,    0x57,    0x3F,    0x1A,    0x00,    0xE8,    0x00,    0xE4,    0x3F,
    0xB9,    0x3E,    0x1D,    0x00,    0xEA,    0x01,    0xE2,    0x3F,    0xAA,    0x3C,
    0x1E,    0x00,    0x3C,    0x0A,
};

ENTR_uint8 entr_chFltr_7MHz[ENTR_cfgChFltrRegCnt] = {
    0x00,    0xFC,    0xEA,    0xEE,    0x11,    0x0B,    0xE2,    0x00,    0x2B,    0x00,
    0xE8,    0x3F,    0xD1,    0xBF,    0x3D,    0x00,    0x20,    0x00,    0x9A,    0x3F,
    0x0E,    0x00,    0x85,    0x00,    0xA1,    0x3F,    0x80,    0x3F,    0xD0,    0x00,
    0x39,    0x00,    0xB0,    0x3E,    0x7B,    0x00,    0xC8,    0x01,    0xF2,    0x3D,
    0xE3,    0x3D,    0x75,    0x08
};

ENTR_uint8 entr_chFltr_8MHz[ENTR_cfgChFltrRegCnt] = {
    0x00,    0x00,    0xF9,    0x0D,    0x22,    0xF3,    0xF2,    0x2A,    0xE8,    0x3F,
    0xDD,    0x3F,    0x4B,    0x40,    0xDF,    0x3F,    0xBA,    0x3F,    0x7D,    0x00,
    0xD7,    0x3F,    0x7F,    0x3F,    0xCB,    0x00,    0xCF,    0x3F,    0x1A,    0x3F,
    0x4F,    0x01,    0xC9,    0x3F,    0x46,    0x3E,    0x7C,    0x02,    0xC5,    0x3F,
    0x2D,    0x3B,    0x1B,    0x0A,
};


/****************************************************************************************************************
* rssi data
*
* rssiGainMin = gain at first entry in lut
* rssiLut containes agc accumultor values
* rssiStep = corresponding gain step for each accum value in lut.
*
* gain = gainMin + i * rssiStep, where i = index of closest lut entry to current accum value.
*
* Too avoid passing multi-dim arrays as function parameters, pass: lut[], lutOffset and lutLen to to array search
* routine. The array index is then lutOffset*lutLen+i, i=0 to lutlen.
* lutOffset = ENTRrfAttMode (0,1,2)
* lutOffset = ENTR_broadcastMedium (0,1)
* lutOffset = 0 (for digGainRssiLut)
*
****************************************************************************************************************/

ENTR_sint16 rssiRfAttMin[3] = {-234, -306, -360};
ENTR_sint16 rssiRfAttStep[3] = {13, 17, 20}; /* tenths of a dB, indexed with ENTRrfAttMode */
ENTR_sint16 rssiRfAttLen = 19;
ENTR_sint16 rssiRfAttLut[57] = {
    0x000, 0x314, 0x391, 0x400, 0x46E, 0x4D2, 0x532, 0x582, 0x5DC, 0x632, 0x68C, 0x6EB, 0x746, 0x7AF, 0x828, 0x8BE, 0x964, 0xA96, 0xFFF,
    0x000, 0x264, 0x396, 0x44B, 0x4D2, 0x54B, 0x5C8, 0x66E, 0x700, 0x78C, 0x800, 0x878, 0x900, 0x991, 0xA7D, 0xB4B, 0xC2D, 0xDAF, 0xFFF,
    0x000, 0x20A, 0x396, 0x432, 0x4B4, 0x532, 0x5A0, 0x61E, 0x6A0, 0x778, 0x7B9, 0x83C, 0x8BE, 0x946, 0xA0A, 0xB00, 0xBF0, 0xD64, 0xFFF
};

ENTR_sint16 rssiDigGainMin = -200;
ENTR_sint16 rssiDigGainStep = 20;
ENTR_sint16 rssiDigGainLen = 19;
ENTR_sint16 rssiDigGainLut[19] = {
    0x0033,    0x0040, 0x0051, 0x0066, 0x0081, 0x00A2, 0x00CC, 0x0101, 0x0143,
    0x0197, 0x0200, 0x0285, 0x032B,    0x03FE,    0x0506, 0x0653, 0x07F6, 0x0A06, 0x0C9F
};

/*********************************************************************
* indexed by ENTR_modulation
* DVB_C, J83B, DVB-T, ATSC, ISDB_T, DMB_T, PAL, NTSC, SECAM, SECAM_L
*********************************************************************/
ENTR_sint16 rssiPar[10] = {0,0,0,0,0,0,0,0,0,0};

/*************************************
* indexed with ENTR_lnaModeStatus
* Value set empiricaly at Pin = -50 dBm
*************************************/
ENTR_sint16 rssiLnaGain[3] = {-23, -10, -17}; //{14, 27, 19};

/****************************************************************************************************************
* end of rssi data
****************************************************************************************************************/

/* ========================================================================= */

ENTR_return entr_user_delay(ENTR_uint8 delayMsec)
{
    //    task_delay(delayMsec*ST_GetClocksPerSecondLow()/1000);
    task_delay(delayMsec);
    return ENTR_ok;
}



/* ========================================================================= */

ENTR_return entr_cfgIcRev (entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 dieRev;
    ENTR_uint8 dieCfg;

    retVal = entr_usrRegRd (tunerCfg, ENTR_dieRev_reg, ENTR_dieRev_mask, &dieRev);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegRd (tunerCfg, ENTR_dieCfg_reg, ENTR_dieCfg_mask, &dieCfg);
    if (retVal != ENTR_ok)
        return retVal;

    if ((dieRev != 0) & (dieCfg != 32))
        return ENTR_err_unsuported_ic_rev;

    return ENTR_ok;
}

/**
 * entr_cfgIc()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success.
 *
 * @note Called by entr_set_chan().
 */

ENTR_return entr_cfgIc (entr_config *tunerCfg)
{
    ENTR_return retVal;

    /* @doc execute before entr_cfgBcMedium and entr_cfgMod */
    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) ||
        (tunerCfg->modulation != tunerCfg->tunerData.modulation) ||
        (tunerCfg->broadcastMedium != tunerCfg->tunerData.broadcastMedium)) {
        retVal = entr_cfgModOrBcMed (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->broadcastMedium != tunerCfg->tunerData.broadcastMedium)) {
        retVal = entr_cfgBcMedium (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->modulation != tunerCfg->tunerData.modulation)) {
        retVal = entr_cfgMod (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->rfAttMode != tunerCfg->tunerData.rfAttMode)) {
        retVal = entr_cfgRfAtten (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->specInvSel != tunerCfg->tunerData.specInvSel)) {
        if (tunerCfg->specInvSel != ENTR_specInvSel_on)
            retVal = entr_usrRegWr(tunerCfg, ENTR_specInv_reg, ENTR_specInv_mask, ENTR_specInv_on);
        else
            retVal = entr_usrRegWr(tunerCfg, ENTR_specInv_reg, ENTR_specInv_mask, ENTR_specInv_off);
        if (retVal != ENTR_ok)
            return retVal;
        tunerCfg->tunerData.specInvSel = tunerCfg->specInvSel;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->ifBw != tunerCfg->tunerData.ifBw)) {
        retVal = entr_cfgBw (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->FifHz != tunerCfg->tunerData.FifHz )) {
        retVal = entr_freqSetFifHz (tunerCfg, tunerCfg->FifHz);
        if (retVal != ENTR_ok)
            return retVal;
        tunerCfg->tunerData.FifHz = tunerCfg->FifHz;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->ifOutputSel != tunerCfg->tunerData.ifOutputSel)) {
        if (tunerCfg->ifOutputSel == ENTR_ifOutputSel_1) {
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifOut2_reg, ENTR_ifOut2_mask, ENTR_ifOut2_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifOut1_reg, ENTR_ifOut1_mask, ENTR_ifOut1_on);
            if (retVal != ENTR_ok)
                return retVal;
        }
        else {
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifOut1_reg, ENTR_ifOut1_mask, ENTR_ifOut1_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifOut2_reg, ENTR_ifOut2_mask, ENTR_ifOut2_on);
            if (retVal != ENTR_ok)
                return retVal;
        }
        tunerCfg->tunerData.ifOutputSel = tunerCfg->ifOutputSel;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->ifOutPutLevel != tunerCfg->tunerData.ifOutPutLevel)) {
        retVal = entr_cfgIfOutPutLevel(tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
    }

    if ((tunerCfg->tunerData.tunerCfg == ENTR_tunerCfg_no) || (tunerCfg->agcCtrlMode != tunerCfg->tunerData.agcCtrlMode)) {
        if (tunerCfg->agcCtrlMode == ENTR_agcCtrlMode_internal)
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcSrc_reg, ENTR_ifAgcSrc_mask, ENTR_ifAgcSrc_useB2wFnlAgc);
        else
            retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcSrc_reg, ENTR_ifAgcSrc_mask, ENTR_ifAgcSrc_useDemodFnlAgc);
        if (retVal != ENTR_ok)
            return retVal;
        tunerCfg->tunerData.agcCtrlMode = tunerCfg->agcCtrlMode;
    }

    tunerCfg->tunerData.tunerCfg = ENTR_tunerCfg_yes;

    return ENTR_ok;
}

/**
 * entr_cfgBcMedium()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called entr_cfgIc only. adjust settings for terrestrial or cable operation.
 */
ENTR_return entr_cfgBcMedium (entr_config *tunerCfg)
{
    ENTR_return retVal;

    if (tunerCfg->broadcastMedium == ENTR_broadcastMedium_ota) {
        retVal = entr_usrRegWr(tunerCfg, ENTR_detLvlCtrl_reg, ENTR_detLvlCtrl_mask, ENTR_detLvlCtrl_rangeM30M27);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_on);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ENTR_ifAgcLoopBw_BW_2p4Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_rfAgcTop_reg, ENTR_rfAgcTop_mask, ENTR_rfAgcTop_Vdet_0p26);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_dcILoopBw_reg, ENTR_dcILoopBw_mask, ENTR_dcILoopBw_BW100Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_dcQLoopBw_reg, ENTR_dcQLoopBw_mask, ENTR_dcQLoopBw_BW100Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_terLoNoise);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_digAgcBckOff_reg, ENTR_digAgcBckOff_mask, ENTR_digAgcBckOff_neg6dB);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ENTR_ifAgcLoopBw_BW_2p4Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_digAgcBW_reg, ENTR_digAgcBW_mask, ENTR_digAgcBW_BW90Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_rfAgcLoopBw_reg, ENTR_rfAgcLoopBw_mask, ENTR_rfAgcLoopBw_BW40Hz);
        if (retVal != ENTR_ok)
            return retVal;
        switch (tunerCfg->modulation) {
            case ENTR_modulation_PAL:
            case ENTR_modulation_NTSC:
            case ENTR_modulation_SECAM:
            case ENTR_modulation_SECAM_L:
                retVal = entr_usrRegWr (tunerCfg, ENTR_bbHysDelta_reg, ENTR_bbHysDelta_mask, ENTR_bbHysDelta_window64);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysDelta_reg, ENTR_rfHysDelta_mask, ENTR_rfHysDelta_window64);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysErrMax_reg, ENTR_rfHysErrMax_mask, ENTR_rfHysErrMax_RFErrMx288);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_dcHysADCMax_reg, ENTR_dcHysADCMax_mask, ENTR_dcHysADCMax_ADCMx1408);
                if (retVal != ENTR_ok)
                    return retVal;
                break;
            default:
                break;
        }
        tunerCfg->lnaModeStatus = ENTR_lnaModeStatus_ota_ln;
    }
    else {
        retVal = entr_usrRegWr(tunerCfg, ENTR_detLvlCtrl_reg, ENTR_detLvlCtrl_mask, ENTR_detLvlCtrl_rangeM26M23);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_off);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ENTR_ifAgcLoopBw_BW_0p6Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_rfAgcTop_reg, ENTR_rfAgcTop_mask, ENTR_rfAgcTop_Vdet_0p24);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_dcILoopBw_reg, ENTR_dcILoopBw_mask, ENTR_dcILoopBw_BW25Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_dcQLoopBw_reg, ENTR_dcQLoopBw_mask, ENTR_dcQLoopBw_BW25Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_cab);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_digAgcBckOff_reg, ENTR_digAgcBckOff_mask, ENTR_digAgcBckOff_neg2p5dB);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ENTR_ifAgcLoopBw_BW_0p6Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_digAgcBW_reg, ENTR_digAgcBW_mask, ENTR_digAgcBW_BW22p5Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr(tunerCfg, ENTR_rfAgcLoopBw_reg, ENTR_rfAgcLoopBw_mask, ENTR_rfAgcLoopBw_BW10Hz);
        if (retVal != ENTR_ok)
            return retVal;
        switch (tunerCfg->modulation) {
            case ENTR_modulation_PAL:
            case ENTR_modulation_NTSC:
            case ENTR_modulation_SECAM:
            case ENTR_modulation_SECAM_L:
                retVal = entr_usrRegWr (tunerCfg, ENTR_bbHysDelta_reg, ENTR_bbHysDelta_mask, ENTR_bbHysDelta_window16);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysDelta_reg, ENTR_rfHysDelta_mask, ENTR_rfHysDelta_window16);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysErrMax_reg, ENTR_rfHysErrMax_mask, ENTR_rfHysErrMax_RFErrMx96);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_dcHysADCMax_reg, ENTR_dcHysADCMax_mask, ENTR_dcHysADCMax_ADCMx1792);
                if (retVal != ENTR_ok)
                    return retVal;
                break;
            default:
                break;
        }
        tunerCfg->lnaModeStatus = ENTR_lnaModeStatus_cable;
    }

    retVal = entr_ifSetAccTop(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    tunerCfg->tunerData.broadcastMedium = tunerCfg->broadcastMedium;

    return ENTR_ok;
}

/**
 * entr_cfgRfAtten()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called entr_cfgIc only.
 */
ENTR_return entr_cfgRfAtten (entr_config *tunerCfg)
{
    ENTR_return retVal;

    switch (tunerCfg->rfAttMode) {
        case ENTR_rfAttMode_seriesPin_shuntPin:
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinCtrl_reg, ENTR_sePinCtrl_mask, ENTR_sePinCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinCtrl_reg, ENTR_shPinCtrl_mask, ENTR_shPinCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttCtrl_reg, ENTR_iAttCtrl_mask, ENTR_iAttCtrl_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinDacCtrl_reg, ENTR_sePinDacCtrl_mask, ENTR_sePinDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinDacCtrl_reg, ENTR_shPinDacCtrl_mask, ENTR_shPinDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttDacCtrl_reg, ENTR_iAttDacCtrl_mask, ENTR_iAttDacCtrl_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitLo_reg, ENTR_shPinLimitLo_mask, ENTR_cfgShPinLimitLo_Cablel);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitHi_reg, ENTR_shPinLimitHi_mask, ENTR_cfgShPinLimitHi_Cablel);
            if (retVal != ENTR_ok)
                return retVal;
            break;
        case ENTR_rfAttMode_seriesPin_internalAtt:
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinCtrl_reg, ENTR_sePinCtrl_mask, ENTR_sePinCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinCtrl_reg, ENTR_shPinCtrl_mask, ENTR_shPinCtrl_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttCtrl_reg, ENTR_iAttCtrl_mask, ENTR_iAttCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinDacCtrl_reg, ENTR_sePinDacCtrl_mask, ENTR_sePinDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinDacCtrl_reg, ENTR_shPinDacCtrl_mask, ENTR_shPinDacCtrl_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttDacCtrl_reg, ENTR_iAttDacCtrl_mask, ENTR_iAttDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            break;
        case ENTR_rfAttMode_seriesPin_shuntPin_internalAtt:
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinCtrl_reg, ENTR_sePinCtrl_mask, ENTR_sePinCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinCtrl_reg, ENTR_shPinCtrl_mask, ENTR_shPinCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttCtrl_reg, ENTR_iAttCtrl_mask, ENTR_iAttCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_sePinDacCtrl_reg, ENTR_sePinDacCtrl_mask, ENTR_sePinDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinDacCtrl_reg, ENTR_shPinDacCtrl_mask, ENTR_shPinDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_iAttDacCtrl_reg, ENTR_iAttDacCtrl_mask, ENTR_iAttDacCtrl_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitLo_reg, ENTR_shPinLimitLo_mask, ENTR_cfgShPinLimitLo_Ter);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitHi_reg, ENTR_shPinLimitHi_mask, ENTR_cfgShPinLimitHi_Ter);
            if (retVal != ENTR_ok)
                return retVal;
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    tunerCfg->tunerData.rfAttMode = tunerCfg->rfAttMode;

    return ENTR_ok;
}

/**
 * entr_cfgBw()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called entr_cfgIc only. sets analog if bw and channel filter.
 */
ENTR_return entr_cfgBw (entr_config *tunerCfg)
{
    ENTR_return retVal;

    if ((tunerCfg->modulation == ENTR_modulation_J83_Annex_B) & (tunerCfg->ifBw == ENTR_ifBw_6MHz)) {
        retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_6MHz, ENTR_cfgChFltrRegCnt);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdWr_reg, ENTR_ifCapWdWr_mask, tunerCfg->tunerData.ifBwWd[ENTR_ifCalBw_8MHz]);
        if (retVal != ENTR_ok)
            return retVal;
    }
    else if ((tunerCfg->modulation == ENTR_modulation_DVB_C) & (tunerCfg->ifBw == ENTR_ifBw_7MHz)) {
        retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_7MHz, ENTR_cfgChFltrRegCnt);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdWr_reg, ENTR_ifCapWdWr_mask, tunerCfg->tunerData.ifBwWd[ENTR_ifCalBw_8MHz]);
        if (retVal != ENTR_ok)
            return retVal;
    }
    else if ((tunerCfg->modulation == ENTR_modulation_DVB_C) & (tunerCfg->ifBw == ENTR_ifBw_8MHz)) {
        retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_8MHz, ENTR_cfgChFltrRegCnt);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdWr_reg, ENTR_ifCapWdWr_mask, tunerCfg->tunerData.ifBwWd[ENTR_ifCalBw_10MHz]);
        if (retVal != ENTR_ok)
            return retVal;
    }
    else {
        if (tunerCfg->ifBw == ENTR_ifBw_6MHz)
            retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_6MHz, ENTR_cfgChFltrRegCnt);
        else if (tunerCfg->ifBw == ENTR_ifBw_7MHz)
            retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_7MHz, ENTR_cfgChFltrRegCnt);
        else
            retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgChFltrReg1, entr_chFltr_8MHz, ENTR_cfgChFltrRegCnt);
        if (retVal != ENTR_ok)
            return retVal;

        retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdWr_reg, ENTR_ifCapWdWr_mask, tunerCfg->tunerData.ifBwWd[tunerCfg->ifBw]);
        if (retVal != ENTR_ok)
            return retVal;
    }

    tunerCfg->tunerData.ifBw = tunerCfg->ifBw;

    return ENTR_ok;
}

/**
 * entr_cfgifOutPutLevel()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_unknown for bad param
 *          ENTR_err_rw otherwise.
 *
 * @note Called entr_cfgIc only.
 */
ENTR_return entr_cfgIfOutPutLevel (entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 ifOuputLvl;

    switch (tunerCfg->ifOutPutLevel) {
        case ENTR_ifOutPutLevel_neg3p2_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg3p2;
            break;
        case ENTR_ifOutPutLevel_neg2p8_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg2p8;
            break;
        case ENTR_ifOutPutLevel_neg2p3_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg2p3;
            break;
        case ENTR_ifOutPutLevel_neg2p0_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg2;
            break;
        case ENTR_ifOutPutLevel_neg1p6_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg1p6;
            break;
        case ENTR_ifOutPutLevel_neg1p0_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg1;
            break;
        case ENTR_ifOutPutLevel_neg0p5_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_neg0p5;
            break;
        case ENTR_ifOutPutLevel_0_dB_nominal:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_Nom;
            break;
        case ENTR_ifOutPutLevel_0p6_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos0p6;
            break;
        case ENTR_ifOutPutLevel_1p2_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos1p2;
            break;
        case ENTR_ifOutPutLevel_1p8_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos1p8;
            break;
        case ENTR_ifOutPutLevel_2p5_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos2p5;
            break;
        case ENTR_ifOutPutLevel_3p3_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos3p3;
            break;
        case ENTR_ifOutPutLevel_4p0_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos4;
            break;
        case ENTR_ifOutPutLevel_5p0_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos5;
            break;
        case ENTR_ifOutPutLevel_6p0_dB:
            ifOuputLvl = ENTR_ifLevel_FSRngRed_pos6;
            break;
        default:
            return ENTR_err_unknown;
    }

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifLevel_reg, ENTR_ifLevel_mask, ifOuputLvl);
    if (retVal != ENTR_ok)
        return retVal;

    tunerCfg->tunerData.ifOutPutLevel = tunerCfg->ifOutPutLevel;

    return ENTR_ok;
}

/**
 * entr_cfgMod()
 *
 * @param[in,out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called entr_cfgIc only. cfg's ic for digital or analog modulation.
 */
ENTR_return entr_cfgMod (entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 bbHysMode;
    ENTR_uint8 dcHysMode;
    ENTR_uint8 digAgcInSel;
    ENTR_uint8 rfHysMode;
    ENTR_uint8 IQGainBwSlw;

    switch (tunerCfg->modulation) {
        case ENTR_modulation_DVB_C:
        case ENTR_modulation_J83_Annex_B:
        case ENTR_modulation_DVB_T:
        case ENTR_modulation_ATSC:
        case ENTR_modulation_ISDB_T:
        case ENTR_modulation_DMB_T:
            rfHysMode = ENTR_rfHysMode_off;
            bbHysMode = ENTR_bbHysMode_off;
            dcHysMode = ENTR_dcHysMode_off;
            digAgcInSel = ENTR_digAgcInSel_Translator_Out;
            IQGainBwSlw = ENTR_IQGainBwSlw_BW3p6Hz;
            break;
        case ENTR_modulation_PAL:
        case ENTR_modulation_NTSC:
        case ENTR_modulation_SECAM:
        case ENTR_modulation_SECAM_L:
            rfHysMode = ENTR_rfHysMode_on;
            bbHysMode = ENTR_bbHysMode_on;
            dcHysMode = ENTR_dcHysMode_on;
            digAgcInSel = ENTR_digAgcInSel_Interp_Out;
            IQGainBwSlw = ENTR_IQGainBwSlw_BW1p8Hz;
            break;
        default:
            return ENTR_err_unknown; /* should never happen */
    }

    retVal = entr_usrRegWr (tunerCfg, ENTR_bbHysMode_reg, ENTR_bbHysMode_mask, bbHysMode);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_dcHysMode_reg, ENTR_dcHysMode_mask, dcHysMode);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_digAgcInSel_reg, ENTR_digAgcInSel_mask, digAgcInSel);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysMode_reg, ENTR_rfHysMode_mask, rfHysMode);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_IQGainBwSlw_reg, ENTR_IQGainBwSlw_mask, IQGainBwSlw);
    if (retVal != ENTR_ok)
        return retVal;

    tunerCfg->tunerData.modulation = tunerCfg->modulation;

    return ENTR_ok;
}

ENTR_return entr_cfgModOrBcMed (entr_config *tunerCfg)
{
    ENTR_return retVal;

    switch (tunerCfg->modulation) {
        case ENTR_modulation_DVB_C:
        case ENTR_modulation_J83_Annex_B:
        case ENTR_modulation_DVB_T:
        case ENTR_modulation_ATSC:
        case ENTR_modulation_ISDB_T:
        case ENTR_modulation_DMB_T:
            retVal = entr_usrRegWr (tunerCfg, ENTR_bbAdcBckOff_reg, ENTR_bbAdcBckOff_mask, ENTR_bbAdcBckOff_BkOffFrmFS_6dB);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_digDCbwFst_reg, ENTR_digDCbwFst_mask, ENTR_digDCbwFst_BW14p6Hz);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_IQPhBwFst_reg, ENTR_IQPhBwFst_mask, ENTR_IQPhBwFst_BW1p8Hz);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_IQGainBwFst_reg, ENTR_IQGainBwFst_mask, ENTR_IQGainBwFst_BW3p6Hz);
            if (retVal != ENTR_ok)
                return retVal;
            break;
        case ENTR_modulation_PAL:
        case ENTR_modulation_NTSC:
        case ENTR_modulation_SECAM:
        case ENTR_modulation_SECAM_L:
            if (tunerCfg->broadcastMedium == ENTR_broadcastMedium_cable) {
                retVal = entr_usrRegWr (tunerCfg, ENTR_bbAdcBckOff_reg, ENTR_bbAdcBckOff_mask, ENTR_bbAdcBckOff_BkOffFrmFS_3dB);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_digDCbwFst_reg, ENTR_digDCbwFst_mask, ENTR_digDCbwFst_BW29Hz);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_IQPhBwFst_reg, ENTR_IQPhBwFst_mask, ENTR_IQPhBwFst_BW14p6Hz);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_IQGainBwFst_reg, ENTR_IQGainBwFst_mask, ENTR_IQGainBwFst_BW14p6Hz);
                if (retVal != ENTR_ok)
                    return retVal;
            }
            else {
                retVal = entr_usrRegWr (tunerCfg, ENTR_bbAdcBckOff_reg, ENTR_bbAdcBckOff_mask, ENTR_bbAdcBckOff_BkOffFrmFS_6dB);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_digDCbwFst_reg, ENTR_digDCbwFst_mask, ENTR_digDCbwFst_BW117Hz);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_IQPhBwFst_reg, ENTR_IQPhBwFst_mask, ENTR_IQPhBwFst_BW58Hz);
                if (retVal != ENTR_ok)
                    return retVal;
                retVal = entr_usrRegWr (tunerCfg, ENTR_IQGainBwFst_reg, ENTR_IQGainBwFst_mask, ENTR_IQGainBwFst_BW58Hz);
                if (retVal != ENTR_ok)
                    return retVal;
            }
            break;
        default:
            return ENTR_err_unknown; /* should never happen */
    }

    return ENTR_ok;
}
/**
 * entr_cfgBase()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called by entr_active() at end of subroutine.
 *       Base configuration for normal opperation. Set
 *       after calibration in entr_active(). Configuration
 *       is completed in entr_set_chan (supports multi
 *       mode operation).
 */
ENTR_return entr_cfgBase (entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgWrReg1, entr_cfgBaseRegs, ENTR_cfgRegCnt);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr(tunerCfg, ENTR_reset_reg, ENTR_reset_mask, ENTR_reset_Reset);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_reset_reg, ENTR_reset_mask, ENTR_reset_NotReset);
    if (retVal != ENTR_ok)
        return retVal;

    /* begin, initialize rf detector */
    retVal = entr_usrRegWr(tunerCfg, ENTR_rfDetCalFrc_reg, ENTR_rfDetCalFrc_mask, ENTR_rfDetCalFrc_on);
    if (retVal != ENTR_ok)
        return retVal;
    entr_user_delay(5);
    retVal = entr_usrRegWr(tunerCfg, ENTR_rfDetCalFrc_reg, ENTR_rfDetCalFrc_mask, ENTR_rfDetCalFrc_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_rfDetCalAuto_reg, ENTR_rfDetCalAuto_mask, ENTR_rfDetCalAuto_on);
    if (retVal != ENTR_ok)
        return retVal;
    /* end, initialize rf detector */

    retVal = entr_cfgResolveBbErrCondition(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* remove and change base cfg */
    retVal = entr_usrRegWr(tunerCfg, ENTR_sePinLimitTest_reg, ENTR_sePinLimitTest_mask, ENTR_sePinLimitTest_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitTest_reg, ENTR_shPinLimitTest_mask, ENTR_shPinLimitTest_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_iAttLimitTest_reg, ENTR_iAttLimitTest_mask, ENTR_iAttLimitTest_off);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_cfgIdle()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Called by entr_idle().
 */
ENTR_return entr_cfgIdle (entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgWrReg1, entr_cfgIdleRegs, ENTR_cfgRegCnt);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_cfgIfBwCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note rf atten max atten
 *       lna off
 *       tfBandSel = ttGen
 *       ttPwr = -44dBV
 *       ttSrc = trans
 *       ifVGA2 on
 *       dac off
 */
ENTR_return entr_cfgIfBwCal (entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_cfgWrReg1, entr_cfgIfCalRegs, ENTR_cfgRegCnt);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr(tunerCfg, ENTR_reset_reg, ENTR_reset_mask, ENTR_reset_Reset);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr(tunerCfg, ENTR_reset_reg, ENTR_reset_mask, ENTR_reset_NotReset);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgResolveBbErrCondition(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetFloHz(tunerCfg, 135000000);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr(tunerCfg, ENTR_ttEna_reg, ENTR_ttEna_mask, ENTR_ttEna_on);
    if (retVal != ENTR_ok)
        return retVal;

    //entr_user_delay(150);
    retVal = entr_cfgBbAgcDelay(tunerCfg, 10);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr(tunerCfg, ENTR_ifAgcEna_reg, ENTR_ifAgcEna_mask, ENTR_ifAgcEna_off);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_cfgentr_ifAccTopCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Changes to entr_cfgIfBwCal for entr_ifAccTopCal.
 */
ENTR_return entr_cfgIfAccTopCal (entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ttSrc_reg, ENTR_ttSrc_mask, ENTR_ttSrc_SpinNCO);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_freqSetFspinHz(tunerCfg, ENTR_cfgIfAccTopFttHz_Ref);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_cfgTfCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Changes to entr_cfgentr_ifAccTopCal for entr_tfCal.
 */
ENTR_return entr_cfgTfCal (entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_on);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitLo_reg, ENTR_shPinLimitLo_mask, 255);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_shPinLimitHi_reg, ENTR_shPinLimitHi_mask, 15);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcEna_reg, ENTR_ifAgcEna_mask, ENTR_ifAgcEna_on);
    if (retVal != ENTR_ok)
        return retVal;
    //retVal = entr_usrRegWr (tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_Cal);
    //if (retVal != ENTR_ok)
        //return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_calDetCtrl_reg, ENTR_calDetCtrl_mask, ENTR_calDetCtrl_SM);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_cfgResolveBbErrCondition()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_cfgResolveBbErrCondition(entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 ifVga2Ena;

    retVal = entr_usrRegRd (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, &ifVga2Ena);
    if (retVal != ENTR_ok)
        return retVal;

    if (ifVga2Ena == ENTR_ifVga2Ena_on) {
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_off);
        if (retVal != ENTR_ok)
            return retVal;
        entr_user_delay(10);
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_on);
        if (retVal != ENTR_ok)
            return retVal;
    }
    else {
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_on);
        if (retVal != ENTR_ok)
            return retVal;
        entr_user_delay(10);
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_off);
        if (retVal != ENTR_ok)
            return retVal;
    }

    return ENTR_ok;
}

/**
 * entr_cfgCalDetRead()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 * @param[out] value.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_cfgCalDetRead(entr_config *tunerCfg, ENTR_sint32 *value)
{
    ENTR_return retVal;
    ENTR_uint8 buff[2];
    ENTR_uint8 calDetDone;

    ENTR_uint8 i;

    retVal = entr_usrRegWr(tunerCfg, ENTR_calDetTrig_reg, ENTR_calDetTrig_mask, ENTR_calDetTrig_Trigger);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr(tunerCfg, ENTR_calDetTrig_reg, ENTR_calDetTrig_mask, ENTR_calDetTrig_ClearTrigger);
    if (retVal != ENTR_ok)
        return retVal;
    for (i=0;i<30;i++) {
        retVal = entr_usrRegRd(tunerCfg, ENTR_calDetDone_reg, ENTR_calDetDone_mask, &calDetDone);
        if (retVal != ENTR_ok)
            return retVal;
        if (calDetDone == ENTR_calDetDone_done)
            break;
    }

    if (calDetDone != ENTR_calDetDone_done)
        return ENTR_err_cfgCalDet_timeout;

    retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_calDetRdLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;

    *value = (((ENTR_sint32) (buff[1] & 0x7f))<<8) + (ENTR_sint32) buff[0];

    return ENTR_ok;
}

/**
 * entr_cfgReadTmprAcc()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 * @param[out] tmpr.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_cfgReadTmprAcc(entr_config *tunerCfg, ENTR_sint32 *tmpr)
{
    ENTR_return retVal;

    ENTR_uint8  i;
    ENTR_uint8  buff[2];
    ENTR_sint32 tmprAcc;
    ENTR_uint8  agcAccSelRestore;

    retVal = entr_usrRegRd (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, &agcAccSelRestore);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_RFDetADC);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tmprCtrl_reg, ENTR_tmprCtrl_mask, ENTR_tmprCtrl_TmprPwrOn);
    if (retVal != ENTR_ok)
        return retVal;

    tmprAcc = 0;
    for (i=0; i<4; i++) {
        retVal = entr_user_2wb_rd (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;
        tmprAcc += (((ENTR_sint32) buff[1])<<8) + (ENTR_sint32) buff[0];
    }

    tmprAcc >>= 2;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tmprCtrl_reg, ENTR_tmprCtrl_mask, ENTR_tmprCtrl_TmprPwrOff);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, agcAccSelRestore);
    if (retVal != ENTR_ok)
        return retVal;

    /*** Convert from 2's comp to offset binary */
    if (tmprAcc > 2047)
        *tmpr = tmprAcc - 2048;
    else
        *tmpr = tmprAcc + 2048;

    return ENTR_ok;
}

/**
 * entr_cfgReadTmprAcc()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 * @param[in] timeOutMsec.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_cfgBbAgcDelay(entr_config *tunerCfg, ENTR_uint8 timeOutMsec)
{
    ENTR_return retVal;
    ENTR_sint16 agcErr;
    ENTR_uint8  buff[2];
    ENTR_uint8  i;

    for (i=0;i<timeOutMsec;i++) {
        retVal = entr_user_2wb_rd(tunerCfg->i2c_id,(ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcErrLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;
        agcErr = (ENTR_sint16) (buff[1] & 0x1F);
        agcErr <<= 8;
        agcErr += (ENTR_sint16) buff[0];
        /* agcErr is 13 bits 2's comp */
        if ((agcErr < 40) || (agcErr > 8151))
            break;
        entr_user_delay(1);
    }

    return ENTR_ok;
}

ENTR_return entr_cfgRssi (entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 agcAccSelRestore;
    ENTR_uint8 buff[2];
    ENTR_sint16 acc;
    ENTR_sint16 gain;
    ENTR_sint16 gainTotal;

    /***********************************************************************************************************
    * restore acc select
    ***********************************************************************************************************/
    retVal = entr_usrRegRd (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, &agcAccSelRestore);
    if (retVal != ENTR_ok)
        return retVal;

    /***********************************************************************************************************
    * if gain
    ***********************************************************************************************************/
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_BB);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    acc = (((ENTR_sint16) buff[1])<<8) + ((ENTR_sint16) buff[0]);

    /*
    retVal = entr_cfgRssiSearchLut (tunerCfg, acc, rssiIfGainLut, rssiIfGainStep[tunerCfg->broadcastMedium],
                                    (ENTR_sint16) tunerCfg->broadcastMedium, rssiIfGainLen, &gainTotal);
    */

    /* gain in tenths of a dB */
    if (tunerCfg->broadcastMedium == ENTR_broadcastMedium_cable) {
        gainTotal = (ENTR_sint16) (((ENTR_sint32) acc * 1020) / 10000 - 45);
        if (gainTotal > 300)
            gainTotal = 300;
    }
    else {
        gainTotal = (ENTR_sint16) (((ENTR_sint32) acc * 2050) / 10000 - 102);
        if (gainTotal > 580)
            gainTotal = 580;
    }
    if (gainTotal < 0)
        gainTotal = 0;

    /***********************************************************************************************************
    * rf gain
    ***********************************************************************************************************/
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_RF);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    acc = (((ENTR_sint16) buff[1])<<8) + ((ENTR_sint16) buff[0]);

    retVal = entr_cfgRssiSearchLut(tunerCfg, acc, rssiRfAttLut, rssiRfAttStep[tunerCfg->rfAttMode],
                                   rssiRfAttMin[tunerCfg->rfAttMode], (ENTR_sint16) tunerCfg->rfAttMode,
                                   rssiRfAttLen, &gain);
    if (retVal != ENTR_ok)
        return retVal;
    gainTotal += gain;

    /***********************************************************************************************************
    * dig gain
    ***********************************************************************************************************/
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_DigitalAGC);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    acc = (((ENTR_sint16) buff[1])<<8) + ((ENTR_sint16) buff[0]);

    retVal = entr_cfgRssiSearchLut (tunerCfg, acc, rssiDigGainLut, rssiDigGainStep, rssiDigGainMin, 0, rssiDigGainLen, &gain);
    if (retVal != ENTR_ok)
        return retVal;
    gainTotal += gain;

    /***********************************************************************************************************
    * dig gain
    ***********************************************************************************************************/
    gainTotal += rssiPar[tunerCfg->modulation];

    /***********************************************************************************************************
    * restore acc select
    ***********************************************************************************************************/
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, agcAccSelRestore);
    if (retVal != ENTR_ok)
        return retVal;

    /***********************************************************************************************************
    * calculte gain
    ***********************************************************************************************************/
    tunerCfg->RSSI = rssiLnaGain[tunerCfg->lnaModeStatus] - gainTotal / 10;

    return ENTR_ok;
}

ENTR_return entr_cfgRssiSearchLut (entr_config *tunerCfg, ENTR_sint16 acc, ENTR_sint16 lut[],
                                   ENTR_sint16 lutGainStep, ENTR_sint16 lutMin, ENTR_sint16 lutOffset,
                                   ENTR_sint16 lutLen, ENTR_sint16 *gain)
{

    ENTR_uint8 i;

    if (acc <= lut[lutOffset * lutLen])
        *gain = lutMin;
    else if (acc >= lut[lutOffset * lutLen + lutLen - 1])
        *gain = (lutLen - 1) * lutGainStep + lutMin;
    else {
        for (i=0; i<lutLen; i++) {
            if (acc <= lut[lutOffset * lutLen + i])
                break;
        }
        /* i > 0 */
        if ((acc - lut[lutOffset * lutLen + i - 1]) < (lut[lutOffset * lutLen + i] - acc))
            --i;
        *gain = i * lutGainStep + lutMin;
    }

    return ENTR_ok;
}

ENTR_return entr_cfgHystModeOff (entr_config *tunerCfg)
{

    ENTR_return retVal;

    switch (tunerCfg->modulation) {
        case ENTR_modulation_PAL:
        case ENTR_modulation_NTSC:
        case ENTR_modulation_SECAM:
        case ENTR_modulation_SECAM_L:
            retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysMode_reg, ENTR_rfHysMode_mask, ENTR_rfHysMode_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_bbHysMode_reg, ENTR_bbHysMode_mask, ENTR_bbHysMode_off);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_dcHysMode_reg, ENTR_dcHysMode_mask, ENTR_dcHysMode_off);
            if (retVal != ENTR_ok)
                return retVal;
            return ENTR_ok;
        default:
            return ENTR_ok;
    }
}

ENTR_return entr_cfgHystModeOn (entr_config *tunerCfg)
{

    ENTR_return retVal;

    switch (tunerCfg->modulation) {
        case ENTR_modulation_PAL:
        case ENTR_modulation_NTSC:
        case ENTR_modulation_SECAM:
        case ENTR_modulation_SECAM_L:
            retVal = entr_usrRegWr (tunerCfg, ENTR_rfHysMode_reg, ENTR_rfHysMode_mask, ENTR_rfHysMode_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_bbHysMode_reg, ENTR_bbHysMode_mask, ENTR_bbHysMode_on);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_dcHysMode_reg, ENTR_dcHysMode_mask, ENTR_dcHysMode_on);
            if (retVal != ENTR_ok)
            return ENTR_ok;
        default:
            return ENTR_ok;
    }
}/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrFreq.c
 *
 * @brief
 *
 *   $xRev: 11931 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/

/* lc lut for 24 MHz xtal */
static const ENTR_sint32 ENTR_freqLcLut[49] = {
    2097152, 2096029, 2092662, 2087054, 2079211, 2069141, 2056856, 2042368, 2025693, 2006849,
    1985856, 1962737, 1937516, 1910220, 1880878, 1849523, 1816187, 1780906, 1743718, 1704663,
    1663783, 1621120, 1576722, 1530636, 1482910, 1433597, 1382748, 1330419, 1276665, 1221544,
    1165115, 1107439, 1048576, 988591,  927547,  865509,  802545,  738722,  674107, 608771,
    542783,  476213,  409134,  341617,  273733,  205557,  137160,  68617,   0
};

/**
 * entr_freqSetFxtalAndFcomp()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success, ENTR_err_rw otherwise.
 *
 * @note Check that the xtal freq is in range and then move it into the tuner data structure..
 */
ENTR_return entr_freqSetFxtalAndFcomp(entr_config *tunerCfg)
{
    ENTR_return retVal;

    /*********************************************
    * parameter checked in entrTuner
    * add case statement when more values added
    * currently only ENTR_Fxtal_24MHz defined
    *********************************************/
    tunerCfg->tunerData.FxtalHz = 24000000;

    /*************************************************************************
    * determined b Fxtal (put in case statement).
    * ex 24 MHz:
    * Fspin = 24MHz/2 * Nspin / 2^20 = 24MHz * N / 2^21
    *       = 46875 * 2^9 * N / 2^21
    *       = 46875 * N / 2^12
    * Fspin,max = 2^31-1 / 2^12 too prevent overflow
    *           < 2^31 / 2^12 = 2^19 = 524288
    *
    * Calculating Fspin Multiply first when Fspin < Fspin,max (minimal error).
    * Divide first if Fspin >= Fspin,max
    * (error due to integer divide)
    *
    * For synth, |Fspin,max| ~ 250 KHz
    *************************************************************************/
    tunerCfg->tunerData.FspinMaxHz = 524288;
    tunerCfg->tunerData.FspinN = 4096;
    tunerCfg->tunerData.FspinD = 46875;

    /*************************************************************************
    * ex 24 MHz:
    * Fif = 24MHz*8 * Nspin / 2^16 = 24MHz * N / 2^13
    *     = 46875 * 2^9 * N / 2^13
    *     = 46875 * N / 2^4
    * Fif,max = 2^31-1 / 2^4 too prevent overflow
    *         < 2^31 / 2^4 = 2^27 = 134217728
    *************************************************************************/
    tunerCfg->tunerData.FifMaxHz = 96000000;
    tunerCfg->tunerData.FifN = 16;
    tunerCfg->tunerData.FifD = 46875;

    retVal = entr_usrRegWr (tunerCfg, ENTR_synR_reg, ENTR_synR_mask, 24);
    if (retVal != ENTR_ok)
        return retVal;

    /*********************************************
    * parameter checked in entrTuner
    * add case statement when more values added
    * currently only ENTR_Fcomp_1MHz defined
    *********************************************/
    tunerCfg->tunerData.FcompHz = 1000000;

   return ENTR_ok;
}

/**
 * entr_freqSetFloHz()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] freqHz Frequency to set to.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_freqPllSm_timeout if unable to lock
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqSetFloHz(entr_config *tunerCfg, ENTR_sint32 freqHz)
{
    ENTR_return retVal;

    ENTR_sint32 Np;
    ENTR_sint32 N0;
    ENTR_sint32 N1;
    ENTR_sint32 B;
    ENTR_sint32 A;
    ENTR_sint32 LoDiv;
    ENTR_sint32 LoDivPrime;
    ENTR_sint32 FloHz;
    ENTR_sint32 Flo0Hz;
    ENTR_sint32 Flo1Hz;
    ENTR_sint32 FvcoPrimeHz;
    ENTR_sint32 FspinHz;
    ENTR_sint32 Fspin0Hz;
    ENTR_sint32 Fspin1Hz;

    ENTR_uint8 LoDivR;
    ENTR_uint8 mixMode;
    ENTR_uint8 synNrange;
    ENTR_uint8 curVco;
    ENTR_uint8 dLock;
    ENTR_uint8 aLock;

    ENTR_uint8 SynVtune;

    ENTR_sint16 i;

    if (freqHz >= 435000000) {
        LoDiv = 4;
        LoDivR = ENTR_synLoDiv_div4Phs2;
        mixMode = ENTR_mixMode_phs2;
    }
    else if (freqHz >= 217500000) {
        LoDiv = 8;
        LoDivR = ENTR_synLoDiv_div8Phs4;
        mixMode = ENTR_mixMode_phs4;
    }
    else if (freqHz >= 108750000) {
        LoDiv = 16;
        LoDivR = ENTR_synLoDiv_div16Phs8;
        mixMode = ENTR_mixMode_phs8;
    }
    else if (freqHz >= 54375000) {
        LoDiv = 32;
        LoDivR = ENTR_synLoDiv_div32Phs8;
        mixMode = ENTR_mixMode_phs8;
    }
    else {
        LoDiv = 64;
        LoDivR = ENTR_synLoDiv_div64Phs8;
        mixMode = ENTR_mixMode_phs8;
    }
    retVal = entr_usrRegWr (tunerCfg, ENTR_synLoDiv_reg, ENTR_synLoDiv_mask, LoDivR);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_mixMode_reg, ENTR_mixMode_mask, mixMode);
    if (retVal != ENTR_ok)
        return retVal;

    LoDivPrime = LoDiv / 2;
    N0 = freqHz * LoDivPrime / tunerCfg->tunerData.FcompHz;
    N1 = N0 + 1;
    Flo0Hz = N0 * tunerCfg->tunerData.FcompHz / LoDivPrime;
    Flo1Hz = N1 * tunerCfg->tunerData.FcompHz / LoDivPrime;
    Fspin0Hz = freqHz - Flo0Hz;
    Fspin1Hz = freqHz - Flo1Hz;

    if (Fspin0Hz < -Fspin1Hz) {
        Np = N0;
        FspinHz = Fspin0Hz;
        FloHz = Flo0Hz;
    }
    else {
        Np = N1;
        FspinHz = Fspin1Hz;
        FloHz = Flo1Hz;
    }

    B = Np / 16;
    A = Np - B * 16;
    FvcoPrimeHz = FloHz * LoDivPrime;

    retVal = entr_usrRegWr (tunerCfg, ENTR_synN_reg, ENTR_synN_mask, (ENTR_uint8) B);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synA_reg, ENTR_synA_mask, (ENTR_uint8) A);
    if (retVal != ENTR_ok)
        return retVal;

    if (Np < 2070 / 2)
        synNrange = ENTR_synNrange_range0;
    else if (Np < 2461 / 2)
        synNrange = ENTR_synNrange_range1;
    else if (Np < 2926 / 2)
        synNrange = ENTR_synNrange_range2;
    else
        synNrange = ENTR_synNrange_range3;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synNrange_reg, ENTR_synNrange_mask, synNrange);
    if (retVal != ENTR_ok)
        return retVal;

    /**************************************************************************************************/
    /* @doc Force current vco to vco8 */
    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmClk_reg, ENTR_synSmClk_mask, ENTR_synSmClk_vcoSM3);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synVcoWr_reg, ENTR_synVcoWr_mask, ENTR_synVcoWr_VCO8);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_ena);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_dis);
    if (retVal != ENTR_ok)
        return retVal;
    entr_user_delay(2);
    /* @doc Set the starting vco to vco0 */
    retVal = entr_usrRegWr (tunerCfg, ENTR_synVcoWr_reg, ENTR_synVcoWr_mask, ENTR_synVcoWr_VCO0);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmClk_reg, ENTR_synSmClk_mask, ENTR_synSmClk_vcoSM2);
    if (retVal != ENTR_ok)
        return retVal;
    /**************************************************************************************************/

    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_ena);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_dis);
    if (retVal != ENTR_ok)
        return retVal;

    for (i=0; i<ENTR_freqLoSmTimeOutMsec; i++) {
        /* read analog and digital lock, break if locked. */
        retVal = entr_usrRegRd (tunerCfg, ENTR_synAnLock_reg, ENTR_synAnLock_mask, &aLock);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_synDigLock_reg, ENTR_synDigLock_mask, &dLock);
        if (retVal != ENTR_ok)
            return retVal;
        if ((aLock == ENTR_synAnLock_on) && (dLock == ENTR_synDigLock_on))
            break;
        entr_user_delay(1);
    }

    /* trigger a second time if not locked */
    if ((aLock != ENTR_synAnLock_on) || (dLock != ENTR_synDigLock_on)) {
        retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_ena);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_synSmTrig_reg, ENTR_synSmTrig_mask, ENTR_synSmTrig_dis);
        if (retVal != ENTR_ok)
            return retVal;
        for (i=0; i<ENTR_freqLoSmTimeOutMsec; i++) {
            retVal = entr_usrRegRd (tunerCfg, ENTR_synAnLock_reg, ENTR_synAnLock_mask, &aLock);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegRd (tunerCfg, ENTR_synDigLock_reg, ENTR_synDigLock_mask, &dLock);
            if (retVal != ENTR_ok)
                return retVal;
            if ((aLock == ENTR_synAnLock_on) && (dLock == ENTR_synDigLock_on))
                break;
            entr_user_delay(1);
        }
    }

    if ((aLock != ENTR_synAnLock_on) || (dLock != ENTR_synDigLock_on))
        return ENTR_err_freqPllSm_timeout;

   /* @doc Optimize phase noise on vco8 */
   retVal = entr_usrRegRd (tunerCfg, ENTR_synVcoRd_reg, ENTR_synVcoRd_mask, &curVco);
   if (retVal != ENTR_ok)
      return retVal;

   if (curVco == ENTR_synVcoWr_VCO8) {
      retVal = entr_usrRegRd (tunerCfg, ENTR_synVtuneRd_reg, ENTR_synVtuneRd_mask, &SynVtune);
      if (retVal != ENTR_ok)
         return retVal;
      /* works because SynVtuneLsb = 0 */
      if (SynVtune > 4) {
         retVal = entr_usrRegWr (tunerCfg, ENTR_synCPcur_reg, ENTR_synCPcur_mask, ENTR_synCPcur_mA1p6);
         if (retVal != ENTR_ok)
            return retVal;
      }
      else {
         retVal = entr_usrRegWr (tunerCfg, ENTR_synCPcur_reg, ENTR_synCPcur_mask, ENTR_synCPcur_mA0p8);
         if (retVal != ENTR_ok)
            return retVal;
      }
   }

   /* @doc Save FloHz in tuner data structure. */
   tunerCfg->tunerData.FloHz = FloHz;

   retVal = entr_freqCfgTtGen(tunerCfg, (ENTR_uint8) LoDiv);
   if (retVal != ENTR_ok)
      return retVal;

    return ENTR_ok;
}

/**
 * entr_freqSetFspinHz()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] FspinHz
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqSetFspinHz(entr_config *tunerCfg, ENTR_sint32 FspinHz)
{
   ENTR_return retVal;
    ENTR_sint32 N;
   ENTR_sint32 Neg;
    ENTR_uint8  buff[3];

   if (FspinHz < 0) {
      Neg = 0x100000;
      FspinHz *= -1;
   }
   else
      Neg = 0;

   if (FspinHz < tunerCfg->tunerData.FspinMaxHz) {
      N = FspinHz * tunerCfg->tunerData.FspinN;
      N /= tunerCfg->tunerData.FspinD;
      if (Neg != 0)
         N = Neg - N;
   }
   else {
      /* will be rounded it FspinHz % tunerCfg->tunerData.FspinD != 0 */
      N = FspinHz /  tunerCfg->tunerData.FspinD;
      N *= tunerCfg->tunerData.FspinN;
   }

    retVal = entr_user_2wb_rd (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_rotHi_reg, buff,1);
    if (retVal != ENTR_ok)
      return retVal;

    buff[2] = (buff[0] & 0xf0) | ((ENTR_uint8) (N>>16));
    buff[0] = (ENTR_uint8) N;
    buff[1] = (ENTR_uint8) (N>>8);
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_rotLo_reg, buff, 3);
    if (retVal != ENTR_ok)
      return retVal;

   if (FspinHz < tunerCfg->tunerData.FspinMaxHz) {
      tunerCfg->tunerData.FspinHz = N * tunerCfg->tunerData.FspinD;
      tunerCfg->tunerData.FspinHz /= tunerCfg->tunerData.FspinN;
   }
   else {
      tunerCfg->tunerData.FspinHz = N  / tunerCfg->tunerData.FspinN;
      tunerCfg->tunerData.FspinHz *= tunerCfg->tunerData.FspinD;
   }

    return ENTR_ok;
}

/**
 * entr_freqSetFifHz()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] FifHz
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqSetFifHz(entr_config *tunerCfg, ENTR_sint32 FifHz)
{
    ENTR_return retVal;
    ENTR_sint32 N;
    ENTR_uint8  buff[2];

    N = FifHz * tunerCfg->tunerData.FifN / tunerCfg->tunerData.FifD;
    if ((tunerCfg->FifHz - N * tunerCfg->tunerData.FifD / tunerCfg->tunerData.FifN) > ((N+1) * tunerCfg->tunerData.FifD / tunerCfg->tunerData.FifN - tunerCfg->FifHz))
        N += 1;

    buff[0] = (ENTR_uint8) N;
    buff[1] = (ENTR_uint8) (N>>8);
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_ifNlo_reg, buff, 2);
    if (retVal != ENTR_ok)
      return retVal;

    tunerCfg->tunerData.FifHz = tunerCfg->FifHz;
    tunerCfg->FifErrHz = tunerCfg->FifHz - N * tunerCfg->tunerData.FifD / tunerCfg->tunerData.FifN;

    return ENTR_ok;
}

/**
 * entr_freqSetTfBand()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqSetTfBand(entr_config *tunerCfg)
{
   ENTR_return retVal;
   ENTR_uint8 tfDeQlb;
   ENTR_uint8 tfBand;

   if (tunerCfg->tunerData.FloHz <= tunerCfg->tunerData.freqLbMax) {
      tfDeQlb = ENTR_tfDeQlb_off;
      tfBand = ENTR_tfBand_LB;
   }
   else if (tunerCfg->tunerData.FloHz <= tunerCfg->tunerData.freqMbMax) {
      tfDeQlb = ENTR_tfDeQlb_on;
      tfBand = ENTR_tfBand_MB;
   }
   else {
      tfDeQlb = ENTR_tfDeQlb_on;
      tfBand = ENTR_tfBand_HB;
   }

    retVal = entr_usrRegWr (tunerCfg, ENTR_tfDeQlb_reg, ENTR_tfDeQlb_mask, tfDeQlb);
    if (retVal != ENTR_ok)
      return retVal;

   retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, tfBand);
    if (retVal != ENTR_ok)
      return retVal;

   return ENTR_ok;
}

/**
 * entr_freqCfgTtGen()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param [in] LoDiv
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqCfgTtGen(entr_config *tunerCfg, ENTR_uint8 LoDiv)
{
   ENTR_return retVal;
   ENTR_uint8 ttFilter;

   if (LoDiv > 8)
      retVal = entr_usrRegWr (tunerCfg, ENTR_ttGenDiv_reg, ENTR_ttGenDiv_mask, ENTR_ttGenDiv_off);
   else
      retVal = entr_usrRegWr (tunerCfg, ENTR_ttGenDiv_reg, ENTR_ttGenDiv_mask, ENTR_ttGenDiv_on);
   if (retVal != ENTR_ok)
      return retVal;

   if (tunerCfg->tunerData.FloHz < 112500000)
      ttFilter = 7;
   else if (tunerCfg->tunerData.FloHz < 225000000)
      ttFilter = 6;
   else if (tunerCfg->tunerData.FloHz < 337500000)
      ttFilter = 5;
   else if (tunerCfg->tunerData.FloHz < 450000000)
      ttFilter = 4;
   else if (tunerCfg->tunerData.FloHz < 562500000)
      ttFilter = 3;
    else if (tunerCfg->tunerData.FloHz < 675000000)
      ttFilter = 2;
   else if (tunerCfg->tunerData.FloHz < 787500000)
      ttFilter = 1;
   else
      ttFilter = 0;
   retVal = entr_usrRegWr (tunerCfg, ENTR_ttFilter_reg, ENTR_ttFilter_mask, ttFilter);
   if (retVal != ENTR_ok)
      return retVal;

   return ENTR_ok;
}

/**
 * entr_freqSetLc()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note PROBLEM!!! This routine only works with fxtal = 24MHz.
 */
ENTR_return entr_freqSetLc(entr_config *tunerCfg)
{
    ENTR_return retVal;

    ENTR_sint32 FlcHz;
    ENTR_sint16 iLcc;
    ENTR_sint16 iLcs;
    ENTR_sint32 lcc;
    ENTR_sint32 lcs;
    ENTR_uint8  buff[3];
    ENTR_uint8  lcCtrl;

    /*******************************************************************************
    *
    *
    *
    *******************************************************************************/
    FlcHz = (tunerCfg->tunerData.FloHz + 4000000) / tunerCfg->tunerData.FxtalHz;
    FlcHz *= tunerCfg->tunerData.FxtalHz;
    FlcHz = tunerCfg->tunerData.FloHz - FlcHz;
    if (tunerCfg->tunerData.specInvSel == ENTR_specInvSel_off)
        FlcHz *= -1;
    if ((FlcHz > -4000001) && (FlcHz < 4000001)) {
        if (FlcHz < 0)
            iLcc = -FlcHz / 62500;
        else
            iLcc = FlcHz / 62500;
        if (iLcc > 48)
            iLcc = 96 - iLcc;
        iLcs = 48 - iLcc;
        if ((iLcc < 0) || (iLcc > 48) || (iLcs < 0) || (iLcs > 48))
            return ENTR_err_unknown;
        lcc = ENTR_freqLcLut[iLcc];
        lcs = ENTR_freqLcLut[iLcs];
        if ((FlcHz > 3000000) || (FlcHz < -3000000))
            lcc = 4194304 - lcc;
        else if (iLcc == 0)
            lcc -= 1;
        if (FlcHz < 0)
            lcs = 4194304 - lcs;
        else if (iLcs == 0)
            lcs -= 1;
        lcCtrl = ENTR_lcCtrl_on;
    }
    else {
        lcc = 0;
        lcs = 0;
        lcCtrl = ENTR_lcCtrl_off;
    }
    buff[0] = (ENTR_uint8) lcc;
    buff[1] = (ENTR_uint8) (lcc>>8);
    buff[2] = (ENTR_uint8) (lcc>>16);
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_lccLo_reg, buff, 3);
    if (retVal != ENTR_ok)
        return retVal;
    buff[0] = (ENTR_uint8) lcs;
    buff[1] = (ENTR_uint8) (lcs>>8);
    buff[2] = (ENTR_uint8) (lcs>>16);
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_lcsLo_reg, buff, 3);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_lcCtrl_reg, ENTR_lcCtrl_mask, lcCtrl);
    if (retVal != ENTR_ok)
        return retVal;

   return ENTR_ok;
}

/**
 * entr_freqSetLnaMode()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_freqSetLnaMode(entr_config *tunerCfg)
{
    ENTR_return retVal;

    ENTR_uint8 agcAccSelRestore;
    ENTR_uint8 ifAgcLoopBwRestore;
    ENTR_uint8 rfAgcLoopBwRestore;
    ENTR_uint8 ifAgcShiftRestore;
    ENTR_uint8 tfAttCwResore;
    ENTR_uint8 detLvlCtrlRestore;
    ENTR_uint8 rfAgcTopRestore;
    ENTR_sint32 rfAcc;
    ENTR_sint32 ifAcc;
    ENTR_uint8 i;
    ENTR_uint8 buff[2];

    if (tunerCfg->broadcastMedium != ENTR_broadcastMedium_cable) {
        retVal = entr_usrRegRd (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, &agcAccSelRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_rfAgcLoopBw_reg, ENTR_rfAgcLoopBw_mask, &rfAgcLoopBwRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, &ifAgcLoopBwRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_ifAgcShift_reg, ENTR_ifAgcShift_mask, &ifAgcShiftRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, &tfAttCwResore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_detLvlCtrl_reg, ENTR_detLvlCtrl_mask, &detLvlCtrlRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_rfAgcTop_reg, ENTR_rfAgcTop_mask, &rfAgcTopRestore);
        if (retVal != ENTR_ok)
            return retVal;

        retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ENTR_ifAgcLoopBw_BW_4p8Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_rfAgcLoopBw_reg, ENTR_rfAgcLoopBw_mask, ENTR_rfAgcLoopBw_BW80Hz);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcShift_reg, ENTR_ifAgcShift_mask, ENTR_ifAgcShift_bbagcshft_7);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_terNorm);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, 15<<3);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_detLvlCtrl_reg, ENTR_detLvlCtrl_mask, ENTR_detLvlCtrl_rangeM35M31);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_rfAgcTop_reg, ENTR_rfAgcTop_mask, ENTR_rfAgcTop_Vdet_0p1);
        if (retVal != ENTR_ok)
            return retVal;

        buff[0] = 255;
        buff[1] = 15;
        retVal = entr_user_2wb_wr(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccWrLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_BB);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_enaWr);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_disWr);
        if (retVal != ENTR_ok)
            return retVal;

        entr_user_delay(10);

        rfAcc = 0;
        ifAcc = 0;
        for (i=0;i<8;i++){
         retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_RF);
         if (retVal != ENTR_ok)
            return retVal;
         retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
         if (retVal != ENTR_ok)
            return retVal;
         rfAcc += (((ENTR_sint32) buff[1])<<8) + (ENTR_sint32) buff[0];

         retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, ENTR_agcAccSel_BB);
         if (retVal != ENTR_ok)
            return retVal;
         retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
         if (retVal != ENTR_ok)
            return retVal;
         ifAcc += (((ENTR_sint32) buff[1])<<8) + (ENTR_sint32) buff[0];

        }
        rfAcc >>= 3;
        ifAcc >>= 3;

        if ((rfAcc < ENTR_freqSetLnaModeAccMin) || (ifAcc < ENTR_freqSetLnaModeAccMin)) {
          /*
             retVal = entr_usrRegWr (tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_terNorm);
             if (retVal != ENTR_ok)
                return retVal;
                */
             retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, tfAttCwResore);
             if (retVal != ENTR_ok)
                return retVal;
             tunerCfg->lnaModeStatus = ENTR_lnaModeStatus_ota;
        }
        else {
            retVal = entr_usrRegWr (tunerCfg, ENTR_lnaMode_reg, ENTR_lnaMode_mask, ENTR_lnaMode_terLoNoise);
            if (retVal != ENTR_ok)
                return retVal;
            retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, 0);
            if (retVal != ENTR_ok)
                return retVal;
            tunerCfg->lnaModeStatus = ENTR_lnaModeStatus_ota_ln;
        }

        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccSel_reg, ENTR_agcAccSel_mask, agcAccSelRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcLoopBw_reg, ENTR_ifAgcLoopBw_mask, ifAgcLoopBwRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_rfAgcLoopBw_reg, ENTR_rfAgcLoopBw_mask, rfAgcLoopBwRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcShift_reg, ENTR_ifAgcShift_mask, ifAgcShiftRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_detLvlCtrl_reg, ENTR_detLvlCtrl_mask, detLvlCtrlRestore);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_rfAgcTop_reg, ENTR_rfAgcTop_mask, rfAgcTopRestore);
        if (retVal != ENTR_ok)
            return retVal;
   }

   return ENTR_ok;
}
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrIfCal.c
 *
 * @brief
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


/*************************************************
* indexed by ENTR_ifCalBw enum:
* ENTR_ifCalBw_6MHz 0
* ENTR_ifCalBw_7MHz 1
* ENTR_ifCalBw_8MHz 2
* ENTR_ifCalBw_10MHz 3
*
* must be made multi-dimensional when new Fxtal
* frequencies are added.
*************************************************/
static const ENTR_uint8 ENTR_NifHi[4] = {5, 6, 7, 9};
static const ENTR_uint8 ENTR_NifLo[4] = {155, 138, 121, 87};
static const ENTR_uint8 ENTR_NspinHi[4] = {5, 6, 7, 6};
static const ENTR_uint8 ENTR_NspinMid[4] = {155, 138, 121, 169};
static const ENTR_uint8 ENTR_NspinLo[4] = {0, 0, 0, 0};

/**
 * entr_ifCalBW()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called (indiectly) in entr_active().
 */
ENTR_return enter_ifCalBW(entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_sint32 ifCalVref;

    retVal = entr_cfgIfBwCal(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* get ref ifCalVref */
    /* set in cfg file
    retVal = entr_ifSetTtFreqHz(tunerCfg, ENTR_ifFttHz_Ref);
    if (retVal != ENTR_ok)
        return retVal;
    */
    retVal = entr_cfgCalDetRead (tunerCfg, &ifCalVref);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_calDetCtrl_reg, ENTR_calDetCtrl_mask, ENTR_calDetCtrl_SM);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdCtrl_reg, ENTR_ifCapWdCtrl_mask, ENTR_ifCapWdCtrl_SM);
    if (retVal != ENTR_ok)
        return retVal;

    /* Cal 6 MHz */
    retVal = entr_ifCalBWSngleFreq(tunerCfg, ifCalVref, ENTR_ifCalD_6MHz, ENTR_ifCalBw_6MHz);
    if (retVal != ENTR_ok)
        return retVal;

    /* Cal 7 MHz */
    retVal = entr_ifCalBWSngleFreq(tunerCfg, ifCalVref, ENTR_ifCalD_7MHz, ENTR_ifCalBw_7MHz);
    if (retVal != ENTR_ok)
        return retVal;

    /* Cal 8 MHz */
     retVal = entr_ifCalBWSngleFreq(tunerCfg, ifCalVref, ENTR_ifCalD_8MHz, ENTR_ifCalBw_8MHz);
    if (retVal != ENTR_ok)
        return retVal;

    /* Cal 10 MHz */
     retVal = entr_ifCalBWSngleFreq(tunerCfg, ifCalVref, ENTR_ifCalD_10MHz, ENTR_ifCalBw_10MHz);
    if (retVal != ENTR_ok)
        return retVal;

    /* set ctrl to b2w */
    retVal = entr_usrRegWr (tunerCfg, ENTR_calDetCtrl_reg, ENTR_calDetCtrl_mask, ENTR_calDetCtrl_B2W);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCapWdCtrl_reg, ENTR_ifCapWdCtrl_mask, ENTR_ifCapWdCtrl_B2W);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/**
 * entr_ifCalBWSngleFreq()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called by entr_ifCalBW only.
 */
ENTR_return entr_ifCalBWSngleFreq(entr_config *tunerCfg, ENTR_sint32 ifCalVref, ENTR_sint32 ifCalD, ENTR_uint8 ifBw)
{
    ENTR_return retVal;
    ENTR_uint8 cw;

    retVal = entr_ifSetTtFreq(tunerCfg, ifBw);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCalVrefLo_reg , ENTR_ifCalVrefLo_mask, ((ENTR_uint8)((ifCalVref * ENTR_ifCalN / ifCalD))));
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCalVrefHi_reg , ENTR_ifCalVrefHi_mask, ((ENTR_uint8)((ifCalVref * ENTR_ifCalN / ifCalD) >> 8)));
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_ifTrigSM(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegRd(tunerCfg, ENTR_ifCapWdRd_reg, ENTR_ifCapWdRd_mask, &cw);
    if (retVal != ENTR_ok)
        return retVal;
    tunerCfg->tunerData.ifBwWd[ifBw] = cw;

    return ENTR_ok;
}

/*
 * entr_ifTrigSM()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_ifcsm_timeout
 *          ENTR_err_rw otherwise.
 *
 * @note Called by entr_ifCalBW only.
 */
ENTR_return entr_ifTrigSM(entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 ifCalDone;
    ENTR_uint8 ifCalErr;
    ENTR_uint8 i;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCalTrig_reg, ENTR_ifCalTrig_mask, ENTR_ifCalTrig_Trigger);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ifCalTrig_reg, ENTR_ifCalTrig_mask, ENTR_ifCalTrig_ClearTrigger);
    if (retVal != ENTR_ok)
        return retVal;

    for (i=0; i<20; i++) {
        retVal = entr_usrRegRd(tunerCfg, ENTR_ifCalDone_reg, ENTR_ifCalDone_mask, &ifCalDone);
        if (retVal != ENTR_ok)
            return retVal;

        if (ifCalDone == ENTR_ifCalDone_done)
            break;
        entr_user_delay(1);
    }

    if (ifCalDone != ENTR_ifCalDone_done)
        return ENTR_err_ifcsm_timeout;

    retVal = entr_usrRegRd(tunerCfg, ENTR_ifCalErr_reg, ENTR_ifCalErr_mask, &ifCalErr);
    if (retVal != ENTR_ok)
        return retVal;
    if (ifCalErr == ENTR_ifCalErr_Err)
        return ENTR_err_ifcsm_timeout;

    return ENTR_ok;
}

/*
 * entr_ifSetTtFreq()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] ifBw
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called by entr_ifCalBW only. fttHz may be rounded by setting Nif = ftt * > FifN / FifD,
 * but the test tones will always be equal since Nspin is calculated from Nif
 */
ENTR_return entr_ifSetTtFreq(entr_config *tunerCfg, ENTR_uint8 ifBw)
{
    ENTR_return retVal;
    ENTR_uint8 buff[3];

    buff[0] = ENTR_NifLo[ifBw];
    buff[1] = ENTR_NifHi[ifBw];
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_ifNlo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_user_2wb_rd (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_rotHi_reg, buff,1);
    if (retVal != ENTR_ok)
      return retVal;
    buff[2] = (buff[0] & 0xf0) | ENTR_NspinHi[ifBw];
    buff[0] = ENTR_NspinLo[ifBw];
    buff[1] = ENTR_NspinMid[ifBw];
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_rotLo_reg, buff, 3);
    if (retVal != ENTR_ok)
      return retVal;

    /* accuracy is not important here */
    tunerCfg->tunerData.FifHz = 0;
    tunerCfg->tunerData.FspinHz = 0;

    return ENTR_ok;
}

/*
 * entr_ifAccTopCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called (indiectly) in entr_active().
 */
ENTR_return entr_ifAccTopCal(entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_cfgIfAccTopCal(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_ifAccTopCalSingleBckOff(tunerCfg, ENTR_broadcastMedium_ota, ENTR_ifAccTopRefN_Ter);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifVga2Ena_reg, ENTR_ifVga2Ena_mask, ENTR_ifVga2Ena_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_ifAccTopCalSingleBckOff(tunerCfg, ENTR_broadcastMedium_cable, ENTR_ifAccTopRefN_Cable);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_ifAccTopCalSingleBckOff()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] tunerMode
 * @param[in] ifAccTopRefN
 *
 * @return  ENTR_ok on success
 *          ENTR_err_ifAccTop_setTtPwr
 *          ENTR_err_rw otherwise.
 *
 * @note Called by entr_ifAccTopCal() only.
 */
ENTR_return entr_ifAccTopCalSingleBckOff(entr_config *tunerCfg, ENTR_uint8 tunerMode, ENTR_sint32 ifAccTopRefN)
{
    ENTR_return retVal;
    ENTR_uint8 ttPwr;
    ENTR_uint8 buff[2];
    ENTR_sint32 ifAccTopVref;
    ENTR_sint32 ifAccTopVtarget;
    ENTR_sint32 ifAccTopVdet;
    ENTR_sint32 ifAccMin;
    ENTR_sint32 ifAcc;
    ENTR_sint32 ifAccMax;

    /* set ttPwr to min */
    ttPwr = ENTR_ttPwr_LnaNeg80MixNeg68dBV;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttPwr_reg, ENTR_ttPwr_mask, ttPwr);
    if (retVal != ENTR_ok)
        return retVal;

    /* set if gain to max (4095)*/
    buff[0] = 255;
    buff[1] = 15;
    retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccWrLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_enaWr);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_disWr);
    if (retVal != ENTR_ok)
        return retVal;

    /* Optimize ttPwr                        */
    /* CalDetVref @ 6 dB backoff ~2200       */
    /* ttPwr increases 6 dB/step             */
    /* increase ttPwr while ifTopVref < 2000 */
    retVal = entr_cfgCalDetRead (tunerCfg, &ifAccTopVref);
    if (retVal != ENTR_ok)
        return retVal;
    while (ifAccTopVref < 2000) {
        if (ttPwr == ENTR_ttPwr_LnaNeg80MixNeg68dBV)
            ttPwr = ENTR_ttPwr_LnaNeg74MixNeg62dBV;
        else if (ttPwr == ENTR_ttPwr_LnaNeg74MixNeg62dBV)
            ttPwr = ENTR_ttPwr_LnaNeg68MixNeg56dBV;
        else if (ttPwr == ENTR_ttPwr_LnaNeg68MixNeg56dBV)
            ttPwr = ENTR_ttPwr_LnaNeg62MixNeg50dBV;
        else if (ttPwr == ENTR_ttPwr_LnaNeg62MixNeg50dBV)
            ttPwr = ENTR_ttPwr_LnaNeg56MixNeg44dBV;
        else if (ttPwr == ENTR_ttPwr_LnaNeg56MixNeg44dBV)
            ttPwr = ENTR_ttPwr_LnaNeg50MixNeg38dBV;
        else if (ttPwr == ENTR_ttPwr_LnaNeg50MixNeg38dBV)
            ttPwr = ENTR_ttPwr_LnaNeg44MixNeg32dBV;
        else
            return ENTR_err_ifAccTop_setTtPwr;
        retVal = entr_usrRegWr (tunerCfg, ENTR_ttPwr_reg, ENTR_ttPwr_mask, ttPwr);
        if (retVal != ENTR_ok)
            return retVal;
        entr_user_delay(5);
        retVal = entr_cfgCalDetRead (tunerCfg, &ifAccTopVref);
        if (retVal != ENTR_ok)
            return retVal;
    }

    ifAccTopVtarget = ifAccTopVref * ifAccTopRefN;
    ifAccTopVtarget >>= ENTR_ifAccTopRefDbits;
    ifAccMin = 0;
    ifAccMax = 4095;
    while (ifAccMin + 1 < ifAccMax) {
        /* set ifGain */
        ifAcc = (ifAccMin + ifAccMax) / 2;
        buff[0] = (ENTR_uint8) ifAcc;
        buff[1] = (ENTR_uint8) (ifAcc >> 8);
        retVal = entr_user_2wb_wr (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccWrLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_enaWr);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_disWr);
        if (retVal != ENTR_ok)
            return retVal;
        /* read detector */
        entr_user_delay(10);
        retVal = entr_cfgCalDetRead (tunerCfg, &ifAccTopVdet);
        if (retVal != ENTR_ok)
            return retVal;
        if (ifAccTopVdet < ifAccTopVtarget)
            ifAccMin = ifAcc;
        else
            ifAccMax = ifAcc;
    }

    tunerCfg->tunerData.ifAccTop[tunerMode] = ifAcc;

    return ENTR_ok;
}

/*
 * entr_ifSetAccTop()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called (indirectly) by entr_set_chan().
 */
ENTR_return entr_ifSetAccTop(entr_config *tunerCfg)
{
    ENTR_return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcAccTopLo_reg, ENTR_ifAgcAccTopLo_mask, (ENTR_uint8)(tunerCfg->tunerData.ifAccTop[tunerCfg->broadcastMedium]));
     if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcAccTopHi_reg, ENTR_ifAgcAccTopHi_mask, (ENTR_uint8)(tunerCfg->tunerData.ifAccTop[tunerCfg->broadcastMedium]>>8));
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrReg.c
 *
 * @brief
 *
 *   $xRev: 11153 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/


/*
 * entr_usrRegWr()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 * @param[in] reg
 * @param[in] mask
 * @param[in] value
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called (indirectly) by entr_set_chan().
 */
ENTR_return entr_usrRegWr(entr_config *tunerCfg, ENTR_uint8 reg, ENTR_uint8 mask, ENTR_uint8 value)
{
   ENTR_return retVal;
   ENTR_uint8 buff;

   retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress,reg,&buff,1);
   if (retVal != ENTR_ok)
      return ENTR_err_rw;
   buff &= ~mask;
   buff |= value;
   retVal = entr_user_2wb_wr(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress,reg,&buff,1);
   if (retVal != ENTR_ok)
      return ENTR_err_rw;

   return ENTR_ok;
}

/*
 * entr_usrRegRd()
 *
 * @param[in] tunerCfg Tuner configuration structure.
 * @param[in] reg
 * @param[in] mask
 * @param[out] value
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note Called (indirectly) by entr_set_chan().
 */
ENTR_return entr_usrRegRd(entr_config *tunerCfg, ENTR_uint8 reg, ENTR_uint8 mask, ENTR_uint8 *value)
{
   ENTR_return retVal;
   ENTR_uint8 buff;

   retVal = entr_user_2wb_rd(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress,reg,&buff,1);
   if (retVal != retVal)
      return ENTR_err_rw;
   *value = buff & mask;

   return ENTR_ok;
}
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

 /**
 * @file entrTfCal.c
 *
 * @brief
 *
 *   $xRev: 11861 $:
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/


ENTR_sint32 entr_tfFreqLb[ENTR_tfFreqCntLb] = {55000000, 71125000, 92062500, 119000000, 154000000};
ENTR_uint8 tfCw0Lb[ENTR_tfFreqCntLb] = {48, 42, 30, 18, 7};

ENTR_sint32 entr_tfFreqMb[ENTR_tfFreqCntMb] = {204000000, 243000000, 289250000, 344250000, 410000000};
ENTR_uint8 entr_tfCw0Mb[ENTR_tfFreqCntMb] = {45, 37, 25, 18, 8};

ENTR_sint32 entr_tfFreqHb[ENTR_tfFreqCntHb] = {490000000, 548500000, 614000000, 687500000, 770000000};
ENTR_uint8 entr_tfCw0Hb[ENTR_tfFreqCntHb] = {25, 20, 16, 10, 6};

/*
 * entr_tfCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfCal(entr_config *tunerCfg)
{
    ENTR_return retVal;

    ENTR_uint8  ttPwr;
    ENTR_uint8  buff[2];
    ENTR_uint8  i;
    ENTR_sint32 flbMax;
    ENTR_sint32 fmbMin;
    ENTR_sint32 fmbMax;
    ENTR_sint32 fhbMin;

    retVal = entr_cfgTfCal (tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* initialize ttCalTtPwrLut's */
    tunerCfg->tunerData.tfcTtLut[0] = 4095;
    for (i=1;i<7;i++) {
        switch(i) {
            case 0:
                ttPwr = ENTR_ttPwr_LnaNeg80MixNeg68dBV;
                break;
            case 1:
                ttPwr = ENTR_ttPwr_LnaNeg74MixNeg62dBV;
                break;
            case 2:
                ttPwr = ENTR_ttPwr_LnaNeg68MixNeg56dBV;
                break;
            case 3:
                ttPwr = ENTR_ttPwr_LnaNeg62MixNeg50dBV;
                break;
            case 4:
                ttPwr = ENTR_ttPwr_LnaNeg56MixNeg44dBV;
                break;
            case 5:
                ttPwr = ENTR_ttPwr_LnaNeg50MixNeg38dBV;
                break;
            case 6:
                ttPwr = ENTR_ttPwr_LnaNeg44MixNeg32dBV;
                break;
            default:
                return ENTR_err_unknown; /* will never happen */
        }
        retVal = entr_usrRegWr (tunerCfg, ENTR_ttPwr_reg, ENTR_ttPwr_mask, ttPwr);
        if (retVal != ENTR_ok)
            return retVal;

        /* agcAccSel = bb */
        buff[0] = 255;
        buff[1] = 15;
        retVal = entr_user_2wb_wr(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccWrLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_enaWr);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_disWr);
        if (retVal != ENTR_ok)
            return retVal;

        retVal = entr_cfgBbAgcDelay(tunerCfg, 10);
        if (retVal != ENTR_ok)
            return retVal;

        retVal = entr_user_2wb_rd (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
        if (retVal != ENTR_ok)
            return retVal;

        tunerCfg->tunerData.tfcTtLut[i] = (((ENTR_sint16) buff[1])<<8) + (ENTR_sint16) buff[0];
    }

    /* low band */
    retVal = entr_tfCalOneBand(tunerCfg, ENTR_tfBand_LB, ENTR_tfcCapWdMax_Max63, ENTR_tfFreqCntLb, tfCw0Lb, entr_tfFreqLb, tunerCfg->tunerData.tffCwLb, tunerCfg->tunerData.tfgCwLb);
    if (retVal != ENTR_ok)
        return retVal;

    /* mid band */
    retVal = entr_tfCalOneBand(tunerCfg, ENTR_tfBand_MB, ENTR_tfcCapWdMax_Max63, ENTR_tfFreqCntMb, entr_tfCw0Mb, entr_tfFreqMb, tunerCfg->tunerData.tffCwMb, tunerCfg->tunerData.tfgCwMb);
    if (retVal != ENTR_ok)
        return retVal;

    /* high band */
    retVal = entr_tfCalOneBand(tunerCfg, ENTR_tfBand_HB, ENTR_tfcCapWdMax_Max31, ENTR_tfFreqCntHb, entr_tfCw0Hb, entr_tfFreqHb, tunerCfg->tunerData.tffCwHb, tunerCfg->tunerData.tfgCwHb);
    if (retVal != ENTR_ok)
        return retVal;

    /* set band transition frequencies */
     flbMax = entr_tfFreqLb[ENTR_tfFreqCntLb-1] - entr_tfFreqLb[ENTR_tfFreqCntLb-2];
    flbMax /= (tunerCfg->tunerData.tffCwLb[ENTR_tfFreqCntLb-2] - tunerCfg->tunerData.tffCwLb[ENTR_tfFreqCntLb-1]);
    flbMax *= tunerCfg->tunerData.tffCwLb[ENTR_tfFreqCntLb-1];
    flbMax += entr_tfFreqLb[ENTR_tfFreqCntLb-1];

    fmbMin = entr_tfFreqMb[1] - entr_tfFreqMb[0];
    fmbMin /= (tunerCfg->tunerData.tffCwMb[0] - tunerCfg->tunerData.tffCwMb[1]);
    fmbMin *= (63 - tunerCfg->tunerData.tffCwMb[0]);
    fmbMin = entr_tfFreqMb[0] - fmbMin;

    tunerCfg->tunerData.freqLbMax = (flbMax + fmbMin) / 2;

     fmbMax = entr_tfFreqMb[ENTR_tfFreqCntMb-1] - entr_tfFreqMb[ENTR_tfFreqCntMb-2];
    fmbMax /= (tunerCfg->tunerData.tffCwMb[ENTR_tfFreqCntMb-2] - tunerCfg->tunerData.tffCwMb[ENTR_tfFreqCntMb-1]);
    fmbMax *= tunerCfg->tunerData.tffCwMb[ENTR_tfFreqCntMb-1];
    fmbMax += entr_tfFreqMb[ENTR_tfFreqCntMb-1];

    fhbMin = entr_tfFreqHb[1] - entr_tfFreqHb[0];
    fhbMin /= (tunerCfg->tunerData.tffCwHb[0] - tunerCfg->tunerData.tffCwHb[1]);
    fhbMin *= (31 - tunerCfg->tunerData.tffCwHb[0]);
    fhbMin = entr_tfFreqHb[0] - fhbMin;

    tunerCfg->tunerData.freqMbMax = (fmbMax + fhbMin) / 2;

    return ENTR_ok;
}

/*
 * entr_tfCalOneBand()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] tfBand
 * @param[in] tfcCapWdMax
 * @param[in] tfFreqCnt
 * @param[in] tfCw0
 * @param[in] tfFreq
 * @param[out] ttPwrLut
 * @param[in] tffCw
 * @param[out] tfgCw
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfCalOneBand(entr_config *tunerCfg, ENTR_uint8 tfBand, ENTR_uint8 tfcCapWdMax, ENTR_uint8 tfFreqCnt, ENTR_uint8 tfCw0[], ENTR_sint32 tfFreq[], ENTR_uint8 tffCw[], ENTR_uint8 tfgCw[])
{
    ENTR_return retVal;
    ENTR_uint8 ttPwr;
    ENTR_uint8 i;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, tfBand);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfcCapWdMax_reg, ENTR_tfcCapWdMax_mask, tfcCapWdMax);
    if (retVal != ENTR_ok)
        return retVal;
    for (i = 0; i < tfFreqCnt; i++) {
        retVal = entr_usrRegWr (tunerCfg, ENTR_tfcCapWd0_reg, ENTR_tfcCapWd0_mask, tfCw0[i]);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_freqSetFloHz(tunerCfg, tfFreq[i]);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_tfSetTtPwr (tunerCfg, &ttPwr);
        if (retVal != ENTR_ok)
            return retVal;
        /* added for testing */
        switch (tfBand) {
            case ENTR_tfBand_LB:
                tunerCfg->tunerData.tfcTtPwrLb[i] = ttPwr;
                break;
            case ENTR_tfBand_MB:
                tunerCfg->tunerData.tfcTtPwrMb[i] = ttPwr;
                break;
            case ENTR_tfBand_HB:
                tunerCfg->tunerData.tfcTtPwrHb[i] = ttPwr;
                break;
            default:
                return ENTR_err_unknown; /* will never happen */
        }
        retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, tfBand);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_tfcTrig (tunerCfg);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_usrRegRd (tunerCfg, ENTR_tfcCapWdRd_reg, ENTR_tfcCapWdRd_mask, &tffCw[i]);
        if (retVal != ENTR_ok)
            return retVal;

        retVal = entr_tfGainCal(tunerCfg, tfBand, &tfgCw[i]);
        if (retVal != ENTR_ok)
            return retVal;

    }
    return ENTR_ok;
}

/*
 * entr_tfGainCal()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] tfBand
 * @param[out] tfgCw
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfGainCal(entr_config *tunerCfg, ENTR_uint8 tfBand, ENTR_uint8 *tfgCw)
{
    ENTR_return retVal;

    ENTR_sint32 tfgVref;
    ENTR_sint32 tfgVtarget;
    ENTR_sint32 tfgVdet;
    ENTR_sint32 tfgVchk;
    ENTR_uint8  tfAttMin;
    ENTR_uint8  tfAtt;
    ENTR_uint8  tfAttMax;
    ENTR_uint8  tfAttChk;

    /* get ref */
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, ENTR_tfBand_TT);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttlnjectPt_reg, ENTR_ttlnjectPt_mask, ENTR_ttlnjectPt_Mix);
    if (retVal != ENTR_ok)
        return retVal;
    entr_user_delay (10);
    retVal = entr_usrRegWr (tunerCfg, ENTR_calDetCtrl_reg, ENTR_calDetCtrl_mask, ENTR_calDetCtrl_B2W);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_cfgCalDetRead (tunerCfg, &tfgVref);
    if (retVal != ENTR_ok)
        return retVal;
    if (tunerCfg->tunerData.FloHz < ENTR_tfgFmbMaxHz)
        tfgVtarget = tfgVref * (tunerCfg->tunerData.FloHz / ENTR_tfgCompMlo + ENTR_tfgCompBlo) / ENTR_tfgD;
    else
        tfgVtarget = tfgVref * (tunerCfg->tunerData.FloHz / ENTR_tfgCompMhi + ENTR_tfgCompBhi) / ENTR_tfgD;

    /* set tt input to lna */
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, tfBand);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttlnjectPt_reg, ENTR_ttlnjectPt_mask, ENTR_ttlnjectPt_Lna);
    if (retVal != ENTR_ok)
        return retVal;

    /* binary search for gain */
    tfAttMin = 0;
    tfAttMax = 15;
    while (tfAttMin + 1 < tfAttMax) {
        tfAtt = (tfAttMin + tfAttMax) / 2;
        retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, (ENTR_uint8) (tfAtt<<3));
        if (retVal != ENTR_ok)
            return retVal;
        entr_user_delay(5);
        retVal = entr_cfgCalDetRead (tunerCfg, &tfgVdet);
        if (retVal != ENTR_ok)
            return retVal;
        if (tfgVdet < tfgVtarget)
            tfAttMax = tfAtt;
        else
            tfAttMin = tfAtt;
    }

    /* loop exits wih tfAttMin = tfAtt or tfAttMax = tfAtt. */
    /* find closest tfAtt or one not equal to tfAtt         */
    /* tfgVdet -> tfAtt                                     */
    if (tfAttMin == tfAtt)
        tfAttChk = tfAttMax;
    else
        tfAttChk = tfAttMin;
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, (ENTR_uint8) (tfAttChk<<3));
    if (retVal != ENTR_ok)
        return retVal;
    entr_user_delay(5);
    retVal = entr_cfgCalDetRead (tunerCfg, &tfgVchk);
    if (retVal != ENTR_ok)
        return retVal;
    tfgVdet -= tfgVtarget;
    if (tfgVdet < 0)
        tfgVdet *= -1;
    tfgVchk -= tfgVtarget;
    if (tfgVchk < 0)
        tfgVchk *= -1;
    if (tfgVdet < tfgVchk)
        *tfgCw = tfAtt;
    else
        *tfgCw = tfAttChk;

    retVal = entr_usrRegWr (tunerCfg, ENTR_calDetCtrl_reg, ENTR_calDetCtrl_mask, ENTR_calDetCtrl_SM);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_tfSetTtPwr()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] ttPwrLut
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfSetTtPwr(entr_config *tunerCfg, ENTR_uint8 *ttPwr)
{
    ENTR_return retVal;

    ENTR_uint8  buff[2];
    ENTR_sint32 ifAcc;
    ENTR_uint8  i;

     retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, 0);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttEna_reg, ENTR_ttEna_mask, ENTR_ttEna_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttlnjectPt_reg, ENTR_ttlnjectPt_mask, ENTR_ttlnjectPt_Lna);
    if (retVal != ENTR_ok)
        return retVal;

    /* agcAccSel = bb */
    buff[0] = 255;
    buff[1] = 15;
    retVal = entr_user_2wb_wr(tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccWrLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_enaWr);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_agcAccLoad_reg, ENTR_agcAccLoad_mask, ENTR_agcAccLoad_disWr);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcEna_reg, ENTR_ifAgcEna_mask, ENTR_ifAgcEna_on);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgBbAgcDelay(tunerCfg, 10);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_user_2wb_rd (tunerCfg->i2c_id, (ENTR_uint8) tunerCfg->b2wAddress, ENTR_agcAccRdLo_reg, buff, 2);
    if (retVal != ENTR_ok)
        return retVal;
    ifAcc = (((ENTR_sint32) buff[1]) << 8) + (ENTR_sint32) buff[0];
    for (i = 0; i <= 6; i++) {
        if (ifAcc >= tunerCfg->tunerData.tfcTtLut[i])
            break;
    }
    if (i > 6)
        i--;
    switch(i) {
        case 0:
            *ttPwr = ENTR_ttPwr_LnaNeg80MixNeg68dBV;
            break;
        case 1:
            *ttPwr = ENTR_ttPwr_LnaNeg74MixNeg62dBV;
            break;
        case 2:
            *ttPwr = ENTR_ttPwr_LnaNeg68MixNeg56dBV;
            break;
        case 3:
            *ttPwr = ENTR_ttPwr_LnaNeg62MixNeg50dBV;
            break;
        case 4:
            *ttPwr = ENTR_ttPwr_LnaNeg56MixNeg44dBV;
            break;
        case 5:
            *ttPwr = ENTR_ttPwr_LnaNeg50MixNeg38dBV;
            break;
        case 6:
            *ttPwr = ENTR_ttPwr_LnaNeg44MixNeg32dBV;
            break;
        default:
            return ENTR_err_unknown; /* will never happen */
    }
    /* run if agc with tt injected at the mixer, then swich to the lna */
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttPwr_reg, ENTR_ttPwr_mask, *ttPwr);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfBand_reg, ENTR_tfBand_mask, ENTR_tfBand_TT);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttlnjectPt_reg, ENTR_ttlnjectPt_mask, ENTR_ttlnjectPt_Mix);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttEna_reg, ENTR_ttEna_mask, ENTR_ttEna_on);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgBbAgcDelay(tunerCfg, 10);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_usrRegWr (tunerCfg, ENTR_ifAgcEna_reg, ENTR_ifAgcEna_mask, ENTR_ifAgcEna_off);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_ttlnjectPt_reg, ENTR_ttlnjectPt_mask, ENTR_ttlnjectPt_Lna);
    if (retVal != ENTR_ok)
        return retVal;
     retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, 7<<3); /* fix. must use define statements */
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_tfcTrig()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_tfcSm_timeout
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfcTrig(entr_config *tunerCfg)
{
    ENTR_return retVal;

    ENTR_uint8 tfcErr;
    ENTR_uint8 tfcDone;
    ENTR_uint8 i;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tfcTrig_reg, ENTR_tfcTrig_mask, ENTR_tfcTrig_Trig);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegWr (tunerCfg, ENTR_tfcTrig_reg, ENTR_tfcTrig_mask, ENTR_tfcTrig_ClearTrig);
    if (retVal != ENTR_ok)
        return retVal;

    for (i=0;i<100;i++) {
        retVal = entr_usrRegRd (tunerCfg, ENTR_tfcDone_reg, ENTR_tfcDone_mask, &tfcDone);
        if (retVal != ENTR_ok)
            return retVal;
        if (tfcDone == ENTR_tfcDone_done)
            break;
        entr_user_delay(1);
    }

    if (tfcDone != ENTR_tfcDone_done)
        return ENTR_err_tfcSm_timeout;

    retVal = entr_usrRegRd (tunerCfg, ENTR_tfcErr_reg, ENTR_tfcErr_mask, &tfcErr);
    if (retVal != ENTR_ok)
        return retVal;
    if (tfcErr == ENTR_tfcErr_err)
        return ENTR_err_tfcSm_timeout;

    return ENTR_ok;
}

/*
 * entr_tfChanChg()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfChanChg(entr_config *tunerCfg)
{
    ENTR_return retVal;
    ENTR_uint8 i;

    if (tunerCfg->FchanHz <= tunerCfg->tunerData.freqLbMax) {
        for (i=0;i<ENTR_tfFreqCntLb;i++) {
            if (entr_tfFreqLb[i] > tunerCfg->FchanHz)
                break;
        }
        if (i == ENTR_tfFreqCntLb)
            --i;
        retVal = entr_tfChanChgFreq(tunerCfg, i, ENTR_tfFreqCntLb, entr_tfFreqLb, tunerCfg->tunerData.tffCwLb, 63);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_tfChanChgGain(tunerCfg, i, ENTR_tfFreqCntLb, entr_tfFreqLb, tunerCfg->tunerData.tfgCwLb);
        if (retVal != ENTR_ok)
            return retVal;
        }
    else if (tunerCfg->FchanHz <= tunerCfg->tunerData.freqMbMax) {
        for (i=0;i<ENTR_tfFreqCntMb;i++) {
            if (entr_tfFreqMb[i] > tunerCfg->FchanHz)
                break;
        }
        if (i == ENTR_tfFreqCntLb)
            --i;
        retVal = entr_tfChanChgFreq(tunerCfg, i, ENTR_tfFreqCntMb, entr_tfFreqMb, tunerCfg->tunerData.tffCwMb, 63);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_tfChanChgGain(tunerCfg, i, ENTR_tfFreqCntMb, entr_tfFreqMb, tunerCfg->tunerData.tfgCwMb);
        if (retVal != ENTR_ok)
            return retVal;
        }
    else {
        for (i=0;i<ENTR_tfFreqCntHb;i++) {
            if (entr_tfFreqHb[i] > tunerCfg->FchanHz)
                break;
        }
        if (i == ENTR_tfFreqCntLb)
            --i;
        retVal = entr_tfChanChgFreq(tunerCfg, i, ENTR_tfFreqCntHb, entr_tfFreqHb, tunerCfg->tunerData.tffCwHb, 31);
        if (retVal != ENTR_ok)
            return retVal;
        retVal = entr_tfChanChgGain(tunerCfg, i, ENTR_tfFreqCntHb, entr_tfFreqHb, tunerCfg->tunerData.tfgCwHb);
        if (retVal != ENTR_ok)
            return retVal;
    }

    return retVal;
}

/*
 * entr_tfChanChgFreq()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] i
 * @param[in] tfFreqCnt
 * @param[in] tfFreq
 * @param[in] tffCw
 * @param[in] cwMax
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfChanChgFreq(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tffCw[], ENTR_uint8 cwMax)
{
    ENTR_return retVal;
    ENTR_sint16 cw;

    retVal = entr_tfChanChgLinInterp(tunerCfg, i, tfFreqCnt, tfFreq, tffCw, &cw);
    if (retVal != ENTR_ok)
        return retVal;

    if (cw < 0)
        cw = 0;
    else if(cw > cwMax)
        cw = cwMax;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tfcCapWd0_reg, ENTR_tfcCapWd0_mask, (ENTR_uint8) cw);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_tfChanChgGain()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] i
 * @param[in] tfFreqCnt
 * @param[in] tfFreq
 * @param[in] tfgCw
 *
 * @return  ENTR_ok on success
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfChanChgGain(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tfgCw[])
{
    ENTR_uint8  retVal;
    ENTR_sint16 cw;

    retVal = entr_tfChanChgLinInterp(tunerCfg, i, tfFreqCnt, tfFreq, tfgCw, &cw);
    if (retVal != ENTR_ok)
        return retVal;

    cw += (tunerCfg->tunerData.calTmpr - tunerCfg->tunerData.curTemp)/25;
    if (cw < 0)
        cw = 0;
    if (cw > 15)
        cw = 15;

    retVal = entr_usrRegWr (tunerCfg, ENTR_tfAttCw_reg, ENTR_tfAttCw_mask, (ENTR_uint8) (cw<<3));
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_tfChanChgLinInterp()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[in] i
 * @param[in] tfFreqCnt
 * @param[in] tfFreq
 * @param[in] tfCw
 * @param[out] cww
 *
 * @return  ENTR_ok on success
 *          ENTR_err_tfSetCh_fail otherwise.
 *
 * @note None.
 */
ENTR_return entr_tfChanChgLinInterp(entr_config *tunerCfg, ENTR_uint8 i, ENTR_uint8 tfFreqCnt, ENTR_sint32 tfFreq[], ENTR_uint8 tfCw[], ENTR_sint16 *cw)
{
    ENTR_sint32 dFreq;
    ENTR_sint32 dCw;
    ENTR_sint32 freqStep;
    ENTR_uint8  j;
    ENTR_sint16 k;

    if (i == 0) {
        dCw = tfCw[i+1] - tfCw[i];
        dFreq = tfFreq[i+1] - tfFreq[i];
    }
    else {
        dCw = tfCw[i] - tfCw[i-1];
        dFreq = tfFreq[i] - tfFreq[i-1];
    }
    if (dCw == 0)
        *cw = tfCw[i];
    else {
        freqStep = dFreq / dCw;
        if (freqStep < 0) {
            freqStep *= -1;
            k=-1;
        }
        else
            k=1;
        j=1;
        if (tfFreq[i] > tunerCfg->FchanHz) {
            while (tfFreq[i] - j*freqStep > tunerCfg->FchanHz) {
                ++j;
                if (j > 50)
                    return ENTR_err_tfSetCh_fail;
            }
            if ((tfFreq[i] - (j-1)*freqStep - tunerCfg->FchanHz) < (tunerCfg->FchanHz - (tfFreq[i] - j*freqStep)))
                j -= 1;
            *cw = tfCw[i] - k * j;
        }
        else {
            while (tunerCfg->FchanHz - j*freqStep > tfFreq[i]) {
                ++j;
                if (j > 50)
                    return ENTR_err_tfSetCh_fail;
            }
            if ((tunerCfg->FchanHz - (j-1)*freqStep - tfFreq[i]) < (tfFreq[i] - (tunerCfg->FchanHz - j*freqStep)))
                j -= 1;
            *cw = tfCw[i] + k * j;
        }

    }

    return ENTR_ok;
}
/*******************************************************************************
 *      Copyright (C) 2008 Entropic Communications
 ******************************************************************************/

/**
 * @file entrTuner.c
 *
 * @brief API calls for activating, tuning, idling, etc.
 *
 *   $xRev: 11927 $
 *   $xDate$
 *   $xAuthor: rfortier $
 */
/*******************************************************************************
* This file is licensed under the terms of your license agreement(s) with      *
* Entropic covering this file. Redistribution, except as permitted by the      *
* terms of your license agreement(s) with Entropic, is strictly prohibited.    *
*******************************************************************************/


/*
 * entr_active()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_invalid_parameter if bad input
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_active(entr_config *tunerCfg)
{
    ENTR_return retVal;

    /* @doc Check input parameters */
    switch(tunerCfg->b2wAddress) {
        case ENTR_b2wAddress_C0:
        case ENTR_b2wAddress_C4:
        case ENTR_b2wAddress_C6:
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    if (tunerCfg->Fxtal != ENTR_Fxtal_24MHz)
        return ENTR_err_invalid_parameter;

    if (tunerCfg->Fcomp != ENTR_Fcomp_1MHz)
        return ENTR_err_invalid_parameter;

    if ((tunerCfg->specInvSel != ENTR_specInvSel_off) && (tunerCfg->specInvSel != ENTR_specInvSel_on))
        return ENTR_err_invalid_parameter;

    if ((tunerCfg->ltaEnaSel != ENTR_ltaEnaSel_off) && (tunerCfg->ltaEnaSel != ENTR_ltaEnaSel_on))
        return ENTR_err_invalid_parameter;

    if ((tunerCfg->tunerCnt != ENTR_tunerCnt_single) && (tunerCfg->tunerCnt != ENTR_tunerCnt_dual))
        return ENTR_err_invalid_parameter;
    /* End: check input parameters */

    retVal = entr_cfgIcRev(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetFxtalAndFcomp(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgReadTmprAcc(tunerCfg, &tunerCfg->tunerData.calTmpr);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = enter_ifCalBW(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_ifAccTopCal(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_tfCal(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgBase(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* @doc Set ltaEna */
    if (tunerCfg->ltaEnaSel == ENTR_ltaEnaSel_on) {
        retVal = entr_usrRegWr(tunerCfg, ENTR_ltaEna_reg, ENTR_ltaEna_mask, ENTR_ltaEna_on);
        if (retVal != ENTR_ok)
            return retVal;
    }

    /* @doc Set tuner count (dual/single) */
    if (tunerCfg->tunerCnt == ENTR_tunerCnt_dual) {
        retVal = entr_usrRegWr(tunerCfg, ENTR_dualChipSel_reg, ENTR_dualChipSel_mask, ENTR_dualChipSel_dual);
        if (retVal != ENTR_ok)
            return retVal;
    }

    /* @doc set tuner config to false */
    tunerCfg->tunerData.tunerCfg = ENTR_tunerCfg_no;

    /* @doc set tuner active to true */
    tunerCfg->tunerData.tunerActive = ENTR_tunerActive_yes;

    return ENTR_ok;
}

/*
 * entr_set_chan()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_invalid_parameter if bad input
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_set_chan(entr_config *tunerCfg)
{
    ENTR_return retVal;

    /* @doc Exit with error if tuner is not active */
     if (tunerCfg->tunerData.tunerActive != ENTR_tunerActive_yes)
        return ENTR_err_not_active;

    /* @doc Check input parameters */
    if ((tunerCfg->FchanHz < 30000000) || (tunerCfg->FchanHz > 900000000))
        return ENTR_err_invalid_parameter;

    switch (tunerCfg->broadcastMedium) {
        case ENTR_broadcastMedium_cable:
        case ENTR_broadcastMedium_ota:
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    if ((tunerCfg->rfAttMode != ENTR_rfAttMode_seriesPin_shuntPin) && (tunerCfg->rfAttMode != ENTR_rfAttMode_seriesPin_internalAtt) && (tunerCfg->rfAttMode != ENTR_rfAttMode_seriesPin_shuntPin_internalAtt))
        return ENTR_err_invalid_parameter;

    if ((tunerCfg->specInvSel != ENTR_specInvSel_off) && (tunerCfg->specInvSel != ENTR_specInvSel_on))
        return ENTR_err_invalid_parameter;

    switch (tunerCfg->ifBw) {
        case ENTR_ifBw_6MHz:
        case ENTR_ifBw_7MHz:
        case ENTR_ifBw_8MHz:
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    if ((tunerCfg->FifHz < 0) || (tunerCfg->FifHz > tunerCfg->tunerData.FifMaxHz))
            return ENTR_err_invalid_parameter;

    if ((tunerCfg->ifOutputSel != ENTR_ifOutputSel_1) && (tunerCfg->ifOutputSel != ENTR_ifOutputSel_2))
        return ENTR_err_invalid_parameter;

    switch (tunerCfg->ifOutPutLevel) {
        case ENTR_ifOutPutLevel_neg3p2_dB:
        case ENTR_ifOutPutLevel_neg2p8_dB:
        case ENTR_ifOutPutLevel_neg2p3_dB:
        case ENTR_ifOutPutLevel_neg2p0_dB:
        case ENTR_ifOutPutLevel_neg1p6_dB:
        case ENTR_ifOutPutLevel_neg1p0_dB:
        case ENTR_ifOutPutLevel_neg0p5_dB:
        case ENTR_ifOutPutLevel_0_dB_nominal:
        case ENTR_ifOutPutLevel_0p6_dB:
        case ENTR_ifOutPutLevel_1p2_dB:
        case ENTR_ifOutPutLevel_1p8_dB:
        case ENTR_ifOutPutLevel_2p5_dB:
        case ENTR_ifOutPutLevel_3p3_dB:
        case ENTR_ifOutPutLevel_4p0_dB:
        case ENTR_ifOutPutLevel_5p0_dB:
        case ENTR_ifOutPutLevel_6p0_dB:
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    if ((tunerCfg->ifOutPutLevel < 0) || (tunerCfg->ifOutPutLevel > 15))
        return ENTR_err_invalid_parameter;

    if ((tunerCfg->agcCtrlMode != ENTR_agcCtrlMode_demod) && (tunerCfg->agcCtrlMode != ENTR_agcCtrlMode_internal))
        return ENTR_err_invalid_parameter;

    switch (tunerCfg->modulation) {
        case ENTR_modulation_DVB_C:
        case ENTR_modulation_J83_Annex_B:
        case ENTR_modulation_DVB_T:
        case ENTR_modulation_ATSC:
        case ENTR_modulation_ISDB_T:
        case ENTR_modulation_DMB_T:
        case ENTR_modulation_PAL:
        case ENTR_modulation_NTSC:
        case ENTR_modulation_SECAM:
        case ENTR_modulation_SECAM_L:
            break;
        default:
            return ENTR_err_invalid_parameter;
    }

    if ((tunerCfg->FchanHz < ENTR_FchanMin) || (tunerCfg->FchanHz > ENTR_FchanMax))
        return ENTR_err_invalid_parameter;
    /* End check param. */

    retVal = entr_cfgIc (tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgReadTmprAcc(tunerCfg, &tunerCfg->tunerData.curTemp);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgHystModeOff(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetFloHz(tunerCfg, tunerCfg->FchanHz);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetFspinHz(tunerCfg, tunerCfg->FchanHz - tunerCfg->tunerData.FloHz);
    if (retVal != ENTR_ok)
        return retVal;

    /* @doc Store channel frequency */
    tunerCfg->tunerData.FchanHz = tunerCfg->FchanHz;

    /* @doc Calculate and store channel frequency error */
    tunerCfg->FchanErrHz = tunerCfg->FchanHz - tunerCfg->tunerData.FloHz - tunerCfg->tunerData.FspinHz;

    retVal = entr_freqSetLc(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetTfBand(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_tfChanChg(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_freqSetLnaMode(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    retVal = entr_cfgHystModeOn(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* @doc Set set tuner configured to true */
    tunerCfg->tunerData.tunerCfg = ENTR_tunerCfg_yes;

    return ENTR_ok;
}

/*
 * entr_idle()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_invalid_parameter if bad input
 *          ENTR_err_rw otherwise.
 *
 * @note None.
 */
ENTR_return entr_idle(entr_config *tunerCfg)
{
    ENTR_return retVal;

    /* @doc Check input parameters */
    if ((tunerCfg->ltaEnaSel != ENTR_ltaEnaSel_off) && (tunerCfg->ltaEnaSel != ENTR_ltaEnaSel_on))
        return ENTR_err_invalid_parameter;

    retVal = entr_cfgIdle(tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    /* @doc set lta enable */
    if (tunerCfg->ltaEnaSel != tunerCfg->tunerData.ltaEnaSel) {
        if (tunerCfg->ltaEnaSel == ENTR_ltaEnaSel_on)
            retVal = entr_usrRegWr(tunerCfg, ENTR_ltaEna_reg, ENTR_ltaEna_mask, ENTR_ltaEna_on);
        else
            retVal = entr_usrRegWr(tunerCfg, ENTR_ltaEna_reg, ENTR_ltaEna_mask, ENTR_ltaEna_off);
        tunerCfg->tunerData.ltaEnaSel = tunerCfg->ltaEnaSel;
    }

    /* @doc Set tuner active to false */
    tunerCfg->tunerData.tunerActive = ENTR_tunerActive_no;

    return ENTR_ok;
}

/*
 * entr_idle()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 * @param[out] rssi
 *
 * @return  ENTR_err_not_active
 *          ENTR_err_not_implemented
 *
 * @note NOT IMPLEMENTED.
 */
ENTR_return entr_rssi(entr_config *tunerCfg)
{
    ENTR_uint8 retVal;

    /* @doc Check input parameters */
    if (tunerCfg->tunerData.tunerActive != ENTR_tunerActive_yes)
        return ENTR_err_not_active;

    retVal = entr_cfgRssi (tunerCfg);
    if (retVal != ENTR_ok)
        return retVal;

    return ENTR_ok;
}

/*
 * entr_status()
 *
 * @param[in, out] tunerCfg Tuner configuration structure.
 *
 * @return  ENTR_ok on success
 *          ENTR_err_freqPll_unlocked if not locked
 *          ENTR_err_rw otherwise.
 *
 * @note Return lock condition of pll. There are no other status indicators in the 4020.
 */
ENTR_return entr_status(entr_config *tunerCfg)
{
   ENTR_return retVal;
   ENTR_uint8 aLock;
   ENTR_uint8 dLock;

    /* @doc Check input parameters */
    if (tunerCfg->tunerData.tunerActive != ENTR_tunerActive_yes)
        return ENTR_err_not_active;

    /* @doc Read analog and digital lock. Return result */
    retVal = entr_usrRegRd (tunerCfg, ENTR_synAnLock_reg, ENTR_synAnLock_mask, &aLock);
    if (retVal != ENTR_ok)
        return retVal;
    retVal = entr_usrRegRd (tunerCfg, ENTR_synDigLock_reg, ENTR_synDigLock_mask, &dLock);
    if (retVal != ENTR_ok)
        return retVal;
    if ((aLock != ENTR_synAnLock_on) || (dLock != ENTR_synDigLock_on))
        return ENTR_err_freqPll_unlocked;

   return ENTR_ok;
}

/*
 * entr_extract_revstr()
 *
 * @param[in] revStr SVN Rev string..
 * @param[out] revision Extracted Revision..
 *
 * @return  ENTR_ok
 *
 * @note None.
 */

ENTR_return entr_extract_revstr(char revStr[], ENTR_uint16 *revision)
{
   int i, index;
   ENTR_uint16 rev = 0;

   // Increment to rev number in string where rev string is of format:
   //      "$xRev: 11927 $"
   for (index = 4; revStr[index] != ' ' && revStr[index] != '\0'; index++)
      ;
   index++;

   // Calculate integer from string
   for (i = index; revStr[i] != ' ' && revStr[i] != '\0'; i++) {
      rev = rev * 10;
      rev = rev +  (revStr[i] - '0');
   }

   *revision = rev;
   return ENTR_ok;
}


/*******/
#define ENTR_codeRev "$Rev: 11943 $"
/*******/

/*
 * entr_code_revision()
 *
 * @param[out] revision Current revision..
 *
 * @return  ENTR_ok
 *
 * @note None.
 */
ENTR_return entr_code_revision(ENTR_uint16 *revision)
{
    ENTR_uint16 hRev, cRev;
    char hRevStr[] = {ENTR_headerRev};
    char cRevStr[] = {ENTR_codeRev};

    /* @doc Return svn revision */
    entr_extract_revstr(hRevStr, &hRev);
    entr_extract_revstr(cRevStr, &cRev);

    *revision = hRev > cRev ? hRev : cRev;

    return ENTR_ok;
}

#endif
