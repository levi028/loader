/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
 *    disclosed to unauthorized individual.
 *
 *    File: smartcard_txrx.c
 *
 *    Description: This file contains all globe micros and functions declare
 *                     of smartcard transmission.
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
#include <asm/chip.h>
#include <api/libc/string.h>

#include "smartcard_txrx.h"

#define GET_UNKOWN_SW_BYTE 0//if you want to get unknow sw1, you should set this to 1,  refer to bug #36967

void smc_dev_clear_tx_rx_buf(struct smartcard_private *tp)
{
    tp->smc_tx_buf = NULL;
    tp->smc_rx_buf = NULL;
    tp->smc_tx_rd = 0;
    tp->smc_tx_wr = 0;
    tp->smc_rx_head = 0;
    tp->got_first_byte = 0;
    tp->smc_rx_tail = 0;
}

void smc_write_tx(struct smartcard_private *tp, UINT32 io_base, UINT16 val)
{
    if(tp->smc_rx_fifo_size>SMC_RX_FIFO_SIZE_64)
    {
        OUTPUT_UINT16(io_base + REG_TX_CNT, val);
    }
    else
    {
        OUTPUT_UINT8(io_base + REG_TX_CNT, ((UINT8)val));
    }
}

UINT16 smc_read_tx(struct smartcard_private *tp, UINT32 io_base)
{
    UINT16 val = 0;

    if(tp->smc_rx_fifo_size>SMC_RX_FIFO_SIZE_64)
    {
        val = INPUT_UINT16(io_base+ REG_TX_CNT);
    }
    else
    {
        val = INPUT_UINT8(io_base + REG_TX_CNT);
    }
    return val;
}

void smc_write_rx(struct smartcard_private *tp, UINT32 io_base, UINT16 val)
{
    if(tp->smc_rx_fifo_size>SMC_RX_FIFO_SIZE_64)
    {
        WRITE_RX_CNT(io_base, val);
    }
    else
    {
        OUTPUT_UINT8(io_base + REG_RFIFO_CNT, ((UINT8)val));
    }
}
UINT16 smc_read_rx(struct smartcard_private *tp, UINT32 io_base)
{
    UINT16 val = 0;

    if(tp->smc_rx_fifo_size>SMC_RX_FIFO_SIZE_64)
    {
        val = READ_RX_CNT(io_base);
    }
    else
    {
        val = INPUT_UINT8(io_base + REG_RFIFO_CNT);
    }
    return val;
}

/******************************************************************************************************
 *     Name        :   smc_dev_write()
 *    Description    :   UART CAM write.
 *    Parameter    :    struct smc_device *dev    : Devcie handle
 *                    UINT8 *buffer            : Wrtie data buffer
 *                    UINT16 size                : Write data size
 *                    UINT16 *actsize            : Write data size
 *    Return        :    INT32                    : return value
 *
 ******************************************************************************************************/
INT32 smc_dev_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{
    //    SMC_PRINTF("%s\n", __FUNCTION__);
    struct smartcard_private *tp = NULL;
    INT32 rlt = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)(dev->priv);
    osal_task_sleep(3);//Patch for S3602F
    osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
#ifdef SMC_RW_DEBUG
    UINT32 wr_tick = 0;

    wr_tick = osal_get_tick();
    SMC_RW_PRINTF("%d\n", osal_get_tick()-wr_tick);
    INT16 len = size;
    INT16 k = 0;
    INT16 len_5 = 5;

    if(len>len_5)
    {
        len = 5;
    }
    SMC_RW_PRINTF("%d\t \tW: ", size);
    for(k=0; k<len; k++)
    {
        SMC_RW_PRINTF("%02x ", buffer[k]);
    }
    if(size>len_5)
    {
        SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4], buffer[size-3], buffer[size-2], buffer[size-1]);
    }
    SMC_RW_PRINTF("\n");
#endif
    rlt = smc_dev_transfer_data(dev, buffer, size, (UINT8 *)tp->smc_rx_tmp_buf, 0, (UINT16 *)actsize);
    osal_semaphore_release(tp->smc_sema_id);
    SMC_PRINTF("%s OVER: %d\n", __FUNCTION__, osal_get_tick() - wr_tick);
    return rlt;
}

/******************************************************************************************************
 *     Name        :   smc_dev_read()
 *    Description    :   UART smartcard read.
 *    Parameter    :    struct smc_device *dev    : Devcie handle
 *                    UINT8 *buffer            : Read data buffer
 *                    INT16 size                : Read data size
 *                    INT16 *actsize            : Read data size
 *    Return        :    INT32                    : return value
 *
 ******************************************************************************************************/
