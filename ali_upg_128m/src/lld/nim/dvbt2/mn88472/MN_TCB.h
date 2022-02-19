/******************************************************
MN_TCB.h
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'11/11/14 : OKAMOTO    Update to "MN88472_Device_Driver_111028".
'11/10/24 : OKAMOTO    Update to "MN88472_Device_Driver_111012"
'11/08/05 : OKAMOTO Implement "Through Mode".
'11/08/05 : OKAMOTO Update to "MN88472_Device_Driver_110802"
'11/08/01 : OKAMOTO    Correct Error.
'11/07/29 : OKAMOTO    Update to "MN88472_Device_Driver_110715"
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/



#ifndef MN_TCB_H
#define MN_TCB_H

#ifdef __cplusplus
extern "C" {
#endif

/* **************************************************** */
/*  System dependence functions */
/* **************************************************** */
/* I2C Bus Functions */
/* these function is defined by MN_DMD_I2C_(system).c */
//extern DMD_ERROR_t DMD_TCB_Write(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t  data );
//extern DMD_ERROR_t DMD_TCB_Read (DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t* data );

//extern DMD_ERROR_t DMD_TCB_WriteRead(DMD_PARAMETER_t* param, DMD_u8_t slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u8_t rlen);
//extern DMD_ERROR_t DMD_TCB_WriteRead(DMD_PARAMETER_t* param, UINT8    tuner_address , UINT8* data , int wlen , int rlen);
//extern DMD_ERROR_t DMD_TCB_WriteRead(DMD_PARAMETER_t* param, DMD_u8_t slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u32_t rlen);
extern DMD_ERROR_t DMD_TCB_WriteRead(void* nim_dev_priv, UINT8    tuner_address , UINT8* wdata , int wlen , UINT8* rdata , int rlen);

/* '11/08/05 : OKAMOTO Implement "Through Mode". */
//extern DMD_ERROR_t DMD_TCB_WriteAnyLength(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t* data , DMD_u32_t wlen);
//extern DMD_ERROR_t DMD_TCB_ReadAnyLength (DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t* data , DMD_u8_t rlen);


#ifdef __cplusplus
}
#endif

#endif
