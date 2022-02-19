/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: vfs.h
*
*    Description: declaration of vfs api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __VFS_H__
#define __VFS_H__
#ifdef __cplusplus
extern "C"
{
#endif
#define FSYSTEM_NUM        8
#define VNODE_NUM        128
#define DEFAULT_FD_TABLE_SIZE    128
#define    NSPACE_NUM        8

#define PATH_MAX    1024
#define NAME_MAX    512

#define  MODIFY_USB_R_W_SEC_SIZE	//robin test  usb rw size
#ifdef MODIFY_USB_R_W_SEC_SIZE
#define USB_R_W_SEC_NUMBER  (120*2)		 /* 240 sector * 512 byte = 120K byte*/
#endif								

/*make sure the read/write buffer size <=120K, becase the 64M solution memory is not enough when connect 4T HDD,
4T HDD nSectorSize is 4096, 240*4096= 960K, 
other HDD nSectorSize is 512, 240*512= 120K,
*/
#define FS_MAX_R_W_SIZE  (120*1024)  // 120K

#ifndef WIN32

#include <types.h>
#include <sys_config.h>
#include <retcode.h>


#include <api/libc/string.h>
#include <api/libc/printf.h>

#include <osal/osal.h>

#include <api/libc/list.h>

//for sync object
typedef ID    lock;



//the debug information
#undef TRACE
#undef FUNCTION
#undef VFS_ASSERT
//#define VFS_DEBUG

#ifdef VFS_DEBUG
    #define TRACE(fmt, args...) libc_printf("vfs-> " fmt, ##args)
    #define FUNCTION(fmt, args...) libc_printf("vfs function-> " fmt, ##args)
    #define VFS_ASSERT(expression) \
    {                                    \
        if (!(expression))                \
            libc_printf("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
    }
    #define printf libc_printf
#else
    #define TRACE(fmt, args...) do{(void)0; }while(0)
    #define FUNCTION(fmt, args...) do{ (void)0; }while(0)
    /*
    #define VFS_ASSERT(expression) \
    {                                    \
        if (!(expression))                \
            libc_printf("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
    }
    */
    #define VFS_ASSERT(expression)  do{ (void)0; }while(0)
    #define printf(...)
#endif


#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/unistd.h>


#else //WIN32

#define _WIN32_WINNT 0x0501
#include <time.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <windows.h>
#define __inline__

void* my_malloc(const char *file, const int line, int size);
void my_free(const char *file, const int line, void *p);


#define MALLOC(x)    \
        my_malloc(__FILE__, __LINE__, (x))

#define FREE(x)    \
        my_free(__FILE__, __LINE__, (x))

#define MEMSET MEMSET
#define MEMCPY MEMCPY


typedef CRITICAL_SECTION lock;

#include "msvc/list.h"

//the debug information
#undef TRACE
#undef FUNCTION
#undef VFS_ASSERT
//#define VFS_DEBUG

#ifdef VFS_DEBUG
    #define TRACE    printf
    #define FUNCTION printf
    #define VFS_ASSERT(expression) \
    {                                    \
        if (!(expression))                \
            printf("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
    }
#else
    #define TRACE null_printf
    #define FUNCTION null_printf
    #define VFS_ASSERT(expression) \
    {                                    \
        if (!(expression))                \
            printf("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
    }
#endif


#include "inc/types.h"
#include "inc/errno.h"
#include "inc/dirent.h"
#include "inc/stat.h"
#include "inc/statvfs.h"
#include "inc/fcntl.h"
#include "inc/unistd.h"


#endif //WIN32


typedef unsigned char bool;

#ifndef true
#undef true
#define true 1
#endif


#ifndef false
#undef false
#define false 0
#endif


#include "fsystem.h"

/** The I/O context of a process/team, holds the fd array among others */
struct vnode;

struct io_context
{
    struct vnode     *cwd;
    lock            io_lock;
    uint32_t        table_size;
    uint32_t        num_used_fds;
    struct file_descriptor **fds;
};

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE,MEMBER) ((size_t)&((TYPE*)0)->MEMBER)

int vfs_init();
int vfs_release();

int common_access(char *path, int mode);
int common_create_link(char *path, char *to_path);
int common_create_symlink(int fd, char *path, char *to_path, int mode);
int common_ioctl(struct file_descriptor *descriptor, unsigned long op, void *buffer, size_t length);
int common_path_read_stat(int fd, char *path, bool traverse_leaf_link, struct stat *stat);
int common_path_write_stat(int fd, char *path, bool traverse_leaf_link, const struct stat *stat, int stat_mask);
int common_read_link(int fd, char *path, char *buffer, size_t *_buffer_size);
int common_read_stat(struct file_descriptor *descriptor, struct stat *stat);
int common_rename(int fd, char *path, int new_fd, char *new_path);
int common_sync(char *device);
int common_unlink(int fd, char *path);
int common_write_stat(struct file_descriptor *descriptor, const struct stat *stat, int stat_mask);



int dir_create(int fd, char *path, int perms);
int dir_remove(int fd, char *path);
int dir_open(int fd, char *path);


int file_create(int fd, char *path, int open_mode, int perms);
int file_open(int fd, char *path, int open_mode);

int install_file_system(fsystem *fs);
int get_fsystem_installed(char fsname[][16], int max);

int vfs_mount(char *path, const char *device, const char *fs_name, uint32_t flags, const char *args);
int vfs_unmount(char *path, uint32_t flags);
int fs_read_statfs(const char *device, struct statvfs *statvfs);
int fs_write_statfs(const char *device, const struct statvfs *statvfs, uint32_t mask);

void qsort(void *, unsigned long, unsigned long,  int (*compar)(const void *, const void *));

struct io_context *get_current_io_context();


#ifdef __cplusplus
}
#endif
#endif// __VFS_H__



