/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     fastcrc.c
*
*    Description:This file contains all functions definition of
*                Fast CRC code/decode functions.
*    NOTE: The type of int must be 32bits weight.
*
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    2000.7.13       Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <api/libc/fast_crc.h>

/************************************* Const define *********************************/
/* Globle CRC select switch */
// #define MG_CRC_32_ARITHMETIC
#define MG_CRC_32_ARITHMETIC_CCITT
/* #define MG_CRC_24_ARITHMETIC_CCITT
 * #define MG_CRC_24_ARITHMETIC */

#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24_BIT
#endif

/* 0xbba1b5 = 1'1011'1011'1010'0001'1011'0101b
 <= x24+x23+x21+x20+x19+x17+x16+x15+x13+x8+x7+x5+x4+x2+1 */
#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_CCITT        0x00ddd0da
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24            0x00ad85dd
#endif

/* 0x04c11db7 = 1'0000'0100'1100'0001'0001'1101'1011'0111b
 <= x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 */
#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_CCITT        0x04c11db7
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32            0xedb88320
#endif

/************************************ Var define ************************************/
#if defined(FIXED_CRC_TABLE)
    unsigned int mg_crc_table[256];
#elif (SYS_SDRAM_SIZE == 2 && (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2) )
    #if(SYS_EPG_MODULE == SYS_FUNC_ON)
        unsigned int mg_crc_table[256];
    #else
        unsigned int *mg_crc_table = (unsigned int *)(__MM_CRC_BUF);
    #endif
#elif (SYS_SDRAM_SIZE == 2 && SUBTITLE_ON != 1)
    #ifdef  _DEBUG_VERSION_
      unsigned int *mg_crc_table = (unsigned int *)(__MM_CRC_BUF);
    #else
       unsigned int mg_crc_table[256];
    #endif
#else
    static unsigned int mg_crc_table[256];
#endif

static unsigned char mg_crc_table_initialized = 0;

/********************************** Function define *********************************/

/*From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
unsigned int mg_compute_crc(register unsigned int crc,
                        register unsigned char *bufptr,
                        register int len)
{
    register int i = 0;

    if(NULL == bufptr)
    {
        return 0; //suppose it's impossble that crc=0;
    }

#ifdef MG_CRC_24_ARITHMETIC_CCITT
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)(*bufptr) << 16);
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x00800000)    /*Highest bit procedure*/
            {
                crc = (crc << 1) ^ MG_CRC_24_CCITT;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_24_ARITHMETIC
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)*bufptr;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 1)                /*Lowest bit procedure*/
            {
                crc = (crc >> 1) ^ MG_CRC_24;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)(*bufptr) << 24;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80000000)    /*Highest bit procedure*/
            {
                crc = (crc << 1) ^ MG_CRC_32_CCITT;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)*bufptr;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 1)                /*Lowest bit procedure*/
                crc = (crc >> 1) ^ MG_CRC_32;
            else
                crc >>= 1;
        }
    }
    return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif
}

