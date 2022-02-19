/*-----------------------------------------------------------------------------
  DB5515revB tuner porting source Header file

  Write by DongWook KIM
  date : 2012.02.10

  [version history]
   - version 0.0.16
   - 2012.03.07 : version 0.0.19
 -------------------------------------------------------------------------------*/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_db5515.h"
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

//#define DB5515_DEBUG  1
#if(DB5515_DEBUG)
#define DB5515_PRINTF  libc_printf
#else
#define DB5515_PRINTF(...)
#endif

/*********************************************************
* ALI
* Tuner DB5515 driver
* Douglass(2012-03-19)
*
**********************************************************/

struct COFDM_TUNER_CONFIG_EXT DB5515_Config[2];
//static DB5515_TunerConfigS DB5515_TunerConfig[2];  //Structure for save local config for tuner driver
__MAYBE_UNUSED__ static BOOL DB5515_Tuner_Inited[2];
static UINT32 tuner_cnt = 0;

#ifdef DB5515_DEBUG
UINT8 reg_dump[256];
#endif

/*****************************************
*I2C Read/Write interface
*
*****************************************/
#define DB_BURST_SZ         4
#define TUN_ADDR        0xc2        //for debug test define
#define TUN_ID        0x00        //for debug test define
typedef enum
{
    DB_BW_6MHz = 6,
    DB_BW_7MHz = 7,
    DB_BW_8MHz = 8
} DB_BW_MHz;

static int Write(UINT32 tuner_id,int Address, int Data)
{
    UINT32 count = 4;
    UINT8 pArray[4];
    int status = 0;
    pArray[0] = (Address >> 8) & 0x00ff;
    pArray[1] = Address & 0x00ff;
    pArray[2] = (Data >> 8) & 0x00ff;
    pArray[3] = Data & 0x00ff;
    status =DB5515_Config[tuner_id].tuner_write(DB5515_Config[tuner_id].i2c_type_id, DB5515_Config[tuner_id].c_tuner_base_addr, pArray, count);
    return status;
}

