/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim_stv0362.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  02.04.2008      WeiChing Tu     Ver 0.1     Create file.
*
*****************************************************************************/

#ifndef __LLD_NIM_STV0362_H__
#define __LLD_NIM_STV0362_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <api/libc/printf.h>


//note: copied from ST code: src\drivers\ter\include\d0362_map.h --- [begin]
/* STV0362 Registers and Fields difinitions */

/*  PLLNDIV */
#define R0362_PLLNDIV                   0x00d9
#define F0362_PLL_NDIV                  0x00d900ff

/*  ID  */
#define R0362_ID                        0x0000
#define F0362_IDENTIFICATIONREGISTER    0x000000ff

/*  I2CRPT  */
#define R0362_I2CRPT                    0x0001
#define F0362_I2CT_ON                   0x00010080
#define F0362_ENARPT_LEVEL         0x00010070
#define F0362_SCLT_DELAY             0x00010008
#define F0362_SCLT_NOD                 0x00010004
#define F0362_STOP_ENABLE           0x00010002
#define F0362_SDAT_NOD                0x00010001

/*  TOPCTRL */
#define R0362_TOPCTRL                   0x0002
#define F0362_STDBY                     0x00020080
#define F0362_STDBY_FEC             0x00020040
#define F0362_STDBY_CORE           0x00020020
#define F0362_DIR_CLK_54            0x00020010
#define F0362_TS_DIS                    0x00020008
#define F0362_DIR_CLK_108           0x00020004
#define F0362_TUNER_BB                0x00020002
#define F0362_DVBT_H                    0x00020001

/*  IOCFG0  */
#define R0362_IOCFG0                    0x0003
#define F0362_OP0_SD                    0x00030080
#define F0362_OP0_VAL                   0x00030040
#define F0362_OP0_OD                    0x00030020
#define F0362_OP0_INV                   0x00030010
#define F0362_OP0_DACVALUE_HI           0x0003000f

/*  DAC0R   */
#define R0362_DAC0R                     0x0004
#define F0362_OP0_DACVALUE_LO           0x000400ff

/*  IOCFG1  */
#define R0362_IOCFG1                    0x0005
#define F0362_IP0                       0x00050040
#define F0362_OP1_OD                    0x00050020
#define F0362_OP1_INV                   0x00050010
#define F0362_OP1_DACVALUE_HI           0x0005000f

/*  DAC1R   */
#define R0362_DAC1R                     0x0006
#define F0362_OP1_DACVALUE_LO           0x000600ff

/*  IOCFG2  */
#define R0362_IOCFG2                    0x0007
#define F0362_OP2_LOCK_CONF             0x000700e0
#define F0362_OP2_OD                    0x00070010
#define F0362_OP2_VAL                   0x00070008
#define F0362_OP1_LOCK_CONF             0x00070007

/*  SDFR    */
#define R0362_SDFR                      0x0008
#define F0362_OP0_FREQ                  0x000800f0
#define F0362_OP1_FREQ                  0x0008000f

/*  STATUS  */
#define R0362_STATUS                    0x0009
#define F0362_TPS_LOCK                  0x00090080
#define F0362_SYR_LOCK                  0x00090040
#define F0362_AGC_LOCK                  0x00090020
#define F0362_PRF                       0x00090010
#define F0362_LK                        0x00090008
#define F0362_PR                        0x00090007

/*  AUX_CLK */
#define R0362_AUX_CLK                   0x000a
#define F0362_AUXFEC_CTL                0x000a00c0
#define F0362_DIS_CKX4                  0x000a0020
#define F0362_CKSEL                     0x000a0018
#define F0362_CKDIV_PROG                0x000a0006
#define F0362_AUXCLK_ENA                0x000a0001

/*  FREESYS1    */
#define R0362_FREESYS1                  0x000b
#define F0362_FREESYS1                  0x000b00ff

/*  FREESYS2    */
#define R0362_FREESYS2                  0x000c
#define F0362_FREESYS2                  0x000c00ff

/*  FREESYS3    */
#define R0362_FREESYS3                  0x000d
#define F0362_FREESYS3                  0x000d00ff

/*  AGC2MAX */
#define R0362_AGC2MAX                   0x0010
#define F0362_AGC2MAX                   0x001000ff

/*  AGC2MIN */
#define R0362_AGC2MIN                   0x0011
#define F0362_AGC2MIN                   0x001100ff

/*  AGC1MAX */
#define R0362_AGC1MAX                   0x0012
#define F0362_AGC1MAX                   0x001200ff

/*  AGC1MIN */
#define R0362_AGC1MIN                   0x0013
#define F0362_AGC1MIN                   0x001300ff

/*  AGCR    */
#define R0362_AGCR                      0x0014
#define F0362_RATIO_A                   0x001400e0
#define F0362_RATIO_B                   0x00140018
#define F0362_RATIO_C                   0x00140007

/*  AGC2TH  */
#define R0362_AGC2TH                    0x0015
#define F0362_AGC2_THRES                0x001500ff

/*  AGC12C  */
#define R0362_AGC12C                    0x0016
#define F0362_AGC1_IV                   0x00160080
#define F0362_AGC1_OD                   0x00160040
#define F0362_AGC1_LOAD                 0x00160020
#define F0362_AGC2_IV                   0x00160010
#define F0362_AGC2_OD                   0x00160008
#define F0362_AGC2_LOAD                 0x00160004
#define F0362_AGC12_MODE                0x00160003

/*  AGCCTRL1    */
#define R0362_AGCCTRL1                  0x0017
#define F0362_DAGC_ON                   0x00170080
#define F0362_INVERT_AGC12              0x00170040
#define F0362_AGC1_MODE                 0x00170008
#define F0362_AGC2_MODE                 0x00170007

/*  AGCCTRL2    */
#define R0362_AGCCTRL2                  0x0018
#define F0362_FRZ2_CTRL                 0x00180060
#define F0362_FRZ1_CTRL                 0x00180018
#define F0362_TIME_CST                  0x00180007

/*  AGC1VAL1    */
#define R0362_AGC1VAL1                  0x0019
#define F0362_AGC1_VAL_LO               0x001900ff

/*  AGC1VAL2    */
#define R0362_AGC1VAL2                  0x001a
#define F0362_AGC1_VAL_HI               0x001a000f

/*  AGC2VAL1    */
#define R0362_AGC2VAL1                  0x001b
#define F0362_AGC2_VAL_LO               0x001b00ff

/*  AGC2VAL2    */
#define R0362_AGC2VAL2                  0x001c
#define F0362_AGC2_VAL_HI               0x001c000f

/*  AGC2PGA */
#define R0362_AGC2PGA                   0x001d
#define F0362_AGC2PGA                   0x001d00ff

/*  OVF_RATE1   */
#define R0362_OVF_RATE1                 0x001e
#define F0362_OVF_RATE_HI               0x001e000f

/*  OVF_RATE2   */
#define R0362_OVF_RATE2                 0x001f
#define F0362_OVF_RATE_LO               0x001f00ff

/*  GAIN_SRC1   */
#define R0362_GAIN_SRC1                 0x0020
#define F0362_INV_SPECTR                0x00200080
#define F0362_IQ_INVERT                 0x00200040
#define F0362_INR_BYPASS                0x00200020
#define F0362_INS_BYPASS                0x00200010
#define F0362_GAIN_SRC_HI               0x0020000f

/*  GAIN_SRC2   */
#define R0362_GAIN_SRC2                 0x0021
#define F0362_GAIN_SRC_LO               0x002100ff

/*  INC_DEROT1  */
#define R0362_INC_DEROT1                0x0022
#define F0362_INC_DEROT_HI              0x002200ff

/*  INC_DEROT2  */
#define R0362_INC_DEROT2                0x0023
#define F0362_INC_DEROT_LO              0x002300ff

/*  PPM_CPAMP_DIR   */
#define R0362_PPM_CPAMP_DIR             0x0024
#define F0362_PPM_CPAMP_DIRECT          0x002400ff

/*  PPM_CPAMP_INV   */
#define R0362_PPM_CPAMP_INV             0x0025
#define F0362_PPM_CPAMP_INV             0x002500ff

/*  FREESTFE_1  */
#define R0362_FREESTFE_1                0x0026
#define F0362_SYMBOL_NUMBER_INC         0x002600c0
#define F0362_SEL_LSB                   0x00260004
#define F0362_AVERAGE_ON                0x00260002
#define F0362_DC_ADJ                    0x00260001

/*  FREESTFE_2  */
#define R0362_FREESTFE_2                0x0027
#define F0362_SEL_SRCOUT                0x002700c0
#define F0362_SEL_SYRTHR                0x0027001f

/*  DCOFFSET    */
#define R0362_DCOFFSET                  0x0028
#define F0362_SELECT_I_Q                0x00280080
#define F0362_DC_OFFSET                 0x0028007f

/*  EN_PROCESS  */
#define R0362_EN_PROCESS                0x0029
#define F0362_INS_NIN_INDEX             0x002900f0
#define F0362_ENAB_MANUAL               0x00290001

/*  SDI_SMOOTHER    */
#define R0362_SDI_SMOOTHER              0x002a
#define F0362_DIS_SMOOTH                0x002a0080
#define F0362_SDI_INC_SMOOTHER          0x002a007f

/*  FE_LOOP_OPEN    */
#define R0362_FE_LOOP_OPEN              0x002b
#define F0362_TRL_LOOP_OP               0x002b0002
#define F0362_CRL_LOOP_OP               0x002b0001

/*  FREQOFF1    */
#define R0362_FREQOFF1                  0x002c
#define F0362_FREQ_OFFSET_LOOP_OPEN_VHI 0x002c00ff

/*  FREQOFF2    */
#define R0362_FREQOFF2                  0x002d
#define F0362_FREQ_OFFSET_LOOP_OPEN_HI  0x002d00ff

/*  FREQOFF3    */
#define R0362_FREQOFF3                  0x002e
#define F0362_FREQ_OFFSET_LOOP_OPEN_LO  0x002e00ff

/*  TIMOFF1 */
#define R0362_TIMOFF1                   0x002f
#define F0362_TIM_OFFSET_LOOP_OPEN_HI   0x002f00ff

/*  TIMOFF2 */
#define R0362_TIMOFF2                   0x0030
#define F0362_TIM_OFFSET_LOOP_OPEN_LO   0x003000ff

/*  EPQ */
#define R0362_EPQ                       0x0031
#define F0362_EPQ                       0x003100ff

/*  EPQAUTO */
#define R0362_EPQAUTO                   0x0032
#define F0362_EPQ2                      0x003200ff

/*  CHP_TAPS    */
#define R0362_CHP_TAPS                  0x0033
#define F0362_SCAT_FILT_EN              0x00330002
#define F0362_TAPS_EN                   0x00330001

/*  CHP_DYN_COEFF   */
#define R0362_CHP_DYN_COEFF             0x0034
#define F0362_CHP_DYNAM_COEFFCIENT      0x003400ff

/*  PPM_STATE_MAC   */
#define R0362_PPM_STATE_MAC             0x0035
#define F0362_PPM_STATE_MACHINE_DECODER 0x0035003f

/*  INR_THRESHOLD   */
#define R0362_INR_THRESHOLD             0x0036
#define F0362_INR_THRESHOLD             0x003600ff

/*  EPQ_TPS_ID_CELL */
#define R0362_EPQ_TPS_ID_CELL           0x0037
#define F0362_DIS_TPS_RSVD              0x00370040
#define F0362_DIS_BCH                   0x00370020
#define F0362_DIS_ID_CEL                0x00370010
#define F0362_HOLD_SLOPE                0x00370008
#define F0362_TPS_ADJUST_SYM            0x00370007

/*  EPQ_CFG */
#define R0362_EPQ_CFG                   0x0038
#define F0362_EPQ_RANGE                 0x00380002
#define F0362_EPQ_SOFT                  0x00380001

/*  EPQ_STATUS  */
#define R0362_EPQ_STATUS                0x0039
#define F0362_SLOPE_INC                 0x003900fc
#define F0362_TPS_FIELD                 0x00390003

