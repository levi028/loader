#ifndef _DRIVER_COMMON_H_
#define _DRIVER_COMMON_H_

#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define sys_write_byte(u_addr, b_val) do{*(volatile UINT8 *)(u_addr) = (b_val);}while(0)
#define sys_write_word(u_addr, w_val) do{*(volatile UINT16 *)(u_addr) = (w_val);}while(0)
#define sys_write_dword(u_addr, dw_val) do{*(volatile UINT32 *)(u_addr) = (dw_val);}while(0)
#define sys_read_byte(u_addr) \
                            ({ \
                                volatile UINT8 b_val; \
                                b_val = (*(volatile UINT8 *)(u_addr)); \
                                b_val; \
                            })
#define sys_read_word(u_addr) \
                            ({ \
                                volatile UINT16 w_val; \
                                w_val = (*(volatile UINT16 *)(u_addr)); \
                                w_val; \
                            })

#define sys_read_dword(u_addr) \
                            ({ \
                                volatile UINT32 dw_val; \
                                dw_val = (*(volatile UINT32 *)(u_addr)); \
                                dw_val; \
                            })

UINT16 sys_read_mem_w(UINT8 *p);
UINT32 sys_read_mem_d(UINT8 *p);
UINT16 sys_write_mem_w(UINT8 *ptr, UINT16 val);
UINT32 sys_write_mem_d(UINT8 *ptr, UINT32 val);

//VP SYS_MemMallocEx(INT blksz, INT bytealign, BOOL cacheable);
//ER SYS_MemFreeEx(VP blk);

#ifdef __cplusplus
}
#endif

#endif


