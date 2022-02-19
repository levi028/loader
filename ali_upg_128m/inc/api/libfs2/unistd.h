/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: unistd.h
*
*    Description: declaration api about fs operation
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _UNISTD_H
#define _UNISTD_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <basic_types.h>
#include "types.h"


__BEGIN_DECLS


#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

int fs_access(const char *path, int mode);
int fs_close(int fd);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_mount(const char *path, const char *device, const char *fs_name, unsigned long flags, const char *args);
int fs_open(const char *path, int oflag, int perms);
ssize_t fs_read(int fd, void *buf, size_t nbyte);
int fs_rename(const char *old_path, const char *new_path);
int fs_remove(const char* path);
int fs_rmdir(const char *path);
int fs_unmount(const char *path, unsigned long flags);
ssize_t fs_write(int fd, const void *buf, size_t nbyte);
int fs_sync(const char *path);
int fs_mkfs(const char *dev, const char* fs_name);
int fs_ioctl(int fd, unsigned long op, void *buffer, size_t length);

int fs_symlink(const char *path1, const char *path2);
int fs_link(const char *path1, const char *path2);
ssize_t fs_readlink(const char *path, char *buf,  size_t bufsize);
int fs_unlink(const char *path);
void fs_prohibit_relative_path(BOOL prohibit);
__END_DECLS
#ifdef __cplusplus
}
#endif
#endif
