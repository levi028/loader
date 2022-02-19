/*--------------------------------------------------------------------------
 DP8051XP.H
 Registers definition for DCD DP8051XP

 Copyright (c) 2003 Keil Elektronik GmbH and Keil Software, Inc.
 All rights reserved.
 -------------------------------------------------------------------------- */
#ifndef _DP8051XP_H_
#define _DP8051XP_H_

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* Defined configuration peripherals */

#define PMU_ENABLED      /* Power Management Unit enabled */
#define DPTR1_ENABLED    /* Enable second DPTR1           */

#define PORT0_ENABLED    /* Port 0 enabled                */
#define PORT1_ENABLED    /* Port 1 enabled                */
#define PORT2_ENABLED    /* Port 2 enabled                */
#define PORT3_ENABLED    /* Port 3 enabled                */

#define TIMERS01_ENABLED /* Timers 0&1 enabled            */
#define TIMER2_ENABLED   /* Timer 2 enabled               */
#define CC_ENABLED       /* Compare/Capture enabled       */

#define UART0_ENABLED    /* UART 0 enabled                */
#define UART1_ENABLED    /* UART 1 enabled                */

#define MDU_ENABLED      /* 16/32-bit MUL/DIV enabled     */

#define DI2CM_ENABLED    /* Master I2C enabled            */
#define DI2CS_ENABLED    /* Slave I2C enabled             */
#define WATCHDOG_ENABLED /* Watchdog enabled              */
#define DSPI_ENABLED     /* Master/Slave SPI enabled      */

/* *********************************************************************** */
/* *********************************************************************** */

/*---------------------------------------------------------------------------
 Defined registers  
 ---------------------------------------------------------------------------*/
/*  BYTE Register  */
#ifdef PORT0_ENABLED
__sfr __at 0x80 P0; /* Port 0                    */
#endif

__sfr __at 0x81 SP;
__sfr __at 0x82 DPL; /* Data Pointer 0 Low byte   */
__sfr __at 0x82 DPL0;
__sfr __at 0x83 DPH; /* Data Pointer 0 Hgih byte  */
__sfr __at 0x83 DPH0;

#ifdef DPTR1_ENABLED
__sfr __at 0x84 DPL1; /* Data Pointer 1 Low byte   */
__sfr __at 0x85 DPH1; /* Data Pointer 1 High byte  */
__sfr __at 0x86 DPS; /* Data pointers select      */
#endif

__sfr __at 0x87 PCON; /* Power Configuration       */
__sfr __at 0x88 TCON; /* Timer 0,1 Configuration   */

#ifdef TIMERS01_ENABLED
__sfr __at 0x89 TMOD; /* Timer 0,1 Mode            */
__sfr __at 0x8A TL0; /* Timer 0 Low byte counter  */
__sfr __at 0x8B TL1; /* Timer 1 Low byte counter  */
__sfr __at 0x8C TH0; /* Timer 0 High byte counter */
__sfr __at 0x8D TH1; /* Timer 1 High byte counter */
#endif

__sfr __at 0x8E CKCON; /* XDATA Wait States         */
/* 0x8F */
#ifdef PORT1_ENABLED
__sfr __at 0x90 P1; /* Port 1                    */
#endif

__sfr __at 0x91 EIF; /* External Interrupt Flag   */

__sfr __at 0x92 WTST; /* Program Wait States       */

__sfr __at 0x93 DPP; /* Data Page Pointer 0       */
__sfr __at 0x93 DPX0;
/* 0x94 */
#ifdef DPTR1_ENABLED
__sfr __at 0x95 DPX1; /* Data Page Pointer 1       */
#endif
/* 0x96
 0x97*/
#ifdef UART0_ENABLED
__sfr __at 0x98 SCON; /* Serial 0 Configuration    */
__sfr __at 0x98 SCON0;
__sfr __at 0x99 SBUF; /* Serial 0 I/O Buffer       */
__sfr __at 0x99 SBUF0;
#endif

#ifdef PORT2_ENABLED
__sfr __at 0xA0 P2; /* Port 2                    */
#endif
/* 0xA1
 0xA2
 0xA3
 0xA4
 0xA5
 0xA6
 0xA7*/
