#ifndef  _USERDEF_H_
#define  _USERDEF_H_

#define NULL ((void *)0)

#if 0
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef signed short int    INT16;
typedef unsigned short int  UINT16;
typedef signed long int     INT32;
typedef unsigned long int   UINT32;
typedef unsigned char       BYTE;
typedef unsigned short      USHORT;
typedef unsigned long       ULONG;

typedef unsigned char  bool;
typedef enum {
    false = 0,
    true
} BoolFlag;
#else

/*************************************************
 I2C Max length of tuner write
**************************************************/
#define MAX_LENGTH_TUNER_WRITE 32

/*************************************************
 I2C slave address
**************************************************/
#define DVBT_SLAVE_ADDRESS  0xD8    /* Demod DVB-T/T2 Slave Address */
#define DVBC_SLAVE_ADDRESS  0xDC    /* Demod DVB-C Slave Address    */
#define TUNER_SLAVE_ADDRESS 0xC0    /* Tuner Slave Address          */

/*************************************************
 Block ID definition for I2C access
**************************************************/
typedef enum {
    TUNER_TERR,       /* Tag for Tuner Block              */
    DEMOD_DVBT,       /* Tag for Demod Block (DVB-T/T2)   */
    DEMOD_DVBC        /* Tag for Demod Block (DVB-C)      */ /* add 2010-10-05 */
} BLOCKID;

#if 0
typedef enum {
    SONY_DVB_OK,                /**< Successful. */
    SONY_DVB_ERROR_ARG,         /**< Invalid argument (maybe software bug). */
    SONY_DVB_ERROR_I2C,         /**< I2C communication error. */
    SONY_DVB_ERROR_SW_STATE,    /**< Invalid software state. */
    SONY_DVB_ERROR_HW_STATE,    /**< Invalid hardware state. */
    SONY_DVB_ERROR_TIMEOUT,     /**< Timeout occured. */
    SONY_DVB_ERROR_UNLOCK,      /**< Failed to lock. */
    SONY_DVB_ERROR_RANGE,       /**< Out of range. */
    SONY_DVB_ERROR_NOSUPPORT,   /**< Not supported for current device. */
    SONY_DVB_ERROR_CANCEL,      /**< The operation is canceled. */
    SONY_DVB_ERROR_OTHER        /**< The operation failed. Enable tracing to determine details of the error. */
} sony_dvb_result_t;
#endif
/*************************************************
 I2C access function
**************************************************/
INT16 iic_write_val(INT16 id, UINT8 subadr, UINT8 val);
INT16 iic_write(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len);
INT16 iic_read(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len);

INT16 write_tuner(
    UINT8 slv0,
    UINT8 slv1,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);
INT16 write_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);
INT16 read_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);

/*************************************************
 Wait
**************************************************/
void Waitms(UINT16 ms);


#endif
#endif /* _USERDEF_H_ */

