#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_edt1022.h"

#if ((SYS_TUN_MODULE == EDT1022) || (SYS_TUN_MODULE == ANY_TUNER)||(SYS_TUN_MODULE == EDT1022B))

#define NIM_PRINTF(...)

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;

/*****************************************************************************
* INT32 tun_edt1022_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner edt1022 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_edt1022_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    //tuner_id = (UINT8)tuner_cnt;
    tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_edt1022_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_edt1022_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    result = ptrTuner_Config->tuner_read(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, &data, 1);
    *lock = ((data & 0x40) >> 6);

    return result;
}

/*****************************************************************************
* INT32 nim_edt1022_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth        : channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const    : AGC time constant
*  Parameter4: UINT8 *data        :
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_edt1022_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result;
    UINT32 tmp;
    UINT16 Npro;
    UINT32 freq1;
    UINT8 CP2_0, SP5, SP4, SP3_1, ATC, R2_0, AL2_0;
        UINT32 err=0;

    UINT8 i;
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

//***************Prog.Byte1 and Prog.Byte2*******************
    //change ref divider according to diff bandwidth.
    /*N value*/
    UINT32 tuner_crystal = ptrTuner_Config->c_tuner_crystal; //khz

    if(tuner_crystal >= 1000)
    {
        // tuner crystal kHz, trans to Mhz.
        tuner_crystal = tuner_crystal/1000;
    }

    tmp=((freq+ptrTuner_Config->w_tuner_if_freq+(ptrTuner_Config->c_tuner_step_freq/2))*(ptrTuner_Config->c_tuner_ref_div_ratio))/tuner_crystal;
    Npro=tmp/1000;
    NIM_PRINTF("Tuner Set Freq Npro=%d\n",Npro);




//***************Cont.Byte1*******************
    if (ptrTuner_Config->c_tuner_ref_div_ratio == 24) //step 166.7
        R2_0 = 0x02;
    else if (ptrTuner_Config->c_tuner_ref_div_ratio == 64)
        R2_0 = 0x00;
    else if (ptrTuner_Config->c_tuner_ref_div_ratio == 80)
        R2_0 = 0x03;
    else
    {
        //libc_printf("R1R0 cal error! \n");
        return ERR_FAILUE;
    }


    if (AGC_Time_Const == FAST_TIMECST_AGC)
        ATC = 0x01;
    else
        ATC = 0x00;

  AL2_0 = ptrTuner_Config->c_tuner_agc_top&0x07;



//***************Cont.Byte2*******************
   freq1 = freq+36000;
  if         (((freq1 >=  85000) && (freq1 < 130000)) | ((freq1 >= 198000) && (freq1 < 290000)) | ((freq1 >= 480000) && (freq1 < 620000)))
      CP2_0 = 0x03;
  else if (((freq1 >= 130000) && (freq1 < 160000)) | ((freq1 >= 290000) && (freq1 < 420000)) | ((freq1 >=  620000) && (freq1 < 830000)))
      CP2_0 = 0x05;
  else if (((freq1 >= 160000) && (freq1 < 198000)) | ((freq1 >= 420000) && (freq1 <  480000)))
      CP2_0 = 0x06;
  else if  ((freq1 >= 830000) && (freq1 < 897000))
      CP2_0 = 0x07;
  else
      {
        //libc_printf("CP error! \n");
        return ERR_FAILUE;
      }


  if      ((freq >= 49000 ) && (freq < 159000))
      SP3_1 = 0x01;
  else if ((freq >= 162000) && (freq < 444000))
      SP3_1 = 0x02;
  else if ((freq >= 448000) && (freq < 861000))
      SP3_1 = 0x04;
  else
       {
        //libc_printf("SP3_1 error! \n");
        return ERR_FAILUE;
      }


  if (bandwidth == 8)
      SP4 = 0x01;
  else
      SP4 = 0x00;

    SP5 = 0x00;



