//-----------------------------------------------------------------------------
// $Header:
// (C) Copyright 2007 NXP Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of NXP Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of NXP
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than NXP Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmbslTDA18218local.h
//
// DESCRIPTION:  define the Object for the TDA18218
//
// DOCUMENT REF: DVP Software Coding Guidelines v1.14
//               DVP Board Support Library Architecture Specification v0.5
//
// NOTES:
//-----------------------------------------------------------------------------
//
#ifndef _TMBSL_TDA18218LOCAL_H //-----------------
#define _TMBSL_TDA18218LOCAL_H

// History
// Date            Who                    Comments
//-----------------------------------------------------------------------------
// 2006/12/01    Michael Vannier     Created  former TDA18218 ES2D

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------

#define DVBT

#ifndef NXPFE
#ifdef _WIN32
#include "windows.h"
#endif
#endif

//#include "tmtypes.h"

#ifndef NXPFE
#include "tmhalFrontEndtypes.h"
#include "dvp.h"
#endif // NXPFE

#include "tmbslTDA18218.h"

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//
#ifndef NXPFE
#ifdef __cplusplus
extern "C"
{
#endif
#endif // NXPFE

    //-----------------------------------------------------------------------------
    // Types and defines:
    //-----------------------------------------------------------------------------
    //
#ifdef  UNICODE
#define __TEXT(quote) L##quote
#else   /* UNICODE */
#define __TEXT(quote) quote
#endif /* UNICODE */

#ifdef NXPFE
#define TEXT(quote) quote
#else // NXPFE
#define TEXT(quote) __TEXT(quote)
#endif // NXPFE

#ifdef NXPFE
#define _SYSTEMFUNC (**pObj->SystemFunc)
#else // NXPFE
#define _SYSTEMFUNC (pObj->SystemFunc)
#endif // NXPFE

#ifdef NXPFE
#define TMBSL_CLASS TEXT(tmdlTDA18218Driver::)
#else // NXPFE
#define TMBSL_CLASS
#endif // NXPFE

#define CONCAT3(a,b,c) a##b##c


#include "tmbslTDA18218.h"
#define TMBSL_FUNC(funcName) CONCAT3(tmbsl,TDA18218,funcName)
#define TMBSL_COMPONENT_NAME TDA18218
#define TMBSL_COMPONENT_NAME_STR "TDA18218"
#define tmTDA18218_DEVICE_NAME "TDA18218"
#define I2C_NO_READ_SUB_ADDRESS 0x00

#define tmTDA18218_BSL_COMP_NUM    1 // Major protocol change - Specification change required
#define tmTDA18218_BSL_MAJOR_VER   3 // Minor protocol change - Specification change required
#define tmTDA18218_BSL_MINOR_VER   21 // Software update - No protocol change - No specification change required

#define tmTDA18218_ID 0xc0

    //-------------
    // ERROR CODES
    //-------------
    // Invalid unit id
#define TDA18218_MAX_UNITS 4

#define TDA18218_LO_INIT (174000000)

#define TDA18218_NB_BYTES 59

#define TM_uRF 0
#define TM_uRF_Max 0
#define TM_uIR_GStep 1
#define TM_uBP_Filter 1
#define TM_uRF_Band 1
#define TM_uGain_Taper 1
#define TM_uRFC_Cprog 1
#define TM_uK 1
#define TM_uM 2
#define TM_uCOAmpl 3
#define TM_uTM_D 0
#define TM_uTM_60_92 1
#define TM_uTM_92_122 2
#define TM_uLO_Max 0
#define TM_uPost_Div 1
#define TM_uDiv 2

    // Invalid unit id
#define TMBSL_ERR_TUNER_BAD_UNIT_NUMBER \
    (TMBSL_ERR_TUNER_BASE + TM_ERR_BAD_UNIT_NUMBER)

    // Component is not initialized
#define TMBSL_ERR_TUNER_NOT_INITIALIZED \
    (TMBSL_ERR_TUNER_BASE+ TM_ERR_NOT_INITIALIZED)

    // Invalid input parameter
#define TMBSL_ERR_TUNER_BAD_PARAMETER \
    (TMBSL_ERR_TUNER_BASE + TM_ERR_BAD_PARAMETER)

    // Function is not supported
#define TMBSL_ERR_TUNER_NOT_SUPPORTED \
    (TMBSL_ERR_TUNER_BASE + TM_ERR_NOT_SUPPORTED)

    // Function is not supported
#define TMBSL_ERR_IIC_ERR \
    (TMBSL_ERR_TUNER_BASE + TM_ERR_IIC_ERR)

#define tmTDA18218_MAX_UNITS                        10            //
#define tmTDA18218_NB_BYTES                        59            //
#define tmTDA18218_LOG_NB_ROWS                    50            //

#define    tmTDA18218_BP_FILTER_MAP_NB_COLUMNS        2            //
#define    tmTDA18218_BP_FILTER_MAP_NB_ROWS            7            //

#define    tmTDA18218_RF_BAND_MAP_NB_COLUMNS            2            //
#define    tmTDA18218_RF_BAND_MAP_NB_ROWS            7            //

#define    tmTDA18218_GAIN_TAPER_MAP_NB_COLUMNS        2            //
#define    tmTDA18218_GAIN_TAPER_MAP_NB_ROWS            85            //

#define    tmTDA18218_RF_CAL_MAP_NB_COLUMNS            2            //
#define    tmTDA18218_RF_CAL_MAP_NB_ROWS                17            //

#define    tmTDA18218_RF_CAL_KMCO_MAP_NB_COLUMNS        4            //
#define    tmTDA18218_RF_CAL_KMCO_MAP_NB_ROWS        5            //

#define    tmTDA18218_THERMOMETER_MAP_NB_COLUMNS        3            //
#define    tmTDA18218_THERMOMETER_MAP_NB_ROWS        16            //

#define    tmTDA18218_MAIN_PLL_MAP_ID                0            //
#define    tmTDA18218_MAIN_PLL_MAP_NB_COLUMNS        3            //
#define    tmTDA18218_MAIN_PLL_MAP_NB_ROWS            40            //

#define    tmTDA18218_CAL_PLL_MAP_ID                    1            //
#define    tmTDA18218_CAL_PLL_MAP_NB_COLUMNS            3            //
#define    tmTDA18218_CAL_PLL_MAP_NB_ROWS            35            //

#define    tmTDA18218_AGC1_TOP_MAP_ID                2            //
    //#define    tmTDA18218_AGC1_TOP_MAP_NB_COLUMNS        8            //for ES2
#define    tmTDA18218_AGC1_TOP_MAP_NB_COLUMNS        18            //for ES3
#define    tmTDA18218_AGC1_TOP_MAP_NB_ROWS            1            //

#define    tmTDA18218_AGC2_TOP_MAP_ID                3            //
#define    tmTDA18218_AGC2_TOP_MAP_NB_COLUMNS        6            //
#define    tmTDA18218_AGC2_TOP_MAP_NB_ROWS            8            //

#define    tmTDA18218_IR_Map_ID                4
#define    tmTDA18218_IR_Map_NB_COLUMNS        1            //
#define    tmTDA18218_IR_Map_NB_ROWS        4            //

#define    tmTDA18218_IR_GSTEP_MAP_ID                5
#define    tmTDA18218_IR_GSTEP_MAP_NB_COLUMNS        2            //
#define    tmTDA18218_IR_GSTEP_MAP_NB_ROWS            3            //

#define    tmTDA18218_IR_Rx_MAP_ID                    6
#define    tmTDA18218_IR_Rx_MAP_NB_COLUMNS            4            //
#define    tmTDA18218_IR_Rx_MAP_NB_ROWS                1            //

#define    tmTDA18218_RAM_FCAL_MAP_ID                7            //
#define    tmTDA18218_RAM_FCAL_MAP_NB_COLUMNS        1            //
#define    tmTDA18218_RAM_FCAL_MAP_NB_ROWS            10            //

#define    tmTDA18218_CAPBANK_LF_MAP_ID                8            //
#define    tmTDA18218_CAPBANK_LF_MAP_NB_COLUMNS        9            //
#define    tmTDA18218_CAPBANK_LF_MAP_NB_ROWS            31            //

#define    tmTDA18218_CAPBANK_HF_MAP_ID                9            //
#define    tmTDA18218_CAPBANK_HF_MAP_NB_COLUMNS        9            //
#define    tmTDA18218_CAPBANK_HF_MAP_NB_ROWS            42            //

#define    tmTDA18218_DELTA_INDEX_MAP_ID                10            //
#define    tmTDA18218_DELTA_INDEX_MAP_NB_COLUMNS        1            //
#define    tmTDA18218_DELTA_INDEX_MAP_NB_ROWS        6            //

#define    tmTDA18218_RFAGC_TOP_MAP_ID                11            //
#define    tmTDA18218_RFAGC_TOP_MAP_NB_COLUMNS        2            //
#define    tmTDA18218_RFAGC_TOP_MAP_NB_ROWS        4            //

#define    tmTDA18218_THERMOMETER_NB_ROWS        16            //

#define    tmTDA18218_IDENTITY_IND                    0            //

#define OM5770C2_BOARD_DEF                    0x57700002  // TDA8295 + TDA10046 + tmTDA18218
#define OM5775_BOARD_DEF                    0x57750000  // 2xTDA8290C1 + 2xTDA10048 + 2xtmTDA18218
#define OM5776_BOARD_DEF                    0x57760000  //
#define OM5777_BOARD_DEF                    0x57770000  //
#define OM5780_BOARD_DEF                    0x57800000  //
#define OM5782_BOARD_DEF                    0x57820000  //
#define OM5764_BOARD_DEF                    0x57640199  //
#define CUSTOM_BOARD_DEF                    0x00000000  // custom
#define OM57XX_STANDARD_DEF                    2

#ifndef NXPFE
#ifdef _WIN32
    typedef     Bool    (*SY_OpenCom_t)            (void);
    typedef     UInt8    (*SY_SetComConfig_t)    (I2C_Conf_T);
    typedef        Bool    (*SY_SetDeviceAdd_t)    (char DeviceName[],UInt32 NewAdd);
    typedef     UInt32    (*SY_GetTickTime_t)        (void);
    typedef     UInt32    (*SY_GetTickPeriod_t)    (void);
    typedef     UInt32    (*SY_GetComError_t)        (void);
    typedef     Bool    (*SY_Write_t)            (UInt32 uAddress,
        UInt32 uSubAddress,
        UInt32 uNbData,
        UInt32* pDataBuff);
    typedef     Bool    (*SY_WriteBit_t)        (UInt32 uAddress,
        UInt32 uSubAddress,
        UInt32 uMaskValue,
        UInt32 uValue);
    typedef     Bool    (*SY_Read_t)            (UInt32 uAddress,
        UInt32 uSubAddress,
        UInt32 uNbData,
        UInt32* pDataBuff);
    typedef     UInt8    (*SY_SetComConfigFromVB_t)    (I2C_Conf_T);
    typedef     Bool    (*SY_OpenComFromVB_t)        (void);
    typedef        Bool    (*SY_SetDeviceAddFromVB_t)    (char DeviceName[],UInt32 NewAdd);
    typedef     Bool    (*SY_SetUrt_t)                (void);
    typedef     Bool    (*SY_SetErrorEnable_t)        (Bool ErrorEnable);
    typedef     void    (*SY_SetI2cPortFromVB_t)    (void);
    typedef     Bool    (*SY_CloseCom_t)        (void);
#endif
#endif //NXPFE

    typedef enum _tmTDA18218RFIOConfig_t {
        tmTDA18218RFIOConfigNotUsed,
        tmTDA18218Single_HZin_noLTO,
        tmTDA18218Single_HZin_LTOonMTO,
        tmTDA18218Single_HZin_LTOonLT_test,
        tmTDA18218Single_RFin_noLTO,
        tmTDA18218Single_RFin_LTOonLT,
        tmTDA18218Single_Rfin_LTOonMTO_test,
        tmTDA18218Master_HZin_noLTO,
        tmTDA18218Master_HZin_LTOonMTO,
        tmTDA18218Master_HZin_LTOonLT_test,
        tmTDA18218Master_RFin_noLTO,
        tmTDA18218Master_RFin_LTOonLT,
        tmTDA18218Master_Rfin_LTOonMTO_test,
        tmTDA18218Slave_M_HZin_noLTO,
        tmTDA18218Slave_M_HZin_LTOonMTO,
        tmTDA18218Slave_M_HZin_LTOonLT_test,
        tmTDA18218Slave_M_RFin_noLTO,
        tmTDA18218Slave_M_RFin_LTOonLT,
        tmTDA18218Slave_M_Rfin_LTOonMTO_test,
    } tmTDA18218RFIOConfig_t, *ptmTDA18218RFIOConfig_t;

    typedef enum _tmTDA18218ChannelType_t {
        tmTDA18218AnalogChannel = 0,
        tmTDA18218DigitalChannel
    } tmTDA18218ChannelType_t, *ptmTDA18218ChannelType_t;

    typedef enum _tmTDA18218ApplicationType_t {
        tmTDA18218HybridApplication = 0,
        tmTDA18218DigitalOnlyApplication ,
        tmTDA18218DigitalOnlyMultiApplication
    } tmTDA18218ApplicationType_t, *ptmTDA18218ApplicationType_t;

    typedef enum _tmTDA18218DeviceType_t {
        tmTDA18218MasterDeviceWithoutLT = 0,
        tmTDA18218MasterDeviceWithLT,
    } tmTDA18218DeviceType_t, *ptmTDA18218DeviceType_t;

    typedef enum _tmTDA18218ThermometerMode_t {
        tmTDA18218ThermometerOff = 0,                        // Thermometer off
        tmTDA18218ThermometerOn,                        // Thermometer on
    } tmTDA18218ThermometerMode_t, *ptmTDA18218ThermometerMode_t;

    typedef struct _tmTDA18218_BP_FILTER_Map_t {
        UInt32 uRF_Max;
        UInt32 uBP_Filter;
    } tmTDA18218_BP_FILTER_Map_t, *ptmTDA18218_BP_FILTER_Map_t;

    typedef struct _tmTDA18218_RF_BAND_Map_t {
        UInt32 uRF_Max;
        UInt32 uRF_Band;
    } tmTDA18218_RF_BAND_Map_t, *ptmTDA18218_RF_BAND_Map_t;

    typedef struct _tmTDA18218_GAIN_TAPER_Map_t {
        UInt32 uRF_Max;
        UInt32 uGain_Taper;
    } tmTDA18218_GAIN_TAPER_Map_t, *ptmTDA18218_GAIN_TAPER_Map_t;

    typedef struct _tmTDA18218_RF_CAL_Map_t {
        UInt32 uRF_Max;
        UInt32 uRFC_Cprog;
    } tmTDA18218_RF_CAL_Map_t, *ptmTDA18218_RF_CAL_Map_t;

    typedef struct _tmTDA18218_RF_CAL_KMCO_Map_t {
        UInt32 uRF_Max;
        UInt32 uK;
        UInt32 uM;
        UInt32 uCOAmpl;
    } tmTDA18218_RF_CAL_KMCO_Map_t, *ptmTDA18218_RF_CAL_KMCO_Map_t;

    typedef struct _tmTDA18218_THERMOMETER_Map_t {
        UInt32 uTM_D;
        UInt32 uTM_60_92;
        UInt32 uTM_92_122;
    } tmTDA18218_THERMOMETER_Map_t, *ptmTDA18218_THERMOMETER_Map_t;

    typedef struct _tmTDA18218_PLL_Map_t {
        UInt32 uLO_Max;
        UInt8 uPost_Div;
        UInt8 uDiv;
    } tmTDA18218_PLL_Map_t, *ptmTDA18218_PLL_Map_t;

    typedef struct _tmTDA18218_AGC1_TOP_Map_t {
        UInt8 uGu;
        UInt8 uGd;
        UInt8 uAd15;
        UInt8 uAd12;
        UInt8 uAd9;
        UInt8 uAd6; //for ES3
        UInt8 uAu3; //for ES3
        UInt8 uAu6;
        UInt8 uAu9;
        UInt8 uAu12;
    } tmTDA18218_AGC1_TOP_Map_t, *ptmTDA18218_AGC1_TOP_Map_t;

    typedef struct _tmTDA18218_AGC2_TOP_Map_t {
        UInt8 uGu2;
        UInt8 uGd2;
        UInt8 uGu3;
        UInt8 uGd3;
        UInt8 uGu4;
        UInt8 uGd4;
    } tmTDA18218_AGC2_TOP_Map_t, *ptmTDA18218_AGC2_TOP_Map_t;

    typedef struct _tmTDA18218_RFAGC_TOP_Map_t {
        UInt32 uAGC1Gain;
        UInt32 uRFAGCTOP;
    } tmTDA18218_RFAGC_TOP_Map_t, *ptmTDA18218_RFAGC_TOP_Map_t;

    typedef struct _tmTDA18218_IR_Map_t {
        UInt32 uRF_Max;
    } tmTDA18218_IR_Map_t, *ptmTDA18218_IR_Map_t;

    typedef struct _tmTDA18218_IR_GSTEP_Map_t {
        UInt32 uRF;
        UInt32 uIR_GStep;
    } tmTDA18218_IR_GSTEP_Map_t, *ptmTDA18218_IR_GSTEP_Map_t;

    typedef struct _tmTDA18218_IR_Rx_Map_t {
        UInt32 uR2I;
        UInt32 uR2Q;
        UInt32 uR3IQ;
        UInt32 uR3InQ;
    } tmTDA18218_IR_Rx_Map_t, *ptmTDA18218_IR_Rx_Map_t;

    typedef struct _tmTDA18218_RAM_FCAL_Map_t {
        UInt32 uFcal_Interval;
    } tmTDA18218_RAM_FCAL_Map_t, *ptmTDA18218_RAM_FCAL_Map_t;

    typedef struct _tmTDA18218_CAPBANK_Map_t {
        UInt32 uCtap;
        UInt32 uCad;
        UInt32 uCo;
        UInt32 uCtapColor;
        UInt32 uCadColor;
        UInt32 uCoColor;
        Int32 uRAMCtap;
        Int32 uRAMCad;
        Int32 uRAMCo;
    } tmTDA18218_CAPBANK_Map_t, *ptmTDA18218_CAPBANK_Map_t;

    typedef struct _tmTDA18218_DELTA_INDEX_Map_t {
        Int32 uDeltaIndex;
    } tmTDA18218_DELTA_INDEX_Map_t, *ptmTDA18218_DELTA_INDEX_Map_t;


    typedef struct _tmTDA18218Config_t {
        UInt32                                uBoard;
        UInt32                                uStandard;
        UInt32                                uRF;
        UInt32                                uIF;
        UInt32                                uBW;
        UInt32                                uRF_IF_Det;
        UInt32                                uAGC1_Init;
        UInt32                                uNominalIRFound;
        UInt32                                IRCoefHighF[4];
        tmTDA18218_BP_FILTER_Map_t            BP_FILTER_Map[tmTDA18218_BP_FILTER_MAP_NB_ROWS];
        tmTDA18218_RF_BAND_Map_t            RF_BAND_Map[tmTDA18218_RF_BAND_MAP_NB_ROWS];
        tmTDA18218_GAIN_TAPER_Map_t            GAIN_TAPER_Map[tmTDA18218_GAIN_TAPER_MAP_NB_ROWS];
        tmTDA18218_RF_CAL_Map_t                RF_CAL_Map[tmTDA18218_RF_CAL_MAP_NB_ROWS];
        tmTDA18218_RF_CAL_KMCO_Map_t        RF_CAL_KMCO_Map[tmTDA18218_RF_CAL_KMCO_MAP_NB_ROWS];
        tmTDA18218_THERMOMETER_Map_t        THERMOMETER_Map[tmTDA18218_THERMOMETER_MAP_NB_ROWS];
        tmTDA18218_PLL_Map_t                MAIN_PLL_Map[tmTDA18218_MAIN_PLL_MAP_NB_ROWS];
        tmTDA18218_PLL_Map_t                CAL_PLL_Map[tmTDA18218_CAL_PLL_MAP_NB_ROWS];
        tmTDA18218_AGC1_TOP_Map_t            AGC1_TOP_Map[tmTDA18218_AGC1_TOP_MAP_NB_ROWS];
        tmTDA18218_AGC2_TOP_Map_t            AGC2_TOP_Map[tmTDA18218_AGC2_TOP_MAP_NB_ROWS];
        tmTDA18218_RFAGC_TOP_Map_t            RFAGC_TOP_Map[tmTDA18218_RFAGC_TOP_MAP_NB_ROWS];
        tmTDA18218_IR_Map_t                    IR_Map[tmTDA18218_IR_Map_NB_ROWS];
        tmTDA18218_IR_Rx_Map_t                IR_Rx_Map[tmTDA18218_IR_Rx_MAP_NB_ROWS];
        tmTDA18218_RAM_FCAL_Map_t            RAM_FCAL_Map[tmTDA18218_RAM_FCAL_MAP_NB_ROWS];
        tmTDA18218_CAPBANK_Map_t            CAPBANK_LF_Map[tmTDA18218_CAPBANK_LF_MAP_NB_ROWS];
        tmTDA18218_CAPBANK_Map_t            CAPBANK_HF_Map[tmTDA18218_CAPBANK_HF_MAP_NB_ROWS];
        tmTDA18218_DELTA_INDEX_Map_t        DELTA_INDEX_Map[tmTDA18218_DELTA_INDEX_MAP_NB_ROWS];
    } tmTDA18218Config_t, *ptmTDA18218Config_t;

    typedef struct _tmTDA18218I2CMap_t
    {
        union
        {
            UInt8 ID_byte;                    // address  0x00
            struct
            {
                UInt8 ID            :7;        // Chip identification number
                UInt8 UNUSED_0x00    :1;        // Reserved, must be written as 1
            }bF;
        }uBx00;

        union
        {
            UInt8 Read_byte_1;                // address 0x01
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 POR            :1;
                UInt8 LO_Lock        :1;
                UInt8 CAL_Lock       :1;
                UInt8 LT_2           :1;
                UInt8 TM_D           :4;
#else
                UInt8 TM_D           :4;
                UInt8 LT_2           :1;
                UInt8 CAL_Lock       :1;
                UInt8 LO_Lock        :1;
                UInt8 POR            :1;
#endif
            }bF;
        }uBx01;

        union
        {
            UInt8 Read_byte_2;                // address 0x02
            UInt8 RSSI;
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RSSI_7        :1;
                UInt8 RSSI_6        :1;
                UInt8 RSSI_5        :1;
                UInt8 RSSI_4        :1;
                UInt8 RSSI_3        :1;
                UInt8 RSSI_2        :1;
                UInt8 RSSI_1        :1;
                UInt8 RSSI_0        :1;
#else
                UInt8 RSSI_0        :1;
                UInt8 RSSI_1        :1;
                UInt8 RSSI_2        :1;
                UInt8 RSSI_3        :1;
                UInt8 RSSI_4        :1;
                UInt8 RSSI_5        :1;
                UInt8 RSSI_6        :1;
                UInt8 RSSI_7        :1;
#endif
            }bF;
        }uBx02;

        union
        {
            UInt8 Read_byte_3;                // address 0x03
            UInt8 Power_Level_15_to_8;
            UInt8 AGC2;
        }uBx03;

        union
        {
            UInt8 Read_byte_4;                // address 0x04
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 Power_Level_AGC1_2        :1;
                UInt8 PB_IRCAL                  :1;
                UInt8 IR_CAL_OK                 :1;
                UInt8 RSSI_Alarm                :1;
                UInt8 LT                        :2;
                UInt8 Power_Level_AGC1          :2;
#else
                UInt8 Power_Level_AGC1          :2;
                UInt8 LT                        :2;
                UInt8 RSSI_Alarm                :1;
                UInt8 IR_CAL_OK                 :1;
                UInt8 PB_IRCAL                  :1;
                UInt8 Power_Level_AGC1_2        :1;
#endif
            }bF;
        }uBx04;

        union
        {
            UInt8 Read_byte_5;                // address 0x05
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 msm_rst                   :1;
                UInt8 FlagClear                 :1;
                UInt8 launch_msm                :1;
                UInt8 FlagMSM_OK                :1;
                UInt8 RSSI_Sense_OK             :1;
                UInt8 FlagSuperpos              :1;
                UInt8 FlagRFcal_AV_End          :1;
                UInt8 FlagRFcal_POR_End         :1;
#else
                UInt8 FlagRFcal_POR_End         :1;
                UInt8 FlagRFcal_AV_End          :1;
                UInt8 FlagSuperpos              :1;
                UInt8 RSSI_Sense_OK             :1;
                UInt8 FlagMSM_OK                :1;
                UInt8 launch_msm                :1;
                UInt8 FlagClear                 :1;
                UInt8 msm_rst                   :1;
#endif
            }bF;
        }uBx05;

        union
        {
            UInt8 Read_byte_6;                // address 0x06
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x06               :2;
                UInt8 DataRAM                   :6;
#else
                UInt8 DataRAM                   :6;
                UInt8 UNUSED_0x06               :2;
#endif
            }bF;
        }uBx06;

        union
        {
            UInt8 PSM_byte_1;                // address 0x07
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x07               :1;
                UInt8 Freq_prog_En              :1;
                UInt8 PSM_LOVCO                 :2;
                UInt8 PSM_Mixer                 :2;
                UInt8 PSM_Lodriver              :2;
#else
                UInt8 PSM_Lodriver              :2;
                UInt8 PSM_Mixer                 :2;
                UInt8 PSM_LOVCO                 :2;
                UInt8 Freq_prog_En              :1;
                UInt8 UNUSED_0x07               :1;
#endif
            }bF;
        }uBx07;

        union
        {   
            UInt8 Main_Divider_byte_1;        // address 0x08
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x08_0             :1;
                UInt8 LOPostDiv                 :3;
                UInt8 UNUSED_0x08_1             :1;
                UInt8 LOPresc                   :3;
#else
                UInt8 LOPresc                   :3;
                UInt8 UNUSED_0x08_1             :1;
                UInt8 LOPostDiv                 :3;
                UInt8 UNUSED_0x08_0             :1;
#endif
            }bF;
        }uBx08;

        union
        {
            UInt8 Main_Divider_byte_2;        // address 0x09
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x09               :1;
                UInt8 LO_Int                    :7;
#else
                UInt8 LO_Int                    :7;
                UInt8 UNUSED_0x09               :1;
#endif
            }bF;
        }uBx09;

        union
        {
            UInt8 Main_Divider_byte_3;        // address 0x0A
            UInt8 LO_Frac_31_to_24;
        }uBx0A;

        union
        {
            UInt8 Main_Divider_byte_4;        // address 0x0B
            UInt8 LO_Frac_23_to_16;
        }uBx0B;

        union
        {
            UInt8 Main_Divider_byte_5;        // address 0x0C
            UInt8 LO_Frac_15_to_8;
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 LO_Frac_15_to_12          :4;
                UInt8 LO_Frac_11_to_8           :4;
#else
                UInt8 LO_Frac_11_to_8           :4;
                UInt8 LO_Frac_15_to_12          :4;
#endif
            }bF;
        }uBx0C;

        union
        {
            UInt8 Main_Divider_byte_6;        // address 0x0D
            UInt8 LO_Frac_7_to_0;
        }uBx0D;

        union
        {
            UInt8 Main_Divider_byte_7;        // address 0x0E
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 LOSD_clock                :1;
                UInt8 LOSD_Static_N             :1;
                UInt8 LOSD_BascD                :1;
                UInt8 LOSD_dpa2                 :1;
                UInt8 LOSD_dpa1                 :1;
                UInt8 LOSDrstN                  :1;
                UInt8 LOSD_test_En              :1;
                UInt8 UNUSED_0x0E               :1;
#else
                UInt8 UNUSED_0x0E               :1;
                UInt8 LOSD_test_En              :1;
                UInt8 LOSDrstN                  :1;
                UInt8 LOSD_dpa1                 :1;
                UInt8 LOSD_dpa2                 :1;
                UInt8 LOSD_BascD                :1;
                UInt8 LOSD_Static_N             :1;
                UInt8 LOSD_clock                :1;
#endif
            }bF;
        }uBx0E;

        union
        {
            UInt8 Main_Divider_byte_8;        // address 0x0F
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 state_pll_out_2           :1;
                UInt8 Freq_prog_Start           :1;
                UInt8 state_pll_out_0           :1;
                UInt8 LO_Sn05                   :1;
                UInt8 LO_Sn05b                  :1;
                UInt8 LO_ForceSrce              :1;
                UInt8 LO_ForceSink              :1;
                UInt8 LO_Auto_src               :1;
#else
                UInt8 LO_Auto_src               :1;
                UInt8 LO_ForceSink              :1;
                UInt8 LO_ForceSrce              :1;
                UInt8 LO_Sn05b                  :1;
                UInt8 LO_Sn05                   :1;
                UInt8 state_pll_out_0           :1;
                UInt8 Freq_prog_Start           :1;
                UInt8 state_pll_out_2           :1;
#endif
            }bF;
        }uBx0F;

        union
        {
            UInt8 Cal_Divider_byte_1;        // address 0x10
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 state_pll_out_1           :1;
                UInt8 CALPostDiv                :3;
                UInt8 CALPrescSW                :1;
                UInt8 CALPresc                  :3;
#else
                UInt8 CALPresc                  :3;
                UInt8 CALPrescSW                :1;
                UInt8 CALPostDiv                :3;
                UInt8 state_pll_out_1           :1;
#endif
            }bF;
        }uBx10;

        union
        {          
            Int8 Cal_Divider_byte_2;        // address 0x11
            Int8 CAL_Int;
        }uBx11;

        union
        {
            UInt8 Cal_Divider_byte_3;        // address 0x12
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x12               :1;
                UInt8 CAL_Frac_22_to_16         :7;
#else
                UInt8 CAL_Frac_22_to_16         :7;
                UInt8 UNUSED_0x12               :1;
#endif
            }bF;
        }uBx12;

        union
        {
            UInt8 Cal_Divider_byte_4;        // address 0x13
            UInt8 CAL_Frac_15_to_8;
        }uBx13;

        union
        {
            UInt8 Cal_Divider_byte_5;        // address 0x14
            UInt8 CAL_Frac_7_to_0;
        }uBx14;

        union
        {
            UInt8 Cal_Divider_byte_6;        // address 0x15
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 CALSD_clock               :1;
                UInt8 CALSD_Static_N            :1;
                UInt8 CALSD_BascD               :1;
                UInt8 CALSD_dpa2                :1;
                UInt8 CALSD_dpa1                :1;
                UInt8 CALSDrstN                 :1;
                UInt8 CALSD_test_En             :1;
                UInt8 PD_CALvco_Testn           :1;
#else
                UInt8 PD_CALvco_Testn           :1;
                UInt8 CALSD_test_En             :1;
                UInt8 CALSDrstN                 :1;
                UInt8 CALSD_dpa1                :1;
                UInt8 CALSD_dpa2                :1;
                UInt8 CALSD_BascD               :1;
                UInt8 CALSD_Static_N            :1;
                UInt8 CALSD_clock               :1;
#endif
            }bF;
        }uBx15;

        union    
        {
            UInt8 Cal_Divider_byte_7;        // address 0x16
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 MixGateDC                 :2;
                UInt8 UNUSED_0x16               :1;
                UInt8 CAL_Sn05                  :1;
                UInt8 CAL_Sn05b                 :1;
                UInt8 CAL_ForceSrce             :1;
                UInt8 CAL_ForceSink             :1;
                UInt8 CAL_Auto_src              :1;
#else
                UInt8 CAL_Auto_src              :1;
                UInt8 CAL_ForceSink             :1;
                UInt8 CAL_ForceSrce             :1;
                UInt8 CAL_Sn05b                 :1;
                UInt8 CAL_Sn05                  :1;
                UInt8 UNUSED_0x16               :1;
                UInt8 MixGateDC                 :2;
#endif
            }bF;
        }uBx16;

        union
        {
            UInt8 Power_Down_byte_1;        // address 0x17
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x17               :1;
                UInt8 pdLT                      :1;
                UInt8 pdAGC1a                   :1;
                UInt8 pdHZlna                   :1;
                UInt8 pdAGC1b                   :1;
                UInt8 PD_RFAGC_Ifout            :1;
                UInt8 PD_LO_Synthe              :1;
                UInt8 SM                        :1;
#else
                UInt8 SM                        :1;
                UInt8 PD_LO_Synthe              :1;
                UInt8 PD_RFAGC_Ifout            :1;
                UInt8 pdAGC1b                   :1;
                UInt8 pdHZlna                   :1;
                UInt8 pdAGC1a                   :1;
                UInt8 pdLT                      :1;
                UInt8 UNUSED_0x17               :1;
#endif
            }bF;
        }uBx17;

        union
        {
            UInt8 Power_Down_byte_2;        // address 0x18
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RFSW_MTO_LT_Hzin          :1;
                UInt8 RFSW_MTO_LT_RFin          :1;
                UInt8 RFSW_Buf1_out             :1;
                UInt8 RFSW_AGC1b_out            :1;
                UInt8 PD_RFAGC_Det              :1;
                UInt8 pdDETECT1                 :1;
                UInt8 pdAGC2b                   :1;
                UInt8 PD_LOtest                 :1;
#else
                UInt8 PD_LOtest                 :1;
                UInt8 pdAGC2b                   :1;
                UInt8 pdDETECT1                 :1;
                UInt8 PD_RFAGC_Det              :1;
                UInt8 RFSW_AGC1b_out            :1;
                UInt8 RFSW_Buf1_out             :1;
                UInt8 RFSW_MTO_LT_RFin          :1;
                UInt8 RFSW_MTO_LT_Hzin          :1;
#endif
            }bF;
        }uBx18;

        union
        {
            UInt8 Xtout_Byte;                // address 0x19
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 hc_mode                   :4;
                UInt8 XtOut                     :4;
#else
                UInt8 XtOut                     :4;
                UInt8 hc_mode                   :4;
#endif
            }bF;
        }uBx19;

        union
        {
            UInt8 IF_Byte_1;                    // address 0x1A
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 IF_notch                  :1;
                UInt8 IF_level                  :3;
                UInt8 H3H5_Gain_3dB             :1;
                UInt8 BP_Filter                 :3;
#else
                UInt8 BP_Filter                 :3;
                UInt8 H3H5_Gain_3dB             :1;
                UInt8 IF_level                  :3;
                UInt8 IF_notch                  :1;
#endif
            }bF;
        }uBx1A;

        union
        {
            UInt8 IF_Byte_2;                    // address 0x1B
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RFAGC_OpenLoop            :1;
                UInt8 RF_IF_Det                 :1;
                UInt8 pdAGC2a                   :1;
                UInt8 AGC_Low_BW                :1;
                UInt8 UNUSED_0x1B               :2;
                UInt8 LP_Fc                     :2;
#else
                UInt8 LP_Fc                     :2;
                UInt8 UNUSED_0x1B               :2;
                UInt8 AGC_Low_BW                :1;
                UInt8 pdAGC2a                   :1;
                UInt8 RF_IF_Det                 :1;
                UInt8 RFAGC_OpenLoop            :1;
#endif
            }bF;
        }uBx1B; 

        union
        {
            UInt8 AGC2b_byte;                // address 0x1C
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 pulse_up_auto             :1;
                UInt8 pulse_up_width            :2;
                UInt8 AGC_On                    :1;
                UInt8 DETECT4_Cap_Reset_En      :1;
                UInt8 pdAGC2_DETECT4            :1;
                UInt8 pdAGC2_DETECT3            :1;
                UInt8 pdAGC2_DETECT2            :1;
#else
                UInt8 pdAGC2_DETECT2            :1;
                UInt8 pdAGC2_DETECT3            :1;
                UInt8 pdAGC2_DETECT4            :1;
                UInt8 DETECT4_Cap_Reset_En      :1;
                UInt8 AGC_On                    :1;
                UInt8 pulse_up_width            :2;
                UInt8 pulse_up_auto             :1;
#endif
            }bF;              
        }uBx1C;

        union
        {
            UInt8 PSM_byte_2;                // address 0x1D
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 TM_Range                  :1;
                UInt8 TM_ON                     :1;
                UInt8 PSM_IFpoly                :2;
                UInt8 test_mode_agc2            :1;
                UInt8 LO_iqfib                  :1;
                UInt8 LO_iqmos                  :1;
                UInt8 H3H5_Dcboostn             :1;
#else
                UInt8 H3H5_Dcboostn             :1;
                UInt8 LO_iqmos                  :1;
                UInt8 LO_iqfib                  :1;
                UInt8 test_mode_agc2            :1;
                UInt8 PSM_IFpoly                :2;
                UInt8 TM_ON                     :1;
                UInt8 TM_Range                  :1;
#endif
            }bF;
        }uBx1D;

        union
        {
            UInt8 PSM_byte_3;                // address 0x1E
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 PSM_AGC1                  :2;
                UInt8 STOB_DC                   :2;
                UInt8 PSM_StoB                  :2;
                UInt8 PSM_RFpoly                :2;
#else
                UInt8 PSM_RFpoly                :2;
                UInt8 PSM_StoB                  :2;
                UInt8 STOB_DC                   :2;
                UInt8 PSM_AGC1                  :2;
#endif
            }bF;
        }uBx1E;

        union
        {
            UInt8 PSM_byte_4;                // address 0x1F
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 AGC1_Speed                :2;
                UInt8 digck_mode                :2;
                UInt8 UNUSED_0x1F               :1;
                UInt8 AGC1_aud_sel              :1;
                UInt8 AGC1_au_ptr               :2;
#else
                UInt8 AGC1_au_ptr               :2;
                UInt8 AGC1_aud_sel              :1;
                UInt8 UNUSED_0x1F               :1;
                UInt8 digck_mode                :2;
                UInt8 AGC1_Speed                :2;
#endif
            }bF;
        }uBx1F;

        union
        {
            UInt8 AGC1_byte_1;                // address 0x20
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 AGC2_RAM_sel              :2;
                UInt8 AGC2_Gup_sel              :1;
                UInt8 AGC1_Gup_sel              :1;
                UInt8 Manual_LT                 :1;
                UInt8 AGC1_aud                  :3;
#else
                UInt8 AGC1_aud                  :3;
                UInt8 Manual_LT                 :1;
                UInt8 AGC1_Gup_sel              :1;
                UInt8 AGC2_Gup_sel              :1;
                UInt8 AGC2_RAM_sel              :2;
#endif
            }bF; 
        }uBx20;

        union
        {
            UInt8 AGC1_byte_2;                // address 0x21
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 AGC2_Speed                :2;
                UInt8 RFagc_fast                :1;
                UInt8 AGC1_Gud                  :5;
#else
                UInt8 AGC1_Gud                  :5;
                UInt8 RFagc_fast                :1;
                UInt8 AGC2_Speed                :2;
#endif
            }bF;
        }uBx21;

        union
        {
            UInt8 AGC1_byte_3;                // address 0x22
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 BUF1b_DCloopIH            :1;
                UInt8 BUF1a_DCCompIH            :1;
                UInt8 AGC1_loop_off             :1;
                UInt8 AGC1_bypass               :1;
                UInt8 AGC1_Speed_Up             :1;
                UInt8 AGC1_Speed_Do             :1;
                UInt8 AGC1_Do_step              :2;
#else
                UInt8 AGC1_Do_step              :2;
                UInt8 AGC1_Speed_Do             :1;
                UInt8 AGC1_Speed_Up             :1;
                UInt8 AGC1_bypass               :1;
                UInt8 AGC1_loop_off             :1;
                UInt8 BUF1a_DCCompIH            :1;
                UInt8 BUF1b_DCloopIH            :1;
#endif
            }bF;
        }uBx22;

        union
        {
            UInt8 AGC2_byte_1;                // address 0x23
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x23               :3;
                UInt8 AGC2_Gud                  :5;
#else
                UInt8 AGC2_Gud                  :5;
                UInt8 UNUSED_0x23               :3;
#endif
            }bF;
        }uBx23;

        union
        {
            UInt8 AGC2_byte_2;                // address 0x24
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 AGC1_gain_Force           :1;
                UInt8 AGC2_gain_Force           :1;
                UInt8 AGC2_loop_off             :1;
                UInt8 AGC2_bypass               :1;
                UInt8 AGC2_Speed_Up             :1;
                UInt8 AGC2_Speed_Do             :1;
                UInt8 AGC2_Do_step              :2;
#else
                UInt8 AGC2_Do_step              :2;
                UInt8 AGC2_Speed_Do             :1;
                UInt8 AGC2_Speed_Up             :1;
                UInt8 AGC2_bypass               :1;
                UInt8 AGC2_loop_off             :1;
                UInt8 AGC2_gain_Force           :1;
                UInt8 AGC1_gain_Force           :1;
#endif
            }bF;
        }uBx24;

        union
        {
            UInt8 Analog_AGC_byte;            // address 0x25
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RC_Cal_Force              :1;
                UInt8 RFAGC_Top                 :3;
                UInt8 IFAGC_Top                 :4;
#else
                UInt8 IFAGC_Top                 :4;
                UInt8 RFAGC_Top                 :3;
                UInt8 RC_Cal_Force              :1;
#endif
            }bF;
        }uBx25;

        union
        {
            UInt8 RC_byte;                    // address 0x26
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RC_Cal_3_to_0             :4;
                UInt8 RC_ReadCal_3_to_0         :4;
#else
                UInt8 RC_ReadCal_3_to_0         :4;
                UInt8 RC_Cal_3_to_0             :4;
#endif
            }bF;
        }uBx26;

        union
        {
            UInt8 RSSI_byte;                // address 0x27
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RC_Cal_4                  :1;
                UInt8 RC_ReadCal_4              :1;
                UInt8 RSSI_AV                   :1;
                UInt8 RSSI_sinus_QAMnot         :1;
                UInt8 RSSI_Cap_Reset_En         :1;
                UInt8 RSSI_Cap_Val              :1;
                UInt8 RSSI_Ck_Speed             :1;
                UInt8 RSSI_Dicho_not            :1;
#else
                UInt8 RSSI_Dicho_not            :1;
                UInt8 RSSI_Ck_Speed             :1;
                UInt8 RSSI_Cap_Val              :1;
                UInt8 RSSI_Cap_Reset_En         :1;
                UInt8 RSSI_sinus_QAMnot         :1;
                UInt8 RSSI_AV                   :1;
                UInt8 RC_ReadCal_4              :1;
                UInt8 RC_Cal_4                  :1;
#endif
            }bF;
        }uBx27;

        union
        {
            UInt8 IR_CAL_byte_1;            // address 0x28
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x28               :1;
                UInt8 IR_Loop                   :3;
                UInt8 Force_Lock                :1;
                UInt8 IR_Target                 :3;
#else
                UInt8 IR_Target                 :3;
                UInt8 Force_Lock                :1;
                UInt8 IR_Loop                   :3;
                UInt8 UNUSED_0x28               :1;
#endif
            }bF;
        }uBx28;

        union
        {            
            UInt8 IR_CAL_byte_2;            // address 0x29
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 extra_control2            :1;
                UInt8 IR_GStep                  :3;
                UInt8 STOBattON                 :1;
                UInt8 STOB_gain                 :1;
                UInt8 IR_Freq                   :2;
#else
                UInt8 IR_Freq                   :2;
                UInt8 STOB_gain                 :1;
                UInt8 STOBattON                 :1;
                UInt8 IR_GStep                  :3;
                UInt8 extra_control2            :1;
#endif
            }bF;
        }uBx29;

        union
        {
            UInt8 IR_CAL_byte_3;            // address 0x2A
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 shift                     :2;
                UInt8 IR_Cal_Boost              :2;
                UInt8 AGCK_minus3               :1;
                UInt8 Rx_Force                  :1;
                UInt8 Ptr_Rx_Read               :2;
#else
                UInt8 Ptr_Rx_Read               :2;
                UInt8 Rx_Force                  :1;
                UInt8 AGCK_minus3               :1;
                UInt8 IR_Cal_Boost              :2;
                UInt8 shift                     :2;
#endif
            }bF;
        }uBx2A;

        union
        {
            UInt8 IR_CAL_byte_4;            // address 0x2B
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 Rx_ptrW                   :2;
                UInt8 Rx_W                      :6;
#else
                UInt8 Rx_W                      :6;
                UInt8 Rx_ptrW                   :2;
#endif
            }bF;
        }uBx2B;

        union
        {
            UInt8 RF_Cal_byte_1;            // address 0x2C
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 HP_RF                     :1;
                UInt8 HP_LO                     :1;
                UInt8 CO_Amp                    :2;
                UInt8 Extra_control1            :1;
                UInt8 UNUSED_0x2C               :1;
                UInt8 AGC_DC                    :2;
#else
                UInt8 AGC_DC                    :2;
                UInt8 UNUSED_0x2C               :1;
                UInt8 Extra_control1            :1;
                UInt8 CO_Amp                    :2;
                UInt8 HP_LO                     :1;
                UInt8 HP_RF                     :1;
#endif
            }bF;
        }uBx2C;

        union
        {
            UInt8 RF_Cal_byte_2;            // address 0x2D
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 RFcal_RST_N               :1;
                UInt8 Indx_filter_sel           :1;
                UInt8 capw_filter_sel           :1;
                UInt8 Sel_3dB                   :1;
                UInt8 RefineAV                  :1;
                UInt8 HFbypassPOR               :1;
                UInt8 rfcal_clock_en            :1;
                UInt8 RFcal_modeOFF             :1;
#else
                UInt8 RFcal_modeOFF             :1;
                UInt8 rfcal_clock_en            :1;
                UInt8 HFbypassPOR               :1;
                UInt8 RefineAV                  :1;
                UInt8 Sel_3dB                   :1;
                UInt8 capw_filter_sel           :1;
                UInt8 Indx_filter_sel           :1;
                UInt8 RFcal_RST_N               :1;
#endif
            }bF;
        }uBx2D;

        union
        {
            UInt8 RF_Cal_byte_3;            // address 0x2E
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 LF_nHF                    :1;
                UInt8 FilterIndex               :7;
#else
                UInt8 FilterIndex               :7;
                UInt8 LF_nHF                    :1;
#endif
            }bF;
        }uBx2E;

        union
        {
            UInt8 RF_Cal_byte_4;            // address 0x2F
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x2F               :1;
                UInt8 Center                    :7;
#else
                UInt8 Center                    :7;
                UInt8 UNUSED_0x2F               :1;
#endif
            }bF;
        }uBx2F;

        union
        {
            UInt8 RF_Cal_byte_5;            // address 0x30
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 FreqCut_sel               :1;
                UInt8 FreqCut                   :7;
#else
                UInt8 FreqCut                   :7;
                UInt8 FreqCut_sel               :1;
#endif
            }bF;
        }uBx30;

        union
        {
            UInt8 RF_Cal_byte_6;            // address 0x31
            UInt8 GainMax_out;
        }uBx31;

        union
        {
            UInt8 RF_Cal_byte_7;            // address 0x32
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 GAIN_threshold            :3;
                UInt8 ShiftLF                   :2;
                UInt8 NmeasAV                   :3;
#else
                UInt8 NmeasAV                   :3;
                UInt8 ShiftLF                   :2;
                UInt8 GAIN_threshold            :3;
#endif
            }bF;
        }uBx32;

        union
        {
            UInt8 RF_Cal_byte_8;            // address 0x33
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x33               :1;
                UInt8 capw_gain                 :7;
#else
                UInt8 capw_gain                 :7;
                UInt8 UNUSED_0x33               :1;
#endif
            }bF;
        }uBx33;

        union
        {
            UInt8 RF_Cal_byte_9;            // address 0x34
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x34               :1;
                UInt8 capw_cad                  :7;
#else
                UInt8 capw_cad                  :7;
                UInt8 UNUSED_0x34               :1;
#endif
            }bF;
        }uBx34;

        union
        {
            UInt8 RF_Cal_byte_10;            // address 0x35
            struct          
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x35               :1;
                UInt8 capw_ctank                :7;
#else
                UInt8 capw_ctank                :7;
                UInt8 UNUSED_0x35               :1;
#endif
            }bF;
        }uBx35;

        union
        {
            UInt8 RF_Cal_RAM_byte_1;        // address 0x36
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 UNUSED_0x36_2             :3;
                UInt8 SelRAM                    :1;
                UInt8 UNUSED_0x36_1             :1;
                UInt8 ModeI2CRAM                :1;
                UInt8 UNUSED_0x36_0             :1;
                UInt8 WRI2CRAM                  :1;
#else
                UInt8 WRI2CRAM                  :1;
                UInt8 UNUSED_0x36_0             :1;
                UInt8 ModeI2CRAM                :1;
                UInt8 UNUSED_0x36_1             :1;
                UInt8 SelRAM                    :1;
                UInt8 UNUSED_0x36_2             :3;
#endif
            }bF;
        }uBx36;

        union
        {
            UInt8 RF_Cal_RAM_byte_2;        // address 0x37
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 test_scan                 :4;
                UInt8 AddRAM                    :4;
#else
                UInt8 AddRAM                    :4;
                UInt8 test_scan                 :4;
#endif
            }bF;
        }uBx37;

        union
        {
            UInt8 Debug_byte;                // address 0x38
            struct
            {
#ifdef _BIG_ENDIAN
                UInt8 Sel_state                  :3;
                UInt8 state_out                  :5;
#else
                UInt8 state_out                  :5;
                UInt8 Sel_state                  :3;
#endif
            }bF;
        }uBx38;

        union
        {
            UInt8 Fmax_byte_1;                // address 0x39
            UInt8 Fmax_Lo;
        }uBx39;

        union
        {
            UInt8 Fmax_byte_2;                // address 0x3A
            UInt8 Fmax_Cal;
        }uBx3A;

    } tmTDA18218I2CMap_t, *ptmTDA18218I2CMap_t;

    typedef struct _tmTDA18218Object_t {
        Bool                            init;
        UInt32                            uHwAddress;
        tmUnitSelect_t                    eUnit;
#ifdef NXPFE
        tmdlDefaultGCRoot<tmdlDefaultDeviceDriver::tmhalFEBslSystemFunc_c^>    SystemFunc;
#else
        tmhalFEBslSystemFunc_t            SystemFunc;
#endif
        tmTDA18218PowerState_t        curPowerState;
        tmTDA18218RFIOConfig_t        RFIOConfig;
        tmTDA18218StandardMode_t        StandardMode;
        tmTDA18218ChannelType_t        ChannelType;
        tmTDA18218ApplicationType_t    ApplicationType;
        tmTDA18218DeviceType_t        DeviceType;
        tmTDA18218Config_t            Config;
        tmTDA18218I2CMap_t            I2CMap;
        UInt32                            uTickEnd;
    } tmTDA18218Object_t, *ptmTDA18218Object_t, **pptmTDA18218Object_t;

#ifndef NXPFE
#ifdef __cplusplus
}
#endif
#endif //NXPFE

#endif // _TMBSL_TDA18218LOCAL_H //---------------
