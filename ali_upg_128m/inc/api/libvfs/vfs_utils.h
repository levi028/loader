/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: vfs_utils.h
*
*    Description: declaration of vfs_utils api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __VFS_UTILS_H__
#define __VFS_UTILS_H__
#ifdef __cplusplus
extern "C"
{
#endif
//type convert functions
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

// lock functions
#ifndef WIN32
    #define LOCK(l)    os_lock_mutex(l, TMO_FEVR)
    #define UNLOCK(l) os_unlock_mutex(l)
#else
    #define LOCK(l)    enter_critical_section(&l)
    #define UNLOCK(l) leave_critical_section(&l)
#endif //WIN32

#ifndef WIN32
    //#define time(...) osal_get_tick()
    //#define panic(fmt, args...) do{libc_printf(fmt, ##args); while(1);}while(0)
      #define panic(fmt, args...) do{while(1);}while(0)
#endif

#define BUF_ALIGN(x, y) (((uint32)(x)  + ((y) - 1)) & (~((y) - 1)))

int new_lock(lock *l);
int free_lock(lock *l);
long atomic_add( long *ptr, long val );
int snooze(int f);

//some sub string functions
char *strdup(const char *s);
size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);

#ifdef WIN32
    void panic( const char *format, ... );
    void null_printf( const char *format, ... );
#endif

#ifdef __cplusplus
 }
#endif
#endif// __VFS_UTILS_H__