//***************Data[4:0]*******************

    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) ((Npro)&0xFF);
    data[2] = (UINT8) ( 0xC0 | 0x08 | R2_0 );
    data[3] = (UINT8) ((CP2_0 << 5) | (SP5 << 4) | (SP4 << 3) | SP3_1 );
    data[4] = (UINT8) ( 0x80 | (ATC << 3) | AL2_0 );

    if(ptrTuner_Config->tuner_write!=NULL)
    {
        if(_i2c_cmd==_1ST_I2C_CMD)
        {
/*            libc_printf("The Value write to EDT1022 is:\n");
            for (i=0;i<5;i++)
            {
                libc_printf("%2x \n", data[i]);
            }
            libc_printf("\n");*/

            err+=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 5);
            NIM_PRINTF("tuner_err=%d\n",err);
        }
    }

    if ((AGC_Time_Const == FAST_TIMECST_AGC) && (_i2c_cmd==_1ST_I2C_CMD))
    {
        //just wait once 100ms in ATC = 1 state.
        osal_task_sleep(100);
    }

    return SUCCESS;


}

// flag bit explanation:
//
//bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
//bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
//bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
//bit5: RF RSSI enable.<0: disable, 1: enalbe>  bit6 :   RF-AGC slop <0: negtive, 1: positive>

#define SLOP_NEGTIVE                  0
#define SLOP_POSITIVE                 1

#define IF_AGC_CTRL_EN_B        0
#define IF_AGC_CTRL_SLOP_B        1
#define RF_AGC_CTRL_EN_B             2
#define RF_AGC_CTRL_SLOP_B         3
#define LOW_OR_ZERO_IF_B            4
#define RF_RSSI_EN_B                     5
#define RF_RSSI_SLOP_B                 6

#define IF_AGC_CTRL_EN            (1 << IF_AGC_CTRL_EN_B)
#define RF_RSSI_EN                    (1 << RF_RSSI_EN_B)

#define IF_AGC_CTRL_SLOP        (1 << IF_AGC_CTRL_SLOP_B)
#define IF_AGC_CTRL_SLOP_POSITIVE  ( SLOP_POSITIVE << IF_AGC_CTRL_SLOP_B)

