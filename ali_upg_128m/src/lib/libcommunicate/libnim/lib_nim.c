/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_nim.c
*
* Description:
*     This file contains all NIM library define.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Sep.9.2003       Justin Wu       Ver 0.1    Create file.
*    2.    Nov.1,2004         Zhengdao         Ver 0.2    fastscan optimize.
*    3.    Dec.20,2004         Zhengdao         Ver 0.3    bug-fix for storage full.
*    4.  06/25/2005       Berg Xing       Ver 0.4    clear some debug array
*    5.  06/29/2005         Berg Xing         Ver 0.5    Remove the limit of scan end freq
*   6.  08/09/2005       Berg Xing       Ver 0.6    Merge M3327C QPSK driver into M3327

*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_dev.h>
#include <api/libc/printf.h>
#include <api/libpub27/lib_as.h>
#include <bus/aci/aci.h>
#include <api/libnim/lib_nim.h>
#include <api/libpub/lib_frontend.h>
#include <string.h>

/*******************************************************
* macro define
********************************************************/
#define NIM_PRINTF(...)
#define SMTANT_PRINTF(...)

/*******************************************************
* structure and enum ...
********************************************************/

/*******************************************************
* global and local variable
********************************************************/
static struct nim_device *lib_nim_dev = NULL;
static int lib_nim_dev_cnt = 0;

#if F00_MODEL1
static BOOL g_smant_exit;
#endif

extern BOOL osal_address_is_code_section(void *dest);
/*******************************************************
* local function declare
********************************************************/

/*****************************************************************************
* INT32 api_nim_register()
*
* Register a NIM library
*
* Arguments:
*  Parameter1: char *nim_name
*
* Return Value: INT32
*****************************************************************************/
__ATTRIBUTE_REUSE_
INT32 api_nim_register(void)
{
    if (NULL == (lib_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0)))
    {
        PRINTF("api_nim_init: Can't find NIM device!\n");
        return ERR_NO_DEV;
    }
    if (0 == lib_nim_dev_cnt)
    {
        if (nim_open(lib_nim_dev) != SUCCESS)
        {
            return ERR_FAILUE;
        }
    }
    else if (1 == lib_nim_dev_cnt)
    {
        if (NULL == (lib_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1)))
        {
            PRINTF("api_nim_init: Can't find NIM device!\n");
            return ERR_NO_DEV;
        }
        if (nim_open(lib_nim_dev) != SUCCESS)
        {
            return ERR_FAILUE;
        }
    }
    lib_nim_dev_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 api_nim_unregister(char *nim_name)
*
* Unregister a NIM library
*
* Arguments:
*  Parameter1: char *nim_name
*
* Return Value: INT32
*****************************************************************************/
INT32 api_nim_unregister(void)
{
    if (0 == (--lib_nim_dev_cnt))
    {
        return nim_close(lib_nim_dev);
    }

    return SUCCESS;
}

/*****************************************************************************
* UINT32 api_nim_get_lock()
*
* Get NIM lock status
*
* Arguments:
*
* Return Value: UINT8
*****************************************************************************/
UINT8 api_nim_get_lock(void)
{
    UINT8 lock = 0;
    INT32 ret = 0;

    if ((ret = nim_get_lock(lib_nim_dev, &lock)) != SUCCESS)
    {
        PRINTF("api_nim_get_lock: operation error! %d\n", ret);
        return 0;
    }

    return lock;
}

/*****************************************************************************
* UINT32 api_nim_get_freq()
*
* Get NIM frequence
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT32 api_nim_get_freq(void)
{
    /*
     * m3327 driver will return the frequency difference, so offset
     * should be set to 0.
     *    Zhengdao, 20050111
     */
    UINT32 freq=0;

    if (nim_get_freq(lib_nim_dev, &freq) != SUCCESS)
    {
        return 0;
    }

    return freq;
}

