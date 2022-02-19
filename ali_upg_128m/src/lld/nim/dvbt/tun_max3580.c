/*
    1.0.  20081104.  Joey. Driver update according to Max3580 sample code, mainly adjust the sequence. Meanwhile
                                     change CR0b from 0x74 to 0x38.
*/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>

#include "tun_max3580.h"

#if ((SYS_TUN_MODULE == MAX3580) || (SYS_TUN_MODULE == ANY_TUNER))

typedef struct
{
    UINT8 tuner_inited;
    UINT8 tf_ser_200;
    UINT8 tf_par_200;
    UINT8 tf_sht_200;
    UINT8 tf_ser_470;
    UINT8 tf_par_470;
    UINT8 tf_sht_470;
    UINT8 tf_ser_860;
    UINT8 tf_par_860;
    UINT8 tf_sht_860;
    UINT8 bb_filt7;
    UINT8 bb_filt8;

}TUN_PRIV_INFO, *PTUN_PRIV_INFO;

#define MAX3580_MINVCO 530000  /* RF frequency below which the RF VCO needs to be divided down in Hz */
#define NIM_PRINTF(...)
//#define NIM_PRINTF soc_printf//libc_printf

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;
static TUN_PRIV_INFO tuner_priv[MAX_TUNER_SUPPORT_NUM];

static void max3580_read_rom(UINT32 tuner_id)
{ /* Read out Max3580 ROM table containing calibration constants
     and setup global variables with that data. */

    int i,b[7];
    UINT8 data[2];
    INT32 result;

    PTUN_PRIV_INFO priv = &tuner_priv[tuner_id];
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config = tuner_dev_cfg[tuner_id];

    for (i=1; i<=6; i++)
    {
        data[0]= 0x0d ;
        data[1] = i;
        result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 2);
        if (result != SUCCESS)
        {
            NIM_PRINTF("i2c failed 7!\n");
        }

        data[0] = 0x10;
        result = ptrTuner_Config->tuner_write_read(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 1,1);
        if (result != SUCCESS)
        {
            NIM_PRINTF("i2c failed 8!\n");
        }

        b[i]=data[0];
    }
    data[0]= 0x0d;
    data[1] = 0;
    result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, data, 2);  //ReSet Index in ROM in table to 0
    if (result != SUCCESS)
    {
        NIM_PRINTF("i2c failed 9!\n");
    }

    priv->tf_ser_200 = b[1] >> 4;
    priv->tf_sht_200 = b[1] & 0xF;
    priv->tf_par_200 = b[2] & 0xF;
    priv->tf_ser_470 = b[3] >> 4;
    priv->tf_sht_470 = b[3] & 0xF;
    priv->tf_par_470 = b[4] & 0xF;
    priv->tf_ser_860 = b[5] >> 4;
    priv->tf_sht_860 = b[5] & 0xF;
    priv->tf_par_860 = b[4] >> 4;
    priv->bb_filt8=b[6] >> 4;
    priv->bb_filt7=b[6] & 0xF;

}



/*****************************************************************************
* INT32 tun_max3580_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner max3580 Initialization
*
* Arguments:
* Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    NIM_PRINTF("tun_max3580_init!!\n");
    if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if (tuner_id)
        *tuner_id = tuner_cnt;
    tuner_priv[tuner_cnt].tuner_inited = 0;

    tuner_cnt++;
    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_max3580_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_max3580_status(UINT32 tuner_id, UINT8 *lock)
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

    data = 0x11;
    result = ptrTuner_Config->tuner_write_read(ptrTuner_Config->i2c_type_id,ptrTuner_Config->c_tuner_base_addr, &data, 1, 1);
    *lock = (UINT8)((data>>4)&0x01);

    if (result != SUCCESS)
    {
        NIM_PRINTF("i2c failed 0!\n");
    }

    return result;
}

/*****************************************************************************
* INT32 tun_max3580_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
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
INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    //value cal for: 1: N-divider 2: N-divider Frac. 3: Tracking Filter series and Parallel and ? Cap. 4: Vco-div
    // 5: Pll config. 6: Test Function. 7: Shut down control. 8: Vco control. 9: Baseband control. 10: DC offset control
    // 11: DC DAC . 12: Rom addr. 13: Rom write data.  15: Mixer Harmonic

    //16: Rom Data read-back. 17: chip status read-back. 18: Auto-tuner read-back.

//joey. 20081031. for max3580 474M sometime tuner unlock issue.

    INT32 result = ERR_FAILUE;
    UINT8 N_int;
    volatile UINT32 N_frac;
    UINT8 i;
    UINT8 tmp_d[16];
    UINT8 LO_div, vco_div = 0, tf_bs = 0, rfs;
    UINT8 LPF_div, Adly, cps, icp, R_div;
    UINT8 cap_par, cap_ser, cap_shunt;
    UINT8 cp_tst, ld_mux, turbo, vas, adl, ade, bb_bw, pd_th, dc_th, dc_sp, dc_mo, bb_bia, vco_sel /*, dc_dac*/;
    UINT8 tf_ser, tf_par, tf_sht;
    UINT8 rf_is;
    UINT8 wr_array[13];

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }

    PTUN_PRIV_INFO priv = &tuner_priv[tuner_id];
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    if (SLOW_TIMECST_AGC == AGC_Time_Const)
    {
        return SUCCESS;
    }

    if (priv->tuner_inited == 0) //do the cal read.
    {
        // read max3580 internal rom at first time.
        max3580_read_rom(tuner_id);
        priv->tuner_inited = 1;
    }

    LO_div = 2;

