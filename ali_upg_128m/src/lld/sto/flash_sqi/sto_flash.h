/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: sto_flash.h
*
*    Description: Provide general flash driver head file for sto type device.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2006.4.xx Shipman Yuan  0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _STO_FLASH_H_
#define _STO_FLASH_H_

#include <types.h>
#include <hld/sto/sto_dev.h>


#ifdef __cplusplus
    extern "C" {
#endif

//#define MUTEX_ENTER     osal_task_dispatch_off
//#define MUTEX_LEAVE     osal_task_dispatch_on

#define MS_TICKS       (sys_ic_get_cpu_clock()*1000000 / 2000)
#define RF_READ_TMO (50*1000)  // in unit of US

#define FLASH_ADDR_MASK     0xFFFFFF

#ifdef FLASH_SOFTWARE_PROTECT
/**
 * struct SWP - structure for software protect
 * @status_register_1:          value of status register-1
 * @status_register_2:          value of status register-2
 * @flash_size:                 total flash size
 * @default_lock_addr:          default lock address
 * @default_lock_len:           default lock length
 * @swp_lock:                   lock flash area
 * @swp_unlock:                 unlock flash area
 * @swp_is_lock:                check flash area is lock or not
 * @swp_get_lock_range:         check flash locked ranged
 * @swp_init:                   initialize structure SWP
 */
struct SWP
{
    INT32 flash_id;
    INT32 flash_size;
    INT32 status_register_1;
    INT32 status_register_2;
    INT32 default_lock_addr;
    INT32 default_lock_len;

    int (* swp_lock)(struct SWP *swp, INT32 addr, INT32 len);
    int (* swp_unlock)(struct SWP *swp, INT32 addr, INT32 len);
    int (* swp_is_lock)(struct SWP *swp, INT32 addr, INT32 len, INT32 *lock);
    int (* swp_get_lock_range)(struct SWP *swp, INT32 *addr, INT32 *len);
    int (* swp_init)(struct SWP *swp);
};
/*Add software protection new chip init function here*/
int wb_swp_init(struct SWP *swp);
int gd_swp_init(struct SWP *swp);
int mx_swp_init(struct SWP *swp);
int sp_swp_init(struct SWP *swp);
int esmt_swp_init(struct SWP *swp);	
int xtx_swp_init(struct SWP *swp);

#endif


struct flash_private
{
    const UINT8  *flash_deviceid;
    const UINT8  *flash_id_p;
    const UINT8  *flash_io_p;
    UINT16 flash_deviceid_num;

    UINT8  flash_id;
    UINT8  flash_io;
    UINT16 flash_sectors;
    UINT32 flash_size;
    //UINT32 flash_cmdaddr;

    int  (*io_ctrl)(UINT32 cmd, UINT32 val);
    int  (*open)( void *arg);  // save baudrate
    int  (*close)( void *arg); // switch baudrate back to 115200

    void (*get_id)( UINT32 *buf, UINT32 cmd_addr );
    int  (*erase_chip)( void );
    int  (*erase_sector)(UINT32 addr);
    int  (*write)( UINT32 addr, UINT8 *data, UINT32 len );
    int  (*read )( void *des, void *src, UINT32 len);
    int  (*verify)(UINT32 addr, UINT8 *data, UINT32 len );
    void (*set_io)(UINT8 io_num, UINT8 chip_idx);
#ifdef FLASH_SOFTWARE_PROTECT    
    struct SWP *swp; 
#endif
};
struct flash_info
{
    UINT32 flash_size;
    UINT32 total_sectors;
    UINT32 sector_size;
};
extern BOOL m_enable_soft_protection;
extern unsigned long unpro_addr_low;
extern unsigned long unpro_addr_up;
extern unsigned long remote_flash_read_tmo;

void sto_fp_init(struct sto_device *dev,INT32 (*init)(struct sto_flash_info *param));
unsigned int sto_pflash_identify(struct sto_device *dev, UINT32 mode);
unsigned int sto_sflash_identify(struct sto_device *dev, UINT32 mode);
unsigned int sto_flash_sector_align(struct sto_device *dev,\
                unsigned long addr);
unsigned long sto_flash_sector_size(struct sto_device *dev,\
                unsigned int sector);
unsigned long sto_flash_sector_start(struct sto_device *dev,\
                unsigned long sector_no);

#ifdef __cplusplus
}
#endif

#endif  /*_STO_FLASH_H_*/