/*****************************************************************************
* UINT8 api_nim_get_FEC()
*
* Get NIM code rate
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT8 api_nim_get_fec(void)
{
    UINT8 fec = 0;

    if (nim_get_fec(lib_nim_dev, &fec) != SUCCESS)
    {
        return 0;
    }

    return fec;
}

/*****************************************************************************
* UINT8 api_nim_get_SNR()
*
* Get NIM SNR
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT8 api_nim_get_snr(void)
{
    UINT8 snr = 0;

    if (nim_get_snr(lib_nim_dev, &snr) != SUCCESS)
    {
        return 0;
    }

    return snr;
}

/*****************************************************************************
* void api_nim_stop_autoscan(void)
*
* Stop Auto Scan
*
* Arguments:
*
* Return Value: 
*****************************************************************************/
void api_nim_stop_autoscan(void)
{
    nim_io_control(lib_nim_dev,NIM_DRIVER_STOP_ATUOSCAN,1);
}

/*****************************************************************************
* void api_nim_reset_autoscan(void) 
*
* Reset Auto Scan
*
* Arguments:
*
* Return Value: 
*****************************************************************************/
void api_nim_reset_autoscan(void)        //50916-01Angus
{
    nim_io_control(lib_nim_dev,NIM_DRIVER_STOP_ATUOSCAN,0);
}


#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)

/*****************************************************************************
* INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)())
*
* NIM autoscan
*
* Arguments:
*  Parameter1: UINT32 sfreq
*  Parameter2: UINT32 efreq
*  Parameter3: INT32 (*callback)()
*
* Return Value: INT32
*****************************************************************************/
#if (((SYS_CHIP_MODULE == ALI_M3327) || (SYS_CHIP_MODULE == ALI_M3327C)) \
    && (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2) \
    || (SYS_CHIP_MODULE == ALI_S3601) || (SYS_CHIP_MODULE == ALI_S3602))
INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq,
        INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec))
{
#if 1   // unified NIM API
    struct NIM_AUTO_SCAN dvb_s_as_param;

    MEMSET(&dvb_s_as_param, 0, sizeof(dvb_s_as_param));
    // Stop current running Autoscan porcedure
    nim_io_control(lib_nim_dev,NIM_DRIVER_STOP_ATUOSCAN,0);

    if((sfreq > efreq) || (!osal_address_is_code_section(callback)))
        return ERR_FAILUE;
    // make up the autoscan parameter list
    dvb_s_as_param.sfreq = sfreq;
    dvb_s_as_param.efreq = efreq;
    dvb_s_as_param.callback = callback;

    return nim_ioctl_ext(lib_nim_dev, NIM_DRIVER_AUTO_SCAN, (void *)( &dvb_s_as_param));

#else // non-unified NIM API
/*    UINT8 cur_fec;
    UINT32 cur_freq, cur_sym;
    INT32 i, locked, ret = SUCCESS;
    INT32 crnum;
    UINT32 FFT_freq, step_freq = 0;
    UINT32 FFT_result;

    PRINTF("api_nim_autoscan: Begin autoscan\n");

    PRINTF("sfreq = %d, efreq = %d\n", sfreq, efreq);

    if ((sfreq < 950) || (efreq > 2150) || (sfreq > efreq))
    {
        return ERR_FAILUE;
    }
    nim_io_control(lib_nim_dev,NIM_DRIVER_STOP_ATUOSCAN,0);

#if ((SYS_CHIP_MODULE == ALI_M3327C) || ((SYS_CHIP_MODULE == ALI_M3327)&& defined(M3327SW_AUTOSCAN)))
    for(FFT_freq = sfreq; FFT_freq < efreq; FFT_freq+=step_freq)
    {
        FFT_result = nim_io_control(lib_nim_dev,NIM_DRIVER_FFT,FFT_freq);
//        if( SUCCESS == FFT_result )
        {
            step_freq = nim_io_control(lib_nim_dev, NIM_FFT_JUMP_STEP, 0);
            crnum = nim_io_control( lib_nim_dev, NIM_DRIVER_GET_CR_NUM, 0);
            PRINTF("FFT_freq=%d, crnum=%d, step_freq=%d\n", FFT_freq, crnum, step_freq);
            for( i = 0; i < crnum; i++ )
            {
                if ( SUCCESS == nim_channel_search(lib_nim_dev, (UINT32)i) )
                {
                    cur_freq = (UINT32)nim_io_control(lib_nim_dev, NIM_DRIVER_GET_CUR_FREQ, 0);
                    nim_get_freq(lib_nim_dev, &cur_freq);
                    nim_get_sym(lib_nim_dev, &cur_sym);
                    nim_get_FEC(lib_nim_dev, &cur_fec);
                    if ((cur_freq >= sfreq) && (cur_freq <= efreq))
                        ret = callback(1, 0, cur_freq, cur_sym, cur_fec);
                    else
                        continue ;
                }
                else
                {
                    ret = callback(0, 0, FFT_freq, 0, 0);
                }
                if (2 == ret)
                {
                    return SUCCESS;
                }
                else if (1 == ret)
                {
                    goto nim_as_break;
                }
            }
            if(crnum == 0)
            {
                if(1 == callback(0, 0,FFT_freq, 0, 0))
                {
                    goto nim_as_break;
                }
            }
        }
    }
#elif (SYS_CHIP_MODULE == ALI_M3327)
    //begin set FFT para firstly
    nim_io_control(lib_nim_dev,NIM_DRIVER_FFT_PARA,0);
    //begin FFT firstly
    for(FFT_freq = sfreq; FFT_freq < efreq; FFT_freq+=step_freq) {
        FFT_result = nim_io_control(lib_nim_dev,NIM_DRIVER_FFT, (UINT32)FFT_freq);
        //libc_printf( "FFT_result is 0x%x\n", FFT_result );
        //step_freq = 0;
        if( SUCCESS == FFT_result )
        {
            step_freq = nim_io_control(lib_nim_dev, NIM_FFT_JUMP_STEP, 0);
        }
        else
        {
            if ((FFT_result&0x0e)&&(FFT_result&0x01))
            {
                //libc_printf( "FFT 3\n");
                step_freq = nim_io_control(lib_nim_dev, NIM_FFT_JUMP_STEP, 0);
            }
            else
            {
                step_freq = 0;
            }

            if((locked =(FFT_result&0x02)>>1)) {
                //libc_printf( "FFT 1\n");
                //lock channel
                cur_freq = FFT_freq;
                PRINTF("important cur_freq is %d\n", cur_freq );
                nim_get_freq(lib_nim_dev, &cur_freq);
                nim_get_sym(lib_nim_dev, &cur_sym);
                nim_get_FEC(lib_nim_dev, &cur_fec);
                if ((cur_freq >= sfreq) && (cur_freq <= efreq))
                    ret = callback(1, 0, cur_freq, cur_sym, cur_fec);
                else
                    continue ;
            }
            else
            {
                ret = callback(0, 0, FFT_freq, 0, 0);
            }
            if (1 == ret)
            {
                return 1;
            }
            else if (ret == 2)
            {
                    goto nim_as_break;
            }
        }
    }

    //begin search channel which FFT no locked
    crnum = nim_io_control( lib_nim_dev, NIM_DRIVER_GET_CR_NUM, 0);
    callback(AS_SGN_CRNUM, 0, crnum, 0, 0);
    PRINTF("crnum is %d\n", crnum);
    for(i = 0; i < crnum; i++) {
        if ( SUCCESS == nim_channel_search(lib_nim_dev, i)){
            cur_freq = (UINT32)nim_io_control(lib_nim_dev, NIM_DRIVER_GET_CUR_FREQ, 0);
            nim_get_freq(lib_nim_dev, &cur_freq);
            nim_get_sym(lib_nim_dev, &cur_sym);
            nim_get_FEC(lib_nim_dev, &cur_fec);
            if ((cur_freq >= sfreq) && (cur_freq <= efreq))
                ret = callback(1, 0, cur_freq, cur_sym, cur_fec);
            else
                continue ;
        }
        else
        {
            ret = callback(0, i, 0, 0, 0);
        }

        if (2 == ret)
        {
            return SUCCESS;
        }
        else if (1 == ret)
        {
            goto nim_as_break;
        }
    }
#endif

    return SUCCESS;
nim_as_break:
    callback(2, 0, 0, 0, 0);    // Tell callback search finished
    PRINTF("%s: finish autoscan\n", __FUNCTION__);

    return ret;
    */
#endif
}
#endif

