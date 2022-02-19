/******************************************************
mxL_user_define.h
----------------------------------------------------
Rf IC control functions

<Revision History>
'11/10/06 : OKAMOTO    Change "commdef.h" to "dcommdef.h" in sample code.
'11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix".
'11/02/22 : OKAMOTO    Read/Write data on specified bit.
'11/02/14 : OKAMOTO    Correct build error. in C++.
'11/02/10 : OKAMOTO    Mount MxL_I2C_Write, MxL_I2C_Read, and MxL_Delay.
'11/02/10 : OKAMOTO    Move  "User-Defined Types" to "commdef.h".
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/

/*

 User defined Data Types and API Functions for MxL Tuner

 Copyright, Maxlinear, Inc.
 All Rights Reserved

 File Name:      mxl_user_define.h
 Date Created:  Jan. 20, 2009

*/


#ifndef __MxL_USER_DEFINE_H
#define __MxL_USER_DEFINE_H

#if 0    /* '11/02/10 : Move  "User-Defined Types" to "commdef.h". */
/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef char           SINT8;
typedef short          SINT16;
typedef int            SINT32;
typedef float          REAL32;


typedef enum
{
  TRUE = 1,
  FALSE = 0
} BOOL;
#else
// #ifdef SHARP_DEBUG_FOR_CYGWIN
//  #include "../_include/dcommdef.h"
// #else
//  #include "commdef.h"
// #endif
#include <types.h>
#endif /* '11/02/10 : Move  "User-Defined Types" to "commdef.h". */

/* '11/02/22 : OKAMOTO    Read/Write data on specified bit. */
typedef struct _MXL301_REG_STRUCT {
    UINT8    RegAddr;
    UINT8    TopBit;
     UINT8    BottomBit;
} MXL301_REG_STRUCT, *PMXL301_REG_STRUCT;

typedef enum _MxL_BIT_NAME{
    MxL_BIT_NAME_DIG_IDAC_CODE,
    MxL_BIT_NAME_DIG_ENIDAC_BYP,
    MxL_BIT_NAME_DIG_ENIDAC,
    MxL_BIT_NAME_DIG_ATTOFF,
    MxL_BIT_NAME_DIG_ATTON,
    MxL_BIT_NAME_DIG_IDAC_MODE,
    MxL_BIT_NAME_FRONT_BO,
    MxL_BIT_NAME_IF1_OFF     ,
    MxL_BIT_NAME_IF2_OFF     ,
    MxL_BIT_NAME_MAIN_TO_IF2    ,
    MxL_BIT_NAME_AGC_MODE    ,

    /* '11/09/30 : OKAMOTO    Implement "MxL_agc_gain_fix". */
    MxL_BIT_NAME_AGC_GAIN_FIX    ,

    MxL_BIT_NAME_MAX,
}MxL_BIT_NAME, *PMxL_BIT_NAME;

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
**
**  Name: MxL_I2C_Write
**
**  Description:    I2C write operations
**
**  Parameters:
**                    DeviceAddr    - MxL201RF Device address
**                    pArray        - Write data array pointer
**                    count        - total number of array
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
//UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count, UINT32 tuner_id);

/******************************************************************************
**
**  Name: MxL_I2C_Read
**
**  Description:    I2C read operations
**
**  Parameters:
**                    DeviceAddr    - MxL201RF Device address
**                    Addr        - register address for read
**                    *Data        - data return
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
//UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, UINT32 tuner_id);

/******************************************************************************
**
**  Name: MxL_Delay
**
**  Description:    Delay function in milli-second
**
**  Parameters:
**                    mSec        - milli-second to delay
**
**  Returns:        0
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
void MxL_Delay(UINT32 mSec);

/*------------------------------------------------------------*/

//=========================================================================
// FUNCTIONS
//=========================================================================

UINT32 MXL301_register_write_bit_name(UINT8 DeviceAddr, MxL_BIT_NAME bit_name, UINT8 RegData, UINT32 tuner_id);

#ifdef __cplusplus
}
#endif

# endif //__MxL_USER_DEFINE_H
