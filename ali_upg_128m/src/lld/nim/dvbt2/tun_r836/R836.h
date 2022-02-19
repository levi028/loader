//#include "..\stdafx.h"
#ifndef  _R836_H_ 
#define _R836_H_

//----------------------------------------------------------//
//                   Define                                            //
//----------------------------------------------------------//
#define VERSION   "R836_v1.10C_Availink_DTMB"    
#define VER_NUM   10

#define R840_USE_16M_XTAL     1    //16 or 24M

#if (R840_USE_16M_XTAL==1)
    #define R840_Xtal	  16000      //16M
#else
    #define R840_Xtal	  24000      //24M
#endif

#define R840_REG_NUM         32
#define R840_TF_HIGH_NUM  7  
#define R840_TF_MID_NUM    8
#define R840_TF_LOW_NUM   8
#define R840_TF_LOWEST_NUM   8
#define R840_RING_POWER_FREQ  115000
#define R840_IMR_IF              5300         
#define R840_IMR_TRIAL       9
//#define R840_Xtal	                   16000

//----------------------------------------------------------//
//                   Type Define                            //
//----------------------------------------------------------//

#define UINT8  unsigned char
#define UINT16 unsigned short
#define UINT32 unsigned long


//----------------------------------------------------------//
//                   Internal Structure                            //
//----------------------------------------------------------//
typedef struct _Sys_Info_Type
{
	UINT8          BW;
	UINT8		    V17M; 
	UINT8		    HPF_COR;
	UINT8          FILT_EXT_ENA;
	UINT8          FILT_EXT_WIDEST;
	UINT8          FILT_EXT_POINT;
	UINT8          LNA_DET_MODE; 
	UINT8          RF_DISCHARGE;
	UINT8          NA_DISCHARGE;
	UINT8          AGC_CLK;
	UINT8		    FILT_COMP;
	UINT8		    FILT_CUR;  
	UINT8		    FILT_3DB; 
	UINT8		    SWBUF_CUR;  
	UINT8          TF_CUR;              
	UINT8		    INDUC_BIAS;  
	UINT8          SWCAP_CLK;
	UINT8		    NA_PWR_DET;  
	UINT16		IF_KHz;
	UINT16	    FILT_CAL_IF;
}Sys_Info_Type;

typedef struct _Freq_Info_Type
{
	UINT8		RF_POLY;
	UINT8		LNA_BAND;
	UINT8		LPF_CAP;
	UINT8		LPF_NOTCH;
    UINT8      XTAL_POW0;
	UINT8      CP_CUR;
	UINT8		IMR_MEM;
	UINT8      Q_CTRL;   
}Freq_Info_Type;

typedef struct _SysFreq_Info_Type
{
	UINT8		LNA_TOP;
	UINT8		LNA_VTH_L;
	UINT8		MIXER_TOP;
	UINT8		MIXER_VTH_L;
	UINT8      RF_TOP;
	UINT8      NRB_TOP;
	UINT8      NRB_BW;
	UINT8      BYP_LPF;
}SysFreq_Info_Type;

typedef struct _R840_Cal_Info_Type
{
	UINT8		FILTER_6DB;
	UINT8		MIXER_AMP_GAIN;
	UINT8		MIXER_BUFFER_GAIN;
	UINT8		LNA_GAIN;
	UINT8		LNA_POWER;
	UINT8		RFBUF_OUT;
}R840_Cal_Info_Type;

typedef struct _R840_SectType
{
	UINT8   Phase_Y;
	UINT8   Gain_X;
	UINT8   Iqcap;
	UINT8   Value;
}R840_SectType;

typedef struct _R840_TF_Result
{
	UINT8   TF_Set;
	UINT8   TF_Value;
}R840_TF_Result;

typedef enum _R840_TF_Band_Type
{
    TF_HIGH = 0,
	TF_MID,
	TF_LOW
}R840_TF_Band_Type;

typedef enum _R840_TF_Type
{
	R840_TF_DVBC_CHINA = 0,     //NC/82n     (china DVB-C)
	R840_TF_NARROW,                  //270n/68n   (ATSC, ISDB-T, DVB-T/T2)
    R840_TF_BEAD,                        //Bead/68n   (DTMB)
	R840_TF_NARROW_LIN,           //270n/68n   (DVB-C, J83B)
	R840_TF_NARROW_ATV_LIN,  //270n/68n   (ATV)
	R840_TF_BEAD_LIN,                 //Bead/68n   (PAL_DK for China Hybrid TV)
	R840_TF_SIZE
}R840_TF_Type;

