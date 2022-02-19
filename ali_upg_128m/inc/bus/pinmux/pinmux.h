/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pinmux.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _BUS_PINMUX_H_
#define _BUS_PINMUX_H_

#ifdef __cplusplus
extern "C"
{
#endif

//#define PIN_MUX_PRINTF  libc_printf
#define PIN_MUX_PRINTF(...)

/***************************************************
 PinMux_COnflict_Detect-  To check whether the chip PinMux set is ok

 Parms:

 Returns:

Notes:     The funciton will pause the system if some Pinmux set is conflict !!!!!!!
 ***************************************************/
RET_CODE pin_mux_conflict_detect(void);

#ifdef __cplusplus
}
#endif

#endif