static UINT32 Read(UINT32 tuner_id,UINT32 Address)
{
    //UINT8 DeviceAddr = TUN_ADDR;
    //UINT32 DB_Addr = Address;
    UINT32 rtn_val=0x00;
    UINT8 sData[2]={0x00,0x00};
    UINT8 R_Cmd[2];
    R_Cmd[0] = (Address >> 8) & 0x00ff;
    R_Cmd[1] = Address & 0x00ff;
    DB5515_Config[tuner_id].tuner_write(DB5515_Config[tuner_id].i2c_type_id, DB5515_Config[tuner_id].c_tuner_base_addr, R_Cmd, 2);
    DB5515_Config[tuner_id].tuner_read(DB5515_Config[tuner_id].i2c_type_id, DB5515_Config[tuner_id].c_tuner_base_addr, sData, 2);
    rtn_val=sData[1] | sData[0]<<8;
    return rtn_val;

}
//------------------------------------------------------------------------------------------------
// Assign SubBand Number, LO Divider, ADC Divider and Mixer Selection from RF-IN Frequency
// Sub-Band = 1 to 36 at RF Frequenc 39.4M ~ 1200MHz
//   - m_fData_RF : RF-IN frequency
//   - m_fData_LODIV : LO Divider
//   - m_FSubband : Subband Number
//   - m_FADC_Div : ADC Divider
//   - m_Mixer_select : Mixer Selection.
//
//   - m_bwsel_6 = 0; // IF Bandwidth = 6MHz
//   - m_bwsel_6 == 1; // IF Bandwidth = 7MHz
//   - m_bwsel_6 == 2; // IF Bandwidth = 8MHz
//   - if Cable TV system,
//     m_OpSysSel == 1
//   - if Terrestrial TV system,
//     m_OpSysSel == 0
//========================= for example ==================================================
//    m_fData_RF = atof(argv[3]); // RF frequncy
//
//    m_fData_IF = atof(argv[4]); // IF frequency
//    m_bwsel_6 = 0; //6MHz ='0', 7MHz='1', 8MHz='2' // IF bandwidth
//    m_fData_CLK = 27; // X-tal frequcny
//    m_OpSysSel = 0; // select to Terrestrial TV system or Cable TV system
//
//    CalculateLODIV(m_bwsel_6, m_OpSysSel);
//    DB5515_PRINTF("CalculateLODIV complet -> 11\n");
//    CalculateVCO(m_bwsel_6);
//    DB5515_PRINTF("CalculateVCO complet -> 22\n");
//    CalculateFractionalMode(m_fData_CLK);
//    DB5515_PRINTF("CalculateFractionalMode complet -> 33\n");
//    CalculateTrackflt();
//    DB5515_PRINTF("CalculateTrackflt complet -> 44\n");
//    SetRegChannel();
//    DB5515_PRINTF("SetRegChannel complet -> 55\n");
//------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//  description : define LO divider variable function due to frequency setting
//  variable : m_bwsel_6, m_OpSysSel
//  description of variable :
//   - m_bwsel_6 = 0; // IF Bandwidth = 6MHz
//   - m_bwsel_6 == 1; // IF Bandwidth = 7MHz
//   - m_bwsel_6 == 2; // IF Bandwidth = 8MHz
//   - if Cable TV system,
//     m_OpSysSel == 1
//   - if Terrestrial TV system,
//     m_OpSysSel == 0
//  return : TRUE or FALSE
// =============================== for example ===================================================
//  **** for example, ISDB-T, IF Bandwidth = 6MHz, Terrestrial TV system
//  **** => m_bwsel_6 = 0, m_OpSysSel == 0
//------------------------------------------------------------------------------------------------
BOOL CalculateLODIV(int m_bwsel_6, int m_OpSysSel)
{

    //int    i2c_temp=0, i2c_temp1=0;



    BOOL bResult  = FALSE;
    m_fData_LODIV = 0;
    m_iDataLODIV  = -1;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // PLL Table:  Calculate  m_Mixer_select, m_fData_LODIV, m_FSubband, m_FADC_Div
    if      ((37.5 <= m_fData_RF) && (m_fData_RF < 39.4) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 1;   m_FADC_Div = 5;   bResult = TRUE;}
    else if ((39.4 <= m_fData_RF) && (m_fData_RF < 42.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 2;   m_FADC_Div = 6;   bResult = TRUE;}
    else if ((42.0 <= m_fData_RF) && (m_fData_RF < 47.3) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 3;   m_FADC_Div = 6;   bResult = TRUE;}
    else if ((47.3 <= m_fData_RF) && (m_fData_RF < 55.2) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 4;   m_FADC_Div = 7;   bResult = TRUE;}
    else if ((55.2 <= m_fData_RF) && (m_fData_RF < 63.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 5;   m_FADC_Div = 8;   bResult = TRUE;}
    else if ((63.0 <= m_fData_RF) && (m_fData_RF < 70.9) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 6;   m_FADC_Div = 9;   bResult = TRUE;}
    else if ((70.9 <= m_fData_RF) && (m_fData_RF < 75.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 32;  m_FSubband = 7;   m_FADC_Div = 10;  bResult = TRUE;}
    else if ((75.0 <= m_fData_RF) && (m_fData_RF < 78.8) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 8;   m_FADC_Div = 5;   bResult = TRUE;}
        else if ((78.8 <= m_fData_RF) && (m_fData_RF < 84.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 9;   m_FADC_Div = 6;   bResult = TRUE;}
        else if ((84.0 <= m_fData_RF) && (m_fData_RF < 94.6) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 10;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((94.6 <= m_fData_RF) && (m_fData_RF < 110.3) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 11;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((110.3 <= m_fData_RF) && (m_fData_RF < 126.1) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 12;  m_FADC_Div = 8;   bResult = TRUE;}
    else if ((126.1 <= m_fData_RF) && (m_fData_RF < 141.8) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 13;  m_FADC_Div = 9;   bResult = TRUE;}
    else if ((141.8 <= m_fData_RF) && (m_fData_RF < 150.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 16;  m_FSubband = 14;  m_FADC_Div = 10;  bResult = TRUE;}
    else if ((150.0 <= m_fData_RF) && (m_fData_RF < 157.6) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 15;  m_FADC_Div = 5;   bResult = TRUE;}
    else if ((157.6 <= m_fData_RF) && (m_fData_RF < 170.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 16;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((170.0 <= m_fData_RF) && (m_fData_RF < 189.1) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 17;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((189.1 <= m_fData_RF) && (m_fData_RF < 220.6) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 18;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((220.6 <= m_fData_RF) && (m_fData_RF < 252.1) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 19;  m_FADC_Div = 8;   bResult = TRUE;}
    else if ((252.1 <= m_fData_RF) && (m_fData_RF < 283.6) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 20;  m_FADC_Div = 9;   bResult = TRUE;}
    else if ((283.6 <= m_fData_RF) && (m_fData_RF < 300.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 8;   m_FSubband = 21;  m_FADC_Div = 10;    bResult = TRUE;}
    else if ((300.0 <= m_fData_RF) && (m_fData_RF < 315.3) )
    { m_Mixer_select = 1;  m_fData_LODIV = 4;   m_FSubband = 22;  m_FADC_Div = 5;   bResult = TRUE;}
        else if ((315.3 <= m_fData_RF) && (m_fData_RF < 330.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 4;   m_FSubband = 23;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((330.0 <= m_fData_RF) && (m_fData_RF < 378.3) )
    { m_Mixer_select = 1;  m_fData_LODIV = 4;   m_FSubband = 24;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((378.3 <= m_fData_RF) && (m_fData_RF < 390.0) )
    { m_Mixer_select = 1;  m_fData_LODIV = 4;   m_FSubband = 25;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((390.0 <= m_fData_RF) && (m_fData_RF < 441.3) ) // 390MHz High Band (same subband)
    { m_Mixer_select = 2;  m_fData_LODIV = 4;   m_FSubband = 25;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((441.3 <= m_fData_RF) && (m_fData_RF < 500.0) )
    { m_Mixer_select = 2;  m_fData_LODIV = 4;   m_FSubband = 26;  m_FADC_Div = 8;   bResult = TRUE;}
    else if ((500.0 <= m_fData_RF) && (m_fData_RF < 525.0) )
    { m_Mixer_select = 2;  m_fData_LODIV = 4;   m_FSubband = 27;  m_FADC_Div = 9;   bResult = TRUE;}
    else if ((525.0 <= m_fData_RF) && (m_fData_RF < 600.0) )
    { m_Mixer_select = 2;  m_fData_LODIV = 4;   m_FSubband = 28;  m_FADC_Div = 10;  bResult = TRUE;}
    else if ((600.0 <= m_fData_RF) && (m_fData_RF < 634.5) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 29;  m_FADC_Div = 5;   bResult = TRUE;}
    else if ((634.5 <= m_fData_RF) && (m_fData_RF < 634.75) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 30;  m_FADC_Div = 6;   bResult = TRUE;}
        //     We need to change
        //        1) charge pump current to 6
        //        2) sigma delta order to 2nd order
    else if ((634.75 <= m_fData_RF) && (m_fData_RF < 670.0) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 31;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((670.0 <= m_fData_RF) && (m_fData_RF < 756.5) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 32;  m_FADC_Div = 6;   bResult = TRUE;}
    else if ((756.5 <= m_fData_RF) && (m_fData_RF < 820.0) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 33;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((820.0 <= m_fData_RF) && (m_fData_RF < 882.5) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 34;  m_FADC_Div = 7;   bResult = TRUE;}
    else if ((882.5 <= m_fData_RF) && (m_fData_RF < 1008.5) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 35;  m_FADC_Div = 8;   bResult = TRUE;}
    else if ((1008.5 <= m_fData_RF) && (m_fData_RF < 1134.5) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 36;  m_FADC_Div = 9;   bResult = TRUE;}
    else if ((1134.5 <= m_fData_RF) && (m_fData_RF < 1200.0) )
    { m_Mixer_select = 4;  m_fData_LODIV = 2;   m_FSubband = 37;  m_FADC_Div = 10;  bResult = TRUE;}
    else
    { bResult = FALSE;}
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Calculate  m_LNA1_select, m_TRACKFLT_select, m_HPF_select
    if      (m_Mixer_select == 1) {   // LB path select
        m_LNA1_select     = 0x0001;
        m_TRACKFLT_select = 0x0001;
        m_HPF_select      = 0x0000;
        bResult           = TRUE;}
    else if (m_Mixer_select == 2) {   // MB path select
        m_LNA1_select     = 0x0003;
        m_TRACKFLT_select = 0x0000;
        m_HPF_select      = 0x0001;
        bResult           = TRUE;}
    else if (m_Mixer_select == 4) {   // HB path select
        m_LNA1_select     = 0x0003;
        m_TRACKFLT_select = 0x0000;
        m_HPF_select      = 0x0001;
        bResult           = TRUE;}
    else {
        bResult           = FALSE;}
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // RF threshold setting: Calculate m_AGC_RFPD_TUNE, m_AGC_RFPD_UPPER, m_AGC_RFPD_LOWER, m_AGC_RFPD_TH
    // For Cable system, lower threshold. For Terrestrial, use as it was set
    if (m_OpSysSel == 1) {
        if      (m_fData_RF < 100)                              {m_AGC_RFPD_TUNE = 45;}
        else if ((100 <= m_fData_RF) && (m_fData_RF < 200.0) )  {m_AGC_RFPD_TUNE = 42;}
        else if ((200 <= m_fData_RF) && (m_fData_RF < 390.0) )  {m_AGC_RFPD_TUNE = 40;}
        else if ((390 <= m_fData_RF) && (m_fData_RF < 400.0) )  {m_AGC_RFPD_TUNE = 45;}
        else if ((400 <= m_fData_RF) && (m_fData_RF < 500.0) )  {m_AGC_RFPD_TUNE = 44;}
        else if ((500 <= m_fData_RF) && (m_fData_RF < 600.0) )  {m_AGC_RFPD_TUNE = 43;}
        else if ((600 <= m_fData_RF) && (m_fData_RF < 700.0) )  {m_AGC_RFPD_TUNE = 42;}
        else                                                    {m_AGC_RFPD_TUNE = 39;}
        DB5515_PRINTF("RFPD threshold set for Cable system!\n");
    }
    else {
        if      (m_fData_RF < 100)                              {m_AGC_RFPD_TUNE = 40;}
        else if ((100 <= m_fData_RF) && (m_fData_RF < 200.0) )    {m_AGC_RFPD_TUNE = 39;}
        else if ((200 <= m_fData_RF) && (m_fData_RF < 390.0) )    {m_AGC_RFPD_TUNE = 38;}
        else if ((390 <= m_fData_RF) && (m_fData_RF < 400.0) )    {m_AGC_RFPD_TUNE = 44;}
        else if ((400 <= m_fData_RF) && (m_fData_RF < 500.0) )    {m_AGC_RFPD_TUNE = 43;}
        else if ((500 <= m_fData_RF) && (m_fData_RF < 600.0) )    {m_AGC_RFPD_TUNE = 41;}
        else if ((600 <= m_fData_RF) && (m_fData_RF < 700.0) )    {m_AGC_RFPD_TUNE = 40;}
        else                                                    {m_AGC_RFPD_TUNE = 38;}
        DB5515_PRINTF("RFPD threshold set for Terrestrial system!\n");
    }

    m_AGC_RFPD_UPPER = m_AGC_RFPD_TUNE+1;
    m_AGC_RFPD_LOWER = m_AGC_RFPD_TUNE-1;
    m_AGC_RFPD_TH    = 1*128 + m_AGC_RFPD_TUNE; // delta = 1
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Calculate  m_lna_rfbsel

    if (m_OpSysSel == 1) { // For Cable
        m_lna_rfbsel = 0x0008;
    }
    else {                 // For Terrestrial
        m_lna_rfbsel = 0x000D;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Calculate  m_rf_lowg_bdr

    if (m_OpSysSel == 1) { // For Cable
        m_rf_lowg_bdr = 0x0000;
    }
    else {                 // For Terrestrial
        m_rf_lowg_bdr = 0x0337;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Calculate  m_mix_cap_ctrl

    if (m_bwsel_6 == 0)
    {m_mix_cap_ctrl = 15;}    // 6MHz BW
    else if (m_bwsel_6 == 1)
    {m_mix_cap_ctrl =  3;}    // 7MHz BW
    else
    {m_mix_cap_ctrl =  0;}    // 8MHz BW

    // m_mix_cap_ctrl = 15;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Calculate  m_bbpd_tune_thres

    if (m_bwsel_6 == 0)
    {m_bbpd_tune_thres =  0x10;}    // 6MHz BW
    else if (m_bwsel_6 == 1)
    {m_bbpd_tune_thres =  0x20;}    // 7MHz BW
    else
    {m_bbpd_tune_thres =  0x20;}    // 8MHz BW

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



    m_LODIV_ctrl_bit = 0;
    if      (m_fData_LODIV == 2)  { m_LODIV_ctrl_bit = 2;  bResult = TRUE; }
    else if (m_fData_LODIV == 4)
      { if (m_Mixer_select == 1)
          { m_LODIV_ctrl_bit = 8;  bResult = TRUE; }
        else
          { m_LODIV_ctrl_bit = 4;  bResult = TRUE; }
      }
    else if (m_fData_LODIV == 8)  { m_LODIV_ctrl_bit = 16; bResult = TRUE; }
    else if (m_fData_LODIV == 16) { m_LODIV_ctrl_bit = 32; bResult = TRUE; }
    else if (m_fData_LODIV == 32) { m_LODIV_ctrl_bit = 64; bResult = TRUE; }
    else                          { bResult = FALSE; }

    m_diven = m_LODIV_ctrl_bit;




    return bResult;
}
//-------------------------------------------------------------------------------------------
//  description : for VCO variable calculate variable function due to frequency setting
//  variable : m_bwsel_6,
//  description of variable :
//   - m_bwsel_6 = 0; // IF Bandwidth = 6MHz
//   - m_bwsel_6 == 1; // IF Bandwidth = 7MHz
//   - m_bwsel_6 == 2; // IF Bandwidth = 8MHz
//
//  return : TRUE or FALSE
// =============================== for example ===================================================
//  **** for example, ISDB-T, IF Bandwidth = 6MHz,
//  **** => m_bwsel_6 = 0,
//------------------------------------------------------------------------------------------------
BOOL CalculateVCO(int m_bwsel_6)
{
    //m_bwsel_6 = 0; // IF Bandwidth = 6MHz
    //m_bwsel_6 == 1; // IF Bandwidth = 7MHz
    //m_bwsel_6 == 2; // IF Bandwidth = 8MHz

    //int i2c_temp=0;

    BOOL bResult = FALSE;
    m_fData_VCO = 0.0f;

    if ((m_fData_RF>0) && (m_fData_LODIV>0))
    {  m_fData_VCO = m_fData_RF * m_fData_LODIV; // For DB5510
       bResult = TRUE;}
    else
    {  bResult = FALSE;}

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // PLL Table
    if      ( m_FSubband==1)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if ( m_FSubband==2)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==3 && (42.0 <= m_fData_RF) && (m_fData_RF < 43.75))
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==3 && (43.75 <= m_fData_RF) && (m_fData_RF < 47.3)) //split subband
    { bResult = TRUE;        m_ctrl_pll4 = 0x0000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==4)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==5)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==6)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==7)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==8)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==9)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==10 && (84.0 <= m_fData_RF) && (m_fData_RF < 87.5))
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==10 && (87.5 <= m_fData_RF) && (m_fData_RF < 94.6)) //split subband
    { bResult = TRUE;        m_ctrl_pll4 = 0x0000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==11)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==12)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==13)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==14)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==15)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==16)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==17 && (170.0 <= m_fData_RF) && (m_fData_RF < 175.0))
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==17 && (175.0 <= m_fData_RF) && (m_fData_RF < 189.1)) //split subband
    { bResult = TRUE;        m_ctrl_pll4 = 0x0000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==18)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==19)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==20)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==21)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==22)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==23)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==24 && (330.0 <= m_fData_RF) && (m_fData_RF < 350.0))
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==24 && (350.0 <= m_fData_RF) && (m_fData_RF < 378.3)) //split subband
    { bResult = TRUE;        m_ctrl_pll4 = 0x0000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==25)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==26)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==27)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==28)
    { bResult = TRUE;        m_ctrl_pll4 = 0x1000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==29)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2800;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==30)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;} // change needed?
    else if( m_FSubband==31)
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==32 && (670.0 <= m_fData_RF) && (m_fData_RF < 700.0))
    { bResult = TRUE;        m_ctrl_pll4 = 0x2000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==32 && (700.0 <= m_fData_RF) && (m_fData_RF < 756.5)) //split subband
    { bResult = TRUE;        m_ctrl_pll4 = 0x0000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==33)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband==34)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if( m_FSubband == 35)
    { bResult = TRUE;        m_ctrl_pll4 = 0x5000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else if(m_FSubband == 36)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}
    else    //if(m_FSubband == 37)
    { bResult = TRUE;        m_ctrl_pll4 = 0x4000;    m_ctrl_pll2 = 0x0018;        m_sd_sdorder = 0xC000;}

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------------------------------------
// Calculate CORDIC
//----------------------------------------------------------------------------------
    float crdc_temp  = (float)(m_fData_VCO/m_FADC_Div)/4;
    int   crdc_temp1 = 0;
    if(crdc_temp)
        crdc_temp1 = (int)floor((2 * 3.141592 * m_fData_IF/crdc_temp * 32768) + 0.5);

    crdc0 = (int)(crdc_temp1 & 0xffff);         // Get 16 LSBs
    crdc1 = (int)((crdc_temp1 >> 16) & 0xffff); // Get 3 MSBs

