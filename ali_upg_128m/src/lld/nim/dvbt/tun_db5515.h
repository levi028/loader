/*-----------------------------------------------------------------------------
  DB5515revB tuner porting source Header file

  Write by DongWook KIM
  date : 2012.02.10
  ver :0.0.21


  ===================initial setting=================
  m_bwsel_6 = 0; //6MHz ='0', 7MHz='1', 8MHz='2'
  m_fData_CLK = 27;   //Main CLK frequency, unit MHz
  m_OpSysSel = 0;  //if m_OpSysSel = 1, it is cable system. if m_OpSysSel = 0, it is terrestrial.
  m_fData_RF : RF input frequency
  m_fData_IF : IF input frequency
-------------------------------------------------------------------------------*/
#ifndef _DB5515_TUN_
#define _DB5515_TUN_

//#include "resource.h"
#include <types.h>

int m_bwsel_6;
//m_bwsel_6 = 0; //6MHz ='0', 7MHz='1', 8MHz='2'
float m_fData_CLK;
//m_fData_CLK = 27;   //Main CLK frequency, unit MHz

int m_OpSysSel;
//m_OpSysSel = 0;  //if m_OpSysSel = 1, it is cable system. if m_OpSysSel = 0, it is terrestrial.


double m_fData_RF; // For DB5510 Test
double m_fData_IF;

double m_fData_VCO; // For DB5510
float m_fData_LODIV;
int m_diven;
int m_Mixer_select;
int m_LNA1_select;
int m_TRACKFLT_select;
int m_HPF_select;
int    m_VCO_select;

float m_fADC_rate;

int    m_CSF_ROM_SEL;
int m_DCF_ROM_SEL;
int m_DCF2_ROM_SEL;

UINT32 u8SD_DIVWORD; //dwkim 100823
UINT32 u23SD_FIN_A; //dwkim 100823
UINT32 u23SD_FIN; //dwkim 100823

int m_iDataLODIV;

int m_FSubband; // For DB5510
int m_FADC_Div; // For DB5510

int m_AGC_RFPD_TH;
int m_AGC_RFPD_TUNE;
int m_AGC_RFPD_UPPER;
int m_AGC_RFPD_LOWER;

int m_LODIV_ctrl_bit;

int m_ctrl_pll2;
int m_ctrl_sd3;

int m_DCF_COEFF_0;
int m_DCF_COEFF_1;
int m_DCF_COEFF_2;
int m_DCF_COEFF_3;
int crdc0; // DB5510
int crdc1; // DB5510


int m_DACoutput;

int m_4mhz_dig;
int m_AGC_RFPD_TUNE2;

int m_mix_cap_ctrl;

int m_bblna_pole;
int m_lb_trackflt_msb;
int m_lb_trackflt_lsb;
int m_mb_trackflt_msb;
int m_mb_trackflt_lsb;


BOOL CalculateLODIV(int m_bwsel_6, int m_OpSysSel);

BOOL CalculateVCO(int m_bwsel_6);

BOOL CalculateFractionalMode(float m_fData_CLK);
void ProcEvent(void);
void SetRegChannel(UINT32 tuner_id);

BOOL m_spectrum;

int m_lna_rfbsel;
int m_rf_lowg_bdr;


int    m_ctrl_pll4;
int m_sd_sdorder;      // sigma detal order control depending on sub-ban
int m_dcf_rom_byp;

