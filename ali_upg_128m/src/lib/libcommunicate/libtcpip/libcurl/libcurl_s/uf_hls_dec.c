//#include "uf_hls.h"
//#include "log.h"


#define __tds__ 


#ifdef __tds__

#include <stdio.h>
#include <assert.h>
//#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <basic_types.h>
#include <osal/osal_mutex.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/librsa/flash_cipher.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#include <api/libcurl/curl.h>
//#include "uf_hls.h"

#define INVALID_DSC_SUB_DEV_ID     0xff
#define INVALID_DSC_STREAM_ID    0xff

typedef struct _ali_hls_aes_t
{
    unsigned char exist_flag;
    unsigned int device_id;
    unsigned int stream_id;
    unsigned int aes_handle;
    unsigned int key_length;
    AES_KEY_PARAM key_info[1];    
    p_aes_dev pAesDev;    
    
    // add by billy
    UINT8 last_iv[16];    
} AliHlsAesT, *pAliHlsAesT;

#else

#include <openssl/aes.h>

#endif//__tds__


#ifdef HLS_SUPPORT


#define LOG(FMT, ARGS...) __log("aes decrypt", FMT, ##ARGS)


void *aes_cbc_init(uint8_t *key, uint32_t bits)
{
#ifdef __tds__
    unsigned int device_id = 0;
    unsigned char stream_id = 0;
    p_aes_dev pAesDev = NULL;
    RET_CODE ret = RET_FAILURE;
    struct aes_init_param aes_param;
    pAliHlsAesT p_ali_hls_aes = NULL;
    
    if ((128 != bits) && (192 != bits) && (256 != bits))
    {
        return NULL;
    }
    
    device_id = dsc_get_free_sub_device_id(AES);
    if (INVALID_DSC_SUB_DEV_ID == device_id)
    {
        return NULL;        
    }
    
    pAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);
    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)    // 取得一个free的stream id
    {
            dsc_set_sub_device_id_idle(AES, device_id);
            return NULL;
    }
    memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;    // 组装aes_param结构体
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = bits/64 - 1;//AES_128BITS_MODE,key_length用在这里
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;//residue是什么意思
    aes_param.work_mode = WORK_MODE_IS_CBC;    // 指明是CBC
    aes_param.stream_id = stream_id;
    ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );// AES Init
    if(ret != RET_SUCCESS)
    {
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(AES, device_id);
        //LOG("failed\n");
        return NULL;
    }
    
    p_ali_hls_aes = (pAliHlsAesT)malloc(sizeof(AliHlsAesT));
    if (NULL == p_ali_hls_aes)
    {
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(AES, device_id);
        return NULL;                
    }
    memset(p_ali_hls_aes, 0x0, sizeof(AliHlsAesT));
    p_ali_hls_aes->device_id = device_id;
    p_ali_hls_aes->pAesDev = pAesDev;
    p_ali_hls_aes->stream_id = stream_id;
    p_ali_hls_aes->key_length = bits;
    if(bits == 128)
        memcpy(p_ali_hls_aes->key_info[0].aes_128bit_key.even_key, key, bits/8);
    else if(bits == 192)
        memcpy(p_ali_hls_aes->key_info[0].aes_192bit_key.even_key, key, bits/8);
    else
        memcpy(p_ali_hls_aes->key_info[0].aes_256bit_key.even_key, key, bits/8);
        
    return p_ali_hls_aes;    
#else
    AES_KEY *aes;

    aes = (AES_KEY *)malloc(sizeof (*aes));
    if (NULL == aes) {
        return NULL;
    }

    if (AES_set_decrypt_key(key, bits, aes) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        free(aes);
        return NULL;
    }

    return (void *)aes;
#endif
}

void aes_cbc_decrypt_hls(void *handle, uint8_t *input, uint8_t *output, uint32_t length, uint8_t *ivec)
{
#ifdef __tds__
    pAliHlsAesT p_ali_hls_aes = NULL;
    KEY_PARAM key_param;
    AES_IV_INFO iv_info[1];
    RET_CODE ret = RET_FAILURE;
    UINT16 raw_id[1] = {0x1234};
    
    p_ali_hls_aes = (pAliHlsAesT)handle;
    
    memset(&key_param, 0, sizeof(KEY_PARAM));
    if (0 == p_ali_hls_aes->exist_flag)
    {        
        memcpy(iv_info[0].even_iv, ivec, 16);//change file + first time
        memcpy(iv_info[0].odd_iv, ivec, 16);//change file + first time

        key_param.handle = 0xFF ;
        key_param.ctr_counter = NULL; 
        key_param.init_vector = NULL;//KEY_FROM_REG/OTP using this ptr
        key_param.key_length = p_ali_hls_aes->key_length;  
        key_param.pid_len = 1;                 
        key_param.p_aes_key_info = p_ali_hls_aes->key_info;
        key_param.p_aes_iv_info = iv_info;
        key_param.stream_id = p_ali_hls_aes->stream_id;
        ret = aes_ioctl(p_ali_hls_aes->pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
        //LOG("aes ioctl, create stream ret: %d\n", ret);

        p_ali_hls_aes->aes_handle = key_param.handle;
        p_ali_hls_aes->exist_flag = 1;
    }
    else
    {
        memcpy(iv_info[0].even_iv, p_ali_hls_aes->last_iv, sizeof(p_ali_hls_aes->last_iv));

        key_param.handle = p_ali_hls_aes->aes_handle;
        key_param.ctr_counter = NULL;
        key_param.init_vector = NULL;    
        key_param.key_length = p_ali_hls_aes->key_length;  
        key_param.pid_len = 1;                 
        key_param.pid_list = raw_id;
        key_param.p_aes_key_info = p_ali_hls_aes->key_info;
        key_param.p_aes_iv_info = iv_info; 
        key_param.stream_id = p_ali_hls_aes->stream_id;
        ret = aes_ioctl(p_ali_hls_aes->pAesDev, IO_KEY_INFO_UPDATE_CMD, (UINT32)&key_param);        
        //LOG("aes ioctl, update ret: %d\n", ret);
    }    
    
    ret = aes_decrypt(p_ali_hls_aes->pAesDev, p_ali_hls_aes->stream_id, input, output, length);

    memcpy(p_ali_hls_aes->last_iv, input+length-sizeof(p_ali_hls_aes->last_iv), sizeof(p_ali_hls_aes->last_iv));
    //LOG("aes decryption, ret: %d\n", ret);

#else
    AES_KEY *aes = (AES_KEY *)handle;

    if (NULL == aes) {
        return;
    }

    AES_cbc_encrypt(input, output, length, aes, ivec, AES_DECRYPT);
#endif
}

void aes_cbc_destroy(void *handle)
{
#ifdef __tds__
    pAliHlsAesT p_ali_hls_aes = NULL;
    
    p_ali_hls_aes = (pAliHlsAesT)handle;
    aes_ioctl(p_ali_hls_aes->pAesDev, IO_DELETE_CRYPT_STREAM_CMD , p_ali_hls_aes->aes_handle);
    dsc_set_stream_id_idle(p_ali_hls_aes->stream_id);
    dsc_set_sub_device_id_idle(AES, p_ali_hls_aes->device_id);
    free(p_ali_hls_aes);    
#else
    if (NULL != handle) {
        free(handle);
    }
#endif
}


#endif
