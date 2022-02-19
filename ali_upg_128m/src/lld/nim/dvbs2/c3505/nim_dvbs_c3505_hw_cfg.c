/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 hardware operate function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#include "nim_dvbs_c3505.h"

static UINT8 va_22k = 0x00;

#ifdef ASKEY_PATCH1
	extern UINT8 g_askey_patch_1_enable;
#endif


/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i = 0;
	
    if((NULL == dev) || (NULL == pData))
    {
        ERR_PRINTF("Error input param in %s line = %d, exit! \n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }

	pData[0] = bMemAdr;
	if (bLen > 8)
	{
		nim_c3505_set_err(dev);
        ERR_PRINTF("Error input param in %s line = %d, exit! \n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	for (i = 0; i < bLen; i++)
	{
		pData[i] = NIM_GET_BYTE(NIM_C3505_BASE_ADDR + bMemAdr + i);		
	}
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: register write function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
 INT32 nim_c3505_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i;

    // Pointer check
    if((NULL == dev) || (NULL == pData))
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }


	if (bLen > 8)
	{
		nim_c3505_set_err(dev);
        ERR_PRINTF("Exit with ERR_PARA in %s, bLen = %d\n",__FUNCTION__, bLen);
		return ERR_PARA;
	}
	
	for (i = 0; i < bLen; i++)
	{
		NIM_SET_BYTE(NIM_C3505_BASE_ADDR + bMemAdr + i, pData[i]);
	}
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_err(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_err(struct nim_device *dev)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	priv->ul_status.m_err_cnts++;

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_get_err(struct nim_device *dev)
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_get_err(struct nim_device *dev)
{
	struct nim_c3505_private *priv = NULL;
	// Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	
	priv = (struct nim_c3505_private *) dev->priv;

	if (priv->ul_status.m_err_cnts > 0x00)
		return ERR_FAILED;
	else
		return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_clear_err(struct nim_device *dev)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_clear_err(struct nim_device *dev)
{
	struct nim_c3505_private *priv = NULL;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;
	priv->ul_status.m_err_cnts = 0x00;

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_sym_config(struct nim_device *dev, UINT32 sym)
* 
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 sym
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_sym_config(struct nim_device *dev, UINT32 sym)
{
	struct nim_c3505_private *priv = NULL;
    
    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;
	if (sym > 40000)
		priv->ul_status.c_rs = 8;
	else if (sym > 30000)
		priv->ul_status.c_rs = 4;
	else if (sym > 20000)
		priv->ul_status.c_rs = 2;
	else if (sym > 10000)
		priv->ul_status.c_rs = 1;
	else
		priv->ul_status.c_rs = 0;
	return SUCCESS;
}

 /*****************************************************************************
*  INT32 nim_C3505_interrupt_mask_clean(struct nim_device *dev)
* Interrupt mask clean 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_interrupt_mask_clean(struct nim_device *dev)
{
	UINT8 data = 0x00;
	// Pointer check
	if(NULL == dev)
	{
		ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
		return ERR_NO_DEV;
	}


	nim_reg_write(dev, R02_IERR, &data, 1);
	data = 0xff;
	nim_reg_write(dev, R03_IMASK, &data, 1);
	return SUCCESS;
}

/*****************************************************************************
* void nim_C3505_interrupt_clear(struct nim_device *dev)
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	 
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_interrupt_clear(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT8 rdata = 0;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	//CR02
	nim_reg_write(dev, R02_IERR, &data, 1);
	nim_reg_write(dev, R04_STATUS, &data, 1);

	nim_reg_read(dev, R00_CTRL, &rdata, 1);
	data = (rdata | 0x10);
	nim_c3505_set_demod_ctrl(dev, data);
	//NIM_PRINTF("    enter nim_C3505_interrupt_clear\n");

    return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_demod_ctrl(struct nim_device *dev, UINT8 c_Value)
* Demodulator Control Register configuration
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_demod_ctrl(struct nim_device *dev, UINT8 c_value)
{
	UINT8 data = c_value;
    
    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	nim_reg_write(dev, R00_CTRL, &data, 1);

	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_set_hw_timeout(struct nim_device *dev, UINT8 time_thr)
*  Demodulator hardware timeout threshold setting
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 time_thr 
*	timeout threshold value, unit is 1/90 second
*	
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_set_hw_timeout(struct nim_device *dev, UINT16 time_thr)
{
	struct nim_c3505_private *priv = NULL;
	UINT8 data_write = 0;
    UINT8 data_read = 0;
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	priv = (struct nim_c3505_private *) dev->priv;

	// HW timeout set for soft search
	if (time_thr != priv->ul_status.m_hw_timeout_thr)
	{
	    data_write = (UINT8)(time_thr & 0xff);
        nim_reg_write(dev, R05_TIMEOUT_TRH, &data_write, 1);
    
        nim_reg_read(dev, R05_TIMEOUT_TRH + 1, &data_read, 1);
        data_write = (data_read & 0xfc) | ((UINT8)(time_thr >> 8) & 0x03);
        nim_reg_write(dev, R05_TIMEOUT_TRH + 1, &data_write, 1);

		priv->ul_status.m_hw_timeout_thr = time_thr;
	}

	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_set_ext_LNB(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner)
{
	/****For external lnb controller config****/
	struct nim_c3505_private *priv_mem = NULL;
    
    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_MEM;
    }
    priv_mem = (struct nim_c3505_private *) dev->priv;
	priv_mem->ext_lnb_control = NULL;

	if (ptr_qpsk_tuner->ext_lnb_config.ext_lnb_control != NULL)
	{
		UINT32 check_sum = 0;
		check_sum = (UINT32) (ptr_qpsk_tuner->ext_lnb_config.ext_lnb_control);
		check_sum += ptr_qpsk_tuner->ext_lnb_config.i2c_base_addr;
		check_sum += ptr_qpsk_tuner->ext_lnb_config.i2c_type_id;
		if (check_sum == ptr_qpsk_tuner->ext_lnb_config.param_check_sum)
		{
			priv_mem->ext_lnb_control = ptr_qpsk_tuner->ext_lnb_config.ext_lnb_control;
			priv_mem->ext_lnb_control(0, LNB_CMD_ALLOC_ID, (UINT32) (&priv_mem->ext_lnb_id));
			priv_mem->ext_lnb_control(priv_mem->ext_lnb_id, LNB_CMD_INIT_CHIP, (UINT32) (&ptr_qpsk_tuner->ext_lnb_config));
		}
	}

	return SUCCESS;
}

/*****************************************************************************
* void nim_C3505_after_reset_set_param(struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_after_reset_set_param(struct nim_device *dev)
{
	UINT8 data = 0;//,data1;
    UINT32 data_tmp = 0;
	int i = 0;
	struct nim_c3505_private *priv = NULL;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }  

	priv = dev->priv;
	if(NULL == priv)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }  
	
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

	nim_c3505_interrupt_mask_clean(dev);
	//AGC normal work 
    data = 0x50; 
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

	// set diseqc clock.
    nim_c3505_get_dsp_clk(dev, &data_tmp);
    // diseqc clock ratio setting
	//data=((data_tmp+500)/1000);
	data=(data_tmp/1000) + 1     ;//97M for 22K frequency accuracy   
    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);

    // set unconti timer
    data = 0x40;
    nim_reg_write(dev, RBA_AGC1_REPORT, &data, 1);

    //set receive timer: 0x88 for default;
    data = 0x88;
    nim_reg_write(dev, R8E_DISEQC_TIME, &data, 1);
    data = 0xff;
    nim_reg_write(dev, R8E_DISEQC_TIME + 0x01, &data, 1);
	// M3501C_DCN : may move those register to HW_INI
#ifdef DISEQC_OUT_INVERT
	// cr96[7] is DISEQC invert
	nim_reg_read(dev, R90_DISEQC_CLK_RATIO+6, &data, 1);
	data = data | 0x80;
	nim_reg_write(dev, R90_DISEQC_CLK_RATIO+6, &data, 1);
#endif
	// Open new tso
	// bit0: tso_bist, bit1, insert dummy, bit2, sync_lenght, bit3, vld gate,
	// bit 4, cap_eco_en, bit5, cap pid enable ,
	// bit[7:6] dsp clk sel: 00: 90m, 01:98m, 10:108m, 11:135m
	//data = 0x12;

	// paladin.ye 2015-11-12	
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic	
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	data = 0x42;    
#else        
	data = 0x40;
#endif

	//data = 0x02;//C3505 remove bit4
	//data = 0x42;
	//END
	
	nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);

	// TSO OEJ
	nim_reg_read(dev, RF0_HW_TSO_CTRL+5, &data, 1);
	data =data & 0xef;
	nim_reg_write(dev, RF0_HW_TSO_CTRL+5, &data, 1);

	// Set TSO bitrate margin, unit 1/16M,
	data = 0x08;
	nim_reg_write(dev, RF0_HW_TSO_CTRL+2, &data, 1);
    for (i = 0; i < 32; i++)//for mormal
    {
		nim_c3505_set_map_beta(dev, i,  1);     
    }

	 for (i = 0; i < 32; i++)//for short
    {
		nim_c3505_set_map_beta(dev, i,  0);
    }
	// FEC use SPI output to TSO, use fast speed
	data = 0x10;
	nim_reg_write(dev, RAD_TSOUT_SYMB+1, &data, 1);
	data = 0x02;
	nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);

	// add by paladin from bentao 2015-09-22
	nim_reg_read(dev, R12d_LDPC_SEL, &data, 1);
    if (CHIP_ID_3503D != priv->ul_status.m_c3505_type)
    {		
		data |= 0x20;	// for 3505/3503C only
	}
	else
	{
    	data |= 0x02;   // for 3503d only
	}
	nim_reg_write(dev, R12d_LDPC_SEL, &data, 1);
    // end

    // For debug task used, 0xed[0]  = 0 : open, 1: closed
    nim_reg_read(dev, 0xed, &data, 1); 
    data &= 0xfe;
    nim_reg_write(dev, 0xed, &data, 1); 

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_adc(struct nim_device *dev)
* configure the adc
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_adc(struct nim_device *dev)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	UINT8 data = 0, ver_data = 0;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	// ADC setting
	NIM_PRINTF(" qpsk_config value: %x\n",priv->tuner_config_data.qpsk_config);
	
	if (priv->tuner_config_data.qpsk_config & C3505_IQ_AD_SWAP)
	{
		data = 0x4a;
		NIM_PRINTF(" Warning C3505 IQ AD SWAP\n");
	}
	else
		data = 0x0a;

	nim_reg_write(dev, R01_ADC, &data, 1);
	nim_reg_read(dev, R01_ADC, &ver_data, 1);
	if (data != ver_data)
	{
		NIM_PRINTF(" wrong 0x8 reg write\n");
		return ERR_FAILED;
	}

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
* DSP clock set
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
{
	UINT8 data;
	//  NIM_PRINTF("Enter Fuction nim_C3505_set_dsp_clk \n");

	data = clk_sel & 0xff;
	nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);

	nim_reg_read(dev, RF1_DSP_CLK_CTRL, &data, 1);
	data = data & 0xfc;
	// bit[1:0] dsp clk sel: // 00: 135m, 01:96m 10:112m  11:XTAL 27Mhz
    if(ADC_SAMPLE_FREQ[1] == clk_sel)
	{
		data = data | (UINT8)(1) ;
    }
	else if(ADC_SAMPLE_FREQ[0] == clk_sel)
	{
		data = data | (UINT8)(0) ;
	}
	else if(ADC_SAMPLE_FREQ[2] == clk_sel)
	{
		data = data | (UINT8)(2) ;
	}
	else if(ADC_SAMPLE_FREQ[3] == clk_sel)
	{
		data = data | (UINT8)(3) ;
	}
	else	
  	{ 
  		NIM_PRINTF(" set dsp clock error!");
	}
       
	nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_get_dsp_clk(struct nim_device *dev, UINT8 *sample_rate)
*  get the DSP work clock 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sample_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate)
{
	UINT8 data;
	//  NIM_PRINTF("Enter Fuction nim_C3505_reg_get_modcode \n");

	nim_reg_read(dev, RF1_DSP_CLK_CTRL, &data, 1);
	data = data & 0x03;
	// bit[1:0] dsp clk sel: // 00: 135m, 01:96m, 10:112m, 11:XTAL CLK(default 27Mhz)
	if(0 == data)
		*sample_rate = 135000;  ////uint is KHz
	else if(1 == data)
		*sample_rate = 96430;  ////uint is KHz
	else if(2 == data)
		*sample_rate = 112500;  ////uint is KHz
	else if(3 == data)  // New for low symbol rate TP
		*sample_rate = XTAL_CLK;  ////uint is KHz

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_Case, UINT8 frz_agc1)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case, UINT8 frz_agc1)
{
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
    UINT8 data = 0;
	
    // AGC1 setting
    if (priv->tuner_config_data.qpsk_config & C3505_NEW_AGC1)
    {
      switch (s_case)
      {
        case NIM_OPTR_CHL_CHANGE:
            if (1 == low_sym)
                data = 0xaf;
            else
                data = 0xb5;
            break;
        case NIM_OPTR_SOFT_SEARCH:
            if (1 == low_sym)
                data = 0xb3;//0xb1;
            else
                data = 0xba;//0xb9;
            break;
        case NIM_OPTR_FFT_RESULT:
                data=0xb9;
            break;
        }
    }
	else
	{
		switch (s_case)
		{
			case NIM_OPTR_CHL_CHANGE:
			case NIM_OPTR_SOFT_SEARCH:
				if (1 == low_sym)
				data = 0x3c;
				else
				data = 0x54;
				break;
			case NIM_OPTR_FFT_RESULT:
				data = 0x54;
				break;
		}
	}
    if ((priv->tuner_config_data.qpsk_config & C3505_AGC_INVERT) == 0x0)  // STV6110's AGC be invert by QinHe
        data = data ^ 0x80;

	//if((priv->Tuner_Config_Data.qpsk_config & 0x100) == 0x100) //C3031 Tuner AGC polarity is reverse to other tuners
	
	//if(SUCCESS == call_tuner_command(dev, NIM_TUNER_C3031B_ID, &channel_freq_err))
	if(priv->tuner_type == IS_M3031)
		data = data | 0x80;

    NIM_PRINTF("Set AGC1 value = %x\n", data);

    nim_reg_write(dev, R07_AGC1_CTRL, &data, 1);

    data = 0x58;
    nim_reg_write(dev, R07_AGC1_CTRL+0x01, &data, 1);
    
    //**************** frz agc1 control add by Jalone*********************// 
    switch (frz_agc1)
    {
	    case NIM_FRZ_AGC1_OPEN:
			nim_reg_read(dev, R0A_AGC1_LCK_CMD, &data, 1);
			data = data | 0x80; // write AGC1_AUTO_FRZ_EN = 1
			nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);	
			break;
	    case NIM_FRZ_AGC1_CLOSE:
			nim_reg_read(dev, R0A_AGC1_LCK_CMD, &data, 1);
			data = data & 0x7f; // write AGC1_AUTO_FRZ_EN = 0
			nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);
			break;
	    default:
	      	NIM_PRINTF(" CMD for nim_c3505_set_agc1 ERROR!!!");
	      	break;    	
    }

    return SUCCESS;
}
/*****************************************************************************
* void nim_C3505_set_RS(struct nim_device *dev, UINT8 coderate)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 rs
*   symbol rate
*	
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_rs(struct nim_device *dev, UINT32 rs)
{
	UINT8 data = 0, ver_data[3] = {0};
	UINT32 sample_rate = 0;
	UINT32 temp = 0;

	nim_c3505_get_dsp_clk(dev, &sample_rate);
	//NIM_PRINTF("\t\t dsp clock is %d\n", sample_rate);
	NIM_PRINTF("\t\t set ts %d with dsp clock %d\n", rs, sample_rate/1000);
	temp = nim_c3505_multu64div(rs, 184320, sample_rate); // 184320 == 90000 * 2.048
	if(temp>0x16800)
	{    
		NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", temp);
		temp = 0x16800;
	}
	//CR3F
	ver_data[0] = (UINT8) (temp & 0xFF);
	ver_data[1] = (UINT8) ((temp & 0xFF00) >> 8);
	ver_data[2] = (UINT8) ((temp & 0x10000) >> 16);
	nim_reg_write(dev, R5F_ACQ_SYM_RATE, ver_data, 3);
	if (rs < 3000)
	{
		if (rs < 2000)
			data = 0x08;
		else
			data = 0x0a;

		nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
	}
	else
	{
		data = 0x1f;
		nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
	}
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3505_set_polar(struct nim_device *dev, UINT8 polar)
* Description: C3505 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
//#define REVERT_POLAR
INT32 nim_c3505_set_polar(struct nim_device *dev, UINT8 polar)
{
	UINT8 data = 0;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	if (priv->ext_lnb_control)
		return priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_SET_POLAR, (UINT32) polar);

	nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);

	if ((priv->tuner_config_data.qpsk_config& C3505_POLAR_REVERT) == 0x00) //not exist H/V polarity revert.
	{
		if (NIM_PORLAR_HORIZONTAL == polar)
		{
			data &= 0xBF;
			NIM_PRINTF("nim_C3505_set_polar CR7C is 00\n");
		}
		else if (NIM_PORLAR_VERTICAL == polar)
		{
			data |= 0x40;
			NIM_PRINTF("nim_C3505_set_polar CR7C is 40\n");
		}
		else
		{
			NIM_PRINTF("nim_C3505_set_polar error\n");
			return ERR_FAILUE;
		}
	}
	else//exist H/V polarity revert.
	{
		if (NIM_PORLAR_HORIZONTAL == polar)
		{
			data |= 0x40;
			NIM_PRINTF("nim_C3505_set_polar CR7C is 40\n");
		}
		else if (NIM_PORLAR_VERTICAL == polar)
		{
			data &= 0xBF;
			NIM_PRINTF("nim_C3505_set_polar CR7C is 00\n");
		}
		else
		{
			NIM_PRINTF("nim_C3505_set_polar error\n");
			return ERR_FAILUE;
		}
	}

	if(priv->tuner_config_data.disqec_polar_position != 0)
	{
		if(data & 0x40)
			HAL_GPIO_BIT_SET(priv->tuner_config_data.disqec_polar_position, 1);
		else
			HAL_GPIO_BIT_SET(priv->tuner_config_data.disqec_polar_position, 0);
	}
	else
	{
		nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
	}

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_FC_Search_Range(struct nim_device *dev, UINT8 s_Case, UINT32 rs)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
    UINT8 data;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
    UINT8 reg59_value;
                 
    nim_c3505_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);

    nim_reg_read(dev, 0x59, &reg59_value, 1);
    switch (s_case)
    {
        case NIM_OPTR_SOFT_SEARCH:
            {
                reg59_value &= 0xFE;//clean bit0
                nim_reg_write(dev, 0x59, &reg59_value, 1);
                if (rs > 18000)
                    temp = 5 * 16; 
                else if (rs > 15000)
                    temp = 4 * 16;
                else if (rs > 10000)
                    temp = 3 * 16;
                else if (rs > 5000)
                    temp = 2 *16;
                else
                    temp = 1 *16;
                data = temp & 0xff;
                nim_reg_write(dev, R62_FC_SEARCH, &data, 1);
                data = (temp >> 8) & 0x3;
                
                if (rs > 10000)
                    data |= 0xa0;   // amy change for 138E 12354V43000 est 1756/37333
                else if (rs > 5000)
                    data |= 0xc0;       //amy change for 91.5E 3814/V/6666
                else
                    data |= 0xb0;       //amy change for 91.5E 3629/V/2200
                    
                nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
                priv->t_param.t_reg_setting_switch |= NIM_SWITCH_FC;
            }
            break;
        case NIM_OPTR_CHL_CHANGE:
            //if (priv->t_Param.t_reg_setting_switch & NIM_SWITCH_FC)
            {
                // set sweep range
                reg59_value |= 0x01;//set bit0
                nim_reg_write(dev, 0x59, &reg59_value, 1);
                
                if (rs > 18000)
                    temp = 5 * 16; 
                else if (rs > 15000)
                    temp = 4 * 16;
                else if (rs > 4000)
                    temp = 3 * 16;
                else
                    temp = 2 *16;

                temp = (temp * 90 + sample_rate/2) / sample_rate;   // Add for 27Mhz clk

                data = temp & 0xff;
                nim_reg_write(dev, R62_FC_SEARCH, &data, 1);
                
                nim_reg_read(dev, R62_FC_SEARCH + 0x01, &data, 1);
                data &= 0xfc;
                data |= (temp >> 8) & 0x3;
                data |= 0xb0;
                nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
                priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_FC;
            }
            break;
        default:
            NIM_PRINTF(" CMD for nim_C3505_set_FC_Search_Range ERROR!!!");
            break;
    }
    return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_set_RS_Search_Range(struct nim_device *dev, UINT8 s_Case, UINT32 rs)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*  Parameter3: UINT32 rs
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
    UINT8 data = 0;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
    UINT8 reg59_value;
                 
    nim_c3505_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);
    nim_reg_read(dev, 0x59, &reg59_value, 1);//Symbol Rate sweep range Register

    switch (s_case)
    {
        case NIM_OPTR_SOFT_SEARCH:
            { 
                reg59_value &= 0xFE;//clean bit0
                nim_reg_write(dev, 0x59, &reg59_value, 1);
                temp =(rs + 1000) / 2000;
                if (temp < 2)
                    temp = 2;
                else if (temp > 11)
                    temp = 11;
                temp = temp << 4;
                data = temp & 0xff;
                nim_reg_write(dev, R64_RS_SEARCH, &data, 1); ////CR64
                data = (temp >> 8) & 0x3;
                data |= 0xa0;       ////RS_Search_Step=2
                nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);////CR65
                priv->t_param.t_reg_setting_switch |= NIM_SWITCH_RS;
            }
            break;
        case NIM_OPTR_CHL_CHANGE:
            if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_RS)
            {
                temp = (3 * 90 * 16+sample_rate/2) /sample_rate;
                data = temp & 0xff;
                nim_reg_write(dev, R64_RS_SEARCH, &data, 1);
                data = (temp >> 8) & 0x3;
                data |= 0x30;
                nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);
                priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_RS;
            }
            break;
        default:
            NIM_PRINTF(" CMD for nim_C3505_set_RS_Search_Range ERROR!!!");
            break;
    }
    return SUCCESS;
}


/*****************************************************************************
* void nim_c3505_cfg_tuner_get_freq_err(struct nim_device *dev)
*  Set tuner(freq rs) and get tuner freq err to correct demod offset value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_tp_scan_para *tp_scan_param
*  
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cfg_tuner_get_freq_err(struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param)
{
	UINT32	exceed_offset = 0;
    UINT8 freq_offset = 0;  // For 3m offset flag
    
	struct nim_c3505_private* dev_priv;

    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	dev_priv = dev->priv;

    if (NULL == dev_priv)
	{
		NIM_PRINTF("[%s %d]NULL==dev_priv\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}

	if((0 == tp_scan_param->est_rs) || ( 0 == tp_scan_param->est_freq))
    {   
        ERR_PRINTF("Exit with ERR_INPUT_PARAM in %s \n",__FUNCTION__);
		//return ERR_INPUT_PARAM;
		return ERR_NO_MEM;
    }
    
	/*if((tp_scan_param->est_freq >= (UINT32)(dev_priv->Tuner_Config_Data.Recv_Freq_High - QPSK_TUNER_FREQ_OFFSET)) || 
	   (tp_scan_param->est_freq <= (UINT32)(dev_priv->Tuner_Config_Data.Recv_Freq_Low + QPSK_TUNER_FREQ_OFFSET)))
		exceed_offset = 1;*/
		exceed_offset = 0;
    
	//	Symbol rate is less than 6.5M, low symbol rate 	
	tp_scan_param->low_sym = tp_scan_param->est_rs < C3505_LOW_SYM_THR ? 1 : 0;	

    // When is low symbol rate ,set tuner freq add 3M, and rs + 3M *2 
	if (dev_priv->tuner_config_data.qpsk_config & C3505_QPSK_FREQ_OFFSET)
	{
		if((0 == exceed_offset) && (1 == tp_scan_param->low_sym))
		{
			tp_scan_param->est_freq += QPSK_TUNER_FREQ_OFFSET * 1024;
            freq_offset = 1;
            // only correct for M3031 
            if(IS_M3031 == dev_priv->tuner_type)
			    tp_scan_param->correct_rs = QPSK_TUNER_FREQ_OFFSET * 2048;
            else
			    tp_scan_param->correct_rs = 0; 
		}
	}

    tp_scan_param->tuner_freq= tp_scan_param->est_freq / 1024;
    tp_scan_param->delfreq += tp_scan_param->est_freq % 1024;

    //Tuner if 
    if(tp_scan_param->delfreq > 512)
    {
        tp_scan_param->delfreq -= 1024;
        tp_scan_param->tuner_freq += 1;
    }


    // Clear tuner freq err
    tp_scan_param->freq_err = 0;

	if(IS_M3031 == dev_priv->tuner_type)
	{
        call_tuner_command(dev, NIM_TUNER_SET_C3031B_FREQ_ERR, &(tp_scan_param->freq_err));

        // Set tuner(freq , rs)
		if (dev_priv->nim_tuner_control != NULL)
		{
        	dev_priv->nim_tuner_control(dev_priv->tuner_index, tp_scan_param->tuner_freq, tp_scan_param->est_rs + tp_scan_param->correct_rs);
		}
		else
		{
			NIM_PRINTF("[%s %d]dev_priv->nim_tuner_control == NULL\n", __FUNCTION__, __LINE__);
			return ERR_FAILURE;
		}

        // Get tuner freq err
        call_tuner_command(dev, NIM_TUNER_GET_C3031B_FREQ_ERR, &(tp_scan_param->freq_err));
    }
    else
    {
        // Set tuner(freq , rs)
		if (dev_priv->nim_tuner_control != NULL)
        {
        	dev_priv->nim_tuner_control(dev_priv->tuner_index, tp_scan_param->tuner_freq, tp_scan_param->est_rs + tp_scan_param->correct_rs);
    	}
		else
		{	
			NIM_PRINTF("[%s %d]dev_priv->nim_tuner_control == NULL\n", __FUNCTION__, __LINE__);
			return ERR_FAILURE;
		}
	}
    // Wait agc stable 
    comm_delay(500);

	tp_scan_param->delfreq -= (tp_scan_param->freq_err * 4); //kHz (1/256)*1024=4
	
    if (1 == freq_offset)
    {
        tp_scan_param->delfreq -= QPSK_TUNER_FREQ_OFFSET * 1024;
    }
       
    tp_scan_param->delfreq = (tp_scan_param->delfreq*1000)/1024;


    // real tuner if freq
    tp_scan_param->est_freq = ((INT32)tp_scan_param->tuner_freq) * 1024 + tp_scan_param->freq_err * 4;
 

    AUTOSCAN_PRINTF("Real tuner freq = %d ,  set_tuner_if = %d , tp_scan_param->freq_err = %d, delta freq = %d\n", tp_scan_param->est_freq, tp_scan_param->tuner_freq, tp_scan_param->freq_err, tp_scan_param->delfreq);
	
	return SUCCESS;
}