/*Setup fast CRC compute table*/
void mg_setup_crc_table(void)
{
    register int count = 0;
    unsigned char zero=0;

    if (mg_crc_table_initialized == 1)
        return ;

    for(count = 0; count <= 255; count++) /*Comput input data's CRC, from 0 to 255*/
    {
#ifdef MG_CRC_24_ARITHMETIC_CCITT
        mg_crc_table[count] = mg_compute_crc(count << 16,&zero,1);
#endif

#ifdef MG_CRC_24_ARITHMETIC
        mg_crc_table[count] = mg_compute_crc(count,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
        mg_crc_table[count] = mg_compute_crc(count << 24,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC
        mg_crc_table[count] = mg_compute_crc(count,&zero,1);
#endif
    }

    mg_crc_table_initialized = 1;

    return ;
}

/*Fast CRC compute*/
unsigned int mg_table_driven_crc(register unsigned int crc,
                register unsigned char *bufptr,
                register int len)
{
    register int i = 0;

    if(NULL == bufptr)
    {
        return 0;
    }
    if (mg_crc_table_initialized == 0)
    {
        mg_setup_crc_table();
    }

    for(i = 0; i < len; i++)
    {
#ifdef MG_CRC_24_ARITHMETIC_CCITT
        crc=(mg_crc_table[((crc >> 16) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
        crc=(mg_crc_table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
        crc=(mg_crc_table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0xffffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
        crc=(mg_crc_table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0xffffffff;
#endif
    }

    return(crc);
}

void mg_fcs_coder(unsigned char *pindata,int len)
{
    unsigned int uifcs = 0;

    if((NULL == pindata) || (0 > len))
    {
        return;
    }
#ifdef MG_CRC_24_ARITHMETIC_CCITT
    uifcs = ~mg_table_driven_crc(0x00ffffff,pindata,len);
    pindata[len + 2] = (unsigned char)uifcs&0xff;
    pindata[len + 1] = (unsigned char)(uifcs >>  8) & 0xff;
    pindata[len]     = (unsigned char)(uifcs >> 16) & 0xff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
    uifcs = ~mg_table_driven_crc(0x00ffffff,pindata,len);
    pindata[len]     = (unsigned char)uifcs&0xff;
    pindata[len + 1] = (unsigned char)(uifcs >>  8) & 0xff;
    pindata[len + 2] = (unsigned char)(uifcs >> 16) & 0xff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
    uifcs = mg_table_driven_crc(0xffffffff,pindata,len);
    pindata[len + 3] = (unsigned char)uifcs & 0xff;
    pindata[len + 2] = (unsigned char)(uifcs >>  8) & 0xff;
    pindata[len + 1] = (unsigned char)(uifcs >> 16) & 0xff;
    pindata[len]     = (unsigned char)(uifcs >> 24) & 0xff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
    uifcs = ~mg_table_driven_crc(0xffffffff,pindata,len);
    pindata[len]     = (unsigned char)uifcs & 0xff;
    pindata[len + 1] = (unsigned char)(uifcs >>  8) & 0xff;
    pindata[len + 2] = (unsigned char)(uifcs >> 16) & 0xff;
    pindata[len + 3] = (unsigned char)(uifcs >> 24) & 0xff;
#endif
}

int mg_fcs_decoder(unsigned char *pindata,int len)
{
    unsigned int uifcs = 0;

    if((NULL == pindata) || (0 > len))
    {
        return -1;
    }
#ifdef MG_CRC_24_ARITHMETIC_CCITT
    pindata[len - 1] = ~pindata[len - 1];
    pindata[len - 2] = ~pindata[len - 2];
    pindata[len - 3] = ~pindata[len - 3];
    if ((uifcs = mg_table_driven_crc(0x00ffffff,pindata,len)) != 0)/*Compute FCS*/
    {
        return(uifcs);                            /* CRC check error */
    }
    pindata[len-1]='\0';
    pindata[len-2]='\0';
    pindata[len-3]='\0';
#endif

#ifdef MG_CRC_24_ARITHMETIC
    pindata[len - 1] = ~pindata[len - 1];
    pindata[len - 2] = ~pindata[len - 2];
    pindata[len - 3] = ~pindata[len - 3];
    if ((uifcs = mg_table_driven_crc(0x00ffffff,pindata,len)) != 0)/*Compute FCS*/
    {
        return(uifcs);                            /* CRC check error */
    }
    pindata[len-1]='\0';
    pindata[len-2]='\0';
    pindata[len-3]='\0';
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
/*    pindata[len - 1] = ~pindata[len - 1];
    pindata[len - 2] = ~pindata[len - 2];
    pindata[len - 3] = ~pindata[len - 3];
    pindata[len - 4] = ~pindata[len - 4];
*/  
    uifcs = mg_table_driven_crc(0xffffffff,pindata,len);
    if ( uifcs != 0)/*Compute FCS*/
    {
        return(uifcs);                            /* CRC check error */
    }
/*    pindata[len - 1] = '\0';
    pindata[len - 2] = '\0';
    pindata[len - 3] = '\0';
    pindata[len - 4] = '\0';
*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
    pindata[len - 1] = ~pindata[len - 1];
    pindata[len - 2] = ~pindata[len - 2];
    pindata[len - 3] = ~pindata[len - 3];
    pindata[len - 4] = ~pindata[len - 4];
    if ((uifcs = mg_table_driven_crc(0xffffffff,pindata,len)) != 0)/*Compute FCS*/
    {
        return(uifcs);                            /* CRC check error */
    }
    pindata[len - 1] = '\0';
    pindata[len - 2] = '\0';
    pindata[len - 3] = '\0';
    pindata[len - 4] = '\0';
#endif

    return(0);                                /* CRC check OK */
}

/***************************************** END ***************************************/
