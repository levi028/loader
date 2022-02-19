/*****************************************************************************
*	 Copyright (c) 2013 ALi Corp. All Rights Reserved
*	 This source is confidential and is ALi's proprietary information.
*	 This source is subject to ALi License Agreement, and shall not be
	 disclosed to unauthorized individual.
*	 File: crypto_remote.c
*
*	 Description: This file provides secure key ladder interface.
*
*	 THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	 PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/crypto/crypto.h>

#define CE_NPARA(x) ((HLD_CRYPTO_MODULE<<24)|(x<<16))

#define INVALID_CE_KEY_POS 0xff
#define MAGIC_CONST_8  8
#define MAGIC_CONST_100   100

#define offsetof(type, f) ((unsigned long) \
	((char *)&((type *)0)->f - (char *)(type *)0))

//#define CE_REMOTE_API_DEBUG
#ifdef CE_REMOTE_API_DEBUG
#ifdef CE_REMOTE_API_PRINTF
#undef CE_REMOTE_API_PRINTF
#endif
#ifdef CE_REMOTE_API_DUMP
#undef CE_REMOTE_API_DUMP
#endif
#define CE_REMOTE_API_PRINTF libc_printf
#define CE_REMOTE_API_DUMP(data,len) \
	do{ \
		int i, l=(len); \
		for(i=0; i<l; i++){ \
			CE_REMOTE_API_PRINTF("0x%x,",*(UINT8 *)((UINT32)data+i)); \
			if(0==((i+1)%16)) \
				CE_REMOTE_API_PRINTF("\n");\
		}\
	}while(0)
#else
#define CE_REMOTE_API_PRINTF(...)  do{}while(0)
#define CE_REMOTE_API_DUMP(...)  do{}while(0)
#endif

#ifdef DUAL_ENABLE


#include <modules.h>

enum HLD_CE_FUNC
{
	FUNC_CE_ATTACH = 0,
	FUNC_CE_SET_AKSV,
	FUNC_PATCH_HDMI,
	FUNC_CE_DETACH,
	FUNC_CE_GENERATE,
	FUNC_CE_LOAD,
	FUNC_CE_IOCTL,
	FUNC_CE_GENERATE_CW_KEY,
	FUNC_CE_GENERATE_SINGLE_LEVEL_KEY,

	FUNC_CE_GENERATE_HDCP_KEY,
};

#ifndef _HLD_CRYPTO_REMOTE

static UINT32 hld_ce_entry[] =
{
	(UINT32)ce_api_attach,
	(UINT32)hdmi_set_aksv,
	(UINT32)patch_write_bksv2hdmi,
	(UINT32)ce_api_detach,
	(UINT32)ce_key_generate_rpc,
	(UINT32)ce_key_load_rpc,
	(UINT32)ce_ioctl_rpc,
	(UINT32)ce_generate_cw_key_rpc,
	(UINT32)ce_generate_single_level_key_rpc,

	(UINT32)ce_generate_hdcp_key,
};

static UINT32 ce_m36_entry[] =
{
};

void hld_crypto_callee(UINT8 *msg)
{
	if(NULL == msg)
	{
		return;
	}
	os_hld_callee((UINT32)hld_ce_entry, msg);
}
void lld_crypto_m36f_callee( UINT8 *msg )
{
	if(NULL == msg)
	{
		return;
	}
	os_hld_callee((UINT32)ce_m36_entry, msg);
}

#endif	/*_HLD_CE_REMOTE*/

#ifdef _HLD_CRYPTO_REMOTE

static UINT32 ce_io_control[] =
{
	//desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,
	0,
};

static RET_CODE ce_api_attach_ex( void )
{
	jump_to_func(NULL, os_hld_caller, NULL, CE_NPARA(0) | FUNC_CE_ATTACH, NULL);
}

static RET_CODE ce_api_detach_ex( void )
{
	jump_to_func(NULL, os_hld_caller, NULL, CE_NPARA(0) | FUNC_CE_DETACH, NULL);
}

