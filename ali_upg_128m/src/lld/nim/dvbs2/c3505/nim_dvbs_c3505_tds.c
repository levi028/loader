/*****************************************************************************
*	Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*	File:	This file contains C3505 DVBS2 system API and hardware 
*           open/closed operate in LLD.
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/
#include <api/libfs2/stdio.h>

#include "nim_dvbs_c3505.h"

extern INT32 c3505_real_freq[];
extern INT32 c3505_real_sym_rate[];
extern INT32 c3505_real_tp_number;

static UINT8 nim_task_num = 0x00;
//static UINT8 demod_mutex_flag = 0;
//UINT8 demod_mutex_flag = 0;


/* Name for the demodulator, the last character must be Number for index */
char nim_c3505_name[3][HLD_MAX_NAME_SIZE] =
{
	"NIM_S3505_0", "NIM_S3505_1", "NIM_S3505_2"
};

void nim_usb_log(char* msg);

/*****************************************************************************
Name:
    multu64div
Description:
    This function implment v1*v2/v3. And v1*v2 is 64 bit
Parameters:
    [IN]
    [OUT]
Return:
***********************************************************************/
//Get HI value from the multple result
#ifdef WIN32
DWORD nim_s3505_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
{
    DWORD v;
    if (v3 == 0)
        return 0;
	v = (UINT64)(v1)*(UINT64)(v2)/v3;
	return v;
}
#else
# define mult64hi(v1, v2)           \
({  DWORD __ret;            \
    __asm__ volatile(                   \
        "multu  %1, %2\n    "               \
        "mfhi %0\n" \
         : "=r" (__ret)         \
         : "r" (v1), "r"(v2));                  \
    __ret;                      \
})
//Get LO value from the multple result
# define mult64lo(v1, v2)           \
({  DWORD __ret;            \
    __asm__ volatile(                   \
        "multu  %1, %2\n    "               \
        "mflo %0\n" \
         : "=r" (__ret)         \
         : "r" (v1), "r"(v2));                  \
    __ret;                      \
})


DWORD nim_c3505_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
{
    DWORD hi, lo;
    UINT64 tmp;
    DWORD *tmp64;
    DWORD ret;
    if (v3 == 0)
        return 0;
    hi = mult64hi(v1, v2);
    lo = mult64lo(v1, v2);
    tmp64 = ((DWORD *)(&tmp))+1;
    *tmp64-- = hi;
    *tmp64 = lo;
    //Few nop here seems required, if no nop is here,
    //then the result wont be correct.
    //I guess maybe this is caused by some GCC bug.
    //Because I had checked the compiled result between with nop and without nop,
    //they are quite different!! I dont have time to search more GCC problem,
    //Therefore, I can only leave the nop here quietly. :(
    //--Michael 2003/10/10
    __asm__("nop; nop; nop; nop");
    ret = tmp/v3;
    return ret;
}
#endif


INT32 nim_c3505_crc8_check(void *input, INT32 len,INT32 polynomial)
{
	/*
	   Func: Implement CRC decoder based on the structure of 
	         figure 2 in DVB-S2 specification (ETSI EN 302 307).
	*/

	INT32 i = 0;
	INT32 j = 0;
	INT32 bit_idx     = 0;

	INT8 *byte_data = NULL;
	INT8 curr_byte  = 0;
	UINT8 shift_reg  = 0;
	INT8 xor_flag   = 0;
	INT8  bit_data  = 0;

    //polynomial = 0x56;// 0x56  test
	byte_data = (INT8 *)input;
	curr_byte = byte_data[i];
	
	while (bit_idx < len )
	{
		bit_data = (curr_byte & 0x80) >> 7;
		bit_idx ++;
		j ++;

		// get one bit  from MSB of a byte
		if (j == 8)
		{
			curr_byte = byte_data[++i]; // input new byte 
			j = 0;
		}
		else
		{   // get next bit of curr_byte
			curr_byte <<= 1;
		}

		// crc check process
		xor_flag = bit_data ^ (shift_reg & 0x01);
		if (xor_flag)
		{
			shift_reg ^= polynomial;  // 0x56 is derived by the tap structure in figure 2 of DVB-S2 Spec.
		}

		shift_reg >>= 1;
		if (xor_flag)
		{
			shift_reg |= 0x80;  // MSB of shift_reg is set to 1.
		}

		//BBFRAME_PRINTF("%d\n",shift_reg);
	}

	if (0 == shift_reg)
	{
		return SUCCESS;
	}
	else
	{
		return ERR_FAILED;
	}
}


UINT32 nim_c3505_Log10Times100_L( UINT32 x)
{
	static const UINT8 scale=15;
	static const UINT8 indexWidth=5;
	/*
	log2lut[n] = (1<<scale) * 200 * log2( 1.0 + ( (1.0/(1<<INDEXWIDTH)) * n ))
	0 <= n < ((1<<INDEXWIDTH)+1)
	*/
	static const UINT32 log2lut[] = {
	0, 290941,  573196,	847269,1113620, 1372674, 1624818, 
	1870412, 2109788, 2343253, 2571091, 2793569,3010931, 
	3223408, 3431216, 3634553, 3833610, 4028562, 4219576, 
	4406807, 4590402, 4770499, 4947231, 5120719, 5291081, 
	5458428, 5622864, 5784489, 5943398,	6099680, 6253421, 
	6404702,  6553600  };

	UINT8  i = 0;
	UINT32 y = 0;
	UINT32 d = 0;
	UINT32 k = 0;
	UINT32 r = 0;
 
	if (x==0) return (0);
 
	/* Scale x (normalize) */
	/* computing y in log(x/y) = log(x) - log(y) */
	if ( (x & (((UINT32)(-1))<<(scale+1)) ) == 0 )
	{
	   for (k = scale; k>0 ; k--)
	   {
		 if (x & (((UINT32)1)<<scale)) break;
		 x <<= 1;
	   }
	} else {
	   for (k = scale; k<31 ; k++)
	   {
		 if ((x & (((UINT32)(-1))<<(scale+1)))==0) break;
		 x >>= 1;
	   }
	}
	/*
	  Now x has binary point between bit[scale] and bit[scale-1]
	  and 1.0 <= x < 2.0 */
 
	/* correction for divison: log(x) = log(x/y)+log(y) */
	y = k * ( ( ((UINT32)1) << scale ) * 200 );
 
	/* remove integer part */
	x &= ((((UINT32)1) << scale)-1);
	/* get index */
	i = (UINT8) (x >> (scale -indexWidth));
	/* compute delta (x-a) */
	d = x & ((((UINT32)1) << (scale-indexWidth))-1);
	/* compute log, multiplication ( d* (.. )) must be within range ! */
	y += log2lut[i] + (( d*( log2lut[i+1]-log2lut[i] ))>>(scale-indexWidth));
	/* Conver to log10() */
	y /= 108853; /* (log2(10) << scale) */
	r = (y>>1);
	/* rounding */
	if (y&((UINT32)1)) r++;
 
	return (r);
 
}

