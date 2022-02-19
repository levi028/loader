/*****************************************************************************
 *    Copyrights(C) 2010 ALI Corp. All Rights Reserved.
 *
 *    FILE NAME:        ci_crypt.c
 *
 *    DESCRIPTION:     CI+ encrypt or decrypt interfaces for CI+ data storage
 *
 *    HISTORY:
 *        Date         Author           Version       Notes
 *        =========    =========      =========      =========
 *        2010/5/14   Elliott, Yun     0.1
 *
 *****************************************************************************/
#include <sys_config.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <hld/dsc/dsc.h>
#include <api/libci/ci_plus.h>
#include <api/libci/ci_cc_api.h>
#include <openssl/evp.h>
#include <hld/hld_dev.h>
#include <hld/crypto/crypto.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto_dev.h>
#ifdef DSC_SUPPORT
#include <hld/dsc/dsc.h>
#endif
#include <api/libc/string.h>
#include "ci_stack.h"

#ifdef CI_PLUS_SUPPORT

#define CIPLUSKEY_CHUNK_ID 0x09F60101
#define AKH_DHSK_MAX_NUM 5
#define AKH_DHSK_FLASH_SIZE 0x10000

#define CIPLUS_KEY_CHUNK_OFFSET_SDK40 0x10000
#define CIPLUS_KEY_CHUNK_OFFSET_SDK50 0x20000
#define CIPLUS_INVALID_AKH_LEN   0xFFFFFFFF
#define CIPLUS_INVALID_KEY_TYPE  0xFFFFFFFF

typedef struct {
    UINT8 num;
    UINT8 pos;
    UINT8 reserved1;
    UINT8 reserved2;
} AKH_DHSK_HDR;

static UINT8 g_ci_tmp_buff[2048] __attribute__((  aligned( 64 )));
static UINT8 g_ci_tmp_buff2[2048] __attribute__((  aligned( 64 )));
extern RET_CODE find_key(UINT32 type, unsigned long *paddr, CIPLUS_KEY_HDR *phdr);

#ifdef CI_ENCRYPT_BY_CPU

/*
 *  @Function name: encrypt_key
 *  @Usage: encrypt AES_KEY or URI for PVR
 *  @Input:
 *  @Output:
 *  @Error:
 */
RET_CODE api_ci_encrypt_aes_key(UINT8 *raw_key, UINT8 *enc_key, UINT32 len)
{
    UINT32 i;
    UINT32 value;
    UINT8 otp_key[16];
    AES_KEY aes_key;

    //read key from otp
    for(i=0; i<4; i++)
    {
        value = ali_otp_read(OTP_PVR_KEY_ADDR+i*4);
        MEMCPY(otp_key+i*4, &value, 4);
    }

    //encrypt key
    aes_set_encrypt_key(otp_key, 128, &aes_key);

    while(len>0)
    {
        if(len<16)
        {
            MEMCPY(enc_key, raw_key, len);
        }
        else
        {
            aes_ecb_encrypt(raw_key, enc_key, &aes_key, 1);
        }
        raw_key += 16;
        enc_key += 16;
        len -= 16;
    }

    return RET_SUCCESS;
}

/*
 *  @Function name: encrypt_key
 *  @Usage: decrypt AES_KEY or URI for PVR
 *  @Input:
 *  @Output:
 *  @Error:
 */
RET_CODE api_ci_decrypt_aes_key(UINT8 *enc_key, UINT8 *dec_key, UINT32 len)
{
    UINT32 i;
    UINT32 value;
    UINT8 otp_key[16];
    AES_KEY aes_key;

    //read key from otp
    for(i=0; i<4; i++)
    {
        value = ali_otp_read(OTP_PVR_KEY_ADDR+i*4);
        MEMCPY(otp_key+i*4, &value, 4);
    }

    //decrypt key
    aes_set_decrypt_key(otp_key, 128, &aes_key);

    while(len>0)
    {
        if(len<16)
        {
            MEMCPY(dec_key, enc_key, len);
        }
        else
        {
            aes_ecb_encrypt(enc_key, dec_key, &aes_key, 0);
        }
        enc_key += 16;
        dec_key += 16;
        len -= 16;
    }

    return RET_SUCCESS;
}

#else

