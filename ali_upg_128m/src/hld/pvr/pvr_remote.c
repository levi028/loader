/*****************************************************************************
*	 Copyright (c) 2015 ALi Corp. All Rights Reserved
*	 This source is confidential and is ALi's proprietary information.
*	 This source is subject to ALi License Agreement, and shall not be
	 disclosed to unauthorized individual.
*	 File: pvr_remote.c
*
*	 Description: This file provides some PVR remote interface.
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
#include <hld/dsc/dsc.h>
#include <hld/pvr/pvr_remote.h>

#define INVALID_CRYPTO_STREAM_ID        (0xff)
#define INVALID_CRYPTO_KEY_POS          (0xff)
#define INVALID_CRYPTO_STREAM_ID        (0xff)
#define INVALID_CRYPTO_STREAM_HANDLE    (0xffffffff)
#define INVALID_CRYPTO_MODE             (0xffffffff)
#define UNCACHE_ADDR(addr)       ((addr)|0xa0000000)

#ifdef DUAL_ENABLE

#include <modules.h>

#ifdef _HLD_PVR_REMOTE

#define QUANTUM_SIZE    (47 * 1024)
#define DEDAULT_BLOCK_SIZE (0xBC00) //384*TS_PACKET_SIZE
#define ALGO_AES_IV_LENGTH    16
#define ALGO_DES_IV_LENGTH    8
#define DEDAULT_BLOCK_SIZE (0xBC00) //384*TS_PACKET_SIZE

/* Per-device (per-pvr) structure */
struct pvr_dev
{
   struct list_head multi_rec;
   struct list_head multi_play;   
};

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

static OSAL_ID s_mutex_pvr = OSAL_INVALID_ID;
struct pvr_dev pvr_dev = {
	.multi_rec = LIST_HEAD_INIT(pvr_dev.multi_rec),
	.multi_play = LIST_HEAD_INIT(pvr_dev.multi_play)
	};
static char decrypt_res_magic[4] =
{
    'A',
    'P',
    'V',
    'R',
};
//static struct decrypt_buffer_mgr   *g_decrypt_buffer_mgr = NULL;
static UINT32 pvr_g_block_size = DEDAULT_BLOCK_SIZE;


enum HLD_PVR_FUNC
{
	FUNC_PVR_RPC_IOCTL = 0,
        
	FUNC_PVR_RPC_DECRYPT = 1,
	/* ... */
	FUNC_PVR_RPC_DEAL_QUANTUM_FOR_MIXED_TS = 2,
	FUNC_SCA_CW_POS_GEN = 3,

	FUNC_SCA_CW_POS_REL = 4,
};
static void pvr_main_cache_clean(UINT32 in, UINT32 data_length)
{
#ifdef DUAL_ENABLE
#ifdef MAIN_CPU
    if((0 == in) || (0 == data_length))
    {
        return;
    }
    if ( !osal_dual_is_private_address ( (void *)in ) )
    {
        osal_cache_flush ( (void *)in, data_length );
        osal_cache_invalidate ( (void *)in, data_length );
    }
#endif
#endif
}

static void pvr_main_cache_flush(UINT32 in, UINT32 out, UINT32 data_length)
{
#ifdef DUAL_ENABLE
#ifdef MAIN_CPU
    if((0 == in) || (0 == out ) || (0 == data_length))
    {
        return;
    }
    if ( !osal_dual_is_private_address ( (void *)in ) )
    {
        osal_cache_flush ( (void *)in, data_length );
    }

    if ( !osal_dual_is_private_address ( (void *)out ))
    {
        pvr_main_cache_clean( out, data_length );
    }
#endif
#endif
}

static void pvr_main_cache_inv(UINT32 out, UINT32 data_length)
{
#ifdef DUAL_ENABLE
#ifdef MAIN_CPU
    if((0 == out ) || (0 == data_length))
    {
        return;
    }
    if ( !osal_dual_is_private_address ( (void *)out ) )
    {
        osal_cache_invalidate ( (void *)out, data_length );
    }
#endif
#endif
}

