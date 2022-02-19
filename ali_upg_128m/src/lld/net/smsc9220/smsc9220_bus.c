/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    smsc9220_bus.c
*
*    Description: Use ATA interface as Data transfer bus. 
*
*    History:
*		Date			Athor		Version		Reason
*	    =======================================================
*     1.	12.24.2008	Mao Feng		Ver 0.1		Create
*
******************************************************************************/
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/cic/cic.h>

#include <hld/net/net.h>
#include <hld/net/ethtool.h>

#include <hal/hal_gpio.h>

extern UINT32 ATA_IO_BASE;
#include "smsc9220.h"
#include "smsc9220_reg.h"

UINT8 g_smsc_addr7 = 0;
extern UINT8 SMSC_ADDR7;

extern UINT8 SMSC_FIFO_SEL;

#ifdef SMSC_UP_BUS
UINT32 smsc_spe_map = 0;
UINT8 smsc_spe_id = 0;

#else //SMSC_UP_BUS
//GPIO-Address
UINT8 g_smsc_addr6 = 0;
extern UINT8 SMSC_ADDR6;

//mutex id for CI & ATA DMA op.
struct smsc_mutex smsc_mutex_id = { 
	.lock = NULL,
	.unlock = NULL,
};

void smsc_mutex_init(void)
{
	if(ALI_M3329E==sys_ic_get_chip_id())
	{
		UINT32 pin_mux = *((volatile UINT32 *)0xb8000010);
		if(0!=(pin_mux&0x3))
		{
			struct cic_device * cic = NULL;
			cic = (struct cic_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_CIC);
			if(cic)
			{
				struct cic_req_mutex req_param;
				MEMSET(&req_param, 0, sizeof(struct cic_req_mutex));
				cic->do_ioctl(cic, CIC_DRIVER_REQMUTEX, (UINT32)(&req_param));
				smsc_mutex_id.lock = req_param.cic_enter_mutex;
				smsc_mutex_id.unlock = req_param.cic_exit_mutex;
			}
		}
	}
}

#endif//SMSC_UP_BUS

extern PPRIVATE_DATA g_p_adapter;

void lan_mutex_lock(OSAL_ID mutex_id);
void lan_mutex_unlock(OSAL_ID mutex_id);

#if (1)
#define SMSC_PROTECT_ENTER()	lan_mutex_lock(g_p_adapter->reg_mutex)
#define SMSC_PROTECT_EXIT()	lan_mutex_unlock(g_p_adapter->reg_mutex)
#else
#define SMSC_PROTECT_ENTER()	osal_interrupt_disable()
#define SMSC_PROTECT_EXIT()	osal_interrupt_enable()
#endif


/**********************************************************************
*	Register Access
**********************************************************************/
UINT16 smsc_r16(UINT32 addr)
{
#ifdef SMSC_UP_BUS
	UINT8 a7;
	a7 = (addr >> 7) & 0x01;
	
	if(a7 != g_smsc_addr7)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR7, a7);
		g_smsc_addr7 = a7;
	}	
	return *(volatile UINT16*)(smsc_spe_map + (addr & 0x7f));
	
#else //SMSC_UP_BUS
	UINT16 data;
	UINT8 a6,a7;
	UINT8 aar;
	
	//Address Analysis.
	aar = (addr >> 1) & 0x1f;
	
	a6 = (addr >> 6) & 0x01;
	a7 = (addr >> 7) & 0x01;

	if(a6 != g_smsc_addr6)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR6, a6);
		g_smsc_addr6 = a6;
	}

	if(a7 != g_smsc_addr7)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR7, a7);
		g_smsc_addr7 = a7;
	}
	
	ata_w8(ATA_AAR, aar);
	data = ata_r16(ATA_DAR);
	
	return data;
	
#endif //SMSC_UP_BUS

}


void smsc_w16(UINT32 addr,UINT16 data)
{

#ifdef SMSC_UP_BUS	
	UINT8 a7;
	a7 = (addr >> 7) & 0x01;
	
	if(a7 != g_smsc_addr7)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR7, a7);
		g_smsc_addr7 = a7;
	}	

	*(volatile UINT16*)(smsc_spe_map + (addr & 0x7f)) = data;

#else //SMSC_UP_BUS
	UINT8 a6,a7;
	UINT8 aar;
	
	//Address Analysis.
	aar = (addr >> 1) & 0x1f;
	
	a6 = (addr >> 6) & 0x01;
	a7 = (addr >> 7) & 0x01;

	if(a6 != g_smsc_addr6)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR6, a6);
		g_smsc_addr6 = a6;
	}

	if(a7 != g_smsc_addr7)
	{
		HAL_GPIO_BIT_SET(SMSC_ADDR7, a7);
		g_smsc_addr7 = a7;
	}

	ata_w8(ATA_AAR, aar);
	ata_w16(ATA_DAR, data);	
	
#endif //SMSC_UP_BUS

}


