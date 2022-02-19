/******************************************************
MN_I2C.h
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'11/10/24 : OKAMOTO    Update to "MN88472_Device_Driver_111012"
'11/10/21 : OKAMOTO    Omit redundant parameter "sys" from "MN88472_CMN_slave_address_select".
'11/10/19 : OKAMOTO    Use common i2c_handler.
'11/08/26 : OKAMOTO Update to "MN88472_Device_Driver_110823".
'11/08/05 : OKAMOTO Correct slave addr in "DMD_trans_reg".
'11/08/01 : OKAMOTO    Implement I2C read / write handler.
'11/08/01 : OKAMOTO    Correct Error.
'11/07/29 : OKAMOTO    Update to "MN88472_Device_Driver_110715"
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/



#ifndef MN_I2C_H
#define MN_I2C_H

//#include "commdef.h"
#include <basic_types.h>

/* '11/08/05 : OKAMOTO Correct slave addr in "DMD_trans_reg". */
#define DMD_BANK_T2_    (MN88472_CMN_slave_address_get(DMD_E_DVBT2))
#define DMD_BANK_T_      (MN88472_CMN_slave_address_get(DMD_E_DVBT))
#define DMD_BANK_C_        (MN88472_CMN_slave_address_get(DMD_E_DVBC))
#define DMD_BANK_1st    (MN88472_CMN_slave_address_get(DMD_E_DVBT2))
#define DMD_BANK_2nd      (MN88472_CMN_slave_address_get(DMD_E_DVBT))
#define DMD_BANK_3rd    (MN88472_CMN_slave_address_get(DMD_E_DVBC))

#ifdef __cplusplus
extern "C" {
#endif

/* **************************************************** */
/*  System dependence functions */
/* **************************************************** */
/* I2C Bus Functions */
/* these function is defined by MN_DMD_I2C_(system).c */
#if 0    /* Not used on SH EVA board. */
extern DMD_ERROR_t DMD_I2C_open();
#endif
extern DMD_ERROR_t DMD_I2C_Write(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t  data );
extern DMD_ERROR_t DMD_I2C_Read (DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t* data );
extern DMD_ERROR_t DMD_I2C_WriteRead(DMD_PARAMETER_t* param, DMD_u8_t slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u32_t rlen);

#if 0    /* Not used on SH EVA board. */
extern DMD_ERROR_t DMD_I2C_term();
#endif
extern DMD_ERROR_t DMD_wait( DMD_u32_t    msecond);
extern DMD_ERROR_t DMD_timer(DMD_u32_t* tim);


#ifdef DMD_I2C_DEBUG
extern DMD_u32_t    dmd_i2c_debug_flag;
#endif

/* '11/10/21 : OKAMOTO    Omit redundant parameter "sys" from "MN88472_CMN_slave_address_select". */
/*====================================================*
    MN88472_CMN_slave_address_select
   --------------------------------------------------
    Description     Select slave address.(without R/W bit)
    Argument        sadr (SADR pin)
    Return Value    DMD_ERROR_t (DMD_E_OK:success, DMD_E_ERROR:error)
 *====================================================*/
extern DMD_ERROR_t MN88472_CMN_slave_address_select(DMD_u8_t sadr);

/* '11/08/01 : OKAMOTO    Implement I2C read / write handler. */
/*====================================================*
    MN88472_CMN_slave_address_get
   --------------------------------------------------
    Description     Get SLADRS to select slave address.
    Argument        module (Select OFDM or PSK)
    Return Value    Slave address.
 *====================================================*/
extern DMD_u8_t MN88472_CMN_slave_address_get(DMD_SYSTEM_t sys);

#ifdef __cplusplus
}
#endif

#endif

