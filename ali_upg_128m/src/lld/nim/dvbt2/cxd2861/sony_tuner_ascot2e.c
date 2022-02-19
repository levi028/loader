/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2013-02-21 16:53:18 #$
  File Revision : $Revision:: 6676 $
------------------------------------------------------------------------------*/
#include "sony_tuner_ascot2e.h"

#include "sony_stdlib.h" /* for memcpy */

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_ascot2e_version =  SONY_ASCOT2E_VERSION;

static sony_tuner_terr_cable_t pTuner;
static sony_ascot2e_t pAscot2ETuner;
static sony_i2c_t pI2c;
static DEM_WRITE_READ_TUNER m_ThroughMode[MAX_TUNER_SUPPORT_NUM];
static struct COFDM_TUNER_CONFIG_EXT cxd2861_Config[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;


/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot2e_Initialize (sony_tuner_terr_cable_t * pTuner);
//static sony_result_t sony_tuner_ascot2e_Initialize (UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);


static sony_result_t sony_tuner_ascot2e_Tune (sony_tuner_terr_cable_t * pTuner, uint32_t frequency, sony_dtv_system_t system, sony_demod_bandwidth_t bandwidth);

static sony_result_t sony_tuner_ascot2e_Shutdown (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot2e_Sleep (sony_tuner_terr_cable_t * pTuner);

static sony_result_t cxd2861_GW_ReadRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size);
static sony_result_t cxd2861_GW_WriteRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t *pData, uint32_t size);
static sony_result_t cxd2861_GW_WriteOneRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_ascot2e_Create (/*sony_tuner_terr_cable_t * pTuner,*/
                                         uint32_t xtalFreqMHz,                                     
                                         uint8_t i2cAddress,
                                         UINT32 i2c_type_id,
                                         /*sony_i2c_t * pI2c,*/
                                         uint32_t configFlags
                                         /*sony_ascot2e_t * pAscot2ETuner*/)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Create");

//    if ((!pI2c) || (!pAscot2ETuner) || (!pTuner)) {
//        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
//    }

