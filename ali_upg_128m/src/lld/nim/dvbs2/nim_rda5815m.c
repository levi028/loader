/*****************************************************************************
*    Copyright (C) 2010 ALi Corp. All Rights Reserved.
*    
*    Company confidential and Properietary information.       
*    This information may not be disclosed to unauthorized  
*    individual.    
*    File: nim_rda5815m.c
*   
*    Description: 
*    
*    History: 
*    Date                         Athor        Version                 Reason
*    ========       ========     ========       ========
*    2013/8/26               David         V1.0
*        
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_rda5815m.h"

// Tuner crystal CLK Freqency
static UINT32 rda5815m_tuner_cnt = 0;
static struct QPSK_TUNER_CONFIG_EXT * rda5815m_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static unsigned char tuner_initial[MAX_TUNER_SUPPORT_NUM] = {0,0}; 
#define Xtal_27M
//#define Xtal_30M
//#define Xtal_24M

// I2C write function (register start address, register array pointer, register length)
static int Tuner_I2C_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;
	struct QPSK_TUNER_CONFIG_EXT * rda5815m_ptr = NULL;

	rda5815m_ptr = rda5815m_dev_id[tuner_id];	
	data[0] = reg_start;

	while((rd+15)<length)
	{
		MEMCPY(&data[1], &buff[rd], 15);
		i2c_result = i2c_write(rda5815m_ptr->i2c_type_id, rda5815m_ptr->c_tuner_base_addr, data, 16);
		rd+=15;
		data[0] += 15;
		if(SUCCESS != i2c_result)
			return i2c_result;
	}
	MEMCPY(&data[1], &buff[rd], length-rd);
	i2c_result = i2c_write(rda5815m_ptr->i2c_type_id, rda5815m_ptr->c_tuner_base_addr, data, length-rd+1);

	return i2c_result;
}

static int Tuner_I2C_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16];
	UINT32 rd = 0;
	int i2c_result;
	struct QPSK_TUNER_CONFIG_EXT * rda5815m_ptr = NULL;

	rda5815m_ptr = rda5815m_dev_id[tuner_id];	
	data[0] = reg_start;

	while((rd+15)<length)
	{
		i2c_result = i2c_write_read(rda5815m_ptr->i2c_type_id, rda5815m_ptr->c_tuner_base_addr, data, 1, 15);
		MEMCPY(&buff[rd], &data[0], 15);
		rd+=15;
		data[0] += 15;
		if(SUCCESS != i2c_result)
			return i2c_result;
	}
	i2c_result = i2c_write_read(rda5815m_ptr->i2c_type_id, rda5815m_ptr->c_tuner_base_addr, data, 1, length-rd);
	MEMCPY(&buff[rd], &data[0], length-rd);

	return i2c_result;
}
// time delay function ( minisecond )
static void Time_DELAY_MS (unsigned int ms)
{
	osal_task_sleep(ms);
}

INT32 nim_rda5815m_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//INT32 result;
	struct QPSK_TUNER_CONFIG_EXT * rda5815m_ptr = NULL;

	if (ptrTuner_Config == NULL||rda5815m_tuner_cnt>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;
	rda5815m_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
	if(!rda5815m_ptr)
	      return ERR_FAILUE;
	MEMCPY(rda5815m_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
	rda5815m_dev_id[rda5815m_tuner_cnt] = rda5815m_ptr;
	*tuner_id = rda5815m_tuner_cnt;
	rda5815m_tuner_cnt++;
	
	return SUCCESS;
}

/***********************************************************************
* unsigned int nim_rda5815m_control (unsigned int tuner_id, unsigned int channel_freq, unsigned int bb_sym)
*  Arguments:
*  Parameter1: unsigned int channel_freq		: Channel frequency (MHz)
*  Parameter2: unsigned int bb_sym		    : Baseband Symbol Rate (KHz)
*  Return Value: unsigned int			: Result
							
					
***********************************************************************/
INT32 nim_rda5815m_control(UINT32 tuner_id, UINT32 channel_freq, UINT32 bb_sym)
{
       UINT8 data;
       UINT8 i;
 	UINT32 temp_value = 0;
	UINT32 bw;
	UINT8 Filter_bw_control_bit;	
    
	//struct QPSK_TUNER_CONFIG_EXT * rda5815m_ptr = NULL;


	if(tuner_id >= rda5815m_tuner_cnt || tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;

	//rda5815m_ptr = rda5815m_dev_id[tuner_id];

	// At Power ON, tuner_initial = 0, 
	if (tuner_initial[tuner_id] == 0) 
	{
		Time_DELAY_MS(1);
		static UINT8 control_data[][2] = 
		{
        	    {0x04,0x04},
        	    {0x04,0x05}, 
        
        	 // Initial configuration start
        
        	//pll setting 

                {0x1a,0x13},
                {0x41,0x53},
                {0x38,0x9B},
                {0x39,0x15},
                {0x3A,0x00},
                {0x3B,0x00},
                {0x3C,0x0c},
                {0x0c,0xE2},
                {0x2e,0x6F},

        #ifdef Xtal_27M
                {0x72,0x07},	
                {0x73,0x10},
                {0x74,0x71},
                {0x75,0x06}, 
                {0x76,0x40},
                {0x77,0x89},
                {0x79,0x04},	
                {0x7A,0x2A},
                {0x7B,0xAA},
                {0x7C,0xAB},
        #endif
        #ifdef Xtal_30M
                {0x72,0x06},	
                {0x73,0x60},
                {0x74,0x66},
                {0x75,0x05}, 
                {0x76,0xA0},
                {0x77,0x7B},
                {0x79,0x03},	
                {0x7A,0xC0},
                {0x7B,0x00},
                {0x7C,0x00},
        #endif
        #ifdef Xtal_24M
                {0x72,0x08},	
                {0x73,0x00},
                {0x74,0x80},
                {0x75,0x07}, 
                {0x76,0x10},
                {0x77,0x9A},
                {0x79,0x04},	
                {0x7A,0xB0},
                {0x7B,0x00},
                {0x7C,0x00},
        #endif

                {0x2f,0x57},
                {0x0d,0x70},
                {0x18,0x4B},
                {0x30,0xFF},
                {0x5c,0xFF},
                {0x65,0x00},
                {0x70,0x3F},
                {0x71,0x3F},
                {0x53,0xA8},
                {0x46,0x21},
                {0x47,0x84},
                {0x48,0x10},
                {0x49,0x08},
                {0x60,0x80},
                {0x61,0x80},
                {0x6A,0x08},
                {0x6B,0x63},
                {0x69,0xF8},
                {0x57,0x64},
                {0x05,0xaa},
                {0x06,0xaa},
                {0x15,0xAE},
                {0x4a,0x67},
                {0x4b,0x77},

           //agc setting

                {0x4f,0x40},
                {0x5b,0x20},

                {0x16,0x0C},
                {0x18,0x0C},            
                {0x30,0x1C},            
                {0x5c,0x2C},            
                {0x6c,0x3C},            
                {0x6e,0x3C},            
                {0x1b,0x7C},            
                {0x1d,0xBD},            
                {0x1f,0xBD},            
                {0x21,0xBE},            
                {0x23,0xBE},            
                {0x25,0xFE},            
                {0x27,0xFF},            
                {0x29,0xFF},            
                {0xb3,0xFF},            
                {0xb5,0xFF},

                {0x17,0xF0},            
                {0x19,0xF0},            
                {0x31,0xF0},            
                {0x5d,0xF0},            
                {0x6d,0xF0},            
                {0x6f,0xF1},            
                {0x1c,0xF5},            
                {0x1e,0x35},            
                {0x20,0x79},            
                {0x22,0x9D},            
                {0x24,0xBE},            
                {0x26,0xBE},            
                {0x28,0xBE},            
                {0x2a,0xCF},            
                {0xb4,0xDF},            
                {0xb6,0x0F},            

                {0xb7,0x15},	//start    
                {0xb9,0x6c},	           
                {0xbb,0x63},	           
                {0xbd,0x5a},	           
                {0xbf,0x5a},	           
                {0xc1,0x55},	           
                {0xc3,0x55},	           
                {0xc5,0x47},	           
                {0xa3,0x53},	           
                {0xa5,0x4f},	           
                {0xa7,0x4e},	           
                {0xa9,0x4e},	           
                {0xab,0x54},            
                {0xad,0x31},            
                {0xaf,0x43},            
                {0xb1,0x9f},

                {0xb8,0x6c}, //end      
                {0xba,0x92},            
                {0xbc,0x8a},            
                {0xbe,0x8a},            
                {0xc0,0x82},            
                {0xc2,0x93},            
                {0xc4,0x85},            
                {0xc6,0x77},            
                {0xa4,0x82},            
                {0xa6,0x7e},            
                {0xa8,0x7d},            
                {0xaa,0x6f},            
                {0xac,0x65},            
                {0xae,0x43},            
                {0xb0,0x9f},             
                {0xb2,0xf0},             

                {0x81,0x92}, //rise     
                {0x82,0xb4},            
                {0x83,0xb3},            
                {0x84,0xac},            
                {0x85,0xba},            
                {0x86,0xbc},            
                {0x87,0xaf},            
                {0x88,0xa2},            
                {0x89,0xac},            
                {0x8a,0xa9},            
                {0x8b,0x9b},            
                {0x8c,0x7d},            
                {0x8d,0x74},            
                {0x8e,0x9f},           
                {0x8f,0xf0},               
                                 
                {0x90,0x15}, //fall     
                {0x91,0x39},            
                {0x92,0x30},            
                {0x93,0x27},            
                {0x94,0x29},            
                {0x95,0x0d},            
                {0x96,0x10},            
                {0x97,0x1e},            
                {0x98,0x1a},            
                {0x99,0x19},            
                {0x9a,0x19},            
                {0x9b,0x32},            
                {0x9c,0x1f},            
                {0x9d,0x31},            
                {0x9e,0x43},                  
		};	
        
		for(i = 0 ; i < sizeof(control_data)/sizeof(control_data[0]); i++)
		{
			data = control_data[i][1];
			Tuner_I2C_write(tuner_id,control_data[i][0], &data, 1);
                     // libc_printf("*_%0x,%0x_*\n",control_data[i][0],data);            
		}
	
		// After power on initial
		tuner_initial[tuner_id] = 1;
	
 		// Time delay 10ms
		Time_DELAY_MS(10);
	
	}

       data=0xc1;
       Tuner_I2C_write(tuner_id,0x04,&data, 1); 
       data=0x95;
       Tuner_I2C_write(tuner_id,0x2b,&data, 1);     
	//set frequency start
	#ifdef Xtal_27M		
	temp_value = (UINT32)(channel_freq* 77672);//((2^21) / RDA5815_XTALFREQ);
	#endif
	#ifdef Xtal_30M		
	temp_value = (UINT32)(channel_freq* 69905);//((2^21) / RDA5815_XTALFREQ);
	#endif
	#ifdef Xtal_24M		
	temp_value = (UINT32)(channel_freq* 87381);//((2^21) / RDA5815_XTALFREQ);
	#endif

	data = ((UINT8)((temp_value>>24)&0xff));
	Tuner_I2C_write(tuner_id,0x07,&data, 1);
	data = ((UINT8)((temp_value>>16)&0xff));	
	Tuner_I2C_write(tuner_id,0x08,&data, 1);	
   	data = ((UINT8)((temp_value>>8)&0xff));
	Tuner_I2C_write(tuner_id,0x09,&data, 1);	
   	data = ((UINT8)( temp_value&0xff));
	Tuner_I2C_write(tuner_id,0x0a,&data, 1);
	//set frequency end
	
	// set Filter bandwidth start
	bw=bb_sym;
	if (bb_sym == 0)
       {
	      //Filter_bw_control_bit = 40;
	     Filter_bw_control_bit = 30; 
	 }
	else
	{
	    Filter_bw_control_bit = (UINT8)((bw*135/200+4000)/1000);
	 }

    	if(Filter_bw_control_bit<4)
		Filter_bw_control_bit = 4;    // MHz
	else if(Filter_bw_control_bit>40)
		Filter_bw_control_bit = 40;   // MHz

       Filter_bw_control_bit&=0x3f;
	Filter_bw_control_bit|=0x40;		
	
	Tuner_I2C_write(tuner_id,0x0b,&Filter_bw_control_bit, 1);
	// set Filter bandwidth end
	
	data=0xc3;
	Tuner_I2C_write(tuner_id,0x04,&data, 1); 
	data=0x97;	
       Tuner_I2C_write(tuner_id,0x2b,&data, 1);
	Time_DELAY_MS(5);//Wait 5ms;
  
    return SUCCESS;
}

INT32 nim_rda5815m_status(UINT32 tuner_id, UINT8 *lock)
{
	if (tuner_id>=rda5815m_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
	{
		*lock = 0;
		return ERR_FAILUE;
	}
	/* Because rda5815m doesn't has this flag,return 1 directly */
	*lock = 1;
	return SUCCESS;
}



