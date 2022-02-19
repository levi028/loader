#include "chip_m3711c.h"
#include "../chip.h"
#include <hld/sec_boot/sec_boot.h>
//=================================================================================//

typedef struct
{
	chip_init_param param;
}ChipPriv;

extern void phy_chip_setting(void);
extern void nim_s3501_reset_tuner(void);
//=================================================================================//

static void GPIO_enable_pos(UINT32 pos, BOOL enable_flag)
{
	UINT32 gpio_enable_reg = GPIO_REG;
	UINT32 reg_idx = 0;
	UINT32 data = 0;

	if(pos >= 160)
	{
		return;
	}

	reg_idx = pos >> 5;
	gpio_enable_reg += reg_idx * 4;
	pos &= 0x1F;

	data = *((volatile UINT32 *)gpio_enable_reg);
	if (enable_flag)
	{
		data |= (1 << pos);
	}
	else
	{
		data &= ~(1 << pos);
	}

	*((volatile UINT32 *)gpio_enable_reg) = data;
}

static void GPIO_enable_group(UINT32 grp_idx, UINT32 pos_mask, BOOL enable_flag)
{
	UINT32 gpio_enable_reg = GPIO_REG;
	UINT32 data = 0;

	if(grp_idx >= 5)
	{
		return;
	}

	gpio_enable_reg += grp_idx * 4;
	data = *((volatile UINT32 *)gpio_enable_reg);
	if (enable_flag)
	{
		data |= pos_mask;
	}
	else
	{
		data &= ~pos_mask;
	}

	*((volatile UINT32 *)gpio_enable_reg) = data;
}

static void m3711c_pin_mux_set(chip* thiz, pin_mux_info* pin_mux_info)
{
	UINT32 data = 0;
	UINT32 pin_mux_sel = pin_mux_info->pin_mux_sel;
	UINT32 val = pin_mux_info->value;
	UINT32 pin_addr = PIM_MUX_BASE + ((pin_mux_sel & PIN_MUX_REG_MASK) >> PIN_MUX_REG_SHIFT);
	UINT32 bit_height = (pin_mux_sel & PIN_BIT_HEIGHT_MASK) >> PIN_BIT_HEIGHT_SHIFT;
	UINT32 bit_low = (pin_mux_sel & PIN_BIT_LOW_MASK) >> PIN_BIT_LOW_SHIFT;
	UINT32 i = 0;

	if(bit_height == 0x00)
	{
		bit_height=bit_low;
	}

	if(bit_low > bit_height)
	{
		data=bit_low;
		bit_low=bit_height;
		bit_height=data;
	}

	data = *(volatile UINT32 *)pin_addr;
	for(i=bit_low; i<=bit_height; i++)
	{
		data &= ~(1 << i);
		data |= (val << i);
	}
	*(volatile UINT32*)(pin_addr) = data;

#ifdef SMC2_SUPPORT
	/*set SCR2 GPIO.*/
	unsigned int scr2_gpio = 0;

	if(!(((*(volatile UINT32 *)0xB8000070) >>19) & 0x1))
	{
		scr2_gpio= *(volatile unsigned long*)(0xB8000430);/*Disable GPIO[10]~[14].*/
		*(volatile unsigned long*)(0xB8000430) = scr2_gpio &(~0x7C00);
	}	
#endif
}

static void m3711c_pin_mux_set_ext(chip* thiz, pin_mux_info pin_mux_info[], UINT8 count)
{
	UINT8 i = 0;

	for(i=0; i<count; i++)
	{
		m3711c_pin_mux_set(thiz, &pin_mux_info[i]);
	}
}

static void m3711c_gpio_dir_set(chip* thiz, gpio_info_t* gpio_info)
{
	HAL_GPIO_BIT_DIR_SET(gpio_info->position, gpio_info->io);
}

static void m3711c_gpio_dir_set_ext(chip* thiz, gpio_info_t gpio_info[], UINT8 count)
{
	volatile unsigned int j = 0;
	volatile gpio_info_t* gpio;

	for(j=0; j<count; j++)
	{
		gpio = &gpio_info[j];
		HAL_GPIO_BIT_DIR_SET(gpio->position, gpio->io);
	}
}

