/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    This file contains stv0362 basic function in LLD.
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  02.04.2008      WeiChing Tu     Ver 0.1     Create file.
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
//#include <bus/spi/spi.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim.h>
#include <bus/i2c/i2c.h>

#include "nim_stv0362.h"

#if 0
  #define NIM_PRINTF  libc_printf //soc_printf
#else
  #define NIM_PRINTF(...)
  #define ALI_PRINTF(...)//  libc_printf
  //#define  NIM_DEBUG
   #define US_TICKS       (sys_ic_get_cpu_clock()*1000000 / 2000000) // (SYS_CPU_CLOCK / 2000000)
#define WAIT_1ms        (1000* US_TICKS)
#define WAIT_100ms    1
ID f_IIC_Sema_ID = OSAL_INVALID_ID;

#endif

#define NIM_MAX_UNLOCK_COUNT              2
#if 1 /*Fix manualscan will fail to lock again for stv0362 demod*/
#define NIM_SYR_UNLOCK_MAX_COUNT          1
#define NIM_TPS_PRF_LK_UNLOCK_MAX_COUNT   10
#else
#define NIM_SYR_UNLOCK_MAX_COUNT          8
#define NIM_TPS_PRF_LK_UNLOCK_MAX_COUNT   8
#endif

#define Delay_Number    1
#define WAIT_Number     1
#define SYS_WAIT_MS(x)  osal_delay((x * 1000 * WAIT_Number))

#define abs(x)  (x<0) ? (-x) : x
#define sign(y) (y>0) ? 1 : (-1)

UINT8 stv0362_autoscan_stop_flag = 0;

ID f_stv0362_CC_tracking_task_id = OSAL_INVALID_ID;
static OSAL_ID l_nim0362_sema_id;

/* Name for the tuner, the last character must be Number for index */
static char nim_stv0362_name[HLD_MAX_NAME_SIZE] = "NIM_COFDM_0";
static struct stv0362_Lock_Info   *stv0362_CurChannelInfo;

//weiching added global variable for ST Demod init parameters setting
static STTUNER_InitParams_t gSTV0362_InitParams;
#if 1 //temp added to meet ST code
static STTUNER_SearchResultData_t gSTV0362_SearchResultData;
#endif

///////////////////////////////////////////////////////////////////

 struct nim_stv0362_private  *gNim_stv0362_private;

///////////////////////////////////////////////////////////////////

//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362_drv.c [begin]
#define M6_F_GAIN_SRC_HI        0x0A
#define M6_F_GAIN_SRC_LO        0xF0 //0x28 /* 2008.02.22 fixed */
#define M6_F_TRL_NOMRATE0       0x00
#define M6_F_TRL_NOMRATE8_1     0x01
#define M6_F_TRL_NOMRATE16_9    0x41

#define M7_GAIN_SRC_HI          0x0C
#define M7_GAIN_SRC_LO          0xE4    /*For normal IF Mode*/

#define M7_E_GAIN_SRC_HI        0x08 //0x07 /* 2008.02.22 fixed */
#define M7_E_GAIN_SRC_LO        0x98 //0x6C    /* For long path IF / IQ modes */ /* 2008.02.22 fixed */

#define M7_F_TRL_NOMRATE0       0x00
#define M7_F_TRL_NOMRATE8_1     0xD6
#define M7_F_TRL_NOMRATE16_9    0x4B


#define M8_GAIN_SRC_HI          0x0C //0x0B  /* 2008.02.22 fixed */
#define M8_GAIN_SRC_LO          0xE4 //0xB8    /*For normal IF Mode*/ /* 2008.02.22 fixed */

#define M8_E_GAIN_SRC_HI        0x08
#define M8_E_GAIN_SRC_LO        0x98 //0x86    /* For long path IF / IQ modes */ /* M2008.02.22 fixed */

#define M8_F_TRL_NOMRATE0       0x01
#define M8_F_TRL_NOMRATE8_1     0xAB
#define M8_F_TRL_NOMRATE16_9    0x56

#define WAIT_N_MS_362(X)        osal_task_sleep(X);//SYS_WAIT_MS(X) //weiching mapped, exactly same timing ??

#define CPQ_LIMIT 23 //0x17

UINT16 CellsCoeffs_8MHz[6][5] = {
                {0x10EF,0xE205,0x10EF,0xCE49,0x6DA7},  /* CELL 1 COEFFS */
                {0x2151,0xc557,0x2151,0xc705,0x6f93},  /* CELL 2 COEFFS */
                {0x2503,0xc000,0x2503,0xc375,0x7194},  /* CELL 3 COEFFS */
                {0x20E9,0xca94,0x20e9,0xc153,0x7194},  /* CELL 4 COEFFS */
                {0x06EF,0xF852,0x06EF,0xC057,0x7207},  /* CELL 5 COEFFS */
                {0x0000,0x0ECC,0x0ECC,0x0000,0x3647}   /* CELL 6 COEFFS */
              } ;


UINT16 CellsCoeffs_7MHz[6][5] = {
                {0x12CA,0xDDAF,0x12CA,0xCCEB,0x6FB1},  /* CELL 1 COEFFS */
                {0x2329,0xC000,0x2329,0xC6B0,0x725F},  /* CELL 2 COEFFS */
                {0x2394,0xC000,0x2394,0xC2C7,0x7410},  /* CELL 3 COEFFS */
                {0x251C,0xC000,0x251C,0xC103,0x74D9},  /* CELL 4 COEFFS */
                {0x0804,0xF546,0x0804,0xC040,0x7544},  /* CELL 5 COEFFS */
                {0x0000,0x0CD9,0x0CD9,0x0000,0x370A}   /* CELL 6 COEFFS */
              } ;

UINT16 CellsCoeffs_6MHz[6][5] = {
                {0x12CA,0xDDAF,0x12CA,0xCCEB,0x6FB1},  /* CELL 1 COEFFS */
                {0x2329,0xC000,0x2329,0xC6B0,0x725F},  /* CELL 2 COEFFS */
                {0x2394,0xC000,0x2394,0xC2C7,0x7410},  /* CELL 3 COEFFS */
                {0x251C,0xC000,0x251C,0xC103,0x74D9},  /* CELL 4 COEFFS */
                {0x0804,0xF546,0x0804,0xC040,0x7544},  /* CELL 5 COEFFS */
                {0x0000,0x0CD9,0x0CD9,0x0000,0x370A}   /* CELL 6 COEFFS */
              } ;


/*********************************************************
--FUNCTION  ::  CheckCPAMP_362
--ACTION    ::  Get CPAMP status
--PARAMS IN ::
--PARAMS OUT::  CPAMP status
--********************************************************/
FE_362_SignalStatus_t CheckCPAMP_362(struct nim_device *dev, INT32 FFTmode)
{
    INT32  CPAMPvalue=0, CPAMPStatus, CPAMPMin;
    int wd=0;

    NIM_PRINTF("Enter CheckCPAMP_362()...\n");

    switch(FFTmode)
    {
        case 0: /* 2k mode */
            CPAMPMin = 20;
            wd = 10;
            break;
        case 1: /* 8k mode */
            CPAMPMin = 80;
            wd = 55;
            break;
        case 2: /* 4k mode */
            CPAMPMin = 40;
            wd = 30;
            break;
        default:
            ALI_PRINTF("CheckCPAMP_362 ==> FFTmode(%d) error !!!\n", FFTmode);
            return NOCPAMP_362;
    }

    CPAMPvalue = ChipGetField(dev, F0362_PPM_CPAMP_DIRECT);
    while ((CPAMPvalue < CPAMPMin) && (wd > 0))
    {
        WAIT_N_MS_362(1);
        wd--;
        CPAMPvalue = ChipGetField(dev, F0362_PPM_CPAMP_DIRECT);
    }

    if (CPAMPvalue < CPAMPMin)
    {
        CPAMPStatus = NOCPAMP_362;
     ALI_PRINTF("NOCPAMP_362,CPAMP=%d \n",CPAMPvalue);
    }
    else
    {
        CPAMPStatus = CPAMPOK_362;
     ALI_PRINTF("CPAMPOK_362,CPAMP=%d \n",CPAMPvalue);
    }

    NIM_PRINTF("CheckCPAMP_362(): CPAMPStatus = %d\n", CPAMPStatus);

    NIM_PRINTF("Exit CheckCPAMP_362() normally.\n");
    return CPAMPStatus;
}

/*****************************************************
**FUNCTION  ::  FE_362_Pow
**ACTION    ::  Compute  x^y (where n is an integer)
**PARAMS IN ::  number -> n
**PARAMS OUT::  NONE
**RETURN    ::  x^y
*****************************************************/
int FE_362_Pow(int number1, int number2)
{
    int i;
    int result=1;

    for(i=0; i<number2; i++)
        result *= number1;

    return result;
}

/*****************************************************
**FUNCTION  ::  FE_362_Pow
**ACTION    ::  Compute  2^n (where n is an integer)
**PARAMS IN ::  number -> n
**PARAMS OUT::  NONE
**RETURN    ::  2^2
*****************************************************/
int FE_362_PowOf2(int number)
{
    int i;
    int result=1;

    for(i=0; i<number; i++)
    {
        result *= 2;
    }
    return result;
}


/*****************************************************
--FUNCTION      ::  FE_362_AGC_IIR_LOCK_DETECTOR_SET
--ACTION        ::  Sets Good values for AGC IIR lock detector
--PARAMS IN     ::
--PARAMS OUT    ::  None
--***************************************************/
void  FE_362_AGC_IIR_LOCK_DETECTOR_SET(struct nim_device *dev)
{
    ALI_PRINTF("Enter FE_362_AGC_IIR_LOCK_DETECTOR_SET()...\n");

    ChipSetField(dev, F0362_LOCK_DETECT_LSB, 0x00);

    /* Lock detect 1 */
    ChipSetField(dev, F0362_LOCK_DETECT_CHOICE, 0x00);
    ChipSetField(dev, F0362_LOCK_DETECT_MSB, 0x06);
    ChipSetField(dev, F0362_AUT_AGC_TARGET_LSB, 0x04);

    /* Lock detect 2 */
    ChipSetField(dev, F0362_LOCK_DETECT_CHOICE, 0x01);
    ChipSetField(dev, F0362_LOCK_DETECT_MSB, 0x06);
    ChipSetField(dev, F0362_AUT_AGC_TARGET_LSB, 0x04);

    /* Lock detect 3 */
    ChipSetField(dev, F0362_LOCK_DETECT_CHOICE, 0x02);
    ChipSetField(dev, F0362_LOCK_DETECT_MSB, 0x01);
    ChipSetField(dev, F0362_AUT_AGC_TARGET_LSB, 0x00);

    /* Lock detect 4 */
    ChipSetField(dev, F0362_LOCK_DETECT_CHOICE, 0x03);
    ChipSetField(dev, F0362_LOCK_DETECT_MSB, 0x01);
    ChipSetField(dev, F0362_AUT_AGC_TARGET_LSB, 0x00);

    NIM_PRINTF("FE_362_AGC_IIR_LOCK_DETECTOR_SET(): AGC threshold setting done\n");
    NIM_PRINTF("Exit FE_362_AGC_IIR_LOCK_DETECTOR_SET() normally.\n");
    return;
}

/*****************************************************
--FUNCTION      ::  FE_362_IIR_FILTER_INIT
--ACTION        ::
--PARAMS IN     ::
--PARAMS OUT    ::  None
--***************************************************/
UINT32  FE_362_IIR_FILTER_INIT(struct nim_device *dev, UINT8 Bandwidth)
{
    ALI_PRINTF("Enter FE_362_IIR_FILTER_INIT() ==> ");

    ChipSetField(dev, F0362_NRST_IIR, 0);
    switch(Bandwidth)
    {
        case STTUNER_CHAN_BW_6M:
            ALI_PRINTF("STTUNER_CHAN_BW_6M\n");
            FE_362_FilterCoeffInit(dev, CellsCoeffs_6MHz);
            break;
        case STTUNER_CHAN_BW_7M:
            ALI_PRINTF("STTUNER_CHAN_BW_7M\n");
            FE_362_FilterCoeffInit(dev, CellsCoeffs_7MHz);
            break;
        case STTUNER_CHAN_BW_8M:
            ALI_PRINTF("STTUNER_CHAN_BW_8M\n");
            FE_362_FilterCoeffInit(dev, CellsCoeffs_8MHz);
            break;
        default:
            return 0;
    }

    ChipSetField(dev, F0362_NRST_IIR, 1);
    NIM_PRINTF("FE_362_IIR_FILTER_INIT(): IIR filter initialization completed\n");

    NIM_PRINTF("Exit FE_362_IIR_FILTER_INIT() normally.\n");
    return 1;
}

/*****************************************************
--FUNCTION      ::  FE_362_AGC_IIR_RESET
--ACTION        ::  AGC reset procedure
--PARAMS IN     ::
--PARAMS OUT    ::  None
--***************************************************/
void FE_362_AGC_IIR_RESET(struct nim_device *dev)
{
    UINT8 com_n;

    ALI_PRINTF("Enter FE_362_AGC_IIR_RESET()...\n");

    com_n = ChipGetField(dev, F0362_COM_N);

    ChipSetField(dev, F0362_COM_N, 0x07);

    ChipSetField(dev, F0362_COM_SOFT_RSTN, 0x00);
    ChipSetField(dev, F0362_COM_AGC_ON, 0x00);

    ChipSetField(dev, F0362_COM_SOFT_RSTN, 0x01);
    ChipSetField(dev, F0362_COM_AGC_ON, 0x01);

    ChipSetField(dev, F0362_COM_N, com_n);

    NIM_PRINTF("FE_362_AGC_IIR_RESET(): AGC reset procedure done\n");
    NIM_PRINTF("Exit FE_362_AGC_IIR_RESET() normally.\n");
    return;
}

/*********************************************************
--FUNCTION  ::  FE_362_FilterCoeffInit
--ACTION    ::  Apply filter coeffs values

--PARAMS OUT::  NONE
--********************************************************/
static void FE_362_FilterCoeffInit(struct nim_device *dev, UINT16 CellsCoeffs[][5])
{
    INT32 i, j;
    UINT32 x, y, k;

    ALI_PRINTF("Enter FE_362_FilterCoeffInit()...\n");

    k = F0362_IIR_CX_COEFF2_MSB - F0362_IIR_CX_COEFF1_MSB;
    for(i=1; i<=6; i++)
    {
        ChipSetField(dev, F0362_IIR_CELL_NB, i-1);
        x = F0362_IIR_CX_COEFF1_MSB;
        y = F0362_IIR_CX_COEFF1_LSB;

        for(j=1; j<=5; j++)
        {
            ChipSetField(dev, x, MSB(CellsCoeffs[i-1][j-1]));
            ChipSetField(dev, y, LSB(CellsCoeffs[i-1][j-1]));
            x += k;
            y += k;
        }
    }

    NIM_PRINTF("FE_362_FilterCoeffInit(): setting IIR filter co-efficient done\n");
    NIM_PRINTF("Exit FE_362_FilterCoeffInit() normally.\n");
    return;
}


/*****************************************************
--FUNCTION  ::  FE_362_WaitTuner
--ACTION    ::  Wait for tuner locked
--PARAMS IN ::  TimeOut ->  Maximum waiting time (in ms)
--PARAMS OUT::  NONE
--RETURN    ::  NONE
--***************************************************/
static void FE_362_WaitTuner(struct nim_device *dev, UINT32 TimeOut, BOOL *TunerLocked)
{
    int Time=0;

#if 0 /*Fix manualscan will fail to lock again for stv0362 demod*/
    *TunerLocked = TRUE;
#else
    *TunerLocked = FALSE;
    while(!(*TunerLocked) && ((UINT32)Time < TimeOut))
    {
        WAIT_N_MS_362(10);

        //weiching linked with ALi Tuner API code -- [begin]
        //get Tuner lock status
        ((struct nim_stv0362_private *)dev->priv)->Tuner_Control.nim_tuner_status(0,(UINT8 *)TunerLocked);
        //weiching linked with ALi Tuner API code -- [begin]

        Time++;
    }
    Time--;

    NIM_PRINTF("FE_362_WaitTuner(): TunerLocked flag = %d\n", *TunerLocked);
    NIM_PRINTF("Exit FE_362_WaitTuner() normally.\n");
#endif
    return;
}

#if 1 /*Fix manualscan will fail to lock again for stv0362 demod*/
static BOOL CheckChannelChange(UINT32 freq, UINT8 bandwidth)
{
//20080606: update the status of Lock_Val [Begin]
//    if (freq == 0)
//       return TRUE;
//  else
//20080606: update the status of Lock_Val [End]
    if (freq == stv0362_CurChannelInfo->CC_Tracking_Frequency && bandwidth == stv0362_CurChannelInfo->CC_Tracking_ChannelBW)
        return FALSE;
    return TRUE;
}
#endif

