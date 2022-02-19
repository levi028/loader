/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: t2malloc.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __T2MALLOC_H
#define __T2MALLOC_H

//#define soc_printf printf
#define DEBUG   0   //1

/*
  Void_t* is the pointer type that malloc should say it returns
*/

#define __STD_C 1

#ifndef void_t
#if (__STD_C || defined(WIN32))
#define void_t      void
#else
#define void_t      char
#endif
#endif /*Void_t*/

#ifndef WIN32
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif
#endif

/* define LACKS_SYS_PARAM_H if your system does not have a <sys/param.h>. */

#define  LACKS_SYS_PARAM_H


//#include <stdio.h>    /* needed for malloc_stats */



#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size */
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))



#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT       (2 * SIZE_SZ)
#endif

/* The corresponding bit mask value */
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)


#ifdef _arch_gba
#    ifndef TRIM_FASTBINS
#        define TRIM_FASTBINS  1
#    endif
#else
#    ifndef TRIM_FASTBINS
#        define TRIM_FASTBINS  0
#    endif
#endif




//#define USE_MALLOC_LOCK
#ifdef USE_MALLOC_LOCK
#define USE_PUBLIC_MALLOC_WRAPPERS
#else
/* #define USE_PUBLIC_MALLOC_WRAPPERS */
#endif



#ifdef USE_PUBLIC_MALLOC_WRAPPERS
#define t2_calloc      public_t2_calloc
#define t2_free        public_t2_free
#define t2_cfree       public_t2_cfree
#define t2_malloc      public_t2_malloc
#define t2_memalign    public_t2_memalign
#define t2_remalloc     public_t2_remalloc
#define t2_valloc      public_t2_valloc
#define t2_pvalloc     public_t2_pvalloc
#define t2_mallinfo    public_t2_mallinfo
#define t2_mallopt     public_t2_mallopt
#define t2_mtrim       public_t2_mtrim
#define t2_mstats      public_t2_mstats
#define t2_musable     public_t2_musable
#define t2_icalloc     public_t2_icalloc
#define t2_icomalloc   public_t2_icomalloc
#endif

#define USE_T2_PREFIX
#ifdef USE_T2_PREFIX
#define public_t2_calloc    t2calloc
#define public_t2_free      t2free
#define public_t2_cfree     t2cfree
#define public_t2_malloc    t2malloc
#define public_t2_memalign  t2memalign
#define public_t2_remalloc   t2realloc
#define public_t2_valloc    t2valloc
#define public_t2_pvalloc   t2pvalloc
#define public_t2_mallinfo  t2mallinfo
#define public_t2_mallopt   t2mallopt
#define public_t2_mtrim     t2malloc_trim
#define public_t2_mstats    t2malloc_stats
#define public_t2_musable   t2malloc_usable_size
#define public_t2_icalloc   t2independent_calloc
#define public_t2_icomalloc t2independent_comalloc
#else /* USE_DL_PREFIX */
#define public_t2_calloc    calloc
#define public_t2_free      free
#define public_t2_cfree     cfree
#define public_t2_malloc    malloc
#define public_t2_memalign  memalign
#define public_t2_remalloc   realloc
#define public_t2_valloc    valloc
#define public_t2_pvalloc   pvalloc
#define public_t2_mallinfo  mallinfo
#define public_t2_mallopt   mallopt
#define public_t2_mtrim     malloc_trim
#define public_t2_mstats    malloc_stats
#define public_t2_musable   malloc_usable_size
#define public_t2_icalloc   independent_calloc
#define public_t2_icomalloc independent_comalloc
#endif /* USE_DL_PREFIX */



#define HAVE_MEMCPY

#ifndef USE_MEMCPY
#ifdef HAVE_MEMCPY
#define USE_MEMCPY 1
#else
#define USE_MEMCPY 0
#endif
#endif


#if (__STD_C || defined(HAVE_MEMCPY))

//#include "baka/string.h"



/* #ifndef MALLOC_FAILURE_ACTION
#if __STD_C
#define MALLOC_FAILURE_ACTION \
   errno = ENOMEM;

#else
#define MALLOC_FAILURE_ACTION
#endif
#endif */