/*  FECM    */
#define R0362_FECM                      0x0040
#define F0362_FEC_MODE                  0x004000f0
#define F0362_VIT_DIFF                  0x00400004
#define F0362_SYNC                      0x00400002
#define F0362_SYM                       0x00400001

/*  VTH0    */
#define R0362_VTH0                      0x0041
#define F0362_VTH0                      0x0041007f

/*  VTH1    */
#define R0362_VTH1                      0x0042
#define F0362_VTH1                      0x0042007f

/*  VTH2    */
#define R0362_VTH2                      0x0043
#define F0362_VTH2                      0x0043007f

/*  VTH3    */
#define R0362_VTH3                      0x0044
#define F0362_VTH3                      0x0044007f

/*  VTH4    */
#define R0362_VTH4                      0x0045
#define F0362_VTH4                      0x0045007f

/*  VTH5    */
#define R0362_VTH5                      0x0046
#define F0362_VTH5                      0x0046007f

/*  FREEVIT */
#define R0362_FREEVIT                   0x0047
#define F0362_FREEVIT                   0x004700ff

/*  VITPROG */
#define R0362_VITPROG                   0x0049
#define F0362_FORCE_ROTA                0x004900c0
#define F0362_AUTO_FREEZE               0x00490030
#define F0362_MDIVIDER                  0x00490003

/*  PR  */
#define R0362_PR                        0x004a
#define F0362_FRAPTCR                   0x004a0080
#define F0362_E7_8                      0x004a0020
#define F0362_E6_7                      0x004a0010
#define F0362_E5_6                      0x004a0008
#define F0362_E3_4                      0x004a0004
#define F0362_E2_3                      0x004a0002
#define F0362_E1_2                      0x004a0001

/*  VSEARCH */
#define R0362_VSEARCH                   0x004b
#define F0362_PR_AUTO                   0x004b0080
#define F0362_PR_FREEZE                 0x004b0040
#define F0362_SAMPNUM                   0x004b0030
#define F0362_TIMEOUT                   0x004b000c
#define F0362_HYSTER                    0x004b0003

/*  RS  */
#define R0362_RS                        0x004c
#define F0362_DEINT_ENA                 0x004c0080
#define F0362_OUTRS_SP                  0x004c0040
#define F0362_RS_ENA                    0x004c0020
#define F0362_DESCR_ENA                 0x004c0010
#define F0362_ERRBIT_ENA                0x004c0008
#define F0362_FORCE47                   0x004c0004
#define F0362_CLK_POL                   0x004c0002
#define F0362_CLK_CFG                   0x004c0001

/*  RSOUT   */
#define R0362_RSOUT                     0x004d
#define F0362_ENA_STBACKEND             0x004d0010
#define F0362_ENA8_LEVEL                0x004d000f

/*  ERRCTRL1    */
#define R0362_ERRCTRL1                  0x004e
#define F0362_ERRMODE1                  0x004e0080
#define F0362_TESTERS1                  0x004e0040
#define F0362_ERR_SOURCE1               0x004e0030
#define F0362_RESET_CNTR1               0x004e0004
#define F0362_NUM_EVENT1                0x004e0003

/*  ERRCNTM1    */
#define R0362_ERRCNTM1                  0x004f
#define F0362_ERROR_COUNT1_HI           0x004f00ff

/*  ERRCNTL1    */
#define R0362_ERRCNTL1                  0x0050
#define F0362_ERROR_COUNT1_LO           0x005000ff

/*  ERRCTRL2    */
#define R0362_ERRCTRL2                  0x0051
#define F0362_ERRMODE2                  0x00510080
#define F0362_TESTERS2                  0x00510040
#define F0362_ERR_SOURCE2               0x00510030
#define F0362_RESET_CNTR2               0x00510004
#define F0362_NUM_EVENT2                0x00510003

/*  ERRCNTM2    */
#define R0362_ERRCNTM2                  0x0052
#define F0362_ERROR_COUNT2_HI           0x005200ff

/*  ERRCNTL2    */
#define R0362_ERRCNTL2                  0x0053
#define F0362_ERROR_COUNT2_LO           0x005300ff

/*  FREEDRS */
#define R0362_FREEDRS                   0x0054
#define F0362_FREEDRS                   0x005400ff

/*  VERROR  */
#define R0362_VERROR                    0x0055
#define F0362_ERROR_VALUE               0x005500ff

/*  ERRCTRL3    */
#define R0362_ERRCTRL3                  0x0056
#define F0362_ERRMODE3                  0x00560080
#define F0362_TESTERS3                  0x00560040
#define F0362_ERR_SOURCE3               0x00560030
#define F0362_RESET_CNTR3               0x00560004
#define F0362_NUM_EVENT3                0x00560003

/*  ERRCNTM3    */
#define R0362_ERRCNTM3                  0x0057
#define F0362_ERROR_COUNT3_HI           0x005700ff

/*  ERRCNTL3    */
#define R0362_ERRCNTL3                  0x0058
#define F0362_ERROR_COUNT3_LO           0x005800ff

/*  DILSTK1 */
#define R0362_DILSTK1                   0x0059
#define F0362_DILSTK_HI                 0x005900ff

/*  DILSTK0 */
#define R0362_DILSTK0                   0x005a
#define F0362_DILSTK_LO                 0x005a00ff

/*  DILBWSTK1   */
#define R0362_DILBWSTK1                 0x005b
#define F0362_DILBWSTK1                 0x005b00ff

/*  DILBWST0    */
#define R0362_DILBWST0                  0x005c
#define F0362_DILBWST0                  0x005c00ff

/*  LNBRX   */
#define R0362_LNBRX                     0x005d
#define F0362_LINE_OK                   0x005d0080
#define F0362_OCCURRED_ERR              0x005d0040
#define F0362_RSOV_DATAIN               0x005d0008
#define F0362_LNBTX_CHIPADDR            0x005d0007

/*  RSTC    */
#define R0362_RSTC                      0x005e
#define F0362_DEINTTC                   0x005e0080
#define F0362_DIL64_ON                  0x005e0040
#define F0362_RSTC                      0x005e0020
#define F0362_DESCRAMTC                 0x005e0010
#define F0362_MODSYNCBYT                0x005e0004
#define F0362_LOWP_DIS                  0x005e0002
#define F0362_HIGHP_DIS                 0x005e0001

/*  VIT_BIST    */
#define R0362_VIT_BIST                  0x005f
#define F0362_RAND_RAMP                 0x005f0040
#define F0362_NOISE_LEVEL               0x005f0038
#define F0362_PR_VIT_BIST               0x005f0007

/*  IIR_CELL_NB */
#define R0362_IIR_CELL_NB               0x0060
#define F0362_NRST_IIR                  0x00600080
#define F0362_IIR_CELL_NB               0x00600007

/*  IIR_CX_COEFF1_MSB   */
#define R0362_IIR_CX_COEFF1_MSB         0x0061
#define F0362_IIR_CX_COEFF1_MSB         0x006100ff

/*  IIR_CX_COEFF1_LSB   */
#define R0362_IIR_CX_COEFF1_LSB         0x0062
#define F0362_IIR_CX_COEFF1_LSB         0x006200ff

/*  IIR_CX_COEFF2_MSB   */
#define R0362_IIR_CX_COEFF2_MSB         0x0063
#define F0362_IIR_CX_COEFF2_MSB         0x006300ff

/*  IIR_CX_COEFF2_LSB   */
#define R0362_IIR_CX_COEFF2_LSB         0x0064
#define F0362_IIR_CX_COEFF2_LSB         0x006400ff

/*  IIR_CX_COEFF3_MSB   */
#define R0362_IIR_CX_COEFF3_MSB         0x0065
#define F0362_IIR_CX_COEFF3_MSB         0x006500ff

/*  IIR_CX_COEFF3_LSB   */
#define R0362_IIR_CX_COEFF3_LSB         0x0066
#define F0362_IIR_CX_COEFF3_LSB         0x006600ff

/*  IIR_CX_COEFF4_MSB   */
#define R0362_IIR_CX_COEFF4_MSB         0x0067
#define F0362_IIR_CX_COEFF4_MSB         0x006700ff

/*  IIR_CX_COEFF4_LSB   */
#define R0362_IIR_CX_COEFF4_LSB         0x0068
#define F0362_IIR_CX_COEFF4_LSB         0x006800ff

/*  IIR_CX_COEFF5_MSB   */
#define R0362_IIR_CX_COEFF5_MSB         0x0069
#define F0362_IIR_CX_COEFF5_MSB         0x006900ff

/*  IIR_CX_COEFF5_LSB   */
#define R0362_IIR_CX_COEFF5_LSB         0x006a
#define F0362_IIR_CX_COEFF5_LSB         0x006a00ff

/*  FEPATH_CFG  */
#define R0362_FEPATH_CFG                0x006b
#define F0362_DEMUX_SWAP                0x006b0004
#define F0362_DIGAGC_SWAP               0x006b0002
#define F0362_LONGPATH_IF               0x006b0001

/*  PMC1_FUNC   */
#define R0362_PMC1_FUNC                 0x006c
#define F0362_SOFT_RSTN                 0x006c0080
#define F0362_PMC1_AVERAGE_TIME         0x006c0078
#define F0362_PMC1_WAIT_TIME            0x006c0006
#define F0362_PMC1_2N_SEL               0x006c0001

/*  PMC1_FORCE  */
#define R0362_PMC1_FORCE                0x006d
#define F0362_PMC1_FORCE                0x006d0080
#define F0362_PMC1_FORCE_VALUE          0x006d007c

/*  PMC2_FUNC   */
#define R0362_PMC2_FUNC                 0x006e
#define F0362_PMC2_SOFT_STN             0x006e0080
#define F0362_PMC2_ACCU_TIME            0x006e0070
#define F0362_PMC2_CMDP_MN              0x006e0008
#define F0362_PMC2_SWAP                 0x006e0004

/*  DIG_AGC_R   */
#define R0362_DIG_AGC_R                 0x0070
#define F0362_COM_SOFT_RSTN             0x00700080
#define F0362_COM_AGC_ON                0x00700040
#define F0362_COM_EARLY                 0x00700020
#define F0362_AUT_SOFT_RESETN           0x00700010
#define F0362_AUT_AGC_ON                0x00700008
#define F0362_AUT_EARLY                 0x00700004
#define F0362_AUT_ROT_EN                0x00700002
#define F0362_LOCK_SOFT_RESETN          0x00700001

/*  COMAGC_TARMSB   */
#define R0362_COMAGC_TARMSB             0x0071
#define F0362_COM_AGC_TARGET_MSB        0x007100ff

/*  COM_AGC_TAR_ENMODE  */
#define R0362_COM_AGC_TAR_ENMODE        0x0072
#define F0362_COM_AGC_TARGET_LSB        0x007200f0
#define F0362_COM_ENMODE                0x0072000f

/*  COM_AGC_CFG */
#define R0362_COM_AGC_CFG               0x0073
#define F0362_COM_N                     0x007300f8
#define F0362_COM_STABMODE              0x00730006
#define F0362_ERR_SEL                   0x00730001

/*  COM_AGC_GAIN1   */
#define R0362_COM_AGC_GAIN1             0x0074
#define F0362_COM_GAIN1ACK              0x007400f0
#define F0362_COM_GAIN1TRK              0x0074000f

/*  AUT_AGC_TARGET_MSB  */
#define R0362_AUT_AGC_TARGET_MSB        0x0075
#define F0362_AUT_AGC_TARGET_MSB        0x007500ff

/*  LOCK_DETECT_MSB */
#define R0362_LOCK_DETECT_MSB           0x0076
#define F0362_LOCK_DETECT_MSB           0x007600ff

/*  AGCTAR_LOCK_LSBS    */
#define R0362_AGCTAR_LOCK_LSBS          0x0077
#define F0362_LOCK_DETECT_LSB           0x007700f0
#define F0362_AUT_AGC_TARGET_LSB        0x0077000f

