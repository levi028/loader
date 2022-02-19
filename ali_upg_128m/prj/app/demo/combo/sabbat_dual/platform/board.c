/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: board.c
*
*    Description: board common function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "board.h"
#if defined (_NV_PROJECT_SUPPORT_)|| defined(_AUI_NESTOR_)
#include <aui_uart.h>
#include <hld/smc/smc.h>
aui_hdl g_uart_handle_0 = NULL;
aui_hdl g_uart_handle_1 = NULL;
#endif

#include <hld/nim/nim.h>

typedef struct
{
    board_cfg *cfg;
}board;

static board g_board;

UINT32 board_get_frontend_type(UINT8 frontend_id)
{
    if((1 < frontend_id) || (NULL == g_board.cfg->front_end_cfg[frontend_id]))
    {
        return 0xffffffff;
    }
    else
    {
        return g_board.cfg->front_end_cfg[frontend_id]->front_end_type;
    }

    return 0xFFFFFFFF;
}

UINT16 board_get_frontend_satid(UINT8 frontend_id)
{
    if((frontend_id > 1) || (NULL == g_board.cfg->front_end_cfg[frontend_id]))
    {
        return 1;
    }
    else
    {
        return g_board.cfg->front_end_cfg[frontend_id]->sat_id;
    }
}

UINT8 board_lnb_state(UINT8 frontend_id)
{
    gpio_info_t *gpio= NULL;

    if((frontend_id > 1) || (NULL == g_board.cfg->front_end_cfg[frontend_id]))
    {
        return 0;
    }
    gpio = g_board.cfg->front_end_cfg[frontend_id]->lnb_short_det;
    return chip_gpio_get(gpio);
}

void board_lnb_power(UINT8 frontend_id, BOOL sw)
{
	front_end_cfg *nim_cfg = NULL;
    gpio_info_t *gpio = NULL;
	struct nim_device *nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM,frontend_id);
	static UINT8 ext_lnb_inited[2];
    UINT8 ext_lnb_exist = 0;
	struct DEMOD_GPIO_STRUCT demod_gpio;
	
    nim_cfg = g_board.cfg->front_end_cfg[frontend_id];
    if((frontend_id > 1) || (NULL == nim_cfg))
    {
        return;
    }
    
    if(nim_cfg->antenna_power)
    {
        return;
    }
    
    gpio = nim_cfg->lnb_power;
    if(nim_cfg->nim_config.s_cfg.qpsk_tuner.ext_lnb_config.ext_lnb_control)
    {
        ext_lnb_exist = 1;
    }

	if(!ext_lnb_exist)
	{
    	if (0 == nim_cfg->gpio_select)
		{
			chip_gpio_set(gpio, (UINT8)sw);
		}
		else if (1 == nim_cfg->gpio_select)
		{
			demod_gpio.position = gpio->position;
			demod_gpio.value = sw;
			nim_io_control(nim_dev, NIM_DRIVER_GPIO_OUTPUT, (UINT32)(&demod_gpio));
		}
	}
	else
	{
		if(!ext_lnb_inited[frontend_id])
		{
			if (0 == nim_cfg->gpio_select)
			{
				chip_gpio_set(gpio, (UINT8)sw);
			}
			else if (1 == nim_cfg->gpio_select)
			{
				demod_gpio.position = gpio->position;
				demod_gpio.value = sw;
				nim_io_control(nim_dev, NIM_DRIVER_GPIO_OUTPUT, (UINT32)(&demod_gpio));
			}	
			ext_lnb_inited[frontend_id] = 1;
		}
		nim_io_control(nim_dev, NIM_LNB_SET_POWER_ONOFF, sw);
	}
}


void board_antenna_power(UINT8 frontend_id, BOOL sw)
{
    gpio_info_t *gpio= NULL;

    if((frontend_id > 1) || (NULL == g_board.cfg->front_end_cfg[frontend_id]))
    {
        return;
    }
    if(g_board.cfg->front_end_cfg[frontend_id]->lnb_power)
    {
        return;
    }
    gpio = g_board.cfg->front_end_cfg[frontend_id]->antenna_power;
    chip_gpio_set(gpio, (UINT8)sw);
}

