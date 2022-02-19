#ifndef __LLD_NIM_M3327_H__
#define __LLD_NIM_M3327_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <api/libc/printf.h>


//#define NIM_M3327_BASE_IO_ADR  0xB8003000

/****************************************************************************************\
*                       ZL10353 register definitions                                     *
\****************************************************************************************/
#define REG353_IRQMAIN                     0x00        /*4  IRQ regs 0 to 3 */
#define REG353_IRQ4                            0x04        /*1  IRQ reg 4 */
#define REG353_IRQ5                            0x05        /*1  IRQ reg 5 (TPS flags cannot be enabled to IRQ pin)*/
#define REG353_STATUS_0                    0x06        /*4  the 4 status registers */
#define REG353_STATUS_1                   0x07        /*4  the 4 status registers */
#define REG353_STATUS_2                    0x08        /*4  the 4 status registers */
#define REG353_AGCGAIN                     0x0A        /*2  14 bit AGC total gain value*/
#define REG353_AGCDIG                0x0C           /*2  16 bit Digital AGC */
#define REG353_RFADCLEVEL                  0x0E        /*1  7 bit ADC reading RF AGC */
#define REG353_SNR                             0x10        /*1  SNR in 1/8 dB steps. */

#define REG353_RSBERCNT                    0x11        /*3  Reed-Solomon error count */
#define REG353_RSUBC                           0x14        /*2  Reed-Solomon uncorrected block count */
#define REG353_ITBFREQOFFSET        0x16           /*2  ITB Freq Offset */
#define REG353_FREQOFFSET                  0x18        /*3 Frequency offset calculated from this */

#define REG353_TPSRECEIVED                 0x1D        /*2  TPS received register */
#define REG353_TPSCURRENT                  0x1F        /*2  TPS current register */
#define REG353_TPSCELLID                     0x21        /*2  CELL ID*/
#define REG353_TPSMISCDATA                 0x23        /*3 TPS misc data */
#define REG353_VITERRCNT                   0x2F        /*3  Viterbi error count */
#define REG353_FIRSTRW                     0x4C        /*   location of first non read only register  */
#define REG353_ENABLE                      0x50        /*1  Power control /enable regsiter*/
#define REG353_CLKCTL0                     0x51        /*1 clock control */
#define REG353_CLKCTL1                     0x52        /*1 clock control */
#define REG353_PLLCTL0                     0x53        /*1 PLL control */
#define REG353_PLLCTL1                     0x54        /*1 PLL control */
#define REG353_RESET                           0x55        /*1 RESET reg*/
#define REG353_AGCTARGET                   0x56        /*1 AGC target and rev agc control*/
#define REG353_OUTCTL0                     0x5A        /*1 MPEG output control */
#define REG353_MCLKCTL                     0x5C        /*1 MPEG clock trim */
#define REG353_ACQCTL                      0x5E        /* acquisition control */
#define REG353_CAPTURE                     0x5F        /*1 capture range register*/
#define REG353_RSERRPER                    0x60        /*2  Reed-Solomon Error period*/
#define REG353_SCANCTL                     0x62        /*1  contains I2C bus master/passthrough switches etc*/
#define REG353_GPPCTL                      0x63        /*1  GPP controls */
#define REG353_TRLRATE                     0x65        /*2  TRL nominal rate */
#define REG353_TUNADDRESS              0x67        /*1  Address of tuner */
//#define REG353_TUNERDATA               0x68        /*4  Tuner data (bytes 2,3,4,5) */
#define REG353_CHAN_START               0x68
#define REG353_TUNER_CONT_1        0x6A
#define REG353_TUNER_CONT_0        0x6B
#define REG353_TPSGIVEN                0x6E
#define REG353_CHAN_STOP            0xE2
#define REG353_ITBFREQ                     0x6C        /*2 ITB mixer frequency */
#define REG353_TPSGIVEN                    0x6E        /*2 TPS GIVEN register */
#define REG353_TUNERGO                     0x70        /*1 Acq start */
#define REG353_FSMGO                          0x71        /*1 Acq start */
#define REG353_IRQMAINENABLE           0x72        /*4 IRQ enable regs 0 to 3 */
#define REG353_IRQ4ENABLE                  0x76        /*1 IRQ 4 enables*/
#define REG353_CHIP_ID                0x7f
#define REG353_AGC_CTL                   0x89       /*4 period in 50 ms steps after AGC_Fail interrupt is generated.*/
#define REG353_VITERRPER                   0xD6        /*1 Viterbi error measurement period */
#define REG353_OFDM_LOCK_TIME        0xE7
#define REG353_ADCCTL0                     0xEA        /*1 ADC recalibrate register */