/*
 *  @Function name: api_ci_encrypt_aes_key
 *  @Usage:
 *  @Input:
 *  @Output:
 *  @Error:
 */
RET_CODE api_ci_encrypt_aes_key(UINT8 *raw_key, UINT8 *enc_key, UINT32 len)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;
    UINT8 aes_seed[32];
    UINT8 *encryped_data;
    UINT8 *key_tmp;
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AES_KEY, &addr, &hdr))
        return RET_FAILURE;

    sto_get_data(dev, aes_seed, addr, 32);    //seed

    encryped_data = g_ci_tmp_buff;
    key_tmp = g_ci_tmp_buff2;
    MEMCPY(key_tmp, raw_key, len);

    encrypt_key(key_tmp, encryped_data, aes_seed, len);
    MEMCPY(enc_key, encryped_data, len);
    return RET_SUCCESS;
}

#endif

//static RET_CODE find_key(UINT32 type, unsigned long *paddr, CIPLUS_KEY_HDR *phdr)
static RET_CODE find_key(UINT32 type, unsigned long *paddr, CIPLUS_KEY_HDR *phdr)
{
    unsigned long addr, chunk_end;
    UINT32 chunk_len,chunk_offset;
    UINT32 ch_id = CIPLUSKEY_CHUNK_ID;

    addr = sto_chunk_goto(&ch_id, 0xFFFFFFFF, 1);
        if(addr & 0x80000000)
        return RET_FAILURE;

    chunk_len = sto_fetch_long(addr + CHUNK_LENGTH);
    chunk_offset = sto_fetch_long(addr + CHUNK_OFFSET);

    APPL_PRINTF("%s: chunk len = 0x%x, chunk offset = 0x%x\n",
                __FUNCTION__, chunk_len, chunk_offset);

    if (CIPLUS_KEY_CHUNK_OFFSET_SDK40 >= chunk_offset)
    {
        chunk_end = addr +16 + chunk_len;
        addr += CHUNK_HEADER_SIZE;
    }
    else if (CIPLUS_KEY_CHUNK_OFFSET_SDK50 >= chunk_offset)
    {
        if(CIPLUS_KEY_AKH_DHSK==type)
        {
            chunk_end = addr +chunk_offset;
            addr = chunk_end - AKH_DHSK_FLASH_SIZE + CHUNK_HEADER_SIZE;
        }
        else
        {
            chunk_end = addr +chunk_offset-AKH_DHSK_FLASH_SIZE;
            addr += CHUNK_HEADER_SIZE;
        }
    }
    else
    {
        APPL_PRINTF("%s: invalid CI Plus key CHUNK offset\n", __FUNCTION__);
        return RET_FAILURE;
    }

    APPL_PRINTF("%s: addr = 0x%x, chunk end = 0x%x\n",
                __FUNCTION__, addr, chunk_end);
    while(addr < chunk_end)
    {

        phdr->type = sto_fetch_long(addr);
        phdr->len = sto_fetch_long(addr+4);
        APPL_PRINTF("%s: type = %d, len = %d\n", __FUNCTION__, phdr->type, phdr->len);
        if(phdr->type == type ||
          (type == CIPLUS_KEY_AKH_DHSK && phdr->type == CIPLUS_INVALID_KEY_TYPE))
        {
            *paddr = addr+sizeof(CIPLUS_KEY_HDR);
            phdr->size = sto_fetch_long(addr+8);
            //phdr->reserved = sto_fetch_long(addr+12);
            return RET_SUCCESS;
        }
        addr += phdr->len;
        if(0==phdr->len || CIPLUS_INVALID_AKH_LEN == phdr->len)
            break;
    }

    return RET_FAILURE;
}

