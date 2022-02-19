/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 get hardware status function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/


#include "nim_dvbs_c3505.h"

static UINT32  va_ber        = 0;
static UINT32  va_ber_window = 0;
static UINT32  va_mer        = 0;
static UINT32  va_mer_window = 0;
static UINT32  c3505_m_mer   = 0;


/*****************************************************************************
* INT32 nim_c3505_get_fast_lock(struct nim_device *dev, UINT8 *lock)
*
*  Get DVB-S2 PL lock or DVB-S TP lock for fast lock detection
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *lock
*
*Return Value: INT32
*****************************************************************************/
void nim_c3505_get_fast_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data = 0;

	comm_delay(150);

	nim_reg_read(dev, R04_STATUS, &data, 1);

	if (((data & 0x57) == 0x57) || ((data & 0x3f) == 0x3f))
	{
		*lock = 1;
	}
	else
	{
		*lock = 0;
	}
}

/*****************************************************************************
*  INT32 nim_C3505_reg_get_chip_type(struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_reg_get_chip_type(struct nim_device *dev)
{
	UINT8 temp[4] ={ 0x00, 0x00, 0x00, 0x00 };
	UINT32 m_Value = 0x00;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	priv->ul_status.m_c3505_type = 0x00;
	nim_reg_read(dev, RA3_CHIP_ID, temp, 4);
	m_Value = temp[1];
	m_Value = (m_Value << 8) | temp[0];
	m_Value = (m_Value << 8) | temp[3];
	m_Value = (m_Value << 8) | temp[2];
	priv->ul_status.m_c3505_type = m_Value;
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3505_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *lock
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data = 0;
    UINT8 cnt = 0;
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	comm_delay(150);

    if (CHIP_ID_3505 != priv->ul_status.m_c3505_type)
    {
    	nim_reg_read(dev, R04_STATUS, &data, 1);
    	if ((data & 0x3f) == 0x3f)
    	{
    		*lock = 1;
    	}
    	else
    	{
    		*lock = 0;
    	}
    }
    else
    {
        // A new way to get lock for c3505 in ACM work mode
        nim_reg_read(dev, R04_STATUS, &data, 1);
        if ((data & 0x3f) == 0x3f)
        {
            nim_c3505_reg_get_work_mode(dev, &data);
            if (1 == data)  // DVB-S2 mode
            {
                nim_reg_read(dev, R97_S2_FEC_THR, &data, 1);
                if (1 == data)  // While fec lock threshold is 1
                {
                    while(1)
                    {
                        cnt++;
                                        
                        if (50 == cnt)
                        {
                            *lock = 0;
                            break;
                        }
                        
                        nim_reg_read(dev, RAC_S2_FEC_RPT, &data, 1);
                        //NIM_PRINTF("RAC_S2_FEC_RPT = 0x%x\n", data);
                        if(0x7f != data)
                        {
                        	//NIM_PRINTF("RAC_S2_FEC_RPT = 0x%x\n", data);
                            *lock = 1;
                            break;
                        }
                        
          
                        comm_sleep(1);
                    }     
                }
                else
                {
                	//NIM_PRINTF("CCM LOCK\n");
                    *lock = 1;
                }
            }
            else if (0 == data)  // DVB-S mode
            {
            	//NIM_PRINTF("DVB-S LOCK\n");
                *lock = 1;
            }
            else // ERR
            {
                *lock = 0;
            }
        }
        else
        {
            *lock = 0;
        }
    }

    comm_delay(150);

    return SUCCESS;

}



/*****************************************************************************
*  INT32 nim_C3505_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
* Description: Get tuner lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *tun_lock
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	//UINT8 data;

	/* Setup tuner */
	NIM_MUTEX_ENTER(priv);
	priv->nim_tuner_status(priv->tuner_index, tun_lock);
	NIM_MUTEX_LEAVE(priv);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_C3505_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag)