//cr[0~3].
    UINT32 tuner_crystal = ptrTuner_Config->c_tuner_crystal; //khz

    if(tuner_crystal >= 1000)
    {
        // tuner crystal kHz, trans to Mhz.
        tuner_crystal = tuner_crystal/1000;
    }

    N_int = (UINT8)(freq/ ((tuner_crystal *1000)/LO_div));
    N_frac = (UINT32)(((freq * 1.0/1000) /(tuner_crystal * 1.0/LO_div) - N_int) * (UINT32)(1<<20));

    tmp_d[0] = N_int;
    //mp=0, LI1LI0 = 00, INT = 1.
    tmp_d[1] = (UINT8)(0x10 | ((N_frac>>16) & 0x0f));
    tmp_d[2] = (UINT8)((N_frac>>8) & 0xff);
    tmp_d[3] = (UINT8)(N_frac & 0xff);


      // libc_printf("freq is %d, N is %x %x %x %x.\n",freq,tmp_d[0],tmp_d[1],tmp_d[2],tmp_d[3]);


//cr[4] && cr[5]

  /* choose VCO divider: start dividing LO if below MAX3580_MINVCO */
  if (freq > MAX3580_MINVCO)
    vco_div = 0x00;                  /* dont divide at all */
  else if (freq > MAX3580_MINVCO/2)
    vco_div = 0x1;                  /* div-by-2 */
  else if (freq > MAX3580_MINVCO/4)
    vco_div = 0x2;                  /* div-by-4 */
  else
    vco_div = 0x2;                  /* div-by-8, but never used >173 MHz */

  /* RF tracking filter. Note different interpolation algorithms
     used for VHF III vs. UHF band. No range checking done. */
  if (freq < 300000)
  {
      /* use calibration constants and substract 1 count per 10 MHz */
       tf_ser  = (UINT8) (0.5 + priv->tf_ser_200 - (freq-200000.0)/10000.0);
       tf_sht = (UINT8)(0.5 + priv->tf_sht_200 - (freq-200000.0)/10000.0);
       tf_par  = (UINT8)(0.5 + priv->tf_par_200 - (freq-200000.0)/10000.0);
    tf_bs = 1;    /*  Vhf */
    rf_is = 1;   /* vhf, rf2*/
  }
  else
  { /* plain linear interpolation between 470M and 860M Hz,
       correctly round to nearest integer */
       tf_ser = (UINT8) (0.5 + 0.5+priv->tf_ser_470+(freq-470000.0)/390000.0*(priv->tf_ser_860-priv->tf_ser_470));
       tf_sht = (UINT8) (0.5 + 0.5+priv->tf_sht_470+(freq-470000.0)/390000.0*(priv->tf_sht_860-priv->tf_sht_470));
       tf_par = (UINT8) (0.5 + 0.5+priv->tf_par_470+(freq-470000.0)/390000.0*(priv->tf_par_860-priv->tf_par_470));
    tf_bs = 0;    /*vhf */
    rf_is = 1;   /*  uhf, rf1  */
  }

    /* just in case, limit to 4 bits to prevent rounding problems near 470 MHz */
  if (tf_ser > 15) tf_ser = 15;
  if (tf_sht > 15) tf_sht = 15;
  if (tf_par > 15) tf_par = 15;
  tmp_d[4] = tf_ser | (tf_par << 4);
  tmp_d[5] = (vco_div<<6 )| (rf_is << 5) | (tf_bs << 4) | tf_sht ;


//cr[6].
    LPF_div = 0x02;// 10-8.
    Adly = 0x01;
    icp = 0x00;
    cps = 0x01;
    R_div = 0x01; //divider by 2.

    tmp_d[6] = (UINT8)(R_div<<7 | icp<<6 | cps<<5 | Adly<<3 | LPF_div);
    
