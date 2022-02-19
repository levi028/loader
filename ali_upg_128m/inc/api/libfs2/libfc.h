/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: libfc.h
*
*    Description: declaration of libfc
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef    __LIBFILECONTROL_H__
#define    __LIBFILECONTROL_H__    1
#ifdef __cplusplus
extern "C"
{
#endif

#include <types.h>
#include <sys_config.h>
#include <retcode.h>

#include <api/libfs2/dirent.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/stdio.h>
#include <api/libc/string.h>
#include <api/libfs2/stat.h>
#include <api/libc/time.h>
#include <api/libfs2/unistd.h>

#if 0
#define FC_DEBUG libc_printf
#else
#define FC_DEBUG(...)    do{} while(0)
#endif

/* Try to pull in PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* Some useful definitions */
#undef FALSE
#define FALSE   ((int) 0)
#undef TRUE
#define TRUE    ((int) 1)


typedef void(*cp_callback_func)(unsigned long type, unsigned long param);


extern int f_mkdir (char *path, int flags);
extern int f_rm(const char *path, int flags);
extern int f_cp(const char *source, const char *dest, int flags, void *callback);
extern int f_cancel_cp();
extern int f_mv(const char *source, const char *dest, int flags);


enum
{    /* DO NOT CHANGE THESE VALUES!  cp.c depends on them. */
    FILEUTILS_PRESERVE_STATUS = 1,
    FILEUTILS_DEREFERENCE = 2,
    FILEUTILS_RECUR = 4,
    FILEUTILS_FORCE = 8,
    FILEUTILS_INTERACTIVE = 0x10,
    FILEUTILS_MAKE_HARDLINK = 0x20,
    FILEUTILS_MAKE_SOFTLINK = 0x40,
};

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

#ifdef _INVW_JUICE
extern inview_usb_plug_msg(bool plug);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __LIBFILECONTROL_H__ */
