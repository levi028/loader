/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard.c
*
*    Description: This file contains all globe micros and functions declare
*                     of smartcard reader.
*
*    History:
*      Date            Author         Version       Comment
*      ====            ======         =======       =======
*    0.              Victor Chen      Ref.          code
*    1. 2005.9.8     Gushun Chen      0.1.000       Initial
*    2. 2006.1.5     Gushun Chen      0.2.000       Add ISO read/write, add semaphore
*    3. 2006.12.5    Victor Chen      0.3.000       Update the driver to only use smartcard module.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/smc/smc.h>
#include <bus/sci/sci.h>
#include <asm/chip.h>

#include "smartcard.h"
#include "smartcard_irq.h"
#include "smartcard_dev.h"
#include "smartcard_txrx.h"

#define    SMC_DEV_NUM                2


/* Name for the smart card reader, the last character must be Number for index */
static char smc_dev_name[HLD_MAX_NAME_SIZE] = "SMC_DEV_0";

#ifdef _M3702_

struct struct_smc_dev smc_dev_set[SMC_DEV_NUM] =
{
    {0xb8018800,24, NULL, 0xb8000000, 0, 0, 0, 0, 0},
    {0xb8018900,25, NULL, 0xb8000000, 0, 0, 0, 0, 0}
};

#else

struct struct_smc_dev smc_dev_set[SMC_DEV_NUM] =
{
    {0xb8018800,20, NULL, 0xb8000000, 0, 0, 0, 0, 0},
    {0xb8018900,21, NULL, 0xb8000000, 0, 0, 0, 0, 0}
};

#endif



static INT32 smc_dev_open(struct smc_device *dev, void (*callback)(UINT32 param));
static INT32 smc_dev_close(struct smc_device *dev);
static INT32 smc_dev_ioctl(struct smc_device *dev, INT32 cmd, UINT32 param);

static void smc_dev_enable(void){}
static void smc_dev_disable(void){}


static void smc_gpio_detect_lsr(UINT32 dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)((struct smc_device *)dev)->priv;

    if(0 == HAL_GPIO_INT_STA_GET(tp->gpio_cd_pos))
    {
        return;
    }
    HAL_GPIO_INT_CLEAR(tp->gpio_cd_pos);
    if((HAL_GPIO_BIT_GET(tp->gpio_cd_pos)) == (UINT16)tp->gpio_cd_pol)
    {
        if(tp->inserted)
        {
            smc_dev_deactive((struct smc_device *)dev);
        }
    }
}

static void smc_dev_binding_set(struct smartcard_private *tp)
{
    tp->smc_chip_id = sys_ic_get_chip_id();
    tp->smc_chip_version = sys_ic_get_rev_id();
    if(sys_ic_is_m3101())
    {
        tp->smc_chip_id = ALI_M3101;
    }
    if((ALI_S3602 == tp->smc_chip_id) && (tp->smc_chip_version >= IC_REV_6))
    {
        tp->smc_chip_id = ALI_S3602F;
        //smc_chip_version = IC_REV_0;
    }

    if(ALI_S3602==tp->smc_chip_id)
    {
        tp->smc_tx_fifo_size = 8;
        tp->smc_rx_fifo_size = 32;
    }

    if((ALI_M3101==tp->smc_chip_id)||((ALI_M3329E==tp->smc_chip_id) && (tp->smc_chip_version>=IC_REV_5)) || \
     (tp->smc_chip_id>=ALI_S3602F))
    {
        tp->smc_tx_fifo_size = 256;
        tp->smc_rx_fifo_size = 256;
    }
}

static void smc_dev_priv_init(struct smartcard_private *tp)
{
    tp->scr_sys_clk = 0;
    tp->pwm_sys_clk = 0;
    tp->smc_chip_id = 0xffffffff;
    tp->smc_chip_version = 0xffffffff;
    tp->smc_tx_fifo_size = 64;
    tp->smc_rx_fifo_size = 64;

    tp->inserted = 0;
    tp->reseted = 0;
    tp->inverse_convention = 0;
    tp->the_last_send = 0;
    tp->ext_atr_delay = 0;
}