//dual agc
#define  MT353REG_AGC_CTRL_5            (0x8E)
#define  MT353REG_AGC_IF_LOLIM            (0x90)
#define  MT353REG_AGC_RF_HILIM            (0x91)
#define  MT353REG_AGC_IF_MAX            (0x92)
#define  MT353REG_AGC_IF_MIN            (0x93)
#define  MT353REG_AGC_RF_MAX            (0x94)
#define  MT353REG_AGC_RF_MIN            (0x58)
#define  MT353REG_AGC_KIF                (0x95)
#define  MT353REG_AGC_KRF                (0x96)
#define  MT353REG_UK_REG1                (0x9C ) /* unknown register */

/*STATUS_3 TUNER_STATUS*/
#define MT353_TUNER_LOCK_FLAG        0x40


/*INDEX*/
#define LOCK_OK               11
#define FEC_UNLOCK             12
#define SCAN_END             13
#define SCAN_TIMEOUT        14
#define TPS_UNLOCK             19
#define TUNER_I2C_FAIL        21
#define TUNER_NO_LOCK        22


/* other defines*/
#define TNIM_WAIT_LOCKUP        500 /* millseconds to wait for channel lock*/
#define TNIM_SCANWAIT_TPS      100 /* milliseconds to wait for valid TPS in sccan*/


/* MT353_TPS*/
#define MT353_HP 0 << 15

#define MT353_FEC_1_2   0 << 7
#define MT353_FEC_2_3   1 << 7
#define MT353_FEC_3_4   2 << 7
#define MT353_FEC_5_6   3 << 7
#define MT353_FEC_7_8   4 << 7

#define MT353_GUARD_1_32 0 <<2
#define MT353_GUARD_1_16 1 <<2
#define MT353_GUARD_1_8  2 <<2
#define MT353_GUARD_1_4  3 <<2

#define MT353_MODE_2K    0
#define MT353_MODE_8K   1

#define  MT353_MODU_QPSK  0  <<13
#define  MT353_MODU_16QAM 1 <<13
#define  MT353_MODU_64QAM 2 <<13





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



#if 1        //51117-01Angus
struct nim_mt353_private
{
    struct COFDM_TUNER_CONFIG_API Tuner_Control;
    //UINT32 tuner_id;
};
#else
struct nim_m3327_private
{
    struct nim_device_stats stats;
    UINT8 chip_id;
};
#endif