/*****************************************************************************
* INT32 nim_c3505_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
* Description: C3505 initialization
*
* Arguments:
* struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner)
{
	static unsigned char nim_dev_num = 0;
	INT32 channel_freq_err;
	struct nim_device *dev;
	struct nim_c3505_private *priv_mem;
	struct nim_dvbs_channel_info *channel_info_mem = NULL;
	struct nim_dvbs_bb_header_info *bb_header_info_mem = NULL;
	struct nim_dvbs_isid *isid_mem = NULL;

	NIM_PRINTF("Enter function: %s!\n",__FUNCTION__);

	if (ptr_qpsk_tuner == NULL)
	{
		NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
		return ERR_NO_DEV;
	}
	if (nim_dev_num > 2)
	{
		NIM_PRINTF("Can not support three or more C3505 !/n");
		return ERR_NO_DEV;
	}

	dev = (struct nim_device *) dev_alloc(nim_c3505_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	priv_mem = (struct nim_c3505_private *) comm_malloc(sizeof(struct nim_c3505_private));
	if (priv_mem == NULL)
	{
		dev_free(dev);
		NIM_PRINTF("Alloc nim device prive memory error!/n");
		return ERR_NO_MEM;
	}
	comm_memset((int *)priv_mem, 0, sizeof(struct nim_c3505_private));
	dev->priv = (void *) priv_mem;
    
    // alloc dev priv channel_info
	channel_info_mem = (struct nim_dvbs_channel_info *) comm_malloc(sizeof(struct nim_dvbs_channel_info));
	if (channel_info_mem == NULL)
	{		
		comm_free(priv_mem);
		dev_free(dev);
		NIM_PRINTF("Alloc nim device prive channel_info memory error!/n");
		return ERR_NO_MEM;
	}
	comm_memset(channel_info_mem, 0, sizeof(struct nim_dvbs_channel_info));
	priv_mem->channel_info = channel_info_mem;

    // alloc dev priv bb_header_info
	bb_header_info_mem = (struct nim_dvbs_bb_header_info *) comm_malloc(sizeof(struct nim_dvbs_bb_header_info));
	if (bb_header_info_mem == NULL)
	{				
		comm_free(channel_info_mem);
		comm_free(priv_mem);
		dev_free(dev);
		NIM_PRINTF("Alloc nim device prive bb_header_info memory error!/n");
		return ERR_NO_MEM;
	}
	comm_memset(bb_header_info_mem, 0, sizeof(struct nim_dvbs_bb_header_info));
	priv_mem->bb_header_info = bb_header_info_mem;

    // alloc dev priv isid
    isid_mem = (struct nim_dvbs_isid *) comm_malloc(sizeof(struct nim_dvbs_isid));
    NIM_PRINTF("comm_malloc isid addr = %x \n", isid_mem);
	if (isid_mem == NULL)
	{				
		comm_free(channel_info_mem);
		comm_free(bb_header_info_mem);
		comm_free(priv_mem);
		dev_free(dev);
		NIM_PRINTF("Alloc nim device prive isid memory error!/n");
		return ERR_NO_MEM;
	}
	comm_memset(isid_mem, 0, sizeof(struct nim_dvbs_isid));
	priv_mem->isid = (void*)isid_mem;
    
	//diseqc state inits
	dev->diseqc_info.diseqc_type = 0;
	dev->diseqc_info.diseqc_port = 0;
	dev->diseqc_info.diseqc_k22 = 0;

	if ((ptr_qpsk_tuner->config_data.qpsk_config & C3505_POLAR_REVERT) == C3505_POLAR_REVERT) //bit4: polarity revert.
		dev->diseqc_info.diseqc_polar = LNB_POL_V;
	else //default usage, not revert.
		dev->diseqc_info.diseqc_polar = LNB_POL_H;

	dev->diseqc_typex = 0;
	dev->diseqc_portx = 0;

	/* Function point init */
	dev->base_addr = ptr_qpsk_tuner->ext_dm_config.i2c_base_addr;
	dev->init = nim_c3505_attach;
	dev->open = nim_c3505_open;
	dev->stop = nim_c3505_close;
	dev->do_ioctl = nim_c3505_ioctl;
	dev->do_ioctl_ext = nim_c3505_ioctl_ext;
	dev->get_lock = nim_c3505_get_lock;
	dev->get_freq = nim_c3505_get_cur_freq;//nim_c3505_get_freq;
	dev->get_fec = nim_c3505_reg_get_code_rate;
	dev->get_snr = nim_c3505_get_snr;
    dev->set_polar = nim_c3505_set_polar;
    dev->set_12v = nim_c3505_set_12v;
    dev->channel_search = (void *)nim_c3505_channel_search;
    dev->di_seq_c_operate = nim_c3505_diseqc_operate;
    dev->di_seq_c2x_operate = nim_c3505_diseqc2x_operate;
    dev->get_sym = nim_c3505_get_symbol_rate;
    dev->get_agc = nim_c3505_get_agc;
    dev->get_ber = nim_c3505_get_ber;
    dev->get_fft_result = nim_c3505_get_fft_result;
    dev->get_ver_infor = NULL;//nim_C3505_get_ver_infor;

	/* tuner configuration function */
	priv_mem->nim_tuner_init = ptr_qpsk_tuner->nim_tuner_init;
	priv_mem->nim_tuner_control = ptr_qpsk_tuner->nim_tuner_control;
	priv_mem->nim_tuner_status = ptr_qpsk_tuner->nim_tuner_status;
	
    //add by dennis on 20140906 for c3031b Tuner
	priv_mem->nim_tuner_command = ptr_qpsk_tuner->nim_tuner_command;
	priv_mem->nim_tuner_gain = ptr_qpsk_tuner->nim_tuner_gain;
    
	priv_mem->i2c_type_id = ptr_qpsk_tuner->tuner_config.i2c_type_id;
	comm_memcpy(&priv_mem->tuner_config_data,&ptr_qpsk_tuner->config_data,sizeof(ptr_qpsk_tuner->config_data));
	priv_mem->ext_dm_config.i2c_type_id = ptr_qpsk_tuner->ext_dm_config.i2c_type_id;
	priv_mem->ext_dm_config.i2c_base_addr = ptr_qpsk_tuner->ext_dm_config.i2c_base_addr;

	priv_mem->search_type = NIM_OPTR_CHL_CHANGE;
	priv_mem->autoscan_debug_flag = 0;

	priv_mem->ul_status.m_enable_dvbs2_hbcd_mode = 0;
	priv_mem->ul_status.m_dvbs2_hbcd_enable_value = 0x7f;
	priv_mem->ul_status.nim_c3505_sema = OSAL_INVALID_ID;
	priv_mem->ul_status.c3505_autoscan_stop_flag = 0;
	priv_mem->ul_status.c3505_chanscan_stop_flag = 0;
	priv_mem->ul_status.old_ber = 0;
	priv_mem->ul_status.old_per = 0;
	priv_mem->ul_status.m_hw_timeout_thr = 0;
	priv_mem->ul_status.old_ldpc_ite_num = 0;
	priv_mem->ul_status.c_rs = 0;
	priv_mem->ul_status.phase_err_check_status = 0;
	priv_mem->ul_status.c3505_lock_status = NIM_LOCK_STUS_NORMAL;
	priv_mem->ul_status.m_c3505_type = 0x00;
	priv_mem->ul_status.m_setting_freq = 123;
	priv_mem->ul_status.m_err_cnts = 0x00;
	priv_mem->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
	priv_mem->tsk_status.m_task_id = OSAL_INVALID_ID;//0x00;
	priv_mem->t_param.t_aver_snr = -1;
	priv_mem->t_param.t_last_iter = -1;
	priv_mem->t_param.t_last_snr = -1;
	priv_mem->t_param.t_snr_state = 0;
	priv_mem->t_param.t_snr_thre1 = 256;
	priv_mem->t_param.t_snr_thre2 = 256;
	priv_mem->t_param.t_snr_thre3 = 256;
	priv_mem->t_param.t_dynamic_power_en = 0;
	priv_mem->t_param.t_phase_noise_detected = 0;
	priv_mem->t_param.t_reg_setting_switch = 0x0f;
	priv_mem->t_param.t_i2c_err_flag = 0x00;
	priv_mem->flag_id = OSAL_INVALID_ID;
	priv_mem->blscan_mode = NIM_SCAN_SLOW; // this mode may abandoned

	priv_mem->plsn_task_id = OSAL_INVALID_ID;
	priv_mem->generate_table_task_id = OSAL_INVALID_ID;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		NIM_PRINTF("Error: Register nim device error!\n");
		comm_free(isid_mem);
		comm_free(channel_info_mem);
		comm_free(bb_header_info_mem);
		comm_free((int*)priv_mem);		
		dev_free(dev);
		return ERR_NO_DEV;
	}
	nim_dev_num++;
	priv_mem->ul_status.nim_c3505_sema = NIM_MUTEX_CREATE(1);

	priv_mem->c3505_mutex = osal_mutex_create();
	if(priv_mem->c3505_mutex == OSAL_INVALID_ID)
	{
		ERR_PRINTF("Error: Register nim device error!\n");
		return ERR_FAILUE;
	}

    priv_mem->scan_mutex = osal_mutex_create();
	if(priv_mem->scan_mutex == OSAL_INVALID_ID)
	{
		ERR_PRINTF("create priv_mem->scan_mutex error!\n");
		return ERR_FAILUE;
	}

	priv_mem->plsn.plsn_mutex = osal_mutex_create();
	if(OSAL_INVALID_ID == priv_mem->plsn.plsn_mutex)
	{
		ERR_PRINTF("create priv_mem->OSAL_INVALID_ID error!\n");
		return ERR_FAILUE;
	} 
	
	nim_c3505_set_ext_lnb(dev, ptr_qpsk_tuner);
	nim_c3505_reg_get_chip_type(dev);
	ptr_qpsk_tuner->device_type = priv_mem->ul_status.m_c3505_type;
	// Initial the QPSK Tuner
	if (priv_mem->nim_tuner_init != NULL)
	{
		NIM_PRINTF(" Initial the Tuner \n");
		if (((struct nim_c3505_private *) dev->priv)->nim_tuner_init(&priv_mem->tuner_index, &(ptr_qpsk_tuner->tuner_config)) != SUCCESS)
		{
			NIM_PRINTF("Error: Init Tuner Failure!\n");
			return ERR_NO_DEV;
		}
        // get tuner type
		if(SUCCESS == call_tuner_command(dev, NIM_TUNER_C3031B_ID, &channel_freq_err))
			priv_mem->tuner_type = IS_M3031;
		else
			priv_mem->tuner_type = NOT_M3031;
	}
	

	return SUCCESS;
}
/*****************************************************************************
* INT32 nim_c3505_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
*
*  device input/output operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 cmd
*  Parameter3: UINT32 param
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	INT32 cr_num = 0;
	UINT32 cur_freq = 0;
	INT32 step = 0;
	struct nim_dvbs_isid *p_isid = NULL;
	struct nim_dvbs_data_cap *p_data_cap = NULL;
	UINT32 plsn_value;

//	NIM_PRINTF("Enter function: %s!\n",__FUNCTION__);

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	switch (cmd)
	{
	case NIM_DRIVER_READ_TUNER_STATUS:
		return nim_c3505_get_tuner_lock(dev, (UINT8 *) param);
	case NIM_DRIVER_READ_QPSK_STATUS:
		return nim_c3505_get_lock(dev, (UINT8 *) param);
	case NIM_DRIVER_READ_FEC_STATUS:
		break;
	case NIM_DRIVER_READ_QPSK_BER:
		return nim_c3505_get_ber(dev, (UINT32 *) param);
	case NIM_DRIVER_READ_VIT_BER:
		break;
	case NIM_DRIVER_READ_RSUB:
		*(UINT32 *)param = priv->channel_info->per;
		break;
	case NIM_DRIVER_STOP_ATUOSCAN:
		priv->ul_status.c3505_autoscan_stop_flag = param;
		NIM_PRINTF("[%s %d]priv->ul_status.c3505_autoscan_stop_flag=%d\n", __FUNCTION__, __LINE__, priv->ul_status.c3505_autoscan_stop_flag);
		break;
	case NIM_DRIVER_GET_CR_NUM:
		cr_num = (INT32) nim_c3505_get_cr_num(dev);
		return cr_num;
	case NIM_DRIVER_GET_CUR_FREQ:
		switch (param)
		{
		case NIM_FREQ_RETURN_SET:
			return priv->ul_status.m_setting_freq;
		case NIM_FREQ_RETURN_REAL:
		default:
			nim_c3505_get_cur_freq(dev, &cur_freq);
			return cur_freq;
		}
	case NIM_DRIVER_FFT_PARA:
		nim_c3505_fft_set_para(dev);
		break;
	case NIM_DRIVER_FFT:
		return nim_c3505_fft(dev, param);
	case NIM_FFT_JUMP_STEP:
		step = 0;
		nim_c3505_get_tune_freq(dev, &step);
		return step;
	case NIM_DRIVER_SET_RESET_CALLBACK:
		priv->ul_status.m_pfn_reset_c3505 = (pfn_nim_reset_callback) param;
		break;
	case NIM_DRIVER_RESET_PRE_CHCHG:
		if (priv->ul_status.m_pfn_reset_c3505)
		{
			priv->ul_status.m_pfn_reset_c3505((priv->tuner_index + 1) << 16);
		}
		break;
	case NIM_DRIVER_ENABLE_DVBS2_HBCD:
//		priv->ul_status.m_enable_dvbs2_hbcd_mode = param;
//		nim_C3505_set_hbcd_timeout(dev, NIM_OPTR_IOCTL);
		break;
	case NIM_DRIVER_STOP_CHANSCAN:
		priv->ul_status.c3505_chanscan_stop_flag = param;
		NIM_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d\n", __FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag);
		break;
	case NIM_DRIVER_SET_BLSCAN_MODE:
		if(0 == param)
		   priv->blscan_mode = NIM_SCAN_FAST;
		else if(1 == param)
		   priv->blscan_mode = NIM_SCAN_SLOW;
		break;

    case NIM_DRIVER_SET_POLAR:
        return nim_c3505_set_polar(dev, (UINT8)param);
        break;
    case NIM_DRIVER_SET_12V:
        return nim_c3505_set_12v(dev, (UINT8)param);
        break;
    case NIM_DRIVER_GET_SYM:
        return nim_c3505_get_symbol_rate(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_GET_BER:
        return nim_c3505_get_ber(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_GET_AGC:
        return nim_c3505_get_agc(dev, (UINT8 *)param);
        break;
	case NIM_TURNER_SET_STANDBY:
		if (priv->nim_tuner_control != NULL)
		{
			priv->nim_tuner_control(priv->tuner_index, NIM_TUNER_SET_STANDBY_CMD, 0);	
		}
		break;
	case NIM_LNB_GET_OCP:
		if (priv->ext_lnb_control != NULL)
			return priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_GET_OCP, 0);
		break;
	case NIM_LNB_SET_POWER_ONOFF:
		if (priv->ext_lnb_control != NULL)
			return priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_POWER_ONOFF, param);
		break;		
	case NIM_ACM_MODE_GET_TP_INFO:
		nim_c3505_get_tp_info(dev);	
		break;

	case NIM_ACM_MODE_SET_ISID:
		p_isid = (struct nim_dvbs_isid *)param;
		nim_c3505_set_isid(dev, p_isid);
		break;
		
	case NIM_ACM_MODE_GET_ISID:		
		//NIM_PRINTF("[%s %d]NIM_ACM_MODE_GET_ISID\n", __FUNCTION__, __LINE__);
		if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
		{
			NIM_PRINTF("[%s %d]don't allow get isid!\n",__FUNCTION__, __LINE__);
			priv->isid->get_finish_flag = 0;			
		}
		comm_memcpy((struct nim_dvbs_isid *)param, priv->isid, sizeof(struct nim_dvbs_isid));
		break;
		
	case NIM_ACM_MODE_GET_BBHEADER_INFO:		
		comm_memcpy((struct nim_dvbs_bb_header_info *)param, priv->bb_header_info, sizeof(struct nim_dvbs_bb_header_info));
		break;	
		
#if (NIM_OPTR_ACM == ACM_CCM_FLAG)	
	 #ifdef ACM_RECORD	
	 // For record stream, in ts mode we use typical way and in gs mode we use TSO_BB way to capture 	
	 case NIM_TS_GS_AUTO_CAP:
       
        // set ISID manually 2015.12.10 gloria.zhou   
        /*
        libc_printf("====== Set Target ISID Manually ====== \n");
		ptr_isid = (priv->isid);
		ptr_isid->isid_write = 3;
		ptr_isid->debug_set_isid = 1;
		nim_c3505_set_isid(dev);  */		         

	 	return nim_c3505_ts_gs_auto_cap_mod(dev);	
		
		break;
	 case NIM_CAP_STOP:	 	
		return nim_c3505_rec_stop(dev);
		break;			
     #endif
