#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <dp8051xp.h>
#endif
#include <stdio.h>
#include "sys.h"
#include "rtc.h"
#include "panel.h"
//======================================================================================================================//

RTC_TIMER g_rtc;
RTC_TIMER g_wake_rtc;
unsigned char g_ci_rtc_count = 0;
//======================================================================================================================//
static void pmu_timer1_int_en(UINT8 en)
{
    if (en)
    {
        PMU_WRITE_BYTE(PMU_RTC1_CTRL,
                       PMU_READ_BYTE(PMU_RTC1_CTRL) | TIMER1_INT_ENABLE);
    }
    else
    {
        PMU_WRITE_BYTE(PMU_RTC1_CTRL,
                       PMU_READ_BYTE(PMU_RTC1_CTRL) & TIMER1_INT_DISABLE);
    }
}

static void timer1_set_time(unsigned char autoload)
{
    UINT8 div = 0x1;
    UINT8 reg = 0;
    UINT8 reg1 = 0;

    if (autoload)
    {
        PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW0, 0x0C);/*init value=0xffffffff-(x sec*1000*1000*27/64).*/
        PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW1, 0x90);/*1 second.*/
        PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW2, 0xF9);
        PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW3, 0xFF);

        PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD, 0x0C);/*only by byte operate for pmu.*/
        PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD + 1, 0x90);
        PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD + 2, 0xF9);
        PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD + 3, 0xFF);
        PMU_WRITE_BYTE(PMU_RTC1_CTRL,
                       PMU_READ_BYTE(PMU_RTC1_CTRL) | AUTO_LOAD_EN);/*AUTO LOAD EN.*/
    }

    EX1 = 1;
    EA = 1;
    reg = PMU_READ_BYTE(MCU_SYS_IPR1);
    reg |= (1 << 0);
    PMU_WRITE_BYTE(MCU_SYS_IPR1, reg);/*MCU porlarity.*/
    reg = PMU_READ_BYTE(MCU_SYS_IER1);/*MCU IE.*/
    reg |= (1 << 0);
    PMU_WRITE_BYTE(MCU_SYS_IER1, reg);

    if (autoload)
    {
        PMU_WRITE_BYTE(PMU_RTC1_CTRL, 0x14 | div | (1 << 7));/*AUTO LOAD EN.*/
    }
    else
    {
        PMU_WRITE_BYTE(PMU_RTC1_CTRL, 0x14 | div);
    }

    pmu_timer1_int_en(1);
}

static UINT8 is_leap_year(pRTC_TIMER rtc)
{
    if (((rtc->year % 4) == 0) && ((rtc->year % 100) != 0))
    {
        return 1;
    }
    else if ((rtc->year % 400) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void update_current_time()
{
    if (g_rtc.sec > 59)
    {
        g_rtc.sec = 0;
        g_rtc.min++;
        if (g_rtc.min > 59)
        {
            g_rtc.min = 0;
            g_rtc.hour++;
            if (g_rtc.hour > 23)
            {
                g_rtc.hour = 0;
                g_rtc.day++;

                if ((g_rtc.month == 1) || (g_rtc.month == 3)
                        || (g_rtc.month == 5) || (g_rtc.month == 7)
                        || (g_rtc.month == 8) || (g_rtc.month == 10)
                        || (g_rtc.month == 12))
                {
                    if (g_rtc.day > 31)
                    {
                        g_rtc.day = 1;
                        g_rtc.month++;
                        if (g_rtc.month > 12)
                        {
                            g_rtc.month = 1;
                            g_rtc.year++;
                        }
                    }
                }

                if (g_rtc.month == 2)
                {
                    if ((g_rtc.day > 29) && (is_leap_year(&g_rtc)))
                    {
                        g_rtc.day = 1;
                        g_rtc.month++;
                    }
                    else if ((g_rtc.day > 28) && (is_leap_year(&g_rtc) == 0))
                    {
                        g_rtc.day = 1;
                        g_rtc.month++;
                    }
                }

                if ((g_rtc.month == 4) || (g_rtc.month == 6)
                        || (g_rtc.month == 9) || (g_rtc.month == 11))
                {
                    if (g_rtc.day > 30)
                    {
                        g_rtc.day = 1;
                        g_rtc.month++;
                    }
                }
            }
        }
    }
}

void main_cpu_get_time()
{
    pRTC_TIMER rtc = &g_rtc;
    rtc->year_h = (rtc->year) / 100;
    rtc->year_l = (rtc->year) % 100;

    PMU_WRITE_BYTE(PMUSRAM_GET_YEAR_H, rtc->year_h);
    PMU_WRITE_BYTE(PMUSRAM_GET_YEAR_L, rtc->year_l);
    PMU_WRITE_BYTE(PMUSRAM_GET_MONTH, rtc->month);
    PMU_WRITE_BYTE(PMUSRAM_GET_DAY, rtc->day);
    PMU_WRITE_BYTE(PMUSRAM_GET_HOUR, rtc->hour);
    PMU_WRITE_BYTE(PMUSRAM_GET_MIN, rtc->min);
    PMU_WRITE_BYTE(PMUSRAM_GET_SEC, rtc->sec);
}

void rtc_init(void)
{
    timer1_set_time (AUTO_LOAD_ENABLE);
}

void rtc_process()
{
    pRTC_TIMER rtc = &g_rtc;
    pRTC_TIMER wakeup_rtc = &g_wake_rtc;
    if ((rtc->month == wakeup_rtc->month) && (rtc->day == wakeup_rtc->day)
            && (rtc->hour == wakeup_rtc->hour) && (rtc->min == wakeup_rtc->min)
            && (rtc->sec == wakeup_rtc->sec))
    {
#ifndef PMU_MCU_DEBUG
        show_panel (SHOW_BANK);
#else
        printf(" rtc exit standby\n");
#endif
        mcu_disable_int();/*Disable all mcu interrupts.*/
        PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, EXIT_STANDBY_TYPE_RTC);
        exit_standby_status();
        main_cpu_get_time();
        mcu_gpio_init();/*disable all gpio.*/
        stb_power_on();

        while (1)
            ;/*exit standby, then hold mcu.*/
    }
}

/*TIMER1 interrupt process.*/
#ifdef __SDCC__
void timer_interrupt_handler(void) __interrupt (2)

#else
void timer_interrupt_handler(void)
interrupt 2
#endif
{
    UINT8 reg = 0;
    UINT8 div = TIMER1_CLK_DIV;

    EA = 0;
    reg = PMU_READ_BYTE(MCU_SYS_INT_STS);

    if(PMU_READ_BYTE(MCU_SYS_INT_STS) & 0x1)
    {
        PMU_WRITE_BYTE(PMU_RTC1_CTRL, PMU_READ_BYTE(PMU_RTC1_CTRL) | 0x1 | TIMER1_OVERFLOW_CLEAR);
        g_rtc.sec++;
        g_ci_rtc_count++;
        update_current_time();
    }
    EA = 1;
}