//AES
RET_CODE sys_write_host_key(UINT8 *key, UINT32 len, UINT32 type)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;
    UINT8 length[4];
    //UINT8 aes_key[16];
    UINT8 aes_seed[32];
    UINT8 *encryped_data;
    UINT8 *key_tmp;
    UINT8 *flash_buffer = NULL;
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AES_KEY, &addr, &hdr))
        return RET_FAILURE;

    //sto_get_data(dev, aes_key, addr, 16);
    sto_get_data(dev, aes_seed, addr, 32);    //seed

    if(RET_SUCCESS != find_key(type, &addr, &hdr))
        return RET_FAILURE;

    encryped_data = g_ci_tmp_buff;

    key_tmp = g_ci_tmp_buff2;
    MEMCPY(key_tmp, key, len);

    if(!flash_buffer)
        flash_buffer = MALLOC(64*1024);
    sto_io_control(dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)flash_buffer);
    sto_io_control(dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);

    //encrypt_key(key_tmp, encryped_data, aes_key, len);
    encrypt_key(key_tmp, encryped_data, aes_seed, len);

    length[0] = len>>24;
    length[1] = (len>>16)&0xFF;
    length[2] = (len>>8)&0xFF;
    length[3] = len&0xFF;

    sto_put_data(dev, addr-8, length, 4);
    sto_put_data(dev, addr, encryped_data, len);

    if(flash_buffer)
    {
        FREE(flash_buffer);
        flash_buffer = NULL;
        sto_io_control(dev, STO_DRIVER_SECTOR_BUFFER, 0);
        sto_io_control(dev, STO_DRIVER_SET_FLAG, 0);
    }

    return RET_SUCCESS;
}

RET_CODE sys_read_host_key(UINT8 *key, UINT32 *len, UINT32 type, UINT32 max_len)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;
//    UINT8 aes_key[16];
    UINT8 aes_seed[32];
    UINT8 *encryped_data;
    UINT8 *key_tmp;
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    APPL_PRINTF("%s - type %d\n",__FUNCTION__,type);

    if(RET_SUCCESS != find_key(type, &addr, &hdr))
        return RET_FAILURE;

    if(hdr.size > max_len)
        return RET_FAILURE;

    encryped_data = g_ci_tmp_buff;
    key_tmp = g_ci_tmp_buff2;

    // sto_get_data(dev, encryped_data, addr, hdr.size);
    sto_get_data(dev, key, addr, hdr.size);
    if (len) *len = hdr.size;

    return RET_SUCCESS;
}

RET_CODE read_ciplus_cert_data()
{
    APPL_PRINTF("%s - read CIPlus Certificate data...\n", __FUNCTION__);
    UINT8 *key_buf;
    UINT32 *len_cert;
    UINT32 *len_hdq;

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_ROOT_CERT);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_ROOT_CERT, CI_CERT_LEN );
    decrypt_cert(CIPLUS_KEY_ROOT_CERT);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_BRAND_CERT);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_BRAND_CERT, CI_CERT_LEN);
    decrypt_cert(CIPLUS_KEY_BRAND_CERT);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_DEVICE_CERT);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_DEVICE_CERT, CI_CERT_LEN);
    decrypt_cert(CIPLUS_KEY_DEVICE_CERT);

    //len = (UINT32 *)MALLOC(sizeof(UINT32));
    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_PRNG_KEY_K);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_PRNG_KEY_K, SAK_LEN);
    decrypt_cert(CIPLUS_KEY_PRNG_KEY_K);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_PRNG_SEED);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_PRNG_SEED, SAK_LEN);
    decrypt_cert(CIPLUS_KEY_PRNG_SEED);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_DH_P);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_DH_P, DH_BYTES_LEN);
    decrypt_cert(CIPLUS_KEY_DH_P);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_DH_G);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_DH_G, DH_BYTES_LEN);
    decrypt_cert(CIPLUS_KEY_DH_G);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_DH_Q);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_DH_Q, DH_Q_LEN);
    decrypt_cert(CIPLUS_KEY_DH_Q);

    ci_cc_get_cert_buf(&key_buf, &len_hdq, CIPLUS_KEY_HDQ);
    sys_read_host_key(key_buf, len_hdq, CIPLUS_KEY_HDQ, CI_CERT_LEN);
    decrypt_cert(CIPLUS_KEY_HDQ);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_SIV);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_SIV, SAK_LEN);
    decrypt_cert(CIPLUS_KEY_SIV);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_SLK);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_SLK, SLK_LEN);
    decrypt_cert(CIPLUS_KEY_SLK);

    ci_cc_get_cert_buf(&key_buf, &len_cert, CIPLUS_KEY_CLK);
    sys_read_host_key(key_buf, len_cert, CIPLUS_KEY_CLK, CLK_LEN);
    decrypt_cert(CIPLUS_KEY_CLK);

    //FREE(len);

    return RET_SUCCESS;
}

