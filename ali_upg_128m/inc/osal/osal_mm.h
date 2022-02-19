/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_mm.h
*
*    Description:
*    This header file contains the definitions about memory management functions
*    of abstract os layer. If there is no memory managed by os, the default glibc
*    MALLOC/FREE is available.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_MM_H__
#define __OSAL_MM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <api/libc/alloc.h>
//#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)
//#include <os/tds2/itron.h>

#define osal_memory_allocate            MALLOC
#define osal_memory_free                FREE

#define osal_mm_pa_to_va(pa)			((void *)((UINT32)(pa) | 0xA0000000))
#define osal_mm_va_to_pa(va)			((void *)((UINT32)(va) & 0x1fffffff))

static inline unsigned long va_to_pa(unsigned long va)
{
	unsigned long pa;
	if (ALI_M3921 == sys_ic_get_chip_id()){
		return va & 0x3FFFFFFF; 
	} else {
		if (va < 0x80000000)
			pa = va + 0x40000000;
		else if (va >= 0x80000000 && va < 0xa0000000)
			pa = va - 0x80000000;
		else if (va >= 0xa0000000 && va < 0xc0000000)
			pa = va - 0xa0000000;
		else
			pa = va;
	}

	return pa;
}

static inline unsigned long pa_to_va(unsigned long pa)
{
	unsigned long va;

	if (ALI_M3921 == sys_ic_get_chip_id()){
		return pa & 0x3FFFFFFF; 
	}
	else {
		if (pa < 0x40000000)
			va = pa + 0x80000000;
		else if (pa >= 0x40000000 && pa < 0xc0000000)
			va = pa - 0x40000000;
		else
			va = pa;

		return va;
	}
}

static inline unsigned long dma_to_pa(unsigned long dma)
{
	if (ALI_M3921 == sys_ic_get_chip_id()){
		return dma & 0x3FFFFFFF; 
	} else {
		return dma | 0x80000000;
	}
}

static inline unsigned long pa_to_dma(unsigned long pa)
{
	unsigned long dma = pa;
	if (ALI_M3921 == sys_ic_get_chip_id()){
		return pa & 0x3FFFFFFF; 
	} else {		
		if (pa >= 0x80000000)
			dma = pa - 0x80000000;	
	}

	return dma;	
}

static inline unsigned long dma_to_va(unsigned long dma)
{
	return pa_to_va((dma_to_pa(dma)));
}

static inline unsigned long va_to_dma(unsigned long va)
{
	return pa_to_dma(va_to_pa(va));
}


#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_memory_allocate(...)        NULL
#define osal_memory_free(...)            OSAL_E_FAIL

#define osal_mm_pa_to_va(pa)            (pa)
#define osal_mm_va_to_pa(va)            (va)

#else

#define osal_memory_allocate            MALLOC
#define osal_memory_free                FREE

#define osal_mm_pa_to_va(pa)            (pa)
#define osal_mm_va_to_pa(va)            (va)

#endif

extern UINT32 heap_start;
extern void *heap_ptr;
extern char __RAM_BASE[];
extern char __RODATA_RAM_ADDR[];

extern char  _end[];


#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_MM_H__ */
