/******************************************************************************
**
**    Copyright (c) Newport Media Inc.  All rights reserved.
**
**     Module Name:  nmiport.c
**
**        This module implements the necessary functions OS and bus function for
**        NM131 tuner.
**
**
*******************************************************************************/

void nmi_log(char *str);
void nmi_delay(uint32 msec);
uint32 nmi_get_tick(void);
int nmi_bus_read(unsigned char DeviceAddr, unsigned char* pArray, unsigned long count);
int nmi_bus_write(unsigned char DeviceAddr, unsigned char* pArray, unsigned long count);
unsigned long nmi_bus_get_read_blksz(void);
unsigned long nmi_bus_get_write_blksz(void);

