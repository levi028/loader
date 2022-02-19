#include <stdio.h>
#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <intrins.h>
#include <dp8051xp.h>
#endif
#include "sys.h"
#include "ir.h"
#include "panel.h"
#include "rtc.h"
/*====================================================================*/

extern unsigned char g_ci_rtc_count;
extern RTC_TIMER g_rtc;
extern RTC_TIMER g_wake_rtc;
UINT8 g_standby_flag = NORMAL_MODE;
UINT8 g_power_key = 0;

/*====================================================================*/

static void hal_gpio_write(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO_DO_REG)) = val;
}

static UINT8 hal_gpio_read(void)
{
    UINT8 ret = 0;

    ret = *((volatile UINT8 xdata*)(HAL_GPIO_DI_REG));
    return ret;
}

static UINT8 hal_gpio_dir_get(void)
{
    UINT8 ret = 0;

    ret = *((volatile UINT8 xdata*)(HAL_GPIO_DIR_REG));
    return ret;
}

static void hal_gpio_dir_set(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO_DIR_REG)) = val;
}

static UINT8 hal_gpio1_dir_get(void)
{
    UINT8 ret = 0;

    ret = *((volatile UINT8 xdata*)(HAL_GPIO1_DIR_REG));
    return ret;
}

static void hal_gpio_en(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO_EN)) = val;
}

static void hal_gpio1_write(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO1_DO_REG)) = val;
}

static UINT8 hal_gpio1_read(void)
{
    UINT8 ret = 0;

    ret = *((volatile UINT8 xdata*)(HAL_GPIO1_DI_REG));
    return ret;
}

static void hal_gpio1_dir_set(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO1_DIR_REG)) = val;
}

static void hal_gpio1_en(UINT8 val)
{
    *((volatile UINT8 xdata*)(HAL_GPIO1_EN)) = val;
}

void hal_mcu_gpio_en(UINT8 pos)
{
    if (pos < 8)
    {
        hal_gpio_en((*((volatile UINT8 xdata*)(HAL_GPIO_EN))) | (1<<pos));
    }
    else
    {
        hal_gpio1_en((*((volatile UINT8 xdata*)(HAL_GPIO1_EN))) | (1<<(pos-8)));
    }
}

UINT8 hal_gpio_bit_get(UINT8 pos)
{
    UINT8 ret = 0;

    if (pos < 8)
    {
        ret = ((hal_gpio_read() >> pos) & 1);
    }
    else
    {
        ret = ((hal_gpio1_read() >> (pos - 8)) & 1);
    }

    return ret;
}

void hal_gpio_bit_set(UINT8 pos, UINT8 val)
{
    if (pos < 8)
    {
        hal_gpio_write((( (*(volatile UINT8 xdata*)(HAL_GPIO_DO_REG)) & (~(1 << pos))) | (val << pos)));
    }
    else
    {
        hal_gpio1_write( ( (*(volatile UINT8 xdata*)(HAL_GPIO1_DO_REG)) & (~(1 << (pos-8)))) | (val << (pos-8)));
    }
}

void hal_gpio_bit_dir_set(UINT8 pos, UINT8 val)
{
    if (pos < 8)
    {
        hal_gpio_dir_set((hal_gpio_dir_get() & (~(1 << pos))) | (val << pos));
    }
    else
    {
        hal_gpio1_dir_set(
            (hal_gpio1_dir_get() & (~(1 << (pos - 8)))) | (val << (pos - 8)));
    }
}

void mcu_disable_int(void)
{
    PMU_WRITE_BYTE(MCU_SYS_IER, 0);
}

void mcu_gpio_init(void)
{
#ifdef PMU_MCU_M3503
    hal_gpio_en(0);
#else
    hal_gpio_en(0);
    hal_gpio1_en(0);
#endif
}
static void init_externint(void)
{
    EX2 = 1;
    EX3 = 1;
    EX4 = 1;
    EX5 = 1;
    EA = 1;
}