static void smc_dev_default_setting(struct smartcard_private *tp, struct smc_dev_config *config_param)
{
    UINT32 tmp = 0;
    UINT32 num_0 = 0;
    UINT32 num_1 = 1;
    UINT32 num_2 = 2;
    UINT64 sys_clk_166m = 166000000;
    UINT64 sys_clk_135m = 135000000;
    UINT64 sys_clk_154m = 154000000;

    if(ALI_M3329E==tp->smc_chip_id)
    {
        if(tp->smc_chip_version>=IC_REV_5)
        {
            smc_dev_set[0].io_base = 0xb8001800;
            smc_dev_set[0].irq = 20;
            smc_dev_set[1].io_base = 0xb8001800;
            smc_dev_set[1].irq = 20;
            tp->scr_sys_clk = 108000000;
            tp->pwm_sys_clk = 108000000;
        }
        else
        {
            smc_dev_set[0].io_base = 0xb8001800;
            smc_dev_set[0].irq = 14;
            smc_dev_set[0].pwm_addr = 0xb8001430;
            smc_dev_set[0].pwm_sel_ofst = 0x0;
            smc_dev_set[0].pwm_seh_ofst = 0x2;
            smc_dev_set[0].pwm_gpio_ofst = 0x4;
            smc_dev_set[0].pwm_cfg_ofst = 0x5;
            smc_dev_set[0].pwm_frac_ofst = 0x6;

            smc_dev_set[1].io_base = 0xb8001900;
            smc_dev_set[1].irq = 15;
            smc_dev_set[1].pwm_addr = 0xb8001460;
            smc_dev_set[1].pwm_sel_ofst = 0x0;
            smc_dev_set[1].pwm_seh_ofst = 0x2;
            smc_dev_set[1].pwm_gpio_ofst = 0x4;
            smc_dev_set[1].pwm_cfg_ofst = 0x5;
            smc_dev_set[1].pwm_frac_ofst = 0x6;
            if(!tp->scr_sys_clk)
            {
                tmp = ((*((volatile UINT32 *)0xb8000070))>>2)&0x3;
                if(tmp == num_0)
                {
                     tp->pwm_sys_clk = 135000000;
                }
                 else if(tmp==num_1)
                 {
                     tp->pwm_sys_clk = 120000000;
                 }
                 else if(tmp==num_2)
                 {
                     tp->pwm_sys_clk = 166000000;
                 }
                 else
                 {
                     tp->pwm_sys_clk = 154000000;
                 }
                    tp->scr_sys_clk = 108000000;
            }
        }
    }
    if(1 == sys_ic_is_m3202())
    {
        smc_dev_set[0].io_base = 0xb8001800;
        smc_dev_set[0].irq = 20;
        smc_dev_set[0].pwm_addr = 0xb8001a00;
        smc_dev_set[0].pwm_sel_ofst = 0x0;
        smc_dev_set[0].pwm_seh_ofst = 0x2;
        smc_dev_set[0].pwm_gpio_ofst = 0x6;
        smc_dev_set[0].pwm_cfg_ofst = 0x4;
        smc_dev_set[0].pwm_frac_ofst = 0x5;

        smc_dev_set[1].io_base = 0xb8001900;
        smc_dev_set[1].irq = 21;
        smc_dev_set[1].pwm_addr = 0xb8001b00;
        smc_dev_set[1].pwm_sel_ofst = 0x0;
        smc_dev_set[1].pwm_seh_ofst = 0x2;
        smc_dev_set[1].pwm_gpio_ofst = 0x6;
        smc_dev_set[1].pwm_cfg_ofst = 0x4;
        smc_dev_set[1].pwm_frac_ofst = 0x5;
        if(!tp->scr_sys_clk)
        {
            tmp = 0;
            if(config_param->sys_clk_trigger)
            {
                tp->scr_sys_clk = config_param->smc_sys_clk;
                if(tp->scr_sys_clk == sys_clk_166m)
                {
                     tmp = 0x3;
                }
                 else if(tp->scr_sys_clk == sys_clk_135m)
                 {
                     tmp = 0x1;
                 }
                 else if(tp->scr_sys_clk == sys_clk_154m)
                 {
                     tmp = 0x2;
                 }
                 else
                 {
                     tmp = 0x0;
                     tp->scr_sys_clk = 108000000;
                 }
                *((volatile UINT8 *)0xb800007a) &= ~(0x3<<1);
                *((volatile UINT8 *)0xb800007a) |= tmp<<1;
            }
            else
            {
                tmp = ((*((volatile UINT32 *)0xb8000078))>>17)&0x3;
                 if(tmp == num_0)
                 {
                     tp->scr_sys_clk = 108000000;
                 }
                 else if(tmp==num_1)
                 {
                     tp->scr_sys_clk = 135000000;
                 }
                 else if(tmp==num_2)
                 {
                     tp->scr_sys_clk = 154000000;
                 }
                 else
                 {
                     tp->scr_sys_clk = 166000000;
                 }
            }
            tp->pwm_sys_clk = tp->scr_sys_clk;
        }
    }
    if((ALI_S3602 ==tp->smc_chip_id)||
       (ALI_S3602F==tp->smc_chip_id)|| 
       (ALI_S3811 ==tp->smc_chip_id)|| 
       (ALI_S3503 ==tp->smc_chip_id)|| 
       (ALI_S3821 ==tp->smc_chip_id)||
       (ALI_C3505 ==tp->smc_chip_id)||
       (ALI_C3702 ==tp->smc_chip_id)||
       (ALI_C3503D==tp->smc_chip_id)||
       (ALI_C3711C==tp->smc_chip_id))
    {
        if(!tp->scr_sys_clk)
        {
            tp->scr_sys_clk = 108000000;
            tp->pwm_sys_clk = tp->scr_sys_clk;
        }
    }

    if(ALI_S3281==tp->smc_chip_id)
    {
        if(!tp->scr_sys_clk)
        {
            tp->scr_sys_clk = 108000000;
            tp->pwm_sys_clk = tp->scr_sys_clk;
        }
        smc_dev_set[0].io_base = 0xb8018800;
        smc_dev_set[0].irq = 20;
        smc_dev_set[1].io_base = 0xb8018800;
        smc_dev_set[1].irq = 20;
    }

    if(ALI_M3101==tp->smc_chip_id)
    {
        smc_dev_set[0].io_base = 0xb8001800;
        smc_dev_set[0].irq = 20;
        smc_dev_set[1].io_base = 0xb8001800;
        smc_dev_set[1].irq = 20;
        tp->scr_sys_clk = 108000000;
        tp->pwm_sys_clk = 108000000;
    }

}