INT32 tun_edt1022_cal_agc(UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data)
{
    UINT8 ret_val = 0;

    INT8  if_gain=0;
    INT8  rf_gain=0;

    INT8   gain_val;

    INT8 rf_in_max = -30;
       INT8 rf_in_min = -80;
       INT8 rf_in;

    INT32  Acoeff= 0, Bcoeff = 0, Ccoeff = 0;
    INT32  x1=0, x2=0, x3=0;

    UINT16 gain_max=0x3ff,gain_min=0;
    UINT8 data1[2];



    // IF RSSI's on
    if (flag & IF_AGC_CTRL_EN)
    {
        // calculate IF gain.
        if ( if_val <= 288 )
        {
            Acoeff = 0;
            Bcoeff = 0;
            Ccoeff = 0;
            if_gain = 0;
        }
        else if (  (if_val>288 ) &&(if_val<=472))
        {
             Acoeff = 0;
             Bcoeff =  -101;
             Ccoeff = 90;
             x1 = 288;  x2 = 375; x3 = 472;
        }
        else if (  (if_val>472 ) &&(if_val<=640))
        {
            Acoeff = 146;
            Bcoeff = -291;
            Ccoeff = 133;
            x1 = 472 ;  x2 = 537; x3 = 640;
        }
        else if (  (if_val>640 ) &&(if_val<=710))
        {
            Acoeff = 671;
            Bcoeff = -2332;
            Ccoeff = 1667;
            x1 = 640;  x2 = 689; x3 = 710;
        }
        else if (  (if_val>710 ) &&(if_val<=965))
        {
            Acoeff = 65;
            Bcoeff = -161;
            Ccoeff = 94;
            x1 = 710;  x2 = 857; x3 = 965;
        }
        else if ( if_val > 965 )
        {
            Acoeff = 0;
            Bcoeff = 0;
            Ccoeff = 0;
            if_gain = 26;
        }

        if ( Acoeff || Bcoeff ||Ccoeff )
        {
            if_gain = ( Acoeff*(if_val-x2)*(if_val-x3) + Bcoeff*(if_val-x1)*(if_val-x3) + Ccoeff*(if_val-x1)*(if_val-x2) )  / 100000;
        }
    }


    // RF RSSI's on
    if (flag & RF_RSSI_EN)
    {
        // calculate RF gain.
        if ( rf_val <= 38 )
            {
                 Acoeff = 0;
                 Bcoeff = 0;
                 Ccoeff = 0;
                 rf_gain = -2;
            }
        else if (  (rf_val>38 ) &&(rf_val<=62))
        {
             Acoeff = -724;
             Bcoeff = -3571;
             Ccoeff = 3521;
             x1 = 38;  x2 = 50; x3 = 62;
        }
            else if (  (rf_val>62 ) &&(rf_val<=100))
        {
             Acoeff = 1518;
             Bcoeff = -4187;
             Ccoeff = 2546;
             x1 = 62;  x2 = 79; x3 = 100;
        }
            else if (  (rf_val>100 ) &&(rf_val<=131))
        {
             Acoeff = 3947;
             Bcoeff = -9375;
             Ccoeff =  5442;
             x1 = 100;  x2 = 116; x3 = 131;
        }
            else if (  (rf_val>131 ) &&(rf_val<=147))
        {
             Acoeff = 19471;
             Bcoeff = -47990;
             Ccoeff = 28551;
             x1 = 131;  x2 = 140; x3 = 147;
        }
            else if (  (rf_val>147) &&(rf_val<=172))
        {
             Acoeff =  16976;
             Bcoeff = -27750;
             Ccoeff = 9973;
             x1 = 147;  x2 = 155; x3 = 172;
        }
            else if (  (rf_val>172 ) &&(rf_val<=232))
        {
             Acoeff = 2092;
             Bcoeff = -4704;
             Ccoeff = 2622;
             x1 = 172;  x2 = 205; x3 = 232;
        }
            else if ( rf_val>232 )
        {
             Acoeff = 0;
             Bcoeff = 0;
             Ccoeff = 0;
                 rf_gain = 43;
        }

        if ( Acoeff || Bcoeff ||Ccoeff )
        {
               rf_gain = ( Acoeff*(rf_val-x2)*(rf_val-x3) + Bcoeff*(rf_val-x1)*(rf_val-x3) + Ccoeff*(rf_val-x1)*(rf_val-x2) ) / 100000;
        }

    }


    if ((flag & (IF_AGC_CTRL_EN | RF_RSSI_EN)) == (IF_AGC_CTRL_EN | RF_RSSI_EN))
    {
        // map absolute gain number to percent.
        gain_val = if_gain + rf_gain;

              // rf_in +  gain_val = -12
           rf_in = -18-gain_val;

              //  confine rf_in in [rf_in_min,rf_in_max]
              if ( rf_in >= rf_in_max )
        {
            rf_in = rf_in_max;
            ret_val = 100;
              }else if  ( rf_in <= rf_in_min )
              {
                  rf_in = rf_in_min;
            ret_val = 0;
              }else
              {
            ret_val = (UINT8)( (100*(rf_in-rf_in_min))/(rf_in_max-rf_in_min));

            if (ret_val >= 100)
            {
                ret_val = 100;
            }
              }

           // OSD display Signal Intensity as (1-d%, d=ret_val), so make a complementary value here.
        *data =100 - ret_val;

        libc_printf("Percent is %d, Rf val is %d, If val is %d, rf_gain is %d, if_gain is %d, rf level is %d. \n ", ret_val,rf_val, if_val, rf_gain,if_gain,rf_in);

    }
       else if ((flag & ( IF_AGC_CTRL_EN | RF_RSSI_EN )) == IF_AGC_CTRL_EN)
    {
        // agc_max=70;
        gain_max=1200;
        gain_min=50;//agc_max_th=800,agc_min_th=50
        ret_val = (UINT8)(100*(if_val -gain_min)/(gain_max-gain_min));//agc_max_th=800,agc_min_th=230 based on practice agc valu

        if(ret_val >= 100)
            ret_val = 99;
        else if (ret_val == 0)
            ret_val = 1;

        //check IF_slope to decide its positive proportion or negative.
              if ( flag & IF_AGC_CTRL_SLOP == IF_AGC_CTRL_SLOP_POSITIVE )
        {
            // ret_val = 100-ret_val;
        }

        *data =  ret_val;
       }
    else
    {
        ret_val = 20;
        *data =100 - ret_val;
    }

    return SUCCESS;

}



#endif