RET_CODE ce_api_attach( void )
{
	CE_DEVICE *p_ce_dev = (CE_DEVICE *)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
	if(!p_ce_dev)
	{
		return ce_api_attach_ex();
	}
	return ALI_CRYPTO_WARNING_DRIVER_ALREADY_INITIALIZED;
}

RET_CODE ce_api_detach( void )
{
	return ce_api_detach_ex();
}

void hdmi_set_aksv(void)
{
	jump_to_func(NULL, os_hld_caller, NULL, CE_NPARA(0) | FUNC_CE_SET_AKSV, NULL);
}

void patch_write_bksv2hdmi(UINT8 *data)
{
	UINT32 desc[] =
	{
		1, DESC_OUTPUT_STRU(0, 5), 1, DESC_P_PARA(0, 0, 0), 0, 0,
	};
	jump_to_func(NULL, os_hld_caller, data, CE_NPARA(1) | FUNC_PATCH_HDMI, desc);
}

RET_CODE ce_key_generate(p_ce_device pcedev, p_ce_data_info pce_data_info)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(CE_DATA_INFO)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, pcedev, CE_NPARA(2) | FUNC_CE_GENERATE, desc);
}

RET_CODE ce_key_load(p_ce_device pcedev, p_otp_param pce_opt_info)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(OTP_PARAM)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, pcedev, CE_NPARA(2) | FUNC_CE_LOAD, desc);
}


RET_CODE ce_ioctl(p_ce_device pcedev, UINT32 cmd, UINT32 param)
{
	UINT32 i = 0;
	UINT32 common_desc[sizeof(ce_io_control)] = {0};
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)ce_io_control;

	if(NULL == pcedev)
	{
		return ALI_CRYPTO_ERROR_INVALID_DEV;
	}
	for(i = 0; i < sizeof(ce_io_control) / sizeof(UINT32); i++)
	{
		desc[i] = b[i];
	}
	switch (cmd)
	{
		case IO_OTP_ROOT_KEY_GET:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(OTP_PARAM));
			break;

		case IO_CRYPT_DEBUG_GET_KEY:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_DEBUG_KEY_INFO));
			break;
		case IO_CRYPT_POS_IS_OCCUPY:
		{
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_POS_STS_PARAM));
		}
		break;
		case IO_CRYPT_POS_SET_USED:
		{
			desc = NULL;
		}
		break;
		case IO_CRYPT_POS_SET_IDLE:
		{
			desc = NULL;
		}
		break;
		case IO_CRYPT_FOUND_FREE_POS:
		{
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		}
		break;

		case IO_DECRYPT_PVR_USER_KEY:
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_PVR_KEY_PARAM));
		}
		break;

		case IO_CRYPT_GEN_NLEVEL_KEY:
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_NLEVEL_PARAM));
		}
		break;

		case IO_CRYPT_GEN_NLEVEL_KEY_ADVANCED:
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_NLEVEL_ADVANCED_PARAM));
		}
		break;

		case IO_CRYPT_CW_DERIVE_CW:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_CW_DERIVATION));
			break;

		case IO_CRYPT_ETSI_CHALLENGE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, \
				sizeof(CE_ETSI_CHALLENGE));
			break;

		default:
			return ALI_CRYPTO_ERROR_OPERATION_NOT_SUPPORTED;
	}
	jump_to_func(NULL, os_hld_caller, pcedev, CE_NPARA(3) | FUNC_CE_IOCTL, desc);
}


RET_CODE ce_generate_cw_key(const UINT8 *in_cw_data, UINT8 mode, UINT8 first_pos, UINT8 second_pos)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, 16),
		1, DESC_P_PARA(0, 0, 0),
		//desc of pointer ret
		0,
		0,
	};

	jump_to_func(NULL, os_hld_caller, in_cw_data, CE_NPARA(4) | FUNC_CE_GENERATE_CW_KEY, desc);
}

