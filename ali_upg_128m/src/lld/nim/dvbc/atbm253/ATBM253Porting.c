/********************************************************************************************************
File:ATBM253Porting.c
Description:
    ATBM253 Tuner porting APIs which need to be implemented by user.

*********************************************************************************************************/
#include <stdio.h>
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>
#include <osal/osal.h>
#include <hld/nim/nim_tuner.h>
#include "ATBM253Porting.h"   

#if (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_USER_SPACE)
#include <pthread.h>
static pthread_mutex_t ATBM253Mutex = PTHREAD_MUTEX_INITIALIZER;
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_KERNEL_SPACE)
#include <linux/mutex.h>
static struct mutex ATBM253Mutex;
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_WINDOWS)
static HANDLE ATBM253Mutex;
#else
static OSAL_ID ATBM253Mutex= OSAL_INVALID_ID; 
#endif
extern struct QAM_TUNER_CONFIG_EXT * atbm253_tuner_dev_id[ ];

static OSAL_ID atbm253_i2c_mutex = OSAL_INVALID_ID; 
#define ATBM_I2C_MUTEX_LOCK()			osal_mutex_lock(atbm253_i2c_mutex, OSAL_WAIT_FOREVER_TIME)
#define ATBM_I2C_MUTEX_UNLOCK()		osal_mutex_unlock(atbm253_i2c_mutex)
#define ATBM_I2C_WRITE_MAX			14
//create mutex lock using for I2C
//高拓的tuner的I2C write/read需先写寄存器register的16位地址再读/写数据
//期间可能因为线程的调度而打断数据的读写,导致I2C通讯错误,故添加互斥锁

/********************************************************************
* Function: atbm253_mutex_create
* Description: protect I2C data. User need to implement this function addcording to system platform.
*			Detail of ATBM253 I2C format is described in software guide document.
* Input: 	
* Output: N/A
* Retrun: 
********************************************************************/
INT32 atbm253_mutex_create(void)
{
	if(atbm253_i2c_mutex == OSAL_INVALID_ID)
	{
		atbm253_i2c_mutex = osal_mutex_create();
		
		if(OSAL_INVALID_ID == atbm253_i2c_mutex)
		{
			ATBM253Print("%s()----TIPS:create mutex for I2C failed!\n", __FUNCTION__);
			return -1;
		}
		return 1;
	}
	return -1;
}

/********************************************************************
* Function: ATBM253I2CRead
* Description: Read I2C data. User need to implement this function according to system platform.
*            Detail of ATBM253 I2C format is described in software guide document.
* Input:     pI2CAddr -- I2C slave address and user param
*        BaseReg -- Base register address
*        OffReg -- Offset register address
*        pData -- Pointer to register value
*        Length - Buffer length of 'pData' in byte
* Output: N/A
* Retrun: ATBM253_ERROR_NO_ERROR -- no error; others refer to ATBM253_ERROR_e
********************************************************************/
ATBM253_ERROR_e ATBM253I2CRead(ATBM253I2CAddr_t *pI2CAddr,ATBM_U8 BaseReg,ATBM_U8 OffReg,ATBM_U8 *pData, ATBM_U32 Length)
{
	INT32 result = 0;
	UINT8 addr[2] = {0,};
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;  
	tuner_dev_ptr = atbm253_tuner_dev_id[0];
	ATBM_I2C_MUTEX_LOCK();
	//#1.读寄存器之前,先写base_reg&Off_Rge
	UINT8 u8_add = tuner_dev_ptr->c_tuner_base_addr ;

	//UINT8 u8_add = pI2CAddr->I2CSlaveAddr;
	addr[0] = BaseReg ;
	addr[1] = OffReg; 
	result = i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, addr, 2);
	if (result != SUCCESS)
	{
		libc_printf("%s(%d) %02x-%02x-%02x-%02d:",__FUNCTION__,tuner_dev_ptr->i2c_type_id,u8_add,BaseReg,OffReg,tuner_dev_ptr->i2c_type_id);
		libc_printf("---i2c write address[%x] failed!\n",u8_add);
		ATBM_I2C_MUTEX_UNLOCK();
		return ATBM253_ERROR_I2C_FAILED;
	}	