//---------------------------------------------------------------------------------
// Calcuate CSF/DCF ROM Table
//---------------------------------------------------------------------------------

    float m_FAdc_clk = (float) m_fData_VCO/m_FADC_Div; // Calculate ADC Clock
    m_fADC_rate = m_FAdc_clk;

    if( m_bwsel_6 == 0) // 6MHz
    {
        m_4mhz_dig = 0;
        //m_GMC_TUNE_COMP = 0x02df;
        m_bblna_pole = 3;
        m_dcf_rom_byp = 1;

        if( 210 <= m_FAdc_clk && m_FAdc_clk < 212)
        {   m_CSF_ROM_SEL  = 0;
            m_DCF_ROM_SEL  = 0;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x0252;
            m_DCF_COEFF_3  = 0x00d2;
            bResult        = TRUE;
        }
        else if( 212 <= m_FAdc_clk && m_FAdc_clk < 214)
        {   m_CSF_ROM_SEL  = 1;
            m_DCF_ROM_SEL  = 1;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x0252;
            m_DCF_COEFF_3  = 0x00d2;
            bResult        = TRUE;
        }
        else if( 214 <= m_FAdc_clk && m_FAdc_clk < 216)
        {   m_CSF_ROM_SEL  = 2;
            m_DCF_ROM_SEL  = 2;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x0252;
            m_DCF_COEFF_3  = 0x00d2;
            bResult        = TRUE;
        }
        else if( 216 <= m_FAdc_clk && m_FAdc_clk < 218)
        {   m_CSF_ROM_SEL  = 3;
            m_DCF_ROM_SEL  = 3;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x0252;
            m_DCF_COEFF_3  = 0x00d2;
            bResult        = TRUE;
        }
        else if( 218 <= m_FAdc_clk && m_FAdc_clk < 220)
        {   m_CSF_ROM_SEL  = 4;
            m_DCF_ROM_SEL  = 4;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x024d;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 220 <= m_FAdc_clk && m_FAdc_clk < 222)
        {   m_CSF_ROM_SEL  = 5;
            m_DCF_ROM_SEL  = 5;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x024d;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 222 <= m_FAdc_clk && m_FAdc_clk < 224)
        {   m_CSF_ROM_SEL  = 6;
            m_DCF_ROM_SEL  = 6;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x024d;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 224 <= m_FAdc_clk && m_FAdc_clk < 226)
        {   m_CSF_ROM_SEL  = 7;
            m_DCF_ROM_SEL  = 7;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0009;
            m_DCF_COEFF_1  = 0x0012;
            m_DCF_COEFF_2  = 0x024d;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 226 <= m_FAdc_clk && m_FAdc_clk < 228)
        {   m_CSF_ROM_SEL  = 8;
            m_DCF_ROM_SEL  = 8;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000f;
            m_DCF_COEFF_2  = 0x024e;
            m_DCF_COEFF_3  = 0x00cf;
            bResult        = TRUE;
        }
        else if( 228 <= m_FAdc_clk && m_FAdc_clk < 230)
        {   m_CSF_ROM_SEL  = 9;
            m_DCF_ROM_SEL  = 9;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000f;
            m_DCF_COEFF_2  = 0x024e;
            m_DCF_COEFF_3  = 0x00cf;
            bResult        = TRUE;
        }
        else if( 230 <= m_FAdc_clk && m_FAdc_clk < 232)
        {   m_CSF_ROM_SEL  = 10;
            m_DCF_ROM_SEL  = 10;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000f;
            m_DCF_COEFF_2  = 0x024e;
            m_DCF_COEFF_3  = 0x00cf;
            bResult        = TRUE;
        }
        else if( 232 <= m_FAdc_clk && m_FAdc_clk < 234)
        {   m_CSF_ROM_SEL  = 11;
            m_DCF_ROM_SEL  = 11;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0247;
            m_DCF_COEFF_3  = 0x00d4;
            bResult        = TRUE;
        }
        else if( 234 <= m_FAdc_clk && m_FAdc_clk < 236)
        {   m_CSF_ROM_SEL  = 12;
            m_DCF_ROM_SEL  = 12;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0247;
            m_DCF_COEFF_3  = 0x00d4;
            bResult        = TRUE;
        }
        else if( 236 <= m_FAdc_clk && m_FAdc_clk < 238)
        {   m_CSF_ROM_SEL  = 13;
            m_DCF_ROM_SEL  = 13;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0247;
            m_DCF_COEFF_3  = 0x00d4;
            bResult        = TRUE;
        }
        else if( 238 <= m_FAdc_clk && m_FAdc_clk < 240)
        {   m_CSF_ROM_SEL  = 14;
            m_DCF_ROM_SEL  = 14;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 240 <= m_FAdc_clk && m_FAdc_clk < 242)
        {   m_CSF_ROM_SEL  = 15;
            m_DCF_ROM_SEL  = 15;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 242 <= m_FAdc_clk && m_FAdc_clk < 244)
        {   m_CSF_ROM_SEL  = 16;
            m_DCF_ROM_SEL  = 16;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 244 <= m_FAdc_clk && m_FAdc_clk < 246)
        {   m_CSF_ROM_SEL  = 17;
            m_DCF_ROM_SEL  = 17;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 246 <= m_FAdc_clk && m_FAdc_clk < 248)
        {   m_CSF_ROM_SEL  = 18;
            m_DCF_ROM_SEL  = 18;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 248 <= m_FAdc_clk && m_FAdc_clk < 250)
        {   m_CSF_ROM_SEL  = 19;
            m_DCF_ROM_SEL  = 19;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 250 <= m_FAdc_clk && m_FAdc_clk < 252)
        {   m_CSF_ROM_SEL  = 20;
            m_DCF_ROM_SEL  = 20;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else if( 252 <= m_FAdc_clk && m_FAdc_clk < 254)
        {   m_CSF_ROM_SEL  = 21;
            m_DCF_ROM_SEL  = 21;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x0007;
            m_DCF_COEFF_1  = 0x000e;
            m_DCF_COEFF_2  = 0x0245;
            m_DCF_COEFF_3  = 0x00d6;
            bResult        = TRUE;
        }
        else
        {   //DB5515_PRINTF("ROM Selection Errors");
            bResult = FALSE;
        }
    }
    else if(m_bwsel_6 == 1 ) //7MHz
    {
        m_4mhz_dig =1;
        //m_GMC_TUNE_COMP = 0x02df;
        m_bblna_pole = 1;
        m_dcf_rom_byp = 1;

        if( 210 <= m_FAdc_clk && m_FAdc_clk < 212)
        {    m_CSF_ROM_SEL  = 32;
            m_DCF_ROM_SEL  = 32;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 212 <= m_FAdc_clk && m_FAdc_clk < 214)
        {    m_CSF_ROM_SEL  = 33;
            m_DCF_ROM_SEL  = 33;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 214 <= m_FAdc_clk && m_FAdc_clk < 216)
        {    m_CSF_ROM_SEL  = 34;
            m_DCF_ROM_SEL  = 34;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 216 <= m_FAdc_clk && m_FAdc_clk < 218)
        {    m_CSF_ROM_SEL  = 35;
            m_DCF_ROM_SEL  = 35;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 218 <= m_FAdc_clk && m_FAdc_clk < 220)
        {    m_CSF_ROM_SEL  = 36;
            m_DCF_ROM_SEL  = 36;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 220 <= m_FAdc_clk && m_FAdc_clk < 222)
        {    m_CSF_ROM_SEL  = 37;
            m_DCF_ROM_SEL  = 37;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 222 <= m_FAdc_clk && m_FAdc_clk < 224)
        {    m_CSF_ROM_SEL  = 38;
            m_DCF_ROM_SEL  = 38;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 224 <= m_FAdc_clk && m_FAdc_clk < 226)
        {    m_CSF_ROM_SEL  = 39;
            m_DCF_ROM_SEL  = 39;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x001f;
            m_DCF_COEFF_2  = 0x0265;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 226 <= m_FAdc_clk && m_FAdc_clk < 228)
        {    m_CSF_ROM_SEL  = 40;
            m_DCF_ROM_SEL  = 40;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x0020;
            m_DCF_COEFF_2  = 0x025d;
            m_DCF_COEFF_3  = 0x00e2;
            bResult        = TRUE;
        }
        else if( 228 <= m_FAdc_clk && m_FAdc_clk < 230)
        {    m_CSF_ROM_SEL  = 41;
            m_DCF_ROM_SEL  = 41;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x0020;
            m_DCF_COEFF_2  = 0x025d;
            m_DCF_COEFF_3  = 0x00e2;
            bResult        = TRUE;
        }
        else if( 230 <= m_FAdc_clk && m_FAdc_clk < 232)
        {    m_CSF_ROM_SEL  = 42;
            m_DCF_ROM_SEL  = 42;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x0010;
            m_DCF_COEFF_1  = 0x0020;
            m_DCF_COEFF_2  = 0x025d;
            m_DCF_COEFF_3  = 0x00e2;
            bResult        = TRUE;
        }
        else if( 232 <= m_FAdc_clk && m_FAdc_clk < 234)
        {    m_CSF_ROM_SEL  = 43;
            m_DCF_ROM_SEL  = 43;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 234 <= m_FAdc_clk && m_FAdc_clk < 236)
        {    m_CSF_ROM_SEL  = 44;
            m_DCF_ROM_SEL  = 44;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 236 <= m_FAdc_clk && m_FAdc_clk < 238)
        {    m_CSF_ROM_SEL  = 45;
            m_DCF_ROM_SEL  = 45;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 238 <= m_FAdc_clk && m_FAdc_clk < 240)
        {    m_CSF_ROM_SEL  = 46;
            m_DCF_ROM_SEL  = 46;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 240 <= m_FAdc_clk && m_FAdc_clk < 242)
        {    m_CSF_ROM_SEL  = 47;
            m_DCF_ROM_SEL  = 47;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 242 <= m_FAdc_clk && m_FAdc_clk < 244)
        {    m_CSF_ROM_SEL  = 48;
            m_DCF_ROM_SEL  = 48;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 244 <= m_FAdc_clk && m_FAdc_clk < 246)
        {    m_CSF_ROM_SEL  = 49;
            m_DCF_ROM_SEL  = 49;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 246 <= m_FAdc_clk && m_FAdc_clk < 248)
        {    m_CSF_ROM_SEL  = 50;
            m_DCF_ROM_SEL  = 50;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 248 <= m_FAdc_clk && m_FAdc_clk < 250)
        {    m_CSF_ROM_SEL  = 51;
            m_DCF_ROM_SEL  = 51;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 250 <= m_FAdc_clk && m_FAdc_clk < 252)
        {    m_CSF_ROM_SEL  = 52;
            m_DCF_ROM_SEL  = 52;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else if( 252 <= m_FAdc_clk && m_FAdc_clk < 254)
        {    m_CSF_ROM_SEL  = 53;
            m_DCF_ROM_SEL  = 53;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001a;
            m_DCF_COEFF_2  = 0x0259;
            m_DCF_COEFF_3  = 0x00da;
            bResult        = TRUE;
        }
        else
        {    //DB5515_PRINTF("ROM Selection Errors");
            bResult = FALSE;
        }
    }
    else if(m_bwsel_6 == 2 ) //8MHz, m_4mhz_dig = 1 --> 2 for REVB
    {
        m_4mhz_dig = 3;
        //m_GMC_TUNE_COMP = 0x02c0;
        m_bblna_pole = 1;
        m_dcf_rom_byp = 0;

        if( 210 <= m_FAdc_clk && m_FAdc_clk < 212)
        {    m_CSF_ROM_SEL  = 64;
            m_DCF_ROM_SEL  = 64;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 212 <= m_FAdc_clk && m_FAdc_clk < 214)
        {    m_CSF_ROM_SEL  = 65;
            m_DCF_ROM_SEL  = 65;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 214 <= m_FAdc_clk && m_FAdc_clk < 216)
        {    m_CSF_ROM_SEL  = 66;
            m_DCF_ROM_SEL  = 66;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 216 <= m_FAdc_clk && m_FAdc_clk < 218)
        {    m_CSF_ROM_SEL  = 67;
            m_DCF_ROM_SEL  = 67;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 218 <= m_FAdc_clk && m_FAdc_clk < 220)
        {    m_CSF_ROM_SEL  = 68;
            m_DCF_ROM_SEL  = 68;
            m_DCF2_ROM_SEL = 0;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 220 <= m_FAdc_clk && m_FAdc_clk < 222)
        {    m_CSF_ROM_SEL  = 69;
            m_DCF_ROM_SEL  = 69;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 222 <= m_FAdc_clk && m_FAdc_clk < 224)
        {    m_CSF_ROM_SEL  = 70;
            m_DCF_ROM_SEL  = 70;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 224 <= m_FAdc_clk && m_FAdc_clk < 226)
        {    m_CSF_ROM_SEL  = 71;
            m_DCF_ROM_SEL  = 71;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0284;
            m_DCF_COEFF_3  = 0x00b1;
            bResult        = TRUE;
        }
        else if( 226 <= m_FAdc_clk && m_FAdc_clk < 228)
        {    m_CSF_ROM_SEL  = 72;
            m_DCF_ROM_SEL  = 72;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 228 <= m_FAdc_clk && m_FAdc_clk < 230)
        {    m_CSF_ROM_SEL  = 73;
            m_DCF_ROM_SEL  = 73;
            m_DCF2_ROM_SEL = 1;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 230 <= m_FAdc_clk && m_FAdc_clk < 232)
        {    m_CSF_ROM_SEL  = 74;
            m_DCF_ROM_SEL  = 74;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 232 <= m_FAdc_clk && m_FAdc_clk < 234)
        {    m_CSF_ROM_SEL  = 75;
            m_DCF_ROM_SEL  = 75;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 234 <= m_FAdc_clk && m_FAdc_clk < 236)
        {    m_CSF_ROM_SEL  = 76;
            m_DCF_ROM_SEL  = 76;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 236 <= m_FAdc_clk && m_FAdc_clk < 238)
        {    m_CSF_ROM_SEL  = 77;
            m_DCF_ROM_SEL  = 77;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 238 <= m_FAdc_clk && m_FAdc_clk < 240)
        {    m_CSF_ROM_SEL  = 78;
            m_DCF_ROM_SEL  = 78;
            m_DCF2_ROM_SEL = 2;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 240 <= m_FAdc_clk && m_FAdc_clk < 242)
        {    m_CSF_ROM_SEL  = 79;
            m_DCF_ROM_SEL  = 79;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 242 <= m_FAdc_clk && m_FAdc_clk < 244)
        {    m_CSF_ROM_SEL  = 80;
            m_DCF_ROM_SEL  = 80;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 244 <= m_FAdc_clk && m_FAdc_clk < 246)
        {    m_CSF_ROM_SEL  = 81;
            m_DCF_ROM_SEL  = 81;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x027c;
            m_DCF_COEFF_3  = 0x00b9;
            bResult        = TRUE;
        }
        else if( 246 <= m_FAdc_clk && m_FAdc_clk < 248)
        {    m_CSF_ROM_SEL  = 82;
            m_DCF_ROM_SEL  = 82;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0273;
            m_DCF_COEFF_3  = 0x00c2;
            bResult        = TRUE;
        }
        else if( 248 <= m_FAdc_clk && m_FAdc_clk < 250)
        {    m_CSF_ROM_SEL  = 83;
            m_DCF_ROM_SEL  = 83;
            m_DCF2_ROM_SEL = 3;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0273;
            m_DCF_COEFF_3  = 0x00c2;
            bResult        = TRUE;
        }
        else if( 250 <= m_FAdc_clk && m_FAdc_clk < 252)
        {    m_CSF_ROM_SEL  = 84;
            m_DCF_ROM_SEL  = 84;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0273;
            m_DCF_COEFF_3  = 0x00c2;
            bResult        = TRUE;
        }
        else if( 252 <= m_FAdc_clk && m_FAdc_clk < 254)
        {    m_CSF_ROM_SEL  = 85;
            m_DCF_ROM_SEL  = 85;
            m_DCF2_ROM_SEL = 4;
            m_DCF_COEFF_0  = 0x000d;
            m_DCF_COEFF_1  = 0x001b;
            m_DCF_COEFF_2  = 0x0273;
            m_DCF_COEFF_3  = 0x00c2;
            bResult        = TRUE;
        }
        else
        {    //DB5515_PRINTF("ROM Selection Errors");
            bResult = FALSE;
        }
    }
    else   // 6,7,8MHz No Selection
    {
        //DB5515_PRINTF("ROM Selection Errors");
            bResult = FALSE;
    }



    if (bResult == TRUE)
    {
        //DB5515_PRINTF("ROM Selection OK");
        //DB5515_PRINTF("(succ) VCO=%.10f, VCO Select = 0x%04x", m_fData_VCO, m_VCO_select);
        //DB5515_PRINTF("(succ) RF_IN : %.10f, LODIV=%.5f", m_fData_RF, m_fData_LODIV);
        //DB5515_PRINTF("ADC Clock = %.5f", m_FAdc_clk);
        //DB5515_PRINTF("ROM Selection = %d, DCF2 ROM Selection = %d", m_CSF_ROM_SEL, m_DCF2_ROM_SEL);
    }
    else
    {
        //        DB5515_PRINTF("ROM Selection Errors");
        //DB5515_PRINTF("(fail) VCO=%.5f", m_fData_VCO);
    }



    return bResult;
}

//-------------------------------------------------------------------------------------------
//  description : calculate FractionalMode PLL function due to frequency setting
//  variable : m_fData_CLK,
//  description of variable :
//   - m_fData_CLK = X-tal frequency
//
//  return : TRUE or FALSE
// =============================== for example ==================================================
//  **** for example, m_fData_CLK = 27MHz,
//  **** => m_fData_CLK = 27,
//------------------------------------------------------------------------------------------------
BOOL CalculateFractionalMode(float m_fData_CLK)
{

 BOOL bResult = FALSE;

    //float m_fData_CLK = 27; //dwkim 100824.

    if (m_fData_VCO>0)
    {
        float fDataDiv = (float)((m_fData_VCO/m_fData_CLK)); //For DB5510
        u8SD_DIVWORD = (int)fDataDiv;




//        u23SD_FIN_A = (float)((fDataDiv - u8SD_DIVWORD)*16777216); //For DB5510 FCW0
        u23SD_FIN = (float)((fDataDiv - u8SD_DIVWORD)*524288); // For DB5510 Manual 2^19

        bResult = TRUE;
    }

    return bResult;
}

//-------------------------------------------------------------------------------------------
//  description : calculate Internal tarcking filter due to frequency setting
//  variable : NO,
//
//  return : TRUE or FALSE
// =============================== for example ==================================================
//  ****
//  ****
//------------------------------------------------------------------------------------------------
BOOL CalculateTrackflt(void)
{
    //CString strTemp;

    BOOL bResult = FALSE;

    if(m_fData_RF < 50)
    {m_lb_trackflt_msb = 255;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 255;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (50 <= m_fData_RF) && (m_fData_RF <60))
    {m_lb_trackflt_msb = 255;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 191;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (60 <= m_fData_RF) && (m_fData_RF <70))
    {m_lb_trackflt_msb = 191;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 127;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (70 <= m_fData_RF) && (m_fData_RF <80))
    {m_lb_trackflt_msb = 127;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 127;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (80 <= m_fData_RF) && (m_fData_RF <90))
    {m_lb_trackflt_msb = 127;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 127;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (90 <= m_fData_RF) && (m_fData_RF <100))
    {m_lb_trackflt_msb = 96;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 63;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (100 <= m_fData_RF) && (m_fData_RF <110))
    {m_lb_trackflt_msb = 96;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 63;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (110 <= m_fData_RF) && (m_fData_RF <120))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 63;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (120 <= m_fData_RF) && (m_fData_RF <130))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 47;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (130 <= m_fData_RF) && (m_fData_RF <140))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 47;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (140 <= m_fData_RF) && (m_fData_RF <150))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 31;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (150 <= m_fData_RF) && (m_fData_RF <160))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 19;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (160 <= m_fData_RF) && (m_fData_RF <170))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 15;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (170 <= m_fData_RF) && (m_fData_RF <180))
    {m_lb_trackflt_msb = 63;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 10;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (180 <= m_fData_RF) && (m_fData_RF <190))
    {m_lb_trackflt_msb = 47;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 10;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (190 <= m_fData_RF) && (m_fData_RF <200))
    {m_lb_trackflt_msb = 47;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 7;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (200 <= m_fData_RF) && (m_fData_RF <210))
    {m_lb_trackflt_msb = 35;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 7;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (210 <= m_fData_RF) && (m_fData_RF <220))
    {m_lb_trackflt_msb = 31;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 7;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (220 <= m_fData_RF) && (m_fData_RF <230))
    {m_lb_trackflt_msb = 39;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (230 <= m_fData_RF) && (m_fData_RF <240))
    {m_lb_trackflt_msb = 29;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (240 <= m_fData_RF) && (m_fData_RF <250))
    {m_lb_trackflt_msb = 23;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (250 <= m_fData_RF) && (m_fData_RF <260))
    {m_lb_trackflt_msb = 15;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (260 <= m_fData_RF) && (m_fData_RF <270))
    {m_lb_trackflt_msb = 7;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (270 <= m_fData_RF) && (m_fData_RF <280))
    {m_lb_trackflt_msb = 7;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (280 <= m_fData_RF) && (m_fData_RF <290))
    {m_lb_trackflt_msb = 3;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (290 <= m_fData_RF) && (m_fData_RF <300))
    {m_lb_trackflt_msb = 1;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (300 <= m_fData_RF) && (m_fData_RF <310))
    {m_lb_trackflt_msb = 0;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (310 <= m_fData_RF) && (m_fData_RF <320))
    {m_lb_trackflt_msb = 0;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if( (320 <= m_fData_RF) && (m_fData_RF <390))
    {m_lb_trackflt_msb = 0;        m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0;        m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else if(390 <= m_fData_RF)
    {m_lb_trackflt_msb = 0xFF;    m_lb_trackflt_lsb = 0;    m_mb_trackflt_msb = 0xFF;    m_mb_trackflt_lsb = 0;    bResult = TRUE;}
    else
    {DB5515_PRINTF("Channel frequency is wrong!!");}

    return     bResult;
}


//-------------------------------------------------------------------------------------------
//  description : set every registers
//  variable : NO,
//
//  return : TRUE or FALSE
// =============================== for example ==================================================
//  ****
//  ****
//------------------------------------------------------------------------------------------------
void SetRegChannel(UINT32 tuner_id)
{
//CString strTemp;

    int    temp1;



    //int    D_SYS_CTL              = 0x0038; // Turn on xtal_enable, clk_enable, i2c_reg_clk_en

    m_spectrum = TRUE;
    int D_CHANNEL_INFO         = (m_spectrum * 4) + (m_bwsel_6);  // Set mem_iq_swap and mem_bw_sel values

    int m_fData_RF_Hz = (int) (m_fData_RF*1000000);
    int m_fData_IF_Hz = (int) (m_fData_IF*1000000);

    int D_FCH_FCW_LSB          = (m_fData_RF_Hz & 0x0000FFFF);
    int D_FCH_FCW_MSB          = ((m_fData_RF_Hz & 0xFFFF0000)/(65536));
    int D_FIF_FCW_LSB          = (m_fData_IF_Hz & 0x000000FF);
    int D_FIF_FCW_MSB          = ((m_fData_IF_Hz & 0xFFFF00)/(256));

    int D_CTRL_PLL4            = m_ctrl_pll4;
    //int D_CTRL_PLL2            = m_ctrl_pll2;

    int D_CTRL_SD1             = u23SD_FIN & 0xFFFF;
    int D_CTRL_SD2             = ((u8SD_DIVWORD & 0x00FF) * 8) + ((u23SD_FIN >> 16) & 0x0007);  //FIN msb and divword only
    int D_CTRL_SD2_B           = m_sd_sdorder;

    int D_CTRL_TRACKFLT1       = m_TRACKFLT_select;
    int D_CTRL_TRACKFLT2       = (m_lb_trackflt_msb * 2) + m_lb_trackflt_lsb;
    int D_CTRL_TRACKFLT3       = (m_mb_trackflt_msb * 2) + m_mb_trackflt_lsb;
    int D_CTRL_HPF_DIG         = m_HPF_select;
    int D_CTRL_MIXERS1         = m_mix_cap_ctrl*256 + m_Mixer_select;  //Not done by SETUP_CHANNEL
    int    D_CTRL_LNA1            = m_LNA1_select;
    int D_CTRL_IQBB2           = m_4mhz_dig;
    int D_COEF_ROM_SEL         = (m_DCF_ROM_SEL * 512) + (m_CSF_ROM_SEL * 4) + (m_dcf_rom_byp * 2);
    int D_CTRL_CLK_GEN_preinit = (17*128  + m_diven + 0) | 0x0004;                 // ADC clock and mix divider configuration
    int D_CTRL_CLK_GEN_initial = ((m_FADC_Div-4) * 4096) + 17*128  + m_diven + 0;  // Set ADC / LO divider. But, no ADC clock
    int D_CTRL_CLK_GEN_final   = ((m_FADC_Div-4) * 4096) + 17*128  + m_diven + 1;  // Enable ADC clock

    // Temp Required by ALAN based on Lab Review
    int D_GMC_TUNE_COMP        = 0x0540;  // Set GMC_TUNE_COMP word (based on lab review) ALAN Req
    int D_PW_DOC_CW1           = 0x0002;  // Turn-off fb_en (based on lab review) ALAN Req
    int D_CTRL_IQBB8           = 0x06F1;  // Set BBPD pole and gain (based on lab review) ALAN Req
    int D_CTRL_IQBB1           = 0x3AE0;  // Set bblna pole (based on lab review) ALAN Req
    int    D_CTRL_LNA1_def        = 0x2AF9;  // Set mem_lna_vb to 0101, set mem_lna_ib to 11110 (based on lab review) ALAN Req
    int    D_CTRL_LNA2            = 0x0030 + m_lna_rfbsel;  // Set mem_lna_rfbsel to 1101/1000 for Terr/Cable resp. (based on lab review) ALAN Req



    /* COMMENT BY CESAR
    int D_FCW_MODE         = 0x0000; //
    int D_DIG_LOOP_CW      = 0x007f; // don't use!  Bypass handled by gui check-boxes
    int D_CTRL_LNA2 = 0x0ff8; //
    int D_CTRL_LNA3 = 0x00ff; //
    int    D_CTRL_PD1 = 0x20a7;  // For Sharp INI 20100630 addr = 0x0084
    int    D_CTRL_PD2 = 0x000f;  //
    int D_CTRL_IQBB1 = 0x02fe + m_bblna_pole * 2048; //
    int D_CTRL_IQBB2 = 0x2080 + m_4mhz_dig;          //
    int D_CTRL_IQBB3 = 0x207f;                       //
    int D_CTRL_IQBB4 = 0x001b;                       // 0x000b -> 0x001b(Bypass DEM) 20100704
    int D_CTRL_IQBB5 = 0xffff;                       //
    int D_CTRL_IQBB6 = 0xffff;                       //
    int D_CTRL_IQBB7 = 0x0fff;                       //
    int D_CTRL_OUTDAC = 0x8276 + m_DACoutput;        // DAC Clock Enable but[15] for REVB
    int    D_CTRL_PLL1 = 0x040F + (64*m_VCO_select);    // REVB  // ver2.4 : changed from 0x1027 to 0x040F
    int D_CTRL_PLL3 = 0x33f1;
    int D_IFAGC_REGX  = 0x0200;  // addr = 0x020F
    int D_IFAGC_REG11 = 0x0341; // addr = 0x020C
    int D_IFAGC_REG13 = 0x0080; // addr = 0x02e
    COMMENT BY CESAR */

// ==============================================================================================================
// ========== START-UP Register Write Sequence ==================================================================

    DB5515_PRINTF("ADC clock rate  = %3.3f MHz\n", m_fADC_rate);

    //Write(tuner_id,A_SYS_CTL, D_SYS_CTL);            DB5515_PRINTF("SYS_CTL        = 0x%04x", D_SYS_CTL);  // Turn on xtal_enable, clk_enable, i2c_reg_clk_en
    temp1 = 0x0028;
    Write(tuner_id,A_SYS_CTL, temp1);               // Turn on XTAL clock only ==> reset synchronously CKM in digital
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_SYS_CTL, temp1);

     temp1 = Read(tuner_id,A_CTRL_LOOP_THRU1) & 0xFFFE;
     Write(tuner_id,A_CTRL_LOOP_THRU1, temp1);
     DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LOOP_THRU1, temp1);  // Clear CTRL_LOOP_THRU1[0] = mem_en_lt

     temp1 = Read(tuner_id,A_CTRL_LNA2) & 0xFEFF;
     Write(tuner_id,A_CTRL_LNA2, temp1);
     DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LNA2, temp1);  // Clear CTRL_LNA2[8] = mem_lna_slvmode

    temp1 = Read(tuner_id,A_OP_MODE) & 0xFFEF;
     Write(tuner_id,A_OP_MODE, temp1);
     DB5515_PRINTF("0x%04x = 0x%04x\n", A_OP_MODE, temp1);  // Clear OP_MODE[4] = mem_lt_only_mode

    DB5515_PRINTF("set mem_en_lt       = %d", m_en_lt);
    DB5515_PRINTF("set mem_lna_slvmode = %d", m_lna_slvmode);
    DB5515_PRINTF("set mem_lt_only     = %d", m_lt_only_mode);

    /* if (m_lna_slvmode == TRUE)
    DB5515_PRINTF("Configured as Slave Tuner Mode");
    else if (m_lt_only_mode == TRUE)
    DB5515_PRINTF("Configured as Loop-Thru Only Mode");
    else if (m_en_lt == TRUE)
    DB5515_PRINTF("Configured as Loop-Thru Mode (Both)");
    else
    DB5515_PRINTF("Configured as Normal Mode (No Loop-Thru)"); */

    temp1 = (Read(tuner_id,A_CTRL_LOOP_THRU3) & 0x00FF) | 0x7F00;
    Write(tuner_id,A_CTRL_LOOP_THRU3, temp1);                    // Set mem_gctrl_fe_amp= [14:8]= 1111111
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LOOP_THRU3, temp1);

    temp1 = (Read(tuner_id,A_CTRL_LOOP_THRU1) & 0x01FF) | 0xC000;
    Write(tuner_id,A_CTRL_LOOP_THRU1, temp1);                    // Set mem_ib_lt= [15:9]= 0110000
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LOOP_THRU1, temp1);

    // Go To Default (ONLY NEED TO CLEAR TRACKING_FILTER)
    //Write(tuner_id,A_EXTIFAGC_REG1,  (Read(tuner_id,A_EXTIFAGC_REG1)  & 0xFFEF)); // Clear EXTIFAGC_REG1[4]   = mem_extifagc_sm_en
    //Write(tuner_id,A_CTRL_OUTDAC,    (Read(tuner_id,A_CTRL_OUTDAC)    & 0xFFFE)); // Clear CTRL_OUTDAC[0]     = mem_dac_en_int
    //Write(tuner_id,A_SYS_CTL,        (Read(tuner_id,A_SYS_CTL)        & 0xFFEF)); // Clear SYS_CTL[4]         = mem_clk_enable
    //Write(tuner_id,A_CTRL_CLK_GEN,   ((Read(tuner_id,A_CTRL_CLK_GEN)   & 0x8F80) | 0x4000)); // Clear CTRL_CLK_GEN[6:0] = mem_diven's, Set [14:12] = mem_adc_div = 3'd4
    //Write(tuner_id,A_COEF_ROM_SEL,   (Read(tuner_id,A_COEF_ROM_SEL)   & 0x0003)); // Clear COEF_ROM_SEL[15:9, 8:2] = mem_dcf/csf_coeff_sel
    //Write(tuner_id,A_CTRL_IQBB2,     (Read(tuner_id,A_CTRL_IQBB2)     & 0xFFFC)); // Clear CTRL_IQBB2[1:0]   = mem_ana_bw_mode
    //Write(tuner_id,A_CTRL_MPADC,     (Read(tuner_id,A_CTRL_MPADC)     & 0xFFFE)); // Clear CTRL_MPADC[0]      = mem_adc_agc_en
    //Write(tuner_id,A_CTRL_IQBB8,     (Read(tuner_id,A_CTRL_IQBB8)     & 0xEFFF)); // Clear CTRL_IQBB8[12]     = mem_en_bbpd
    //Write(tuner_id,A_CTRL_IQBB1,     (Read(tuner_id,A_CTRL_IQBB1)     & 0xFFE5)); // Clear CTRL_IQBB1[4:3, 1] = mem_en_adc/flt/bblna
    //Write(tuner_id,A_CTRL_MIXERS1,   (Read(tuner_id,A_CTRL_MIXERS1)   & 0xFFF0)); // Clear CTRL_MIXERS1[3:0]  = mem_en_lb/mb/hb_mix, mem_en_vcm
    //Write(tuner_id,A_CTRL_HPF_DIG,   (Read(tuner_id,A_CTRL_HPF_DIG)   & 0xFFFE)); // Clear CTRL_HPF_DIG[0]    = mem_hpf_en

    temp1 = (Read(tuner_id,A_CTRL_TRACKFLT3) & 0xFE00) | 0x01FE;
    Write(tuner_id,A_CTRL_TRACKFLT3, temp1);                                                // Clear and set [8:1]=mem_mb_trackflt_msb=8'hFF
                                                                                           //                 [0]=mem_mb_trackflt_lsb=1'b0
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT3, temp1);

    temp1 = (Read(tuner_id,A_CTRL_TRACKFLT2) & 0xFE00) | 0x01FE;
    Write(tuner_id,A_CTRL_TRACKFLT2, temp1);                                                // Clear and set [8:1]=mem_lb_trackflt_msb=8'hFF
                                                                                           //                 [0]=mem_lb_trackflt_lsb=1'b0
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT2, temp1);

    temp1 = Read(tuner_id,A_CTRL_TRACKFLT1) & 0xFFFE;
    Write(tuner_id,A_CTRL_TRACKFLT1, temp1);                                                // Clear CTRL_TRACKFLT1[0]  = mem_trackflt_en_lb
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT1, temp1);

    //Write(tuner_id,A_GMC_TUNE_CW,    (Read(tuner_id,A_GMC_TUNE_CW)    & 0xFFFE)); // Clear GMC_TUNE_CW[0]     = mem_gmc_tune_en
    //Write(tuner_id,A_CTRL_SD2,       ((Read(tuner_id,A_CTRL_SD2)       & 0x2000) | 0xD800)); // Clear and set [15:14]=mem_sd_sdorder=3
                                                                                           //                  [12]=mem_mimctrl_resetn=1
                                                                                           //                  [11]=mem_sd_resetn=1
                                                                                           //                [10:3]=mem_sd_divword=0
                                                                                           //                 [2:0]=mem_sd_fin_msb=0
    //Write(tuner_id,A_CTRL_SD1,       (Read(tuner_id,A_CTRL_SD1)       & 0x0000)); // Clear CTRL_SD1[15:0]   = mem_sd_fin_lsb
    //Write(tuner_id,A_CRDC_CW_0,      (Read(tuner_id,A_CRDC_CW_0)      & 0x0000)); // Clear CRDC_CW_0[15:0]  = mem_cordic_if_phase_lsb
    //Write(tuner_id,A_CRDC_CW_1,      (Read(tuner_id,A_CRDC_CW_1)      & 0xFFF8)); // Clear CRDC_CW_1[2:0]   = mem_cordic_if_phase_msb
    //Write(tuner_id,A_CTRL_PLL1,      (Read(tuner_id,A_CTRL_PLL1)      & 0xFF6F)); // Clear CTRL_PLL1[7,4]   = mem_en_mimadc, mem_en_div45

    // Set FCW
    Write(tuner_id,A_FIF_FCW_LSB, D_FIF_FCW_LSB);  // Write FIF_FCW
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FIF_FCW_LSB, D_FIF_FCW_LSB);
    Write(tuner_id,A_FIF_FCW_MSB, D_FIF_FCW_MSB);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FIF_FCW_MSB, D_FIF_FCW_MSB);
    Write(tuner_id,A_FCH_FCW_LSB, D_FCH_FCW_LSB);  // Write FCH_FCW
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FCH_FCW_LSB, D_FCH_FCW_LSB);
    Write(tuner_id,A_FCH_FCW_MSB, D_FCH_FCW_MSB);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FCH_FCW_MSB, D_FCH_FCW_MSB);

    // Start Long Sequence
    // Leave agc_freeze value as previously set
    temp1 = 0x0029;
    Write(tuner_id,A_SYS_CTL, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_SYS_CTL, temp1);               // Toggle SYS_CTL[0]=mem_sw_reset

    temp1 = 0x0028;
    Write(tuner_id,A_SYS_CTL, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_SYS_CTL, temp1);

    Write(tuner_id,A_CHANNEL_INFO, D_CHANNEL_INFO);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CHANNEL_INFO, D_CHANNEL_INFO); // Set mem_iq_swap and mem_bw_sel

    //TEMPORARY
    temp1 = 0x0682;
    Write(tuner_id,A_ADC_OL_CTL, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_ADC_OL_CTL, temp1);        // Bypass ADC Over-Load

    temp1 = 0x09ED;
    Write(tuner_id,A_DIG_LOOP_CW, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DIG_LOOP_CW, temp1);       // Bypass all cal loops but gmc tune loop is active

    temp1 = 0x0502;
    Write(tuner_id,A_CTRL_TCXO1, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TCXO1, temp1);        // TCXO setup, enable pll ref clock, max current now (default)

    temp1 = 0x0001;
    Write(tuner_id,A_FCW_MODE, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FCW_MODE, temp1);          // Trigger setup_channel/channel_change

    temp1 = 0x0000;
    Write(tuner_id,A_FCW_MODE, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_FCW_MODE, temp1);          // Set and clear FCW_MODE[0] = mem_channel_change

    Write(tuner_id,A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_preinit);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_preinit);    // ADC clock and mix divider configuration

    Write(tuner_id,A_GMC_TUNE_COMP, D_GMC_TUNE_COMP);        // Set GMC_TUNE_COMP word (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_GMC_TUNE_COMP, D_GMC_TUNE_COMP);

    Write(tuner_id,A_PW_DOC_CW1, D_PW_DOC_CW1);              // Turn-off fb_en (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_PW_DOC_CW1, D_PW_DOC_CW1);

    Write(tuner_id,A_CTRL_IQBB8, D_CTRL_IQBB8);              // Set BBPD pole and gain (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB8, D_CTRL_IQBB8);

    Write(tuner_id,A_CTRL_IQBB1, D_CTRL_IQBB1);              // Set bblna pole (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB1, D_CTRL_IQBB1);

    Write(tuner_id,A_CTRL_LNA1, D_CTRL_LNA1_def);            // Set mem_lna_vb to 0101, set mem_lna_ib to 11110 (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LNA1, D_CTRL_LNA1_def);

    temp1 = ((Read(tuner_id,A_CTRL_LNA2) & 0xFF00) | D_CTRL_LNA2);
    Write(tuner_id,A_CTRL_LNA2, temp1);                      // Set mem_lna_rfbsel to 1101 (based on lab review) ALAN Req
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LNA2, temp1);

    temp1 = 0x6500;
    Write(tuner_id,A_CTRL_PLL6, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL6, temp1);          // Change the default value from 0x6531 to 0x6500

    // -----------------------------------------------------
    // Set RFPD/BBPD Threshold setting
    Write(tuner_id,A_AGC_RFPD_THRESH_CTRL, m_AGC_RFPD_TH);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_RFPD_THRESH_CTRL, m_AGC_RFPD_TH);  // Updated! (NOT DONE by the SETUP_CHANNEL)

    temp1 = 0x0100 | m_bbpd_tune_thres;
    Write(tuner_id,A_AGC_BBPD_THRESH_CTRL, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_BBPD_THRESH_CTRL, temp1);          // Updated! (default) constant...(NOT DONE by the SETUP_CHANNEL)

    temp1 = 0x02F7;
    Write(tuner_id,A_AGC_CSFPD_THRESH_CTRL1, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_CSFPD_THRESH_CTRL1, temp1);        // Updated! (default) constant...(NOT DONE by the SETUP_CHANNEL)

    temp1 = 0x0033;
    Write(tuner_id,A_AGC_CSFPD_THRESH_CTRL2, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_CSFPD_THRESH_CTRL2, temp1);        // Updated! (default) constant...(NOT DONE by the SETUP_CHANNEL)

    temp1 = 0x01EA;
    Write(tuner_id,A_AGC_DLY2, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_DLY2, temp1);                      // Updated! (default) constant...(NOT DONE by the SETUP_CHANNEL)

    temp1 = 0x00F5;
    Write(tuner_id,A_AGC_DOC_DAC_LMT, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_DOC_DAC_LMT, temp1);               // Updated! (default) constant...(NOT DONE by the SETUP_CHANNEL)
    // -----------------------------------------------------

    // ***ST_1***
    //Write(tuner_id,A_CTRL_PLL4,      ((Read(tuner_id,A_CTRL_PLL4)      & 0x83FF) | D_CTRL_PLL4)); // Write CTRL_PLL4[14:10] = value from PLL_bands table
    temp1 = ((Read(tuner_id,A_CTRL_PLL4) & 0x83FF) | D_CTRL_PLL4);
    Write(tuner_id,A_CTRL_PLL4, temp1);               // Write CTRL_PLL4[14:10] = value from PLL_bands table [vco_byp always 1]
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL4, temp1);

    // ***ST_2, SUBST2***
    temp1 = (Read(tuner_id,A_CTRL_PLL1) & 0xFFF0) | 0x000F;
    Write(tuner_id,A_CTRL_PLL1, temp1);               // Write CTRL_PLL1  [3:0] = 4'hF
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL1, temp1);

    // ***ST_2, SUBST3***
    temp1 = (Read(tuner_id,A_CTRL_PLL1) & 0xFF9F) | 0x0060;
    Write(tuner_id,A_CTRL_PLL1, temp1);               // Write CTRL_PLL1  [6:5] = 2'b11
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL1, temp1);

    temp1 = (Read(tuner_id,A_CTRL_PLL2) & 0xFFFE) | 0x0000;
    Write(tuner_id,A_CTRL_PLL2, temp1);               // Write CTRL_PLL2    [0] = 1'b0
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL2, temp1);

    // ***ST_2, SUBST4***
    //Write(tuner_id,A_CTRL_PLL2,      ((Read(tuner_id,A_CTRL_PLL2)      & 0xFFC7) | D_CTRL_PLL2)); // Write CTRL_PLL2  [5:3] = value from PLL_bands table (bug fix)
    temp1 = 0x0426;
    Write(tuner_id,A_CTRL_PLL2, temp1);                // CP setup
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL2, temp1);   // Print final PLL2 value

    temp1 = 0x01DB;
    Write(tuner_id,A_CTRL_PLL3, temp1);                // Reference voltage setup
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL3, temp1);   // Print final PLL3 value

    temp1 = 0xE808;
    Write(tuner_id,A_CTRL_PLL5, temp1);                // Set mim cap search short timer, SD dither mode and strength
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL5, temp1);   // Print final PLL5 value

    temp1 = (Read(tuner_id,A_CTRL_IQBB1) & 0xFFFB) | 0x0004;
    Write(tuner_id,A_CTRL_IQBB1, temp1);               // Write CTRL_IQBB1   [2] = mem_en_vc = 1'b1
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB1, temp1);

    // ***ST_RFPDCAL, SUBST1***
    temp1 = (Read(tuner_id,A_CTRL_PD1) & 0xFFFE) | 0x0001;
    Write(tuner_id,A_CTRL_PD1, temp1);                 // Write CTRL_PD1     [0] = mem_det_en = 1'b1
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PD1, temp1);

    // ***ST_RFPDCAL, SUBST2*** (check if need this)
    //Write(tuner_id,A_RFPD_CAL_CW,    ((Read(tuner_id,A_RFPD_CAL_CW)    & 0xFFFE) | 0x0001));      // Write RFPD_CAL_CW  [0] = mem_rfpd_cal_start = 1'b1
    temp1 = 0x0084;
    Write(tuner_id,A_RFPD_CAL_CW, temp1);              // Temp
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_RFPD_CAL_CW, temp1);

    temp1 = 0x0030;
    Write(tuner_id,A_AGC_CTL1, temp1);                 // Temp
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_CTL1, temp1);

    Write(tuner_id,A_AGC_CTL2, m_rf_lowg_bdr);         // Temp
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_CTL2, m_rf_lowg_bdr);

    temp1 = 0x0235;
    Write(tuner_id,A_AGC_CTL3, temp1);                 // Temp
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_AGC_CTL3, temp1);

    // ***ST_3, SUBST1***
    //temp1 = ((Read(tuner_id,A_CTRL_PLL1)      & 0xFF6F) | 0x0090);
    temp1 = ((Read(tuner_id,A_CTRL_PLL1) & 0x0C6F) | 0x8790);
    Write(tuner_id,A_CTRL_PLL1, temp1);                // Write CTRL_PLL1  [7,4] = 1'b1, 1'b1; [9:8]=11, [15:12]=1000
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_PLL1, temp1);   // Print final PLL1 value

    // ***ST_3, SUBST2***
    Write(tuner_id,A_CRDC_CW_0, crdc0);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CRDC_CW_0, crdc0);

    Write(tuner_id,A_CRDC_CW_1, crdc1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CRDC_CW_1, crdc1);

    Write(tuner_id,A_CTRL_SD1, D_CTRL_SD1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_SD1, D_CTRL_SD1);  // Write mem_sd_fin_lsb

    temp1 = ((Read(tuner_id,A_CTRL_SD2) & 0xF800) | D_CTRL_SD2);
    Write(tuner_id,A_CTRL_SD2, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_SD2, temp1);       // Write mem_sd_fin_msb and mem_sd_divword

    // ***ST_3, SUBST3*** (Can I combine these?)
    //Write(tuner_id,A_CTRL_SD2,       ((Read(tuner_id,A_CTRL_SD2)       & 0x3FFF) | D_CTRL_SD2_B));     // Set mem_sd_sdorder = 2'b11 (default) from table value
    temp1 = ((Read(tuner_id,A_CTRL_SD2) & 0x3FFF) | D_CTRL_SD2_B) & 0xCFFF;
    Write(tuner_id,A_CTRL_SD2, temp1);                    // Set mem_sd_sdorder = 2'b11 (default) from table value and disable sd_dither_en
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_SD2, temp1);

    temp1 = (Read(tuner_id,A_CTRL_SD2) & 0xE7FF) | 0x0000;
    Write(tuner_id,A_CTRL_SD2, temp1);                    // Reset mem_sd_resetn and mem_mimctrl_resetn
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_SD2, temp1);

    temp1 = (Read(tuner_id,A_CTRL_SD2) & 0xE7FF) | 0x1800;
    Write(tuner_id,A_CTRL_SD2, temp1);                    // Toggle CTRL_SD2[12:11]
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_SD2, temp1);

    // ***ST_3, SUBST6***
    temp1 = (Read(tuner_id,A_GMC_TUNE_CW) & 0xFFFE) | 0x0001;
    Write(tuner_id,A_GMC_TUNE_CW, temp1);                 // Write GMC_TUNE_CW  [0] = mem_gmc_tune_en = 1'b1
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_GMC_TUNE_CW, temp1);

    // ***ST_3, SUBST7***
    temp1 = ((Read(tuner_id,A_CTRL_TRACKFLT1) & 0xFFFE) | D_CTRL_TRACKFLT1);
    Write(tuner_id,A_CTRL_TRACKFLT1, temp1);              // Set CTRL_TRACKFLT1[0]  = mem_trackflt_en_lb
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT1, temp1);

    Write(tuner_id,A_CTRL_TRACKFLT2, D_CTRL_TRACKFLT2);   // Write CTRL_TRACKFLT2[8:0] = value from table
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT2, D_CTRL_TRACKFLT2);

    Write(tuner_id,A_CTRL_TRACKFLT3, D_CTRL_TRACKFLT3);   // Write CTRL_TRACKFLT3[8:0] = value from table
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_TRACKFLT3, D_CTRL_TRACKFLT3);

    temp1 = (Read(tuner_id,A_CTRL_HPF_DIG) & 0xFFFE) | D_CTRL_HPF_DIG;
    Write(tuner_id,A_CTRL_HPF_DIG, temp1);                // Set CTRL_HPF_DIG[0]    = mem_hpf_en
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_HPF_DIG, temp1);

    temp1 = (Read(tuner_id,A_CTRL_MIXERS1) & 0x0018) | D_CTRL_MIXERS1;
    Write(tuner_id,A_CTRL_MIXERS1, temp1);                // Set CTRL_MIXERS1[2:0]  // mix_cap_ctrl not set by setup channel
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_MIXERS1, temp1);

    // ***ST_3, SUBST8***
    temp1 = (Read(tuner_id,A_CTRL_MIXERS1) & 0xFF07) | 0x0018;
    Write(tuner_id,A_CTRL_MIXERS1, temp1);                // Set CTRL_MIXERS1  [3] = mem_en_vcm
                                                 // Set CTRL_MIXERS1 [7:4] - 0001
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_MIXERS1, temp1);

    // ***ST_LPPATH, SUBST1***
    temp1 = (Read(tuner_id,A_CTRL_LNA1) & 0xFFFC) | D_CTRL_LNA1;
    Write(tuner_id,A_CTRL_LNA1, temp1);                   // Set CTRL_LNA1[1:0]
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_LNA1, temp1);

    // ***ST_4***
    temp1 = (Read(tuner_id,A_CTRL_IQBB1) & 0xFFE5) | 0x001A;
    Write(tuner_id,A_CTRL_IQBB1, temp1);                  // Set CTRL_IQBB1[4,3,1] = mem_en_adc, mem_en_flt, mem_en_bblna
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB1, temp1);

    temp1 = (Read(tuner_id,A_CTRL_IQBB8) & 0xEFFF) | 0x1000;
    Write(tuner_id,A_CTRL_IQBB8, temp1);                  // Set CTRL_IQBB8   [12] = mem_en_bbpd
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB8, temp1);

    temp1 = (Read(tuner_id,A_CTRL_MPADC) & 0xFFFE) | 0x0001;
    Write(tuner_id,A_CTRL_MPADC, temp1);                  // Set CTRL_MPADC    [0] = mem_adc_agc_en
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_MPADC, temp1);

    // ***ST_5, SUBST1***
    temp1 = (Read(tuner_id,A_CTRL_IQBB2) & 0xFFFC) | D_CTRL_IQBB2;
    Write(tuner_id,A_CTRL_IQBB2, temp1);                  // Set CTRL_IQBB2  [1:0] = bwmode
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_IQBB2, temp1);

    // ***ST_5, SUBST3***
    temp1 = (Read(tuner_id,A_COEF_ROM_SEL) & 0x0001) | D_COEF_ROM_SEL;
    Write(tuner_id,A_COEF_ROM_SEL, temp1);                // Set COEFF_ROM_SEL[15:2] = CSF/DCF ROM Select, bypass dcf ROM for 6/7 bands.
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_COEF_ROM_SEL, temp1);

    Write(tuner_id,A_DCF_COEFF_0, m_DCF_COEFF_0);  // Set DCF_COEFF manually...
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DCF_COEFF_0, m_DCF_COEFF_0);

    Write(tuner_id,A_DCF_COEFF_1, m_DCF_COEFF_1);  // Set DCF_COEFF manually...
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DCF_COEFF_1, m_DCF_COEFF_1);

    Write(tuner_id,A_DCF_COEFF_2, m_DCF_COEFF_2);  // Set DCF_COEFF manually...
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DCF_COEFF_2, m_DCF_COEFF_2);

    Write(tuner_id,A_DCF_COEFF_3, m_DCF_COEFF_3);  // Set DCF_COEFF manually...
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DCF_COEFF_3, m_DCF_COEFF_3);

    // ***ST_5, SUBST4***
    Write(tuner_id,A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_initial);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_initial);    // Set ADC / LO divider

    // ***ST_5, SUBST5***
    Write(tuner_id,A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_final);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_CLK_GEN, D_CTRL_CLK_GEN_final);   // enable ADC

    // ***ST_5, SUBST6***
    temp1 = ((Read(tuner_id,A_SYS_CTL) & 0xFFEF) | 0x0010);
    Write(tuner_id,A_SYS_CTL, temp1);              // Set SYS_CTL[4] = mem_clk_en
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_SYS_CTL, temp1);

    // S***T_5, SUBST7***
    //Write(tuner_id,A_CTRL_OUTDAC, ((Read(tuner_id,A_CTRL_OUTDAC) & 0xFFFD) | 0x0002));                 // Set CTRL_OUTDAC[1]  = mem_dac_en
    temp1 = 0x009E;
    Write(tuner_id,A_CTRL_OUTDAC, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_CTRL_OUTDAC, temp1);      // Set CTRL_OUTDAC[1]  = mem_dac_en

    temp1 = (Read(tuner_id,A_EXTIFAGC_REG1)& 0xFFEF) | 0x0010;
    Write(tuner_id,A_EXTIFAGC_REG1, temp1);                 // Set EXTIFAGC_REG1[4] = mem_extifagc_sm_en
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_EXTIFAGC_REG1, temp1);

    // ***ST_AGCTRIG***
    //May need to write DIG_LOOP_CW[0] here...
    //TEMPORARY
    temp1 = 0x09EC;
    Write(tuner_id,A_DIG_LOOP_CW, temp1);
    DB5515_PRINTF("0x%04x = 0x%04x\n", A_DIG_LOOP_CW, temp1);  // Trigger the AGC Loop



    ///Write(tuner_id,IFAGC_REGX, 0x00a7);                    DB5515_PRINTF("address 0x0200 = 0x%04x\n", 0x00a7);    // DIG_LOOP_CW

}


