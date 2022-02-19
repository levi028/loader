
#define UNZIP_BASE_SIZE 1846
#define UNZIP_LIT_SIZE 768

#include <api/libzip/gnuzip.h>
//#include "../lzma/LzmaDecode.h"

#ifdef _LZMA_OUT_READ
static unsigned char *m_dict_buffer;
static unsigned int m_dict_size;

void set_dictionary_buffer(unsigned char *buffer, unsigned int size)
{
    m_dict_buffer = buffer;
    m_dict_size = size;
}

unsigned int get_dictionary_size(unsigned char *in)
{
    unsigned int size;
    size = (in[4] << 24) | (in[3] << 16) | (in[2] << 8) | in[1];
    return size;
}

unsigned int get_unzip_size(unsigned char *in)
{
    unsigned int size;
    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

unsigned int get_internal_size(unsigned char *in)
{
    unsigned int lzma_internal_size;
    int lc, lp, pb;
    unsigned char prop0 = in[0];
    if (prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return 1;
    }

    for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5))
        ;
    for (lp = 0; prop0 >= 9; lp++, prop0 -= 9)
        ;
    lc = prop0;

    // 2) Calculate required amount for LZMA lzma_internal_size:
    lzma_internal_size = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) *
          sizeof(unsigned short);

    return (lzma_internal_size+100);
}

int un7zip(unsigned char *in, unsigned int in_size,
           unsigned char *buf, unsigned int buf_size,
           unsigned char *out, unsigned int out_size, UN7ZIP_OUT_CALLBACK out_callback)
{
    unsigned int lzma_internal_size = 0;
    unsigned int dict_size = 0;
    unsigned int now_pos = 0;
    unsigned int unzip_size = 0;
    unsigned int out_size_processed;
    int lc = 0;
    int lp = 0;
    int pb = 0;
    int ret = 0;

    // 1) Read first byte of properties for LZMA compressed stream,
    // check that it has correct value and calculate three
    // LZMA property variables:
    unsigned char prop0 = in[0];
    if(prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return LZMA_RESULT_DATA_ERROR;
    }

    for(pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
    for(lp = 0; prop0 >= 9; lp++, prop0 -= 9);
    lc = prop0;

    // 2) Calculate required amount for LZMA lzma_internal_size:
    lzma_internal_size = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) * sizeof(unsigned short) + 100;
    if(lzma_internal_size > buf_size)
    {
        return LZMA_RESULT_NOT_ENOUGH_MEM;
    }

    // 3) Get decompressed data dictionary size:
    dict_size = get_dictionary_size(in);
    if(dict_size > m_dict_size)
    {
        return LZMA_RESULT_NOT_ENOUGH_MEM;
    }

    unzip_size = get_unzip_size(in);
    if(!out_callback && (unzip_size > out_size))
    {
        return LZMA_RESULT_NOT_ENOUGH_MEM;
    }

    // 4) Init decoder
    ret = lzma_decoder_init(buf, lzma_internal_size, lc, lp, pb,
                          m_dict_buffer, dict_size, in+13, in_size-13);
    if(ret != LZMA_RESULT_OK)
    {
        return ret;
    }

    // 4) Decompress data:
    for(now_pos=0; now_pos<unzip_size; now_pos+=out_size_processed)
    {
        uint32 block_size = unzip_size - now_pos;
        if(block_size > out_size)
        {
            block_size = out_size;
        }
        ret = LzmaDecode(buf, out, block_size, &out_size_processed);
        if(ret != LZMA_RESULT_OK || !out_size_processed)
        {
            break;
        }
        if(out_callback)
        {
            out_size = out_size_processed;
            if(out_callback(&out, &out_size))
                break;
        }
    }
    if(!ret)
    {
        for(lc=0; lc<(int)sizeof(unsigned int); lc++)
        {
            buf[lc] = ((unsigned char *)&now_pos)[lc];
        }
    }
    return ret;
}
#else
int un7zip(unsigned char *in, unsigned char *out, \
      unsigned char *buf)
{
    unsigned int lzma_internal_size = 0;
    unsigned int out_size = 0;
    unsigned int out_size_processed = 0;
    int lc = 0;
    int lp = 0;
    int pb = 0;
    int ret = 0;

    if((!in) || (!out) || (!buf))
    {
        return 1;
    }
    // 1) Read first byte of properties for LZMA compressed stream,
    // check that it has correct value and calculate three
    // LZMA property variables:
    unsigned char prop0 = in[0];

    if (prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return 1;
    }

    for (pb = 0; prop0 >= (9 * 5); pb++)
    {
        prop0 -= (9 * 5);
    }
    for (lp = 0; prop0 >= 9; lp++)
    {
        prop0 -= 9;
    }
    lc = prop0;

    // 2) Calculate required amount for LZMA lzma_internal_size:
    lzma_internal_size = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) *
          sizeof(unsigned short);

    // 3) Get decompressed data size:
    out_size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];

    // 4) Decompress data:
    ret = LzmaDecode(buf, lzma_internal_size,
          lc, lp, pb,
          in + 13, 0x2000000,
          out, out_size, &out_size_processed);

    *(unsigned int *)buf = out_size_processed;
    return ret;
}
#endif