/*****************************************************************************
* void nim_C3505_set_freq_offset(struct nim_device *dev, INT32 delfreq)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_freq_offset(struct nim_device *dev, INT32 delfreq)
{
	UINT8 data[3];
	UINT32 temp, delfreq_abs;
	UINT32 sample_rate;
	//delfreq unit is KHz, sample_rate unit is KHz

	if(delfreq < 0)
		delfreq_abs = 0 - delfreq;
	else
		delfreq_abs = delfreq;

	nim_c3505_get_dsp_clk(dev, &sample_rate);
	temp = nim_c3505_multu64div(delfreq_abs, 92160, sample_rate); // 92160 == 90000 * 1.024

	if(temp>0xffff)
	{    
		NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", temp);
		temp = 0xffff;
	}
	if(delfreq<0)
		temp = (temp^0xffff) | 0x10000;
	else
		temp = temp & 0xffff ;
	//CR5C
	data[0] = (UINT8) (temp & 0xFF);
	data[1] = (UINT8) ((temp & 0xFF00) >> 8);
	data[2] = (UINT8) ((temp & 0x10000) >> 16);
	nim_reg_write(dev, R5C_ACQ_CARRIER, data, 3);

	return SUCCESS;
}



/*****************************************************************************
*  INT32 nim_C3505_cr_setting(struct nim_device *dev, UINT8 s_Case)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_setting(struct nim_device *dev, UINT8 s_case)
{
	UINT8 data = 0;

	switch (s_case)
	{
    	case NIM_OPTR_SOFT_SEARCH:
    		// set CR parameter
			nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
			data = data|0x08;     ///force old CR
			nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);

    		data = 0xaa;
    		nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
    		data = 0x45;
    		nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
    		data = 0x97;
    		nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);
    		data = 0x87;
    		nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
    		break;
    	case NIM_OPTR_CHL_CHANGE:
    		// set CR parameter
#if 0	// For cr fake lock by hyman 2016/12/26
    		nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
			data = data&0xf7;    ///disable old CR
			nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);

    		data = 0xaa;    // old
    		nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
    		data = 0x4a;
    		nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
            data = 0x86;
    		nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);
    		data = 0x88; 
    		nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);

#else
    		nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
			data = data&0xf7;    ///disable old CR
			nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);

            data = 0x0f;//data = 0x4f;	// For low symbol rate tp lock speedup, 07/14/2017 hyman
            nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);

            data = 0x42;//data = 0x4a;	// For low symbol rate tp lock speedup, 07/14/2017 hyman
            nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);

            data = 0x86;
            nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);
            // end

     		data = 0x88; 
    		nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);           
#endif


            
    		break;
    	default:
    		NIM_PRINTF(" CMD for nim_C3505_cr_setting ERROR!!!");
    		break;
	}

	return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_C3505_TR_CR_Setting(struct nim_device *dev, UINT8 s_Case)
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 s_Case
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_tr_cr_setting(struct nim_device *dev, UINT8 s_case)
{
	 UINT8 data = 0;
	 struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	 switch (s_case)
	 {
	 	case NIM_OPTR_SOFT_SEARCH:
	 	if (!(priv->t_param.t_reg_setting_switch & NIM_SWITCH_TR_CR))
		 {
		     nim_reg_read(dev, R66_TR_SEARCH, &data, 1);
             data &= 0xc0;
             data |= 0x09;
			 //data = 0x49;//0x4d;
			 nim_reg_write(dev, R66_TR_SEARCH, &data, 1);
			 data = 0x31;
			 nim_reg_write(dev, R67_VB_CR_RETRY, &data, 1);
	 	     priv->t_param.t_reg_setting_switch |= NIM_SWITCH_TR_CR;
#ifdef ASKEY_PATCH3
             data = 0xb1;
             nim_reg_write(dev, R18_TR_CTRL + 2, &data, 1);
         
             data = 0x24;
             nim_reg_write(dev, R16_AGC2_REFVAL, &data, 1);  
#endif        
		 }
		 break;
	 	case NIM_OPTR_CHL_CHANGE:
		 if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_TR_CR)
		 {
			 // set reg to default value
		     nim_reg_read(dev, R66_TR_SEARCH, &data, 1);
             data &= 0xc0;
             data |= 0x19;
			 //data = 0x49;//0x4d; 
			 //data = 0x59;
			 nim_reg_write(dev, R66_TR_SEARCH, &data, 1);
			 data = 0x33;
			 nim_reg_write(dev, R67_VB_CR_RETRY, &data, 1);
			 priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_TR_CR;
#ifdef ASKEY_PATCH3
             data = 0xf2;
             nim_reg_write(dev, R18_TR_CTRL + 2, &data, 1);
         
             data = 0x18;
             nim_reg_write(dev, R16_AGC2_REFVAL, &data, 1);  
#endif   
		 }
		 break;
	 }
	 return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_C3505_cr_adaptive_initial (struct nim_device *dev)
*
*
*	Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_adaptive_initial (struct nim_device *dev)
{	  
#ifdef SW_ADPT_CR
	UINT8 data;
	// Just for debug now, should depends on MODCOD
	data = 0xc3;
	nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); // enable SNR estimation
	data = 0x9e;
	nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); // Bypass Adpt ForceOldCR
	nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
	data |= 0x08;
	nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1); // force old CR, just for debug now
#endif
 
	#ifdef HW_ADPT_CR
 
		 nim_C3505_cr_tab_init(dev);
		 //Bypass Setting: All use CR adaptive table
		 data = 0x81;
		 nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
 
		#ifdef HW_ADPT_CR_MONITOR
			 data = 0xe6; // enable hw adpt CR report, also enable always update to regfile, latter need review
			 nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
		#endif
	#endif
	 
	 return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_cr_adaptive_configure (struct nim_device *dev)
*
*
*	Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_adaptive_configure (struct nim_device *dev, UINT32 sym)
{
#ifdef HW_ADPT_CR
	 UINT8 data = 0;
	 UINT8 TabID = 0;
	 UINT32 TabVal = 0,TabValTemp = 0;
	 UINT8 DatArray[2] = {0};
#endif

#ifdef SW_ADPT_CR
	 snr_initial_en = 1;
#endif // SW_ADPT_CR
 
#ifdef HW_ADPT_CR
	 NIM_PRINTF("			 nim m3501c close dummy for dvbs \n");
 
	 // To circumvent the symbol rate 13M problem
	 /*  TAB_ID=15, CellID = 2
		 WIDE_2 |  Reserved  |<------8PSK POFF-------->|  S2_QPSK | 	   |
							 |9/10 8/9 5/6 3/4 2/3 3/5 |PON POFF  | DVBS
		 0x002 :			 |0    0   0   0   0   0   |0	1	  | 0		 */
 
	 // config ON/OFF Table of WIDE_2
	 data = 0xe1; // CR Tab init en
	 nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
 
	 data = 0xf0 | 2; // TAB_ID | CellID
	 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
	 if(sym<=10000) // enable wider CR when S2 QPSK Rs<=10M
		 DatArray[0] = 0x02;
	 else
		 DatArray[0] = 0x00;
	 DatArray[1] = 0x80;
	 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	 // End of to circumvent the symbol rate 13M problem
 
	 // PRS=8,IRS=0x12 for 8PSK pilot off Rs<3M
	 if(sym <= 5000)
		 TabVal = 0x10600;
	 else if((sym > 5000)&&(sym < 17000))//for pn_astraf.fcf 20M symbol rate performance worse than C3501C 20121204
	 //  TabVal = CR_PARA_8PSK_others[TAB_SIZE-1];
		 TabVal = 0x10700;
	 else		 
		 TabVal = 0x10800;
	 TabValTemp = (TabVal & 0x7c) << 7;
	 TabVal = (TabVal>>8) |TabValTemp | 0x8000;  // to HW format;
	 DatArray[0] = (UINT8)(TabVal & 0x0ff);
	 DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
 
	for(TabID = 4; TabID <= 6; TabID++)
	{
		data = (TabID<<4) | (PSK8_TAB_SIZE-1);//12.5
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);

		data = (TabID<<4) | (PSK8_TAB_SIZE-2);//12.0
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);

		data = (TabID<<4) | (PSK8_TAB_SIZE-3);//11.5
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	}
	if(sym >= 17000)
	{
		for(TabID = 4; TabID <= 6; TabID++)
		{
			data = (TabID << 4) | (PSK8_TAB_SIZE - 4);//11.0
			nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
			data = (TabID << 4) | (PSK8_TAB_SIZE - 5);//10.5
			nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);

			data = (TabID << 4) | (PSK8_TAB_SIZE - 6);//10.0
			nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
			data = (TabID << 4) | (PSK8_TAB_SIZE - 7);//9.5
			nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
		}
	}
	//end of Rs<3M issue
	if(sym <= 1500)
	{	 
		data = 0xf0 | 0;		 //force old cr
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		TabVal = 0x19 | 0x8000;
		DatArray[0] = (UINT8)(TabVal & 0x0ff);
		DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);

		data = 0xf0 | 3;	 //enable snr est	 
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		TabVal = 0x7fa | 0x8000;
		DatArray[0] = (UINT8)(TabVal & 0x0ff);
		DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);

		//pilot_on_gain=0,head_ped_gain=0 is same as M3501B for 1M pilot on lock slow issue
		nim_reg_read(dev, 0x127, &data, 1);		   
		data = data & 0x83;
		nim_reg_write(dev, 0x127, &data, 1);		 
	}
 
	#ifdef HW_ADPT_CR_MONITOR
		 data = 0xe6; // enable hw adpt CR report, also enable always update to regfile, latter need review
	#else
		 data = 0xe0; // CR Tab init off
	#endif
	 nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
#endif // HW_ADPT_CR

	 return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_cr_tab_init(struct nim_device *dev)