void board_frontend_reset(UINT8 frontend_id)
{
    gpio_info_t *gpio= NULL;

    if((frontend_id > 1) || (NULL == g_board.cfg->front_end_cfg[frontend_id]))
    {
        return;
    }

    gpio = g_board.cfg->front_end_cfg[frontend_id]->nim_reset;
    chip_gpio_set(gpio, 0);
    osal_task_sleep(300);   //200
    chip_gpio_set(gpio, 1);
    osal_task_sleep(10);
}

void board_standby_led_onoff(BOOL onoff)
{
    chip_gpio_set(g_board.cfg->fp_standby, (UINT8)onoff);  //turn off/on standby led
}

void board_usb_power(BOOL onoff)
{
    chip_gpio_set(g_board.cfg->usb_power, (UINT8)onoff);
}

void board_power(BOOL sw)
{
    chip_gpio_set(g_board.cfg->fp_standby, !(UINT8)sw);     //turn off/on standby led
    chip_ci_power((UINT8)sw);                               //power on CI
    chip_gpio_set(g_board.cfg->sys_power, (UINT8)sw);       //power on system
    chip_gpio_set(g_board.cfg->usb_power, (UINT8)sw);       //power on USB
    board_lnb_power(0, sw);                          //power on lnb0
    board_lnb_power(1, sw);                          //power on lnb1
    board_antenna_power(0, sw);
    board_antenna_power(1, sw);
    if(sw)
    {
        board_frontend_reset(0);
        board_frontend_reset(1);
    }
}

#if defined (MULTI_CAS)|| defined(_AUI_NESTOR_) || defined(_AUI_TEST_)
void board_class_select(enum class_selection cls_sel)
{
    if(SMC_CLASS_A_SELECT == cls_sel)
    {
       //libc_printf("select class A!\n");
       chip_gpio_set(g_board.cfg->smc_5v_ctl, 1);
    }
    else if(SMC_CLASS_B_SELECT == cls_sel)
    {
        //libc_printf("select class B!\n");
        chip_gpio_set(g_board.cfg->smc_5v_ctl, 0);
    }
    else //unknown type
    {
        //libc_printf("Unknow type!\n");
        chip_gpio_set(g_board.cfg->smc_5v_ctl, 1);
    }
}
#endif

static void aui_uart_attach(void *pv_param)
{
    int dev_num = *(int *)pv_param;

    sci_16550uart_attach(dev_num);
}

void board_sci_init(void)
{
    board_cfg *cfg = g_board.cfg;
#ifdef _NV_PROJECT_SUPPORT_
    int dev_num = 0;
    aui_cfg_uart uart_param = { 0 };
#endif

    /* sci */
    libc_printf_uart_id_set(cfg->libc_printf_uart_id);
#ifdef _NV_PROJECT_SUPPORT_
    if (NULL != cfg->sci_config[0])
    {
        dev_num ++;
    }

    if (NULL != cfg->sci_config[1])
    {
        dev_num ++;
    }

    aui_uart_init(aui_uart_attach, (void *)&dev_num);

    if(NULL != cfg->sci_config[0])
    {
        MEMSET(&uart_param, 0, sizeof(uart_param));
        uart_param.ul_baudrate = cfg->sci_config[0]->bps;
        uart_param.ul_parity = AUI_ENUM_UART_NO_PARITY;
        uart_param.ul_stop_bits = AUI_ENUM_UART_STOP_1_0;
        uart_param.ul_stop_bits = AUI_ENUM_UART_DATA_8BITS;
        aui_uart_open(cfg->sci_config[0]->sci_id, &uart_param, &g_uart_handle_0);
    }

    if(NULL != cfg->sci_config[1])
    {
        MEMSET(&uart_param, 0, sizeof(uart_param));
        uart_param.ul_baudrate = cfg->sci_config[1]->bps;
        uart_param.ul_parity = AUI_ENUM_UART_NO_PARITY;
        uart_param.ul_stop_bits = AUI_ENUM_UART_STOP_1_0;
        uart_param.ul_stop_bits = AUI_ENUM_UART_DATA_8BITS;
        aui_uart_open(cfg->sci_config[1]->sci_id, &uart_param, &g_uart_handle_1);
    }
#else
    if((NULL != cfg->sci_config[0]) && (NULL != cfg->sci_config[1]))
    {
        sci_16550uart_attach(2);
        sci_mode_set(cfg->sci_config[0]->sci_id, cfg->sci_config[0]->bps, cfg->sci_config[0]->sci_parity);
        sci_mode_set(cfg->sci_config[1]->sci_id, cfg->sci_config[1]->bps, cfg->sci_config[1]->sci_parity);
    }
    else if(NULL != cfg->sci_config[0])
    {
        sci_16550uart_attach(1);
        sci_mode_set(cfg->sci_config[0]->sci_id, cfg->sci_config[0]->bps, cfg->sci_config[0]->sci_parity);
    }
#endif
}