typedef enum _R840_Cal_Type
{
	R840_IMR_CAL = 0,
	R840_IMR_LNA_CAL,
	R840_TF_CAL,
	R840_TF_LNA_CAL,
	R840_LPF_CAL,
	R840_LPF_LNA_CAL
}R840_Cal_Type;

typedef enum _R840_BW_Type
{
	BW_6M = 0,
	BW_7M,
	BW_8M,
	BW_1_7M,
	BW_10M,
	BW_200K
}R840_BW_Type;

enum XTAL_PWR_VALUE
{
	XTAL_NOR_LOWEST = 0,
    XTAL_NOR_LOW,
    XTAL_NOR_HIGH,
    XTAL_NOR_HIGHEST,
    XTAL_GM3V_HIGHEST,
    XTAL_GM2V_HIGHEST,
	XTAL_CHECK_SIZE
};

typedef enum _Rafael_Chip_Type  //Don't modify chip list
{
	R840_MP = 0,
	R840_MC1,
	R840_MC2,
	R836_MC2,
	R840_MC5,
	R836_MC5,
}Rafael_Chip_Type;

//----------------------------------------------------------//
//                   Type Define                                    //
//----------------------------------------------------------//
#define UINT8  unsigned char
#define UINT16 unsigned short
#define UINT32 unsigned long

//----------------------------------------------------------//
//                   R840 Public Parameter                     //
//----------------------------------------------------------//
typedef enum _R840_ErrCode
{
	RT_Success = 0,
	RT_Fail    = 1
}R840_ErrCode;

typedef enum _R840_Standard_Type  //Don't remove standand list!!
{
	R840_MN_5100 = 0,                     //for NTSC_MN, PAL_M (IF=5.1M)
	R840_MN_5800,                           //for NTSC_MN, PLA_M (IF=5.8M)
	R840_MN_CIF_5M,                      //for NTSC_MN, PLA_M (CIF=5.0M)
	R840_MN_W_DTMB,                   //for NTSC_MN in "MN+DTMB" hybrid TV (IF=5.8M)
	R840_MN_W_DTMB_CIF_5M,     //for NTSC_MN in "MN+DTMB" hybrid TV (CIF=5M)
	R840_PAL_I,                                //for PAL-I
	R840_PAL_I_CIF_5M,                  //for PAL-I (CIF=5.0M)
	R840_PAL_I_W_DTMB,               //for PAL-I in "PAL-I+DTMB" hybrid TV 
	R840_PAL_I_W_DTMB_CIF_5M, //for PAL-I in "PAL-I+DTMB" hybrid TV (CIF=5M)
	R840_PAL_DK,                            //for PAL DK in non-"DTMB+PAL DK" case
	R840_PAL_DK_CIF_5M,              //for PAL DK in non-"DTMB+PAL DK" case (CIF=5M)
	R840_PAL_DK_W_DTMB,               //for PAL DK in "PAL-DK+DTMB" hybrid TV
	R840_PAL_DK_W_DTMB_CIF_5M, //for PAL DK in "PAL-DK+DTMB" hybrid TV (CIF=5M)
	R840_PAL_B_7M,                                 //for PAL-B 7M
	R840_PAL_B_7M_CIF_5M,                   //for PAL-B 7M (CIF=5M)
	R840_PAL_B_7M_W_DTMB,                //for PAL-B 7M in "PAL-B+DTMB" hybrid TV
	R840_PAL_B_7M_W_DTMB_CIF_5M,  //for PAL-B 7M in "PAL-B+DTMB" hybrid TV (CIF=5M)
	R840_PAL_BGH_8M,                                //for PAL G/H 8M
	R840_PAL_BGH_8M_CIF_5M,                  //for PAL G/H 8M (CIF=5M)
	R840_PAL_BGH_8M_W_DTMB,               //for PAL-G/H 8M in "PAL-G/H+DTMB" hybrid TV
	R840_PAL_BGH_8M_W_DTMB_CIF_5M, //for PAL-G/H 8M in "PAL-G/H+DTMB" hybrid TV (CIF=5M)
	R840_SECAM_L,                                 //for SECAM L
	R840_SECAM_L_CIF_5M,                   //for SECAM L (CIF=5M)
	R840_SECAM_L_W_DTMB,                //for SECAM L in "SECAM L+DTMB" hybrid TV
	R840_SECAM_L_W_DTMB_CIF_5M,  //for SECAM L in "SECAM L+DTMB" hybrid TV (CIF=5M)
	R840_SECAM_L1,                        //for SECAM L'
	R840_SECAM_L1_CIF_5M,          //for SECAM L' (CIF=5M)
	R840_SECAM_L1_INV,       
	R840_SECAM_L1_INV_CIF_5M,        //(CIF=5M)
	R840_ATV_SIZE,
	R840_DVB_T_6M = R840_ATV_SIZE,
	R840_DVB_T_7M,
	R840_DVB_T_8M, 
    R840_DVB_T2_6M,       //IF=4.57M
	R840_DVB_T2_7M,       //IF=4.57M
	R840_DVB_T2_8M,       //IF=4.57M
	R840_DVB_T2_1_7M,
	R840_DVB_T2_10M,
	R840_DVB_C_CHINA,  //RF > 115MHz
	R840_DVB_C_8M,
	R840_DVB_C_8M_W_DTMB,
	R840_DVB_C_6M, 
	R840_J83B,
	R840_ISDB_T,             //IF=4.063M
	R840_DTMB_4570,      //IF=4.57M
	R840_DTMB_6000,      //IF=6.00M
	R840_DTMB_6M_BW, //IF=5.0M, BW=6M
	R840_ATSC,  
	R840_DVB_T_6M_IF_5M,
	R840_DVB_T_7M_IF_5M,
	R840_DVB_T_8M_IF_5M,
	R840_DVB_T2_6M_IF_5M,
	R840_DVB_T2_7M_IF_5M,
	R840_DVB_T2_8M_IF_5M,
	R840_DVB_T2_1_7M_IF_5M,
	R840_DVB_C_CHINA_IF_5M,  //RF > 115MHz
	R840_DVB_C_8M_IF_5M,
	R840_DVB_C_8M_W_DTMB_IF_5M,
	R840_DVB_C_6M_IF_5M, 
	R840_J83B_IF_5M,
	R840_ISDB_T_IF_5M,            
	R840_DTMB_IF_5M,     
	R840_ATSC_IF_5M,  
	R840_FM,
	R840_STD_SIZE,
}R840_Standard_Type;


