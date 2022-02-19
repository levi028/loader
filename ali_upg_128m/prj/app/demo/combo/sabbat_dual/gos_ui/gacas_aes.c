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
//#include <hld/trng/trng.h>
//#include <api/libga/lib_ga.h> 
#include <bus/otp/otp.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#else
#include "bootloader.h"
#endif
#include "gacas_loader_db.h"
#include "gacas_aes.h"

#define GACAS_AES_PRINTF(...) do{}while(0)
//#define GACAS_AES_PRINTF libc_printf
static unsigned int const c_crc32_table[]=
{
		0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
		0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
		0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
		0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
		0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
		0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
		0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
		0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
		0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
		0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
		0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
		0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
		0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
		0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
		0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
		0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
		0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
		0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
		0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
		0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
		0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
		0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
		0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
		0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
		0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
		0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
		0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
		0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
		0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
		0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
		0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
		0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
		0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
		0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
		0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
		0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
		0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
		0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
		0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
		0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
		0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
		0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
		0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
		0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
		0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
		0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
		0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
		0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
		0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
		0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
		0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
		0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
		0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
		0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
		0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
		0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
		0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
		0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
		0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
		0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
		0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
		0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
		0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
		0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

unsigned int gacas_aes_crc32(unsigned char *szData, unsigned int size)
{
	unsigned int cntByte;
	unsigned int crc32;
	unsigned char* pszData;

	pszData = (unsigned char*)szData;
	crc32 = 0xFFFFFFFF;

	for (cntByte = 0; cntByte < size; cntByte++)
	{
		crc32 = (crc32 << 8 ) ^ c_crc32_table[((crc32 >> 24) ^ *pszData++) & 0xFF];
	}
	return crc32;	
}

void gacas_aes_printf_bin_16(UINT8 *p_data)
{
	UINT16 i;
	for(i = 0; i < 16; i++)
		GACAS_AES_PRINTF("[%02x]", p_data[i]);
	GACAS_AES_PRINTF("\n");	
}
RET_CODE gacas_aes_cbc_with_ce_key(UINT8 *input, UINT8 *output, UINT32 length, UINT32 key_pos,UINT8 crypt_mode)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	p_aes_dev paesdev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	UINT32 device_id = 0;

	if(( NULL == input) || (NULL == output) || (0 == length))
	{
		return ret;
	}

	device_id = dsc_get_free_sub_device_id(AES);
	if(INVALID_DSC_SUB_DEV_ID == device_id )
	{
		return ret;
	}
	paesdev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
	if (INVALID_DSC_STREAM_ID  == stream_id)
	{
		dsc_set_sub_device_id_idle(AES, device_id);
		return ret;
	}

	MEMSET( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode 		= PURE_DATA_MODE;
	aes_param.key_from 		= KEY_FROM_CRYPTO;
	aes_param.key_mode 		= AES_128BITS_MODE ;
	aes_param.parity_mode 	= EVEN_PARITY_MODE;
	aes_param.residue_mode 	= RESIDUE_BLOCK_IS_NO_HANDLE;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC;
	aes_param.cbc_cts_enable = 0;
	ret = aes_ioctl ( paesdev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(RET_SUCCESS != ret)
	{
		dsc_set_stream_id_idle(stream_id);
		dsc_set_sub_device_id_idle(AES, device_id);
		return ret;
	}

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL;
	key_param.init_vector = NULL;
	key_param.key_length = 128;
	key_param.pid_len = 1;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id;
	key_param.pos = key_pos;
	ret = aes_ioctl (paesdev , IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(RET_SUCCESS != ret)
	{
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
	}

	if(DSC_ENCRYPT == crypt_mode)
	{
	ret = aes_encrypt( paesdev, stream_id, input, output, length );
	}
	else
	{
	ret = aes_decrypt( paesdev, stream_id, input, output, length );
	}

	aes_ioctl( paesdev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}
/*
    key_pos -> CE key pos
    ctr -> as the IV/Counter
    length should < 64MB
    crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE gacas_aes_ecb_with_ce_key(UINT8 *input, UINT8 *output, UINT32 length, UINT32 key_pos,UINT8 crypt_mode)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	p_aes_dev paesdev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	UINT32 device_id = 0;

	if(( NULL == input) || (NULL == output) || (0 == length))
	{
		return ret;
	}

	device_id = dsc_get_free_sub_device_id(AES);
	if(INVALID_DSC_SUB_DEV_ID == device_id )
	{
		return ret;
	}
	paesdev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
	if (INVALID_DSC_STREAM_ID  == stream_id)
	{
		dsc_set_sub_device_id_idle(AES, device_id);
		return ret;
	}

	MEMSET( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_ECB;
	aes_param.cbc_cts_enable = 0;
	ret = aes_ioctl ( paesdev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(RET_SUCCESS != ret)
	{
		dsc_set_stream_id_idle(stream_id);
		dsc_set_sub_device_id_idle(AES, device_id);
		return ret;
	}

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL;
	key_param.init_vector = NULL;
	key_param.key_length = 128;
	key_param.pid_len = 1;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id;
	key_param.pos = (key_pos&0xFF);
	key_param.kl_sel = (key_pos>>8)&0x0F;
	ret = aes_ioctl (paesdev , IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(RET_SUCCESS != ret)
	{
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
	}

	if(DSC_ENCRYPT == crypt_mode)
	{
	ret = aes_encrypt( paesdev, stream_id, input, output, length );
	}
	else
	{
	ret = aes_decrypt( paesdev, stream_id, input, output, length );
	}

	aes_ioctl( paesdev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

UINT32  gacas_aes_key_ladder_lv3(UINT32 root_index,UINT8 *pk,UINT8 *ck,UINT32 ce_crypt_select)
{
	p_ce_device pce_dev;
	RET_CODE ret = RET_FAILURE;
	UINT32 kl_level = 0;
	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
	CE_NLEVEL_PARAM nlevel_param;
	UINT32 t_key_pos = INVALID_CE_KEY_POS;
	pce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
	if(NULL == pce_dev)
	{
		return INVALID_CE_KEY_POS;
	}
	
	kl_level = sys_ic_get_kl_key_mode (root_index);
	GACAS_AES_PRINTF("gacas_aes_key_ladder_lv3--Key Ladder %d is %d level\n", root_index, kl_level);   
	{
		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		key_pos_param.pos = ALI_INVALID_CRYPTO_KEY_POS;
		key_pos_param.root 	= root_index;
		key_pos_param.number	= 1;
		key_pos_param.ce_key_level = kl_level; //Must same to the otp	
		ret = ce_ioctl(pce_dev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
		if (RET_SUCCESS != ret)
		{
			GACAS_AES_PRINTF("IO_CRYPT_FOUND_FREE_POS fail!\n");
			return INVALID_CE_KEY_POS;
		}

		t_key_pos = key_pos_param.pos;

		GACAS_AES_PRINTF("IO_CRYPT_FOUND_FREE_POS: 0x%x\n", t_key_pos);
	}
	MEMSET(&nlevel_param, 0, sizeof(CE_NLEVEL_PARAM));
	if(root_index == 0)
		nlevel_param.otp_addr = OTP_ADDESS_1;
	else if(root_index == 1)
		nlevel_param.otp_addr = OTP_ADDESS_2;
	else if(root_index == 2)
		nlevel_param.otp_addr = OTP_ADDESS_3;
	else if(root_index == 3)
		nlevel_param.otp_addr = OTP_ADDESS_4;
	else if(root_index == 4)
		nlevel_param.otp_addr = OTP_ADDESS_5;
	else if(root_index == 5)
		nlevel_param.otp_addr = OTP_ADDESS_6;
	else if(root_index == 6)
		nlevel_param.otp_addr = OTP_ADDESS_7;
	else if(root_index == 7)
		nlevel_param.otp_addr = OTP_ADDESS_8;	
	nlevel_param.kl_index 		= root_index;
	nlevel_param.algo 			= CE_SELECT_AES;
	nlevel_param.crypto_mode 	= CE_IS_DECRYPT;
	nlevel_param.pos 			= t_key_pos;
	nlevel_param.protecting_key_num = (kl_level-1);
	//copy first stage pk to protecting_key[0-15]
	MEMCPY(nlevel_param.protecting_key, pk, 32);
	MEMCPY(nlevel_param.content_key, ck, 16);
	ret = ce_ioctl(pce_dev, IO_CRYPT_GEN_NLEVEL_KEY, (UINT32)&nlevel_param);
	if (RET_SUCCESS != ret)
	{
		GACAS_AES_PRINTF("IO_CRYPT_GEN_NLEVEL_KEY fail!, ret = %d\n", ret);
		ce_ioctl(pce_dev, IO_CRYPT_POS_SET_IDLE, (UINT32)t_key_pos);
		return INVALID_CE_KEY_POS;
	}
	return t_key_pos;
}

RET_CODE gacas_aes_lv3_encypt(UINT8 skey_index,UINT32 data_len,UINT8*data_in,UINT8*data_out,UINT8*pk,UINT8*ck)
{
	UINT32 key_pos = INVALID_CE_KEY_POS;
	RET_CODE ret = RET_FAILURE;
	p_ce_device pCeDev0 = NULL;	
	UINT32 otp_addr= OTP_ADDESS_1;

	if(skey_index == 0)
		otp_addr = OTP_ADDESS_1;
	else if(skey_index == 1)
		otp_addr = OTP_ADDESS_2;
	else if(skey_index == 2)
		otp_addr = OTP_ADDESS_3;
	else if(skey_index == 3)
		otp_addr = OTP_ADDESS_4;
	else if(skey_index == 4)
		otp_addr = OTP_ADDESS_5;
	else if(skey_index == 5)
		otp_addr = OTP_ADDESS_6;
	else
		return -1;
	otp_addr = otp_addr;
#if 0
	AES_KEY_LADDER_BUF_PARAM aes_param;

	aes_param.key_ladder = sys_ic_get_kl_key_mode (skey_index);;
	aes_param.root_key_pos = skey_index;
	memcpy(aes_param.r,pk,32);
	memcpy(&aes_param.r[32],ck,32);
	aes_generate_key_with_multi_keyladder(&aes_param,&key_pos);
#else
	key_pos = gacas_aes_key_ladder_lv3(skey_index,pk,ck,DSC_DECRYPT);
#endif
	GACAS_AES_PRINTF("\n[AES_E]gacas_aes_key_ladder_lv3----[%x][%x]\n",otp_addr,key_pos);
	ret =  aes_crypt_puredata_with_ce_key(data_in,data_out,data_len,key_pos,DSC_ENCRYPT);
//	ret =  gacas_aes_ecb_with_ce_key(data_in,data_out,data_len,key_pos,DSC_ENCRYPT);
	pCeDev0 	= (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);		
	return ret;
}

RET_CODE gacas_aes_lv3_decypt(UINT8 skey_index,UINT32 data_len,UINT8*data_in,UINT8*data_out,UINT8*pk,UINT8*ck)
{
	UINT32 key_pos = INVALID_CE_KEY_POS;
	RET_CODE ret = RET_FAILURE;
	p_ce_device pCeDev0 = NULL;	
	UINT32 otp_addr= OTP_ADDESS_1;

	if(skey_index == 0)
		otp_addr = OTP_ADDESS_1;
	else if(skey_index == 1)
		otp_addr = OTP_ADDESS_2;
	else if(skey_index == 2)
		otp_addr = OTP_ADDESS_3;
	else if(skey_index == 3)
		otp_addr = OTP_ADDESS_4;
	else if(skey_index == 4)
		otp_addr = OTP_ADDESS_5;
	else if(skey_index == 5)
		otp_addr = OTP_ADDESS_6;
	else
		return -1;
	otp_addr = otp_addr;
#if 0
	AES_KEY_LADDER_BUF_PARAM aes_param;

	aes_param.key_ladder = sys_ic_get_kl_key_mode (skey_index);;
	aes_param.root_key_pos = skey_index;
	memcpy(aes_param.r,pk,32);
	memcpy(&aes_param.r[32],ck,32);
	aes_generate_key_with_multi_keyladder(&aes_param,&key_pos);
#else
	key_pos 	=  gacas_aes_key_ladder_lv3(skey_index,pk,ck,DSC_DECRYPT);
#endif
	GACAS_AES_PRINTF("\n[AES_D]gacas_aes_key_ladder_lv3----[%x][%x]\n",otp_addr,key_pos);
	ret =  aes_crypt_puredata_with_ce_key(data_in,data_out,data_len,key_pos,DSC_DECRYPT);
//	ret =  gacas_aes_ecb_with_ce_key(data_in,data_out,data_len,key_pos,DSC_DECRYPT);
	pCeDev0 	= (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);		
	return ret;
}
UINT8 const c_gscas_pk[32]	= {0x11,0x25,0x71,0xe9,0x53,0x54,0x99,0x20,0x51,0x55,0x7c,0x60,0x5f,0xbf,0x1f,0x65,0x1a,0xc1,0xb7,0x3d,0x53,0xd9,0x89,0x47,0xf6,0x85,0x18,0x12,0x7a,0xe3,0xf2,0x82};
UINT8 const c_gscas_ck[16] 	= {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x12}; 
#if 0 //ca_with_card
UINT8 const c_gacas_upg_key[16] 	= {0xa8, 0xbb, 0xcc, 0xa7, 0xb3,0xc1, 0x1b, 0x21, 0x35, 0x4e, 0x50, 0x6f, 0x73, 0x83, 0x98, 0xa8};
#else
UINT8 const c_gacas_upg_key[16] 	= {0xff,0xbc,0xbf,0x70,0x05,0x00,0xc3,0x76,0x0f,0x00,0xc7,0x7d,0x1a,0x00,0xca,0x84};
#endif

UINT8 const c_gacas_chunk_key[16] 	= {0xb7, 0xbb, 0xdc, 0xa7, 0xa3,0x51, 0x4b, 0x65, 0xa5, 0x4e, 0x10, 0xcf, 0x13, 0xa3, 0x38, 0x90};

RET_CODE gacas_aes_ecb_encypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	return aes_pure_ecb_crypt(key,data_in,data_out,data_len,DSC_ENCRYPT);
	//gacas_ase_soft_decrypt_data(key,data_in,data_out,data_len,DSC_ENCRYPT);
}
RET_CODE gacas_aes_ecb_decypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	return aes_pure_ecb_crypt(key,data_in,data_out,data_len,DSC_DECRYPT);
	//gacas_ase_soft_decrypt_data(key,data_in,data_out,data_len,DSC_DECRYPT);
}
RET_CODE gacas_aes_cbc_encypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	char iv[16];
	MEMSET(iv,0,sizeof(iv));
	return aes_pure_cbc_crypt(key,(UINT8*)iv,data_in,data_out,data_len,DSC_ENCRYPT);
}
RET_CODE gacas_aes_cbc_decypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	char iv[16];
	MEMSET(iv,0,sizeof(iv));
	return aes_pure_cbc_crypt(key,(UINT8*)iv,data_in,data_out,data_len,DSC_DECRYPT);
}
INT32 gacas_encrypt_chunk_by_otp(UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{	
	return gacas_aes_lv3_encypt(GACAS_OTP_KEY_INDEX,data_len,data_in,data_out,(UINT8*)c_gscas_pk,(UINT8*)c_gscas_ck);
}
INT32 gacas_decrypt_chunk_by_otp(UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{	
	return gacas_aes_lv3_decypt(GACAS_OTP_KEY_INDEX,data_len,data_in,data_out,(UINT8*)c_gscas_pk,(UINT8*)c_gscas_ck);
}
INT32 gacas_decrypt_chunk_data(UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{	
	INT32 ret = RET_FAILURE;
#if 0
	UINT8 key_pos = 0;
	UINT32 key_id = 0;
#define NEW_HEAD_ID             0xaead0000
#define NEW_HEAD_MASK           0xffff0000
	if((ALI_S3281==sys_ic_get_chip_id()))
	{
		key_id = (NEW_HEAD_ID&NEW_HEAD_MASK)|0xFE;//FIRST_FLASH_KEY;
		libc_printf("[AES]key_id --1--[%x]!\n",key_id);
	}
	else
	{
		key_id = 0x22DD0100;//DECRPT_KEY_ID;
		libc_printf("[AES]key_id --2--[%x]!\n",key_id);
	}
	if (decrypt_universal_key(&key_pos, key_id) < NUM_ZERO)
	{
		libc_printf("[AES]Decrypt universal key failed!\n");
		return RET_FAILURE;
	}
	ret = aes_cbc_decrypt_ram_chunk(key_pos, data_out, data_in, data_len);
	ce_ioctl((p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0), IO_CRYPT_POS_SET_IDLE, key_pos);
	libc_printf("[AES]gacas_decrypt_chunk_data----[%x][%8x][%d]\n",key_pos,data_len,ret);
	libc_printf("[AES]data_in--1:\n");
	gacas_aes_printf_bin_16(data_in);
	gacas_aes_printf_bin_16(&data_in[16]);
	libc_printf("[AES]data_out--1:\n");
	gacas_aes_printf_bin_16(data_out);
	gacas_aes_printf_bin_16(&data_out[16]);
#else
	GACAS_AES_PRINTF("[AES]gacas_decrypt_chunk_data------[%8x]:\n",data_len);
	ret = gacas_aes_cbc_decypt((UINT8*)c_gacas_chunk_key,data_in,data_out,data_len);
//	GACAS_AES_PRINTF("[AES]data_in--:\n");
//	gacas_aes_printf_bin_16(data_in);
//	gacas_aes_printf_bin_16(&data_in[16]);
//	GACAS_AES_PRINTF("[AES]data_out--:\n");
//	gacas_aes_printf_bin_16(data_out);
//	gacas_aes_printf_bin_16(&data_out[16]);
#endif
	return ret;
}

INT32 gacas_decrypt_upg_file(UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	INT32 ret = RET_FAILURE;

	GACAS_AES_PRINTF("[AES]gacas_decrypt_upg_file------[%8x]:\n",data_len);
	ret = gacas_aes_ecb_decypt((UINT8*)c_gacas_upg_key, data_in, data_out, data_len);
//	GACAS_AES_PRINTF("[AES]data_in--:\n");
//	gacas_aes_printf_bin_16(data_in);
//	gacas_aes_printf_bin_16(&data_in[16]);
//	GACAS_AES_PRINTF("[AES]data_out--:\n");
//	gacas_aes_printf_bin_16(data_out);
//	gacas_aes_printf_bin_16(&data_out[16]);
	return ret;
}
#if 0

/*************************************
pChipID	318766082	                12FFFC02
CSA2RK	"C235637491A271F625457FAACD290F26"	
CSA3RK	"05CADF0DABBF46E8AC67D026210FD3BA"	
R2RKey	"214A62DFAE9BB889406C56078D41E956"	
DSCKey	"FEF2FF25D2C838153E711BAD46EF6321"	
JTAGKey	"5854DEBB06AA1F8D0000000000000000"	
PVRKey	"F2D6AFDA3484D0FEED598BD44900D942"	
Resv1K	"B9B4A5A0263DEF91F2D157FDE9DCA562"	
Resv2K	"393629CA3A5FDEF35CD716B5F5C6F06E"	
Resv3K	"1632E72CF540BB19719BA6DB1D221A99"	
ChipID="f9fffc03"  
CAS2RK="b8d20b87b9c796dba7813848293f51ac"  
CAS3RK="40866c704e3800e378914ad0ed39f0ed"  
R2RKey="3dbd6c929826188bacce2da57321692c"  
PVRRK="25df792766f8da909b7f8c1f47bac607"  
DSCRK="51d46a795023c7e85601c29f60ac6bf8"  
JTAGKey="873a7c284efe11a1"  
ReservedKey1="b1dcc306b9dc108fe07352a0f42c989c"  
ReservedKey2="3c21b4e67a8dd543838209aea12f27c2"  
ReservedKey3="de41a98ee3726981cde2735d5838a3cd" 
ChipID="f9fffc01"  
CAS2RK="8eb008f4553b42ccb433da2af91cd17d"  
CAS3RK="e264b1e2b9805fb46d7e59ebe1663f75"  
R2RKey="51bf8eaef9b08d86003dfaae30d46d5f"  
PVRRK="494ce7fdea9819ad990ed9903a9884d3"  
DSCRK="0d89fac9b41b75906334942215024b24"  
JTAGKey="e9ba5a09c1fe2ebd"  
ReservedKey1="8d28990fc1ed9089efa782adc292ed79"  
ReservedKey2="b00fcdd73e50a2f69a6ebfa53083a6ad"  
ReservedKey3="020cec683043b6d19fb5225a7d617353" 
ChipID="f9fffc02"  
CAS2RK="00ecd17572d5ff29de43030dd55f1402"  
CAS3RK="9b595dd6a744ff2e094f326ccccaa7b6"  
R2RKey="f095415cbbee6b86fc76b9e02e2f971a"  
PVRRK="20363a823f50aae261ca0e0bf24b20ac"  
DSCRK="a120c15f0c747e6ec69976c8682a601d"  
JTAGKey="915e8a456d7e39c4"  
ReservedKey1="a81f83c562b6088510f9edc7273b1929"  
ReservedKey2="d9ae8c1aa0bead2005a9bd8c9811bcea"  
ReservedKey3="6e87b51d512d0d497390af8482ed5b88" 
************************************/
//pcCSA2RootKey=268b3986693a037d7bdc7d0e78164e5e
//pcCSA3RootKey=5d597dfcc17d7ea6244db72ca3efa72c
//pcJTAGKey=33234ecdbe479e6c0000000000000000
//pcR2RRootKey=99a777fc585b93efdc153e41aa02a96a
//pcDSCKey=1847ddf2de46d0984b61f31c2bc005dc
//pcPVRKey=9f098da456d5bbf53c16d05ecb328439
UINT8 gacas_pk[32]={
		0x11,0x25,0x71,0xe9,0x53,0x54,0x99,0x20,0x51,0x55,0x7c,0x60,0x5f,0xbf,0x1f,0x65,
		0x1a,0xc1,0xb7,0x3d,0x53,0xd9,0x89,0x47,0xf6,0x85,0x18,0x12,0x7a,0xe3,0xf2,0x82,};
UINT8 gacas_ck[16] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x12}; 
//CSA2RootKey
static UINT8 gacas_root_key0[] = "00ecd17572d5ff29de43030dd55f1402";
//PVRKey
static UINT8 gacas_root_key1[] = "20363a823f50aae261ca0e0bf24b20ac" ;
//R2RRootKey
static UINT8 gacas_root_key2[] ="f095415cbbee6b86fc76b9e02e2f971a";
//CSA3RootKey
static UINT8 gacas_root_key5[] = "9b595dd6a744ff2e094f326ccccaa7b6";
//extern signed int gacas_ase_soft_decrypt_data(unsigned char* p_key,unsigned char* p_in,unsigned char * p_out,unsigned int data_len,unsigned char type);
UINT8 *p_gacas_kdf_buff = NULL;
UINT8 *p_gacas_cw_e = NULL;
UINT8 *p_gacas_cw_d = NULL;
UINT8 *p_gacas_key1_d = NULL;
UINT8 *p_gacas_key2_d = NULL;
UINT8 *p_gacas_rs_key = NULL;
UINT8 *p_gacas_key1 = NULL; 
UINT8 *p_gacas_key2 = NULL; 

UINT32 gacas_aes_change_str_to_data(UINT8 *p_str,UINT8 *p_data,UINT32 max_len)
{
	UINT8 *p_t_str 	= p_str;
	UINT8 *p_t_data 	= p_data;
	UINT32 data_len	= 0;
	UINT8 data_l,data_h;

	while(p_t_str[0] != 0)
	{
		if(p_t_str[0] >= '0' && p_t_str[0] <= '9')
			data_h = (p_t_str[0]-'0');
		else if(p_t_str[0] >= 'a' && p_t_str[0] <= 'f')
			data_h = (p_t_str[0]-'a')+0xa;
		else if(p_t_str[0] >= 'A' && p_t_str[0] <= 'F')
			data_h = (p_t_str[0]-'A')+0xa;
		else
			break;
		p_t_str++;
		if(p_t_str[0] >= '0' && p_t_str[0] <= '9')
			data_l = (p_t_str[0]-'0');
		else if(p_t_str[0] >= 'a' && p_t_str[0] <= 'f')
			data_l = (p_t_str[0]-'a')+0xa;
		else if(p_t_str[0] >= 'A' && p_t_str[0] <= 'F')
			data_l = (p_t_str[0]-'A')+0xa;
		else
			break;		
		p_t_str++;
		p_t_data[0] = (data_h<<4)|data_l;
		p_t_data++;
		data_len++;
		if(max_len <= data_len)
			break;
	}
	return data_len;
}


void gacas_aes_lv3_test(UINT8 key_index)
{
	//UINT16 i;
	UINT8 input[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
	UINT8 key[16];
	UINT8 output[16];
	UINT8 output2[16];
	UINT8 pk1[16];
	UINT8 pk2[16];
	UINT8 ck[16];

	INT32 kl_level;
	
	kl_level = sys_ic_get_kl_key_mode (key_index);
	libc_printf("Key Ladder %d is %d level\n", key_index, kl_level);

	if(key_index == 0)
	{
		libc_printf("gacas_kdf_decypt_aes_lv3_test--0--CSA2RK\n");
		gacas_aes_lv3_encypt(0,16,input,output,gacas_pk,gacas_ck);	
		gacas_aes_lv3_decypt(0,16,output,output2,gacas_pk,gacas_ck);
	}
	else if(key_index == 1)
	{
		libc_printf("gacas_kdf_decypt_aes_lv3_test--1--PVRKey\n");
		gacas_aes_lv3_encypt(1,16,input,output,gacas_pk,gacas_ck);	
		gacas_aes_lv3_decypt(1,16,output,output2,gacas_pk,gacas_ck);
	}
	else if(key_index == 2)
	{
		libc_printf("gacas_kdf_decypt_aes_lv3_test--2--R2R-ROOT Key\n");
		gacas_aes_lv3_encypt(2,16,input,output,gacas_pk,gacas_ck);	
		gacas_aes_lv3_decypt(2,16,output,output2,gacas_pk,gacas_ck);
	}
	else  if(key_index == 5)
	{
		libc_printf("gacas_kdf_decypt_aes_lv3_test--5--CSA3RK\n");
		gacas_aes_lv3_encypt(5,16,input,output,gacas_pk,gacas_ck);	
		gacas_aes_lv3_decypt(5,16,output,output2,gacas_pk,gacas_ck);
	}
	else
	{
		return;
	}
	libc_printf("input-e: 	");
	gacas_aes_printf_bin_16(input);
	libc_printf("output-e: 	");
	gacas_aes_printf_bin_16(output);
	libc_printf("output-d: 	");
	gacas_aes_printf_bin_16(output2);
#if 1
	if(key_index == 0)
		gacas_aes_change_str_to_data(gacas_root_key0,key,16);
	else if(key_index == 1)
		gacas_aes_change_str_to_data(gacas_root_key1,key,16);
	else if(key_index == 2)
		gacas_aes_change_str_to_data(gacas_root_key2,key,16);
	else if(key_index == 5)
		gacas_aes_change_str_to_data(gacas_root_key5,key,16);
	gacas_aes_ecb_decypt(key,&gacas_pk[0],pk2,16);
	gacas_aes_ecb_decypt(pk2,&gacas_pk[16],pk1,16);
	gacas_aes_ecb_decypt(pk1,gacas_ck,ck,16);	
	gacas_aes_ecb_encypt(ck,input,output,16);
	
	gacas_aes_ecb_decypt(key,&gacas_pk[0],pk2,16);
	gacas_aes_ecb_decypt(pk2,&gacas_pk[16],pk1,16);
	gacas_aes_ecb_decypt(pk1,gacas_ck,ck,16);	
	gacas_aes_ecb_decypt(ck,output,output2,16);
	
	libc_printf("\n");
	libc_printf("key: 		");	
	gacas_aes_printf_bin_16(key);
	libc_printf("input-e: 	");
	gacas_aes_printf_bin_16(input);
	libc_printf("output-e: 	");
	gacas_aes_printf_bin_16(output);
	libc_printf("output-d: 	");
	gacas_aes_printf_bin_16(output2);
#endif
}
void gacas_aes_test(UINT8 skey_index)
{
	UINT16 i;
	UINT8 test_char[256];
	UINT8 test_char_e[256];
	UINT8 test_char_d[256];
	UINT8  pk[32]	;
	UINT8  ck[16]; 

	GACAS_AES_PRINTF("\n[AES_E]gacas_aes_test********START[%d]\n",skey_index);
	for(i=0;i<32;i++)
		pk[i] = i+1;
	for(i=0;i<16;i++)
		ck[i] = i+0x80;
	
	for(i=0;i<256;i++)
	{
		test_char[i] = i;
		test_char_e[i] = 0;
		test_char_d[i] = 0;
	}
	gacas_encypt_aes_lv3(skey_index,256,test_char,test_char_e,(UINT8*)pk,(UINT8*)ck);
	gacas_decypt_aes_lv3(skey_index,256,test_char_e,test_char_d,(UINT8*)pk,(UINT8*)ck);
	for(i=0;i<256;i++)
	{
		GACAS_AES_PRINTF("[%2X][%2X][%2X]\n", test_char[i], test_char_e[i], test_char_d[i]);		
	}
}
#endif

