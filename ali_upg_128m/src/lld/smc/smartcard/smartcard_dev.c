/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
 *    disclosed to unauthorized individual.
 *
 *    File: smartcard_dev.c
 *
 *    Description: This file contains all globe micros and functions declare
 *                  of smartcard device operation.
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

#include "smartcard_txrx.h"
#include "smartcard.h"

static UINT8 pps_no_response_flag = 0; /* no need to send PPS request next time.*/  //20150713_libo remove it because of observation4
static INT32 reset_err_flag = 0;
static UINT8 swaptab[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};

void invert(UINT8 *data, INT32 n)
{
    INT32 i = 0;

    for(i=n-1; i>=0; i--)
    {
        data[i] = (swaptab[data[i]&0x0F]<<4) | swaptab[data[i]>>4];
    }
}
/******************************************************************************************************
 *     Name        :    smc_dev_deactive()
 *    Description    :    Smart card deactive.
 *    Parameter    :    struct smc_device *dev        : Devcie handle
 *    Return        :    INT32                : SUCCESS.
 *
 ******************************************************************************************************/
INT32 smc_dev_deactive(struct smc_device *dev)
{
    UINT32 base_addr = 0;
    struct smartcard_private *tp = NULL;
    UINT32 i = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    base_addr = dev->base_addr;
    OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST));        // RST L
    osal_delay(/*100*/DATV_RST2CLK);
    OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_CLK));        // CLK L
    osal_delay(/*100*/DATV_CLK2IO);
    OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_DIO));// DIO L
    osal_delay(DATV_IO2VCC>>1);
    if(tp->use_gpio_vpp)
    {
        if(tp->internal_ctrl_vpp)
        {
            OUTPUT_UINT8(base_addr + REG_CLK_VPP, INPUT_UINT8(base_addr + REG_CLK_VPP)&(~SMC_RB_CTRL_VPP));
        }
        else
        {
            HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, !(tp->gpio_vpp_pol));
        }
        SMC_PRINTF("SMC DEV: Vpp Low!");
        //osal_delay(100);
    }
    osal_delay(/*100*/DATV_IO2VCC>>1);
    OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) | SMC_RB_ICCR_VCC);        // VCC Inactive
    for(i=0;i<11;i++)
    {
        osal_delay(1000);
    }
    SMC_PRINTF("SMC DEV: Smartcard deactived!\n");
    tp->reseted = 0;

    return RET_SUCCESS;
}