void board_i2c_init(void)
{
    volatile UINT8 i = 0;
    volatile UINT32 i2c_id = 0;
    gpio_info_t *sda = NULL;
    gpio_info_t *scl = NULL;
    board_cfg *cfg = g_board.cfg;

    if(0 != cfg->i2c_scb_num)
    {
        i2c_scb_attach(cfg->i2c_scb_num);
    }

    if(0 != cfg->i2c_gpio_num)
    {
        i2c_gpio_attach(cfg->i2c_gpio_num);
    }

    for(i=0; i<6; i++)
    {
        if(NULL != cfg->i2c_config[i])
        {
            i2c_id = cfg->i2c_config[i]->i2c_id;
            sda = cfg->i2c_config[i]->sda;
            scl = cfg->i2c_config[i]->scl;
            if((I2C_TYPE_GPIO == (i2c_id & I2C_TYPE_MASK)) && (NULL != sda) && (NULL != scl))
            {
                i2c_gpio_set(i2c_id & I2C_ID_MASK, sda->position, scl->position);
            }

            if(0 != cfg->i2c_config[i]->bps)
            {
                i2c_mode_set(i2c_id, cfg->i2c_config[i]->bps, 1);
            }
        }
    }
}


// call the funcion to set the truely tuner valid status for combo UI
// only set DVBT/ISDBT/DVBC front end
// the function should be called after DB init
//add on 2011-11-07 for combo S2+DVBC
#ifdef DVBC_SUPPORT
void board_update_nim_valid(void)
{
#ifndef _AUI_

    UINT8     i          = 0;
    BOOL      b_modify_sat = FALSE;
    front_end_cfg *nim_cfg = NULL;
    S_NODE snode;

    recreate_sat_view(VIEW_ALL, 0);

    for(i = 0; i < 2; i++)
    {
        nim_cfg = g_board.cfg->front_end_cfg[i];
        if(NULL == nim_cfg)
        {
            break;
        }
        if(FRONTEND_TYPE_S != nim_cfg->front_end_type)
        {
            if (RET_SUCCESS != get_sat_by_id(nim_cfg->sat_id,&snode))
            {
               return;
            }

            //set tuner flag
            if ((0 == i) && (0 == snode.tuner1_valid))
            {
                snode.tuner1_valid = 1;
                snode.tuner2_valid = 0;
                modify_sat(nim_cfg->sat_id,&snode);
                b_modify_sat = TRUE;
            }
            else if ((1 == i) && (0 == snode.tuner2_valid))
            {
                snode.tuner2_valid = 1;
                modify_sat(nim_cfg->sat_id,&snode);
                b_modify_sat = TRUE;
            }
        }
    }
    if (b_modify_sat)
    {
        update_data();
    }
#endif
}


UINT8 board_get_dvbc_valid_tuner(void)
{
    front_end_cfg *nim_cfg = NULL;
    UINT8 i = 0;
    UINT8 ret = 0;

    for(i=0; i<2; i++)
    {
        nim_cfg = g_board.cfg->front_end_cfg[i];
        if(NULL == nim_cfg)
        {
            continue;
        }

        if(FRONTEND_TYPE_C == nim_cfg->front_end_type)
        {
            ret=i;
            break;
        }
    }

    return ret;

}


