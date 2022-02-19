/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: generate_key.c
*
*    Description: This file contains sample code for CA vendor

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/trng/trng.h>
#include <bus/otp/otp.h>
#include <osal/osal.h>
#include <hld/crypto/crypto.h>
#include <api/libpvr/lib_pvr_eng.h>
#include <api/libtsi/db_3l.h>
#include "generate_key.h"
#include "genCA_pvr.h"


#define GEN_CA_M2M2_KEY_OTP_ADDR            OTP_ADDESS_2
#define GEN_CA_PVR_KEY_LEN                16        // measured in byte

#define GEN_CA_PVR_KEY_OTP_ADDR	  (ALI_C3701>sys_ic_get_chip_id() ? 0x70: 0x88)

#define INVALID_CE_KEY_POS                0xff
#define INVALID_CRYPTO_STREAM_HANDLE    0xffffffff
UINT32 g_pvr_level2pos = INVALID_CE_KEY_POS;
//static p_ce_device pvr_ce_dev = NULL;

static const UINT8 const_ramdom_key[GEN_CA_PVR_KEY_LEN]=
{
    0x55,0x55,0x55,0x55,0xa5,0xa5,0xa5,0xa5,
    0xaa,0xaa,0xaa,0xaa,0x5a,0x5a,0x5a,0x5a,
};
//for Ali internal test
/*
static UINT8 base_random_number[GEN_CA_PVR_KEY_LEN]=
{
    0x12,0x34,0x56,0x78,0x9a,0xbc,0xef,0xef,
    0x21,0x43,0x65,0x87,0xa9,0xcb,0xfe,0x00,
};
*/
static const UINT8 EvenKeyData[GEN_CA_PVR_KEY_LEN]=
{    
    0x12,0x34,0x56,0x78,0x9a,0xbc,0xef,0xef,
    0x21,0x43,0x65,0x87,0xa9,0xcb,0xfe,0x28,
};
static const UINT8 OddKeyData[GEN_CA_PVR_KEY_LEN]=
{    
    0xae,0x34,0xef,0x78,0xef,0xcb,0xfe,0x12,
    0xbc,0x12,0x78,0x56,0x9a,0x19,0x49,0x37,
};

static const UINT8 key_encrypt[GEN_CA_PVR_KEY_LEN]=
{
    0x59,0x34,0x28,0x18,0xaf,0xde,0xfa,0x12,
    0xbc,0x12,0x5a,0x3e,0x9a,0x1f,0xef,0x37,
};
struct gen_ca_pvr_info_data
{
	UINT8 state;	//0:not use , 1: used
	UINT32 channel_id;	//record channel id
	UINT32 crypto_key_pos;	//recording key position
	UINT32 gen_ca_key_cnt;	//number of recording key
	UINT8 crypto_mode;	//re-encrypt mode : AES,DES,TDES...
	UINT8 sub_device_id;//sub_device(AES,DES,TDES...) id
	UINT8 key_type;	//even or odd key
};
struct gen_ca_pvr_info_data gen_ca_pvr_crypto_info[GEN_CA_PVR_RSC_REC_NUM];


static UINT32 gen_ca_key_len = 16;
#if 0
#define GENERATE_KEY_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#define GENERATE_KEY_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define GENERATE_KEY_DEBUG(...)            do{} while(0)
#define GENERATE_KEY_ERROR(...)            do{} while(0)
#endif

