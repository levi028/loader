/******************************************************
MN_DMD_types.h
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'11/10/24 : OKAMOTO    Update to "MN88472_Device_Driver_111012"
'11/08/27 : OKAMOTO Update to "MN88472_Device_Driver_110826".
'11/08/26 : OKAMOTO Update to "MN88472_Device_Driver_110823".
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/


#ifndef MN_DMD_TYPES_H
#define MN_DMD_TYPES_H

/* **************************************************** */
/* types */
/* **************************************************** */


typedef unsigned char DMD_u8_t;
typedef char DMD_s8_t;
typedef    unsigned int  DMD_u32_t;
typedef    int              DMD_s32_t;
typedef    char*          DMD_text_t;
#define    DMD_API

/* '11/12/27 : OKAMOTO    Move option from project to source code. */
//#define DMD_FLOATING_FUNCTION

#ifdef DMD_FLOATING_FUNCTION
typedef float DMD_float_t;
#else
typedef int DMD_float_t;            //temporary
#endif


#endif