static void power_key_cfg(void)
{
    UINT8 reg = 0;

    /*about 0.2s==>>>0x3e80 (UINT8)(4000*4/256).*/
    PMU_WRITE_BYTE(PRS_KEY_CFG, 0x80);/*about 0.1s==>>>0x1f40 (UINT8)(4000*2/256).*/
    PMU_WRITE_BYTE(PRS_KEY_CFG + 1, 0x3E);
    PMU_WRITE_BYTE(PRS_KEY_CFG + 2, 0x00);
    PMU_WRITE_BYTE(PRS_KEY_CFG + 3, PMU_READ_BYTE(PRS_KEY_CFG + 3) & 0xF0);

    /*KEY_DISABLE_VAL=n us*1.5/(4*(5+1)).*/
    PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW0, 0x6A);
    PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW1, 0x18);/*0.5 second.*/
    PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW2, 0x00);
    PMU_WRITE_BYTE(KEY_DISABLE_EN, 0x80);
    PMU_WRITE_BYTE(MCU_SYS_IPR,
                   PMU_READ_BYTE(MCU_SYS_IPR) | PRS_KEY_INT_IN_STANDBY_HIGH_POLARITY);
    PMU_WRITE_BYTE(MCU_SYS_IER,
                   PMU_READ_BYTE(MCU_SYS_IER) | PRS_KEY_INT_IN_STANDBY_ENABLE);
    IT0 = 1;
    EX0 = 1;

    PMU_WRITE_BYTE(PRS_KEY_CFG + 3,
                   PMU_READ_BYTE(PRS_KEY_CFG + 3) & PRS_KEY_WAKEUP_DISABLE);

    EA = 1;
}

void pmu_init(void)
{
    /*Close pmu gpio function.*/
    mcu_gpio_init();
    init_externint();
    /*PMU_WRITE_BYTE(PMU_TM0_CTRL, 0x0);*/
    PMU_WRITE_BYTE(PMU_RTC1_CTRL, 0x0);
    /*PMU_WRITE_BYTE(PMU_TM2_CTRL, 0x0);*/
    PMU_WRITE_BYTE(SYS_REG_IER, PMU_READ_BYTE(SYS_REG_IER) & IR_INTERRUPT_DISABLE);/*dis sys ir interrupt.*/
    PMU_WRITE_BYTE(IR_REG_IER, 0x0);/*disable ir ip interrupt.*/
    PMU_WRITE_BYTE(PMU_CFG, PMU_READ_BYTE(PMU_CFG) | MCU_WORK_IN_STANDBY);/*standby modde.*/
    power_key_cfg();
}

void stb_power_on(void)
{
#if ((defined PMU_MCU_M3702) || (defined PMU_MCU_M3922) || (defined PMU_MCU_M3711C))
    PMU_WRITE_BYTE(0xC100, 0x0);
    PMU_WRITE_BYTE(0xC101, 0x0);
    PMU_WRITE_BYTE(0xC102, 0x0);
    PMU_WRITE_BYTE(0xC103, 0x0);
#else
    PMU_WRITE_BYTE(0xC101, 0x0);
#endif
}

/*PMU->MAIN CPU, exit standby, get status.*/
void exit_standby_status(void)
{
    PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS0, ASCII_P);
    PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS1, ASCII_M);
    PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS2, ASCII_U);
}

static UINT32 stb_power_off_done(void)
{
    UINT32 ret = 0;
#if ((defined PMU_MCU_M3702) || (defined PMU_MCU_M3922) || (defined PMU_MCU_M3711C))
    if((0x54 == PMU_READ_BYTE(0xC100)) && (0x55 == PMU_READ_BYTE(0xC101))
            && (0x48 == PMU_READ_BYTE(0xC102)) && (0x53 == PMU_READ_BYTE(0xC103)))
    {
        ret = POWER_OFF_DONE;
    }
#else
    if (0x1 == PMU_READ_BYTE(0xC101))
    {
        ret = POWER_OFF_DONE;
    }
#endif
    return ret;
}

static void stb_power_off(void)
{
#if ((defined PMU_MCU_M3702) || (defined PMU_MCU_M3922) || (defined PMU_MCU_M3711C))
    PMU_WRITE_BYTE(0xC100, 0x54);
    PMU_WRITE_BYTE(0xC101, 0x55);
    PMU_WRITE_BYTE(0xC102, 0x48);
    PMU_WRITE_BYTE(0xC103, 0x53);
#else
    PMU_WRITE_BYTE(0xC101, 0x1);
#endif
}