* 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_tab_init(struct nim_device *dev)
{
	UINT8 TabID = 0,CellID = 0;
	UINT32 TabVal = 0, TabValTemp = 0;
	UINT8 data = 0;
	UINT8 DatArray[2] = {0};
	 /*  TAB_ID=15, CellID = 0,1,2,3
	 bits		  14 13 12 11	10	   9	   8	   7		6	 5		  4    3	   2	 1		  0
	 OLD_CR |		  Reserved	   |  32APSK   |  16APSK  |  8PSK	| S2_QPSK | 	   |
									  PON  POF	 PON  POF  PON POF	 PON POFF  DVBS
		0x01						  0 	0	  0    0   0   1	  0  0		 1
	 WIDE_1 |	   Reserved   | 	32	   |   16	|<------16APSK POFF------>| 	   |
								 PON POF	PON   9/10	8/9  5/6  4/5  3/4	2/3 8_PON
		0x81					 0	  0 	 0	   0	 0	 0	   0	0	 0	   0 
	 WIDE_2 |	Reserved				 |<------8PSK POFF-------->|  S2_QPSK | 	   |	 
									   9/10  8/9  5/6  3/4	2/3  3/5   PON POFF   DVBS
		0x006						   0	 0	  0    0	0	  0 	0	 1		 0
	 SNR_EN 	  |   32   | 16APSK |	  |<------8PSK POFF---------->|  S2_QPSK |	   |
				   ON POF	 PON POF 8ON 9/10  8/9	5/6  3/4  2/3  3/5	 PON  POFF	DVBS
	   0x5f8	   0   0	 0	 1	  0   1    1	1	 1	  1 	1	  0    1	 0	
	 */
	// OLD_CR	WIDE_1	WIDE_2	SNR_EN	CR_FIX	HEAD
	//const unsigned OTHERS_TAB[6] = {0x09,	0x00,	 0x002,   0x5fa,	0x000,	 0x00};
	const unsigned OTHERS_TAB[6] = {0x09,	  0x00,    0x002,	0x5fa,	  0x000,   0x00};
	//const unsigned OTHERS_TAB[6] = {0x00,	0x00,	 0x002,   0x5fa,	0x000,	 0x00};
 
#ifdef HW_ADPT_CR_MONITOR
	ADPT_CR_PRINTF("CR TAB Initialization Begin \n");
#endif
	data = 0xe1; // CR Tab init en
	nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

	// Write SNR threshold low, TabID=0~1;
	for(CellID = 0; CellID < SNR_TAB_SIZE; CellID++)
	{
		TabID =  (CellID >= 16) ? 1 : 0;
		data = (TabID << 4) | (CellID & 0x0f);
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		TabVal = c3505_snr_thr[2*CellID]  | 0x8000;
		DatArray[0] = (UINT8)(TabVal & 0x0ff);
		DatArray[1] = (UINT8)((TabVal >> 8) & 0x0ff);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	}
 
	// Write SNR threshold high, TabID=2~3;
	for(CellID = 0; CellID < SNR_TAB_SIZE; CellID++)
	{
		TabID =  (CellID >= 16) ? 3 : 2;
		data = (TabID << 4) | (CellID & 0x0f);
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		TabVal = c3505_snr_thr[2*CellID+1] | 0x8000;
		DatArray[0] = (UINT8)(TabVal & 0x0ff);
		DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
		nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	}
 
	 // Write CR PRS/IRS/FRAC/HEAD_GAIN
	 // SW format: {IRS[4:0],  PRS[3:0], HEAD_GAIN[3:0], FRAC[3:0],}
	 // HW format: {HEAD_GAIN[2:0],FRAC[1:0],IRS[4:0], PRS[3:0]}
		 // 8PSK 3/5, TabID=4;
	 for(CellID = 0; CellID < PSK8_TAB_SIZE; CellID++)
	 {
		 data = (4<<4) | CellID;
		 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		 TabVal = c3505_cr_para_8psk_3f5[CellID];
		 TabValTemp = (TabVal & 0x7c)<<7;
		 TabVal = (TabVal >> 8) | TabValTemp | 0x8000;  // to HW format;
		 DatArray[0] = (UINT8)(TabVal & 0x0ff);
		 DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
		 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	 }
 
	 // 8PSK 2/3 ~ 9/10,  3/4~9/10 share one group
	 for(TabID = 5; TabID <= 6; TabID++)
	 {	 
		 for(CellID = 0; CellID < PSK8_TAB_SIZE; CellID++)
		 {
			 data = (TabID<<4) | CellID;
			 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			 TabVal = c3505_cr_para_8psk_others[CellID];
			 TabValTemp = (TabVal & 0x7c)<<7;
			 TabVal = (TabVal>>8) |TabValTemp | 0x8000;  // to HW format;
			 DatArray[0] = (UINT8)(TabVal & 0x0ff);
			 DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
			 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
		 }
	 }
 
	 // QPSK: TabID=7~9, need parameter table later
	 for(TabID = 7; TabID <= 9; TabID++)
	 {	 
		 for(CellID = 0; CellID < QPSK_TAB_SIZE; CellID++)
		 {
			 data = (TabID << 4) | CellID;
			 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			 TabVal = 0x129 | 0x8000;
			 DatArray[0] = (UINT8)(TabVal & 0x0ff);
			 DatArray[1] = (UINT8)((TabVal >> 8) & 0x0ff);
			 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
		 }
	 }
 
	 // 16APSK: TabID=10~12
	 for(TabID = 10; TabID <= 12; TabID++)
	 {	 
		 for(CellID = 0; CellID < APSK16_TAB_SIZE; CellID++)
		 {
			 data = (TabID<<4) | CellID;
			 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
			 TabVal = 0x129 | 0x8000;
			 DatArray[0] = (UINT8)(TabVal & 0x0ff);
			 DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
			 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);  
		 }
	 }
	 
			 // ON/OFF Table
	  for(CellID = 0; CellID < 6; CellID++)
	 {
		 data = 0xf0 | CellID;
		 nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
		 TabVal = OTHERS_TAB[CellID] | 0x8000;
		 DatArray[0] = (UINT8)(TabVal & 0x0ff);
		 DatArray[1] = (UINT8)((TabVal>>8) & 0x0ff);
		 nim_reg_write(dev, R11_DCC_OF_I, DatArray, 2);
	 }		 
	 
	 data = 0xe0; // CR Tab init off
	 nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
 
	#ifdef HW_ADPT_CR_MONITOR
		 ADPT_CR_PRINTF("CR TAB Initialization Done \n");
	#endif
	 return SUCCESS;
}
/*****************************************************************************

*  INT32 nim_C3505_cr_new_tab_init(struct nim_device *dev)
*
* NEW CR TAB initial
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_new_tab_init(struct nim_device *dev)
{
	//  initial TAB of manson's adaptive TAB
	/*  add hy Hongyu
	*	 
	*
	*	 wire [2:0] TAB_ID = CR_PARA_ADDR[7:5];
	*	 wire [4:0] TAB_CELL = CR_PARA_ADDR[4:0];
	*	 CR_PARA_ADDR = reg_cr9d_t: this is the same as the old CR ADPT method.

	*	 reg [29:0] S2_CODE_RATE_TABLE [27:0];
	*	 reg [16:0] AWGN_COEF_TABLE [7:0];
	*	 reg [18:0] LOOP_COEF_TABLE [7:0];
	*	 reg [22:0] CLIP_PED_TABLE [7:0];
	*	 reg [25:0] AVG_PED_TABLE [7:0];
	*	 reg [29:0] S_CODE_RATE_TABLE[5:0];

	*	 wire sel_S2_CODE_RATE		= (TAB_ID==0);
	*	 wire sel_AWGN_COEF 	 = (TAB_ID==1);
	*	 wire sel_LOOP_COEF 	 = (TAB_ID==2);
	*	 wire sel_CLIP_PED		 = (TAB_ID==3);
	*	 wire sel_AVG_PED		 = (TAB_ID==4);
	*	 wire sel_S_CODE_RATE	   = (TAB_ID==5);
	*
	*	 S2_CODE_RATE_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[29:0];
	*	 AWGN_COEF_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[16:0];
	*	 LOOP_COEF_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[18:0];
	*	 CLIP_PED_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[22:0];
	*	 AVG_PED_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[25:0];
	*	 S_CODE_RATE_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[29:0];
	*	 
	*	 CR_PARA_DIN[29:0]={
	*						 reg_cr131[6:0]  // 29:23
	*						 reg_cr130[7:0]; //  22:15
	*						 reg_cr12_l[6:0],	//	 14:8
	*						 reg_cr11_l[7:0] //  7:0
	*						 }
	*	 CR_PARA_WE_2 = reg_cr131[7];
	*
	*	 step:	 1. write CR_PARA_ADDR: TAB_ID and TAB_CELL
	*			 2. write CR_PARA_DIN[14:0]: or write reg_cr11,reg_cr12, but reg_cr12[7]=0
	*			 3. write reg_cr130 and reg_cr131: with reg_cr131[7]=1
	*/	
	UINT8 TabID,CellID;
	UINT16 TabVal_14_0,TavVal_29_15;//,temp;
	UINT8 data;
	UINT8 DatArray[2];

	//data = 0xe1;
	//nim_reg_write(dev, R0E_ADPT_CR_CTRL,&data,1); // CR Tab init en
	// CR_TAB_INT =1

	// wire sel_CODE_RATE		= (TAB_ID==0);
	// CODE_RATE_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[29:0];
	UINT32 S2_CODE_RATE_TABLE[28];
	UINT32 AWGN_COEF_TABLE[8];
	UINT32 LOOP_COEF_TABLE[8];
	UINT32 CLIP_PED_TABLE[8];
	UINT32 AVG_PED_TABLE[8];
	UINT32 S_CODE_RATE_TABLE[6];

	ADPT_NEW_CR_PRINTF("NEW CR TAB Initialization Begin \n");
	/*
	1) S2_CODE_RATE_TABLE size = 28*30 = 840 bits
	index = {code_rate}, 28 index
	value = { //30 bits
	S2_AWGN_NOISE, 			//10 bits, EST_NOISE (MSBs) level to apply AWGN coefficients
	S2_AWGN_COEF_SET,	//3 bit, select one set from the AWGN_COEF table
	S2_PON_IRS_DELTA,	//3 bit signed, subtract from AWGN IRS when Pilot On
	S2_PON_PRS_DELTA,	//3 bit signed, subtract from AWGN PRS when Pilot On
	S2_LOOP_COEF_SET,	//3 bit, select one set from the CLIP_PED table
	S2_CLIP_PED_SET,	//3 bit, select one set from the CLIP_PED table
	S2_AVG_PED_SET,	//3 bit, select one set from the AVG_PED table
	S2_FORCE_OLD_CR,	//1 bit, only affect pilot off
	S2_LLR_SHIFT , 			//1 bit
	} 
	*/
	for(CellID = 0; CellID < 28; CellID++)
	{
		//CellID=11;
		TabID =  0;
		S2_CODE_RATE_TABLE[CellID] = 								        \
			((((c3505_s2_awgn_noise[CellID]) << 1) & 0x00000ffe) << 20) | \
			((c3505_s2_awgn_coef_set[CellID] & 0x00000007)<<17) | 		    \
			((c3505_s2_pon_irs_delta[CellID] & 0x00000007)<<14) | 		    \
			((c3505_s2_pon_prs_delta[CellID] & 0x00000007)<<11) | 		    \
			((c3505_s2_loop_coef_set[CellID] & 0x00000007)<<8) | 			\
			((c3505_s2_clip_ped_set[CellID] & 0x00000007)<<5) | 			\
			((c3505_s2_avg_ped_set[CellID] & 0x00000007)<<2) | 			    \
			((c3505_s2_force_old_cr[CellID] & 0x00000001)<<1) | 			\
			(c3505_s2_llr_shift[CellID] & 0x00000001);
		//step 1: 
		data = (TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );
		//step 2:
		TabVal_14_0 = (UINT16)((S2_CODE_RATE_TABLE[CellID] & 0x7fff)); // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0 >> 8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(S2_CODE_RATE_TABLE[CellID] >> 15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15 >> 8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2); 
	}

	/*
	2) AWGN_COEF_TABLE size = 8 * 17 = 136 bits  //AVG_PED and Wider_Loop and disabled for AWGN, SYMBOL_BEFORE_HEAD only affect pilot off
	index = {AWGN_COEF_SET} 8 index
	value = {
	IRS,							 //5 bits, IRS_TRA_S2 for AWGN
	IRS_FRACTION,			 //2 bits
	PRS,							 //4 bits
	PRS_FRACTION,			 //2 bits
	HEAD_PED_GAIN, 		 //3 bits
	SYMBOL_BEFORE_HEAD 	 //1 bit, only affect pilot off with AWGN SNR
	}

	*/
	// wire sel_AWGN_COEF		= (TAB_ID==1);
	// AWGN_COEF_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[16:0];
	for(CellID = 0; CellID < 8; CellID++)
	{
		TabID = 1;
		//CellID=0;
		AWGN_COEF_TABLE[CellID] = 						\
			((c3505_irs[CellID] & 0x0000001f)<<12)  | 		\
			((c3505_irs_fraction[CellID] & 0x00000003)<<10) | \
			((c3505_prs[CellID] & 0x0000000f)<<6) | 			\
			((c3505_prs_fraction[CellID] & 0x00000003)<<4) | 	\
			((c3505_head_ped_gain[CellID] & 0x00000007)<<1) |	\
			(c3505_symbol_before_head[CellID] & 0x00000001);
		//step 1: 
		data=(TabID<<5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((AWGN_COEF_TABLE[CellID]  & 0x7fff));	 // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0]=(UINT8)TabVal_14_0;
		DatArray[1]=((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(AWGN_COEF_TABLE[CellID]>>15);
		DatArray[0]=(UINT8)(TavVal_29_15);
		DatArray[1]=(UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);

	}
 
	/*
	3) LOOP_COEF_TABLE size = 8 * 19 = 152 bits
	index = {LOOP_COEF_SET} 8 index
	value = {
	HEAD_GAIN_SNR_DELTA,	 //3 bits, SNR_DELTA (dB) to disable HEAD_PED_GAIN, if HEAD_PED_GAIN is big, disable it result in big bandwidth increase
	MAX_SNR_DELTA, 				 //3 bits, clip SNR_DELTA so that the bandwidth doesn't become too big for high SNR_DELTA
	IRS_STEP,								 //3 bits, IRS step for each SNR_DELTA, have 2 bits fraction (same as IRS_FRACTION)
	PRS_STEP,								 //3 bits
	WIDER_SNR_DELTA,				 //3 bits, SNR_DELTA to enable WIDER_LOOP, =7 means WIDER_LOOP always disabled
	WIDER_IRS_DELTA,				 //2 bits
	WIDER_PRS_DELTA				 //2 bits
	}
	*/
	// wire sel_LOOP_COEF		= (TAB_ID==2);
	// LOOP_COEF_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[18:0];
	for(CellID = 0; CellID < 8; CellID++)
	{
		//CellID=0;
		TabID =  2;
		LOOP_COEF_TABLE[CellID] =								        \
			((c3505_head_gain_snr_delta[CellID] & 0x00000007)<<16) |	\
			((c3505_max_snr_delta[CellID] & 0x00000007)<<13) | 		    \
			((c3505_irs_step[CellID] & 0x00000007)<<10) | 			    \
			((c3505_prs_step[CellID] & 0x00000007)<<7) | 				\
			((c3505_wider_snr_delta[CellID] & 0x00000007)<<4) | 		\
			((c3505_wider_irs_delta[CellID] & 0x00000003)<<2) | 		\
			(c3505_wider_prs_delta[CellID] & 0x00000003);
		//step 1: 
		data=(TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);

		//step 2:
		TabVal_14_0 = (UINT16)((LOOP_COEF_TABLE[CellID]  & 0x7fff));	 // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0]=(UINT8)TabVal_14_0;
		DatArray[1]=((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(LOOP_COEF_TABLE[CellID]>>15);
		DatArray[0]=(UINT8)(TavVal_29_15);
		DatArray[1]=(UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);
	}
 
	/*
	4) CLIP_PED_TABLE, size = 8*23 = 184 bits
	index = {CLIP_PED_SET}, 8 index
	value = {
	CLIP_MULT_STEP, //5 bits, step to increase CLIP_PED1 and 2 if SNR_DELTA > 0
	CLIP_PED1_EN,	 //1 bit
	CLIP_PED1, 			 //8 bits
	CLIP_PED2_EN,	 //1 bit
	CLIP_PED2				 //8 bits
	}
	*/
	// wire sel_CLIP_PED		= (TAB_ID==3);
	// CLIP_PED_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[22:0];
	for(CellID=0; CellID<8; CellID++)
	{
		//CellID=0;
		TabID =  3;
		CLIP_PED_TABLE[CellID] = 							\
			((c3505_clip_mult_step[CellID] & 0x0000001f)<<18) | 	\
			((c3505_clip_ped1_en[CellID]  & 0x00000001)<< 17) | 	\
			((c3505_clip_ped1[CellID] & 0x000000ff)<<9) | 		\
			((c3505_clip_ped2_en[CellID] & 0x00000001)<<8)|		\
			(c3505_clip_ped2[CellID] & 0x000000ff);				\
		//step 1: 
		data = (TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((CLIP_PED_TABLE[CellID]  & 0x7fff)); // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0 >> 8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(CLIP_PED_TABLE[CellID]>>15);
		DatArray[0]=(UINT8)(TavVal_29_15);
		DatArray[1]=(UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);
	}

	// wire sel_AVG_PED		= (TAB_ID==4);
	// AVG_PED_TABLE[TAB_CELL] <= #UDLY CR_PARA_DIN[25:0];
	/*  5) AVG_PED_TABLE, size = 8*26 = 224 bits
	index = {code_rate}, 8 index
	value = {
	AVG_SNR_DELTA,  //3 bits SNR_DELTA to apply AVG_PED
	AVG_MULT_STEP,  //5 bits, step to increase AVG_PED1 and 2 if SNR_DELTA > AVG_SNR_DELTA
	AVG_PED1_EN,	 //1 bit 
	AVG_PED1,				 //8 bits
	AVG_PED2_en,	 //1 bit
	AVG_PED2,				 //8 bits
	}
	*/
	for(CellID = 0; CellID < 8; CellID++)
	{
		// 	 CellID=0;
		TabID = 4;
		AVG_PED_TABLE[CellID] = 							    \
			((c3505_avg_snr_delta[CellID] & 0x00000007)<<23) | 	\
			((c3505_avg_mult_step[CellID] & 0x0000001f)<<18) | 	\
			((c3505_avg_ped1_en[CellID] & 0x00000001)<<17) | 	\
			((c3505_avg_ped1[CellID] & 0x000000ff)<<9) | 		\
			((c3505_avg_ped2_en[CellID] & 0x00000001)<<8) |		\
			(c3505_avg_ped2[CellID] & 0x000000ff);
		//step 1: 
		data=(TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((AVG_PED_TABLE[CellID]	& 0x7fff));  // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(AVG_PED_TABLE[CellID]>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);
	}
 
	/*
	6) S_CODE_RATE_TABLE size = 28*30 = 840 bits
	index = {code_rate}, 28 index
	value = { //30 bits
	S_AWGN_NOISE,			   //10 bits, EST_NOISE (MSBs) level to apply AWGN coefficients
	S_AWGN_COEF_SET,  //3 bit, select one set from the AWGN_COEF table
	S_PON_IRS_DELTA,  //3 bit signed, subtract from AWGN IRS when Pilot On
	S_PON_PRS_DELTA,  //3 bit signed, subtract from AWGN PRS when Pilot On
	S_LOOP_COEF_SET,  //3 bit, select one set from the CLIP_PED table
	S_CLIP_PED_SET,   //3 bit, select one set from the CLIP_PED table
	S_AVG_PED_SET,    //3 bit, select one set from the AVG_PED table
	S_FORCE_OLD_CR,   //1 bit, only affect pilot off
	S_LLR_SHIFT ,			   //1 bit
	} 
	*/
	for(CellID=0; CellID<6; CellID++)
	{
		//CellID=11;
		TabID =  5;
		S_CODE_RATE_TABLE[CellID]=((((c3505_s_awgn_noise[CellID])<<1) & 0x00000ffe)<<20) | // why?
						 ((c3505_s_awgn_coef_set[CellID] & 0x00000007)<<17) | 
						 ((c3505_s_pon_irs_delta[CellID] & 0x00000007)<<14) | 
						 ((c3505_s_pon_prs_delta[CellID] & 0x00000007)<<11) | 
						 ((c3505_s_loop_coef_set[CellID] & 0x00000007)<<8) | 
						 ((c3505_s_clip_ped_set[CellID] & 0x00000007)<<5) | 
						 ((c3505_s_avg_ped_set[CellID] & 0x00000007)<<2) | 
						 ((c3505_s_force_old_cr[CellID] & 0x00000001)<<1) | 
						 (c3505_s_llr_shift[CellID] & 0x00000001);
		//step 1: 
		data=(TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((S_CODE_RATE_TABLE[CellID]	& 0x7fff));  // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(S_CODE_RATE_TABLE[CellID]>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2); 
	}
	 
	 // setting SNR_EST_LEN to 7, so that the value of EST_NOISE can keep stable.
	 nim_reg_read(dev,R113_NEW_CR_ADPT_CTRL,&data,1);
	 data |=0x70;
	 nim_reg_write(dev,R113_NEW_CR_ADPT_CTRL,&data,1);
 
	 ADPT_NEW_CR_PRINTF("NEW CR TAB Initialization Done \n");
 
	 return SUCCESS;
 }

/*****************************************************************************
* INT32 nim_C3505_cr_new_adaptive_unlock_monitor(struct nim_device *dev)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_new_adaptive_unlock_monitor(struct nim_device *dev)
{// this function is to lock when strong noise

	UINT8 rdata[2] = {0},data = 0;
	UINT16 est_noise = 0; // here must be test again
	UINT16 rdata16 = 0;
	UINT8 fec_lock_st = 0x00;
	static UINT8 last_fec_lock_st = 0x00;
	UINT8 cr_adpt_choice = 0;
	UINT8 cr_para_en = 0;
	static UINT8 cr_para_en_flag = 0;

	nim_reg_read(dev,R13b_EST_NOISE,rdata,2);
	rdata16 = rdata[1];
	est_noise = (rdata[0] | (rdata16 << 8)) >> 4; // here must be test again

	nim_reg_read(dev,R04_STATUS,&fec_lock_st,1);
	fec_lock_st &=0x3f;// complicable with DVBS

	nim_reg_read(dev,0x13a,&cr_adpt_choice,1);
	cr_adpt_choice = cr_adpt_choice >>4;
	cr_adpt_choice &= 0x03;// when using OldAdpt, dont need to change coefficients
	nim_reg_read(dev,0x113,&cr_para_en,1);
	cr_para_en &=0x02;// Only for new adpt

	if((est_noise == 0x1ff) && (fec_lock_st != 0x3f) && (cr_adpt_choice != 0x02) && ((cr_para_en == 0x02) | cr_para_en_flag))
	{
		// add by hongyu for CR LOCK Based on NEW ADPT Begin
		nim_reg_read(dev,RB5_CR_PRS_TRA,&data,1);//prs
		data &=0x0f;
		//data |=0x70;
#ifdef ASKEY_PATCH1
		if (g_askey_patch_1_enable)
		{
			data |= 0x60;
		}
		else
		{
			data |=0x70;
		}
#else
		data |=0x70;
#endif
		nim_reg_write(dev,RB5_CR_PRS_TRA,&data,1);
		nim_reg_read(dev,0x37,&data,1);//irs
		data &=0x83;
		data |= 0x40;   //data |= 0x48; 
		nim_reg_write(dev,0x37,&data,1);
		nim_reg_read(dev,RE0_PPLL_CTRL,&data,1);//sbh
		data |= 0x20;// enable symbol before head
		data &= 0xf7;// disable force_old_cr
		nim_reg_write(dev,RE0_PPLL_CTRL,&data,1);
		nim_reg_read(dev,0x127,&data,1);//hpg
		data &= 0x8f;
		nim_reg_write(dev,0x127,&data,1);
		nim_reg_read(dev,0x137,&data,1);//clip_ped
		data &= 0xee;
		nim_reg_write(dev,0x137,&data,1);
		// add by hongyu for CR LOCK Based on NEW ADPT end

		//data = 0x20;					 
		//nim_reg_write(dev,0x13a,&data,1);

		nim_reg_read(dev,0x113,&data,1);
		data &=0xfd;
		nim_reg_write(dev,0x113,&data,1);
		cr_para_en_flag=1;
		//comm_sleep(1000); // waiting for CR LOCK 
		ADPT_NEW_CR_PRINTF("strong phase noise happened and try register parameters\n");
	}
	// ADPT_NEW_CR_PRINTF("last_fec_lock_st is %x\nfec_lock_st is %x\ncr_adpt_choice is %x\ncr_para_en_flag is %x\n ");
	if((last_fec_lock_st != 0x3f) && (fec_lock_st == 0x3f)  && cr_para_en_flag)
	{
		nim_reg_read(dev,0x113,&data,1);
		data |=0x02;
		nim_reg_write(dev,0x113,&data,1); 
		ADPT_NEW_CR_PRINTF("change to NEW ADPT\n");
		cr_para_en_flag=0;
	}
	last_fec_lock_st = fec_lock_st;

	return  SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_cr_new_modcod_table_init(struct nim_device *dev,UINT32 sym)
*	
*
* Arguments:
*	 Parameter1: struct nim_device *dev
*	 Parameter2: UINT32 sym
* 
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_new_modcod_table_init(struct nim_device *dev,UINT32 sym)
{
	/*
	To Re-initial S2_CODE_RATE_table according to symbol rate
	*/

	UINT8 TabID,CellID;
	UINT16 TabVal_14_0,TavVal_29_15;//,temp;
	UINT8 data;
	UINT8 DatArray[2];
	UINT32 AWGN_COEF_TABLE[8];

	UINT16 S2_AWGN_NOISE_tmp;
	UINT8 S2_AWGN_COEF_SET_tmp;
	UINT8 S2_PON_IRS_DELTA_tmp;
	UINT8 S2_PON_PRS_DELTA_tmp;
	UINT8 S2_LOOP_COEF_SET_tmp;
	UINT8 S2_CLIP_PED_SET_tmp;
	UINT8 S2_AVG_PED_SET_tmp;
	UINT8 S2_FORCE_OLD_CR_tmp;

#ifdef ASKEY_PATCH2
	UINT32 S_CODE_RATE_TABLE;
	UINT16 S_AWGN_NOISE_tmp;
	UINT8	S_AWGN_COEF_SET_tmp;
	UINT8	S_PON_IRS_DELTA_tmp;
	UINT8	S_PON_PRS_DELTA_tmp;
	UINT8	S_LOOP_COEF_SET_tmp;
	UINT8	S_CLIP_PED_SET_tmp;
	UINT8	S_AVG_PED_SET_tmp;
	UINT8	S_FORCE_OLD_CR_tmp;
#endif


	UINT8 IRS_tmp;
	UINT8 IRS_FRACTION_tmp;
	UINT8 PRS_tmp;
	UINT8 PRS_FRACTION_tmp;
	UINT8 HEAD_PED_GAIN_tmp;
	UINT8 SYMBOL_BEFORE_HEAD_tmp;

	UINT32 S2_CODE_RATE_TABLE[28];

	ADPT_NEW_CR_PRINTF("Re-inital S2_CODE_RATE_table Begin\n");
	
	for(CellID = 0; CellID < 28; CellID++)
	{
		TabID =  0;
        // For 43.2M symbol TP performance improve
		//if((6500 < sym) && (sym < 33000))
		if(6500 < sym)
		{
			S2_AWGN_NOISE_tmp = c3505_s2_awgn_noise_extra[CellID];
			S2_PON_IRS_DELTA_tmp = c3505_s2_pon_irs_delta_extra[CellID];
			S2_PON_PRS_DELTA_tmp = c3505_s2_pon_prs_delta_extra[CellID];
			S2_CLIP_PED_SET_tmp = c3505_s2_clip_ped_set_extra[CellID];
			S2_AVG_PED_SET_tmp = c3505_s2_avg_ped_set_extra[CellID];
			S2_AWGN_COEF_SET_tmp = c3505_s2_awgn_coef_set_extra[CellID];
			//S2_LOOP_COEF_SET_tmp = c3505_s2_loop_coef_set[CellID];	// By paladin 02/14/2017
			S2_LOOP_COEF_SET_tmp = c3505_s2_loop_coef_extra[CellID];
			S2_FORCE_OLD_CR_tmp = c3505_s2_force_old_cr[CellID];
			if((CellID == 13) && (sym < 16000)) // patch for 8psk R3_4 using 5M parameters
			{
				S2_LOOP_COEF_SET_tmp = 0x0;// LOOP_COEF_table dont EXTRA
				S2_AWGN_COEF_SET_tmp = 0x7;
				S2_FORCE_OLD_CR_tmp = 0x1;
			}
			if((CellID == 11) && (sym < 16000)) //pathc for 8psk R3_5
			{
				S2_AWGN_COEF_SET_tmp = 0x6;//
				S2_AVG_PED_SET_tmp = 0x6;
				S2_FORCE_OLD_CR_tmp = 0x1;
				S2_PON_PRS_DELTA_tmp = 0;
				S2_PON_IRS_DELTA_tmp = 0;
			}
			if((CellID == 12) && (sym < 16000))// patch for 8psk R2_3
			{
				S2_AWGN_COEF_SET_tmp = 0x6;
				//S2_AWGN_NOISE_tmp=0x8c;//0xa0
				S2_LOOP_COEF_SET_tmp = 0x5;
				S2_FORCE_OLD_CR_tmp = 0x1;
			}
			if((CellID == 14) && (sym < 16000))// for 8psk R5_6
			{
			#ifdef ASKEY_PATCH1

				if (g_askey_patch_1_enable)
				{
					S2_AWGN_NOISE_tmp	 = 0x1ff; // For default set prs to 5 to lock the worst signal on 3700/V/15000
					S2_AWGN_COEF_SET_tmp = 0x7;
				    S2_PON_IRS_DELTA_tmp = 0x1;
				    S2_PON_PRS_DELTA_tmp = 0x1;
				    S2_LOOP_COEF_SET_tmp = 0x2;
					S2_CLIP_PED_SET_tmp	= 0x0;
					S2_AVG_PED_SET_tmp	= 0x6;
					S2_FORCE_OLD_CR_tmp	= 0x0;
				}
			#else
				S2_AWGN_COEF_SET_tmp = 0x7;
				S2_LOOP_COEF_SET_tmp = 0x6;
			#endif
				//S2_AWGN_COEF_SET_tmp = 0x7;
				//S2_LOOP_COEF_SET_tmp = 0x6;
			}
			if((CellID == 3) && (sym < 12000))// for qpsk R1_2
			{
				// S2_FORCE_OLD_CR_tmp=0x1;
				S2_AWGN_COEF_SET_tmp = 0x1;
				S2_PON_PRS_DELTA_tmp = 0x7;// in order to not infulence pilot on
				S2_PON_IRS_DELTA_tmp = 0x7;
				S2_AWGN_NOISE_tmp = 0x1c9;// only for 6.5M to 16M
			}

#ifdef _INDIA_VDL_8PSK_R35_IMPULSE_
            if((CellID == 11) && (sym > 20000)) //pathc for 8psk R3_5
            {
                S2_AWGN_COEF_SET_tmp = 0x1;
			    S2_PON_IRS_DELTA_tmp = 0x0;
			    S2_PON_PRS_DELTA_tmp = 0x0;
                S2_LOOP_COEF_SET_tmp = 0x2;
			    S2_CLIP_PED_SET_tmp  = 0x0;
			    S2_AVG_PED_SET_tmp   = 0x7;
			    S2_FORCE_OLD_CR_tmp  = 0x0;
            }
#endif
#ifdef _INDIA_VDL_8PSK_R34_IMPULSE_
            if((CellID == 13) && (sym > 20000)) // patch for 8psk R3_4
            {
                S2_AWGN_COEF_SET_tmp = 0x7;
			    S2_PON_IRS_DELTA_tmp = 0x0;
			    S2_PON_PRS_DELTA_tmp = 0x0;
                S2_LOOP_COEF_SET_tmp = 0x2;
			    S2_CLIP_PED_SET_tmp  = 0x1;
			    S2_AVG_PED_SET_tmp   = 0x7;
			    S2_FORCE_OLD_CR_tmp  = 0x0;
            }

            //8psk R5_6 use S2_AWGN_COEF_SET_tmp=0x7 when sym<16000, 
            //but i change the table for patch 8psk R3_4 to be S2_AWGN_COEF_SET_tmp=0x7,so i have to change 8psk R5_6 table)
            if((CellID == 14) && (sym > 20000)) // for 8psk R5_6
            {
                S2_AWGN_COEF_SET_tmp = c3505_s2_awgn_coef_set_extra[CellID];
                S2_PON_IRS_DELTA_tmp = c3505_s2_pon_irs_delta_extra[CellID];
			    S2_PON_PRS_DELTA_tmp = c3505_s2_pon_prs_delta_extra[CellID];
                S2_LOOP_COEF_SET_tmp = c3505_s2_loop_coef_extra[CellID]    ;
			    S2_CLIP_PED_SET_tmp  = c3505_s2_clip_ped_set_extra[CellID] ;
			    S2_AVG_PED_SET_tmp   = c3505_s2_avg_ped_set_extra[CellID]  ;
			    S2_FORCE_OLD_CR_tmp  = c3505_s2_force_old_cr[CellID]       ;
            }
#endif
		}
		else
		{
			 S2_AWGN_NOISE_tmp = c3505_s2_awgn_noise[CellID];
			 S2_AWGN_COEF_SET_tmp = c3505_s2_awgn_coef_set[CellID];
			 S2_LOOP_COEF_SET_tmp = c3505_s2_loop_coef_set[CellID];
			 S2_CLIP_PED_SET_tmp = c3505_s2_clip_ped_set[CellID];
			 S2_AVG_PED_SET_tmp= c3505_s2_avg_ped_set[CellID];
			 S2_PON_IRS_DELTA_tmp = c3505_s2_pon_irs_delta[CellID];
			 S2_PON_PRS_DELTA_tmp = c3505_s2_pon_prs_delta[CellID];
			 S2_FORCE_OLD_CR_tmp = c3505_s2_force_old_cr[CellID];
			 if((CellID == 11) && (sym < 4000))// patch for 8psk R3_5 symbol_rate<4000
			 {
				 S2_PON_IRS_DELTA_tmp=0x6;
				 S2_PON_PRS_DELTA_tmp=0x6;
				 ADPT_NEW_CR_PRINTF("below 4000\n");
			 } 
			 if((CellID == 3) && (sym <= 6500))// for qpsk R1_2
			 {
				 S2_AWGN_NOISE_tmp = 0x1ed;
				 S2_AWGN_COEF_SET_tmp = 0x1;
			 }	 
		}
		if(CellID == 11)
		{
			ADPT_NEW_CR_PRINTF("S2_PON_IRS_DELTA_tmp is %d\n",S2_PON_IRS_DELTA_tmp);
		}
		S2_CODE_RATE_TABLE[CellID]=((((S2_AWGN_NOISE_tmp)<<1) & 0x00000ffe)<<20) | // why?
						 ((S2_AWGN_COEF_SET_tmp & 0x00000007)<<17) | 
						 ((S2_PON_IRS_DELTA_tmp & 0x00000007)<<14) | 
						 ((S2_PON_PRS_DELTA_tmp & 0x00000007)<<11) | 
						 ((S2_LOOP_COEF_SET_tmp & 0x00000007)<<8) | 
						 ((S2_CLIP_PED_SET_tmp & 0x00000007)<<5) | 
						 ((S2_AVG_PED_SET_tmp & 0x00000007)<<2) | 
						 ((S2_FORCE_OLD_CR_tmp & 0x00000001)<<1) | 
						 (c3505_s2_llr_shift[CellID] & 0x00000001);
		//step 1: 
		data = (TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );
		//step 2:
		TabVal_14_0 = (UINT16)((S2_CODE_RATE_TABLE[CellID]  & 0x7fff)); // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(S2_CODE_RATE_TABLE[CellID]>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2); 
	}

	for(CellID = 0; CellID < 8; CellID++)
	{
		TabID =  1;
        // For 43.2M symbol TP performance improve
		//if((6500 < sym) && (sym < 33000))
		if(6500 < sym)
		{

			IRS_tmp = c3505_irs_extra[CellID];
			IRS_FRACTION_tmp = c3505_irs_fraction_extra[CellID];
			PRS_tmp = c3505_prs_extra[CellID];
			PRS_FRACTION_tmp = c3505_prs_fraction_extra[CellID];
			HEAD_PED_GAIN_tmp = c3505_head_ped_gain_extra[CellID];
			SYMBOL_BEFORE_HEAD_tmp = c3505_symbol_before_head_extra[CellID];
#ifdef _INDIA_VDL_8PSK_R34_IMPULSE_
            if((CellID==7) && (sym > 20000))
            {
                IRS_tmp                = 0x10;
			    IRS_FRACTION_tmp       = 0x0;
			    PRS_tmp                = 0x9;
			    PRS_FRACTION_tmp       = 0x0;
			    HEAD_PED_GAIN_tmp      = 0x0;
			    SYMBOL_BEFORE_HEAD_tmp = 0x0;
            }
#endif
			ADPT_NEW_CR_PRINTF("6500 < sym < 33000\n");
		}
		else
		{
			IRS_tmp = c3505_irs[CellID];
			IRS_FRACTION_tmp = c3505_irs_fraction[CellID];
			PRS_tmp = c3505_prs[CellID];
			PRS_FRACTION_tmp = c3505_prs_fraction[CellID];
			HEAD_PED_GAIN_tmp = c3505_head_ped_gain[CellID];
			SYMBOL_BEFORE_HEAD_tmp = c3505_symbol_before_head[CellID];
#ifdef ASKEY_PATCH2
			if(CellID==5)   //use cellid=5, this cell has not been use by other modcod
			{
			   IRS_tmp				  = 0xf;
			   IRS_FRACTION_tmp 	  = 0;
			   PRS_tmp				  = 6;
			   PRS_FRACTION_tmp 	  = 0;
			   HEAD_PED_GAIN_tmp	  = 0;
			   SYMBOL_BEFORE_HEAD_tmp = 0;
			}
#endif
			ADPT_NEW_CR_PRINTF("sym<6500 or sym>33000\n");
		}
		AWGN_COEF_TABLE[CellID]=((IRS_tmp & 0x0000001f)<<12)  | 
						 ((IRS_FRACTION_tmp & 0x00000003)<<10) | 
						 ((PRS_tmp & 0x0000000f)<<6) | 
						 ((PRS_FRACTION_tmp & 0x00000003)<<4) | 
						 ((HEAD_PED_GAIN_tmp & 0x00000007)<<1) |
						 (SYMBOL_BEFORE_HEAD_tmp & 0x00000001);
		//step 1: 
		data=(TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((AWGN_COEF_TABLE[CellID]  & 0x7fff));	 // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(AWGN_COEF_TABLE[CellID]>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;  // enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);

	}
	
#ifdef ASKEY_PATCH2
	for(CellID=2; CellID<6; CellID++) //dvbs 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:6/7(null), 5:7/8
	{
		TabID =  5;
		if(6500 < sym) //use original value for big symbolrate
		{  
			S_AWGN_NOISE_tmp	= c3505_s_awgn_noise[CellID];
			S_AWGN_COEF_SET_tmp = c3505_s_awgn_coef_set[CellID];  //use the fifth table
			S_PON_IRS_DELTA_tmp = c3505_s_pon_irs_delta[CellID];
			S_PON_PRS_DELTA_tmp = c3505_s_pon_prs_delta[CellID];
			S_LOOP_COEF_SET_tmp = c3505_s_loop_coef_set[CellID];
			S_CLIP_PED_SET_tmp	= c3505_s_clip_ped_set[CellID]; //0x0:use clip_ped,  0x2: don't use clip_ped
			S_AVG_PED_SET_tmp	= c3505_s_avg_ped_set[CellID]; //0x5:use avg_ped,  0x6:don't use avg_ped
			S_FORCE_OLD_CR_tmp	= c3505_s_force_old_cr[CellID];
		}
		else
		{
			S_AWGN_NOISE_tmp	= 0x0;	//0: never chang parameters by noise
			S_AWGN_COEF_SET_tmp = 0x5;	//use the fifth table
			S_PON_IRS_DELTA_tmp = 0x0;
			S_PON_PRS_DELTA_tmp = 0x0;
			S_LOOP_COEF_SET_tmp = 0x4;
			S_CLIP_PED_SET_tmp	= 0x2; //0x0:use clip_ped,	0x2: don't use clip_ped
			S_AVG_PED_SET_tmp	= 0x6; //0x5:use avg_ped,  0x6:don't use avg_ped
			S_FORCE_OLD_CR_tmp	= 0x0;
		}	
			S_CODE_RATE_TABLE=(((S_AWGN_NOISE_tmp<<1) & 0x00000ffe)<<20) | // why?
		 ((S_AWGN_COEF_SET_tmp & 0x00000007)<<17) | 
		 ((S_PON_IRS_DELTA_tmp & 0x00000007)<<14) | 
		 ((S_PON_PRS_DELTA_tmp & 0x00000007)<<11) | 
		 ((S_LOOP_COEF_SET_tmp & 0x00000007)<<8) | 
		 ((S_CLIP_PED_SET_tmp  & 0x00000007)<<5) | 
		 ((S_AVG_PED_SET_tmp   & 0x00000007)<<2) | 
		 ((S_FORCE_OLD_CR_tmp  & 0x00000001)<<1) | 
		 (0 & 0x00000001);
		

		//step 1: 
		data=(TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );

		//step 2:
		TabVal_14_0 = (UINT16)((S_CODE_RATE_TABLE & 0x7fff));  // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(S_CODE_RATE_TABLE>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;	// enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2); 
	}
	
#endif

	ADPT_NEW_CR_PRINTF("Re-inital S2_CODE_RATE_table Done\n");
	return SUCCESS;

}
 
 /*****************************************************************************
 *  INT32 nim_C3505_cr_adaptive_method_choice(struct nim_device *dev)
 * 
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 choice_type
 *
 * Return Value: INT32
 *****************************************************************************/
  INT32 nim_c3505_cr_adaptive_method_choice(struct nim_device *dev, UINT8 choice_type)
 {
 /******************************************************************************
	 * choice_type = 0: only choice NEW CR ADPT method
	 * choice_type = 1: only choice OLD CR ADPT method
	 * choice_type = 2: part choice NEW CR ADPT and part choice OLD ADPT method
	 * choice_type = 3: using the value of regfile
 ******************************************************************************/
 /*
			 ADPT_CR_METHOD_CHOICE_2_ALL=0	 reg_cr13a[4]=0
			 ADPT_CR_METHOD_CHOICE_1_ALL=0	 reg_cr13a[5]=0
			 ADPT_CR_PARA_EN1 = 1		 reg_cr113[1]=1  
 */
	 UINT8 rdata;
	 switch(choice_type)// only choice NEW CR ADPT method
	 {
	 case 0:
		 nim_reg_read(dev,0x13a, &rdata,1);
		 rdata |= 0x10;
		 nim_reg_write(dev,0x13a, &rdata,1);
		 nim_reg_read(dev,0x113, &rdata,1);
		 rdata |= 0x02;
		 nim_reg_write(dev,0x113, &rdata,1);
		 break;
	 case 1: //only choice OLD CR ADPT method
		 nim_reg_read(dev,0x13a, &rdata,1);
		 rdata |= 0x20;
		 rdata &= 0xef;
		 nim_reg_write(dev,0x13a, &rdata,1);
		 nim_reg_read(dev,0x12e,&rdata,1);// ap1e=0 ap2e=0
		 rdata &=0xcf;
		 nim_reg_write(dev,0x12e,&rdata,1);
		 nim_reg_read(dev,0x137,&rdata,1);//cp1e=0 cp2e=0
		 rdata &=0xee;
		 nim_reg_write(dev,0x137,&rdata,1);
		 nim_reg_read(dev,0x113, &rdata,1);
		 rdata |= 0x02;
		 nim_reg_write(dev,0x113, &rdata,1);
		 break;
	 case 2:// if you want to use this circumstance, you need to edit again
		 nim_reg_read(dev,0x13a, &rdata,1);
		 rdata &= 0xcf;
		 nim_reg_write(dev,0x13a, &rdata,1);
		 nim_reg_read(dev,0x113, &rdata,1);
		 rdata |= 0x02;
		 nim_reg_write(dev,0x113, &rdata,1);
		 // you need edit begin here
		 //
		 //ADPT_CR_METHOD_CHOICE[i] = 1 or 0
		 //
		 break;
	 case 3://using the value of regfile
		 nim_reg_read(dev,0x13a, &rdata,1);
		 rdata |= 0x10;
		 nim_reg_write(dev,0x13a, &rdata,1);
		 nim_reg_read(dev,0x113, &rdata,1);
		 rdata &= 0xfd;
		 nim_reg_write(dev,0x113, &rdata,1);
		 break;
	 default:// choice NEW CR ADPT method
		 nim_reg_read(dev,0x13a, &rdata,1);
		 rdata |= 0x10;
		 nim_reg_write(dev,0x13a, &rdata,1);
		 nim_reg_read(dev,0x113, &rdata,1);
		 rdata |= 0x02;
		 nim_reg_write(dev,0x113, &rdata,1);
		 break;
	 }
	 return SUCCESS;
 }

 /*****************************************************************************
 * INT32 nim_C3505_NframeStepTso_setting(struct nim_device *dev,UINT32 *sym_rate,UINT8 s_Case)
 *	
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT32 *sym_rate
 *	Parameter3: UINT8 s_Case
 *
 * Return Value: INT32
 *****************************************************************************/
  INT32 nim_c3505_nframe_step_tso_setting(struct nim_device *dev, UINT32 sym_rate, UINT8 s_Case)
 {
	UINT8 rdata[2];
	UINT8 tso_st = 0;
	UINT8 current_lock_st = 0x00;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_reg_read(dev,R04_STATUS,&current_lock_st,1);
	current_lock_st &= 0x3f;

	nim_reg_read(dev,RFA_RESET_CTRL+2,&tso_st,1);
	if(((current_lock_st != 0x3f) && (s_Case==0x00)) | (s_Case==0x01))
	{	 // s_Case==0x00 called by task function
		// s_Case==0x01 called by channnel change function
		nim_reg_read(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		rdata[0] &= 0x88;
		rdata[0] |= 0x21;
		rdata[1] &= 0x88;
		rdata[1] |= 0x32;
		nim_reg_write(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		nim_reg_read(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		// NIM_PRINTF("nim_C3505_NframeStepTso_setting:\n\tnframe is %x\n\tstep is %x\nTSO is OFF\n",rdata[0],rdata[1]);
		nim_c3505_tso_off(dev);

		// tso off_on flag set, namely crfc[3] = 1
		//nim_reg_read(dev,0xfc,&tso_st,1);
		tso_st = tso_st | 0x08; //
		nim_reg_write(dev,RFA_RESET_CTRL+2,&tso_st,1);
	}
	
	if((current_lock_st==0x3f) && ((tso_st & 0x08)==0x08) && (priv->ul_status.c3505_lock_adaptive_done))
	{
		nim_reg_read(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		//ADPT_NEW_CR_PRINTF("all state lock and change nframe/step:\n\tnframe is %x\n\tstep is %x\n",rdata[0],rdata[1]);
		rdata[0] &= 0x88;
		rdata[0] |= 0x74;
		rdata[1] &= 0x88;
		rdata[1] |= 0x70;
		nim_reg_write(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		nim_reg_read(dev,R124_HEAD_DIFF_NFRAME,rdata,2);
		ADPT_NEW_CR_PRINTF("nim_C3505_NframeStepTso_setting:\n\tnframe is %x\n\tstep is %x\nTSO is ON\n",rdata[0],rdata[1]);
		nim_c3505_tso_on(dev);
		NIM_PRINTF("\n nim_C3505_tso_on !\n");
		NIM_PRINTF("line=%d, in %s\n", __LINE__, __FUNCTION__);
		//tso off_on flag reset, namely crfc[3] = 0
		tso_st = tso_st & 0xf7;
		nim_reg_write(dev,RFA_RESET_CTRL+2,&tso_st,1);
	}
	 return SUCCESS;
 }
 
 /*****************************************************************************
 *  INT32 nim_C3505_fec_set_ldpc(struct nim_device *dev, UINT8 s_Case, UINT8 c_ldpc, UINT8 c_fec)
 * Get bit error ratio
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT16* RsUbc
 *
 * Return Value: INT32
 *****************************************************************************/
 INT32 nim_c3505_fec_set_ldpc(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec)
 {
	 UINT8 data;
	 // LDPC parameter
	 switch (s_case)
	 {
		case NIM_OPTR_CHL_CHANGE:
			 data = 0x00; // disactive ldpc avg iter and ldpc_max_iter_num[9:8]=0x3
			 nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
		 break;
		 case NIM_OPTR_SOFT_SEARCH:
			 data = 0x00; 
			 nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
		 break;
		 default:
			 NIM_PRINTF(" CMD for nim_C3505_fec_set_ldpc ERROR!!!");
		 break;
	 }
	 return SUCCESS;
 }
 

/*****************************************************************************
*  INT32 nim_c3505_tso_initial (struct nim_device *dev)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode)
{
	UINT8 data;

	// paladin.ye 2015-11-12	
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic	
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	data = 0x42;    
#else        
	data = 0x40;
#endif

	//data = 0x02;//C3505 bit4 remove
	nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
	NIM_PRINTF("            nim C3505 insert dummy for dvbs \n");
	switch(tso_mode)
	{
		case 0 : 
		{
			data = 0x37;// 1bit
			nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
			NIM_PRINTF("            tso_mode is SSI \n"); 
		}
		break;
		case 1 : 
		{
			data = 0x27;//8bit
			nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
			NIM_PRINTF("            tso_mode is SPI \n"); 
		}
		break;
	}
	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_tso_off (struct nim_device *dev)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_tso_off (struct nim_device *dev)
{
	UINT8 data;
	// close fec ts output
	nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
	data = data | 0x10;
	nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
	//NIM_PRINTF("            fec ts off\n");
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_tso_on (struct nim_device *dev)
* 	Open TSO output
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_tso_on (struct nim_device *dev)
{
	UINT8 data;
	// Open fec ts output
	nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
	data = data & 0xef;
	nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
	NIM_PRINTF("            fec ts on\n");
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3505_set_tso_clk (struct nim_device *dev, struct nim_tso_cfg *tso_cfg)
* 	Set tso clk
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_tso_cfg *tso_cfg, defined in nim_dev.h
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_tso_clk (struct nim_device *dev, struct nim_tso_cfg *tso_cfg)
{
	struct nim_c3505_private *priv = NULL;
	UINT8 data = 0;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;

    // First save the configuration
    //priv->nim_tso_cfg = *tso_cfg;   // ???
    comm_memcpy(&(priv->nim_tso_cfg), tso_cfg, sizeof(priv->nim_tso_cfg));

    if (tso_cfg->tso_clk_cfg.is_manual_mode)
    {
        nim_reg_read(dev, RFC_TSO_CLK_SW_CFG, &data, 1);
        data &= 0xf8;
        data |= 0x10;
        data |= (tso_cfg->tso_clk_cfg.tso_clk_src&0x07);
        nim_reg_write(dev, RFC_TSO_CLK_SW_CFG, &data, 1);
        
        nim_reg_read(dev, RFC_TSO_CLK_SW_CFG + 1, &data, 1);
        data &= 0x60;
        data |= ((tso_cfg->tso_clk_cfg.clk_div_bypass&0x01) << 7);
        data |= (tso_cfg->tso_clk_cfg.clk_div&0x1f);
        nim_reg_write(dev, RFC_TSO_CLK_SW_CFG + 1, &data, 1);
    }
    else
    {
        nim_reg_read(dev, RFC_TSO_CLK_SW_CFG, &data, 1);
        data = data & 0xef;
        nim_reg_write(dev, RFC_TSO_CLK_SW_CFG, &data, 1);
    }

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_C3505_tso_dummy_off (struct nim_device *dev)
* disable the insertion of dummy TS pscket
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_tso_dummy_off (struct nim_device *dev)
{
	UINT8 data;
	nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
	data = data & 0xfd;
	nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
	NIM_PRINTF("            nim m3503 close dummy for dvbs \n");
	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_tso_dummy_on (struct nim_device *dev)
*  Enable the insertion of dummy TS packet 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_tso_dummy_on (struct nim_device *dev)
{
	UINT8 data;
	nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
	data = data | 0x02;
	nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
	//clear full then send mode of TSO
	nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
	data = data & 0xbf;
	nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
	NIM_PRINTF("%s\n", __FUNCTION__);
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_DiSEqC_initial (struct nim_device *dev)
*
*  defines DiSEqC operations
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8* cmd
*  Parameter4: UINT8 cnt
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_diseqc_initial(struct nim_device *dev)
{
	UINT8 data;
    
	NIM_PRINTF("Diseqc initial \n");
/*	
    // Diseqc time setting
    //4   DISEQC_HV_OEJ
    //3   DISEQC_OUT_OEJ
    //2   DISEQC_COMP_DIN_EN
    //1   DISEQC_ALIGN_EN
    //0   DISEQC_COMP_EN
*/    
	nim_reg_read(dev, R15D_DISEQC_CMD_QUE_CTL, &data, 1);
	data |=0x01; 
	nim_reg_write(dev, R15D_DISEQC_CMD_QUE_CTL, &data, 1);
    data = 0x02;
    nim_reg_write(dev, R114_DISEQC_TIME_SET, &data, 1);
	 data = 0xff;
	nim_reg_write(dev, R8E_DISEQC_TIME + 0x01, &data, 1);    
#ifdef DISEQC_OUT_INVERT
	// invert diseqc out.
	nim_reg_read(dev, R90_DISEQC_CLK_RATIO+6, &data, 1);
	data = (data | 0x80);
	nim_reg_write(dev, R90_DISEQC_CLK_RATIO+6, &data, 1);
#endif
return SUCCESS;

}
/*****************************************************************************
* INT32 nim_C3505_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
*
*  defines DiSEqC operations
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8* cmd
*  Parameter4: UINT8 cnt
*
* Return Value: void
*****************************************************************************/
 INT32 nim_c3505_diseqc_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt)
{
	UINT8 data, temp, rec_22k = 0;
	UINT16 timeout, timer;
	UINT8 i;
    
	NIM_PRINTF("mode = 0x%d\n", mode);

	switch (mode)
	{
	case NIM_DISEQC_MODE_22KOFF:
	case NIM_DISEQC_MODE_22KON:
		nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
		data = ((data & 0xF8) | mode);
		nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
		break;
	case NIM_DISEQC_MODE_BURST0:
		nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        rec_22k = data;
		//tone burst 0
		temp = 0x02;
		data = ((data & 0xF8) | temp);
		nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
		// For combined transmission,wait burst finished then do another thing by paladin 2016/03/08
        //comm_sleep(16); 
        comm_sleep(40); 
        data = (rec_22k & 0xf9); // clear diseqc FSM
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
		break;
	case NIM_DISEQC_MODE_BURST1:
		nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        rec_22k = data;
		//tone bust 1
		temp = 0x03;
		data = ((data & 0xF8) | temp);
		nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
		// For combined transmission,wait burst finished then do another thing by paladin 2016/03/08
        //comm_sleep(16); 
        comm_sleep(40); 
        data = (rec_22k & 0xf9); // clear diseqc FSM
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
		break;
	case NIM_DISEQC_MODE_BYTES:
        comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k=(data & 0x41);      // get DISEQC_22k origianl value
        if(cnt>8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        else
        {
            // close 22K and set TX byte number
            data = ((data & 0xc0) | ((cnt - 1)<<3));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        }
        //write the tx data, max 8 byte
        for (i = 0; i < cnt; i++)
        {
            nim_reg_write(dev, (i + 0x7E), cmd + i, 1);
        }
	 // remove clean interrupt, since reg7d is read only
        //write the control bits, start TX
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);    
        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);
            if ((0 != (data & 0x07))&&(timer>50))
            {
                break;
            }
            comm_sleep(10);
            timer += 10;
        }
        if (1 == (data & 0x07))
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            return  SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            return ERR_FAILED;
        }
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP1:
        comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k=(data & 0x41);      // get DISEQC_22k origianl value
        if(cnt>8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        else
        {
            // close 22K and set TX byte number
            data = ((data & 0xc0) | ((cnt - 1)<<3));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        }

        //write the data
        for (i = 0; i < cnt; i++)
        {
            nim_reg_write(dev, (i + 0x7E), cmd + i, 1);
        }
	 // remove clean interrupt, since reg7d is read only
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP2:
        //TX start : Send byte
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);   
        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);
            if ((0 != (data & 0x07))&&(timer>50))
            {
                break;
            }
            comm_sleep(10);
            timer += 10;
        }
        if (1 == (data & 0x07))
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            
            return SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            
            return ERR_FAILED;
        }
        break;
	case NIM_DISEQC_MODE_ENVELOP_ON:
		{
			nim_reg_read(dev, R24_MATCH_FILTER, &data, 1);
			data |= 0x01;
			nim_reg_write(dev, R24_MATCH_FILTER, &data, 1);
		}
		break;
	case NIM_DISEQC_MODE_ENVELOP_OFF:
		{
			nim_reg_read(dev, R24_MATCH_FILTER, &data, 1);
			data &= 0xFE;
			nim_reg_write(dev, R24_MATCH_FILTER, &data, 1);
		}
		break;
	default :
		break;
	}
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
*   							UINT8 *rt_value, UINT8 *rt_cnt)
*
*  defines DiSEqC 2.X operations
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8* cmd
*  Parameter4: UINT8 cnt
*  Parameter5: UINT8 *rt_value
*  Parameter6: UINT8 *rt_cnt
*
* Return Value: Operation result.
*****************************************************************************/
INT32 nim_c3505_diseqc2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt)
{
    INT32 result;
    UINT8 data, temp;
    UINT16 timeout, timer;
    UINT8 i;
    UINT8 need_reply = 0;// 0:No reply required  1:reply required 

	NIM_PRINTF("\t\t Enter Function nim_c3505_diseqc2x_operate mode = %d\n", mode);
	
    switch (mode)
    {
    case NIM_DISEQC_MODE_BYTES:
        //write the data to send buffer
        if(cnt > 8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        if(cmd[0] == 0xE0 ||cmd[0] == 0xE1)
        	need_reply = 0;
        else
            need_reply = 1;
            
        for (i = 0; i < cnt; i++)
        {
            data = cmd[i];
            NIM_PRINTF("\t\t Diseqc2.X:cmd[%d]:0x%x\n", i,data);
            nim_reg_write(dev, (i + R7C_DISEQC_CTRL + 0x02), &data, 1);
        }

        //set diseqc data counter
        temp = cnt - 1;
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k=(data & 0x41);      // get DISEQC_22k origianl value
        data = ((data & 0x47) | (temp << 3));
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        //enable diseqc interrupt mask event bit.
        nim_reg_read(dev, R03_IMASK, &data, 1);
        data |= 0x80;
        nim_reg_write(dev, R03_IMASK, &data, 1);

        //clear co-responding diseqc interrupt event bit.
        nim_reg_read(dev, R02_IERR, &data, 1);
        data &= 0x7f;
        nim_reg_write(dev, R02_IERR, &data, 1);

		NIM_PRINTF("\t\t Diseqc2.X:need_reply:%d\n", need_reply);
        //write the control bits, need reply
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);

        if(need_reply == 1)
        	temp = 0x84;
        else
        	temp = 0x04;
        	
        data = ((data & 0x78) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1); 

        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt + 200; 
        data = 0;

        //check diseqc interrupt state.
        while (timer < timeout)
        {
            nim_reg_read(dev, R02_IERR, &data, 1);
            if (0x80 == (data & 0x80)) 
            {
				NIM_PRINTF("\t\t DISEQC2.X transaction end\n");
                break;
            }
            comm_sleep(10);
            timer += 1;
        }

        //init value for error happens.
        result = ERR_FAILUE;
        rt_value[0] = DISEQC2X_ERR_NO_REPLY;
        *rt_cnt = 0;
        if (0x80 == (data & 0x80)) 
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);

            switch (data & 0x07)
            {
            case 1:
                *rt_cnt = (UINT8) ((data >> 4) & 0x0f);
                NIM_PRINTF("\t\t DISEQC2.X:rt_cnt :%d\n", *rt_cnt);
				if (*rt_cnt > 0)
                {
                    for (i = 0; i < *rt_cnt; i++)
                    {
                        nim_reg_read(dev, (i + R86_DISEQC_RDATA), (rt_value + i), 1);
						NIM_PRINTF("\t\t DISEQC2.X:RDATA :0x%x\n", rt_value[i]);
                    }
                    result = SUCCESS;
                }

                break;

            case 2:
                rt_value[0] = DISEQC2X_ERR_NO_REPLY;
                NIM_PRINTF("\t\t DISEQC2X_ERR_NO_REPLY\n");
                break;
            case 3:
                rt_value[0] = DISEQC2X_ERR_REPLY_PARITY;
                NIM_PRINTF("\t\t DISEQC2X_ERR_REPLY_PARITY\n");
                break;
            case 4:
                rt_value[0] = DISEQC2X_ERR_REPLY_UNKNOWN;
                NIM_PRINTF("\t\t DISEQC2X_ERR_REPLY_UNKNOWN\n");
                break;
            case 5:
                rt_value[0] = DISEQC2X_ERR_REPLY_BUF_FUL;
                NIM_PRINTF("\t\t DISEQC2X_ERR_REPLY_BUF_FUL\n");
                break;
            default:
                rt_value[0] = DISEQC2X_ERR_NO_REPLY;
                break;
            }
        }

        //set 22k and polarity by origianl value; other-values are not care.
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
	    data = (data&0xb8) | va_22k;
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        return result;

    default :
        break;
    }

	comm_sleep(1000);

	return SUCCESS;
}

/*****************************************************************************
* void nim_C3505_fec_set_demap_noise(struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: void
*****************************************************************************/
/*
 INT32 nim_C3505_fec_set_demap_noise(struct nim_device *dev)
{
	UINT8 data, noise_index;
	UINT16 est_noise;

	// activate noise
	nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
	data &= 0xfc;
	nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);

	// set noise_index  //crd1 RO register, can't write
	noise_index = 0x0c; // 8psk,3/5.
	nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);

	// set noise
	est_noise = DEMAP_NOISE[noise_index];
	data = est_noise & 0xff;
	nim_reg_write(dev, RD0_DEMAP_NOISE_RPT, &data, 1);
	data = (est_noise >> 8) & 0x3f;
	data |= 0xc0;
	nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);

       return SUCCESS;
}
*/
/*****************************************************************************
*  INT32 nim_C3505_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *ci_plus_flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_c3505_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag)
{
	UINT8 data;

	// For CI plus test.
	data = 0x02;	// symbol period from reg, 2 cycle
	nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
	NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

	nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
	data = data | 0x80;    // enable symbol period from reg
	nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

	nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
	data = data | 0xe0;
	nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

	nim_reg_read(dev, RDF_TS_OUT_DVBS2, &data, 1);
	data = (data & 0xfc) | 0x01;
	nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

	*ci_plus_flag = 1;

	return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_C3505_set_12v(struct nim_device *dev, UINT8 flag)
* Description: C3505 set LNB votage 12V enable or not
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_c3505_set_12v(struct nim_device *dev, UINT8 flag)
{
	return SUCCESS;
}

/*****************************************************************************
* void nim_C3505_FFT_set_para(struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_fft_set_para(struct nim_device *dev)
{
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_get_bypass_buffer(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_bypass_buffer(struct nim_device *dev)
{
	// According to DMX IC spec, bypass buffer must locate in 8MB memory segment.
	// Bypass buffer size is (BYPASS_BUFFER_REG_SIZE * 188)

	UINT32 tmp;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	priv->ul_status.adc_data_malloc_addr = (UINT8 *) comm_malloc(BYPASS_BUF_SIZE * 2);
	if (priv->ul_status.adc_data_malloc_addr == NULL)
		return ERR_NO_MEM;

	NIM_PRINTF("ADCdata_malloc_addr=0x%08x\n", priv->ul_status.adc_data_malloc_addr);

	comm_memset((int *)priv->ul_status.adc_data_malloc_addr, 0, BYPASS_BUF_SIZE * 2);
	tmp = ((UINT32) (priv->ul_status.adc_data_malloc_addr)) & BYPASS_BUF_MASK;
	if (tmp)
		priv->ul_status.adc_data = priv->ul_status.adc_data_malloc_addr + BYPASS_BUF_SIZE - tmp;
	else
		priv->ul_status.adc_data = priv->ul_status.adc_data_malloc_addr;
	
	NIM_PRINTF("ADCdata=0x%08x\n", priv->ul_status.adc_data);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_tr_setting(struct nim_device *dev, UINT8 s_Case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_tr_setting(struct nim_device *dev, UINT8 s_case)
{
	return SUCCESS;
}

INT32 nim_c3505_task_tso_setting(struct nim_device * dev,UINT32 sym_rate,UINT8 s_case)
{
	UINT8 tso_st = 0;
	UINT8 current_lock_st = 0x00;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_reg_read(dev,R04_STATUS,&current_lock_st,1);
	current_lock_st &=0x3f;

	nim_reg_read(dev,RFA_RESET_CTRL+2,&tso_st,1);

	if(((current_lock_st != 0x3f) && (s_case == 0x00)) | (s_case == 0x01))
	{   
		// s_Case==0x00 called by task function
		// s_Case==0x01 called by channnel change function
		nim_c3505_tso_off(dev);

		// tso off_on flag set, namely crfc[3] = 1
		//nim_reg_read(dev,0xfc,&tso_st,1);
		tso_st = tso_st | 0x08; //
		nim_reg_write(dev,RFA_RESET_CTRL+2,&tso_st,1);
	}
	if((current_lock_st == 0x3f) && ((tso_st & 0x08) == 0x08) && (priv->ul_status.c3505_lock_adaptive_done))
	{
		nim_c3505_tso_on(dev);
		//tso off_on flag reset, namely crfc[3] = 0
		tso_st = tso_st & 0xf7;
		nim_reg_write(dev,RFA_RESET_CTRL+2,&tso_st,1);
	}

	return SUCCESS;
}

 /*****************************************************************************
*  void nim_c3505_set_map_beta(struct nim_device *dev, UINT8 index, UINT8 frame_mode)
* set demap beta
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_set_map_beta(struct nim_device *dev, UINT8 index, UINT8 frame_mode)
{
	UINT8 data;
    
	if(frame_mode == 0)//short
	{
		data = index;//index
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);
		data = c3505_map_beta_active_buf_short[index];//act enable
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);
		data = 0x08;
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
		data = c3505_map_beta_buf_short[index];//data
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);
		data = 0x07;
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
	}
	else if(frame_mode == 1)//normal
	{
		data = index;//index                                        			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);          			
		data = c3505_map_beta_active_buf_normal[index];//act enable             
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);               			
		data = 0x04;                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			
		data = c3505_map_beta_buf_normal[index];//data                                
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);                 			
		data = 0x03;                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			
	}
    
    return SUCCESS;
}
/*****************************************************************************
*  void nim_c3505_set_demap_noise(struct nim_device *dev,UINT8 index, UINT8 frame_mode)
* set demap noise
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_set_demap_noise(struct nim_device *dev,UINT8 index, UINT8 frame_mode)
{
  UINT8 data, noise_index;
  UINT16 est_noise;
  
  if(frame_mode == 0)     //short
	{
		nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
		data &= 0xfb;//ES_ACT_BP disable
		data |= 0x80;
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
		noise_index = index; //index                                   
		noise_index &= 0x7f;                                           
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);  
		est_noise = c3505_demap_noise_short[noise_index]; //data                                      
		data = est_noise & 0xff;                                       
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT, &data, 1);             
		data = (est_noise >> 8) & 0x3f;                                
		data |= 0xc0;                                                  
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);	       
	}
	else if(frame_mode == 1)//normal
	{
		nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
		data &= 0x7e;//ES_ACT_BP disable
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
		noise_index = index; //index                                   		
		noise_index &= 0x7f;                                           		
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);  
		est_noise = c3505_demap_noise_normal[noise_index]; //data                                     
		data = est_noise & 0xff;                                       
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT, &data, 1);             
		data = (est_noise >> 8) & 0x3f;                                
		data |= 0xc0;                                                  
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);	       
	}

    return SUCCESS;
}
/*****************************************************************************
*  INT32 nim_s3503_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode)
* set dvbs2 work mode : ACM,CCM, AUTO
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 sub_work_mode
*
* Return Value: UINT8
*****************************************************************************/
INT32 nim_c3505_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode)
{
	UINT8 data;

	nim_reg_read(dev,R5B_ACQ_WORK_MODE,&data,1);
	data &= 0xf3; 
	data |= (sub_work_mode<<2); 
	nim_reg_write(dev,R5B_ACQ_WORK_MODE,&data,1);

	return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_c3505_ACM_set(struct nim_device *dev, UINT8 s_Case)
* set dvbs2 work mode : ACM,CCM, AUTO
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*
* Return Value: UINT8
*****************************************************************************/ 
INT32 nim_c3505_acm_set(struct nim_device *dev, UINT8 s_case)
{
	UINT8 data = 0;
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	switch (s_case)
	{	    
		case NIM_OPTR_ACM:
			/*-------------------------------*\
			CR5B	[3:2]: ACQ_SUB_MODE:
			0: ACM/VCM mode
			1: CCM mode
			2: Reserved
			3: Auto mode
			\*-------------------------------*/	
			data =0x53; 
			nim_reg_write(dev,R5B_ACQ_WORK_MODE,&data,1);
			//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
			nim_reg_read(dev, R10b_ISI_SET, &data, 1);
            
            if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
            {
			    data |= 0x09;
            }
            else
            {
                data |= 0x08;
            }
            
			nim_reg_write(dev, R10b_ISI_SET, &data, 1);	//set 10b[3] = 1
			//Enable ISI ID FILTER
			nim_reg_read(dev, R106_BBF_CFG, &data, 1);
			data &= 0xef;

            // Enable update dfl every frame by paladin.ye from gavin 2015/11/24
            // when update dfl,there will be another bug appearing ,about the error bbheader in reg_105 gloria
            // if update dfl,the crc of bbheader will be invalid  when report to reg_105                           gloria
 
            data |= 0x08;
			nim_reg_write(dev, R106_BBF_CFG, &data, 1);	//set 106[3] = 1

			// Update bbframe header  every frame added by gloria 2016.1.8
			// 109 [1] = 1   
			nim_reg_read(dev, R106_BBF_CFG + 3, &data, 1);
			data |= 0x02;
			nim_reg_write(dev, R106_BBF_CFG + 3, &data, 1);	//set 106[3] = 1
					
			//BEQ CENTRAL UPDATE STEP SET
			//data = 0x6f;  // Just for 32apsk 9/10 case
			//data = 0x1f;    // For usual case to resist echo by paladin 01/16/2017
            // For cstm freeze issue 12341  by hyman 01/22/2017
            data = 0x0f;    
			nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1

            data = 0x04;    
			nim_reg_write(dev, R21_BEQ_CTRL, &data, 1);
            // End


            // TSO CLK config
            priv->nim_tso_cfg.tso_clk_cfg.is_manual_mode = 1;
			if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
            {
                priv->nim_tso_cfg.tso_clk_cfg.tso_clk_src = 1;
            }
            else
            {
			    priv->nim_tso_cfg.tso_clk_cfg.tso_clk_src = 3;
            }
            priv->nim_tso_cfg.tso_clk_cfg.clk_div_bypass= 0;
            priv->nim_tso_cfg.tso_clk_cfg.clk_div = 0;
                
            nim_c3505_set_tso_clk(dev, &(priv->nim_tso_cfg));
            // End

			data = 0x07;		  
			nim_reg_write(dev, R1BF_PL_SYNC_CFG, &data, 1); //PLSYNC cfg: cr_1bf[3]=0 in CCM mode
			data = 0x01;       
			nim_reg_write(dev, R97_S2_FEC_THR, &data, 1);//set S2_FEC_LOCK_THR[7:0] = 1

			//data = 0xff;
			data = 0xfe;
			nim_reg_write(dev, R98_S2_FEC_FAIL_THR, &data, 1);//set S2_FEC_FAIL_THR[7:0] = ff 		  

            // CR_FAST_UPDATE_EN      = reg_cred[1] = 0 for acm pn performance by hyman
			//nim_reg_read(dev, RED_CR_CTRL, &data, 1);
			//data &= 0xfd;
			//nim_reg_write(dev, RED_CR_CTRL, &data, 1);
            
			break;             
		case NIM_OPTR_CCM: 
			data =0x57; 
			nim_reg_write(dev,R5B_ACQ_WORK_MODE,&data,1);

			nim_reg_read(dev, R10b_ISI_SET, &data, 1);
			data &= 0xf7;
			nim_reg_write(dev, R10b_ISI_SET, &data, 1);	//set 10b[3] = 0

			nim_reg_read(dev, R106_BBF_CFG, &data, 1);
			data |= 0x10;

             // Enable update dfl every frame by paladin.ye from gavin 2015/11/24
            data |= 0x08;
             
			nim_reg_write(dev, R106_BBF_CFG, &data, 1);	//set 106[4] = 1

			data = 0x1f;
			nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1
			
            // TSO CLK config
            priv->nim_tso_cfg.tso_clk_cfg.is_manual_mode = 0;
            nim_c3505_set_tso_clk(dev, &(priv->nim_tso_cfg));
            // End
			
            data = 0x0f;
			nim_reg_write(dev, R1BF_PL_SYNC_CFG, &data, 1);//PLSYNC cfg: cr_1bf[3]=1 in CCM mode

			data = 0x4;        
			nim_reg_write(dev, R97_S2_FEC_THR, &data, 1);//set S2_FEC_LOCK_THR[7:0] = 4

			data = 0x20;
			nim_reg_write(dev, R98_S2_FEC_FAIL_THR, &data, 1);//set S2_FEC_FAIL_THR[7:0] = 20  

            nim_reg_read(dev, R21_BEQ_CTRL, &data,1);
            data &= 0x1f;
            data |= 0x20;// OTHER_UPDATE_STEP =1
            nim_reg_write(dev, R21_BEQ_CTRL, &data,1);

            
			break;  	       
		case NIM_OPTR_AUTO:         
			break;

		//nim_c3505_set_sub_work_mode(dev,s_Case);
	}

    // by paladin.ye from hyman 2015/11/24
	//data = 0x10;
	//nim_reg_write(dev, R98_S2_FEC_FAIL_THR, &data, 1);//set S2_FEC_FAIL_THR[7:0] = 7f

    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        data = 0xae;    // carcy 05/17/2017 mail
        nim_reg_write(dev, R21_BEQ_CTRL, &data,1);

		data = 0x5f;    // carcy 06/06/2017
		nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 5    
    }
    
	return SUCCESS;

}