/*  AUT_GAIN_EN */
#define R0362_AUT_GAIN_EN               0x0078
#define F0362_AUT_ENMODE                0x007800f0
#define F0362_AUT_GAIN2                 0x0078000f

/*  AUT_CFG */
#define R0362_AUT_CFG                   0x0079
#define F0362_AUT_N                     0x007900f8
#define F0362_INT_CHOICE                0x00790006
#define F0362_INT_LOAD                  0x00790001

/*  LOCKN   */
#define R0362_LOCKN                     0x007a
#define F0362_LOCKN                     0x007a00f8
#define F0362_SEL_IQNTAR                0x007a0004
#define F0362_LOCK_DETECT_CHOICE        0x007a0003

/*  INT_X_3 */
#define R0362_INT_X_3                   0x007b
#define F0362_INT_X3                    0x007b00ff

/*  INT_X_2 */
#define R0362_INT_X_2                   0x007c
#define F0362_INT_X2                    0x007c00ff

/*  INT_X_1 */
#define R0362_INT_X_1                   0x007d
#define F0362_INT_X1                    0x007d00ff

/*  INT_X_0 */
#define R0362_INT_X_0                   0x007e
#define F0362_INT_X0                    0x007e00ff

/*  MIN_ERR_X_MSB   */
#define R0362_MIN_ERR_X_MSB             0x007f
#define F0362_MIN_ERR_X_MSB             0x007f00ff

/*  STATUS_ERR_DA   */
#define R0362_STATUS_ERR_DA             0x006f
#define F0362_COM_USEGAINTRK            0x006f0080
#define F0362_COM_AGCLOCK               0x006f0040
#define F0362_AUT_AGCLOCK               0x006f0020
#define F0362_MIN_ERR_X_LSB             0x006f000f

/*  COR_CTL */
#define R0362_COR_CTL                   0x0080
#define F0362_CORE_ACTIVE               0x00800020
#define F0362_HOLD                      0x00800010
#define F0362_CORE_STATE_CTL            0x0080000f

/*  COR_STAT    */
#define R0362_COR_STAT                  0x0081
#define F0362_SCATT_LOCKED              0x00810080
#define F0362_TPS_LOCKED                0x00810040
#define F0362_SYR_LOCKED_COR            0x00810020
#define F0362_AGC_LOCKED_STAT           0x00810010
#define F0362_CORE_STATE_STAT           0x0081000f

/*  COR_INTEN   */
#define R0362_COR_INTEN                 0x0082
#define F0362_INTEN                     0x00820080
#define F0362_INTEN_SYR                 0x00820020
#define F0362_INTEN_FFT                 0x00820010
#define F0362_INTEN_AGC                 0x00820008
#define F0362_INTEN_TPS1                0x00820004
#define F0362_INTEN_TPS2                0x00820002
#define F0362_INTEN_TPS3                0x00820001

/*  COR_INTSTAT */
#define R0362_COR_INTSTAT               0x0083
#define F0362_INTSTAT_SYR               0x00830020
#define F0362_INTSTAT_FFT               0x00830010
#define F0362_INTSAT_AGC                0x00830008
#define F0362_INTSTAT_TPS1              0x00830004
#define F0362_INTSTAT_TPS2              0x00830002
#define F0362_INTSTAT_TPS3              0x00830001

/*  COR_MODEGUARD   */
#define R0362_COR_MODEGUARD             0x0084
#define F0362_FORCE                     0x00840010
#define F0362_MODE                      0x0084000c
#define F0362_GUARD                     0x00840003

/*  AGC_CTL */
#define R0362_AGC_CTL                   0x0085
#define F0362_AGC_TIMING_FACTOR         0x008500e0
#define F0362_AGC_LAST                  0x00850010
#define F0362_AGC_GAIN                  0x0085000c
#define F0362_AGC_NEG                   0x00850002
#define F0362_AGC_SET                   0x00850001

/*  AGC_MANUAL1 */
#define R0362_AGC_MANUAL1               0x0086
#define F0362_AGC_VAL_LO                0x008600ff

/*  AGC_MANUAL2 */
#define R0362_AGC_MANUAL2               0x0087
#define F0362_AGC_VAL_HI                0x0087000f

/*  AGC_TARGET  */
#define R0362_AGC_TARGET                0x0088
#define F0362_AGC_TARGET                0x008800ff

/*  AGC_GAIN1   */
#define R0362_AGC_GAIN1                 0x0089
#define F0362_AGC_GAIN_LO               0x008900ff

/*  AGC_GAIN2   */
#define R0362_AGC_GAIN2                 0x008a
#define F0362_AGC_LOCKED_GAIN2          0x008a0010
#define F0362_AGC_GAIN_HI               0x008a000f

/*  RESERVED_1  */
#define R0362_RESERVED_1                0x008b
#define F0362_RESERVED_1                0x008b00ff

/*  RESERVED_2  */
#define R0362_RESERVED_2                0x008c
#define F0362_RESERVED_2                0x008c00ff

/*  RESERVED_3  */
#define R0362_RESERVED_3                0x008d
#define F0362_RESERVED_3                0x008d00ff

/*  CAS_CTL */
#define R0362_CAS_CTL                   0x008e
#define F0362_CCS_ENABLE                0x008e0080
#define F0362_ACS_DISABLE               0x008e0040
#define F0362_DAGC_DIS                  0x008e0020
#define F0362_DAGC_GAIN                 0x008e0018
#define F0362_CCSMU                     0x008e0007

/*  CAS_FREQ    */
#define R0362_CAS_FREQ                  0x008f
#define F0362_CCS_FREQ                  0x008f00ff

/*  CAS_DAGCGAIN    */
#define R0362_CAS_DAGCGAIN              0x0090
#define F0362_CAS_DAGC_GAIN             0x009000ff

/*  SYR_CTL */
#define R0362_SYR_CTL                   0x0091
#define F0362_SICTH_ENABLE              0x00910080
#define F0362_LONG_ECHO                 0x00910078
#define F0362_AUTO_LE_EN                0x00910004
#define F0362_SYR_BYPASS                0x00910002
#define F0362_SYR_TR_DIS                0x00910001

/*  SYR_STAT    */
#define R0362_SYR_STAT                  0x0092
#define F0362_SYR_LOCKED_STAT           0x00920010
#define F0362_SYR_MODE                  0x00920004
#define F0362_SYR_GUARD                 0x00920003

/*  SYR_NCO1    */
#define R0362_SYR_NCO1                  0x0093
#define F0362_SYR_NCO_LO                0x009300ff

/*  SYR_NCO2    */
#define R0362_SYR_NCO2                  0x0094
#define F0362_SYR_NCO_HI                0x0094003f

/*  SYR_OFFSET1 */
#define R0362_SYR_OFFSET1               0x0095
#define F0362_SYR_OFFSET_LO             0x009500ff

/*  SYR_OFFSET2 */
#define R0362_SYR_OFFSET2               0x0096
#define F0362_SYR_OFFSET_HI             0x0096003f

/*  FFT_CTL */
#define R0362_FFT_CTL                   0x0097
#define F0362_SHIFT_FFT_TRIG            0x00970018
#define F0362_FFT_TRIGGER               0x00970004
#define F0362_FFT_MANUAL                0x00970002
#define F0362_IFFT_MODE                 0x00970001

/*  SCR_CTL */
#define R0362_SCR_CTL                   0x0098
#define F0362_SYRADJDECAY               0x00980070
#define F0362_SCR_CPEDIS                0x00980002
#define F0362_SCR_DIS                   0x00980001

/*  PPM_CTL1    */
#define R0362_PPM_CTL1                  0x0099
#define F0362_MEAN_OFF                  0x00990080
#define F0362_GRAD_OFF                  0x00990040
#define F0362_PPM_MAXFREQ               0x00990030
#define F0362_PPM_MAXTIM                0x00990008
#define F0362_PPM_INVSEL                0x00990004
#define F0362_PPM_SCATDIS               0x00990002
#define F0362_PPM_BYP                   0x00990001

/*  TRL_CTL */
#define R0362_TRL_CTL                   0x009a
#define F0362_TRL_NOMRATE_LSB           0x009a0080
#define F0362_TRL_GAIN_FACTOR           0x009a0078
#define F0362_TRL_LOOPGAIN              0x009a0007

/*  TRL_NOMRATE1    */
#define R0362_TRL_NOMRATE1              0x009b
#define F0362_TRL_NOMRATE_LO            0x009b00ff

/*  TRL_NOMRATE2    */
#define R0362_TRL_NOMRATE2              0x009c
#define F0362_TRL_NOMRATE_HI            0x009c00ff

/*  TRL_TIME1   */
#define R0362_TRL_TIME1                 0x009d
#define F0362_TRL_TOFFSET_LO            0x009d00ff

/*  TRL_TIME2   */
#define R0362_TRL_TIME2                 0x009e
#define F0362_TRL_TOFFSET_HI            0x009e00ff

/*  CRL_CTL */
#define R0362_CRL_CTL                   0x009f
#define F0362_CRL_DIS                   0x009f0080
#define F0362_CRL_GAIN_FACTOR           0x009f0078
#define F0362_CRL_LOOPGAIN              0x009f0007

/*  CRL_FREQ1   */
#define R0362_CRL_FREQ1                 0x00a0
#define F0362_CRL_FOFFSET_LO            0x00a000ff

/*  CRL_FREQ2   */
#define R0362_CRL_FREQ2                 0x00a1
#define F0362_CRL_FOFFSET_HI            0x00a100ff

/*  CRL_FREQ3   */
#define R0362_CRL_FREQ3                 0x00a2
#define F0362_CRL_FOFFSET_VHI           0x00a200ff

/*  CHC_CTL1    */
#define R0362_CHC_CTL1                  0x00a3
#define F0362_MEAN_PILOT_GAIN           0x00a300e0
#define F0362_MANMEANP                  0x00a30010
#define F0362_DBADP                     0x00a30008
#define F0362_DNOISEN                   0x00a30004
#define F0362_DCHCPRED                  0x00a30002
#define F0362_CHC_INT                   0x00a30001

/*  CHC_SNR */
#define R0362_CHC_SNR                   0x00a4
#define F0362_CHC_SNR                   0x00a400ff

/*  BDI_CTL */
#define R0362_BDI_CTL                   0x00a5
#define F0362_BDI_LPSEL                 0x00a50002
#define F0362_BDI_SERIAL                0x00a50001

/*  DMP_CTL */
#define R0362_DMP_CTL                   0x00a6
#define F0362_DMP_SCALING_FACTOR        0x00a6001e
#define F0362_DMP_SDDIS                 0x00a60001

/*  TPS_RCVD1   */
#define R0362_TPS_RCVD1                 0x00a7
#define F0362_TPS_CHANGE                0x00a70040
#define F0362_BCH_OK                    0x00a70020
#define F0362_TPS_SYNC                  0x00a70010
#define F0362_TPS_FRAME                 0x00a70003

/*  TPS_RCVD2   */
#define R0362_TPS_RCVD2                 0x00a8
#define F0362_TPS_HIERMODE              0x00a80070
#define F0362_TPS_CONST                 0x00a80003

/*  TPS_RCVD3   */
#define R0362_TPS_RCVD3                 0x00a9
#define F0362_TPS_LPCODE                0x00a90070
#define F0362_TPS_HPCODE                0x00a90007

/*  TPS_RCVD4   */
#define R0362_TPS_RCVD4                 0x00aa
#define F0362_TPS_GUARD                 0x00aa0030
#define F0362_TPS_MODE                  0x00aa0003

/*  TPS_ID_CELL1    */
#define R0362_TPS_ID_CELL1              0x00ab
#define F0362_TPS_ID_CELL_LO            0x00ab00ff

/*  TPS_ID_CELL2    */
#define R0362_TPS_ID_CELL2              0x00ac
#define F0362_TPS_ID_CELL_HI            0x00ac00ff