/*****************************************************************************
* INT32  api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec)
*
* NIM change channel
*
* Arguments:
*  Parameter1: UINT32 freq
*  Parameter2: UINT32 sym
*  Parameter3: UINT8 fec
*
* Return Value: INT32
*****************************************************************************/
INT32  api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec)
{
#if 1    // Unified NIM API
    struct NIM_CHANNEL_CHANGE dvb_s_cc_param;

    MEMSET(&dvb_s_cc_param, 0, sizeof(dvb_s_cc_param));
    // make up the autoscan parameter list
    if((freq == 0) && (sym == 0) && (fec == 0))
         return ERR_FAILUE;
    dvb_s_cc_param.freq = freq;
    dvb_s_cc_param.sym = sym;
    dvb_s_cc_param.fec = fec;

    return nim_ioctl_ext(lib_nim_dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)( &dvb_s_cc_param));
#else    // Non-unified NIM API
    return nim_channel_change(lib_nim_dev, freq, sym, fec);
#endif
}

/*****************************************************************************
* UINT32 api_nim_get_sym()
*
* Get NIM symbol rate
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT32 api_nim_get_sym(void)
{
    UINT32 sym = 0;

    if (nim_get_sym(lib_nim_dev, &sym) != SUCCESS)
    {
        return 0;
    }

    return sym;
}

/*****************************************************************************
* UINT8 api_nim_get_AGC()
*
* Get NIM AGC
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT8 api_nim_get_agc(void)
{
    UINT8 agc = 0;

    if (nim_get_agc(lib_nim_dev, &agc) != SUCCESS)
    {
        return 0;
    }

    return agc;
}

/*****************************************************************************
* UINT32 api_nim_get_BER()
*
* Get NIM BER
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT32 api_nim_get_ber(void)
{
    UINT32 ber = 0;

    if (nim_get_ber(lib_nim_dev, &ber) != SUCCESS)
    {
        return 0;
    }

    return ber;
}

void api_nim_update_param(void)
{
    nim_io_control(lib_nim_dev,NIM_DRIVER_UPDATE_PARAM,0);
}


INT32 api_nim_get_fft_result(UINT32 freq, UINT32* start_adr)
{
    /*
     * m3327 driver will return the frequency difference, so offset
     * should be set to 0.
     *    Zhengdao, 20050111
     */
    if((freq<FRONTEND_LNB_FREQ_LOW) || (freq>FRONTEND_LNB_FREQ_HIGH))
    {
        return ERR_FAILUE;
    }
    if(NULL == start_adr)
    {
        return ERR_ADDRESS;
    }

    return nim_get_fft_result(lib_nim_dev, freq, start_adr);
}


#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)

#ifdef SMART_ANT_SUPPORT

/********************************************************************************
Description:
Finds optimum antenna setting.

Parameters:
Inputs:
UINT32 freq   - frequency
Outputs:
UINT16 *antval final lock data about smart ant setting
UINT8 *detect smart ant detect
sm_status_update- Reserve

Return: INT32, operation status code; configuration successful return with SUCCESS
*******************************************************************************/


