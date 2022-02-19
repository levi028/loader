//The sample code use method 1 of gain fine-tune function.
/*****************************************************************************
*    Tuner sample code
*
*    History:
*          Date           Athor          Version    Reason
*      ============    =============    =========    =================
*        2013/1/7            David           Ver 0.1
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_sp7006.h"

//#define NIM_PRINTF libc_printf
#define NIM_PRINTF(...)

// Tuner crystal CLK Freqency
static UINT32 sp7006_tuner_cnt = 0;
static struct QPSK_TUNER_CONFIG_EXT * sp7006_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static unsigned char tuner_initial[MAX_TUNER_SUPPORT_NUM] = {0,0};

// I2C write function (register start address, register array pointer, register length)
static int Tuner_I2C_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16] ={0};
    UINT32 rd = 0;
	int i2c_result = -1;
    struct QPSK_TUNER_CONFIG_EXT * sp7006_ptr = NULL;

    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;

    sp7006_ptr = sp7006_dev_id[tuner_id];
    data[0] = reg_start;

    while((rd+15)<length)
    {
        MEMCPY(&data[1], &buff[rd], 15);
        i2c_result = i2c_write(sp7006_ptr->i2c_type_id, sp7006_ptr->c_tuner_base_addr, data, 16);
        rd+=15;
        data[0] += 15;
        if(SUCCESS != i2c_result)
            return i2c_result;
    }
    MEMCPY(&data[1], &buff[rd], length-rd);
    i2c_result = i2c_write(sp7006_ptr->i2c_type_id, sp7006_ptr->c_tuner_base_addr, data, length-rd+1);

    return i2c_result;
}

static int Tuner_I2C_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16]={0};
    UINT32 rd = 0;
	int i2c_result = -1;
    struct QPSK_TUNER_CONFIG_EXT * sp7006_ptr = NULL;

    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;

    sp7006_ptr = sp7006_dev_id[tuner_id];
    data[0] = reg_start;

    while((rd+15)<length)
    {
        i2c_result = i2c_write_read(sp7006_ptr->i2c_type_id, sp7006_ptr->c_tuner_base_addr, data, 1, 15);
        MEMCPY(&buff[rd], &data[0], 15);
        rd+=15;
        data[0] += 15;
        if(SUCCESS != i2c_result)
            return i2c_result;
    }
    i2c_result = i2c_write_read(sp7006_ptr->i2c_type_id, sp7006_ptr->c_tuner_base_addr, data, 1, length-rd);
    MEMCPY(&buff[rd], &data[0], 15);
    return i2c_result;
}
// time delay function ( minisecond )
static void Time_DELAY_MS (unsigned int ms)
{
    if(ms)
    osal_task_sleep(ms);
}

INT32 nim_sp7006_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//INT32 result = -1;
    struct QPSK_TUNER_CONFIG_EXT * sp7006_ptr = NULL;

	if ((ptrTuner_Config == NULL)||(sp7006_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    sp7006_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!sp7006_ptr)
          return ERR_FAILUE;
    MEMCPY(sp7006_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    sp7006_dev_id[sp7006_tuner_cnt] = sp7006_ptr;
    *tuner_id = sp7006_tuner_cnt;
    sp7006_tuner_cnt++;

    return SUCCESS;
}

/*======================================================
 * X-TAL frequency unit : kHz
 *=====================================================*/
//#define RTV_SRC_CLK_FREQ_KHz         16000
#define RTV_SRC_CLK_FREQ_KHz          27000

#define PLLF_MAX(a,b) ((a)>(b) ? 1:0)

#if (RTV_SRC_CLK_FREQ_KHz ==16000)
    #define PLLF_BOUNDARY_VALUE  (0x4000)
    #define PLLF_CHK_VALUE (0x7FFF)
    #define PLLF_MASK_VALUE (0xFFFF8000)
#elif (RTV_SRC_CLK_FREQ_KHz ==18000)
    #define PLLF_BOUNDARY_VALUE  (0x3C3C)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==20000)
    #define PLLF_BOUNDARY_VALUE  (0x3333)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==24000)
    #define PLLF_BOUNDARY_VALUE  (0x2AAA)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==26000)
    #define PLLF_BOUNDARY_VALUE  (0x2762)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==27000)
    #define PLLF_BOUNDARY_VALUE  (0x25ED)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==32000)
    #define PLLF_BOUNDARY_VALUE  (0x2000)
    #define PLLF_CHK_VALUE (0x3FFF)
    #define PLLF_MASK_VALUE (0xFFFFC000)
