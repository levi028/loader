/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
 *    disclosed to unauthorized individual.
 *
 *    File: smartcard_atr.c
 *
 *    Description: This file contains all globe micros and functions declare
 *                     of smartcard ATR operation.
 *
 *    History:
 *      Date          Author          Version         Comment
 *      ====          ======          =======         =======
 *
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 *    PARTICULAR PURPOSE.
 *****************************************************************************/
#include <types.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>

#include "smartcard.h"
#include "smartcard_atr.h"
#include "smartcard_txrx.h"
#include "smartcard_dev.h"

#define ATR_TIME_OVER  24  // ms  (((40000/(tp->smc_clock/1000))+1)*2)

#define SMC_ATR_INIT(tp) \
    do { \
        tp->atr_size = 0; \
        tp->atr_rlt = SMC_ATR_NONE; \
        MEMSET(tp->atr, 0x00, ATR_MAX_SIZE); \
    } while (0)
static UINT32 atr_num_ib_table[ATR_ID_NUM] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
UINT32 atr_f_table[ATR_FI_NUM] ={372,372,558,744,1116,1488,1860,F_RFU,F_RFU,512,768,1024,1536,2048,F_RFU,F_RFU};
static UINT32 atr_f_table_T14[ATR_FI_NUM_T14] = {F_RFU, 416, 620, F_RFU, F_RFU, F_RFU, F_RFU, F_RFU, F_RFU, \
                                         512, F_RFU, 1152, F_RFU, F_RFU, F_RFU, F_RFU};
UINT32 atr_d_table[ATR_DI_NUM] ={D_RFU,1,2,4,8,16,32,D_RFU,12,20,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU};
static UINT32 atr_d_table_T14[ATR_DI_NUM_T14] = {D_RFU, 1, 2, 4, 8, 16, 32, 64, D_RFU, D_RFU, D_RFU, D_RFU, \
                                         D_RFU, D_RFU, D_RFU, D_RFU};
UINT32 atr_i_table[ATR_I_NUM] = {25, 50, 100, 0};

static UINT8 atr_temp_buffer[ATR_MAX_SIZE];
static UINT8 atr_temp_size = 0;


/******************************************************************************************************
 *     Name        :    atr_init()
 *    Description    :    Initilize ATR structure.
 *    Parameter    :    atr_t atr        : Pointer to point ATR structure.
 *                UINT8 *buffer        : ATR buffer.
 *                UINT8 length        : ATR size.
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
UINT32 atr_init(atr_t *atr, UINT8 *buffer, UINT8 length)
{
    UINT8 td_i = 0;
    UINT8 pointer = 0;
    UINT8 pn = 0;
    UINT8 i = 0;
    UINT8 byte_2 = 2;
    UINT8 td_i_x7f = 0x7f;

    /* Check size of buffer */
    if (length < byte_2)
    {
        return SMART_WRONG_ATR;
    }
    /* Store T0 and TS */
    atr->TS = buffer[0];
    atr->T0 = buffer[1];
    td_i = buffer[1];
    pointer = 1;
    /* Store number of historical bytes */
    atr->hbn = td_i & 0x0F;
    /* TCK is not present by default */
    (atr->TCK).present = FALSE;

    /* Extract interface bytes */
    while (pointer < length)
    {
        /* Check buffer is long enought */
        if (pointer + atr_num_ib_table[(0xF0 & td_i) >> 4] >= length)
        {
            return SMART_WRONG_ATR;
        }
        /* Check TAi is present */
        if (0xFF == (td_i | 0xEF))
        {
            pointer++;
            atr->ib[pn][ATR_INTERFACE_BYTE_TA].value = buffer[pointer];
            atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = TRUE;
        }
        else
        {
            atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = FALSE;
        }
        /* Check TBi is present */
        if (0xFF == (td_i | 0xDF))
        {
            pointer++;
            atr->ib[pn][ATR_INTERFACE_BYTE_TB].value = buffer[pointer];
            atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = TRUE;
        }
        else
        {
            atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = FALSE;
        }

        /* Check TCi is present */
        if (0xFF == (td_i | 0xBF))
        {
            pointer++;
            atr->ib[pn][ATR_INTERFACE_BYTE_TC].value = buffer[pointer];
            atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = TRUE;
        }
        else
        {
            atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = FALSE;
        }

        /* Read td_i if present */
        if (0xFF == (td_i | td_i_x7f))
        {
            pointer++;
            td_i =  buffer[pointer];
            atr->ib[pn][ATR_INTERFACE_BYTE_TD].value = td_i;
            atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = TRUE;
            (atr->TCK).present = ((td_i & 0x0F) != ATR_PROTOCOL_TYPE_T0);
            if (pn >= ATR_MAX_PROTOCOLS)
            {
                return SMART_WRONG_ATR;
            }
            pn++;
        }
        else
        {
            atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = FALSE;
            break;
        }
    }

    /* Store number of protocols */
    atr->pn = pn + 1;

    /* Store historical bytes */
    if (pointer + atr->hbn >= length)
    {
        return SMART_WRONG_ATR;
    }

    //    MEMCPY (atr->hb, buffer + pointer + 1, atr->hbn);
    for(i=0; i<atr->hbn; i++)
    {
        *(atr->hb+i) = *(buffer+pointer+1+i);
    }
    pointer += (atr->hbn);

    /* Store TCK  */
    if ((atr->TCK).present)
    {
        if (pointer + 1 >= length)
        {
            return SMART_WRONG_ATR;
        }

        pointer++;
        (atr->TCK).value = buffer[pointer];
    }

    atr->length = pointer + 1;
    return SMART_NO_ERROR;
}

