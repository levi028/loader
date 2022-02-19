/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dirent.h
*
*    Description: declaration of dirent
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _DIRENT_H
#define _DIRENT_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
struct dirent
{
    dev_t        d_dev;
    ino_t        d_ino;
    uint32         d_type;
    off_t         d_size;
    unsigned short  d_reclen;
    char        d_name[1];
};


int fs_opendir(const char *path);
ssize_t fs_readdir(int fd, struct dirent *buffer);
int fs_rewinddir(int fd);
int fs_closedir(int fd);

#ifdef __cplusplus
 }
#endif
#endif
