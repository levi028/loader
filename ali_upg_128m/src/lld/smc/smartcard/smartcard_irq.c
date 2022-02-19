/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_irq.c
*
*    Description: This file contains all globe micros and functions declare
*                     of smartcard interrupt.
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
#include "smartcard.h"
#include "smartcard_txrx.h"

void smc_dev_hsr_0(UINT32 param)
{
    struct smc_device *dev = smc_dev_set[0].dev;
    struct smartcard_private *tp = NULL;

    if(!dev)
    {
        return;
    }
    tp = (struct smartcard_private *)(dev->priv);
    if(param&SMC_INSERTED)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
    }
    if(param&SMC_REMOVED)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
    }
    osal_flag_set(tp->smc_flag_id, param);
}

void smc_dev_hsr_1(UINT32 param)
{
    struct smc_device *dev = smc_dev_set[1].dev;
    struct smartcard_private *tp = NULL;

    if(!dev)
    {
        return;
    }
    tp = (struct smartcard_private *)(dev->priv);
    if(param&SMC_INSERTED)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
    }
    if(param&SMC_REMOVED)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
    }
    osal_flag_set(tp->smc_flag_id, param);
}
static void smc_irq_isr0_bytes_receive(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;
    UINT32 i = 0;
#ifdef SMC_RW_DEBUG
    volatile UINT8 patch = 0;
#endif
    UINT16 c = 0;
    UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received
	
	
    //SMC_PRINTF("byte : %d\n", smc_read_rx(tp, ioaddr));
	/* This patch will fix only M3603 IC issue: 
		RX PE ERROR will discard correct byte. */
    if(tp->isr0_interrupt_status&SMC_ISR0_PE_RECV)
    {        
        if(smc_read_rx(tp, ioaddr))
        {
        #ifdef SMC_RW_DEBUG
            patch = INPUT_UINT8(ioaddr + REG_RBR);
        #endif
            tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
        #ifdef SMC_RW_DEBUG
            SMC_RW_PRINTF("RX Parity Error, lose byte: 0x%x\n",patch);
        #endif
        }
    }
    if((0 != tp->smc_rx_tail)&&(0 == tp->got_first_byte))
    {
        tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
        tp->got_first_byte = 1;
        //OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_BYTE_RECV_TRIG));
        osal_interrupt_register_hsr(tp->hsr, SMC_RX_BYTE_RCV);
    }
    if(tp->smc_rx_buf == tp->smc_rx_tmp_buf)
    {
        rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received
        c = smc_read_rx(tp, ioaddr);//how many data in FIFO
        //soc_printf("rcv %d bytes\n", c);
        c = (c<=rem_space?c:rem_space);

        for(i = 0; i < c; i++)
        {
            tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);//read data from register to buf
        }

        tp->smc_rx_head += c;//refresh have received data number
        tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
        //SMC_PRINTF("rcv %d bytes\n", tp->smc_rx_head);
    }
}

static void smc_irq_isr0_fifo_receive(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;
    UINT32 i = 0;
    UINT16 c = 0;
    UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received

    tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
    if((tp->smc_rx_tail) && (tp->smc_rx_buf!=tp->smc_rx_tmp_buf))
    {
        c =smc_read_rx(tp, ioaddr);//how many data in FIFO
        c = (c<=rem_space?c:rem_space);
        for(i=0;i<c;i++)
        {
            tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);//read data from register to buf
        }
        tp->smc_rx_head += c;//refresh have received data number

        if(tp->smc_rx_head == tp->smc_rx_tail)//received finished
        {
            //SMC_PRINTF("Notify rx over: %d\n", tp->smc_rx_tail);
            //tp->smc_flag_ptn = SMC_RX_FINISHED;
            osal_interrupt_register_hsr(tp->hsr, SMC_RX_FINISHED);
        }
        else
        {
            rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received
            if(rem_space/tp->smc_rx_fifo_size)
            {
                smc_write_rx(tp, ioaddr, 32);
            }
            else
            {
                smc_write_rx(tp, ioaddr, rem_space);
            }
            SMC_PRINTF("SMC IRQ: continue rx %d data\n", rem_space);
        }
    }
}

static int smc_irq_isr0_fifo_transfer_force_loop(struct smc_device *dev)
{
    UINT32 force_loop_tmo = 0;
    UINT8 force_tx_rx = 1;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    force_loop_tmo = (smc_read_tx(tp, ioaddr)+1) * tp->char_frm_dura;
    //if(0==smc_read_tx(tp, ioaddr))libc_libc_printf("tmo2: too late!\n");
    while(0 != smc_read_tx(tp, ioaddr))
    {
        osal_delay(1);
        force_loop_tmo--;
        if(!force_loop_tmo)
        {
            force_tx_rx = 0;
            SMC_PRINTF("SMC IRQ: tmo2: tmo %d\n", smc_read_tx(tp, ioaddr));
            break;
        }

        if(0 == (INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80))
        {
            SMC_PRINTF("SMC IRQ: smart card not inserted!\n");
            return !RET_SUCCESS;
        }
    }
    if(force_tx_rx)
    {
        OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS)) \
                                            | SMC_SCR_CTRL_RECV);
    }
    //SMC_PRINTF("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(sys_ic_get_cpu_clock()/2));
                    //libc_printf("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));

    return RET_SUCCESS;
}

