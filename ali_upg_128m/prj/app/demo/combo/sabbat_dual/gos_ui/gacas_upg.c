#include "gos_config.h"
#ifndef GOS_CONFIG_BOOT
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
//#include <hld/dmx/dmx.h>
#include <hld/ge/ge.h>
#include <hld/ge/ge_old.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common.h>
#include <hld/decv/vdec_driver.h>
#include <hld/deca/deca.h>
#include <hld/deca/deca_dev.h>
//#include <hld/dis/vpo.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
//#include "../copper_common/system_data.h"
//#include <api/libtsi/si_monitor.h>
#include <api/libtsi/sec_pmt.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#include <hld/trng/trng.h>
//#include <api/libga/lib_ga.h> 
#include <api/librsa/rsa_verify.h>
#include <bus/otp/otp.h>
#include "gacas_loader_db.h"
#include "gacas_upg.h"
#include "gacas_aes.h"
#include <api/librsa/flash_cipher.h>
#else
#include "bootloader.h"
#include "gacas_loader_db.h"
#include "gacas_upg.h"
#include "gacas_aes.h"
#include "gacas_otp.h"
#include <ali/otp.h>
#endif

//#define UPG_PRINTF(...) do{}while(0)
#define UPG_PRINTF libc_printf

gacas_upg_show_process g_progress_show = NULL;

extern UINT32 g_gacas_chip_id;
extern UINT32 g_gacas_mak_id;
UINT32 g_upg_sectors_total 	= 0;
UINT32 g_upg_sectors_index 	= 0;
LOADER_CHUNK s_chunk_info_upg_file[GACAS_MAX_CHUNK_NUM];
LOADER_CHUNK s_chunk_info_flash[GACAS_MAX_CHUNK_NUM];
UINT16 s_chunk_num_upg_file 	= 0;
UINT16 s_chunk_num_flash 	= 0;
UINT32 s_back_up_addr = 0;
UINT32 s_back_up_len = 0;
UINT32 s_loader_addr 	= 0;
UINT32 s_loader_len 	= 0;
UINT8 *s_back_up_data = NULL;
UINT32 g_gacas_tmp_buf_addr = 0;
UINT8 	is_need_save_loader = 0;
UINT8 	is_need_save_db_back = 0;

#ifndef GOS_CONFIG_BOOT
extern int heap_printf_free_size(UINT8 b_printf_now);
#endif
RET_CODE gacas_upg_convert_be_to_le(UINT8 *src, UINT8 *dest, UINT32 len)
{
    UINT32 i = 0;

    if ((src == NULL) || (dest == NULL) || (len == 0) || (len%4 != 0) )
    {
        return RET_FAILURE;
    }

    for (i = 0; i < len/4; i++)
    {
        *(dest + 4*i + 0) = *(src + 4*i + 3);
        *(dest + 4*i + 1) = *(src + 4*i + 2);
        *(dest + 4*i + 2) = *(src + 4*i + 1);
        *(dest + 4*i + 3) = *(src + 4*i + 0);
    }

    return RET_SUCCESS;
}

