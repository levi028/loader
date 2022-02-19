/*****************************************************************************
*    Copyright (c) 2015 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: generate_hmac.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _GENERATE_HMAC_H_
#define _GENERATE_HMAC_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 1
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

#endif

#if 0
/*
* api_gen_stbinfo_chunk_hmac() - calculate the input data HMAC (specific for data of stbinfo chunk only)
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
*
* note: HAMC salt key is  constant data.
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE api_gen_stbinfo_chunk_hmac(UINT8 *input, const UINT32 len, UINT8 *output);
#endif

/*
* api_gen_hmac_ext() - calculate the input data HMAC (will check need use OTP HMAC key or not)
* if use OTP key, input HMAC key will be ignored
*/
RET_CODE api_gen_hmac_ext(UINT8 *input, const UINT32 len, UINT8 *output, UINT8 *key);

/*
* api_gen_hmac1_hdrchunk() - calculate the input data HMAC (specific for whole stbinfo header chunk only, header included)
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
*
* note: HAMC salt key is  constant data.
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE api_gen_hmac1_hdrchunk(UINT8 *input, const UINT32 len, UINT8 *output);

/*
* api_gen_hmac2_allhdr() - calculate the input data HMAC (specific for all chunk header only)
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
*
* note: HAMC salt key is  constant data.
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE api_gen_hmac2_allhdr(UINT8 *input, const UINT32 len, UINT8 *output);

/*
* api_gen_hmac3_chunkdata() - calculate the input data HMAC (specific for some specific data chunk only)
* @input : zone specified
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
*
* note: HAMC salt key is  constant data.
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE api_gen_hmac3_chunkdata(UINT8 seq,UINT8 *input, const UINT32 len, UINT8 *output);

#ifdef __cplusplus
 }
#endif

#endif
