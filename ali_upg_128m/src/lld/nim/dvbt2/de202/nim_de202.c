/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    This file contains m3327 basic function in LLD.
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    4.14.2005        Sam Chen      Ver 0.1       Create file.
*
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
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
#include "nim_de202.h"

//===========================
#include "sut_de202_dvbt.h"
#include "sut_de202_dvbt2.h"
#include "userdef.h"

//#define DVBT_ENABLE

#if 1//(SYS_DEM_MODULE==de202 )

#define NIM_PRINTF(...)
#define DE202_DEBUG_FLAG    0
#if(DE202_DEBUG_FLAG)
    #define NIM_de202_PRINTF  libc_printf
#else
    #define NIM_de202_PRINTF(...)
#endif

//#define soc_printf(...)
#define Delay_Number    1
#define WAIT_Number    1
#define SYS_WAIT_MS(x) osal_delay((x*1000*WAIT_Number))//osal_task_sleep(x*WAIT_Number)    //osal_delay((x*1000))

#define abs(x)    (x<0) ? -x : x
#define sign(y)    (y>0) ? 1 : -1

#define de202_rule
#define Mode_a


#define    NIM_de202_FLAG_ENABLE             0x00000100    //050513 Tom
#define NIM_de202_SCAN_END                        0x00000001
#define NIM_de202_CHECKING            0x00000002

ID f_de202_tracking_task_id = OSAL_INVALID_ID;

ID      nim_de202_flag[2] = {OSAL_INVALID_ID,OSAL_INVALID_ID};
struct de202_Lock_Info                 *de202_CurChannelInfo[2];
__MAYBE_UNUSED__ static struct de202_Demod_Info                *de202_COFDM_Info[2];

INT8 de202_autoscan_stop_flag = 0;    //50622-01Angus
//ID f_de202_IIC_Sema_ID[2] = {OSAL_INVALID_ID,OSAL_INVALID_ID};
ID f_de202_IIC_Sema_ID = OSAL_INVALID_ID;

extern ID f_MXL241_IIC_Sema_ID;
struct nim_device *nim_de202[2] = {NULL,NULL};

static UINT32 tuner_cnt = 0;
/**************************************************************************************/
extern  BLOCKID tuner_block_id ;
extern BLOCKID demod_block_id ;
//extern UINT8 gTS_SETTING_DVBT2;



/****************************************************************************************/
#define SYS_TS_INPUT_INTERFACE   TSI_SPI_1
#define WAIT_TUNER_LOCK 200
#define WAIT_OFDM_LOCK  200
#define WAIT_FEC_LOCK     512

static UINT32 de202_ber = 0;
static UINT32 de202_per = 0;
static UINT32 de202_per_tot_cnt = 0;

__MAYBE_UNUSED__ static char nim_de202_name[HLD_MAX_NAME_SIZE] = "NIM_COFDM_0";


static INT32  f_de202_read(UINT8 tuner_id,UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    //struct nim_de202_private *priv = (struct nim_de202_private *)nim_de202[tuner_id]->priv;
    //struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    INT32 err = 0;


    osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);


    osal_semaphore_release(f_de202_IIC_Sema_ID);
    return err;
}

static INT32  f_de202_write(UINT8 tuner_id,UINT8 dev_add, UINT8 reg_add, UINT8 *data, UINT8 len)
{
    //struct nim_de202_private *priv = (struct nim_de202_private *)nim_de202[tuner_id]->priv;
    //struct EXT_DM_CONFIG *cfg = (struct EXT_DM_CONFIG *)&priv->Tuner_Control.ext_dm_config;
    //UINT8  i, buffer[8] = {0};
    INT32 err = 0;


    osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);

    osal_semaphore_release(f_de202_IIC_Sema_ID);
    return  err;
}



//=======================================================
/*------------------------------------------------------------------------------
  Sample implementation of trace log function
  These functions are declared in sony_dvb.h
------------------------------------------------------------------------------*/
__MAYBE_UNUSED__ static const char *g_callStack[256];    /* To output function name in TRACE_RETURN log */
__MAYBE_UNUSED__ static int g_callStackTop = 0;


//=======================================================
/*****************************************************************************
* INT32  f_de202_attach()
* Description: de202 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
INT32 f_de202_attach(char *name,struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner)    //51117-01Angus
{

    struct nim_device *dev;
    struct nim_de202_private * priv_mem;

    if ((ptrCOFDM_Tuner == NULL))
    {
        NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (struct nim_de202_private *)MALLOC(sizeof(struct nim_de202_private));
    if ((void*)priv_mem == NULL)
    {
        dev_free(dev);

        NIM_de202_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    MEMCPY((void*)&(priv_mem->Tuner_Control), (void*)ptrCOFDM_Tuner, sizeof(struct COFDM_TUNER_CONFIG_API));
    priv_mem->tuner_id = tuner_cnt;
    dev->priv = (void*)priv_mem;

    tuner_cnt++;

    /* Function point init */

    dev->base_addr = priv_mem->Tuner_Control.ext_dm_config.i2c_base_addr;
    dev->init = f_de202_attach;
    dev->open = f_de202_open;
    dev->stop = f_de202_close;//Sam
    dev->disable=f_de202_disable;//Sam
    dev->do_ioctl = f_de202_ioctl;//nim_m3327_ioctl;//050810 yuchun for stop channel search
    dev->channel_change = f_de202_channel_change;
    dev->channel_search = (void *)f_de202_channel_search;//NULL;//f_de202_channel_search;//nim_m3327_channel_search;
    dev->get_lock = f_de202_get_lock;
    dev->get_freq = f_de202_get_freq;
    dev->get_fec = f_de202_get_code_rate;
    dev->get_agc = (void *)f_de202_get_AGC;
    dev->get_snr = f_de202_get_SNR;
    dev->get_ber = f_de202_get_BER;
    dev->get_guard_interval = f_de202_get_GI;
    dev->get_fftmode = f_de202_get_fftmode;
    dev->get_modulation = f_de202_get_modulation;
    dev->get_spectrum_inv = f_de202_get_specinv;
//    dev->get_HIER= f_de202_get_hier_mode;
//    dev->get_priority=f_de202_priority;
    dev->get_freq_offset =    f_de202_get_freq_offset;

    f_de202_IIC_Sema_ID=osal_semaphore_create(1);

    /*if((((struct nim_de202_private*)dev->priv)->Tuner_Control.tuner_config.cChip)!=Tuner_Chip_QUANTEK )
    {
        ptrCOFDM_Tuner->tuner_config.Tuner_Write=NULL;
        ptrCOFDM_Tuner->tuner_config.Tuner_Read=NULL;
    }*/

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    nim_de202[priv_mem->tuner_id] = dev;