#endif
void board_frontend_init(void)
{
    UINT8 i = 0;
    UINT8 tsi_id = 0;
    UINT8 tsi_attr = 0;
    UINT8 tsi_sel = 0;
    UINT8 first_tsi_id = 0xff;
    UINT8 second_tsi_id = 0xff;
    front_end_cfg *nim_cfg = NULL;
    struct QPSK_TUNER_CONFIG_API  *qpsk_tuner  = NULL;
    struct QAM_TUNER_CONFIG_API   *qam_tuner   = NULL;
    struct COFDM_TUNER_CONFIG_API *cofdm_tuner = NULL;

    for(i = 0; i < 2; i++)
    {
        nim_cfg = g_board.cfg->front_end_cfg[i];
        if(NULL == nim_cfg)
        {
            break;
        }

        tsi_id = nim_cfg->tsi_cfg.tsi_id;
        tsi_attr = nim_cfg->tsi_cfg.tsi_attr;
        tsi_sel = nim_cfg->tsi_cfg.tsi_select;
        if(FRONTEND_TYPE_S == nim_cfg->front_end_type)
        {
            qpsk_tuner = &(nim_cfg->nim_config.s_cfg.qpsk_tuner);
            nim_cfg->nim_config.s_cfg.attach(qpsk_tuner);
            if(NIM_CHIP_ID_M3501A == qpsk_tuner->device_type)
            {
                tsi_id = nim_cfg->nim_config.s_cfg.tsi_id_m3501a;
                tsi_attr = nim_cfg->nim_config.s_cfg.tsi_attr_m3501a;
            }
        }
        else if ((FRONTEND_TYPE_T == nim_cfg->front_end_type) || (FRONTEND_TYPE_ISDBT == nim_cfg->front_end_type))
        {
            cofdm_tuner = &(nim_cfg->nim_config.t_cfg.cofdm_tuner);
            if(NULL != nim_cfg->nim_config.t_cfg.attach_with_name)
            {
                nim_cfg->nim_config.t_cfg.attach_with_name(nim_cfg->nim_name, cofdm_tuner);
            }
            else
            {
                nim_cfg->nim_config.t_cfg.attach(cofdm_tuner);
            }
        }
        else if(FRONTEND_TYPE_C == nim_cfg->front_end_type)
        {
            qam_tuner = &(nim_cfg->nim_config.c_cfg.qam_tuner);
            nim_cfg->nim_config.c_cfg.attach(qam_tuner);
        }

        // tsg config
        tsi_mode_set(TSI_SPI_TSG, nim_cfg->tsi_cfg.tsi_spi_tsg_attr);

        chip_tsi_pin_cfg(tsi_id, !i);
        ts_route_set_nim_tsiid(i, tsi_id);
        tsi_mode_set(tsi_id, tsi_attr);
        tsi_select(tsi_sel, tsi_id);

        if(0 == i)
        {
            first_tsi_id = tsi_id;
        }
        else if(1 == i)
        {
            second_tsi_id = tsi_id;
        }
    } 
    
#ifdef FSC_SUPPORT
#ifndef _M3505_
    //struct QAM_TUNER_CONFIG_API demod_config;
    //demod_config.dem_config_advanced.qam_config_advanced = NIM_DVBC_J83AC_MODE;
    //nim_s3281_dvbc_attach(&demod_config);
    
    tsi_mode_set(TSI_SSI_0,0x27);
    tsi_mode_set(TSI_SSI_1,0x27);
    //tsi_mode_set(TSI_SSI_2,0x27);
    tsi_disable_error_signal(TSI_SSI_0);
    tsi_disable_error_signal(TSI_SSI_1);
    //tsi_disable_error_signal(TSI_SSI_2);    
    ts_route_set_nim_tsiid(1, TSI_SSI_1);
    //ts_route_set_nim_tsiid(2, 0x0C); 
#endif
#endif

#if (SYS_MAIN_BOARD == BOARD_DB_C3047_T2_01V01)
#ifdef ENABLE_SSI1_2_DATA_SWAP
    chip_tsi_pin_cfg(TSI_SSI_0, TRUE);
    chip_tsi_pin_cfg(TSI_SSI_1, FALSE);
#ifdef M36F_CHIP_MODE
    // Work around to Enable SSI1_2 data swap
    *((volatile UINT8 *)(0xb801a00b)) = (*(volatile UINT8 *)(0xb801a00b)) | 0x10;
#endif
#endif
#endif

    tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
#if (SYS_MAIN_BOARD == BOARD_DB_M3601B_01V01)
    tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
#endif
    if(0xff != second_tsi_id)                       //two tuner
    {
        tsi_dmx_src_select(TSI_DMX_1, TSI_TS_B);
    }
    else
    {
        // single tuner project
        #if defined CC_USE_TSG_PLAYER || defined CI_SLOT_DYNAMIC_DETECT
        tsi_dmx_src_select(TSI_DMX_0, TSI_TS_B);
        #ifdef DVR_PVR_SUPPORT
        pvr_set_default_switch_route(first_tsi_id, tsi_attr);
        #endif
        #endif
        tsi_select(TSI_DMX_1, first_tsi_id);
    }
}


