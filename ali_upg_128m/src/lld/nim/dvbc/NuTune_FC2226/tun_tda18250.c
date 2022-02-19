#if defined(__NIM_LINUX_PLATFORM__)
#include "tun_tda18250_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "tun_tda18250_tds.h"
#endif



#if 0
#define TDA18250_PRINTF     nim_print
#else
#define TDA18250_PRINTF(...)
#endif


#define MAGIC_CONFIG_1      1
#define MAGIC_CONFIG_15     15



#ifdef MAX_TUNER_SUPPORT_NUM
#undef MAX_TUNER_SUPPORT_NUM
#endif
#define MAX_TUNER_SUPPORT_NUM 4//1
#define NIM_TUNER_SET_STANDBY_CMD    0xffffffff
#define BURST_SZ 14

static struct QAM_TUNER_CONFIG_EXT * tda18250_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 tda18250_tuner_cnt = 0;
static UINT32 tda18250_tuner_i2c = 0;
static UINT8  tda18250ab_i2c_addr = 0;
static UINT32 g_mutex_lock_arr[MAX_TUNER_SUPPORT_NUM] = {0};
static INT8   tun_status=0;
static UINT8  t_val=0x00;

#if defined(__NIM_LINUX_PLATFORM__)

static struct mutex  g_mutex_lock;

#elif defined(__NIM_TDS_PLATFORM__)
static UINT32        g_mutex_lock=OSAL_INVALID_ID;

#endif


//*--------------------------------------------------------------------------------------
//* Prototype of function to be provided by customer
//*--------------------------------------------------------------------------------------
tmErrorCode_t     porting_i2c_read(tmUnitSelect_t tunit,UInt32 addrsize, UInt8* paddr,UInt32 readlen, UInt8* pdata);
tmErrorCode_t     porting_i2c_write(tmUnitSelect_t tunit, UInt32 addrsize, UInt8* paddr,UInt32 writelen, UInt8* pdata);
tmErrorCode_t     porting_wait(tmUnitSelect_t tunit, UInt32 tms);
tmErrorCode_t     porting_print(UInt32 level, const char* format, ...);
tmErrorCode_t      porting_mutex_init(ptmbslFrontEndMutexHandle *ppMutexHandle);
tmErrorCode_t      porting_mutex_deinit( ptmbslFrontEndMutexHandle pMutex);
tmErrorCode_t      porting_mutex_acquire(ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut);
tmErrorCode_t      porting_mutex_release(ptmbslFrontEndMutexHandle pMutex);


//*--------------------------------------------------------------------------------------
//* Template of function to be provided by customer
//*--------------------------------------------------------------------------------------

