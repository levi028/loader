/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dsc_remote.c
*
*    Description: This file provides Descramble Scramble Core (DSC) interface.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
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
#include <api/librsa/flash_cipher.h>
#include <osal/osal.h>
#include <asm/chip.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#include <hld/trng/trng.h>

#define SHA_MODE_CHECK(mode)    (((mode) != SHA_SHA_1) && \
        ((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
        ((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))
#define DSC_NPARA(x) ((HLD_DSC_MODULE<<24)|(x<<16))

//#define DSC_REMOTE_API_DEBUG
#ifdef DSC_REMOTE_API_DEBUG
#define DSC_REMOTE_API_PRINTF libc_printf
#define DSC_REMOTE_API_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            DSC_REMOTE_API_PRINTF("0x%x,",*(data+i)); \
            if(0==((i+1)%16)) \
                DSC_REMOTE_API_PRINTF("\n");\
        }\
    }while(0)
#else
#define DSC_REMOTE_API_PRINTF(...)  do{}while(0)
#define DSC_REMOTE_API_DUMP(...)  do{}while(0)
#endif

#ifdef DUAL_ENABLE
#ifdef MAIN_CPU
static void dsc_main_cache_clean(UINT32 in, UINT32 data_length)
{
    if((0 == in) || (0 == data_length))
    {
        return;
    }
    if ( !osal_dual_is_private_address ( (void *)in ) )
    {
        osal_cache_flush ( (void *)in, data_length );
        osal_cache_invalidate ( (void *)in, data_length );
    }
}
#endif
#endif


static void dsc_main_cache_flush(__attribute__((unused)) UINT32 in, __attribute__((unused)) UINT32 out, __attribute__((unused)) UINT32 data_length)
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
        dsc_main_cache_clean( out, data_length );
    }
#endif
#endif
}

static void dsc_main_cache_inv(__attribute__((unused)) UINT32 out, __attribute__((unused)) UINT32 data_length)
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

#ifdef DUAL_ENABLE
#include <modules.h>

enum HLD_DSC_FUNC
{
    /*TRNG*/
    FUNC_TRNG_GENERATE_BYTE = 0,
    FUNC_TRNG_GENERATE_64BITS,
    FUNC_TRNG_SEE_GET64BIT,
    /*DES*/
    FUNC_DES_DECRYPT,
    FUNC_DES_ENCRYPT,
    FUNC_DES_IOCTL,
    /*AES*/
    FUNC_AES_DECRYPT,
    FUNC_AES_ENCRYPT,
    FUNC_AES_IOCTL,
    /*CSA*/
    FUNC_CSA_DECRYPT,
    FUNC_CSA_IOCTL,
    /*SHA*/
    FUNC_SHA_DIGEST,
    FUNC_SHA_IOCTL,

    FUNC_DSC_ATTACH,
    FUNC_DSC_DETACH,
    FUNC_DSC_IOCTL,

    FUNC_TRIG_RAM_MON,
    FUNC_DEENCRYPT,
    FUNC_GET_FREE_STREAM_ID,
    FUNC_SET_STREAM_ID_IDLE,
    FUNC_GET_FREE_SUB_DEVICE_ID,
    FUNC_SET_SUB_DEVICE_ID_IDLE,
    FUNC_SET_STREAM_ID_USED,
    FUNC_SET_SUB_DEVICE_ID_USED,
    FUNC_DEAL_QUANTUM_MIXED_TS,

	FUNC_GET_IDLE_PID_SRAM_NUM,
	FUNC_GET_IDLE_KEY_SRAM_NUM,
	FUNC_GET_FREE_SUB_DEVICE_NUM,
	FUNC_GET_FREE_STREAM_ID_NUM,
};

#ifndef _HLD_DSC_REMOTE
static UINT32 hld_dsc_entry[] =
{
    (UINT32)trng_generate_byte,
    (UINT32)trng_generate_64bits,
    (UINT32)trng_get_64bits,

    (UINT32)des_decrypt_rpc,
    (UINT32)des_encrypt_rpc,
    (UINT32)des_ioctl,

    (UINT32)aes_decrypt_rpc,
    (UINT32)aes_encrypt_rpc,
    (UINT32)aes_ioctl,

    (UINT32)csa_decrypt_rpc,
    (UINT32)csa_ioctl,

    (UINT32)sha_digest_rpc,
    (UINT32)sha_ioctl,

    (UINT32)dsc_api_attach,
    (UINT32)dsc_api_detach,
    (UINT32)dsc_ioctl_rpc,

    (UINT32)trig_ram_mon,
    (UINT32)de_encrypt,
    (UINT32)dsc_get_free_stream_id,
    (UINT32)dsc_set_stream_id_idle,
    (UINT32)dsc_get_free_sub_device_id,
    (UINT32)dsc_set_sub_device_id_idle,
    (UINT32)dsc_set_stream_id_used,
    (UINT32)dsc_set_sub_device_id_used,
    (UINT32)dsc_deal_quantum_for_mixed_ts,

	(UINT32)dsc_get_idle_pid_sram_num,
	(UINT32)dsc_get_idle_key_sram_num,
	(UINT32)dsc_get_free_sub_device_num,
	(UINT32)dsc_get_free_stream_id_num,
};

static UINT32 dsc_m36_entry[] =
{
};

void hld_dsc_callee(UINT8 *msg)
{
    if(NULL== msg)
    {
        return;
    }
    os_hld_callee((UINT32)hld_dsc_entry, msg);
}

/* just define the function handle, do not use this */
void lld_dsc_m36f_callee( UINT8 *msg )
{
    if(NULL== msg)
    {
        return;
    }
    os_hld_callee((UINT32)dsc_m36_entry, msg);
}
void lld_trng_m36f_callee( UINT8 *msg )
{
    if(NULL== msg)
    {
        return;
    }
    os_hld_callee((UINT32)dsc_m36_entry, msg);
}