static void smc_atr_handle_f(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_fi = 0;
    UINT32 atr_f = 0;

    if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
    {
        atr_fi = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;

		if(14 == handler->T)
		{
        	atr_f = atr_f_table_T14[atr_fi];
		}
		else
		{
        	atr_f = atr_f_table[atr_fi];
		}
		//robbin change
        if(F_RFU == atr_f)
        {
            atr_f = ATR_DEFAULT_F;
        }
        SMC_RW_PRINTF("SMC ATR: Clock Rate Conversion atr_f=%d, atr_fi=%d\n", atr_f, atr_fi);
    }
    else
    {
        atr_f = ATR_DEFAULT_F;
        SMC_RW_PRINTF("SMC ATR: Clock Rate Conversion atr_f=(Default)%d\n", atr_f);
    }
    handler->F = atr_f;
}

static void smc_atr_handle_d(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_di = 0;
    UINT32 atr_d = 0;

    if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
    {
        atr_di = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
		if(14 == handler->T)
		{
        	atr_d = atr_d_table_T14[atr_di];
		}
		else
		{
        	atr_d = atr_d_table[atr_di];
		}
		if(D_RFU == atr_d)
        {
            atr_d = ATR_DEFAULT_D;
        }
        SMC_RW_PRINTF("SMC ATR: Bit Rate Adjustment Factor atr_d=%d, atr_di=%d\n", atr_d, atr_di);
    }
    else
    {
        atr_d = ATR_DEFAULT_D;
        SMC_RW_PRINTF("SMC ATR: Bit Rate Adjustment Factor atr_d=(Default)%d\n", atr_d);
    }
    handler->D = atr_d;
}

static void smc_atr_handle_i(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_ii = 0;
    UINT32 atr_i = 0;

    if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
    {
        atr_ii = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x60) >> 5;
        atr_i = atr_i_table[atr_ii];
        if(I_RFU == atr_i)
        {
            atr_i = ATR_DEFAULT_I;
        }
        SMC_RW_PRINTF("SMC ATR: Programming Current Factor atr_i=%d, atr_ii=%d\n", atr_i, atr_ii);
    }
    else
    {
        atr_i= ATR_DEFAULT_I;
        SMC_RW_PRINTF("SMC ATR: Programming Current Factor atr_i=(Default)%d\n", atr_i);
    }
    handler->I = atr_i;
}