//Register Addresses
static const int A_SYS_CTL           = 0x0000;
static const int A_OP_MODE           = 0x0001;
static const int A_VERSION           = 0x0002;
static const int A_I2C_AUTOINC_CW    = 0x0003;
static const int A_DIG_LOOP_CW       = 0x0004;
static const int A_CHANNEL_INFO      = 0x0005;
static const int A_FCH_FCW_MSB       = 0x0006;
static const int A_FCH_FCW_LSB       = 0x0007;
static const int A_FIF_FCW_MSB       = 0x0008;
static const int A_FIF_FCW_LSB       = 0x0009;
static const int A_FCW_MODE          = 0x000A;
static const int A_CRDC_CW_0         = 0x000B;
static const int A_CRDC_CW_1         = 0x000C;
static const int A_XTAL_SEL          = 0x000D;
static const int A_CTRL_GPO1         = 0x000E;
static const int A_CTRL_GPO2         = 0x000F;
static const int A_RFAGC_CSF_PWR_EST = 0x0010;
static const int A_RFGAIN_IDX        = 0x0011;
static const int A_BBGAIN_IDX        = 0x0012;
static const int A_CTRL_LOOP_THRU1   = 0x0013;
static const int A_CTRL_LOOP_THRU2   = 0x0014;
static const int A_CTRL_LOOP_THRU3   = 0x0015;
static const int A_LT_GCTL_DELTA00   = 0x0016;
static const int A_LT_GCTL_DELTA01   = 0x0017;
static const int A_LT_GCTL_DELTA02   = 0x0018;
static const int A_LT_GCTL_DELTA03   = 0x0019;
static const int A_LT_GCTL_DELTA04   = 0x001A;
static const int A_LT_GCTL_DELTA05   = 0x001B;
static const int A_LT_GCTL_DELTA06   = 0x001C;
static const int A_LT_GCTL_DELTA07   = 0x001D;
static const int A_LT_GCTL_DELTA08   = 0x001E;
static const int A_LT_GCTL_DELTA09   = 0x001F;
static const int A_LT_GCTL_DELTA10   = 0x0020;
static const int A_LT_GCTL_DELTA11   = 0x0021;
static const int A_LT_GCTL_DELTA12   = 0x0022;
static const int A_LT_GCTL_DELTA13   = 0x0023;
static const int A_LT_DEGENCTL_MINCODE = 0x0024;
static const int A_LT_DEGENCTL_DELTA00 = 0x0025;
static const int A_LT_DEGENCTL_DELTA01 = 0x0026;
static const int A_LT_DEGENCTL_DELTA02 = 0x0027;
static const int A_LT_DEGENCTL_DELTA03 = 0x0028;
static const int A_LT_DEGENCTL_DELTA04 = 0x0029;
static const int A_LT_DEGENCTL_DELTA05 = 0x002A;
static const int A_LT_DEGENCTL_DELTA06 = 0x002B;
static const int A_LT_DEGENCTL_DELTA07 = 0x002C;
static const int A_LT_DEGENCTL_DELTA08 = 0x002D;
static const int A_LT_DEGENCTL_DELTA09 = 0x002E;
static const int A_LT_DEGENCTL_DELTA10 = 0x002F;
static const int A_LT_DEGENCTL_DELTA11 = 0x0030;
static const int A_LT_DEGENCTL_DELTA12 = 0x0031;
static const int A_CTRL_SLAVE        = 0x0032;
static const int A_CTRL_LNA1         = 0x0033;
static const int A_CTRL_LNA2         = 0x0034;
static const int A_CTRL_LNA3         = 0x0035;
static const int A_CTRL_LNA4         = 0x0036;
static const int A_CTRL_LNA5         = 0x0037;
static const int A_CTRL_LNA6         = 0x0038;
static const int A_CTRL_PD1          = 0x0039;
static const int A_CTRL_PD2          = 0x003A;
static const int A_CTRL_TRACKFLT1    = 0x003B;
static const int A_CTRL_TRACKFLT2    = 0x003C;
static const int A_CTRL_TRACKFLT3    = 0x003D;
static const int A_CTRL_HPF_DIG      = 0x003E;
static const int A_CTRL_MIXERS1      = 0x003F;
static const int A_CTRL_MIXERS2      = 0x0040;
static const int A_CTRL_IQBB1        = 0x0041;
static const int A_CTRL_IQBB2        = 0x0042;
static const int A_CTRL_IQBB3        = 0x0043;
static const int A_CTRL_IQBB4        = 0x0044;
static const int A_CTRL_IQBB5        = 0x0045;
static const int A_CTRL_IQBB6        = 0x0046;
static const int A_CTRL_IQBB7        = 0x0047;
static const int A_CTRL_IQBB8        = 0x0048;
static const int A_CTRL_MPADC        = 0x0049;
static const int A_CTRL_OUTDAC       = 0x004A;
static const int A_CTRL_PLL1         = 0x004B;
static const int A_CTRL_PLL2         = 0x004C;
static const int A_CTRL_PLL3         = 0x004D;
static const int A_CTRL_PLL4         = 0x004E;
static const int A_CTRL_PLL5         = 0x004F;
static const int A_CTRL_PLL6         = 0x0050;
static const int A_CTRL_PLL7         = 0x0051;
static const int A_CTRL_PLL8         = 0x0052;
static const int A_CTRL_PLL9         = 0x0053;
static const int A_CTRL_SD1          = 0x0054;
static const int A_CTRL_SD2          = 0x0055;
static const int A_CTRL_CLK_GEN      = 0x0056;
static const int A_CTRL_TCXO1        = 0x0057;
static const int A_CTRL_TCXO2        = 0x0058;
static const int A_CTRL_VREG_TRIM    = 0x0059;
static const int A_CTRL_BG_TEST      = 0x005A;
static const int A_IQMC_CTRL         = 0x005B;
static const int A_COEF_ROM_SEL      = 0x005C;
static const int A_DCF_MODE_SEL      = 0x005D;
static const int A_PCF_MODE_SEL      = 0x005E;
static const int A_DCF_COEFF_0       = 0x005F;
static const int A_DCF_COEFF_1       = 0x0060;
static const int A_DCF_COEFF_2       = 0x0061;
static const int A_DCF_COEFF_3       = 0x0062;
static const int A_CSF_COEFF_00      = 0x0063;
static const int A_CSF_COEFF_01      = 0x0064;
static const int A_CSF_COEFF_02      = 0x0065;
static const int A_CSF_COEFF_03      = 0x0066;
static const int A_CSF_COEFF_04      = 0x0067;
static const int A_CSF_COEFF_05      = 0x0068;
static const int A_CSF_COEFF_06      = 0x0069;
static const int A_CSF_COEFF_07      = 0x006A;
static const int A_CSF_COEFF_08      = 0x006B;
static const int A_CSF_COEFF_09      = 0x006C;
static const int A_CSF_COEFF_10      = 0x006D;
static const int A_CSF_COEFF_11      = 0x006E;
static const int A_CSF_COEFF_12      = 0x006F;
static const int A_CSF_COEFF_13      = 0x0070;
static const int A_CSF_COEFF_14      = 0x0071;
static const int A_CSF_COEFF_15      = 0x0072;
static const int A_CSF_COEFF_16      = 0x0073;
static const int A_CSF_COEFF_17      = 0x0074;
static const int A_CSF_COEFF_18      = 0x0075;
static const int A_CSF_COEFF_19      = 0x0076;
static const int A_CSF_COEFF_20      = 0x0077;
static const int A_CSF_COEFF_21      = 0x0078;
static const int A_CSF_COEFF_22      = 0x0079;
static const int A_CSF_COEFF_23      = 0x007A;
static const int A_CSF_COEFF_24      = 0x007B;
static const int A_CSF_COEFF_25      = 0x007C;
static const int A_CSF_COEFF_26      = 0x007D;
static const int A_CSF_COEFF_27      = 0x007E;
static const int A_CSF_COEFF_28      = 0x007F;
static const int A_CSF_COEFF_29      = 0x0080;
static const int A_CSF_COEFF_30      = 0x0081;
static const int A_CSF_COEFF_31      = 0x0082;
static const int A_PCF_CTL_0         = 0x0083;
static const int A_PCF_CTL_1         = 0x0084;
static const int A_GMC_TUNE_CW       = 0x0085;
static const int A_GMC_TUNE_COMP     = 0x0086;
static const int A_RFPD_CAL_CW       = 0x0087;
static const int A_TL_WAIT_TIME_CTL0 = 0x0088;
static const int A_TL_WAIT_TIME_CTL1 = 0x0089;
static const int A_ADC_OL_CTL        = 0x008A;
static const int A_AGC_CTL1          = 0x008B;
static const int A_AGC_CTL2          = 0x008C;
static const int A_AGC_CTL3          = 0x008D;
static const int A_AGC_RFPD_THRESH_CTRL    = 0x008E;
static const int A_AGC_BBPD_THRESH_CTRL    = 0x008F;
static const int A_AGC_CSFPD_THRESH_CTRL1  = 0x0090;
static const int A_AGC_CSFPD_THRESH_CTRL2  = 0x0091;
static const int A_AGC_DOC_CW1             = 0x0092;
static const int A_AGC_DLY1                = 0x0093;
static const int A_AGC_DLY2                = 0x0094;
static const int A_AGC_BBDAC_CW3           = 0x0095;
static const int A_AGC_DET_FLAG_THRES      = 0x0096;
static const int A_AGC_DOC_DAC_LMT         = 0x0097;
static const int A_AGC_DOC_CW3             = 0x0098;
static const int A_AGC_RFGTBL_ZONE_MUXSEL1 = 0x0099;
static const int A_AGC_RFGTBL_ZONE_MUXSEL2 = 0x009A;
static const int A_AGC_RFGTBL_ZONE_BDRY0   = 0x009B;
static const int A_AGC_RFGTBL_ZONE_BDRY1   = 0x009C;
static const int A_AGC_RFGTBL_ZONE_BDRY2   = 0x009D;
static const int A_AGC_RFGTBL_ZONE_BDRY3   = 0x009E;
static const int A_AGC_RFGTBL_ZONE_BDRY4   = 0x009F;
static const int A_AGC_RFGTBL_ZONE_BDRY5   = 0x00A0;
static const int A_AGC_ZONE0_DFLT_CTL1   = 0x00A1;
static const int A_AGC_ZONE0_DFLT_CTL2   = 0x00A2;
static const int A_AGC_ZONE0_DFLT_CTL3   = 0x00A3;
static const int A_AGC_ZONE1_DFLT_CTL1   = 0x00A4;
static const int A_AGC_ZONE1_DFLT_CTL2   = 0x00A5;
static const int A_AGC_ZONE1_DFLT_CTL3   = 0x00A6;
static const int A_AGC_ZONE2_DFLT_CTL1   = 0x00A7;
static const int A_AGC_ZONE2_DFLT_CTL2   = 0x00A8;
static const int A_AGC_ZONE2_DFLT_CTL3   = 0x00A9;
static const int A_AGC_ZONE3_DFLT_CTL1   = 0x00AA;
static const int A_AGC_ZONE3_DFLT_CTL2   = 0x00AB;
static const int A_AGC_ZONE3_DFLT_CTL3   = 0x00AC;
static const int A_AGC_ZONE4_DFLT_CTL1   = 0x00AD;
static const int A_AGC_ZONE4_DFLT_CTL2   = 0x00AE;
static const int A_AGC_ZONE4_DFLT_CTL3   = 0x00AF;
static const int A_AGC_ZONE5_DFLT_CTL1   = 0x00B0;
static const int A_AGC_ZONE5_DFLT_CTL2   = 0x00B1;
static const int A_AGC_ZONE5_DFLT_CTL3   = 0x00B2;
static const int A_AGC_ZONE6_DFLT_CTL1   = 0x00B3;
static const int A_AGC_ZONE6_DFLT_CTL2   = 0x00B4;
static const int A_AGC_ZONE6_DFLT_CTL3   = 0x00B5;
static const int A_PW_DOC_CW1            = 0x00B6;
static const int A_PW_DOC_CW2            = 0x00B7;
static const int A_CSF_PWR_EST1          = 0x00B8;
static const int A_CSF_PWR_EST2          = 0x00B9;
static const int A_EXTIFAGC_REG1         = 0x00BA;
static const int A_EXTIFAGC_REG7         = 0x00BB;
static const int A_EXTIFAGC_MAXG_CODE    = 0x00BC;
static const int A_EXTIFAGC_MAXATT_CODE  = 0x00BD;
static const int A_CKMREGXTAL            = 0x00BE;
static const int A_TICK_TIMER_CW         = 0x00BF;
static const int A_CKM_DLY_CTL           = 0x00C0;
static const int A_RFAGC_MON0            = 0x00C1;
static const int A_RFAGC_MON1            = 0x00C2;
static const int A_RFAGC_MON2            = 0x00C3;
static const int A_RFPD_CAL_MON          = 0x00C4;
static const int A_RFAGC_FLAG_MON        = 0x00C5;
static const int A_AUXAGC_MON0           = 0x00C6;
static const int A_AUXAGC_MON1           = 0x00C7;
static const int A_DC_MON_I              = 0x00C8;
static const int A_DC_MON_Q              = 0x00C9;
static const int A_PLL_MON               = 0x00CA;
static const int A_GMC_TUNE_MEAS         = 0x00CB;
static const int A_GMC_DAC_RD            = 0x00CC;

int D_FCH_FCW_LSB;
int D_FCH_FCW_MSB;
int D_FIF_FCW_LSB;
int D_FIF_FCW_MSB;

int m_fData_RF_Hz;
int m_fData_IF_Hz;

int m_bbpd_tune_thres;

#endif