__sfr __at 0xA8 IE; /* Interrupt Enable          */
/* 0xA9
 0xAA
 0xAB
 0xAC
 0xAD
 0xAE
 0xAF*/
#ifdef PORT3_ENABLED
__sfr __at 0xB0 P3; /* Port 3                    */
#endif
/* 0xB1
 0xB2
 0xB3
 0xB4
 0xB5
 0xB6
 0xB7*/
__sfr __at 0xB8 IP;
/* 0xB9
 0xBA
 0xBB
 0xBC
 0xBD
 0xBE
 0xBF*/

#ifdef UART1_ENABLED
__sfr __at 0xC0 SCON1; /* Serial 1 Configuration    */
__sfr __at 0xC1 SBUF1; /* Serial 1 I/O Buffer       */
#endif

#ifdef CC_ENABLED
__sfr __at 0xC2 CCL1; /* Comare/Capture/Reload 1 Low */
__sfr __at 0xC3 CCH1; /* Comare/Capture/Reload 1 High*/
__sfr __at 0xC4 CCL2; /* Comare/Capture/Reload 2 Low */
__sfr __at 0xC5 CCH2; /* Comare/Capture/Reload 2 High*/
__sfr __at 0xC6 CCL3; /* Comare/Capture/Reload 3 Low */
__sfr __at 0xC7 CCH3; /* Comare/Capture/Reload 3 High*/
#endif

#if defined(TIMER2_ENABLED)  || defined(CC_ENABLED)
__sfr __at 0xC8 T2CON; /* Timer 2 Configuration     */
#endif

#ifdef CC_ENABLED
__sfr __at 0xC9 T2IF; /* Timer 2 Interrupt Flags   */
__sfr __at 0xCA RCRL; /* Timer 2 Reload Low byte   */
__sfr __at 0xCB RCRH; /* Timer 2 Reload High byte  */
__sfr __at 0xCC TL2; /* Timer 2 Low byte counter  */
__sfr __at 0xCD TH2; /* Timer 2 High byte counter */
#else
#ifdef TIMER2_ENABLED
__sfr __at 0xCA RLDL; /* Timer 2 Reload Low byte   */
__sfr __at 0xCB RLDH; /* Timer 2 Reload High byte  */
__sfr __at 0xCC TL2; /* Timer 2 Low byte counter  */
__sfr __at 0xCD TH2; /* Timer 2 High byte counter */
#endif
#endif

#ifdef CC_ENABLED
__sfr __at 0xCE CCEN; /* Comare/Capture/Reload Enable*/
#endif
/* 0xCF */
__sfr __at 0xD0 PSW; /* Program Status Word       */
/* 0xD1
 0xD2
 0xD3
 0xD4
 0xD5
 0xD6
 0xD7*/
#ifdef WATCHDOG_ENABLED
__sfr __at 0xD8 WDCON; /* Watchdog configuration    */
#endif
/* 0xD9
 0xDA
 0xDB
 0xDC
 0xDD
 0xDE
 0xDF*/
__sfr __at 0xE0 ACC; /* Accumulator               */
/* 0xE1
 0xE2
 0xE3
 0xE4
 0xE5
 0xE6
 0xE7*/
__sfr __at 0xE8 EIE; /* External Interrupt Enable */

#ifdef PMU_ENABLED
__sfr __at 0xE9 STATUS; /* Status register           */
#endif

__sfr __at 0xEA MXAX; /* MOVX @Ri High address     */

#ifdef WATCHDOG_ENABLED
__sfr __at 0xEB TA; /* Timed Access protection   */
#endif

#ifdef DSPI_ENABLED
__sfr __at 0xEC SPCR; /* SPI Control Register      */
__sfr __at 0xED SPSR; /* SPI Status Register       */
__sfr __at 0xEE SPDR; /* SPI I/O Register          */
__sfr __at 0xEF SSCR; /* SPI Sleve Select Register */
#endif

__sfr __at 0xF0 B; /* B Working register        */

#ifdef DI2CS_ENABLED
__sfr __at 0xF1 I2CSADR; /* I2CS Address              */
__sfr __at 0xF2 I2CSCR; /* I2CS Control Register     */
__sfr __at 0xF3 I2CSBUF; /* I2CS I/O Buffer           */
#endif