static void smc_atr_handle_p(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_pi1 = 0;
    UINT8 atr_pi2 = 0;
    UINT32 atr_p = 0;

    if (atr->ib[1][ATR_INTERFACE_BYTE_TB].present)
    {
        atr_pi2 = atr->ib[1][ATR_INTERFACE_BYTE_TB].value;
        atr_p = atr_pi2;
        SMC_RW_PRINTF("SMC ATR: Programming Voltage Factor atr_p=%d, atr_pi2=%d\n", atr_p, atr_pi2);
    }
    else if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
    {
        atr_pi1 = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x1F);
        atr_p = atr_pi1;
        SMC_RW_PRINTF("SMC ATR: Programming Voltage Factor atr_p=%d, atr_pi1=%d\n", atr_p, atr_pi1);
    }
    else
    {
        atr_p = ATR_DEFAULT_P;
        SMC_RW_PRINTF("SMC ATR: Programming Voltage Factor atr_p=(Default)%d\n", atr_p);
    }
    handler->P = atr_p;
}

static void smc_atr_handle_n(struct smartcard_private *handler, atr_t *atr)
{
    UINT32 atr_n = 0;

    if (atr->ib[0][ATR_INTERFACE_BYTE_TC].present)
    {
        atr_n = atr->ib[0][ATR_INTERFACE_BYTE_TC].value;
        /*if(n == 0xFF)
          n = 11;
         */
    }
    else
    {
        atr_n = ATR_DEFAULT_N;
    }
    SMC_RW_PRINTF("SMC ATR: Extra Guardtime atr_n=%d\n", atr_n);
    handler->N = atr_n;
}

static void smc_atr_config_t0(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_wi = 0;

    if (atr->ib[1][ATR_INTERFACE_BYTE_TC].present)//tc2
    {
        atr_wi = atr->ib[1][ATR_INTERFACE_BYTE_TC].value;
    }
    else
    {
        atr_wi = ATR_DEFAULT_WI;
    }

    SMC_RW_PRINTF("SMC ATR: Work Waiting Time atr_wi=%d\n", atr_wi);
    handler->WI = atr_wi;

    handler->cwt = ((960*atr_wi*handler->F)/(handler->smc_clock/1000))+1;
    handler->first_cwt = handler->cwt;
}
static void smc_atr_config_t1(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 i = 0;
    UINT8 checksum=0;
	t1_state_t T1 = handler->T1;//20151021
	
    for (i = 1 ; i < atr->pn ; i++)
    {
        /* check for the first occurance of atr_t=1 in td_i */
        if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present &&
                (ATR_PROTOCOL_TYPE_T1 == (atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F)))
        {
            /* check if ifsc exist */
            if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].present)
            {
                handler->T1.ifsc = atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].value;
            }
            else
            {
                handler->T1.ifsc = ATR_DEFAULT_IFSC; /* default 32*/
            }

            /* Get CWI */
            if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            {
                handler->CWI = atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0x0F;
            }
            else
            {
                handler->CWI = ATR_DEFAULT_CWI; /* default 13*/
            }
            handler->cwt =  (((1<<(handler->CWI))+11 )*handler->smc_etu)/(handler->smc_clock/1000) + 1;
            /*Get BWI*/
            if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            {
                handler->BWI = (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
            }
            else
            {
                handler->BWI = ATR_DEFAULT_BWI; /* default 4*/
            }
            //handler->first_cwt= (((1<<(handler->BWI))*960+11)*handler->smc_etu)/(handler->smc_clock/1000);
            handler->first_cwt = (11*handler->smc_etu)/(handler->smc_clock/1000) \
                         + ((1<<(handler->BWI))*960*ATR_DEFAULT_F)/(handler->smc_clock/1000) + 2;
            if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].present)
            {
                checksum = atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].value & 0x01;
            }
            else
            {
                checksum = ATR_DEFAULT_CHK; /* default - LRC */
            }

            handler->error_check_type = ((ATR_DEFAULT_CHK == checksum)?IFD_PROTOCOL_T1_CHECKSUM_LRC \
                    :IFD_PROTOCOL_T1_CHECKSUM_CRC);

			t1_set_param(&T1, handler->error_check_type, 0);//add on 20151021, set the check type base on ATR

        }
    }
    SMC_RW_PRINTF("SMC ATR: T1 special ifsc: %d,  CWI:%d,  BWI:%d, checksum:%d(3:LRC,2:CRC)\n",
            handler->T1.ifsc, handler->CWI, handler->BWI, handler->error_check_type);
}