static RET_CODE de_encrypt_ex(DEEN_CONFIG *p_deen, UINT8 *input, UINT8 *output ,
									UINT32 total_length, PVR_REC_VIDEO_PARAM *video_param)
{
    UINT32 desc2[] = {   2, DESC_OUTPUT_STRU(0, sizeof(DEEN_CONFIG)), DESC_OUTPUT_STRU(1, sizeof(PVR_REC_VIDEO_PARAM)), \
                        2, DESC_P_PARA(0, 0, 0), DESC_P_PARA(1, 4, 1), \
                        0, 0,};
    
    DESC_OUTPUT_STRU_SET_SIZE(desc2, 0, sizeof(DEEN_CONFIG));
    
    jump_to_func(NULL, os_hld_caller, p_deen, PVR_RPC_NPARA(5) | FUNC_PVR_RPC_DECRYPT, desc2);
}

RET_CODE pvr_de_encrypt(DEEN_CONFIG *p_deen, UINT8 *input, UINT8 *output , UINT32 total_length, PVR_REC_VIDEO_PARAM *video_param)
{
    RET_CODE ret = RET_FAILURE;
    PVR_RPC_PRINTF("pvr_de_encrypt command is set!\n");

    if((total_length > TS_MAX_SIZE)||( NULL == input) || (NULL == output) || (NULL == p_deen))
    {
        return ret;
    }

    pvr_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    ret = de_encrypt_ex(p_deen, input, output, total_length, video_param);
    pvr_main_cache_inv((UINT32)output, total_length * 188);

    return ret;
}

RET_CODE pvr_deal_quantum_for_mixed_ts(DEEN_CONFIG* p_deen, UINT32 temp_length)
{
		UINT32 desc2[] = {   1, DESC_OUTPUT_STRU(0, sizeof(DEEN_CONFIG)), \
                        1, DESC_STATIC_STRU(0, 0), \
                        0, 0,};
	
    	DESC_OUTPUT_STRU_SET_SIZE(desc2, 0, sizeof(DEEN_CONFIG));
        jump_to_func(NULL, os_hld_caller, p_deen, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_DEAL_QUANTUM_FOR_MIXED_TS, desc2);
}

static int ali_pvr_capture_decrypt_res
(
    unsigned int cmd,
    unsigned long arg
)
{
    int ret = 0;
    struct ali_pvr_capt_decrypt_res_param *capt_decrypt_param = (struct ali_pvr_capt_decrypt_res_param *)arg;
    struct pvr_decrypt_res_param  *res_param = NULL;

    if((capt_decrypt_param->block_data_size % TS_PACKET_SIZE) != 0)
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    res_param = (struct pvr_decrypt_res_param  *)MALLOC(sizeof(struct pvr_decrypt_res_param));

    if(NULL == res_param)
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }
    memset(res_param,0x00,sizeof(struct pvr_decrypt_res_param));
    res_param->decrypt_hdl = (unsigned int)(res_param);
    memcpy(res_param->magic,decrypt_res_magic,sizeof(decrypt_res_magic));
    res_param->dsc_handle_num = capt_decrypt_param->decrypt_dsc_num;
    res_param->block_data_size = capt_decrypt_param->block_data_size;
    res_param->block_data_buf = NULL;
    res_param->key_item_info = NULL;

    res_param->key_item_info = (struct pvr_key_item *)MALLOC(sizeof(struct pvr_key_item)*res_param->dsc_handle_num);
    if(NULL == res_param->key_item_info)
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        FREE(res_param);
        return RET_FAILURE;
    }

    //res_param->block_data_buf = (unsigned char   *)kmalloc(res_param->block_data_size, GFP_KERNEL);
    if(pvr_g_block_size != 0)/*for OTT certifcation*/
    {
        res_param->block_data_buf = (unsigned char *)MALLOC(res_param->block_data_size);
    }
    if(NULL == res_param->block_data_buf && pvr_g_block_size != 0)
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        FREE(res_param->key_item_info);
        FREE(res_param);
        return RET_FAILURE;
    }

    osal_mutex_lock(s_mutex_pvr,OSAL_WAIT_FOREVER_TIME);
    list_add(&res_param->list_head,&pvr_dev.multi_play);
    osal_mutex_unlock(s_mutex_pvr);
    capt_decrypt_param->decrypt_hdl = res_param->decrypt_hdl;
	capt_decrypt_param->buf = res_param->block_data_buf;

    return ret;
}


