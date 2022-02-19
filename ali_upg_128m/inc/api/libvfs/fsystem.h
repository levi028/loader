/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: fsystem.h
*
*    Description: declaration of fsystem api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __VFS_FSYSTEM_H__
#define __VFS_FSYSTEM_H__
#include <sys_config.h>
#define  FS_IS_READONLY       0x00000001
#define  FS_IS_REMOVABLE       0x00000002
#define  FS_IS_PERSISTENT   0x00000004
#define  FS_IS_SHARED       0x00000008

#define  DEFAULT_MODE_BITS   (0644)
#define  DEFAULT_DIR_MODE_BITS   (0755)
#define  WSTAT_MODE               0x0001
#define  WSTAT_UID              0x0002
#define  WSTAT_GID               0x0004
#define  WSTAT_SIZE               0x0008
#define  WSTAT_ATIME               0x0010
#define  WSTAT_MTIME               0x0020
#define  WSTAT_CRTIME           0x0040
#define  WFSSTAT_NAME              0x0001



typedef unsigned long volume_id;
#ifdef WIN32
    typedef unsigned _int64 vnode_id;
#else
    typedef unsigned long long vnode_id;

#endif


/* the file system's private data structures */
typedef void *fs_volume;
typedef void *fs_vnode;
typedef void *fs_cookie;




#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fsystem
{
    char *name;

    /* general operations */
    int (*mount)(volume_id id, const char *device, uint32_t flags,
                const char *args, fs_volume *_fs, vnode_id *_root_vnode_id);
    int (*unmount)(fs_volume fs);

    int (*read_fs_info)(fs_volume fs, struct statvfs *info);
    int (*write_fs_info)(fs_volume fs, const struct statvfs *info, uint32_t mask);
    int (*sync)(fs_volume fs);

    /* vnode operations */
    int (*walk)(fs_volume fs, fs_vnode dir, const char *name, vnode_id *_id, int *_type);
    int (*get_vnode_name)(fs_volume fs, fs_vnode vnode, char *buffer, size_t buffer_size);

    int (*read_vnode)(fs_volume fs, vnode_id id, fs_vnode *_vnode, bool reenter);
    int (*release_vnode)(fs_volume fs, fs_vnode vnode, bool reenter);
    int (*remove_vnode)(fs_volume fs, fs_vnode vnode, bool reenter);


    /* common operations */
    int (*ioctl)(fs_volume fs, fs_vnode v, fs_cookie cookie, unsigned long op, void *buffer, size_t length);
    int (*fsync)(fs_volume fs, fs_vnode v);

    int (*read_symlink)(fs_volume fs, fs_vnode link, char *buffer, size_t *_buffer_size);
    int (*write_symlink)(fs_volume fs, fs_vnode link, char *to_path);
    int (*create_symlink)(fs_volume fs, fs_vnode dir, const char *name, const char *path, int mode);

    int (*link)(fs_volume fs, fs_vnode dir, const char *name, fs_vnode vnode);
    int (*unlink)(fs_volume fs, fs_vnode dir, const char *name);
    int (*rename)(fs_volume fs, fs_vnode from_dir, const char *from_name, fs_vnode to_dir, const char *to_name);

    int (*access)(fs_volume fs, fs_vnode vnode, int mode);
    int (*read_stat)(fs_volume fs, fs_vnode vnode, struct stat *stat);
    int (*write_stat)(fs_volume fs, fs_vnode vnode, const struct stat *stat, uint32_t stat_mask);

    /* file operations */
    int (*create)(fs_volume fs, fs_vnode dir, const char *name, int open_mode,
                        int perms, fs_cookie *_cookie, vnode_id *_new_vnode_id);
    int (*open)(fs_volume fs, fs_vnode v, int open_mode, fs_cookie *_cookie);
    int (*close)(fs_volume fs, fs_vnode v, fs_cookie cookie);
    int (*free_cookie)(fs_volume fs, fs_vnode v, fs_cookie cookie);
    int (*read)(fs_volume fs, fs_vnode v, fs_cookie cookie, off_t pos, void *buffer, size_t *length);
    int (*write)(fs_volume fs, fs_vnode v, fs_cookie cookie, off_t pos, const void *buffer, size_t *length);

    /* directory operations */
    int (*create_dir)(fs_volume fs, fs_vnode parent, const char *name, int perms, vnode_id *_new_vnode_id);
    int (*remove_dir)(fs_volume fs, fs_vnode parent, const char *name);
    int (*open_dir)(fs_volume fs, fs_vnode vnode, fs_cookie *_cookie);
    int (*close_dir)(fs_volume fs, fs_vnode vnode, fs_cookie cookie);
    int (*free_dir_cookie)(fs_volume fs, fs_vnode vnode, fs_cookie cookie);
    int (*read_dir)(fs_volume fs, fs_vnode vnode, fs_cookie cookie,
                            struct dirent *buffer, size_t buffer_size, uint32_t *_num);
    int (*rewind_dir)(fs_volume fs, fs_vnode vnode, fs_cookie cookie);

    /* tools for file system */
    int (*mkfs)(const char *dev, int flags);
    int (*chkfs)(const char *dev, int flags);


}fsystem;

#define FSYSTEM(x) const fsystem x __attribute__ ((section(".fs.init")))

/* file system add-ons only prototypes */
extern int new_vnode(volume_id volume_id, vnode_id vnode_id, fs_vnode private_node);
extern int publish_vnode(volume_id volume_id, vnode_id vnode_id, fs_vnode private_node);
extern int get_vnode(volume_id volume_id, vnode_id vnode_id, fs_vnode *_private_node);
extern int put_vnode(volume_id volume_id, vnode_id vnode_id);
extern int remove_vnode(volume_id volume_id, vnode_id vnode_id);
extern int unremove_vnode(volume_id volume_id, vnode_id vnode_id);

extern int vfs_get_fs_node_from_path(volume_id vol_id, const char *path, void **_node);
extern int vfs_disconnect_vnode(volume_id vol_id, vnode_id vnode_id);



#ifdef __cplusplus
}
#endif

#endif    /* __VFS_FSYSTEM_H__ */