#endif		

	case NIM_DRIVER_GET_MER:
		return nim_c3505_get_mer(dev, (UINT32 *)param);
		break;

    case NIM_DRIVER_CAP_DATA:
        p_data_cap = (struct nim_dvbs_data_cap *)param;
        return nim_c3505_adc2mem_entity(dev, (UINT8 *)p_data_cap->dram_base_addr, p_data_cap->cap_len, p_data_cap->cap_src);
        break; 

    case NIM_DRIVER_SET_PLSN:
		plsn_value = *(UINT32 *)param;
        return nim_c3505_set_plsn(dev);
        break;
		
	case NIM_DRIVER_GET_PLSN:
		*(UINT32 *)param = priv->plsn.plsn_report;
		NIM_PRINTF("[%s %d]*(UINT32 *)param=%d, priv->plsn.plsn_report=%d\n", __FUNCTION__, __LINE__, *(UINT32 *)param, priv->plsn.plsn_report);
		break;
	
	case NIM_DRIVER_PLSN_GOLD_TO_ROOT:
		plsn_value = nim_c3505_plsn_gold_to_root(*(UINT32 *)param);

		*(UINT32 *)param = plsn_value;
		break;
		
	case NIM_DRIVER_PLSN_ROOT_TO_GOLD:
		plsn_value = nim_c3505_plsn_root_to_gold(*(UINT32 *)param);		
		*(UINT32 *)param = plsn_value;
		break;

	case NIM_DRIVER_GENERATE_TABLE:
		 priv->plsn.generate_table_happened_flag = 1;
		 nim_c3505_start_generate(dev, (struct ali_plsn_address *)param);
		 PLSN_PRINTF("[%s %d]generate_table_happened_flag=%d\n", __FUNCTION__, __LINE__,  priv->plsn.generate_table_happened_flag);	
		break;
		
	case NIM_DRIVER_RELEASE_TABLE:
		if(priv->plsn.generate_table_happened_flag == 1)
		{
			nim_c3505_release_table(dev);
			priv->plsn.generate_table_happened_flag = 0;
			NIM_PRINTF("[%s %d]generate_table_happened_flag=%d\n", __FUNCTION__, __LINE__,  priv->plsn.generate_table_happened_flag);
		}	
		break;

	case NIM_DRIVER_GET_PLSN_FINISH_FLAG:
		*(UINT8 *)param = priv->plsn.plsn_finish_flag;
		break;
		
	default:
		break;
	}
	return SUCCESS;
}


