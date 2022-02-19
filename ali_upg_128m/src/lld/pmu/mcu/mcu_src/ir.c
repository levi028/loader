#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <dp8051xp.h>
#endif
#include <stdio.h>
#include "ir.h"
#include "sys.h"
#include "panel.h"
#include "rtc.h"
//======================================================================================================================//
#define ALI_IR_POWER_KEY                                                         (0x60DF708F)
#ifdef __SDCC__
UINT32 at 0x3FC0 g_ir_power_key[8];
#else
UINT32 g_ir_power_key[8] _at_ 0x3FC0;
#endif
unsigned char bufferin = 0;
unsigned char bufferout = 0;
unsigned char ir_buffer[IR_BUF_LEN];
static UINT8 g_ir_flag = IR_KEY_MATCH_FAIL;
#define BigLittleSwap32(A)          ((((UINT32)(A) & 0xff000000) >> 24) \
                                    | (((UINT32)(A) & 0x00ff0000) >> 8) \
                                    | (((UINT32)(A) & 0x0000ff00) << 8) \
                                    | (((UINT32)(A) & 0x000000ff) << 24))

//======================================================================================================================//

static enum ir_waveform ir_nec_normal[] =
{
    IR_LEADING,/*start bit.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits command.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits address.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits ~command.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits ~address.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static enum ir_waveform ir_nec_repeat[] =
{ IR_REPEAT_LEADING, IR_END, };

static struct ir_attr ir_attr_nec =
{
    IR_TYPE_NEC, 1, 0, 0, 1, 0, 280, 15000,
    {
        { IR_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 4500, 500 },/*start pulse.*/
        { IR_NULL, 0, 0, 0 },
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200 },/*logic '0'.*/
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1680, 300 },/*logic '1'.*/
        { IR_REPEAT_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 2250, 500 },
        { IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000 },
        { IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000 },
    }, ir_nec_normal,
    ir_nec_repeat,
};

struct irc_decoder irc_decoders_nec =
{
    &ir_attr_nec, //.attr = &ir_attr_nec,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};
#if 0
static enum ir_waveform ir_lab[] =
{
    IR_LEADING, IR_SPECIAL, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static struct ir_attr ir_attr_lab =
{
    IR_TYPE_LAB, 1, 0, 0, 0, 0, 140, 15000,
    {
        { IR_LEADING, PULSE_LOW | 280, PULSE_HIGH | 7300, 140 },
        { IR_SPECIAL, PULSE_LOW | 280, PULSE_HIGH | 6150, 1500 },
        { IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 4780, 500 },
        { IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 7300, 500 },
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000 },
    }, ir_lab, NULL,
};

static enum ir_waveform ir_50560[] =
{
    IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_SPECIAL, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_SPECIAL, IR_END,
};

static struct ir_attr ir_attr_50560 =
{
    IR_TYPE_50560, 1, 0, 0, 1, 0, 260, 15000,
    {
        { IR_LEADING, PULSE_LOW | 8400, PULSE_HIGH | 4200, 500 },
        { IR_SPECIAL, PULSE_LOW | 520, PULSE_HIGH | 4200, 500 },
        { IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 1050, 500 },
        { IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 2100, 500 },
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000 },
    }, ir_50560, NULL,
};

static enum ir_waveform ir_kf[] =
{
    IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_END,
};

static struct ir_attr ir_attr_kf =
{
    IR_TYPE_KF, 0, 1, 0, 0, 0, 190, 20000,
    {
        { IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 500 },
        { IR_NULL, 0, 0, 0 },
        { IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 380, 150 },
        { IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 1350, 300 },
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000 },
    }, ir_kf, NULL,
};

static enum ir_waveform ir_logic[] =
{
    IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static struct ir_attr ir_attr_logic =
{
    IR_TYPE_LOGIC, 0, 0, 0, 0, 0, 125, 20000,
    {
        { IR_LEADING, PULSE_LOW | 5000, PULSE_HIGH | 5000, 500 },
        { IR_NULL, 0, 0, 0 },
        { IR_DATA, PULSE_LOW | 250, PULSE_HIGH | 500, 100 },
        { IR_DATA, PULSE_LOW | 500, PULSE_HIGH | 1000, 200 },
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000 },
    }, ir_logic, NULL,
};

