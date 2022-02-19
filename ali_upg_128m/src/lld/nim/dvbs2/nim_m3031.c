//Copyright (C)2014 Ali Corporation. All Rights Reserved.

//Histroy
//2014-04-21  Manson  Create

/*****************************************************************************
*    Copyright (C)2014 Ali Corporation. All Rights Reserved.
*
*    File:    nim_m3031.c
*
*    Description: M3031 Tuner driver
*    History:
*	Date				Athor			Version			Reason
*	============       =========       ======    =================
*	1.	2014-04-21		Manson  Chen	Ver 1.0		Create file.
*	2.	2015-10-27		Paladin Ye		Ver 1.1		Correct mix gain, 
*													Add command get c3031b LNA status 
*	3.	2017-04-21		Paladin Ye		Ver 1.2		Creat file for M3031B
*****************************************************************************/




//include other files needed 
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>
#include "nim_m3031.h"
#include "nim_m3031_reg.h"


#if 0
#define NIM_PRINTF   libc_printf
#else
#define NIM_PRINTF(...)
#endif

#define COMPATIBILITY_WITH_OLD_M3031B

// For new  M3031B
#define LNA_H7_ATT    // 0 : no additional rejection, 1 : activated rejection(while freq < H7_ATT_FREQ)
#define H7_ATT_FREQ 900
#define LNA_HGAIN_EN 0  // 0 : high LNA gain (active LOW !),  1 : reduced LNA gain by 2dB
#define LNA_S11_TILT 0  // 0 : best S11, 0.15dB NF degradation, 3 : best NF, 2 ~ 3dB S11 degradation
// REG_D[0] : pdiv_corr_45 : PDIV phase corr to ATM
// REG_D[1] : pdiv_corr_135 : PDIV phase corr to ATM
// REG_D[2] : pll_div_bias : increase PLL div current by 50%
// REG_D[3] : pll_cap : decrease internal LF cap by 5pF
// REG_D[4] : backup for PDIV robustness, increase current in PDIV sub-modules
#define REG_D 0
#define VGA_REG_OFF 0   // 0 : no shift, 7 : max shift (3.5dB gain shift)
#define VGA_AGC_INV 0   // 0 : non inverted, 1 : inverted; agc inverse for compatibility, then it agc slope will same as av2018
#define EXTEND_LOW_FREQ // For 250~950Mhz, total 250~2550Mhz
//#define FOR_ES_TEST
// end

static UINT32 m3031_tuner_cnt = 0;
static UINT8 m3031_rc_freq[2] = {0};  //Retreived during init
static UINT8 chip_version[MAX_TUNER_SUPPORT_NUM] = {0};

// For new  M3031B
static UINT8 sub_version[MAX_TUNER_SUPPORT_NUM] = {0};  // 0: old m3031b, 1: new m3031b

//#ifdef FOR_ES_TEST
#if 0
static UINT8 pwr_1v2_present[MAX_TUNER_SUPPORT_NUM] = {0};  // 0 : no supply present, 1 : 1.2V is present
#endif
// end

static UINT8 remember_auto_search_mode[MAX_TUNER_SUPPORT_NUM] = {0}; //to enable decide RF_VRMS at the beginning of auto search
static UINT8 remember_reg_b[MAX_TUNER_SUPPORT_NUM] = {0}; //remember REG_B value we have written to the tuner so that we don't have to read it back
static UINT8 remember_rf_vrms[MAX_TUNER_SUPPORT_NUM] = {0}; //remember RF_VRMS value we have written to the tuner so that we don't have to read it back
static int remember_rf_signal_level[MAX_TUNER_SUPPORT_NUM] = {0}; //remember rf signal level for detecting signal level large change
static int rf_vrms_signal_thres[3] = {11,16,21}; //the first value must >9 for VRMS=3
static UINT32 pll_d = (UINT8)(M3031_XTAL_FREQ_MHZ >> 1); //match TCL round
static INT32 g_channel_freq_err = 0;   
static INT32 g_agc_gain_flag = 0; //  LNA status, bit[0] = mix_lgain , bit[1] =  rf_coarse_gain , bit[2] =  dgb_gain, used to choose the formula for dBm calculate 


