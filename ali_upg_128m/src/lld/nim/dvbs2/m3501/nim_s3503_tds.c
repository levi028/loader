/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_tds.c
*
*    Description:  s3503 nim driver for tds api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3503.h"

#define NIM_DEV_NUM_2 		2

//begin:added by robin.gan on 2015.4.15
#define HLD_MAX_NAME_SIZE	16
//static UINT32 ascan_stop_flag=0x00;
//end

/* Name for the tuner, the last character must be Number for index */
static char nim_s3501_name[3][HLD_MAX_NAME_SIZE] =
{
    "NIM_S3503_0", "NIM_S3503_1", "NIM_S3503_2"
};

static unsigned char nim_dev_num = 0;
static UINT8 nim_task_num = 0x00;

static INT32 nim_s3503_open(struct nim_device *dev);
static INT32 nim_s3503_close(struct nim_device *dev);
static INT32 nim_s3503_task_init(struct nim_device *dev);
static INT32 nim_s3503_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list);

/*****************************************************************************
* INT32 nim_s3503_attach (struct QPSK_TUNER_CONFIG_API * ptrqpsk_tuner)
* Description: S3501 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_attach(struct QPSK_TUNER_CONFIG_API *ptrqpsk_tuner)
{
    struct nim_device *dev = NULL;
    struct nim_s3501_private *priv_mem = NULL;

    if (ptrqpsk_tuner == NULL)
    {
        NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }
	
    if (nim_dev_num > NIM_DEV_NUM_2)
    {
        NIM_PRINTF("Can not support three or more S3501 !/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *) dev_alloc((INT8 *)nim_s3501_name[nim_dev_num],
                                           HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    priv_mem = (struct nim_s3501_private *) comm_malloc(sizeof(struct nim_s3501_private));
    if (priv_mem == NULL)
    {
        dev_free(dev);
        NIM_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    comm_memset((int *)priv_mem, 0, sizeof(struct nim_s3501_private));
    dev->priv = (void *) priv_mem;
    //diseqc state init
    dev->diseqc_info.diseqc_type = 0;
    dev->diseqc_info.diseqc_port = 0;
    dev->diseqc_info.diseqc_k22 = 0;

    if ((ptrqpsk_tuner->config_data.qpsk_config & M3501_POLAR_REVERT) == M3501_POLAR_REVERT) //bit4: polarity revert.
    {
        dev->diseqc_info.diseqc_polar = LNB_POL_V;
    }
    else //default usage, not revert.
    {
        dev->diseqc_info.diseqc_polar = LNB_POL_H;
    }

    dev->diseqc_typex = 0;
    dev->diseqc_portx = 0;

    /* Function point init */
    dev->base_addr = ptrqpsk_tuner->ext_dm_config.i2c_base_addr;
    dev->init = nim_s3503_attach;
    dev->open = nim_s3503_open;
    dev->stop = nim_s3503_close;
    dev->do_ioctl = nim_s3503_ioctl;
    dev->do_ioctl_ext = nim_s3503_ioctl_ext;
    dev->get_lock = nim_s3503_get_lock;
    dev->get_freq = nim_s3503_get_curfreq;//nim_s3503_get_freq;
    dev->get_fec = nim_s3503_get_code_rate;
    dev->get_snr = nim_s3503_get_snr;
    dev->set_polar = nim_s3503_set_polar;
    dev->set_12v = nim_s3503_set_12v;
    dev->channel_search = (void *)nim_s3503_channel_search;
    dev->di_seq_c_operate = nim_s3503_di_seq_c_operate;
    dev->di_seq_c2x_operate = nim_s3503_di_seq_c2x_operate;
    dev->get_sym = nim_s3503_get_symbol_rate;
    dev->get_agc = nim_s3503_get_agc;
    dev->get_ber = nim_s3503_get_ber;
    dev->get_fft_result = nim_s3503_get_fft_result;
    dev->get_ver_infor = NULL;//nim_s3503_get_ver_infor;

    /* tuner configuration function */
    priv_mem->nim_tuner_init = ptrqpsk_tuner->nim_tuner_init;
    priv_mem->nim_tuner_control = ptrqpsk_tuner->nim_tuner_control;
    priv_mem->nim_tuner_status = ptrqpsk_tuner->nim_tuner_status;

	
	//add by dennis on 20140906 for c3031b Tuner
	priv_mem->nim_tuner_command = ptrqpsk_tuner->nim_tuner_command;
	priv_mem->nim_tuner_gain = ptrqpsk_tuner->nim_tuner_gain;//add by dennis on 20140909 for Ali c3031b tuner 
  	//NIM_PRINTF("[%s]line=%d,tuner_index=%x!\n",__FUNCTION__,__LINE__,priv_mem->tuner_index);
	
    priv_mem->i2c_type_id = ptrqpsk_tuner->tuner_config.i2c_type_id;
    priv_mem->tuner_config_data.qpsk_config = ptrqpsk_tuner->config_data.qpsk_config;
    priv_mem->ext_dm_config.i2c_type_id = ptrqpsk_tuner->ext_dm_config.i2c_type_id;
    priv_mem->ext_dm_config.i2c_base_addr = ptrqpsk_tuner->ext_dm_config.i2c_base_addr;

    priv_mem->ul_status.m_enable_dvbs2_hbcd_mode = 0;
    priv_mem->ul_status.m_dvbs2_hbcd_enable_value = 0x7f;
    priv_mem->ul_status.nim_s3501_sema = OSAL_INVALID_ID;
    priv_mem->ul_status.s3501_autoscan_stop_flag = 0;
    priv_mem->ul_status.s3501_chanscan_stop_flag = 0;
    priv_mem->ul_status.old_ber = 0;
    priv_mem->ul_status.old_per = 0;
    priv_mem->ul_status.m_hw_timeout_thr = 0;
    priv_mem->ul_status.old_ldpc_ite_num = 0;
    priv_mem->ul_status.c_rs = 0;
    priv_mem->ul_status.phase_err_check_status = 0;
    priv_mem->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
    priv_mem->ul_status.m_s3501_type = 0x00;
    priv_mem->ul_status.m_setting_freq = 123;
    priv_mem->ul_status.m_err_cnts = 0x00;
    priv_mem->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
    priv_mem->tsk_status.m_task_id = 0x00;
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

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        comm_free((int *)priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }
    nim_dev_num++;
    priv_mem->ul_status.nim_s3501_sema = NIM_MUTEX_CREATE(1);

	priv_mem->m3501_mutex= osal_mutex_create();
	if(priv_mem->m3501_mutex == OSAL_INVALID_ID)
	{
		NIM_PRINTF("Error: Register nim device error!\n");
		return ERR_FAILUE;
	}
	
    // Initial the QPSK Tuner
    if (priv_mem->nim_tuner_init != NULL)
    {
        NIM_PRINTF(" Initial the Tuner \n");
        if (((struct nim_s3501_private *) dev->priv)->nim_tuner_init(&priv_mem->tuner_index, \
                                                 &(ptrqpsk_tuner->tuner_config)) != SUCCESS)
        {
            NIM_PRINTF("Error: Init Tuner Failure!\n");


            return ERR_NO_DEV;
        }
    }

    nim_s3503_set_ext_lnb(dev, ptrqpsk_tuner);

    nim_s3503_get_type(dev);

    ptrqpsk_tuner->device_type = priv_mem->ul_status.m_s3501_type;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_open(struct nim_device *dev)