/*****************************************************************************
*  INT32 nim_c3505_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid)
*  Get input stream id when it is in mult stream mode, must make sure TP is locked && dvb-s2 mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: UINT8
*****************************************************************************/ 
INT32 nim_c3505_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid)
{
    struct nim_c3505_private *priv = NULL;
    struct nim_dvbs_bb_header_info *p_bb_header_info = NULL;
	UINT8 data = 0;
    UINT16 time_out = 0;
    UINT8 cnt = 0;
    UINT8 i = 0, j = 0;
    UINT8 data_valid = 1;
    INT32 ret_get_bb_header = 0;
    //UINT8 work_mode = 0;

    ACM_DEBUG_PRINTF("Enter %s \n", __FUNCTION__);
    
	if(NULL == dev)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}

	priv = (struct nim_c3505_private *)dev->priv;
	if(NULL == priv)
	{
        ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}
	
	p_bb_header_info = (struct nim_dvbs_bb_header_info*)(priv->bb_header_info);
    
	comm_memset(p_isid->isid_read, 0, sizeof(p_isid->isid_read));	
    p_isid->isid_num = 0;
    p_isid->isid_overflow = 0;
	//p_isid->get_finish_flag = 0;
    time_out = p_isid->isid_read_timeout;
    
    if(time_out <= 100)
        time_out = 100;
    else if(time_out >= 5000)
        time_out = 5000;
 	
	while(1)// Check lock status
	{
		if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
		{
			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
				priv->ul_status.c3505_autoscan_stop_flag);	
			//p_isid->get_finish_flag = 0;
			return ERR_FAILED;
		}
		
		nim_c3505_get_lock(dev, &data);
		
		if(1 == data)// lock
			break;
		
		if(cnt >= time_out)// unlock
		{
			ACM_DEBUG_PRINTF("In %s exit with TP unlock \n", __FUNCTION__);
			return ERR_FAILED; 
		}

		cnt++; 
		comm_sleep(1);
	}

    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        // Clear isid fifo
    	nim_reg_read(dev, R10b_ISI_SET + 2, &data, 1);
    	data |= 0x20;
    	nim_reg_write(dev, R10b_ISI_SET + 2, &data, 1);

    	// wait capture
    	comm_sleep(time_out);

        // Begin get isid
    	for (i = 0; i < 32; i++)
    	{
    		// write index
    		nim_reg_read(dev, R106_BBF_CFG + 4, &data, 1);
    		data &= 0xe0;
    		data |= i;
    		nim_reg_write(dev, R106_BBF_CFG + 4, &data, 1);

    		// read
    		nim_reg_read(dev, R10b_ISI_SET + 1, &data, 1);

			for (j = 0; j < 8; j++)
			{
				if (0x01 & (data >> j))
				{
				    p_isid->isid_read[p_isid->isid_num] = ((i*8) + j);
					p_isid->isid_num++;
				}
            }
        }

    	ACM_DEBUG_PRINTF("Get isid finished, isid_num = %d\r\n", p_isid->isid_num);
    	for (i = 0; i < p_isid->isid_num; i++)
    		ACM_DEBUG_PRINTF("isid_read[%d] = 0x%x \r\n", i, p_isid->isid_read[i]);
    }
    else
    {
        // Check bbheader crc status
        cnt = 0; 
        while(1)
        {   
        	if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
    		{
    			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
    				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
    				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
    				priv->ul_status.c3505_autoscan_stop_flag);
    			//p_isid->get_finish_flag = 0;
    			return ERR_FAILED;
    		}
    		
            ret_get_bb_header = nim_c3505_get_bb_header_info(dev);

            if (SUCCESS == ret_get_bb_header)
            {
                cnt ++;
                if (CRC_THRESHOLD == cnt)
                {
                    ACM_DEBUG_PRINTF("BBHeader CRC check pass!\n");
                    if(NIM_STREAM_MULTIPLE == p_bb_header_info->stream_type)
                    {
                        ACM_DEBUG_PRINTF("Find a multi stream TP, continue!\n");
                        break;
                    }
                    else
                    {
    					ACM_DEBUG_PRINTF("Not a multi stream TP, eixt!\n");
    					//p_isid->get_finish_flag = 1;
    					return SUCCESS;//not need to get isid
                    }
                }
            }
            else
            {
                ACM_DEBUG_PRINTF("BBHeader CRC check fail!, exit!\n");
    			//p_isid->get_finish_flag = 1;
                return ERR_FAILED;
            }
        }
        
    	
        // Begin get isid
        cnt = 0;
        while(1)
        {
        	if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
    		{
    			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
    				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
    				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
    				priv->ul_status.c3505_autoscan_stop_flag);
    			//p_isid->get_finish_flag = 0;
    			return ERR_FAILED;
    		}
    		
            // step 1, get isid fifo index and fifo[7]
            nim_reg_read(dev, R10b_ISI_SET + 2, &data, 1);
            data &= 0x07;

            // step 2, if isid fifo index < 7, get  fifo[0~fifo index]
            if(data < 7)
            {
                if(cnt >= time_out)
                {   
                    p_isid->isid_num = data;
                    for(i = 0; i < p_isid->isid_num; i++)
                    {
                        // Configure BB ISI ID FIFO INDEX
                        nim_reg_read(dev, R10b_ISI_SET, &data, 1);
                        data &= 0x8f;
                        data |= ((i & 0x07) << 4);
                        nim_reg_write(dev, R10b_ISI_SET, &data, 1);       
                        nim_reg_read(dev, R10b_ISI_SET + 1, &data, 1);
                        p_isid->isid_read[i] = data;
                        ACM_DEBUG_PRINTF("Get isid[%d] = %d \n", i, p_isid->isid_read[i]);
                    }
    				//p_isid->get_finish_flag = 1;
    				if (priv->isid != p_isid)
    				{
    					comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
    				}
    				NIM_PRINTF("[%s %d]return!\n", __FUNCTION__, __LINE__);
                    return SUCCESS;
                }
                
                comm_sleep(1);
                cnt++;    
            }
            //  fifo index = 7, fifo[0~7]
            else
            {
                p_isid->isid_num = data + 1;
                for(i = 0; i < p_isid->isid_num; i++)
                {
                    // Configure BB ISI ID FIFO INDEX
                    nim_reg_read(dev, R10b_ISI_SET, &data, 1);
                    data &= 0x8f;
                    data |= ((i & 0x07) << 4);
                    nim_reg_write(dev, R10b_ISI_SET, &data, 1);       
                    nim_reg_read(dev, R10b_ISI_SET + 1, &data, 1);
                    
                    if(i < 7)
                    {
                        p_isid->isid_read[i] = data;
                        ACM_DEBUG_PRINTF("Get isid[%d] = %d \n", i, p_isid->isid_read[i]);
                    }
                    else
                    // i = 7, for fifo[7] data may be same with fifo[0~6]
                    {
                        for(j = 0; j < 7; j++)
                        {
                            if(data == p_isid->isid_read[j])
                            {
                                data_valid = 0;
                                break;
                            }
                            else
                            {
                                data_valid = 1;
                            }
                        }

                        if(1 == data_valid)
                        {
                            p_isid->isid_read[i] = data;
                            ACM_DEBUG_PRINTF("Get isid[%d] = %d \n", i, p_isid->isid_read[i]);
                            break;
                        }
                        else
                        {
                            p_isid->isid_num--;
                            ACM_DEBUG_PRINTF("Get isid[7] = %d, same with record \n", data);
    						//p_isid->get_finish_flag = 1;
    						if (priv->isid != p_isid)
    						{
    							comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
    						}
    						NIM_PRINTF("[%s %d]return!\n", __FUNCTION__, __LINE__);
                            return SUCCESS;
                        }
                    }
                }
                
                ACM_DEBUG_PRINTF("Isid num may be more than %d, Jump to step 3\n", p_isid->isid_num);
                break;
            }
        }

        // step 3, if isid fifo index >= 7,then we need  read the last fifo[7] data continual that collect all the isid
        cnt = 0;
        
        while(1)
        {
        	if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
    		{
    			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
    				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
    				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
    				priv->ul_status.c3505_autoscan_stop_flag);
    			//p_isid->get_finish_flag = 0;
    			return ERR_FAILED;
    		}
    		
            nim_reg_read(dev, R10b_ISI_SET + 1, &data, 1);
            
            // find a new isid
            for(i = 0; i < p_isid->isid_num; i++)
            {
                if(data == p_isid->isid_read[i])
                {
                    data_valid = 0;
                    break;
                }
                else
                {
                    data_valid = 1;
                }
            }

            if(1 == data_valid)
            {
                p_isid->isid_num++;
                p_isid->isid_read[p_isid->isid_num - 1] = data;
                cnt = 0;
                ACM_DEBUG_PRINTF("Setp 3 find a new isid[%d] = %d\n", p_isid->isid_num - 1, data);
            }

            if(cnt > time_out)
            {
    			ACM_DEBUG_PRINTF("Exit get isid process have not found new isid, time_out = %d\n", cnt); 
    			//p_isid->get_finish_flag = 1;
    			if (priv->isid != p_isid)
    			{
    				comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
    			}
    			NIM_PRINTF("[%s %d]return!\n", __FUNCTION__, __LINE__);
    			return SUCCESS;
            }
            
            cnt++;
            comm_sleep(1);
        }
    }

	//p_isid->get_finish_flag = 1;
	if (priv->isid != p_isid)
	{
		comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
	}
	NIM_PRINTF("[%s %d]return!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_set_isid(struct nim_device *dev, struct nim_dvbs_isid *isid)
*  Set input stream id when it is in mult stream mode, must set a real id by isid_get
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: UINT8
*****************************************************************************/ 
INT32 nim_c3505_set_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid)
{   
	UINT8 data = 0;	
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
    
	if ((NULL == dev) || (NULL==p_isid))
	{
        ERR_PRINTF("[%s %d](NULL == dev) or (NULL==isid)\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}
    
    ACM_DEBUG_PRINTF("Enter %s ID = %d\n", __FUNCTION__, p_isid->isid_write);
	    
    // Config isi filter
    nim_reg_read(dev, R106_BBF_CFG, &data, 1);
    data &= 0xef;
    data |= ((p_isid->isid_bypass & 0x01) << 4);
    nim_reg_write(dev, R106_BBF_CFG, &data, 1);  

    // Config BB_MULTI_TS_ISI_RST_EN
    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        if (0x01 & p_isid->isid_bypass)
        {
            nim_reg_read(dev, R10b_ISI_SET + 2, &data, 1);
        	data &= 0xef;
        	nim_reg_write(dev, R10b_ISI_SET + 2, &data, 1);
        }
        else
        {
            nim_reg_read(dev, R10b_ISI_SET + 2, &data, 1);
        	data |= 0x10;
        	nim_reg_write(dev, R10b_ISI_SET + 2, &data, 1);
        }
    }

    // Config isid
    data = p_isid->isid_write;
    nim_reg_write(dev, R106_BBF_CFG + 1, &data, 1);    

    // Configure isi mode
    nim_reg_read(dev, R10b_ISI_SET, &data, 1);
    //data &= 0xf9;
    //data |= ((p_isid->isid_mode & 0x03) << 1);
    data &= 0xfd;
    data |= ((p_isid->isid_mode & 0x01) << 1);    
    nim_reg_write(dev, R10b_ISI_SET, &data, 1);  

    return SUCCESS;
}

/***************************************************************************************************
*  INT32 nim_c3505_set_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid)
*  When TP has been locked, we need to set isid parameter in hardware for normal play
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: UINT8
****************************************************************************************************/ 
INT32 nim_c3505_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid)
{
    UINT8 work_mode = 0;
	struct nim_c3505_private *priv = NULL;
    
    // Check 
    if ((NULL == dev) || (NULL == p_isid))
    {
        ERR_PRINTF("[%s %d](NULL == dev) or (NULL == p_isid)\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;
    if (NULL == priv)
    {
    	ERR_PRINTF("[%s %d](NULL == priv)\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
    }

    
    ACM_DEBUG_PRINTF("Run to %s, change_type = %d\n", __FUNCTION__, change_type);       

    // clear isid num
    p_isid->isid_num = 0;
    
    // Check TP type
    nim_c3505_reg_get_work_mode(dev, &work_mode);
    if(1 != work_mode)
    {
        ACM_DEBUG_PRINTF("In %s exit with TP is not DVB-S2\n", __FUNCTION__);		
        //return SUCCESS;//because only dvbs2 need get isid, so other mode return success. 
        return ERR_FAILED;
    }    

    // In DVBS2 & ACM & Mult stream mode, we need set isid to hardware by software in auto mode
    if(SUCCESS != nim_c3505_get_bb_header_info(dev))
    {
        //data = 0x1f;
        //nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1
        ACM_DEBUG_PRINTF("[%s %d], Get bbheader fail!\n", __FUNCTION__, __LINE__);
        return ERR_CRC_HEADER; 
    }

	// For ccm tp in acm work mode
    //if(NIM_CCM == priv->bb_header_info->acm_ccm)
    //	nim_c3505_adapt_ppll_mode(dev);	// move it to nim_c3505_lock_unlock_adaptive

    // Check stream type
    if(NIM_STREAM_MULTIPLE != priv->bb_header_info->stream_type)
    {
        ACM_DEBUG_PRINTF("[%s %d], it is not a multi stream TP!\n", __FUNCTION__, __LINE__);
        return ERR_FAILED; 
    }


    //if(3 == change_type)// Set TP and get ISID
    if(NIM_CHANGE_SET_TP_AND_GET_ISID == change_type)
    {
        if(SUCCESS != nim_c3505_get_isid(dev, p_isid))
        {
        	ACM_DEBUG_PRINTF("[%s %d]get isid error!\n", __FUNCTION__, __LINE__);
            return ERR_FAILED;
        }
		
        p_isid->isid_bypass = 0;
        p_isid->isid_mode = 1;
        p_isid->isid_write = p_isid->isid_read[0];//default set the first isid to demod 
        if(SUCCESS != nim_c3505_set_isid(dev, p_isid))
        {
        	ACM_DEBUG_PRINTF("[%s %d]set isid error!\n", __FUNCTION__, __LINE__);
            return ERR_FAILED;
        }
    }
  
    return SUCCESS;
}

INT32 nim_c3505_s2_ldpc_est_ber_enable(struct nim_device *dev)
{
	UINT8 data;
	data = 0x06;
	// enable ber s, update ber each fec frame
	nim_reg_write(dev,RD3_BER_REG,&data,1);
    return SUCCESS;
}

INT32 nim_c3505_s2_ldpc_est_ber_disable(struct nim_device *dev)
{
	UINT8 data;
	data = 0x02;
	// disable ber s
	nim_reg_write(dev,RD3_BER_REG,&data,1);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_hw_check(struct nim_device *dev)
* Description: Chip ID check
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_hw_check(struct nim_device *dev)
{
	UINT8 data;
    UINT16 chip_id = 0, chip_version = 0;
	NIM_PRINTF("Enter function: %s \n",__FUNCTION__);

    nim_reg_read(dev, RA3_CHIP_ID, &data, 1);
    chip_id |= (UINT16)data;
    
    nim_reg_read(dev, RA3_CHIP_ID + 1, &data, 1);
    chip_id |= (UINT16)data << 8;

    nim_reg_read(dev, RA5_VER_ID, &data, 1);
    chip_version |= (UINT16)data;
    
    nim_reg_read(dev, RA5_VER_ID + 1, &data, 1);
    chip_version |= (UINT16)data << 8;
    
	NIM_PRINTF("Chip ID = %x , Chip Version = %x \n",chip_id, chip_version);
        
	if (CHIP_ID == (chip_id >> 8))
		return SUCCESS;
	else
    {
        ERR_PRINTF("Exit with Chip ID error %s \n", __FUNCTION__);
        ERR_PRINTF("The expect Chip ID = %x , real Chip ID = %x \n", CHIP_ID, chip_id);
        return ERR_FAILED;
    }  

}

/*****************************************************************************
*  INT32 nim_C3505_hw_init(struct nim_device *dev)
*  C3505 hardware initialization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_hw_init(struct nim_device *dev)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	UINT8 data ;
	
	NIM_PRINTF("Enter function: %s!\n",__FUNCTION__);

    //change from 0x07 to 0x03, for rxadc rst timing
	//it should be delay 1us after RXADC Ref Power Up to reset I/Q
	data = 0x03;	
	nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);    

	// set TR lock symbol number thr, k unit.
	data = 0x1f; // setting for soft search function
	nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);

	// Carcy change PL time out, for low symbol rate. 2008-03-12   
	data = 0x84;
#if (NIM_OPTR_ACM == ACM_CCM_FLAG)
        if (CHIP_ID_3503D == priv->ul_status.m_c3505_type) 
        {
            data = 0x88;
        }
#endif
	nim_reg_write(dev, R28_PL_TIMEOUT_BND + 0x01, &data, 1);

	// Set Hardware time out
	nim_c3505_set_hw_timeout(dev, 0xff);

	//----eq demod setting
	// Open EQ controll for QPSK and 8PSK

    // Move it in acm_set function 5/17/2017
	//data = 0x04;		//  set EQ control
	//nim_reg_write(dev, R21_BEQ_CTRL, &data, 1);

    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        data = 0x20;        //  5/17/2017 carcy mail
    }
    else
    {
        data = 0x24;        //  set EQ mask mode, mask EQ for 1/4,1/3,2/5 code rate
    }
	nim_reg_write(dev, R25_BEQ_MASK, &data, 1);

	// Carcy add for 16APSK
	data = 0x6c;
	nim_reg_write(dev, R2A_PL_BND_CTRL + 0x02, &data, 1);
	//----eq demod setting end

	// FEC clock select:270M, 224M for 3503d
	if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        data = 0xb1;
    }   
    else
    {
	    data = 0x81;
    }
	nim_reg_write(dev, RFA_RESET_CTRL, &data, 1);

	nim_c3505_set_adc(dev);

	if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_8BIT_MODE)
		nim_c3505_tso_initial (dev, 1, 1);
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_1BIT_MODE)
		nim_c3505_tso_initial (dev, 1, 0);

	nim_c3505_diseqc_initial(dev);
	nim_c3505_cr_adaptive_initial(dev);

	// register diff with c3503
	/*
	data = 0xe1;
	nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
	*/

	nim_reg_read (dev, RF1_DSP_CLK_CTRL, &data, 1);
	data &= 0xfb	  ; //LDPC clock gate enable to save power by Gavin 
	data = data | 0x08; // enable DVBS/S2 FEC clock gate.
	data = data | 0x10; // enable mdo_cnt_clean with sync
	nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);


	// For C3503 new CR only
	//    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
	//    {
	nim_c3505_cr_new_tab_init(dev);
	nim_c3505_cr_adaptive_method_choice(dev,0); //0:new;1:old;2:both;3:register  
	// enable MERGE_ROUND 
	nim_reg_read(dev,0x137,&data,1);
	data |=0x20; 
	nim_reg_write(dev,0x137,&data,1);    
	//    }


	// For C3503 FEC only
	//    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
	//    {          
#ifdef INIT_32APSK_TARGET
	nim_c3505_set_32apsk_target(dev);
#endif
	nim_reg_read(dev,0xb4,&data,1);
	data &=0x8f;
	data |=0x10;// CENTRAL_UPDATE_STEP =1
	nim_reg_write(dev,0xb4,&data,1);


	data=0xff;
	nim_reg_write(dev,RC1_DVBS2_FEC_LDPC+1,&data,1);
	nim_reg_read(dev,RC1_DVBS2_FEC_LDPC,&data,1);
	data &= 0xf0; //disable LDPC_AVG_ITER_ACT
	//  data |=0x30; // ldpc_max_iter_num[9:8]=0x3
	nim_reg_write(dev,RC1_DVBS2_FEC_LDPC,&data,1);      
	//}

	nim_c3505_acm_set(dev,ACM_CCM_FLAG);

    // For c3503d lock rpt
    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        data = 0x01;       
        nim_reg_write(dev, R97_S2_FEC_THR, &data, 1);//set S2_FEC_LOCK_THR[7:0] = 1
    
        nim_reg_read(dev, R122_S2_FEC_FSM, &data, 1);
        data |= 0x80;
        nim_reg_write(dev, R122_S2_FEC_FSM, &data, 1); 
    }


	// new PLSYNC setting
	if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {   
	    nim_c3505_pl_sync_cfg(dev, 1);
    }
    else
    {
        #if (NIM_OPTR_CCM == ACM_CCM_FLAG)
            nim_c3505_pl_sync_cfg(dev, 0);
        #else
            nim_c3505_pl_sync_cfg(dev, 1);
        #endif
    }
	
    // Gavin Zhang 11/04/2015
    nim_c3505_s2_ldpc_est_ber_enable(dev);

	//	nim_c3505_TSBB_cap_set(dev,NIM_CAP_TS1,0x00112233,0x00556677);
	//	nim_c3505_TSBB_cap_set(dev,NIM_CAP_TS2,0x00112233,0x00556677);

    // Paladin.ye 05/30/2016 Reduce dvb-s mode lock threshold
    
    data = 0x92;
    nim_reg_write(dev, R48_VITERBI_CTRL, &data, 1);
    
    data = 0xc9;
    nim_reg_write(dev, R48_VITERBI_CTRL + 1, &data, 1);

    data = 0x44;
    nim_reg_write(dev, R48_VITERBI_CTRL + 2, &data, 1);

    data = 0xbc;
    nim_reg_write(dev, R48_VITERBI_CTRL + 3, &data, 1);

    data = 0x02;
    nim_reg_write(dev, R48_VITERBI_CTRL + 4, &data, 1);

    data = 0x19;
    nim_reg_write(dev, R48_VITERBI_CTRL + 5, &data, 1);

    data = 0x2c;
    nim_reg_write(dev, R48_VITERBI_CTRL + 6, &data, 1);

    data = 0x01;
    nim_reg_write(dev, R48_VITERBI_CTRL + 7, &data, 1);

    data = 0x64;
    nim_reg_write(dev, R33_CR_CTRL + 6, &data, 1);
    // end

    // Paladin.ye 05/30/2016 For fix hardware bug
    // bb frame's BYPASS_ALIGN_BIT should be 1
	nim_reg_read(dev, R106_BBF_CFG, &data, 1);
	data |= 0x20; 
	// Updata UPL every frame, for not 188byte lenth stream, paladin 2016/11/01
	data |= 0x04;		
	nim_reg_write(dev, R106_BBF_CFG, &data, 1); 
    // packing register PKT_ERR_CTL_BIT should be 0
	nim_reg_read(dev, R106_BBF_CFG + 8, &data, 1);
	data &= 0xfb; 
	nim_reg_write(dev, R106_BBF_CFG + 8, &data, 1); 
    // end

    // For c3503d dvbs 
    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
    {
        nim_reg_read(dev, R1C4_DVBS_FEC_CFG, &data, 1);
        data |= 0x03; 
        nim_reg_write(dev, R1C4_DVBS_FEC_CFG, &data, 1); 
    }

	//fix issue 92710 paladin.ye 20180315
	nim_reg_read(dev, R1BE_PL_SYNC_CFG, &data, 1);
	data |= 0x07;
	nim_reg_write(dev, R1BE_PL_SYNC_CFG, &data, 1);
	
	data = 0x0b; 
	nim_reg_write(dev, R1A7_SNR_ESTIMATOR, &data, 1);//set snr estimator register, window is 8192 

	// enable ADC
	data = 0x00;
	nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);

	// Hyman add for 1MRs S2 TP lock slow issue 2018/1/26
    data = 0x01;
    nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1); 

	return SUCCESS;
}