static INT32 f_mt353_read(UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len);
static INT32 f_mt353_write(UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len);
INT32 f_mt353_attach();
void f_mt353_hw_init();
void f_mt353_hardware_reset();
void f_mt353_after_reset_set_param();
static INT32 f_mt353_open(struct nim_device *dev);
static INT32 f_mt353_close(struct nim_device *dev);
static INT32 f_mt353_disable(struct nim_device *dev);//Sam
void     f_mt353_dump_register();
static INT32 f_mt353_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority);
static INT32 f_mt353_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority);
void f_mt353_clear_int( );
static INT32 f_mt353_get_lock(struct nim_device *dev, UINT8 *lock);
static INT32 f_mt353_get_freq(struct nim_device *dev, UINT32 *freq);
static INT32 f_mt353_get_code_rate(struct nim_device *dev, UINT8* code_rate);
static INT32 f_mt353_get_AGC(struct nim_device *dev, UINT16 *agc);
static INT32 f_mt353_get_SNR(struct nim_device *dev, UINT8 *snr);
static INT32 f_mt353_get_BER(struct nim_device *dev, UINT32 *RsUbc);
static INT32 f_mt353_get_GI(struct nim_device *dev, UINT8 *guard_interval);
static INT32 f_mt353_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
static INT32 f_mt353_get_modulation(struct nim_device *dev, UINT8 *modulation);
static INT32 f_mt353_get_bandwidth(struct nim_device *dev, UINT8 *bandwidth);
static INT32 f_mt353_get_RFLevel(struct nim_device *dev, INT32  *RFlevel);
static INT32 f_mt353_Tracking(struct nim_device *dev);
static INT32 f_mt353_get_specinv(struct nim_device *dev, UINT8 *Inv);
static INT32 f_mt353_i2c_bypass(struct nim_device *dev);
static INT32 f_mt353_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);    //50622-01Angus


static INT32 f_mt353_init_reg(struct nim_device *dev);//051124 yuchun
static INT32 f_mt353_freq_offset(struct nim_device *dev, UINT32 bandwidth, UINT8 fft_mode, INT32 *freq_offset);//051124 yuchun
static INT32 f_mt353_getinfo(struct nim_device *dev, UINT8 *code_rate, UINT8 *guard_interval, UINT8 *fft_mode, UINT8 *modulation);//051124 yuchun
void f_mt353_tracking_task(struct nim_device *dev);
static INT32 f_mt353_tracking_task_init(struct nim_device *dev);


//TRLNOMRATE
INT32 f_get_TRL_OFFSET(struct nim_device *dev);
void f_mt353_Core_Switch(struct nim_device *dev);
INT32 f_mt353_get_TRLNOMRATE(struct nim_device *dev);
void f_mt353_set_TRLNOMRATE(struct nim_device *dev,UINT32  value);
INT32 f_mt353_TRLNOMRATE_Tuning(struct nim_device *dev);
void f_mt353_dump_reg(struct nim_device *dev);
void f_mt353_gpio_reset(void);  //050905 yuchun  for HW RESET

static INT32 f_mt353_get_freq_offset(struct nim_device *dev, INT32 *freq_offset);//051222 yuchun
void f_mt353_PassThrough(struct nim_device *dev,BOOL Open);

#if 1// (SYS_SDRAM_SIZE == 8)
INT8 f_mt353_priority(struct nim_device *dev, UINT8*priority);
static INT32 f_mt353_get_hier_mode(struct nim_device *dev, UINT8*hier);
static INT32 f_mt353_hier_mode(struct nim_device *dev);
//auto test
INT32 CE6353_PostViterbi_BER_Read(struct nim_device *dev);
UINT32 Tnim_RSUBC(struct nim_device *dev);


#endif

struct MT353_Lock_Info
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

};
struct MT353_Demod_Info
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




/****************************************************************************************\
*                       ZL10353 register bits                                            *
\****************************************************************************************/