static void smc_atr_config_t(struct smartcard_private *handler, atr_t *atr)
{
    if(handler->D != 0)
    {
        handler->smc_etu = handler->F / handler->D;
    }
    handler->first_cwt = FIRST_CWT_VAL;
    handler->cwt = CWT_VAL;

    if(0 == handler->T)
    {
        smc_atr_config_t0(handler, atr);
    }
    else if(1 == handler->T)
    {
        smc_atr_config_t1(handler, atr);
    }
	else if(14 == handler->T)
	{
		smc_atr_config_t0(handler, atr); //Irdeto spec not mention WI ,so we use T0 as defulat
	}
}

static void smc_atr_handle_t(struct smartcard_private *handler, atr_t *atr)
{
    UINT8 atr_t_protocol = 0xFF;
    UINT8 ta_after_t15 = 0;
    UINT8 find_t15 = 0;
    UINT8 i = 0;

    for (i=0; i<atr->pn; i++)
    {
        if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && (0xFF == atr_t_protocol))
        {
            /* set to the first protocol byte found */
            atr_t_protocol = atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
            SMC_RW_PRINTF("SMC ATR: default protocol: atr_t=%d\n", atr_t_protocol);
        }
    }
    /* Try to find 1st TA after atr_t = 15 */
    for (i=0; i<atr->pn; i++)
    {
        if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present)
        {
            if((!find_t15)&&(0xF == (atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F)))
            {
                find_t15 = 1;
                SMC_RW_PRINTF("SMC ATR: Find atr_t==15 at TD%d\n", i+1);
                continue;
            }
        }
        if ((find_t15)&&(atr->ib[i][ATR_INTERFACE_BYTE_TA].present))
        {
            ta_after_t15 = atr->ib[i][ATR_INTERFACE_BYTE_TA].value;
            SMC_RW_PRINTF("SMC ATR: Find 1st TA after atr_t==15 at TA%d, value %02x\n", (i+1), ta_after_t15);
            break;
        }
    }

    if(handler->class_selection_supported)
    {
        if(0==(ta_after_t15&0x3f))
        {
            SMC_RW_PRINTF("SMC ATR: smc has no class indicator!\n");
            handler->smc_supported_class = 0x0;
        }
        else
        {
            handler->smc_supported_class = ta_after_t15&0x3f;
        }
    }
    /* if has TA2, it indicate the special protocal */
    if (atr->ib[1][ATR_INTERFACE_BYTE_TA].present)
    {
        atr_t_protocol = atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F;
        SMC_RW_PRINTF("SMC ATR: specific mode found: atr_t_protocol=%d\n", atr_t_protocol);
        if (atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x10) //check TA(2), bit 5
        {
            handler->ta2_spec = 0;  //use the default value of f/d
        }
        else
        {
            handler->ta2_spec = 1;  //Use the value specified in the ATR
        }
    }

    if (0xFF == atr_t_protocol)
    {
        SMC_RW_PRINTF("SMC ATR: no default protocol found in ATR. Using atr_t_protocol=0\n");
        atr_t_protocol = ATR_PROTOCOL_TYPE_T0;
    }
    handler->T = atr_t_protocol;

    smc_atr_config_t(handler, atr);
}
/******************************************************************************************************
 *     Name        :    atr_config_parameter()
 *    Description    :    By analyze ATR data to get the card information.
 *    Parameter    :    struct smc_device *dev    : Private structure handle.
 *                atr_t atr                : Pointer to point ATR buffer.
 *    Return        :    INT32                    : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
UINT32 atr_config_parameter(struct smc_device *dev, atr_t *atr)
{
    UINT8 i = 0;
    struct smartcard_private *handler = (struct smartcard_private *)dev->priv;

    smc_atr_handle_f(handler, atr);
    smc_atr_handle_d(handler, atr);
    smc_atr_handle_i(handler, atr);
    smc_atr_handle_p(handler, atr);
    smc_atr_handle_n(handler, atr);
    smc_atr_handle_t(handler, atr);

    SMC_RW_PRINTF("SMC ATR: first CWT: %d, CWT: %d\n", handler->first_cwt, handler->cwt);
    SMC_RW_PRINTF("SMC ATR: history bytes:");
    for(i=0; i < atr->hbn; i++)
    {
        SMC_RW_PRINTF("%02x ", (atr->hb)[i]);
    }
    SMC_RW_PRINTF("\n");

    return SMART_NO_ERROR;
}

static void smc_atr_set_receiver_mode(struct smartcard_private *tp, UINT32 ioaddr, INT8 enable)
{
    if (enable)/* enable receiver mode and set to the direct mode */
    {
        OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));
    }
    else       /* disable receiver mode. */
    {
        OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL) & (~SMC_SCR_CTRL_RECV)));
    }
}

