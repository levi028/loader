//-----------------------------------------------------------------------------
// $Header: /dvp1/common/inc/tmtypes.h 37    2/15/01 3:19p Rappanna $
// (C) Copyright 2000,2001 Philips Semiconductors, All rights reserved
//
// This source code and any compilation or derivative thereof is the sole
// property of Philips Corporation and is provided pursuant to a Software
// License Agreement.  This code is the proprietary information of Philips
// Corporation and is confidential in nature.  Its use and dissemination by
// any party other than Philips Corporation is strictly limited by the
// confidential information provisions of the Agreement referenced above.
//-----------------------------------------------------------------------------
// FILE NAME:    tmtypes.h
//
// DESCRIPTION:  Digital Video Platform TriMedia/MIPS global type definitions.
//
// DOCUMENT REF: DVP Software Coding Guidelines
//               DVP Software Versioning Specification
//               DVP Device Library Architecture Specification
//               DVP Board Support Library Architecture Specification
//               DVP Hardware API Architecture Specification
//
// NOTES:        None
//-----------------------------------------------------------------------------
//
#ifndef TMTYPES_H //--------------------
#define TMTYPES_H
/* &&&&& */
//
// NOTE: This preprocessor check is designed to ensure that the obsolete
//       'tmtypes.h' header file from the TriMedia SDE is not used.  This
//       'tmtypes.h' DVP header file supersedes it.
//
#ifdef  _TMtypes_h
#error  ERROR: Legacy 'tmtypes.h' file used; replace with DVP 'tmtypes.h' file !
#else
#define _TMtypes_h                      // Prevent 'tmtypes.h' from being used
#endif

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------

//#include <tmFlags.h>                    // DVP common build control flags


#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//

#define False           0
#define True            (!False)

#ifdef __cplusplus
#define Null            0
#else
#define Null            ((Void *)0)
#endif

// Maximum length of device name in all BSP and capability structures
#define HAL_DEVICE_NAME_LENGTH 16


//
// DVP Standard Types    Type   Pointer
//                      ------  -------
typedef signed   char   Int8,   *pInt8;     //  8 bit   signed integer
typedef signed   short  Int16,  *pInt16;    // 16 bit   signed integer
typedef signed   long   Int32,  *pInt32;    // 32 bit   signed integer
typedef unsigned char   UInt8,  *pUInt8;    //  8 bit unsigned integer
typedef unsigned short  UInt16, *pUInt16;   // 16 bit unsigned integer
typedef unsigned long   UInt32, *pUInt32;   // 32 bit unsigned integer
typedef void            Void,   *pVoid;     // Void (typeless)
typedef float           Float,  *pFloat;    // 32 bit floating point
typedef double          Double, *pDouble;   // 32/64 bit floating point
typedef unsigned int    Bool,   *pBool;     // Boolean (True/False)
typedef char            Char,   *String;    // char, Null term 8 bit char str

typedef UInt32 tmErrorCode_t;
typedef UInt32 tmProgressCode_t;


typedef union   _tmColor3_t                 // 3 byte color structure
{
    unsigned long u32;
#if (TMFL_ENDIAN == TMFL_ENDIAN_BIG)
    struct {
        UInt32       : 8;
        UInt32 red   : 8;
        UInt32 green : 8;
        UInt32 blue  : 8;
    } rgb;
    struct {
        UInt32   : 8;
        UInt32 y : 8;
        UInt32 u : 8;
        UInt32 v : 8;
    } yuv;
    struct {
        UInt32   : 8;
        UInt32 u : 8;
        UInt32 m : 8;
        UInt32 l : 8;
    } uml;
#else
    struct {
        UInt32 blue  : 8;
        UInt32 green : 8;
        UInt32 red   : 8;
        UInt32       : 8;
    } rgb;
    struct {
        UInt32 v : 8;
        UInt32 u : 8;
        UInt32 y : 8;
        UInt32   : 8;
    } yuv;
    struct {
        UInt32 l : 8;
        UInt32 m : 8;
        UInt32 u : 8;
        UInt32   : 8;
    } uml;
#endif
}   tmColor3_t, *ptmColor3_t;