#if 0

    if (((struct nim_de202_private*)dev->priv)->Tuner_Control.nim_Tuner_Init != NULL)
    {
        if( (((struct nim_de202_private*)dev->priv)->Tuner_Control.tuner_config.cChip)==Tuner_Chip_MAXLINEAR)
        {
        //_de202_PassThrough(dev,1);

            if (((struct nim_de202_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(&((struct nim_de202_private*)dev->priv)->tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");
                return ERR_NO_DEV;
            }
        //_de202_PassThrough(dev,0);
        }
        else
        {
            if (((struct nim_de202_private*)dev->priv)->Tuner_Control.nim_Tuner_Init(priv_mem->tuner_id, &(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
            {
                NIM_PRINTF("Error: Init Tuner Failure!\n");

                return ERR_NO_DEV;
            }

        }

    }
#endif

    return SUCCESS;
}



/*****************************************************************************
* INT32 f_de202_open(struct nim_device *dev)
* Description: de202 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_open(struct nim_device *dev)
{

    //UINT8    data = 0;
    //UINT8    tdata[5] ;
   // UINT8    lock = 0;
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    //struct COFDM_TUNER_CONFIG_API * config_info = &(priv_mem->Tuner_Control);


    NIM_PRINTF("f_de202_open\n");

    nim_de202_flag[priv_mem->tuner_id] = osal_flag_create(NIM_de202_FLAG_ENABLE);


    if (nim_de202_flag[priv_mem->tuner_id]==OSAL_INVALID_ID)
        return ERR_FAILUE;

    de202_CurChannelInfo[priv_mem->tuner_id] = (struct de202_Lock_Info *)MALLOC(sizeof(struct de202_Lock_Info));
    if (de202_CurChannelInfo[priv_mem->tuner_id] == NULL)
    {
        NIM_PRINTF("f_de202_open MALLOC fail!\n");
        return ERR_FAILUE;
    }

    MEMSET(de202_CurChannelInfo[priv_mem->tuner_id], 0, sizeof(struct de202_Lock_Info));

#ifdef DVBT_ENABLE
     /* tuner init */
            tuner_init();
     /* demod sleep */
            dvbt_demod_init();
     /* tuner awake */
        dvbt_tuner_wakeup();
     /* demod wakeup */
        dvbt_demod_wakeup();
#else
 /* tuner init */
            dvbt2_tuner_init();
     /* demod sleep */
            dvbt2_demod_init();
      /* tuner awake */
            dvbt2_tuner_wakeup();
        /* demod wakeup */
            dvbt2_demod_wakeup();

#endif

        return SUCCESS;
}
/*****************************************************************************
* void  f_de202_get_chip_id(struct nim_device *dev)
* Description:  get  de202chip_id
*
* Arguments: struct nim_device *dev*
*
* Return Value:
*****************************************************************************/

void f_de202_get_chip_id(struct nim_device *dev)
{
    //UINT8 data[1];

}

void f_de202_PassThrough(struct nim_device *dev,BOOL Open)
{
    //UINT8 data;
}

/*****************************************************************************
* INT32 f_de202_close(struct nim_device *dev)
* Description: de202 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_close(struct nim_device *dev)
{
    //UINT8 data = 0;


    return SUCCESS;
}



/*****************************************************************************
* INT32 f_de202_disable(struct nim_device *dev)
* Description: de202 disable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_de202_disable(struct nim_device *dev)
{
    //UINT8 data = 0;

    return SUCCESS;
}
/*****************************************************************************
* INT32 f_de202_channel_search(struct nim_device *dev, UINT32 freq);

* Description: de202 channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency*
* Return Value: INT32
*****************************************************************************/
//050803 yuchun  add
static INT32 f_de202_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority)
{
    //UINT8     i,data[1];
    INT32    chsearch=ERR_FAILED; //,center_freq;
    //INT32 freq_offset_reg;

    // UINT32    Center_NPRO,Search_NPRO,tmp_freq;
    //UINT32 First_Frequency;
    //UINT8 num,freq_step;
    //UINT8 j=0,k = 0;
    //INT32 get_freq_offset;
    //INT32 remainder;
    //UINT16 temp_Step_Freq;

    chsearch=f_de202_channel_change(dev,freq,bandwidth,guard_interval,fft_mode,modulation,fec,usage_type,inverse, priority);
    return chsearch;
}


static INT32 f_de202_freq_offset(struct nim_device *dev, UINT32 bandwidth, UINT8 fft_mode, INT32 *freq_offset)
{
    //UINT8 data[4];
    //UINT8 mode;
    //UINT32 freq_offset_tmp = 0;

     *freq_offset=0;
    return SUCCESS;
}



static int get_dvbt_stat(UINT32 bandwidth)
{
    UINT8   SyncStat;
    UINT8   TSLockStat;
    UINT16  IFAGCOut;
    INT16   RSSI;                           /* 2010-05-21 add */
    INT32   Offset;
    INT16   CN_q2;
    UINT32  BitError;
    UINT32  Period;
    UINT16  RSError;
    T_TPSInfo Info;
    T_DigitalTuningParam param;

switch(bandwidth)
        {
          case 6:
            param.m_BandWidth = 1;
            break;

          case 7:
            param.m_BandWidth = 2;
            break;

          case 8:
            param.m_BandWidth = 3;
            break;

          default:
            param.m_BandWidth = 3;
    }

    dvbt_get_syncstat(&SyncStat, &TSLockStat);
    dvbt_get_ifagc(&IFAGCOut);
    dvbt_get_rssi(&RSSI);                   /* 2010-05-21 add */
    dvbt_get_CarrierOffset(param.m_BandWidth, &Offset);
    dvbt_get_CN(&CN_q2);
    dvbt_get_PreRSBer(&BitError, &Period);
    dvbt_get_TPSInfo(&Info);
    dvbt_get_RSError(&RSError);
    /* SyncStat */
    libc_printf("\n--------------------------------\n");
    if( SyncStat == 6 ){
        libc_printf("TPS Locked | ");
    }else{
        libc_printf("Not TPS Locked(%d) | ", SyncStat);
    }
    if( TSLockStat == 1 ){
        libc_printf("TS Locked\n");
    }else{
        libc_printf("TS Not Locked\n");
    }
    /* IFAGC { 0.0374 * IIFAGC_OUT - 101.52 } */
    libc_printf("IFAGCOut(0...4095): %04u \n", IFAGCOut);
    libc_printf("RSSI [dBm]        : %4d.%01d \n", RSSI/10, abs(RSSI % 10) );    /* 2010-05-21 add */
    libc_printf("TPS Info \n");

    /* Mode */
    switch(Info.mode){
    case 0:
        libc_printf("  -----mode        = <     2k mode    >\n"); break;
    case 1:
        libc_printf("  -----mode        = <     8k mode    >\n"); break;
    default:
        libc_printf("  -----mode        = < Unknown mode(%d) >\n", Info.mode); break;
    }


    /* Guard */
    switch(Info.guard){
    case 0:
        libc_printf("  -----GI          = <      1/32      >\n"); break;
    case 1:
        libc_printf("  -----GI          = <      1/16      >\n"); break;
    case 2:
        libc_printf("  -----GI          = <      1/8       >\n"); break;
    case 3:
        libc_printf("  -----GI          = <      1/4       >\n"); break;
    default:
        libc_printf("  -----GI          = < Unknown guard(%d) >\n", (INT16)Info.guard); break;
    }

    /* hierarchy */
    switch(Info.hierarchy){
    case 0:
        libc_printf("  -----hierarchy   = <Non hierarchical>\n"); break;
    case 1:
        libc_printf("  -----hierarchy   = <      alpha = 1 >\n"); break;
    case 2:
        libc_printf("  -----hierarchy   = <      alpha = 2 >\n"); break;
    case 3:
        libc_printf("  -----hierarchy   = <      alpha = 4 >\n"); break;
    default:
        libc_printf("  -----hierarchy   = < Unknown Hir(%d) >\n", (INT16)Info.hierarchy); break;
    }

    /* Modulation */
    switch(Info.constellation){
    case 0:
        libc_printf("  -----Modulation  = <      QPSK      >\n"); break;
    case 1:
        libc_printf("  -----Modulation  = <      16QAM     >\n"); break;
    case 2:
        libc_printf("  -----Modulation  = <      64QAM     >\n"); break;
    default:
        libc_printf("  -----Modulation  = < Unknown Mod(%d) >\n", (INT16)Info.constellation); break;
    }

    /* rateHP */
    switch(Info.rateHP){
    case 0:
        libc_printf("  -----CodeRate HP = <      1/2       >\n"); break;
    case 1:
        libc_printf("  -----CodeRate HP = <      2/3       >\n"); break;
    case 2:
        libc_printf("  -----CodeRate HP = <      3/4       >\n"); break;
    case 3:
        libc_printf("  -----CodeRate HP = <      5/6       >\n"); break;
    case 4:
        libc_printf("  -----CodeRate HP = <      7/8       >\n"); break;
    default:
        libc_printf("  -----CodeRate HP = < Unknown CR(%d) >\n", (INT16)Info.rateHP); break;
    }

    /* rateLP */
    switch(Info.rateLP){
    case 0:
        libc_printf("  -----CodeRate LP = <      1/2       >\n"); break;
    case 1:
        libc_printf("  -----CodeRate LP = <      2/3       >\n"); break;
    case 2:
        libc_printf("  -----CodeRate LP = <      3/4       >\n"); break;
    case 3:
        libc_printf("  -----CodeRate LP = <      5/6       >\n"); break;
    case 4:
        libc_printf("  -----CodeRate LP = <      7/8       >\n"); break;
    default:
        libc_printf("  -----CodeRate LP = < Unknown CR(%d) >\n", (INT16)Info.rateLP); break;
    }

    /* Cell ID */
    libc_printf("  -----cell ID     = <       %2d       >\n",Info.cellID);

    /* CarrierOffset */
    libc_printf("CarrierOffset     = %ld kHz\n", Offset);
    /* CN */
    libc_printf("CN                = %d.%02d [dB] \n", ( CN_q2 / 4 ), ( 0x03 & CN_q2 ) * 25 );
    /* preRS-BER */
    libc_printf("PreRSBER :BitError=%8ld Period=%8ld BER=%e\n", BitError, Period, (float)BitError/(float)Period);
    /* RSError */
    libc_printf("RSError  :BitError=%8d \n", RSError);
    libc_printf("--------------------------------\n");

    return 0;
}

void demp_reg(UINT8 bank){

    UINT16 rdata=0,i=0,j=0;

for(j=0;j<=0xff;j++){

    libc_printf("Bank[%x]\n",j);
    iic_write_val( demod_block_id, 0x00, j );

    for(i=0;i<=0xff;i++){

     iic_read( demod_block_id, i, (UINT8 *)&rdata, 1 );
    libc_printf("reg[%x]=0x%x \n",i,rdata);

    }
}

}

static int get_dvbt2_stat(UINT32 bandwidth)
{
    UINT8   SyncStat;
    UINT8   TSLockStat;
    UINT16  IFAGCOut;
    INT16   RSSI;                           /* 2010-05-21 add */
    INT32   Offset;
    INT16   CN_q2;
    UINT32  PreBchBitError, PreBchBerPeriod;
    UINT32  PostBchFrameError, PostBchFerPeriod;
    T2_DigitalTuningParam   param;
    /* 2010-06-21 delete T2_DemodInfo Info : This data has been merged to L1PreData */
    UINT8           Current_PLP_ID;         /* 2010-05-21 add */
    PLPSEL_FLAG     PlpSelFlag;             /* 2010-05-21 add */
    UINT8   plpNum;
    UINT8   plpId[255];
    L1CHG_RECOVERY_FLAG L1ChgRecoveryFlag;  /* 2010-05-21 add */
    T2_L1PreInfo  L1PreData;                /* 2010-06-21 add */
    T2_L1PostInfo L1PostData;               /* 2010-06-21 add */
    UINT16  SSI;                            /* 2010-10-15 add */
    UINT16  SQI;                            /* 2010-10-15 add */



    switch(bandwidth)
    {
          case 6:
            param.m_BandWidth = 1;
            break;

          case 7:
            param.m_BandWidth = 2;
            break;

          case 8:
            param.m_BandWidth = 3;
            break;

          default:
            param.m_BandWidth = 3;
    }


    if( ( param.m_BandWidth != 2 )&&( param.m_BandWidth != 3 ) ) {
        libc_printf("not supported bandwidth\n");
        return -1;
    }

    dvbt2_get_syncstat(&SyncStat, &TSLockStat);
    dvbt2_get_ifagc(&IFAGCOut);
    dvbt2_get_rssi(&RSSI);                  /* 2010-05-21 add */
    dvbt2_get_CarrierOffset(param.m_BandWidth, &Offset);
    dvbt2_get_CN(&CN_q2);
    dvbt2_get_PreBCHBer(&PreBchBitError, &PreBchBerPeriod);
    dvbt2_get_PostBCHFer(&PostBchFrameError, &PostBchFerPeriod);
    /* 2010-06-21 delete dvbt2_get_T2DemodInfo -> This monitoring function has been merged to dvbt2_get_T2L1PreInfo  */
    dvbt2_get_T2PLPSelFlag(&PlpSelFlag, &Current_PLP_ID);    /* 2010-05-21 add */
    dvbt2_get_PlpIdInfo(&plpNum, plpId);

    /* SyncStat */
    libc_printf("\n--------------------------------------------\n");
    if(SyncStat == 6){
        libc_printf("-----Demod       = <     LOCKED     >\n");
    }else{
        libc_printf("-----Demod       = <    UNLOCKED    >\n");
    }
    if(TSLockStat){
        libc_printf("-----TS OutPut   = <     LOCKED     >\n");
    }else{
        libc_printf("-----TS OutPut   = <    UNLOCKED    >\n");
    }
    /* IFAGC */

    libc_printf("-----IFAGC out   = <     %4d       >\n", IFAGCOut);
    libc_printf("-----RSSI [dBm]    <     %4d.%01d    >\n", RSSI/10, abs(RSSI % 10) ); /* 2010-05-21 add */

    /* CarrierOffset */
    libc_printf("--Cariier Offset = <     %4ld kHz   >\n", Offset);

    /* CN */
    libc_printf("-----CN          = <     %2d.%02d      >\n", ( CN_q2 / 4 ), ( 0x03 & CN_q2 ) * 25);


    /* PreBCH-Ber */
    libc_printf("-----Pre BCH-Ber = <   BitError=%ld Period=%ld >\n", PreBchBitError, PreBchBerPeriod);

    /* PostBCH-Fer */
    libc_printf("-----PostBCH-Fer = < FrameError=%ld Period=%ld >\n", PostBchFrameError, PostBchFerPeriod);

    /* 2010-06-21 demod info output has been removed. This monitoring was merged to dvbt2_print_T2L1PreInfo */

    dvbt2_print_PlpIdInfo(plpNum, plpId);

    /* PLP Sel Error Flag */ /* 2010-05-21 add */
    switch(PlpSelFlag){
    case PLPSEL_OK:
        libc_printf("-----PLP Sel     = <   PLP Sel OK    >\n"); break;
    case PLPSEL_ERROR:
        libc_printf("-----PLP Sel     = <   PLP Sel Error >\n"); break;
    default:
        libc_printf("-----PLP Sel     = <  Unknown PLP Sel Error Flag (%d) >\n", PlpSelFlag); break;
    }

    /* Current PLP_ID */ /* 2010-05-21 add */
    libc_printf("Current_PLP_ID   = <       %4d      >\n", Current_PLP_ID);

    /* L1 Change Recovery */ /* 2010-05-21 add */
    dvbt2_L1ChangeRecovery( &L1ChgRecoveryFlag );
    switch(L1ChgRecoveryFlag){
    case NO_CHANGE:
        libc_printf("-----L1Change    = < No Change       >\n"); break;
    case CHANGE_AND_SRST:
        libc_printf("-----L1Change    = < Change and SRST >\n"); break;
    default:
        libc_printf("-----L1Change    = < No Change  (%d) >\n", L1ChgRecoveryFlag); break;
    }

    /* L1 Pre/Post Signaling Data */
    dvbt2_get_T2L1PreInfo(&L1PreData);              /* 2010-06-21 add */
    dvbt2_get_T2L1PostInfo(&L1PostData);            /* 2010-06-21 add */

    dvbt2_print_T2L1PreInfo(&L1PreData);            /* 2010-06-21 add */
    dvbt2_print_T2L1PostInfo(&L1PostData);          /* 2010-06-21 add */

    /* SSI/SQI */ /* 2010-10-15 add */
    dvbt2_get_ssi_sqi(&SQI,&SSI);
    libc_printf("SSI %d\n",SSI);
    libc_printf("SQI %d\n",SQI);
    return 0;
}



/*****************************************************************************
* INT32 f_de202_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type,UINT8 inverse);

* Description: stv0360 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency
*  Parameter3: UINT32 bandwidth
*  Parameter4: UINT8  guard_interval
*  Parameter5: UINT8  fft_mode
*  Parameter6: UINT8  modulation
*  Parameter7: UINT8  fec
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority)
{

    //UINT8    data[6];
    __MAYBE_UNUSED__ UINT8    ret_flag = SCAN_TIMEOUT;
    __MAYBE_UNUSED__ TMO     start_time, temp_time;
    struct nim_de202_private *dev_priv;
    UINT8 code_rate = 0;
    //UINT8 lock=0,i=0;
//================================
    __MAYBE_UNUSED__ T_DigitalTuningParam    param;
    __MAYBE_UNUSED__ T_TPS_LOCK              TPS_Lock;
        T_TS_LOCK               TS_Lock = TS_UNKNOWN;
        INT32                   offset;

    T2_DigitalTuningParam   dvbt2_param;
        T2_DEM_LOCK             demLockResult;
        T2_TS_LOCK              tsLockResult = DVBT2_TS_UNKNOWN;
    UINT8 PlpIdNum;
    UINT8 plpId[255];


//================================
    dev_priv = (struct nim_de202_private *)dev->priv;
    UINT32 tuner_id =dev_priv->tuner_id;

    libc_printf("freq %d, bandwidth %d ,system=%d\n",freq,bandwidth,inverse);

    osal_flag_clear(nim_de202_flag[tuner_id], NIM_de202_SCAN_END);

    de202_CurChannelInfo[tuner_id]->Frequency = freq;
    de202_CurChannelInfo[tuner_id]->ChannelBW = bandwidth;


    dev_priv = dev->priv;


#ifdef DVBT_ENABLE
      param.m_frequency =freq;

//if(inverse){
    switch(bandwidth)
        {
          case 6:
            param.m_BandWidth = 1;
            break;

          case 7:
            param.m_BandWidth = 2;
            break;

          case 8: 
            param.m_BandWidth = 3;
            break;

          default:
            param.m_BandWidth = 3;
    }

    if(priority)
        param.m_hierarchy = 0;
     else
         param.m_hierarchy = 0;

//====================================================

    /* demod setting */
    dvbt_demod_setparam(&param);

    /* tuner setting */
    dvbt_tuner_tune( &param );

    /* Lock Sequence */
    LockSequence( param.m_BandWidth, &TPS_Lock, &TS_Lock, &offset);

    //get_dvbt_stat(bandwidth);


       /* In case of carrier offset > threshold, Go back to tuner setting */
    if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
        libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( param.m_frequency + offset ) );

        param.m_frequency += offset;

        dvbt_tuner_tune( &param );

        /* wait DVB-T lock */
        LockSequence( param.m_BandWidth, &TPS_Lock, &TS_Lock, &offset);

        libc_printf("Tuning Freq   : %ld [kHz]\n",param.m_frequency);
        libc_printf("CarrierOffset : %ld [kHz]\n",offset);
        libc_printf("TPS_Lock      : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TPS_Lock);
        libc_printf("TS_Lock       : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TS_Lock);
    }
//}
#else
//else{

//Reference to sample code: cmd_tune_dvbt2()

        /* tuner awake */
            dvbt2_tuner_wakeup();
        /* demod wakeup */
            dvbt2_demod_wakeup();

    dvbt2_param.m_frequency = freq;

    switch(bandwidth)
        {
          case 6:
            dvbt2_param.m_BandWidth = 1;
            break;

          case 7:
            dvbt2_param.m_BandWidth = 2;
            break;

          case 8:
            dvbt2_param.m_BandWidth = 3;
            break;

          default:
            dvbt2_param.m_BandWidth = 3;
    }


    if( ( dvbt2_param.m_BandWidth != 2 )&&( dvbt2_param.m_BandWidth != 3 ) )
    {
            libc_printf("not supported bandwidth now \n");
        osal_flag_set(nim_de202_flag[tuner_id],NIM_de202_SCAN_END);
            return -1;
    }


    dvbt2_get_PlpIdInfo(&PlpIdNum, plpId);
    dvbt2_param.m_MplpSelect = 0;/* 0:DefaultPLP  1:PLP selection  */
    dvbt2_param.m_MplpId = 0;


    /* demod setparam */
    dvbt2_demod_setparam(&dvbt2_param );

    dvbt2_tuner_tune( &dvbt2_param );

    /* 2010-10-05 LongEchoSeq_Ena is used to select DVB-T2 Acquisition Sequnece.
          LongEchoSeq_Ena = 0 : DVB-T2 Lock Sequence
          LongEchoSeq_Ena = 1 : DVB-T2 Long Echo Sequence
    */
    if (LongEchoSeq_Ena==0)
    {
        /* DVB-T2 Lock Sequence */
        /* wait DVB-T2 lock */
        demodLockSequence(&dvbt2_param, &demLockResult, &tsLockResult, &offset);    /* demod Lock Sequence */

        /* If carrier offset > threshold, Go back to tuner setting */
        if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
            libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( dvbt2_param.m_frequency + offset ) );
    
            dvbt2_param.m_frequency += offset;

            dvbt2_tuner_tune( &dvbt2_param );

                /* wait DVB-T2 lock */
            demodLockSequence(&dvbt2_param, &demLockResult, &tsLockResult, &offset);    /* demod Lock Sequence */
        }
    }
    else {
        /* 2010-10-05 DVB-T2 Long Echo Sequence has been added */
        /* Demod SRST */
        dvbt2_demod_start();

        Waitms(100);

        /* wait DVB-T2 demod lock */
        t2_le_Seq(&demLockResult);

        if( demLockResult == DEM_LOCK ) {
            /* Check Carrier Offset */
            dvbt2_get_CarrierOffset(dvbt2_param.m_BandWidth, &offset);
            libc_printf("CarrierOffset %ld \n",offset);

            /* If carrier offset > threshold, Go back to tuner setting */
            if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
                libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( dvbt2_param.m_frequency + offset ) );

                dvbt2_param.m_frequency += offset;

                dvbt2_demod_setparam(&dvbt2_param);

                dvbt2_tuner_tune(&dvbt2_param);

                /* Demod SRST */
                dvbt2_demod_start();

                Waitms(100);

                /* wait DVB-T2 demod lock */
                t2_le_Seq(&demLockResult);

                if( demLockResult == DEM_LOCK ) {
                    /* wait DVB-T2 TS lock */
                    t2_le_SeqTsLock(&tsLockResult);
                } else {
                    /* Demod Unlock */
                }
            } else {
                /* wait DVB-T2 TS lock */
                t2_le_SeqTsLock(&tsLockResult);
            }
        }
        libc_printf("DemodLock %d\n",demLockResult);
        libc_printf("TSLock    %d\n",tsLockResult);
    }