INT32 smc_dev_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{
    struct smartcard_private *tp = NULL;
    UINT32 smc_rd_tmo = 0;
    UINT32 cur_rx_head = 0;
    UINT32 tmp_rd_tick= 0;
    //SMC_PRINTF("%s\n", __FUNCTION__);
    UINT32 cpu_clk = 0;
    UINT32 real_clk = 24;
    //UINT32 len_26 = 26;
    UINT32 rem_space = 0;
    UINT32 i = 0;
    UINT16 c = 0;
    UINT16 total_cnt = 0;
    //UINT32 start_t,end_t;
    
    //start_t=osal_get_tick();
		
    cpu_clk = sys_ic_get_cpu_clock();	
	
    if (NULL == dev)
    {
        return ERR_PARA;
    }
	
    tp = (struct smartcard_private *)dev->priv;
    rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received
    smc_rd_tmo = tp->first_cwt;
	
	if((ALI_S3281==tp->smc_chip_id))
	{
    	tmp_rd_tick = read_tsc(); //osal_get_tick();    	

	}
	else
	{
		tmp_rd_tick = osal_get_tick();
		SMC_PRINTF("SMC TXRX: in %s, line=%d, smc_rd_tmo=%d, tmp_rd_tick=%d, size=%d!\n", __FUNCTION__, __LINE__, smc_rd_tmo, tmp_rd_tick, size);
	}
	
    if((ALI_S3281==tp->smc_chip_id))
    {
        if(CPU_CLK_308 == cpu_clk)
        {
            real_clk = 24;
        }
        else if(CPU_CLK_270 == cpu_clk)
        {
            real_clk = 21;
        }
        else if(CPU_CLK_180 == cpu_clk)
        {
            real_clk = 14;
        }
        else if(CPU_CLK_90 == cpu_clk)
        {
            real_clk = 7;
        }
        else if(CPU_CLK_300 == cpu_clk)
        {
            real_clk = 24;
        }
        else
        {
            real_clk = 21;
        }
    }

    osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
    if(smc_read_rx(tp, dev->base_addr))
    {
        osal_interrupt_disable();
        c =smc_read_rx(tp, dev->base_addr);//how many data in FIFO
        total_cnt = c;
        c = (c<=rem_space?c:rem_space);
        total_cnt -= c;
        for(i=0;i<c;i++)
        {
            tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(dev->base_addr + REG_RBR);//read data from register to buf
        }
        tp->smc_rx_head += c;//refresh have received data number
        osal_interrupt_enable();
    }
    cur_rx_head = tp->smc_rx_head;
    if(cur_rx_head)
    {
        smc_rd_tmo = tp->cwt;
    }
    /*
       while((tp->smc_rx_head<((UINT32)size+2) && size >1) || \
       (tp->smc_rx_head == 0 && size == 1))
     */
    SMC_PRINTF("SMC TXRX: in %s, line=%d, tp->smc_rx_head=%d!\n", __FUNCTION__, __LINE__, tp->smc_rx_head);
    while(tp->smc_rx_head<(UINT32)size)
    {
        if(0 == tp->inserted)
        {
            SMC_PRINTF("SMC TXRX: in %s, line=%d, smart card not inserted!\n", __FUNCTION__, __LINE__);
            osal_semaphore_release(tp->smc_sema_id);
            return SMART_NOT_INSERTED;
        }
        else if(tp->reseted != 1)
        {
            SMC_PRINTF("SMC TXRX: in %s, line=%d, smart card not reseted!\n", __FUNCTION__, __LINE__);
            osal_semaphore_release(tp->smc_sema_id);
            return SMART_NOT_RESETED;
        }
        osal_task_sleep(1);
        /*
            smc_rd_tmo--;
            if(!smc_rd_tmo)
            break;
         */
        if(cur_rx_head!=tp->smc_rx_head)
        {
            cur_rx_head = tp->smc_rx_head;
            smc_rd_tmo = tp->cwt;
            //tmp_rd_tick = read_tsc();
            if((ALI_S3281==tp->smc_chip_id))
			{
		    	tmp_rd_tick = read_tsc(); //osal_get_tick();		
			}
			else
			{
				tmp_rd_tick = osal_get_tick();
				SMC_PRINTF("SMC TXRX: in %s, line=%d, smc_rd_tmo=%d, tmp_rd_tick=%d!\n", __FUNCTION__, __LINE__, smc_rd_tmo, tmp_rd_tick);
			}
        }

        if(1 == tp->T)
        {
            if((INT32)tp->smc_rx_head == tp->smc_rx_tmp_buf[2]+3+tp->T1.rc_bytes)
            {
                SMC_PRINTF("SMC TXRX:tp->smc_rx_head=%d, tp->smc_rx_tmp_buf[2]=%d, T1.rc_bytes=%d\n", tp->smc_rx_head, tp->smc_rx_tmp_buf[2], tp->T1.rc_bytes);
                break;
            }
        }
        if((ALI_S3281==tp->smc_chip_id))
        {
            if((smc_rd_tmo*1000)<((read_tsc()-tmp_rd_tick)/(540*real_clk)*84+1))
            {
                //libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
                smc_rd_tmo = 0;
                break;
            }
        }
        else
        {
            //if((smc_rd_tmo*1000)<(read_tsc()-tmp_rd_tick)/(cpu_clk/2))
			if((smc_rd_tmo+5)<(osal_get_tick()-tmp_rd_tick))
			{
				SMC_PRINTF("SMC TXRX: in %s, line=%d, smc_rd_tmo=%d, osal_get_tick()=%d!\n", __FUNCTION__, __LINE__, smc_rd_tmo, osal_get_tick());
                //libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
                smc_rd_tmo = 0;
                break;
            }
        }
    }

    osal_interrupt_disable();
	SMC_PRINTF("[%s %d], size=%d, tp->smc_rx_head=%d\n", __FUNCTION__, __LINE__, size, tp->smc_rx_head);
    if(((UINT32)size)>tp->smc_rx_head)
    {
        size = tp->smc_rx_head;
    }
    //if(tp->smc_rx_tmp_buf[0]==0x60)ASSERT(0);

    MEMCPY(buffer, (const void *)tp->smc_rx_tmp_buf, size);
    *actsize = size;
    tp->smc_rx_head -= (UINT32)size;
    if(tp->smc_rx_head)
    {
        MEMCPY((void *)tp->smc_rx_tmp_buf, (const void *)(tp->smc_rx_tmp_buf+size), tp->smc_rx_head);
    }
    osal_interrupt_enable();

	//end_t = osal_get_tick();
    if((0==smc_rd_tmo) && (0==(*actsize)))
    {
        tp->smc_current_select = 0;
        //SMC_PRINTF("SMC TXRX: in[%s]line=%d, read %d bytes TMO with %dms\n", __FUNCTION__, __LINE__, size, end_t-start_t);
        osal_semaphore_release(tp->smc_sema_id);
        return SMART_NO_ANSWER;
    }
#ifdef SMC_RW_DEBUG
    INT16 len = *actsize;
    INT16 len_26 = 26;
    INT16 k = 0;

    SMC_RW_PRINTF("%d\t \tR: ", size);
    size = *actsize;
    if(len > len_26)
    {
        len = 26;
    }
    for(k=0; k<len; k++)
    {
        SMC_RW_PRINTF("%02x ", buffer[k]);
    }
    if(size>len_26)
    {
        SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4],buffer[size-3], buffer[size-2], buffer[size-1]);
    }
    SMC_RW_PRINTF("\n");
#endif
   // SMC_PRINTF("SMC TXRX: in[%s]line=%d, read %d bytes with %dms!\n", __FUNCTION__, __LINE__, *actsize, end_t-start_t);
    osal_semaphore_release(tp->smc_sema_id);
    return RET_SUCCESS;
}


/******************************************************************************************************
 *    Name        :   pps_check()
 *    Description    :   checking for pps.
 *    Parameter    :    
 *                    UINT8 *buffer            : reponse pps data buffer
 *                    UINT32 len                : reponse pps data length               
 *    Return        :    INT16                    : pps length to get
 *
 ******************************************************************************************************/
INT16 pps_check(volatile UINT8 *buffer, UINT32 len)
{       
        UINT32 i = 0;
        INT16 pps_size = 6;                
         

        if ((NULL == buffer) || (0 == len))
        {
                SMC_PRINTF("[ %s %d ], error!\n", __FUNCTION__, __LINE__);
                return (-1);
        }    

        for (i=0; i<len; i++)
        {
            SMC_PRINTF("%02x ", buffer[i]);
        }
        SMC_PRINTF("\n");

        for (i=0; i<len; i++)
        {
            if (0 == i)
            {
                if (0xFF != buffer[i])
                {
                    SMC_PRINTF("[ %s %d ], error!\n", __FUNCTION__, __LINE__);
                    return (-1);
                }
            }
            else if (1 == i)
            {                 
                   if (0 == (buffer[i] & 0x10))
                   {
                        pps_size--;
                        SMC_PRINTF("[ %s %d ], pps_size = %d\n", __FUNCTION__, __LINE__, pps_size);
                   }
                   if (0 == (buffer[i] & 0x20))                   
                   {
                        pps_size--;
                        SMC_PRINTF("[ %s %d ], pps_size = %d\n", __FUNCTION__, __LINE__, pps_size);
                   }
                   if (0 == (buffer[i] & 0x40))
                   {
                        pps_size--;
                        SMC_PRINTF("[ %s %d ], pps_size = %d\n", __FUNCTION__, __LINE__, pps_size);
                   }                          
            }
            else
            {
                break;
            }
        }

        SMC_PRINTF("[ %s %d ], pps_size = %d\n", __FUNCTION__, __LINE__, pps_size);

        return pps_size;      
}


