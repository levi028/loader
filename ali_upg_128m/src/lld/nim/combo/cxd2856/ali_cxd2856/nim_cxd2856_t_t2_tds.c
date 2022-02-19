/*----------------------------------------------------------------------------
   Solution:  demodulater CXD2856. + tuner MXL603
------------------------------------------------------------------------------
    History:       
	Date                  athor           version                reason 
------------------------------------------------------------------------------
    2017/04/06          leo.liu           v0.1       create ali cxd2856 driver  to support dvbt/t2/isdbt      
-------------------------------------------------------------------------------
NOTE:
cxd2856 support repeater and getway mode of i2c to w/r tuner.repteater is simple.
so,we use the repeater mode of i2c,but you must enable repeater mode  before w/r tuner,
and disable repeater mode after w/r tuner .use the function: 
		sony_demod_I2cRepeaterEnable (pdemod, 0x01);
-------------------------------------------------------------------------------*/
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

#include "../sony_common.h"
#include "nim_cxd2856_channel_change.h"
#include "nim_cxd2856_monitor.h"
#include "nim_cxd2856_common.h"





//#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0])) 
#define TUNER_IFAGCPOS

#define BER_TIMEOUT_VAL  5000
#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))

#define NIM_FLAG_ENABLE    0x00000100
#define NIM_SCAN_END       0x00000001
UINT32 flag_tmp = 0;
static UINT8  t2_lite_support =0;


sony_i2c_t demodI2c; //I2C interface of DEM.
static INT32 nim_cxd2856_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	sony_integ_t * priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
    //INT32 freq_offset;
    //UINT8 data[2];
    INT32 ret_val = SUCCESS;
	if(dev == NULL)
	{
		return ERR_FAILUE;
	}
    if(cmd & NIM_TUNER_COMMAND)
    {
        if(NIM_TUNER_CHECK == cmd)
            return SUCCESS;

        if(pdemod->dvbt_config.nim_tuner_command != NULL)
        {
			if((UINT32)cmd == NIM_TUNER_SET_STANDBY_CMD)
        	{
        		NIM_MUTEX_ENTER(pdemod);
        		ret_val = pdemod->dvbt_config.nim_tuner_command(pdemod->tuner_id,NIM_TUNER_POWER_CONTROL, TRUE);
				NIM_MUTEX_LEAVE(pdemod);
        	}
			else
			{
				NIM_MUTEX_ENTER(pdemod);
				ret_val = pdemod->dvbt_config.nim_tuner_command(pdemod->tuner_id, cmd, param);
				NIM_MUTEX_LEAVE(pdemod);
			}
        }
        else
            ret_val = ERR_FAILUE;
        return ret_val;
    }

    switch( cmd )
    {
    	 case NIM_DRIVER_GET_AGC:
            ret_val =  nim_cxd2856_get_SSI(dev, (UINT8 *)param);						
            break;
		 case NIM_DRIVER_STOP_ATUOSCAN:
            pdemod->autoscan_stop_flag = param;
            break;
		case NIM_DRIVER_T2_LITE_ENABLE:
			t2_lite_support =(UINT8)param;
			break;
		case NIM_DRIVER_READ_RSUB:
        	ret_val = nim_cxd2856_get_per(dev, (UINT32 *)param);
        	break;
		case NIM_DRIVER_GET_BER:
            return nim_cxd2856_get_ber(dev, (UINT32 *)param);
            break;
		case NIM_DRIVER_GET_MODULATION:
            return nim_cxd2856_get_modulation(dev, (UINT8 *)param);
            break;
		case NIM_DRIVER_GET_FFT_MODE:
            return nim_cxd2856_get_fftmode(dev, (UINT8 *)param);
            break;
			
    	#if 0
       

        case NIM_DRIVER_DISABLE_DEBUG_LOG:
            param = (UINT32)NULL;
        case NIM_DRIVER_ENABLE_DEBUG_LOG:
            if(NULL == pdemod->output_buffer)
            {
                pdemod->output_buffer = (char *)MALLOC(LOG_STRING_BUFFER_SIZE);
                ASSERT(pdemod->output_buffer != NULL);
            }
            pdemod->fn_output_string = (LOG_STRING_FUNCTION)param;
            break;
		
        case NIM_DRIVER_DISABLED:
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return nim_cxd2837_get_GI(dev, (UINT8 *)param);
            break;
        
        case NIM_DRIVER_GET_DEMOD_LOCK_MODE:
		*(UINT8*)param = TRUE;
		break;
			break;
		
        case NIM_DRIVER_GET_SPECTRUM_INV:
            break;
		#endif
        default:
            ret_val = ERR_FAILUE;
            break;
    }

    return ret_val;
}