#ifdef DI2CM_ENABLED
__sfr __at 0xF4 I2CMSA; /* I2CM Slave Address        */
__sfr __at 0xF5 I2CMCR; /* I2CM Control Register     */
__sfr __at 0xF6 I2CMBUF; /* I2CM I/O Buffer           */
__sfr __at 0xF7 I2CMTP; /* I2CM Timer Period         */
#endif

__sfr __at 0xF8 EIP; /* External Interr. Priority */

#ifdef MDU_ENABLED
__sfr __at 0xF9 MD0; /* MDU Data 0                */
__sfr __at 0xFA MD1; /* MDU Data 1                */
__sfr __at 0xFB MD2; /* MDU Data 2                */
__sfr __at 0xFC MD3; /* MDU Data 3                */
__sfr __at 0xFD MD4; /* MDU Data 4                */
__sfr __at 0xFE MD5; /* MDU Data 5                */
__sfr __at 0xFF ARCON; /* MDU Control               */
#endif

/*-------------------------------------------------------------------------
 BIT Register  
 -------------------------------------------------------------------------*/
/*__sbit __at P0_7     = P0^7;  old*/
/*  P0  */
#ifdef PORT0_ENABLED
__sbit __at 0x80^7 P0_7;
__sbit __at 0x80^6 P0_6;
__sbit __at 0x80^5 P0_5;
__sbit __at 0x80^4 P0_4;
__sbit __at 0x80^3 P0_3;
__sbit __at 0x80^2 P0_2;
__sbit __at 0x80^1 P0_1;
__sbit __at 0x80^0 P0_0;
#endif

/*  P1  */
#ifdef PORT1_ENABLED
__sbit __at 0x90^7 P1_7;
__sbit __at 0x90^6 P1_6;
__sbit __at 0x90^5 P1_5;
__sbit __at 0x90^4 P1_4;
__sbit __at 0x90^3 P1_3;
__sbit __at 0x90^2 P1_2;
__sbit __at 0x90^1 P1_1;
__sbit __at 0x90^0 P1_0;
#endif
/*  P2  */
#ifdef PORT2_ENABLED
__sbit __at 0xA0^7 P2_7;
__sbit __at 0xA0^6 P2_6;
__sbit __at 0xA0^5 P2_5;
__sbit __at 0xA0^4 P2_4;
__sbit __at 0xA0^3 P2_3;
__sbit __at 0xA0^2 P2_2;
__sbit __at 0xA0^1 P2_1;
__sbit __at 0xA0^0 P2_0;
#endif
/*  P3  */
#ifdef PORT3_ENABLED
__sbit __at 0xB0^7 P3_7;
__sbit __at 0xB0^6 P3_6;
__sbit __at 0xB0^5 P3_5;
__sbit __at 0xB0^4 P3_4;
__sbit __at 0xB0^3 P3_3;
__sbit __at 0xB0^2 P3_2;
__sbit __at 0xB0^1 P3_1;
__sbit __at 0xB0^0 P3_0;
#endif
/*  TCON  */
__sbit __at 0x88^0 IT0;
__sbit __at 0x88^1 IE0;
__sbit __at 0x88^2 IT1;
__sbit __at 0x88^3 IE1;
#ifdef TIMERS01_ENABLED
__sbit __at 0x88^4 TR0;
__sbit __at 0x88^5 TF0;
__sbit __at 0x88^6 TR1;
__sbit __at 0x88^7 TF1;
#endif

/*  SCON0 */
#ifdef UART0_ENABLED
__sbit __at 0x98^0 RI0;
__sbit __at 0x98^1 TI0;
__sbit __at 0x98^2 RB08;
__sbit __at 0x98^3 TB08;
__sbit __at 0x98^4 REN0;
__sbit __at 0x98^5 SM02;
__sbit __at 0x98^6 SM01;
__sbit __at 0x98^7 SM00;

/* SCON */
__sbit __at 0x98^0 RI;
__sbit __at 0x98^1 TI;
__sbit __at 0x98^2 RB8;
__sbit __at 0x98^3 TB8;
__sbit __at 0x98^4 REN;
__sbit __at 0x98^5 SM2;
__sbit __at 0x98^6 SM1;
__sbit __at 0x98^7 SM0;
#endif

