/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 hardware data capture function  
*           
*
*    Description:  
*    History:		'refer to update_history.txt'
*******************************************************************************/

#include "nim_dvbs_c3505.h"

INT32 snr_initial_en;

INT32 nim_c3505_tsbb_cap_set(struct nim_device *dev, UINT8 s_case,UINT32 swpidf,UINT32 swpidd)
{
	UINT8 data = 0 ;
	// disable the insertion of dummy TS packet
	nim_reg_read(dev, 0xf0, &data, 1);
    data &= 0xfd;//[1]:0
    nim_reg_write(dev, 0xf0, &data, 1);

	data = 0x04;
    nim_reg_write(dev, RFC_TSO_CLK_SW_CFG+1, &data, 1); //TSO_SRC_DIV=2
    data = 0x13;
    nim_reg_write(dev, RFC_TSO_CLK_SW_CFG, &data, 1); //TSO_SRC=108MHz
	
	// config PIDF PIDD
	nim_reg_read(dev, 0x110, &data, 1);
    data |= 0x01;
    nim_reg_write(dev, 0x110, &data, 1);
	data = (UINT8)(swpidf & 0xff) ;
	nim_reg_write(dev, 0x100, &data, 1);
	data = (UINT8)((swpidf>>8)&0xff);
	nim_reg_write(dev, 0x101, &data, 1);
	data = (UINT8)((swpidf>>16)&0xff);
	nim_reg_write(dev, 0x102, &data, 1);

	nim_reg_read(dev, 0x110, &data, 1);
	data &= 0xfe;//[0]=0
	nim_reg_write(dev, 0x110, &data, 1);
	data = (UINT8)(swpidd & 0xff) ;
	nim_reg_write(dev, 0x100, &data, 1);
	data = (UINT8)((swpidd>>8)&0xff);
	nim_reg_write(dev, 0x101, &data, 1);
	data = (UINT8)((swpidd>>16)&0xff);
	nim_reg_write(dev, 0x102, &data, 1); 

	nim_reg_read(dev, 0x110, &data, 1);
    data &= 0xef;//[4]:0
    nim_reg_write(dev, 0x110, &data, 1);
	
	nim_reg_read(dev, 0x10e, &data, 1);
    data &= 0xaf;//[6]:0 [4]:0
    nim_reg_write(dev, 0x10e, &data, 1);
	
    nim_reg_read(dev, 0x106, &data, 1);
    data &= 0x7e;//[7]:0 [0]:0
    nim_reg_write(dev, 0x106, &data, 1);

	switch(s_case)
	{
		case NIM_CAP_TS1://TS_BB_L3
		
			NIM_PRINTF("TS_BB_L3 mode: swpidf is %x swpidd is %x\n",swpidf,swpidd);
		
			nim_reg_read(dev, 0x10e, &data, 1);
		    data |= 0x80;//[7]:1
    		nim_reg_write(dev, 0x10e, &data, 1);			
			break;
		case NIM_CAP_TS2://TS_BB_ALI
		
			NIM_PRINTF("TS_BB_ALI mode: swpidf is %x swpidd is %x\n",swpidf,swpidd);

			nim_reg_read(dev, 0x10e, &data, 1);
		    data &= 0x7f;//[7]:0
    		nim_reg_write(dev, 0x10e, &data, 1);
			break;
		default:

			break;							
	}
	
	return SUCCESS;
}

INT32 nim_c3505_dma_cap_set(struct nim_device *dev, UINT8 s_case)
{

	UINT8 data ;	

	switch(s_case)
	{
		case NIM_CAP_PAT0://DMA_BB_L3
			// configure fid
			nim_reg_read(dev,0x110, &data, 1);
			data |= 0x01;//[0]=1
			nim_reg_write(dev,0x110, &data, 1);
			data = 0xbb;
			nim_reg_write(dev,0x100, &data, 1);
			data = 0x13;
			nim_reg_write(dev,0x101, &data, 1);
			//configure BB_L3 mode
			nim_reg_read(dev,0x110, &data, 1);
			data &= 0xef;//[4]=0
			nim_reg_write(dev,0x110, &data, 1);
				
			nim_reg_read(dev,0x10e, &data, 1);
			data |= 0x90;//[7]=1 [4]=1
			nim_reg_write(dev,0x10e, &data, 1);
			
			nim_reg_read(dev,0x106, &data, 1);
			data &= 0x7e;//[7]=0 [0]=0
			nim_reg_write(dev,0x106, &data, 1);

			NIM_PRINTF("DMA_BB_L3 mode: fid is 0xbb 13\n");
			break;
		case NIM_CAP_PAT1:// DMA_BB_PURE_L3 
		
			nim_reg_read(dev,0x110, &data, 1);
			data |= 0x10;//[4]=1
			nim_reg_write(dev,0x110, &data, 1);

			nim_reg_read(dev,0x10e, &data, 1);
			data |= 0x90;//[7]=1 [4]=1
			nim_reg_write(dev,0x10e, &data, 1);
		
			nim_reg_read(dev,0x106, &data, 1);
			data &= 0x7e;//[7]=0 [0]=0
			nim_reg_write(dev,0x106, &data, 1);	

			NIM_PRINTF("DMA_BB_PURE_L3 mode: no need fid cfg\n");
			break;
		case NIM_CAP_PAT2:// DMA_BB_ALI
			// configure fid
			nim_reg_read(dev,0x110, &data, 1);
			data |= 0x01;//[0]=1
			nim_reg_write(dev,0x110, &data, 1);
			data = 0xda;
			nim_reg_write(dev,0x100, &data, 1);
			data = 0xbb;
			nim_reg_write(dev,0x101, &data, 1);
			data = 0xa1;
			nim_reg_write(dev,0x102, &data, 1);
			//configure BB_ALI mode
			nim_reg_read(dev,0x110, &data, 1);
			data &= 0xef;//[4]=0
			nim_reg_write(dev,0x110, &data, 1);
			
			nim_reg_read(dev,0x10e, &data, 1);
			data &= 0x7f;//[7]=0
			data |= 0x10;//[4]=1
			nim_reg_write(dev,0x10e, &data, 1);
		
			nim_reg_read(dev,0x106, &data, 1);
			data &= 0x7e;//[7]=0 [0]=0
			nim_reg_write(dev,0x106, &data, 1);	

			NIM_PRINTF("DMA_BB_ALI mode: fid is 0xda bb aa\n");
			break;
		default:
			break;					
	}

	return SUCCESS;
}

