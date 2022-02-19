/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     alloc.c
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

#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <errno.h>


#ifndef CHAR_BIT
#define CHAR_BIT    8
#endif
#define INT_BIT     (CHAR_BIT * sizeof(int))
#define BLOCKLOG    (INT_BIT > 16 ? 12 : 9)
#define BLOCKSIZE   (1 << BLOCKLOG)
#define HALFBLOCKSIZE   (BLOCKSIZE>>1)  //BLOCKSIZE/2
#define BLOCKIFY(SIZE)  (((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)
#define BASE_2      2
#ifdef SUNOS_LOCALTIME_BUG
#define NUM_16      16
#endif

/* Address to block number and vice versa.  */
#define BLOCK(A)    (((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B)    ((__ptr_t) (((B) - 1) * BLOCKSIZE + _heapbase))

#define DRAM_SPLIT_CTRL_BASE 0xb8041000
#define PVT_S_ADDR 0x10
#define SHR_S_ADDR 0x18



/* Determine the amount of memory spanned by the initial heap table
(not an absolute limit).  */
#ifdef SYS_SDRAM_SIZE
#if(SYS_SDRAM_SIZE>=64)
#define HEAP        0x2000000
#else
#define HEAP        (INT_BIT > 16 ? 4194304 : 65536)
#endif
#else
#define HEAP        (INT_BIT > 16 ? 4194304 : 65536)
#endif

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* Number of contiguous free blocks allowed to build up at the end of
memory before they will be returned to the system.  */
#define FINAL_FREE_BLOCKS    8
#define    CADDR_T        void *

typedef void *POINTER;

/* Data structure giving per-block information.  */
typedef union
{
    /* Heap information for a busy block.  */
    struct
    {
        /* Zero for a large (multiblock) object, or positive giving the
        logarithm to the base two of the fragment size.  */
        int type;
        union
        {
            struct
            {
                __malloc_size_t nfree; /* Free frags in a fragmented block.  */
                __malloc_size_t first; /* First free fragment of the block.  */
            } frag;
            /* For a large object, in its first block, this has the number
            of blocks in the object.  In the other blocks, this has a
            negative number which says how far back the first block is.  */
            __malloc_ptrdiff_t size;
        } info;
    } busy;
    /* Heap information for a free block
    (that may be the first of a free cluster).  */
    struct
    {
        __malloc_size_t size;    /* Size (in blocks) of a free cluster.  */
        __malloc_size_t next;    /* Index of next free cluster.  */
        __malloc_size_t prev;    /* Index of previous free cluster.  */
    } free;
} malloc_info;


/* Current search index for the heap table.  */
//extern __malloc_size_t _heapindex;

/* Limit of valid info table indices.  */
//extern __malloc_size_t _heaplimit;

/* Doubly linked lists of free fragments.  */
struct list
{
    struct list *next;
    struct list *prev;
};

/* Free list headers for each fragment size.  */
//extern struct list _fraghead[];

/* List of blocks allocated with `memalign' (or `valloc').  */
struct alignlist
{
    struct alignlist *next;
    __ptr_t aligned;        /* The address that memaligned returned.  */
    __ptr_t exact;        /* The address that malloc returned.  */
};
static struct alignlist *_aligned_blocks = NULL;

/* Pointer to the base of the first block.  */
static char *_heapbase = NULL;

/* Block information table.  Allocated with align/__free (not malloc/free).  */
static malloc_info *_heapinfo = NULL;

/* Number of info entries.  */
static __malloc_size_t heapsize = 0;

/* Search index in the info table.  */
static __malloc_size_t _heapindex = 0;

/* Limit of valid info table indices.  */
static __malloc_size_t _heaplimit = 0;

/* Free lists for each fragment size.  */
static struct list _fraghead[BLOCKLOG];

/* Instrumentation.  */
static __malloc_size_t _chunks_used = 0;
static __malloc_size_t _bytes_used = 0;
static __malloc_size_t _chunks_free = 0;
static __malloc_size_t _bytes_free = 0;


//typedef void* caddr_t;
#if defined(_BL_ENABLE_) && (!defined (SEE_CPU))
CADDR_T RAMSIZE = (CADDR_T)(0x87000000);//to save heap top addr
#else
CADDR_T RAMSIZE = (CADDR_T)(0x81000000);//to save heap top addr
#endif
static int error_no = 0;