static enum ir_waveform ir_src_normal[] =
{
    IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_SPECIAL, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static struct ir_attr ir_attr_src =
{
    IR_TYPE_SRC, 1, 0, 0, 1, 0, 280, 60000,
    {
        { IR_LEADING, PULSE_LOW | 6000, PULSE_HIGH | 2000, 400 },
        { IR_SPECIAL, PULSE_LOW | 3640, PULSE_HIGH | 3640, 400 },
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200 },
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300 },
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000 },
    },
    ir_src_normal, NULL,
};

static enum ir_waveform ir_nse_normal[] =
{
    IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_LEADING, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static enum ir_waveform ir_nse_repeat[] =
{ IR_REPEAT_LEADING, IR_END, };

static struct ir_attr ir_attr_nse =
{
    IR_TYPE_NSE, 1, 0, 0, 1, 0, 280, 60000,
    {
        { IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 400 },
        { IR_NULL, 0, 0, 0 },
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200 },
        { IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300 },
        { IR_REPEAT_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 3640, 500 },
        { IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000 },
        { IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000 },
    }, ir_nse_normal,
    ir_nse_repeat,
};

static enum ir_waveform ir_rc5[] =
{
    IR_LEADING, IR_LEADING,/*S1 & S2 bit.*/
    IR_SPECIAL,/*toggle bit.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*5 bits addr & 6 bits command.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_END,
};

static struct ir_attr ir_attr_rc5 =
{
    IR_TYPE_RC5,/*type name.*/
    1,/*bits MSB.*/
    0, 0, 0, 1,/*repeat enable.*/
    200, /*pulse prec.*/
    15000,/*pulse max width.*/
    {
        { IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*leading logic.*/
        { IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*toggle logic.*/
        { IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200 },/*data logic '0'.*/
        { IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*data logic '1'.*/
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000 },
    }, ir_rc5, NULL,
};

static enum ir_waveform ir_rc5_x[] =
{
    IR_LEADING, IR_DATA,/*S1 & S2 bit.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*5 bits addr & 6 bits command.*/
    IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA, IR_DATA,
};

static struct ir_attr ir_attr_rc5_x =
{
    IR_TYPE_RC5,/*type name.*/
    1,/*bits MSB.*/
    0, 0, 0, 1,/*repeat enable.*/
    200,/*pulse prec.*/
    15000,/*pulse max width.*/
    {
        { IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*leading logic.*/
        { IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*toggle logic.*/
        { IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200 },/*data logic '0'.*/
        { IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200 },/*data logic '1'.*/
        { IR_NULL, 0, 0, 0 },
        { IR_NULL, 0, 0, 0 },
        { IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000 },
    }, ir_rc5_x, NULL,
};

struct irc_decoder irc_decoders_lab =
{
    &ir_attr_lab, //.attr = &ir_attr_lab,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_50560 =
{
    &ir_attr_50560, //.attr = &ir_attr_50560,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_kf =
{
    &ir_attr_kf, //.attr = &ir_attr_kf,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.key_code = {0,},
    { 0, }, //.last_pulse_width = 0,
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_logic =
{
    &ir_attr_logic, //.attr = &ir_attr_logic,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_src =
{
    &ir_attr_src, //.attr = &ir_attr_src,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_nse =
{
    &ir_attr_nse, //.attr = &ir_attr_nse,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rc5 =
{
    &ir_attr_rc5, //.attr = &ir_attr_rc5,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rx5_x =
{
    &ir_attr_rc5_x, //.attr = &ir_attr_rc5_x,
    0, //.first_half_got = 0,
    0, //.decode_step = 0,
    0, //.key_bit_cnt = 0,
    0, //.last_pulse_width = 0,
    { 0, }, //.key_code = {0,},
    { 0, }, //.last_key_code = {0,},
};
#endif
static struct irc_decoder* irc_decoders[] =
{
    &irc_decoders_nec,
#if 0
    &irc_decoders_lab,
    &irc_decoders_50560,
    &irc_decoders_kf,
    &irc_decoders_logic,
    &irc_decoders_src,
    &irc_decoders_nse,
    &irc_decoders_rc5,
    &irc_decoders_rx5_x,
#endif
};
//======================================================================================================================//

static void COPY_CODE(UINT8* dst, UINT8* src)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        *(dst + i) = *(src + i);
    }
}

static void CLEAR_CODE(UINT8* p)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        *(p + i) = 0;
    }
}

static void reverse_code_bit(UINT32 *ir_code, UINT32 bit_cnt)
{
    UINT8 i = 0;
    UINT32 src_code[4] =
    { 0 };
    UINT32 dst_code[4] =
    { 0 };

    COPY_CODE((UINT8*) src_code, (UINT8*) ir_code);

    for (i = 0; i < bit_cnt; i++)
    {
        if (src_code[i / 32] & ((UINT32) 0x01L << (i % 32)))
        {
            dst_code[(bit_cnt - 1 - i) / 32] |= (UINT32) 0x01L
                    << ((bit_cnt - 1 - i) % 32);
        }
    }

    COPY_CODE((UINT8*) ir_code, (UINT8*) dst_code);
}

static void reverse_code_byte(UINT32 *ir_code, UINT8 bit_cnt)
{
    UINT8 i = 0;
    UINT8 src_code[16] =
    { 0 };
    UINT8 dst_code[16] =
    { 0 };
    UINT8 byte_cnt = (bit_cnt + 7) / 8;

    COPY_CODE((UINT8*) src_code, (UINT8*) ir_code);

    dst_code[0] = src_code[3];
    dst_code[1] = src_code[2];
    dst_code[2] = src_code[1];
    dst_code[3] = src_code[0];
    dst_code[4] = src_code[7];
    dst_code[5] = src_code[6];
    dst_code[6] = src_code[5];
    dst_code[7] = src_code[4];
    dst_code[8] = src_code[11];
    dst_code[9] = src_code[10];
    dst_code[10] = src_code[9];
    dst_code[11] = src_code[8];
    dst_code[12] = src_code[15];
    dst_code[13] = src_code[14];
    dst_code[14] = src_code[13];
    dst_code[15] = src_code[12];

    COPY_CODE((UINT8*) src_code, (UINT8*) dst_code);
    for (i = 0; i < byte_cnt; i++)
    {
        dst_code[i] = src_code[byte_cnt - i - 1];
    }

    src_code[0] = dst_code[3];
    src_code[1] = dst_code[2];
    src_code[2] = dst_code[1];
    src_code[3] = dst_code[0];
    src_code[4] = dst_code[7];
    src_code[5] = dst_code[6];
    src_code[6] = dst_code[5];
    src_code[7] = dst_code[4];
    src_code[8] = dst_code[11];
    src_code[9] = dst_code[10];
    src_code[10] = dst_code[9];
    src_code[11] = dst_code[8];
    src_code[12] = dst_code[15];
    src_code[13] = dst_code[14];
    src_code[14] = dst_code[13];
    src_code[15] = dst_code[12];

    COPY_CODE((UINT8*) ir_code, (UINT8*) src_code);
}

/*****************************************************************************
 * Description: Translate pulse width to IR code
 * Arguments:
 *	  struct irc_decoder *ird	: IR decode structure
 *    UINT32 pulse_width		: Input pulse width, in uS
 *    UINT32 pulse_polarity		: Input pulse polarity,
 *							'1' = high level, '0' = low level
 * Return Value:
 *    UINT32					: Key code
 ******************************************************************************/
static UINT32 irc_decode(UINT8 ird_index, UINT32 pulse_width,
UINT32 pulse_polarity)
{
    UINT32 accum_pulse_width = 0;
    enum irc_decode_ret result = IRC_DECODE_FAIL;
    enum ir_waveform step = IR_LEADING;
    enum ir_waveform next_step = IR_LEADING;
    UINT32 last_key = 0;
    UINT16 step_temp = 0;
    UINT32 accum_pulse = 0;
    UINT32 tolerance_pulse = 0;
    UINT32 same_polarity = 0;
    UINT32 accum_pulse_1 = 0;
    UINT32 tolerance_pulse_1 = 0;
    UINT32 same_polarity_1 = 0;
    UINT32 half_pol = 0;
    UINT32 half_width = 0;
    struct irc_decoder *ird = irc_decoders[ird_index];

    /*Ignore burr pulses.*/
    if (pulse_width < ird->attr->pulse_prec)
    {
        ird->first_half_got = 0;
        return PAN_KEY_INVALID;
    }

    if (ird->first_half_got == 0)/*first_half_got is flag for dule-pulse senser.*/
    {
        if (ird->decode_step == 0)/*just check leading pulse.*/
        {
            if (PULSE_POL(ird->attr->pulse[0].fst_half))/*if leading start with high level.*/
            {
                half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
                half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
            }
            else
            {
                half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
                half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
            }

            /*Check leading code first half pulse polarity.*/
            if (half_pol ^ ((UINT16)(pulse_polarity << 15)))
            {
                return PAN_KEY_INVALID;
            }

            /*Check leading code first half pulse width.*/
            if (!INRANGE(pulse_width, half_width,
            ird->attr->pulse[0].tolerance))
            {
                return PAN_KEY_INVALID;
            }
        }

        ird->first_half_got = 1;
        ird->last_pulse_width = pulse_width;
        if ((ird->decode_step == 0)
        && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
        {
            ird->first_half_got = 0;
            ird->last_pulse_width = 0;
            ird->decode_step++;
        }

        return PAN_KEY_INVALID;
    }
    else
    {
        accum_pulse_width = ird->last_pulse_width + pulse_width;
        ird->last_pulse_width = 0;
        ird->first_half_got = 0;

        /*Process normal key.*/
        if (ird->attr->normal != NULL)
        {
            step = ird->attr->normal[ird->decode_step];
            next_step = ird->attr->normal[ird->decode_step + 1];
            if (step > IR_DATA)
            {
                step++;
            }

            accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half)
            + PULSE_VALUE(ird->attr->pulse[step].scd_half);
            tolerance_pulse = ird->attr->pulse[step].tolerance;
            same_polarity = !((PULSE_POL(ird->attr->pulse[step].scd_half)
            ^ (pulse_polarity << 15)) != 0);

            if (ird->attr->type == IR_TYPE_RC5)
            {
                accum_pulse += PULSE_VALUE(
                    ird->attr->pulse[next_step].fst_half);
                tolerance_pulse += PULSE_VALUE(
                    ird->attr->pulse[next_step].fst_half);
            }

            if (ird->attr->normal[ird->decode_step] == IR_DATA)
            {
                accum_pulse_1 = PULSE_VALUE(
                    ird->attr->pulse[IR_DATA + 1].fst_half)
                + PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].scd_half);
                tolerance_pulse_1 = ird->attr->pulse[IR_DATA + 1].tolerance;
                same_polarity_1 = !((PULSE_POL(
                    ird->attr->pulse[IR_DATA + 1].scd_half)
                ^ (pulse_polarity << 15)) != 0);

                if (ird->attr->type == IR_TYPE_RC5)
                {
                    accum_pulse_1 += PULSE_VALUE(
                        ird->attr->pulse[IR_DATA + 1].fst_half);
                    tolerance_pulse_1 += PULSE_VALUE(
                        ird->attr->pulse[IR_DATA + 1].fst_half);
                }

                if ((((ird->attr->ignore_lastpulse)
                && (ird->attr->normal[ird->decode_step + 1] == IR_END))
                || INRANGE(accum_pulse_width, accum_pulse,
                tolerance_pulse)) && (same_polarity ? 1 : 0))
                {
                    result = IRC_DECODE_DATA0;
                }
                else if ((((ird->attr->ignore_lastpulse)
                && (ird->attr->normal[ird->decode_step + 1] == IR_END))
                || INRANGE(accum_pulse_width, accum_pulse_1,
                tolerance_pulse_1))
                && (same_polarity_1 ? 1 : 0))
                {
                    result = IRC_DECODE_DATA1;
                }
                else
                {
                    result = IRC_DECODE_FAIL;
                }
            }
            else
            {
                if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
                {
                    if ((ird->attr->type == IR_TYPE_RC5)
                    && (step == IR_SPECIAL))
                    {
                        result = IRC_DECODE_SUCCEED;
                    }
                    else if (same_polarity)
                    {
                        result = IRC_DECODE_SUCCEED;
                    }
                    else
                    {
                        result = IRC_DECODE_FAIL;
                    }
                }
                else
                {
                    result = IRC_DECODE_FAIL;
                }
            }

            if ((result == IRC_DECODE_SUCCEED) || (result == IRC_DECODE_DATA0)
            || (result == IRC_DECODE_DATA1))
            {
                ird->decode_step++;
                if ((result == IRC_DECODE_DATA0)
                || (result == IRC_DECODE_DATA1))
                {
                    //ird->key_code[0] <<= 1;
                    if (result == IRC_DECODE_DATA1)
                    {
                        ird->key_code[ird->key_bit_cnt / 32] |= ((UINT32) 0x1L)
                                << (ird->key_bit_cnt % 32);
                    }

                    ird->key_bit_cnt++;
                }

                if ((ird->attr->type == IR_TYPE_RC5)
                && INRANGE(accum_pulse_width, accum_pulse,
                ird->attr->pulse[next_step].tolerance))
                {
                    ird->first_half_got = 1;
                    ird->last_pulse_width = PULSE_VALUE(
                        ird->attr->pulse[next_step].fst_half);
                }

                /*deal with bit/byte reverse if need at last.*/
                if (ird->attr->normal[ird->decode_step] == IR_END)
                {
                    last_key = PAN_KEY_INVALID;
                    if (ird->attr->bit_msb_first)
                    {
                        reverse_code_bit(ird->key_code, ird->key_bit_cnt);
                    }

                    if (ird->attr->byte_msb_first)
                    {
                        reverse_code_byte(ird->key_code, ird->key_bit_cnt);
                    }

                    ird->decode_step = 0;
                    ird->key_bit_cnt = 0;
                    last_key = ird->last_key_code[0];
                    COPY_CODE((UINT8*) (ird->last_key_code),
                    (UINT8*) (ird->key_code));
                    CLEAR_CODE((UINT8*) (ird->key_code));

                    return ird->last_key_code[0];
                }
                return PAN_KEY_INVALID;
            }
            else if (result == IRC_DECODE_FAIL)
            {
                step_temp = ird->decode_step;
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE((UINT8*) (ird->key_code));
                if (ird->attr->normal[step_temp] != IR_LEADING)
                {
                    return PAN_KEY_INVALID;
                }
            }
            else
            {
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE((UINT8*) (ird->key_code));
                return PAN_KEY_INVALID;
            }
        }

        /*Process repeat key.*/
        if (ird->attr->repeat_enable && ird->attr->repeat)
        {
            enum ir_waveform step = ird->attr->repeat[ird->decode_step] + 1;
            if (INRANGE(accum_pulse_width,
            PULSE_VALUE(ird->attr->pulse[step].fst_half)
            + PULSE_VALUE(ird->attr->pulse[step].scd_half),
            ird->attr->pulse[step].tolerance))
            {
                result = IRC_DECODE_SUCCEED;
            }
            else
            {
                result = IRC_DECODE_FAIL;
            }

            if (result == IRC_DECODE_SUCCEED)
            {
                ird->decode_step++;
                if (ird->attr->repeat[ird->decode_step] == IR_END)
                {
                    ird->decode_step = 0;
                    ird->key_bit_cnt = 0;
                    return ird->last_key_code[0];
                }
                return PAN_KEY_INVALID;
            }
            else if (result == IRC_DECODE_FAIL)
            {
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE((UINT8*) (ird->key_code));
                return PAN_KEY_INVALID;
            }
            else
            {
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE((UINT8*) (ird->key_code));
                return PAN_KEY_INVALID;
            }
        }
        else
        {
            if (accum_pulse_width < ird->attr->pulse_max_width)
            {
                ird->last_pulse_width = accum_pulse_width;
            }
            else
            {
                ird->last_pulse_width = 0;
            }

            ird->decode_step = 0;
            ird->key_bit_cnt = 0;
            CLEAR_CODE((UINT8*) (ird->key_code));
            return PAN_KEY_INVALID;
        }
    }
}

/*****************************************************************************
 * UINT32 irc_pulse_to_code(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 *From now on, irc_decode support fellow protocols :
 *NEC   LAB  50560  KF Logic  SRC  NSE  RC5  RC6_mode1  ----2010.2.11 by ryan.chen
 *Arguments:
 *UINT32 pulse_width	: Input pulse width, in uS
 *UINT32 pulse_polarity	: Input pulse polarity,
 *					  '1' = high level, '0' = low level
 * Return Value:
 *  INT32				: Key code
 ****************************************************************************/
static UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity)
{
    UINT8 i = 0;
    UINT32 key_code = PAN_KEY_INVALID;

    for (i = 0; i < (sizeof(irc_decoders) / sizeof(struct irc_decoder*)); i++)
    {
        key_code = irc_decode(i, pulse_width, pulse_polarity);
        if ((key_code != PAN_KEY_INVALID) && (key_code != 0x0))
        {
            return key_code;
        }
    }

    return PAN_KEY_INVALID;
}

/*----------------------------------------------------------------------
 * Function_Name: IR_GenerateCode
 * Description: IR controler generate code
 * Arguments: None
 * Return Value: None
 *----------------------------------------------------------------------*/
static void IR_GenerateCode(void)
{
    UINT32 cur_code = 0;
    UINT32 pulse_width = 0;
    UINT1 pulse_polarity = 0;
    UINT8 rlc_data = 0;
    UINT8 i = 0;
    struct irc_decoder *pird = NULL;

    for (i = 0; i < (sizeof(irc_decoders) / sizeof(struct irc_decoder*)); i++)
    {
        pird = irc_decoders[i];
        pird->first_half_got = 0;
        pird->decode_step = 0;
        pird->key_bit_cnt = 0;
        CLEAR_CODE((UINT8*) (pird->key_code));
    }

    pulse_width = 0;
    while (bufferin != bufferout)
    {
        rlc_data = ir_buffer[bufferout];
        //printf("rptr=%bx, wptr=%bx, data=%bx\n", bufferout, bufferin, rlc_data);

        /*The MSB bit is the status bit, LSB 7 bits is time ticks,
         *If some one status is too long, mult-bytes maybe used.*/
        pulse_width += ((rlc_data & 0x7F) * CLK_CYC_US);/*Pulse width.*/
        pulse_polarity = ((rlc_data & 0x80) >> 7);
        bufferout++;
        if (bufferout == 0xFF)
        {
            bufferout = 0;
        }

        //bufferout = (++bufferout & (IR_BUF_LEN - 1));/*Next data.*/
        /*Long pulse.*/
        if ((!((rlc_data ^ ir_buffer[bufferout]) & 0x80))
        && (bufferout != bufferin))
        {
            continue;
        }

        cur_code = irc_pulse_to_code(pulse_width, pulse_polarity);
        if (ALI_IR_POWER_KEY == cur_code)
        {
            g_ir_flag = IR_KEY_MATCH_SUCCESS;
        }
        else
        {
            for (i = 0; i < 8; i++)
            {
#ifdef __SDCC__
                if (cur_code == g_ir_power_key[i])
#else
                if (cur_code == BigLittleSwap32(g_ir_power_key[i]))
#endif
                {
                    g_ir_flag = IR_KEY_MATCH_SUCCESS;
                }
            }
        }
        pulse_width = 0;
    }
}

/*----------------------------------------------------------------------
 * Function_Name: IR_ISR
 * Description: Receive the remote controller signal from Interrupt Requested.
 * Arguments: None
 * Return Value: None
 *----------------------------------------------------------------------*/
static void IR_ISR(void)
{
    volatile unsigned char status = 0;
    volatile unsigned char num = 0;
    volatile unsigned char num1 = 0;

    while (status = PMU_READ_BYTE(IR_REG_ISR) & 3)
    {
        PMU_WRITE_BYTE(IR_REG_ISR, status);
        switch (status)
        {
        case 0x02:/*If timeout, generate IR code in HSR.*/
        case 0x01:/*If FIFO trigger, copy data to buffer.*/
            do
            {
                num1 = num = (PMU_READ_BYTE(IR_REG_FIFOCTRL) & 0x7F);
                while (num > 0)
                {
                    /*Put RLC to buffer.*/
                    ir_buffer[bufferin++] = PMU_READ_BYTE(IR_REG_RLCBYTE);
                    if (bufferin == 0xFF)
                    {
                        bufferin = 0;
                    }
                    //bufferin &= (IR_BUF_LEN - 1);
                    num--;
                }
            }
            while (num1 > 0);

            break;

        default:
            break;
        }

        if (status == 0x02)
        {
            IR_GenerateCode();
            num = (PMU_READ_BYTE(IR_REG_FIFOCTRL) & 0x7F);
            while (num > 0)
            {
                PMU_READ_BYTE (IR_REG_RLCBYTE);
                num--;
            }
            bufferin = 0;
            bufferout = 0;
        }
    }
}

/*----------------------------------------------------------------------
 * Function_Name: ir_init
 * Description: IR service initial routine
 * Arguments:
 * Return Value:
 *----------------------------------------------------------------------*/
void ir_init(void)
{
    unsigned char reg = 0;

    /*Reset IR.*/
    reg = PMU_READ_BYTE(SYS_REG_RST);
    reg &= ~(1 << IR_BIT);
    PMU_WRITE_BYTE(SYS_REG_RST, reg);
    reg |= (1 << IR_BIT);
    PMU_WRITE_BYTE(SYS_REG_RST, reg);

    PMU_WRITE_BYTE(IR_REG_CFG, 0x00);
    /*Working clock expressions:
     *(IR_CLK / ((CLK_SEL+1)*4)) = 1 / CLK_CYC_US, IR_CLK = 1.5MHz
     *=> CLK_SEL = (IR_CLK*CLK_CYC_US)/4 - 1
     */
    //reg = 0x80 | ((15*CLK_CYC_US/40)-1);
    reg = ((15 * CLK_CYC_US / 40) - 1);

    //reg = ((15*CLK_CYC_US/40)-1);
    PMU_WRITE_BYTE(IR_REG_CFG, reg);

    /*FIFO threshold.*/
    PMU_WRITE_BYTE(IR_REG_FIFOCTRL, 0x8F); //16bytes //0xbf);

    /*Timeout threshold expressions:
     *((TIMETHR + 1) * 128 * CLK_CYC_US) = TIMEOUT_US
     *=> TIMETHR = (TIMEOUT_US/(128 * CLK_CYC_US)) - 1
     */
    reg = TIMEOUT_US / (CLK_CYC_US << 7) - 1;
#ifdef PMU_MCU_M3921
    PMU_WRITE_BYTE(IR_REG_TIMETHR, reg);
#else
    reg = TIMEOUT_US / (8 << 7) - 1;
    reg = 22;
    PMU_WRITE_BYTE(IR_REG_TIMETHR, reg);
#endif

    /*Noise pulse timeout expressions: Value = VALUE_NOISETHR / CLK_CYC_US.*/
    reg = NOISETHR_US / CLK_CYC_US;
#ifdef PMU_MCU_M3921
    PMU_WRITE_BYTE(IR_REG_NOISETHR, reg);
#else
    PMU_WRITE_BYTE(IR_REG_NOISETHR, 10);
#endif

    /*Ensure no pending interrupt.*/
    PMU_WRITE_BYTE(IR_REG_ISR, 0x03);

    /*Enable IR Interrupt.*/
    PMU_WRITE_BYTE(IR_REG_IER, 0x03);

    reg = PMU_READ_BYTE(SYS_REG_IPR);
    reg |= (1 << IR_BIT);
    PMU_WRITE_BYTE(SYS_REG_IPR, reg);

    reg = PMU_READ_BYTE(SYS_REG_IER);
    reg |= (1 << IR_BIT);

    PMU_WRITE_BYTE(SYS_REG_IER, reg);

    EX0 = 1;
}

static INT8 get_ir(void)
{
    INT8 ret = ERROR;

    if (IR_KEY_MATCH_SUCCESS == g_ir_flag)
    {
        ret = SUCCESS;
    }

    return ret;
}

void ir_process()
{
    if(SUCCESS == get_ir())
    {
#ifndef PMU_MCU_DEBUG
        show_panel (SHOW_BANK);
#else
        printf(" ir exit standby\n");
#endif
        mcu_disable_int();/*Disable all mcu interrupts.*/
        PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, EXIT_STANDBY_TYPE_IR);
        exit_standby_status();
        main_cpu_get_time();
        mcu_gpio_init();/*disable all gpio.*/
        stb_power_on();

        while (1)
            ;/*exit standby, then hold mcu.*/
    }
}

/*
 interrupt process
 1: time 0 interrupt
 2: ir interrupt
 */
#ifdef __SDCC__
void ir_interrupt_handler(void) __interrupt (0)

#else
void ir_interrupt_handler(void)
interrupt 0
#endif
{
    UINT8 reg = 0;

    EA = 0;
    reg = PMU_READ_BYTE(SYS_REG_ISR);

    if(reg & PMU_TIMER0_BIT)
    {
        PMU_WRITE_BYTE(PMU_TM0_CTRL, PMU_READ_BYTE(PMU_TM0_CTRL) | TIMER0_OVERFLOW_CLEAR);/*write 1 to clear Timer0 Overflow interrupt.*/
    }

    if(reg & (1<<IR_BIT))/*ir interrupt.*/
    {
        IR_ISR();
    }

    EA = 1;
}