INT32 gacas_upg_flash_wirte_block(UINT32 flash_addr,UINT32 flash_len,UINT8 *data,UINT32 data_len)
{
	struct sto_device *flash_dev = NULL;
	UINT32 write_size = 0;
	UINT32 write_flash_addr = 0;
	UINT32 write_flash_len = 0;
	UINT32 write_data_len = 0;
	UINT8 *write_data = NULL;
	UINT8 *resume_data = NULL;
	UINT32 offset_addr = 0;
	UINT32 write_addr = 0;
//	UINT8 led_data[4];
	UINT32 param[2];
		

	UPG_PRINTF("gacas_upg_flash_wirte_block ---[%8x][%8x]\n",flash_addr,flash_len);
	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == flash_dev)
	{
		UPG_PRINTF("-FAILURE0\n");
		return RET_FAILURE;
	}
	if((flash_addr % GACAS_FLASH_BLOCK_SIZE) != 0)
	{
		write_addr 	= flash_addr/GACAS_FLASH_BLOCK_SIZE*GACAS_FLASH_BLOCK_SIZE;
		write_size 	= flash_addr - write_addr;
		resume_data 	= (UINT8 *)MALLOC(GACAS_FLASH_BLOCK_SIZE);
		UPG_PRINTF("***RESUME***[%8x][%8x]", write_addr,write_size);
		if(resume_data != NULL)
		{
			if ((INT32)write_addr != sto_lseek(flash_dev, (INT32)write_addr, STO_LSEEK_SET))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE1\n");
				return RET_FAILURE;
			}	
			if ((INT32)write_size != sto_read(flash_dev, resume_data, write_size))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE2\n");
				return RET_FAILURE;
			}	
			UPG_PRINTF("-OK\n");
			MEMCPY(&resume_data[write_size],data,write_addr+GACAS_FLASH_BLOCK_SIZE-flash_addr);
			param[0] = write_addr;
			param[1] = GACAS_FLASH_BLOCK_SIZE >> 10;
			UPG_PRINTF("***ERASE***[%8x]", write_addr);
			if (RET_SUCCESS == sto_io_control(flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param))
			{
				UPG_PRINTF("***WRITE***[%8x][%8x]", write_size,&resume_data[0]);
				if ((INT32)write_addr != sto_lseek(flash_dev, (INT32)write_addr, STO_LSEEK_SET))
				{
					FREE((void*)resume_data);
					UPG_PRINTF("-FAILURE3\n");
					return RET_FAILURE;
				}	
				if (GACAS_FLASH_BLOCK_SIZE!= sto_write(flash_dev, resume_data, GACAS_FLASH_BLOCK_SIZE))
				{
					FREE((void*)resume_data);
					UPG_PRINTF("-FAILURE4\n");
					return RET_FAILURE;
				}
				FREE((void*)resume_data);
				write_flash_addr 	= write_addr+GACAS_FLASH_BLOCK_SIZE;
				write_flash_len		= flash_len/GACAS_FLASH_BLOCK_SIZE*GACAS_FLASH_BLOCK_SIZE;
				write_data 		= data;
				write_data		+= (write_addr+GACAS_FLASH_BLOCK_SIZE-flash_addr);
				write_data_len		= data_len - (write_addr+GACAS_FLASH_BLOCK_SIZE-flash_addr);
				UPG_PRINTF("-OK\n");
				gacas_upg_set_burning_progress();
			}	
			else
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE5\n");
				return RET_FAILURE;
			}
		}
		else
		{
			UPG_PRINTF("-FAILURE6\n");
			return RET_FAILURE;		
		}
	}
	else
	{
		write_flash_addr = flash_addr;
		write_flash_len	= flash_len;
		write_data	= data;
		write_data_len	= data_len;
	}
	if((write_flash_len % GACAS_FLASH_BLOCK_SIZE) != 0)
	{
		resume_data 	= (UINT8 *)MALLOC(GACAS_FLASH_BLOCK_SIZE);
		write_addr 	= write_flash_addr+write_flash_len;
		write_size 	= GACAS_FLASH_BLOCK_SIZE - (write_flash_len%GACAS_FLASH_BLOCK_SIZE);
		if(resume_data != NULL)
		{
			if ((INT32)write_addr != sto_lseek(flash_dev, (INT32)write_addr, STO_LSEEK_SET))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE7\n");
				return RET_FAILURE;
			}	
			if ((INT32)write_size != sto_read(flash_dev, resume_data, write_size))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE8\n");
				return RET_FAILURE;
			}	
		}
		else
		{
			UPG_PRINTF("-FAILURE9\n");
			return RET_FAILURE;		
		}
	}
	for(offset_addr=0;offset_addr<write_flash_len;offset_addr+= GACAS_FLASH_BLOCK_SIZE)
	{
		param[0] = write_flash_addr+offset_addr;
		param[1] = GACAS_FLASH_BLOCK_SIZE >> 10;
		if (RET_SUCCESS == sto_io_control(flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param))
		{
			UPG_PRINTF("---ERASE---[%8x]", write_flash_addr+offset_addr);
			if(offset_addr < write_data_len)
			{
				write_addr = write_flash_addr+offset_addr;
				if((offset_addr+GACAS_FLASH_BLOCK_SIZE)<write_data_len)
					write_size  = GACAS_FLASH_BLOCK_SIZE;
				else
					write_size = write_data_len-offset_addr;
				UPG_PRINTF("---WRITE---[%2x][%2x][%2x][%2x]", write_data[0],write_data[1],write_data[2],write_data[3]);
				if ((INT32)write_addr != sto_lseek(flash_dev, (INT32)write_addr, STO_LSEEK_SET))
				{
					UPG_PRINTF("-FAILURE10\n");
					return RET_FAILURE;
				}	
				if ((INT32)write_size!= sto_write(flash_dev, write_data, write_size))
				{
					UPG_PRINTF("-FAILURE11\n");
					return RET_FAILURE;
				}	
				write_data += write_size;
				UPG_PRINTF("-OK\n");
			}
			else
			{
				UPG_PRINTF("-OK\n");
			}
			gacas_upg_set_burning_progress();
		}
		else
		{
			UPG_PRINTF("---ERASE---[%8x]FAILURE\n", write_flash_addr+offset_addr);
			return RET_FAILURE;
		}
	}
	if((write_flash_len % GACAS_FLASH_BLOCK_SIZE) != 0)
	{
		write_addr 	= write_flash_addr+write_flash_len;
		write_size 	= GACAS_FLASH_BLOCK_SIZE - (write_flash_len%GACAS_FLASH_BLOCK_SIZE);
		UPG_PRINTF("###RESUME###WRITE[%8x][%8x]", write_addr,write_size);
		if(resume_data != NULL)
		{
			if ((INT32)write_addr != sto_lseek(flash_dev, (INT32)write_addr, STO_LSEEK_SET))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE12\n");
				return RET_FAILURE;
			}	
			if ((INT32)write_size != sto_write(flash_dev, resume_data, write_size))
			{
				FREE((void*)resume_data);
				UPG_PRINTF("-FAILURE13\n");
				return RET_FAILURE;
			}	
			FREE((void*)resume_data);
			UPG_PRINTF("-OK\n");
		}	
		else
		{
			UPG_PRINTF("-FAILURE14\n");
			return RET_FAILURE;		
		}
	}
	return RET_SUCCESS;
}