* Description: S3501 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_open(struct nim_device *dev)
{
    INT32 ret= 0;
    if(dev == NULL)
        return RET_FAILURE;
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF("    Enter fuction nim_s3503_open\n");
    nim_s3503_set_acq_workmode(dev, NIM_OPTR_HW_OPEN);

    ret = nim_s3503_hw_check(dev);
    if (ret != SUCCESS)
    {
        return ret;
    }
    ret = nim_s3503_hw_init(dev);

    nim_s3503_after_reset_set_param(dev);

    nim_s3503_set_hbcd_timeout(dev, NIM_OPTR_HW_OPEN);

    nim_s3503_task_init(dev);

#ifdef CHANNEL_CHANGE_ASYNC
    //nim_flag_create(priv);
	if (priv->flag_id == OSAL_INVALID_ID)
	{
		priv->flag_id = NIM_FLAG_CREATE(0);
	}
#endif
    NIM_PRINTF("    Leave fuction nim_s3503_open\n");
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_close(struct nim_device *dev)
* Description: S3501 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_close(struct nim_device *dev)
{
    //UINT8  data,ver_data;
    if(dev == NULL)
        return RET_FAILURE;

    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);

    nim_s3503_set_acq_workmode(dev, NIM_OPTR_HW_CLOSE);

    NIM_MUTEX_DELETE(priv->ul_status.nim_s3501_sema);

#ifdef CHANNEL_CHANGE_ASYNC
    //nim_flag_del(priv);
	NIM_FLAG_DEL(priv->flag_id);
#endif

    if(OSAL_E_OK != osal_task_delete(priv->tsk_status.m_task_id))                 //Seiya add
    {
        AUTOSCAN_PRINTF("S3503 Task Delete Error Happenened!\n");
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
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
INT32 nim_s3503_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    INT32 crnum = 0;
    UINT32 curfreq = 0;
    INT32 step = 0;
    //UINT8 data;
    if(dev == NULL)
        return RET_FAILURE;

    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    //NIM_PRINTF("  Enter fuction nim_s3503_ioctl\n");
    switch (cmd)
    {
    case NIM_DRIVER_READ_TUNER_STATUS:
        return nim_s3503_tuner_lock(dev, (UINT8 *) param);
    case NIM_DRIVER_READ_QPSK_STATUS:
        return nim_s3503_get_lock(dev, (UINT8 *) param);
    case NIM_DRIVER_READ_FEC_STATUS:
        break;
    case NIM_DRIVER_READ_QPSK_BER:
        return nim_s3503_get_ber(dev, (UINT32 *) param);
    case NIM_DRIVER_READ_VIT_BER:
        break;
    case NIM_DRIVER_READ_RSUB:
        return nim_s3503_get_per(dev, (UINT32 *) param);
    case NIM_DRIVER_STOP_ATUOSCAN:
        priv->ul_status.s3501_autoscan_stop_flag = param;
        break;
    case NIM_DRIVER_GET_CR_NUM:
        crnum = (INT32) nim_s3503_get_crnum(dev);
        return crnum;
    case NIM_DRIVER_GET_CUR_FREQ:
        switch (param)
        {
        case NIM_FREQ_RETURN_SET:
            return priv->ul_status.m_setting_freq;
        case NIM_FREQ_RETURN_REAL:
        default:
            nim_s3503_get_curfreq(dev, &curfreq);
            return curfreq;
        }
    break;
    case NIM_DRIVER_FFT_PARA:
        nim_s3503_fft_set_para(dev);
        break;
    case NIM_DRIVER_FFT:
        return nim_s3503_fft(dev, param);
    case NIM_FFT_JUMP_STEP:
        step = 0;
        nim_s3503_get_tune_freq(dev, &step);
        return step;
    case NIM_DRIVER_SET_RESET_CALLBACK:
        priv->ul_status.m_pfn_reset_s3501 = (pfn_nim_reset_callback) param;
        break;
    case NIM_DRIVER_RESET_PRE_CHCHG:
        if (priv->ul_status.m_pfn_reset_s3501)
        {
            priv->ul_status.m_pfn_reset_s3501((priv->tuner_index+ 1) << 16);
        }
        break;
    case NIM_DRIVER_ENABLE_DVBS2_HBCD:
        priv->ul_status.m_enable_dvbs2_hbcd_mode = param;
        nim_s3503_set_hbcd_timeout(dev, NIM_OPTR_IOCTL);
        break;
    case NIM_DRIVER_STOP_CHANSCAN:
        priv->ul_status.s3501_chanscan_stop_flag = param;
        break;
    case NIM_DRIVER_SET_BLSCAN_MODE:
        if(0 == param)
        {
        priv->blscan_mode = NIM_SCAN_FAST;
         }
        else if(1 == param)
        {
        priv->blscan_mode = NIM_SCAN_SLOW;
        }
        break;

    case NIM_DRIVER_SET_POLAR:
        return nim_s3503_set_polar(dev, (UINT8)param);

    case NIM_DRIVER_SET_12V:
        return nim_s3503_set_12v(dev, (UINT8)param);

    case NIM_DRIVER_GET_SYM:
        return nim_s3503_get_symbol_rate(dev, (UINT32 *)param);

    case NIM_DRIVER_GET_BER:
        return nim_s3503_get_ber(dev, (UINT32 *)param);

    case NIM_DRIVER_GET_AGC:
        return nim_s3503_get_agc(dev, (UINT8 *)param);

    case NIM_TURNER_SET_STANDBY:
        if (priv->nim_tuner_control != NULL)
        {
            priv->nim_tuner_control(priv->tuner_index, NIM_TUNER_SET_STANDBY_CMD, 0);
        }
        break;

	case NIM_DRIVER_GET_MER:	 	
		return nim_s3503_get_mer(dev, (UINT32 *) param);	 	
		break;
		
    default:
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
*INT32 nim_s3503_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
{
    nim_get_fft_result_t *fft_para = NULL;
    nim_diseqc_operate_para_t *diseqc_para = NULL;
    if((dev == NULL) || (param_list == NULL))
        return RET_FAILURE;
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;
    INT32 result = SUCCESS;

    //UINT32 v_scan_loop = 0x01;

    NIM_PRINTF("    Enter fuction nim_s3503_event_control\n");

    switch (cmd)
    {
    case NIM_DRIVER_AUTO_SCAN:
        /* Do AutoScan Procedure */
#ifdef NIM_S3501_ASCAN_TOOLS
        //v_scan_loop= vs_s3501_ascan.va_ascan_g8_loop_cnt;// add code to set autoscan loop count
        //vs_s3501_ascan.va_ascan_g8_loop_cur_idx=0x00;
#endif

#ifdef NIM_S3501_ASCAN_TOOLS
        ascan_stop_flag = 0x00;
#endif

        NIM_PRINTF(">>>[JIE] NIM_DRIVER_AUTO_SCAN\n");
        result = nim_s3503_autoscan(dev, (NIM_AUTO_SCAN_T *) (param_list));
		
#ifdef NIM_S3501_ASCAN_TOOLS
        if(priv->ul_status.s3501_autoscan_stop_flag)
            ascan_stop_flag = 0x01;
        else
            nim_s3501_ascan_tps_lock_check();
#endif

        return result;

    case NIM_DRIVER_CHANNEL_CHANGE:
        /* Do Channel Change */
        NIM_PRINTF(">>>[JIE] NIM_DRIVER_CHANNEL_CHANGE\n");
        {
            struct NIM_CHANNEL_CHANGE *nim_param = (struct NIM_CHANNEL_CHANGE *) (param_list);

            return nim_s3503_channel_change(dev, nim_param->freq, nim_param->sym, nim_param->fec);
        }
    case NIM_DRIVER_CHANNEL_SEARCH:
        /* Do Channel Search */
        break;
    case NIM_DRIVER_GET_ID:
        *((UINT32 *) param_list) = priv->ul_status.m_s3501_type;
        break;
    case NIM_DRIVER_GET_FFT_RESULT:
        fft_para = (nim_get_fft_result_t *)(param_list);
        return nim_s3503_get_fft_result(dev, fft_para->freq, fft_para->start_addr);

    case NIM_DRIVER_DISEQC_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_s3503_di_seq_c_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt);

    case NIM_DRIVER_DISEQC2X_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_s3503_di_seq_c2x_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt, \
                                          diseqc_para->rt_value, diseqc_para->rt_cnt);


    default:
        break;
    }

    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_task_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_task_init(struct nim_device *dev)
{
    UINT8 nim_device[3][3] =
    {
       {'N', 'M', '0'},
       {'N', 'M', '1'},
       {'N', 'M', '2'}
    };
    T_CTSK nim_task_praram;
    if(dev == NULL)
        return RET_FAILURE;
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    if (nim_task_num > 1)
    {
        return SUCCESS;
    }

    MEMSET(&nim_task_praram, 0 , sizeof(nim_task_praram));
    nim_task_praram.task = nim_s3503_task;//dmx_m3327_record_task ;
    nim_task_praram.name[0] = nim_device[nim_task_num][0];
    nim_task_praram.name[1] = nim_device[nim_task_num][1];
    nim_task_praram.name[2] = nim_device[nim_task_num][2];
    nim_task_praram.stksz = 0xc00 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 10 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;//Reserved for future use.
    priv->tsk_status.m_task_id = osal_task_create(&nim_task_praram);
    if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)
    {
        //NIM_PRINTF("Task create error\n");
        return OSAL_E_FAIL;
    }
    nim_task_num++;

    return SUCCESS;
}