/*****************************************************
--FUNCTION  ::  FE_362_Algo
--ACTION    ::  Search for Signal, Timing, Carrier and then data at a given Frequency,
--              in a given range:

--PARAMS IN ::
--PARAMS OUT::
--RETURN    ::  Type of the founded signal (if any)

--REMARKS   ::  This function is supposed to replace FE_362_SearchRun according
--              to last findings on SYR block
--***************************************************/
FE_362_SignalStatus_t FE_362_Algo(struct nim_device *dev, FE_362_SearchParams_t *pParams)
{
    FE_362_SignalStatus_t ret_flag;
    INT16  wd, tempo;
    UINT16  try_cnt, u_var1=0, u_var2=0, u_var3=0, mode, guard;

    NIM_PRINTF("Enter FE_362_Algo()...\n");

    try_cnt = 0;
    do
    {
        ret_flag = LOCKOK_362;
        if (try_cnt == 0)
        {
        ChipSetField(dev, F0362_CORE_ACTIVE, 0);

        if(pParams->IF_IQ_Mode != STTUNER_NORMAL_IF_TUNER)
        {
            ChipSetField(dev, F0362_COM_N, 0x07);
        }

        ChipSetField(dev, F0362_GUARD, 3); /* suggested mode is 2k 1/4 */
        ChipSetField(dev, F0362_MODE, 0);

        WAIT_N_MS_362(5);

        ChipSetField(dev, F0362_CORE_ACTIVE, 1);
               WAIT_N_MS_362(350);

           }
        if (try_cnt == 0 && CheckSYR_362(dev) == NOSYMBOL_362)
        {
            ALI_PRINTF("Exit FE_362_Algo() with error, return NOSYMBOL_362;\n");
            return NOSYMBOL_362;
        }
        else
        {
            /* if chip locked on wrong mode first try, it must lock correctly second try *db*/
            mode = ChipGetField(dev, F0362_SYR_MODE);
            if (CheckCPAMP_362(dev, mode) == NOCPAMP_362)
            {
                if (try_cnt == 0)
                {
                    ret_flag = NOCPAMP_362;
                }
            }
        }
    } while ((++try_cnt < 2) && (ret_flag != LOCKOK_362));

    if( (mode != 0) && (mode != 1) && (mode != 2) )
    {
        ALI_PRINTF("Exit FE_362_Algo(), mode(%d) error !!!\n", mode);
        return SWNOK_362;
    }

    guard = ChipGetField(dev, F0362_SYR_GUARD);

    /* supress EPQ auto for SYR_GARD 1/16 or 1/32 and set channel predictor in automatic */
    switch(guard)
    {
        case 0:
        case 1:
            ChipSetField(dev, F0362_AUTO_LE_EN, 0);
            ChipSetOneRegister(dev, R0362_CHC_CTL1, 0x01);
            NIM_PRINTF("FE_362_Algo(): Supress EPQ auto for GI 1/16 and 1/32\n");
            break;

        case 2:
        case 3:
            ChipSetField(dev, F0362_AUTO_LE_EN,1);
            ChipSetOneRegister(dev, R0362_CHC_CTL1, 0x11);
            NIM_PRINTF("FE_362_Algo(): EPQ Auto Enable for GI 1/8 and 1/4\n");
            break;

        default:
            ALI_PRINTF("Exit FE_362_Algo(), guard(%d) error\n", guard);
            return SWNOK_362;
    }

    if(mode == STTUNER_MODE_2K)
    {
        ChipSetOneRegister(dev, R0362_CHP_TAPS, 0x01); /*** For 2k mode the chp_taps reg. must always set to 0x01 ***/
    }
    else
    {
        ChipSetOneRegister(dev, R0362_CHP_TAPS, 0x03); /*** For 8k mode the chp_taps reg. must always set to 0x03 ***/
    }

    u_var1 = ChipGetField(dev, F0362_LK);
    u_var2 = ChipGetField(dev, F0362_PRF);
    u_var3 = ChipGetField(dev, F0362_TPS_LOCK);
    wd = duration(mode, 125, 500, 250);
    tempo = duration(mode, 4, 16, 8);
#if 0    //for debug only
    {
        UINT8  data;
        f_stv0362_read(dev->base_addr, (UINT8) R0362_STATUS, &data, 1);
        NIM_PRINTF("FE_362_Algo(): STATUS reg = 0x%02x (u_var1=%d, u_var2=%d, u_var3=%d, tempo=%d, wd=%d)\n", data, u_var1, u_var2, u_var3, tempo, wd);
    }
#endif

    try_cnt = NIM_TPS_PRF_LK_UNLOCK_MAX_COUNT;
    while( ((!u_var1) || (!u_var2) || (!u_var3)) && ((wd >= 0) && (try_cnt > 0)) )
    {
        try_cnt--;
        f_stv0362_Core_Switch(dev); //2008.03.04 for STV0362 Re-Lock issue after RF cable unplug/plug
        /*don't need to do it*/
        //WAIT_N_MS_362(tempo);
        //wd -= tempo;
        u_var1 = ChipGetField(dev, F0362_LK);
        u_var2 = ChipGetField(dev, F0362_PRF);
        u_var3 = ChipGetField(dev, F0362_TPS_LOCK);

#if 0 //for debug only
        {
            UINT8  data;
            f_stv0362_read(dev->base_addr, (UINT8) R0362_STATUS, &data, 1);
            NIM_PRINTF("FE_362_Algo(): in loops, STATUS reg = 0x%02x (u_var1=%d, u_var2=%d, u_var3=%d, tempo=%d, wd=%d)\n", data, u_var1, u_var2, u_var3, tempo, wd);
        }
#endif
    }



    if(!u_var1)
    {
        ALI_PRINTF("Exit FE_362_Algo() with error, 0362_ULK;\n");
        return NOLOCK_362;
    }

    if(!u_var2)
    {
        ALI_PRINTF("Exit FE_362_Algo() with error, 0362_PRF_ULK;\n");
        return NOPRFOUND_362;
    }

    if (!u_var3)
    {
        ALI_PRINTF("Exit FE_362_Algo() with error, 0362_TPS_ULK;\n");
        return NOTPS_362;
    }

    if(pParams->IF_IQ_Mode != STTUNER_NORMAL_IF_TUNER)
    {
        tempo = 0;
        while (((ChipGetField(dev, F0362_COM_USEGAINTRK) != 1) && (ChipGetField(dev, F0362_COM_AGCLOCK) != 1)) && (tempo < 100)) /* to be checked 1000 or 100 */
        {
            WAIT_N_MS_362(1);
            tempo += 1;
        }
        ChipSetField(dev, F0362_COM_N, 0x17);
    }

    NIM_PRINTF("Exit FE_362_Algo() normally.\n");
    return  LOCKOK_362;
}

/*****************************************************
--FUNCTION  ::  FE_362_LookFor
--ACTION    ::  Intermediate layer before launching Search
--PARAMS IN ::
                pSearch ==> Search parameters
                pResult ==> Result of the search
--PARAMS OUT::  NONE
--RETURN    ::  Error
--***************************************************/
FE_362_Error_t  FE_362_LookFor(struct nim_device *dev, FE_362_SearchParams_t *pSearch, FE_362_SearchResult_t *pResult)
{
    FE_362_SearchParams_t pLook;
    UINT8 trials[2];
    INT8 num_trials=0, index;
    FE_362_Error_t error = FE_362_NO_ERROR;
    UINT8 flag_spec_inv;
    UINT8 flag;
    FE_362_InternalParams_t pParams;

    NIM_PRINTF("Enter FE_362_LookFor()...\n");

    pLook.Frequency = pSearch->Frequency;
    pLook.Mode = pSearch->Mode;
    pLook.Guard = pSearch->Guard;
    pLook.Inv = pSearch->Inv;
    pLook.Force = pSearch->Force;
    pLook.ChannelBW = pSearch->ChannelBW;
    pLook.EchoPos = pSearch->EchoPos;
    pLook.IF_IQ_Mode = pSearch->IF_IQ_Mode;
    pParams.Inv = pSearch->Inv;
    pLook.Hierarchy = pParams.Hierarchy = pSearch->Hierarchy;
  #if 1 //wctu added (?)
    pLook.Offset = pSearch->Offset;
  #endif

    flag_spec_inv = 0;
    flag = ((pSearch->Inv >> 1) & 1);

    /* This is made as 0 whenevr FE_362_LookFor fucntion is called.
       If in a band two different frequencies have different spectrum type then
       re-initialization of first_lock flag is good. As it will search both the cases
       in the next freqeuency even if it finds a channel (INVERTED or NORMAL) spectrum */
    pParams.first_lock = 0;

    trials[0] = NINV;
    trials[1] = INV;
    switch(flag)
    {
        case 0:
            if ((pParams.Inv == STTUNER_INVERSION_NONE) || (pParams.Inv == STTUNER_INVERSION))
                num_trials = 1;
            else
                num_trials = 2;
            break;

        case 1:
            num_trials = 2;
            if ((pParams.first_lock) && (pParams.Inv == STTUNER_INVERSION_AUTO))
            {
                num_trials = 1;
            }
            break;

        default:
            break;
    }

    if(((gSTV0362_SearchResultData.Search).first_spectrum_locked == 1) && (pSearch->Inv == STTUNER_INVERSION_UNK))  //steven:080219  open it.
    {
        pLook.Inv = (gSTV0362_SearchResultData.Result).spectrum_direction; /*whatever value is stored from first locking is given to pLook.inv*/  //steven:080219  open it.
        num_trials = 1;
    }

    pResult->SignalStatus = NOLOCK_362;
    index = 0;

    /*Fix manualscan will fail to lock again for stv0362 demod*/
    num_trials = 1;

    while ((index < num_trials) && (pResult->SignalStatus != LOCKOK_362))
    {
        if ((!pParams.first_lock) || (pParams.Inv == STTUNER_INVERSION_AUTO) || (pParams.Inv == STTUNER_INVERSION_UNK))
        {
            pParams.Sense = trials[(UINT8)index];
        }

        NIM_PRINTF("FE_362_LookFor(): => FE_362_Search()...\n");
        error = FE_362_Search(dev, &pLook, &pParams, pResult);

        if (error == FE_362_NO_ERROR)  /*If Locked is successful*/
        {
            (gSTV0362_SearchResultData.Search).first_spectrum_locked = 1; //steven:080219  open it.
            break;
        }

        index++;
    }
    NIM_PRINTF("Exit FE_362_LookFor(): Return value of FE_362_Search() = %d\n", error);
    NIM_PRINTF("Exit FE_362_LookFor() normally.\n");
    return error;
}

/*****************************************************
--FUNCTION  ::  FE_362_Search
--ACTION    ::  Search for a valid channel
--PARAMS IN ::
                pSearch ==> Search parameters
                pResult ==> Result of the search
--PARAMS OUT::  NONE
--RETURN    ::  Error (if any)
--***************************************************/
static FE_362_Error_t  FE_362_Search(struct nim_device *dev, FE_362_SearchParams_t *pSearch, FE_362_InternalParams_t *Params, FE_362_SearchResult_t *pResult)
{
    INT32 offset=0;
    INT32 offset_type=0;
    UINT8 constell, counter;
    UINT8 Rgain, gain_src[2]={0}, Rtrlctl, trl_ctl[3]={0}, /*inc_derot[2]={0},*/ tps_rcvd[3]={0};
    UINT8 syr_stat[1]={0}, agc2max[13]={0}, trl_ctl1[5]={0}, trl_ctl2[2]={0}, crl_freq[3]={0};
    INT8 step;
    BOOL TunerLocked=FALSE;
    INT32 timing_offset=0;
    UINT32 trl_nomrate=0;
    FE_362_Error_t error = FE_362_NO_ERROR;
    UINT32 CrlRegVal=0;
    INT16 tempo;
    UINT8 u_var=0;
    //UINT8 count=0;
    //UINT8 lock=0;
        struct nim_stv0362_private *dev_priv;
            dev_priv = gNim_stv0362_private;

    NIM_PRINTF("Enter FE_362_Search()...\n");
    switch(pSearch->IF_IQ_Mode)
    {
        case STTUNER_NORMAL_IF_TUNER:  /* Normal IF mode */
            ChipSetField(dev, F0362_TUNER_BB, 0);
            ChipSetField(dev, F0362_LONGPATH_IF, 0);
            ChipSetField(dev, F0362_INV_SPECTR, 1);
            ChipSetField(dev, F0362_PPM_INVSEL, 0); /* spectrum inversion hw detection  off  *db*/
            NIM_PRINTF("FE_362_Search(): Normal IF mode Hardware Spectrum inversion OFF\n");
            break;

        case STTUNER_NORMAL_LONGPATH_IF_TUNER:  /* Long IF mode */
            ChipSetField(dev, F0362_TUNER_BB, 0);
            ChipSetField(dev, F0362_LONGPATH_IF, 1);
            ChipSetField(dev, F0362_INV_SPECTR, 0);
            ChipSetField(dev, F0362_PPM_INVSEL, 0); /* spectrum inversion hw detection off */
            NIM_PRINTF("FE_362_Search(): Long Path IF mode Hardware Spectrum inversion OFF\n");
            break;

        case STTUNER_LONGPATH_IQ_TUNER:  /* IQ mode */
            ChipSetField(dev, F0362_TUNER_BB, 1);
            //ChipSetField(dev, F0362_PMC2_SWAP, 1);
            ChipSetField(dev, F0362_INV_SPECTR, 0);
            ChipSetField(dev, F0362_PPM_INVSEL, 0); /* spectrum inversion hw detection off */
            NIM_PRINTF("FE_362_Search(): Long Path IQ mode Hardware Spectrum inversion OFF \n");
            break;

        default:
            ALI_PRINTF("FE_362_Search() ==> UNKNOWN IF_IQ_Mode\n");
            NIM_PRINTF("FE_362_Search(): Error  FE_362_BAD_PARAMETER (Bad IFIQ mode selected)\n");
            return FE_362_BAD_PARAMETER;
    }

/****this part of code is not compared yet with LLA */
    if (pSearch->Inv == STTUNER_INVERSION_NONE)
    {
        if (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)
        {
            ChipSetField(dev, F0362_PMC2_SWAP, 0);
            gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)? STTUNER_INVERSION_NONE : STTUNER_INVERSION;  //steven:080219  open it.
        }
        else
        {
            ChipSetField(dev, F0362_INV_SPECTR, (pSearch->IF_IQ_Mode == 0));
            gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)? STTUNER_INVERSION_NONE : STTUNER_INVERSION;  //steven:080219  open it.
        }
    }
    else if (pSearch->Inv == STTUNER_INVERSION)
    {
        if (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)
        {
            ChipSetField(dev, F0362_PMC2_SWAP, 1);
            gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)? STTUNER_INVERSION : STTUNER_INVERSION_NONE;  //steven:080219  open it.
        }
        else
        {
            ChipSetField(dev, F0362_INV_SPECTR, !(pSearch->IF_IQ_Mode == 0));
            gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)? STTUNER_INVERSION : STTUNER_INVERSION_NONE;  //steven:080219  open it.
        }
    }
    else if ((pSearch->Inv == STTUNER_INVERSION_AUTO) || ((pSearch->Inv == STTUNER_INVERSION_UNK) && (!Params->first_lock)))
    {
        if (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)
        {
            if (Params->Sense == 1)
            {
                ChipSetField(dev, F0362_PMC2_SWAP, 1);
                gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)? STTUNER_INVERSION : STTUNER_INVERSION_NONE;  //steven:080219  open it.
            }
            else
            {
                ChipSetField(dev, F0362_PMC2_SWAP, 0);
                gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_LONGPATH_IQ_TUNER)? STTUNER_INVERSION_NONE : STTUNER_INVERSION;  //steven:080219  open it.
            }
        }
        else
        {
            if (Params->Sense == 1)
            {
                ChipSetField(dev, F0362_INV_SPECTR, !(pSearch->IF_IQ_Mode == 0));
                gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)? STTUNER_INVERSION : STTUNER_INVERSION_NONE;  //steven:080219  open it.
            }
            else
            {
                ChipSetField(dev, F0362_INV_SPECTR, (pSearch->IF_IQ_Mode == 0));
                gSTV0362_SearchResultData.ResultSpectrum = (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)? STTUNER_INVERSION_NONE :  STTUNER_INVERSION;  //steven:080219  open it.
            }
        }
        NIM_PRINTF("FE_362_Search(): Spectrum inversion auto or unknown mode with sense %d \n", Params->Sense);
    }

    if(pSearch->IF_IQ_Mode != STTUNER_NORMAL_IF_TUNER)
    {
        FE_362_AGC_IIR_LOCK_DETECTOR_SET(dev);
        if(!FE_362_IIR_FILTER_INIT(dev, pSearch->ChannelBW))
        {
            return FE_362_BAD_PARAMETER; /*This return value should be changed to a meaningful name*/
        }
        FE_362_AGC_IIR_RESET(dev);

        NIM_PRINTF("FE_362_Search(): IIR filter setting for Long path IF and IQ mode\n");
    }