#if DEBUG
#define MALLOC_FAILURE_ACTION \
    DBG_PRINTF("malloc: core exhausted\n"); \
    *(unsigned int *)1 = 0;    while(1);
#else
#define MALLOC_FAILURE_ACTION   DBG_PRINTF("malloc: core exhausted\n")
#endif

/*
  MORECORE-related declarations. By default, rely on sbrk
*/


#ifdef LACKS_UNISTD_H
#if !defined(__free_bsd__) && !defined(__open_bsd__) && !defined(__net_bsd__)
#if __STD_C
extern void_t*     sbrk(ptrdiff_t);
#else
extern void_t*     sbrk();
#endif
#endif
#endif


#ifndef MORECORE
#define MORECORE t2_sbrk
#endif


#ifndef MORECORE_FAILURE
#define MORECORE_FAILURE (-1)
#endif


#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif


#define HAVE_MMAP 0

#ifndef HAVE_MMAP
#define HAVE_MMAP 1


#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif

#else /* no mmap */
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 0
#endif
#endif



#ifndef MMAP_AS_MORECORE_SIZE
#define MMAP_AS_MORECORE_SIZE (1024 * 1024)
#endif


#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#else
#define HAVE_MREMAP 0
#endif

#endif /* HAVE_MMAP */


#define malloc_getpagesize 4096        /* XXX: Should we make this 16384
                       for cache considerations? */

/* SVID2/XPG mallinfo structure */

struct mallinfo {
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

#endif


#if __STD_C
void_t*  public_t2_malloc(size_t);
#else
void_t*  public_t2_malloc();
#endif

#if __STD_C
void     public_t2_free(void_t*);
#else
void     public_t2_free();
#endif

#if __STD_C
void_t*  public_t2_calloc(size_t, size_t);
#else
void_t*  public_t2_calloc();
#endif

#if __STD_C
void_t*  public_t2_remalloc(void_t*, size_t);
#else
void_t*  public_t2_remalloc();
#endif

#if __STD_C
void_t*  public_t2_memalign(size_t, size_t);
#else
void_t*  public_t2_memalign();
#endif

#if __STD_C
void_t*  public_t2_valloc(size_t);
#else
void_t*  public_t2_valloc();
#endif

#if __STD_C
int      public_t2_mallopt(int, int);
#else
int      public_t2_mallopt();
#endif


#if __STD_C
struct mallinfo public_t2_mallinfo(void);
#else
struct mallinfo public_t2_mallinfo();
#endif

#if __STD_C
void_t** public_t2_icalloc(size_t, size_t, void_t**);
#else
void_t** public_t2_icalloc();
#endif

#if __STD_C
void_t** public_t2_icomalloc(size_t, size_t*, void_t**);
#else
void_t** public_t2_icomalloc();
#endif


#if __STD_C
void_t*  public_t2_pvalloc(size_t);
#else
void_t*  public_t2_pvalloc();
#endif

#if __STD_C
void     public_t2_cfree(void_t*);
#else
void     public_t2_cfree();
#endif

#if __STD_C
int      public_t2_mtrim(size_t);
#else
int      public_t2_mtrim();
#endif

#if __STD_C
size_t   public_t2_musable(void_t*);
#else
size_t   public_t2_musable();
#endif

#if __STD_C
void     public_t2_mstats();
#else
void     public_t2_mstats();
#endif


/* M_MXFAST is a standard SVID/XPG tuning option, usually listed in malloc.h */
#ifndef M_MXFAST
#define M_MXFAST            1
#endif

#ifndef DEFAULT_MXFAST
#define DEFAULT_MXFAST     64
#endif



#define M_TRIM_THRESHOLD       -1

#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (128 * 1024)
#endif


#define M_TOP_PAD              -2

#ifndef DEFAULT_TOP_PAD
#define DEFAULT_TOP_PAD        (0)
#endif


#define M_MMAP_THRESHOLD      -3

#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD (128 * 1024)
#endif


#define M_MMAP_MAX             -4

#ifndef DEFAULT_MMAP_MAX
#if HAVE_MMAP
#define DEFAULT_MMAP_MAX       (65536)
#else
#define DEFAULT_MMAP_MAX       (0)
#endif
#endif


#endif    /* __T2MALLOC_H */
