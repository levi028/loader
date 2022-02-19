/******************************************************
MN88472_register_sh.h
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'12/03/02 : OKAMOTO    Sepalate "Register Setting Array" to Xtal 20.48MHz and 20.5MHz.
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/
#ifndef _MN88472_REGISTER_SH_H_
#define _MN88472_REGISTER_SH_H_

#include "MN_DMD_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern DMD_I2C_Register_t    MN88472_REG_DVBT2_6MHZ_XTAL20480KHZ_IF4MHZ[];
extern DMD_I2C_Register_t    MN88472_REG_DVBT2_7MHZ_XTAL20480KHZ_IF4500KHZ[];
extern DMD_I2C_Register_t    MN88472_REG_DVBT2_8MHZ_XTAL20480KHZ_IF5MHZ[];
extern DMD_I2C_Register_t    MN88472_REG_DVBT2_1_7MHZ_XTAL20480KHZ_IF5MHZ[];

extern DMD_I2C_Register_t    MN88472_REG_DVBT_6MHZ_XTAL20480KHZ_IF4MHZ[];
extern DMD_I2C_Register_t    MN88472_REG_DVBT_7MHZ_XTAL20480KHZ_IF4500KHZ[];
extern DMD_I2C_Register_t    MN88472_REG_DVBT_8MHZ_XTAL20480KHZ_IF5MHZ[];

extern DMD_I2C_Register_t    MN88472_REG_DVBC_7MHZ_XTAL20480KHZ_IF4500KHZ[] ;
extern DMD_I2C_Register_t    MN88472_REG_DVBC_8MHZ_XTAL20480KHZ_IF5MHZ[];
#ifdef __cplusplus
}
#endif

#endif //#ifndef _MN88472_REGISTER_SH_H_