/******************************************************************************
*  INT32 nim_c3505_module_demap_flt_ini(struct nim_device *dev)
* Configure demap output filter
*
* Arguments:
* Parameter1: struct nim_device *dev

* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_module_demap_flt_ini(struct nim_device *dev)
{
	UINT8 data;
	// disable demap filter in default
	nim_reg_read(dev, R1C3_DEMAP_FLT_CODE_RATE, &data, 1);
	data = data & 0x7F;
	nim_reg_write(dev, R1C3_DEMAP_FLT_CODE_RATE, &data, 1);

	return SUCCESS;
}

 /*****************************************************************************
*  INT32  nim_c3505_module_demap_flt_rpt(struct nim_device *dev, UINT32 *demap_flt_status)
* Report demap filter status
*
* Arguments:
* Parameter1: struct nim_device *dev
*
* Return Value: UINT16 *demap_flt_status
*****************************************************************************/
INT32 nim_c3505_module_demap_flt_rpt(struct nim_device *dev, UINT16 *demap_flt_status)
{
    UINT8 data;

    nim_reg_read(dev, R1C3_DEMAP_FLT_CODE_RATE, &data, 1);
    if(data >>7)
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt is enable\n");
    }
    else
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt is disable\n");
    }

    nim_reg_read(dev, R1C0_DEMAP_FLT_MODU_TYPE, &data, 1);
    NIM_PRINTF("\t\t: SUB_MODULE Demap_flt modu type is %02h\n", data);

    nim_reg_read(dev, R1C1_DEMAP_FLT_FRAME_PILOT, &data, 1);
    if(data & 0x01)
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass long frame\n");
    }
    else
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt block long frame\n");
    }

    if(data & 0x02)
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass short frame\n");
    }
    else
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt block short frame\n");
    }

    if(data & 0x10)
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass pilot off frame\n");
    }
    else
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt block pilot off frame\n");
    }
	
    if(data & 0x20)
    {
    	NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass pilot on frame\n");
    }
    else
    {
        NIM_PRINTF("\t\t: SUB_MODULE Demap_flt block pilot on frame\n");
    }
	
    nim_reg_read(dev, R1C2_DEMAP_FLT_CODE_RATE, &data, 1);
    NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass code rate %02h [4/5 3/4 2/3 3/5 1/2 2/5 1/3 1/4] \n", data);

    nim_reg_read(dev, R1C3_DEMAP_FLT_CODE_RATE, &data, 1);
    NIM_PRINTF("\t\t: SUB_MODULE Demap_flt pass code rate %02h [9/10 8/9 5/6] \n", data&0x07);

    return SUCCESS;
}

 /*****************************************************************************
*  INT32 nim_c3505_module_demap_flt_set(struct nim_device *dev, UINT8 flt_en, UINT8 flt_modu_type, UINT8 flt_frame_type, UINT8 flt_pilot, UINT16 flt_code_rate)
* Set demap filter parameter
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flt_en: 0: default, disable filter function. 1: enable filter function
*     Note: Bellow setting, the bit set to 1 means pass, 0 means block.
*  Parameter3: UINT8 flt_modu_type[3:0]: {32APSK, 16psk, 8psk, qpsk}
*  Parameter4: UINT8 flt_frame_type[1:0]: {short frame, long frame}
*  Parameter5: UINT8 flt_pilot[1:0]: {pilot on, pilog off}
*  Parameter6: UINT8 flt_code_rate[10:0] : [9/10 8/9 5/6 4/5 3/4 2/3 3/5 1/2 2/5 1/3 1/4]

* Return Value: INT32
*****************************************************************************/
 