* Get I/Q swap indication
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *iqswap_flag
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag)
{
	UINT8 data;
	nim_reg_read(dev, R6C_RPT_SYM_RATE + 0x02, &data, 1);
	*iqswap_flag = ((data >> 4) & 0x01);
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_C3503_get_demod_gain(struct nim_device *dev, UINT8 *agc)
*
*	This function will get the demod agc that feed to tuner
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_demod_gain(struct nim_device *dev, UINT8 *agc)
{	 
	nim_reg_read(dev, R0A_AGC1_LCK_CMD + 0x01, agc, 1);
	//MON_PRINTF("demod get agc gain value %d\n",*agc);
	return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_c3503_get_tuner_gain(struct nim_device *dev,UINT8 agc, INT32 *agc_tuner)
*
*	This function will get the tuner total gain
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*	Parameter3: UINT16*agc_tuner
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_tuner_gain(struct nim_device *dev,UINT8 agc, INT32 *agc_tuner)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	if(priv->tuner_type == IS_M3031)
	{		
		if (priv->nim_tuner_gain != NULL)
		{
			*agc_tuner = priv->nim_tuner_gain(priv->tuner_index,agc);
		}
		else
		{
			//MON_PRINTF("priv->nim_tuner_gain == NULL\n");
			*agc_tuner = 0;
		}
	}

	//MON_PRINTF("demod get tuner gain value %d\n",*agc_tuner);
	return SUCCESS;
}
/*****************************************************************************
* static INT32 nim_C3505_get_dBm_level(struct nim_device *dev,UINT8 agc_demod, INT32 agc_tuner,INT8 *level_db)
*
*	This function will acount the signal intensity with dBm unit
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_dbm_level(struct nim_device *dev, UINT8 agc_demod, INT32 agc_tuner,INT8 *level_db)
{
	INT32 level_db_tmp ;
	INT8 agc_demod_tmp = 0 ;
    INT32 agc_gain_flag = 0;    //  Bit[0] = mix_lgain_status , bit[1] =  rf_coarse_gain_status, bit[2] =  dgb_gain_status, used to choose the formula for dBm calculate 
    UINT8 mix_lgain_flag = 0, rf_coarse_gain_flag = 0, dgb_gain_flag = 0;
	struct ali_nim_agc nim_agc;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	if(priv->tuner_type == IS_M3031)
	{		
        // Add by paladin 2015/10/22 for agc dbm linear indication
        // Get tuner gain status
        call_tuner_command(dev, NIM_TUNER_GET_C3031B_GAIN_FLAG, &agc_gain_flag);

        // Bit[0] =  mix_lgain status
        if(agc_gain_flag & 0x00000001)
            mix_lgain_flag = 1; // open
        else
            mix_lgain_flag = 0; // closed

        // Bit[1] =  rf_coarse_gain status
        if(agc_gain_flag & 0x00000002)
            rf_coarse_gain_flag = 1;    // saturation
        else
            rf_coarse_gain_flag = 0;    // not saturation

        // Bit[2] =  dgb_gain status
        if(agc_gain_flag & 0x00000004)
            dgb_gain_flag = 1;          // saturation
        else
            dgb_gain_flag = 0;          // not saturation

        // rf_coarse_gain not saturation
        if(0 == rf_coarse_gain_flag)
        {
            level_db_tmp = (-(agc_tuner + 134)/267 - 29);
        }
        else
        {
            // rf_coarse_gain saturation  && mix_lgain closed
            if(0 == mix_lgain_flag)  
            {
                level_db_tmp = (-(agc_tuner + 25)/245 - 29);
            }
            //  rf_coarse_gain saturation  && mix_lgain open
            else
            {
                if(agc_tuner > 14000)
                    level_db_tmp = -96;
                else
                    
                //  rf_coarse_gain saturation  && mix_lgain open && dgb_gain saturation 
                if(dgb_gain_flag)   
                    level_db_tmp = (-(agc_tuner + 171)/190 - 20);
                else
                //  rf_coarse_gain saturation  && mix_lgain open && dgb_gain not saturation 
                    level_db_tmp = (-(agc_tuner + 208)/259 - 29);
            }
        }
        //end

		if(level_db_tmp > 0)	// agc with the lowest gain,set 0dBm
			level_db_tmp = 0;
	}
	else
	{	
		if (priv->nim_tuner_command != NULL)
		{
			priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_GET_RF_POWER_LEVEL, (INT32 *)&nim_agc);			

			level_db_tmp = nim_agc.rf_level;			
		}
		else
		{			
			if(agc_demod < 128)
				agc_demod_tmp = agc_demod;
			else
				agc_demod_tmp = agc_demod-256 ;

		if(agc_demod_tmp > 83) 
			level_db_tmp = - 8;
		else if(agc_demod_tmp > 80) 
			level_db_tmp = agc_demod_tmp*2 -76 ;
		else if(agc_demod_tmp > 65)
			level_db_tmp = agc_demod_tmp - 96; 
		else if(agc_demod_tmp > 30)
			level_db_tmp =  agc_demod_tmp*83/100 -87 ;
		else if(agc_demod_tmp > 0) 	 
			level_db_tmp = agc_demod_tmp/2 -74 ;
		else if(agc_demod_tmp > -100)		 
			level_db_tmp = agc_demod_tmp/4 -77 ;
		else	 
			level_db_tmp = -96 ;

			if(level_db_tmp < -96) // agc with the highest gain,set -96dBm
			level_db_tmp = -96 ;
		}
	}

	if(level_db_tmp & 0x80000000)
		*level_db = (INT8)level_db_tmp|0x80;
	else
		*level_db = (INT8)level_db_tmp ;
		
	//MON_PRINTF("level_db = %d\n",*level_db);

	return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_C3503_get_AGC_filter(struct nim_device *dev, INT8 *agc)
*
*	This function will make the agc value smoothly
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
#define   s_filter_len  2  //even is better.

INT32 nim_c3505_get_agc_filter(struct nim_device *dev, INT8 *agc)
{
	INT32 s_sum = 0; 
	int k,n1,n2;
	static int j = 0;
	static INT8   s_data[15]={0};	
	static INT8   s_agc=0x00; 

	if(*agc == -96) j = 0;
	
	if (j == (s_filter_len - 1)) 
	{		
		for (k=0; k<(s_filter_len-1); k++)
		{
			s_sum=s_sum+s_data[k];
		}
		s_sum = *agc + s_sum;
		*agc = s_sum / s_filter_len;
		n1 = s_sum-(*agc) * s_filter_len;
		if(n1 >= (s_filter_len / 2))
			n1 = 1;
		else
			n1=0;
		
		*agc = *agc + n1;			
		s_agc = *agc;
		j = j + 1;
	}
	else if (j==s_filter_len)
	{
		n1 = (s_agc * (s_filter_len - 1)) / s_filter_len;
		n1 = (s_agc * (s_filter_len-1)) - n1 * s_filter_len;
		n2 = *agc / s_filter_len;
		n2 = *agc-n2 * s_filter_len;
		n2 = n1 + n2;
		if( n2 >= (3*(s_filter_len / 2)))
			n1 = 2;
		else if((n2<(3*(s_filter_len / 2))) && (n2 >= (s_filter_len/2)))
			n1 = 1;
		else
			n1 = 0;		   

		*agc = (s_agc * (s_filter_len - 1)) / s_filter_len + *agc / s_filter_len + n1;
		s_agc = *agc;
	}
	else 
	{
		s_data[j] = *agc;
		*agc = *agc;
		j = j + 1;
	} 
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3503_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*	This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_agc(struct nim_device *dev, UINT8 *agc)
{
	UINT8 agc_demod = 0 ;
	INT32 agc_tuner = 0 ;
	INT8 level_db_tmp = 0 ;	

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_get_demod_gain(dev,&agc_demod);

	if(priv->tuner_type == IS_M3031)
	{	 
		nim_c3505_get_tuner_gain(dev,agc_demod, &agc_tuner);
		//MON_PRINTF("tuner agc total gain value %d\n",agc_tuner);
	}

	nim_c3505_get_dbm_level(dev,agc_demod,agc_tuner,&level_db_tmp);	 
	//MON_PRINTF("-----------------agc gain dBm value %d\n",level_db_tmp);
	nim_c3505_get_agc_filter(dev,&level_db_tmp);	 
	//MON_PRINTF("-----------------dBm value after filter %d\n\n",level_db_tmp);

	if (level_db_tmp > 0) //limit the range from -96----0
	{
		level_db_tmp = 0;
	}
	else if (level_db_tmp < -96)
	{
		level_db_tmp = -96;
	}
			
	*agc = (UINT8)(level_db_tmp+100);

return SUCCESS;
}
	 

/*****************************************************************************
* INT32 nim_C3505_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*	This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_agc_dbm(struct nim_device *dev, INT8 *agc_dbm)
{
	UINT8 agc_demod = 0 ;
	INT32 agc_tuner = 0 ;
	INT8 level_db_tmp = 0 ;	

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_get_demod_gain(dev,&agc_demod);

	if(priv->tuner_type == IS_M3031)
	{	 
		nim_c3505_get_tuner_gain(dev,agc_demod, &agc_tuner);
		//MON_PRINTF("tuner agc total gain value %d\n",agc_tuner);
	}

	nim_c3505_get_dbm_level(dev,agc_demod,agc_tuner,&level_db_tmp);	 
	//MON_PRINTF("-----------------agc gain dBm value %d\n",level_db_tmp);
	nim_c3505_get_agc_filter(dev,&level_db_tmp);	 
	//MON_PRINTF("-----------------dBm value after filter %d\n\n",level_db_tmp);

	*agc_dbm = level_db_tmp;

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
*  Get symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
	UINT8 data[3];//, i;
	UINT32 sample_rate = 0;
	//UINT32 temp = 0;
	UINT32 symrate = 0;

	//temp = 0;
	nim_reg_read(dev, R6C_RPT_SYM_RATE, data, 3);
	symrate = data[0] + (data[1] <<8) + ((data[2]&0x01) << 16); 
	nim_c3505_get_dsp_clk(dev, &sample_rate);
	sample_rate = sample_rate/2;
	*sym_rate = nim_c3505_multu64div(symrate, sample_rate, 92160);

	return SUCCESS;
}
 
/*****************************************************************************
* INT32 nim_C3505_get_freq(struct nim_device *dev, INT32 cmd, UINT32 param)
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 *freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_freq(struct nim_device *dev, UINT32 *freq)
{
	INT32 freq_off = 0;
	UINT8 data[3] = {0};
	UINT32 sample_rate = 0;
	UINT32 tdata = 0;
	UINT32 temp = 0;

	nim_reg_read(dev, R69_RPT_CARRIER, data, 3);
	tdata = (data[0]&0xff)  + ((data[1]&0xff) <<8);

	if ((data[2] & 0x01) == 1)
		temp = (tdata ^ 0xffff) + 1;
	else
		temp = tdata & 0xffff;

	nim_c3505_get_dsp_clk(dev, &sample_rate);
	if ((data[2] & 0x01) == 1)
		freq_off = 0 - nim_c3505_multu64div(temp, sample_rate, 90000);
	else
        freq_off = nim_c3505_multu64div(temp, sample_rate, 90000);

	*freq += freq_off;

	return SUCCESS;
}
 
  /*****************************************************************************
 *  INT32 nim_C3505_get_tune_freq(struct nim_device *dev, INT32 *freq)
 * Get bit error ratio
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT16* RsUbc
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3505_get_tune_freq(struct nim_device *dev, INT32 *freq)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	*freq += priv->ul_status.m_step_freq;

	if (*freq <= 0)
		*freq = 30;
	else if (*freq > 70)
		*freq = 70;

	//NIM_PRINTF("  tune *freq step is %d\n", *freq );
	return SUCCESS;
}
  
/*****************************************************************************
*  INT32 nim_C3505_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
*  Get work mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *work_mode
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
{
	UINT8 data = 0;

	//  NIM_PRINTF("Enter Fuction nim_C3505_reg_get_work_mode \n");
	nim_reg_read(dev, R68_WORK_MODE, &data, 1);
	*work_mode = data & 0x03;
	return SUCCESS;
	//  Work Mode
	//	  0x0:	  DVB-S
	//	  0x1:	  DVB-S2
	//	  0x3:	  DVB-S2 HBC
}
/*****************************************************************************
* INT32 nim_C3505_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
*  Get code rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *code_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
{
	UINT8 data;
	//  NIM_PRINTF("Enter Fuction nim_C3505_reg_get_code_rate \n");
	nim_reg_read(dev, R69_RPT_CARRIER + 0x02, &data, 1);
	*code_rate = ((data >> 1) & 0x0f);

	return SUCCESS;

//  Code rate list
//  for DVBS:
//	  0x0:	  1/2,
//	  0x1:	  2/3,
//	  0x2:	  3/4,
//	  0x3:	  5/6,
//	  0x4:	  6/7,
//	  0x5:	  7/8.
//  For DVBS2 :
//	  0x0:	  1/4 ,
//	  0x1:	  1/3 ,
//	  0x2:	  2/5 ,
//	  0x3:	  1/2 ,
//	  0x4:	  3/5 ,
//	  0x5:	  2/3 ,
//	  0x6:	  3/4 ,
//	  0x7:	  4/5 ,
//	  0x8:	  5/6 ,
//	  0x9:	  8/9 ,
//	  0xa:	  9/10.
}

 /*****************************************************************************
 *  INT32 nim_C3505_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
 * Get map type
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 *map_type
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3505_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
{
	UINT8 data = 0;
	// NIM_PRINTF("Enter Fuction nim_C3505_reg_get_map_type \n");
	nim_reg_read(dev, R69_RPT_CARRIER + 0x02, &data, 1);
	*map_type = ((data >> 5) & 0x07);

	return SUCCESS;
	//  Map type:
	// 	 0x0:	 HBCD.
	// 	 0x1:	 BPSK
	// 	 0x2:	 QPSK
	// 	 0x3:	 8PSK
	// 	 0x4:	 16APSK
	// 	 0x5:	 32APSK
}

/*****************************************************************************
*  INT32 nim_c3505_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
* Get roll off 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *roll_off
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
{
	UINT8 data = 0;
	//  NIM_PRINTF("Enter Fuction nim_C3505_reg_get_roll_off \n");
	nim_reg_read(dev, R6C_RPT_SYM_RATE + 0x02, &data, 1);
	*roll_off = ((data >> 5) & 0x03);

	return SUCCESS;

	//  DVBS2 Roll off report
	// 	 0x0:	 0.35
	// 	 0x1:	 0.25
	// 	 0x2:	 0.20
	// 	 0x3:	 Reserved
}
 
/*****************************************************************************
*  INT32 nim_C3505_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
* Get mod-code
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *modcod
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
{
	UINT8 data = 0;
	//  NIM_PRINTF("Enter Fuction nim_C3505_reg_get_modcode \n");
	nim_reg_read(dev, RF8_MODCOD_RPT, &data, 1);
	*modcod = data & 0x7f;
	return SUCCESS;
	// bit0 : pilot on/off
	// bit[6:1] : modcod,
}

/*****************************************************************************
*  INT32 nim_C3505_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 Rs, UINT32 *bit_rate)
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 work_mode
*	Parameter3: UINT8 map_type
*	Parameter4: UINT8 code_rate
*	Parameter5: UINT32 Rs
*	Parameter6: UINT32 *bit_rate
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_c3505_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 rs, UINT32 *bit_rate)
{
	UINT32 data = 0;
	UINT32 temp = 0;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	if (work_mode != C3505_DVBS2_MODE) // DVBS mode
	{
		if (code_rate == 0)
			temp = (rs * 2 * 1 + 1000) / 2000;
		else if (code_rate == 1)
			temp = (rs * 2 * 2 + 1500) / 3000;
		else if (code_rate == 2)
			temp = (rs * 2 * 3 + 2000) / 4000;
		else if (code_rate == 3)
			temp = (rs * 2 * 5 + 3000) / 6000;
		else
			temp = (rs * 2 * 7 + 4000) / 8000;

		if (temp > 254)
			data = 255;
		else
			data = temp + 1;	 // add 1 M for margin

		*bit_rate = data;
		NIM_PRINTF("xxx dvbs bit_rate is %d \n", *bit_rate);
		return SUCCESS;
	}
	else	 //DVBS2 mode
	{
		if (code_rate == 0)
			temp = (rs * 1 + 2000) / 4000;
		else if (code_rate == 1)
			temp = (rs * 1 + 1500) / 3000;
		else if (code_rate == 2)
			temp = (rs * 2 + 2500) / 5000;
		else if (code_rate == 3)
			temp = (rs * 1 + 1000) / 2000;
		else if (code_rate == 4)
			temp = (rs * 3 + 2500) / 5000;
		else if (code_rate == 5)
			temp = (rs * 2 + 1500) / 3000;
		else if (code_rate == 6)
			temp = (rs * 3 + 2000) / 4000;
		else if (code_rate == 7)
			temp = (rs * 4 + 2500) / 5000;
		else if (code_rate == 8)
			temp = (rs * 5 + 3000) / 6000;
		else if (code_rate == 9)
			temp = (rs * 8 + 4500) / 9000;
		else
			temp = (rs * 9 + 5000) / 10000;

		if (map_type == 2)
			temp = temp * 2;
		else if (map_type == 3)
			temp = temp * 3;
		else if (map_type == 4)
			temp = temp * 4;
		else
		{
			NIM_PRINTF("Map type error: %02x \n", map_type);
		}

		if ((priv->tuner_config_data.qpsk_config & C3505_USE_188_MODE) == C3505_USE_188_MODE)
		{
			temp = temp;
		}
		else
		{
			temp = (temp * 204 + 94) / 188;
		}

		if (temp > 200)
			data = 200;
		else
			data = temp;

		NIM_PRINTF("Code rate is: %02x \n", code_rate);
		NIM_PRINTF("Map type is: %02x \n", map_type);

		data += 1; // Add 1M
		*bit_rate = data;
		NIM_PRINTF("xxx dvbs2 bit_rate is %d \n", *bit_rate);
		return SUCCESS;
	}
}

/*****************************************************************************
* INT32 nim_C3505_get_bitmode(struct nim_device *dev, UINT8 *bitMode)
* Get bit error ratio
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_bitmode(struct nim_device *dev, UINT8 *bitMode)
{
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_1BIT_MODE)
		*bitMode = 0x60;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_2BIT_MODE)
		*bitMode = 0x00;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_4BIT_MODE)
		*bitMode = 0x20;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3505_8BIT_MODE)
		*bitMode = 0x40;
	else
		*bitMode = 0x40;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_get_BER(struct nim_device *dev, UINT32 *RsUbc)
* Get bit error ratio
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_ber(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 data = 0;
	UINT8 ber_data[3] = {0};
	UINT32 u32ber_data[3] = {0};
	UINT32 uber_data = 0;

	struct nim_c3505_private *priv = NULL;

	if((dev == NULL) || (rs_ubc == NULL))
		return RET_FAILURE;

	priv = (struct nim_c3505_private *) dev->priv;

	//CR78
	nim_reg_read(dev, R76_BIT_ERR + 0x02, &data, 1);
	if (0x00 == (0x80 & data))
	{
		// 	 NIM_PRINTF( "CR78= %x\n", data);
		//CR76
		nim_reg_read(dev, R76_BIT_ERR, &ber_data[0], 1);
		u32ber_data[0] = (UINT32) ber_data[0];
		//CR77
		nim_reg_read(dev, R76_BIT_ERR + 0x01, &ber_data[1], 1);
		u32ber_data[1] = (UINT32) ber_data[1];
		u32ber_data[1] <<= 8;
		//CR78
		nim_reg_read(dev, R76_BIT_ERR + 0x02, &ber_data[2], 1);
		u32ber_data[2] = (UINT32) ber_data[2];
		u32ber_data[2] <<= 16;

		uber_data = u32ber_data[2] + u32ber_data[1] + u32ber_data[0];

		uber_data *= 100;
		uber_data /= 1632;
		*rs_ubc = uber_data;
		priv->ul_status.old_ber = uber_data;
		//CR78
		data = 0x80;
		nim_reg_write(dev, R76_BIT_ERR + 0x02, &data, 1);
	}
	else
	{
		*rs_ubc = priv->ul_status.old_ber;
	}

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_get_new_BER(struct nim_device *dev, UINT32 *ber)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *ber
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_new_ber(struct nim_device *dev, UINT32 *ber)
{
	UINT8 data = 0;
	UINT32 t_count = 0, myber = 0;

	for (t_count = 0; t_count < 200; t_count++)
	{
		nim_reg_read(dev, R76_BIT_ERR + 0x02, &data, 1);
		if ((data & 0x80) == 0)
		{
			myber = data & 0x7f;
			nim_reg_read(dev, R76_BIT_ERR + 0x01, &data, 1);
			myber <<= 8;
			myber += data;
			nim_reg_read(dev, R76_BIT_ERR, &data, 1);
			myber <<= 8;
			myber += data;
			break;
		}
	}
	*ber = myber;

	return SUCCESS;
}
  
/*****************************************************************************
* INT32 nim_C3505_get_PER(struct nim_device *dev, UINT32 *RsUbc)
* Get packet error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_per(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 per[2];
	UINT16 percount;
	UINT8 data;

	*rs_ubc = 1010;
	nim_reg_read(dev, R04_STATUS, &data, 1);
	if (0x00 != (0x20 & data))
	{
		nim_reg_read(dev, R79_PKT_ERR + 0x01, &per[1], 1);
		per[1] = per[1] & 0x7f;
		nim_reg_read(dev, R79_PKT_ERR, &per[0], 1);
		percount = (UINT16) (per[1] * 256 + per[0]);
		*rs_ubc = (UINT32) percount;
        //if (0 != percount)
        //    NIM_PRINTF("************ Find PER ************\n");
        
    	//NIM_PRINTF("current PER is  %d\n", percount);

		nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
		data=0x7f&data;
		nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);
		comm_delay(10);
		nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
		data &= 0x3f;
		data |= 0x80;
		nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);

		return SUCCESS;
	}
	else
	{
		return ERR_PARA;
	}
}
 
/*****************************************************************************
* INT32 nim_C3505_get_fer(struct nim_device *dev, UINT32 *RsUbc)
* Get frame error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_fer(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 per[2];
	UINT16 percount;
	UINT8 data;

	*rs_ubc = 1010;
	nim_reg_read(dev, R04_STATUS, &data, 1);
	if (0x00 != (0x20 & data))
	{
		nim_reg_read(dev, R79_PKT_ERR + 0x01, &per[1], 1);
		per[1] = per[1] & 0x7f;
		nim_reg_read(dev, R79_PKT_ERR, &per[0], 1);
		percount = (UINT16) (per[1] * 256 + per[0]);
		*rs_ubc = (UINT32) percount;
        //if (0 != percount)
        //    NIM_PRINTF("************ Find PER ************\n");
        
    	//NIM_PRINTF("current PER is  %d\n", percount);

		nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
		data=0x7f&data;
		nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);
		comm_delay(10);
		nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
		data &= 0x3f;
		data |= 0xc0;
		nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);

		return SUCCESS;
	}
	else
	{
		return ERR_PARA;
	}
}
 
/*****************************************************************************
*  INT32 nim_c3505_get_phase_error(struct nim_device *dev, INT32 *phase_error)
 * 
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: INT32 *phase_error
 *
 * Return Value: SUCCESS
 *****************************************************************************/
 INT32 nim_c3505_get_phase_error(struct nim_device *dev, INT32 *phase_error)
 {
	 static UINT8 data = 0;
 
	 nim_reg_read(dev, R71_CAP_RDY, &data, 1);
	 if (0x00 == (data & 0x02))
	 {
         data = data | 0x02;
         nim_reg_write(dev, R71_CAP_RDY, &data, 1);          
         
         //read out the phase error, ECO_CR_PHASE_ERROR/256*360
         nim_reg_read(dev, R70_CAP_REG, &data, 1);   
         *phase_error = data;
         return SUCCESS;
	 }
	 else
	 {
		 *phase_error = data;
		 return ERR_FAILUE;  // means that phase error is not ready
	 }
 }



 
 /*****************************************************************************
*  INT32 nim_C3505_get_new_PER(struct nim_device *dev, UINT32 *per)
* Get bit error ratio
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_new_per(struct nim_device *dev, UINT32 *per)
{
	UINT8 data = 0;
	UINT32 t_count = 0, myper = 0;

	for (t_count = 0; t_count < 200; t_count++)
	{
		nim_reg_read(dev, R79_PKT_ERR + 0x01, &data, 1);
		if ((data & 0x80) == 0)
		{
			myper = data & 0x7f;
			nim_reg_read(dev, R79_PKT_ERR, &data, 1);
			myper <<= 8;
			myper += data;
			break;
		}
	}
	*per = myper;
	//  NIM_PRINTF("!!!!!!!! myPER cost %d time, per = %d\n",t_count,myper);
	return SUCCESS;
}
 
/*****************************************************************************
* INT32 nim_C3505_get_MER(struct nim_device *dev, UINT32 *mer)
* Description: Ger MER
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 *mer
*
* Return Value: UINT32
*****************************************************************************/
INT32 nim_c3505_get_mer(struct nim_device *dev, UINT32 *mer)
{
    *mer = c3505_m_mer;
    NIM_PRINTF("Get mer = %d\n", *mer);
    return SUCCESS;
}


 
/*****************************************************************************
* INT32 nim_c3505_get_mer_task(struct nim_device *dev, UINT32 *mer)
* Description: Ger MER
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 *mer
*
* Return Value: UINT32
*****************************************************************************/
INT32 nim_c3505_get_mer_task(struct nim_device *dev)
{
    UINT8 data;
    UINT8 rdata[4]; 
    char data_char; 
    int data_in_i=0, data_in_q=0, data_out_i=0,data_out_q=0;
    int err_sum=0, mer_tmp=0;
    INT16 timer=0, snr_db=0;
    UINT32 i=0;

    #define NIM_MER_AVG_LEN 8
    #define NIM_MER_STA_LEN 4

    nim_reg_read(dev,R04_STATUS,&data,1);
    if (data&0x08)
    { // CR lock
        err_sum = 0;
        for (i=0;i<NIM_MER_AVG_LEN;i++)
        {
            timer = 0;
            data = 0x01;
            nim_reg_write(dev, R11C_MAP_IN_I, &data, 1); // start cap I/Q
            while (timer < 250)
            {
		    	nim_reg_read(dev, R11C_MAP_IN_I - 0x0b, &data, 1);
            	if(0 == (data & 0x40))
            	    break; // latch I/Q success
            	else
            	    comm_delay(1);
                timer += 1;
            }
            nim_reg_read(dev, R11C_MAP_IN_I, rdata, 4);
            data_char = rdata[0];
            data_in_i = (int)data_char;
            data_char = rdata[1];
            data_in_q = (int)data_char;
            data_char = rdata[2];
            data_out_i = (int)data_char;
            data_char = rdata[3];
            data_out_q = (int)data_char;
            err_sum = err_sum + (data_out_i-data_in_i)*(data_out_i-data_in_i) + (data_out_q-data_in_q)*(data_out_q-data_in_q);
        }
        if(va_mer_window < NIM_MER_STA_LEN)
        {
            va_mer = va_mer + err_sum;
            va_mer_window++;
            mer_tmp = err_sum/NIM_MER_AVG_LEN;
        }
        else
        {
            if(va_mer == 0)
                va_mer = err_sum;
            else if(va_mer < NIM_MER_STA_LEN)
                va_mer = va_mer + err_sum - 1;
            else
                va_mer = va_mer + err_sum/NIM_MER_AVG_LEN - (va_mer/(NIM_MER_STA_LEN*NIM_MER_AVG_LEN));
            mer_tmp = va_mer/(NIM_MER_AVG_LEN*NIM_MER_STA_LEN);
        }
        if (mer_tmp > 1681)
            mer_tmp = 1681;
            mer_tmp = 323-nim_c3505_Log10Times100_L(mer_tmp); // 323 = 10*log10(1681)*10, align to 0.1dB.
        if (mer_tmp>255)
            c3505_m_mer = 255;
        else
            c3505_m_mer = mer_tmp;

        // While snr is less than 10dB , that nim_c3505_get_snr_db may be more accurately, paladin.ye 20160817
        if(c3505_m_mer < 100)
        {
            nim_c3505_get_snr_db(dev, &snr_db);
			if (snr_db < 0)
				snr_db = 0;
            c3505_m_mer = (snr_db + 5) / 10;
        }
        //NIM_PRINTF("Get mer = %d\n", c3505_m_mer);
        return SUCCESS;
    }
    else
    {
        //NIM_PRINTF("Can not get MER when demod unlock.\n");      
        c3505_m_mer = 0;
        va_mer = 0;
        va_mer_window = 0;
        return ERR_FAILUE;
    }
}

/*****************************************************************************
* INT32 nim_C3505_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*	The Eb No is calculated using the SNR from the NIM, using the formula:
*	   Eb ~ 	13312- M_SNR_H
*	   -- =    ----------------  dB.
*	   NO			683
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *snr
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_snr(struct nim_device *dev, UINT8 *snr)
{
	UINT8 data = 0;
	INT16 snr_db = 0;
	UINT8 work_mode = 0;

    nim_c3505_get_lock(dev, &data);

    if(0 == data)
    {
        *snr = 10;
        return ERR_FAILED;
    }

    nim_c3505_get_snr_db(dev, &snr_db);
    if (snr_db < 0)
        snr_db = 0;

    snr_db = snr_db / 10;
	nim_c3505_reg_get_work_mode(dev,&work_mode);
	if (0 == work_mode)  //DVB-S Mapping
	{	
		if(snr_db >= 150)
			*snr = 90;
		else if(snr_db < 120)
		{
			*snr = snr_db / 4;									 
		}
		else
		{
		*snr = (snr_db-120) * 2 + 30;
		}
	}
	else              //DVB-S2 Mapping
	{
		if(snr_db >= 180)
			*snr = 90;
		else if(snr_db < 120)
		{
			*snr = snr_db / 4;									 
		}
		else
		{
			*snr = (snr_db-120) + 30;
		}
	}
    //MON_PRINTF("SNR db = %d, snr = %d\n", snr_db, *snr);

	return SUCCESS;
}
 
/*****************************************************************************
*  INT32 nim_C3505_get_snr_db(struct nim_device *dev)
* Get estimated CNR in unit 0.01dB from CR new adaptive 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter1: INT16 *snr_db
*
* Return Value: UINT16
*****************************************************************************/
INT32 nim_c3505_get_snr_db(struct nim_device *dev, INT16 *snr_db)
{
	///// display est_noise and then return the current c/n value
	/*
	wire [7:0] reg_cr13b = EST_NOISE[7:0];
	wire [7:0] reg_cr13c = {3'b0,EST_NOISE[12:8]};
	but here only use EST_NOISE[12:3]
	*/
	UINT16 linear_cn = 1 ;
	UINT16 diff_cn = 0;
	UINT16 ref_est_noise = 0 ;
	UINT16 ref_cn = 0;	 
	UINT8 rdata[2] = {0};
	UINT16 rdata16 = 0;
	UINT16 est_noise_1 = 0; // here must be test again
	UINT8 modu = 0;  
	INT16 snr_cn = 0;
	UINT8 work_mode = 0;
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    nim_c3505_reg_get_work_mode(dev,&work_mode);

    if ((CHIP_ID_3505 != priv->ul_status.m_c3505_type) || (1 == work_mode))
    {
        return nim_c3505_get_snr_db_new(dev, snr_db); 
    }
    else
    {
		modu = 11;
    	nim_reg_read(dev,R13b_EST_NOISE, rdata, 2);
    	rdata16 = rdata[1];
    	est_noise_1 = (rdata[0] | (rdata16 << 8)) >> 4; // here must be test again
    	ADPT_NEW_CR_PRINTF("EST_NOISE=%d--0x%x\n",est_noise_1,est_noise_1);
    }

	switch(modu)
	{
		case 0: 
			ADPT_NEW_CR_PRINTF("unknown constellation\n");
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11: // QPSK
			ADPT_NEW_CR_PRINTF("QPSK\n");
			if(est_noise_1>221)	 //8.0dB because when c/n>8.0dB, estimate c/n error is bigger than 0.4dB
			{
				linear_cn = 196;
				diff_cn = 5;
				ref_est_noise = 47900; // QPSK, 2.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;1/2code rate
				ref_cn = 200;
			}
			else
			{
				linear_cn = 155;
				diff_cn = 5;
				ref_est_noise = 22100; // QPSK, 8.1dB AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;1/2code rate
				ref_cn = 860;//810; //in order to make snr smoothly
			}
		break;
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:  // 8PSK
			ADPT_NEW_CR_PRINTF("8PSK\n");
			linear_cn = 100;
			diff_cn = 5;
			ref_est_noise = 21500; // 8psk, 6.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;3/5code rate
			ref_cn = 600;
		break;
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23: // 16APSK
			ADPT_NEW_CR_PRINTF("16PSK\n");
			if(est_noise_1 > 96)
			{
			linear_cn = 50;
			diff_cn = 5;
			ref_est_noise = 11300; // 10.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;2/3code rate
			ref_cn = 1000;
			}
			else
			{
			linear_cn = 51;
			diff_cn = 5;
			ref_est_noise = 9600; // 11.4 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;2/3code rate
			ref_cn = 1150;
			}
		break;
		case 24:
		case 25:
		case 26:
		case 27://32APSK
			ADPT_NEW_CR_PRINTF("32PSK,24->28 \n");
			linear_cn = 25;
			diff_cn = 5;
			ref_est_noise = 5400; // 14 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;3/4code rate
			ref_cn = 1400;
		break;
		case 28:	 // 32APSK
			ADPT_NEW_CR_PRINTF("32PSK,28\n");
			linear_cn = 25;
			diff_cn = 5;
			ref_est_noise = 3700; // 17 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;9/10code rate
			ref_cn = 1700;
		break;
		default:
		break; 			   
	}
	snr_cn = ((ref_est_noise-est_noise_1*100)/linear_cn) * diff_cn + ref_cn; // the current c/n value X10	 
		
	//MON_PRINTF("c/n=%d\n",snr_cn);
	*snr_db = snr_cn;

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_snr_filter(struct nim_device *dev, INT32 *p_snr)
*  This function will make the snr value smoothly
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter1: INT32 *p_snr
*
* Return Value: UINT16
*****************************************************************************/
INT32 nim_c3505_snr_filter(struct nim_device *dev, INT32 *p_snr)
{
	INT32 s_sum = 0; 
	int k,n1,n2;
	static int j = 0;
	static INT16   s_data[15]={0};	
	static INT16   s_snr=0x00; 

	
	if (j == (s_filter_len - 1)) 
	{		
		for (k=0; k<(s_filter_len-1); k++)
		{
			s_sum=s_sum+s_data[k];
		}
		s_sum = *p_snr + s_sum;
		*p_snr = s_sum / s_filter_len;
		n1 = s_sum-(*p_snr) * s_filter_len;
		if(n1 >= (s_filter_len / 2))
			n1 = 1;
		else
			n1=0;
		
		*p_snr = *p_snr + n1;			
		s_snr = *p_snr;
		j = j + 1;
	}
	else if (j==s_filter_len)
	{
		n1 = (s_snr * (s_filter_len - 1)) / s_filter_len;
		n1 = (s_snr * (s_filter_len-1)) - n1 * s_filter_len;
		n2 = *p_snr / s_filter_len;
		n2 = *p_snr-n2 * s_filter_len;
		n2 = n1 + n2;
		if( n2 >= (3*(s_filter_len / 2)))
			n1 = 2;
		else if((n2<(3*(s_filter_len / 2))) && (n2 >= (s_filter_len/2)))
			n1 = 1;
		else
			n1 = 0;		   

		*p_snr = (s_snr * (s_filter_len - 1)) / s_filter_len + *p_snr / s_filter_len + n1;
		s_snr = *p_snr;
	}
	else 
	{
		s_data[j] = *p_snr;
		*p_snr = *p_snr;
		j = j + 1;
	} 
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3505_get_snr_db_new(struct nim_device *dev)
* Get estimated CNR in unit 0.01dB  only for dvb-s2 c3505, but dvb-s/s2 for c3503c/c3503d , can be negative
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter1: INT16 *snr_db
*
* Return Value: UINT16
*****************************************************************************/
INT32 nim_c3505_get_snr_db_new(struct nim_device *dev, INT16 *snr_db)
{
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    UINT16 snr_factor = 32;
    INT32 snr_tmp = 0;
	UINT8 data[2] = { 0 };

	// For 3503c need set reg0xa9[4] = 1 to read value
	nim_reg_read(dev, 0xa9, data, 1);
	data[0] |= 0x10;
	nim_reg_write(dev, 0xa9, data, 1);

	nim_reg_read(dev,0xa8, data, 2);

	snr_tmp = (data[1] << 8) | data[0];	
    
    // Negative
    if(data[1] & 0x08)
       snr_tmp =  snr_tmp - 4096;

    if (CHIP_ID_3505 == priv->ul_status.m_c3505_type)
        snr_factor = 32;
    else
        snr_factor = 40;

    snr_tmp *= 125;

    if(0 == snr_factor)
        snr_factor = 32;

    snr_tmp = (snr_tmp + snr_factor/2 ) / snr_factor;
	nim_c3505_snr_filter(dev, &snr_tmp);
    *snr_db = (INT16)snr_tmp;
    
	//MON_PRINTF("snr_db value : %d\n", *snr_db);

	return SUCCESS;
}



/*****************************************************************************
* INT32 nim_C3505_check_BER(struct nim_device *dev, UINT32 *RsUbc)
* Get bit error ratio(10E-5)
*
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_C3505_check_BER(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 data = 0;
	UINT8 work_mode = 0;
	UINT8 ber_data[2] = {0};
	UINT32 u32ber_data[2] = {0};
	UINT32 uber_data = {0};
	UINT32 i = 0;

#define ber_dvbs_beta 4   
#define ber_dvbs2_beta 4

	//struct nim_c3505_private *priv = NULL;

	if((dev == NULL) || (rs_ubc == NULL))
		return RET_FAILURE;
	
	//priv = (struct nim_c3505_private *) dev->priv;	
	nim_reg_read(dev, R04_STATUS, &data, 1);
	if(0x3f != (data&0x3f))
	{
		va_ber = 0;
		va_ber_window = 0;
		*rs_ubc = 0xffff;
		NIM_PRINTF("----NIM_WARNING: FEC unlock, can not get BER\n");

		return SUCCESS;
	}
	else
	{
		nim_c3505_reg_get_work_mode(dev, &work_mode);
		if(0 == work_mode) // DVBS 
		{
			if(va_ber_window < ber_dvbs_beta)
			{
				for (i = 0;i < ber_dvbs_beta;i++)
				{
					 nim_reg_read(dev, R54_VITERBI_FRAME_SYNC+1, ber_data, 2);
					 u32ber_data[0] = (UINT32) ber_data[0];
					 u32ber_data[1] = (UINT32) (ber_data[1]&0x0f);
					 u32ber_data[1] <<= 8;
					 uber_data = u32ber_data[1] + u32ber_data[0];                  
					 va_ber = va_ber + uber_data;
					 va_ber_window++;                   
				}
				*rs_ubc = (va_ber*100)/ber_dvbs_beta;  //10*RsUbc = 108*(va_ber*1000000)/(ber_dvbs_beta*10000); 
				}
			else
			{
				nim_reg_read(dev, R54_VITERBI_FRAME_SYNC+1, ber_data, 2);
				u32ber_data[0] = (UINT32) ber_data[0];
				u32ber_data[1] = (UINT32) (ber_data[1]&0x0f);
				u32ber_data[1] <<= 8;
				uber_data = u32ber_data[1] + u32ber_data[0];
				
				if(va_ber == 0)
					va_ber = uber_data;
				else if(va_ber < ber_dvbs_beta)
					va_ber = va_ber + uber_data - 1;
				else
					va_ber = va_ber + uber_data - (va_ber/ber_dvbs_beta); 
				
				*rs_ubc = (va_ber*100)/ber_dvbs_beta;  //10*RsUbc = 108*(va_ber*1000000)/(ber_dvbs_beta*10000)          
			}         
		}
		else // DVBS2 
		{
			if(va_ber_window < ber_dvbs2_beta)
			{
				for (i = 0;i < ber_dvbs2_beta;i++)
				{
					nim_reg_read(dev, RD3_BER_REG + 0x01, ber_data, 2);
					u32ber_data[0] = (UINT32) ber_data[0];
					u32ber_data[1] = (UINT32) ber_data[1];
					u32ber_data[1] <<= 8;
					uber_data = u32ber_data[1] + u32ber_data[0];                 
					va_ber = va_ber + uber_data;
					va_ber_window++;
				}
				*rs_ubc = (va_ber*10000)/(ber_dvbs2_beta*648);//10*RsUbc =10*(va_ber*100000)/(ber_dvbs2_beta*64800)
			}
			else
			{
				nim_reg_read(dev, RD3_BER_REG + 0x01, ber_data, 2);
				u32ber_data[0] = (UINT32) ber_data[0];
				u32ber_data[1] = (UINT32) ber_data[1];
				u32ber_data[1] <<= 8;
				uber_data = u32ber_data[1] + u32ber_data[0];

				if(va_ber == 0)
					va_ber = uber_data;
				else if(va_ber < ber_dvbs2_beta)
					va_ber = va_ber + uber_data - 1;
				else
					va_ber = va_ber + uber_data - (va_ber/ber_dvbs2_beta);
				
				*rs_ubc = (va_ber*10000)/(ber_dvbs2_beta*648);//10*RsUbc =10*(va_ber*100000)/(ber_dvbs2_beta*64800)             
			}         
		}

		return SUCCESS;
	}
}

/*****************************************************************************
*  UINT8 nim_C3505_get_SNR_index(struct nim_device *dev)
*	
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
UINT8 nim_c3505_get_snr_index(struct nim_device *dev)
{
	INT16 lpf_out16 = 0;
	INT16 agc2_ref5 = 0;
	INT32 snr_indx = 0;
	UINT8 data[2] = {0};
	UINT16 tdata[2] = {0};

	//CR45
	nim_reg_read(dev, R45_CR_LCK_DETECT, &data[0], 1);
	//CR46
	nim_reg_read(dev, R45_CR_LCK_DETECT + 0x01, &data[1], 1);

	tdata[0] = (UINT16) data[0];
	tdata[1] = (UINT16) (data[1] << 8);
	lpf_out16 = (INT16) (tdata[0] + tdata[1]);
	lpf_out16 /= (16 * 2);

	//CR07
	nim_reg_read(dev, R07_AGC1_CTRL, &data[0], 1);
	agc2_ref5 = (INT16) (data[0] & 0x1F);

	snr_indx = (lpf_out16 * agc2_ref5 / 21) - 27;//27~0

	if (snr_indx < 0)
	snr_indx = 0;
	else if (snr_indx > 176)
	snr_indx = 176;

	return snr_indx;
}
 /*****************************************************************************
 * INT32 nim_c3505_get_CURFreq(struct nim_device *dev)
 * Description: 
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 pol
 *
 * Return Value: void
 *****************************************************************************/
INT32 nim_c3505_get_cur_freq(struct nim_device *dev, UINT32 *freq)
{
	*freq = ((struct nim_c3505_private *) dev->priv)->ul_status.m_cur_freq/1024;
	return SUCCESS;
}
 
 /*****************************************************************************
 * UINT8 nim_C3505_get_CRNum(struct nim_device *dev)
 * Description: S3501 set polarization
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 pol
 *
 * Return Value: void
 *****************************************************************************/
 UINT8 nim_c3505_get_cr_num(struct nim_device *dev)
 {
	 return ((struct nim_c3505_private *) (dev->priv))->ul_status.m_cr_num;
 }

/*****************************************************************************
* INT32 nim_C3505_get_LDPC(struct nim_device *dev, UINT32 *RsUbc)
* Get LDPC average iteration number
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_C3505_get_LDPC(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 data;
	//UINT8   ite_num;
	//struct nim_c3505_private* priv = (struct nim_c3505_private  *)dev->priv;

	// read single LDPC iteration number
	nim_reg_read(dev, RAA_S2_FEC_ITER, &data, 1);
	*rs_ubc = (UINT32) data;

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_get_LDPC_iter_cnt(struct nim_device *dev, UINT16 *iter_cnt)
* Description: get ldpc iter counter 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *iter_cnt, it is 10 bits width
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_ldpc_iter_cnt(struct nim_device *dev, UINT16 *iter_cnt)
{
	UINT8 data[2] = {0};
	// read LDPC iteration number, maybe once, maybe the max, according to reg_crc1[3]
	nim_reg_read(dev, RAA_S2_FEC_ITER, data, 2);
	data[1] &= 0x03;
	*iter_cnt = (UINT16)(data[1]*256 + data[0]);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_cr_sw_snr_rpt(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cr_sw_snr_rpt(struct nim_device *dev)
{
	UINT8 data = 0;
	static UINT16 snr_max = 0;
	static UINT16 snr_min = 0;
	UINT16 var = 0;
	static INT32 interval_cnt = 0;

	interval_cnt++;
	if(interval_cnt == 10)
	{
		nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
		if (data & 0x80) // SNR estimation data ready
		{
			var = (data & 0x1f) << 6;
			nim_reg_read(dev, RBB_SNR_RPT1, &data, 1); // read SNR LSB
			var += (data>>2) & 0x03f;

			if(snr_initial_en)
			{
			snr_max = var;
			snr_min = var;
			snr_initial_en = 0;
			}

			if(var > snr_max)
			snr_max = var;
			if(var < snr_min)
			snr_min = var;

			data = 0x00;
			nim_reg_write(dev, RBC_SNR_RPT2, &data, 1); // clear SNR ready flag

			NIM_PRINTF("*** SNR = %u min=%u max=%u\n", var, snr_min, snr_max);
		}

		interval_cnt = 0;
	}

	return SUCCESS;
}


/*****************************************************************************
*  UINT8 nim_c3505_map_beta_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 *demap_beta_active, UINT8 *demap_beta)
* report demap beta active and data
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
* Return Value: UINT8 
*****************************************************************************/
UINT8 nim_c3505_map_beta_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 *demap_beta_active, UINT8 *demap_beta)
{
	UINT8 data = 0;	

	if(frame_mode == 0)//short
	{
		data = index;//index                                        			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);          			
		data = 0x06; //read demap_beta active                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			            
		nim_reg_read(dev, R0E_ADPT_CR_CTRL, &data, 1);                                                          			
		data &= 0xfe;
		data |= 0xe0;                             
		nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); 
		nim_reg_read(dev, R9D_RPT_DEMAP_BETA, &data, 1);  
		*demap_beta_active = data & 0x01;                			
		data = 0x05; //read demap_beta data                                                			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			            
		nim_reg_read(dev, R0E_ADPT_CR_CTRL, &data, 1);                                                          			
		data &= 0xfe;
		data |= 0xe0;                             
		nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); 
		nim_reg_read(dev, R9D_RPT_DEMAP_BETA, &data, 1);  
		*demap_beta = data & 0xff;                 			
	}
	else if(frame_mode == 1)//normal
	{
		data = index;//index                                        			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);          			
		data = 0x02; //read demap_beta active                                                  			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			            
		nim_reg_read(dev, R0E_ADPT_CR_CTRL, &data, 1);                                                          			
		data &= 0xfe;
		data |= 0xe0;                             
		nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); 
		nim_reg_read(dev, R9D_RPT_DEMAP_BETA, &data, 1);  
		*demap_beta_active = data & 0x01;                			
		data = 0x01; //read demap_beta data                                                			
		nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);          			            
		nim_reg_read(dev, R0E_ADPT_CR_CTRL, &data, 1);                                                          			
		data &= 0xfe;
		data |= 0xe0;                             
		nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); 
		nim_reg_read(dev, R9D_RPT_DEMAP_BETA, &data, 1);  
		*demap_beta = data & 0xff;                 			
	}

	return SUCCESS;
}