/*  TPS_RCVD5_SET1  */
#define R0362_TPS_RCVD5_SET1            0x00ad
#define F0362_TPS_NA                    0x00ad00fc
#define F0362_TPS_SETFRAME              0x00ad0003

/*  TPS_SET2    */
#define R0362_TPS_SET2                  0x00ae
#define F0362_TPS_SETHIERMODE           0x00ae0070
#define F0362_TPS_SETCONST              0x00ae0003

/*  TPS_SET3    */
#define R0362_TPS_SET3                  0x00af
#define F0362_TPS_SETLPCODE             0x00af0070
#define F0362_TPS_SETHPCODE             0x00af0007

/*  TPS_CTL */
#define R0362_TPS_CTL                   0x00b0
#define F0362_TPS_IMM                   0x00b00004
#define F0362_TPS_BCHDIS                0x00b00002
#define F0362_TPS_UPDDIS                0x00b00001

/*  CTL_FFTOSNUM    */
#define R0362_CTL_FFTOSNUM              0x00b1
#define F0362_SYMBOL_NUMBER             0x00b1007f

/*  TESTSELECT  */
#define R0362_TESTSELECT                0x00b2
#define F0362_TESTSELECT                0x00b2001f

/*  MSC_REV */
#define R0362_MSC_REV                   0x00b3
#define F0362_REV_NUMBER                0x00b300ff

/*  PIR_CTL */
#define R0362_PIR_CTL                   0x00b4
#define F0362_FREEZE                    0x00b40001

/*  SNR_CARRIER1    */
#define R0362_SNR_CARRIER1              0x00b5
#define F0362_SNR_CARRIER_LO            0x00b500ff

/*  SNR_CARRIER2    */
#define R0362_SNR_CARRIER2              0x00b6
#define F0362_MEAN                      0x00b60080
#define F0362_SNR_CARRIER_HI            0x00b6001f

/*  PPM_CPAMP   */
#define R0362_PPM_CPAMP                 0x00b7
#define F0362_PPM_CPC                   0x00b700ff

/*  TSM_AP0 */
#define R0362_TSM_AP0                   0x00b8
#define F0362_ADDRESS_BYTE_0            0x00b800ff

/*  TSM_AP1 */
#define R0362_TSM_AP1                   0x00b9
#define F0362_ADDRESS_BYTE_1            0x00b900ff

/*  TSM_AP2 */
#define R0362_TSM_AP2                   0x00ba
#define F0362_DATA_BYTE_0               0x00ba00ff

/*  TSM_AP3 */
#define R0362_TSM_AP3                   0x00bb
#define F0362_DATA_BYTE_1               0x00bb00ff

/*  TSM_AP4 */
#define R0362_TSM_AP4                   0x00bc
#define F0362_DATA_BYTE_2               0x00bc00ff

/*  TSM_AP5 */
#define R0362_TSM_AP5                   0x00bd
#define F0362_DATA_BYTE_3               0x00bd00ff

/*  TSM_AP6 */
#define R0362_TSM_AP6                   0x00be
#define F0362_TSM_AP6                   0x00be00ff

/*  TSM_AP7 */
#define R0362_TSM_AP7                   0x00bf
#define F0362_MEM_SELECT_BYTE           0x00bf00ff

/*  TSTRES  */
#define R0362_TSTRES                    0x00c0
#define F0362_FRES_DISPLAY              0x00c00080
#define F0362_FRES_FIFO_AD              0x00c00020
#define F0362_FRESRS                    0x00c00010
#define F0362_FRESACS                   0x00c00008
#define F0362_FRESFEC                   0x00c00004
#define F0362_FRES_PRIF                 0x00c00002
#define F0362_FRESCORE                  0x00c00001

/*  ANACTRL */
#define R0362_ANACTRL                   0x00c1
#define F0362_BYPASS_XTAL               0x00c10040
#define F0362_BYPASS_PLLXN              0x00c1000c
#define F0362_DIS_PAD_OSC               0x00c10002
#define F0362_STDBY_PLLXN               0x00c10001

/*  TSTBUS  */
#define R0362_TSTBUS                    0x00c2
#define F0362_FORCERATE1                0x00c20080
#define F0362_TSTCKRS                   0x00c20040
#define F0362_TSTCKDIL                  0x00c20020
#define F0362_CFG_TST                   0x00c2000f

/*  TSTRATE */
#define R0362_TSTRATE                   0x00c6
#define F0362_FORCEPHA                  0x00c60080
#define F0362_FNEWPHA                   0x00c60010
#define F0362_FROT90                    0x00c60008
#define F0362_FR                        0x00c60007

/*  CONSTMODE   */
#define R0362_CONSTMODE                 0x00cb
#define F0362_TST_PRIF                  0x00cb00e0
#define F0362_CAR_TYPE                  0x00cb0018
#define F0362_CONST_MODE                0x00cb0003

/*  CONSTCARR1  */
#define R0362_CONSTCARR1                0x00cc
#define F0362_CONST_CARR_LO             0x00cc00ff

/*  CONSTCARR2  */
#define R0362_CONSTCARR2                0x00cd
#define F0362_CONST_CARR_HI             0x00cd001f

/*  ICONSTEL    */
#define R0362_ICONSTEL                  0x00ce
#define F0362_ICONSTEL                  0x00ce01ff

/*  QCONSTEL    */
#define R0362_QCONSTEL                  0x00cf
#define F0362_QCONSTEL                  0x00cf01ff

/*  TSTBISTRES0 */
#define R0362_TSTBISTRES0               0x00d0
#define F0362_BEND_BDI                  0x00d00080
#define F0362_BBAD_BDI                  0x00d00040
#define F0362_BEND_PPM                  0x00d00020
#define F0362_BBAD_PPM                  0x00d00010
#define F0362_BEND_SDI                  0x00d00008
#define F0362_BBAD_SDI                  0x00d00004
#define F0362_BEND_INS                  0x00d00002
#define F0362_BBAD_INS                  0x00d00001

/*  TSTBISTRES1 */
#define R0362_TSTBISTRES1               0x00d1
#define F0362_BEND_CHC2B                0x00d10080
#define F0362_BBAD_CHC2B                0x00d10040
#define F0362_BEND_CHC3                 0x00d10020
#define F0362_BBAD_CHC3                 0x00d10010
#define F0362_BEND_FFTI                 0x00d10008
#define F0362_BBAD_FFTI                 0x00d10004
#define F0362_BEND_FFTW                 0x00d10002
#define F0362_BBAD_FFTW                 0x00d10001

/*  TSTBISTRES2 */
#define R0362_TSTBISTRES2               0x00d2
#define F0362_BEND_RS                   0x00d20080
#define F0362_BBAD_RS                   0x00d20040
#define F0362_BEND_SYR                  0x00d20020
#define F0362_BBAD_SYR                  0x00d20010
#define F0362_BEND_CHC1                 0x00d20008
#define F0362_BBAD_CHC1                 0x00d20004
#define F0362_BEND_CHC2                 0x00d20002
#define F0362_BBAD_CHC2                 0x00d20001

/*  TSTBISTRES3 */
#define R0362_TSTBISTRES3               0x00d3
#define F0362_BEND_FIFO                 0x00d30080
#define F0362_BBAD_FIFO                 0x00d30040
#define F0362_BEND_VIT2                 0x00d30020
#define F0362_BBAD_VIT2                 0x00d30010
#define F0362_BEND_VIT1                 0x00d30008
#define F0362_BBAD_VIT1                 0x00d30004
#define F0362_BEND_DIL                  0x00d30002
#define F0362_BBAD_DIL                  0x00d30001

/*  RF_AGC1 */
#define R0362_RF_AGC1                   0x00d4
#define F0362_RF_AGC1_LEVEL_HI          0x00d400ff

/*  RF_AGC2 */
#define R0362_RF_AGC2                   0x00d5
#define F0362_REF_ADGP                  0x00d50080
#define F0362_STDBY_ADCGP               0x00d50020
#define F0362_CHANNEL_SEL               0x00d5001c
#define F0362_RF_AGC1_LEVEL_LO          0x00d50003

/*  ANADIGCTRL  */
#define R0362_ANADIGCTRL                0x00d7
#define F0362_SEL_CLKDEM                0x00d70020
#define F0362_ADC_RIS_EGDE              0x00d70004
#define F0362_SGN_ADC                   0x00d70002
#define F0362_SEL_AD12_SYNC             0x00d70001

/*  PLLMDIV */
#define R0362_PLLMDIV                   0x00d8
#define F0362_PLL_MDIV                  0x00d800ff

/*  PLLSETUP    */
#define R0362_PLLSETUP                  0x00da
#define F0362_PLL_PDIV                  0x00da0070

/*  DUAL_AD12   */
#define R0362_DUAL_AD12                 0x00db
#define F0362_FS20M                     0x00db0020
#define F0362_FS50M                     0x00db0010
#define F0362_INMODE                    0x00db0008
#define F0362_POFFQ                     0x00db0004
#define F0362_POFFI                     0x00db0002
#define F0362_POFFREF                   0x00db0001

/*  TSTBIST */
#define R0362_TSTBIST                   0x00dc
#define F0362_TST_GCLKENA               0x00dc0020
#define F0362_TST_MEMBIST               0x00dc001f

/*  PAD_COMP_CTRL   */
#define R0362_PAD_COMP_CTRL             0x00dd
#define F0362_COMPTQ                    0x00dd0010
#define F0362_COMPEN                    0x00dd0008
#define F0362_FREEZE2                   0x00dd0004
#define F0362_SLEEP_INHBT               0x00dd0002
#define F0362_CHIP_SLEEP                0x00dd0001

/*  PAD_COMP_WR */
#define R0362_PAD_COMP_WR               0x00de
#define F0362_WR_ASRC                   0x00de007f

/*  PAD_COMP_RD */
#define R0362_PAD_COMP_RD               0x00df
#define F0362_COMPOK                    0x00df0080
#define F0362_RD_ASRC                   0x00df007f

/*  GHOSTREG    */
#define R0362_GHOSTREG                  0x0000
#define F0362_GHOSTFIELD                0x000000ff


#define STV0362_NBREGS                  208
#define STV0362_NBFIELDS                478
//note: copied from ST code: src\drivers\ter\include\d0362_map.h --- [end]