/*****************************************************************************
*INT32 nim_c3505_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
* 	device input/output operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 cmd
*  Parameter3: void *param_list
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
{
    nim_get_fft_result_t *fft_para;
    nim_diseqc_operate_para_t *diseqc_para;
	struct NIM_AUTO_SCAN *p_get_spectrum = NULL;
	struct NIM_AUTO_SCAN*p_get_valid_freq = NULL;
	struct nim_get_symbol *p_sym = NULL;
	struct nim_c3505_private *priv = NULL;
	INT32 result=SUCCESS;
	UINT8 *fast_lock_detect;	
	//UINT32 time_out;
	
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
        
	priv = (struct nim_c3505_private *) dev->priv;

	NIM_PRINTF("Enter function: %s!\n",__FUNCTION__);

	switch (cmd)
	{
#if 1
	case NIM_DRIVER_AUTO_SCAN:
        //demod_mutex_flag = 1;
		priv->chanscan_autoscan_flag = 1;//Enter autoscan function
		priv->autoscan_control = NORMAL_MODE;
		osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
		result = nim_c3505_autoscan(dev, (struct NIM_AUTO_SCAN *) (param_list));
		osal_mutex_unlock(priv->scan_mutex);
		priv->chanscan_autoscan_flag = 0;//leave autoscan funtion
        //demod_mutex_flag = 0;
        return result; 
#else
    case NIM_DRIVER_AUTO_SCAN:
        priv->autoscan_control = NORMAL_MODE;
        osal_mutex_lock(priv->c3505_mutex, OSAL_WAIT_FOREVER_TIME);
        demod_mutex_flag = 1;
        result = nim_c3505_autoscan(dev, (struct NIM_AUTO_SCAN *) (param_list));
        demod_mutex_flag = 0;
        osal_mutex_unlock(priv->c3505_mutex);
        
        return result;
#endif

#if 1
	case NIM_DRIVER_CHANNEL_CHANGE:
        //demod_mutex_flag = 1;
		priv->chanscan_autoscan_flag = 1;//Enter channelchange function
		NIM_PRINTF(">>>[JIE] NIM_DRIVER_CHANNEL_CHANGE\n");
		osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
		result = nim_c3505_channel_change(dev, (struct NIM_CHANNEL_CHANGE *) (param_list));		
		osal_mutex_unlock(priv->scan_mutex);
        #ifndef CHANNEL_CHANGE_ASYNC
            priv->chanscan_autoscan_flag = 0;//Leave channelchange function
        #endif
		if (SUCCESS != result)
    	{
        	priv->chanscan_autoscan_flag = 0;//Leave channelchange function
        }
        return result;
#else
case NIM_DRIVER_CHANNEL_CHANGE:
    priv->ul_status.c3505_chanscan_stop_flag = 1;
    NIM_PRINTF(">>>[JIE] NIM_DRIVER_CHANNEL_CHANGE\n");
    osal_mutex_lock(priv->c3505_mutex, OSAL_WAIT_FOREVER_TIME);
    demod_mutex_flag = 1;
    result = nim_c3505_channel_change(dev, (struct NIM_CHANNEL_CHANGE *) (param_list));     
    #ifndef CHANNEL_CHANGE_ASYNC
        demod_mutex_flag = 0;
        osal_mutex_unlock(priv->c3505_mutex);
    #endif

    if (SUCCESS != result)
    {
        demod_mutex_flag = 0;
        osal_mutex_unlock(priv->c3505_mutex);   
    }
	//osal_mutex_unlock(priv->c3505_mutex); 
    return result;
#endif
        
	case NIM_DRIVER_CHANNEL_SEARCH:
		/* Do Channel Search */
		break;
	case NIM_DRIVER_GET_ID:
		*((UINT32 *) param_list) = priv->ul_status.m_c3505_type;
		break;
    case NIM_DRIVER_GET_FFT_RESULT:
        fft_para = (nim_get_fft_result_t *)(param_list);
        return nim_c3505_get_fft_result(dev, fft_para->freq, fft_para->start_addr);
        break;
    case NIM_DRIVER_DISEQC_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_c3505_diseqc_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt);
        break;
    case NIM_DRIVER_DISEQC2X_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_c3505_diseqc2x_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt, \
            diseqc_para->rt_value, diseqc_para->rt_cnt);
        break;

	//20160811
	case NIM_DRIVER_GET_SPECTRUM:		
		priv->autoscan_control = GET_SPECTRUM_ONLY;
		p_get_spectrum = (struct NIM_AUTO_SCAN *)(param_list);
		result = nim_c3505_autoscan(dev, p_get_spectrum);
		p_get_spectrum->spectrum.spectrum_length = chlspec_num;		
        p_get_spectrum->spectrum.p_channel_spectrum = channel_spectrum;		
		return result;

	case NIM_DRIVER_GET_VALID_FREQ:
		priv->autoscan_control = GET_TP_ONLY;
		p_get_valid_freq = (struct NIM_AUTO_SCAN *)(param_list);
		result = nim_c3505_autoscan(dev, p_get_valid_freq);
		p_get_valid_freq->valid_freq_sym.real_freq_sym = c3505_real_tp_number;
		p_get_valid_freq->valid_freq_sym.p_frequency = c3505_real_freq;
		p_get_valid_freq->valid_freq_sym.p_symbol_rate = c3505_real_sym_rate;
		return result;
		
	case NIM_DRIVER_GET_SYMBOL:
		p_sym = (struct nim_get_symbol *)param_list;
		if ((p_sym->p_cr_out_i != NULL) && (p_sym->p_cr_out_q != NULL))
		{
			nim_c3505_get_symbol(dev, p_sym);
		}
		break;

	case NIM_DRIVER_GET_AGC_DBM:
		nim_c3505_get_agc_dbm(dev, (INT8 *)param_list);
		break;
	//20160811


	case NIM_DRIVER_GET_RF_LEVEL:
		nim_c3505_get_agc_dbm(dev, (INT8 *)param_list);  
		break;

	case NIM_DRIVER_GET_CN_VALUE:
		nim_c3505_get_snr_db(dev, (INT16 *)param_list); 
		break;
		/*add new interface GET_FAST_LNB_DETECT*/
	case NIM_DRIVER_GET_FAST_LOCK_DETECT:
	{
		fast_lock_detect = (UINT8 *)param_list;/*param_list(out)*/
		nim_c3505_get_fast_lock(dev, fast_lock_detect);
		break;
	}
	default:
		break;
	}

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_C3505_task_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_task_init(struct nim_device *dev)
{
	UINT8 nim_device[3][3] =    \
	{   \
		{'N', 'M', '0'}, {'N', 'M', '1'}, {'N', 'M', '2'}   \
	};

	UINT8 plsn_task_name[3] = {'P', 'L', 'S'};
	UINT8 generate_table_task_name[3] = {'P', 'G', 'T'};
	T_CTSK nim_task_praram;
	
	struct nim_c3505_private *priv = NULL;

	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;

	if(NULL == priv)
    {
        ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }

	if (nim_task_num > 1)
	{
		NIM_PRINTF("[%s %d]nim_task_num=%d, task has inited!\n", __FUNCTION__, __LINE__, nim_task_num);
		return SUCCESS;
	}

	if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)
	{
		nim_task_praram.task = nim_c3505_task;
		nim_task_praram.name[0] = nim_device[nim_task_num][0];
		nim_task_praram.name[1] = nim_device[nim_task_num][1];
		nim_task_praram.name[2] = nim_device[nim_task_num][2];
		nim_task_praram.stksz = 0xc00;
		nim_task_praram.itskpri = OSAL_PRI_NORMAL;
		nim_task_praram.quantum = 10;
		nim_task_praram.para1 = (UINT32) dev ;
		nim_task_praram.para2 = 0 ;//Reserved for future use.
		priv->tsk_status.m_task_id = osal_task_create(&nim_task_praram);
		if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)
		{
			ERR_PRINTF("Task create error\n");
			return ERR_FAILURE;
		}
	}

	if (OSAL_INVALID_ID == priv->plsn_task_id)
	{
		nim_task_praram.task = nim_c3505_plsn_task;
		nim_task_praram.name[0] = plsn_task_name[0];
		nim_task_praram.name[1] = plsn_task_name[1];
		nim_task_praram.name[2] = plsn_task_name[2];
		nim_task_praram.stksz = 0xc00;
		nim_task_praram.itskpri = OSAL_PRI_NORMAL;
		nim_task_praram.quantum = 10;
		nim_task_praram.para1 = (UINT32) dev ;
		nim_task_praram.para2 = 0;//Reserved for future use.
		priv->plsn_task_id = osal_task_create(&nim_task_praram);
		if (OSAL_INVALID_ID == priv->plsn_task_id)
		{
			ERR_PRINTF("Task create error\n");
			return ERR_FAILURE;
		}
	}

	if (OSAL_INVALID_ID == priv->generate_table_task_id)
	{
		nim_task_praram.task = nim_c3505_generate_table_task;
		nim_task_praram.name[0] = generate_table_task_name[0];
		nim_task_praram.name[1] = generate_table_task_name[1];
		nim_task_praram.name[2] = generate_table_task_name[2];
		nim_task_praram.stksz = 0xc00;
		nim_task_praram.itskpri = OSAL_PRI_NORMAL;
		nim_task_praram.quantum = 10;
		nim_task_praram.para1 = (UINT32) dev ;
		nim_task_praram.para2 = 0;//Reserved for future use.
		priv->generate_table_task_id = osal_task_create(&nim_task_praram);
		if (OSAL_INVALID_ID == priv->generate_table_task_id)
		{
			ERR_PRINTF("Task create error\n");
			return ERR_FAILURE;
		}
	}
	
	nim_task_num++;

	return SUCCESS;
}