/*
*High level API on SEE side for software version check
*/
RET_CODE see_version_check(UINT32 block_id, UINT32 block_addr, UINT32 block_len)
{
    RET_CODE ret = RET_FAILURE;
    p_dsc_dev pdscdev = NULL;
    DSC_VER_CHK_PARAM ver_param;

    pdscdev = ( p_dsc_dev ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
    if(NULL != pdscdev)
    {
        MEMSET(&ver_param, 0, sizeof(DSC_VER_CHK_PARAM));
        ver_param.chk_id = block_id ;
        ver_param.input_mem = block_addr | 0xa0000000;
        ver_param.len = block_len ;
        ver_param.chk_mode =   CHECK_END  ;

        ret = dsc_ioctl(pdscdev, IO_DSC_VER_CHECK, (UINT32)&ver_param);
        return ret ;
    }
    return RET_SUCCESS;
}

#endif  /*_HLD_DSC_REMOTE*/

#ifdef _HLD_DSC_REMOTE
static UINT32 des_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 aes_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 csa_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 sha_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 dsc_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

#if 0
static UINT32 dsc_io_control_1[] =
{
    //desc of pointer para
    9, DESC_OUTPUT_STRU(0, sizeof(KEY_PARAM)), DESC_STATIC_STRU(1, 0), \
    DESC_STATIC_STRU(2, 4 * sizeof(AES_KEY_PARAM)), DESC_STATIC_STRU(3, 4 * sizeof(CSA_KEY_PARAM)), \
    DESC_STATIC_STRU(4, 4 * sizeof(DES_KEY_PARAM)), DESC_STATIC_STRU(5, 4 * sizeof(AES_IV_INFO)), \
    DESC_STATIC_STRU(6, 4 * sizeof(DES_IV_INFO)), DESC_STATIC_STRU(7, 16), DESC_STATIC_STRU(8, 16),
    9, DESC_P_PARA(0, 2, 0), DESC_P_STRU(1, 0, 1, offsetof(KEY_PARAM, pid_list)), \
    DESC_P_STRU(1, 0, 2, offsetof(KEY_PARAM, p_aes_key_info)),
    DESC_P_STRU(1, 0, 3, offsetof(KEY_PARAM, p_csa_key_info)), \
    DESC_P_STRU(1, 0, 4, offsetof(KEY_PARAM, p_des_key_info)),
    DESC_P_STRU(1, 0, 5, offsetof(KEY_PARAM, p_aes_iv_info)), \
    DESC_P_STRU(1, 0, 6, offsetof(KEY_PARAM, p_des_iv_info)),
    DESC_P_STRU(1, 0, 7, offsetof(KEY_PARAM, init_vector)), \
    DESC_P_STRU(1, 0, 8, offsetof(KEY_PARAM, ctr_counter)),
    //desc of pointer ret
    0,
    0,
};
#endif

static UINT32 sha_crypt_control[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 64),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

UINT32 dsc_bl_ctrl[] =
{
    //desc of pointer para
    2, DESC_OUTPUT_STRU(0, sizeof(DSC_BL_UK_PARAM)), DESC_OUTPUT_STRU(1, 16),
    2, DESC_P_PARA(0, 2, 0),
    DESC_P_STRU(1, 0, 1, offsetof(DSC_BL_UK_PARAM, output_key)),
    //desc of pointer ret
    0,
    0,
};

void insert_one_desc_size_item(UINT32 *desc, UINT32 idx, UINT32 size, UINT32 off, UINT32 a)
{
	UINT32 cur_offset = 0;
	
	//increase the paramter counter...
	desc[0] = desc[0] + 1;


	//set values...
	cur_offset = 1 + idx*3;
	
	desc[cur_offset] = size;
	desc[cur_offset+1] = off;
	desc[cur_offset+2] = a;
}

void insert_one_desc_offset_item(UINT32 *desc, UINT32 idx, UINT32 offset)
{
	UINT32 cur_offset = 0;

	cur_offset = 1 + desc[0] * 3;
	
	if (0 == idx)
		desc[cur_offset] = desc[0];

	desc[1 + cur_offset + idx] = offset;
}

#define INSERT_ONE_DESC_ITEM(name, size)  do {\
	if (key_param->name) { \
		i++; \
		if (0 == loop) { \
			insert_one_desc_size_item( desc, i, DESC_STATIC_STRU(i, size) ); \
		} else { \
			insert_one_desc_offset_item(desc, i, DESC_P_STRU(1, 0, i, offsetof(KEY_PARAM, name))); \
		} \
	} \
} while(0);

RET_CODE key_param_rpc_desc_constructor(UINT32 *desc, KEY_PARAM *key_param)
{
	UINT32 i = 0, loop = 0;
	if ((NULL == desc) || (NULL == key_param))
		return RET_FAILURE;

	//init paramter counter to zero.
	desc[0] = 0;
	
	for(loop = 0; loop < 2; loop++)
	{
		i = 0;

		if (0 == loop) {
			insert_one_desc_size_item( desc, i, DESC_OUTPUT_STRU(0, sizeof(KEY_PARAM)) );
		} else {
			insert_one_desc_offset_item(desc, i, DESC_P_PARA(0, 2, 0));
		}
		
		if ((0 == loop) && (0 == key_param->pid_len))
			key_param->pid_len = 1;
		
		INSERT_ONE_DESC_ITEM(pid_list, sizeof(UINT16)*key_param->pid_len);
		INSERT_ONE_DESC_ITEM(p_aes_key_info, sizeof(AES_KEY_PARAM));
		INSERT_ONE_DESC_ITEM(p_csa_key_info, sizeof(CSA_KEY_PARAM));
		INSERT_ONE_DESC_ITEM(p_des_key_info, sizeof(DES_KEY_PARAM));
		INSERT_ONE_DESC_ITEM(p_aes_iv_info, sizeof(AES_IV_INFO));
		INSERT_ONE_DESC_ITEM(p_des_iv_info, sizeof(DES_IV_INFO));
		INSERT_ONE_DESC_ITEM(init_vector, 16);
		INSERT_ONE_DESC_ITEM(ctr_counter, 16);
	}

	desc[2 + 4 * (i+1)] = 0;
	desc[3 + 4 * (i+1)] = 0;
	return RET_SUCCESS;
}


RET_CODE dsc_api_attach_ex( void )
{
    jump_to_func(NULL, os_hld_caller, NULL, DSC_NPARA(0) | FUNC_DSC_ATTACH, NULL);
}

RET_CODE dsc_api_detach_ex( void )
{
    jump_to_func(NULL, os_hld_caller, NULL, DSC_NPARA(0) | FUNC_DSC_DETACH, NULL);
}

RET_CODE dsc_api_attach( void )
{
    RET_CODE ret = RET_SUCCESS;
    DSC_DEV *p_dsc_dev = ( DSC_DEV *) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
    if(!p_dsc_dev)
    {
        ret = dsc_api_attach_ex();
        run_time_integrity_check();
        return ret;
    }
    return ALI_DSC_WARNING_DRIVER_ALREADY_INITIALIZED;
}

RET_CODE dsc_api_detach( void )
{
    return dsc_api_detach_ex();
}

RET_CODE trng_generate_byte( UINT8 *data )
{
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 1), 1, DESC_P_PARA(0, 0, 0), 0, 0,
    };
    jump_to_func(NULL, os_hld_caller, data, DSC_NPARA(1) | FUNC_TRNG_GENERATE_BYTE, desc);
}

RET_CODE trng_generate_64bits( UINT8 *data )
{
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 8), 1, DESC_P_PARA(0, 0, 0), 0, 0,
    };
    jump_to_func(NULL, os_hld_caller, data, DSC_NPARA(1) | FUNC_TRNG_GENERATE_64BITS, desc);
}

static RET_CODE trng_get_64bits_ex(UINT8 *data, UINT32 n)
{
    if(0 == n)
    {
        return RET_FAILURE;
    }
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 0), 1, DESC_P_PARA(0, 0, 0), 0, 0,
    };
    DESC_OUTPUT_STRU_SET_SIZE(desc, 0, (ALI_TRNG_64BITS_SIZE * n));
    jump_to_func(NULL, os_hld_caller, data, DSC_NPARA(2) | FUNC_TRNG_SEE_GET64BIT, desc);
}

