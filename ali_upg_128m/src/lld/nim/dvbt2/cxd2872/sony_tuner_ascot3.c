/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2013-01-29 12:48:35 #$
  File Revision : $Revision:: 6581 $
------------------------------------------------------------------------------*/
#include "sony_tuner_ascot3.h"

#include "sony_stdlib.h" /* for memcpy */
/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_ascot3_version =  SONY_ASCOT3_VERSION;

static sony_tuner_terr_cable_t pTuner;
static sony_ascot3_t pAscot3Tuner;
static sony_i2c_t pI2c;
static DEM_WRITE_READ_TUNER m_ThroughMode[MAX_TUNER_SUPPORT_NUM];
static struct COFDM_TUNER_CONFIG_EXT cxd2872_Config[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot3_Initialize (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot3_Tune (sony_tuner_terr_cable_t * pTuner,
                                             uint32_t frequency, 
                                             sony_dtv_system_t system, 
                                             sony_demod_bandwidth_t bandwidth);

static sony_result_t sony_tuner_ascot3_Shutdown (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot3_Sleep (sony_tuner_terr_cable_t * pTuner);

static sony_result_t cxd2872_GW_ReadRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size);
static sony_result_t cxd2872_GW_WriteRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t *pData, uint32_t size);
static sony_result_t cxd2872_GW_WriteOneRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data);
/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_ascot3_Create (/*sony_tuner_terr_cable_t * pTuner,*/
                                        uint32_t xtalFreq,  
                                        uint8_t i2cAddress,
                                        UINT32 i2c_type_id,
                                        uint32_t configFlags
                                        /*sony_ascot3_t * pAscot3Tuner*/)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Create");

    //if ((!pI2c) || (!pAscot3Tuner) || (!pTuner)) {
    //    SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    //}

	//tuner i2c interface setup
	pI2c.i2c_type_id = i2c_type_id;
	pI2c.ReadRegister = cxd2872_GW_ReadRegister;
	pI2c.WriteRegister = cxd2872_GW_WriteRegister;
	pI2c.WriteOneRegister = cxd2872_GW_WriteOneRegister;
	//pI2c.gwAddress = 0x00;
	//pI2c.gwSub = 0x00;      // N/A
	//pI2c.Read = sony_i2c_Read;
	//pI2c.Write = sony_i2c_Write;
	//pI2c.user = (void *) &useri2c;
	
    /* Create the underlying Ascot3 reference driver. */
    result = sony_ascot3_Create (&pAscot3Tuner, xtalFreq, i2cAddress, &pI2c, configFlags);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Create local copy of instance data. */
    pTuner.Initialize = sony_tuner_ascot3_Initialize;
    pTuner.Sleep = sony_tuner_ascot3_Sleep;
    pTuner.Shutdown = sony_tuner_ascot3_Shutdown;
    pTuner.Tune = sony_tuner_ascot3_Tune;
    pTuner.system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner.bandwidth = SONY_DEMOD_BW_UNKNOWN;  
    pTuner.frequencyKHz = 0;
    pTuner.i2cAddress = i2cAddress;
    pTuner.pI2c = &pI2c;
    pTuner.flags = configFlags;
    pTuner.user = &pAscot3Tuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_RFFilterConfig (sony_tuner_terr_cable_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_RFFilterConfig");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_RFFilterConfig (((sony_ascot3_t *) pTuner->user), coeff, offset);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_SetGPO (sony_tuner_terr_cable_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Write_GPIO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_SetGPO (((sony_ascot3_t *) pTuner->user), id, value);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_ReadRssi (sony_tuner_terr_cable_t * pTuner, int32_t * pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_ReadRssi");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_ReadRssi (((sony_ascot3_t *) pTuner->user), pRssi);

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot3_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Initialize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Initialize (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Tune (sony_tuner_terr_cable_t * pTuner,
                                             uint32_t frequencyKHz, 
                                             sony_dtv_system_t system, 
                                             sony_demod_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_ascot3_tv_system_t dtvSystem;

    SONY_TRACE_ENTER ("sony_tuner_ascot3_Tune");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Convert system, bandwidth into dtv system. */
    switch (system) {
    case SONY_DTV_SYSTEM_DVBC:
        switch (bandwidth) {
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBC_6;
            break;
        case SONY_DEMOD_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */ 
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBC_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth) {
        case SONY_DEMOD_BW_5_MHZ:
             dtvSystem = SONY_ASCOT3_DTV_DVBT_5;
            break;
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT_6;
            break;
        case SONY_DEMOD_BW_7_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT_7;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth) {
        case SONY_DEMOD_BW_1_7_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT2_1_7;
            break;
        case SONY_DEMOD_BW_5_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT2_5;
            break;
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT2_6;
            break;
        case SONY_DEMOD_BW_7_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT2_7;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBT2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth) {
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBC2_6;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT3_DTV_DVBC2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
            break;

    /* Intentional fall-through */
    case SONY_DTV_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Tune(((sony_ascot3_t *) pTuner->user), frequencyKHz, dtvSystem);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    result = sony_ascot3_TuneEnd((sony_ascot3_t *) pTuner->user);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = ((sony_ascot3_t *) pTuner->user)->frequencykHz;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Shutdown (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Sleep (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Sleep (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}


//*************************************************************************************
static BOOL run_on_through_mode(UINT32 tuner_id)
{
    return (m_ThroughMode[tuner_id].nim_dev_priv && m_ThroughMode[tuner_id].dem_write_read_tuner);
}

static sony_result_t cxd2872_GW_WriteOneRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
	UINT32 status = ERR_FAILUE;
	UINT8 Cmd[2];
	Cmd[0] = subAddress;
	Cmd[1] = data;

	UINT32 tuner_id = pI2c->tuner_id;
	if (run_on_through_mode(tuner_id))
	{
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, pTuner.i2cAddress, Cmd, 2, 0, 0);
	}
	else
	{
        status = i2c_write(cxd2872_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Cmd, 2);
	}
    return (status==SUCCESS ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);
}

static sony_result_t cxd2872_GW_WriteRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t *pData, uint32_t size)
{
	UINT32 status = ERR_FAILUE;
	UINT8 wLen=0;
	wLen = size+1;
	UINT8 Cmd[wLen];
	Cmd[0] = subAddress;
	//Cmd[1] = RegData;
	sony_memcpy(&(Cmd[1]), pData, size);

	UINT32 tuner_id = pI2c->tuner_id;
	if (run_on_through_mode(tuner_id))
	{
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, pTuner.i2cAddress, Cmd, wLen, 0, 0);
	}
	else
	{
        status = i2c_write(cxd2872_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Cmd, wLen);
	}
    return (status==SUCCESS ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);
}

static sony_result_t cxd2872_GW_ReadRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
	UINT32 status = ERR_FAILUE;

	//UINT8 rLen;
	//rLen = size+1;
	UINT8 Read_Cmd[size];

	/* read step 1. accroding to mxl301 driver API user guide. */

	#if 0
	Read_Cmd[0] = 0xFB;
	Read_Cmd[1] = subAddress;
	#else
	Read_Cmd[0] = subAddress;
	#endif

	//sony_memcpy(&(Cmd[1]), pData, size);

	UINT32 tuner_id = pI2c->tuner_id;
	if (run_on_through_mode(tuner_id))
	{
		//status = m_ThroughMode[tuner_id].Dem_Write_Read_Tuner(m_ThroughMode[tuner_id].nim_dev_priv,  pTuner.i2cAddress, Read_Cmd, 2, 0, 0);
		status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv,  pTuner.i2cAddress, Read_Cmd, 1, 0, 0);

		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
		status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, pTuner.i2cAddress, 0, 0, pData, size);
		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
	}
	else
	{
		//status = i2c_write(cxd2872_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Read_Cmd, 2);
		status = i2c_write(cxd2872_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Read_Cmd, 1);
		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
		status = i2c_read(cxd2872_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, pData, size);
		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
	}
	return SONY_RESULT_OK;
	//return status;
}