static int ali_pvr_release_decrypt_res
(
    unsigned int cmd,
    unsigned long arg
)
{
    int ret = 0;
    unsigned int decrypt_handle;
    struct pvr_decrypt_res_param  *res_param = NULL;
    struct list_head *pos = NULL;
    int find= 0;
    decrypt_handle  = arg;


    osal_mutex_lock(s_mutex_pvr,OSAL_WAIT_FOREVER_TIME);
    list_for_each(pos,&pvr_dev.multi_play)
    {
        res_param = container_of(pos,struct pvr_decrypt_res_param,list_head);
        if((res_param != NULL) && (res_param->decrypt_hdl == decrypt_handle) &&
           (!memcmp(res_param->magic,decrypt_res_magic,sizeof(decrypt_res_magic)))
          )
        {
            list_del(pos);
            FREE(res_param->key_item_info);
            res_param->key_item_info = NULL;
            if(res_param->block_data_buf) /*for OTT certifcation*/
            {
            	FREE(res_param->block_data_buf);
                res_param->block_data_buf = NULL;
            }
            FREE(res_param);
            res_param = NULL;
            find = 1;
            break;
        }
    }
    osal_mutex_unlock(s_mutex_pvr);

    if(1 != find)
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        ret = RET_FAILURE;
    }
    return ret;
}


static int _ali_pvr_set_decrytp_res_to_key_item
(
    struct ali_pvr_set_decrypt_res_param *res_param,
    struct pvr_key_item  *key_item
)
{
    int  ret = 0;
    //int  ret2 = 0;
    if(res_param != NULL && key_item != NULL)
    {
        key_item->switch_block_count = res_param->decrypt_switch_block;
        key_item->decrypt_evo_info.algo         =  res_param->dsc_ca_attr.crypt_mode;
        key_item->decrypt_evo_info.stream_id    = res_param->dsc_ca_attr.stream_id;
        key_item->decrypt_evo_info.dev          = res_param->dsc_ca_attr.sub_dev_see_hdl;
        key_item->switch_block_count            = res_param->decrypt_switch_block;
        if(res_param->dsc_mode == BLOCK_DATA_MODE_PURE_DATA)
        {
            key_item->decrypt_evo_info.dsc_dma_mode    = res_param->dsc_mode;
        }
        else
        {
            key_item->decrypt_evo_info.dsc_dma_mode    = 1<<24;
        }
        key_item->decrypt_evo_info.key_handle = res_param->dsc_key_attr.key_handle;

        if(AES == key_item->decrypt_evo_info.algo)
        {
            if(res_param->iv_lenth > ALGO_AES_IV_LENGTH)
            {
                PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
                return -4;
            }
        }
        else if(DES == key_item->decrypt_evo_info.algo || TDES == key_item->decrypt_evo_info.algo)
        {
            if(res_param->iv_lenth > ALGO_DES_IV_LENGTH)
            {
                PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
                return -3;
            }
        }

        if(res_param->iv_data != NULL)
        {
            memcpy(key_item->decrypt_evo_info.iv_data,res_param->iv_data,
                   (res_param->iv_lenth > sizeof(key_item->decrypt_evo_info.iv_data)?
                    sizeof(key_item->decrypt_evo_info.iv_data):res_param->iv_lenth)
                  );
        }
        key_item->decrypt_evo_info.iv_length =
            (res_param->iv_lenth > sizeof(key_item->decrypt_evo_info.iv_data)?
             sizeof(key_item->decrypt_evo_info.iv_data):res_param->iv_lenth);
        key_item->decrypt_evo_info.iv_parity = res_param->iv_parity;
    }
    else
    {
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        ret = -1;
    }
    return ret;

}


