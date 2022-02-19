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
extern int sflash_write_status_reg8(INT32 sr);
extern int sflash_read_status_reg();
extern int sflash_write_esmt_tb_bit(BOOL value);		//Ben 171025#1

//Normal mode
#define SR_BP0 1<<2 
#define SR_BP1 1<<3 
#define SR_BP2 1<<4 
#define SR_BP3 1<<5 

//OTP mode
#define SR_TB  1<<3

/* Just define the area below
*	 TB | BP3 |BP2 | BP1 | BP0 | Prot Length | Protected Portion 
*  ---------------------------------------------------------------
*	 1	|   0   |  0	|  0    |  0	 | NONE	     | NONE			  
*	 1	|   0   |  0	|  0    |  1	 | 064 KB	     | Lower 1/128		  
*	 1	|   0   |  0	|  1    |  0	 | 128 KB	     | Lower 2/128		  
*	 1	|   0   |  0 	|  1    |  1	 | 256 KB	     | Lower 4/128		  
*	 1	|   0   |  1	|  0    |  0	 | 512 MB	     | Lower 8/128	  
*	 1	|   0   |  1	|  0    |  1	 | 1 MB		     | Lower 16/128		  
*	 1	|   0   |  1	|  1    |  0	 | 2 MB		     | Lower 32/128		  
*	 1	|   0   |  1	|  1    |  1	 | 4 MB		     | Lower 64/128	
*	 1	|   1   |  1	|  1    |  X	 | 8 MB		     | ALL	
*/

#define MB_FIRST_BP_BIT 2

#define MB_PROTECT_LEVEL 		9

typedef struct _ESMT_FP_MAP
{
	UINT32 size;
	UINT8 reg_value;
}ESMT_FP_MAP;

ESMT_FP_MAP size2value[MB_PROTECT_LEVEL] = { 
							0,0x0,
							0x10000,0x04,
							0x20000,0x08,
							0x40000,0x0C,
							0x80000,0x10,
							0x100000,0x14,
							0x200000,0x18,
							0x400000,0x1C,
							0x800000,0x3C,
							};
//

#define ESMT_ID_TABLE_NUM 		1
static UINT32 esmt_support_id_table[ESMT_ID_TABLE_NUM] = {
    0x17701c    //EN25QH64
};
extern void esmt_sflash_eon_enter_otp_mode(int en);
extern int esmt_sflash_write_esmt_tb_bit(BOOL value);
static void esmt_dead_lock(){
    libc_printf("[%s] going to dead lock...\n", __func__);
    while(1){}
}

INT32 esmt_get_sr(void)
{
    return sflash_read_status_reg();
}

INT32 esmt_set_sr(struct SWP *swp, INT32 sr)
{
	libc_printf("esmt_set_sr---flash_id[%8x][%x]!\n",swp->flash_id,sr);
	//Ben 190117#1
	INT32 tmp_val;
	tmp_val = esmt_get_sr();
	libc_printf("old sr reg[%x]\n",tmp_val);
	tmp_val = tmp_val&0xC3;
	tmp_val = sr|tmp_val;
	libc_printf("write sr reg[%x]\n",tmp_val);
	return sflash_write_status_reg8(tmp_val);
	//return sflash_write_status_reg8(sr);
	//
}

static INT32 esmt_size_2_sr(INT32 total_size, INT32 protected_size)
{
    INT32 val;
	UINT8 bI;
    int pow;

    if(0 == total_size || protected_size < 0)
        return -1;
    if(0 == protected_size)
        return 0x00;

	val = 0x00;
	for(bI=0; bI<MB_PROTECT_LEVEL; bI++)
	{
		if((protected_size >= size2value[bI].size)&&(protected_size < (size2value[bI].size*2)))
		{
			val = size2value[bI].reg_value;
			break;
		}
	}
	libc_printf("%s val[%x] \n",__FUNCTION__,val);	//Ben 171025#1

    return val;
}


