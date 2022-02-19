/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "st_lnbh29.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t st_lnbh29_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t st_lnbh29_SetConfig (sony_lnbc_t * pLnbc,
                                          sony_lnbc_config_id_t configId,
                                          int32_t value);

static sony_result_t st_lnbh29_SetVoltage (sony_lnbc_t * pLnbc,
                                           sony_lnbc_voltage_t voltage);

static sony_result_t st_lnbh29_SetTone (sony_lnbc_t * pLnbc,
                                        sony_lnbc_tone_t tone);

static sony_result_t st_lnbh29_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                sony_lnbc_transmit_mode_t mode);

static sony_result_t st_lnbh29_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t st_lnbh29_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t create_data (sony_lnbc_t * pLnbc,
                                  sony_lnbc_voltage_t voltage,
                                  uint8_t * pData);
/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t st_lnbh29_Create (sony_lnbc_t * pLnbc,
                                uint8_t i2cAddress,
                                sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("st_lnbh29_Create");

    if ((!pI2c) || (!pLnbc)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = st_lnbh29_Initialize;
    pLnbc->SetConfig = st_lnbh29_SetConfig;
    pLnbc->SetVoltage = st_lnbh29_SetVoltage;
    pLnbc->SetTone = st_lnbh29_SetTone;
    pLnbc->SetTransmitMode = st_lnbh29_SetTransmitMode;
    pLnbc->Sleep = st_lnbh29_Sleep;
    pLnbc->WakeUp = st_lnbh29_WakeUp;

    pLnbc->isInternalTone = 1; /* Fixed to 1 */
    pLnbc->lowVoltage = ST_LNBH29_CONFIG_VOLTAGE_LOW_13_0V; /* Default value */
    pLnbc->highVoltage = ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_150V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_Initialize (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh29_Initialize");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->isInternalTone = 1; /* Fixed to 1 */
    pLnbc->lowVoltage = ST_LNBH29_CONFIG_VOLTAGE_LOW_13_0V; /* Default value */
    pLnbc->highVoltage = ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_150V; /* Default value */

    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_SetConfig (sony_lnbc_t * pLnbc,
                                          sony_lnbc_config_id_t configId,
                                          int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("st_lnbh29_SetConfig");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(configId)
    {
    case SONY_LNBC_CONFIG_ID_TONE_INTERNAL:
        if ((value != 0) && (value != 1)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->isInternalTone = value;
            result = st_lnbh29_SetTone (pLnbc, pLnbc->tone);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != ST_LNBH29_CONFIG_VOLTAGE_LOW_13_0V) &&
            (value != ST_LNBH29_CONFIG_VOLTAGE_LOW_13_667V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = st_lnbh29_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_150V) &&
            (value != ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_817V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = st_lnbh29_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_SetVoltage (sony_lnbc_t * pLnbc,
                                           sony_lnbc_voltage_t voltage)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("st_lnbh29_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_data (pLnbc, voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_SetTone (sony_lnbc_t * pLnbc,
                                        sony_lnbc_tone_t tone)
{
    SONY_TRACE_ENTER ("st_lnbh29_SetTone");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(tone)
    {
    case SONY_LNBC_TONE_ON:
    case SONY_LNBC_TONE_OFF:
        /* ST LNBH29 doesn't support this function. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        break;

    case SONY_LNBC_TONE_AUTO:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                sony_lnbc_transmit_mode_t mode)
{
    SONY_TRACE_ENTER ("st_lnbh29_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(mode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        /* Do nothing */
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_Sleep (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh29_Sleep");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Continue */
        break;

    case SONY_LNBC_STATE_SLEEP:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh29_WakeUp (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("st_lnbh29_WakeUp");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_SLEEP:
        /* Continue */
        break;

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_data (pLnbc, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_data (sony_lnbc_t * pLnbc,
                                  sony_lnbc_voltage_t voltage,
                                  uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_data");

    if ((!pLnbc) || (!pData)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    *pData = 0;

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
        switch(pLnbc->lowVoltage)
        {
        case ST_LNBH29_CONFIG_VOLTAGE_LOW_13_0V:
            /* 13.0V */
            *pData |= 0x01;
            break;

        case ST_LNBH29_CONFIG_VOLTAGE_LOW_13_667V:
            /* 13.667V */
            *pData |= 0x03;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    case SONY_LNBC_VOLTAGE_HIGH:
        switch(pLnbc->highVoltage)
        {
        case ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_150V:
            /* 18.150V */
            *pData |= 0x05;
            break;

        case ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_817V:
            /* 18.817V */
            *pData |= 0x07;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
