/********************************************
9FT225-70
********************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_cdt_9VM135_40.h"

#if ((SYS_TUN_MODULE == CDT_9VM135_40) || (SYS_TUN_MODULE == ANY_TUNER))

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;


static INT32 Tuner_I2C_Write(UINT32 tuner_id, UINT8* pArray, UINT32 count)
{
    INT32 status = ERR_FAILUE;
    struct COFDM_TUNER_CONFIG_EXT *p_tuner_config;
    p_tuner_config = tuner_dev_cfg[tuner_id];
    status = i2c_write(p_tuner_config->i2c_type_id, p_tuner_config->c_tuner_base_addr, pArray, count);
    return status;
}

#if 0
INT32 Tuner_I2C_Read(UINT32 tuner_id, UINT8 Reg, UINT8* pArray, UINT32 count)
{
    INT32 status = ERR_FAILUE;
    struct COFDM_TUNER_CONFIG_EXT *p_tuner_config;

    UINT8 Read_Cmd[2];
    Read_Cmd[0] = 0xFB;
    Read_Cmd[1] = Reg;

    p_tuner_config = tuner_dev_cfg[tuner_id];
    status = i2c_write(p_tuner_config->i2c_type_id, p_tuner_config->c_tuner_base_addr, Read_Cmd, 2);
    status = i2c_read(p_tuner_config->i2c_type_id, p_tuner_config->c_tuner_base_addr, pArray, count);
    return status;
}
#endif

static INT32 CDT_DTV4X_SetTuner( UINT32 tuner_id, UINT32 dwFreq, UINT8 ucBw )
{
    // TD1611
    // IF Frequency: 36.16Mhz
    //
    // 60uS delay
    //ADDR , FREQ_HIGH, FREQ_LOW,CB1,CB2.
    // 60uS delay
       UINT8    CB0, CB1, CB2;
       INT32   dwError = SUCCESS;
       UINT8    Buf[5];
       UINT8    ucfreq[2];
       UINT32    wFrequency;
       UINT8    ucDividerRatio = 6;
       //UINT8    ucDividerRatio = 1000000/FREQSTEP;//50KHz

       UINT8 testi;
        //printf ("dwFreq is %d\n", dwFreq);
       // Use critical section to protect Tuner frequency setting
//       User_enterCriticalSection(NULL);

       CB0 = 0xb4;//0xC4;//CB0 = 0xC8;
       CB1 = 0x10;

       wFrequency = (UINT32)(((dwFreq + TUNER_IF_FREQ) * ucDividerRatio) / 1000);
        ucfreq[1] = (UINT8)((wFrequency >> 8) & 0x00FF);
        ucfreq[0] = (UINT8)(wFrequency & 0x00FF);

        //P3 P2 P1
        if ( dwFreq < 164000 ) //( dwFreq <= 174000 ) // VHF low
        {
            CB1 |= 0x01;
        }
        else if ( dwFreq  < 450000 )//( dwFreq  <= 470000 ) // VHF high
        {
            CB1 |= 0x02;
        }
        else  // UHF
        {
            CB1 |= 0x04;
        }
       //Charge Pump  CP2 CP1 CP0
       if((dwFreq>=51000 && dwFreq<79000)
            ||(dwFreq>=157000 && dwFreq<220000))
       {
              CB0 &= 0x87; //cp set 0
              CB0 |= 0x30; //cp set 1
       }
       else if((dwFreq>=79000 && dwFreq<128000)
            ||(dwFreq>=220000 && dwFreq<370000)
            //||(dwFreq>=442000 && dwFreq<498000)
            )
       {
              CB0 &= 0x87;//
              CB0 |= 0x38;
       }
       else if(
                   (dwFreq>=128000 && dwFreq<142000)
            ||(dwFreq>=370000 && dwFreq<418000)
            ||(dwFreq>=442000 && dwFreq<802000)//||(dwFreq>=498000 && dwFreq<746000)
            )
       {
              CB0 &= 0x87;//
              CB0 |= 0x70;
       }
       else if((dwFreq>=142000 && dwFreq<157000)
            ||(dwFreq>=418000 && dwFreq<442000)
            ||(dwFreq>=802000 && dwFreq<=862000))
       {
              CB0 &= 0x87;//
              CB0 |= 0x78;
       }
       if((dwFreq>=474000 && dwFreq<= 612000))//for YUDAFU
       {
              CB0 &= 0x87;//
              CB0 |= 0x38;
       }

       Buf[0]  = ucfreq[1];
       Buf[1]  = ucfreq[0];
       Buf[2]  = CB0;
       Buf[3]  = CB1;
       Buf[4]  = 0x40;

//       for (testi = 0; testi < 5; testi++)
//          printf("Tx Buf[%d] is 0x%x\n", testi, Buf[testi]);

       dwError = Tuner_I2C_Write(tuner_id, &Buf[0],4);

      // Release ownership of the critical section
//      User_leaveCriticalSection(NULL);
      return (dwError);
}


#if 0
//**********************************************************************
// Set Tuner to desired frequency
//
//**********************************************************************
INT32 SetTuner(INT32 dwFreq/*KHz*/,  UINT8  ucBw /*MHz*/, UINT32 dwIF)
{
    INT32   dwError = Error_NO_ERROR;
    User_enterCriticalSection(NULL);
    //printf ("dwFreq is %d\n", dwFreq);
    //printf ("ucBw is %d\n", ucBw);
//    printf("Cdt SetTuner\n");
    if ((dwError = CDT_DTV4X_SetTuner(
                                        TUNER_DEV_ADR,
                                        dwFreq,
                                        ucBw)))
    {
//        printf("dwError is 0x%x\n", dwError);
//        printf("#### %s Fail\n\n", __FUNCTION__);
    }

    // Release ownership of the critical section
    User_leaveCriticalSection(NULL);
    return (dwError);
}
#endif

//DTV Intermediate Frequency: 36MHz
INT32 tun_cdt_9VM135_40_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    struct COFDM_TUNER_CONFIG_EXT *cfg;

    if ((ptrTuner_Config == NULL))
        return ERR_FAILUE;

    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if (tuner_id != NULL)
        *tuner_id = tuner_cnt;
    tuner_cnt++;

    return SUCCESS;
}

INT32 tun_cdt_9VM135_40_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data;

    struct COFDM_TUNER_CONFIG_EXT *p_tuner_config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    p_tuner_config = tuner_dev_cfg[tuner_id];

//    result = i2c_read(p_tuner_config->i2c_type_id,p_tuner_config->c_tuner_base_addr, &data, 1);
//    *lock = ((data & 0x40) >> 6);

    *lock = 1;

    return result;
}

INT32 tun_cdt_9VM135_40_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    return CDT_DTV4X_SetTuner( tuner_id, freq, bandwidth );
}


#endif