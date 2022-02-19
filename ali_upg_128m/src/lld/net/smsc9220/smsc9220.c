/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    smsc9220.c
*
*    Description:    SMSC LAN9220 NIC Driver From Linux.
*
*    History:
*		Date			Athor		Version		Reason
*	    =======================================================
*	1.	12.23.2008	Mao Feng		Ver 0.1		Create
*	2.	05.08.2009							Support Both 3329 & 3602
*
******************************************************************************/
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>

#include <hld/net/net.h>
#include <hld/net/ethtool.h>
#include <hal/hal_gpio.h>

UINT32 ATA_IO_BASE;
#include "smsc9220.h"
#include "smsc9220_reg.h"

//GPIO-Address
UINT8 SMSC9220_GPIO = 7; //lan_int
UINT8 SMSC9220_POLAR = 0; //int polar

UINT8 SMSC_FIFO_SEL = 36; //fifo_sel

UINT8 SMSC_ADDR7 = 6; //lan_a7
extern UINT8 g_smsc_addr7;

#ifdef SMSC_UP_BUS
extern UINT32 smsc_spe_map;
extern UINT8 smsc_spe_id;

#else //SMSC_UP_BUS
UINT8 SMSC_CS = 10; //lan_a5

UINT8 SMSC_ADDR6 = 61; //lan_a6
extern UINT8 g_smsc_addr6;

//mutex id for CI & ATA DMA op.
extern struct smsc_mutex smsc_mutex_id;

#endif//SMSC_UP_BUS

PPRIVATE_DATA g_p_adapter = NULL;

static INT8 smsc9220_name[HLD_MAX_NAME_SIZE] = "SMSC9220_0";

//88h register settings.
volatile UINT32 g_gpio_setting=(GPIO_CFG_LED1_EN_|GPIO_CFG_LED2_EN_);
volatile UINT32 g_gpio_setting_original=(GPIO_CFG_LED1_EN_|GPIO_CFG_LED2_EN_);

//global options.
UINT32 int_deas=0x22UL;	//Int De-assertion interval.

#define TX_FIFO_LOW_THRESHOLD	(1600)

UINT32 smsc_r32(UINT32 addr);
void smsc_w32(UINT32 addr,UINT32 data);
void smsc_write_data_fifo(PPRIVATE_DATA p_adapter, void *buf, UINT16 len);
void smsc_read_data_fifo(UINT32 transfer, UINT32 *pdw_buf, UINT32 dw_dword_count);
void smsc_mutex_init(void);

#define lan_get_reg_dw(dw_offset)	\
	smsc_r32(dw_offset)

#define lan_set_reg_dw(dw_offset,dw_val)	\
	smsc_w32(dw_offset,dw_val)

#define lan_clr_bits_dw(dw_offset,dw_bits)	\
	smsc_w32(dw_offset,smsc_r32(dw_offset)&(~dw_bits))

#define lan_set_bits_dw(dw_offset,dw_bits)	\
	smsc_w32(dw_offset,smsc_r32(dw_offset)|dw_bits)

//Debug Use.
void lan_print_buf(UINT8 *p, UINT32 len)
{
	UINT32 i,label=0,count=0;

	SMSC_TRACE("buffer addr: %x\n", p);
	for(i=0;i<len;i++)
	{
		if(!(i%16))
		{
			SMSC_TRACE("\n");
			label = 1;
		}
		if(label)
		{
			SMSC_TRACE("%08x   ",count);
			count+=16;
			label = 0;
		}
			SMSC_TRACE("%02x ",p[i]);
	}
	SMSC_TRACE("\n");
}


void lan_mutex_lock(OSAL_ID mutex_id)
{
	SMSC_ASSERT(OSAL_INVALID_ID!=mutex_id);
	osal_mutex_lock(mutex_id, OSAL_WAIT_FOREVER_TIME);
}


void lan_mutex_unlock(OSAL_ID mutex_id)
{
	SMSC_ASSERT(OSAL_INVALID_ID!=mutex_id);	
	osal_mutex_unlock(mutex_id);
}


UINT32 lan_reg_get_print(UINT32 addr)
{
	UINT32 tmp_32 = lan_get_reg_dw(addr);
	
	SMSC_TRACE("=>Smsc io Reg 0x%02x : 0x%08x.\n", addr, tmp_32);

	return tmp_32;
}


void lan_reg_test()
{
	//UINT16 tmp_16;
	//UINT32 tmp_32;
	int i;

	//Smsc io Regs test
	SMSC_TRACE("Smsc io Regs Write&Read Test: \n");
	
	for(i = 0x00; i < 0xb8; i += 4)
	{
		if(1)
		{
			//smsc_r32(i);
			SMSC_TRACE("=>Smsc io Reg 0x%02x : 0x%08x.\n", i, smsc_r32(i));
		}
		else
		{
			SMSC_TRACE("=>Smsc io Reg 0x%02x : 0x%08x", i, smsc_r32(i));
			smsc_w32(i, 0x00000000);
			SMSC_TRACE("=======>after write 0 : 0x%08x", smsc_r32(i));
			smsc_w32(i, 0xffffffff);
			SMSC_TRACE("=======>after write 1 : 0x%08x. \n", smsc_r32(i));
		}
	}

	osal_task_sleep(1000);
	//asm("sdbbp");

}


RET_CODE
lan_reset(PPRIVATE_DATA p_adapter)
{
	UINT32 dw_time_out=0;
	UINT32 dw_temp=0;
	UINT32 dw_reset_count=1;	
	//int i;
	
	while(dw_reset_count>0) 
	{
		//SMSC_WARNING("%s()=>HW_CFG(0x%x).\n", __FUNCTION__, Lan_GetRegDW(HW_CFG));
		lan_set_reg_dw(HW_CFG,HW_CFG_SRST_|HW_CFG_SF_);
		
		dw_time_out=10;	// 1s ?
		do {
			osal_task_sleep(100);
			dw_temp=lan_get_reg_dw(HW_CFG);
			SMSC_WARNING("%s()=>HW_CFG(0x%x).\n", __FUNCTION__, dw_temp);
			dw_time_out--;
		} while((dw_time_out>0)&&(dw_temp&HW_CFG_SRST_));
		if(dw_temp&HW_CFG_SRST_) {
			SMSC_WARNING("%s()=>Failed to complete reset.\n", __FUNCTION__);
			return ERR_FAILED;
		}
		dw_reset_count--;
	}

	return SUCCESS;
}


/*
Step 1=>Chip Reset
Step 2=>Hw Config(HW_CFG AFC_CFG)
Step 3=>GPIO Config(GPIO_CFG)
*/
RET_CODE
lan_initialize(PPRIVATE_DATA p_adapter)
{
	RET_CODE ret;
	UINT32 dw_tx_fif_sz = p_adapter->tx_fifo_size;
	UINT32 dw_afc_cfg = p_adapter->afc_config;
	//UINT32 dw_int_cfg = p_adapter->interrupt_config;
	//UINT32 dw_time_out=0;

	ret = lan_reset(p_adapter);
	if(SUCCESS != ret)
		return ret;

	SMSC_ASSERT(dw_tx_fif_sz>=0x00020000UL);
	SMSC_ASSERT(dw_tx_fif_sz<=0x000E0000UL);
	SMSC_ASSERT((dw_tx_fif_sz&(~HW_CFG_TX_FIF_SZ_))==0);
	lan_set_reg_dw(HW_CFG,dw_tx_fif_sz);
	
	p_adapter->rx_max_data_fifo_size=0;
	switch(dw_tx_fif_sz>>16) 
	{
		case 2:p_adapter->rx_max_data_fifo_size=13440;break;
		case 3:p_adapter->rx_max_data_fifo_size=12480;break;
		case 4:p_adapter->rx_max_data_fifo_size=11520;break;
		case 5:p_adapter->rx_max_data_fifo_size=10560;break;
		case 6:p_adapter->rx_max_data_fifo_size=9600;break;
		case 7:p_adapter->rx_max_data_fifo_size=8640;break;
		case 8:p_adapter->rx_max_data_fifo_size=7680;break;
		case 9:p_adapter->rx_max_data_fifo_size=6720;break;
		case 10:p_adapter->rx_max_data_fifo_size=5760;break;
		case 11:p_adapter->rx_max_data_fifo_size=4800;break;
		case 12:p_adapter->rx_max_data_fifo_size=3840;break;
		case 13:p_adapter->rx_max_data_fifo_size=2880;break;
		case 14:p_adapter->rx_max_data_fifo_size=1920;break;
		default:SMSC_ASSERT(FALSE);break;
	}
	
	if(dw_afc_cfg==0xFFFFFFFF) {
		switch(dw_tx_fif_sz) {
		//AFC_HI is about ((Rx Data Fifo Size)*2/3)/64
		//AFC_LO is AFC_HI/2
		//BACK_DUR is about 5uS*(AFC_LO) rounded down
		case 0x00020000UL://13440 Rx Data Fifo Size
			dw_afc_cfg=0x008C46AF;break;
		case 0x00030000UL://12480 Rx Data Fifo Size
			dw_afc_cfg=0x0082419F;break;
		case 0x00040000UL://11520 Rx Data Fifo Size
			dw_afc_cfg=0x00783C9F;break;
		case 0x00050000UL://10560 Rx Data Fifo Size
			//dwAfcCfg=0x006E378F;break;
			dw_afc_cfg=0x006E374F;break;
		case 0x00060000UL:// 9600 Rx Data Fifo Size
			dw_afc_cfg=0x0064328F;break;
		case 0x00070000UL:// 8640 Rx Data Fifo Size
			dw_afc_cfg=0x005A2D7F;break;
		case 0x00080000UL:// 7680 Rx Data Fifo Size
			dw_afc_cfg=0x0050287F;break;
		case 0x00090000UL:// 6720 Rx Data Fifo Size
			dw_afc_cfg=0x0046236F;break;
		case 0x000A0000UL:// 5760 Rx Data Fifo Size
			dw_afc_cfg=0x003C1E6F;break;
		case 0x000B0000UL:// 4800 Rx Data Fifo Size
			dw_afc_cfg=0x0032195F;break;

		//AFC_HI is ~1520 bytes less than RX Data Fifo Size
		//AFC_LO is AFC_HI/2
		//BACK_DUR is about 5uS*(AFC_LO) rounded down
		case 0x000C0000UL:// 3840 Rx Data Fifo Size
			dw_afc_cfg=0x0024124F;break;
		case 0x000D0000UL:// 2880 Rx Data Fifo Size
			dw_afc_cfg=0x0015073F;break;
		case 0x000E0000UL:// 1920 Rx Data Fifo Size
			dw_afc_cfg=0x0006032F;break;
		default:SMSC_ASSERT(FALSE);break;
		}
	}
	lan_set_reg_dw(AFC_CFG,(dw_afc_cfg&0xFFFFFFF0UL));

	/*
	//make sure EEPROM has finished loading before setting GPIO_CFG
	
	dwTimeOut=1000;
	while((dwTimeOut>0)&&(Lan_GetRegDW(E2P_CMD)&E2P_CMD_EPC_BUSY_)) {
		osal_delay(5);
		dwTimeOut--;
	}
	if(dwTimeOut==0) {
		SMSC_WARNING("%s()=>Timed out waiting for EEPROM busy bit to clear\n", __FUNCTION__);
	}
	*/
	
	//Config associated pins as LEDs
	g_gpio_setting_original = 0x70070000UL;
	g_gpio_setting = g_gpio_setting_original;
	lan_set_reg_dw(GPIO_CFG,g_gpio_setting);

	p_adapter->lan_initialized=TRUE;

	return SUCCESS;
	
}