/*********Code Added For Hierarchical Modulation****************/

    if (pSearch->Hierarchy == STTUNER_HIER_LOW_PRIO)
    {
        ChipSetField(dev, F0362_BDI_LPSEL, 0x01);
        NIM_PRINTF("FE_362_Search(): Hierarchical Low priority mode\n");
    }
    else
    {
        ChipSetField(dev, F0362_BDI_LPSEL, 0x00);
        NIM_PRINTF("FE_362_Search(): Hierarchical High priority mode\n");
    }

    //tuner-dependent block in ST code!!BW_6M
    {
        if (pSearch->ChannelBW == STTUNER_CHAN_BW_6M)
        {
            NIM_PRINTF("FE_362_Search(): GAIN_SRC, TRL, INC DEROT settings for Tuner in 6MHz mode\n");
            Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
            gain_src[0] = (Rgain & 0xf0) | M6_F_GAIN_SRC_HI;
            gain_src[1] = M6_F_GAIN_SRC_LO;

        #if 1 //for debug only
            ALI_PRINTF("FE_362_Search(): value to set to GAIN_SRC1 = 0x%02x\n", gain_src[0]);
            ALI_PRINTF("FE_362_Search(): value to set to GAIN_SRC2 = 0x%02x\n", gain_src[1]);
        #endif
            ChipSetRegisters(dev, R0362_GAIN_SRC1, gain_src, 2);

            Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);
            trl_ctl[0] = (Rtrlctl & 0x7f) | (M6_F_TRL_NOMRATE0 << 7);
            trl_ctl[1] = M6_F_TRL_NOMRATE8_1;
            trl_ctl[2] = M6_F_TRL_NOMRATE16_9;

        #if 1 //for debug only
            ALI_PRINTF("FE_362_Search(): value to set to TRL_CTL register = 0x%02x\n", trl_ctl[0]);
            ALI_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE1 register = 0x%02x\n", trl_ctl[1]);
            ALI_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE2 register = 0x%02x\n", trl_ctl[2]);
        #endif
            ChipSetRegisters(dev, R0362_TRL_CTL, trl_ctl, 3);
        }
    else if (pSearch->ChannelBW == STTUNER_CHAN_BW_7M)
        {
            NIM_PRINTF("FE_362_Search(): GAIN_SRC, TRL, INC DEROT settings for Tuner in 7MHz mode\n");
            //tuner-dependent block in ST code!!
            {
                Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);
                trl_ctl[0] = (Rtrlctl & 0x7f) | (M7_F_TRL_NOMRATE0 << 7);
                trl_ctl[1] = M7_F_TRL_NOMRATE8_1;
                trl_ctl[2] = M7_F_TRL_NOMRATE16_9;

            #if 1 //for debug only
                NIM_PRINTF("FE_362_Search(): value to set to TRL_CTL register = 0x%02x\n", trl_ctl[0]);
                NIM_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE1 register = 0x%02x\n", trl_ctl[1]);
                NIM_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE2 register = 0x%02x\n", trl_ctl[2]);
            #endif
            }
            ChipSetRegisters(dev, R0362_TRL_CTL, trl_ctl, 3);

            if (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)
            {
                //tuner-dependent block in ST code!!
                {
                    Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
                    gain_src[0] = (Rgain & 0xf0) | M7_GAIN_SRC_HI;
                    gain_src[1] = M7_GAIN_SRC_LO;

                #if 1 //for debug only
                    NIM_PRINTF("FE_362_Search(): value to set to GAIN_SRC1 = 0x%02x\n", gain_src[0]);
                    NIM_PRINTF("FE_362_Search(): value to set to GAIN_SRC2 = 0x%02x\n", gain_src[1]);
                #endif
                }
                ChipSetRegisters(dev, R0362_GAIN_SRC1, gain_src, 2);
            }
            else
            {
                Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
                gain_src[0] = (Rgain & 0xf0) | M7_E_GAIN_SRC_HI;
                gain_src[1] = M7_E_GAIN_SRC_LO;
                ChipSetRegisters(dev, R0362_GAIN_SRC1, gain_src, 2);
            }
        }
        else /* Channel Bandwidth = 8M */
        {
            NIM_PRINTF("FE_362_Search(): GAIN_SRC, TRL, INC DEROT settings for Tuner in 8MHz mode\n");

            if (gSTV0362_InitParams.IOCTL_Set_30MZ_REG_Flag != TRUE)
            {
                //tuner-dependent block in ST code!!
                {
                 Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);
                #if 1
                    trl_ctl[0] = (Rtrlctl & 0x7f) | (M8_F_TRL_NOMRATE0 << 7);
                    trl_ctl[1] = M8_F_TRL_NOMRATE8_1;
                    trl_ctl[2] = M8_F_TRL_NOMRATE16_9;

                #else
            trl_ctl[0] = (Rtrlctl & 0x7f) | ((dev_priv->Tuner_Control.config_data.ptST0362[0] )<< 7);
                trl_ctl[1] = dev_priv->Tuner_Control.config_data.ptST0362[1];//M8_F_TRL_NOMRATE8_1;
                trl_ctl[2] = dev_priv->Tuner_Control.config_data.ptST0362[2];//M8_F_TRL_NOMRATE16_9;
         #endif

                #if 1 //for debug only
                    ALI_PRINTF("FE_362_Search(): value to set to TRL_CTL register = 0x%02x\n", trl_ctl[0]);
                    ALI_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE1 register = 0x%02x\n", trl_ctl[1]);
                    ALI_PRINTF("FE_362_Search(): value to set to TRL_NOMRATE2 register = 0x%02x\n", trl_ctl[2]);
                #endif
                }
                ChipSetRegisters(dev, R0362_TRL_CTL, trl_ctl, 3);
            }
            if (pSearch->IF_IQ_Mode == STTUNER_NORMAL_IF_TUNER)
            {
                //tuner-dependent block in ST code!!
                {
                    Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
        #if 1
                    gain_src[0] = (Rgain & 0xf0) | M8_GAIN_SRC_HI;
                    gain_src[1] = M8_GAIN_SRC_LO;
         #else
            gain_src[0] = (Rgain & 0xf0) | (dev_priv->Tuner_Control.config_data.ptST0362[3]);
                     gain_src[1] = dev_priv->Tuner_Control.config_data.ptST0362[4];
         #endif

                #if 1 //for debug only
                    ALI_PRINTF("FE_362_Search(): value to set to GAIN_SRC1 = 0x%02x\n", gain_src[0]);
                    ALI_PRINTF("FE_362_Search(): value to set to GAIN_SRC2 = 0x%02x\n", gain_src[1]);
                #endif
                }
                ChipSetRegisters(dev, R0362_GAIN_SRC1, gain_src, 2);
            }
            else
            {
             Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
            #if 1
                gain_src[0] = (Rgain & 0xf0) | M8_E_GAIN_SRC_HI;
                gain_src[1] = M8_E_GAIN_SRC_LO;
            #else
        gain_src[0] = (Rgain & 0xf0) | (dev_priv->Tuner_Control.config_data.ptST0362[3]);
                gain_src[1] = dev_priv->Tuner_Control.config_data.ptST0362[4];
        #endif


            #if 1 //for debug only
                NIM_PRINTF("FE_362_Search(): value to set to GAIN_SRC1 = 0x%02x\n", gain_src[0]);
                NIM_PRINTF("FE_362_Search(): value to set to GAIN_SRC2 = 0x%02x\n", gain_src[1]);
            #endif
                ChipSetRegisters(dev, R0362_GAIN_SRC1, gain_src, 2);
            }
        }
    }
#if 0 //for debug only
    Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC1);
    NIM_PRINTF("FE_362_Search(): after setting, GAIN_SRC1 register = 0x%02x\n", Rgain);
    Rgain = ChipGetOneRegister(dev, R0362_GAIN_SRC2);
    NIM_PRINTF("FE_362_Search(): after setting, GAIN_SRC2 register = 0x%02x\n", Rgain);

    Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);
    NIM_PRINTF("FE_362_Search(): after setting, TRL_CTL register = 0x%02x\n", Rtrlctl);
    Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL+1);
    NIM_PRINTF("FE_362_Search(): after setting, TRL_NOMRATE1 register = 0x%02x\n", Rtrlctl);
    Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL+2);
    NIM_PRINTF("FE_362_Search(): after setting, TRL_NOMRATE2 register = 0x%02x\n", Rtrlctl);
  #endif

    pSearch->EchoPos = pSearch->EchoPos;
    ChipSetField(dev, F0362_LONG_ECHO, pSearch->EchoPos);

    NIM_PRINTF("FE_362_Search(): frequency = %d, bandwidth = %d, is going to set in Tuner...\n", pSearch->Frequency, pSearch->ChannelBW);

    //weiching linked with ALi Tuner API code -- [begin]
    {
        INT32  tuner_error; //ALi's error code, not ST error code
        UINT8  u8_CtrlData[8];
//if( stv0362_CurChannelInfo->CC_Tracking_staus==0)
// {
 //feed frequency into Tuner
        //20080512: modify DTT75411's control byte [Begin]
        ST_d0362_repeateraccess(dev, TRUE);
       tuner_error = ((struct nim_stv0362_private *)dev->priv)->Tuner_Control.nim_tuner_control(0,pSearch->Frequency, pSearch->ChannelBW, FAST_TIMECST_AGC, u8_CtrlData, _1ST_I2C_CMD);
    ST_d0362_repeateraccess(dev, FALSE);

        if(tuner_error != SUCCESS)
         {
             error = FE_362_BAD_PARAMETER;
        ALI_PRINTF("Set  tuner fail \n");
        }
   // }
    //tuner-dependent block in ST code!!
  #if 0//ST code specially for TUNER_DTT7592A
        TunerLocked = TRUE; /*For DTT7592 Tunerlocked flag is made true by default. Actually tuner locked isn't checked */
  #else //weiching note: FE_362_WaitTuner() not work with DTT7592 Tuner chip !!
      ST_d0362_repeateraccess(dev, TRUE);
        FE_362_WaitTuner(dev, 100, &TunerLocked); /* Is tuner Locked ? (wait 100 ms max) */
    ST_d0362_repeateraccess(dev, FALSE);
  #endif
    }
    //weiching linked with ALi Tuner API code -- [end]

    if((error != FE_362_NO_ERROR) || (TunerLocked == FALSE))
    {
        ALI_PRINTF("FE_362_Search(): failed, Tuner not locked\n");
        return(FE_362_SEARCH_FAILED);
    }

    NIM_PRINTF("FE_362_Search(): => FE_362_Algo()...\n");

    if((pResult->SignalStatus = FE_362_Algo(dev, pSearch)) == LOCKOK_362)
    {
        NIM_PRINTF("FE_362_Search(): FE_362_Algo() get LOCKED!\n");
        pResult->Locked = TRUE;

        pResult->SignalStatus = LOCKOK_362;

        /* update results */

        tps_rcvd[0] = ChipGetOneRegister(dev, R0362_TPS_RCVD2);
        tps_rcvd[1] = ChipGetOneRegister(dev, R0362_TPS_RCVD3);
        tps_rcvd[2] = ChipGetOneRegister(dev, R0362_TPS_RCVD4);

        ChipGetRegisters(dev, R0362_SYR_STAT, 1, syr_stat);

        pResult->Mode = (syr_stat[0] & 0x04) >> 2; /*SYR_SAT@0x92[2]--Previously equivalent to get field image*/
        pResult->Guard = syr_stat[0] & 0x03; /*SYR_SAT@0x92[:0]--Previously equivalent to get field image*/

        constell = tps_rcvd[0] & 0x03; /*TPS_RCV2@0xA8[1:0]*/

        if (constell == 0)
            pResult->Modulation = STTUNER_MOD_QPSK;
        else if (constell == 1)
            pResult->Modulation = STTUNER_MOD_16QAM;
        else
            pResult->Modulation = STTUNER_MOD_64QAM;

        /***Code replced and changed  for HM**/
        pResult->hier = pSearch->Hierarchy;
        pResult->Hierarchy_Alpha = (tps_rcvd[0] & 0x70) >> 4; /*TPS_RCV2@0xA8[6:4]*/
        /****/
        pResult->Hprate = tps_rcvd[1] & 0x07; /*TPS_RCV2@0xA9[2:0]*/
        pResult->Lprate = (tps_rcvd[1] & 0x70) >> 4; /*TPS_RCV2@0xA9[6:4]*/
        /****/
        constell = ChipGetField(dev, F0362_PR);
        if (constell == 5)
            constell = 4;
        pResult->pr = (FE_362_Rate_t) constell;

        pResult->Sense = ChipGetField(dev, F0362_INV_SPECTR);

        Params->first_lock = 1;

        ChipGetRegisters(dev, R0362_AGC2MAX, 13, agc2max);
        pResult->Agc_val = (agc2max[9] << 16) + ((agc2max[10] & 0x0f) << 24) + agc2max[11] + ((agc2max[12] & 0x0f) << 8);

        /* Carrier offset calculation */
        ChipSetField(dev, F0362_FREEZE, 1);
        ChipGetRegisters(dev, R0362_CRL_FREQ1, 3, crl_freq);
        ChipSetField(dev, F0362_FREEZE, 0);

        CrlRegVal = (crl_freq[2] << 16);
        CrlRegVal += (crl_freq[1] << 8);
        CrlRegVal += crl_freq[0];

        if(CrlRegVal > 8388607)
        {
            offset = CrlRegVal - 16777216; /* 2's complement negative value */
        }
        else
        {
            offset = CrlRegVal;
        }

        offset = offset*2/16384;

        if(pResult->Mode == STTUNER_MODE_2K)
        {
            offset = (offset*4464)/1000; /*** 1 FFT BIN=4.464khz***/
        }
        else if(pResult->Mode == STTUNER_MODE_4K)
        {
            offset = (offset*223)/100; /*** 1 FFT BIN=2.23khz***/
        }
        else  if(pResult->Mode == STTUNER_MODE_8K)
        {
            offset = (offset*111)/100; /*** 1 FFT BIN=1.1khz***/
        }

        /* pSearch->Frequency += offset; */
        pResult->Frequency = pSearch->Frequency;

        if (gSTV0362_SearchResultData.ResultSpectrum == STTUNER_INVERSION)  //steven:080219  open it.
        {
            offset = offset;
        }
        else
        {
            offset = offset * -1;
        }

        if (offset > 0)
        {
            offset_type = STTUNER_OFFSET_POSITIVE;
        }
        else if (offset < 0)
        {
            offset_type = STTUNER_OFFSET_NEGATIVE;
        }
        else
        {
            offset_type = STTUNER_OFFSET_NONE;
        }

        pResult->offset = offset;
        pResult->offset_type = offset_type;

        pResult->spectrum_direction = gSTV0362_SearchResultData.ResultSpectrum; //steven:080219  open it.

        pResult->Echo_pos = ChipGetField(dev, F0362_LONG_ECHO);

        tempo = 10;  /* exit even if timing_offset stays null *db* */
        while ((timing_offset == 0) && (tempo > 0))
        {
            WAIT_N_MS_362(10);  /* was 20 ms */
            /* fine tuning of timing offset if required */
            ChipGetRegisters(dev, R0362_TRL_CTL, 5, trl_ctl1);
            timing_offset = trl_ctl1[3] + 256*trl_ctl1[4]; /*TRL_TIME2@0x9E = TRL_OFFSET[15:8],TRL_TIME1@9D=TRL_OFFSET[7:0]*/

            if (timing_offset >= 32768)
            {
                timing_offset -= 65536;
            }
            /* timing_offset = (timing_offset+10)/20; */ /* rounding */
            trl_nomrate = (512*trl_ctl1[2] + trl_ctl1[1]*2 + ((trl_ctl1[0] & 0x80) >> 7)); /*TRL_NOMRATE @0x9C[16:9]+TRL_NOMRATE @0x9B[8:1]+TRL_CTL 0x@9A,TRL_NOMRATE[0]*/
            /*Calculate the timing offset in ppm.. (TRL_OFFSET*10e6)/(32*128*TRL_NOMRATE/2)*/
            timing_offset = ((signed)(1000000/trl_nomrate) * timing_offset)/2048;
            tempo--;
        }

        if (timing_offset <= 0)
        {
            timing_offset = (timing_offset - 11)/22;
            step = -1;
        }
        else
        {
            timing_offset = (timing_offset + 11)/22;
            step = 1;
        }

        ALI_PRINTF("FE_362_Search(): Timing offset value %d\n", timing_offset);

        UINT8 tmp_max_cnt = (timing_offset)? ((UINT8) timing_offset):((UINT8) (-1)*(timing_offset));
        for (counter=0; counter < tmp_max_cnt; counter++)
        {
            NIM_PRINTF("FE_362_Search(): debug---loop A--!! counter = %d, abs(timing_offset) = %d\n", counter, abs(timing_offset));

            trl_nomrate += step;
            Rtrlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);
            trl_ctl2[0] = ((trl_nomrate % 2) << 7) | (Rtrlctl & 0x7f) ;
            trl_ctl2[1] = trl_nomrate/2;
            ChipSetRegisters(dev, R0362_TRL_CTL, trl_ctl2, 2);
            WAIT_N_MS_362(1);
        }

        WAIT_N_MS_362(5);
        /* unlocks could happen in case of trl centring big step, then a core off/on restarts demod */
        u_var = ChipGetField(dev, F0362_LK);

        if(!u_var)
        {
             NIM_PRINTF("FE_362_Search(): Core ON /OFF after TRL fine centering\n");
             ChipSetField(dev, F0362_CORE_ACTIVE, 0);
            WAIT_N_MS_362(5);
             ChipSetField(dev, F0362_CORE_ACTIVE, 1);
        }
    }
    else
    {
        NIM_PRINTF("FE_362_Search(): failed .... Demod not locked\n");

        pResult->Locked = FALSE;
        error = FE_362_SEARCH_FAILED;
    }

    NIM_PRINTF("Exit FE_362_Search() normally, return code = %d.\n", error);
    return error;
}

/*----------------------------------------------------
FUNCTION    :  FE_362_GetNoiseEstimator
ACTION :
PARAMS IN :
PARAMS OUT :
RETURN :
------------------------------------------------------*/
void FE_362_GetNoiseEstimator(struct nim_device *dev, UINT32 *pNoise, UINT32 *pBer)
{
    short unsigned int  source,prf;
    UINT32 quoz, error;
    UINT32 snr=0;

    NIM_PRINTF("Enter FE_362_GetNoiseEstimator()...\n");

    error = ChipGetField(dev, F0362_ERROR_COUNT1_LO);
    error += ChipGetField(dev, F0362_ERROR_COUNT1_HI) * 256;
    error = error * 100;

    quoz = 1;
    if (!ChipGetField(dev, F0362_ERRMODE1))
    {
        source = ChipGetField(dev, F0362_ERR_SOURCE1);
        quoz = FE_362_PowOf2(12 + 2*(ChipGetField(dev, F0362_NUM_EVENT1)));

        switch(source)
        {
            case 0:
                quoz = quoz * 8;
                prf = ChipGetField(dev, F0362_PR);

                switch(prf)
                {
                    case 0:
                        error = (UINT32)(error*(1/2)); /*gbgbcast*/
                        break;
                    case 1:
                        error = (UINT32)(error*(2/3)); /*gbgbcast*/
                        break;
                    case 2:
                        error = (UINT32)(error*(3/4)); /*gbgbcast*/
                        break;
                    case 3:
                        error = (UINT32)(error*(5/6)); /*gbgbcast*/
                        break;
                    case 4:
                        error = (UINT32)(error*(6/7)); /*gbgbcast*/
                        break;
                    case 5:
                        error = (UINT32)(error*(7/8)); /*gbgbcast*/
                        break;
                    default:
                        error = error;
                        break;
                }
                break;

            case 1:
                quoz = quoz * 8;
                break;

            case 2:
                break;

            case 3:
                error = error * 188;
                break;

            default:
                error = error;
                break;
        }
    }

    snr = ChipGetField(dev, F0362_CHC_SNR);

    /* *pNoise = (snr*10) >> 3; */
    *pNoise = ((snr/8)*100)/32;
    *pBer = error*(100000000/quoz); /**** (error/quoz) gives my actual BER . We multiply it with
                                     10*e+8 now and 10*e+2 before for scaling purpose so that we can send a integer
                                     value to application user . So total scaling factor is 10*e+10*****/

        //ALI_PRINTF(" C/N= %d , BER=%d \n", *pNoise, *pBer);

    NIM_PRINTF("Exit FE_362_GetNoiseEstimator() normally.\n");
    return;
}