INT32 gacas_upg_loader_back_up_open(UINT8 b_db_back)
{
	UINT8 i=0;
	UINT32 chunk_id;
	UINT32 tmp_data;
	
	UPG_PRINTF("gacas_upg_loader_back_up_open--[%d]\n",b_db_back);
	for(i = 0; i < s_chunk_num_flash; i++)
	{
		if(CHUNK_ID_DB == s_chunk_info_flash[i].id)
		{
			s_back_up_addr 	= s_chunk_info_flash[i].addr+CHUNK_HEADER_SIZE;
			s_back_up_len 	= s_chunk_info_flash[i].offset -CHUNK_HEADER_SIZE;			
		}
		
	}
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		if(CHUNK_ID_USER_DATA == s_chunk_info_upg_file[i].id)
		{
			s_loader_addr 	= s_chunk_info_upg_file[i].addr + CHUNK_HEADER_SIZE;
			s_loader_len 		= s_chunk_info_upg_file[i].offset -CHUNK_HEADER_SIZE;			
			if(s_loader_len > s_back_up_len)
				s_loader_len = s_back_up_len;
		}
	}
	if(s_loader_addr == 0 || s_loader_len == 0 || s_back_up_addr == 0 || s_back_up_len == 0 || s_loader_len > s_back_up_len)
	{
		UPG_PRINTF("gacas_upg_loader_back_up_open--FAIL[%08x][%08x][%08x][%08x]\n",s_loader_addr,s_loader_len,s_back_up_addr,s_back_up_len);
		return -1;
	}
	MEMCPY((UINT8*)&tmp_data,(UINT8*)(s_loader_addr),4);
	gacas_upg_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&chunk_id, 4);
	UPG_PRINTF("---Chunk---ID-- 	[%08x][%08x]\n", chunk_id,CHUNK_ID_LOADER);
	UPG_PRINTF("---flash---addr-	[%8x][%8x]\n", s_back_up_addr,s_back_up_len);
	UPG_PRINTF("---upg_file---addr-	[%8x][%8x]\n", s_loader_addr,s_loader_len);
	if(chunk_id == CHUNK_ID_LOADER)
	{
		struct sto_device *flash_dev = NULL;
		
		flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
		if (NULL != flash_dev)
		{
			s_back_up_data 	= (UINT8 *)MALLOC(s_back_up_len);
			if(s_back_up_data != NULL)
			{
				UPG_PRINTF("---READ---DB--BACK UP	[%8x][%8x]", s_back_up_addr,s_back_up_len);
				if ((INT32)s_back_up_addr == sto_lseek(flash_dev, (INT32)s_back_up_addr, STO_LSEEK_SET))
				{
					if ((INT32)s_back_up_len == sto_read(flash_dev,&s_back_up_data[0], s_back_up_len))
					{
						g_upg_sectors_total  += s_loader_len/GACAS_FLASH_BLOCK_SIZE;
						if(b_db_back == 1)
						{
							g_upg_sectors_total  += s_back_up_len/GACAS_FLASH_BLOCK_SIZE;
							is_need_save_db_back = 1;
						}
						is_need_save_loader = 1;
					}		
				}
				if(is_need_save_loader == 0)
				{
					FREE((void*)s_back_up_data);
					s_back_up_data 	= NULL;
					s_back_up_addr 	= 0;
					s_back_up_len 	= 0;
					is_need_save_db_back	= 0;
				}
			}					
		}	
	}	
	UPG_PRINTF("gacas_upg_loader_back_up_open--[%08x][%08x][%d[%d]]\n",s_back_up_addr,s_back_up_len,is_need_save_loader,is_need_save_db_back);
	return 0;
}
INT32 gacas_upg_loader_back_up_close(void)
{
	if(s_back_up_data != NULL)
		FREE((void*)s_back_up_data);
	s_back_up_data 	= NULL;
	s_back_up_addr 	= 0;
	s_back_up_len 	= 0;
	s_loader_addr 	= 0;
	s_loader_len 	= 0;
	is_need_save_db_back	= 0;
	is_need_save_loader = 0;
	return 0;
}
void gacas_upg_set_burning_progress(void)
{
    	g_upg_sectors_index++;
	if(g_progress_show != NULL)
    	g_progress_show((UINT32)(g_upg_sectors_index*100 /g_upg_sectors_total));
}
INT32 gacas_upg_burn_flash_by_addr(UINT32 upg_flash_addr,UINT32 upg_flash_size,UINT32 upg_file_addr,UINT32 upg_file_size,UINT8 b_save_loader)
{
	INT32 ret = SUCCESS ;
	UINT8 b_write_db_back = 1;
	struct sto_device *sto_dev = NULL;
	UINT32 flash_max_size = GACAS_FLASH_TOTAL_SIZE;//soft size
	
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	UPG_PRINTF("gacas_upg_burn_flash_by_addr---1---[%8x][%8x][%8x][%8x][%d]\n",upg_flash_addr,upg_flash_size,upg_file_addr,upg_file_size,b_save_loader);
	if(sto_dev != NULL)
	{
		flash_max_size = sto_dev->totol_size;
	}
	if((upg_flash_size + upg_flash_addr) > flash_max_size)
	{
		upg_flash_size = flash_max_size - upg_flash_addr;
	}
	UPG_PRINTF("gacas_upg_burn_flash_by_addr---2---[%8x][%8x][%8x][%8x]\n",upg_flash_addr,upg_flash_size,GACAS_FLASH_TOTAL_SIZE,flash_max_size);
	g_upg_sectors_total 	= 0;
	g_upg_sectors_index 	= 0;
	g_upg_sectors_total 	+= upg_flash_size/GACAS_FLASH_BLOCK_SIZE;
	if((upg_flash_addr+upg_flash_size) == GACAS_FLASH_TOTAL_SIZE)
		b_write_db_back = 0;
	if(b_save_loader == 1)
	{
		gacas_upg_loader_back_up_open(b_write_db_back);
	}
	if(upg_flash_size > 0)
	{
		//if(upg_flash_addr < GACAS_UPG_FLASH_LOCK_SIZE)
			gacas_loader_flash_unlock(0,GACAS_UPG_FLASH_LOCK_SIZE);
		if(is_need_save_loader == 1) 
		{
			if (SUCCESS != gacas_upg_flash_wirte_block(s_back_up_addr,s_back_up_len, (UINT8 *)s_loader_addr,s_loader_len))
			{
				UPG_PRINTF("gacas_upg_flash_wirte_block --1--ERROR\n");
				ret=  ERR_FAILUE;
			}
		}
		if (SUCCESS != gacas_upg_flash_wirte_block(upg_flash_addr, upg_flash_size, (UINT8 *)upg_file_addr,upg_flash_size))
		{
			UPG_PRINTF("gacas_upg_flash_wirte_block --2--ERROR\n");
			ret=  ERR_FAILUE;
		}
		if(is_need_save_db_back == 1)
		{
			if (SUCCESS != gacas_upg_flash_wirte_block(s_back_up_addr,s_back_up_len,(UINT8 *)s_back_up_data,s_back_up_len))
			{
				UPG_PRINTF("gacas_upg_flash_wirte_block --3--ERROR\n");
				ret=  ERR_FAILUE;
			}
		}
		if(upg_flash_addr < GACAS_UPG_FLASH_LOCK_SIZE)
			gacas_loader_flash_lock(0,GACAS_UPG_FLASH_LOCK_SIZE);
	}
	else
	{
		UPG_PRINTF("ERROR --upg_flash_size == 0\n");
		ret=  ERR_FAILUE;			
	}
	if(b_save_loader == 1)
	{
		gacas_upg_loader_back_up_close();
	}
    	return ret;
}