/*****************************************************************************
*  void nim_c3505_task(UINT32 param1, UINT32 param2)
*  Task of nim driver,  do some monitor or config
*
* Arguments:
*  Parameter1:  device struct point
*  Parameter2:  unused
*
* Return Value: INT32
*****************************************************************************/
void nim_c3505_task(UINT32 param1, UINT32 param2)
{    
	struct nim_device *dev = (struct nim_device *) param1 ;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv ;
    UINT32 last_time = 0;	
    UINT8 lock = 0;
	INT32 ret = SUCCESS;
	
#if (NIM_OPTR_CCM != ACM_CCM_FLAG)	
	UINT8 work_mode = 0;
#endif

#ifdef DEBUG_IN_TASK  
    UINT32 v_cnt_val = 0;
#endif

#ifdef CHANNEL_CHANGE_ASYNC
	UINT32 flag_ptn;
#endif
	priv->tsk_status.m_sym_rate = 0x00;
	priv->tsk_status.m_code_rate = 0x00;
	priv->tsk_status.m_map_type = 0x00;
	priv->tsk_status.m_work_mode = 0x00;
	priv->tsk_status.m_info_data = 0x00;
    
	NIM_PRINTF("Enter %s \n", __FUNCTION__);

	while (1)
	{

	
#ifdef CHANNEL_CHANGE_ASYNC
		flag_ptn = 0;
		if (NIM_FLAG_WAIT(&flag_ptn, priv->flag_id, NIM_FLAG_CHN_CHG_START, OSAL_TWF_ANDW | OSAL_TWF_CLR, 0) == OSAL_E_OK)
		{
			//osal_flag_clear(priv->flag_id, NIM_FLAG_CHN_CHG_START);
			NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHANGING);
			osal_mutex_lock(priv->c3505_mutex, OSAL_WAIT_FOREVER_TIME);
			priv->wait_chanlock_finish_flag = 0;
			NIM_PRINTF("\t\t ====== TP Signal INtensity Quality ======= \n");
			ret = nim_c3505_waiting_channel_lock(dev, priv->cur_freq, priv->cur_sym, priv->change_type, priv->isid);
			NIM_PRINTF("\t\t the result of wait channel lock ret = %d \n",ret);
			if (SUCCESS != ret)
            {   
                nim_c3505_get_lock(dev, &lock);
                if (0 == lock)
                    nim_c3505_set_work_mode(dev, 0x03); // Lock fail, then need to set work mode to auto
				if (ERR_TIME_OUT == ret)
					priv->chanscan_autoscan_flag = 0;//when time out ,it mean that channel changel finish
			}
			else
			{
				priv->chanscan_autoscan_flag = 0;//channel changel finish
			}
			priv->wait_chanlock_finish_flag = 1;
            osal_mutex_unlock(priv->c3505_mutex);
			NIM_FLAG_CLEAR(priv->flag_id, NIM_FLAG_CHN_CHANGING);
			NIM_PRINTF("\t\t Here is the task for M3505 wait channel lock \n");
            
        }
#endif
    
        // Has been locked
		if(priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR)
		{
            ;
		}
		else
		{   // Waitting lock(lock process han been break by chanscan_stop_flag ) and i2c is normal 
			if ((priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_SETTING) && (priv->t_param.t_i2c_err_flag == 0x00))
			{
				nim_c3505_get_lock(dev, &(priv->tsk_status.m_info_data));
                // Found locked then refresh the tp information
				if (priv->tsk_status.m_info_data && (priv->t_param.t_i2c_err_flag == 0x00))
				{
					nim_c3505_get_symbol_rate(dev, &(priv->tsk_status.m_sym_rate));
					nim_c3505_reg_get_code_rate(dev, &(priv->tsk_status.m_code_rate));
					nim_c3505_reg_get_work_mode(dev, &(priv->tsk_status.m_work_mode));
					nim_c3505_reg_get_map_type(dev, &(priv->tsk_status.m_map_type));
					nim_c3505_interrupt_clear(dev);
					priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
				}
			}
		}

		if(priv->search_type == NIM_OPTR_CHL_CHANGE)
        {                   
	    #ifdef ANTI_WIMAX_INTF
   	        if(force_adpt_disable == 1)
            {
                nim_c3505_cr_new_adaptive_unlock_monitor(dev);
                nim_c3505_nframe_step_tso_setting(dev,100,0x00);
		    }
		    else
		    {
		        nim_c3505_task_tso_setting(dev,100,0x00);
		    }
	    #else		 
            nim_c3505_cr_new_adaptive_unlock_monitor(dev);
            nim_c3505_nframe_step_tso_setting(dev,100,0x00);
	    #endif	
        }
    
        if(priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR)
	        comm_sleep(100);
        else
	        comm_sleep(5);
			    
    // Monitor MER and do debug_in_task and auto adaptive
    if ((osal_get_tick() - last_time) > 250)
    {
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)	
        nim_c3505_get_per(dev , &(priv->channel_info->per));
		//osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
		priv->channel_info->period_per+= priv->channel_info->per;
		//osal_mutex_unlock(priv->c3505_mutex);
#else
		nim_c3505_reg_get_work_mode(dev, &work_mode);
		if (1 == work_mode)
        	nim_c3505_get_fer(dev , &(priv->channel_info->per));
		else
		{
			nim_c3505_get_per(dev , &(priv->channel_info->per));
			//osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
			priv->channel_info->period_per+= priv->channel_info->per;
			//osal_mutex_unlock(priv->c3505_mutex);
		}
#endif
        nim_c3505_get_snr_db(dev, &(priv->channel_info->snr));
        nim_c3505_get_phase_error(dev , &(priv->channel_info->phase_err));

        nim_c3505_get_mer_task(dev);

		//when channel change or autoscan finished,the following function can be executed
        if (0 == priv->chanscan_autoscan_flag) 
        {
            priv->t_param.auto_adaptive_doing = 1;
#ifdef DEBUG_IN_TASK 
            nim_c3505_debug_intask(dev);
#endif  

#ifdef IMPLUSE_NOISE_IMPROVED
            nim_c3505_auto_adaptive(dev);    
#endif 
            nim_c3505_mon_fake_lock(dev);

            nim_c3505_lock_unlock_adaptive(dev);    // For fc searh/ppll/CR Feedback Carrier Threshold adaptive 

            //osal_mutex_unlock(priv->scan_mutex);
        }   
        priv->t_param.auto_adaptive_doing = 0;
        last_time = osal_get_tick();
    }
    
	}
    ERR_PRINTF("Run out of %s \n", __FUNCTION__);
}