static void smc_atr_warm_reset_config_register(struct smartcard_private *tp, UINT32 ioaddr)
{
    if(tp->inverse_convention)
    {
        SMC_RW_PRINTF("SMC ATR: Already know it's inverse card, dis apd\n");
        OUTPUT_UINT8(ioaddr + REG_ICCR, (INPUT_UINT8(ioaddr + REG_ICCR)&(~0x30))|SMC_RB_ICCR_AUTO_PRT);
    }
    else
    {
        SMC_RW_PRINTF("SMC ATR: Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
        OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)&0xcf);
        OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|(tp->parity_disable<<5)|(tp->apd_disable<<4));
    }
}

static void smc_atr_cold_reset_config_register(struct smartcard_private *tp, UINT32 ioaddr)
{
    if(tp->inverse_convention)
    {
        SMC_RW_PRINTF("SMC ATR: Already know it's inverse card, dis apd\n");
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT);
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|0x40);
        }
    }
    else
    {
        SMC_RW_PRINTF("SMC ATR: Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4));
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4)|0x40);
        }
    }
    osal_delay(ATV_VCC2IO>>1);

    if(tp->use_gpio_vpp)
    {
        if(tp->internal_ctrl_vpp)
        {
            OUTPUT_UINT8(ioaddr + REG_CLK_VPP, INPUT_UINT8(ioaddr + REG_CLK_VPP)|SMC_RB_CTRL_VPP);
        }
        else
        {
            HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
        }
    }
    osal_delay(/*20*/ATV_VCC2IO>>1);

    if((ALI_S3281==tp->smc_chip_id))
    {
        osal_delay(10*(ATV_VCC2IO>>1));
    }

    OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_DIO);
}

static void smc_atr_set_register_special_card_check(struct smartcard_private *tp, UINT32 ioaddr)
{
    UINT32 etu_550 = 550;

    /* Basically, we should not disable parity check while get ATR */
    /* but for C1800A cards, we have to do so.*/
    /* C1800A cards, disable parity check */
    if(INPUT_UINT16(ioaddr + REG_ETU0) > etu_550)
    {
        OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|0x20);
    }
}