void gen_ca_clear_crypto_info(UINT32 prog_id)
{
	UINT8 index = 0;
	for(index=0;index<GEN_CA_PVR_RSC_REC_NUM;index++)
	{
		if(gen_ca_pvr_crypto_info[index].state == 1)
		{
			if(gen_ca_pvr_crypto_info[index].channel_id == prog_id)
			{
				gen_ca_pvr_crypto_info[index].state = 0;
				gen_ca_pvr_crypto_info[index].channel_id = 0;
				gen_ca_pvr_crypto_info[index].gen_ca_key_cnt = 0;
				gen_ca_pvr_crypto_info[index].crypto_key_pos = 0;
				gen_ca_pvr_crypto_info[index].crypto_mode = 0;
				gen_ca_pvr_crypto_info[index].sub_device_id = 0;
				gen_ca_pvr_crypto_info[index].key_type = 0;
				break;
			}
		}
	}
}
//get root key
int gen_ca_load_m2m2_key(void)
{
    	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    	OTP_PARAM opt_info={0,0};

    	MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
    	opt_info.otp_addr = GEN_CA_M2M2_KEY_OTP_ADDR;
    	opt_info.otp_key_pos = GEN_CA_M2M2_KEY_POS;

    	if (RET_SUCCESS != ce_key_load(p_ce_dev0, &opt_info))
    	{
       	GENERATE_KEY_DEBUG("load m2m2 key failed!");
       	return -1;
    	}
    	return 0;
}
//generate PVR key
int gen_ca_load_pk_to_ce(void)
{
    return 0;
    
    	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    	CE_DATA_INFO ce_data_info;
    	UINT8 encrypted_pk[GEN_CA_PVR_KEY_LEN] = {0};
        int i = 0;
    	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};

    	otp_init(NULL);
    	MEMSET(encrypted_pk, 0, GEN_CA_PVR_KEY_LEN);
    	for (i = 0; i < GEN_CA_PVR_KEY_LEN / 4; i++)
    	{
       	otp_read((GEN_CA_PVR_KEY_OTP_ADDR+i)*4, &encrypted_pk[i*4], 4);
    	}
	//generate temp key
    	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
    	MEMCPY(ce_data_info.data_info.crypt_data, encrypted_pk, GEN_CA_PVR_KEY_LEN);
    	ce_data_info.data_info.data_len             = GEN_CA_PVR_KEY_LEN;    /* aes is 16 bytes des/tdes is 8 bytes*/
    	ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
    	ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
    	ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
    	ce_data_info.key_info.first_key_pos         = GEN_CA_M2M2_KEY_POS ;
    	ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
    	ce_data_info.key_info.second_key_pos         = GEN_CA_PVR_KEY_POS;
    	/* get otp data from this address */
    	ce_data_info.otp_info.otp_addr                 = GEN_CA_M2M2_KEY_OTP_ADDR;
    	/*the opt key will load to the position, it  make sure "otp_key_pos" = first_key_pos*/
    	ce_data_info.otp_info.otp_key_pos             = GEN_CA_M2M2_KEY_POS;
    	if (ce_key_generate(p_ce_dev0, &ce_data_info) != RET_SUCCESS)
    	{
       	GENERATE_KEY_ERROR("generate Temp Key fail!\n");
        	return -1;
    	}
	//generate PVR key if key level is 3
    	if(THREE_LEVEL == sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS))
    	{
        	if(INVALID_CE_KEY_POS == g_pvr_level2pos)
        	{
             		//find level two pos
            		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
            		key_pos_param.ce_key_level = TWO_LEVEL;
            		key_pos_param.pos = INVALID_CE_KEY_POS;
            		key_pos_param.root = GEN_CA_M2M2_KEY_POS;
            		if (RET_SUCCESS != ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param))
            		{
                		GENERATE_KEY_ERROR("Error: find free key pos fail!\n");
                		return -1;
            		}

            		if((INVALID_CE_KEY_POS == key_pos_param.pos ) || (KEY_2_7 < key_pos_param.pos))
            		{
                		GENERATE_KEY_ERROR("Error: find level 2 free key pos fail!\n");
                		return -1;
            		}
            		g_pvr_level2pos = key_pos_param.pos;
        	}
		//generate PVR key
	       MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	       MEMCPY(ce_data_info.data_info.crypt_data, const_ramdom_key, GEN_CA_PVR_KEY_LEN);
	       ce_data_info.data_info.data_len             = GEN_CA_PVR_KEY_LEN;    /* aes is 16 bytes des/tdes is 8 bytes*/
	       ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
	       ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
	       ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
	       ce_data_info.key_info.first_key_pos         = GEN_CA_PVR_KEY_POS ;
	       ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
	       ce_data_info.key_info.second_key_pos         = g_pvr_level2pos;
	       if (RET_SUCCESS != ce_generate_single_level_key(p_ce_dev0, &ce_data_info))
	       {
	           	GENERATE_KEY_ERROR("generate PVR Key fail!\n");
	            	return -1;
	       }
	       GENERATE_KEY_DEBUG("%s key_level(%d), gLevel2Pos(%d)\n", \
	            __FUNCTION__,sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS),g_pvr_level2pos);
    	}
    	return 0;
}
/*****************************************************************************
 * Function: gen_ca_aes_key_map_ex
 * Description:
 *    set key mapping parameter
 * Input:
 *        Para 1: UINT32 key_pos , The position of recoding key
 *        Para 2: UINT16 *pid_list , PID list for re-encrypt
 *        Para 3: UINT16 pid_num , Number of PID
 *        Para 4: UINT32 stream_id , Stream id for re-encrypt
 *        Para 5: INT8 index , Device id for re-encrypt
 * Output:
 *        None
 *
 * Returns:   UINT32 key_param.handle , Crypto stream handle
 *
*****************************************************************************/
static UINT32 gen_ca_aes_key_map_ex(UINT32 key_pos, UINT16 *pid_list, UINT16 pid_num, UINT32 stream_id,INT8 index)
{
    AES_INIT_PARAM aes_param;
    KEY_PARAM key_param;
    RET_CODE ret = RET_FAILURE;
    p_aes_dev pvr_aes_dev  = NULL;

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    pvr_aes_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, index);
    MEMSET(&aes_param, 0, sizeof(AES_INIT_PARAM));
    aes_param.dma_mode = TS_MODE ;
    aes_param.key_from = KEY_FROM_CRYPTO;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = AUTO_PARITY_MODE0 ;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_ECB;
    aes_param.cbc_cts_enable = 0;
    ret = aes_ioctl(pvr_aes_dev, IO_INIT_CMD, (UINT32)&aes_param);
    if (RET_SUCCESS != ret)
    {
        GENERATE_KEY_ERROR("AES IO_INIT_CMD fail\n");
        return INVALID_CRYPTO_STREAM_HANDLE;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.ctr_counter = NULL ;
    key_param.init_vector = NULL ;
    key_param.key_length = 128;
    key_param.pid_len = pid_num;
    key_param.pid_list = pid_list;
    key_param.p_aes_iv_info = NULL ;
    key_param.stream_id = stream_id;
    key_param.force_mode = 1;
    if(sys_ic_get_chip_id()>=ALI_C3505)
    {
        key_param.pos = key_pos&0xFF;
        key_param.kl_sel = (key_pos>>8)&0x0F;
    }
    else
        key_param.pos = key_pos;
    ret = aes_ioctl(pvr_aes_dev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if (RET_SUCCESS != ret )
    {
        GENERATE_KEY_ERROR("AES IO_CREAT_CRYPT_STREAM_CMD fail\n");
        return INVALID_CRYPTO_STREAM_HANDLE;
    }

    return key_param.handle;
}

RET_CODE gen_ca_pvr_setup_key(UINT32 key_pos)
{
    UINT8 encrypted_pk[16] = {0};
    UINT8 i=0;
    RET_CODE ret = 0;
    p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    
    CE_FOUND_FREE_POS_PARAM key_pos_param;

	UINT32 key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);	//Ben 180613#1
	otp_init(NULL);
    MEMSET(encrypted_pk, 0, sizeof(encrypted_pk));
    for (i = 0; i < GEN_CA_PVR_KEY_LEN/ 4; i++)
    {
        otp_read((GEN_CA_PVR_KEY_OTP_ADDR + i) * 4, &encrypted_pk[i * 4], 4);
    }
    
	//2. Generate the even key
	CE_NLEVEL_PARAM nlevel_param;
    MEMSET(&nlevel_param, 0, sizeof(CE_NLEVEL_PARAM));
    nlevel_param.kl_index = key_pos >> 8;
    nlevel_param.otp_addr = OTP_ADDESS_3;
    nlevel_param.algo = CE_SELECT_AES;
    nlevel_param.crypto_mode = CE_IS_DECRYPT;
    nlevel_param.pos = key_pos;
    nlevel_param.protecting_key_num = key_level-1;	//0;	
    
    //memcpy(&(nlevel_param.content_key[8]), encrypted_pk, 8);
    memcpy(&(nlevel_param.content_key[0]), &encrypted_pk[0], 16);
    nlevel_param.parity = CE_PARITY_EVEN_ODD;  

    ret = ce_ioctl(p_ce_dev0, IO_CRYPT_GEN_NLEVEL_KEY, (UINT32)&nlevel_param);
    if (RET_SUCCESS != ret)
    {
        GENERATE_KEY_ERROR("%s(): IO_CRYPT_GEN_NLEVEL_KEY - fail!\n",__FUNCTION__);
        ret = RET_FAILURE;
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        return ret;
    }
    ret = RET_SUCCESS;
    return ret;
}