static void smc_irq_isr0_fifo_transfer_auto_loop(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if(!(tp->auto_tx_rx_triger))
    {
        OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
    }
    /*****************************************/
    /* TO DO: Notify SMC_TX_FINISHED message */
    /*****************************************/
    osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
    if(ALI_S3602 == tp->smc_chip_id)
    {
        tp->the_last_send = 0;
        tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
    }
}

static int smc_irq_isr0_fifo_transfer_end(struct smc_device *dev)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    //SMC_PRINTF("tx finish:\n");
    /*Once TX finished, set interface device to RX mode immediately*/
    if((tp->force_tx_rx_triger) && (1==tp->force_tx_rx_state))
    {
        smc_write_tx(tp, ioaddr, tp->force_tx_rx_thld);
        tp->force_tx_rx_state = 2;
        if(0 == smc_read_tx(tp, ioaddr))
        {
            OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
            SMC_PRINTF("SMC IRQ: tmo2 too late!\n");
        }
        else
        {
            ret = smc_irq_isr0_fifo_transfer_force_loop(dev);
        }
    }
    else
    {
        smc_irq_isr0_fifo_transfer_auto_loop(dev);
    }
    return ret;
}

static void smc_irq_isr0_fifo_transfer_force_size(struct smc_device *dev, UINT32 prepare_size)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if (0==smc_read_tx(tp, ioaddr))
    {
        if (prepare_size > tp->force_tx_rx_thld)
        {
            smc_write_tx(tp, ioaddr, prepare_size - tp->force_tx_rx_thld);
            tp->force_tx_rx_state = 1;
        }
        else
        {
            smc_write_tx(tp, ioaddr, prepare_size);
            tp->force_tx_rx_state = 2;
        }
    }
    else
    {
        smc_write_tx(tp, ioaddr,
                     (prepare_size + (tp->smc_tx_fifo_size>>1)) - tp->force_tx_rx_thld);
        tp->force_tx_rx_state = 1;
    }
}


static void smc_irq_isr0_fifo_transfer_auto_size(struct smc_device *dev, UINT32 prepare_size)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;


    if(ALI_S3602 == tp->smc_chip_id)
    {
        OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0) \
                                |(prepare_size + (tp->smc_tx_fifo_size>>1)));
        tp->the_last_send = 1;
        tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
    }
    else
    {
        if(0 == smc_read_tx(tp, ioaddr))
        {
            smc_write_tx(tp, ioaddr,prepare_size);
        }
        else
        {
            smc_write_tx(tp, ioaddr,prepare_size + (tp->smc_tx_fifo_size>>1));
        }
    }
}

static UINT32 smc_irq_isr0_fifo_transfer_size_config(struct smc_device *dev, UINT32 prepare_size)
{
    UINT32 size = prepare_size;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if (size > (tp->smc_tx_fifo_size>>1))
    {
        size = tp->smc_tx_fifo_size>>1;
        if(ALI_S3602 == tp->smc_chip_id)
        {
            OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
        }
        else
        {
            smc_write_tx(tp, ioaddr, size);
        }
    }
    else
    {
        if(tp->force_tx_rx_triger)
        {
            smc_irq_isr0_fifo_transfer_force_size(dev, size);
        }
        else
        {
            smc_irq_isr0_fifo_transfer_auto_size(dev, size);
        }
    }

    return size;
}

static void smc_irq_isr0_fifo_transfer_action(struct smc_device *dev, UINT32 size)
{
    UINT32 i = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    for(i = 0; i < size; i++)
    {
        if((tp->smc_tx_rd+i+1) == tp->smc_tx_wr)
        {
            if(tp->auto_tx_rx_triger)
            {
                OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); /* tx->rx auto switch */
                SMC_PRINTF("SMC IRQ: 2, tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, i, tp->smc_tx_wr);
            }
        }
        OUTPUT_UINT8(ioaddr + REG_THR, tp->smc_tx_buf[tp->smc_tx_rd+i]);
    }
    tp->smc_tx_rd += size;
    SMC_PRINTF("SMC IRQ: continue feed data %d \n", size);

}


static int smc_irq_isr0_fifo_transfer_start(struct smc_device *dev)
{
    int ret = 0;
    int num_2 = 2;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 size = tp->smc_tx_wr - tp->smc_tx_rd;

    size = smc_irq_isr0_fifo_transfer_size_config(dev, size);
    smc_irq_isr0_fifo_transfer_action(dev, size);

    if ((tp->smc_tx_rd == tp->smc_tx_wr) && \
        (1 == tp->force_tx_rx_triger) && \
        (tp->force_tx_rx_state == num_2))
    {
        ret = smc_irq_isr0_fifo_transfer_force_loop(dev);
    }

    return ret;
}