RET_CODE ci_get_hdr(UINT32 type, CIPLUS_KEY_HDR *hdr)
{
    unsigned long addr;
    //UINT8 key[16];
    UINT8 aes_seed[32];
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(RET_SUCCESS != find_key(type, &addr, hdr))
    {
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

RET_CODE read_akh_dhsk(UINT8 *buf, UINT8 idx)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;
    AKH_DHSK_HDR akh_hdr;
    //UINT8 key[16];
    UINT8 aes_seed[32];
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AKH_DHSK, &addr, &hdr))
    {
        MEMSET(buf, 0, AKH_ARRAY_SIZE);
        return RET_FAILURE;
    }

    sto_get_data(dev, (UINT8 *)&akh_hdr, addr-4, 4);
    //akh_hdr = (AKH_DHSK_HDR *)&hdr.reserved;

    if((idx >= akh_hdr.num) || (akh_hdr.num == 0) || (akh_hdr.pos >akh_hdr.num))
    {
        MEMSET(buf, 0, AKH_ARRAY_SIZE);
        return RET_FAILURE;
    }

    //get real index
    idx = (akh_hdr.pos+akh_hdr.num-idx)%akh_hdr.num;

    sto_get_data(dev, buf, addr+idx*AKH_ARRAY_SIZE, AKH_ARRAY_SIZE);

    return RET_SUCCESS;
}

RET_CODE read_ciplus_akh_etc()
{
    APPL_PRINTF("%s - read CIPlus CICAM_ID, AKH, DHSK, etc...\n", __FUNCTION__);
    int i = 0;

    UINT8 *akh_dhsk_buf;

    for(i=0; i<5; i++)
    {
        ci_cc_get_akhetc_buf(&akh_dhsk_buf, i);
        read_akh_dhsk(akh_dhsk_buf, i);
    }

    decrypt_akh_dhsk();

    return RET_SUCCESS;
}

/*************************************/

RET_CODE ci_get_aes_seed(UINT8 *aes_seed)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;

    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    if(!aes_seed) return RET_FAILURE;

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AES_KEY, &addr, &hdr))
        return RET_FAILURE;

    //sto_get_data(dev, key, addr, 16);
    sto_get_data(dev, aes_seed, addr, 32);

    return RET_SUCCESS;
}

/*************************************/

RET_CODE write_akh_dhsk(UINT8 *buf)
{
    unsigned long addr;
    CIPLUS_KEY_HDR hdr;
    AKH_DHSK_HDR akh_hdr;
    //UINT8 key[16];
    UINT8 aes_seed[32];
    UINT8 *flash_buffer = NULL;
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    APPL_PRINTF("==>%s\n", __FUNCTION__);

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AES_KEY, &addr, &hdr))
        return RET_FAILURE;

    //sto_get_data(dev, key, addr, 16);
    sto_get_data(dev, aes_seed, addr, 32);

    if(RET_SUCCESS != find_key(CIPLUS_KEY_AKH_DHSK, &addr, &hdr))
        return RET_FAILURE;

    if(!flash_buffer)
        flash_buffer = MALLOC(64*1024);
    sto_io_control(dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)flash_buffer);

    //akh_hdr = (AKH_DHSK_HDR *)&hdr.reserved;
    sto_get_data(dev, (UINT8 *)&akh_hdr, addr-4, 4);

    if(akh_hdr.num != 0)
    {
        akh_hdr.pos++;
        if(akh_hdr.pos >= AKH_DHSK_MAX_NUM)
            akh_hdr.pos = 0;
    }

    if(akh_hdr.num < AKH_DHSK_MAX_NUM)
        akh_hdr.num++;

    sto_io_control(dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);

    sto_put_data(dev, addr-4, (UINT8 *)&akh_hdr, sizeof(AKH_DHSK_HDR));

    sto_put_data(dev, addr+akh_hdr.pos*AKH_ARRAY_SIZE, buf, AKH_ARRAY_SIZE);

    if(flash_buffer)
    {
        FREE(flash_buffer);
        flash_buffer = NULL;
        sto_io_control(dev, STO_DRIVER_SECTOR_BUFFER, 0);
        sto_io_control(dev, STO_DRIVER_SET_FLAG, 0);
    }
    return RET_SUCCESS;
}

#endif