#if 0
RET_CODE trng_get_64bits(UINT8 *data, UINT32 n)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 trng_c = 0;
    UINT32 deal_c = 0;

    if((0 == n) || (NULL == data))
    {
        return RET_FAILURE;
    }

    while(deal_c < n)
    {
        trng_c = (n-deal_c)>ALI_TRNG_MAX_GROUP?ALI_TRNG_MAX_GROUP:(n-deal_c);
        ret = trng_get_64bits_ex(data+ALI_TRNG_64BITS_SIZE*deal_c,(UINT32)trng_c);
        if (RET_SUCCESS != ret)
        {
            return ret;
        }
        deal_c += trng_c;
    }

    return ret;
}
#else
/*patch for vmx BC_Init() slowly. reduce the continual rpc call*/
static UINT8 trng_data[ALI_TRNG_MAX_GROUP][ALI_TRNG_64BITS_SIZE];
static UINT32 trng_index=ALI_TRNG_MAX_GROUP;
static ID trng_mutex=INVALID_ID;
RET_CODE trng_get_64bits(UINT8 *data, UINT32 n)
{
    RET_CODE ret = RET_SUCCESS;
    UINT32 i;
    if((0 == n) || (NULL == data))
    {
        return RET_FAILURE;
    }
    if(trng_mutex == INVALID_ID)
    {
        trng_mutex = osal_mutex_create();
    }
    osal_mutex_lock(trng_mutex, OSAL_WAIT_FOREVER_TIME);
    for(i=0; i<n; i++)
    {
        if(trng_index>=ALI_TRNG_MAX_GROUP)
        {
            ret = trng_get_64bits_ex((UINT8 *)trng_data,ALI_TRNG_MAX_GROUP);
            if (RET_SUCCESS != ret)
            {
                osal_mutex_unlock(trng_mutex);
                return ret;
            }
            trng_index = 0;
        }
        MEMCPY(data+i*ALI_TRNG_64BITS_SIZE, trng_data[trng_index], ALI_TRNG_64BITS_SIZE);
        trng_index++;
    }
    osal_mutex_unlock(trng_mutex);
    return ret;
}
#endif

static RET_CODE des_decrypt_ex(p_des_dev pdesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    jump_to_func(NULL, os_hld_caller, pdesdev, DSC_NPARA(5) | FUNC_DES_DECRYPT, NULL);
}

static RET_CODE des_encrypt_ex(p_des_dev pdesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    jump_to_func(NULL, os_hld_caller, pdesdev, DSC_NPARA(5) | FUNC_DES_ENCRYPT, NULL);
}

RET_CODE des_decrypt(p_des_dev pdesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || (NULL == pdesdev))
    {
        return RET_FAILURE;
    }

    if(( dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        if(total_length > TS_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    }
    else
    {
        if(total_length > PURE_DATA_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length);
    }

    ret = des_decrypt_ex(pdesdev, dmx_id, input, output, total_length);

    if((dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        dsc_main_cache_inv((UINT32)output, total_length * 188);
    }
    else
    {
        dsc_main_cache_inv((UINT32)output, total_length);
    }

    return ret;
}


RET_CODE des_encrypt(p_des_dev pdesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || (NULL == pdesdev))
    {
        return RET_FAILURE;
    }

    if(( dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        if(total_length > TS_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    }
    else
    {
        if(total_length > PURE_DATA_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length);
    }

    ret = des_encrypt_ex(pdesdev, dmx_id, input, output, total_length);

    if((dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        dsc_main_cache_inv((UINT32)output, total_length * 188);
    }
    else
    {
        dsc_main_cache_inv((UINT32)output, total_length);
    }

    return ret;
}

RET_CODE des_ioctl(DES_DEV *pdesdev ,UINT32 cmd , UINT32 param)
{
    UINT32 i = 0;
    UINT32 common_desc[40]={0};
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)des_io_control;
    //KEY_PARAM *param_tmp;

    if(NULL == pdesdev)
    {
        return RET_FAILURE;
    }

    for(i = 0; i < sizeof(des_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }
    switch(cmd)
    {
        case IO_INIT_CMD:
        {
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DES_INIT_PARAM));
        }
        break ;

        case IO_CREAT_CRYPT_STREAM_CMD :
        case IO_KEY_INFO_UPDATE_CMD:
        {
			key_param_rpc_desc_constructor(desc, (KEY_PARAM *)param);

			#if 0
            /*has return value*/
            b = (UINT32 *)dsc_io_control_1;
            param_tmp = (KEY_PARAM *)param;
            if(NULL == param_tmp)
            {
                return RET_FAILURE;
            }
            for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
            {
                desc[i] = b[i];
            }

            if(0 == param_tmp->pid_len)
            {
                param_tmp->pid_len = 1; //at least transfer - one set of key
            }

            for(i=0; i<3; i++)
            {
                DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
            }
            DESC_STATIC_STRU_SET_SIZE(desc, 6, 2*param_tmp->pid_len*8);
            DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
			#endif
        }
        break;

        case IO_DELETE_CRYPT_STREAM_CMD:
        {
            desc = NULL;
        }
        break;

        case IO_ADD_DEL_PID:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(PID_PARAM));
            break;
            
        case IO_PARAM_INFO_UPDATE:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(IV_OTHER_PARAM));
            break;

        default:
            if(sys_ic_get_chip_id() >= ALI_S3602F)
            {
                return RET_FAILURE;
            }
            break;
    }

    jump_to_func(NULL, os_hld_caller, pdesdev, DSC_NPARA(3)|FUNC_DES_IOCTL, desc);
}

static RET_CODE aes_encrypt_ex(p_aes_dev paesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    jump_to_func(NULL, os_hld_caller, paesdev, DSC_NPARA(5) | FUNC_AES_ENCRYPT, NULL);
}


static RET_CODE aes_decrypt_ex(p_aes_dev paesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    jump_to_func(NULL, os_hld_caller, paesdev, DSC_NPARA(5) | FUNC_AES_DECRYPT, NULL);
}