static int ali_pvr_set_decrypt_res
(
    unsigned int cmd,
    unsigned long arg
)
{
    int ret = 0;
    struct ali_pvr_set_decrypt_res_param *set_decrypt_param = (struct ali_pvr_set_decrypt_res_param *)arg;
    struct pvr_decrypt_res_param  *res_param = NULL;
    unsigned char *k_buf = NULL;
    struct list_head *pos = NULL;
    int set_ok = 0;
    unsigned int i = 0;

    osal_mutex_lock(s_mutex_pvr,OSAL_WAIT_FOREVER_TIME);
    list_for_each(pos,&pvr_dev.multi_play)
    {
        res_param = container_of(pos,struct pvr_decrypt_res_param,list_head);
        if((res_param != NULL) && (res_param->decrypt_hdl == set_decrypt_param->decrypt_hdl) &&
           (!memcmp(res_param->magic,decrypt_res_magic,sizeof(decrypt_res_magic)))
          )
        {
            for(i = 0; i < res_param->dsc_handle_num; i++)
            {
                if(i == set_decrypt_param->decrypt_index)
                {
                    ret = _ali_pvr_set_decrytp_res_to_key_item(set_decrypt_param,res_param->key_item_info + i);
                    if(ret == 0)
                    {
                        set_ok = 1;
                    }
                    break;
                }
            }
            break;
        }
    }
    osal_mutex_unlock(s_mutex_pvr);

    if(1 != set_ok)
    {
        ret = -1;
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
    }
    if(NULL != k_buf)
    {
        FREE(k_buf);
        k_buf = NULL;
    }
    return ret;
}

static int pvr_decrytp_raw_evo(UINT32 cmd, PVR_decrypt_evo *param)
{
	static UINT32 desc3[] =
		{
			//desc of pointer para
			1, DESC_OUTPUT_STRU(0, sizeof(PVR_decrypt_evo)),
			1, DESC_P_PARA(0, 1, 0),
			//desc of pointer ret
			0,
			0,
		};
	jump_to_func(NULL, os_hld_caller, NULL, (HLD_PVR_MODULE<<24) | (2<<16) | FUNC_PVR_RPC_IOCTL, desc3);
}

int ali_pvr_decrytp_raw_evo
(
    unsigned int cmd,
    unsigned long arg
)
{
    int rpc_cmd = PVR_RPC_IO_RAW_DECRYPT_EVO;
    int ret = 0;
    struct ali_pvr_data_decrypt_param *raw_decrypt_param = (struct ali_pvr_data_decrypt_param *)arg;
    struct pvr_key_item   *p_item = NULL;
    struct pvr_decrypt_res_param  *res_param = NULL;
    struct list_head *pos = NULL;
    unsigned int i = 0,use_key_pos = 0xFFFFFFFF;
    unsigned int block_dif = 0xFFFFFFFF;

    PVR_decrypt_evo decrypt_evo;

    osal_mutex_lock(s_mutex_pvr,OSAL_WAIT_FOREVER_TIME);
    list_for_each(pos,&pvr_dev.multi_play)
    {
        res_param = container_of(pos,struct pvr_decrypt_res_param,list_head);
        if((res_param != NULL) && (res_param->decrypt_hdl == raw_decrypt_param->decrypt_hdl) &&
           (!memcmp(res_param->magic,decrypt_res_magic,sizeof(decrypt_res_magic))))
        {
            if(res_param->dsc_handle_num> 0)
            {
                for(i = 0; i < res_param->dsc_handle_num; i++)
                {
                    p_item = res_param->key_item_info + i;
                    if(((raw_decrypt_param->block_index < p_item->switch_block_count)
                        && ((p_item->switch_block_count - raw_decrypt_param->block_index) < block_dif))
                       || ((p_item->switch_block_count == 0xFFFFFFFF ) && (raw_decrypt_param->block_index == 0)))   //speical case.
                    {
                        block_dif = (p_item->switch_block_count - raw_decrypt_param->block_index);
                        use_key_pos = i;
                    }
                }
                if(use_key_pos != 0xFFFFFFFF)
                {
                    break;
                }
            }
            else
            {
                ret = -RET_FAILURE;
                break;
            }
        }
    }
    osal_mutex_unlock(s_mutex_pvr);
    if((use_key_pos == 0xFFFFFFFF) || (use_key_pos > res_param->dsc_handle_num))
    {
        ret = -RET_FAILURE;
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        return ret;
    }
    if(((raw_decrypt_param->length > res_param->block_data_size) && res_param->block_data_size != 0)
       || (((raw_decrypt_param->length % TS_PACKET_SIZE) != 0) && (raw_decrypt_param->des_flag == PVR_OTT_DATA_DMX))
       || (raw_decrypt_param->length == 0 ) )
    {
        ret = -RET_FAILURE;
        PVR_RPC_PRINTF("%s,line:%d\n", __FUNCTION__, __LINE__);
        return ret;
    }
  
	decrypt_evo.input = raw_decrypt_param->input;

    p_item = res_param->key_item_info + use_key_pos;
    decrypt_evo.algo      = p_item->decrypt_evo_info.algo;
    decrypt_evo.data_mode = p_item->decrypt_evo_info.dsc_dma_mode;
    decrypt_evo.dev       = p_item->decrypt_evo_info.dev;
    memcpy(decrypt_evo.iv_data,p_item->decrypt_evo_info.iv_data,p_item->decrypt_evo_info.iv_length);
    decrypt_evo.iv_length = p_item->decrypt_evo_info.iv_length;
    if(decrypt_evo.iv_length > 0)
    {
		if ( !osal_dual_is_private_address ( (void *)(decrypt_evo.input) ) )
    	{
        	osal_cache_flush ( (void *)(decrypt_evo.iv_data), (decrypt_evo.iv_length));
    	}
    }
    decrypt_evo.iv_parity = p_item->decrypt_evo_info.iv_parity;
    decrypt_evo.length    = raw_decrypt_param->length;
    decrypt_evo.stream_id = p_item->decrypt_evo_info.stream_id;
    decrypt_evo.key_handle= p_item->decrypt_evo_info.key_handle;
    decrypt_evo.block_indicator = raw_decrypt_param->block_vob_indicator;
    decrypt_evo.decv_id = raw_decrypt_param->decv_id;
    if (pvr_g_block_size > 0)
    {
        decrypt_evo.des_flag = PVR_OTT_DATA_DMX;
        memcpy(decrypt_evo.iv_data,p_item->decrypt_evo_info.iv_data,p_item->decrypt_evo_info.iv_length);
        decrypt_evo.iv_length = p_item->decrypt_evo_info.iv_length;
    }
    else
    {
        decrypt_evo.des_flag = raw_decrypt_param->des_flag;
        if(raw_decrypt_param->iv_length > 0)
        {
            memcpy(decrypt_evo.iv_data,raw_decrypt_param->iv_data,raw_decrypt_param->iv_length);
        }
        decrypt_evo.iv_length = raw_decrypt_param->iv_length;
    }
	if ( !osal_dual_is_private_address ( (void *)(decrypt_evo.input) ) )
    {
        osal_cache_flush ( (void *)(decrypt_evo.input), (decrypt_evo.length));
    }

    cmd = rpc_cmd;
	
    ret = pvr_decrytp_raw_evo(cmd,&decrypt_evo);

    return ret;
}

