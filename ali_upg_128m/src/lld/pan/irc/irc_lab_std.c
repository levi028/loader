/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: irc_lab_std.c
 *
 *  Description: This file implements the LAB mode Infra Receiver common
 *               driver function library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2005.03.01  Angus       Create
 *  2.  2005.04.14  Goliath        Bug fix on repeat key identify, remove the toggle bit
 *  3.  2006.03.28  Justin Wu   Clean up.
 ****************************************************************************/

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>
#include "irc.h"

#define IRC_PTC_PULSE_UNIT            140                            /* 0.14ms */
#define IRC_PTC_LEADING                140                            /* 0.14mS */
#define IRC_PTC_LEADING_REPT        (20 * IRC_PTC_PULSE_UNIT)    /* 11.2mS */
#define IRC_PTC_LEADING_FUNC        7590                        /* 7.59mS */
#define IRC_PTC_REPEATKEY_COUNTER    1                            /* for repeat key use */
#define IRC_PTS_REPEATKEY_INTERVAL    121600                        /*121.6ms*/
#define DBG_PRINTF(...)                //soc_printf

//#define IRC_PTC_Lead_Customer_Mask    0x5C0    //bit6~8 => customer code, bit10 => Leading Code
//#define IRC_PTC_Lead_Customer_Value    0x500    //For subsystem addresses 1 to 7 case

static UINT32 irc_ptc_state = 0;        /* IRC state */
static UINT32 irc_ptc_precision = 0;    /* Pulse uint precision,should < IRC_NEC_PULSE_UNIT/2 */
static UINT32 irc_ptc_prec_lead = 0;    /* Leading precision,should < IRC_NEC_PULSE_UNIT*2 */
static UINT32 irc_ptc_repeat_to = 0;
static UINT32 code = 0;
static UINT32 last_code = PAN_KEY_INVALID;
static UINT32 final_code = PAN_KEY_INVALID;
static UINT32 first_half = 1;        /* For dule-pulse senser */
static UINT32 last_width = 0;
static unsigned char key_counter =0;

/*****************************************************************************
 * void irc_LAB_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
 * Description: NEC mode IR controler: mode setup
 *
 * Arguments:
 *    UINT32 state        : Current IRC state, init is 0
 *    UINT32 precision    : Code pulse precision
 *    UINT32 repeat_to    : Repeat code timeout limit
 *
 * Return Value:
 ****************************************************************************/
void irc_lab_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
{
    irc_ptc_state = state;            /* 0 */
    irc_ptc_precision = precision;    /* 280 =>70 */
    irc_ptc_prec_lead = precision;    /* (precision << 2); */
    irc_ptc_repeat_to = repeat_to;    /* 500000 */

}

/*****************************************************************************
 * UINT32 irc_LAB_pulse_to_code(UINT32 pulse_width)
 * Description: NEC mode IR controler: translate pulse width to IR code
 *              This function support both of one and two trigle per bit mode.
 *
 * Arguments:
 *    UINT32 pulse_width    : Input pulse width, in uS
 *
 * Return Value:
 *    INT32                    : Key code
 ****************************************************************************/

UINT32 irc_lab_pulse_to_code(UINT32 pulse_width)
{
    UINT32 pic_11_state = 11;

    DBG_PRINTF("Enter pulse_width=%d \n", pulse_width);
    DBG_PRINTF("Enter last_width=%d \n", last_width);
    last_width += pulse_width;
    DBG_PRINTF("Af last_width=%d \n", last_width);
    DBG_PRINTF("State Count=%d \n", irc_ptc_state);
    DBG_PRINTF("============================== \n");

    /* Ignore burst pluses */
    if (pulse_width < irc_ptc_precision)
    {
        DBG_PRINTF("Return => ERROR !!!! pulse_width=%d <70ns \n", pulse_width);
        DBG_PRINTF("1E pw=%d\n", pulse_width);
        return PAN_KEY_INVALID;
    }

    /* if first pulse_width < 0.21ms (0.14*2-0.07), OK! */
    //if (first_half == 1 && pulse_width < (IRC_PTC_PULSE_UNIT << 1) - irc_PTC_precision)
    if ((1 == first_half )&& (pulse_width < (IRC_PTC_PULSE_UNIT * 3)))
    {
        {
            DBG_PRINTF("Return => Enter First pulse_width, forget it!\n");
            first_half = 0;
            return PAN_KEY_INVALID;
        }
    }

    pulse_width = last_width;
    last_width = 0;
    first_half = 1;

    /* Pulse process: transmit cycle pulse to code.
     * All of waveform send out by remote is inverted at receiver end.
     */
    /* Check if pulse_width is < 7.84ms && > 4.76ms or not */
    //if (pulse_width > (IRC_PTC_PULSE_UNIT * 34) && pulse_width < (IRC_PTC_PULSE_UNIT * 56))
    if ((pulse_width > (IRC_PTC_PULSE_UNIT * 34) )&& (pulse_width < (IRC_PTC_PULSE_UNIT * 58)))
    {
        /* if pulse_width > 5.18ms => identify as Bit"1" */
        code = (code << 1) + (pulse_width > IRC_PTC_PULSE_UNIT * 37);
        DBG_PRINTF("Receive Correct Code = 0x%x\n", code);
    }
    else
    {
        if(pic_11_state == irc_ptc_state)
        {
            /* Generate code */
            // soc_printf("\n\n\n $$$ Return => Receive IR Complete, Code=0x%x\n\n\n\n",code);
            code &= 0x000005ff;            /* Don't care the toggle bit */
            if(last_code ^ code)
            {
                key_counter = 0;
                final_code = code&0x000001ff;
                DBG_PRINTF("R 0x%x\n", code);
            }
            else                        /* Is Repeat Key */
            {
                key_counter++;
                if(key_counter > IRC_PTC_REPEATKEY_COUNTER)
                {
                    key_counter = 0;
                    final_code = code&0x000001ff;
                }
                else
                {
                    final_code=PAN_KEY_INVALID;
                }
            }

            last_code = code;
            irc_ptc_state = 0;
            code = 0;    /* for PTC codeword is 11 bits, not 32bits, need to reset */

            /* Check the code is valid or not again */
            DBG_PRINTF("Key 0x=%x\n", final_code);
            return final_code;
           }
        else
        {
            DBG_PRINTF("Return =>  ERROR! Unknow data code pulse, pulse_width=%d ! \n", pulse_width);
            DBG_PRINTF("2E pw=%d,sta=%d\n", pulse_width, irc_ptc_state);
               if(pulse_width > (IRC_PTS_REPEATKEY_INTERVAL * 2))
            {
                key_cnt = 0;
                key_counter = 1;
            }
            code = 0;
            irc_ptc_state = 0;
            return PAN_KEY_INVALID;
        }
    }

    /* Next state */
    irc_ptc_state++;
    DBG_PRINTF("Return => Goto next stage irc_PTC_state=%d\n", irc_ptc_state);
    return PAN_KEY_INVALID;
}

