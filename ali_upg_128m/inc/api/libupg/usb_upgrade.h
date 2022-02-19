/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: usb_upgrade.h
*
*    Description: this file describes upgrade operations by u disk
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _USB_UPGRADE_H_
#define _USB_UPGRADE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SECTOR_SIZE (64*1024)
#define USBUPG_MAX_FILENUM_SUPPORT 20
#define USBUPG_ROOT_DIR_LENGTH 2
#define FILE_NAME_SIZE (1024)

#ifndef USB_UPGRADE_SUPPORT_LONG_FILENAME
#define USBUPG_FILE_NAME_LENGTH (USBUPG_ROOT_DIR_LENGTH + 8 + 1 + 3 + 1)
#else
#define USBUPG_FILE_NAME_LENGTH FILE_NAME_SIZE
#endif
#define USBUPG_CHUNK_SIZE 128
#define USBUPG_CHUNKCRC_OFFSET (128 - 4*sizeof(UINT32))//128 = sizeof(CHUNK_HEADER) + reserved bytes size

#define USBUPG_ALLCODE_ID 0xFFFF0000
#define USBUPG_BOOTLOADER_ID 0x00000000
#define USBUPG_OTALOADER_ID 0x00FF0000
#define USBUPG_MAINCODE_ID 0x01FE0000
#define USBUPG_LOGO_ID 0x02FD0000
#define USBUPG_DFLTDB_ID 0x03FC0000
#define USBUPG_USERDB_ID 0x04FB0000
#ifdef HDCP_IN_FLASH
#define USBUPG_HDCPKEY_ID HDCPKEY_CHUNID_TYPE//0x05FA0000
#endif
#ifdef LEGACY_HOST_SUPPORT
#define USBUPG_RSAKEY_ID 0x05FA0000
#endif
//#define USBUPG_BEEP_ID 0x06F90000
#define USBUPG_KEYDATA_ID 0x08F70000
#define USBUPG_SEECODE_ID 0x06F90000//0x0AF50000
#define USBUPG_MEMCFG_ID 0x07F80000

typedef enum
{
    USBUPG_FILE_WR_ERROR = -106,
    USBUPG_FLASH_WR_ERROR = -105,
    USBUPG_CHUNK_SIZE_MISMATCH = -104,
    USBUPG_CHUNK_CNT_MISMATCH = -103,
    USBUPG_FILE_SIZE_MISMATCH = -102,
    USBUPG_FILE_NOT_EXIST = -101,
    USBUPG_BUFFER_MALLOC_FAILURE = -5,
    USBUPG_FILEREAD_ERROR_NOFILE = -4,
    USBUPG_FIELREAD_ERROR_READERROR = -3,
    USBUPG_FIELREAD_ERROR_OVERFLOW = -2,
    USBUPG_FIELREAD_ERROR_UNKOWN = -1,
    USBUPG_FILEREAD_NO_ERROR = 0,
}USBUPG_ERROR_CODE;

typedef enum
{
    USBUPG_ALLCODE = 0,
    USBUPG_BOOTLOADER,
    USBUPG_SECONDLOADER,
    USBUPG_MAINCODE,
    USBUPG_LOGO,
    USBUPG_KEYDB,
    USBUPG_DFLTDB,
    USBUPG_USERDB,
#ifdef HDCP_IN_FLASH
    USBUPG_HDCPKEY,
#endif
#ifdef LEGACY_HOST_SUPPORT
     USBUPG_RSAKEY,
#endif
    USBUPG_SEECODE,
    USBUPG_MEMCFG,
    USBUPG_OTHER,
}USBUPG_BLOCK_TYPE;

typedef struct
{
    char upg_file_name[USBUPG_FILE_NAME_LENGTH];
    USBUPG_BLOCK_TYPE upg_file_type;
}USBUPG_FILENODE,*PUSBUPG_FILENODE;
typedef void (*usbupg_prgrs_disp)(UINT32);

RET_CODE usbupg_block_init(UINT16 *block_num);
RET_CODE usbupg_block_free();
RET_CODE usbupg_create_filelist(UINT16 block_idx,UINT16* usbupg_nodenum);
RET_CODE usbupg_get_filenode(PUSBUPG_FILENODE usbupg_node,UINT16 usbupg_nodeidx);
RET_CODE usbupg_readfile(UINT16 block_idx,UINT16 usbupg_fileidx,void *file_buffer,
                                                INT32 buffer_size,usbupg_prgrs_disp progress_disp);
BOOL usbupg_burnflash(UINT16 block_idx,UINT8 *buffer_adr,UINT32 buff_size,usbupg_prgrs_disp progress_disp);
USBUPG_BLOCK_TYPE usbupg_get_block_type(UINT16 block_idx);
RET_CODE usbupg_get_file_filter(UINT16 block_idx, UINT8 *upgfile_extends, UINT8 *upgfile_filters);

RET_CODE usbupg_dump_flash(UINT32 block_idx,UINT8 *str_file,usbupg_prgrs_disp progress_disp);
RET_CODE usbupg_match_file_structure(UINT16 absfile_idx);
void set_upgrade_bloader_flag(UINT8 flag);
RET_CODE usbupg_allcode_burn(UINT16 absfile_idx,usbupg_prgrs_disp progress_disp);
RET_CODE usbupg_readfile_block(UINT16 block_idx,UINT16 absfile_idx,void *file_buffer,UINT32 buffer_size);
RET_CODE usbupg_get_blockinfo(UINT16 block_idx,char *str_blockname,UINT32 *pchunk_offset);

#ifdef __cplusplus
extern "C"
{
#endif

#endif /* _USB_UPGRADE_H_ */

