/*****************************************************************************
*	Copyright (C)2017 Ali Corporation. All Rights Reserved.
*
*	File:	This file contains C3501H DVBS2X system API and hardware 
*           open/closed operate in LLD.
*
*    Description:    
*    History:
*   Date			Athor			Version		Reason
*   ============    =========   	=======   	=================
* 1.  01/03/2017	Paladin.Ye     	Ver 1.0   	Create file for C3501H DVBS2X project
*******************************************************************************/

#include "nim_dvbs_c3501h.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_tds.c																													COPY FROM nim_dvbs_c3505_tds.c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UINT8 nim_c3501h_task_num = 0x00;
static UINT8 demod_mutex_flag = 0;
//static UINT8 g_fsm_cfg_bak = 0; // For reg00 protection

static ID tuner_i2c_sema = OSAL_INVALID_ID;//only one function can control the tuner at the same time

#ifdef AUTOSCAN_DEBUG
static UINT16 config_data;
#endif


/* Name for the demodulator, the last character must be Number for index */
char nim_c3501h_name[3][HLD_MAX_NAME_SIZE] =
{
	"NIM_S3501H_0", "NIM_S3501H_1", "NIM_S3501H_2"
};

static UINT8 c3501h_id[10] =
{
    0x47, 0x55, 0x33, 0x35, 0x30, 0x31, 0X44, 0x41, 0x54, 0x41
};

/* sub reg read/write mutex, protect for the muti thread process*/
UINT8 SUB_REG_FSM_MUTEX        = 0;
UINT8 SUB_REG_NEW_IQB_MUTEX    = 0;
UINT8 SUB_REG_TR_MUTEX         = 0;
UINT8 SUB_REG_BEQ_MUTEX        = 0;
UINT8 SUB_REG_PL_MUTEX         = 0;
UINT8 SUB_REG_DEMAP_AGC3_MUTEX = 0;
UINT8 SUB_REG_CR_MUTEX         = 0;
UINT8 SUB_REG_EQ_DD_MUTEX      = 0;
UINT8 SUB_REG_CCI_MUTEX        = 0;
UINT8 SUB_REG_DEMAP_MUTEX      = 0;
UINT8 SUB_REG_DEMAP_FLT_MUTEX  = 0;
UINT8 SUB_REG_DEMAP_MON_TSN_MUTEX = 0;
UINT8 SUB_REG_DEMAP_FLT_TSN_MUTEX = 0;
UINT8 SUB_REG_PLSN_MUTEX       = 0;
UINT8 SUB_REG_VB_MUTEX         = 0;
UINT8 SUB_REG_PECKET_MUTEX     = 0;
                                                                                                                                         
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
DWORD nim_c3501h_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
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


DWORD nim_c3501h_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
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

/*****************************************************************************                                                           
* INT32 nim_c3501h_crc8_check(void *input, INT32 len,INT32 polynomial)                                                                 
*                                                                                                                                        
*  crc8 check function                                                                                                      
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1: void *input : input data                                                                                                   
*  Parameter2: INT32 len: lenth of input data                                                                                                                 
*  Parameter3: INT32 polynomial: crc-8 polynomial                                                                                                              
*                                                                                                                                        
* Return Value: INT32: result of crc-8, suceess or fail                                                                                                       
*****************************************************************************/ 
INT32 nim_c3501h_crc8_check(void *input, INT32 len, INT32 polynomial)                                                                      
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
                                                                                                                                         
		//BBFRAME_PRINTF("%d\r\n",shift_reg);                                                                                              
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
                                                                                                                                         
                                                                                                                                         
UINT32 nim_c3501h_Log10Times100_L( UINT32 x)                                                                                              
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
* INT32 nim_c3501h_open(struct nim_device *dev)                                                                                           
* Description: NIM device open operate                                                                                                   
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1: struct nim_device *dev                                                                                                    
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/                                                           
INT32 nim_c3501h_open(struct nim_device *dev)                                                                                             
{                                                                                                                                        
	INT32 ret = ERR_FAILED;                                                                                                              
                                                                                                                                         
	if(NULL == dev)                                                                                                                      
    {                                                                                                                                    
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);                                                                        
        return ERR_NO_DEV;                                                                                                               
    }                                                                                                                                    
                                                                                                                                         
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;                                                             
	NIM_PRINTF("Enter fuction %s \r\n",__FUNCTION__);                                                                                      
	                            
	nim_c3501h_hw_open(dev);

	ret = nim_c3501h_hw_check(dev);                                                                                                       
	if (SUCCESS != ret)  
	{
		ERR_PRINTF("[%s %d]hw check error!\n", __FUNCTION__, __LINE__);
		return ret;                                                                                                                                                                                                                                                   
	}
	
	ret = nim_c3501h_hw_init(dev);                                                                                                        
	if (SUCCESS != ret) 
	{
		ERR_PRINTF("[%s %d]hw init error!\n", __FUNCTION__, __LINE__);
		return ret;                                                                                                                      
	}                                                                                                                                     
	nim_c3501h_after_reset_set_param(dev);                                                                                                
	
#ifndef FPGA_PLATFORM  
	nim_c3501h_task_init(dev);                                                                                                            
#endif                 

#ifdef CHANNEL_CHANGE_ASYNC                                                                                                              
	if (priv->flag_id == OSAL_INVALID_ID)                                                                                                
		priv->flag_id = NIM_FLAG_CREATE(0);                                                                                              
#endif                                                                                                                                   
                                                                                                                                         
	return SUCCESS;                                                                                                                      
}                                                                                                                                        
                                                                                                                                         

/*****************************************************************************                                                           
* INT32 nim_c3501h_close(struct nim_device *dev)                                                                                          
* Description: c3501h close                                                                                                               
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1: struct nim_device *dev                                                                                                    
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/                                                           
 INT32 nim_c3501h_close(struct nim_device *dev)                                                                                           
{	                                                                                                                                                                                                                                                                         
	if(NULL == dev)                                                                                                                      
    {                                                                                                                                    
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);                                                                        
        return ERR_NO_DEV;                                                                                                               
    }                                                                                                                                    
                                                                                                                                         
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;                                                             
                                                                                                                                         
	nim_c3501h_reset_fsm(dev);                                                                                  

#ifdef FPGA_PLATFORM
    if (IIC == dev->type)
    {
        nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        data &= 0xfc;
        nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
    }
#endif

    nim_c3501h_hw_close(dev);

#ifndef FPGA_PLATFORM
        NIM_MUTEX_DELETE(priv->ul_status.nim_c3501h_sema);  
#endif

#ifdef CHANNEL_CHANGE_ASYNC
    NIM_FLAG_DEL(priv->flag_id);
#endif

#ifndef FPGA_PLATFORM
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
#endif

    if (nim_c3501h_task_num > 0)
    {
        nim_c3501h_task_num--;
    }                                                                                                                           

	return SUCCESS;                                                                                                                      
}    


/*****************************************************************************                                                           
* INT32 nim_c3501h_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)                                                                  
* Description: c3501h initialization                                                                                                      
*                                                                                                                                        
* Arguments:                                                                                                                             
* struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner                                                                                            
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/     
#ifdef FPGA_PLATFORM
INT32 nim_c3501h_attach(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner)  
#else
INT32 nim_c3501h_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner)  
#endif
{                                                                                                                                        
	static unsigned char nim_dev_num = 0;                                                                                                
	INT32 channel_freq_err;                                                                                                              
#ifndef FPGA_PLATFORM                                                                                                                    
	struct nim_device *dev;                                                                                                              
#endif                                                                                                                                   
	struct nim_c3501h_private *priv_mem;                                                                                                  
	struct nim_dvbs_channel_info *channel_info_mem = NULL;                                                                              
	struct nim_dvbs_bb_header_info *bb_header_info_mem = NULL;                                                                          
	struct nim_dvbs_isid *isid_mem = NULL;  
	struct nim_dvbs_tsn *tsn_mem = NULL; 
                                                                                                                                      
	NIM_PRINTF("Enter function: %s!\r\n",__FUNCTION__);                                                                                    
                                                                                                                                         
	if (ptr_qpsk_tuner == NULL)                                                                                                          
	{                                                                                                                                    
		ERR_PRINTF("[%s %d]Tuner Configuration API structure is NULL!/n", __FUNCTION__, __LINE__);                                                                      
		return ERR_NO_DEV;                                                                                                               
	}   
	
	if (nim_dev_num > 2)                                                                                                                 
	{                                                                                                                                    
		ERR_PRINTF("[%s %d]Can not support three or more c3501h !/n", __FUNCTION__, __LINE__);                                                                           
		return ERR_NO_DEV;                                                                                                               
	}                                                                                                                                    
                                                                                                                                         
#ifndef FPGA_PLATFORM                                                                                                                    
	dev = (struct nim_device *) dev_alloc((INT8 *)nim_c3501h_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));    
	if (dev == NULL)                                                                                                                     
	{                                                                                                                                    
		ERR_PRINTF("[%s %d]Error: Alloc nim device error!\r\n", __FUNCTION__, __LINE__);                                                                                  
		return ERR_NO_MEM;                                                                                                               
	}                                                                                                                                    
#endif                                                                                                                                   
                                                                                                                                         
	priv_mem = (struct nim_c3501h_private *) comm_malloc(sizeof(struct nim_c3501h_private));                                             
	if (priv_mem == NULL)                                                                                                                
	{                                                                                                                                    
#ifndef FPGA_PLATFORM                                                                                                                    
		dev_free(dev);                                                                                                                   
#else                                                                                                                                    
		free(dev);                                                                                                                       
#endif                                                                                                                                   
		ERR_PRINTF("[%s %d]Alloc nim device prive memory error!/n", __FUNCTION__, __LINE__);
		return ERR_NO_MEM;                                                                                                               
	}                                                                                                                                    
	comm_memset((int *)priv_mem, 0, sizeof(struct nim_c3501h_private));                                                                   
	dev->priv = (void *) priv_mem;                                                                                                       
                                                                                                                                         
    // alloc dev priv channel_info                                                                                                       
	channel_info_mem = (struct nim_dvbs_channel_info *) comm_malloc(sizeof(struct nim_dvbs_channel_info));                             
	if (channel_info_mem == NULL)                                                                                                        
	{		                                                                                                                             
		comm_free(priv_mem);                                                                                                             
#ifndef FPGA_PLATFORM                                                                                                                    
		dev_free(dev);                                                                                                                   
#else                                                                                                                                    
		free(dev);                                                                                                                       
#endif                                                                                                                      
		ERR_PRINTF("[%s %d]Alloc nim device prive channel_info memory error!/n", __FUNCTION__, __LINE__);
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
#ifndef FPGA_PLATFORM                                                                                                                    
		dev_free(dev);                                                                                                                   
#else                                                                                                                                    
		free(dev);                                                                                                                       
#endif                                                                                                                     
		ERR_PRINTF("[%s %d]Alloc nim device prive bb_header_info memory error!/n", __FUNCTION__, __LINE__);
		return ERR_NO_MEM;                                                                                                               
	}                                                                                                                                    
	comm_memset(bb_header_info_mem, 0, sizeof(struct nim_dvbs_bb_header_info));                                                         
	priv_mem->bb_header_info = bb_header_info_mem;                                                                                       
                                                                                                                                         
    // alloc dev priv isid                                                                                                               
    isid_mem = (struct nim_dvbs_isid *) comm_malloc(sizeof(struct nim_dvbs_isid));                                                       
    NIM_PRINTF("comm_malloc isid addr = %x \r\n", isid_mem);                                                                               
	if (isid_mem == NULL)                                                                                                                
	{				                                                                                                                     
		comm_free(channel_info_mem);                                                                                                     
		comm_free(bb_header_info_mem);                                                                                                   
		comm_free(priv_mem);                                                                                                             
#ifndef FPGA_PLATFORM                                                                                                                    
		dev_free(dev);                                                                                                                   
#else                                                                                                                                    
		free(dev);                                                                                                                       
#endif                                                                                                                   
		ERR_PRINTF("Alloc nim device prive isid memory error!/n", __FUNCTION__, __LINE__);
		return ERR_NO_MEM;                                                                                                               
	}                                                                                                                                    
	comm_memset(isid_mem, 0, sizeof(struct nim_dvbs_isid));                                                                              
	priv_mem->isid = isid_mem;                                                                                                    

    // alloc dev priv tsn 
    tsn_mem = (struct nim_dvbs_tsn *) comm_malloc(sizeof(struct nim_dvbs_tsn));                                                       
    NIM_PRINTF("comm_malloc tsn addr = %x \r\n", tsn_mem);                                                                               
    if (tsn_mem == NULL)                                                                                                                
    {                                                                                                                                    
        comm_free(channel_info_mem);                                                                                                     
        comm_free(bb_header_info_mem);                                                                                                   
        comm_free(priv_mem);  
        comm_free(isid_mem);
#ifndef FPGA_PLATFORM                                                                                                                    
        dev_free(dev);                                                                                                                   
#else                                                                                                                                    
        free(dev);                                                                                                                       
#endif                                                                                                                   
        ERR_PRINTF("[%s %d]Alloc nim device prive tsn memory error!/n", __FUNCTION__, __LINE__);
        return ERR_NO_MEM;                                                                                                               
    }                                                                                                                                    
    comm_memset(tsn_mem, 0, sizeof(struct nim_dvbs_tsn));                                                                              
    priv_mem->tsn = tsn_mem;

	priv_mem->tsn->tsn_mutex = osal_mutex_create();//robin.gan add
	if(OSAL_INVALID_ID == priv_mem->tsn->tsn_mutex)
	{
		TSN_PRINTF("[%s %d]create priv_mem->tsn->tsn_mutex!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}
                                                                                                                                     
	//diseqc state inits                                                                                                                 
	dev->diseqc_info.diseqc_type = 0;                                                                                                    
	dev->diseqc_info.diseqc_port = 0;                                                                                                    
	dev->diseqc_info.diseqc_k22 = 0;                                                                                                     
                                                                                                                                         
	if ((ptr_qpsk_tuner->config_data.qpsk_config & C3501H_POLAR_REVERT) == C3501H_POLAR_REVERT) //bit4: polarity revert.                   
		dev->diseqc_info.diseqc_polar = LNB_POL_V;                                                                                       
	else //default usage, not revert.                                                                                                    
		dev->diseqc_info.diseqc_polar = LNB_POL_H;                                                                                       
                                                                                                                                         
	dev->diseqc_typex = 0;                                                                                                               
	dev->diseqc_portx = 0;                                                                                                               
                                                                                                                                         
	/* Function point init */                                                                                                            
	priv_mem->t_param.t_reg_setting_switch = 0x0f;
	dev->base_addr = ptr_qpsk_tuner->ext_dm_config.i2c_base_addr;
#ifndef FPGA_PLATFORM
	dev->init = nim_c3501h_attach;    
#else
	dev->qpsk_init = nim_c3501h_attach; 
#endif
	dev->open = nim_c3501h_open;                                                                                                          
	dev->stop = nim_c3501h_close;                                                                                                         
	dev->do_ioctl = nim_c3501h_ioctl;                                                                                                     
	dev->do_ioctl_ext = nim_c3501h_ioctl_ext;     
    #ifndef FC_SEARCH_RANGE_LIMITED
	dev->get_lock = nim_c3501h_get_lock; 
    #else
    dev->get_lock = nim_c3501h_get_lock_check_limited;
    #endif
	dev->get_freq = nim_c3501h_get_cur_freq;                                                                         
	dev->get_fec= nim_c3501h_reg_get_code_rate;                                                                                          
	dev->get_snr= nim_c3501h_get_snr;  
    dev->set_polar = nim_c3501h_set_polar;                                                                                                
    dev->set_12v = nim_c3501h_set_12v;                                                                                                    
    dev->channel_search = (void *)nim_c3501h_channel_search;
    dev->di_seq_c_operate = nim_c3501h_diseqc_operate;                                                                                    
    dev->di_seq_c2x_operate = nim_c3501h_diseqc2x_operate;
    dev->get_sym = nim_c3501h_get_symbol_rate;
    dev->get_agc= nim_c3501h_get_agc;       
    dev->get_ber= nim_c3501h_get_ber;                                                                                                    
    dev->get_fft_result = nim_c3501h_get_fft_result;        
#ifdef FPGA_PLATFORM
	dev->get_snr_db = nim_c3501h_get_snr_db;
	dev->get_snir_fisher = nim_c3501h_get_snr_db;
	dev->get_PER = nim_c3501h_get_per;
	dev->get_FER = nim_c3501h_get_fer;
	dev->get_ldpc_iter = nim_c3501h_get_ldpc_iter_cnt;
	dev->get_bchover = nim_c3501h_get_bchover;
	dev->get_modcod = nim_c3501h_get_modcod;
	dev->ger_workmod = nim_c3501h_reg_get_work_mode;
	dev->get_lock_reg = nim_c3501h_get_lock_reg;
	dev->qpsk_channel_change = nim_c3501h_channel_change;
	dev->qpsk_channel_change_task = nim_c3501h_channel_change_task;
	dev->qpsk_plsn_search_task = nim_c3501h_plsn_task;
	dev->set_sub_work_mode = nim_c3501h_set_sub_work_mode;
	dev->set_isi_work_mode = NULL;    
    dev->get_ver_infor = NULL;//nim_c3501h_get_ver_infor;                                                                                   
	//dev->get_iter = nim_s3503_get_LDPC_iter_cnt ; 
	//dev->get_snr_seen = nim_s3503_get_snr_seen ; // add by hongyu
	dev->sub_read_fsm      = nim_c3501h_sub_read_fsm;
    dev->sub_write_fsm     = nim_c3501h_sub_write_fsm;
    dev->sub_read_new_iqb  = nim_c3501h_sub_read_new_iqb;
    dev->sub_write_new_iqb = nim_c3501h_sub_write_new_iqb;
    dev->sub_read_cci      = nim_c3501h_sub_read_cci;
    dev->sub_write_cci     = nim_c3501h_sub_write_cci;
    dev->sub_read_tr       = nim_c3501h_sub_read_tr;
    dev->sub_write_tr      = nim_c3501h_sub_write_tr;
    dev->sub_read_beq      = nim_c3501h_sub_read_beq;
    dev->sub_write_beq     = nim_c3501h_sub_write_beq;
    dev->sub_read_pl       = nim_c3501h_sub_read_pl;
    dev->sub_write_pl      = nim_c3501h_sub_write_pl;
    dev->sub_read_demap_agc3  = nim_c3501h_sub_read_demap_agc3;
    dev->sub_write_demap_agc3 = nim_c3501h_sub_write_demap_agc3;
    dev->sub_read_cr       = nim_c3501h_sub_read_cr;
    dev->sub_write_cr      = nim_c3501h_sub_write_cr;
    dev->sub_read_eq_dd    = nim_c3501h_sub_read_eq_dd;
    dev->sub_write_eq_dd   = nim_c3501h_sub_write_eq_dd;
    dev->sub_read_demap    = nim_c3501h_sub_read_demap;
    dev->sub_write_demap   = nim_c3501h_sub_write_demap;
    dev->sub_read_demap_flt     = nim_c3501h_sub_read_demap_flt;
    dev->sub_write_demap_flt    = nim_c3501h_sub_write_demap_flt;
    dev->sub_read_demap_mon_tsn = nim_c3501h_sub_read_demap_mon_tsn;
    dev->sub_read_demap_flt_tsn = nim_c3501h_sub_read_demap_flt_tsn;
    dev->sub_write_demap_flt_tsn = nim_c3501h_sub_write_demap_flt_tsn;
    dev->sub_read_plsn     = nim_c3501h_sub_read_plsn;
    dev->sub_write_plsn    = nim_c3501h_sub_write_plsn;
    dev->sub_read_vb       = nim_c3501h_sub_read_vb;
    dev->sub_write_vb      = nim_c3501h_sub_write_vb;
    dev->sub_read_packet   = nim_c3501h_sub_read_packet;
    dev->sub_write_packet  = nim_c3501h_sub_write_packet;
	dev->get_isid_debug    = get_isid_debug;
#endif  

#if 1//just for debug i2c write
    dev->reg_read       = nim_reg_read;
    dev->reg_write      = nim_reg_write;
#endif
	/* tuner configuration function */                                                                                                   
	priv_mem->nim_tuner_init = ptr_qpsk_tuner->nim_tuner_init;                                                                           
	priv_mem->nim_tuner_control = ptr_qpsk_tuner->nim_tuner_control;                                                                     
	priv_mem->nim_tuner_status = ptr_qpsk_tuner->nim_tuner_status;                                                                       
	                                                                                                                                     
    //add by dennis on 20140906 for c3031b Tuner         
#ifndef FPGA_PLATFORM
	priv_mem->nim_tuner_command = ptr_qpsk_tuner->nim_tuner_command;                                                                     
	priv_mem->nim_tuner_gain = ptr_qpsk_tuner->nim_tuner_gain;     
#endif
                                                                                                                                         
	priv_mem->i2c_type_id = ptr_qpsk_tuner->tuner_config.i2c_type_id;                                                                    
	comm_memcpy(&priv_mem->tuner_config_data,&ptr_qpsk_tuner->config_data,sizeof(ptr_qpsk_tuner->config_data));                          
	priv_mem->ext_dm_config.i2c_type_id = ptr_qpsk_tuner->ext_dm_config.i2c_type_id;                                                     
	priv_mem->ext_dm_config.i2c_base_addr = ptr_qpsk_tuner->ext_dm_config.i2c_base_addr;                                                 
                                                                                                                                         
	priv_mem->search_type = NIM_OPTR_CHL_CHANGE;                                                                                         
	priv_mem->debug_flag = 0;                                                                                                   
                                                                                                                                         
	priv_mem->ul_status.m_enable_dvbs2_hbcd_mode = 0;                                                                                    
	priv_mem->ul_status.m_dvbs2_hbcd_enable_value = 0x7f;                                                                                
	priv_mem->ul_status.nim_c3501h_sema = OSAL_INVALID_ID;                                                                                
	priv_mem->ul_status.c3501h_autoscan_stop_flag = 0;                                                                                    
	priv_mem->ul_status.c3501h_chanscan_stop_flag = 0;                                                                                    
	priv_mem->ul_status.old_ber = 0;                                                                                                     
	priv_mem->ul_status.old_per = 0;                                                                                                     
	priv_mem->ul_status.m_hw_timeout_thr = 0;                                                                                            
	priv_mem->ul_status.old_ldpc_ite_num = 0;                                                                                            
	priv_mem->ul_status.c_rs = 0;                                                                                                        
	priv_mem->ul_status.phase_err_check_status = 0;                                                                                      
	priv_mem->ul_status.lock_status = 3;  
	priv_mem->ul_status.m_c3501h_type = 0x00;                                                                                             
	priv_mem->ul_status.m_setting_freq = 123;                                                                                            
	priv_mem->ul_status.m_err_cnts = 0x00;                                                                                               
	priv_mem->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;                                                                             
	priv_mem->tsk_status.m_task_id = OSAL_INVALID_ID;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	priv_mem->t_param.t_i2c_err_flag = 0x00;                                                                                             
	priv_mem->flag_id = OSAL_INVALID_ID;                                                                                                 
	priv_mem->blscan_mode = NIM_SCAN_SLOW; // this mode may abandoned    
	priv_mem->plsn_task_id = OSAL_INVALID_ID;    
            
#ifndef FPGA_PLATFORM 
	/* Add this device to queue */                                                                                                       
	if (dev_register(dev) != SUCCESS)                                                                                                    
	{                                                                                                                                    
		ERR_PRINTF("[%s %d]Error: Register nim device error!\r\n", __FUNCTION__, __LINE__);
		comm_free(isid_mem);                                                                                                             
		comm_free(channel_info_mem);                                                                                                     
		comm_free(bb_header_info_mem);                                                                                                   
		comm_free((int*)priv_mem);		                                                                                                                                                                                                       
		dev_free(dev);                                                                                                                                                                                                                         
		return ERR_NO_DEV;                                                                                                               
	}                                                                                                                 
#endif   

	nim_dev_num++;       

#ifndef FPGA_PLATFORM 
	priv_mem->ul_status.nim_c3501h_sema = NIM_MUTEX_CREATE(1);                                                                            
                                                                                                                                         
	priv_mem->c3501h_mutex = osal_mutex_create();                                                                                         
	if(priv_mem->c3501h_mutex == OSAL_INVALID_ID)                                                                                         
	{                                                                                                                                    
		ERR_PRINTF("[%s %d]Error: Register nim device error!\r\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;                                                                                                               
	}                                                                                                                                    
#endif  

	priv_mem->plsn.plsn_mutex = osal_mutex_create();//robin.gan add
	if(OSAL_INVALID_ID == priv_mem->plsn.plsn_mutex)
	{
		ERR_PRINTF("create priv_mem->OSAL_INVALID_ID error!\n");
		return ERR_FAILUE;
	}	
	
    //begin:only one fucntion can control the tuner at the same time
    tuner_i2c_sema = osal_semaphore_create(1);
    if (OSAL_INVALID_ID == tuner_i2c_sema)
    {
        ERR_PRINTF("[%s %d]create tuner i2c sema failed.\n", __FUNCTION__, __LINE__);
        return ERR_FAILUE;
    }
    //end

#ifdef I2C_THROUGH_ALWAYS_OPEN
    UINT8 data = 0;
    data = 0x33;   // Open THROUGH_EN, IIC_PASS_THR
    nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
#endif

    if (nim_c3501h_i2c_through_open(dev))
    {
        ERR_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
        return C3501H_ERR_I2C_NO_ACK;
    }   
    NIM_MUTEX_ENTER(priv_mem);

	// Initial the QPSK Tuner                                                                                                            
	if (priv_mem->nim_tuner_init != NULL)                                                                                                
	{                
		NIM_PRINTF(" Initial the Tuner \r\n");                                                                                             
		if (((struct nim_c3501h_private *) dev->priv)->nim_tuner_init(&priv_mem->tuner_index, &(ptr_qpsk_tuner->tuner_config)) != SUCCESS)
		{   
			NIM_MUTEX_LEAVE(priv_mem)                                                                                                                             
			NIM_PRINTF("Error: Init Tuner Failure!\r\n");   
            if (nim_c3501h_i2c_through_close(dev))
            {
                 NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
                 return C3501H_ERR_I2C_NO_ACK;
            }
			return ERR_NO_DEV;                                                                                                           
		}        
		NIM_MUTEX_LEAVE(priv_mem)
#ifndef FPGA_PLATFORM
        // get tuner type                                                                                                                
		//if(SUCCESS == call_tuner_command(dev, NIM_TUNER_C3031B_ID, &channel_freq_err))  
		if (priv_mem->nim_tuner_command)
	    {
            if (SUCCESS == priv_mem->nim_tuner_command(priv_mem->tuner_index, NIM_TUNER_C3031B_ID, &channel_freq_err))    
	    	{
	    		priv_mem->tuner_type = IS_M3031;
	    	}
			else
			{
				priv_mem->tuner_type = NOT_M3031;
			}
		}			                                                                                             
		else   
		{
			NIM_PRINTF("[%s %d]priv_mem->nim_tuner_command is null!\n", __FUNCTION__, __LINE__);
			priv_mem->tuner_type = NOT_M3031;     
		}
		NIM_PRINTF("[%s %d]priv_mem->tuner_type=%d\n", __FUNCTION__, __LINE__, priv_mem->tuner_type);
#else
		priv_mem->tuner_type = NOT_M3031; 
#endif  
	}     
	else
	{
		NIM_PRINTF("[%s %d]priv_mem->nim_tuner_init == NULL!\n", __FUNCTION__, __LINE__);
		NIM_MUTEX_LEAVE(priv_mem);
		return ERR_NO_DEV; 
	}
    

	if (nim_c3501h_i2c_through_close(dev))
    {
        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
        return C3501H_ERR_I2C_NO_ACK;
    }

	nim_c3501h_set_ext_lnb(dev, ptr_qpsk_tuner);                                                                                          
	nim_c3501h_reg_get_chip_type(dev);                                                                                                    
	ptr_qpsk_tuner->device_type = priv_mem->ul_status.m_c3501h_type;  

    nim_c3501h_reg_get_strap_bond_info(dev); 

    // For printf a initial register value
#ifdef DEBUG_REGISTER
    nim_c3501h_mon_reg(dev);
#endif
  
	return SUCCESS;                                                                                                                      
}                                                                                                                                        

                                                                                                                                         
extern INT32 c3501h_real_freq[];                                                                                                          
extern INT32 c3501h_real_sym_rate[];                                                                                                      
extern INT32 c3501h_real_tp_number;                                                                                                       


/*****************************************************************************                                                           
* INT32 nim_c3501h_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)                                                                 
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
INT32 nim_c3501h_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{                                                                                                                                        
	INT32 cr_num = 0;                                                                                                                    
	UINT32 cur_freq = 0;                                                                                                                  
	INT32 step = 0;                                                                                                                      
	struct nim_dvbs_isid *p_isid = NULL;                                                                                                 
	struct nim_dvbs_data_cap *p_data_cap = NULL;
 	UINT32 plsn_value = 0;
	UINT8 pin_num;
	UINT8 value;

//	NIM_PRINTF("Enter function: %s!\r\n",__FUNCTION__);                                                                                    
                                                                                                                                         
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;                                                             
	switch (cmd)                                                                                                                         
	{             
#if 1
	case NIM_DRIVER_READ_TUNER_STATUS:                                                                                                   
		return nim_c3501h_get_tuner_lock(dev, (UINT8 *) param);                                                                           
	case NIM_DRIVER_READ_QPSK_STATUS:    
        #ifndef FC_SEARCH_RANGE_LIMITED
		return nim_c3501h_get_lock(dev, (UINT8 *) param); 
        #else
        return nim_c3501h_get_lock_check_limited(dev, (UINT8 *) param); 
        #endif
	case NIM_DRIVER_READ_FEC_STATUS:                                                                                                     
		break;                                                                                                                           
	case NIM_DRIVER_READ_QPSK_BER:                                                                                                       
		return nim_c3501h_get_ber(dev, (UINT32 *) param);                                                                                 
	case NIM_DRIVER_READ_VIT_BER:                                                                                                        
		break;                                                                                                                           
	case NIM_DRIVER_READ_RSUB:   
        #ifdef DEBUG_IN_TASK
            param = &(priv->channel_info->per);
        #else
            nim_c3501h_reg_get_work_mode(dev, &(priv->channel_info->work_mode));
            if (1 == priv->channel_info->work_mode)
                nim_c3501h_get_fer(dev, (UINT32 *) param); 
            else
                nim_c3501h_get_per(dev, (UINT32 *) param); 
        #endif
        break;
	case NIM_DRIVER_STOP_ATUOSCAN:                                                                                                       
		priv->ul_status.c3501h_autoscan_stop_flag = param;                                                                                
		break;                                                                                                                           
	case NIM_DRIVER_GET_CR_NUM:                                                                                                          
		cr_num = (INT32) nim_c3501h_get_cr_num(dev);                                                                                      
		return cr_num;                                                                                                                   
	case NIM_DRIVER_GET_CUR_FREQ:                                                                                                        
		switch (param)                                                                                                                   
		{                                                                                                                                
    		case NIM_FREQ_RETURN_SET:                                                                                                        
    			return priv->ul_status.m_setting_freq;                                                                                       
    		case NIM_FREQ_RETURN_REAL:                                                                                                       
    		default:                                                                                                                         
    			nim_c3501h_get_cur_freq(dev, &cur_freq);                                                                                      
    			return cur_freq;                                                                                                             
		}                                                                                                                                
	case NIM_DRIVER_FFT_PARA:                                                                                                            
        return SUCCESS;
        //nim_c3501h_fft_set_para(dev);                                                                                                     
		break;                                                                                                                           
	case NIM_DRIVER_FFT:   
        return SUCCESS;
		//return nim_c3501h_fft(dev, param);                                                                                                
	case NIM_FFT_JUMP_STEP:                                                                                                              
		step = 0;                                                                                                                        
		nim_c3501h_get_tune_freq(dev, &step);                                                                                             
		return step;                                                                                                                     
	case NIM_DRIVER_SET_RESET_CALLBACK:                                                                                                  
		priv->ul_status.m_pfn_reset_c3501h = (pfn_nim_reset_callback) param;                                                              
		NIM_PRINTF("[%s %d]priv->ul_status.m_pfn_reset_c3501h=0x%x\n", __FUNCTION__, __LINE__, priv->ul_status.m_pfn_reset_c3501h);
		break;                                                                                                                           
	case NIM_DRIVER_RESET_PRE_CHCHG:                                                                                                     
		if (priv->ul_status.m_pfn_reset_c3501h)                                                                                           
		{                                                                                                                                
			priv->ul_status.m_pfn_reset_c3501h((priv->tuner_index + 1) << 16);                                                            
		}                                                                                                                                
		break;                                                                                                                           
	case NIM_DRIVER_ENABLE_DVBS2_HBCD:                                                                                                   
//		priv->ul_status.m_enable_dvbs2_hbcd_mode = param;                                                                                
//		nim_c3501h_set_hbcd_timeout(dev, NIM_OPTR_IOCTL);                                                                                 
		break;                                                                                                                           
	case NIM_DRIVER_STOP_CHANSCAN:                                                                                                       
		priv->ul_status.c3501h_chanscan_stop_flag = param;                                                                                
		break;                                                                                                                           
	case NIM_DRIVER_SET_BLSCAN_MODE:                                                                                                     
		if(0 == param)                                                                                                                   
		   priv->blscan_mode = NIM_SCAN_FAST;                                                                                            
		else if(1 == param)                                                                                                              
		   priv->blscan_mode = NIM_SCAN_SLOW;                                                                                            
		break;                                                                                                                           
                                                                                                                                         
    case NIM_DRIVER_SET_POLAR:                                                                                                           
        return nim_c3501h_set_polar(dev, (UINT8)param);                                                                                   
        break;                                                           
    case NIM_DRIVER_SET_12V:                                                                                                             
        return nim_c3501h_set_12v(dev, (UINT8)param);                                                                                     
        break;
    case NIM_DRIVER_GET_SYM:
        return nim_c3501h_get_symbol_rate(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_GET_BER:
        return nim_c3501h_get_ber(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_GET_AGC:
        return nim_c3501h_get_agc(dev, (UINT8 *)param);
        break;
	case NIM_TURNER_SET_STANDBY:

		if (nim_c3501h_i2c_through_open(dev))
	    {
	        NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
	        return C3501H_ERR_I2C_NO_ACK;
	    }
		
		if (priv->nim_tuner_command != NULL)                                                                                             
		{                                                                                                                                
			priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_SET_STANDBY_CMD, 0);	                                                 
		}   

		if (nim_c3501h_i2c_through_close(dev))
	    {
	        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
	        return C3501H_ERR_I2C_NO_ACK;
	    }
		break;                                                                                                                           
                                                                                                                                         
	case NIM_ACM_MODE_GET_TP_INFO:                                                                                                       
		nim_c3501h_get_tp_info(dev);	                                                                                                     
		break;                                                                                                                           
                                                                                                                                         
	case NIM_ACM_MODE_SET_ISID:                                                                                                          
		p_isid = (struct nim_dvbs_isid *)param;                                                                                          
		nim_c3501h_set_isid(dev, p_isid);                                                                                                 
		break;                                                                                                                           
		                                                                                                                                 
	case NIM_ACM_MODE_GET_ISID:		                                                                                                     
		//NIM_PRINTF("[%s %d]NIM_ACM_MODE_GET_ISID\r\n", __FUNCTION__, __LINE__);                                                          
		if (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)                                        
		{                                                                                                                                
			NIM_PRINTF("[%s %d]don't allow get isid!\r\n",__FUNCTION__, __LINE__);                                                         
			priv->isid->get_finish_flag = 0;			                                                                                 
		}                                                                                                                                
		comm_memcpy((struct nim_dvbs_isid *)param, priv->isid, sizeof(struct nim_dvbs_isid));                                            
		break;                                                                                                                           
		                                                                                                                                 
	case NIM_ACM_MODE_GET_BBHEADER_INFO:		                                                                                         
		comm_memcpy((struct nim_dvbs_bb_header_info *)param, priv->bb_header_info, sizeof(struct nim_dvbs_bb_header_info));            
		break;	                                                                                                                         
                                                                                                   
#ifdef ACM_RECORD	                                                                                                                 
	 // For record stream, in ts mode we use typical way and in gs mode we use TSO_BB way to capture 	                                 
	 case NIM_TS_GS_AUTO_CAP:         
		NIM_PRINTF("~~~~~~~~Enter C3501H NIM_auto_cap_mod~~~~~~~~\n");
	 	return nim_c3501h_ts_gs_auto_cap_mod(dev);	                                                                                                                                                                                                                   
		break;                                                                                                                           
	 case NIM_CAP_STOP:	 	                                                                                                             
		return nim_c3501h_rec_stop(dev);                                                                                                  
		break;			                                                                                                                                                                                                                                              
                                                                                                                         
#endif		                                                                                                                             
                                                                                                                                         
	case NIM_DRIVER_GET_MER:                                                                                                             
		//return nim_c3501h_get_mer(dev, (UINT32 *)param);       
        return nim_c3501h_get_snr_db(dev, (INT16 *)param);   
		break;                                                                                                                           
#endif

#ifdef FPGA_PLATFORM
	case NIM_DRIVER_WAIT_LOCK:
        //return nim_c3501h_waiting_channel_lock(dev,0,(UINT32)param);
		return nim_c3501h_waiting_channel_lock(dev, 0, (UINT32)param, 0, 0);
        break;

    case NIM_DRIVER_GET_LOCKSTATE:                                                                                                             
        return nim_c3501h_get_lock_reg(dev, (UINT16 *)param);                                                                                   
        break; 
#endif

    case NIM_DRIVER_CAP_DATA:                                                                                                            
        p_data_cap = (struct nim_dvbs_data_cap *)param;                                                                                  
        return nim_c3501h_cap_data_entity(dev, (UINT32 *)p_data_cap->dram_base_addr, p_data_cap->cap_len, p_data_cap->cap_src);   
        break;   
                                                                                                                                  
    case NIM_DRIVER_SET_PLSN:
        return nim_c3501h_set_plsn(dev);
        break;
		
	case NIM_DRIVER_GET_PLSN:
		*(UINT32 *)param = priv->plsn.plsn_report;
		PLSN_PRINTF("[%s %d]*(UINT32 *)param=%d, priv->plsn.plsn_report=%d\n", __FUNCTION__, __LINE__, *(UINT32 *)param, priv->plsn.plsn_report);
		break;
	
	case NIM_DRIVER_PLSN_GOLD_TO_ROOT:
		plsn_value = nim_c3501h_plsn_gold_to_root(*(UINT32 *)param);
		*(UINT32 *)param = plsn_value;
		break;
		
	case NIM_DRIVER_PLSN_ROOT_TO_GOLD:
		plsn_value = nim_c3501h_plsn_root_to_gold(*(UINT32 *)param);
		*(UINT32 *)param = plsn_value;
		break;     

	case NIM_DRIVER_GENERATE_TABLE:
		//nim_c3505_start_generate(dev, (struct ali_plsn_address *)param);
		break;
		
	case NIM_DRIVER_RELEASE_TABLE:
		//nim_c3505_release_table(dev);
		break;

	case NIM_DRIVER_GET_PLSN_FINISH_FLAG:
		*(UINT8 *)param = priv->plsn.plsn_finish_flag;
		PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d\n", __FUNCTION__, __LINE__, *(UINT8 *)param);		
		break;
		
	case NIM_DRIVER_SET_TSN:
		comm_memcpy(priv->tsn, (struct nim_dvbs_tsn *)param, sizeof(struct nim_dvbs_tsn));
		nim_c3501h_set_tsn(dev, priv->tsn);
		break;

	case NIM_DRIVER_GET_TSN:
		nim_c3501h_get_tsn(dev, priv->tsn);
		comm_memcpy((struct nim_dvbs_tsn *)param, priv->tsn, sizeof(struct nim_dvbs_tsn));
		break;
		
	case NIM_DRIVER_GET_WIDEBAND_MODE:
		osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);
		*(UINT8 *)param = priv->tsn->is_wideband;
		TSN_PRINTF("[%s %d]is_wideband=%d\n", __FUNCTION__, __LINE__, *(UINT8 *)param);
		osal_mutex_unlock(priv->tsn->tsn_mutex);
		break;
		
    case NIM_DRIVER_GET_TSN_FINISH_FLAG:
		osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);
		*(UINT8 *)param = priv->tsn->get_finish_flag;
		TSN_PRINTF("[%s %d]tsn get_finish_flag=%d\n", __FUNCTION__, __LINE__, *(UINT8 *)param);
		osal_mutex_unlock(priv->tsn->tsn_mutex);
		break;
		
	case NIM_LNB_SET_POWER_ONOFF:
		if (priv->ext_lnb_control != NULL)
		{
			NIM_PRINTF("ext_lnb_control() isn't NULL!\n");
			if(priv->lnb_gpio_select)
			{
				NIM_PRINTF("[%s %d]set 3501h_pin_%d lnb_cut_pin_enable\n", __FUNCTION__, __LINE__, priv->lnb_gpio_num);
				nim_c3501h_gpio_output(dev, priv->lnb_gpio_num, 1);//keep lnb_cut always enable on lnb i2c control mode
			}
			else
			{
				NIM_PRINTF("[%s %d]set main_chip_pin_%d lnb_cut_pin_enable\n", __FUNCTION__, __LINE__, priv->lnb_gpio_num);
				HAL_GPIO_BIT_SET(priv->lnb_gpio_num, 1);//keep lnb_cut always enable on lnb i2c control mode
			}

			return priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_POWER_ONOFF, param);
		}
		//gpio control mode
		if(priv->lnb_gpio_select)
		{
			NIM_PRINTF("[%s %d]set 3501h_pin_%d lnb_cut_pin %d\n", __FUNCTION__, __LINE__, priv->lnb_gpio_num, *(UINT8 *)param);
			nim_c3501h_gpio_output(dev, priv->lnb_gpio_num, *(UINT8 *)param);
		}
		else
		{
			NIM_PRINTF("[%s %d]set main_chip_pin_%d lnb_cut_pin %d\n", __FUNCTION__, __LINE__, priv->lnb_gpio_num, *(UINT8 *)param);
			HAL_GPIO_BIT_SET(priv->lnb_gpio_num, *(UINT8 *)param);
		}
		break;	

	case NIM_DRIVER_GPIO_OUTPUT:
		pin_num = ((struct DEMOD_GPIO_STRUCT *)(param))->position;
		value = ((struct DEMOD_GPIO_STRUCT *)(param))->value;
		NIM_PRINTF("[%s %d]pin_num=%d, value=%d\n", __FUNCTION__, __LINE__, pin_num, value);
		nim_c3501h_gpio_output(dev, pin_num, value);
		break;
		
	case NIM_DRIVER_ENABLE_GPIO_INPUT:
		pin_num = ((struct DEMOD_GPIO_STRUCT *)(param))->position;
		nim_c3501h_enable_gpio_input(dev, pin_num);
		break;
		
	case NIM_DRIVER_GET_GPIO_INPUT:
		pin_num = ((struct DEMOD_GPIO_STRUCT *)(param))->position;
		nim_c3501h_get_gpio_input(dev, pin_num, &value);
		((struct DEMOD_GPIO_STRUCT *)(param))->value = value;
		break;
		
	default:                                                                                                                             
		break;                                                                                                                           
	}                                                                                                                                    
	return SUCCESS;                                                                                                                      
}

                                                                                                                                    
/*****************************************************************************                                                           
*INT32 nim_c3501h_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)                                                          
* 	device input/output operation                                                                                                        
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1: struct nim_device *dev                                                                                                    
*  Parameter2: INT32 cmd                                                                                                                 
*  Parameter3: void *param_list                                                                                                          
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/                                                           
INT32 nim_c3501h_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)                                                           
{                                                                                                                                        
    nim_get_fft_result_t *fft_para;                                                                                                      
    nim_diseqc_operate_para_t *diseqc_para;                                                                                              
	struct NIM_AUTO_SCAN *p_get_spectrum = NULL;                                                                                         
	struct NIM_AUTO_SCAN*p_get_valid_freq = NULL;                                                                                        
	struct nim_get_symbol *p_sym = NULL;                                                                                                 
	struct nim_c3501h_private *priv = NULL;                                                                                               
	INT32 result = SUCCESS;                                                                                                                
	                                                                                                                                     
    if(NULL == dev)                                                                                                                      
    {                                                                                                                                    
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);                                                                        
        return ERR_NO_DEV;                                                                                                               
    }                                                                                                                                    
                                                                                                                                         
	priv = (struct nim_c3501h_private *) dev->priv;                                                                                       
                                                                                                                                         
	NIM_PRINTF("Enter function: %s!\r\n",__FUNCTION__);                                                                                    
                                                                                                                                         
	switch (cmd)                                                                                                                         
	{                                                                                                                                    
	case NIM_DRIVER_AUTO_SCAN:                                                                                                           
        demod_mutex_flag = 1;
		priv->autoscan_control = NORMAL_MODE;
		osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
		result = nim_c3501h_autoscan(dev, (struct NIM_AUTO_SCAN *) (param_list));                                                           
		osal_mutex_unlock(priv->scan_mutex);
        demod_mutex_flag = 0;
        return result; 
	case NIM_DRIVER_CHANNEL_CHANGE:                                                                                                      
        demod_mutex_flag = 1;
		NIM_PRINTF(">>>[JIE] NIM_DRIVER_CHANNEL_CHANGE\n");
		osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
		result = nim_c3501h_channel_change(dev, (struct NIM_CHANNEL_CHANGE *) (param_list));                                            
		osal_mutex_unlock(priv->scan_mutex);
        #ifndef CHANNEL_CHANGE_ASYNC
            demod_mutex_flag = 0;
        #endif
        return result;
                                                                                                                              
	case NIM_DRIVER_CHANNEL_SEARCH:
		/* Do Channel Search */                                                                                                          
		break;                                                                                                                           
	case NIM_DRIVER_GET_ID:                                                                                                              
		*((UINT32 *) param_list) = priv->ul_status.m_c3501h_type;                                                                         
		break;                                                                                                                           
    case NIM_DRIVER_GET_FFT_RESULT:                                                                                                      
        fft_para = (nim_get_fft_result_t *)(param_list);                                                                                 
        return nim_c3501h_get_fft_result(dev, fft_para->freq, fft_para->start_addr);                                                      
        break;                                                                                                                           
    case NIM_DRIVER_DISEQC_OPERATION:                                                                                                    
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);                                                                         
        return nim_c3501h_diseqc_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt);                                     
        break;                                                                                                                           
    case NIM_DRIVER_DISEQC2X_OPERATION:                                                                                                  
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);                                                                         
        return nim_c3501h_diseqc2x_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt,\
            diseqc_para->rt_value, diseqc_para->rt_cnt);                                                                                 
        break;                                                                                                                           
                                                                                                                                         
	//20160811                                                                                                                           
	case NIM_DRIVER_GET_SPECTRUM:		                                                                                                 
		priv->autoscan_control = GET_SPECTRUM_ONLY;                                                                                      
		p_get_spectrum = (struct NIM_AUTO_SCAN *)(param_list);                                                                           
		result = nim_c3501h_autoscan(dev, p_get_spectrum);                                                                                
		p_get_spectrum->spectrum.spectrum_length = chlspec_num;		                                                                     
        p_get_spectrum->spectrum.p_channel_spectrum = channel_spectrum;		                                                             
		return result;                                                                                                                   
                                                                                                                                         
	case NIM_DRIVER_GET_VALID_FREQ:                                                                                                      
		priv->autoscan_control = GET_TP_ONLY;                                                                                            
		p_get_valid_freq = (struct NIM_AUTO_SCAN *)(param_list);                                                                         
		result = nim_c3501h_autoscan(dev, p_get_valid_freq);                                                                              
		p_get_valid_freq->valid_freq_sym.real_freq_sym = c3501h_real_tp_number;                                                           
		p_get_valid_freq->valid_freq_sym.p_frequency = c3501h_real_freq;                                                                  
		p_get_valid_freq->valid_freq_sym.p_symbol_rate = c3501h_real_sym_rate;                                                            
		return result;                                                                                                                   
		                                                                                                                                 
	case NIM_DRIVER_GET_SYMBOL:                                                                                                          
		p_sym = (struct nim_get_symbol *)param_list;                                                                                     
		if ((p_sym->p_cr_out_i != NULL) && (p_sym->p_cr_out_q != NULL))                                                                  
		{                                                                                                                                
			nim_c3501h_get_symbol(dev, p_sym);                                                                                            
		}                                                                                                                                
		break;                                                                                                                           
                                                                                                                                         
	case NIM_DRIVER_GET_AGC_DBM:                                                                                                         
		nim_c3501h_get_agc_dbm(dev, (INT8 *)param_list);                                                                                 
		break;                                                                                                                           
	//20160811     

	
	case NIM_DRIVER_GET_RF_LEVEL:
		nim_c3501h_get_agc_dbm(dev, (INT8 *)param_list);  
		break;

	case NIM_DRIVER_GET_CN_VALUE:
		nim_c3501h_get_snr_db(dev, (INT16 *)param_list); 
		break;
	                                                                                                                                     
	default:                                                                                                                             
		break;                                                                                                                           
	}                                                                                                                                    
                                                                                                                                         
	return SUCCESS;                                                                                                                      
}                                                                                                                                        

#ifndef FPGA_PLATFPRM                                                                                                                                     
/*****************************************************************************                                                           
*  INT32 nim_c3501h_task_init(struct nim_device *dev)                                                                                     
* Get bit error ratio                                                                                                                    
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1: struct nim_device *dev                                                                                                    
*  Parameter2: UINT16* RsUbc                                                                                                             
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/                                                           
INT32 nim_c3501h_task_init(struct nim_device *dev)                                                                                        
{                                                                                                                                        
    UINT8 nim_device[3][3] = \
    {
        {'a', 'M', '0'}, {'b', 'M', '1'}, {'c', 'M', '2'}
    };                                                                                                                                   
                                                                                                                                         
    UINT8 plsn_task_name[3] = {'P', 'L', 'S'};
    T_CTSK nim_task_praram;                                                                                                                                                                                                                

    NIM_PRINTF("[%s %d]\n", __FUNCTION__, __LINE__);
                                                                                                                                  
    if(NULL == dev)                                                                                                                      
    {                                                                                                                                    
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);                                                                        
        return ERR_NO_DEV;                                                                                                               
    }                                                                                                                                    
                                                                                                                                         
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;                                                             
                                                                                                                                         
    if(NULL == priv)
    {
        ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }
    if (nim_c3501h_task_num > 1)                                                                                                                
    {
        NIM_PRINTF("[%s %d]nim_c3501h_task_num=%d, task has inited!\n", __FUNCTION__, __LINE__, nim_c3501h_task_num);
        return SUCCESS;
    }
    if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)  // need initial m_task_id value in attach function
    {
        nim_task_praram.task = nim_c3501h_task;                                                                   
        nim_task_praram.name[0] = nim_device[nim_c3501h_task_num][0];
        nim_task_praram.name[1] = nim_device[nim_c3501h_task_num][1];
        nim_task_praram.name[2] = nim_device[nim_c3501h_task_num][2];
        nim_task_praram.stksz = 0xc00;
        nim_task_praram.itskpri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;
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

    if (OSAL_INVALID_ID == priv->plsn_task_id)  // need initial plsn_task_id value in attach function
    {
        nim_task_praram.task = nim_c3501h_plsn_task;
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
    nim_c3501h_task_num++;                                                                                                                      
                                                                                                                                         
    return SUCCESS;                                                                                                                      
} 
#endif                                                                                                                                                                                                                                                                                 

/*****************************************************************************                                                           
*  void nim_c3501h_task(UINT32 param1, UINT32 param2)                                                                                     
*  Task of nim driver,  do some monitor or config task in real time                                                                                      
*                                                                                                                                        
* Arguments:                                                                                                                             
*  Parameter1:  UINT32 param1 : Structure pointer of device                                                                                        
*  Parameter2:  UINT32 param2 : unused                                                                                                                 
*                                                                                                                                        
* Return Value: INT32                                                                                                                    
*****************************************************************************/                                                           
void nim_c3501h_task(UINT32 param1, UINT32 param2)                                                                                        
{                                                                                                                                        
	struct nim_device *dev = (struct nim_device *) param1 ;                                                                              
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv ;                                                            
    UINT32 last_time = 0;                                                                                                                               
                                                                                                                                         
#ifdef CHANNEL_CHANGE_ASYNC                                                                                                              
	UINT32 flag_ptn;                                                                                                                     
#endif   

#ifdef FC_SEARCH_RANGE_LIMITED
    UINT8 lock_state = 0;
#endif

	priv->tsk_status.m_sym_rate = 0x00;                                                                                                  
	priv->tsk_status.m_code_rate = 0x00;                                                                                                 
	priv->tsk_status.m_map_type = 0x00;                                                                                                  
	priv->tsk_status.m_work_mode = 0x00;                                                                                                 
	priv->tsk_status.m_info_data = 0x00;                                                                                                 
                                                                                                                                         
	NIM_PRINTF("Enter %s \r\n", __FUNCTION__);  
                                                                                                                                         
	while (1)                                                                                                                            
	{                                                                                                                                  
    #ifdef CHANNEL_CHANGE_ASYNC                                                                                                          
		flag_ptn = 0;                                                                                                                    
		if (NIM_FLAG_WAIT(&flag_ptn, priv->flag_id, NIM_FLAG_CHN_CHG_START, OSAL_TWF_ANDW | OSAL_TWF_CLR, 0) == OSAL_E_OK)               
		{                                                                                                                                
			//osal_flag_clear(priv->flag_id, NIM_FLAG_CHN_CHG_START);                                                                    
			NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHANGING);                                                                          
			osal_mutex_lock(priv->c3501h_mutex, OSAL_WAIT_FOREVER_TIME);
			priv->wait_chanlock_finish_flag = 0;
			NIM_PRINTF("\t\t ====== TP Signal INtensity Quality ======= \r\n");                                                            
			nim_c3501h_waiting_channel_lock(dev, priv->cur_freq, priv->cur_sym, priv->change_type, priv->isid);  
#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
            nim_c3501h_set_work_mode(dev, 0x03);
#endif
			priv->wait_chanlock_finish_flag = 1;
			osal_mutex_unlock(priv->c3501h_mutex);                                                                                        
			NIM_FLAG_CLEAR(priv->flag_id, NIM_FLAG_CHN_CHANGING);                                                                        
			NIM_PRINTF("\t\t Here is the task for C3505H wait channel lock \r\n");                                                          
            demod_mutex_flag = 0;
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
				nim_c3501h_get_lock(dev, &(priv->tsk_status.m_info_data));                                                                
                // Found locked then refresh the tp information                                                                          
				if (priv->tsk_status.m_info_data && (priv->t_param.t_i2c_err_flag == 0x00))                                              
				{      
				    #if 0
					nim_c3501h_get_symbol_rate(dev, &(priv->tsk_status.m_sym_rate));                                                      
					nim_c3501h_reg_get_code_rate(dev, &(priv->tsk_status.m_code_rate));                                                   
					nim_c3501h_reg_get_work_mode(dev, &(priv->tsk_status.m_work_mode));                                                   
					nim_c3501h_reg_get_map_type(dev, &(priv->tsk_status.m_map_type));                                                     
					nim_c3501h_clear_interrupt(dev);               
                    #endif
					priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;                                                                  
				}                                                                                                                        
			}                                                                                                                            
		}                                                                                                                                

#if 0                                                                                                                                         
	    if(priv->search_type == NIM_OPTR_CHL_CHANGE)                                                                                     
        {                                                                                                                                
	    #ifdef ANTI_WIMAX_INTF                                                                                                           
   	        if(force_adpt_disable == 1)                                                                                                  
            {                                                                                                                            
                nim_c3501h_cr_new_adaptive_unlock_monitor(dev);                                                                           
                nim_c3501h_nframe_step_tso_setting(dev,100,0x00);                                                                         
		    }                                                                                                                            
		    else                                                                                                                         
		    {                                                                                                                            
		        nim_c3501h_task_tso_setting(dev,100,0x00);                                                                                
		    }                                                                                                                            
	    #else		                                                                                                                     
            nim_c3501h_cr_new_adaptive_unlock_monitor(dev);                                                                               
            nim_c3501h_nframe_step_tso_setting(dev,100,0x00);                                                                             
	    #endif	                                                                                                                         
        }                                                                                                                                
#endif
                                                                                                                                         
        if(priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR)                                                                          
	        comm_sleep(100);                                                                                                             
        else                                                                                                                             
	        comm_sleep(5);  
        
#ifdef FC_SEARCH_RANGE_LIMITED
        if ((osal_get_tick() > priv->ul_status.check_lock_time) > 250)
        {
            NIM_PRINTF("nim_c3501h_get_lock_check_limited function is not called by UI every 250ms,  Make up for it here\n");
            nim_c3501h_get_lock_check_limited(dev, &lock_state);
        }
#endif 
     
	    // Monitor MER and do debug_in_task and auto adaptive
	    if ((osal_get_tick() - last_time) > 250)
	    {
#if 0
#ifdef C3501H_DEBUG_FLAG
	    if (0x04 != (priv->debug_flag&0x04))
#endif
	    {
	        //nim_c3501h_check_hw_reg(dev);
	    }
#endif
	    
#ifdef DEBUG_IN_TASK   // For debug in task used
			nim_c3501h_reg_get_work_mode(dev, &(priv->channel_info->work_mode));
			if (1 == priv->channel_info->work_mode)
	        	nim_c3501h_get_fer(dev , &(priv->channel_info->per));
			else
				nim_c3501h_get_per(dev , &(priv->channel_info->per));

	        nim_c3501h_get_ber(dev , &(priv->channel_info->ber));
	        //nim_c3501h_get_phase_error(dev , &(priv->channel_info->phase_err));
	        //nim_c3501h_get_mer_task(dev);
#endif

#if (defined(DEBUG_IN_TASK) || defined(IMPLUSE_NOISE_IMPROVED))  // For debug in task and auto adaptive used
	        nim_c3501h_get_snr_db(dev, &(priv->channel_info->snr));
#endif
	        if (0 == demod_mutex_flag)
	        {
	            osal_mutex_lock(priv->scan_mutex, OSAL_WAIT_FOREVER_TIME);
	            priv->t_param.auto_adaptive_doing = 1;
#ifdef DEBUG_IN_TASK 
	            nim_c3501h_debug_intask(dev);
#endif  

#ifdef IMPLUSE_NOISE_IMPROVED
	            nim_c3501h_auto_adaptive(dev);    
#endif 
	            //nim_c3501h_mon_fake_lock(dev);
	            //nim_c3501h_fc_search_adaptive(dev);

	            osal_mutex_unlock(priv->scan_mutex);
	        }   
	        priv->t_param.auto_adaptive_doing = 0;
	        last_time = osal_get_tick();
	    }
	}
    ERR_PRINTF("Run out of %s \r\n", __FUNCTION__);                                                                                        
}        


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_data_cap.c																												COPY FROM nim_dvbs_c3505_data_cap.c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
*  INT32 nim_c3501h_adc2mem_entity(struct nim_device *dev, UINT32 *cap_buffer, UINT32 dram_len,UINT8 cmd)
* capture RX_ADC data to DRAM
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*
* Return Value: INT32
*****************************************************************************/



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_debug.c																													COPY FROM nim_dvbs_c3505_debug.c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_hw_cfg.c																												COPY FROM nim_dvbs_c3505_hw_cfg.c	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef FPGA_PLATFORM

#if 1
static UINT8 g_reg_lock = 0;

/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i = 0;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	while (g_reg_lock)
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}
	g_reg_lock = 1;

	//pData[0] = bMemAdr;
	if (bLen > 8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
		//dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
		//dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
	}
	g_reg_lock = 0;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_read_16bits(struct nim_device *dev, UINT16 bMemAdr, UINT16 *pData, UINT8 bLen)
{
	UINT8 i = 0;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	while (g_reg_lock)
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}
	g_reg_lock = 1;

	//pData[0] = bMemAdr;
	if (bLen > 8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_input_16b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
	}
	g_reg_lock = 0;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register write function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i = 0;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;


	while (g_reg_lock)
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}
	g_reg_lock = 1;

	if (bLen >8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
		//dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
		//dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
	}

	g_reg_lock = 0;
	return SUCCESS;
}
#else
/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;


	//pData[0] = bMemAdr;
	if (bLen > 8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
		//dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
		//dev_input_8b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_read_16bits(struct nim_device *dev, UINT16 bMemAdr, UINT16 *pData, UINT8 bLen)
{
	UINT8 i;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;


	//pData[0] = bMemAdr;
	if (bLen > 8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_input_16b(&pData[i], NIM_C3501H_BASE_ADDR, bMemAdr+i);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register write function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 i;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if (bLen >8)
	{
		nim_c3501h_set_err(dev);
		return ERR_FAILUE;
	}

	for (i = 0; i < bLen; i++)
	{
		dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
		//dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
		//dev_output_8b(NIM_C3501H_BASE_ADDR, bMemAdr+i, pData[i]);
	}

	return SUCCESS;
}
#endif

#else

#ifdef I2C_RETRY

/*****************************************************************************
* INT32 nim_reg_read_core(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read core function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : read lenth
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_reg_read_core(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    INT32 err;
    UINT8 chip_adr ;
    UINT32 i2c_type_id;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    //NIM_PRINTF("%s, chip_adr=0x%x, bMemAdr=0x%x\n", __FUNCTION__, chip_adr, bMemAdr);
	
    // For c3501h iic reg addr 
    if (bMemAdr >= 256)
    {
        bMemAdr -= 256;
        chip_adr += 2;
    }
    
    pData[0] = bMemAdr;
    NIM_MUTEX_ENTER(priv);
    err = i2c_write_read(i2c_type_id, chip_adr, pData, 1, bLen);
    NIM_MUTEX_LEAVE(priv);
    if (err)
    {
        if (priv->ul_status.m_pfn_reset_c3501h)
        {
            priv->ul_status.m_pfn_reset_c3501h((priv->tuner_index + 1) << 16);
            osal_task_sleep(100);
            priv->t_param.t_i2c_err_flag = 0x01;
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
        }
        nim_c3501h_set_err(dev);
        NIM_PRINTF("c3501h i2c read error = %d,chip_adr=0x%x,bMemAdr=0x%x,I2C_FOR_C3501H = %d,TaskID=%d\n", -err,chip_adr, pData[0],i2c_type_id,osal_task_get_current_id());
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
            priv->t_param.t_i2c_err_flag = 0x00;
            nim_c3501h_open(dev);
        }
    }
    return err;
}

/*****************************************************************************
* INT32 nim_reg_write_core(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register write core function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : write lenth
*
* Return Value: The result of writing  successfully or not
*****************************************************************************/
INT32 nim_reg_write_core(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    int err;
    UINT8 i, buffer[8];
    UINT8 chip_adr ;
    UINT32 i2c_type_id;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    //NIM_PRINTF("%s, chip_adr=0x%x, bMemAdr=0x%x\n", __FUNCTION__, chip_adr, bMemAdr);

    // For c3501h iic reg addr 
    if (bMemAdr >= 256)
    {
        bMemAdr -= 256;
        chip_adr += 2;
    }

    if (bLen > 7)
    {
        nim_c3501h_set_err(dev);
		NIM_PRINTF("line=%d, in %s, blen>7.\n", __LINE__, __FUNCTION__);
        return ERR_FAILUE;
    }
    buffer[0] = bMemAdr;
    for (i = 0; i < bLen; i++)
    {
        buffer[i + 1] = pData[i];
    }
    NIM_MUTEX_ENTER(priv);
    err = i2c_write(i2c_type_id, chip_adr, buffer, bLen + 1);	
    NIM_MUTEX_LEAVE(priv);
    if (err != 0)
    {
        if (priv->ul_status.m_pfn_reset_c3501h)
        {
            priv->ul_status.m_pfn_reset_c3501h((priv->tuner_index + 1) << 16);
            osal_task_sleep(100);
            priv->t_param.t_i2c_err_flag = 0x01;
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
        }
        nim_c3501h_set_err(dev);
        NIM_PRINTF("c3501h i2c write error = %d,chip_adr=0x%x, bMemAdr=0x%x,I2C_FOR_C3501H = %d,TaskID=%d\n", -err,chip_adr, pData[0],i2c_type_id,osal_task_get_current_id());
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
            priv->t_param.t_i2c_err_flag = 0x00;
            nim_c3501h_open(dev);
        }
    }
    return err;
}


/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read  function
*                   Can read 8-byte one time and bLen must no more than 8, this function call
*                   nim_reg_read_core, check its execution results, and redo it while result was faild
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : read lenth
*
* Return Value: The result of writing  successfully or not
*****************************************************************************/
INT32 nim_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    UINT8 ret = -1;
    UINT8 err_cnt = 0;

    while (1)
    {
        ret = nim_reg_read_core(dev, bMemAdr, pData, bLen);
        if (SUCCESS != ret)
            err_cnt++;
        else
            return ret;

        if (err_cnt > 2)
        {
            ERR_PRINTF("%s, line:%d, bMemAdr:0x%x, pData[0]:0x%x, bLen:0x%x,err_cnt:%d, Exit!!!\n", \
                __FUNCTION__, __LINE__, bMemAdr, pData[0], bLen, err_cnt);
            return ret;
        }
    }
}


/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register write  function
*                   Can write 8-byte one time and bLen must no more than 8, this function call
*                   nim_reg_write_core, check its execution results, and redo it while result was faild
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : write lenth
*
* Return Value: The result of writing  successfully or not
*****************************************************************************/
INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    UINT8 ret = -1;
    UINT8 err_cnt = 0;

    while (1)
    {
        ret = nim_reg_write_core(dev, bMemAdr, pData, bLen);
        if (SUCCESS != ret)
            err_cnt++;
        else
            return ret;

        if (err_cnt > 2)
        {
            ERR_PRINTF("%s, line:%d, bMemAdr:0x%x, pData[0]:0x%x, bLen:0x%x,err_cnt:%d, Exit!!!\n", \
                __FUNCTION__, __LINE__, bMemAdr, pData[0], bLen, err_cnt);
            return ret;
        }
    }
}

#else

/*****************************************************************************
* INT32 nim_reg_read_core(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register read core function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : read lenth
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    INT32 err;
    UINT8 chip_adr ;
    UINT32 i2c_type_id;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	UINT16 origin_addr;
    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    //NIM_PRINTF("%s, chip_adr=0x%x, bMemAdr=0x%x\n", __FUNCTION__, chip_adr, bMemAdr);

	origin_addr=bMemAdr;
	
    // For c3501h iic reg addr 
    if (bMemAdr >= 256)
    {
        bMemAdr -= 256;
        chip_adr += 2;
    }
    
    pData[0] = bMemAdr;
    NIM_MUTEX_ENTER(priv);
    err = i2c_write_read(i2c_type_id, chip_adr, pData, 1, bLen);
    NIM_MUTEX_LEAVE(priv);
    if (err)
    {
        if (priv->ul_status.m_pfn_reset_c3501h)
        {
            priv->ul_status.m_pfn_reset_c3501h((priv->tuner_index + 1) << 16);
            osal_task_sleep(100);
            priv->t_param.t_i2c_err_flag = 0x01;
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
        }
        nim_c3501h_set_err(dev);
        NIM_PRINTF("c3501h i2c read error = %d,chip_adr=0x%x,origin_addr=0x%x,bMemAdr=0x%x,I2C_FOR_C3501H = %d,TaskID=%d\n", -err,chip_adr, origin_addr, pData[0],i2c_type_id,osal_task_get_current_id());
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
            priv->t_param.t_i2c_err_flag = 0x00;
            nim_c3501h_open(dev);
        }
    }
    return err;
}


/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: c3501h register write core function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 bMemAdr : register address
*  Parameter3: UINT8 *pData : data pointer
*  Parameter4: UINT8 bLen : write lenth
*
* Return Value: The result of writing  successfully or not
*****************************************************************************/
INT32 nim_c3501h_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen)
{
    int err;
    UINT8 i, buffer[8];
    UINT8 chip_adr ;
    UINT32 i2c_type_id;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	UINT16 origin_addr;
    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    //NIM_PRINTF("%s, chip_adr=0x%x, bMemAdr=0x%x\n", __FUNCTION__, chip_adr, bMemAdr);

    //if (bMemAdr == 344)
    //    NIM_PRINTF("%s, chip_adr=0x%x, bMemAdr=0x%x, pData=0x%x\n", __FUNCTION__, chip_adr, bMemAdr, pData[0]);

	origin_addr=bMemAdr;
    // For c3501h iic reg addr 
    if (bMemAdr >= 256)
    {
        bMemAdr -= 256;
        chip_adr += 2;
    }

    if (bLen > 7)
    {
        nim_c3501h_set_err(dev);
		NIM_PRINTF("line=%d, in %s, blen>7.\n", __LINE__, __FUNCTION__);
        return ERR_FAILUE;
    }
    buffer[0] = bMemAdr;
    for (i = 0; i < bLen; i++)
    {
        buffer[i + 1] = pData[i];
    }
    NIM_MUTEX_ENTER(priv);
    err = i2c_write(i2c_type_id, chip_adr, buffer, bLen + 1);	
    NIM_MUTEX_LEAVE(priv);
    if (err != 0)
    {
        if (priv->ul_status.m_pfn_reset_c3501h)
        {
            priv->ul_status.m_pfn_reset_c3501h((priv->tuner_index + 1) << 16);
            osal_task_sleep(100);
            priv->t_param.t_i2c_err_flag = 0x01;
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
        }
        nim_c3501h_set_err(dev);
        NIM_PRINTF("c3501h i2c write error = %d,chip_adr=0x%x,origin_addr=0x%x, bMemAdr=0x%x,I2C_FOR_C3501H = %d,TaskID=%d\n", -err,chip_adr,origin_addr, pData[0],i2c_type_id,osal_task_get_current_id());
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
			NIM_PRINTF("[%s %d]priv->t_param.t_i2c_err_flag=%d\n", __FUNCTION__, __LINE__, priv->t_param.t_i2c_err_flag);
            priv->t_param.t_i2c_err_flag = 0x00;
            nim_c3501h_open(dev);
        }
    }
    return err;
}
#endif

#endif

/*****************************************************************************
*  INT32 nim_c3501h_set_err(struct nim_device *dev)
* Description: set error flag for i2c read/wirte function
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_err(struct nim_device *dev)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	priv->ul_status.m_err_cnts++;
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_get_err(struct nim_device *dev)
* Description: get error flag for i2c read/wirte function
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_get_err(struct nim_device *dev)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \r\n",__FUNCTION__);
        return ERR_NO_MEM;
    }

	if (priv->ul_status.m_err_cnts > 0x00)
		return ERR_FAILED;
	else
		return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_clear_err(struct nim_device *dev)
* Description: clear error flag for i2c read/wirte function
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_clear_err(struct nim_device *dev)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \r\n",__FUNCTION__);
        return ERR_NO_MEM;
    }
    
	priv->ul_status.m_err_cnts = 0x00;

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_reg_get_chip_type(struct nim_device *dev)
* Description: read chip id to device struct
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_reg_get_chip_type(struct nim_device *dev)
{
	UINT8 temp[4] ={ 0x00, 0x00, 0x00, 0x00 };
	UINT32 m_Value = 0x00;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	
	priv->ul_status.m_c3501h_type = 0x00;
	nim_reg_read(dev, C3501H_R00_CHIP_ID, temp, 4);
	m_Value = temp[3];
	m_Value = (m_Value << 8) | temp[2];
	m_Value = (m_Value << 8) | temp[1];
	m_Value = (m_Value << 8) | temp[0];
	priv->ul_status.m_c3501h_type = m_Value;

    NIM_PRINTF("%s, ID = 0x%x\n", __FUNCTION__, m_Value);
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_reg_get_strap_bond_info(struct nim_device *dev)
* Description: read chip strap bond pin information to device struct
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_reg_get_strap_bond_info(struct nim_device *dev)
{
	UINT8 temp[4] ={ 0x00, 0x00, 0x00, 0x00 };
	UINT32 m_Value = 0x00;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	
	priv->ul_status.m_c3501h_strap_bond_info = 0x00;
	nim_reg_read(dev, C3501H_R00_STRAP_BOND_INFO, temp, 4);
	m_Value = temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
	priv->ul_status.m_c3501h_strap_bond_info = m_Value;

    NIM_PRINTF("%s, C3501H_R00_STRAP_BOND_INFO = 0x%x\n", __FUNCTION__, m_Value);
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_hw_check(struct nim_device *dev)
* Description: Chip ID check
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_hw_check(struct nim_device *dev)
{
	//struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	NIM_PRINTF("Enter function: %s \r\n",__FUNCTION__);

	nim_c3501h_reg_get_chip_type(dev);
#if 0
	if (CHIP_ID_3501H == priv->ul_status.m_c3501h_type)
		return SUCCESS;
	else
    {
        ERR_PRINTF("Exit with Chip ID error %s \r\n", __FUNCTION__);
        ERR_PRINTF("The expect Chip ID = %x , real Chip ID = %x \r\n", CHIP_ID_3501H, priv->ul_status.m_c3501h_type);
        return ERR_FAILED;
    }  
#endif	

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_clear_interrupt(struct nim_device *dev)
* Description: Clear the hw interrupt
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_clear_interrupt(struct nim_device *dev)
{
	UINT8 data = 0;	
	
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1 + 3, &data, 1);
	data &= 0x7f;//clear bit7, DEMOD_INT
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1 + 3, &data, 1);

	data = 0;
	nim_reg_write(dev, C3501H_R18_TOP_FSM_RPT_2 + 1, &data, 1);
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_interrupt_mask(struct nim_device *dev, UINT8 int_mask)
* Description: Set interrupt mask
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 int_mask: set 1 to enable it.
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_interrupt_mask(struct nim_device *dev, UINT8 int_mask)
{
	//NIM_PRINTF("Enter function: %s, int_mask = 0x%x\r\n", __FUNCTION__, int_mask);
	// Bit 7:0 is interrupt mask 
	//INT_MASK	[7:0]	[7:0]
	//INT_CAP_END	1	[8]
	//INT_CH_LOCK	1	[9]
	//INT_CH_UNLOCK	1	[10]
	//INT_HW_TIMEOUT	1	[11]
	//INT_TR_UNLOCK	1	[12]
	//INT_CR_UNLOCK	1	[13]
	//INT_FRAME_UNLOCK	1	[14]
	//INT_DISEQC_END	1	[15]

	nim_reg_write(dev, C3501H_R18_TOP_FSM_RPT_2, &int_mask, 1);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reset_fsm(struct nim_device *dev)
* Description: Reset top fsm, usually it used with nim_c3501h_start_fsm
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reset_fsm(struct nim_device *dev)
{
	UINT8 data = 0;

	//NIM_PRINTF("Enter function: %s\r\n",__FUNCTION__);

	// Bit 1 is reset 
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	data |= 0x02;
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3501h_hw_check(struct nim_device *dev)
* Description: Start top fsm, usually it used with nim_c3501h_reset_fsm
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_start_fsm(struct nim_device *dev)
{
	UINT8 data = 0;

	//NIM_PRINTF("Enter function: %s \r\n",__FUNCTION__);
	// Bit 0 is start 
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	data |= 0x01;
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_clean_fsm_st(struct nim_device *dev)
* Description: Clean fsm lock register state, while LOCK_ST_RPT_MODE = 1
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_clean_fsm_st(struct nim_device *dev)
{
	UINT8 data = 0;

	//NIM_PRINTF("Enter function: %s\r\n",__FUNCTION__);

	nim_reg_read(dev, C3501H_R14_TOP_FSM_RPT_1 + 3, &data, 1);
	data |= 0x10;
	nim_reg_write(dev, C3501H_R14_TOP_FSM_RPT_1 + 3, &data, 1);
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_fsm_state(struct nim_device *dev, UINT8 *fsm_state)
* Description: Read top fsm state
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_fsm_state(struct nim_device *dev, UINT8 *fsm_state)
{
    UINT8 data = 0;

    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1); 
    
	nim_reg_read(dev, C3501H_R20_MODU_INFO_RPT_2 + 2, &data, 1);
    *fsm_state = data&0x3f;
    NIM_PRINTF("Enter function: %s, fsm_state:0x%x\r\n",__FUNCTION__, *fsm_state);
    
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_cap_mode(struct nim_device *dev, UINT8 cap_mode)
* Description: Set top fsm to cap mode, unused
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_cap_mode(struct nim_device *dev, UINT8 cap_mode)
{
	UINT8 data;
	
	NIM_PRINTF("Enter function: %s , cap_mode = %d\r\n",__FUNCTION__, cap_mode);
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	// Bit 3 is cap mode 
	if (1 == (cap_mode & 0x01))
		data |= 0x08;
	else
		data &= 0xf7;

	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_hw_timeout(struct nim_device *dev, UINT16 time_thr)
* Description: Set top fsm 's timeout value for tp lock, This is invalid if in retry mode(RTYFLAG = 1)
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_hw_timeout(struct nim_device *dev, UINT16 time_thr)
{
	UINT8 data = 0;

	NIM_PRINTF("Enter function: %s , time_thr = %d\r\n",__FUNCTION__, time_thr);
	// Bit 9:0 is hw timeout threshold
	data = time_thr & 0xff;
	nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3, &data, 1);
	nim_reg_read(dev, C3501H_R08_TOP_FSM_CFG_3 + 1, &data, 1);
	data &= 0xfc;
	data |= ((time_thr >> 8) & 0x03);
	nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3 + 1, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_hw_open(struct nim_device *dev)
* Description: Open hardware, power up some modules
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_hw_open(struct nim_device *dev)
{
	UINT8 data = 0;

#ifdef FPGA_PLATFORM
	if (IIC == dev->type)
	{
		nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
		data |= 0x03;
		nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
	}
#endif
    
    // Rst rxadc, first power down then power up it, by R1
	// Power down RXADC
	data = 0x31;
	nim_reg_write(dev, C3501H_R04_SYS_CFG + 1, &data, 1);

    comm_sleep(1);

	// Power up RXADC
	data = 0x01;
	nim_reg_write(dev, C3501H_R04_SYS_CFG + 1, &data, 1);

    // Power up PLL
    //data = 0x48;    
    //nim_reg_write(dev, C3501H_R04_MPLL_CFG, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_hw_close(struct nim_device *dev)
* Description: Close hardware, power down some modules
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_hw_close(struct nim_device *dev)
{
	UINT8 data = 0;                                                                                                                      
 
    // Power down RXADC
    data = 0x31;
    nim_reg_write(dev, C3501H_R04_SYS_CFG + 1, &data, 1);

    // Power down PLL
    //data = 0x49;    
    //nim_reg_write(dev, C3501H_R04_MPLL_CFG, &data, 1);    

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_hw_init(struct nim_device *dev)
* Description: Initial hardware register
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_hw_init(struct nim_device *dev)
{
	UINT8 data = 0; 
	UINT8 data_w = 0;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

    // Set ADC iq swap from board_config
    nim_c3501h_set_adc(dev, 0);

    // AGC1 Init
    data = 0x56;
    nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 2, &data, 1);    

    data = 0x10;
    nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1); 

    // Set FC/RS search RS_FC_RANGE_LMT_EN, can't used it, use sw limited
#if 0
    nim_c3501h_set_search_range_limited(dev, 1);
#endif

//---------- ldpc use bbheader crc to decide lock and fail, should be changed later---------------------------------
	//enable per statics
	//nim_reg_read(dev, C3501H_R0C_S2_FEC_CFG_0, &data, 1);	
	//data_w = 0x084;
	//nim_reg_write(dev, C3501H_R0C_S2_FEC_CFG_0, &data_w, 1);

//---------- change  S2_FEC_LOCK_THR 1 --> 2---------------------------------
    //nim_reg_read(dev, C3501H_R0C_S2_FEC_CFG_0, &data, 1);	
    // 10c[5:0] = 2
    //data &= 0xc0;
	//data |=0x02;
	//nim_reg_write(dev, C3501H_R0C_S2_FEC_CFG_0, &data, 1);

//---------- get BB_START signal through LA == 20180104 gloria start==---------------------------------
    // [1]. LA_P1_MUX_SEL(8'hca);
    /* P1_CAP_AFT_TRG = 0x184[23:16]
           P1_CAP_AFT_TRG[7:6] : 00/01->LA_P1_DSP_IN
                                             10     ->LA_P1_PHY_IN
                                             11     ->LA_P1_FEC_IN      ->BB_START @FEC_IN
           P1_CAP_AFT_TRG[5:0] = a ->S2_FEC_TEST_OUT3
       */
    
    data = 0xca;
    nim_reg_write(dev, C3501H_R44_LA_PROBE_1_CFG+2, &data, 1);

	// [2]. LA_P1_TRG_MASK(16'h0100);  just check BB_START @bit8
	// data = 0x0100;
	// nim_reg_write(dev, C3501H_R48_LA_PROBE_1_TAR+1, &data, 2);
	data = 0x00;
	nim_reg_write(dev, C3501H_R48_LA_PROBE_1_TAR, &data, 1);
	data = 0x01;
	nim_reg_write(dev, C3501H_R48_LA_PROBE_1_TAR+1, &data, 1);

	// [3]. check BB_START posedge  set trigger type
	/*P1_TRG_TYPE=0x184[2:0]
       Trigger type:
       0: Posedge
       1: Negedge
       2: Equal to TAR
       3: None-equal to TAR
       4: NOR with TAR
       5: NOR with Self
       6: Force no trigger
       7: Force always tigger
       */
	data = 0x00;
	nim_reg_write(dev, C3501H_R44_LA_PROBE_1_CFG, &data, 1);
//---------- get BB_START signal through LA == 20180104 gloria end==---------------------------------

//---------- per monitor initial, should be encapsulated to per initial later---------------------------------
	//enable per statics
	nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);
	data &= 0xf8;	// For commond
	data_w = data | 0x08;
	nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);

	// Clear per cnt
	nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);
	data &= 0xf8;
	data_w = data | 0x02;
	nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);

	// Restart monitor
	nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
	data &= 0xf8;
	data_w = data | 0x01;
	nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);

//---------- fer monitor initial, should be encapsulated to fer initial later---------------------------------
	//enable fer statics
	nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
	data &= 0xf8;
	data_w = data | 0x08;
	nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);

	// Clear fer cnt
	nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
	data &= 0xf8;
	data_w = data | 0x02;
	nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);

	// Restart monitor
	nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
	data &= 0xf8;
	data_w = data | 0x01;
	nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);


//---------- ber monitor initial, should be encapsulated to ber initial later---------------------------------
	// init ber_thr to: 2: 1,000,000; reg138[14:12]
	nim_reg_read(dev, C3501H_R38_MON_BER_CFG + 1, &data, 1);	
	data_w = data & 0x8f;
	data_w |= 0x20;
	nim_reg_write(dev, C3501H_R38_MON_BER_CFG + 1, &data_w, 1);

	//enable ber statics
	nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
	data_w = data | 0x08;
	nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);

	// Clear ber cnt
	nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
	data_w = data | 0x02;
	nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);

	// Restart monitor
	nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
	data_w = data | 0x01;
	nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);


//------------ hyman cr adaptive initial  -------------------------
    nim_reg_read(dev, C3501H_RC4_CR_ADPT_CF_RPT + 2, &data, 1);
    data |= 1;      // set CR_ADPT_PARA_EN=1  c4[16]
    data |= 0x30;   // set CR_ADPT_LOCK_MUX = 3, c4[21:20], enable cr adaptive when est_noise is ready
    nim_reg_write(dev, C3501H_RC4_CR_ADPT_CF_RPT + 2, &data, 1);

    comm_sleep(100);
    
    nim_c3501h_cr_adpt_tab_init(dev); // init table

    // TSO, Software < bit6bit7:QPSK bitmode:00:1bit,01:2bit,10:4bit,11:8bit
#if defined FPGA_PLATFORM || defined SDK_FPGA_TEST
    data = 3;   // Fix to 8bit mode
#else
    data = ((priv->tuner_config_data.qpsk_config & 0xc0) >> 6); // TSO BIT MODE
#endif

#if (NIM_OPTR_CCM == ACM_CCM_FLAG)	  
    nim_c3501h_tso_initial (dev, 1, data);
#else
    nim_c3501h_tso_initial (dev, 0, data);  // ACM mode colse dummy frame for vrc mosaic.?? found in c3505??
#endif

    nim_c3501h_tso_soft_cbr_off(dev);  // Default auto hw config tso clk;

    nim_c3501h_clear_plsn(dev); // Paladin for plsn default value

    // FEC 
    data = 0xff;
    //data = 0x32;    // Default 
	nim_reg_write(dev, C3501H_R14_S2_FEC_RPT, &data, 1);

    //set PL_S2X_MODE_BY_ACM_ALG1=0x1f
    //for china 6b 3770/h/12360
    data = 0xf0;
    nim_reg_write(dev, C3501H_R84_PL_CFG_2 + 2, &data, 1);
    data = 0x09;
    nim_reg_write(dev, C3501H_R84_PL_CFG_2 + 3, &data, 1);

	//// Yiping for iqswap 11/08/2017
	//set PL_SYNC_CNT_THR_TO_LOCK=0x14
	//data = 0x14;
	//nim_reg_write(dev, C3501H_R94_PL_CFG_6 + 2, &data, 1);
	//data = 0x88;
	//nim_reg_write(dev, C3501H_R90_PL_CFG_5 + 3, &data, 1);

	//set TR_LD_THR_HIGH=0x20(default:0x0c)
	data = 0x20;
	nim_reg_write(dev, C3501H_R58_TR_CFG_0, &data, 1);
	data = 0x41;
	nim_reg_write(dev, C3501H_R58_TR_CFG_0 + 2, &data, 1);
	//// 

    // Cfg DVB-S Lock threshold
    #if 0
    nim_c3501h_sub_write_vb(dev, 0, 2450);
    nim_c3501h_sub_write_vb(dev, 1, 1400);
    nim_c3501h_sub_write_vb(dev, 2, 910);
    nim_c3501h_sub_write_vb(dev, 3, 550);
    //nim_c3501h_sub_write_vb(dev, 4, 400); 
    nim_c3501h_sub_write_vb(dev, 5, 400);
	#else
    nim_c3501h_sub_write_vb(dev, 0, 2200);
    nim_c3501h_sub_write_vb(dev, 1, 1100);
    nim_c3501h_sub_write_vb(dev, 2, 700);
    nim_c3501h_sub_write_vb(dev, 3, 400);
    //nim_c3501h_sub_write_vb(dev, 4, 400); 
    nim_c3501h_sub_write_vb(dev, 5, 300);
    #endif
    
	nim_c3501h_set_beq(dev);

//#ifdef FOR_SKEW_LNB_TEST_QPSK
#if 0
    data = 0x00;  
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG, &data, 1);
    data = 0x00;  
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG + 1, &data, 1);
    data = 0x0a;  
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG + 2, &data, 1);
    data = 0x22;  
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG + 3, &data, 1);   
    NIM_PRINTF("Set force disable mapbeta for qpsk 9/10\n");                   
#endif

// For TSN support we need force work mode to ACM/VCM mode not auto
#ifdef TSN_SUPPORT
	//TSN_PRINTF("Force ACM mode for TSN support\n");
    //data = 0x34;  
    //nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
#else
	// Clear tsn flt buffer to zero.
	nim_c3501h_tsn_enable(dev, FALSE);
#endif

	nim_c3501h_set_pl_cfe_dis_pilot(dev);   // Paladin 20180106 for iq swap & pilot pl cfe bug

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_after_reset_set_param(struct nim_device *dev)
* Description: Initial hardware register same as nim_c3501h_hw_init
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_after_reset_set_param(struct nim_device *dev)
{
	//UINT8 data = 0;

	nim_c3501h_set_interrupt_mask(dev, 0xff);

    // Defailt is ok
	//nim_c3501h_set_work_mode(dev, AUTO_WORK_MODE);
	//nim_c3501h_set_sub_work_mode(dev, AUTO_SUB_WORK_MODE);
	//nim_c3501h_set_retry(dev, 1);

	// clip rs search range by yiping
	nim_c3501h_sub_write_fsm(dev, 0x04, 0xfe);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3501h_i2c_through_open(struct nim_device *dev)
* Description: Enable c3501h I2C through, after set this, soc can control tuner through I2C bus
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_c3501h_i2c_through_open(struct nim_device *dev)
{
#ifdef I2C_THROUGH_ALWAYS_OPEN
        return 0;
#endif

    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    nim_c3501h_clear_err(dev);	

	//NIM_PRINTF("[%s %d]Enter!\n", __FUNCTION__, __LINE__);
    if (priv->tuner_config_data.qpsk_config & C3501H_I2C_THROUGH)
    {
    	ER result;
		result = osal_semaphore_capture(tuner_i2c_sema,1000);
		if (E_OK != result)
		{
			ERR_PRINTF("[%s %d]result=%d\n", __FUNCTION__, __LINE__, result);
			return result;
		}
		
        UINT8 data = 0;

#if 0
        nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        data |= 0x03;   // Open THROUGH_EN, IIC_PASS_THR
        comm_delay(200);
        nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
#else
	data = 0x33;	// Open THROUGH_EN
	nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
    comm_delay(200);

    #if 0
    nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
    if (0x33 != data)
    {
        data = 0x33;	// Open THROUGH_EN
	    nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        comm_delay(200);
    }
    #endif
#endif
        
        //NIM_PRINTF("%s\r\n", __FUNCTION__);

        if (nim_c3501h_get_err(dev))
        {
        	if (OSAL_INVALID_ID!=tuner_i2c_sema)
        	{
        		osal_semaphore_release(tuner_i2c_sema);
        	}
			ERR_PRINTF("[%s %d]priv->ul_status.m_err_cnts = %d\n", __FUNCTION__, __LINE__, priv->ul_status.m_err_cnts);
            return ERR_FAILED;
        }

		//NIM_PRINTF("[%s %d]leave success!\n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

	//NIM_PRINTF("[%s %d]leave success!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_i2c_through_close(struct nim_device *dev)
* Description: Disable c3501h I2C through, after set this, soc cann't control tuner through I2C bus
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_c3501h_i2c_through_close(struct nim_device *dev)
{
#ifdef I2C_THROUGH_ALWAYS_OPEN
    return 0;
#endif

    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    nim_c3501h_clear_err(dev);
	//NIM_PRINTF("[%s %d]Enter!\n", __FUNCTION__, __LINE__);
    if (priv->tuner_config_data.qpsk_config & C3501H_I2C_THROUGH)
    {
        UINT8 data = 0;

		#if 0
        nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        data &= 0xfc;   // Close THROUGH_EN, IIC_PASS_THR
        comm_delay(200);
        nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
		#else
        comm_delay(200);
        data = 0x30;   // Close THROUGH_EN, IIC_PASS_THR
        nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);

        #if 0
        nim_reg_read(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        if (0x30 != data)
        {
            data = 0x30;    // Open THROUGH_EN
            nim_reg_write(dev, C3501H_R0C_I2C_SLAVE_CFG_2, &data, 1);
        }
        #endif
		#endif
        
        //NIM_PRINTF("%s\r\n", __FUNCTION__);
        
		if (OSAL_INVALID_ID!=tuner_i2c_sema)
    	{
    		osal_semaphore_release(tuner_i2c_sema);
    	}
		
        if (nim_c3501h_get_err(dev))
        {
        	ERR_PRINTF("[%s %d]priv->ul_status.m_err_cnts = %d\n", __FUNCTION__, __LINE__, priv->ul_status.m_err_cnts);
            return ERR_FAILED;
        }

		//NIM_PRINTF("[%s %d]leave success!\n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }
	//NIM_PRINTF("[%s %d]leave success!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner)
* Description: If we define use external LNB ic in board_config.c that we need initial it here
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner)
{
	/****For external lnb controller config****/
	struct nim_c3501h_private *priv_mem = (struct nim_c3501h_private *) dev->priv;
	priv_mem->ext_lnb_control = NULL;
	if (ptrQPSK_Tuner->ext_lnb_config.ext_lnb_control)
	{
		UINT32 check_sum = 0;
		check_sum = (UINT32) (ptrQPSK_Tuner->ext_lnb_config.ext_lnb_control);
		check_sum += ptrQPSK_Tuner->ext_lnb_config.i2c_base_addr;
		check_sum += ptrQPSK_Tuner->ext_lnb_config.i2c_type_id;
		if (check_sum == ptrQPSK_Tuner->ext_lnb_config.param_check_sum)
		{						
			priv_mem->ext_lnb_control = ptrQPSK_Tuner->ext_lnb_config.ext_lnb_control;
            NIM_MUTEX_ENTER(priv_mem);
			priv_mem->ext_lnb_control(0, LNB_CMD_ALLOC_ID, (UINT32) (&priv_mem->ext_lnb_id));
			priv_mem->ext_lnb_control(priv_mem->ext_lnb_id, LNB_CMD_INIT_CHIP, (UINT32) (&ptrQPSK_Tuner->ext_lnb_config));
            NIM_MUTEX_LEAVE(priv_mem);

			NIM_PRINTF("[%s %d]init over!\n", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		priv_mem->lnb_gpio_select = ptrQPSK_Tuner->ext_lnb_config.int_gpio_select;
		priv_mem->lnb_gpio_num = ptrQPSK_Tuner->ext_lnb_config.int_gpio_num;		
	}

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_set_adc(struct nim_device *dev)
* Description: Configure the adc include IQ swap and rx Vpp
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 vpp_sel, 0: 0.7v default, 1: 0.6v
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_adc(struct nim_device *dev, UINT8 vpp_sel)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	UINT8 data = 0;

    // Pointer check
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with Pointer check error in %s \r\n",__FUNCTION__);
        return ERR_NO_MEM;
    }

	// ADC setting
	NIM_PRINTF(" qpsk_config value: %x\r\n",priv->tuner_config_data.qpsk_config);

	if (priv->tuner_config_data.qpsk_config & C3501H_IQ_AD_SWAP)
	{
		data = 0x25;
		NIM_PRINTF(" Warning c3501h IQ AD SWAP\r\n");
	}
	else
		data = 0x05;

	nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG, &data, 1);

    data = ((vpp_sel&0x01) << 3);
    nim_reg_write(dev, C3501H_R04_SYS_CFG + 2, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case, UINT8 frz_agc1)
* Description: Set agc1 mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 low_sym: 1: symbol less than 6.5M
*  Parameter2: UINT8 s_case: for channel change and auto scan
*  Parameter2: UINT8 frz_agc1: for agc1 stable speedup
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case, UINT8 frz_agc1)
{
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    UINT8 data = 0;
    UINT8 agc1_bw = 2;
	
    // AGC1 setting
    if (priv->tuner_config_data.qpsk_config & C3501H_NEW_AGC1)
    {
      switch (s_case)
      {
        case NIM_OPTR_CHL_CHANGE:
            if (1 == low_sym)
                data = 0x8f;
            else
                data = 0x95;
            break;
        case NIM_OPTR_SOFT_SEARCH:
            if (1 == low_sym)
                data = 0x93;
            else
                data = 0x9a;
            break;
        case NIM_OPTR_FFT_RESULT:
            #if 0
                if (0x80 & priv->debug_flag)
                {
                    data = 0x90;
                    NIM_PRINTF("Set agc ref 0x10 for autoscan\n");
                }
                else
            #endif
                {
                    data = 0x99;
                    NIM_PRINTF("Set agc ref 0x19 for autoscan\n");
                }
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
				data = 0x93;
				else
				data = 0x9a;
				break;
			case NIM_OPTR_FFT_RESULT:
				data = 0x99;
				break;
		}
	}
    if ((priv->tuner_config_data.qpsk_config & C3501H_AGC_INVERT) == 0x0)  // STV6110's AGC be invert by QinHe
        data = data ^ 0x80;

	if(priv->tuner_type == IS_M3031)
	{
		data = data | 0x80;
		//data = data & 0x7f;
		//data = 0x18;
	}

    NIM_PRINTF("Set AGC1 value = %x\n", data);

    nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 1, &data, 1);

    nim_reg_read(dev, C3501H_R30_AGC1_CFG, &data, 1);
    data &= 0xe0;
    data |= 0x18;
    nim_reg_write(dev, C3501H_R30_AGC1_CFG+0x01, &data, 1);
    
    //**************** frz agc1 control add by Jalone*********************// 
    switch (frz_agc1)
    {
	    case NIM_FRZ_AGC1_OPEN:
			nim_reg_read(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
			data = data | 0x10; // write AGC1_AUTO_FRZ_EN = 1
			nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);	
			break;
	    case NIM_FRZ_AGC1_CLOSE:
			nim_reg_read(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
			data = data & 0xef; // write AGC1_AUTO_FRZ_EN = 0
			nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
			break;
	    default:
	      	NIM_PRINTF(" CMD for nim_c3501h_set_agc1 ERROR!!!");
	      	break;    	
    }

    // Set agc1 bw for low symbol rate TP(1~2M, 16/32/64apsk lock unstable issue) 20180110 paladin
    if (1 == low_sym)
    {
        agc1_bw = 3;
    }
    else
    {
        agc1_bw = 2;
    }
	nim_reg_read(dev, C3501H_R2C_ANA_AGC1_CFG + 2, &data, 1);
	data = data & 0xfc;
    data = (data | (agc1_bw&0x03));
	nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 2, &data, 1);    
    // End

    return SUCCESS;
}



/*****************************************************************************
* INT32 nim_c3501h_cfg_tuner_get_freq_err(struct nim_device *dev)
* Description: Set tuner(freq rs) and get tuner freq err to correct demod offset value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3501h_tp_scan_para *tp_scan_param
*  
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cfg_tuner_get_freq_err(struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param)
{
	UINT32	exceed_offset = 0;
    UINT8 freq_offset = 0;  // For 3m offset flag
    
	struct nim_c3501h_private* dev_priv;

    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	dev_priv = dev->priv;

	if((0 == tp_scan_param->est_rs) || ( 0 == tp_scan_param->est_freq))
    {   
        ERR_PRINTF("Exit with ERR_INPUT_PARAM in %s \n",__FUNCTION__);
		//return ERR_INPUT_PARAM;
		return ERR_NO_DEV;
    }
    
	/*if((tp_scan_param->est_freq >= (UINT32)(dev_priv->Tuner_Config_Data.Recv_Freq_High - QPSK_TUNER_FREQ_OFFSET)) || 
	   (tp_scan_param->est_freq <= (UINT32)(dev_priv->Tuner_Config_Data.Recv_Freq_Low + QPSK_TUNER_FREQ_OFFSET)))
		exceed_offset = 1;*/
		exceed_offset = 0;
    
	//	Symbol rate is less than 6.5M, low symbol rate 	
	tp_scan_param->low_sym = tp_scan_param->est_rs < C3501H_LOW_SYM_THR ? 1 : 0;	

    // When is low symbol rate ,set tuner freq add 3M, and rs + 3M *2 
#ifndef _SHIFT_3M_
	if (dev_priv->tuner_config_data.qpsk_config & C3501H_QPSK_FREQ_OFFSET)
#endif
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

    nim_c3501h_tuner_control(dev, tp_scan_param->tuner_freq, tp_scan_param->est_rs + tp_scan_param->correct_rs, &(tp_scan_param->freq_err));

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
*  INT32 nim_c3501h_tso_initial (struct nim_device *dev)
*Description: Initial the tso bit mode and clk and dummy enable/disable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 insert_dummy 1:dummy on, 0: dummy off
*  Parameter2: UINT8 tso_mode 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode)
{
	UINT8 data = 0;

    if (1 == insert_dummy)
    	nim_c3501h_tso_dummy_on(dev);
    else if (0 == insert_dummy)
        nim_c3501h_tso_dummy_off(dev);
    // Software < bit6bit7:QPSK bitmode:00:1bit,01:2bit,10:4bit,11:8bit
    // Hardware  0:2bit, 1:4bit, 2:8bit, 3:1bit
    
    nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
    data &= 0xfc;

	switch(tso_mode)
	{   
		case 0 : // 1bit
		{
			data |= 0x03;
			nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
            #ifdef SW_TSO_CLK_CFG   // Set clk div
            data = 0x01;
            nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 2, &data, 1);
            #endif

			//robin add, according to Bentao, TSO data1 data0 pad driver from 1 to 3
			nim_reg_read(dev, C3501H_R34_PINMUX_DRV_1 + 1, &data, 1);
		    data |= 0x03;
			nim_reg_write(dev, C3501H_R34_PINMUX_DRV_1 + 1, &data, 1);
			NIM_PRINTF("            tso_mode is 1bit mode \n"); 
		}
		break;
		case 1 : //2bit
		{
			nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
            #ifdef SW_TSO_CLK_CFG
            data = 0x02;
            nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 2, &data, 1);
            #endif

			//robin add, according to Bentao, TSO data1 data0 pad driver from 1 to 3
			nim_reg_read(dev, C3501H_R34_PINMUX_DRV_1 + 1, &data, 1);
		    data |= 0x03;
			nim_reg_write(dev, C3501H_R34_PINMUX_DRV_1 + 1, &data, 1);
			NIM_PRINTF("            tso_mode is 2bit mode \n"); 
		}
		break;
		case 2 : //4bit
		{
			data |= 0x01;
			nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
            #ifdef SW_TSO_CLK_CFG
            data = 0x04;
            nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 2, &data, 1); 
            #endif
			NIM_PRINTF("            tso_mode is 4bit mode \n"); 
		}
		break;
		case 3 : //8bit
		{
			data |= 0x02;
			nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
            #ifdef SW_TSO_CLK_CFG
            data = 0x08;
            nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 2, &data, 1);
            #endif
			NIM_PRINTF("            tso_mode is 8bit mode \n"); 
		}
		break;
	}

#ifdef SW_TSO_CLK_CFG   
    // Enable sw tso clk
    nim_reg_read(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);
    data |= 0x10;   
    nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);   
    // Set clk src 248Mhz
    nim_reg_read(dev, C3501H_R10_TSO_CLKG_CFG_1 + 3, &data, 1); 
    data |= 0x03;   
    nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 3, &data, 1);    
#endif	

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_tso_off (struct nim_device *dev)
* 	Close TSO output
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_tso_off (struct nim_device *dev)
{
	UINT8 data;
	// close fec ts output
	nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
	data = data | 0x10;
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
	NIM_PRINTF("            fec ts off\n");
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_tso_on (struct nim_device *dev)
* 	Open TSO output
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_tso_on (struct nim_device *dev)
{
	UINT8 data;
	// Open fec ts output
	nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
	data = data & 0xef;
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
	NIM_PRINTF("            fec ts on\n");
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_tso_dummy_off (struct nim_device *dev)
* disable the insertion of dummy TS pscket
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_tso_dummy_off (struct nim_device *dev)
{
	UINT8 data;
	nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
	data = data & 0x7f;
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);

    #if 0   // bentao 20170803
	nim_reg_read(dev, C3501H_R18_TSO_CFG + 1, &data, 1);
	data = data | 0x04;
	nim_reg_write(dev, C3501H_R18_TSO_CFG + 1, &data, 1);
    #endif
    
	NIM_PRINTF("%s\n", __FUNCTION__);
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_tso_dummy_on (struct nim_device *dev)
*  Enable the insertion of dummy TS packet 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
 INT32 nim_c3501h_tso_dummy_on (struct nim_device *dev)
{
	UINT8 data;
	nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
	data = data | 0x80;
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);
	//clear full then send mode of TSO
	#if 0   // bentao 20170803
	nim_reg_read(dev, C3501H_R18_TSO_CFG + 1, &data, 1);
	data = data & 0xfb;
	nim_reg_write(dev, C3501H_R18_TSO_CFG + 1, &data, 1);
    #endif
	NIM_PRINTF("%s\n", __FUNCTION__);
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate)
*Description: unused
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 bitrate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate)
{
	UINT8 data;
    /*
	if (bitrate > 512)
		bitrate = 512;
	data = (bitrate&0x0f)<<4;
	nim_reg_write(dev, RF0_HW_TSO_CTRL + 3, &data, 1);
	data = ((bitrate>>4)&0x1f) | 0x80;
	nim_reg_write(dev, RF0_HW_TSO_CTRL + 4, &data, 1);
	*/

    // Write CBR_SOFT_BRATE[11:0], 9.3 format, max is 511Mbits/s, unit is 1/8Mbits/s
    //... wait new code

    // Enable
	nim_reg_read(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);
	data = data | 0x20;
	nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);    

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_tso_soft_cbr_off (struct nim_device *dev)
*Description: unused
*
* Arguments:
*  Parameter1: struct nim_device *dev
* 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tso_soft_cbr_off (struct nim_device *dev)
{
	UINT8 data;

	nim_reg_read(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);
	data = data & 0xdf;
	nim_reg_write(dev, C3501H_R10_TSO_CLKG_CFG_1 + 1, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_set_rs (struct nim_device *dev, UINT32 rs)  
*Description: Set symbol rate to hardware and config tr parameter with symbol rate 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 rs, uint is k
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_rs(struct nim_device *dev, UINT32 rs)  
{
	//UINT8 data = 0;
    UINT8 ver_data[2] = {0};
	UINT32 sample_rate = 0;
	UINT32 temp = 0;

	nim_c3501h_get_dsp_clk(dev, &sample_rate);
	NIM_PRINTF("\t\t set RS %d with dsp clock %d\r\n", rs, sample_rate/1000);
	temp = nim_c3501h_multu64div(rs, 131072, sample_rate); // 131072 = 2^17

	//CR3F
	ver_data[0] = (UINT8) (temp & 0xFF);
	ver_data[1] = (UINT8) ((temp & 0xFF00) >> 8);

	NIM_PRINTF("\t\t set RS reg %x %x \r\n", ver_data[0], ver_data[1]);
	nim_reg_write(dev, C3501H_R04_TOP_FSM_CFG_2, ver_data, 2);
    
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_isid (struct nim_device *dev, struct nim_dvbs_isid *p_isid) 
*Description: Set isid filter work mode and isid value to hardware
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_dvbs_isid *p_isid
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid) 
{
	UINT8 data = 0;
	UINT8 isid_bypass = 0;
	UINT8 isid_mode   = 0;
	UINT8 isid_id   = 0;

	isid_bypass = p_isid->isid_bypass;
	isid_mode   = p_isid->isid_mode;
	isid_id     = p_isid->isid_write;

	if (isid_bypass)    // In 191 byte mode when we open filter and clsoe it , it maybe can/t play normally, need reset fsm
	{
	    nim_reg_read(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
	    data |= 0x08;
	    data &= 0xfb;
	    nim_reg_write(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
	}
	else if(isid_mode)
	{
		nim_reg_read(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
		data |= 0x04;
		data &= 0xf7;
		nim_reg_write(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
		nim_reg_write(dev, C3501H_R1C_BB_DEC_CFG+1, &isid_id, 1);
	}
	else if(isid_mode == 0)
	{
		nim_reg_read(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
		data &= 0xf3;
		nim_reg_write(dev, C3501H_R18_BB_CMD_CFG+2, &data, 1);
	}

	return SUCCESS;
}

/***************************************************************************************************
*  INT32 nim_c3501h_set_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid)
*  After TP locked, check TP's info and get isid.
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: UINT8
****************************************************************************************************/ 
INT32 nim_c3501h_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid)
{
    UINT8 work_mode = 0;
	struct nim_c3501h_private *priv = NULL;
    
    // Check 
    if ((NULL == dev) || (NULL == p_isid))
    {
        ERR_PRINTF("[%s %d](NULL == dev) or (NULL == p_isid)\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3501h_private *) dev->priv;
    if (NULL == priv)
    {
    	ERR_PRINTF("[%s %d](NULL == priv)\r\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
    }

    
    ACM_DEBUG_PRINTF("Run to %s, change_type = %d\r\n", __FUNCTION__, change_type);       

    // clear isid num
    p_isid->isid_num = 0;
    
    // Check TP type
    nim_c3501h_reg_get_work_mode(dev, &work_mode);
    if(1 != work_mode)
    {
        ACM_DEBUG_PRINTF("In %s exit with TP is not DVB-S2\r\n", __FUNCTION__);		
        //return SUCCESS;//because only dvbs2 need get isid, so other mode return success. 
        return ERR_FAILED;
    }    

    // In DVBS2 & ACM & Mult stream mode, we need set isid to hardware by software in auto mode
    if(SUCCESS != nim_c3501h_get_bb_header_info(dev))
    {
        //data = 0x1f;
        //nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1
        ACM_DEBUG_PRINTF("[%s %d], Get bbheader fail!\r\n", __FUNCTION__, __LINE__);
        return ERR_CRC_HEADER; 
    }

    // Check stream type
    if(NIM_STREAM_MULTIPLE != priv->bb_header_info->stream_type)
    {
        ACM_DEBUG_PRINTF("[%s %d], it is not a multi stream TP!\r\n", __FUNCTION__, __LINE__);
        return ERR_FAILED; 
    }


    if(3 == change_type)// Set TP and get ISID
    {
        if(SUCCESS != nim_c3501h_get_isid(dev, p_isid))
        {
        	ACM_DEBUG_PRINTF("[%s %d]get isid error!\r\n", __FUNCTION__, __LINE__);
            return ERR_FAILED;
        }
		
        p_isid->isid_bypass = 0;
        p_isid->isid_mode = 1;
        p_isid->isid_write = p_isid->isid_read[0];//default set the first isid to demod
        if(SUCCESS != nim_c3501h_set_isid(dev, p_isid))
        {
        	ACM_DEBUG_PRINTF("[%s %d]set isid error!\r\n", __FUNCTION__, __LINE__);
            return ERR_FAILED;
        }
    }
  
    return SUCCESS;
}




/*****************************************************************************
* INT32 nim_c3501h_set_polar(struct nim_device *dev, UINT8 polar)
* Description: c3501h set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 polar
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3501h_set_polar(struct nim_device *dev, UINT8 polar)     
{
	INT32 result;
	UINT8 data = 0;

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if (priv->ext_lnb_control)
	{
		NIM_PRINTF("nim_c3501h_set_polar used ext_lnb_control function\r\n");
		NIM_MUTEX_ENTER(priv);
		result = priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_SET_POLAR, (UINT32) polar);
        NIM_MUTEX_LEAVE(priv);
		return result;
	}

	nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);

	if ((priv->tuner_config_data.qpsk_config & C3501H_POLAR_REVERT) == 0x00) //not exist H/V polarity revert.
	{
		if (NIM_PORLAR_HORIZONTAL == polar)
		{
			data &= 0xBF;
		}
		else if (NIM_PORLAR_VERTICAL == polar)
		{
			data |= 0x40;
		}
		else
		{
			NIM_PRINTF("nim_c3501h_set_polar error\n");
			return ERR_FAILUE;
		}
	}
	else//exist H/V polarity revert.
	{
		if (NIM_PORLAR_HORIZONTAL == polar)
		{
			data |= 0x40;
		}
		else if (NIM_PORLAR_VERTICAL == polar)
		{
			data &= 0xBF;
		}
		else
		{
			NIM_PRINTF("nim_c3501h_set_polar error\n");
			return ERR_FAILUE;
		}
	}

#ifndef FPGA_PLATFORM
	if(priv->tuner_config_data.disqec_polar_position != 0)
	{
		if(data & 0x40)
			HAL_GPIO_BIT_SET(priv->tuner_config_data.disqec_polar_position, 1);
		else
			HAL_GPIO_BIT_SET(priv->tuner_config_data.disqec_polar_position, 0);
	}
	else
#endif
	{
		nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
	}

	return SUCCESS;
}

// unused
INT32 nim_c3501h_set_12v(struct nim_device *dev, UINT8 flag)       
{
	return SUCCESS;
}


/*****************************************************************************
* void nim_c3501h_FFT_set_para(struct nim_device *dev)
* Description: unused
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3501h_fft_set_para(struct nim_device *dev)
{
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_get_bypass_buffer(struct nim_device *dev)
* Description: Get bypass buffer for autoscan, we need the demux buffer to store rxadc data for fft 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_bypass_buffer(struct nim_device *dev)
{
	// According to DMX IC spec, bypass buffer must locate in 8MB memory segment.
	// Bypass buffer size is (BYPASS_BUFFER_REG_SIZE * 188)

	UINT32 tmp;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

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


static UINT8 va_22k = 0x00;
/*****************************************************************************
* INT32 nim_c3501h_diseqc_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt)  
* Description: For diseqc
*               
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8 *cmd
*  Parameter4: UINT8 cnt
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_diseqc_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt)   
{
	UINT8 data, temp, rec_22k = 0;
	UINT16 timeout, timer;
	UINT8 i;
    
	NIM_PRINTF("[%s %d]mode = 0x%d\r\n", __FUNCTION__, __LINE__, mode);

	switch (mode)
	{
	case NIM_DISEQC_MODE_22KOFF:
	case NIM_DISEQC_MODE_22KON:
		nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		data = ((data & 0xF8) | mode);
		nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		break;
	case NIM_DISEQC_MODE_BURST0:
		nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        rec_22k = data;
		//tone burst 0
		temp = 0x02;
		data = ((data & 0xF8) | temp);
		nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		// For combined transmission,wait burst finished then do another thing by paladin 2016/03/08
        //comm_sleep(16); 
        comm_sleep(40); 
        data = (rec_22k & 0xf9); // clear diseqc FSM
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		break;
	case NIM_DISEQC_MODE_BURST1:
		nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        rec_22k = data;
		//tone bust 1
		temp = 0x03;
		data = ((data & 0xF8) | temp);
		nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		// For combined transmission,wait burst finished then do another thing by paladin 2016/03/08
        //comm_sleep(16); 
        comm_sleep(40); 
        data = (rec_22k & 0xf9); // clear diseqc FSM
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
		break;
	case NIM_DISEQC_MODE_BYTES:
        comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
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
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        }
        //write the tx data, max 8 byte
        for (i = 0; i < cnt; i++)
        {
            NIM_PRINTF("Diseqc write[%d]:0x%x\n", i, *(cmd + i));
            nim_reg_write(dev, (i + C3501H_R34_DISEQC_WRITE_DATA_L), cmd + i, 1);
        }
	 // remove clean interrupt, since reg7d is read only
        //write the control bits, start TX
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);    
        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, C3501H_R20_CAPTURE_DISEQC_RPT + 0x02, &data, 1);
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
            nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
            return  SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
            return ERR_FAILED;
        }
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP1:
        comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
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
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        }

        //write the data
        for (i = 0; i < cnt; i++)
        {
            nim_reg_write(dev, (i + C3501H_R34_DISEQC_WRITE_DATA_L), cmd + i, 1);
        }
	 // remove clean interrupt, since reg7d is read only
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP2:
        //TX start : Send byte
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);   
        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, C3501H_R20_CAPTURE_DISEQC_RPT + 0x02, &data, 1);
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
            nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
            
            return SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);            
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
            
            return ERR_FAILED;
        }
        break;

	case NIM_DISEQC_MODE_ENVELOP_ON:
		{
			nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG + 1, &data, 1);
			data |= 0x02;
			nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG + 1, &data, 1);
		}
		break;
	case NIM_DISEQC_MODE_ENVELOP_OFF:
		{
			nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG + 1, &data, 1);
			data &= 0xfd;
			nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG + 1, &data, 1);
		}
		break;
	default :
		break;
	}


	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
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
INT32 nim_c3501h_diseqc2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt)
{
    INT32 result;
    UINT8 data, temp;
    UINT16 timeout, timer;
    UINT8 i;
    UINT8 need_reply = 0;// 0:No reply required  1:reply required 

	NIM_PRINTF("\t\t Enter Function nim_c3501h_diseqc2x_operate mode = %d\n", mode);
	
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
            nim_reg_write(dev, (i + C3501H_R34_DISEQC_WRITE_DATA_L), cmd + i, 1);
        }

        //set diseqc data counter
        temp = cnt - 1;
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
        va_22k=(data & 0x41);      // get DISEQC_22k origianl value
        data = ((data & 0x47) | (temp << 3));
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);

        //enable diseqc interrupt mask event bit.
        nim_c3501h_set_interrupt_mask(dev, 0xff);

        //clear co-responding diseqc interrupt event bit.
        nim_c3501h_clear_interrupt(dev);

		NIM_PRINTF("\t\t Diseqc2.X:need_reply:%d\n", need_reply);
        //write the control bits, need reply
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);

        if(need_reply == 1)
        	temp = 0x84;
        else
        	temp = 0x04;
        	
        data = ((data & 0x78) | temp);
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
         //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1); 

        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt + 200; 
        data = 0;

        //check diseqc interrupt state.
        while (timer < timeout)
        {
            nim_c3501h_get_interrupt(dev, &data);
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
            nim_reg_read(dev, C3501H_R20_CAPTURE_DISEQC_RPT + 2, &data, 1);

            switch (data & 0x07)
            {
            case 1:
                *rt_cnt = (UINT8) ((data >> 4) & 0x0f);
                NIM_PRINTF("\t\t DISEQC2.X:rt_cnt :%d\n", *rt_cnt);
				if (*rt_cnt > 0)
                {
                    for (i = 0; i < *rt_cnt; i++)
                    {
                        nim_reg_read(dev, (i + C3501H_R3C_DISEQC_READ_DATA_L), (rt_value + i), 1);
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
        nim_reg_read(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);
	    data = (data&0xb8) | va_22k;
        nim_reg_write(dev, C3501H_R24_DISEQC_MODE_REG, &data, 1);

        return result;

    default :
        break;
    }

	comm_sleep(1000);

	return SUCCESS;
}



/*****************************************************************************
*  INT32 nim_c3501h_set_pl_cfe_dis_pilot(struct nim_device *dev)
* Description: For plsn search   
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_pl_cfe_dis_pilot(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_R80_PL_CFG_1, &data, 1);
	data &= 0xfe;
	nim_reg_write(dev, C3501H_R80_PL_CFG_1, &data, 1);

	NIM_PRINTF("[%s %d]: disable pilot in PL_CFE \r\n", __FUNCTION__, __LINE__);

	return 0;
}

/*****************************************************************************
*  INT32 nim_c3501h_set_pl_cfe_en_pilot(struct nim_device *dev)
* Description: For plsn search      
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_pl_cfe_en_pilot(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_R80_PL_CFG_1, &data, 1);
	data |= 0x01;
	nim_reg_write(dev, C3501H_R80_PL_CFG_1, &data, 1);

	NIM_PRINTF("[%s %d]: enable pilot in PL_CFE \r\n", __FUNCTION__, __LINE__);

	return 0;
}

/*****************************************************************************
*  INT32 nim_c3501h_set_set_cr_dis_pilot(struct nim_device *dev)
* Description: For plsn search     
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_set_cr_dis_pilot(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_RA8_CR_CFG_2, &data, 1);
	data |= 0x40;
	nim_reg_write(dev, C3501H_RA8_CR_CFG_2, &data, 1);

	NIM_PRINTF("[%s %d]: disable pilot in CR \r\n", __FUNCTION__, __LINE__);

	return 0;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_set_cr_en_pilot(struct nim_device *dev)
* Description: For plsn search     
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_set_cr_en_pilot(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_RA8_CR_CFG_2, &data, 1);
	data &= 0xbf;
	nim_reg_write(dev, C3501H_RA8_CR_CFG_2, &data, 1);

	NIM_PRINTF("[%s %d]: enable pilot in CR \r\n", __FUNCTION__, __LINE__);

	return 0;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_work_mode(struct nim_device *dev, UINT8 work_mode)
*   Set work mode,  such as DVBS/DVBS2...
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 work_mode, 0:DVBS; 1:DVBS2; 2:DSS; 3:AUTO
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_work_mode(struct nim_device *dev, UINT8 work_mode)
{
	UINT8 data;
	NIM_PRINTF("Enter function: %s , work_mode = %d\r\n",__FUNCTION__, work_mode);

	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
    data &= 0xcf;
    data |= ((work_mode & 0x03) << 4);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode)
*   Set sub work mode,  such as ACM/CCM...
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 sub_work_mode, 0:ACM; 1:CCM; 2:CCM DUMMY; 3:AUTO
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode)
{
	UINT8 data;
	
	NIM_PRINTF("Enter function: %s , sub_work_mode = %d\r\n",__FUNCTION__, sub_work_mode);
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	data &= 0x3f;
	data |= ((sub_work_mode & 0x03) << 6);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_retry(struct nim_device *dev, UINT8 sub_work_mode)
*   Set top fsm re try flag, it set this bit, top fsm will retry to lock tp while it's unlock
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 retry, 0:undo; 1:always try
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_retry(struct nim_device *dev, UINT8 retry)
{
	UINT8 data;
	NIM_PRINTF("Enter function: %s , retry = %d\r\n",__FUNCTION__, retry);

	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	data &= 0xfb;
	data |= ((retry & 0x01) << 2);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_iq_swap(struct nim_device *dev, UINT8 work_mode)
*   Set default iq swap mode, different with RXADC iq swap
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 default_iqswap, 1: swap, 0: normal
*  Parameter3: UINT8 search_en, 1: enale, 0: disable
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_iq_swap(struct nim_device *dev, UINT8 default_iqswap, UINT8 search_en)
{
	UINT8 data = 0;

	NIM_PRINTF("Enter function: %s , default_iqswap = %d, search_en = %d\r\n",__FUNCTION__, default_iqswap, search_en);
	
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1 + 1, &data, 1);
	data &= 0xfe;
	data |= (default_iqswap & 0x01);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1 + 1, &data, 1);

	nim_reg_read(dev, C3501H_R0C_TOP_FSM_CFG_4 + 2, &data, 1);
	data &= 0xfb;
	data |= ((search_en & 0x01) << 2);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1 + 2, &data, 1);

	return SUCCESS;
}


// ================   hyman init cr adapt tab begin  ======================
//S2_AWGN_NOISE  S2_AWGN_COEF_SET  S2_PON_IRS_DELTA  S2_PON_PRS_DELTA  S2_LOOP_COEF_SET  S2_CLIP_PED_SET  S2_AVG_PED_SET  S2_FORCE_OLD_CR
//  10              4                   3                  3                  4                  3            3                 1
//rs < 6.5M
int S2_CODE_RATE_TAB_LOW[75][8] ={
    {0x2d0,   0x0,   0x0,   0x0,   0x2,   0x3/*0x0*/,   0x0,   0x0},   //0, QPSK 1/4
    {0x2c6,   0x0,   0x0,   0x0,   0x2,   0x3/*0x0*/,   0x0,   0x0},   //1, QPSK 1/3
    {0x2a8,   0x1,   0x0,   0x0,   0x3,   0x3/*0x0*/,   0x0,   0x0},   //2, QPSK 2/5
    {0x280,   0x0,   0x0,   0x0,   0x4,   0x2,   0x7,   0x0},   //3, QPSK 1/2
	{0x257/*0x244*/,   0x1/*0x0*/,   0x0,   0x0,   0x5/*0x4*/,   0x2,   0x5,   0x0},   //4, QPSK 3/5
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //5, QPSK 2/3
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //6, QPSK 3/4
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //7, QPSK 4/5
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //8, QPSK 5/6
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //9, QPSK 8/9
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //10, QPSK 9/10
    //{0x117,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //11, 8PSK 3/5
    {0xeb ,   0xb,   0x0,   0x6,   0xb,   0x0,   0x0,   0x0},   //11, 8PSK 3/5
    //{0xf5 ,   0x0,   0x0,   0x0,   0x6,   0x0,   0x0,   0x0},   //12, 8PSK 2/3
    {0xf5 ,   0xa,   0x0,   0x0,   0xb,   0x0,   0x1,   0x0},   //12, 8PSK 2/3
    {0xd3 ,   0x1,   0x0,   0x0,   0x5,   0x0,   0x1,   0x0},   //13, 8PSK 3/4
    {0xb5 ,   0x1,   0x0,   0x0,   0x5,   0x0,   0x2,   0x0},   //14, 8PSK 5/6
    {0x8d ,   0x2,   0x0,   0x0,   0x7,   0x1,   0x3,   0x0},   //15, 8PSK 8/9
    {0x8a ,   0x2,   0x0,   0x0,   0x7,   0x2,   0x3,   0x0},   //16, 8PSK 9/10
    {0x6e ,   0x0,   0x0,   0x0,   0x2,   0x0,   0x0,   0x0},   //17, 16APSK 2/3
    {0x69 ,   0x0,   0x0,   0x0,   0x2,   0x0,   0x2,   0x0},   //18, 16APSK 3/4
    {0x65 ,   0x1,   0x0,   0x0,   0x0,   0x2,   0x0,   0x0},   //19, 16APSK 4/5
    {0x65 ,   0x1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0},   //20, 16APSK 5/6
    {0x56 ,   0x3,   0x0,   0x0,   0x1,   0x0,   0x2,   0x0},   //21, 16APSK 8/9
    {0x51 ,   0x3,   0x0,   0x0,   0x1,   0x0,   0x2,   0x0},   //22, 16APSK 9/10
    {0x35 ,   0x0,   0x0,   0x0,   0x0,   0x2,   0x5,   0x0},   //23, 32APSK 3/4
    {0x35 ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x5,   0x0},   //24, 32APSK 4/5
    {0x35 ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x5,   0x0},   //25, 32APSK 5/6
    {0x2f ,   0x2,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //26, 32APSK 8/9
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //27, 32APSK 9/10
    {0x2bc,   0x7,   0x0,   0x0,   0x9,   0x3,   0x7,   0x0},   //28, QPSK 13/45
    {0x2a8,   0x7,   0x0,   0x0,   0x9,   0x2,   0x7,   0x0},   //29, QPSK 9/20
    {0x258,   0x1,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //30, QPSK 11/20
    {0x12c,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //31, 8APSK_L 5/9
    {0x12c,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //32, 8APSK_L 26/45
    {0xf0 ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //33, 8PSK 23/36
    {0xe6 ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //34, 8PSK 25/36
    {0xdc ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //35, 8PSK 13/18
    {0xb6 ,   0x8,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //36, 16APSK_L 1/2
    {0xb4 ,   0x8,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //37, 16APSK_L 8/15
    {0xb0 ,   0x8,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //38, 16APSK_L 5/9
    {0x73/*0x91*/ ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //39, 16APSK 26/45
    {0x73/*0x91*/ ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //40, 16APSK 3/5
    {0xb0 ,   0x8,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //41, 16APSK_L 3/5
    {0x73/*0x8a*/ ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //42, 16APSK 28/45
    {0x73/*0x82*/ ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //43, 16APSK 23/36
    {0x8c ,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //44, 16APSK_L 2/3
    {0x73 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //45, 16APSK 25/36
    {0x73 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //46, 16APSK 13/18
    {0x6b ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x0,   0x0},   //47, 16APSK 7/9
    {0x5f ,   0x8,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0},   //48, 16APSK 77/90
    {0x46 ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //49, 32APSK_L 2/3
    {0x43 ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //50, 32APSK 32/45
    {0x41 ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //51, 32APSK 11/15
    {0x3e ,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //52, 32APSK 7/9
    {0x23 ,   0x1,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //53, 64APSK_L 32/45
    {0x1c/*0x1e*/ ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //54, 64APSK 11/15
    {0x1e ,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //55, 64APSK 7/9
    {0x1e ,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //56, 64APSK 4/5
    {0x1c ,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //57, 64APSK 5/6
    {0x2bc,   0x7,   0x0,   0x0,   0x9,   0x3,   0x7,   0x0},   //58, QPSK 11/45
    {0x2bc,   0x7,   0x0,   0x0,   0x9,   0x3,   0x7,   0x0},   //59, QPSK 4/15
    {0x2b2,   0x7,   0x0,   0x0,   0x9,   0x3,   0x7,   0x0},   //60, QPSK 14/45
    {0x27b,   0x1,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //61, QPSK 7/15
    {0x262,   0x1,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //62, QPSK 8/15
    {0x226,   0x5,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //63, QPSK 32/45
    {0x136/*0x118*/,   0x9,   0x0,   0x0,   0xa,   0x1,   0x7,   0x0},   //64, 8PSK 7/15
    {0x136/*0x118*/,   0x9,   0x0,   0x0,   0xa,   0x1,   0x7,   0x0},   //65, 8PSK 8/15
    {0xf0 ,   0x7,   0x0,   0x0,   0x9,   0x1,   0x7,   0x0},   //66, 8PSK 26/45
    {0xc8 ,   0x1,   0x0,   0x0,   0x5,   0x1,   0x7,   0x0},   //67, 8PSK 32/45
    {0xa8 ,   0x9,   0x0,   0x0,   0xa,   0x0,   0x7,   0x0},   //68, 16APSK 7/15
    {0x82 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //69, 16APSK 8/15
    {0x82 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //70, 16APSK 26/45
    {0x82 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //71, 16APSK 3/5
    {0x6e ,   0x0,   0x0,   0x0,   0x2,   0x3,   0x7,   0x0},   //72, 16APSK 32/45
    {0x46 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //73, 32APSK 2/3
    {0x44 ,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0}    //74, 32APSK 32/45
};

//S2_AWGN_NOISE  S2_AWGN_COEF_SET  S2_PON_IRS_DELTA  S2_PON_PRS_DELTA  S2_LOOP_COEF_SET  S2_CLIP_PED_SET  S2_AVG_PED_SET  S2_FORCE_OLD_CR
//  10              4                   3                  3                  4                  3            3                 1
//rs > 6.5M
int S2_CODE_RATE_TAB[75][8] ={
    {0x28a/*0x2a8*/,   0x0,   0x0,   0x0,   0x0,   0x3/*0x2*/,   0x7,   0x0},   //0, QPSK 1/4
    {0x28a/*0x2a8*/,   0x0,   0x0,   0x0,   0x0,   0x3/*0x2*/,   0x7,   0x0},   //1, QPSK 1/3
    {0x28a/*0x2a8*/,   0x0,   0x0,   0x0,   0x0,   0x3/*0x2*/,   0x7,   0x0},   //2, QPSK 2/5
    {0x271/*0x28a*/,   0x0,   0x0,   0x0,   0x0,   0x2,   0x7,   0x0},   //3, QPSK 1/2
#ifdef FOR_SKEW_LNB_TEST_QPSK
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //4, QPSK 3/5
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //5, QPSK 2/3
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //6, QPSK 3/4
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //7, QPSK 4/5
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //8, QPSK 5/6
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //9, QPSK 8/9
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //10, QPSK 9/10 
 #else
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //4, QPSK 3/5
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //5, QPSK 2/3
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //6, QPSK 3/4
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //7, QPSK 4/5
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //8, QPSK 5/6
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //9, QPSK 8/9
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //10, QPSK 9/10
 #endif
 #ifdef FOR_SKEW_LNB_TEST_8PSK
    {0xfa ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //11, 8PSK 3/5
    {0xe6 ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //12, 8PSK 2/3
    {0xc8 ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //13, 8PSK 3/4
    {0xad ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //14, 8PSK 5/6
    {0xad ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //15, 8PSK 8/9
    {0xad ,   0x4,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //16, 8PSK 9/10
 #else
    {0xfa/*0x113*/,   0x4,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //11, 8PSK 3/5
    {0xe6/*0xfa */,   0x4,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //12, 8PSK 2/3
    {0xc8/*0xd7 */,   0x2,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //13, 8PSK 3/4
    {0xad/*0xb6 */,   0x1,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //14, 8PSK 5/6
    {0x0  ,   0x5,   0x0,   0x0,   0x1,   0x1,   0x7,   0x0},   //15, 8PSK 8/9
    {0x0  ,   0x5,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //16, 8PSK 9/10
 #endif
    {0x78/*0x82*/ ,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //17, 16APSK 2/3
    {0x70/*0x78*/ ,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //18, 16APSK 3/4
    {0x0  ,   0x4,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //19, 16APSK 4/5
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //20, 16APSK 5/6
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //21, 16APSK 8/9
    {0x0  ,   0x2,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //22, 16APSK 9/10
    {0x3c/*0x41*/ ,   0x0,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //23, 32APSK 3/4
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //24, 32APSK 4/5
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //25, 32APSK 5/6
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //26, 32APSK 8/9
    {0x0  ,   0x6,   0x0,   0x0,   0x1,   0x0,   0x7,   0x0},   //27, 32APSK 9/10
    {0x28a/*0x2a8*/,   0x7,   0x0,   0x0,   0x5,   0x3,   0x7,   0x0},   //28, QPSK 13/45
    {0x28a/*0x2a8*/,   0x7,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //29, QPSK 9/20
    {0x267/*0x285*/,   0x6,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //30, QPSK 11/20
    {0x137/*0x145*/,   0x0,   0x0,   0x0,   0x0,   0x2,   0x7,   0x0},   //31, 8APSK_L 5/9
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //32, 8APSK_L 26/45
    {0xe1 /*0xf0 */,   0x0,   0x0,   0x0,   0x0,   0x3,   0x7,   0x0},   //33, 8PSK 23/36
    {0xdc /*0xe6 */,   0x0,   0x0,   0x0,   0x0,   0x2,   0x7,   0x0},   //34, 8PSK 25/36
    {0xcf /*0xdc */,   0x0,   0x0,   0x0,   0x0,   0x2,   0x7,   0x0},   //35, 8PSK 13/18
    {0xaf /*0xb7 */,   0x5,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //36, 16APSK_L 1/2
    {0xaf /*0xb7 */,   0x5,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //37, 16APSK_L 8/15
    {0xad /*0xb5 */,   0x5,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //38, 16APSK_L 5/9
    {0x85 /*0x8c */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //39, 16APSK 26/45
    {0x85 /*0x8c */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //40, 16APSK 3/5
    {0xae /*0xb6 */,   0x0,   0x0,   0x0,   0x0,   0x2,   0x7,   0x0},   //41, 16APSK_L 3/5
    {0x7d /*0x8a */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //42, 16APSK 28/45
    {0x7d /*0x88 */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //43, 16APSK 23/36
    {0x8c /*0x93 */,   0x1,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //44, 16APSK_L 2/3
    {0x7c /*0x87 */,   0x6,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //45, 16APSK 25/36
    {0x78 /*0x80 */,   0x6,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //46, 16APSK 13/18
    {0x6a /*0x72 */,   0x1,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //47, 16APSK 7/9
    {0x50 /*0x55 */,   0x8,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //48, 16APSK 77/90
    {0x43 /*0x48 */,   0x0,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //49, 32APSK_L 2/3
    {0x43 /*0x48 */,   0x0,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //50, 32APSK 32/45
    {0x2d /*0x32 */,   0x1,   0x0,   0x0,   0x7,   0x0,   0x7,   0x0},   //51, 32APSK 11/15
    {0x3b /*0x3f */,   0x0,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //52, 32APSK 7/9
    {0x28 /*0x26 */,   0x1,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //53, 64APSK_L 32/45
    {0x1b /*0x21 */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //54, 64APSK 11/15
    {0x1f /*0x21 */,   0x1,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0},   //55, 64APSK 7/9
    {0x20 /*0x21 */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //56, 64APSK 4/5
    {0x1d /*0x1e */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //57, 64APSK 5/6
    {0x28a/*0x2a8*/,   0x7,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //58, QPSK 11/45
    {0x28a/*0x2a8*/,   0x7,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //59, QPSK 4/15
    {0x28a/*0x2a8*/,   0x7,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //60, QPSK 14/45
    {0x276/*0x294*/,   0x7,   0x0,   0x0,   0x5,   0x2,   0x7,   0x0},   //61, QPSK 7/15
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //62, QPSK 8/15
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //63, QPSK 32/45
    {0x113/*0x12c*/,   0x9,   0x0,   0x0,   0x8,   0x2,   0x7,   0x0},   //64, 8PSK 7/15
    {0x113/*0x12c*/,   0x9,   0x0,   0x0,   0x8,   0x2,   0x7,   0x0},   //65, 8PSK 8/15
    {0xf5 /*0x104*/,   0x0,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //66, 8PSK 26/45
    {0xd2 /*0xe1 */,   0x1,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //67, 8PSK 32/45
    {0x82 /*0x8c */,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //68, 16APSK 7/15
    {0x82 /*0x8c */,   0x7,   0x0,   0x0,   0x9,   0x0,   0x7,   0x0},   //69, 16APSK 8/15
    {0x82 /*0x8c */,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //70, 16APSK 26/45
    {0x82 /*0x8c */,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //71, 16APSK 3/5
    {0x74 /*0x7b */,   0x0,   0x0,   0x0,   0x2,   0x0,   0x7,   0x0},   //72, 16APSK 32/45
    {0x42 /*0x46 */,   0x7,   0x0,   0x0,   0x5,   0x0,   0x7,   0x0},   //73, 32APSK 2/3
    {0x3e /*0x42 */,   0x0,   0x0,   0x0,   0x0,   0x0,   0x7,   0x0}    //74, 32APSK 32/45
};

//S2_AWGN_NOISE  S2_AWGN_COEF_SET  S2_PON_IRS_DELTA  S2_PON_PRS_DELTA  S2_LOOP_COEF_SET  S2_CLIP_PED_SET  S2_AVG_PED_SET  S2_FORCE_OLD_CR
//  10              4                   3                  3                  4                  3            3                 1
//rs < 6.5M
int S_CODE_RATE_TAB_LOW[6][8] = {
    //{0x244,   0x6,   0x0,   0x0,   0x7,   0x2,   0x7,   0x0},   //QPSK 1/2
    {0x244,   0x6,   0x0,   0x0,   0x4,   0x2,   0x7,   0x0},   //0, QPSK 1/2
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //1, QPSK 2/3
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //2, QPSK 3/4
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //3, QPSK 5/6
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},   //4, QPSK 6/7
    {0x0  ,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0}    //5, QPSK 7/8
};

//S2_AWGN_NOISE  S2_AWGN_COEF_SET  S2_PON_IRS_DELTA  S2_PON_PRS_DELTA  S2_LOOP_COEF_SET  S2_CLIP_PED_SET  S2_AVG_PED_SET  S2_FORCE_OLD_CR
//  10              4                   3                  3                  4                  3            3                 1
//rs > 6.5M
int S_CODE_RATE_TAB[6][8] = {
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},     //0, QPSK 1/2
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},     //1, QPSK 2/3
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},     //2, QPSK 3/4
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},     //3, QPSK 5/6
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0},     //4, QPSK 6/7
    {0x0,   0x1,   0x0,   0x0,   0x1,   0x2,   0x7,   0x0}      //5, QPSK 7/8
};

int AWGN_COEF_TAB[6][16] = {
    {0x14,  0x14,  0x14,  0x14,  0x14,  0x14,  0x14,  0x16,  0x14,  0x16,  0x14,   0x14,   0x0,   0x0,   0x0,   0x0},   //IRS                5
    {0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0,   0x0,   0x0,   0x0,   0x0,   0x0},   //IRS_FRACTION       2
    {0x9 ,  0x9 ,  0x8 ,  0x8 ,  0x9 ,  0x8 ,  0x9 ,  0xa ,  0x8 ,  0xc ,  0x8,   0x8,   0x0,   0x0,   0x0,   0x0},   //PRS                4
    {0x3 ,  0x0 ,  0x0 ,  0x1 ,  0x1 ,  0x2 ,  0x2 ,  0x0 ,  0x3 ,  0x0 ,  0x2,   0x3,   0x0,   0x0,   0x0,   0x0},   //PRS_FRACTION       2
    {0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x1,   0x1,   0x0,   0x0,   0x0,   0x0},   //HEAD_PED_GAIN      3
    {0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0,   0x0,   0x0,   0x0,   0x0,   0x0}    //SYMBOL_BEFORE_HEAD 1
};

int LOOP_COEF_TABLE[7][16] = {
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x2,   0x0,   0x0,   0x0,   0x0},   //HEAD_GAIN_SNR_DELTA 3
    {0x1,   0x0,   0x2,   0x2,   0x1,   0x1,   0x3,   0x1,   0x2,   0x2,   0x4,   0x3,   0x0,   0x0,   0x0,   0x0},   //MAX_SNR_DELTA       3
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x4,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0},   //IRS_STEP            3
    {0x3,   0x0,   0x3,   0x2,   0x6,   0x4,   0x2,   0x2,   0x6,   0x4,   0x4,   0x2,   0x0,   0x0,   0x0,   0x0},   //PRS_STEP            3
    {0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x7,   0x0,   0x0,   0x0,   0x0},   //WIDER_SNR_DELTA     3
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0},   //WIDER_IRS_DELTA     2
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0}    //WIDER_PRS_DELTA     2
};

int CLIP_PED_TABLE[6][8] = {
    {0x7 ,  0x7 ,  0x7,   0x0 ,  0x7,   0x7,   0x7,   0x7},   //CLIP_SNR_DELTA 3 // if ==7, then always use CLIP_PEDx_EN
    {0x0 ,  0x0 ,  0x0,   0x0 ,  0x0,   0x0,   0x0,   0x0},   //CLIP_MULT_STEP 5
    {0x1 ,  0x1 ,  0x0,   0x1 ,  0x0,   0x0,   0x0,   0x0},   //CLIP_PED1_EN   1
    {0x37,  0x90,  0x0,   0x37,  0x0,   0x0,   0x0,   0x0},   //CLIP_PED1      8
    {0x1 ,  0x1 ,  0x0,   0x1 ,  0x0,   0x0,   0x0,   0x0},   //CLIP_PED2_EN   1
    {0x73,  0x80,  0x0,   0x73,  0x0,   0x0,   0x0,   0x0}    //CLIP_PED2      8
};

int AVG_PED_TABLE[6][8] = {
    {0x1 ,  0x1 ,  0x0 ,  0x0 ,  0x0,   0x0 ,  0x0,  0x0},    //AVG_SNR_DELTA  3
    {0x0 ,  0x0 ,  0x0 ,  0x0 ,  0x0,   0x0 ,  0x0,  0x0},    //AVG_MULT_STEP  5
    {0x1 ,  0x1 ,  0x1 ,  0x1 ,  0x0,   0x1 ,  0x0,  0x0},    //AVG_PED1_EN    1
    {0x96,  0x30,  0xe0,  0x0a,  0x0,   0x96,  0x0,  0x0},    //AVG_PED1       8
    {0x1 ,  0x1 ,  0x1 ,  0x1 ,  0x0,   0x1 ,  0x0,  0x0},    //AVG_PED2_en    1
    {0x32,  0x18,  0x70,  0x05,  0x0,   0x32,  0x0,  0x0}     //AVG_PED2       8
}; 

void nim_c3501h_cr_write_adpt_tab(struct nim_device *dev, int TAB_ADDR, int TAB_ITEM)
{
    UINT8  DatArray[4];

    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);

    //write addr first ,then write data
    DatArray[0] =  (TAB_ADDR     & 0xff);
    DatArray[1] = ((TAB_ADDR>>8) & 0xff);
    nim_reg_write(dev, C3501H_RC4_CR_ADPT_CF_RPT, DatArray, 2);
    
    DatArray[0] =  (TAB_ITEM      & 0xff);
    DatArray[1] = ((TAB_ITEM>>8)  & 0xff);
    DatArray[2] = ((TAB_ITEM>>16) & 0xff);
    DatArray[3] = ((TAB_ITEM>>24) & 0xff);
    nim_reg_write(dev, C3501H_RC0_CR_ADPT_CF, DatArray, 3);
    
    //i am not quite sure the sequence of nim_reg_write when write muti bytes,
    //so i write DatArray[3] at last.
    DatArray[3] = (1<<7) | DatArray[3];   //CR_ADPT_PARA_WE
    nim_reg_write(dev, C3501H_RC0_CR_ADPT_CF + 3, &(DatArray[3]), 1); //make sure CR_ADPT_PARA_WE write at last
}

 INT32 nim_c3501h_cr_adpt_tab_init(struct nim_device *dev)
{
	int i;

    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);

    // cr adaptive table reconfig, different from default value.
    // (IC noise thresh different from fpga)
    nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 11); // 8PSK 3/5 5M
    nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 12); // 8PSK 2/3 5M
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 0 ); //QPSK 1/4
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 1 ); //QPSK 1/3
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 2 ); //QPSK 2/5
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 3 ); //QPSK 1/2
#ifdef FOR_SKEW_LNB_TEST_QPSK
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 4); //QPSK 3/5
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 5); //QPSK 2/3
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 6); //QPSK 3/4
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 7); //QPSK 4/5
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 8); //QPSK 5/6
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 9); //QPSK 8/9
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 10); //QPSK 9/10
#endif
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 11); //8PSK 3/5
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 12); //8PSK 2/3
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 13); //8PSK 3/4
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 14); //8PSK 5/6
#ifdef FOR_SKEW_LNB_TEST_8PSK
	nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 15); //8PSK 8/9
	nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 16); //8PSK 9/10
#endif
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 17); //16APSK 2/3
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 18); //16APSK 3/4
    nim_c3501h_cr_write_adpt_s2_coderatetab(dev, 23); //32APSK 3/4

    nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 39); // s2x 16APSK 26/45 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 40); // s2x 16APSK 3/5 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 42); // s2x 16APSK 28/45 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 43); // s2x 16APSK 23/36 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 54); // s2x 64APSK 11/15 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 64); // s2x 8PSK 7/15 5M
	nim_c3501h_cr_write_adpt_s2_coderatetablow(dev, 65); // s2x 8PSK 8/15 5M

	for(i=28; i<=74;i++)
	{        
		nim_c3501h_cr_write_adpt_s2_coderatetab(dev, i ); //dvbs2x 13m    
	}
	
    nim_c3501h_cr_write_adpt_awgncoeftab(dev, 10); // add new table item
	nim_c3501h_cr_write_adpt_awgncoeftab(dev, 11); // add new table item
	nim_c3501h_cr_write_adpt_loopcoeftab(dev, 11); // add new table item

	return SUCCESS;
}

void nim_c3501h_cr_write_adpt_s2_coderatetab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    UINT32 temp;
    
    //S2_CODE_RATE_TAB
    TAB_ID   = 0;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][0] & 0x3ff;  //AWGN_NOISE 10
    TAB_ITEM = temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][1] & 0xf;  //AWGN_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][2] & 0x7;  //S2_PON_IRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][3] & 0x7;  //S2_PON_PRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][4] & 0xf;  //S2_LOOP_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][5] & 0x7;  //S2_CLIP_PED_SET  3
    TAB_ITEM = (TAB_ITEM<<3)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][6] & 0x7;  //S2_AVG_PED_SET   3
    TAB_ITEM = (TAB_ITEM<<3)| temp;
    
    temp = S2_CODE_RATE_TAB[TAB_CELL][7] & 0x1;  //S2_FORCE_OLD_CR  1
    TAB_ITEM = (TAB_ITEM<<1)| temp;
    
    //libc_printf("[%s],line[%d], TAB_ADDR = %d, TAB_ITEM = %d\n", __FUNCTION__, __LINE__, TAB_ADDR, TAB_ITEM);
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);
    
}

void nim_c3501h_cr_write_adpt_s2_coderatetablow(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //S2_CODE_RATE_TAB_LOW
    TAB_ID   = 6;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][0] & 0x3ff;  //AWGN_NOISE 10
    TAB_ITEM = temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][1] & 0xf;  //AWGN_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][2] & 0x7;  //S2_PON_IRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][3] & 0x7;  //S2_PON_PRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][4] & 0xf;  //S2_LOOP_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][5] & 0x7;  //S2_CLIP_PED_SET  3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][6] & 0x7;  //S2_AVG_PED_SET   3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S2_CODE_RATE_TAB_LOW[TAB_CELL][7] & 0x1;  //S2_FORCE_OLD_CR  1
    TAB_ITEM = (TAB_ITEM<<1)| temp;

    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

void nim_c3501h_cr_write_adpt_s_coderatetab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //S_CODE_RATE_TAB
    TAB_ID   = 5;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = S_CODE_RATE_TAB[TAB_CELL][0] & 0x3ff;  //AWGN_NOISE 10
    TAB_ITEM = temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][1] & 0xf;  //AWGN_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][2] & 0x7;  //S2_PON_IRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][3] & 0x7;  //S2_PON_PRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][4] & 0xf;  //S2_LOOP_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][5] & 0x7;  //S2_CLIP_PED_SET  3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][6] & 0x7;  //S2_AVG_PED_SET   3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB[TAB_CELL][7] & 0x1;  //S2_FORCE_OLD_CR  1
    TAB_ITEM = (TAB_ITEM<<1)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

void nim_c3501h_cr_write_adpt_s_coderatetab_low(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //S_CODE_RATE_TAB
    TAB_ID   = 7;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][0] & 0x3ff;  //AWGN_NOISE 10
    TAB_ITEM = temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][1] & 0xf;  //AWGN_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][2] & 0x7;  //S2_PON_IRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][3] & 0x7;  //S2_PON_PRS_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][4] & 0xf;  //S2_LOOP_COEF_SET 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][5] & 0x7;  //S2_CLIP_PED_SET  3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][6] & 0x7;  //S2_AVG_PED_SET   3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = S_CODE_RATE_TAB_LOW[TAB_CELL][7] & 0x1;  //S2_FORCE_OLD_CR  1
    TAB_ITEM = (TAB_ITEM<<1)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);
}

void nim_c3501h_cr_write_adpt_awgncoeftab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //AWGN_COEF_TAB
    TAB_ID   = 1;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = AWGN_COEF_TAB[0][TAB_CELL] & 0x1f;  //IRS 5
    TAB_ITEM = temp;

    temp = AWGN_COEF_TAB[1][TAB_CELL] & 0x3;  //IRS_FRACTION 2
    TAB_ITEM = (TAB_ITEM<<2)| temp;

    temp = AWGN_COEF_TAB[2][TAB_CELL] & 0xf;  //PRS 4
    TAB_ITEM = (TAB_ITEM<<4)| temp;

    temp = AWGN_COEF_TAB[3][TAB_CELL] & 0x3;  //PRS_FRACTION 2
    TAB_ITEM = (TAB_ITEM<<2)| temp;

    temp = AWGN_COEF_TAB[4][TAB_CELL] & 0x7;  //HEAD_PED_GAIN 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = AWGN_COEF_TAB[5][TAB_CELL] & 0x1;  //SYMBOL_BEFORE_HEAD  1
    TAB_ITEM = (TAB_ITEM<<1)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

void nim_c3501h_cr_write_adpt_loopcoeftab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //LOOP_COEF_TABLE
    TAB_ID   = 2;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = LOOP_COEF_TABLE[0][TAB_CELL] & 0x7;  //HEAD_GAIN_SNR_DELTA 3
    TAB_ITEM = temp;

    temp = LOOP_COEF_TABLE[1][TAB_CELL] & 0x7;  //MAX_SNR_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = LOOP_COEF_TABLE[2][TAB_CELL] & 0x7;  //IRS_STEP 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = LOOP_COEF_TABLE[3][TAB_CELL] & 0x7;  //PRS_STEP 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = LOOP_COEF_TABLE[4][TAB_CELL] & 0x7;  //WIDER_SNR_DELTA 3
    TAB_ITEM = (TAB_ITEM<<3)| temp;

    temp = LOOP_COEF_TABLE[5][TAB_CELL] & 0x3;  //WIDER_IRS_DELTA  2
    TAB_ITEM = (TAB_ITEM<<2)| temp;

    temp = LOOP_COEF_TABLE[6][TAB_CELL] & 0x3;  //WIDER_PRS_DELTA  2
    TAB_ITEM = (TAB_ITEM<<2)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

void nim_c3501h_cr_write_adpt_clippedtab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //CLIP_PED_TABLE
    TAB_ID   = 3;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

	temp = CLIP_PED_TABLE[0][TAB_CELL] & 0x7;  //CLIP_SNR_DELTA 3
    TAB_ITEM = temp;

    temp = CLIP_PED_TABLE[1][TAB_CELL] & 0x1f;  //CLIP_MULT_STEP 5
    TAB_ITEM = (TAB_ITEM<<5)| temp;

    temp = CLIP_PED_TABLE[2][TAB_CELL] & 0x1;  //CLIP_PED1_EN 1
    TAB_ITEM = (TAB_ITEM<<1)| temp;

    temp = CLIP_PED_TABLE[3][TAB_CELL] & 0xff;  //CLIP_PED1 8
    TAB_ITEM = (TAB_ITEM<<8)| temp;

    temp = CLIP_PED_TABLE[4][TAB_CELL] & 0x1;  //CLIP_PED2_EN 1
    TAB_ITEM = (TAB_ITEM<<1)| temp;

    temp = CLIP_PED_TABLE[5][TAB_CELL] & 0xff;  //CLIP_PED2 8
    TAB_ITEM = (TAB_ITEM<<8)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

void nim_c3501h_cr_write_adpt_avgpedtab(struct nim_device *dev, int TAB_CELL)
{
    UINT8  TAB_ID;
    UINT16 TAB_ADDR;
    UINT32 TAB_ITEM;
    
    //libc_printf("[%s],line[%d]\n", __FUNCTION__, __LINE__);
    UINT32 temp;
    
    //AVG_PED_TABLE
    TAB_ID   = 4;
    TAB_ADDR = (TAB_ID<<7) | TAB_CELL;

    temp = AVG_PED_TABLE[0][TAB_CELL] & 0x7;  //AVG_SNR_DELTA 3
    TAB_ITEM = temp;

    temp = AVG_PED_TABLE[1][TAB_CELL] & 0x1f;   //AVG_MULT_STEP 5
    TAB_ITEM = (TAB_ITEM<<5)| temp;

    temp = AVG_PED_TABLE[2][TAB_CELL] & 0x1;  //AVG_PED1_EN 1
    TAB_ITEM = (TAB_ITEM<<1)| temp;

    temp = AVG_PED_TABLE[3][TAB_CELL] & 0xff;   //AVG_PED1 8
    TAB_ITEM = (TAB_ITEM<<8)| temp;

    temp = AVG_PED_TABLE[4][TAB_CELL] & 0x1;  //AVG_PED2_en 1
    TAB_ITEM = (TAB_ITEM<<1)| temp;

    temp = AVG_PED_TABLE[5][TAB_CELL] & 0xff;  //AVG_PED2 8
    TAB_ITEM = (TAB_ITEM<<8)| temp;
    
    nim_c3501h_cr_write_adpt_tab(dev, TAB_ADDR, TAB_ITEM);

}

// ================   hyman init cr adapt tab end  ======================


/*****************************************************************************
*  INT32 nim_c3501h_freeze_fsm(struct nim_device *dev)
*   FSM should not going when it reach target state
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 enable, 1: enable freeze fsm; 0:disable(normal)
*  Parameter3: UINT8 state, target state
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_freeze_fsm(struct nim_device *dev, UINT8 enable, UINT8 state)
{
	UINT8 data = 0;

	//NIM_PRINTF("Enter function: %s , enable = %d, status = %d\r\n",__FUNCTION__, enable, state);
	nim_reg_read(dev, C3501H_R18_TOP_FSM_RPT_2 + 3, &data, 1);
	data &= 0x80;

	if (0x01 == enable)
		data |= ((state & 0x3f) + 0x40);

	nim_reg_write(dev, C3501H_R18_TOP_FSM_RPT_2 + 3, &data, 1);

	return SUCCESS;
} 

/*****************************************************************************
*  INT32 nim_c3501h_preset_fsm(struct nim_device *dev)
*   preset fsm state according to target state, top fsm then keeps going
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 enable, 1: enable freeze fsm; 0:disable(normal)
*  Parameter3: UINT8 state, target state
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_preset_fsm(struct nim_device *dev, UINT8 enable, UINT8 state)
{
	UINT8 data = 0;

	//NIM_PRINTF("Enter function: %s , enable = %d, status = %d\r\n",__FUNCTION__, enable, state);
	nim_reg_read(dev, C3501H_R18_TOP_FSM_RPT_2 + 2, &data, 1);
	data &= 0x80;

	if (0x01 == enable)
		data |= ((state & 0x3f) + 0x40);

	nim_reg_write(dev, C3501H_R18_TOP_FSM_RPT_2 + 2, &data, 1);

	return SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_monitor.c																													COPY FROM nim_dvbs_c3505_monitor.c	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
*  INT32 nim_c3501h_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
* Description: Get tuner lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *tun_lock
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	//UINT8 data;

	/* Setup tuner */

    if (nim_c3501h_i2c_through_open(dev))
    {
        NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
        return C3501H_ERR_I2C_NO_ACK;
    }

	NIM_MUTEX_ENTER(priv);
	priv->nim_tuner_status(priv->tuner_index, tun_lock);
	NIM_MUTEX_LEAVE(priv);
    
    if (nim_c3501h_i2c_through_close(dev))
    {
        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
        return C3501H_ERR_I2C_NO_ACK;
    }

	return SUCCESS;
}


 /*****************************************************************************
 *  INT32 nim_c3501h_get_tune_freq(struct nim_device *dev, INT32 *freq)
 * Get tune freq for autoscan, unused now!
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: INT32* freq
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3501h_get_tune_freq(struct nim_device *dev, INT32 *freq)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	*freq += priv->ul_status.m_step_freq;

	if (*freq <= 0)
		*freq = 30;
	else if (*freq > 70)
		*freq = 70;

	//NIM_PRINTF("  tune *freq step is %d\n", *freq );
	return SUCCESS;
}


/*****************************************************************************
* UINT8 nim_c3501h_get_cr_num(struct nim_device *dev)
* Description: Get est tp number for autoscan, unused now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: UINT8
*****************************************************************************/
UINT8 nim_c3501h_get_cr_num(struct nim_device *dev)
{
    return ((struct nim_c3501h_private *) (dev->priv))->ul_status.m_cr_num;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_interrupt(struct nim_device *dev, UINT8 *intdata)
* Description: Get demod interrupt flag
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* intdata
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_interrupt(struct nim_device *dev, UINT8 *intdata)
{
    UINT8 data = 0;
	//NIM_PRINTF("Enter function: %s \r\n",__FUNCTION__);

	nim_reg_read(dev, C3501H_R18_TOP_FSM_RPT_2 + 1, &data, 1);
    *intdata = data;
	return SUCCESS;
}


#ifdef FC_SEARCH_RANGE_LIMITED

/*****************************************************************************
* INT32 nim_c3501h_get_lock_check_limited(struct nim_device *dev, UINT8 *lock_state)
*
*  Read FEC lock register value, And check whether the freq range meets the range, then 
*  report the lock state and open/close tso. This function needs to be called every 250ms 
*  Ensure the status is updated in time.
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *lock_state
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_lock_check_limited(struct nim_device *dev, UINT8 *lock_state)
{
    UINT8 data = 0;
    INT32 freq_offset = 0;
    UINT8 status = 0;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	
    nim_c3501h_get_lock(dev, &data);

    if (NIM_OPTR_SOFT_SEARCH == priv->search_type)
    {
        *lock_state = data;
    }
    else if (0x01 == (data & 0x01))
    {
        nim_c3501h_reg_get_freqoffset(dev, &freq_offset);
        priv->ul_status.m_real_freq = priv->cur_freq + freq_offset; // Real tuner freq + freq offset
        //NIM_PRINTF("Tuner real freq is %d, lock freq is %d, UI set freq is %d\r\n", priv->cur_freq, priv->ul_status.m_real_freq, priv->ul_status.m_setting_freq*1024);
        // Real freq offset, means real freq - ui freq
        freq_offset = priv->ul_status.m_real_freq - priv->ul_status.m_setting_freq*1024;
        //NIM_PRINTF("Freq offset acq is %d, Freq offset real is %d\r\n", priv->t_param.freq_search_range, freq_offset);

        // Freq offset real is smaller than acq, than continue lock flow
        if (((freq_offset >= 0) && (freq_offset <= priv->t_param.freq_search_range)) ||
        ((freq_offset < 0) && (freq_offset > (-priv->t_param.freq_search_range))))
        {
            *lock_state = 1;
            //NIM_PRINTF("[%s, %d] Freq offset real is smaller than acq, exit!\r\n", __FUNCTION__, __LINE__);   
            status = 1;
            if (status != priv->ul_status.lock_status)
            {
                nim_c3501h_tso_on(dev);
                //nim_c3501h_force_cur_work_mode(dev);
            }
        }
        else
        {
           *lock_state = 0;
           //NIM_PRINTF("[%s, %d] Freq offset real is bigger than acq, exit!\r\n", __FUNCTION__, __LINE__);
           status = 2;
           if (status != priv->ul_status.lock_status)
           {
               nim_c3501h_tso_off(dev);
               //nim_c3501h_set_work_mode(dev, 0x03);
           }
        }
    }   	
	else
    {   
		*lock_state = 0;
        status = 3;
       if (status != priv->ul_status.lock_status)
       {
           nim_c3501h_tso_off(dev);
           //nim_c3501h_set_work_mode(dev, 0x03);
       } 
    }

    priv->ul_status.lock_status = status;
    priv->ul_status.check_lock_time = osal_get_tick();

	return SUCCESS;
}

#endif

/*****************************************************************************
* INT32 nim_c3501h_la_p1_trg_start(struct nim_device *dev)
*
*  set LA P1 trigger start
*
*Arguments:
*  Parameter1: struct nim_device *dev
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_la_p1_trg_start(struct nim_device *dev)
{
    UINT8 data = 0;

    nim_reg_read(dev,C3501H_R44_LA_PROBE_1_CFG+3,&data,1); 
	//P1_TRG_START=0x184[25]
	// Posedge: start trigle   Negedge: stop trigle
	data |= 0x02;
	nim_reg_write(dev,C3501H_R44_LA_PROBE_1_CFG+3,&data,1);
	return SUCCESS;
		
}

/*****************************************************************************
* INT32 nim_c3501h_la_p1_trg_stop(struct nim_device *dev)
*
*  set LA P1 trigger stop
*
*Arguments:
*  Parameter1: struct nim_device *dev
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_la_p1_trg_stop(struct nim_device *dev)
{
    UINT8 data = 0;

    nim_reg_read(dev,C3501H_R44_LA_PROBE_1_CFG+3,&data,1); 
	//P1_TRG_START=0x184[25]
	// Posedge: start trigle   Negedge: stop trigle
	data &= 0xfd;
	nim_reg_write(dev,C3501H_R44_LA_PROBE_1_CFG+3,&data,1);
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3501h_la_p1_trg_done_check(struct nim_device *dev,UINT8 *bbstart)
*
*  get BBSTART signal from LA P1 TRIGGER
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *bbstart
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_la_p1_trg_done_check(struct nim_device *dev, UINT8 *bbstart)
{
	UINT8 data = 0;
	
	// P1_TRG_DONE = 0x184[26]
	nim_reg_read(dev, C3501H_R44_LA_PROBE_1_CFG+3,&data,1);
	if (0x04 == (data & 0x04))
	{
	    *bbstart = 1;
		nim_c3501h_la_p1_trg_stop(dev);
	}
	else
	{
	    *bbstart = 0;
	}

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3501h_get_lock_check_limited(struct nim_device *dev, UINT8 *lock_state)
*
*  Read FEC lock register value
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *lock_state
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_lock(struct nim_device *dev, UINT8 *lock)
{
	UINT8 data;
	
	// Bit 14:0, Lock status rpt
	//   0:    CH_LOCK
	//   1:    AGC1_LOCK 
	//   2:    DCC_LOCK
	//   3:    IQB_SETTLE
	//   4:    FLT_SETTLE
	//   5:    AGC2_LOCK
	//   6:    TR_LOCK
	//   7:    PL_SYNC_FIND
	//   8:    PL_SYNC_LOCK
	//   9:    AGC3_LOCK
	//  10:    CR_LOCK
	//  11:    EQ_DD_LOCK
	//  12:    S2_FEC_LOCK
	//  13:    VB_SYNC_SUCCESS
	//  14:    DVBS_DES_LOCK

	nim_reg_read(dev, C3501H_R14_TOP_FSM_RPT_1, &data, 1);
	
	if (0x01 == (data & 0x01))
		*lock = 1;
	else
		*lock = 0;

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_get_lock_reg(struct nim_device *dev, UINT16 *lock_state)
*
*  Read FEC lock register value
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *lock_state
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_lock_reg(struct nim_device *dev, UINT16 *lock_state)
{
	UINT8 data[2] = {0, 0};
	
	// Bit 14:0, Lock status rpt
	//   0:    CH_LOCK
	//   1:    AGC1_LOCK
	//   2:    DCC_LOCK
	//   3:    IQB_SETTLE
	//   4:    FLT_SETTLE
	//   5:    AGC2_LOCK
	//   6:    TR_LOCK
	//   7:    PL_SYNC_FIND
	//   8:    PL_SYNC_LOCK
	//   9:    AGC3_LOCK
	//  10:    CR_LOCK
	//  11:    EQ_DD_LOCK
	//  12:    S2_FEC_LOCK
	//  13:    VB_SYNC_SUCCESS
	//  14:    DVBS_DES_LOCK

	nim_reg_read(dev, C3501H_R14_TOP_FSM_RPT_1, data, 2);

	*lock_state = data[0] | (data[1] << 8);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_freqoffset(struct nim_device *dev, INT32 *freq_offset)
* Get freq offset from demod
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32* freq_offset
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_freqoffset(struct nim_device *dev, INT32 *freq_offset)
{
	UINT8 data[2];
	UINT32 sample_rate;
	UINT32 tdata;
	UINT32 temp;

    nim_c3501h_get_dsp_clk(dev, &sample_rate);

    data[0] = 0x40;
	nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &(data[0]), 1);

	nim_reg_read(dev, C3501H_R24_MODU_INFO_RPT_3 + 2, data, 2); 
	tdata = (data[0]&0xff)  + ((data[1]&0xff) <<8); // fix{1,-1,16}

	if (data[1] & 0x80)
    {   
		temp = ((tdata & 0x7fff) ^ 0x7fff) + 1;
        //NIM_PRINTF("temp = 0x%x\n", temp);
        //*freq_offset = 0 - nim_c3501h_multu64div(temp, sample_rate, 65536); // 131072 = 2^17
        *freq_offset = 0 - nim_c3501h_multu64div(temp, sample_rate, 64000); // 131072 = 2^17
    }
	else
    {   
		temp = tdata & 0x7fff;
        //NIM_PRINTF("temp = 0x%x\n", temp);
        //*freq_offset = nim_c3501h_multu64div(temp, sample_rate, 65536); // 131072 = 2^17
        *freq_offset = nim_c3501h_multu64div(temp, sample_rate, 64000); // 131072 = 2^17
    }
    //NIM_PRINTF("nim_c3501h_reg_get_freq offset Normal freq_offset = %d\n", *freq_offset);
    // While DVB-S2 & IQ SWAP the rpt freq offset need negation
    nim_c3501h_reg_get_work_mode(dev, &(data[0]));
    if (1 == data[0])
    {
        nim_c3501h_reg_get_iqswap_flag(dev, &(data[0]));
        if (1 == data[0])
        {
            *freq_offset = -*freq_offset;            
            //NIM_PRINTF("nim_c3501h_reg_get_freq offset DVB-S2 & IQ SWAP freq_offset = %d\n", *freq_offset);
        }
    }
    
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_get_freq(struct nim_device *dev, UINT32 *freq)
* Get TP's real freq by *freq + freq offset
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_freq(struct nim_device *dev, UINT32 *freq)
{
	UINT8 data[2];

	INT32 freq_off;
	UINT32 sample_rate;
	UINT32 tdata;
	UINT32 temp;

    nim_c3501h_get_dsp_clk(dev, &sample_rate);

    data[0] = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &(data[0]), 1);

	nim_reg_read(dev, C3501H_R24_MODU_INFO_RPT_3 + 2, data, 2); 
	tdata = (data[0]&0xff)  + ((data[1]&0xff) <<8); // fix{1,-1,16}
    
    //NIM_PRINTF("%s, data_1 = 0x%2x, data_0 = 0x%2x\r\n", __FUNCTION__, data[1], data[0]);

	if (data[1] & 0x80)
    {   
		temp = ((tdata & 0x7fff) ^ 0x7fff) + 1;
        //NIM_PRINTF("temp = 0x%x\n", temp);
        //freq_off = 0 - nim_c3501h_multu64div(temp, sample_rate, 65536); // 131072 = 2^17
        freq_off = 0 - nim_c3501h_multu64div(temp, sample_rate, 64000); // 131072 = 2^17
    }
	else
    {   
		temp = tdata & 0x7fff;
        //NIM_PRINTF("temp = 0x%x\n", temp);
        //freq_off = nim_c3501h_multu64div(temp, sample_rate, 65536); // 131072 = 2^17
        freq_off = nim_c3501h_multu64div(temp, sample_rate, 64000); // 131072 = 2^17
    }
    //NIM_PRINTF("nim_c3501h_get_freq Normal freq_off = %d\n", freq_off);

     // While DVB-S2 & IQ SWAP the rpt freq offset need negation
     nim_c3501h_reg_get_work_mode(dev, &(data[0]));
     if (1 == data[0])
     {
         nim_c3501h_reg_get_iqswap_flag(dev, &(data[0]));
         if (1 == data[0])
         {
             freq_off = -freq_off;            
             //NIM_PRINTF("nim_c3501h_get_freq DVB-S2 & IQ SWAP freq_off = %d\n", freq_off);
         }
     }
     
    *freq += freq_off;
    
    //NIM_PRINTF("%s, freq offset = 0x%x, freq output = 0x%x\r\n", __FUNCTION__, freq_off, *freq);
    
	return 0;
}


/*****************************************************************************
* INT32 nim_c3501h_get_cur_freq(struct nim_device *dev, UINT32 *freq)
* Read TP's real freq from driver to upper layer, so ul_status.m_cur_freq needs update while tp locked
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32* freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_cur_freq(struct nim_device *dev, UINT32 *freq)
{
	 if(dev == NULL)
        return RET_FAILURE;
	 
    *freq = ((struct nim_c3501h_private *) dev->priv)->ul_status.m_real_freq/1024;
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
* Read TP's real freq from driver to upper layer
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
{
	UINT8 data = 0;
	// Bit 13:8 is rpt code rate
	//Code rate reported by demodulator
	//Code rate list
	//for DVBS:
	//	0x0: 1/2,
	//	0x1: 2/3,
	//	0x2: 3/4,
	//	0x3: 5/6,
	//	0x4: ,
	//	0x5: 7/8.
	//For DVBS2 :
	//	0x0: 1/4 ,
	//	0x1: 1/3 ,
	//	0x2: 2/5 ,
	//	0x3: 1/2 ,
	//	0x4: 3/5 ,
	//	0x5: 2/3 ,
	//	0x6: 3/4 ,
	//	0x7: 4/5 ,
	//	0x8: 5/6 ,
	//	0x9: 8/9 ,
	//	0xa: 9/10.

	//NIM_PRINTF("Enter function: %s\r\n",__FUNCTION__);

    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 1, &data, 1);
	data &= 0x3f;
	*code_rate = data;

	//NIM_PRINTF("rpt_code_rate = %d\r\n", *code_rate);

	return SUCCESS;
}

INT32 nim_c3501h_get_snr(struct nim_device *dev, UINT8 *snr)
{
	INT16 snr_db = 0;

    nim_c3501h_get_snr_db(dev, &snr_db);
    
    if (snr_db < 0)
        snr_db = 0;

    snr_db = snr_db / 10;

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

    return SUCCESS;
}

INT32 nim_c3501h_get_snr_db(struct nim_device *dev, INT16 *snr_db)
{
	UINT8 lock = 0, data = 0;
	UINT8 snir[2] ;
    UINT16 snr_factor = 32;
    INT32 snr_tmp = 0;

	if (NULL == snr_db)
		return ERR_FAILED;

    nim_c3501h_get_lock(dev, &lock);

    if (0 == lock)
    {
        *snr_db = 0;
        return SUCCESS;
    }

	nim_reg_read(dev, C3501H_R50_FLT_SNIR + 1, &data, 1);
	data |= 0x08;
	nim_reg_write(dev, C3501H_R50_FLT_SNIR + 1, &data, 1 );

	nim_reg_read(dev, C3501H_R50_FLT_SNIR + 1, snir, 2 );
	data = snir[0] & 0xf0;
	data = (data >> 4);
	snr_tmp = (snir[1] << 4) + data;

    if(snr_tmp & 0x0800)
    {
        snr_tmp =  snr_tmp - 4096;
    }

    snr_factor = 40;
    snr_tmp *= 125;

    snr_tmp = (snr_tmp + snr_factor/2 ) / snr_factor;
    *snr_db = (INT16)snr_tmp;

	return SUCCESS;
}


INT32 nim_c3501h_get_per(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 per[3];
	static UINT32 per_cnt = 1010;
	UINT8 lock = 0, data = 0, data_w = 0;

	*rs_ubc = 1010;

	nim_c3501h_get_lock(dev, &lock);
	if (1 == lock)	// Channel Lock
	{
		// Get status
		nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
		
		if(0x20&data)
		{
			data_w = data&0xdf;	//report error packet number
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);
		}
		nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
		if(0x10&data)
		{
			data_w = data&0xef;	//sta till counter full
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);
			// Clear per cnt
			nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x02;
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);
			// Restart monitor
			nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x01;
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);
		}

		// change to read per
		nim_reg_read(dev, C3501H_R2C_MON_PER_RPT+3, &data, 1);	
		data_w = data&0x7f;
		nim_reg_write(dev, C3501H_R2C_MON_PER_RPT+3, &data_w, 1);

		// Get result
		nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
		data_w = data & 0xf8;
		data_w |= 0x04;
		nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);

		nim_reg_read(dev, C3501H_R2C_MON_PER_RPT, per, 3);	
		per_cnt = per[0] + (per[1]<<8) + (per[2]<<16);
		//NIM_PRINTF("nim_c3501h_get_per = %d\r\n", per_cnt);
		if(per_cnt != 0)	// only resart when finding error packet
		{
			// Clear per cnt
			nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x02;
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);

			// Restart monitor
			nim_reg_read(dev, C3501H_R28_MON_PER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x01;
			nim_reg_write(dev, C3501H_R28_MON_PER_CFG, &data_w, 1);
		}

		*rs_ubc = per_cnt;
		return SUCCESS;
	}
	else
	{
		return ERR_FAILED;
	}

}

INT32 nim_c3501h_get_fer(struct nim_device *dev, UINT32 *rs_ubc)
{
	UINT8 fer[3];
	static UINT32 fer_cnt = 1010;
	UINT8 lock = 0, data = 0, data_w = 0;

	*rs_ubc = 1010;

	nim_c3501h_get_lock(dev, &lock);
	if (1 == lock)	// Channel Lock
	{
		// Get status
		nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
		if(0x20&data)
		{
			data_w = data&0xdf;	//report error packet number
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);
		}
		nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
		if(0x10&data)
		{
			data_w = data&0xef;	//sta till counter full
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);
			// Clear fer cnt
			nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x02;
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);
			// Restart monitor
			nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x01;
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);
		}
		// change to read fer
		nim_reg_read(dev, C3501H_R2C_MON_PER_RPT+3, &data, 1);	
		data_w = data|0x80;
		nim_reg_write(dev, C3501H_R2C_MON_PER_RPT+3, &data_w, 1);

		// Get result
		nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
		data_w = data & 0xf8;
		data_w |= 0x04;
		nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);

		nim_reg_read(dev, C3501H_R2C_MON_PER_RPT, fer, 3);	
		fer_cnt = fer[0] + (fer[1]<<8) + (fer[2]<<16);
		//NIM_PRINTF("nim_c3501h_get_fer = %d\r\n", fer_cnt);
		if(fer_cnt != 0)	// only resart when finding error packet
		{
			// Clear fer cnt
			nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x02;
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);

			// Restart monitor
			nim_reg_read(dev, C3501H_R30_MON_FER_CFG, &data, 1);	
			data_w = data & 0xf8;
			data_w |= 0x01;
			nim_reg_write(dev, C3501H_R30_MON_FER_CFG, &data_w, 1);
		}

		*rs_ubc = fer_cnt;

		return SUCCESS;
	}
	else
	{
		return ERR_FAILED;
	}

}



INT32 nim_c3501h_get_ldpc_iter_cnt(struct nim_device *dev, UINT8 *rs_ub)
{
    UINT8 data;

	nim_reg_read(dev, C3501H_R14_S2_FEC_RPT + 1, &data,1);
	*rs_ub = data;
	
	return SUCCESS;
}

INT32 nim_c3501h_get_bchover(struct nim_device *dev, UINT8 *rs_ub)
{
    UINT8 data = 0;
	UINT8 bch_over = 0;

	nim_reg_read(dev, C3501H_R14_S2_FEC_RPT + 2, &data,1);
	// set the MSB of reg-1f to 1
	bch_over = data | 0x10;
	*rs_ub   = bch_over;
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_sub_work_mode(struct nim_device *dev, UINT8 *rs_ubc)
*   Get sub work mode from hw,  such as ACM/CCM...
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *rs_ubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_sub_work_mode(struct nim_device *dev, UINT8 *rs_ubc)
{
	UINT8 data = 0;

	if (NULL == rs_ubc)
		return ERR_FAILED;
    
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data,1);
	*rs_ubc = (data >> 2) & 0x03;

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
*  Get symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
	UINT8 data[2];
	UINT32 sample_rate = 0;
	UINT32 symrate = 0;

    data[0] = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &(data[0]), 1);

	nim_reg_read(dev, C3501H_R24_MODU_INFO_RPT_3, data, 2);
	symrate = data[0] + (data[1] <<8); 

	nim_c3501h_get_dsp_clk(dev, &sample_rate);
	*sym_rate = nim_c3501h_multu64div(symrate, sample_rate, 131072); // 131072 = 2^17

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
INT32 nim_c3501h_get_demod_gain(struct nim_device *dev, UINT8 *agc)
{	 
	nim_reg_read(dev, C3501H_R34_AGC1_DCC_RPT, agc, 1);
	//MON_PRINTF("demod get agc gain value %d\n",*agc);
	return SUCCESS;
}

 
/*****************************************************************************
*  INT32 nim_c3503_get_tuner_gain(struct nim_device *dev, UINT8 agc, INT32 *agc_tuner)
*
*	This function will get the tuner total gain, only for m3031 tuner
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 agc
*	Parameter3: INT32 *agc_tuner
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_tuner_gain(struct nim_device *dev, UINT8 agc, INT32 *agc_tuner)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if(priv->tuner_type == IS_M3031)
	{		
		if (priv->nim_tuner_gain != NULL)
		{
			if (nim_c3501h_i2c_through_open(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }
            
            NIM_MUTEX_ENTER(priv);
			*agc_tuner = priv->nim_tuner_gain(priv->tuner_index,agc);
            NIM_MUTEX_LEAVE(priv);

			if (nim_c3501h_i2c_through_close(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }
		}
		else
		{
			*agc_tuner = 0;
		}
	}

	//MON_PRINTF("demod get tuner gain value %d\n",*agc_tuner);
	return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_c3501h_get_dbm_level(struct nim_device *dev,UINT8 agc_demod, INT32 agc_tuner,INT8 *level_db)
*
*	This function will acount the signal intensity with dBm unit
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 agc_demod: demod's agc value
*	Parameter3: INT32 agc_tuner:tuner's agc value
*	Parameter4: INT8 *level_db
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_dbm_level(struct nim_device *dev, UINT8 agc_demod, INT32 agc_tuner, INT8 *level_db)
{
	INT32 level_db_tmp ;
	INT8 agc_demod_tmp = 0 ;
    INT32 agc_gain_flag = 0;    //  Bit[0] = mix_lgain_status , bit[1] =  rf_coarse_gain_status, bit[2] =  dgb_gain_status, used to choose the formula for dBm calculate 
    UINT8 mix_lgain_flag = 0, rf_coarse_gain_flag = 0, dgb_gain_flag = 0;
	struct ali_nim_agc nim_agc;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if(priv->tuner_type == IS_M3031)
	{		
        // Add by paladin 2015/10/22 for agc dbm linear indication
        // Get tuner gain status
        //call_tuner_command(dev, NIM_TUNER_GET_C3031B_GAIN_FLAG, &agc_gain_flag);
        if (priv->nim_tuner_command)
	    {
	    	priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_GET_C3031B_GAIN_FLAG, &agc_gain_flag);
		}

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
			if (nim_c3501h_i2c_through_open(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }
            NIM_MUTEX_ENTER(priv);
			priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_GET_RF_POWER_LEVEL, (INT32 *)&nim_agc);	
            NIM_MUTEX_LEAVE(priv);
			if (nim_c3501h_i2c_through_close(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }
			level_db_tmp = nim_agc.rf_level;			
		}
		else
		{	
		#if 0
			if (nim_c3501h_i2c_through_open(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }
			priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_GET_RF_POWER_LEVEL, 0);
			if (nim_c3501h_i2c_through_close(dev))
		    {
		        NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
		        return C3501H_ERR_I2C_NO_ACK;
		    }		
		#endif
		
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
*  INT32 nim_c3501h_get_agc_filter(struct nim_device *dev, INT8 *agc)
*
*	This function will make the agc value smoothly
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: INT8 *agc
*
* Return Value: INT32
*****************************************************************************/
#define   s_filter_len  2  //even is better.

INT32 nim_c3501h_get_agc_filter(struct nim_device *dev, INT8 *agc)
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
* INT32 nim_c3501h_get_agc(struct nim_device *dev, UINT8 *agc)
*
*	This function will access the NIM to determine the AGC feedback value for ui
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_agc(struct nim_device *dev, UINT8 *agc)
{
	UINT8 agc_demod = 0 ;
	INT32 agc_tuner = 0 ;
	INT8 level_db_tmp = 0 ;	

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	nim_c3501h_get_demod_gain(dev,&agc_demod);

	if(priv->tuner_type == IS_M3031)
	{	 
		nim_c3501h_get_tuner_gain(dev,agc_demod, &agc_tuner);
		//MON_PRINTF("tuner agc total gain value %d\n",agc_tuner);
	}

	nim_c3501h_get_dbm_level(dev,agc_demod,agc_tuner,&level_db_tmp);	 
	//MON_PRINTF("-----------------agc gain dBm value %d\n",level_db_tmp);
	nim_c3501h_get_agc_filter(dev,&level_db_tmp);	 
	//MON_PRINTF("-----------------dBm value after filter %d\n\n",level_db_tmp);

	if (level_db_tmp > 0) //limit the range from -96----0
	{
		level_db_tmp = 0;
	}
	else if (level_db_tmp < -96)
	{
		level_db_tmp = -96;
	}
			
	*agc= level_db_tmp+100 ;

return SUCCESS;
}
	 

/*****************************************************************************
* INT32 nim_c3501h_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*	This function will access the NIM to determine the AGC feedback value, uint is dbm
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: INT8 *agc_dbm
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_agc_dbm(struct nim_device *dev, INT8 *agc_dbm)
{
	UINT8 agc_demod = 0 ;
	INT32 agc_tuner = 0 ;
	INT8 level_db_tmp = 0 ;	

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	nim_c3501h_get_demod_gain(dev,&agc_demod);

	if(priv->tuner_type == IS_M3031)
	{	 
		nim_c3501h_get_tuner_gain(dev,agc_demod, &agc_tuner);
		//MON_PRINTF("tuner agc total gain value %d\n",agc_tuner);
	}

	nim_c3501h_get_dbm_level(dev,agc_demod,agc_tuner,&level_db_tmp);	 
	//MON_PRINTF("-----------------agc gain dBm value %d\n",level_db_tmp);
	nim_c3501h_get_agc_filter(dev,&level_db_tmp);	 
	//MON_PRINTF("-----------------dBm value after filter %d\n\n",level_db_tmp);

	*agc_dbm = level_db_tmp;

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_get_ber(struct nim_device *dev, UINT32 *rs_ubc)  
*Description: Read per number from hardware
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT32 *rs_ubc: ber number, if TP unlock it will return 1010
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_ber(struct nim_device *dev, UINT32 *rs_ubc)     
{
	UINT8 ber[4];
	static UINT32 ber_cnt = 1010;
	UINT8 lock = 0, data = 0, data_w = 0;

	*rs_ubc = 1010;

	nim_c3501h_get_lock(dev, &lock);
	if (1 == lock)	// Channel Lock
	{
		// Get status
		nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
		
		if(0x20&data)
		{
			data_w = data&0xdf;	//report error packet number
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);
		}
		nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	

#if 0	// If ber_dec_sel = 0, we used full met mode
		if(0x10&data)
		{
			data_w = data&0xef;	//sta till counter full
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);
			// Clear per cnt
			nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
			data_w = data | 0x02;
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);
			// Restart monitor
			nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
			data_w = data | 0x01;
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);
		}
#endif

		// Get result
		nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);
		//NIM_PRINTF("C3501H_R38_MON_BER_CFG = 0x%2x \n", data);
		if (data & 0x80)	// if static done
		{
			data_w = data | 0x04;
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);

			nim_reg_read(dev, C3501H_R3C_MON_BER_RPT, ber, 4);	
			ber_cnt = ber[0] + (ber[1]<<8) + (ber[2]<<16) + ((ber[3] & 0x3f)<<24);

			// Clear per cnt
			nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
			data_w = data | 0x02;
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);

			// Restart monitor
			nim_reg_read(dev, C3501H_R38_MON_BER_CFG, &data, 1);	
			data_w = data | 0x01;
			nim_reg_write(dev, C3501H_R38_MON_BER_CFG, &data_w, 1);
		}
		else	// else keep last value
		{
			ber_cnt = ber_cnt;
		}

		*rs_ubc = ber_cnt;
		return SUCCESS;
	}
	else
	{
		return ERR_FAILED;
	}
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
*Description: Get current TP's work mode
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *work_mode
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
{
	UINT8 data = 0;
	// Bit 25:24 is rpt work mode
	// 0: DVB-S
	// 1: DVB-S2
	// 2: DVB-DSS
	// 3: Auto

	//NIM_PRINTF("Enter function: %s\r\n",__FUNCTION__);
	
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);
	data &= 0x03;
	*work_mode = data;

	//NIM_PRINTF("rpt_work_mode = %d\r\n", *work_mode);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_sub_work_mode(struct nim_device *dev, UINT8 *sub_work_mode)
*Description: Get current TP's sub work mode
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *sub_work_mode
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_sub_work_mode(struct nim_device *dev, UINT8 *sub_work_mode)
{
	UINT8 data = 0;
	// Bit 25:24 is rpt work mode
	// 0: ACM/VCM mode
	// 1: CCM without dummy mode
	// 2: CCM with dummy mode
	// 3: Reserved

	//NIM_PRINTF("Enter function: %s\r\n",__FUNCTION__);
	
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);
	data = (data >> 2) & 0x03;
	*sub_work_mode = data;

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
*Description: Get current TP's roll off
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *roll_off
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
{
	UINT8 data = 0;
    
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

  	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 2, &data, 1);
	data &= 0x70;
	*roll_off = (data>>4);  
    /* 
    BB frame roll off report
    0 = 0.35
    1 = 0.25
    2 = 0.20
    3 = Reserved
    4 = 0.15
    5 = 0.10
    6 = 0.05
    7 = Reserved
    */
    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iq_swap)
*Description: Get current TP's iq swap state
*	
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *iq_swap
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iq_swap)
{
	UINT8 data = 0;
    
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);

  	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 2, &data, 1);
	data &= 0x80;
	*iq_swap = (data>>7);  
    /* 
    0: Do not swap.
    1: Swap IQ
    */

    return SUCCESS;
}


/*****************************************************************************
 *  INT32 nim_c3501h_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
 *Description: Get current TP's map type
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 *map_type
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3501h_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
{
	UINT8 data = 0;
	// NIM_PRINTF("Enter Fuction nim_c3501h_reg_get_map_type \n");

    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);
    
	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1, &data, 1);
	*map_type = data&0x0f;

	return SUCCESS;
	/*  Map type:
    0x0: Reserved
    0x1: BPSK
    0x2: QPSK
    0x3: 8PSK
    0x4: 16APSK
    0x5: 32APSK
    0x6: 64APSK
    */
}


/*****************************************************************************
 *  INT32 nim_c3501h_reg_get_pilot(struct nim_device *dev, UINT8 *pilot)
 * Description: Get current TP's pilot
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 *pilot, 1: on, 0: off
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3501h_reg_get_pilot(struct nim_device *dev, UINT8 *pilot)
{
	UINT8 data = 0;
	// NIM_PRINTF("Enter Fuction nim_c3501h_reg_get_map_type \n");

    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);  
    
	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1, &data, 1);
	*pilot = (data&0x10)>>4;

	return SUCCESS;
}


/*****************************************************************************
 *  INT32 nim_c3501h_reg_get_frame_type(struct nim_device *dev, UINT8 *frame)
 * Description: Get current TP's frame type
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT8 *frame, normal or short
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3501h_reg_get_frame_type(struct nim_device *dev, UINT8 *frame)
{
	UINT8 data = 0;
	// NIM_PRINTF("Enter Fuction nim_c3501h_reg_get_map_type \n");
	
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);
    
	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1, &data, 1);
	*frame = (data&0x60)>>5;

	return SUCCESS;
	/*  Frame type:
    0x0: normal frame
    0x1: short frame
    0x2: medium frame
    */
}


/*****************************************************************************
*  INT32 nim_c3501h_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
* Description: Get current TP's modcod
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 *modcod
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
{
	UINT8 data = 0;
	//  NIM_PRINTF("Enter Fuction nim_c3501h_reg_get_modcode \n");

    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);
   
	nim_reg_read(dev, C3501H_R20_MODU_INFO_RPT_2, &data, 1);
	*modcod = data & 0xff;
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 Rs, UINT32 *bit_rate)
* Description: Calculate the bit rate by input conditions
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
INT32 nim_c3501h_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 rs, UINT32 *bit_rate)
{
	UINT32 data = 0;
	UINT32 temp = 0;
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if (work_mode != C3501H_DVBS2_MODE) // DVBS mode
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

		if ((priv->tuner_config_data.qpsk_config & C3501H_USE_188_MODE) == C3501H_USE_188_MODE)
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
* INT32 nim_c3501h_get_bitmode(struct nim_device *dev, UINT8 *bitMode)
* Description:Get ts bit mode from board_config.c
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8* bitMode
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_bitmode(struct nim_device *dev, UINT8 *bitMode)
{
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3501H_1BIT_MODE)
		*bitMode = 0x60;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3501H_2BIT_MODE)
		*bitMode = 0x00;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3501H_4BIT_MODE)
		*bitMode = 0x20;
	else if ((priv->tuner_config_data.qpsk_config & 0xc0) == C3501H_8BIT_MODE)
		*bitMode = 0x40;
	else
		*bitMode = 0x40;
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_get_tp_info(struct nim_device *dev)
* Description:Get current TP's information
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_tp_info(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 time_out = 0;
    UINT32 symbol_rate = 0;
    INT32 freq_offset = 0;
    UINT8 work_mode = 0;
    UINT8 sub_work_mode = 0;    
    UINT8 code_rate = 0;
    UINT8 roll_off = 0;
    UINT8 pilot_mode = 0;
    UINT8 modcod = 0;
    UINT8 iqswap_flag = 0;
    UINT8 map_type = 0;
    UINT8 frame_type = 0;
    INT16 snr = 0;
    INT8 signal_level;

    //NIM_RPINTF("Enter %s \n", __FUNCTION__);

	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
	}

 	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    struct nim_dvbs_channel_info *channel_info = priv->channel_info;
  
    // Check lock status
    while(1)
    {
        nim_c3501h_get_lock(dev, &data);
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
    
    nim_c3501h_get_symbol_rate(dev, &symbol_rate);
    nim_c3501h_reg_get_freqoffset(dev, &freq_offset);
    nim_c3501h_reg_get_work_mode(dev, &work_mode);
    nim_c3501h_reg_get_sub_work_mode(dev, &sub_work_mode);
    nim_c3501h_reg_get_code_rate(dev, &code_rate);
    nim_c3501h_reg_get_roll_off(dev, &roll_off);
    nim_c3501h_reg_get_modcod(dev, &modcod);
    nim_c3501h_reg_get_pilot(dev, &pilot_mode);
    nim_c3501h_reg_get_iqswap_flag(dev, &iqswap_flag);
    nim_c3501h_reg_get_map_type(dev, &map_type);
    nim_c3501h_reg_get_frame_type(dev, &frame_type);
    nim_c3501h_get_agc_dbm(dev, &signal_level);
    nim_c3501h_get_snr_db(dev, &snr);
    channel_info->symbol_rate = symbol_rate;
    channel_info->freq_offset = freq_offset;
    channel_info->work_mode = work_mode;
    channel_info->sub_work_mode = sub_work_mode;
    channel_info->code_rate = code_rate;
    channel_info->roll_off = roll_off;
    channel_info->pilot_mode = pilot_mode;
    channel_info->modcod = modcod;
    channel_info->iqswap_flag = iqswap_flag;
    channel_info->map_type = map_type;
    channel_info->frame_type = frame_type;
    channel_info->signal_level = signal_level;
    channel_info->snr = snr;

    return SUCCESS;
}
                                                                                                  
// For Debug                                                                                      
/*****************************************************************************
* INT32 nim_c3501h_printf_tp_info(struct nim_device *dev)
* Description: Print current TP's information
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
*
* Return Value: INT32
*****************************************************************************/                 
INT32 nim_c3501h_printf_tp_info(struct nim_device *dev)                                           
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
	                                                                                              
	struct nim_c3501h_private *priv = (struct nim_c3501h_private *)dev->priv;                     
	channel_info = priv->channel_info;                                                            
	bb_header_info = priv->bb_header_info;                                                        
	isid = priv->isid;                                                                                                                                                                           
                                                                                                  
    NIM_PRINTF("--------------------------------- \n");                                          
    NIM_PRINTF("Now print modulation information \n");                                           
    NIM_PRINTF("--------------------------------- \n");                                          
                                                                                              
    NIM_PRINTF("Symbol rate = %d\n", channel_info->symbol_rate);                                  
    NIM_PRINTF("Freq offset = %d\n", channel_info->freq_offset);                                  
                                                                                                  
	 switch(channel_info->work_mode)                                                              
	 {                                                                                            
		 case 0 : {NIM_PRINTF("Work mode is DVB-S \n"); break;}                                   
		 case 1 : {NIM_PRINTF("Work mode is DVB-S2 \n"); break;}                                  
		 case 2 : {NIM_PRINTF("Work mode is DVB-DSS \n"); break;}                                 
		 default: {NIM_PRINTF("Work mode is Reserved \n"); break;}                                
	 }                                                                                            
                                                                                                  
	 if((0 == channel_info->work_mode) || (2 == channel_info->work_mode))  // DVB-S DSS           
	 {                                                                                            
		 switch(channel_info->code_rate)                                                          
		 {                                                                                        
			 case 0 : {NIM_PRINTF("Code rate is 1/2\n"); break;}                                  
			 case 1 : {NIM_PRINTF("Code rate is 2/3\n"); break;}                                  
			 case 2 : {NIM_PRINTF("Code rate is 3/4\n"); break;}                                  
			 case 3 : {NIM_PRINTF("Code rate is 5/6\n"); break;}                                  
			 case 4 : {NIM_PRINTF("Code rate is 6/7\n"); break;}                                  
			 case 5 : {NIM_PRINTF("Code rate is 7/8\n"); break;}                                  
			 default : {NIM_PRINTF("Code rate is Reserved\n"); break;}                            
		 }                                                                                        
	 }                                                                                            
	 else if (1 == channel_info->work_mode)                                                       
	 {                                                                                            
        if (0 == channel_info->sub_work_mode)                                                     
        {                                                                                         
            NIM_PRINTF("ACM/VCM sub work mode\n");                                                
        }                                                                                         
        else if (1 == channel_info->sub_work_mode)                                                
        {                                                                                         
            NIM_PRINTF("CCM without dummy mode sub work mode\n");                                 
        }                                                                                         
        else if (2 == channel_info->sub_work_mode)                                                
        {                                                                                         
            NIM_PRINTF("CCM with dummy mode sub work mode\n");                                    
        }                                                                                         
        else                                                                                      
        {                                                                                         
            NIM_PRINTF("Reserved sub work mode\n");                                               
        }                                                                                         
                                                                                                  
        if (0 == channel_info->frame_type)   // Noraml                                            
        {                                                                                         
            NIM_PRINTF("Normal frame type\n");                                                    
                                                                                                  
    		 switch(channel_info->code_rate)                                                      
    		 {                                                                                    
    			 case 0 : {NIM_PRINTF("Code rate is 1/4\n"); break;}                              
    			 case 1 : {NIM_PRINTF("Code rate is 1/3\n"); break;}                              
    			 case 2 : {NIM_PRINTF("Code rate is 2/5\n"); break;}                              
    			 case 3 : {NIM_PRINTF("Code rate is 1/2\n"); break;}                              
    			 case 4 : {NIM_PRINTF("Code rate is 3/5\n"); break;}                              
    			 case 5 : {NIM_PRINTF("Code rate is 2/3\n"); break;}                              
    			 case 6 : {NIM_PRINTF("Code rate is 3/4\n"); break;}                              
    			 case 7 : {NIM_PRINTF("Code rate is 4/5\n"); break;}                              
    			 case 8 : {NIM_PRINTF("Code rate is 5/6\n"); break;}                              
    			 case 9 : {NIM_PRINTF("Code rate is 8/9\n"); break;}                              
    			 case 10 : {NIM_PRINTF("Code rate is 9/10\n"); break;}                            
                 case 11 : {NIM_PRINTF("Code rate is 2/9\n"); break;}                             
                 case 12 : {NIM_PRINTF("Code rate is 13/45\n"); break;}                           
                 case 13 : {NIM_PRINTF("Code rate is 9/20\n"); break;}                            
                 case 14 : {NIM_PRINTF("Code rate is 90/180\n"); break;}                          
                 case 15 : {NIM_PRINTF("Code rate is 96/180\n"); break;}                          
                 case 16 : {NIM_PRINTF("Code rate is 11/20\n"); break;}                           
                 case 17 : {NIM_PRINTF("Code rate is 100/180\n"); break;}                         
                 case 18 : {NIM_PRINTF("Code rate is 26/45\n"); break;}                           
                 case 19 : {NIM_PRINTF("Code rate is 104/180\n"); break;}                         
                 case 20 : {NIM_PRINTF("Code rate is 18/30\n"); break;}                           
                 case 21 : {NIM_PRINTF("Code rate is 28/45\n"); break;}                           
                 case 22 : {NIM_PRINTF("Code rate is 23/36\n"); break;}                           
                 case 23 : {NIM_PRINTF("Code rate is 116/180\n"); break;}                         
                 case 24 : {NIM_PRINTF("Code rate is 20/30\n"); break;}                           
                 case 25 : {NIM_PRINTF("Code rate is 124/180\n"); break;}                         
                 case 26 : {NIM_PRINTF("Code rate is 25/36\n"); break;}                           
                 case 27 : {NIM_PRINTF("Code rate is 128/180\n"); break;}                         
                 case 28 : {NIM_PRINTF("Code rate is 13/18\n"); break;}                           
                 case 29 : {NIM_PRINTF("Code rate is 22/30\n"); break;}                           
                 case 30 : {NIM_PRINTF("Code rate is 132/180\n"); break;}                         
                 case 31 : {NIM_PRINTF("Code rate is 135/180\n"); break;}                         
                 case 32 : {NIM_PRINTF("Code rate is 7/9\n"); break;}                             
                 case 33 : {NIM_PRINTF("Code rate is 140/180\n"); break;}                         
                 case 34 : {NIM_PRINTF("Code rate is 154/180\n"); break;}                         
    			 default : {NIM_PRINTF("Code rate is Reserved\n"); break;}                        
    		 }                                                                                    
        }                                                                                         
        else if (1 == channel_info->frame_type)   // Short                                        
        {                                                                                         
            NIM_PRINTF("Normal frame type\n");                                                    
                                                                                                  
            switch(channel_info->code_rate)                                                       
            {                                                                                     
                case 0 : {NIM_PRINTF("Code rate is 1/4\n"); break;}                               
                case 1 : {NIM_PRINTF("Code rate is 1/3\n"); break;}                               
                case 2 : {NIM_PRINTF("Code rate is 2/5\n"); break;}                               
                case 3 : {NIM_PRINTF("Code rate is 1/2\n"); break;}                               
                case 4 : {NIM_PRINTF("Code rate is 3/5\n"); break;}                               
                case 5 : {NIM_PRINTF("Code rate is 2/3\n"); break;}                               
                case 6 : {NIM_PRINTF("Code rate is 3/4\n"); break;}                               
                case 7 : {NIM_PRINTF("Code rate is 4/5\n"); break;}                               
                case 8 : {NIM_PRINTF("Code rate is 5/6\n"); break;}                               
                case 9 : {NIM_PRINTF("Code rate is 8/9\n"); break;}                               
                case 10 : {NIM_PRINTF("Code rate is 11/45\n"); break;}                            
                case 11 : {NIM_PRINTF("Code rate is 4/15\n"); break;}                             
                case 12 : {NIM_PRINTF("Code rate is 14/45\n"); break;}                            
                case 13 : {NIM_PRINTF("Code rate is 7/15\n"); break;}                             
                case 14 : {NIM_PRINTF("Code rate is 8/15\n"); break;}                             
                case 15 : {NIM_PRINTF("Code rate is 26/45\n"); break;}                            
                case 16 : {NIM_PRINTF("Code rate is 32/45\n"); break;}                            
                default : {NIM_PRINTF("Code rate is Reserved\n"); break;}                         
            }                                                                                     
        }                                                                                         
        else if (2 == channel_info->frame_type)   // Medium                                       
        {                                                                                         
            NIM_PRINTF("Medium frame type\n");                                                    
        }                                                                                         
        else                                                                                      
        {                                                                                         
            NIM_PRINTF("Reserved frame type\n");                                                  
        }                                                                                         
	 }                                                                                            
	                                                                                              
	 if(1 == channel_info->work_mode)                                                             
	 {                                                                                            
		 switch(channel_info->roll_off)                                                           
		 {                                                                                        
			 case 0 : {NIM_PRINTF("Roll off is 0.35\n");break;};                                  
			 case 1 : {NIM_PRINTF("Roll off is 0.25\n");break;};                                  
			 case 2 : {NIM_PRINTF("Roll off is 0.20\n");break;};                                  
			 case 3 : {NIM_PRINTF("Roll off is Reserved\n");break;};                              
			 case 4 : {NIM_PRINTF("Roll off is 0.15\n");break;};                                  
			 case 5 : {NIM_PRINTF("Roll off is 0.10\n");break;};                                  
			 case 6 : {NIM_PRINTF("Roll off is 0.05\n");break;};                                  
			 default : {NIM_PRINTF("Roll off is Reserved\n");break;};                             
		 }                                                                                        
	 }                                                                                            
	                                                                                              
	 if(1 == channel_info->work_mode)                                                             
	 {                                                                                            
		 if(channel_info->pilot_mode)                                                             
		 {
		 	NIM_PRINTF("Pilot on \n");                                                           
		 }
		 else    
		 {
			 NIM_PRINTF("Pilot off \n");                                                          
		 }
	 }                                                                                                                                                                                      
                                                                                                  
     NIM_PRINTF("Modcod is %d\n", channel_info->modcod);                                          
	                                                                                              
	 NIM_PRINTF("Iqswap flag is %d\n", channel_info->iqswap_flag);                                
                                                                                                  
	 if(1 == channel_info->work_mode)                                                             
	 {                                                                                            
		 switch(channel_info->map_type)                                                           
		 {                                                                                        
			 case 0 : {NIM_PRINTF("Map type is HBCD\n"); break;}                                  
			 case 1 : {NIM_PRINTF("Map type is BPSK\n"); break;}                                  
			 case 2 : {NIM_PRINTF("Map type is QPSK\n"); break;}                                  
			 case 3 : {NIM_PRINTF("Map type is 8PSK\n"); break;}                                  
			 case 4 : {NIM_PRINTF("Map type is 16APSK\n"); break;}                                
			 case 5 : {NIM_PRINTF("Map type is 32APSK\n"); break;}                                
			 case 6 : {NIM_PRINTF("Map type is 64APSK\n"); break;}                                
			 default : {NIM_PRINTF("Map type is Reserved\n"); break;}                             
		 }                                                                                        
	 }                                                                                            
	 else                                                                                         
	 {                                                                                            
		 NIM_PRINTF("Map type is QPSK\n");                                                        
	 }                                                                                            
                                                                                                  
	 NIM_PRINTF("Signal level is %ddBm\n", channel_info->signal_level);                           
     NIM_PRINTF("Signal snr is %d, unit is 0.01dB\n\n", channel_info->snr);                                                                                                                       
                                                                                                  
    // For debug review                                                                           
    NIM_PRINTF("------------------------------------- \n");                                       
    NIM_PRINTF("Now printf BBFrame header information \n");                                       
    NIM_PRINTF("------------------------------------- \n");                                       
                                                                                                  
    if (1 == channel_info->work_mode)                                                             
    {                                                                                             
        for(i = 0; i < 10; i++)                                                                   
            NIM_PRINTF("bb_header.source_data[%d] = 0x%2x \n", i, bb_header_info->source_data[i]);
                                                                                                  
        NIM_PRINTF("\n");                                                                         
                                                                                                  
                                                                                                  
        switch(bb_header_info->stream_mode)                                                       
        {                                                                                         
            case NIM_GS_PACKET:                                                                   
                NIM_PRINTF("Stream mode is NIM_GS_PACKET\n");                                     
                break;                                                                            
            case NIM_GS_CONTINUE:                                                                 
                NIM_PRINTF("Stream mode is NIM_GS_CONTINUE\n");                                   
                break;                                                                            
            case NIM_RESERVED:                                                                    
                NIM_PRINTF("Stream mode is NIM_RESERVED\n");                                      
                break;                                                                            
            case NIM_TS:                                                                          
                NIM_PRINTF("Stream mode is NIM_TS\n");                                            
                break;                                                                            
            default:                                                                              
                NIM_PRINTF("Stream mode is UNKNOWN\n");                                           
                break;                                                                            
        }                                                                                         
                                                                                        
        if(NIM_STREAM_MULTIPLE == bb_header_info->stream_type)                                    
        {                                                                                         
            NIM_PRINTF("Stream type is NIM_STREAM_MULTIPLE\n");                                   
            NIM_PRINTF("Current isi :   %d\n", bb_header_info->isi);                              
        }                                                                                         
        else  
        {
            NIM_PRINTF("Stream type is NIM_STREAM_SINGLE\n");                                     
        }
		
        if(NIM_ACM == bb_header_info->acm_ccm)                                                    
        {
        	NIM_PRINTF("Sub work mode is NIM_ACM/VCM\n");                                               
        }
        else                                                                                      
        {
        	NIM_PRINTF("Sub work mode is NIM_CCM\n");                                                   
        }
                                                                                                  
        if(NIM_ISSYI_ACTIVE == bb_header_info->issyi)                                             
        {
        	NIM_PRINTF("Issyi is NIM_ISSYI_ACTIVE\n");                                            
        }
        else                                                                                      
        {
        	NIM_PRINTF("Issyi is NIM_ISSYI_NOT_ACTIVE\n");                                        
        }
                                                                                                  
        if(NIM_NPD_ACTIVE == bb_header_info->npd)                                                 
        {
        	NIM_PRINTF("Npd is NIM_NPD_ACTIVE\n");                                                
        }
        else                                                                                      
        {
        	NIM_PRINTF("Npd is NIM_NPD_NOT_ACTIVE\n");                                            
        }
                                                                                                  
        switch(bb_header_info->roll_off)                                                          
        {                                                                                         
            case NIM_RO_035:                                                                      
                NIM_PRINTF("Roll off is NIM_RO_035\n");                                           
                break;                                                                            
            case NIM_RO_025:                                                                      
                NIM_PRINTF("Roll off is NIM_RO_025\n");                                           
                break;                                                                            
            case NIM_RO_020:                                                                      
                NIM_PRINTF("Roll off is NIM_RO_020\n");                                           
                break;                                                                            
            case NIM_RO_RESERVED:                                                                 
                NIM_PRINTF("Roll off is NIM_RO_RESERVED\n");                                      
                break;                                                                            
            default:                                                                              
                NIM_PRINTF("Roll off is unknown\n");                                              
                break;                                                                            
        }                                                                                         
                                                                                                  
        NIM_PRINTF("upl   is  0x%x\n", bb_header_info->upl);                                      
        NIM_PRINTF("dfl   is  0x%x\n", bb_header_info->dfl);                                      
        NIM_PRINTF("sync  is  0x%x\n", bb_header_info->sync);                                     
        NIM_PRINTF("syncd is  0x%x\n\n", bb_header_info->syncd);                                  
                                                                                                  
     }                                                                                            
                                                                                             
	NIM_PRINTF("--------------------------------- \n");	                                          
	NIM_PRINTF("    Now printf ISI information    \n");                                           
	NIM_PRINTF("--------------------------------- \n");                                           
                                                                                                  
	if(NIM_STREAM_SINGLE == bb_header_info->stream_type)                                          
	{                                                                                             
		NIM_PRINTF("Is NIM_STREAM_SINGLE, there is no isid info\n\n");                            
		return SUCCESS;                                                                           
	}                                                                                             
                                                                                                  
	NIM_PRINTF("There are %d Streams\n", isid->isid_num);                                         
    for(i = 0; i < isid->isid_num; i++)                                                           
    {                                                                                             
		NIM_PRINTF("isid[%d] = %d \n", i, isid->isid_read[i]);                                    
    }                                                                                             
                                                                                                  
	NIM_PRINTF("\n");                                                                             
	                                                                                              
	return SUCCESS;                                                                               
}                                                                                                 



/*****************************************************************************
*  INT32 nim_c3501h_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
* Description: Set the DSP clock
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 clk_sel
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
{
	UINT8 data;
	//  NIM_PRINTF("Enter Fuction nim_c3501h_set_dsp_clk \n");

    // need to check if there need set by dsp clk !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//data = clk_sel & 0xff;
	//nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
    // 0: 77.625
    // 1: 112.9 (default)
    // 2: 124.2
    // 3: 138
    // 4~7: FIN

	nim_reg_read(dev, C3501H_R04_SYS_CFG, &data, 1);
	data &= 0xf8;
    data |= (clk_sel&0x07);  
	nim_reg_write(dev, C3501H_R04_SYS_CFG, &data, 1);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_dsp_clk(struct nim_device *dev, UINT8 *sample_rate)
* Description: Get the DSP work clock 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sample_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate)
{
	UINT8 data;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_modcode \r\n");

#ifdef FPGA_PLATFORM
    if(global_ctx.demod_driver_mode == DRIVER_FPGA_MODE)
    {
        *sample_rate = dev->dsp_clock*1000;
        return SUCCESS;
    }
#else
    nim_reg_read(dev, C3501H_R04_SYS_CFG, &data, 1);
    data = data & 0x07;
    // 0: 77.625
    // 1: 112.9 (default)
    // 2: 124.2
    // 3: 138
    // 4~7: FIN
    if (0 == data)
        *sample_rate = 77625;  ////uint is KHz
    else if (1 == data)
        *sample_rate = 112900;  ////uint is KHz
    else if (2 == data)
        *sample_rate = 124200;  ////uint is KHz
    else if (3 == data)
        *sample_rate = 138000;  ////uint is KHz
    else if (priv->ul_status.m_c3501h_strap_bond_info & 0x4000)
        *sample_rate = XTAL_CLK_27M;  ////uint is KHz
    else
        *sample_rate = XTAL_CLK_13_5M;  ////uint is KHz

    // For sdk + fpga test
    #ifdef SDK_FPGA_TEST
        *sample_rate = 27000;
    #endif
#endif

	return SUCCESS;
}


 /*****************************************************************************
 * INT32 nim_c3501h_get_bb_header_info(struct nim_device *dev)
 *
 * This function get bb header data from hardware 
 *
 * Arguments:
 *  Parameter1: struct nim_device *dev
 * 
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3501h_get_bb_header_info(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT8 i = 0;
	UINT8 work_mode = 0;
	UINT8 time_out = 0;
	UINT32 bbheader_crc_flg  = 0;
    UINT8 crc_err_cnt = 0;
    UINT8 time_out_threshold = 0;
	void *ptr_bbheader = NULL;
	int polynomial   = 0x56;

	struct nim_c3501h_private *priv = NULL;
	struct nim_dvbs_bb_header_info *bb_header_info = NULL;
	 
	if(NULL == dev->priv)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n", __FUNCTION__);
        return ERR_NO_DEV;
	}
    
    ACM_DEBUG_PRINTF("Enter %s\r\n", __FUNCTION__);
    
	priv = (struct nim_c3501h_private *)dev->priv;
	bb_header_info = priv->bb_header_info;	
	comm_memset(bb_header_info, 0, sizeof(struct nim_dvbs_bb_header_info));

	// Check BB_START positive edge
#ifdef GET_BBH_SOLUTION
	nim_c3501h_la_p1_trg_start(dev);
#endif

	while(1)
	{
		if (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			ERR_PRINTF("[%s %d]priv->ul_status.c3501h_chanscan_stop_flag=%d,\r\n\
				priv->ul_status.c3501h_autoscan_stop_flag=%d\r\n", 
				__FUNCTION__, __LINE__, priv->ul_status.c3501h_chanscan_stop_flag,
				priv->ul_status.c3501h_autoscan_stop_flag);
			
			return ERR_FAILED;
		}
		
#ifdef GET_BBH_SOLUTION
        nim_c3501h_la_p1_trg_done_check(dev, &data);
#else
		nim_c3501h_get_lock(dev, &data); 
#endif
		// lock
		if(1 == data)
			break;
		
		// unlock
		if(time_out >= 15)
		{
			ERR_PRINTF("In %s exit with TP unlock \r\n", __FUNCTION__);	

			return ERR_FAILED;
		}

		time_out++; 
		comm_sleep(5);
	}
	time_out = 0;

	// Check TP type
	nim_c3501h_reg_get_work_mode(dev, &work_mode);
	if((0 == work_mode) || (3 == work_mode))
	{
		ERR_PRINTF("In %s exit with TP is not DVB-S2\r\n", __FUNCTION__);

		return ERR_FAILED;
	} 

    if (priv->cur_sym < 3000)
    {
        // bentao has gotten a solution of this issue via LA to get the BB_EN,we need to set BB REG after  getting BB_EN   GLoria 08/01/2018
        //comm_sleep(10);  // ??? If don't delay here, HW will not  report capture finished when Rs = 1M. Paladin 28/10/2017
        time_out_threshold = 30;
    }
    else
    {
        time_out_threshold = 15;
    }

	while(1)
	{
	    // Capture one bb frame's header when BB_CMD == 5. The BB information report will stop update when this bit is 1
		nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 3, &data, 1);
		data |= 0x80;
		nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 3, &data, 1);

		// 0x5: begin capture one BB frame header
		nim_reg_read(dev, C3501H_R18_BB_CMD_CFG, &data, 1);
		data &= 0xe0;
		data |= 0x05;
		nim_reg_write(dev, C3501H_R18_BB_CMD_CFG, &data, 1);

		// wait finished
		time_out = 0;
		while(1)
		{
			//BB header information latch status[12], 0: idle or wait latching, 1: latch a bb header successfully.
			nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 1, &data, 1);
			if (data & 0x10)
			{
				ACM_DEBUG_PRINTF("BBHeader capture finished\r\n");
				break;
			}

			if(time_out >= time_out_threshold)
			{
				ERR_PRINTF("[%s %d]exit with capture timeout, time_out=%d, time_out_threshold=%d \r\n", __FUNCTION__, __LINE__, time_out, time_out_threshold);

				return ERR_FAILED;
			}

			time_out++; 
			comm_sleep(5);
		}

		// read bbheader
		for (i = 0; i < 10; i++)
		{
			// write index
			nim_reg_read(dev, C3501H_R18_BB_CMD_CFG, &data, 1);
			data &= 0xe0;
			data |= (i + 0x10);
			nim_reg_write(dev, C3501H_R18_BB_CMD_CFG, &data, 1);

			// read
			nim_reg_read(dev, C3501H_R1C_BB_DEC_CFG + 2, &data, 1);
			bb_header_info->source_data[i] = data;
			ACM_DEBUG_PRINTF("bb_header[%d] = 0x%x \r\n", i, data);
		}
    	// added bbheader crc in driver by gloria 2016.1.6
    	ptr_bbheader = (void*)(bb_header_info->source_data);

    	bbheader_crc_flg = nim_c3501h_crc8_check(ptr_bbheader, 80, polynomial);

//#define TEST_BBHEADER_CAP

#ifdef TEST_BBHEADER_CAP
        if ((0 == bbheader_crc_flg) && (0x47 == bb_header_info->source_data[6]))

        {
			crc_err_cnt++;
            
            if (20 == crc_err_cnt)
            {
				break;
            } 
        }
        else
        {
            crc_err_cnt++;
            ACM_DEBUG_PRINTF("Get BBHEADER error, crc_err_cnt = %d\r\n", crc_err_cnt);
            if (CRC_THRESHOLD == crc_err_cnt)
            {
    			return ERR_CRC_HEADER;
            } 
        }
#else
        if ((0 == bbheader_crc_flg) && (0x47 == bb_header_info->source_data[6]))

        {
            break;
        }
        else
        {
            crc_err_cnt++;
            
            if (CRC_THRESHOLD == crc_err_cnt)
            {
                ERR_PRINTF("Get BBHEADER error, crc_err_cnt = %d\r\n", crc_err_cnt);

    			return ERR_CRC_HEADER;
            } 
        }
#endif

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

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid)
*  Get input stream id when it is in mult stream mode, must make sure TP is locked && dvb-s2 mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_dvbs_isid *p_isid
*
* Return Value: UINT8
*****************************************************************************/ 
INT32 nim_c3501h_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid)
{
    struct nim_c3501h_private *priv = NULL;
    //struct nim_dvbs_bb_header_info *p_bb_header_info = NULL;
	UINT8 data = 0;
    UINT16 time_out = 0;
    UINT8 cnt = 0;
    UINT8 i = 0, j = 0;
    //UINT8 data_valid = 1;
    //INT32 ret_get_bb_header = 0;

    ACM_DEBUG_PRINTF("Enter %s \r\n", __FUNCTION__);
    
	if(NULL == dev)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n", __FUNCTION__);
        return ERR_NO_DEV;
	}

	priv = (struct nim_c3501h_private *)dev->priv;
	if(NULL == priv)
	{
        ERR_PRINTF("[%s %d]NULL == priv\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}
	
	//p_bb_header_info = (struct nim_dvbs_bb_header_info*)(priv->bb_header_info);
    
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
		if (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3501h_chanscan_stop_flag=%d,\r\n\
				priv->ul_status.c3501h_autoscan_stop_flag=%d\r\n", 
				__FUNCTION__, __LINE__, priv->ul_status.c3501h_chanscan_stop_flag,
				priv->ul_status.c3501h_autoscan_stop_flag);	
			//p_isid->get_finish_flag = 0;
			return ERR_FAILED;
		}
		
		nim_c3501h_get_lock(dev, &data);
		
		if(1 == data)// lock
			break;
		
		if(cnt >= time_out)// unlock
		{
			ACM_DEBUG_PRINTF("In %s exit with TP unlock \r\n", __FUNCTION__);
			return ERR_FAILED; 
		}

		cnt++; 
		comm_sleep(1);
	}

	// 0x2: clear bb isiid buffer to zero.
	nim_reg_read(dev, C3501H_R18_BB_CMD_CFG, &data, 1);
	data &= 0xe0;
	data |= 0x02;
	nim_reg_write(dev, C3501H_R18_BB_CMD_CFG, &data, 1);

	// wait capture
	comm_sleep(time_out);

    // Begin get isid
	for (i = 0; i < 32; i++)
	{
		// write index
		nim_reg_read(dev, C3501H_R20_BB_ROLL_OFF_CFG + 3, &data, 1);
		data &= 0xe0;
		data |= i;
		nim_reg_write(dev, C3501H_R20_BB_ROLL_OFF_CFG + 3, &data, 1);

		// read
		nim_reg_read(dev, C3501H_R1C_BB_DEC_CFG + 3, &data, 1);
		//ACM_DEBUG_PRINTF("isid_rpt[%d] = 0x%x\r\n", i, data);

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

	//p_isid->get_finish_flag = 1;
	if (priv->isid != p_isid)
	{
		comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
	}
	NIM_PRINTF("[%s %d]return!\r\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_private.c																													COPY FROM nim_dvbs_c3505_private.c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_tp_scan.c																													COPY FROM nim_dvbs_c3505_tp_scan.c
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UINT32 last_tp_freq = 0;
#ifdef DEBUG_CHANNEL_CHANGE
static INT32 fft_energy_store[1024];
#endif

#ifdef AUTOSCAN_DEBUG
static UINT32 search_times = 0;
#endif
static UINT32 re_search_num = 0;

INT32 c3501h_real_freq[TP_MAXNUM];//the real frequency and sysbolrate used by NIM_DRIVER_GET_VALID_FREQ
INT32 c3501h_real_sym_rate[TP_MAXNUM];
INT32 c3501h_real_tp_number;


//UINT16 config_data;
UINT32 c3501h_tp_lock_time = 0;

/*****************************************************************************
* INT32 nim_c3501h_channel_change(struct nim_device *dev , struct NIM_CHANNEL_CHANGE * pst_channel_change)
*
* Description: Set TP function, and it also can get isid/set isid/auto search plsn by different argument
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct NIM_CHANNEL_CHANGE * pst_channel_change
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_channel_change(struct nim_device *dev, struct NIM_CHANNEL_CHANGE * pst_channel_change)
{
#ifdef IMPLUSE_NOISE_IMPROVED
	UINT8 i = 0;
#endif
    UINT32 sym = 0, freq = 0;
    UINT8 fec = 0;
	UINT32 stop_start = 0;	// used for exit while loop
    struct nim_c3501h_private *priv = NULL;
    struct nim_c3501h_tp_scan_para *tp_scan_param;

#ifdef TSN_SUPPORT
	UINT8 data;
#endif

    if((NULL == dev->priv) || (NULL == pst_channel_change))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3501h_private *)dev->priv;  
  
#ifndef FPGA_PLATFORM
    c3501h_tp_lock_time = osal_get_tick();
#endif

    freq = pst_channel_change->freq;
    sym = pst_channel_change->sym;
    fec = pst_channel_change->fec; 

    NIM_PRINTF("Enter Fuction %s : Freq = %d, Sym = %d, Fec = %d, change_type = %d\r\n ", __FUNCTION__, freq, sym, fec, pst_channel_change->change_type);

#ifdef C3501H_DEBUG_FLAG
	nim_c3501h_set_debug_flag(dev, freq, sym);
#endif
    //------------------------------------------------------------------------------//

	//begin:add by robin for tsn			
#ifdef TSN_SUPPORT
	priv->tsn->tsn_operate_tpye = pst_channel_change->tsn->tsn_operate_tpye;
	TSN_PRINTF("[%s %d]priv->tsn->tsn_operate_tpye=%d\n ", __FUNCTION__, __LINE__, priv->tsn->tsn_operate_tpye);
	TSN_PRINTF("[%s %d]Force to ACM mode!\n", __FUNCTION__, __LINE__);
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
	data &= 0x3F;  
    nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

	nim_c3501h_tsn_enable(dev, FALSE);//disable tsn filter
    if (TSN_SET_TP_AND_TSN == pst_channel_change->tsn->tsn_operate_tpye)// Set tp and tsn
    {
        TSN_PRINTF("[%s %d]pst_channel_change->tsn->tsn_enable=%d, pst_channel_change->tsn->tsn_write = %d\n ", 
			__FUNCTION__, __LINE__, pst_channel_change->tsn->tsn_enable, pst_channel_change->tsn->tsn_write);
		
        if(SUCCESS != nim_c3501h_set_tsn(dev, pst_channel_change->tsn))
        {
        	TSN_PRINTF("[%s %d]set tsn error!\r\n", __FUNCTION__, __LINE__);
        }
    }
	else if (TSN_SET_TP_AND_GET_TSN == pst_channel_change->tsn->tsn_operate_tpye)//Set tp and get tsn
	{
		TSN_PRINTF("[%s %d]disable tsn filter\n ", __FUNCTION__, __LINE__);		
		//nim_c3501h_tsn_enable(dev, FALSE);//disable tsn filter
		osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);					
		priv->tsn->get_finish_flag = 0;
		osal_mutex_unlock(priv->tsn->tsn_mutex);
	}
#endif
	//end
	
    // Set isid
    if((2 == pst_channel_change->change_type) || (1 == pst_channel_change->change_type))
    {
        NIM_PRINTF("Enter Fuction %s : Set isid Only, isid = %d\r\n ", __FUNCTION__, pst_channel_change->isid->isid_write);
        if(SUCCESS != nim_c3501h_set_isid(dev, pst_channel_change->isid))
        {
        	ERR_PRINTF("[%s %d]set isid error!\r\n", __FUNCTION__, __LINE__);
			return ERR_FAILED;
        }

        // Set isid only
        if (2 == pst_channel_change->change_type)
            return SUCCESS;
    }
	
    // Param  check
	if((0 == freq) || (0 == sym))
    {   
	    ERR_PRINTF("Exit channel change param ERROR in %s \r\n",__FUNCTION__);
	    return ERR_FAILED;
    }

    tp_scan_param = (struct nim_c3501h_tp_scan_para *) comm_malloc(sizeof(struct nim_c3501h_tp_scan_para));
    
	if (NULL == tp_scan_param)
	{
		ERR_PRINTF("Alloc tp_scan_param memory error!/n");
		return ERR_NO_MEM;
	}
    
    comm_memset((int *)tp_scan_param, 0, sizeof(struct nim_c3501h_tp_scan_para));

	//reset first
	nim_c3501h_reset_fsm(dev); 

  // initial param
	priv->plsn.super_scan = pst_channel_change->super_scan;
#ifdef FPGA_PLATFORM
    priv->plsn.plsn_algo_preset = pst_channel_change->search_algo_preset;
#else
    priv->plsn.plsn_algo_preset = 0x31;

  #endif
	priv->plsn.plsn_now = pst_channel_change->plsn_db;	
	priv->plsn.plsn_report = pst_channel_change->plsn_db;
	priv->plsn.plsn_finish_flag = 0;
	priv->plsn.plsn_hw_en = 1;
	
	NIM_PRINTF("priv->plsn.super_scan=%d, priv->plsn.plsn_now=%d, priv->plsn.plsn_hw_en = %d\r\n", priv->plsn.super_scan, priv->plsn.plsn_now,priv->plsn.plsn_hw_en);

	priv->plsn.plsn_try = 0;
    nim_c3501h_set_plsn(dev);
    priv->isid->get_finish_flag = 0;

    tp_scan_param->est_freq = freq * 1024;
    tp_scan_param->est_rs = sym;
    tp_scan_param->est_fec= fec;
    
	priv->search_type = NIM_OPTR_CHL_CHANGE;
    priv->ul_status.lock_status = 3; // tp unlock

#ifdef IMPLUSE_NOISE_IMPROVED
        while(priv->t_param.auto_adaptive_doing)
        {
            comm_sleep(10);
            i++;
            if (i > 10)
                break;
        }
        NIM_PRINTF("Wait auto adaptive timeout = %d\n", i);
        priv->t_param.auto_adaptive_cnt = 0;
        priv->t_param.auto_adaptive_state = 0;

        nim_c3501h_set_demap_llr_shift(dev, 0);
#endif	

	// capture freq
#ifdef DEBUG_CHANNEL_CHANGE
	nim_c3501h_cap(dev, freq,  fft_energy_store,  sym);
	nim_c3501h_cap(dev, freq,  fft_energy_store,  0);
#endif	

#ifdef CHANNEL_CHANGE_ASYNC
	UINT32 flag_ptn = 0;
	#ifdef __TDS__
	if (NIM_FLAG_WAIT(&flag_ptn, priv->flag_id, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0) == OSAL_E_OK)
	#else
	flag_ptn = nim_flag_read(&priv->flag_lock, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
	if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
	#endif
	{
		// channel chaning, stop the old changing first.
		stop_start = osal_get_tick();
		priv->ul_status.c3501h_chanscan_stop_flag = 1;
		NIM_PRINTF("channel changing already, stop it first\r\n");//autoscan force cancel and start TP scan 
		while (0 == priv->wait_chanlock_finish_flag)
		{
			comm_sleep(1);
			if (osal_get_tick() - stop_start > 15)
			{
				NIM_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				break;
			}
		}
		
		priv->plsn.plsn_finish_flag = 0;
	}
	NIM_PRINTF("CHANNEL_CHANGE_ASYNC Enable\r\n");
#endif
	priv->ul_status.phase_err_check_status = 0;
	priv->ul_status.m_setting_freq = freq;

    //if(tp_scan_param->est_rs < 1500) 
    if(tp_scan_param->est_rs < 1800) // 112.9/64 about 1800 not 1500
    {
        nim_c3501h_set_dsp_clk (dev, 0);     // Here use 77M, because 13.5M have performace loss
    }
    else 
    {
        nim_c3501h_set_dsp_clk (dev, 1);    // 112.9M 
    }
#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
    nim_c3501h_set_work_mode(dev, 0x03);
#endif
    nim_c3501h_cfg_tuner_get_freq_err(dev, tp_scan_param);
	nim_c3501h_set_adc(dev, 0);

	nim_c3501h_clear_interrupt(dev);
#ifdef FC_SEARCH_RANGE_LIMITED
    nim_c3501h_tso_off(dev);
#endif

	nim_c3501h_freeze_fsm(dev, 0, 0);
	nim_c3501h_preset_fsm(dev, 0, 0);  

	//reset fsm
	nim_c3501h_reset_fsm(dev);

	// hardware timeout setting
	nim_c3501h_set_hw_timeout(dev, 0xff);

#ifdef ANTI_WIMAX_INTF
    if ((freq > 1680) & (freq < 1700) & (sym > 28000))    
    {
		nim_c3501h_set_agc1(dev, 1, NIM_OPTR_CHL_CHANGE, NIM_FRZ_AGC1_OPEN);
    }
	else
	{
		nim_c3501h_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);
	}
#else
	nim_c3501h_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE, NIM_FRZ_AGC1_OPEN);
#endif

	nim_c3501h_set_roll_off(dev, 1, 2);
	nim_c3501h_set_rs(dev, sym);
	nim_c3501h_tr_setting(dev, NIM_OPTR_CHL_CHANGE, sym);
    nim_c3501h_pl_setting(dev, NIM_OPTR_CHL_CHANGE, sym);
	nim_c3501h_cr_setting(dev, NIM_OPTR_CHL_CHANGE, sym);

#ifdef ES_TEST
    if (0 == sym%2)
    {   
        libc_printf("Set c3501h fc/rs search range as channel change\n");
        nim_c3501h_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
    	nim_c3501h_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    	nim_c3501h_set_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    }
    else
    {
        libc_printf("Set c3501h fc/rs search range as soft search\n");
        nim_c3501h_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    	nim_c3501h_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, sym);
    	nim_c3501h_set_rs_search_range(dev, NIM_OPTR_SOFT_SEARCH, sym);
    }
#else
	nim_c3501h_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
    nim_c3501h_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    nim_c3501h_set_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
#endif

	// Set carry offset
    nim_c3501h_set_freq_offset(dev, tp_scan_param->delfreq);

	// LDPC parameter
	//nim_c3501h_fec_set_ldpc(dev, NIM_OPTR_CHL_CHANGE, 0x00, 0x01);  // No need

	// paladin.ye 2015-11-12	
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic	
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	nim_c3501h_tso_dummy_on(dev);
#endif

	//nim_c3501h_tso_soft_cbr_off(dev); // Move it to hw init
	//nim_c3501h_cr_adaptive_configure (dev, sym);  // No need

	// Begin lock
	nim_c3501h_start_fsm(dev);
	c3501h_tp_lock_time = osal_get_tick();

#if 0
	//force to stop the current search and start the next search
	if (priv->plsn.super_scan)
	{
		stop_start = osal_get_tick();
		priv->plsn.search_plsn_force_stop = 1;
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		while (1==priv->plsn.start_search)
		{			
			comm_sleep(2);
			if (osal_get_tick() - stop_start > 6000)
			{
				PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				break;
			}
		}
		PLSN_PRINTF("[%s %d]stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
		priv->plsn.search_plsn_force_stop = 0;
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		priv->plsn.start_search = 1;		
	}
#endif

#if 0
	// re-initial NEW CR TAB
    nim_c3501h_cr_new_modcod_table_init(dev,sym);
    nim_c3501h_cr_new_adaptive_unlock_monitor(dev);
    nim_c3501h_nframe_step_tso_setting(dev, sym, 0x01);

	//  CR Feedback Carrier Threshold setting	
	data = 0x00; 
	nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1);
#endif
	//priv->cur_freq = freq * 1024; // 20170825
	priv->cur_freq = tp_scan_param->est_freq;
	priv->cur_sym = sym;
	
#if defined CHANNEL_CHANGE_ASYNC && !defined NIM_S3501_ASCAN_TOOLS && !defined FPGA_PLATFORM
    priv->change_type = pst_channel_change->change_type;
	if ((pst_channel_change->isid != NULL) && (priv->change_type != 0))
	{
		comm_memcpy(priv->isid, pst_channel_change->isid, sizeof(struct nim_dvbs_isid));
	}
	NIM_PRINTF("set flag NIM_FLAG_CHN_CHG_START\r\n");
	#ifdef __TDS__
	NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHG_START);
	#else
	nim_flag_set(&priv->flag_lock, NIM_FLAG_CHN_CHG_START);
	#endif
#else
	if (NULL == pst_channel_change->isid)
	{
		pst_channel_change->isid = priv->isid;
	}
	nim_c3501h_waiting_channel_lock(dev, tp_scan_param->est_freq, sym, pst_channel_change->change_type, pst_channel_change->isid);
    #ifdef FORCE_WORK_MODE_IN_CHANGE_TP
    nim_c3501h_set_work_mode(dev, 0x03);
    #endif
#endif

	NIM_PRINTF("    Leave Fuction nim_c3501h_channel_change \r\n");
	//priv->isid->get_finish_flag = 0;
	//priv->ul_status.c3501h_chanscan_stop_flag = 0;

#if 0
	#ifdef ANTI_WIMAX_INTF
		if(force_adpt_disable == 1)
		{
			nim_c3501h_cr_new_adaptive_unlock_monitor(dev);
			nim_c3501h_nframe_step_tso_setting(dev,100,0x00);
		}
		else
		{
			nim_c3501h_task_tso_setting(dev,100,0x00);
		}
	#else		 
		nim_c3501h_cr_new_adaptive_unlock_monitor(dev);
		nim_c3501h_nframe_step_tso_setting(dev,100,0x00);
	#endif
#endif

	comm_free(tp_scan_param);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 change_type, struct nim_dvbs_isid *p_isid)
*
* Description: After nim_c3501h_channel_change called, we need to call nim_c3501h_waiting_channel_lock to wait lock and do some action
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 sym
*  Parameter4: UINT8 change_type
*  Parameter5: struct nim_dvbs_isid *p_isid
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 change_type, struct nim_dvbs_isid *p_isid)
{
	UINT32 timeout = 0, locktimes = 200;
	INT32 tempFreq = 0;
	UINT8 work_mode = 3;
	UINT16 intdata = 0;
    UINT8 lock = 0;
	struct nim_c3501h_private *priv = NULL;
	INT32 result;
	UINT32 lock_time_s = 0;
	UINT8 try_index = 0;
	plsn_state state = PLSN_UNSUPPORT;
	UINT32 search_start_single = 0; //record timeout for one time
	UINT32 search_single_timeout = 0;
	UINT32 search_start_total = 0; //record timeout for total
	UINT32 search_total_timeout = 0;
	UINT32 stop_start = 0;//used for exit while loop
    UINT8 err_flag = 0; 
	UINT8 data_tmp = 0;
#ifdef NIM_PRINTF_DEBUG
	UINT32 start_w = 0;
#endif
	UINT8 pl_find_reset_fsm = 0;

#ifdef FIND_VALUE_FIRST
	UINT8 use_default_flag = 1;
#endif

#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
    UINT32 pl_find_thr = 0;
    UINT32 time_tr_lock = 0;
    UINT8 pl_find_flag = 0;
    UINT8 tr_lock_flag = 0;
    UINT8 change_work_mode_flag = 0;
    UINT8 fsm_state = 0;
#endif


    // Check 
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3501h_private *) dev->priv;   
	if(NULL == priv)
    {
        ERR_PRINTF("[%s %d]NULL == priv\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }
	priv->ul_status.c3501h_chanscan_stop_flag = 0;
    
    //NIM_PRINTF("Enter Fuction %s : Freq = %d, Sym = %d, change_type = %d \r\n ", __FUNCTION__, freq, sym, change_type);

    // Set default
#ifndef FPGA_PLATFORM
	p_isid->get_finish_flag = 0;
	p_isid->isid_num = 0;
#endif

    priv->bb_header_info->stream_mode = 3;
    priv->bb_header_info->stream_type = 1;
    priv->bb_header_info->acm_ccm = 1;

    // Set timeout
	if (sym > 40000)
		locktimes = 204;
	else if (sym < 2000)
		locktimes = 2000;
	else if (sym < 4000)
		locktimes = 1604 - sym / 40;
	else if (sym < 6500)
		locktimes = 1004 - sym / 60;
	else
		locktimes = 604 - sym / 100;

	// Unit is ms
	locktimes *= 2;	
	locktimes += COMPEN_MULTISWITCH_DELAY;   // For some multiswitch,  it need more time to make signal steady.

#ifdef ES_TEST
    locktimes += 5000; 
#endif

#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
    pl_find_thr = (900*1024)/sym;   // More margn for channel change
    if(pl_find_thr < 300)
    {
        pl_find_thr = 300;
    }
#endif

	search_start_single = osal_get_tick();
	search_start_total = osal_get_tick();
	lock_time_s = osal_get_tick();
	PLSN_PRINTF("[%s %d]: lock_time_s = %d \r\n", __FUNCTION__, __LINE__, lock_time_s);

	while (priv->ul_status.c3501h_chanscan_stop_flag == 0)
	{
	    timeout = osal_get_tick() - lock_time_s;
		
		if (locktimes < timeout)// hardware timeout, the first timeout will used the upper level's plsn  
		{	
		    if(err_flag == 0)
		    {
			    PLSN_PRINTF("[%s %d]: enter ERR_HANDLE, locktimes = %dms, timeout = %dms \r\n", __FUNCTION__, __LINE__, locktimes, timeout);
		    }
			err_flag = 1;
			goto ERR_HANDLE;
		}

#ifdef FIND_VALUE_FIRST//when use default value can't lock, and have find new plsn, use this value
		if (use_default_flag)
		{
			state = nim_c3501h_try_plsn(dev, &try_index);
			if (PLSN_FIND == state)
			{
				PLSN_PRINTF("[%s %d]reinit the start time, try_index=%d!\n", __FUNCTION__, __LINE__, try_index);
				use_default_flag = 0;
				lock_time_s = osal_get_tick();//reinit the start time
				PLSN_PRINTF("[%s %d]: reinit lock_time_s = %d \r\n", __FUNCTION__, __LINE__, lock_time_s);
				err_flag = 0;
			}
		}
#endif
    nim_c3501h_get_lock_reg(dev, &intdata);

#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
        // Tr lock
        if ((0x40 == (intdata&0x40)) && (0 == tr_lock_flag))
        {
            tr_lock_flag = 1;
            time_tr_lock = osal_get_tick();
         }

        // PL find
        if (0 == change_work_mode_flag)
        {      
            if((0x80 == (intdata&0x80)) && (pl_find_flag == 0) && (tr_lock_flag))     
            {
                pl_find_flag = 1;
                NIM_PRINTF("*********C3501H PL Find, timeout %d, intdata = 0x%x ************\n", timeout, intdata);
#if (defined(ES_TEST) && !defined(C3501H_DEBUG_FLAG))   
                libc_printf("*********C3501H PL Find, timeout %d, intdata = 0x%x ************\n", timeout, intdata);
#endif
				if(0x100 == (intdata&0x100))
				{
					pl_find_reset_fsm = 1; //when PL_LOCK=1, not need to reset FSM.
					NIM_PRINTF("		PL_LOCK = 1\n");
				}
#if 1
               if(pl_find_reset_fsm == 1)
				{
					//force to stop the current search and start the next search
					if (priv->plsn.super_scan)
					{
						stop_start = osal_get_tick();
						priv->plsn.search_plsn_force_stop = 1;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						while (1==priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}
						PLSN_PRINTF("[%s %d]stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						priv->plsn.plsn_same_cnt = 0;
						priv->plsn.plsn_same_value = 262144; //initialize with an invalid value by yiping 20170918
						priv->plsn.start_search = 1;
						priv->plsn.plsn_patch_flag = 0; // For spec plsn value:262141 in BBH algo 20180106 paladin
					}
					search_start_single = osal_get_tick();
					search_start_total = osal_get_tick();
               	}
#endif

				if(pl_find_reset_fsm == 0)
				{
				#ifdef NIM_PRINTF_DEBUG
				    start_w = osal_get_tick();
				#endif
				    while(1)
				    {
				        //get PL_SYNC_CAPTURED_FRAME
					    nim_reg_read(dev, C3501H_R70_PL_RPT_1 + 2, &data_tmp, 1);
					    if(data_tmp > 0)
					    {
						    NIM_PRINTF("read pl_frame_cnt = %d , cost %d ms \n", data_tmp, osal_get_tick() - start_w);
					        pl_find_reset_fsm = 1;
					        pl_find_flag = 0;
					        tr_lock_flag = 0;
					        // Set HW
                            nim_c3501h_reset_fsm(dev);
                            NIM_PRINTF("Reset FSM \n");
                            // Start search
                            nim_c3501h_start_fsm(dev);
							lock_time_s = osal_get_tick();//reinit the start time
							NIM_PRINTF("[%s %d]: reinit lock_time_s = %d \r\n", __FUNCTION__, __LINE__, lock_time_s);
						    break;
					    }
					    else
					    {
					        //get PL_TIMEOUT
                            nim_reg_read(dev, C3501H_R6C_PL_RPT , &data_tmp, 1);
							if (data_tmp&0x10) //pl_timeout
							{
								NIM_PRINTF("[%s %d]read pl_frame_cnt fail, pl_timeout = 1, cost %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - start_w);
								break;
							}
							else
							{
							    comm_sleep(5);
							}
						}
				    }
				}

            }

            // Can't find pl
            if ((0 == pl_find_flag) && ((osal_get_tick() - time_tr_lock) > pl_find_thr) && tr_lock_flag)
            {         
                // Set force DVB-S mode timeout
                change_work_mode_flag = 1;
                nim_c3501h_reg_get_fsm_state(dev, &fsm_state);

                // Force DVB-S mode
                nim_c3501h_set_work_mode(dev, 0x00);
                NIM_PRINTF("Now change to DVB-S mode in waiting channel lock, fsm_state: 0x%x\n", fsm_state);

                if (fsm_state < 8) 
                {
                    // Set HW
                    nim_c3501h_reset_fsm(dev);
                    NIM_PRINTF("Reset FSM \n");
                    // Start search
                    nim_c3501h_start_fsm(dev);   
                } 
            }
        }
        
#endif

#ifdef RPT_WORK_MODE
		nim_c3501h_reg_get_work_mode(dev, &work_mode);
		if (0 == work_mode)
			NIM_PRINTF("Lock DVB-S tp cost %d ms time\r\n", osal_get_tick() - c3501h_tp_lock_time);
		else if (1 == work_mode)
			NIM_PRINTF("Lock DVB-S2 tp cost %d ms time\r\n", osal_get_tick() - c3501h_tp_lock_time);
		else if (2 == work_mode)
			NIM_PRINTF("Lock DVB-S2x tp cost %d ms time\r\n", osal_get_tick() - c3501h_tp_lock_time);
#endif

        if (0x01 == (intdata&0x01))	// If lock, get and print TP information
		{	 
			NIM_PRINTF("*********Lock tp cost %d ms time*********\r\n", osal_get_tick() - c3501h_tp_lock_time);
#if (defined(ES_TEST) && !defined(C3501H_DEBUG_FLAG))	 
			libc_printf("*********Lock tp cost %d ms time*********\r\n", osal_get_tick() - c3501h_tp_lock_time);
#endif

		    //force to stop to search PLSN
			stop_start = osal_get_tick();
			priv->plsn.search_plsn_force_stop = 1;//force stop
			PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
			while (1 == priv->plsn.start_search)
			{	
				comm_sleep(2);
				if (osal_get_tick() - stop_start > 6000)
				{
					PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
					break;
				}
			}	
			PLSN_PRINTF("[%s %d]: stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
			priv->plsn.search_plsn_force_stop = 0;
			PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
			
            if (priv->plsn.plsn_try)
            {
                priv->plsn.plsn_report = priv->plsn.plsn_try_val;
            }
            else
            {
                priv->plsn.plsn_report = nim_c3501h_get_plsn(dev);
            }
			priv->plsn.plsn_finish_flag = 1;
            
#ifdef FC_SEARCH_RANGE_LIMITED
            nim_c3501h_get_lock_check_limited(dev, &lock); // Only for update tso on/off
#endif

            nim_c3501h_reg_get_work_mode(dev, &work_mode);

			//begin:robin add for tsn
			if (TSN_SET_TP_AND_GET_TSN == priv->tsn->tsn_operate_tpye)
			{
				if (WIDEBAND == nim_c3501h_get_wideband_mode(dev))
				{
					//nim_c3501h_get_tsn(dev, priv->tsn);//just for test
					osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);					
					priv->tsn->get_finish_flag = 1;
					TSN_PRINTF("[%s %d]exit waiting channel lock\n", __FUNCTION__, __LINE__);
					osal_mutex_unlock(priv->tsn->tsn_mutex);
					nim_c3501h_clear_interrupt(dev);
					return SUCCESS;
				}
			}

			osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);
			priv->tsn->get_finish_flag = 1;
			osal_mutex_unlock(priv->tsn->tsn_mutex);
			//end
			
	   		NIM_PRINTF("[%s %d]lock_reg=0x%x, work_mode=%d, count=%d\n", __FUNCTION__, __LINE__, intdata, work_mode, timeout);
            // In DVBS2 & Mult stream mode, we need set isid to hardware by software in auto mode
            if(1 == work_mode)
            {   
            	p_isid->get_finish_flag = 0;
                result = nim_c3501h_get_isid_after_lock(dev, change_type, p_isid);
				if ((result != SUCCESS) && (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag))
				{
					p_isid->get_finish_flag = 0;
					
					NIM_PRINTF("[%s %d]result=%d, p_isid->get_finish_flag=%d\r\n", 
					__FUNCTION__, __LINE__, result, p_isid->get_finish_flag);
					
					return result;
				}	
            }    

			if (priv->isid != p_isid)
			{
				comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
			}
			
			priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
			priv->ul_status.m_cur_freq = tempFreq;
			priv->ul_status.c3501h_chanscan_stop_flag = 0;

#if 1
			// ------ Get information start
            nim_c3501h_reg_get_freqoffset(dev, &tempFreq);
            priv->ul_status.m_real_freq = tempFreq + freq;
			NIM_PRINTF("Tuner real freq is %d, lock freq is %d\r\n", freq, priv->ul_status.m_real_freq);

#ifdef C3501H_DEBUG_FLAG
            nim_c3501h_get_tp_info(dev);
            nim_c3501h_printf_tp_info(dev);
#endif

			// ------ Get information end

		#ifdef NIM_S3501_ASCAN_TOOLS
			tmp_lock = 0x01;
		#endif			
			//  CR Feedback Carrier Threshold setting, for tai5 3441/v/1555 loss lock
			#if 0	// Fix TP loss lock(pdpd 3430/h/1250) issue
			if(rs < 3000)
			{
				data = 0x08; //using high threshold after lock 
				nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1);
			}
			#endif
			/* Keep current frequency.*/
			priv->ul_status.m_cur_freq = tempFreq;
			nim_c3501h_clear_interrupt(dev);
			priv->ul_status.c3501h_chanscan_stop_flag = 0;			

			p_isid->get_finish_flag = 1;
			if (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
			{
				ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3501h_chanscan_stop_flag=%d,\r\n\
					priv->ul_status.c3501h_autoscan_stop_flag=%d\r\n", 
					__FUNCTION__, __LINE__, priv->ul_status.c3501h_chanscan_stop_flag,
					priv->ul_status.c3501h_autoscan_stop_flag);
				p_isid->get_finish_flag = 0;
				return ERR_FAILED;
			}
			NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\r\n", 
					__FUNCTION__, __LINE__, p_isid->get_finish_flag);
#endif
			NIM_PRINTF("[%s %d]return success!\n", __FUNCTION__, __LINE__);
			return SUCCESS;
		}
		else
		{
			/*if (priv->ul_status.c3501h_chanscan_stop_flag) 
			{
				priv->ul_status.c3501h_chanscan_stop_flag = 0;
				priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
				p_isid->get_finish_flag = 1;			
				return ERR_FAILED;
			}*/
			
			if (priv->ul_status.c3501h_chanscan_stop_flag)
			{								
				priv->ul_status.c3501h_chanscan_stop_flag = 0;
				priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
				priv->plsn.plsn_finish_flag = 1;
#ifndef FPGA_PLATFORM
				p_isid->get_finish_flag = 1;
				NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\r\n", 
					__FUNCTION__, __LINE__, p_isid->get_finish_flag);
#endif
				return ERR_FAILED;
			}
			comm_sleep(1);
			
			// For monitor lock status
            if (0 == (timeout%50))
			{
			    NIM_PRINTF("wait lock... loop = %d, cr04 = 0x%x\r\n", timeout, intdata);
                //nim_c3501h_reg_get_fsm_state(dev, &fsm_state);
			}
		}

ERR_HANDLE:
		if (err_flag)
		{
			nim_c3501h_get_search_timeout(dev, &search_single_timeout, &search_total_timeout);
				
			state = nim_c3501h_try_plsn(dev, &try_index);
					
			if (PLSN_FIND == state) 
			{
				PLSN_PRINTF("[%s %d]: reinit the start time!\n", __FUNCTION__, __LINE__);
				lock_time_s = osal_get_tick();//reinit the start time
				PLSN_PRINTF("[%s %d]: reinit lock_time_s = %d \r\n", __FUNCTION__, __LINE__, lock_time_s);
				err_flag = 0;
			}
			else if ((PLSN_OVER == state) || (PLSN_RUNNING == state))
			{
				comm_sleep(2);//sleep, so the system can dispatch search task
				if ((PLSN_OVER == state) || (osal_get_tick() - search_start_single > search_single_timeout))
				{
					if(priv->plsn.plsn_same_cnt >= 2)
					{
						PLSN_PRINTF("[%s %d]: plsn_same_cnt = %d \n", __FUNCTION__, __LINE__, priv->plsn.plsn_same_cnt);
						PLSN_PRINTF("[%s %d]: state=%d! \n", __FUNCTION__, __LINE__, state);
						nim_c3501h_clear_interrupt(dev);
						PLSN_PRINTF("[%s %d]: timeout= %d locktimes =%d\n", __FUNCTION__, __LINE__, timeout, locktimes);
						PLSN_PRINTF("[%s %d]: Lock tp fail cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - c3501h_tp_lock_time);
						PLSN_PRINTF("[%s %d]: lock fail, search cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
						priv->ul_status.c3501h_chanscan_stop_flag = 0;
						priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
						p_isid->get_finish_flag = 1;
						//NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, p_isid->get_finish_flag);
	
						priv->plsn.plsn_finish_flag = 1;
						//PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);

						stop_start = osal_get_tick();
						priv->plsn.search_plsn_force_stop = 1;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d!\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						while (1 == priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}	
						PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

						return ERR_FAILED;
					}
					else if (osal_get_tick() - search_start_total < search_total_timeout)
					{
						PLSN_PRINTF("[%s %d]: start next search plsn, state=%d!, search_single_timeout=%d ms, search_total_timeout=%d ms, timeout=%d ms\n", 
								__FUNCTION__, __LINE__, state, search_single_timeout, search_total_timeout, osal_get_tick() - search_start_single); 			
										
						stop_start = osal_get_tick();
						priv->plsn.search_plsn_force_stop = 1;//force stop
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						while (1 == priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}	
						PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

						PLSN_PRINTF("[%s %d]: reset demod before next search!\n", __FUNCTION__, __LINE__);
						//reset demod
						nim_c3501h_reset_fsm(dev); 
						priv->plsn.plsn_try = 0;
						nim_c3501h_set_plsn(dev);
						comm_sleep(2);
						//start demod
						nim_c3501h_start_fsm(dev);
							
						priv->plsn.start_search = 1;
						priv->plsn.search_plsn_stop = 0;//to avoid get wrong state when plsn task not start immediate
						PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_stop);
						try_index = 0;//reinit the try index
						PLSN_PRINTF("[%s %d]: reinit try index!\n", __FUNCTION__, __LINE__);
						comm_sleep(5);//sleep, so the system can dispatch search task
						lock_time_s = osal_get_tick();//reinit the start time
						search_start_single = osal_get_tick();
					}
					else
					{
						PLSN_PRINTF("[%s %d]: state=%d!\n", __FUNCTION__, __LINE__, state);//robin
						nim_c3501h_clear_interrupt(dev);
						PLSN_PRINTF("[%s %d]: timeout= %d locktimes =%d\n", __FUNCTION__, __LINE__, timeout, locktimes);
						PLSN_PRINTF("[%s %d]: Lock tp fail cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - c3501h_tp_lock_time);
						PLSN_PRINTF("[%s %d]: lock fail, search cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
						priv->ul_status.c3501h_chanscan_stop_flag = 0;
						priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
						p_isid->get_finish_flag = 1;
						//NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, p_isid->get_finish_flag);
	
						priv->plsn.plsn_finish_flag = 1;
						//PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);

						stop_start = osal_get_tick();
						priv->plsn.search_plsn_force_stop = 1;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d!\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						while (1 == priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}	
						PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

						return ERR_FAILED;
					}
				}
			}
			else
			{
				PLSN_PRINTF("[%s %d]: state=%d!\n", __FUNCTION__, __LINE__, state);//robin
				nim_c3501h_clear_interrupt(dev);
				PLSN_PRINTF("[%s %d]: timeout= %d locktimes =%d\n", __FUNCTION__, __LINE__, timeout, locktimes);
				PLSN_PRINTF("[%s %d]: Lock tp fail cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - c3501h_tp_lock_time);
				PLSN_PRINTF("[%s %d]: lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
				priv->ul_status.c3501h_chanscan_stop_flag = 0;
				priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
				p_isid->get_finish_flag = 1;
				//NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, p_isid->get_finish_flag);
	
				priv->plsn.plsn_finish_flag = 1;
				//PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);

				stop_start = osal_get_tick();
				priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d!\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				while (1 == priv->plsn.start_search)
				{			
					comm_sleep(2);
					if (osal_get_tick() - stop_start > 6000)
					{
						PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
						break;
					}
				}	
				PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
				priv->plsn.search_plsn_force_stop = 0;
				PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

				return ERR_FAILED;
			}
		}
		
	}

	priv->ul_status.c3501h_chanscan_stop_flag = 0;
	priv->plsn.plsn_finish_flag = 1;
#ifndef FPGA_PLATFORM
	p_isid->get_finish_flag = 1;
#endif
	NIM_PRINTF("[%s %d]Leave!\r\n", __FUNCTION__, __LINE__);
	return SUCCESS;

}



/*****************************************************************************
*  INT32 nim_c3501h_get_search_timeout(struct nim_device *dev, UINT32 *single_timeout, UINT32 *total_timeout)
* Description: For plsn search used 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: single_timeout, uint is ms
*  Parameter3: total_timeout, uint is ms, some is same with parameter2, some is 2 times parameter2
* Return Value: result
*****************************************************************************/
INT32 nim_c3501h_get_search_timeout(struct nim_device *dev, UINT32 *single_timeout, UINT32 *total_timeout)
{
	struct nim_c3501h_private *priv = NULL;

	if(NULL == dev)
	{
		ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
		return ERR_NO_DEV;
	}
	priv = (struct nim_c3501h_private *) dev->priv;   
	if(NULL == priv)
	{
		ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}	

	if (priv->plsn.super_scan)//only in super_scan mode, it will start search plsn
	{
		if (5 == priv->plsn.search_algo) //NP
		{
			*single_timeout = 20000; // 20s
			*total_timeout = 20000;
		}
		else //others
		{
			*single_timeout = 10000; // 20s
			*total_timeout = 20000;
		}
	}
	else
	{
		*single_timeout = 0;
		*total_timeout = 0;
	}

	#ifdef FOR_QT_IC_TEST
		*total_timeout = 40000;	// By yiping
	#endif

	return SUCCESS;
}





/*****************************************************************************
*  INT32 nim_c3501h_autoscan_init(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
* 	autoscan initial function
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3501h_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_autoscan_init(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
{
    //UINT8 data = 0;
    UINT32 i = 0;

    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);

	// For some multiswitch,  it need more time to make signal steady
    comm_sleep(COMPEN_MULTISWITCH_DELAY);
	
    last_tp_freq = 0;

	params->priv->search_type = NIM_OPTR_SOFT_SEARCH;
    params->priv->ul_status.lock_status = 3;

#ifdef AUTOSCAN_DEBUG
	config_data = params->priv->tuner_config_data.qpsk_config;
	params->priv->tuner_config_data.qpsk_config |= C3501H_SIGNAL_DISPLAY_LIN;
#endif

#ifdef CHANNEL_CHANGE_ASYNC
	UINT32 flag_ptn = 0;
#ifdef __TDS__
	if(NIM_FLAG_WAIT(&flag_ptn, params->priv->flag_id, NIM_FLAG_CHN_CHG_START|NIM_FLAG_CHN_CHANGING,OSAL_TWF_ORW, 0) == OSAL_E_OK)
#else
	flag_ptn = nim_flag_read(&params->priv->flag_lock, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
	if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
#endif
	{
		params->priv->ul_status.c3501h_chanscan_stop_flag = 1;
		comm_sleep(10);
	}
#endif
    params->pst_auto_scan->isid_num = 0;
    params->pst_auto_scan->isid = 0;

	params->priv->plsn.plsn_algo_preset = 0x31;
	params->priv->plsn.plsn_hw_en = 1;

	params->priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;

    // For auto adaptive
#ifdef IMPLUSE_NOISE_IMPROVED	
    params->priv->t_param.auto_adaptive_cnt = 0;
    params->priv->t_param.auto_adaptive_state = 0;
#endif	
    // end


#ifdef IMPLUSE_NOISE_IMPROVED
    while(params->priv->t_param.auto_adaptive_doing)
    {
        comm_sleep(10);
        i++;
        if (i > 10)
            break;
    }
    NIM_PRINTF("Wait auto adaptive timeout = %d\n", i);
    params->priv->t_param.auto_adaptive_cnt = 0;
    params->priv->t_param.auto_adaptive_state = 0;
    
    nim_c3501h_set_demap_llr_shift(dev, 0);
#endif	


	// reset HW FSM
	nim_c3501h_reset_fsm(dev);
	params->priv->ul_status.c3501h_autoscan_stop_flag = 0;

	//nim_c3501h_cr_setting(dev, NIM_OPTR_SOFT_SEARCH, 30000);

    #ifdef USE_CHANNEL_CHANGE_PARA 
	nim_c3501h_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
    #else
    nim_c3501h_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    #endif
	nim_c3501h_set_roll_off(dev, 1, 2);
    nim_c3501h_set_dsp_clk (dev, 1);    // 112.9M 
    nim_c3501h_set_adc(dev, 1);
    nim_c3501h_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT, NIM_FRZ_AGC1_OPEN);

/////////////////////////////////////////////////////////////////////////////////    
    #ifdef UNKNOW_FUNCTION
	//nim_c3501h_fec_set_ldpc(dev, NIM_OPTR_SOFT_SEARCH, 0x00, 0x01); // No need
    #endif
	nim_c3501h_tso_on(dev);
    // Disable software control TS output speed function.
#ifdef C3501H_ERRJ_LOCK
    nim_m3501c_recover_moerrj(dev);
#endif
    nim_c3501h_tso_dummy_off(dev);
/////////////////////////////////////////////////////////////////////////////////    

	last_tuner_if = 0;
	chlspec_num = 0;
	called_num = 0;	
	max_fft_energy = 0;

	channel_spectrum = (INT32 *) comm_malloc(FS_MAXNUM * 4);
	if(channel_spectrum == NULL)
	{
		AUTOSCAN_PRINTF("\n channel_spectrum--> no enough memory!\n");
		return ERR_NO_MEM;
	}

	channel_spectrum_tmp = (INT32 *)comm_malloc(FS_MAXNUM * 4);
	if(channel_spectrum_tmp == NULL)
	{
		AUTOSCAN_PRINTF("\n channel_spectrum_tmp--> no enough memory!\n");
		comm_free(channel_spectrum);
		return ERR_NO_MEM;
	}

    if (nim_c3501h_get_bypass_buffer(dev))
    {
        AUTOSCAN_PRINTF("\n ADCdata--> no enough memory!\n");
        comm_free(channel_spectrum);
        comm_free(channel_spectrum_tmp);
        return ERR_NO_MEM;
    }

    return SUCCESS;
}

                                                                                                                                        
/*****************************************************************************                                                          
*  INT32 nim_c3501h_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)                              
* 	autoscan get wideband spectrum by fft                                                                                               
*                                                                                                                                       
* Arguments:                                                                                                                            
*  Parameter1: struct nim_device *dev                                                                                                   
*  Parameter2: struct nim_c3501h_autoscan_params *params                                                                                 
*                                                                                                                                       
* Return Value: INT32                                                                                                                   
*****************************************************************************/                                                          
INT32 nim_c3501h_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)                                 
{                                                                                                                                       
    INT32 i = 0;                                                                                                                        
    UINT32 step_freq = 0;                                                                                                               
    INT32 channel_freq_err = 0;                                                                                                         
    INT32 fft_shift_num = 0;                                                                                                            
    INT32 scan_mode = 0;                                                                                                                
    INT32 energy_step = 0;                                                                                                              
                                                                                                                                       
    if((NULL == dev) || (NULL == params))                                                                                               
    {                                                                                                                                   
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);                                                                       
        return ERR_NO_DEV;                                                                                                              
    }                                                                                                                                   
	                                                                                                                                    
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);                                                                                       
                                                                                                                                        
	params->adc_sample_freq = 112;    
                                                                                                                                        
	if(IS_M3031 == params->priv->tuner_type)	                                                                                        
	{                                                                                                                                   
		step_freq = params->adc_sample_freq/4;                                                                                          
	}                                                                                                                                   
	else                                                                                                                                
	{                   
	    //step_freq = params->adc_sample_freq/2;  // 96.4
		step_freq = params->adc_sample_freq/4;  // 112.9                                                                                  
	}                                                                                                                                   
                                                                                                                                    
    AUTOSCAN_PRINTF("sfreq = %d, efreq = %d, step_freq = %d, adc_sample_freq =%d\n", params->pst_auto_scan->sfreq, params->pst_auto_scan->efreq, step_freq, params->adc_sample_freq);
                                                                                                                                        
	// start cap fft data and calculate the tp                                                                                          
    for (params->fft_freq = params->pst_auto_scan->sfreq; params->fft_freq < params->pst_auto_scan->efreq; \
        params->fft_freq += step_freq)                                                                                               
    {        
        
		comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));                                                                       
		while(1)                                                                                                                        
		{                                                                                                                        
			nim_c3501h_reset_fsm(dev);                                                                  
			nim_c3501h_cap_start(dev, params->fft_freq, 0, fft_energy_1024);   
            channel_freq_err = 0;
			call_tuner_command(dev, NIM_TUNER_GET_C3031B_FREQ_ERR, &channel_freq_err);                                                                                                                                                                          
                                                                                                                                        
            // make sure fft_energy is ok                                                                                               
            for(i = 0; i < 1024; i++)                                                                                                   
            {                                                                                                                           
                if(fft_energy_1024[i] > 0)                                                                                              
                    break;                                                                                                              
            }                                                                                                                           
            if(i < 1024)                                                                                                                
                break;                                                                                                                  
                                                                                                                                        
			if(params->priv->ul_status.c3501h_autoscan_stop_flag)                                                                        
			{                                                                                                                                                                                                                                                   
				comm_free(channel_spectrum);                                                                                            
				comm_free(channel_spectrum_tmp);                                                                                        
				comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);                                                         
				AUTOSCAN_PRINTF("\tleave fuction nim_c3501h_autoscan\n");                                                                
				return USER_STOP;                                                                                                       
			}                                                                                                                           
		}                                                                                                                               
                                                                                                                                        
        //remove DC component parts, need before correct freq err, paladin add 20160621                                                 
        energy_step = fft_energy_1024[511+2] - fft_energy_1024[511-2];                                                                  
        //energy_step >>=2;
        energy_step /=4;
        for(i=(511-2); i<(511+2); i++)                                                                                                  
            fft_energy_1024[i+1] = fft_energy_1024[i] + energy_step;                                                                    
                                                                                                                                        
        // correct the fft point with the freq err                                                                                      
		if(IS_M3031 == params->priv->tuner_type)                                                                                        
		{                                                                                                                               
			fft_shift_num = (4*channel_freq_err + params->adc_sample_freq/2) / params->adc_sample_freq;                                 
			AUTOSCAN_PRINTF("channel_freq_err = %d, FFT_shift_num = %d \n", channel_freq_err, fft_shift_num);                           
                                                                                                                                        
			if(0 == fft_shift_num) // nothing to do                                                                                     
				;                                                                                                                       
			else if(fft_shift_num > 0) // cut end                                                                                       
			{                                                                                                                           
				for (i = 1023; i >= 0; i--)                                                                                             
				{                                                                                                                       
					if(i<fft_shift_num)                                                                                                 
					fft_energy_1024[i] = 0;                                                                                             
					else                                                                                                                
					fft_energy_1024[i] = fft_energy_1024[i - fft_shift_num];                                                            
				}                                                                                                                       
			}                                                                                                                           
			else //cut start                                                                                                            
			{                                                                                                                           
				fft_shift_num = -fft_shift_num;                                                                                         
				for (i = 0; i < 1024; i++)                                                                                              
				{                                                                                                                       
				    if((i + fft_shift_num) > 1023)                                                                                      
					    fft_energy_1024[i] = 0;                                                                                         
				    else                                                                                                                
					    fft_energy_1024[i] = fft_energy_1024[i + fft_shift_num];                                                        
				}                                                                                                                       
			}                                                                                                                           
		}                                                                                                                               
		called_num++;                                                                                                                   
                                                                                                                              
    #if (defined(FFT_PRINTF_DEBUG)&& defined(NIM_GET_1024FFT))                                                                          
        if(params->priv->debug_flag & 0x20)                                                                                                                                      
        {                                                                                                                               
            FFT_PRINTF("call_num = %d, Tuner_IF = %d \n", called_num, params->fft_freq);                                                
            FFT_PRINTF("FFT1024 point without median filter\n");                                                                        
            for(i = 0; i < 1024; i++)                                                                                                   
                FFT_PRINTF("FFT1024-> %4d :  %d \n", i, fft_energy_1024[i]);                                                            
        }                                                                                                                               
    #endif                                                                                                                              
                                                                                                                                        
        if(NIM_SCAN_SLOW == params->priv->blscan_mode)                                                                                  
        {                                                                                                                               
            scan_mode = 1;                                                                                                              
        }                                                                                                                               
        else                                                                                                                            
        {                                                                                                                               
            scan_mode = 0;                                                                                                              
        }                                                                                                                               
                                                                                                                                        
        //nim_s3501_smoothfilter3(1024,fft_energy_1024);                                                                                
        nim_s3501_median_filter(1024, fft_energy_1024, scan_mode); // Median filter for restrain single tone noise                      
        //nim_s3501_smoothfilter3(1024,fft_energy_1024);                                                                                

    #if 0
        if(IS_M3031 == params->priv->tuner_type)                                                                                        
        {                                                                                                                               
            nim_s3501_fft_wideband_scan_m3031(dev, params->fft_freq, params->adc_sample_freq);                                          
		}                                                                                                                               
		else                                                                                                                            
		{                                                                                                                               
			nim_s3501_fft_wideband_scan(dev, params->fft_freq, params->adc_sample_freq);                                                
		}  
    #else
        {
            nim_s3501_fft_wideband_scan(dev, params->fft_freq, params->adc_sample_freq);     
        }
    #endif
        
		if(params->priv->ul_status.c3501h_autoscan_stop_flag)                                                                            
		{                                                                                                                                                                                                                                                           
			comm_free(channel_spectrum);                                                                                                
			comm_free(channel_spectrum_tmp);                                                                                            
			comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);                                                             
			AUTOSCAN_PRINTF("\tleave fuction nim_c3501h_autoscan\n");                                                                    
			return USER_STOP;                                                                                                           
		}                                                                                                                               
	}                                                                                                                                   
                                                                                                                                                                                                                                                                        
    comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);                                                                     
                                                                                                                                        
#ifdef FFT_PRINTF_DEBUG                                                                                                                 
    if(params->priv->debug_flag & 0x40)                                                                                       
    {                                                                                                                                   
        for(i = 0; i < chlspec_num; i++)                                                                                                
            FFT_PRINTF("FFT_WBS-->%d :      %d\n", i, channel_spectrum[i]);                                                             
    }                                                                                                                                   
#endif                                                                                                                                  
                                                                                                                                        
#ifdef AUTOSCAN_DEBUG                                                                                                                   
    params->end_t = os_get_tick_count();                                                                                                
    AUTOSCAN_PRINTF("\tGet wideband spectrum by FFT cost time      %dms\n", params->end_t - params->start_t);                           
    params->temp_t = params->end_t;                                                                                                     
#endif                                                                                                                                  
                                                                                                                                        
    return SUCCESS;                                                                                                                     
}                                                                                                                                       


/*****************************************************************************
*  INT32 nim_c3501h_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
* 	autoscan estimate tp from wideband spectrum
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3501h_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
{
    INT32 loop = 1;
    INT32 i = 0;
    INT32 loop_index = 0;
#ifdef DEBUG_SOFT_SEARCH    
    INT32 bug_freq[2]={0};
    INT32 bug_sym[2]={0};
#endif

    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);
	AUTOSCAN_PRINTF("max_fft_energy = %d\n",max_fft_energy);
	loop = 1;
	if(max_fft_energy > 8388607)
	{
		loop = 2;
	}
	AUTOSCAN_PRINTF("************loop=%d****************\n",loop);
	if(loop == 2)
	{
		for(i = 0;i < chlspec_num;i++)
		{
			channel_spectrum_tmp[i]= channel_spectrum[i]/(8388607>>5);

			if(channel_spectrum_tmp[i] <= 0)
				channel_spectrum_tmp[i] = 1;
			else if(channel_spectrum_tmp[i] > 8388607)
				channel_spectrum_tmp[i] = 8388607;

			if (channel_spectrum[i] > 8388607)    // max 23bit = 0x7fffff
				channel_spectrum[i] = 8388607;
			else if(channel_spectrum[i] == 0) 
				channel_spectrum[i] = 1;
		}
	}

	////******find TP***
	tp_number = 0;
	comm_memset(frequency_est, 0, sizeof(frequency_est));
	comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));

	for(loop_index = 0; loop_index < loop; loop_index++)
	{
		if(loop_index == 0)
		{
			nim_s3501_search_tp(chlspec_num, channel_spectrum, params->pst_auto_scan->sfreq, \
                params->adc_sample_freq,loop_index);
			AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n",loop_index,tp_number);
		}
		else if(loop_index == 1)
		{
			nim_s3501_search_tp(chlspec_num, channel_spectrum_tmp, params->pst_auto_scan->sfreq, \
                params->adc_sample_freq,loop_index);
			AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n",loop_index,tp_number);
		}
	}
	comm_free(channel_spectrum);
	comm_free(channel_spectrum_tmp);

#ifdef DEBUG_SOFT_SEARCH
	params->success=0;
	tp_number=200;
	bug_freq[0] = 1159056; ////TP1
	bug_sym[0] = 2296;
	bug_freq[1] = 1159056; ////TP2
	bug_sym[1] = 2296;

	comm_memset(frequency_est, 0, sizeof(frequency_est));
	comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));
	for(i=0; i < tp_number; i++)
	{
		frequency_est[i] = bug_freq[i%2];
		symbol_rate_est[i] = bug_sym[i%2];
	}
#endif

	if(tp_number > 0)
    {
        params->success = NIM_SUCCESS;
		AUTOSCAN_PRINTF("\n\n---------------------------------------------- \n");
        AUTOSCAN_PRINTF("Search TP success , TP_number = %d  \n", tp_number);
        for(i = 0; i < tp_number; i++)
            AUTOSCAN_PRINTF("TP_number =   %d  Freq =  %d  Sym =   %d \n", i, frequency_est[i], symbol_rate_est[i]);            
		AUTOSCAN_PRINTF("\n");
		c3501h_real_tp_number = 0;
    	comm_memset(c3501h_real_freq, 0, sizeof(c3501h_real_freq));
    	comm_memset(c3501h_real_sym_rate, 0, sizeof(c3501h_real_sym_rate));
    }
    else
    {
        params->success = NIM_FAILED;
        AUTOSCAN_PRINTF("Search TP failed\n");
    }

#ifdef AUTOSCAN_DEBUG
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("Search TP cost time      %dms\n", params->end_t - params->temp_t);
    params->temp_t = params->end_t;
#endif

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_autoscan_try_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
* 	autoscan try to lock estimate tp
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3501h_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_autoscan_try_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params)
{
    UINT32 last_lock_rs = 0;
    INT32 last_lock_freq = 0;
    INT32 i = 0, j = 0;
    INT32 temp_freq = 0;
    UINT8 cur_fec = 0;
    UINT32 cur_freq = 0;
    UINT32 cur_sym = 0;
    UINT32 success_num = 0;
#ifdef AUTOSCAN_DEBUG	
    UINT32 fail_num = 0;
#endif
    INT32  ret_value = SUCCESS;
    INT32 ret = SUCCESS;
    UINT8 lock = 0;
    UINT32 unlock_cnt = 0;
    UINT8 do_next_tp = 0;
    INT32 lock_tp_idx = 0;
	UINT32 stop_start = 0;
 	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	UINT8 tsn_count = 0;
	UINT8 data;
	
    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);
    
	if (NIM_SUCCESS == params->success)
	{
		last_lock_freq = 0;
		last_lock_rs = 0;
		last_tp_freq = 0;
		final_est_freq = 0;
		final_est_sym_rate = 0;
        
		for (i = 0; i < tp_number; i++)
		{
            #ifdef AUTOSCAN_DEBUG
                params->temp_t = os_get_tick_count();
            #endif 

			AUTOSCAN_PRINTF("\n\n----> Try %d-th TP [freq,Sym]=[%d, %d]\n",i,frequency_est[i],symbol_rate_est[i]);			
			AUTOSCAN_PRINTF("[%s %d]Force to ACM mode!\n", __FUNCTION__, __LINE__);
			nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
			data &= 0x3F;  
		    nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
			nim_c3501h_tsn_enable(dev, FALSE);//disable tsn filter
			if (params->priv->ul_status.c3501h_autoscan_stop_flag)
			{
				AUTOSCAN_PRINTF("\tleave fuction nim_c3501h_autoscan\n");
				return USER_STOP;
			}
			
    		#ifdef DEBUG_SOFT_SEARCH
    			params->priv->ul_status.m_cr_num = 1;
    			params->priv->ul_status.m_freq[0] = frequency_est[i];
    			params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
    		#else
            
    			//if(frequency_est[i]/1024 < (params->pst_auto_scan->sfreq - 5))
    			if(frequency_est[i]/1024 < 945)
                {         
    				AUTOSCAN_PRINTF("Exit_1 search TP[%d][freq,Sym]=[%d, %d] with freq too closed sfreq %d\n", i, frequency_est[i]/1024, symbol_rate_est[i], params->pst_auto_scan->sfreq);
    				continue;
                }

    			
    			//if(frequency_est[i]/1024 > (params->pst_auto_scan->efreq + 5))
    			if(frequency_est[i]/1024 > 2155)
                {         
    				AUTOSCAN_PRINTF("Exit_2 search TP[%d][freq,Sym]=[%d, %d] with freq too closed efreq %d\n", i, frequency_est[i]/1024, symbol_rate_est[i], params->pst_auto_scan->efreq);
    				continue;
                }
                
    			temp_freq = frequency_est[i] - last_lock_freq;

                if (temp_freq < 0)
                  temp_freq = -temp_freq;

                AUTOSCAN_PRINTF("Temp_freq = %d \n", temp_freq);

                if(temp_freq <= 1024)
                {         
    				AUTOSCAN_PRINTF("Exit_3 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i], temp_freq);
    				continue;
                }
                
                if ((last_lock_rs + symbol_rate_est[i]*0.8)/2 > (UINT32)temp_freq)
                {    
                  AUTOSCAN_PRINTF("Exit_6 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
                  continue;
                } 

                if (symbol_rate_est[i]/2 > temp_freq)
                {
    				AUTOSCAN_PRINTF("Exit_4 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
    				continue;
                }
                
                if (last_lock_rs/2 > (UINT32)temp_freq)
                {    
    				AUTOSCAN_PRINTF("Exit_5 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
    				continue;
                }                       

    			params->priv->ul_status.m_cr_num = 1;
    			params->priv->ul_status.m_freq[0] = frequency_est[i];
    			params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
                
		    #endif
			
#if 0
            if (0 != success_num)
            {
                params->priv->ul_status.m_rs[0] = last_lock_rs;
                params->priv->ul_status.m_freq[0] = last_lock_freq;
            }
#endif      
      
			if (SUCCESS == nim_c3501h_channel_search(dev, 0))
			{
#ifdef FC_SEARCH_RANGE_LIMITED
                //nim_c3501h_get_lock_check_limited(dev, &lock);  // Only for update tso on/off
#endif    
				if((final_est_sym_rate<1000)&&(final_est_sym_rate>=990))
					final_est_sym_rate = 1000;					
            
			    success_num++;
            #ifdef AUTOSCAN_DEBUG
                params->end_t = os_get_tick_count();
			    AUTOSCAN_PRINTF("Lock SUCCESS %d-th TP[freq %d , sym %d ] ,Cost time     %d\n", \
                    i, final_est_freq, final_est_sym_rate, params->end_t - params->temp_t);
            #endif 
				nim_c3501h_reg_get_code_rate(dev, &cur_fec);
                cur_sym = final_est_sym_rate;
                last_lock_freq = final_est_freq;
                last_lock_rs = cur_sym;
                priv->cur_sym = cur_sym;
                final_est_freq = (final_est_freq + 512) / 1024;
				cur_freq = final_est_freq;                

				stop_start = osal_get_tick();
				priv->plsn.search_plsn_force_stop = 1;//stop plsn search
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				while (1 == priv->plsn.start_search)
				{			
					comm_sleep(2);
					if (osal_get_tick() - stop_start > 6000)
					{
						PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
						break;
					}
				}	
				PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
				priv->plsn.search_plsn_force_stop = 0;
				PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

				if (priv->plsn.plsn_try)
				{
					priv->plsn.plsn_report = priv->plsn.plsn_try_val;
					params->pst_auto_scan->plsn_db = priv->plsn.plsn_try_val;
				}
				else
				{
					priv->plsn.plsn_report = nim_c3501h_get_plsn(dev);
					params->pst_auto_scan->plsn_db = priv->plsn.plsn_report;
				}
				AUTOSCAN_PRINTF("[%s %d]params->pst_auto_scan->plsn_db=%d!\n", __FUNCTION__, __LINE__, params->pst_auto_scan->plsn_db);//robin
				if (GET_TP_ONLY == priv->autoscan_control)//get tp only
				{
					continue;
				}
				
				if ((cur_freq >= params->pst_auto_scan->sfreq) && (cur_freq <= params->pst_auto_scan->efreq))
				{
					if (params->pst_auto_scan->unicable) //cur_freq -> [950, 2150]
					{
						AUTOSCAN_PRINTF("\tUnicable FFT_freq: %d, cur_freq: %d, Fub: %d\n", params->fft_freq, cur_freq, params->pst_auto_scan->fub);
					#ifdef __TDS__
						ret_value = params->pst_auto_scan->callback(1, 0, params->fft_freq + cur_freq - params->pst_auto_scan->fub, cur_sym, cur_fec);
					#else
						ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, params->fft_freq + cur_freq - params->pst_auto_scan->fub, cur_sym, cur_fec, 0);
					#endif
					}
					else
					{
						if((cur_freq > (last_tp_freq - 2)) && (cur_freq < (last_tp_freq + 2)))
						{
						#ifdef __TDS__
							ret_value = params->pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);						
						#else
							ret_value = nim_callback(params->pst_auto_scan, priv, 0, 0, frequency_est[i]/1024, 0, 0, last_tp_freq);
						#endif
							AUTOSCAN_PRINTF("\tRescan a tp: FFT_freq: %d, last_tp_freq: %d\n", frequency_est[i]/1024, last_tp_freq);
						}
						else
						{	
                            unlock_cnt = 0;
                            do_next_tp = 0;

                            while(1)
                            {    
                                // If open this macro, driver may be report "fail TP" to upper which can not locked. paladin 20160614
                                //#ifdef REPORT_FAIL_TP
                               	#if 1
                                    lock = 1;
                                #else
                                    nim_c3501h_get_lock(dev, &lock);	// Double check lock with bch
                                #endif
                                
                                if(1 == lock)
                                {
                                    lock_tp_idx ++;
                                    // Find the ACM/CCM TP for debug
                                    #ifdef FIND_ACM_TP
                                        nim_c3501h_autoscan_find_acm_tp(dev, lock_tp_idx, cur_freq, cur_sym);
                                        nim_c3501h_printf_tp_info(dev);
                                    #endif

									comm_memset(priv->tsn->tsn_read, 0, sizeof(priv->tsn->tsn_read));
									priv->tsn->tsn_num = 0;
									tsn_count = 0;
									if (WIDEBAND == nim_c3501h_get_wideband_mode(dev))
									{
										TSN_PRINTF("it is wideband, priv->tsn->tsn_num=%d\n", priv->tsn->tsn_num);
										nim_c3501h_get_tsn(dev, priv->tsn);
									}
									
									do
									{
										if (priv->tsn->tsn_num != 0)
										{
											priv->tsn->tsn_write = priv->tsn->tsn_read[tsn_count];
											priv->tsn->tsn_enable = 1;
											nim_c3501h_set_tsn(dev, priv->tsn);
											tsn_count++;
											TSN_PRINTF("[%s %d]tsn_count=%d,priv->tsn->tsn_num=%d\n", __FUNCTION__, __LINE__, tsn_count, priv->tsn->tsn_num);
										}
										
	                                    params->priv->bb_header_info->stream_mode = 3;
	                                    params->priv->bb_header_info->stream_type = 1;
	                                    params->priv->bb_header_info->acm_ccm = 1;

	                                    // Get work mode and config hardware, then check if the TP is multi stream or not, don't get isid yet here
	                                    ret = nim_c3501h_get_isid_after_lock(dev, 0, params->priv->isid);

	                                    if(1 == params->pst_auto_scan->scan_type)
	                                    {
	                                        // Now begin get isid for multi stream autoscan, DVB-S2 && multi stream && get bbheader success
	                                        if(SUCCESS == ret)
	                                        {
	                                            // While get isid error,  jump to next tp
	                                            if(SUCCESS != nim_c3501h_get_isid(dev, params->priv->isid))
	                                            {
	                                                ACM_DEBUG_PRINTF("[%s %d]get isid error!\n", __FUNCTION__, __LINE__);
	                                                do_next_tp = 1;
	                                                break ;   
	                                            }
	                                            
	                                            params->priv->isid->isid_bypass = 0;
	                                            params->priv->isid->isid_mode = 1;

	                                            for(j = 0; j < params->priv->isid->isid_num; j++)
	                                            {
	                                                params->priv->isid->isid_write = params->priv->isid->isid_read[j];

	                                                if(SUCCESS == nim_c3501h_set_isid(dev, params->priv->isid))
	                                                {
	                                                	ACM_DEBUG_PRINTF("[%s %d]cur_freq=%d, cur_sym=%d, plsn=%d\n", __FUNCTION__, __LINE__, cur_freq, cur_sym, params->priv->plsn.plsn_report);
	                                                    #ifdef __TDS__
	                                                        ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec); 
	                                                        //ret_value = params->pst_auto_scan->callback(1, 0, cur_freq + success_num*3, cur_sym, cur_fec);  
	                                                    #else
	                                                        ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
	                                                    #endif
	                                                }
	                                                // While set isid error,  break current "for loop" and  jump to next tp
	                                                else
	                                                {
	                                                    ACM_DEBUG_PRINTF("[%s %d]set isid error!\n", __FUNCTION__, __LINE__);
	                                                    do_next_tp = 1;
	                                                    j = params->priv->isid->isid_num;
	                                                    break ;  
	                                                }
	                                            }
	                                        }
	                                        // DVB-S || single stream || get bbheader fail
	                                        else    
	                                        {
	                                            if (ERR_CRC_HEADER == ret)
	                                            {
	                                                ACM_DEBUG_PRINTF("[%s %d]BBHeader check fail in autosacn try tp, do next tp!\n", __FUNCTION__, __LINE__);
	                                                do_next_tp = 1;
	                                                break ;   
	                                            }
	                                            else
	                                            {
	                                            	ACM_DEBUG_PRINTF("[%s %d]cur_freq=%d, cur_sym=%d, plsn=%d, stream_mode=%d\n", __FUNCTION__, __LINE__, cur_freq, cur_sym, params->priv->plsn.plsn_report, params->priv->bb_header_info->stream_mode);
	                                                #ifdef __TDS__
	                                                    ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);  
	                                                    //ret_value = params->pst_auto_scan->callback(1, 0, cur_freq + success_num*3, cur_sym, cur_fec);  
	                                                #else
	                                                    ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
	                                                #endif
	                                            }
	                                        }    
	                                    }
	                                    else
	                                    {
	                                    	ACM_DEBUG_PRINTF("[%s %d]cur_freq=%d, cur_sym=%d, plsn=%d \n", __FUNCTION__, __LINE__, cur_freq, cur_sym, params->priv->plsn.plsn_report);
	                                        #ifdef __TDS__
	                                            ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);  
	                                            //ret_value = params->pst_auto_scan->callback(1, 0, cur_freq + success_num*3, cur_sym, cur_fec);   
	                                        #else
	                                            ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
	                                        #endif
	                                    }
									}while(tsn_count < priv->tsn->tsn_num);
									break;//exit while (1)
                                }
                                else
                                {
                                    if(unlock_cnt >= 10)//18-2-2017 By Paladin
                                    {
                                        do_next_tp = 1;
                                        //ret = pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);             
                                        AUTOSCAN_PRINTF(" Can not lock TP para fre: %d, rs: %d\n", cur_freq, cur_sym);
                                        break;  
                                    }
                                    else
                                    {
                                        unlock_cnt ++;
                                        comm_sleep(5);
                                    }
                                }   
                            }

                            if(do_next_tp)
                                continue ;   

							AUTOSCAN_PRINTF(" Add TP[TS, single stream] para fre: %d, rs: %d\n", cur_freq, cur_sym);
                            AUTOSCAN_PRINTF("Find Program %d-th TP[freq %d , sym %d ] ---> SUCCESS! ,Cost time        %d\n", i, final_est_freq, final_est_sym_rate, os_get_tick_count() - params->end_t);
                            
							last_tp_freq = cur_freq;
						}
					}
				}
				else
				{
					AUTOSCAN_PRINTF("tleave out of the frequence range %d-th TP[freq%d, sym %d] ,Cost time     %d\n",i,final_est_freq,final_est_sym_rate,os_get_tick_count() - params->end_t);
                    
					if (params->priv->ul_status.c3501h_autoscan_stop_flag)
					{
						AUTOSCAN_PRINTF("\tleave fuction nim_c3501h_autoscan\n");
						return USER_STOP;
					}
					else
					{
						continue ;
					}
				}
			}
			else
			{ 
			    #ifdef AUTOSCAN_DEBUG
					fail_num++;
                    params->end_t = os_get_tick_count();
                    AUTOSCAN_PRINTF("Lock Faild %d-th TP[freq %d , sym %d ] ,Cost time        %d\n",i,(frequency_est[i] + 512)/1024,symbol_rate_est[i],params->end_t - params->temp_t);
                #endif 
				
				#ifdef __TDS__
					ret_value = params->pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);				
				#else
					ret_value = nim_callback(params->pst_auto_scan, priv, 0, 0, frequency_est[i]/1024, 0, 0, 0);
				#endif
				AUTOSCAN_PRINTF("ret2 value is: %d \n", ret_value);

                #ifdef AUTO_SCAN_SAVE_TIME
                if ((0 != final_est_freq) && ((0 != final_est_sym_rate)))
                {
                    last_lock_freq = final_est_freq;
                    last_lock_rs = final_est_sym_rate;
                }
                #endif
                
			}
			
			if (ret_value == 2)
			{
				return ERR_FAILED;
			}
			else if (ret_value == 1)
			{
				goto nim_as_break;
			}
			

		}
	}

#ifdef AUTOSCAN_DEBUG
	params->priv->tuner_config_data.qpsk_config = config_data;
#endif

#ifdef AUTOSCAN_DEBUG
    params->end_t=os_get_tick_count();
    AUTOSCAN_PRINTF("\t autoscan cost time   %d:%d  ,est_tp_num = %d, soft_search_times = %d, success_num = %d, fail_num = %d, re_search_num = %d \n", \
        (((params->end_t - params->start_t)+500)/1000/60), \
        (((params->end_t - params->start_t)+500)/1000 -(((params->end_t - params->start_t)+500)/1000/60)*60), \
        tp_number, search_times, success_num, fail_num, re_search_num);        
#endif    
    
    NIM_PRINTF("\tFinish autoscan\n");
    return SUCCESS;
	
nim_as_break:
	#ifdef __TDS__
		params->pst_auto_scan->callback(2, 0, 0, 0, 0);  /* Tell callback search finished */
	#else
		nim_callback(params->pst_auto_scan, priv, 2, 0, 0, 0, 0, 0);  /* Tell callback search finished */
	#endif
	
    return ret_value;
}



/*****************************************************************************
*  INT32 nim_c3501h_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan)
* 	autoscan function, actually it is blindscan
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct NIM_AUTO_SCAN *pst_auto_scan
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan)
{
    INT32 ret = SUCCESS;
	//UINT8 data = 0;
	
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

    struct nim_c3501h_autoscan_params params;
    
    if((NULL == dev) || (NULL == pst_auto_scan))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
    }

	if (pst_auto_scan->sfreq < AS_FREQ_MIN || pst_auto_scan->efreq > AS_FREQ_MAX || \
		pst_auto_scan->sfreq > pst_auto_scan->efreq)
	{
	    ERR_PRINTF("Exit Auto scan param ERROR in %s \n",__FUNCTION__);
	    ERR_PRINTF("Expect sfreq = %d real = %d , efreq = %d real = %d \n", \
            AS_FREQ_MIN, pst_auto_scan->sfreq, AS_FREQ_MAX, pst_auto_scan->efreq);

        return ERR_PARA;
	}
    
    //nim_c3501h_ioctl(dev, NIM_DRIVER_STOP_ATUOSCAN, 0); // ?? need?

    AUTOSCAN_PRINTF("Enter %s, sfreq = %d, efreq = %d, band = %d\n", __FUNCTION__, pst_auto_scan->sfreq, pst_auto_scan->efreq, pst_auto_scan->freq_band);

	priv->plsn.super_scan = pst_auto_scan->super_scan;
	//priv->plsn.super_scan = 1;
	priv->plsn.plsn_now = 0;
	
	AUTOSCAN_PRINTF("priv->plsn.super_scan=%d\n", priv->plsn.super_scan);
	
    comm_memset(&params, 0, sizeof(params));

    // debug for auto scan time cost
#ifdef AUTOSCAN_DEBUG
    //UINT32 start_t,end_t,temp_t;
    params.start_t = os_get_tick_count();
#endif
 
    params.priv = priv;
    params.pst_auto_scan = pst_auto_scan;
    params.freq_band = pst_auto_scan->freq_band;

	nim_c3501h_tsn_enable(dev, FALSE);//disable tsn filter
	
    ret = nim_c3501h_autoscan_init(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3501h_autoscan_init error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = nim_c3501h_autoscan_get_spectrum(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3501h_autoscan_get_spectrum error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

	if (GET_SPECTRUM_ONLY == priv->autoscan_control) //only get spectrum, below code need not execute
	{
		return SUCCESS;
	}
	
    ret = nim_c3501h_autoscan_estimate_tp(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3501h_autoscan_estimate_tp error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    // set iq swap freq_band
    if (1 == params.freq_band)  // C band used iq swap first
    {
        AUTOSCAN_PRINTF("\n\n\nForce set iq swap\n\n\n");
        nim_c3501h_set_iq_swap(dev, 1, 1);
    }
    
    ret = nim_c3501h_autoscan_try_tp(dev, &params);

    if (1 == params.freq_band)  // Recover
    {
        nim_c3501h_set_iq_swap(dev, 0, 1);
    }

    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3501h_autoscan_try_tp error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    
    return ret;
}

                                                                                                                                                                                                                   
/*****************************************************************************                                    
* INT32 nim_c3501h_soft_search_init( struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param)         
* Description: c3501h soft-searching initial                                                                       
*                                                                                                                 
* Arguments:                                                                                                      
*  Parameter1: struct nim_device *dev                                                                             
*  Parameter2: struct nim_c3501h_tp_scan_para *tp_scan_paraml                                                      
*                                                                                                                 
* Return Value: INT32                                                                                             
*****************************************************************************/                                                                                                                                               
INT32 nim_c3501h_soft_search_init( struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param)           
{                                                                                                                 
	UINT16 data;                                                                                                 
	                                                                        
	if(NULL == tp_scan_param)                                                           
	{                                                                                                             
		ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);                                                 
		return ERR_NO_DEV;                                                                                        
	}                                                                                                             
	                                                                        	                                                                        
    //if(tp_scan_param->est_rs < 1500) 
    if(tp_scan_param->est_rs < 1800) // 112.9/64 about 1800 not 1500    
    {
        nim_c3501h_set_dsp_clk (dev, 0);     // Here use 77M, because 13.5M have performace loss
    }
    else 
    {
        nim_c3501h_set_dsp_clk (dev, 1);    // 112.9M 
    }

    final_est_freq = 0;
    final_est_sym_rate = 0;

    nim_c3501h_set_interrupt_mask(dev, 0xff);                                                                     
	nim_c3501h_clear_interrupt(dev);                                                                          

	data = 0x1ff;    
	nim_c3501h_set_hw_timeout(dev, data); 

	if(0 == tp_scan_param->search_status)
	{
	    #ifdef USE_CHANNEL_CHANGE_PARA
		    nim_c3501h_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE, NIM_FRZ_AGC1_OPEN);////CR07  AGC setting
		#else
		    nim_c3501h_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_SOFT_SEARCH, NIM_FRZ_AGC1_OPEN);////CR07  AGC setting
        #endif
	}
	else
	{
		nim_c3501h_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE, NIM_FRZ_AGC1_OPEN);////CR07  AGC setting   
	}

	nim_c3501h_set_rs(dev, tp_scan_param->est_rs); 
	nim_c3501h_set_freq_offset(dev, tp_scan_param->delfreq);    
    #ifdef USE_CHANNEL_CHANGE_PARA
	    nim_c3501h_tr_setting(dev, NIM_OPTR_CHL_CHANGE, tp_scan_param->est_rs);
        nim_c3501h_pl_setting(dev, NIM_OPTR_CHL_CHANGE, tp_scan_param->est_rs);
        nim_c3501h_cr_setting(dev, NIM_OPTR_CHL_CHANGE, tp_scan_param->est_rs);
    	nim_c3501h_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, tp_scan_param->est_rs);                              
    	nim_c3501h_set_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, tp_scan_param->est_rs); 
    #else
	    nim_c3501h_tr_setting(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);
        nim_c3501h_pl_setting(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);
        nim_c3501h_cr_setting(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);
    	nim_c3501h_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);                              
    	nim_c3501h_set_rs_search_range(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs); 
    #endif
	//nim_c3501h_tso_soft_cbr_off(dev);  // Move it to hw init 

     // FSM state monitor for PL Lock find
    //data = 0x87;
    //nim_reg_write(dev, R6F_FSM_STATE, &data, 1);	
    
	// paladin.ye 2015-11-12    
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic 
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)        
	nim_c3501h_tso_dummy_on(dev);  
#endif

	tp_scan_param->change_work_mode_flag = 0;

	if(0 == tp_scan_param->search_status)   
	{                                                                                                     
		if(tp_scan_param->est_rs < 2500)                                                                          
			tp_scan_param->tr_lock_thr = 20;
		else                                                                                                      
			tp_scan_param->tr_lock_thr = 12 ;
		                                                                        
		if(tp_scan_param->est_rs <= 3000)                                                                         
			tp_scan_param->cr_lock_thr = 60;                                                                      
		else if(tp_scan_param->est_rs < 10000)                                                                    
			tp_scan_param->cr_lock_thr = 40 ;                                                                      
		else                                                                                                      
			tp_scan_param->cr_lock_thr = 30;                                                                      

		if(tp_scan_param->est_rs < 4000)                                                                          
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 50;                                         
		else                                                                                                      
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 30; 
	}
	else
	{
		if(tp_scan_param->est_rs < 2500)                                                                          
			tp_scan_param->tr_lock_thr = 16;
		else                                                                                                      
			tp_scan_param->tr_lock_thr = 8; 

		if (tp_scan_param->est_rs <= 2000)                                                                          
			tp_scan_param->cr_lock_thr = 70;                                                                      
		else if (tp_scan_param->est_rs <= 3000)                                                                                                          
			tp_scan_param->cr_lock_thr = 60; 
		else if (tp_scan_param->est_rs <= 10000)                                                                                                          
			tp_scan_param->cr_lock_thr = 40 ; 
		else
			tp_scan_param->cr_lock_thr = 30;

		if(tp_scan_param->est_rs < 4000)                                                                          
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 50;                                         
		else                                                                                                      
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 30 ;
    }

    // Max 400k symbol wait for pl find, the margin must be > 50% for noise
    tp_scan_param->pl_lock_thr = (600*1024)/tp_scan_param->est_rs;
    tp_scan_param->pl_lock_thr = (tp_scan_param->pl_lock_thr)/25;
    if(tp_scan_param->pl_lock_thr < 10)
        tp_scan_param->pl_lock_thr = 10;  
	
	return SUCCESS;                                                                                               
}                                                                                                                 


/*****************************************************************************
* INT32 nim_c3501h_soft_search(struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param)
* Description: c3501h soft-searching EST TP, used for autoscan
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3501h_tp_scan_para *tp_scan_paraml
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_soft_search(struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param)
{
	UINT8 data = 0;
	//UINT8 lock_status = 0;
	UINT8 intindex = 0;
    UINT16 lock_state = 0;
    UINT8 fsm_state = 0;
	UINT8 intdata = 0;
	UINT8 lock_monitor = 0;
	UINT8 modcod = 0;
	UINT32 tempFreq = 0;
	UINT32 Rs, rs_rev=0;
	UINT8 code_rate, map_type, work_mode;//ver_data,
	UINT32 timeout;
	INT32 tr_lock_num;
	INT32 cr_lock_num;
	UINT32 tr_lock_flag = 0;
	INT32 last_freq, last_rs, del_freq, del_rs; // from UINT32 to INT32 by russell
	UINT32 s2_lock_cnt = 0;

	UINT8 try_index = 0;
	plsn_state state = PLSN_UNSUPPORT;

	UINT32 search_start_single = 0; //record timeout for one time
	UINT32 search_single_timeout = 0;
	UINT32 search_start_total = 0; //record timeout for total
	UINT32 search_total_timeout = 0;
	
	UINT8 err_flag = 0; 

	UINT8 pl_find_flag = 0;
  	//UINT8 cr_loss_cnt = 0;
    UINT8 tr_loss_cnt = 0;
	UINT32 stop_start = 0;//used for exit while loop
	//UINT32 search_timeout;

	UINT8 data_tmp = 0;
#ifdef NIM_PRINTF_DEBUG
	UINT32 start_w = 0;
#endif
	UINT8 pl_find_reset_fsm = 0;

#ifdef FIND_VALUE_FIRST
	UINT8 use_default_flag = 1;
#endif

#ifdef AUTOSCAN_DEBUG
    UINT32 fsm_begin_time = 0;
#endif
    INT32 channel_freq_err = 0;

	AUTOSCAN_PRINTF("[%s %d]Enter\n", __FUNCTION__, __LINE__);
	
	if((NULL == dev->priv) || (NULL == tp_scan_param))
	{
		ERR_PRINTF("Exit with ERR_NO_DEV in %s\n",__FUNCTION__);
		return ERR_NO_DEV;
	}

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	AUTOSCAN_PRINTF("In %s Try TP : Freq = %d, RS = %d, delfc = %d, freq_err = %d\n",\
	__FUNCTION__, tp_scan_param->est_freq, tp_scan_param->est_rs, tp_scan_param->delfreq, tp_scan_param->freq_err);

	if (priv->plsn.super_scan)
	{
		priv->plsn.plsn_try = 0;
		priv->plsn.plsn_now = 0;
		nim_c3501h_set_plsn(dev);
	}
	
	//reset first
	nim_c3501h_reset_fsm(dev);
	comm_delay(500);

	// initial hardware and set/get some parameter
	tp_scan_param->search_status = 0;
	nim_c3501h_soft_search_init(dev, tp_scan_param);
	nim_c3501h_set_work_mode(dev, 0x03);

	// start search
	nim_c3501h_start_fsm(dev);
#ifdef AUTOSCAN_DEBUG
    fsm_begin_time = osal_get_tick();
#endif

	search_start_single = osal_get_tick();
	search_start_total = osal_get_tick();
	
	timeout = 0;
	lock_monitor = 0;
	tr_lock_num = 0;
	cr_lock_num = 0;
	last_freq = 0;
	last_rs = 0;

	while (1)
	{
		if ((tp_scan_param->fs_lock_thr + 20) < timeout)
		{	
			AUTOSCAN_PRINTF("[%s %d]Timeout: %d\n", __FUNCTION__, __LINE__, timeout);
			err_flag = 1;
			goto ERR_HANDLE;
		}

		if (priv->ul_status.c3501h_autoscan_stop_flag)
		{
			stop_start = osal_get_tick();
			priv->plsn.search_plsn_force_stop = 1;
			PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
			while (1 == priv->plsn.start_search)
			{			
				comm_sleep(2);
				if (osal_get_tick() - stop_start > 6000)
				{
					PLSN_PRINTF("[%s %d]: force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
					break;
				}
			}	
			PLSN_PRINTF("[%s %d]: stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
			priv->plsn.search_plsn_force_stop = 0;
			PLSN_PRINTF("[%s %d]: priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

			AUTOSCAN_PRINTF("\tleave fuction nim_c3501h_soft_search\n");
			return USER_STOP;
		}
		
	#ifdef FIND_VALUE_FIRST//when use default value can't lock, and have find new plsn, use this value
		if (use_default_flag)
		{
			state = nim_c3501h_try_plsn(dev, &try_index);
			if (FIND == state)
			{
				PLSN_PRINTF("[%s %d]try_index=%d\n", __FUNCTION__, __LINE__, try_index);
				PLSN_PRINTF("[%s %d]reinit time!\n", __FUNCTION__, __LINE__);
				use_default_flag = 0;
				timeout = 0;//reinit the start time
				err_flag = 0;
				lock_monitor = 0;
				tr_lock_flag = 0;
				tr_lock_num = 0;
				cr_lock_num = 0;
				//cr_loss_cnt = 0;		
                tr_loss_cnt = 0;	
				nim_c3501h_clear_interrupt(dev);
			}
		}
	#endif
		
		timeout ++ ;
		comm_sleep(25);

		// get demod lock status
		lock_state = 0;
        nim_c3501h_get_lock_reg(dev, &lock_state);
        
		if (lock_state & 0x100)
			s2_lock_cnt++;
		else
			s2_lock_cnt = 0;


#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
        // PL find
        if (0 == tp_scan_param->change_work_mode_flag)
        {      
            if((0x80 == (lock_state&0x80)) && (pl_find_flag == 0) && (tr_lock_flag))     
            {
                pl_find_flag = 1;
				data = 0x00;
				nim_reg_read(dev, C3501H_R6C_PL_RPT + 1, &data, 1);
                NIM_PRINTF("*********C3501H PL Find, timeout %d , pl_find_indicator = 0x%x ************\n", timeout, (data&0x0c));
#if (defined(ES_TEST) && !defined(C3501H_DEBUG_FLAG))   
                libc_printf("*********C3501H PL Find, timeout %d , pl_find_indicator = 0x%x ************\n", timeout, (data&0x0c));
#endif
				if(0x100 == (lock_state&0x100))
				{
					pl_find_reset_fsm = 1; //when PL_LOCK=1, not need to reset FSM.
					NIM_PRINTF("		PL_LOCK = 1\n");
				}

#if 1			
				if(pl_find_reset_fsm == 1)
				{
				    //force to stop & re-start
				    if (priv->plsn.super_scan)
				    {
					    stop_start = osal_get_tick();
					    priv->plsn.search_plsn_force_stop = 1;	
					    PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
					    while (1==priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}
						PLSN_PRINTF("[%s %d]stop cost=%d ms \n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						try_index = 0;
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						priv->plsn.plsn_same_cnt = 0;
						priv->plsn.plsn_same_value = 262144; //initialize with an invalid value by yiping 20170918
						priv->plsn.start_search = 1;//added by robin, start to search plsn
						priv->plsn.auto_scan_start = 1;
						priv->plsn.plsn_patch_flag = 0; // For spec plsn value:262141 in BBH algo 20180106 paladin
						PLSN_PRINTF("priv->plsn.auto_scan_start=%d\n", priv->plsn.auto_scan_start);
					}
					search_start_single = osal_get_tick();
					search_start_total = osal_get_tick();
				}
#endif
				
				if(pl_find_reset_fsm == 0)
				{
				#ifdef NIM_PRINTF_DEBUG
				    start_w = osal_get_tick();
				#endif
				    while(1)
				    {
				        //get PL_SYNC_CAPTURED_FRAME
					    nim_reg_read(dev, C3501H_R70_PL_RPT_1 + 2, &data_tmp, 1);
					    if(data_tmp > 0)
					    {
						    AUTOSCAN_PRINTF("read pl_frame_cnt = %d , cost %d ms \n", data_tmp, osal_get_tick() - start_w);
					        pl_find_reset_fsm = 1;
					        pl_find_flag = 0;
					        //tr_lock_flag = 0;
							lock_monitor = 0;
    		   				cr_lock_num = 0;
							tr_lock_num = 0;
    		   				timeout = 0;
					        // Set HW
                            nim_c3501h_reset_fsm(dev);
                            AUTOSCAN_PRINTF("Reset FSM \n");
                            // Start search
                            nim_c3501h_start_fsm(dev);
						    break;
					    }
					    else
					    {
					        //get PL_TIMEOUT
                            nim_reg_read(dev, C3501H_R6C_PL_RPT , &data_tmp, 1);
							if (data_tmp&0x10) //pl_timeout
							{
								AUTOSCAN_PRINTF("[%s %d]read pl_frame_cnt fail, pl_timeout = 1, cost %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - start_w);
								break;
							}
							else
							{
							    comm_sleep(5);
							}
						}
				    }
				}


				}

            // Can't find pl
            if ((0 == pl_find_flag) && (timeout > tp_scan_param->pl_lock_thr) && (tp_scan_param->change_work_mode_flag == 0) && tr_lock_flag)
            {         
                // Set force DVB-S mode timeout
                tp_scan_param->change_work_mode_flag = 1;
                nim_c3501h_reg_get_fsm_state(dev, &fsm_state);

                // Force DVB-S mode
                nim_c3501h_set_work_mode(dev, 0x00);
                AUTOSCAN_PRINTF("Now change to DVB-S mode TP[freq %d , sym %d] timeout %d, lock_status = 0x%x, fsm_state = 0x%x\n",  \
                 tp_scan_param->est_freq, tp_scan_param->est_rs, timeout, intindex, fsm_state);

                if (fsm_state < 8) 
                {
                    // Set HW
                    nim_c3501h_reset_fsm(dev);
                    AUTOSCAN_PRINTF("Reset FSM \n");
                    // Start search
                    nim_c3501h_start_fsm(dev);   
                } 
    		   	lock_monitor = 0;
    		   	cr_lock_num = 0;
				tr_lock_num = 0;
    		   	timeout = 0;
            }
        }
        
#endif

		//FS_LOCK or CR_lock -- c3505
		//ch lock or cr lock or s2 fec lock or vb sync or des lock
		if(lock_state & 0x7401)
		{
			nim_c3501h_get_symbol_rate(dev, &Rs);
			tempFreq = tp_scan_param->est_freq;
			nim_c3501h_get_freq(dev, &tempFreq);
			nim_c3501h_reg_get_work_mode(dev, &work_mode);
			del_freq = tempFreq - last_freq;
			if (del_freq < 0)
				del_freq = -del_freq;
			
			del_rs = Rs - last_rs;
			if (del_rs < 0)
				del_rs = -del_rs;
			
			//if(del_freq <= 2048)
			 //if(del_freq <= 2048)
	        if ((del_freq <= 100) && (lock_state & 0x400))
	        {   
	            lock_monitor = lock_monitor | lock_state;
	            cr_lock_num++;
                final_est_freq = tempFreq;
	        }				
			else
			{
				cr_lock_num = 0;
				last_freq = tempFreq;
				last_rs = Rs;
			}
			AUTOSCAN_PRINTF("       current lock Rs is %d,  Freq is %d at time %d\n",Rs, tempFreq,timeout);

			del_rs = rs_rev-Rs;
			if(del_rs < 0)
				del_rs = -del_rs;

			//if((Rs >= 1000)&&(del_rs >= 100))
			if((Rs >= (RS_LIMITED - 1))&&(del_rs >= 100))
				rs_rev=Rs;
			
			Rs=rs_rev;

			//nim_reg_read(dev, R04_STATUS, &lock_status, 1);
			//if (((0 == work_mode) || (1 == work_mode) || (s2_lock_cnt > 2)) && (0x3f == (0x3f&data)))
			if (lock_state & 0x1)
			{
                AUTOSCAN_PRINTF("Path2 lock TP, cost:%dms \n", osal_get_tick() - fsm_begin_time);
            
				NIM_PRINTF("    path2:lock chanel \n");
				NIM_PRINTF("            Freq is %d\n", tempFreq);
				priv->ul_status.m_cur_freq = tempFreq ;
				NIM_PRINTF("            Rs is %d\n", Rs);
				nim_c3501h_reg_get_code_rate(dev, &code_rate);
				NIM_PRINTF("            code_rate is %d\n", code_rate);
				NIM_PRINTF("            work_mode is %d\n", work_mode);
				nim_c3501h_reg_get_map_type(dev, &map_type);
				NIM_PRINTF("            map_type is %d\n", map_type);
				AUTOSCAN_PRINTF("\tLock freq %d Rs %d with del_Rs %d del_f %d \n",\
				tempFreq, Rs, Rs - tp_scan_param->est_rs, tempFreq - tp_scan_param->est_freq);

				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);

				AUTOSCAN_PRINTF("work_mode= %d, s2_lock_cnt= %d \n",work_mode,s2_lock_cnt);
				//final_est_freq = (tempFreq + 512) / 1024;
				final_est_freq = tempFreq;
				final_est_sym_rate = Rs;

				c3501h_real_freq[c3501h_real_tp_number] = tempFreq/1024;//store the frequency and sysbolrate used by NIM_DRIVER_GET_VALID_FREQ	 
				c3501h_real_sym_rate[c3501h_real_tp_number] =Rs;				
				c3501h_real_tp_number = c3501h_real_tp_number + 1;
#ifdef NIM_S3501_ASCAN_TOOLS
				nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP,0x00,tempFreq, Rs, tempFreq>1550?1:0);
#endif
				nim_c3501h_clear_interrupt(dev);
				AUTOSCAN_PRINTF("[%s %d]timeout = %d, s2_lock_cnt=%d\n", __FUNCTION__, __LINE__, timeout, s2_lock_cnt);
				return SUCCESS;
			}
		}

		//TR_lock
		if((lock_state&0x40) == 0x40)
		{
#ifdef AUTOSCAN_SPEEDUP
            nim_c3501h_get_symbol_rate(dev, &Rs);
            if (Rs >= (RS_LIMITED - 1))
            {
                lock_monitor = lock_monitor | 0x40;
                tr_lock_num++;
                final_est_sym_rate = Rs;        
            }
            else
            {
                tr_lock_num--;
                AUTOSCAN_PRINTF("        RS=%d. Lower than RS_LIMITED: %d \n", Rs, RS_LIMITED);
            }
#else
            lock_monitor = lock_monitor | 0x40;
            tr_lock_num++;
            nim_c3501h_get_symbol_rate(dev, &Rs);
            final_est_sym_rate = Rs;
#endif
           
			if (tr_lock_flag==0)
			{
				AUTOSCAN_PRINTF("		TR lock for the first time: Rs = %d , timeout = %d \n", Rs, timeout);

				if(Rs < (RS_LIMITED - 1)) //for the most TPs (their symbolrates all are 1000Msps), RS_RPTs are 999Msps after TR locks.
				{
					tr_lock_num--;
					if (tr_lock_num < 0)
                    	tr_lock_num = 0;
				}
				else // when lock the tr, refresh the RS value and set it to demod , now not to tuner ????
				{
					tr_lock_flag = 1;
					tp_scan_param->est_rs = Rs;
                    
					nim_c3501h_reset_fsm(dev);
                    nim_c3501h_tuner_control(dev, tp_scan_param->tuner_freq, tp_scan_param->est_rs + tp_scan_param->correct_rs, &channel_freq_err);
                    
					// initial hardware and set/get some parameter
					tp_scan_param->search_status = 1;
					nim_c3501h_soft_search_init(dev, tp_scan_param);

					// set DVB-S2 Physical Layer Frame Sync detector time out threshold
					#if 0
					if (CHIP_ID_3503D != priv->ul_status.m_c3501h_type)
                    {               
    					data = 0x00;
    					nim_reg_write(dev, R28_PL_TIMEOUT_BND,&data,1);
    					nim_reg_read(dev,R28_PL_TIMEOUT_BND+1,&data,1);
    					data &= 0xe0;
    					data |= 0x01;
    					nim_reg_write(dev,R28_PL_TIMEOUT_BND+1,&data,1);
                    }
                    #endif
					// start search
					nim_c3501h_start_fsm(dev);

					AUTOSCAN_PRINTF("Reset: freq = %d, Rs = %d at time %d\n", tp_scan_param->est_freq, Rs, timeout);
					lock_monitor = 0;
					tr_lock_num = 0;
					cr_lock_num = 0;
					rs_rev = Rs;
					timeout = 0;
				}
			}
		}

		// AUTOSCAN_PRINTF("     lock_monitor is 0x%x\n", lock_monitor );

		// check the time out value
		if(tp_scan_param->tr_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->tr_lock_thr=%d, timeout=%d\n", tp_scan_param->tr_lock_thr, timeout);
			if((lock_monitor&0x40) == 0x00) // TR never lock
			{
			#if 0
				AUTOSCAN_PRINTF("        Fault TP, exit without TR lock, time out %d\n", timeout);
				AUTOSCAN_PRINTF("        Fault TP, exit without TR lock, time out %d\n", timeout);

				err_flag = 1;
				goto ERR_HANDLE;
            #endif
            
                nim_c3501h_clear_interrupt(dev);

				stop_start = osal_get_tick();
				priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				while (1==priv->plsn.start_search)
				{			
					comm_sleep(2);
					if (osal_get_tick() - stop_start > 6000)
					{
						libc_printf("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
						break;
					}
				}
				PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				priv->plsn.search_plsn_force_stop = 0;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

				PLSN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
                return ERR_FAILED; 
			}
		}
		else if(tp_scan_param->cr_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->cr_lock_thr=%d, timeout=%d\n", tp_scan_param->cr_lock_thr, timeout);
            // ch lock or cr lock or pl lock or pl find or s2 fec lock or vb sync or des lock
            #ifdef AUTOSCAN_SPEEDUP
			    if(((lock_monitor&0x7581) == 0x00) ||  (tr_lock_num < 10)) // Maybe or is better? paladin
			#else
			    if(((lock_monitor&0x7581) == 0x00) &&  (tr_lock_num < 10)) // TR  lock
			#endif
			{
				AUTOSCAN_PRINTF("        Fault TP, exit without CR lock,  tr_lock_num %d, time out %d\n", tr_lock_num, timeout);

                #if 1
    				err_flag = 1;
    				goto ERR_HANDLE;
                #else	// exit immediately, but maybe wrong plsn cause cr unlock
                    nim_c3501h_clear_interrupt(dev);
                    priv->plsn.search_plsn_force_stop = 1;
    				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
                    PLSN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
                    return ERR_FAILED; 
                #endif     
			}
		}
		else if(tp_scan_param->fs_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->fs_lock_thr=%d, timeout=%d\n", tp_scan_param->fs_lock_thr, timeout);
			if(((lock_monitor&0x7181) == 0x00) &  (cr_lock_num < 10)) // CR  lock
			{
				AUTOSCAN_PRINTF("        Fault TP, exit without Frame lock, cr_lock_num %d, time out %d\n", cr_lock_num, timeout);
				err_flag = 1;
				goto ERR_HANDLE;
			}
		}

		// Get the interrupt status 
		nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1 + 3, &intindex, 1);
		//if (0 == (intindex & 0x80))
		if (0x80 == (intindex & 0x80))
		{
            nim_c3501h_get_interrupt(dev, &intdata);
			nim_c3501h_clear_interrupt(dev);

			AUTOSCAN_PRINTF("       Interrupt register is 0x%02x\n", intdata);
			if (0x04 == (intdata & 0x04))
			{
				AUTOSCAN_PRINTF("        Can not lock chanel, time out %d\n", timeout);
				
				err_flag = 1;
				goto ERR_HANDLE;
			}          
			else if(0x08 == (intdata & 0x08))
			{
				AUTOSCAN_PRINTF("    HW timeout, time out %d\n", timeout);
				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);
				err_flag = 1;		
				goto ERR_HANDLE;
			}

			if (0x20 == (intdata & 0x20))
			{
				AUTOSCAN_PRINTF("        CR loss lock  \n");
                #if 0
				cr_loss_cnt++;
				if (cr_loss_cnt > 3)
				{
					AUTOSCAN_PRINTF("\t CR loss lock, cr_loss_cnt %d, cr_lock_num %d, lock_monitor %d,\n",\
					cr_loss_cnt, cr_lock_num, lock_monitor);
					err_flag = 1;
					goto ERR_HANDLE;
				} 
                #endif
			}

#ifdef AUTOSCAN_SPEEDUP
            // For autosacn speedup 20170803 paladin
            if (0x10 == (intdata & 0x10))
            {
                AUTOSCAN_PRINTF("        TR loss lock  \n");
                #if 1
                tr_loss_cnt++;
                if ((tr_loss_cnt > 2) && (timeout > tp_scan_param->tr_lock_thr))
                {
                    AUTOSCAN_PRINTF("\t TR loss lock, tr_loss_cnt %d, tr_lock_num %d, lock_monitor %d,\n",\
                    tr_loss_cnt, tr_lock_num, lock_monitor);
                    return ERR_FAILED; 
                } 
                #endif
            }
            else
            {
                tr_loss_cnt = 0;
            }
#endif


#if 0//NIM_OPTR_ACM == ACM_CCM_FLAG    // In acm work mode this bit is valid only for dvb-s try
			if ((intdata & 0x02) && tp_scan_param->change_work_mode_flag) 
#else
			if (intdata & 0x02)
#endif
			{
                AUTOSCAN_PRINTF("Path1 lock TP, cost:%dms \n", osal_get_tick() - fsm_begin_time);

				NIM_PRINTF("    path1:lock chanel \n");
				tempFreq = tp_scan_param->est_freq;
				nim_c3501h_get_freq(dev, &tempFreq);             

				NIM_PRINTF("            Freq is %d\n", tempFreq);

				priv->ul_status.m_cur_freq = tempFreq ;

				nim_c3501h_get_symbol_rate(dev, &Rs);

				NIM_PRINTF("            Rs is %d\n", Rs);
				del_rs = rs_rev - Rs;
				if(del_rs < 0)
				del_rs = -del_rs;

				if((Rs >= 1000)&&(del_rs >= 100))
					rs_rev = Rs;
				Rs = rs_rev;
				nim_c3501h_reg_get_code_rate(dev, &code_rate);
				NIM_PRINTF("            code_rate is %d\n", code_rate);
				nim_c3501h_reg_get_work_mode(dev, &work_mode);
				NIM_PRINTF("            work_mode is %d\n", work_mode);
				nim_c3501h_reg_get_map_type(dev, &map_type);
				NIM_PRINTF("            map_type is %d\n", map_type);
				nim_c3501h_reg_get_modcod(dev, &modcod);
				if(modcod & 0x01)
					NIM_PRINTF("            Pilot on \n");
				else
				{
					NIM_PRINTF("            Pilot off \n");
				}
				modcod = modcod >> 1;
				NIM_PRINTF("            Modcod is %x\n", modcod);

				//final_est_freq = (tempFreq + 512) / 1024;
				final_est_freq = tempFreq;
				final_est_sym_rate = Rs;
#ifdef NIM_S3501_ASCAN_TOOLS
				nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP,0x00, tempFreq, Rs, tempFreq > 1550? 1:0);
#endif
				AUTOSCAN_PRINTF("\tLock path1 freq %d Rs %d with del_Rs %d del_f %d \n",\
				tempFreq, Rs, Rs - tp_scan_param->est_rs, tempFreq - tp_scan_param->est_freq);
				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);

				AUTOSCAN_PRINTF("\t timeout = %d\n", timeout);
				//nim_c3501h_clear_interrupt(dev);
				AUTOSCAN_PRINTF("[%s %d]timeout = %d\n", __FUNCTION__, __LINE__, timeout);
				return SUCCESS;
			}
			//nim_c3501h_clear_interrupt(dev);
		}

ERR_HANDLE:				
		if (err_flag)
		{
			nim_c3501h_get_search_timeout(dev, &search_single_timeout, &search_total_timeout);
			state = nim_c3501h_try_plsn(dev, &try_index);
			if (pl_find_flag)
			{
				if (PLSN_FIND == state)
				{
					PLSN_PRINTF("[%s %d]try_index=%d\n", __FUNCTION__, __LINE__, try_index);
					PLSN_PRINTF("[%s %d]reinit time!\n", __FUNCTION__, __LINE__);
					timeout = 0;//reinit the start time
					err_flag = 0;
					lock_monitor = 0;
					tr_lock_flag = 0;
					tr_lock_num = 0;
					cr_lock_num = 0;
					//cr_loss_cnt = 0;					
					nim_c3501h_clear_interrupt(dev);					
				}
				else if ((PLSN_OVER == state) || (PLSN_RUNNING == state))
				{
					comm_sleep(2);//sleep, so the system can dispatch search task
					if ((PLSN_OVER == state) || (osal_get_tick() - search_start_single > search_single_timeout))
					{
						if(priv->plsn.plsn_same_cnt >= 2)
						{
							PLSN_PRINTF("[%s %d]plsn_same_cnt = %d \n", __FUNCTION__, __LINE__, priv->plsn.plsn_same_cnt);
							PLSN_PRINTF("[%s %d]over,state=%d!, timeout=%d ms\n", __FUNCTION__, __LINE__, state, osal_get_tick() - search_start_single);
							PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
							nim_c3501h_clear_interrupt(dev);

							stop_start = osal_get_tick();
							priv->plsn.search_plsn_force_stop = 1;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
							while (1==priv->plsn.start_search)
							{			
								comm_sleep(2);
								if (osal_get_tick() - stop_start > 6000)
								{
									libc_printf("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
									break;
								}
							}
							PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
							priv->plsn.search_plsn_force_stop = 0;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

							AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
							return ERR_FAILED;
						}
						else if (osal_get_tick() - search_start_total < search_total_timeout)
						{
							PLSN_PRINTF("[%s %d]start next search plsn, state=%d!, search_single_timeout=%d ms, search_total_timeout=%d ms, timeout=%d ms\n", 
							__FUNCTION__, __LINE__, state, search_single_timeout, search_total_timeout, osal_get_tick() - search_start_single);				
						
							stop_start = osal_get_tick();
							priv->plsn.search_plsn_force_stop = 1;	
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
							while (1==priv->plsn.start_search)
							{			
								comm_sleep(2);
								if (osal_get_tick() - stop_start > 6000)
								{
									PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
									break;
								}
							}
							PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
							try_index = 0;
							priv->plsn.search_plsn_force_stop = 0;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

							PLSN_PRINTF("[%s %d]reset demod before next search!\n", __FUNCTION__, __LINE__);
							//reset demod
							nim_c3501h_reset_fsm(dev);	
							priv->plsn.plsn_try = 0;
							nim_c3501h_set_plsn(dev);																			
							comm_sleep(2);							
							//start demod
							nim_c3501h_start_fsm(dev);

							priv->plsn.start_search = 1;//start to search plsn
							priv->plsn.auto_scan_start = 1;
							PLSN_PRINTF("priv->plsn.auto_scan_start=%d\n", priv->plsn.auto_scan_start);
							priv->plsn.search_plsn_stop = 0;//to avoid get wrong state when plsn task not start immediate
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_stop);
							try_index = 0;//reinit the try index
							comm_sleep(5);//sleep, so the system can dispatch search task
							PLSN_PRINTF("[%s %d]reinit try index!\n", __FUNCTION__, __LINE__);
							search_start_single = osal_get_tick();
						}
						else
						{
							PLSN_PRINTF("[%s %d]over,state=%d!, search_single_timeout=%d ms, search_total_timeout=%d ms, timeout=%d ms\n", __FUNCTION__, __LINE__, state, search_single_timeout, search_total_timeout, osal_get_tick() - search_start_single);
							PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
							nim_c3501h_clear_interrupt(dev);

							stop_start = osal_get_tick();
							priv->plsn.search_plsn_force_stop = 1;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
							while (1==priv->plsn.start_search)
							{			
								comm_sleep(2);
								if (osal_get_tick() - stop_start > 6000)
								{
									PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
									break;
								}
							}
							PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
							priv->plsn.search_plsn_force_stop = 0;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

							AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
							return ERR_FAILED;
						}
					}
				}
				else
				{
					PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
					nim_c3501h_clear_interrupt(dev);

					stop_start = osal_get_tick();
					priv->plsn.search_plsn_force_stop = 1;
					PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
					while (1==priv->plsn.start_search)
					{			
						comm_sleep(2);
						if (osal_get_tick() - stop_start > 6000)
						{
							PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
							break;
						}
					}
					PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
					priv->plsn.search_plsn_force_stop = 0;
					PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

					AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
					return ERR_FAILED;
				}
			}
			else
			{
				PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
				nim_c3501h_clear_interrupt(dev);

				stop_start = osal_get_tick();
				priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				while (1==priv->plsn.start_search)
				{			
					comm_sleep(2);
					if (osal_get_tick() - stop_start > 6000)
					{
						PLSN_PRINTF("[%s %d]force stop timeout = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
						break;
					}
				}
				PLSN_PRINTF("[%s %d]stop cost = %d ms! \n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				priv->plsn.search_plsn_force_stop = 0;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

				AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
				return ERR_FAILED;
			}
		}
	}
}


/*****************************************************************************
* INT32 nim_c3501h_channel_search(struct nim_device *dev, UINT32 CRNum );
* Description: c3501h channel blind searching operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_channel_search(struct nim_device *dev, UINT32 cr_num)
{
    struct nim_c3501h_private *dev_priv;
    struct nim_c3501h_tp_scan_para *tp_scan_param;

	AUTOSCAN_PRINTF("[%s %d]Enter\n", __FUNCTION__, __LINE__);
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    dev_priv = (struct nim_c3501h_private *)dev->priv;

    tp_scan_param = (struct nim_c3501h_tp_scan_para *) comm_malloc(sizeof(struct nim_c3501h_tp_scan_para));
    
	if (NULL == tp_scan_param)
	{
		ERR_PRINTF("Alloc tp_scan_param memory error!/n");
		return ERR_NO_MEM;
	}
    
    comm_memset((int *)tp_scan_param, 0, sizeof(struct nim_c3501h_tp_scan_para));

    AUTOSCAN_PRINTF("ENTER %s: TP -> %d. Freq= %d, Rs= %d\n",__FUNCTION__, \
        cr_num, dev_priv->ul_status.m_freq[cr_num], dev_priv->ul_status.m_rs[cr_num]);

    // get tp scan param
	tp_scan_param->est_rs = dev_priv->ul_status.m_rs[cr_num];
	tp_scan_param->est_freq = dev_priv->ul_status.m_freq[cr_num];

    nim_c3501h_cfg_tuner_get_freq_err(dev, tp_scan_param);    
    
    // record currect freq for soft search
    dev_priv->ul_status.m_cur_freq = tp_scan_param->est_freq;
    
    // start soft search
    if (SUCCESS == nim_c3501h_soft_search(dev, tp_scan_param))
    {
    #if 0
        UINT8 i = 0;
        for (i = 0; i < 30; i++)
        {
           AUTOSCAN_PRINTF("\n\n\n\n\n*************Try_again*************\n");
           nim_c3501h_soft_search(dev, tp_scan_param); 
        }
    #endif
    
        comm_free(tp_scan_param);
        return SUCCESS;
    }
    else if(((dev_priv->ul_status.m_freq[cr_num] <= 1846) && (dev_priv->ul_status.m_freq[cr_num] >= 1845)) && \
         ((tp_scan_param->est_rs <= 3000) && (tp_scan_param->est_rs >= 1500))) //Yatai5:12446/V/1537
    {
        re_search_num++;
        AUTOSCAN_PRINTF("*************Try_again*************\n");
        if (SUCCESS == nim_c3501h_soft_search(dev, tp_scan_param))
        {            
            comm_free(tp_scan_param);
            return SUCCESS;
        }
    }
	else if((IS_M3031 == dev_priv->tuner_type) && \
			(GET_TP_ONLY == dev_priv->autoscan_control))
	{
		if (SUCCESS == nim_c3501h_soft_search(dev, tp_scan_param))
        {
        	comm_free(tp_scan_param);
            return SUCCESS;
        }
	}

    comm_free(tp_scan_param);
    return ERR_FAILUE;
}

// Unused now!
INT32 nim_c3501h_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr)  
{
    return SUCCESS;
}

#ifdef FPGA_PLATFORM
/*****************************************************************************
* INT32 nim_c3501h_adc2mem_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
* Description: Capture data by dma, only used in FPGA Platform
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*  Parameter4: UINT8 cap_src
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_adc2mem_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
{	
	UINT8 data, data_1;
	UINT8 user_force_stop_adc_dma=0;
    UINT32 dram_base;
	UINT32 cap_len ;
	struct nim_c3501h_private *priv = NULL;

	if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
      
	priv = (struct nim_c3501h_private *) dev->priv;
    dram_base = (UINT32)cap_buffer;


	//1, Reset ADCDMA & Clear int 
   	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);

   	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3 + 1, &data, 1);
	data &= 0xe0;
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3 + 1, &data, 1);

	// 2, Configure DRAM base address, unit is byte
	//	dram_base &=  0x7fffffe0;
	NIM_PRINTF("Enter function %s with DRAM base address: 0x%08x\r\n",__FUNCTION__,dram_base);
    dram_base &=  0xfffffff;    // For address transform

	data = (UINT8)(dram_base&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1, &data, 1);
	data = (UINT8)((dram_base>>8)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 1, &data, 1);
	data = (UINT8)((dram_base>>16)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 2, &data, 1);
	data = (UINT8)((dram_base>>24)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);
	data &= 0x0f;
	data |= 0x90;
	nim_reg_write(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);


 	// 3, Configure DRAM length, can support 1G byte, but should consider with base address 
 	//and make sure that it is not beyond availbe sdram range.
	if (dram_len > 0x10000000)
	    dram_len = 0x10000000;
	
	NIM_PRINTF("\t Configure DRAM length: 0x%08x.\r\n", dram_len);

	cap_len= (dram_len>>5);	// unit is 32byte


	data = (UINT8)(cap_len&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1, &data, 1);
	data = (UINT8)((cap_len>>8)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 1, &data, 1);
	data = (UINT8)((cap_len>>16)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 2, &data, 1);
	data = (UINT8)((cap_len>>24)&0xff);
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 3, &data, 1);

	nim_reg_read(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);
	data &= 0x0f;
	data |= 0xa0;
	nim_reg_write(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);


	// 4, Configure capture data target size:64kByte
	//Unit 32K Byte, can capture max 2GB data.
	cap_len = dram_len >> 15 ;
	data = (UINT8)(cap_len&0xff) ;
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1, &data, 1);
	data = (UINT8)((cap_len>>8)&0xff) ; 
	nim_reg_write(dev, C3501H_R60_CAP2DRAM_CFG_1 + 1, &data, 1);	
	
	nim_reg_read(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);
	data &= 0x0f;
	data |= 0xb0;
	data |= 0x07; //last_idx[26:24]=3'7
	nim_reg_write(dev, C3501H_R68_CAP2DRAM_CFG_2 + 3, &data, 1);


	// 5, Configure CAP2DRAM_SRC_SEL & LA_CAP_EN_BYPASS
	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3 + 2, &data, 1);
	data &= 0xfc;
	data |= 0x04;
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3 + 2, &data, 1);

	// 6, Configure source
	nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
	data &= 0x0f;
	data |= ((cap_src & 0x0f)<<4);
	NIM_PRINTF("capture data source value reg: 0x%x\r\n",data);
	nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
	
	// 6, Reset ADCDMA.	  
   	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	
	// 7, Start ADC DMA
	NIM_PRINTF("\t Start ADC DMA.\r\n");
   	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	data |= 0x01;	
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);

	
	if(14 == cap_src)	// PLSN Cap
	{
		nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
		data |= 0x01;
		nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
		// wait cap done?
	}

	// 8, Wait ADC DMA finish.
	NIM_PRINTF("\t Wait ADC DMA finish.\r\n");
	while(1)
	{
		/*
		if (user_force_stop_adc_dma)
		{
			user_force_stop_adc_dma = 0;
   			nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
			data |= 0x80;	
			nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
			NIM_PRINTF("ADC_DMA force stopped by user.\r\n");
			break;
		}
		*/

		nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3 + 1, &data, 1);
		//nim_reg_read(dev,0x04, &data_1, 1);
        
		NIM_PRINTF("\t Waiting: ... Status 0x%x\r\n", data);

		if (data&0x02)
		{
			NIM_PRINTF("DMA overflowed.\r\n");
			//break;
		}

		if (data&0x04)
		{
			NIM_PRINTF("WR_FIFO overflowed.\r\n");
			comm_sleep(100);
			break;
		}

		if (data&0x01)
		{
			NIM_PRINTF("ADC_DMA finished.\r\n");
			break;
		}

		comm_sleep(50);
	}

	NIM_PRINTF("%s Exit.\r\n",__FUNCTION__);

	// Reset ADC2DMA
   	nim_reg_read(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	data |= 0x80;	
	nim_reg_write(dev, C3501H_R6C_CAP2DRAM_CFG_3, &data, 1);
	
	return SUCCESS;
}
#endif


/*****************************************************************************
* INT32 nim_c3501h_ts_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
* Description: Capture data by ts
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*  Parameter4: UINT8 cap_src
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_ts_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
{
	UINT8 data = 0;
	UINT32 timeout = 0;
	UINT32 block_size = 0;
	UINT8 cap_out_mode = 1;	// 1: TS output, 0: REG output
	UINT8 cap_pid_mode = 1; // 1: every packet insert 4 stuff data(include 8'h47)； 0:just first packet insert 10 stuff data(include 8'h47)
	UINT8 cap_pid_cfg = 0; // 1: PID config by sw, 0: PID config by hw default
	//UINT8 data_idx = 0;
	//UINT8 data_len = 0;
	UINT8 reg_tso_bak = 0;

	struct nim_c3501h_private *priv = NULL;

	NIM_PRINTF("Enter function %s with block size: 0x%x, block num: 0x%x, cap src: 0x%x\r\n",__FUNCTION__, (dram_len >> 16), (dram_len & 0xff), cap_src);

	// 0, close null packet
	nim_reg_read(dev, C3501H_R18_TSO_CFG, &data, 1);
	reg_tso_bak = data;
	data &= 0x7f;
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);

	// 1, select source
	nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
	data &= 0x0f;
	data |= ((cap_src & 0x0f) << 4);
	nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);

	nim_c3501h_freeze_fsm(dev, 1, 9);	// freeze at cr
	nim_c3501h_preset_fsm(dev, 1, 1);	// jump to agc1

	nim_c3501h_reset_fsm(dev);
	nim_c3501h_start_fsm(dev);
	comm_sleep(1500);


	// 2, config cap bolck size and number
	block_size = ((dram_len & 0xffff0000) >> 16);

	if (512 == block_size)
		data = 0;
	else if (1024 == block_size)
		data = 1;
	else if (2048 == block_size)
		data = 2;
	else if (4096 == block_size)
		data = 3;

	data += ((dram_len & 0x3f) << 2);


	nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG, &data, 1);

	// 3, config output 
	cap_out_mode = 1;
	// 4, config ts mode that is use PID
	cap_pid_mode = 0;
	cap_pid_cfg = 0;

	nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
	data &= 0xf2;
	//data &= 0xf7;
	data |= (cap_out_mode + (cap_pid_mode << 2) + (cap_pid_cfg << 3));
	//data |= (cap_out_mode  + (cap_pid_cfg << 3));
	//data |= (cap_pid_cfg << 3);
	nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);

	// 5, config PID
	if (1 == cap_pid_cfg)
	{
		// CAP_TS_PID_1
		data = 0x47;
		nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 2, &data, 1);
		nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 3, &data, 1);
		data &= 0xe0;
		data |= 0x10;
		nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 3, &data, 1);

		//...
	}

	// 5.4 end plsn capture 
	nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
	data |= 0x02;
	nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);

    // 5.5 reset
	nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
	data |= 0x40;
	nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);

	// 6, Start
	nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
	data |= 0x80;
	nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);

	// Wait finished
    comm_delay(1000);
    memset(priv->ul_status.adc_data_malloc_addr,0,0x20000 * 2);
    dmx_pause((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    nim_c3501h_autoscan_signal_input(dev,NIM_SIGNAL_INPUT_OPEN);


	// plsn一次只能抓一帧，所以这里需要一直抓，直到TS抓完
	if (14==(cap_src & 0x0f))
	{
		nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);
		//data &= 0xf0;
		data &= 0xfc;   //set reg from vm when select cr_out or pl_out
		//data |= 0x04;	// 1:cr 0:pl
		data |= 0x01;	// start sclr
		//data |= 0x02;	// end sclr
		nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 3, &data, 1);

		do
		{	
			comm_sleep(1);
			timeout++;
			if(0 == (timeout%100))
			{
				NIM_PRINTF("[PLSN] Wait TS Cap finish, timeout = %d\r\n", timeout);
			}
			nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
		}while(0x80 & data);

		NIM_PRINTF("[PLSN]TS Cap success  %dms\r\n",timeout);
	}
	else
	{
		while (1)
		{
			comm_sleep(1);
			timeout++;
			if(0 == (timeout%100))
			{
				NIM_PRINTF("Wait TS Cap finish, timeout = %d\r\n", timeout);
			}
			nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
			if (0 == (0x80 & data))
			{
				NIM_PRINTF("TS Cap success  %d\r\n",timeout);
				break;
			}

			if (timeout > 10000)
			{
				NIM_PRINTF("TS Cap timeout\r\n");
				break;
			}
		}	
	}

    nim_c3501h_autoscan_signal_input(dev,NIM_SIGNAL_INPUT_CLOSE);
    dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
    
    // Recover tso config
	nim_reg_write(dev, C3501H_R18_TSO_CFG, &reg_tso_bak, 1);

	return SUCCESS;
}


 /*****************************************************************************
 * INT32 nim_c3501h_autoscan_signal_input(struct nim_device *dev, UINT8 s_case)
 * Description: Open or close demux for autoscan rxadc data capture by ts
 *
 * Arguments:
 *  Parameter1: struct nim_device *dev
 *  Parameter2: UINT8 s_case
 *
 * Return Value: INT32
 *****************************************************************************/
 INT32 nim_c3501h_autoscan_signal_input(struct nim_device *dev, UINT8 s_case)
 {
     struct dmx_device *l_nim_dmx_dev;
     struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
     l_nim_dmx_dev = (struct dmx_device *) dev_get_by_name("DMX_S3601_0");
     switch (s_case)
     {
     case NIM_SIGNAL_INPUT_OPEN:
         osal_cache_invalidate((void *)priv->ul_status.adc_data, 0x20000 * 2);
         dmx_io_control(l_nim_dmx_dev, IO_SET_BYPASS_MODE, (UINT32) (priv->ul_status.adc_data));//Set bypass mode
         break;
     case NIM_SIGNAL_INPUT_CLOSE:
         osal_cache_flush((void *)priv->ul_status.adc_data, 0x20000 * 2);
         dmx_io_control(l_nim_dmx_dev, IO_CLS_BYPASS_MODE, 0);//Clear bypass mode.
         break;
     }
 
     return SUCCESS;
 }


/*****************************************************************************
* INT32 nim_c3501h_cap_iq_enerage(struct nim_device *dev)   
* Description: Read rxadc data from register by i2c
*
* Arguments:
*  Parameter1: struct nim_device *dev
* 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cap_iq_enerage(struct nim_device *dev)                                                     
{                                                                                                                 
    //UINT8 data = 0;                                                                                               
    //UINT8 buffer[8];                                                                                              
    INT32 i = 0, j = 0, k = 0;                                                                                    
    UINT8 fft_data[3];                                                                                            
    INT32 fft_i, fft_q;                                                                                           
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;                                      
                                                                                                                  
    for (i = 0; i < 1024; i++)                                                                                    
    {     
    #if 0
        data = (UINT8) ((i) & 0xff);                                                                              
        buffer[0] = data;                                                                                         
        data = (UINT8) (i >> 8);                                                                                  
        data &= 0x3;                                                                                              
        buffer[1] = data;                                                                                         
        nim_reg_write(dev, C3501H_R20_CAPTURE_DISEQC_RPT, buffer, 2);  
    #endif
        nim_reg_read(dev, C3501H_R20_CAPTURE_DISEQC_RPT, fft_data, 2);                                                       
                                                                                                                  
        fft_i = fft_data[0];                                                                                      
        fft_q = fft_data[1];                                                                                      
                                                                                                                  
        if (fft_i & 0x80)                                                                                         
            fft_i |= 0xffffff00;                                                                                  
        if (fft_q & 0x80)                                                                                         
            fft_q |= 0xffffff00;                                                                                  
                                                                                                                  
        k = 0;                                                                                                    
        for (j = 0; j < 10; j++)                                                                                  
        {                                                                                                         
            k <<= 1;k += ((i >> j) & 0x1);                                                                        
        }                                                                                                         
                                                                                                                  
        fft_i = fft_i << (FFT_BITWIDTH - 8);                                                                      
        fft_q = fft_q << (FFT_BITWIDTH - 8);                                                                      
                                                                                                                  
        priv->ul_status.fft_i_1024[k] = fft_i;                                                                    
        priv->ul_status.fft_q_1024[k] = fft_q;                                                                    
    }                                                                                                             
	                                                                                                              
	if(0)                                                                                
	{                                                                                                             
		for(i = 0; i< 1024; i++)                                                                                  
		{                                                                                                         
			AUTOSCAN_PRINTF("ADCdata%d:[%d,%d]\n",i,priv->ul_status.fft_i_1024[i] ,priv->ul_status.fft_q_1024[i]);
		}                                                                                                         
	}                                                                                                             
                                                                                                                  
    return SUCCESS;                                                                                               
}                                                                                                                 



 /*****************************************************************************
* void nim_c3501h_cap_fft_result_read(struct nim_device *dev)
* Get RX-ADC data for calculate the spectrum energe
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
*
* Return Value: INT32
*****************************************************************************/
void nim_c3501h_cap_fft_result_read(struct nim_device *dev)
{
	UINT8 data = 0;
	INT32 m = 0;

	//NIM_PRINTF("Enter function %s \r\n",__FUNCTION__);

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	nim_c3501h_start_fsm(dev);
	nim_c3501h_clear_interrupt(dev);

	// tso dummy off
	//nim_c3501h_tso_dummy_off(dev);
	
	//data = 0x11;
	//nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);	// c3501c config

	//nim_c3501h_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT, NIM_FRZ_AGC1_OPEN);
    
	//Enable AGC1 auto freeze function and let AGC can not lock, try to modify tuner's gain    
    nim_reg_read(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
    data &= 0xf0;
    data |= 0x05;
    nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);


	//CRBE: capture out mode and capture block number setting
    //Capture data length setting 1: capture 1024 samples
#ifdef NIM_TS_PORT_CAP
    data = 0xf1;
#else
    data = 0x05;
#endif
    nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG, &data, 1);

    // Reset cap module
#ifdef NIM_TS_PORT_CAP
    data = 0x41;
#else
    data = 0x40;
#endif
    nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);

	// Start cap module
#ifdef NIM_TS_PORT_CAP
        memset(priv->ul_status.adc_data_malloc_addr,0,0x20000 * 2);
        dmx_pause((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
        nim_c3501h_autoscan_signal_input(dev,NIM_SIGNAL_INPUT_OPEN);

        data = 0x81;
#else
        data = 0x80;
#endif
    nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);	

#ifdef NIM_TS_PORT_CAP
    for (m = 0; m < 1000; m++)
    {
        comm_delay(1000);

        nim_reg_read(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
        if (0 == (0x80 & data))
        {
            NIM_PRINTF("TS Cap success nim_c3501h_cap_fft_result_read loop:%d\r\n", m);
            break;
        }
    }
#else
    comm_delay(1000);

    nim_c3501h_cap_iq_enerage(dev);
    //R2FFT
    R2FFT(priv->ul_status.fft_i_1024, priv->ul_status.fft_q_1024);
    NIM_PRINTF("By read reg adc data transfer finish\n");
#endif

    nim_reg_read(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
    data &= 0xf0;
    nim_reg_write(dev, C3501H_R2C_ANA_AGC1_CFG + 3, &data, 1);
    
#ifdef NIM_TS_PORT_CAP
    nim_c3501h_autoscan_signal_input(dev,NIM_SIGNAL_INPUT_CLOSE);
    dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));

 #endif  

     // Reset cap module
#ifdef NIM_TS_PORT_CAP
     data = 0x41;
#else
     data = 0x40;
#endif
     nim_reg_write(dev, C3501H_R1C_CAPTURE_CFG + 1, &data, 1);
 
}


 /*****************************************************************************
* INT32 nim_c3501h_cap_calculate_energy(struct nim_device *dev)
* Description: Get RX-ADC data and calculate the spectrum energe
*
* Arguments:
*  Parameter1: struct nim_device *dev
* 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cap_calculate_energy(struct nim_device *dev)
{
    INT32 i, j, k, m,n;
    INT32 energy_real, energy_imag;
    INT32 energy;
    INT32 energy_tmp;
    INT32 fft_i, fft_q;
    UINT8 data;
    UINT8 *pfftdata, *pData = &data;
    UINT32 OldIDPos, Find11Packet, PacketNum;
    UINT32 NO_c3501hID;
    UINT32 overflow_cnt = 0;
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	//NIM_PRINTF("line=%d, Enter nim_c3501h_calculate_energy\n", __LINE__);
    comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));

#ifdef NIM_TS_PORT_CAP
    nim_c3501h_cap_fft_result_read(dev);
    //nim_c3501h_ts_cap_data_entity(dev, NULL, 0x400003c, 0);
    pfftdata = (UINT8 *) (priv->ul_status.adc_data);
    comm_delay(100); 
#endif

    OldIDPos = 0;
    PacketNum = 0;
    
#ifdef NIM_TS_PORT_CAP
    for (k = 0; k < 20; k++)
    {
        if (priv->ul_status.c3501h_autoscan_stop_flag)
        {
            return USER_STOP;
        }

        for(n=0;n<1024;n++)
        {
            fft_energy_1024_tmp[n] = 0;
        }
        Find11Packet = 0;
        while ((Find11Packet == 0) && (PacketNum < 662))
        {
            NO_c3501hID = 1;
            for (i = 0; i < 10; i++)
            {
                data = *(pfftdata + i);
                if (c3501h_id[i] != data)
                {
                    NO_c3501hID = 0;
                }
            }

            if (NO_c3501hID == 1)
            {
                if ((PacketNum - OldIDPos) == 11)
                {
                    pData = pfftdata - 2068;
                    Find11Packet = 1;
                }
                OldIDPos = PacketNum;
            }
            pfftdata = pfftdata + 188;
            PacketNum++;
        }
		
        if (Find11Packet == 0)
        {
            NIM_PRINTF("cannot find 20 complete c3501h packet\n");

            for(i=0;i<1024;i++)
                fft_energy_1024[i] = 0;
            return SUCCESS;
        }
		
        comm_memcpy(priv->ul_status.adc_data, (pData + 10), 178);
        for (i = 0; i < 10; i++)
        {
            comm_memcpy(priv->ul_status.adc_data + 178 + i * 187, pData + (i + 1) * 188 + 1, 187);
        }
        overflow_cnt = 0;
        for (i = 0; i < 1024; i++)
        {
            fft_i = priv->ul_status.adc_data[2 * i];
            fft_q = priv->ul_status.adc_data[2 * i + 1];

            if ((fft_i==127) ||(fft_i==128))
                overflow_cnt++;
            if ((fft_q==127) || (fft_q==128))
                overflow_cnt++;

            if (fft_i & 0x80)
                fft_i |= 0xffffff00;
            if (fft_q & 0x80)
                fft_q |= 0xffffff00;

            m = 0;
			for (j = 0; j < 10; j++)
            {
                  m <<= 1;m += ((i >> j) & 0x1);
            }
            fft_i = fft_i << (FFT_BITWIDTH - 8);
            fft_q = fft_q << (FFT_BITWIDTH - 8);
            priv->ul_status.fft_i_1024[m] = fft_i;
            priv->ul_status.fft_q_1024[m] = fft_q;
        }
		
		if(0x10 == (priv->debug_flag&0x10))
		{
			for(i = 0; i< 1024; i++)
			{
				AUTOSCAN_PRINTF("ADCdata%d:[%d,%d]\n",i,priv->ul_status.fft_i_1024[i],priv->ul_status.fft_q_1024[i]);
			}
		}
		
        //R2FFT
        R2FFT(priv->ul_status.fft_i_1024, priv->ul_status.fft_q_1024);
#else
    for (k = 0; k < 20; k++)
    //for (k = 0; k < 10; k++)
    {
        if (priv->ul_status.c3501h_autoscan_stop_flag)
        {
            return USER_STOP;
        }
        //NIM_PRINTF("fft_start\n");
        nim_c3501h_cap_fft_result_read(dev);
        //NIM_PRINTF("fft_end\n");
#endif
        for (i = 0; i < 1024; i++)
        {
            fft_i = priv->ul_status.fft_i_1024[i];
            fft_q = priv->ul_status.fft_q_1024[i];

            energy_real = fft_i * fft_i;
            energy_imag = fft_q * fft_q;
            energy = energy_real + energy_imag;
            energy >>= 3;
            j = (i + 511) & 1023;
            energy_tmp =  energy;

            if ((energy_tmp >> 20) & 1)
            {
                fft_energy_1024_tmp[j] = 1048575;
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
	//NIM_PRINTF("line=%d, Leave nim_c3501h_cap_calculate_energy\n", __LINE__);
    return SUCCESS;
    
}



/*****************************************************************************
* INT32 nim_c3501h_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 *cap_buffer)
*  start to capture RX-ADC data and calculate the spectrum energy
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 startFreq
*		input IF frequence that send to tuner
*  Parameter3: UINT32 sym
*		symbol rate is set to zero in autoscan mode
*  Parameter3:UINT32 *cap_buffer: unused
*			
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, INT32 *cap_buffer)
{
    INT32	 channel_freq_err = 0;
	struct nim_c3501h_private *dev_priv;
	dev_priv = (struct nim_c3501h_private *)dev->priv;

	NIM_PRINTF("Enter nim_c3501h_cap_start : Tuner_IF=%d\n",startFreq);

    nim_c3501h_tuner_control(dev, startFreq, sym, &channel_freq_err);

	if (GET_SPECTRUM_ONLY == dev_priv->autoscan_control) //get spectrum or get frequency/sysbolrate
	{
		comm_delay(20000);
	}
	else												//autoscan or get tp
	{
		if(dev_priv->tuner_type == IS_M3031)
		{
			comm_delay(40000);//wait 20ms for agc1 stable	  
		}
		else
		{
			comm_delay(10000);	//wait 5ms for agc1 stable	   
		}	
	}

	return nim_c3501h_cap_calculate_energy(dev);
}


/*****************************************************************************
* INT32 nim_c3501h_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
*  Entry of data capture 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *cap_buffer
*  Parameter3: UINT32 dram_len
*  Parameter3:UINT8 cap_src
*			
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src)
{
	if (cap_src & 0x80)
	{
		nim_c3501h_ts_cap_data_entity(dev, cap_buffer, dram_len, cap_src);                      
	}
	else
	{
	#if FPGA_PLATFORAM
		nim_c3501h_adc2mem_entity(dev, cap_buffer, dram_len, cap_src);   
    #endif
	}
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_freq_offset(struct nim_device *dev, INT32 delfreq)
* Description: Set freq offset to demod for it lock faster. freq offset = real tp freq - real tuner freq
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: Data capture delfreq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_freq_offset(struct nim_device *dev, INT32 delfreq)
{
	UINT8 data[2];
	UINT32 temp, delfreq_abs;
    UINT32 sample_rate;

	if(delfreq<0)
		delfreq_abs = 0 - delfreq;
    else
	    delfreq_abs = delfreq;

	nim_c3501h_get_dsp_clk(dev, &sample_rate);
	temp = nim_c3501h_multu64div(delfreq_abs, 65536, sample_rate); //fix{1,-1,16}, norm to sample rate

    if(temp>0xffff)
	{    
		NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\r\n", temp);
		temp = 0xffff;
    }

	if(delfreq<0)
		temp = (0-temp)&0xffff;
	else
		temp = temp&0xffff ;

	data[0] = (UINT8) (temp & 0xFF);
    data[1] = (UINT8) ((temp >>8)&0xff);
	nim_reg_write(dev, C3501H_R04_TOP_FSM_CFG_2+2, data, 2);

	return SUCCESS;
}                  


/*****************************************************************************
* INT32 nim_c3501h_start_plsn_search(struct nim_device *dev, INT32 delfreq)
* Enable plsn search
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_start_plsn_search(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_RF4_PLSN_CFG, &data, 1);
	data |= 0x81; 
	nim_reg_write(dev, C3501H_RF4_PLSN_CFG, &data, 1);

	return SUCCESS;
}                  


/*****************************************************************************
* INT32 nim_c3501h_close_plsn_search(struct nim_device *dev, INT32 delfreq)
* Disable plsn search
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_close_plsn_search(struct nim_device *dev)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_RF4_PLSN_CFG, &data, 1);
	data &= 0xee; 
	nim_reg_write(dev, C3501H_RF4_PLSN_CFG, &data, 1);

	return SUCCESS;
}   

/*****************************************************************************
* INT32 nim_c3501h_sel_plsn_search_algo(struct nim_device *dev, INT32 delfreq)
* Select plsn search method
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: method, 0: method 1, 1: method 2, 2: method 3
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_sel_plsn_search_algo(struct nim_device *dev, UINT8 method)
{
	UINT8 data = 0;

	nim_reg_read(dev, C3501H_RF4_PLSN_CFG, &data, 1);
	data &= 0xc1; 

	if (0 == method)
		data |= 0x10; //PD_PL_BLOCK1
	else if (1 == method)
		data |= 0x18; //PD_CR_BLOCK1
	else if (2 == method)
		data |= 0x20; //PD_PL_BLOCK2
	else if (3 == method)
		data |= 0x28; //PD_CR_BLOCK2
	else if (4 == method)
		data |= 0x1a; //BBH_CR_BLOCK1
	else if (5 == method)
		data |= 0x1c; //NP_CR_BLOCK1
	else
		data |= 0x10; //PD_PL_BLOCK1

	nim_reg_write(dev, C3501H_RF4_PLSN_CFG, &data, 1);

	return SUCCESS;
}      


/*****************************************************************************
* INT32 nim_c3501h_rw_plsn_search_sub_reg(struct nim_device *dev, UINT8 rw, UINT8 cmd, UINT8 *value)
* read/write plsn searh sub reg
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 rw: 0 = write, 1 = read
*  Parameter3: UINT8 cmd: enum PLSN_SUB_REG
*  Parameter4: UINT8 *value: write/read value
* 
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_rw_plsn_search_sub_reg(struct nim_device *dev, UINT8 rw, UINT8 cmd, UINT8 *value)
{
	UINT8 data = 0;

	// 1, Write cmd to sub reg
	nim_reg_read(dev, C3501H_RF4_PLSN_CFG + 2, &data, 1);
	data &= 0x80; 
	data |= (cmd & 0x1f);
	
	// 2, Set read/write pulse
	rw &= 0x01;
	data |= (1 << (5 + rw));


	// 3, Write/Read value
	if (0 == rw)
	{
		nim_reg_write(dev, C3501H_RF4_PLSN_CFG + 1, value, 1);
		nim_reg_write(dev, C3501H_RF4_PLSN_CFG + 2, &data, 1);
	}
	else if(1 == rw)
	{
		nim_reg_write(dev, C3501H_RF4_PLSN_CFG + 2, &data, 1);
		nim_reg_read(dev, C3501H_RF4_PLSN_CFG + 3, value, 1);
	}

	return SUCCESS;
} 


/*****************************************************************************
* INT32 nim_c3501h_get_plsn_search_ret(struct nim_device *dev)
* Get plsn search result
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_get_plsn_search_ret(struct nim_device *dev)
{
	UINT8 data = 0;
	UINT8 plsn_read[3] = {0, 0, 0};
	UINT8 i = 0;

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	nim_reg_read(dev, C3501H_RFC_PLSN_RPT + 2, &data, 1);
	//PLSN_PRINTF("		C3501H_RFC_PLSN_RPT = 0x%02x \n", data);

	osal_mutex_lock(priv->plsn.plsn_mutex, OSAL_WAIT_FOREVER_TIME);//robin.gan add
	priv->plsn.plsn_hw_find = ((data >> 2) & 0x01);
	priv->plsn.plsn_hw_end = ((data >> 3) & 0x01);

	if ((priv->plsn.plsn_hw_find) && (priv->plsn.plsn_hw_end))
	{
		priv->plsn.plsn_num = ((data >> 4) & 0x0f);

		nim_reg_read(dev, C3501H_RFC_PLSN_RPT + 3, &data, 1);

		for (i = 0; i < priv->plsn.plsn_num; i++)
		{
			data &= 0xe0;
			data |= (0x10 + i);
			nim_reg_write(dev, C3501H_RFC_PLSN_RPT + 3, &data, 1);
			
			nim_reg_read(dev, C3501H_RFC_PLSN_RPT, plsn_read, 3);
			priv->plsn.plsn_val[i] = (plsn_read[0] + (plsn_read[1] << 8) + ((plsn_read[2] & 0x03) << 16));
		}
	}
	osal_mutex_unlock(priv->plsn.plsn_mutex);
	
	return SUCCESS;
}   


/*****************************************************************************
* INT32 nim_c3501h_plsn_gold_to_root(INT32 plsn_gold)
* Description: Transform plsn format from gold to root
*
* Arguments:
*  Parameter1: INT32 plsn_gold
*  
*
* Return Value: INT32 plsn root
*****************************************************************************/
INT32 nim_c3501h_plsn_gold_to_root(INT32 plsn_gold)
{
	INT32 plsn_root = 0;
	INT32 i    = 0;
	UINT8 bit_0 = 0;
	UINT8 bit_7 = 0;
	UINT8 bit_18 = 0;
	UINT8 pbuff_x[3] = {0, 0, 0};

	pbuff_x[2] = 64;//0000 0000 0000 0000 01xx xxxx

	while(i < plsn_gold)
	{
		bit_0 = (pbuff_x[2] >> 6) & 0x01;
		bit_7 = (pbuff_x[1] >> 5) & 0x01;
		bit_18 = 0;
		bit_18 = bit_0^bit_7;

		// update
		pbuff_x[2] = (pbuff_x[1] & 0x01)*128 + (pbuff_x[2] >> 1);
		pbuff_x[1] = (pbuff_x[0] & 0x01)*128 + (pbuff_x[1] >> 1);
		pbuff_x[0] = bit_18*128 + (pbuff_x[0] >> 1);

		i ++;
	}

	plsn_root = pbuff_x[0] * 1024 + pbuff_x[1]*4 + ((pbuff_x[2] >> 6) & 0x03);
	
	NIM_PRINTF("plsn_gold = %d , translate to plsn_root is %d\r\n",plsn_gold,plsn_root);

	return plsn_root;
}


/*****************************************************************************
* INT32 nim_c3501h_plsn_root_to_gold(INT32 plsn_root)
* Description: Transform plsn format from root to gold
*
* Arguments:
*  Parameter1: INT32 plsn_root
*  
*
* Return Value: INT32 plsn gold
*****************************************************************************/
INT32 nim_c3501h_plsn_root_to_gold(INT32 plsn_root)
{
	INT32 gold_try = 0;
	INT32 root_try = 0;
	UINT8 bit_0 = 0;
	UINT8 bit_7 = 0;
	UINT8 bit_18 = 0;
	UINT8 pbuff_x[3] = {0,0,0};
	
	pbuff_x[2] = 64;//0000 0000 0000 0000 01xx xxxx

	while(gold_try < 262143) // 2^18 -1
	{
		root_try = 0;
		root_try = pbuff_x[0] * 1024 + pbuff_x[1]*4 + ((pbuff_x[2] >> 6) & 0x03);

		if (plsn_root == root_try)
		{
			NIM_PRINTF("find gold num %d \r\n", gold_try);
			break;
		}

		bit_0 = (pbuff_x[2] >> 6) & 0x01;
		bit_7 = (pbuff_x[1] >> 5) & 0x01;
		bit_18 = 0;
		bit_18 = bit_0^bit_7;

		// update
		pbuff_x[2] = (pbuff_x[1] & 0x01)*128 + (pbuff_x[2] >> 1);
		pbuff_x[1] = (pbuff_x[0] & 0x01)*128 + (pbuff_x[1] >> 1);
		pbuff_x[0] = bit_18*128 + (pbuff_x[0] >> 1);

		gold_try ++;

	}

	if (gold_try >= 262143) 
	{
		gold_try = -1;
		NIM_PRINTF("the gold num is invalid \r\n");
	}
	NIM_PRINTF("plsn_root is %d , translate to plsn_gold = %d \r\n",plsn_root,gold_try);
	return gold_try;
}


/*****************************************************************************               
*  INT32 nim_c3501h_set_plsn(struct nim_device *dev)            
* Set pls number to hw, only use root format,target plsn was set in dev struct            
*                                                                                            
* Arguments:                                                                                 
*  Parameter1: struct nim_device *dev                                                                    
*                                                                                            
* Return Value: INT32                                                                        
*****************************************************************************/               
INT32 nim_c3501h_set_plsn(struct nim_device *dev)                                             
{                                                                                            
	UINT8 data = 0;                                                                          
    //INT32 ret = ERR_FAILED;                                                                  
	INT32 plsn_root;                                                                         
	INT32 plsn_gold;                                                                         
	struct nim_c3501h_private *priv = NULL;                                                   
	priv = (struct nim_c3501h_private *)dev->priv;                                            
                                                                                             
	//NIM_PRINTF("Enter %s \r\n",__FUNCTION__);                                                  
    // Pointer check                                                                         
    if (NULL == dev->priv)                                                                   
    {                                                                                        
        ERR_PRINTF("Exit with Pointer check error in %s \r\n", __FUNCTION__);                  
        return ERR_NO_MEM;                                                                   
    }                                                                                        
                                                                                             
	//get plsn_gold                                                                          
	if(priv->plsn.plsn_try == 1)                                                             
		plsn_gold = priv->plsn.plsn_try_val;                                                 
	else                                                                                     
		plsn_gold = priv->plsn.plsn_now;                                                     
                                                                                             
	plsn_root = nim_c3501h_plsn_gold_to_root(plsn_gold);                                      
	                                                                                         
	NIM_PRINTF("In [%s %d], plsn_gold = %d, plsn_root = %d\r\n", __FUNCTION__,__LINE__, plsn_gold,plsn_root);
                                                                                             
	//set root format plsn                                                                       
	data = ((plsn_root >> 4) & 0xff); 
	nim_reg_write(dev, C3501H_R8C_PL_CFG_4 + 1, &data, 1);  

	nim_reg_read(dev, C3501H_R8C_PL_CFG_4 + 2, &data, 1);
	data &= 0xc0;
	data |= ((plsn_root >> 12) & 0x3f); 
	nim_reg_write(dev, C3501H_R8C_PL_CFG_4 + 2, &data, 1); 

	nim_reg_read(dev, C3501H_R8C_PL_CFG_4, &data, 1);
	data &= 0x0f;
	data |= 0x01;
	data |= ((plsn_root & 0x0f) << 4); 
	nim_reg_write(dev, C3501H_R8C_PL_CFG_4, &data, 1); 

    return SUCCESS;                                                                          
}                                                                                            
    
/*****************************************************************************               
*  INT32 nim_c3501h_clear_plsn(struct nim_device *dev)            
* Clear plsn in hw               
*                                                                                            
* Arguments:                                                                                 
*  Parameter1: struct nim_device *dev                                                                    
*                                                                                            
* Return Value: INT32                                                                        
*****************************************************************************/               
INT32 nim_c3501h_clear_plsn(struct nim_device *dev)                                             
{  
	UINT8 data = 0;

	nim_reg_write(dev, C3501H_R8C_PL_CFG_4 + 1, &data, 1);  

	nim_reg_read(dev, C3501H_R8C_PL_CFG_4 + 2, &data, 1);
	data &= 0xc0;
	nim_reg_write(dev, C3501H_R8C_PL_CFG_4 + 2, &data, 1); 

	nim_reg_read(dev, C3501H_R8C_PL_CFG_4, &data, 1);
	//data &= 0x0e;
    data &= 0x0f;
    data |= 0x10;
	nim_reg_write(dev, C3501H_R8C_PL_CFG_4, &data, 1);  

	return SUCCESS;  
}


/*****************************************************************************               
*  INT32 nim_c3501h_get_plsn(struct nim_device *dev)            
* Get plsn from hw               
*                                                                                            
* Arguments:                                                                                 
*  Parameter1: struct nim_device *dev                                                                    
*                                                                                            
* Return Value: INT32                                                                        
*****************************************************************************/ 
INT32 nim_c3501h_get_plsn(struct nim_device *dev)                                             
{                                                                                            
	UINT8 data = 0;                                                                          
    //INT32 ret = ERR_FAILED;                                                                  
	INT32 plsn_root;                                                                         
	INT32 plsn_gold;                                                                                                                   
                                                                                             
	//NIM_PRINTF("Enter %s \r\n",__FUNCTION__);                                                  

	nim_reg_read(dev, C3501H_R8C_PL_CFG_4, &data, 1);
	plsn_root = ((data & 0xf0) >> 4);
	nim_reg_read(dev, C3501H_R8C_PL_CFG_4 + 1, &data, 1);
	plsn_root += data << 4;
	nim_reg_read(dev, C3501H_R8C_PL_CFG_4 + 2, &data, 1);
	plsn_root += ((data & 0x3f) << 12);
                                                                                  
	plsn_gold = nim_c3501h_plsn_root_to_gold(plsn_root);                                      
 	                                                                                         
	NIM_PRINTF("In %s, plsn_gold = %d, plsn_root = %d\r\n", __FUNCTION__, plsn_gold,plsn_root);
                                                                                             
    return plsn_gold;                                                                        
}     


/*****************************************************************************
* INT32 nim_c3501h_try_plsn(struct nim_device *dev, UINT8 *index)
* Description: Set est plsn in hardware try to lock TP
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *index
*
* Return Value: INT32
*****************************************************************************/
plsn_state nim_c3501h_try_plsn(struct nim_device *dev, UINT8 *index)
{
	struct nim_c3501h_private *priv = NULL;
	
	if (NULL == dev)
	{
		ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return PLSN_UNSUPPORT;
	}
	priv = (struct nim_c3501h_private *)dev->priv;
	if(NULL == priv)
    {
        ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
        return PLSN_UNSUPPORT;
    }

	//NIM_PRINTF("Run in [%s %d]\r\n", __FUNCTION__, __LINE__);
	
	osal_mutex_lock(priv->plsn.plsn_mutex, OSAL_WAIT_FOREVER_TIME);//robin.gan add
	if (priv->plsn.super_scan)//support super scan
	{
			if ((0 == priv->plsn.start_search)
				&& (1 == priv->plsn.search_plsn_stop) //Completely withdrawal from the search process
				&& ((priv->plsn.plsn_find) || ((priv->plsn.plsn_hw_find) && (priv->plsn.plsn_hw_end))))//find the right value
			{
				PLSN_PRINTF("[%s %d]plsn_find = %d, plsn_hw_find = %d\r\n", __FUNCTION__, __LINE__, priv->plsn.plsn_find, priv->plsn.plsn_hw_find);

				if (*index == priv->plsn.plsn_num)//try over
				{
				    osal_mutex_unlock(priv->plsn.plsn_mutex);
					PLSN_PRINTF("[%s %d]return PLSN_OVER\r\n", __FUNCTION__, __LINE__);
					return PLSN_OVER;
				}

				//added by yiping 20170918
				if(*index == 0)
				{
					if(priv->plsn.plsn_num == 1)
					{
						if(priv->plsn.plsn_same_value == priv->plsn.plsn_val[0])
						{
							priv->plsn.plsn_same_cnt ++;
						}
						else
						{
							priv->plsn.plsn_same_value = priv->plsn.plsn_val[0];
							priv->plsn.plsn_same_cnt = 1;
						}
						PLSN_PRINTF("[%s %d] plsn_same_value = %d, plsn_same_cnt = %d \n", __FUNCTION__, __LINE__, priv->plsn.plsn_same_value,priv->plsn.plsn_same_cnt);
					}
					else
					{
						priv->plsn.plsn_same_value = 262144;
						priv->plsn.plsn_same_cnt = 0;
					}
				}
				
				for ( ; *index < priv->plsn.plsn_num;)
				{
					priv->plsn.plsn_try_val = priv->plsn.plsn_val[*index];
					priv->plsn.plsn_try = 0;
					*index = *index + 1;
					PLSN_PRINTF("[%s %d]*index=%d, priv->plsn.plsn_num=%d\n", __FUNCTION__, __LINE__, *index, priv->plsn.plsn_num);
					
					//even if priv->plsn.plsn_val=0, also need to try again. //yiping 20170918
					//if (priv->plsn.plsn_now != priv->plsn.plsn_try_val)
					{
						PLSN_PRINTF("[%s %d]priv->plsn.plsn_try_val=%d\n", __FUNCTION__, __LINE__, priv->plsn.plsn_try_val);
						//reset demod
						nim_c3501h_reset_fsm(dev);
						//set plsN
						priv->plsn.plsn_try = 1;
						nim_c3501h_set_plsn(dev);					
						//start demod
						nim_c3501h_start_fsm(dev);

						break;
					}
				}			
			}
			else//the plsn hasn't found
			{
				if ((0 == priv->plsn.start_search) && (1 == priv->plsn.search_plsn_stop))//Completely withdrawal from the search process
				{
					osal_mutex_unlock(priv->plsn.plsn_mutex);
					PLSN_PRINTF("[%s %d]return PLSN_STOPED\r\n", __FUNCTION__, __LINE__);
					return PLSN_STOPED;
				}
				else//the task is still running
				{	
					//NIM_PRINTF("[%s %d]return PLSN_RUNNING\r\n", __FUNCTION__, __LINE__);
					osal_mutex_unlock(priv->plsn.plsn_mutex);
					return PLSN_RUNNING;
				}
			}
	}
	else
	{
	    osal_mutex_unlock(priv->plsn.plsn_mutex);
		PLSN_PRINTF("[%s %d]return PLSN_UNSUPPORT: super_scan=%d\r\n", __FUNCTION__, __LINE__,priv->plsn.super_scan);
		return PLSN_UNSUPPORT;	//not support super scan
	}

    osal_mutex_unlock(priv->plsn.plsn_mutex);
	PLSN_PRINTF("[%s %d]return PLSN_FIND\r\n", __FUNCTION__, __LINE__);
	return PLSN_FIND;
}


/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_exit(struct nim_device *dev)
* Description: Exit plsn searh and initial some variable
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_search_plsn_exit(struct nim_device *dev)
{
	struct nim_c3501h_private *priv = NULL;

	if(NULL == dev)
    {
        ERR_PRINTF("[%s %d]NULL == dev\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }
	
	priv = (struct nim_c3501h_private *)dev->priv; 
	if(NULL == dev->priv)
    {
        ERR_PRINTF("[%s %d]NULL == dev->priv\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }

	osal_mutex_lock(priv->plsn.plsn_mutex, OSAL_WAIT_FOREVER_TIME);//robin.gan add
	priv->plsn.search_plsn_stop = 1; //for plsn_software and plsn_hardware 
	priv->plsn.plsn_find = 0; //for plsn_software
	priv->plsn.plsn_hw_find = 0; //for plsn_hardware
	priv->plsn.plsn_hw_end = 0; //for plsn_hardware
	priv->plsn.plsn_num = 0; //for plsn_software and plsn_hardware 
	osal_mutex_unlock(priv->plsn.plsn_mutex);
	memset(priv->plsn.plsn_val,0,sizeof(priv->plsn.plsn_val)); //for plsn_software and plsn_hardware 

	return SUCCESS;
}



/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_hw(struct nim_device *dev)
* Description: Plsn hardware search flow
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_search_plsn_hw(struct nim_device *dev)
{
    UINT32 i = 0;
	UINT8  data = 0;
	UINT16 lock_state = 0;
	INT32  plsn_timer = 0;
	INT32  search_timeout = 500;
	INT32  pl_lock_timeout = 500;
	INT32  cr_lock_timeout = 500;
	INT32  lock_timer = 0;
	UINT8  sim_end = 0;
    #ifdef PLSN_DEBUG
	    INT32  start_t, tmp_t, end_t;
	#endif
    
	struct nim_c3501h_private *priv = NULL;
	if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	priv = (struct nim_c3501h_private *)dev->priv; 

	PLSN_PRINTF("enter [%s %d] \r\n",__FUNCTION__, __LINE__);

    #ifdef PLSN_DEBUG
	    start_t = osal_get_tick(); //record the start time
    #endif

	//initialization
	priv->plsn.plsn_hw_end = 0;
	priv->plsn.plsn_hw_find = 0;
	priv->plsn.plsn_num = 0;
	memset(priv->plsn.plsn_val, 0, sizeof(priv->plsn.plsn_val));

    //set parameters
    if (0 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 9);	// freeze at cr
		search_timeout = 500;
	}
	else if (1 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 11);	// freeze at ldpc
		search_timeout = 500;
	}
	else if (2 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 9);
		search_timeout = 500;
	}
	else if (3 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 11);
		search_timeout = 500;
	}
	else if (4 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 11);
		search_timeout = 750;
	}
	else if (5 == priv->plsn.search_algo)
	{
		nim_c3501h_freeze_fsm(dev, 1, 11);
		search_timeout = 1000;
	}

	//wait for PL/CR to lock
	sim_end = 0;
	while(1)
	{
		if(priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: stop_flag = 1 \r\n", __FUNCTION__, __LINE__);
			sim_end = 1;
		}

		if(priv->plsn.search_plsn_force_stop)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: search_plsn_force_stop = 1 \r\n", __FUNCTION__, __LINE__);
			sim_end = 1;
		}

		if(sim_end == 1)
		{
			PLSN_PRINTF("[%s %d]clear nim_c3501h_freeze_fsm and nim_c3501h_preset_fsm\n", __FUNCTION__, __LINE__);
			nim_c3501h_freeze_fsm(dev, 0, 0);
			nim_c3501h_preset_fsm(dev, 0, 0);
			return SUCCESS;
		}

	 	comm_sleep(10);
		nim_c3501h_get_lock_reg(dev, &lock_state);
		lock_timer ++;
		if((lock_timer%10) == 0)
		{
		    PLSN_PRINTF("		lock_timer = %d, c3501h_lock_state = 0x%04x \r\n", lock_timer, lock_state);
		}
		if ((0 == priv->plsn.search_algo) || (2 == priv->plsn.search_algo))
		{
			if (lock_state & 0x100) //wait for PL to lock
			{
				PLSN_PRINTF("		pl_lock at pl_timer = %d \r\n", lock_timer);
				priv->plsn.search_lock_state |= 0x01;
				break;
			}
			else if(lock_timer >= pl_lock_timeout)
			{
				PLSN_PRINTF("		pl_lock_timeout at pl_timer = %d \r\n", lock_timer);
				nim_c3501h_search_plsn_exit(dev);
				sim_end = 1;
			}
		}
		else 
		{
			if (lock_state & 0x400) //wait for CR to lock
			{
				PLSN_PRINTF("		cr_lock at cr_timer = %d \r\n", lock_timer);
				priv->plsn.search_lock_state |= 0x02;
				break;
			}
			else if(lock_timer >= cr_lock_timeout)
			{
				PLSN_PRINTF("		cr_lock_timeout at cr_timer = %d \r\n", lock_timer);
				nim_c3501h_search_plsn_exit(dev);
				sim_end = 1;
			}
		}
	}

	PLSN_PRINTF("[%s %d]: waiting for PL/CR to lock, cost_time = %d ms \r\n", __FUNCTION__, __LINE__, osal_get_tick() - start_t);
	
	nim_c3501h_sel_plsn_search_algo(dev, priv->plsn.search_algo);
	nim_c3501h_clear_interrupt(dev);
    
	#ifdef PLSN_DEBUG
	    tmp_t = osal_get_tick();
    #endif

	//enable plsn_en
	nim_c3501h_start_plsn_search(dev);

    //wait for plsn_hw to search PLSN
    sim_end = 0;
	while(1)
	{
		if(priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: stop_flag = 1 \r\n", __FUNCTION__, __LINE__);
			sim_end = 1;
		}

		if(priv->plsn.search_plsn_force_stop)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: search_plsn_force_stop = 1 \r\n",__FUNCTION__, __LINE__);
			sim_end = 1;
		}
		
		if(sim_end == 1)
		{
			PLSN_PRINTF("[%s %d]clear nim_c3501h_freeze_fsm and nim_c3501h_preset_fsm\n", __FUNCTION__, __LINE__);
			nim_c3501h_freeze_fsm(dev, 0, 0);
			nim_c3501h_preset_fsm(dev, 0, 0);
			break;
		}

		comm_sleep(10);
		plsn_timer ++;
		nim_c3501h_get_plsn_search_ret(dev);

		if (priv->plsn.plsn_hw_end) //end to search???
		{
			PLSN_PRINTF("[%s %d]: PLSN_HW end to search, plsn_timer = %d, cost_time = %d ms \r\n", __FUNCTION__, __LINE__, plsn_timer, osal_get_tick() - tmp_t);

			if(priv->plsn.plsn_hw_find)
			{
				nim_c3501h_get_interrupt(dev, &data);
				if (data & 0x30)	// tr or cr loss lock
				{
					PLSN_PRINTF("[%s %d]: interrupt found !!!!!! data =0x%02x \r\n", __FUNCTION__, __LINE__, data);
				}
				if(priv->plsn.plsn_num > 0)
				{						
					PLSN_PRINTF("[%s %d]: search successfully, plsn_num = %d \r\n", __FUNCTION__, __LINE__, priv->plsn.plsn_num);
					for (i = 0; i < priv->plsn.plsn_num; i++)
					{
						PLSN_PRINTF("		priv->plsn.plsn_val[%d]=%d \r\n", i, priv->plsn.plsn_val[i]);
					}
				}
			}
			else
			{
				nim_reg_read(dev, C3501H_RFC_PLSN_RPT + 3, &data, 1);
			    PLSN_PRINTF("[%s %d]: search failed, PLSN_STATUS_MON=%x \r\n", __FUNCTION__, __LINE__, data&0xf0);
			}
			sim_end = 1;
		}
		else if(plsn_timer >= search_timeout)
		{
			nim_c3501h_search_plsn_exit(dev);
			nim_reg_read(dev, C3501H_RFC_PLSN_RPT + 3, &data, 1);
			PLSN_PRINTF("[%s %d]: PLSN_HW timeout at max plsn_timer = %d ,PLSN_STATUS_MON = %x \r\n", __FUNCTION__, __LINE__, search_timeout,data&0xf0);
			sim_end = 1;
		}
	}

	//disable plsn_en
	nim_c3501h_close_plsn_search(dev);

    #ifdef PLSN_DEBUG
	    end_t = osal_get_tick();
    #endif
	PLSN_PRINTF("[%s %d]: search_algo = %d, cost_time = %dms, plsn_hw_end = %d, plsn_num = %d \r\n", __FUNCTION__, __LINE__, priv->plsn.search_algo, end_t - tmp_t, priv->plsn.plsn_hw_end, priv->plsn.plsn_num);

	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_hw(struct nim_device *dev)
* Description: Top of Plsn hardware search flow, search plsn and try to lock it
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_search_plsn_hw_top(struct nim_device *dev)
{
    UINT8 data = 0;
	INT32 plsn_hw_search_algo_index = 0;
	INT32 plsn_hw_search_cnt_thr[6] = {10,10,5,5,10,5};
	INT32 plsn_hw_search_cnt = 0;
	INT32 result;
	
    struct nim_c3501h_private *priv = NULL;
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	priv = (struct nim_c3501h_private *)dev->priv; 

	PLSN_PRINTF("enter [%s %d] \r\n",__FUNCTION__, __LINE__);

	//search plsn by all kinds of searching methods
	plsn_hw_search_algo_index = 0; //0,1,2,3,4,5
	plsn_hw_search_cnt = 0;
	while(1)
	{
		if(priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: stop_flag = 1 \r\n", __FUNCTION__, __LINE__);
			break;
		}

		if(priv->plsn.search_plsn_force_stop)
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d]: search_plsn_force_stop = 1 \r\n",__FUNCTION__, __LINE__);
			break;
		}

		if (0x01 & (priv->plsn.plsn_algo_preset >> plsn_hw_search_algo_index)) //if searching method is enable, enter to search plsn
		{
		    priv->plsn.search_algo = plsn_hw_search_algo_index;
			priv->plsn.plsn_hw_find = 0;
			priv->plsn.plsn_hw_end = 0;
			priv->plsn.plsn_num = 0;
			memset(priv->plsn.plsn_val, 0, sizeof(priv->plsn.plsn_val));
			
			PLSN_PRINTF("[%s %d]: try search_algo = %d, search_cnt = %d \r\n", __FUNCTION__, __LINE__, priv->plsn.search_algo, plsn_hw_search_cnt);
            result = nim_c3501h_search_plsn_hw(dev);
			plsn_hw_search_cnt ++;
			
            if(result == SUCCESS)
            {
				if(1 == priv->plsn.plsn_hw_end) //enter to search plsn_hw
				{
				    nim_reg_read(dev, C3501H_RFC_PLSN_RPT + 3, &data, 1);
			    	if((1 == priv->plsn.plsn_hw_find) && (priv->plsn.plsn_num <= 5))//if find sucessfully, then exit
					{
			    		priv->plsn.search_plsn_stop = 1;
						break;
					}
#ifdef PLSN_262141_PATCH    // For spec plsn value:262141 in BBH algo 20180106 paladin
					else if ((4 == priv->plsn.search_algo) && (0 == priv->plsn.plsn_patch_flag))
					{
						priv->plsn.search_plsn_stop = 1;
						priv->plsn.plsn_hw_find = 1;
						priv->plsn.plsn_val[0] = 262141;
						priv->plsn.plsn_num = 1;
						priv->plsn.plsn_patch_flag = 1;
						break; 
					}
#endif
					else if((priv->plsn.search_algo < 4) && (0 == priv->plsn.plsn_hw_find) && (0 == (data&0x20))) //never enter the plsn_search phase  //have no pilots and dummy-frames
					{
			        	PLSN_PRINTF("[%s %d]: have no pilots and dummy-frames \r\n", __FUNCTION__, __LINE__);
			        	plsn_hw_search_algo_index ++;
				    	plsn_hw_search_cnt = 0;
					}
					else if(plsn_hw_search_cnt >= plsn_hw_search_cnt_thr[priv->plsn.search_algo]) //if more than the allowed count, change to the next method
					{
			    		plsn_hw_search_algo_index ++;
						plsn_hw_search_cnt = 0;
					}
				}
				else //PL/CR_lock timeout or searching plsn timeout, then exit
				{
			        break;
				}
            }
			else //ERR_NO_DEV
			{
			    break;
			}
		}
		else //if searching method is disable, skip to the next method
		{
		    plsn_hw_search_algo_index ++;
		}

		if(plsn_hw_search_algo_index >= PLSN_SEARCH_ALGO_NUM) //all methods have been used, but PLSN cannot be found
		{
			nim_c3501h_search_plsn_exit(dev);
			PLSN_PRINTF("[%s %d] Sorry! All methods have been tried, but failed! \r\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	PLSN_PRINTF("leave [%s %d] \r\n",__FUNCTION__, __LINE__);

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_sw_top(struct nim_device *dev)
* Description: Top of Plsn software search flow, search plsn and try to lock it, now is unused
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_search_plsn_sw_top(struct nim_device *dev)
{
#if 0
    struct nim_c3501h_private *priv = NULL;
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	priv = (struct nim_c3501h_private *)dev->priv; 
#endif

	PLSN_PRINTF("Sorry, searching PLSN by software method in c3501H cannot supported!!!!!!! \r\n");

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_sw_top(struct nim_device *dev)
* Description: Top of Plsn search flow, search plsn and try to lock it
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_search_plsn_top(struct nim_device *dev)
{
    struct nim_c3501h_private *priv = NULL;
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n",__FUNCTION__);
        return ERR_NO_DEV;
    }
	priv = (struct nim_c3501h_private *)dev->priv; 

	PLSN_PRINTF("[%s %d]priv->plsn.plsn_hw_en=%d\n", __FUNCTION__, __LINE__, priv->plsn.plsn_hw_en);
	PLSN_PRINTF("[%s %d]priv->plsn.plsn_algo_preset=%d\n", __FUNCTION__, __LINE__, priv->plsn.plsn_algo_preset);
	if(priv->plsn.plsn_hw_en)
	{
	    nim_c3501h_search_plsn_hw_top(dev);
	}
    else
    {
        nim_c3501h_search_plsn_sw_top(dev);
    }

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_search_plsn_sw_top(struct nim_device *dev)
* Description: 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
void nim_c3501h_plsn_task(UINT32 param1, UINT32 param2)                                                                                        
{                                                                                                                                        
	struct nim_device *dev = (struct nim_device *) param1;  
    struct nim_c3501h_private *priv = NULL;
    priv = (struct nim_c3501h_private *)dev->priv; 
    
	//int i = 0;
	#ifdef PLSN_DEBUG
	    UINT32 start_t;
    #endif

	NIM_PRINTF("Enter %s \r\n", __FUNCTION__);

	while (1)		
	{
	    //priv->plsn.super_scan is set by upper level
		//priv->plsn.start_search is set in nim_c3501h_channel_change and nim_c3501h_soft_search
		//priv->ul_status.c3501h_chanscan_stop_flag is cleared in nim_c3501h_waiting_channel_lock
		//priv->plsn.auto_scan_start is set in nim_c3501h_soft_search
		if ((1 == priv->plsn.super_scan) 
			&& (1 == priv->plsn.start_search) 
			&& ((0 == priv->ul_status.c3501h_chanscan_stop_flag)
			     || (1 == priv->plsn.auto_scan_start)))// do super scan for plsn
		{
			PLSN_PRINTF("start to search plsn! \r\n");
            #ifdef PLSN_DEBUG
			    start_t = osal_get_tick();
            #endif
			
			//nim_c3501h_set_pl_cfe_dis_pilot(dev); // Paladin 20180106 for iq swap & pilot pl cfe bug
			nim_c3501h_set_set_cr_dis_pilot(dev);
			
			priv->plsn.search_plsn_stop = 0;
			priv->plsn.plsn_find = 0;
			priv->plsn.plsn_hw_find = 0;
			priv->plsn.plsn_hw_end = 0;
			priv->plsn.search_lock_state = 0;
			priv->plsn.plsn_num = 0;
			priv->plsn.plsn_try = 0;
			priv->plsn.plsn_try_val = 0;
			memset(priv->plsn.plsn_val, 0, sizeof(priv->plsn.plsn_val));
			priv->plsn.auto_scan_start = 0;

			nim_c3501h_search_plsn_top(dev);

			//nim_c3501h_set_pl_cfe_en_pilot(dev);  // Paladin 20180106 for iq swap & pilot pl cfe bug
			nim_c3501h_set_set_cr_en_pilot(dev);

			priv->plsn.start_search = 0;
			
			PLSN_PRINTF("end to search plsn, cost %d ms\n", osal_get_tick() - start_t);
		}
		comm_sleep(10);
	}
	
	//return SUCCESS;
}


// Sub register read/write operate

//FSM_SUB_REG_INDEX       //0X10[6:0]
//FSM_SUB_REG_RW          //0X10[7]
//FSM_SUB_REG_WDATA       //0X10[15:8]
//FSM_SUB_REG_RDATA[7:0]  //0X10[23:16],RO
INT32 nim_c3501h_sub_read_fsm(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_FSM_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_FSM_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0x7f;
    nim_reg_write(dev, C3501H_R10_TOP_FSM_CFG_5  , &reg_addr, 1); //write sub addr
    nim_reg_read (dev, C3501H_R10_TOP_FSM_CFG_5+2, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_FSM_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_fsm(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_FSM_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_FSM_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
	reg_addr = (ADDR & 0x7f) | 0x80; 
    nim_reg_write(dev, C3501H_R10_TOP_FSM_CFG_5+1, &reg_data, 1); // write data first
    nim_reg_write(dev, C3501H_R10_TOP_FSM_CFG_5  , &reg_addr, 1); // write_en is the 7th bit of addr

    //release mutex
    SUB_REG_FSM_MUTEX = 0;
    
    return SUCCESS;
}

//NEW_IQB_WDAT      //0X3C[11:0]
//NEW_IQB_WR_MUX    //0X3C[14:12]
//NEW_IQB_WEN       //0X3C[15]
//NEW_IQB_PAR_R_PUL //0X3C[16]
//NEW_IQB_RDAT[24:0]//0X40[24:0] ,RO
INT32 nim_c3501h_sub_read_new_iqb(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[4];

    //wait for mutex
    while (SUB_REG_NEW_IQB_MUTEX)
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_NEW_IQB_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_R3C_NEW_IQB_CFG+1, &reg_data, 1); 
    reg_data = reg_data & 0x0f;
    ADDR     = ADDR & 0x7;
    reg_data = reg_data | (ADDR<<4);
    nim_reg_write(dev, C3501H_R3C_NEW_IQB_CFG+1, &reg_data, 1); //write sub addr

    nim_reg_read (dev, C3501H_R3C_NEW_IQB_CFG+2, &reg_data, 1);
    reg_data = reg_data | 0x1;
    nim_reg_write(dev, C3501H_R3C_NEW_IQB_CFG+2, &reg_data, 1); // write sub read pulse

    nim_reg_read (dev, C3501H_R40_NEW_IQB_RPT, reg_data_array, 4); //read sub value
    reg_data_array[3] = reg_data_array[3] & 0x1;
    
    //release mutex
    SUB_REG_NEW_IQB_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1]<<8) | (reg_data_array[2]<<16) | (reg_data_array[3]<<24);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_new_iqb(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	//UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_NEW_IQB_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_NEW_IQB_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;        //data
    reg_data_array[1] = ((VALUE>>8) & 0xf);  //data
    reg_data_array[1] = reg_data_array[1] | ((ADDR & 07)<<4); //addr
    reg_data_array[1] = reg_data_array[1] | 0x80; //wen

    nim_reg_write(dev, C3501H_R3C_NEW_IQB_CFG  , &reg_data_array[0], 1);
    nim_reg_write(dev, C3501H_R3C_NEW_IQB_CFG+1, &reg_data_array[1], 1);//make sure wen write at last

    //release mutex
    SUB_REG_NEW_IQB_MUTEX = 0;
    
    return SUCCESS;
}

//CCI_WEN               //0X48[15]
//CCI_WDAT              //0X48[27:16]
//CCI_WR_MUX            //0X48[31:28] 
//CCI_RDAT[11:0]        //0X4C[11:0] ,RO
//CCI_PLL_PAR_R_PUL     //0X4C[25]
//CCI_PLL_PAR_R_DIRECT  //0X4C[26]
INT32 nim_c3501h_sub_read_cci(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_CCI_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_CCI_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_R48_CCI_CANCEL_CFG_1+3, &reg_data, 1); 
    reg_data = reg_data & 0x0f;
    ADDR     = ADDR & 0xf;
    reg_data = reg_data | (ADDR<<4);
    nim_reg_write(dev, C3501H_R48_CCI_CANCEL_CFG_1+3, &reg_data, 1);  //write sub addr

    nim_reg_read (dev, C3501H_R4C_CCI_CANCEL_RPT+3, &reg_data, 1);
    reg_data = reg_data | 0x02;
    nim_reg_write(dev, C3501H_R4C_CCI_CANCEL_RPT+3, &reg_data, 1);    // write sub read pulse

    nim_reg_read (dev, C3501H_R4C_CCI_CANCEL_RPT, reg_data_array, 2); //read sub value
    reg_data_array[1] = reg_data_array[1] & 0x0f;
    
    //release mutex
    SUB_REG_CCI_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1]<<8);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_cci(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_CCI_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_CCI_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;        //data
    reg_data_array[1] = ((VALUE>>8) & 0xf);  //data
    ADDR = ADDR & 0xf;
    reg_data_array[1] = reg_data_array[1] | (ADDR<<4); //addr
    nim_reg_write(dev, C3501H_R48_CCI_CANCEL_CFG_1+2, reg_data_array, 2);

    nim_reg_read (dev, C3501H_R48_CCI_CANCEL_CFG_1+1, &reg_data, 1);
    reg_data = reg_data | 0x80;
    nim_reg_write(dev, C3501H_R48_CCI_CANCEL_CFG_1+1, &reg_data, 1); //wen

    //release mutex
    SUB_REG_CCI_MUTEX = 0;
    
    return SUCCESS;
}

//TR_SUB_REG_WDATA       = TR_CFG[15:0]            ; //0X58[15:0]
//TR_SUB_REG_INDEX       = TR_CFG[21:16]           ; //0X58[21:16]
//TR_SUB_REG_WEN_PLUSE   = TR_CFG[22]              ; //0X58[22]
//TR_SUB_REG_REN_PLUSE   = TR_CFG[23]              ; //0X58[23]
//TR_RPT                 = TR_SUB_REG_RDATA[15:0]  ; //0X5C[15:0] ,RO
INT32 nim_c3501h_sub_read_tr(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	//UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_TR_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_TR_MUTEX = 1; 

    //main body
    reg_addr = (ADDR & 0x3f) | 0x80;
    nim_reg_write(dev, C3501H_R58_TR_CFG_0+2, &reg_addr, 1);      //write sub addr && read pulse
    nim_reg_read (dev, C3501H_R5C_TR_CFG_1  , reg_data_array, 2); //read sub value
    
    //release mutex
    SUB_REG_TR_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1]<<8);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_tr(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	//UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_TR_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_TR_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;         //data
    reg_data_array[1] = ((VALUE>>8) & 0xff);
    nim_reg_write(dev, C3501H_R58_TR_CFG_0, reg_data_array, 2);

    ADDR              = ADDR & 0x3f;          //addr
    reg_addr          = ADDR | 0x40;          //wen
    nim_reg_write(dev, C3501H_R58_TR_CFG_0+2, &reg_addr, 1);

    //release mutex
    SUB_REG_TR_MUTEX = 0;
    
    return SUCCESS;
}

//BEQ_SUB_REG_INDEX = BEQ_CFG[2:0]               ; //0X68[2:0]
//BEQ_SUB_REG_RW    = BEQ_CFG[3]                 ; //0X68[7]
//BEQ_SUB_REG_WDATA = BEQ_CFG[11:4]              ; //0X68[15:8]
//BEQ_RPT[7:0]      = BEQ_SUB_REG_RDATA[7:0]     ; //0X68[23:16],RO
INT32 nim_c3501h_sub_read_beq(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_BEQ_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_BEQ_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0x07;
    nim_reg_write(dev, C3501H_R68_BEQ_WORK_MODE_CFG  , &reg_addr, 1); //write sub addr
    nim_reg_read (dev, C3501H_R68_BEQ_WORK_MODE_CFG+2, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_BEQ_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_beq(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_BEQ_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_BEQ_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
	ADDR     = ADDR  & 0x07; 
	reg_addr = ADDR  | 0x80; //wen
    nim_reg_write(dev, C3501H_R68_BEQ_WORK_MODE_CFG+1, &reg_data, 1); // write data first
    nim_reg_write(dev, C3501H_R68_BEQ_WORK_MODE_CFG  , &reg_addr, 1); // write_en is the 7th bit of addr

    //release mutex
    SUB_REG_BEQ_MUTEX = 0;
    
    return SUCCESS;
}

//PL_CFG_ADDR     ; //0X90[30:24]
//PL_CFG_WR       ; //0X90[31]
//PL_RPT[124:109] ; //0X94[15:0] ,RO
//PL_CFG_DATA     ; //0X94[31:16]
INT32 nim_c3501h_sub_read_pl(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	//UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_PL_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PL_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0x7f;
    nim_reg_write(dev, C3501H_R90_PL_CFG_5+3  , &reg_addr, 1); //write sub addr
    nim_reg_read (dev, C3501H_R94_PL_CFG_6    , reg_data_array, 2); //read sub value
    
    //release mutex
    SUB_REG_PL_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1]<<8);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_pl(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	//UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_PL_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PL_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;
    reg_data_array[1] = (VALUE>>8) & 0xff;
	ADDR              = ADDR  & 0x7f; 
	reg_addr          = ADDR  | 0x80; //wen
    nim_reg_write(dev, C3501H_R94_PL_CFG_6+2, reg_data_array, 2); // write data first
    nim_reg_write(dev, C3501H_R90_PL_CFG_5+3, &reg_addr, 1);      // write_en is the 7th bit of addr

    //release mutex
    SUB_REG_PL_MUTEX = 0;
    
    return SUCCESS;
}

//DEMAP_AGC3_WR_MUX   ; //0X98[30:28]
//DEMAP_AGC3_WEN      ; //0X98[31]
//DEMAP_AGC3_WDAT     ; //0X9C[7:0]
//DEMAP_AGC3_RPT[7:0] ; //0X9C[15:8] ,RO
INT32 nim_c3501h_sub_read_demap_agc3(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_AGC3_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_AGC3_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_R98_AGC3_CFG+3, &reg_data, 1); 
    reg_data = reg_data & 0x0f;
    ADDR     = ADDR & 0x7;
    reg_data = reg_data | (ADDR<<4);
    nim_reg_write(dev, C3501H_R98_AGC3_CFG+3, &reg_data, 1);       //write sub addr

    nim_reg_read (dev, C3501H_R9C_DEMAP_AGC3_CFG+1, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_DEMAP_AGC3_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_demap_agc3(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_AGC3_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_AGC3_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
    nim_reg_write(dev, C3501H_R9C_DEMAP_AGC3_CFG, &reg_data, 1); // write data first

    nim_reg_read (dev, C3501H_R98_AGC3_CFG+3, &reg_data, 1); 
    reg_data = reg_data & 0x0f;
    ADDR     = ADDR & 0x7;
    reg_data = reg_data | (ADDR<<4) | 0x80;
    nim_reg_write(dev, C3501H_R98_AGC3_CFG+3, &reg_data, 1); //write sub addr

    //release mutex
    SUB_REG_DEMAP_AGC3_MUTEX = 0;
    
    return SUCCESS;
}

//CR_SUB_REG_ADDR  //0XD0[6:0]
//CR_SUB_REG_RW    //0XD0[7]
//CR_SUB_REG_WDATA //0XD0[15:8]
//CR_SUB_REG_RDATA //0XD0[23:16],RO
INT32 nim_c3501h_sub_read_cr(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_CR_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_CR_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0x7f;
    nim_reg_write(dev, C3501H_RD0_CR_SUB_REG_CFG  , &reg_addr, 1); //write sub addr
    nim_reg_read (dev, C3501H_RD0_CR_SUB_REG_CFG+2, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_CR_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;

}

INT32 nim_c3501h_sub_write_cr(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_CR_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_CR_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
	ADDR     = ADDR  & 0x7f; 
	reg_addr = ADDR  | 0x80;
    nim_reg_write(dev, C3501H_RD0_CR_SUB_REG_CFG+1, &reg_data, 1); // write data first
    nim_reg_write(dev, C3501H_RD0_CR_SUB_REG_CFG  , &reg_addr, 1); // write_en is the 7th bit of addr

    //release mutex
    SUB_REG_CR_MUTEX = 0;
    
    return SUCCESS;
}

//EQ_DD_SUB_REG_INDEX  ; //0XDC[3:0]
//EQ_DD_SUB_REG_RW     ; //0XDC[7]
//EQ_DD_SUB_REG_WDATA  ; //0XDC[15:8]
//EQ_DD_RPT[7:0]       ; //0XDC[23:16],RO
INT32 nim_c3501h_sub_read_eq_dd(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_EQ_DD_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_EQ_DD_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0x0f;
    nim_reg_write(dev, C3501H_RDC_EQ_DD_CFG_1  , &reg_addr, 1); //write sub addr
    nim_reg_read (dev, C3501H_RDC_EQ_DD_CFG_1+2, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_EQ_DD_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_eq_dd(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_EQ_DD_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_EQ_DD_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
	ADDR     = ADDR  & 0x0f; 
	reg_addr = ADDR  | 0x80;
    nim_reg_write(dev, C3501H_RDC_EQ_DD_CFG_1+1, &reg_data, 1); // write data first
    nim_reg_write(dev, C3501H_RDC_EQ_DD_CFG_1  , &reg_addr, 1); // write_en is the 7th bit of addr

    //release mutex
    SUB_REG_EQ_DD_MUTEX = 0;
    
    return SUCCESS;
}

//DEMAP_SUB_REG_WDATA      //0XE4[15:0]
//DEMAP_SUB_REG_INDEX      //0XE4[23:16]
//DEMAP_SUB_REG_CMD        //0XE4[28:24]
//DEMAP_SUB_REG_WEN_PLUSE  //0XE4[29]
//DEMAP_SUB_REG_REN_PLUSE  //0XE4[30]
//DEMAP_SUB_REG_RDATA[15:0]//0XE8[15:0] ,RO
INT32 nim_c3501h_sub_read_demap(struct nim_device *dev, INT32 ADDR, INT32 CMD, INT32 *VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_DEMAP_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_MUTEX = 1; 

    //main body
    reg_addr = ADDR & 0xff;
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG+2, &reg_addr, 1); //write sub addr

    nim_reg_read (dev, C3501H_RE4_DEMAP_CFG+3, &reg_data, 1); 
    reg_data = reg_data & 0x80;
    CMD      = CMD & 0x1f;
    reg_data = reg_data | CMD | 0x40;
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG+3, &reg_data, 1); //write ren pulse and cmd

    nim_reg_read (dev, C3501H_RE8_DEMAP_RPT  , reg_data_array, 2); //read sub value
    
    //release mutex
    SUB_REG_DEMAP_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1] <<8);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_demap(struct nim_device *dev, INT32 ADDR, INT32 CMD, INT32 VALUE)
{
    int i=0;
    UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_DEMAP_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;
    reg_data_array[1] = (VALUE>>8) & 0xff;
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG, reg_data_array, 2); //write data

    reg_addr = ADDR  & 0xff;
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG+2, &reg_addr, 1); //write addr

    nim_reg_read (dev, C3501H_RE4_DEMAP_CFG+3, &reg_data, 1); 
    reg_data = reg_data & 0x80;
    CMD      = CMD & 0x1f;
    reg_data = reg_data | CMD | 0x20;
    nim_reg_write(dev, C3501H_RE4_DEMAP_CFG+3, &reg_data, 1); //write wen pulse and cmd

    //release mutex
    SUB_REG_DEMAP_MUTEX = 0;
    
    return SUCCESS;
}

//DEMAP_FLT_SUB_REG_WEN_PLUSE    //0XEC[1]
//DEMAP_FLT_SUB_REG_REN_PLUSE    //0XEC[2]
//DEMAP_FLT_SUB_REG_INDEX        //0XEC[7:3]
//DEMAP_FLT_SUB_REG_WDATA        //0XEC[15:8]
//DEMAP_FLT_SUB_REG_RDATA[7:0]   //0XEC[23:16],RO
INT32 nim_c3501h_sub_read_demap_flt(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_FLT_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_FLT_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_REC_DEMAP_FLT_CFG, &reg_data, 1); 
    reg_data = reg_data & 0x1;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | (ADDR<<3) | 0x04; //addr and ren pulse
    nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG  , &reg_data, 1);  //write sub addr and ren pulse
    nim_reg_read (dev, C3501H_REC_DEMAP_FLT_CFG+2, &reg_data, 1);  //read sub value
    
    //release mutex
    SUB_REG_DEMAP_FLT_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_demap_flt(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_FLT_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_FLT_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
    nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG+1, &reg_data, 1); //write data first

    nim_reg_read (dev, C3501H_REC_DEMAP_FLT_CFG, &reg_data, 1); 
    reg_data = reg_data & 0x1;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | (ADDR<<3) | 0x02; //addr and wen pulse
    nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG  , &reg_data, 1);  //write sub addr and wen pulse

    //release mutex
    SUB_REG_DEMAP_FLT_MUTEX = 0;
    
    return SUCCESS;
}

//DEMAP_MON_TSN_SUB_REG_REN_PLUSE    //0XEF[0]
//DEMAP_MON_TSN_SUB_REG_INDEX        //0XEF[5:1]
//DEMAP_MON_TSN_SUB_REG_RDATA[7:0]   //0XF3[7:0],RO
INT32 nim_c3501h_sub_read_demap_mon_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_MON_TSN_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_MON_TSN_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_REC_DEMAP_FLT_CFG+3, &reg_data, 1); 
    reg_data = reg_data & 0xc0;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | (ADDR<<1) | 0x01; //addr and ren pulse
    nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG+3, &reg_data, 1);  //write sub addr and ren pulse
    nim_reg_read (dev, C3501H_RF0_DEMAP_FLT_CFG+3, &reg_data, 1);  //read sub value
    
    //release mutex
    SUB_REG_DEMAP_MON_TSN_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

//DEMAP_FLT_TSN_SUB_REG_WEN_PLUSE   //0XF0[1]
//DEMAP_FLT_TSN_SUB_REG_REN_PLUSE   //0XF0[2]
//DEMAP_FLT_TSN_SUB_REG_INDEX       //0XF0[7:3]
//DEMAP_FLT_TSN_SUB_REG_WDATA       //0XF1[7:0]
//DEMAP_FLT_TSN_SUB_REG_RDATA[7:0]  //0XF2[7:0],RO
INT32 nim_c3501h_sub_read_demap_flt_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_FLT_TSN_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_FLT_TSN_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_RF0_DEMAP_FLT_CFG, &reg_data, 1); 
    reg_data = reg_data & 0x1;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | (ADDR<<3) | 0x04; //addr and ren pulse
    nim_reg_write(dev, C3501H_RF0_DEMAP_FLT_CFG  , &reg_data, 1);  //write sub addr and ren pulse
    nim_reg_read (dev, C3501H_RF0_DEMAP_FLT_CFG+2, &reg_data, 1);  //read sub value
    
    //release mutex
    SUB_REG_DEMAP_FLT_TSN_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_demap_flt_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_DEMAP_FLT_TSN_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_DEMAP_FLT_TSN_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
    nim_reg_write(dev, C3501H_RF0_DEMAP_FLT_CFG+1, &reg_data, 1); //write data first

    nim_reg_read (dev, C3501H_RF0_DEMAP_FLT_CFG, &reg_data, 1); 
    reg_data = reg_data & 0x1;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | (ADDR<<3) | 0x02; //addr and wen pulse
    nim_reg_write(dev, C3501H_RF0_DEMAP_FLT_CFG  , &reg_data, 1);  //write sub addr and wen pulse

    //release mutex
    SUB_REG_DEMAP_FLT_TSN_MUTEX = 0;
    
    return SUCCESS;
}

//PLSN_SUB_REG_WDATA     //0XF4[15:8]
//PLSN_SUB_REG_INDEX     //0XF4[20:16]
//PLSN_SUB_REG_WEN_PULSE //0XF4[21]
//PLSN_SUB_REG_REN_PULSE //0XF4[22]
//PLSN_SUB_REG_RDATA[7:0]//0XF4[31:24],RO
INT32 nim_c3501h_sub_read_plsn(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_PLSN_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PLSN_MUTEX = 1;

    //main body
    nim_reg_read (dev, C3501H_RF4_PLSN_CFG+2, &reg_data, 1); 
    reg_data = reg_data & 0x80;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | ADDR | 0x40; //addr and ren pulse
    nim_reg_write(dev, C3501H_RF4_PLSN_CFG+2, &reg_data, 1);  //write sub addr and ren pulse
    nim_reg_read (dev, C3501H_RF4_PLSN_CFG+3, &reg_data, 1);  //read sub value
    
    //release mutex
    SUB_REG_PLSN_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_plsn(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_PLSN_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PLSN_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
    nim_reg_write(dev, C3501H_RF4_PLSN_CFG+1, &reg_data, 1); //write data first

    nim_reg_read (dev, C3501H_RF4_PLSN_CFG+2, &reg_data, 1); 
    reg_data = reg_data & 0x80;
    ADDR     = ADDR & 0x1f;
    reg_data = reg_data | ADDR | 0x20; //addr and wen pulse
    nim_reg_write(dev, C3501H_RF4_PLSN_CFG+2, &reg_data, 1);  //write sub addr and ren pulse

    //release mutex
    SUB_REG_PLSN_MUTEX = 0;
    
    return SUCCESS;
}

//VB_WDAT[15:0] , 0X00[15:0]
//VB_RDAT[15:0] , 0X00[31:16]
//VB_WR_MUX[3:0], 0X04[3:0]
//VB_WEN        , 0X04[4]
INT32 nim_c3501h_sub_read_vb(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_VB_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_VB_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_R04_RS_CFG_RPT, &reg_data, 1);
    reg_data = reg_data & 0xe0;
    reg_data = reg_data | (ADDR & 0xf);
    nim_reg_write(dev, C3501H_R04_RS_CFG_RPT  , &reg_data, 1);       //write sub addr
    nim_reg_read (dev, C3501H_R00_VB_CFG_RPT+2, reg_data_array, 2); //read sub value
    
    //release mutex
    SUB_REG_VB_MUTEX = 0; 

    *VALUE = reg_data_array[0] | (reg_data_array[1] <<8);
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_vb(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;
    UINT8 reg_data_array[2];

    //wait for mutex
    while (SUB_REG_VB_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_VB_MUTEX = 1; 

    //main body
    reg_data_array[0] = VALUE & 0xff;
    reg_data_array[1] = (VALUE>>8) & 0xff;
    nim_reg_write(dev, C3501H_R00_VB_CFG_RPT, reg_data_array, 2); //write sub data first

    nim_reg_read (dev, C3501H_R04_RS_CFG_RPT, &reg_data, 1);
    reg_data = reg_data & 0xe0;
    reg_data = reg_data | (ADDR & 0xf) | 0x10;
    nim_reg_write(dev, C3501H_R04_RS_CFG_RPT, &reg_data, 1);       //write sub addr

    //release mutex
    SUB_REG_VB_MUTEX = 0;
    
    return SUCCESS;
}

//PACKET_RDAT   ,0X24[15:8] ,RO
//PACKET_WDAT   ,0X24[23:16]
//PACKET_WR_MUX ,0X24[27:24]
//PACKET_WEN    ,0X24[28]
INT32 nim_c3501h_sub_read_packet(struct nim_device *dev, INT32 ADDR, INT32 *VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_PECKET_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PECKET_MUTEX = 1; 

    //main body
    nim_reg_read (dev, C3501H_R24_PACKET_CFG+3, &reg_data, 1);
    reg_data = reg_data & 0xe0;
    reg_data = reg_data | (ADDR & 0xf);
    nim_reg_write(dev, C3501H_R24_PACKET_CFG+3, &reg_data, 1); //write sub addr
    nim_reg_read (dev, C3501H_R24_PACKET_CFG+1, &reg_data, 1); //read sub value
    
    //release mutex
    SUB_REG_PECKET_MUTEX = 0; 

    *VALUE = reg_data;
    return SUCCESS;
}

INT32 nim_c3501h_sub_write_packet(struct nim_device *dev, INT32 ADDR, INT32 VALUE)
{
    int i=0;
    //UINT8 reg_addr;
	UINT8 reg_data;

    //wait for mutex
    while (SUB_REG_PECKET_MUTEX) 
	{
		i++;
		comm_sleep(1);
		if (i > 100)
		{
			NIM_PRINTF("Function: %s error!!!!!!\r\n",__FUNCTION__);
			return -1;
		}
	}

    //set mutex
	SUB_REG_PECKET_MUTEX = 1; 

    //main body
    reg_data = VALUE & 0xff;
    nim_reg_write(dev, C3501H_R24_PACKET_CFG+2, &reg_data, 1); //write sub data first

    nim_reg_read (dev, C3501H_R24_PACKET_CFG+3, &reg_data, 1);
    reg_data = reg_data & 0xe0;
    reg_data = reg_data | (ADDR & 0xf) | 0x10;
    nim_reg_write(dev, C3501H_R24_PACKET_CFG+3, &reg_data, 1); //write sub addr

    //release mutex
    SUB_REG_PECKET_MUTEX = 0;
    
    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_tsbb_cap_set(struct nim_device *dev, UINT8 s_case, UINT32 swpidf, UINT32 swpidd)
* Description: Set ts bbframe data capture mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_case: select TS_BB_L3 or NIM_CAP_TS2
*  Parameter3: UINT32 swpidf
*  Parameter4: UINT32 swpidd
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tsbb_cap_set(struct nim_device *dev, UINT8 s_case, UINT32 swpidf, UINT32 swpidd)
{
	UINT8 data = 0 ;
	// disable the insertion of dummy TS packet
	nim_reg_read(dev,C3501H_R18_TSO_CFG, &data, 1);
	data &= 0x7f; // [7] = 0;
    nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);

	NIM_PRINTF("[nim_c3501h_tsbb_cap_set]ENTER\n");

    // C3501H' TSO CLK is adaptive ,so delete the section below
	/*data = 0x04;
    nim_reg_write(dev, RFC_TSO_CLK_SW_CFG+1, &data, 1); //TSO_SRC_DIV=2
    data = 0x13;
    nim_reg_write(dev, RFC_TSO_CLK_SW_CFG, &data, 1); //TSO_SRC=108MHz*/


	// sub reg
    // 4/5/6 :fid
    // write to cr126[7:0]
    data = (UINT8)(swpidf & 0xff) ;
    nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x14;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);

	//write to cr126[7:0]
	data = (UINT8)((swpidf>>8)&0xff);
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x15;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);
	
	// write to cr126[7:0]
	data = (UINT8)((swpidf>>16)&0xff);
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x16;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);

	data = (UINT8)(swpidd & 0xff) ;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x10;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);

	data = (UINT8)((swpidd>>8)&0xff) ;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x11;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);

	data = (UINT8)((swpidd>>16)&0xff) ;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 2, &data, 1);
	data = 0x12;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG + 3, &data, 1);


	switch(s_case)
	{
		case NIM_CAP_TS1://TS_BB_L3
		
			NIM_PRINTF("TS_BB_L3 mode: swpidf is %x swpidd is %x\n",swpidf,swpidd);
		    // cr_124[1] = 1: output BB Frame with L3 format
		    //                  
		    // cr_124[3] = 0:disable ISSY Mode
			nim_reg_read(dev, C3501H_R24_PACKET_CFG, &data, 1);
		    data |= 0x02;
			data &= 0xf7;
    		nim_reg_write(dev, C3501H_R24_PACKET_CFG, &data, 1);	

			// cr_118[17] = 0 :
			//                    1: base on bb header ACM/CCM bit.auto swtch packing's ts/gs work mode
			// cr_118[16] = 0: gs, 1:ts
			nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
			data &= 0xfc;
			nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
			
			break;
		case NIM_CAP_TS2://TS_BB_ALI
		
			NIM_PRINTF("TS_BB_ALI mode: swpidf is %x swpidd is %x\n",swpidf,swpidd);
            // cr_124[1] = 0:  Ali_mode
            // cr_124[2] = 0:  header byte4: frmnum  byte5:frmnum
            //                   1: header byte4: modcod  byte5:frmnum
            // cr_124[3] = 0:disable ISSY Mode
			nim_reg_read(dev, C3501H_R24_PACKET_CFG, &data, 1);
			data &= 0xf5;
			data |= 0x04;
			nim_reg_write(dev, C3501H_R24_PACKET_CFG, &data, 1);
            
            // cr_118[17] = 0 :
			//                    1: base on bb header ACM/CCM bit.auto swtch packing's ts/gs work mode
			// cr_118[16] = 0: gs, 1:ts
			nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
			data &= 0xfc;
			nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);

			break;
		default:

			break;							
	}
	
	return SUCCESS;
}


/*****************************************************************************
* 
* INT32 nim_c3501h_cap_cfg(struct nim_device *dev,UINT32 dram_len)
* Description: Set data capture config
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*  Parameter2: UINT32 dram_len
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3501h_cap_cfg(struct nim_device *dev, UINT32 dram_len)
{
	//UINT8 *dram_base_t = NULL;
	UINT8 data =0 ,data_tmp = 0;
	UINT8 cmd = 0;
	//UINT8 cap_src = 0;

	NIM_PRINTF("Enter function nim_C3501h_cap_pkt!\n");

	//REG 14e
	//[7:4] 0:ADC_OUT 1:DCC_OUT 2:IQB_OUT  3: CCI_OUT 4:DC_OUT
	//        5:FLT_OUT 6:AGC2_OUT 7:TR_OUT 8:EQ_OUT
	//        9:PL_OUT 10:AGC3_OUT 11:DELAY_PLL_OUT 12:CR_OUT 
	//        13:EQ_DD_OUT    14:PLSN_OUT
	//        15: PKT_DMA 
	//[3:1]  000--DMA_BB_L3 001--DMA_BB_PURE_L3 010--DMA_BB_ALI
	//         011--TS-BB-L3  100--TS-BB-ALI  111--cap BIST data
	//bit 0   1:enable capture 0:disable capture
	nim_reg_read(dev, C3501H_R0C_SYS_REG + 2, &data, 1);
	data_tmp =data >> 4;	
	if(15 == data_tmp)
	{
		data_tmp =(data & 0x0e)>>1 ;

		if((data_tmp != 3) && (data_tmp != 4))//DMA Cap,can't support     gloria 2017.8.21
		{
		    NIM_PRINTF("xxxxx C3501H can not support DMA Capture,return!xxxx\n");
			return ERR_FAILED;
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
			nim_c3501h_tsbb_cap_set(dev,cmd,0x000a0135,0x000b0135);
		}

	}

	return SUCCESS;	
}



#ifdef ACM_RECORD
/*****************************************************************************
* 
* INT32 nim_c3501h_rec_stop(struct nim_device *dev)
*  When record finish, Upper software need to call this function for recover some config
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3501h_rec_stop(struct nim_device *dev)
{
    UINT8 data = 0;
	NIM_PRINTF("====== enter nim_rec_stop ====== \n");

	// enable the insertion of dummy TS packet
	nim_reg_read(dev,C3501H_R18_TSO_CFG, &data, 1);
	data |= 0x80; // [7] = 1;
    nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);

	// cr120 bit[3:2] = 0  open ISI ID filter
	nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1); 
	data = data & 0xf3;
	nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);

	// set cr1a bit[4] = 1:update bbheader info to internal logic with bch confirm
	nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
	data |= 0x10;
	nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);

	nim_reg_read(dev, C3501H_R24_PACKET_CFG, &data, 1);
	data &= 0xf5;
	data |= 0x04;
	nim_reg_write(dev, C3501H_R24_PACKET_CFG, &data, 1);
	
	// cr_118[17] = 0 :
	//					  1: base on bb header ACM/CCM bit.auto swtch packing's ts/gs work mode
	// cr_118[16] = 1:  1:ts
	nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
	data &= 0xfd;
	data |= 0x01;
	nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);

	// added by gloria for BBCAP BUG  2017.9.7
	nim_c3501h_reset_fsm(dev);
    nim_c3501h_start_fsm(dev);

	return SUCCESS;
}


/*****************************************************************************
* 
* INT32 nim_c3501h_tso_bbfrm_cap_mod(struct nim_device *dev)
*  Set capture mode for record function
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3501h_tso_bbfrm_cap_mod(struct nim_device *dev)
{
	UINT8 tso_bbfrm_cfg = 0;
	UINT8 data = 0;
    NIM_PRINTF("====== enter nim_tso_bbfrm_cap_mod ====== \n");
	
#ifdef ISID_FILTER_BYPASS	
	// cr120 bit[3:2] = 2  bypass ISI ID filter
	nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1); 
	// bit3 ->1
	data = data | 0x08;
	// bit2 ->0
	data = data & 0xfb;
	nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
#endif

    //TSO_SW_SEL : C3501H_R0C_SYS_REG + 2
    // TS_L3 : 0xf7
    // TS_ALI: 0xf9
    // C3501H can not support DMA any more

	tso_bbfrm_cfg = 0xf7;
	nim_reg_write(dev, C3501H_R0C_SYS_REG + 2, &tso_bbfrm_cfg, 1);
	
	nim_reg_read(dev, C3501H_R0C_SYS_REG + 2, &data, 1);			
	if((data & 0x01) == 0x01)			
	{				
		nim_c3501h_cap_cfg(dev,0x100000);		
		nim_reg_read(dev, C3501H_R0C_SYS_REG + 2, &data, 1);		
		data &=0xfe ;				
		nim_reg_write(dev, C3501H_R0C_SYS_REG + 2, &data, 1);		
	}
	return SUCCESS;
}


/*****************************************************************************
* 
* INT32 nim_c3501h_ts_gs_auto_cap_mod(struct nim_device *dev)
*  When record start, config the capture mode
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_ts_gs_auto_cap_mod(struct nim_device *dev)
{
	//UINT8 ts_gs_type = 0;
	//UINT8 acm_ccm_type = 0;
	//UINT8 sis_mis_type = 0;
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
	
	//struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	
	// Check lock status
	while(1)
	{
		nim_c3501h_get_lock(dev, &data);
		// lock
		if(1 == data)
			break;
		// unlock
		if(time_out >= 15)
		{
			ACM_DEBUG_PRINTF("[C3501H] In %s exit with TP unlock \n", __FUNCTION__);
			return ERR_FAILED; 
		}
		
		time_out++; 
		comm_sleep(5);
	}
	
	time_out = 0;

    nim_c3501h_reg_get_work_mode(dev, &work_mode);
	// DVBS2 || DSS ,then record
	if(1 == work_mode || 2 == work_mode)
	{
		while(1)
		{	 
			ret_get_bb_header = nim_c3501h_get_bb_header_info(dev);
			 
			if (SUCCESS == ret_get_bb_header)
			{
				crc_correct_cnt ++;
				 
				if (crc_correct_cnt == CRC_THRESHOLD)
				{
				    //ts_gs_type = priv->bb_header_info->stream_mode;
				    //acm_ccm_type = priv->bb_header_info->acm_ccm;
                    //sis_mis_type = priv->bb_header_info->stream_type;
				
		    	    // to set TSO_BBFrame Output
		    	    // gs stream or acm or mis
		    	    //if ((0 == ts_gs_type) || (1 == ts_gs_type) || (NIM_ACM == acm_ccm_type) || (NIM_STREAM_MULTIPLE == sis_mis_type))
                    if (1) // after cap c3501h.  then delete this code  gloria 2017.6.7
					{
					
#ifdef ISID_FILTER_BYPASS
					
						// cr120 bit[3:2] = 2  bypass ISI ID filter
						nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1); 
						// bit3 ->1
						data = data | 0x08;
						// bit2 ->0
						data = data & 0xfb;
						nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
#endif
						
						//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
						// inorder to get all bbframe, we need to record all the frames ,including the error bbframe,gloria
						// set cr120 bit[4] = 0:update bbheader info to internal logic with crc right only
						nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
						data &= 0xef;
						nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);


						// disable the insertion of dummy TS packet
	                    nim_reg_read(dev,C3501H_R18_TSO_CFG, &data, 1);
	                    data &= 0x7f; // [7] = 0;
                        nim_reg_write(dev, C3501H_R18_TSO_CFG, &data, 1);

		    			return nim_c3501h_tso_bbfrm_cap_mod(dev);
		    		}
		    		else
		    		{
		    			//default
		    			//return nim_c3501h_tso_bbfrm_cap_mod(dev);
		    		}
						 
					break;
				}
			}
			else
			{
			    // when get bbheader failed,use tso_bb to record stream data
				// cr120 bit[3:2] = 2  bypass ISI ID filter
#ifdef ISID_FILTER_BYPASS				
				nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1); 
				// bit3 ->1
				data = data | 0x08;
				// bit2 ->0
				data = data & 0xfb;
				nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
#endif
				
				//isi_bch_confirm : When BB crc ok, but bch error, bb_isi_flt_act is active.
				// inorder to get all bbframe, we need to record all the frames ,including the error bbframe,gloria
				// set cr120 bit[4] = 0:update bbheader info to internal logic with crc right only
				nim_reg_read(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
				data &= 0xef;
				nim_reg_write(dev, C3501H_R18_BB_CMD_CFG + 2, &data, 1);
				
				return nim_c3501h_tso_bbfrm_cap_mod(dev);

				break;
			}
		}          
	}

    return SUCCESS;
}
#endif


/*****************************************************************************
*  INT32 nim_c3501h_set_fc_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs)
* Description: Set top fsm freq search range, depend on nim_c3501h_set_search_range_limited
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_case: for channel or autoscan
*  Parameter3: UINT32 rs
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    UINT8 data;
    UINT32 temp;
	UINT32 sample_rate = 0;

	nim_c3501h_get_dsp_clk(dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);

    switch (s_case)
    {
        case NIM_OPTR_SOFT_SEARCH:
        	{
	            if (rs > 18000)
	                temp = 5; 
	            else if (rs > 15000)
	                temp = 4;
	            else if (rs > 10000)
	                temp = 3;
	            else if (rs > 5000)
	                temp = 2;
	            else
	                temp = 1;
                
                priv->t_param.freq_search_range = temp*1024;
				temp = (temp * 2048)/sample_rate;
	            data = temp & 0xff;
	            nim_reg_write(dev, C3501H_R0C_TOP_FSM_CFG_4, &data, 1);
	            data = (temp >> 8) & 0x3;
	            
	            if (rs > 10000)
	                data |= 0xa0;	// amy change for 138E 12354V43000 est 1756/37333
	            else if (rs > 5000)
	                data |= 0xc0;   	//amy change for 91.5E 3814/V/6666
	            else
	                data |= 0xb0;   	//amy change for 91.5E 3629/V/2200
	                
				nim_reg_write(dev, C3501H_R0C_TOP_FSM_CFG_4 + 1, &data, 1);
				priv->t_param.t_reg_setting_switch |= NIM_SWITCH_FC;
        	}
			break;
        case NIM_OPTR_CHL_CHANGE:
        	//if (priv->t_Param.t_reg_setting_switch & NIM_SWITCH_FC)
        	{
				// set sweep range
				//temp = (3 * 90 * 16+sample_rate/2) / sample_rate;
	            if (rs > 18000)
	                temp = 5; 
	            else if (rs > 15000)
	                temp = 4;
	            else if (rs > 4000)
	                temp = 3;
	            else
	                temp = 2;

                priv->t_param.freq_search_range = temp*1024;
				temp = (temp * 2048)/sample_rate;
				data = temp & 0xff;
				nim_reg_write(dev, C3501H_R0C_TOP_FSM_CFG_4, &data, 1);
				data = (temp >> 8) & 0x3;
				data |= 0xb0;
				nim_reg_write(dev, C3501H_R0C_TOP_FSM_CFG_4 + 1, &data, 1);
				priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_FC;
        	}
        	break;
        default:
            NIM_PRINTF(" CMD for nim_c3501h_set_FC_Search_Range ERROR!!!\r\n");
            break;
    }
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_rs_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs)
* Description: Set top fsm symbol rate search range, depend on nim_c3501h_set_search_range_limited
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_case: for channel or autoscan
*  Parameter3: UINT32 rs
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    UINT8 data = 0;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
                 
    nim_c3501h_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);

    switch (s_case)
    {
        case NIM_OPTR_SOFT_SEARCH:
            { 
                temp =(rs + 1000) / 2000;
    			if (temp < 2)
    				temp = 2;
    			else if (temp > 11)
    				temp = 11;

    			temp = (temp * 2048)/sample_rate;
				data = temp & 0xff;

    			nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3 + 2, &data, 1); ////CR64
    			data = (temp >> 8) & 0x3;
    			data |= 0xa0;		////RS_Search_Step=2
    			nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3 + 3, &data, 1);////CR65
    			priv->t_param.t_reg_setting_switch |= NIM_SWITCH_RS;
            }
            break;
        case NIM_OPTR_CHL_CHANGE:
            if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_RS)
            {
                temp = (3 * 2048)/sample_rate;
                data = temp & 0xff;
                nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3 + 2, &data, 1);
                data = (temp >> 8) & 0x3;
                data |= 0x30;
                nim_reg_write(dev, C3501H_R08_TOP_FSM_CFG_3 + 3, &data, 1);
                priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_RS;
            }
            break;
        default:
            NIM_PRINTF(" CMD for nim_c3501h_set_RS_Search_Range ERROR!!!\r\n");
            break;
    }
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_search_range_limited(struct nim_device *dev, UINT8 cfg)
* Description: Enable/disable search range function
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 cfg, 0: disable, 1:enable
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_search_range_limited(struct nim_device *dev, UINT8 cfg)
{
    UINT8 data = 0;
    
    nim_reg_read(dev, C3501H_R0C_TOP_FSM_CFG_4 + 2, &data, 1);
    data &= 0xfe; 
    data |= (cfg&0x01);
    nim_reg_write(dev, C3501H_R0C_TOP_FSM_CFG_4 + 2, &data, 1);

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_tr_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*  Parameter2: UINT32 sym
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tr_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym)
{
	UINT8  data = 0;
	//UINT32 value = 0;

	// yiping 11/08/2017
	//set TR_LD_TIME_BND=2 & TR_IRS_TRA=22
	data = ((2&0x07)<<5) | (22&0x1f);
	nim_reg_write(dev, C3501H_R60_TR_CFG_2 + 2, &data, 1);

	//set TR_IRS_ACQ=14 & TR_IRS_MID_COARSE=18
	data = ((14&0x07)<<5) | (18&0x1f);
	nim_reg_write(dev, C3501H_R60_TR_CFG_2, &data, 1);

	//set TR_IRS_MID_FINE=20 & TR_MF_OUT=1 & TR_IRS_ACQ=14
	data = ((20&0x1f)<<3) | ((1&0x01)<<2) | ((14>>3)&0x03);
	nim_reg_write(dev, C3501H_R60_TR_CFG_2 + 1, &data, 1);
	
	//set TR_PRS_MID_COARSE=7 & TR_PRS_ACQ=5
	data = 0x75;
	nim_reg_write(dev, C3501H_R5C_TR_CFG_1 + 2, &data, 1);
	
	//set TR_TIME_OUT_BND
	if (sym < 2500)
		data = 0x08;
	else if (sym < 3000)
		data = 0x0a;
	else
		data = 0x1f;
	nim_reg_write(dev, C3501H_R60_TR_CFG_2 + 3, &data, 1);
	
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_pl_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case
*  Parameter2: UINT32 sym
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_pl_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym)
{
	//UINT32  data = 0;
	UINT32  pl_lock_thre = 0;

    if (sym < C3501H_LOW_SYM_THR)
    {
       pl_lock_thre = 20; 
    }
    else
    {
        pl_lock_thre = 40;
    }
    
	nim_c3501h_sub_write_pl(dev, 8, pl_lock_thre);
    
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_cr_setting(struct nim_device *dev, UINT8 s_Case, UINT32 sym)
* Description: CR parameter setting for different case.
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 s_Case: channel change or autoscan
*  Parameter2: UINT32 sym: current TP's symbol rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_cr_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym)
{
	UINT8 data = 0;

	switch (s_case)
	{
    	case NIM_OPTR_SOFT_SEARCH:
    		break;
    	case NIM_OPTR_CHL_CHANGE:
    		break;
    	default:
    		NIM_PRINTF(" CMD for nim_c3501h_cr_setting ERROR!!!\r\n");
    		break;
	}

    // hyman set cr para, flowing code should be put into nim_c3501h_cr_setting
    nim_reg_read(dev, C3501H_RC4_CR_ADPT_CF_RPT + 2, &data, 1);
    if(sym < C3501H_LOW_SYM_THR)
    {
        data |= 0x2; // us low rs para CR_ADPT_LOW_SYMBOL_SEL = c4[17]
    }
    else
    {
        data &= 0xfd;  // use high rs para CR_ADPT_LOW_SYMBOL_SEL = c4[17]
    }
    nim_reg_write(dev, C3501H_RC4_CR_ADPT_CF_RPT + 2, &data, 1);

	return SUCCESS;
}

 
/*****************************************************************************
*  INT32 nim_c3501h_TR_CR_Setting(struct nim_device *dev, UINT8 s_Case)
*	Set max try times in fsm for different case.
*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT8 s_Case: channel change or autoscan
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tr_cr_setting(struct nim_device *dev, UINT8 s_case)
{
	 UINT32 data = 0;
	 struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
	 switch (s_case)
	 {
	 	case NIM_OPTR_SOFT_SEARCH:  // In blindscan cr maxtry = 1, tr maxtry = 1;
	 	//if (!(priv->t_Param.t_reg_setting_switch & NIM_SWITCH_TR_CR))
		 {
		    data = 0x11;
            nim_c3501h_sub_write_fsm(dev, 0, data);
	 	//priv->t_Param.t_reg_setting_switch |= NIM_SWITCH_TR_CR;
		 }
		 break;
	 	case NIM_OPTR_CHL_CHANGE:   // Default cr maxtry = 3, tr maxtry = 3;
		 if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_TR_CR)
		 {
			 // set reg to default value
             data = 0x31; //0x33;	// yiping 11/08/2017
             nim_c3501h_sub_write_fsm(dev, 0, data);
			 priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_TR_CR;
		 }
		 break;
	 }
	 return SUCCESS;
}


#ifdef FPGA_PLATFORM
// For debug used
INT32 get_isid_debug(struct nim_device *dev)
{
	int  i=0;
	UINT8 j=0;
	UINT8 data = 0;
	UINT8 isid_num=0;         // Isid mumber, 0 means it not a multistream TP
    UINT8 isid_read[256];   // Read  isid from hardware

	for(i=0; i<256; i++)
		isid_read[i] = 0;


	ACM_DEBUG_PRINTF("==== Get isid start===========\r\n");
	for (i = 0; i < 32; i++)
	{
		// write index
		nim_reg_read(dev, C3501H_R20_BB_ROLL_OFF_CFG + 3, &data, 1);
		data &= 0xe0;
		data |= i;
		nim_reg_write(dev, C3501H_R20_BB_ROLL_OFF_CFG + 3, &data, 1);

		// read
		nim_reg_read(dev, C3501H_R1C_BB_DEC_CFG + 3, &data, 1);
		ACM_DEBUG_PRINTF("isid_rpt[%d] = 0x%x\r\n", i, data);

		for (j = 0; j < 8; j++)
		{
			if (0x01 & (data >> j))
			{
				isid_read[isid_num] = ((i*8) + j);
				isid_num++;
			}
		}
	}

	ACM_DEBUG_PRINTF("Get isid finished, isid_num = %d\r\n", isid_num);
	for (i = 0; i < isid_num; i++)
		ACM_DEBUG_PRINTF("isid_read[%d] = 0x%x \r\n", i, isid_read[i]);

	return SUCCESS;
}
#endif



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////debug.c

#ifdef DEBUG_IN_TASK
/*****************************************************************************
*  INT32 nim_c3501h_debug_intask(struct nim_device *dev)
* Description: Debug function for signal monitor, lock status monitor, register value monitor
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_debug_intask(struct nim_device *dev)
{
#ifdef DEBUG_REGISTER   
    static UINT32 count = 0;
#endif
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	//NIM_PRINTF(" Enter function nim_c3501h_debug_intask \n");

#if 0    
#ifdef HW_ADPT_CR
	#ifdef HW_ADPT_CR_MONITOR
		nim_c3501h_cr_adaptive_monitor(dev);
	#endif    
	#ifdef HW_ADPT_NEW_CR_MONITOR
		nim_c3501h_cr_new_adaptive_monitor(dev);
		#ifdef NEW_CR_ADPT_SNR_EST_RPT
			nim_c3501h_get_snr_db(dev,&snr_db);
		#endif
	#endif
#else
	#ifdef SW_ADPT_CR
		#ifdef SW_SNR_RPT_ONLY
			nim_c3501h_cr_sw_snr_rpt(dev);
		#else
			nim_c3501h_cr_sw_adaptive(dev);
		#endif
	#endif
#endif
#endif

#ifdef C3501H_DEBUG_FLAG
    if (priv->debug_flag&0x01)
        return SUCCESS;

        NIM_PRINTF("\n\nNim_c3501h_debug_intask, Current system time: %d\n", osal_get_tick());
    
        // Loct status monitor
        nim_c3501h_mon_lock_status(dev);
    
        // Signal Monitor
        nim_c3501h_mon_signal(dev);
        
        // Read register
#ifdef DEBUG_REGISTER        
		count++;
		if (count >= 20)
		{
        	nim_c3501h_mon_reg(dev);
			count = 0;
		}
#endif	
#endif	

	return SUCCESS;
}

#endif


#ifdef C3501H_DEBUG_FLAG
/*****************************************************************************
* INT32 nim_c3501h_mon_signal(struct nim_device *dev)
*
*	This function will monitor signal status  for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_mon_signal(struct nim_device *dev)
{
    //UINT8 data = 0;
    INT16 snr = 0;
    INT8 agc = 0;
	UINT8 per_flag = 0, cr_phase_err = 0, pl_phase_err = 0;
    UINT16 status = 0;
	UINT32 per = 0, ber = 0;

	struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

	nim_c3501h_get_agc_dbm(dev , &agc);
	
    per = priv->channel_info->per;
    ber = priv->channel_info->ber;
    snr = priv->channel_info->snr;

#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	NIM_PRINTF("In %s, level = %d dBm, per = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
#else
    if (1 == priv->channel_info->work_mode)
	    NIM_PRINTF("In %s, level = %d dBm, fer = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
        //NIM_PRINTF("In %s, level = %d dBm, per = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
    else
	    NIM_PRINTF("In %s, level = %d dBm, per = %d, ber = %d, snr = %d (0.01)dB\n", __FUNCTION__, agc, per, ber, snr);
        
#endif
    
	// read cr phase error
    //cr_phase_err = priv->channel_info->phase_err;

	if((1 == per_flag) && (0 == per))
	{
		per_flag = 0 ; 	 
	}

	if((0 != per) && (1010 != per)) 
	{
	    nim_c3501h_get_lock_reg(dev, &status);
		NIM_PRINTF("---status: 0x%x\n",status);
		NIM_PRINTF("---ber: %d,------snr: %d, ----------cr phase_err: %d,pl phase_err: %d \n",ber,snr,cr_phase_err,pl_phase_err);
		NIM_PRINTF("---Per !=0 happpened, FFT start log! per = %d\n",per);
		per_flag = 1 ;  
	}

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3501h_mon_lock_status(struct nim_device *dev)
*
*	This function will monitor demod lock status for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_mon_lock_status(struct nim_device *dev)
{
    static UINT16 lock_status = 0;
    UINT16 lock_status_tmp = 0;
    UINT8 int_data = 0;

	//struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

    // Lock status monitor
    nim_c3501h_get_lock_reg(dev, &lock_status_tmp);
	
    if (lock_status_tmp != lock_status)
    {
        lock_status = lock_status_tmp;  
        NIM_PRINTF("&&&&&&&& lock status reg = 0x%x\n", lock_status_tmp);
    
        nim_c3501h_get_interrupt(dev, &int_data);   
        NIM_PRINTF("int_data = 0x%x\n", int_data);

        nim_c3501h_clear_interrupt(dev);               
    }  

    return SUCCESS;
}


#endif


#ifdef DEBUG_REGISTER
/*****************************************************************************
* INT32 nim_c3501h_mon_reg(struct nim_device *dev)
*
*	This function will monitor demod register for debug
*
* Arguments:
*	Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_mon_reg(struct nim_device *dev)
{
    UINT8 data = 0;
	UINT8 i = 0, j = 0;

	//struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;

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


/*****************************************************************************
* INT32 nim_c3501h_check_hw_reg(struct nim_device *dev)
* Description: I2C maybe has some problem cause that reg00 to 0x00, so we do some protection here to protect the reg00
* 
* Arguments:
*  Parameter1: struct nim_device *dev
*
*
* Return Value: INT32
*****************************************************************************/
/*INT32 nim_c3501h_check_hw_reg(struct nim_device *dev)
{
    UINT8 data = 0;

    nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
    if (g_fsm_cfg_bak != data)
    {
        ERR_PRINTF("Find demod reg error, reg:0x%x, bak_val:0x%x, restart!!!\n", data, g_fsm_cfg_bak);
        nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &g_fsm_cfg_bak, 1);
        nim_c3501h_reset_fsm(dev);
        nim_c3501h_start_fsm(dev);
    }

    return SUCCESS;
}*/


/*****************************************************************************
* INT32 nim_c3501h_force_cur_work_mode(struct nim_device *dev)
* Description: Force current work mode to hardware.
* 
* Arguments:
*  Parameter1: struct nim_device *dev
*
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_force_cur_work_mode(struct nim_device *dev)
{
    UINT8 work_mode = 3;

    nim_c3501h_reg_get_work_mode(dev, &work_mode);
    if (3 != work_mode)
    {
        nim_c3501h_set_work_mode(dev, work_mode);
        NIM_PRINTF("[%s, %d], force current work mode to %d\n", __FUNCTION__, __LINE__, work_mode);
    }

    return SUCCESS;
}


#ifdef IMPLUSE_NOISE_IMPROVED

/*****************************************************************************
* INT32 nim_c3501h_set_eq_dd_gain(struct nim_device *dev, UINT8 enable, UINT16 eq_dd_gain)
* Description: Set eqdd gain for impluse noise improvement
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 enable
*  Parameter3: UINT16 eq_dd_gain: valid only enable = 1
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_eq_dd_gain(struct nim_device *dev, UINT8 enable, UINT16 eq_dd_gain)
{
    UINT8 data = 0;
    
    enable &= 0x01;
    if (enable)
    {
        eq_dd_gain &= 0x1ff;
        data = eq_dd_gain & 0xff;
        nim_reg_write(dev, C3501H_RE0_EQ_DD_CFG_2, &data, 1);
    }

    nim_reg_read(dev, C3501H_RE0_EQ_DD_CFG_2 + 1, &data, 1);
    data &= 0xfc;
    data = ((enable<<1) + data + (eq_dd_gain>>8));
    nim_reg_write(dev, C3501H_RE0_EQ_DD_CFG_2 + 1, &data, 1);  

    return SUCCESS;
}


const UINT8 c3501h_map_beta_normal_adaptive_en[32] = 
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
    1,  //  2/3  			
    0,  //  3/4  			
    0,  //  3/4  			
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

const UINT8 c3501h_map_beta_short_adaptive_en[32] = 
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
    0,  //  3/4  			
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

const UINT16 c3501h_map_beta_normal_snr_thres[32][2] =                     
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
    {650,   750},  //  3/5 of 8PSK		
    {750,   950  },  //  2/3  	
    {5000,  10000},  //  3/4  			
    {5000,  10000},  //  3/4  			
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


/*****************************************************************************
* INT32 nim_c3501h_set_demap_llr_shift(struct nim_device *dev, UINT8 enable)
* Description: Set demap llr shift, means map beta/2
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 enable
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_demap_llr_shift(struct nim_device *dev, UINT8 enable)
{
    UINT8 data = 0;

    NIM_PRINTF("%s, %d, enable:%d\n", __FUNCTION__, __LINE__, enable);
    nim_reg_read(dev, C3501H_RE8_DEMAP_RPT + 2, &data, 1);
    data &= 0xfe;
    data |= (enable&0x01);
    nim_reg_write(dev, C3501H_RE8_DEMAP_RPT + 2, &data, 1);

    return SUCCESS;
}


/*****************************************************************************
*  void nim_c3501h_auto_adaptive(struct nim_device *dev)
* Set demap beta for impluse noise improvement, it only for 8psk 2/3 normal mode used now!
*
* Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: none 
*****************************************************************************/
INT32 nim_c3501h_auto_adaptive(struct nim_device *dev)
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
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
#ifdef FOR_SKEW_LNB_TEST_QPSK
	static UINT8 map_type_last = 0;
    UINT8 map_type = 0;
#endif

    auto_adaptive_state_current = priv->t_param.auto_adaptive_state;
    
    nim_c3501h_reg_get_work_mode(dev, &work_mode);
    if (1 == work_mode) // In DVB-S2 Mode
    {

#ifdef FOR_SKEW_LNB_TEST_QPSK
        nim_c3501h_reg_get_map_type(dev, &map_type);
        if (map_type_last != map_type)
        {
            if (2 == map_type)
            {
                nim_c3501h_set_roll_off(dev, 0, 0);
            }
            else
            {
                nim_c3501h_set_roll_off(dev, 1, 2);
            }
            map_type_last = map_type;
        }
#endif	

        nim_c3501h_reg_get_modcod(dev, &modcod);
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
                //NIM_PRINTF("snr_array[%d] = %d\n", i, snr_array[i]);
            }
            snr = (snr - snr_max - snr_min)/6;
            NIM_PRINTF("snr_avg = %d, snr_max = %d, snr_min = %d\n", snr, snr_max, snr_min);

            if (0 == (modcod&0x80))   // Not s2x
            {
	            frame_mode = (modcod & 0x02) >> 1;
	            if (0 == frame_mode)    // Only for normal frame now!
	            {
	                modcod = (modcod & 0x7c) >> 2;
	                if (c3501h_map_beta_normal_adaptive_en[modcod])
	                {
	                    switch (priv->t_param.auto_adaptive_state)
	                    {
	                        case 0: 
	                            if (snr > (c3501h_map_beta_normal_snr_thres[modcod][1] + SNR_MERGE))
	                                auto_adaptive_state_current = 2;
	                            else if (snr > (c3501h_map_beta_normal_snr_thres[modcod][0] + SNR_MERGE))
	                                auto_adaptive_state_current = 1; 
	                            break;

	                        case 1:
	                            if (snr > (c3501h_map_beta_normal_snr_thres[modcod][1] + SNR_MERGE))
	                                auto_adaptive_state_current = 2;
	                            else if (snr < (c3501h_map_beta_normal_snr_thres[modcod][0] - SNR_MERGE))
	                                auto_adaptive_state_current = 0;
	                            break;
             
	                        case 2:
	                            if (snr < (c3501h_map_beta_normal_snr_thres[modcod][0] - SNR_MERGE))
	                                auto_adaptive_state_current = 0;
	                            else if (snr < (c3501h_map_beta_normal_snr_thres[modcod][1] - SNR_MERGE))
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

	                        if (2 == auto_adaptive_state_current)
	                        {
	                            nim_c3501h_set_demap_llr_shift(dev, 1);
	                        }
	                        else
	                        {
	                            #if 0
	                            if (1 == auto_adaptive_state_current)
	                            {
	                                nim_c3501h_set_eq_dd_gain(dev, 1, 0x1ff);
	                            }
	                            else
	                            {
	                                nim_c3501h_set_eq_dd_gain(dev, 0, 0x100);
	                            }
	                            #endif
	                            nim_c3501h_set_demap_llr_shift(dev, 0);
	                        }
	                        //nim_c3501h_set_map_beta_extra(dev, modcod, frame_mode, auto_adaptive_state_current);
                        }
                    }	    
                } 
            }
        } 
    }
    else    // Unlock or DVB-S mode set map beta to default
    {   if (0 != priv->t_param.auto_adaptive_state)
        {
            priv->t_param.auto_adaptive_cnt = 0;
            priv->t_param.auto_adaptive_state = 0;
            //nim_c3501h_set_eq_dd_gain(dev, 0, 0x100);
            
            for (i = 0; i < 32; i++) // For normal, set map beta to default
            {
                if (c3501h_map_beta_normal_adaptive_en[i])
                {
                    nim_c3501h_set_demap_llr_shift(dev, 0);
                    //nim_c3501h_set_map_beta_extra(dev, i,  0, priv->t_param.auto_adaptive_state);     
                }
            }
            
            for (i = 0; i < 32; i++) // For short, set map beta to default
            {
                if (c3501h_map_beta_short_adaptive_en[i])
                {
                    nim_c3501h_set_demap_llr_shift(dev, 0);
                    //nim_c3501h_set_map_beta_extra(dev, i,  1, priv->t_param.auto_adaptive_state);     
                }
            }
        }
    }
    
    return SUCCESS;
}

#endif


#ifdef C3501H_DEBUG_FLAG

/*****************************************************************************
* INT32 nim_c3501h_set_debug_flag(struct nim_device *dev, UINT32 freq, UINT32 sym)
* Description: Set debug flag for debug
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 sym
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_debug_flag(struct nim_device *dev, UINT32 freq, UINT32 sym)
{ 
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;
    
    if ((1000 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x01;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1000 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xfe;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1100 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x02;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1100 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xfd;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1200 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x04;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1200 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xfb;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1300 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x08;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1300 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xf7;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1400 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x10;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1400 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xef;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1500 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x20;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1500 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xdf;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1600 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x40;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1600 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0xbf;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1700 == freq) && (34567 == sym))
    {
        priv->debug_flag |= 0x80;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    else if ((1700 == freq) && (45678 == sym))
    {
        priv->debug_flag &= 0x7f;
        NIM_PRINTF("Set TP: %d/H/%d, set priv->debug_flag = 0x%x\n", freq, sym, priv->debug_flag);
    }
    
    return SUCCESS;
}
#endif


/*****************************************************************************
* INT32 nim_c3501h_tuner_control(struct nim_device *dev, UINT32 freq, UINT32 sym, INT32 *freq_err)
* Description: Set tuner: 1,open i2c through 2,set tuner 3,close i2c through
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 sym
*  Parameter4: INT32 *freq_err: Only for m3031 tuner
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_tuner_control(struct nim_device *dev, UINT32 freq, UINT32 sym, INT32 *freq_err)
{
    struct nim_c3501h_private *priv = (struct nim_c3501h_private *) dev->priv;    

    *freq_err = 0;
    
    if (nim_c3501h_i2c_through_open(dev))
    {
       NIM_PRINTF("line=%d,in %s, open I2C error!\n",__LINE__, __FUNCTION__);
       return C3501H_ERR_I2C_NO_ACK;
    }

    if(IS_M3031 == priv->tuner_type)
    {
       if (priv->nim_tuner_command)
       {
           priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_SET_C3031B_FREQ_ERR, freq_err);
       }
       
       NIM_MUTEX_ENTER(priv);
       priv->nim_tuner_control(priv->tuner_index, freq, sym);
       NIM_MUTEX_LEAVE(priv);
        
       if (priv->nim_tuner_command)
       {
           priv->nim_tuner_command(priv->tuner_index, NIM_TUNER_GET_C3031B_FREQ_ERR, freq_err);
       }
    }
    else
    {
        NIM_MUTEX_ENTER(priv);
        priv->nim_tuner_control(priv->tuner_index, freq, sym);
        NIM_MUTEX_LEAVE(priv);
    }

    if (nim_c3501h_i2c_through_close(dev))
    {
       NIM_PRINTF("line=%d,in %s, close I2C error!\n",__LINE__, __FUNCTION__);
       return C3501H_ERR_I2C_NO_ACK;
    }

    // Wait agc stable 
    comm_delay(500);

    return SUCCESS;
}


#if 0
#define BEQ_ACQ_ERR_SHIFT   3
#define BEQ_MID_ERR_SHIFT   3
#define BEQ_TRA_ERR_SHIFT   3
#define BEQ_ACQ_CENTR_STEP  3
#define BEQ_MID_CENTR_STEP  4
#define BEQ_TRA_CENTR_STEP  5
#define BEQ_ACQ_OTHER_STEP  3
#define BEQ_MID_OTHER_STEP  4
#define BEQ_TRA_OTHER_STEP  5
#define BEQ_SYMBOL_NUM      4
#else
#define BEQ_ACQ_ERR_SHIFT   2
#define BEQ_MID_ERR_SHIFT   2
#define BEQ_TRA_ERR_SHIFT   2
#define BEQ_ACQ_CENTR_STEP  2
#define BEQ_MID_CENTR_STEP  2
#define BEQ_TRA_CENTR_STEP  2
#define BEQ_ACQ_OTHER_STEP  2
#define BEQ_MID_OTHER_STEP  2
#define BEQ_TRA_OTHER_STEP  2
#define BEQ_SYMBOL_NUM      2
#endif


/*****************************************************************************
* INT32 nim_c3501h_set_beq(struct nim_device *dev)
* Description: Set beq parameters
* 
*
* Arguments:

*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_beq(struct nim_device *dev)
{ 
    INT32 addr = 0;
    INT32 value = 0;

    NIM_PRINTF("%s\n", __FUNCTION__);

    addr = 3;
    value = (BEQ_ACQ_ERR_SHIFT&0x07) + ((BEQ_MID_ERR_SHIFT&0x07)<<4);
    nim_c3501h_sub_write_beq(dev, addr, value);

    addr = 4;
    value = (BEQ_TRA_ERR_SHIFT&0x07) + ((BEQ_ACQ_CENTR_STEP&0x07)<<4);
    nim_c3501h_sub_write_beq(dev, addr, value);

    addr = 5;
    value = (BEQ_MID_CENTR_STEP&0x07) + ((BEQ_TRA_CENTR_STEP&0x07)<<4);
    nim_c3501h_sub_write_beq(dev, addr, value);    
    
    addr = 6;
    value = (BEQ_ACQ_OTHER_STEP&0x07) + ((BEQ_MID_OTHER_STEP&0x07)<<4);
    nim_c3501h_sub_write_beq(dev, addr, value);    

    addr = 7;
    value = (BEQ_TRA_OTHER_STEP&0x07) + ((BEQ_SYMBOL_NUM&0x1f)<<3);
    nim_c3501h_sub_write_beq(dev, addr, value);   

    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_c3501h_set_roll_off(struct nim_device *dev, UINT8 ro_en, UINT8 ro_val)
* Description: Set roll off parameters
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 ro_en, 0: roll off only use ACQ_ROLL_OFF,
	 1: roll off firstly use ACQ_ROLL_OFF, then will be updated when bbframe decode the final roll off
*  Parameter3: UINT8 ro_val, ACQ_ROLL_OFF
*
* Return Value: INT32: The result of reading successfully or not
*****************************************************************************/
INT32 nim_c3501h_set_roll_off(struct nim_device *dev, UINT8 ro_en, UINT8 ro_val)
{ 
	UINT8 data = 0;
	
	nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1 + 1, &data, 1);
	data &= 0x0f;
	data |= ((ro_en&0x01) << 7) + ((ro_val&0x07) << 4);
	nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1 + 1, &data, 1);
	NIM_PRINTF("Set roll off ro_en:%d, ro_val:%d\n", ro_en, ro_val);
	
	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3501h_tsn_enable(struct nim_device *dev, BOOL tsn_enable) 
*Description: Set tsn filter work mode and tsn value to hardware
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: bool tsn_enable, TRUE:enbale tsn, FALSE:disable tsn
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_tsn_enable(struct nim_device *dev, BOOL tsn_enable) 
{	
	UINT8 data;
	
    TSN_PRINTF("[%s %d]tsn_enable = %d\n", __FUNCTION__, __LINE__, tsn_enable);    
    nim_reg_read(dev, C3501H_RF0_DEMAP_FLT_CFG, &data, 1);
	data &= 0xfe;

	if (tsn_enable)
	{
    	data |= 0x01;
	}
	nim_reg_write(dev, C3501H_RF0_DEMAP_FLT_CFG, &data, 1);    
    
	return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_get_tsn(struct nim_device *dev, struct nim_dvbs_tsn *p_tsn)
*  Get TSN for wideband mode, must make sure TP is locked && dvb-s2x mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_dvbs_tsn *p_tsn
*
* Return Value: UINT8
*****************************************************************************/ 
INT32 nim_c3501h_get_tsn(struct nim_device *dev, struct nim_dvbs_tsn *p_tsn)
{
    struct nim_c3501h_private *priv = NULL;
	UINT8 data = 0;
    UINT16 time_out = 0;
    UINT8 cnt = 0;
    UINT8 i = 0, j = 0;

    TSN_PRINTF("[%s %d]Enter\n", __FUNCTION__, __LINE__);
    
	if(NULL == dev)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n", __FUNCTION__);
        return ERR_NO_DEV;
	}

	priv = (struct nim_c3501h_private *)dev->priv;
	if(NULL == priv)
	{
        ERR_PRINTF("[%s %d]NULL == priv\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}

	comm_memset(p_tsn->tsn_read, 0, sizeof(p_tsn->tsn_read));	
    p_tsn->tsn_num = 0;
    time_out = p_tsn->tsn_read_timeout;

	if (WIDEBAND != nim_c3501h_get_wideband_mode(dev))
	{
		TSN_PRINTF("[%s %d]it is not wideband, need not get tsn\n", __FUNCTION__, __LINE__);
        return ERR_FAILURE;
	}
	
    if(time_out <= 100)
        time_out = 100;
    else if(time_out >= 5000)
        time_out = 5000;
 	
	while(1)// Check lock status
	{
		if (priv->ul_status.c3501h_chanscan_stop_flag || priv->ul_status.c3501h_autoscan_stop_flag)
		{
			TSN_PRINTF("[%s %d]priv->ul_status.c3501h_chanscan_stop_flag=%d,\r\n\
				priv->ul_status.c3501h_autoscan_stop_flag=%d\r\n", 
				__FUNCTION__, __LINE__, priv->ul_status.c3501h_chanscan_stop_flag,
				priv->ul_status.c3501h_autoscan_stop_flag);	
			return ERR_FAILED;
		}
		
		nim_c3501h_get_lock(dev, &data);
		
		if(1 == data)// lock
			break;
		
		if(cnt >= time_out)// unlock
		{
			TSN_PRINTF("In %s exit with TP unlock \r\n", __FUNCTION__);
			return ERR_FAILED; 
		}

		cnt++; 
		comm_sleep(1);
	}

	// clear tsn mon buffer to zero.
	nim_reg_read(dev, C3501H_REC_DEMAP_FLT_CFG + 3, &data, 1);
	data |= 0x40;
	nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG + 3, &data, 1);

	// wait capture
	comm_sleep(time_out);

    // Begin get tsn
	for (i = 0; i < 32; i++)
	{
        nim_c3501h_sub_read_demap_mon_tsn(dev, i, &data);
		TSN_PRINTF("tsn_read[%d] = 0x%x\r\n", i, data);

		for (j = 0; j < 8; j++)
		{
			if (0x01 & (data >> j))
			{
				p_tsn->tsn_read[p_tsn->tsn_num] = ((i*8) + j);
				p_tsn->tsn_num++;
			}
		}
	}

	TSN_PRINTF("Get tsn finished, tsn_num = %d\r\n", p_tsn->tsn_num);
	for (i = 0; i < p_tsn->tsn_num; i++)
	{
		TSN_PRINTF("tsn_read[%d] = 0x%x \r\n", i, p_tsn->tsn_read[i]);
	}
	
	if (priv->tsn != p_tsn)
	{
		comm_memcpy(priv->tsn, p_tsn, sizeof(struct nim_dvbs_tsn));
	}

	TSN_PRINTF("[%s %d]return!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3501h_set_tsn (struct nim_device *dev, struct nim_dvbs_tsn *p_tsn) 
*Description: Set tsn filter work mode and tsn value to hardware
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_dvbs_tsn *p_tsn
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3501h_set_tsn(struct nim_device *dev, struct nim_dvbs_tsn *p_tsn) 
{
	UINT8 data = 0;
	UINT8 tsn_enable = 0;
	UINT8 tsn_id   = 0;
    UINT8 addr = 0;

	tsn_enable = p_tsn->tsn_enable;
	tsn_id     = p_tsn->tsn_write;

    TSN_PRINTF("[%s %d]tsn_write = 0x%x, tsn_enable = %d\n", __FUNCTION__, __LINE__, tsn_id, tsn_enable);
    // Write tsn enable to hw
    nim_reg_read(dev, C3501H_RF0_DEMAP_FLT_CFG, &data, 1);
	data &= 0xfe;
    data |= (tsn_enable&0x01);
	nim_reg_write(dev, C3501H_RF0_DEMAP_FLT_CFG, &data, 1);    
    
    if (tsn_enable)
    {
        // Clear tsn flt buffer to zero.
        nim_reg_read(dev, C3501H_REC_DEMAP_FLT_CFG + 3, &data, 1);
    	data |= 0x80;
    	nim_reg_write(dev, C3501H_REC_DEMAP_FLT_CFG + 3, &data, 1);

        addr = tsn_id/8;
        data = (1 << (tsn_id - ((tsn_id/8)*8)));
        TSN_PRINTF("addr = 0x%x, data = 0x%x\n", addr, data);
        nim_c3501h_sub_write_demap_flt_tsn(dev, addr, data);
    }

	return SUCCESS;
}

/************************************************************************************
 *  dvbs2x_wideband_mode_type_t nim_c3501h_get_wideband_mode(struct nim_device *dev)
 * Description: Get current TP's work mode
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	
 *
 * Return Value: dvbs2x_wideband_mode_type_t
 ************************************************************************************/
dvbs2x_wideband_mode_type_t nim_c3501h_get_wideband_mode(struct nim_device *dev)
{
	UINT8 data = 0;
	struct nim_c3501h_private *priv = NULL;
	
	if(NULL == dev)
	{
        TSN_PRINTF("Exit with ERR_NO_DEV in %s \r\n", __FUNCTION__);
        return ERR_NO_DEV;
	}

	priv = (struct nim_c3501h_private *)dev->priv;
	if(NULL == priv)
	{
        TSN_PRINTF("[%s %d]NULL == priv\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}
	
    data = 0x40;
    nim_reg_write(dev, C3501H_R1C_MODU_INFO_RPT_1 + 3, &data, 1);  
	
	nim_reg_read(dev, C3501H_R1C_MODU_INFO_RPT_1 + 2, &data, 1);
	TSN_PRINTF("[%s %d]reg(C3501H_R1C_MODU_INFO_RPT_1 + 2)=0x%x\n", __FUNCTION__, __LINE__, data);

	osal_mutex_lock(priv->tsn->tsn_mutex, OSAL_WAIT_FOREVER_TIME);
	if (0x04 == (data & 0x04))//bit2(RPT_WIDEBAND_MODE)->1:wideband mode
	{
		priv->tsn->is_wideband = 1;
		TSN_PRINTF("[%s %d]it is wideband mode!\n", __FUNCTION__, __LINE__);
		osal_mutex_unlock(priv->tsn->tsn_mutex);
		return WIDEBAND;
	}
	else
	{
		priv->tsn->is_wideband = 0;
		TSN_PRINTF("[%s %d]it is not wideband, change to auto mode\n", __FUNCTION__, __LINE__);
		nim_reg_read(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);
		data |= 0xC0;  
	    nim_reg_write(dev, C3501H_R00_TOP_FSM_CFG_1, &data, 1);

		osal_mutex_unlock(priv->tsn->tsn_mutex);
		return NOT_WIDEBAND;
	}
	
}

/*****************************************************************************
*  INT32 nim_c3501h_get_symbol(struct nim_device *dev, struct nim_get_symbol *p_symbol)
*  Get symbol for constellation, for 3501h we cap from eq_dd not cr module
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_get_symbol *p_symbol
*
* Return Value: INT32
*****************************************************************************/ 
INT32 nim_c3501h_get_symbol(struct nim_device *dev, struct nim_get_symbol *p_symbol)
{
    struct nim_c3501h_private *priv = NULL;
	UINT8 data = 0;
    UINT32 i = 0;
    UINT8 data_i;
    UINT8 data_q;
    UINT16 lock_state = 0;

	if(NULL == dev)
	{
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \r\n", __FUNCTION__);
        return ERR_NO_DEV;
	}

	priv = (struct nim_c3501h_private *)dev->priv;
	if(NULL == priv)
	{
        ERR_PRINTF("[%s %d]NULL == priv\r\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
	}

    // Get EQ_DD Lock status
    nim_c3501h_get_lock_reg(dev, &lock_state);
    if (0x800 != (0x800&lock_state))
    {
    	ERR_PRINTF("[%s %d]error, lock_state=0x%x\n", __FUNCTION__, __LINE__, lock_state);
        return ERR_FAILED;
    }
	
    // Trigger EQ_DD output pulse & Config test out select
    for (i = 0; i < p_symbol->point_length; i++)
    {
        data = 0xc8;    // 0x48: EQ_DD output
        nim_reg_write(dev, C3501H_RF8_TEST_MUX_OUT + 2, &data, 1);

        // Get EQ_DD output
        nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT, &data_i, 1);
        nim_reg_read(dev, C3501H_RF8_TEST_MUX_OUT + 1, &data_q, 1);

        p_symbol->p_cr_out_i[i] = data_i;
        p_symbol->p_cr_out_q[i] = data_q;
    }
	return SUCCESS;
}

void nim_c3501h_gpio_output(struct nim_device *dev, UINT8 pin_num, UINT8 value)
{
	UINT8 data = 0;

	nim_reg_read(dev, 0x1ec + 3, &data, 1);
	data = data | (1 << pin_num);
	NIM_PRINTF("[%s %d]enable gpio pinmux, data=0x%02x\n", __FUNCTION__, __LINE__, (int)data);
	nim_reg_write(dev, 0x1ec + 3, &data, 1);//enable gpio pinmux

	nim_reg_read(dev, 0x1ec + 2, &data, 1);
	data = (data & (~(1 << pin_num))) | (1 << (4 + pin_num)) | (value << pin_num);
	NIM_PRINTF("[%s %d]output, data=0x%02x\n", __FUNCTION__, __LINE__, (int)data);
	
	nim_reg_write(dev, 0x1ec + 2, &data, 1);//enable output and output value
}

void nim_c3501h_enable_gpio_input(struct nim_device *dev, UINT8 pin_num)
{
	UINT8 data = 0;

	nim_reg_read(dev, 0x1ec + 3, &data, 1);
	data = data | (1 << pin_num);
	NIM_PRINTF("[%s %d]enable gpio pinmux, data=0x%02x\n", __FUNCTION__, __LINE__, (int)data);
	nim_reg_write(dev, 0x1ec + 3, &data, 1);//enable gpio pinmux

	nim_reg_read(dev, 0x1ec + 2, &data, 1);
	data = data & (~(1 << (4 + pin_num)));
	nim_reg_write(dev, 0x1ec + 2, &data, 1);//enable input
}

void nim_c3501h_get_gpio_input(struct nim_device *dev, UINT8 pin_num, UINT8 *value)
{
	UINT8 data = 0;

	nim_reg_read(dev, 0x1ec + 1, &data, 1);

	*value = data >> (4 + pin_num);
}


