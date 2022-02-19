#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <dp8051xp.h>
#endif
#include <stdio.h>
#include "sys.h"
//======================================================================================================================//

#define OSC_FREQ                                                                                 (27000000L)
#define BAUD_RATE                                                                               (9600L)
#define BAUD_TIMER                                                                              (1)
#define T1_INIT                                                                                     (256 - (OSC_FREQ/(96L*BAUD_RATE)+1)/2)
#define RCAP2_INIT                                                                               (65536 - (OSC_FREQ/(16L*BAUD_RATE)+1)/2)
//======================================================================================================================//

#ifdef PMU_MCU_DEBUG
void uart_init(void)
{
    SCON = 0x52;/*SCON: mode 1, 8-bit UART, enable rcvr.*/

#if(BAUD_TIMER == 1)
    PCON = 0x80;/*1 <=> fclk/16 0 <=> fclk/32.*/
    TMOD |= 0x20;/*TMOD: t1 mode 2, 8-bit reload.*/
    TH1 = TL1 = T1_INIT;
    TR1 = 1;/*TR1:timer 1 run.*/
#else
    PCON = 0x00;/*1 <=> fclk/16 0 <=> fclk/32.*/
    TH2 = RCAP2H = RCAP2_INIT/256;
    TL2 = RCAP2L = RCAP2_INIT%256;
    T2CON = 0x34;
#endif
}

#ifdef __SDCC__
void uart_interrupt(void) __interrupt (4)
{
    EA = 0;
    ES = 0;
    if(RI != 0)
    {
        RI = 0;
    }

    if(TI != 0)
    {
        TI = 0;
    }
    ES = 1;
    EA = 1;
    return;
}

void mdelay(void)
{
    UINT32 ms = 10000;
    while (ms--)
        NOP;
    return;
}

void putchar(char c)
{
    if (c != '\n')
    {
        SBUF = c;
        mdelay();
    }
    else
    {
        SBUF = 0x0D;
        mdelay();
        SBUF = 0x0A;
        mdelay();
    }
    return;
}
#endif
#endif