/* Are you experienced?  */
static void* _sbrk(int nbytes);

static int __malloc_initialized = 0;
static POINTER (*__morecore) () = _sbrk;

static void _free_internal (__ptr_t ptr);

#ifdef MT_BUFF_LEN /* malloc trace  --Doy.Dong, 2014-12-31*/
struct __mtrace
{
    void *mem; // memory address
    void *fun; // return function address
    UINT32 len;//alloc len
};
static struct __mtrace g_malloc_trace[MT_BUFF_LEN];
static UINT32 g_mt_enable = 0;

#define MALLOC_TRACE(p, L) do{\
        if(g_mt_enable){\
            UINT32 i=0;\
            for(i=0;i<MT_BUFF_LEN;i++){\
                if(0==g_malloc_trace[i].mem){\
                    g_malloc_trace[i].fun=__builtin_return_address(0);\
                    g_malloc_trace[i].mem=p;\
                    g_malloc_trace[i].len=L;\
                    break;\
                }\
            }\
        }\
   }while(0);

#define MALLOC_RELEASE(p) do{\
        if(g_mt_enable){\
            UINT32 i=0;\
            for(i=0;i<MT_BUFF_LEN;i++){\
                if(p==g_malloc_trace[i].mem){\
                    g_malloc_trace[i].mem=0;\
                    g_malloc_trace[i].fun=0;\
                    g_malloc_trace[i].len=0;\
                    break;\
                }\
            }\
        }\
    }while(0);

// This function use to check the memset/memcpy dest memory space whether overlap(larger than alloc space)
void check_alloc_mem(unsigned long src, unsigned long len)
{
    unsigned long i=0;
    signed long find_start = -1;
    signed long find_end =-1;

    if(g_mt_enable == 0)
        return;
    
    if((src ==0) || (len==0))
    {
        //ASSERT(0);
        return;
    }

    if((src>=(UINT32)RAMSIZE) || (!__malloc_initialized))
        return;
 
    for(i=0;i<MT_BUFF_LEN;i++)
    {
        if(g_malloc_trace[i].mem == 0)
            continue;
        if(find_start ==-1)
        {
            if((src>=(UINT32)g_malloc_trace[i].mem) && (src<(UINT32)(g_malloc_trace[i].mem+g_malloc_trace[i].len)))
                find_start = i;
        }
        if(find_end == -1)
        {
            if(((src+len)>(UINT32)g_malloc_trace[i].mem) && ((src+len)<=(UINT32)(g_malloc_trace[i].mem+g_malloc_trace[i].len)))
                find_end = i;
        }
        if((find_start>=0)&& (find_end>=0))
        {
            if(find_start == find_end)
               	return;
            else
            {
				libc_printf("check_alloc_mem error(find_start %d find_end %d), going to cover the space:\n");
				libc_printf("[0x%X, 0x%X], overlap [0x%X, 0x%X][0x%X, 0x%X]\n", find_start, find_end, src, src+len, 
					g_malloc_trace[find_start].mem, g_malloc_trace[find_start].mem+g_malloc_trace[find_start].len,
					g_malloc_trace[find_end].mem, g_malloc_trace[find_end].mem+g_malloc_trace[find_end].len);
               //ASSERT(0);
               	return;
            }
        }
    }
    if(find_start!=find_end)
    {
		// Can not find the memory space in g_malloc_trace
        libc_printf("check_alloc_mem warning src=0x%x, len=0x%x, find_start %d, find_end %d\n",src, len, find_start, find_end);
        //ASSERT(0);
        return;
    }    
    
}
#else
#define MALLOC_TRACE(p, L)
#define MALLOC_RELEASE(p)
#endif


//hook functions
//__ptr_t (*__malloc_hook)(__malloc_size_t __size);
//void (*__free_hook)(__ptr_t __ptr);
//__ptr_t (*__realloc_hook)(__ptr_t __ptr, __malloc_size_t __size);


/*  malloc hook functions  */
 __ptr_t (*__malloc_hook)(__malloc_size_t __size) = NULL;
/* free hook functions  */
 void (*__free_hook)(__ptr_t __ptr)  = NULL;
/* realloc hook functions  */
 __ptr_t (*__realloc_hook)(__ptr_t __ptr, __malloc_size_t __size)  = NULL;