static void check_hw_for_auto_tx_rx (struct smartcard_private *tp, struct smc_dev_config *config_param)
{
    if(ALI_M3329E==tp->smc_chip_id)
    {
        if(tp->smc_chip_version>=IC_REV_2)
        {
            tp->auto_tx_rx_triger = 1;
        }
        else
        {
            tp->force_tx_rx_triger = 1;
        }

        if(tp->smc_chip_version>=IC_REV_3)
        {
            tp->ts_auto_detect = 1;
        }

        if(tp->smc_chip_version>=IC_REV_5)
        {
            tp->invert_power = config_param->invert_power;
            tp->invert_detect = config_param->invert_detect;
        }
    }
    else if((ALI_M3101 ==tp->smc_chip_id)||
            (ALI_S3602F==tp->smc_chip_id)|| 
            (ALI_S3281 ==tp->smc_chip_id)|| 
            (ALI_S3811 ==tp->smc_chip_id)|| 
            (ALI_S3503 ==tp->smc_chip_id)|| 
            (ALI_S3821 ==tp->smc_chip_id)||
            (ALI_C3505 ==tp->smc_chip_id)||
            (ALI_C3702 ==tp->smc_chip_id)||
            (ALI_C3503D==tp->smc_chip_id)||
            (ALI_C3711C==tp->smc_chip_id))
    {
        tp->invert_power = config_param->invert_power;
        tp->invert_detect = config_param->invert_detect;
        tp->auto_tx_rx_triger = 1;
        tp->ts_auto_detect = 1;
    }

    if(sys_ic_is_m3202())
    {
        if(tp->smc_chip_version>=IC_REV_2)
        {
            tp->auto_tx_rx_triger = 1;
            tp->invert_power = config_param->invert_power;
            tp->invert_detect = config_param->invert_detect;
            if(tp->smc_chip_version>IC_REV_2)
            {
                 tp->ts_auto_detect = 1;
            }
        }
        else
        {
            tp->force_tx_rx_triger = 1;
        }
    }
    if(ALI_S3602==tp->smc_chip_id)
    {
        if(tp->smc_chip_version>=IC_REV_2)
        {
            tp->auto_tx_rx_triger = 1;
        }
    }
}
#ifdef _M3702_
#define XSC_1_PRESJ_PINMUX_REG  	0xB808B488
#define XSC_1_RST_PINMUX_REG 		0xB808B48C
#define XSC_1_DATA_PINMUX_REG 		0xB808B490
#define XSC_1_POWENJ_PINMUX_REG 	0xB808B494
#define XSC_1_CLK_PINMUX_REG 		0xB808B498

#define XSC_2_PRESJ_PINMUX_REG  	0xB808B81C
#define XSC_2_RST_PINMUX_REG 		0xB808B820
#define XSC_2_DATA_PINMUX_REG 		0xB808BC00
#define XSC_2_POWENJ_PINMUX_REG 	0xB808BC04
#define XSC_2_CLK_PINMUX_REG 		0xB808BC08
#endif