extern INT32 *plsn_sq_rn_pilot_table;
void nim_c3505_generate_table_task(UINT32 param1, UINT32 param2)
{
	struct nim_device *dev = (struct nim_device *) param1 ;
#ifdef PLSN_DEBUG
	UINT32 gen_start = 0;
#endif
	while(1)
	{
		if (!g_table_finish_flag)
		{
		#ifdef PLSN_DEBUG
			gen_start = osal_get_tick();
		#endif
			nim_c3505_generate_table(dev);
			PLSN_PRINTF("generate tabel cost:%d ms\n", osal_get_tick()-gen_start);
		}
		
		comm_sleep(5);
	}
	
}
void nim_c3505_plsn_task(UINT32 param1, UINT32 param2)
{    
	struct nim_device *dev = (struct nim_device *) param1 ;
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv ;

#ifdef PLSN_DEBUG
	UINT32 start = 0;
#endif

	NIM_PRINTF("Enter %s \n", __FUNCTION__);
	while (1)		
	{
		//priv->plsn.super_scan is set by upper level
		//priv->plsn.start_search is set in nim_c3505_channel_change and nim_c3505_soft_search
		//priv->ul_status.c3505_chanscan_stop_flag is cleared in nim_c3505_waiting_channel_lock
		//priv->plsn.auto_scan_start is set in nim_c3505_soft_search
		if ((1 == priv->plsn.super_scan) 
			&& (1 == priv->plsn.start_search) 
			&& ((0 == priv->ul_status.c3505_chanscan_stop_flag)
			     || (1 == priv->plsn.auto_scan_start)))// do super scan for plsn
		{
			PLSN_PRINTF("start to search plsn!\n");
		#ifdef PLSN_DEBUG
			start = osal_get_tick();
		#endif
			priv->plsn.search_plsn_force_stop = 0;
			PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
			priv->plsn.search_plsn_stop = 0;
			PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_stop);
			priv->plsn.plsn_find = 0;
			priv->plsn.plsn_num = 0;
			priv->plsn.plsn_try = 0;
			priv->plsn.plsn_try_val = 0;
			priv->plsn.auto_scan_start = 0;
			PLSN_PRINTF("priv->plsn.auto_scan_start=%d\n", priv->plsn.auto_scan_start);
			memset(priv->plsn.plsn_val, 0, sizeof(priv->plsn.plsn_val));

			nim_c3505_search_plsn_top(dev);
			
			priv->plsn.start_search = 0;

			PLSN_PRINTF("search end, cost %d ms\n", osal_get_tick() - start);
		}
		comm_sleep(5);
	}
}
/*****************************************************************************
* INT32 nim_C3505_open(struct nim_device *dev)
* Description: NIM device open operate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_open(struct nim_device *dev)
{
	INT32 ret = ERR_FAILED;

	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	NIM_PRINTF("Enter fuction %s \n",__FUNCTION__);
	
	ret = nim_c3505_hw_check(dev);
	if (SUCCESS != ret)
		return ret;

    nim_c3505_reg_get_chip_type(dev);

    if ((NIM_OPTR_ACM == ACM_CCM_FLAG) && (CHIP_ID_3503C == priv->ul_status.m_c3505_type))
    {
        NIM_PRINTF("Error, C3503c can not support ACM work mode, Exit!\n");
        return ERR_FAILED;
    }

	ret = nim_c3505_hw_init(dev);
	if (SUCCESS != ret)
		return ret;

	nim_c3505_after_reset_set_param(dev);
	nim_c3505_task_init(dev);

#ifdef CHANNEL_CHANGE_ASYNC
	if (priv->flag_id == OSAL_INVALID_ID)
		priv->flag_id = NIM_FLAG_CREATE(0);
#endif	

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3505_close(struct nim_device *dev)
* Description: C3505 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3505_close(struct nim_device *dev)
{	
	UINT8 data = 0;

	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);

	// close ADC
	data = 0x07;
	nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);
			
    //data = 0x3f;
	data = 0x1f;//S2_CLK_EN bit set to 0 for C3505
	nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);

	NIM_MUTEX_DELETE(priv->ul_status.nim_c3505_sema);

#ifdef CHANNEL_CHANGE_ASYNC
	NIM_FLAG_DEL(priv->flag_id);
#endif

	if (priv->tsk_status.m_task_id != OSAL_INVALID_ID)
	{
		if(OSAL_E_OK != osal_task_delete(priv->tsk_status.m_task_id))
		{
			NIM_PRINTF("[%s %d]priv->tsk_status.m_task_id delete error!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			priv->tsk_status.m_task_id = OSAL_INVALID_ID;
		}
	}

	if (priv->plsn_task_id != OSAL_INVALID_ID)
	{
		if(OSAL_E_OK != osal_task_delete(priv->plsn_task_id))
		{
			NIM_PRINTF("[%s %d]priv->plsn_task_id delete error!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			priv->plsn_task_id = OSAL_INVALID_ID;
		}
	}

	if (priv->generate_table_task_id != OSAL_INVALID_ID)
	{
		if(OSAL_E_OK != osal_task_delete(priv->generate_table_task_id))
		{
			NIM_PRINTF("[%s %d]priv->generate_table_task_id delete error!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			priv->generate_table_task_id = OSAL_INVALID_ID;
		}
	}

	if (nim_task_num > 0)
	{
		nim_task_num--;
	}
	
	return SUCCESS;
}

//==============================================================================
#ifdef PRINT_TO_USB

static UINT8 log_file_name[100];
static UINT8 log_buf[1000];
FILE *data_file = NULL;
BOOL log_usb_on = FALSE;
ID usb_log_id = OSAL_INVALID_ID;

void nim_usb_log_enable()
{
    
    UINT16 len=0;

    if(usb_log_id == OSAL_INVALID_ID)
            usb_log_id = osal_mutex_create();

    if (OSAL_INVALID_ID == usb_log_id)
    {
        return;
    }
    
    sprintf(log_file_name, "/mnt/uda1/nim_log.txt");
    data_file = fopen(log_file_name, "wb+");
    sprintf(log_buf, "log file open success\n");
    len = strlen(log_buf);
   
    fwrite(log_buf, len, 1, data_file);
    fflush(data_file);
    log_usb_on = TRUE;
    //fclose(data_file);
   }

void usb_log_lock()
{
     osal_mutex_lock(usb_log_id, OSAL_WAIT_FOREVER_TIME);
}


void usb_log_unlock()
{
     osal_mutex_unlock(usb_log_id);
}


void nim_usb_log_disable()
{
    if(data_file !=NULL)
    {
         //fflush(data_file);
        fclose(data_file);
        data_file = NULL;
    }
}

void nim_usb_log(char* msg)
{
    UINT16 len=0;
    if(data_file !=NULL)
    { 
        //memset(log_buf,0, sizeof(log_buf));
        //sprintf(log_buf, format);
        len = strlen(msg);
        fwrite(msg, len, 1, data_file);
         fflush(data_file);
    }
}


static UINT8 tmp_buf[1000];

void usb_log_example()
{
    //need to make sure USB insert already.
    
    nim_usb_log_enable();

    sprintf(tmp_buf, "log message that you want show\n");
    nim_usb_log(tmp_buf);
    
    nim_usb_log_disable();
    
}

#endif

//==============================================================================

