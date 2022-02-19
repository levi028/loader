#ifndef __LLD_VZ7903_H
#define __LLD_VZ7903_H

#include <types.h>
#include <hld/nim/nim_tuner.h>

#define REF_OSC_FREQ    4000 /* 4MHZ  */

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _QM1D1C0045_INIT_CFG_DATA{
    QM1D1C0045_LOCAL_FREQ,
    QM1D1C0045_XTAL_FREQ,
    QM1D1C0045_CHIP_ID,
    QM1D1C0045_LPF_WAIT_TIME,
    QM1D1C0045_FAST_SEARCH_WAIT_TIME,
    QM1D1C0045_NORMAL_SEARCH_WAIT_TIME,
    QM1D1C0045_INI_CGF_MAX
}QM1D1C0045_INIT_CFG_DATA, *PQM1D1C0045_INIT_CFG_DATA ;

typedef enum _QM1D1C0045_INIT_REG_DATA{
    QM1D1C0045_REG_00 = 0,
    QM1D1C0045_REG_01,
    QM1D1C0045_REG_02,
    QM1D1C0045_REG_03,
    QM1D1C0045_REG_04,
    QM1D1C0045_REG_05,
    QM1D1C0045_REG_06,
    QM1D1C0045_REG_07,
    QM1D1C0045_REG_08,
    QM1D1C0045_REG_09,
    QM1D1C0045_REG_0A,
    QM1D1C0045_REG_0B,
    QM1D1C0045_REG_0C,
    QM1D1C0045_REG_0D,
    QM1D1C0045_REG_0E,
    QM1D1C0045_REG_0F,
    QM1D1C0045_REG_10,
    QM1D1C0045_REG_11,
    QM1D1C0045_REG_12,
    QM1D1C0045_REG_13,
    QM1D1C0045_REG_14,
    QM1D1C0045_REG_15,
    QM1D1C0045_REG_16,
    QM1D1C0045_REG_17,
    QM1D1C0045_REG_18,
    QM1D1C0045_REG_19,
    QM1D1C0045_REG_1A,
    QM1D1C0045_REG_1B,
    QM1D1C0045_REG_1C,
    QM1D1C0045_REG_1D,
    QM1D1C0045_REG_1E,
    QM1D1C0045_REG_1F,
    QM1D1C0045_INI_REG_MAX
}QM1D1C0045_INIT_REG_DATA, *PQM1D1C0045_INIT_REG_DATA ;

typedef enum _QM1D1C0045_LPF_FC{
    QM1D1C0045_LPF_FC_4MHz=0,    //0000:4MHz
    QM1D1C0045_LPF_FC_6MHz,     //0001:6MHz
    QM1D1C0045_LPF_FC_8MHz,     //0010:8MHz
    QM1D1C0045_LPF_FC_10MHz,    //0011:10MHz
    QM1D1C0045_LPF_FC_12MHz,    //0100:12MHz
    QM1D1C0045_LPF_FC_14MHz,    //0101:14MHz
    QM1D1C0045_LPF_FC_16MHz,    //0110:16MHz
    QM1D1C0045_LPF_FC_18MHz,    //0111:18MHz
    QM1D1C0045_LPF_FC_20MHz,    //1000:20MHz
    QM1D1C0045_LPF_FC_22MHz,    //1001:22MHz
    QM1D1C0045_LPF_FC_24MHz,    //1010:24MHz
    QM1D1C0045_LPF_FC_26MHz,    //1011:26MHz
    QM1D1C0045_LPF_FC_28MHz,    //1100:28MHz
    QM1D1C0045_LPF_FC_30MHz,    //1101:30MHz
    QM1D1C0045_LPF_FC_32MHz,    //1110:32MHz
    QM1D1C0045_LPF_FC_34MHz,    //1111:34MHz
    QM1D1C0045_LPF_FC_MAX,
}QM1D1C0045_LPF_FC;

typedef enum _QM1D1C0045_LPF_ADJUSTMENT_CURRENT{
    QM1D1C0045_LPF_ADJUSTMENT_CURRENT_25UA=0,    //0x1B b[1:0] = b00
    QM1D1C0045_LPF_ADJUSTMENT_CURRENT_DUMMY1,    //0x1B b[1:0] = b01
    QM1D1C0045_LPF_ADJUSTMENT_CURRENT_37R5UA,    //0x1B b[1:0] = b10
    QM1D1C0045_LPF_ADJUSTMENT_CURRENT_DUMMY2,    //0x1B b[1:0] = b11
}QM1D1C0045_LPF_ADJUSTMENT_CURRENT;

