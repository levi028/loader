#include "chip.h"

static chip *g_chip = NULL;

void chip_pin_mux_set(pin_mux_info *pin_mux_info)
{
    if((NULL != pin_mux_info) && (NULL != g_chip) && (NULL != g_chip->pin_mux_set))
    {
        return g_chip->pin_mux_set(g_chip, pin_mux_info);
    }
}

void chip_pin_mux_set_ext(pin_mux_info pin_mux_info[], UINT8 count)
{
    if((NULL != pin_mux_info) && (NULL != g_chip) && (NULL != g_chip->pin_mux_set_ext))
    {
        return g_chip->pin_mux_set_ext(g_chip, pin_mux_info, count);
    }
}

void chip_gpio_pin_init(gpio_info_t *gpio_info, BOOL enabled)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_pin_init))
    {
        return g_chip->gpio_pin_init(g_chip, gpio_info, enabled);
    }
}

void chip_gpio_pin_init_ext(gpio_info_t gpio_info[], UINT8 count)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_pin_init_ext))
    {
        return g_chip->gpio_pin_init_ext(g_chip, gpio_info, count);
    }
}

void chip_gpio_set(gpio_info_t *gpio_info, UINT8 value)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_set))
    {
        return g_chip->gpio_set(g_chip, gpio_info, value);
    }
}

UINT32 chip_gpio_get(gpio_info_t *gpio_info)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_get))
    {
        return g_chip->gpio_get(g_chip, gpio_info);
    }
    else
    {
        return 0;
    }
}

void chip_gpio_dir_set(gpio_info_t *gpio_info)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_dir_set))
    {
        return g_chip->gpio_dir_set(g_chip, gpio_info);
    }
}

void chip_gpio_dir_set_ext(gpio_info_t gpio_info[], UINT8 count)
{
    if((NULL != gpio_info) && (NULL != g_chip) && (NULL != g_chip->gpio_dir_set_ext))
    {
        return g_chip->gpio_dir_set_ext(g_chip, gpio_info, count);
    }
}

void chip_init(chip_init_param *param)
{
    if((NULL != g_chip) && (NULL != g_chip->init))
    {
        return g_chip->init(g_chip, param);
    }
}

void chip_tsi_pin_cfg(UINT8 tsi_id, BOOL clear)
{
    if((NULL != g_chip) && (NULL != g_chip->tsi_pin_cfg))
    {
        return g_chip->tsi_pin_cfg(g_chip, tsi_id, clear);
    }
}

void chip_ci_power(BOOL power_on)
{
    if((NULL != g_chip) && (NULL != g_chip->ci_power))
    {
        return g_chip->ci_power(g_chip, power_on);
    }
}

void  global_chip_set(chip *chip)
{
    g_chip = chip;
}

chip *global_chip(void)
{
    return g_chip;
}