/******************************************************************************************************
 *     Name        :   smc_dev_pps_write()
 *    Description    :   UART CAM write only for pps
 *    Parameter    :    struct smc_device *dev    : Devcie handle
 *                    UINT8 *buffer            : Wrtie data buffer
 *                    UINT16 size                : Write data size
 *                    UINT16 *actsize            : Write data size
 *    Return        :    INT32                    : return value
 *
 ******************************************************************************************************/

INT32 smc_dev_pps_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{    
    struct smartcard_private *tp = NULL;
    INT32 rlt = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)(dev->priv);
    osal_task_sleep(3);//Patch for S3602F
    //osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
#ifdef SMC_RW_DEBUG
    UINT32 wr_tick = 0;

    wr_tick = osal_get_tick();
    SMC_RW_PRINTF("%d\n", osal_get_tick()-wr_tick);
    INT16 len = size;
    INT16 k = 0;
    INT16 len_5 = 5;

    if(len>len_5)
    {
        len = 5;
    }
    SMC_RW_PRINTF("%d\t \tW: ", size);
    for(k=0; k<len; k++)
    {
        SMC_RW_PRINTF("%02x ", buffer[k]);
    }
    if(size>len_5)
    {
        SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4], buffer[size-3], buffer[size-2], buffer[size-1]);
    }
    SMC_RW_PRINTF("\n");
#endif
    rlt = smc_dev_transfer_data(dev, buffer, size, (UINT8 *)tp->smc_rx_tmp_buf, 0, (UINT16 *)actsize);
    //osal_semaphore_release(tp->smc_sema_id);
    SMC_PRINTF("%s OVER: %d\n", __FUNCTION__, osal_get_tick() - wr_tick);
    return rlt;
}


/******************************************************************************************************
 *     Name        :   smc_dev_pps_read()
 *    Description    :   UART smartcard read only for pps.
 *    Parameter    :    struct smc_device *dev    : Devcie handle
 *                    UINT8 *buffer            : Read data buffer
 *                    INT16 size                : Read data size
 *                    INT16 *actsize            : Read data size
 *    Return        :    INT32                    : return value
 *
 ******************************************************************************************************/
INT16 smc_dev_pps_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{
    struct smartcard_private *tp = NULL;
    UINT32 smc_rd_tmo = 0;
    UINT32 cur_rx_head = 0;
    UINT32 tmp_rd_tick= 0;    
    UINT32 cpu_clk = 0;
    UINT32 real_clk = 24;   
    UINT32 rem_space = 0;
    UINT32 i = 0;
    UINT16 c = 0;
    UINT16 total_cnt = 0;
    INT16 pps_size = 0;
    //UINT32 start_t,end_t;
    
    //start_t=osal_get_tick();
    cpu_clk = sys_ic_get_cpu_clock();
    tmp_rd_tick = read_tsc(); //osal_get_tick();

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    rem_space = tp->smc_rx_tail - tp->smc_rx_head;//how many data should be received
    smc_rd_tmo = tp->first_cwt;
    if((ALI_S3281==tp->smc_chip_id))
    {
        if(CPU_CLK_308 == cpu_clk)
        {
            real_clk = 24;
        }
        else if(CPU_CLK_270 == cpu_clk)
        {
            real_clk = 21;
        }
        else if(CPU_CLK_180 == cpu_clk)
        {
            real_clk = 14;
        }
        else if(CPU_CLK_90 == cpu_clk)
        {
            real_clk = 7;
        }
        else if(CPU_CLK_300 == cpu_clk)
        {
            real_clk = 24;
        }
        else
        {
            real_clk = 21;
        }
    }

    //osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
    if(smc_read_rx(tp, dev->base_addr))
    {
        osal_interrupt_disable();
        c =smc_read_rx(tp, dev->base_addr);//how many data in FIFO
        total_cnt = c;
        c = (c<=rem_space?c:rem_space);
        total_cnt -= c;
        for(i=0;i<c;i++)
        {
            tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(dev->base_addr + REG_RBR);//read data from register to buf
        }
        tp->smc_rx_head += c;//refresh have received data number
        osal_interrupt_enable();
    }
    cur_rx_head = tp->smc_rx_head;
    if(cur_rx_head)
    {
        smc_rd_tmo = tp->cwt;
    }

    pps_size = pps_check(tp->smc_rx_buf, cur_rx_head);
    if (-1 != pps_size)
    {
        if (size > pps_size)
        {
                size = pps_size;
                SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
        }
    }
    
    while(tp->smc_rx_head<(UINT32)size)
    {
        SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
        
        if(0 == tp->inserted)
        {
            SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
            //osal_semaphore_release(tp->smc_sema_id);
            return SMART_NOT_INSERTED;
        }
        else if(tp->reseted != 1)
        {
            SMC_PRINTF("SMC TXRX: smart card not reseted!\n");
            //osal_semaphore_release(tp->smc_sema_id);
            return SMART_NOT_RESETED;
        }
        osal_task_sleep(1);
        
        if(cur_rx_head!=tp->smc_rx_head)
        {
            cur_rx_head = tp->smc_rx_head;
            smc_rd_tmo = tp->cwt;
            tmp_rd_tick = read_tsc();
            
            pps_size = pps_check(tp->smc_rx_buf, cur_rx_head);
            if (-1 != pps_size)
            {
                if (size > pps_size)
                {
                        size = pps_size;
                        SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
                }
            }
        }

        if(1 == tp->T)
        {
            if((INT32)tp->smc_rx_head == tp->smc_rx_tmp_buf[2]+3+tp->T1.rc_bytes)
            {
                SMC_PRINTF("SMC TXRX: len=%d\n",tp->smc_rx_tmp_buf[2]);
                break;
            }
        }
        if((ALI_S3281==tp->smc_chip_id))
        {
            if((smc_rd_tmo*1000)<((read_tsc()-tmp_rd_tick)/(540*real_clk)*84+1))
            {
                //libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
                smc_rd_tmo = 0;
                break;
            }
        }
        else
        {
            if((smc_rd_tmo*1000)<(read_tsc()-tmp_rd_tick)/(cpu_clk/2))
            {
                //libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
                smc_rd_tmo = 0;
                break;
            }
        }
    }

    osal_interrupt_disable();
    if(((UINT32)size)>tp->smc_rx_head)
    {
        size = tp->smc_rx_head;
    }  

    MEMCPY(buffer, (const void *)tp->smc_rx_tmp_buf, size);
    *actsize = size;
    tp->smc_rx_head -= (UINT32)size;
    if(tp->smc_rx_head)
    {
        MEMCPY((void *)tp->smc_rx_tmp_buf, (const void *)(tp->smc_rx_tmp_buf+size), tp->smc_rx_head);
    }
    osal_interrupt_enable();
	//end_t=osal_get_tick();
    if((0==smc_rd_tmo) &&	(0==(*actsize)))
    {
    	tp->smc_current_select = 0;
        //SMC_PRINTF("SMC TXRX: read %d bytes TMO with %d!\n", size, start_t-end_t);
        //osal_semaphore_release(tp->smc_sema_id);
        return SMART_NO_ANSWER;
    }
#ifdef SMC_RW_DEBUG
    INT16 len = *actsize;
    INT16 len_26 = 26;
    INT16 k = 0;

    SMC_RW_PRINTF("%d\t \tR: ", size);
    size = *actsize;
    if(len > len_26)
    {
        len = 26;
    }
    for(k=0; k<len; k++)
    {
        SMC_RW_PRINTF("%02x ", buffer[k]);
    }
    if(size>len_26)
    {
        SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4],buffer[size-3], buffer[size-2], buffer[size-1]);
    }
    SMC_RW_PRINTF("\n");