static INT32 tun_cxd2872_set_through_mode(UINT32 tuner_id, DEM_WRITE_READ_TUNER *ThroughMode)
{
	if(tuner_id >= tuner_cnt)
	{
		return ERR_FAILUE;
	} 
  	sony_memcpy((UINT8*)(&m_ThroughMode[tuner_id]), (UINT8*)ThroughMode, sizeof(DEM_WRITE_READ_TUNER));
	return SUCCESS;
}

static INT32 tun_cxd2872_powcontrol(UINT32 tuner_id, UINT8 stdby)
{
	sony_result_t status;

	if(tuner_id >= tuner_cnt)
	{
		return ERR_FAILUE;
	}
	if (stdby)
	{
		//start sleep mode
		//status = sony_tuner_ascot3_Sleep(&pTuner);
		status = sony_tuner_ascot3_Shutdown(&pTuner);
		if (status != SONY_RESULT_OK)
		{
			return ERR_FAILUE;
		}
	}
	else
	{
		//PRE-TUNE:start wakeup mode!
		status = sony_tuner_ascot3_Initialize (&pTuner);


		if (status != SONY_RESULT_OK)
		{
			return ERR_FAILUE;
		}
	}
	return SUCCESS;
}

static INT32 tun_cxd2872_get_rf_level(UINT32 tuner_id, UINT32 *pRFLeveldB)
{
#if 1
	int32_t Rssi;

	sony_result_t result = SONY_RESULT_OK;
	result = sony_tuner_ascot3_ReadRssi(&pTuner,&Rssi);
		
	if (result != SONY_RESULT_OK) 
	{
		*pRFLeveldB = 0;
		SONY_TRACE_RETURN (result);
	}

       /* RF Level dBm = RSSI + IFOUT
        * IFOUT is the target IF level for tuner, -4.0dBm
      */
	*pRFLeveldB = 10 * (Rssi - 400);
	*pRFLeveldB += 3000;
				
#else
	{
		uint32_t ifAgc;
		int32_t ifGain;
		int32_t rfGain;

		if (pInteg->pTunerTerrCable) 
		{
			if (pInteg->pDemod->system == SONY_DTV_SYSTEM_DVBT) {
				result = sony_demod_dvbt_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
			}
			else {
				result = sony_demod_dvbt2_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
			}

			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			}

			/* Protect against overflow. IFAGC is unsigned 12-bit. */
			if (ifAgc > 0xFFF) {
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
			}

			result = sony_tuner_ascot3_CalcGainFromAGC(pInteg->pTunerTerrCable, ifAgc, &ifGain, &rfGain);
			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			}

			/* RF Level dBm = IFOUT - IFGAIN - RFGAIN
			* IFOUT is the target IF level for tuner, -4.0dBm
			*/
			*pRFLeveldB = 10 * (-400 - ifGain - rfGain);

			/* Note : An implementation specific offset may be required
			* to compensate for component gains / attenuations */
		}
		else {
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
		}
	}