typedef union   _tmColor4_t                 // 4 byte color structure
{
    unsigned long u32;
#if (TMFL_ENDIAN == TMFL_ENDIAN_BIG)
    struct {
        UInt32 alpha    : 8;
        UInt32 red      : 8;
        UInt32 green    : 8;
        UInt32 blue     : 8;
    } argb;
    struct {
        UInt32 alpha    : 8;
        UInt32 y        : 8;
        UInt32 u        : 8;
        UInt32 v        : 8;
    } ayuv;
    struct {
        UInt32 alpha    : 8;
        UInt32 u        : 8;
        UInt32 m        : 8;
        UInt32 l        : 8;
    } auml;
#else
    struct {
        UInt32 blue     : 8;
        UInt32 green    : 8;
        UInt32 red      : 8;
        UInt32 alpha    : 8;
    } argb;
    struct {
        UInt32 v        : 8;
        UInt32 u        : 8;
        UInt32 y        : 8;
        UInt32 alpha    : 8;
    } ayuv;
    struct {
        UInt32 l        : 8;
        UInt32 m        : 8;
        UInt32 u        : 8;
        UInt32 alpha    : 8;
    } auml;
#endif
}   tmColor4_t, *ptmColor4_t;


//-----------------------------------------------------------------------------
// Hardware device power states
//
typedef enum    _tmPowerState_t
{
    tmPowerOn,                          // Device powered on      (D0 state)
    tmPowerStandby,                     // Device power standby   (D1 state)
    tmPowerSuspend,                     // Device power suspended (D2 state)
    tmPowerOff,                         // Device powered off     (D3 state)

    tmPowerNormalMode,                        // Device normal mode
    tmPowerSleepMode,                        // Device sleep mode
    tmPowerStandbyWith16MHz,                // Device standby with 16Mhz
    tmPowerStandbyWith16MHzAndLoopThrough,    // Device standby with 16Mhz and loop through
    tmPowerStandbyWithLoopThrough,            // Device standby with loop through
    tmPowerNotSupported,                    // Not supported power mode
}   tmPowerState_t, *ptmPowerState_t;


//-----------------------------------------------------------------------------
// Software Version Structure
//
typedef struct  _tmSWVersion_t
{
    UInt32      compatibilityNr;        // Interface compatibility number
    UInt32      majorVersionNr;         // Interface major version number
    UInt32      minorVersionNr;         // Interface minor version number

}   tmSWVersion_t, *ptmSWVersion_t;


//-----------------------------------------------------------------------------
// HW Unit Selection
//
typedef enum _tmUnitSelect_t
{
    tmUnit0 = 0,
    tmUnit1 = 1,
    tmUnit2 = 2,
    tmUnit3 = 3,
    tmUnit4 = 4

}   tmUnitSelect_t, *ptmUnitSelect_t;

//-----------------------------------------------------------------------------
// Instance handle
//
typedef int tmInstance_t;


//-----------------------------------------------------------------------------
// Legacy TM Types/Structures (Not necessarily DVP Coding Guideline compliant)
// NOTE: For DVP Coding Gudeline compliant code, do not use these types.
//
typedef char *          Address;        // Ready for address-arithmetic
typedef char const *    ConstAddress;
typedef unsigned char   Byte;           // Raw byte
typedef float           Float32;        // Single-precision float
#if !defined(__TCS__) && !defined(__MWERKS__)
typedef double          Float64;        // Double-precision float
#endif
typedef int             Int;            // Machine-natural integer
typedef void *          Pointer;        // Pointer to anonymous object
typedef void const *    ConstPointer;
typedef char const *    ConstString;
typedef unsigned int    UInt;           // Machine-natural unsigned integer


typedef Int     Endian;
#define BigEndian       0
#define LittleEndian    1


typedef struct
{
    UInt8   majorVersion;
    UInt8   minorVersion;
    UInt16  buildVersion;
}   tmVersion_t, *ptmVersion_t;

// callback function declaration
typedef Void (*tmCallback_t) (  UInt32  events,
                                Void*   pData,
                                UInt32  userData );