INT32  api_nim_auto_seek_smartenna(UINT32 freq, UINT16 *antval,UINT8 *detect, sm_status_update sm_status)
{
#if ((SMART_ANT_SUPPORT == SMART_ANT_BY_GPIO)||(SMART_ANT_SUPPORT == SMART_ANT_BY_ALI))
    UINT8 i = 0;
    UINT8 j = 0;
    UINT8 k = 0;
    UINT32 data = 0;
    UINT8 position = 0;
    UINT8 gain = 0;
    UINT8 pol = 0;
    UINT8  channel = 0;
    INT32 lock = 0;
    UINT16 pre_snr=0,
    UINT16 pre_agc=0xffff;
    UINT16 snr = 0;
    UINT16 agc = 0;
    UINT8 det = 0;
    struct aci_device*    aci_dev = NULL;
    const UINT32 low_freq = 177000;
    const UINT16 agc_value_check = 6000;
    const UINT16 snr_threshod = 240;

    if ((NULL == antval) || (NULL == detect) || (NULL == sm_status))
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    aci_dev = (struct aci_device *)dev_get_by_id(HLD_DEV_TYPE_ACI, 0);
    SMTANT_PRINTF("freq=%d\n",freq);
    if(freq<=low_freq)
    {
        channel=4;
    }
    else
    {
        channel=38;
    }
    det=aci_detect_antenna(aci_dev);
    *detect=det;
    if(det)
    {
        //soc_printf("detect wo ant\n");
        return RET_FAILURE;
    }
    for(i=0;i<16;i++)
    {
        sm_status(i);
        #if F00_MODEL1
        if(g_smant_exit)
        {
            g_smant_exit=FALSE;
            //soc_printf("loop\n");
            break;
        }
        #endif

        for(j=0;j<4;j++)
        {        //soc_printf("dir=%d,gain=%d\n",i,j);
            api_nim_set_smartenna ( i, j, 0, channel);
            osal_task_sleep(400);
            lock=api_nim_get_lock();
            SMTANT_PRINTF("lock=%d\n",lock);
            if (lock)
            {
                snr = api_nim_get_vsb_snr();
                SMTANT_PRINTF("snr=%d\n",snr);
                if (snr>pre_snr )
                {
                    pre_snr = snr;
                    //osal_task_sleep(20);
                    agc = api_nim_get_vsb_agc();
                    if(agc_value_check == agc)
                    {
                        agc = api_nim_get_vsb_agc();
                    //    soc_printf("agc ustable\n");
                    }
                    if(agc<pre_agc)
                    {
                        pre_agc = agc;
                    }
                    *antval = (i <<10)|(j<<7)|(channel);
                    if (snr >= snr_threshod)
                    {
                        //soc_printf("GOOD SNR\n");
                        break;
                    }
                }
            }
        }
    }

    position=(*antval&0x3c00)>>10;;
    pol=(*antval&0x200)>>9;
    gain=(*antval&0x180)>>7;

     SMTANT_PRINTF("freq=%d, ,agc=%d, snr=%d, seek result=%x, Position=%d,Gain=%d,Pol=%d,Channel=%d\n",
            freq,pre_agc,pre_snr,*antval,position,gain,pol,channel);
#endif
    return RET_SUCCESS;
}

/*******************************************************************************
Description:
Configures antenna specifics

Parameters:
Inputs:
UINT16 value    - value to send to smart antenna.
Outputs:
 UINT8 *pPosition        - 4 unsignificant bits (range 0-15)
 UINT8 *pGain        - 2 unsignificant bits (range 0-3)
 UINT8 ata8 *pPol        - 1 unsignificant bit (0 or 1)
 UINT8 *pChannel        - 7 unsignificant bits (range 0-127)

*******************************************************************************/
INT8 api_nim_config_smartenna(UINT16 value,UINT8 *p_position,UINT8 *p_pol,UINT8 *p_gain,UINT8 *p_channel)
{
    UINT8 position = 0;
    UINT8 gain = 0;
    UINT8 pol = 0;
    UINT8 channel = 0;
    UINT8 lock = 0;

    if ((NULL == p_position) || (NULL == p_pol) ||
        (NULL == p_gain) || (NULL == p_channel))
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    position=(value&0x3c00)>>10;;
    pol=(value&0x200)>>9;
    gain=(value&0x180)>>7;
    channel=(value&0x7f);
    // soc_printf("pPosition=%d,pGain=%d,pPol=%dp,Channel=%d\n",position,gain,pol,channel);

     api_nim_set_smartenna(position,gain,pol,channel);
        //osal_task_sleep(400);
    // lock=api_nim_get_lock();

    *p_position=position;
    *p_pol=pol;
    *p_gain=gain;
    *p_channel=channel;

//soc_printf("position =%d  , pol=%d , gain=%d ,channel=%d  \n",*pPosition,*pPol,*pGain,*pChannel);

    return RET_SUCCESS;
}


