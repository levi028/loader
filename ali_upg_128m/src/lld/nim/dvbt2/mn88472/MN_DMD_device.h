/******************************************************
MN_DMD_deviced.h
----------------------------------------------------
Demodulator IC control functions				

<Revision History>
'11/10/24 : OKAMOTO	Update to "MN88472_Device_Driver_111012"
'11/08/29 : OKAMOTO	Select TS output.
'11/08/29 : OKAMOTO	Error Flag output.
'11/08/27 : OKAMOTO Update to "MN88472_Device_Driver_110826".
'11/08/26 : OKAMOTO Update to "MN88472_Device_Driver_110823".
'11/08/05 : OKAMOTO Update to "MN88472_Device_Driver_110802"
'11/08/01 : OKAMOTO	Correct Error.
'11/07/29 : OKAMOTO	Update to "MN88472_Device_Driver_110715"
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION			
******************************************************/

/*!
   this file defines common interface for each demodulator device
   */

#include "MN_DMD_driver.h"

#ifndef MN_DMD_DEVICE_H
#define MN_DMD_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

//return DMD_E_OK , if device support the system & bandwidth
extern DMD_ERROR_t	DMD_system_support( DMD_SYSTEM_t sys );

/* **************************************************** */
/*  Demodulator dependence functions (not exported)*/
/* **************************************************** */
//these functions is defined by each device (device_name.c)
#if 0	/* Not used in SH EVA boad. */
extern DMD_ERROR_t	DMD_device_open( DMD_PARAMETER_t *param );
#endif
extern DMD_ERROR_t	DMD_device_term( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_close( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_init( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_load_pseq ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_pre_tune ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_post_tune ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_set_system( DMD_PARAMETER_t *param ); 
extern DMD_ERROR_t	DMD_device_reset( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_scan( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_get_info( DMD_PARAMETER_t *param , DMD_u32_t id);
extern DMD_ERROR_t	DMD_device_set_info( DMD_PARAMETER_t *param , DMD_u32_t id ,DMD_u32_t val);
extern DMD_s32_t DMD_Carrier_Frequency_bias(DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback IF center frequency bias for Tuner to retune, which is used for field test
extern DMD_s32_t DMD_XTAL_Frequency_bias(DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback Clock frequency bias, , which is used for signal lock issue.

extern DMD_u32_t	DMD_RegSet_Rev;

/* '11/08/29 : OKAMOTO	Error Flag output. */
extern DMD_ERROR_t	DMD_set_error_flag_output(DMD_PARAMETER_t* param, DMD_u8_t bErrorFlagOutputEnable );

/* '11/08/29 : OKAMOTO	Select TS output. */
extern DMD_ERROR_t	DMD_set_ts_output(DMD_PARAMETER_t* param, DMD_TSOUT ts_out );

extern DMD_ERROR_t DMD_API DMD_get_dataPLPs( DMD_u8_t * pDataPLPIndex, DMD_u8_t * pDataPLPNumber , DMD_PARAMETER_t* param );
extern DMD_ERROR_t DMD_API DMD_set_PLP_no( DMD_PARAMETER_t* param, DMD_u8_t PLP_id );

extern DMD_u32_t    DMD_BER(DMD_PARAMETER_t* param,  DMD_SYSTEM_t sys , DMD_u32_t *err , DMD_u32_t *sum);
extern DMD_u32_t    DMD_PER(DMD_PARAMETER_t* param,  DMD_SYSTEM_t sys , DMD_u32_t *err , DMD_u32_t *sum);
extern DMD_ERROR_t DMD_API DMD_channel_search_setting( DMD_PARAMETER_t* param );
extern DMD_ERROR_t fe_mn_api_set_regval( DMD_PARAMETER_t* param, DMD_u8_t reg, DMD_u8_t val);
extern DMD_ERROR_t DMD_API DMD_get_next_data_PLP_info( DMD_PARAMETER_t* param );
extern DMD_ERROR_t DMD_API DMD_get_the_first_data_PLP_info( DMD_PARAMETER_t* param );
extern DMD_u32_t    DMD_AGC(DMD_PARAMETER_t* param);
extern DMD_u32_t    DMD_CNR(DMD_PARAMETER_t* param,  DMD_SYSTEM_t sys , DMD_u32_t *cnr_i,DMD_u32_t *cnr_d);

#ifdef __cplusplus
}
#endif

#endif
