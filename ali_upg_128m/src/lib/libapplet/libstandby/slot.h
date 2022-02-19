#ifndef _SLOT_H_
#define _SLOT_H_
//=====================================================================================//

#define GET_DWORD(i)                                           (*(volatile unsigned long *)(i))
#define SET_DWORD(i, d)                                       (*(volatile unsigned long *)(i)) = (d)
#define GET_WORD(i)                                              (*(volatile unsigned short *)(i))
#define SET_WORD(i, d)                                          (*(volatile unsigned short *)(i)) = (d)
#define GET_BYTE(i)                                               (*(volatile unsigned char *)(i))
#define SET_BYTE(i, d)                                            (*(volatile unsigned char *)(i)) = (d)
//=====================================================================================//

extern void stdby_init_ir(void);
extern void stdby_save_time(unsigned long time);
extern void Set_C0_COUNT_1S(void);
extern void stdby_uart_debug(unsigned long data);
extern void stby_pan_init(void);
extern unsigned char stby_scan_key(void);
//=====================================================================================//
#endif