/*  IE   */
__sbit __at 0xA8^0 EX0;
#ifdef TIMERS01_ENABLED
__sbit __at 0xA8^1 ET0;
#endif
__sbit __at 0xA8^2 EX1;
#ifdef TIMERS01_ENABLED
__sbit __at 0xA8^3 ET1; /*E ??????????  */
#endif
#ifdef UART0_ENABLED
__sbit __at 0xA8^4 ES0;
__sbit __at 0xA8^4 ES;
#endif
#ifdef TIMER2_ENABLED
__sbit __at 0xA8^5 ET2;
#endif
#ifdef UART1_ENABLED
__sbit __at 0xA8^6 ES1;
#endif
__sbit __at 0xA8^7 EA;

/*  IP   */
__sbit __at 0xB8^0 PX0;
#ifdef TIMERS01_ENABLED
__sbit __at 0xB8^1 PT0;
#endif
__sbit __at 0xB8^2 PX1;
#ifdef TIMERS01_ENABLED
__sbit __at 0xB8^3 PT1;
#endif
#ifdef UART0_ENABLED
__sbit __at 0xB8^4 PS0;
__sbit __at 0xB8^4 PS;
#endif
#ifdef TIMER2_ENABLED
__sbit __at 0xB8^5 PT2;
#endif
#ifdef UART1_ENABLED
__sbit __at 0xB8^6 PS1;
#endif

/*  SCON1  */
#ifdef UART1_ENABLED
__sbit __at 0xC0^0 RI1;
__sbit __at 0xC0^1 TI1;
__sbit __at 0xC0^2 RB18;
__sbit __at 0xC0^3 TB18;
__sbit __at 0xC0^4 REN1;
__sbit __at 0xC0^5 SM12;
__sbit __at 0xC0^6 SM11;
__sbit __at 0xC0^7 SM10;
#endif

/*  T2CON  */
#ifdef CC_ENABLED
__sbit __at 0xC8^0 T2I0;
__sbit __at 0xC8^1 T2I1;
__sbit __at 0xC8^2 T2CM;
__sbit __at 0xC8^3 T2R0;
__sbit __at 0xC8^4 T2R1;
/* T2CON^5;*/
__sbit __at 0xC8^6 I3FR;
__sbit __at 0xC8^7 T2PS;
#else
#ifdef TIMER2_ENABLED
__sbit __at 0xC8^0 CPRL2;
__sbit __at 0xC8^1 CT2;
__sbit __at 0xC8^2 TR2;
__sbit __at 0xC8^3 EXEN2;
__sbit __at 0xC8^4 TCLK;
__sbit __at 0xC8^5 RCLK;
__sbit __at 0xC8^6 EXF2;
__sbit __at 0xC8^7 TF2;
#endif
#endif

/*  PSW   */
__sbit __at 0xD0^0 P;
__sbit __at 0xD0^1 F1;
__sbit __at 0xD0^2 OV;
__sbit __at 0xD0^3 RS0;
__sbit __at 0xD0^4 RS1;
__sbit __at 0xD0^5 F0;
__sbit __at 0xD0^6 AC;
__sbit __at 0xD0^7 CY;

/*  WDCON  */
#ifdef WATCHDOG_ENABLED
__sbit __at 0xD8^0 RWT;
__sbit __at 0xD8^1 EWT;
__sbit __at 0xD8^2 WTRF;
__sbit __at 0xD8^3 WDIF;
__sbit __at 0xD8^4 INT5F;
__sbit __at 0xD8^5 INT6F;
__sbit __at 0xD8^6 INT7F;
__sbit __at 0xD8^7 INT8F;
#endif

/*  EIE  */
__sbit __at 0xE8^0 EX2;
__sbit __at 0xE8^1 EX3;
__sbit __at 0xE8^2 EX4;
__sbit __at 0xE8^3 EX5;
__sbit __at 0xE8^4 EX6;
#ifdef WATCHDOG_ENABLED
__sbit __at 0xE8^5 EWDI;
#endif
#ifdef DI2CM_ENABLED
__sbit __at 0xE8^6 EI2CM;
#endif
#ifdef DI2CS_ENABLED
__sbit __at 0xE8^7 EI2CS;
#endif

