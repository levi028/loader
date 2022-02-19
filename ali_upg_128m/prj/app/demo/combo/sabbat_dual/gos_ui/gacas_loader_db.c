#include "gos_config.h"
#ifndef GOS_CONFIG_BOOT
#include <api/libc/string.h>
#include <bus/otp/otp.h>
//#include <hld/dmx/dmx_dev.h>
//#include <hld/dmx/dmx.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <api/libc/fast_crc.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
//#include <hld/trng/trng.h>
//#include <api/libga/lib_ga.h> 
#include <api/librsa/flash_cipher.h>
#include "api/libdb/db_config.h"
#include "api/libdb/db_node_combo.h"
#include "gacas_loader_db.h"
#include "gacas_aes.h"
#include "../copper_common/system_type.h"
#else
#include "bootloader.h"
#include "gacas_loader_db.h"
#include "gacas_aes.h"
#include "gacas_otp.h"
#include <ali/otp.h>
#include "../../../boot/pub/boot_logo.h"
#endif
//#define LOADER_DB_PRINTF(...) do{}while(0)
#define LOADER_DB_PRINTF libc_printf
GACAS_LOADER_DB_T g_loader_db;
UINT32	g_loader_db_store_index = 0xfe;
UINT32	g_loader_db_ota_start_addr = 0x140000;
UINT32	g_gacas_chip_id = 0;
UINT32	g_gacas_mak_id = 0;
typedef struct
{
	UINT32 id;
	UINT32 offset;
}GA_DEFAULT_CHUNK_INFO;

const GA_DEFAULT_CHUNK_INFO c_default_chunk_id[] = 
{
	{0x30820122,	0x80000},

	{0x08F70101,	0x10000},
	{0x00FF0201,	0x48000},
	{0x20DF0100,	0x7D80},	
	{0x09F60100,	0x0200},	
	{0x21DE0100,	0x10080},   
	{0x20DF0200,	0x10000},

	{0x00FF0100,	0xC0000},
	{0x07F80000,	0x40000},
        
	{0x01FE0101,	0x200000},
	{0x06F90101,	0x100000},

	{0x02FD0100,	0xff80},	
	{0x0AF50100,	0x10080},	
	{0x02FD0200,	0x10000},	
	{0x02FD0300,	0xFF80},
	
	{0x05FA0100,	0x180080},
	
	{0x03FC0100,    0xff80},
	{0x04FB0100,	0x130080},	
};
#ifdef GOS_CONFIG_APP
GOS_DEFAULT_INFO_T gos_default_info;//add by yuj for default config tool
UINT8 gos_default_info_is_ok = 0;//add by yuj for default config tool
#endif
unsigned long gacas_loader_default_chunk_id_find(unsigned long chunk_id)
{
	unsigned long len = 0;
	unsigned long i = 0;
	unsigned long chunk_num = sizeof(c_default_chunk_id)/sizeof(c_default_chunk_id[0]);

	for (i = 0; i < chunk_num; i++)
	{
		if (c_default_chunk_id[i].id == chunk_id)
			break;
		len += c_default_chunk_id[i].offset;
	}
	if (i == chunk_num)
		return ERR_FAILUE;
	return len;
}
unsigned long gacas_loader_default_chunk_num(void)
{
	unsigned long chunk_num = sizeof(c_default_chunk_id)/sizeof(c_default_chunk_id[0]);
	return chunk_num;
}

void gacas_loader_chip_id_init(void)
{
   	UINT32 buf_0x00=0;
		
    	otp_init(NULL);
    	otp_read(0x00*4, (UINT8 *)&buf_0x00, 4) ;
	g_gacas_chip_id =  buf_0x00;
    	otp_read(0x02*4, (UINT8 *)&buf_0x00, 4) ;
	g_gacas_mak_id =  buf_0x00;
	LOADER_DB_PRINTF("[LOADER_DB]gacas_loader_chip_id_init----CAID[%08x][%u]\n",g_gacas_chip_id,g_gacas_chip_id);
	LOADER_DB_PRINTF("[LOADER_DB]gacas_loader_chip_id_init----MSID[%08x][%u]\n",g_gacas_mak_id,g_gacas_mak_id);
	if(g_gacas_mak_id == 0)
	{
		g_gacas_mak_id = 0x12345678;//OTP_VALUE_MSID;
	}
#ifdef GOS_CONFIG_BOOT
	LOADER_DB_PRINTF("[LOADER_DB]gacas_loader_chip_id_init----(%d ms)\n",read_tsc()/MS_TICKS);
#ifdef GOS_CONFIG_HDCP
	if(g_gacas_chip_id != 0 && g_gacas_chip_id != 0xffffffff)
	{
		gacas_otp_write_hdcp_protect_key();
		gacas_cmd_requst_hdcp_key(g_gacas_chip_id);
	}
#endif
#endif
}
UINT32 gacas_loader_crc32(UINT8 *szData, UINT32 size)
{
	return mg_table_driven_crc(0xFFFFFFFF,szData,size);;	
	//return gacas_aes_crc32(szData,size);
}
INT8 gacas_loader_char2hex(UINT8 ch)
{
    INT8 ret =  - 1;

    if ((ch <= 0x39) && (ch >= 0x30))
    {
            // '0'~'9'
        ret = ch &0xf;
    }
    else if ((ch <= 102) && (ch >= 97))
    {
            //'a'~'f'
        ret = ch - 97+10;
    }
    else if ((ch <= 70) && (ch >= 65))
    {
            //'A'~'F'
        ret = ch - 65+10;
    }

    return ret;
}


UINT32 gacas_loader_str2uint32(UINT8 *str, UINT8 len)
{
    UINT32 ret = 0;
    UINT8 i=0;
    INT temp=0;


    if (NULL == str)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        temp = gacas_loader_char2hex(*str);
        str++;
        if (-1 == temp)
        {
            return 0;
        }

        ret = (ret << 4) | temp;
    }

    return ret;
}

INT32 gacas_loader_db_save(void)
{
	UINT32 chid = 0;
	UINT32 block_addr = 0;
	UINT32 block_len = 0;
	UINT32 block_offset = 0;
	struct sto_device *sto_dev = NULL;
	RET_CODE ret = RET_FAILURE;
	UINT32 param[2];
	
    LDR_PRINT("in (u8_type=%d, store_index=%d) \n", g_loader_db.u8_type, g_loader_db_store_index);	
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == sto_dev)
	{
		LDR_PRINT("[LD]Error-- can't get sto device.\n");
		return RET_FAILURE;
	}	
	chid = CHUNK_ID_LOADER_DB;
	block_len = 0;
	ret = sto_get_chunk_len(chid, &block_addr, &block_len);
	if ((RET_SUCCESS != ret) || (0 == block_len))
	{
		LDR_PRINT("[LD]Error--can't find chunk 0x%08x.\n",  chid);
        ret = gacas_loader_db_get_chunk_len(chid, &block_addr, &block_len,&block_offset);
		if ((RET_SUCCESS != ret) || (0 == block_len))		
          block_addr = GACAS_LOADER_DB_FLASH_ADDR;
	}
	param[0] = block_addr;
	param[1] = GACAS_FLASH_BLOCK_SIZE >> 10;
	if(g_loader_db_store_index >= GACAS_LOADER_DB_STORE_NUM)
	{
		LDR_PRINT("[LD]---ERASE---[%8x]", block_addr);
		if (RET_SUCCESS == sto_io_control(sto_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param))
		{
			LDR_PRINT("-OK\n");
			g_loader_db_store_index = 0;
		}
		else
		{
			LDR_PRINT("-FAILURE\n");
			return RET_FAILURE;
		}	
	}
	block_addr = block_addr+(GACAS_FLASH_BLOCK_SIZE/GACAS_LOADER_DB_STORE_NUM*g_loader_db_store_index);
	block_len = sizeof(GACAS_LOADER_DB_T);
	g_loader_db.u32_crc32 = gacas_loader_crc32((UINT8*)&g_loader_db,block_len-8);
	LDR_PRINT("[LD]---WRITE---[%2d][%8x][%8x][%8x]", g_loader_db_store_index,block_addr,block_len,g_loader_db.u32_crc32);
	if ((INT32)block_len != sto_put_data(sto_dev, block_addr,(UINT8*)&g_loader_db, block_len))
	{
		LDR_PRINT("-FAILURE\n");
		return RET_FAILURE;
	}
	LDR_PRINT("-OK\n");
	g_loader_db_store_index++;
	return RET_SUCCESS;
}

BOOL gacas_loader_db_is_chunk_use(void)
{
	if(g_loader_db.u16_chunk_num > 0 && g_loader_db.u16_chunk_num <= GACAS_MAX_CHUNK_NUM)
	{
		if(g_loader_db.u16_chunk_use == 1)
			return TRUE;
	}
	return FALSE;
}
BOOL gacas_loader_db_is_usb(void)
{
	if(g_loader_db.u8_type == GACAS_LOADER_TYPE_USB_MANNAL
	||g_loader_db.u8_type == GACAS_LOADER_TYPE_USB_BOOT)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
}
}

