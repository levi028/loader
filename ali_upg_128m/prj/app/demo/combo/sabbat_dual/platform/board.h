/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: board.h
*
*    Description: board common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "../head_file.h"
#include "./chip/chip.h"
#include "./advanced_cfg/advanced_cfg.h"
#include "../key.h"


typedef INT32 (*snim_attach)(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
typedef INT32 (*cnim_attach)(struct QAM_TUNER_CONFIG_API *ptr_qam_tuner);
typedef INT32 (*tnim_attach)(struct COFDM_TUNER_CONFIG_API *ptr_cofdm_tuner);
typedef INT32 (*tnim_attact_with_name)(char *name, PCOFDM_TUNER_CONFIG_API p_config);

typedef INT32 (*scart_attach)(struct scart_init_param * param);
typedef void (*pan_i2c_id_set)(struct pan_device *dev, UINT32 i2c_id);
typedef INT32 (*pan_attach)(struct pan_configuration *config);
typedef INT32 (*rfm_attach)(void);

typedef struct
{
    UINT32 sci_id;
    UINT32 bps;
    UINT32 sci_parity;
}sci_cfg;

typedef struct
{
    UINT32 i2c_id;
    UINT32 bps;
    UINT32 en;
    gpio_info_t *sda;
    gpio_info_t *scl;
}i2c_cfg;

typedef struct
{
    UINT8 tsi_id;
    UINT8 tsi_attr;
    UINT8 tsi_spi_tsg_attr;
    UINT8 tsi_select;  // this thi_id connect to TSI_TS_A or TSI_TS_B
}tsi_cfg;

typedef struct
{
    struct QPSK_TUNER_CONFIG_API qpsk_tuner;
    snim_attach   attach;
    UINT8 tsi_id_m3501a;
    UINT8 tsi_attr_m3501a;
}snim_cfg;

typedef struct
{
    struct QAM_TUNER_CONFIG_API qam_tuner;
    cnim_attach  attach;
    gpio_info_t *qam_int;
}cnim_cfg;

typedef struct
{
    struct COFDM_TUNER_CONFIG_API cofdm_tuner;
    tnim_attach attach;
    tnim_attact_with_name attach_with_name;
}tnim_cfg;


typedef union
{
    snim_cfg  s_cfg;
    cnim_cfg  c_cfg;
    tnim_cfg  t_cfg;
}nim_cfg;

typedef struct
{
    UINT8 front_end_type;  // FRONTEND_TYPE_S FRONTEND_TYPE_C FRONTEND_TYPE_T
    UINT8 is_isdbt;
    UINT16 sat_id;         // if front_end_type == FRONTEND_TYPE_C/FRONTEND_TYPE_T then,
                           // all this type program will be linked with this sat.
                           // if front_end_type == FRONTEND_TYPE_S  then,
                           // all dvbs program will use satellite with sat id(from sat_id to MAX_SAT_NUM-1)
    char *nim_name;
    tsi_cfg tsi_cfg;
    gpio_info_t *nim_reset;
	UINT8 gpio_select;//select which gpio is used to control lnb_power, 0:main chip gpio, 1:demod gpio(like C3501H)
    gpio_info_t *lnb_power;
    gpio_info_t *lnb_short_det;
    nim_cfg nim_config;
    gpio_info_t *antenna_power;
}front_end_cfg;


typedef struct
{
    //pinmux gpio
    UINT8 pin_mux_count;
    UINT8 all_gpio_count;       //for easy init all gpio
    BOOL mem_gpio_enabled;
    pin_mux_info *pin_mux_array;
    gpio_info_t* all_gpio_array;   //for easy init all gpio


    gpio_info_t *ddr_power_ctl;
    gpio_info_t *sys_power;
    gpio_info_t *mute;
    gpio_info_t *usb_power;
    gpio_info_t *fp_standby;   //turn off/on standby led
    gpio_info_t *fp_standby_key;   //GPIO standby key on panel.
    gpio_info_t *scart_tvsat_switch;
    gpio_info_t *scart_aspect;
    gpio_info_t *scart_tv_fb;
    gpio_info_t *scart_power;
    gpio_info_t *scart_vcr_switch;
    gpio_info_t *scart_vcr_detech;  //POS_SCART_VCR_DETECT
    gpio_info_t *smc_5v_ctl;
    gpio_info_t *ata2lan_cs;
    gpio_info_t *sdio_detect;
    gpio_info_t *sdio_lock;
    gpio_info_t *sdio_cs;
    //GpioInfo* sdio_polor;
    gpio_info_t *rmii_rst;  // For AUI nestor test, some demoboard need reset PHY.

    //frontend;
    front_end_cfg *front_end_cfg[2];

    //SCI
    UINT8 libc_printf_uart_id;
    sci_cfg *sci_config[2];

    //i2c
    i2c_cfg *i2c_config[6];
    UINT8 i2c_gpio_num;
    UINT8 i2c_scb_num;

    //scart
    struct scart_init_param *scart_param;
    scart_attach scart_attach;

    //panel
    struct pan_configuration *pan_config;
    pan_i2c_id_set pan_i2c_id_set;
    pan_attach pan_attach;
    UINT32 pan_i2c_id;

    //flash
    UINT8 flash_type; // 0 sflash, 1 pflash

    //rfm
    rfm_attach rfm_attach;
    UINT8 rfm_i2c_id;            //#define I2C_FOR_RFM   I2C_TYPE_GPIO0

    //chip
    chip_init_param chip_param;
    //board advance cfg
    advance_cfg_para adv_cfg_para;

    //key map
    UINT32 key_map_cnt;
    const struct ir_key_map_t *key_map;
    ir_key_map_t *ir_key_maps;
}board_cfg;



typedef enum
{
    POWER_OFF = 0,
    POWER_ON,
}power_on_off;


UINT8 board_get_dvbc_valid_tuner(void);
UINT32 board_get_frontend_type(UINT8 frontend_id);
UINT16 board_get_frontend_satid(UINT8 frontend_id);
UINT8  board_lnb_state(UINT8 frontend_id);               // 0: normal, 1: short deteched(overload)
void board_lnb_power(UINT8 frontend_id, BOOL sw);    // 0: 0ff 1: on
void board_antanne_power(UINT8 frontend_id, BOOL sw);
void board_frontend_reset(UINT8 frontend_id);
void board_power(BOOL sw);                           // 0: off 1: on
#ifdef MULTI_CAS
void board_class_select(enum class_selection cls_sel);
#endif
void board_sci_init();
void board_i2c_init();
void board_frontend_init();
void board_standby_led_onoff(BOOL onoff);
void board_usb_power(BOOL onoff);
void set_board_cfg(board_cfg* cfg);
board_cfg* get_board_cfg(void);
void board_antenna_power(UINT8 frontend_id, BOOL sw);
UINT8 board_frontend_is_isdbt(UINT8 frontend_id);
struct ir_key_map_t* board_get_key_map(void);
UINT32 board_get_key_map_cnt(void);
ir_key_map_t* board_get_ir_key_map(void);
#ifdef DVBC_SUPPORT
void board_update_nim_valid(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

