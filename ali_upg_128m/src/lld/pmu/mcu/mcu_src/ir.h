#ifndef __IR_H__
#define __IR_H__
#include "sys.h"
//======================================================================================================================//

#define IR_BIT                                                                    (0)
#define IR_IOBASE                                                              (0xE000)
#define INTR_REG_STAT                                                       (IR_IOBASE+0x30)
#define INTR_REG_CTRL                                                       (IR_IOBASE+0x38)
#define INTR_REG_PRO                                                        (IR_IOBASE+0x28)
#define IR_REG_CFG                                                            (IR_IOBASE+0x00)
#define IR_REG_FIFOCTRL                                                    (IR_IOBASE+0x01)
#define IR_REG_TIMETHR                                                     (IR_IOBASE+0x02)
#define IR_REG_NOISETHR                                                   (IR_IOBASE+0x03)
#define IR_REG_IER                                                             (IR_IOBASE+0x06)
#define IR_REG_ISR                                                             (IR_IOBASE+0x07)
#define IR_REG_RLCBYTE                                                     (IR_IOBASE+0x08)
#define IR_KEY_MATCH_SUCCESS                                        (1)
#define IR_KEY_MATCH_FAIL                                               (0)
#define PMU_TM0_CTRL                                                   (0xE608)

#define IR_BUF_LEN                                                            (256)
#define CLK_CYC_US                                                           (16)/*Work clock cycle, in uS.*/
#define TIMEOUT_US                                                           (24000)/*Timeout threshold, in uS.*/
#define NOISETHR_US                                                         (80)/*Noise threshold, in uS.*/
#define PAN_KEY_INVALID                                                  (0xFFFFFFFF)
#define PULSE_LOW                                                            ((unsigned short)0x0 << 15)
#define PULSE_HIGH                                                           ((unsigned short)0x1 << 15)
#define PULSE_POL(x)                                                         (x & ((unsigned short)0x1 << 15))
#define PULSE_VALUE(x)                                                     (x & (~((unsigned short)0x1 << 15)))
#define INRANGE(x, value, tol)                                             (((x) > ((value)-(tol))) && ((x) < ((value)+(tol))))
//======================================================================================================================//

typedef struct
{
    UINT8 ir_key_low0;
    UINT8 ir_key_low1;
    UINT8 ir_key_low2;
    UINT8 ir_key_low3;
} IR_KEY, pIR_KEY;

enum irc_decode_ret
{
    IRC_DECODE_SUCCEED,
    IRC_DECODE_DATA0,
    IRC_DECODE_DATA1,
    IRC_DECODE_FAIL,
    IRC_DECODE_STOP,
};

enum irp_type
{
    IR_TYPE_NEC = 0,
    IR_TYPE_LAB,
    IR_TYPE_50560,
    IR_TYPE_KF,
    IR_TYPE_LOGIC,
    IR_TYPE_SRC,
    IR_TYPE_NSE,
    IR_TYPE_RC5,
    IR_TYPE_RC5_X,
};

enum ir_waveform
{
    IR_LEADING = 0,
    IR_SPECIAL,
    IR_DATA,
    IR_REPEAT_LEADING,
    IR_REPEAT_DATA,
    IR_STOP,
    IR_END,
    IR_NULL,
};

struct irc_pulse
{
    UINT8 type;
    UINT16 fst_half;
    UINT16 scd_half;
    UINT16 tolerance;
};

struct ir_attr
{
    UINT8 type;
    UINT1 bit_msb_first;
    UINT1 byte_msb_first;
    UINT1 pulse_invert;
    UINT1 repeat_enable;
    UINT1 ignore_lastpulse;
    UINT32 pulse_prec;
    UINT32 pulse_max_width;
    struct irc_pulse pulse[7];
    enum ir_waveform *normal;
    enum ir_waveform *repeat;
};

struct irc_decoder
{
    struct ir_attr *attr;
    UINT8 first_half_got;
    UINT8 decode_step;
    UINT8 key_bit_cnt;
    UINT16 last_pulse_width;
    UINT32 key_code[4];
    UINT32 last_key_code[4];
};

void ir_init(void);
void ir_process();

#endif
