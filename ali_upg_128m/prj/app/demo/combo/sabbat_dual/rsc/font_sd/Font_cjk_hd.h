const short fonts_cjk_name_hd[32] __attribute__((section(".flash"))) =
{
    0x3000,    0x3001,    0x3002,    0x3003,    0x3004,    0x3005,    0x3006,    0x3007,    0x3008,    0x3009,    0x300a,    0x300b,    0x300c,    0x300d,    0x300e,    0x300f,
    0x3010,    0x3011,    0x3012,    0x3013,    0x3014,    0x3015,    0x3016,    0x3017,    0x3018,    0x3019,    0x301a,    0x301b,    0x301c,    0x301d,    0x301e,    0x301f
};
const UINT8 fonts_cjk_width_hd[] __attribute__((section(".flash"))) =
{
    8,    11,    11,    20,    27,    23,    25,    27,    26,    12,    27,    14,    26,    13,    27,    14,
    27,    13,    24,    25,    26,    11,    26,    13,    25,    11,    25,    12,    28,    25,    15,    17
};
const UINT32 fonts_cjk_data_hd[] __attribute__((section(".flash"))) =
{
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3000*/
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3001*/
    0x00000000,      0x00000000,      0x00000000,      0x00000700,
    0xf00f00f0,      0x0f00e018,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3002*/
    0x00000000,      0x00000000,      0x00000000,      0x001e0661,
    0x8630c618,      0x66078000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3003*/
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00006600,      0x0ee001dc,      0x00198003,      0xb8007700,
    0x066000cc,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3004*/
    0x00000000,      0x00078c00,      0x0381e001,      0xc03e0070,
    0x06e01c00,      0xcc030018,      0xc0c06318,      0x180c6186,
    0x01f830c0,      0x30061806,      0x00c30fc0,      0x18633803,
    0x0c670061,      0x8c601819,      0xc003031e,      0x00c031e0,
    0x38071c0e,      0x00718380,      0x0731c000,      0x3de00000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3005*/
    0x00000c00,      0x003c0000,      0x700001e0,      0x00038000,
    0x0fffe03f,      0xffc0700f,      0x01e01e07,      0x80780601,
    0xe00003c0,      0x018f0007,      0xfc0007f0,      0x0007e000,
    0x07c00003,      0xc00003c0,      0x0003c000,      0x03000000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3006*/
    0x00000000,      0x00000000,      0x00000000,      0x00000300,
    0x0003c000,      0x03c00003,      0xc00003c0,      0x03e3c003,
    0xffc001df,      0xc001e780,      0x00e7e000,      0x77f8007f,
    0x98003f80,      0x001f8000,      0x0f80000f,      0x80000780,
    0x00018000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3007*/
    0x00000000,      0x0007f000,      0x0783c001,      0xc01c0060,
    0x00c01800,      0x0c070001,      0xc0c00018,      0x18000306,
    0x000030c0,      0x00061800,      0x00c30000,      0x18600003,
    0x0c000060,      0xc0001818,      0x00030380,      0x00e03000,
    0x18030006,      0x00380380,      0x03c1e000,      0x0fe00000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000001,  /*0x3008*/
    0x800000c0,      0x00003000,      0x00180000,      0x0c000003,
    0x00000180,      0x00006000,      0x00300000,      0x18000006,
    0x00000300,      0x0000c000,      0x00180000,      0x06000000,
    0xc0000018,      0x00000600,      0x0000c000,      0x00300000,
    0x06000000,      0xc0000030,      0x00000600,      0x00000000,
    0x00000000,      0x00000000,
    0x00000000,      0x00003001,      0x801800c0,      0x06006003,  /*0x3009*/
    0x00300180,      0x0c00c006,      0x00600c00,      0xc0180300,
    0x30060060,      0x0c018018,      0x03000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x300a*/
    0x6c00001b,      0x00000360,      0x0000d800,      0x00360000,
    0x06c00001,      0xb0000036,      0x00000d80,      0x00036000,
    0x006c0000,      0x1b000003,      0x60000036,      0x000006c0,
    0x00006c00,      0x0006c000,      0x00d80000,      0x0d800001,
    0xb000001b,      0x000001b0,      0x00003600,      0x00036000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x0000006c,      0x00d80360,      0x06c00d80,  /*0x300b*/
    0x36006c01,      0xb0036006,      0xc01b0036,      0x00d806c0,
    0x1b00d806,      0xc01b00d8,      0x03601b00,      0xd803601b,
    0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x000001ff,  /*0x300c*/
    0x80006000,      0x00180000,      0x06000001,      0x80000060,
    0x00001800,      0x00060000,      0x01800000,      0x60000018,
    0x00000600,      0x00018000,      0x00600000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x300d*/
    0x00000000,      0x00000000,      0x00000030,      0x01800c00,
    0x60030018,      0x00c00600,      0x3001800c,      0x7fe00000,
    0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x0000001f,  /*0x300e*/
    0xfc000301,      0x80006030,      0x000dfe00,      0x01b00000,
    0x36000006,      0xc00000d8,      0x00001b00,      0x00036000,
    0x006c0000,      0x0d800001,      0xf0000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x300f*/
    0x00000000,      0x00000000,      0x00000000,      0x00f80360,
    0x0d803600,      0xd803600d,      0x803600d8,      0xff63018c,
    0x063ff800,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000007,  /*0x3010*/
    0xfc0000ff,      0x00001fc0,      0x0003f800,      0x007e0000,
    0x0fc00001,      0xf800003f,      0x000007c0,      0x0000f800,
    0x001f0000,      0x03e00000,      0x7c00000f,      0x800001f0,
    0x00003e00,      0x0007c000,      0x00fc0000,      0x1f800003,
    0xf000007f,      0x00000fe0,      0x0001fe00,      0x003fe000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x000001ff,      0x07f81fc0,      0xfe03f01f,  /*0x3011*/
    0x80fc07e0,      0x1f00f807,      0xc03e01f0,      0x0f807c03,
    0xe01f01f8,      0x0fc07e07,      0xf03f83fc,      0x3fe00000,
    0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3012*/
    0x00000000,      0x000ffffe,      0x0ffffe00,      0x00000000,
    0x000ffffe,      0x0ffffe00,      0x0e00000e,      0x00000e00,
    0x000e0000,      0x0e00000e,      0x00000e00,      0x000e0000,
    0x0e00000e,      0x00000e00,      0x000e0000,      0x0e00000e,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x3013*/
    0x00000000,      0x00001fff,      0xfe0fffff,      0x07ffff83,
    0xffffc1ff,      0xffe0ffff,      0xf0000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00007fff,      0xf83ffffc,      0x1ffffe0f,      0xffff07ff,
    0xff83ffff,      0xc0000000,      0x00000000,      0x00000000,
    0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000003,  /*0x3014*/
    0x800001c0,      0x0000e000,      0x00700000,      0x3800001c,
    0x00000600,      0x00018000,      0x00600000,      0x18000006,
    0x00000180,      0x00006000,      0x00180000,      0x06000001,
    0x80000060,      0x00001800,      0x00060000,      0x01c00000,
    0x38000007,      0x80000070,      0x00000e00,      0x00000000,
    0x00000000,      0x00000000,
    0x00000000,      0x00038038,      0x03803803,      0x80380300,  /*0x3015*/
    0x600c0180,      0x300600c0,      0x18030060,      0x0c018030,
    0x0e0381e0,      0x701c0000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x000001ff,  /*0x3016*/
    0x800063c0,      0x0018e000,      0x06300001,      0x9c000066,
    0x00001980,      0x0006e000,      0x01b00000,      0x6c00001b,
    0x000006c0,      0x0001b000,      0x006c0000,      0x1b000006,
    0xc00001b8,      0x00006600,      0x00198000,      0x06700001,
    0x8c000063,      0x800018f0,      0x0007fe00,      0x00000000,
    0x00000000,      0x00000000,
    0x00000000,      0x000003ff,      0x0e1838c1,      0xc60e3031,  /*0x3017*/
    0x81cc0e60,      0x3301980c,      0xc0660330,      0x1980cc06,
    0x60330398,      0x18c0c60e,      0x3071870c,      0x7fe00000,
    0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000030,  /*0x3018*/
    0x00003000,      0x00380000,      0x3800003c,      0x00001e00,
    0x000f0000,      0x07800003,      0xc00001e0,      0x0000f000,
    0x00780000,      0x3c00001e,      0x00000f00,      0x00078000,
    0x03c00001,      0xe00000f0,      0x00007800,      0x001c0000,
    0x06000001,      0x80000060,      0x00000000,      0x00000000,
    0x00000000,
    0x00000000,      0x0000c00c,      0x01c01c03,      0xc0780f01,  /*0x3019*/
    0xe03c0780,      0xf01e03c0,      0x780f01e0,      0x3c0780f0,
    0x1e038060,      0x18060000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x301a*/
    0x0007f800,      0x03600001,      0xb00000d8,      0x00006c00,
    0x00360000,      0x1b00000d,      0x800006c0,      0x00036000,
    0x01b00000,      0xd800006c,      0x00003600,      0x001b0000,
    0x0d800006,      0xc0000360,      0x0001b000,      0x00d80000,
    0x6c000036,      0x00001b00,      0x000ff000,      0x00000000,
    0x00000000,
    0x00000000,      0x00000001,      0xfe036036,      0x03603603,  /*0x301b*/
    0x60360360,      0x36036036,      0x03603603,      0x60360360,
    0x36036036,      0x03603603,      0x60361fe0,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x301c*/
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x003800fe,      0x01c03c38,      0x0f0f00e0,      0x1f800600,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000300,  /*0x301d*/
    0x0019c000,      0x0e600003,      0x380001cc,      0x00006300,
    0x00198000,      0x0c600003,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,
    0x00000000,      0x00000000,      0x0c003980,      0xe7018c06,  /*0x301e*/
    0x381c6031,      0x80c3000c,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000000,      0x00000000,  /*0x301f*/
    0x00000000,      0x00000000,      0x00000000,      0x00000000,
    0x00000000,      0x00000000,      0x00000003,      0x00039803,
    0x9c018c01,      0x8e01c600,      0xc600c300,      0x03000000,
    0x00000000
};