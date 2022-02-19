/*****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: basic_types.h
 *
 *  Contents:   This file define the basic data types which may be used
 *          throughout the project.
 *  History:
 *      Date        Author              Version     Comment
 *      ==========  ==================  ==========  =======
 *  1.  03/09/2004  Tom Gao             0.1.000     Initial
 *
 *****************************************************************************/
#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef INT8
typedef char            INT8;
#endif

#ifndef UINT8
typedef unsigned char   UINT8;
#endif

#ifndef INT16
typedef short           INT16;
#endif

#ifndef UINT16
typedef unsigned short  UINT16;
#endif

#ifndef INT32
typedef long            INT32;
#endif

#ifndef UINT32
typedef unsigned long   UINT32;
#endif

#ifndef UINT64
typedef unsigned long long UINT64;
#endif

#ifndef INT64
typedef long long INT64;
#endif

//we recommand not using BOOL as return value, just use RET_CODE
#ifndef BOOL
typedef int             BOOL;
#endif

#ifndef FALSE
  #define   FALSE           (0)
#endif
#ifndef TRUE
  #define   TRUE            (!FALSE)
#endif

#ifndef NUM_ZERO
  #define NUM_ZERO          (0)
#endif
#ifndef NUM_ONE
  #define NUM_ONE           (1)
#endif
#ifndef COUNT_0
  #define COUNT_0           (0)
#endif
#ifndef COUNT_1
  #define COUNT_1           (1)
#endif
#ifndef FLAG_TRUE
  #define FLAG_TRUE         (1)
#endif
#ifndef FLAG_FALSE
  #define FLAG_FALSE        (0)
#endif


//RET for return code, you mey define your privte RET_??? code from
//0x80000000 to 0xFFFFFFFF.
typedef INT32           RET_CODE;
#define RET_SUCCESS     ((INT32)0)
#define RET_FAILURE     ((INT32)(-1))
#define RET_BUSY        ((INT32)(-2))
#define RET_STA_ERR ((INT32)(-3))
#define RET_STA_PLUGIN_NOT_EXIST  ((INT32)(-4))

#ifdef NULL
#undef NULL
#endif
#define NULL            ((void *)0)

//use void, NO VOID exist

typedef UINT32          HANDLE;
#define INVALID_HANDLE  ((HANDLE)0xFFFFFFFF)
typedef void*              VP;

//==>summer add begin

typedef short           WCHAR;
typedef short *         LPWCHAR;
typedef short *         LPWSTR;
typedef char            CHAR;
typedef char *          PCHAR;
typedef char *          LPCHAR;
typedef char *          LPSTR;
typedef void *          PVOID;
typedef void *          LPVOID;

typedef int             BOOLEAN;

#ifndef WIN32
#ifndef INT64
#define INT64_MAX 0x7fffffffffffffffLL
#endif
#else
#ifndef INT64
#define INT64_MAX 9223372036854775807L
#endif
#endif

#if defined(WIN32) && !defined(__GNUC__)
//  typedef __int64   int64;
#ifndef INT64_MIN
# define INT64_MIN        (-9223372036854775807i64 - 1i64)
#endif
#else
//  typedef long long int64;
#ifndef INT64_MIN
# define INT64_MIN        (-9223372036854775807LL - 1LL)
#endif
#endif

#ifndef UINT_MAX
#define UINT_MAX 0xffffffff
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#define INNO_DRIVER_ID   0
#define INNO_DRIVER_BASE (INNO_DRIVER_ID << 16)
enum
{
    INNO_NO_ERROR = INNO_DRIVER_BASE,       /* 0 */
    INNO_ERROR_BAD_PARAMETER,             /* 1 Bad parameter passed       */
    INNO_ERROR_NO_MEMORY,                 /* 2 Memory allocation failed   */
    INNO_ERROR_UNKNOWN_DEVICE,            /* 3 Unknown device name        */
    INNO_ERROR_ALREADY_INITIALIZED,       /* 4 Device already initialized */
    INNO_ERROR_NO_FREE_HANDLES,           /* 5 Cannot open device again   */
    INNO_ERROR_OPEN_HANDLE,               /* 6 At least one open handle   */
    INNO_ERROR_INVALID_HANDLE,            /* 7 Handle is not valid        */
    INNO_ERROR_FEATURE_NOT_SUPPORTED,     /* 8 Feature unavailable        */
    INNO_ERROR_INTERRUPT_INSTALL,         /* 9 Interrupt install failed   */
    INNO_ERROR_INTERRUPT_UNINSTALL,       /* 10 Interrupt uninstall failed */
    INNO_ERROR_TIMEOUT,                   /* 11 Timeout occured            */
    INNO_ERROR_DEVICE_BUSY,               /* 12 Device is currently busy   */
    INNO_ERROR_SUSPENDED,                  /* 13 Device is in D1 or D2 state */
    INNO_ERROR_DEVICE_DISABLED,        /* 14 Device is in disabled state */
    INNO_ERROR_OTHERS,                          /* 15 Others error */
};
typedef UINT32 INNO_ErrorCode_t;

#if 1

typedef struct see_ctrl_flg_i_st
{
    UINT32 p0;
    UINT32 p1;
    UINT32 p2;
    UINT32 p3;
    UINT32 p4;
    UINT32 p5;
    UINT32 p6;
    UINT32 p7;
    UINT32 p8;

}see_ctrl_flg_i;

struct see_ctrl_flg_i_st g_ctl_flgs;
struct see_ctrl_flg_i_st g_inctl_flgs;


#endif


typedef enum
{
	ALI_MAIN_CPU = 0,//!<Main cpu is selected.
	ALI_SEE_CPU,//!<SEE cpu is selected.
}CPU_TYPE;
//==>summer add end
#define __MAYBE_UNUSED__ __attribute__((unused))
#define __NORETURN__ __attribute__ ((noreturn))
#define __WEAK_SYMBOL__ __attribute__((weak))
#ifdef __cplusplus
}
#endif


#endif  //__BASIC_TYPES_H__

