/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 debug function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#include "nim_dvbs_c3505.h"

#ifdef DEBUG_IN_TASK
/*****************************************************************************
*  void nim_C3505_cr_adaptive_monitor(struct nim_device *dev)
* 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
void nim_c3505_cr_adaptive_monitor(struct nim_device *dev)
{
	static UINT8 interval_cnt = 0;
	UINT8 data = 0,  cr_prs = 0, snr_idx = 0, cr_irs = 0, cr_frac = 0,head_gain = 0;
	UINT32 rdata = 0;

	if(interval_cnt==0)
	{
		UINT8 data1 = 0xe6;

		nim_reg_write(dev,0x0e,&data1,1);
		//ByteDat = ReadMemB(0x180030bc);
		nim_reg_read(dev, RBB_SNR_RPT1, &data, 1);
		rdata = data;
		nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
		rdata |= data<<8;
		nim_reg_read(dev, R26_TR_LD_LPF_OPT, &data, 1);
		rdata |= data<<16;
		//if(rdata != old_cr_para)
		//{
		//  old_cr_para = rdata;	 
		cr_prs =(UINT8)((rdata>>9) & 0xf);
		cr_irs =(UINT8)((rdata>>4) & 0x1f);
		cr_frac = (UINT8)((rdata>>2) & 3);
		head_gain =  (UINT8)((rdata>>13) & 7);

		snr_idx =(UINT8) ((rdata>>16) & 0x1f);

		if((rdata & 1))
		{
			ADPT_CR_PRINTF("OLDCR \n");
		}
		ADPT_CR_PRINTF("SNR is:%d/10 dB, ",SNR_dBX10_TAB[snr_idx]);
		ADPT_CR_PRINTF("head_gain=%d, cr_prs=0x%x, cr_irs=0x%x, cr_frac=%d, wider_loop=%d\n",head_gain, cr_prs,cr_irs,cr_frac,(rdata>>1 & 1));
		interval_cnt = 6;
	}

	interval_cnt--;

#ifdef RPT_CR_SNR_EST
	UINT8 ByteDat;
	UINT16 SNR_EST;
	//nim_reg_read(dev,0xe7,ByteDat,1);
	//ByteDat |= 0x80;
	//nim_reg_write(dev,0xe7, &ByteDat,1);

	ADPT_CR_PRINTF("\t\tenter RPT_CR_SNR_EST block\n");
	ByteDat = 0xe4;// ADPT_CR_DEBUG=0
	nim_reg_write(dev,0x0e,&ByteDat,1);

	nim_reg_read(dev,0xbc,&ByteDat,1);
	ByteDat &= 0x7f; // reg_crbc[7]=0
	nim_reg_write(dev,0xbc,&ByteDat,1);
	//if (ByteDat & 0x80)
	{
		//NIM_PRINTF("\tenter the if block");
		nim_reg_read(dev,0xbc,&ByteDat,1);
		SNR_EST = (ByteDat & 0x1f) << 6;
		nim_reg_read(dev,0xbb,&ByteDat,1);  

		SNR_EST += (ByteDat>>2) & 0x03f;
		ByteDat=0x00;
		nim_reg_write(dev,0xbc, &ByteDat,1);

		//if(var>var_max)
		//var_max=var;
		//  if(var<var_min)
		//  var_min = var;
		ADPT_CR_PRINTF("SNR_EST=%d\n",SNR_EST);
	}
	//nim_reg_read(dev,0xe7,ByteDat,1);
	//ByteDat &= 0x7f;
	//nim_reg_write(dev,0xe7, &ByteDat,1);
#endif

};

/*****************************************************************************
*  void nim_C3505_cr_new_adaptive_monitor(struct nim_device *dev)
* Get main parameters of CR and give snr_est value to estimate c/n
* 
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
void nim_c3505_cr_new_adaptive_monitor(struct nim_device *dev)
{
	// Adaptive parameter display	 // add by hongyu
	/*
	*	 these value is that will send to cr module and display here for debug
	*	 the whole is 63'bit and 20 parameters
	*	 ADPT_PRS_TRA_FRAC[1:0]=
	*	 ADPT_IRS_TRA_FRAC[1:0]=
	*	 ADPT_CR_PRS_TRA_S2[3:0]=
	*	 ADPT_CR_IRS_TRA_S2[4:0]=
	*	 ADPT_CR_HEAD_PED_GAIN[2:0]=
	*	 ADPT_WIDER_LOOP_EN=
	*	 ADPT_FORCE_OLD_CR=
	*	 ADPT_AVG_HEAD_2HALF_EN=
	*	 ADPT_WIDER_IRS_DELTA[2:0]=
	*	 ADPT_WIDER_PRS_DELTA[2:0]=
	*	 ADPT_SYMBOL_BEFORE_HEAD=
	*	 ADPT_LLR_SHIFT=
	*	 ADPT_AVG_PED1_EN=
	*	 ADPT_AVG_PED1[7:0]=
	*	 ADPT_AVG_PED2_EN=
	*	 ADPT_AVG_PED2[7:0]= 
	*	 ADPT_CLIP_PED1_EN=
	*	 ADPT_CLIP_PED1[7:0]=
	*	 ADPT_CLIP_PED2_EN=
	*	 ADPT_CLIP_PED2[7:0]=

	wire [7:0] reg_cr13b = EST_NOISE[7:0];
	wire [7:0] reg_cr13c = {3'b0,EST_NOISE[12:8]};
	wire [7:0] reg_cr13d = {5'b0,
	ADPT_WIDER_LOOP_EN,
	ADPT_SYMBOL_BEFORE_HEAD,
	ADPT_LLR_SHIFT
	};
	wire [7:0] reg_cr13e = {
	ADPT_FORCE_OLD_CR,
	ADPT_WIDER_PRS_DELTA[2:0],
	ADPT_AVG_HEAD_2HALF_EN,
	ADPT_WIDER_IRS_DELTA[2:0]
	};
	wire [7:0] reg_cr13f = {
	ADPT_CR_PRS_TRA_S2[3:0],
	ADPT_PRS_TRA_FRAC[1:0],
	ADPT_IRS_TRA_FRAC[1:0]
	};
	wire [7:0] reg_cr140 = {
	ADPT_CR_IRS_TRA_S2[4:0],
	ADPT_CR_HEAD_PED_GAIN[2:0]
	};
	wire [7:0] reg_cr141 = {4'b0,
	ADPT_CLIP_PED2_EN,
	ADPT_CLIP_PED1_EN,
	ADPT_AVG_PED2_EN,
	ADPT_AVG_PED1_EN
	};
	wire [7:0] reg_cr142 = ADPT_AVG_PED1[7:0];
	wire [7:0] reg_cr143 = ADPT_AVG_PED2[7:0];
	wire [7:0] reg_cr144 = ADPT_CLIP_PED1[7:0];
	wire [7:0] reg_cr145 = ADPT_CLIP_PED2[7:0];


	*/
	/*
	R13b_EST_NOISE = 0x13b,
	R13d_ADPT_CR_PARA_0 = 0x13d,
	R140_ADPT_CR_PARA_1 = 0x140,
	R144_ADPT_CR_PARA_2 = 0x144,
	*/
	//UINT8 ap1e,ap2e,cp1e,cp2e;
	//UINT8 cp2, cp1, ap2,ap1,ls,sbh,wle,wid,ah2e,wpd,old,itf,ptf,p,hpg,i;

	UINT8 rdata[4] = {0};

	nim_reg_read(dev, R13d_ADPT_CR_PARA_0, rdata, 3);
	// reg_cr13d
	UINT8 ADPT_LLR_SHIFT = rdata[0] & 0x01;//ls .
	UINT8 ADPT_SYMBOL_BEFORE_HEAD = (rdata[0] >> 1) & 0x01;//sbh .
	UINT8 ADPT_WIDER_LOOP_EN = (rdata[0] >> 2) & 0x01;//wle .
	// reg_cr13e
	UINT8 ADPT_WIDER_IRS_DELTA = rdata[1] & 0x07;//wid .
	UINT8 ADPT_AVG_HEAD_2HALF_EN = (rdata[1] >> 3) & 0x01;//ah2e .
	UINT8 ADPT_WIDER_PRS_DELTA = (rdata[1] >> 4) & 0x07;//wpd .
	UINT8 ADPT_FORCE_OLD_CR = (rdata[1] >> 7) & 0x01;//old .
	// reg_cr13f
	UINT8 ADPT_IRS_TRA_FRAC =  rdata[2] & 0x03;//itf .
	UINT8 ADPT_PRS_TRA_FRAC =  (rdata[2] >> 2) & 0x03;//ptf .
	UINT8 ADPT_CR_PRS_TRA_S2 = (rdata[2] >> 4) & 0x0f;//p .

	nim_reg_read(dev,R140_ADPT_CR_PARA_1, rdata, 4);
	// reg_cr140
	UINT8 ADPT_CR_HEAD_PED_GAIN = rdata[0] & 0x07;//hpg .
	UINT8 ADPT_CR_IRS_TRA_S2 = (rdata[0] >> 3) & 0x1f;
	//// reg_cr141
	UINT8 adpt_avg_ped1_en  =  rdata[1] & 0x01; //ap1e .
	UINT8 adpt_avg_ped2_en  = (rdata[1] >> 1) & 0x01;//ap2e .
	UINT8 adpt_clip_ped1_en = (rdata[1] >> 2) & 0x01;//cp1e .
	UINT8 adpt_clip_ped2_en = (rdata[1] >> 3) & 0x01;//cp2e .
	// reg_cr142 & reg_cr143
	UINT8 adpt_avg_ped1 = rdata[2];//ap1 .
	UINT8 adpt_avg_ped2 = rdata[3];//ap2 .

	nim_reg_read(dev,R144_ADPT_CR_PARA_2,rdata,2);
	// reg_cr144 & reg_cr145
	UINT8 adpt_clip_ped1 = rdata[0];//cp1.
	UINT8 adpt_clip_ped2 = rdata[1];//cp2 .


	ADPT_NEW_CR_PRINTF("p=%x i=%x ptf=%x itf=%x ap1e=%x ap1=%x ap2e=%x ap2=%x cp1e=%x cp1=%x cp2e=%x cp2=%x hpg=%x sbh=%x ah2e=%x ls=%x old=%x wpd=%x wid=%x wle=%x\n",
		ADPT_CR_PRS_TRA_S2,	\
		ADPT_CR_IRS_TRA_S2,	\
		ADPT_PRS_TRA_FRAC,	\
		ADPT_IRS_TRA_FRAC,	\
		adpt_avg_ped1_en,	\
		adpt_avg_ped1,		\
		adpt_avg_ped2_en,	\
		adpt_avg_ped2,		\
		adpt_clip_ped1_en,	\
		adpt_clip_ped1,		\
		adpt_clip_ped2_en,	\
		adpt_clip_ped2,		\
		ADPT_CR_HEAD_PED_GAIN,\
		ADPT_SYMBOL_BEFORE_HEAD,\
		ADPT_AVG_HEAD_2HALF_EN,\
		ADPT_LLR_SHIFT,\
		ADPT_FORCE_OLD_CR,\
		ADPT_WIDER_PRS_DELTA,\
		ADPT_WIDER_IRS_DELTA,\
		ADPT_WIDER_LOOP_EN\
	);
