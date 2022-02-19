/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: si_utility.c
*
*    Description:function in common use for SI module
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libsi/si_utility.h>

#if (SI_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SI_PRINTF               libc_printf
#else
#define SI_PRINTF(...)          do{}while(0)
#endif

enum
{
    BCD_WIDTH               = 4,
    BCD_POWER               = 10,
    BCD_MASK                = 0x0F,
};

UINT32 bcd2integer(UINT8 *ptr, UINT8 digits, INT32 offset)
{
    UINT32 ret=0;
    INT32 i = 0;
    UINT8 bcd = 0;
    UINT8 offt = 0;

    if(NULL != ptr)
    {
        for(i=0; i<digits; i++)
        {
            offt = i+offset;
            bcd = ptr[offt>>1];
            bcd = (offt&0x01)? bcd&BCD_MASK: bcd>>BCD_WIDTH;
            ret = ret*BCD_POWER+bcd;
        }
    }

    return ret;
}

#if (SI_DEBUG_LEVEL>0)
void DUMP_SECTION(UINT8 *data, INT32 len, INT32 level)
{
    UINT8 min_data_len = 8;

    if ((data != NULL)&&(len >= min_data_len))
    {
        SI_PRINTF("[%02x][%02x%02x][%d/%d]\n", data[0], data[1]&0x0f, data[2], data[6], data[7]);

        if (level > 0)
        {
            for(i = 0; i < len; i++)
            {
                SI_PRINTF("%02x ", data[i]);
                if (0x0f == (0x0f&i))
                {
                    SI_PRINTF("\n");
                }
            }
            SI_PRINTF("\n");
        }
    }
}
#endif

UINT16 SI_MERGE_UINT16(UINT8 *ptr)
{
    return (UINT16)((ptr[0]<< 8)|ptr[1]);
}

UINT32 SI_MERGE_UINT24(UINT8 *ptr)
{
    return (UINT32)((ptr[0]<<16)|(ptr[1]<< 8)|ptr[2]);
}

UINT32 SI_MERGE_UINT32(UINT8 *ptr)
{
    return (UINT32)((ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3]);
}