static INT32 smc_atr_isr1_rst_low_trigger(struct smartcard_private *tp, UINT32 ioaddr)
{
    SMC_RW_PRINTF("SMC ATR: ATR RST on low trigger!\n");
    tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
    if(ALI_S3602==tp->smc_chip_id)
    {
        if(0!=smc_read_rx(tp, ioaddr))
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }

    return 0;
}
static INT32 smc_atr_isr1_count_trigger(struct smc_device *dev, UINT32 ioaddr)
{
    UINT8 cnt_trigger = 0;
    UINT32 old_tick = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    SMC_RW_PRINTF("SMC ATR: set RST to high.\n");

    tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
    tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);

    OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
    OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);

    for( ; ; )
    {
        if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
        {
            tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
            SMC_RW_PRINTF("SMC ATR: reset high!\n");
            break;
        }
               
		if ((tp->isr1_interrupt_status & SMC_ISR1_RST_NATR) && (0 == cnt_trigger))
    	{
        	tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);            
			cnt_trigger = 1;
			old_tick = osal_get_tick();
    	}
        
		if (0 != cnt_trigger)
		{		
			if (0 != tp->ext_atr_delay)
			{
				if ((osal_get_tick() - old_tick) > tp->ext_atr_delay)
				{						
					return SMART_NO_ANSWER;
				}
			}
			else
			{					
				return SMART_NO_ANSWER;
			}
		}
        /*
           if( tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT)
           {
           tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);
           SMC_PRINTF("Wait ATR Time Out!\n");
           return !RET_SUCCESS;
           }*/
        if(smc_dev_card_exist(dev) != SUCCESS)
        {
            return SMART_NOT_INSERTED;
        }
    }
    return 0;
}

static INT32 smc_atr_isr1_process(struct smc_device *dev, UINT32 ioaddr)
{
    INT32 ret = 0;
	UINT32 old_tick = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	old_tick = osal_get_tick();
    for( ; ; )
    {
        if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
        {
            ret = smc_atr_isr1_rst_low_trigger(tp, ioaddr);
            if (ret)
            {
                return ret;
            }
        }

        if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
        {
            ret = smc_atr_isr1_count_trigger(dev, ioaddr);
            return ret;
        }
		
		if ((osal_get_tick() - old_tick) > ATR_TIME_OVER)
		{					
			SMC_RW_PRINTF("in[%s]line=%d, rst high to timeout= %dms\n", __FUNCTION__, __LINE__, osal_get_tick()-old_tick);
			return SMART_NO_ANSWER; 			
		}

        if(smc_dev_card_exist(dev) != SUCCESS)
        {
            return SMART_NOT_INSERTED;
        }
    }

    return ret;
}


static INT32 smc_atr_check(UINT32 ioaddr, UINT8 *buffer, UINT8 size, UINT8 start_addr)
{
	UINT8 i = 0;
	atr_t * atr_info =NULL;
	INT32 ret = RET_FAILURE;
	
	if (NULL == buffer)
	{
		SMC_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return RET_FAILURE;
	}

	SMC_PRINTF("[ %s %d ], size = %d, start_addr = %d\n", __FUNCTION__, __LINE__, size, start_addr);

	atr_info = (atr_t *)MALLOC(sizeof(atr_t));
	if(NULL == atr_info)
	{
		SMC_PRINTF("[ %s ], Memory allocate failed.\n", __FUNCTION__);
		return RET_FAILURE;
	}
	MEMSET(atr_info, 0, sizeof(atr_t));
	
	for(i=0; i<size; i++)
	{
		buffer[start_addr + i] = INPUT_UINT8(ioaddr + REG_RBR);
		SMC_PRINTF("%d[%02x] ", (start_addr + i), buffer[start_addr + i]);		
	}	
	SMC_PRINTF("\n");

	if(SMART_NO_ERROR == atr_init(atr_info, buffer, size+start_addr))
	{
		ret = RET_SUCCESS;
	}
	else
	{
		ret = RET_FAILURE;
	}


	if (NULL != atr_info)
	{
		FREE(atr_info);
		atr_info = NULL;		
	}			

	return ret;
}


