/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ttx_font_g2_greek.c
*
*    Description: The file is mainly to define the font set of greek G2 and
     the function to get char from the font set.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <basic_types.h>
#include <sys_config.h>

//#define SUPPORT_PACKET_26

#if defined(TTX_GREEK_G2_SUPPORT)

static const UINT32 ttx_g2_greek_font[][96][9] =
{
    //Greek G2 set, 0x20~0x7f maps to 0x20~0x7f
        {
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x20
             { 0x00000000, 0x00000000, 0x00000000, 0x07de3ff9, 0xf3e7cf9e,
               0x1e7879e1, 0xe7cf8ffe, 0x1f780000, 0x00000000},    //0x21
             { 0x00000000, 0x00000078, 0x01e00780, 0x1fe07fc1, 0xf3c7cf1e,
               0x3c78f1e3, 0xc7cf1ff0, 0x7f800000, 0x00000000},    //0x22
             { 0x00000000, 0x000ffc7f, 0xf9ffe7c7, 0x8f007f01, 0xfc07f01f,
               0xc03c01fc, 0x67ff9efe, 0x79f00000, 0x00000000},    // 0x23
             { 0x00000000, 0x00000000, 0x00000000, 0x03f01fe0, 0xf3c3870f,
               0xfc3ff0e0, 0x03c707fc, 0x0fe00000, 0x00000000},    //0x24
             { 0x00000000, 0x00000070, 0x01c00700, 0x1c007fc1, 0xff87fe1c,
               0x3870e1c3, 0x870e1c38, 0x70e00000, 0x00000000},    //0x25
             { 0x00000000, 0x0000000f, 0x003c0000, 0x00000f00, 0x3c00f003,
               0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x26
             { 0x00000000, 0x000ff07f, 0xf1e3c780, 0x1ff03ff1, 0xe3c7ff1f,
               0xfc3fc003, 0xc78f1ffc, 0x3fc00000, 0x00000000},    //0x27
             { 0x00000000, 0x00000000, 0x003c00f0, 0x03c00f00, 0x3c000000,
               0x000f003c, 0x00f003c0, 0x00000000, 0x00000000},    //0x28
             { 0x00000000, 0x0003e00f, 0x803e00f0, 0x03c00780, 0x1e000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x29
             { 0x00000000, 0x00071c1c, 0x71f7c79e, 0x1e787df1, 0xf7c00000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x2a
             { 0x00000000, 0x000000f0, 0x03c00f00, 0x3cf0f783, 0xfc0fe03f,
               0x00fc03fc, 0x0f783cf0, 0xf0e00000, 0x00000000},    //0x2b
             { 0x00000000, 0x0003c01f, 0x00f807c0, 0x3c00fffb, 0xffefffbf,
               0xfef001f0, 0x03e007c0, 0x0f000000, 0x00000000},    //0x2c
             { 0x00000000, 0x0003c01f, 0x80ff07fe, 0x3ffeef7b, 0x3ce0f003,
               0xc00f003c, 0x00f003c0, 0x0f003c00, 0x00000000},    //0x2d
             { 0x00000000, 0x0003c00f, 0x800f001e, 0x003efffb, 0xffefffbf,
               0xfe00f807, 0x803c03e0, 0x0f000000, 0x00000000},    //0x2e
             { 0x00000000, 0x0003c00f, 0x003c00f0, 0x03c00f00, 0x3c00f033,
               0xceef7bff, 0xe7fe0ff0, 0x1f803c00, 0x00000000},    //0x2f
             { 0x00007e03, 0xfc0c303f, 0xc07e0000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x30
             { 0x00000000, 0x0001e007, 0x801e0078, 0x1ffc7ff1, 0xffc07801,
               0xe0078000, 0x00001ffc, 0x7ff00000, 0x00000000},    //0x31
             { 0x00000007, 0xf01fc00f, 0x003c01f0, 0x0f807c01, 0xfc07f000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x32
             { 0x0000000f, 0x803f003c, 0x03f00f80, 0x0f00fc03, 0xf0000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x33
             { 0x00000000, 0x00000000, 0x00000000, 0x1c1c78f0, 0xff83fe07,
               0xf00f807f, 0x03fe1e3c, 0x70700000, 0x00000000},    //0x34
             { 0x00000000, 0x00000000, 0x00000000, 0x3ff8fff3, 0xccef33bc,
               0xcef33bcc, 0xef33bcce, 0xf3380000, 0x00000000},    //0x35
             { 0x00000000, 0x00000000, 0x00000000, 0x1ff07ff1, 0xe3c78f1e,
               0x3c78f1e3, 0xc78f1e3c, 0x78f00000, 0x00000000},    //0x36
             { 0x00000000, 0x00000000, 0x00000000, 0x1de07fe1, 0xf3c7cf1e,
               0x3c78f1e3, 0xc7cf1ff8, 0x7f81e007, 0x801e0000},    //0x37
             { 0x00000000, 0x0003c00f, 0x003c0000, 0x00007fe1, 0xff87fe1f,
               0xf8000000, 0x00f003c0, 0x0f000000, 0x00000000},    //0x38
             { 0x00000000, 0x0003e00f, 0x803e0078, 0x01e00f00, 0x3c000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x39
             { 0x00000000, 0x001f7c7d, 0xf1f7c3cf, 0x0f3c79c1, 0xe7000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x3a
             { 0x00000000, 0x00000003, 0x003c00f0, 0x07f01fc0, 0x3c00f003,
               0xc00f003c, 0x00f003f0, 0x03c00000, 0x00000000},    //0x3b
             { 0x00000000, 0x003c38f1, 0xe3c70f3c, 0x3cc0f6fb, 0xffefddbf,
               0x763b19ff, 0xe77fb006, 0xc0180000, 0x00000000},    //0x3c
             { 0x00000000, 0x003c38f0, 0xe3c60f38, 0x3cc0f7fb, 0xffefc7bf,
               0x1e38f9e7, 0xc73e3cfe, 0xc3f80000, 0x00000000},    //0x3d
             { 0x00000000, 0x000000f8, 0x73f3c3ce, 0x3f78f900, 0xfdefffbf,
               0x3e3918e4, 0x67ff9dfe, 0xc01b0060, 0x00000000},    //0x3e
             { 0x00000000, 0x00000000, 0x00000000, 0x1c1c78f0, 0xff83fe07,
               0xf00f807f, 0x03fe1e3c, 0x70700000, 0x00000000},    //0x3f
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x40
             { 0x1c003800, 0x7001c000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x41
             { 0x03c01e00, 0xf003c000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x42
             { 0x07801e01, 0xce073800, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x43
             { 0x3f98fe63, 0x1f8c7e00, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x44
             { 0x0000ffc3, 0xff0ffc00, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x45
             { 0x3870e1c0, 0xfc03f000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x46
             { 0x00000000, 0xf003c00f, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x47
             { 0x00000003, 0xcf0f3c3c, 0xf0000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x48
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00003c00, 0xf003c000},    //0x49
             { 0x1f80ff03, 0x0c0ff01f, 0x80000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x4a
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001,
               0x80060018, 0x00600180, 0x0600f803, 0xe00f8000},    //0x4b
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x0001ffc7, 0xff1ffc00},    //0x4c
             { 0x1c70e387, 0x1c1c7000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x4d
             { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
               0x60018006, 0x00180060, 0x018007c0, 0x1f007c00},    //0x4e
             { 0x1ce07380, 0x7801e000, 0x00000000, 0x00000000, 0x00000000,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x4f
             { 0x00000000, 0x000ff83f, 0xf1ffc78f, 0x003c00f0, 0x03c07f01,
               0xf8070000, 0x007001c0, 0x07000000, 0x00000000},    //0x50
             { 0x00000000, 0x003c00f0, 0x03c00f00, 0x3c00f003, 0xc00f003c,
               0x00000000, 0x00000000, 0x00000000, 0x00000000},    //0x51
             { 0x0fc07f82, 0x011002df, 0xcf7fbd8e, 0xf63bd8ef, 0x63bdfcf7,
               0xe3dbcf67, 0xbd8ef61b, 0xd82d0023, 0xff07f800},    //0x52
             { 0x00000001, 0xff0ffe60, 0x0f1f3dfc, 0xf7f3dc4f, 0x603d80f7,
               0x13dfcf7f, 0x3c7cf003, 0x7ff8ffc0, 0x00000000},    //0x53
             { 0x00000000, 0x000000ff, 0x1ffc767b, 0xd9ef67bd, 0x9ef67fd9,
               0xbb66ed9b, 0xb664d993, 0x664c0000, 0x00000000},    //0x54
             { 0x00001c00, 0x7801f007, 0xe01fe077, 0x81ce0700, 0x1c007001,
               0xc03f01fc, 0x0ff03fc0, 0x7e00f000, 0x00000000},    //0x55
             { 0x0003ff0f, 0xfc3000c0, 0x033ffcff, 0xf380ce03, 0x380ce03f,
               0xfefff838, 0x00e00380, 0x0ffc3ff0, 0x00000000},    //0x56
             { 0x0000e067, 0xc39b1c6c, 0x61f30398, 0x00c00e00, 0x7003801b,
               0x8e5f7c6d, 0xb1f7c38e, 0x00000000, 0x00000000},    //0x57
             { 0x00000000, 0x00000000, 0x00fc03f0, 0x1ce6e1fb, 0x038c0c38,
               0xe0f3c3ff, 0x87ff8fc6, 0x3f000000, 0x00000000},    //0x58
             { 0x00000000, 0x00000000, 0x00dc0370, 0x0dc02700, 0x1c007001,
               0xc007001c, 0x007001c0, 0x07000000, 0x00000000},//0x59
             { 0x00000000, 0x00000000, 0x0370ede7, 0xb3fc87e0, 0x0f003c00,
               0x60018006, 0x00180060, 0x01800000, 0x00000000},//0x5a
             { 0x00000000, 0x000000c7, 0x837f8dee, 0x2f3c7879, 0xe1e7878e,
               0x1c1ce073, 0x87ef9fbe, 0x7ef80000, 0x00000000},//0x5b
             { 0x00000000, 0x003c38f0, 0xe3c60f38, 0x3cc0f7ff, 0xfffff1ff,
               0xc73bfdef, 0xf73ffcc7, 0xc31c0ff0, 0x3fc00000},    //0x5c
             { 0x00000000, 0x000038f8, 0xe3f603f8, 0x3fc0fffc, 0xfffff1ff,
               0xc73bfccf, 0xf73fdcc7, 0xc31f0ff0, 0x3fc00000},    //0x5d
             { 0x00000000, 0x000038f8, 0xe3e60e38, 0x39c0fffc, 0xfffff1ff,
               0xc73bfccf, 0xf73fdcc7, 0xc31f0ff0, 0x3fc00000},    //0x5e
             { 0x00000000, 0x003e38f8, 0xe06601b8, 0x06c01ffc, 0x7ff1f1c7,
               0xc73bfdef, 0xf73ffcc7, 0xc31c0ff0, 0x3fc00000},    //0x5f
             { 0x00000000, 0x0000001f, 0x80ff03ce, 0x1e1c7871, 0xe007801e,
               0x007801e1, 0xc3ce0ff0, 0x1f800000, 0x00000000},    //0x60C
             { 0x00000000, 0x0000007f, 0x81ff071c, 0x1c3c70f1, 0xc3c70f1c,
               0x3c70f1c3, 0xc71c1ff0, 0x7f800000, 0x00000000},    //0x61D
             { 0x00000000, 0x0000003f, 0xe0ff83c0, 0x0f003c00, 0xf003fc0f,
               0xf03c00f0, 0x03c00f00, 0x3c000000, 0x00000000},    //0x62F
             { 0x00000000, 0x0000000f, 0xc0ff838f, 0x9c1e7079, 0xc007001c,
               0x7e71f9c1, 0xe38f8ffe, 0x0fc00000, 0x00000000},    //0x63G
             { 0x00000000, 0x00000000, 0xe003800e, 0x003800e0, 0x03800e00,
               0x3870e1c3, 0x878e0ff0, 0x1f800000, 0x00000000},    //0x64J
             { 0x00000000, 0x0000001c, 0x007001c0, 0x07001c00, 0x7001c007,
               0x001c0070, 0x01c007fc, 0x1ff00000, 0x00000000},    //0x65L
             { 0x00000000, 0x00000007, 0x80ffc3cf, 0x1e1e7879, 0xe1e7879e,
               0x1e7b79ed, 0xe3df0ffc, 0x07f80060, 0x00000000},    //0x66Q
             { 0x00000000, 0x0000007f, 0xc1ffc78f, 0x1e3c78f1, 0xe3c7fc1f,
               0xe07bc1e7, 0xc78f9e1e, 0x78380000, 0x00000000},    //0x67R
             { 0x00000000, 0x0000001f, 0x80ff078f, 0x1e1c7801, 0xfe03fe07,
               0xfc00f1c3, 0xc79f0ff8, 0x1fc00000, 0x00000000},    //0x68S
             { 0x00000000, 0x00000070, 0x79c1e707, 0x9c1e7079, 0xc1e7079c,
               0x1e7079c1, 0xe7cf8ff8, 0x1fc00000, 0x00000000},    //0x69U
             { 0x00000000, 0x000000e0, 0x7b81e70e, 0x1c383dc0, 0xf703dc07,
               0xe01f803c, 0x00f00080, 0x02000000, 0x00000000},    //0x6AV
             { 0x00000000, 0x000000e7, 0x3b9cee73, 0xb9cee73b, 0x9cee73bb,
               0xeefdfbe3, 0xef0fbc1e, 0x60300000, 0x00000000},    //0x6BW
             { 0x00000000, 0x000000e0, 0x73c3c79e, 0x0ff01f80, 0x7e00f003,
               0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x6CY
             { 0x00000000, 0x0000007f, 0xe1ff800e, 0x007803c0, 0x1e00f007,
               0x803c01e0, 0x07001ff8, 0x7fe00000, 0x00000000},    //0x6DZ
             { 0x00000000, 0x000000c7, 0x031c0df0, 0x27c01f00, 0xf703dc1e,
               0x3878e3ff, 0xcfff383c, 0xc0300000, 0x00000000},//0x6e
             { 0x00000000, 0x000000dc, 0x7371cdc7, 0x271c1c70, 0x71c1ff07,
               0xfc1c7071, 0xc1c7071c, 0x1c700000, 0x00000000},//0x6f
             { 0x00000000, 0x00000000, 0x00000000, 0x01f01ff0, 0xffc3c70e,
               0x003800f1, 0xc3ff07fc, 0x07c00000, 0x00000000},    //0x70c
             { 0x00000000, 0x00000000, 0x7001c007, 0x03dc1ff0, 0xffc3870e,
               0x1c3870e1, 0xc3ff07fc, 0x0f700000, 0x00000000},    //0x71d
             { 0x00000000, 0x00000003, 0xc03f00f0, 0x03c01fc0, 0x7f01fc03,
               0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x72f
             { 0x00000000, 0x00000000, 0x00000000, 0x03ee3ff8, 0xe3e38f9c,
               0x1e7078e3, 0xe3ff87fe, 0x0078e1e3, 0xff07f800},    //0x73g
             { 0x00000000, 0x0000000f, 0x003c0000, 0x00000f00, 0x3c00f003,
               0xc00f003c, 0x00f003c0, 0x0f003c03, 0xf00f8000},    //0x74j
             { 0x00000000, 0x0000000e, 0x003800e0, 0x03800e00, 0x3800e003,
               0x800e0038, 0x00e00380, 0x0e000000, 0x00000000},    //0x75l
             { 0x00000000, 0x00000000, 0x00000000, 0x07dc3ff1, 0xe7c79f1e,
               0x3c78f1e3, 0xc79f0ffc, 0x1ff003c0, 0x0f003c00},    //0x76q
             { 0x00000000, 0x00000000, 0x00000000, 0x07b81fe0, 0x7f01f007,
               0x801e0078, 0x01e00780, 0x1e000000, 0x00000000},    //0x77r
             { 0x00000000, 0x00000000, 0x00000000, 0x07f83ff0, 0xe1c3800f,
               0xf81ff001, 0xc3870ffc, 0x1fe00000, 0x00000000},    //0x78s
             { 0x00000000, 0x00000000, 0x00000000, 0x1c3870e1, 0xc3870e1c,
               0x3870e1c3, 0x870e1ff8, 0x3fc00000, 0x00000000},    //0x79u
             { 0x00000000, 0x00000000, 0x00000000, 0x1e3c78f0, 0xf783de07,
               0x701fc03e, 0x00f801c0, 0x02000000, 0x00000000},    //0x7av
             { 0x00000000, 0x00000000, 0x00000000, 0x3bdeef7b, 0xbdeef7bb,
               0xdeef7bbd, 0xeef7bffe, 0x79e00000, 0x00000000},    //0x7bw
             { 0x00000000, 0x00000000, 0x00000000, 0x1c3870e0, 0xef03bc0e,
               0xf01fc07e, 0x00f803e0, 0x07003c03, 0xc00e0000},    //0x7cy
             { 0x00000000, 0x00000000, 0x00000000, 0x0ff03fc0, 0x0f003c01,
               0xe00f0070, 0x03800ff0, 0x3fc00000, 0x00000000},    //0x7dz
             { 0x00000000, 0x000000df, 0xf37fcdc0, 0x27001c00, 0x7001ff07,
               0xfc1c0070, 0x01c007fc, 0x1ff00000, 0x00000000},//0x7e
             { 0x0001ffe7, 0xff9ffe7f, 0xf9ffe7ff, 0x9ffe7ff9, 0xffe7ff9f,
               0xfe7ff9ff, 0xe7ff8000, 0x00000000, 0x00000000},    //  127  0x7f
        },
};

UINT32* get_char_from_g2_greek(UINT8 charset, UINT8 character, UINT8 i)
{
    if((0xff == charset)||(character>=96)||(i>=9))
   {
       return NULL;//error value;
   }
   return((UINT32*)&ttx_g2_greek_font[charset][character][i]);
}

#endif