static INT32 nim_cxd2856_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
	struct NIM_CHANNEL_CHANGE *change_para;
	INT32 ret_val = SUCCESS;
	//UINT16 cn = 0;
	if(dev == NULL || param_list == NULL)
	{
		return ERR_FAILUE;
	}
	switch (cmd)
	{
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            ret_val = nim_cxd2856_channel_change(dev, change_para);
            break;
		case NIM_DRIVER_GET_CN_VALUE:
			ret_val = nim_cxd2856_get_cn(dev,(UINT16 *)param_list);
        	break;
		case NIM_DRIVER_GET_RF_LEVEL:
        	ret_val = nim_cxd2856_get_rf_level(dev, (UINT16 *)param_list);
        	break;
		 default:
            break;
	}

	return ret_val;
}

/*****************************************************************************
* INT32 nim_cxd2834_close(struct nim_device *dev)
* Description: cxd2834 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_cxd2856_close(struct nim_device *dev)
{
    //UINT8 data;
    sony_integ_t * priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
    INT32 result = SUCCESS;

    // tuner power off
    nim_cxd2856_ioctl(dev, NIM_TUNER_POWER_CONTROL, TRUE);

    result = sony_demod_Shutdown(pdemod);//sony_dvb_demod_Finalize
    if (result != SONY_RESULT_OK)
        result = ERR_FAILUE;
    return result;
}


__ATTRIBUTE_REUSE_
static INT32 nim_cxd2856_open(struct nim_device *dev)
{
	sony_integ_t * priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	
    //struct COFDM_TUNER_CONFIG_API * config_info = &(pdemod->dvbt_config);
    //sony_dvbt2_tune_param_t  TuneParam;
    sony_result_t result = SONY_RESULT_OK;
	// Initialize the demod.
	result = sony_integ_Initialize (priv);
    if(result != SONY_RESULT_OK)
	{
        CXD2856_LOG("sony_integ_Initialize failed.\n");
        return result;
    }
    //Demod can be config now.

    result = sony_demod_I2cRepeaterEnable (pdemod, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
	pdemod->dvbt_config.nim_tuner_command(pdemod->tuner_id, NIM_TUNER_POWER_CONTROL, FALSE);//tuner power on
	result = sony_demod_I2cRepeaterEnable (pdemod, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
	
	/********************* config demod *********************/
	//step 1:=====config iffreq of demod
	sony_demod_iffreq_config_t iffreqConfig;
#if 1
	iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);

	iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);

	iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);
	iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);
	iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);

	iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);
	iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(5);

	iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.75);
#else

	iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.6);
	iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.6);
	iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.2);
	iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.8);

	iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.5);
	iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.6);
	iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.6);
	iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.2);
	iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.8);

	iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);
	iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);
	iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.9);

	iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.55);
	iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.15);
	iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(4.75);

	iffreqConfig.configISDBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configJ83B_5_06_5_36 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.7);
	iffreqConfig.configJ83B_5_60 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(3.75);