INT32 gacas_upg_burn_flash_by_chunk(UINT8 b_save_loader)
{
	UINT8 i=0;
	UINT8 k=0;
	UINT8 b_write_db_back = 1;

	INT32 ret = SUCCESS ;
	UINT8 loader_update = 0;
	
	g_upg_sectors_total 	= 0;
	g_upg_sectors_index 	= 0;
	UPG_PRINTF("gacas_upg_burn_flash_by_chunk------[%d][%d]\n",s_chunk_num_upg_file,s_chunk_num_flash);
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		if(TRUE == s_chunk_info_upg_file[i].b_update)
		{
			for(k = 0; k < s_chunk_num_flash; k++)
			{
				if(s_chunk_info_flash[k].id == s_chunk_info_upg_file[i].id)
				{		
					if(s_chunk_info_flash[k].offset >= s_chunk_info_upg_file[i].offset)
					{
						g_upg_sectors_total 	+= s_chunk_info_flash[k].offset/GACAS_FLASH_BLOCK_SIZE;
						if((s_chunk_info_flash[k].offset%C_SECTOR_SIZE) != 0) 
						{
							g_upg_sectors_total++;
						}
					}
					else
					{
						s_chunk_info_upg_file[i].b_update = FALSE;
					}
					break;
				}
			}
			if(TRUE == s_chunk_info_upg_file[i].b_update)
			{
				if(CHUNK_ID_DB == s_chunk_info_upg_file[i].id)
					b_write_db_back = 0;
				UPG_PRINTF("upg-file[%8x]----update[%d]\n",s_chunk_info_upg_file[i].id,g_upg_sectors_total);
			}
		}
	}
	gacas_loader_flash_unlock(0,GACAS_UPG_FLASH_LOCK_SIZE);
	if(b_save_loader == 1)
	{
		gacas_upg_loader_back_up_open(b_write_db_back);
	}
	UPG_PRINTF("gacas_upg_flash_wirte_block--total_sectors[%x]\n",g_upg_sectors_total);
	if(is_need_save_loader == 1) 
	{
		if (SUCCESS != gacas_upg_flash_wirte_block(s_back_up_addr,s_back_up_len, (UINT8 *)s_loader_addr,s_loader_len))
		{
			UPG_PRINTF("gacas_upg_flash_wirte_block --loader--ERROR\n");
			ret=  ERR_FAILUE;
		}
	}	
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		if(s_chunk_info_upg_file[i].b_update 	== TRUE)
		{
			for(k = 0; k < s_chunk_num_flash; k++)
			{
				if(s_chunk_info_flash[k].id == s_chunk_info_upg_file[i].id)
				{
					UPG_PRINTF("\n FLASH-WRITE----[%8x][%8x][%8x][%8x]\n", s_chunk_info_upg_file[i].id,s_chunk_info_flash[k].addr,s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].offset);
					if (SUCCESS != gacas_upg_flash_wirte_block(s_chunk_info_flash[k].addr, s_chunk_info_flash[k].offset, (UINT8 *)s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].offset))
					{
						UPG_PRINTF("gacas_upg_flash_wirte_block ---[%8x]ERROR\n",s_chunk_info_upg_file[i].id);
						ret= ERR_FAILED;
						break;
					}		
					if(s_chunk_info_flash[k].id == CHUNK_ID_LOADER)
						loader_update++;
				}
			}		
		}
		else 
		{
			if(s_chunk_info_flash[k].id == CHUNK_ID_LOADER)
				loader_update++;			
		}
		if(loader_update == 2)
		{
			gacas_loader_db_set_index(0);
		}
	}
	if(is_need_save_db_back == 1)
	{
		if (SUCCESS != gacas_upg_flash_wirte_block(s_back_up_addr,s_back_up_len,(UINT8 *)s_back_up_data,s_back_up_len))
		{
			UPG_PRINTF("gacas_upg_flash_wirte_block --db_back--ERROR\n");
			ret=  ERR_FAILUE;
		}
	}	
	if(b_save_loader == 1)
	{
		gacas_upg_loader_back_up_close();
	}
	gacas_loader_flash_lock(0,GACAS_UPG_FLASH_LOCK_SIZE);
    	return ret;
}
UINT8 gacas_upg_get_updata_chunk_num(void)
{
	UINT8 i=0;
	UINT8 chunk_num = 0;

	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		if(s_chunk_info_upg_file[i].b_update == TRUE)
			chunk_num++;
	}
	UPG_PRINTF("gacas_upg_get_updata_chunk_num-------[%d]\n",chunk_num);
    	return chunk_num;
}
INT32 gacas_upg_add_updata_chunk_id(UINT32 chunk_id)
{
	UINT8 i=0;
	UINT8 k=0;

	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		for(k = 0; k < s_chunk_num_flash; k++)
		{
			if(s_chunk_info_flash[k].id == s_chunk_info_upg_file[i].id && chunk_id == s_chunk_info_upg_file[i].id)
			{		
				UPG_PRINTF("add_updata_chunk-------[%8x][%8x][%8x]\n",s_chunk_info_upg_file[i].id,s_chunk_info_upg_file[i].offset,s_chunk_info_flash[k].offset);
				s_chunk_info_upg_file[i].b_update = TRUE;
				break;
			}
		}
	}
    	return 0;
}

INT32 gacas_upg_set_updata_chunk_id(UINT32 *p_chunk_id,UINT8 chunk_num)
{
	UINT8 i=0;
	UINT8 k=0;
	UINT8 m=0;


	if(p_chunk_id == NULL || chunk_num == 0)
		return -1;
	UPG_PRINTF("gacas_upg_set_updata_chunk_id-------[%d][%d][%d]num\n",chunk_num,s_chunk_num_upg_file,s_chunk_num_flash);
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		s_chunk_info_upg_file[i].b_update = FALSE;
		for(k = 0; k < s_chunk_num_flash; k++)
		{
			if(s_chunk_info_flash[k].id == s_chunk_info_upg_file[i].id)
			{		
				for(m=0;m<chunk_num;m++)
				{
					if(s_chunk_info_upg_file[i].id == p_chunk_id[m])
					{
						UPG_PRINTF("chunk-------[%8x][%8x][%8x]\n",s_chunk_info_upg_file[i].id,s_chunk_info_upg_file[i].offset,s_chunk_info_flash[k].offset);
						if(s_chunk_info_flash[k].offset >= s_chunk_info_upg_file[i].offset)
						{
							s_chunk_info_upg_file[i].b_update = TRUE;
						}
						break;
					}
				}
			}
		}
	}
    	return 0;
}

