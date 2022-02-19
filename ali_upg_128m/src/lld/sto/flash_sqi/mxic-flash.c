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

/*
 * Sample protect area table for 128MB flash (MXIC mx25l128):
 * All the combination with TB=1, one blocks size is 64k Bytes
 *
 *   BP3 | BP2 | BP1 | BP0 | Prot Length
 *  -------------------------------------
 *    0  |  0  |  0  |  0  | NONE
 *    0  |  0  |  0  |  1  | 0th block
 *    0  |  0  |  1  |  0  | 0-1st block
 *    0  |  0  |  1  |  1  | 0-3rd block
 *    0  |  1  |  0  |  0  | 0-7th block
 *    0  |  1  |  0  |  1  | 0-15th block
 *    0  |  1  |  1  |  0  | 0-31st block
 *    0  |  1  |  1  |  1  | 0-63rd block
 *    1  |  0  |  0  |  0  | 0-127th block
 *    1  |  0  |  0  |  1  | 0-256th block (all)
 *    1  |  0  |  1  |  0  | 0-256th block (all)
 *    1  |  0  |  1  |  1  | 0-256th block (all)
 *    1  |  1  |  0  |  0  | 0-256th block (all)
 *    1  |  1  |  0  |  1  | 0-256th block (all)
 *    1  |  1  |  1  |  0  | 0-256th block (all)
 *    1  |  1  |  1  |  1  | 0-256th block (all)
 *
 * Status Register:
 * |  S7  |  S6  |  S5  |  S4  |  S3  |  S2  |  S1  |  S0  |
 * | SRWD |  QE  |  BP3 |  BP2 |  BP1 |  BP0 |  WEL |  WIP |
 *
 * Configuration Register:
 * |  C7  |  C6  |  C5  |  C4  |  C3  |  C2  |  C1  |  C0  |
 * |  DC1 |  DC0 |  R   |  R   |  TB  | ODS2 | ODS1 | ODS0 |
 */

#define SR_BP0 1<<2
#define SR_BP1 1<<3
#define SR_BP2 1<<4
#define SR_BP3 1<<5
#define CON_TB 1<<3

#define MX_FIRST_BP_BIT 2
#define MX_PROTECT_LEVEL 16

#define MX_BLOCK_SIZE 0x10000 //64kB

#define MX_ID_TABLE_NUM 3
static UINT32 mx_id_table[MX_ID_TABLE_NUM] = {
    0x1620c2,    //mx25q32
    0x1720c2,    //mx25q64
    0x1820c2     //mx25q128
};

static void mx_dead_lock()
{
    libc_printf("[%s] going to dead lock...\n", __func__);
    while(1) {}
}

extern void sflash_write_enable(int en);
extern void sflash_wait_free();

static unsigned int mlog2 (unsigned int val)
{
    if (val == 0) return UINT_MAX;
    if (val == 1) return 0;
    unsigned int ret = 0;
    unsigned int comp_val = 1;

    while(1) {
        ret++;
        if(comp_val >= val)
            break;
        comp_val <<= 1;
    }

    return ret;
}

static unsigned int mpow2 (unsigned int val)
{
    if (val == 0) return 1;
    unsigned int ret = 1;

    return ret << (val);
}

INT32 mx_set_sr(INT32 sr)
{
    return sflash_write_status_reg16(sr);
}

INT32 mx_get_sr(void)
{
    return sflash_read_status_reg();
}

/*
 *  According to the spec, Status Register could be count by function as following:
 *
 *  2^(n-1) x MX_BLOCK_SIZE = protected_size
 *
 *  n = log2(protected_size) - log2(MX_BLOCK_SIZE) + 1
 *
 *  SR = (n << 2)
 *
 */
static INT32 mx_size_2_sr(INT32 total_size, INT32 protected_size)
{
    int mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;
    INT32 val;
    int pow;

    if(0 == total_size || protected_size < 0)
        return -1;
    if(0 == protected_size)
        return 0x00;

    pow = mlog2(protected_size) - mlog2(MX_BLOCK_SIZE) + 1;

    if(pow > MX_PROTECT_LEVEL - 1)
        pow = MX_PROTECT_LEVEL - 1;

    val = pow << MX_FIRST_BP_BIT;

    if (val & ~mask)
        return -1;

    return val;
}