//in max3580 1.0.7 version. the below is not exist. but for the old compatible, we still use old result.
//cr[0a].
    if ( freq > 300000 )    // UHF
    {
        bb_bw = priv->bb_filt8;
    }
    else                         // VHF
    {
        bb_bw = priv->bb_filt7;
    }
    pd_th = 0x07;
    tmp_d[10] = (UINT8)(bb_bw<<4 | pd_th);

//cr[7].
    cp_tst = 0x00;
    ld_mux = 0x00;
    turbo = 0x01;
    tmp_d[7] = (UINT8)(cp_tst<<5 | turbo<<3 | ld_mux);
//cr[8].//shut down control, no shut down.

    //use as Cofdm_Config&0x04
    tmp_d[8] = 0x20;
    /*
    if ((ptrTuner_Config->Cofdm_Config&0x04) == 0x04 ) //rf-agc enabled.
    {
        // disable baseband Power Detector circuit
        tmp_d[8] = 0x20;
    }
    else
    {
        tmp_d[8] = 0x00;
    }
    */

//cr[9].
    vas = 0x01; //auto-tuner.
    adl = 0x00;
    ade = 0x00;
    vco_sel = 0x03;
    tmp_d[9] = (UINT8)(vco_sel<<6 | vas<<2 | adl<<1 | ade);

//cr[0b]~cr[0c].
    dc_th = 0x00;
    dc_sp = 0x01; //02
    dc_mo =  0x03;
    bb_bia = 0x00;
    tmp_d[11] = (UINT8)(bb_bia<<7 | 0x00<<6 | dc_mo<<4 | dc_sp<<2 | dc_th);
    tmp_d[12] = 0x00;

//joey. 20081031. for max3580 474M sometime tuner unlock issue.
//cr[0d]~cr[0f]
    tmp_d[13] = 0x00;
    tmp_d[14] = 0x00;
    tmp_d[15] = 0x00;


//write in cr4~0f. auto-tuner function.
    wr_array[0] = 4;
    for (i=0;i<12;i++)
    {
        wr_array[i+1] = tmp_d[i+4];
    }
    result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, wr_array, 13);
    if (result != SUCCESS)
    {
        NIM_PRINTF("i2c failed 1!\n");
    }

//write in cr0~3. pll divider.
    wr_array[0] = 0;
    for (i=0;i<4;i++)
    {
        wr_array[i+1] = tmp_d[i];
    }
    result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, wr_array, 5);
    if (result != SUCCESS)
    {
        NIM_PRINTF("i2c failed 4!\n");
    }

//wait for lock.
//wait for 50ms.
    osal_task_sleep(50);

    i = 0;
    while (1)
    {
        wr_array[0] = 0x11;
        result = ptrTuner_Config->tuner_write_read(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, wr_array, 1, 1);
        if (result == SUCCESS)
        {
            if ((wr_array[0] & 0x10) == 0x10) //LD detect.
                break;
        }
        if (result != SUCCESS)
        {
            NIM_PRINTF("i2c failed 6!\n");
        }

        //wait for 10ms.
        osal_task_sleep(10);
        i++;
        if (i>5)
        {
            break;
        }
    }

    return result;

}

INT32 tun_max3580_powcontrol(UINT32 tuner_id,UINT8 stdby)
{
    INT32 result = ERR_FAILUE;
    UINT8    data[2];
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;
    PTUN_PRIV_INFO priv = &tuner_priv[tuner_id];
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    data[0]=0x08;        //address data

    if (stdby)
    {
        //libc_printf("start standby mode!\n");
        data[1]=0x6F;
        result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, data, 2);
        if (result!=SUCCESS)
            return result;
    }
    else
    {
        //use as Cofdm_Config&0x04
        data[1] = 0x20;
        /*
        //libc_printf("start wakeup mode!\n");
        if ((ptrTuner_Config->Cofdm_Config&0x04) == 0x04 ) //rf-agc enabled.
        {
        // disable baseband Power Detector circuit
        data[1] = 0x20;
        }
        else
        {
        data[1] = 0x00;
        }
        */
        result = ptrTuner_Config->tuner_write(ptrTuner_Config->i2c_type_id, ptrTuner_Config->c_tuner_base_addr, data, 2);
        if (result!=SUCCESS)
            return result;
    }

    return SUCCESS;

}

INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
/*
    switch(cmd)
    {
        case NIM_TUNER_POWER_CONTROL:
            tun_max3580_powcontrol(tuner_id, param);
            break;

        default:
            ret = ERR_FAILUE;
            break;
    }
*/
    return ret;
}

#endif