/*  EIP  */
__sbit __at 0xF8^0 PX2;
__sbit __at 0xF8^1 PX3;
__sbit __at 0xF8^2 PX4;
__sbit __at 0xF8^3 PX5;
__sbit __at 0xF8^4 PX6;
#ifdef WATCHDOG_ENABLED
__sbit __at 0xF8^5 PWDI;
#endif
#ifdef DI2CM_ENABLED
__sbit __at 0xF8^6 PI2CM;
#endif
#ifdef DI2CS_ENABLED
__sbit __at 0xF8^7 PI2CS;
#endif

/*-------------------------------------------------------------------------
 BIT Values  
 -------------------------------------------------------------------------*/

/* TMOD Bit Values */
#define T0_M0_   0x01
#define T0_M1_   0x02
#define T0_CT_   0x04
#define T0_GATE_ 0x08
#define T1_M0_   0x10
#define T1_M1_   0x20
#define T1_CT_   0x40
#define T1_GATE_ 0x80

/* CKCON Bit Values  */
#define MD_    0x07
#define T0M_   0x08
#define T1M_   0x10
#define T2M_   0x20
#define WD_    0xC0

/* EIF Bit Values  */
#define  INT2F_   0x01
#define  INT3F_   0x02
#define  INT4F_   0x04
#define  INT5F_   0x08
#define  INT6F_   0x10

#ifdef DI2CM_ENABLED
#define  I2CMIF_  0x40
#endif
#ifdef DI2CS_ENABLED
#define  I2CSIF_  0x80
#endif

/* T2IF Bit Values */
#ifdef CC_ENABLED
#define T2_TF2_    0x01
#define T2_EXF2_   0x02
#define T2_EXEN2_  0x04
#endif

/* CCEN Bit Values */
#ifdef CC_ENABLED
#define T2_CM0_    0x03
#define T2_CM1_    0x0C
#define T2_CM2_    0x30
#define T2_CM3_    0xC0
#endif

#ifdef DSPI_ENABLED
/* DSPI Control bits                                             */
#define SPCR_SPIE_          0x80 /* Interrupt enable */
#define SPCR_SPE_           0x40 /* SPI Enable */
#define SPCR_MSTR_          0x10 /* Master(1)/Slave(0) */
#define SPCR_CPOL_          0x08 /* Clock polarity Low(1)/High(0) */
#define SPCR_CPHA_          0x04 /* Clock phase */

#define SPCR_SPR_           0x23 /* Clock rate mask */
#define SPR(x)              ((x&0x03)| ((x<<3)&0x20) )
#define DIV_BY_4            0x0
#define DIV_BY_8            0x1
#define DIV_BY_16           0x2
#define DIV_BY_32           0x3
#define DIV_BY_64           0x4
#define DIV_BY_128          0x5
#define DIV_BY_256          0x6
#define DIV_BY_512          0x7

/* DSPI Status bits                                            */
#define SPSR_SPIF_          0x80 /* Interrupt */
#define SPSR_WCOL_          0x40 /* Write colision */
#define SPSR_MODF_          0x10 /* Model fault */

/* DSPI Select Codes For SSCR register                         */
#define SEL_7               0x80
#define SEL_6               0x40
#define SEL_5               0x20
#define SEL_4               0x10
#define SEL_3               0x08
#define SEL_2               0x04
#define SEL_1               0x02
#define SEL_0               0x01
#endif

#ifdef DI2CS_ENABLED
/* DI2CS Status bits                                           */
#define SR_TREQ                        0x02
#define SR_RREQ                        0x01

/*
 DI2CS CODES written into control register I2CSCR to perfrom an action 
 Naming rules
 <actual state of DI2CS>_<next code>_<RS_value>
 */
/* DI2CS in IDLE                                               */
#define IDLE_START_SEND_0             0x03 /* RS=0 */
#define IDLE_START_SEND_STOP_0        0x07 /* RS=0 */

#define IDLE_START_RECEIVE_nACK_1     0x03 /* RS=1 */
#define IDLE_START_RECEIVE_STOP_1     0x07 /* RS=1 */

#define IDLE_START_RECEIVE_1          0x0B /* RS=1 */
#define IDLE_MASTERCODE_HS_0          0x11 /* RS=0 */