//	 osal_task_sleep(10);
	
	//#2.读寄存器中的值
	result = 0;
	u8_add = tuner_dev_ptr->c_tuner_base_addr | 0x01;
	//u8_add = pI2CAddr->I2CSlaveAddr | 0x01;
	result = i2c_read(tuner_dev_ptr->i2c_type_id, u8_add, pData, Length);
	if (result == SUCCESS)
	{
		//libc_printf("%s(%d) %02x-%02x-%02x-%02d:",__FUNCTION__,tuner_dev_ptr->i2c_type_id,u8_add,BaseReg,OffReg,tuner_dev_ptr->i2c_type_id);
		//libc_printf("[%x]\n",pData[0]);
		//ATBM253Print("%s()----i2c read data successful!\n",__FUNCTION__);
		osal_task_sleep(10);
		ATBM_I2C_MUTEX_UNLOCK();
		return ATBM253_NO_ERROR;
	}
	else
	{
		libc_printf("%s(%d) %02x-%02x-%02x-%02d:",__FUNCTION__,tuner_dev_ptr->i2c_type_id,u8_add,BaseReg,OffReg,tuner_dev_ptr->i2c_type_id);
		libc_printf("----i2c read[%x] data failed!\n",u8_add);
		ATBM_I2C_MUTEX_UNLOCK();
		return ATBM253_ERROR_I2C_FAILED;
	}

	return ATBM253_NO_ERROR;
}

/********************************************************************
* Function: ATBM253I2CWrite
* Description: Write I2C data. User need to implement this function according to system platform.
*            Detail of ATBM253 I2C format is described in software guide document.
* Input:     pI2CAddr -- I2C slave address and user param
*        BaseReg -- Base register address
*        OffReg -- Offset register address
*        pData -- Register value
*        Length - Buffer length of 'pData' in byte
* Output: N/A
* Retrun: ATBM253_ERROR_NO_ERROR -- no error; others refer to ATBM253_ERROR_e
********************************************************************/
ATBM253_ERROR_e ATBM253I2CWrite(ATBM253I2CAddr_t *pI2CAddr,ATBM_U8 BaseReg,ATBM_U8 OffReg,ATBM_U8 *pData, ATBM_U32 Length)
{
	INT32 result = 0;
	UINT8 write_data[ATBM_I2C_WRITE_MAX+2] ={0,};
	INT32 RemainLen=0, BurstNum=0;
	INT32 i,j;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	tuner_dev_ptr = atbm253_tuner_dev_id[0];
	
	ATBM_I2C_MUTEX_LOCK();
	UINT8 u8_add = tuner_dev_ptr->c_tuner_base_addr;
	//UINT8 u8_add = pI2CAddr->I2CSlaveAddr;
	//#1 将BaseReg&OffReg作为数据和pData[]中的数据一起发送
	//ATBM253Print("%s() %02x  %02x  %02x \n",__FUNCTION__,u8_add,BaseReg,OffReg);
	
	write_data[0] = BaseReg;
	write_data[1] = OffReg ;

	RemainLen = (Length%ATBM_I2C_WRITE_MAX);
	if(RemainLen)
	{
		BurstNum = (Length/ATBM_I2C_WRITE_MAX);
	}
	else
	{
		BurstNum = ((Length/ATBM_I2C_WRITE_MAX)-1);
		RemainLen = ATBM_I2C_WRITE_MAX;
	}

	for(i=0; i<BurstNum; i++)
	{
		for(j=0; j<ATBM_I2C_WRITE_MAX; j++)
		{
			write_data[j+2] = pData[i*ATBM_I2C_WRITE_MAX+j];
		}
		write_data[1] = (OffReg+ (i*ATBM_I2C_WRITE_MAX));  //地址进行偏移
		result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, write_data,ATBM_I2C_WRITE_MAX+2);
		if(SUCCESS != result)
		{
			libc_printf("%s(%d) %02x-%02x-%02x \n",__FUNCTION__,tuner_dev_ptr->i2c_type_id,u8_add,BaseReg,OffReg);
			libc_printf("%s()----TIPS:i2c write data failure,length:%d,i/BurstNum:(%d/%d) \n",__FUNCTION__,ATBM_I2C_WRITE_MAX+2,i,BurstNum);
			ATBM_I2C_MUTEX_UNLOCK();
			return ATBM253_ERROR_I2C_FAILED;
		}
			
	}

	for(i=0; i<RemainLen; i++)
	{
		write_data[i+2] = pData[BurstNum*ATBM_I2C_WRITE_MAX+i];
		
	}
	write_data[1] =(OffReg+ (BurstNum*ATBM_I2C_WRITE_MAX));  //地址进行偏移
	result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, write_data,RemainLen+2);
	if(SUCCESS != result)
	{
		 osal_task_sleep(10);
		libc_printf("%s(%d) %02x-%02x-%02x \n",__FUNCTION__,tuner_dev_ptr->i2c_type_id,u8_add,BaseReg,OffReg);
		libc_printf("%s()----TIPS:i2c write data failure,length:%d,RemainLen:%d\n",__FUNCTION__,RemainLen+2,RemainLen);
		ATBM_I2C_MUTEX_UNLOCK();
		return ATBM253_ERROR_I2C_FAILED;
	}
	else
	{
		//ATBM253Print("%s()----TIPS:i2c write data successful,length=%d.\n",__FUNCTION__,Length+2);
		//osal_task_sleep(10);
		ATBM_I2C_MUTEX_UNLOCK();
		return ATBM253_NO_ERROR;
	}
			
	return ATBM253_NO_ERROR;
}

