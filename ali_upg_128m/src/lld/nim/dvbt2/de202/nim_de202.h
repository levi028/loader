#ifndef __LLD_NIM_M3327_H__
#define __LLD_NIM_M3327_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <api/libc/printf.h>

//*********************************************/

//MaxLinearDataTypes.h
/******************************************************************************
    Macros
******************************************************************************/
//#define __WINDOWS_PLATFORM__

// Macro for contitional compilation of code in driver
// This macro will be defined if there is a need to control Demod from driver
// for eg. running GraphEdit.
// if there is no need to allow control of demod from driver this macro will be
// disabled
//#define MXL_LIB_CTRL

//#define _TARGET_FPGA_V6_

//#define BIG_TO_LITTLE_16(ptr, offset)    ((ptr[offset++] << 8)| ptr[offset++])

//#define WRITE_OP  0
//#define READ_OP   1

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
//typedef unsigned char  UINT8;
//typedef unsigned short UINT16;
//typedef unsigned int   UINT32;
typedef char           SINT8;
typedef short          SINT16;
typedef int            SINT32;
typedef float          REAL32;
typedef double         REAL64;


#if 1//def __WINDOWS_PLATFORM__

typedef enum
{
  MXL_TRUE = 0,
  MXL_FALSE = 1,

} MXL_STATUS;

typedef enum
{
  MXL_DISABLE = 0,
  MXL_ENABLE,

  MXL_NO_FREEZE = 0,
  MXL_FREEZE,

  MXL_UNLOCKED = 0,
  MXL_LOCKED,

  MXL_OFF = 0,
  MXL_ON

} MXL_BOOL;

#else // __WINDOWS_PLATFORM__

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#ifdef BOOL
#undef BOOL
#endif

typedef enum
{
  TRUE = 1,
  FALSE = 0,

  PASSED = 1,
  FAILED = 0,

  GOOD = 1,
  BAD = 0,

  YES = 1,
  NO = 0,

  ENABLED = 1,
  DISABLED = 0,

  ON = 1,
  OFF = 0,

  SUCCESS = 1,
  FAIL = 0,

  VALID = 1,
  INVALID = 0,

  BUSY = 1,
  IDLE = 0,

} BOOL;
#endif  //  __WINDOWS_PLATFORM__

// The macro for memory-mapped register access

//#define HWD_REG32(addr)             (*(volatile UINT32*)addr)

//#define HWD_REG_WRITE32(addr, data) (*(volatile UINT32*)addr = (UINT32)data)
//#define HWD_REG_WRITE16(addr, data) (*(volatile UINT16*)addr = (UINT16)data)
//#define HWD_REG_WRITE08(addr, data) (*(volatile UINT8*)addr = (UINT8)data)

//#define HWD_REG_READ32(addr)        (*(volatile UINT32*)addr)
//#define HWD_REG_READ16(addr)        (*(volatile UINT16*)addr)
//#define HWD_REG_READ08(addr)        (*(volatile UINT8*)addr)

//#define PHY_REG32(addr)             (*(volatile UINT32*)addr)

//#define PHY_REG_WRITE32(addr, data) (*(volatile UINT32*)addr = (UINT32)data)
//#define PHY_REG_WRITE16(addr, data) (*(volatile UINT16*)addr = (UINT16)data)
//#define PHY_REG_WRITE08(addr, data) (*(volatile UINT8*)addr = (UINT8)data)

//#define PHY_REG_READ32(addr)        (*(volatile UINT32*)addr)
//#define PHY_REG_READ16(addr)        (*(volatile UINT16*)addr)
//#define PHY_REG_READ08(addr)        (*(volatile UINT8*)addr)

//MaxLinearDataTypes.h end


//cxd2820SF_OEM_Drv.h
MXL_STATUS Ctrl_WriteRegister(UINT32 tuner_id,UINT8 regAddr, UINT8 regData);
MXL_STATUS Ctrl_ReadRegister(UINT32 tuner_id,UINT8 regAddr, UINT8 *dataPtr);
MXL_STATUS Ctrl_Sleep(UINT16 TimeinMilliseconds);
MXL_STATUS Ctrl_GetTime(UINT32 *TimeinMilliseconds);
//cxd2820SF_OEM_Drv.h end


//cxd2820SF_PhyCtrlApi.h
/******************************************************************************
    Macros
******************************************************************************/

#define CALCULATE_SNR(data)        ((1563 * data) - 25000)
#define CALCULATE_BER(avg_errors, count)  (UINT32)(avg_errors * 4 * 3247)

#define MXL_TUNER_MODE         0
#define MXL_SOC_MODE           1
#define MXL_DEV_MODE_MASK      0x01

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/

