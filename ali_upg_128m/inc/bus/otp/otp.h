/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: otp.h
*
*    Description: This file provide common otp interface.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OTP_H__
#define __OTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>

#define OTP_DW_LEN  0x4

/* voltage control callback function,
 * OTP driver will tell APP the correct voltage by
 *  OTP_VOLTAGE_6V5 or OTP_VOLTAGE_6V5
 */
typedef void(*OTP_VOLTAGE_CONTROL)(UINT32 voltage);

/* APP must make OTP driver to control programming voltage
 * to guarantee program timming. So App can choose to register
 * GPIO to OTP driver or register voltage control callback function
 * to OTP driver
 */
typedef struct
{
    UINT16    vpp_by_gpio: 1; /*1: we need use one GPIO control vpp voltage.*/
                              /*0: we use Jumper to switch vpp voltage.*/
    UINT16    reserved1: 15;   /*reserved for future usage*/

    /* Polarity of GPIO, 0 or 1 active to set VPP to 6.5V*/
    UINT16    vpp_polar    : 1;

    /* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
    UINT16    vpp_io        : 1;

    UINT16    vpp_position: 14;    /* GPIO Number*/

    OTP_VOLTAGE_CONTROL volctl_cb;    /*OTP program voltage control callback*/
                       /*OTP_VOLTAGE_6V5 for 6.5V,OTP_VOLTAGE_1V8 for 1.8V*/
    UINT32  see_mutex_protect_en;      //if SEE has OTP driver need set it to 1
}OTP_CONFIG;


/* OTP API*/
INT32 otp_init(OTP_CONFIG *cfg);
INT32 otp_read(UINT32 offset, UINT8 *buf, INT32 len);
INT32 otp_write(UINT8 *buf, UINT32 offset, INT32 len);
INT32 otp_write_dw(UINT8 *buf, UINT32 offset, UINT32 mask);

BOOL enable_boot_signature_check(void);
BOOL enable_boot_encrypt_flow (void);
BOOL enable_ejtag_protected (UINT8 mode);
BOOL enable_uart_protected (void);
BOOL enable_hw_monitor (void);
BOOL enable_cw_mandatory_mode(void);
BOOL enable_usb_protected (UINT8 dev_id);
BOOL write_market_id (UINT16 *p_id);
BOOL read_market_id (UINT16 *p_id);
void security_key_read_protect(void);
void ali_otp_get_mutex(UINT32 flag);
int otp_etsi_kdf_trigger(int text_idx, 
	UINT64 vendor_id);
int otp_std_kdf_trigger(int mrk_idx, int text_idx);

#ifdef __cplusplus
 }
#endif

#endif    /* __OTP_H__ */

