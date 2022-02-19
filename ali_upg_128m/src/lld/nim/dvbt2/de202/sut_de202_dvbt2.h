/******************************************************************************
 *   $ Copyright $ Sony Corporation
 *-----------------------------------------------------------------------------
 *   File Name   : $File: sut_de202_dvbt2_vXX.c $
 *   Modified    : $Date: 2010/10/15 $ By $Author:  $
 *   Revision    : $Revision: 2.10 $
 *   Description : SUT-DE202 Tuner Control Sample Code
 *                 This code is not include I2C communication function.
 *                 It is necessary to add codes for the I2C communication
 *                 properly to execute and compile the code.
 *-----------------------------------------------------------------------------
 * This program may contain information about unfinished products and is subject
 * to change without notice.
 * Sony cannot assume responsibility for any problems arising out of the
 * use of the program
 *****************************************************************************/
/******************************************************************************
 *   includes:
 *    - system includes
 *    - application includes
 *****************************************************************************/
/*************************************************
 TS Interface Setting
 
 TS_SETTING_DVBT2 specifies the TS output mode: serial or parallel.
 The value is set to the demod register:bank=0x20,adr=0x70.
 
 0x38 : TS parallel output
 0x08 : TS serial output
**************************************************/
#define TS_SETTING_DVBT2 0x08//0x38 /* default : parallel */
//UINT8 gTS_SETTING_DVBT2=0x08;

/*************************************************
 I2C Max length of tuner write
**************************************************/
#define MAX_LENGTH_TUNER_WRITE 32

/*************************************************
 I2C slave address
**************************************************/
#define DVBT_SLAVE_ADDRESS  0xD8    /* Demod DVB-T/T2 Slave Address */
#define DVBC_SLAVE_ADDRESS  0xDC    /* Demod DVB-C Slave Address    */
#define TUNER_SLAVE_ADDRESS 0xC0    /* Tuner Slave Address          */

/******************************************************************************
 * T2 Lock/Unlock state check timer
*******************************************************************************/
#define T2_UNLOCK_STATUS_CHECK1_TIMER         3
#define T2_UNLOCK_STATUS_CHECK2_TIMER         6

#define T2_DEMOD_LOCK_CHECK_TIMEOUT          60
#define T2_TS_LOCK_CHECK_TIMEOUT             50
#define OFFSET_THRESHOLD                    100   /* [kHz] Carrier Offset Threshold */

/*************************************************
 Number of demod setting
**************************************************/
#define CORE_REGSETTING_MAX_ENTRY          24
#define CORE_COMMON_REGSETTING_MAX_ENTRY   43     /* 2010-10-05 change */

/* 2010-05-21 add */
/*************************************************
 AGC value to RSSI[dBm]
 AGC_R is defined as [voltage dividing ratio x 100]
 and should be changed according to actual IFAGC circuit.
**************************************************/
#define AGC_R                               69    /* AGC Voltage Dividing Radio : 0.69  ( div-registers assumption : 10kohm, 22kohm) */
#define AGC_A_T2                            54    /* AGC value to RSSI[dBm] : slope     ( fixed value ) */
#define AGC_B_T2                            1080  /* AGC value to RSSI[dBm] : intercept ( fixed value ) */

/*************************************************
 stdout function for debug
**************************************************/
/* #define DBG_PRINT_OUT */
#ifdef  DBG_PRINT_OUT
#define DBG_PRINT(...) libc_printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 * type definition (struct / enum):
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/
/*************************************************
 Block ID definition for I2C access
**************************************************/
#if 0
typedef enum {
    TUNER_TERR,       /* Tag for Tuner Block              */
    DEMOD_DVBT,       /* Tag for Demod Block (DVB-T/T2)   */
    DEMOD_DVBC        /* Tag for Demod Block (DVB-C)      */ /* add 2010-10-05 */
} BLOCKID;
#endif
/*************************************************
 TS Lock Status
**************************************************/
typedef enum {
    DVBT2_TS_UNKNOWN,
    DVBT2_TS_LOCK,
    DVBT2_TS_UNLOCK
} T2_TS_LOCK;

