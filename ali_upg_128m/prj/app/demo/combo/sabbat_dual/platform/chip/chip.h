/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: chip.h
*
*    Description: chip common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _CHIP_H_
#define _CHIP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../head_file.h"

//pinmux information
typedef struct
{
    UINT32 pin_mux_sel;        /* value defined in chip_*.h*/
    UINT32 value;              /* register value */
}pin_mux_info;

//gpio config
typedef struct pan_gpio_info gpio_info_t;
//#define GpioInfo struct pan_gpio_info

typedef struct
{
    BOOL mem_gpio_enabled;
    BOOL ali_ethmac_enabled; // NET_ALIETHMAC
    BOOL ci_power_ctl_enabled;
    BOOL ci_vccen_active_high;
    BOOL dual_enabled;       //not used now
}chip_init_param;

struct _chip;
typedef struct _chip chip;

typedef void (*chip_pin_mux_set_func)(chip* thiz, pin_mux_info* pin_mux_info);
typedef void (*chip_pin_mux_set_ext_func)(chip* thiz, pin_mux_info pin_mux_info[], UINT8 count);
typedef void (*chip_gpio_set_func)(chip* thiz, gpio_info_t* gpio_info, UINT8 value);
typedef UINT32 (*chip_gpio_get_func)(chip* thiz, gpio_info_t* gpio_info);
typedef void (*chip_gpio_pin_init_func)(chip* thiz, gpio_info_t* gpio_info, BOOL enabled);
typedef void (*chip_gpio_dir_set_func)(chip* thiz, gpio_info_t* gpio_info);
typedef void (*chip_gpio_pin_init_ext_func)(chip* thiz, gpio_info_t gpio_info[], UINT8 count);
typedef void (*chip_gpio_dir_set_ext_func)(chip* thiz, gpio_info_t gpio_info[], UINT8 count);
typedef void (*chip_init_func)(chip* thiz, chip_init_param* param);
typedef void (*chip_destory)(chip* thiz);
typedef void (*chip_tsi_pin_cfg_func)(chip* thiz, UINT8 tsi_id, BOOL clear);
typedef void (*chip_ci_power_func)(chip* thiz, BOOL power_on);


struct _chip
{
    chip_pin_mux_set_func       pin_mux_set;
    chip_pin_mux_set_ext_func    pin_mux_set_ext;
    chip_gpio_set_func         gpio_set;
    chip_gpio_get_func         gpio_get;
    chip_gpio_pin_init_func     gpio_pin_init;
    chip_gpio_pin_init_ext_func  gpio_pin_init_ext;
    chip_gpio_dir_set_func      gpio_dir_set;
    chip_gpio_dir_set_ext_func   gpio_dir_set_ext;
    chip_tsi_pin_cfg_func       tsi_pin_cfg;
    chip_ci_power_func         ci_power;
    chip_init_func            init;
    chip_destory         destory;
    INT8 priv[];
};

void chip_pin_mux_set(pin_mux_info* pin_mux_info);
void chip_pin_mux_set_ext(pin_mux_info pin_mux_info[], UINT8 count);
void chip_gpio_pin_init(gpio_info_t* gpio_info, BOOL enabled);
void chip_gpio_pin_init_ext(gpio_info_t gpio_info[], UINT8 count);
void chip_gpio_set(gpio_info_t* gpio_info, UINT8 value);
UINT32 chip_gpio_get(gpio_info_t* gpio_info);
void chip_gpio_dir_set(gpio_info_t* gpio_info);
void chip_gpio_dir_set_ext(gpio_info_t gpio_info[], UINT8 count);
void chip_init(chip_init_param* param);
void chip_tsi_pin_cfg(UINT8 tsi_id, BOOL clear);
void chip_ci_power(BOOL power_on);

void  global_chip_set(chip* chip);
chip* global_chip(void);
chip* chip_create(void);

#define G_CHIP  global_chip()

#ifdef __cplusplus
}
#endif

#endif