/*****************************************************************************
 * Function: gen_ca_r_set_pvr_key
 * Description:
 *    generate recording key.Then, encrypt key data
 * Input:
 *        Para 1: UINT8 *input , key data to generate recording key
 *        Para 3: UINT32 key_pos , the position of recording key
 * Output:
 *        Para 2: UINT8 *output , encryption of input
 *
 * Returns:   TRUE : success , FALSE : failure
 *
*****************************************************************************/
static BOOL gen_ca_r_set_pvr_key(UINT8 *input , UINT8 *output , UINT32 key_pos , UINT32 first_key_pos)
{
#if 1
    RET_CODE ret = 0;
    ret = gen_ca_pvr_setup_key(key_pos);
    if(ret!=RET_SUCCESS)
        return FALSE;
    else
        return TRUE;
#else
	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	RET_CODE ret = RET_FAILURE; 
	CE_DATA_INFO ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
	
	//generate key for pvr re-encrypt
	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
       MEMCPY(ce_data_info.data_info.crypt_data, input, gen_ca_key_len);
       ce_data_info.data_info.data_len             = gen_ca_key_len;    /* aes is 16 bytes des/tdes is 8 bytes*/
       ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
       ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
       ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
       ce_data_info.key_info.first_key_pos         = first_key_pos ;
       ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
       ce_data_info.key_info.second_key_pos         = key_pos;
       if (RET_SUCCESS != ce_generate_single_level_key(p_ce_dev0, &ce_data_info))
       {
           	GENERATE_KEY_ERROR("%s() : set recording key fail!\n", __FUNCTION__);

           	return FALSE;
       }
	//generate key for encrypt input data

	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.number = 1;
    	key_pos_param.pos = INVALID_CE_KEY_POS;
    	key_pos_param.root = GEN_CA_M2M2_KEY_POS;
    	ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (RET_SUCCESS != ret)
    	{
       	GENERATE_KEY_ERROR("find free key pos for CA fail!\n");
        	return FALSE;
    	}
	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	MEMCPY(ce_data_info.data_info.crypt_data, key_encrypt, gen_ca_key_len);
	ce_data_info.data_info.data_len             = gen_ca_key_len;    /* aes is 16 bytes des/tdes is 8 bytes*/
       ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
       ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
       ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
       ce_data_info.key_info.first_key_pos         = first_key_pos;
       ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
       ce_data_info.key_info.second_key_pos         = key_pos_param.pos;
       if (RET_SUCCESS != ce_generate_single_level_key(p_ce_dev0, &ce_data_info))
       {
           	GENERATE_KEY_ERROR("%s() : set key_data encrypt key fail!\n", __FUNCTION__);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
           	return FALSE;
       }
	
	ret = aes_crypt_puredata_with_ce_key(input,output,gen_ca_key_len,key_pos_param.pos,DSC_ENCRYPT);
    	if(ret)
    	{
       	GENERATE_KEY_ERROR("%s() : encrypt key data fail!\n",__FUNCTION__);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return FALSE;
    	}
	ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
#endif    
	return TRUE;
}
/*****************************************************************************
 * Function: gen_ca_p_set_pvr_key
 * Description:
 *    decrypt key data.Then, generate recording key 
 * Input:
 *        Para 1: gen_ca_key_info *key_info , key data to generate recording key
 *        Para 2: UINT32 key_pos , the position of recording key
 * Output:
 *        None
 *
 * Returns:   TRUE : success , FALSE : failure
 *
*****************************************************************************/
BOOL gen_ca_p_set_pvr_key(gen_ca_key_info *key_info , UINT32 key_pos , UINT32 first_key_pos)
{
#if 1
    RET_CODE ret = 0;
    ret = gen_ca_pvr_setup_key(key_pos);
    if(ret!=RET_SUCCESS)
        return FALSE;
    else
        return TRUE;
#else
	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	RET_CODE ret = RET_FAILURE; 
	UINT8 *key_data = NULL;
	CE_DATA_INFO ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};

	key_data = (UINT8 *)MALLOC(sizeof(UINT8)*key_info->key_len);
	MEMSET(key_data,0,sizeof(UINT8)*key_info->key_len);

	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.number = 1;
    	key_pos_param.pos = INVALID_CE_KEY_POS;
    	key_pos_param.root = GEN_CA_M2M2_KEY_POS;
    	ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (RET_SUCCESS != ret)
    	{
       	GENERATE_KEY_ERROR("find free key pos for CA fail!\n");
        	return FALSE;
    	}
	
	//generate key for decrypt input data
	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	MEMCPY(ce_data_info.data_info.crypt_data, key_encrypt, gen_ca_key_len);
	ce_data_info.data_info.data_len             = gen_ca_key_len;    /* aes is 16 bytes des/tdes is 8 bytes*/
       ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
       ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
       ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
       ce_data_info.key_info.first_key_pos         = first_key_pos ;
       ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
       ce_data_info.key_info.second_key_pos         = key_pos_param.pos;
       if (RET_SUCCESS != ce_generate_single_level_key(p_ce_dev0, &ce_data_info))
       {
           	GENERATE_KEY_ERROR("%s() : set key_data decrypt key fail!\n", __FUNCTION__);
           	return FALSE;
       }
	   
	ret = aes_crypt_puredata_with_ce_key(key_info->key_data,key_data,key_info->key_len,key_pos_param.pos,DSC_DECRYPT);
    	if(ret)
    	{
       	GENERATE_KEY_ERROR("%s() : decrypt key data fail!\n",__FUNCTION__);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return FALSE;
    	}
	//generate key for pvr re-encrypt	
	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
       MEMCPY(ce_data_info.data_info.crypt_data, key_data, key_info->key_len);
       ce_data_info.data_info.data_len             = key_info->key_len;    /* aes is 16 bytes des/tdes is 8 bytes*/
       ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
       ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
       ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
       ce_data_info.key_info.first_key_pos         = first_key_pos ;
       ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
       ce_data_info.key_info.second_key_pos         = key_pos;
       if (RET_SUCCESS != ce_generate_single_level_key(p_ce_dev0, &ce_data_info))
       {
           	GENERATE_KEY_ERROR("%s() : set recording key fail!\n", __FUNCTION__);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
	 	FREE(key_data);
           	return FALSE;
       }
	ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
	FREE(key_data);
	return TRUE;