void 
lan_enable_interrupt(PPRIVATE_DATA p_adapter,UINT32 dw_int_en_mask)
{
	//UINT32 dw_int_flags=0;
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	lan_set_bits_dw(INT_EN,dw_int_en_mask);
}


void 
lan_disable_interrupt(PPRIVATE_DATA p_adapter,UINT32 dw_int_en_mask)
{
	//UINT32 dw_int_flags=0;
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	lan_clr_bits_dw(INT_EN,dw_int_en_mask);
}


void 
lan_disable_irq()
{
	lan_clr_bits_dw(INT_CFG,INT_CFG_IRQ_EN_);
}
	


void 
lan_set_intdeas(PPRIVATE_DATA p_adapter, UINT32 dw_int_deas)
{
	//UINT32 dw_int_flags=0;
	{
		lan_clr_bits_dw(INT_CFG,INT_CFG_INT_DEAS_);
		lan_set_bits_dw(INT_CFG,(dw_int_deas<<24));
	}
}


void 
lan_signal_software_interrupt(PPRIVATE_DATA p_adapter)
{
	SMSC_ASSERT(p_adapter!=NULL);
	p_adapter->software_interrupt_signal=FALSE;
	lan_enable_interrupt(p_adapter,INT_EN_SW_INT_EN_);
}


void 
lan_set_tdfl(PPRIVATE_DATA p_adapter, UINT8 level) 
{
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	{
		UINT32 temp=lan_get_reg_dw(FIFO_INT);
		temp&=0x00FFFFFFUL;
		temp|=((UINT32)level)<<24;
		lan_set_reg_dw(FIFO_INT,temp);
	}

}


void 
lan_set_tsfl(PPRIVATE_DATA p_adapter, UINT8 level) 
{
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	{
		UINT32 temp=lan_get_reg_dw(FIFO_INT);
		temp&=0xFF00FFFFUL;
		temp|=((UINT32)level)<<16;
		lan_set_reg_dw(FIFO_INT,temp);
	}

}


void 
lan_set_rsfl(PPRIVATE_DATA p_adapter, UINT8 level) 
{
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	{
		UINT32 temp=lan_get_reg_dw(FIFO_INT);
		temp&=0xFFFFFF00UL;
		temp|=((UINT32)level);
		lan_set_reg_dw(FIFO_INT,temp);
	}

}