RET_CODE aes_decrypt(p_aes_dev paesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || (NULL == paesdev))
    {
        return RET_FAILURE;
    }

    if(( dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        if(total_length > TS_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    }
    else
    {
        if(total_length > PURE_DATA_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length);
    }

    ret = aes_decrypt_ex(paesdev, dmx_id, input, output, total_length);

    if((dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        dsc_main_cache_inv((UINT32)output, total_length * 188);
    }
    else
    {
        dsc_main_cache_inv((UINT32)output, total_length);
    }

    return ret;
}

RET_CODE aes_encrypt(p_aes_dev paesdev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || (NULL == paesdev))
    {
        return RET_FAILURE;
    }

    if(( dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        if(total_length > TS_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    }
    else
    {
        if(total_length > PURE_DATA_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length);
    }

    ret = aes_encrypt_ex(paesdev, dmx_id, input, output, total_length);

    if((dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        dsc_main_cache_inv((UINT32)output, total_length * 188);
    }
    else
    {
        dsc_main_cache_inv((UINT32)output, total_length);
    }

    return ret;
}

RET_CODE aes_ioctl( AES_DEV *paesdev ,UINT32 cmd , UINT32 param)
{
    UINT32 i=0;
    UINT32 common_desc[40]={0};
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)aes_io_control;
    //KEY_PARAM *param_tmp;
    AES_CRYPT_WITH_FP* aes_fp;

    if( NULL == paesdev)
    {
        return RET_FAILURE;
    }

    for(i = 0; i < sizeof(aes_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }
    switch(cmd)
    {
        case IO_INIT_CMD:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(AES_INIT_PARAM));
            break ;
        case IO_CREAT_CRYPT_STREAM_CMD :
        case IO_KEY_INFO_UPDATE_CMD:
        {
			key_param_rpc_desc_constructor(desc, (KEY_PARAM *)param);

			#if 0
            /*has return value*/
            b = (UINT32 *)dsc_io_control_1;
            param_tmp = (KEY_PARAM *)param;
            if(NULL == param_tmp)
            {
                 return RET_FAILURE;
            }
            for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
            {
                desc[i] = b[i];
            }

            if(0 == param_tmp->pid_len)
            {
                param_tmp->pid_len = 1; //at least transfer - one set of key
            }

            for(i=0; i<3; i++)
            {
                DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
            }
            DESC_STATIC_STRU_SET_SIZE(desc, 5, 2*param_tmp->pid_len*16);
            DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
			#endif
        }
        break;

        case IO_DELETE_CRYPT_STREAM_CMD:
            desc = NULL;
            break;

        case IO_ADD_DEL_PID:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(PID_PARAM));
            break;
            
        case IO_PARAM_INFO_UPDATE:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(IV_OTHER_PARAM));
            break;

		case IO_AES_CRYPT_WITH_FP:
			if(0 == param)
            {
                 return RET_FAILURE;
            }
            aes_fp = (AES_CRYPT_WITH_FP*)param;
            dsc_main_cache_flush((UINT32)(aes_fp->input), (UINT32)(&(aes_fp->output[0])), 16);
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(AES_CRYPT_WITH_FP));
			break;

        default:
            if(sys_ic_get_chip_id() >= ALI_S3602F)
            {
                return RET_FAILURE;
            }
            break;
    }
    jump_to_func(NULL, os_hld_caller, paesdev, DSC_NPARA(3)|FUNC_AES_IOCTL, desc);
}

static RET_CODE csa_decrypt_ex(p_csa_dev pcsadev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    jump_to_func(NULL, os_hld_caller, pcsadev, DSC_NPARA(5) | FUNC_CSA_DECRYPT, NULL);
}

RET_CODE csa_decrypt(p_csa_dev pcsadev, UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || (NULL == pcsadev))
    {
        return RET_FAILURE;
    }

    if(( dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        if(total_length > TS_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    }
    else
    {
        if(total_length > PURE_DATA_MAX_SIZE)
        {
            return ret;
        }
        dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length);
    }

    ret = csa_decrypt_ex(pcsadev, dmx_id, input, output, total_length);

    if((dmx_id <= 0x3) || (dmx_id >= 0x8 && dmx_id <= 0xf))
    {
        dsc_main_cache_inv((UINT32)output, total_length * 188);
    }
    else
    {
        dsc_main_cache_inv((UINT32)output, total_length);
    }

    return ret;
}

RET_CODE csa_ioctl( CSA_DEV *pcsadev, UINT32 cmd, UINT32 param)
{
    UINT32 i = 0;
    UINT32 common_desc[40] = {0};
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)csa_io_control;
    //KEY_PARAM *param_tmp;

    if( NULL == pcsadev)
    {
        return RET_FAILURE;
    }

    for(i = 0; i < sizeof(csa_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }
    switch(cmd)
    {
        case IO_INIT_CMD:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CSA_INIT_PARAM ));
            break;
        case IO_CREAT_CRYPT_STREAM_CMD :
        case IO_KEY_INFO_UPDATE_CMD:
        {
			key_param_rpc_desc_constructor(desc, (KEY_PARAM *)param);

			#if 0
            /*has return value*/
            b = (UINT32 *)dsc_io_control_1;
            param_tmp = (KEY_PARAM *)param;
            if(NULL == param_tmp)
            {
                 return RET_FAILURE;
            }
            for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
            {
                desc[i] = b[i];
            }

            if(0 == param_tmp->pid_len)
            {
                param_tmp->pid_len = 1; //at least transfer - one set of key
            }

            for(i=0; i<3; i++)
            {
                DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
            }
            DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
			#endif
        }
        break;

        case IO_DELETE_CRYPT_STREAM_CMD:
            desc = NULL;
            break;

        case IO_ADD_DEL_PID:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(PID_PARAM));
            break;
            
        case IO_PARAM_INFO_UPDATE:
            if(0 == param)
            {
                 return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(IV_OTHER_PARAM));
            break;

        default:
            if(sys_ic_get_chip_id() >= ALI_S3602F)
            {
                return RET_FAILURE;
            }
            break;
    }
    jump_to_func(NULL, os_hld_caller, pcsadev, DSC_NPARA(3)|FUNC_CSA_IOCTL, desc);
}


RET_CODE sha_ioctl( p_sha_dev pshadev , UINT32 cmd , UINT32 param)
{
    UINT32 i = 0;
    UINT32 common_desc[sizeof(sha_io_control)];
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)sha_io_control;

    if( NULL == pshadev)
    {
        return RET_FAILURE;
    }

    for(i = 0; i < sizeof(sha_io_control) / sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }

    switch(cmd)
    {
        case IO_INIT_CMD:
        {
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(SHA_INIT_PARAM ));
        }
        break;

        default:
            SDBBP();
            break ;
    }

    jump_to_func(NULL, os_hld_caller, pshadev, DSC_NPARA(3) | FUNC_SHA_IOCTL, desc);
}

static RET_CODE sha_digest_ex(p_sha_dev pshadev, UINT8 *input, UINT8 *output, UINT32 data_length)
{
    jump_to_func(NULL, os_hld_caller, pshadev, DSC_NPARA(4) | FUNC_SHA_DIGEST, sha_crypt_control);
}


RET_CODE sha_digest(p_sha_dev pshadev, UINT8 *input, UINT8 *output, UINT32 data_length)
{
    RET_CODE ret = RET_FAILURE;
    if((data_length > PURE_DATA_MAX_SIZE) ||( NULL == input) || (NULL == output) || (NULL == pshadev))
    {
        return ret;
    }
    dsc_main_cache_flush((UINT32)input, (UINT32)input, data_length);
    ret = sha_digest_ex(pshadev, input, output, data_length);
    return ret;
}

static RET_CODE dsc_ioctl_set_pvr_key_ex( p_dsc_dev pdscdev , UINT32 cmd , UINT32 param)
{
    UINT32 i = 0;
    UINT32 common_desc[40] = {0};
    UINT32 *b = (UINT32 *)dsc_io_control;
    UINT32 *desc = (UINT32 *)common_desc;

    for(i = 0; i < sizeof(dsc_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }

    DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_PVR_KEY_PARAM));

    jump_to_func(NULL, os_hld_caller, pdscdev, DSC_NPARA(3) | FUNC_DSC_IOCTL, desc);
}

