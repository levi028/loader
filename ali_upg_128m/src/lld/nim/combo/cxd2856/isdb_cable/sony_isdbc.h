/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_isdbc.h

          This file provides ISDB-C related type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_ISDBC_H
#define SONY_ISDBC_H

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief Type of TSID.
*/
typedef enum {
    SONY_ISDBC_TSID_TYPE_TSID,               /**< TSID(16bit) */
    SONY_ISDBC_TSID_TYPE_RELATIVE_TS_NUMBER  /**< Relative TS number */
} sony_isdbc_tsid_type_t;

/**
 @brief ISDB-C constellation.
*/
typedef enum {
    SONY_ISDBC_CONSTELLATION_RESERVED_0,     /**< RESERVED */
    SONY_ISDBC_CONSTELLATION_RESERVED_1,     /**< RESERVED */
    SONY_ISDBC_CONSTELLATION_64QAM,          /**< 64-QAM */
    SONY_ISDBC_CONSTELLATION_RESERVED_3,     /**< RESERVED */
    SONY_ISDBC_CONSTELLATION_256QAM          /**< 256-QAM */
} sony_isdbc_constellation_t;

/**
 @brief TSMF receive status
*/
typedef enum {
    SONY_ISDBC_TSMF_RECEIVE_STATUS_GOOD,            /**< Receive status is good */
    SONY_ISDBC_TSMF_RECEIVE_STATUS_NORMAL,          /**< Receive status is normal */
    SONY_ISDBC_TSMF_RECEIVE_STATUS_BAD,             /**< Receive status is bad */
    SONY_ISDBC_TSMF_RECEIVE_STATUS_UNDEFINED        /**< Receive status is undefined */
} sony_isdbc_tsmf_receive_status_t;

/**
 @brief TSMF Header information structure.
*/
typedef struct sony_isdbc_tsmf_header_t {
    /**
     @brief TSMF Synchronization error.

            The following cases, Synchronization error will occur.
            - The header can not be detected after 53 slot from the previous header detected.
            - The selected TSID(Relative TS number) when the sony_demod_isdbc_Tune() called does not exist.
    */
    uint8_t syncError;
    uint8_t versionNumber;                               /**< Version number*/
    uint8_t tsNumMode;                                   /**< Relative TS number mode  */
    uint8_t frameType;                                   /**< Frame type 0:Static assignment 1:Reserved */
    uint8_t tsStatus[15];                                /**< Status of each relative TS ID 0x01:Multiples 0x0F:Unused Others:Reserved */
    sony_isdbc_tsmf_receive_status_t receiveStatus[15];  /**< Receive status of each relative TS number */
    uint16_t tsid[15];                                   /**< TS ID of each relative TS number */
    uint16_t networkId[15];                              /**< Network ID of each relative TS number */
    uint8_t relativeTSNumForEachSlot[52];                /**< Relative TS number of each Slot number */
    uint8_t emergency;                                   /**< Emergency indicator 0:Emergency boot control is DISABLED 1:Emergency boot control is ENABLED */
    uint8_t privateData[85];                             /**< Private data */
} sony_isdbc_tsmf_header_t;

#endif /* SONY_ISDBC_H */