/******************************************************************************

Description:
Sets position, gain, polarization, and channel number manually.

Parameters:
Inputs:
UINT8 position    - 4 unsignificant bits (range 0-15)
UINT8 gain    - 2 unsignificant bits (range 0-3)
UINT8 pol        - 1 unsignificant bit (0 or 1)
UINT8 channel    - 7 unsignificant bits (range 0-127)

Return: INT32, operation status code; configuration successful return with SUCCESS

*******************************************************************************/
INT8 api_nim_set_smartenna(UINT8 position,UINT8 gain,    UINT8 pol,UINT8 channel)
{
#if ((SMART_ANT_SUPPORT == SMART_ANT_BY_GPIO)||(SMART_ANT_SUPPORT == SMART_ANT_BY_ALI))
    UINT32 data;

    struct aci_device*    aci_dev = (struct aci_device *)dev_get_by_id(HLD_DEV_TYPE_ACI, 0);

    data = (position <<10)|(pol<<9)|(gain<<7)|(channel);
    aci_transmit_series_data(aci_dev, 14,data);
#elif (SMART_ANT_SUPPORT == SMART_ANT_BY_DEMOD)
    if (nim_set_smartenna(lib_nim_dev, position,gain,pol,channel) != SUCCESS)
    {
        return 0;
    }
#endif
    //soc_printf("ap hier %d\n",hier);
    return SUCCESS;
}



INT8 api_nim_get_smartenna_setting(UINT8 *p_position,UINT8 *p_gain,UINT8 *p_pol,UINT8 *p_channel)
{

    if (nim_get_smartenna_setting(lib_nim_dev, p_position,p_gain,p_pol,p_channel) != SUCCESS)
    {
        return 0;
    }

    //soc_printf("ap hier %d\n",hier);
    return SUCCESS;
}

INT8 api_nim_get_smartenna_metric(UINT8 metric,UINT16 *p_metric)
{


    if (nim_get_smartenna_metric(lib_nim_dev, metric,p_metric) != SUCCESS)
    {
        return 0;
    }

    //soc_printf("ap hier %d\n",hier);
    return SUCCESS;
}


UINT16 api_nim_get_vsb_snr(void)
{
    UINT16 snr;

    if (nim_get_vsb_snr(lib_nim_dev, &snr) != SUCCESS)
    {
        return 0;
    }

    return snr;
}

UINT16 api_nim_get_vsb_agc(void)
{
    UINT16 agc;

    if (nim_get_vsb_agc(lib_nim_dev, &agc) != SUCCESS)
    {
        return 0;
    }

    return agc;
}

UINT32 api_nim_get_vsb_per(void)
{
    UINT32 per;

    if (nim_get_vsb_per(lib_nim_dev, &per) != SUCCESS)
    {
        return 0;
    }

    return per;
}
#endif
/*****************************************************************************
INT8 f_api_HIER_mode()
*
* Get HIER_mode
*
* Arguments:
*
* Return Value: INT8
*****************************************************************************/

