/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: devfs.h
*
*    Description: declaration of devfs api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __VFS_DEVFS_H__
#define __VFS_DEVFS_H__
#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_PARTITION_SUPPORT 4
#define MAX_PARTITION_NAME_LEN   16

typedef struct
{
    UINT8    fs_type;
    char    name[MAX_PARTITION_NAME_LEN];
}published_patition_info;

typedef enum
{
    FAT_PARTITION,
    NTFS_PARTITION
}PARTITION_TYPE;

int devfs_publish_device(const char* path, struct device_opertions* ops, void* pnode );
int    devfs_unpublish_device(const char *path, bool disconnect);
int devfs_publish_partition(const char *path, const struct partition_info *info);
int devfs_unpublish_partition(const char *path);
int devfs_publish_file_device(const char *path, char *file_path);
int devfs_unpublish_file_device(const char *path);

int decode_disk_partitions( device_geometry* ps_disk_geom,
                            struct partition_info* pas_partitions,
                            int n_max_partitions,
                            void* pnode,
                            disk_read_op* pf_read_callback );


#ifdef __cplusplus
}
#endif
#endif //__VFS_DEVFS_H__