/*
 *  According to the spec, Protected size could be count by function as following:
 *
 *  2^(n-1) x MX_BLOCK_SIZE = protected_size
 *
 */
static INT32 mx_sr_2_size(INT32 total_size, INT32 protected_type)
{
    INT32 size = -1;
    INT32 val;
    int mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;

    if (protected_type & ~mask)
        return -1;

    if(0 == protected_type)
        return 0;

    val = (mask & protected_type) >> MX_FIRST_BP_BIT;

    size = mpow2(val-1)*MX_BLOCK_SIZE;

    return size;
}

static INT32 mx_sr_2_last_sr(INT32 protected_type)
{
    INT32 val;
    //int mask = SR_BP3 | SR_BP2 | SR_BP1 | SR_BP0;

    if(0 == protected_type)
        return 0;

    val = protected_type - (1 << MX_FIRST_BP_BIT);

    return val;
}

int mx_get_lock_range(struct SWP *swp, INT32 *addr, INT32 *len)
{
    INT32 tmp_sr;
    INT32 size;

    if(swp->flash_size <= 0)
        return -1;

    tmp_sr = mx_get_sr();

    size = mx_sr_2_size(swp->flash_size, tmp_sr);
	if(size < 0)
		return -1;

	*addr = 0;
	*len = size;
	
	return 0;
}
int mx_is_lock(struct SWP *swp, INT32 addr, INT32 len, INT32 *lock)
{
    INT32 size = addr + len;
    INT32 tmp_sr;
    INT32 sr;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    tmp_sr = mx_get_sr();

    sr = mx_sr_2_size(swp->flash_size, tmp_sr);

    if(sr >= size)
        *lock = 1;
    else
        *lock = 0;

	libc_printf("mx_is_lock-- sr:[%8x][%8x][%8x][%d]\n",tmp_sr,addr,len,*lock);
    return 0;
}

int mx_lock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0, ret = 0;;
    INT32 total_size = swp->flash_size;

	libc_printf("mx_lock-- addr:0x%x, len:0x%x\n", addr, len);
	libc_printf("mx_lock-- size:0x%x, total_size:0x%x\n", size, total_size);
    if(size < 0 || total_size <= 0 || size > total_size) {
        libc_printf("locked size:0x%x, total_size:0x%x\n", size, total_size);
        mx_dead_lock();
        return -1;
    }

    sr = mx_size_2_sr(total_size, size);
    if(sr < 0) {
        libc_printf("wb_get_protect_type_by_size failed\n");
        return -1;
    }

	libc_printf("mx_lock-- sr:0x%x\n",sr);
    /* We assume user should know the area actually.
       If area is wrong, stay dead-lock. */
    if(addr != 0 || size != mx_sr_2_size(total_size, sr)) {
        libc_printf("[%s] wrong locked size:0x%x, it should be 0x%x\n", __func__, size, mx_sr_2_size(total_size, sr));
        mx_dead_lock();
        return -1;
    }

    sr |= (CON_TB << 8);

    ret = mx_set_sr(sr);
    if(ret < 0) {
        libc_printf("ret:%d\n", ret);
        return ret;
    }

    return size;
}

int mx_unlock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0;
    //int ret = 0;
    INT32 lock = 0;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    mx_is_lock(swp, addr, len, &lock);
    if(!lock)
        return -1;

    sr = mx_size_2_sr(swp->flash_size, addr);
    if(sr < 0)
        return -1;

    sr = mx_sr_2_last_sr(sr);

    sr |= (CON_TB << 8);

    return mx_set_sr(sr);
}


int mx_swp_init(struct SWP *swp)
{
    int i = 0, ret = -1, sr = 0;

    if(NULL == swp)
        return ret;

    for(i = 0; i < MX_ID_TABLE_NUM; i++) {
        if((mx_id_table[i] & 0xff) == (UINT32)(swp->flash_id & 0xff)) {
            ret = 0;
            break;
        }
    }
    if(0 != ret)
        return ret;

    swp->swp_lock = mx_lock;
    swp->swp_unlock = mx_unlock;
    swp->swp_is_lock = mx_is_lock;
	swp->swp_get_lock_range = mx_get_lock_range;

    sr = mx_get_sr();
    if(0 == sr)
        mx_lock(swp, swp->default_lock_addr, swp->default_lock_len);

    return ret;
}

#endif