/*****************************************************************************
* 
* INT32 nim_c3505_cap_pkt(struct nim_device *dev,UINT32 dram_len,UINT8 cmd,UINT32 sw_fid)
*  capture RX-ADC data and calculate the spectrum energe 
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3505_cap_cfg(struct nim_device *dev,UINT32 dram_len)
{
	UINT8 *dram_base_t = NULL;
	UINT8 data =0 ,data_tmp = 0;
	UINT8 cmd = 0;
	UINT8 cap_src = 0;

	NIM_PRINTF("Enter function nim_C3505_cap_pkt!\n");

	dram_base_t = (UINT8 *) MALLOC(dram_len);

	if(NULL == dram_base_t)
	{
		NIM_PRINTF("C3505 adc2mem memory allocation error!\n"); 
		return ERR_FAILUE;
	}

	//REG 57
	//[7:4] 0:ADC_OUT 1:DCC_OUT 2:IQB_OUT 3:DC_OUT
	//        4:FLT_OUT 5:AGC2_OUT 6:TR_OUT 7:EQ_OUT
	//        8:PLDS_OUT 9:AGC3_OUT 10:DELAY_PLL_OUT 11:CR_OUT 
	//        12: PKT_DMA 13:LA_OUT
	//[3:1] 000--DMA_BB_L3 001--DMA_BB_PURE_L3 010--DMA_BB_ALI
	//         011--TS-BB-L3  100--TS-BB-ALI  111--cap BIST data
	//bit 0   1:enable capture 0:disable capture
	nim_reg_read(dev, R57_PKT_CAP, &data, 1);
	data_tmp =data >> 4;	
	if(12 == data_tmp)
	{
		data_tmp =(data & 0x0e)>>1 ;

		if(data_tmp < 3)//DMA Cap
		{
			if(0 == data_tmp)
				cmd = NIM_CAP_PAT0; //DMA_BB_L3				
			else if(1 == data_tmp)
				cmd = NIM_CAP_PAT1; //DMA_BB_PURE_L3
			else 
				cmd = NIM_CAP_PAT2; //DMA_BB_ALI
			nim_c3505_dma_cap_set(dev,cmd);				
			nim_c3505_adc2mem_entity(dev,dram_base_t, dram_len,12);
			// if you use WinGDB tool to capture data ,you couldn't set dram free.
			//			comm_free(dram_base_t);
			nim_c3505_normal_tso_set(dev);
		}
		else if(7 == data_tmp)
		{
			NIM_PRINTF("\t Configure test pattern mode and BIST mode.\n");
			// enable CAP2DRAM BIST
			nim_reg_read(dev,0x16c, &data, 1);	
			data |= 0x02;
			nim_reg_write(dev,0x16c, &data, 1);
			// Configure CAP2DRAM BIST MODE 16c[2]:1-- pn seq, 0--counter by 1
			if (1)
			{
				nim_reg_read(dev,0x16c, &data, 1);	
				data &= 0xfb;
				nim_reg_write(dev,0x16c, &data, 1);
			}
			else
			{
				nim_reg_read(dev,0x16c, &data, 1);
				data |= 0x04;
				nim_reg_write(dev,0x16c, &data, 1);					
			}
			nim_c3505_adc2mem_entity(dev,dram_base_t, dram_len,0);
			//clear bist enable
			nim_reg_read(dev,0x16c, &data, 1);	
			data &= 0xfd;
			nim_reg_write(dev,0x16c, &data, 1);

		}	
		else	
		{
			if(3 == data_tmp)
			{
				cmd = NIM_CAP_TS1 ;// TS_BB_L3			
			}
			else if(4 == data_tmp)
			{
				cmd = NIM_CAP_TS2 ;//TS_BB_ALI
			}
			nim_c3505_tsbb_cap_set(dev,cmd,0x00010535,0x00020535);
		}

	}
	else if(13 == data_tmp)//LA_OUT
	{		
		nim_c3505_la_cfg(dev);
		cap_src =(data & 0xf0)>>4 ;
		nim_c3505_adc2mem_entity(dev,dram_base_t, dram_len,cap_src);
	}	
	else
	{  
		cap_src =(data & 0xf0)>>4 ;
		nim_c3505_adc2mem_entity(dev,dram_base_t, dram_len,cap_src);			 
	}
	return SUCCESS;	
}

#ifdef ACM_RECORD
/*****************************************************************************
* 
* INT32 nim_c3505_rec_stop(struct nim_device *dev)
*  When record finish, Upper software need to call this function for recover some config
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3505_rec_stop(struct nim_device *dev)
{
    UINT8 data = 0;
	//libc_printf("====== enter nim_rec_stop ====== \n");

	//set 106 bit[0] to 1 
	nim_reg_read(dev, R106_BBF_CFG, &data, 1);
	data = data | 0x01;
	nim_reg_write(dev, R106_BBF_CFG, &data, 1);

	//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
	nim_reg_read(dev, R10b_ISI_SET, &data, 1);
	data |= 0x08;
	nim_reg_write(dev, R10b_ISI_SET, &data, 1);	//set 10b[3] = 1

	// set 106 bit[4] to 0
	nim_reg_read(dev, R106_BBF_CFG, &data, 1);
	data = data & 0xef;
	nim_reg_write(dev, R106_BBF_CFG, &data, 1);

	// set 10e bit[7] to 0
	nim_reg_read(dev, R10b_ISI_SET + 3, &data, 1);
	data = data & 0x7f;
	nim_reg_write(dev, R10b_ISI_SET + 3, &data, 1);	
	return SUCCESS;
}

/*****************************************************************************
* 
* INT32 nim_c3505_tso_bbfrm_cap_mod(struct nim_device *dev)
*  Set capture mode for record function
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3505_tso_bbfrm_cap_mod(struct nim_device *dev)
{
	UINT8 tso_bbfrm_cfg = 0;
	UINT8 data = 0;
    //libc_printf("====== enter nim_tso_bbfrm_cap_mod ====== \n");
    // 106 bit[4] = 1 isi filter bypass
    nim_reg_read(dev, R106_BBF_CFG, &data, 1); 
    data = data | 0x10;
	nim_reg_write(dev, R106_BBF_CFG, &data, 1);

	tso_bbfrm_cfg = 0xc7;
	nim_reg_write(dev, R57_PKT_CAP, &tso_bbfrm_cfg, 1);
	
	nim_reg_read(dev, R57_PKT_CAP, &data, 1);			
	if((data & 0x01) == 0x01)			
	{				
		nim_c3505_cap_cfg(dev,0x100000);		
		nim_reg_read(dev, R57_PKT_CAP, &data, 1);		
		data &=0xfe ;				
		nim_reg_write(dev, R57_PKT_CAP, &data, 1);		
	}
	return SUCCESS;
}


/*****************************************************************************
* 
* INT32 nim_c3505_ts_gs_auto_cap_mod(struct nim_device *dev)
*  When record start, config the capture mode
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_ts_gs_auto_cap_mod(struct nim_device *dev)
{
	UINT8 ts_gs_type = 0;
	UINT8 acm_ccm_type = 0;
	UINT8 sis_mis_type = 0;
    UINT8 work_mode = 0;
	UINT8 data = 0;
    INT32 ret_get_bb_header = ERR_CRC_HEADER;
    INT32 crc_correct_cnt = 0;
    INT32 time_out = 0;
	
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}	
	
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	
	// Check lock status
	while(1)
	{
		nim_c3505_get_lock(dev, &data);
		// lock
		if(1 == data)
			break;
		// unlock
		if(time_out >= 15)
		{
			ACM_DEBUG_PRINTF("In %s exit with TP unlock \n", __FUNCTION__);
			return ERR_FAILED; 
		}
		
		time_out++; 
		comm_sleep(5);
	}
	
	time_out = 0;

    nim_c3505_reg_get_work_mode(dev, &work_mode);
	if(1 == work_mode)
	{
		while(1)
		{	 
			ret_get_bb_header = nim_c3505_get_bb_header_info(dev);
			 
			if (SUCCESS == ret_get_bb_header)
			{
				crc_correct_cnt ++;
				 
				if (crc_correct_cnt == CRC_THRESHOLD)
				{
				    ts_gs_type = priv->bb_header_info->stream_mode;
				    acm_ccm_type = priv->bb_header_info->acm_ccm;
                    sis_mis_type = priv->bb_header_info->stream_type;
				
		    	    // to set TSO_BBFrame Output
		    	    // gs stream or acm or mis
		    	    if ((0 == ts_gs_type) || (1 == ts_gs_type) || (NIM_ACM == acm_ccm_type) || (NIM_STREAM_MULTIPLE == sis_mis_type))
					{
						// 106 bit[4] = 1 isi filter bypass
						nim_reg_read(dev, R106_BBF_CFG, &data, 1); 
						data = data | 0x10;
						nim_reg_write(dev, R106_BBF_CFG, &data, 1);
						
						//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
						// inorder to get all bbframe, we need to pass all the frames ,including the error bbframe,gloria
						nim_reg_read(dev, R10b_ISI_SET, &data, 1);
						data &= 0xf7;
						nim_reg_write(dev, R10b_ISI_SET, &data, 1); //set 10b[3] = 0

		    			return nim_c3505_tso_bbfrm_cap_mod(dev);
		    		}
		    		else
		    		{
		    			//default
		    			//return nim_c3505_tso_bbfrm_cap_mod(dev);
		    		}
						 
					break;
				}
			}
			else
			{											 
				//tso_bb
				// 106 bit[4] = 1 isi filter bypass
				nim_reg_read(dev, R106_BBF_CFG, &data, 1); 
				data = data | 0x10;
				nim_reg_write(dev, R106_BBF_CFG, &data, 1);
				
				//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
				// inorder to get all bbframe, we need to pass all the frames ,including the error bbframe,gloria
				nim_reg_read(dev, R10b_ISI_SET, &data, 1);
				data &= 0xf7;
				nim_reg_write(dev, R10b_ISI_SET, &data, 1); //set 10b[3] = 0

				return nim_c3505_tso_bbfrm_cap_mod(dev);
				break;
			}
		}          
	}

    return SUCCESS;
}
#endif



/*****************************************************************************
* INT32 nim_c3505_LA_cfg(struct nim_device *dev)
*    
*
* Arguments:
*  
*
* Return Value: UINT8
*****************************************************************************/
INT32 nim_c3505_la_cfg(struct nim_device *dev)
{
	UINT8 data = 0;

	//config test_mux, cr7b[6:0], select trigger source data
	data = 0x40 ; //TSO_TEST_OUT
	nim_reg_write(dev,R7B_TEST_MUX, &data, 1);
	
	//config LA_TRG_MASK:1--trigger signals 0--not trigger signals
	data = 0xff ;
	nim_reg_write(dev,R197_LA_TRG_MASK, &data, 1);
	data = 0x04 ;
	nim_reg_write(dev,R197_LA_TRG_MASK+1, &data, 1);
	
//config LA_TRG_TAR
//when trigger type la_trg_type_equ,la_trg_type_neq,la_trg_type_nor1, need config it
	data = 0x47 ;
	nim_reg_write(dev,R199_LA_TRG_TAR, &data, 1);
	data = 0x04 ;
	nim_reg_write(dev,R199_LA_TRG_TAR+1, &data, 1);
	
//config trigger meet type:LA_TRG_PARA[2:0]  = cr19b[2:0]
//config rec_mode: record data format (cr19b[6:4])
//0--LA_IN, 1:(la_in_trg_d1 & (~la_in_trg_mask)), triger data neg, 
//2--((~la_in_trg_d1) & la_in_trg_mask); triger data pos, 3--la_in_trg_mask, ...
	nim_reg_read(dev,R19B_LA_TRG_PARA, &data, 1);
	data &= 0x88;
  //data |= 0x32;//2:la_trg_type_equ, 3:record data is la_in_trg_mask=LA_IN&R197_LA_TRG_MASK
    data |= 0x02;//2:la_trg_type_equ, 0:LA_IN
	nim_reg_write(dev,R19B_LA_TRG_PARA, &data, 1);
	
//config record data valid, only when trigger vld data can send to cap2dram.
	nim_reg_read(dev,R19B_LA_TRG_PARA+1, &data, 1);
	data &= 0xf8 ;
  //data |= 0x03 ;//3--la_trg_type_equ   
	nim_reg_write(dev,R19B_LA_TRG_PARA+1, &data, 1);
	
//config Trigger number, 19d[0]=0, unit is 2K byte; 19d[0]=1, unit is 2M byte
	data = 0x40 ; //record 128K data
	nim_reg_write(dev,R19D_LA_TRG_TIM, &data, 1);

//config LA_CAP_EN_MODE (cr19c[3])
//0--LA_CAP_EN begin after la_start, 1--LA_CAP_EN begin when la_trg_met 
	nim_reg_read(dev,R19C_LA_TRG_CMD, &data, 1);
	data &= 0xf7 ;
	data |= 0x08 ;
	nim_reg_write(dev,R19C_LA_TRG_CMD, &data, 1);

//config LA_CAP_EN_BYPASS, cap2dram when LA_CAP_EN 	
	nim_reg_read(dev,0x16b, &data, 1);
	data &= 0xf7 ;
	nim_reg_write(dev,0x16b, &data, 1);	

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3505_normal_tso_set(struct nim_device *dev)
*    after capture TS BB data, reture normal tso mode.
*
* Arguments:
*  
*
* Return Value: UINT8
*****************************************************************************/

INT32 nim_c3505_normal_tso_set(struct nim_device *dev)
{
	UINT8 data = 0 ;
		
	nim_reg_read(dev,0x10e, &data, 1);
	data &= 0xef;//[4]=0
	nim_reg_write(dev,0x10e, &data, 1);
	
	nim_reg_read(dev,0x106, &data, 1);
	data &= 0x7f;//[7]=0
	data |= 0x01;//[0]=1
	nim_reg_write(dev,0x106, &data, 1);	

	return SUCCESS;
	
}



/*****************************************************************************
*  INT32 nim_C3505_adc2mem_entity(struct nim_device *dev, UINT32 *cap_buffer, UINT32 dram_len,UINT8 cmd)
* capture RX_ADC data to DRAM
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_adc2mem_entity(struct nim_device *dev,  UINT8 *cap_buffer, UINT32 dram_len,UINT8 cap_src)
{	
	UINT8 data, data_1;
	UINT8 user_force_stop_adc_dma=0;
    UINT32 dram_base;
	UINT32 cap_len ;
	struct nim_c3505_private *priv = NULL;

	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
        
	priv = (struct nim_c3505_private *) dev->priv;
	
    dram_base = (UINT32)cap_buffer;
	//Reset ADCDMA.	  
   	nim_reg_read(dev,0x16c, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev,0x16c, &data, 1);

 	//Configure DRAM length, can support 1G byte, but should consider with base address 
 	//and make sure that it is not beyond availbe sdram range.
	if (dram_len > 0x10000000)
	    dram_len = 0x10000000;
	
	cap_len= (dram_len>>5);	// unit is 32byte	
	nim_reg_read(dev,0x16d, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev,0x16d, &data, 1);
	NIM_PRINTF("\t Configure DRAM length: 0x%08x.\n", cap_len<<5);
	data = (UINT8)(cap_len&0xff);
	nim_reg_write(dev,0x160, &data, 1);
	data = (UINT8)((cap_len>>8)&0xff);
	nim_reg_write(dev,0x161, &data, 1);
	data = (UINT8)((cap_len>>16)&0xff);
	nim_reg_write(dev,0x162, &data, 1);
	data = (UINT8)((cap_len>>24)&0xff);
	nim_reg_write(dev,0x163, &data, 1);

//Configure DRAM base address, unit is byte
	nim_reg_read(dev,0x16d, &data, 1);
	data &= 0x7f;	
	nim_reg_write(dev,0x16d, &data, 1);	
//	dram_base &=  0x7fffffe0;
	NIM_PRINTF("Enter function %s with DRAM base address: 0x%08x\n",__FUNCTION__,dram_base);
    //dram_base &=  0x7fffffff;
    dram_base &=  0xfffffff;    // Paladin for address transform
//	dram_base = (dram_base&0xffffffff)>>5;
	data = (UINT8)(dram_base&0xff);
	nim_reg_write(dev,0x160, &data, 1);
	data = (UINT8)((dram_base>>8)&0xff);
	nim_reg_write(dev,0x161, &data, 1);
	data = (UINT8)((dram_base>>16)&0xff);
	nim_reg_write(dev,0x162, &data, 1);
	data = (UINT8)((dram_base>>24)&0xff);
	nim_reg_write(dev,0x163, &data, 1);

	//Configure capture data target size:64kByte
	//Unit 32K Byte, can capture max 2GB data.
	cap_len = dram_len >> 15 ;
	data = (UINT8)(cap_len&0xff) ;
	nim_reg_write(dev,0x16e, &data, 1);
	data = (UINT8)((cap_len>>8)&0xff) ; 
	nim_reg_write(dev,0x16f, &data, 1);	
	
	// configure capture source data
	//Configure capture points, reg16b[7:4]
	//0:ADC_OUT 1:DCC_OUT 2:IQB_OUT 3:DC_OUT
	//4:FLT_OUT 5:AGC2_OUT 6:TR_OUT 7:EQ_OUT
	//8:PLDS_OUT 9:AGC3_OUT 10:DELAY_PLL_OUT 11:CR_OUT 
	//12: PKT_DMA 13:LA_OUT

	nim_reg_read(dev,0x16b, &data, 1);
	data &= 0x0f;
	data |= ((cap_src & 0x0f)<<4);
	NIM_PRINTF("capture data source value reg: 0x%x\n",data);
	nim_reg_write(dev,0x16b, &data, 1);
	
	//Reset ADCDMA.	  
   	nim_reg_read(dev,0x16c, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev,0x16c, &data, 1);
	
	//Start ADC DMA
	NIM_PRINTF("\t Start ADC DMA.\n");
	nim_reg_read(dev,0x16c, &data, 1);
	data |= 0x01;
	nim_reg_write(dev,0x16c, &data, 1);

	if(13 == cap_src)
	{
		nim_reg_read(dev,R19C_LA_TRG_CMD, &data, 1);
		data |= 0x10 ;
		nim_reg_write(dev,R19C_LA_TRG_CMD, &data, 1);
		data &= 0xef ;
		nim_reg_write(dev,R19C_LA_TRG_CMD, &data, 1);
	}
	//Wait ADC DMA finish.
	NIM_PRINTF("\t Wait ADC DMA finish.\n");
	while(1)
	{
		if (user_force_stop_adc_dma)
		{
			user_force_stop_adc_dma = 0;
			nim_reg_read(dev,0x16c, &data, 1);
			data |= 0x80;	
			nim_reg_write(dev,0x16c, &data, 1);
			NIM_PRINTF("ADC_DMA force stopped by user.\n");
			break;
		}
		nim_reg_read(dev,0x16d, &data, 1);
		nim_reg_read(dev,0x04, &data_1, 1);
        
		NIM_PRINTF("\t Waiting: ... CR 0x16d: 0x%02x. status 0x%x\n", data, data_1);

		if (data&0x04)
		{
			NIM_PRINTF("WR_FIFO overflowed.\n");
			break;
		}

		if (data&0x01)
		{
			NIM_PRINTF("ADC_DMA finished.\n");
			break;
		}

		if (GET_SPECTRUM_ONLY != priv->autoscan_control) //autoscan maybe need delay some time
		{
			comm_sleep(50);
		}
	}

	NIM_PRINTF("%s Exit.\n",__FUNCTION__);
  	nim_reg_read(dev,0x16c, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev,0x16c, &data, 1);
	
	return SUCCESS;
}



/*****************************************************************************
* void nim_C3505_cap_calculate_energy(struct nim_device *dev)
*  capture RX-ADC data and calculate the spectrum energe
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
void nim_c3505_cap_calculate_energy(struct nim_device *dev)
{
	INT32 i = 0, j = 0, k = 0, n = 0;//,printf_1 = 1;
	INT32 energy_real = 0, energy_imag = 0;//unsigned energy
	INT32 energy = 0;//unsigned energy
	INT32 energy_tmp = 0;
	INT32 fft_I, fft_Q;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));

	for (k = 0; k < 20; k++)
	{
		NIM_PRINTF("k = %d\n",k);

		if (priv->ul_status.c3505_autoscan_stop_flag)
		{
			return;
		}
		//get FFT result
		NIM_PRINTF("fft_start\n");
		nim_c3505_cap_fft_result_read(dev);
		NIM_PRINTF("fft_end\n");
		//accumulation
		for (i = 0; i < 1024; i++)
		{
			fft_I = priv->ul_status.fft_i_1024[i];
			fft_Q = priv->ul_status.fft_q_1024[i];

			energy_real = fft_I * fft_I;
			energy_imag = fft_Q * fft_Q;
			energy = energy_real + energy_imag;
			//because 20 times of accumulation needed, result will extend 5 bits
			//nevertherless maximum energy of possible signal(eg. 2Msps) is 5 times 				//lessthan theoretic max energy, ie. 2 bits less
			//if x is maximum FFT level, x^2/20 is maximum signal(2Msps) energy level(psd)
			//if we clip the MSB in FFT module(when FFT layer is 5), the maximum energy is  		//x^2/4
			//so we can get above conclusion
			//so we only need to reduce 3 bits LSB of accumulation result
			energy >>= 3;
			j = (i + 511) & 1023;//fold FFT sequence to natural sequence
			//energy_tmp = fft_energy_1024_tmp[j] + energy;
			//energy_tmp=fft_energy_1024[j]+energy*20;
			energy_tmp =  energy;
			//fft_energy_1024[j] = fft_I;

			if ((energy_tmp >> 20) & 1)
			{
			fft_energy_1024_tmp[j] = 1048575;//maximum unsigned value of 20 bits
			}
			else
			{
			fft_energy_1024_tmp[j] = energy_tmp;
			}
		}
		for(n = 0;n < 1024;n++)
		{
			fft_energy_1024[n] += fft_energy_1024_tmp[n];
		}
	}
}

/*
 const UINT32 agc_table[256] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,   5, \
  6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21, \
 22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37, \
 38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  52,  55,  57, \
 60,  62,  65,  67,  70,  72,  75,  77,  78,  79,  80,  82,  83,  84,  85,  87, \
 88,  89,  90,  92,  93,  94,  95,  97,  98,  99, 101, 102, 103, 104, 105, 106, \
107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, \
123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, \
139, 140, 141, 142, 144, 145, 146, 147, 149, 150, 151, 152, 154, 155, 156, 157, \
159, 160, 161, 162, 164, 165, 166, 167, 169, 170, 171, 173, 175, 176, 178, 180, \
182, 184, 187, 189, 192, 194, 197, 199, 202, 204, 207, 209, 212, 215, 218, 222, \
225, 228, 232, 235, 238, 242, 245, 248, 252, 255, 258, 262, 265, 268, 272, 275, \
278, 282, 285, 288, 292, 295, 298, 302, 305, 308, 312, 317, 322, 327, 332, 337, \
342, 347, 352, 357, 362, 367, 372, 377, 382, 387, 392, 397, 402, 407, 412, 417, \
422, 427, 432, 437, 442, 447, 452, 457, 462, 467, 472, 477, 482, 487, 492, 493, \
494, 495, 496, 497, 498, 499, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, \
};
*/


/*****************************************************************************
* void nim_C3505_adc2mem_calculate_energy(struct nim_device *dev,UINT8 *cap_buffer,UINT32 dram_len)
* capture RX-ADC data to DRAM and calaulate the spectrum energy
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_adc2mem_calculate_energy(struct nim_device *dev,UINT8 *cap_buffer,UINT32 dram_len)
{
	INT32 i, j, k, m,n;//,printf_1 = 1;
	INT32 energy_real, energy_imag;//unsigned energy
	INT32 energy;//unsigned energy
	INT32 energy_tmp;
	INT32 fft_I, fft_Q;
	UINT8 *data_ptr;
	struct nim_c3505_private *priv = NULL;
	UINT8 fft_cnt = 10;
	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
        
	priv = (struct nim_c3505_private *) dev->priv;
	
	comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
	
	if (GET_SPECTRUM_ONLY == priv->autoscan_control)//get spectrum or get frequency/sysbolrate use 10
	{
		fft_cnt = 10;
	}
	else 											//autoscan use 20
	{
		fft_cnt = 20;
	}
	
	/*This function used to write back data cache to DRAM by given the address and byte length.*/
	osal_cache_flush(cap_buffer,dram_len);
	/*capture 64Kbyte ADC2DRAM data*/
    nim_c3505_adc2mem_entity( dev,cap_buffer,dram_len,0);
	/*This function used to update data to cache from DRAM, by given the address and byte length.*/
	osal_cache_invalidate(cap_buffer,dram_len);

    for(k = 0; k < fft_cnt; k++)
    { 
        data_ptr = cap_buffer + k*2048;
		for (i = 0; i < 1024; i++)
		{
			fft_I = data_ptr[2 * i];
			fft_Q = data_ptr[2 * i + 1];
            
			if (fft_I & 0x80)
				fft_I |= 0xffffff00;
			if (fft_Q & 0x80)
				fft_Q |= 0xffffff00;

			m = 0;
			for (j = 0; j < 10; j++)
			{
				  	m <<= 1;m += ((i >> j) & 0x1);
			}//address change for 1024 FFT
			fft_I = fft_I << (FFT_BITWIDTH - 8);
			fft_Q = fft_Q << (FFT_BITWIDTH - 8);         	
			priv->ul_status.fft_i_1024[m] = fft_I;
			priv->ul_status.fft_q_1024[m] = fft_Q;
		}
	    // calculate FFT
		R2FFT(priv->ul_status.fft_i_1024, priv->ul_status.fft_q_1024);
 
		//accumulation
		for (i = 0; i < 1024; i++)
		{
			fft_I = priv->ul_status.fft_i_1024[i];
			fft_Q = priv->ul_status.fft_q_1024[i];

			energy_real = fft_I * fft_I;
			energy_imag = fft_Q * fft_Q;
			energy = energy_real + energy_imag;

			//because 20 times of accumulation needed, result will extend 5 bits
			//nevertherless maximum energy of possible signal(eg. 2Msps) is 5 times 				//lessthan theoretic max energy, ie. 2 bits less
			//if x is maximum FFT level, x^2/20 is maximum signal(2Msps) energy level(psd)
			//if we clip the MSB in FFT module(when FFT layer is 5), the maximum energy is  		//x^2/4
			//so we can get above conclusion
			//so we only need to reduce 3 bits LSB of accumulation result
			energy >>= 3;
			j = (i + 511) & 1023;//fold FFT sequence to natural sequence
			//energy_tmp = fft_energy_1024_tmp[j] + energy;
			//energy_tmp=fft_energy_1024[j]+energy*20;
			energy_tmp =  energy;
			//  		fft_energy_1024[j] = fft_I;

			if ((energy_tmp >> 20) & 1)
			{
				fft_energy_1024_tmp[j] = 1048575;//maximum unsigned value of 20 bits
			}
			else
			{
				fft_energy_1024_tmp[j] = energy_tmp;
			}
		}
		for(n=0;n<1024;n++)
		{
			fft_energy_1024[n] += fft_energy_1024_tmp[n];
		}
    }

       return SUCCESS;
 }


 /*****************************************************************************
* void nim_c3505_cap_fft_result_read(struct nim_device *dev)
* Get RX-ADC data and calculate the spectrum energe
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
void nim_c3505_cap_fft_result_read(struct nim_device *dev)
{
	UINT8 data = 0;
	INT32 m = 0;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
	nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[1]);
	nim_c3505_set_adc(dev);
	nim_c3505_interrupt_mask_clean(dev);

	// tso dummy off
	nim_c3505_tso_dummy_off(dev);
	//Enable AGC1 auto freeze function and let AGC can not lock, try to modify tuner's gain
	data = 0xd5; 
	nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);
	nim_c3505_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT,NIM_FRZ_AGC1_OPEN);

	//CR118:Capture data length setting
	data = 0x10  ;
	nim_reg_write(dev, 0x118, &data, 1);
	
	//CR71:CAP_SRC Setting
	nim_reg_read(dev, R71_CAP_RDY, &data, 1);
	data = data|0x10 ;
	nim_reg_write(dev, R71_CAP_RDY, &data, 1);

	//CRBE: capture out mode and capture block number setting
	data = 0x01;
	nim_reg_write(dev, RBD_CAP_PRM+1, &data, 1);
		
	//set capture sample mode
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);

	comm_delay(1000);

	data = 0x50; ////0x2B; let AGC  lock, try to modify tuner's gain
	nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

	for (m = 0; m < 1000; m++)
	{
		nim_c3505_cap_iq_enerage(dev);
		//do software 1024 points R2 FFT
		R2FFT(priv->ul_status.fft_i_1024, priv->ul_status.fft_q_1024);
		NIM_PRINTF(" ADC DATA  transfer finish\n");
		break;		
	}
}

/*****************************************************************************
*  INT32 nim_C3505_cap_IQ_enerage(struct nim_device *dev)
* FFT calculation preprocessing
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cap_iq_enerage(struct nim_device *dev)
{
	UINT8 data = 0;
	INT32 i = 0, m = 0, j = 0, k = 0;
	UINT8 fft_data[3];
	INT32 fft_I, fft_Q;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	for (i = 0; i < 1024; i++)
	{
		for (m = 0; m < 1000; m++)
		{
			comm_delay(1000);
			nim_reg_read(dev, R71_CAP_RDY, &data, 1);
			if(data & 0x01)
			{					 
				break ;			 
			}	 
		}
		nim_reg_read(dev, R72_CAP_REG, fft_data, 2);
		//if data sending in hardware is fatster than data taking in software, there is no need to reset
		//reset read enable
		data &= 0xfe ;
		nim_reg_write(dev, R71_CAP_RDY, &data, 1);

		fft_I = fft_data[0];
		fft_Q = fft_data[1];

		if (fft_I & 0x80)
		fft_I |= 0xffffff00;
		if (fft_Q & 0x80)
		fft_Q |= 0xffffff00;

		k = 0;
		for (j = 0; j < 10; j++)
		{
			k <<= 1;k += ((i >> j) & 0x1);
		}//address change for 1024 FFT

		fft_I = fft_I << (FFT_BITWIDTH - 8);
		fft_Q = fft_Q << (FFT_BITWIDTH - 8);

		priv->ul_status.fft_i_1024[k] = fft_I;
		priv->ul_status.fft_q_1024[k] = fft_Q;
	}

	//if(1==AUTOSCAN_DEBUG_FLAG&&(0x01))
	if(priv->autoscan_debug_flag & 0x01)
	{
		for(i = 0; i< 1024; i++)
		{
			AUTOSCAN_PRINTF("ADCdata%d:[%d,%d]\n",i,priv->ul_status.fft_i_1024[i] ,priv->ul_status.fft_q_1024[i]);
		}
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_cap(struct nim_device *dev, UINT32 startFreq, UINT32 *cap_buffer, UINT32 sym)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter2: UINT32 sym
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cap(struct nim_device *dev, UINT32 start_freq, INT32 *cap_buffer, UINT32 sym)
{
	NIM_PRINTF("Enter nim_C3505_cap : Tuner_IF=%d\n",start_freq);

	nim_c3505_cap_start(dev, start_freq, sym, cap_buffer);

	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_WidebandScan_open(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq,UINT32 step_freq)
* 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 start_freq
*	Parameter3: UINT32 end_freq
*	Parameter4: UINT32 step_freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_wide_band_scan_open(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq,UINT32 step_freq)
{
	UINT32 full_size = 0;

	NIM_PRINTF("Enter nim_C3505_WidebandScan_open\n");

	last_tuner_if = 0;
	chlspec_num = 0;
	called_num = 0;

	full_size = (end_freq-start_freq)/step_freq;
	full_size = (full_size+2)*512;
	channel_spectrum = (INT32 *) comm_malloc(full_size * 4);
	if(channel_spectrum == NULL)
	{
		NIM_PRINTF("\n channel_spectrum--> no enough memory!\n");
		return ERR_NO_MEM;
	}

	if (nim_c3505_get_bypass_buffer(dev))
	{
		NIM_PRINTF("\n ADCdata--> no enough memory!\n");
		comm_free(channel_spectrum);
		return ERR_NO_MEM;
	}
	return SUCCESS;

}
/*****************************************************************************
*  INT32 nim_C3505_autoscan_WidebandScan_close()
*
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_wide_band_scan_close()
{
	comm_free(channel_spectrum);
	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_autoscan_WidebandScan(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq)
* 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 start_freq
*	Parameter3: UINT32 end_freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_wide_band_scan(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq)
{
	UINT32 FFT_freq = 0;
	UINT32 i = 0,j = 0;
	UINT32 ADC_sample_freq = 0;
	UINT32  step_freq = 0;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	NIM_PRINTF("Enter nim_C3505_WidebandScan\n");

	ADC_sample_freq = ADC_SAMPLE_FREQ[1];
	step_freq = ADC_sample_freq / 2;  

	if(SUCCESS == nim_c3505_wide_band_scan_open(dev,start_freq,end_freq,step_freq))
	{
		for (FFT_freq = start_freq; FFT_freq < end_freq; FFT_freq += step_freq)
		{
			nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
			comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
			for(j = 0;j < 100; j++)
			{
				nim_c3505_cap_start(dev, FFT_freq, 0, fft_energy_1024);
				for(i = 0;i < 1024;i++)
				{
					if(fft_energy_1024[i] > 0)
					break;
				}
				if(i < 1024)
					break;
			}

			if(100 == j)
			{
				comm_free(priv->ul_status.adc_data_malloc_addr);
				NIM_PRINTF("nim_C3505_WidebandScan works wrong !\n");
				NIM_PRINTF("ERR_FAILURE, Leave nim_C3505_WidebandScan!\n");
				return ERR_FAILURE;
			}
			//nim_s3501_FFT_WidebandScan(dev,FFT_freq,ADC_sample_freq);

			// if(SUCCESS == call_tuner_command(dev, NIM_TUNER_C3031B_ID, &channel_freq_err))//modify by dennis on 20140909 for Ali c3031b tuner 
			if(priv->tuner_type == IS_M3031)
			{
				//nim_s3501_fft_wideband_scan_m3031(dev,FFT_freq,ADC_sample_freq); //ziv.gu
				nim_s3501_fft_wideband_scan(dev,FFT_freq,ADC_sample_freq);
			}
			else
			{
				nim_s3501_fft_wideband_scan(dev,FFT_freq,ADC_sample_freq);
			}
		}
		comm_free(priv->ul_status.adc_data_malloc_addr);
		NIM_PRINTF("SUCCESS, Leave nim_C3505_WidebandScan!\n");
		return SUCCESS;
	}
	else
	{
		NIM_PRINTF("ERR_NO_MEM, Leave nim_C3505_WidebandScan!\n");
		return ERR_NO_MEM;
	}
}
/*****************************************************************************
*  INT32 nim_C3505_cap_fre(struct nim_device *dev)
*  capture RX-ADC data and calculate the spectrum energe 
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cap_fre(struct nim_device *dev)
{
	//reg_cr113[0] is used to enable capture frequency function
	UINT8  *dram_base_t = NULL;
	UINT8 	data = 0;
	int n = 0;

	nim_reg_read(dev, R113_NEW_CR_ADPT_CTRL, &data, 1);
	if((data & 0x01) == 0x01)
	{
		//enable capture
		dram_base_t = (UINT8 *) MALLOC(0x10000 );
		if(NULL == dram_base_t)
		{
			ERR_PRINTF("C3505 adc2mem memory allocation error!\n"); 
			return ERR_FAILUE;
		}
		nim_c3505_adc2mem_calculate_energy(dev, dram_base_t, 0x10000);
		comm_free(dram_base_t);
		NIM_PRINTF("adc2dram begin\n");

		for(n = 0;n < 1024;n++)
		{
			NIM_PRINTF("##%d##\n",fft_energy_1024[n]);
		}
		NIM_PRINTF("adc2dram end\n");
		data = data & 0xfe;			// disable
		nim_reg_write(dev,R113_NEW_CR_ADPT_CTRL,&data,1);
	}

	return SUCCESS;
}