UINT32 smsc_r32(UINT32 addr)
{
	UINT32 data;
	UINT16 data_high, data_low;
	//int cnt = 0;
	
	SMSC_PROTECT_ENTER();		

	data_low = smsc_r16(addr);
	data_high = smsc_r16(addr+2);

	data = (UINT32)(data_high<<16)|(UINT32)data_low;

	SMSC_PROTECT_EXIT();	
	
	

	return data;
}


void smsc_w32(UINT32 addr,UINT32 data)
{
	UINT16 data_high, data_low;
	
	data_low = (UINT16)data;
	data_high = (UINT16)(data>>16);
	
	SMSC_PROTECT_ENTER();		
	
	smsc_w16(addr, data_low);
	smsc_w16(addr+2, data_high);
	
	SMSC_PROTECT_EXIT();
}


/**********************************************************************
*	Fifo Access
**********************************************************************/
UINT32 smsc_fifo_r32(UINT32 addr)
{
	UINT32 data;
	UINT16 data_high, data_low;
	
	data_low = smsc_r16(addr);
	data_high = smsc_r16(addr+2);
	
	data = (UINT32)(data_high<<16)|(UINT32)data_low;

	return data;

}

void smsc_fifo_w32(UINT32 addr,UINT32 data)
{
	UINT16 data_high, data_low;
	
	data_low = (UINT16)data;
	data_high = (UINT16)(data>>16);

	smsc_w16(addr, data_low);
	smsc_w16(addr+2, data_high);

}


void smsc_write_data_fifo(PPRIVATE_DATA p_adapter, void *buf, UINT16 len)
{
	UINT32 data_off = 0;
	UINT32 tx_cmd_a, tx_cmd_b, fifo_len;
	UINT32 *buf_32;

	//Lan_PrintBuf((UINT8 *)buf,len);
	
	data_off = ((UINT32)(buf) & 0x03UL);
	buf_32 = (UINT32 *)((UINT32)(buf) & ~0x03UL);
	fifo_len = (len + data_off + 3)>>2;
	
	tx_cmd_a = ((data_off<<16) |TX_CMD_A_INT_FIRST_SEG_ |TX_CMD_A_INT_LAST_SEG_| ((UINT32)len));	
	tx_cmd_b=(((UINT32)len<<16)| ((UINT32)len & 0x7FFUL));

	//SMSC_TRACE("%s()=>cmdA(0x%08x) cmdB(0x%08x).",__FUNCTION__, tx_cmdA, tx_cmdB);				
	//SMSC_TRACE("%s()=>len(%d) fifo_len(%d).\n",__FUNCTION__, len, fifo_len);				

	SMSC_PROTECT_ENTER();
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 1);
	
	smsc_fifo_w32(TX_DATA_FIFO, tx_cmd_a);
	smsc_fifo_w32(TX_DATA_FIFO, tx_cmd_b);

	if((SMSC_TRANSFER_DMA == p_adapter->transfer_mode))//&& (dwDwordCount*4 > g_pAdapter->DmaThreshold))
	{
#ifdef SMSC_UP_BUS
		osal_cache_flush(buf, len);
		spe_dma_copy(smsc_spe_id, (smsc_spe_map + TX_DATA_FIFO), buf_32, (fifo_len*4), \
			(SPE_ADDR_NOINC|SPE_DMA_WRITE|SPE_DMA_SYNC));
			//(SPE_BYTE_TRANS|SPE_ADDR_NOINC|SPE_DMA_WRITE|SPE_DMA_SYNC));
			//(SPE_BYTE_TRANS|SPE_DMA_WRITE|SPE_DMA_SYNC));
			//(SPE_BYTE_TRANS|SPE_ADDR_NOINC|SPE_DMA_WRITE));
#endif
	}
	else
	{
		while (fifo_len)
		{
			smsc_fifo_w32(TX_DATA_FIFO, *buf_32++);
			fifo_len--;
		}
	}
	
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 0);
	SMSC_PROTECT_EXIT();
}


void smsc_read_data_fifo(UINT32 transfer, UINT32 *pdw_buf, UINT32 dw_dword_count)
{
	//libc_printf("%d-", dwDwordCount);
	SMSC_PROTECT_ENTER();
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 1);

	if((SMSC_TRANSFER_DMA == transfer))
	{
#ifdef SMSC_UP_BUS
		osal_cache_invalidate(pdw_buf, (dw_dword_count*4));
		spe_dma_copy(smsc_spe_id, (smsc_spe_map + RX_DATA_FIFO), pdw_buf, (dw_dword_count*4), \
		(SPE_ADDR_NOINC|SPE_DMA_READ|SPE_DMA_SYNC));
		//(SPE_BYTE_TRANS|SPE_ADDR_NOINC|SPE_DMA_READ|SPE_DMA_SYNC));
		//(SPE_BYTE_TRANS|SPE_DMA_READ|SPE_DMA_SYNC));
		//(SPE_BYTE_TRANS|SPE_ADDR_NOINC|SPE_DMA_READ));
#endif
	}
	else
	{
		while (dw_dword_count)
		{
			*pdw_buf++ = smsc_fifo_r32(RX_DATA_FIFO);
			dw_dword_count--;
		}
	}
	
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 0);
	SMSC_PROTECT_EXIT();
}