/*************************************************
 Demod Lock Status
**************************************************/
typedef enum {
    DEM_UNKNOWN,
    DEM_LOCK,
    DEM_UNLOCK
} T2_DEM_LOCK;


/* 2010-05-21 add */
/*************************************************
 PLP Sel Error Flag
**************************************************/
typedef enum {
    PLPSEL_OK,
    PLPSEL_ERROR
} PLPSEL_FLAG;

/* 2010-05-21 add */
/*************************************************
 PLP Sel Error Flag
**************************************************/
typedef enum {
    NO_CHANGE,
    CHANGE_AND_SRST
} L1CHG_RECOVERY_FLAG;

/*************************************************
 DVB-T2 tuning parameter struct
**************************************************/
typedef struct {
    INT32  m_frequency;     /* center frequency                                                */
    UINT8  m_BandWidth;     /* 0:N/A 1:6MHzBandWidth 2:7MHzBandWidth 3:8MHzBandWidth           */
    UINT8  m_MplpSelect;    /* 0:DefaultPLP  1:PLP selection                                   */
    UINT8  m_MplpId;        /* PLP ID                                                          */

} T2_DigitalTuningParam;

/*************************************************
 DVB-T2 L1 signaling information struct
**************************************************/
typedef struct {
    UINT16    type;
    UINT16    bwt_ext;
    UINT16    mixed;
    UINT16    miso;
    UINT16    mode;
    UINT16    guard;
    UINT16    pp;
    UINT16    papr;
    UINT16    ndsym;
} T2_DemodInfo;

/* 2010-06-21 add */
/* L1 Pre Signalling Data */
typedef struct {
    UINT8   type;
    UINT8   bwt_ext;
    UINT8   s1;
    UINT8   s2;
    UINT8   guard;
    UINT8   papr;
    UINT8   L1_mod;
    UINT8   pp;
    UINT8   tx_id_avaliablity;
    UINT16  cell_id;
    UINT16  network_id;
    UINT16  t2_system_id;
    UINT16  num_data_symbols;
} T2_L1PreInfo;

/* 2010-06-21 add */
/* L1 Post Signalling Data */
typedef struct {
    UINT8    num_aux;
    UINT8    aux_config_rfu;
    UINT8    fef_type;
    UINT32   fef_length;
    UINT8    fef_interval;

    /* Current PLP */
    UINT8    plp_id;
    UINT8    plp_type;
    UINT8    plp_code;
    UINT8    plp_mod;
    UINT8    plp_rotation;
    UINT8    plp_fec_type;
} T2_L1PostInfo;

/*****************************
 Demod & TSlock  State
*****************************/
typedef struct {
    enum {
        DEMOD_SEQ_START,               /* 0 */
        DEMOD_SEQ_WAIT_DEM_LOCK,       /* 1 */
        DEMOD_SEQ_WAIT_TS_LOCK,        /* 2 */
        DEMOD_SEQ_END                  /* 3 */
    } m_state;
    T2_DEM_LOCK   m_dem_lock;
    T2_TS_LOCK    m_ts_lock;
    INT16         m_waittime;         /* Wait time                                             */
    UINT8         m_seq_count;        /* Sequence_counter                                      */
    UINT8         m_spectrum_flag;    /* 0: normal, 1:iversion                                 */
    INT32         m_frequency;        /* Current frequency                                     */
    UINT8         m_BandWidth;        /* 0:N/A 1:6MHzBandWidth 2:7MHzBandWidth 3:8MHzBandWidth */
    INT32         m_offset;           /* CarrierOffset [kHz]                                   */
} T2_DemodSeqState;

/*************************************************
 Demod setting array struct
**************************************************/
struct CoreSettingInfo
{
    UINT8 m_address;                   /* I2C SubAddress               */
    UINT8 m_data;                      /* I2C Data                     */
};

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 * constants & variable:
 *  - constant
 *  - global variable
 *  - static variable
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/



/*************************************************
 DVB-T2 Demod Setting Common
**************************************************/

