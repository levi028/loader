/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: irc_std.c
 *
 *  Description: This file implements Infra Receiver common
 *               driver function library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2009.5.22    Trueve Hu    0.1.000  Create file.
 ****************************************************************************/

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>
#include "irc.h"

//#define IRC_DEBUG

#ifdef IRC_DEBUG
    #define IRC_DEBUG_PRINTF    libc_printf
#else
    #define IRC_DEBUG_PRINTF(...)
#endif

//#define IRC_TRACE
#ifdef IRC_TRACE
    #define IRC_TRACE_TYPE        7
    #define IRC_TRACE_PRINTF    libc_printf
#else
    #define IRC_TRACE_PRINTF(...)
#endif


#define PULSE_LOW        (0 << 31)
#define PULSE_HIGH        (1 << 31)
#define PULSE_POL(x)    (x & (1 << 31))
#define PULSE_VALUE(x)    (x & (~(1 << 31)))

#ifdef RC5_EXTEND
#define RC5_X
#endif

#ifdef RC5_X
static UINT32 key_got_tick = 0;
#endif
static UINT32 key_g_tick = 0;


/*
static const char *irp_strs[] =
{
    "nec",
    "lab",
    "50560",
    "kf",
    "logic",
    "src",
    "nse",
    "rc5",
    "rc5_x",

};
*/
enum ir_waveform
{
    IR_LEADING    = 0,
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
    UINT32 type;
    UINT32 fst_half;
    UINT32 scd_half;
    UINT32 tolerance;
};

struct ir_attr
{
    UINT32 type;
    UINT32 bit_msb_first    : 1;
    UINT32 byte_msb_first    : 1;
    UINT32 pulse_invert        : 1;
    UINT32 repeat_enable    : 1;
    UINT32 ignore_lastpulse    : 1;
    UINT32 pulse_prec;
    UINT32 pulse_max_width;
    struct irc_pulse pulse[7];
    enum ir_waveform *normal;
    enum ir_waveform *repeat;
};

struct irc_decoder
{
    struct ir_attr *attr;
    UINT16 first_half_got;
    UINT16 decode_step;
    UINT32 key_bit_cnt;
    UINT32 last_pulse_width;
    UINT32 key_code[4];
    UINT32 last_key_code[4];
};


static enum ir_waveform ir_nec_normal[] =
{
    IR_LEADING,                            // start bit
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits command
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,   //8 bits address
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~command
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~address
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_END,
};

static enum ir_waveform ir_nec_repeat[] =
{
    IR_REPEAT_LEADING,
    IR_END,
};