#endif
//}
       dvbt_tsout( TRUE );

    /*-------------------------------------------------------------------------*/
    /* (7)Status update:                            */
    /*-------------------------------------------------------------------------*/

    if (TS_Lock == TS_LOCK||tsLockResult == DVBT2_TS_LOCK )
    {
        //get_dvbt2_stat(bandwidth);

        de202_CurChannelInfo[tuner_id]->Frequency = freq;
        de202_CurChannelInfo[tuner_id]->ChannelBW = (UINT8)bandwidth;

//        f_de202_getinfo(dev, &code_rate, &guard_interval, &fft_mode, &modulation);

        //MT352_CurChannelInfo->HPRates = code_rate;
        de202_CurChannelInfo[tuner_id]->Guard= guard_interval;
        de202_CurChannelInfo[tuner_id]->Mode= fft_mode;
        de202_CurChannelInfo[tuner_id]->Modulation= modulation;
        de202_CurChannelInfo[tuner_id]->FECRates = code_rate;

        de202_CurChannelInfo[tuner_id]->Hierarchy = f_de202_hier_mode(dev);
        de202_CurChannelInfo[tuner_id]->Priority=priority;
        de202_CurChannelInfo[tuner_id]->lock_status=1;

        NIM_de202_PRINTF("SUCCESS end_time = %d\n",osal_get_tick()-start_time);

        osal_flag_set(nim_de202_flag[tuner_id],NIM_de202_SCAN_END);
        return SUCCESS;
    }
    else
    {
          de202_CurChannelInfo[tuner_id]->lock_status=0;
        NIM_de202_PRINTF("ERR_FAILED end_time = %d ret=%d\n",osal_get_tick()-start_time,ret_flag);

        osal_flag_set(nim_de202_flag[tuner_id],NIM_de202_SCAN_END);
        return ERR_FAILED;
    }
}