/* Command Types  */
typedef enum
{
  // MxL Device configure command
  MXL_DEV_SOFT_RESET_CFG = 0,
  MXL_DEV_XTAL_SETTINGS_CFG,
  MXL_DEV_ID_VERSION_REQ,
  MXL_DEV_OPERATIONAL_MODE_CFG,
  MXL_DEV_GPO_PINS_CFG,
  MXL_DEV_CABLE_CFG,

  // cxd2820SF Config specific commands
  MXL_DEV_101SF_OVERWRITE_DEFAULTS_CFG,
  MXL_DEV_101SF_POWER_MODE_CFG,
  MXL_DEV_MPEG_OUT_CFG,
  MXL_DEV_TS_CTRL_CFG,

  // Reset/Clear Demod status
  MXL_DEMOD_RESET_IRQ_CFG,
  MXL_DEMOD_RESET_PEC_CFG,
  MXL_DEMOD_TS_PRIORITY_CFG,

  // Demod Config commands
  MXL_DEMOD_SPECTRUM_CFG,
  MXL_DEMOD_RS_PKT_THRESH_CFG,

  // Demod Status and Info command
  MXL_DEMOD_SNR_REQ,
  MXL_DEMOD_BER_REQ,
  MXL_DEMOD_TPS_CODE_RATE_REQ,
  MXL_DEMOD_TPS_HIERARCHY_REQ,
  MXL_DEMOD_TPS_CONSTELLATION_REQ,
  MXL_DEMOD_TPS_FFT_MODE_REQ,
  MXL_DEMOD_TPS_HIERARCHICAL_ALPHA_REQ,
  MXL_DEMOD_TPS_GUARD_INTERVAL_REQ,
  MXL_DEMOD_TPS_CELL_ID_REQ,
  MXL_DEMOD_TPS_LOCK_REQ,
  MXL_DEMOD_PACKET_ERROR_COUNT_REQ,
  MXL_DEMOD_SYNC_LOCK_REQ,
  MXL_DEMOD_RS_LOCK_REQ,
  MXL_DEMOD_CP_LOCK_REQ,
  MXL_DEMOD_FEC_LOCK_REQ,

  // Tuner Config commands
  MXL_TUNER_TOP_MASTER_CFG,
  MXL_TUNER_CHAN_TUNE_CFG,
  MXL_TUNER_IF_OUTPUT_FREQ_CFG,
  //MXL_TUNER_CHAN_TUNE_EXT_CFG,

  // Tuner Status and Info command
  MXL_TUNER_LOCK_STATUS_REQ,
  MXL_TUNER_SIGNAL_STRENGTH_REQ,

  // Channel Scan commands
  MXL_TUNER_CHAN_SCAN_CFG,
  MXL_TUNER_CHECK_CHAN_STATUS_REQ,
  MXL_TUNER_CHAN_OFFSET_REQ,
  MXL_TUNER_FREQ_OFFSET_SRCH_RANGE_CFG,

  MAX_NUM_CMD
} MXL_CMD_TYPE_E;

// MXL_DEV_ID_VERSION_REQ
typedef struct
{
  UINT8 DevId;                    /* OUT, Device Id of cxd2820SF Device */
  UINT8 DevVer;                   /* OUT, Device Version of cxd2820SF Device */
} MXL_DEV_INFO_T, *PMXL_DEV_INFO_T;

// MXL_DEV_POWER_MODE_CFG
typedef enum
{
  STANDBY_ON = 0,
  SLEEP_ON,
  STANDBY_OFF,
  SLEEP_OFF,
} MXL_PWR_MODE_E;

typedef struct
{
  MXL_PWR_MODE_E PowerMode;       /* IN, Power mode for cxd2820SF Device */
} MXL_PWR_MODE_CFG_T, *PMXL_PWR_MODE_CFG_T;

// MXL_TUNER_TOP_MASTER_CFG
typedef struct
{
  MXL_BOOL  TopMasterEnable;      /* IN, Enable or Disable cxd2820SF Tuner */
}MXL_TOP_MASTER_CFG_T, *PMXL_TOP_MASTER_CFG_T;

// MXL_DEV_OPERATIONAL_MODE_CFG
typedef struct
{
  UINT8 DeviceMode;             /* IN, Operational mode of cxd2820SF */
} MXL_DEV_MODE_CFG_T, *PMXL_DEV_MODE_CFG_T;

// MXL_DEMOD_SNR_REQ
typedef struct
{
  UINT32 SNR;                   /* OUT, SNR data from cxd2820SF */
} MXL_DEMOD_SNR_INFO_T, *PMXL_DEMOD_SNR_INFO_T;

// MXL_DEMOD_BER_REQ
typedef struct
{
  UINT32 BER;                 /* OUT, BER data from cxd2820SF */
} MXL_DEMOD_BER_INFO_T, *PMXL_DEMOD_BER_INFO_T;