static struct ir_attr ir_attr_nec =
{
    IR_TYPE_NEC,
    1,
    0,
    0,
    1,
    0,
    280,
    15000,
    {
        {IR_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 4500, 500}, // start pulse
        {IR_NULL, 0, 0, 0},
        {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},  //logic '0'
        {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1680, 300},  // logic '1'
        {IR_REPEAT_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 2250, 500},
        {IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
    {IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
    },
    ir_nec_normal,
    ir_nec_repeat,
};

static enum ir_waveform ir_lab[] =
{
    IR_LEADING,
    IR_SPECIAL,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA,
    IR_END,
};

static struct ir_attr ir_attr_lab =
{
    IR_TYPE_LAB,
    1,
    0,
    0,
    0,
    0,
    140,
    15000,
    {
    {IR_LEADING, PULSE_LOW | 280, PULSE_HIGH | 7300, 140},
    {IR_SPECIAL, PULSE_LOW | 280, PULSE_HIGH | 6150, 1500},
    {IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 4780, 500},
    {IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 7300, 500},
    {IR_NULL, 0, 0, 0},
    {IR_NULL, 0, 0, 0},
    {IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
    },
    ir_lab,
    NULL,
};

static enum ir_waveform ir_50560[] =
{
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_SPECIAL,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_SPECIAL,
    IR_END,
};

static struct ir_attr ir_attr_50560 =
{
    IR_TYPE_50560,
    1,
    0,
    0,
    1,
    0,
    260,
    15000,
    {
    {IR_LEADING, PULSE_LOW | 8400, PULSE_HIGH | 4200, 500},
    {IR_SPECIAL, PULSE_LOW | 520, PULSE_HIGH | 4200, 500},
    {IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 1050, 500},
    {IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 2100, 500},
    {IR_NULL, 0, 0, 0},
    {IR_NULL, 0, 0, 0},
    {IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
    },
    ir_50560,
    NULL,
};

static enum ir_waveform ir_kf[] =
{
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_END,
};

static struct ir_attr ir_attr_kf =
{
    IR_TYPE_KF,
    0,
    1,
    0,
    0,
    0,
    190,
    20000,
    {
    {IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 500},
    {IR_NULL, 0, 0, 0},
    {IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 380, 150},
    {IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 1350, 300},
    {IR_NULL, 0, 0, 0},
    {IR_NULL, 0, 0, 0},
    {IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000},
    },
    ir_kf,
    NULL,
};


static enum ir_waveform ir_logic[] =
{
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_END,
};

static struct ir_attr ir_attr_logic =
{
    IR_TYPE_LOGIC,
    0,
    0,
    0,
    0,
    0,
    125,
    20000,
    {
    {IR_LEADING, PULSE_LOW | 5000, PULSE_HIGH | 5000, 500},
    {IR_NULL, 0, 0, 0},
    {IR_DATA, PULSE_LOW | 250, PULSE_HIGH | 500, 100},
    {IR_DATA, PULSE_LOW | 500, PULSE_HIGH | 1000, 200},
    {IR_NULL, 0, 0, 0},
    {IR_NULL, 0, 0, 0},
    {IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000},
    },
    ir_logic,
    NULL,
};

static enum ir_waveform ir_src_normal[] =
{
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_SPECIAL,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_END,
};

static struct ir_attr ir_attr_src =
{
    IR_TYPE_SRC,
    1,
    0,
    0,
    1,
    0,
    280,
    60000,
    {
    {IR_LEADING, PULSE_LOW | 6000, PULSE_HIGH | 2000, 400},
    {IR_SPECIAL, PULSE_LOW | 3640, PULSE_HIGH | 3640, 400},
    {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
    {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
    {IR_NULL, 0, 0, 0},
    {IR_NULL, 0, 0, 0},
    {IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
    },
    ir_src_normal,
    NULL,
};

static enum ir_waveform ir_nse_normal[] =
{
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_LEADING,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_END,
};

static enum ir_waveform ir_nse_repeat[] =
{
    IR_REPEAT_LEADING,
    IR_END,
};

static struct ir_attr ir_attr_nse =
{
    IR_TYPE_NSE,
    1,
    0,
    0,
    1,
    0,
    280,
    60000,
    {
    {IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 400},
    {IR_NULL, 0, 0, 0},
    {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
    {IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
    {IR_REPEAT_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 3640, 500},
    {IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
    {IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
    },
    ir_nse_normal,
    ir_nse_repeat,
};

static enum ir_waveform ir_rc5[] =
{
    IR_LEADING, IR_LEADING,   //S1 & S2 bit
    IR_DATA,               //toggle bit
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 5 bits addr & 6 bits command
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA, IR_DATA,
    IR_END,             // ???
};

static struct ir_attr ir_attr_rc5 =
{
    IR_TYPE_RC5,  // type name
    1,            //bits MSB
    0,
    0,
    0,
    1,            // repeat enable
    200,            //pulse prec
    15000,        // pulse max width
    {
        {IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200}, // leading logic
        {IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},  // toggle logic
        {IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},       // data logic '0'
        {IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},    // data logic '1'
        {IR_NULL, 0, 0, 0},
        {IR_NULL, 0, 0, 0},
        {IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000},
    },
    ir_rc5,
    NULL,
};
static enum ir_waveform ir_rc5_x[] =
{
    IR_LEADING,
    IR_DATA,   //S1 & S2 bit
    IR_DATA,
    IR_DATA,
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 5 bits addr & 6 bits command
    IR_DATA, IR_DATA, IR_DATA, IR_DATA,
    IR_DATA, IR_DATA,
};

static struct ir_attr ir_attr_rc5_x =
{
    IR_TYPE_RC5,  // type name
    1,            //bits MSB
    0,
    0,
    0,
    1,            // repeat enable
    200,            //pulse prec
    15000,        // pulse max width
    {
        {IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200}, // leading logic
        {IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},  // toggle logic
        {IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},       // data logic '0'
        {IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},    // data logic '1'
        {IR_NULL, 0, 0, 0},
        {IR_NULL, 0, 0, 0},
        {IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000},
    },
    ir_rc5_x,
    NULL,
};

static struct irc_decoder irc_decoders[] =
{
    {
        .attr = &ir_attr_nec,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_lab,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_50560,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_kf,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .key_code = {0,},
        .last_pulse_width = 0,
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_logic,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_src,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_nse,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
    {
        .attr = &ir_attr_rc5,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },
        {
        .attr = &ir_attr_rc5_x,
        .first_half_got = 0,
        .decode_step = 0,
        .key_bit_cnt = 0,
        .last_pulse_width = 0,
        .key_code = {0,},
        .last_key_code = {0,},
    },


};


//extern UINT16 key_cnt;


#define INRANGE(x, value, tol)        (((x) > ((value)-(tol)) )&& ((x) < ((value)+(tol))))
#define IRP_CNT                    (sizeof(irc_decoders) / sizeof(struct irc_decoder))

#define COPY_CODE(dst, src)        \
    do {int i = 0; \
        for(i=0;i<16;i++) *((UINT8 *)dst+i) = *((UINT8 *)src+i);\
       } while(0)
#define CLEAR_CODE(p)            \
    do {int i = 0;\
        for(i=0;i<16;i++) *((UINT8 *)p+i) = 0;\
        } while(0)

static void reverse_code_bit(UINT32 *code, UINT32 bit_cnt)
{
    UINT32 i = 0;
    UINT32 src_code[4] = {0,};
    UINT32 dst_code[4] = {0,};

    COPY_CODE(src_code, code);

    for (i=0; i<bit_cnt; i++)
    {
        if (src_code[i / 32] & (1 << (i % 32)))
            dst_code[(bit_cnt - 1 - i) / 32] |= 1 << ((bit_cnt - 1 - i) % 32);
    }

    COPY_CODE(code, dst_code);
}

static void reverse_code_byte(UINT32 *code, UINT32 bit_cnt)
{
    UINT32 i = 0;

    UINT8 src_code[16] = {0,};
    UINT8 dst_code[16] = {0,};
    UINT32 byte_cnt = (bit_cnt  + 7) / 8;

    COPY_CODE(src_code, code);

    for (i=0; i<byte_cnt; i++)
    {
        dst_code[i] = src_code[byte_cnt - i - 1];
    }

    COPY_CODE(code, dst_code);
}

enum irc_decode_ret
{
    IRC_DECODE_SUCCEED,
    IRC_DECODE_DATA0,
    IRC_DECODE_DATA1,
    IRC_DECODE_FAIL,
    IRC_DECODE_STOP,
};
 #ifdef RC5_X
 static UINT32 rc5_x_decode(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity)
{
    UINT32 accum_pulse_width = 0;
    enum irc_decode_ret result = IRC_DECODE_FAIL;
    static UINT32 rc5_tick =0 ;  // only for RC5
    static UINT32 last_key = PAN_KEY_INVALID;
    UINT32 tick_200 = 200;
    UINT8 rc5_1_tick = 1;
    UINT32 tmp_pulse_width = pulse_width;

    /* Ignore burr pulses */
    if (pulse_width < ird->attr->pulse_prec)
    {
        ird->first_half_got = 0;
        IRC_DEBUG_PRINTF("Burr pulses!\n");
        return PAN_KEY_INVALID;
    }

  //  libc_printf("pulse_width %d[%d]\n", pulse_width , pulse_polarity);
    if (0 == ird->first_half_got)        /* first_half_got is flag  for dule-pulse senser */
    {
        if (0 == ird->decode_step) /*just check leading pulse*/
        {
            UINT32 half_pol;
            UINT32 half_width;
            if (PULSE_POL(ird->attr->pulse[0].fst_half)) /* if leading start with high level */
            {
                half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
                half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
            }
            else
            {
                half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
                half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
            }

            /* Check leading code first half pulse polarity */
            if (half_pol ^ (pulse_polarity << 31))
            {
                #ifdef IRC_TRACE
                if (IRC_TRACE_TYPE == ird->attr->type)
                #endif
                IRC_DEBUG_PRINTF("Error leading half pulse polarity!\n");
                return PAN_KEY_INVALID;
            }
            /* Check leading code first half pulse width */
            if (!(INRANGE(pulse_width, 2*half_width, 2*ird->attr->pulse[0].tolerance) || \
                 INRANGE(pulse_width,  half_width,  ird->attr->pulse[0].tolerance) ))
            {
                #ifdef IRC_TRACE
                if (IRC_TRACE_TYPE == ird->attr->type)
                #endif
                IRC_DEBUG_PRINTF("Leading half pulse not in range!(%d)\n", pulse_width);
                return PAN_KEY_INVALID;
            }
            if( INRANGE(pulse_width,  2*half_width,  2*ird->attr->pulse[0].tolerance) )
            {
                rc5_tick =1 ;
            }

             if( pulse_width > 4 * half_width)
             {
                pulse_width = half_width ;
             }
			 ird->key_bit_cnt = 0;
        }
        ird->first_half_got = 1; //

       if ((0 == ird->decode_step) && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
        {
            ird->first_half_got = 0;
            ird->last_pulse_width = 0;
            ird->decode_step++;
			if(INRANGE(pulse_width, 2*PULSE_VALUE(ird->attr->pulse[0].fst_half),2*ird->attr->pulse[0].tolerance ))
           	{
				ird->last_pulse_width = pulse_width - PULSE_VALUE(ird->attr->pulse[0].fst_half) ;
            	ird->first_half_got = 1;
        	}
        }

        if(INRANGE(pulse_width, PULSE_VALUE(ird->attr->pulse[0].fst_half),ird->attr->pulse[0].tolerance )) // 1T
            ird->last_pulse_width = pulse_width;
        else if(INRANGE(pulse_width, 2*PULSE_VALUE(ird->attr->pulse[0].fst_half),2*ird->attr->pulse[0].tolerance ))
            ird->last_pulse_width = pulse_width - PULSE_VALUE(ird->attr->pulse[0].fst_half) ;

        return PAN_KEY_INVALID;
    }
    else
    {
        if(INRANGE(pulse_width, PULSE_VALUE(ird->attr->pulse[0].scd_half),ird->attr->pulse[0].tolerance )) // 1T
        {
            accum_pulse_width = ird->last_pulse_width + pulse_width;
            ird->last_pulse_width = 0;
            ird->first_half_got = 0;

        }
        else if(INRANGE(pulse_width, 2*PULSE_VALUE(ird->attr->pulse[0].scd_half),2*ird->attr->pulse[0].tolerance ))
        {
            pulse_width -= PULSE_VALUE(ird->attr->pulse[0].fst_half) ;
            accum_pulse_width = ird->last_pulse_width + pulse_width;

            ird->last_pulse_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
            ird->first_half_got = 1;
        }
         else if(pulse_width> 4*PULSE_VALUE(ird->attr->pulse[0].scd_half)) // >4T
         {
               pulse_width = PULSE_VALUE(ird->attr->pulse[0].scd_half) ;
               accum_pulse_width = ird->last_pulse_width + pulse_width;

               ird->last_pulse_width = 0 ;
               ird->first_half_got = 0;
			if (ird->key_bit_cnt != 12)
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
         }

        /* Process normal key */
        if (ird->attr->normal)
        {
            enum ir_waveform step = ird->attr->normal[ird->decode_step];
            //enum ir_waveform next_step = ird->attr->normal[ird->decode_step + 1];
            if (step > IR_DATA)
                step++;

            UINT32 accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half);
            UINT32 tolerance_pulse = ird->attr->pulse[step].tolerance;
            if(INRANGE(tmp_pulse_width, 2*PULSE_VALUE(ird->attr->pulse[0].scd_half),2*ird->attr->pulse[0].tolerance )){
                tolerance_pulse = 2*ird->attr->pulse[step].tolerance;
            }
            UINT32 same_polarity = !(pulse_polarity);

            if (1)
            {
                if ((INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse)) && (same_polarity ? 1 : 0))
                {
                    result = 1;
                    #ifdef IRC_TRACE
                    if (IRC_TRACE_TYPE == ird->attr->type)
                    #endif
                    IRC_TRACE_PRINTF("[0]");
                }
                else if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
                {
                    result = 0;
                    #ifdef IRC_TRACE
                    if (IRC_TRACE_TYPE == ird->attr->type)
                    #endif
                    IRC_TRACE_PRINTF("[1]");
                }
                else
                    result = IRC_DECODE_FAIL;
            }
            else
            {
                if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
                {
                    if (((IR_TYPE_RC5 == ird->attr->type) ) && IR_SPECIAL == step)
                        result = IRC_DECODE_SUCCEED;
                    else if (same_polarity)
                        result = IRC_DECODE_SUCCEED;
                    else
                        result = IRC_DECODE_FAIL;
                }
                else
                    result = IRC_DECODE_FAIL;
            }

            if (IRC_DECODE_SUCCEED == result || \
                IRC_DECODE_DATA0 == result || \
                IRC_DECODE_DATA1 == result)
            {
                ird->decode_step++;
                if (0 == result || \
                    1 == result)
                {
                    if (1 == result)
                        /*decode ONE BIT success*/
                        ird->key_code[ird->key_bit_cnt / 32] |= 1 << (ird->key_bit_cnt % 32);
                    ird->key_bit_cnt++;

                }

                /* deal with bit/byte reverse if need at last */
                if(13 == ird->key_bit_cnt)
                {

                    if (ird->attr->bit_msb_first)
                        reverse_code_bit(ird->key_code, ird->key_bit_cnt);
                    if (ird->attr->byte_msb_first)
                        reverse_code_byte(ird->key_code, ird->key_bit_cnt);

                    ird->decode_step = 0;
                    ird->key_bit_cnt = 0;
                    ird->last_pulse_width =0 ;
                    ird->first_half_got = 0 ;

                    COPY_CODE(ird->last_key_code, ird->key_code);
                    if(rc5_1_tick == rc5_tick)
                    {
                     // ird->last_key_code[0] |= 0x1<< 6 ; //s2
                     ird->last_key_code[0] +=64 ; //s2
                      rc5_tick =0 ;
                    }
                    if(last_key == ird->last_key_code[0])
                    {
                         if( osal_get_tick() - key_got_tick < tick_200)
                         {
                            CLEAR_CODE(ird->key_code);
                            return  PAN_KEY_INVALID ;
                         }
                    }else{
                     	key_cnt = 0 ;
					}
                    last_key = ird->last_key_code[0] ;
                    ird->last_key_code[0] &= ~(0x1<<11)  ; // toggle bit
                    CLEAR_CODE(ird->key_code);
                    IRC_TRACE_PRINTF("^_^ %x!\n", ird->last_key_code[0]);
                    key_got_tick = osal_get_tick();
                    return ird->last_key_code[0];
                }
                return PAN_KEY_INVALID;
            }
            else if (IRC_DECODE_FAIL == result)
            {
                #ifdef IRC_TRACE
                if (IRC_TRACE_TYPE == ird->attr->type)
                #endif
                IRC_DEBUG_PRINTF("decode failed @ step %d!\n", ird->decode_step);
                UINT16 step = ird->decode_step;
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE(ird->key_code);
                if (ird->attr->normal[step] != IR_LEADING)
                    return PAN_KEY_INVALID;
            }
            /* dead_code
            else
            {
                ird->decode_step = 0;
                ird->key_bit_cnt = 0;
                CLEAR_CODE(ird->key_code);
                return PAN_KEY_INVALID;
            }
            */
        }


    }
    return PAN_KEY_INVALID;
}
#endif

INT32 check_leading_pulse(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity)
{
    UINT32 half_pol = 0;
    UINT32 half_width = 0;

    if (0 == ird->decode_step) /*just check leading pulse*/
    {
        half_pol = 0;
        half_width = 0;
        if (PULSE_POL(ird->attr->pulse[0].fst_half)) /* if leading start with high level */
        {
            half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
            half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
        }
        else
        {
            half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
            half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
        }

        /* Check leading code first half pulse polarity */
        if (half_pol ^ (pulse_polarity << 31))
        {
            #ifdef IRC_TRACE
            if (IRC_TRACE_TYPE == ird->attr->type)
            #endif
            IRC_DEBUG_PRINTF("Error leading half pulse polarity!\n");
            return PAN_KEY_INVALID;
        }
        /* Check leading code first half pulse width */
        if (!INRANGE(pulse_width, half_width, ird->attr->pulse[0].tolerance))
        {
            #ifdef IRC_TRACE
            if (IRC_TRACE_TYPE == ird->attr->type)
            #endif
            IRC_DEBUG_PRINTF("Leading half pulse not in range!(%d)\n", pulse_width);
            return PAN_KEY_INVALID;
        }

    }
    return RET_SUCCESS;
}

void reverse_keycode(struct irc_decoder *ird)
{
    struct pan_ir_endian *ir_endian = (struct pan_ir_endian *)pan_ir_get_endian();

    if ((ird->attr->type == ir_endian->protocol) && (1 == ir_endian->enable))
    {
        if (1 != ir_endian->bit_msb_first)
        {
            reverse_code_bit(ird->key_code, ird->key_bit_cnt);
        }
        if (1 == ir_endian->byte_msb_first)
        {
            reverse_code_byte(ird->key_code, ird->key_bit_cnt);
        }
    }
    else
    {
        if (ird->attr->bit_msb_first)
        {
            reverse_code_bit(ird->key_code, ird->key_bit_cnt);
        }
        if (ird->attr->byte_msb_first)
        {
            reverse_code_byte(ird->key_code, ird->key_bit_cnt);
        }
    }
}

INT32 repeat_proc(struct irc_decoder *ird, UINT32 accum_pulse_width)
{

    enum ir_waveform step = ird->attr->repeat[ird->decode_step] + 1;
    enum irc_decode_ret result = IRC_DECODE_FAIL;

    if (INRANGE(accum_pulse_width, \
           PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half), \
           ird->attr->pulse[step].tolerance))
    {
        result = IRC_DECODE_SUCCEED;
    }
    else
    {
        result = IRC_DECODE_FAIL;
    }

    if (IRC_DECODE_SUCCEED == result)
    {
        ird->decode_step++;
        if (IR_END == ird->attr->repeat[ird->decode_step])
        {
            ird->decode_step = 0;
            ird->key_bit_cnt = 0;

            if (osal_get_tick() > (key_g_tick + 200))
            {
                ird->last_key_code[0] = PAN_KEY_INVALID;
                return PAN_KEY_INVALID;
            }
            key_g_tick = osal_get_tick();

            return ird->last_key_code[0];
        }
        return PAN_KEY_INVALID;
    }
    else if (IRC_DECODE_FAIL == result)
    {
        ird->decode_step = 0;
        ird->key_bit_cnt = 0;
        CLEAR_CODE(ird->key_code);
        return PAN_KEY_INVALID;
    }
    else
    {
        ird->decode_step = 0;
        ird->key_bit_cnt = 0;
        CLEAR_CODE(ird->key_code);
        return PAN_KEY_INVALID;
    }

}

INT32 normal_key_proc(struct irc_decoder *ird, UINT32 pulse_polarity, UINT32 accum_pulse_width)
{

    enum ir_waveform step = ird->attr->normal[ird->decode_step];
    enum ir_waveform next_step = ird->attr->normal[ird->decode_step + 1];
    enum irc_decode_ret result = IRC_DECODE_FAIL;

    if (step > IR_DATA)
    {
        step++;
    }

    UINT32 accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half);
    UINT32 tolerance_pulse = ird->attr->pulse[step].tolerance;
    UINT32 same_polarity = !(PULSE_POL(ird->attr->pulse[step].scd_half) ^ (pulse_polarity << 31));

    if (IR_TYPE_RC5 == ird->attr->type)
    {
        accum_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
        tolerance_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
    }

    if (IR_DATA == ird->attr->normal[ird->decode_step])
    {
        UINT32 accum_pulse_1 = PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half) \
                             + PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].scd_half);
        UINT32 tolerance_pulse_1 = ird->attr->pulse[IR_DATA + 1].tolerance;
        UINT32 same_polarity_1 = !(PULSE_POL(ird->attr->pulse[IR_DATA + 1].scd_half) ^ (pulse_polarity << 31));

        if (IR_TYPE_RC5 == ird->attr->type)
        {
            accum_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
            tolerance_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
        }
        if ((((ird->attr->ignore_lastpulse) &&( IR_END == ird->attr->normal[ird->decode_step + 1]) )|| \
            INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse)) && (same_polarity ? 1 : 0))
        {
            result = IRC_DECODE_DATA0;
            #ifdef IRC_TRACE
            if (IRC_TRACE_TYPE == ird->attr->type)
            #endif
            IRC_TRACE_PRINTF("[0]");
        }
        else if (((ird->attr->ignore_lastpulse &&( IR_END == ird->attr->normal[ird->decode_step + 1])) || \
            INRANGE(accum_pulse_width, accum_pulse_1, tolerance_pulse_1)) && (same_polarity_1 ? 1 : 0))
        {
            result = IRC_DECODE_DATA1;
            #ifdef IRC_TRACE
            if (IRC_TRACE_TYPE == ird->attr->type)
            #endif
            IRC_TRACE_PRINTF("[1]");
        }
        else
            result = IRC_DECODE_FAIL;
    }
    else
    {
        if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
        {
            if ((IR_TYPE_RC5 == ird->attr->type) && (IR_SPECIAL == step))
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

    if ((IRC_DECODE_SUCCEED == result) || \
        (IRC_DECODE_DATA0 == result) || \
        (IRC_DECODE_DATA1 == result))
    {
        ird->decode_step++;
        if ((IRC_DECODE_DATA0 == result) || \
            (IRC_DECODE_DATA1 == result))
        {
            if (IRC_DECODE_DATA1 == result)
            {
                ird->key_code[ird->key_bit_cnt / 32] |= 1 << (ird->key_bit_cnt % 32);
            }
            ird->key_bit_cnt++;
        }

        if ((IR_TYPE_RC5 == ird->attr->type) \
                            && INRANGE(accum_pulse_width, accum_pulse, ird->attr->pulse[next_step].tolerance))
        {
            ird->first_half_got = 1;
            ird->last_pulse_width = PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
        }

        /* deal with bit/byte reverse if need at last */
        if (IR_END == ird->attr->normal[ird->decode_step])
        {
            UINT32 last_key = PAN_KEY_INVALID;
            reverse_keycode(ird);

            ird->decode_step = 0;
            ird->key_bit_cnt = 0;
            last_key = ird->last_key_code[0];
            COPY_CODE(ird->last_key_code, ird->key_code);
            CLEAR_CODE(ird->key_code);
            IRC_TRACE_PRINTF("^_^ %08x!\n", ird->last_key_code[0]);
            if ((last_key != ird->last_key_code[0]) ||( ird->attr->repeat != NULL))
            {
                key_cnt = 0;
            }
            key_g_tick = osal_get_tick();
            return ird->last_key_code[0];
        }
        return PAN_KEY_INVALID;
    }
    else if (IRC_DECODE_FAIL == result)
    {
        #ifdef IRC_TRACE
        if (ird->attr->type == IRC_TRACE_TYPE)
        #endif
        IRC_DEBUG_PRINTF("decode failed @ step %d!\n", ird->decode_step);

        UINT16 step = ird->decode_step;

        ird->decode_step = 0;
        ird->key_bit_cnt = 0;
        CLEAR_CODE(ird->key_code);
        if (ird->attr->normal[step] != IR_LEADING)
        {
            return PAN_KEY_INVALID;
        }
    }
    /* dead-code
    else
    {
        ird->decode_step = 0;
        ird->key_bit_cnt = 0;
        CLEAR_CODE(ird->key_code);
        return PAN_KEY_INVALID;
    }
    */
    return RET_SUCCESS;

}

/*****************************************************************************
 * UINT32 irc_decode(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 * Arguments:
 *      struct irc_decoder *ird    : IR decode structure
 *    UINT32 pulse_width        : Input pulse width, in uS
 *    UINT32 pulse_polarity        : Input pulse polarity,
 *                                    '1' = high level, '0' = low level
 *
 * Return Value:
 *    INT32                        : Key code
 ****************************************************************************/
 static UINT32 irc_decode(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity)
{
    UINT32 accum_pulse_width = 0;
    //enum irc_decode_ret result = IRC_DECODE_FAIL;

    int ret = 0;

    /* Ignore burr pulses */
    if (pulse_width < ird->attr->pulse_prec)
    {
        ird->first_half_got = 0;
        IRC_DEBUG_PRINTF("Burr pulses!\n");
        return PAN_KEY_INVALID;
    }

    if (0 == ird->first_half_got)        /* first_half_got is flag  for dule-pulse senser */
    {
        ret = check_leading_pulse(ird, pulse_width, pulse_polarity);
        if(ret != RET_SUCCESS)
        {
            return ret;
        }

        ird->first_half_got = 1;
        ird->last_pulse_width = pulse_width;

        if ((0 == ird->decode_step) && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
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

        /* Process normal key */
        if (ird->attr->normal)
        {
            ret = normal_key_proc(ird, pulse_polarity, accum_pulse_width);
            if(ret != RET_SUCCESS)
            {
                return ret;
            }
        }
        /* Process repeat key */
        if (ird->attr->repeat_enable && ird->attr->repeat)
        {
            ret = repeat_proc(ird, accum_pulse_width);
            return ret;
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
            CLEAR_CODE(ird->key_code);
            return PAN_KEY_INVALID;
        }
    }
}

/*****************************************************************************
 * UINT32 irc_pulse_to_code(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 *               From now on, irc_decode support fellow protocols :
 *                NEC   LAB  50560  KF Logic  SRC  NSE  RC5  RC6_mode1  ----2010.2.11 by ryan.chen
 * Arguments:
 *    UINT32 pulse_width    : Input pulse width, in uS
 *    UINT32 pulse_polarity    : Input pulse polarity,
 *                                '1' = high level, '0' = low level
 *
 * Return Value:
 *    INT32                : Key code
 ****************************************************************************/
UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity)
{
    UINT32 i = 0;
    UINT32 key_code = PAN_KEY_INVALID;

    //libc_printf("%d\t(%d)\n", pulse_width, pulse_polarity);
   #ifdef RC5_X
        key_code =rc5_x_decode(&irc_decoders[8], pulse_width, pulse_polarity);
        if (key_code != PAN_KEY_INVALID && key_code != 0)
        {
            return key_code;
        }
   #else
    for (i = 0; i< IRP_CNT; i++)
    {
        key_code = irc_decode(&irc_decoders[i], pulse_width, pulse_polarity);
        if ((key_code != PAN_KEY_INVALID) && (key_code != 0))
        {
            return key_code;
        }
    }
   #endif

    return PAN_KEY_INVALID;

}