INT32 smc_dev_set_pps(struct smc_device *dev, UINT8 pps0, UINT8 atr_fi, UINT8 atr_di, UINT8 *need_reset_etu)
{
    UINT8 pps_buf[6] = {0};
    UINT8 pps_echo[6] = {0};
    INT32 rlt = (!RET_SUCCESS);
    UINT16 actsize = 0;
    UINT16 rw_len = 0;

    SMC_PRINTF("SMC DEV: PPS exchange");

    MEMSET(pps_buf, 0, 6);
    MEMSET(pps_echo, 0, 6);
    pps_buf[0] = 0xff;
    pps_buf[1] = pps0;
    if(pps0&0x10)
    {
        pps_buf[2] = ((atr_fi&0xf)<<4)|(atr_di&0xf);
        pps_buf[3] = pps_buf[0]^pps_buf[1]^pps_buf[2];
        SMC_PRINTF("SMC DEV: ppss %02x pps0 %02x pps1 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2], pps_buf[3]);
    }
    else
    {
        pps_buf[2] = pps_buf[0]^pps_buf[1];
        SMC_PRINTF("SMC DEV: ppss %02x pps0 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2]);
    }

    if(pps0&0x10)
    {
        rw_len = 4;
    }
    else
    {
        rw_len = 3;
    }
    MEMSET(pps_echo, 0x5a, 4);
    #if 0
        smc_dev_transfer_data(dev, pps_buf, rw_len, pps_echo, rw_len, &actsize);
    #else
        smc_dev_pps_write(dev, pps_buf, rw_len, (INT16 *)(&actsize));
        smc_dev_pps_read(dev, pps_echo, rw_len, (INT16 *)(&actsize));
    #endif
    SMC_PRINTF("SMC DEV: pps echo: %02x %02x %02x %02x\n", pps_echo[0], pps_echo[1], pps_echo[2], pps_echo[3]);
    if(actsize)
    {
        if(pps_buf[0] == pps_echo[0])
        {
            if((pps_buf[1]&0xf)==(pps_echo[1]&0xf))
            {
                if(pps0&0x10)
                {
                    if((pps_buf[1]&0x10)==(pps_echo[1]&0x10))
                    {
                        rlt = RET_SUCCESS;
                        *need_reset_etu = 1;
                        SMC_PRINTF("SMC DEV: PPS SUCCESS!\n");
                    }
                    else
                    {
                        rlt = RET_SUCCESS;
                        *need_reset_etu = 0;
                        SMC_PRINTF("pps SUCCESS! USE default Fi, Di\n");
                    }
                }
                else
                {
                    rlt = RET_SUCCESS;
                    *need_reset_etu = 1;
                    SMC_PRINTF("SMC DEV: PPS SUCCESS!\n");
                }
            }
            //else
            //    SMC_PRINTF("pps : USE default atr_t!\n");
        }
        //else
        //    SMC_PRINTF("card don't support PPS!\n");
    }
    //else
    //    SMC_PRINTF("pps got NO Response!\n");

    osal_task_sleep(1);  // delay 1ms before setting etu register(after pps) to avoid a etu low level

    return rlt;
}

/******************************************************************************************************
 *     Name        :    smc_dev_card_exist()
 *    Description    :    Smart card reader controler close funciton.
 *    Parameter    :    struct smc_device *dev        : Devcie handle.
 *    Return        :    INT32                : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_card_exist(struct smc_device *dev)
{
    struct smartcard_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    if(tp->inserted)
    {
        return SUCCESS;
    }
    else
    {
        return !SUCCESS;
    }
}

INT32 smc_warm_reset(struct smc_device *dev)
{
    UINT32 base_addr = dev->base_addr;
    UINT32 i = 0;

    OUTPUT_UINT8(base_addr + REG_ICCR, (INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST)));        // RST L
    for(i=0;i<11;i++)
    {
        osal_delay(1000);
    }
    SMC_PRINTF("SMC DEV: Warm Reset\n");
    return RET_SUCCESS;
}

static void smc_proc_card_reset (struct smartcard_private *tp, UINT32 ioaddr)
{
    if(tp->warm_reset)
    {
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN);
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN|0x40);
        }
    }
    else
    {
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN);
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|0x40);
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
    osal_interrupt_enable();
    if(!tp->warm_reset)
    {
        osal_delay(/*200*/ATV_IO2CLK);
        OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
        osal_delay(200);
    }
}
/******************************************************************************************************
 *     Name        :    smc_dev_get_card_etu()
 *    Description    :    To get the smart card ETU.
 *    Parameter    :    struct smc_device *dev    : Devcie handle.
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_get_card_etu(struct smc_device *dev)
{
    UINT32 ioaddr = dev->base_addr;
    UINT32 etu=0;
    UINT32 etu3=0;
    UINT32 wai_atr_tmo = 0;
    UINT32 wai_atr_time = 0;
    UINT8 cc = 0;
    UINT16 rx_cnt =0;
    UINT8 etu_trigger = 0;    
    UINT32 old_etu_tick1=0;
    UINT8 atr_3b = 0x3b;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    SMC_RW_PRINTF("SMC DEV: %s ioaddr: %x\n",__FUNCTION__,ioaddr);

    OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));
    osal_delay(200);

    OUTPUT_UINT8(ioaddr + REG_ISR0, 0xff);
    OUTPUT_UINT8(ioaddr + REG_ISR1, 0xff);

    //no fifo mode
    //OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,0x00);
    OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);
    smc_write_rx(tp, ioaddr, 32);
    osal_interrupt_disable();
    if(tp->inserted != 1)  //card not insert
    {
        osal_interrupt_enable();
        return RET_FAILURE;
    }
    smc_proc_card_reset (tp, ioaddr);
    while(1)
    {
        if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
        {
            tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
            SMC_RW_PRINTF("SMC DEV: atr_on_low trigger!\n");
            if(ALI_S3602==tp->smc_chip_id)
            {
                if(0!=smc_read_rx(tp, ioaddr))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
        {
            SMC_RW_PRINTF("SMC DEV: set RST to high\n");
            tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
            tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);

            OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
            OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);
            while(0==smc_read_rx(tp, ioaddr))
            {                               
	      		if ((tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(0 == etu_trigger))
                	{
                    	tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);                    
				etu_trigger = 1;
				old_etu_tick1 = osal_get_tick(); 
                	}
                
			if (0 != etu_trigger)
			{
				if (0 != tp->ext_atr_delay)
				{
					if ((osal_get_tick() - old_etu_tick1) > tp->ext_atr_delay)
					{
						return !RET_SUCCESS;
					}
				}
				else
				{
					return !RET_SUCCESS;
				}
			}

                if(smc_dev_card_exist(dev) != SUCCESS)
                {
                    return !RET_SUCCESS;
                }
            }
            cc = INPUT_UINT8(ioaddr + REG_RBR);
            if(cc == atr_3b)
            {
                tp->inverse_convention = 0;
                SMC_RW_PRINTF("SMC DEV: Normal card %02x\n", cc);
            }
            else
            {
                tp->inverse_convention = 1;
                SMC_RW_PRINTF("SMC DEV: Inverse card %02x\n", cc);
            }
            while(1)
            {
                if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
                {
                    tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
                    break;
                }

                if(tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)
                {
                    tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);

                    return !RET_SUCCESS;
                }

                if(smc_dev_card_exist(dev) != SUCCESS)
                {
                    return !RET_SUCCESS;
                }
                //osal_delay(1);
                //Why there is no time out??
            }
            break;
        }
        if(smc_dev_card_exist(dev) != SUCCESS)
        {
            return !RET_SUCCESS;
        }
        //osal_delay(1);
    }
    wai_atr_tmo =  (9600*372*2)/(tp->smc_clock/1000);
    wai_atr_time = osal_get_tick();
    rx_cnt = smc_read_rx(tp, ioaddr);
    while((tp->isr0_interrupt_status & SMC_ISR0_BYTE_RECV) != SMC_ISR0_BYTE_RECV )
    {
        if(smc_dev_card_exist(dev) != SUCCESS)
        {
            return !RET_SUCCESS;
        }
        if(rx_cnt == smc_read_rx(tp, ioaddr))
        {
            osal_task_sleep(1);
            wai_atr_time = osal_get_tick() - wai_atr_time;
            if(wai_atr_tmo>=wai_atr_time)
            {
                wai_atr_tmo -= wai_atr_time;
            }
            else
            {
                SMC_RW_PRINTF("SMC DEV: Wait ATR time out!\n");
                return !RET_SUCCESS;
            }
        }
        else
        {
            rx_cnt = smc_read_rx(tp, ioaddr);
            wai_atr_tmo = (9600*372*2)/(tp->smc_clock/1000);
        }
        wai_atr_time = osal_get_tick();
    }
    //Disable receiver mode
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_RECV)));
    tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
    SMC_RW_PRINTF("SMC DEV: Got card etu is %d, 3etu is %d. \n",INPUT_UINT16(ioaddr + REG_RCNT_ETU), \
            INPUT_UINT16(ioaddr + REG_RCNT_3ETU));

    etu = INPUT_UINT16(ioaddr + REG_RCNT_ETU);

    etu3 = (INPUT_UINT16(ioaddr + REG_RCNT_3ETU)/3-15);
    etu = (etu3<DFT_WORK_ETU)? etu : etu3;

    SMC_RW_PRINTF("SMC DEV: set card etu: %d\n",etu);
    tp->smc_etu = etu;
    OUTPUT_UINT16(ioaddr + REG_ETU0, etu);            //Set right etu

    return RET_SUCCESS;
}

static INT32 smc_config_after_get_atr(struct smc_device *dev)
{
    UINT32 ioaddr = dev->base_addr;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
#ifdef SMC_RW_DEBUG
    UINT32 work_etu = 0;
#endif
    UINT8 t_14 = 14;

    if(1 == tp->T)
    {
        if(tp->D != 0)
        {
            tp->smc_etu = tp->F/tp->D;
        #ifdef SMC_RW_DEBUG
            work_etu = tp->F/tp->D;
            SMC_RW_PRINTF("SMC DEV: working etu : %d\n",work_etu);
        #endif
        }

        osal_interrupt_disable();
        if(tp->inserted != 1)  //card not insert
        {
            osal_interrupt_enable();
            osal_semaphore_release(tp->smc_sema_id);
            return RET_FAILURE;
        }
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP);
        }
        else
        {
            //disable auto parity error pull down control for T=1 card.
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP|0x40);
        }
        osal_interrupt_enable();
    }
    else if(tp->T == t_14)
    {
        osal_interrupt_disable();
        if(tp->inserted != 1)  //card not insert
        {
            osal_interrupt_enable();
            osal_semaphore_release(tp->smc_sema_id);
            return RET_FAILURE;
        }
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP);
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP|0x40);        //disable parity control for T=14 card.
        }
        osal_interrupt_enable();
    }
    else
    {
        if(tp->D != 0)
        {
            tp->smc_etu = tp->F/tp->D;
        #ifdef SMC_RW_DEBUG
            work_etu = tp->F/tp->D;
            SMC_RW_PRINTF("SMC DEV: working etu : %d\n",work_etu);
        #endif
        }
        osal_interrupt_disable();
        if(tp->inserted != 1)  //card not insert
        {
            osal_interrupt_enable();
            osal_semaphore_release(tp->smc_sema_id);
            return RET_FAILURE;
        }
        if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP);
        }
        else
        {
            OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP|0x40);
        }
        //enable parity and auto parity error pull down control.T=0 card.
        //OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_OP|0x40);
        osal_interrupt_enable();
    }

    return RET_SUCCESS;
}

static int smc_dev_reset_with_default_etu(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if(smc_dev_get_card_atr(dev, buffer, atr_size) != SUCCESS)
    {
        if((0==tp->inverse_convention) && (0==tp->ts_auto_detect))
        {
            if(!tp->warm_reset)
            {
                smc_dev_deactive(dev);
                osal_task_sleep(1);
            }
            else
            {
                smc_warm_reset(dev);
                osal_task_sleep(1);
            }
            tp->inverse_convention = 1;
            if(smc_dev_get_card_atr(dev,buffer,atr_size) != SUCCESS)
            {
                tp->inverse_convention = 0;
                reset_err_flag = 1;
            }
        }
        else
        {
            reset_err_flag = 1;
        }
    }

    return ret;
}

static int smc_dev_reset_with_card_etu(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if(SUCCESS == smc_dev_get_card_etu(dev))
    {
        if(!tp->warm_reset)
        {
            smc_dev_deactive(dev);
            osal_task_sleep(1);
        }
        else
        {
            smc_warm_reset(dev);
            osal_task_sleep(1);
        }
        if(smc_dev_get_card_atr(dev,buffer,atr_size) != SUCCESS)
        {
            reset_err_flag = 1;
        }
    }
    else
    {
        reset_err_flag = 1;
    }

    return ret;
}
static int smc_dev_reset_conditional(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if (tp->use_default_etu)
    {
        ret = smc_dev_reset_with_default_etu(dev, buffer, atr_size);
    }
    else
    {
        ret = smc_dev_reset_with_card_etu(dev, buffer, atr_size);
    }

    return ret;
}

static void smc_dev_set_time_unit(struct smartcard_private *tp, UINT32 ioaddr)
{
    UINT32 work_etu = 0;
	UINT32 tmp_etu = 0;

	tmp_etu = tp->smc_etu;
	
    if(tp->use_default_etu)
    {
        tp->smc_etu = tp->default_etu;
    }
    else
    {
        tp->smc_etu = DFT_WORK_ETU;
    }

	if(tp->reset_etu_flag)
	{
		tp->smc_etu = tmp_etu;
	}
	
    work_etu = tp->smc_etu;
    OUTPUT_UINT16(ioaddr + REG_ETU0, work_etu);
    OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12        //set gtr to 12
    OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);
}

static void smc_dev_check_reset_status(struct smartcard_private *tp)
{
    if(1 == tp->reseted)
    {
        tp->reseted = 0;
        if(1 == tp->warm_reset_enable)
        {
            tp->warm_reset = 1;
            SMC_RW_PRINTF("SMC DEV: smartcard has been reseted, Warm Reset!\n");
        }
        else
        {
            SMC_RW_PRINTF("SMC DEV: smartcard has been reseted, Cold Reset!\n");
        }
    }
    else
    {
        tp->warm_reset = 0;
        SMC_RW_PRINTF("SMC DEV: smartcard has not been reseted, Cold Reset!\n");
    }
}



static void smc_dev_reset_default(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if(!tp->warm_reset)
    {
        tp->smc_clock = tp->init_clk_array[tp->init_clk_idx];
        SMC_RW_PRINTF("SMC DEV: try init clk %d, No. %d\n", tp->smc_clock, tp->init_clk_idx);
        smc_set_wclk(tp, ioaddr, tp->smc_clock);
        osal_task_sleep(1);
        smc_dev_deactive(dev);
        osal_task_sleep(1);
    }
    else
    {
        osal_task_sleep(1);
        smc_warm_reset(dev);
        osal_task_sleep(1);
    }
}

static void smc_atr_set_guardtime(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if(0xFF == tp->N)
    {
        if(1 == tp->T)
        {
            OUTPUT_UINT16(ioaddr + REG_GTR0, 11);/* T1 set guart time value.*/
        }
        else
        {
            OUTPUT_UINT16(ioaddr + REG_GTR0, 12);/* T0 set guart time value.*/
        }
    }
    else
    {
        OUTPUT_UINT16(ioaddr + REG_GTR0, 12+tp->N);
    }
}

