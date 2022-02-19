#include <stdio.h>
#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <dp8051xp.h>
#include <intrins.h>
#endif
#include "sys.h"
#include "ir.h"
#include "panel.h"
#include "rtc.h"
#include "uart.h"
//======================================================================================================================//

#ifdef __SDCC__
void ir_interrupt_handler(void) __interrupt (0);
void timer_interrupt_handler(void) __interrupt (2);
#ifdef PMU_MCU_DEBUG
void uart_interrupt(void) __interrupt (4);
#endif
void powerOff_interrupt_handler(void) __interrupt (7);
void extern_int3(void) __interrupt (8);
void extern_int4(void) __interrupt (9);
void extern_int5(void) __interrupt (10);
#endif

void main(void)
{
    /*init for normal mode.*/
    pmu_init();
    /*wait mailbox interrupt come.*/
    while (g_standby_flag != STANDBY_MODE)
        ;
    g_standby_flag = NORMAL_MODE;
    rtc_init();
    ir_init();
#ifdef PMU_MCU_DEBUG
    uart_init();
#else
    panel_init();
#endif
    while (1)
    {
        /*It will loop, until ir, panel_key or timer wakeup coming.*/
#ifndef PMU_MCU_DEBUG
        panel_process();
#endif
        rtc_process();
        ir_process();
#ifdef IR_SHARE_WITH_PANEL_ENABLE
        ali_power_key_process();/*Reserved for special case----ir share pin with panel power key.*/
#endif
    }
}