/* MXL_DEMOD_PACKET_ERROR_COUNT_REQ */
typedef struct
{
  UINT32 PEC;               /* OUT, PEC data from cxd2820SF */
}MXL_DEMOD_PEC_INFO_T, *PMXL_DEMOD_PEC_INFO_T;

// MXL_TUNER_CHAN_TUNE_CFG
typedef struct
{
  UINT32 Frequency;                       /* IN, Frequency in MHz */
  UINT8 Bandwidth;                        /* IN, Channel Bandwidth in MHz */
  MXL_BOOL TpsCellIdRbCtrl;               /* IN, Enable TPS Cell ID Read Back feature */
} MXL_RF_TUNE_CFG_T, *PMXL_RF_TUNE_CFG_T;

// Tune RF Lock Status MXL_TUNER_LOCK_STATUS_REQ
typedef struct
{
  MXL_BOOL RfSynthLock;               /* OUT, RF SYNT Lock status of cxd2820SF Tuner */
  MXL_BOOL RefSynthLock;              /* OUT, REF SYNT Lock status of cxd2820SF Tuner */
} MXL_TUNER_LOCK_STATUS_T, *PMXL_TUNER_LOCK_STATUS_T;

// MXL_DEMOD_TPS_CONSTELLATION_REQ
// MXL_DEMOD_TPS_CODE_RATE_REQ
// MXL_DEMOD_TPS_FFT_MODE_REQ
// MXL_DEMOD_TPS_GUARD_INTERVAL_REQ
// MXL_DEMOD_TPS_HIERARCHY_REQ
typedef struct
{
  UINT8 TpsInfo;                      /* OUT, TPS data for respective TPS command */
}MXL_DEMOD_TPS_INFO_T, *PMXL_DEMOD_TPS_INFO_T;

// MXL_DEMOD_TPS_LOCK_REQ
// MXL_DEMOD_RS_LOCK_REQ
// MXL_DEMOD_CP_LOCK_REQ
// MXL_DEMOD_RS_LOCK_REQ
typedef struct
{
  MXL_BOOL Status;                  /* OUT, Lock status of cxd2820SF */
} MXL_DEMOD_LOCK_STATUS_T, *PMXL_DEMOD_LOCK_STATUS_T;

// MXL_DEMOD_TS_PRIORITY_CFG
typedef enum
{
  HP_STREAM = 0,
  LP_STREAM,
}MXL_HPORLP_E;

typedef struct
{
  MXL_HPORLP_E StreamPriority;         /* IN,  Value to select transport stream priority*/
} MXL_DEMOD_TS_PRIORITY_CFG_T, *PMXL_DEMOD_TS_PRIORITY_CFG_T;

// MXL_TUNER_IF_OUTPUT_FREQ_CFG
typedef enum
{
  IF_OTHER_12MHZ = 0,
  IF_4_0MHZ,
  IF_4_5MHZ,
  IF_4_57MHZ,
  IF_5_0MHZ,
  IF_5_38MHZ,
  IF_6_0MHZ,
  IF_6_28MHZ,
  IF_7_2MHZ,
  IF_35_250MHZ,
  IF_36_0MHZ,
  IF_36_15MHZ,
  IF_44_0MHZ,
  IF_OTHER_35MHZ_45MHZ = 0x0F,
} MXL_IF_FREQ_E;

typedef struct
{
  MXL_IF_FREQ_E   IF_Index;               /* IN, Index for predefined IF frequency */
  UINT8           IF_Polarity;            /* IN, IF Spectrum - Normal or Inverted */
  UINT32          IF_Freq;                /* IN, IF Frequency in Hz for non-predefined frequencies */
} MXL_TUNER_IF_FREQ_T, *PMXL_TUNER_IF_FREQ_T;

// MXL_DEV_XTAL_SETTINGS_CFG
typedef enum
{
  XTAL_24MHz = 4,
  XTAL_27MHz = 7,
  XTAL_28_8MHz = 8,
  XTAL_48MHz = 12,
  XTAL_NA = 13
} MXL_XTAL_FREQ_E;

typedef enum
{
  I123_5uA = 0,
  I238_8uA,
  I351_4uA,
  I462_4uA,
  I572_0uA,
  I680_4uA,
  I787_8uA,
  I894_2uA,
  XTAL_BIAS_NA
} MXL_XTAL_BIAS_E;