/******************************************************************************************************
 *     Name        :    smc_dev_attach()
 *    Description    :    Smart card reader init funciton.
 *    Parameter    :    int dev_id        : Index of smart card slot.
                                struct smc_dev_config *config_param  :smc config info
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_attach(int dev_id, struct smc_dev_config *config_param)
{
    struct smc_device *dev = NULL;
    struct smartcard_private *tp = NULL;
    void *priv_mem = NULL;
	
	
#ifdef _M3702_
	UINT32 data_long=0x00;
#ifdef _BOARD_DB_M3712L_01V01_
		data_long=(*(volatile UINT32 *)(XSC_1_PRESJ_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_1_PRESJ_PINMUX_REG) = data_long;
		
		data_long=(*(volatile UINT32 *)(XSC_1_RST_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_1_RST_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_1_DATA_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_1_DATA_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_1_POWENJ_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_1_POWENJ_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_1_CLK_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_1_CLK_PINMUX_REG) = data_long;
#else
		data_long=(*(volatile UINT32 *)(XSC_2_PRESJ_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_2_PRESJ_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_2_RST_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_2_RST_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_2_DATA_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_2_DATA_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_2_POWENJ_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_2_POWENJ_PINMUX_REG) = data_long;

		data_long=(*(volatile UINT32 *)(XSC_2_CLK_PINMUX_REG));
		data_long = data_long | 0x00000002;
		*(volatile UINT32 *)(XSC_2_CLK_PINMUX_REG) = data_long;
#endif
#endif

    if((0x80000000!=(((UINT32)config_param)&0xf0000000))&& \
        (0xa0000000!=(((UINT32)config_param)&0xf0000000))&& \
        (0xb0000000!=(((UINT32)config_param)&0xf0000000)))
    {
        return !SUCCESS;
    }

    if(0 == dev_id)
    {
        smc_dev_name[STRLEN(smc_dev_name) - 1] = '0';
    }
    else if(1 == dev_id)
    {
        if(ALI_M3329E == sys_ic_get_chip_id())
        {
            if(sys_ic_get_rev_id()>=IC_REV_5)
            {
                return ERR_FAILUE;
            }
        }
        smc_dev_name[STRLEN(smc_dev_name) - 1] = '1';
    }
    else
    {
        return ERR_FAILUE;
    }

    SMC_PRINTF("SMC MAIN: %s\n",smc_dev_name);

    dev = (struct smc_device *)dev_alloc(smc_dev_name, HLD_DEV_TYPE_SMC,sizeof(struct smc_device));

    if (NULL == dev)
    {
        PRINTF("Error: Alloc smart card reader error!\n");
        return ERR_NO_MEM;
    }
    smc_dev_set[dev_id].dev = dev;
    /* Alloc structure space of private */
    priv_mem = (void *)malloc(sizeof(struct smartcard_private));

    ASSERT(priv_mem != NULL);
    if(priv_mem != NULL)
    {
        MEMSET(priv_mem, 0, sizeof(struct smartcard_private));
    }

    dev->priv = priv_mem;

    tp = (struct smartcard_private *)dev->priv;
    tp->atr_info = (atr_t *)malloc(sizeof(atr_t));
    ASSERT(tp->atr_info!=NULL);
    if(tp->atr_info!=NULL)
    {
        MEMSET(tp->atr_info, 0, sizeof(atr_t));
    }

    smc_dev_priv_init(tp);
    smc_dev_binding_set(tp);

    if(0==dev_id)
    {
        tp->hsr = smc_dev_hsr_0;
    }
    else
    {
        tp->hsr = smc_dev_hsr_1;
    }

    if(config_param->init_clk_trigger&&
        config_param->init_clk_number&&
        config_param->init_clk_array)
    {
        tp->init_clk_number = config_param->init_clk_number;
        tp->init_clk_array = (UINT32 *)MALLOC(tp->init_clk_number*sizeof(UINT32));
        ASSERT(NULL!=tp->init_clk_array);
        if(NULL!=tp->init_clk_array)
        {
            MEMCPY(tp->init_clk_array, config_param->init_clk_array, (tp->init_clk_number*sizeof(UINT32)));
        }
    }
    else
    {
        tp->init_clk_number = 1;
        tp->init_clk_array = &(tp->smc_clock);
        tp->smc_clock = DFT_WORK_CLK;
    }
    if(config_param->class_selection_supported&&
        (config_param->board_supported_class&0x7)&&
        (NULL!=config_param->class_select))
    {
        tp->class_selection_supported = 1;
        tp->board_supported_class = (config_param->board_supported_class&0x7);
        tp->class_select = config_param->class_select;
    }
    if(config_param->gpio_cd_trigger)
    {
        tp->use_gpio_cd = 1;
        tp->gpio_cd_io = config_param->gpio_cd_io;
        tp->gpio_cd_pol = config_param->gpio_cd_pol;
        tp->gpio_cd_pos = config_param->gpio_cd_pos;
    }
    if(config_param->gpio_vpp_trigger)
    {
        tp->use_gpio_vpp = 1;
        if((ALI_M3101==tp->smc_chip_id)\
            ||((ALI_M3329E==tp->smc_chip_id) && (tp->smc_chip_version>=IC_REV_3)) \
            ||(tp->smc_chip_id>=ALI_S3602F))
        {
            tp->internal_ctrl_vpp = 1;
        }
        tp->gpio_vpp_io = config_param->gpio_vpp_io;
        tp->gpio_vpp_pol = config_param->gpio_vpp_pol;
        tp->gpio_vpp_pos = config_param->gpio_vpp_pos;
    }
    if(config_param->def_etu_trigger)
    {
        tp->use_default_etu = 1;
        tp->default_etu = config_param->default_etu;
    }

    /* check HW auto TX/RX */
    check_hw_for_auto_tx_rx (tp, config_param);
    tp->parity_disable = config_param->parity_disable_trigger;
    tp->parity_odd = config_param->parity_odd_trigger;
    tp->apd_disable = config_param->apd_disable_trigger;
    tp->warm_reset_enable = config_param->warm_reset_trigger;
    tp->disable_pps = config_param->disable_pps;
    tp->ext_atr_delay = config_param->ext_atr_delay;

    smc_dev_default_setting(tp, config_param);

    dev->base_addr = smc_dev_set[dev_id].io_base;
    dev->irq = smc_dev_set[dev_id].irq;

    /* Function point init */
    dev->open = smc_dev_open;
    dev->close = smc_dev_close;
    dev->card_exist = smc_dev_card_exist;
    if(tp->class_selection_supported)
    {
        dev->reset = smc_dev_multi_class_reset;
    }
    else
    {
        dev->reset = smc_dev_reset;
    }
    dev->deactive = smc_dev_deactive;
    dev->raw_read = smc_dev_read;
    dev->raw_write = smc_dev_write;
    dev->raw_fifo_write = smc_dev_write;
    //dev->transmit = smc_dev_transfer_data;
    dev->iso_transfer = smc_dev_iso_transfer;
    dev->iso_transfer_t1 = smc_dev_iso_transfer_t1;
    dev->do_ioctl = smc_dev_ioctl;
    dev->t1_transfer = t1_transceive;
    dev->t1_xcv = t1_xcv;
    dev->t1_negociate_ifsd = t1_negociate_ifsd;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        SMC_PRINTF("Error: Register smart card reader device error!\n");
        FREE(priv_mem);
        dev_free(dev);
    }
    SMC_PRINTF("SMC MAIN: attach ok\n");
    return SUCCESS;
}