//static UINT32 PLL_D_Div_Freq = (PLL_D << 16) / C3031_XTAL_FREQ_MHZ; 
static UINT32 pll_d_div_freq = ((M3031_XTAL_FREQ_MHZ >> 1) << 16) / M3031_XTAL_FREQ_MHZ; 
//Registers that are programmed again in nim_c3031_control() don't care the initial value
//------------------------------0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f-------
static UINT8 init_10[16]    = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2, 3, 1, 1}; 
//0x1c(BB_VRMS) not allow to be 0! initialized with everything enabled. If standby mode is needed, call nim_c3031_standby
//0x1e(DC_COMP_SW_OFFSET must be 1 to match chip test setting)
static UINT8 init_20[16]    = { 0, 6, 0, 0, 2, 2, 1, 2, 3, 1, 0, 0, 0, 6, 0, (1 + ((LNA_HGAIN_EN & 0x01)<<2) + ((LNA_S11_TILT & 0x03)<<3))}; 
static UINT8 init_30[4]     = { (2 + ((REG_D & 0x1f)<<5)), 0, 1, (0 + ((VGA_REG_OFF& 0x07)<<2) + ((VGA_AGC_INV & 0x01)<<5))};
static UINT8 init_50[16]    = { 0, 0, 0, 0, 0, 0,21, 7, 0, 0, 0, 0, 0, 0, 0, 0}; //RF AGC speed is initialized as fastest
static UINT8 init_60[5]     = { 0, 0, 0, 0, 11};//0x64 bit [3] must be 1 to match chip test settings
static UINT8 init_78[8]     =                         {0, 0, 0, 0, 0, 0, 4, 0}; //BB AGC speed is initialized as fastest
static UINT8 init_80[5]     = { 6,18, 6,12,32};
static UINT8 standby_10[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //disable ENs in standby mode, note: don't touch LT_EN

#ifdef COMPATIBILITY_WITH_OLD_M3031B
//lgt_fbc   lgt_atta  lgt_attb
static UINT8 lna_gain_table[22][3] = {
	{0,          126,       0}, //first data change from 4 to 0 to increase LNA gain
	{10,         56,        0},
	{16,         46,        0},
	{24,         46,        0},
	{30,         28,        0},
	{37,         20,        0},
	{47,         20,        0},
	{43,         14,        2},
	{47,         10,        2},
	{54,         10,        3},
	{51,         7,         3},
	{49,         5,         4},
	{59,         4,         4},
	{34,         3,         5},
	{39,         3,         6},
	{42,         3,         8},
	{53,         3,         8},
	{53,         3,         11},
	{51,         3,         15},
	{107,        1,         15},
	{117,        1,         15},
	{127,        1,         15},
};
#endif


static struct QPSK_TUNER_CONFIG_EXT  m3031_dev_id[MAX_TUNER_SUPPORT_NUM];
static int m3031_in_standby[MAX_TUNER_SUPPORT_NUM] = {0};
static int tuner_i2c_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;
	struct QPSK_TUNER_CONFIG_EXT * m3031_ptr = NULL;
	
	//nim_comm_delay(10);
	m3031_ptr = &m3031_dev_id[tuner_id];
	data[0] = reg_start;

	while((rd+15)<length)
	{
		MEMCPY(&data[1], &buff[rd], 15);
		i2c_result = i2c_write(m3031_ptr->i2c_type_id, m3031_ptr->c_tuner_base_addr, data, 16);
		rd+=15;
		data[0] += 15;
		if(SUCCESS != i2c_result)
			return i2c_result;
	}
	NIM_PRINTF("Tuner_I2C_write addr:0x%x\n",m3031_ptr->c_tuner_base_addr);
	MEMCPY(&data[1], &buff[rd], length-rd);
	i2c_result = i2c_write(m3031_ptr->i2c_type_id, m3031_ptr->c_tuner_base_addr, data, length-rd+1);
	
	return i2c_result;
}

static int tuner_i2c_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;
	struct QPSK_TUNER_CONFIG_EXT * m3031_ptr = NULL;

	//nim_comm_delay(10);


	m3031_ptr = &m3031_dev_id[tuner_id];	
	data[0] = reg_start;
		
	i2c_result = i2c_write_read(m3031_ptr->i2c_type_id, m3031_ptr->c_tuner_base_addr, data, 1, length-rd);
	NIM_PRINTF("Tuner_I2C_read, chip_adr=0x%x, bMemAdr=0x%x, i2c_result = %d\n", m3031_ptr->c_tuner_base_addr, reg_start, i2c_result);
	MEMCPY(&buff[0], &data[0],length);
	
	return i2c_result;
}

INT32 fast_settle_rf_agc(UINT32 tuner_id, UINT8 slow_speed) //This is NOT a API function
{
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;
	INT32 result;
	UINT8 byte;
	//UINT32 start_t,end_t; //warning: variable 'end_t' set but not used
	//m3031_ptr = &m3031_dev_id[tuner_id];
	byte = FAST_RFAGC_SPEED; //fast RF AGC with the remember_RF_VRMS applied before entering standby mode
	if ((result = tuner_i2c_write(tuner_id, ADDR_RFAGC_SPEED, &byte, 1)) != SUCCESS)
	{
		return result;
	}
	//start_t = os_get_tick_count();//warning: variable 'end_t' set but not used
	osal_task_sleep(FAST_RFAGC_WAIT_MS); //wait for RF AGC to settle at FAST_RFAGC_SPEED
	//end_t = os_get_tick_count();//warning: variable 'end_t' set but not used
	byte = slow_speed;
	if ((result = tuner_i2c_write(tuner_id, ADDR_RFAGC_SPEED, &byte, 1)) != SUCCESS) 
	{
		return result;
	}	

	return SUCCESS;
}