/********************************************************************
* Function: ATBM253Delay
* Description: Delay. User need to implement this function according to system platform.
*
* Input:     TimeoutUs -- timeout in us to delay 
*
* Output: N/A
* Return: N/A
********************************************************************/
void ATBM253Delay(ATBM_U32 TimeoutUs)
{
	osal_delay(TimeoutUs);			
    
}
/********************************************************************
* Function: ATBM253MutexInit
* Description: Initialize one mutex for SDK. 
* User need to implement this function according to system platform.
*
* Input:     N/A
*
* Output: N/A
* Retrun: ATBM253_ERROR_NO_ERROR -- no error; others refer to ATBM253_ERROR_e
********************************************************************/
ATBM253_ERROR_e ATBM253MutexInit(void)
{
#if (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_USER_SPACE)
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_KERNEL_SPACE)
	mutex_init(&ATBM253Mutex);
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_WINDOWS)
	ATBM253Mutex  = CreateSemaphore(ATBM_NULL,1,1,ATBM_NULL);
#else
	ATBM253Mutex = osal_mutex_create();

	if(OSAL_INVALID_ID == ATBM253Mutex)
	{
		ATBM253Print("%s()----TIPS:create mutex failed!\n", __FUNCTION__);
		return -1;
	}
#endif
    return ATBM253_NO_ERROR;
}
/********************************************************************
* Function: ATBM253MutexLock
* Description: Lock the mutex of SDK. 
* User need to implement this function according to system platform.
*
* Input:    N/A
*
* Output: N/A
* Retrun: ATBM253_ERROR_NO_ERROR -- no error; others refer to ATBM253_ERROR_e
********************************************************************/
ATBM253_ERROR_e ATBM253MutexLock(void)
{
#if (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_USER_SPACE)
	int ret = 0;
	ret = pthread_mutex_lock(&ATBM253Mutex);
	if(0 != ret)
	{
	    return ATBM253_ERROR_UNKNOWN;
	}
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_KERNEL_SPACE)
	mutex_lock(&ATBM253Mutex);
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_WINDOWS)
	WaitForSingleObject(ATBM253Mutex,INFINITE);
#else
	/*To realize mutex according to system platform.*/
	osal_mutex_lock(ATBM253Mutex, OSAL_WAIT_FOREVER_TIME);
#endif
    	return ATBM253_NO_ERROR;
}

/********************************************************************
* Function: ATBM253MutexUnLock
* Description: Unlock the mutex of SDK. 
* User need to implement this function according to system platform.
*
* Input:    N/A
*
* Output: N/A
* Retrun: ATBM253_ERROR_NO_ERROR -- no error; others refer to ATBM253_ERROR_e
********************************************************************/
ATBM253_ERROR_e ATBM253MutexUnLock(void)
{
#if (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_USER_SPACE)
	int ret = 0;
	ret = pthread_mutex_unlock(&ATBM253Mutex);
	if(0 != ret)
	{
		return ATBM253_ERROR_UNKNOWN;
	}
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_LINUX_KERNEL_SPACE)
	mutex_unlock(&ATBM253Mutex);
#elif (ATBM253_OS_TYPE_SEL == ATBM253_OS_TYPE_WINDOWS)
	ReleaseSemaphore(ATBM253Mutex,1,ATBM_NULL);
#else
	/*To realize mutex according to system platform.*/
	osal_mutex_unlock(ATBM253Mutex);
#endif
    	return ATBM253_NO_ERROR;
}