#if 1 /*Fix manualscan will fail to lock again for stv0362 demod*/
/*********************************************************
--FUNCTION  ::  FE_362_Tracking
--ACTION    ::  Once the chip is locked tracks its state
--PARAMS IN ::  period/duration of tracking in seconds

--PARAMS OUT::  NONE
--RETURN    ::  NONE
--********************************************************/
void FE_362_Tracking(struct nim_device *dev)
{
    static UINT8     Lock_Count = 0;
    static UINT32     TK_Count = 0;

    osal_semaphore_capture(l_nim0362_sema_id, TMO_FEVR);

    if (ChipGetField(dev, F0362_TPS_LOCK) && ChipGetField(dev, F0362_LK))
    {
        Lock_Count = 0;
    if (stv0362_CurChannelInfo->Lock_Val == 0)
        {
            ALI_PRINTF("FE_362_Tracking ==> get Lock !!\n");
            stv0362_CurChannelInfo->Lock_Val = 1;
        }
    }
    else
    {
        if (stv0362_CurChannelInfo->Lock_Val == 1)
        {
            if (++Lock_Count == NIM_MAX_UNLOCK_COUNT)
            {
                ALI_PRINTF("FE_362_Tracking ==> get unLock !!!\n");
                stv0362_CurChannelInfo->Lock_Val = 0;
                Lock_Count = 0;
               }
        }
    }
    if (stv0362_CurChannelInfo->Lock_Val == 1)
        TK_Count = 0;
    else
    {
        if (++TK_Count == 2)
        {
            if (!stv0362_CurChannelInfo->CC_Tracking_flag)
                f_stv0362_CC_Tracking(dev);
             TK_Count = 0;
        }
    }
    osal_semaphore_release(l_nim0362_sema_id);
    NIM_PRINTF("Exit FE_362_Tracking() normally.\n");
    return;
}

/*****************************************************************************
* INT32 f_stv0362_enable(struct nim_device *dev)
* Description: stv0362 enable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_enable(struct nim_device *dev)
{
    UINT8  TOPCTRL_data, tmp_data;
    ALI_PRINTF("Enter f_stv0362_enable()...\n");
    while (1)
    {
        //Enable TS Output
        TOPCTRL_data = (STV0362_DefVal[f_stv0362_ArrayIdx_of_Reg(R0362_TOPCTRL)] & (~(f_stv0362_Mask_of_RegField(F0362_TS_DIS))));
     ALI_PRINTF("TOPCTRL_data=%x\n",TOPCTRL_data);
        tmp_data = TOPCTRL_data;
        f_stv0362_write(dev->base_addr, (UINT8)R0362_TOPCTRL, &tmp_data, 1);


        f_stv0362_read(dev->base_addr, (UINT8)R0362_TOPCTRL, &tmp_data, 1);
    ALI_PRINTF("stv0362_enable=>tmp_data=%x\n",tmp_data);
        if(tmp_data == TOPCTRL_data)
            break;
    }
    NIM_PRINTF("Exit f_stv0362_enable() normally.\n");
    return SUCCESS;
}
#endif

/*********************************************************
--FUNCTION  ::  duration
--ACTION    ::  wait for a duration regarding mode
--PARAMS IN ::  mode, tempo1, tempo2, tempo3
--PARAMS OUT::  none
--********************************************************/
static INT16 duration(INT32 mode, int tempo1, int tempo2, int tempo3)
{
    int local_tempo=0;

    switch(mode)
    {
        case 0:
            local_tempo = tempo1;
            break;
        case 1:
            local_tempo = tempo2;
            break;
        case 2:
            local_tempo = tempo3;
            break;
        default:
            break;
    }

    return local_tempo;
}

/*********************************************************
--FUNCTION  ::  CheckSYR_362
--ACTION    ::  Check for SYMBOL recovery loop lock status
--PARAMS IN ::
--PARAMS OUT::  SYR check status
--********************************************************/
static FE_362_SignalStatus_t CheckSYR_362(struct nim_device *dev)
{
    UINT8 cnt=NIM_SYR_UNLOCK_MAX_COUNT;
    UINT8 SYR_var;
    INT32 SYRStatus;

    NIM_PRINTF("Enter CheckSYR_362()...\n");
    SYR_var = ChipGetField(dev, F0362_SYR_LOCK);
    while(!SYR_var && (cnt > 0))
    {
        NIM_PRINTF("CheckSYR_362(): in loops [%d], SYR_LOCK not locked => f_stv0362_Core_Switch()...\n", cnt);
        f_stv0362_Core_Switch(dev); //2008.03.04 for STV0362 Re-Lock issue after RF cable unplug/plug
        SYR_var = ChipGetField(dev, F0362_SYR_LOCK);
        cnt--;
    }

    if(!SYR_var)
    {
        SYRStatus = NOSYMBOL_362;
    }
    else
    {
        SYRStatus = SYMBOLOK_362;
    }

    NIM_PRINTF("Exit CheckSYR_362() normally.\n");
    return SYRStatus;
}
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362_drv.c [end]


static INT32 f_stv0362_read(UINT8 dev_addr, UINT8 reg_addr, UINT8 *data, UINT8 len)
{
    INT32 err;
#ifdef NIM_DEBUG
 return 0;
return SUCCESS;
#endif
//20080508: Speed up register reading of demodulator [Begin]
#if 0
    data[0] = reg_addr;
    err = i2c_write_read(0, dev_addr, data, 1, len);
    if(err != SUCCESS)
          ALI_PRINTF("read==> %d\n",err);
#else
//20080508: Speed up register reading of demodulator [End]
osal_semaphore_capture(f_IIC_Sema_ID,TMO_FEVR);
    data[0] = reg_addr;
    err = i2c_write(0, dev_addr, &reg_addr, 1);
   if(err != SUCCESS)
    {
           ALI_PRINTF("read_R_w==> %d\n",err);
           ;
    }

    err = i2c_read(0 ,dev_addr, data, len);
    if(err != SUCCESS)
    {
           ALI_PRINTF("read_R_r==> %d\n",err);
           ;
    }

      osal_task_sleep(10);

osal_semaphore_release(f_IIC_Sema_ID);
#endif //20080508: Speed up register reading of demodulator
    return err;
}

static INT32 f_stv0362_write(UINT8 dev_addr, UINT8 reg_addr, UINT8 *data, UINT8 len)
{
    UINT8  i, buffer[8];
    INT32 err;

#ifdef NIM_DEBUG
 return 0;
return SUCCESS;
#endif
osal_semaphore_capture(f_IIC_Sema_ID,TMO_FEVR);
    //if (len > 7)
    //{
        //return ERR_FAILUE;
    //}
    buffer[0] = reg_addr;
    for (i=0; i<len; i++)
    {
        buffer[i+1] = data[i];
    }

    err=i2c_write(0, dev_addr, buffer, len+1);
      if(err != SUCCESS)
           {
               ALI_PRINTF("W==> %d\n",err);
           ;
       }
  // osal_task_sleep(10);
osal_semaphore_release(f_IIC_Sema_ID);
    return err;
}

