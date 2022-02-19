#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include "sto_flash.h"

#ifdef FLASH_SOFTWARE_PROTECT

/*Gigadevice flash lock rule is as same as Winbond, please check winbond-flash.c*/
int gd_swp_init(struct SWP *swp)
{
    return wb_swp_init(swp);
}

#endif