/* Aligned allocation.  */
static __ptr_t align (__malloc_size_t size)
{
    __ptr_t result = NULL;
    unsigned long int adj = 0;

    if(UINT_MAX == size)
    {
        return result;
    }

    result = (*__morecore) (size);
    if(0xffffffff==(UINT32)result)
    {
        return NULL;
    }
    adj = (unsigned long int) ((unsigned long int) ((char *) result -
                               (char *) NULL)) % BLOCKSIZE;
    if (adj != 0)
    {
        adj = BLOCKSIZE - adj;
        (void) (*__morecore) (adj);
        result = (char *) result + adj;
    }

    return result;
}


/*
 * _sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
static CADDR_T _sbrk(nbytes)
          int nbytes;
{
    CADDR_T        base = NULL;

    //osal_interrupt_disable();

    if(RAMSIZE > heap_ptr + nbytes)
    {
        base = heap_ptr;
        heap_ptr += nbytes;
        //osal_interrupt_enable();
        return (base);
    }
    else
    {
        error_no = ENOMEM;
        //ASSERTMSG(0, "Heap overflow!");
        return ((CADDR_T)-1);
    }

}

int see_heaptop_init(unsigned int addr)
{
    if(UINT_MAX != addr)
    {
        RAMSIZE = (CADDR_T)(addr & 0x8FFFFFFF);
    }
    return 0;
}

static int initialize (void)
{
#ifdef BL_SECURITY_CONFIG
    #ifdef SEE_CPU
        RAMSIZE = (CADDR_T)((*(UINT32 *)(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR)|0x80000000)& 0X8FFFFFFF);
    #else
        RAMSIZE = (CADDR_T)((*(UINT32 *)(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR)|0x80000000)& 0X8FFFFFFF);
    #endif
#else
#ifndef SEE_CPU
    RAMSIZE = (CADDR_T)(__MM_HEAP_TOP_ADDR & 0X8FFFFFFF);
    #ifdef DDR_256M_SUPPORT
    /*set 256M~128M For heap use 0xa8000000&0x8FFFFFFF*/
    heap_ptr = 0x88000000;
    heap_start = 0x88000000;
    #endif
#else
    RAMSIZE = (CADDR_T)(0X8FFFFFFF);
#endif
#endif
    heapsize = HEAP / BLOCKSIZE;
    _heapinfo = (malloc_info *) align (heapsize * sizeof (malloc_info));
    if (NULL == _heapinfo)
    {
        return 0;
    }

    MEMSET (_heapinfo, 0, heapsize * sizeof (malloc_info));
    _heapinfo[0].free.size = 0;
    _heapinfo[0].free.next = 0;
    _heapinfo[0].free.prev = 0;
    _heapindex = 0;
    _heapbase = (char *) _heapinfo;

    /* Account for the _heapinfo block itself in the statistics.  */
    _bytes_used = heapsize * sizeof (malloc_info);
    _chunks_used = 1;

    __malloc_initialized = 1;
    return 1;
}

/* Get neatly aligned memory, initializing or
growing the heap info table as necessary. */
static __ptr_t morecore (__malloc_size_t size)
{
    __ptr_t result = NULL;
    malloc_info *newinfo = NULL;
    malloc_info *oldinfo = NULL;
    __malloc_size_t newsize = 0;

    result = align (size);
    if (NULL == result)
    {
        return NULL;
    }

    /* Check if we need to grow the info table.  */
    if ((__malloc_size_t) BLOCK ((char *) result + size) > heapsize)
    {
        newsize = heapsize;
        while ((__malloc_size_t) BLOCK ((char *) result + size) > newsize)
        {
            newsize *= 2;
        }
        newinfo = (malloc_info *) align (newsize * sizeof (malloc_info));
        if (NULL == newinfo)
        {
            (*__morecore) (-size);
            return NULL;
        }
        MEMCPY(newinfo, _heapinfo, heapsize * sizeof (malloc_info));
        MEMSET(&newinfo[heapsize], 0,(newsize-heapsize) * sizeof(malloc_info));
        oldinfo = _heapinfo;
        newinfo[BLOCK (oldinfo)].busy.type = 0;
        newinfo[BLOCK (oldinfo)].busy.info.size
        = BLOCKIFY (heapsize * sizeof (malloc_info));
        _heapinfo = newinfo;
        /* Account for the _heapinfo block itself in the statistics.  */
        _bytes_used += newsize * sizeof (malloc_info);
        ++_chunks_used;
        _free_internal (oldinfo);
        heapsize = newsize;
    }

    _heaplimit = BLOCK ((char *) result + size);
    return result;
}