static const struct CoreSettingInfo
CoreRegSettingT2_COMMON[CORE_COMMON_REGSETTING_MAX_ENTRY] = 
{
    { 0x00, 0x20 },    /* Bank 20 */
    { 0x6f, 0x18 },
    { 0x70, TS_SETTING_DVBT2 }, /* 2010-05-21 change */
    { 0x71, 0xfd },
    { 0x72, 0x18 },
    { 0x7f, 0x2a },    /* 2010-05-21 add */
    { 0x82, 0x0a },
    { 0x83, 0x0a },
    { 0x95, 0x1a },
    { 0x96, 0x50 },
    { 0xb5, 0x00 },    /* 2010-10-05 add DNCNV_SRVS   = 0x00 */
    { 0xe0, 0x0f },    /* 2010-10-05 add SP_WR_SCALE  = 0x0f */
    { 0xe1, 0xff },    /* 2010-10-05 add SP_WR_SCALE  = 0xff */
    { 0x00, 0x21 },    /* Bank 21 */
    { 0x48, 0x30 },
    { 0x00, 0x25 },    /* Bank 25 */
    { 0x67, 0x07 },
    { 0x69, 0x03 },
    { 0x00, 0x27 },    /* Bank 27 */
    { 0x46, 0x0f },    /* 2010-05-21 add */
    { 0x47, 0x0f },    /* 2010-05-21 add */ 
    { 0x48, 0x0f },    /* 2010-05-21 add */
    { 0x69, 0x00 },    /* 2010-10-05 add CRL_OPEN    = 0x00 */
    { 0xC9, 0x07 },    /* 2010-10-05 add SYR_ACTMODE = 0x07 */    
    { 0xDD, 0x33 },    /* 2010-10-05 add FCS_ALPHA   = 0x33 */    
    { 0xE2, 0x00 },    /* 2010-10-05 add GTHETA_OFST = 0x00 */    
    { 0xE3, 0x00 },    /* 2010-10-05 add GTHETA_OFST = 0x00 */    
    { 0x86, 0x22 },
    { 0x87, 0x30 },
    { 0xe6, 0x14 },
    { 0xef, 0x15 }, 
    { 0x00, 0x2a },    /* Bank 2a */
    { 0x45, 0x06 },
    { 0x8c, 0x00 },
    { 0x8d, 0x34 },
    { 0x00, 0x3f },    /* Bank 3f */
    { 0x10, 0x0d },
    { 0x11, 0x02 },
    { 0x12, 0x01 },
    { 0x23, 0x2c },
    { 0x51, 0x13 },
    { 0x52, 0x01 },
    { 0x53, 0x00 }
};

/*************************************************
 DVB-T2 Demod Setting 8MBW
**************************************************/
static const struct CoreSettingInfo
CoreRegSettingT2_8MBW[CORE_REGSETTING_MAX_ENTRY] =
{
    { 0x00, 0x20 },
    { 0x9f, 0x11 },
    { 0xa0, 0xf0 },
    { 0xa1, 0x00 },
    { 0xa2, 0x00 },
    { 0xa3, 0x00 },
    { 0xa6, 0x1e },
    { 0xa7, 0x1d },
    { 0xa8, 0x29 },
    { 0xa9, 0xc9 },
    { 0xaa, 0x2a },
    { 0xab, 0xba },
    { 0xac, 0x29 },
    { 0xad, 0xad },
    { 0xae, 0x29 },
    { 0xaf, 0xa4 },
    { 0xb0, 0x29 },
    { 0xb1, 0x9a },
    { 0xb2, 0x28 },
    { 0xb3, 0x9e },
    { 0xb6, 0x1e },
    { 0xb7, 0x48 },
    { 0xb8, 0x6e },
    { 0xd7, 0x02 }
};

/* 2010-05-21 add */
/*************************************************
 DVB-T Demod Setting 7MBW
**************************************************/
static const struct CoreSettingInfo
CoreRegSettingT2_7MBW[CORE_REGSETTING_MAX_ENTRY] =
{
    { 0x00, 0x20 },
    { 0x9f, 0x14 },
    { 0xa0, 0x80 },
    { 0xa1, 0x00 },
    { 0xa2, 0x00 },
    { 0xa3, 0x00 },
    { 0xa6, 0x1b },
    { 0xa7, 0x22 },
    { 0xa8, 0x2b },
    { 0xa9, 0xc1 },
    { 0xaa, 0x2c },
    { 0xab, 0xb3 },
    { 0xac, 0x2b },
    { 0xad, 0xa9 },
    { 0xae, 0x2b },
    { 0xaf, 0xa0 },
    { 0xb0, 0x2b },
    { 0xb1, 0x97 },
    { 0xb2, 0x2b },
    { 0xb3, 0x9b },
    { 0xb6, 0x1b },
    { 0xb7, 0x29 },
    { 0xb8, 0x36 },
    { 0xd7, 0x42 }
};