static INT32 esmt_sr_2_size(INT32 total_size, INT32 protected_type)
{
    INT32 size = -1;
	UINT8 bI;
    INT32 val;

    if(0 == protected_type)
    {
		libc_printf("1.%s size[%x] \n",__FUNCTION__,size);
        return 0;
    }
	
	for(bI=0; bI<MB_PROTECT_LEVEL; bI++)
	{
		if(protected_type == size2value[bI].reg_value)
		{
			size = size2value[bI].size;
			break;
		}
	}
	libc_printf("2.%s size[%x] \n",__FUNCTION__,size);
	
    return size;
}

static INT32 esmt_sr_2_last_sr(INT32 protected_type)
{
    INT32 val = 0x0;
	UINT8 bI;

	for(bI=0; bI<MB_PROTECT_LEVEL; bI++)
	{
		if(protected_type == size2value[bI].reg_value)
		{
			break;
		}
	}

	if(0 == bI)
		val = 0x0;
	else
		val = size2value[bI-1].reg_value;

	libc_printf("%s val[%x] \n",__FUNCTION__,val);
	
    return val;
}

int esmt_get_lock_range(struct SWP *swp, INT32 *addr, INT32 *len)
{
    INT32 tmp_sr;
    INT32 size;

    if(swp->flash_size <= 0)
        return -1;

    tmp_sr = esmt_get_sr();

    size = esmt_sr_2_size(swp->flash_size, tmp_sr);
	if(size < 0)
		return -1;

	*addr = 0;
	*len = size;
	
	return 0;
}

int esmt_is_lock(struct SWP *swp, INT32 addr, INT32 len, INT32 *lock)
{
    INT32 size = addr + len;
    INT32 tmp_sr;
    INT32 sr;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    tmp_sr = esmt_get_sr();

	tmp_sr = tmp_sr&0x3C;	//Ben 190117#1
    sr = esmt_sr_2_size(swp->flash_size, tmp_sr);

    if(sr >= size)
        *lock = 1;
    else
        *lock = 0;

    return 0;
}

int esmt_lock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0, ret = 0;;
    INT32 total_size = swp->flash_size;

    if(size < 0 || total_size <= 0)
        return -1;

    if(size > total_size) 
        size = total_size;

    sr = esmt_size_2_sr(total_size, size);
    if(sr < 0){
        libc_printf("esmt_get_protect_type_by_size failed\n");
        return -1;
    }

    /* We assume user should know the area actually.
       If area is wrong, stay dead-lock. */
    if(addr != 0 || size != esmt_sr_2_size(total_size, sr))
        esmt_dead_lock();

    ret = esmt_set_sr(swp, sr);
    if(ret < 0)
        return ret;

    return size;
}

int esmt_unlock(struct SWP *swp, INT32 addr, INT32 len)
{
    INT32 size = addr + len;
    INT32 sr = 0;
    int lock = 0;

    if(size < 0 || swp->flash_size <= 0)
        return -1;

    esmt_is_lock(swp, addr, len, &lock);
    if(!lock)
        return -1;

    sr = esmt_size_2_sr(swp->flash_size, addr);
    if(sr < 0)
        return -1;

    sr = esmt_sr_2_last_sr(sr);
    
    return esmt_set_sr(swp, sr);
}


int esmt_swp_init(struct SWP *swp)
{
    int i = 0, ret = -1, sr = 0;

    if(NULL == swp)
        return ret;

    for(i = 0; i < ESMT_ID_TABLE_NUM; i++){
        if((esmt_support_id_table[i] & 0xff) == (swp->flash_id & 0xff)){
            ret = 0;
            break;
        }
    }
    
    if(0 != ret)
        return ret;
	
	esmt_sflash_write_esmt_tb_bit(TRUE);	//Ben 171025#1
	
    swp->swp_lock = esmt_lock;
    swp->swp_unlock = esmt_unlock;
    swp->swp_is_lock = esmt_is_lock;
    swp->swp_get_lock_range = esmt_get_lock_range;
    sr = esmt_get_sr();
	libc_printf("sr[%x] \n",sr);
    if(0 == sr)
        esmt_lock(swp, swp->default_lock_addr, swp->default_lock_len);

    return ret;
}

#endif
