/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_lnbc.h

          This file provides the LNB controller control interface.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_LNBC_H
#define SONY_LNBC_H

#include "sony_common.h"
#include "sony_i2c.h"

/**
 @brief Definition of the state of the driver.
*/
typedef enum {
    SONY_LNBC_STATE_UNKNOWN,    /**< Unknown */
    SONY_LNBC_STATE_SLEEP,      /**< Sleep */
    SONY_LNBC_STATE_ACTIVE      /**< Active */
} sony_lnbc_state_t;

/**
 @brief Definition of the voltage.
*/
typedef enum {
    SONY_LNBC_VOLTAGE_LOW,      /**< Low voltage. */
    SONY_LNBC_VOLTAGE_HIGH,     /**< High voltage. */
    SONY_LNBC_VOLTAGE_AUTO,     /**< Control voltage by demodulator. (For single cable) */
} sony_lnbc_voltage_t;

/**
 @brief Definition of the tone state.
*/
typedef enum {
    SONY_LNBC_TONE_OFF,         /**< Not output tone signal. */
    SONY_LNBC_TONE_ON,          /**< Output tone signal. */
    SONY_LNBC_TONE_AUTO,        /**< Control tone by demodulator. (Default) */
} sony_lnbc_tone_t;

/**
 @brief Transmit mode.
*/
typedef enum {
    SONY_LNBC_TRANSMIT_MODE_TX,     /**< TX mode. */
    SONY_LNBC_TRANSMIT_MODE_RX,     /**< RX mode. */
    SONY_LNBC_TRANSMIT_MODE_AUTO,   /**< Control transmit mode by demodulator. (For receiving DiSEqC2.x reply) */
} sony_lnbc_transmit_mode_t;

/**
 @brief Config ID for SetConfig API.
*/
typedef enum {
    SONY_LNBC_CONFIG_ID_TONE_INTERNAL,  /**< Tone mode (0: External tone, 1: Internal tone). */
    SONY_LNBC_CONFIG_ID_LOW_VOLTAGE,    /**< Definition of voltage for "Low voltage". */
    SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE    /**< Definition of voltage for "High voltage". */
} sony_lnbc_config_id_t;

/**
 @brief Definition of the LNB controller driver API.
*/
typedef struct sony_lnbc_t {
    uint8_t i2cAddress;                     /**< I2C address. */
    sony_i2c_t * pI2c;                      /**< I2C driver instance. */
    sony_lnbc_state_t state;                /**< The state of this driver */
    sony_lnbc_voltage_t voltage;            /**< Current voltage. */
    sony_lnbc_tone_t tone;                  /**< Current tone status. */
    sony_lnbc_transmit_mode_t transmitMode; /**< Current transmit mode. */

    int32_t isInternalTone;                 /**< Tone mode flag.(0: External tone, 1: Internal tone) */
    int32_t lowVoltage;                     /**< Definition of low voltage. */
    int32_t highVoltage;                    /**< Definition of high voltage. */

    /**
     @brief Initialize the LNB controller.

     When user call this API, the LNB controller's state is following.
       - Voltage      : Low
       - Tone         : Auto (Control tone by demodulator)
       - TransmitMode : TX mode

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Initialize) (struct sony_lnbc_t * pLnbc);

    /**
     @brief Configure to LNB controller

     The parameters set by this API are initialized to default value by Initialize API.

     @param pLnbc Instance of the LNB controller driver.
     @param configId Configure parameter ID.
     @param value The parameter value.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetConfig) (struct sony_lnbc_t * pLnbc, sony_lnbc_config_id_t configId, int32_t value);

    /**
     @brief Set voltage.

     @param pLnbc Instance of the LNB controller driver.
     @param voltage Voltage to set.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetVoltage) (struct sony_lnbc_t * pLnbc, sony_lnbc_voltage_t voltage);

    /**
     @brief Set tone mode.

     @param pLnbc Instance of the LNB controller driver.
     @param tone Tone mode.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetTone) (struct sony_lnbc_t * pLnbc, sony_lnbc_tone_t tone);

    /**
     @brief Set transmit mode.

     @param pLnbc Instance of the LNB controller driver.
     @param mode Transmit mode.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetTransmitMode) (struct sony_lnbc_t * pLnbc, sony_lnbc_transmit_mode_t mode);

    /**
     @brief Sleep the LNB controller.

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Sleep) (struct sony_lnbc_t * pLnbc);

    /**
     @brief Return from sleep the LNB controller.

     When user call this API, the LNB controller's state is kept before sleep.

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*WakeUp) (struct sony_lnbc_t * pLnbc);

    /**
     @brief User defined data.
    */
    void * user;

} sony_lnbc_t;

#endif /* SONY_LNBC_H */