#if (SYS_SDRAM_SIZE == 8)
INT8 api_nim_hier_mode(void)
{
    UINT8 hier;

    if (nim_get_hier_mode(lib_nim_dev, &hier) != SUCCESS)
    {
        return 0;
    }

    //soc_printf("ap hier %d\n",hier);
    return hier;
}

/*****************************************************************************
* INT8 api_nim_get_priority()
*
* Get get_priority
*
* Arguments:
*
* Return Value: INT8
*****************************************************************************/
INT8 api_nim_get_priority(void)
{
    UINT8 priority;
    if (nim_get_priority(lib_nim_dev, &priority) != SUCCESS)
    {
        return 0;
    }

    return priority;
}
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) || (defined(PORTING_ATSC))
void api_switch_nim_device(UINT8 nim_id)
{
    lib_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (nim_id-1));

}
#endif

INT32 api_nim_channel_change(UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, UINT8 fft_mode,
        UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
    NIM_PRINTF("api freq=%d\n",freq);
    NIM_PRINTF("api bandwidth=%d\n",bandwidth);
    api_nim_reset_autoscan();        //50916-01Angus
    return nim_channel_change(lib_nim_dev, freq, bandwidth, guard_interval, fft_mode,
        modulation, fec, usage_type, inverse, priority);
}


INT32 api_nim_channel_search(UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, UINT8 fft_mode,
        UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority)
{
    NIM_PRINTF("channel_search api freq=%d\n",freq);
    NIM_PRINTF("channel_search api bandwidth=%d\n",bandwidth);

    api_nim_reset_autoscan();        //50916-01Angus
    NIM_PRINTF("nim_io_control set STOP_ATUOSCAN=0 \n ");
    return nim_channel_search(lib_nim_dev, freq, bandwidth, guard_interval,
            fft_mode, modulation, fec, usage_type, inverse, freq_offset, priority);
}


UINT16 api_nim_get_agc(void)
{
    UINT16 agc;

    if (nim_get_agc(lib_nim_dev, &agc) != SUCCESS)
    {
        return 0;
    }

    return agc;
}

UINT8 api_nim_get_freqinv(void)
{
    UINT8 inv = 0;

    if (nim_get_spec_inv(lib_nim_dev, &inv) != SUCCESS)
    {
        return 1;
    }

    return inv;
}

UINT8 api_nim_get_fftmode(void)
{
    UINT8 fft_mode = 0;

    if (nim_get_fftmode(lib_nim_dev, &fft_mode) != SUCCESS)
    {
        return 0;
    }

    return fft_mode;
}

UINT8 api_nim_get_gi(void)
{
    UINT8 guard_interval = 0;

    if (nim_get_gi(lib_nim_dev, &guard_interval) != SUCCESS)
    {
        NIM_PRINTF("nim_get_gi, unsuccess\n");
        return 0;
    }
    NIM_PRINTF("nim_get_gi, guard_interval=%d\n",guard_interval);
    return guard_interval;
}

UINT8 api_nim_get_modulation(void)
{
    UINT8 modulation = 0;

    if (nim_get_modulation(lib_nim_dev, &modulation) != SUCCESS)
    {
        return 0;
    }
    return modulation;
}

INT32 api_nim_disable(void)//Sam_Chen 20050616
{
//    if (--lib_nim_dev_cnt == 0)
//    {
//        soc_printf("Api_Disable!!,enter Api disable\n");
        return nim_disable(lib_nim_dev);
//    }

//    return SUCCESS;
}
#if (SYS_SDRAM_SIZE == 8 || GET_BER == SYS_FUNC_ON)
UINT32 api_nim_get_ber(void)
{
    UINT32 ber;

//return 0;
    if (nim_get_ber(lib_nim_dev, &ber) != SUCCESS)
    {
        return 0;
    }

    return ber;
}
#endif
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)