static int smc_irq_isr0_fifo_transfer(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    int ret = 0;

    tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_TRANS);

    if (tp->smc_tx_wr)
    {
        if(tp->smc_tx_rd == tp->smc_tx_wr)
        {
            ret = smc_irq_isr0_fifo_transfer_end(dev);
        }
        else
        {
            ret = smc_irq_isr0_fifo_transfer_start(dev);
        }
    }
    return ret;
}

static void smc_irq_isr0_fifo_empty(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if(!(tp->auto_tx_rx_triger))
    {
        OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS)) \
                                           | SMC_SCR_CTRL_RECV);
    }
    /*****************************************/
    /* TO DO: Notify SMC_TX_FINISHED message */
    /*****************************************/
    osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
    tp->the_last_send = 0;
    tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
}

static INT32 smc_irq_isr0_process(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT8 isr0_status = 0;
    UINT32 ioaddr = dev->base_addr;
    INT32 ret = 0;

    isr0_status = INPUT_UINT8(ioaddr + REG_ISR0);
    tp->isr0_interrupt_status |= isr0_status;
    OUTPUT_UINT8(ioaddr + REG_ISR0, isr0_status);

    if(isr0_status&SMC_ISR0_BYTE_RECV)
    {
        smc_irq_isr0_bytes_receive(dev);
    }

    if((isr0_status&SMC_ISR0_FIFO_TRANS)&&(tp->force_tx_rx_triger||tp->auto_tx_rx_triger))
    {
        ret = smc_irq_isr0_fifo_transfer(dev);
    }

    if(isr0_status& SMC_ISR0_FIFO_RECV)
    {
        smc_irq_isr0_fifo_receive(dev);
    }

    if((ALI_S3602==tp->smc_chip_id)&& \
      (isr0_status& SMC_ISR0_FIFO_EMPTY)&& \
      (1==tp->the_last_send))
    {
        smc_irq_isr0_fifo_empty(dev);
    }

    return ret;
}
/******************************************************************************************************
 *     Name        :    smc_dev_interrupt()
 *    Description    :    smart card reader controler interrupt handle.
 *    Parameter    :    struct smc_device *dev    : Devcie handle.
 *    Return        :    none
 *
 ******************************************************************************************************/
void smc_dev_interrupt(UINT32 param)
{
    struct smc_device *dev = (struct smc_device *)param;
    struct smartcard_private *tp = (struct smartcard_private *)((struct smc_device *)param)->priv;
    UINT32 ioaddr = ((struct smc_device *)param)->base_addr;
    UINT8 isr1_status = 0;
    INT32 cls = 0;
    INT32 ret = 0;
	
	ret = smc_irq_isr0_process(dev);
    if (ret != RET_SUCCESS)
    {
        SMC_RW_PRINTF("SMC IRQ: ISR0 errno %d\n", ret);
        return ;
    }

    isr1_status = INPUT_UINT8(ioaddr + REG_ISR1);
    tp->isr1_interrupt_status |= isr1_status;
    //SMC_PRINTF("isr0: %02x, isr1: %02x\n", isr0_status, isr1_status);
    OUTPUT_UINT8(ioaddr + REG_ISR1, isr1_status);

    if((isr1_status & SMC_ISR1_CARD_INSERT) != 0)
    {
        tp->inserted = 1;
        tp->atr_rlt = SMC_ATR_NONE;
        tp->reseted = 0;
		tp->reset_etu_flag = 0;
		
        osal_interrupt_register_hsr(tp->hsr, SMC_INSERTED);
        SMC_PRINTF("SMC IRQ: smart card inserted!\n");
    }
    else if((isr1_status & SMC_ISR1_CARD_REMOVE) != 0)
    {
        tp->inserted = 0;
        tp->reseted = 0;
        tp->atr_rlt = SMC_ATR_NONE;
        tp->smc_supported_class = 0;
        tp->smc_current_select = SMC_CLASS_NONE_SELECT;
        //smc_dev_deactive((struct smc_device *)param);
        tp->reseted = 0;
        if(tp->class_selection_supported)
        {
            for(cls = 0; cls<3; cls++)
            {
                if(tp->board_supported_class&(1<<cls))
                {
                    SMC_PRINTF("SMC IRQ: smc remove, set class to %c\n", ('A'+cls));
                    tp->class_select((enum class_selection)(SMC_CLASS_NONE_SELECT+cls+1));
                    break;
                }
             }
        }
        smc_init_hw((struct smc_device *)param);
        osal_interrupt_register_hsr(tp->hsr, SMC_REMOVED);
        SMC_PRINTF("SMC IRQ: smart card removed!\n");

    }
    else
    {
        return;
    }

    if (((struct smc_device *)param)->callback != NULL)
    {
        osal_interrupt_register_hsr((OSAL_T_HSR_PROC_FUNC_PTR)(((struct smc_device *)param)->callback), tp->inserted);
    }
    return;
}

