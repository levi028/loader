/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: stdio.h
*
*    Description: declaration of stdio
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __VFS_STDIO_H__
#define __VFS_STDIO_H__

#include <sys_config.h>

#ifdef FS_STDIO_SUPPORT

#include "lib_stdio.h"

#else

#include "types.h"

struct __stdio_file
{
    int fd;
    int flags;
    unsigned int bs;            /* read: bytes in buffer */
    unsigned int bm;            /* position in buffer */
    unsigned int buflen;        /* length of buf */
    char *buf;
    unsigned char ungetbuf;
    char ungotten;
#ifdef WANT_THREAD_SAFE
    pthread_mutex_t m;
#endif
};

typedef struct __stdio_file FILE;

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define EOF (-1)

#ifdef __cplusplus
extern "C"
{
#endif
FILE *fopen (const char *path, const char *mode) ;
off_t ftell(FILE *stream);
int fseek(FILE *stream, off_t offset, int whence);
void rewind(FILE *stream) ;
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) ;
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) ;
int fflush(FILE *stream) ;
int fclose(FILE *stream) ;
int feof(FILE *stream) ;
int ferror(FILE *stream) ;
#ifdef __cplusplus
 }
#endif

#endif

#endif// __VFS_STDIO_H__