BOOL gacas_loader_db_is_ota_force(void)
{
	if(g_loader_db.u8_type == GACAS_LOADER_TYPE_OTA_FORCE)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL gacas_loader_db_is_upg(void)
{
	if(g_loader_db.u8_type == GACAS_LOADER_TYPE_OTA_FORCE
	|| g_loader_db.u8_type == GACAS_LOADER_TYPE_OTA_MANNAL
	|| g_loader_db.u8_type == GACAS_LOADER_TYPE_USB_MANNAL
	|| g_loader_db.u8_type == GACAS_LOADER_TYPE_USB_BOOT)
	{
		if(g_loader_db.u8_times > 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
BOOL gacas_loader_db_is_back_up(void)
{
	if(g_loader_db.u32_loader_index == 1)
		return TRUE;
	else
		return FALSE;
}
void gacas_loader_db_deal_one_time(UINT8 type)
{
	UINT16 i;

	if(type == g_loader_db.u8_type && g_loader_db.u8_type < GACAS_LOADER_TYPE_BUTT)
	{
		if(g_loader_db.u8_times > 0)
			g_loader_db.u8_times --;
		if(g_loader_db.u8_times == 0)
		{
			g_loader_db.u8_type 			= GACAS_LOADER_TYPE_BUTT;
			g_loader_db.u32_flash_addr 	= 0;
			g_loader_db.u32_flash_len 	= 0;
			for(i = 0; i < g_loader_db.u16_chunk_num; i++)
			{
				g_loader_db.st_chunk[i].b_update = FALSE;
			}	
		}
		gacas_loader_db_save();
	}
}

void gacas_loader_db_deal_one_time_all(void)
{
	//UINT16 i;

	if(g_loader_db.u8_type < GACAS_LOADER_TYPE_BUTT)
	{
		if(g_loader_db.u8_times > 5)
		{
			g_loader_db.u8_times = 5;
		}
		if(g_loader_db.u8_times > 0)
		{
			g_loader_db.u8_times --;
		}
        LDR_PRINT("bf gacas_loader_db_save (u8_type=%d, u8_times=%d)\n", g_loader_db.u8_type, g_loader_db.u8_times);
		gacas_loader_db_save();
	}
}

void gacas_loader_db_ota_success(void)
{
//	UINT16 i;

	if(g_loader_db.u8_type < GACAS_LOADER_TYPE_BUTT)
	{
		g_loader_db.u8_type 			= GACAS_LOADER_TYPE_BUTT;
		//按flash块升级后需要更新chunk_list
		//if(g_loader_db.u32_flash_addr >= 0 && g_loader_db.u32_flash_len > 0)
		if( g_loader_db.u32_flash_len > 0)
			g_loader_db.u16_chunk_use = 0;
		g_loader_db.u32_flash_addr 	= 0;
		g_loader_db.u32_flash_len 	= 0;
		g_loader_db.u8_times			= 0;
		g_loader_db.u32_file_id		= g_loader_db.u32_file_id_new;
		g_loader_db_store_index 	= 0xff;
		gacas_loader_db_save();
	}
}

BOOL gacas_loader_db_is_ota_by_addr(UINT32 *p_addr,UINT32 *p_size)
{
	LOADER_DB_PRINTF("[LD]gacas_loader_db_is_ota_by_addr-------[%8x][%8x]\n",g_loader_db.u32_flash_addr,g_loader_db.u32_flash_len);
	//if(g_loader_db.u32_flash_addr >= 0 && g_loader_db.u32_flash_len > 0)
	if( g_loader_db.u32_flash_len > 0)
	{
		*p_addr = g_loader_db.u32_flash_addr;
		*p_size 	= g_loader_db.u32_flash_len;
		return TRUE;
	}
	return FALSE;
}

INT32 gacas_loader_db_clear_chunk_update(void)
{
	UINT16 i;
	LOADER_DB_PRINTF("[LD]gacas_loader_db_clear_chunk_update-------\n");
	for(i = 0; i < g_loader_db.u16_chunk_num; i++)
	{
		g_loader_db.st_chunk[i].b_update = FALSE;
	}	
	return RET_SUCCESS;
}
INT32 gacas_loader_db_set_chunk(LOADER_CHUNK *p_chuck,UINT16 num)
{
	LOADER_DB_PRINTF("[LD]gacas_loader_db_set_chunk-------[%d]\n",num);
	if(num > GACAS_MAX_CHUNK_NUM)
	{
		return RET_FAILURE;
	}
	if(g_loader_db.u16_chunk_num != num || (MEMCMP(p_chuck,g_loader_db.st_chunk,sizeof(LOADER_CHUNK)*num) != 0))
	{		
		MEMCPY(g_loader_db.st_chunk,p_chuck,sizeof(LOADER_CHUNK)*num);
		g_loader_db.u16_chunk_use 	= 1;
		g_loader_db.u16_chunk_num 	= num;
	}
	return RET_SUCCESS;
}
INT32 gacas_loader_db_get_chunk(LOADER_CHUNK *p_chuck,UINT16 *p_num)
{
	LOADER_DB_PRINTF("[LD]gacas_loader_db_get_chunk-------[%d]\n",g_loader_db.u16_chunk_num);
	if(p_chuck == NULL || p_num == NULL)
	{
		return RET_FAILURE;
	}
	if(g_loader_db.u16_chunk_num > 0 && g_loader_db.u16_chunk_use == 1)
	{		
		MEMCPY(p_chuck,g_loader_db.st_chunk,sizeof(LOADER_CHUNK)*g_loader_db.u16_chunk_num);
		*p_num = g_loader_db.u16_chunk_num;
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}
INT32 gacas_loader_db_get_ota_cab(UINT16 *p_pid,UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod)
{
	if(g_loader_db.u32_sig_type == GACAS_TUNER_SIG_TYPE_CAB)
	{
		*p_pid 		= g_loader_db.u16_pid;
		*p_freq		= g_loader_db.st_cab.u32_freq_kHz;
		*p_symbol	= g_loader_db.st_cab.u32_symb_kbps;
		*p_mod		= g_loader_db.st_cab.u32_modulation;
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}
INT32 gacas_loader_db_set_ota_cab(UINT16 pid,UINT32 freq,UINT32 symbol,UINT32 mod)
{
	g_loader_db.u32_sig_type 			= GACAS_TUNER_SIG_TYPE_CAB;
	g_loader_db.u16_pid				= pid;
	g_loader_db.st_cab.u32_freq_kHz	= freq;
	g_loader_db.st_cab.u32_symb_kbps	= symbol;
	g_loader_db.st_cab.u32_modulation	= mod;
	return RET_SUCCESS;
}
UINT32 gacas_loader_db_get_chip_id(void)
{
	return g_gacas_chip_id;
}
UINT32 gacas_loader_db_get_file_id_new(void)
{
	return g_loader_db.u32_file_id_new;
}
UINT32 gacas_loader_db_get_file_id(void)
{
	return g_loader_db.u32_file_id;
}
INT32 gacas_loader_db_set_file_id_new(UINT32 file_id)
{
	if(g_loader_db.u32_file_id_new != file_id)
	{
		g_loader_db.u32_file_id_new = file_id;
	}
	return RET_SUCCESS;
}

INT32 	gacas_loader_db_get_centrt_tp(UINT8 tp_index,UINT32 	u32_sig_type,void*p_info)
{
	if(tp_index >= 2 || u32_sig_type >= GACAS_TUNER_SIG_TYPE_BUTT || p_info == NULL)
		return RET_FAILURE;
	//LOADER_DB_PRINTF("[LD]gacas_loader_db_get_centrt_tp-------	[%d][%d]\n",tp_index,u32_sig_type);
	if(u32_sig_type == GACAS_TUNER_SIG_TYPE_CAB)
	{
		//GACAS_TUNER_CAB_PARA_T 	t_cab;       	/**<Cable signal parameter *//**<CNcomment: 有线信号参数 */
		
		//LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz);
		//LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_symb_kbps);
		//LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_modulation);
		MEMCPY(p_info,&g_loader_db.st_centrt_cab[tp_index],sizeof(GACAS_TUNER_CAB_PARA_T));
	}
	else if(u32_sig_type == GACAS_TUNER_SIG_TYPE_SAT)
	{
		//GACAS_TUNER_SAT_PARA_T 	t_sat;        	/**<Satellite signal parameter*//**<CNcomment: 卫星信号参数 */
		#ifdef DEF_SAT_BY_GOS
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_freq_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_symb_kbps);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_polar);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_low_LO-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_low_LO);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_high_LO-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_high_LO);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_switch_22K-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_switch_22K);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_LNB_power-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_LNB_power);
		#else
		LOADER_DB_PRINTF("[LD]st_centrt_sat-sat_id-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].sat_id);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_high-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_high);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_low-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_low);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_power-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_power);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_type-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_type);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-frq-------			[%d]\n",g_loader_db.st_centrt_sat[tp_index].frq);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-sym-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].sym);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-pol-------		[%d][%d]\n",g_loader_db.st_centrt_sat[tp_index].pol,g_loader_db.st_centrt_sat[tp_index].pol_tp);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-k22-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].k22);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-v12-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].v12);		
		#endif
		MEMCPY(p_info,&g_loader_db.st_centrt_sat[tp_index],sizeof(GACAS_TUNER_SAT_PARA_T));
	}
	else if(u32_sig_type == GACAS_TUNER_SIG_TYPE_TER)
	{
		//GACAS_TUNER_TER_PARA_T 	t_ter;        	/**<Terrestrial signal parameter*//**<CNcomment: 地面信号参数 */
		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_freq_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_band_width_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_band_width_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_modulation);	
		MEMCPY(p_info,&g_loader_db.st_centrt_ter[tp_index],sizeof(GACAS_TUNER_TER_PARA_T));
	}
	return RET_SUCCESS;
}

