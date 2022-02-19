/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: hdcp_des.c
*
*    Description: 3DES Encryption/Decryption function
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <stdio.h>
#include <api/libc/string.h>
#ifndef HDCP_FROM_CE
#include "hdcp_des.h"

/*****************************************************************************
 For  key Schedule Calculation
*****************************************************************************/
// permuted choice table PC-1(key) (Pg.20)
static const char pc1_table[56] = { 57, 49, 41, 33, 25, 17,  9,
                                     1, 58, 50, 42, 34, 26, 18,
                                    10,  2, 59, 51, 43, 35, 27,
                                    19, 11,  3, 60, 52, 44, 36,
                                    63, 55, 47, 39, 31, 23, 15,
                                     7, 62, 54, 46, 38, 30, 22,
                                    14,  6, 61, 53, 45, 37, 29,
                                    21, 13,  5, 28, 20, 12,  4
                   };
// number left rotations of pc1
static const char left_shift_table[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

// permuted choice key PC-2(table)  (Pg.21)
static const char pc2_table[48] = {  14, 17, 11, 24,  1,  5,
                                      3, 28, 15,  6, 21, 10,
                                     23, 19, 12,  4, 26,  8,
                                     16,  7, 27, 20, 13,  2,
                                     41, 52, 31, 37, 47, 55,
                                     30, 40, 51, 45, 33, 48,
                                     44, 49, 39, 56, 34, 53,
                                     46, 42, 50, 36, 29, 32
                   };

/*****************************************************************************
 For  DES Encryption Calculation
*****************************************************************************/
// initial permutation IP (Pg.7)
static const char ic_p_table[64] = { 58, 50, 42, 34, 26, 18, 10, 2,
                                   60, 52, 44, 36, 28, 20, 12, 4,
                                   62, 54, 46, 38, 30, 22, 14, 6,
                                   64, 56, 48, 40, 32, 24, 16, 8,
                                   57, 49, 41, 33, 25, 17,  9, 1,
                                   59, 51, 43, 35, 27, 19, 11, 3,
                                   61, 53, 45, 37, 29, 21, 13, 5,
                                   63, 55, 47, 39, 31, 23, 15, 7
                  };
// Bit Selection table (Pg.10) for function f
static const char e_table[48] = {  32,  1,  2,  3,  4,  5,
                                    4,  5,  6,  7,  8,  9,
                                    8,  9, 10, 11, 12, 13,
                                   12, 13, 14, 15, 16, 17,
                                   16, 17, 18, 19, 20, 21,
                                   20, 21, 22, 23, 24, 25,
                                   24, 25, 26, 27, 28, 29,
                                   28, 29, 30, 31, 32,  1
                 };

// 32-bit permutation function P used on the output of the S-boxes
static const char c_p_table[32] = { 16,  7, 20, 21,
                                  29, 12, 28, 17,
                                   1, 15, 23, 26,
                                   5, 18, 31, 10,
                                   2,  8, 24, 14,
                                  32, 27,  3,  9,
                                  19, 13, 30,  6,
                                  22, 11,  4,  25
                  };
// The (in)famous S-boxes
static const char s_boxes[8][4][16] = {
        {   // S1
            {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
            { 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
            { 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
            {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
        },
        {   // S2
            {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
            { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
            { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
            {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
        },
        { // S3
            {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
            {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
            {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
            { 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
        },
        { // S4
            { 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
            {13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
            {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
            { 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
        },
        { // S5
            { 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
            {14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
            { 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
            {11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
        },
        { // S6
            {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
            {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
            { 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
            { 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
        },
        { // S7
            { 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
            {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
            { 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
            { 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
        },
        { // S8
            {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
            { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
            { 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
            { 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
        }
};
// final permutation Inverse of IP (Pg.7)
static const char ipr_table[64] = { 40, 8, 48, 16, 56, 24, 64, 32,
                                    39, 7, 47, 15, 55, 23, 63, 31,
                                    38, 6, 46, 14, 54, 22, 62, 30,
                                    37, 5, 45, 13, 53, 21, 61, 29,
                                    36, 4, 44, 12, 52, 20, 60, 28,
                                    35, 3, 43, 11, 51, 19, 59, 27,
                                    34, 2, 42, 10, 50, 18, 58, 26,
                                    33, 1, 41,  9, 49, 17, 57, 25
                   };

/*********************************************************************************************
* void permutation(unsigned char *out, unsigned char *in,  const char *table, int output_len)
* permutation Bit position change by lookup table.
* E.g. table[index = 0] = 58
* Output arrary [0] value is input arry[58-1].
*********************************************************************************************/
static void permutation(unsigned char *out, unsigned char *in,  const char *table, int output_len)
{
    int i= 0;
    unsigned char temp = 0;

    if((NULL == out) || (NULL == in) || (NULL == table))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    for(i=0; i<output_len; i++)
    {

        // lookup Input ( bit: table[i] -1 ) value is "0" or "1"
        temp = in[ (table[i]-1)/8 ] & (0x80 >> ( (table[i]-1)%8) );

        // Set Output (bit i) value to temp.
        if( temp != 0x00)
            {
            out[i/8] |= (0x80 >> ( i%8)) ;
            }
        else
            {
            out[i/8] &= ~(0x80 >> ( i%8));
            }

    }
}

/*********************************************************************************************
hdcp_des_key_schedule(unsigned char *main_key, unsigned char *sub_key)

method:
1. 64 bits key -> Permuted Choice 1 -->56 bits key

2. separate 56 bits key to 2 part which is 28 bits (C0 & D0)
    (LSi)
    Ci = LSi (Ci-1)
    Di = LSi (Di-1)

3. Sub_Keyi = PC-2 (Ci || Di)    i = 1, 2, 3,
*********************************************************************************************/
static void hdcp_des_key_schedule(unsigned char *main_key, unsigned char *sub_key)
{
    unsigned char c_d[7] = {0};
    unsigned char temp = 0; // 56bits key
    int i= 0;

    if((NULL == main_key) || (NULL == sub_key))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    // Permuted choice 1
    permutation(c_d, main_key,  pc1_table, 56);

    for(i=0; i<16; i++)
    {

        // Left Shift rotate
        // Ci = LSi (Ci-1)
        temp = (c_d[0] >> (8-left_shift_table[i]) ) << 4;
        c_d[0] = (c_d[0] << left_shift_table[i]) | (c_d[1] >> (8-left_shift_table[i]));
        c_d[1] = (c_d[1] << left_shift_table[i]) | (c_d[2] >> (8-left_shift_table[i]));
        c_d[2] = (c_d[2] << left_shift_table[i]) | (c_d[3] >> (8-left_shift_table[i]));
        c_d[3] = ((c_d[3]&0xF0) << left_shift_table[i]) | temp | (c_d[3]&0x0F);

        // Di = LSi (Di-1)
        temp = (c_d[3]&0x0F) >> (4-left_shift_table[i]);
        c_d[3] = (c_d[3]&0xF0) | ((c_d[3] << left_shift_table[i])& 0x0F) | (c_d[4] >> (8-left_shift_table[i]));
        c_d[4] = (c_d[4] << left_shift_table[i]) | (c_d[5] >> (8-left_shift_table[i]));
        c_d[5] = (c_d[5] << left_shift_table[i]) | (c_d[6] >> (8-left_shift_table[i]));
        c_d[6] = (c_d[6] << left_shift_table[i]) | temp;

        // Permuted choice 2
        permutation(sub_key+i*6, c_d,  pc2_table, 48);
    }

}

static void xor(unsigned char *ina, unsigned char *inb, int len)
{
    int i= 0;

    if((NULL == ina) || (NULL == inb))
    {
        libc_printf("%s NULL input!\n",__FUNCTION__);
        return;
    }

    for(i=0; i<len/8; i++)
    {
        ina[i] ^= inb[i];
    }
}

static void s_func(unsigned char sbox_out[4], unsigned char sbox_in[6])
{
    unsigned char temp = 0;

    // S0 out
    temp =  (sbox_in[0] >> 2) & 0x3F;
    sbox_out[0] = s_boxes[0][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1] << 4;

    // S1 out
    temp = ((sbox_in[0]&0x03) << 4) | ((sbox_in[1] >>4) & 0x2F);
    sbox_out[0] |= s_boxes[1][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1];

    // S2 out
    temp = ((sbox_in[1]&0x0F) << 2) | (((sbox_in[2]&0xC0) >>6) & 0x2F);
    sbox_out[1] = s_boxes[2][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1] << 4;

    // S3 out
    temp = sbox_in[2] & 0x3F;
    sbox_out[1] |= s_boxes[3][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1];

    // S4 out
    temp = (sbox_in[3] >> 2) & 0x3F;
    sbox_out[2] = (s_boxes[4][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1] << 4);

    // S5 out
    temp = ((sbox_in[3]&0x03) << 4) | ((sbox_in[4] >>4) & 0x2F);
    sbox_out[2] |= s_boxes[5][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1];

    // S6 out
    temp = ((sbox_in[4]&0x0F) << 2) | (((sbox_in[5]&0xC0) >>6) & 0x2F);
    sbox_out[3] = (s_boxes[6][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1] << 4);

    // S7 out
    temp = sbox_in[5] & 0x3F;
    sbox_out[3] |= s_boxes[7][(temp & 0x20) >> 4 | (temp & 0x01)][(temp & 0x1E) >> 1];
}

static void f_func(unsigned char *r, unsigned char *sub_key)
{
    unsigned char mr[6] = {0};
    unsigned char sbox_out[4] = {0};

    if((NULL==r) ||(NULL==sub_key))
    {
        return ;
    }
    // E table permutation 32bits -> 48 bits
    permutation(mr, r,  e_table, 48);

    // XOR with sub_key
    xor(mr, sub_key, 48);

    // S box transformation
    s_func(sbox_out, mr);

    // P table permutation 32bits -> 32 bits
    permutation(r, sbox_out, c_p_table, 32);

}

void des_encryption(unsigned char *plaintext, unsigned char *chiphertext, unsigned char *key)
{
    int i= 0;
    unsigned char sub_key[16][6] = {{0}};
    unsigned char lr[8] = {0};
    unsigned char temp[4] = {0};
    unsigned char *li = &lr[0];
    unsigned char *ri = &lr[4];

    if((NULL==plaintext) || (NULL==chiphertext) || (NULL==key))
    {
        return ;
    }

    // key Schedule Calculation
    hdcp_des_key_schedule(key, (unsigned char*) sub_key);

    // Initial permutation
    permutation(lr, plaintext,  ic_p_table, 64);

    li = &lr[0];
    ri = &lr[4];

    for(i=0; i<16; i++)
    {
        MEMCPY(temp, li, 4);            // Backup Ln-1

        MEMCPY(li, ri, 4);              // Ln = Rn-1

        f_func(ri, &sub_key[i][0]);     // Rn' = f(Rn-1,Kn)

        xor(ri, temp, 32);             // Rn = Ln-1 xor Rn'

    }

    // Reverse Conneting  L <-> r
    MEMCPY(temp, ri, 4);
    MEMCPY(ri, li, 4);
    MEMCPY(li, temp, 4);

    // Inverse Initial Permutation
    permutation(chiphertext, lr,  ipr_table, 64);

}

void des_decryption(unsigned char *chiphertext, unsigned char *plaintext, unsigned char *key)
{
    unsigned char sub_key[16][6] = {{0}};
    unsigned char lr[8] = {0};
    unsigned char temp[6] = {0};
    unsigned char *li = &lr[0];
    unsigned char *ri = &lr[4];
    int i= 0;

    if((NULL==chiphertext)||(NULL==plaintext)||(NULL==key))
    {
        return;
    }
    // key Schedule Calculation
    hdcp_des_key_schedule(key, (unsigned char*) sub_key);

    // Initial permutation
    permutation(lr, chiphertext,  ic_p_table, 64);

    li = &lr[4];
    ri = &lr[0];

    // Reverse Conneting  L <-> r
    MEMCPY(temp, ri, 4);
    MEMCPY(ri, li, 4);  
    MEMCPY(li, temp, 4);

    for(i=15; i>=0; i--)
    {
        MEMCPY(temp, ri, 4);    // Put Rn-1 to temp

        // Rn = F(Rn-1, Kn) xor Ln-1
        f_func(ri, &sub_key[i][0]);
        xor(ri, li, 32);

        // Ln = Rn-1
        MEMCPY(li, temp, 4);
    }

    // Inverse Initial Permutation
    permutation(plaintext, lr,  ipr_table, 64);

}

void triple_des_encryption(unsigned char *plaintext, unsigned char *chiphertext,
                            unsigned char *key1,unsigned char *key2, unsigned char *key3)
{
    if((NULL==plaintext)||(NULL==chiphertext)||(NULL==key1)||(NULL==key2)||(NULL==key3))
    {
        return;
    }
    //DEA1-Fkey1
    des_encryption(plaintext, chiphertext, key1);

    //DEA2-Ikey2
    des_decryption(chiphertext, plaintext, key2);

    //DEA3-Fkey3
    des_encryption(plaintext, chiphertext, key3);

}

void triple_des_decryption(unsigned char *chiphertext, unsigned char *plaintext,
                            unsigned char *key1, unsigned char *key2, unsigned char *key3)
{
    if((NULL==plaintext)||(NULL==chiphertext)||(NULL==key1)||(NULL==key2)||(NULL==key3))
    {
        return;
    }
    //DEA3-Ikey3
    des_decryption(chiphertext, plaintext, key3);

    //DEA2-Fkey2
    des_encryption(plaintext, chiphertext, key2);

    //DEA1-Ikey1
    des_decryption(chiphertext, plaintext, key1);

}
#endif