RET_CODE ce_generate_single_level_key(p_ce_device pcedev, p_ce_data_info ce_data_info)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(CE_DATA_INFO)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};

	jump_to_func(NULL, os_hld_caller, pcedev, CE_NPARA(2) | FUNC_CE_GENERATE_SINGLE_LEVEL_KEY, desc);
}

RET_CODE ce_generate_hdcp_key(p_ce_device pcedev, UINT8 *en_hdcp_key, UINT16 len)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, 288),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};

	jump_to_func(NULL, os_hld_caller, pcedev, CE_NPARA(3) | FUNC_CE_GENERATE_HDCP_KEY, desc);	
}

#endif	/*_HLD_CE_REMOTE*/

#endif	/*DUAL_ENABLE*/

/*add some common API for CE*/
/* This function is used to load key from otp to keyladder
key_pos:  OTP key pos
KEY_0_0,KEY_0_1,KEY_0_2,KEY_0_3
*/
int ce_load_otp_key(UINT32 key_pos)
{
	p_ce_device pcedev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM opt_info;
	RET_CODE ret = RET_SUCCESS;

	if((NULL == pcedev0) || (key_pos > KEY_0_3))
	{
		return RET_FAILURE;
	}
	MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
	opt_info.otp_addr = OTP_ADDESS_1 + 4 * key_pos;
	opt_info.otp_key_pos = key_pos;
	ret =ce_key_load(pcedev0,&opt_info);

	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("load OTP key failed!");
		return -1;
	}
	return 0;
}

/* This function is used to generate key use keyladder
*key:		   input key data
first_key_pos: the root key for generated
second_key_pos:the target key position
ce_crypt_select: select the ce is decrypted or encrypted
*/
int ce_generate_key_by_aes(const UINT8 *key, UINT32 first_key_pos, UINT32 second_key_pos, UINT32 ce_crypt_select)
{
	p_ce_device pcedev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO ce_data_info;
	RET_CODE ret = RET_SUCCESS;

	if((NULL == key) || ((CE_IS_DECRYPT != ce_crypt_select) && (CE_IS_ENCRYPT != ce_crypt_select)))
	{
		return RET_FAILURE;
	}

	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	MEMCPY(ce_data_info.data_info.crypt_data, key, AES_CE_KEY_LEN);
	ce_data_info.data_info.data_len 			= AES_CE_KEY_LEN;
	ce_data_info.des_aes_info.aes_or_des		 = CE_SELECT_AES;
	ce_data_info.des_aes_info.crypt_mode		 = ce_crypt_select;
	ce_data_info.des_aes_info.des_low_or_high	 = 0;
	ce_data_info.key_info.first_key_pos 		= first_key_pos ;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP;
	ce_data_info.key_info.second_key_pos		 = second_key_pos;
	ret = ce_generate_single_level_key(pcedev0, &ce_data_info);

	if ( RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("generate AES Key fail!\n");
		return -1;
	}
	return 0;
}