void smc_set_wclk(struct smartcard_private *tp, UINT32 ioaddr, UINT32 clk)
{
    UINT32 scr_div_inte = 0;
    UINT32 pwm_div_inte = 0;
    UINT32 scr_div_fract = 0;
    UINT32 pwm_div_fract = 0;
    UINT32 dev_id = 0;
    double db_temp = 0.0;
    UINT8  fract = 0;

    if(ioaddr==smc_dev_set[0].io_base)
    {
        dev_id = 0;
    }
    else
    {
        dev_id = 1;
    }
    db_temp = ((double)tp->scr_sys_clk)/((double)clk);
    scr_div_inte = (UINT32)(db_temp*100);
    scr_div_fract = scr_div_inte%100;
    scr_div_inte = scr_div_inte/100;
    if(scr_div_fract)
    {
        db_temp = ((double)tp->pwm_sys_clk)/((double)clk);
        pwm_div_inte = (UINT32)(db_temp*100);
        pwm_div_fract = pwm_div_inte%100;
        pwm_div_inte = pwm_div_inte/100;
        fract = 100/pwm_div_fract;

        if(ALI_M3329E==tp->smc_chip_id)
        {
            OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_gpio_ofst, 0);
        }
        OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1);
        OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));
        OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_seh_ofst, pwm_div_inte>>1);
        OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_sel_ofst, (pwm_div_inte>>1)+(pwm_div_inte&0x1));
        OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_frac_ofst, fract);
        OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0x81);
        if(((ALI_M3329E==tp->smc_chip_id) && (tp->smc_chip_version>=IC_REV_5))\
            || (tp->smc_chip_id>=ALI_S3602F) || (ALI_M3101==tp->smc_chip_id))
        {
            OUTPUT_UINT8(ioaddr + REG_CLK_FRAC, fract);
        }
    }
    else
    {
        if(ALI_M3329E==tp->smc_chip_id)
        {
            OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_gpio_ofst, 0);
        }
        OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0);
        OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1);
        OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));
    }

}