#endif
   // SMC_PRINTF("SMC TXRX: read %d bytes with %d\n", *actsize, start_t-end_t);
    //osal_semaphore_release(tp->smc_sema_id);
    return RET_SUCCESS;
}

static int smc_txrx_preset_check(struct smartcard_private *tp, UINT8 *buffer,
        UINT16 size, UINT8 *recv_buffer, UINT16 reply_num)
{
    /* This should be assigned for the upper error check */

    if(0 == tp->inserted)
    {
        SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
        return !RET_SUCCESS;
    }
    else if(tp->reseted != 1)
    {
        SMC_PRINTF("SMC TXRX: smart card not reseted!\n");
        return !RET_SUCCESS;
    }

    tp->smc_tx_buf = buffer;
    tp->smc_rx_buf = recv_buffer;
    tp->smc_tx_rd = 0;
    tp->smc_tx_wr = size;
    tp->smc_rx_head = 0;
    tp->got_first_byte = 0;

    if(reply_num)
    {
        tp->smc_rx_tail = reply_num;
    }
    else
    {
        tp->smc_rx_tail = SMC_RX_BUF_SIZE;
    }

    return 0;
}

static void smc_txrx_config_rx_receiver(struct smartcard_private *tp,
        UINT32 ioaddr, int reply_num)
{
    /* enable transmit mode disable receiver mode */
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~(SMC_SCR_CTRL_RECV|SMC_SCR_CTRL_TRANS)));
    OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~(SMC_IER0_BYTE_RECV_TRIG \
                    |SMC_IER0_BYTE_TRANS_TRIG)));

    if(reply_num)
    {
        if(reply_num/tp->smc_rx_fifo_size)
        {
            smc_write_rx(tp, ioaddr, (tp->smc_rx_fifo_size>>1));
            //SMC_PRINTF("set rx thld %d\n", 32);
        }
        else
        {
            smc_write_rx(tp, ioaddr, reply_num);
            //SMC_PRINTF("set rx thld %d\n", reply_num);
        }
    }
    else
    {
        smc_write_rx(tp, ioaddr, 0);
        //SMC_PRINTF("set rx thld %d\n", 0);
    }
    /*Always enable byte receive int*/
    OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_BYTE_RECV_TRIG);

    OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL, SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);

}

static int smc_txrx_set_tx_transfer_param(struct smartcard_private *tp, UINT32 ioaddr, UINT16 scale)
{
    INT32 j = 0;
    UINT32 tmo = 0;

    for(j = 0; j < scale; j++)
    {
        OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
    }
    tp->smc_tx_rd += scale;
    tmo = (scale + 2)*2;
    while(0 == (tp->isr0_interrupt_status & SMC_ISR0_FIFO_EMPTY))
    {
        osal_task_sleep(1);
        tmo--;
        if(0 == tmo)
        {
            return !RET_SUCCESS;
        }
        if( 0 == (INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80))
        {
            SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
            return !RET_SUCCESS;
        }
    }
    tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;

    return RET_SUCCESS;
}

static int smc_txrx_set_tx_transfer_param_lock_irq(struct smartcard_private *tp, UINT32 ioaddr, UINT16 scale)
{
    INT32 j = 0;
    UINT32 tmo = 0;

    osal_interrupt_disable();
    for(j = 0; j < scale; j++)
    {
        OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
    }
    tp->smc_tx_rd += scale;
    tmo = scale*2000;
    while(0 == (INPUT_UINT8(ioaddr + REG_ISR0)&SMC_ISR0_FIFO_EMPTY))
    {
        osal_delay(1);
        tmo--;
        if(0 == tmo)
        {
            osal_interrupt_enable();
            return !RET_SUCCESS;
        }
        if(0 == (INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80))
        {
            SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
            osal_interrupt_enable();
            return !RET_SUCCESS;
        }
    }
    return RET_SUCCESS;
}

static int __smc_txrx_config_tx_rx_trigger_group_one(struct smc_device *dev,
        int size, int loop)
{
    UINT16 loop_cnt = 0;
    UINT16 loop_remain = 0;
    UINT32 ioaddr = dev->base_addr;
    INT32 i = 0;
    INT32 ret = 0;
    UINT8 size_5 = 5;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    /* Enable Fifo Empty Interrupt */
    OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_TRANS_FIFO_EMPY);

    osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);

    if(size>size_5)
    {
        loop_remain = size - 5;
        loop_cnt = loop_remain/tp->smc_tx_fifo_size;
        loop_remain = loop_remain%tp->smc_tx_fifo_size;
        size = 5;
    }

    for(i = 0; i < loop_cnt; i++)
    {
    	ret = smc_txrx_set_tx_transfer_param(tp, ioaddr, tp->smc_tx_fifo_size);
        if(ret != RET_SUCCESS)
        {
        	SMC_PRINTF("SMC TXRX: in %s, line=%d, ret=%d\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }

    if(loop_remain)
    {
    	ret = smc_txrx_set_tx_transfer_param(tp, ioaddr, loop_remain);
        if(ret != RET_SUCCESS)
        {
        	SMC_PRINTF("SMC TXRX: in %s, line=%d, ret=%d\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
	
	ret = smc_txrx_set_tx_transfer_param_lock_irq(tp, ioaddr, size);
    if(ret != RET_SUCCESS)
    {
    	SMC_PRINTF("SMC TXRX: in %s, line=%d, ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    OUTPUT_UINT8(ioaddr + REG_ISR0, SMC_ISR0_FIFO_EMPTY);
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS)) \
            |SMC_SCR_CTRL_RECV);
    osal_interrupt_enable();
    tp->hsr(SMC_TX_FINISHED);
    return ret;
}

static void __smc_txrx_config_force_tx_rx_trigger_group_one(struct smartcard_private *tp,
        UINT32 ioaddr)
{
    UINT16 real_work_etu = INPUT_UINT16(ioaddr + REG_ETU0);//the clock's number of 1etu 
    UINT16 char_frame = INPUT_UINT16(ioaddr + REG_GTR0);//the etu's number of GT
    UINT32 force_tx_rx_thld_5 = 5;

    tp->char_frm_dura = char_frame*((real_work_etu*1000)/(tp->smc_clock/1000));//the GT time in unit us
    if(tp->force_tx_rx_triger)
    {
        tp->force_tx_rx_state = 0;
        /*((thld*char_faram*etu*1000)/(smc_clock/1000))>TX_RX_THLD*/
        tp->force_tx_rx_thld = ((tp->smc_clock/1000)*(TX_RX_THLD/1000))/(real_work_etu*char_frame);
        if(tp->force_tx_rx_thld < force_tx_rx_thld_5)
        {
            tp->force_tx_rx_thld = 5;
        }
        /* if(tp->force_tx_rx_thld > 10)tp->force_tx_rx_thld = 10; */

        SMC_PRINTF("SMC TXRX: force tx rx thld %d, char frm dura %d\n", tp->force_tx_rx_thld, tp->char_frm_dura);
    }
}

static void __smc_txrx_config_tx_transmitter(struct smartcard_private *tp,
        UINT32 ioaddr, int size, int loop)
{
    if(loop)
    {
        size = tp->smc_tx_fifo_size;
        if(ALI_S3602 == tp->smc_chip_id)
        {
            OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size>>1));
        }
        else
        {
            smc_write_tx(tp, ioaddr, tp->smc_tx_fifo_size>>1);
        }
    }
    else
    {
        if((tp->force_tx_rx_triger) && (size>tp->force_tx_rx_thld))
        {
            smc_write_tx(tp, ioaddr,size-tp->force_tx_rx_thld);
            tp->force_tx_rx_state = 1;
        }
        else
        {
            if(ALI_S3602 == tp->smc_chip_id)
            {
                OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
            }
            else
            {
                smc_write_tx(tp, ioaddr,size);
            }
        }
    }
}

static int __smc_txrx_config_force_tx_rx_trigger_group_two(struct smartcard_private *tp,
        UINT32 ioaddr, int loop)
{
    UINT32 force_loop_tmo = (smc_read_tx(tp, ioaddr)+1)*tp->char_frm_dura;
    UINT8  force_tx_rx = 1;
#ifdef SMC_RW_DEBUG
    UINT32 tmo = 0;
#endif

#ifdef SMC_RW_DEBUG
    tmo = read_tsc();
#endif
    if(tp->force_tx_rx_triger)
    {
        if((0 == loop) && (0 == tp->force_tx_rx_state))
        {
            if(0 == smc_read_tx(tp, ioaddr))
            {
                OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
		#ifdef SMC_RW_DEBUG
                SMC_PRINTF("SMC TXRX: tmo1: too late!\n");
            #endif
            }
            else
            {
                while(0 != smc_read_tx(tp, ioaddr))
                {
                    osal_delay(1);
                    force_loop_tmo--;

                    if(!force_loop_tmo)
                    {
                        force_tx_rx = 0;
                        SMC_PRINTF("SMC TXRX: tmo1: tmo %d\n", smc_read_tx(tp, ioaddr));
                        break;
                    }
                    if( 0 == (INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80))
                    {
                        SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
                        osal_interrupt_enable();
                        return !RET_SUCCESS;
                    }
                }
                if(force_tx_rx)
                {
                    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))| \
                            SMC_SCR_CTRL_RECV);
                }
                SMC_PRINTF("SMC TXRX: tmo1: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(sys_ic_get_cpu_clock()/ 2));
            }
            osal_interrupt_enable();
        }
        else
            osal_interrupt_enable();
    }
    else
        osal_interrupt_enable();

    return RET_SUCCESS;
}

