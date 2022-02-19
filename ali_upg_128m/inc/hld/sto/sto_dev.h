
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: pan.h
*
*    Description: This file contains sto_device structure define in HLD.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __HLD_STO_DEV_H__
#define __HLD_STO_DEV_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <types.h>
#include <sys_config.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <bus/erom/erom.h>

//#define FLASH_SOFTWARE_PROTECT

#define STO_FLAG_AUTO_ERASE     0x80000000
#define STO_FLAG_SAVE_REST      0x40000000

#define P2P_MODE  DUPLEX_MODE    /* Remote access peer to peer mode */
#define M2S_MODE  SIMPLEX_MODE    /* Remote access master to slave mode */

#define RAM_BASE_ADDR           0x88800000
#define RAM_FLASH_SIZE          0x400000    // 4M

enum remote_flash_ctrl_cmd
{
    FLASH_CONTROL ,
    FLASH_SET_CMDADDR,
    FLASH_READ_TMO,
    UPG_MODE_CONTROL,
    UART_BAUDRATE_CONTROL
};

/* IO control command */
enum sto_device_ioctrl_command
{
    STO_DRIVER_ATTACH    = 0,        /* Driver attach command */
    STO_DRIVER_SUSPEND    = 1,       /* Driver suspend command */
    STO_DRIVER_RESUME    = 2,        /* Driver resume command */
    STO_DRIVER_DETACH    = 3,        /* Driver detach command */
    STO_DRIVER_GET_STATUS    = 4,    /* Driver get status command */
    STO_DRIVER_SELECT_SECTOR= 5,     /* Driver device select sector command */
    STO_DRIVER_DEVICE_ERASE    = 6,  /* Driver device erase command */
    STO_DRIVER_SECTOR_ERASE    = 7,  /* Driver sector erase command */
    STO_DRIVER_DEVICE_PROTE    = 8,  /* Driver device write protect command */
    STO_DRIVER_SECTOR_PROTE    = 9,  /* Driver sector write protect command */
    STO_DRIVER_SET_FLAG     = 10,
    STO_DRIVER_SECTOR_BUFFER= 11,

    /*Remote Flash Only! config remote flash writing enable*/
    STO_DRIVER_FLASH_CONTROL = 12,

    /*Remote Flash Only! config P2P or M2S mode*/
    STO_DRIVER_UPG_MODE_CONTROL = 13,

    /*Remote Flash Only! config UART high speed */
    STO_DRIVER_UART_SPEED_CONTROL = 14,
    STO_DRIVER_SECTOR_ERASE_EXT = 15,
    STO_DRIVER_GET_FLASH_INFO = 16,      /*provide upper flash size, sector size, total sectors*/
    STO_DRIVER_GET_UNIQUE_ID = 17        /*provide unique ID */
};

struct sto_device_stats
{
    UINT32 errors;         /* Device operate errors */
};

/*
 *  Structure sto_device, the basic
 *    structure between HLD and LLD of panel device.
 */
/*
#if (defined(SYS_OS_TASK_NUM))
#define STO_TASK_SUPPORT_NUM_MAX     (SYS_OS_TASK_NUM + 1)
#else
#define STO_TASK_SUPPORT_NUM_MAX    16
#endif
*/
#define STO_TASK_SUPPORT_NUM_MAX        256
struct sto_device
{
    struct hld_device *next;        /* Next device structure */
    UINT32            type;         /* Interface hardware type */
    INT8              name[HLD_MAX_NAME_SIZE];    /* Device name */
    UINT16            flags;        /* Interface flags, status and ability */

    /* Hardware privative structure */
    void              *priv;        /* pointer to private data */
    UINT32            base_addr;    /* Device base address */
     /* Current operate address */
    INT32             curr_addr[STO_TASK_SUPPORT_NUM_MAX];
    INT32             totol_size;   /* Device totol space size, in K bytes */
    unsigned char     *sector_buffer;
    unsigned long     flag;
    OSAL_ID           mutex_id;

/*
 *  Functions of this panel device
 */
    INT32    (*init)();

    INT32    (*open)(struct sto_device *dev);

    INT32    (*close)(struct sto_device *dev);

    INT32    (*lseek)(struct sto_device *dev, INT32 offset, int origin);

    INT32    (*read)(struct sto_device *dev, UINT8 *data, INT32 len);

    INT32    (*write)(struct sto_device *dev, UINT8 *data, INT32 len);

    INT32    (*put_data)(struct sto_device *dev, UINT32 offset,
                 UINT8 *data, INT32 len);

    INT32    (*get_data)(struct sto_device *dev, UINT8 *data,
                 UINT32 offset, INT32 len);

    INT32    (*do_ioctl)(struct sto_device *dev, INT32 cmd, UINT32 param);

#ifdef FLASH_SOFTWARE_PROTECT    
    INT32   (*lock)(struct sto_device *dev, INT32 offset, INT32 len);  /* Lock flash by offset and length. */

    INT32   (*unlock)(struct sto_device *dev, INT32 offset, INT32 len);  /* Unlock flash by offset and length. */

    INT32   (*is_lock)(struct sto_device *dev, INT32 offset, INT32 len, INT32 *locked); /* Check flash is lock or not by offset and length. */

	INT32   (*get_lock_range)(struct sto_device *dev, INT32 *offset, INT32 *len); /* Check flash is lock or not by offset and length. */
#endif
};

struct sto_flash_info
{
    UINT8   mode;                   /* Remote access P2P or M2S mode */
    UINT32  sync_tmo;               /* Remote access totoal sync interval in mS */
    UINT32  flash_ctrl;             /* Remote write enable */
    UINT32  uart_baudrate;          /* Remote access baudrate */

    short   flash_deviceid_num;     /* ID table length */
    UINT8   *flash_deviceid;        /* ID table */
    UINT8   *flash_id;              /* Sector index */
    BOOL    enable_soft_protect;

    /*For s-flash and sqi-flash, record io number: 1, 2 or 4*/
    UINT8    *flash_io;
};

/* remote flash */
INT32 sto_remote_flash_sync(struct sto_flash_info *param);

/* local PARALLEL flash init */
INT32 sto_local_flash_attach(struct sto_flash_info *param);        

/* remote PARALLEL flash init */
INT32 sto_remote_flash_attach(struct sto_flash_info *param);

/* local SERIAL flash init */
INT32 sto_local_sflash_attach(struct sto_flash_info *param);

/* remote SERIAL flash init */
INT32 sto_remote_sflash_attach(struct sto_flash_info *param);

void sto_flash_soft_protect(unsigned long db_addr_low, unsigned long db_addr_up);
void sto_flash_disable_protect(void);

/* lld sto_flash_sqi decalaration */
void flash_info_pl_init(void);
void flash_info_sl_init(void);
void flash_info_sl_config(UINT8 *pdeviceid, UINT8 *pflashid, \
        UINT16 flash_id_num, UINT16 *pdeviceid_ex, UINT8 *pflashid_ex, UINT16 flash_id_num_ex);

/* for compabile with old version */
#define sto_remote_flash_init    sto_remote_flash_attach

#ifndef STO_PROTECT_BY_MUTEX
    #ifdef PFLASH_CI_SHARE_PIN
        #define STO_PROTECT_BY_MUTEX
    #endif
#endif

void sto_mutex_enter(void);
void sto_mutex_exit(void);

#define MUTEX_ENTER()    sto_mutex_enter()
#define MUTEX_LEAVE()    sto_mutex_exit()
#ifdef __cplusplus
 }
#endif
#endif  /* __HLD_STO_DEV_H__ */