#endif	
	if (sony_demod_SetIFFreqConfig (pdemod, &iffreqConfig)!= SONY_RESULT_OK) {
		CXD2856_LOG("sony_demod_SetIFFreqConfig failed. (%s)\n",__FUNCTION__);
		return ERR_DEV_ERROR;
	}
	
	/*
	 * In default, The setting is optimized for Sony silicon tuners.If non-Sony tuner is used, the user should call following to
	 * disable Sony silicon tuner optimized setting.
	 */
	//step 2:=====if non-sony is used.config it
	if (sony_demod_SetConfig (pdemod, SONY_DEMOD_CONFIG_TUNER_OPTIMIZE, SONY_DEMOD_TUNER_OPTIMIZE_NONSONY) != SONY_RESULT_OK) {
	   	CXD2856_LOG("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TUNER_OPTIMIZE) failed. (%s)\n",__FUNCTION__);
		return ERR_DEV_ERROR;
	}
	// step 3:=====config TS output
    //Config TS output mode: SSI/SPI. Default is SPI.
    result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_PARALLEL_SEL, 0 );
	if (result != SONY_RESULT_OK)
	{
		CXD2856_LOG( "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
	//Confif Data MSB or LSB pin: 0:LSB , 1:MSB, default:1.
	sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_OUTPUT_SEL_MSB, 1 );

    //Confif Data output pin: 0:TS0(pin13), 1:TS7(pin20), default:1.
    result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, 0 );
	if (result != SONY_RESULT_OK)
	{
		CXD2856_LOG( "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
	//Config TS Sync output.
	result = sony_demod_SetConfig(pdemod,SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI, 1);
	if (result != SONY_RESULT_OK)
	{
		CXD2856_LOG( "Unable to configure DEMOD_CONFIG_TSERR_ENABLE. (status=%d, %s)\r\n",result, FormatResult(result));
	}
    //config TS Error output.
	result = sony_demod_SetConfig(pdemod,SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI, 1);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2856_LOG( "Error: Unable to configure DEMOD_CONFIG_TSERR_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	//step 4 : ======config polarity of IFAGC for tuner;0 : for positive; 1 : for negtive
	if(SYS_TUN_MODULE == MXL603)
	{
		libc_printf("======== TUNER_IFAGCPOS \n");
		result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
		if (result != SONY_RESULT_OK)
		{
			CXD2856_LOG( "Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
			return result;
		}
	}
	else
	{
		result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_IFAGCNEG, 1);
		if (result != SONY_RESULT_OK)
		{
			CXD2856_LOG( "Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
			return result;
		}
	}
	/***************************** special config**********************
	Before burning sony demod ,if frequently called function of monitoring(SSI,SQI,RF,PER...).
	sony demod will  be abnormal,resulting in i2c error,so add the special config
	*/

	//Configure the clock frequency for Serial TS in terrestrial and cable active states.
	 result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ, 3 );
	 if (result != SONY_RESULT_OK)
	 {
			  CXD2856_LOG("Unable to configure SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ.",ERR_DEV_ERROR);
	 }
	 //Configure Serial TS clock gated on valid TS data or is continuous.
	 result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_TSCLK_CONT, 0);
	 if (result != SONY_RESULT_OK)
	 {
			  CXD2856_LOG("Unable to configure SONY_DEMOD_CONFIG_TSCLK_CONT.",ERR_DEV_ERROR);
	 }
	// Configure the driving current for the TS Clk pin.
     result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_TSCLK_CURRENT, 1 );
     if (result != SONY_RESULT_OK)
     {
              CXD2856_LOG("Unable to configure SONY_DEMOD_CONFIG_TSCLK_CURRENT.",ERR_DEV_ERROR);
     }
     // Configure the driving current for the TS Sync / TS Valid.
     result = sony_demod_SetConfig(pdemod, SONY_DEMOD_CONFIG_TS_CURRENT, 1);
     if (result != SONY_RESULT_OK)
     {
              CXD2856_LOG("Unable to configure SONY_DEMOD_CONFIG_TS_CURRENT.",ERR_DEV_ERROR);
     }

	/**********************************************************************/

	/*IFAGC ADC range[0-2]     0 : 1.4Vpp, 1 : 1.0Vpp, 2 : 0.7Vpp*/   
	result = sony_demod_SetConfig(pdemod,SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 1);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2856_LOG( "Error: Unable to configure SONY_DEMOD_CONFIG_IFAGC_ADC_FS. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	//Ben Debug 140221#1
	//add by AEC for TS error enable 2013-09-09
	 // TSERR output enable from GPIO2 pin
	result = sony_demod_GPIOSetConfig(pdemod, 2, 1, SONY_DEMOD_GPIO_MODE_TS_OUTPUT);
	if(result != SONY_RESULT_OK)
	{
		CXD2856_LOG("Error in sony_demod_GPIOSetConfig for TS error.\n");
		return result;
	}
#if MONITOR_DEBUG
	if(nim_cxd2856_monitor_task_init(dev) != SUCCESS)
	{
		return ERR_NO_DEV;
	}
#endif	
    osal_flag_set(pdemod->flag_id, NIM_SCAN_END);
    return SUCCESS;
}

static ID create_i2c_mutex(UINT32 i2c_type_id)
{ 
	sony_integ_t * priv = NULL;
	sony_demod_t * pdemod = NULL;
    struct nim_device *dev;
    UINT16 dev_id = 0;
    UINT16 mutex_id = OSAL_INVALID_ID;

    while(1)
    {
        dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, dev_id++);
        if(NULL == dev)
            break;
		priv = (sony_integ_t *)dev->priv;
        pdemod = (sony_demod_t *)priv;
    }

    mutex_id = osal_mutex_create();
    return mutex_id;
}