//note: copied from ST code: src\drivers\ter\demod\stv0362\d0362.c --- [begin]
/* STV0362 register addresses, do not change its order. */
const UINT16  STV0362_Address[STV0362_NBREGS]=
{
    0x00d9,
    0x0000,
    0x0001,
    0x0002,
    0x0003,
    0x0004,
    0x0005,
    0x0006,
    0x0007,
    0x0008,
    0x0009,
    0x000a,
    0x000b,
    0x000c,
    0x000d,
    0x0010,
    0x0011,
    0x0012,
    0x0013,
    0x0014,
    0x0015,
    0x0016,
    0x0017,
    0x0018,
    0x0019,
    0x001a,
    0x001b,
    0x001c,
    0x001d,
    0x001e,
    0x001f,
    0x0020,
    0x0021,
    0x0022,
    0x0023,
    0x0024,
    0x0025,
    0x0026,
    0x0027,
    0x0028,
    0x0029,
    0x002a,
    0x002b,
    0x002c,
    0x002d,
    0x002e,
    0x002f,
    0x0030,
    0x0031,
    0x0032,
    0x0033,
    0x0034,
    0x0035,
    0x0036,
    0x0037,
    0x0038,
    0x0039,
    0x0040,
    0x0041,
    0x0042,
    0x0043,
    0x0044,
    0x0045,
    0x0046,
    0x0047,
    0x0049,
    0x004a,
    0x004b,
    0x004c,
    0x004d,
    0x004e,
    0x004f,
    0x0050,
    0x0051,
    0x0052,
    0x0053,
    0x0054,
    0x0055,
    0x0056,
    0x0057,
    0x0058,
    0x0059,
    0x005a,
    0x005b,
    0x005c,
    0x005d,
    0x005e,
    0x005f,
    0x0060,
    0x0061,
    0x0062,
    0x0063,
    0x0064,
    0x0065,
    0x0066,
    0x0067,
    0x0068,
    0x0069,
    0x006a,
    0x006b,
    0x006c,
    0x006d,
    0x006e,
    0x0070,
    0x0071,
    0x0072,
    0x0073,
    0x0074,
    0x0075,
    0x0076,
    0x0077,
    0x0078,
    0x0079,
    0x007a,
    0x007b,
    0x007c,
    0x007d,
    0x007e,
    0x007f,
    0x006f,
    0x0080,
    0x0081,
    0x0082,
    0x0083,
    0x0084,
    0x0085,
    0x0086,
    0x0087,
    0x0088,
    0x0089,
    0x008a,
    0x008b,
    0x008c,
    0x008d,
    0x008e,
    0x008f,
    0x0090,
    0x0091,
    0x0092,
    0x0093,
    0x0094,
    0x0095,
    0x0096,
    0x0097,
    0x0098,
    0x0099,
    0x009a,
    0x009b,
    0x009c,
    0x009d,
    0x009e,
    0x009f,
    0x00a0,
    0x00a1,
    0x00a2,
    0x00a3,
    0x00a4,
    0x00a5,
    0x00a6,
    0x00a7,
    0x00a8,
    0x00a9,
    0x00aa,
    0x00ab,
    0x00ac,
    0x00ad,
    0x00ae,
    0x00af,
    0x00b0,
    0x00b1,
    0x00b2,
    0x00b3,
    0x00b4,
    0x00b5,
    0x00b6,
    0x00b7,
    0x00b8,
    0x00b9,
    0x00ba,
    0x00bb,
    0x00bc,
    0x00bd,
    0x00be,
    0x00bf,
    0x00c0,
    0x00c1,
    0x00c2,
    0x00c6,
    0x00cb,
    0x00cc,
    0x00cd,
    0x00ce,
    0x00cf,
    0x00d0,
    0x00d1,
    0x00d2,
    0x00d3,
    0x00d4,
    0x00d5,
    0x00d7,
    0x00d8,
    0x00da,
    0x00db,
    0x00dc,
    0x00dd,
    0x00de,
    0x00df,
    0x0000
};

#define MAX_ADDRESS 0x00df

//note: default value of each STV0362 register
UINT8 STV0362_DefVal[STV0362_NBREGS]=
#if 1 //from ST STV0362 release code
{
    0x0f,/*  PLLNDIV                0x00d9*/
    0x40,/*  ID                     0x0000*/
    0x22,/*  I2CRPT                 0x0001*/
    /*0x01*/0x00,/*  TOPCTRL                0x0002*/ /* 2008.02.22 fixed */
    0x40,/*  IOCFG0                 0x0003*/
    0x00,/*  DAC0R                  0x0004*/
    0x00,/*  IOCFG1                 0x0005*/
    0x00,/*  DAC1R                  0x0006*/
    0x00,/*  IOCFG2                 0x0007*/
    0x00,/*  SDFR                   0x0008*/
    0xfa,/*  STATUS                 0x0009*/
    0x0b,/*  AUX_CLK                0x000a*/
    0x00,/*  FREESYS1               0x000b*/
    0x00,/*  FREESYS2               0x000c*/
    0x00,/*  FREESYS3               0x000d*/
    0xff,/*  AGC2MAX                0x0010*/
    0x00,/*  AGC2MIN                0x0011*/
    0xff,/*  AGC1MAX                0x0012*/
    0x00,/*  AGC1MIN                0x0013*/
    0xbc,/*  AGCR                   0x0014*/
    0x00,/*  AGC2TH                 0x0015*/
    0x40,/*  AGC12C                 0x0016*/
    0x85,/*  AGCCTRL1               0x0017*/
    0x18,/*  AGCCTRL2               0x0018*/
    0xff,/*  AGC1VAL1               0x0019*/
    0x0f,/*  AGC1VAL2               0x001a*/
    0x8d,/*  AGC2VAL1               0x001b*/
    0x02,/*  AGC2VAL2               0x001c*/
    0x00,/*  AGC2PGA                0x001d*/
    0x00,/*  OVF_RATE1              0x001e*/
    0x00,/*  OVF_RATE2              0x001f*/
    0xfe,/*  GAIN_SRC1              0x0020*/
    0xd8,/*  GAIN_SRC2              0x0021*/
    0x55,/*  INC_DEROT1             0x0022*/
    /*0x4c*/0x58,/*  INC_DEROT2             0x0023*/ /* 2008.02.22 fixed */
    0x2c,/*  PPM_CPAMP_DIR          0x0024*/
    0x00,/*  PPM_CPAMP_INV          0x0025*/
    /*0x00*/0x03,/*  FREESTFE_1             0x0026*/ /* 2008.02.22 fixed */
    0x1c,/*  FREESTFE_2             0x0027*/
    0x00,/*  DCOFFSET               0x0028*/
    0xb1,/*  EN_PROCESS             0x0029*/
    0xff,/*  SDI_SMOOTHER           0x002a*/
    0x00,/*  FE_LOOP_OPEN           0x002b*/
    0x00,/*  FREQOFF1               0x002c*/
    0x00,/*  FREQOFF2               0x002d*/
    0x00,/*  FREQOFF3               0x002e*/
    0x00,/*  TIMOFF1                0x002f*/
    0x00,/*  TIMOFF2                0x0030*/
    0x01,/*  EPQ                    0x0031*/
    0x01,/*  EPQAUTO                0x0032*/
    0x01,/*  CHP_TAPS               0x0033*/
    0x02,/*  CHP_DYN_COEFF          0x0034*/
    0x23,/*  PPM_STATE_MAC          0x0035*/
    0xff,/*  INR_THRESHOLD          0x0036*/
    0x89,/*  EPQ_TPS_ID_CELL        0x0037*/
    0x00,/*  EPQ_CFG                0x0038*/
    0xfe,/*  EPQ_STATUS             0x0039*/
    0x00,/*  FECM                   0x0040*/
    0x1e,/*  VTH0                   0x0041*/
    0x1e,/*  VTH1                   0x0042*/
    0x0f,/*  VTH2                   0x0043*/
    0x09,/*  VTH3                   0x0044*/
    0x00,/*  VTH4                   0x0045*/
    0x05,/*  VTH5                   0x0046*/
    0x00,/*  FREEVIT                0x0047*/
    0x92,/*  VITPROG                0x0049*/
    /*0x04*/0xAF,/*  PR                     0x004a*/
    /*0xb0*/0x30,/*  VSEARCH                0x004b*/
    0xbc,/*  RS                     0x004c*/
    0x05,/*  RSOUT                  0x004d*/
    0x12,/*  ERRCTRL1               0x004e*/
    0x00,/*  ERRCNTM1               0x004f*/
    0x00,/*  ERRCNTL1               0x0050*/
    0xb3,/*  ERRCTRL2               0x0051*/
    0x00,/*  ERRCNTM2               0x0052*/
    0x00,/*  ERRCNTL2               0x0053*/
    0x00,/*  FREEDRS                0x0054*/
    0x00,/*  VERROR                 0x0055*/
    0x12,/*  ERRCTRL3               0x0056*/
    0x00,/*  ERRCNTM3               0x0057*/
    0x00,/*  ERRCNTL3               0x0058*/
    0x00,/*  DILSTK1                0x0059*/
    0x03,/*  DILSTK0                0x005a*/
    0x00,/*  DILBWSTK1              0x005b*/
    0x03,/*  DILBWST0               0x005c*/
    0x80,/*  LNBRX                  0x005d*/
    0xb0,/*  RSTC                   0x005e*/
    0x07,/*  VIT_BIST               0x005f*/
    0x8d,/*  IIR_CELL_NB            0x0060*/
    0x00,/*  IIR_CX_COEFF1_MSB      0x0061*/
    0x00,/*  IIR_CX_COEFF1_LSB      0x0062*/
    0x0e,/*  IIR_CX_COEFF2_MSB      0x0063*/
    0xcc,/*  IIR_CX_COEFF2_LSB      0x0064*/
    0x0e,/*  IIR_CX_COEFF3_MSB      0x0065*/
    0xcc,/*  IIR_CX_COEFF3_LSB      0x0066*/
    0x00,/*  IIR_CX_COEFF4_MSB      0x0067*/
    0x00,/*  IIR_CX_COEFF4_LSB      0x0068*/
    0x36,/*  IIR_CX_COEFF5_MSB      0x0069*/
    0x47,/*  IIR_CX_COEFF5_LSB      0x006a*/
    0x00,/*  FEPATH_CFG             0x006b*/
    0x25,/*  PMC1_FUNC              0x006c*/
    0x00,/*  PMC1_FORCE             0x006d*/
    0x00,/*  PMC2_FUNC              0x006e*/
    0xf8,/*  DIG_AGC_R              0x0070*/
    0x0d,/*  COMAGC_TARMSB          0x0071*/
    0xc1,/*  COM_AGC_TAR_ENMODE     0x0072*/
    0x3b,/*  COM_AGC_CFG            0x0073*/
    0x39,/*  COM_AGC_GAIN1          0x0074*/
    0x10,/*  AUT_AGC_TARGET_MSB     0x0075*/
    0x01,/*  LOCK_DETECT_MSB        0x0076*/
    0x00,/*  AGCTAR_LOCK_LSBS       0x0077*/
    0xf4,/*  AUT_GAIN_EN            0x0078*/
    0xf0,/*  AUT_CFG                0x0079*/
    0x23,/*  LOCKN                  0x007a*/
    0x00,/*  INT_X_3                0x007b*/
    0x05,/*  INT_X_2                0x007c*/
    0x60,/*  INT_X_1                0x007d*/
    0xc0,/*  INT_X_0                0x007e*/
    0x01,/*  MIN_ERR_X_MSB          0x007f*/
    0xec,/*  STATUS_ERR_DA          0x006f*/
    0x20,/*  COR_CTL                0x0080*/
    0xf6,/*  COR_STAT               0x0081*/
    0x00,/*  COR_INTEN              0x0082*/
    0x39,/*  COR_INTSTAT            0x0083*/
    0x0a,/*  COR_MODEGUARD          0x0084*/
    0x18,/*  AGC_CTL                0x0085*/
    0x00,/*  AGC_MANUAL1            0x0086*/
    0x00,/*  AGC_MANUAL2            0x0087*/
    0x1e,/*  AGC_TARGET             0x0088*/
    0x97,/*  AGC_GAIN1              0x0089*/
    0x1a,/*  AGC_GAIN2              0x008a*/
    0x00,/*  RESERVED_1             0x008b*/
    0x00,/*  RESERVED_2             0x008c*/
    0x00,/*  RESERVED_3             0x008d*/
    0x40,/*  CAS_CTL                0x008e*/
    0xb3,/*  CAS_FREQ               0x008f*/
    0x0f,/*  CAS_DAGCGAIN           0x0090*/
    0x04,/*  SYR_CTL                0x0091*/
    0x13,/*  SYR_STAT               0x0092*/
    0x00,/*  SYR_NCO1               0x0093*/
    0x00,/*  SYR_NCO2               0x0094*/
    0x00,/*  SYR_OFFSET1            0x0095*/
    0x00,/*  SYR_OFFSET2            0x0096*/
    0x08, /*  FFT_CTL                0x0097*/  /* 2008.03.18: Improve 6MHz sensitivity. */
    0x00,/*  SCR_CTL                0x0098*/
    0x38,/*  PPM_CTL1               0x0099*/
    0x14,/*  TRL_CTL                0x009a*/
    0xac,/*  TRL_NOMRATE1           0x009b*/
    0x56,/*  TRL_NOMRATE2           0x009c*/
    0x87,/*  TRL_TIME1              0x009d*/
    0xfd,/*  TRL_TIME2              0x009e*/
    0x4f,/*  CRL_CTL                0x009f*/
    0xa0,/*  CRL_FREQ1              0x00a0*/
    0xb6,/*  CRL_FREQ2              0x00a1*/
    0xff,/*  CRL_FREQ3              0x00a2*/
    0x11,/*  CHC_CTL1               0x00a3*/
    0xee,/*  CHC_SNR                0x00a4*/
    0x00,/*  BDI_CTL                0x00a5*/
    0x00,/*  DMP_CTL                0x00a6*/
    0x33,/*  TPS_RCVD1              0x00a7*/
    0x02,/*  TPS_RCVD2              0x00a8*/
    0x02,/*  TPS_RCVD3              0x00a9*/
    0x30,/*  TPS_RCVD4              0x00aa*/
    0x00,/*  TPS_ID_CELL1           0x00ab*/
    0x00,/*  TPS_ID_CELL2           0x00ac*/
    0x00,/*  TPS_RCVD5_SET1         0x00ad*/
    0x02,/*  TPS_SET2               0x00ae*/
    0x02,/*  TPS_SET3               0x00af*/
    0x00,/*  TPS_CTL                0x00b0*/
    0x2b,/*  CTL_FFTOSNUM           0x00b1*/
    0x09,/*  TESTSELECT             0x00b2*/
    0x0a,/*  MSC_REV                0x00b3*/
    0x00,/*  PIR_CTL                0x00b4*/
    0xa9,/*  SNR_CARRIER1           0x00b5*/
    0x86,/*  SNR_CARRIER2           0x00b6*/
    0x31,/*  PPM_CPAMP              0x00b7*/
    0x00,/*  TSM_AP0                0x00b8*/
    0x00,/*  TSM_AP1                0x00b9*/
    0x00,/*  TSM_AP2                0x00ba*/
    0x00,/*  TSM_AP3                0x00bb*/
    0x00,/*  TSM_AP4                0x00bc*/
    0x00,/*  TSM_AP5                0x00bd*/
    0x00,/*  TSM_AP6                0x00be*/
    0x00,/*  TSM_AP7                0x00bf*/
    0x00,/*  TSTRES                 0x00c0*/
    0x00,/*  ANACTRL                0x00c1*/
    0x00,/*  TSTBUS                 0x00c2*/
    0x00,/*  TSTRATE                0x00c6*/
    0x00,/*  CONSTMODE              0x00cb*/
    0x00,/*  CONSTCARR1             0x00cc*/
    0x00,/*  CONSTCARR2             0x00cd*/
    0x00,/*  ICONSTEL               0x00ce*/
    0x00,/*  QCONSTEL               0x00cf*/
    0x02,/*  TSTBISTRES0            0x00d0*/
    0x00,/*  TSTBISTRES1            0x00d1*/
    0x00,/*  TSTBISTRES2            0x00d2*/
    0x00,/*  TSTBISTRES3            0x00d3*/
    0xe6,/*  RF_AGC1                0x00d4*/
    0x81,/*  RF_AGC2                0x00d5*/
    0x01,/*  ANADIGCTRL             0x00d7*/
    0x00,/*  PLLMDIV                0x00d8*/
    0x10,/*  PLLSETUP               0x00da*/
    0x08,/*  DUAL_AD12              0x00db*/
    0x00,/*  TSTBIST                0x00dc*/
    0x00,/*  PAD_COMP_CTRL          0x00dd*/
    0x00,/*  PAD_COMP_WR            0x00de*/
    0xe0,/*  PAD_COMP_RD            0x00df*/
    0x40 /*  GHOSTREG               0x0000*/
};