typedef enum
{
  CLK_OUT_0dB = 0,
  CLK_OUT_1_25dB,
  CLK_OUT_2_50dB,
  CLK_OUT_3_75dB,
  CLK_OUT_5_00dB,
  CLK_OUT_6_25dB,
  CLK_OUT_7_50dB,
  CLK_OUT_8_75dB,
  CLK_OUT_10_0dB,
  CLK_OUT_11_25dB,
  CLK_OUT_12_50dB,
  CLK_OUT_13_75dB,
  CLK_OUT_15_00dB,
  CLK_OUT_16_25dB,
  CLK_OUT_17_50dB,
  CLK_OUT_18_75dB,
  CLK_OUT_NA
} MXL_XTAL_CLK_OUT_GAIN_E;

typedef struct
{
  MXL_XTAL_FREQ_E           XtalFreq;             /* IN, XTAL frequency */
  MXL_XTAL_BIAS_E           XtalBiasCurrent;      /* IN, XTAL Bias current */
  UINT8                     XtalCap;              /* IN, XTAL Capacitance value */
  MXL_BOOL                  XtalClkOutEnable;     /* IN, XTAL Clock out control */
  MXL_XTAL_CLK_OUT_GAIN_E   XtalClkOutGain;       /* IN, XTAL Clock out gain value */
  MXL_BOOL                  LoopThruEnable;       /* IN, Frequency loop thru */
} MXL_XTAL_CFG_T, *PMXL_XTAL_CFG_T;

// MXL_DEMOD_MPEG_OUT_CFG
typedef enum
{
  MPEG_CLOCK_36_571429MHz = 0,
  MPEG_CLOCK_2_285714MHz,
  MPEG_CLOCK_4_571429MHz,
  MPEG_CLOCK_6_857143MHz,
  MPEG_CLOCK_9_142857MHz,
  MPEG_CLOCK_13_714286MHz,
  MPEG_CLOCK_18_285714MHz,
  MPEG_CLOCK_27_428571MHz
} MPEG_CLOCK_FREQ_E;

typedef enum
{
  MPEG_ACTIVE_LOW = 0,
  MPEG_ACTIVE_HIGH,

  MPEG_CLK_POSITIVE  = 0,
  MPEG_CLK_NEGATIVE,

  MPEG_CLK_IN_PHASE = 0,
  MPEG_CLK_INVERTED,

  MPEG_CLK_EXTERNAL = 0,
  MPEG_CLK_INTERNAL
} MPEG_CLK_FMT_E;

typedef enum
{
  MPEG_SERIAL_LSB_1ST = 0,
  MPEG_SERIAL_MSB_1ST,

  MPEG_DATA_SERIAL  = 0,
  MPEG_DATA_PARALLEL,

  MPEG_SYNC_WIDTH_BIT = 0,
  MPEG_SYNC_WIDTH_BYTE
} MPEG_DATA_FMT_E;

typedef struct
{
  MPEG_DATA_FMT_E   SerialOrPar;             /* IN, Serial or Parallel mode */
  MPEG_DATA_FMT_E   LsbOrMsbFirst;           /* IN, Serial mode MSB or LSB first */
  MPEG_CLOCK_FREQ_E MpegClkFreq;             /* IN, MPEG Clock frequency */
  MPEG_CLK_FMT_E    MpegValidPol;            /* IN, MPEG Valid polarity */
  MPEG_CLK_FMT_E    MpegClkPhase;            /* IN, MPEG Clock phase */
  MPEG_CLK_FMT_E    MpegSyncPol;             /* IN, MPEG SYNC Polarity */
} MXL_MPEG_CFG_T, *PMXL_MPEG_CFG_T;

// MXL_DEV_TS_CTRL_CFG
typedef enum
{
  TS_DRIVE_STRENGTH_DEFAULT = 0,
  TS_DRIVE_STRENGTH_1X = 0,
  TS_DRIVE_STRENGTH_2X
} MXL_TS_DRIVE_STRENGTH_E;

typedef struct
{
  MXL_BOOL                    TsCtrl;             /* IN, Control to turn TS ON or OFF  */
  MXL_TS_DRIVE_STRENGTH_E     TsDriveStrength;    /* IN, Control MPEG drive strength */
} MXL_TS_CTRL_T, *PMXL_TS_CTRL_T;

// MXL_DEV_GPO_PINS_CFG
typedef enum
{
  MXL_GPO_0 = 0,
  MXL_GPO_1,
  MXL_GPO_NA
} MXL_GPO_E;

typedef struct
{
  MXL_GPO_E           GpoPinId;           /* IN, GPIO Pin ID */
  MXL_BOOL            GpoPinCtrl;         /* IN, GPIO Pin control */
} MXL_DEV_GPO_CFG_T, *PMXL_DEV_GPO_CFG_T;

// MXL_TUNER_SIGNAL_STRENGTH_REQ
typedef struct
{
  SINT32 SignalStrength;                    /* OUT, Tuner Signal strength in dBm */
} MXL_SIGNAL_STATS_T, *PMXL_SIGNAL_STATS_T;