static INT32 f_de202_getinfo(struct nim_device *dev, UINT8 *code_rate, UINT8 *guard_interval, UINT8 *fft_mode, UINT8 *modulation)
{
//    UINT8 data[2];
//    UINT16 tdata;
//    UINT16 i;
struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
  __MAYBE_UNUSED__ MXL_STATUS status = MXL_TRUE;

  __MAYBE_UNUSED__ MXL_TUNER_CHAN_SCAN_CFG_T mxlChanScanCfg;
  MXL_TUNER_CHECK_CHAN_REQ_T de202sfChanScan;

  __MAYBE_UNUSED__ MXL_DEMOD_SNR_INFO_T Snr;
  __MAYBE_UNUSED__ MXL_DEMOD_BER_INFO_T Ber;
  MXL_DEMOD_TPS_INFO_T TpsData;

  __MAYBE_UNUSED__ MXL_TUNER_CHAN_OFFSET_T mxlChanOffset;
  __MAYBE_UNUSED__ MXL_DEMOD_CELL_ID_INFO_T TpsCellIdInfo;
  MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;

  MEMSET(&tpsLockStatus, 0x0, sizeof (tpsLockStatus));
     // Check untill RS lock is achived for each channel frequency
    de202sfChanScan.ChanScanCtrl = MXL_BREAK_AT_CP_LOCK; // MXL_FULL_MODE

    // Enable TPS Cell Id feature if required, else Disable it
    de202sfChanScan.TpsCellIdRbCtrl = MXL_DISABLE;

    // Frequency of the channel
    de202sfChanScan.Frequency = de202_CurChannelInfo[priv_mem->tuner_id]->Frequency;

    // Bandwidth for the channel
    de202sfChanScan.Bandwidth = de202_CurChannelInfo[priv_mem->tuner_id]->ChannelBW;

    NIM_de202_PRINTF("Scanning for %d Hz, bandwidth= %d .....\n", de202sfChanScan.Frequency, de202sfChanScan.Bandwidth);

   // API to check for the presence of the channel
    // If the channel is detected then "de202sfChanScan.ChanPresent"
    // will contain value "MXL_TRUE"
//    status = MxLWare_API_GetTunerStatus(MXL_TUNER_CHECK_CHAN_STATUS_REQ, &de202sfChanScan);

    // Check if Channel is detected or not
//    if (de202sfChanScan.ChanPresent == MXL_TRUE)
    {
      // If Channel is detected record all the necessary
      // staus of the channel or de202SF

      // Before recording channel status make sure TPS Lock is present
      //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_LOCK_REQ, &tpsLockStatus);
      if (tpsLockStatus.Status == MXL_LOCKED)
      {
        NIM_de202_PRINTF ("\n Frequency Detected : %d Hz\n", de202sfChanScan.Frequency);

        // Record Frequency offset
       // MxLWare_API_GetTunerStatus(priv_mem->tuner_id, MXL_TUNER_CHAN_OFFSET_REQ, &mxlChanOffset);
        NIM_de202_PRINTF ("Frequency Offset : %d Hz\n", mxlChanOffset.ChanOffset);

        // SNR
        //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_SNR_REQ, &Snr);
        NIM_de202_PRINTF ("SNR : %f dB\n", (REAL32)(Snr.SNR/10000));

        // BER
        //.MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_BER_REQ, &Ber);
        NIM_de202_PRINTF ("BER : %f\n", (REAL32)(Ber.BER/100));

        // TPS Cell Id
        if (de202sfChanScan.TpsCellIdRbCtrl == MXL_ENABLE)
        {
          //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CELL_ID_REQ, &TpsCellIdInfo);
          NIM_de202_PRINTF ("TPS Cell ID : %d\n", TpsCellIdInfo.TpsCellId);
        }

        // TPS Code Rate
        //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CODE_RATE_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_de202_PRINTF ("TPS Code Rate : 1/2\n");
            *code_rate =FEC_1_2;
            break;

          case 1:
            NIM_de202_PRINTF ("TPS Code Rate : 2/3\n");
            *code_rate =FEC_2_3;
            break;

          case 2:
            NIM_de202_PRINTF ("TPS Code Rate : 3/4\n");
            *code_rate =FEC_3_4;
            break;

          case 3:
            NIM_de202_PRINTF ("TPS Code Rate : 5/6\n");
            *code_rate =FEC_5_6;
            break;

          case 4:
            NIM_de202_PRINTF ("TPS Code Rate : 7/8\n");
            *code_rate =FEC_7_8;
            break;

          default:
            *code_rate = 0; /* error */
            NIM_de202_PRINTF ("TPS Code Rate : INVALID CR!\n");
        }

        // TPS Constellation
        //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_CONSTELLATION_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_de202_PRINTF ("TPS Constellation : QPSK\n");
            * modulation = TPS_CONST_QPSK;
            break;

          case 1:
            NIM_de202_PRINTF ("TPS Constellation : 16QAM\n");
            * modulation = TPS_CONST_16QAM;
            break;

          case 2:
            NIM_de202_PRINTF ("TPS Constellation : 64QAM\n");
            * modulation = TPS_CONST_64QAM;
            break;

          default:
            NIM_de202_PRINTF ("TPS Constellation : INVALID TPS CONSTELLATION!\n");
            * modulation = 0xff; // error
        }

        // TPS GI
        //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_GUARD_INTERVAL_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/32\n");
            *guard_interval = guard_1_32;
            break;

          case 1:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/16\n");
            *guard_interval = guard_1_16;
            break;

          case 2:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/8\n");
            *guard_interval = guard_1_8;
            break;

          case 3:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/4\n");
            *guard_interval = guard_1_4;
            break;

          default:
            *guard_interval = 0xff; /* error */
            NIM_de202_PRINTF ("TPS Code Rate : INVALID GUARD INTERVAL!\n");
        }

        // FFT
        //MxLWare_API_GetDemodStatus(priv_mem->tuner_id, MXL_DEMOD_TPS_FFT_MODE_REQ, &TpsData);
        switch(TpsData.TpsInfo)
        {
          case 0:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/32\n");
            *fft_mode = MODE_2K;
            break;

          case 1:
            NIM_de202_PRINTF ("TPS Guard Interval : 1/16\n");
            *fft_mode = MODE_8K;
            break;

          default:
            NIM_de202_PRINTF ("FFT : INVALID !\n");
            *fft_mode = 0xff; // error
        }

      } // End of if (tpsLockStatus.Status == MXL_LOCKED)
    } // End of if (de202sfChanScan.ChanPresent == MXL_TRUE)

    return SUCCESS;
}