#else //copy from ST GUI tool CD, LLA v1.8.5\362_test.c for STV0362 Chip ID 0x41
{
    0x0f,0x41,0x22,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0xf8,0x0a,0x00,
    0x00,0x00,0xff,0x00,0xff,0x00,0xbc,0x00,0x40,0x85,0x18,0xff,0x0f,
    0x6e,0x02,0x00,0x00,0x00,0xdc,0x80,0x55,0x5d,0x62,0x00,0x03,0x1c,
    0x7e,0xb1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x06,0x03,0x13,
    0x23,0xc0,0x89,0x00,0x0e,0x00,0x1e,0x14,0x0f,0x09,0x00,0x05,0x00,
    0x92,0xaf,0x30,0xfc,0x05,0x12,0x00,0x00,0xb3,0x00,0x00,0x00,0x00,
    0x12,0x00,0x00,0x00,0x03,0x00,0x03,0x80,0xb0,0x07,0x8d,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x00,0x00,0xe0,
    0x0b,0x41,0x3a,0x39,0x00,0x06,0x00,0xf4,0xf0,0x20,0x7f,0xff,0xff,
    0xff,0x0f,0x0f,0x20,0xf6,0x00,0x3f,0x03,0x18,0x00,0x00,0x20,0x6d,
    0x1a,0x00,0x00,0x00,0x44,0xb3,0x11,0x00,0x17,0x00,0x00,0x00,0x00,
    0x00,0x50,0x38,0x14,0xad,0x56,0xda,0xf3,0x4f,0xaf,0x57,0x00,0x11,
    0xea,0x00,0x00,0x30,0x00,0x00,0x31,0x00,0x00,0x01,0x00,0x00,0x00,
    0x26,0x09,0x0a,0x00,0xa1,0x9a,0xc3,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
    0x00,0x00,0xff,0x83,0x01,0x00,0x10,0x08,0x00,0x00,0x00,0xe0,0x41
};
#endif
//note: copied from ST code: src\drivers\ter\demod\stv0362\d0362.c --- [end]


//note: copied from ST code: src\drivers\include\ioreg.h [begin]
#define LSB(X) ((X & 0xFF))
#define MSB(Y) ((Y >> 8) & 0xFF)
//note: copied from ST code: src\drivers\include\ioreg.h [end]


#if 1 //used types/constants in ST 0362 driver code !!
//note: copied from ST code: sttuner\sttuner.h --- [begin]
    typedef enum
    {
        STTUNER_NORMAL_IF_TUNER,
        STTUNER_NORMAL_LONGPATH_IF_TUNER,
        STTUNER_LONGPATH_IQ_TUNER
    } STTUNER_IF_IQ_Mode;

    /* Alpha value  during Hierarchical Modulation */
    typedef enum STTUNER_Hierarchy_Alpha_e
    {
        STTUNER_HIER_ALPHA_NONE,
        STTUNER_HIER_ALPHA_1,
        STTUNER_HIER_ALPHA_2,
        STTUNER_HIER_ALPHA_4
    } STTUNER_Hierarchy_Alpha_t;

    /* hierarchy */
    typedef enum STTUNER_Hierarchy_e
    {
        STTUNER_HIER_NONE,
        STTUNER_HIER_LOW_PRIO,
        STTUNER_HIER_HIGH_PRIO,
        STTUNER_HIER_PRIO_ANY
    } STTUNER_Hierarchy_t;

    typedef enum STTUNER_FreqOff_e
    {
        STTUNER_OFFSET_NONE = 0,
        STTUNER_OFFSET = 1,
        STTUNER_OFFSET_POSITIVE = 2,
        STTUNER_OFFSET_NEGATIVE = 3
    } STTUNER_FreqOff_t;

    /* StandBy Mode */
    typedef enum STTUNER_StandByMode_e
    {
        STTUNER_NORMAL_POWER_MODE = 0,
        STTUNER_STANDBY_POWER_MODE = 1
    } STTUNER_StandByMode_t;

    /* High/Low RF value */
    #define STTUNER_HIGH_RF -15
    #define STTUNER_LOW_RF  -65
//note: copied from ST code: sttuner\sttuner.h --- [end]