/* REG353_IRQMAIN (0x00 - 0x03) and     REG353_IRQENMAIN (0x05-0x06) */
#define REG353_IRQMAIN_OFDM_NOLOCK           0x80000000
#define REG353_IRQMAIN_OFDM_LOCK             0x40000000       /* OFDM/pilots/timing found waiting TPS)*/
#define REG353_IRQMAIN_MG_FAIL               0x20000000       /* mode/guard search failed */
#define REG353_IRQMAIN_MG_SEARCH             0x10000000       /* mode/gaurd search started */
#define REG353_IRQMAIN_SYR_NOLOCK            0x08000000       /* forced mode/guard not found, search started*/
#define REG353_IRQMAIN_SYR_LOCK              0x04000000       /* No pilots yet, TPS_CURRENT valid though*/
#define REG353_IRQMAIN_AGC_NOLOCK            0x02000000       /* AGC lock timed out*/
#define REG353_IRQMAIN_AGC_LOCK                 0x01000000       /* AGC locked*/
#define REG353_IRQMAIN_OFDM_FEC_LOCK         0x00800000       /* full lock*/
#define REG353_IRQMAIN_TRSW_FAILED           0x00400000       /* trigger sweep failed */
#define REG353_IRQMAIN_TRSW_TRACKING         0x00200000       /* trigger sweep tracking (locked)*/
#define REG353_IRQMAIN_TRSW_START            0x00100000       /* trigger sweep started*/
#define REG353_IRQMAIN_FEC_NOLOCK            0x00080000       /* FEC lock timeout*/
#define REG353_IRQMAIN_FEC_LOCK              0x00040000       /* byte lock, waiting echo trigger sweep (if required)*/
#define REG353_IRQMAIN_TPS_NOLOCK            0x00020000       /* TPS lock timeout */
#define REG353_IRQMAIN_TPS_LOCK              0x00010000       /* first TPS with valid block check received */
#define REG353_IRQMAIN_VIT_BERPER            0x00002000       /* Viterbi timer Pre-Viterbi update*/
#define REG353_IRQMAIN_RS_BERPER             0x00000400       /* RS timer - Post-Viterbi BER update*/
#define REG353_IRQMAIN_TUNER_NOFL            0x00000010
/* TODO: interrupts consumed by the driver */
#define REG353_IRQMAIN_LOCKIRQS         \
   (    REG353_IRQMAIN_OFDM_LOCK        \
      | REG353_IRQMAIN_TUNER_NOFL       \
      | REG353_IRQMAIN_AGC_NOLOCK       \
      | REG353_IRQMAIN_SYR_NOLOCK       \
      | REG353_IRQMAIN_MG_FAIL          \
      | REG353_IRQMAIN_FEC_LOCK         \
      | REG353_IRQMAIN_TRSW_FAILED      \
      | REG353_IRQMAIN_FEC_NOLOCK       \
      | REG353_IRQMAIN_TRSW_START       \
      | REG353_IRQMAIN_OFDM_FEC_LOCK    \
    )

#define  REG353_IRQMAIN_DRIVERIRQS      \
    (     REG353_IRQMAIN_LOCKIRQS       \
        | REG353_IRQMAIN_RS_BERPER      \
        | REG353_IRQMAIN_VIT_BERPER     \
    )

#define MT353_FECSTAT_LOCKED        0x30
#define MT353_LOGIC_ENABLE        0x01

//#define REG353_STATUS_0         0x06
//#define REG353_STATUS_1         0x07
//#define REG353_STATUS_2        0x08

#define REG353_STATUS_MASK       0xEF3FC058 /* these are the bits in the manual*/

/* REG353_TPSRECEIVED   0x1D */
/* REG353_TPSGIVEN    0x6E*/
#define REG353_TPSGIVEN_LPSEL                 0x8000


/* REG353_ENABLE    0x50*/
#define REG353_ENABLE_DVRPORTSEL              0x20
#define REG353_ENABLE_DVROUTEN                0x10
#define REG353_ENABLE_DVRPORTDIS              0x08
#define REG353_ENABLE_MPEGDIS                 0x04
#define REG353_ENABLE_ADCEN                   0x02
#define REG353_ENABLE_CHIPEN                  0x01


/* REG353_CLKCTL0   0x51*/
#define REG353_CLKCTL0_HIGHSAMPLE             0x04
#define REG353_CLKCTL0_SERIALOUT              0x20
#define REG353_CLKCTL0_MCLKINVERT             0x40
/* REG353_CLKCTL1   0x52*/
#define REG353_CLKCTL1_ADCXTAL                0x01
/*REG353_RESET        0x55*/
#define REG353_RESET_FULL                     0x80
#define REG353_RESET_PART                     0x40
#define REG353_RESET_ALL                      0xFF
/*REG353_AGCTARGET    0x56*/
#define REG_AGCTARGET_REVAGC                 0x80
#if 0        //51117-01Angus
#if(SYS_TUN_MODULE == G151D)
#define IF_AGC_TARGET_INIT          0x31
#else
#define IF_AGC_TARGET_INIT                  0x2B
#endif
#endif