INT32 nim_c3505_module_demap_flt_set(struct nim_device *dev, UINT8 flt_en, UINT8 flt_modu_type, UINT8 flt_frame_type, UINT8 flt_pilot, UINT16 flt_code_rate)
{
	UINT8 data;

	data = flt_modu_type&0x0f;
	nim_reg_write(dev, R1C0_DEMAP_FLT_MODU_TYPE, &data, 1);

	data = ((flt_pilot & 0x3) << 4) | (flt_frame_type&0x03);
	nim_reg_write(dev, R1C1_DEMAP_FLT_FRAME_PILOT, &data, 1);

	data = flt_code_rate & 0xff;
	nim_reg_write(dev, R1C2_DEMAP_FLT_CODE_RATE, &data, 1);

	data = ((flt_en & 0x1) << 7) | ((flt_code_rate>>8)&0x07);
	nim_reg_write(dev, R1C3_DEMAP_FLT_CODE_RATE, &data, 1);

	return SUCCESS;
}


 /******************************************************************************
*  INT32 nim_c3505_module_cci_ini(struct nim_device *dev)
* Cci Cancel initialization 
*
* Arguments:
*  Parameter1: struct nim_device *dev

* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_module_cci_ini(struct nim_device *dev)
{
	UINT8 data;

	nim_reg_read(dev, R184_CCI_DIRECT_BYPASS, &data, 1);
	data = data | 0x80;
	nim_reg_write(dev, R184_CCI_DIRECT_BYPASS, &data, 1);

	return SUCCESS;
}
 /*****************************************************************************
*  INT32 nim_c3505_module_cci_rpt(struct nim_device *dev, UINT16 *cci_rpt_freq, UINT8 *cci_rpt_level, UINT8 *cci_rpt_lock, UINT8 s_Case)
* Cci Cancel report
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *cci_rpt_freq
*  Parameter3:  UINT8 *cci_rpt_level
*  Parameter4: UINT8 *cci_rpt_lock
*  Parameter5: UINT8 s_Case

*
* Return Value: UINT16 *cci_rpt_freq
* Return Value: UINT8 *cci_rpt_level
* Return Value: UINT8 *cci_rpt_lock
*****************************************************************************/
INT32 nim_c3505_module_cci_rpt(struct nim_device *dev, UINT16 *cci_rpt_freq, UINT8 *cci_rpt_level, UINT8 *cci_rpt_lock, UINT8 s_case)
{
	UINT8 data = 0;
	UINT16 temp = 0;

	switch (s_case)
	{
	case NIM_RPT_PLL0:
		nim_reg_read(dev, R183_CCI_PLL_R_SEL, &data, 1);
		data = data & 0xfc;
		nim_reg_write(dev, R183_CCI_PLL_R_SEL, &data, 1); // read pll0 enable
		break;
	case NIM_RPT_PLL1:
		nim_reg_read(dev, R183_CCI_PLL_R_SEL, &data, 1);
		data = data & 0xfc;
		data = data | 0x01;
		nim_reg_write(dev, R183_CCI_PLL_R_SEL, &data, 1); // read pll1 enable
		break;		
	case NIM_RPT_PLL2:
		nim_reg_read(dev, R183_CCI_PLL_R_SEL, &data, 1);
		data = data & 0xfc;
		data = data & 0x02;
		nim_reg_write(dev, R183_CCI_PLL_R_SEL, &data, 1); // read pll2 enable
		break;
	default:
		NIM_PRINTF(" CMD for nim_c3505_module_cci_rpt ERROR!!!");
		break;	   				
	}
	nim_reg_read(dev, R18F_CCI_REPORT_LOCK, &data, 1);
	data = data & 0x0f;
	temp = data ;
	temp = temp << 8;
	nim_reg_read(dev, R18E_CCI_REPORT_FREQ, &data, 1);
	temp = temp + data;
	*cci_rpt_freq = temp;  // read pll lock frequency

	nim_reg_read(dev, R190_CCI_REPORT_LEVEL, &data, 1);
	*cci_rpt_level = data;  //read pll lock level

	nim_reg_read(dev, R18F_CCI_REPORT_LOCK, &data, 1);
	data = data & 0x10;
	data =  data >> 4;
	*cci_rpt_lock = data; //read pll lock status

	return SUCCESS;
}

 /*****************************************************************************
*  INT32 nim_c3505_module_cci_set(struct nim_device *dev, UINT8 s_Case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*
* Return Value: INT32
*****************************************************************************/