/*****************************************************************************
*  UINT16 nim_c3505_demap_noise_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT16 *demap_noise))
* report demap noise
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 index
*  Parameter3: UINT8 frame_mode
* Return Value: UINT16 
*****************************************************************************/
UINT16 nim_c3505_demap_noise_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT16 *demap_noise)
{
	UINT8 data = 0,noise_index = 0;
	UINT16 t_data[2] = { 0 };

	if(frame_mode == 0)//short
	{
        noise_index = index; //index                                   		
        noise_index &= 0x7f;                                           		
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);         			
	    nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);	//es_noise reg
		data &= 0xfd;  
		data |= 0x88;  
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);	  
        nim_reg_read(dev, RD0_DEMAP_NOISE_RPT, &data, 1);	//es_noise_buf_reg[7:0]
		t_data[0] = (UINT16) data;      			
	    nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);	//es_noise_buf_reg[14:8]
		t_data[1] = (UINT16) (data << 8);    
		*demap_noise =  (t_data[0] + t_data[1]) & 0x7fff;               			
	}
	else if(frame_mode == 1)//normal
	{
		noise_index = index; //index                                   		
        noise_index &= 0x7f;                                           		
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);         			
		nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);	//es_noise reg
		data &= 0x75;  
		nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);	  
        nim_reg_read(dev, RD0_DEMAP_NOISE_RPT, &data, 1);	//es_noise_buf_reg[7:0]
		t_data[0] = (UINT16) data;      			
	    nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);	//es_noise_buf_reg[14:8]
		t_data[1] = (UINT16) (data << 8);    
		*demap_noise =  (t_data[0] + t_data[1]) & 0x7fff;               			
	}
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_C3505_reg_get_freqoffset(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_reg_get_freqoffset(struct nim_device *dev)
{
	INT32 freq_off;
	UINT8 data[3];
	UINT32 tdata;
	UINT32 temp;
	UINT32  sample_rate;

	nim_c3505_get_dsp_clk(dev, &sample_rate);
	temp = 0;
	nim_reg_read(dev, R69_RPT_CARRIER, data, 3);
	tdata = (data[0] & 0xff) + ((data[1] & 0xff)<<8);

	if ((data[2] & 0x01) == 1)
		temp = (tdata ^ 0xffff) + 1;
	else
		temp = tdata & 0xffff;

	if ((data[2] & 0x01) == 1)
		freq_off = 0 - nim_c3505_multu64div(temp, sample_rate, 92160000); // 92160000 == 90000*1024
	else
		freq_off = nim_c3505_multu64div(temp, sample_rate, 92160000);

	return freq_off;
}


/*****************************************************************************
* INT32 nim_C3505_get_symbol(struct nim_device *dev)
*
* This function printf the Constellation Points 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_symbol(struct nim_device *dev, struct nim_get_symbol *p_symbol)
{
	UINT8 data = 0;
	UINT32 i = 0;
	UINT8 data_2 = 0;
	UINT8 data_i;
	UINT8 data_q;

	nim_reg_read(dev, R130_CR_PARA_DIN + 7, &data_2, 1);
	data_2 |= 0x80;//bit7 set 1
	nim_reg_write(dev, R130_CR_PARA_DIN + 7, &data_2, 1);//it must set R137 bit7 to 1 for read I,Q
	
	for (i = 0; i < p_symbol->point_length; i++)
	{
		data = 0xc1;
		nim_reg_write(dev, RC8_BIST_TOLERATOR, &data, 1);
		comm_delay(5);
		nim_reg_read(dev, RC9_CR_OUT_IO_RPT, &data_i, 1);
		//libc_printf("%02x\n", data_i);
		nim_reg_read(dev, RC9_CR_OUT_IO_RPT + 0x01, &data_q, 1);
		//libc_printf("%02x\n", data_q);

		p_symbol->p_cr_out_i[i] = data_i;
		p_symbol->p_cr_out_q[i] = data_q;
	}
	data = 0xc0;
	nim_reg_write(dev, RC8_BIST_TOLERATOR, &data, 1);
	data_2 &= 0x7f;
	nim_reg_write(dev,R130_CR_PARA_DIN+7,&data_2,1);
	
	nim_reg_read(dev, R130_CR_PARA_DIN + 7, &data_2, 1);
	data_2 &= 0x7F;//bit7 set 0
	nim_reg_write(dev, R130_CR_PARA_DIN + 7, &data_2, 1);//clear bit7
	
	return SUCCESS;
}


 /*****************************************************************************
 * INT32 nim_c3505_get_bb_header_info(struct nim_device *dev)
 *
 * This function get bb header data from hardware 
 *
 * Arguments:
 *  Parameter1: struct nim_device *dev
 *  Parameter2: 
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3505_get_bb_header_info(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT8 work_mode = 0;
	UINT8 time_out = 0;
	//UINT8 reg106_bak = 0x27;
	UINT32 i = 0;
	UINT32 bbheader_crc_flg  = 0;
    UINT8 crc_err_cnt = 0;
    INT32 ret = 0;
	void *ptr_bbheader = NULL;
	int polynomial   = 0x56;
	struct nim_c3505_private *priv = NULL;
	struct nim_dvbs_bb_header_info *bb_header_info = NULL;
	 
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}
    
    ACM_DEBUG_PRINTF("Enter %s\n", __FUNCTION__);
    
	priv = (struct nim_c3505_private *)dev->priv;
	bb_header_info = priv->bb_header_info;	

	comm_memset(bb_header_info, 0, sizeof(struct nim_dvbs_bb_header_info));

	//ACM_DEBUG_PRINTF("Enter %s \n", __FUNCTION__);    
	// Disable isid filter(use bypass), use dfl
	#if 0
	nim_reg_read(dev, R106_BBF_CFG, &data, 1);
	reg106_bak = data;

	//2.18.2017 -- Paladin
	data |= 0x18;
	nim_reg_write(dev, R106_BBF_CFG, &data, 1);
	#endif
	
	// Check lock status
	while(1)
	{
		if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
		{
			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
				priv->ul_status.c3505_autoscan_stop_flag);
			
			ret = ERR_FAILED;
			goto EXIT;
		}
		
		nim_c3505_get_lock(dev, &data);
		// lock
		if(1 == data)
			break;
		
		// unlock
		if(time_out >= 15)
		{
			ACM_DEBUG_PRINTF("In %s exit with TP unlock \n", __FUNCTION__);	

			ret = ERR_FAILED;
			goto EXIT;
		}

		time_out++; 
		comm_sleep(5);
	}
	time_out = 0;

	// Check TP type
	nim_c3505_reg_get_work_mode(dev, &work_mode);
	if(0 == work_mode)
	{
		ACM_DEBUG_PRINTF("In %s exit with TP is not DVB-S2\n", __FUNCTION__);

		ret = ERR_FAILED;
		goto EXIT;
	} 



	while(1)
	{
    	// Begin latch the latest BB frame head information
    	nim_reg_read(dev, R104_BBF, &data, 1);
    	if(data & 0x10)
    	{
    		ACM_DEBUG_PRINTF("In %s get reg104 = %d error \n", __FUNCTION__, data);
    		//return ERR_FAILED;
    	}
    	data = 0x10;
    	nim_reg_write(dev, R104_BBF, &data, 1);

    	// Wait capture finished
    	while(1)
    	{
    		if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
    		{
    			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
    				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
    				__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
    				priv->ul_status.c3505_autoscan_stop_flag);	

    			ret = ERR_FAILED;
    			goto EXIT;
    		}
    		
    		nim_reg_read(dev, R104_BBF, &data, 1);

    		if(0 == (data & 0x10))
    		{
    			//ACM_DEBUG_PRINTF("Capture BB Frame finished \n");
    			break;
    		}

    		comm_sleep(5);
    		time_out++;

    		if(time_out >= 50)
    		{
    			ACM_DEBUG_PRINTF("Wait capture BB Frame timeout \n");

    			ret = ERR_FAILED;
    			goto EXIT;
    		}
    	}

    	// Read bb header from register
    	for(i = 0; i < 10; i++)
    	{
    		nim_reg_read(dev, R105_BBF, &data, 1);
    		bb_header_info->source_data[i] = data;
            ACM_DEBUG_PRINTF("bb_header[%d] = 0x%x \n", i, data);
    		nim_reg_read(dev, R104_BBF, &data, 1);
    		data++;
    		nim_reg_write(dev, R104_BBF, &data, 1);
    	}

    	// added bbheader crc in driver by gloria 2016.1.6
    	ptr_bbheader = (void*)(bb_header_info->source_data);

        if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
        {
            if (0x02 == (0x02&bb_header_info->source_data[9]))
            {
                break;
            }
            else
            {
                crc_err_cnt++;
                
                if (CRC_THRESHOLD == crc_err_cnt)
                {
                    ACM_DEBUG_PRINTF("Get BBHEADER error, crc_err_cnt = %d\n", crc_err_cnt);
            
                    ret = ERR_CRC_HEADER;
                    goto EXIT;
                } 
            }
        }
        else
        {

        	bbheader_crc_flg = nim_c3505_crc8_check(ptr_bbheader, 80, polynomial);

            if ((0x47 == bb_header_info->source_data[6]) && (0 == bbheader_crc_flg))
            {
                break;
            }
            else
            {
                crc_err_cnt++;
                
                if (CRC_THRESHOLD == crc_err_cnt)
                {
                    ACM_DEBUG_PRINTF("Get BBHEADER error, crc_err_cnt = %d\n", crc_err_cnt);

        			ret = ERR_CRC_HEADER;
        			goto EXIT;
                } 
            }
        }
    }
    

    // Analysis bb header
    bb_header_info->stream_mode = (bb_header_info->source_data[0] & 0xc0) >> 6;
    bb_header_info->stream_type = (bb_header_info->source_data[0] & 0x20) >> 5;
    bb_header_info->acm_ccm = (bb_header_info->source_data[0] & 0x10) >> 4;
    
    bb_header_info->issyi = (bb_header_info->source_data[0] & 0x08) >> 3;
    bb_header_info->npd = (bb_header_info->source_data[0] & 0x04) >> 2;
    bb_header_info->roll_off = (bb_header_info->source_data[0] & 0x03);
    
    if(NIM_STREAM_MULTIPLE == bb_header_info->stream_type)
    {
        bb_header_info->isi = bb_header_info->source_data[1];
    }
    else
    {
        bb_header_info->isi = 0;
    }
    
    bb_header_info->upl = (bb_header_info->source_data[2] << 8) + bb_header_info->source_data[3];
    bb_header_info->dfl = (bb_header_info->source_data[4] << 8) + bb_header_info->source_data[5];
    bb_header_info->sync = bb_header_info->source_data[6];
    bb_header_info->syncd = (bb_header_info->source_data[7] << 8) + bb_header_info->source_data[8];
    bb_header_info->crc_8 = bb_header_info->source_data[9];

	ret = SUCCESS;


EXIT:
#if 0
	nim_reg_write(dev, R106_BBF_CFG, &reg106_bak, 1);

	
	//Reset FSM 2-18-2017 Paladin
	if (SUCCESS == ret)
	{
		UINT32 time_out = 0;

		nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
		comm_sleep(1);
		nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

		while(1)
		{
			nim_c3505_get_lock(dev, &data);
			// lock
			if(1 == data)
				break;

			// unlock
			if(time_out >= 200)
			{
				ACM_DEBUG_PRINTF("In %s exit with TP unlock \n", __FUNCTION__);
				return ERR_FAILED;
			}

			time_out++;
			comm_sleep(1);
		}
	}
	// end
#endif
	
	return ret;
}



/*****************************************************************************
* INT32 nim_c3505_get_channel_info(struct nim_device *dev)
*
* This function get channel info from hardware 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_channel_info(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 time_out = 0;
    UINT8 work_mode = 0;
    UINT8 code_rate = 0;
    UINT8 roll_off = 0;
    UINT8 pilot_mode = 0;
    UINT8 modcod = 0;
    UINT8 iqswap_flag = 0;
    UINT8 map_type = 0;
    UINT8 agc = 0;
    INT16 snr = 0;
    struct nim_c3505_private *priv = NULL;
    struct nim_dvbs_channel_info *channel_info = NULL;

    //ACM_DEBUG_PRINTF("Enter %s \n", __FUNCTION__);
	 
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}
	
	priv = (struct nim_c3505_private *)dev->priv;
	channel_info = priv->channel_info;

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
    nim_c3505_reg_get_code_rate(dev, &code_rate);
    nim_c3505_reg_get_roll_off(dev, &roll_off);
    nim_c3505_reg_get_modcod(dev, &modcod);
    pilot_mode = modcod & 0x01;
    modcod = modcod>>2;
    nim_c3505_reg_get_iqswap_flag(dev, &iqswap_flag);
    nim_c3505_reg_get_map_type(dev, &map_type);
    nim_c3505_get_agc(dev, &agc);
    agc -= 100;

    nim_c3505_get_snr_db(dev, &snr);
	snr = (snr + 50) / 100;

    channel_info->work_mode = work_mode;
    channel_info->code_rate = code_rate;
    channel_info->roll_off = roll_off;
    channel_info->pilot_mode = pilot_mode;
    channel_info->modcod = modcod;
    channel_info->iqswap_flag = iqswap_flag;
    channel_info->map_type = map_type;
    channel_info->signal_level = agc;
    channel_info->snr = (UINT8)snr;

    return SUCCESS;
}


// For test
/*****************************************************************************
* INT32 nim_c3505_get_tp_info(struct nim_device *dev)
*
* This function get tp info from hardware 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_tp_info(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT32 time_out = 0;
    struct nim_c3505_private *priv = NULL;
	INT32 ret_val = 0;

    //ACM_DEBUG_PRINTF("Enter %s \n", __FUNCTION__);
	 
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}
	
	priv = (struct nim_c3505_private *)dev->priv;
    
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
            //ACM_DEBUG_PRINTF("In %s exit with TP unlock \n", __FUNCTION__);
            return ERR_FAILED; 
        }
        
        time_out++; 
        comm_sleep(5);
    }
	
    if(SUCCESS != nim_c3505_get_channel_info(dev))
		return ERR_FAILED;
	// DVB-S2
	if(1 == priv->channel_info->work_mode)	
	{
	/*
    	if(SUCCESS != nim_c3505_get_bb_header_info(dev))
			return ERR_FAILED;
        #if (NIM_OPTR_ACM == ACM_CCM_FLAG)
    		if(NIM_STREAM_MULTIPLE == priv->bb_header_info->stream_type) 
			{
        		if(SUCCESS != nim_c3505_get_isid(dev))
					return ERR_FAILED;
			}
        #endif
		*/
	    ret_val = nim_c3505_get_bb_header_info(dev);
		if (SUCCESS != ret_val)
		{ 
		    return ret_val;
		}
	}
    //ACM_DEBUG_PRINTF("Get info finished, leave %s, Current TP is %s \n",   __FUNCTION__, ((1 == priv->channel_info->work_mode)? "DVB-S2" : "DVB-S"));
	
    return SUCCESS;   
}

