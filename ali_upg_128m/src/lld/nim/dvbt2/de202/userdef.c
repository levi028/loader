

#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include"userdef.h"

extern ID f_de202_IIC_Sema_ID ;

/******************************************************************************
 * iic_write_val
 *-----------------------------------------------------------------------------
 * INT16    id      : Block ID 
 * UINT8    subadr  : sub address
 * UINT8    val     : data value
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 iic_write_val(INT16 id, UINT8 subadr, UINT8 val){
    iic_write(id, subadr, &val, 1);
    return 0;
}

/******************************************************************************
 * iic_write
 *-----------------------------------------------------------------------------
 * INT16    id      : Block ID 
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer to send
 * UINT8    len     : data byte length to send
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 iic_write(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len){
 osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);

    switch( (BLOCKID)id ) {
        
        case TUNER_TERR:
            write_tuner( DVBT_SLAVE_ADDRESS, TUNER_SLAVE_ADDRESS, subadr, data, len );
            break;
        case DEMOD_DVBT:
            write_demod( DVBT_SLAVE_ADDRESS, subadr, data, len );
            break;
        default :
            break;
    }
 osal_semaphore_release(f_de202_IIC_Sema_ID);

    return 0;
}

/******************************************************************************
 * iic_read
 *-----------------------------------------------------------------------------
 * INT16    id      : Block ID
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer to put
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 iic_read(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len){

 osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);

    switch( (BLOCKID)id ) {
        case TUNER_TERR:
            libc_printf("tuner read function is not implemented. \n");
            break;
        case DEMOD_DVBT:
            read_demod( DVBT_SLAVE_ADDRESS, subadr, data, len );
            break;
        default :
            break;
    }
 osal_semaphore_release(f_de202_IIC_Sema_ID);

    return 0;
}

/******************************************************************************
 * write_tuner
 *-----------------------------------------------------------------------------
 * UINT8    slv0    : Slave Address        (0xD8)
 * UINT8    slv1    : Tuner Slave Addressr (0xC0)
 * UINT8    subadr  : Sub Address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 write_tuner(
    UINT8 slv0,
    UINT8 slv1,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
 //   struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/
    UINT8 senddata[ MAX_LENGTH_TUNER_WRITE + 2];
    INT16 i;
    BOOL ret;
	
// osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);
    
    senddata[0] = 0x9;//tuner repeator
    senddata[1] = slv1;//tuner reg
      senddata[2] = subadr;//tuner reg
    
    if( len <= MAX_LENGTH_TUNER_WRITE ) {
        for(i = 0; i < len; i++){
            senddata[i+3] = data[i];  
        }
#if 0
        i2cMsg.type    = IIC_TASK_WRITE;
        i2cMsg.addr    = slv0;
        i2cMsg.subtype = IIC_SUB_1B;
        i2cMsg.sub     = (UINT16)(0x09);
        i2cMsg.size    = len + 2;
        i2cMsg.buf     = senddata;
#else
	ret = i2c_write(0,slv0, senddata, len+3);
	if(ret!=0){
		libc_printf("write_tuner err = %d \n",ret);
		 //osal_semaphore_release(f_de202_IIC_Sema_ID);
		 return  ret;
	}
#endif    
 //osal_semaphore_release(f_de202_IIC_Sema_ID);
        return 0;
    }        
    return 0;
}
    
/******************************************************************************
 * write_demod
 *-----------------------------------------------------------------------------
 * UINT8    slv     : Slave Address of Demod (0xD8)
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 write_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
	BOOL ret;
	UINT8  i, buffer[MAX_LENGTH_TUNER_WRITE+1];
#if 0
    struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/

    i2cMsg.type    = IIC_TASK_WRITE;
    i2cMsg.addr    = slv;
    i2cMsg.subtype = IIC_SUB_1B;
    i2cMsg.sub     = (UINT16)subadr;
    i2cMsg.size    = len;
    i2cMsg.buf     = data;
#else
 //osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);
	 buffer[0] = subadr;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = data[i];
    }
	ret = i2c_write(0,slv, buffer, len+1);
	if(ret!=0){
		libc_printf("write_demod err = %d , %x  %x  %x \n",ret,slv,buffer[0],buffer[1]);
		// osal_semaphore_release(f_de202_IIC_Sema_ID);
		 return  ret;
	}

#endif
        /*------------------------------------------------
            Add code here for I2C communication
        ------------------------------------------------*/
 //osal_semaphore_release(f_de202_IIC_Sema_ID);
    return 0;
}

/******************************************************************************
 * read_demod
 *-----------------------------------------------------------------------------
 * UINT8    slv     : Slave Address of Demod (0xD8)
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
INT16 read_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
	BOOL ret;
	UINT8  i, buffer[MAX_LENGTH_TUNER_WRITE];
	
#if 0
    struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/

    i2cMsg.type     = IIC_TASK_READ;
    i2cMsg.addr     = slv;
    i2cMsg.subtype  = IIC_SUB_1B;
    i2cMsg.sub      = (UINT16)subadr;
    i2cMsg.size     = len;
    i2cMsg.buf      = data;
#else
// osal_semaphore_capture(f_de202_IIC_Sema_ID,TMO_FEVR);
#if 0
	buffer[0] = subadr;
 
	ret = i2c_write_read(0, slv, buffer, 1, len);
	if(ret!=0)
	{	
		 libc_printf("read_demod err = %d ,subadr =%x \n",ret,subadr);
		// osal_semaphore_release(f_de202_IIC_Sema_ID);
		 return  ret;
	}

    for (i = 0; i < len; i++)
    {
        data[i] = buffer[i];
    }
#else
  for (i = 0; i < len; i++)
{
	buffer[0] = subadr+i; 
	ret = i2c_write_read(0, slv, buffer, 1, 1);
	if(ret!=0)
	{	
		 libc_printf("read_demod err = %d ,subadr =%x \n",ret,subadr);
		// osal_semaphore_release(f_de202_IIC_Sema_ID);
		 return  ret;
	}
	  data[i] = buffer[0];
}
#endif

#endif
        /*------------------------------------------------
            Add code here for I2C communication
        ------------------------------------------------*/
//  osal_semaphore_release(f_de202_IIC_Sema_ID);
    return 0;
}

/******************************************************************************
 * Waitms
 *-----------------------------------------------------------------------------
 * Input : INT16 ms
 * Output: none
 * Return: void
 *****************************************************************************/
void Waitms(UINT16 ms) {

     osal_task_sleep(ms);
    return;
}

/******************************************************************************
 * History
 *
 * 2010-03-24 v0.10 initial version
 *
 * 2010-05-21 v0.20 
 *                  Tuner setting was revised. (tuner_init,dvbt_tuner_tune,setRfTunerParam)
 *                  Tuning wait time was changed. (cmd_tune_dvbt,dvbt_tuner_tune)
 *                  Demod setting was revised. (CoreRegSetting_8MBW,CoreRegSetting_7MBW)
 *
 * 2010-06-10 v0.21 
 *                  Lock Sequence was updated.
 *                       (dvbt_lockseq_start,dvbt_lockseq_wait_tps_lock,dvbt_lockseq_wait_ts_lock)
 *
 * 2010-07-23 v0.30 
 *                  Descriptions of DE204 was removed.
 *
 ******************************************************************************/
