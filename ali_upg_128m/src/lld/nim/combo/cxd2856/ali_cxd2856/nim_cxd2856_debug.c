#include "nim_cxd2856_common.h"
static void  nim_cxd2856_monitor_task(UINT32 param1, UINT32 param2)
{
	struct nim_device *dev = (struct nim_device *) param1;
	//sony_integ_t * priv = (sony_integ_t *)dev->priv;
	//sony_demod_t * pdemod = priv->pDemod;
	UINT32 per = 0;
	UINT32 start_time = 0;
	
	start_time = osal_get_tick();
	while(1)
	{	 
	
		if (osal_get_tick() - start_time > 100)
		{			
			
			nim_cxd2856_get_per(dev, &per);
			if(0 != per)
			{
				libc_printf("++++++++ per = %d\n",per);
			}
			start_time = osal_get_tick();
		}
		osal_task_sleep(10); 	
	}


}

INT32 nim_cxd2856_monitor_task_init(struct nim_device *dev)
{
	T_CTSK nim_task_praram;
	sony_integ_t * priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;

    MEMSET(&nim_task_praram, 0, sizeof(nim_task_praram));
    nim_task_praram.task = nim_cxd2856_monitor_task;
    nim_task_praram.stksz = 0x1000 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 5 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;//Reserved for future use.
    pdemod->thread_id = osal_task_create(&nim_task_praram);
	
    if (OSAL_INVALID_ID == pdemod->thread_id)
    {
    	libc_printf("nim cxd2856 monitor task create fail\n");
        return OSAL_E_FAIL;
    }
	return SUCCESS;
}
/*void CXD2856_LOG_I2C(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len)
{
	UINT8 i = 0;
    if (write)
        CXD2856_LOG(param, "I2C_Write,0x%02X", (slv_addr&0xFE));
    else
        CXD2856_LOG(param, "I2C_Read,0x%02X", (slv_addr|1));

    for ( i=0; i<len; ++i )
    {
        CXD2856_LOG(param, ",0x%02X", data[i]);
    }

    if (err)
        CXD2856_LOG(param, "\terror");
    CXD2856_LOG(param, "\r\n");
}*/