#ifdef FIND_ACM_TP
// For Debug
/*****************************************************************************
* INT32 nim_c3505_printf_tp_info(struct nim_device *dev)
*
* This function get tp info from hardware 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/

INT32 nim_c3505_printf_tp_info(struct nim_device *dev)
{
	struct nim_dvbs_channel_info *channel_info = NULL;
	struct nim_dvbs_bb_header_info *bb_header_info = NULL;
	struct nim_dvbs_isid *isid = NULL;

    UINT32 i = 0;
	
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}
	
	struct nim_c3505_private *priv = (struct nim_c3505_private *)dev->priv;
	channel_info = priv->channel_info;
	bb_header_info = priv->bb_header_info;
	isid = priv->isid; 


	 // For debug review
	 ACM_DEBUG_PRINTF("------------------------------------- \n");	 
	 ACM_DEBUG_PRINTF("Now printf BBFrame header information \n");
	 ACM_DEBUG_PRINTF("------------------------------------- \n");

	 if (1 == channel_info->work_mode)
	 {	 
		 for(i = 0; i < 10; i++)
			 ACM_DEBUG_PRINTF("bb_header.source_data[%d] = 0x%2x \n", i, bb_header_info->source_data[i]);

		 ACM_DEBUG_PRINTF("\n");


		 switch(bb_header_info->stream_mode)
		 {
			 case NIM_GS_PACKET:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_GS_PACKET\n");
				 break;
			 case NIM_GS_CONTINUE:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_GS_CONTINUE\n");
				 break;
			 case NIM_RESERVED:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_RESERVED\n");
				 break;
			 case NIM_TS:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_TS\n");
				 break;
			 default:
				 ACM_DEBUG_PRINTF("stream_mode is UNKNOWN\n");
				 break; 	  
		 }
	

		 if(NIM_STREAM_MULTIPLE == bb_header_info->stream_type)
		 {
			 ACM_DEBUG_PRINTF("stream_type is NIM_STREAM_MULTIPLE\n");
			 ACM_DEBUG_PRINTF("current isi :   %d\n", bb_header_info->isi);
		 }
		 else
			 ACM_DEBUG_PRINTF("stream_type is NIM_STREAM_SINGLE\n");

		 if(NIM_ACM == bb_header_info->acm_ccm)
			 ACM_DEBUG_PRINTF("acm_ccm is NIM_ACM/VCM\n");
		 else
			 ACM_DEBUG_PRINTF("acm_ccm is NIM_CCM\n");

		 if(NIM_ISSYI_ACTIVE == bb_header_info->issyi)
			 ACM_DEBUG_PRINTF("issyi is NIM_ISSYI_ACTIVE\n");
		 else
			 ACM_DEBUG_PRINTF("issyi is NIM_ISSYI_NOT_ACTIVE\n"); 

		 if(NIM_NPD_ACTIVE == bb_header_info->npd)
			 ACM_DEBUG_PRINTF("npd is NIM_NPD_ACTIVE\n");
		 else
			 ACM_DEBUG_PRINTF("npd is NIM_NPD_NOT_ACTIVE\n");

		 switch(bb_header_info->roll_off)
		 {
			 case NIM_RO_035:
				 ACM_DEBUG_PRINTF("roll_off is NIM_RO_035\n");
				 break;
			 case NIM_RO_025:
				 ACM_DEBUG_PRINTF("roll_off is NIM_RO_025\n");
				 break;
			 case NIM_RO_020:
				 ACM_DEBUG_PRINTF("roll_off is NIM_RO_020\n");
				 break;
			 case NIM_RO_RESERVED:
				 ACM_DEBUG_PRINTF("roll_off is NIM_RO_RESERVED\n");
				 break;
			 default:
				 ACM_DEBUG_PRINTF("roll_off is unknown\n");
				 break; 	  
		 }

		 ACM_DEBUG_PRINTF("upl   is  0x%x\n", bb_header_info->upl);
		 ACM_DEBUG_PRINTF("dfl   is  0x%x\n", bb_header_info->dfl);
		 ACM_DEBUG_PRINTF("sync  is  0x%x\n", bb_header_info->sync);
		 ACM_DEBUG_PRINTF("syncd is  0x%x\n\n", bb_header_info->syncd);

	  }

	 ACM_DEBUG_PRINTF("--------------------------------- \n");
	 ACM_DEBUG_PRINTF("Now printf Modulation information \n");
	 ACM_DEBUG_PRINTF("--------------------------------- \n");

	 switch(channel_info->work_mode)
	 {
		 case 0 : {ACM_DEBUG_PRINTF("Work_mode is DVB-S \n"); break;}
		 case 1 : {ACM_DEBUG_PRINTF("Work_mode is DVB-S2 \n"); break;}
		 case 2 : {ACM_DEBUG_PRINTF("Work_mode is DVB-H8PSK \n"); break;}
		 default: {ACM_DEBUG_PRINTF("Work_mode is unknown \n"); break;}  
	 }

	 if(0 == channel_info->work_mode)
	 {
		 switch(channel_info->code_rate)
		 {
			 case 0 : {ACM_DEBUG_PRINTF("Code_rate is 1/2\n"); break;}
			 case 1 : {ACM_DEBUG_PRINTF("Code_rate is 2/3\n"); break;}
			 case 2 : {ACM_DEBUG_PRINTF("Code_rate is 3/4\n"); break;}
			 case 3 : {ACM_DEBUG_PRINTF("Code_rate is 5/6\n"); break;}
			 case 4 : {ACM_DEBUG_PRINTF("Code_rate wrong\n"); break;}
			 case 5 : {ACM_DEBUG_PRINTF("Code_rate is 7/8\n"); break;}
			 default : {ACM_DEBUG_PRINTF("Code_rate is unknown\n"); break;}
		 }
	 
	 }
	 else if(1 == channel_info->work_mode)
	 {
		 switch(channel_info->code_rate)
		 {
			 case 0 : {ACM_DEBUG_PRINTF("Code_rate is 1/4\n"); break;}
			 case 1 : {ACM_DEBUG_PRINTF("Code_rate is 1/3\n"); break;}
			 case 2 : {ACM_DEBUG_PRINTF("Code_rate is 2/5\n"); break;}
			 case 3 : {ACM_DEBUG_PRINTF("Code_rate is 1/2\n"); break;}
			 case 4 : {ACM_DEBUG_PRINTF("Code_rate is 3/5\n"); break;}
			 case 5 : {ACM_DEBUG_PRINTF("Code_rate is 2/3\n"); break;}
			 case 6 : {ACM_DEBUG_PRINTF("Code_rate is 3/4\n"); break;}
			 case 7 : {ACM_DEBUG_PRINTF("Code_rate is 4/5\n"); break;}
			 case 8 : {ACM_DEBUG_PRINTF("Code_rate is 5/6\n"); break;}
			 case 9 : {ACM_DEBUG_PRINTF("Code_rate is 8/9\n"); break;}
			 case 10 : {ACM_DEBUG_PRINTF("Code_rate is 9/10\n"); break;}
			 default : {ACM_DEBUG_PRINTF("Code_rate is unknown\n"); break;}
		 }
	 
	 }
	 
	 if(1 == channel_info->work_mode)
	 {
		 switch(channel_info->roll_off)
		 {
			 case 0 : {ACM_DEBUG_PRINTF("Roll_off is 0.35\n");break;};
			 case 1 : {ACM_DEBUG_PRINTF("Roll_off is 0.25\n");break;};
			 case 2 : {ACM_DEBUG_PRINTF("Roll_off is 0.20\n");break;};
			 case 3 : {ACM_DEBUG_PRINTF("Roll_off is unknown\n");break;};
			 default : {ACM_DEBUG_PRINTF("Roll_off is unknown\n");break;};
		 }
	 }
	 
	 if(1 == channel_info->work_mode)
	 {
		 if(channel_info->pilot_mode)
			 ACM_DEBUG_PRINTF("Pilot on \n");
		 else
			 ACM_DEBUG_PRINTF("Pilot off \n");
		 ACM_DEBUG_PRINTF("Modcod is %d\n", channel_info->modcod);
	 }
	 
	 ACM_DEBUG_PRINTF("Iqswap_flag is %d\n", channel_info->iqswap_flag); 
	 if(1 == channel_info->work_mode)
	 {
		 switch(channel_info->map_type)
		 {
			 case 0 : {ACM_DEBUG_PRINTF("Map_type is HBCD\n"); break;}
			 case 1 : {ACM_DEBUG_PRINTF("Map_type is BPSK\n"); break;}
			 case 2 : {ACM_DEBUG_PRINTF("Map_type is QPSK\n"); break;}
			 case 3 : {ACM_DEBUG_PRINTF("Map_type is 8PSK\n"); break;}
			 case 4 : {ACM_DEBUG_PRINTF("Map_type is 16APSK\n"); break;}
			 case 5 : {ACM_DEBUG_PRINTF("Map_type is 32APSK\n"); break;}
			 default : {ACM_DEBUG_PRINTF("Map_type is unknow\n"); break;}
		 }
	 }
	 else
	 {
		 ACM_DEBUG_PRINTF("Map_type is QPSK\n");
	 }

	 ACM_DEBUG_PRINTF("Signal level is %d\n", channel_info->signal_level);
     ACM_DEBUG_PRINTF("Signal snr is %d dB\n\n", channel_info->snr);



	ACM_DEBUG_PRINTF("--------------------------------- \n");	
	ACM_DEBUG_PRINTF("    Now printf ISI information    \n");
	ACM_DEBUG_PRINTF("--------------------------------- \n");

	if(NIM_STREAM_SINGLE == bb_header_info->stream_type)
	{
		ACM_DEBUG_PRINTF("Is NIM_STREAM_SINGLE, there is no isid info\n\n");
		return SUCCESS;
	}

	ACM_DEBUG_PRINTF("There are %d Streams\n", isid->isid_num); 
    for(i = 0; i < isid->isid_num; i++)
    {
		ACM_DEBUG_PRINTF("isid[%d] = %d \n", i, isid->isid_read[i]);
    }

	ACM_DEBUG_PRINTF("\n");
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3505_show_acm_ccm_info(struct nim_device *dev)
*
* This function get tp info from hardware 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_show_acm_ccm_info(struct nim_device *dev)
{
	struct nim_dvbs_channel_info *channel_info = NULL;
	struct nim_dvbs_bb_header_info *bb_header_info = NULL;
	struct nim_dvbs_isid *isid = NULL;
	
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}
	
	struct nim_c3505_private *priv = (struct nim_c3505_private *)dev->priv;
	channel_info = priv->channel_info;
	bb_header_info = priv->bb_header_info;
	isid = priv->isid; 
	
	if (1 == channel_info->work_mode)
	{
	    ACM_DEBUG_PRINTF("\n");
		switch(bb_header_info->stream_mode)
		{
			 case NIM_GS_PACKET:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_GS_PACKET\n");
				 break;
			 case NIM_GS_CONTINUE:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_GS_CONTINUE\n");
				 break;
			 case NIM_RESERVED:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_RESERVED\n");
				 break;
			 case NIM_TS:
				 ACM_DEBUG_PRINTF("stream_mode is NIM_TS\n");
				 break;
			 default:
				 ACM_DEBUG_PRINTF("stream_mode is UNKNOWN\n");
				 break; 	  
		 }
		 if(NIM_ACM == bb_header_info->acm_ccm)
			 ACM_DEBUG_PRINTF("acm_ccm is NIM_ACM/VCM\n");
		 else
			 ACM_DEBUG_PRINTF("acm_ccm is NIM_CCM\n");
		 
		 if(NIM_STREAM_MULTIPLE == bb_header_info->stream_type)
		 {
			 ACM_DEBUG_PRINTF("stream_type is NIM_STREAM_MULTIPLE\n");
			/* 
             ACM_DEBUG_PRINTF("There are %d Streams\n", isid->isid_num); 
             for(i = 0; i < isid->isid_num; i++)
             {
        		ACM_DEBUG_PRINTF("isid[%d] = %d \n", i, isid->isid_read[i]);
             }
			 */
		 }
		 else
			 ACM_DEBUG_PRINTF("stream_type is NIM_STREAM_SINGLE\n");
		 
	}
	else
	{
		ACM_DEBUG_PRINTF("It is not a DVB-S2 TP\n");
	}
     return SUCCESS;
}

#endif

