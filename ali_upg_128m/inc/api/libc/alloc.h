/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     alloc.h
*
*    Description:This file contains all functions definition of memory operations.
*
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Dec.23.2002       Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_C_ALLOC_H__
#define __LIB_C_ALLOC_H__


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MALLOC
#undef MALLOC
#endif
#define MALLOC(size)                malloc(size)

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC(ptr, len)            realloc(ptr, len)

#ifdef FREE
#undef FREE
#endif
#define FREE(ptr)                   free(ptr)

#define    __malloc_size_t        unsigned int
#define    __malloc_ptrdiff_t  int
#define    __ptr_t        void *

/*  malloc hook functions  */
extern __ptr_t (*__malloc_hook)(__malloc_size_t __size);
/* free hook functions  */
extern void (*__free_hook)(__ptr_t __ptr);
/* realloc hook functions  */
extern __ptr_t (*__realloc_hook)(__ptr_t __ptr, __malloc_size_t __size);

extern int see_heaptop_init(unsigned int addr);

/* Limit of valid info table indices.  */
//extern __malloc_size_t _heaplimit;

/* Current search index for the heap table.  */
//extern __malloc_size_t _heapindex;

/* Free list headers for each fragment size.  */
//extern struct list _fraghead[];

/* malloc initialize flag.  */
//extern int __malloc_initialized;

#ifndef SKIP_ALLOC_DECLARE   //to avoid the conflict with the <stdlib.h>
__ptr_t malloc (__malloc_size_t size);
__ptr_t realloc (__ptr_t ptr, __malloc_size_t size);
void free (__ptr_t ptr);
__ptr_t calloc(__malloc_size_t nelem, __malloc_size_t elsize);

#endif //#ifndef SKIP_ALLOC_DECLARE

//#define MT_BUFF_LEN 500
#ifdef MT_BUFF_LEN/* malloc trace*/
void check_alloc_mem(unsigned long src, unsigned long len);
#else
#define check_alloc_mem(...)
#endif


int __alloc_initialize (unsigned long *handle, __ptr_t start,  __ptr_t end);
int __alloc_cleanup (unsigned long handle);

__ptr_t __malloc (unsigned long handle, __malloc_size_t size);
__ptr_t __calloc(unsigned long handle, __malloc_size_t nelem, __malloc_size_t elsize);
void __free (unsigned long handle, __ptr_t ptr);
__ptr_t __realloc (unsigned long handle, __ptr_t ptr, __malloc_size_t size);
int __get_used_ramsize(unsigned long handle);
int __get_free_ramsize(unsigned long handle, int *free, int *maxblock);

/*heap_get_free_size
* This API can get heap free memory size roughly. It just caculate the continuous free space, 
* non-continuous free space not caculated.
*/
int heap_get_free_size(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_C_ALLOC_H__ */
