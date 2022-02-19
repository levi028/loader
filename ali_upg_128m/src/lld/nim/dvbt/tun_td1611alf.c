#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "tun_td1611alf.h"

#if ((SYS_TUN_MODULE == TD1611ALF) || (SYS_TUN_MODULE == ANY_TUNER))

#define  TD1611_DEBUG_FLAG 0
#if(TD1611_DEBUG_FLAG)
#define NIM_PRINTF soc_printf
#else
#define NIM_PRINTF(...)
#endif

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;

/*****************************************************************************
* INT32 tun_td1611alf_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner td1611alf Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_td1611alf_init(UINT32 *tuner_id , struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    *tuner_id = tuner_cnt;
    NIM_PRINTF(" INIT td1611 : tuner_id=%d\n",tuner_id);
    tuner_cnt++;

    return SUCCESS;
}
// flag bit explanation:
//
//bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
//bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
//bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>

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

INT32 tun_td1611alf_cal_agc(UINT32 tuner_id, UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data)
{
    UINT8 ret_val = 0;
    UINT16 if_max = 1023;
    UINT16 if_min = 0;
    INT8  if_gain=0;
    INT8  rf_gain=0;

    INT8   gain_max = 60;
    INT8   gain_min = -25;
    INT8   gain_val;

    INT8 rf_in_max = -30;
    //INT8 rf_in_min = -80;
    INT8 rf_in_min = -72;
       INT8 rf_in;

    INT32  Acoeff= 0, Bcoeff = 0, Ccoeff = 0;
    INT32  x1=0, x2=0, x3=0;

    UINT16 agc_max=0x3ff,agc_min=0;
    UINT8 roll_tmp;

    // IF RSSI's on
    if (flag & IF_AGC_CTRL_EN)
    {
        // calculate IF gain.
        if ( if_val <= 257 )
        {
            Acoeff = 0;
            Bcoeff = 0;
            Ccoeff = 0;
            if_gain = 0;
        }
        else if (  (if_val>257 ) &&(if_val<=511))
        {
             Acoeff = 0;
             Bcoeff = -81;
             Ccoeff = 70;
             x1 = 257;  x2 = 382; x3 = 511;
        }
        else if (  (if_val>511 ) &&(if_val<=689))
        {
            Acoeff = 166;
            Bcoeff = -346;
            Ccoeff = 169;
            x1 = 511;  x2 = 589; x3 = 689;
        }
        else if (  (if_val>689 ) &&(if_val<=880))
        {
            Acoeff = 175;
            Bcoeff = -341;
            Ccoeff = 171;
            x1 = 689;  x2 = 779; x3 = 880;
        }
        else if ( if_val > 880 )
        {
            Acoeff = 0;
            Bcoeff = 0;
            Ccoeff = 0;
            if_gain = 35;
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
        if ( rf_val <= 52 )
            {
                 Acoeff = 0;
                 Bcoeff = 0;
                 Ccoeff = 0;
                 rf_gain = -25;
            }
        else if (  (rf_val>52 ) &&(rf_val<=73))
        {
             Acoeff = -13227;  
             Bcoeff = 16666;
             Ccoeff = -4761;
             x1 = 52;  x2 = 61; x3 = 73;
        }
            else if (  (rf_val>73 ) &&(rf_val<=100))
        {
             Acoeff = -4444;
             Bcoeff = 5294;
             Ccoeff = -1089;
             x1 = 73;  x2 = 83; x3 = 100;
        }
            else if (  (rf_val>100 ) &&(rf_val<=132))
        {
             Acoeff = -977;
             Bcoeff = 781;
             Ccoeff = 391;
             x1 = 100;  x2 = 116; x3 = 132;
        }
            else if (  (rf_val>132 ) &&(rf_val<=147))
        {
             Acoeff = 2222;
             Bcoeff = -7407;
             Ccoeff = 5185;
             x1 = 132;  x2 = 138; x3 = 147;
        }
            else if (  (rf_val>147 ) &&(rf_val<=163))
        {
             Acoeff = 5468;
             Bcoeff = -17188;
             Ccoeff = 11719;
             x1 = 147;  x2 = 155; x3 = 163;
        }
            else if (  (rf_val>163) &&(rf_val<=184))
        {
             Acoeff = 7143;
             Bcoeff = -17273;
             Ccoeff = 9091;
             x1 = 163;  x2 = 173; x3 = 184;
        }
            else if (  (rf_val>184 ) &&(rf_val<=233))
        {
             Acoeff = 2679;
             Bcoeff = -4167;
             Ccoeff = 1546;
             x1 = 184;  x2 = 200; x3 = 233;
        }
            else if ( rf_val>233 )
        {
             Acoeff = 0;
             Bcoeff = 0;
             Ccoeff = 0;
                 rf_gain = 25;
        }

        if ( Acoeff || Bcoeff ||Ccoeff )
        {
               rf_gain = ( Acoeff*(rf_val-x2)*(rf_val-x3) + Bcoeff*(rf_val-x1)*(rf_val-x3) + Ccoeff*(rf_val-x1)*(rf_val-x2) ) / 100000;
        }

    }


    // if both IF and RF is off, set  default value.
    if ((flag & (IF_AGC_CTRL_EN | RF_RSSI_EN)) == (IF_AGC_CTRL_EN | RF_RSSI_EN))
    {
        // map absolute gain number to percent.
        gain_val = if_gain + rf_gain;

              // rf_in +  gain_val = -35
           rf_in = -35-gain_val;

              //  confine rf_in in [rf_in_min,rf_in_max]
              if ( rf_in >= rf_in_max ) rf_in = rf_in_max;
              if ( rf_in <= rf_in_min )  rf_in = rf_in_min;

        ret_val = (UINT8)( 100*(rf_in-rf_in_min)/(rf_in_max-rf_in_min));

        if (ret_val >= 100)
        {
            ret_val = 100;
        }
    }
       else if ((flag & ( IF_AGC_CTRL_EN | RF_RSSI_EN )) == IF_AGC_CTRL_EN)
    {

              // redefine the range as requeird by JiuZhou customer.
              // see changelist 55440 description of this file.
           rf_in_max = -60;
              rf_in_min = -80;

           // map absolute gain number to percent.
        gain_val = if_gain ;

              // rf_in +  gain_val = -35
              // -35-if_gain range from [-80 -60 ] requires if_gain range from [25,45],
              // but the if_gain ranges from [0,35] , so re-map if_gain range  by the following eqution:
              // 20/35*(if_gain) + 25
              // which make if_gain range from [25,45]
           rf_in = -35-(gain_val*20/35 + 25 );

              //  confine rf_in in [rf_in_min,rf_in_max]
              if ( rf_in >= rf_in_max ) rf_in = rf_in_max;
              if ( rf_in <= rf_in_min )  rf_in = rf_in_min;

        ret_val = (UINT8)( 100*(rf_in-rf_in_min)/(rf_in_max-rf_in_min));

        if (ret_val >= 100)
        {
            ret_val = 100;
        }

        // libc_printf("ret_val: %d, rf_in:%d , gain:%d, rf_in_max:%d,rf_in_min:%d\n", ret_val,rf_in,gain_val,rf_in_max,rf_in_min);

       }
    else
    {
             ret_val = 20;
    }

       // OSD display Signal Intensity as (1-d%, d=ret_val), so make a complementary value here.
    *data =100 - ret_val;

    return SUCCESS;

}

/*****************************************************************************
* INT32 tun_td1611alf_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_td1611alf_status(UINT32 tuner_id, UINT8 *lock)
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
        if(result!=0)
    {
        NIM_PRINTF("t_read=%d\n",result);
    }
        *lock = ((data & 0x40) >> 6);
    NIM_PRINTF(" Tuner lock=%d\n",*lock);
    return result;
}

/*****************************************************************************
* INT32 nim_td1611alf_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_td1611alf_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result;
    UINT32 tmp;
    UINT16 Npro;
    UINT8 CP2_0, SP4_3, SP2_0 = 0, ATC, R2_0;
    INT32 err=0;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    NIM_PRINTF("td1611alf_control : tuner_id=%d\n",tuner_id);

    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    //change ref divider according to diff bandwidth.
    if (bandwidth == 8)
    {
        ptrTuner_Config->c_tuner_ref_div_ratio = 28;
    }
    else
    {
        ptrTuner_Config->c_tuner_ref_div_ratio = 24;
    }
    /*N value*/

    UINT32 tuner_crystal = ptrTuner_Config->c_tuner_crystal; //khz

    if(tuner_crystal >= 1000)
    {
        // tuner crystal kHz, trans to Mhz.
        tuner_crystal = tuner_crystal/1000;
    }
    NIM_PRINTF("tuner_crystal=%d\n",tuner_crystal);

    tmp=((freq+ptrTuner_Config->w_tuner_if_freq+(ptrTuner_Config->c_tuner_step_freq/2))*(ptrTuner_Config->c_tuner_ref_div_ratio))/tuner_crystal;

    Npro=tmp/1000;
    NIM_PRINTF("Tuner Set Freq Npro=%d\n",Npro);
    data[0] = (UINT8) ((Npro>>8)&0x7F);
    data[1] = (UINT8) ((Npro)&0xFF);


    if (ptrTuner_Config->c_tuner_ref_div_ratio == 24) //step 166.7
    {
        R2_0 = 0x02;
    }
    else if (ptrTuner_Config->c_tuner_ref_div_ratio == 28) //step 142.86
    {
        R2_0 = 0x01;
    }
    else
    {
        return ERR_FAILUE;

    }

    if (FAST_TIMECST_AGC == AGC_Time_Const)
        ATC = 0x01;
    else
        ATC = 0x00;

    // control byte 1.
    if ((FAST_TIMECST_AGC == AGC_Time_Const) && (_i2c_cmd == _1ST_I2C_CMD))
    {
        data[2] = (UINT8)(0x80 | 0x40 | 0x08 | (R2_0&0x07));
                data[4] = (UINT8)(0x80 | 0x00 | ((ATC<<3)&0x08) | (ptrTuner_Config->c_tuner_agc_top&0x07));
    }
    else
    {
        data[2] = (UINT8)(0x80 | 0x00 | ((ATC<<3)&0x08) | (ptrTuner_Config->c_tuner_agc_top&0x07));
    }


    /*CP */
    if ((freq <= 94000) || ((freq>164000) && (freq<=254000)) || ((freq>444000) && (freq<=584000)))
    {
        CP2_0 = 0x03;
    }
    else if (((freq>94000) && (freq<= 124000)) || ((freq>254000) && (freq<=384000)) || ((freq>584000) && (freq<=794000)))
    {
        CP2_0 = 0x05;
    }
    else if (((freq>124000) && (freq<=164000)) || ((freq>384000) && (freq<=444000)))
    {
        CP2_0 = 0x06;
    }
    else if (freq>794000)
    {
        CP2_0 = 0x07;
    }
    else
    {
        return ERR_FAILUE;
    }

    if (freq < 162000)
    {
        SP2_0 = 0x01;
    }
    else if ((freq>=162000) && (freq<448000))
    {
        SP2_0 = 0x02;
    }
    else if (freq>=448000)
    {
        SP2_0 = 0x04;
    }

    if (bandwidth == 8)
    {
        SP4_3 = 0x01;
    }
    else
    {
        SP4_3 = 0x00;
    }

    // control byte 2.
    data[3] = (UINT8)(((CP2_0<<5)&0xe0)  | ((SP4_3<<3)&0x18) | (SP2_0&0x07));


    if(ptrTuner_Config->tuner_write!=NULL)
    {

        osal_task_sleep(10);
                 #if(SYS_DEM_MODULE == ST0362)
            if (FAST_TIMECST_AGC == AGC_Time_Const)
        {
                err+=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 5);
        }

        if(err!=0)
        {
            NIM_PRINTF("Tuner_WR=%d\n",err);
            ;
        }
        else
            NIM_PRINTF("Tuner_WR_SUCCESS\n",err);
                 #else
        if (FAST_TIMECST_AGC == AGC_Time_Const)
        {
            if(_i2c_cmd==_1ST_I2C_CMD)
            {
                err+=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 4);
            }
            else if(_i2c_cmd==_2ND_I2C_CMD)
            {
                err+=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, &data[2], 2);
            }
        }
        else
        {
            if(_i2c_cmd==_2ND_I2C_CMD)
            {
                err+=ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, &data[2], 2);
            }

        }
        if(err!=0)
        {
            NIM_PRINTF("Tuner_WR=%d\n",err);
            ;
        }
               #endif
    }

    if ((AGC_Time_Const == FAST_TIMECST_AGC) && (_i2c_cmd==_2ND_I2C_CMD))
    {
        //just wait once 100ms in ATC = 1 state.
        osal_task_sleep(100);
    }

    return SUCCESS;


}

#endif