static void m3711c_gpio_pin_init(chip* thiz, gpio_info_t* gpio_info, BOOL enabled)
{
	GPIO_enable_pos(gpio_info->position, enabled);
	if(enabled)
	{
		HAL_GPIO_BIT_DIR_SET(gpio_info->position, gpio_info->io);
	}
}

static void m3711c_gpio_pin_init_ext(chip* thiz, gpio_info_t gpio_info[], UINT8 count)
{
	UINT32 reg_addr[] = {GPIO_REG, GPIOA_REG, GPIOB_REG, GPIOC_REG};
	UINT32 i =0, j = 0, value = 0;
	gpio_info_t* gpio;

	/*disable all GPIO.*/
	for(i=0; i<ARRAY_SIZE(reg_addr); i++)
	{
		value = 0;
		for(j=0; j<count; j++)
		{
			gpio = &gpio_info[j];
			if(gpio->position >= (INT16)(i*32) && gpio->position < (INT16)((i+1)*32))
			{
				value |= 1 << (gpio->position - (i*32));
			}
		}
		*((volatile unsigned long *)reg_addr[i]) = value;
	}

	m3711c_gpio_dir_set_ext(thiz, gpio_info, count);
}

static void m3711c_gpio_set(chip* thiz, gpio_info_t* gpio_info, UINT8 value)
{
	__MAYBE_UNUSED__ UINT8 val = 0;

	if(gpio_info->io != HAL_GPIO_O_DIR)
	{
		return;
	}
	val = gpio_info->polar ? value : !value; 
	HAL_GPIO_BIT_SET(gpio_info->position, val);
}

static UINT32 m3711c_gpio_get(chip* thiz, gpio_info_t* gpio_info)
{
	UINT32 val = HAL_GPIO_BIT_GET(gpio_info->position);
	return gpio_info->polar ? val : !val;
}

static void m3711c_tsi_pin_cfg(chip* thiz, UINT8 tsi_id, BOOL clear)
{
	return;

	static UINT32 config_array[][2] = 
	{
		{TSI_SPI_1, (1<<22)}, {TSI_SSI_0, (1<<23)},
		{TSI_SSI_1, (1<<24)}, {TSI_SSI2B_0, (1<<25)},
		{TSI_SSI2B_1, (1<<26)},
	};

	UINT32 i = 0;
	UINT32 data = *(volatile unsigned long *)(0xB8000088);

	if(clear)
	{
		data &= 0xF83FFFFF;
	}

	for(i=0; i<ARRAY_SIZE(config_array); i++)
	{
		if(tsi_id == config_array[i][0])
		{
			data |= config_array[i][1];
			*(volatile unsigned long *)(0xB8000088) = data;
			break;
		}
	}
}

static void m3711c_ci_power(chip* thiz, BOOL power_on)
{
	//ChipPriv* priv = (ChipPriv*)thiz->priv;
}

static void phy_chip_enable(void)
{
	phy_chip_setting();
}