extern UINT32 g_ir_power_key[8];
/*
 mailbox interrupt
 * MAIN CPU => PMU MCU
 *(1)	set ir wakeup power key
 *(2)	set timer wake up time
 *(3)	set current time
 */
#ifdef __SDCC__
void powerOff_interrupt_handler(void) __interrupt 7
#else
void powerOff_interrupt_handler(void)
interrupt 7
#endif
{
    UINT32 key1 = 0, key2 = 0, key3 = 0, key4 = 0;
    EA = 0;
    /*MCU to CPU Mailbox(204h~207h) interrupt 0.*/
    PMU_WRITE_BYTE(CPU_TO_MCU_INT_CLEAR_REG, CLEAR_MAILBOX0);

    /*get wakeup power key.*/
    key1 = PMU_READ_BYTE(MAILBOX_SET_POWERLOW0);
    key2 = PMU_READ_BYTE(MAILBOX_SET_POWERLOW1);
    key3 = PMU_READ_BYTE(MAILBOX_SET_POWERLOW2);
    key4 = PMU_READ_BYTE(MAILBOX_SET_POWERLOW3);
    g_ir_power_key[7] = ((key4<<24) | (key3<<16) | (key2<<8) | (key1));

    /*get wakeup time.*/
    g_wake_rtc.month = PMU_READ_BYTE(MAILBOX_WAKE_MONTH);
    g_wake_rtc.day = PMU_READ_BYTE(MAILBOX_WAKE_DAY);
    g_wake_rtc.hour = PMU_READ_BYTE(MAILBOX_WAKE_HOUR);
    g_wake_rtc.min = PMU_READ_BYTE(MAILBOX_WAKE_MIN);
    g_wake_rtc.sec = PMU_READ_BYTE(MAILBOX_WAKE_SECOND);

    /*get current time.*/
    g_rtc.year_h = PMU_READ_BYTE(MAILBOX_SET_YEAR_H);
    g_rtc.year_l = PMU_READ_BYTE(MAILBOX_SET_YEAR_L);
    g_rtc.month = PMU_READ_BYTE(MAILBOX_SET_MONTH);
    g_rtc.day = PMU_READ_BYTE(MAILBOX_SET_DAY);
    g_rtc.hour = PMU_READ_BYTE(MAILBOX_SET_HOUR);
    g_rtc.min = PMU_READ_BYTE(MAILBOX_SET_MIN);
    g_rtc.sec = PMU_READ_BYTE(MAILBOX_SET_SEC);
    if(g_rtc.sec == 0) g_rtc.sec = 1;
    g_rtc.year = (g_rtc.year_h*100) + g_rtc.year_l;

    while(POWER_OFF_DONE != stb_power_off_done())
    {
        stb_power_off();
    }
    PMU_WRITE_BYTE(SYS_REG_ISR, PMU_READ_BYTE(SYS_REG_ISR));/*clear interrupt.*/
    g_standby_flag = STANDBY_MODE;
    EA = 1;
}

#ifdef __SDCC__
void extern_int3(void) __interrupt (8)
#else
void extern_int3(void)
interrupt 8
#endif
{
    EA = 0;
    PMU_WRITE_BYTE(CPU_TO_MCU_INT_CLEAR_REG, CLEAR_MAILBOX1);/*MCU to CPU Mailbox(208h~20bh) interrupt 1.*/
    EA = 1;
}

#ifdef __SDCC__
void extern_int4(void) __interrupt (9)

#else
void extern_int4(void)
interrupt 9
#endif
{
    EA = 0;
    PMU_WRITE_BYTE(CPU_TO_MCU_INT_CLEAR_REG, CLEAR_MAILBOX2);/*MCU to CPU Mailbox(20ch~20fh) interrupt 2.*/
    EA = 1;
}

#ifdef __SDCC__
void extern_int5(void) __interrupt (10)
#else
void extern_int5(void)
interrupt 10
#endif
{
    EA = 0;
    EIF = 0;
    PMU_WRITE_BYTE(CPU_TO_MCU_INT_CLEAR_REG, CLEAR_MAILBOX3);/*MCU to CPU Mailbox(0x5000h~0x5fffh) interrupt.*/
    EA = 1;
}