//note: copied and tuned from ST code: src\drivers\ter\include\d0362_drv.h --- [begin]
    #define NINV 0
    #define INV  1

    typedef enum
    {
        NOAGC_362=0,
        AGCOK_362=5,
        NOTPS_362=6,
        TPSOK_362=7,
        NOSYMBOL_362=8,
        BAD_CPQ_362=9,
        PRFOUNDOK_362=10,
        NOPRFOUND_362=11,
        LOCKOK_362=12,
        NOLOCK_362=13,
        SYMBOLOK_362=15,
        CPAMPOK_362=16,
        NOCPAMP_362=17,
        SWNOK_362=18
    } FE_362_SignalStatus_t;

    typedef enum
    {
        FE_362_NO_ERROR,
        FE_362_INVALID_HANDLE,
        FE_362_BAD_PARAMETER,
        FE_362_MISSING_PARAMETER,
        FE_362_ALREADY_INITIALIZED,
        FE_362_I2C_ERROR,
        FE_362_SEARCH_FAILED,
        FE_362_TRACKING_FAILED,
        FE_362_TERM_FAILED
    } FE_362_Error_t;

    /* type of modulation (common) */
    typedef enum STTUNER_Modulation_e
    {
        STTUNER_MOD_NONE   = 0x00,  /* Modulation unknown */
        STTUNER_MOD_ALL    = 0x1FF, /* Logical OR of all MODs */
        STTUNER_MOD_QPSK   = 1,
        STTUNER_MOD_8PSK   = (1 << 1),
        STTUNER_MOD_QAM    = (1 << 2),
        STTUNER_MOD_16QAM  = (1 << 3),
        STTUNER_MOD_32QAM  = (1 << 4),
        STTUNER_MOD_64QAM  = (1 << 5),
        STTUNER_MOD_128QAM = (1 << 6),
        STTUNER_MOD_256QAM = (1 << 7),
        STTUNER_MOD_BPSK   = (1 << 8)
    } STTUNER_Modulation_t;

    /* mode of OFDM signal */
    typedef enum STTUNER_Mode_e
    {
        STTUNER_MODE_2K,
        STTUNER_MODE_8K,
        STTUNER_MODE_4K
    } STTUNER_Mode_t;

    /* guard of OFDM signal */
    typedef enum STTUNER_Guard_e
    {
        STTUNER_GUARD_1_32,               /* Guard interval = 1/32 */
        STTUNER_GUARD_1_16,               /* Guard interval = 1/16 */
        STTUNER_GUARD_1_8,                /* Guard interval = 1/8  */
        STTUNER_GUARD_1_4                 /* Guard interval = 1/4  */
    } STTUNER_Guard_t;

  #if 0 //與 ST sttuner\sttuner.h 裡的重複且定義不同!!
    /* hierarchy */
    typedef enum STTUNER_Hierarchy_e
    {
        STTUNER_HIER_NONE,              /* Regular modulation */
        STTUNER_HIER_1,                 /* Hierarchical modulation a = 1*/
        STTUNER_HIER_2,                 /* Hierarchical modulation a = 2*/
        STTUNER_HIER_4                  /* Hierarchical modulation a = 4*/
    } STTUNER_Hierarchy_t;

    typedef enum STTUNER_FreqOff_e
    {
        STTUNER_OFFSET_NONE = 0,
        STTUNER_OFFSET      = 1
    } STTUNER_FreqOff_t;
  #endif //與 ST sttuner\sttuner.h 裡的重複且定義不同!!

    typedef enum STTUNER_Spectrum_e
    {
        STTUNER_INVERSION_NONE = 0,
        STTUNER_INVERSION      = 1,
        STTUNER_INVERSION_AUTO = 2,
        STTUNER_INVERSION_UNK  = 4
    } STTUNER_Spectrum_t;

    typedef enum STTUNER_Force_e
    {
        STTUNER_FORCENONE  = 0,
        STTUNER_FORCE_M_G  = 1
    } STTUNER_Force_t;

    typedef enum STTUNER_ChannelBW_e
    {
        STTUNER_CHAN_BW_6M  = 6,
        STTUNER_CHAN_BW_7M  = 7,
        STTUNER_CHAN_BW_8M  = 8
    } STTUNER_ChannelBW_t;

    typedef enum STTUNER_FECRate_e
    {
        STTUNER_FEC_NONE = 0x00,    /* no FEC rate specified */
        STTUNER_FEC_ALL = 0xFF,     /* Logical OR of all FECs */
        STTUNER_FEC_1_2 = 1,
        STTUNER_FEC_2_3 = (1 << 1),
        STTUNER_FEC_3_4 = (1 << 2),
        STTUNER_FEC_4_5 = (1 << 3),
        STTUNER_FEC_5_6 = (1 << 4),
        STTUNER_FEC_6_7 = (1 << 5),
        STTUNER_FEC_7_8 = (1 << 6),
        STTUNER_FEC_8_9 = (1 << 7)
    } STTUNER_FECRate_t;

    typedef enum
    {
        FE_362_TPS_1_2  =   0,
        FE_362_TPS_2_3  =   1,
        FE_362_TPS_3_4  =   2,
        FE_362_TPS_5_6  =   3,
        FE_362_TPS_7_8  =   4
    } FE_362_Rate_TPS_t;

    typedef enum
    {
        FE_362_1_2  =   0,
        FE_362_2_3  =   1,
        FE_362_3_4  =   2,
        FE_362_5_6  =   3,
        FE_362_6_7  =   4,
        FE_362_7_8  =   5
    } FE_362_Rate_t;

    typedef enum
    {
        FE_362_NO_FORCE =0,
        FE_362_FORCE_PR_1_2 = 1 ,
        FE_362_FORCE_PR_2_3 = 1<<1,
        FE_362_FORCE_PR_3_4 = 1<<2,
        FE_362_FORCE_PR_5_6 = 1<<3 ,
        FE_362_FORCE_PR_7_8 = 1<<4
    } FE_362_Force_PR_t;

    typedef enum
    {
        NOT_FORCED_362  = 0,
        WAIT_TRL_362    = 1,
        WAIT_AGC_362    = 2,
        WAIT_SYR_362    = 3,
        WAIT_PPM_362    = 4,
        WAIT_TPS_362    = 5,
        MONITOR_TPS_362 = 6,
        RESERVED_362    = 7
    } FE_362_State_Machine_t ;

    /****************************************************************
                        INIT STRUCTURES
     ****************************************************************/

    typedef enum
    {
        FE_362_PARALLEL_CLOCK,
        FE_362_SERIAL_MASTER_CLOCK,
        FE_362_SERIAL_VCODIV6_CLOCK
    } FE_362_Clock_t;

    typedef enum
    {
        FE_362_PARITY_ON,
        FE_362_PARITY_OFF
    } FE_362_DataParity_t;

    typedef enum
    {
        FE_362_DVB_STANDARD,                /* The FEC mode corresponds to the DVB standard */
        FE_362_DSS_STANDARD                 /* The FEC mode corresponds to the DSS standard */
    } FE_362_CodingStandard_t;

    /*
        structure passed to the FE_362_Init() function
    */
    typedef struct
    {
        FE_362_SignalStatus_t   SignalType;     /* Type of founded signal */
        STTUNER_FECRate_t       PunctureRate;   /* Puncture rate found */
        double Frequency;                       /* Why double? Transponder frequency (KHz) */
        STTUNER_Mode_t          Mode;           /* Mode 2K or 8K */
        STTUNER_Guard_t         Guard;          /* Guard interval */
        STTUNER_Modulation_t    Modulation;     /* modulation */
        STTUNER_Hierarchy_t     hier;
        FE_362_Rate_TPS_t       Hprate;
        FE_362_Rate_TPS_t       Lprate;
        FE_362_Rate_TPS_t       pr;
        UINT32 SymbolRate;                      /* Symbol rate (Bds) */
    } FE_362_InternalResults_t;

    typedef struct
    {
        FE_362_SignalStatus_t       State;              /*  Current state of the search algorithm */
        STTUNER_IF_IQ_Mode          IF_IQ_Mode;
        STTUNER_Mode_t              Mode;               /*  Mode 2K or 8K   */
        STTUNER_Guard_t             Guard;              /*  Guard interval  */
        STTUNER_Hierarchy_t         Hierarchy;          /** Hierarchical Mode***/
        UINT32                      Frequency;          /*  Current tuner frequency (KHz) */
        UINT8                       I2Cspeed;
    #if 0 //not used in ST 0362 driver code at all !!
        FE_362_OFDMEchoParams_t     Echo;
    #endif
        STTUNER_Spectrum_t          Inv;                /* 0 no spectrum inverted search to be perfomed */
        STTUNER_FreqOff_t           Offset;             /* 0 no freq offset channel search to be perfomed */
        UINT8                       Delta;              /* offset of frequency */
        UINT8                       Sense;              /* current search,spectrum not inveerted */
        UINT8                       Force;              /* force mode/guard */
        UINT8                       ChannelBW;          /* channel width */
        INT8                        EchoPos;            /* echo position */
        UINT8                       first_lock;
        UINT8                       prev_lock_status;   /* verbose status of the previous lock (for scan) */
        FE_362_InternalResults_t    Results;            /* Results of the search */
    } FE_362_InternalParams_t;


    /****************************************************************
                                SEARCH STRUCTURES
     ****************************************************************/
    typedef struct
    {
        UINT32              Frequency;
        STTUNER_IF_IQ_Mode  IF_IQ_Mode;
        STTUNER_Mode_t      Mode;
        STTUNER_Guard_t     Guard;
        STTUNER_FreqOff_t   Offset;
        STTUNER_Force_t     Force;
        STTUNER_Spectrum_t  Inv;
        STTUNER_ChannelBW_t ChannelBW;
        INT8                EchoPos;
        STTUNER_Hierarchy_t Hierarchy;
        UINT8               first_spectrum_locked;
    } FE_362_SearchParams_t;

    /************************
        INFO STRUCTURE
    ************************/
    typedef struct
    {
        UINT32                    Frequency;
        UINT32                    Agc_val; /* Agc1 on MSB */
        INT32                     offset;
        INT32                     offset_type;

        STTUNER_Mode_t            Mode;
        STTUNER_Guard_t           Guard;
        STTUNER_Modulation_t      Modulation;
        STTUNER_Hierarchy_t       hier;
        STTUNER_Spectrum_t        Sense; /* 0 spectrum not inverted */
        FE_362_Rate_TPS_t         Hprate;
        FE_362_Rate_TPS_t         Lprate;
        FE_362_Rate_TPS_t         pr;

        FE_362_State_Machine_t    State;
        INT8                      Echo_pos;
        FE_362_SignalStatus_t     SignalStatus;
        STTUNER_Hierarchy_Alpha_t Hierarchy_Alpha;
        BOOL                      Locked;
        STTUNER_Spectrum_t        spectrum_direction;
    } FE_362_SearchResult_t;

    typedef struct
    {
        UINT32 Frequency;
        FE_362_SearchResult_t Result;
    } FE_362_Scan_Result_t;
//note: copied and tuned from ST code: src\drivers\ter\include\d0362_drv.h --- [end]

//note: copied and tuned from ST code: sttuner\sttuner.h --- [begin]
    /* transport stream output mode */
    typedef enum STTUNER_TSOutputMode_e
    {
        STTUNER_TS_MODE_DEFAULT,    /* TS output not changeable */
        STTUNER_TS_MODE_SERIAL,     /* TS output serial */
        STTUNER_TS_MODE_PARALLEL,   /* TS output parallel */
        STTUNER_TS_MODE_DVBCI       /* TS output DVB-CI */
    } STTUNER_TSOutputMode_t;

    /* serial clock source */
    typedef enum STTUNER_SerialClockSource_e
    {
        STTUNER_SCLK_DEFAULT,   /* Use default SCLK */
        STTUNER_SCLK_MASTER,    /* Derived from MCLK */
        STTUNER_SCLK_VCODIV6    /* SCLK = FVCO/6 */
    } STTUNER_SerialClockSource_t;

    /* serial clock modes */
    typedef enum STTUNER_SerialDataMode_e
    {
        STTUNER_SDAT_DEFAULT       = -1,  /* Use default SDAT mode */
        STTUNER_SDAT_VALID_RISING  = 1,             /* Data valid on clock rising edge */
        STTUNER_SDAT_PARITY_ENABLE = (1 << 1)       /* Data includes parity */
    } STTUNER_SerialDataMode_t;

    /* FEC Modes */
    typedef enum STTUNER_FECMode_e
    {
        STTUNER_FEC_MODE_DEFAULT,    /* Use default FEC mode */
        STTUNER_FEC_MODE_DIRECTV,    /* DirecTV Legacy mode */
        STTUNER_FEC_MODE_ATSC,       /* For ATSC VSB transmission */
        STTUNER_FEC_MODE_DVB         /* DVB mode */
    } STTUNER_FECMode_t;

    /* Configuration of clock polarity in case of terrestrial */
    typedef enum STTUNER_DataClockPolarity_e
    {
        STTUNER_DATA_CLOCK_POLARITY_DEFAULT = 0x00,  /* Clock polarity default */
        STTUNER_DATA_CLOCK_POLARITY_FALLING,         /* Clock polarity in rising edge */
        STTUNER_DATA_CLOCK_POLARITY_RISING,          /* Clock polarity in falling edge */
        STTUNER_DATA_CLOCK_NONINVERTED,              /* Non inverted */
        STTUNER_DATA_CLOCK_INVERTED                  /* inverted */
    } STTUNER_DataClockPolarity_t;

    /* initialization parameters, terrestrial-specific parameters */
    typedef struct STTUNER_InitParams_s
    {
        /* configure demod */
        UINT32                        ExternalClock;
        STTUNER_TSOutputMode_t        TSOutputMode;
        STTUNER_SerialClockSource_t   SerialClockSource;
        STTUNER_SerialDataMode_t      SerialDataMode;
        STTUNER_FECMode_t             FECMode;
        STTUNER_DataClockPolarity_t   ClockPolarity; /* Configuration of clock rising/falling edge */
        STTUNER_StandByMode_t         StandBy_Flag;
      #if 1
        BOOL                          IOCTL_Set_30MZ_REG_Flag; /* for setting some STV0362 registers for 30 MHZ crystal */
      #endif
    } STTUNER_InitParams_t;

    //weiching note: I removed some ST fileds that not referenced in ST 0362 code
    /* ST scan parameters */
    typedef struct STTUNER_Scan_s
    {
        STTUNER_Modulation_t   Modulation; /* configurable parameter for modulation scheme */
        UINT32                 Band; /* define the which band to be searched */
        INT16                  AGC;  /* store AGC value returned from low level driver */
        INT32                  ResidualOffset; /* store delta frequency value for fine tunning */

        STTUNER_Mode_t         Mode; /* configurable parameter for TER mode */
        STTUNER_Guard_t        Guard; /* configurable parameter for TER guard */
        STTUNER_Force_t        Force; /* configurable parameter to understand whether the demod will
                                         only work with the search parameter given by user or
                                         it will do automatic search with all possible parameter */
        STTUNER_Hierarchy_t    Hierarchy; /* configurable parameter for hierarchical transmission */

        INT32                  EchoPos; /* Gives back the echo position value */

        STTUNER_FreqOff_t      FreqOff;
        STTUNER_ChannelBW_t    ChannelBW;

        STTUNER_Spectrum_t     Spectrum;
        STTUNER_FECRate_t      FECRates; /* Configurable parameter for different code rate */
    } STTUNER_Scan_t;

  #if 1 //useful in ALi code??
    /* ST tuner instance information */
    typedef struct STTUNER_TunerInfo_s
    {
        STTUNER_Scan_t             ScanInfo;
        UINT32                     Frequency;
        UINT32                     SignalQuality;
        UINT32                     BitErrorRate;
        UINT32                     FrequencyFound;
        STTUNER_Hierarchy_Alpha_t  Hierarchy_Alpha; /* Hierarchy Alpha level used for reporting */
    } STTUNER_TunerInfo_t;
  #endif //useful in ALi code??

//note: copied and tuned from ST code: sttuner\sttuner.h --- [end]


//note: copied and tuned from ST code: src\include\mdemod.h --- [begin]
    typedef struct
    {
        UINT8  RPLLDIV; /* PLLNDIV register value */
        UINT8  TRLNORMRATELSB; /* TRL Normrate registers value */
        UINT8  TRLNORMRATELO;
        UINT8  TRLNORMRATEHI;
        UINT8  INCDEROT1; /* INC derotator register value */
        UINT8  INCDEROT2;
    } STTUNER_demod_IOCTL_30MHZ_REG_t;