INT32 	gacas_loader_db_set_centrt_tp(UINT8 tp_index,UINT32 	u32_sig_type,void*p_info)
{
	if(tp_index >= 2 || u32_sig_type >= GACAS_TUNER_SIG_TYPE_BUTT || p_info == NULL)
		return RET_FAILURE;
	LOADER_DB_PRINTF("[LD]gacas_loader_db_set_centrt_tp-------	[%d][%d]\n",tp_index,u32_sig_type);
	if(u32_sig_type == GACAS_TUNER_SIG_TYPE_CAB)
	{
		MEMCPY(&g_loader_db.st_centrt_cab[tp_index],p_info,sizeof(GACAS_TUNER_CAB_PARA_T));		
		LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_symb_kbps);
		LOADER_DB_PRINTF("[LD]st_centrt_cab-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_cab[tp_index].u32_modulation);
	}
	else if(u32_sig_type == GACAS_TUNER_SIG_TYPE_SAT)
	{
		MEMCPY(&g_loader_db.st_centrt_sat[tp_index],p_info,sizeof(GACAS_TUNER_SAT_PARA_T));
		#ifdef DEF_SAT_BY_GOS
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_freq_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_symb_kbps);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_polar);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_low_LO-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_low_LO);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_high_LO-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_high_LO);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_switch_22K-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_switch_22K);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-u32_LNB_power-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].u32_LNB_power);
		#else
		LOADER_DB_PRINTF("[LD]st_centrt_sat-sat_id-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].sat_id);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_high-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_high);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_low-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_low);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_power-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_power);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-lnb_type-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].lnb_type);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-frq-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].frq);
		LOADER_DB_PRINTF("[LD]st_centrt_sat-sym-------	[%d]\n",g_loader_db.st_centrt_sat[tp_index].sym);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-pol-------		[%d][%d]\n",g_loader_db.st_centrt_sat[tp_index].pol,g_loader_db.st_centrt_sat[tp_index].pol_tp);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-k22-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].k22);		
		LOADER_DB_PRINTF("[LD]st_centrt_sat-v12-------		[%d]\n",g_loader_db.st_centrt_sat[tp_index].v12);		
		#endif
	}
	else if(u32_sig_type == GACAS_TUNER_SIG_TYPE_TER)
	{
		MEMCPY(&g_loader_db.st_centrt_ter[tp_index],p_info,sizeof(GACAS_TUNER_TER_PARA_T));

		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_freq_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_band_width_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_band_width_kHz);
		LOADER_DB_PRINTF("[LD]st_centrt_ter-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_ter[tp_index].u32_modulation);	
	}
	return RET_SUCCESS;
}
INT32 gacas_loader_db_get_reserved(UINT8 *p_data,UINT32 *p_len)
{
	if(g_loader_db.reserved_len > 16 || p_data == NULL ||p_len == NULL )
		return RET_FAILURE;
	*p_len = g_loader_db.reserved_len;
	MEMCPY(p_data,&g_loader_db.reserved[0],g_loader_db.reserved_len);
	return RET_SUCCESS;
}
INT32 gacas_loader_db_set_reserved(UINT8 *p_data,UINT32 len)
{
	if(p_data == NULL)
		return RET_FAILURE;
	if(len > 16 )
		return RET_FAILURE;
	if(MEMCMP(&g_loader_db.reserved[0],p_data,len) != 0)
	{
		MEMCPY(&g_loader_db.reserved[0],p_data,len);
		 g_loader_db.reserved_len = len;
		gacas_loader_db_save();
	}
	return RET_SUCCESS;
}
INT32 gacas_loader_db_clear_reserved(void)
{
	if(g_loader_db.reserved_len > 0)
	{
		MEMSET(&g_loader_db.reserved[0],0,16);
		g_loader_db.reserved_len = 0;	
		gacas_loader_db_save();
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}
INT32 gacas_loader_db_is_first_time(void)
{
	if(g_loader_db.b_first_time == 1)
	{
		return RET_FAILURE;
	}
	else
	{
		g_loader_db.b_first_time = 1;
		gacas_loader_db_save();
		return RET_SUCCESS;
	}
}
void gacas_loader_db_set_key(UINT32 u32_key_up,UINT32 u32_key_down,UINT32 u32_key_left,UINT32 u32_key_right)
{
	if(	u32_key_up != g_loader_db.u32_key_up
	||	u32_key_down != g_loader_db.u32_key_down
	||	u32_key_left != g_loader_db.u32_key_left
	||	u32_key_right != g_loader_db.u32_key_right)
	{
		g_loader_db.u32_key_up 	= u32_key_up;
		g_loader_db.u32_key_down 	= u32_key_down;
		g_loader_db.u32_key_left	= u32_key_left;
		g_loader_db.u32_key_right	= u32_key_right;
		gacas_loader_db_save();
	}
}
INT32 gacas_loader_db_get_key(UINT32 *p_key_up,UINT32 *p_key_down,UINT32 *p_key_left,UINT32 *p_key_righ)
{
	*p_key_up 		= g_loader_db.u32_key_up;
	*p_key_down 	= g_loader_db.u32_key_down;
	*p_key_left 		= g_loader_db.u32_key_left;
	*p_key_righ 		= g_loader_db.u32_key_right;
	return RET_SUCCESS;
}

UINT32 gacas_loader_db_get_run_area(void)
{
	return g_loader_db.u32_run_area;
}
void gacas_loader_db_set_run_area(UINT32 run_area)
{
	if(run_area != g_loader_db.u32_run_area)
	{
		g_loader_db.u32_run_area = run_area;
		gacas_loader_db_save();
	}
}
UINT32 gacas_loader_db_get_nit_ver(void)
{
	return g_loader_db.u8_nit_ver;
}
void gacas_loader_db_set_nit_ver(UINT8 nit_ver)
{
	if(nit_ver != g_loader_db.u8_nit_ver)
	{
		g_loader_db.u8_nit_ver = nit_ver;
		gacas_loader_db_save();
	}
}
UINT8 gacas_loader_db_get_usb_log(void)
{
	if(g_loader_db_store_index 	== 0xfe)
		return 1;
	return g_loader_db.u8_usb_log_out;
}
void gacas_loader_db_set_usb_log_out(UINT8 usb_log_out)
{
	if(usb_log_out != g_loader_db.u8_usb_log_out)
	{
		g_loader_db.u8_usb_log_out = usb_log_out;
		gacas_loader_db_save();
	}
}
UINT32 gacas_loader_db_get_crc32_upg(void)
{
	return g_loader_db.u32_crc32_upg;
}
void gacas_loader_db_set_crc32_upg(UINT32 crc32_upg)
{
	if(crc32_upg != g_loader_db.u32_crc32_upg)
	{
		g_loader_db.u32_crc32_upg = crc32_upg;
		gacas_loader_db_save();
	}
}
UINT32 gacas_loader_db_get_smc_id(void)
{
	if(g_loader_db.u32_smc_id_bak != g_loader_db.u32_smc_id)
		return 0;
	return g_loader_db.u32_smc_id;
}
void gacas_loader_db_set_smc_id(UINT32 smc_id)
{
	if(smc_id != g_loader_db.u32_smc_id || smc_id != g_loader_db.u32_smc_id_bak)
	{
		g_loader_db.u32_smc_id = smc_id;
		g_loader_db.u32_smc_id_bak 		= smc_id;
		gacas_loader_db_save();
	}
}
INT32 gacas_loader_db_set_av_default_mode(UINT8 default_mode)
{
	if(g_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && g_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
	{
		if(g_loader_db.default_mode  != default_mode)
		{
			g_loader_db.default_mode= default_mode;
			gacas_loader_db_save();	
		}
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}
UINT8 gacas_loader_db_get_av_default_mode(void)
{
	if(g_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && g_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
	{
		return g_loader_db.default_mode;
	}
	return 0;	
}
INT32 gacas_loader_db_set_av(UINT8 tv_mode, UINT8 tv_ratio, UINT8 display_mode, UINT8 scart_out)
{
	libc_printf("%s --1--[%d][%d][%d][%d]\n",__FUNCTION__,tv_mode,tv_ratio,display_mode,scart_out);
	if(g_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && g_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
	{
		libc_printf("%s --2--[%d][%d][%d][%d][%d]\n",__FUNCTION__,g_loader_db.default_mode,g_loader_db.tv_mode,g_loader_db.tv_ratio,g_loader_db.display_mode,g_loader_db.scart_out);
		if(g_loader_db.default_mode == 0)
		{
			if(tv_mode != g_loader_db.tv_mode 
			|| tv_ratio != g_loader_db.tv_ratio
			|| display_mode != g_loader_db.display_mode
			|| scart_out != g_loader_db.scart_out)
			{
				g_loader_db.tv_mode 		= tv_mode;
				g_loader_db.tv_ratio 		= tv_ratio;
				g_loader_db.display_mode 	= display_mode;
				g_loader_db.scart_out 	= scart_out;
				g_loader_db.default_mode 	= 0;
				gacas_loader_db_save();
			}
		}
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}

INT32 gacas_loader_db_get_av(UINT8 *p_tv_mode, UINT8 *p_tv_ratio, UINT8 *p_display_mode, UINT8 *p_scart_out)
{
	if(g_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && g_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
	{
		libc_printf("%s --1--[%d][%d][%d][%d][%d]\n",__FUNCTION__,g_loader_db.default_mode,g_loader_db.tv_mode,g_loader_db.tv_ratio,g_loader_db.display_mode,g_loader_db.scart_out);
		if(	g_loader_db.tv_mode >= TV_MODE_COUNT
		|| 	g_loader_db.tv_ratio > TV_ASPECT_RATIO_169 
		|| 	g_loader_db.scart_out > SCART_YUV 
		|| 	g_loader_db.display_mode > DISPLAY_MODE_PANSCAN )
		{
			g_loader_db.tv_mode		= TV_MODE_1080I_25;
			g_loader_db.tv_ratio			= TV_ASPECT_RATIO_AUTO;
			g_loader_db.display_mode		= DISPLAY_MODE_PANSCAN;
			g_loader_db.scart_out		= SCART_YUV;	
		}
		if( g_loader_db.tv_mode == 0
		&& g_loader_db.tv_ratio == 0
		&& g_loader_db.scart_out == 0
		&& g_loader_db.display_mode == 0)
		{
			g_loader_db.tv_mode			= TV_MODE_1080I_25;
			g_loader_db.tv_ratio			= TV_ASPECT_RATIO_AUTO;
			g_loader_db.display_mode		= DISPLAY_MODE_PANSCAN;
			g_loader_db.scart_out		= SCART_YUV;	
		}
		if(g_loader_db.default_mode == 2)
		{
			*p_tv_mode 		= TV_MODE_1080I_25;
			*p_tv_ratio 		= TV_ASPECT_RATIO_AUTO;
			*p_scart_out 	= SCART_YUV;
			*p_display_mode 	= DISPLAY_MODE_PANSCAN;
		}
		else if(g_loader_db.default_mode == 1)
		{
			*p_tv_mode 		= TV_MODE_PAL;
			*p_tv_ratio 		= TV_ASPECT_RATIO_43;
			*p_scart_out 	= SCART_YUV;
			*p_display_mode 	= DISPLAY_MODE_PANSCAN;
		}
		else
		{
			*p_tv_mode = g_loader_db.tv_mode;
			*p_tv_ratio = g_loader_db.tv_ratio;
			*p_display_mode = g_loader_db.display_mode;
			*p_scart_out = g_loader_db.scart_out;	
		}
		libc_printf("%s --2--[%d][%d][%d][%d]\n",__FUNCTION__,*p_tv_mode,*p_tv_ratio,*p_display_mode,*p_scart_out);
		return RET_SUCCESS;
	}
	return RET_FAILURE;
}

INT32 gacas_loader_db_set_chunk_update(UINT32 id)
{
	UINT16 i;
	for(i = 0; i < g_loader_db.u16_chunk_num; i++)
	{
		if(id == g_loader_db.st_chunk[i].id)
		{
			LOADER_DB_PRINTF("[LD]gacas_loader_db_set_chunk_update-------[%8x]TRUE\n",id);
			g_loader_db.st_chunk[i].b_update = TRUE;
			return RET_SUCCESS;
		}
	}	
	return RET_FAILURE;
}
INT32 gacas_loader_db_set_type(UINT8 type,UINT32 start_addr,UINT32 size,UINT8 times)
{
	LOADER_DB_PRINTF("[LD]gacas_loader_db_set_type-------[%x][%8x][%8x][%x]\n",type,start_addr,size,times);
	if(type > GACAS_LOADER_TYPE_BUTT)
		return RET_FAILURE;
	g_loader_db.u8_type			= type;
	g_loader_db.u32_flash_addr	= start_addr/GACAS_FLASH_BLOCK_SIZE*GACAS_FLASH_BLOCK_SIZE;
	g_loader_db.u32_flash_len		= size/GACAS_FLASH_BLOCK_SIZE*GACAS_FLASH_BLOCK_SIZE;
	g_loader_db.u8_times			= times;	
	if(g_loader_db.u32_flash_len	> 0)
	{
		if((g_loader_db.u32_flash_len + g_loader_db.u32_flash_addr) > GACAS_FLASH_TOTAL_SIZE)
		{
			g_loader_db.u32_flash_len = GACAS_FLASH_TOTAL_SIZE - g_loader_db.u32_flash_addr;
		}
/*		if(g_loader_db.u32_flash_addr <= g_loader_db_ota_start_addr)
		{
			if(g_loader_db.u32_flash_len > (g_loader_db_ota_start_addr - g_loader_db.u32_flash_addr))
			{
				g_loader_db.u32_flash_len		= g_loader_db.u32_flash_len - (g_loader_db_ota_start_addr - g_loader_db.u32_flash_addr);
				g_loader_db.u32_flash_addr 	= g_loader_db_ota_start_addr;
			}
			else
			{
				g_loader_db.u32_flash_addr	= 0;
				g_loader_db.u32_flash_len		= 0;
			}
		}
*/
	}
	LOADER_DB_PRINTF("[LD]gacas_loader_db_set_type-------[%x][%8x][%8x][%x]\n",type,g_loader_db.u32_flash_addr,g_loader_db.u32_flash_len,times);
	return RET_SUCCESS;
}

INT32 gacas_loader_db_get_chunk_len(UINT32 id, UINT32 *addr, UINT32 *len,UINT32 *offset)
{
	UINT16 i;
	
	if(g_loader_db.u16_chunk_num > 0 && g_loader_db.u16_chunk_use == 1)
	{		
		for(i = 0; i < g_loader_db.u16_chunk_num; i++)
		{
			if(id == g_loader_db.st_chunk[i].id)
			{
				*addr 	= g_loader_db.st_chunk[i].addr + CHUNK_HEADER_SIZE;
				*len 	= g_loader_db.st_chunk[i].len - CHUNK_HEADER_SIZE + CHUNK_NAME;
				*offset 	= g_loader_db.st_chunk[i].offset;
				LOADER_DB_PRINTF("[LD]gacas_loader_db_get_chunk_len-------[%8x][%8x][%8x][%8x]\n",g_loader_db.st_chunk[i].id,*addr,*len,*offset);
				return RET_SUCCESS;
			}
		}			
	}
	return RET_FAILURE;
}

void gacas_loader_db_set_index(UINT32 index)
{
	LOADER_DB_PRINTF("[LD]gacas_loader_db_set_index-------[%x][%x]\n",g_loader_db.u32_loader_index,index);
	if(g_loader_db.u32_loader_index != index)
	{
		g_loader_db.u32_loader_index = index;
		gacas_loader_db_save();
	}
}

INT32 gacas_loader_db_init(void)
{
	UINT32 chid = 0;
	UINT32 block_addr = 0;
	UINT32 block_len = 0;
	UINT32 data_len = 0;
	RET_CODE ret = RET_FAILURE;
	struct sto_device *sto_dev = NULL;
	UINT32 boot_total_area_len = 0;
	UINT16 i;
	GACAS_LOADER_DB_T t_loader_db;

	gacas_loader_chip_id_init();
#ifdef GOS_CONFIG_BOOT
	LOADER_DB_PRINTF("[LD]*******************************BOOT\n");
	LOADER_DB_PRINTF("[LD]*******************************BOOT\n");
	LOADER_DB_PRINTF("[LD]*******************************BOOT\n");
	LOADER_DB_PRINTF("[LD]gacas_loader_db_init-------BOOT\n");
#elif defined (GOS_CONFIG_APP)
	LOADER_DB_PRINTF("[LD]-------------------------------APP\n");
	LOADER_DB_PRINTF("[LD]-------------------------------APP\n");
	LOADER_DB_PRINTF("[LD]-------------------------------APP\n");
	LOADER_DB_PRINTF("[LD]gacas_loader_db_init-------APP\n");
#else
	LOADER_DB_PRINTF("[LD]~~~~~~~~~~~~~~~~~~~~~~~~~~UPG\n");
	LOADER_DB_PRINTF("[LD]~~~~~~~~~~~~~~~~~~~~~~~~~~UPG\n");
	LOADER_DB_PRINTF("[LD]~~~~~~~~~~~~~~~~~~~~~~~~~~UPG\n");
	LOADER_DB_PRINTF("[LD]gacas_loader_db_init-------UPG\n");
#endif
	LDR_PRINT("[LD]: GACAS_LOADER_DB_T size:%d\n", sizeof(GACAS_LOADER_DB_T));
	LDR_PRINT("[LD]: GACAS_TUNER_SAT_PARA_T size:%d\n", sizeof(GACAS_TUNER_SAT_PARA_T));
	chid = CHUNK_ID_LOADER_DB;
	block_len = 0;
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == sto_dev)
	{
		LDR_PRINT("[LD]Error-- can't get sto device.\n");
		return RET_FAILURE;
	}	
	boot_total_area_len = 0;
	ret = sto_get_chunk_len(chid, &block_addr, &block_len);
	if ((RET_SUCCESS != ret) || (0 == block_len))
	{
		LDR_PRINT("[LD]Error--can't find chunk 0x%08x.\n",  chid);
		block_addr = GACAS_LOADER_DB_FLASH_ADDR;
	}
#ifdef GOS_CONFIG_APP
	else if(gos_default_info_is_ok == 0)//add by yuj for default config tool
	{
		data_len = sizeof(GOS_DEFAULT_INFO_T);
		MEMSET(&gos_default_info,0,data_len);
		if ((INT32)data_len == sto_get_data(sto_dev, (UINT8*)&gos_default_info, block_addr-0x40,(INT32)data_len))
		{
			UINT32 t_crc32;
			t_crc32 = gacas_aes_crc32((unsigned char *)&gos_default_info,data_len-4);
			if(t_crc32 == gos_default_info.crc32)
			{
				gos_default_info_is_ok = 1;
			}
			libc_printf("gos_default_info-----data_len[%X][%X]\n",sizeof(GOS_DEFAULT_INFO_T),data_len);
			libc_printf("gos_default_info-----crc32[%08X][%08X]\n",t_crc32,gos_default_info.crc32);
			libc_printf("gos_default_info----is ok[%d]\n",gos_default_info_is_ok);
			libc_printf("gos_default_info----water_mark_x[%d]\n",gos_default_info.water_mark_x);
			libc_printf("gos_default_info----water_mark_y[%d]\n",gos_default_info.water_mark_y);
			libc_printf("gos_default_info----scan_mode[%d]\n",gos_default_info.scan_mode);
			libc_printf("gos_default_info----order_mode[%d]\n",gos_default_info.order_mode);
			libc_printf("gos_default_info----time_area[%d]\n",gos_default_info.time_area);
			libc_printf("gos_default_info----water_apha[%02X]\n",gos_default_info.water_apha);
			libc_printf("default_home_tp--0--freq_kHz[%d]\n",gos_default_info.center_tp[0].u32_freq_kHz);
			libc_printf("default_home_tp--0--symb_kbps[%d]\n",gos_default_info.center_tp[0].u32_symb_kbps);
			libc_printf("default_home_tp--0--modulation[%d]\n",gos_default_info.center_tp[0].u32_modulation);
			libc_printf("default_home_tp--1--freq_kHz[%d]\n",gos_default_info.center_tp[1].u32_freq_kHz);
			libc_printf("default_home_tp--1--symb_kbps[%d]\n",gos_default_info.center_tp[1].u32_symb_kbps);
			libc_printf("default_home_tp--1--modulation[%d]\n",gos_default_info.center_tp[1].u32_modulation);
		}
	}
#endif
	LDR_PRINT("[LD]block_addr = 0x%08x, block_len = 0x%08x\n", block_addr, block_len);
	// Load the chunk header

	data_len = sizeof(GACAS_LOADER_DB_T);
	MEMSET(&g_loader_db, 0, data_len);
	for(i=0; i<GACAS_LOADER_DB_STORE_NUM; i++)
	{
		if ((INT32)data_len != sto_get_data(sto_dev, (UINT8*)&t_loader_db, block_addr,(INT32)data_len))
		{
			LDR_PRINT("[LD]Error-- load OTA SeeCode chunk header failed\n");
			break;
		}	
		LDR_PRINT("[LD]sto_get_data---[0x%08x][%d][0x%08x][0x%08x]\n", block_addr, data_len, t_loader_db.u32_flag_start, t_loader_db.u32_flag_end);
		if(t_loader_db.u32_flag_start == 0xffffffff)
		{
			g_loader_db_store_index = i;
            LDR_PRINT("g_loader_db_store_index=%d \n", g_loader_db_store_index);
			break;
		}
		else if(t_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && t_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
		{
			MEMCPY(&g_loader_db, &t_loader_db, data_len);
            LDR_PRINT("find GACAS_LOADER_DB_FLAG (i=%d) \n", i);
		}
		block_addr += (GACAS_FLASH_BLOCK_SIZE/GACAS_LOADER_DB_STORE_NUM);
	}
	if(i == GACAS_LOADER_DB_STORE_NUM)
	{
		g_loader_db_store_index = GACAS_LOADER_DB_STORE_NUM;
        LDR_PRINT("g_loader_db_store_index=%d \n", g_loader_db_store_index);
	}
	
	if(g_loader_db.u32_flag_start == GACAS_LOADER_DB_FLAG && g_loader_db.u32_flag_end == GACAS_LOADER_DB_FLAG)
	{	
		UINT32 t_crc32 = gacas_loader_crc32((UINT8*)&g_loader_db,data_len-8);
		if(g_loader_db.u32_crc32 != t_crc32)
		{
			LDR_PRINT("[LD]Error--crc32[%8x][%8x]\n", g_loader_db.u32_crc32, t_crc32);
		}
		else
		{
			LDR_PRINT("[LD]data-- OK\n");
		}
	}
	else
	{
		LDR_PRINT("[LD]INIT-- DB\n");
		g_loader_db.u32_flag_start		= GACAS_LOADER_DB_FLAG;
		g_loader_db.u32_flag_end		= GACAS_LOADER_DB_FLAG;
		g_loader_db.u32_loader_index 	= 0;
		g_loader_db.u8_type			= GACAS_LOADER_TYPE_BUTT;
		g_loader_db.u8_times			= 0;
		g_loader_db.u16_pid			= 7003;
		g_loader_db.u32_flash_addr		= DEFAULTE_FLASH_BURN_ADDR;
		g_loader_db.u32_flash_len		= DEFAULTE_FLASH_BURN_SIZE;
        
		g_loader_db.default_mode		= 0;
		g_loader_db.tv_mode			= TV_MODE_1080I_25;
		g_loader_db.tv_ratio			= TV_ASPECT_RATIO_AUTO;
		g_loader_db.display_mode		= DISPLAY_MODE_PANSCAN;
		g_loader_db.scart_out		= SCART_YUV;
#ifdef DEF_SAT_BY_GOS
		g_loader_db.st_sat.u32_freq_kHz		= 0;
		g_loader_db.st_sat.u32_symb_kbps	= 0;
		g_loader_db.st_sat.u32_low_LO		= 0;
        	g_loader_db.st_sat.u32_high_LO  		= 0;
#else
		g_loader_db.st_sat.frq			= 0;
		g_loader_db.st_sat.sym			= 0;
		g_loader_db.st_sat.lnb_high		= 0;
		g_loader_db.st_sat.lnb_low			= 0;
#endif
		g_loader_db.u16_chunk_use 		= 0;
		g_loader_db_store_index = 0xff;
	}

    LDR_PRINT("u16_chunk_use=%d \n", g_loader_db.u16_chunk_use);
	if(g_loader_db.u16_chunk_use == 0)
	{
		UINT32 start_addr = 0;
		UINT32 id=0;
		UINT32 mask=0;
		CHUNK_HEADER t_chh;
		
		memset(&g_loader_db.st_chunk[0],0,GACAS_MAX_CHUNK_NUM*sizeof(LOADER_CHUNK));
    		g_loader_db.u16_chunk_num	= sto_chunk_count(0,0);
		/* bootloader 已经在flash初始化的时候跳过了，所以计算时需要跳过 */
		g_loader_db.u16_chunk_num += 1;
        
		if(g_loader_db.u16_chunk_num > GACAS_MAX_CHUNK_NUM)
		{
			g_loader_db.u16_chunk_num = GACAS_MAX_CHUNK_NUM;
		}
		LDR_PRINT("chunk_num----[%d][%d][%d]\n",g_loader_db.u16_chunk_num,sizeof(c_default_chunk_id)/sizeof(GA_DEFAULT_CHUNK_INFO),GACAS_MAX_CHUNK_NUM);

        /* flash 块不完整的情况 */
		if(g_loader_db.u16_chunk_num < sizeof(c_default_chunk_id)/sizeof(GA_DEFAULT_CHUNK_INFO))
		{
			start_addr = 0;
			g_loader_db.u16_chunk_num = sizeof(c_default_chunk_id)/sizeof(GA_DEFAULT_CHUNK_INFO);
			for(i = 0; i < g_loader_db.u16_chunk_num; i++)
			{
				g_loader_db.st_chunk[i].id			= c_default_chunk_id[i].id;
				g_loader_db.st_chunk[i].offset		= c_default_chunk_id[i].offset;
				g_loader_db.st_chunk[i].crc32		= 0;
				g_loader_db.st_chunk[i].len		= c_default_chunk_id[i].offset;
				g_loader_db.st_chunk[i].sign_len	= 0;
				g_loader_db.st_chunk[i].ver 		= 0;
				g_loader_db.st_chunk[i].addr		= start_addr;
				g_loader_db.st_chunk[i].b_update 	= FALSE;	
				start_addr += g_loader_db.st_chunk[i].offset;
		        LDR_PRINT("default-file[%8x]--%d--[%d][%8x][%8x][%8x][%8x]\n",g_loader_db.st_chunk[i].id,i,g_loader_db.st_chunk[i].b_update,g_loader_db.st_chunk[i].addr,t_chh.offset,g_loader_db.st_chunk[i].len,g_loader_db.st_chunk[i].sign_len);
			}
		}
		else
		{
			start_addr = 0;
			for(i = 0; i < g_loader_db.u16_chunk_num; i++)
			{
				id = 0;
				mask = 0;

		                /* bootloader 由于没有chunk header，所以需要单独赋值 */
		                if (i == 0)
		                {
		        			g_loader_db.st_chunk[i].id		= GACAS_BOOT_CHUNK_ID;
		        			g_loader_db.st_chunk[i].offset	= GACAS_BOOT_CHUNK_OFFSET;
		        			g_loader_db.st_chunk[i].crc32	= 0x40232425;/* NCRC */;
		        			g_loader_db.st_chunk[i].len		= GACAS_BOOT_CHUNK_OFFSET-16;
		        			g_loader_db.st_chunk[i].sign_len	= 0;
		        			g_loader_db.st_chunk[i].ver 		= 0;
		        			g_loader_db.st_chunk[i].addr		= start_addr+boot_total_area_len;
		        			g_loader_db.st_chunk[i].b_update 	= FALSE;
		                            
		        	        t_chh.offset = GACAS_BOOT_CHUNK_OFFSET;
		                }
		                /* 其他各块需要读取flash实际内容 */
				else if((unsigned long)ERR_FAILUE != sto_chunk_goto(&id, mask, i))
				{
					if(sto_get_chunk_header(id, &t_chh) != 1)
					{
						break;
					}
					g_loader_db.st_chunk[i].id			= t_chh.id;
					g_loader_db.st_chunk[i].offset		= t_chh.offset;
					g_loader_db.st_chunk[i].crc32		= t_chh.crc;
					g_loader_db.st_chunk[i].len		= (t_chh.len+CHUNK_NAME);
					g_loader_db.st_chunk[i].sign_len	= (t_chh.len +CHUNK_NAME+ SIG_ALIGN_LEN - 1) / SIG_ALIGN_LEN * SIG_ALIGN_LEN + SIG_LEN;
					g_loader_db.st_chunk[i].ver 		= gacas_loader_str2uint32(t_chh.version, 8);
					g_loader_db.st_chunk[i].addr		= start_addr+boot_total_area_len;
					g_loader_db.st_chunk[i].b_update 	= FALSE;
					if(g_loader_db.st_chunk[i].offset == 0)
                    {
						g_loader_db.st_chunk[i].offset 	= g_loader_db.st_chunk[i].len;
				}
                }
                		start_addr += t_chh.offset;
                LDR_PRINT("chunk-file[%8x]----[%d][%8x][%8x][%8x][%8x]\n",t_chh.id,g_loader_db.st_chunk[i].b_update,g_loader_db.st_chunk[i].addr,t_chh.offset,g_loader_db.st_chunk[i].len,g_loader_db.st_chunk[i].sign_len);
			}	
		}
		if(g_loader_db.u16_chunk_num > 0)
		{
			g_loader_db.u16_chunk_use = 1;
		}
		g_loader_db_store_index = 0xff;
	}
	if(g_loader_db_store_index == 0xff)
	{
        LDR_PRINT("bf gacas_loader_db_save \n");
		gacas_loader_db_save();
	}

	LDR_PRINT("[LD]u32_flag--------	[%08x][%08x]\n",g_loader_db.u32_flag_start,g_loader_db.u32_flag_end);
	LDR_PRINT("[LD]u32_loader_index----	[%d]\n",g_loader_db.u32_loader_index);
	LDR_PRINT("[LD]u32_file_id--------	[%08x]\n",g_loader_db.u32_file_id);
	LDR_PRINT("[LD]u32_file_id_new----	[%08x]\n",g_loader_db.u32_file_id_new);
	LDR_PRINT("[LD]u8_type--------	[%02x]\n",g_loader_db.u8_type);
	LDR_PRINT("[LD]u8_times--------	[%d]\n",g_loader_db.u8_times);
	LDR_PRINT("[LD]u16_pid--------	[%d]\n",g_loader_db.u16_pid);
	LDR_PRINT("[LD]u32_flash_addr-	[%8x]\n",g_loader_db.u32_flash_addr);
	LDR_PRINT("[LD]u32_flash_len--	[%8x]\n",g_loader_db.u32_flash_len);
	LDR_PRINT("[LD]u32_key_up--		[%8x]\n",g_loader_db.u32_key_up);
	LDR_PRINT("[LD]u32_key_down--	[%8x]\n",g_loader_db.u32_key_down);
	LDR_PRINT("[LD]u32_key_left--		[%8x]\n",g_loader_db.u32_key_left);
	LDR_PRINT("[LD]u32_key_right--		[%8x]\n",g_loader_db.u32_key_right);
	LDR_PRINT("[LD]u32_smc_id--		[%8x]\n",g_loader_db.u32_smc_id);
	LDR_PRINT("[LD]u32_run_area--		[%8x]\n",g_loader_db.u32_run_area);
	LDR_PRINT("[LD]u8_nit_ver--		[%8x]\n",g_loader_db.u8_nit_ver);
	LDR_PRINT("[LD]u8_usb_log_out--	[%8x]\n",g_loader_db.u8_usb_log_out);
	LDR_PRINT("[LD]u32_crc32_upg--	[%8x]\n",g_loader_db.u32_crc32_upg);
	LDR_PRINT("[LD]default_mode--------	[%d]\n",g_loader_db.default_mode);
	LDR_PRINT("[LD]tv_mode--------	[%d]\n",g_loader_db.tv_mode);
	LDR_PRINT("[LD]tv_ratio--------	[%d]\n",g_loader_db.tv_ratio);
	LDR_PRINT("[LD]display_mode--------	[%d]\n",g_loader_db.display_mode);
	LDR_PRINT("[LD]scart_out--------	[%d]\n",g_loader_db.scart_out);
	LDR_PRINT("[LD]u32_sig_type--------	[%08x]\n",g_loader_db.u32_sig_type);
	if(g_loader_db.u32_sig_type == GACAS_TUNER_SIG_TYPE_CAB)
	{
		LDR_PRINT("[LD]st_cab-u32_freq_kHz-------	[%d]\n",g_loader_db.st_cab.u32_freq_kHz);
		LDR_PRINT("[LD]st_cab-u32_symb_kbps-------	[%d]\n",g_loader_db.st_cab.u32_symb_kbps);
		LDR_PRINT("[LD]st_cab-u32_modulation-------	[%d]\n",g_loader_db.st_cab.u32_modulation);
		LDR_PRINT("[LD]st_centrt_cab[0]-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_cab[0].u32_freq_kHz);
		LDR_PRINT("[LD]st_centrt_cab[0]-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_cab[0].u32_symb_kbps);
		LDR_PRINT("[LD]st_centrt_cab[0]-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_cab[0].u32_modulation);
		LDR_PRINT("[LD]st_centrt_cab[1]-u32_freq_kHz-------	[%d]\n",g_loader_db.st_centrt_cab[1].u32_freq_kHz);
		LDR_PRINT("[LD]st_centrt_cab[1]-u32_symb_kbps-------	[%d]\n",g_loader_db.st_centrt_cab[1].u32_symb_kbps);
		LDR_PRINT("[LD]st_centrt_cab[1]-u32_modulation-------	[%d]\n",g_loader_db.st_centrt_cab[1].u32_modulation);
	}
	else if(g_loader_db.u32_sig_type == GACAS_TUNER_SIG_TYPE_SAT)
	{
		#ifdef DEF_SAT_BY_GOS
		LDR_PRINT("[LD]st_sat-u32_freq_kHz-------	[%d]\n",g_loader_db.st_sat.u32_freq_kHz);
		LDR_PRINT("[LD]st_sat-u32_symb_kbps-------	[%d]\n",g_loader_db.st_sat.u32_symb_kbps);
		LDR_PRINT("[LD]st_sat-u32_modulation-------	[%d]\n",g_loader_db.st_sat.u32_polar);
		LDR_PRINT("[LD]st_sat-u32_low_LO-------	[%d]\n",g_loader_db.st_sat.u32_low_LO);
		LDR_PRINT("[LD]st_sat-u32_high_LO-------	[%d]\n",g_loader_db.st_sat.u32_high_LO);
		LDR_PRINT("[LD]st_sat-u32_switch_22K-------	[%d]\n",g_loader_db.st_sat.u32_switch_22K);
		LDR_PRINT("[LD]st_sat-u32_LNB_power-------	[%d]\n",g_loader_db.st_sat.u32_LNB_power);
		#else
		LDR_PRINT("[LD]st_sat-sat_id-------		[%d]\n",g_loader_db.st_sat.sat_id);
		LDR_PRINT("[LD]st_sat-lnb_high-------	[%d]\n",g_loader_db.st_sat.lnb_high);
		LDR_PRINT("[LD]st_sat-lnb_low-------		[%d]\n",g_loader_db.st_sat.lnb_low);
		LDR_PRINT("[LD]st_sat-lnb_power-------	[%d]\n",g_loader_db.st_sat.lnb_power);
		LDR_PRINT("[LD]st_sat-lnb_type-------	[%d]\n",g_loader_db.st_sat.lnb_type);
		LDR_PRINT("[LD]st_sat-frq-------			[%d]\n",g_loader_db.st_sat.frq);
		LDR_PRINT("[LD]st_sat-sym-------		[%d]\n",g_loader_db.st_sat.sym);		
		LDR_PRINT("[LD]st_sat-pol-------		[%d][%d]\n",g_loader_db.st_sat.pol,g_loader_db.st_sat.pol_tp);		
		LDR_PRINT("[LD]st_sat-k22-------		[%d]\n",g_loader_db.st_sat.k22);		
		LDR_PRINT("[LD]st_sat-v12-------		[%d]\n",g_loader_db.st_sat.v12);		
		LDR_PRINT("[LD]st_centrt_sat[0]-sat_id-------		[%d]\n",g_loader_db.st_centrt_sat[0].sat_id);
		LDR_PRINT("[LD]st_centrt_sat[0]-lnb_high-------	[%d]\n",g_loader_db.st_centrt_sat[0].lnb_high);
		LDR_PRINT("[LD]st_centrt_sat[0]-lnb_low-------	[%d]\n",g_loader_db.st_centrt_sat[0].lnb_low);
		LDR_PRINT("[LD]st_centrt_sat[0]-lnb_power-------	[%d]\n",g_loader_db.st_centrt_sat[0].lnb_power);
		LDR_PRINT("[LD]st_centrt_sat[0]-lnb_type-------	[%d]\n",g_loader_db.st_centrt_sat[0].lnb_type);
		LDR_PRINT("[LD]st_centrt_sat[0]-frq-------		[%d]\n",g_loader_db.st_centrt_sat[0].frq);
		LDR_PRINT("[LD]st_centrt_sat[0]-sym-------		[%d]\n",g_loader_db.st_centrt_sat[0].sym);		
		LDR_PRINT("[LD]st_centrt_sat[0]-pol-------		[%d][%d]\n",g_loader_db.st_centrt_sat[0].pol,g_loader_db.st_centrt_sat[0].pol_tp);		
		LDR_PRINT("[LD]st_centrt_sat[0]-k22-------		[%d]\n",g_loader_db.st_centrt_sat[0].k22);		
		LDR_PRINT("[LD]st_centrt_sat[0]-v12-------		[%d]\n",g_loader_db.st_centrt_sat[0].v12);		
		LDR_PRINT("[LD]st_centrt_sat[1]-sat_id-------		[%d]\n",g_loader_db.st_centrt_sat[1].sat_id);
		LDR_PRINT("[LD]st_centrt_sat[1]-lnb_high-------	[%d]\n",g_loader_db.st_centrt_sat[1].lnb_high);
		LDR_PRINT("[LD]st_centrt_sat[1]-lnb_low-------	[%d]\n",g_loader_db.st_centrt_sat[1].lnb_low);
		LDR_PRINT("[LD]st_centrt_sat[1]-lnb_power-------	[%d]\n",g_loader_db.st_centrt_sat[1].lnb_power);
		LDR_PRINT("[LD]st_centrt_sat[1]-lnb_type-------	[%d]\n",g_loader_db.st_centrt_sat[1].lnb_type);
		LDR_PRINT("[LD]st_centrt_sat[1]-frq-------		[%d]\n",g_loader_db.st_centrt_sat[1].frq);
		LDR_PRINT("[LD]st_centrt_sat[1-sym-------		[%d]\n",g_loader_db.st_centrt_sat[1].sym);		
		LDR_PRINT("[LD]st_centrt_sat[1]-pol-------		[%d][%d]\n",g_loader_db.st_centrt_sat[1].pol,g_loader_db.st_centrt_sat[1].pol_tp);		
		LDR_PRINT("[LD]st_centrt_sat[1]-k22-------		[%d]\n",g_loader_db.st_centrt_sat[1].k22);		
		LDR_PRINT("[LD]st_centrt_sat[1]-v12-------		[%d]\n",g_loader_db.st_centrt_sat[1].v12);		
		#endif
	}
	else if(g_loader_db.u32_sig_type == GACAS_TUNER_SIG_TYPE_TER)
	{
		LDR_PRINT("[LD]st_ter-u32_freq_kHz-------	[%d]\n",g_loader_db.st_ter.u32_freq_kHz);
		LDR_PRINT("[LD]st_ter-u32_band_width_kHz-------	[%d]\n",g_loader_db.st_ter.u32_band_width_kHz);
		LDR_PRINT("[LD]st_ter-u32_modulation-------	[%d]\n",g_loader_db.st_ter.u32_modulation);
		LDR_PRINT("[LD]st_centrt_ter[0]-u32_freq_kHz-------		[%d]\n",g_loader_db.st_centrt_ter[0].u32_freq_kHz);
		LDR_PRINT("[LD]st_centrt_ter[0]-u32_band_width_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[0].u32_band_width_kHz);
		LDR_PRINT("[LD]st_centrt_ter[0]-u32_modulation-------		[%d]\n",g_loader_db.st_centrt_ter[0].u32_modulation);
		LDR_PRINT("[LD]st_centrt_ter[1]-u32_freq_kHz-------		[%d]\n",g_loader_db.st_centrt_ter[1].u32_freq_kHz);
		LDR_PRINT("[LD]st_centrt_ter[1]-u32_band_width_kHz-------	[%d]\n",g_loader_db.st_centrt_ter[1].u32_band_width_kHz);
		LDR_PRINT("[LD]st_centrt_ter[1]-u32_modulation-------		[%d]\n",g_loader_db.st_centrt_ter[1].u32_modulation); // LOADER_DB_PRINTF
	}	
	if(g_loader_db.u16_chunk_num>0 && g_loader_db.u16_chunk_use == 1)
	{
		for(i = 0; i < g_loader_db.u16_chunk_num; i++)
		{
			if(g_loader_db.st_chunk[i].id == CHUNK_ID_LOADER)
			{
				g_loader_db_ota_start_addr = g_loader_db.st_chunk[i].addr;
			}
			LOADER_DB_PRINTF("[LD]chunk[%2d][%8x]----[%d][%8x][%8x][%8x][%8x][%d]\n",i,g_loader_db.st_chunk[i].id,g_loader_db.st_chunk[i].b_update,g_loader_db.st_chunk[i].addr,g_loader_db.st_chunk[i].offset,g_loader_db.st_chunk[i].len,g_loader_db.st_chunk[i].sign_len,g_loader_db.st_chunk[i].b_update);
		}			
	}	
	return RET_SUCCESS;
}

//add by yuj 20171206
RET_CODE gacas_loader_convert_be_to_le(UINT8 *src, UINT8 *dest, UINT32 len)
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
INT32 gacas_loader_flash_erase_block(UINT32 flash_addr,UINT32 flash_len)
{
	struct sto_device *flash_dev = NULL;
	UINT32 offset_addr = 0;
	UINT32 param[2];
		
	LOADER_DB_PRINTF("[LD]---flash_wirte---[%08x][%08x]\n", flash_addr,flash_len);

	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == flash_dev)
	{
		LOADER_DB_PRINTF("[LD]------ERROR---flash_dev==NULL\n");
		return RET_FAILURE;
	}
	if((flash_addr % GACAS_FLASH_BLOCK_SIZE) != 0)
	{
		LOADER_DB_PRINTF("[LD]------ERROR---flash_addr\n");
		return RET_FAILURE;
	}
	for(offset_addr=0;offset_addr<flash_len;offset_addr+= GACAS_FLASH_BLOCK_SIZE)
	{
		param[0] = flash_addr+offset_addr;
		param[1] = GACAS_FLASH_BLOCK_SIZE >> 10;
		if (RET_SUCCESS == sto_io_control(flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param))
		{
			LOADER_DB_PRINTF("[LD]---ERASE---[%8x]OK\n", flash_addr+offset_addr);
		}
		else
		{
			LOADER_DB_PRINTF("[LD]---ERASE---[%8x]FAILURE\n", flash_addr+offset_addr);
			return RET_FAILURE;
		}
	}
	return RET_SUCCESS;
}

INT32 gacas_loader_flash_recover_block(UINT32 read_addr,UINT32 read_len,UINT32 write_addr)
{
	struct sto_device *flash_dev = NULL;
	UINT32 flash_addr = 0;
	UINT8 *flash_data = NULL;
	UINT32 offset_addr = 0;
	UINT32 param[2];
		
	LOADER_DB_PRINTF("[LD]---recover_block---[%08x][%08x][%08x]\n", read_addr,write_addr,read_len);
	flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == flash_dev)
	{
		LOADER_DB_PRINTF("[LD]------ERROR---flash_dev==NULL\n");
		return RET_FAILURE;
	}
	if((write_addr % GACAS_FLASH_BLOCK_SIZE) != 0 || (read_addr % GACAS_FLASH_BLOCK_SIZE) != 0)
	{
		LOADER_DB_PRINTF("[LD]------ERROR---flash_addr\n");
		return RET_FAILURE;
	}
	flash_data = MALLOC(GACAS_FLASH_BLOCK_SIZE);
	if(flash_data == NULL)
	{
		LOADER_DB_PRINTF("[LD]------ERROR---flash_data == NULL\n");
		return RET_FAILURE;
	}		
	for(offset_addr=0;offset_addr<read_len;offset_addr+= GACAS_FLASH_BLOCK_SIZE)
	{
		flash_addr = read_addr+offset_addr;
		LOADER_DB_PRINTF("[RB]---READ---[%8x][%8x]", flash_addr,GACAS_FLASH_BLOCK_SIZE);
		if ((INT32)flash_addr != sto_lseek(flash_dev, (INT32)flash_addr, STO_LSEEK_SET))
		{
			LOADER_DB_PRINTF("-FAILURE1\n");
			return RET_FAILURE;
		}	
		if (GACAS_FLASH_BLOCK_SIZE!= sto_read(flash_dev, flash_data, GACAS_FLASH_BLOCK_SIZE))
		{
			LOADER_DB_PRINTF("-FAILURE2\n");
			return RET_FAILURE;
		}	
		param[0] = write_addr+offset_addr;
		param[1] = GACAS_FLASH_BLOCK_SIZE >> 10;
		LOADER_DB_PRINTF("--ERASE--[%8x]", write_addr+offset_addr);
		if (RET_SUCCESS == sto_io_control(flash_dev, STO_DRIVER_SECTOR_ERASE_EXT, (UINT32)param))
		{
			flash_addr = write_addr+offset_addr;
			LOADER_DB_PRINTF("---WRITE---");
			if ((INT32)flash_addr != sto_lseek(flash_dev, (INT32)flash_addr, STO_LSEEK_SET))
			{
				LOADER_DB_PRINTF("-FAILURE3\n");
				return RET_FAILURE;
			}	
			if (GACAS_FLASH_BLOCK_SIZE!= sto_write(flash_dev, flash_data, GACAS_FLASH_BLOCK_SIZE))
			{
				LOADER_DB_PRINTF("-FAILURE4\n");
				return RET_FAILURE;
			}	
			LOADER_DB_PRINTF("-OK\n");
		}
		else
		{
			LOADER_DB_PRINTF("-FAILURE5\n");
			return RET_FAILURE;
		}
	}
	return RET_SUCCESS;
}

UINT8 gacas_loader_find(void)
{
	UINT32 pointer 	= 0;
	UINT32 tmp_data = 0;
	UINT32 data_id 	= 0;
	UINT32 data_len 	= 0;
	UINT32 data_off 	= 0;
	UINT32 data_crc 	= 0;
	UINT32 end_flag 	= 0;
	//UINT8 *buffer 	= NULL;
	//UINT32 crc 		= 0;
	//UINT8 b_find_loader 	= 0;
	UINT8 b_loader 	= 0;
	UINT8 b_loader_see = 0;
	UINT8 b_loader_back 		= 0;
	UINT8 b_loader_see_back 	= 0;
	UINT32 loader1_addr 	= 0;
	UINT32 loader1_offset 	= 0;
	UINT32 see1_addr 		= 0;
	UINT32 see1_offset 	= 0;
	UINT32 loader2_addr 	= 0;
	UINT32 loader2_offset 	= 0;
	UINT32 see2_addr 		= 0;
	UINT32 see2_offset 	= 0;
	
	struct sto_device *sto_dev = NULL;

	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (NULL == sto_dev)
	{
		//LOADER_DB_PRINTF("[BL]Error : %s, can't get sto device.\n", __FUNCTION__);
		return 0;
	}
	pointer = 0x80000;
	while ((INT32)pointer < 0x300000)
	{
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_ID, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_id, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_LENGTH, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_len, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_OFFSET, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_off, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_CRC, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_crc, 4);

		//LOADER_DB_PRINTF("[LD]-1-[%8X][%8X][%8X][%8X][%8X]\n", pointer, data_id, data_len, data_off, data_crc);

		if ((0 != data_len) && (data_len < data_off) && (CHUNK_ID_LOADER == data_id))
		{
			b_loader 		= 1;
			loader1_addr 	= pointer;
			loader1_offset	= data_off;
			if(b_loader_see == 1)
			{
				LOADER_DB_PRINTF("[LD]-loader-1-[%8X][%8X][%8X][%8X]\n", loader1_addr, loader1_offset, see1_addr, see1_offset);
				break;
			}
		}
		if ((0 != data_len) && (data_len < data_off) && (CHUNK_ID_LOADER_SEE== data_id))
		{
			b_loader_see 	= 1;
			see1_addr 	= pointer;
			see1_offset	= data_off;
			if(b_loader == 1)
			{
				LOADER_DB_PRINTF("[LD]-loader-11-[%8X][%8X][%8X][%8X]\n", loader1_addr, loader1_offset, see1_addr, see1_offset);
				break;
			}
		}
		pointer += GACAS_FLASH_BLOCK_SIZE;
   	}
	pointer = 0x680000;
	while ((INT32)pointer < sto_dev->totol_size)
	{
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_ID, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_id, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_LENGTH, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_len, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_OFFSET, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_off, 4);
		tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&tmp_data, (UINT32)pointer + CHUNK_CRC, 4);
		gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&data_crc, 4);

		//LOADER_DB_PRINTF("[LD]-2-[%8X][%8X][%8X][%8X][%8X]\n", pointer, data_id, data_len, data_off, data_crc);

		if ((0 != data_len) && (data_len < data_off) && (CHUNK_ID_LOADER == data_id))
		{
			b_loader_back 	= 1;
			loader2_addr 	= pointer;
			loader2_offset	= data_off;
			if(b_loader_see_back == 1)
			{
				LOADER_DB_PRINTF("[LD]-loader-2-[%8X][%8X][%8X][%8X]\n", loader2_addr, loader2_offset, see2_addr, see2_offset);
				break;
			}
		}
		else if ((0 != data_len) && (data_len < data_off) && (CHUNK_ID_LOADER_SEE == data_id))
		{
			b_loader_see_back = 1;
			see2_addr 	= pointer;
			see2_offset	= data_off;
			if(b_loader_back == 1)
			{
				LOADER_DB_PRINTF("[LD]-loader-22-[%8X][%8X][%8X][%8X]\n", loader2_addr, loader2_offset, see2_addr, see2_offset);
				break;
			}
		}
		pointer += GACAS_FLASH_BLOCK_SIZE;
   	}		
	LOADER_DB_PRINTF("[LD]Find UPG LOADER1--- 	[%8x][%8x][%8x][%8x]\n", loader1_addr,loader1_offset,see1_addr,see1_offset);
	LOADER_DB_PRINTF("[LD]Find UPG LOADER2--- 	[%8x][%8x][%8x][%8x]\n", loader2_addr,loader2_offset,see2_addr,see2_offset);
	if(b_loader_back == 1 && b_loader_see_back == 1 )//rewrite
	{
		//tmp_data = 0;
		sto_get_data(sto_dev, (UINT8 *)&end_flag, (UINT32)loader2_addr + loader2_offset-4, 4);
		//gacas_loader_convert_be_to_le((UINT8 *)&tmp_data, (UINT8 *)&end_flag, 4);
		LOADER_DB_PRINTF("[LD]Find UPG LOADER2--- end_flag[%8x]\n", end_flag);
		if(end_flag == 0x87654321)
		{
			if(loader1_addr == 0)
				loader1_addr 	= 0x100000;
			if(see1_addr == 0)
				see1_addr 	= 0xA0000;
			gacas_loader_flash_unlock(0,0x800000);
			gacas_loader_flash_recover_block(see2_addr,see2_offset,see1_addr);	
			gacas_loader_flash_recover_block(loader2_addr,loader2_offset,loader1_addr);	
			gacas_loader_flash_erase_block(see2_addr,see2_offset);
			gacas_loader_flash_erase_block(loader2_addr,loader2_offset);
		}
	}
	return 0;
}

void gacas_loader_get_boot_len(UINT32 *p_boot_len)
{
//	UINT32 id=0;
//	UINT32 mask=0;
//	CHUNK_HEADER t_chh;
//	UINT8 chunk_num,i;

	*p_boot_len =  0x80000;
}

//add by yuj 20170823
INT32 gacas_loader_flash_lock(UINT32 addr,UINT32 len)
{
	struct sto_device *flash_dev = (struct sto_device *) dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	UINT32  lock = 0;

	//gacas_loader_find();
	libc_printf("[F_LOCK]gacas_boot_flash_lock--[%8x][%8x]\n",addr,len);	
	if(flash_dev == NULL)
	{
		libc_printf("[F_LOCK]Can't find FLASH device!\n");
		return  ERR_FAILURE;
	}
	if(len == 0)
	{
		libc_printf("[F_LOCK](len == 0)----fail!\n");
		return  ERR_FAILURE;
	}
	
	if(sto_open(flash_dev) != SUCCESS)
	{
		libc_printf("[F_LOCK]Can't open FLASH device!\n");
		return  ERR_FAILURE;
	}
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x20000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x20000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x40000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x40000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x80000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x80000);	
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x100000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x100000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x200000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x200000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x400000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x400000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x800000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x800000);
	if(SUCCESS != sto_is_lock(flash_dev, addr, len, &lock))
	{
		libc_printf("[F_LOCK]sto_is_lock--0--fail!\n");
	}

	if(0 == lock)  //0->unlock
	{
		if((INT32)len != sto_lock(flash_dev, addr, (INT32)len))  // lock Nor
		{
			libc_printf("[F_LOCK]sto_lock-----fail!\n");
		}
		if(SUCCESS != sto_is_lock(flash_dev, addr, len, &lock))
		{
			libc_printf("[F_LOCK]sto_is_lock--1--fail!\n");
		}
		if(0 != lock)  // ->unlock
		{
			libc_printf("[F_LOCK]------OK[%8x][%8x]\n",addr,len);	
		}
		else
		{
			libc_printf("[F_LOCK]----FAIL[%8x][%8x]\n",addr,len);				
		}
	}
	else
	{
		libc_printf("[F_LOCK]----has locked[%8x][%8x]\n",addr,len);			
	}
	return SUCCESS;
}

INT32 gacas_loader_flash_unlock(UINT32 addr,UINT32 len)
{
	struct sto_device *flash_dev = (struct sto_device *) dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	UINT32  lock = 0;

	libc_printf("[F_LOCK]gacas_loader_flash_unlock--[%8x][%8x]\n",addr,len);	
	if(flash_dev == NULL)
	{
		libc_printf("[F_UNLOCK]Can't find FLASH device!\n");
		return  ERR_FAILURE;
	}
	if(len == 0)
	{
		libc_printf("[F_UNLOCK](len == 0)----fail!\n");
		return  ERR_FAILURE;
	}
	if(sto_open(flash_dev) != SUCCESS)
	{
		libc_printf("[F_UNLOCK]Can't open FLASH device!\n");
		return  ERR_FAILURE;
	}
#if 1
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x20000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x20000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x40000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x40000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x80000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x80000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x100000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x100000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x200000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x200000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x400000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x400000);
	if(SUCCESS == sto_is_lock(flash_dev, 0, 0x800000, &lock) && lock == 1)
		sto_unlock(flash_dev, 0, 0x800000);
#else
	if(SUCCESS != sto_is_lock(flash_dev, addr, len, &lock))
	{
		libc_printf("[F_UNLOCK]sto_is_lock--0--fail!\n");
	}

	if(0 == lock)  //0->unlock
	{
		libc_printf("[F_UNLOCK]----has unlocked[%8x][%8x]\n",addr,len);			
	}
	else
	{
		if(SUCCESS != sto_unlock(flash_dev, addr, len))  // lock Nor
		{
			libc_printf("[F_UNLOCK]sto_lock-----fail!\n");
		}
		if(SUCCESS != sto_is_lock(flash_dev, addr, len, &lock))
		{
			libc_printf("[F_UNLOCK]sto_is_lock--1--fail!\n");
		}
		if(0 == lock)  // ->unlock
		{
			libc_printf("[F_UNLOCK]-----OK[%8x][%8x]\n",addr,len);	
		}
		else
		{
			libc_printf("[F_UNLOCK]----FAIL[%8x][%8x]\n",addr,len);				
		}
	}
#endif
	return SUCCESS;
}
INT32 gacas_loader_check_center_tp_cab(UINT8 tp_index)
{
	if(g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz== 0 ||g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz == 0)
		return -1;
	return 0;
}
INT32 gacas_loader_check_center_tp_is_same(void)//add by yuj for default config tool
{
	if(g_loader_db.st_centrt_cab[0].u32_freq_kHz != g_loader_db.st_centrt_cab[1].u32_freq_kHz)
		return -1;
	if(g_loader_db.st_centrt_cab[0].u32_symb_kbps != g_loader_db.st_centrt_cab[1].u32_symb_kbps)
		return -1;
	if(g_loader_db.st_centrt_cab[0].u32_modulation != g_loader_db.st_centrt_cab[1].u32_modulation)
		return -1;
	return 0;
}

INT32 gacas_loader_get_centrt_tp_cab(UINT8 tp_index, UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod)
{
	if(tp_index >= 2)
		return -1;
	*p_freq 		= g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz;
	*p_symbol 	= g_loader_db.st_centrt_cab[tp_index].u32_symb_kbps;
	*p_mod 		= g_loader_db.st_centrt_cab[tp_index].u32_modulation;
	return 0;
}
INT32 gacas_loader_set_centrt_tp_cab(UINT8 tp_index, UINT32 freq,UINT32 symbol,UINT32 mod)
{
	if(tp_index >= 2)
	return -1;
	if(freq != g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz
	||symbol != g_loader_db.st_centrt_cab[tp_index].u32_symb_kbps
	||mod != g_loader_db.st_centrt_cab[tp_index].u32_modulation)
	{
		g_loader_db.st_centrt_cab[tp_index].u32_freq_kHz = freq;
		g_loader_db.st_centrt_cab[tp_index].u32_symb_kbps = symbol;
		g_loader_db.st_centrt_cab[tp_index].u32_modulation = mod;
		gacas_loader_db_save();
	}
	return 0;	
}
//add by yuj for default config tool -s
#ifdef GOS_CONFIG_APP
INT32 gacas_loader_check_center_tp_is_home_freq(void)
{
	if(gos_default_info_is_ok == 1)
	{
		if(g_loader_db.st_centrt_cab[0].u32_freq_kHz 	!= gos_default_info.center_tp[0].u32_freq_kHz)
			return -1;
		if(g_loader_db.st_centrt_cab[0].u32_symb_kbps 	!= gos_default_info.center_tp[0].u32_symb_kbps)
			return -1;
		if(g_loader_db.st_centrt_cab[0].u32_modulation 	!= gos_default_info.center_tp[0].u32_modulation)
			return -1;	
		return 0;
	}
	if(g_loader_db.st_centrt_cab[0].u32_freq_kHz 	!= HOME_FREQUENCY)
		return -1;
	if(g_loader_db.st_centrt_cab[0].u32_symb_kbps 	!= HOME_SYMBOL_RATE)
		return -1;
	if(g_loader_db.st_centrt_cab[0].u32_modulation 	!= HOME_QAM)
		return -1;
	return 0;
}
void gacas_loader_reset_home_freq(void)
{
	gacas_loader_db_set_ota_cab(DEFAULT_OTA_PID, HOME_FREQUENCY, HOME_SYMBOL_RATE, HOME_QAM);
	if(gos_default_info_is_ok == 1)
	{
		libc_printf("%s()--B0--freq_kHz[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_freq_kHz);
		libc_printf("%s()--B0--symb_kbps[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_symb_kbps);
		libc_printf("%s()--B0--modulation[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_modulation);
		libc_printf("%s()--B1--freq_kHz[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_freq_kHz);
		libc_printf("%s()--B1--symb_kbps[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_symb_kbps);
		libc_printf("%s()--B1--modulation[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_modulation);
		gacas_loader_set_centrt_tp_cab(0, gos_default_info.center_tp[0].u32_freq_kHz, gos_default_info.center_tp[0].u32_symb_kbps, gos_default_info.center_tp[0].u32_modulation);
		gacas_loader_set_centrt_tp_cab(1, gos_default_info.center_tp[1].u32_freq_kHz, gos_default_info.center_tp[1].u32_symb_kbps, gos_default_info.center_tp[1].u32_modulation);
		return;
	}
	libc_printf("%s()--A0--freq_kHz[%d]\n",__FUNCTION__,HOME_FREQUENCY);
	libc_printf("%s()--A0--symb_kbps[%d]\n",__FUNCTION__,HOME_SYMBOL_RATE);
	libc_printf("%s()--A0--modulation[%d]\n",__FUNCTION__,HOME_QAM);
	libc_printf("%s()--A1--freq_kHz[%d]\n",__FUNCTION__,HOME_FREQUENCY2);
	libc_printf("%s()--A1--symb_kbps[%d]\n",__FUNCTION__,HOME_SYMBOL_RATE2);
	libc_printf("%s()--A1--modulation[%d]\n",__FUNCTION__,HOME_QAM2);
	gacas_loader_set_centrt_tp_cab(0, HOME_FREQUENCY, HOME_SYMBOL_RATE, HOME_QAM);
	gacas_loader_set_centrt_tp_cab(1, HOME_FREQUENCY2, HOME_SYMBOL_RATE2, HOME_QAM2);
}

INT32 gacas_loader_get_default_home_tp_cab(UINT8 tp_index, UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod)
{
	if(tp_index >= 2)
		return -1;
	if(gos_default_info_is_ok == 0)
	{
		if(tp_index == 0)
		{
			*p_freq 			= HOME_FREQUENCY;
			*p_symbol 		= HOME_SYMBOL_RATE;
			*p_mod 		= HOME_QAM;
		}
		else
		{
			*p_freq 			= HOME_FREQUENCY2;
			*p_symbol 		= HOME_SYMBOL_RATE2;
			*p_mod 		= HOME_QAM2;
		}
	}
	else
	{
		*p_freq 			= gos_default_info.center_tp[tp_index].u32_freq_kHz;
		*p_symbol 		= gos_default_info.center_tp[tp_index].u32_symb_kbps;
		*p_mod 		= gos_default_info.center_tp[tp_index].u32_modulation;
	}
	return 0;	
}
UINT8 gacas_loader_get_default_scan_mode(void)
{
	if(gos_default_info_is_ok == 0)
		return NIT_SCAN_MODE;
	else
	{
		if(gos_default_info.scan_mode == 0)
			return NIT_SCAN_DEFAULT;
		else if(gos_default_info.scan_mode == 1)
			return NIT_SCAN_SDT;
		else
			return NIT_SCAN_DEFAULT;
	}
}
UINT8 gacas_loader_get_default_order_mode(void)
{
	if(gos_default_info_is_ok == 0)
		return GOS_PROG_ORDER_MODE;
	else
	{
		if(gos_default_info.order_mode == 0)
			return GOS_PROG_ORDER_BY_LCN;
		else if(gos_default_info.order_mode == 1)
			return GOS_PROG_ORDER_BY_SID;
		else if(gos_default_info.order_mode == 2)
			return GOS_PROG_ORDER_BY_FREQ;
		else
			return GOS_PROG_ORDER_BY_SID;
	}
}

UINT8 gacas_loader_get_default_time_area(void)
{
	if(gos_default_info_is_ok == 0)
	{
#ifdef GOS_LOCAL_TIME_OFFSET
		return GOS_LOCAL_TIME_OFFSET;
#else
		return 34;
#endif		
	}
	else
	{
		return gos_default_info.time_area;
	}
}

INT32 gacas_loader_get_default_water_mark_xy(UINT16 *p_x,UINT16 *p_y)
{
	if(gos_default_info_is_ok == 0)
	{
		return -1;
	}
	else
	{
		*p_x = gos_default_info.water_mark_x;
		*p_y = gos_default_info.water_mark_y;
	}
	return 0;
}
UINT8 gacas_loader_get_default_water_apha(void)
{
	if(gos_default_info_is_ok == 0)
	{
		return GOS_WATER_APHA;
	}
	else
	{
		return gos_default_info.water_apha;
	}
}
void gacas_loader_default_info_init(UINT8 *p_data,UINT16 len)
{
	if(sizeof(GOS_DEFAULT_INFO_T) == len && gos_default_info_is_ok == 0)
	{
		UINT32 t_crc32;
		MEMCPY(&gos_default_info,p_data,len);
		t_crc32 = gacas_aes_crc32((unsigned char *)p_data,len-4);
		libc_printf("%s()-----data_len[%X][%X]\n",__FUNCTION__,sizeof(GOS_DEFAULT_INFO_T),len);
		libc_printf("%s()-----crc32[%08X][%08X]\n",__FUNCTION__,t_crc32,gos_default_info.crc32);
		if(t_crc32 == gos_default_info.crc32)
		{
			gos_default_info_is_ok = 1;
			libc_printf("%s()----water_mark_x[%d]\n",__FUNCTION__,gos_default_info.water_mark_x);
			libc_printf("%s()----water_mark_y[%d]\n",__FUNCTION__,gos_default_info.water_mark_y);
			libc_printf("%s()----scan_mode[%d]\n",__FUNCTION__,gos_default_info.scan_mode);
			libc_printf("%s()----order_mode[%d]\n",__FUNCTION__,gos_default_info.order_mode);
			libc_printf("%s()----time_area[%d]\n",__FUNCTION__,gos_default_info.time_area);
			libc_printf("%s()----water_apha[%02X]\n",__FUNCTION__,gos_default_info.water_apha);
			libc_printf("%s()--0--freq_kHz[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_freq_kHz);
			libc_printf("%s()--0--symb_kbps[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_symb_kbps);
			libc_printf("%s()--0--modulation[%d]\n",__FUNCTION__,gos_default_info.center_tp[0].u32_modulation);
			libc_printf("%s()--1--freq_kHz[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_freq_kHz);
			libc_printf("%s()--1--symb_kbps[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_symb_kbps);
			libc_printf("%s()--1--modulation[%d]\n",__FUNCTION__,gos_default_info.center_tp[1].u32_modulation);
		}
		else
		{
			MEMSET(&gos_default_info,0,sizeof(GOS_DEFAULT_INFO_T));
			gos_default_info_is_ok = 0;
		}
		libc_printf("%s()----is ok[%d]\n",__FUNCTION__,gos_default_info_is_ok);
	}
}
#endif
//add by yuj for default config tool -e