static void smc_atr_calculate_t1_param_group_one(struct smartcard_private *tp)
{
    /*BGT = 22 etu*/
    tp->T1.BGT =  (22*tp->smc_etu)/(tp->smc_clock/1000);
    /*CWT = (2^CWI + 11) etu*/
    tp->T1.CWT =  tp->cwt;
    /*BWT = (2^BWI*960 + 11)etu */
    /*Attention: BWT in ms, If calc in us, it will overflow for UINT32*/
    tp->T1.BWT= tp->first_cwt;

    /*Add error check type*/
    t1_set_checksum(&tp->T1, tp->error_check_type);
    /*reset the T1 state to undead state*/
    //t1_set_param(&tp->T1, IFD_PROTOCOL_T1_STATE, SENDING);
    t1_set_param(&(tp->T1), IFD_PROTOCOL_T1_STATE, SENDING);
    SMC_RW_PRINTF("SMC DEV: T1 special BGT:%d, CWT:%d, us BWT:%d  ms \n",tp->T1.BGT, tp->T1.CWT, tp->T1.BWT);
}

static void smc_atr_calculate_t1_param_group_two(struct smartcard_private *tp)
{
    tp->cwt =  (((1<<(tp->CWI))+11 )*tp->smc_etu)/(tp->smc_clock/1000) + 1;
    tp->first_cwt = (11*tp->smc_etu)/(tp->smc_clock/1000) \
            +((1<<((UINT32)tp->BWI))*960*ATR_DEFAULT_F)/(tp->smc_clock/1000) + 2;
    /*BGT = 22 etu*/
    tp->T1.BGT =  (22*tp->smc_etu)/(tp->smc_clock/1000);
    /*CWT = (2^CWI + 11) etu*/
    tp->T1.CWT =  tp->cwt;
    /*BWT = (2^BWI*960 + 11)etu, */
    /*Attention: BWT in ms, If calc in us, it will overflow for UINT32*/
    tp->T1.BWT= tp->first_cwt;
    SMC_PRINTF("SMC DEV: T1 disable PPS, CWT %d, BWT %d\n", tp->cwt,  tp->first_cwt);
}