void smc_init_hw(struct smc_device *dev)
{
    UINT32 i = 0;
    UINT32 rst_msk = 0;
    UINT32 sys_rst_addr = 0xb8000000;
    UINT32 ioaddr = dev->base_addr;
    UINT32 cnt_2000 = 2000;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
     UINT8 temp_val =  INPUT_UINT8(ioaddr + REG_CLK_VPP);
    UINT8 temp = 0;

    if(ALI_M3329E == tp->smc_chip_id)
    {
        if(dev->base_addr == smc_dev_set[0].io_base)
        {
            rst_msk = 1<<23;
        }
        else
        {
            rst_msk = 1<<24;
        }
        sys_rst_addr = 0xb8000060;
    }
    if(1 == sys_ic_is_m3202())
    {
        if(dev->base_addr == smc_dev_set[0].io_base)
        {
            rst_msk = 1<<15;
        }
        else
        {
            rst_msk = 1<<28;
        }
        sys_rst_addr = 0xb8000060;
    }
    if((ALI_S3602  == tp->smc_chip_id)|| 
       (ALI_S3602F == tp->smc_chip_id)|| 
       (ALI_S3811  == tp->smc_chip_id)|| 
       (ALI_S3503  == tp->smc_chip_id)|| 
       (ALI_S3821  == tp->smc_chip_id)||
       (ALI_C3505  == tp->smc_chip_id)||
       (ALI_C3702  == tp->smc_chip_id)||
       (ALI_C3503D == tp->smc_chip_id)||
       (ALI_C3711C == tp->smc_chip_id))
    {
        if(dev->base_addr == smc_dev_set[0].io_base)
        {
            rst_msk = 1<<20;
        }
        else
        {
            rst_msk = 1<<21;
        }
        sys_rst_addr = 0xb8000080;
    }
    if((ALI_M3101 == tp->smc_chip_id)||(ALI_S3281 == tp->smc_chip_id))
    {
        rst_msk = 1<<15;
        sys_rst_addr = 0xb8000060;
    }
    *((volatile UINT32 *)sys_rst_addr) |= rst_msk;
    osal_delay(3);
    *((volatile UINT32 *)sys_rst_addr) &= ~rst_msk;
    tp->smc_clock = DFT_WORK_CLK;     /* Set default clk, is necessary. */
    tp->smc_clock = tp->init_clk_array[0];
    if(tp->use_default_etu)
    {
        tp->smc_etu = tp->default_etu;
    }
    else
    {
        tp->smc_etu = DFT_WORK_ETU;
    }
    tp->inverse_convention = 0;
    SMC_PRINTF("SMC MAIN: init smc regiseter\n");
    if(tp->ts_auto_detect)
    {
        OUTPUT_UINT8(ioaddr + REG_DEV_CTRL, 0x10|(tp->invert_detect<<5)|(tp->invert_power<<6));
    }
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x00);
    //osal_delay(2000);
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x80|(tp->parity_odd<<4)); /* enable SCR interface */
    if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
    {
        OUTPUT_UINT8(ioaddr + REG_ICCR, 0x1|(tp->parity_disable<<5)|(tp->apd_disable<<4));
    }
    else
    {
        OUTPUT_UINT8(ioaddr + REG_ICCR, 0x41|(tp->parity_disable<<5)|(tp->apd_disable<<4));     // power off
    }
     if((tp->use_gpio_vpp) && (tp->internal_ctrl_vpp))
     {
         temp_val &= 0xf3;
         temp_val |= (tp->gpio_vpp_pol<<3);
         if((tp->smc_chip_id>=ALI_S3602F)||(ALI_M3101==tp->smc_chip_id))
        {
        /*
        //s3602f SmartCard interface auto disable clock function has problem
        while meet the smart card without parity bit such as C1800A.
        need disable this function for s3602f
        */
            temp_val |= 0x10;
        }
         OUTPUT_UINT8(ioaddr + REG_CLK_VPP, temp_val);
         OUTPUT_UINT8(ioaddr + REG_VPP_GPIO, (tp->gpio_vpp_pos&0x3f)|0x80);
     }
    if((sys_ic_is_m3202())&&(IC_REV_2 == tp->smc_chip_version))
    {
        temp = *((volatile UINT8 *)0xb800001d);
        temp &= ~((1<<6)|(1<<7));
        temp |= ((tp->invert_power<<7)|(tp->invert_detect <<6));
        *((volatile UINT8 *)0xb800001d) = temp;
    }

    /* enable interrupt */
    OUTPUT_UINT8(ioaddr + REG_IER0, 0x7f); /* enable receive interrupt, enable wait timeout interrupt */
    OUTPUT_UINT8(ioaddr + REG_IER1, 0xff); /* detect Card inserting or removal interrupt enable */
    i = 0;
    while(tp->invert_detect)
    {
        if(INPUT_UINT8(ioaddr + REG_ISR1) & (SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT))
        {
            SMC_PRINTF("SMC MAIN: i %d: %02x, isr0: %02x, isr1: %02x\n", i, INPUT_UINT8(ioaddr + REG_DEV_CTRL), \
                                      INPUT_UINT8(ioaddr + REG_ISR0), INPUT_UINT8(ioaddr + REG_ISR1));
            OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT);
            break;
        }
        osal_delay(1);
        i++;
        if(i>cnt_2000)
        {
            break;
        }
    }
    OUTPUT_UINT8(ioaddr + REG_PDBR, 0xff);        //set de-bounce to 15
    OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);        //set etu
    OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12        //set gtr to 12
    OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);//9600    //set wt to 9600
    OUTPUT_UINT8(ioaddr + REG_RCVPR, 0x49);        //set value1_r to 4 and glitch_v to 3

    /* This patch will fix only M3603 IC issue */
    if(ALI_S3602F == tp->smc_chip_id)
    {
        OUTPUT_UINT8(ioaddr + REG_RXTX_PP, 0x71);    //set rxpp to 1 and txpp to 7
    }
    else
    {
        OUTPUT_UINT8(ioaddr + REG_RXTX_PP, 0x77);    //set rxpp to 7 and txpp to 7
    }
    smc_set_wclk(tp, ioaddr, tp->smc_clock);
}
/******************************************************************************************************
 *     Name        :    smc_dev_open()
 *    Description    :    Smart card reader open funciton.
 *    Parameter    :    struct smc_device *dev        : Devcie handle.
 *                void (*callback)        : Callback function.
 *    Return        :    INT32                : SUCCESS.
 *
 ******************************************************************************************************/