/* Allocate memory from the heap.  */
__ptr_t malloc (__malloc_size_t size)
{
    __ptr_t result = NULL;
    __malloc_size_t block = 0;
    __malloc_size_t blocks = 0;
    __malloc_size_t lastblocks = 0;
    __malloc_size_t start = 0;
    register __malloc_size_t i = 0;
    struct list *next = NULL;

    /* ANSI C allows `malloc (0)' to either return NULL, or to return a
       valid address you can realloc and free (though not dereference).
    It turns out that some extant code (sunrpc, at least Ultrix's version)
    expects `malloc (0)' to return non-NULL and breaks otherwise. Be compatible.  */
    if(UINT_MAX == size)
    {
        return result;
    }
    osal_task_dispatch_off();
    __MAYBE_UNUSED__ UINT32 test_len = size;
    if (__malloc_hook != NULL)
    {
        result =  (*__malloc_hook) (size);
		MALLOC_TRACE(result, test_len);
        osal_task_dispatch_on();
        return result;
    }

    if ((!__malloc_initialized) && (!initialize ()))
    {
        osal_task_dispatch_on();
        return NULL;
    }

    if (size < sizeof (struct list))    // round up
    {
        size = sizeof (struct list);
    }

#ifdef SUNOS_LOCALTIME_BUG
    if (size < NUM_16)
        size = NUM_16;
#endif

    /* Determine the allocation policy based on the request size.  */
    if (size <= HALFBLOCKSIZE)    //2 need fragment of a block
    {
        /* Small allocation to receive a fragment of a block.
        Determine the logarithm to base two of the fragment size. */
        register __malloc_size_t log = 1;

        --size;
        while ((size /= BASE_2) != 0)
        {
            ++log;
        }
        /* Look in the fragment lists for a free fragment of the desired size. */
        next = _fraghead[log].next;
        if (next != NULL)
        {
            /* There are free fragments of this size.
            Pop a fragment out of the fragment list and return it.
            Update the block's nfree and first counters. */
            result = (__ptr_t) next;
            next->prev->next = next->next;
            if (next->next != NULL)
            {
                next->next->prev = next->prev;
            }
            block = BLOCK (result);
            if (--_heapinfo[block].busy.info.frag.nfree != 0)
            {
                _heapinfo[block].busy.info.frag.first = (unsigned long int)((unsigned long int)
                                                ((char *)next->next-(char *)NULL) % BLOCKSIZE) >> log;
            }

            /* Update the statistics.  */
            ++_chunks_used;
            _bytes_used += 1 << log;
            --_chunks_free;
            _bytes_free -= 1 << log;
        }
        else
        {
            /* No free fragments of the desired size, so get a new block
            and break it into fragments, returning the first.  */
            result = (__ptr_t)malloc (BLOCKSIZE);    //3 only once recursive self-call
            if (NULL == result)
            {
                osal_task_dispatch_on();
                return NULL;
            }

            /* Link all fragments but the first into the free list.  */
            for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> log); ++i)
            {
                next = (struct list *) ((char *) result + (i << log));
                next->next = _fraghead[log].next;
                next->prev = &_fraghead[log];
                next->prev->next = next;
                if (next->next != NULL)
                {
                    next->next->prev = next;
                }
            }

            /* Initialize the nfree and first counters for this block.  */
            block = BLOCK (result);
            _heapinfo[block].busy.type = log;
            _heapinfo[block].busy.info.frag.nfree = i - 1;
            _heapinfo[block].busy.info.frag.first = i - 1;

            _chunks_free += (BLOCKSIZE >> log) - 1;
            _bytes_free += BLOCKSIZE - (1 << log);
            _bytes_used -= BLOCKSIZE - (1 << log);
        }
    }
    else    //2 need one or more blocks
    {
        /* Large allocation to receive one or more blocks.
        Search the free list in a circle starting at the last place visited.
        If we loop completely around without finding a large enough
        space we will have to get more memory from the system.  */
        blocks = BLOCKIFY (size);
        start = _heapindex;
        block = _heapindex;
        while (_heapinfo[block].free.size < blocks)
        {
            block = _heapinfo[block].free.next;
            if (block == start)
            {
                /* Need to get more from the system.  Check to see if
                the new core will be contiguous with the final free
                block; if so we don't need to get as much.  */
                block = _heapinfo[0].free.prev;
                lastblocks = _heapinfo[block].free.size;
                if ((_heaplimit != 0) && (block + lastblocks == _heaplimit) &&
                     ((*__morecore) (0) == ADDRESS (block + lastblocks)) &&
                     ((morecore ((blocks - lastblocks) * BLOCKSIZE)) != NULL))
                {
                    /* Which block we are extending (the `final free
                    block' referred to above) might have changed, if
                    it got combined with a freed info table.  */
                    block = _heapinfo[0].free.prev;
                    _heapinfo[block].free.size += (blocks - lastblocks);
                    _bytes_free += (blocks - lastblocks) * BLOCKSIZE;
                    continue;
                }
                result = morecore (blocks * BLOCKSIZE);
                if (NULL == result)
                {
                    osal_task_dispatch_on();
                    return NULL;
                }
                block = BLOCK (result);
                _heapinfo[block].busy.type = 0;
                _heapinfo[block].busy.info.size = blocks;
                ++_chunks_used;
                _bytes_used += blocks * BLOCKSIZE;
				MALLOC_TRACE(result, test_len);
                osal_task_dispatch_on();
                return result;
            }
        }

        /* At this point we have found a suitable free list entry.
        Figure out how to remove what we need from the list. */
        result = ADDRESS (block);
        if (_heapinfo[block].free.size > blocks)
        {
            /* The block we found has a bit left over,
            so relink the tail end back into the free list. */
            _heapinfo[block + blocks].free.size = _heapinfo[block].free.size - blocks;
            _heapinfo[block + blocks].free.next = _heapinfo[block].free.next;
            _heapinfo[block + blocks].free.prev = _heapinfo[block].free.prev;

            _heapindex = block + blocks;
            _heapinfo[_heapinfo[block].free.next].free.prev = _heapindex;
            _heapinfo[_heapinfo[block].free.prev].free.next = _heapinfo[_heapinfo[block].free.next].free.prev;
        }
        else
        {
            /* The block exactly matches our requirements, so just remove it from the list. */
            _heapinfo[_heapinfo[block].free.next].free.prev = _heapinfo[block].free.prev;
            _heapindex = _heapinfo[block].free.next;
            _heapinfo[_heapinfo[block].free.prev].free.next = _heapindex;
            --_chunks_free;
        }

        _heapinfo[block].busy.type = 0;
        _heapinfo[block].busy.info.size = blocks;
        ++_chunks_used;
        _bytes_used += blocks * BLOCKSIZE;
        _bytes_free -= blocks * BLOCKSIZE;

        /* Mark all the blocks of the object just allocated except for the
        first with a negative number so you can find the first block by
        adding that adjustment.  */
        while (--blocks > 0)
        {
            _heapinfo[block + blocks].busy.info.size = -blocks;
        }
    }
	MALLOC_TRACE(result,test_len);
    osal_task_dispatch_on();
    return result;
}