// MXL_DEMOD_TPS_CELL_ID_REQ
typedef struct
{
  UINT16 TpsCellId;                      /* OUT, TPS Cell ID Info */
}MXL_DEMOD_CELL_ID_INFO_T, *PMXL_DEMOD_CELL_ID_INFO_T;

// MXL_TUNER_CHECK_CHAN_STATUS_REQ
typedef enum
{
  MXL_BREAK_AT_CP_LOCK = 1,       /* Break after CP Lock */
  MXL_BREAK_AT_TPS_LOCK,          /* Break after TPS Lock */
  MXL_BREAK_AT_RS_LOCK,           /* Break after RS Lock */
  MXL_FULL_MODE                   /* Default mode, break after FEC lcok */
}MXL_CHAN_SCAN_CTRL_E;

typedef struct
{
  MXL_CHAN_SCAN_CTRL_E ChanScanCtrl;   /* IN, value of Highest lock level check for a frequency */
  UINT32 Frequency;                    /* IN, Frequency in MHz */
  UINT8 Bandwidth;                     /* IN, Channel Bandwidth in MHz */
  MXL_BOOL TpsCellIdRbCtrl;            /* IN, Enable TPS Cell ID Read Back feature */
  //MXL_BOOL IncreaseMaxTempRange;       /* IN, Enable or disable Max temp range while tuning */
  MXL_STATUS ChanPresent;              /* OUT, Indicates presence of a frequency */
} MXL_TUNER_CHECK_CHAN_REQ_T, *PMXL_TUNER_CHECK_CHAN_REQ_T;

// MXL_TUNER_CHAN_OFFSET_REQ
typedef struct
{
  SINT32 ChanOffset;                      /* OUT, Frequency Offset for a Channel in Hz*/
}MXL_TUNER_CHAN_OFFSET_T, *PMXL_TUNER_CHAN_OFFSET_T;

// MXL_TUNER_CHAN_SCAN_CFG
typedef struct
{
  MXL_BOOL ChanScanCfg;                      /* IN, Enable or Disable Channel Scan */
}MXL_TUNER_CHAN_SCAN_CFG_T, *PMXL_TUNER_CHAN_SCAN_CFG_T;

// MXL_DEMOD_SPECTRUM_CFG
typedef struct
{
  MXL_BOOL SpectrumCfg;                      /* IN, Enable or Disable inverted signal processing */
}MXL_DEMOD_SPECTRUM_CFG_T, *PMXL_DEMOD_SPECTRUM_CFG_T;

// MXL_TUNER_FREQ_OFFSET_SRCH_RANGE_CFG
typedef struct
{
  MXL_BOOL MaxFreqOffsetRangeCfg;   /* IN, Enable or Disable Maximum frequency offset search range */
}MXL_TUNER_FREQ_OFFSET_CFG_T, *PMXL_TUNER_FREQ_OFFSET_CFG_T;

// MXL_DEMOD_RS_PKT_THRESH_CFG
typedef struct
{
  UINT8 RsPktThresh;                      /* IN, RS Pkt threshold for demod */
}MXL_DEMOD_RS_PKT_THRESH_T, *PMXL_DEMOD_RS_PKT_THRESH_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/