static int smc_atr_check_special_card(struct smartcard_private *tp, UINT8 *buffer)
{
#ifdef SUPPORT_NDS_CARD
    if((0 == tp->T) && (1 == tp->inverse_convention))
    {
        if(((0x7F == buffer[1]) || (0xFF == buffer[1]) || (0xFD == buffer[1])) &&
                ((0x13 == buffer[2]) || (0x11 == buffer[2])))
        {
            tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present = TRUE;
            return 1;
        }
    }
    return 0;
#else
    return 0;
#endif
}

static int smc_atr_is_multi_procotol(struct smartcard_private *tp, UINT8 *protocol_num)
{
    INT32 i = 0;
    INT32 multi_protocol = 0;
    UINT8 t_type = tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
    UINT8 atr_t = 0xff;

    *protocol_num = 0;
    for (i = 0; i < ATR_MAX_PROTOCOLS; i++)
    {
        if (tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present)
        {
            if(0xf == t_type)
            {
                break;
            }
            if(0xFF == atr_t)
            {
                atr_t = t_type;
            }
            else if(atr_t != t_type)
            {
                atr_t = t_type;
                multi_protocol = 1;
            }
        }
    }
    *protocol_num = i;

    return multi_protocol;
}

static inline void smc_atr_reset_etu(struct smartcard_private *tp, UINT32 ioaddr,
        int atr_di, int atr_fi, int protocol_num)
{
    UINT8 bwi = ATR_DEFAULT_BWI;
    UINT8 atr_wi = ATR_DEFAULT_WI;

    if(0 == tp->T)
    {
        if (tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TC].present)
        {
            atr_wi = tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TC].value;
        }

        OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*atr_wi*atr_di);
    }
    else if(1 == tp->T)
    {
        if (tp->atr_info->ib[protocol_num + 1][ATR_INTERFACE_BYTE_TB].present)
        {
            bwi = (tp->atr_info->ib[protocol_num + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
        }
        OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<bwi)*atr_di)/atr_fi));
    }
}