#elif (RTV_SRC_CLK_FREQ_KHz ==36000)
    #define PLLF_BOUNDARY_VALUE  (0x1C71)
    #define PLLF_CHK_VALUE (0x1FFF)
    #define PLLF_MASK_VALUE (0xFFFFE000)
#else
        NIM_PRINTF("///---#error  Selected X-TAL Frequency is not Supported\n");
#endif

/*==================================================
 * BBAGC Polarity selection : Normal or Inverse
*==================================================*/
#define RTV_AGC_POL_INVERSE
//#define RTV_AGC_POL_NORMAL



typedef struct
{
      unsigned char    bReg;
      unsigned char    bVal;
}RTV_REG_INIT_INFO;

typedef struct
{
    unsigned char    CB15_I2C;
    unsigned char    CB24_I2C;
    unsigned char    CB3_I2C;
    unsigned char    RSB_I2C;
}RTV_SET_LPF_INFO;

#define RTV_SUCCESS                                    0
#define RTV_INVAILD_LPF_BW_TYPE                -1
#define RTV_INVAILD_FREQUENCY_RANGE            -2
#define RTV_INVAILD_RF_BAND                                  -3


static const RTV_REG_INIT_INFO t_MTV600_INIT[] =
{
    {0x20,    0x0b},
    {0x21,    0xb8},
    {0x22,    0x09},
    {0x26,    0x81},
    {0x27,    0x20},
    {0x28,    0xe2},
    {0x29,    0xd4},
    {0x2a,    0x02},
    {0x2b,    0x0c},
    {0x2c,    0x00},
    {0x2d,    0x2f},
    //{0x2e,    0x10},
       {0x2e,    0x19},
    {0x2f,    0x80},
    {0x30,    0x10},
    {0x31,    0x8c},
    {0x32,    0x17},
    {0x33,    0x34},
    {0x34,    0x30},
    {0x35,    0x4a},
    {0x36,    0x81},
    {0x37,    0x92},
    {0x38,    0x90},
    {0x39,    0x18},
    {0x3a,    0x00},
    {0x3b,    0x44},
    {0x3c,    0x92},
    {0x3d,    0x85},
    {0x3e,    0x00},
    {0x3f,    0x00},
    {0x40,    0xff},
    {0x41,    0x57},
    {0x42,    0x9a},
    {0x43,    0x04}
};


static const RTV_SET_LPF_INFO g_atLpfBwSetValue[]=
{
    {   0x60,     0x58,     0x50,     0x8 },    //4
    {   0x52,     0x34,     0x40,     0x8 },      //5
    {   0x70,     0x80,     0x60,     0x4 },      //6
    {   0x60,     0x60,     0x58,     0x4 },      //7
    {   0x60,     0x58,     0x38,     0x4 },      //8
    {   0x58,     0x48,     0x30,     0x4 },      //9
    {      0x52,     0x34,     0x24,     0x4 },      //10
    {      0x70,     0x88,     0x68,     0x2 },      //11
    {      0x70,     0x80,     0x48,     0x2 },      //12
    {      0x60,     0x60,     0x40,     0x2 },      //13
    {      0x60,     0x58,     0x40,     0x2 },      //14
    {      0x58,     0x50,     0x40,     0x2 },      //15
    {      0x58,     0x50,     0x30,     0x2 },      //16
    {      0x52,     0x48,     0x28,     0x2 },      //17
    {      0x48,     0x40,     0x28,     0x2 },      //18
    {      0x40,     0x38,     0x28,     0x2 },      //19
    {      0x40,     0x34,     0x20,     0x2 },      //20
    {      0x70,     0x88,     0x58,     0x1 },      //21
    {      0x70,     0x80,     0x50,     0x1 },      //22
    {      0x68,     0x70,     0x50,     0x1 },      //23
    {      0x68,     0x70,     0x48,     0x1 },      //24
    {      0x68,     0x70,     0x38,     0x1 },      //25
    {      0x60,     0x60,     0x40,     0x1 },      //26
    {      0x60,     0x58,     0x40,     0x1 },      //27
    {      0x58,     0x50,     0x40,     0x1 },      //28
    {      0x58,     0x50,     0x38,     0x1 },      //29
    {      0x58,     0x50,     0x34,     0x1 },      //30
    {      0x52,     0x48,     0x38,     0x1 },      //31
    {      0x48,     0x40,     0x38,     0x1 },      //32
    {      0x40,     0x38,     0x40,     0x1 },      //33
    {      0x40,     0x34,     0x40,     0x1 },      //34
    {      0x40,     0x30,     0x3c,    0x1 },      //35
    {      0x38,     0x30,     0x3c,    0x1 },      //36
    {      0x30,     0x30,     0x34,     0x1 },      //37
    {      0x30,     0x2c,    0x34,     0x1 },      //38
    {      0x2c,    0x2c,    0x30,     0x1 },      //39
    {      0x28,     0x28,     0x24,     0x1 }      //40
};