/* This function is used to generate key use multi keyladder
p_aes_key_ladder_buf_param pceaesparam:
u32 key_ladder; 	 how many key ladder want to use
u32 root_key_pos;	 root key pos
u8 r[256];			 random data for each key ladder
*key_pos:			 the output key pos
*/
RET_CODE aes_generate_key_with_multi_keyladder(p_aes_key_ladder_buf_param pceaesparam, UINT32 *key_pos)
{
	UINT32 i = 0;
	p_ce_device pcedev = NULL;
	CE_FOUND_FREE_POS_PARAM key_pos_param;
	UINT32 f_key_pos = 0;
	UINT32 s_key_pos = 0;
	int ret = -1;

	pcedev = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	if((NULL == pceaesparam) || (pceaesparam->root_key_pos > OTP_KEY_0_3) ||
		(pceaesparam->key_ladder >= MAGIC_CONST_8))
	{
		CE_REMOTE_API_PRINTF("input parameter error (0x%x,0x%x,0x%x)!!\n", pceaesparam, pceaesparam->root_key_pos, \
							 pceaesparam->key_ladder);
		return -1;
	}
	if(NULL == pcedev)
	{
		CE_REMOTE_API_PRINTF("crypto engine not avaliable!!\n");
		return -2;
	}
	if(NULL == key_pos)
	{
		return -1;
	}
	f_key_pos = pceaesparam->root_key_pos;
	s_key_pos = pceaesparam->root_key_pos + 4;

	//load OTP key
	ret = ce_load_otp_key(f_key_pos);
	if(ret)
	{
		CE_REMOTE_API_PRINTF("Load OTP key fail!!\n");
		return -3;
	}
	//first level key
	ret = ce_generate_key_by_aes(&(pceaesparam->r[0]), f_key_pos, s_key_pos, CE_IS_DECRYPT);
	if(ret)
	{
		CE_REMOTE_API_PRINTF("generate 1 level key fail!!\n");
		return -4;
	}
	f_key_pos = s_key_pos;
	for(i = 0; i < pceaesparam->key_ladder - 1; i++)
	{
		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		key_pos_param.ce_key_level = TWO_LEVEL + i;
		if(key_pos_param.ce_key_level > THREE_LEVEL)
		{
			key_pos_param.ce_key_level = THREE_LEVEL;
		}
		key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
		key_pos_param.root = pceaesparam->root_key_pos;
		ret = ce_ioctl(pcedev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
		if (ret)
		{
			CE_REMOTE_API_PRINTF("find free key pos fail!\n");
			return -5;
		}
		s_key_pos = key_pos_param.pos;
		ret = ce_generate_key_by_aes(&pceaesparam->r[AES_CE_KEY_LEN * (i + 1)], f_key_pos, s_key_pos, CE_IS_DECRYPT);

		if(ret)
		{
			CE_REMOTE_API_PRINTF("generate %d level key fail!!\n", (i + 2));
		}
		ce_ioctl(pcedev, IO_CRYPT_POS_SET_IDLE, f_key_pos);
		f_key_pos = s_key_pos;
	}
	*key_pos = s_key_pos;
	CE_REMOTE_API_PRINTF("Key pos is 0x%x\n", *key_pos);
	return RET_SUCCESS;
}


 /*
	 AES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
	input -> data to be decrypt
	root_pos -> root pos in key ladder(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_three key_pos in key ladder, will return to caller for other use
	if this key_pos won't be used anymore, need to set it idle:
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
RET_CODE aes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT32 *key_pos)
{
	RET_CODE ret = RET_FAILURE;
	CE_DEVICE *p_ce_dev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	if((NULL == input) || (NULL == key_pos) || (root_pos > KEY_0_3))
	{
		return RET_FAILURE;
	}

	//generate Level 1 key with decrypt raw data 1
	ret = ce_generate_key_by_aes(input,root_pos,KEY_1_0+root_pos,CE_IS_DECRYPT);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = ce_generate_key_by_aes(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2,CE_IS_DECRYPT);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		return RET_FAILURE;
	}


	//find a free level 3 key_pos
	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(p_ce_dev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 3 key with decrypt raw data 3
	ret = ce_generate_key_by_aes(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos,CE_IS_DECRYPT);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level three failed!\n");
		ce_ioctl(p_ce_dev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	CE_REMOTE_API_PRINTF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}

RET_CODE tdes_decrypt_key_to_ce_64bit(UINT8 *eck, UINT8 first_key, enum CE_CRYPT_TARGET target, UINT8 hilo_addr)
{
	CE_DATA_INFO ce_data_info;

	 CE_DEVICE *pcedev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	 if((NULL == pcedev) || (NULL == eck))
	 {
		return RET_FAILURE;
	 }

	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	ce_data_info.data_info.data_len 			= 8;				/* aes is 16 bytes des/tdes is 8 bytes */
	ce_data_info.des_aes_info.aes_or_des		 = CE_SELECT_DES;	  /* select AES or DES module */
	ce_data_info.des_aes_info.crypt_mode		 = CE_IS_DECRYPT;
	ce_data_info.des_aes_info.des_low_or_high	  = hilo_addr;
	ce_data_info.key_info.first_key_pos 		=	  first_key ;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP;
	ce_data_info.key_info.second_key_pos		 = target;
	MEMCPY(ce_data_info.data_info.crypt_data,&eck[0],8);

	if(RET_SUCCESS != ce_generate_single_level_key(pcedev, &ce_data_info))
	{
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}


RET_CODE tdes_decrypt_key_to_ce_one_level(UINT8 *eck, UINT8 level_one, enum CE_CRYPT_TARGET level_gen)
{
	CE_DATA_INFO ce_data_info;
	RET_CODE ret = RET_FAILURE;
	CE_DEVICE *pcedev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	if((NULL == pcedev) || (NULL == eck))
	{
		return RET_FAILURE;
	}

	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	ce_data_info.data_info.data_len 			= 8;			/* aes is 16 bytes des/tdes is 8 bytes*/
	ce_data_info.des_aes_info.aes_or_des		 = CE_SELECT_DES;	  /* select AES or DES module*/
	ce_data_info.des_aes_info.crypt_mode		 = CE_IS_DECRYPT;
	ce_data_info.des_aes_info.des_low_or_high	  = 1;
	ce_data_info.key_info.first_key_pos 		= level_one ;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP;
	ce_data_info.key_info.second_key_pos		 = level_gen;

	MEMCPY(ce_data_info.data_info.crypt_data, &eck[0], 8);
	ret = ce_generate_single_level_key(pcedev, &ce_data_info);

	if(RET_SUCCESS != ret)
	{
		return RET_FAILURE;
	}

	ce_data_info.des_aes_info.des_low_or_high	  = 0;	  //LOW_ADDR
	MEMCPY(ce_data_info.data_info.crypt_data, &eck[8], 8);
	if(RET_SUCCESS != ce_generate_single_level_key(pcedev, &ce_data_info))
	{
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}


RET_CODE tdes_decrypt_to_ce_two_level(UINT8 *input, UINT8 level_root, UINT32 *key_pos)
{
	RET_CODE ret = RET_FAILURE;
	CE_DEVICE *pcedev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	if((NULL == pcedev) || (NULL == input) || (NULL == key_pos) || (level_root > KEY_0_3))
	{
		return RET_FAILURE;
	}

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input, level_root, KEY_1_0 + level_root);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//find level two pos
	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = TWO_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = level_root;
	ret = ce_ioctl(pcedev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16], KEY_1_0 + level_root, key_pos_param.pos);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		ce_ioctl(pcedev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	CE_REMOTE_API_PRINTF("key_pos is %d\n", *key_pos);
	return RET_SUCCESS;
}


 /*
	 TDES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
	input -> data to be decrypt
	root_pos -> root pos in key ladder(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_three key_pos in key ladder, will return to caller for other use
	if this key_pos won't be used anymore, need to set it idle:
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
RET_CODE tdes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT32 *key_pos)
{
	RET_CODE ret = RET_FAILURE;
	CE_DEVICE *p_ce_dev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	if((NULL == p_ce_dev) || (NULL == input) || (NULL == key_pos) || (root_pos > KEY_0_3))
	{
		return RET_FAILURE;
	}

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input,root_pos,KEY_1_0+root_pos);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		return RET_FAILURE;
	}

	//find a free level 3 key_pos
	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(p_ce_dev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}

	//generate Level 3 key with decrypt raw data 3
	ret = tdes_decrypt_key_to_ce_one_level(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos);
	if (RET_SUCCESS != ret)
	{
		CE_REMOTE_API_PRINTF("Error: level three failed!\n");
		ce_ioctl(p_ce_dev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	CE_REMOTE_API_PRINTF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}