typedef struct _QM1D1C0045_CONFIG_STRUCT {
    unsigned int        ui_QM1D1C0045_RFChannelkHz;    /* direct channel */
    unsigned int        ui_QM1D1C0045_XtalFreqKHz;
    BOOL                b_QM1D1C0045_fast_search_mode;
    BOOL                b_QM1D1C0045_loop_through;
    BOOL                b_QM1D1C0045_tuner_standby;
    BOOL                b_QM1D1C0045_head_amp;
    QM1D1C0045_LPF_FC    QM1D1C0045_lpf;
    unsigned int        ui_QM1D1C0045_LpfWaitTime;
    unsigned int        ui_QM1D1C0045_FastSearchWaitTime;
    unsigned int        ui_QM1D1C0045_NormalSearchWaitTime;
    BOOL                b_QM1D1C0045_iq_output;
} QM1D1C0045_CONFIG_STRUCT, *PQM1D1C0045_CONFIG_STRUCT;


//=========================================================================
// GLOBAL VARIALBLES
//=========================================================================

const unsigned long QM1D1C0045_d[QM1D1C0045_INI_CGF_MAX]={
    0x0017a6b0,    //    LOCAL_FREQ,
    0x00003e80,    //    XTAL_FREQ,
    0x00000068,    //    CHIP_ID,
    0x00000014,    //    LPF_WAIT_TIME,
    0x00000004,    //    FAST_SEARCH_WAIT_TIME,
    0x0000000f,    //    NORMAL_SEARCH_WAIT_TIME,
};
UINT8    QM1D1C0045_d_reg[QM1D1C0045_INI_REG_MAX]={
    0x68,    //0x00
    0x1c,    //0x01
    0xc0,    //0x02
    0x10,    //0x03
    0xbc,    //0x04
    0xc1,    //0x05
    0x15,    //0x06
    0x34,    //0x07
    0x06,    //0x08
    0x3e,    //0x09
    0x00,    //0x0a
    0x00,    //0x0b
    0x43,    //0x0c
    0x00,    //0x0d
    0x00,    //0x0e
    0x00,    //0x0f
    0x00,    //0x10
    0xff,    //0x11
    0xf3,    //0x12
    0x00,    //0x13
//    0x3f,    //0x14
    0x3e,    //0x14
    0x25,    //0x15
    0x5c,    //0x16
    0xd6,    //0x17
    0x55,    //0x18
//    0xcf,    //0x19
    0x8f,    //0x19
    0x95,    //0x1a
//    0xf6,    //0x1b
    0xfe,    //0x1b
    0x36,    //0x1c
    0xf2,    //0x1d
    0x09,    //0x1e
    0x00,    //0x1f
};
const UINT8    QM1D1C0045_d_flg[QM1D1C0045_INI_REG_MAX]=    /* 0:R, 1:R/W */
{
    0,    // 0x0
    1,    // 0x1
    1,    // 0x2
    1,    // 0x3
    1,    // 0x4
    1,    // 0x5
    1,    // 0x6
    1,    // 0x7
    1,    // 0x8
    1,    // 0x9
    1,    // 0xA
    1,    // 0xB
    1,    // 0xC
    0,    // 0xD
    0,    // 0xE
    0,    // 0xF
    0,    // 0x10
    1,    // 0x11
    1,    // 0x12
    1,    // 0x13
    1,    // 0x14
    1,    // 0x15
    1,    // 0x16
    1,    // 0x17
    1,    // 0x18
    1,    // 0x19
    1,    // 0x1A
    1,    // 0x1B
    1,    // 0x1C
    1,    // 0x1D
    1,    // 0x1E
    1,    // 0x1F
};
const unsigned long QM1D1C0045_local_f[]={//kHz
//    2151000,
    2551000,
    1950000,
    1800000,
    1600000,
    1450000,
    1250000,
    1200000,
    975000,
    950000,
    0,
    0,
    0,
    0,
    0,
    0,
};
const unsigned long QM1D1C0045_div2[]={
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
const unsigned long QM1D1C0045_vco_band[]={
    7,
    6,
    5,
    4,
    3,
    2,
    7,
    6,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
const unsigned long QM1D1C0045_div45_lband[]={
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};




INT32 nim_vz7903_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_vz7903_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_vz7903_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_VZ7903_H__ */