static INT32 smc_dev_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
    struct smartcard_private *tp = NULL;
    UINT32 ioaddr = 0;
    UINT8 status = 0;
    UINT32 gpio_cd_pos_64 = 64;
    UINT32 gpio_cd_pos_32 = 32;
    UINT32 gpio_int_irq = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    ioaddr = dev->base_addr;
    SMC_PRINTF("SMC MAIN: create smc semaphore\n");
    tp->smc_sema_id = OSAL_INVALID_ID;
    tp->smc_sema_id = osal_semaphore_create(1);
    if(OSAL_INVALID_ID == tp->smc_sema_id)
    {
        ASSERT(0);
    }
    tp->smc_flag_id = OSAL_INVALID_ID;
    tp->smc_flag_id = osal_flag_create(0);
    if(OSAL_INVALID_ID == tp->smc_flag_id)
    {
        ASSERT(0);
    }
    //tp->smc_tx_buf = (UINT8 *)MALLOC(SMC_TX_BUF_SIZE);
    //    ASSERT(tp->smc_tx_buf!=NULL);
    tp->smc_rx_tmp_buf = (UINT8 *)MALLOC(SMC_RX_BUF_SIZE);
    ASSERT(tp->smc_rx_tmp_buf!=NULL);
    if(tp->smc_rx_tmp_buf!=NULL)
    {
        tp->smc_rx_tmp_buf = (UINT8 *)((((UINT32)(tp->smc_rx_tmp_buf))\
                                            &0x1fffffff)|0xa0000000); 
    }
    dev->callback = callback;
    if(tp->use_gpio_vpp)
    {
        HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
        HAL_GPIO_BIT_DIR_SET(tp->gpio_vpp_pos, tp->gpio_vpp_io);
    }
    /* when open the card, init the T1 parameter */
    t1_init(&tp->T1);
    smc_dev_deactive(dev);
    smc_init_hw(dev);
    if(tp->use_gpio_cd)
    {
        HAL_GPIO_INT_SET(tp->gpio_cd_pos, 1);
        HAL_GPIO_INT_REDG_SET(tp->gpio_cd_pos, 1);
        HAL_GPIO_INT_FEDG_SET(tp->gpio_cd_pos, 1);
        HAL_GPIO_INT_CLEAR(tp->gpio_cd_pos);
        if(ALI_S3602 == tp->smc_chip_id)
        {
            gpio_int_irq = 8;
        }
        else
        {
            if((UINT16)tp->gpio_cd_pos < gpio_cd_pos_32)
            {
                gpio_int_irq = 9;
            }
            else
            {
                if(ALI_M3329E == tp->smc_chip_id)
                {
                    if((UINT16)tp->gpio_cd_pos<gpio_cd_pos_64)
                    {
                        gpio_int_irq = 24;
                    }
                    else
                    {
                        gpio_int_irq = 31;
                    }
                }
                else
                {
                    gpio_int_irq = 24;
                }
            }
        }
        HAL_GPIO_BIT_DIR_SET(tp->gpio_cd_pos,  tp->gpio_cd_io);
        osal_interrupt_register_lsr(gpio_int_irq, smc_gpio_detect_lsr, (UINT32)dev);
    }
    //else
    {
        status = INPUT_UINT8(ioaddr + REG_ICCSR);

        SMC_PRINTF("SMC MAIN: to check card insert 0x80 : %x\n",status);
        if((status & 0x80) != 0)
        {
            SMC_PRINTF("SMC MAIN: Card insert\n");
            tp->inserted = 1;
            dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
            OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_INSERT);
            if(dev->callback != NULL)
            {
                osal_interrupt_register_hsr((OSAL_T_HSR_PROC_FUNC_PTR)(dev->callback), tp->inserted);
            }
        }
    }
    SMC_PRINTF("SMC MAIN: register interrupt server\n");
    osal_interrupt_register_lsr(dev->irq + 8, smc_dev_interrupt, (UINT32)dev);

    SMC_PRINTF("SMC MAIN: smc dev open ok\n");

    return RET_SUCCESS;
}

/******************************************************************************************************
 *     Name        :    smc_dev_close()
 *    Description    :    Smart card reader controler close funciton.
 *    Parameter    :    struct smc_device *dev        : Devcie handle
 *    Return        :    INT32                : SUCCESS.
 *
 ******************************************************************************************************/
static INT32 smc_dev_close(struct smc_device *dev)
{
    struct smartcard_private *tp = NULL;
    UINT32 ioaddr = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    ioaddr = dev->base_addr;
    tp->inserted = 0;
    tp->reseted = 0;
    /* Disable interrupt */
    OUTPUT_UINT8(ioaddr + REG_IER0, 0x00);
    OUTPUT_UINT8(ioaddr + REG_IER1, 0x00);
    osal_interrupt_unregister_lsr(dev->irq+ 8, smc_dev_interrupt);
    smc_dev_deactive(dev);
    osal_semaphore_delete(((struct smartcard_private *)dev->priv)->smc_sema_id);
    osal_flag_delete(((struct smartcard_private *)dev->priv)->smc_flag_id);   

    if (NULL != tp->smc_rx_tmp_buf)
    {
        FREE((void *)tp->smc_rx_tmp_buf);
        tp->smc_rx_tmp_buf = NULL;
    }        

    return RET_SUCCESS;
}

static void smc_set_etu(struct smc_device *dev, UINT32 new_etu)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;
    UINT8 atr_fi = 1;
    UINT8 atr_di = 1;
    UINT8 atr_wi = ATR_DEFAULT_WI;
    UINT8 bwi = tp->BWI;

    if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
    {
        atr_fi = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
        atr_di =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
    }
    if(tp->reseted)
    {
        if(0 == tp->T)
        {
            if (tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TC].present)
            {
                atr_wi = tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TC].value;
            }

            OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*atr_wi*atr_di);
        }
        else if(1 == tp->T)
        {
            OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<bwi)*atr_di)/atr_fi));
        }
        tp->smc_etu = new_etu;
		tp->reset_etu_flag = 0;
        OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);
    }
	else
	{
		tp->reset_etu_flag = 1;
	}

}
/******************************************************************************************************
 *     Name        :    smc_dev_ioctl()
 *    Description    :    Smart card reader IO control function.
 *    Parameter    :    struct smc_device *dev        : Devcie handle
 *                INT32 cmd            : IO command
 *                UINT32 param            : Command parameter
 *    Return        :    INT32                : return value
 *
 ******************************************************************************************************/