/*****************************************************************************
* INT32  f_stv0362_attach()
* Description: stv0362 initialization
*
* Arguments:
*
* Return Value: INT32
*****************************************************************************/
INT32 f_stv0362_attach(struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner)
{
    struct nim_device *dev;
    void  *priv_mem;
    UINT32 tuner_id;

    ALI_PRINTF("Enter f_stv0362_attach()...\n");

    if ((ptrCOFDM_Tuner == NULL))
    {
        NIM_PRINTF("f_stv0362_attach(): Tuner Configuration API structure is NULL!\n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(nim_stv0362_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("f_stv0362_attach(): Error, Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (void *)MALLOC(sizeof(struct nim_stv0362_private));
    if (priv_mem == NULL)
    {
        dev_free(dev);
        NIM_PRINTF("f_stv0362_attach(): Alloc nim device prive memory error!\n");
        return ERR_NO_MEM;
    }

    MEMCPY(priv_mem, ptrCOFDM_Tuner, sizeof(struct nim_stv0362_private));
    dev->priv = priv_mem;
gNim_stv0362_private=priv_mem;

  #if 0 //DVB-T disable diseqc
    //diseqc state init
    dev->diseqc_info.diseqc_type = 0;
    dev->diseqc_info.diseqc_port = 0;
    dev->diseqc_info.diseqc_k22 = 0;
    dev->diseqc_info.diseqc_polar = 0;

    dev->diseqc_typex = 0;
    dev->diseqc_portx = 0;
  #endif
 f_IIC_Sema_ID=osal_semaphore_create(1);

    /* Function point init */
    dev->base_addr = SYS_COFDM_ST0362_CHIP_ADRRESS;
    dev->init = f_stv0362_attach;
    dev->open = f_stv0362_open;
    dev->stop = f_stv0362_close;
    dev->do_ioctl = f_stv0362_ioctl;
    dev->do_ioctl_ext = f_stv0362_ioctl_ext;
    //dev->set_polar = NULL;
    //dev->set_12v = NULL;
    //dev->DiSEqC_operate = NULL;  //DVB-T disable diseqc
//    dev->get_lock_status = f_stv0362_get_lock_status; /*don't need to create task for tracking lock status*/
    dev->get_lock = f_stv0362_get_lock_status;//f_stv0362_get_lock;
    dev->get_freq = f_stv0362_get_freq;
    //dev->get_sym = NULL;
    dev->get_fec = f_stv0362_get_code_rate;
    dev->get_snr = f_stv0362_get_SNR;
    //dev->signal_tracking = NULL;
    //dev->signal_CC_Tracking = f_stv0362_CC_Tracking;
    //dev->set_i2c_bypass = f_stv0362_i2c_bypass;
  #if 0 //weiching newly added (ported from ST stv0362 code)
    dev->get_RFLevel = f_stv0362_get_RFLevel;
    dev->demod_StandByMode = f_stv0362_StandByMode;
  #endif

#if(SYS_PROJECT_FE == PROJECT_FE_DVBT)
    dev->disable = f_stv0362_disable;
    dev->channel_change = f_stv0362_channel_change;
    dev->channel_search = (void *)f_stv0362_channel_search;
    dev->get_agc = (void *)f_stv0362_get_AGC;
    dev->get_ber = f_stv0362_get_BER;
    dev->get_hier = f_stv0362_get_hier_mode;
    dev->get_priority = f_stv0362_priority;

    /*fft function require - by Joey*/
    //dev->get_fft_result = NULL;

    //added for DVB-T additional elements
    dev->get_guard_interval = f_stv0362_get_GI;
    dev->get_fftmode = f_stv0362_get_fftmode;
    dev->get_modulation = f_stv0362_get_modulation;
    dev->get_spectrum_inv = f_stv0362_get_specinv;
    //dev->get_bandwidth = f_stv0362_get_bandwidth=0;
    dev->get_freq_offset = f_stv0362_get_freq_offset;

#endif


  #if 1  //weiching copied and tuned from OLD ST code: tuner.c (not included in new ST code)
    //init config parameters for stv0362 demod
    MEMSET(&gSTV0362_InitParams, 0, sizeof(STTUNER_InitParams_t));
    gSTV0362_InitParams.ExternalClock = 4000000;
    gSTV0362_InitParams.TSOutputMode = STTUNER_TS_MODE_PARALLEL;
    gSTV0362_InitParams.SerialClockSource = STTUNER_SCLK_DEFAULT;
    gSTV0362_InitParams.SerialDataMode = STTUNER_SDAT_DEFAULT;
    gSTV0362_InitParams.FECMode = STTUNER_FEC_MODE_DVB;
    gSTV0362_InitParams.ClockPolarity = STTUNER_DATA_CLOCK_POLARITY_FALLING;
    gSTV0362_InitParams.StandBy_Flag = STTUNER_NORMAL_POWER_MODE;
    gSTV0362_InitParams.IOCTL_Set_30MZ_REG_Flag = FALSE; //for setting some STV0362 registers for 30 MHZ crystal
  #endif

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("f_stv0362_attach(): Error, Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if (((struct nim_stv0362_private *)dev->priv)->Tuner_Control.nim_tuner_init != NULL)
    {
        if (((struct nim_stv0362_private *)dev->priv)->Tuner_Control.nim_tuner_init(&tuner_id,&(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
        //if (((struct nim_stv0362_private *)dev->priv)->Tuner_Control.nim_Tuner_Init(&(ptrCOFDM_Tuner->tuner_config)) != SUCCESS)
        {
            NIM_PRINTF("f_stv0362_attach(): Error, Init Tuner Failure!\n");
            return ERR_NO_DEV;
        }
    }

    NIM_PRINTF("Exit f_stv0362_attach() normally.\n");
    return SUCCESS;
}


/*****************************************************************************
* UINT32 f_stv0362_ArrayIdx_of_Reg(UINT16 reg_addr)
* Description: For mapping Register Address to its array index, because the Register array
*              is ordered following all registers initialization order!
*
* Arguments:
*  Parameter1: UINT16 reg_addr, stv0362 register address (here, use UINT16 to meet ST register definition!)
*
* Return Value: UINT32, index of stv0362 register array
*****************************************************************************/
static UINT32 f_stv0362_ArrayIdx_of_Reg(UINT16 reg_addr)
{
    UINT32 i;//, idx=0;

    for (i=0; i<STV0362_NBREGS; i++)
    {
        if(STV0362_Address[i] == reg_addr)
        {
            return i;
        }
    }
    return 0;
}

/*****************************************************************************
* UINT8 f_stv0362_Mask_of_RegField(UINT32 reg_field)
* Description: For mapping Register Field to 8-bit mask value, because ST Register Field definitions
*              are 32-bit long and prefixed with its Register Address.
*
* Arguments:
*  Parameter1: UINT32 reg_field, stv0362 register field (here, use UINT32 to meet ST register field definition!)
*
* Return Value: UINT8, 8-bit mask value
*****************************************************************************/
static UINT8 f_stv0362_Mask_of_RegField(UINT32 reg_field)
{
    UINT8 field_mask;

    field_mask = (UINT8) (reg_field & 0x000000ff);

    return field_mask;
}

/*****************************************************************************
* UINT16 f_stv0362_RegAddr_of_RegField(UINT32 reg_field)
* Description: For mapping Register Field to its Register Address
*
* Arguments:
*  Parameter1: UINT32 reg_field, stv0362 register field (here, use UINT32 to meet ST register field definition!)
*
* Return Value: UINT16, Register Address (here, use UINT16 to meet ST register definition!)
*****************************************************************************/
static UINT16 f_stv0362_RegAddr_of_RegField(UINT32 reg_field)
{
    UINT16 reg_addr;

    reg_addr = (UINT16) ((reg_field & 0xffff0000) >> 16);

    return reg_addr;
}

/*****************************************************************************
* UINT8 f_stv0362_BitShiftCount_of_RegFieldMask(UINT8 reg_field_mask)
* Description: For counting bits needed to be right-shifted for a Register Field Mask
*
* Arguments:
*  Parameter1: UINT8 reg_field_mask, 8-bit mask of a register field
*
* Return Value: UINT8, bits count needed to be right-shifted for the Register Field value
*****************************************************************************/
static UINT8 f_stv0362_BitShiftCount_of_RegFieldMask(UINT8 reg_field_mask)
{
    UINT8 right_shift_cnt=0, i;

    for(i=0; i<8; i++) //8-bit register
    {
        if((reg_field_mask & 0x01) == 0)
        {
            reg_field_mask >>= 1;
            right_shift_cnt++;
        } else
            break;
    }
    return right_shift_cnt;
}

/*****************************************************************************
* void ChipSetOneRegister(struct nim_device *dev, UINT16 reg_addr, UINT8 value)
* Description: weiching implement this function to meet ST STV0362 code Register Write usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT16 reg_addr, stv0362 register address (here, use UINT16 to meet ST register definition!)
*  Parameter3: UINT8 value, 8-bit value to set to register
*
* Return Value: None
*****************************************************************************/
static void ChipSetOneRegister(struct nim_device *dev, UINT16 reg_addr, UINT8 value)
{
    UINT8 data = value;
    f_stv0362_write(dev->base_addr, (UINT8) reg_addr, &data, 1);

    return;
}

/*****************************************************************************
* UINT8 ChipGetOneRegister(struct nim_device *dev, UINT16 reg_addr)
* Description: weiching implement this function to meet ST STV0362 code Register Read usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT16 reg_addr, stv0362 register address (here, use UINT16 to meet ST register definition!)
*
* Return Value: UINT8, 8-bit register value
*****************************************************************************/
static UINT8 ChipGetOneRegister(struct nim_device *dev, UINT16 reg_addr)
{
    UINT8 value;
    f_stv0362_read(dev->base_addr, (UINT8) reg_addr, &value, 1);

    return value;
}

/*****************************************************************************
* void ChipSetField(struct nim_device *dev, UINT32 reg_field, UINT8 value)
* Description: weiching implement this function to meet ST STV0362 code Register Field Write usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT32 reg_field, stv0362 register field (here, use UINT32 to meet ST register field definition!)
*  Parameter3: UINT8 value, 8-bit value to set to register
*
* Return Value: None
*****************************************************************************/
static void ChipSetField(struct nim_device *dev, UINT32 reg_field, UINT8 value)
{
    UINT8 reg_data, field_mask, shift_bits_cnt;

    field_mask = f_stv0362_Mask_of_RegField(reg_field);
    shift_bits_cnt = f_stv0362_BitShiftCount_of_RegFieldMask(field_mask);

    //read original register value first
    f_stv0362_read(dev->base_addr, (UINT8) f_stv0362_RegAddr_of_RegField(reg_field), &reg_data, 1);

    //set Register Field value
    //2008.03.10 fixed ChipSetField() bug which cause register value not valid [begin]
    reg_data &= ~(field_mask);
    reg_data |= (value << shift_bits_cnt);
    //2008.03.10 fixed ChipSetField() bug which cause register value not valid [end]

    //write value to register
    f_stv0362_write(dev->base_addr, (UINT8) f_stv0362_RegAddr_of_RegField(reg_field), &reg_data, 1);

    return;
}

/*****************************************************************************
* UINT8 ChipGetField(struct nim_device *dev, UINT32 reg_field)
* Description: weiching implement this function to meet ST STV0362 code Register Field Read usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT32 reg_field, stv0362 register field (here, use UINT32 to meet ST register field definition!)
*
* Return Value: UINT8, 8-bit register field value
*****************************************************************************/
static UINT8 ChipGetField(struct nim_device *dev, UINT32 reg_field)
{
    UINT8 reg_data, field_value, field_mask, shift_bits_cnt;

    field_mask = f_stv0362_Mask_of_RegField(reg_field);
    shift_bits_cnt = f_stv0362_BitShiftCount_of_RegFieldMask(field_mask);

    //read register value
    f_stv0362_read(dev->base_addr, (UINT8) f_stv0362_RegAddr_of_RegField(reg_field), &reg_data, 1);

    //extract Register Field value
    field_value = ((reg_data & field_mask) >> shift_bits_cnt);

    return field_value;
}

/*****************************************************************************
* void ChipSetRegisters(struct nim_device *dev, UINT16 first_reg_addr, UINT8 *RegsVal, int Number)
* Description: weiching implement this function to meet ST STV0362 code Multiple Register Write usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT16 first_reg_addr, first address of contiguous stv0362 registers (here, use UINT16 to meet ST register definition!)
*  Parameter3: UINT8 *RegsVal, array of 8-bit values to set to stv0362 registers
*  Parameter4: int Number, number of registers to write
*
* Return Value: None
*****************************************************************************/
static void ChipSetRegisters(struct nim_device *dev, UINT16 first_reg_addr, UINT8 *RegsVal, int Number)
{
    f_stv0362_write(dev->base_addr, (UINT8) first_reg_addr, RegsVal, Number);
    return;
}

/*****************************************************************************
* void ChipGetRegisters(struct nim_device *dev, UINT16 first_reg_addr, int Number, UINT8 *RegsVal)
* Description: weiching implement this function to meet ST STV0362 code Multiple Register Read usage
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*  Parameter2: UINT16 first_reg_addr, first address of contiguous stv0362 registers (here, use UINT16 to meet ST register definition!)
*  Parameter3: int Number, number of registers to read
*  Parameter4: UINT8 *RegsVal, buffer array of 8-bit values to store stv0362 registers which are read
*
* Return Value: UINT8, 8-bit register value
*****************************************************************************/
static void ChipGetRegisters(struct nim_device *dev, UINT16 first_reg_addr, int Number, UINT8 *RegsVal)
{
    f_stv0362_read(dev->base_addr, (UINT8) first_reg_addr, RegsVal, Number);
    return;
}


/*****************************************************************************
* void  f_stv0362_hw_init()
* Description: stv0362 initialization (set initial register)
*
* Arguments:
*  Parameter1: struct nim_device *dev, to meet ALi code Device Address usage
*
* Return Value: None
*****************************************************************************/
void f_stv0362_hw_init(struct nim_device *dev)
{
    UINT32  i;
    UINT8  *data;
    UINT8  tmp_data;
    UINT8  reg_addr;
UINT8 temp=0;

    ALI_PRINTF("Enter f_stv0362_hw_init()...\n");

    f_stv0362_read(dev->base_addr, (UINT8) R0362_ID, &tmp_data, 1);
    ALI_PRINTF("f_stv0362_hw_init(): STV0362 Chip ID = 0x%02x\n", tmp_data);

    f_stv0362_disable(dev); /*20080401*/

    /* STV0362_Demod_Integration [begin] */
    for (i=0; i<STV0362_NBREGS; i++)
    {
        data = &STV0362_DefVal[i];
        reg_addr = STV0362_Address[i];
        f_stv0362_write(dev->base_addr, (UINT8) reg_addr, data, 1);
     f_stv0362_read(dev->base_addr, (UINT8) reg_addr, &temp, 1);
     ALI_PRINTF("reg[%x]=0x%x\n",reg_addr,temp);
    }

    //note: copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_Open() --- [begin]
    /* Set serial/parallel data mode */
    if (gSTV0362_InitParams.TSOutputMode == STTUNER_TS_MODE_SERIAL)
    {
        ChipSetField(dev, F0362_OUTRS_SP, 1);
    }
    else
    {
        ChipSetField(dev, F0362_OUTRS_SP, 0);
    }

    /* set data clock polarity inversion mode (rising/falling) */
    switch(gSTV0362_InitParams.ClockPolarity)
    {
        case STTUNER_DATA_CLOCK_POLARITY_RISING:
            ChipSetField(dev, F0362_CLK_POL, 1);
            break;
        case STTUNER_DATA_CLOCK_POLARITY_FALLING:
            ChipSetField(dev, F0362_CLK_POL, 0);
            break;
        case STTUNER_DATA_CLOCK_POLARITY_DEFAULT:
            ChipSetField(dev, F0362_CLK_POL, 0);
            break;
        default:
            break;
    }
    //note: copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_Open() --- [end]
    /* STV0362_Demod_Integration [end] */

    f_stv0362_enable(dev); /*20080401*/

    //enable I2C repeater access for Tuner I2C access
    //ST_d0362_repeateraccess(dev, TRUE); //20080512: This setting will be usful before tuner programming only

#if 1 /*don't need to create task for tracking lock status*/
    stv0362_CurChannelInfo->Dis_TS_Output = 1;
#else
    //start CC_Tracking task
    if(FALSE == f_stv0362_CC_tracking_task_init(dev))
    {
        NIM_PRINTF("f_stv0362_hw_init(): Creak NIM tracking task fail!\n");
    }
#endif
    NIM_PRINTF("Exit f_stv0362_hw_init() normally.\n");
    return;
}


/*****************************************************************************
* INT32 f_stv0362_open(struct nim_device *dev)
* Description: stv0362 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_open(struct nim_device *dev)
{
    ALI_PRINTF("Enter f_stv0362_open()...\n");

    l_nim0362_sema_id = osal_semaphore_create(1);
    if (l_nim0362_sema_id == OSAL_INVALID_ID)
    {
        NIM_PRINTF("f_stv0362_open(): Create semaphore fail!\n");
        return ERR_FAILUE;
    }

    stv0362_CurChannelInfo = (struct stv0362_Lock_Info *)MALLOC(sizeof(struct stv0362_Lock_Info));
    if (stv0362_CurChannelInfo == NULL)
    {
        NIM_PRINTF("f_stv0362_open(): MALLOC fail!\n");
        return ERR_FAILUE;
    }
    MEMSET(stv0362_CurChannelInfo, 0, sizeof(struct stv0362_Lock_Info));

    NIM_PRINTF("f_stv0362_open(): => f_stv0362_hw_init()...\n");
    f_stv0362_hw_init(dev);

    NIM_PRINTF("Exit f_stv0362_open() normally.\n");
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_stv0362_close(struct nim_device *dev)
* Description: stv0362 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_close(struct nim_device *dev)
{
  #if 1 //weiching: here, follow ALi stv0361 code for safety [begin]
    UINT8 data = ( STV0362_DefVal[f_stv0362_ArrayIdx_of_Reg(R0362_TOPCTRL)] | f_stv0362_Mask_of_RegField(F0362_STDBY) | f_stv0362_Mask_of_RegField(F0362_TS_DIS) ); //Standby, Disable TS
    ALI_PRINTF("Enter f_stv0362_close()...\n");
    f_stv0362_write(dev->base_addr, (UINT8) R0362_TOPCTRL, &data, 1);
  #endif //weiching: here, follow ALi stv0361 code for safety [end]

    stv0362_CurChannelInfo->Dis_TS_Output = 1;
    NIM_PRINTF("Exit f_stv0362_close() normally.\n");
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_disable(struct nim_device *dev)
* Description: stv0362 disable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 f_stv0362_disable(struct nim_device *dev)
{
    ALI_PRINTF("Enter f_stv0362_disable()...\n");
  #if 0 /*don't need to disable the nim interface except power-off*/
    UINT8 data = ( STV0362_DefVal[f_stv0362_ArrayIdx_of_Reg(R0362_TOPCTRL)] | f_stv0362_Mask_of_RegField(F0362_TS_DIS) ); //Disable TS
    f_stv0362_write(dev->base_addr, (UINT8) R0362_TOPCTRL, &data, 1);
    stv0362_CurChannelInfo->Dis_TS_Output = 1;
   #endif
    NIM_PRINTF("Exit f_stv0362_disable() normally.\n");
    return SUCCESS;
}

/***********************************************************************************
--FUNCTION  ::  Core_Switch
--ACTION    ::  Switch Core
--PARAMS IN ::  nim_device *dev
--PARAMS OUT::  NONE
--RETURN    ::
--**********************************************************************************/

static INT32 f_stv0362_Core_Switch(struct nim_device *dev)
{
    UINT8 data;

    NIM_PRINTF("Enter f_stv0362_Core_Switch()...\n");

    ChipSetField(dev, F0362_CORE_ACTIVE, 0);

    osal_delay(5000); //5 ms

    ChipSetField(dev, F0362_CORE_ACTIVE, 1);

    //should delay 350 ms
    for(data=0; data<35; data++)
    {
        if (stv0362_autoscan_stop_flag)
        {
            NIM_PRINTF("f_stv0362_Core_Switch(): channel search was broken!!\n");
            return ERR_FAILED;
        }
        osal_delay(10000); //10 ms
    }

    NIM_PRINTF("Exit f_stv0362_Core_Switch() normally.\n");
    return SUCCESS;
}

#if 1 /*don't need to create task for tracking lock status*/
/*****************************************************************************
* INT32 f_stv0361_get_lock_status(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_lock_status(struct nim_device *dev, UINT8 *lock)
{
    UINT8 temp=0;
    static UINT32 t1;
    //UINT8  data[2];
    static UINT8 rec_lock=0;

#ifdef NIM_DEBUG
  *lock=1;
return SUCCESS;
#endif

    //*lock= stv0362_CurChannelInfo->Lock_Val;
    //NIM_PRINTF(" get LOCK !! *lock=%d\n", *lock);
    if((( read_tsc() - t1 )/WAIT_1ms)> 1000 )
        {

                //libc_printf("%d\n",( read_tsc() - t1 )/WAIT_1ms);

                t1 = read_tsc();
                 f_stv0362_read(dev->base_addr, 0x09, &temp, 1);
                 ALI_PRINTF("CR=%d,LK=%x,PRF=%x,AGC=%x,SYR=%x,TPS=%x,data=%x\n",(temp&0x07),(temp&0x08)>>3,(temp&0x10)>>4,(temp&0x20)>>5,(temp&0x40)>>6,(temp&0x80)>>7,temp);

                if(((temp&0xf8)>>3)!=0x1f)
                {
                    *lock=0;
                stv0362_CurChannelInfo->Lock_Val=0;
                rec_lock=0;
                }
                else
                {
                    *lock=1;
                    stv0362_CurChannelInfo->Lock_Val=1;
                    rec_lock=1;
                    #ifdef NIM_DEBUG
                    f_stv0362_read(dev->base_addr, 0x02, &temp, 1);
                     ALI_PRINTF("TS_EN=%x\n",temp);

                     f_stv0362_read(dev->base_addr, 0x24, &temp, 1);
                    ALI_PRINTF("PPM_CP_AMP=%d\n",temp);
                    #endif
                }

                if ((!stv0362_CurChannelInfo->Dis_TS_Output)&&(stv0362_CurChannelInfo->Lock_Val==0))
                {
                    FE_362_Tracking(dev);
                    }
                //f_stv0362_read(dev->base_addr, 0x80, data, 2);
                //libc_printf("CORE_ACT=%x,COR_STAUS_80=%x,COR_STAUS_81=%x,data_80=%x,data_81=%x\n",    (data[0]&0x20)>>5,(data[0]&0x0F),(data[1]&0x0F),data[0],data[1]);

        }

    *lock=rec_lock;

    return SUCCESS;
}
#endif



/*****************************************************************************
* INT32 f_stv0362_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *lock, [output] lock status
*
* Return Value: INT32, Error code
*****************************************************************************/
static INT32 f_stv0362_get_lock(struct nim_device *dev, UINT8 *lock)
{
#ifdef NIM_DEBUG
  *lock=1;
return SUCCESS;
#endif
    *lock =stv0362_CurChannelInfo->Lock_Val;

    /*if((( read_tsc() - t1 )/WAIT_1ms)> 200 )
    {
        f_stv0362_read(dev->base_addr, 0x80, data, 2);
        // libc_printf("data_0=%x,data_1=%x,AG conf=%d,HOLD=%x,COR_STAUS_80=%x,TPS_LK=%x,SYR_LK=%x,AGC_LK,=%x,COR_STAUS_81=%x\n",    data[0],data[1],(data[0]&0xc0)>>6,(data[0]&0x40)>>5,(data[0]&0x20)>>4,(data[0]&0x07),(data[1]&0x40)>>6,(data[1]&0x20)>>5,(data[1]&0x10)>>4,(data[1]&0x0F));
        libc_printf("data_0=%x,data_1=%x,CORE_ACT=%x,COR_STAUS_80=%x,COR_STAUS_81=%x\n",    data[0],data[1],(data[0]&0x40)>>5,(data[0]&0x0F),(data[1]&0x0F));

    }*/



   #if 0 //By default, close this because it will print too many lock status msg
    NIM_PRINTF("f_stv0362_get_lock(): lock = %d\n", *lock);
   #endif
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_get_freq(struct nim_device *dev, UINT32 *freq)
* Read frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *freq, [output] frequency
*
* Return Value: void
*****************************************************************************/
static INT32 f_stv0362_get_freq(struct nim_device *dev, UINT32 *freq)
{
    *freq = stv0362_CurChannelInfo->Frequency;

    NIM_PRINTF("f_stv0362_get_freq(): freq = %d KHz\n", *freq);
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_get_code_rate(struct nim_device *dev, UINT8* code_rate)
* Description: Read stv0362 code rate
*   FEC status (b6-b4)                      code rate                 return value
*   0                   1/2         1
*   1                   2/3         4
*   2                   3/4         8
*   3                   5/6         16
*   5                   7/8         32
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
    STTUNER_FECRate_t st_fec_rate;

    NIM_PRINTF("Enter f_stv0362_get_code_rate()...\n");

    ST_d0362_GetFECRates(dev, &st_fec_rate);
    stv0362_CurChannelInfo->HPRates = st_fec_rate;
    NIM_PRINTF("f_stv0362_get_code_rate(): code_rate = %d\n", stv0362_CurChannelInfo->HPRates);

    switch(stv0362_CurChannelInfo->HPRates)
    {
        case STTUNER_FEC_1_2:
            *code_rate = FEC_1_2;
            break;
        case STTUNER_FEC_2_3:
            *code_rate = FEC_2_3;
            break;
        case STTUNER_FEC_3_4:
            *code_rate = FEC_3_4;
            break;
        case STTUNER_FEC_5_6:
            *code_rate = FEC_5_6;
            break;
        case STTUNER_FEC_7_8:
            *code_rate = FEC_7_8;
            break;
        default:
            ALI_PRINTF("f_stv0362_get_code_rate() ==> Error\n");
            *code_rate = 0; /* error */
            break;
    }

    NIM_PRINTF("Exit f_stv0362_get_code_rate() normally.\n");
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_get_GI(struct nim_device *dev, UINT8 *guard_interval)
*
* Description: Read stv0362 guard interval
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* guard_interval
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    STTUNER_Guard_t  st_guard_interval;

    NIM_PRINTF("Enter f_stv0362_get_GI()...\n");

    ST_d0362_GetGuard(dev, &st_guard_interval);
    stv0362_CurChannelInfo->Guard = st_guard_interval;
    NIM_PRINTF("f_stv0362_get_GI(): guard_interval = %d\n", stv0362_CurChannelInfo->Guard);

    switch(stv0362_CurChannelInfo->Guard)
    {
        case STTUNER_GUARD_1_32:
            *guard_interval = guard_1_32;
            break;
        case STTUNER_GUARD_1_16:
            *guard_interval = guard_1_16;
            break;
        case STTUNER_GUARD_1_8:
            *guard_interval = guard_1_8;
            break;
        case STTUNER_GUARD_1_4:
            *guard_interval = guard_1_4;
            break;
        default:
            ALI_PRINTF("f_stv0362_get_GI() ==> Guard Error\n");
            *guard_interval = 0xff; /* error */
            break;
    }

    NIM_PRINTF("Exit f_stv0362_get_GI() normally.\n");
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_stv0362_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
* Description: Read stv0362 fft_mode
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* fft_mode
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    STTUNER_Mode_t st_fft_mode;

    NIM_PRINTF("Enter f_stv0362_get_fftmode()...\n");

    ST_d0362_GetMode(dev, &st_fft_mode);
    stv0362_CurChannelInfo->Mode = st_fft_mode;
    NIM_PRINTF("f_stv0362_get_fftmode(): fft_mode = %d\n", stv0362_CurChannelInfo->Mode);

    switch(stv0362_CurChannelInfo->Mode)
    {
        case STTUNER_MODE_2K:
            *fft_mode = MODE_2K;
            break;
        case STTUNER_MODE_8K:
            *fft_mode = MODE_8K;
            break;
        default:
            ALI_PRINTF("f_stv0362_get_fftmode() ==> Error\n");
            *fft_mode = 0xff; /* error */
            break;
    }

    NIM_PRINTF("Exit f_stv0362_get_fftmode() normally.\n");
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_stv0362_get_modulation(struct nim_device *dev, UINT8 *modulation)
* Description: Read stv0362 modulation
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* modulation
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    STTUNER_Modulation_t st_modulation;

    NIM_PRINTF("Enter f_stv0362_get_modulation()...\n");

    ST_d0362_GetModulation(dev, &st_modulation);
    stv0362_CurChannelInfo->Modulation = st_modulation;
    NIM_PRINTF("f_stv0362_get_modulation(): modulation = %d\n", stv0362_CurChannelInfo->Modulation);

    switch(stv0362_CurChannelInfo->Modulation)
    {
        case STTUNER_MOD_QPSK:
            *modulation = TPS_CONST_QPSK;
            break;
        case STTUNER_MOD_16QAM:
            *modulation = TPS_CONST_16QAM;
            break;
        case STTUNER_MOD_64QAM:
            *modulation = TPS_CONST_64QAM;
            break;
        default:
            ALI_PRINTF("f_stv0362_get_modulation() ==> UNKNOWN\n");
            *modulation = 0xff; /* error */
            break;
    }

    NIM_PRINTF("Exit f_stv0362_get_modulation() normally.\n");
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    *Inv = stv0362_CurChannelInfo->Spectrum;

    NIM_PRINTF("f_stv0362_get_specinv(): Spectrum Inv = %d\n", *Inv);
    return SUCCESS;
}

/*****************************************************************************
* INT32 f_stv0362_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec);

* Description: stv0362 channel change operation
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
static INT32 f_stv0362_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
    INT32 ret_code;
    FE_362_Error_t error;
    BOOL f_scan_success = FALSE;

    UINT32  fft_Mode;
    UINT32  Guard;
    UINT8 tmp_data;
        struct nim_stv0362_private *dev_priv;
            dev_priv = gNim_stv0362_private;
            //UINT8 temp_data;
#ifdef NIM_DEBUG
return SUCCESS;
#endif
//freq=722000;

    libc_printf(" freq=%d, bandwidth=%d, guard_interval=%d, fft_mode=%d, modulation=%d, fec=%d, usage_type=%d, inverse=%d\n\n",
               freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse);

    osal_semaphore_capture(l_nim0362_sema_id, TMO_FEVR);
     stv0362_CurChannelInfo->CC_Tracking_staus=0;

#if 0

temp_data = dev_priv->Tuner_Control.config_data.ptST0362[5];
 ChipSetRegisters(dev, 0x4c, &temp_data, 1);
temp_data= ChipGetOneRegister(dev, 0x4C);
ALI_PRINTF("Set RS=%x\n",temp_data);

  ChipSetField(dev, F0362_ENA8_LEVEL, dev_priv->Tuner_Control.config_data.ptST0362[6]);
  temp_data= ChipGetOneRegister(dev, 0x4D);
  ALI_PRINTF("Set RSOUT=%x\n",temp_data);

f_stv0362_read(dev->base_addr, (UINT8) R0362_ID, &tmp_data, 1);
ALI_PRINTF(" Chip ID = 0x%02x\n\n", tmp_data);

#endif





#if 1 /*Fix manualscan will fail to lock again for stv0362 demod*/
    if (stv0362_CurChannelInfo->Lock_Val == 1 && !CheckChannelChange(freq, bandwidth))
    {
        osal_semaphore_release(l_nim0362_sema_id);
        return SUCCESS;
}
#endif

    stv0362_CurChannelInfo->CC_Tracking_flag = 1; //TRACKING CHECK IF HAVE DONE CC, TRACKING RESET.
    stv0362_CurChannelInfo->Lock_Val = 0;
    stv0362_CurChannelInfo->Dis_TS_Output = 0; //check TS w/wo output
    //convert ALi constant definitions to ST definitions -- [begin]
    if(guard_interval == guard_1_32)
        Guard = STTUNER_GUARD_1_32;
    else if(guard_interval == guard_1_16)
        Guard = STTUNER_GUARD_1_16;
    else if(guard_interval == guard_1_8)
        Guard = STTUNER_GUARD_1_8;
    else
        Guard = STTUNER_GUARD_1_4;

    if(fft_mode == MODE_8K)
        fft_Mode = STTUNER_MODE_8K;
    else
        fft_Mode = STTUNER_MODE_2K;

//20080512: Based on application layer's cmd to set proper hierarchy mode to demodulator [Begin]
#if 1//(HIERARCHY_MODE == TRUE)
   STTUNER_Hierarchy_t eHierarchyMode;

    if(!priority) //Low proority
        eHierarchyMode= STTUNER_HIER_LOW_PRIO;
    else //High proority or any
        eHierarchyMode = STTUNER_HIER_HIGH_PRIO; //STTUNER_HIER_PRIO_ANY

    error = ST_d0362_ScanFrequency(dev,
                                   freq,                    //InitialFrequency, unit: KHz
                                   &f_scan_success,         //ScanSuccess result
                                   fft_Mode,                //Mode
                                   Guard,                   //Guard
                                   STTUNER_FORCENONE,       //Force
                                   eHierarchyMode,   //Hierarchy
                                   STTUNER_INVERSION_AUTO,  //Spectrum
                                   STTUNER_OFFSET_NONE,     //FreqOff
                                   bandwidth,               //ChannelBW
                                   STTUNER_NORMAL_IF_TUNER, //IF_IQ_Mode (whether normal IF or long path IF or long path IQ)
                                   0);                      //EchoPos
#else
//20080512: Based on application layer's cmd to set proper hierarchy mode to demodulator [End]
    error = ST_d0362_ScanFrequency(dev,
                                   freq,                    //InitialFrequency, unit: KHz
                                   &f_scan_success,         //ScanSuccess result
                                   fft_Mode,                //Mode
                                   Guard,                   //Guard
                                   STTUNER_FORCENONE,       //Force
                                   STTUNER_HIER_PRIO_ANY,   //Hierarchy
                                   STTUNER_INVERSION_AUTO,  //Spectrum
                                   STTUNER_OFFSET_NONE,     //FreqOff
                                   bandwidth,               //ChannelBW
                                   STTUNER_NORMAL_IF_TUNER, //IF_IQ_Mode (whether normal IF or long path IF or long path IQ)
                                   0);                      //EchoPos
#endif //20080512: Based on application layer's cmd to set proper hierarchy mode to demodulator


 //20080606: update the status of Lock_Val [Begin]
    stv0362_CurChannelInfo->ChannelBW = bandwidth; //place here, suitable??
    stv0362_CurChannelInfo->Frequency = freq; //place here, suitable??
 //20080606: update the status of Lock_Val [End]
    if((error == FE_362_NO_ERROR) && (f_scan_success == TRUE))
    {


    while(1)
    {
    //ALI_PRINTF("f_stv0361_channel_change...\n");
     tmp_data=0x00;
     f_stv0362_write(dev->base_addr,0x2,&tmp_data,1);

      f_stv0362_read(dev->base_addr,0x2,&tmp_data,1);
      ALI_PRINTF("LK=>TS EN=%x\n",tmp_data);
     if(tmp_data==0x00)
         break;
    }

    UINT8 temp=0;
     // f_stv0362_write(dev->base_addr,  0x88, &temp, 1);
    /* f_stv0362_read(dev->base_addr,  0x88, &temp, 1);
     ALI_PRINTF("TARGET=%x\n,",temp);

       // f_stv0362_write(dev->base_addr,  0x4B, &temp, 1);
     f_stv0362_read(dev->base_addr,  0x4B, &temp, 1);
     ALI_PRINTF("VSEARCH=%x\n",temp);

     //  f_stv0362_write(dev->base_addr,  0x4C, &temp, 1);
     f_stv0362_read(dev->base_addr,  0x4C, &temp, 1);
     ALI_PRINTF("RS=%x\n",temp);

     // f_stv0362_write(dev->base_addr,  0x4D, &temp, 1);
     f_stv0362_read(dev->base_addr,  0x4D, &temp, 1);
     ALI_PRINTF("RSOUT=%x\n",temp);

      f_stv0362_read(dev->base_addr, 0x4E, &temp, 1);
    ALI_PRINTF("ERR_CLT=%x\n",temp);


     */

     f_stv0362_read(dev->base_addr, 0x24, &temp, 1);
    ALI_PRINTF("PPM_CP_AMP=%x\n",temp);

        // f_stv0362_read(dev->base_addr, 0x01, &temp, 1);
    //libc_printf("I2Crpt=%x\n",temp);





 //20080606: update the status of Lock_Val [Begin]
        //need to update some status info for ALi back-end code!!
        //stv0362_CurChannelInfo->ChannelBW = bandwidth; //place here, suitable??
        //stv0362_CurChannelInfo->Frequency = freq; //place here, suitable??
 //20080606: update the status of Lock_Val [End]

        stv0362_CurChannelInfo->Lock_Val = 1;
        ALI_PRINTF("f_stv0362_channel_change SUCCESS !!!\n");
        NIM_PRINTF("f_stv0362_channel_change(): SUCCESS!! stv0362 LOCK OK!! \n\n");
        ret_code = SUCCESS;
    }
    else
    {
        stv0362_CurChannelInfo->Lock_Val = 0; //C20080606: update the status of Lock_Val
        ALI_PRINTF("f_stv0362_channel_change FAIL !!!\n");
        NIM_PRINTF("f_stv0362_channel_change(): FAILED!! stv0362 LOCK Failed!! \n\n");
        ret_code = ERR_FAILED;
    }

#if 1 /*Fix manualscan will fail to lock again for stv0362 demod*/
    stv0362_CurChannelInfo->CC_Tracking_Frequency = freq;
    stv0362_CurChannelInfo->CC_Tracking_ChannelBW = bandwidth;
    stv0362_CurChannelInfo->CC_Tracking_Modulation = modulation;
    stv0362_CurChannelInfo->CC_Tracking_Mode = fft_Mode;
    stv0362_CurChannelInfo->CC_Tracking_Guard = Guard;
    stv0362_CurChannelInfo->CC_Tracking_Spectrum = STTUNER_INVERSION_AUTO;
    stv0362_CurChannelInfo->CC_Tracking_flag = 0;
#endif

    osal_semaphore_release(l_nim0362_sema_id);

    return ret_code;
}


/*****************************************************************************
* INT32 f_stv0362_channel_search(struct nim_device *dev, UINT32 freq);

* Description: stv0362 channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq : Frequency*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT16 freq_offset, UINT8 priority)
{
    UINT8   i, j;
    INT32   chsearch;
    UINT32  Center_NPRO, Search_NPRO, tmp_freq;

    NIM_PRINTF("Enter f_stv0362_channel_search()..., freq = %d, bandwidth = %d\n", freq, bandwidth);

    //osal_flag_clear(nim_0362_flag,NIM_0362_SCAN_END);

    Center_NPRO = (((freq + 36250)*6)/100);

    for(i=0; i<1; i++)
    {
        chsearch = ERR_FAILED;
        if (stv0362_autoscan_stop_flag)
        {
            stv0362_autoscan_stop_flag = 0;
            break;
        }

        if(i % 2)
            j = 1;
        else
            j = (-1);

        Search_NPRO = (((i+1)/2)*30*j)+Center_NPRO;
        tmp_freq = (Search_NPRO*100)/6;
        NIM_PRINTF("f_stv0362_channel_search(): i=%d, Search_NPRO=%d, tmp_freq=%d\n", i, Search_NPRO, tmp_freq);
        if(tmp_freq < 36250) {
            NIM_PRINTF("f_stv0362_channel_search(): channel search break!!=%d ==> tmp_freq(%d)\n", i, tmp_freq);
            break;
        }
        else
            freq = tmp_freq - 36250;

        NIM_PRINTF("f_stv0362_channel_search(): => f_stv0362_channel_change()..., freq = %d\n", freq);

        chsearch = f_stv0362_channel_change(dev, freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse, priority);
        if(chsearch == SUCCESS)
        {
            break;
        }
    }

    //osal_flag_set(nim_0362_flag, NIM_0362_SCAN_END);

    ALI_PRINTF ("channel_search RETURN !!!  chsearch=%d\n",chsearch);
    NIM_PRINTF("Exit f_stv0362_channel_change() normally.\n");
    return chsearch;
}


/*****************************************************************************
* INT32 f_stv0362_get_AGC(struct nim_device *dev, UINT16 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *agc, [output] AGC value
*
* Return Value: INT32, Error code
*****************************************************************************/
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetAGC()
static INT32 f_stv0362_get_AGC(struct nim_device *dev, UINT16 *agc)
{
    UINT16    Data1=0;
    UINT16    Data2=0;
    //UINT16    agc_min,agc_max;
    //struct nim_stv0362_private *dev_priv;
//    static UINT32 t2;

#ifdef NIM_DEBUG
 *agc=100;
return SUCCESS;
#endif

  if(stv0362_CurChannelInfo->Lock_Val==0)
  {
        *agc=0;
      return SUCCESS;
  }
  else
  {

        Data1 = ChipGetField(dev, F0362_AGC2_VAL_LO);
        Data2 = ChipGetField(dev, F0362_AGC2_VAL_HI);
        stv0362_CurChannelInfo->AGC_Val= (((Data2 << 8) & 0x0F00) | (Data1) );
        /*if((( read_tsc() - t2 )/WAIT_1ms)> 2000 )
        {
            soc_printf("LO=%x,HI=%x%d\n", Data1,Data2,stv0362_CurChannelInfo->AGC_Val);
        }*/

        *agc = (stv0362_CurChannelInfo->AGC_Val);
        *agc =100;
  }
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_stv0362_get_SNR(struct nim_device *dev, UINT8 *snr)
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
#define HIGH_LEVEL    190 //230
#define MEDIUM_LEVEL  170 //190
#define LOW_LEVEL     150 //170

static INT32 f_stv0362_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    UINT8   data;
    //OSAL_ER result;
   // UINT32  flgptn;
    UINT8 modulation;
UINT32 SignalQuality=0 ;
UINT32 Ber=0;

#ifdef NIM_DEBUG
 *snr=100;
return SUCCESS;
#endif

    NIM_PRINTF("Enter f_stv0362_get_SNR()...\n");

    stv0362_CurChannelInfo->SNR_Val = ChipGetField(dev, F0362_CHC_SNR); //steven: get SNR value.

    data = stv0362_CurChannelInfo->SNR_Val;
    /*fix show signal quality fail for unlock status*/
    if(0 == stv0362_CurChannelInfo->Lock_Val)
    //if((0 == stv0362_CurChannelInfo->Lock_Val) && (*snr > 60))
    {
        *snr = FALSE_SNR_VALUE;
        return SUCCESS;
    }


    ST_d0362_GetSignalQuality(dev,  &SignalQuality, &Ber);
    if(Ber>=2000000)
    {
        ALI_PRINTF(" C/N= %d , BER=%d \n", SignalQuality, Ber);
        ;
    }


    f_stv0362_get_modulation(dev, &modulation);

    if (modulation == TPS_CONST_64QAM)
    {
        if (data > MEDIUM_LEVEL)
        {
            if (data > HIGH_LEVEL)
                data = HIGH_LEVEL;
            *snr = (data - MEDIUM_LEVEL) * 30 / (HIGH_LEVEL - MEDIUM_LEVEL) + 70;  //140~80
        }
        else if (data > 140)
        {
            *snr = (data - 140) * 30 / (MEDIUM_LEVEL - 140) + 40;  //140~80
        }
        else
        {
            if (data < 100)
                data = 100;
            *snr = (data - 100) * 40 / (140 - 100);  //80~0
        }
    }
    else if (modulation == TPS_CONST_16QAM)
    {
        if (data > LOW_LEVEL)
        {
            if (data > HIGH_LEVEL)
                data = HIGH_LEVEL;
            *snr = (data - LOW_LEVEL) * 30 / (HIGH_LEVEL - LOW_LEVEL) + 70;  //140~80
        }
        else if (data > 110)
        {
            *snr = (data - 110) * 30 / (LOW_LEVEL - 110) + 40;  //140~80
        }
        else
        {
            if (data < 50)
                data = 50;
            *snr = (data - 50) * 40 / (110 - 50);  //80~0
        }
    }
    else //TPS_CONST_QPSK
    {
        if (data > LOW_LEVEL)
        {
            if (data > HIGH_LEVEL)
                data = HIGH_LEVEL;
            *snr = (data - LOW_LEVEL) * 30 / (HIGH_LEVEL - LOW_LEVEL) + 70;  //140~80
        }
        else if (data > 70)
        {
            *snr = (data - 70) * 30 / (LOW_LEVEL - 70) + 40;  //140~80
        }
        else
        {
            if (data < 20)
                data = 20;
            *snr = (data - 20) * 40 / (70 - 20);  //80~0
        }
    }

    NIM_PRINTF("f_stv0362_get_SNR(): SNR =%d\n", *snr);

    NIM_PRINTF("Exit f_stv0362_get_SNR() normally.\n");
    return SUCCESS;
}

/**********************************************************************
**FUNCTION  ::  f_stv0362_PowOf2
**ACTION    ::  Compute  2^n (where n is an integer)
**PARAMS IN ::  number -> n
**PARAMS OUT::  NONE
**RETURN    ::  2^n
******************************************************************/
UINT32 f_stv0362_PowOf2(UINT8 number)
{
    UINT8 i;
    UINT32 result=1;

    for(i=0; i<number; i++)
        result *= 2;

    return result;
}


//wctu test STV0362 => 此函式要改寫!!  [begin]
/*****************************************************************************
* INT32 f_stv0362_get_BER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 f_stv0362_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    //UINT8   data,source,prf;
    //UINT8   tdata[4];
    __MAYBE_UNUSED__ UINT32  error,quoz,temp1,temp,ber,pkber,pkerr,i,pkbar;
    //OSAL_ER result;
    //UINT32  flgptn;

    NIM_PRINTF("Enter f_stv0362_get_BER()...\n");

    if (stv0362_CurChannelInfo->Lock_Val != 1)
    {
        *vbber = 0;
        return SUCCESS;
    }
    else
    {
        error = (stv0362_CurChannelInfo->BER_Val) * 100000;
        quoz = 1;

        quoz = f_stv0362_PowOf2(12 + 2*(2));
        quoz *= 8; //ERR BIT RATE

        ber = (error/quoz);

        temp = 100000 - ber;
        temp1 = (100000000/temp)*100;
        temp = (temp1 - 100000)*(temp1 + 100000);
        if (temp == 0)
        {
            *vbber = 100;
            NIM_PRINTF("f_stv0362_get_BER(): *vbber = %d\n", *vbber);
            return SUCCESS;
        }
        else
        {
            temp1 = (100000000/temp)*100;
            NIM_PRINTF("f_stv0362_get_BER(): te2222 = %d\n",temp1);
        }

        for (i=16; i>0; i--)
        {
            if (temp1 & (1 << i))
            {
                temp = ((temp1) / (1 << i));
                temp += i;
                temp *= 3;
                break;
            }
        }

        if((stv0362_CurChannelInfo->BER_HB <= 10) && (stv0362_CurChannelInfo->PER_LB > 40))
        {
            *vbber = 25 + (stv0362_CurChannelInfo->BER_LB/100);
            NIM_PRINTF("f_stv0362_get_BER(): FALSE, vbber = %d\n", *vbber);
        }
        else
        {
            *vbber = 2*temp;
            NIM_PRINTF("f_stv0362_get_BER(): vbber = %d\n", *vbber);
        }
    }

    NIM_PRINTF("Exit f_stv0362_get_BER() normally.\n");
    return SUCCESS;
}
//wctu test STV0362 => 此函式要改寫!!  [end]


/*****************************************************************************
* INT32 f_stv0362_i2c_bypass(struct nim_device *dev)
*
*  Set I2C bypass
*
*Arguments:
*  Parameter1: struct nim_device *dev
*
*Return Value: INT32
*****************************************************************************/
/*static INT32 f_stv0362_i2c_bypass(struct nim_device *dev)
{
    //enable I2C repeater access for Tuner I2C access
    ST_d0362_repeateraccess(dev, TRUE);
    return SUCCESS;
} */

/*****************************************************************************
* INT32 f_stv0362_get_RFLevel(struct nim_device *dev, INT32 *RFlevel)
*
*  Get RF level
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 *RFlevel, [output] RF level value
*
* Return Value: INT32, Error code
*****************************************************************************/
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetRFLevel()
static INT32 f_stv0362_get_RFLevel(struct nim_device *dev, INT32 *RFlevel)
{
    UINT32 Data1=0;
    UINT32 Data2=0;
    UINT8  Rfregval;
    INT32  RFleveldBm = 0;

    NIM_PRINTF("Enter f_stv0362_get_RFLevel()...\n");

    /* Set the START_ADC field of register EN_RF_AGC1 to Enable the RF clock */

    /* Now read the RF_AGC1_LEVEL field of the register AGC1RF */
    Rfregval = ChipGetField(dev, F0362_RF_AGC1_LEVEL_HI);

    if (Rfregval >= 255)
    {
        Data1 = ChipGetField(dev, F0362_AGC2_VAL_LO);
        Data2 = ChipGetField(dev, F0362_AGC2_VAL_HI);
        Data2 <<= 8;
        Data2 &= 0x0F00;
        Data2 |= Data1;

        if (Data2 >= 670 && Data2 <= 730)
        {
            RFleveldBm = -((Data2 - 100)/20 + 21);
        }
        else if (Data2 >= 730 && Data2 <= 769)
        {
            RFleveldBm = -((Data2 - 100)/20 + 22);
        }
        else if (Data2 >= 770 && Data2 <= 820)
        {
            RFleveldBm = -((Data2 - 100)/20 + 22);
        }
        else if (Data2 >= 821 && Data2 <= 900)
        {
            RFleveldBm = -((Data2 - 100)/20 + 22);
        }
        else
        {
            RFleveldBm = STTUNER_LOW_RF;
            NIM_PRINTF("RF level is  < -60dBm \n");
        }
    }
    else if (Rfregval >= 160 && Rfregval <= 170)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 39);
    }
    else if (Rfregval >= 171 && Rfregval <= 220)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 38);
    }
    else if (Rfregval >= 221 && Rfregval <= 230)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 41);
    }
    else if (Rfregval >= 231 && Rfregval <= 245)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 44);
    }
    else if (Rfregval >= 246 && Rfregval <= 253)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 51);
    }
    else if (Rfregval == 254)
    {
        RFleveldBm = -(((Rfregval - 50)/2) - 48);
    }
    else
    {
        RFleveldBm = STTUNER_HIGH_RF;
        NIM_PRINTF("RF level is  > -20dBm \n");
    }

    *RFlevel = RFleveldBm;

    NIM_PRINTF("Exit f_stv0362_get_RFLevel() normally.\n");
    return SUCCESS;
}