void set_board_cfg(board_cfg *cfg)
{
    if(NULL == cfg)
    {
        ASSERT(0);
        return;
    }
    if(NULL == cfg->front_end_cfg[0])
    {
        ASSERT(0);
        return;
    }

    g_board.cfg = cfg;
}

board_cfg *get_board_cfg(void)
{
    return g_board.cfg;
}

struct ir_key_map_t *board_get_key_map(void)
{
    return (struct ir_key_map_t *)(g_board.cfg->key_map);
}

UINT32 board_get_key_map_cnt(void)
{
    return g_board.cfg->key_map_cnt;
}

ir_key_map_t *board_get_ir_key_map(void)
{
    return g_board.cfg->ir_key_maps;
}

UINT8 board_frontend_is_isdbt(UINT8 frontend_id)
{
    front_end_cfg *nim_cfg = g_board.cfg->front_end_cfg[frontend_id];

    if(NULL != nim_cfg)
    {
        return nim_cfg->is_isdbt;
    }
    else
    {
        return 0;
    }
}



/*
void board_init(BoardCfg* cfg)
{
    check_board_cfg(cfg);
    g_board.cfg = cfg;

    //init chip
    global_chip_set(chip_create());
    chip_init(cfg->chip_param);

    //config gpio
    chip_gpio_pin_init_ext(cfg->all_gpio_array, cfg->all_gpio_count);


    if((NULL != cfg->front_end_cfg[0] && cfg->front_end_cfg[0].front_end_type == FRONTEND_TYPE_S)
        || (NULL != cfg->front_end_cfg[1] && cfg->front_end_cfg[1].front_end_type == FRONTEND_TYPE_S))
    {
        // power off 1000ms to work around IC bug
        chip_gpio_set(cfg->sys_power, 0);
        osal_task_sleep(1000);
    }

    //config pinmux
    chip_pin_mux_set_ext(cfg->pin_mux_array, cfg->pin_mux_count);

    //power on
    board_power(TRUE);

#ifdef DUAL_ENABLE
    hld_dev_see_init(__MM_SEE_DBG_MEM_ADDR);
#endif

#ifndef DISABLE_PRINTF
    board_sci_init();
#endif

#ifndef _BUILD_OTA_E_
    // warning, please don't remove version information
    // it's very helpfull for us to debug/trace issue.
    FIXED_PRINTF("MC: APP  init ok\r\n");
    FIXED_PRINTF("%s\r\n", get_sdk_ver());
    FIXED_PRINTF("%s\r\n", get_core_ver());
    FIXED_PRINTF("%s\r\n", get_app_ver());
#endif

    board_i2c_init();

    //scart
    if(NULL != cfg->scart_attach && NULL != cfg->scart_param);
    {
        cfg->scart_attach(cfg->scart_param);
    }

    //front panel
    if(NULL != cfg->pan_attach && NULL != cfg->pan_config)
    {
        cfg->pan_attach(cfg->pan_config);
        if(NULL != cfg->pan_i2c_id_set);
            cfg->pan_i2c_id_set(cfg->pan_i2c_id);
        g_board.pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
        pan_open(g_board.pan_dev);
        pan_display(g_board.pan_dev, " on ", 4);
    }

    //flash
    if(cfg->flash_type == 1)
        sto_local_flash_attach(NULL);
    else
        sto_local_sflash_attach(NULL);
    g_board.sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    sto_open(g_board.sto_dev);
    sto_chunk_init(0, g_board.sto_dev->totol_size); //sto_eeprom_init();

    //rfm
    if(NULL != cfg->rfm_attach)
    {
        cfg->rfm_attach();
        g_board.rfm_dev = (struct rfm_device*) dev_get_by_id(HLD_DEV_TYPE_RFM, 0);
        g_board.rfm_dev->i2c_type_id_cfg = 1;
        g_board.rfm_dev->i2c_type_id = cfg->rfm_i2c_id;
        rfm_open(g_board.rfm_dev);
    }
   
    //frontend
    front_end_init(cfg->front_end_cfg);

    //advance init
    board_advance_init(cfg->adv_cfg_para);



#ifdef USB_MP_SUPPORT
    #ifdef ALI_SDK_API_ENABLE
        //deca_ext_dec_enable((struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
    #endif
#ifdef PVR_FS_ZH
    init_file_system();
#endif
#endif

#ifdef NETWORK_SUPPORT
    network_attach();
#endif

}
*/