#endif
}


INT32 tun_cxd_ascot3_Init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{

	if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
		return SONY_RESULT_ERROR_ARG;
	
	sony_memcpy(&cxd2872_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
	m_ThroughMode[tuner_cnt].dem_write_read_tuner = NULL;

	sony_result_t result = SONY_RESULT_OK;

	/* Create tuner instance */
	UINT32 xtalFreq;
	if(ptrTuner_Config->c_tuner_crystal == 16)
		xtalFreq = SONY_ASCOT3_XTAL_16000KHz;
	else if(ptrTuner_Config->c_tuner_crystal == 20)
		xtalFreq = SONY_ASCOT3_XTAL_20500KHz;
	else if(ptrTuner_Config->c_tuner_crystal == 24)
		xtalFreq = SONY_ASCOT3_XTAL_24000KHz;
	else if(ptrTuner_Config->c_tuner_crystal == 41)
		xtalFreq = SONY_ASCOT3_XTAL_41000KHz;	
		
	result = sony_tuner_ascot3_Create(xtalFreq, ptrTuner_Config->c_tuner_base_addr, ptrTuner_Config->i2c_type_id,
		SONY_ASCOT3_CONFIG_IFAGCSEL_ALL2|SONY_ASCOT3_CONFIG_SLEEP_DISABLEXTAL|SONY_ASCOT3_CONFIG_LOOPTHRU_ENABLE);
	if(result !=	SONY_RESULT_OK)
	{
		return ERR_FAILUE;
		libc_printf("tun_cxd_ascot3_Init:step1 create error!\n");
	}

	if(tuner_id)
	{
		*tuner_id = tuner_cnt;
		pI2c.tuner_id = tuner_cnt;
	}
	tuner_cnt++;

	return SUCCESS;
}

INT32 tun_cxd_ascot3_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
	sony_result_t result;
	sony_dtv_system_t system = (sony_dtv_system_t)(*data);
	result = sony_tuner_ascot3_Tune(&pTuner, freq, system, bandwidth);
	if(SONY_RESULT_OK == result)
		return SUCCESS;
	return ERR_FAILUE;
}

INT32 tun_cxd_ascot3_status(UINT32 tuner_id, UINT8 *lock)
{
	//not support by tuner cxd2872
	*lock = 1;
	return SUCCESS;
}

INT32 tun_cxd_ascot3_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
	
    switch(cmd)
    {
        case NIM_TUNER_SET_THROUGH_MODE:
            ret = tun_cxd2872_set_through_mode(tuner_id, (DEM_WRITE_READ_TUNER *)param);
            break;

        case NIM_TUNER_POWER_CONTROL:
            ret = tun_cxd2872_powcontrol(tuner_id, param);
            break;

		case NIM_TUNER_GET_AGC:
			ret = tun_cxd2872_get_rf_level(tuner_id, (UINT32 *)param);
			break;

		case NIM_TUNER_GET_RF_POWER_LEVEL:
			//ret = MxLWare603_API_ReqTunerRxPower(tuner_id, (REAL32*)param);
			ret = tun_cxd2872_get_rf_level(tuner_id, (UINT32 *)param);
			break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}



