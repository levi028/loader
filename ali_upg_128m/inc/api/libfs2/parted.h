/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: parted.h
*
*    Description: declaration of PedPartition
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIBPARTED_H_
#define _LIBPARTED_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>


/* We can address 2^63 sectors */
typedef long long ped_sector;

/* A cylinder-head-sector geometry. */
typedef struct _ped_chsgeometry
{
    long long   cylinders;
    int         heads;
    int         sectors;    /* sectors per track */
} ped_chsgeometry;

/* Partition types */
typedef enum
{
    PED_PARTITION_NORMAL        = 0x00,
    PED_PARTITION_LOGICAL       = 0x01,
    PED_PARTITION_EXTENDED      = 0x02,
    PED_PARTITION_FREESPACE     = 0x04,
    PED_PARTITION_METADATA      = 0x08
} ped_partition_type;

/* Geometry of the partition */
typedef struct _ped_geometry
{
    ped_sector   start;
    ped_sector   length;
    ped_sector   end;
} ped_geometry;

typedef struct _ped_disk ped_disk;
typedef struct _ped_partition ped_partition;

/* PedPartition structure represents a partition. */
struct _ped_partition
{
    ped_partition       *prev;
    ped_partition       *next;

    ped_disk            *disk;
    ped_geometry         geom;

    int                 num;
    ped_partition_type    type;

    int                 part_id;

   /*Only used for an extended partition.The list of logical partitions and free space within the extended partition.*/
    ped_partition       *part_list;

    void               *priv;
};

/* A whole disk, eg. /dev/hda, not /dev/hda3 */
struct _ped_disk
{
    ped_chsgeometry      geom;           /* disk geometry */

    int                 sector_size;    /* sector size */
    long long           length;         /* disk length, measured in sector */

    ped_partition       *part_list;      /* list of partitions. Access with ped_disk_next_partition() */
    int                 pri_part_num;   /* primary partitions number */
    int                 ext_part_num;   /* extended partitions number */
    int                 log_part_num;   /* logical partitions number */

    char                path[32];       /* disk path, eg. /dev/hda */
    int                 dev_fd;         /* device file descriptor for read/write*/

    int                 update_mode;
    int                 dirty;
};

typedef enum
{
    PED_FS_TYPE_NONE = 0,
    PED_FS_TYPE_FAT12,
    PED_FS_TYPE_FAT16,
    PED_FS_TYPE_FAT32,
    PED_FS_TYPE_NTFS
} ped_file_system_type;

ped_sector ped_round_down_to(ped_sector sector, ped_sector grain_size);
ped_sector ped_round_up_to(ped_sector sector, ped_sector grain_size);
ped_sector ped_round_to_nearest(ped_sector sector, ped_sector grain_size);

extern int ped_partiton_priv_init(ped_partition *part, void *buf, int size);
extern int ped_partiton_priv_destroy(ped_partition *part);

extern ped_partition *ped_disk_extended_partition(const ped_disk *disk);
extern ped_partition *ped_disk_next_partition(const ped_disk *disk,
                                             const ped_partition *part);
extern ped_partition *ped_disk_add_partition(ped_disk *disk, ped_partition_type part_type,
        ped_file_system_type fs_type, ped_sector start, ped_sector length);
extern int ped_disk_delete_partition(ped_disk *disk, ped_partition *part);

extern ped_disk *ped_disk_new(const char *disk_path);
extern int ped_disk_commit(ped_disk *disk);
extern int ped_disk_destroy(ped_disk *disk);
extern void ped_disk_print(const ped_disk *disk);

#ifdef __cplusplus
}
#endif
#endif /* _LIBPARTED_H_ */
