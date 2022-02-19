/*************************************************
 TS Interface Setting
 
 TS_SETTING_DVBT specifies the TS output mode: serial or parallel.
 The value is set to the demod register:bank=0x00,adr=0x70.
 
 0xB8 : TS parallel output
 0x08 : TS serial output
**************************************************/
#define TS_SETTING_DVBT 0xB8 /* default parallel */


 /******************************************************************************
 ******************************************************************************
 * type definition (struct / enum):
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/*************************************************
 TS Lock Status
**************************************************/
typedef enum {
    TS_UNKNOWN,     /* Unknown */
    TS_LOCK,        /* TS Lock */
    TS_UNLOCK       /* Unlock ( Unlock_Flag==1 or Time out) */
} T_TS_LOCK;

/*************************************************
 TPS Lock Status
**************************************************/
typedef enum {
    TPS_UNKNOWN,
    TPS_LOCK,
    TPS_UNLOCK
} T_TPS_LOCK;


/*************************************************
 DVB-T tuning parameter struct
**************************************************/
typedef struct {
    UINT32 m_frequency;     /* center frequency                                      */
    UINT8  m_BandWidth;     /* 0:N/A 1:6MHzBandWidth 2:7MHzBandWidth 3:8MHzBandWidth */
    UINT8  m_hierarchy;     /* 0:High Priority 1:Low Priority                        */
} T_DigitalTuningParam;

/*************************************************
 DVB-T TPS signaling information struct
**************************************************/
typedef struct {
    UINT16    constellation;/* QAM              */
    UINT16    hierarchy;    /* hierarchy on/off */
    UINT16    rateHP;       /* HP Rate          */
    UINT16    rateLP;       /* LP Rate          */
    UINT16    guard;        /* Guard Interval   */
    UINT16    mode;         /* Mode             */
    UINT16    cellID;       /* Cell ID          */
} T_TPSInfo;

/*****************************
 LockState
*****************************/
typedef struct {
    enum {
        DVBT_LOCKSTAT_START,
        DVBT_LOCKSTAT_WAIT_TPS_LOCK,
        DVBT_LOCKSTAT_WAIT_TS_LOCK,
        DVBT_LOCKSTAT_END,
    } m_state;                      /* Lock Sequence State */
    T_TS_LOCK     m_tslock;         /* TS Lock Result      */
    T_TPS_LOCK    m_tpslock;        /* TPS Lock Result     */
    INT32         m_offset;         /* CarrierOffset [kHz] */
    INT16         m_waittime;       /* Wait time           */
    UINT8         m_seq_count;      /* Time-Out Counter    */
    UINT8         m_bandwidth;      /* bandwidth           */
} T_LOCKSTAT;

/*************************************************
 TS Lock Polling
**************************************************/
#define DVBT_LOCK_TIMEOUT                  15    /* DVB-T Lock Sequence Timeout 1.5sec      */
#define LOCK_POLLING_PERIOD                100   /* [ms] lock status polling period  100ms  */
#define OFFSET_THRESHOLD                   100   /* [kHz] Carrier Offset Threshold          */

/*************************************************
 Number of demod setting
**************************************************/
#define CORE_REGSETTING_MAX_ENTRY          27

/* 2010-05-21 add */
/*************************************************
 AGC value to RSSI[dBm]
 AGC_R is defined as [voltage dividing ratio x 100]
 and should be changed according to actual IFAGC circuit.
**************************************************/
#define AGC_R                               69    /* AGC Voltage Dividing Radio : 0.69  ( div-registers assumption : 10kohm, 22kohm) */
#define AGC_A_T                             54    /* AGC value to RSSI[dBm] : slope     ( fixed value ) */
#define AGC_B_T                             1080  /* AGC value to RSSI[dBm] : intercept ( fixed value ) */