//-----------------------------------------------------------------------------
// INLINE keyword for inline functions in all environments
//
// WinNT/WinCE: Use TMSHARED_DATA_BEGIN/TMSHARED_DATA_END for multiprocess
//  shared data on a single CPU.  To define data variables that are shared
//  across all processes for WinNT/WinCE, use the defined #pragma macros
//  TMSHARED_DATA_BEGIN/TMSHARED_DATA_END and initialize the data variables
//  as shown in the example below.  Data defined outside of the begin/end
//  section or not initialized will not be shared across all processes for
//  WinNT/WinCE; there will be a separate instance of the variable in each
//  process.  Use WinNT Explorer "QuickView" on the target DLL or text edit
//  the target DLL *.map file to verify the shared data section presence and
//  size (shared/static variables will not be named in the MAP file but will
//  be included in the shared section virtual size).
// NOTE: All data variables in the multiprocess section _MUST_BE_INITIALIZED_
//       to be shared across processes; if no explicit initialization is
//       done, the data variables will not be shared across processes.  This
//       shared data mechanism only applies to WinNT/WinCE multiprocess data
//       on a single CPU (pSOS shares all data across tasks by default).  Use
//       the TMML MP shared data region for shared data across multiple CPUs
//       and multiple processes.  Example (note global variable naming):
//
//  #if     (TMFL_OS_IS_CE || TMFL_OS_IS_NT)
//  #pragma TMSHARED_DATA_BEGIN         // Multiprocess shared data begin
//  #endif
//
//  static g<Multiprocess shared data variable> = <Initialization value>;
//         gtm<Multiprocess shared data variable> = <Initialization value>;
//
//  #if     (TMFL_OS_IS_CE || TMFL_OS_IS_NT)
//  #pragma TMSHARED_DATA_END           // Multiprocess shared data end
//  #endif
//

#if        TMFL_OS_IS_CE || TMFL_OS_IS_NT
#define inline  __inline

//
// Places shared data in named DLL data segment for WinNT/WinCE builds.
// NOTE: These pragma defines require DLLFLAGS += -section:.tmShare,RWS in the
//       nt.mak and ce.mak target OS makefiles for this mechanism to work.
//
#define TMSHARED_DATA_BEGIN     data_seg(".tmShare")
#define TMSHARED_DATA_END       data_seg()

#elif      TMFL_OS_IS_PSOS

// NOTE regarding the keyword INLINE:
//
// Inline is not an ANSI-C keyword, hence every compiler can implement inlining
// the way it wants to. When using the dcc compiler this might possibly lead to
// redeclaration warnings when linking. For example:
//
//      dld: warning: Redeclaration of tmmlGetMemHandle
//      dld:    Defined in root.o
//      dld:    and        tmmlApi.o(../../lib/pSOS-MIPS/libtmml.a)
//
// For the dcc compiler inlining is not on by default. When building a retail
// version ( _TMTGTREL=ret), inlining is turned on explicitly in the dvp1 pSOS
// makefiles by specifying -XO, which enables all standard optimizations plus
// some others, including inlining (see the Language User's Manual, D-CC and
// D-C++ Compiler Suites p46). When building a debug version ( _TMTGTREL=dbg),
// the optimizations are not turned on (and even if they were they would have
// been overruled by -g anyway).
//
// When a .h file with inline function declarations gets included in multiple
// source files, redeclaration warnings are issued.
//
// When building a retail version those functions are inlined, but in addition
// the function is also declared within the .o file, resulting in redeclaration
// warnings as the same function is also defined by including that same header
// file in other source files. Defining the functions as static inline rather
// than inline solves the problem, as now the additional function declaration
// is omitted (as now the compiler realizes that there is no point in keeping
// that declaration as it can only be called from within this specific file,
// but it isn't , because all calls are already inline).
//
// When building a debug version no inlining is done, but the functions are
// still defined within the .o file, again resulting in redeclaration warnings.
// Again, defining the functions to be static inline rather than inline solves
// the problem.

// Now if we would change the definition of the inline keyword for pSOS from
// __inline__ to static __inline__, all inline function definitions throughout
// the code would not issue redeclaration warnings anymore, but all static
// inline definitions would.
// If we don't change the definition of the inline keyword, all inline func-
// tion definitions would return redeclaration warnings.
//
// As this is a pSOS linker bug, it was decided not to change the code but
// rather to ignore the issued warnings.
//

#define inline  __inline__

#elif      TMFL_OS_IS_PSOSTM
// TriMedia keyword is already inline

#elif      TMFL_OS_IS_HPUX

#define inline

//#else

//#error confusing value in TMFL_OS!
#endif

/*Assume that |restrict| is supported by tmcc and C99 compilers only. More
   discrimination in this area may be added here as necessary.*/
#if !(defined __TCS__ || \
      (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L))
#define restrict /**/
#endif

//-----------------------------------------------------------------------------
// Exported functions:
//-----------------------------------------------------------------------------
//

#ifdef __cplusplus
}
#endif

#endif // TMTYPES_H //------------------