/* DI2CS in TRANSMIT                                           */
#define TRANSMIT_SEND_x               0x01 /* RS=x */
#define TRANSMIT_STOP_x               0x04 /* RS=x */
#define TRANSMIT_SEND_STOP_x          0x05 /* RS=x */

#define TRANSMIT_START_SEND_0         0x03 /* RS=0 */
#define TRANSMIT_START_SEND_STOP_0    0x07 /* RS=0 */

#define TRANSMIT_START_RECEIVE_nACK_1 0x03 /* RS=1 */
#define TRANSMIT_START_SEND_STOP_1    0x07 /* RS=1 */
#define TRANSMIT_START_RECEIVE_1      0x0B /* RS=1 */

/* DI2CS in RECEIVE                                            */
#define RECEIVE_RECEIVE_nACK_x        0x01 /* RS=x */
#define RECEIVE_STOP_x                0x04 /* RS=x */
#define RECEIVE_RECEIVE_STOP_x        0x05 /* RS=x */
#define RECEIVE_RECEIVE_x             0x09 /* RS=x */

#define RECEIVE_START_RECEIVE_nACK_1  0x03 /* RS=1 */
#define RECEIVE_START_RECEIVE_STOP_1  0x07 /* RS=1 */
#define RECEIVE_START_RECEIVE_1       0x0B /* RS=1 */

#define RECEIVE_START_SEND_0          0x03 /* RS=0 */
#define RECEIVE_START_SEND_STOP_0     0x07 /* RS=0 */

#endif

#ifdef DI2CM_ENABLED
#define RS_1      0x01
#define RS_0      0xFE

/* DI2CM Status bits                                           */
#define SR_BUS_BUSY                   0x40
#define SR_IDLE                       0x20
#define SR_ARB_LOST                   0x10
#define SR_DATA_ACK                   0x08
#define SR_ADDR_ACK                   0x04
#define SR_ERROR                      0x02
#define SR_BUSY                       0x01

/*
 DI2CM CODES written into control register I2CMCR to perfrom 
 an action. Naming rules
 <actual state of DI2CM>_<next code>_<RS_value>
 */

/* DI2CM in IDLE state                                        */
#define IDLE_START_SEND_0             0x03 /* RS=0 */
#define IDLE_START_SEND_STOP_0        0x07 /* RS=0 */

#define IDLE_START_RECEIVE_nACK_1     0x03 /* RS=1 */
#define IDLE_START_RECEIVE_STOP_1     0x07 /* RS=1 */

#define IDLE_START_RECEIVE_1          0x0B /* RS=1 */
#define IDLE_MASTERCODE_HS_x          0x11 /* RS=x */

/* DI2CM in TRANSMIT state                                    */
#define TRANSMIT_SEND_x               0x01 /* RS=x */
#define TRANSMIT_STOP_x               0x04 /* RS=x */
#define TRANSMIT_SEND_STOP_x          0x05 /* RS=x */

#define TRANSMIT_START_SEND_0         0x03 /* RS=0 */
#define TRANSMIT_START_SEND_STOP_0    0x07 /* RS=0 */

#define TRANSMIT_START_RECEIVE_nACK_1 0x03 /* RS=1 */
#define TRANSMIT_START_SEND_STOP_1    0x07 /* RS=1 */
#define TRANSMIT_START_RECEIVE_1      0x0B /* RS=1 */

/* DI2CM in RECEIVE state                                     */
#define RECEIVE_RECEIVE_nACK_x        0x01 /* RS=x */
#define RECEIVE_STOP_x                0x04 /* RS=x */
#define RECEIVE_RECEIVE_STOP_x        0x05 /* RS=x */
#define RECEIVE_RECEIVE_x             0x09 /* RS=x */

#define RECEIVE_START_RECEIVE_nACK_1  0x03 /* RS=1 */
#define RECEIVE_START_RECEIVE_STOP_1  0x07 /* RS=1 */
#define RECEIVE_START_RECEIVE_1       0x0B /* RS=1 */

#define RECEIVE_START_SEND_0          0x03 /* RS=0 */
#define RECEIVE_START_SEND_STOP_0     0x07 /* RS=0 */
#endif

#endif /*_DP8051XP_H_*/