static INT32 smc_atr_isr0_process(struct smc_device *dev, UINT32 ioaddr)
{
    UINT32 t_wait_atr_tmo = 0;
    UINT32 t_wait_atr_time = 0;
    UINT16 i_rx_cnt=0;
    INT32 ret = RET_FAILURE;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;    	

	
	MEMSET(atr_temp_buffer, 0x00, sizeof(atr_temp_buffer));
	atr_temp_size = 0;

    t_wait_atr_tmo = (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);
    t_wait_atr_time = osal_get_tick();
    i_rx_cnt = smc_read_rx(tp, ioaddr);
    
	ret = smc_atr_check(ioaddr, atr_temp_buffer, i_rx_cnt, atr_temp_size);
	atr_temp_size += i_rx_cnt;
	if (RET_SUCCESS == ret)
	{		
		return ret;
	}	

    for( ; ; )
    {
        if(smc_dev_card_exist(dev) != SUCCESS)
        {
            return SMART_NOT_INSERTED;
        }

        if((tp->isr0_interrupt_status & SMC_ISR0_FIFO_RECV )|| \
                ( tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT))
        {
            tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
            tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);
            break;
        }

        if((0==(INPUT_UINT8(ioaddr + REG_ICCR)&0x30)) && \
                (tp->isr0_interrupt_status&SMC_ISR0_PE_RECV))
        {
            SMC_RW_PRINTF("SMC ATR: Get ATR Parity Error!\n");
            tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
            return SMART_PARITY_ERROR;
        }

        if(i_rx_cnt == smc_read_rx(tp, ioaddr))
        {
            osal_task_sleep(1);
            t_wait_atr_time = osal_get_tick() - t_wait_atr_time;

            if(t_wait_atr_tmo >= t_wait_atr_time)
            {
                t_wait_atr_tmo -= t_wait_atr_time;
            }
            else
            {
                if(smc_read_rx(tp, ioaddr))
                {
                    break;
                }

                SMC_RW_PRINTF("SMC ATR: Wait ATR time out!\n");
                return SMART_NO_ANSWER;
            }
        }
        else
        {
            i_rx_cnt = smc_read_rx(tp, ioaddr);
            t_wait_atr_tmo = (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);
            
			ret = smc_atr_check(ioaddr, atr_temp_buffer, i_rx_cnt, atr_temp_size);
			atr_temp_size += i_rx_cnt;			
			if (RET_SUCCESS == ret)
			{				
				return ret;
			}			
        }
        t_wait_atr_time = osal_get_tick();
    }

    return ret;
}