RET_CODE dsc_ioctl( p_dsc_dev pdscdev , UINT32 cmd , UINT32 param)
{
    UINT32 i = 0;
    UINT32 common_desc[40] = {0};
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)dsc_io_control;
    DSC_PVR_KEY_PARAM *pvr_k = NULL;

    if( NULL == pdscdev)
    {
        return RET_FAILURE;
    }

    for(i = 0; i < sizeof(dsc_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }

    switch(cmd)
    {
        case IO_PARSE_DMX_ID_GET_CMD:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
            break;
        case IO_PARSE_DMX_ID_SET_CMD:
            desc = NULL;
            break;
        case IO_DSC_GET_DES_HANDLE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
            break;
        case IO_DSC_GET_AES_HANDLE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
            break;
        case IO_DSC_GET_CSA_HANDLE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
            break;
        case IO_DSC_GET_SHA_HANDLE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
            break;
        case IO_DSC_SET_PVR_KEY_PARAM:
            pvr_k = (DSC_PVR_KEY_PARAM *)param;
            if(NULL == pvr_k)
            {
                return RET_FAILURE;
            }
            osal_cache_flush((UINT8 *)pvr_k->input_addr,pvr_k->valid_key_num*pvr_k->pvr_key_length/8);
            return dsc_ioctl_set_pvr_key_ex(pdscdev, cmd, param);
        case IO_DSC_VER_CHECK:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_VER_CHK_PARAM));
            break;
        case IO_DSC_ENCRYTP_BL_UK:
            b = (UINT32 *)dsc_bl_ctrl;
            for(i = 0; i < sizeof(dsc_bl_ctrl)/sizeof(UINT32); i++)
            {
                desc[i] = b[i];
            }
            break;  
        case IO_DSC_SET_PVR_KEY_IDLE:
            desc = NULL;
            break;
        case IO_DSC_SET_ENCRYPT_PRIORITY:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_EN_PRIORITY));
            break;
        case IO_DSC_GET_DRIVER_VERSION:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 20);
            break;
        case IO_DSC_SET_CLR_CMDQ_EN:
            desc = NULL;
            break;
        case IO_DSC_DELETE_HANDLE_CMD:
            desc = NULL;
            break;
        case IO_DSC_FIXED_DECRYPTION:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_FIXED_CRYPTION));
            break;
        case IO_DSC_SYS_UK_FW:
            if(0 == param)
            {
                return RET_FAILURE;
            }
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_SYS_UK_FW));
            break;
        default:
            break;
    }

    jump_to_func(NULL, os_hld_caller, pdscdev, DSC_NPARA(3) | FUNC_DSC_IOCTL, desc);
}

static RET_CODE trig_ram_mon_ex(UINT32 start_addr, UINT32 end_addr, UINT32 interval, enum SHA_MODE sha_mode,
                          BOOL disableorenable)
{
    jump_to_func(NULL, os_hld_caller, start_addr, DSC_NPARA(5) | FUNC_TRIG_RAM_MON, NULL);
}

RET_CODE trig_ram_mon(UINT32 start_addr, UINT32 end_addr, UINT32 interval,
            enum SHA_MODE sha_mode, BOOL disableorenable)
{
    return  trig_ram_mon_ex(start_addr, end_addr, interval, sha_mode, disableorenable);
}

static RET_CODE de_encrypt_ex(DEEN_CONFIG *p_deen, UINT8 *input, UINT8 *output , UINT32 total_length)
{
    UINT32 desc[] = {1, DESC_STATIC_STRU(0, sizeof(DEEN_CONFIG)), 1, DESC_P_PARA(0, 0, 0), 0, 0,};
    jump_to_func(NULL, os_hld_caller, p_deen, DSC_NPARA(4) | FUNC_DEENCRYPT, desc);
}

RET_CODE de_encrypt(DEEN_CONFIG *p_deen, UINT8 *input, UINT8 *output , UINT32 total_length)
{
    RET_CODE ret = RET_FAILURE;

    if((total_length > TS_MAX_SIZE)||( NULL == input) || (NULL == output) || (NULL == p_deen))
    {
        return ret;
    }

    dsc_main_cache_flush((UINT32)input, (UINT32)output, total_length * 188);
    ret = de_encrypt_ex(p_deen, input, output, total_length);
    dsc_main_cache_inv((UINT32)output, total_length * 188);

    return ret;
}
UINT16 dsc_get_free_stream_id(enum DMA_MODE dma_mode)
{
    jump_to_func(NULL, os_hld_caller, dma_mode, DSC_NPARA(1) | FUNC_GET_FREE_STREAM_ID, NULL);
}
UINT32 dsc_get_free_sub_device_id(enum WORK_SUB_MODULE sub_mode)
{
    jump_to_func(NULL, os_hld_caller, sub_mode, DSC_NPARA(1) | FUNC_GET_FREE_SUB_DEVICE_ID, NULL);
}
RET_CODE dsc_set_sub_device_id_idle(enum WORK_SUB_MODULE sub_mode, UINT32 device_id)
{
    jump_to_func(NULL, os_hld_caller, sub_mode, DSC_NPARA(2) | FUNC_SET_SUB_DEVICE_ID_IDLE, NULL);
}
RET_CODE dsc_set_stream_id_idle(UINT32 pos)
{
    jump_to_func(NULL, os_hld_caller, pos, DSC_NPARA(1) | FUNC_SET_STREAM_ID_IDLE, NULL);
}

void dsc_set_stream_id_used(UINT32 pos)
{
    jump_to_func(NULL, os_hld_caller, pos, DSC_NPARA(1) | FUNC_SET_STREAM_ID_USED, NULL);
}
RET_CODE dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode, UINT32 device_id)
{
    jump_to_func(NULL, os_hld_caller, sub_mode, DSC_NPARA(2) | FUNC_SET_SUB_DEVICE_ID_USED, NULL);
}

RET_CODE dsc_deal_quantum_for_mixed_ts(p_deen_config p_deen, UINT32 temp_length)
{
    jump_to_func(NULL, os_hld_caller, p_deen , DSC_NPARA(2) | FUNC_DEAL_QUANTUM_MIXED_TS, NULL);
}

UINT32 dsc_get_idle_pid_sram_num(enum DMA_MODE dma_mode)
{
	jump_to_func(NULL, os_hld_caller, dma_mode,DSC_NPARA(1)|FUNC_GET_IDLE_PID_SRAM_NUM, NULL);
}

UINT32 dsc_get_idle_key_sram_num( UINT32 key_len )
{
	jump_to_func(NULL, os_hld_caller, key_len,DSC_NPARA(1)|FUNC_GET_IDLE_KEY_SRAM_NUM, NULL);
}

UINT32 dsc_get_free_sub_device_num( enum WORK_SUB_MODULE sub_mode)
{
	jump_to_func(NULL, os_hld_caller, sub_mode,DSC_NPARA(1)|FUNC_GET_FREE_SUB_DEVICE_NUM, NULL);
}

UINT16 dsc_get_free_stream_id_num( enum DMA_MODE dma_mode)
{
	jump_to_func(NULL, os_hld_caller, dma_mode,DSC_NPARA(1)|FUNC_GET_FREE_STREAM_ID_NUM, NULL);
}


#endif  /*_HLD_DSC_REMOTE*/

#endif  /*DUAL_ENABLE*/


