/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: irc_nec_std.c
 *
 *  Description: This file implements the NEC mode Infra Receiver common
 *               driver function library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.7.5    Justin Wu   0.1.000  Create
 *  2.  2003.7.18   Justin Wu   0.2.000  Upgrade statement machime
 *  3.  2004.9.21   Justin Wu   0.3.000  Enhance compatibility
 ****************************************************************************/

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>

//#define NEC_DEBUG

#ifdef NEC_DEBUG
    #define NEC_DEBUG_PRINTF    libc_printf
#else
    #define NEC_DEBUG_PRINTF(...)
#endif

#define ENDIAN_REVERSE_BYTE(X)  ((((UINT32)(X) & 0xff000000) >> 24) | \
                                (((UINT32)(X) & 0x00ff0000) >> 8) | \
                                (((UINT32)(X) & 0x0000ff00) << 8) | \
                                (((UINT32)(X) & 0x000000ff) << 24))

/* NEC IRC code standard params */
#if (SYS_CHIP_MODULE == ALI_S3602)
#define IRC_NEC_PULSE_UNIT        530                            /* 560uS */
#else
#define IRC_NEC_PULSE_UNIT        560                            /* 560uS */
#endif
#define IRC_NEC_LEADING            (16 * IRC_NEC_PULSE_UNIT)    /* 8.96mS */
#define IRC_NEC_LEADING_REPT    (20 * IRC_NEC_PULSE_UNIT)    /* 11.2mS */
#define IRC_NEC_LEADING_FUNC    (24 * IRC_NEC_PULSE_UNIT)    /* 13.44mS */

extern unsigned short key_cnt;

static UINT32 irc_nec_state = 0;        /* IRC state */

static UINT32 irc_nec_precision = 0;    /* Pulse uint precision, should < IRC_NEC_PULSE_UNIT/2 */
static UINT32 irc_nec_prec_lead = 0;    /* Leading precision,should < IRC_NEC_PULSE_UNIT*2 */
static UINT32 irc_nec_repeat_to = 0;
static UINT32 code = 0;
static UINT32 last_code = PAN_KEY_INVALID;
static UINT32 accum_width = 0;
static UINT32 first_half = 1;        /* For dule-pulse senser */
static UINT32 last_width = 0;



static void reverse_code_bit(UINT32 *code, UINT32 bit_cnt)
{
    UINT32 i = 0;
    UINT32 dst_code=0;

    for (i = 1; i < bit_cnt; i++)
    {
        dst_code |= (*code&0x1);
        dst_code <<= 1;
        *code >>= 1;
    }
    *code = dst_code;
}

/*****************************************************************************
 * void irc_NEC_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
 * Description: NEC mode IR controler: mode setup
 *
 * Arguments:
 *    UINT32 state        : Current IRC state, init is 0
 *    UINT32 precision    : Code pulse precision
 *    UINT32 repeat_to    : Repeat code timeout limit
 *
 * Return Value:
 ****************************************************************************/
void irc_nec_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
{
    irc_nec_state = state;
    irc_nec_precision = precision;
    irc_nec_prec_lead = (precision << 2);
    irc_nec_repeat_to = repeat_to;
}

/*****************************************************************************
 * UINT32 irc_NEC_pulse_to_code(UINT32 pulse_width)
 * Description: NEC mode IR controler: translate pulse width to IR code
 *              This function support both of one and two trigle per bit mode.
 *
 * Arguments:
 *    UINT32 pulse_width    : Input pulse width, in uS
 *
 * Return Value:
 *    INT32                    : Key code
 ****************************************************************************/
UINT32 irc_nec_pulse_to_code(UINT32 pulse_width)
{
    UINT8 got_full_status = 32;

    last_width += pulse_width;
    /* Ignore burr pluses */
    if (pulse_width < irc_nec_precision)
    {
        return PAN_KEY_INVALID;
    }

//    PRINTF("state: %d, totol width: %d, first_half: %d\n",
//      irc_NEC_state, last_width, first_half);

    /* Pre process: Combine high & low together */
    if (0 == irc_nec_state)
    {
        /* Is leading pulse, can goto the next state */
        if ((pulse_width > IRC_NEC_LEADING - irc_nec_prec_lead )&&
            (pulse_width < IRC_NEC_LEADING + irc_nec_prec_lead))
        {
            first_half = 0;
            return PAN_KEY_INVALID;
        }
    }
    else
    {
        if ((1 == first_half )&&
            (pulse_width < (IRC_NEC_PULSE_UNIT << 1) - irc_nec_precision))
        {
            first_half = 0;
            return PAN_KEY_INVALID;
        }
    }

    pulse_width = last_width;
    last_width = 0;

    first_half = 1;
    accum_width += pulse_width;

    /* Pulse process: transmit cycle pulse to code.
     * All of waveform send out by remote is inverted at receiver end.
     */
    if (0 == irc_nec_state)
    {
        /* For 16 + 8ms leading code: function code, can goto next state */
        if ((pulse_width > IRC_NEC_LEADING_FUNC - irc_nec_prec_lead )&&
            (pulse_width < IRC_NEC_LEADING_FUNC + irc_nec_prec_lead))
        {
            /* New key pressed: Leading pulse finished */
            accum_width = pulse_width;
            last_code = PAN_KEY_INVALID;
            key_cnt = 0;
        }
        /* For 16 + 4ms leading code: repeat code */
        else if ((pulse_width > IRC_NEC_LEADING_REPT - irc_nec_prec_lead) &&
                 (pulse_width < IRC_NEC_LEADING_REPT + irc_nec_prec_lead))
        {
            /* Reset state */
            irc_nec_state = 0;
            if (accum_width < irc_nec_repeat_to)
            {
                /* Reset key-repeat time count, wait new repeat */
                accum_width = 0;
                if (last_code != PAN_KEY_INVALID)
                {
                    return last_code;
                }
            }
            return PAN_KEY_INVALID;
        }
        else
        {
            /* Unknow Leading pulse, reset state, support continue repeat */
            if (pulse_width < IRC_NEC_LEADING_REPT)
            {
                irc_nec_state = 0;
            }
            return PAN_KEY_INVALID;
        }
    }
    else
    {
        if ((pulse_width > IRC_NEC_PULSE_UNIT) &&
            (pulse_width < IRC_NEC_PULSE_UNIT * 5))
        {
            code = (code << 1) + (pulse_width > IRC_NEC_PULSE_UNIT * 3);
            /* Generate code */
            if (got_full_status == irc_nec_state)
            {
                  struct pan_ir_endian *ir_endian = (struct pan_ir_endian *)pan_ir_get_endian();

                if (1 == ir_endian->enable)
                {
                    NEC_DEBUG_PRINTF("before reverse: code = 0x%x\n",code);
                    if (1 == ir_endian->bit_msb_first)
                    {
                        reverse_code_bit(&code, 32);
                        NEC_DEBUG_PRINTF("reverse bit: code = 0x%x\n",code);
                    }
                    if (1 == ir_endian->byte_msb_first)
                    {
                        code = ENDIAN_REVERSE_BYTE(code);
                        NEC_DEBUG_PRINTF("reverse byte: code = 0x%x\n",code);
                    }
                }
                last_code = code;
                /* Got a key code */
                irc_nec_state = 0;

                return code;
            }
        }
        else
        {
            irc_nec_state = 0;
            return PAN_KEY_INVALID;
        }
    }

    /* Next state */
    irc_nec_state++;
    return PAN_KEY_INVALID;
}