/*****************************************************************************
* INT32  nim_cxd2856_attach_t(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
* Description: cxd2856 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
__ATTRIBUTE_REUSE_
INT32 nim_cxd2856_attach_t(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
{
	/*nim_device	|
				|----->sony_integ_t	|
				|					|--->sony_demod_t
	*/
    struct nim_device *dev = NULL;
    sony_integ_t *priv = NULL;
	sony_demod_t *pdemod = NULL;
	sony_demod_create_param_t createParam;//xtal,i2c_addr
	sony_tuner_terr_cable_t * tunerTerrCable = NULL;
    DEM_WRITE_READ_TUNER ThroughMode;
    //UINT32 TunerIF;
    if(NULL == pConfig)
    {
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if(NULL == dev)
    {
        return ERR_NO_MEM;
    }

	/* Alloc structure space of private */
    priv = (sony_integ_t*)MALLOC(sizeof(sony_integ_t));
    if(NULL == priv)
    {
        dev_free(dev);
        return ERR_NO_MEM;
    }
    MEMSET((UINT8*)priv, 0, sizeof(sony_integ_t));
	
    pdemod = (sony_demod_t*)MALLOC(sizeof(sony_demod_t));
	if(NULL == pdemod)
	{
		dev_free(dev);
		FREE(priv);
        return ERR_NO_MEM;
	}
	MEMSET((UINT8*)pdemod, 0, sizeof(sony_demod_t));
	
	tunerTerrCable = (sony_tuner_terr_cable_t *)MALLOC(sizeof(sony_tuner_terr_cable_t));
    MEMSET(tunerTerrCable, 0, sizeof(sony_tuner_terr_cable_t));
	/*Setup demod I2C && XTAL interfaces.*/
	createParam.xtalFreq = SONY_DEMOD_XTAL_24000KHz; /* 24MHz Xtal */
    createParam.i2cAddressSLVT = pConfig->ext_dm_config.i2c_base_addr; /* I2C slave address */

	
	/*
	   Note: If use the repeater of i2c mode,demod will be abnormal when a long time burn.
	   So,must use the gatewat of i2c mode.
	*/
    //createParam.tunerI2cConfig = SONY_DEMOD_TUNER_I2C_CONFIG_REPEATER; /* I2C repeater is used */
	createParam.tunerI2cConfig = SONY_DEMOD_TUNER_I2C_CONFIG_GATEWAY;  /* I2C gateway is used */
	demodI2c.i2c_type_id = pConfig->ext_dm_config.i2c_type_id;
    demodI2c.ReadRegister = nim_cxd2856_i2c_CommonReadRegister;
    demodI2c.WriteRegister = nim_cxd2856_i2c_CommonWriteRegister;
    demodI2c.WriteOneRegister = nim_cxd2856_i2c_CommonWriteOneRegister;
	CXD2856_LOG("%s %d i2c_id = %d i2c_addr = 0x%x\n",__FUNCTION__,__LINE__,demodI2c.i2c_type_id,createParam.i2cAddressSLVT);
	/* Create demodulator instance */
	if(sony_integ_Create(priv,pdemod,&createParam,&demodI2c,tunerTerrCable)!= SONY_RESULT_OK)
	{
		libc_printf("[%s %d]sony_integ_Create fail!\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}
	pdemod->state = SONY_DEMOD_STATE_ACTIVE;
	demodI2c.pDemod = pdemod;
	//Keep Ali config. It must be run after sony_dvb_demod_Create(),because sony_dvb_demod_Create() will empty all setting.
	MEMCPY((UINT8*)&(pdemod->dvbt_config), (UINT8*)pConfig, sizeof(struct COFDM_TUNER_CONFIG_API));


	/* Demod tunerOptimize member allows the demod to be optimized internally when connected to Sony RF parts. */
#if(SYS_TUN_MODULE == CXD2861 || SYS_TUN_MODULE == CXD2872)
	pdemod->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_SONYSILICON;	
#else
	pdemod->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_NONSONY;	
#endif
	dev->priv = (void*)priv;
	dev->init = NULL;
	dev->open = nim_cxd2856_open;
	dev->stop = nim_cxd2856_close;
	dev->channel_change = (void *)nim_cxd2856_channel_change; 
	dev->channel_search = NULL;
	dev->do_ioctl = nim_cxd2856_ioctl;
	dev->do_ioctl_ext = nim_cxd2856_ioctl_ext;
	dev->get_lock = nim_cxd2856_get_lock;
	dev->get_snr = nim_cxd2856_get_SQI;
	dev->get_agc = nim_cxd2856_get_SSI;
	dev->get_fftmode = nim_cxd2856_get_fftmode;
	dev->get_modulation = nim_cxd2856_get_modulation;
	dev->get_freq = nim_cxd2856_get_freq;
	dev->get_ber  = nim_cxd2856_get_ber;
	//dev->get_FEC = nim_cxd2856_get_FEC;
	pdemod->flag_id = OSAL_INVALID_ID;
	pdemod->i2c_mutex_id = OSAL_INVALID_ID;
	pdemod->demodMode_mutex_id = OSAL_INVALID_ID;
	pdemod->though_mutex = OSAL_INVALID_ID;

	pdemod->flag_id = osal_flag_create(NIM_FLAG_ENABLE);
	if (pdemod->flag_id==OSAL_INVALID_ID)
	{
		CXD2856_LOG( "%s: no more flag\r\n", __FUNCTION__);
		return ERR_ID_FULL;
	}

    pdemod->i2c_mutex_id = create_i2c_mutex(pdemod->dvbt_config.ext_dm_config.i2c_type_id);
    if(pdemod->i2c_mutex_id == OSAL_INVALID_ID)
    {
        CXD2856_LOG( "%s: no more mutex\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

	pdemod->demodMode_mutex_id = osal_mutex_create();
    if(pdemod->demodMode_mutex_id == OSAL_INVALID_ID)
    {
        CXD2856_LOG( "%s: creat demodMode_mutex_id error\r\n", __FUNCTION__);
    }

	pdemod->though_mutex = osal_semaphore_create(1);
	if (OSAL_INVALID_ID == pdemod->though_mutex)
	{
		CXD2856_LOG( "[%s %d]creat pdemod->though_mutex error!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}

    /* Add this device to queue */
    if(dev_register(dev) != SUCCESS)
    {
        CXD2856_LOG("Error: Register nim device error!\r\n");
        FREE(pdemod);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if(pdemod->dvbt_config.nim_tuner_init != NULL)
    {
    	//init tuner && get tuner_id from tuner
    	pdemod->tuner_id = 1;
		NIM_MUTEX_ENTER(pdemod);
    	if(pdemod->dvbt_config.nim_tuner_init(&(pdemod->tuner_id), &(pdemod->dvbt_config.tuner_config)) != SUCCESS)
        {
			libc_printf("[%s %d]Init Tuner Failure!\n", __FUNCTION__, __LINE__);
			NIM_MUTEX_LEAVE(pdemod);
            return ERR_NO_DEV;
        }
		NIM_MUTEX_LEAVE(pdemod);
		if(SONY_DEMOD_TUNER_I2C_CONFIG_GATEWAY == createParam.tunerI2cConfig)
		{
			ThroughMode.nim_dev_priv = pdemod;
	    	ThroughMode.dem_write_read_tuner = (INTERFACE_DEM_WRITE_READ_TUNER)nim_cxd2856_TunerGateway;
	    	nim_cxd2856_ioctl(dev, NIM_TUNER_SET_THROUGH_MODE, (UINT32)&ThroughMode);
		}
    }
    return SUCCESS;
}