/*****************************************************************************
* INT32 tun_db5515_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner db5515 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_db5515_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    /* check Tuner Configuration structure is available or not */
    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    if(tuner_id)
        *tuner_id = tuner_cnt;

    MEMCPY(&DB5515_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));

    //Write tuner init code here
    //Init the tuner mode,local structure and others

    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_db5515_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_db5515_status(UINT32 tuner_id, UINT8 *lock)
{
    BOOL IfLock=TRUE;

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    //Call get lock function here

    *lock = ( IfLock == TRUE ) ? 1 : 0;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_db5515_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_db5515_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    __MAYBE_UNUSED__ int i;  //m_bwsel_6,

    if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;
    //example: freq=554000(554MHz), bandwidth=8(8MHz)

    //Call write function for set freq/bandwidth to registers here
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////DB5515 tuner control and init////////////////////////////////////////
    //m_bwsel_6 = 0; //m_bwsel_6 = Bandwith --> 6MHz ='0', 7MHz='1', 8MHz='2'

    if(bandwidth == 6)
    {
        m_bwsel_6 = 0;
    }
    else if(bandwidth == 7)
    {
        m_bwsel_6 = 1;
    }
    else if(bandwidth == 8)
    {
        m_bwsel_6 = 2;
    }
    else
    {
        m_bwsel_6 = 0;
    }

    m_fData_RF = (double) freq / 1000;

    m_fData_IF = DB5515_Config[tuner_id].w_tuner_if_freq/1000; //if IF frequency is 4MHz, m_fData_IF is 4.
    //tuner_id = 194; //if tuner_id is 0xc2, tuner_id is 194(decimal).
    //m_fData_CLK = 27;
    //m_OpSysSel = 1;
    //m_OpSysSel = 0;

    m_fData_CLK = DB5515_Config[tuner_id].c_tuner_crystal/1000;  //27;//if X-tal frequency is 27MHz, m_fData_CLK is 27.
    //m_OpSysSel = 1;
    m_OpSysSel = 0;
    //if (m_OpSysSel == 1) For Cable (in case of DVB-C)
    //if {m_OpSysSel == 0) For Terrestrial(in case of DVB-T, ATSC, ISDB-T)

    CalculateLODIV(m_bwsel_6, m_OpSysSel);
    //printf("CalculateLODIV complet -> 11\n");
    CalculateVCO(m_bwsel_6);
    //printf("CalculateVCO complet -> 22\n");
    CalculateFractionalMode(m_fData_CLK);
    //printf("CalculateFractionalMode complet -> 33\n");
    CalculateTrackflt();
    //printf("CalculateTrackflt complet -> 44\n");
    SetRegChannel(tuner_id);
    //printf("SetRegChannel complet -> 55\n");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DB5515_DEBUG
    //UINT8 i;
    for(i = 0; i < 255; i++)
    {
        //DB5515_PRINTF("Reg:%d,    Value:%x\n", i, &reg_dump[i]);
    }
#endif
    return SUCCESS;
}