/*************************************************
 Demod setting array struct
**************************************************/
struct DVBT_CoreSettingInfo
{
    UINT8 m_address;                   /* I2C SubAddress */
    UINT8 m_data;                      /* I2C Data       */
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
 DVB-T Demod Setting 8MBW
**************************************************/
static const struct DVBT_CoreSettingInfo
CoreRegSetting_8MBW[CORE_REGSETTING_MAX_ENTRY] =
{
    { 0x00 , 0x00 },
    { 0x9f , 0x11 },
    { 0xa0 , 0xF0 },
    { 0xa1 , 0x00 },
    { 0xa2 , 0x00 },
    { 0xa3 , 0x00 },
    { 0xa6 , 0x1E },
    { 0xa7 , 0x1D },
    { 0xa8 , 0x29 },
    { 0xa9 , 0xC9 },
    { 0xaa , 0x2A },
    { 0xab , 0xBA },
    { 0xac , 0x29 },
    { 0xad , 0xAD },
    { 0xae , 0x29 },
    { 0xaf , 0xA4 },
    { 0xb0 , 0x29 },
    { 0xb1 , 0x9A },
    { 0xb2 , 0x28 },
    { 0xb3 , 0x9E },
    { 0xb6 , 0x1E }, /* 2010-05-21 change */
    { 0xb7 , 0x48 }, /* 2010-05-21 change */
    { 0xb8 , 0x6E }, /* 2010-05-21 change */
    { 0xc2 , 0x11 },
    { 0xd7 , 0x03 },
    { 0xd9 , 0x01 },
    { 0xda , 0xE0 },
};

/*************************************************
 DVB-T Demod Setting 7MBW
**************************************************/
static const struct DVBT_CoreSettingInfo
CoreRegSetting_7MBW[CORE_REGSETTING_MAX_ENTRY] =
{
    { 0x00 , 0x00 },
    { 0x9f , 0x14 },
    { 0xa0 , 0x80 },
    { 0xa1 , 0x00 },
    { 0xa2 , 0x00 },
    { 0xa3 , 0x00 },
    { 0xa6 , 0x1b },
    { 0xa7 , 0x22 },
    { 0xa8 , 0x2b },
    { 0xa9 , 0xc1 },
    { 0xaa , 0x2c },
    { 0xab , 0xb3 },
    { 0xac , 0x2b },
    { 0xad , 0xa9 },
    { 0xae , 0x2b },
    { 0xaf , 0xa0 },
    { 0xb0 , 0x2b },
    { 0xb1 , 0x97 },
    { 0xb2 , 0x2b },
    { 0xb3 , 0x9b },
    { 0xb6 , 0x1b }, /* 2010-05-21 change */
    { 0xb7 , 0x29 }, /* 2010-05-21 change */
    { 0xb8 , 0x36 }, /* 2010-05-21 change */
    { 0xc2 , 0x13 },
    { 0xd7 , 0x43 },
    { 0xd9 , 0x12 },
    { 0xda , 0xf8 },
};

/*************************************************
 DVB-T Demod Setting 6MBW
**************************************************/
static const struct DVBT_CoreSettingInfo
CoreRegSetting_6MBW[CORE_REGSETTING_MAX_ENTRY] =
{
    { 0x00 , 0x00 },
    { 0x9f , 0x17 },
    { 0xa0 , 0xea },
    { 0xa1 , 0xaa },
    { 0xa2 , 0xaa },
    { 0xa3 , 0xaa },
    { 0xa6 , 0x19 },
    { 0xa7 , 0x24 },
    { 0xa8 , 0x2b },
    { 0xa9 , 0xb7 },
    { 0xaa , 0x2c },
    { 0xab , 0xac },
    { 0xac , 0x29 },
    { 0xad , 0xa6 },
    { 0xae , 0x2a },
    { 0xaf , 0x9f },
    { 0xb0 , 0x2a },
    { 0xb1 , 0x99 },
    { 0xb2 , 0x2a },
    { 0xb3 , 0x9b },
    { 0xb6 , 0x17 }, /* 2010-05-21 change */
    { 0xb7 , 0xba }, /* 2010-05-21 change */
    { 0xb8 , 0x11 }, /* 2010-05-21 change */
    { 0xc2 , 0x13 },
    { 0xd7 , 0x83 },
    { 0xd9 , 0x1f },
    { 0xda , 0xdc },
};
/*************************************************
 Log Table. used by CnCalc function.
**************************************************/
static const UINT16 CnTable[32] =
{
    0,  0,  12, 19, 24, 28, 31, 34,
    36, 38, 40, 42, 43, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 54,
    55, 56, 57, 57, 58, 58, 59, 60
};

/* Add 2010-10-15 */
/*************************************************
 *************************************************
 Constant Variables for SSI/SQI Calculation
 *************************************************
 *************************************************/
/*************************************************
 Maximum Required C/N (x100 [dB])
    2 talbes are provided.
    DVBT_ReqiredCNdBx100      : Required C/N for non hierarchal transmission (from NorDig Unified V2.2 Table 3.9 )
    DVBT_ReqiredCNdBx100_hier : Required C/N for hierarchal transmission     (from NorDig Unified V2.2 B.3 Table1,2)
 *************************************************/
/* DVBT_ReqiredCNdBx100 : required C/N for non hierarchal */
static const INT16 DVBT_ReqiredCNdBx100[3][5] = /* [Modulation][CodeRate] */
{   
    /* non hierarchal transmission */
    /* 1/2,  2/3,  3/4,  5/6,  7/8 */
    {  510,  690,  790,  890,  970  },  /* QPSK */
    {  1080, 1310, 1460, 1560, 1600 },  /* 16-QAM */
    {  1650, 1870, 2020, 2160, 2250 }   /* 64-QAM */
};
/* DVBT_ReqiredCNdBx100_hier : required C/N for hierarchal */
static const INT16 DVBT_ReqiredCNdBx100_hier[6][2][5] = /* [alpha,const][prioirty][Rate] */
{   
    {   /* alpha=1 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {910,    1200,    1360,    1500,    1660},  /* HP */
        {1250,   1430,    1530,    1630,    1690},  /* LP */
    },
    {   /* alpha=1 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {1090,   1410,    1570,    1940,    2060},  /* HP */
        {1670,   1910,    2090,    2250,    2370},  /* LP */
    },
    {   /* alpha=2 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {680,     910,     1040,    1190,    1270},  /* HP */
        {1500,    1720,    1840,    1910,    2010},  /* LP */
    },
    {   /* alpha=2 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {850,     1100,    1280,    1500,    1600},  /* HP */
        {1850,    2120,    2360,    2470,    2590},  /* LP */
    },
    {   /* alpha=4 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {580,     790,     910,     1030,    1210},  /* HP */
        {1950,    2140,    2250,    2370,    2470},  /* LP */
    },
    {   /* alpha=4 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {800,     930,     1160,    1300,    1290},  /* HP */
        {2190,    2420,    2560,    2690,    2780},  /* LP */
    },
};

/*************************************************
 Reference Signal Level (x10 [dBm])
    2 talbes are provided.
    DVBT_RefRSSIx10      : Reference Signal Level for non hierarchal transmission
    DVBT_RefRSSIx10_hier : Reference Signal Level for hierarchal transmission
**************************************************/
/* DVBT_RefRSSIx10 : Reference signal level for non hierarchal */
static const UINT16 DVBT_RefRSSIx10[3][5] = 
{
  /* 1/2, 2/3, 3/4, 5/6, 7/8 */
    { -930, -910, -900, -890, -880 },    /* QPSK  */
    { -870, -850, -840, -830, -820 },    /* 16QAM */
    { -820, -800, -780, -770, -760 }     /* 64QAM */
};
/* DVBT_RefRSSIx10_hier : Reference signal level for hierarchal */
static const INT16 DVBT_RefRSSIx10_hier[6][2][5] = /* [alpha,const][prioirty][Rate] */
{
    {   /* alpha=1 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-899,    -870,    -854,    -840,    -824},  /* HP */
        {-865,    -847,    -837,    -827,    -821},  /* LP */
    },
    {   /* alpha=1 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-885,    -839,    -819,    -796,    -784},  /* HP */
        {-814,    -792,    -779,    -767,    -759},  /* LP */
    },
    {   /* alpha=2 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-919,    -897,    -884,    -871,    -863},  /* HP */
        {-834,    -816,    -806,    -796,    -789},  /* LP */
    },
    {   /* alpha=2 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-905,    -877,    -858,    -840,    -830},  /* HP */
        {-798,    -775,    -763,    -750,    -743},  /* LP */
    },
    {   /* alpha=4 16QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-928,    -910,    -899,    -887,    -869},  /* HP */
        {-792,    -773,    -762,    -751,    -744},  /* LP */
    },
    {   /* alpha=4 64QAM */
        /* 1/2,   2/3,  3/4,  5/6,  7/8 */
        {-910,    -897,    -874,    -860,    -861},  /* HP */
        {-771,    -748,    -734,    -721,    -712},  /* LP */
    },
};

/*************************************************
 Log10 Table
 ( = 10 * log10(1+(index/32)) * 100)
**************************************************/
static const UINT16 log10x100_table[] =  {
      0, 13, 26, 39, 51, 63, 75, 86, 
     97,108,118,128,138,148,158,167,
    176,185,194,202,211,219,227,235,
    243,251,258,266,273,280,287,294
};

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
static int cmd_init(int argc, char **argv);                        /* init               */
static int cmd_init_dvbt(int argc, char **argv);                   /* DVB-T    t_wakeup  */
static int cmd_tune_dvbt(int argc, char **argv);                   /* DVB-T    t_tune    */
static int cmd_sleep_dvbt(int argc, char **argv);                  /* DVB-T    t_sleep   */
static int cmd_tsout(int argc, char **argv);                       /* DVB-T    t_tsout   */

/* status */
static int cmd_get_dvbtstat(int argc, char **argv);                /* DVB-T    t_monitor */
static int cmd_help(int argc, char **argv);                        /* help               */
static int cmd_error(int argc, char **argv);

/*************************************************
 Tuning Function
**************************************************/
void tuner_init(void);                                      /* tuner: Initialize       */
void dvbt_tuner_wakeup(void);                               /* tuner: DVB-T wakeup     */
void dvbt_tuner_tune(T_DigitalTuningParam* pDigParam);      /* tuner: DVB-T tuning     */
void dvbt_tuner_sleep(void);                                /* tuner: sleep            */
void dvbt_setRfTunerParam(UINT8 bandWidth);                      /* tuner register setting  */

void dvbt_demod_init(void);                                      /* demod init              */
void dvbt_demod_wakeup(void);                               /* demod wakeup            */
void dvbt_demod_setparam(T_DigitalTuningParam* pDigParam);  /* demod parameter setting */
void dvbt_demod_start(void);                                /* demod start             */
void dvbt_demod_sleep(void);                                /* demod sleep             */
void dvbt_tsout( UINT8 enable );                            /* demod TS output on/off  */

/** Lock Sequence */
void dvbt_GetDemodState(UINT8 bandwidth, T_TPS_LOCK* pTPS_Lock, T_TS_LOCK* pTS_Lock, UINT8* pUnlock_Flag, INT32* pOffset);
void LockSequence(UINT8 bandwidth, T_TPS_LOCK* pTPS_Lock, T_TS_LOCK* pTS_Lock, INT32* pOffset);
void dvbt_lockseq_start(T_LOCKSTAT* pLockState);
void dvbt_lockseq_wait_tps_lock(T_LOCKSTAT* pLockState);
void dvbt_lockseq_wait_ts_lock(T_LOCKSTAT* pLockState);

/* DVB-T Monitoring */
void dvbt_get_syncstat(UINT8* pSyncStat, UINT8* pTSLockStat );
void dvbt_get_ifagc(UINT16* pIFAGCOut );
 void dvbt_get_rssi(INT16* pRSSI );                         /* 2010-05-21 add */
 void dvbt_get_CarrierOffset(UINT8 bandwidth, INT32* pOffset );
 void dvbt_get_CN(INT16* pCN_q2 );
static void dvbt_get_CNdBx100(INT16* pCNx100 );                   /* 2010-10-15 add */
 void dvbt_get_PreRSBer(UINT32* pBitError, UINT32* pPeriod );
 void dvbt_PreRSBer_ResetBer(void);
 void dvbt_PreRSBer_GetBer(UINT8* valid, UINT32* pBitError);
 void dvbt_PreRSBer_BerSize(UINT32* pPeriod );
 void dvbt_get_RSError(UINT16* pBitError);
 void dvbt_get_TPSInfo(T_TPSInfo* pInfo);

/* SSI/SQI calc */ /* Add 2010-10-15 */
static void dvbt_get_ssi_sqi (UINT16* pSQI, UINT16* pSSI);
static UINT32 calc_10log10_x100(UINT32 input);
/* utility */
 INT32 dvb_Convert2SComplement(UINT32 value, UINT32 bitlen);
 UINT16 CnCalc( UINT16 input );   

 