static void reset_err_flag_init(void)
{
	reset_err_flag = 0;
}
static INT32 get_reset_err_flag(void)
{
	return reset_err_flag;
}

/******************************************************************************************************
 *     Name        :    smc_dev_reset()
 *    Description    :    Smart card reset.
 *    Parameter    :    struct smc_device *dev    : Devcie handle.
 *                UINT8 *buffer        : Read data buffer.
 *                UINT16 *atr_size    : ATR data size.
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    struct smartcard_private *tp = NULL;
    UINT32 ioaddr = 0;
    UINT32 first_cwt = 0;
    UINT32 cwt = 0;
    INT32 ret = 0;
    UINT8 need_reset_etu = 0;
    UINT8 atr_fi = 1;
    UINT8 atr_di = 1;
    UINT8 pps0 = 0;
    UINT8 multi_protocol = 0;
    UINT8 diff_f_d = 0;
    UINT8 protocol_num = 0;

    SMC_RW_PRINTF("SMC DEV: in %s\n",__FUNCTION__);
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    ioaddr = dev->base_addr;
    osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);

    t1_init(&tp->T1);
    smc_dev_clear_tx_rx_buf(tp);

    smc_dev_set_time_unit(tp, ioaddr);

    if(0==tp->inserted)
    {
        SMC_RW_PRINTF("SMC DEV: smart card not inserted!\n");
        osal_semaphore_release(tp->smc_sema_id);
        return !RET_SUCCESS;
    }

    smc_dev_check_reset_status(tp);

    tp->isr0_interrupt_status = 0;
    tp->isr1_interrupt_status = 0;
    OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_TRANS_FIFO_EMPY));

    tp->init_clk_idx = 0;
    tp->atr_rlt = SMC_ATR_NONE;

    while(tp->init_clk_idx < tp->init_clk_number)
    {
        smc_dev_reset_default(dev);

        if(0 == tp->inserted)
        {
            SMC_PRINTF("SMC DEV: smart card not inserted!\n");
            osal_semaphore_release(tp->smc_sema_id);
            return !RET_SUCCESS;
        }

		reset_err_flag_init();
        ret = smc_dev_reset_conditional(dev, buffer, atr_size);

        if(get_reset_err_flag())
        {
            if(!tp->warm_reset)
            {
                tp->init_clk_idx++;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    if(get_reset_err_flag())
    {
    	tp->smc_current_select = 0;
        smc_dev_deactive(dev);
        osal_semaphore_release(tp->smc_sema_id);
        return !RET_SUCCESS;
    }
    SMC_DUMP(buffer, *atr_size);
    MEMSET(tp->atr_info, 0, sizeof(atr_t));

    if(SMART_WRONG_ATR==atr_init(tp->atr_info, buffer, *atr_size))
    {
        tp->atr_rlt = SMC_ATR_WRONG;
    }
    else
    {
        tp->atr_rlt = SMC_ATR_OK;
    }

    atr_config_parameter(dev, tp->atr_info);
	
	ret = smc_config_after_get_atr(dev);
    if(ret != RET_SUCCESS)
    {
        return ret;
    }

    smc_atr_set_guardtime(dev);

    if(buffer != tp->atr)
    {
        MEMCPY(tp->atr, buffer, *atr_size);
    }
    /* calculate the bwt,cwt,bgt for T1, in us(1/1000000 second) */
    if (1 == tp->T)
    {
        smc_atr_calculate_t1_param_group_one(tp);
    }

    tp->reseted = 1;
    need_reset_etu = smc_atr_check_special_card(tp, buffer);

    if(FALSE == tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present)
    {
        if((1 == tp->disable_pps) || (1 == pps_no_response_flag)) //20150713_libo remove it because of observation4
         //if(1 == tp->disable_pps)
        {
            pps_no_response_flag = 0; // 20150713_libo remove it because of observation4 
            if(1==tp->T)
            {
                tp->smc_etu = INPUT_UINT16(ioaddr + REG_ETU0);
                smc_atr_calculate_t1_param_group_two(tp);
            }
            osal_task_sleep(10);
            osal_semaphore_release(tp->smc_sema_id);
            return RET_SUCCESS;
        }

        if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
        {
            atr_fi = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
            atr_di =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
            diff_f_d = 1;
        }

        multi_protocol = smc_atr_is_multi_procotol(tp, &protocol_num);

        if(multi_protocol || diff_f_d)
        {
            first_cwt = tp->first_cwt;
            cwt = tp->cwt;
            if (diff_f_d)
            {
                if(1 == tp->T)
                {
                    pps0 |= 0x11;
                }
                else
                {
                    pps0 |= 0x10;
                }
            }
            SMC_RW_PRINTF("SMC DEV: pps0 %d atr_t %d, atr_fi %d, atr_di %d \n ", pps0, tp->T, atr_fi, atr_di);
            tp->cwt = (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000) + 1;
            tp->first_cwt = tp->cwt;

            if(0x11 == (pps0&0x11))/* T1_PPS for BC card */
            {
                atr_fi = 0xa;
                atr_di = 0x5;//125k
                tp->smc_etu = 48;
                SMC_RW_PRINTF("SMC DEV: pps0=0x11, atr_fi=%x ,atr_di=%x\n",atr_fi, atr_di);
            }

            if(RET_SUCCESS == smc_dev_set_pps(dev, pps0|(tp->T&0xf), atr_fi, atr_di, &need_reset_etu))
            {            	
				if(1 == need_reset_etu)
				{
                	tp->first_cwt = first_cwt;
                	tp->cwt = cwt;
				}
				else
				{
					tp->first_cwt = tp->cwt =  (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000);
				}	
                SMC_RW_PRINTF("SMC DEV: PPS OK, set etu as %d\n", tp->smc_etu);
            }
            else
            {            	
				tp->cwt = tp->first_cwt = (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000);
                SMC_RW_PRINTF("SMC DEV: PPS exchange Fail, deactive smartcard!\n");
                pps_no_response_flag = 1;  // 20150713_libo remove it because of observation4
                smc_dev_deactive(dev);
                osal_semaphore_release(tp->smc_sema_id);
                return !RET_SUCCESS;
            }
        }
    }
    else
    {
        SMC_RW_PRINTF("SMC DEV: Specific mode!\n");
        if(!((tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value)&0x10))
        {
            need_reset_etu = 1;
        }
    }
    if(need_reset_etu)
    {
        smc_atr_reset_etu(tp, ioaddr, atr_di, atr_fi, protocol_num);
        OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);
    }
	else
	{
		tp->cwt = tp->first_cwt = (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000);
	}
    osal_task_sleep(10);
    osal_semaphore_release(tp->smc_sema_id);
    return RET_SUCCESS;
}

