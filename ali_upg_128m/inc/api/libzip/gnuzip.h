/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: gnuzip.h
*
*    Description: This file contains all function definations about gnuzip.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _GNUZIP_H_
#define _GNUZIP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef uint32
#ifdef _LZMA_UINT32_IS_ULONG
#define uint32 unsigned long
#else
#define uint32 unsigned int
#endif
#endif

#ifdef _LZMA_PROB32
#define cprob uint32
#else
#define cprob unsigned short
#endif

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1
#define LZMA_RESULT_NOT_ENOUGH_MEM 2

#ifdef _LZMA_IN_CB
typedef struct _ilzma_in_callback
{
  int (*read)(void *object, unsigned char **buffer, uint32 *buffer_size);
} ilzma_in_callback;
#endif

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#ifdef _LZMA_OUT_READ
int lzma_decoder_init(
    unsigned char *buffer, uint32 buffer_size,
    int lc, int lp, int pb,
    unsigned char *dictionary, uint32 dictionary_size,
  #ifdef _LZMA_IN_CB
    ilzma_in_callback *in_callback
  #else
    unsigned char *in_stream, uint32 in_size
  #endif
);
#endif

int lzma_decode(
    unsigned char *buffer,
  #ifndef _LZMA_OUT_READ
    uint32 buffer_size,
    int lc, int lp, int pb,
  #ifdef _LZMA_IN_CB
    ilzma_in_callback *in_callback,
  #else
    unsigned char *in_stream, uint32 in_size,
  #endif
  #endif
    unsigned char *out_stream, uint32 out_size,
    uint32 *out_size_processed);

int LzmaDecode(
    unsigned char *buffer,
  #ifndef _LZMA_OUT_READ
    uint32 buffer_size,
    int lc, int lp, int pb,
  #ifdef _LZMA_IN_CB
    ilzma_in_callback *in_callback,
  #else
    unsigned char *in_stream, uint32 in_size,
  #endif
  #endif
    unsigned char *out_stream, uint32 out_size,
    uint32 *out_size_processed);

int gzip_decompress(void *input, void *output);
#ifdef _LZMA_OUT_READ
typedef int (*UN7ZIP_OUT_CALLBACK)(unsigned char **ptr_out, unsigned int *ptr_size);
int un7zip(unsigned char *in, unsigned int in_size,
           unsigned char *buf, unsigned int buf_size,
           unsigned char *out, unsigned int out_size, UN7ZIP_OUT_CALLBACK out_callback);

#else
int un7zip(unsigned char *in, unsigned char *out, unsigned char *buf);
#endif


#ifdef __cplusplus
}
#endif

#endif /* _GNUZIP_H_ */