INT32 gacas_upg_encrypt_chunk(UINT32 chunk_id,UINT8 b_data_encypt)
{
	UINT8 i=0;
	INT32 ret = ERR_FAILURE ;
	
	UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x][%d]\n",chunk_id,b_data_encypt);
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		if(chunk_id == s_chunk_info_upg_file[i].id)
#if 1
		{
			UINT8 *encrypt_data_in		= NULL;
			UINT8 *encrypt_data_1		= NULL;
			UINT8 *encrypt_data_2		= NULL;
			UINT32 encrypt_data_data_len = 0;
			UINT32 encrypt_data_add_1 = 0;
			UINT32 encrypt_data_add_2 = 0;
			UINT8  b_free = 0;
			
			UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x]sign_len\n",s_chunk_info_upg_file[i].sign_len);
			UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x]len\n",s_chunk_info_upg_file[i].len);
			UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x]offset\n",s_chunk_info_upg_file[i].offset);
			UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x]crc32\n",s_chunk_info_upg_file[i].crc32);
			UPG_PRINTF("gacas_upg_encrypt_chunk----[%8x]b_update\n",s_chunk_info_upg_file[i].b_update);

			encrypt_data_data_len 		= s_chunk_info_upg_file[i].sign_len - CHUNK_HEADER_SIZE;
			encrypt_data_in 			= (UINT8 *)(s_chunk_info_upg_file[i].addr+CHUNK_HEADER_SIZE);
			if(g_gacas_tmp_buf_addr == 0 || encrypt_data_data_len > ((GACAS_UPG_BUFFER_MAX_LEN-0x20)/2))
			{
				encrypt_data_add_1 		= (UINT32)MALLOC(encrypt_data_data_len + 0xf);
				encrypt_data_add_2 		= (UINT32)MALLOC(encrypt_data_data_len + 0xf);	
				b_free = 1;
			}
			else
			{
				encrypt_data_add_1 		= (UINT32)g_gacas_tmp_buf_addr;
				encrypt_data_add_2 		= (UINT32)(g_gacas_tmp_buf_addr+(GACAS_UPG_BUFFER_MAX_LEN-0x20)/2);
			}
			if(encrypt_data_add_1 != 0)
				encrypt_data_1 		= (UINT8 *)(0xFFFFFFF8 & encrypt_data_add_1);
			if(encrypt_data_add_1 != 0)
				encrypt_data_2 		= (UINT8 *)(0xFFFFFFF8 & encrypt_data_add_2);
			//encrypt_data_1 	= (UINT8 *)(0xFFFFFFF8 &(__MM_PVR_VOB_BUFFER_ADDR | 0xa0000000));
			//encrypt_data_2	= encrypt_data_1 + encrypt_data_data_len;
			UPG_PRINTF("--encrypt_data--in[%8x]d[%8x]e[%8x]len[%8x]\n",encrypt_data_in,encrypt_data_1,encrypt_data_2,encrypt_data_data_len);
#ifndef GOS_CONFIG_BOOT
			heap_printf_free_size(1);
