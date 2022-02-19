/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: statvfs.h
*
*    Description: declaration of statvfs
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _SYS_STATFS_H
#define _SYS_STATFS_H
#ifdef __cplusplus
extern "C"
{
#endif
//#include "types.h"

struct statvfs
{
    unsigned long f_bsize;     // File system block size.
    unsigned long f_frsize;     // Fundamental file system block size.
    unsigned long f_blocks;     // Total number of blocks on file system in units of f_frsize.
    unsigned long f_bfree;     // Total number of free blocks.
    unsigned long f_bavail;     // Number of free blocks available to
                             //     non-privileged process.
    unsigned long f_files;     // Total number of file serial numbers.
    unsigned long f_ffree;     // Total number of free file serial numbers.
    unsigned long f_favail;     // Number of file serial numbers available to
                             //     non-privileged process.
    unsigned long f_fsid;     // File system ID.
    unsigned long f_flag;     // Bit mask of f_flag values.
    unsigned long f_namemax; // Maximum filename length.

    char f_device_name[128];
    char f_volume_name[128];
    char f_fsh_name[128];

};

#define        WFSSTAT_NAME    0x0001

int fs_statvfs(const char *path, struct statvfs *buf);
int fs_write_statvfs(const char *path, struct statvfs *buf, unsigned int mask);

#ifdef __cplusplus
 }
#endif
#endif