typedef enum _R840_LoopThrough_Type
{
	LT_ON = 0,
	LT_OFF= 1
}R840_LoopThrough_Type;

typedef enum _R840_GPIO_Type
{
	HI_SIG = 0,
	LO_SIG = 1
}R840_GPIO_Type;

typedef enum _R840_ClkOutMode_Type
{
	CLK_OUT_OFF = 0,
	CLK_OUT_ON
}R840_ClkOutMode_Type;

typedef enum _R840_RF_Gain_TYPE
{
	RF_AUTO = 0,
	RF_MANUAL
}R840_RF_Gain_TYPE;

typedef enum _R840_Xtal_Div_TYPE
{
	XTAL_DIV1 = 0,
	XTAL_DIV2
}R840_Xtal_Div_TYPE;

typedef enum _R840_IfAgc_Type
{
	IF_AGC1 = 0,
	IF_AGC2
}R840_IfAgc_Type;

typedef struct _R840_Set_Info
{
	UINT32                              RF_KHz;
	R840_Standard_Type           R840_Standard;
	R840_LoopThrough_Type    R840_LT;
	R840_ClkOutMode_Type     R840_ClkOutMode;
	R840_IfAgc_Type               R840_IfAgc_Select; 
}R840_Set_Info;

typedef struct _R840_RF_Gain_Info
{
	UINT16   RF_gain_comb;
	UINT8   RF_gain1;
	UINT8   RF_gain2;
	UINT8   RF_gain3;
}R840_RF_Gain_Info;

//----------------------------------------------------------//
//                   R840 Public Function                       //
//----------------------------------------------------------//
//#define R840_Delay_MS	Sleep
void R840_Delay_MS(int ms);

typedef struct _I2C_LEN_TYPE
{
	UINT8 RegAddr;
	UINT8 Data[50];
	UINT8 Len;
}I2C_LEN_TYPE;

typedef struct _I2C_TYPE
{
	UINT8 RegAddr;
	UINT8 Data;
}I2C_TYPE;

R840_ErrCode R840_Init(void);
R840_ErrCode R840_SetPllData(R840_Set_Info R840_INFO);
R840_ErrCode R840_Standby(R840_LoopThrough_Type R840_LoopSwitch);
R840_ErrCode R840_GPIO(R840_GPIO_Type R840_GPIO_Conrl);
//R840_ErrCode R840_SetStandard(R840_Standard_Type RT_Standard);
//R840_ErrCode R840_SetFrequency(R840_Set_Info R840_INFO);
R840_ErrCode R840_GetRfGain(R840_RF_Gain_Info *pR840_rf_gain);
R840_ErrCode R840_RfGainMode(R840_RF_Gain_TYPE R840_RfGainType);


#endif