#endif
			if(encrypt_data_1 != NULL && encrypt_data_2 != NULL)
			{
				MEMCPY(encrypt_data_1,encrypt_data_in,encrypt_data_data_len);
				if(b_data_encypt == 1)
				{
					if(RET_SUCCESS == gacas_decrypt_chunk_data(encrypt_data_1,encrypt_data_2,encrypt_data_data_len))
					{
						MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
						UPG_PRINTF("--encrypt_data_in--:[%8x]\n",encrypt_data_data_len);
						gacas_aes_printf_bin_16(&encrypt_data_in[0]);
						gacas_aes_printf_bin_16(&encrypt_data_in[16]);
						gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
						gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);							
						if (RET_SUCCESS != test_rsa_ram((UINT32)s_chunk_info_upg_file[i].addr, s_chunk_info_upg_file[i].sign_len))
						{
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							UPG_PRINTF("--test_rsa_ram-0-[%8x][%8x]ERROR\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
						}					
						else if(RET_SUCCESS == gacas_encrypt_chunk_by_otp(encrypt_data_2,encrypt_data_in,encrypt_data_data_len))
						{
							UPG_PRINTF("--test_rsa_ram-0-[%8x][%8x]OK\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							UPG_PRINTF("--encrypt_data_d--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_2[0]);
							gacas_aes_printf_bin_16(&encrypt_data_2[16]);
							gacas_aes_printf_bin_16(&encrypt_data_2[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_2[encrypt_data_data_len-16]);
							UPG_PRINTF("--encrypt_data_e--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_in[0]);
							gacas_aes_printf_bin_16(&encrypt_data_in[16]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);
							#if 0
							//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							gacas_decrypt_chunk_by_otp(encrypt_data_1,encrypt_data_2,encrypt_data_data_len);
							if(MEMCMP(encrypt_data_in,encrypt_data_2,encrypt_data_data_len) != 0)
							{
								UPG_PRINTF("--encrypt_data_in--xxx--encrypt_data_2\n");
								MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							}				
							UPG_PRINTF("--encrypt_data_in--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_in[0]);
							gacas_aes_printf_bin_16(&encrypt_data_in[16]);
							gacas_aes_printf_bin_16(&encrypt_data_in[32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[48]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);
							ret = test_rsa_ram((UINT32)s_chunk_info_upg_file[i].addr, s_chunk_info_upg_file[i].sign_len);
							if(ret == 0)
								UPG_PRINTF("--test_rsa_ram-1-[%8x][%8x]OK\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							else
								UPG_PRINTF("--test_rsa_ram-1-[%8x][%8x]ERROR\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							gacas_encrypt_chunk_by_otp(encrypt_data_2,encrypt_data_1,encrypt_data_data_len);
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							#endif
							MEMCPY((UINT8 *)(s_chunk_info_upg_file[i].addr+s_chunk_info_upg_file[i].offset - 8),(UINT8*)&g_gacas_mak_id, 4);
							MEMCPY((UINT8 *)(s_chunk_info_upg_file[i].addr+s_chunk_info_upg_file[i].offset - 4),(UINT8*)&g_gacas_chip_id, 4);
							UPG_PRINTF("--encrypt--aes[%8x][%8x][%8x][OK]\n",encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							ret = SUCCESS ;
						}
						else
						{
							UPG_PRINTF("--gacas_encrypt_chunk_by_otp-[%8x][%8x]ERROR\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
						}
					}
				}
				else
				{
					if(RET_SUCCESS == gacas_encrypt_chunk_by_otp(encrypt_data_1,encrypt_data_in,encrypt_data_data_len))
					{
						//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
						MEMCPY((UINT8 *)(s_chunk_info_upg_file[i].addr+s_chunk_info_upg_file[i].offset - 4),(UINT8*)&g_gacas_chip_id, 4);
						UPG_PRINTF("--encrypt--aes[%8x][%8x][%8x][OK]\n",encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
						ret = SUCCESS ;
					}					
					else
					{
						MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
					}
				}
				if(b_free == 1)
				{
					FREE((void*)encrypt_data_add_1);
					FREE((void*)encrypt_data_add_2);
				}
			}
			break;
		}
#else
{
			UINT8 *encrypt_data_in		= NULL;
			UINT8 *encrypt_data_1		= NULL;
			UINT8 *encrypt_data_2		= NULL;
			UINT32 encrypt_data_data_len = 0;
			UINT32 encrypt_data_add_1 = 0;
			UINT32 encrypt_data_add_2 = 0;
			

			encrypt_data_data_len 		= s_chunk_info_upg_file[i].sign_len ;
			encrypt_data_in 			= (UINT8 *)(s_chunk_info_upg_file[i].addr);
			encrypt_data_add_1 		= g_gacas_tmp_buf_addr;//(UINT32)MALLOC(encrypt_data_data_len + 0xf);
			encrypt_data_add_2 		= g_gacas_tmp_buf_addr+0x400000;//(UINT32)MALLOC(encrypt_data_data_len + 0xf);
			if(encrypt_data_add_1 != 0)
				encrypt_data_1 		= (UINT8 *)(0xFFFFFFF8 & encrypt_data_add_1);
			if(encrypt_data_add_1 != 0)
				encrypt_data_2 		= (UINT8 *)(0xFFFFFFF8 & encrypt_data_add_2);
			
			UPG_PRINTF("--encrypt_data--1[%8x]2[%8x]len[%8x]\n",encrypt_data_add_1,encrypt_data_add_2,encrypt_data_data_len);
			UPG_PRINTF("--encrypt_data--in[%8x]d[%8x]e[%8x]len[%8x]\n",encrypt_data_in,encrypt_data_1,encrypt_data_2,encrypt_data_data_len);
#ifndef GOS_CONFIG_BOOT
			heap_printf_free_size(1);
#endif
			if(encrypt_data_1 != NULL && encrypt_data_2 != NULL)
			{
				MEMCPY(encrypt_data_1,encrypt_data_in,encrypt_data_data_len);
				//MEMCPY(encrypt_data_2,encrypt_data_in,CHUNK_HEADER_SIZE);
				if(b_data_encypt == 1)
				{
					if(RET_SUCCESS == gacas_decrypt_chunk_data(&encrypt_data_1[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE))
					{
						MEMCPY(&encrypt_data_in[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE);
						//MEMCPY(&encrypt_data_1[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE);
						UPG_PRINTF("--encrypt_data_in--:[%8x]\n",encrypt_data_data_len);
						gacas_aes_printf_bin_16(&encrypt_data_in[0]);
						gacas_aes_printf_bin_16(&encrypt_data_in[16]);
						gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
						gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);		
						/*
						ret = test_rsa_ram((UINT32)encrypt_data_in, encrypt_data_data_len);
						if(ret == 0)
							UPG_PRINTF("--test_rsa_ram-0-[%8x][%8x]OK\n",encrypt_data_in,encrypt_data_data_len);
						else
							UPG_PRINTF("--test_rsa_ram-0-[%8x][%8x]ERROR\n",encrypt_data_in,encrypt_data_data_len);

						if(MEMCMP(&encrypt_data_in[0],&encrypt_data_2[0],CHUNK_HEADER_SIZE) != 0)
						{
							UPG_PRINTF("--encrypt_data_in != encrypt_data_2--010---\n");
						}
						if(MEMCMP(&encrypt_data_in[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE) != 0)
						{
							UPG_PRINTF("--encrypt_data_in != encrypt_data_2--020---\n");
						}
						ret = test_rsa_ram((UINT32)encrypt_data_2, encrypt_data_data_len);
						if(ret == 0)
							UPG_PRINTF("--test_rsa_ram-1-[%8x][%8x]OK\n",encrypt_data_2,encrypt_data_data_len);
						else
							UPG_PRINTF("--test_rsa_ram-1-[%8x][%8x]ERROR\n",encrypt_data_2,encrypt_data_data_len);
						*/
						if (RET_SUCCESS != test_rsa_ram((UINT32)encrypt_data_in,encrypt_data_data_len))
						{
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							UPG_PRINTF("--test_rsa_ram-2-[%8x][%8x]ERROR\n",encrypt_data_1,encrypt_data_data_len);
						}					
						else if(RET_SUCCESS == gacas_encrypt_chunk_by_otp(&encrypt_data_2[CHUNK_HEADER_SIZE],&encrypt_data_in[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE))
						{
							UPG_PRINTF("--test_rsa_ram-2-[%8x][%8x]OK\n",encrypt_data_1,encrypt_data_data_len);
							UPG_PRINTF("--encrypt_data_d--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_2[0]);
							gacas_aes_printf_bin_16(&encrypt_data_2[16]);
							gacas_aes_printf_bin_16(&encrypt_data_2[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_2[encrypt_data_data_len-16]);
							UPG_PRINTF("--encrypt_data_e--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_in[0]);
							gacas_aes_printf_bin_16(&encrypt_data_in[16]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);
							#if 0
							if(MEMCMP(&encrypt_data_in[0],&encrypt_data_2[0],CHUNK_HEADER_SIZE) != 0)
							{
								UPG_PRINTF("--encrypt_data_in != encrypt_data_2--110---\n");
								//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							}
							if(MEMCMP(&encrypt_data_in[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE) != 0)
							{
								UPG_PRINTF("--encrypt_data_in != encrypt_data_2--120---\n");
								//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							}
							//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							gacas_decrypt_chunk_by_otp(&encrypt_data_1[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE);
							if(MEMCMP(&encrypt_data_in[0],&encrypt_data_2[0],CHUNK_HEADER_SIZE) != 0)
							{
								UPG_PRINTF("--encrypt_data_in != encrypt_data_2--310---\n");
								//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							}
							if(MEMCMP(&encrypt_data_in[CHUNK_HEADER_SIZE],&encrypt_data_2[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE) != 0)
							{
								UPG_PRINTF("--encrypt_data_in != encrypt_data_2--320---\n");
								//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
							}
							UPG_PRINTF("--encrypt_data_in--:[%8x]\n",encrypt_data_data_len);
							gacas_aes_printf_bin_16(&encrypt_data_in[0]);
							gacas_aes_printf_bin_16(&encrypt_data_in[16]);
							gacas_aes_printf_bin_16(&encrypt_data_in[32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[48]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-32]);
							gacas_aes_printf_bin_16(&encrypt_data_in[encrypt_data_data_len-16]);
							ret = test_rsa_ram((UINT32)s_chunk_info_upg_file[i].addr, s_chunk_info_upg_file[i].sign_len);
							if(ret == 0)
								UPG_PRINTF("--test_rsa_ram-3-[%8x][%8x]OK\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							else
								UPG_PRINTF("--test_rsa_ram-3-[%8x][%8x]ERROR\n",s_chunk_info_upg_file[i].addr,s_chunk_info_upg_file[i].sign_len);
							gacas_encrypt_chunk_by_otp(&encrypt_data_2[CHUNK_HEADER_SIZE],&encrypt_data_1[CHUNK_HEADER_SIZE],encrypt_data_data_len-CHUNK_HEADER_SIZE);
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							#endif
							MEMCPY((UINT8 *)(s_chunk_info_upg_file[i].addr+s_chunk_info_upg_file[i].offset - 4),(UINT8*)&g_gacas_chip_id, 4);
							UPG_PRINTF("--encrypt--aes[%8x][%8x][%8x][OK]\n",encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
							ret = SUCCESS ;
						}
						else
						{
							MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
						}
					}
				}
				else
				{
					if(RET_SUCCESS == gacas_encrypt_chunk_by_otp(encrypt_data_1,encrypt_data_in,encrypt_data_data_len))
					{
						//MEMCPY(encrypt_data_in,encrypt_data_2,encrypt_data_data_len);
						MEMCPY((UINT8 *)(s_chunk_info_upg_file[i].addr+s_chunk_info_upg_file[i].offset - 4),(UINT8*)&g_gacas_chip_id, 4);
						UPG_PRINTF("--encrypt--aes[%8x][%8x][%8x][OK]\n",encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
						ret = SUCCESS ;
					}					
					else
					{
						MEMCPY(encrypt_data_in,encrypt_data_1,encrypt_data_data_len);
					}
				}
				//FREE((void*)encrypt_data_add_1);
				//FREE((void*)encrypt_data_add_2);
			}
			break;
		}
#endif
	}
	return ret;
}

INT32 gacas_upg_check_file(UINT32 data_addr,UINT32 data_len)
{
	UINT8 *p_data;
	UINT8 i=0;
	UINT8 m=0;
	UINT32 id=0;
	UINT32 mask=0;
	UINT32 upg_file_addr;
	UINT32 start_addr = 0;
	UINT32 chunk_head = 0;
	CHUNK_HEADER t_chh;
    	UINT32 u32_bootOffset = 0;
	
	s_chunk_num_flash = 0;
	s_chunk_num_upg_file = 0;
	MEMSET(s_chunk_info_flash,0,sizeof(s_chunk_info_flash));
	MEMSET(s_chunk_info_upg_file,0,sizeof(s_chunk_info_upg_file));
	p_data = (UINT8*)data_addr;
	chunk_head = (p_data[0]<<24)|(p_data[1]<<16)|(p_data[2]<<8)|(p_data[3]<<0);
	UPG_PRINTF("gacas_check_file----chunk_head[%8x]!\n",chunk_head);
	if(chunk_head == 0x30820122)
	{
		UPG_PRINTF("gacas_check_file----Ali3711c_64M!\n");
	}
	else 
	{
		UPG_PRINTF("gacas_check_file----Ali3711c_64M--ERROR!\n");
		return -1;
	}
	if(TRUE == gacas_loader_db_is_chunk_use())
	{
		gacas_loader_db_get_chunk(s_chunk_info_flash,&s_chunk_num_flash);
		for(i = 0; i < s_chunk_num_flash; i++)
		{
			UPG_PRINTF("flash1-%d-file[%8x]----[%d][%8x][%8x][%8x][%8x]\n",i,s_chunk_info_flash[i].id,s_chunk_info_flash[i].b_update,s_chunk_info_flash[i].addr,s_chunk_info_flash[i].offset,s_chunk_info_flash[i].len,s_chunk_info_flash[i].sign_len);
		}
	}
	else
	{
    		s_chunk_num_flash 		= sto_chunk_count(0,0)+1;
		start_addr = 0;
		for(i = 0; i < s_chunk_num_flash; i++)
		{
			id = 0;
			mask = 0;
			if (i == 0)
			{
				s_chunk_info_flash[i].id			= GACAS_BOOT_CHUNK_ID;
				s_chunk_info_flash[i].offset		= GACAS_BOOT_CHUNK_OFFSET;
				s_chunk_info_flash[i].crc32		= 0x40232425;/* NCRC */
				s_chunk_info_flash[i].len		= 0;
				s_chunk_info_flash[i].sign_len	= 0;
				s_chunk_info_flash[i].ver 		= 0;
				s_chunk_info_flash[i].addr		= start_addr;
				t_chh.offset = GACAS_BOOT_CHUNK_OFFSET;
			}
			else if((unsigned long)ERR_FAILUE != sto_chunk_goto(&id, mask, i))
			{
				if(sto_get_chunk_header(id, &t_chh) != 1)
				{
					break;
				}
				s_chunk_info_flash[i].id			= t_chh.id;
				s_chunk_info_flash[i].offset		= t_chh.offset;
				s_chunk_info_flash[i].crc32		= t_chh.crc;
				s_chunk_info_flash[i].len		= (t_chh.len+CHUNK_NAME);
				s_chunk_info_flash[i].sign_len	= (t_chh.len +CHUNK_NAME+ SIG_ALIGN_LEN - 1) / SIG_ALIGN_LEN * SIG_ALIGN_LEN + SIG_LEN;
				s_chunk_info_flash[i].ver 		= gacas_loader_str2uint32(t_chh.version, 8);
				s_chunk_info_flash[i].addr		= start_addr;
				if(s_chunk_info_flash[i].offset == 0)
					s_chunk_info_flash[i].offset = s_chunk_info_flash[i].len;
				UPG_PRINTF("flash2-%d-file[%8x]----[%d][%8x][%8x][%8x][%8x]\n",i,t_chh.id,s_chunk_info_flash[i].b_update,s_chunk_info_flash[i].addr,t_chh.offset,s_chunk_info_flash[i].len,s_chunk_info_flash[i].sign_len);
			}
            		start_addr += t_chh.offset;
		}	
		if(i == s_chunk_num_flash && s_chunk_num_flash > 0)
		{
			gacas_loader_db_set_chunk(s_chunk_info_flash,s_chunk_num_flash);
			gacas_loader_db_save();
		}
	}	
	
	upg_file_addr = data_addr;
	gacas_loader_get_boot_len(&u32_bootOffset);
	chunk_init(data_addr+u32_bootOffset, data_len-u32_bootOffset);
	/* Boot chunk需要加上 */
	s_chunk_num_upg_file 	= chunk_count(0,0)+1;
	UPG_PRINTF("UPG---file[%8x][%8x]----chunk_count[%d]\n",data_addr,data_len,s_chunk_num_upg_file);
	start_addr = 0;
	for(i = 0; i < s_chunk_num_upg_file; i++)
	{
		id = 0;
		mask = 0;
        /* flash chunk */
		if (i != 0)
		{
			if (!chunk_goto(&id, mask, i) && (i>0))
			{
				continue;
			}
			if(get_chunk_header(id, &t_chh) != 1)
			{
				break;
			}
			s_chunk_info_upg_file[i].id		= t_chh.id;
			s_chunk_info_upg_file[i].offset	= t_chh.offset;
			s_chunk_info_upg_file[i].crc32	= t_chh.crc;
			s_chunk_info_upg_file[i].len		= (t_chh.len+CHUNK_NAME);
			s_chunk_info_upg_file[i].sign_len	= (t_chh.len +CHUNK_NAME+ SIG_ALIGN_LEN - 1) / SIG_ALIGN_LEN * SIG_ALIGN_LEN + SIG_LEN;
			s_chunk_info_upg_file[i].ver 		= gacas_loader_str2uint32(t_chh.version, 8);
			s_chunk_info_upg_file[i].addr	= start_addr+upg_file_addr;
			if(s_chunk_info_upg_file[i].offset == 0)
				s_chunk_info_upg_file[i].offset = s_chunk_info_upg_file[i].len;           
	        }
	        else/* bootloader */
	        {
	            		s_chunk_info_upg_file[i].id		= GACAS_BOOT_CHUNK_ID;
				s_chunk_info_upg_file[i].offset	= GACAS_BOOT_CHUNK_OFFSET;
				s_chunk_info_upg_file[i].crc32	= 0x40232425;/* NCRC */
				s_chunk_info_upg_file[i].len		= 0;
				s_chunk_info_upg_file[i].sign_len	= 0;
				s_chunk_info_upg_file[i].ver 		= 0;
				s_chunk_info_upg_file[i].addr	= start_addr+upg_file_addr;
	            		t_chh.offset = GACAS_BOOT_CHUNK_OFFSET;
	        }

	        start_addr += t_chh.offset;
	        for(m= 0;m<s_chunk_num_flash;m++)
	        {
	            if(s_chunk_info_upg_file[i].id == s_chunk_info_flash[m].id && s_chunk_info_flash[m].b_update == TRUE)
	            {
	                if(s_chunk_info_flash[m].offset >= s_chunk_info_upg_file[i].offset)
	                {
	                    s_chunk_info_upg_file[i].b_update = TRUE;
	                    UPG_PRINTF("------------------update\n");
	                }
	            }       
	        }
	}
	return 0;
}

void gacas_upg_init(gacas_upg_show_process progress_show)
{
	g_progress_show = progress_show;
	otp_init(NULL);
	if(otp_read(0x0*4,(UINT8*) &g_gacas_chip_id, 4) < OTP_DW_LEN)
	{
		UPG_PRINTF ( "%S:gacas_upg_init chip_id fail !!\n", __FUNCTION__ );
		return;
	}
	UPG_PRINTF("gacas_upg_init********[%8x]\n",g_gacas_chip_id);
}

void gacas_upg_flash_lock(void)
{
	gacas_loader_flash_lock(0,GACAS_UPG_FLASH_LOCK_SIZE);
}
void gacas_upg_flash_unlock(void)
{
	gacas_loader_flash_unlock(0,GACAS_UPG_FLASH_LOCK_SIZE);
}