static int __smc_txrx_config_tx_rx_trigger_group_two(struct smc_device *dev,
        int size, int loop, UINT8 *buffer)
{
    INT32 j = 0;
    UINT32 ioaddr = dev->base_addr;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    __smc_txrx_config_force_tx_rx_trigger_group_one(tp, ioaddr);
    __smc_txrx_config_tx_transmitter(tp, ioaddr, size, loop);

    for(j = 0; j < size; j++)
    {
        if((tp->smc_tx_rd+j+1) == tp->smc_tx_wr)
        {
            if(tp->auto_tx_rx_triger)
            {
                OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); /* tx->rx auto switch */
                SMC_PRINTF("SMC TXRX: tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, j, tp->smc_tx_wr);
            }
        }
        OUTPUT_UINT8(ioaddr + REG_THR, buffer[j]);
    }
    tp->smc_tx_rd += size;
    osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);

    OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_ISR0_FIFO_TRANS);

    osal_interrupt_disable();
    OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);

    return __smc_txrx_config_force_tx_rx_trigger_group_two(tp, ioaddr, loop);
}

static int smc_txrx_config_tx_rx_trigger(struct smc_device *dev,
        int size, int loop, UINT8 *buffer)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    SMC_RW_PRINTF("SMC TXRX: In %s Trigger method %d %d\n",
            __FUNCTION__, tp->force_tx_rx_triger, tp->auto_tx_rx_triger);

    if ((!tp->force_tx_rx_triger) && (!tp->auto_tx_rx_triger))
    {
        return __smc_txrx_config_tx_rx_trigger_group_one(dev, size, loop);
    }
    else
    {
        return __smc_txrx_config_tx_rx_trigger_group_two(dev, size, loop, buffer);
    }
}

static int __smc_txrx_wait_finish_timeout_type_one(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;

    if(tp->smc_tx_wr)
    {
        osal_interrupt_disable();
        if(tp->smc_tx_rd == tp->smc_tx_wr)
        {
            if((tp->force_tx_rx_triger) && (1==tp->force_tx_rx_state))
            {
                smc_write_tx(tp, ioaddr, tp->force_tx_rx_thld);
                tp->force_tx_rx_state = 2;
            }
        }
        else
        {
            SMC_PRINTF("SMC TXRX: tx not finished: w %d: r %d!\n", tp->smc_tx_wr, tp->smc_tx_rd);
            tp->smc_tx_rd=tp->smc_tx_wr;
            if(tp->force_tx_rx_triger)
            {
                tp->force_tx_rx_state = 2;
            }
            osal_interrupt_enable();
            return !RET_SUCCESS;
        }
        osal_interrupt_enable();
    }

    return RET_SUCCESS;
}

static int smc_txrx_wait_for_byte(struct smc_device *dev,
        OSAL_ER *wait_ret, UINT16 *actsize)
{
    UINT32 t_wait_tmo = 6000;
    OSAL_ER result = 0;
    UINT32 flgptn = 0;
    UINT16 tmp_cnt = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;
    UINT16 current_cnt = smc_read_rx(tp, ioaddr);
    UINT32 current_head = tp->smc_rx_head;
#ifdef SMC_RW_DEBUG
    UINT32 old_tick = osal_get_tick();
#endif

    t_wait_tmo = tp->first_cwt;
    SMC_PRINTF("[ %s %d ], first_cwt = %d\n", __FUNCTION__, __LINE__, tp->first_cwt);
    result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED| \
            SMC_RX_FINISHED|SMC_RX_BYTE_RCV, OSAL_TWF_ORW, t_wait_tmo);

    if(OSAL_E_OK != result)
    {
        *actsize = tp->smc_rx_head;
    #ifdef SMC_RW_DEBUG
        SMC_PRINTF("SMC TXRX: wait 1st byte TMO with %d, tick = %d\n",*actsize, osal_get_tick() - old_tick);
    #endif
        osal_flag_clear(tp->smc_flag_id, SMC_RX_FINISHED|SMC_TX_FINISHED| \
                SMC_RX_BYTE_RCV);
        smc_dev_clear_tx_rx_buf(tp);
        *wait_ret = result;
        return !RET_SUCCESS;
    }

#ifdef SMC_RW_DEBUG
    SMC_PRINTF("[ %s %d ], tick = %d\n", __FUNCTION__, __LINE__, osal_get_tick() - old_tick);
    old_tick = osal_get_tick();
