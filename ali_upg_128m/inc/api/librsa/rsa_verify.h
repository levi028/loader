/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: rsa_verify.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _RSA_VERIFY_H_
#define _RSA_VERIFY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GET_DWORD(i)        (*(volatile UINT32 *)(i))
#define CHIP_IS_3701        (0x3701 == *(unsigned short *)(0xb8000002))
#define CHIP_IS_3503         (0x3503 == *(unsigned short *)(0xb8000002))
#define OTP_ROOT_PVR_KEY_ADDR       (((CHIP_IS_3701)||(CHIP_IS_3503))?  \
                        0x88:0x70)
#define OTP_PVR_KEY_WP_ADDR      (((CHIP_IS_3701)||(CHIP_IS_3503))?  \
                        0x80:0x5F)

#define OTP_PVR_KEY_WP_BIT       (((CHIP_IS_3701)||(CHIP_IS_3503))? \
                    (0x1<<2):(0x1<<9))


#define HMAC_OUT_LENGTH   32
#define HASH_BLOCK_LENGTH   64
#define FIRST_KEY_LENGTH   16
typedef struct hmac_param
{
    UINT8 key[FIRST_KEY_LENGTH];
    UINT8 k0[HASH_BLOCK_LENGTH];
    UINT8 ipad[HASH_BLOCK_LENGTH];
    UINT8 opad[HASH_BLOCK_LENGTH];
    UINT8 hout[HASH_BLOCK_LENGTH];
}HMAC_PARAM, *PHMAC_PARAM;

typedef enum
{
    PK_FROM_FLASH = 0,
    PK_FROM_OTP,
    PK_FROM_SEE_ROOT_RSA,
    PK_FROM_CUST_RSA,
}PK_POS;

/*
* illegal_rsa_verify_func()
*
*/
void illegal_rsa_key_func(void);

/*
* illegal_rsa_verify_func()
*
*/
void illegal_rsa_verify_func(void);

void get_vsc_outter_rsa_pk (UINT8 *pk_buf, UINT32 pk_length);
/*
* bl_generate_store_key_for_pvr() - generate random number by HW and store it
* in OTP PVR key Zone.
*/
void bl_generate_store_key_for_pvr();

/*
* sw_generate_store_key_for_pvr() - generate random number by SW and store it
* in OTP PVR key Zone.
*/
void sw_generate_store_key_for_pvr();

/*set_parameter_from_bl() - Store software PK to firmware by specify DRAM space.
* This function should be called by Bootloader.
UINT32 flag:     reserved
UINT32 *param: reserved
*/
RET_CODE set_parameter_from_bl(UINT32 flag, UINT32 *param);


/*
*get_parameter_from_bl() - get the parameter from Bootloader. It should be
* called in Firmware initialization phase.
*@type_flag:
*      0:  get system software public key,
*           store the public key in the globe paramter
*   others: reserved
*@get_mode: reseved
*/
RET_CODE get_parameter_from_bl(UINT32 type_flag, UINT32 *get_mode) ;

RET_CODE reset_rsa_pub_key(void);

/*
* fetch_sys_pub_key() -  get the system sw public key from target to an internel variable.
* The bootloader shall call it before signature verification.
*@target_flag:
*       0:  from bootloader
*       1:  from OTP 0x4~0x4C
*/
RET_CODE fetch_sys_pub_key(UINT32 target_flag);

/*
* get_sys_pub_key() -  get the system sw public key from bootloader to the spceify buffer.
*  The API for the application to get the public key
*@ target : public key position
*@ pk: the sepcify public key buffer in memory.
*@ len: pk buffer length of public key
*/
RET_CODE get_sys_pub_key(PK_POS target, UINT8 *pk,UINT32 len);

/*fetch_sys_kernel_pub_key() - Store the linux system software public key
* to the specify memory space. BL pass the pk to the u-boot
* by calling this function before entering u-boot.
*
*@target: The target memory address by 0x8xxx_xxxx
*@Return: 0:  success
*         -1  fail
*/
RET_CODE fetch_sys_kernel_pub_key(UINT32 target);

/*get_2nd_pub_key() - get the second public key from bootloader
*@target: second public key position
*@Return: 0:  success
*         -1  fail
*/
RET_CODE get_2nd_pub_key(UINT8 *pk);
/*
* Test_RSA_Ram() - Verify the signature of input data , if pass verification,
*                  return RET_SUCCESS, otherwise, return error NO.
*@addr: the start address for the check data, the data format need
*        follow ALi format,use ALi sign tools can generate the special format
*@len:  data length for verify
*@Return :
*        RET_SUCCESS means signature verfication passing otherwise fail.
*/
RET_CODE test_rsa_ram(const UINT32 addr, const UINT32 len);

/*
* Test_RSA_Ram() - Verify the VSC signature of input data , if pass verification,
*                  return RET_SUCCESS, otherwise, return error NO.
*@addr: the start address for the check data, the data format need
*        follow ALi format,use ALi sign tools can generate the special format
*@len:  data length for verify
*@Return :
*        RET_SUCCESS means signature verfication passing otherwise fail.
*/
RET_CODE test_vsc_rsa_ram(const UINT32 addr, const UINT32 len);

/*
* calculate_hmac() - calculate the input data HMAC
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
* @key : HAMC salt key
*
* note: KEY_1_2 is the PVR tmp. this function is only used in PVR now!
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE calculate_hmac(unsigned char *input, unsigned long length,
                      unsigned char *output, unsigned char *key);

/*
*create_sha_ramdrv() - calculate the input digest.
*@input:  the point of input buffer
*@length: the length of input data
*@output: the output digest
*/
RET_CODE create_sha_ramdrv(UINT8 *input, UINT32 length, UINT32 *output);

/*
* get_sys_pub_key_by_id() -  get the system sw public key from bootloader to the spceify buffer.
*  The API for the application to get the public key
*@ target : public key source, OTP or Flash
*@ key_id : RSA key index
*@ pk: the sepcify public key buffer in memory.
*@ len: pk buffer length of public key
*/
RET_CODE get_sys_pub_key_by_id(PK_POS target, UINT8 key_id,UINT8 *pk,UINT32 len);

/*
* test_rsa_ram_by_id() - Verify the signature of input data , if pass verification,
*                  return RET_SUCCESS, otherwise, return error NO.
*@addr: the start address for the check data, the data format need
*        follow ALi format,use ALi sign tools can generate the special format
*@len:  data length for verify
*@rsa_key_id: RSA key index
*@Return :
*        RET_SUCCESS means signature verfication passing otherwise fail.
*/
RET_CODE test_rsa_ram_by_id(const UINT32 addr, const UINT32 len,UINT8 rsa_key_id);

#ifdef __cplusplus
 }
#endif

#endif