static void _free_internal (__ptr_t ptr)
{
    int type = 0;
    __malloc_size_t block = 0;
    __malloc_size_t blocks = 0;
    register __malloc_size_t i = 0;
    struct list *prev = NULL;
    struct list *next = NULL;

    if(ptr == NULL)
        return;
	MALLOC_RELEASE(ptr);
    block = BLOCK (ptr);
    if(NULL == _heapinfo)
    {
        ASSERT(0);
        return;
    }
    type = _heapinfo[block].busy.type;
    switch (type)
    {
    case 0:
        /* Get as many statistics as early as we can.  */
        --_chunks_used;
        _bytes_used -= _heapinfo[block].busy.info.size * BLOCKSIZE;
        _bytes_free += _heapinfo[block].busy.info.size * BLOCKSIZE;

        /* Find the free cluster previous to this one in the free list.
        Start searching at the last block referenced; this may benefit
        programs with locality of allocation.  */
        i = _heapindex;
        if (i > block)
        {
            while (i > block)
            {
                i = _heapinfo[i].free.prev;
            }
        }
        else
        {
            do
            {
                i = _heapinfo[i].free.next;
            }while ((i > 0) && (i < block));

            i = _heapinfo[i].free.prev;
        }

        /* Determine how to link this block into the free list.  */
        if (block == i + _heapinfo[i].free.size)
        {
            /* Coalesce this block with its predecessor.  */
            _heapinfo[i].free.size += _heapinfo[block].busy.info.size;
            block = i;
        }
        else
        {
            /* Really link this block back into the free list.  */
            _heapinfo[block].free.size = _heapinfo[block].busy.info.size;
            _heapinfo[block].free.next = _heapinfo[i].free.next;
            _heapinfo[block].free.prev = i;
            _heapinfo[i].free.next = block;
            _heapinfo[_heapinfo[block].free.next].free.prev = block;
            ++_chunks_free;
        }

        /* Now that the block is linked in, see if we can coalesce it
        with its successor (by deleting its successor from the list
        and adding in its size).  */
        if (block + _heapinfo[block].free.size == _heapinfo[block].free.next)
        {
            _heapinfo[block].free.size
            += _heapinfo[_heapinfo[block].free.next].free.size;
            _heapinfo[block].free.next
            = _heapinfo[_heapinfo[block].free.next].free.next;
            _heapinfo[_heapinfo[block].free.next].free.prev = block;
            --_chunks_free;
        }

        /* Now see if we can return stuff to the system.  */
        blocks = _heapinfo[block].free.size;
        if ((blocks >= FINAL_FREE_BLOCKS) && (block + blocks == _heaplimit)
                && ((*__morecore) (0) == ADDRESS (block + blocks)))
        {
            register __malloc_size_t bytes = blocks * BLOCKSIZE;

            _heaplimit -= blocks;
            (*__morecore) (-bytes);
            _heapinfo[_heapinfo[block].free.prev].free.next
            = _heapinfo[block].free.next;
            _heapinfo[_heapinfo[block].free.next].free.prev
            = _heapinfo[block].free.prev;
            block = _heapinfo[block].free.prev;
            --_chunks_free;
            _bytes_free -= bytes;
        }

        /* Set the next search to begin at this block.  */
        _heapindex = block;
        break;

    default:
        /* Do some of the statistics.  */
        --_chunks_used;
        _bytes_used -= 1 << type;
        ++_chunks_free;
        _bytes_free += 1 << type;

        /* Get the address of the first free fragment in this block.  */
        prev = (struct list *)((char *) ADDRESS (block) +
                               (_heapinfo[block].busy.info.frag.first<<type));

        if (_heapinfo[block].busy.info.frag.nfree == (__malloc_size_t)((BLOCKSIZE >> type) - 1))
        {
            /* If all fragments of this block are free, remove them
            from the fragment list and free the whole block.  */
            next = prev;
            for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> type); ++i)
            {
                next = next->next;
            }
            prev->prev->next = next;
            if (next != NULL)
            {
                next->prev = prev->prev;
            }
            _heapinfo[block].busy.type = 0;
            _heapinfo[block].busy.info.size = 1;

            /* Keep the statistics accurate.  */
            ++_chunks_used;
            _bytes_used += BLOCKSIZE;
            _chunks_free -= BLOCKSIZE >> type;
            _bytes_free -= BLOCKSIZE;

            free (ADDRESS (block));
        }
        else if (_heapinfo[block].busy.info.frag.nfree != 0)
        {
            /* If some fragments of this block are free, link this
            fragment into the fragment list after the first free
            fragment of this block. */
            next = (struct list *) ptr;
            if(next != NULL)
            {
                next->next = prev->next;
                next->prev = prev;
                prev->next = next;
                if (next->next != NULL)
                {
                    next->next->prev = next;
                }
            }
            ++_heapinfo[block].busy.info.frag.nfree;
        }
        else
        {
            /* No fragments of this block are free, so link this
            fragment into the fragment list and announce that
            it is the first free fragment of this block. */
            prev = (struct list *) ptr;
            _heapinfo[block].busy.info.frag.nfree = 1;
            _heapinfo[block].busy.info.frag.first = (unsigned long int)((unsigned long int)((char *)ptr-(char *)NULL)
                                                     % BLOCKSIZE >> type);
            if(prev != NULL)
            {
                prev->next = _fraghead[type].next;
                prev->prev = &_fraghead[type];
                prev->prev->next = prev;
                if (prev->next != NULL)
                {
                    prev->next->prev = prev;
                }
            }
        }
        break;
    }
}