/*****************************************************************************
* INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)())
*
* NIM autoscan
*
* Arguments:
*  Parameter1: UINT32 sfreq
*  Parameter2: UINT32 efreq
*  Parameter3: INT32 (*callback)()
*
* Return Value: INT32
*****************************************************************************/
INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq,
        INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec))
{
#if 1   // unified NIM API
    struct NIM_AUTO_SCAN dvb_c_as_param ;

    MEMSET(&dvb_c_as_param, 0, sizeof(dvb_c_as_param));
    // make up the autoscan parameter list
    dvb_c_as_param.sfreq = sfreq;
    dvb_c_as_param.efreq = efreq;
    dvb_c_as_param.callback = callback;

    return nim_ioctl_ext(lib_nim_dev, NIM_DRIVER_AUTO_SCAN, (void *)( &dvb_c_as_param));

#else // non-unified NIM API
/*    UINT8 cur_fec;
    UINT32 cur_freq, cur_sym;
    INT32 i, locked, ret;
    INT32 crnum;
    UINT32 FFT_freq;
    UINT32 FFT_result;

    PRINTF("dvb-c api_nim_autoscan: Begin autoscan\n");
    for(FFT_freq = sfreq; FFT_freq < efreq; FFT_freq+=8) {
        FFT_result = nim_channel_search(lib_nim_dev, FFT_freq);
        if(SUCCESS == FFT_result)
        {
                cur_freq = FFT_freq;
                nim_get_freq(lib_nim_dev, &cur_freq);
                nim_get_sym(lib_nim_dev, &cur_sym);
                nim_get_FEC(lib_nim_dev, &cur_fec);
                ret = callback(1, 0, cur_freq, cur_sym, cur_fec);
        }
        else
        {
            ret = callback(0, 0, FFT_freq, 0, 0);
        }
        if (1 == ret)
        {
            return 1;
        }
        else if (ret == 2)
        {
            goto nim_as_break;
        }
    }
    return SUCCESS;
nim_as_break:
    callback(2, 0, 0, 0, 0);    // Tell callback search finished
    PRINTF("dvb-c %s: finish autoscan\n", __FUNCTION__);

    return ret;*/
#endif
}
INT32  api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec)
{
#if 1    // Unified NIM API
    struct NIM_CHANNEL_CHANGE dvb_c_cc_param ;

    MEMSET(&dvb_c_cc_param, 0, sizeof(dvb_c_cc_param));
    // make up the autoscan parameter list
    dvb_c_cc_param.freq = freq;
    dvb_c_cc_param.sym = sym;
    dvb_c_cc_param.modulation = fec;

    return nim_ioctl_ext(lib_nim_dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)( &dvb_c_cc_param));
#else    // Non-unified NIM API
    return nim_channel_change(lib_nim_dev, freq, sym, fec);
#endif
}

/*****************************************************************************
* UINT32 api_nim_get_sym()
*
* Get NIM symbol rate
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT32 api_nim_get_sym(void)
{
    UINT32 sym = 0;

    if (nim_get_sym(lib_nim_dev, &sym) != SUCCESS)
    {
        return 0;
    }

    return sym;
}

/*****************************************************************************
* UINT8 api_nim_get_AGC()
*
* Get NIM AGC
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT8 api_nim_get_agc(void)
{
    UINT8 agc = 0;

    if (nim_get_agc(lib_nim_dev, &agc) != SUCCESS)
    {
        return 0;
    }

    return agc;
}

/*****************************************************************************
* UINT32 api_nim_get_BER()
*
* Get NIM BER
*
* Arguments:
*
* Return Value: UINT32
*****************************************************************************/
UINT32 api_nim_get_ber(void)
{
    UINT32 ber = 0;

    if (nim_get_ber(lib_nim_dev, &ber) != SUCCESS)
    {
        return 0;
    }

    return ber;
}

#endif

#if ((SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))
INT32 api_nim_get_freq_offset(void)
{
    INT32 freq_offset = 0;

    if (nim_get_freq_offset(lib_nim_dev, &freq_offset) != SUCCESS)
    {
        return 0;
    }

    return freq_offset;
}
#endif

