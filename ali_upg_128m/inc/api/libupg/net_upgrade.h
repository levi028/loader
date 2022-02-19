#ifndef _NET_UPGRADE_H_
#define _NET_UPGRADE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SECTOR_SIZE (64*1024)
#define NETUPG_MAX_FILENUM_SUPPORT 20
#define NETUPG_ROOT_DIR_LENGTH 2
#define FILE_NAME_SIZE (1024)

#ifndef USB_UPGRADE_SUPPORT_LONG_FILENAME
#define NETUPG_FILE_NAME_LENGTH (NETUPG_ROOT_DIR_LENGTH + 8 + 1 + 3 + 1)
#else
#define NETUPG_FILE_NAME_LENGTH FILE_NAME_SIZE
#endif
#define NETUPG_CHUNK_SIZE 128
#define NETUPG_CHUNKCRC_OFFSET (128 - 4*sizeof(UINT32))//128 = sizeof(CHUNK_HEADER) + reserved bytes size

#define NETUPG_ALLCODE_ID 0xFFFF0000
#define NETUPG_BOOTLOADER_ID 0x00000000
#define NETUPG_OTALOADER_ID 0x00FF0000
#define NETUPG_MAINCODE_ID 0x01FE0000
#define NETUPG_LOGO_ID 0x02FD0000
#define NETUPG_DFLTDB_ID 0x03FC0000
#define NETUPG_USERDB_ID 0x04FB0000
#ifdef HDCP_IN_FLASH
#define NETUPG_HDCPKEY_ID HDCPKEY_CHUNID_TYPE//0x05FA0000
#endif
#define NETUPG_BEEP_ID 0x06F90000
#define NETUPG_KEYDATA_ID 0x08F70000

typedef enum
{
    NETUPG_FILE_WR_ERROR = -106,
    NETUPG_FLASH_WR_ERROR = -105,
    NETUPG_CHUNK_SIZE_MISMATCH = -104,
    NETUPG_CHUNK_CNT_MISMATCH = -103,
    NETUPG_FILE_SIZE_MISMATCH = -102,
    NETUPG_FILE_NOT_EXIST = -101,
    NETUPG_ERROR_PARAM= -6,
    NETUPG_BUFFER_MALLOC_FAILURE = -5,
    NETUPG_FILEREAD_ERROR_NOFILE = -4,
    NETUPG_FIELREAD_ERROR_READERROR = -3,
    NETUPG_FIELREAD_ERROR_OVERFLOW = -2,
    NETUPG_FIELREAD_ERROR_UNKOWN = -1,
    NETUPG_FILEREAD_NO_ERROR = 0,
}NETUPG_ERROR_CODE;

typedef enum
{
    NETUPG_ALLCODE = 0,
    NETUPG_BOOTLOADER,
    NETUPG_SECONDLOADER,
    NETUPG_MAINCODE,
    NETUPG_LOGO,
    NETUPG_KEYDB,
    NETUPG_DFLTDB,
    NETUPG_USERDB,
#ifdef HDCP_IN_FLASH
    NETUPG_HDCPKEY,
#endif
    NETUPG_OTHER,
}NETUPG_BLOCK_TYPE;

typedef struct
{
    char upg_file_name[NETUPG_FILE_NAME_LENGTH];
    NETUPG_BLOCK_TYPE upg_file_type;
}NETUPG_FILENODE,*PNETUPG_FILENODE;


typedef void (*netupg_prgrs_disp)(UINT32);
typedef void (*netupg_prgrs_disp_ext)(UINT32,UINT32);

RET_CODE netupg_block_init(UINT16 *block_num);
RET_CODE netupg_block_free();
BOOL netupg_burnflash(UINT16 block_idx,UINT8 *buffer_adr,UINT32 buff_size,netupg_prgrs_disp progress_disp);
NETUPG_BLOCK_TYPE netupg_get_block_type(UINT16 block_idx);
void netupg_set_upgrade_bloader_flag(UINT8 flag);
RET_CODE netupg_allcode_burn(unsigned char* buffer, int len, netupg_prgrs_disp progress_disp);
RET_CODE netupg_ms_burn_by_chunk(UINT32 sector,UINT8 *buffer, int len,UINT8 mode);
void netupg_set_process_callback(netupg_prgrs_disp_ext cb);

#ifdef __cplusplus
}
#endif

#endif /* _USB_UPGRADE_H_ */