/******************************************************************************************************
 *     Name        :    smc_dev_multi_class_reset()
 *    Description    :    Smart card reset support class selection.
 *    Parameter    :    struct smc_device *dev    : Devcie handle.
 *                UINT8 *buffer        : Read data buffer.
 *                UINT16 *atr_size    : ATR data size.
 *    Return        :    INT32            : SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_multi_class_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
    INT32 i = 0;
    INT32 ret_code = RET_SUCCESS;
    struct smartcard_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;

    if(tp->smc_current_select)  // which have selected
    {
        if(RET_SUCCESS != smc_dev_reset(dev, buffer, atr_size))
        {
        	tp->smc_current_select = 0;
            smc_dev_deactive(dev);
            ret_code = !RET_SUCCESS;
        }
        return ret_code;
    }

    if(SMC_CLASS_NONE_SELECT == tp->smc_current_select)
    {
        for(i = 0; i<3; i++)
        {
            if(tp->board_supported_class&(1<<i))
            {
                SMC_RW_PRINTF("SMC DEV: first class select %c\n", ('A'+i));
                tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT+i+1);
                break;
            }
        }
    }
    i = ((INT32)tp->smc_current_select -SMC_CLASS_NONE_SELECT -1);
    SMC_RW_PRINTF("SMC DEV: class select, start from %c:\n", ('A'+i));
    for(; i<3; i++)
    {
        if(tp->board_supported_class&(1<<i))
        {
            if(0 == tp->inserted)
            {
                SMC_RW_PRINTF("SMC DEV: smartcard not insert!\n");
                ret_code = !RET_SUCCESS;
                return ret_code;
            }
            tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT+i+1);
            tp->class_select(tp->smc_current_select);
            SMC_RW_PRINTF("SMC DEV: class select, try %c: \n", ('A'+i));

            if(RET_SUCCESS==smc_dev_reset(dev, buffer, atr_size))
            {
                SMC_RW_PRINTF("SMC DEV: class select, smc reset OK!\n");
                if(0==(tp->smc_supported_class&0x7))
                {
                    SMC_RW_PRINTF("SMC DEV: SMC has no class indicator!\n");
                    break;
                }
                else
                {
                    SMC_RW_PRINTF("SMC DEV: SMC has class indicator: %02x!\n", (tp->smc_supported_class&0x7));
                    if(tp->smc_supported_class&(1<<i))
                    {
                        SMC_RW_PRINTF("SMC DEV: SMC class selection match at %c !\n", ('A'+i));
                        break;
                    }
                }
            }
            else
            {
                SMC_RW_PRINTF("SMC DEV: class select failed!\n");
                smc_dev_deactive(dev);
                //osal_task_sleep(10);
            }

        }
    }
    if(i>2)
    {
        tp->smc_current_select = SMC_CLASS_NONE_SELECT;
        ret_code = !RET_SUCCESS;
    }
    return ret_code;
}