#ifdef NEW_CR_ADPT_SNR_EST_RPT
	//nim_c3503_get_snr_cn(dev);
#endif

}


 
/*****************************************************************************
*  INT32 nim_C3505_cr_sw_adaptive(struct nim_device *dev)
* 
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_C3505_cr_sw_adaptive(struct nim_device *dev)
{
	UINT8 data = 0;
	static UINT16 snr_lpf = 0;
	UINT16 var = 0;
	UINT16 cr_data = 0;
	UINT8 frac = 0,prs = 0,irs = 0;
	UINT32 *cr_para = NULL;
	INT32 snr_chg = 0;
	INT32 i = 0;
	static INT32 interval_cnt = 0;
	static INT32 curr_snr_idx = 0;

	//ByteDat = ReadMemB(0x180030bc);
	nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
	if (data & 0x80) // SNR estimation data ready
	{
		var = (data & 0x0f) << 6;
		//ByteDat = ReadMemB(0x180030bb);
		nim_reg_read(dev, RBB_SNR_RPT1, &data, 1); // read SNR LSB
		var += (data>>2) & 0x03f;

		// slide filter
		if(snr_initial_en)
			snr_lpf = var;
		else
		{
			snr_lpf = (snr_lpf*7+var)>>2;
			snr_lpf += 1;
			snr_lpf >>= 1;
		}
		interval_cnt++;

		if(interval_cnt == 16)
		{
			interval_cnt = 0;
			//NIM_PRINTF("*** SNR = %u \r\n", snr_lpf);
		}

		if(snr_initial_en) // for first SNR data
		{
			curr_snr_idx = SNR_TAB_SIZE - 1;
			for(i = SNR_TAB_SIZE - 1; i >= 0; i--)
			{
				if(var >= c3505_snr_thr[i*2])
					curr_snr_idx = i;
				else
					break;
			}

			snr_initial_en = 0;
			snr_chg = 1;
			snr_lpf = var;
		}
		else if(curr_snr_idx > 0 && snr_lpf >= c3505_snr_thr[2*(curr_snr_idx - 1)] \
			&& (interval_cnt == 0)) // switch to lower snr
		{
			curr_snr_idx--;
			snr_chg = 1;
		}
		else if(curr_snr_idx < SNR_TAB_SIZE - 1 && snr_lpf <= c3505_snr_thr[2*curr_snr_idx+3] \
			&& (interval_cnt==0)) // to higher snr
		{
			curr_snr_idx++;
			snr_chg = 1;
		}

		if(snr_chg)
		{
			cr_para = (UINT32 *)c3505_cr_para_8psk_3f5;
			cr_data = cr_para[curr_snr_idx];
			frac = cr_data & 0x0f;
			prs = (cr_data>>4) & 0x0f;
			irs = (cr_data>>8) & 0x1f;
			data = (((irs & 0x03)<<4) | frac)<<2;
			//WriteMemB(0x180030bb, data);
			nim_reg_write(dev, RBB_SNR_RPT1, &data, 1);

			data = ((irs>>2)<<4) | prs | 0x80;
			//WriteMemB(0x180030bc, data);
			nim_reg_write(dev, RBC_SNR_RPT2, &data, 1);

			data = 0xe8;
			//WriteMemB(0x1800300e, 0xe8); // enable cr para update
			nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
			
			data = (curr_snr_idx&0x01) ? 5:0;
			NIM_PRINTF("--->switch to %d.%d dB Setting \n", 6+curr_snr_idx/2,data);
		}

		data = 0x00;
		nim_reg_write(dev, RBC_SNR_RPT2, &data, 1); // clear SNR ready flag

		}

	return SUCCESS;
}



 /*****************************************************************************
*  INT32 nim_C3505_debug_intask(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_debug_intask(struct nim_device *dev)
{
    static UINT32 count = 0;
    UINT8 run_flag = 0;
    
#ifdef NEW_CR_ADPT_SNR_EST_RPT
	UINT16  snr_db;
#endif

    nim_reg_read(dev, 0xed, &run_flag, 1); 
    if (run_flag&0x01)
        return SUCCESS;
		
	NIM_PRINTF(" Enter function nim_C3505_debug_intask \n");

#ifdef HW_ADPT_CR
	#ifdef HW_ADPT_CR_MONITOR
		nim_c3505_cr_adaptive_monitor(dev);
	#endif    
	#ifdef HW_ADPT_NEW_CR_MONITOR
		nim_c3505_cr_new_adaptive_monitor(dev);
		#ifdef NEW_CR_ADPT_SNR_EST_RPT
			nim_c3505_get_snr_db(dev,&snr_db);
		#endif
	#endif
#else
	#ifdef SW_ADPT_CR
		#ifdef SW_SNR_RPT_ONLY
			nim_c3505_cr_sw_snr_rpt(dev);
		#else
			nim_C3505_cr_sw_adaptive(dev);
		#endif
	#endif
#endif

#ifdef C3505_DEBUG_FLAG
        NIM_PRINTF("\n\nNim_c3505_debug_intask, Current system time: %d\n", osal_get_tick());
    
        // Loct status monitor
        nim_c3505_mon_lock_status(dev);
    
        // Signal Monitor
        nim_c3505_mon_signal(dev);
        
        // Read register
	#ifdef DEBUG_REGISTER
		count++;
		if (count >= 20)
		{
        	nim_c3505_mon_reg(dev);
			count = 0;
		}
    #endif
#endif	

	return SUCCESS;
}

#endif


#ifdef C3505_DEBUG_FLAG
/*****************************************************************************
* INT32 nim_c3505_mon_signal(struct nim_device *dev)
*
*	This function will monitor signal status  for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_mon_signal(struct nim_device *dev)
{
    UINT8 data = 0;
    INT16 snr = 0;
    INT8 agc = 0;
	UINT8 per_flag = 0, unlock_ierr = 0, status = 0, cr_phase_err = 0, pl_phase_err = 0;
	UINT32 per = 0, ber = 0;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_get_agc_dbm(dev , &agc);
	
    per = priv->channel_info->per;

	nim_c3505_get_new_ber(dev , &ber);
 
    snr = priv->channel_info->snr;

#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	NIM_PRINTF("In %s, level = %d dBm, per = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
#else
	NIM_PRINTF("In %s, level = %d dBm, fer = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
#endif
	nim_reg_read(dev, R02_IERR, &data, 1);
	unlock_ierr = data;
	nim_reg_read(dev, R04_STATUS, &data, 1);	 
	status = data;
    
	// read cr phase error
    cr_phase_err = priv->channel_info->phase_err;

#if 0
	//read pl phase error
	nim_reg_read(dev, 0xe0, &data, 1); 
	data = data | 0x02;
    data = data & 0x7f;
	nim_reg_write(dev, 0xe0, &data, 1);			 

	//read out the phase error
	nim_reg_read(dev, 0xef, &data, 1); 	 
	pl_phase_err = data;
#endif

	if((1 == per_flag) && (0 == per))
	{
		per_flag = 0 ; 	 
	}

	if((0 != per) && (1010 != per)) 
	{
		NIM_PRINTF("---status: %d,---unlock_ierr: %d\n",status,unlock_ierr);
		NIM_PRINTF("---ber: %d,------snr: %d, ----------cr phase_err: %d,pl phase_err: %d \n",ber,snr,cr_phase_err,pl_phase_err);
		NIM_PRINTF("---Per !=0 happpened, FFT start log! per = %d\n",per);

		nim_reg_read(dev, R113_NEW_CR_ADPT_CTRL, &data, 1);
		data = data | 0x01 ;
		nim_reg_write(dev,R113_NEW_CR_ADPT_CTRL, &data, 1);

		nim_reg_read(dev,R130_CR_PARA_DIN+7,&data,1);
		data = data | 0x80 ;
		nim_reg_write(dev,R130_CR_PARA_DIN+7,&data,1);  
		per_flag = 1 ;  
	}

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3505_mon_lock_status(struct nim_device *dev)
*
*	This function will monitor demod lock status for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_mon_lock_status(struct nim_device *dev)
{
    static UINT8 lock_status = 0;
    UINT8 lock_status_tmp = 0;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    // Lock status monitor
    //nim_c3505_get_lock(dev, &lock_status_tmp);
    nim_reg_read(dev, 0x04, &lock_status_tmp, 1);
	
    if (lock_status_tmp != lock_status)
    {
        lock_status = lock_status_tmp;  
        NIM_PRINTF("&&&&&&&& lock status reg04 = 0x%x\n", lock_status_tmp);
    
        nim_reg_read(dev, 0x02, &lock_status_tmp, 1);   
        NIM_PRINTF("reg02 = 0x%x\n", lock_status_tmp);

        lock_status_tmp = 0x00;
        nim_reg_write(dev, 0x02, &lock_status_tmp, 1);                 
    }  

    return SUCCESS;
}


#endif


#ifdef DEBUG_REGISTER
/*****************************************************************************
* INT32 nim_c3505_mon_signal(struct nim_device *dev)
*
*	This function will monitor demod register for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_mon_reg(struct nim_device *dev)
{
    UINT8 data = 0;
	UINT8 i = 0, j = 0;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    libc_printf("\n\nDEBUG_REGISTER");
    
    for (i = 0; i < 32; i++)
    {
        libc_printf("\nRead reg[0x%3x]: ", (i*16));
        for (j = 0; j < 16; j++)
        {
            nim_reg_read(dev, (j + i*16), &data, 1);
            libc_printf("%2x ", data);
        }
    }
    
    libc_printf("\n\n");

    return SUCCESS;
}
#endif