INT32 decide_rf_vrms(UINT32 tuner_id, int rf_signal_level) //This is NOT a API function
{
	UINT8 gain_index[2];	
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;
	INT32 result;
	//int i;
	//m3031_ptr = &m3031_dev_id[tuner_id];

	if(rf_signal_level == -1000)
	{ //the caller didn't calculate it
		if ((result = tuner_i2c_read(tuner_id, ADDR_MO_GAIN_C_INDEX, gain_index, 2)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}
		rf_signal_level = (gain_index[0] * 2) + ((gain_index[1]+2)>>2) + (remember_rf_vrms[tuner_id] * 3); //for the same signal level, higher RF_VRMS result in lower gain_index
		remember_rf_signal_level[tuner_id] = rf_signal_level;
	}
	
	//Use only RF_VRMS = 0 or 1 to maximize the linearity
	if(rf_signal_level <rf_vrms_signal_thres[2])
		remember_rf_vrms[tuner_id] = 1;
	else
		remember_rf_vrms[tuner_id] = 0;
	
	
  
	result = tuner_i2c_write(tuner_id,ADDR_RF_VRMS,&remember_rf_vrms[tuner_id], 1);
	if (result != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
    	return result;
	}
	
	return SUCCESS;
}

INT32 enter_deep_sleep(UINT32 tuner_id) //This is NOT a API function
//disable band gap reference for 1mA standby current
{
	UINT8 byte;
	INT32 result;
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr =NULL;
	//m3031_ptr = &m3031_dev_id[tuner_id];

	byte = 0x80; //set STANDBY_LDO = 1
	if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_A, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	byte = 0x1; //set PLL_D = 1
	if ((result = tuner_i2c_write(tuner_id,  ADDR_PLL_D, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	return SUCCESS;
}

INT32 leave_deep_sleep(UINT32 tuner_id) //This is NOT a API function
//enable band gap reference to leave deep sleep standby mode
{
	UINT8 byte;
	INT32 result;
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;

	//m3031_ptr = &m3031_dev_id[tuner_id];

	byte = pll_d; //set PLL_D = working value
	if ((result = tuner_i2c_write(tuner_id,  ADDR_PLL_D, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	byte = 0x0; //set STANDBY_LDO = 0
	if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_A, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	return SUCCESS;
}

INT32 get_sub_version(UINT32 tuner_id) //This is NOT a API function
{
    INT32 result = 0;
    
	if ((result = tuner_i2c_read(tuner_id,  ADDR_RSSI_STCK2, &sub_version[tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}
    sub_version[tuner_id] = ((sub_version[tuner_id] & 0x40) >> 6);
    NIM_PRINTF("Tuner sub version: %d\n", sub_version[tuner_id]);
	return result;
}

//#ifdef FOR_ES_TEST
#if 0
INT32 get_pwr_1v2_status(UINT32 tuner_id) //This is NOT a API function
{
    INT32 result = 0;
    
	if ((result = tuner_i2c_read(tuner_id,  ADDR_RSSI_STCK2, &pwr_1v2_present[tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}
    pwr_1v2_present[tuner_id] = ((pwr_1v2_present[tuner_id] & 0x80) >> 7);
    NIM_PRINTF("Tuner 1v2 status: %d\n", pwr_1v2_present[tuner_id]);
}

INT32 get_vga_comb_gain(UINT32 tuner_id, UINT8 *vga_comb_gain) //This is NOT a API function
{
    INT32 result = 0;
    
	if ((result = tuner_i2c_read(tuner_id,  ADDR_VGA_COMB_GAIN, vga_comb_gain, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}
    NIM_PRINTF("Tuner VGA combination gain: %d\n", *vga_comb_gain);
}
#endif

INT32 nim_m3031_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptr_tuner_config)
{
	UINT8 byte = 0;
	INT32 result = 0;
	int i = 0;
	UINT8 rf_agc_9[3] = {0};

	struct QPSK_TUNER_CONFIG_EXT * m3031_ptr = NULL;
	if (ptr_tuner_config == NULL||m3031_tuner_cnt>=MAX_TUNER_SUPPORT_NUM)
	{
	   return ERR_FAIL;
	}   

	NIM_PRINTF("enter nim_c3031_init tuner addr:0x%x!\n",ptr_tuner_config->c_tuner_base_addr);//20150526
	
	m3031_ptr=&m3031_dev_id[m3031_tuner_cnt];

	MEMCPY(m3031_ptr, ptr_tuner_config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
	
	*tuner_id = m3031_tuner_cnt;
	
	m3031_tuner_cnt++;
	
	if ((result = tuner_i2c_read(* tuner_id,  ADDR_MASK_ID, &chip_version[*tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}
    
    get_sub_version(*tuner_id);
	
//#ifdef FOR_ES_TEST
#if 0
    get_pwr_1v2_status(*tuner_id);
#endif
	if ((result = tuner_i2c_write(* tuner_id,  0x20, init_20, 16)) != SUCCESS) //initialize LPF as soon as possible to avoid C3031 LPF problem
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	    return result;
	}	

#ifdef COMPATIBILITY_WITH_OLD_M3031B  // Old M3031B need config the lna table
	//loading the LNA gain table
	for(i=0;i<22;i++)
	{
		rf_agc_9[0] = (lna_gain_table[i][2] & 0xf) | ((lna_gain_table[i][1] & 0xf) << 4);
		rf_agc_9[1] = ((lna_gain_table[i][1] >> 4) & 0x7) | ((lna_gain_table[i][0] & 0x1f) << 3); 
		rf_agc_9[2] = ((lna_gain_table[i][0] >> 5) & 0x3); 
		if ((result = tuner_i2c_write(* tuner_id, 0x5c, rf_agc_9, 3)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!result:%d\n", __FUNCTION__, __LINE__,result);//20150526
		   	return result;
		}  
		
		byte = i+192; //address must be written after the three bytes data, not before it
		if ((result = tuner_i2c_write(* tuner_id,0x5b, &byte, 1)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		   	return result;
		}  		
	}
#endif

#if 0
//#ifdef FOR_ES_TEST
    // 1 --> LGT can be uploaded 
    byte = 0x01;
    if ((result = tuner_i2c_write(* tuner_id, ADDR_RFAGC_14, &byte, 1)) != SUCCESS)
    {
        NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!result:%d\n", __FUNCTION__, __LINE__,result);//20150526
        return result;
    }  


    for(i=0;i<22;i++)
    {
        rf_agc_9[0] = (lna_gain_table_m3031b[i][2] & 0xf) | ((lna_gain_table_m3031b[i][1] & 0xf) << 4);
        rf_agc_9[1] = ((lna_gain_table_m3031b[i][1] >> 4) & 0x7) | ((lna_gain_table_m3031b[i][0] & 0x1f) << 3); 
        rf_agc_9[2] = ((lna_gain_table_m3031b[i][0] >> 5) & 0x3); 
        if ((result = tuner_i2c_write(* tuner_id, 0x5c, rf_agc_9, 3)) != SUCCESS)
        {
            NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!result:%d\n", __FUNCTION__, __LINE__,result);//20150526
            return result;
        }  
        
        byte = i+192; //address must be written after the three bytes data, not before it
        if ((result = tuner_i2c_write(* tuner_id,0x5b, &byte, 1)) != SUCCESS)
        {
            NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
            return result;
        }       
    }
#endif
	if ((result = tuner_i2c_write(*tuner_id, 0x10, init_10, 16)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	if ((result = tuner_i2c_write(*tuner_id,  0x30, init_30, 4)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}   
	
	if ((result = tuner_i2c_write(*tuner_id,  0x50, init_50, 16)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}  
	
	if ((result = tuner_i2c_write(*tuner_id,  0x60, init_60, 5)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	    return result;
	}	
	
	if ((result = tuner_i2c_write(*tuner_id,  0x78, init_78, 8)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}  
	
	if ((result = tuner_i2c_write(*tuner_id,  0x80, init_80, 5)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
	   	return result;
	}  

	remember_reg_b[*tuner_id] = init_20[0xd]; //ADDR_REG_B = 0x2d
	//RC_FREQ FSM, for process calibration, result is stored in C3031_RC_FREQ
	byte = 1+128;
	if ((result = tuner_i2c_write(*tuner_id,  0x50, &byte, 1)) != SUCCESS) //Enable and Run only RC_FREQ FSM
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	osal_task_sleep(FAST_RFAGC_WAIT_MS); //Wait for RF AGC and FSMs to settle
	if ((result = tuner_i2c_read(*tuner_id,  ADDR_MO_OSC_FREQ_L, m3031_rc_freq, 2)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	for(i=0;i<MAX_TUNER_SUPPORT_NUM;i++){
		m3031_in_standby[i] = 0;
		remember_rf_vrms[i] = init_30[0];
		remember_auto_search_mode[i] = 0;
	}
	decide_rf_vrms(*tuner_id,-1000);
    fast_settle_rf_agc(*tuner_id,0);	

	nim_m3031_lt_gain(*tuner_id, LT_GAIN_DISABLE);//disable Loop Through for non-Loop_Through models
    //nim_m3031_lt_gain(*tuner_id, 3);

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3031_control(UINT32 tuner_id,UINT32 channel_freq, UINT32 bb_sym, INT32 *channel_freq_err)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: tuner_id		:	which tuner
*  Parameter2: UINT32 freq		: channel frequency in unit of 1MHz
*  Parameter3: UINT32 bb_sym	: symbol rate
*  Parameter3: INT32 *channel_freq_err	: called: fraction of channel_freq in unit of 1/256 MHz, if not available, must assign 0   //delete  by dennis
*										  return: (real channel frequency - channel_freq) in unit of 1/256 MHz
*
* Return Value: INT32			: Result
*****************************************************************************/

//!!! important: M3031 demodulator to tuner AGC polarity (also called direction in some demodulators)
//is reverse of other tuners. The deomdulator's driver must be changed to use the 
//correct polarity. If a wrong polarity is used, M3031 performance will be down graded but it still can 
//receive some channels if the C/N requirement is not high. If you measure M3031 AGC input pin voltage
//you can see the voltage change with signal level change when signal level is in the high region if the 
//AGC polarity is set correct, otherwise you will see the voltage is tied at lowest or highest.
//Demodulator driver may program the AGC polarity at multiple position, all the position should be corrected
//or a touching detection can be done at the register program function to detect and correct it. 

INT32 nim_m3031_control(UINT32 tuner_id,UINT32 channel_freq, UINT32 bb_sym)
{
	UINT8 pll_p;//PLL_P;
	UINT32 p; 
	UINT32 fvco;//Fvco;
	int icp;//Icp;
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;
	UINT32 ideal_cut_off;//Ideal_Cut_Off;
	UINT8 lpf_ctu_off;//LPF_CUTOFF;
	UINT32 pll_cp;//PLL_CP; //result is 8 bits
	UINT8 band_start;//BAND_START;
	UINT32 pll_m;//PLL_M;
	UINT8 pll_m_msb;//PLL_M_MSB;
	UINT8 pll_m_lsb;//PLL_M_LSB;
	UINT8 pll_fltr;//PLL_FLTR;
	UINT8 mix_cap;//MIX_CAP;
	//UINT8 BUF_DRIVE;
	UINT32 lfp_freq_10khz;//LPF_FREQ_10KHz;
	UINT32 lpf_tuning;//LPF_TUNING; //result is 8 bits
	UINT8 pll_reg_26[6];//PLL_reg_26[6];
	UINT8 lpf_reg_21[3];//LPF_reg_21[3];
	int result;
	UINT8 byte;
	UINT8 old_rf_vrms;//old_RF_VRMS;
	int auto_search_mode;
	UINT32 real_fvco;
	UINT32 real_freq;
	UINT32 hp_channel_freq; //merge channel_freq and channel_freq_err into a high precision value
	UINT32 freq_err_5bits;
	UINT32 channel_freq_tmp;
	UINT8 rf_filter;//RF_filter;
    UINT8 bbagc_speed;
	UINT8 rfagc_speed;

	NIM_PRINTF("Enter nim_c3031_control: channel_freq = %d bb_sym = %d \n",channel_freq,bb_sym);
	
	if(NIM_TUNER_SET_STANDBY_CMD == channel_freq)
	{//the standby command
		result = nim_m3031_standby(tuner_id);
		return result;
	}
	
	freq_err_5bits = ((g_channel_freq_err >> 2) + 1) >> 1;
	hp_channel_freq = (channel_freq << 5) + freq_err_5bits; //the precision is limitted by hp_channel_freq * PLL_D_Div_Freq not overflow 32 bits

	if(tuner_id >= m3031_tuner_cnt)
	{
		return ERR_FAIL;
	}
	//m3031_ptr = &m3031_dev_id[tuner_id];

	if(bb_sym==0)
	{ //auto search mode
		bb_sym = 100000; //50000 for 39M cut off
		auto_search_mode = 1;
	}
	else
	{
		auto_search_mode = 0;
	}
  	
	byte = 0x00;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_BBAGC_1, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	if(bb_sym < 4000)
	{
		bbagc_speed = 7;
		rfagc_speed = 7;
	}
	else if(bb_sym < 12000)
	{
	  	bbagc_speed = 8;
	  	rfagc_speed = 8;
	}
	else 
	{
	  	bbagc_speed = 8;
	  	rfagc_speed = 8;
	}
	
	//fast_settle_RFAGC(tuner_id,rfagc_speed);	//there is a wait in this function, so we write directly to avoid waiting
	if ((result = tuner_i2c_write(tuner_id, ADDR_RFAGC_SPEED, &rfagc_speed, 1)) != SUCCESS) 
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	//enable all modules
	if(m3031_in_standby[tuner_id])
	{
		leave_deep_sleep(tuner_id);
		if ((result = tuner_i2c_write(tuner_id,  0x10, init_10, 5)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}
		
		//leave LT_EN not touched
		if ((result = tuner_i2c_write(tuner_id,  0x16, &init_10[6], 4)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}

		byte = 0;
		if ((result = tuner_i2c_write(tuner_id,  ADDR_MAN_RFAGC, &byte, 1)) != SUCCESS) //enable RFAGC
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}
			fast_settle_rf_agc(tuner_id,rfagc_speed);	
	}

	//you can disable decide_RF_VRMS for debug
  if(!(remember_auto_search_mode[tuner_id] & auto_search_mode)){ //only skip decide RF VRMS when two continuous auto search mode is called
    	old_rf_vrms = remember_rf_vrms[tuner_id];
	    decide_rf_vrms(tuner_id,-1000);
	    if(old_rf_vrms!=remember_rf_vrms[tuner_id]) //signal level change largely
           fast_settle_rf_agc(tuner_id,rfagc_speed);	
  }     

	
	remember_auto_search_mode[tuner_id] = auto_search_mode;
  	if ((result = tuner_i2c_write(tuner_id,  ADDR_BBAGC_1, &bbagc_speed, 1)) != SUCCESS)
  	{
  		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
  	}

	//(1) Compute 'channel frequency' dependent register 
#ifdef EXTEND_LOW_FREQ
	if (channel_freq < 250)
    {
        return ERR_FAIL;
	} 
    else if (channel_freq < 312.5)
    {
        pll_p = 3; //divide by 16
        p = 16;
    } 
    else if (channel_freq < 625)
    {
        pll_p = 2; //divide by 8
        p = 8;
    } 
	else 
#endif
    if (channel_freq < 1250) 
    {
		pll_p = 1; //divide by 4
		p = 4;
	} 
    else 
    {
		pll_p = 0; //divide by 2
		p = 2;
	}
   
	fvco = channel_freq * p;
	//M  = fvco / Fref = Flo * P / (Fxtal/D)  = fvco P * D/Fxtal
	pll_m = hp_channel_freq * pll_d_div_freq; //hp_channel_freq is 16.x bits, PLL_D_Div_Freq is 15.x bits so no overflow
	if(p==2)
		pll_m = pll_m >> 1;
	else if(p>=8) //for EXTEND_LOW_FREQ
		pll_m = pll_m << (pll_p - 1);
	
	//if(chip_version[tuner_id]==CHIP_ID_M3031B){  //avoid PLL_M odd value because of a S3031b bug
	//if((CHIP_ID_M3031B == chip_version[tuner_id]) && (M3031B_OLD == sub_version[tuner_id])){
		pll_m = (pll_m >> 19); //right shift 5 bits for hp_channel_freq and 16 bits for pll_d_div_freq, left shift 2 bits for P
		if(pll_p==1) //round to 4x
			pll_m = (pll_m + 2) & 0xfffc;
		else  //round to 2x
			pll_m = (pll_m + 1) & 0xfffe;
	//	}
	//else
	//	pll_m = ((pll_m >> 18)+1)>>1; //right shift 5 bits for hp_channel_freq and 16 bits for pll_d_div_freq, left shift 2 bits for P
	//compute the round off error to help the demodulator lock faster
	//real_fvco = PLL_M / pll_d_div_freq = (PLL_M * M3031_XTAL_FREQ_MHZ) / pll_d
	real_fvco = ((pll_m * M3031_XTAL_FREQ_MHZ)<<8) / pll_d;
	real_freq = real_fvco / p;
	channel_freq_tmp = (channel_freq << 8) + g_channel_freq_err;
	
	//*channel_freq_err = real_freq - channel_freq_tmp;
	g_channel_freq_err = real_freq - channel_freq_tmp;
	
	pll_m_msb = (UINT8)(pll_m>>8);
	pll_m_lsb = (UINT8)(pll_m & 0xff);

	//Compute ideal charge pump current (mA)
	//icp = 0.4*(fvco - 2500)/3000 + 0.6; = (2*(Fvco-2500)+9000)  (//all * 15000)
    //pll_cp = round((icp -0.2)/0.2)]; = (icp - 3000) / 3000 
	icp = 2*(fvco - 2500) + 9000; 
    pll_cp = ((icp -3000)+1500)/3000; //+1500 for round off
   
	//#If Fvco > 4GHz then reduce Loop Filter Resistance
	if (fvco>4000) {
		pll_fltr = 3;          //25KOhms
	} else {
		pll_fltr = 4;          //30KOhms
	}

	band_start = 32; //start at half band

	//(2) Compute 'Symbol Rate' dependent register
//	ideal_cut_off = (bb_sym*1485)/2000; //1.35 * 1.1 = 1.485, for 10% margin. 
	ideal_cut_off = (bb_sym*(1485+100))/2000;//1.35 * 1.1 = 1.485, for 10% margin. 
	ideal_cut_off += 1000; //for LO limitted resolution 

	//Apply 10 row look-up table by selecting LPF cut frequency just above Ideal
	//disable narrowest bandwidths so that the higher level software can apply low symbol rate shift 3M

		lpf_ctu_off = 7;
	if (ideal_cut_off > 9890)
		lpf_ctu_off = 6;
	if (ideal_cut_off > 12670)
		lpf_ctu_off = 5;
	if (ideal_cut_off > 15890)
		lpf_ctu_off = 4;
	if (ideal_cut_off > 19800)
		lpf_ctu_off = 3;
	if (ideal_cut_off > 24210)
		lpf_ctu_off = 2;
	if (ideal_cut_off > 31170)
		lpf_ctu_off = 1;
	if (ideal_cut_off > 38470)
		lpf_ctu_off = 0;

	if(auto_search_mode)
		lpf_ctu_off = 4;
		//LPF_CUTOFF = 4;  //force to use a bandwidth close to AV2018 to avoid auto search mistake

/*#ifdef AVOID_LPF_WIDEST
	if(lpf_ctu_off < 2) //limit LPF_CUTOFF according to demod's sample rate
		lpf_ctu_off = 2;  //also limit LPF_CUTOFF = 2 to avoid S3031b widest bandwidth filter oscillation
#endif*/
	if((CHIP_ID_M3031B == chip_version[tuner_id]) && (M3031B_OLD == sub_version[tuner_id]))
		{
			if(lpf_ctu_off < 2) //limit LPF_CUTOFF according to demod's sample rate
			{
				lpf_ctu_off = 2;  //also limit LPF_CUTOFF = 2 to avoid S3031b widest bandwidth filter oscillation
				lpf_tuning  = 0;  //extend the LPF widest frequency when LPF is limitted to 2, "lpf_ctu_off == 1" doesn't need extend because it support 38470*2 = 76.94MHz
			}
       	}
    else{
			if(lpf_ctu_off < 1) 
				lpf_ctu_off = 1;  //M3031B_NEW and future chips limit LPF_CUTOFF = 1 because it is wide enough for 72MHz and "lpf_ctu_off == 0" has not enough low pass filter
       	}
	
	lfp_freq_10khz = (UINT32)m3031_rc_freq[0] + ((UINT32)m3031_rc_freq[1] << 8);
	if (lfp_freq_10khz<490) 
		lfp_freq_10khz = 490;           //To protect equation below
	lpf_tuning  = (29*(lfp_freq_10khz-490) + 500) /1000; //+500 for round off
/*
#ifdef AVOID_LPF_WIDEST
     if(lpf_ctu_off==2)
     	lpf_tuning  = 0; //extend the LPF widest frequency when LPF is limitted to 2
#endif*/

//BUF_DRIVE and MIX_C versus symbol rate
	mix_cap = 3;                //3 if < 16M
	
	//BUF_DRIVE is fixed at 3 (max) for in case there is strong adjacent channel
	//BUF_DRIVE = 0;              //0.5mA if <16M
	if (bb_sym > 16000){
		mix_cap = 2;
//		BUF_DRIVE = 1;           //1mA
	}
	if (bb_sym > 32000){
		mix_cap = 1;
//		BUF_DRIVE = 2;           //2mA
	}
	if (bb_sym > 60000){
		mix_cap = 0;             //Full Bandwidth only for fast scan
//		BUF_DRIVE = 3;           //4mA for fast scan
	}
	//(3) Apply computed value
	pll_reg_26[0] = (UINT8)pll_cp;
	pll_reg_26[1] = (UINT8)pll_d;
	pll_reg_26[2] = (UINT8)(pll_fltr<<1) ;
	//pll_reg_26[2] |= 0x01;
	pll_reg_26[2] = (UINT8)(((pll_fltr & 0x07)<<1) | (init_20[8] & 0xf1)); // retain the intitial value
	pll_reg_26[3] = (UINT8)pll_m_msb;
	pll_reg_26[4] = (UINT8)pll_m_lsb;
	//pll_reg_26[5] = (UINT8)pll_p;
	NIM_PRINTF("[%s %d]init_20[11] = 0x%x,init_20[15]= 0x%x\n",__FUNCTION__, __LINE__,init_20[11],init_20[15]);
	
#ifdef LNA_H7_ATT
	if (channel_freq < H7_ATT_FREQ)
	{
		pll_reg_26[5] = (UINT8)((pll_p & 0x03) | 0x04);
	}
	else	
#endif
	{
		pll_reg_26[5] = (UINT8)(pll_p & 0x03);
	}
	NIM_PRINTF("[%s %d]pll_reg_26[5] = 0x%x\n", __FUNCTION__, __LINE__, pll_reg_26[5]);
	
	lpf_reg_21[0] = (UINT8)lpf_ctu_off;
	lpf_reg_21[1] = (UINT8)lpf_tuning;
	lpf_reg_21[2] = (UINT8)mix_cap;

	if (channel_freq < 1260)
		rf_filter = 2; //use 2 has better WIFI rejection, but may loss a <1dB sensitivity and the return loss is downgraded a little
	else
		rf_filter = 0;
	remember_reg_b[tuner_id] = (remember_reg_b[tuner_id] & 0xcf) | (rf_filter << 4);
	if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_B, &remember_reg_b[tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	if ((result = tuner_i2c_write(tuner_id,  0x26, pll_reg_26, 6)) != SUCCESS) 
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	if ((result = tuner_i2c_write(tuner_id,  0x21, lpf_reg_21, 3)) != SUCCESS) 
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}	

	if ((result = tuner_i2c_write(tuner_id,  ADDR_PLL_INIT_8, &band_start, 1)) != SUCCESS) //Manson: we may remove this line because it is initialized and never changed
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	//if ((result = Tuner_I2C_write(tuner_id,  ADDR_BUF_DRIVE, &BUF_DRIVE, 1)) != SUCCESS)
	//	return result;

	//(4) Reset and Restart FSMs
	byte = 0x18; //don't reset RF and BB AGC
	if ((result = tuner_i2c_write(tuner_id,  ADDR_TOP_1, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	byte = 2+4+8+16+128;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_TOP_1, &byte, 1)) != SUCCESS) //Start RFAGC + BBAGC +  PLLINIT + DC INIT
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	m3031_in_standby[tuner_id] = 0;
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3031_status(UINT32 tuner_id)
*
* Read tuner FSM current status
*
* Arguments:
*  Parameter1: tuner_id		:	which tuner
*
* Return Value: INT32		:	C3031 FSM current status
*****************************************************************************/
#define C3031_STATUS_ERROR -1
INT32 nim_m3031_status(UINT32 tuner_id, UINT8 *lock)
{
	UINT8 status =0;
	int result;

	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr = NULL;
	*lock = 1;
	
	if(tuner_id >= m3031_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return C3031_STATUS_ERROR;
	
	//m3031_ptr = &m3031_dev_id[tuner_id];
	
	result = tuner_i2c_read(tuner_id, ADDR_MO_TOP_FSM, &status, 1);
	if ( result!= SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
		return C3031_STATUS_ERROR;
	}
	else 
	{
		return status;
	}
}
/*****************************************************************************
* INT32 nim_c3031_gain(UINT32 tuner_id, int AGC_level)
*
* Read and calculate tuner gain
*
* Arguments:
*  Parameter1: tuner_id		:	which tuner
*  Parameter2: AGC_level	:	demodulator's AGC out value
*
* Return Value: INT32		: total gain in 1/256 dB, gain is signed and can be negative or positive
*****************************************************************************/
INT32 nim_m3031_gain(UINT32 tuner_id, UINT32 agc_level)
{
	INT32 total_gain;
	int rf_coarse_gain;//RF_Coarse_Gain;
	int rf_fine_gain;//RF_Fine_Gain;
	int mix_lgain;//MIX_LGAIN;
	int dgb_gain;//DGB_GAIN;
	int vga_gain;//VGA_GAIN;
	int result;
	UINT8 rfagc_gain_index[2]; //read from tuner to calculate its gain
	UINT8 bbagc_gain_index; //read from tuner to calculate its gain
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;
	int rf_signal_level;
	int rf_signal_change;
	int agc_level_tmp;//AGC_level_tmp;
	//UINT8 old_rf_vrms;//old_RF_VRMS;
	
	//I2C read must return the current register value, otherwise we will implement waiting and read I2C twice
	if(tuner_id>=m3031_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAIL;

	//m3031_ptr = &m3031_dev_id[tuner_id];
	
	if ((result = tuner_i2c_read(tuner_id,  ADDR_MO_GAIN_C_INDEX, rfagc_gain_index, 2)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	if ((result = tuner_i2c_read(tuner_id,  ADDR_BBAGC_3, &bbagc_gain_index, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_read Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	//detect signal level large change (only happen in SFU testing)
	rf_signal_level = (rfagc_gain_index[0] * 2) + ((rfagc_gain_index[1]+2)>>2) + (remember_rf_vrms[tuner_id] * 3); //for the same signal level, higher RF_VRMS result in lower rfagc_gain_index
	rf_signal_change = rf_signal_level - remember_rf_signal_level[tuner_id];
	if(rf_signal_change < 0)
		rf_signal_change = -rf_signal_change;
 //disable decide_rf_vrms for debug
/* 
	if(rf_signal_change > 5){ //large change happen from last time RF_VRMS is changed, if we set this threshold small, we will need to consider fine gain and dead zone.
		old_RF_VRMS = remember_RF_VRMS[tuner_id];
		decide_RF_VRMS(tuner_id,rf_signal_level); //RF AGC speed must be slow here to avoid demodulator AGC can't catch up
	    fast_settle_RFAGC(tuner_id);	
		if(old_RF_VRMS != remember_RF_VRMS[tuner_id]){ //only update signal level when RF_VRMS is changed. 
			remember_rf_signal_level[tuner_id] = rf_signal_level; 
		}
	}
*/
	rf_coarse_gain	= 256*(27 - rfagc_gain_index[0]*2);	//Signed Int, (Unit is dB/256)   Gain_dB = 27-Coarse*2
	rf_fine_gain	= 64*(7-rfagc_gain_index[1]);		//Signed Int, (Unit is dB/256)   Gain_dB = 0.25*(7-Fine)
	mix_lgain		= (bbagc_gain_index & 64) >> 6;		//MIX_LGAIN is bit pos 6        Gain_dB = (1-MIX_LGAIN)*12
	dgb_gain		= 56*(bbagc_gain_index & 63);		//DGB_GAIN is bit pos [5:0].    Gain_dB = 0.22*DGB_GAIN, 56 = 256 * 0.22

	if(agc_level>127)
		agc_level_tmp = 256-agc_level;
	else
		agc_level_tmp = -agc_level;
	vga_gain = agc_level_tmp*37-3961; //Formula is Gain_dB=AGC_in_V*11.3-15.3, Gmin=-13.5, Gmax=26, unit is dB/256, 37 = 3.3*11.3

    // Add by paladin from manson 2015/10/27 for correct mix gain
	//total_gain = vga_gain + (1-mix_lgain)*3072 + rf_coarse_gain + rf_fine_gain + dgb_gain; //unit is dB/256
	total_gain = vga_gain + (1-mix_lgain)*2560 + 512 + rf_coarse_gain + rf_fine_gain + dgb_gain; //unit is dB/256


    // Add by paladin 2015/10/22 for agc dbm linear indication
    NIM_PRINTF("From Tuner: total_gain = %d, vga_gain = %d, mix_lgain = %d, rf_coarse_gain = %d, rf_fine_gain = %d, dgb_gain = %d\n",\
        total_gain, vga_gain, mix_lgain, rf_coarse_gain, rf_fine_gain, dgb_gain);


    // Bit[0] =  mix_lgain status
    if(0 == mix_lgain)
        g_agc_gain_flag |= 0x00000001;  // mix_lgain open  
    else
        g_agc_gain_flag &= 0xFFFFFFFE;  // mix_lgain closed

    // Bit[1] =  rf_coarse_gain status
    if(0 == rfagc_gain_index[0])
        g_agc_gain_flag |= 0x00000002;  //  rf_coarse_gain saturation
    else
        g_agc_gain_flag &= 0xFFFFFFFD;  // rf_coarse_gain not saturation

    // Bit[2] =  dgb_gain status
    if(3080 == dgb_gain)
        g_agc_gain_flag |= 0x00000004;  // dgb_gain saturation
    else
        g_agc_gain_flag &= 0xFFFFFFFB; // dgb_gain not saturation
    // end

//#ifdef FOR_ES_TEST
#if 0
    UINT8 vga_comb_gain = 0;
    get_vga_comb_gain(tuner_id, &vga_comb_gain);
#endif


	return total_gain;
}

/*****************************************************************************
* INT32 nim_c3031_LT_gain(UINT32 tuner_id,  UINT32 LT_gain)
*
* Set tuner loop through gain, called once at initialization, and when UI select LT GAIN on UI
* Change LT gain will cause the secondary receiver large signal level change. So don't call this function frequently
* When input signal is strong, the loop through gain is reduced by hardware automatically
* Arguments:
*  Parameter1: tuner_id		:	which tuner
*
* Return Value: INT32		:	SUCCESS or ERROR code
*****************************************************************************/
INT32 nim_m3031_lt_gain(UINT32 tuner_id, INT32 lt_gain)
{
	//UINT8 rfagc_gain_c_index;
	UINT8 lt_gain_apply;//LT_gain_apply;
	UINT8 byte;
	int result;
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;
	//int rf_signal_level;

	if(tuner_id>=m3031_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAIL;
    //m3031_ptr = &m3031_dev_id[tuner_id];

	if(lt_gain==LT_GAIN_DISABLE){
		remember_reg_b[tuner_id] = remember_reg_b[tuner_id] & 0xf8;
		if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_B, &remember_reg_b[tuner_id], 1)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}

		byte = 0;
		if ((result = tuner_i2c_write(tuner_id,  ADDR_LT_EN, &byte, 1)) != SUCCESS)
		{
			NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
			return result;
		}

		return SUCCESS;
	}


	if(lt_gain==-4)
		lt_gain_apply = 0; 
	else if(lt_gain==-3)
		lt_gain_apply = 2; 
	else if(lt_gain==-2)
		lt_gain_apply = 1; 
	else if(lt_gain==-1)
		lt_gain_apply = 3; 
	else if(lt_gain==0)
		lt_gain_apply = 4; 
	else if(lt_gain==1)
		lt_gain_apply = 6; 
	else if(lt_gain==2)
		lt_gain_apply = 5; 
	else if(lt_gain==3)
		lt_gain_apply = 7; 
	else
		return M3031_GAIN_ERROR;

	remember_reg_b[tuner_id] = (remember_reg_b[tuner_id] & 0xf8) | lt_gain_apply;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_B, &remember_reg_b[tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	byte = 1;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_LT_EN, &byte, 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3031_standby(UINT32 tuner_id)
*
* Tuner enter standby mode
*
* Arguments:
*  Parameter1: tuner_id		:	which tuner
*  Parameter2: LT_gain		:	100--auto, (-4~3) -- (-15,-8,-4,-2,0,2,4,6)dB
*
* Return Value: INT32		:	SUCCESS or ERROR code
*****************************************************************************/
INT32 nim_m3031_standby(UINT32 tuner_id){
	int result;
	UINT8 byte;
	UINT8 rf_filter;//RF_filter;
	//struct QPSK_TUNER_CONFIG_EXT * m3031_ptr;

	if(tuner_id >= m3031_tuner_cnt)
		return ERR_FAIL;
	//m3031_ptr = &m3031_dev_id[tuner_id];

	byte = 1;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_MAN_RFAGC, &byte, 1)) != SUCCESS) //Stop RFAGC and set to manual gain mode
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	byte = 20;
	if ((result = tuner_i2c_write(tuner_id,  ADDR_RG_GAIN_C_INDEX, &byte, 1)) != SUCCESS) //Set LNA to almost minimum. 
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	if ((result = tuner_i2c_write(tuner_id,  0x10, standby_10, 5)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}
	
	rf_filter = 0; //turn off RF filter in standby mode
	remember_reg_b[tuner_id] = (remember_reg_b[tuner_id] & 0xcf) | (rf_filter << 4);
	if ((result = tuner_i2c_write(tuner_id,  ADDR_REG_B, &remember_reg_b[tuner_id], 1)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	//leave LT_EN not touched
	if ((result = tuner_i2c_write(tuner_id,  0x16, &standby_10[6], 4)) != SUCCESS)
	{
		NIM_PRINTF("[%s]line=%d, Tuner_I2C_write Failed!\n", __FUNCTION__, __LINE__);//20150526
		return result;
	}

	enter_deep_sleep(tuner_id);
	m3031_in_standby[tuner_id] = 1;
    return SUCCESS;
}

INT32 nim_m3031b_command(UINT32 tuner_id, INT32 cmd, INT32 *param)
{    
	switch(cmd)
	{
		case NIM_TUNER_C3031B_ID://c3031b tuner	    
			break;

		case NIM_TUNER_SET_C3031B_FREQ_ERR://set c3031b channel_freq_err
			g_channel_freq_err = *param;
			break; 

		case NIM_TUNER_GET_C3031B_FREQ_ERR://get c3031b channel_freq_err
			*param = g_channel_freq_err;
			break;
            
        case NIM_TUNER_GET_C3031B_GAIN_FLAG://get c3031b LNA status 
            *param = g_agc_gain_flag;
            break;       

		default:
			return  ERR_FAILUE; 
	}
    return SUCCESS; 
}

