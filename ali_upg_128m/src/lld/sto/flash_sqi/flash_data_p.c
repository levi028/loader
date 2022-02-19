/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: flash_data_p.c
*
*    Description: Provide paral flash ID and description.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  1.  2005.5.28   Liu Lan     0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#include "flash_data.h"

const unsigned char pflash_deviceid[] =
{
/*0*/    0x11, 64,   /* 29LV128MB */
/*1*/    0x20, 64,   /* 29DW128F */
/*2*/    0xAB, 16,   /* 29F400B */
/*3*/    0xBA, 16,   /* 29LV400B */
/*4*/    0x13,  8,   /* 49F/LV040 */
/*5*/    0xB5,  8,   /* A29L400B */
/*6*/    0x46, 40,   /* MX29F004B */
/*7*/    0x23, 16,   /* 29F400T */
/*8*/    0xB9, 16,   /* 29LV400T */
/*9*/    0x34,  8,   /* A29L400T */
/*10*/    0x45, 40,   /* MX29F004T */
/*11*/    0x58, 16,   /* 29F800B */
/*12*/    0x5B, 16,   /* 29LV800B */
/*13*/    0x9B,  8,   /* A29L800B */
/*14*/    0xD6, 16,   /* 29F800T */
/*15*/    0xDA, 16,   /* 29LV800T */
/*16*/    0x1A,  8,   /* A29L800T */
/*17*/    0x49, 16,   /* 29LV160B */
/*18*/    0xC4, 16,   /* 29LV160T */
/*19*/    0xA4, 16,   /* 29F040 */
/*20*/    0x04, 40,   /* 29F040 */
/*21*/    0x4F, 16,   /* 29LV040 */
/*22*/    0x4F,  8,   /* KH29LV040 */
/*23*/    0x92,  8,   /* A29L040 */
/*24*/    0x86,  8,   /* A29040B */
/*25*/    0xD6,  8,   /* W39L040A */
/*26*/    0xC1, 16,   /* 49BV802 */
/*27*/    0xC3, 16,   /* 49BV802T */
/*28*/    0xD7,  8,   /* 39LF/VF040 */
/*29*/    0xD8,  8,   /* 39LF/VF080, 39VF088 */
/*30*/    0x4b, 16,   /* 36VF1601C */
/*31*/    0xD9,  8,   /* 39LF/VF016 */
/*32*/    0xC8,  8,   /* 39VF1681 */
/*33*/    0xC9,  8,   /* 39VF1682 */
/*34*/    0x77, 16,   /* K8D1716UB */
/*35*/    0x75, 16,   /* K8D1716UT */
/*36*/    0xA3,  8,   /* S29AL032D 00 */
/*37*/    0xF6,  8,   /* S29AL032D 03 */
/*38*/    0xA8, 16,   /* MX29LV320B */
/*39*/    0xF9,  8,   /* S29AL032D 04 */
/*40*/    0xA7, 16,   /* MX29LV320T */
/*41*/    0x73, 72,   /* V29C31004B */
/*42*/    0x63, 72,   /* V29C31004T */
/*43*/    0x21, 72,      /* S29GL128N */
/*44*/    0x1a, 72,   /*  S29GL032 T or B*/
/*45*/  0x03, 80,    /* S29GL64N*/
/*46*/  0xc2, 8,     /*MX29GA128F*/
/*45*/  0x04, 80,    /* S29GL32N */
};

const unsigned char pflash_id[] =
{
    20, 21,        /*0*/
    1, 1, 1, 1, 1,    /*2*/
    2, 2, 2, 2,    /*7*/
    3, 3, 3,        /*11*/
    4, 4, 4,        /*14*/
    5, 6,            /*17*/
    7, 7, 7, 7, 7, 7, 7,    /*19*/
    8, 9,            /*26*/
    10, 11, 12,     /*28*/  /* 13 reserved for 39VF088 */
    12,            /*31*/
    14, 14,        /*32*/
    15, 16,        /*34*/
    17, 18, 18, 19, 19, /*36*/
    23, 23, 22, 19, 41, 22, 19/*41*/
};

const unsigned short pflash_deviceid_num = \
    (sizeof(pflash_deviceid) / (2 * sizeof(unsigned char))) ;