/************************Zarlink Semiconductor*********************************
*   Name:Tnim_PostViterbiBER
*   Purpose:calculates Post-viterbi BER
*   Remarks:formula is RS_BERCNT/(RS_ERRPER*1024*204*8)
*           the result is multiplied by 100E6 to get an integer value, the
*            formula becomes
*            BER = 100e6/(1024*204*8) *RS_BERCNT/RS_ERRPER = 60*RS_BERCNT/RS_ERRPER
*                    RS_ERRPER=77
*   Inputs:
*   Outputs:  BER * 100E6 , or -1 if an error occurs
*******************************************************************************/


INT32 de202_PostViterbi_BER_Read(struct nim_device *dev)
{
      //UINT8  Interrupt_2;

         return 0;
}



/*****************************************************************************
* INT32 f_de202_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/

static INT32 f_de202_get_lock(struct nim_device *dev, UINT8 *lock)
{
    __MAYBE_UNUSED__ UINT8 rdata=0;
    OSAL_ER    result;
       UINT32    flgptn;
    __MAYBE_UNUSED__ static UINT32 dd_time=0;
    //UINT8 ber_vld;
    //UINT32 m_vbber, m_per;
    struct nim_de202_private *dev_priv;

    dev_priv = (struct nim_de202_private *)dev->priv;
    UINT32 tuner_id = dev_priv->tuner_id;
    UINT16  SSI;                            /* 2010-10-15 add */
    UINT16  SQI;                            /* 2010-10-15 add */
    UINT8   SyncStat;
    UINT8   TSLockStat;


    result = osal_flag_wait(&flgptn,nim_de202_flag[tuner_id], NIM_de202_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("lock result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            //libc_printf("Get LOCK Fail, Wait ChannelChg Complete!\n");
        *lock=0xff;
    }
    else
    {
#if 1
    #ifdef DVBT_ENABLE

        if(rdata!=0x00)
                 iic_write_val( demod_block_id,  0x00 , 0x00 );

         iic_read( demod_block_id, 0x73, &rdata, 1 );
         rdata=( 0x08 & rdata) >> 3;
    #else
    #if 0
                iic_read( demod_block_id, 0x00, &rdata, 1 );
                if(rdata!=0x20)
                    iic_write_val( demod_block_id,  0x00 , 0x20 );

                iic_read( demod_block_id, 0x10, &rdata, 1 );
                libc_printf("%x\n",rdata);
                 rdata=( 0x20 & rdata) >> 5;
                 iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */
    #else
              dvbt2_get_syncstat(&SyncStat, &TSLockStat);
    #endif
    #endif
#endif
         //libc_printf("rdata=%x\n",rdata);
         if(TSLockStat)//(rdata)
        {
                    *lock = 1;
                  de202_CurChannelInfo[tuner_id]->lock_status=1;

                      dvbt2_get_ssi_sqi(&SQI,&SSI);
                  de202_CurChannelInfo[tuner_id]->sig_strength=SSI;
                  de202_CurChannelInfo[tuner_id]->sig_quality=SQI;



             }
            else
          {
                      *lock = 0;
                 de202_CurChannelInfo[tuner_id]->lock_status=0;
                  de202_CurChannelInfo[tuner_id]->sig_strength=0;
                  de202_CurChannelInfo[tuner_id]->sig_quality=0;
        }

      }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_get_freq(struct nim_device *dev, UINT32 *freq)
* Read M3327 frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *freq            :
*
* Return Value: void
*****************************************************************************/
static INT32 f_de202_get_freq(struct nim_device *dev, UINT32 *freq)
{
    //INT32 freq_off=0;
    //UINT8  data[3];
    //INT32  tdata=0;
    //    OSAL_ER    result;
    //    UINT32    flgptn;
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
        //*freq=de202_CurChannelInfo->Frequency+de202_CurChannelInfo->FreqOffset;
        *freq=de202_CurChannelInfo[priv_mem->tuner_id]->Frequency;
        NIM_de202_PRINTF("freq=%d KHz \n", *freq);

    return SUCCESS;

}

/*****************************************************************************
* INT32 f_de202_get_code_rate(struct nim_device *dev, UINT8* code_rate)
* Description: Read de202 code rate
*   FEC status (b6-b4)                      code rate                 return value
*    0                    1/2            1
*    1                    2/3            4
*    2                    3/4            8
*    3                    5/6            16
*    5                    7/8            32
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
    //MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    //MXL_DEMOD_TPS_INFO_T TpsData;
//    *code_rate = de202_CurChannelInfo->FECRates;
   //struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);


    switch(4)
    {
      case 0:
        NIM_de202_PRINTF ("TPS Code Rate : 1/2\n");
        *code_rate =FEC_1_2;
        break;

      case 1:
        NIM_de202_PRINTF ("TPS Code Rate : 2/3\n");
        *code_rate =FEC_2_3;
        break;

      case 2:
        NIM_de202_PRINTF ("TPS Code Rate : 3/4\n");
        *code_rate =FEC_3_4;
        break;

      case 3:
        NIM_de202_PRINTF ("TPS Code Rate : 5/6\n");
        *code_rate =FEC_5_6;
        break;

      case 4:
        NIM_de202_PRINTF ("TPS Code Rate : 7/8\n");
        *code_rate =FEC_7_8;
        break;

      default:
        NIM_de202_PRINTF ("TPS Code Rate : INVALID CR!\n");
        return ERR_FAILURE; /* error */
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_get_GI(struct nim_device *dev, UINT8 *guard_interval)
* Description: Read de202 guard interval
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* guard_interval
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    //MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    //MXL_DEMOD_TPS_INFO_T TpsData;
//    *guard_interval = de202_CurChannelInfo->Guard;
   //struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);


    switch(0)
    {
      case 0:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/32\n");
        *guard_interval = guard_1_32;
        break;

      case 1:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/16\n");
        *guard_interval = guard_1_16;
        break;

      case 2:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/8\n");
        *guard_interval = guard_1_8;
        break;

      case 3:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/4\n");
        *guard_interval = guard_1_4;
        break;

      default:
        NIM_de202_PRINTF ("TPS Code Rate : INVALID GUARD INTERVAL!\n");
        return ERR_FAILURE;
    }

    return SUCCESS;
}