#define IF_AGC_TARGET_VAL        0x1C

/*REG353_AGCTARGET    0x57*/
#define DIG_AGC_TARGET_VAL                    0x62


/* REG353_OUTCTL0    0x5A*/
#define REG353_OUTCTL0_BG12_5B                0x01
#define REG353_OUTCTL0_MDO_SWP                0x02
#define REG353_OUTCTL0_BKERR_INV              0x04
#define REG353_OUTCTL0_EN_TEI                 0x08
#define REG353_OUTCTL0_MCLKIN_EN              0x10
#define REG353_OUTCTL0_MCLKRAT_EN             0x20
#define REG353_OUTCTL0_AGC2_DIS               0x40
#define REG353_OUTCTL0_RFADC_EN               0x80

/*REG353_ACQCTL        0x5E*/
#define REG353_ACQCTL_FORCEMODE               0x01
#define REG353_ACQCTL_FORCEGUARD              0x02
#define REG353_ACQ_CTL_AUTO              0xFC

/*REG353_SCANCTL       0x62*/
#define REG353_SCANCTL_BUSMASTER              0x08
#define REG353_SCANCTL_PASSTHRO               0x10

/*REG353_GPPCTL        0x63*/
#define REG353_GPPCTL_GPP0                    0x01
#define REG353_GPPCTL_GPP1                    0x02
#define REG353_GPPCTL_GPP2                    0x04
#define REG353_GPPCTL_GPP3                    0x08
#define REG353_GPPCTL_GPP0_OUT                0x10
#define REG353_GPPCTL_GPP1_OUT                0x20
#define REG353_GPPCTL_GPP2_OUT                0x40
#define REG353_GPPCTL_GPP3_OUT                0x80

/*REG353_TUNERGO      0x70*/
#define REG353_TUNERGO_GO                     0x01
/*REG353_FSMGO        0x71*/
#define REG353_FSMGO_GO                       0x01

/*REG353_AGC_CTL       0x89*/
#define AGC_NOLOCK_PER                        0x03

/* REG353_ADCCTL0     0xEA*/
#define REG353_ADCCTL0_CAL                    0x01
#define REG353_ADCCTL0_CAL_RESET              0x0
/* LOCK IMFO   */
#define MT353_TUNER_LOCK_FLAG              0x40
#define MT353_SYMBOL_LOCK_FLAG        0x02
#define MT353_OFDM_LOCK_FLAG              0x03
#define MT353_FEC_LOCK_FLAG                  0x30
/*set OFDM_WAIT_TIME    */
#define MT353_OFDM_8K_WAIT               0x40
/*set CLK mode   */
const UINT8 MT353_ClockMode_IF[]=
{
    //0x44, 0x46, 21, 15,           /* mode 10 20.48 MHz, 36 MHz IF */
   // 0x44, 0x66, 23, 7,            /* mode 11 20.48MHz, 44 MHz IF */
    ////FOR 36.125-LDW_070511///
    0x44, 0x46, 21, 15,           /* mode 10 20.48 MHz, 36 MHz IF */
        0x44, 0x66, 23, 7,            /* mode 11 20.48MHz, 44 MHz IF */
        0x45, 0x22,  9, 15,           /* mode 12 27MHz, 36MHz IF */
        0x44, 0x44, 12, 15,           /* mode 13 27 MHz, 44MHz IF */
        0x44, 0x22, 67,  3,           /* mode 14  4MHz, 36 MHz IF */
        0x45, 0x22, 52,  1            /* mode 15  4MHz, 44 MHz IF */



} ;

