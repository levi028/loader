#ifndef _GACAS_AES_H_
#define _GACAS_AES_H_

#ifdef __cplusplus
extern "C" {
#endif
unsigned int gacas_aes_crc32(unsigned char *szData, unsigned int size);
void gacas_aes_printf_bin_16(UINT8 *p_data);
RET_CODE gacas_aes_lv3_encypt(UINT8 skey_index,UINT32 data_len,UINT8*data_in,UINT8*data_out,UINT8*pk,UINT8*ck);
RET_CODE gacas_aes_lv3_decypt(UINT8 skey_index,UINT32 data_len,UINT8*data_in,UINT8*data_out,UINT8*pk,UINT8*ck);
RET_CODE gacas_aes_ecb_encypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
RET_CODE gacas_aes_ecb_decypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
RET_CODE gacas_aes_cbc_encypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
RET_CODE gacas_aes_cbc_decypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
INT32 gacas_encrypt_chunk_by_otp(UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
INT32 gacas_decrypt_chunk_by_otp(UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
INT32 gacas_decrypt_chunk_data(UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
INT32 gacas_decrypt_upg_file(UINT8 *data_in,UINT8 *data_out,UINT32 data_len);
//void gacas_kdf_test(void);
#ifdef __cplusplus
 }
#endif

#endif