/*****************************************************************************
* INT32 f_de202_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
* Description: Read de202 fft_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* fft_mode
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    //MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    //MXL_DEMOD_TPS_INFO_T TpsData;
//    *fft_mode = de202_CurChannelInfo->Mode;
    //struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);


    switch(1)
    {
      case 0:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/32\n");
        *fft_mode = MODE_2K;
        break;

      case 1:
        NIM_de202_PRINTF ("TPS Guard Interval : 1/16\n");
        *fft_mode = MODE_8K;
        break;

      default:
        NIM_de202_PRINTF ("FFT : INVALID !\n");
        return ERR_FAILURE; // error
    }

    return SUCCESS;

}


/*****************************************************************************
* INT32 f_de202_get_modulation(struct nim_device *dev, UINT8 *modulation)
* Description: Read de202 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* modulation
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_de202_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    //MXL_DEMOD_LOCK_STATUS_T  tpsLockStatus;
    //MXL_DEMOD_TPS_INFO_T TpsData;
//    *modulation = de202_CurChannelInfo->Modulation;
    //struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);


    switch(2)
    {
      case 0:
        NIM_de202_PRINTF ("TPS Constellation : QPSK\n");
        * modulation = TPS_CONST_QPSK;
        break;

      case 1:
        NIM_de202_PRINTF ("TPS Constellation : 16QAM\n");
        * modulation = TPS_CONST_16QAM;
        break;

      case 2:
        NIM_de202_PRINTF ("TPS Constellation : 64QAM\n");
        * modulation = TPS_CONST_64QAM;
        break;

      default:
        NIM_de202_PRINTF ("TPS Constellation : INVALID TPS CONSTELLATION!\n");
        return ERR_FAILURE; // error
    }
    return SUCCESS;

}

/*****************************************************************************
* INT32 f_de202_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    //UINT8    data;
    UINT32    flgptn;
    OSAL_ER    result;
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    result = osal_flag_wait(&flgptn,nim_de202_flag[priv_mem->tuner_id], NIM_de202_SCAN_END, OSAL_TWF_ANDW,0);
    if(OSAL_E_OK!=result)
    {
        *Inv=0;
    }
    else
    {
        *Inv=0;// 1:inv 0:non-inv
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_AGC(struct nim_device *dev, UINT16 *agc)
{
     OSAL_ER    result;
     UINT32        flgptn;

     // UINT8 status;
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    //UINT32 tuner_id = priv_mem->tuner_id;


    result = osal_flag_wait(&flgptn,nim_de202_flag[priv_mem->tuner_id], NIM_de202_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("AGC result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get AGC Fail, Wait ChannelChg Complete!\n");
        *agc = 0;
    }
    else
    {

        if( de202_CurChannelInfo[priv_mem->tuner_id]->lock_status==1)
        {

             *agc=  de202_CurChannelInfo[priv_mem->tuner_id]->sig_strength;
            if(*agc>100)
                *agc=100;

        }
        else
        {
            *agc=0;
        }
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *snr
*
* Return Value: * snr

*****************************************************************************/
static INT32 f_de202_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    //UINT8    data;
    OSAL_ER result;
    UINT32     flgptn;
    // struct nim_de202_private *dev_priv;
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    //UINT32 tuner_id = priv_mem->tuner_id;

    result = osal_flag_wait(&flgptn,nim_de202_flag[priv_mem->tuner_id], NIM_de202_SCAN_END, OSAL_TWF_ANDW,0);
    //libc_printf("SNR result = %d\r\n",result);
    if(OSAL_E_OK!=result)
    {
        NIM_PRINTF("Get SNR Fail, Wait ChannelChg Complete!\n");
        *snr = 0;
        return ERR_FAILURE;
    }
    else
    {
        *snr= (UINT8) de202_CurChannelInfo[priv_mem->tuner_id]->sig_quality;
    }
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_de202_get_BER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    //MXL_DEMOD_BER_INFO_T Ber;
    //struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);

       *vbber = 0;
    return SUCCESS;
}

