/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: p2p_upgrade.h
*
*    Description: this file describes upgrade operations by rs232.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _P2P_UPGRADE_H_
#define _P2P_UPGRADE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define ID_TYPE(x) (x>>16)
#define BOOTLOADER_ID 0
#define USERDB_ID 0x04FB
#define TEMPDB_ID 0x05FA
#define UPGRADE_MODE_ONE2ONE        0x51
#define UPGRADE_MODE_MUL_NORMAL        0x61
#define UPGRADE_MODE_MUL_COMPRESS    0x62
#define EUP_OFFSET(field)   ((UINT32)&(((PEROM_UPGRADE_PARAM)0)->field))
#define UPGRADE_PARAM_ADDR  0xa0000100
#define UPGRADE_PARAM_ADDR_EXT  0xa00001F0

enum
{
    UPGRADE_MULTI,
    UPGRADE_SINGLE
};

typedef struct _EROM_UPGRADE_PARAM
{
    UINT32 upgrade_mode;    // byte[0]:p2p or p2m,
                            // byte[1]:uart bitrate, 1: 2M, 2:6M, else 115200
                            // byte[2-3]:reserved
    UINT32 binary_len;
    UINT32 unzip_source_addr;
    UINT32 unzip_temp_addr;
//    UINT32 image_addr;
}EROM_UPGRADE_PARAM, *PEROM_UPGRADE_PARAM;

typedef struct _CMD_VERSION_HEAD
{
    UINT32 flash_size;

    UINT32 fw_status    : 8;
    UINT32 reserved     : 8;
    UINT32 data_len     : 16;
}CMD_VERSION_HEAD, *PCMD_VERSION_HEAD;
//#endif

struct upge_feature_config
{
    UINT8 sys_sdram_size; // 2:2M, 8:8M
    char *bk_buff;
    UINT8 remote_flash_read_tmo;
    UINT8 remote_flash_write_verify;
    UINT8 enable_serial_flash;
    UINT8 enable_fast_erom_upgrade;
    UINT8 project_fe_is_dvbt;
    UINT8 enable_lzma_out_read;
    UINT32 chip_flashwr;
    UINT32 chip_config;

    INT32 (*init_block_list)();
    void (*free_block_list)();
    void (*free_slave_list)();
    UINT32 (*index_to_id)(INT32 index);
    void (*clear_upg_flag)();
    INT32 (*set_upg_flag)(UINT32 id, UINT32 mask, INT32 flag);
    void (*p2p_delay)(void);

    INT32 (*sys_upgrade)(void (*callback)(INT32 type, INT32 process, UINT8 *str),UINT32 (*get_exit_key_arg)(void));
    void (*set_upgrade_mode)(INT32 mode);
};

INT32 init_block_list();
//INT32 InitBlockList_ext(struct upge_feature_config *pupge_feature_config);
//INT32 InitBlockList_ext_normal(struct upge_feature_config *pupge_feature_config);

void free_block_list();
void free_slave_list();
UINT32 index_to_id(INT32 index);
void clear_upg_flag();
INT32 set_upg_flag(UINT32 id, UINT32 mask, INT32 flag);
void p2p_delay(void);

//void p2p_stop_slave_command(void);
#ifndef ENABLE_FAST_EROM_UPGRADE
#endif
INT32 sys_upgrade(void (*callback)(INT32 type, INT32 process, UINT8 *str), UINT32 (*get_exit_key_arg)(void));
INT32 sys_upgrade2(void (*callback)(INT32 type, INT32 process, UINT8 *str),UINT32 (*get_exit_key_arg)(void));
//void SetUpgradeMode(INT32 mode);
void p2p_uart_set_id(UINT32 uart_id);

UINT32 get_chunk(BYTE *buffer, UINT32 n_len);
#if (defined _EROM_UPG_HOST_ENABLE_ || defined _MPLOADER_UPG_HOST_ENABLE_)
void set_upgrade_mode(UINT32 mode);
void set_transfer_mode(UINT32 mode);
#endif
void p2p_enable_upgrade_bootloader(BOOL enable);

extern UINT32 g_protocol_version; /* ***** */
extern UINT32 p2p_uart_id; /* ******* */

#ifdef __cplusplus
extern "C"
{
#endif

#endif /* _P2P_UPGRADE_H_ */