//note: copied and tuned from ST code: src\include\mdemod.h --- [end]

#endif


//ALi-defined values ==> 需改用上面的ST-defined enum values, 並改寫相關API parameter and return value!! --- [begin]
#define FEC_1_2   0
#define FEC_2_3   1
#define FEC_3_4   2
#define FEC_5_6   3
#define FEC_7_8   4

#define guard_1_32  0x20
#define guard_1_16  0x10
#define guard_1_8   0x08
#define guard_1_4   0x04

#define MODE_2K     0x02
#define MODE_8K     0x08

#define FFT_MODE_2K 0x0
#define FFT_MODE_8K 0x1

#define TPS_CONST_QPSK  0x04
#define TPS_CONST_16QAM 0x10
#define TPS_CONST_64QAM 0x40

#define HIER_NONE  1
#define HIER_1     1
#define HIER_2     2
#define HIER_4     4
//ALi-defined values ==> 需改用上面的ST-defined enum values, 並改寫相關API parameter and return value!! --- [begin]

//ALi-defined front-end channel scan status?? -- [begin]
#define Mode_AUTOSCAN    0x00
#define Mode_CHANSCAN    0x01
#define Mode_CHANCHG     0x02
#define Mode_AERIALTUNE  0x03

#define RST_TUNNING   10
#define LOCK_OK       11
#define BAD_CPQ       12
#define AGC_UNLOCK    13
#define SYR_UNLOCK    14
//#define ERR_FAILURE   16
#define LK_UNLOCK     17
#define TPS_UNLOCK    19
#define PRF_UNLOCK    20
//ALi-defined front-end channel scan status?? -- [end]

#define FALSE_SNR_VALUE  0
#define DELTA_TIME       2


struct nim_stv0362_private
{
    struct COFDM_TUNER_CONFIG_API Tuner_Control;
};

static INT32 f_stv0362_read(UINT8 dev_addr, UINT8 reg_addr, UINT8 *data, UINT8 len);
static INT32 f_stv0362_write(UINT8 dev_addr, UINT8 reg_addr, UINT8 *data, UINT8 len);
INT32 f_stv0362_attach();
void f_stv0362_hw_init();
static INT32 f_stv0362_open(struct nim_device *dev);
static INT32 f_stv0362_close(struct nim_device *dev);
static INT32 f_stv0362_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
static INT32 f_stv0362_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list);
static INT32 f_stv0362_disable(struct nim_device *dev);
  static INT32 f_stv0362_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);
  static INT32 f_stv0362_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT16 freq_offset, UINT8 priority);
/*@don't need to create task for tracking lock status*/
static INT32 f_stv0362_get_lock_status(struct nim_device *dev, UINT8 *lock);
static INT32 f_stv0362_get_lock(struct nim_device *dev, UINT8 *lock);
static INT32 f_stv0362_get_freq(struct nim_device *dev, UINT32 *freq);
static INT32 f_stv0362_get_code_rate(struct nim_device *dev, UINT8* code_rate);
static INT32 f_stv0362_get_AGC(struct nim_device *dev, UINT16 *agc);
static INT32 f_stv0362_get_SNR(struct nim_device *dev, UINT8 *snr);
static INT32 f_stv0362_get_BER(struct nim_device *dev, UINT32 *RsUbc);
static INT32 f_stv0362_get_GI(struct nim_device *dev, UINT8 *guard_interval);
static INT32 f_stv0362_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
static INT32 f_stv0362_get_modulation(struct nim_device *dev, UINT8 *modulation);
//static INT32 f_stv0362_get_bandwidth(struct nim_device *dev, UINT8 *bandwidth);
static INT32 f_stv0362_get_RFLevel(struct nim_device *dev, INT32 *RFlevel);
static INT32 f_stv0362_CC_Tracking(struct nim_device *dev);
static INT32 f_stv0362_freq_offset(struct nim_device *dev, INT32 *freq_offset);
static INT32 f_stv0362_get_freq_offset(struct nim_device *dev, INT32 *freq_offset);
static INT32 f_stv0362_hier_mode(struct nim_device *dev);
static INT32 f_stv0362_get_hier_mode(struct nim_device *dev, UINT8*hier);
static INT8 f_stv0362_priority(struct nim_device *dev, UINT8*priority);

static INT32 f_stv0362_Trl_Switch(struct nim_device *dev);
static INT32 f_stv0362_get_specinv(struct nim_device *dev, UINT8 *Inv);
//static INT32 f_stv0362_i2c_bypass(struct nim_device *dev);
void f_stv0362_CC_tracking_task(UINT32 param1);

//TRLNOMRATE
INT32 f_get_TRL_OFFSET(struct nim_device *dev);
static INT32 f_stv0362_Core_Switch(struct nim_device *dev);
INT32 f_stv0362_get_TRLNOMRATE(struct nim_device *dev);
void f_stv0362_set_TRLNOMRATE(struct nim_device *dev,UINT32  value);
static INT32 f_stv0362_CC_tracking_task_init(struct nim_device *dev);
static INT32 f_stv0362_TRLNOMRATE_Tuning(struct nim_device *dev, BOOL Tracking_Mode);

//weiching added function for helping Register,Field,Mask processing [begin]
static UINT32 f_stv0362_ArrayIdx_of_Reg(UINT16 reg_addr); //here, use UINT16 reg_addr to meet ST register definition!
static UINT8 f_stv0362_Mask_of_RegField(UINT32 reg_field); //here, use UINT32 reg_field to meet ST register field definition!
static UINT16 f_stv0362_RegAddr_of_RegField(UINT32 reg_field); //here, use UINT32 reg_field to meet ST register field definition!
static UINT8 f_stv0362_BitShiftCount_of_RegFieldMask(UINT8 reg_field_mask); //8-bit register
//weiching added function for helping Register,Field,Mask processing [end]

//weiching added functions to meet STV0362 code Register Access usage, and thus can ureduce some porting efforts [begin]
static void ChipSetOneRegister(struct nim_device *dev, UINT16 reg_addr, UINT8 value); //here, use UINT16 reg_addr to meet ST register definition!
static UINT8 ChipGetOneRegister(struct nim_device *dev, UINT16 reg_addr); //here, use UINT16 reg_addr to meet ST register definition!
static void ChipSetField(struct nim_device *dev, UINT32 reg_field, UINT8 value); //here, use UINT32 reg_field to meet ST register field definition!
static UINT8 ChipGetField(struct nim_device *dev, UINT32 reg_field); //here, use UINT32 reg_field to meet ST register field definition!
static void ChipSetRegisters(struct nim_device *dev, UINT16 first_reg_addr, UINT8 *RegsVal, int Number); //here, use UINT16 first_reg_addr to meet ST register definition!
static void ChipGetRegisters(struct nim_device *dev, UINT16 first_reg_addr, int Number, UINT8 *RegsVal); //here, use UINT16 first_reg_addr to meet ST register definition!
//weiching added functions to meet STV0362 code Register Access usage, and thus can ureduce some porting efforts [end]

//copied and tuned from ST code: src\drivers\ter\include\d0362_drv.h --- [begin]
static FE_362_Error_t  FE_362_Search(struct nim_device *dev, FE_362_SearchParams_t *pSearch, FE_362_InternalParams_t *Params, FE_362_SearchResult_t *pResult);
static void FE_362_FilterCoeffInit(struct nim_device *dev, UINT16 CellsCoeffs[][5]);
static FE_362_SignalStatus_t CheckSYR_362(struct nim_device *dev);
static INT16 duration(INT32 mode, int tempo1, int tempo2, int tempo3);
//copied and tuned from ST code: src\drivers\ter\include\d0362_drv.h --- [end]

/* ALi scan info (common) */
struct stv0362_Lock_Info
{
    UINT8  FECRates;
    UINT8  HPRates;
    UINT8  LPRates;
    UINT8  Modulation;
    UINT8  Mode;
    UINT8  Guard;
    UINT8  Force;
    UINT8  Hierarchy;
    UINT8  Priority;
    UINT8  Spectrum;
    UINT8  ChannelBW;
    UINT8  TrlTunning;
    UINT32 Frequency;
    INT8   EchoPos;
    INT32  FreqOffset;
    UINT8  Dis_TS_Output;

    //CC_tracking
    UINT8  CC_Tracking_Scan_Type;
    UINT8  CC_Tracking_FECRates;
    UINT8  CC_Tracking_HPRates;
    UINT8  CC_Tracking_LPRates;
    UINT8  CC_Tracking_Modulation;
    UINT8  CC_Tracking_Mode;
    UINT8  CC_Tracking_Guard;
    UINT8  CC_Tracking_Hierarchy;
    UINT8  CC_Tracking_Spectrum;
    UINT8  CC_Tracking_ChannelBW;
    UINT32 CC_Tracking_Frequency;
    UINT8  CC_Tracking_flag;
    UINT8  CC_Tracking_staus;
    UINT8  Lock_Val;
    UINT16 AGC_Val ;
    UINT8  SNR_Val;
    UINT32 BER_Val;
    UINT32 PER_Val;
    UINT8  BER_HB;
    UINT8  BER_LB;
    UINT8  PER_HB;
    UINT8  PER_LB;
};


#if 1 //temp added to meet ST code for compilation
//note: copied and tuned from ST code: src\drivers\ter\include\d0362.h --- [begin]
//steven: add new. Copy from ST D0362_InstanceData_t type
    typedef struct
    {
        STTUNER_Spectrum_t        ResultSpectrum; /* Used to retain the result to value of spectrum to let tunerinfo know the value */
        UINT32                    UnlockCounter;  /* Used to check unlock counter value before performing core on/off in tuner tracking */
        FE_362_SearchResult_t     Result;
        FE_362_SearchParams_t     Search;
   } STTUNER_SearchResultData_t;
//note: copied and tuned from ST code: src\drivers\ter\include\d0362.h --- [end]

//copied and tuned from ST code: src\drivers\ter\include\d0362.c --- [begin]
static INT32 f_stv0362_StandByMode(struct nim_device *dev, STTUNER_StandByMode_t PowerMode);

/* ST native local functions */
static void ST_d0362_GetSignalQuality(struct nim_device *dev, UINT32 *SignalQuality_p, UINT32 *Ber);
static void ST_d0362_GetModulation(struct nim_device *dev, STTUNER_Modulation_t *Modulation);
static void ST_d0362_GetFECRates(struct nim_device *dev, STTUNER_FECRate_t *FECRates);
static void ST_d0362_GetMode(struct nim_device *dev, STTUNER_Mode_t *Mode);
static void ST_d0362_GetGuard(struct nim_device *dev, STTUNER_Guard_t *Guard);
static FE_362_Error_t ST_d0362_GetTPSCellId(struct nim_device *dev, UINT16 *TPSCellId);
static void ST_d0362_GetTunerInfo(struct nim_device *dev, STTUNER_TunerInfo_t *TunerInfo_p);
FE_362_Error_t ST_d0362_ScanFrequency(struct nim_device *dev,
                                      UINT32  InitialFrequency,
                                      BOOL    *ScanSuccess,
                                      UINT32  Mode,
                                      UINT32  Guard,
                                      UINT32  Force,
                                      UINT32  Hierarchy,
                                      UINT32  Spectrum,
                                      UINT32  FreqOff,
                                      UINT32  ChannelBW,
                                      UINT32  IF_IQ_Mode,
                                      INT32   EchoPos);

static void ST_d0362_PLLDIV_TRL_INC_ioctl_set(struct nim_device *dev, void *InParams);
static void ST_d0362_PLLDIV_TRL_INC_ioctl_get(struct nim_device *dev, void *OutParams);
static void ST_d0362_repeateraccess(struct nim_device *dev, BOOL REPEATER_STATUS);
//copied and tuned from ST code: src\drivers\ter\include\d0362.c --- [end]

#endif //temp added to meet ST code for compilation


#endif  /* __LLD_NIM_STV0362_H__ */