static INT32 f_de202_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)    //50622-01Angus
{
    static UINT32 rec_ber_cnt = 0;
    nim_rec_performance_t * p_nim_rec_performance;
    INT32 ret_val = ERR_FAILUE;

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            de202_autoscan_stop_flag = param;
        break;

        case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
            p_nim_rec_performance = (nim_rec_performance_t *)param;
            f_de202_get_lock(dev, &(p_nim_rec_performance->lock));

            if (p_nim_rec_performance->lock == 1)
            {
                if (rec_ber_cnt !=de202_per_tot_cnt)
                {
                    rec_ber_cnt = de202_per_tot_cnt;
                    p_nim_rec_performance->ber = de202_ber;
                    p_nim_rec_performance->per = de202_per;
                    p_nim_rec_performance->valid = 1;
                }
                else
                {
                    p_nim_rec_performance->valid = 0;
                }
            }

            ret_val = SUCCESS;

            break;

        default:
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_get_freq_offset(struct nim_device *dev,INT32 *freq_offset)
* Description: Read de202 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: INT32 *freq_offset
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_de202_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)//051222 yuchun
{
        OSAL_ER    result;
        UINT32    flgptn;
        struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
        result = osal_flag_wait(&flgptn,nim_de202_flag[priv_mem->tuner_id], NIM_de202_SCAN_END, OSAL_TWF_ANDW,0);
       // libc_printf("offset result = %d\r\n",result);

    if(OSAL_E_OK!=result)
    {
            NIM_PRINTF("Get guard_interval Fail, Wait ChannelChg Complete!\n");
//        *guard_interval=0;
        *freq_offset=0xffffffff;//050725 yuchun
    }
        else
        {
            f_de202_freq_offset(dev, de202_CurChannelInfo[priv_mem->tuner_id]->ChannelBW, de202_CurChannelInfo[priv_mem->tuner_id]->Mode, freq_offset);
       }

//       soc_printf(" offset =%d\n", *freq_offset );
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_hier_mode(struct nim_device *dev)
* Description: Read de202 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_hier_mode(struct nim_device *dev)
{
UINT8 data = 0;
UINT8 hier = 0;

// f_de202_read(dev->base_addr, REGde202_TPSRECEIVED ,&data,1);

    data=(data&TPS_HIERMODE)>>2;
    switch(data)
    {
        case 0 : hier=HIER_NONE;
        break;
        case 1 :hier=HIER_1;
        break;
        case 2 : hier=HIER_2;
        break;
        case 3 : hier=HIER_4;
        break;
        default :
                hier=HIER_NONE;
        break; /* error */
    }
        return hier;
}


/*****************************************************************************
* INT32 f_de202_get_hier_mode(struct nim_device *dev,UINT8*hier)
* Description: Read de202 hier_mode
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8*hier
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_de202_get_hier_mode(struct nim_device *dev, UINT8*hier)
{
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    *hier= de202_CurChannelInfo[priv_mem->tuner_id]->Hierarchy;
     //soc_printf(" hier a %d\n", *hier);
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_de202_priority(struct nim_device *dev,UINT8*priority)
* Description: Read de202 priority
*  Arguments:
*  Parameter1: struct nim_device *dev
  Parameter2: UINT8*priority
*
* Return Value: INT32
*****************************************************************************/

INT8 f_de202_priority(struct nim_device *dev, UINT8*priority)
{
    struct nim_de202_private * priv_mem = (struct nim_de202_private *)(dev->priv);
    *priority= de202_CurChannelInfo[priv_mem->tuner_id]->Priority;
    return SUCCESS;
}


#endif