/*
void ddd_test()
{
    int state = 0;

    state = read_cpu_state();

    if (state & 0x6)
        PRINTF("called in intr or exp!!!\n");
}
*/

/* Return memory to the heap.  */
void free (__ptr_t ptr)
{
    register struct alignlist *l = NULL;

    //ddd_test();

    if((ptr<(void *)(_heapbase+heapsize*sizeof(malloc_info))) || (NULL == ptr) || ( ptr>=RAMSIZE) )
    {
        return;
    }
    
    osal_task_dispatch_off();
    for (l = _aligned_blocks; l != NULL; l = l->next)
    {
        if (l->aligned == ptr)
        {
            l->aligned = NULL;    /* Mark the slot in the list as free.  */
            ptr = l->exact;
            break;
        }
    }

    if (__free_hook != NULL)
    {
        (*__free_hook) (ptr);
    }
    else
    {
        _free_internal (ptr);
    }

    osal_task_dispatch_on();

    return;
}

/* Resize the given region to the new size, returning a pointer
to the (possibly moved) region.  This is optimized for speed;
some benchmarks seem to indicate that greater compactness is
achieved by unconditionally allocating and copying to a
new region.  This module has incestuous knowledge of the
internals of both free and malloc. */
__ptr_t realloc (__ptr_t ptr, __malloc_size_t size)
{
    __ptr_t result = NULL;
    int type = 0;
    __malloc_size_t block = 0;
    __malloc_size_t blocks = 0;
    __malloc_size_t oldlimit = 0;
    __ptr_t previous = NULL;

    osal_task_dispatch_off();
    if (0 == size)
    {
        free (ptr);
        osal_task_dispatch_on();
        return malloc (0);
    }
    else if (NULL == ptr)
    {
        osal_task_dispatch_on();
        return malloc (size);
    }

    if (__realloc_hook != NULL)
    {
        osal_task_dispatch_on();
        return (*__realloc_hook) (ptr, size);
    }


    block = BLOCK (ptr);

    if(NULL == _heapinfo)
    {
        ASSERT(0);
        osal_task_dispatch_on();
        return NULL;
    }
    type = _heapinfo[block].busy.type;
    switch (type)
    {
    case 0:
        /* Maybe reallocate a large block to a small fragment.  */
        if (size <= HALFBLOCKSIZE)
        {
            result = (__ptr_t)malloc (size);
            if (result != NULL)
            {
                MEMCPY (result, ptr, size);
                _free_internal (ptr);
                osal_task_dispatch_on();
                return result;
            }
        }

        /* The new size is a large allocation as well;
        see if we can hold it in place. */
        blocks = BLOCKIFY (size);
        if (blocks < (__malloc_size_t)_heapinfo[block].busy.info.size)
        {
            /* The new size is smaller; return
            excess memory to the free list. */
            _heapinfo[block + blocks].busy.type = 0;
            _heapinfo[block + blocks].busy.info.size
            = _heapinfo[block].busy.info.size - blocks;
            _heapinfo[block].busy.info.size = blocks;
            /* We have just created a new chunk by splitting a chunk in two.
            Now we will free this chunk; increment the statistics counter
            so it doesn't become wrong when _free_internal decrements it.  */
            ++_chunks_used;
            _free_internal (ADDRESS (block + blocks));
            result = ptr;
        }
        else if (blocks == (__malloc_size_t)_heapinfo[block].busy.info.size)
        {
            /* No size change necessary.  */
            result = ptr;
        }
        else
        {
            /* Won't fit, so allocate a new region that will.
            Free the old region first in case there is sufficient
            adjacent free space to grow without moving. */
            blocks = _heapinfo[block].busy.info.size;
            /* Prevent free from actually returning memory to the system.  */
            oldlimit = _heaplimit;
            _heaplimit = 0;
            _free_internal (ptr);
            _heaplimit = oldlimit;
            result = (__ptr_t)malloc (size);
            if (NULL == result)
            {
                /* Now we're really in trouble.  We have to unfree
                the thing we just freed.  Unfortunately it might
                have been coalesced with its neighbors.  */
                if (_heapindex == block)
                {
                    (void) malloc (blocks * BLOCKSIZE);
                }
                else
                {
                    previous = (__ptr_t)malloc ((block - _heapindex) * BLOCKSIZE);
                    (void) malloc (blocks * BLOCKSIZE);
                    _free_internal (previous);
                }
                osal_task_dispatch_on();
                return NULL;
            }
            if (ptr != result)
            {
                MEMMOVE (result, ptr, blocks * BLOCKSIZE);
            }
        }
        break;

    default:
        /* Old size is a fragment; type is logarithm
        to base two of the fragment size.  */
        if ((size > (__malloc_size_t) (1 << (type - 1))) &&
                (size <= (__malloc_size_t) (1 << type)))
        {
            /* The new size is the same kind of fragment.  */
            result = ptr;
        }
        else
        {
            /* The new size is different; allocate a new space,
            and copy the lesser of the new size and the old. */
            result = (__ptr_t)malloc (size);
            if (NULL == result)
            {
                osal_task_dispatch_on();
                return NULL;
            }

            MEMCPY (result, ptr, MIN (size, (__malloc_size_t) 1 << type));
            free (ptr);
        }
        break;
    }

    osal_task_dispatch_on();
    return result;
}


