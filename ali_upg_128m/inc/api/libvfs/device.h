/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: device.h
*
*    Description: declaration of device api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __DEVICE_H__
#define __DEVICE_H__
#ifdef __cplusplus
extern "C"
{
#endif
enum
{
    IOCTL_GET_DEVICE_GEOMETRY = 1,
    IOCTL_REREAD_PTABLE,
    IOCTL_GET_DEVICE_PATH,
    IOCTL_GET_NODEID,
    IOCTL_SET_FS_CACHE,
    IOCTL_USER = 100000000
};



typedef struct device_geometry
{
	uint32	start_sector;
	off_t 	sector_count;			//may int64 in the feature
	off_t 	cylinder_count;			//may int64 in the feature
	off_t 	sectors_per_track;
	uint32 	head_count;
	uint32 	bytes_per_sector;
	bool	read_only;
	bool	removable;
	bool overflow;                  //extend for over 2T HDD
	uint64 start_sector_ex;			//extend for over 2T HDD
	char product[257];	
    char manufact[257];
    char serialid[257];
} device_geometry;


struct partition_info
{
    off_t    start;                    /* start lba */
    off_t    offset;                    /* offset (in bytes) */
    off_t    size;                    /* size (in bytes) */
    int32    type;
    int32    status;
    int32    block_size;                /* block size of partition */
    int32    session;                /* id of session */
    int32    partition;                /* id of partition */
    char    device[256];            /* path to the physical device */
};



typedef size_t disk_read_op( void* pnode, off_t n_offset, void* p_buffer, size_t n_size );

typedef int dop_open( void* pnode, uint32 n_flags, void **pcookie );
typedef int dop_free_cookie(void *cookie);
typedef int dop_close( void* pnode, void* pcookie );
typedef int dop_ioctl( void* pnode, void* pcookie, uint32 command, void* p_args);
typedef int dop_read( void* pnode, void* pcookie, off_t n_position, void* p_buffer, size_t n_size );
typedef int dop_write( void* pnode, void* pcookie, off_t n_position, const void* p_buffer, size_t n_size );


struct    device_opertions
{
    dop_open*            open;
    dop_close*            close;
    dop_free_cookie*    free_cookie;
    dop_ioctl*            ioctl;
    dop_read*            read;
    dop_write*            write;
};

#ifdef __cplusplus
 }
#endif

#endif// __DEVICE_H__


