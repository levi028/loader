/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: usb_devfs.h
*
*    Description: declaration of usb_devfs api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __USB_DEVFS_H__
#define __USB_DEVFS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define PRODUCT_NAME_LEN 256
//callback functions for USB devfs
struct usb_dev_func
{
	//callback functions for no hub functions
	INT32 (*fusbd_device_read)(UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);
	INT32 (*fusbd_device_write)(UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);

	//callback functions for usb hub functions
	INT32 (*fusbd_device_read_ex)(void* nodeid,UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);
	INT32 (*fusbd_device_write_ex)(void* nodeid, UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);
	//extend for over 2T hdd
	INT32 (*fusbd_device_read_ex_16)(void* nodeid,UINT8 sub_lun, UINT64 lbn, UINT8* buffer, UINT32 sector_count);
	INT32 (*fusbd_device_write_ex_16)(void* nodeid, UINT8 sub_lun, UINT64 lbn, UINT8* buffer, UINT32 sector_count);
		
};

struct fs_usb_dev
{
    //device information
    uint32 nodeid;
    uint32 lun;

    //disk information
    int        n_sector_size;
    off_t    n_sectors;
    off_t    n_start;
    off_t    n_size;

    //callback functions
    struct usb_dev_func func;
    UINT32 which_port;
    UINT32 host_id;
    char product[PRODUCT_NAME_LEN+1];	
    char manufact[PRODUCT_NAME_LEN+1];
    char serialid[PRODUCT_NAME_LEN+1];
};


int usb_hdd_init(struct fs_usb_dev *pusb_dev);
int usb_hdd_get_dev(uint32 nodeid, int lun);
int usb_hdd_cleanup(uint32 nodeid, int lun);
int usb_hdd_get_lunlist(uint32 nodeid, int *lunlist, int maxlun);

int usb_devfs_init(void);
int init_block_cache( int heap_size, int flags );

#ifdef __cplusplus
}
#endif
#endif// __USB_DEVFS_H__