#ifdef DUAL_ENABLE
//#define MAX_SEE_LEN (1000000)/*10M.*/
static RET_CODE set_see_parameter(UINT32 run_addr, UINT32 code_addr, UINT32 len)
{
	*(volatile UINT32 *)(SEEROM_SEE_REAL_RUN_ADDR) = run_addr |0xA0000000;
	osal_cache_flush((UINT8*)code_addr,len);
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = *(volatile UINT32 *)(0xA70001F0);
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = code_addr|0xA0000000;
	*(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = 0xA7000200;

	return RET_SUCCESS;
}

static void see_boot(UINT32 addr)
{
	/*run from AS bl.*/
	if(is_hw_ack_flag_true(SEE_ROM_RUN_BIT))
	{
		set_main_trig_sw_run();
		osal_delay(100);

		/*wait see software run flag*/
		while(1)
		{
			if((*(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR)) == SEE_SW_RUN_FLAG)
			{
				break;
			}
		}

		return;
	}
	
	if((*(volatile UINT32 *)(0xB8000200) != 0xB8000280) \
		&& ((*(volatile UINT32 *)(0xB8000220) & 0x2) != 0))
	{
		return;
	}
	
	if(sys_ic_seerom_is_enabled())
	{
		addr = (addr & 0x0FFFFFFF) | 0xA0000000;
		set_see_parameter(addr, addr, MAX_SEE_LEN);/*double check if operate addr.*/
		*(volatile UINT32 *)(0xB8000200) = addr; 
		*(volatile UINT32 *)(0xB8000220) |= 0x2; 
	}
	else
	{
		addr = (addr & 0x0FFFFFFF) | 0xA0000000;
		*(volatile UINT32 *)(0xB8000200) = addr;
		*(volatile UINT32 *)(0xB8000220) |= 0x2;
	}	

	osal_task_sleep (1000);
}
#endif

static void m3711c_init(chip* thiz, chip_init_param* param)
{
	//UINT32 data = 0;
    UINT32 chip_id = 0;
	ChipPriv* priv = (ChipPriv*)thiz->priv;

	chip_id = sys_ic_get_chip_id();
	if(ALI_C3711C != chip_id)
	{
		ASSERT(0);
	}

	MEMCPY(&priv->param, param, sizeof(chip_init_param));
#ifdef DUAL_ENABLE
	#if(defined _CAS9_VSC_ENABLE_ || defined _C0700A_VSC_ENABLE_)
		see_boot(osal_dual_get_see_run_address() + 0xB0000);
	#else
		see_boot(osal_dual_get_see_run_address() + __MM_VSC_BUF_LEN + __MM_SEE_BL_BUF_LEN);
	#endif
#else
	/*B8000220, byte = 0, disable SEE.*/
	*(volatile UINT8 *)(0xb8000220) = 0;
#endif

#if ((SYS_PROJECT_FE == PROJECT_FE_DVBS) || (SYS_PROJECT_FE == PROJECT_FE_DVBS2) \
	|| (SYS_PROJECT_FE == PROJECT_FE_DTMB))
	*(UINT32 *)(0xB8000004) = 1;
#else
	*(UINT32 *)(0xB8000004) = 0;
#endif

	/*disabled all gpio.*/
	*(unsigned long *)(GPIO_REG) = 0;/*GPIO 0~31.*/
	*(unsigned long *)(GPIOA_REG) = 0;/*GPIO 32~63.*/
	*(unsigned long *)(GPIOB_REG) = 0;/*GPIO 64~95.*/
	*(unsigned long *)(GPIOC_REG) = 0;/*GPIO 96~127.*/
	*(unsigned long *)(GPIOD_REG) = 0;

	if(param->ali_ethmac_enabled)
	{
		phy_chip_enable();
	}

	/*reset SCB 0/1/2.*/
	*(unsigned long *)(0xB8000080) = (0x00040000 | (1<<25) | (1<<26));
	osal_delay(100);

	/*clear reset.*/
	*(unsigned long *)(0xB8000080) = 0x00000000;

#ifndef NETWORK_SUPPORT
	*(unsigned long *)(0xB8000060) |= (1<<12);
#endif

#ifndef DUAL_VIDEO_OUTPUT_USE_VCAP
	*(unsigned long *)(0xB8000064) |= (1<<8);
	*(unsigned long *)(0xB8000064) |= (1<<9);
	*(unsigned long *)(0xB8000064) |= (1<<10);
#endif
}

static void m3711c_destory(chip* thiz)
{
	return FREE(thiz);
}

chip* chip_create(void)
{
	chip* thiz = NULL;
	//ChipPriv *priv = NULL;
	thiz = (chip*)MALLOC(sizeof(chip) + sizeof(ChipPriv));

	if(NULL != thiz)
	{
		MEMSET(thiz, 0, sizeof(chip) + sizeof(ChipPriv));
		thiz->init = m3711c_init;
		thiz->destory = m3711c_destory;
		thiz->gpio_dir_set = m3711c_gpio_dir_set;
		thiz->gpio_dir_set_ext = m3711c_gpio_dir_set_ext;
		thiz->gpio_pin_init = m3711c_gpio_pin_init;
		thiz->gpio_pin_init_ext = m3711c_gpio_pin_init_ext;
		thiz->gpio_set = m3711c_gpio_set;
		thiz->pin_mux_set = m3711c_pin_mux_set;
		thiz->pin_mux_set_ext = m3711c_pin_mux_set_ext;
		thiz->tsi_pin_cfg = m3711c_tsi_pin_cfg;
		thiz->ci_power = m3711c_ci_power;
		thiz->gpio_get = m3711c_gpio_get;
	}

	return thiz;
}

