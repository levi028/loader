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
extern int sflash_write_status_reg8(INT32 sr);
extern int sflash_read_status_reg();

/*
 * Compatible with ST Micro and similar flash.
 * Supports only the block protection bits BP{0,1,2} in the status register
 * (SR). Does not support these features found in newer SR bitfields:
 *   - TB: top/bottom protect - only handle TB=0 (top protect)
 *   - SEC: sector/block protect - only handle SEC=0 (block protect)
 *   - CMP: complement protect - only support CMP=0 (range is not complemented)
 *
 * Sample table portion for 8MB flash (Winbond w25q64fw):
 *
 *   SEC |  TB | BP2 | BP1 | BP0 | Prot Length | Protected Portion 
 *  ---------------------------------------------------------------
 *    X  |  X  |  0  |  0  |  0  | NONE        | NONE              
 *    0  |  1  |  0  |  0  |  1  | 128 KB      | Lower 1/64        
 *    0  |  1  |  0  |  1  |  0  | 256 KB      | Lower 1/32        
 *    0  |  1  |  0  |  1  |  1  | 512 KB      | Lower 1/16        
 *    0  |  1  |  1  |  0  |  0  | 1 MB        | Lower 1/8         
 *    0  |  1  |  1  |  0  |  1  | 2 MB        | Lower 1/4         
 *    0  |  1  |  1  |  1  |  0  | 4 MB        | Lower 1/2         
 *    X  |  X  |  1  |  1  |  1  | 8 MB        | ALL               
 *
 * Status Register-1 for 8MB flash (Winbond w25q64fw):
 *
 * |  S7  |  S6  |  S5  |  S4  |  S3  |  S2  |  S1  |  S0  | 
 * | SRP0 |  SEC |  TB  |  BP2 |  BP1 |  BP0 |  WEL | BUSY |
 *
 */

#define SR_BP0 1<<2 
#define SR_BP1 1<<3 
#define SR_BP2 1<<4 
#define SR_TB  1<<5
#define MB_FIRST_BP_BIT 2

#define MB_PROTECT_LEVEL 7

#define WB_ID_TABLE_NUM 1
static UINT32 wb_support_id_table[WB_ID_TABLE_NUM] = {
    0x1740ef    //w25q64
};
#define GD_ID_TABLE_NUM 3
static UINT32 gd_support_id_table[GD_ID_TABLE_NUM] = {
    0x1640c8,    //gd25q32
    0x1740c8,    //gd25q64
    0x1840c8     //gd25q128
};

extern void sflash_write_enable(int en);
extern void sflash_wait_free();

static void wb_dead_lock(){
    libc_printf("[%s] going to dead lock...\n", __func__);
    while(1){}
}

static unsigned int slog2 (unsigned int val) {
    if (val == 0) return UINT_MAX;
    if (val == 1) return 0;
    unsigned int ret = 0;
    unsigned int comp_val = 1;

    while(1){
        ret++;
        if(comp_val >= val)
            break;
        comp_val <<= 1;
    }

    return ret;
}

static unsigned int spow2 (unsigned int val) {
    if (val == 0) return 1;
    unsigned int ret = 1;

    return ret << (val);
}

INT32 wb_set_sr(struct SWP *swp, INT32 sr)
{
    int gd = 0, i = 0;

    /* Gigadevice flash lock has to write 8bit sr */
    for(i = 0; i < GD_ID_TABLE_NUM; i++)
{
        if((gd_support_id_table[i] & 0xff) == (UINT32)(swp->flash_id & 0xff)){
            gd = 1;
            break;
        }
    }
	libc_printf("wb_set_sr---flash_id[%8x][%d]!\n",swp->flash_id,gd);
    if(gd)
        return sflash_write_status_reg8(sr);
    else
        return sflash_write_status_reg16(sr);
}

INT32 wb_get_sr(void)
{
    return sflash_read_status_reg();
}

/*
 *  According to the spec, Status Register-1 could be count by function as following:
 *
 *
 *  (protected_size/total_size) <= 1/(2^n)
 *
 *  n = log2(total_size) - log2(protected_size)
 *
 *  SR = 0b00111100 - (n << 2)
 *
 */
static INT32 wb_size_2_sr(INT32 total_size, INT32 protected_size)
{
    int mask = SR_TB | SR_BP2 | SR_BP1 | SR_BP0;
    INT32 val;
    int pow;

    if(0 == total_size || protected_size < 0)
        return -1;
    if(0 == protected_size)
        return 0x00;

    pow = slog2(total_size) - slog2(protected_size);
    
    if(pow > MB_PROTECT_LEVEL - 1)
        pow = MB_PROTECT_LEVEL - 1;
    
    val = mask - (pow << MB_FIRST_BP_BIT);
    
	if (val & ~mask)
		return -1;
    
    if (!(val & mask))
		return -1;

    return val;
}