/*****************************************************************************
* INT32 f_stv0362_StandByMode(struct nim_device *dev, STTUNER_StandByMode_t PowerMode)
*
*  Set Demod power mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: STTUNER_StandByMode_t PowerMode, normal or standby power mode
*
* Return Value: INT32, Error code
*****************************************************************************/
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_StandByMode()
static INT32 f_stv0362_StandByMode(struct nim_device *dev, STTUNER_StandByMode_t PowerMode)
{
    NIM_PRINTF("Enter f_stv0362_StandByMode()...\n");

    switch(PowerMode)
    {
        case STTUNER_NORMAL_POWER_MODE:
            if(gSTV0362_InitParams.StandBy_Flag == STTUNER_STANDBY_POWER_MODE)
            {
                ChipSetField(dev, F0362_STDBY, 0);
                ChipSetField(dev, F0362_CORE_ACTIVE, 0);
                WAIT_N_MS_362(5);
                ChipSetField(dev, F0362_CORE_ACTIVE, 1);

                gSTV0362_InitParams.StandBy_Flag = STTUNER_NORMAL_POWER_MODE;
            }
            break;
        case STTUNER_STANDBY_POWER_MODE:
            if(gSTV0362_InitParams.StandBy_Flag == STTUNER_NORMAL_POWER_MODE)
            {
                ChipSetField(dev, F0362_STDBY, 1);

                gSTV0362_InitParams.StandBy_Flag = STTUNER_STANDBY_POWER_MODE;
            }
            break;
        default:
            break;
    }

    NIM_PRINTF("Exit f_stv0362_StandByMode() normally.\n");
    return SUCCESS;
}