#endif    
}
/*****************************************************************************
 * Function: gen_ca_r_set_pvr_crypto_para
 * Description:
 *    set recording key and key mapping parameter when record
 * Input:
 *        Para 1: gen_ca_pvr_crypto_param *param , re-encrypt parameter
 * Output:
 *        Para 2: UINT8 *key_data , encrypted data for generating recording key
 *
 * Returns:   UINT32 crypto_stream_hnd , Crypto stream handle
 *
*****************************************************************************/
UINT32 gen_ca_r_set_pvr_crypto_para(struct gen_ca_pvr_crypto_param *param, UINT8 *key_data)
{
    	UINT32 crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
	UINT32 key_level = 0;
	UINT8 i=0;
		
    	if(NULL == param)
    	{
        	return INVALID_CRYPTO_STREAM_HANDLE;
    	}

	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
	//if(FALSE == gen_ca_r_set_pvr_key(base_random_number,key_data,param->crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))//for Ali internal test
	if(FALSE == gen_ca_r_set_pvr_key((UINT8 *)EvenKeyData,key_data,param->crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
	{		
		GENERATE_KEY_ERROR("%s() : gen_ca_r_set_pvr_key fail!\n", __FUNCTION__);
		return INVALID_CRYPTO_STREAM_HANDLE;
	}

    	crypto_stream_hnd = gen_ca_aes_key_map_ex(param->crypto_key_pos, param->pid_list, param->pid_num, param->stream_id,param->crypto_dev_id);
    	if (INVALID_CRYPTO_STREAM_HANDLE == crypto_stream_hnd)
    	{
        	GENERATE_KEY_ERROR("%s generate crypto_stream_hnd fail\n",__FUNCTION__);
		return INVALID_CRYPTO_STREAM_HANDLE;
    	}	

	for(i=0;i<GEN_CA_PVR_RSC_REC_NUM;i++)
	{
		if(gen_ca_pvr_crypto_info[i].state == 0)
		{
			gen_ca_pvr_crypto_info[i].state = 1;
			gen_ca_pvr_crypto_info[i].channel_id = param->prog_id;
			gen_ca_pvr_crypto_info[i].crypto_key_pos = param->crypto_key_pos;
			gen_ca_pvr_crypto_info[i].gen_ca_key_cnt = 0;
			gen_ca_pvr_crypto_info[i].crypto_mode = AES;
			gen_ca_pvr_crypto_info[i].sub_device_id = param->crypto_dev_id;
			break;
		}
	}
    	return crypto_stream_hnd;
}
/*****************************************************************************
 * Function: gen_ca_p_set_pvr_crypto_para
 * Description:
 *    set recording key and key mapping parameter when playback
 * Input:
 *        Para 1: gen_ca_pvr_crypto_param *param , decrypt parameter
 *        Para 2: gen_ca_key_info *key_info , encrypted data for generating recording key
 * Output:
 *        None
 *
 * Returns:   UINT32 crypto_stream_hnd , Decrypt  stream handle
 *
*****************************************************************************/
UINT32 gen_ca_p_set_pvr_crypto_para(struct gen_ca_pvr_crypto_param *param , gen_ca_key_info *key_info)
{  	
    	UINT32 crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
	UINT32 key_level = 0;
	
    	if(NULL == param)
    	{
        	return INVALID_CRYPTO_STREAM_HANDLE;
    	}
		
	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
	//if(0 == key_info->key_type)
	//{
	if(FALSE == gen_ca_p_set_pvr_key(key_info,param->crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
	{
		GENERATE_KEY_ERROR("%s : gen_ca_p_set_pvr_key fail\n",__FUNCTION__);
		return INVALID_CRYPTO_STREAM_HANDLE;
	}
	//}
	//else
	//{
	//	if(FALSE == gen_ca_p_set_pvr_key(key_info,param->crypto_key_pos+1,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
	//	{
	//		GENERATE_KEY_ERROR("%s : gen_ca_p_set_pvr_key fail\n",__FUNCTION__);			
	//		return INVALID_CRYPTO_STREAM_HANDLE;
	//	}
//	}
	
    	crypto_stream_hnd = gen_ca_aes_key_map_ex(param->crypto_key_pos, param->pid_list, param->pid_num, param->stream_id,param->crypto_dev_id);
    	if (INVALID_CRYPTO_STREAM_HANDLE == crypto_stream_hnd)
    	{
        	GENERATE_KEY_ERROR("%s generate crypto_stream_hnd fail\n",__FUNCTION__);
    	}
	
    	return crypto_stream_hnd;
}
/*****************************************************************************
 * Function: change_key
 * Description:
 *    change key when recording
 * Input:
 *        Para 1: UINT32 prog_id , recording channel id
 * Output:
 *        None
 *
 * Returns:   None
 *
*****************************************************************************/
void change_key(UINT32 prog_id)
{
	UINT8 *key_data = NULL;
	UINT8 __MAYBE_UNUSED__ crypto_mode = 0;
	UINT8 sub_device_id = 0;
	UINT32 gen_ca_key_cnt = 0;
	UINT32 crypto_key_pos = 0;
	UINT8 key_type = 0;
	UINT32 key_level = 0;
	UINT8 index = 0;	
	//UINT8 key_random_number[GEN_CA_PVR_KEY_LEN] = {0};//for Ali internal test
	struct gen_ca_crypto_info crypto_info;
	
	for(index=0;index<GEN_CA_PVR_RSC_REC_NUM;index++)
	{
		if(gen_ca_pvr_crypto_info[index].state == 1)
		{
			if(gen_ca_pvr_crypto_info[index].channel_id == prog_id)
			{
				gen_ca_key_cnt = gen_ca_pvr_crypto_info[index].gen_ca_key_cnt;
				crypto_key_pos = gen_ca_pvr_crypto_info[index].crypto_key_pos;
				crypto_mode = gen_ca_pvr_crypto_info[index].crypto_mode;
				sub_device_id = gen_ca_pvr_crypto_info[index].sub_device_id;
				key_type = gen_ca_pvr_crypto_info[index].key_type;
				break;
			}
		}
	}
	if(index == GEN_CA_PVR_RSC_REC_NUM)
	{
		GENERATE_KEY_ERROR("%s() : Can't find recording channel id\n", __FUNCTION__);
	}
	
	key_data = (UINT8 *)MALLOC(sizeof(UINT8)*gen_ca_key_len);
	MEMSET(key_data,0,sizeof(UINT8)*gen_ca_key_len);
	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
	//for Ali internal test
	/*
	MEMCPY(key_random_number,base_random_number,GEN_CA_PVR_KEY_LEN);
	key_random_number[15] = base_random_number[15] + gen_ca_key_cnt + 1;
	*/
	if(key_type==1)
	{
		//if(FALSE == gen_ca_r_set_pvr_key(key_random_number, key_data , crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))//for Ali internal test
		if(FALSE == gen_ca_r_set_pvr_key((UINT8 *)EvenKeyData, key_data , crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
		{
			GENERATE_KEY_ERROR("%s() : gen_ca_r_set_pvr_key fail!\n", __FUNCTION__);
		}
		gen_ca_key_cnt++;
		crypto_info.gen_ca_key_cnt= gen_ca_key_cnt;
		crypto_info.key_len = gen_ca_key_len;
		crypto_info.key_type= EVEN_PARITY_MODE;
		crypto_info.crypto_mode = AES;
		crypto_info.sub_device_id = sub_device_id;
	}
	else
	{		
		//if(FALSE == gen_ca_r_set_pvr_key(key_random_number, key_data , crypto_key_pos+1,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))//for Ali internal test
		if(FALSE == gen_ca_r_set_pvr_key((UINT8 *)OddKeyData, key_data , crypto_key_pos,THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
		{
			GENERATE_KEY_ERROR("%s() : gen_ca_r_set_pvr_key fail!\n", __FUNCTION__);
		}
		gen_ca_key_cnt++;
		crypto_info.gen_ca_key_cnt= gen_ca_key_cnt;
		crypto_info.key_len = gen_ca_key_len;
		crypto_info.key_type= ODD_PARITY_MODE;
		crypto_info.crypto_mode = AES;
		crypto_info.sub_device_id = sub_device_id;
	}
	
	if(FALSE == gen_ca_pvr_set_key_info(prog_id,key_data,crypto_info))
	{
		GENERATE_KEY_ERROR("%s() : gen_ca_pvr_save_key fail!\n", __FUNCTION__);
		FREE(key_data);
	}
	
	FREE(key_data);
	gen_ca_pvr_crypto_info[index].gen_ca_key_cnt = crypto_info.gen_ca_key_cnt;
	gen_ca_pvr_crypto_info[index].key_type = crypto_info.key_type;	
}