/******************************************************************************
    Prototypes
******************************************************************************/
#if 0
MXL_STATUS MxLWare_API_ConfigDevice(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetDeviceStatus(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_ConfigDemod(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetDemodStatus(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_ConfigTuner(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetTunerStatus(UINT32 tuner_id,MXL_CMD_TYPE_E CmdType, void *ParamPtr);
#endif
//cxd2820SF_PhyCtrlApi.h end


//cxd2820SF_PhyDefs.h
/******************************************************************************
    Macros
******************************************************************************/

#define CHIP_ID_REG                  0xFC
#define TOP_CHIP_REV_ID_REG          0xFA

#define V6_SNR_RB_LSB_REG            0x27
#define V6_SNR_RB_MSB_REG            0x28

#define V6_N_ACCUMULATE_REG          0x11
#define V6_RS_AVG_ERRORS_LSB_REG     0x2C
#define V6_RS_AVG_ERRORS_MSB_REG     0x2D

#define V6_IRQ_STATUS_REG            0x24
#define  IRQ_MASK_FEC_LOCK       0x10

#define V6_SYNC_LOCK_REG             0x28
#define SYNC_LOCK_MASK           0x10

#define V6_RS_LOCK_DET_REG           0x28
#define  RS_LOCK_DET_MASK        0x08

#define V6_INITACQ_NODETECT_REG    0x20
#define V6_FORCE_NFFT_CPSIZE_REG   0x20

#define V6_CODE_RATE_TPS_REG       0x29
#define V6_CODE_RATE_TPS_MASK      0x07

#define V6_CP_LOCK_DET_REG        0x28
#define V6_CP_LOCK_DET_MASK       0x04

#define V6_TPS_HIERACHY_REG        0x29
#define V6_TPS_HIERARCHY_INFO_MASK  0x40

#define V6_TPS_HPORLP_REG        0x20
#define V6_TPS_HPORLP_INFO_MASK  0x80

#define V6_MODORDER_TPS_REG        0x2A
#define V6_PARAM_CONSTELLATION_MASK   0x30

#define V6_TPS_ALPHA_REG        0x2A
#define V6_TPS_ALPHA_MASK   0x03

#define V6_MODE_TPS_REG            0x2A
#define V6_PARAM_FFT_MODE_MASK        0x0C

#define V6_CP_TPS_REG             0x29
#define V6_PARAM_GI_MASK              0x30

#define V6_AGC_LOCK_REG             0x26
#define V6_AGC_LOCK_MASK              0x20

#define V6_TPS_LOCK_REG           0x2A
#define V6_PARAM_TPS_LOCK_MASK        0x40

#define V6_FEC_PER_COUNT_REG      0x2E
#define V6_FEC_PER_SCALE_REG      0x2B
#define V6_FEC_PER_SCALE_MASK        0x03

#define V6_PIN_MUX_MODE_REG       0x1B
#define V6_ENABLE_PIN_MUX            0x1E

#define V6_MPEG_OUT_CLK_INV_REG    0x17
#define V6_MPEG_OUT_CTRL_REG       0x18

#define V6_INVERTED_CLK_PHASE       0x20
#define V6_MPEG_DATA_PARALLEL    0x01
#define V6_MPEG_DATA_SERIAL      0x02

#define V6_INVERTED_MPEG_SYNC       0x04
#define V6_INVERTED_MPEG_VALID      0x08

#define V6_MPEG_INOUT_BIT_ORDER_CTRL_REG  0x19
#define V6_MPEG_SER_MSB_FIRST                0x80
#define MPEG_SER_MSB_FIRST_ENABLED        0x01

#define V6_TUNER_LOOP_THRU_CONTROL_REG  0x09
#define V6_ENABLE_LOOP_THRU               0x01

#define TOTAL_NUM_IF_OUTPUT_FREQ       16

#define TUNER_NORMAL_IF_SPECTRUM       0x0
#define TUNER_INVERT_IF_SPECTRUM       0x10

#define V6_TUNER_IF_SEL_REG              0x06
#define V6_TUNER_IF_FCW_REG              0x3C
#define V6_TUNER_IF_FCW_BYP_REG          0x3D
#define V6_RF_LOCK_STATUS_REG            0x23

#define V6_DIG_CLK_FREQ_SEL_REG  0x07
#define V6_REF_SYNTH_INT_REG     0x5C
#define V6_REF_SYNTH_REMAIN_REG  0x58
#define V6_DIG_RFREFSELECT_REG   0x32
#define V6_XTAL_CLK_OUT_GAIN_REG   0x31
#define V6_TUNER_LOOP_THRU_CTRL_REG      0x09
#define V6_DIG_XTAL_ENABLE_REG  0x06
#define V6_DIG_XTAL_BIAS_REG  0x66
#define V6_XTAL_CAP_REG    0x08

#define V6_GPO_CTRL_REG     0x18
#define V6_GPO_0_MASK       0x10
#define V6_GPO_1_MASK       0x20

#define MXL_MODE_REG    0x03
#define START_TUNE_REG  0x1C

#define V6_DIG_RF_PWR_LSB_REG   0x46
#define V6_DIG_RF_PWR_MSB_REG   0x47

#define V6_AGC_LOCK_REG             0x26
#define V6_AGC_LOCK_MASK              0x20

#define V6_TPS_CELL_ID_LSB_REG  0x98
#define V6_TPS_CELL_ID_MSB_REG  0x99

#define V6_FREQ_OFFSET_LSB_REG  0x9D
#define V6_FREQ_OFFSET_MSB_REG  0x9E

#define V6_SPECTRUM_CTRL_REG  0x8D
#define V6_SPECTRUM_MASK  0x01

#define FREQ_OFFSET_SRCH_RANGE_REG  0xEA

#define RS_PKT_THRESH_REG  0xE4

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/

typedef struct
{
  UINT8 regAddr;
  UINT8 mask;
  UINT8 data;
} REG_CTRL_INFO_T, *PREG_CTRL_INFO_T;

typedef struct
{
  UINT32 StartTime;
  UINT32 AgcGain;
  UINT16 Timeout;
  UINT8 TpsCellId;
  UINT8 ChanConfigStatus;
}MXL_TUNER_CHAN_CONFIG_T, *PMXL_TUNER_CHAN_CONFIG_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/

extern REG_CTRL_INFO_T MxL_101OverwriteDefault[];
extern REG_CTRL_INFO_T MxL_101SuspendMode[];
extern REG_CTRL_INFO_T MxL_101WakeUp[];
extern REG_CTRL_INFO_T MxL_MpegDataOutToTSIF[];
extern REG_CTRL_INFO_T MxL_MpegDataIn[];
extern REG_CTRL_INFO_T MxL_PhySoftReset[];
extern REG_CTRL_INFO_T MxL_TunerDemodMode[];
extern REG_CTRL_INFO_T MxL_TunerMode[];
extern REG_CTRL_INFO_T MxL_TopMasterEnable[];
extern REG_CTRL_INFO_T MxL_TopMasterDisable[];
extern REG_CTRL_INFO_T MxL_IrqClear[];
extern REG_CTRL_INFO_T MxL_ResetPerCount[];
extern REG_CTRL_INFO_T MxL_CableSettings[];
extern REG_CTRL_INFO_T MxL_EnableCellId[];
extern REG_CTRL_INFO_T MxL_DisableCellId[];
extern REG_CTRL_INFO_T MxL_EnableChanScan[];
extern REG_CTRL_INFO_T MxL_DisableChanScan[];
extern REG_CTRL_INFO_T MxL_StartTune[];
extern REG_CTRL_INFO_T MxL_EnableHighTempRange[];
extern REG_CTRL_INFO_T MxL_DisableHighTempRange[];
extern REG_CTRL_INFO_T MxL_Config6MHzBandwidth[];
extern REG_CTRL_INFO_T MxL_Config7MHzBandwidth[];

/******************************************************************************
    Prototypes
******************************************************************************/
MXL_STATUS Ctrl_ProgramRegisters(UINT32 tuner_id, PREG_CTRL_INFO_T ctrlRegInfoPtr);
PREG_CTRL_INFO_T Ctrl_PhyTune(UINT32 frequency, UINT8 bandwidth);

// cxd2820SF_OEM_Drv.h specific APIs
MXL_STATUS Ctrl_WriteRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 regData);

MXL_STATUS Ctrl_ReadRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 *dataPtr);


MXL_STATUS Ctrl_Sleep(UINT16 TimeinMilliseconds);
MXL_STATUS Ctrl_GetTime(UINT32 *TimeinMilliseconds);

//cxd2820SF_PhyDefs.h end
/********************************************/
//#define NIM_M3327_BASE_IO_ADR  0xB8003000
/*STATUS_3 TUNER_STATUS*/
#define de202_TUNER_LOCK_FLAG        0x40


/*INDEX*/
#define LOCK_OK               11
#define FEC_UNLOCK             12
#define SCAN_END             13
#define SCAN_TIMEOUT        14
//#define TPS_UNLOCK             19
#define TUNER_I2C_FAIL        21
#define TUNER_NO_LOCK        22


/* other defines*/
#define TNIM_WAIT_LOCKUP        500 /* millseconds to wait for channel lock*/
#define TNIM_SCANWAIT_TPS      100 /* milliseconds to wait for valid TPS in sccan*/


/* de202_TPS*/
#define de202_HP 0 << 15

#define de202_FEC_1_2   0 << 7
#define de202_FEC_2_3   1 << 7
#define de202_FEC_3_4   2 << 7
#define de202_FEC_5_6   3 << 7
#define de202_FEC_7_8   4 << 7

#define de202_GUARD_1_32 0 <<2
#define de202_GUARD_1_16 1 <<2
#define de202_GUARD_1_8  2 <<2
#define de202_GUARD_1_4  3 <<2

#define de202_MODE_2K    0
#define de202_MODE_8K   1

#define  de202_MODU_QPSK  0  <<13
#define  de202_MODU_16QAM 1 <<13
#define  de202_MODU_64QAM 2 <<13

/*Get Imfom*/
#define FE_1_2   0
#define FE_2_3   1
#define FE_3_4   2
#define FE_5_6   3
#define FE_7_8   4

#define FEC_1_2   0
#define FEC_2_3   1
#define FEC_3_4   2
#define FEC_5_6   3
#define FEC_7_8   4

#define guard_1_32 0x20
#define guard_1_16 0x10
#define guard_1_8  0x08
#define guard_1_4  0x04

#define GUARD_1_32 0x20
#define GUARD_1_16 0x10
#define GUARD_1_8  0x08
#define GUARD_1_4  0x04

#define MODE_2K    0x02
#define MODE_8K 0x08

#define  TPS_CONST_QPSK  0x04
#define  TPS_CONST_16QAM 0x10
#define  TPS_CONST_64QAM 0x40

struct nim_de202_private
{
    struct COFDM_TUNER_CONFIG_API Tuner_Control;
    UINT32 tuner_id;
};

//sony_dvb_result_t  f_de202_read(UINT32 tuner_id, UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len);
//sony_dvb_result_t  f_de202_write(UINT32 tuner_id, UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len);
INT32 f_de202_attach();
static INT32 f_de202_open(struct nim_device *dev);
static INT32 f_de202_close(struct nim_device *dev);
static INT32 f_de202_disable(struct nim_device *dev);//Sam
static INT32 f_de202_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority);
static INT32 f_de202_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority);
static INT32 f_de202_get_lock(struct nim_device *dev, UINT8 *lock);
static INT32 f_de202_get_freq(struct nim_device *dev, UINT32 *freq);
static INT32 f_de202_get_code_rate(struct nim_device *dev, UINT8* code_rate);
static INT32 f_de202_get_AGC(struct nim_device *dev, UINT16 *agc);
static INT32 f_de202_get_SNR(struct nim_device *dev, UINT8 *snr);
static INT32 f_de202_get_BER(struct nim_device *dev, UINT32 *RsUbc);
static INT32 f_de202_get_GI(struct nim_device *dev, UINT8 *guard_interval);
static INT32 f_de202_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
static INT32 f_de202_get_modulation(struct nim_device *dev, UINT8 *modulation);
static INT32 f_de202_get_bandwidth(struct nim_device *dev, UINT8 *bandwidth);
static INT32 f_de202_get_specinv(struct nim_device *dev, UINT8 *Inv);
static INT32 f_de202_i2c_bypass(struct nim_device *dev);
static INT32 f_de202_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);    //50622-01Angus