const UINT8 MT353_ClockMode_Low_IF[]=
{
    //////////for 4.571 LDW_070511////
    0x44, 0x46, 21, 15,   // ; Clocks=20480, Mode a        // JILEE 080729, 0x51 : 0x04 to 0x44
         0x06, 0x22,  9, 15,   // ; Mode b ;Clocks=27000,
         0x04, 0x22, 67,  3,   // ; Mode c ;Clocks= 4000,
         0x02, 0x22, 11,  7,   // ; Mode d ;Clocks=20480,
         0x02, 0x22,  8, 15,   // ; Mode e ;Clocks=27000,
         0x02, 0x22, 60,  1,   // ; Mode f ;Clocks= 4000,


} ;



UINT8 g_ucInitTable[]=
{
//    0x56, 0x2B, 0x9C, 0xA1, 0x00, 0x00
//    0X56,0X2B,0X9C,0XA1,0XBA,0XFD,0X7C,0X2B,0X00,0X00

    0X56,0X30,
    //0X57,0X62,
    0X9C,0XA1,
    0XBA,0XFD,
    0X7C,0X2B,
    0X00,0X00
};
int g_ucInitTable_size = sizeof(g_ucInitTable) / sizeof(UINT8);

UINT8 g_ucBw6FilterCoeff[]=
{
    0x9A,0xB4,
    0x9B,0x04,
    0x9A,0xCD,
    0x9B,0x19,
    0x9A,0x87,
    0x9B,0x22,
    0x9A,0xBD,
    0x9B,0x3D,
    0x9A,0x19,
    0x9B,0x4A,
    0x9A,0x09,
    0x9B,0x53,
    0x9A,0xC1,
    0x9B,0x6B,
    0x9A,0x5C,
    0x9B,0x7A,
    0x9A,0x8B,
    0x9B,0x83,
    0x9A,0x37,
    0x9B,0x9B,
    0x9A,0x6E,
    0x9B,0xAA,
    0x9A,0xDF,
    0x9B,0xB3,
    0x9A,0x34,
    0x9B,0xC0,
    0x9A,0x51,
    0x9B,0xD0,
};
#define REG353_OFDMBWCTRL     0x64
#define REG353_RESET_OFDM_BLOCK 0x20

/****************************************************************************************\
*                                     Macros                                             *
\****************************************************************************************/
/* Timers */
#define TNIM_TIMER_TIMEOUT        0         /* Overall time-out required for pending operations */
#define TNIM_TIMER_HOLDOFF        1         /* kernel re-run minimum interval (limits I2C access in polling mode only)*/
#define TNIM_TIMER_RFSIGLEVEL     2         /* timer for updating RF signal level measurement */
#define TNIM_TIMER_RSUBC          3         /* timer for 1 second RSUBC updating*/
#define TNIM_TIMER_NUMTIMERS      4         /* total number of timers in system*/
/* Timer Values (milliseconds)*/
#define TNIM_TIMEOUT_NOHANDLE      100       /* time-out whiles hTnim=NULL (IRQ mode only)*/
#define TNIM_TIMEOUT_DEFAULT       100       /* default value for timeout */
#define TNIM_TIMEOUT_OFFSTATE      200       /* how often to check chip state in off state */
#define TNIM_TIMEOUT_WAITRESET     100       /* how often to check for reset release */
#define TNIM_TIMEOUT_IDLE          100       /* how often to check for lock, in idle state */
#define TNIM_TIMEOUT_LOCKED        100       /* How often to check whilst locked */
#define TNIM_TIMEOUT_TUNERFL       50        /* msec between checks of tuner lock (non auto only*/
#define TNIM_TIMEOUT_ACQ           2500      /* overall timeout for acquisition */

#define TNIM_TIME_HOLDOFF          10        /* time between kernel runs in polling mode*/
#define TNIM_TIME_RFLEVEL          250       /* time between RF signal level updates (ZL10353 RFADC system */
#define TNIM_TIME_RSUBC            1000      /* update interval for RSUBCs */
#define TNIM_COUNT_RESET           6         /* number of TNIM_TIMEOUT_WAITRESET periods to wait for hardware reset to clear*/
#define TNIM_COUNT_TUNERFL         5








#endif    /* __LLD_NIM_M3327_H__ */