/* Unused array was removed 2010-05-21 */
/* Log Table. used by CnCalc function. */
static const UINT16 DVBT2_CnTable[32] =
{
    0,  0,  12, 19, 24, 28, 31, 34,
    36, 38, 40, 42, 43, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 54,
    55, 56, 57, 57, 58, 58, 59, 60
};

/* add 2010-10-15 */
/*************************************************
 *************************************************
 Constant Variables for SSI/SQI Calculation
 *************************************************
 *************************************************/
/*************************************************
 Maximum Required C/N (x100 [dB])
 From NorDig Ver 2.2
 *************************************************/
static const INT16 DVBT2_ReqiredCNdBx100[4][6] = /* [Modulation][CodeRate] */
{   
    /* 1/2  3/5   2/3   3/4   4/5   5/6 */
    { 340,  470,  550,  650,  710,  760}, /* QPSK   */
    { 830, 1000, 1120, 1240, 1330, 1390}, /* 16QAM  */
    {1220, 1440, 1590, 1770, 1880, 1950}, /* 64QAM  */
    {1550, 1850, 2050, 2310, 2480, 2580}  /* 256QAM */
};

/*************************************************
 Reference Signal Level (x10 [dBm]) 
**************************************************/
static const INT16 DVBT2_RefRSSIx10[4][6] = 
{
    /*1/2   3/5   2/3   3/4   4/5   5/6 */
    {-960, -940, -940, -930, -920, -910}, /*   QPSK */
    {-910, -890, -880, -870, -860, -850}, /*  16QAM */
    {-870, -850, -830, -810, -800, -800}, /*  64QAM */
    {-840, -810, -790, -760, -740, -730}  /* 256QAM */
};

/*************************************************
 Log10 Table
 ( = 10 * log10(1+(index/32)) * 100)
**************************************************/
static const INT16 dvbt2_log10x100_table[] =  {
      0, 13, 26, 39, 51, 63, 75, 86,
     97,108,118,128,138,148,158,167,
    176,185,194,202,211,219,227,235,
    243,251,258,266,273,280,287,294 
};

#define ADJUST_T2 4301 /* BER adjustment PreBCH BER to PreRS BER ( 4301 = 10log(20000) x 100 ) */

/* add 2010-10-05 */
/*************************************************
 DVB-T2 Long Echo Sequence Enable/Disable
7.2.3 DVB-T2 Lock Sequence : meet DTG specification
7.2.4 DVB-T2 Long Echo Sequence : meet DTG and NorDig specification
*************************************************/
static const UINT8 LongEchoSeq_Ena = 1; /* 0:disable,1:enable */

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 * function prototype:
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*************************************************
 Command Function
**************************************************/
static int dvbt2_cmd_init(int argc, char **argv);                         /* tuner_init() */
static int cmd_init_dvbt2(int argc, char **argv);                   /* DVB-T2 init */
static int cmd_tune_dvbt2(int argc, char **argv);                   /* DVB-T2 tune */
static int cmd_sleep_dvbt2(int argc, char **argv);                  /* DVB-T2 sleep */
static int cmd_tsout(int argc, char **argv);                        /* DVB-T2 TS output on/off */

/* status */
static int cmd_get_dvbt2stat(int argc, char **argv);                /* get status DVB-T2 */
static int cmd_help(int argc, char **argv);                         /* help */
static int cmd_error(int argc, char **argv);