#endif
    if(flgptn & SMC_RX_BYTE_RCV)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV);
        t_wait_tmo = tp->cwt;
        SMC_PRINTF("[ %s %d ], cwt = %d\n", __FUNCTION__, __LINE__, tp->cwt);
        do
        {
            flgptn = 0;
            result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_RX_FINISHED,  \
                    OSAL_TWF_ORW, t_wait_tmo);
            if(result == OSAL_E_OK)
			{				
				break;				
			}

            tmp_cnt = smc_read_rx(tp, ioaddr);
            if((tmp_cnt!=current_cnt) || (current_head != tp->smc_rx_head))
            {
                current_cnt = tmp_cnt;
                current_head=tp->smc_rx_head;
                result = OSAL_E_OK;
            }
        }while(result != OSAL_E_TIMEOUT);

        *wait_ret = result;
    }
    SMC_PRINTF("[ %s %d ], tick = %d, cnt = %d\n",
        __FUNCTION__, __LINE__, osal_get_tick() - old_tick, current_cnt);

    return RET_SUCCESS;
}

static void smc_txrx_s3281_change_clk(struct smartcard_private *tp, UINT32 *cpu_clk, UINT32 *real_clk)
{
    *cpu_clk = sys_ic_get_cpu_clock();

    if(ALI_S3281 == tp->smc_chip_id)
    {
        if(CPU_CLK_308 == *cpu_clk)
        {
            *real_clk = 24;
        }
        else if(CPU_CLK_270 == *cpu_clk)
        {
            *real_clk = 21;
        }
        else if(CPU_CLK_180 == *cpu_clk)
        {
            *real_clk = 14;
        }
        else if(CPU_CLK_90 == *cpu_clk)
        {
            *real_clk = 7;
        }
        else if(CPU_CLK_300 == *cpu_clk)
        {
            *real_clk = 24;
        }
        else
        {
            *real_clk = 21;
        }
    }
}

static int __smc_txrx_wait_finish_timeout_type_two(struct smc_device *dev,
        int reply_num, UINT16 *actsize, UINT32 cpu_clk, UINT32 real_clk)
{
    UINT32 cur_rx_head = 0;
    UINT32 smc_rd_tmo = 0;
    UINT32 tmp_rd_tick = 0;
    UINT32 rem_space = 0;
    UINT32 i = 0;
    UINT16 total_cnt = 0;
    UINT16 c = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 ioaddr = dev->base_addr;
    UINT32 old_tick = 0;

#ifdef SMC_RW_DEBUG
    UINT32 bak_tick = 0;
#endif
	
#ifdef SMC_RW_DEBUG
    bak_tick = osal_get_tick();
#endif
    /* <== patch for miss tx finish flag */
    if(reply_num)
    {
        if(tp->smc_rx_tail)
        {
            cur_rx_head = tp->smc_rx_head;

            if(cur_rx_head)
            {
                smc_rd_tmo = tp->cwt;
            }
            else
            {
                smc_rd_tmo = tp->first_cwt;
            }

            tmp_rd_tick= read_tsc();
            old_tick = osal_get_tick();
            while(tp->smc_rx_head<tp->smc_rx_tail )
            {
                if(0 == tp->inserted)
                {
                    SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
                    return !RET_SUCCESS;
                }
                osal_task_sleep(1);
                if(cur_rx_head!=tp->smc_rx_head)
                {
                    cur_rx_head = tp->smc_rx_head;
                    smc_rd_tmo = tp->cwt;
                    tmp_rd_tick = read_tsc();
                    old_tick = osal_get_tick();
                }
				
                if((ALI_S3281 == tp->smc_chip_id))
                {
                    if((smc_rd_tmo*1000) < ((read_tsc()-tmp_rd_tick)/(540*real_clk)*84+1))
                    {
                        smc_rd_tmo = 0;
                        break;
                    }
                }
                else
                {
                    if(smc_rd_tmo/*tp->first_cwt*/ < (osal_get_tick()-old_tick))
                    {
                        smc_rd_tmo = 0;
                        break;
                    }
                }
            }
	#ifdef SMC_RW_DEBUG
            SMC_PRINTF("[ %s %d ], tick = %d\n",
                        __FUNCTION__, __LINE__, osal_get_tick() - bak_tick);
	#endif
            //==>Patch By Goliath
            osal_interrupt_disable();
            rem_space = tp->smc_rx_tail - tp->smc_rx_head;
            c = smc_read_rx(tp, ioaddr);
            SMC_PRINTF("read second time, count is %d\n", c);
            total_cnt = c;
            c = (c<=rem_space?c:rem_space);
            total_cnt -= c;
            for(i = 0; i < c; i++)
            {
                tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
            }
            tp->smc_rx_head += c;
            osal_interrupt_enable();
            for(i = 0; i < total_cnt; i++)
            {
                c = INPUT_UINT8(ioaddr + REG_RBR);
            }
            SMC_PRINTF("SMC TXRX: TMO with %d, rem %d \n", tp->smc_rx_head, total_cnt);
        }

        *actsize = tp->smc_rx_head;

        if(tp->smc_rx_head)
        {
            smc_dev_clear_tx_rx_buf(tp);
            return RET_SUCCESS;
        }
        SMC_PRINTF("SMC TXRX: In %s Failed \n", __FUNCTION__);
        smc_dev_clear_tx_rx_buf(tp);
        return !RET_SUCCESS;
    }
    else
    {
        if(tp->smc_tx_rd!=tp->smc_tx_wr)
        {
            *actsize = tp->smc_tx_rd;
            SMC_PRINTF("SMC TXRX: tx tmo: w %d: r %d!\n", tp->smc_tx_wr, tp->smc_tx_rd);
            return !RET_SUCCESS;
        }
    }

    return 0;
}


/******************************************************************************************************
 *     Name        :   smc_dev_transfer_data()
 *    Description    :   Send the command or data to the device.
 *    Parameter    :   struct smc_device *dev    : Devcie handle
 *                UINT8 *buffer        : Wrtie data buffer.
 *                UINT16 size            : Write data size.
 *                UINT8 *recv_buffer        : Receive buffer to store the response data from the card.
 *                UINT16 reply_num        : Response number.
 *    Return        :   INT32            : SUCCESS or FAIL
 *
 ******************************************************************************************************/