//*--------------------------------------------------------------------------------------
//* Function Name       : porting_i2c_read
//* Object              :
//* Input Parameters    :     tmUnitSelect_t tunit
//*                         UInt32 addrsize,
//*                         UInt8* paddr,
//*                         UInt32 readlen,
//*                         UInt8* pdata
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t porting_i2c_read(tmUnitSelect_t tunit,    UInt32 addrsize, UInt8* paddr,UInt32 readlen, UInt8* pdata)
{
       tmErrorCode_t err = TM_OK;
    INT32 result = 0;

    if (addrsize != MAGIC_CONFIG_1)
    {
        TDA18250_PRINTF("TDA18250 error 1!\n");
    }

    if (readlen > MAGIC_CONFIG_15)
    {
        TDA18250_PRINTF("TDA18250 error 2 !\n");
    }

    pdata[0] = paddr[0];

    result = nim_i2c_write_read(tda18250_tuner_i2c, tda18250ab_i2c_addr, pdata, 1, readlen);

    if (result == SUCCESS)
    {
        err = TM_OK;
    }
    else
    {
        err = !TM_OK;
    }


   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              :
//* Input Parameters    :     tmUnitSelect_t tunit
//*                         UInt32 addrsize,
//*                         UInt8* paddr,
//*                         UInt32 writelen,
//*                         UInt8* pdata
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------

tmErrorCode_t porting_i2c_write (tmUnitSelect_t tunit,     UInt32 addrsize, UInt8* paddr,UInt32 writelen, UInt8* pdata)
{
       tmErrorCode_t err = TM_OK;
    INT32 result = 0;
    UINT8 data[BURST_SZ+1] = {0}; // every time, write 14 byte..
    INT32 remainlen = 0 ;
    INT32 burstnum = 0;
    INT32 i = 0;
    INT32 j = 0;

    //TDA18250_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__,__LINE__);

    if (addrsize != 1)
    {
        TDA18250_PRINTF("TDA18250 error 1 !\n");

    }

    remainlen = writelen % BURST_SZ;
    if (remainlen)
    {
        burstnum = writelen / BURST_SZ;
    }
    else
    {
        burstnum = writelen / BURST_SZ - 1;
        remainlen = BURST_SZ;
    }

    for ( i = 0 ; i < burstnum; i ++ )
    {
        for ( j = 0 ; j < BURST_SZ ; j++  )
        {
            data[j+1]   = pdata[i * BURST_SZ + j ];
        }
        data[0] = paddr[0]+ (i* BURST_SZ);

        result |= nim_i2c_write(tda18250_tuner_i2c, tda18250ab_i2c_addr, data, BURST_SZ+1);

    }

    for ( i = 0 ; i < remainlen ; i++ )
    {
        data[i+1]   = pdata[burstnum * BURST_SZ + i ];
        data[0] = paddr[0]+burstnum*BURST_SZ;
    }

    result |= nim_i2c_write(tda18250_tuner_i2c, tda18250ab_i2c_addr, data, remainlen+1);

    if (result == SUCCESS)
    {
        err = TM_OK;
    }
    else
    {
        err = !TM_OK;
    }
   // TDA18250_PRINTF("[%s]line=%d,end!\n", __FUNCTION__,__LINE__);
   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : porting_wait
//* Object              :
//* Input Parameters    :     tmUnitSelect_t tunit
//*                         UInt32 tms
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t porting_wait(tmUnitSelect_t tunit, UInt32 tms)
{
    comm_sleep(tms);

   return TM_OK;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : porting_print
//* Object              :
//* Input Parameters    :     UInt32 level, const char* format, ...
//*
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t             porting_print(UInt32 level, const char* format, ...)
{

   return TM_OK;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : porting_mutex_init
//* Object              :
//* Input Parameters    :     ptmbslFrontEndMutexHandle *ppMutexHandle
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  porting_mutex_init(ptmbslFrontEndMutexHandle *pmutex)
{
    static UINT8 mux_cnt = 0;

    if (NULL == pmutex)
    {
        ASSERT(0);
        return !TM_OK;
    }

#if defined(__NIM_LINUX_PLATFORM__)

    mutex_init(&g_mutex_lock);
    g_mutex_lock_arr[mux_cnt] = (UINT32)(&g_mutex_lock);
    *pmutex = (void*)g_mutex_lock_arr[mux_cnt];
#elif defined(__NIM_TDS_PLATFORM__)

    g_mutex_lock=osal_mutex_create();
    if(g_mutex_lock == OSAL_INVALID_ID)
    {
        TDA18250_PRINTF("i2c mutex error\n");
    }
    g_mutex_lock_arr[mux_cnt] = (UINT32)(g_mutex_lock);
    *pmutex = (void*)&g_mutex_lock_arr[mux_cnt];
#endif

    //*ppMutexHandle = (ptmbslFrontEndMutexHandle*)&g_mutex_lock;
    //*pmutex = (void*)g_mutex_lock_arr[mux_cnt];

    //TDA18250_PRINTF("[%s]line=%d,end pMute=0x%x!\n", __FUNCTION__,__LINE__,*ppMutexHandle);
    mux_cnt += 1;

    //

   return TM_OK;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : porting_mutex_deinit
//* Object              :
//* Input Parameters    :      ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  porting_mutex_deinit( ptmbslFrontEndMutexHandle pmutex)
{

#if defined(__NIM_LINUX_PLATFORM__)
    mutex_destroy((struct mutex *)pmutex);
#elif defined(__NIM_TDS_PLATFORM__)
    osal_mutex_delete(*((OSAL_ID*)pmutex));
#endif

   return TM_OK;
}



//*--------------------------------------------------------------------------------------
//* Function Name       : porting_mutex_acquire
//* Object              :
//* Input Parameters    :     ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  porting_mutex_acquire(ptmbslFrontEndMutexHandle pmutex, UInt32 timeout)
{
//    TDA18250_PRINTF("[%s]line=%d,enter pMute=0x%x!\n", __FUNCTION__,__LINE__,pmutex);

#if defined(__NIM_LINUX_PLATFORM__)
    char i = 0;

    for (i=0; i<4; i++)
    {
       if (0 == mutex_trylock((struct mutex *)pmutex))
       {
            break;
        }
        comm_sleep(timeout/3400);  //timeOut = TDA18250_MUTEX_TIMEOUT(5000) need improve
    }

#elif defined(__NIM_TDS_PLATFORM__)

    osal_mutex_lock(*((OSAL_ID*)pmutex), timeout);

#endif

//    TDA18250_PRINTF("[%s]line=%d,end!\n", __FUNCTION__,__LINE__);
    return TM_OK;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : porting_mutex_release
//* Object              :
//* Input Parameters    :     ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  porting_mutex_release(ptmbslFrontEndMutexHandle pmutex)
{

#if defined(__NIM_LINUX_PLATFORM__)
    mutex_unlock((struct mutex *)pmutex);
#elif defined(__NIM_TDS_PLATFORM__)
    osal_mutex_unlock(*((OSAL_ID*)pmutex));
#endif

   return TM_OK;
}


INT32 tun_tda18250_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrtuner_config)
{
    /* check Tuner Configuration structure is available or not */

    if ((ptrtuner_config == NULL) || (tda18250_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
    {
        TDA18250_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
        return ERR_FAILUE;
    }


    
    tda18250_tuner_i2c = ptrtuner_config->i2c_type_id;
    tda18250ab_i2c_addr = ptrtuner_config->c_tuner_base_addr;
    tda18250_dev_id[tda18250_tuner_cnt] = ptrtuner_config;
    *tuner_id = tda18250_tuner_cnt;

    TDA18250_PRINTF("[%s]line=%d,enter i2c_type=0x%x,i2c_addr=0x%x!\n", __FUNCTION__,__LINE__,tda18250_tuner_i2c,tda18250ab_i2c_addr);

    tda18250_tuner_cnt ++;

    TDA18250_PRINTF("[%s] end !\n", __FUNCTION__);

    return SUCCESS;
}


INT32 tun_tda18250_close(UINT32 tuner_id)
{
#ifndef TWO_TUNER_SUPPORT
    tda18250_tuner_cnt = 0;
#else
    if (0 >= tda18250_tuner_cnt)
    {
        return SUCCESS;
    }
    tda18250_tuner_cnt--;
#endif
    return SUCCESS;
}



INT32 tun_tda18250_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32            result = 0;
    tmErrorCode_t    err = 0;
    tmbslFrontEndState_t plllockmaster = tmbslFrontEndStateUnknown;

    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    TDA18250_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__,__LINE__);

    if ((tuner_id>=tda18250_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        return ERR_FAILUE;
    }

    tuner_dev_ptr = tda18250_dev_id[tuner_id];

    /* Get TDA18250 Master PLL Lock status */
    err = tmbslTDA18250GetLockStatus(0, &plllockmaster);
    if(err != TM_OK)
    {
        *lock = 0;
        result = !SUCCESS;
    }
    else
    {
        if (tmbslFrontEndStateLocked == plllockmaster)
        {
            *lock = 1;
        }
        else
        {
            *lock = 0;
        }
        result = SUCCESS;
    }
       TDA18250_PRINTF("[%s]line=%d,end!\n", __FUNCTION__,__LINE__);
    return result;
}

INT32 tun_tda18250_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd)
{
    INT32           result = 0;
    tmErrorCode_t     err = 0;
    UINT32             urfmaster = freq*1000;
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

    if (tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        ASSERT(0);
        return RET_FAILURE;
    }

    tuner_dev_ptr = tda18250_dev_id[tuner_id];


    if(tuner_dev_ptr->c_tuner_freq_param !=0x00)
    {
        if(t_val!=tuner_dev_ptr->c_tuner_freq_param)
        {
            t_val=tuner_dev_ptr->c_tuner_freq_param;
            tun_status=0x00;
        }
    }


    TDA18250_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__,__LINE__);
    if (0==tun_status)
    {
       /* Variable declarations */
       tmErrorCode_t err = TM_OK;
       tmbslFrontEndDependency_t ssrvtunerfunc;

       tmTDA18250StandardMode_t stdmodemaster = tmTDA18250_DIG_9MHz;

       switch(tuner_dev_ptr->c_tuner_freq_param)
           {
               case 0x09:
                stdmodemaster=tmTDA18250_DIG_9MHz;
                break;
            case 0x0C:
                stdmodemaster=tmTDA18250_DIG_6MHz;
                break;
            default:
                stdmodemaster = tmTDA18250_DIG_9MHz;
                break;
           }
       tuner_dev_ptr->c_tuner_freq_param =0x00;
       memset(&ssrvtunerfunc, 0, sizeof(ssrvtunerfunc));
    /* Low layer struct set-up to link with user written functions */
       ssrvtunerfunc.sIo.Write             = porting_i2c_write;
       ssrvtunerfunc.sIo.Read              = porting_i2c_read;
       ssrvtunerfunc.sTime.Get             = Null;
       ssrvtunerfunc.sTime.Wait            = porting_wait;
       ssrvtunerfunc.sDebug.Print          = porting_print;
       ssrvtunerfunc.sMutex.Init           = porting_mutex_init;
       ssrvtunerfunc.sMutex.DeInit         = porting_mutex_deinit;
       ssrvtunerfunc.sMutex.Acquire        = porting_mutex_acquire;
       ssrvtunerfunc.sMutex.Release        = porting_mutex_release;
       ssrvtunerfunc.dwAdditionalDataSize  = 0;
       ssrvtunerfunc.pAdditionalData       = Null;

       /* TDA18250 Master Driver low layer setup */
       err = tmbslTDA18250Init(0, &ssrvtunerfunc);
       if(err != TM_OK)
       {
           return err;
       }

       /* TDA18250 Master Hardware initialization */
       err = tmbslTDA18250Reset(0);
       if(err != TM_OK)
       {
           return err;
       }

       /* TDA18250 Master Hardware power state */
       err = tmbslTDA18250SetPowerState(0, tmTDA18250_PowerNormalMode);
       if(err != TM_OK)
       {
          return err;
       }

       /* TDA18250 Master standard mode */
       err = tmbslTDA18250SetStandardMode(0, stdmodemaster);
       if(err != TM_OK)
       {
           return err;
       }

        tun_status = 1;
    }

        //kent,2013-7-13
    if( freq == NIM_TUNER_SET_STANDBY_CMD )
    {
        if( sym==0 )
        {
            TDA18250_PRINTF("[%s]line=%d,standby!!\n",__FUNCTION__,__LINE__);
            tmbslTDA18250SetPowerState(0, tmTDA18250_PowerStandbyWithLoopThroughOnAndWithXtalOn);
        }
        else if(MAGIC_CONFIG_1 == sym)
        {
            tmbslTDA18250SetPowerState(0, tmTDA18250_PowerNormalMode);
        }
        else
        {
            return ERR_FAILED;
        }

        TDA18250_PRINTF("[%s]line=%d,standby,return!!\n",__FUNCTION__,__LINE__);
        return SUCCESS;


    }

       /* TDA18250 Master RF frequency */
       err = tmbslTDA18250SetRf(0, urfmaster);
       if(err != TM_OK)
       {
           return err;
        }

    comm_sleep(100);

    if (err !=TM_OK)
    {
        result= ERR_FAILED;
    }
    else
    {
        result = SUCCESS;
    }

    TDA18250_PRINTF("[%s]line=%d,end!\n", __FUNCTION__,__LINE__);

    return result;

}

INT32 tun_tda18250_control_X(UINT32 tuner_id, UINT32 freq, UINT32 sym)
{
   return tun_tda18250_control(tuner_id, freq, sym,0, 0);
}