INT32 nim_c3505_module_cci_set(struct nim_device *dev, UINT8 s_case)
{
	UINT8 data;
	switch(s_case)
	{
		case NIM_CCI_CANCEL:     // cci_cancel enable 
			nim_reg_read(dev, R184_CCI_DIRECT_BYPASS, &data, 1);
			data = data & 0x7f;
			nim_reg_write(dev, R184_CCI_DIRECT_BYPASS, &data, 1);
			break;
		case NIM_CCI_BYPASS:  // cci_cancel bypass
			nim_reg_read(dev, R184_CCI_DIRECT_BYPASS, &data, 1);
			data = data | 0x80;
			nim_reg_write(dev, R184_CCI_DIRECT_BYPASS, &data, 1);
			break;
		default:
			NIM_PRINTF("nim_c3505_module_cci_set ERROR!!!");
			break;
	}
	return SUCCESS;
}


 /******************************************************************************
*  INT32 nim_c3505_module_diseqc_cmd_queue_ini(struct nim_device *dev)
* Configure diseqc command queue
*
* Arguments:
* Parameter1: struct nim_device *dev

* Return Value: INT32
*****************************************************************************/
 
INT32 nim_c3505_module_diseqc_cmd_queue_ini(struct nim_device *dev)
{
	UINT8 data = 0;
	// disable diseqc command queue
	nim_reg_read(dev, R15D_DISEQC_CMD_QUE_CTL, &data, 1);
	data = data | 0x01;
	nim_reg_write(dev, R15D_DISEQC_CMD_QUE_CTL, &data, 1);
	
    return SUCCESS;
}
 /*****************************************************************************
*  INT32  nim_c3505_module_diseqc_cmd_queue_rpt (struct nim_device *dev)
* Report diseqc command queue status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: none
*****************************************************************************/
INT32 nim_c3505_module_diseqc_cmd_queue_rpt(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, R15E_DISEQC_CMD_QUE_RPT, &data, 1);
	if(data & 0x01)
	{
		NIM_PRINTF("\t\t: SUB_MODULE diseqc cmd fifo is full\n");
	}
	
	if(data & 0x20)
	{
		NIM_PRINTF("\t\t: SUB_MODULE diseqc cmd queue have error\n");
	}

	NIM_PRINTF("\t\t: SUB_MODULE diseqc cmd queue state[3:0]: %02x \n", data>>4);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_module_diseqc_cmd_queue_set(struct nim_device *dev, UINT8 diseqc_que_cmd)
* Set demap filter parameter
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 diseqc_que_cmd
*      cmd	parameter	                Comment
*      1	time parameter	            Silience unit ms
*      0	0	0	0   x   x   x   0	V
*      0	0	0	0   x   x   x   1	H
*      0	0	0	1   x   x   x   0	22k off
*      0	0	0	1   x   x   x   1	22k on
*      0	0	1	0   x   x   x   0	tuneburst 0
*      0	0	1	0   x   x   x   1	tuneburst 1
*      0	0	1	1   x   x   x   x   store HV/22k status
*      0	1	0	0	data num	    write
*      0	1	0	1	data num	    write with reply
*      0	1	1	0	x   x   x   x   Resume HV/22k status
*      0	1	1	1	x   x   x   1	start command queue
*      0	1	1	1	x   x   x  	0	clear command queue
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_module_diseqc_cmd_queue_set(struct nim_device *dev, UINT8 diseqc_que_cmd)
{
	UINT8 data = 0;

	nim_reg_read(dev, R15E_DISEQC_CMD_QUE_RPT, &data, 1);
	if(data & 0x01)
	{
		NIM_PRINTF("\t\t: SUB_MODULE diseqc cmd queue fifo is full\n");
		return ERR_NO_MEM;
	}
	else
	{
		data = diseqc_que_cmd & 0xff;
		nim_reg_write(dev, R15C_DISEQC_CMD_QUE_IN, &data, 1);
	}

	return SUCCESS;
}