static INT32 smc_dev_ioctl(struct smc_device *dev, INT32 cmd, UINT32 param)
{
    INT32 ret_code = RET_SUCCESS;
    struct smartcard_private *tp = NULL;
    UINT8 len = 0;
    struct smc_hb_t *p_hb = (struct smc_hb_t *)param;
    UINT8 atr_len = 0;
    struct atr_t *p_atr = (struct atr_t *)param;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    len = tp->atr_info->hbn;
    atr_len = tp->atr_info->length;
    switch (cmd)
    {
        case SMC_DRIVER_SET_IO_ONOFF:
            if(SMC_IO_ON == param)
            {
                smc_dev_enable();
            }
            else if(SMC_IO_OFF == param)
            {
                smc_dev_disable();
            }
            break;
        case SMC_DRIVER_CHECK_STATUS:
            if(0 == tp->inserted)
            {
                *(UINT8 *) param = SMC_STATUS_NOT_EXIST;
            }
            else if(0 == tp->reseted)
            {
                *(UINT8 *) param = SMC_STATUS_NOT_RESET;
            }
            else
            {
                *(UINT8 *) param = SMC_STATUS_OK;
            }
            break;
        case SMC_DRIVER_SET_WWT:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            tp->first_cwt = param;
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_SET_CWT:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            tp->cwt = param;
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_SET_ETU:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            smc_set_etu(dev, param);
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_F:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            if(!tp->reseted)
            {
                ret_code = !RET_SUCCESS;
            }
            else
            {
                *((UINT32 *)param) = tp->F;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_D:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            if(!tp->reseted)
            {
                ret_code = !RET_SUCCESS;
            }
            else
            {
                *((UINT32 *)param) = tp->D;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_ATR_RESULT:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            *((enum smc_atr_result *)param) = tp->atr_rlt;
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_PROTOCOL:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            if((tp->inserted) && (tp->reseted))
            {
                *((UINT32 *)param) = tp->T;
            }
            else
            {
                *((UINT32 *)param) = 0xffffffff;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_HB:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            {
                if(len>(sizeof(struct smc_hb_t)-1))
                {
                    len = sizeof(struct smc_hb_t)-1;
                }
                MEMCPY((void *)p_hb->hb, tp->atr_info->hb, len);
                p_hb->hbn = len;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_SET_WCLK:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            tp->init_clk_array[tp->init_clk_idx] = param;
            osal_semaphore_release(tp->smc_sema_id);
            break;
		case SMC_DRIVER_GET_WCLK:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            *((UINT32 *)param) = tp->init_clk_array[tp->init_clk_idx];
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_CLASS:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            if((tp->class_selection_supported)&&(!tp->reseted))
            {
                *((enum class_selection *)param) = tp->smc_current_select;
            }
            else
            {
                ret_code = !RET_SUCCESS;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_SET_CLASS:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            if((tp->class_selection_supported)&&(!tp->reseted))
            {
                tp->smc_current_select = (enum class_selection )param;
            }
            else
            {
                ret_code = !RET_SUCCESS;
            }
            osal_semaphore_release(tp->smc_sema_id);
            break;
        case SMC_DRIVER_GET_ATR:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            {
                MEMCPY((void *)p_atr, tp->atr_info, atr_len);
            }
            osal_semaphore_release(tp->smc_sema_id);
			break;
        case SMC_DRIVER_SET_RESET_MODE:
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            tp->warm_reset_enable = !!(param);
            osal_semaphore_release(tp->smc_sema_id);
            break;
			
		//begin:add on 20151021
		case SMC_DRIVER_SET_PROTOCOL:
		{			
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if (tp->inserted && tp->reseted)
			{
               	tp->T = param;
				SMC_PRINTF("[%s]line=%d, protocol_type=%d!\n", __FUNCTION__, __LINE__, tp->T);
				
				if (ATR_PROTOCOL_TYPE_T1==tp->T)
				{
					smc_set_protocol_config_T_1(tp);					
				}
				else if(ATR_PROTOCOL_TYPE_T0==tp->T)
				{
					smc_set_protocol_config_T_0(tp);
				}
			}
			else 
			{
                ret_code = !RET_SUCCESS;
                SMC_PRINTF("card is not inserted or reseted!\n");
			}
			osal_semaphore_release(tp->smc_sema_id);
			break;
		}
		//end:20151021
		case SMC_DRIVER_DISABLE_PPS:
		{
            osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
            tp->disable_pps = (param & 0x00000001)? 1 : 0;
            osal_semaphore_release(tp->smc_sema_id);
			break;
		}
		
        default:
            break;
    }

    return ret_code;
}