/*
 *  According to the spec, Protected size could be count by function as following:
 *
 *
 *  SR = 0b00111100 - (n << 2)
 *
 *  (protected_size/total_size) <= 1/(2^n)
 *
 *  protected_size = total_size/(2^n)
 *
 */
static INT32 wb_sr_2_size(INT32 total_size, INT32 protected_type)
{
    INT32 size = -1;
    INT32 val;
    int mask = SR_TB | SR_BP2 | SR_BP1 | SR_BP0;

    if(0 == protected_type)
        return 0;

    protected_type &= mask;

    val = ((mask - protected_type) >> MB_FIRST_BP_BIT) & 0x07;
    size = total_size / spow2(val);

    return size;
}

static INT32 wb_sr_2_last_sr(INT32 protected_type)
{
    INT32 val;
    int mask = SR_TB | SR_BP2 | SR_BP1 | SR_BP0;

    if(0 == protected_type)
        return 0;

    val = ((mask - protected_type) >> MB_FIRST_BP_BIT) + 1;
    if(val >= MB_PROTECT_LEVEL){
        val = 0;
    }else{
        val = mask - (val << MB_FIRST_BP_BIT);
        if (val & ~mask)
    		return -1;
        if (!(val & mask))
	    	return -1;
    }

    return val;
}

int wb_get_lock_range(struct SWP *swp, INT32 *addr, INT32 *len)
{
    INT32 tmp_sr;
    INT32 size;

    if(swp->flash_size <= 0)
        return -1;

    tmp_sr = wb_get_sr();

    size = wb_sr_2_size(swp->flash_size, tmp_sr);
	if(size < 0)
		return -1;

	*addr = 0;
	*len = size;
	
	return 0;
}
int wb_is_lock(struct SWP *swp, INT32 addr, INT32 len, INT32 *lock)
{
    INT32 size = addr + len;
    INT32 tmp_sr;
    INT32 sr;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    tmp_sr = wb_get_sr();

    sr = wb_sr_2_size(swp->flash_size, tmp_sr);

    if(sr >= size)
        *lock = 1;
    else
        *lock = 0;

    return 0;
}

int wb_lock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0, ret = 0;;
    INT32 total_size = swp->flash_size;

    if(size < 0 || total_size <= 0)
        return -1;

    if(size > total_size) 
        size = total_size;

    sr = wb_size_2_sr(total_size, size);
    if(sr < 0){
        libc_printf("wb_get_protect_type_by_size failed\n");
        return -1;
    }

    /* We assume user should know the area actually.
       If area is wrong, stay dead-lock. */
    if(addr != 0 || size != wb_sr_2_size(total_size, sr))
        wb_dead_lock();

    ret = wb_set_sr(swp, sr);
    if(ret < 0)
        return ret;

    return size;
}

int wb_unlock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0;
    INT32 lock = 0;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    wb_is_lock(swp, addr, len, &lock);
    if(!lock)
        return -1;

    sr = wb_size_2_sr(swp->flash_size, addr);
    if(sr < 0)
        return -1;

    sr = wb_sr_2_last_sr(sr);
    
    return wb_set_sr(swp, sr);
}


int wb_swp_init(struct SWP *swp)
{
    int i = 0, ret = -1, sr = 0;

    if(NULL == swp)
        return ret;

    for(i = 0; i < WB_ID_TABLE_NUM; i++){
        if((wb_support_id_table[i] & 0xff) == (UINT32)(swp->flash_id & 0xff)){
            ret = 0;
            break;
        }
    }
    
    /* Gigadevice flash lock rule is as same as Winbond */
    for(i = 0; i < GD_ID_TABLE_NUM; i++){
        if((gd_support_id_table[i] & 0xff) == (UINT32)(swp->flash_id & 0xff)){
            ret = 0;
            break;
        }
    }
    if(0 != ret)
        return ret;

    swp->swp_lock = wb_lock;
    swp->swp_unlock = wb_unlock;
    swp->swp_is_lock = wb_is_lock;
	swp->swp_get_lock_range = wb_get_lock_range;

    sr = wb_get_sr();
    if(0 == sr)
        wb_lock(swp, swp->default_lock_addr, swp->default_lock_len);

    return ret;
}

#endif