static INT32 smc_atr_decode(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    INT32 i = 0;
	#if 0 /* SMC_ATR_PRESET_TEST_BUF */
    UINT8 cc = 0;
	#endif
    UINT32 ioaddr = dev->base_addr;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    
    if((0 == atr_temp_size) || (atr_temp_size > ATR_MAX_SIZE))
    {
        SMC_RW_PRINTF("SMC ATR: Invalid ATR length: %d\n", atr_temp_size);
        if(atr_temp_size>ATR_MAX_SIZE)
        {
            atr_temp_size = ATR_MAX_SIZE;
        }
        else
        {
            return SMART_WRONG_ATR;
        }
    }
    SMC_RW_PRINTF("SMC ATR: %d ATR bytes received: ", atr_temp_size);

    tp->atr_size = atr_temp_size;
    *atr_size = atr_temp_size;    
	MEMCPY(buffer, atr_temp_buffer, atr_temp_size);	
	for (i=0; i<atr_temp_size; i++)
	{		
		SMC_RW_PRINTF("%02x ",buffer[i] );		
	}	
	SMC_RW_PRINTF("\n");    

#if 0 /* SMC_ATR_PRESET_TEST_BUF */

    UINT8 atr_test_str[] =
    {
        0X3B, 0XDA, 0X18, 0XFF, 0X81, 0XB1, 0XFE,
        0X75, 0X1F, 0X03, 0X00, 0X31, 0XC5, 0X73,
        0XC0, 0X01, 0X40, 0X00, 0X90, 0X00, 0X0C
    };

    tp->atr_size = *atr_size = atr_temp_size;
    MEMCPY(buffer, atr_temp_buffer, atr_temp_size);	

    tp->atr_size = *atr_size = sizeof(atr_test_str);
    for(i=0;i<tp->atr_size;i++)
    {
        cc = atr_test_str[i];
        SMC_RW_PRINTF("%02x ",cc);
        buffer[i] = cc;
    }

    SMC_RW_PRINTF("\n");
#endif

    if(0x03 == buffer[0])
    {
        SMC_RW_PRINTF("SMC ATR: Inverse card detected!\n");
        /* set to the inverse mode. */
        tp->inverse_convention = 1;
        OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP| \
                SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
        invert(buffer, tp->atr_size);
    }
    else if((0x3f == buffer[0]) && (1 == tp->ts_auto_detect))
    {
        SMC_RW_PRINTF("SMC ATR: Inverse card auto detected!\n");
        /* set to the inverse mode. */
        tp->inverse_convention = 1;
        OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP| \
                SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
    }
    else if(0x3b == buffer[0])
    {
        SMC_RW_PRINTF("SMC ATR: Normal card detected!\n");
        tp->inverse_convention = 0;
    }
    else
    {
        return SMART_WRONG_ATR;
    }

    return RET_SUCCESS;
}

/******************************************************************************************************
 *     Name        :    smc_dev_get_card_atr()
 *    Description    :    To get the smart card ATR.
 *    Parameter    :    struct smc_device *dev    : Devcie handle.
 *                UINT8 *buffer        : Read data buffer.
 *                UINT16 *atr_size    : ATR data size.
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_get_card_atr(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    UINT32 ioaddr = dev->base_addr;
    INT32 ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    SMC_RW_PRINTF("SMC ATR: To get smartcard ATR\n");

    tp->force_tx_rx_thld = FORCE_TX_RX_THLD;

    SMC_ATR_INIT(tp);
    smc_atr_set_receiver_mode(tp, ioaddr, TRUE);

    OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);

    smc_write_rx(tp, ioaddr, ATR_MAX_SIZE);

    /* clear interrupt status */
    OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1));
    OUTPUT_UINT8(ioaddr + REG_ISR0, INPUT_UINT8(ioaddr + REG_ISR0));

    osal_interrupt_disable();
    if(tp->inserted != 1)  //card not insert
    {
        osal_interrupt_enable();
        return SMART_NOT_INSERTED;
    }

    if(tp->warm_reset)
    {
        smc_atr_warm_reset_config_register(tp, ioaddr);
    }
    else
    {
        smc_atr_cold_reset_config_register(tp, ioaddr);
    }

    smc_atr_set_register_special_card_check(tp, ioaddr);
    osal_interrupt_enable();

    if(!tp->warm_reset)
    {
        osal_delay(/*200*/ATV_IO2CLK);
        OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
        osal_delay(200);
    }
	
	ret = smc_atr_isr1_process(dev, ioaddr);
    if (ret != RET_SUCCESS)
    {
        return ret;
    }

	ret = smc_atr_isr0_process(dev, ioaddr);
    if (ret != RET_SUCCESS)
    {
        return ret;
    }

	ret = smc_atr_decode(dev, buffer, atr_size);
    if (ret != RET_SUCCESS)
    {
        return ret;
    }
    /* disable receiver mode. */
    smc_atr_set_receiver_mode(tp, ioaddr, FALSE);

    return RET_SUCCESS;
}

//20151021
void smc_set_protocol_config_T_0(struct smartcard_private *tp)
{
	smc_atr_config_t0(tp, tp->atr_info);
}

void smc_set_protocol_config_T_1(struct smartcard_private *tp)
{
	smc_atr_config_t1(tp, tp->atr_info);
}
//20151021