static RET_CODE pvr_update_enc_param_m(int cmd, void *param)
{
	UINT32 decrypt_control[] =
    {
        //desc of pointer para
        1, DESC_OUTPUT_STRU(0, sizeof(struct PVR_BLOCK_ENC_PARAM)),
        1, DESC_P_PARA(0, 1, 0),
        //desc of pointer ret
        0,
        0,
    };
	DESC_OUTPUT_STRU_SET_SIZE(decrypt_control, 0, 
			sizeof(struct PVR_BLOCK_ENC_PARAM));
	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, decrypt_control);
}
static RET_CODE pvr_start_block(int cmd, void *param)
{
    UINT32 decrypt_control[] =
    {
        //desc of pointer para
        1, DESC_OUTPUT_STRU(0, sizeof(struct PVR_BLOCK_ENC_PARAM)),
        1, DESC_P_PARA(0, 1, 0),
        //desc of pointer ret
        0,
        0,
    };

	struct PVR_BLOCK_ENC_PARAM *p;
	p = (struct PVR_BLOCK_ENC_PARAM *)param;
	PVR_RPC_PRINTF("PVR_RPC_IO_START_BLOCK command is set!\n");
	p->request_res = 1;

	//p.kl_level = PVR_KL_THREE;
	DESC_OUTPUT_STRU_SET_SIZE(decrypt_control, 0, 
	sizeof(struct PVR_BLOCK_ENC_PARAM));
	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, decrypt_control);
}

static RET_CODE pvr_free_block_key(int cmd, void *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, 0),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};

	DESC_STATIC_STRU_SET_SIZE(desc, 0, 
	sizeof(struct PVR_RPC_RAW_DECRYPT));
	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, desc);
}