static INT32 f_stv0362_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    NIM_PRINTF("Enter f_stv0362_ioctl()...\n");

    switch(cmd)
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            stv0362_autoscan_stop_flag = param;
            break;
        case NIM_DRIVER_DISABLED:
            return f_stv0362_disable(dev);
            break;
        case NIM_DRIVER_GET_AGC:
            return f_stv0362_get_AGC(dev, (UINT16 *)param);
            break;
        case NIM_DRIVER_GET_BER:
            return f_stv0362_get_BER(dev, (UINT32 *)param);
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return f_stv0362_get_GI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_FFT_MODE:
            return f_stv0362_get_fftmode(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_MODULATION:
            return f_stv0362_get_modulation(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_SPECTRUM_INV:
            return f_stv0362_get_specinv(dev, (UINT8 *)param);
            break;
        default:
            break;
    }

    NIM_PRINTF("Exit f_stv0362_ioctl() normally.\n");
    return SUCCESS;
}


static INT32 f_stv0362_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
    struct NIM_CHANNEL_CHANGE *change_para;
    struct NIM_CHANNEL_SEARCH *search_para;
    NIM_PRINTF("Enter f_stv0362_ioctl_ext()...\n");
    switch (cmd)
    {
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            return f_stv0362_channel_change(dev, change_para->freq, change_para->bandwidth, \
                change_para->guard_interval, change_para->fft_mode, change_para->modulation, \
                change_para->fec, change_para->usage_type, change_para->inverse, change_para->priority);
            break;
        case NIM_DRIVER_CHANNEL_SEARCH:
            search_para = (struct NIM_CHANNEL_SEARCH *)param_list;
            return f_stv0362_channel_search(dev, search_para->freq, search_para->bandwidth, \
                search_para->guard_interval, search_para->fft_mode, search_para->modulation, \
                search_para->fec, search_para->usage_type, search_para->inverse, \
                search_para->freq_offset, search_para->priority);
            break;
        default:
            break;
    }
    NIM_PRINTF("Exit f_stv0362_ioctl_ext() normally.\n");
    return SUCCESS;
}




static INT32 f_stv0362_CC_tracking_task_init(struct nim_device *dev)
{
    //ER  ret_val;
    OSAL_T_CTSK t_ctsk;

    NIM_PRINTF("Enter f_stv0362_CC_tracking_task_init()...\n");
    NIM_PRINTF("Exit f_stv0362_CC_tracking_task_init() normally.\n");

    t_ctsk.stksz = 0x400;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.task = (FP)f_stv0362_CC_tracking_task;
    t_ctsk.para1 = (UINT32)dev;

    f_stv0362_CC_tracking_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == f_stv0362_CC_tracking_task_id)
    {
        NIM_PRINTF("create_task nim_stv0362_CC_tracking_task_id failed\n");

        NIM_PRINTF("Exit f_stv0362_CC_tracking_task_init() with error!\n");
        return FALSE;
    }

    NIM_PRINTF("Exit f_stv0362_CC_tracking_task_init() normally.\n");
    return TRUE;
}

void f_stv0362_CC_tracking_task(UINT32 param1)
{
    //UINT32 freq, Pre_TKCount=0, TKCount=0, LKCNT=0;
    //UINT32 bandwidth;
    //UINT8 data=0;
    //UINT8 lock=0;
    //UINT8 i=0;
    //UINT8 agc, snr, BerCNT;
    //UINT8  unlock_cnt=0;

    NIM_PRINTF("f_stv0362_CC_tracking_task(): stv0362 CC Tracking task started...\n");

    stv0362_CurChannelInfo->Dis_TS_Output = 1;

    while(1)
    {
        if (!stv0362_CurChannelInfo->Dis_TS_Output)
            {
            FE_362_Tracking((struct nim_device *)param1);
            }
        osal_task_sleep(300);
    }
}

static INT32 f_stv0362_CC_Tracking(struct nim_device *dev)
{
    UINT8     inverse;
    UINT32    freq, bandwidth;
    UINT32  fft_Mode;
    UINT32  Guard;

    ALI_PRINTF("f_stv0362_CC_Tracking...\n");
    freq = stv0362_CurChannelInfo->CC_Tracking_Frequency;
    bandwidth = stv0362_CurChannelInfo->CC_Tracking_ChannelBW;
    Guard=stv0362_CurChannelInfo->CC_Tracking_Guard;
    fft_Mode=stv0362_CurChannelInfo->CC_Tracking_Mode;
    inverse = stv0362_CurChannelInfo->CC_Tracking_Spectrum;
     stv0362_CurChannelInfo->CC_Tracking_staus=0;

    FE_362_Error_t error;
    BOOL f_scan_success;
    error = ST_d0362_ScanFrequency(dev,
                                   freq,                    //InitialFrequency, unit: KHz
                                   &f_scan_success,         //ScanSuccess result
                                   fft_Mode,                //Mode
                                   Guard,                   //Guard
                                   STTUNER_FORCENONE,       //Force
                                   STTUNER_HIER_PRIO_ANY,   //Hierarchy
                                   inverse,                  //Spectrum
                                   STTUNER_OFFSET_NONE,     //FreqOff
                                   bandwidth,               //ChannelBW
                                   STTUNER_NORMAL_IF_TUNER, //IF_IQ_Mode (whether normal IF or long path IF or long path IQ)
                                   0);                      //EchoPos

    if((error == FE_362_NO_ERROR) && (f_scan_success == TRUE))
    {
        ALI_PRINTF("f_stv0362_CC_Traking SUCCESS !!!\n");
        stv0362_CurChannelInfo->Lock_Val = 1;
    }
    else
    {
        stv0362_CurChannelInfo->Lock_Val = 0; //20080606: update the status of Lock_Val
        ALI_PRINTF("f_stv0362_CC_Traking FAIL !!!\n");
    }

    return SUCCESS;
}

static INT32 f_stv0362_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)
{
    *freq_offset = stv0362_CurChannelInfo->FreqOffset;
    return SUCCESS;
}

static INT32 f_stv0362_get_hier_mode(struct nim_device *dev, UINT8*hier)
{
    //*hier = stv0362_CurChannelInfo->Hierarchy;
   *hier = gSTV0362_SearchResultData.Result.Hierarchy_Alpha;     //20080512: Based on application layer's cmd to set proper hierarchy mode to demodulator
    return SUCCESS;
}

