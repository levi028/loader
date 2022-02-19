//#include "vdec_mpeg4_def.h"
#include "bit_stream.h"


#define SWAP(a, b) (b =((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]))

__forceinline void rewind_bits (bit_context *pb, int n)
{
    pb->bit_count -= n;
    while(pb->bit_count < 0)
    {
        pb->cache1 = pb->cache0;

        SWAP((pb->read-4), (pb->cache0));
        pb->read -= 4;
        pb->bit_count += 32;
    }
}

/* advance by n bits */
__forceinline void flush_bits (bit_context *pb, int n)
{
    pb->bit_count += n;
    if (pb->bit_count >= 32)
    {
        pb->cache0 = pb->cache1;

        SWAP(pb->read, pb->cache1);
        pb->read += 4;
        pb->bit_count -= 32;
    }
}

/* read n bits */
__forceinline unsigned int show_bits (bit_context *pb, int n)
{
    int nbit = (n + pb->bit_count) - 32;

    if (nbit > 0)
    {
        // The bits are on both ints
        return (((pb->cache0 & (0xFFFFFFFFU >> (pb->bit_count))) << nbit) |
            (pb->cache1 >> (32 - nbit)));

    }
    else
    {
        int rbit = 32 - pb->bit_count;
        return (pb->cache0 & (0xFFFFFFFFU >> (pb->bit_count))) >> (rbit-n);
    }
}

__forceinline unsigned int show_bits1 (bit_context *pb)
{
  if(pb->cache0 & (0x80000000U >> pb->bit_count))
      return 1;
  else
      return 0;
}

// returns absolute bis position inside the stream
__forceinline unsigned int get_bit_pos(bit_context *pb)
{
    return 8*(pb->read - pb->start) + pb->bit_count - 64;
}

__forceinline void get_used_len(bit_context *pb, unsigned long *used_len, unsigned long *start_bits)
{
    unsigned long used_bits = get_bit_pos(pb);
    *start_bits = used_bits&0x7;
    *used_len = (used_bits/8);
}

unsigned int get_bits (bit_context *pb, int bit_num)
{
    unsigned int l = show_bits (pb, bit_num);
    flush_bits (pb, bit_num);
    return l;
}

__forceinline unsigned int skip_bits1(bit_context *pb)
{
    unsigned int l = show_bits1(pb);
    flush_bits(pb, 1);
    return l;
}

__forceinline int get_bits_count(bit_context *pb) {
    return (pb->read - pb->start)*8 - 32 + (32 - pb->bit_count);
}

__forceinline void align_get_bits(bit_context *pb)
{
    int i = 0;
    int n = -get_bits_count(pb) & 7;
    if (n)
    {
        for(i=0; i<n; i++)
           skip_bits1(pb);
    }
}

/* initialize bit buffer, call once before first get_bits or show_bits */
void  get_bits_init (bit_context *pb, const unsigned char *buffer, int length)
{
    pb->bit_count = 0;
    pb->start = pb->read = buffer;
    pb->length = length;

    SWAP(pb->read, pb->cache0);
    pb->read += 4;
    SWAP(pb->read, pb->cache1);
    pb->read += 4;
}

/*******************************************************************************
Function name: get_bits_reset()
Description:
    1. we assume that ld has already been read some bits. Its number is bit_offset.
    2. Note: Its number is the truely read number. Not include cache0 and cache1.
Parameters:
  
Return:

Time: 2009/2/5
*******************************************************************************/
void get_bits_reset(bit_context *pb, unsigned long bit_offset)
{
    pb->read = pb->start + ((bit_offset&0xFFFFFFE0)>>3);//4-bytes align
    pb->bit_count = bit_offset%32;

    SWAP(pb->read, pb->cache0);
    pb->read += 4;
    SWAP(pb->read, pb->cache1);
    pb->read += 4;
}


