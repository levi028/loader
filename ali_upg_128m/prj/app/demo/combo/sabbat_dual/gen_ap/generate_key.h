/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: generate_key.h
*
*    Description: This file contains the callback functions for re-encryption of transport stream
                 and encryption and HMAC of related metadata
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __GENERATE_KEY_H__
#define __GENERATE_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _C0700A_VSC_ENABLE_
#define GEN_CA_M2M2_KEY_POS                KEY_0_2//KEY_0_1
#define GEN_CA_PVR_KEY_POS                KEY_1_2//KEY_1_1
#define GEN_CA_CRYPTO_KEY_LEVEL            ONE_LEVEL//TWO_LEVEL
#else
#define GEN_CA_M2M2_KEY_POS                KEY_0_1
#define GEN_CA_PVR_KEY_POS                KEY_1_1
#define GEN_CA_CRYPTO_KEY_LEVEL            TWO_LEVEL
#endif

struct gen_ca_pvr_crypto_param
{
	UINT32    crypto_key_pos;	//key position
	UINT16    *pid_list;			//PID list
   	UINT16    pid_num;			//PID number
   	UINT32    stream_id;			//stream ID
   	UINT32    crypto_dev_id;		//device ID
	UINT32    prog_id;			//record channel id
};

/*****************************************************************************
 * Function: gen_ca_load_m2m2_key
 * Description:
 *    get root key from OTP
 * Input:
 *        None
 * Output:
 *        None
 *
 * Returns:   0 : success , -1 : failure
 *
*****************************************************************************/
int gen_ca_load_m2m2_key(void);
void gen_ca_clear_crypto_info(UINT32 prog_id);
/*****************************************************************************
 * Function: gen_ca_load_pk_to_ce
 * Description:
 *    generate PVR key
 * Input:
 *        None
 * Output:
 *        None
 *
 * Returns:   0 : success , -1 : failure
 *
*****************************************************************************/
int gen_ca_load_pk_to_ce(void);
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
BOOL gen_ca_p_set_pvr_key(gen_ca_key_info *key_info , UINT32 key_pos , UINT32 first_key_pos);
/*****************************************************************************
 * Function: gen_ca_r_set_pvr_crypto_para
 * Description:
 *    find free key position. Then , calling ¡§gen_ca_r_set_pvr_key¡¨ and ¡§gen_ca_aes_key_map_ex¡¨ 
 * Input:
 *        Para 1: gen_ca_pvr_crypto_param *param , re-encrypt parameter
 * Output:
 *        Para 1: gen_ca_pvr_crypto_param *param , re-encrypt parameter
 *        Para 2: UINT8 *key_data , encrypted data for generating recording key
 *
 * Returns:   UINT32 crypto_stream_hnd , Crypto stream handle
 *
*****************************************************************************/
UINT32 gen_ca_r_set_pvr_crypto_para(struct gen_ca_pvr_crypto_param *param, UINT8 *key_data);
/*****************************************************************************
 * Function: gen_ca_p_set_pvr_crypto_para
 * Description:
 *    set decrypt parameter when playback
 * Input:
 *        Para 1: gen_ca_pvr_crypto_param *param , decrypt parameter
 *        Para 2: gen_ca_key_info *key_info , encrypted data for generating recording key
 * Output:
 *        Para 1: gen_ca_pvr_crypto_param *param , decrypt parameter
 *
 * Returns:   UINT32 crypto_stream_hnd , Decrypt  stream handle
 *
*****************************************************************************/
UINT32 gen_ca_p_set_pvr_crypto_para(struct gen_ca_pvr_crypto_param *param , gen_ca_key_info *key_info);
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
void change_key(UINT32 prog_id);

#ifdef __cplusplus
}
#endif

#endif /* __GENERATE_KEY_H__ */