static INT8 f_stv0362_priority(struct nim_device *dev, UINT8*priority)
{
    *priority = stv0362_CurChannelInfo->Priority;
    return SUCCESS;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_PLLDIV_TRL_INC_ioctl_set()

Description:
    This fuction is used for setting some registers for 30 MHZ crystal
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_PLLDIV_TRL_INC_ioctl_set()
static void ST_d0362_PLLDIV_TRL_INC_ioctl_set(struct nim_device *dev, void *InParams)
{
    STTUNER_demod_IOCTL_30MHZ_REG_t *Reg30MHZSet;
    UINT8 trl_ctl[3]={0}, R_trlctl, plldiv;
    UINT8 incderot[2]={0};

    NIM_PRINTF("Enter ST_d0362_PLLDIV_TRL_INC_ioctl_set()...\n");

    Reg30MHZSet = (STTUNER_demod_IOCTL_30MHZ_REG_t *)InParams;

    plldiv = Reg30MHZSet->RPLLDIV;
    ChipSetOneRegister(dev, R0362_PLLNDIV, plldiv); /* PLLNDIV set for 30 MHZ crystal */
    R_trlctl = ChipGetOneRegister(dev, R0362_TRL_CTL);

    trl_ctl[0] = ((Reg30MHZSet->TRLNORMRATELSB) << 7) | (R_trlctl & 0x7f);
    trl_ctl[1] = Reg30MHZSet->TRLNORMRATELO;
    trl_ctl[2] = Reg30MHZSet->TRLNORMRATEHI;
    ChipSetRegisters(dev, R0362_TRL_CTL, trl_ctl, 3); /* TRL registers setting for 30 MHz crystal */

    incderot[0] = Reg30MHZSet->INCDEROT1;
    incderot[1] = Reg30MHZSet->INCDEROT2;
    ChipSetRegisters(dev, R0362_INC_DEROT1, incderot, 2); /* INC_DEROT register set for 30 MHZ crystal */

    NIM_PRINTF("Exit ST_d0362_PLLDIV_TRL_INC_ioctl_set() normally.\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_PLLDIV_TRL_INC_ioctl_get()

Description:
    This fuction is used for setting some registers for 30 MHZ crystal
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_PLLDIV_TRL_INC_ioctl_get()
static void ST_d0362_PLLDIV_TRL_INC_ioctl_get(struct nim_device *dev, void *OutParams)
{
    STTUNER_demod_IOCTL_30MHZ_REG_t   *Reg30MHZGet;

    NIM_PRINTF("Enter ST_d0362_PLLDIV_TRL_INC_ioctl_get()...\n");

    Reg30MHZGet = (STTUNER_demod_IOCTL_30MHZ_REG_t *)OutParams;

    Reg30MHZGet->RPLLDIV = ChipGetOneRegister(dev, R0362_PLLNDIV);
    Reg30MHZGet->TRLNORMRATEHI = ChipGetField(dev, F0362_TRL_NOMRATE_HI);
    Reg30MHZGet->TRLNORMRATELO = ChipGetField(dev, F0362_TRL_NOMRATE_LO);
    Reg30MHZGet->TRLNORMRATELSB = ChipGetField(dev, F0362_TRL_NOMRATE_LSB);
    Reg30MHZGet->INCDEROT1 = ChipGetOneRegister(dev, R0362_INC_DEROT1);
    Reg30MHZGet->INCDEROT2 = ChipGetOneRegister(dev, R0362_INC_DEROT2);

    NIM_PRINTF("Exit ST_d0362_PLLDIV_TRL_INC_ioctl_get() normally.\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_repeateraccess()

Description:
    called when some driver does I/O but with the repeater/passthru
    set to point to this function.
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_repeateraccess()
static void ST_d0362_repeateraccess(struct nim_device *dev, BOOL REPEATER_STATUS)
{
    if (REPEATER_STATUS == TRUE)
    {
        ALI_PRINTF("ST_d0362_repeateraccess(): enable STV0362 I2C Repeater access!\n");
        ChipSetField(dev, F0362_I2CT_ON, 1);
    }
    else
    {
        ALI_PRINTF("ST_d0362_repeateraccess(): disable STV0362 I2C Repeater access!\n");
        ChipSetField(dev, F0362_I2CT_ON, 0);
    }
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetSignalQuality()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetSignalQuality()
static void ST_d0362_GetSignalQuality(struct nim_device *dev, UINT32 *SignalQuality_p, UINT32 *Ber)
{
    /* Read noise estimations for C/N and BER */
    FE_362_GetNoiseEstimator(dev, SignalQuality_p, Ber);

    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetModulation()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetModulation()
static void ST_d0362_GetModulation(struct nim_device *dev, STTUNER_Modulation_t *Modulation)
{
    STTUNER_Modulation_t CurModulation;
    UINT32 Data;

    NIM_PRINTF("Enter ST_d0362_GetModulation()...\n");

    /* Get the modulation type */
    Data = ChipGetField(dev, F0362_TPS_CONST);
    switch(Data)
    {
        case 0:
            CurModulation = STTUNER_MOD_QPSK;
            break;
        case 1:
            CurModulation = STTUNER_MOD_16QAM;
            break;
        case 2:
            CurModulation = STTUNER_MOD_64QAM;
            break;
        default:
            CurModulation = STTUNER_MOD_ALL;
            break;
    }

    *Modulation = CurModulation;

    NIM_PRINTF("Exit ST_d0362_GetModulation() normally\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetFECRates()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetFECRates()
static void ST_d0362_GetFECRates(struct nim_device *dev, STTUNER_FECRate_t *FECRates)
{
    UINT8 Data;
    STTUNER_FECRate_t CurFecRate;

    NIM_PRINTF("Enter ST_d0362_GetFECRates()...\n");

    CurFecRate = 0;
    ChipGetField(dev, F0362_TPS_HPCODE);
    Data = ChipGetField(dev, F0362_TPS_HPCODE);

    switch(Data)
    {
        case 0:
            CurFecRate = STTUNER_FEC_1_2;
            break;
        case 1:
            CurFecRate = STTUNER_FEC_2_3;
            break;
        case 2:
            CurFecRate = STTUNER_FEC_3_4;
            break;
        case 3:
            CurFecRate = STTUNER_FEC_5_6;
            break;
        case 4:
            CurFecRate = STTUNER_FEC_7_8;
            break;
        default:
            ALI_PRINTF("ST_d0362_GetFECRates() ==> Error\n");
            CurFecRate = 0; /* error */
            break;
    }

    *FECRates = CurFecRate;

    NIM_PRINTF("Exit ST_d0362_GetFECRates() normally\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetMode()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetMode()
static void ST_d0362_GetMode(struct nim_device *dev, STTUNER_Mode_t *Mode)
{
    UINT8 Data;
    STTUNER_Mode_t CurMode;

    NIM_PRINTF("Enter ST_d0362_GetMode()...\n");

    /* Get the mode type */
    Data = ChipGetField(dev, F0362_TPS_MODE);
    switch(Data)
    {
        case 0:
            CurMode = STTUNER_MODE_2K;
            break;
        case 1:
            CurMode = STTUNER_MODE_8K;
            break;
        default:
            ALI_PRINTF("ST_d0362_GetMode() ==> Error\n");
            CurMode = 0xff; /* error */
            break;
    }

    *Mode = CurMode;

    NIM_PRINTF("Exit ST_d0362_GetMode() normally\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetGuard()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetGuard()
static void ST_d0362_GetGuard(struct nim_device *dev, STTUNER_Guard_t *Guard)
{
    UINT8 Data;
    STTUNER_Guard_t CurGuard;

    NIM_PRINTF("Enter ST_d0362_GetGuard()...\n");

    CurGuard = 0;
    Data = ChipGetField(dev, F0362_TPS_GUARD);
    switch(Data)
    {
        case 0:
            CurGuard = STTUNER_GUARD_1_32;
            break;
        case 1:
            CurGuard = STTUNER_GUARD_1_16;
            break;
        case 2:
            CurGuard = STTUNER_GUARD_1_8;
            break;
        case 3:
            CurGuard = STTUNER_GUARD_1_4;
            break;
        default:
            ALI_PRINTF("ST_d0362_GetGuard() ==> UNKNOWN\n");
            CurGuard = 0xff; /* error */
            break;
    }

    *Guard = CurGuard;

    NIM_PRINTF("Exit ST_d0362_GetGuard() normally\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_GetTPSCellId()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetTPSCellId()
static FE_362_Error_t ST_d0362_GetTPSCellId(struct nim_device *dev, UINT16 *TPSCellId)
{
    FE_362_Error_t error=FE_362_NO_ERROR;
    UINT8 tps_rcvd[7], FrameNo, TPSCellID_MSB, TPSCellID_LSB;
    UINT8 Frame0_MSB=0, Frame1_MSB=0, Frame2_MSB=0, Frame3_MSB=0;
    UINT8 Frame0_LSB=0, Frame1_LSB=0, Frame2_LSB=0, Frame3_LSB=0;
    BOOL Frame0_CellIDFound=FALSE;
    BOOL Frame1_CellIDFound=FALSE;
    BOOL Frame2_CellIDFound=FALSE;
    BOOL Frame3_CellIDFound=FALSE;
    BOOL All_Frames_OK=FALSE;
    UINT16 Counter=0;

    NIM_PRINTF("Enter ST_d0362_GetTPSCellId()...\n");

    do
    {
        ChipGetRegisters(dev, R0362_TPS_RCVD1, 7, tps_rcvd);

        WAIT_N_MS_362(6);
        FrameNo = tps_rcvd[0] & f_stv0362_Mask_of_RegField(F0362_TPS_FRAME);

        TPSCellID_MSB = tps_rcvd[5];
        TPSCellID_LSB = tps_rcvd[4];

        switch(FrameNo)
        {
            case 0:
                if (Frame0_CellIDFound == FALSE)
                {
                    Frame0_MSB = TPSCellID_MSB;
                    Frame0_LSB = TPSCellID_LSB;
                    Frame0_CellIDFound = TRUE;
                }
                break;
            case 1:
                if (Frame1_CellIDFound == FALSE)
                {
                    Frame1_MSB = TPSCellID_MSB;
                    Frame1_LSB = TPSCellID_LSB;
                    Frame1_CellIDFound = TRUE;
                }
                break;
            case 2:
                if (Frame2_CellIDFound == FALSE)
                {
                    Frame2_MSB = TPSCellID_MSB;
                    Frame2_LSB = TPSCellID_LSB;
                    Frame2_CellIDFound = TRUE;
                }
                break;
            case 3:
                if (Frame3_CellIDFound == FALSE)
                {
                    Frame3_MSB = TPSCellID_MSB;
                    Frame3_LSB = TPSCellID_LSB;
                    Frame3_CellIDFound = TRUE;
                }
                break;
            default:
                break;
        }

        if ((Frame0_CellIDFound == TRUE) &&
            (Frame1_CellIDFound == TRUE) &&
            (Frame2_CellIDFound == TRUE) &&
            (Frame3_CellIDFound == TRUE))
        {
            if ((Frame0_MSB == Frame1_MSB) &&
                (Frame2_MSB == Frame3_MSB) &&
                (Frame0_MSB == Frame2_MSB) &&
                (Frame0_LSB == Frame1_LSB) &&
                (Frame2_LSB == Frame3_LSB) &&
                (Frame0_LSB == Frame2_LSB))
            {
                All_Frames_OK = TRUE;
            }
            else
            {
                Frame0_CellIDFound = FALSE;
                Frame1_CellIDFound = FALSE;
                Frame2_CellIDFound = FALSE;
                Frame3_CellIDFound = FALSE;

                NIM_PRINTF("ST_d0362_GetTPSCellId(): Frames Mismatch.\n");
            }
        }
    } while ((++Counter <= 100) && !(All_Frames_OK));

    if (Counter > 100)
    {
        NIM_PRINTF("ST_d0362_GetTPSCellId(): Timeout, TPS Cell ID not found!\n");
        error = FE_362_BAD_PARAMETER;
    }
    else
    {
        *TPSCellId = TPSCellID_MSB;
        *TPSCellId = (*TPSCellId << 8) | TPSCellID_LSB;

        NIM_PRINTF("ST_d0362_GetTPSCellId(): TPS Cell ID = 0x%x found in %d iterations\n", *TPSCellId, Counter);
    }

    NIM_PRINTF("Exit ST_d0362_GetTPSCellId() normally\n");
    return(error);
}


/* ----------------------------------------------------------------------------
Name: ST_d0362_GetTunerInfo()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_GetTunerInfo()
static void ST_d0362_GetTunerInfo(struct nim_device *dev, STTUNER_TunerInfo_t *TunerInfo_p)
{
    FE_362_Error_t            TunerError=FE_362_NO_ERROR;
    UINT8                     Data;
    UINT32                    CurFrequency, CurSignalQuality, CurBitErrorRate;
    STTUNER_Modulation_t      CurModulation;
    STTUNER_Mode_t            CurMode;
    STTUNER_FECRate_t         CurFECRates;
    STTUNER_Guard_t           CurGuard;
    STTUNER_Spectrum_t        CurSpectrum;
    STTUNER_Hierarchy_Alpha_t CurHierMode;
    INT32                     CurEchoPos;
    INT32 offset=0;
    UINT8 crl_freq1[3];

    NIM_PRINTF("Enter ST_d0362_GetTunerInfo()...\n");

    /* Read noise estimations for C/N and BER */
    FE_362_GetNoiseEstimator(dev, &CurSignalQuality, &CurBitErrorRate);

    Data = ChipGetField(dev, F0362_TPS_CONST);
    switch(Data)
    {
        case 0:  CurModulation = STTUNER_MOD_QPSK;  break;
        case 1:  CurModulation = STTUNER_MOD_16QAM; break;
        case 2:  CurModulation = STTUNER_MOD_64QAM; break;
        default:
            CurModulation = Data;
            SYS_WAIT_MS(5);
            Data = ChipGetField(dev, F0362_TPS_CONST);
            switch(Data)
            {
                case 0:  CurModulation = STTUNER_MOD_QPSK;  break;
                case 1:  CurModulation = STTUNER_MOD_16QAM; break;
                case 2:  CurModulation = STTUNER_MOD_64QAM; break;
                default:
                    CurModulation = Data;
                    TunerError = FE_362_BAD_PARAMETER;
                    break;
            }
            break;
    }

    /* Get the mode */
    Data = ChipGetField(dev, F0362_TPS_MODE);
    switch(Data)
    {
        case 0:  CurMode = STTUNER_MODE_2K; break;
        case 1:  CurMode = STTUNER_MODE_8K; break;
        default:
            CurMode = Data;
            SYS_WAIT_MS(5);
            Data = ChipGetField(dev, F0362_TPS_MODE);
            switch(Data)
            {
                case 0:  CurMode = STTUNER_MODE_2K; break;
                case 1:  CurMode = STTUNER_MODE_8K; break;
                default:
                    CurMode = Data;
                    TunerError = FE_362_BAD_PARAMETER;
                    break;
            }
            break; /* error */
    }

    /* Get the Hierarchical Mode */
    Data = ChipGetField(dev, F0362_TPS_HIERMODE);
    switch(Data)
    {
        case 0:  CurHierMode = STTUNER_HIER_ALPHA_NONE; break;
        case 1:  CurHierMode = STTUNER_HIER_ALPHA_1; break;
        case 2:  CurHierMode = STTUNER_HIER_ALPHA_2; break;
        case 3:  CurHierMode = STTUNER_HIER_ALPHA_4; break;
        default:
            CurHierMode = Data;
            TunerError = FE_362_BAD_PARAMETER;
            break;
    }

     /* Get the FEC Rate */
    if((gSTV0362_SearchResultData.Result).hier == STTUNER_HIER_LOW_PRIO)
    {
        Data = ChipGetField(dev, F0362_TPS_LPCODE);
    }
    else
    {
        Data = ChipGetField(dev, F0362_TPS_HPCODE);
    }

    switch(Data)
    {
        case 0:  CurFECRates = STTUNER_FEC_1_2; break;
        case 1:  CurFECRates = STTUNER_FEC_2_3; break;
        case 2:  CurFECRates = STTUNER_FEC_3_4; break;
        case 3:  CurFECRates = STTUNER_FEC_5_6; break;
        case 4:  CurFECRates = STTUNER_FEC_7_8; break;
        default:
            CurFECRates = Data;
            TunerError = FE_362_BAD_PARAMETER;
            break;
    }

    /* Get the Guard time */
    Data = ChipGetField(dev, F0362_TPS_GUARD);
    switch(Data)
    {
        case 0:  CurGuard = STTUNER_GUARD_1_32; break;
        case 1:  CurGuard = STTUNER_GUARD_1_16; break;
        case 2:  CurGuard = STTUNER_GUARD_1_8;  break;
        case 3:  CurGuard = STTUNER_GUARD_1_4;  break;
        default:
            CurGuard = Data;
            TunerError = FE_362_BAD_PARAMETER;
            break;
    }

    /* Get the spectrum.Register is not read as spectrum inversion code added*/
     CurSpectrum = gSTV0362_SearchResultData.ResultSpectrum;

    /* Get the correct frequency */
    CurFrequency = TunerInfo_p->Frequency;
    /********Frequency offset calculation done here*******************/

    ChipSetField(dev, F0362_FREEZE, 1);
    ChipGetRegisters(dev, R0362_CRL_FREQ1, 3, crl_freq1);
    ChipSetField(dev, F0362_FREEZE, 0);

    offset = gSTV0362_SearchResultData.Result.offset;

    TunerInfo_p->ScanInfo.FreqOff = gSTV0362_SearchResultData.Result.offset_type;
    TunerInfo_p->ScanInfo.ResidualOffset = offset;
    /************************************************/

    /* Get the echo position */
    CurEchoPos = ChipGetField(dev, F0362_LONG_ECHO);

    TunerInfo_p->FrequencyFound      = CurFrequency;
    TunerInfo_p->SignalQuality       = CurSignalQuality;
    TunerInfo_p->BitErrorRate        = CurBitErrorRate;
    TunerInfo_p->ScanInfo.Modulation = CurModulation;
    TunerInfo_p->ScanInfo.Mode       = CurMode;
    TunerInfo_p->ScanInfo.FECRates   = CurFECRates;
    TunerInfo_p->ScanInfo.Guard      = CurGuard;
    TunerInfo_p->ScanInfo.Spectrum   = CurSpectrum;
    TunerInfo_p->ScanInfo.EchoPos    = CurEchoPos;
    TunerInfo_p->Hierarchy_Alpha     = CurHierMode; /* added for Hierarchical modulation */
    TunerInfo_p->ScanInfo.Hierarchy  = (gSTV0362_SearchResultData.Result).hier; /*added for Hierarchical modulation */

    NIM_PRINTF("Exit ST_d0362_GetTunerInfo() normally\n");
    return;
}

/* ----------------------------------------------------------------------------
Name: ST_d0362_ScanFrequency()
---------------------------------------------------------------------------- */
//copied and tuned from ST code: src\drivers\ter\demod\stv0362\d0362.c demod_d0362_ScanFrequency()
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
                                         INT32   EchoPos)
{
    FE_362_Error_t error = FE_362_NO_ERROR;
    FE_362_SearchParams_t  Search;
    FE_362_SearchResult_t  Result;

    Search.Frequency = (UINT32)InitialFrequency;
    Search.Mode      = (STTUNER_Mode_t)Mode;
    Search.Guard     = (STTUNER_Guard_t)Guard;
    Search.Force     = (STTUNER_Force_t)Force;
    Search.Inv       = (STTUNER_Spectrum_t)Spectrum;
    Search.Offset    = (STTUNER_FreqOff_t)FreqOff;
    Search.ChannelBW = ChannelBW;
    Search.EchoPos   = EchoPos;
    Search.Hierarchy = (STTUNER_Hierarchy_t)Hierarchy;
    Search.IF_IQ_Mode = IF_IQ_Mode;

    NIM_PRINTF("Enter ST_d0362_ScanFrequency()..., freq = %d, bandwidth = %d\n", Search.Frequency, Search.ChannelBW);

    NIM_PRINTF("ST_d0362_ScanFrequency(): => FE_362_LookFor()...\n");

    error = FE_362_LookFor(dev, &Search, &Result);

    if(error == FE_362_BAD_PARAMETER)
    {
        NIM_PRINTF("ST_d0362_ScanFrequency(): failed, bad parameter(s)! FE_362_LookFor() == FE_362_BAD_PARAMETER\n");
        NIM_PRINTF("Exit ST_d0362_ScanFrequency(), return FE_362_BAD_PARAMETER\n");
        return error;
    }
    else if (error == FE_362_SEARCH_FAILED)
    {
        NIM_PRINTF("ST_d0362_ScanFrequency(): failed! FE_362_LookFor() == FE_362_SEARCH_FAILED\n");
        NIM_PRINTF("Exit ST_d0362_ScanFrequency(), return FE_362_SEARCH_FAILED\n");
        return error;
    }

    *ScanSuccess = Result.Locked;
    if (*ScanSuccess == TRUE)
    {
        //*NewFrequency = Result.Frequency;

        /* Update Hierarchical parameters into strucuture */
        (gSTV0362_SearchResultData.Result).hier = Result.hier;
        (gSTV0362_SearchResultData.Result).Hierarchy_Alpha = Result.Hierarchy_Alpha; //20080512: Based on application layer's cmd to set proper hierarchy mode to demodulator
        (gSTV0362_SearchResultData.Result).Hprate = Result.Hprate;
        (gSTV0362_SearchResultData.Result).Lprate = Result.Lprate;
        (gSTV0362_SearchResultData.Result).offset = Result.offset;
        (gSTV0362_SearchResultData.Result).offset_type = Result.offset_type;
        (gSTV0362_SearchResultData.Result).spectrum_direction = Result.spectrum_direction;
    }

    NIM_PRINTF("Exit ST_d0362_ScanFrequency(), return FE_362_NO_ERROR\n");
    return error;
}