/*************************************************
 Tuning Function
**************************************************/
void dvbt2_tuner_init(void);                                        /* tuner: Initialize */
void dvbt2_tuner_wakeup(void);                                /* tuner: DVB-T2 init */
void dvbt2_tuner_tune(T2_DigitalTuningParam* pDigParam);      /* tuner: DVB-T2 tuning */
void dvbt2_tuner_sleep(void);                                 /* tuner: sleep */
void setRfTunerParam(UINT8 bandWidth);                        /* tuner: set RfTuning parameter */

void dvbt2_demod_init(void);                                        /* demod init */
void dvbt2_demod_wakeup(void);                                /* demod wakeup */
void dvbt2_demod_setparam(T2_DigitalTuningParam* pDigParam);  /* demod parameter setting */
void dvbt2_demod_start(void);                                 /* demod start */
void dvbt2_demod_sleep(void);                                 /* demod sleep */
void dvbt2_tsout( UINT8 enable );                             /* demod TS output on/off */

/* L1 Change Recovery */ /* 2010-05-21 add */
 void dvbt2_L1ChangeRecovery( L1CHG_RECOVERY_FLAG* pL1ChgRecoveryFlag );

/*Tuning Demod Sequece */
void demodLockSequence(T2_DigitalTuningParam * pParam, 
                      T2_DEM_LOCK* pDem_Lock, 
                      T2_TS_LOCK* pTs_Lock,
                      INT32* pOffset);                                   /* Demod sequence task manage */
void dvbt2_demodlock_start_seq( T2_DemodSeqState* pLockState );   /* Sequence start */
void dvbt2_demodlock_wait_demlock( T2_DemodSeqState* pLockState );/* Demod lock sequence */
void dvbt2_demodlock_wait_tslock( T2_DemodSeqState* pLockState ); /* Ts lock sequence */
void dvbt2_output_demodResults( T2_DemodSeqState* pLockState );   /* show Demod lock results */

/* DVB-T2 Long Echo Sequence. */ /* add 2010-10-05 */
int t2_le_Seq(T2_DEM_LOCK* pResult);         /* DVB-T2 Long Echo WaitDemodLock */
int t2_le_SeqTsLock(T2_TS_LOCK* pResult);    /* DVB-T2 Long Echo WaitTsLock */

/* DVB-T2 Monitoring */
void dvbt2_get_syncstat(UINT8* pSyncStat, UINT8* pTSLockStat );
void dvbt2_get_ifagc(UINT16* pIFAGCOut );
void dvbt2_get_rssi(INT16* pRSSI );                              /* 2010-05-21 add */
void dvbt2_get_CarrierOffset(UINT8 bandwidth, INT32* pOffset );
void dvbt2_get_CN(INT16* pCN_q2 );
void dvbt2_get_PreBCHBer(UINT32* pBitError, UINT32* pPeriod );
void dvbt2_get_PostBCHFer(UINT32* pFrameError, UINT32* pPeriod );
void dvbt2_get_T2DemodInfo(T2_DemodInfo* pInfo);                 /* 2010-06-21 This function has been merged to dvbt2_get_T2L1PreInfo. */
void dvbt2_get_T2PLPSelFlag(PLPSEL_FLAG* pPlpSelFlag, UINT8* pCurrent_PLP_ID);/* 2010-05-21 add */
void dvbt2_get_PlpIdInfo(UINT8* PlpIdNum, UINT8 *PlpId);
void dvbt2_print_PlpIdInfo(UINT8 PlpIdNum, UINT8 *plpId);
void dvbt2_get_T2L1PreInfo(T2_L1PreInfo* pInfo);
void dvbt2_get_T2L1PostInfo(T2_L1PostInfo* pInfo);
void dvbt2_print_T2L1PreInfo(T2_L1PreInfo* pInfo);
void dvbt2_print_T2L1PostInfo(T2_L1PostInfo* pInfo);

/* SSI/SQI calc */ /* add 2010-10-15*/
static void dvbt2_get_CNdBx100(INT16* pCNx100 );
void dvbt2_get_ssi_sqi (UINT16* pSQI, UINT16* pSSI);
static UINT32 dvbt2_calc_10log10_x100(UINT32 input);

/* utility */
INT32 dvb_Convert2SComplement(UINT32 value, UINT32 bitlen);
UINT16 DVBT2_CnCalc( UINT16 input );