static BOOL 
mac_not_busy(PPRIVATE_DATA p_adapter)
{
	int i=0;

	// wait for MAC not busy, w/ timeout
	for(i=0;i<40;i++)
	{
		if((lan_get_reg_dw(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)==(0UL)) {
			return TRUE;
		}
	}
	SMSC_WARNING("timeout waiting for MAC not BUSY. MAC_CSR_CMD = 0x%08lX",
		lan_get_reg_dw(MAC_CSR_CMD));
	return FALSE;
}


UINT32 
mac_get_reg32(PPRIVATE_DATA p_adapter,UINT32 dw_reg_offset)
{
	UINT32 result=0xFFFFFFFFUL;
	//UINT32 dw_temp=0;
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);
	
	//wait until not busy.
	if (lan_get_reg_dw(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
	{
		SMSC_WARNING("%s()=>failed, MAC already busy at entry.\n", __FUNCTION__);
		goto DONE;
	}

	//send the MAC Cmd w/ offset.
	lan_set_reg_dw(MAC_CSR_CMD,
		((dw_reg_offset & 0x000000FFUL) | 
		MAC_CSR_CMD_CSR_BUSY_ | MAC_CSR_CMD_R_NOT_W_));

	//Should not Read BYTE_TEST.FM-20090110
	//dwTemp=Lan_GetRegDW(BYTE_TEST);//to flush previous write
	//dwTemp=dwTemp;

	//wait for the read to happen, w/ timeout.
	if (!mac_not_busy(p_adapter))
	{
		SMSC_WARNING("%s()=>failed, waiting for MAC not busy after read.\n", __FUNCTION__);
		goto DONE;
	} else {
		//finally, return the read data.
		result = lan_get_reg_dw(MAC_CSR_DATA);
	}
	
DONE:
	return result;
}


void 
mac_set_reg32(PPRIVATE_DATA p_adapter,UINT32 dw_reg_offset,UINT32 dw_val)
{
	//UINT32 dw_temp=0;
	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);

	if (lan_get_reg_dw(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
	{
		SMSC_WARNING("%s()=>failed, MAC already busy at entry.\n", __FUNCTION__);
		goto DONE;
	}

	//send the data to write.
	lan_set_reg_dw(MAC_CSR_DATA,dw_val);

	//do the actual write.
	lan_set_reg_dw(MAC_CSR_CMD,((dw_reg_offset & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_));
	
	//dwTemp=Lan_GetRegDW(BYTE_TEST);//force flush of previous write
	//dwTemp=dwTemp;

	//wait for the write to complete, w/ timeout.
	if (!mac_not_busy(p_adapter))
	{
		SMSC_WARNING("%s()=>failed, waiting for MAC not busy after write", __FUNCTION__);
	}
	
DONE:
	return;
}


UINT32
mac_get_print(PPRIVATE_DATA p_adapter, UINT32 off)
{
	UINT32 tmp_32 = mac_get_reg32(p_adapter, off);

	SMSC_TRACE("=>Smsc Mac Reg 0x%02x : 0x%08x.\n", off, tmp_32);
	return tmp_32;
}


void mac_reg_test(PPRIVATE_DATA p_adapter)
{
	//UINT16 tmp_16;
	//UINT32 tmp_32;
	UINT32 i;

	//Smsc io Regs test
	SMSC_TRACE("Smsc Mac Regs Write&Read Test: \n");

	for(i = 1; i < 0xf; i += 1)
	{
		SMSC_TRACE("=>Smsc Mac Reg 0x%02x : 0x%08x", i, mac_get_reg32(p_adapter,i));
		mac_set_reg32(p_adapter,i, 0x00000000);
		SMSC_TRACE("=>=======>after write 0 : 0x%02x : 0x%08x", i, mac_get_reg32(p_adapter,i));
		mac_set_reg32(p_adapter,i, 0xffffffff);
		SMSC_TRACE("=>=======>after write 1 : 0x%02x : 0x%08x.\n", i, mac_get_reg32(p_adapter,i));
	}

	//asm("sdbbp");
	SDBBP();
}


void 
rx_receiver_on(PPRIVATE_DATA p_adapter)
{
	//UINT32 dw_int_flags=0;

	if(p_adapter->rx_off_count>0) 
	{
		p_adapter->rx_off_count--;
		if(p_adapter->rx_off_count==0) 
		{
			lan_mutex_lock(p_adapter->mac_phy_lock);
			UINT32 dw_mac_cr=mac_get_reg32(p_adapter,MAC_CR);

			//Mac working mode=>Promiscuous.
			dw_mac_cr &= 0xFFF0FFFF;
			dw_mac_cr |= MAC_CR_PRMS_;
			
			if(dw_mac_cr&MAC_CR_RXEN_) 
			{
				SMSC_WARNING("%()=>Receiver is already on.\n",__FUNCTION__);
			}
			dw_mac_cr|=MAC_CR_RXEN_;
			mac_set_reg32(p_adapter,MAC_CR,dw_mac_cr);
			//SET_GPIO(GP_RX);
			lan_mutex_unlock(p_adapter->mac_phy_lock);
		}
	} 
	else 
	{
		SMSC_WARNING("%()=>RxOffCount<=0.\n",__FUNCTION__);
		SMSC_ASSERT(FALSE);
	}

}


//Get Rx Status info from Rx Status FIFO. 
static UINT32 
rx_pop_rx_status(PPRIVATE_DATA p_adapter)
{
	
	UINT32 result;
	//int i=0;
		
	result=lan_get_reg_dw(RX_FIFO_INF);
	
	//Check If Rx Status Fifo is Available.
	if(result&0x00FF0000UL)
		result=lan_get_reg_dw(RX_STATUS_FIFO);
	else 
		result=0;

	return result;
}


//Record Rx Error status.
void 
rx_count_errors(PPRIVATE_DATA p_adapter,UINT32 dw_rx_status) 
{
	BOOL crc_error=FALSE;
	
	//bit15(Error Status)
	if(dw_rx_status&0x00008000UL) {
		p_adapter->stats.rx_errors++;
		//bit1(CRC Error)
		if(dw_rx_status&0x00000002UL) {
			p_adapter->stats.rx_crc_errors++;
			crc_error=TRUE;
		}
	}
	if(!crc_error) {
		//bit5(Frame Type) bit12(Length Error)
		if((dw_rx_status&0x00001020UL)==0x00001020UL) {
			p_adapter->stats.rx_length_errors++;
		}
		//bit10(Multicast Frame)
		if(dw_rx_status&RX_STS_MCAST_) {
			p_adapter->stats.multicast++;
		}
	}
}


void 
rx_fast_forward(PPRIVATE_DATA p_adapter,UINT32 dw_dword_count)
{
	
	if(dw_dword_count>=4)
	{
		UINT32 dw_time_out=500;
		lan_set_reg_dw(RX_DP_CTRL,(dw_dword_count|RX_DP_CTRL_FFWD_BUSY_));
		while((dw_time_out)&&(lan_get_reg_dw(RX_DP_CTRL)&RX_DP_CTRL_FFWD_BUSY_))
		{
			osal_delay(1);
			dw_time_out--;
		}
		if(dw_time_out==0) 
		{
			SMSC_WARNING("%s()=>timed out waiting for RX FFWD to finish, RX_DP_CTRL=0x%08lX.\n",
				__FUNCTION__, lan_get_reg_dw(RX_DP_CTRL));
		}
	} 
	else 
	{
		while(dw_dword_count) {
			UINT32 dw_temp=lan_get_reg_dw(RX_DATA_FIFO);
			dw_temp=dw_temp;
			dw_dword_count--;
		}
	}
}


void 
rx_receive_packets(PPRIVATE_DATA p_adapter)
{
	UINT32 dw_rx_status=0;
	UINT32 packet_length;
	int cnt =0;

	lan_set_reg_dw(RX_CFG,RX_CFG_RXDOFF_2_);
	
	cnt =0;			
	while((dw_rx_status=rx_pop_rx_status(p_adapter))!=0)
	{
		UINT32 dw_packet_length=((dw_rx_status&0x3FFF0000UL)>>16);

		//SMSC_TRACE("%s()=>PacketLength(%d)", __FUNCTION__, (UINT32) dwPacketLength);
		//SMSC_TRACE("RxStatus(0x%08lx).\n", (UINT32) dwRxStatus);
			
		rx_count_errors(p_adapter,dw_rx_status);

		//If No Rx Errors.
		if((dw_rx_status&RX_STS_ES_)==0)
	 	{
			p_adapter->stats.rx_packets++;

			//Rx Checksum(2) FCS(4)
			if (p_adapter->use_rx_csum)
			{
				packet_length = dw_packet_length-4-2;
			}
			else
			{	
				packet_length = dw_packet_length-4;
			}

			p_adapter->stats.rx_bytes+=packet_length;

			//Lan_MutexLock(pAdapter->TransMutex);
			//Rx Data Offset(2) Padding(3) 
			smsc_read_data_fifo(p_adapter->transfer_mode, p_adapter->rx_buffer, (dw_packet_length+2+3)>>2);
			//Lan_MutexUnlock(pAdapter->TransMutex);
				
			//Lan_PrintBuf((UINT8 *)(pAdapter->RxBuffer)+2, PacketLength);
				
			if (p_adapter->dev->callback)
			{
				struct net_pkt_t cur_rx_pkt;
					
				cur_rx_pkt.buffer = (UINT8 *)(p_adapter->rx_buffer)+2;
				cur_rx_pkt.length = packet_length;
					
				p_adapter->dev->callback(p_adapter->dev, NET_RECV_PKT, (UINT32)&cur_rx_pkt);
			}
				
			cnt ++;
			if(cnt > SMSC_RXPKTS_PERTIME)
				break;

		}
		else //Rx Error happens
		{
			SMSC_WARNING("%s()=>Rx Error happens.", __FUNCTION__);
			SMSC_TRACE("RxStatus(0x%08lx).\n", (UINT32) dw_rx_status);
			//if we get here then the packet is to be read
			//	out of the fifo and discarded
			//Rx Data Offset(2) Padding(3) 
			dw_packet_length+=(2+3);
			dw_packet_length>>=2;
			rx_fast_forward(p_adapter,dw_packet_length);
		}
	}

	//Clear Rx Int Status.
	lan_set_reg_dw(INT_STS,INT_STS_RSFL_);
}


void rx_interrupt(PPRIVATE_DATA p_adapter, UINT32 dw_int_sts)
{
	//pAdapter->LastReasonForReleasingCPU=0;
	//bit14=>Rx Err Int.
	if(dw_int_sts&INT_STS_RXE_) 
	{
		SMSC_TRACE("%s()=>RXE signalled.\n",__FUNCTION__);
		p_adapter->stats.rx_errors++;
		lan_set_reg_dw(INT_STS,INT_STS_RXE_);
	}

	//bit23=>Rx Drop Frame Halfway Int.
	if(dw_int_sts&INT_STS_RXDFH_INT_) 
	{
		SMSC_TRACE("%s()=>RXDFH signalled.\n",__FUNCTION__);
		p_adapter->stats.rx_dropped+=lan_get_reg_dw(RX_DROP);
		lan_set_reg_dw(INT_STS,INT_STS_RXDFH_INT_);
	}

	//bit6=>Rx Drop Frame Int.
	if(dw_int_sts&(INT_STS_RDFO_)) 
	{
		SMSC_TRACE("%s()=>RXDF signalled.\n",__FUNCTION__);
		lan_set_reg_dw(INT_STS,INT_STS_RDFO_);
	}

	//bit3=>Rx Status Fifo Level Int.	
	if(dw_int_sts&INT_STS_RSFL_)
	{
		rx_receive_packets(p_adapter);
	}
	
}


/*FM-20091214
Added for Rx flow control.
*/
BOOL rx_stop_interrupt(PPRIVATE_DATA private_data, DWORD dw_int_sts)
{
	if(dw_int_sts&INT_STS_RXSTOP_INT_) 
	{
		SMSC_TRACE("Rx Stopped...\n");
		lan_set_reg_dw(INT_STS,INT_STS_RXSTOP_INT_);
		lan_disable_interrupt(private_data,INT_EN_RXSTOP_INT_EN_);
	}
    return TRUE;
}


//Remove Rx FC.FM-20091216
void 
rx_initialize(PPRIVATE_DATA p_adapter)
{
	if(int_deas!=0xFFFFFFFFUL)
		lan_set_intdeas(p_adapter,int_deas);

	if(0)//Set vlan tag
	{
		//UINT32 dw_int_flags=0;
		lan_mutex_lock(p_adapter->mac_phy_lock);
		//Mac_SetReg32(pAdapter,VLAN1,ETH_P_8021Q);
		lan_mutex_unlock(p_adapter->mac_phy_lock);
		//pAdapter->RxVLanPkt=TRUE;		
	}

	//Rx checksum.
	if (p_adapter->use_rx_csum)
	{
		//UINT32 dw_int_flags=0;
		lan_mutex_lock(p_adapter->mac_phy_lock);
		
		UINT32 dw_coe_cr=mac_get_reg32(p_adapter,COE_CR);		
		//dwCoeCr|=(RX_COE_EN |  RX_COE_MODE);
		//do not care VLAN tag or SNAP header.FM-20081231
		dw_coe_cr|=(RX_COE_EN);
		mac_set_reg32(p_adapter,COE_CR,dw_coe_cr);
		lan_mutex_unlock(p_adapter->mac_phy_lock);
	}

	//initially the receiver is off
	//a following link up detection will turn the receiver on
	p_adapter->rx_off_count=1;

	lan_set_reg_dw(RX_CFG,RX_CFG_RXDOFF_2_);
	rx_receiver_on(p_adapter);
	
	//Lan_SetRDFL(pAdapter,0x01);
	lan_set_rsfl(p_adapter,0x00);
	p_adapter->rx_interrupts=INT_EN_RSFL_EN_;	//rx status fifo level en
	p_adapter->rx_interrupts|=INT_EN_RXE_EN_;	//rx err int en
	p_adapter->rx_interrupts|=INT_EN_RDFO_EN_;	//rx data fifo overflow(RXDF) en
	p_adapter->rx_interrupts|=INT_EN_RXDFH_INT_EN_;//RX Drop Fram Halfway en

	lan_enable_interrupt(p_adapter,p_adapter->rx_interrupts);
}


void 
tx_initialize(PPRIVATE_DATA p_adapter)
{
	UINT32 dw_reg_val=0;

	dw_reg_val=lan_get_reg_dw(HW_CFG);
	//HW_CFG=>bit20(Must Be One) bit19:16(TX FIFO Size)
	dw_reg_val&=(HW_CFG_TX_FIF_SZ_|0x00000FFFUL);
	dw_reg_val|=HW_CFG_SF_;
	lan_set_reg_dw(HW_CFG,dw_reg_val);

	if(p_adapter->use_tx_csum)
	//Set TX COE
 	{
		//UINT32 dw_int_flags=0;
		lan_mutex_lock(p_adapter->mac_phy_lock);
		
		UINT32 dw_coe_cr=mac_get_reg32(p_adapter,COE_CR);
		
		dw_coe_cr|=(TX_COE_EN);
		mac_set_reg32(p_adapter,COE_CR,dw_coe_cr);
		
		//SMSC_TRACE("%s()=>COE_CR = 0x%08lx\n", __FUNCTION__, Mac_GetReg32(pAdapter,COE_CR));
		lan_mutex_unlock(p_adapter->mac_phy_lock);	
	}	
	
	//Set TX Data Available Level to Highest level.
	lan_set_tdfl(p_adapter,0xFF);
	lan_enable_interrupt(p_adapter,INT_EN_TDFA_EN_);

	{//En Tx
		//UINT32 dw_int_flags=0;
		lan_mutex_lock(p_adapter->mac_phy_lock);
		
		UINT32 dw_mac_cr=mac_get_reg32(p_adapter,MAC_CR);
		//dwMacCr|=(MAC_CR_TXEN_|MAC_CR_HBDIS_);
		dw_mac_cr|=MAC_CR_TXEN_;
		mac_set_reg32(p_adapter,MAC_CR,dw_mac_cr);
		lan_set_reg_dw(TX_CFG,TX_CFG_TX_ON_);
		
		lan_mutex_unlock(p_adapter->mac_phy_lock);	
	}

}


static UINT32 
tx_get_status_count(PPRIVATE_DATA p_adapter)
{
	UINT32 result=0;

	result=lan_get_reg_dw(TX_FIFO_INF);

	result&=TX_FIFO_INF_TSUSED_;
	result>>=16;

	return result;
}


static UINT32 
tx_complete(PPRIVATE_DATA p_adapter)
{
	UINT32 result=0;

	result=lan_get_reg_dw(TX_FIFO_INF);
	
	result&=TX_FIFO_INF_TSUSED_;
	if(result!=0x00000000UL) {
		result=lan_get_reg_dw(TX_STATUS_FIFO);
	} else {
		result=0;
	}

	return result;
}


int 
tx_update_counters(PPRIVATE_DATA p_adapter)
{
	int re = 0;
	UINT32 dw_tx_status=0;

	while((dw_tx_status=tx_complete(p_adapter))!=0)
	{
		if(dw_tx_status&0x80000000UL) 
		{
			SMSC_WARNING("Tx_UpdateCounters()=>Packet tag reserved bit is high.\n");
			p_adapter->stats.tx_errors++; re = -1;
		} 
		else if(dw_tx_status&0x00007080UL) 
		{
			SMSC_WARNING("Tx_UpdateCounters()=>Tx Status reserved bits are high.\n");
			p_adapter->stats.tx_errors++; re = -1;
		} 
		else 
		{
			if(dw_tx_status&0x00008000UL) 
			{
				//not care No Carrier Errors.
				if(!(dw_tx_status&0x00000400UL))
				{
					SMSC_WARNING("Tx_UpdateCounters()=>Tx errors(0x%08x).\n", dw_tx_status);
					p_adapter->stats.tx_errors++; re = -1;
				}
			} 
			else 
			{
				p_adapter->stats.tx_packets++;
				p_adapter->stats.tx_bytes+=(dw_tx_status>>16);
			}
			
			if(dw_tx_status&0x00000100UL) 
			{
				p_adapter->stats.collisions+=16;
				p_adapter->stats.tx_aborted_errors+=1;
			}
			else 
			{
				p_adapter->stats.collisions+=
					((dw_tx_status>>3)&0xFUL);
			}
			
			if(dw_tx_status&0x00000800UL) 
			{
				p_adapter->stats.tx_carrier_errors+=1;
			}
			
			if(dw_tx_status&0x00000200UL) 
			{
				p_adapter->stats.collisions++;
				p_adapter->stats.tx_aborted_errors++;
			}
		}
	}

	return re;

}


//it's maybe ugly.FM-20090102
int
tx_send_buf(PPRIVATE_DATA p_adapter, void *buf, UINT16 len)
{
	UINT32 __MAYBE_UNUSED__ data_off = 0;
	int re = 0;
	
#ifndef SMSC_UP_BUS	
	//Enter Mutex
	if(smsc_mutex_id.lock)
	{
		smsc_mutex_id.lock();
		//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
		HAL_GPIO_BIT_SET(SMSC_CS, 0);	
	}
#endif //SMSC_UP_BUS

	if((len & 0x7ff) < 14 || (len & 0x7ff) > 1514)
	{
		SMSC_WARNING("%s()=>pkt len err(%d).\n",__FUNCTION__, len);
		re = -1;
		goto tx_err;
	}

	data_off = ((UINT32)(buf) & 0x03UL);
	
	smsc_write_data_fifo(p_adapter, buf, len);
	
	if(tx_get_status_count(p_adapter)>=30)
	{
		tx_update_counters(p_adapter);
	}

tx_err:	
	
#ifndef SMSC_UP_BUS	
	//Exit Mutex
	if(smsc_mutex_id.unlock)
	{
		//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
		HAL_GPIO_BIT_SET(SMSC_CS, 1);
		smsc_mutex_id.unlock();
	}
#endif //SMSC_UP_BUS
	
	return re;
}


BOOL
tx_start_interrupt(PPRIVATE_DATA p_adapter, UINT32 dw_int_sts)
{
	if(dw_int_sts&INT_STS_TDFA_) 
	{
		//SMSC_TRACE("Tx Started...\n");
		lan_set_tdfl(p_adapter,0xFF);
		lan_set_reg_dw(INT_STS,INT_STS_TDFA_);
	}
    return TRUE;
}


UINT16 
 phy_get_reg16(PPRIVATE_DATA p_adapter,UINT32 dw_reg_index)
{
	UINT32 dw_addr=0;
	int i=0;
	UINT16 result=0xFFFFU;

	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);

	// confirm MII not busy
	if ((mac_get_reg32(p_adapter, MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
	{
		SMSC_WARNING("%s()=>MII is busy???\n", __FUNCTION__);
		result=0;
		goto DONE;
	}

	// set the address, index & direction (read from PHY)
	dw_addr = ((p_adapter->phy_addr&0x1FUL)<<11) | ((dw_reg_index & 0x1FUL)<<6);
	mac_set_reg32(p_adapter, MII_ACC, dw_addr);

	//Mac_GetPrint(pAdapter, MII_ACC);

	// wait for read to complete w/ timeout
	for(i=0;i<100;i++) {
		// see if MII is finished yet
		if ((mac_get_reg32(p_adapter, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{
			// get the read data from the MAC & return i
			result=((WORD)mac_get_reg32(p_adapter, MII_DATA));
			goto DONE;
		}
	}
	SMSC_WARNING("%s()=>timeout waiting for MII write to finish.\n", __FUNCTION__);

DONE:
	return result;
}

void phy_set_reg16(PPRIVATE_DATA p_adapter,UINT32 dw_reg_index,UINT16 w_val)
{
	UINT32 dw_addr=0;
	int i=0;

	SMSC_ASSERT(p_adapter!=NULL);
	SMSC_ASSERT(p_adapter->lan_initialized==TRUE);

	if(dw_reg_index==0) {
		if((w_val&0x1200)==0x1200) {
			p_adapter->last_advat_restart=p_adapter->w_last_adv;
		}
	}
	if(dw_reg_index==4) {
		p_adapter->w_last_adv=w_val;
	}

	// confirm MII not busy
	if ((mac_get_reg32(p_adapter, MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
	{
		SMSC_WARNING("%s()=>MII is busy???\n", __FUNCTION__);
		goto DONE;
	}

	// put the data to write in the MAC
	mac_set_reg32(p_adapter, MII_DATA, (UINT32)w_val);

	// set the address, index & direction (write to PHY)
	dw_addr = ((p_adapter->phy_addr&0x1FUL)<<11) | ((dw_reg_index & 0x1FUL)<<6) | MII_ACC_MII_WRITE_;
	mac_set_reg32(p_adapter, MII_ACC, dw_addr);

	// wait for write to complete w/ timeout
	for(i=0;i<100;i++) {
		// see if MII is finished yet
		if ((mac_get_reg32(p_adapter, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{
			goto DONE;
		}
	}
	SMSC_WARNING("%s()=>timeout waiting for MII write to finish.\n", __FUNCTION__);
DONE:
	return;
}


void
phy_reg_test(PPRIVATE_DATA p_adapter)
{
	//UINT16 tmp_16;
	UINT32 i;

	//Smsc io Regs test
	SMSC_TRACE("Smsc Phy Regs Write&Read Test: \n");

	for(i = 0; i < 31; i += 1)
	{
		if(1)
			SMSC_TRACE("=>Smsc Phy Reg %d: 0x%04x.\n", i, phy_get_reg16(p_adapter,i));
		else
		{
			SMSC_TRACE("=>Smsc Phy Reg %d: 0x%04x", i, phy_get_reg16(p_adapter,i));
			phy_set_reg16(p_adapter,i, 0x0000);
			SMSC_TRACE("=>=======>after write 0: 0x%04x", phy_get_reg16(p_adapter,i));
			phy_set_reg16(p_adapter,i, 0xffff);
			SMSC_TRACE("=>=======>after write 1: 0x%04x.\n", phy_get_reg16(p_adapter,i));
		}
	}

	//asm("sdbbp");
	SDBBP();
}


void 
phy_get_link_mode(PPRIVATE_DATA p_adapter)
	
{
	UINT32 result=LINK_OFF;
	UINT16 w_reg_val=0;

	UINT16 w_reg_bsr=phy_get_reg16(p_adapter,PHY_BSR);
	p_adapter->link_settings=LINK_OFF;

	//Check if Link Status is UP.
	if(w_reg_bsr&PHY_BSR_LINK_STATUS_) 
	{
		w_reg_val=phy_get_reg16(p_adapter,PHY_BCR);
		
		//Check if Auto-Neg is Enable.
		if(w_reg_val&PHY_BCR_AUTO_NEG_ENABLE_) 
		{
			UINT32 link_settings=LINK_AUTO_NEGOTIATE;
			UINT16 w_reg_adv=p_adapter->last_advat_restart;
//					Phy_GetRegW(
//						pAdapter,
//						PHY_ANEG_ADV,keyCode);
			UINT16 w_reg_lpa=phy_get_reg16(p_adapter,PHY_ANEG_LPA);
			if(w_reg_adv&PHY_ANEG_ADV_ASYMP_) {
				link_settings|=LINK_ASYMMETRIC_PAUSE;//one-direction Pause Fame
			}
			if(w_reg_adv&PHY_ANEG_ADV_SYMP_) {
				link_settings|=LINK_SYMMETRIC_PAUSE;//both-direction Pause Fame
			}
			if(w_reg_adv&PHY_ANEG_LPA_100FDX_) {
				link_settings|=LINK_SPEED_100FD;
			}
			if(w_reg_adv&PHY_ANEG_LPA_100HDX_) {
				link_settings|=LINK_SPEED_100HD;
			}
			if(w_reg_adv&PHY_ANEG_LPA_10FDX_) {
				link_settings|=LINK_SPEED_10FD;
			}
			if(w_reg_adv&PHY_ANEG_LPA_10HDX_) {
				link_settings|=LINK_SPEED_10HD;
			}
			p_adapter->link_settings=link_settings;
			w_reg_lpa&=w_reg_adv;
			if(w_reg_lpa&PHY_ANEG_LPA_100FDX_) {
				result=LINK_SPEED_100FD;
			} else if(w_reg_lpa&PHY_ANEG_LPA_100HDX_) {
				result=LINK_SPEED_100HD;
			} else if(w_reg_lpa&PHY_ANEG_LPA_10FDX_) {
				result=LINK_SPEED_10FD;
			} else if(w_reg_lpa&PHY_ANEG_LPA_10HDX_) {
				result=LINK_SPEED_10HD;
			}
		} else {
			if(w_reg_val&PHY_BCR_SPEED_SELECT_) {
				if(w_reg_val&PHY_BCR_DUPLEX_MODE_) {
					p_adapter->link_settings=result=LINK_SPEED_100FD;
				} else {
					p_adapter->link_settings=result=LINK_SPEED_100HD;
				}
			} else {
				if(w_reg_val&PHY_BCR_DUPLEX_MODE_) {
					p_adapter->link_settings=result=LINK_SPEED_10FD;
				} else {
					p_adapter->link_settings=result=LINK_SPEED_10HD;
				}
			}
		}
	}
	p_adapter->link_speed=result;
}


/*
FM-20090105.
Step 1=>Check Link Mode & Set Advertisement.(PHY_ANEG_ADV)
Step 2=>Start Auto-Negotiation.(PHY_BCR).
*/
void 
phy_set_link(PPRIVATE_DATA p_adapter) 
{
	//UINT32 dw_int_flags=0;
	UINT32 dw_link_request = p_adapter->link_mode;
	UINT16 w_temp=0;
	
	lan_mutex_lock(p_adapter->mac_phy_lock);
	
	if(dw_link_request&LINK_AUTO_NEGOTIATE) 
	{
		//Check Link Mode & Set Advertisement.
		w_temp=phy_get_reg16(p_adapter,PHY_ANEG_ADV);
		w_temp&=~PHY_ANEG_ADV_PAUSE_;
		if(dw_link_request&LINK_ASYMMETRIC_PAUSE) {
			w_temp|=PHY_ANEG_ADV_ASYMP_;
		} 
		if(dw_link_request&LINK_SYMMETRIC_PAUSE) {
			w_temp|=PHY_ANEG_ADV_SYMP_;
		}
		w_temp&=~PHY_ANEG_ADV_SPEED_;
		if(dw_link_request&LINK_SPEED_10HD) {
			w_temp|=PHY_ANEG_ADV_10H_;
		}
		if(dw_link_request&LINK_SPEED_10FD) {
			w_temp|=PHY_ANEG_ADV_10F_;
		}
		if(dw_link_request&LINK_SPEED_100HD) {
			w_temp|=PHY_ANEG_ADV_100H_;
		}
		if(dw_link_request&LINK_SPEED_100FD) {
			w_temp|=PHY_ANEG_ADV_100F_;
		}
		phy_set_reg16(p_adapter,PHY_ANEG_ADV,w_temp);

		//Start Auto-Negotiation
		phy_set_reg16(p_adapter,PHY_BCR,
			PHY_BCR_AUTO_NEG_ENABLE_|PHY_BCR_RESTART_AUTO_NEG_);
	}
	else //if(dwLinkRequest&LINK_AUTO_NEGOTIATE) 
	{
		if(dw_link_request&(LINK_SPEED_100FD)) {
			dw_link_request=LINK_SPEED_100FD;
		} else if(dw_link_request&(LINK_SPEED_100HD)) {
			dw_link_request=LINK_SPEED_100HD;
		} else if(dw_link_request&(LINK_SPEED_10FD)) {
			dw_link_request=LINK_SPEED_10FD;
		} else if(dw_link_request&(LINK_SPEED_10HD)) {
			dw_link_request=LINK_SPEED_10HD;
		}
		if(dw_link_request&(LINK_SPEED_10FD|LINK_SPEED_100FD)) {
			w_temp|=PHY_BCR_DUPLEX_MODE_;
		}
		if(dw_link_request&(LINK_SPEED_100HD|LINK_SPEED_100FD)) {
			w_temp|=PHY_BCR_SPEED_SELECT_;
		}
		phy_set_reg16(p_adapter,PHY_BCR,w_temp);
	}
	lan_mutex_unlock(p_adapter->mac_phy_lock);
}


/*
Set AutoMdix Mode.
	0: Override Strap, Disable AutoMdix, Using Straight Cable
	1: Override Strap, Disable AutoMdix, Using CrossOver Cable
	2: Override Strap, Enable AutoMdix
	>=3 or No Keyword: AutoMdix controlled by Strap
*/
void 
phy_set_auto_mdix_sts(PPRIVATE_DATA p_adapter)
{
	UINT16 w_auto_mdix_sts = p_adapter->auto_mdix;
	UINT16 special_ctrl_sts=0;
	//UINT32 dw_int_flags=0;

	//AutoMdix controlled by Strap(pin73)
	if (w_auto_mdix_sts > 2)
	{
		lan_mutex_lock(p_adapter->mac_phy_lock);
		//Phy_RegTest(pAdapter);
		special_ctrl_sts=phy_get_reg16(p_adapter, SPECIAL_CTRL_STS);
		special_ctrl_sts = (special_ctrl_sts&0x1FFF);
		phy_set_reg16(p_adapter, SPECIAL_CTRL_STS,special_ctrl_sts);
		lan_mutex_unlock(p_adapter->mac_phy_lock);

		if (lan_get_reg_dw(HW_CFG) & HW_CFG_AMDIX_EN_STRAP_STS_) 
			SMSC_TRACE("%s()=>Auto-MDIX Enable by default!!!\n", __FUNCTION__);
		else 
			SMSC_TRACE("%s()=>Auto-MDIX Disable by default!!!\n", __FUNCTION__);
       }
	else 
	{
		lan_mutex_lock(p_adapter->mac_phy_lock);
		special_ctrl_sts=phy_get_reg16(p_adapter, SPECIAL_CTRL_STS);
		//bit15(Override AMDIX Strap) bit14(AutoMdix En) bit13(AutoMdix State)
		special_ctrl_sts = (((w_auto_mdix_sts+4) << 13) | (special_ctrl_sts&0x1FFF));
		phy_set_reg16(p_adapter, SPECIAL_CTRL_STS,special_ctrl_sts);
		lan_mutex_unlock(p_adapter->mac_phy_lock);	

		if (w_auto_mdix_sts & AMDIX_ENABLE) 
			SMSC_TRACE("%()=>Override Strap, Enable Auto-MDIX.\n", __FUNCTION__);
		else if (w_auto_mdix_sts & AMDIX_DISABLE_CROSSOVER) 
			SMSC_TRACE("%()=>Override Strap, Disable Auto-MDIX, CrossOver Cable.\n", __FUNCTION__);		
		else 
			SMSC_TRACE("%()=>Override Strap, Disable Auto-MDIX, Straight Cable.\n", __FUNCTION__);		
	}

}


/*
Step 1=>Update Link Status.
Step 2=>If it changed, Update Mac working mode.
Step 3=>Flow Control.(If it Needs)
*/
void 
phy_update_link_mode(PPRIVATE_DATA p_adapter)
{
	UINT32 dw_old_link_speed=p_adapter->link_speed;
	//UINT32 dw_int_flags=0;
	
	lan_mutex_lock(p_adapter->mac_phy_lock);

	phy_get_link_mode(p_adapter);

	//If Link Status Changed.
	if(dw_old_link_speed!=(p_adapter->link_speed)) 
	{
		//Link Status is ON.
		if(p_adapter->link_speed!=LINK_OFF) 
		{
			UINT32 dw_reg_val=0;
			
			switch(p_adapter->link_speed) 
			{
				case LINK_SPEED_10HD:
					SMSC_TRACE("%s()=>Link is now UP at 10Mbps HD.\n",__FUNCTION__);
					break;
				case LINK_SPEED_10FD:
					SMSC_TRACE("%s()=>Link is now UP at 10Mbps FD.\n",__FUNCTION__);
					break;
				case LINK_SPEED_100HD:
					SMSC_TRACE("%s()=>Link is now UP at 100Mbps HD.\n",__FUNCTION__);
					break;
				case LINK_SPEED_100FD:
					SMSC_TRACE("%s()=>Link is now UP at 100Mbps FD.\n",__FUNCTION__);
					break;
				default:
					SMSC_WARNING("%s()=>Link is now UP at Unknown Link Speed, LinkSpeed=0x%08lX.\n",
						__FUNCTION__, p_adapter->link_speed);
					break;
			}
		
			dw_reg_val=mac_get_reg32(p_adapter,MAC_CR);
			dw_reg_val&=~(MAC_CR_FDPX_|MAC_CR_RCVOWN_);
			
			switch(p_adapter->link_speed) 
			{
				case LINK_SPEED_10HD:
				case LINK_SPEED_100HD:
					dw_reg_val|=MAC_CR_RCVOWN_;
					break;
				case LINK_SPEED_10FD:
				case LINK_SPEED_100FD:
					dw_reg_val|=MAC_CR_FDPX_;
					break;
				default:break;//make lint happy
			}

			mac_set_reg32(p_adapter,MAC_CR,dw_reg_val);

			//if(pAdapter->LinkSettings&LINK_AUTO_NEGOTIATE)
			if(0) //Just Dis FC.FM-20090106. 
			{
				UINT16 link_partner=0;
				UINT16 local_link=0;
				local_link=phy_get_reg16(p_adapter,4);
				link_partner=phy_get_reg16(p_adapter,5);
				switch(p_adapter->link_speed) 
				{
					case LINK_SPEED_10FD:
					case LINK_SPEED_100FD:
						//Check if Smmetric PAUSE bit has been set by both sides.
						if(((local_link&link_partner)&((UINT16)0x0400U)) != ((UINT16)0U)) 
						{
							//Enable PAUSE receive and transmit
							//FLOW=>bit2(Flow Control Enable) bit31:16(Pause Time)
							mac_set_reg32(p_adapter,FLOW,0xFFFF0002UL);
							lan_set_bits_dw(AFC_CFG,(p_adapter->afc_config&0x0000000FUL));
						} 
						else if(((local_link&((UINT16)0x0C00U))==((UINT16)0x0C00U)) &&
								((link_partner&((UINT16)0x0C00U))==((UINT16)0x0800U)))
						{
							//Enable PAUSE receive, disable PAUSE transmit
							mac_set_reg32(p_adapter,FLOW,0xFFFF0002UL);
							lan_clr_bits_dw(AFC_CFG,0x0000000FUL);
						} 
						else 
						{
							//Disable PAUSE receive and transmit
							mac_set_reg32(p_adapter,FLOW,0UL);
							lan_clr_bits_dw(AFC_CFG,0x0000000FUL);
						}
						break;
					case LINK_SPEED_10HD:
					case LINK_SPEED_100HD:
						mac_set_reg32(p_adapter,FLOW,0UL);
						lan_set_bits_dw(AFC_CFG,0x0000000FUL);
						break;
					default:break;//make lint happy
				}
				SMSC_TRACE("%s()=>LAN9220: %s,%s,%s,%s,%s,%s.\n",__FUNCTION__, 
					(local_link&PHY_ANEG_ADV_ASYMP_)?"ASYMP":"     ",
					(local_link&PHY_ANEG_ADV_SYMP_)?"SYMP ":"     ",
					(local_link&PHY_ANEG_ADV_100F_)?"100FD":"     ",
					(local_link&PHY_ANEG_ADV_100H_)?"100HD":"     ",
					(local_link&PHY_ANEG_ADV_10F_)?"10FD ":"     ",
					(local_link&PHY_ANEG_ADV_10H_)?"10HD ":"     ");
	
				SMSC_TRACE("%s()=>Partner: %s,%s,%s,%s,%s,%s.\n",__FUNCTION__, 
					(link_partner&PHY_ANEG_LPA_ASYMP_)?"ASYMP":"     ",
					(link_partner&PHY_ANEG_LPA_SYMP_)?"SYMP ":"     ",
					(link_partner&PHY_ANEG_LPA_100FDX_)?"100FD":"     ",
					(link_partner&PHY_ANEG_LPA_100HDX_)?"100HD":"     ",
					(link_partner&PHY_ANEG_LPA_10FDX_)?"10FD ":"     ",
					(link_partner&PHY_ANEG_LPA_10HDX_)?"10HD ":"     ");
			} 
			else 
			{
				switch(p_adapter->link_speed) 
				{
					case LINK_SPEED_10HD:
					case LINK_SPEED_100HD:
						mac_set_reg32(p_adapter,FLOW,0x0UL);
						lan_set_bits_dw(AFC_CFG,0x0000000FUL);
						break;
					default:
						mac_set_reg32(p_adapter,FLOW,0x0UL);
						lan_clr_bits_dw(AFC_CFG,0x0000000FUL);
						break;
				}
			}
			
		} 
		//Link Status is DOWN.
		else 
		{
			SMSC_TRACE("%s()=>Link is now DOWN.\n", __FUNCTION__);
			//Turn off netif, dis Flow Control, dis Auto Flow Control.
			mac_set_reg32(p_adapter,FLOW,0UL);
			lan_clr_bits_dw(AFC_CFG,0x0000000FUL);

			// Check global setting that LED1 usage is 10/100 indicator
//			g_GpioSetting = Lan_GetRegDW(GPIO_CFG);
			if (g_gpio_setting & GPIO_CFG_LED1_EN_)
			{
				//Force 10/100 LED off, after saving orginal GPIO configuration
				g_gpio_setting_original = g_gpio_setting;

				g_gpio_setting &= ~GPIO_CFG_LED1_EN_;
				g_gpio_setting |=
					(GPIO_CFG_GPIOBUF0_|GPIO_CFG_GPIODIR0_|GPIO_CFG_GPIOD0_);
				lan_set_reg_dw(GPIO_CFG,g_gpio_setting);
			}

		}
	}
	
	lan_mutex_unlock(p_adapter->mac_phy_lock);
}


void phy_check_link(UINT32 ptr)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)ptr;
	if(p_adapter==NULL) {
		SMSC_WARNING("%s()=>(ptr==0).\n",__FUNCTION__);
		return;
	}
	
	//must call this twice
	phy_update_link_mode(p_adapter);
	osal_task_sleep(100);//need to Check more.FM-20090112 
	phy_update_link_mode(p_adapter);
	
	osal_task_sleep(900);

}

void phy_interrupt(PPRIVATE_DATA p_adapter)
{
	UINT16 sts;

	lan_mutex_lock(p_adapter->mac_phy_lock);

	sts = phy_get_reg16(p_adapter, PHY_INT_SRC);
	//SMSC_TRACE("%s()=>Phy Int Src(0x%04x).\n",__FUNCTION__, sts);
	
	if((sts & PHY_INT_MASK_ENERGY_ON_) &&(sts & PHY_INT_MASK_ANEG_COMP_))
	{
		SMSC_TRACE("%s()=>Link is OKay(0x%04x).\n",__FUNCTION__, sts);
		p_adapter->link_status = TRUE;
	}
	
	if((sts & PHY_INT_MASK_ANEG_COMP_) == 0)
	{
		SMSC_TRACE("%s()=>Link is Down(0x%04x).\n",__FUNCTION__, sts);
		p_adapter->link_status = FALSE;
	}		


	lan_mutex_unlock(p_adapter->mac_phy_lock);
}


/*
Step 1=>Set AutoMdix Mode.
Step 2=>Reset Phy.
Step 3=>Auto Negotiation.
*/
RET_CODE
phy_initialize(PPRIVATE_DATA p_adapter)
{
	RET_CODE result=SUCCESS;
	//UINT32 dw_temp=0;
	UINT16 w_temp=0;
	UINT32 dw_loop_count=0;
	//UINT32 dw_int_flags=0;
	//UINT16 SpecialCtrlSts=0U;

	SMSC_ASSERT(p_adapter->link_mode<=0x7FUL);

	p_adapter->link_speed=LINK_OFF;
	p_adapter->link_settings=LINK_OFF;
	
	//auto-MDIX set.
	phy_set_auto_mdix_sts(p_adapter);
	
	lan_mutex_lock(p_adapter->mac_phy_lock);
	//Reset PHY
	phy_set_reg16(p_adapter,PHY_BCR,PHY_BCR_RESET_);
	dw_loop_count=0;
	do {
		SMSC_WARNING("%s()=>Wait %d ms untill Phy Reset completed.\n",__FUNCTION__, 10*dw_loop_count);
		osal_task_sleep(10);
		w_temp=phy_get_reg16(p_adapter,PHY_BCR);
		dw_loop_count++;
	} while((dw_loop_count>1000) && (w_temp&PHY_BCR_RESET_));
	lan_mutex_unlock(p_adapter->mac_phy_lock);

	if(w_temp&PHY_BCR_RESET_) 
	{
		SMSC_WARNING("%s()=>PHY reset failed to complete.\n",__FUNCTION__);
		result = RET_FAILURE;
		goto DONE;
	}
	
	SMSC_WARNING("%s()=>PHY reset complete.\n",__FUNCTION__);
	osal_task_sleep(100);
	
	//Start Auto Negotiation.
	phy_set_link(p_adapter);

	//En Phy Int
	phy_set_reg16(p_adapter, PHY_INT_MASK, (PHY_INT_MASK_ENERGY_ON_ \
		|PHY_INT_MASK_ANEG_COMP_|PHY_INT_MASK_REMOTE_FAULT_|PHY_INT_MASK_LINK_DOWN_));
DONE:
	
	return result;
}


void 
smsc9220_hsr(UINT32 sema_id)
{
	osal_semaphore_release(sema_id);
}


static void 
smsc9220_isr(UINT32 param)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)param;

	HAL_GPIO_INT_CLEAR(SMSC9220_GPIO);			//Clear GPIO int status
	osal_interrupt_register_hsr((OSAL_T_HSR_PROC_FUNC_PTR)(smsc9220_hsr), (UINT32)p_adapter->int_sem);
}


void
smsc9220_task(UINT32 para)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)para;	
	
	//UINT32 dw_int_cfg=0;
	UINT32 dw_int_sts=0;
	UINT32 dw_int_en=0;
	UINT32 dw_int_bits=0;

	while(1)
	{
		UINT32 ret = osal_semaphore_capture(p_adapter->int_sem, /*OSAL_WAIT_FOREVER_TIME*/100);
		if(OSAL_E_OK != ret)
		{
			if(SMSC9220_POLAR == HAL_GPIO_BIT_GET(SMSC9220_GPIO))
			{
				libc_printf("%s()=>Int missed(0x%08x).\n", __FUNCTION__, lan_get_reg_dw(INT_STS));
			}
			else
				continue;
		}
		
#ifndef SMSC_UP_BUS						
		//Enter Mutex
		if(smsc_mutex_id.lock)
		{
			smsc_mutex_id.lock();
			//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
			HAL_GPIO_BIT_SET(SMSC_CS, 0);
		}
#endif //SMSC_UP_BUS

		//Int Handler
			
		dw_int_sts=lan_get_reg_dw(INT_STS);		//int status
		dw_int_en=lan_get_reg_dw(INT_EN);		//int mask
		dw_int_bits=dw_int_sts&dw_int_en;
			
		p_adapter->last_int_status3=p_adapter->last_int_status2;
		p_adapter->last_int_status2=p_adapter->last_int_status1;
		p_adapter->last_int_status1=dw_int_bits;	

		if(dw_int_bits)
		{
			//SMSC_TRACE("%s()=>INT_STS(0x%08x) INT_EN(0x%08x)\n", __FUNCTION__, dwIntSts, dwIntEn);
			
			tx_start_interrupt(p_adapter,dw_int_bits);
			//Rx_StopInterrupt(pAdapter,dwIntBits);
			rx_interrupt(p_adapter,dw_int_bits);

			if(INT_EN_PHY_INT_EN_ & dw_int_sts)
			{
				//SMSC_WARNING("%s()=>PHY interrupt.\n", __FUNCTION__);
				phy_interrupt(p_adapter);
				lan_set_reg_dw(INT_STS,INT_EN_PHY_INT_EN_);
			}
			
		}
		
#ifndef SMSC_UP_BUS		
		//Exit Mutex
		if(smsc_mutex_id.unlock)
		{
			//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
			HAL_GPIO_BIT_SET(SMSC_CS, 1);	
			smsc_mutex_id.unlock();
		}
#endif //SMSC_UP_BUS

	}

}


__ATTRIBUTE_REUSE_
RET_CODE smsc9220_attach(SMSC_CONFIG *cfg)
{
	struct net_device *dev;
	PPRIVATE_DATA p_adapter=NULL;
	//UINT16 tmp_16;
	int i; 
	
	if(NULL==cfg)
		return ERR_FAILUE;

#ifdef SMSC_UP_BUS	
	if(cfg->en_gpio_setting)
	{
		SMSC9220_GPIO = cfg->int_gpio_position;
		SMSC_ADDR7 = cfg->a7_gpio_position;
		SMSC_FIFO_SEL = cfg->sel_gpio_position;
	}

	//Should Set GPIO-Addr before Access Any Regs.
	HAL_GPIO_BIT_DIR_SET(SMSC_FIFO_SEL, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 0);
	
	HAL_GPIO_BIT_DIR_SET(SMSC_ADDR7, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(SMSC_ADDR7, g_smsc_addr7);
#else //SMSC_UP_BUS 	
	//init mutex for ATA and CI Share Pin.
	smsc_mutex_init();

	if(cfg->en_gpio_setting)
	{
		SMSC_CS = cfg->cs_gpio_position;

		SMSC_ADDR7 = cfg->a7_gpio_position;
		SMSC_ADDR6 = cfg->a6_gpio_position;
		SMSC_FIFO_SEL = cfg->sel_gpio_position;
		SMSC9220_GPIO = cfg->int_gpio_position;
		SMSC9220_POLAR = cfg->int_gpio_polar;
	}
	//Should Set GPIO-Addr before Access Any Regs.
	HAL_GPIO_BIT_DIR_SET(SMSC_FIFO_SEL, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(SMSC_FIFO_SEL, 0);	

	HAL_GPIO_BIT_DIR_SET(SMSC_ADDR7, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(SMSC_ADDR7, g_smsc_addr7);
	
	HAL_GPIO_BIT_DIR_SET(SMSC_ADDR6, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(SMSC_ADDR6, g_smsc_addr6);
#endif //SMSC_UP_BUS	

	//alloc memory space for net device.
	dev = (struct net_device *)dev_alloc(smsc9220_name, HLD_DEV_TYPE_NET, sizeof(struct net_device));
	if (dev == NULL)
	{
		SMSC_WARNING("%s()=>alloc net device error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	}

	//alloc memory space for NIC private structure.
	dev->priv=(void *)MALLOC(sizeof(PRIVATE_DATA));
	if(dev->priv==NULL) 
	{
		dev_free(dev);
		SMSC_WARNING("%s()=>alloc ethernet mac private memory error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	}
	MEMSET(dev->priv,0,sizeof(PRIVATE_DATA));

	p_adapter=(PPRIVATE_DATA)(dev->priv);
	g_p_adapter = p_adapter;

	//Get Chip ID
	p_adapter->chip_id = sys_ic_get_chip_id();
#ifdef SMSC_UP_BUS
	if(SMSC9220_GPIO < 32)
		p_adapter->irq = 9;
	else
	{
		if(ALI_M3329E == p_adapter->chip_id)
		{
			if(SMSC9220_GPIO<64)
				p_adapter->irq = 24;
			else
				p_adapter->irq = 31;
		}
		else
			p_adapter->irq = 24;
	}
#else 
	if(ALI_S3602 == p_adapter->chip_id)
	{
		ATA_IO_BASE = _3602_IDE_IO_BASE;
		p_adapter->irq= 8;
	}
	else
	{
		if(ALI_M3329E == p_adapter->chip_id)
		{
			HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
			HAL_GPIO_BIT_SET(SMSC_CS, 0);
		}
		
		ATA_IO_BASE = _3329_IDE_IO_BASE;
		if(SMSC9220_GPIO < 32)
			p_adapter->irq = 9;
		else
		{
			if(ALI_M3329E == p_adapter->chip_id)
			{
				if(SMSC9220_GPIO<64)
					p_adapter->irq = 24;
				else
					p_adapter->irq = 31;
			}
			else
				p_adapter->irq = 24;
		}
	}

	if(cfg->en_ata_setting)
	{
		ata_w32(ATA_PIOTCR, cfg->ata_timing);
	}	
	else
	{
		//ata_w32(ATA_PIOTCR, 0x02190900); //(cnt%2 ==0)
		//ata_w32(ATA_PIOTCR, 0x02190800); //(cnt%3 ==0)
		//ata_w32(ATA_PIOTCR, 0x02200800); //(cnt%5 ==0) 	
		ata_w32(ATA_PIOTCR, 0x02190900);
	}
#endif //SMSC_UP_BUS	

	if(cfg->en_mac_setting)
	{	
		p_adapter->mac_addr_lo32 = 0;
		p_adapter->mac_addr_hi16 =0;
		
		for(i = 0; i < 4; i ++)
			p_adapter->mac_addr_lo32 |= (UINT32)(cfg->mac_addr[i]<<(8*i));

		for(; i < 6; i ++)
			p_adapter->mac_addr_hi16 |= (UINT32)(cfg->mac_addr[i]<<(8*(i-4)));
	}

	dev->next = NULL;

	//init function point.
	//dev->attach = smsc9220_attach;
	dev->detach = smsc9220_detach;
	dev->open = smsc9220_open;
	dev->close = smsc9220_close;
	dev->ioctl = smsc9220_ioctl;
	dev->send_packet = smsc9220_send_packet;
	dev->callback = NULL;
	dev->get_info = smsc9220_get_info;
	dev->set_info = smsc9220_set_info;

	/* Add this device to queue */ 
	if (dev_register(dev) != RET_SUCCESS)
	{
		SMSC_WARNING("%()=>Register net device error!\n", __FUNCTION__);
		FREE(p_adapter);
		dev_free(dev);
		return RET_FAILURE;
	}
    return RET_SUCCESS;
}


RET_CODE 
smsc9220_open(struct net_device*dev, void (*callback)(struct net_device *,UINT32, UINT32))
{
	PPRIVATE_DATA p_adapter=NULL;
	RET_CODE result = SUCCESS;
	UINT32 dw_int_cfg=0;
	//UINT32 k;
	int cnt = 0;

	OSAL_T_CTSK task_param ;

	if(dev==NULL) 
	{
		SMSC_WARNING("%s()=>(dev==NULL)\n", __FUNCTION__);
		result=ERR_FAILUE;
		goto DONE;
	}
	p_adapter=(PPRIVATE_DATA)(dev->priv);
	if(p_adapter==NULL) 
	{
		SMSC_WARNING("%s()=>(pAdapter==NULL)\n", __FUNCTION__);
		result=ERR_FAILUE;
		goto DONE;
	}
	p_adapter->dev = dev;
	
#ifdef SMSC_UP_BUS
	smsc_spe_id = spe_get_dev(SPE_UP_ETH, 0x2f3fa0f1, &smsc_spe_map); 
	//smsc_spe_id = spe_get_dev(SPE_UP_ETH, 0x230e10c1, &smsc_spe_map); //135M
	SMSC_TRACE("%s()=>Get SPE ID(%d) Mapping Addr(0x%08x).\r\n",__FUNCTION__, smsc_spe_id, smsc_spe_map);

	p_adapter->transfer_mode = SMSC_TRANSFER_DMA;
	//pAdapter->TransferMode = SMSC_TRANSFER_PIO;
#else
	//Enter Mutex
	if(smsc_mutex_id.lock)
	{
		smsc_mutex_id.lock();
		//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
		HAL_GPIO_BIT_SET(SMSC_CS, 0);
	}
	
	p_adapter->transfer_mode = SMSC_TRANSFER_PIO;
#endif //SMSC_UP_BUS

	//chip verification.
	p_adapter->id_rev = lan_get_reg_dw(ID_REV);
	SMSC_TRACE("%s()=>IdRev(0x%08lX)\n", __FUNCTION__, p_adapter->id_rev);
	if(0x92200000 != p_adapter->id_rev)
	{
		SMSC_WARNING("%s()=>chip ID error.\n", __FUNCTION__);
		result=ERR_NO_DEV;
		goto DONE;
	}

	p_adapter->tx_fifo_size = 0x00020000UL; 			// 2k bytes.

	p_adapter->debug_mode = 0x0UL; 				// not use debug mode
	//pAdapter->DebugMode = 0x7UL; 				// use debug mode

	if(p_adapter->mac_addr_hi16 == 0)
		p_adapter->mac_addr_hi16 = 0xFFFFFFFFUL; 	//decided by driver.
	
	p_adapter->phy_addr = 1; 						//decided by driver.

	p_adapter->link_mode = LINK_ADVERTISE; 			// advertise all(100M/10M, Full/Half duplex)	
	p_adapter->auto_mdix = 0x2U;					//En Auto Mdix.
	//pAdapter->AutoMdix = 0x0U;					//Straight.
	
	p_adapter->afc_config = 0xFFFFFFFFUL; 			//decided by driver. set to AFC_CFG register.

	p_adapter->use_rx_csum = FALSE;					//RxCOE
	p_adapter->use_tx_csum = FALSE;					//TxCOE
	
	//Specifies the minimum packet size for DMA to be used.
	p_adapter->dma_threshold = PLATFORM_DMA_THRESHOLD;

	//mutex for MAC CSR accessing.
	p_adapter->mac_phy_lock = osal_mutex_create();
	p_adapter->trans_mutex = osal_mutex_create();
	p_adapter->reg_mutex = osal_mutex_create();

	p_adapter->link_status = FALSE;
	
	if ((p_adapter->int_sem = osal_semaphore_create(1))==OSAL_INVALID_ID) 
	{
		SMSC_WARNING("%s()=>semaphore create failure!\n", __FUNCTION__);
		result=ERR_FAILUE;
		goto DONE;
	}
	
	while(OSAL_E_OK!=osal_semaphore_capture(p_adapter->int_sem,1000))
		SMSC_WARNING("%s()=>Capture IntSem timeout !\n", __FUNCTION__);

	cnt = 0;
	while(0)
	{
		if(cnt%100 ==0)
			SMSC_WARNING("%s()=>cnt(%d)\n", __FUNCTION__, cnt);
		cnt ++;
		lan_reg_test();
	}

	if(SUCCESS != lan_initialize(p_adapter))
	{
		SMSC_WARNING("%s()=>Failed lan_initialize.\n", __FUNCTION__);
		result=ERR_FAILUE;
		goto DONE;
	}

	//Interrupt Settings.
	//Disable All Interrupts & Clear Interrupt Status.
	lan_set_reg_dw(INT_EN,0);
	lan_set_reg_dw(INT_STS,0xFFFFFFFFUL);

	//Int Cfg=>IRQ polarity(Low)
	if(SMSC9220_POLAR == 1)
		dw_int_cfg|=INT_CFG_IRQ_POL_;	//IRQ polarity(High)
	//dwIntCfg|=INT_CFG_IRQ_TYPE_;		//IRQ buffer type	
	dw_int_cfg|=INT_CFG_IRQ_EN_;
	//dwIntCfg|=0x22000000;
	lan_set_reg_dw(INT_CFG,dw_int_cfg);
	
	if(1)//get mac address
	{
		UINT32 dw_high16=0;
		UINT32 dw_low32=0;
		//UINT32 dw_int_flags=0;
		//Wait For Lock.
		lan_mutex_lock(p_adapter->mac_phy_lock);

		//Mac_RegTest(pAdapter);
		
		if(p_adapter->mac_addr_hi16==0xFFFFFFFF) 
		{//Set Mac Addr by OurSelves.FM-20090105
			dw_high16=mac_get_reg32(p_adapter,ADDRH);
			dw_low32=mac_get_reg32(p_adapter,ADDRL);
			
			if((dw_high16==0x0000FFFFUL)&&(dw_low32==0xFFFFFFFF))
			{
				dw_high16=0x00000070UL;
				dw_low32=0x110F8000UL;
				mac_set_reg32(p_adapter,ADDRH,dw_high16);
				mac_set_reg32(p_adapter,ADDRL,dw_low32);
				SMSC_TRACE("%s()=>Mac Address is set by default to 0x%04lX%08lX",
					__FUNCTION__,dw_high16,dw_low32);
			} else {
				SMSC_TRACE("%s()=>Mac Address is read from LAN9220 as 0x%04lX%08lX",
					__FUNCTION__,dw_high16,dw_low32);
			}
		} 
		else //Use Mac Address in ADDRH/ADDRL Registers.FM-20090105
		{
			SMSC_TRACE("%s()=>Mac Addr(0x%x%x).\n", __FUNCTION__, p_adapter->mac_addr_hi16, p_adapter->mac_addr_lo32);
			dw_high16=p_adapter->mac_addr_hi16;
			dw_low32=p_adapter->mac_addr_lo32;
			mac_set_reg32(p_adapter,ADDRH,dw_high16);
			mac_set_reg32(p_adapter,ADDRL,dw_low32);
			SMSC_TRACE("%s()=>Mac Address is set by parameter to 0x%04lX%08lX \n",
				__FUNCTION__,dw_high16,dw_low32);
		}
		lan_mutex_unlock(p_adapter->mac_phy_lock);
		p_adapter->dev_addr[0]=LOBYTE(LOWORD(dw_low32));
		p_adapter->dev_addr[1]=HIBYTE(LOWORD(dw_low32));
		p_adapter->dev_addr[2]=LOBYTE(HIWORD(dw_low32));
		p_adapter->dev_addr[3]=HIBYTE(HIWORD(dw_low32));
		p_adapter->dev_addr[4]=LOBYTE(LOWORD(dw_high16));
		p_adapter->dev_addr[5]=HIBYTE(LOWORD(dw_high16));
	}

	if(SUCCESS != phy_initialize(p_adapter))
	{
		SMSC_WARNING("%s()=>Failed to initialize Phy.\n", __FUNCTION__);
		result=ERR_FAILED;
		goto DONE;
	}

	//Phy link test
	cnt = 0;
	while(0)
	{
		cnt ++;
		SMSC_WARNING("%s()=>Cnt(%d).\n",__FUNCTION__, cnt);
		phy_check_link((UINT32)p_adapter);
	}

	//Phy_CheckLink((UINT32)pAdapter);

	tx_initialize(p_adapter);
	rx_initialize(p_adapter);

	//en PHY int.
	lan_enable_interrupt(p_adapter,INT_EN_PHY_INT_EN_);
	

	p_adapter->rx_buffer = NULL;
	p_adapter->rx_buffer = (UINT32 *)MALLOC(SMSC_PKT_BUFFER + 15);
	if(p_adapter->rx_buffer == NULL)
	{
		SMSC_WARNING("%s()=>Alloc Rx Buf Failed!\n", __FUNCTION__);
		goto DONE;
	}
	else
	{
		p_adapter->rx_buffer =(UINT32 *)(((UINT32)(p_adapter->rx_buffer) + SMSC_ALIGNMENT - 1) & ~(UINT32)(SMSC_ALIGNMENT-1));
		SMSC_WARNING("%s()=>RxBuffer(0x%08x)\n", __FUNCTION__, (UINT32)p_adapter->rx_buffer);
	}
	MEMSET(p_adapter->rx_buffer, 0x00, SMSC_PKT_BUFFER);

	//Dis Internal Timer Int.
	//Lan_EnableInterrupt(pAdapter,INT_EN_GPT_INT_EN_);

	//Set receiving callback function of TCPIP stack.   
	dev->callback = callback;

	// GPIO interrupt For SMSC9220 chip.
	HAL_GPIO_INT_SET(SMSC9220_GPIO, 1);			//GPIO Int En
	//HAL_GPIO_INT_SET(SMSC9220_GPIO, 0);			//GPIO Int Dis

	if(SMSC9220_POLAR == 0)
	{
		HAL_GPIO_INT_REDG_SET(SMSC9220_GPIO, 0);		//not Bring int at Rising edge
		HAL_GPIO_INT_FEDG_SET(SMSC9220_GPIO, 1);		//but Bring int at Failing edge
	}
	else
	{
		HAL_GPIO_INT_REDG_SET(SMSC9220_GPIO, 1);		//but Bring int at Rising edge
		HAL_GPIO_INT_FEDG_SET(SMSC9220_GPIO, 0);		//not Bring int at Failing edge
	}	
	
	HAL_GPIO_INT_CLEAR(SMSC9220_GPIO);			//Clear GPIO int status
	HAL_GPIO_BIT_DIR_SET(SMSC9220_GPIO,  0);		//Set GPIO direction to Input

	if(OSAL_E_OK!=osal_interrupt_register_lsr(p_adapter->irq, smsc9220_isr, (UINT32)p_adapter))
	{
		SMSC_WARNING("%s()=>GPIO ISR registered FAILED!\n", __FUNCTION__);
		result=ERR_FAILED;
		goto DONE;
	}

	p_adapter->register_isr = TRUE;

	lan_reg_get_print(INT_EN);
	
	//create a task to update link mode.
	task_param.task   =  (FP)smsc9220_task;
	task_param.stksz  =  0x1000; 
	task_param.quantum = 10;
	task_param.itskpri = OSAL_PRI_NORMAL;
	task_param.para1= (UINT32)p_adapter;
	task_param.para2 = 0;
	task_param.name[0] = 'N';
	task_param.name[1] = 'E';
	task_param.name[2] = '9';

	p_adapter->task_id = osal_task_create(&task_param);
	if(OSAL_INVALID_ID == p_adapter->task_id)
	{
		SMSC_WARNING("%s()=>Create phy_checklink FAILED!\n", __FUNCTION__);
		goto DONE;
	}

	result=0;
DONE:
	if(result!=0) 
	{
		smsc9220_close(dev);
		SMSC_WARNING("%s()=>smsc9220_close()\n", __FUNCTION__);
	}

#ifndef SMSC_UP_BUS
	//Exit Mutex
	if(smsc_mutex_id.unlock)
	{
		//HAL_GPIO_BIT_DIR_SET(SMSC_CS, HAL_GPIO_O_DIR);
		HAL_GPIO_BIT_SET(SMSC_CS, 1);	
		smsc_mutex_id.unlock();
	}
#endif //SMSC_UP_BUS

	return result;
}


RET_CODE 
smsc9220_send_packet(struct net_device *dev, void *buf, UINT16 len)
{
	RET_CODE result = SUCCESS;
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)(dev->priv);
	int re = 0;

	if(p_adapter->link_status == FALSE)
	{
		SMSC_WARNING("%s()=>Link's not ready.\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	lan_mutex_lock(p_adapter->trans_mutex);

	if(buf==NULL) {
		SMSC_WARNING("%()=>(buf==NULL).\n", __FUNCTION__);
		result=ERR_PARA;
		goto DONE;
	}
	if(dev==NULL) {
		SMSC_WARNING("%()=>(dev==NULL).\n", __FUNCTION__);
		result=ERR_PARA;
		goto DONE;
	}
	if(dev->priv==NULL) {
		SMSC_WARNING("%()=>(dev->priv==NULL).\n", __FUNCTION__);
		result=ERR_PARA;
		goto DONE;
	}
	
	re = tx_send_buf(p_adapter, buf, len);
	if(re != 0)
		result = ERR_FAILURE;
	
	lan_mutex_unlock(p_adapter->trans_mutex);

DONE:
	return result;
}


RET_CODE 
smsc9220_ioctl(struct net_device *dev, UINT32 cmd, UINT32 param)
{
	
	SMSC_TRACE("smsc9220_ioctl()=>Noting Done!\n");
	return RET_SUCCESS;
}


RET_CODE 
smsc9220_get_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)(dev->priv);
	UINT32 ret = RET_SUCCESS;
	UINT8 ret_info[32];
	UINT32 ret_len = 0;

	SMSC_TRACE("%s()=>Net device get information, type = %d.\n", __FUNCTION__, info_type);

	switch(info_type)
	{
		case NET_GET_CURRENT_MAC :
			MEMCPY(ret_info, p_adapter->dev_addr, ETH_ALEN);
			ret_len = ETH_ALEN;
			break;
		case NET_GET_PERMANENT_MAC :
			MEMCPY(ret_info, p_adapter->dev_addr, ETH_ALEN);
			ret_len = ETH_ALEN;
			break;
			
		case NET_GET_LINK_STATUS :
			if(p_adapter->link_status == TRUE)
				*((enum net_link_status *)ret_info) = NET_LINK_CONNECTED;
			else
				*((enum net_link_status *)ret_info) = NET_LINK_DISCONNECTED;
			ret_len = sizeof(enum net_link_status);
	    		break;
	
		case NET_GET_LINK_SPEED :
			if((p_adapter->link_speed == LINK_SPEED_10FD) || (p_adapter->link_speed == LINK_SPEED_10HD))
	    	  		*((enum net_link_speed *)ret_info) = NET_LINK_10MBPS;
			else
				*((enum net_link_speed *)ret_info) = NET_LINK_100MBPS;
			ret_len = sizeof(enum net_link_speed);	
	    		break;
			
		case NET_GET_LINK_MODE :
			if((p_adapter->link_speed == LINK_SPEED_10FD) || (p_adapter->link_speed == LINK_SPEED_100FD))
	    	  		*((enum net_link_mode *)ret_info) = NET_LINK_FULL;
			else
				*((enum net_link_mode *)ret_info) = NET_LINK_HALF;
			ret_len = sizeof(enum net_link_mode);	
	    		break;

		default :
			ret = RET_FAILURE;
	}
	if (ret == RET_SUCCESS)
	{
		MEMCPY(info_buf, ret_info, ret_len);
	}
	
	return ret;
}


RET_CODE 
smsc9220_set_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)(dev->priv);
	UINT32 ret = RET_SUCCESS;
	SMSC_TRACE("%s()=>Net device set information, type = %d.\n", __FUNCTION__, info_type);

	switch(info_type)
	{
		case NET_SET_MAC :
			if((*((UINT32 *)info_buf + 1) & 0xFFFF8000UL)!=0)
			{
				SMSC_TRACE("%s()=>Bad Mac Address.\n");
				ret = RET_FAILURE;
			}
			else
			{
				p_adapter->mac_addr_hi16 = *((UINT32 *)((UINT32)info_buf + 4));
				p_adapter->mac_addr_lo32= *((UINT32 *)info_buf);
				SMSC_TRACE("%s()=>Mac Addr(0x%x%x).\n", __FUNCTION__, p_adapter->mac_addr_hi16, p_adapter->mac_addr_lo32);
			}
			break;
		case NET_SET_LINK_SPEED :
		case NET_SET_LINK_MODE :
		case NET_ADD_MULTICAST_ADDR :
		case NET_DEL_MULTICAST_ADDR :
			SMSC_TRACE("%s()=>Command not supported yet.\n", __FUNCTION__);
			break;

		default :
			ret = RET_FAILURE;
	}

	return ret;
}


RET_CODE 
smsc9220_close(struct net_device *dev)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)(dev->priv);

	lan_disable_irq();

	SMSC_TRACE("smsc9220_detach()=>Detach net device driver.\n");
	if(p_adapter!=NULL)
	{
		if(p_adapter->mac_phy_lock != OSAL_INVALID_ID)
		{
			SMSC_WARNING("%s()=>Delete mutex(MacPhyLock).\n", __FUNCTION__);
			osal_mutex_delete(p_adapter->mac_phy_lock);
			p_adapter->mac_phy_lock = OSAL_INVALID_ID;
		}

		if(p_adapter->trans_mutex!= OSAL_INVALID_ID)
		{
			SMSC_WARNING("%s()=>Delete mutex(TransMutex).\n", __FUNCTION__);
			osal_mutex_delete(p_adapter->trans_mutex);
			p_adapter->trans_mutex = OSAL_INVALID_ID;
		}
		
		if(p_adapter->int_sem!= OSAL_INVALID_ID)
		{
			SMSC_WARNING("%s()=>Delete Sem(IntSem).\n", __FUNCTION__);
			osal_semaphore_delete(p_adapter->int_sem);
			p_adapter->int_sem = OSAL_INVALID_ID;
		}	
		
		if(p_adapter->rx_buffer!= NULL)
		{
			SMSC_WARNING("%s()=>Free Rx Buffer(RxBuffer).\n", __FUNCTION__);
			FREE(p_adapter->rx_buffer);
			p_adapter->rx_buffer = NULL;
		}

		if(p_adapter->register_isr== TRUE)
		{
			//unregister GPIO interrupt handler. 
			osal_interrupt_unregister_lsr(8, smsc9220_isr);
			p_adapter->register_isr = FALSE;
		}
			
		if(p_adapter->task_id != OSAL_INVALID_ID)
		{
			SMSC_WARNING("%s()=>Delete task(phy_checklink).\n", __FUNCTION__);
			osal_task_delete(p_adapter->task_id);
		}
			
	}
    return RET_SUCCESS;
}


RET_CODE 
smsc9220_detach(struct net_device *dev)
{
	PPRIVATE_DATA p_adapter = (PPRIVATE_DATA)(dev->priv);;

	SMSC_TRACE("smsc9220_detach: Detach net device driver.\n");
	if(p_adapter!=NULL)
	{
		MEMSET(p_adapter, 0x00, sizeof(PRIVATE_DATA));
		FREE(p_adapter);
		p_adapter = NULL;					
	}

	dev_free(dev);
	dev = NULL ;
	
	return RET_SUCCESS;
}