RET_CODE pvr_rpc_ioctl(int cmd, void *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, 0),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
    UINT32 decrypt_control[] =
    {
        //desc of pointer para
        1, DESC_OUTPUT_STRU(0, sizeof(struct PVR_BLOCK_ENC_PARAM)),
        1, DESC_P_PARA(0, 1, 0),
        //desc of pointer ret
        0,
        0,
    };
	struct PVR_BLOCK_ENC_PARAM *p = NULL;
	int ret = 0;

	if(OSAL_INVALID_ID == s_mutex_pvr)
	{
		s_mutex_pvr = osal_mutex_create();
		if(OSAL_INVALID_ID == s_mutex_pvr)
			return RET_FAILURE;
	}

	switch (cmd)
	{
		case PVR_RPC_IO_RAW_DECRYPT:
            PVR_RPC_PRINTF("PVR_RPC_IO_RAW_DECRYPT command is set!\n");
			DESC_STATIC_STRU_SET_SIZE(desc, 0, 
					sizeof(PVR_RPC_RAW_DECRYPT));
            jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, desc);
			break;
	    case PVR_RPC_IO_UPDATE_ENC_PARAMTOR:
		{
			p = (struct PVR_BLOCK_ENC_PARAM *)param;
            PVR_RPC_PRINTF("PVR_RPC_IO_UPDATE_ENC_PARAMTOR command is set!\n");
			p->kl_level = PVR_KL_THREE;
			p->request_res = 0;
			DESC_OUTPUT_STRU_SET_SIZE(decrypt_control, 0, 
					sizeof(struct PVR_BLOCK_ENC_PARAM));
            jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, decrypt_control);
            break;
	    }
        case PVR_RPC_IO_SET_BLOCK_SIZE:
		{
			unsigned long arg = (unsigned long)param;
            PVR_RPC_PRINTF("PVR_RPC_IO_SET_BLOCK_SIZE command is set!\n");
			#if 0
            if (( arg%188 != 0) && (((arg/188) % 64) != 0))
            {
                PVR_RPC_PRINTF("PVR_RPC_IO_SET_BLOCK_SIZE block size is not quantum size, failed!\n");
                ret = -1;
            }
			#endif
            pvr_g_block_size = arg;
           	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, NULL);
			break;
        }
       	case PVR_RPC_IO_FREE_BLOCK:
           PVR_RPC_PRINTF("PVR_RPC_IO_FREE_BLOCK command is set!\n");
           jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, NULL);
          break;
      	case PVR_RPC_IO_START_BLOCK:
	  		p = (struct PVR_BLOCK_ENC_PARAM *)param;
          	PVR_RPC_PRINTF("PVR_RPC_IO_START_BLOCK command is set!\n");
			p->request_res = 0;
		if(p->kl_level <=0)
			p->kl_level = sys_ic_get_kl_key_mode(p->root_key_pos);
          	DESC_STATIC_STRU_SET_SIZE(desc, 0, 
				sizeof(struct PVR_BLOCK_ENC_PARAM));
          	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, desc);

			cmd = PVR_RPC_IO_UPDATE_ENC_PARAMTOR;
			DESC_OUTPUT_STRU_SET_SIZE(decrypt_control, 0, 
					sizeof(struct PVR_BLOCK_ENC_PARAM));
            jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, decrypt_control);
          	break;
		case PVR_IO_UPDATE_ENC_PARAMTOR_EVO:
        {
			p = (struct PVR_BLOCK_ENC_PARAM *)param;
            PVR_RPC_PRINTF("PVR_RPC_IO_UPDATE_ENC_PARAMTOR command is set!\n");

            p->request_res = 1;

  			cmd = PVR_RPC_IO_UPDATE_ENC_PARAMTOR;
			ret |= pvr_update_enc_param_m(cmd,param);
			
            break;
        }
        case PVR_IO_START_BLOCK_EVO:
        {
			cmd = PVR_RPC_IO_START_BLOCK;
			ret |= pvr_start_block(cmd,param);

			cmd = PVR_RPC_IO_UPDATE_ENC_PARAMTOR;
			ret |= pvr_update_enc_param_m(cmd,param);
            break;
        }
        case PVR_IO_FREE_BLOCK_EVO:
        {
			PVR_RPC_RAW_DECRYPT cryption;
			cryption.stream_id = (int)param;
			
			cmd = PVR_RPC_IO_FREE_BLOCK;
			PVR_RPC_PRINTF("PVR_RPC_IO_FREE_BLOCK command is set!\n");
			ret = pvr_free_block_key(cmd,&cryption);
            break;
        }
        case PVR_IO_CAPTURE_DECRYPT_RES:
        {
            ret = ali_pvr_capture_decrypt_res(cmd,(long unsigned int)param);
            break;
        }
        case PVR_IO_RELEASE_DECRYPT_RES:
        {
            ret = ali_pvr_release_decrypt_res(cmd,(long unsigned int)param);
            break;
        }
        case PVR_IO_SET_DECRYPT_RES:
        {
            ret = ali_pvr_set_decrypt_res(cmd,(long unsigned int)param);
            break;
        }
        case PVR_IO_DECRYPT_EVO:
        {
            ret = ali_pvr_decrytp_raw_evo(cmd,(long unsigned int)param);
            break;
        }
	case  PVR_RPC_IO_CAPTURE_PVR_KEY:
	    DESC_STATIC_STRU_SET_SIZE(desc, 0, sizeof(PVR_KEY_PARAM));
		jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, desc);
		break;
	case PVR_RPC_IO_RELEASE_PVR_KEY:
		jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, NULL);
		break;
	case PVR_RPC_IO_PVR_PLAYBCK_SET_KEY:
		DESC_STATIC_STRU_SET_SIZE(desc, 0, 
		 		sizeof(struct PVR_BLOCK_ENC_PARAM));
        	  jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, desc);
		break;
	case PVR_RPC_IO_FREE_BLOCK_EX:
		{
			 PVR_RPC_PRINTF("PVR_RPC_IO_FREE_BLOCK_EX command is set!\n");
           		jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(2) | FUNC_PVR_RPC_IOCTL, NULL);
          		break;
		}
		break;
	default:
		return -1;
	}
    return ret;
}

