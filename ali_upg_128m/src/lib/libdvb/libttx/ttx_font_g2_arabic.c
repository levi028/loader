/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ttx_font_g2_arabic.c
*
*    Description: The file is mainly to define the font set of arabic G2 and
     the function to get char from the font set.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <basic_types.h>
#include <sys_config.h>

//#define SUPPORT_PACKET_26

#if defined(TTX_ARABIC_G2_SUPPORT)

static const UINT32 ttx_g2_arabic_font[][96][9] =
{
    //Arabic G2 set, 0x20~0x7f maps to 0x20~0x7f
        {
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
              0x00000000, 0x00000000, 0x00000000, 0x00000000},    //  032  0x20
            { 0x00000000, 0x00000000, 0x001e00f8, 0x03800e00, 0x3800fc01,
              0xf00e0078, 0x01c00700, 0x1c007801, 0xff03fc00},//0x21
            { 0x00000000, 0x000fc03f, 0x00000000, 0x01c00700, 0x1c007001,
              0xc007001c, 0x00700000, 0x00000000, 0x00000000},//0x22
            { 0x0f007c01, 0x800fc03f, 0x00000000, 0x01c00700, 0x1c007001,
              0xc007001c, 0x00700000, 0x00000000, 0x00000000},//0x23
            { 0x00000000, 0x1c00f003, 0x001f807e, 0x00000000, 0x1f80ff03,
              0x9c0e703f, 0xe07f801c, 0x00f00383, 0xfc0fe000},//0x24
            { 0x00000000, 0x7001c007, 0x001c0070, 0x01c00700, 0x1c007001,
              0xc007001c, 0x00700000, 0x0f007c01, 0x800fc000},//0x25
            { 0x00000000, 0x1c00f003, 0x001f807e, 0x00000000, 0x00003800,
              0xf000c3ff, 0x0ff80000, 0x00000000, 0x00000000},//0x26
            { 0x00000000, 0x1c00f003, 0x001f807e, 0x00000000, 0x03801f38,
              0x6ce1bb86, 0x6e0c3ff0, 0x7f800000, 0x00000000},//0x27
            { 0x00000000, 0x00000000, 0x00000000, 0x00001f01, 0xfe071800,
              0x7800e3ff, 0x8ffe0000, 0x70e1c380, 0x7001c000},//0x28
            //{ 0x00000000, 0x00000000, 0x00000000, 0x00001f01, 0xfe071800,
              //0x7800e3ff, 0xefff8000, 0x70e1c380, 0x7001c000},//0x29
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000e001c,
              0x0038007f, 0xf0ffc000, 0x00000000, 0x00000000},//0x29
            { 0x00000000, 0x00000000, 0x007e07f8, 0x1c7873e0, 0x1fe1ff8f,
              0x8678d9cf, 0x07301c20, 0x7081c0e7, 0xff8ffc00},//0x2a
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00003800,
              0xf000c3ff, 0x0ff80000, 0x70e1c380, 0x7001c000},//0x2b
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00007003,
              0xc00f03f7, 0xefdf8000, 0x70e1c380, 0x7001c000},//0x2c
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000e03b8,
              0x0ee03bff, 0xe7ff0000, 0x70e1c380, 0x7001c000},//0x2d
            { 0x00000c00, 0x3000c01c, 0xe07381ce, 0x00000000, 0x00000000,
              0x3c00f003, 0xe00f801c, 0x00f00383, 0xfc0fe000},//0x2e
            { 0x0000e1e7, 0x8fb838f1, 0xc3ce0070, 0x03c00700, 0x0e001c00,
              0x3800f3ff, 0xefff8000, 0x00000000, 0x00000000},//0x2f
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000003,
              0xc00f003c, 0x00f00000, 0x00000000, 0x00000000},//0x30
            { 0x00000000, 0x7001c007, 0x001c0070, 0x01c00700, 0x1c007001,
              0xc007001c, 0x00700000, 0x00000000, 0x00000000},//0x31
            { 0x00000003, 0xfc1ff078, 0x01c00700, 0x1c007000, 0xe0038007,
              0x001e003c, 0x00700000, 0x00000000, 0x00000000},//0x32
            { 0x00000007, 0x7e1df87f, 0xe1ff8700, 0x1c007001, 0xc007001c,
              0x007001c0, 0x07000000, 0x00000000, 0x00000000},//0x33
            { 0x00000001, 0xf00fc038, 0x00e003c0, 0x07000f00, 0x3c00e007,
              0x801c007f, 0x81fe0000, 0x00000000, 0x00000000},//0x34
            { 0x00000000, 0x00000000, 0x00000000, 0x07f03fe0, 0xe387071c,
              0x1c38e0ff, 0x81fc0000, 0x00000000, 0x00000000},//0x35
            { 0x00000003, 0xfe0ff800, 0xe003800e, 0x003800e0, 0x03800e00,
              0x3800e003, 0x800e0000, 0x00000000, 0x00000000},//0x36
            { 0x00000007, 0x0e1c3870, 0xe1c3838c, 0x0e3038c0, 0x6201d807,
              0x600d003c, 0x00700000, 0x00000000, 0x00000000},//0x37
            { 0x00000000, 0x7003c00d, 0x007601d8, 0x062038c0, 0xe3038c1c,
              0x3870e1c3, 0x870e0000, 0x00000000, 0x00000000},//0x38
            { 0x00000000, 0xfc07f81c, 0xe07381fe, 0x03f800e0, 0x03800e00,
              0x3800e003, 0x800e0000, 0x00000000, 0x00000000},//0x39
            { 0x00000600, 0x18006000, 0x000001fe, 0x07f80780, 0x0f007f03,
              0xfe1e38f0, 0x03800e00, 0x3800e003, 0xff07fc00},//0x3a
            { 0x03000c00, 0x30000000, 0x001e00f8, 0x03800e00, 0x3800fc01,
              0xf00e0078, 0x01c00700, 0x1c007801, 0xff03fc00},//0x3b
            { 0x00000000, 0x0e1c3870, 0xe0e7839e, 0x07f81fe0, 0x3f807e03,
              0xb80ee073, 0xe7c79e00, 0x00000000, 0x00000000},//0x3c
            { 0x00000000, 0x00000000, 0x00000000, 0x00003f80, 0xff038c0e,
              0x703983ff, 0xefff8e60, 0x39c0e303, 0xfc07e000},//0x3d
            { 0x00000000, 0x00000000, 0x0000000e, 0x003807e0, 0x7f83ce1e,
              0x3870f1fe, 0xe3fb8000, 0x00000000, 0x00000000},//0x3e
            { 0x00000000, 0x00000000, 0x00000000, 0x00001fc0, 0xff838e1c,
              0x1c7071ff, 0xc3fe0000, 0x00000000, 0x00000000},//0x3f
            { 0x1c003800, 0x7001c000, 0x00000000, 0x07bc7ff3, 0xe7cf0f3c,
              0x3cf0f3c3, 0xcf9f1ffc, 0x1ef00000, 0x00000000},    //0x40
            { 0x00000000, 0x00000007, 0x001c01f8, 0x07e01f80, 0xf783de1e,
              0x3c78f3ff, 0xefffb81e, 0xc0180000, 0x00000000},    //0x41
            { 0x00000000, 0x0000003f, 0x80ff03ce, 0x0f383ce0, 0xf383fc0f,
              0xf83c70f1, 0xc3c70ff8, 0x3fc00000, 0x00000000},    //0x42
            { 0x00000000, 0x0000001f, 0x80ff03ce, 0x1e1c7871, 0xe007801e,
              0x007801e1, 0xc3ce0ff0, 0x1f800000, 0x00000000},    //0x43
            { 0x00000000, 0x0000007f, 0x81ff071c, 0x1c3c70f1, 0xc3c70f1c,
              0x3c70f1c3, 0xc71c1ff0, 0x7f800000, 0x00000000},    //0x44
            { 0x00000000, 0x0000007f, 0xe1ff8700, 0x1c007001, 0xc007fc1f,
              0xf07001c0, 0x07001ff8, 0x7fe00000, 0x00000000},    //0x45
            { 0x00000000, 0x0000003f, 0xe0ff83c0, 0x0f003c00, 0xf003fc0f,
              0xf03c00f0, 0x03c00f00, 0x3c000000, 0x00000000},    //0x46
            { 0x00000000, 0x0000000f, 0xc0ff838f, 0x9c1e7079, 0xc007001c,
              0x7e71f9c1, 0xe38f8ffe, 0x0fc00000, 0x00000000},    //0x47
            { 0x00000000, 0x00000070, 0x71c1c707, 0x1c1c7071, 0xc1c7ff1f,
              0xfc7071c1, 0xc7071c1c, 0x70700000, 0x00000000},    //0x48
            { 0x00000000, 0x0000000f, 0x003c00f0, 0x03c00f00, 0x3c00f003,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x49
            { 0x00000000, 0x00000000, 0xe003800e, 0x003800e0, 0x03800e00,
              0x3870e1c3, 0x878e0ff0, 0x1f800000, 0x00000000},    //0x4a
            { 0x00000000, 0x00000078, 0x39e1c78e, 0x1e707b81, 0xfc07e01f,
              0xc07f81e7, 0x078e1e1c, 0x78380000, 0x00000000},    //0x4b
            { 0x00000000, 0x0000001c, 0x007001c0, 0x07001c00, 0x7001c007,
              0x001c0070, 0x01c007fc, 0x1ff00000, 0x00000000},    //0x4c
            { 0x00000000, 0x0000007c, 0x7df1f7ef, 0xdfbf7efd, 0xfbf7ffde,
              0xe77b9dee, 0x7791de47, 0x791c0000, 0x00000000},    //0x4d
            { 0x00000000, 0x00000070, 0x39e0e7c3, 0x9f8e7e39, 0xfce7ff9e,
              0xfe79f9e3, 0xe7839e0e, 0x78380000, 0x00000000},    //0x4e
            { 0x00000000, 0x00000007, 0x80ffc3cf, 0x1e1e7879, 0xe1e7879e,
              0x1e7879e1, 0xe3cf0ffc, 0x07800000, 0x00000000},    //0x4f
            { 0x00000000, 0x0000007f, 0xe1ffc78f, 0x1e3c78f1, 0xe3c7ff1f,
              0xf87801e0, 0x07801e00, 0x78000000, 0x00000000},    //0x50
            { 0x00000000, 0x00000007, 0x80ffc3cf, 0x1e1e7879, 0xe1e7879e,
              0x1e7b79ed, 0xe3df0ffc, 0x07f80060, 0x00000000},    //0x51
            { 0x00000000, 0x0000007f, 0xc1ffc78f, 0x1e3c78f1, 0xe3c7fc1f,
              0xe07bc1e7, 0xc78f9e1e, 0x78380000, 0x00000000},    //0x52
            { 0x00000000, 0x0000001f, 0x80ff078f, 0x1e1c7801, 0xfe03fe07,
              0xfc00f1c3, 0xc79f0ff8, 0x1fc00000, 0x00000000},    //0x53
            { 0x00000000, 0x000000ff, 0xfbffe0f0, 0x03c00f00, 0x3c00f003,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x54
            { 0x00000000, 0x00000070, 0x79c1e707, 0x9c1e7079, 0xc1e7079c,
              0x1e7079c1, 0xe7cf8ff8, 0x1fc00000, 0x00000000},    //0x55
            { 0x00000000, 0x000000e0, 0x7b81e70e, 0x1c383dc0, 0xf703dc07,
              0xe01f803c, 0x00f00080, 0x02000000, 0x00000000},    //0x56
            { 0x00000000, 0x000000e7, 0x3b9cee73, 0xb9cee73b, 0x9cee73bb,
              0xeefdfbe3, 0xef0fbc1e, 0x60300000, 0x00000000},    //0x57
            { 0x00000000, 0x000000e0, 0x7bc3e79e, 0x0ff01f80, 0x7e00f003,
              0xc01f80ff, 0x079e3c3c, 0xe0700000, 0x00000000},    //0x58
            { 0x00000000, 0x000000e0, 0x73c3c79e, 0x0ff01f80, 0x7e00f003,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x59
            { 0x00000000, 0x0000007f, 0xe1ff800e, 0x007803c0, 0x1e00f007,
              0x803c01e0, 0x07001ff8, 0x7fe00000, 0x00000000},    //0x5a
            { 0x79e1e787, 0x9e1e7800, 0x00000000, 0x07c07fc3, 0xef8f1e3f,
              0xf8ffe3c0, 0x0f001ff0, 0x1f000000, 0x00000000},    //0x5b
            { 0x07803701, 0x86061800, 0x00000000, 0x07e03fc3, 0xef8f1e3f,
              0xf8ffe3c0, 0x0f000ff0, 0x1f800000, 0x00000000},    //0x5c
            { 0x1e003c00, 0x38000000, 0x00000000, 0x1c3870e1, 0xc3870e1c,
              0x3870e1c3, 0x87fe1ff8, 0x3fc00000, 0x00000000},    //0x5d
            { 0x0f007603, 0x0c000000, 0x00000000, 0x00000f00, 0x3c00f003,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x5e
            { 0x00000000, 0x00000000, 0x000001fe, 0x07f80780, 0x0f007f03,
              0xfe1e38f0, 0x03800e00, 0x3800e003, 0xff07fc00},//0x5f
            { 0x03c01e00, 0xf003c000, 0x00000000, 0x07e03fc3, 0xef8f0e3f,
              0xf8ffe3c0, 0x0f000ff0, 0x1f800000, 0x00000000},    //0x60
            { 0x00000000, 0x00000000, 0x00000000, 0x07de3ff9, 0xf3e7cf9e,
              0x1e7879e1, 0xe7cf8ffe, 0x1f780000, 0x00000000},    //0x61
            { 0x00000000, 0x00000078, 0x01e00780, 0x1fe07fc1, 0xf3c7cf1e,
              0x3c78f1e3, 0xc7cf1ff0, 0x7f800000, 0x00000000},    //0x62
            { 0x00000000, 0x00000000, 0x00000000, 0x01f01ff0, 0xffc3c70e,
              0x003800f1, 0xc3ff07fc, 0x07c00000, 0x00000000},    //0x63
            { 0x00000000, 0x00000000, 0x7001c007, 0x03dc1ff0, 0xffc3870e,
              0x1c3870e1, 0xc3ff07fc, 0x0f700000, 0x00000000},    //0x64
            { 0x00000000, 0x00000000, 0x00000000, 0x03f01fe0, 0xf3c3870f,
              0xfc3ff0e0, 0x03c707fc, 0x0fe00000, 0x00000000},    //0x65
            { 0x00000000, 0x00000003, 0xc03f00f0, 0x03c01fc0, 0x7f01fc03,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x66
            { 0x00000000, 0x00000000, 0x00000000, 0x03ee3ff8, 0xe3e38f9c,
              0x1e7078e3, 0xe3ff87fe, 0x0078e1e3, 0xff07f800},    //0x67
            { 0x00000000, 0x00000070, 0x01c00700, 0x1c007fc1, 0xff87fe1c,
              0x3870e1c3, 0x870e1c38, 0x70e00000, 0x00000000},    //0x68
            { 0x00000000, 0x0000000f, 0x003c0000, 0x00000f00, 0x3c00f003,
              0xc00f003c, 0x00f003c0, 0x0f000000, 0x00000000},    //0x69
            { 0x00000000, 0x0000000f, 0x003c0000, 0x00000f00, 0x3c00f003,
              0xc00f003c, 0x00f003c0, 0x0f003c03, 0xf00f8000},    //0x6a
            { 0x00000000, 0x000000f0, 0x03c00f00, 0x3cf0f783, 0xfc0fe03f,
              0x00fc03fc, 0x0f783cf0, 0xf0e00000, 0x00000000},    //0x6b
            { 0x00000000, 0x0000000e, 0x003800e0, 0x03800e00, 0x3800e003,
              0x800e0038, 0x00e00380, 0x0e000000, 0x00000000},    //0x6c
            { 0x00000000, 0x00000000, 0x00000000, 0x3ff8fff3, 0xccef33bc,
              0xcef33bcc, 0xef33bcce, 0xf3380000, 0x00000000},    //0x6d
            { 0x00000000, 0x00000000, 0x00000000, 0x1ff07ff1, 0xe3c78f1e,
              0x3c78f1e3, 0xc78f1e3c, 0x78f00000, 0x00000000},    //0x6e
            { 0x00000000, 0x00000000, 0x00000000, 0x03e03fe1, 0xf7c78f1e,
              0x3c78f1e3, 0xc7df0ff8, 0x0f800000, 0x00000000},    //0x6f
            { 0x00000000, 0x00000000, 0x00000000, 0x1de07fe1, 0xf3c7cf1e,
              0x3c78f1e3, 0xc7cf1ff8, 0x7f81e007, 0x801e0000},    //0x70
            { 0x00000000, 0x00000000, 0x00000000, 0x07dc3ff1, 0xe7c79f1e,
              0x3c78f1e3, 0xc79f0ffc, 0x1ff003c0, 0x0f003c00},    //0x71
            { 0x00000000, 0x00000000, 0x00000000, 0x07b81fe0, 0x7f01f007,
              0x801e0078, 0x01e00780, 0x1e000000, 0x00000000},    //0x72
            { 0x00000000, 0x00000000, 0x00000000, 0x07f83ff0, 0xe1c3800f,
              0xf81ff001, 0xc3870ffc, 0x1fe00000, 0x00000000},    //0x73
            { 0x00000000, 0x00000003, 0x003c00f0, 0x07f01fc0, 0x3c00f003,
              0xc00f003c, 0x00f003f0, 0x03c00000, 0x00000000},    //0x74
            { 0x00000000, 0x00000000, 0x00000000, 0x1c3870e1, 0xc3870e1c,
              0x3870e1c3, 0x870e1ff8, 0x3fc00000, 0x00000000},    //0x75
            { 0x00000000, 0x00000000, 0x00000000, 0x1e3c78f0, 0xf783de07,
              0x701fc03e, 0x00f801c0, 0x02000000, 0x00000000},    //0x76
            { 0x00000000, 0x00000000, 0x00000000, 0x3bdeef7b, 0xbdeef7bb,
              0xdeef7bbd, 0xeef7bffe, 0x79e00000, 0x00000000},    //0x77
            { 0x00000000, 0x00000000, 0x00000000, 0x1c1c78f0, 0xff83fe07,
              0xf00f807f, 0x03fe1e3c, 0x70700000, 0x00000000},    //0x78
            { 0x00000000, 0x00000000, 0x00000000, 0x1c3870e0, 0xef03bc0e,
              0xf01fc07e, 0x00f803e0, 0x07003c03, 0xc00e0000},    //0x79
            { 0x00000000, 0x00000000, 0x00000000, 0x0ff03fc0, 0x0f003c01,
              0xe00f0070, 0x03800ff0, 0x3fc00000, 0x00000000},    //0x7a
            { 0x03801b81, 0xc7071c00, 0x00000000, 0x07de3ff9, 0xf3e7879e,
              0x1e7879f3, 0xe7cf8ffe, 0x1f780000, 0x00000000},    //0x7b
            { 0x07803301, 0x86000000, 0x00000000, 0x07e03fc3, 0xef8f0e3c,
              0x38f0e3ef, 0x8fbe0ff0, 0x1f800000, 0x00000000},    //0x7c
            { 0x0f006603, 0x0c000000, 0x00000000, 0x1c3870e1, 0xc3870e1c,
              0x3870e1c3, 0x87fe1ff8, 0x3fc00000, 0x00000000},    //0x7d
            { 0x00000000, 0x0000001f, 0x81ff079e, 0x3c18f003, 0xc00f003c,
              0x00f063c1, 0x879e1ff0, 0x1f80380f, 0xe03f8000},    //0x7e
            { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
              0x00000000, 0x00000000, 0x00000000, 0x00000000}    //0x7f
        },
};

UINT32 *get_char_from_g2_arabic(UINT8 charset, UINT8 character, UINT8 i)
{
    if((0xff == charset)||(character>=96)||(i>=9))
   {
       return NULL;//error value;
   }
   return((UINT32*)&ttx_g2_arabic_font[charset][character][i]);
}

#endif