__ptr_t calloc(__malloc_size_t nelem, __malloc_size_t elsize)
{
    unsigned char *ptr = malloc(nelem*elsize);

    if(ptr)
    {
        libc_memset(ptr, 0, nelem*elsize);
    }
    return ptr;
}

void malloc_trace(__attribute__((unused))int enable)
{
#ifdef MT_BUFF_LEN
    g_mt_enable = enable;
#endif
}
int malloc_trace_reset(void)
{
	int ret = -1;
#ifdef MT_BUFF_LEN
    if (g_mt_enable == 0)
    {
		// only can reset when you disable trace
		MEMSET(&g_malloc_trace, 0, sizeof(g_malloc_trace));
		ret = 0;
    }
#endif
	return ret;
}

void malloc_trace_print()
{
#if ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))
#ifdef MT_BUFF_LEN
    int i = 0;
    UINT32 total_len =0; 
    libc_printf("----------------------------------\nMalloc Trace:\n");
    libc_printf("Memory          Function\n");
    for(i=0;i<MT_BUFF_LEN;i++)
    {
        if(g_malloc_trace[i].mem!=0)
        {
            libc_printf("0x%8x 0x%8x 0x%x\n", g_malloc_trace[i].mem, g_malloc_trace[i].len, g_malloc_trace[i].fun);
            total_len +=g_malloc_trace[i].len;
        }
    }
    libc_printf("----------------------------------\n");
    libc_printf("total used len 0x%x\n", total_len);
#endif
#endif
}