#if 0
/*****************************************************************************
*  INT32 nim_c3505_trans_pls(struct nim_device *dev, struct nim_dvbs_pls *pls_data)
* Transform the root/gold  pls data
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct *pls_data
            UINT32 root_num;        // 0 ~ 262141
            UINT32 gold_num;        // 0 ~ 262141
            UINT8 sel;                     // 0 = default, 1 = root, 2 = gold
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_trans_pls(struct nim_device *dev, PLS *pls_data)
{
    // Pointer check
    if ((NULL == dev) || (NULL == pls_data))
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n",__FUNCTION__);
        return ERR_NO_MEM;
    }

    if (1 == pls_data->pls_flag)    // Root format 
    {
        if ((pls_data->pls_number > 262141) || (pls_data->pls_number < 0))
        {
            NIM_PRINTF("In %s, root_num = %d over range\n", pls_data->pls_number);
            return ERR_FAILED;
        }
        //pls_data->pls_number = nim_c3505_pls_root_to_gold(pls_data->pls_number);
    }
    else if (2 == pls_data->pls_flag)    // set gold format pls to hw
    {
        if ((pls_data->pls_number > 262141) || (pls_data->pls_number < 0))
        {
            NIM_PRINTF("In %s, gold_num = %d over range\n", pls_data->pls_number);
            return ERR_FAILED;
        }
        pls_data->pls_number = nim_c3505_pls_gold_to_root(pls_data->pls_number);
    }
        
    return SUCCESS;
}
#endif

/*****************************************************************************
*  INT32 nim_c3505_set_pls(struct nim_device *dev, struct nim_dvbs_pls *pls_data)
* Set root/gold format pls to hw, and return gold/root pls to upper layer 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct *pls_data
            UINT32 root_num;        // 0 ~ 262141
            UINT32 gold_num;        // 0 ~ 262141
            UINT8 sel;                     // 0 = default, 1 = root, 2 = gold
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_plsn(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT32 plsn_root;
	UINT32 plsn_gold;
	struct nim_c3505_private *priv = NULL;
	priv = (struct nim_c3505_private *)dev->priv; 

	NIM_PRINTF("Enter %s \n",__FUNCTION__);
    // Pointer check
    if (NULL == dev->priv)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
    }

	//get plsn_gold
	if(priv->plsn.plsn_try == 1)
		plsn_gold = priv->plsn.plsn_try_val;
	else
		plsn_gold = priv->plsn.plsn_now;

	plsn_root = nim_c3505_plsn_gold_to_root(plsn_gold);
	
	NIM_PRINTF("In %s, plsn_gold = %d, plsn_root = %d\n", __FUNCTION__, plsn_gold,plsn_root);

	//set root format plsn
    data = plsn_root & 0xff;
		nim_reg_write(dev, R159_PLDS_SW_INDEX, &data, 1);
    data = (plsn_root >> 8) & 0xff;
        nim_reg_write(dev, R159_PLDS_SW_INDEX + 1, &data, 1);
    data =  ((plsn_root >> 16) & 0x03) | 0x80;
        nim_reg_write(dev, R159_PLDS_SW_INDEX + 2, &data, 1);
 
    return SUCCESS;
}

UINT32 nim_c3505_get_plsn(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT32 plsn_root;
	UINT32 plsn_gold;

	NIM_PRINTF("Enter %s \n",__FUNCTION__);
    // Pointer check
    if (NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
    }

    nim_reg_read(dev, R159_PLDS_SW_INDEX + 2, &data, 1);
	plsn_root = (data&0x03);
	NIM_PRINTF("data=0x%x, plsn_root=%d\n", data, plsn_root);
	
	nim_reg_read(dev, R159_PLDS_SW_INDEX + 1, &data, 1);
	plsn_root <<= 8;
	plsn_root += data;
	NIM_PRINTF("data=0x%x, plsn_root=%d\n", data, plsn_root);
	
	nim_reg_read(dev, R159_PLDS_SW_INDEX, &data, 1);
	plsn_root <<= 8;
	plsn_root += data;
	NIM_PRINTF("data=0x%x, plsn_root=%d\n", data, plsn_root);
	
	plsn_gold = nim_c3505_plsn_root_to_gold(plsn_root);
 	
	NIM_PRINTF("In %s, plsn_gold = %d, plsn_root = %d\n", __FUNCTION__, plsn_gold,plsn_root);

    return plsn_gold;
}


/*****************************************************************************
*  INT32 nim_c3505_close_ppll(struct nim_device *dev)
* Close PPLL for CCM pilot off mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_close_ppll(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);

    if (0x02 == (data&0x02))
    {
    	data &= 0xfd;
    	nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);
    }

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3505_open_ppll(struct nim_device *dev)
* Open PPLL for Try lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_open_ppll(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);

    if (0x00 == (data&0x02))
    {    
    	data |= 0x02;
    	nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);
    }

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3505_adapt_ppll_mode(struct nim_device *dev)
* set ppll mode when channel lock
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_adapt_ppll_mode(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT32 cnt = 0;

	// step1, check acq sub work mdoe is acm, cr5b[3:2] 0 is acm
	nim_reg_read(dev, R5B_ACQ_WORK_MODE, &data, 1);
	if (0 != (data & 0x0c))
		return SUCCESS;

	// step2, check current frame is not dummy, crf8[7], 1 is dummy
	while(1)
	{
		nim_reg_read(dev, RF8_MODCOD_RPT, &data, 1);
		if(0 == (data & 0x80))
			break;
		comm_sleep(1);
		cnt++;
		if(cnt > 50)
			return SUCCESS;
	}

	// step3, check current pilot mode, crf8[0]
	if (0 == (data & 0x01))
    {
        NIM_PRINTF("nim_c3505_colse_ppll\n");
		nim_c3505_close_ppll(dev);
    }
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_set_work_mode_auto(struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_work_mode(struct nim_device *dev, UINT8 work_mode)
{
    UINT8 data = 0;
    
    // Set work mode
    nim_reg_read(dev, R5B_ACQ_WORK_MODE, &data, 1);
    data &= 0xfc;
    data |= (work_mode & 0x03);
    nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
    
    return SUCCESS;
}


/*****************************************************************************
*  void nim_c3505_set_map_beta_extra(struct nim_device *dev, UINT8 index, UINT8 frame_mode)
* set demap beta, it only for 8psk 2/3 normal used now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
*  Parameter4: UINT8 state
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_set_map_beta_extra(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 state)
{
	UINT8 data;

    NIM_PRINTF("%s, index = %d, frame_mode = %d, state = %d\n", __FUNCTION__, index, frame_mode, state);
    
	if(frame_mode == 1)//short
	{
        
	}
	else if(frame_mode == 0)//normal
	{
		data = index;//index                                        			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);          			
		data = c3505_map_beta_active_buf_normal[index];//act enable             
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);               			
		data = 0x04;                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			
		data = c3505_map_beta_buf_normal_extra[index][state];//data                                
		nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);                 			
		data = 0x03;                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			
	}
    
    return SUCCESS;
}

#ifdef ASKEY_PATCH1
const UINT8 c3505_cr_prs_normal_adaptive_en[32] = 
{
    0,  //  index 0, do not use                                                                                                 
    0,  //  1/4 of QPSK  
    0,  //  1/3  			
    0,  //  2/5  			
    0,  //  1/2  			
    0,  //  3/5  			
    0,  //  2/3  			
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			                                                                                                         
    0,  //  3/5 of 8PSK		
    0,  //  2/3  			
    0,  //  3/4  			
    1,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,  //  2/3 of 16APSK
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  9/10 			
    0,  //  3/4  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,                   
    0,                
    0
};


const UINT8 c3505_cr_prs_normal_table[32][3] =                     
{
    {5, 7, 7},   // index 0, do not use                                                                                                 
    {5, 7, 7},   // 1/4 of QPSK                                                                                                 
    {5, 7, 7},   // 1/3                                                                                      
    {5, 7, 7},   // 2/5                                                                                      
    {5, 7, 7},   // 1/2                                                                                      
    {5, 7, 7},   // 3/5                                                                                      
    {5, 7, 7},   // 2/3                                                           
    {5, 7, 7},   // 3/4                                                                                      
    {5, 7, 7},   // 4/5                                                                                      
    {5, 7, 7},   // 5/6                                                                                      
    {5, 7, 7},   // 8/9                                                                                      
    {5, 7, 7},   // 9/10                                                                                                                                                                                                                                        
    {5, 7, 7},   // 3/5                                                                                      
    {5, 7, 7},   // 2/3  					
    {5, 7, 7},   // 3/4  					                                                                                   
	{5, 7, 7},   // 5/6  					                                                                     
    {5, 7, 7},   // 8/9  					                                                                     
    {5, 7, 7},   // 9/10 					                                                                     
    {5, 7, 7},   // 2/3 of 16APSK			                                                       
    {5, 7, 7},   // 3/4                                               
    {5, 7, 7},   // 4/5                                               
    {5, 7, 7},   // 5/6                                               
    {5, 7, 7},   // 9/10                                  
    {5, 7, 7},   // 3/4                                   
    {5, 7, 7},   // 5/6                                   
    {5, 7, 7},   // 8/9                                   
    {5, 7, 7},   // 5/6                                                  
    {5, 7, 7},   // 8/9                                                  
    {5, 7, 7},   // 9/10                                                 
    {5, 7, 7},                                                                                                                                       
    {5, 7, 7},                                                                                                                                       
    {5, 7, 7}              
};   

const UINT16 c3505_cr_prs_normal_snr_thres[32][2] =                     
{
    {5000,  10000},  //  index 0, do not use                                                                                                 
    {5000,  10000},  //  1/4 of QPSK  
    {5000,  10000},  //  1/3  			
    {5000,  10000},  //  2/5  			
    {5000,  10000},  //  1/2  			
    {5000,  10000},  //  3/5  			
    {5000,  10000},  //  2/3  			
    {5000,  10000},  //  3/4  			
    {5000,  10000},  //  4/5  			
    {5000,  10000},  //  5/6  			
    {5000,  10000},  //  8/9  			
    {5000,  10000},  //  9/10 			                                                                                                         
    {5000,  10000},  //  3/5 of 8PSK			
    {5000,  10000},  //  2/3  	
    {5000,  10000},  //  3/4  			
    {960,   990},  //  5/6   // For askey in brazil			
    {5000,  10000},  //  8/9  			
    {5000,  10000},  //  9/10 			
    {5000,  10000},  //  2/3 of 16APSK
    {5000,  10000},  //  3/4  			
    {5000,  10000},  //  4/5  			
    {5000,  10000},  //  5/6  			
    {5000,  10000},  //  9/10 			
    {5000,  10000},  //  3/4  			
    {5000,  10000},  //  5/6  			
    {5000,  10000},  //  8/9  			
    {5000,  10000},  //  5/6  			
    {5000,  10000},  //  8/9  			
    {5000,  10000},  //  9/10 			
    {5000,  10000},                   
    {5000,  10000},                
    {5000,  10000}                 
};   
#endif



/*****************************************************************************
*  void nim_c3505_auto_adaptive(struct nim_device *dev)
* set demap beta, it only for 8psk 2/3 normal used now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_auto_adaptive(struct nim_device *dev)
{
	//UINT8 data = 0;
    UINT32 i = 0;
    UINT8 work_mode = 0;
    UINT8 modcod = 0;
    UINT8 frame_mode = 0;
    static UINT8 modcod_last = 0;
    //static UINT8 work_mode_last = 0;
    INT16 snr = 0;
    static INT16 snr_max = 0, snr_min = 10000;
    static INT16 snr_array[8];
    UINT8 auto_adaptive_state_current = 0;
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

#ifdef ASKEY_PATCH1
	UINT8 cr_adapt_state_current = 0;
	static UINT8 cr_adapt_state_last = 0;
#endif


    auto_adaptive_state_current = priv->t_param.auto_adaptive_state;

    nim_c3505_reg_get_work_mode(dev, &work_mode);
    if (1 == work_mode) // In DVB-S2 Mode
    {
        nim_c3505_reg_get_modcod(dev, &modcod);
        if (0 == priv->t_param.auto_adaptive_cnt)
        {
            NIM_PRINTF("%s init!!\n", __FUNCTION__);
            modcod_last = modcod;
            snr_max = 0;
            snr_min = 10000;  
            for (i = 0; i < 8; i++)
                snr_array[i] = 0;
        }

        if (modcod_last != modcod)  // Modcod changed!!
        {
            priv->t_param.auto_adaptive_cnt = 0;
            NIM_PRINTF("%s found modcod changed!!\n", __FUNCTION__);
            //return SUCCESS;
        }
        else
        {
            modcod_last = modcod;

            snr = priv->channel_info->snr;

            if (0 != snr)
            {
                if (snr < 0)
                    snr = 1;
                
                snr_array[priv->t_param.auto_adaptive_cnt] = snr;
                priv->t_param.auto_adaptive_cnt++;
                
                if (snr > snr_max)
                    snr_max = snr;
                
                if (snr < snr_min)
                    snr_min = snr;   
            }
        }
            
 
        if (priv->t_param.auto_adaptive_cnt >= 8)
        {
            snr = 0;
            priv->t_param.auto_adaptive_cnt = 0;
            for (i = 0; i < 8; i++)
            {
                snr += snr_array[i];
                NIM_PRINTF("snr_array[%d] = %d\n", i, snr_array[i]);
            }
            snr = (snr - snr_max - snr_min)/6;
            NIM_PRINTF("snr_avg = %d, snr_max = %d, snr_min = %d\n", snr, snr_max, snr_min);

            frame_mode = (modcod&0x02) >> 1;
            if (0 == frame_mode)    // Only for normal frame now!
            {
                modcod = (modcod & 0x7c) >> 2;
                if (c3505_map_beta_normal_adaptive_en[modcod])
                {
                    switch (priv->t_param.auto_adaptive_state)
                    {
                        case 0: 
                            if (snr > (c3505_map_beta_normal_snr_thres[modcod][1] + SNR_MERGE))
                                auto_adaptive_state_current = 2;
                            else if (snr > (c3505_map_beta_normal_snr_thres[modcod][0] + SNR_MERGE))
                                auto_adaptive_state_current = 1; 
                            break;

                        case 1:
                            if (snr > (c3505_map_beta_normal_snr_thres[modcod][1] + SNR_MERGE))
                                auto_adaptive_state_current = 2;
                            else if (snr < (c3505_map_beta_normal_snr_thres[modcod][0] - SNR_MERGE))
                                auto_adaptive_state_current = 0;
                            break;
             
                        case 2:
                            if (snr < (c3505_map_beta_normal_snr_thres[modcod][0] - SNR_MERGE))
                                auto_adaptive_state_current = 0;
                            else if (snr < (c3505_map_beta_normal_snr_thres[modcod][1] - SNR_MERGE))
                                auto_adaptive_state_current = 1;
                            break;
                            
                        default:
                            auto_adaptive_state_current = 0;
                            
                            break;
                    }

                    NIM_PRINTF("%s, auto_adaptive_state_last = %d, auto_adaptive_state_current = %d\n", \
                        __FUNCTION__, priv->t_param.auto_adaptive_state, auto_adaptive_state_current);
                        
                    if (auto_adaptive_state_current != priv->t_param.auto_adaptive_state)
                    {
                        priv->t_param.auto_adaptive_state = auto_adaptive_state_current;
                        nim_c3505_set_map_beta_extra(dev, modcod, frame_mode, auto_adaptive_state_current);
                    }
                }
				
#ifdef ASKEY_PATCH1
				if ((c3505_cr_prs_normal_adaptive_en[modcod]) && (g_askey_patch_1_enable))
				{
					if (snr > (c3505_cr_prs_normal_snr_thres[modcod][1])) // cn>10
					{
						cr_adapt_state_current = 2;
					}
					else if (snr > (c3505_cr_prs_normal_snr_thres[modcod][0])) // 9.7<cn<10
					{
						cr_adapt_state_current = 1;
					}
					else	// cn<9.7
					{
						cr_adapt_state_current = 0;
					}

					if (cr_adapt_state_current != cr_adapt_state_last)
					{
						NIM_PRINTF("CN changed!! reinit cr parameters, cn = %d, cr_adapt_state_current = %d\n", snr, cr_adapt_state_current);
						cr_adapt_state_last = cr_adapt_state_current;
						nim_c3505_set_cr_prs_table(dev, modcod, frame_mode, cr_adapt_state_current);
					}
				}
#endif
            } 
        } 
    }
    else    // Unlock or DVB-S mode set map beta to default
    {   if (0 != priv->t_param.auto_adaptive_state)
        {
            priv->t_param.auto_adaptive_cnt = 0;
            priv->t_param.auto_adaptive_state = 0;
            
            for (i = 0; i < 32; i++) // For normal, set map beta to default
            {
                if (c3505_map_beta_normal_adaptive_en[i])
                {
                    nim_c3505_set_map_beta_extra(dev, i,  0, priv->t_param.auto_adaptive_state);     
                }
            }
            
            for (i = 0; i < 32; i++) // For short, set map beta to default
            {
                if (c3505_map_beta_short_adaptive_en[i])
                {
                    nim_c3505_set_map_beta_extra(dev, i,  1, priv->t_param.auto_adaptive_state);     
                }
            }
        }
    }
    
    return SUCCESS;
}


/*****************************************************************************
*  void nim_c3505_mon_fake_lock(struct nim_device *dev)
* set demap beta, it only for 8psk 2/3 normal used now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_mon_fake_lock(struct nim_device *dev)
{
	//UINT8 data = 0;
    //UINT32 i = 0;
    UINT8 work_mode = 0;
    UINT32 per = 0;
    
    INT16 snr = 0;
    UINT8 snr_err_flag = 0;
    static INT16 snr_last = 0;
    static UINT8 snr_err_cnt = 0;
    
    INT32 phase_err = 0;
    UINT8 phase_err_flag = 0;
    static INT32 phase_err_last = 0;
    static UINT8 phase_err_cnt = 0;

    INT32 phase_err_avg = 0;
    UINT8 phase_err_avg_flag = 0;
    static INT32 phase_err_acc = 0;

    static UINT8 snr_zero_cnt;
		
#ifdef ASKEY_PATCH1
		UINT8 modcod = 0;
		UINT8 frame_mode = 0;
#endif

	
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    snr = priv->channel_info->snr;
    per = priv->channel_info->per;
    phase_err = priv->channel_info->phase_err;

    // Wait snr != 0, than dealy 3 times to monitor
    if (0 == snr)
    {
        snr_zero_cnt = 3;
    }
    else
    {
        if (snr_zero_cnt > 0)
            snr_zero_cnt--;
        else
            snr_zero_cnt = 0;
    }

    nim_c3505_reg_get_work_mode(dev, &work_mode);
    if ((1 == work_mode) && (0 != per) && (0 == snr_zero_cnt)) // In DVB-S2 Mode and have per
    {
        // Analyze snr
        if (snr_last != snr)
        {
            if (snr < SNR_ERR_MIN)
                snr_err_cnt++;
            else
                snr_err_cnt = 0;
        }
        
        snr_last = snr; 

        if (snr_err_cnt > priv->channel_info->fake_lock_dete_thr)
            snr_err_flag = 1;


        // Analyze phase error
        if (phase_err_last != phase_err)
        {       
            if ((phase_err < PHASE_ERR_MIN)  || (phase_err > PHASE_ERR_MAX))
                phase_err_cnt++;
            else
                phase_err_cnt = 0;

            if (priv->channel_info->phase_err_stat_cnt < PHASE_ERR_AVG_NUM)
            {
                phase_err_acc += phase_err;
                priv->channel_info->phase_err_stat_cnt++;
            }
            else
            {
                phase_err_avg = phase_err_acc/PHASE_ERR_AVG_NUM;
                priv->channel_info->phase_err_stat_cnt = 0;
                phase_err_acc = 0;
            }

            if ((phase_err_avg > PHASE_ERR_AVG_ERR_MAX) || (phase_err_avg < PHASE_ERR_AVG_ERR_MIN))
                phase_err_avg_flag = 1;
        
        }
        
        phase_err_last = phase_err; 

        if (phase_err_cnt > priv->channel_info->fake_lock_dete_thr)
            phase_err_flag = 1;        

        //NIM_PRINTF("In %s, snr = %d, per = %d, phase_err = %d, snr_err_cnt = %d, snr_err_flag = %d \n phase_err_cnt = %d, phase_err_flag = %d \n phase_err_avg_flag = %d, phase_err_acc = %d, phase_err_stat_cnt = %d, phase_err_avg = %d\n", __FUNCTION__, 
        //    snr, per, phase_err, snr_err_cnt, snr_err_flag, phase_err_cnt, phase_err_flag, phase_err_avg_flag, phase_err_acc, phase_err_stat_cnt, phase_err_avg);

        NIM_PRINTF("In %s, threshold = %d, snr_err_flag = %d, phase_err_flag = %d, phase_err_avg_flag = %d\n", __FUNCTION__, priv->channel_info->fake_lock_dete_thr, snr_err_flag, phase_err_flag, phase_err_avg_flag);

        if (((snr_err_flag) || (phase_err_flag) || (phase_err_avg_flag)) && (priv->channel_info->fake_lock_rst_cnt < 2))
        {
#ifdef ASKEY_PATCH1
			nim_c3505_reg_get_modcod(dev, &modcod);
			frame_mode = (modcod&0x02) >> 1;
			 if (0 == frame_mode)	 // Only for normal frame now!
			 {
				 modcod = (modcod & 0x7c) >> 2;
				 if (0xf == modcod)
				 {
				 	NIM_PRINTF("Don't do reset while 5_6 mode fake lock for low snr\n");
				 	return SUCCESS;
				 }
			 }
#endif
            NIM_PRINTF("\n\n\n\n*************Reset demod for fake lock!**********\n\n\n\n");
            nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

            // clear state
            snr_last = 0;
            snr_err_cnt = 0;
            phase_err_last = 0;
            phase_err_cnt = 0;       
            priv->channel_info->phase_err_stat_cnt = 0;
            phase_err_acc = 0;
            priv->channel_info->fake_lock_rst_cnt++;
        }
    }
    else    // Unlock or have not per
    {
        // clear state
        snr_last = 0;
        snr_err_cnt = 0;

        phase_err_last = 0;
        phase_err_cnt = 0;       

        priv->channel_info->phase_err_stat_cnt = 0;
        phase_err_acc = 0;
        NIM_PRINTF("In %s, Unlock or dvb-s or have not per\n", __FUNCTION__);
    }
    return SUCCESS;
}



/*****************************************************************************
*  void nim_c3505_lock_unlock_adaptive(struct nim_device *dev)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_lock_unlock_adaptive(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 work_mode = 0;
    UINT8 int_data = 0;
    UINT8 freq_offset[3];
    static UINT8 unlock_cnt = 0;
    static UINT8 lock_cnt = 0; 
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    //if (0 == priv->channel_info->snr)
    //    return SUCCESS;

    nim_reg_read(dev, R04_STATUS, &data, 1);

    if (0x3f == (data&0x3f))
    { 
        // Add interrupt monitor here for find the signal mode changed
        nim_reg_read(dev, R02_IERR, &int_data, 1);
        if (0 != int_data)
        {
            // Redo a adaptive
            priv->ul_status.c3505_lock_adaptive_done = 0;
        }
    
        if (0 == priv->ul_status.c3505_lock_adaptive_done)
        {
            lock_cnt++;
            if (lock_cnt >= 1)	// Speedup the tso output when change tp, c3505_lock_adaptive_done
            {
                lock_cnt = 0;

                nim_c3505_reg_get_work_mode(dev, &work_mode);

                if (1 == work_mode)
                {
                    if(SUCCESS != nim_c3505_get_bb_header_info(dev))
                    {
                        ACM_DEBUG_PRINTF("[%s %d], Get bbheader fail!\n", __FUNCTION__, __LINE__);
                        //return ERR_CRC_HEADER;  // Here whether success or not, keep going
                    }
                }

                // PPLL adaptive and TSO CLK adaptive for ACM/CCM mode
                // Only ACM mode work in CCM TP need ppll adaptive
                // ACM mode work in ACM TP need use manual  TSO CLK
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)
                priv->nim_tso_cfg.tso_clk_cfg.is_manual_mode = 0;
#else
                if ((NIM_CCM == priv->bb_header_info->acm_ccm) || (0 == work_mode))    // CCM or DVB-S mode, auto tso clk
                {
                    nim_c3505_adapt_ppll_mode(dev);
                    
                    priv->nim_tso_cfg.tso_clk_cfg.is_manual_mode = 0;
                }
                else    // ACM mode, force a max tso clk first(next step we will set a max clk for CI card) -- paladin
                {
                    priv->nim_tso_cfg.tso_clk_cfg.is_manual_mode = 1;
                    if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
                    {
                        priv->nim_tso_cfg.tso_clk_cfg.tso_clk_src = 1;
                    }
                    else
                    {
                        priv->nim_tso_cfg.tso_clk_cfg.tso_clk_src = 3;
                    }
                    priv->nim_tso_cfg.tso_clk_cfg.clk_div_bypass= 0;
                    priv->nim_tso_cfg.tso_clk_cfg.clk_div = 0;      
                }
#endif          
                nim_c3505_set_tso_clk(dev, &(priv->nim_tso_cfg));
                // end

                nim_reg_read(dev, R62_FC_SEARCH + 0x01, &data, 1);
                if (0x80 == (data&0x80))
                {
                    // Close fc search
                    data &= 0x7f;
                    nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);

                    // Set rpt fc offset to acq offset
                    nim_reg_read(dev, R5C_ACQ_CARRIER, freq_offset, 3);
                    freq_offset[2] &= 0x01;
                    NIM_PRINTF("%s, ACQ fc offset = 0x%2x%2x%2x\n", __FUNCTION__, freq_offset[2], freq_offset[1], freq_offset[0]);
                    nim_reg_read(dev, R69_RPT_CARRIER, freq_offset, 3);
                    freq_offset[2] &= 0x01;
                    NIM_PRINTF("%s, RPT fc offset = 0x%2x%2x%2x\n", __FUNCTION__, freq_offset[2], freq_offset[1], freq_offset[0]);
                    nim_reg_write(dev, R5C_ACQ_CARRIER, freq_offset, 3);
                    NIM_PRINTF("%s, close fc search while tp lock\n", __FUNCTION__);

                    if ((0 == work_mode) || (1 == work_mode))
                    {
                        nim_c3505_set_work_mode(dev, work_mode);
                        NIM_PRINTF("%s, force work mode to %d, while tp lock\n", __FUNCTION__, work_mode);    
                    }
                }

                // For low symbol rate tp lock speedup, 07/14/2017 hyman
                //data = 0x00;
                //nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1); 

				data = 0;
				nim_reg_write(dev, R02_IERR, &data, 1);
				priv->ul_status.c3505_lock_adaptive_done = 1;
            }
        }
        unlock_cnt = 0;
    }
    else
    {
        // Since Tp goes unlock we need to open PPLL immediate
        nim_c3505_open_ppll(dev);
        priv->ul_status.c3505_lock_adaptive_done = 0;
        lock_cnt = 0;

        // FC adaptive need to keep a little time to fast lock recover when tp unlock by adjust the cn on sfu
        nim_reg_read(dev, R62_FC_SEARCH + 0x01, &data, 1);
        if (0x00 == (data&0x80))
        {
            unlock_cnt++;
            if (unlock_cnt > 12)
            {
                unlock_cnt = 0;

                // Open fc search
                nim_c3505_set_freq_offset(dev, priv->t_param.freq_offset);
                data |= 0x80;
                nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
                nim_c3505_set_work_mode(dev, 3);
                NIM_PRINTF("%s, open fc search and auto work mode while tp unlock, priv->t_param.freq_offset = %d\n", __FUNCTION__, priv->t_param.freq_offset);

                // For low symbol rate tp lock speedup, 07/14/2017 hyman
                //data = 0x70;
                //nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1); 
            }
        } 
    }
    
    return SUCCESS;
}



#ifdef ASKEY_PATCH1
/*****************************************************************************
*  INT32 nim_c3505_askey_patch_in_set_tp(struct nim_device *dev, UINT8 enable, struct nim_c3505_tp_scan_para *tp_scan_param)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 enable, 1: enable, 0: disable
*  Parameter3: struct nim_c3505_tp_scan_para *tp_scan_param

* Return Value: none 
*****************************************************************************/
INT32 nim_c3505_askey_patch_in_set_tp(struct nim_device *dev, UINT8 enable, struct nim_c3505_tp_scan_para *tp_scan_param)
{
	UINT8 data = 0;
	static UINT8 cr98_bak = 0x00;
	struct nim_c3505_private* dev_priv;

    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	dev_priv = dev->priv;


	NIM_PRINTF("Run in %s, enable = %d\n", __FUNCTION__, enable);

	if (1 == enable)
	{
		// Tuner BW
		dev_priv->nim_tuner_control(dev_priv->tuner_index, tp_scan_param->tuner_freq, 10000); // Force tuner bw = 7


		// DSP CLK
		nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[0]); // Force DSK clk to 135m


		// AGC1 Ref
		data = 0xb0;
		nim_reg_write(dev, R07_AGC1_CTRL, &data, 1);
	}

	// AGC2 Ref -- set it inASKEY_PATCH3
	// TR -- set it in ASKEY_PATCH3
	// CR -- set it in ASKEY_PATCH3
	// Mapbeta -- set it in nim_c3505_auto_adaptive

	// LDPC RWR 
	nim_reg_read(dev, R12d_LDPC_SEL, &data, 1);
	if (CHIP_ID_3503D != dev_priv->ul_status.m_c3505_type)
	{	
		if (1 == enable)
		{
			data &= 0xdf;	// for 3505/3503C only
		}
		else
		{
			data |= 0x20;	// for 3505/3503C only
		}
	}
	else
	{
		data |= 0x02;	// for 3503d only
	}
	nim_reg_write(dev, R12d_LDPC_SEL, &data, 1);


	// LDPC Force unlock
	if (0x00 == cr98_bak)
	{
		nim_reg_read(dev, R98_S2_FEC_FAIL_THR, &cr98_bak, 1);
	}
	
	if (1 == enable)
	{
		data = 0xff;
	}
	else
	{	
		data = cr98_bak;
	}
	nim_reg_write(dev, R98_S2_FEC_FAIL_THR, &data, 1);


	return SUCCESS;

}

	
/*****************************************************************************
*  INT32 nim_c3505_set_cr_prs_table(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 auto_adaptive_state_current)
* set demap beta, it only for 8psk 2/3 normal used now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
*  Parameter4: UINT8 state
* Return Value:  
*****************************************************************************/
INT32 nim_c3505_set_cr_prs_table(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 state)
{
	UINT8 data;
	UINT32 sym = 0;

	NIM_PRINTF("%s, index = %d, frame_mode = %d, state = %d\n", __FUNCTION__, index, frame_mode, state);

	UINT8 TabID,CellID;
	UINT16 TabVal_14_0,TavVal_29_15;
	//UINT8 data;
	UINT8 DatArray[2];
	
	UINT16 S2_AWGN_NOISE_tmp;
	UINT8 S2_AWGN_COEF_SET_tmp;
	UINT8 S2_PON_IRS_DELTA_tmp;
	UINT8 S2_PON_PRS_DELTA_tmp;
	UINT8 S2_LOOP_COEF_SET_tmp;
	UINT8 S2_CLIP_PED_SET_tmp;
	UINT8 S2_AVG_PED_SET_tmp;
	UINT8 S2_FORCE_OLD_CR_tmp;
	
	UINT8 IRS_tmp;
	UINT8 IRS_FRACTION_tmp;
	UINT8 PRS_tmp;
	UINT8 PRS_FRACTION_tmp;
	UINT8 HEAD_PED_GAIN_tmp;
	UINT8 SYMBOL_BEFORE_HEAD_tmp;
	UINT32 S2_CODE_RATE_TABLE_tmp;
	
	//8psk 5/6
	TabID =  0;
	CellID = 14;  

	if(frame_mode == 1)//short
	{
		
	}
	else if(frame_mode == 0)//normal
	{
		if (0 == state) //askey 8psk R5_6 pon issue solution_2 {prs,frac}=5.1 CN<9.7
		{
			S2_AWGN_NOISE_tmp	 = 0x1ff;//0x8c
			S2_AWGN_COEF_SET_tmp = 0x7;
			S2_PON_IRS_DELTA_tmp = 0x1;
			S2_PON_PRS_DELTA_tmp = 0x1;
			S2_LOOP_COEF_SET_tmp = 0x0;
			S2_CLIP_PED_SET_tmp  = 0x0;
			S2_AVG_PED_SET_tmp   = 0x6;
			S2_FORCE_OLD_CR_tmp  = 0x0;
		}
		else if (1 == state) //askey 8psk R5_6 pon issue solution_2 {prs,frac}=6.1 9.7<CN<10
		{
			S2_AWGN_NOISE_tmp	 = 0x1ff;//0x8c
			S2_AWGN_COEF_SET_tmp = 0x7;
			S2_PON_IRS_DELTA_tmp = 0x0;
			S2_PON_PRS_DELTA_tmp = 0x0;
			S2_LOOP_COEF_SET_tmp = 0x0;
			S2_CLIP_PED_SET_tmp  = 0x0;
			S2_AVG_PED_SET_tmp   = 0x6;
			S2_FORCE_OLD_CR_tmp  = 0x0;
		}
		else //default CN>10
		{
			nim_c3505_get_symbol_rate(dev, &sym);
			
			S2_AWGN_NOISE_tmp	 = c3505_s2_awgn_noise_extra[CellID];
			S2_AWGN_COEF_SET_tmp = c3505_s2_awgn_coef_set_extra[CellID];
		 	S2_PON_IRS_DELTA_tmp = c3505_s2_pon_irs_delta_extra[CellID];
		 	S2_PON_PRS_DELTA_tmp = c3505_s2_pon_prs_delta_extra[CellID];
			S2_LOOP_COEF_SET_tmp = c3505_s2_loop_coef_extra[CellID];
		 	S2_CLIP_PED_SET_tmp  = c3505_s2_clip_ped_set_extra[CellID];
		 	S2_AVG_PED_SET_tmp   = c3505_s2_avg_ped_set_extra[CellID];
		 	S2_FORCE_OLD_CR_tmp  = c3505_s2_force_old_cr[CellID];

			if ((sym>6500) && (sym < 16000))
			{
				S2_AWGN_COEF_SET_tmp = 0x7;
		  		S2_LOOP_COEF_SET_tmp = 0x6;
			}
		}

		S2_CODE_RATE_TABLE_tmp=((((S2_AWGN_NOISE_tmp)<<1) & 0x00000ffe)<<20) | // why?
			   ((S2_AWGN_COEF_SET_tmp & 0x00000007)<<17) | 
			   ((S2_PON_IRS_DELTA_tmp & 0x00000007)<<14) | 
			   ((S2_PON_PRS_DELTA_tmp & 0x00000007)<<11) | 
			   ((S2_LOOP_COEF_SET_tmp & 0x00000007)<<8) | 
			   ((S2_CLIP_PED_SET_tmp & 0x00000007)<<5) | 
			   ((S2_AVG_PED_SET_tmp & 0x00000007)<<2) | 
			   ((S2_FORCE_OLD_CR_tmp & 0x00000001)<<1) | 
			   (c3505_s2_llr_shift[CellID] & 0x00000001);
		//step 1: 
		data = (TabID << 5) | (CellID & 0x1f);
		nim_reg_write(dev,R9D_RPT_DEMAP_BETA,&data,1 );
		//step 2:
		TabVal_14_0 = (UINT16)((S2_CODE_RATE_TABLE_tmp	& 0x7fff)); // and disable the CR_PARA_WE to avoid disturb with seen's adpt
		DatArray[0] = (UINT8)TabVal_14_0;
		DatArray[1] = ((UINT8)(TabVal_14_0>>8)) & 0x7f;
		nim_reg_write(dev,R11_DCC_OF_I, DatArray, 2);

		//step 3:
		TavVal_29_15 = (UINT16)(S2_CODE_RATE_TABLE_tmp>>15);
		DatArray[0] = (UINT8)(TavVal_29_15);
		DatArray[1] = (UINT8)(TavVal_29_15>>8) | 0x80;	// enable CR_PARA_WE_2
		nim_reg_write(dev,R130_CR_PARA_DIN,DatArray,2);
	}

	return SUCCESS;
	
}

#endif



