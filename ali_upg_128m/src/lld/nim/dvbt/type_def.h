/**

 \file type_def.h
 \brief Header file which contains declaration for all types used
        by   Host and Firmware

 \date  2009/05/01 Created by M.GANDILLON, V.PILLOUX, B. CALVET

 \section section1 file description :
         Based on typedef done for Aton2 project

     Constant definition declaration.\n
     Enum declaration\n
     Structure declaration

*/

#ifndef TYPE_DEF_H
#define TYPE_DEF_H

#ifdef MSNWIN32
#include <stdint.h>
#else

/** \typedef int8_t
    \brief  Define of type signed 8 bits
 */
typedef signed char             int8_t;

/** \typedef int16_t
    \brief   Define of type signed 16 bits
 */
typedef signed short int        int16_t;

/** \typedef int32_t
    \brief  Define of type signed 32 bits
 */
typedef signed int              int32_t;

/** \typedef uint8_t
    \brief  Define of type unsigned 8 bits
 */
typedef unsigned char           uint8_t;

/** \typedef uint16_t
    \brief  Define of type unsigned 16 bits
 */
typedef unsigned short int      uint16_t;

#ifndef UINT16_MAX
#define UINT16_MAX 0xFFFF
#endif

/** \typedef uint32_t
    \brief  Define of type unsigned 32 bits
 */
typedef unsigned int            uint32_t;

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF
#endif

#endif


#ifndef FALSE
/** Boolean value for FALSE : 0 */
#define FALSE (0)
#endif

#ifndef TRUE
/** Boolean value for TRUE : 1 */
#define TRUE (!FALSE)
#endif

/** \typedef bool_t
    \brief Define of type boolean (unsigned 8 bits)
 */
typedef unsigned char bool_t;

/** \typedef Handle_ts
    \brief Define of type for function handling
 */
typedef uint32_t Handle_ts;

/** \typedef CallBackFunction_tf
    \brief Define of type for callback definition
 */
typedef void (*CallBackFunction_tf) (uint8_t* pu8Buffer, uint32_t u32Size,
                                     uint32_t *pu32ConsumedSize, void* param);

#endif /* TYPE_DEF_H */