int heap_get_free_size(void)
{
	unsigned long siz = 0;
	unsigned long heap_top = (unsigned long)RAMSIZE;
	unsigned long heap_cur = (unsigned long)heap_ptr;
	if (heap_cur >= heap_top)
	{
		siz = 0;
	}
	else
	{
		siz = heap_top - heap_cur;
	}

	return (int)siz;
}

static unsigned long s_free_siz = 0;//new or change add by yuj
static unsigned long s_free_times = 0;//new or change add by yuj
int heap_printf_free_size(UINT8 b_printf_now)//new or change add by yuj
{
	unsigned long siz = 0;
	unsigned long heap_top = (unsigned long)RAMSIZE;
	unsigned long heap_cur = (unsigned long)heap_ptr;
	unsigned long tick = osal_get_tick();
	unsigned long day,hour,minute,second;
	if (heap_cur >= heap_top)
	{
		siz = 0;
	}
	else
	{
		siz = heap_top - heap_cur;
	}
	if(s_free_siz != siz || (s_free_times %10000) == 0 || b_printf_now == 1)
	{
		second 	= tick/1000%60;
		minute 	= tick/60000%60;
		hour	 	= tick/3600000%24;
		day		= tick/86400000;
    		libc_printf("[HEAP]-----FREE[%8x][%8x][%8dK][%08X][%d--%02d:%02d:%02d]\n",heap_top,heap_cur,siz/1024,siz,day,hour,minute,second);
		s_free_siz = siz;
	}
	s_free_times++;
	return (int)siz;
}