RET_CODE pvr_block_aes_decrypt(p_aes_dev paesdev, UINT16 stream_id, UINT8 *input, UINT32 total_length)
{
    PVR_RPC_RAW_DECRYPT param;

    if (0 == paesdev || NULL == input || 0 == total_length)
    {
        return RET_FAILURE;
    }

	param.algo = AES;
	param.dev = paesdev;
	param.stream_id = stream_id;
	param.input = input;
	param.length = total_length;
    osal_cache_flush((void*)param.input, param.length);
    return pvr_rpc_ioctl(PVR_RPC_IO_RAW_DECRYPT, &param);
}

RET_CODE pvr_block_des_decrypt(p_des_dev pdesdev, UINT16 stream_id, UINT8 *input, UINT32 total_length)
{
    PVR_RPC_RAW_DECRYPT param;

    if (0 == pdesdev || NULL == input || 0 == total_length)
    {
        return RET_FAILURE;
    }

	param.algo = DES;
	param.dev = pdesdev;
	param.stream_id = stream_id;
	param.input = input;
	param.length = total_length;
    osal_cache_flush((void *)param.input, param.length);
    return pvr_rpc_ioctl(PVR_RPC_IO_RAW_DECRYPT, &param);
}

RET_CODE sca_cw_pos_generate(UINT8 datalen, const UINT8 *pure_data, UINT32 *keypos)
{
    UINT32 cw_pos_gen[] =
    {
        //desc of pointer para
        1, DESC_OUTPUT_STRU(0, sizeof(UINT32)),
        1, DESC_P_PARA(0, 2, 0),
        //desc of pointer ret
        0,
        0,
    };
	
	osal_cache_flush((void *)pure_data, datalen);
	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(3) | FUNC_SCA_CW_POS_GEN, cw_pos_gen);
	osal_cache_invalidate((void *)keypos, sizeof(UINT32));
}