RET_CODE dsc_fixed_cryption(UINT8 *input, UINT32 length, UINT32 pos)
{
    DSC_FIXED_CRYPTION param_fixed;
    p_dsc_dev p_dscdev = NULL;
    RET_CODE ret = RET_FAILURE;

    if(0 == length)
    {
        return RET_FAILURE;
    }

    p_dscdev = ( p_dsc_dev ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
    if((NULL == p_dscdev) || (NULL == input))
    {
        return ret;
    }

    dsc_main_cache_flush((UINT32)input, (UINT32)input, length);
    param_fixed.input = input;
    param_fixed.length = length;
    param_fixed.pos = pos;
    ret = dsc_ioctl(p_dscdev, IO_DSC_FIXED_DECRYPTION, (UINT32)&param_fixed);

    return ret;
}

RET_CODE dsc_deal_sys_uk_fw(UINT8 *input, UINT8 *output, UINT8 *key, UINT32 length,
							UINT32 pos, enum CRYPT_SELECT mode, UINT8 root)
{
    DSC_DEV *p_dscdev = NULL;
    RET_CODE ret = RET_FAILURE;
    DSC_SYS_UK_FW dsc_key;

    p_dscdev = (DSC_DEV *) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
    if((NULL == p_dscdev) || (NULL == input) || (NULL == output) || (NULL == key) || (0 == length))
    {
        return ret;
    }

    if((mode != DSC_DECRYPT) && (mode != DSC_DECRYPT))
    {
        return ret;
    }

    MEMSET((void*)&dsc_key, 0x00, sizeof(dsc_key));

    MEMCPY((void*)dsc_key.ck,key,sizeof(dsc_key.ck));
    dsc_key.input = input;
    dsc_key.output = output;
    dsc_key.length = length;
    dsc_key.mode = mode;
    dsc_key.pos = pos;
	dsc_key.root = root;
    dsc_main_cache_flush((UINT32)input, (UINT32)output, length);
    ret = dsc_ioctl(p_dscdev, IO_DSC_SYS_UK_FW, (UINT32)&dsc_key);
    dsc_main_cache_inv((UINT32)output, length);

    return ret;
}


/*
0 ->Disable
1 -> Enable
*/
RET_CODE dsc_enable_disable_cmdq(UINT32 en_or_dis)
{
    DSC_DEV *p_dsc_dev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

    if((0 != en_or_dis) && (1 != en_or_dis))
    {
        return RET_FAILURE;
    }

    if(p_dsc_dev)
    {
        return dsc_ioctl(p_dsc_dev,IO_DSC_SET_CLR_CMDQ_EN,en_or_dis);
    }
    else
    {
        return RET_FAILURE;
    }
}
RET_CODE ali_sha_digest(UINT8 *input, UINT32 input_len,
                        enum SHA_MODE sha_mode, UINT8 *output)
{
    UINT32 shadevid = ALI_INVALID_DSC_SUB_DEV_ID;
    p_sha_dev pshadev = NULL;
    SHA_INIT_PARAM param;
    RET_CODE ret = RET_FAILURE;

    if ( (NULL == input) || (NULL == output) \
        || (0 == input_len) || (SHA_MODE_CHECK ( sha_mode )) )
    {
        DSC_REMOTE_API_PRINTF( "%s(): Wrong para!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    shadevid = dsc_get_free_sub_device_id(SHA);
    if (ALI_INVALID_DSC_SUB_DEV_ID == shadevid)
    {
        DSC_REMOTE_API_PRINTF("%s() get free SHA device failed!\n", __FUNCTION__);
        return RET_FAILURE;
    }
    pshadev = (p_sha_dev)dev_get_by_id(HLD_DEV_TYPE_SHA, shadevid);
    if (NULL == pshadev)
    {
        DSC_REMOTE_API_PRINTF("%s() get SHA device %d failed!\n", \
                              __FUNCTION__, shadevid);
        dsc_set_sub_device_id_idle(SHA, shadevid);
        return RET_FAILURE;
    }

    MEMSET(&param, 0, sizeof(SHA_INIT_PARAM));
    param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
    param.sha_work_mode = sha_mode;
    sha_ioctl(pshadev, IO_INIT_CMD, (UINT32)&param);
    ret = sha_digest(pshadev, input, (UINT8 *)output, input_len);

    dsc_set_sub_device_id_idle(SHA, shadevid);
    return ret;
}

RET_CODE see_ali_sha_digest(UINT8 *input, UINT32 input_len,
                        enum SHA_MODE sha_mode, UINT8 *output)
{
    if ( (NULL == input) || (NULL == output) || (0 == input_len) || (SHA_MODE_CHECK ( sha_mode )) )
    {
        DSC_REMOTE_API_PRINTF( "%s(): Wrong para!\n", __FUNCTION__);
        return RET_FAILURE;
    }
	
    return ali_sha_digest(input, input_len, sha_mode, output);
}


/* This function is used to decrypt or encrypt the puredata use key from crypto engine
u8 *input,                the input data
u8 *output,               the output data
u32 length,               the data length
u32 key_pos,              the key pos in crypto engine
enum CRYPT_SELECT sel     decrypt or encrypt select
*/
RET_CODE aes_crypt_puredata_with_ce_key(UINT8 *input, UINT8 *output, UINT32 length, UINT32 key_pos,
                                        enum CRYPT_SELECT sel)
{
    UINT32 aesdevid = INVALID_DSC_SUB_DEV_ID;
    p_aes_dev  paesdev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    UINT16 pid[1] = {0x1234};
    AES_IV_INFO iv;

    if((NULL == input) || (NULL == output) || (0 == length))
    {
        return ret;
    }

    aesdevid = dsc_get_free_sub_device_id(AES);
    if (INVALID_DSC_SUB_DEV_ID == aesdevid)
    {
        DSC_REMOTE_API_PRINTF("dsc_get_free_sub_device_id() failed\n");
        return -1;
    }

    paesdev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aesdevid);
    if ( NULL  == paesdev)
    {
        DSC_REMOTE_API_PRINTF("%s() get AES device %d failed!\n", __FUNCTION__, aes_dev_id);
        dsc_set_sub_device_id_idle(AES, aesdevid);
        return -1;
    }
    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
    if ( INVALID_DSC_STREAM_ID  == stream_id)
    {
        DSC_REMOTE_API_PRINTF("%s() get free stream id failed!\n", __FUNCTION__);
        dsc_set_sub_device_id_idle(AES, aesdevid);
        return -1;
    }
    MEMSET(iv.even_iv, 0, sizeof(iv.even_iv));
    MEMSET(iv.odd_iv, 0, sizeof(iv.odd_iv));
    MEMSET(&aes_param, 0, sizeof(struct aes_init_param));
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_CRYPTO;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CBC ;
    aes_param.cbc_cts_enable = 0;
    aes_ioctl(paesdev , IO_INIT_CMD , (UINT32)&aes_param);

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = 0xFF ;
    key_param.ctr_counter = NULL ;
    key_param.init_vector = (UINT8 *)&iv ;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.p_aes_iv_info = &iv ;
    key_param.p_aes_key_info = NULL;
    key_param.stream_id = stream_id;
    key_param.force_mode = 1;
    key_param.pos = (key_pos&0xFF);
    key_param.kl_sel = (key_pos>>8)&0x0F;
	dsc_main_cache_flush((UINT32)&iv, (UINT32)NULL, sizeof(AES_IV_INFO));

    aes_ioctl(paesdev , IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(DSC_ENCRYPT == sel)
    {
        ret = aes_encrypt(paesdev, stream_id, input, output, length);
    }
    else
    {
        ret = aes_decrypt(paesdev, stream_id, input, output, length);
    }
    if(RET_SUCCESS != ret)
    {
        DSC_REMOTE_API_PRINTF("%s() aes crypt fail!\n", __FUNCTION__);
        aes_ioctl(paesdev , IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(AES, aesdevid);
        return -1;
    }
    aes_ioctl(paesdev , IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
    dsc_set_stream_id_idle(stream_id);
    dsc_set_sub_device_id_idle(AES, aesdevid);
    return RET_SUCCESS;
}

/*
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ecb_crypt( UINT8 *key, UINT8 *input, UINT8 *output, UINT32
                             length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_aes_dev paesdev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    AES_KEY_PARAM key_info[1];
    UINT16 pid[1] = {0x1234};
    UINT32 device_id = dsc_get_free_sub_device_id(AES);

    if((NULL == key) || (NULL == input) || (NULL == output) || (0 == length))
    {
        return ret;
    }

    if(INVALID_DSC_SUB_DEV_ID == device_id )
    {
        return ret;
    }

    paesdev = (AES_DEV *)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);

    if (INVALID_DSC_STREAM_ID == stream_id )
    {
        goto DONE1;
    }

    MEMSET( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = AES_128BITS_MODE;
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_ECB ;
    ret=aes_ioctl ( paesdev , IO_INIT_CMD , ( UINT32 ) &aes_param );
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }


    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    MEMCPY(key_info[0].aes_128bit_key.even_key, key, 16);
    key_param.handle = 0xFF ;
    key_param.ctr_counter = NULL;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.p_aes_key_info = key_info;
    key_param.stream_id = stream_id;
    key_param.force_mode = 1;
    ret = aes_ioctl (paesdev , IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(RET_SUCCESS != ret)
    {
        goto DONE2;
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
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}

/*
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_cbc_crypt( UINT8 *key, UINT8 *iv, UINT8 *input, UINT8 *output,
                             UINT32 length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_aes_dev paesdev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    AES_KEY_PARAM key_info[1];
    AES_IV_INFO iv_info;
    UINT16 pid[1] = {0x1234};
    UINT32 device_id = 0;

    if((NULL == key)||( NULL == input) || (NULL == output) || (NULL == iv) || (0 == length))
    {
        return ret;
    }

    device_id = dsc_get_free_sub_device_id(AES);

    if(INVALID_DSC_SUB_DEV_ID == device_id)
    {
        return ret;
    }
    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);

    if (INVALID_DSC_STREAM_ID == stream_id )
    {
        goto DONE1;
    }

    paesdev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

    MEMSET( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CBC ;
    ret=aes_ioctl ( paesdev , IO_INIT_CMD , ( UINT32 ) &aes_param );
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    MEMCPY(key_info[0].aes_128bit_key.even_key, key, 16);
    MEMCPY(iv_info.even_iv, iv, 16);
    key_param.handle = 0xFF ;
    key_param.ctr_counter = NULL;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.p_aes_iv_info = &iv_info;
    key_param.p_aes_key_info = key_info;
    key_param.stream_id = stream_id;
    ret=aes_ioctl (paesdev,IO_CREAT_CRYPT_STREAM_CMD,(UINT32)&key_param);
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
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
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}

/*
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ctr_crypt( UINT8 *key, UINT8 *ctr, UINT8 *input, UINT8 *
                             output, UINT32 length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_aes_dev paesdev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    AES_KEY_PARAM key_info[1];
    UINT32 device_id = 0;
    UINT16 pid[1] = {0x1234};

    if((NULL == key)||( NULL == input) || (NULL == output) || (NULL == ctr) || (0 == length))
    {
        return ret;
    }

    device_id = dsc_get_free_sub_device_id(AES);
    if(INVALID_DSC_SUB_DEV_ID == device_id)
    {
        return ret;
    }

    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);

    if (INVALID_DSC_STREAM_ID == stream_id )
    {
        goto DONE1;
    }
    paesdev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

    MEMSET( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_AS_ATSC;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CTR ;
    ret=aes_ioctl ( paesdev , IO_INIT_CMD , ( UINT32 ) &aes_param );
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    MEMCPY(key_info[0].aes_128bit_key.even_key, key, 16);
    key_param.handle = 0xFF ;
    key_param.ctr_counter = ctr;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.p_aes_key_info = key_info;
    key_param.stream_id = stream_id;
    ret=aes_ioctl (paesdev,IO_CREAT_CRYPT_STREAM_CMD,(UINT32)&key_param);
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
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
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}

/*
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE tdes_pure_ecb_crypt( UINT8 *key, UINT8 *input, UINT8 *output, UINT32
                              length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_des_dev pdesdev = NULL;
    struct des_init_param des_param;
    KEY_PARAM key_param;
    DES_KEY_PARAM key_info[1];
    UINT16 pid[1] = {0x1234};
    UINT32 device_id = 0;

    if((NULL == key)||( NULL == input) || (NULL == output) || (0 == length))
    {
        return ret;
    }
    device_id = dsc_get_free_sub_device_id(DES);
    if(INVALID_DSC_SUB_DEV_ID == device_id)
    {
        return ret;
    }
    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);

    if (INVALID_DSC_STREAM_ID == stream_id )
    {
        goto DONE1;
    }
    pdesdev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, device_id);

    MEMSET( &des_param, 0, sizeof ( struct des_init_param ) );
    des_param.sub_module = TDES;
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_SRAM;
    des_param.key_mode = TDES_ABA_MODE ;
    des_param.parity_mode = EVEN_PARITY_MODE;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0 ;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_ECB ;
    ret=des_ioctl ( pdesdev , IO_INIT_CMD , ( UINT32 ) &des_param );
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    MEMCPY(key_info[0].des_128bits_key.even_key, key, 16);
    key_param.handle = 0xFF ;
    key_param.ctr_counter = NULL;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.p_des_key_info = key_info;
    key_param.stream_id = stream_id;
    ret=des_ioctl (pdesdev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }

    if(DSC_ENCRYPT == crypt_mode)
    {
        ret = des_encrypt( pdesdev, stream_id, input, output, length );
    }
    else
    {
        ret = des_decrypt( pdesdev, stream_id, input, output, length );
    }

    des_ioctl( pdesdev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(DES, device_id);
    return ret;
}
/*
    key_pos -> CE key pos
    length should < 64MB
    crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE tdes_pure_ecb_crypt_with_ce_key( UINT32 key_pos, UINT8 *input, UINT8 *output, UINT32
        length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_des_dev pdesdev = NULL;
    struct des_init_param des_param;
    KEY_PARAM key_param;
    UINT16 pid[1] = {0x1234};
    UINT32 device_id = 0;

    if(( NULL == input) || (NULL == output) || (0 == length))
    {
        return ret;
    }

    device_id = dsc_get_free_sub_device_id(DES);
    if(INVALID_DSC_SUB_DEV_ID == device_id)
    {
        return ret;
    }
    pdesdev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, device_id);

    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
    if (INVALID_DSC_STREAM_ID  == stream_id)
    {
       dsc_set_sub_device_id_idle(DES, device_id);
       return ret;
    }

    MEMSET( &des_param, 0, sizeof ( struct des_init_param ) );
    des_param.sub_module = TDES;
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_CRYPTO;
    des_param.key_mode = TDES_ABA_MODE;
    des_param.parity_mode = EVEN_PARITY_MODE;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0 ;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_ECB ;
    des_param.cbc_cts_enable = 0;
    ret = des_ioctl ( pdesdev , IO_INIT_CMD , ( UINT32 ) &des_param );
    if(RET_SUCCESS != ret)
    {
       dsc_set_stream_id_idle(stream_id);
       dsc_set_sub_device_id_idle(DES, device_id);
       return ret;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = 0xFF ;
    key_param.ctr_counter = NULL;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = pid;
    key_param.stream_id = stream_id;
    key_param.force_mode = 1;
    key_param.pos = (key_pos&0xFF);
    key_param.kl_sel = (key_pos>>8)&0x0F;
    ret = des_ioctl (pdesdev , IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(RET_SUCCESS != ret)
    {
       dsc_set_stream_id_idle(stream_id);
       dsc_set_sub_device_id_idle(DES, device_id);
       return ret;
    }

    if(DSC_ENCRYPT == crypt_mode)
    {
        ret = des_encrypt( pdesdev, stream_id, input, output, length );
    }
    else
    {
        ret = des_decrypt( pdesdev, stream_id, input, output, length );
    }

    des_ioctl( pdesdev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);

    dsc_set_stream_id_idle(stream_id);

    dsc_set_sub_device_id_idle(DES, device_id);
    return ret;
}


/*
    key_pos -> CE key pos
    ctr -> as the IV/Counter
    length should < 64MB
    crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ctr_crypt_with_ce_key( UINT32 key_pos, UINT8 *ctr, UINT8 *input, UINT8 *
        output, UINT32 length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    p_aes_dev paesdev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    UINT32 device_id = 0;

    if(( NULL == input) || (NULL == output) || (NULL == ctr) || (0 == length))
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
    aes_param.residue_mode = RESIDUE_BLOCK_IS_AS_ATSC;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CTR;
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
    key_param.ctr_counter = ctr;
    key_param.init_vector = NULL;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.force_mode = 1;
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
/*
    this function used to encrypt the bootloader universal key
    encrypt_type
    0: encrypt bl uk use key 6
    1: encrypt bl uk use key 6 with r1
    2: encrypt bl uk use key 7
*/
RET_CODE ali_dsc_encrypt_bl_uk(UINT8 *input, UINT8 *r_key, UINT8 *output, UINT32 encrypt_type)
{
    p_dsc_dev  pdscdev = NULL;
    DSC_BL_UK_PARAM bl_uk_param;
    RET_CODE ret = RET_FAILURE;

    if(( NULL == input) || (NULL == output) || encrypt_type > 3)
    {
        return ret;
    }

    pdscdev =(p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
    if(NULL == pdscdev)
    {
        return RET_FAILURE;
    }
    MEMSET(&bl_uk_param, 0 , sizeof(DSC_BL_UK_PARAM));
    bl_uk_param.input_key = input;
    bl_uk_param.r_key = r_key;
    bl_uk_param.output_key = output;
    bl_uk_param.crypt_type = encrypt_type;
    osal_cache_flush((void*)bl_uk_param.input_key, ALIASIX_BL_UK_LEN);
	if(bl_uk_param.r_key)
	{
    	osal_cache_flush((void*)bl_uk_param.r_key, ALIASIX_BL_UK_LEN);
	}
    ret = dsc_ioctl(pdscdev, IO_DSC_ENCRYTP_BL_UK, (UINT32)&bl_uk_param);
    return ret;
}

/* Using clear key -> KEY_FROM_SRAM to do AES_CBC, residue mode is configurable
key_length -> in bits //128,192,256
crypt_mode -> DSC_ENCRYPT or DSC_DECRYPT
residue -> NO_HANDLE, ATSC or CipherStealing
*/
RET_CODE aes_nbits_pure_cbc_crypt_keysram( UINT8 *key, UINT8 *iv, UINT8 *input, 
	UINT8 *output, UINT32 length, UINT8 crypt_mode, UINT32 key_length, UINT32 residue)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	p_aes_dev pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info[1];
	AES_IV_INFO iv_info[1];

	if(key_length != 128 &&  key_length != 192 && key_length != 256)
		return ret;

	if(!key || !iv || !input || !output)
		return ret;

	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = key_length/64 - 1;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = residue;
	aes_param.work_mode = WORK_MODE_IS_CBC;
	aes_param.stream_id = stream_id;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
	    goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	if(key_length == 128)
		memcpy(key_info[0].aes_128bit_key.even_key, key, key_length/8);
	else if(key_length == 192)
		memcpy(key_info[0].aes_192bit_key.even_key, key, key_length/8);
	else
		memcpy(key_info[0].aes_256bit_key.even_key, key, key_length/8);

	memcpy(iv_info[0].even_iv,iv,16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;//KEY_FROM_OTP using this ptr
	key_param.key_length = key_length;  
	key_param.pid_len = 1; 				
	key_param.p_aes_key_info = key_info;
	key_param.p_aes_iv_info = iv_info;//KEY_FROM_SRAM/CRYPTO using this ptr
	key_param.stream_id = stream_id;
	key_param.key_from = KEY_FROM_SRAM;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
	    goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
	    ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
	    ret = aes_decrypt( pAesDev, stream_id, input, output, length );

    // exit
	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}



/* Using secure OTP key -> KEY_FROM_OTP to do AES_ECB_128
otp_key_pos -> OTP_KEY_FROM_68 or OTP_KEY_FROM_6C
crypt_mode -> DSC_ENCRYPT or DSC_DECRYPT
*/
RET_CODE aes_128_pure_ecb_crypt_keyotp(UINT8 *input, UINT8 *output, 
UINT32 length, UINT8 crypt_mode, UINT32 otp_key_pos)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	AES_DEV *pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_CRYPT_WITH_FP aes_crypt_fp;

	if(!input || !output)
		return ret;

	if(otp_key_pos != OTP_KEY_FROM_68 &&
		otp_key_pos != OTP_KEY_FROM_6C &&
		otp_key_pos != OTP_KEY_FROM_FP)
		return ret;

	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pAesDev = (AES_DEV *)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_OTP;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = otp_key_pos;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_ECB ;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE1;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 		
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id;
	key_param.key_from = KEY_FROM_OTP;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE1;
	}

	if(OTP_KEY_FROM_FP == otp_key_pos)
	{
		if(16 != length)
		{
			ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
			goto DONE2;
		}
        
		memset(&aes_crypt_fp, 0, sizeof(AES_CRYPT_WITH_FP));
		aes_crypt_fp.crypt_mode = crypt_mode;
		aes_crypt_fp.input = input;
		ret=aes_ioctl (pAesDev, IO_AES_CRYPT_WITH_FP, (UINT32)&aes_crypt_fp);
		if(ret != RET_SUCCESS)
		{
			goto DONE2;
		}

		memcpy(output, aes_crypt_fp.output, 16);
	}
	else
	{
		if(crypt_mode == DSC_ENCRYPT)
			ret = aes_encrypt( pAesDev, stream_id, input, output, length );
		else
			ret = aes_decrypt( pAesDev, stream_id, input, output, length );
	}
    
DONE2:
	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}