//tuner i2c interface setup
	pI2c.i2c_type_id = i2c_type_id;
	pI2c.ReadRegister = cxd2861_GW_ReadRegister;
	pI2c.WriteRegister = cxd2861_GW_WriteRegister;
	pI2c.WriteOneRegister = cxd2861_GW_WriteOneRegister;
	//pI2c.gwAddress = 0x00;
	//pI2c.gwSub = 0x00;      // N/A
	//pI2c.Read = sony_i2c_Read;
	//pI2c.Write = sony_i2c_Write;
	//pI2c.user = (void *) &useri2c;

    /* Create the underlying Ascot2E reference driver. */
    result = sony_ascot2e_Create (&pAscot2ETuner, xtalFreqMHz, i2cAddress, &pI2c, configFlags);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Create local copy of instance data. */
    pTuner.Initialize = sony_tuner_ascot2e_Initialize;
    pTuner.Sleep = sony_tuner_ascot2e_Sleep;
    pTuner.Shutdown = sony_tuner_ascot2e_Shutdown;
    pTuner.Tune = sony_tuner_ascot2e_Tune;
    pTuner.system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner.bandwidth = SONY_DEMOD_BW_UNKNOWN;  
    pTuner.frequencyKHz = 0;
    pTuner.i2cAddress = i2cAddress;
    pTuner.pI2c = &pI2c;
    pTuner.flags = configFlags;
    pTuner.user = &pAscot2ETuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot2e_RFFilterConfig (sony_tuner_terr_cable_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_RFFilterConfig");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot2e_RFFilterConfig (((sony_ascot2e_t *) pTuner->user), coeff, offset);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot2e_SetGPO (sony_tuner_terr_cable_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Write_GPIO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot2e_SetGPO (((sony_ascot2e_t *) pTuner->user), id, value);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot2e_ReadGain (sony_tuner_terr_cable_t * pTuner, int32_t * pIFGain, int32_t * pRFGain, uint8_t forceRFAGCRead)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_ReadGain");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot2e_ReadGain (((sony_ascot2e_t *) pTuner->user), pIFGain, pRFGain, forceRFAGCRead);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot2e_CalcGainFromAGC (sony_tuner_terr_cable_t * pTuner, uint32_t agcReg, int32_t * pIFGain, int32_t * pRFGain)
{
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_CalcGainFromAGC");

    if (!pTuner || !pIFGain || !pRFGain) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
        IFGAIN = if(AGC > 0.6){
                     6 + IF_BPF_GC 
                 }else{
                     if(AGC > 0.19){
                         6 + IF_BPF_GC + 69 * (0.6 - AGC)
                     }else{
                         6 + IF_BPF_GC + 69 * 0.41
                     }
                 }

        Note that AGC(V) = DemodAGCReg * 2.27 / 4096
        So...
        IFGAIN(100xdB) = if(DemodAGCReg * 227 > 245760){
                             (6 + IF_BPF_GC) * 100 
                         }else if(DemodAGCReg * 227 > 77824){
                             (6 + IF_BPF_GC) * 100 + (69 * (245760 - DemodAGCReg * 227))/4096
                         }else{
                             (6 + IF_BPF_GC) * 100 + 69 * 41
                         }
    */
    {
        int8_t if_bpf_gc = 0;
        int32_t agcRegX = (int32_t)agcReg * 227;

        switch(pTuner->system){
        case SONY_DTV_SYSTEM_DVBT:
        case SONY_DTV_SYSTEM_DVBT2:
            if_bpf_gc = 12;
            break;
        case SONY_DTV_SYSTEM_DVBC:
            if_bpf_gc = 4;
            break;
        case SONY_DTV_SYSTEM_DVBC2:
            if_bpf_gc = 0;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }

        if(agcRegX > 245760){
            *pIFGain = (6 + if_bpf_gc) * 100;
        }else if(agcRegX > 77824){
            *pIFGain = (6 + if_bpf_gc) * 100 + (69 * (245760 - agcRegX) + 2048) / 4096; /* Round */
        }else{
            *pIFGain = (6 + if_bpf_gc) * 100 + 69 * 41;
        }
    }

    /*
        RFGAIN = if(AGC < 0.42){
                     RFGAIN_MAX
                 }else if(AGC < 0.49){
                     RF_GAIN_MAX - 63 * (AGC - 0.42)
                 }else if(AGC < 0.6){
                     RF_GAIN_MAX - 63 * 0.07
                 }else if(AGC < 1.13){
                     RF_GAIN_MAX - 63 * 0.07 - 63 * (AGC - 0.6)
                 }else if(AGC < 1.38){
                     RF_GAIN_MAX - 63 * 0.6 - 45 * (AGC - 1.13)
                 }else{
                     RF_GAIN_MAX - 63 * 0.6 - 45 * 0.25 + RFGRAD * (AGC - 1.38)
                 }

        Note that AGC(V) = DemodAGCReg * 2.27 / 4096

        So...
        RFGAIN(100xdB) = if(DemodAGCReg * 227 < 172032){
                             RFGAIN_MAX * 100
                         }else if(DemodAGCReg * 227 < 200704){
                             RFGAIN_MAX * 100 - (63 * (DemodAGCReg * 227 - 172032))/4096
                         }else if(DemodAGCReg * 227 < 245760){
                             RFGAIN_MAX * 100 - 63 * 7
                         }else if(DemodAGCReg * 227 < 462848){
                             RFGAIN_MAX * 100 - 63 * 7 - (63 * (DemodAGCReg * 227 - 245760))/4096
                         }else if(DemodAGCReg * 227 < 565248){
                             RFGAIN_MAX * 100 - 63 * 60 - (45 * (DemodAGCReg * 227 - 462848))/4096
                         }else{
                             RFGAIN_MAX * 100 - 63 * 60 - 45 * 25 + RFGRAD * (DemodAGCReg * 227 - 565248))/4096
                         }
    */
    {
        int32_t agcRegX = (int32_t)agcReg * 227;
        int32_t rfgainmax_x100 = 0;

        if(pTuner->frequencyKHz > 900000){
            rfgainmax_x100 = 4320;
        }else if(pTuner->frequencyKHz > 700000){
            rfgainmax_x100 = 4420;
        }else if(pTuner->frequencyKHz > 600000){
            rfgainmax_x100 = 4330;
        }else if(pTuner->frequencyKHz > 504000){
            rfgainmax_x100 = 4160;
        }else if(pTuner->frequencyKHz > 400000){
            rfgainmax_x100 = 4550;
        }else if(pTuner->frequencyKHz > 320000){
            rfgainmax_x100 = 4400;
        }else if(pTuner->frequencyKHz > 270000){
            rfgainmax_x100 = 4520;
        }else if(pTuner->frequencyKHz > 235000){
            rfgainmax_x100 = 4370;
        }else if(pTuner->frequencyKHz > 192000){
            rfgainmax_x100 = 4190;
        }else if(pTuner->frequencyKHz > 130000){
            rfgainmax_x100 = 4550;
        }else if(pTuner->frequencyKHz > 86000){
            rfgainmax_x100 = 4630;
        }else if(pTuner->frequencyKHz > 50000){
            rfgainmax_x100 = 4350;
        }else{
            rfgainmax_x100 = 4450;
        }

        if(agcRegX < 172032){
            *pRFGain = rfgainmax_x100;
        }else if(agcRegX < 200704){
            *pRFGain = rfgainmax_x100 - (63 * (agcRegX - 172032) + 2048) / 4096; /* Round */
        }else if(agcRegX < 245760){
            *pRFGain = rfgainmax_x100 - 63 * 7;
        }else if(agcRegX < 462848){
            *pRFGain = rfgainmax_x100 - 63 * 7 - (63 * (agcRegX - 245760) + 2048) / 4096; /* Round */
        }else if(agcRegX < 565248){
            *pRFGain = rfgainmax_x100 - 63 * 60 - (45 * (agcRegX - 462848) + 2048) / 4096; /* Round */
        }else{
            int32_t rfgrad = -95;

            *pRFGain = rfgainmax_x100 - 63 * 60 - 45 * 25 + (rfgrad * (agcRegX - 565248) - 2048) / 4096; /* Round */
        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot2e_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Initialize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2e_Initialize (((sony_ascot2e_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot2e_Tune(sony_tuner_terr_cable_t * pTuner, uint32_t centerFrequencyKHz, sony_dtv_system_t system, sony_demod_bandwidth_t bandwidth)
{
	sony_result_t result = SONY_RESULT_OK;
	sony_ascot2e_tv_system_t dtvSystem;

    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Tune");

    if (!pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

 /* Call into underlying driver. Convert system, bandwidth into dtv system. */
    switch (system) {
    case SONY_DTV_SYSTEM_DVBC:
        dtvSystem = SONY_ASCOT2E_DTV_DVBC;
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth) {
        case SONY_DEMOD_BW_5_MHZ:
             dtvSystem = SONY_ASCOT2E_DTV_DVBT_5;
            break;
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT_6;
            break;
        case SONY_DEMOD_BW_7_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT_7;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth) {
        case SONY_DEMOD_BW_1_7_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT2_1_7;
            break;
        case SONY_DEMOD_BW_5_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT2_5;
            break;
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT2_6;
            break;
        case SONY_DEMOD_BW_7_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT2_7;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBT2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth) {
        case SONY_DEMOD_BW_6_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBC2_6;
            break;
        case SONY_DEMOD_BW_8_MHZ:
            dtvSystem = SONY_ASCOT2E_DTV_DVBC2_8;
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

    result = sony_ascot2e_Tune(((sony_ascot2e_t *) pTuner->user), centerFrequencyKHz, dtvSystem);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    result = sony_ascot2e_TuneEnd((sony_ascot2e_t *) pTuner->user);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = ((sony_ascot2e_t *) pTuner->user)->frequencykHz;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot2e_Shutdown (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2e_Sleep (((sony_ascot2e_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot2e_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot2e_Sleep (((sony_ascot2e_t *) pTuner->user));

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

static sony_result_t cxd2861_GW_WriteOneRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
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
        status = i2c_write(cxd2861_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Cmd, 2);
	}
    return (status==SUCCESS ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);
}

static sony_result_t cxd2861_GW_WriteRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t *pData, uint32_t size)
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
        status = i2c_write(cxd2861_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Cmd, wLen);
	}
    return (status==SUCCESS ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);
}

static sony_result_t cxd2861_GW_ReadRegister(struct sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
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
		//status = i2c_write(cxd2861_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Read_Cmd, 2);
		status = i2c_write(cxd2861_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, Read_Cmd, 1);
		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
		status = i2c_read(cxd2861_Config[tuner_id].i2c_type_id, pTuner.i2cAddress, pData, size);
		if(status != SUCCESS)
		{
			return SONY_RESULT_ERROR_I2C;
		}
	}
	return SONY_RESULT_OK;
	//return status;
}


static INT32 tun_cxd2861_set_through_mode(UINT32 tuner_id, DEM_WRITE_READ_TUNER *ThroughMode)
{
	if(tuner_id >= tuner_cnt)
	{
		return ERR_FAILUE;
	} 
  	sony_memcpy((UINT8*)(&m_ThroughMode[tuner_id]), (UINT8*)ThroughMode, sizeof(DEM_WRITE_READ_TUNER));
	return SUCCESS;
}

static INT32 tun_cxd2861_powcontrol(UINT32 tuner_id, UINT8 stdby)
{
	sony_result_t status;

	if(tuner_id >= tuner_cnt)
	{
		return ERR_FAILUE;
	}
	if (stdby)
	{
		//start sleep mode
		//status = sony_tuner_ascot2e_Sleep(&pTuner);
		status = sony_tuner_ascot2e_Shutdown(&pTuner);
		if (status != SONY_RESULT_OK)
		{
			return ERR_FAILUE;
		}
	}
	else
	{
		//PRE-TUNE:start wakeup mode!
		status = sony_tuner_ascot2e_Initialize (&pTuner);


		if (status != SONY_RESULT_OK)
		{
			return ERR_FAILUE;
		}
	}
	return SUCCESS;
}

static INT32 tun_cxd2861_get_rf_level(UINT32 tuner_id, UINT32 *pRFLeveldB)
{
#if 1
	//uint32_t ifAgc;
	int32_t ifGain;
	int32_t rfGain;

	sony_result_t result = SONY_RESULT_OK;
	//result = sony_tuner_ascot2e_CalcGainFromAGC(&pTuner, ifAgc, &ifGain, &rfGain);
	result = sony_tuner_ascot2e_ReadGain(&pTuner, &ifGain, &rfGain, 0);
	if (result != SONY_RESULT_OK) 
	{
		*pRFLeveldB = 0;
		SONY_TRACE_RETURN (result);
	}

	/* RF Level dBm = IFOUT - IFGAIN - RFGAIN
	* IFOUT is the target IF level for tuner, -4.0dBm
	*/
	*pRFLeveldB = 10 * (-400 - ifGain - rfGain);

	/*
	if (*pRFLeveldB > 255)
	{
		*pRFLeveldB = 255;
	}
	else if (*pRFLeveldB < 0)
	{
		*pRFLeveldB = 0;
	}*/
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

			result = sony_tuner_ascot2e_CalcGainFromAGC(pInteg->pTunerTerrCable, ifAgc, &ifGain, &rfGain);
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


INT32 tun_cxd_ascot2e_Init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{

	if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
		return SONY_RESULT_ERROR_ARG;
	
	sony_memcpy(&cxd2861_Config[tuner_cnt], ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
	m_ThroughMode[tuner_cnt].dem_write_read_tuner = NULL;

	sony_result_t result = SONY_RESULT_OK;

	/* Create tuner instance */
	result = sony_tuner_ascot2e_Create(ptrTuner_Config->c_tuner_crystal, ptrTuner_Config->c_tuner_base_addr, ptrTuner_Config->i2c_type_id, SONY_ASCOT2E_CONFIG_IFAGCSEL_ALL1 );//SONY_ASCOT2E_CONFIG_IFAGCSEL_D1A2SONY_ASCOT2E_CONFIG_IFAGCSEL_A1D2
	if(result !=	SONY_RESULT_OK)
	{
		return ERR_FAILUE;
		libc_printf("tun_cxd_ascot2e_Init:step1 create error!\n");
	}

	if(tuner_id)
	{
		*tuner_id = tuner_cnt;
		pI2c.tuner_id = tuner_cnt;
	}
	tuner_cnt++;

	return SUCCESS;
}

INT32 tun_cxd_ascot2e_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
	sony_result_t result;
	sony_dtv_system_t system = (sony_dtv_system_t)(*data);
	result = sony_tuner_ascot2e_Tune(&pTuner, freq, system, bandwidth);
	if(SONY_RESULT_OK == result)
		return SUCCESS;
	return ERR_FAILUE;
}

INT32 tun_cxd_ascot2e_status(UINT32 tuner_id, UINT8 *lock)
{
	//not support by tuner cxd2861
	*lock = 1;
	return SUCCESS;
}

INT32 tun_cxd_ascot2e_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
	
    switch(cmd)
    {
        case NIM_TUNER_SET_THROUGH_MODE:
            ret = tun_cxd2861_set_through_mode(tuner_id, (DEM_WRITE_READ_TUNER *)param);
            break;

        case NIM_TUNER_POWER_CONTROL:
            ret = tun_cxd2861_powcontrol(tuner_id, param);
            break;

		case NIM_TUNER_GET_AGC:
			ret = tun_cxd2861_get_rf_level(tuner_id, (UINT32 *)param);
			break;

		case NIM_TUNER_GET_RF_POWER_LEVEL:
			//ret = MxLWare603_API_ReqTunerRxPower(tuner_id, (REAL32*)param);
			ret = tun_cxd2861_get_rf_level(tuner_id, (UINT32 *)param);
			break;

        default:
            ret = ERR_FAILUE;
            break;
    }

    return ret;
}



