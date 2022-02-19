/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: hdcp_des.h
*
*    Description: 3DES Encryption/Decryption function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __HDCP_DES_H__
#define __HDCP_DES_H__
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HDCP_FROM_CE
void des_encryption(unsigned char *plaintext, unsigned char *ciphertext, unsigned char *key);
void des_decryption(unsigned char *ciphertext, unsigned char *plaintext, unsigned char *key);
void triple_des_encryption(unsigned char *plaintext, unsigned char *ciphertext,
                            unsigned char *key1, unsigned char *key2, unsigned char *key3);
void triple_des_decryption(unsigned char *ciphertext, unsigned char *plaintext,
                            unsigned char *key1, unsigned char *key2, unsigned char *key3);
#endif

#ifdef __cplusplus
 }
#endif
#endif