volatile unsigned char g_aeLpfBwType;

/***********************************************************************
* unsigned int rtvRF_ConfigureLpfBwType (unsigned int tuner_id, unsigned int bb_sym, unsigned char uRollOff)
*  Arguments:
*  Parameter1: unsigned int bb_sym            : Baseband Symbol Rate (KHz)
*  Parameter2: unsigned char uRollOff            : rolloff is 0.35
*  Return Value: unsigned int            : Result
***********************************************************************/
static INT rtvRF_ConfigureLpfBwType(UINT32 tuner_id,unsigned int  bb_sym, unsigned char uRollOff)
{

    unsigned char uReg=0;
    unsigned char uReg26=0;  
    unsigned char temp=0;
    unsigned char uCalculatedLPF=0;
    unsigned int dwSymbolrateReconfig=0;
    int result = 0;

    if(tuner_id>=MAX_TUNER_SUPPORT_NUM)
        return RET_FAILURE;
    dwSymbolrateReconfig = bb_sym * (100 + uRollOff);
       dwSymbolrateReconfig = (dwSymbolrateReconfig >> 1) * 120;

       uCalculatedLPF = (dwSymbolrateReconfig/10000000) + 1;   //ksps(1000) * rolloff(100) * 120(100) = 10000000 is to be MHz of LPF Setting. +1 means ceil().

      if(uCalculatedLPF < 4 ) uCalculatedLPF = 4;
    else if( uCalculatedLPF > 40)  uCalculatedLPF = 40;
    else uCalculatedLPF = uCalculatedLPF;

    result=Tuner_I2C_read((UINT32)tuner_id,0x26,&uReg26,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp = ((g_atLpfBwSetValue[uCalculatedLPF-4].CB15_I2C << 1) | 0x01);
    result =Tuner_I2C_write((UINT32)tuner_id,0x32,&temp,1);  //EN_EXT_LPFCON = 1
    if(result!=SUCCESS)
        ali_trace(&result);
    temp =g_atLpfBwSetValue[uCalculatedLPF-4].CB24_I2C;
    result =Tuner_I2C_write((UINT32)tuner_id,0x33,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);
    temp = g_atLpfBwSetValue[uCalculatedLPF-4].CB3_I2C;
    result = Tuner_I2C_write((UINT32)tuner_id,0x34,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);

    result = Tuner_I2C_read((UINT32)tuner_id,0x35,&uReg,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp = ((uReg)|(unsigned char)(0xF0)) & (unsigned char)((~(0xF0))|((g_atLpfBwSetValue[uCalculatedLPF-4].RSB_I2C << 4)));
    result = Tuner_I2C_write((UINT32)tuner_id,0x35,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp =(uReg26 | 0x40) ;
    result = Tuner_I2C_write((UINT32)tuner_id,0x26,&temp,1);  // LPFBW_I2C<0> = 1
    if(result!=SUCCESS)
        ali_trace(&result);
    temp =(uReg26& 0xBF) ;
    result = Tuner_I2C_write((UINT32)tuner_id,0x26,&temp,1);  // LPFBW_I2C<0> = 0
    if(result!=SUCCESS)
        ali_trace(&result);


    g_aeLpfBwType = uCalculatedLPF;

    return RTV_SUCCESS;
}

/***********************************************************************
* unsigned int rtvRF_ChangeLpfBwType (unsigned int tuner_id, unsigned int bb_sym, unsigned char uRollOff)
*  Arguments:
*  Parameter1: unsigned int bb_sym            : Baseband Symbol Rate (KHz)
*  Parameter2: unsigned char uRollOff            : rolloff is 0.35
*  Return Value: unsigned int            : Result
***********************************************************************/
static INT rtvRF_ChangeLpfBwType(UINT32 tuner_id,unsigned int bb_sym, unsigned char uRollOff)
{
       unsigned char uReg=0;
    unsigned char uReg26=0;
    unsigned char temp=0;
    unsigned char uCalculatedLPF=0;
    unsigned int dwSymbolrateReconfig=0;
    int result = 0;

    if(tuner_id>=MAX_TUNER_SUPPORT_NUM)
        return RET_FAILURE;
    dwSymbolrateReconfig = bb_sym * (100 + uRollOff);
       dwSymbolrateReconfig = (dwSymbolrateReconfig >> 1) * 120;


       uCalculatedLPF = (dwSymbolrateReconfig/10000000) + 1;   //ksps(1000) * rolloff(100) * 120(100) = 10000000 is to be MHz of LPF Setting.// +1 means ceil().

      if(uCalculatedLPF < 4 ) uCalculatedLPF = 4;
    else if( uCalculatedLPF > 40)  uCalculatedLPF = 40;
    else uCalculatedLPF = uCalculatedLPF;

    if(g_aeLpfBwType == uCalculatedLPF)
       return RTV_SUCCESS;

    result = Tuner_I2C_read((UINT32)tuner_id,0x26,&uReg26,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp = ((g_atLpfBwSetValue[uCalculatedLPF-4].CB15_I2C << 1) | 0x01);
    result = Tuner_I2C_write((UINT32)tuner_id,0x32,&temp,1);  //EN_EXT_LPFCON = 1
    if(result!=SUCCESS)
        ali_trace(&result);
    temp =g_atLpfBwSetValue[uCalculatedLPF-4].CB24_I2C;
    result = Tuner_I2C_write((UINT32)tuner_id,0x33,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);
    temp = g_atLpfBwSetValue[uCalculatedLPF-4].CB3_I2C;
    result = Tuner_I2C_write((UINT32)tuner_id,0x34, &temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);
    result = Tuner_I2C_read((UINT32)tuner_id,0x35,&uReg,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp = ((uReg)|(unsigned char)(0xF0)) &(unsigned char) ((~(0xF0))|((g_atLpfBwSetValue[uCalculatedLPF-4].RSB_I2C << 4)));
    result = Tuner_I2C_write((UINT32)tuner_id,0x35,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp =(uReg26 | 0x40);
    result = Tuner_I2C_write((UINT32)tuner_id,0x26,&temp,1);  // LPFBW_I2C<0> = 1
    if(result!=SUCCESS)
        ali_trace(&result);
    temp =(uReg26 & 0xBF);
    result = Tuner_I2C_write((UINT32)tuner_id,0x26,&temp,1);  // LPFBW_I2C<0> = 0
    if(result!=SUCCESS)
        ali_trace(&result);

    g_aeLpfBwType = uCalculatedLPF;


    return RTV_SUCCESS;
}

/***********************************************************************
* unsigned int rtvRF_SetVcoBand (unsigned int tuner_id, unsigned int dwLoFreq)
*  Arguments:
*  Parameter1: unsigned int dwLoFreq        : Channel frequency (kHz)
*  Return Value: unsigned int                   : Result
***********************************************************************/
static INT rtvRF_SetVcoBand(UINT32 tuner_id, unsigned int  dwLoFreq)
{
    INT nRet = -1;
	unsigned char nVcoBandI2C = 0;
       unsigned char uReg=0;
        unsigned char temp=0;
    int result = 0;

    if(tuner_id>=MAX_TUNER_SUPPORT_NUM)
        return RET_FAILURE;
    if((dwLoFreq > 900000)&&(dwLoFreq < 1175000))
    {
         nVcoBandI2C = 2;
    }
    else if((dwLoFreq >= 1175000)&&(dwLoFreq < 1750000))
    {
         nVcoBandI2C = 1;
    }
    else if((dwLoFreq >= 1750000)&&(dwLoFreq < 2200000))
    {
                nVcoBandI2C = 0;
    }
    else
        return  RTV_INVAILD_FREQUENCY_RANGE;

    result = Tuner_I2C_read(tuner_id,0x23,&uReg,1);
    if(result!=SUCCESS)
        ali_trace(&result);
       temp = ((uReg)|(unsigned char)(0xC0))& (unsigned char)((~(0xC0))|(nVcoBandI2C << 6));
    result = Tuner_I2C_write(tuner_id,0x23,&temp,1);
    if(result!=SUCCESS)
        ali_trace(&result);

    nRet =  RTV_SUCCESS;

    return nRet;
}

/***********************************************************************
* unsigned int nim_sp7006_control (unsigned int tuner_id, unsigned int channel_freq, unsigned int bb_sym)
*  Arguments:
*  Parameter1: unsigned int channel_freq        : Channel frequency (MHz)
*  Parameter2: unsigned int bb_sym            : Baseband Symbol Rate (KHz)
*  Return Value: unsigned int            : Result
***********************************************************************/
INT32 nim_sp7006_control(UINT32 tuner_id, UINT32 channel_freq, UINT32 bb_sym)
{
       unsigned char uRollOff=35;        // rolloff is 0.35
    unsigned int nNumTblEntry=0;
        unsigned char pllf_mul=0, r_div=4;
	unsigned int dwPLLN = 0,dwPLLF = 0, dwPLLNF = 0;
    unsigned int dwPLLFRefkVal=0,dwPLLFCkeckVal=0,dwPLLFMaskVal=0;
       unsigned int dwChFreqKHz=channel_freq*1000;
       unsigned char uReg=0;
        unsigned char temp=0;
      // unsigned short tuner_crystal = 0;
       //unsigned char data[50] = {0};
       unsigned char  value = 0;  
      int result = 0;

    //struct QPSK_TUNER_CONFIG_EXT * sp7006_ptr = NULL;

	if((tuner_id >= sp7006_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;

      if( bb_sym == 0)   //autoscan mode, set tuner LPF bandwidth to 45MHz
             bb_sym = 45000;

    //   else if(0<bb_sym<6500)
      else if((bb_sym>0)&& (bb_sym<6500))
            bb_sym = bb_sym+3000;


    //sp7006_ptr = sp7006_dev_id[tuner_id];

    // At Power ON, tuner_initial = 0.
    if (tuner_initial[tuner_id] == 0)
    {
    const RTV_REG_INIT_INFO *ptInitTbl = NULL;
    ptInitTbl = t_MTV600_INIT;
    nNumTblEntry = sizeof(t_MTV600_INIT) / sizeof(RTV_REG_INIT_INFO);
    do
    {
        value = (unsigned char ) (ptInitTbl->bVal);
            result = Tuner_I2C_write(tuner_id, ptInitTbl->bReg, &value,1);
            if(result!=SUCCESS)
                ali_trace(&result);
        ptInitTbl++;
    } while( --nNumTblEntry );
#if  defined(RTV_AGC_POL_INVERSE)
       temp=0x40;
           result = Tuner_I2C_write(tuner_id, 0x23,&temp,1);
           if(result!=SUCCESS)
               ali_trace(&result);
#elif defined(RTV_AGC_POL_NORMAL)
       temp=0x42;
           result = Tuner_I2C_write(tuner_id, 0x23,&temp,1);
           if(result!=SUCCESS)
               ali_trace(&result);
#else
        NIM_PRINTF("///---#error  AGC Polarity type is not defined\n");
#endif
	 rtvRF_ConfigureLpfBwType(tuner_id,bb_sym,uRollOff);                           		
     }

       // After power on initial
    tuner_initial[tuner_id] = 1;

     // Time delay 4ms
    Time_DELAY_MS(4);

    // Set Frequency
      if(rtvRF_ChangeLpfBwType(tuner_id,bb_sym,uRollOff)!= RTV_SUCCESS)
        return RTV_INVAILD_LPF_BW_TYPE;

    if( rtvRF_SetVcoBand(tuner_id,dwChFreqKHz) != RTV_SUCCESS)
        return RTV_INVAILD_FREQUENCY_RANGE;

    dwPLLN = ( dwChFreqKHz / RTV_SRC_CLK_FREQ_KHz  );
    dwPLLF = dwChFreqKHz -(dwPLLN * RTV_SRC_CLK_FREQ_KHz);
    if (RTV_SRC_CLK_FREQ_KHz==13000 || RTV_SRC_CLK_FREQ_KHz==27000)
    {
        pllf_mul=1;
        r_div=3;
    }

    if(dwPLLN < 40)
    {
          result = Tuner_I2C_read(tuner_id,0x22,&uReg,1);
          if(result!=SUCCESS)
               ali_trace(&result);
             temp = ((uReg)|(unsigned char)(0x01)) & (unsigned char)((~(0x01))|(0x00));
          result = Tuner_I2C_write(tuner_id,0x22,&temp,1);
          if(result!=SUCCESS)
               ali_trace(&result);
          result = Tuner_I2C_read(tuner_id,0x26,&uReg,1);
          if(result!=SUCCESS)
               ali_trace(&result);
             temp = ((uReg)|(unsigned char)(0x0E)) &(unsigned char) ((~(0x0E))|(0x02));
          result = Tuner_I2C_write(tuner_id,0x26,&temp,1);
          if(result!=SUCCESS)
               ali_trace(&result);

        dwPLLNF = ((dwPLLN << 20 ) + (  ((dwPLLF <<16) / (RTV_SRC_CLK_FREQ_KHz>>r_div))  << pllf_mul));
        dwPLLFRefkVal = PLLF_BOUNDARY_VALUE << 1;
        dwPLLFCkeckVal= (PLLF_CHK_VALUE << 1 ) + 1;
        dwPLLFMaskVal= (PLLF_MASK_VALUE  << 1 );
    }
    else
    {
          result = Tuner_I2C_read(tuner_id,0x22,&uReg,1);
          if(result!=SUCCESS)
               ali_trace(&result);
             temp = ((uReg)|(unsigned char)(0x01)) &(unsigned char) ((~(0x01))|(0x01));
          result = Tuner_I2C_write(tuner_id,0x22,&temp,1);
          if(result!=SUCCESS)
               ali_trace(&result);
          result = Tuner_I2C_read(tuner_id,0x26,&uReg,1);
          if(result!=SUCCESS)
               ali_trace(&result);
             temp = ((uReg)|(unsigned char)(0x0E)) & (unsigned char)((~(0x0E))|(0x00));
          result = Tuner_I2C_write(tuner_id,0x26,&temp,1);
          if(result!=SUCCESS)
               ali_trace(&result);

           dwPLLNF = ((dwPLLN << 20 ) + (  ((dwPLLF <<16) / (RTV_SRC_CLK_FREQ_KHz>>r_div))  << pllf_mul)) >> 1;
        dwPLLFRefkVal = PLLF_BOUNDARY_VALUE;
        dwPLLFCkeckVal= PLLF_CHK_VALUE;
        dwPLLFMaskVal= PLLF_MASK_VALUE;
    }

     if(PLLF_MAX(dwPLLFRefkVal,(dwPLLNF & dwPLLFCkeckVal )))
          dwPLLNF = (dwPLLNF & dwPLLFMaskVal) + dwPLLFRefkVal;

     /* Important :  Do not modify the writing sequence */
           temp = ((dwPLLNF>>7) & 0xFF);
        result = Tuner_I2C_write(tuner_id,0x24,&temp,1);
        if(result!=SUCCESS)
            ali_trace(&result);
           temp = (((dwPLLNF & 0x7F)<<1) | ((dwPLLNF>>29) & 0x01));
        result = Tuner_I2C_write(tuner_id,0x25,&temp,1);
        if(result!=SUCCESS)
            ali_trace(&result);

        result =Tuner_I2C_read(tuner_id,0x31,&uReg,1);
        if(result!=SUCCESS)
            ali_trace(&result);
           temp = ((uReg)|(unsigned char)(0x01)) &(unsigned char) ((~(0x01))|((dwPLLNF>>28) & 0x01));
        result = Tuner_I2C_write(tuner_id,0x31,&temp,1);
        if(result!=SUCCESS)
            ali_trace(&result);
        result = Tuner_I2C_read(tuner_id,0x20,&uReg,1);
        if(result!=SUCCESS)
            ali_trace(&result);
           temp = ((uReg)|(unsigned char)(0x1F)) & (unsigned char)((~(0x1F))|((dwPLLNF>>23) & 0x1F));
        result = Tuner_I2C_write(tuner_id,0x20,&temp,1);
        if(result!=SUCCESS)
            ali_trace(&result);

    /*Address 0x21 must be written as last of PLL value setting sequence*/
           temp =(dwPLLNF>>15) & 0xFF;
        result = Tuner_I2C_write(tuner_id,0x21,&temp,1);
        if(result!=SUCCESS)
            ali_trace(&result);
          Time_DELAY_MS(1);

    return RTV_SUCCESS;
}

INT32 nim_sp7006_status(UINT32 tuner_id, UINT8 *lock)
{
    if(lock == NULL)
        return RET_FAILURE;
	if ((tuner_id>=sp7006_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }

    *lock = 1;
    return SUCCESS;
}

