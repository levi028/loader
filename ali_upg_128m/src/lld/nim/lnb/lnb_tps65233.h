#ifndef __TPS65233_H__
#define __TPS65233_H__

#include <api/libc/printf.h>

#if 0
#define LNB_PRINTF   libc_printf
#else
#define LNB_PRINTF(...)
#endif

#define REG_CONTROL_1	0x00
#define REG_CONTROL_2	0x01
#define REG_STATUS		0x02

INT32 lnb_tps65233_init(UINT32 id);
INT32 lnb_tps65233_control(UINT32 id, UINT32 cmd, UINT32 param);



#endif