INT32 smc_dev_transfer_data(struct smc_device *dev, UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, \
        UINT16 reply_num, UINT16 *actsize)
{
    struct smartcard_private *tp = NULL;
    UINT32 ioaddr = 0;
    UINT32 loop=0;
    UINT32 flgptn = 0;
    UINT32 waitmo = 6000;
    OSAL_ER result = 0;
    UINT32 cpu_clk = 0;
    UINT32 real_clk = 24;
    //<== patch for miss tx finish flag
    int ret = 0;

    SMC_PRINTF("SMC TXRX: %s: Send %d, Receive %d\n", __FUNCTION__, size, reply_num);
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    ioaddr = dev->base_addr;
	ret = smc_txrx_preset_check(tp, buffer, size, recv_buffer, reply_num);
    if (ret != RET_SUCCESS)
    {
        return ret;
    }

    if(size > tp->smc_tx_fifo_size)
    {
        loop = 1;
    }
    smc_txrx_config_rx_receiver(tp, ioaddr, reply_num);

	ret = smc_txrx_config_tx_rx_trigger(dev, size, loop, buffer);
    if (ret != RET_SUCCESS)
    {
    	SMC_PRINTF("SMC TXRX: in %s, line=%d, ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* Wait TX ready: */
    waitmo = ((tp->smc_tx_wr+1)*tp->char_frm_dura*3)/1000 + 1;//total GT time in unit ms

    SMC_PRINTF("smc_dev_transfer_data waitmo = %d\n", waitmo);
    flgptn = 0;
    result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED| \
            SMC_TX_FINISHED, OSAL_TWF_ORW, waitmo);
    /* timeout */
    if(OSAL_E_OK != result)
    {
    	SMC_PRINTF("[%s]line=%d, OSAL_E_OK != result, result=%d\n", __FUNCTION__, __LINE__, result);
		ret = __smc_txrx_wait_finish_timeout_type_one(dev);
		if(ret != RET_SUCCESS)
        {
            SMC_PRINTF("SMC TXRX:in %s, line=%d, ret=%d\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }

    if(tp->isr0_interrupt_status & SMC_ISR0_PE_TRANS)
    {
        SMC_PRINTF("SMC TXRX: TX Parity Error!\n");
        tp->isr0_interrupt_status &= (~SMC_ISR0_PE_TRANS);
    }

    SMC_PRINTF("[ %s %d ], 0x%x,  0x%x,  0x%x,  0x%x\n",
        __FUNCTION__, __LINE__, tp->smc_rx_tail, tp->smc_rx_buf,
        tp->smc_rx_tmp_buf, flgptn);
    if((tp->smc_rx_tail) && (tp->smc_rx_buf != tp->smc_rx_tmp_buf) &&  \
            (flgptn&SMC_TX_FINISHED) && (!(flgptn&SMC_REMOVED)))
    {
    	ret = smc_txrx_wait_for_byte(dev, &result, actsize);
        if(ret != RET_SUCCESS)
        {
            SMC_PRINTF("[ %s %d ]smc_txrx_wait_for_byte, ret=%d\n", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }

    if(OSAL_E_OK != result)
    {
        smc_txrx_s3281_change_clk(tp, &cpu_clk, &real_clk);

        ret = __smc_txrx_wait_finish_timeout_type_two(dev, reply_num, actsize, cpu_clk, real_clk);
        SMC_PRINTF("TMO %d, %d\n", __LINE__, ret);
        return ret;
    }

    if(flgptn & SMC_REMOVED)
    {
        osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
        *actsize = 0;
        SMC_PRINTF("SMC TXRX: In %s card removed!\n\n", __FUNCTION__);
        smc_dev_clear_tx_rx_buf(tp);
        return !RET_SUCCESS;
    }

    if(reply_num)
    {
        *actsize = tp->smc_rx_head;
    }
    else
    {
        *actsize = tp->smc_tx_rd;
    }

    SMC_PRINTF("SMC TXRX: In %s SUCCESS with %d\n\n", __FUNCTION__, *actsize);

    osal_interrupt_disable();
    tp->smc_tx_buf = NULL;
    tp->smc_tx_rd = 0;
    tp->smc_tx_wr = 0;
    osal_interrupt_enable();

    return RET_SUCCESS;
}

/******************************************************************************************************
 *     Name        :   smc_dev_transfer_data()
 *    Description    :   After command send to analyze the reply data.
 *    Parameter    :   struct smc_device *dev    : Devcie handle.
 *                UINT8 INS            : The instruction code.
 *                INT16 num_to_transfer    : Transmit data size
 *                UINT8 *status        : Status buffer pointer.
 *                INT16 size            : Response size.
 *    Return        :   INT32            : SUCCESS or FAIL
 *
 ******************************************************************************************************/
static INT32 smc_process_procedure_bytes(struct smc_device *dev, UINT8 ins, INT16 num_to_transfer, UINT8 status[2])
{
    INT16 r = 0;
    INT16 act_size = 0;
    UINT8 buff = 0;
    UINT8 sw_x60 = 0x60;
    UINT8 sw_x90 = 0x90;
    UINT8 ins_01 = 0x01;

    SMC_PRINTF("SMC TXRX: ISO: <- PROC:\n ");
    do
    {
        do
        {
            if(smc_dev_read(dev, &buff, 1, &act_size) != RET_SUCCESS)
            {
                SMC_PRINTF("SMC TXRX: 1 read error!\n");
                SMC_RW_PRINTF("SMC TXRX: 1 read error, in %s, line=%d, procedure bytes =%02x", __FUNCTION__, __LINE__, buff);
                return -1;
            }
            SMC_PRINTF("%02x ", buff);
            SMC_RW_PRINTF("SMC TXRX: in %s, line=%d, procedure bytes =%02x ", __FUNCTION__, __LINE__, buff);
        } while(buff == sw_x60);    /*NULL, send by the card to reset WWT*/

        if(((buff&0xF0)==sw_x60) || ((buff&0xF0)==sw_x90))     // SW1/SW2
        {
            status[0]=buff;
            if(smc_dev_read(dev, &buff, 1, &act_size) != RET_SUCCESS)
            {
                SMC_PRINTF("SMC TXRX: 2 read error!\n");
                SMC_RW_PRINTF("SMC TXRX: 2 read error!, in %s, line=%d\n", __FUNCTION__, __LINE__);
                return -1;
            }
            SMC_PRINTF("%02x\n", buff);
            status[1] = buff;
            SMC_RW_PRINTF("SMC TXRX: in %s, line=%d, status[0]=%02x, status[1]=%02x\n", __FUNCTION__, __LINE__, status[0], status[1]);
            return 0;
        }
        else
        {
        		SMC_RW_PRINTF("SMC TXRX: in %s, line=%d, buff=%02x!\n", __FUNCTION__, __LINE__, buff);
            if(0 == (buff ^ ins))    /* ACK == INS*/
            {
                /*Vpp is idle. All remaining bytes are transfered subsequently.*/
                r = num_to_transfer;
            }
            else if(0xFF == (buff ^ ins))        /* ACK == ~INS*/
            {
                /*Vpp is idle. Next byte is transfered subsequently.*/
                r = 1;
            }
            else if((buff ^ ins) == ins_01)    /* ACK == INS+1*/
            {
                /*Vpp is active. All remaining bytes are transfered subsequently.*/
                r = num_to_transfer;
            }
            else if(0xFE == (buff ^ ins))    /* ACK == ~INS+1*/
            {
                /*Vpp is active. Next bytes is transfered subsequently.*/
                r = 1;
            }
            //###########################################################
            //seca exceptions
            else if((0x3C == (buff ^ ins))||(0x40 == (buff ^ ins)))
            {
                r = num_to_transfer;
            }
            //###########################################################
            else
            {
                SMC_PRINTF("SMC TXRX: cannot handle procedure %02x (INS=%02x)\n", buff, ins);
                SMC_RW_PRINTF("SMC TXRX: in %s, line=%d,cannot handle procedure %02x (INS=%02x)\n", __FUNCTION__, __LINE__, buff, ins);
                #if GET_UNKOWN_SW_BYTE
				status[0]=buff;
                status[1] = 0x00;
				#endif
                return -1;
            }
            if(r > num_to_transfer)
            {
                SMC_PRINTF("SMC TXRX: data overrun r=%d num_to_transfer=%d\n", r, num_to_transfer);
                SMC_RW_PRINTF("SMC TXRX: in %s, line=%d,data overrun r=%d num_to_transfer=%d\n",__FUNCTION__, __LINE__, r, num_to_transfer);
                #if GET_UNKOWN_SW_BYTE
				status[0]=buff;
                status[1] = 0x00;
				#endif
                return -1;

            }
        }
    } while(0 == r);
    return r;
}

/******************************************************************************************************
 *     Name        :       smc_dev_iso_transfer()
 *    Description    :    Combines the functionality of both write and read.
 *                    Implement ISO7816-3 command transfer.
 *    Parameter    :    struct smc_device *dev        : Device structuer.
 *                    UINT8 *command            : ISO7816 command buffer pointer.
 *                    INT16 num_to_write        : Number to transmit.
 *                    UINT8 *response,             : Response data buffer pointer.
 *                    INT16 num_to_read        : number to read from SMC, 0 means it's write cmd
 *                    INT16 *actual_size            : pointer to the actual size received from SMC
 *    Return        :    INT32                     : SUCCESS or FAIL.
 ******************************************************************************************************/
INT32 smc_dev_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write,
        UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
    INT32 error = SMART_NO_ERROR;
    UINT8 write_flag = 0;
    INT16 size = 0;
    INT16 num_to_transfer = 0;
    INT16 length = 0;
    INT16 temp_length = 0;
    UINT8 status[2] = {0,0};

    struct smartcard_private *tp = NULL;
	SMC_PRINTF("SMC TXRX: Enter %s, line=%d\n", __FUNCTION__, __LINE__);
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    if(0 == tp->inserted)
    {
        SMC_PRINTF("SMC TXRX: in %s: smart card not inserted!\n",__FUNCTION__);
        return !RET_SUCCESS;
    }
    else if(tp->reseted != 1)
    {
        SMC_PRINTF("SMC TXRX: in %s: smart card not reseted!\n",__FUNCTION__);
        return !RET_SUCCESS;
    }
    if(tp->T > 1)
    {
        return SMART_INVALID_PROTOCOL;
    }
    SMC_PRINTF("SMC TXRX: CMD: IFD -> SMC ");
    SMC_DUMP(command, CMD_LENGTH);
    *actual_size = 0;

    if(num_to_write > CMD_LENGTH)
    {
        write_flag = 1;
        num_to_transfer = command[P3_OFFSET];
        //ASSERT(num_to_transfer == (num_to_write - CMD_LENGTH))
    }
    else if(CMD_LENGTH == num_to_write)
    {
        write_flag = 0;
        //if(response == NULL)     /* write data to smart card*/
        if(0 == num_to_read)
        {
            num_to_transfer = 0;
        }
        else        /*read data from smart card*/
        {
            num_to_transfer = (0 == command[P3_OFFSET]) ? MAX_LENGTH : command[P3_OFFSET];
        }
    }
    else
    {
        SMC_PRINTF("SMC TXRX: in %s: error command length!\n", __FUNCTION__);
        return !RET_SUCCESS;
    }
    SMC_PRINTF("SMC TXRX: in %s:write_flag = %d\n", __FUNCTION__, write_flag);
    /* Check the CLA and INS bytes are valid */
    if (command[CLA_OFFSET] != 0xFF)
    {
        if (((command[INS_OFFSET] & 0xF0) != 0x60) &&
                ((command[INS_OFFSET] & 0xF0) != 0x90))
        {
            if(smc_dev_write(dev, command, CMD_LENGTH, &size) != SUCCESS)
            {
                SMC_PRINTF("SMC TXRX: in %s: 1 write cmd error!\n", __FUNCTION__);
                return !RET_SUCCESS;
            }
            length = 0;
            while(1)
            {
                temp_length = smc_process_procedure_bytes(dev, command[INS_OFFSET], num_to_transfer - length, status);
                if(0 == temp_length)
                {
                    if (NULL != response)
                    {
                        response[*actual_size] = status[0];
                        response[(*actual_size)+1] = status[1];
                        *actual_size += 2;
                    }
                    return RET_SUCCESS;
                }
                else if(temp_length < 0)
                {
                    SMC_PRINTF("SMC TXRX: in %s: procedure return error! CMD is:\n", __FUNCTION__);
                    SMC_DUMP(command, num_to_write);
					#if GET_UNKOWN_SW_BYTE
                    if (NULL != response)
                    {
                        response[*actual_size] = status[0];
                        response[(*actual_size)+1] = status[1];
                        *actual_size += 2;
                    }
					#endif
                    return !RET_SUCCESS;
                }

                if(1 == write_flag)
                {
                    if(smc_dev_write(dev, command + CMD_LENGTH + length, temp_length, &size) != RET_SUCCESS)
                    {
                        SMC_PRINTF("SMC TXRX: in %s: 2 write data error!\n", __FUNCTION__);
                        return !RET_SUCCESS;
                    }
                    SMC_PRINTF("SMC TXRX: DATA: IFD -> SMC: ");
                    SMC_DUMP(command + CMD_LENGTH + length, temp_length);
                }
                else
                {
                    if(smc_dev_read(dev, response + length, temp_length, &size) != RET_SUCCESS)
                    {
                        SMC_PRINTF("SMC TXRX: in %s: data read error!\n", __FUNCTION__);
                        return !RET_SUCCESS;
                    }

					/* no enough data, reset card, for A.61 */
                    if(temp_length - size >0)
                	{
                        SMC_PRINTF("no enough data, reset card, for A.61. temp_length=0x%x, size=0x%x\n",temp_length,size);
                		return !RET_SUCCESS;
                	}
                    *actual_size += temp_length;
                    SMC_PRINTF("SMC TXRX: DATA: IFD <- SMC, ");
                    SMC_DUMP(response + length, temp_length);
                }
                length += temp_length;
            }
        }
        else
        {
            /* INS is invalid */
            error = SMART_INVALID_CODE;
        }
    }
    else
    {
        /* CLA is invalid */
        error = SMART_INVALID_CLASS;
    }
    return error;
}

INT32 smc_dev_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write,
        UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
    INT32 error = RET_SUCCESS;
    INT16 size = 0;

    struct smartcard_private *tp = NULL;
	SMC_PRINTF("SMC TXRX: Enter %s, line=%d\n", __FUNCTION__, __LINE__);
    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct smartcard_private *)dev->priv;
    if(0 == tp->inserted)
    {
        SMC_PRINTF("SMC TXRX: smart card not inserted!\n");
        return !RET_SUCCESS;
    }
    else if(tp->reseted != 1)
    {
        SMC_PRINTF("SMC TXRX: smart card not reseted!\n");
        return !RET_SUCCESS;
    }

    osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);

    if(smc_dev_transfer_data(dev, command, num_to_write, response, num_to_read, (UINT16 *)(&size)) != SUCCESS)
    {
        osal_semaphore_release(tp->smc_sema_id);
        return !RET_SUCCESS;
    }
    //SMC_DUMP(response,size);
    *actual_size = size;
    osal_semaphore_release(tp->smc_sema_id);
    return error;
}

