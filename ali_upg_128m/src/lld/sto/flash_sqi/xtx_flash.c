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

extern int sflash_write_status_reg16(INT32 sr);
extern int sflash_read_status_reg();

#define SR_BP0 1<<2
#define SR_BP1 1<<3
#define SR_BP2 1<<4
#define SR_BP3 1<<5
#define SR_BP4 1<<6
#define CON_TB 1<<14

#define XTX_FIRST_BP_BIT 2
#define XTX_PROTECT_LEVEL 16

#define XTX_BLOCK_SIZE 0x10000 //64kB

#define XTX_ID_TABLE_NUM 1
#define XTX_SIZE_NUM 8
typedef struct 
{
	UINT32 size;
	UINT8 reg_value;
}LOCK_FP_MAP;
LOCK_FP_MAP xtx_size2value[XTX_SIZE_NUM] = { 
							0,0x0,
							0x020000,0x09,
							0x040000,0x0a,
							0x080000,0x0b,
							0x100000,0x0c,
							0x200000,0x0d,
							0x400000,0x0e,
							0x800000,0x0f,
							};
static UINT32 xtx_id_table[XTX_ID_TABLE_NUM] = {
    0x17400b
};

static void xtx_dead_lock()
{
    libc_printf("[%s] going to dead lock...\n", __func__);
    while(1) {}
}

extern void sflash_write_enable(int en);
extern void sflash_wait_free();

INT32 xtx_set_sr(INT32 sr)
{
    //add by yuj 20170823
   // return sflash_write_status_reg16(sr);
    return sflash_write_status_reg8(sr);
}

INT32 xtx_get_sr(void)
{
    return sflash_read_status_reg();
}

static INT32 xtx_size_2_sr(INT32 total_size, INT32 protected_size)
{
	int mask = SR_BP4 |SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
	INT32 val = 0;
	INT32 i;

	if(0 == total_size || protected_size < 0)
		return -1;
	if(0 == protected_size)
		return 0x00;
	for(i=0;i<9;i++)
	{
		if(xtx_size2value[i].size == protected_size)
		{
			val = xtx_size2value[i].reg_value;
			break;
		}
	}
	return ((val<<2)&mask);
}

/*
 *  According to the spec, Protected size could be count by function as following:
 *
 *  2^(n-1) x XTX_BLOCK_SIZE = protected_size
 *
 */
static INT32 xtx_sr_2_size(INT32 total_size, INT32 protected_type)
{
	int mask = SR_BP4 |SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
	INT32 size = -1;
	INT32 i;
	INT32 val = (protected_type&mask) >>2;

	for(i=0;i<9;i++)
	{
		if(xtx_size2value[i].reg_value == val)
		{
			size = xtx_size2value[i].size;
			break;
		}
	}
	return size;
}

int xtx_get_lock_range(struct SWP *swp, INT32 *addr, INT32 *len)
{
    INT32 tmp_sr;
    INT32 size;

    if(swp->flash_size <= 0)
        return -1;

    tmp_sr = xtx_get_sr();

    size = xtx_sr_2_size(swp->flash_size, tmp_sr);
	if(size < 0)
		return -1;

	*addr = 0;
	*len = size;
	
	return 0;
}

int xtx_is_lock(struct SWP *swp, INT32 addr, INT32 len, INT32 *lock)
{
	INT32 size = addr + len;
	INT32 tmp_sr;
	INT32 sr;

	if(size < 0 || swp->flash_size <= 0)
	    return -1;

	tmp_sr = xtx_get_sr();

	sr = xtx_sr_2_size(swp->flash_size, tmp_sr);

	if(sr >= size)
	    *lock = 1;
	else
	    *lock = 0;

	libc_printf("xtx_is_lock-- sr:[%8x][%8x][%8x][%d]\n",tmp_sr,addr,len,*lock);
	return 0;
}

int xtx_lock(struct SWP *swp, INT32 addr, INT32 len)
{
	INT32 size = addr + len;
	INT32 sr = 0, ret = 0;;
	INT32 total_size = swp->flash_size;

	libc_printf("xtx_lock-- addr:0x%x, len:0x%x\n", addr, len);
	libc_printf("xtx_lock-- size:0x%x, total_size:0x%x\n", size, total_size);
	if(size < 0 || total_size <= 0 || size > total_size) {
		libc_printf("locked size:0x%x, total_size:0x%x\n", size, total_size);
		xtx_dead_lock();
		return -1;
	}
	sr = xtx_size_2_sr(total_size, size);
	if(sr < 0) 
	{
		libc_printf("wb_get_protect_type_by_size failed\n");
		return -1;
	}

	/* We assume user should know the area actually.
	If area is wrong, stay dead-lock. */
	if(addr != 0 || size != xtx_sr_2_size(total_size, sr)) 
	{
		libc_printf("[%s] wrong locked size:0x%x, it should be 0x%x\n", __func__, size, xtx_sr_2_size(total_size, sr));
		xtx_dead_lock();
		return -1;
	}
	libc_printf("xtx_lock-- sr:0x%x\n",sr);
	ret = xtx_set_sr(sr);
	if(ret < 0) 
	{
		libc_printf("ret:%d\n", ret);
		return ret;
	}
	return size;
}

int xtx_unlock(struct SWP *swp, INT32 addr, INT32 len)
{
	INT32 size = addr + len;
	INT32 sr = 0;
	int ret = 0;
	int lock = 0;

	if(size < 0 || swp->flash_size <= 0)
		return -1;

	xtx_is_lock(swp, addr, len, &lock);
	if(!lock)
		return -1;

	sr = xtx_size_2_sr(swp->flash_size, addr);
	if(sr < 0)
		return -1;
	return xtx_set_sr(sr);
}


int xtx_swp_init(struct SWP *swp)
{
	int i = 0, ret = -1, sr = 0;

	if(NULL == swp)
	return ret;

	for(i = 0; i < XTX_ID_TABLE_NUM; i++) 
	{
		if((xtx_id_table[i] & 0xff) == (swp->flash_id & 0xff)) 
		{
			ret = 0;
			break;
		}
	}
	if(0 != ret)
	return ret;

	swp->swp_lock = xtx_lock;
	swp->swp_unlock = xtx_unlock;
	swp->swp_is_lock = xtx_is_lock;
	swp->swp_get_lock_range = xtx_get_lock_range;
	sr = xtx_get_sr();
	if(0 == sr)
	xtx_lock(swp, swp->default_lock_addr, swp->default_lock_len);

	return ret;
}

#endif