static INT32 f_de202_init_reg(struct nim_device *dev);//051124 yuchun
static INT32 f_de202_freq_offset(struct nim_device *dev, UINT32 bandwidth, UINT8 fft_mode, INT32 *freq_offset);//051124 yuchun
static INT32 f_de202_getinfo(struct nim_device *dev, UINT8 *code_rate, UINT8 *guard_interval, UINT8 *fft_mode, UINT8 *modulation);//051124 yuchun



static INT32 f_de202_get_freq_offset(struct nim_device *dev, INT32 *freq_offset);//051222 yuchun
void f_de202_PassThrough(struct nim_device *dev,BOOL Open);

INT8 f_de202_priority(struct nim_device *dev, UINT8*priority);
static INT32 f_de202_get_hier_mode(struct nim_device *dev, UINT8*hier);
static INT32 f_de202_hier_mode(struct nim_device *dev);


struct de202_Lock_Info
{
        UINT8    FECRates;
        UINT8    HPRates;
        UINT8    LPRates;
        UINT8    Modulation;
        UINT8    Mode;            /* (ter) */
        UINT8    Guard;           /* (ter) */
        UINT8    Force;           /* (ter) */
        UINT8    Hierarchy;       /* (ter) */
        UINT8    Spectrum;
        UINT8    ChannelBW;       /* (ter) */
        UINT8    TrlTunning;
        UINT32    Frequency;
        INT8    EchoPos;         /* (ter) */
    INT32    FreqOffset;  /* (ter) */
    UINT32 ADC_CLK;
    UINT32 FEC_CLK;
     UINT8    Priority;
     UINT8    lock_status;
     UINT16 sig_strength;
     UINT16 sig_quality;

};
struct de202_Demod_Info
{
     UINT8 best_EPQ_val;
     UINT8 bad_EPQ_val;
     INT8  best_EPQ_val_idx;
     UINT8 EPQ_ref;
     UINT8 I2CSpeed;
     UINT8 past_EPQ_val[8];
     UINT8 EPQ_val[16];
     INT8  L1s2va3vp4;

};


//usage_type//051124 yuchun
#define MODE_AUTOSCAN        0x00
#define MODE_CHANSCAN        0x01
#define MODE_CHANCHG        0x02
#define MODE_AERIALTUNE    0x03

//***********HIERARCHY********//060331 Sam
#define TPS_LPSEL 0x80
#define LPSEL  0x02
#define TPS_HIERMODE 0x1c
#define HIER_NONE 0
#define HIER_1  1
#define HIER_2  2
#define HIER_4  4


#define de202_FECSTAT_LOCKED        0x30
#define de202_LOGIC_ENABLE        0x01

/* LOCK IMFO   */
#define de202_TUNER_LOCK_FLAG              0x40
#define de202_SYMBOL_LOCK_FLAG        0x02
#define de202_OFDM_LOCK_FLAG              0x03
#define de202_FEC_LOCK_FLAG                  0x30
/*set OFDM_WAIT_TIME    */
#define de202_OFDM_8K_WAIT               0x40

#endif    /* __LLD_NIM_M3327_H__ */

