#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

//#include "vdec_mpeg4_def.h"
#ifndef __forceinline
#define __forceinline
#endif

typedef struct _bit_context
{
    const unsigned char *start;
    const unsigned char *read;
    int bit_count;
    unsigned int cache0, cache1;
    unsigned int length;
} bit_context;


void  get_bits_init (bit_context *pb, const unsigned char *buffer, int length);
void get_bits_reset(bit_context *pb, unsigned long bit_offset);
void flush_bits (bit_context *pb, int n);
unsigned int get_bits (bit_context *pb, int n);
__forceinline void rewind_bits (bit_context *pb, int n);
/* advance by n bits */
__forceinline void flush_bits (bit_context *pb, int n);
/* read n bits */
__forceinline unsigned int show_bits (bit_context *pb, int n);
__forceinline unsigned int show_bits1 (bit_context *pb);

// returns absolute bis position inside the stream
__forceinline unsigned int get_bit_pos(bit_context *pb);
__forceinline void get_used_len(bit_context *pb, unsigned long *used_len, unsigned long *start_bits);
__forceinline unsigned int skip_bits1(bit_context *pb);
__forceinline int get_bits_count(bit_context *pb);
__forceinline void align_get_bits(bit_context *pb);

#endif