RET_CODE sca_cw_pos_release(UINT32 key_pos)
{
	jump_to_func(NULL, os_hld_caller, NULL, PVR_RPC_NPARA(1) | FUNC_SCA_CW_POS_REL, NULL);
}
RET_CODE DVTSTBCA_Generate_PureData()
{
    /* 3823 use
	BYTE EnData[144] = 
	{
		0xb8,0xfa,0x90,0x6a,0xf8,0x2d,0x64,0xed,0xb6,0x62,0xb8,0x55,0x94,0xc3,0xe2,0x2f,
		0xeb,0xcf,0x4a,0x12,0x60,0x3e,0x56,0x14,0xeb,0xcf,0x4a,0x12,0x60,0x3e,0x56,0x14,
		0xc5,0xf6,0x7f,0x5a,0x37,0x9a,0x77,0xac,0xf9,0xf7,0xec,0xb2,0xca,0x70,0x8d,0x0f,
		0xac,0xdc,0x42,0x11,0xac,0x71,0xf5,0x4f,0x47,0x3e,0x5f,0x7b,0x0e,0x90,0x22,0xb8,
		0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,
		0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,
		0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,0xdc,0x71,0x7c,0xe4,0xa1,0x32,0xc1,0x5d,
		0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,0xdc,0x58,0x7c,0xa8,0xb9,0x6e,0x22,0x5b,
		0x64,0x13,0x7a,0x08,0xbe,0x2c,0x76,0x79,0x74,0x49,0xb2,0xd9,0xff,0x46,0x3b,0x8c,
	};
*/
/*
    //A3510L use
    BYTE EnData[144] =
    {
        0x44,0x48,0xc7,0x96,0x74,0x67,0x9a,0xcf,0xf0,0xbd,0x7c,0xbd,0xab,0x3f,0x1e,0x9c,
        0xd5,0x8d,0x2a,0x2a,0x27,0x6b,0xbf,0x8f,0xd5,0x8d,0x2a,0x2a,0x27,0x6b,0xbf,0x8f,
        0xae,0x9e,0x23,0x0d,0x30,0x50,0x61,0x15,0x7e,0xc2,0xee,0x4a,0x89,0xb4,0x13,0x43,
        0xa1,0xe0,0x31,0xcb,0x8b,0x51,0xd3,0x48,0x01,0xe5,0xb3,0xf9,0xba,0x25,0x28,0xa2,
        0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,0x22,0xad,0x17,0xc2,0x8c,0x00,0x97,0x8c,
        0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,
        0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,
        0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,0x37,0x55,0xea,0xd1,0x38,0xf5,0xb7,0x3f,
        0x29,0xa5,0xaa,0xd2,0x42,0x08,0xdb,0x28,0xdb,0x67,0xab,0x6c,0x6d,0x89,0xa5,0xa1,
    };*/

BYTE EnData[144] =
    {
        0x30,0x84,0x8c,0x5b,0x66,0x53,0x15,0x22,0x2d,0xe3,0xee,0x38,0x47,0xc7,0xf1,0x62,
0x52,0xdf,0xe1,0xeb,0x21,0x8b,0x23,0x7f,0x5d,0x0b,0xe5,0x9b,0xf8,0x90,0x1d,0x1f,
0xba,0x19,0x33,0xe6,0xfd,0xe7,0x1b,0xa2,0x0c,0xe3,0x3e,0xa1,0x78,0xb0,0xc5,0x60,
0xd2,0xc4,0xf5,0x77,0x86,0x1c,0xb2,0xcf,0x2c,0x9e,0xb7,0xbd,0x81,0xaf,0x6f,0x9e,
0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,0xd4,0xb4,0x7c,0x07,0xb8,0xe9,0xcd,0x5e,
0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,
0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,
0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,0x69,0x4b,0x72,0x98,0xd6,0x67,0x98,0x3d,
0x4c,0x59,0x19,0x67,0x05,0xf0,0xfb,0x40,0x7a,0xec,0x66,0x89,0xdc,0x91,0xd4,0xa9,
    };
	RET_CODE ret = RET_FAILURE;
	UINT32 key_pos = 0xff;

	libc_printf("CPU: &key_pos=%p\n", &key_pos);
	ret = sca_cw_pos_generate(144, EnData, &key_pos);
	if (ret != RET_SUCCESS)
	{
		libc_printf("\nError is %d\n", ret);
	}
	libc_printf("CPU: CW key_pos=%d\n", key_pos);

	libc_printf("\n");
	sca_cw_pos_release(key_pos);

	return ret;
}


#endif	/*_HLD_PVR_REMOTE*/

#endif	/*DUAL_ENABLE*/

